/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "forage.h"

#include "common/eqemu_logsys.h"
#include "common/events/player_event_logs.h"
#include "common/misc_functions.h"
#include "common/repositories/criteria/content_filter_criteria.h"
#include "common/repositories/fishing_repository.h"
#include "common/repositories/forage_repository.h"
#include "common/rulesys.h"
#include "common/strings.h"
#include "zone/entity.h"
#include "zone/npc.h"
#include "zone/queryserv.h"
#include "zone/quest_parser_collection.h"
#include "zone/string_ids.h"
#include "zone/titles.h"
#include "zone/water_map.h"
#include "zone/worldserver.h"
#include "zone/zonedb.h"

#include <numbers>

extern WorldServer worldserver;
extern QueryServ  *QServ;

struct NPCType;

//max number of items which can be in the foraging
// and fishing tables for a given zone.
constexpr uint8 FORAGE_ITEM_LIMIT = 50;
constexpr uint8 FISHING_ITEM_LIMIT = 50;

uint32 ZoneDatabase::LoadForage(uint32 zone_id, uint8 skill_level)
{
	uint32 forage_items[FORAGE_ITEM_LIMIT] = {};

	const auto& l = ForageRepository::GetWhere(
		*this,
		fmt::format(
			"(`zoneid` = {} || `zoneid` = 0) AND `level` <= {} {} LIMIT {}",
			zone_id,
			skill_level,
			ContentFilterCriteria::apply(),
			FORAGE_ITEM_LIMIT
		)
	);

	if (l.empty()) {
		return 0;
	}

	LogInfo(
		"Loaded [{}] Forage Item{}",
		Strings::Commify(l.size()),
		l.size() != 1 ? "s" : ""
	);

	int forage_chances[FORAGE_ITEM_LIMIT] = {};

	int    current_chance = 0;
	uint32 item_id        = 0;
	uint8  count          = 0;

	for (const auto& e : l) {
		if (count >= FORAGE_ITEM_LIMIT) {
			break;
		}

		forage_items[count]   = e.Itemid;
		forage_chances[count] = e.chance + current_chance;

		current_chance = forage_chances[count];

		count++;
	}

	if (current_chance == 0 || count < 1) {
		return 0;
	}

	if (count == 1) {
		return forage_items[0];
	}

	const int roll = zone->random.Int(1, current_chance);

	for (uint16 slot_id = 0; slot_id < count; slot_id++) {
		if (roll <= forage_chances[slot_id]) {
			item_id = forage_items[slot_id];
			break;
		}
	}

	return item_id;
}

uint32 ZoneDatabase::LoadFishing(uint32 zone_id, uint8 skill_level, uint32 &npc_id, uint8 &npc_chance)
{
	uint32 fishing_items[FISHING_ITEM_LIMIT] = {};
	int fishing_chances[FISHING_ITEM_LIMIT] = {};

	const auto& l = FishingRepository::GetWhere(
		*this,
		fmt::format(
			"(`zoneid` = {} || `zoneid` = 0) AND `skill_level` <= {} {}",
			zone_id,
			skill_level,
			ContentFilterCriteria::apply()
		)
	);

	if (l.empty()) {
		return 0;
	}

	LogInfo(
		"Loaded [{}] Fishing Item{}",
		Strings::Commify(l.size()),
		l.size() != 1 ? "s" : ""
	);

	uint32 npc_ids[FISHING_ITEM_LIMIT] = {};
	uint32 npc_chances[FISHING_ITEM_LIMIT] = {};

	int    current_chance = 0;
	uint32 item_id        = 0;
	uint8  count          = 0;

	for (const auto &e: l) {
		if (count >= FISHING_ITEM_LIMIT) {
			break;
		}

		fishing_items[count]   = e.Itemid;
		fishing_chances[count] = e.chance + current_chance;
		npc_ids[count]         = e.npc_id;
		npc_chances[count]     = e.npc_chance;

		current_chance = fishing_chances[count];

		count++;
	}

	npc_id     = 0;
	npc_chance = 0;

	if (count <= 0) {
		return 0;
	}

	const int roll = zone->random.Int(1, current_chance);

	for (uint8 i = 0; i < count; i++) {
		if (roll > fishing_chances[i]) {
			continue;
		}

		item_id    = fishing_items[i];
		npc_id     = npc_ids[i];
		npc_chance = npc_chances[i];
		break;
	}

	return item_id;
}

//we need this function to immediately determine, after we receive OP_Fishing, if we can even try to fish, otherwise we have to wait a while to get the failure
bool Client::CanFish() {
	//make sure we still have a fishing pole on:
	const EQ::ItemInstance* Pole = m_inv[EQ::invslot::slotPrimary];
	int32 bslot = m_inv.HasItemByUse(EQ::item::ItemTypeFishingBait, 1, invWhereWorn | invWherePersonal);
	const EQ::ItemInstance* Bait = nullptr;
	if (bslot != INVALID_INDEX)
		Bait = m_inv.GetItem(bslot);

	if (!Pole || !Pole->IsClassCommon() || Pole->GetItem()->ItemType != EQ::item::ItemTypeFishingPole) {
		if (m_inv.HasItemByUse(EQ::item::ItemTypeFishingPole, 1, invWhereWorn | invWherePersonal | invWhereBank | invWhereSharedBank | invWhereTrading | invWhereCursor))	//We have a fishing pole somewhere, just not equipped
			MessageString(Chat::Skills, FISHING_EQUIP_POLE);	//You need to put your fishing pole in your primary hand.
		else	//We don't have a fishing pole anywhere
			MessageString(Chat::Skills, FISHING_NO_POLE);	//You can't fish without a fishing pole, go buy one.
		return false;
	}

	if (!Bait || !Bait->IsClassCommon() || Bait->GetItem()->ItemType != EQ::item::ItemTypeFishingBait) {
		MessageString(Chat::Skills, FISHING_NO_BAIT);	//You can't fish without fishing bait, go buy some.
		return false;
	}

	if(zone->zonemap != nullptr && zone->watermap != nullptr && RuleB(Watermap, CheckForWaterWhenFishing)) {

		glm::vec3 rodPosition;
		// Tweak Rod and LineLength if required
		const float RodLength = RuleR(Watermap, FishingRodLength);
		const float LineLength = RuleR(Watermap, FishingLineLength);
		int HeadingDegrees;

		HeadingDegrees = (int) ((GetHeading()*360)/512);
		HeadingDegrees = HeadingDegrees % 360;

		rodPosition.x = m_Position.x + RodLength * sin(HeadingDegrees * std::numbers::pi / 180.0f);
		rodPosition.y = m_Position.y + RodLength * cos(HeadingDegrees * std::numbers::pi / 180.0f);
		rodPosition.z = m_Position.z;

		float bestz = zone->zonemap->FindBestZ(rodPosition, nullptr);
		float len = m_Position.z - bestz;
		if(len > LineLength || len < 0.0f) {
			MessageString(Chat::Skills, FISHING_LAND);
			return false;
		}

		float step_size = RuleR(Watermap, FishingLineStepSize);

		for(float i = 0.0f; i < LineLength; i += step_size) {
			glm::vec3 dest(rodPosition.x, rodPosition.y, m_Position.z - i);

			bool in_lava = zone->watermap->InLava(dest);
			bool in_water = zone->watermap->InWater(dest) || zone->watermap->InVWater(dest);

			if (in_lava) {
				MessageString(Chat::Skills, FISHING_LAVA);	//Trying to catch a fire elemental or something?
				return false;
			}

			if(in_water) {
				return true;
			}
		}

		MessageString(Chat::Skills, FISHING_LAND);
		return false;
	}
	return true;
}

void Client::GoFish(bool guarantee, bool use_bait)
{

	//TODO: generate a message if we're already fishing
	/*if (!fishing_timer.Check()) {	//this isn't the right check, may need to add something to the Client class like 'bool is_fishing'
		MessageString(Chat::White, ALREADY_FISHING);	//You are already fishing!
		return;
	}*/

	fishing_timer.Disable();

	//we're doing this a second time (1st in Client::Handle_OP_Fishing) to make sure that, between when we started fishing & now, we're still able to fish (in case we move, change equip, etc)
	if (!CanFish()) { //if we can't fish here, we don't need to bother with the rest
		return;
	}

	//multiple entries yeilds higher probability of dropping...
	uint32 common_fish_ids[MAX_COMMON_FISH_IDS] = {
		1038,  // Tattered Cloth Sandals
		1038,  // Tattered Cloth Sandals
		1038,  // Tattered Cloth Sandals
		13019, // Fresh Fish
		13076, // Fish Scales
		13076, // Fish Scales
		7007,  // Rusty Dagger
		7007,  // Rusty Dagger
		7007   // Rusty Dagger
	};

	//success formula is not researched at all

	uint16 fishing_skill = GetSkill(EQ::skills::SkillFishing);	//will take into account skill bonuses on pole & bait

	//make sure we still have a fishing pole on:
	int16 bslot = m_inv.HasItemByUse(EQ::item::ItemTypeFishingBait, 1, invWhereWorn | invWherePersonal);
	const EQ::ItemInstance* Bait = nullptr;
	if (bslot != INVALID_INDEX) {
		Bait = m_inv.GetItem(bslot);
	}

	//if the bait isnt equipped, need to add its skill bonus
	if (bslot >= EQ::invslot::GENERAL_BEGIN && Bait != nullptr && Bait->GetItem()->SkillModType == EQ::skills::SkillFishing) {
		fishing_skill += Bait->GetItem()->SkillModValue;
	}

	if (fishing_skill > 100)
	{
		fishing_skill = 100+((fishing_skill-100)/2);
	}

	if (guarantee || zone->random.Int(0,175) < fishing_skill) {
		uint32 food_id = 0;

		//chance to fish a zone item.
		if (zone->random.Int(0, RuleI(Zone, FishingChance)) <= fishing_skill ) {
			uint32 npc_id = 0;
			uint8 npc_chance = 0;
			food_id = content_db.LoadFishing(m_pp.zone_id, fishing_skill, npc_id, npc_chance);

			//check for add NPC
			if (npc_chance > 0 && npc_id) {
				if (zone->random.Roll(npc_chance)) {
					if (zone->CanDoCombat()) {
						const NPCType *tmp = content_db.LoadNPCTypesData(npc_id);
						if (tmp != nullptr) {
							auto positionNPC = GetPosition();
							positionNPC.x = positionNPC.x + 3;
							auto npc = new NPC(tmp, nullptr, positionNPC, GravityBehavior::Water);
							npc->AddLootTable();
							if (npc->DropsGlobalLoot())
								npc->CheckGlobalLootTables();

							npc->AddToHateList(this, 1, 0, false); // no help yelling

							entity_list.AddNPC(npc);

							Message(Chat::Emote,
								"You fish up a little more than you bargained for...");
						}
					}
					else {
						Message(Chat::Emote, "You notice something lurking just below the water's surface...");
					}
				}
			}
		}

		if (use_bait) {
			//consume bait, should we always consume bait on success?
			DeleteItemInInventory(bslot, 1, true);    //do we need client update?
		}

		if (food_id == 0) {
			int index = zone->random.Int(0, MAX_COMMON_FISH_IDS-1);
			food_id = (RuleB(Character, UseNoJunkFishing) ? 13019 : common_fish_ids[index]);
		}

		const EQ::ItemData* food_item = database.GetItem(food_id);
		if (food_item) {

			if (food_item->ItemType != EQ::item::ItemTypeFood) {
				MessageString(Chat::Skills, FISHING_SUCCESS);
			}
			else {
				MessageString(Chat::Skills, FISHING_SUCCESS_FISH_NAME, food_item->Name);
			}

			EQ::ItemInstance* inst = database.CreateItem(food_item, 1);
			if (inst != nullptr) {
				if (CheckLoreConflict(inst->GetItem()))
				{
					MessageString(Chat::White, DUP_LORE);
					safe_delete(inst);
				}
				else
				{
					PushItemOnCursor(*inst);
					SendItemPacket(EQ::invslot::slotCursor, inst, ItemPacketLimbo);
					if (RuleB(TaskSystem, EnableTaskSystem))
						UpdateTasksForItem(TaskActivityType::Fish, food_id);

					safe_delete(inst);
					inst = m_inv.GetItem(EQ::invslot::slotCursor);
				}

				if (inst) {
					if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::FISH_SUCCESS)) {
						auto e = PlayerEvent::FishSuccessEvent{
							.item_id      = inst->GetItem()->ID,
							.augment_1_id = inst->GetAugmentItemID(0),
							.augment_2_id = inst->GetAugmentItemID(1),
							.augment_3_id = inst->GetAugmentItemID(2),
							.augment_4_id = inst->GetAugmentItemID(3),
							.augment_5_id = inst->GetAugmentItemID(4),
							.augment_6_id = inst->GetAugmentItemID(5),
							.item_name    = inst->GetItem()->Name,
						};
						RecordPlayerEventLog(PlayerEvent::FISH_SUCCESS, e);
					}

					CheckItemDiscoverability(inst->GetID());

					if (parse->PlayerHasQuestSub(EVENT_FISH_SUCCESS)) {
						std::vector<std::any> args = {inst};
						parse->EventPlayer(EVENT_FISH_SUCCESS, this, "", inst->GetID(), &args);
					}
				}
			}
		}
	}
	else
	{
		//chance to use bait when you dont catch anything...
		if (zone->random.Int(0, 4) == 1) {
			DeleteItemInInventory(bslot, 1, true);	//do we need client update?
			MessageString(Chat::Skills, FISHING_LOST_BAIT);	//You lost your bait!
		} else {
			if (zone->random.Int(0, 15) == 1)	//give about a 1 in 15 chance to spill your beer. we could make this a rule, but it doesn't really seem worth it
				//TODO: check for & consume an alcoholic beverage from inventory when this triggers, and set it as a rule that's disabled by default
				MessageString(Chat::Skills, FISHING_SPILL_BEER);	//You spill your beer while bringing in your line.
			else
				MessageString(Chat::Skills, FISHING_FAILED);	//You didn't catch anything.
		}

		RecordPlayerEventLog(PlayerEvent::FISH_FAILURE, PlayerEvent::EmptyEvent{});
		if (parse->PlayerHasQuestSub(EVENT_FISH_FAILURE)) {
			parse->EventPlayer(EVENT_FISH_FAILURE, this, "", 0);
		}
	}

	//chance to break fishing pole...
	//this is potentially exploitable in that they can fish
	//and then swap out items in primary slot... too lazy to fix right now
	const EQ::ItemInstance* Pole = m_inv[EQ::invslot::slotPrimary];

	if (Pole) {
		const EQ::ItemData* fishing_item = Pole->GetItem();
		if (fishing_item && fishing_item->SubType == 0 && zone->random.Int(0, 49) == 1) {
			MessageString(Chat::Skills, FISHING_POLE_BROKE);	//Your fishing pole broke!
			DeleteItemInInventory(EQ::invslot::slotPrimary, 0, true);
		}
	}

	if (CheckIncreaseSkill(EQ::skills::SkillFishing, nullptr, 5))
	{
		if (title_manager.IsNewTradeSkillTitleAvailable(EQ::skills::SkillFishing, GetRawSkill(EQ::skills::SkillFishing)))
			NotifyNewTitlesAvailable();
	}
}

void Client::ForageItem(bool guarantee) {
	int skill_level = GetSkill(EQ::skills::SkillForage);

	//be wary of the string ids in switch below when changing this.
	uint32 common_food_ids[MAX_COMMON_FOOD_IDS] = {
		13046, // Fruit
		13045, // Berries
		13419, // Vegetables
		13048, // Rabbit Meat
		13047, // Roots
		13044, // Pod of Water
		14905, // Mushroom
		13106  // Fishing Grubs
	};

	// these may need to be fine tuned, I am just guessing here
	if (guarantee || zone->random.Int(0,199) < skill_level) {
		uint32 foragedfood = 0;
		uint32 stringid = FORAGE_NOEAT;

		if (zone->random.Roll(RuleI(Zone, ForageChance))) {
			foragedfood = content_db.LoadForage(m_pp.zone_id, skill_level);
		}

		//not an else in case theres no DB food
		if (foragedfood == 0 && RuleB(Character, UseForageCommonFood)) {
			uint8 index = 0;
			index = zone->random.Int(0, MAX_COMMON_FOOD_IDS-1);
			foragedfood = common_food_ids[index];
		}

		const EQ::ItemData* food_item = database.GetItem(foragedfood);

		if (!food_item) {
			LogError("nullptr returned from database.GetItem in ClientForageItem");
			return;
		}

		if (foragedfood == 13106) {
			stringid = FORAGE_GRUBS;
		} else {
			switch(food_item->ItemType) {
			case EQ::item::ItemTypeFood:
				stringid = FORAGE_FOOD;
				break;
			case EQ::item::ItemTypeDrink:
				if (strstr(food_item->Name, "ater")) {
					stringid = FORAGE_WATER;
				} else {
					stringid = FORAGE_DRINK;
				}
				break;
			default:
				break;
			}
		}

		MessageString(Chat::Skills, stringid);
		EQ::ItemInstance* inst = database.CreateItem(food_item, 1);
		if (inst != nullptr) {
			// check to make sure it isn't a foraged lore item
			if (CheckLoreConflict(inst->GetItem())) {
				MessageString(Chat::White, DUP_LORE);
				safe_delete(inst);
			} else {
				PushItemOnCursor(*inst);
				SendItemPacket(EQ::invslot::slotCursor, inst, ItemPacketLimbo);
				if(RuleB(TaskSystem, EnableTaskSystem)) {
					UpdateTasksForItem(TaskActivityType::Forage, foragedfood);
				}

				safe_delete(inst);
				inst = m_inv.GetItem(EQ::invslot::slotCursor);
			}

			if (inst) {
				if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::FORAGE_SUCCESS)) {
					auto e = PlayerEvent::ForageSuccessEvent{
						.item_id      = inst->GetItem()->ID,
						.augment_1_id = inst->GetAugmentItemID(0),
						.augment_2_id = inst->GetAugmentItemID(1),
						.augment_3_id = inst->GetAugmentItemID(2),
						.augment_4_id = inst->GetAugmentItemID(3),
						.augment_5_id = inst->GetAugmentItemID(4),
						.augment_6_id = inst->GetAugmentItemID(5),
						.item_name    = inst->GetItem()->Name,
					};
					RecordPlayerEventLog(PlayerEvent::FORAGE_SUCCESS, e);
				}

				CheckItemDiscoverability(inst->GetID());

				if (parse->PlayerHasQuestSub(EVENT_FORAGE_SUCCESS)) {
					std::vector<std::any> args = { inst };
					parse->EventPlayer(EVENT_FORAGE_SUCCESS, this, "", inst->GetID(), &args);
				}
			}
		}

		int ChanceSecondForage = aabonuses.ForageAdditionalItems + itembonuses.ForageAdditionalItems + spellbonuses.ForageAdditionalItems;
		if (!guarantee && zone->random.Roll(ChanceSecondForage)) {
			MessageString(Chat::Skills, FORAGE_MASTERY);
			ForageItem(true);
		}
	} else {
		MessageString(Chat::Skills, FORAGE_FAILED);
		RecordPlayerEventLog(PlayerEvent::FORAGE_FAILURE, PlayerEvent::EmptyEvent{});

		if (parse->PlayerHasQuestSub(EVENT_FORAGE_FAILURE)) {
			parse->EventPlayer(EVENT_FORAGE_FAILURE, this, "", 0);
		}
	}

	CheckIncreaseSkill(EQ::skills::SkillForage, nullptr, 5);
}
