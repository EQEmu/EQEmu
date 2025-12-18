#include "corpse.h"

#include "common/data_verification.h"
#include "common/eqemu_logsys.h"
#include "common/events/player_event_logs.h"
#include "common/json/json.hpp"
#include "common/repositories/character_corpse_items_repository.h"
#include "common/repositories/character_corpses_repository.h"
#include "common/rulesys.h"
#include "common/say_link.h"
#include "common/strings.h"
#include "zone/bot.h"
#include "zone/dynamic_zone.h"
#include "zone/entity.h"
#include "zone/groups.h"
#include "zone/mob.h"
#include "zone/queryserv.h"
#include "zone/quest_parser_collection.h"
#include "zone/raids.h"
#include "zone/string_ids.h"
#include "zone/worldserver.h"

#include <iostream>

using json = nlohmann::json;

extern EntityList           entity_list;
extern Zone                *zone;
extern WorldServer          worldserver;
extern npcDecayTimes_Struct npcCorpseDecayTimes[100];
extern QueryServ           *QServ;

void Corpse::SendEndLootErrorPacket(Client *client)
{
	auto outapp = new EQApplicationPacket(OP_LootComplete, 0);
	client->QueuePacket(outapp);
	safe_delete(outapp);
}

void Corpse::SendLootReqErrorPacket(Client *client, LootResponse response)
{
	auto outapp = new EQApplicationPacket(OP_MoneyOnCorpse, sizeof(moneyOnCorpseStruct));
	auto *d     = (moneyOnCorpseStruct *) outapp->pBuffer;
	d->response = static_cast<uint8>(response);
	d->unknown1 = 0x5a;
	d->unknown2 = 0x40;
	client->QueuePacket(outapp);
	safe_delete(outapp);
}

Corpse::Corpse(
	NPC *npc,
	LootItems *item_list,
	uint32 npc_type_id,
	const NPCType **npc_type_data,
	uint32 decay_time
) : Mob(
	"Unnamed_Corpse", // in_name
	"", // in_lastname
	0, // in_cur_hp
	0, // in_max_hp
	npc->GetGender(), // in_gender
	npc->GetRace(), // in_race
	npc->GetClass(), // in_class
	BodyType::Humanoid, // in_bodytype
	npc->GetDeity(), // in_deity
	npc->GetLevel(), // in_level
	npc->GetNPCTypeID(), // in_npctype_id
	npc->GetSize(), // in_size
	0.0f, // in_runspeed
	npc->GetPosition(), // position
	npc->GetInnateLightType(), // in_light
	npc->GetTexture(), // in_texture
	npc->GetHelmTexture(), // in_helmtexture
	0, // in_ac
	0, // in_atk
	0, // in_str
	0, // in_sta
	0, // in_dex
	0, // in_agi
	0, // in_int
	0, // in_wis
	0, // in_cha
	0, // in_haircolor
	0, // in_beardcolor
	0, // in_eyecolor1
	0, // in_eyecolor2
	0, // in_hairstyle
	0, // in_luclinface
	0, // in_beard
	0, // in_drakkin_heritage
	0, // in_drakkin_tattoo
	0, // in_drakkin_details
	EQ::TintProfile(), // in_armor_tint
	0xFF, // in_aa_title
	0, // in_see_invis
	0, // in_see_invis_undead
	0, // in_see_hide
	0, // in_see_improved_hide
	0, // in_hp_regen
	0, // in_mana_regen
	0, // in_qglobal
	0, // in_maxlevel
	0, // in_scalerate
	0, // in_armtexture
	0, // in_bracertexture
	0, // in_handtexture
	0, // in_legtexture
	0, // in_feettexture
	(*npc_type_data)->use_model, // in_usemodel
	false, // in_always_aggros_foes
	0, // in_heroic_strikethrough
	false // in_keeps_sold_items
),
	m_corpse_decay_timer(decay_time),
	m_corpse_rezzable_timer(0),
	m_corpse_delay_timer(RuleI(NPC, CorpseUnlockTimer)),
	m_corpse_graveyard_timer(0),
	m_loot_cooldown_timer(10)
{
	m_corpse_graveyard_timer.Disable();

	m_is_corpse_changed         = false;
	m_is_player_corpse          = false;
	m_is_locked                 = false;
	m_being_looted_by_entity_id = 0xFFFFFFFF;
	if (item_list) {
		m_item_list = *item_list;
		item_list->clear();
	}

	SetCash(npc->GetCopper(), npc->GetSilver(), npc->GetGold(), npc->GetPlatinum());

	npctype_id = npc_type_id;
	SetPlayerKillItemID(0);
	m_character_id        = 0;
	m_corpse_db_id        = 0;
	m_player_corpse_depop = false;
	strcpy(corpse_name, npc->GetName());
	strcpy(name, npc->GetName());

	for (auto &npcCorpseDecayTime: npcCorpseDecayTimes) {
		if (
			EQ::ValueWithin(
				level,
				npcCorpseDecayTime.minlvl,
				npcCorpseDecayTime.maxlvl
			)
			) {
			m_corpse_decay_timer.SetTimer(npcCorpseDecayTime.seconds * 1000);
			break;
		}
	}

	if (IsEmpty()) {
		m_corpse_decay_timer.SetTimer(RuleI(NPC, EmptyNPCCorpseDecayTime) + 1000);
	}


	if (npc->HasPrivateCorpse()) {
		m_corpse_delay_timer.SetTimer(m_corpse_decay_timer.GetRemainingTime() + 1000);
	}

	for (int &allowed_looter: m_allowed_looters) {
		allowed_looter = 0;
	}

	m_rezzed_experience = 0;

	UpdateEquipmentLight();
	UpdateActiveLight();

	m_loot_request_type = LootRequestType::Forbidden;
}

Corpse::Corpse(Client *c, int32 rez_exp, KilledByTypes in_killed_by) : Mob(
	"Unnamed_Corpse", // in_name
	"", // in_lastname
	0, // in_cur_hp
	0, // in_max_hp
	c->GetGender(), // in_gender
	c->GetRace(), // in_race
	c->GetClass(), // in_class
	BodyType::Humanoid, // in_bodytype
	c->GetDeity(), // in_deity
	c->GetLevel(), // in_level
	0, // in_npctype_id
	c->GetSize(), // in_size
	0, // in_runspeed
	c->GetPosition(), // position
	c->GetInnateLightType(), // in_light
	c->GetTexture(), // in_texture
	c->GetHelmTexture(), // in_helmtexture
	0, // in_ac
	0, // in_atk
	0, // in_str
	0, // in_sta
	0, // in_dex
	0, // in_agi
	0, // in_int
	0, // in_wis
	0, // in_cha
	c->GetPP().haircolor, // in_haircolor
	c->GetPP().beardcolor, // in_beardcolor
	c->GetPP().eyecolor1, // in_eyecolor1
	c->GetPP().eyecolor2, // in_eyecolor2
	c->GetPP().hairstyle, // in_hairstyle
	c->GetPP().face, // in_luclinface
	c->GetPP().beard, // in_beard
	c->GetPP().drakkin_heritage, // in_drakkin_heritage
	c->GetPP().drakkin_tattoo, // in_drakkin_tattoo
	c->GetPP().drakkin_details, // in_drakkin_details
	EQ::TintProfile(), // in_armor_tint
	0xff, // in_aa_title
	0, // in_see_invis
	0, // in_see_invis_undead
	0, // in_see_hide
	0, // in_see_improved_hide
	0, // in_hp_regen
	0, // in_mana_regen
	0, // in_qglobal
	0, // in_maxlevel
	0, // in_scalerate
	0, // in_armtexture
	0, // in_bracertexture
	0, // in_handtexture
	0, // in_legtexture
	0, // in_feettexture
	0, // in_usemodel
	false, // in_always_aggro
	0, // in_heroic_strikethrough
	false // in_keeps_sold_items
)
{
	PlayerProfile_Struct *pp   = &c->GetPP();
	EQ::ItemInstance     *item = nullptr;

	if (!zone->HasGraveyard()) {
		m_corpse_graveyard_timer.Disable();
	}

	for (int &allowed_looter: m_allowed_looters) {
		allowed_looter = 0;
	}

	if (c->AutoConsentGroupEnabled()) {
		auto *g = c->GetGroup();
		m_consented_group_id = g ? g->GetID() : 0;
	}

	if (c->AutoConsentRaidEnabled()) {
		auto *r = c->GetRaid();
		m_consented_raid_id = r ? r->GetID() : 0;
	}

	m_consented_guild_id = c->AutoConsentGuildEnabled() ? c->GuildID() : 0;

	m_is_corpse_changed         = true;
	m_rezzed_experience         = rez_exp;
	m_is_player_corpse          = true;
	m_is_locked                 = false;
	m_being_looted_by_entity_id = 0xFFFFFFFF;
	m_character_id              = c->CharacterID();
	m_corpse_db_id              = 0;
	m_player_corpse_depop       = false;
	m_copper                    = 0;
	m_silver                    = 0;
	m_gold                      = 0;
	m_platinum                  = 0;
	m_killed_by_type            = (uint8) in_killed_by;
	m_is_rezzable               = true;
	m_remaining_rez_time        = 0;
	m_is_owner_online           = false;
	m_account_id                = c->AccountID();

	// timers
	m_corpse_decay_timer.SetTimer(RuleI(Character, CorpseDecayTime));
	m_corpse_rezzable_timer.SetTimer(RuleI(Character, CorpseResTime));
	m_corpse_delay_timer.SetTimer(RuleI(NPC, CorpseUnlockTimer));
	m_corpse_graveyard_timer.SetTimer(RuleI(Zone, GraveyardTimeMS));
	m_loot_cooldown_timer.SetTimer(10);
	m_check_rezzable_timer.SetTimer(1000);
	m_check_owner_online_timer.SetTimer(RuleI(Character, CorpseOwnerOnlineCheckTime) * 1000);

	m_corpse_rezzable_timer.Disable();
	SetRezTimer(true);

	strcpy(corpse_name, pp->name);
	strcpy(name, pp->name);

	/* become_npc was not being initialized which led to some pretty funky things with newly created corpses */
	m_become_npc = false;

	SetPlayerKillItemID(0);

	/* Check Rule to see if we can leave corpses */
	if (
		(!RuleB(Character, LeaveNakedCorpses) || RuleB(Character, LeaveCorpses)) &&
		GetLevel() >= RuleI(Character, DeathItemLossLevel)
		) {
		// cash
		// Let's not move the cash when 'RespawnFromHover = true' && 'client->GetClientVersion() < EQClientSoF' since the client doesn't.
		// (change to first client that supports 'death hover' mode, if not SoF.)
		if (!RuleB(Character, RespawnFromHover) || c->ClientVersion() < EQ::versions::ClientVersion::SoF) {
			auto corpse_copper   = pp->copper;
			auto corpse_silver   = pp->silver;
			auto corpse_gold     = pp->gold;
			auto corpse_platinum = pp->platinum;

			pp->copper   = 0;
			pp->silver   = 0;
			pp->gold     = 0;
			pp->platinum = 0;

			if (RuleB(Character, LeaveCursorMoneyOnCorpse)) {
				corpse_copper += pp->copper_cursor;
				corpse_silver += pp->silver_cursor;
				corpse_gold += pp->gold_cursor;
				corpse_platinum += pp->platinum_cursor;

				pp->copper_cursor   = 0;
				pp->silver_cursor   = 0;
				pp->gold_cursor     = 0;
				pp->platinum_cursor = 0;
			}

			SetCash(corpse_copper, corpse_silver, corpse_gold, corpse_platinum);
		}

		// get their tints
		memcpy(&m_item_tint.Slot, &c->GetPP().item_tint, sizeof(m_item_tint));

		// TODO soulbound items need not be added to corpse, but they need
		// to go into the regular slots on the player, out of bags
		std::list<uint32> removed_list;

		// ideally, we would start at invslot::slotGeneral1 and progress to invslot::slotCursor..
		// ..then regress and process invslot::EQUIPMENT_BEGIN through invslot::EQUIPMENT_END...
		// without additional work to database loading of player corpses, this order is not
		// currently preserved and a re-work of this processing loop is not warranted.
		for (int i = EQ::invslot::POSSESSIONS_BEGIN; i <= EQ::invslot::POSSESSIONS_END; ++i) {
			item = c->GetInv().GetItem(i);
			if (!item) {
				continue;
			}

			if (!c->IsBecomeNPC() || (c->IsBecomeNPC() && !item->GetItem()->NoRent)) {
				MoveItemToCorpse(c, item, i, removed_list);
			}
		}

		database.TransactionBegin();

		// this should not be modified to include the entire range of invtype::TYPE_POSSESSIONS slots by default..
		// ..due to the possibility of 'hidden' items from client version bias..or, possibly, soul-bound items (WoW?)
		if (!removed_list.empty()) {
			auto iter = removed_list.begin();

			if (iter != removed_list.end()) {
				std::stringstream ss("");
				ss << "DELETE FROM `inventory` WHERE `character_id` = " << c->CharacterID();
				ss << " AND `slot_id` IN (" << (*iter);
				++iter;

				while (iter != removed_list.end()) {
					ss << ", " << (*iter);
					++iter;
				}
				ss << ")";

				database.QueryDatabase(ss.str().c_str());
			}
		}

		auto start  = c->GetInv().cursor_cbegin();
		auto finish = c->GetInv().cursor_cend();
		database.SaveCursor(c->CharacterID(), start, finish);

		c->CalcBonuses();
		c->Save();

		IsRezzed(false);
		Save();

		database.TransactionCommit();

		UpdateEquipmentLight();
		UpdateActiveLight();

		return;
	}

	UpdateEquipmentLight();
	UpdateActiveLight();

	m_loot_request_type = LootRequestType::Forbidden;

	IsRezzed(false);
	Save();
}

void Corpse::MoveItemToCorpse(Client *client, EQ::ItemInstance *inst, int16 equipSlot, std::list<uint32> &removedList)
{
	AddItem(
		inst->GetItem()->ID,
		inst->GetCharges(),
		equipSlot,
		inst->GetAugmentItemID(0),
		inst->GetAugmentItemID(1),
		inst->GetAugmentItemID(2),
		inst->GetAugmentItemID(3),
		inst->GetAugmentItemID(4),
		inst->GetAugmentItemID(5),
		inst->IsAttuned(),
		inst->GetCustomDataString(),
		inst->GetOrnamentationIcon(),
		inst->GetOrnamentationIDFile(),
		inst->GetOrnamentHeroModel()
	);

	removedList.push_back(equipSlot);

	while (true) {
		if (!inst->IsClassBag()) { break; }
		if (equipSlot < EQ::invslot::GENERAL_BEGIN || equipSlot > EQ::invslot::slotCursor) { break; }

		for (int16 sub_index = EQ::invbag::SLOT_BEGIN; sub_index <= EQ::invbag::SLOT_END; ++sub_index) {
			int16 real_bag_slot = EQ::InventoryProfile::CalcSlotId(equipSlot, sub_index);
			auto  bag_inst      = client->GetInv().GetItem(real_bag_slot);
			if (bag_inst == nullptr) { continue; }

			AddItem(
				bag_inst->GetItem()->ID,
				bag_inst->GetCharges(),
				real_bag_slot,
				bag_inst->GetAugmentItemID(0),
				bag_inst->GetAugmentItemID(1),
				bag_inst->GetAugmentItemID(2),
				bag_inst->GetAugmentItemID(3),
				bag_inst->GetAugmentItemID(4),
				bag_inst->GetAugmentItemID(5),
				bag_inst->IsAttuned(),
				bag_inst->GetCustomDataString(),
				bag_inst->GetOrnamentationIcon(),
				bag_inst->GetOrnamentationIDFile(),
				bag_inst->GetOrnamentHeroModel()
			);

			removedList.push_back(real_bag_slot);
			client->DeleteItemInInventory(real_bag_slot, 0, true, false);
		}
		break;
	}
	client->DeleteItemInInventory(equipSlot, 0, true, false);
}

// To be called from LoadFromDBData
Corpse::Corpse(
	uint32 corpse_id,
	uint32 character_id,
	const char *character_name,
	LootItems *item_list,
	uint32 copper,
	uint32 silver,
	uint32 gold,
	uint32 platinum,
	const glm::vec4 &position,
	float size,
	uint8 gender,
	uint16 race,
	uint8 class_,
	uint8 deity,
	uint8 level,
	uint8 texture,
	uint8 helm_texture,
	uint32 rez_exp,
	uint32 gm_rez_exp,
	KilledByTypes killed_by,
	bool is_rezzable,
	uint32 rez_remaining_time,
	bool was_at_graveyard
) : Mob(
	"Unnamed_Corpse", // in_name
	"", // in_lastname
	0, // in_cur_hp
	0, // in_max_hp
	gender, // in_gender
	race, // in_race
	class_, // in_class
	BodyType::Humanoid, // in_bodytype
	deity, // in_deity
	level, // in_level
	0, // in_npctype_id
	size, // in_size
	0.0f, // in_runspeed
	position, // position
	0, // in_light
	texture, // in_texture
	helm_texture, // in_helmtexture
	0, // in_ac
	0, // in_atk
	0, // in_str
	0, // in_sta
	0, // in_dex
	0, // in_agi
	0, // in_int
	0, // in_wis
	0, // in_cha
	0, // in_haircolor
	0, // in_beardcolor
	0, // in_eyecolor1
	0, // in_eyecolor2
	0, // in_hairstyle
	0, // in_luclinface
	0, // in_beard
	0, // in_drakkin_heritage
	0, // in_drakkin_tattoo
	0, // in_drakkin_details
	EQ::TintProfile(), // in_armor_tint
	0xFF, // in_aa_title
	0, // in_see_invis
	0, // in_see_invis_undead
	0, // in_see_hide
	0, // in_see_improved_hide
	0, // in_hp_regen
	0, // in_mana_regen
	0, // in_qglobal
	0, // in_maxlevel
	0, // in_scalerate
	0, // in_armtexture
	0, // in_bracertexture
	0, // in_handtexture
	0, // in_legtexture
	0, // in_feettexture
	0, // in_usemodel
	false, // in_always_aggros_foes
	0, // in_heroic_strikethrough
	false // in_keeps_sold_items
)
{
	LoadPlayerCorpseDecayTime(corpse_id);

	if (!zone->HasGraveyard() || was_at_graveyard) {
		m_corpse_graveyard_timer.Disable();
	}

	strcpy(corpse_name, character_name);
	strcpy(name, character_name);

	m_copper                    = copper;
	m_silver                    = silver;
	m_gold                      = gold;
	m_platinum                  = platinum;
	m_rezzed_experience         = rez_exp;
	m_gm_rezzed_experience      = gm_rez_exp;
	m_killed_by_type            = (uint8) killed_by;
	m_is_rezzable               = is_rezzable;
	m_remaining_rez_time        = rez_remaining_time;
	m_is_owner_online           = false;
	m_is_corpse_changed         = false;
	m_is_player_corpse          = true;
	m_is_locked                 = false;
	m_being_looted_by_entity_id = 0xFFFFFFFF;
	m_corpse_db_id              = corpse_id;
	m_player_corpse_depop       = false;
	m_character_id              = character_id;
	m_item_list                 = *item_list;
	item_list->clear();

	// timers
	m_corpse_decay_timer.SetTimer(RuleI(Character, CorpseDecayTime));
	m_corpse_rezzable_timer.SetTimer(RuleI(Character, CorpseResTime));
	m_corpse_delay_timer.SetTimer(RuleI(NPC, CorpseUnlockTimer));
	m_corpse_graveyard_timer.SetTimer(RuleI(Zone, GraveyardTimeMS));
	m_loot_cooldown_timer.SetTimer(10);
	m_check_owner_online_timer.SetTimer(RuleI(Character, CorpseOwnerOnlineCheckTime) * 1000);
	m_check_rezzable_timer.SetTimer(1000);
	m_corpse_rezzable_timer.Disable();

	SetRezTimer();

	for (int &allowed_looter: m_allowed_looters) {
		allowed_looter = 0;
	}

	SetPlayerKillItemID(0);
	UpdateEquipmentLight();
	UpdateActiveLight();

	m_loot_request_type = LootRequestType::Forbidden;
}

Corpse::~Corpse()
{
	if (m_is_player_corpse && !(m_player_corpse_depop && m_corpse_db_id == 0)) {
		Save();
	}
	LootItems::iterator cur, end;
	cur = m_item_list.begin();
	end = m_item_list.end();
	for (; cur != end; ++cur) {
		LootItem *item = *cur;
		safe_delete(item);
	}
	m_item_list.clear();
}

/*
this needs to be called AFTER the entity_id is set
the client does this too, so it's unchangable
*/
void Corpse::CalcCorpseName()
{
	EntityList::RemoveNumbers(name);
	char tmp[64];
	if (m_is_player_corpse) {
		snprintf(tmp, sizeof(tmp), "'s corpse%d", GetID());
	}
	else {
		snprintf(tmp, sizeof(tmp), "`s_corpse%d", GetID());
	}
	name[(sizeof(name) - 1) - strlen(tmp)] = 0;
	strcat(name, tmp);
}

bool Corpse::Save()
{
	if (!m_is_player_corpse) {
		return true;
	}

	if (!m_is_corpse_changed) {
		return true;
	}

	CharacterCorpseEntry ce;

	ce.size             = size;
	ce.locked           = m_is_locked;
	ce.copper           = m_copper;
	ce.silver           = m_silver;
	ce.gold             = m_gold;
	ce.plat             = m_platinum;
	ce.race             = race;
	ce.class_           = class_;
	ce.gender           = gender;
	ce.deity            = deity;
	ce.level            = level;
	ce.texture          = texture;
	ce.helmtexture      = helmtexture;
	ce.exp              = m_rezzed_experience;
	ce.gm_exp           = m_gm_exp;
	ce.killed_by        = m_killed_by_type;
	ce.rezzable         = m_is_rezzable;
	ce.rez_time         = m_remaining_rez_time;
	ce.item_tint        = m_item_tint;
	ce.haircolor        = haircolor;
	ce.beardcolor       = beardcolor;
	ce.eyecolor2        = eyecolor1;
	ce.hairstyle        = hairstyle;
	ce.face             = luclinface;
	ce.beard            = beard;
	ce.drakkin_heritage = drakkin_heritage;
	ce.drakkin_tattoo   = drakkin_tattoo;
	ce.drakkin_details  = drakkin_details;

	{
		json j;
		for (const auto& kv : m_EntityVariables) {
			j[kv.first] = kv.second;
		}

		if (!j.empty()) {
			ce.entity_variables = j.dump();
		} else {
			ce.entity_variables = "{}";
		}

		LogCorpses("Corpse entity_variables: %s", ce.entity_variables.c_str());
	}

	for (auto &item: m_item_list) {
		CharacterCorpseItemEntry e;

		e.item_id             = item->item_id;
		e.equip_slot          = item->equip_slot;
		e.charges             = item->charges;
		e.lootslot            = item->lootslot;
		e.aug_1               = item->aug_1;
		e.aug_2               = item->aug_2;
		e.aug_3               = item->aug_3;
		e.aug_4               = item->aug_4;
		e.aug_5               = item->aug_5;
		e.aug_6               = item->aug_6;
		e.attuned             = item->attuned;
		e.custom_data         = item->custom_data;
		e.ornamenticon        = item->ornamenticon;
		e.ornamentidfile      = item->ornamentidfile;
		e.ornament_hero_model = item->ornament_hero_model;

		ce.items.emplace_back(std::move(e));
	}

	if (m_corpse_db_id == 0) {
		m_corpse_db_id = database.SaveCharacterCorpse(
			m_character_id,
			corpse_name,
			zone->GetZoneID(),
			zone->GetInstanceID(),
			ce,
			m_Position,
			m_consented_guild_id
		);
	}
	else {
		m_corpse_db_id = database.UpdateCharacterCorpse(
			m_corpse_db_id,
			m_character_id,
			corpse_name,
			zone->GetZoneID(),
			zone->GetInstanceID(),
			ce,
			m_Position,
			m_consented_guild_id,
			IsRezzed()
		);
	}

	return true;
}

void Corpse::Delete()
{
	if (IsPlayerCorpse() && m_corpse_db_id != 0) {
		database.DeleteCharacterCorpse(m_corpse_db_id);
	}

	m_corpse_db_id        = 0;
	m_player_corpse_depop = true;
}

void Corpse::Bury()
{
	if (IsPlayerCorpse() && m_corpse_db_id != 0) {
		database.BuryCharacterCorpse(m_corpse_db_id);
	}

	m_corpse_db_id        = 0;
	m_player_corpse_depop = true;
}

void Corpse::DepopNPCCorpse()
{
	if (IsNPCCorpse()) {
		m_player_corpse_depop = true;
	}
}

void Corpse::DepopPlayerCorpse()
{
	m_player_corpse_depop = true;
}

void Corpse::AddConsentName(const std::string &consent_player_name)
{
	for (const auto &consented_player_name: m_consented_player_names) {
		if (strcasecmp(consented_player_name.c_str(), consent_player_name.c_str()) == 0) {
			return;
		}
	}

	m_consented_player_names.emplace_back(consent_player_name);
}

void Corpse::RemoveConsentName(const std::string &consent_player_name)
{
	m_consented_player_names.erase(
		std::remove_if(
			m_consented_player_names.begin(), m_consented_player_names.end(),
			[consent_player_name](const std::string &consented_player_name) {
				return strcasecmp(consented_player_name.c_str(), consent_player_name.c_str()) == 0;
			}
		), m_consented_player_names.end()
	);
}

uint32 Corpse::CountItems()
{
	return m_item_list.size();
}

void Corpse::AddItem(
	uint32 itemnum,
	uint16 charges,
	int16 slot,
	uint32 aug1,
	uint32 aug2,
	uint32 aug3,
	uint32 aug4,
	uint32 aug5,
	uint32 aug6,
	bool attuned,
	const std::string &custom_data,
	uint32 ornamenticon,
	uint32 ornamentidfile,
	uint32 ornament_hero_model
)
{

	if (!database.GetItem(itemnum)) {
		return;
	}

	m_is_corpse_changed = true;

	auto i = new LootItem;

	i->item_id             = itemnum;
	i->charges             = charges;
	i->equip_slot          = slot;
	i->aug_1               = aug1;
	i->aug_2               = aug2;
	i->aug_3               = aug3;
	i->aug_4               = aug4;
	i->aug_5               = aug5;
	i->aug_6               = aug6;
	i->attuned             = attuned;
	i->custom_data         = custom_data;
	i->ornamenticon        = ornamenticon;
	i->ornamentidfile      = ornamentidfile;
	i->ornament_hero_model = ornament_hero_model;

	m_item_list.push_back(i);

	UpdateEquipmentLight();
}

LootItem *Corpse::GetItem(uint16 lootslot, LootItem **bag_item_data)
{
	LootItem *sitem  = nullptr;
	LootItem *sitem2 = nullptr;

	for (const auto &item: m_item_list) {
		if (item->lootslot == lootslot) {
			sitem = item;
			break;
		}
	}

	if (sitem && bag_item_data && EQ::InventoryProfile::SupportsContainers(sitem->equip_slot)) {
		int16 bagstart = EQ::InventoryProfile::CalcSlotId(sitem->equip_slot, EQ::invbag::SLOT_BEGIN);

		// convert above code to for loop
		for (const auto &item: m_item_list) {
			if (item->equip_slot >= bagstart && item->equip_slot < bagstart + EQ::invbag::SLOT_COUNT) {
				bag_item_data[item->equip_slot - bagstart] = item;
			}
		}
	}

	return sitem;
}

uint32 Corpse::GetWornItem(int16 equip_slot) const
{
	for (const auto &item: m_item_list) {
		if (item->equip_slot == equip_slot) {
			return item->item_id;
		}
	}

	return 0;
}

void Corpse::RemoveItem(uint16 lootslot)
{
	if (lootslot == 0xFFFF) {
		return;
	}

	for (const auto &item: m_item_list) {
		if (item->lootslot == lootslot) {
			RemoveItem(item);
			return;
		}
	}
}

void Corpse::RemoveItem(LootItem *item_data)
{
	for (auto iter = m_item_list.begin(); iter != m_item_list.end(); ++iter) {
		auto sitem = *iter;
		if (sitem != item_data) { continue; }

		m_is_corpse_changed = true;
		m_item_list.erase(iter);

		uint8 material = EQ::InventoryProfile::CalcMaterialFromSlot(sitem->equip_slot); // autos to unsigned char
		if (material != EQ::textures::materialInvalid) {
			SendWearChange(material);
		}

		UpdateEquipmentLight();
		if (UpdateActiveLight()) {
			SendAppearancePacket(AppearanceType::Light, GetActiveLightType());
		}

		safe_delete(sitem);
		return;
	}
}

void Corpse::RemoveItemByID(uint32 item_id, int quantity)
{
	if (!database.GetItem(item_id)) {
		return;
	}

	if (!HasItem(item_id)) {
		return;
	}

	int       removed_count = 0;
	for (auto current_item  = m_item_list.begin(); current_item != m_item_list.end(); ++current_item) {
		LootItem *sitem = *current_item;
		if (removed_count == quantity) {
			break;
		}

		if (sitem && sitem->item_id == item_id) {
			int stack_size = sitem->charges > 1 ? sitem->charges : 1;
			if ((removed_count + stack_size) <= quantity) {
				removed_count += stack_size;
				m_is_corpse_changed = true;
				m_item_list.erase(current_item);
			}
			else {
				int amount_left = (quantity - removed_count);
				if (amount_left > 0) {
					if (stack_size > amount_left) {
						removed_count += amount_left;
						sitem->charges -= amount_left;
						m_is_corpse_changed = true;
					}
					else if (stack_size == amount_left) {
						removed_count += amount_left;
						m_item_list.erase(current_item);
					}
				}
			}
		}
	}
}

void Corpse::SetCash(uint32 in_copper, uint32 in_silver, uint32 in_gold, uint32 in_platinum)
{
	m_copper            = in_copper;
	m_silver            = in_silver;
	m_gold              = in_gold;
	m_platinum          = in_platinum;
	m_is_corpse_changed = true;
}

void Corpse::RemoveCash()
{
	m_copper            = 0;
	m_silver            = 0;
	m_gold              = 0;
	m_platinum          = 0;
	m_is_corpse_changed = true;
}

bool Corpse::IsEmpty() const
{
	if (m_copper != 0 || m_silver != 0 || m_gold != 0 || m_platinum != 0) {
		return false;
	}

	return m_item_list.empty();
}

bool Corpse::Process()
{
	if (m_player_corpse_depop) {
		return false;
	}

	if (m_check_owner_online_timer.Check() && m_is_rezzable) {
		CheckIsOwnerOnline();
	}

	if (m_corpse_delay_timer.Check()) {
		for (int &allowed_looter: m_allowed_looters) {
			allowed_looter = 0;
		}
		m_corpse_delay_timer.Disable();
		return true;
	}

	if (m_corpse_graveyard_timer.Check()) {
		if (MovePlayerCorpseToGraveyard()) {
			m_corpse_graveyard_timer.Disable();
			return false;
		}
		return true; // If the corpse was not moved, continue the corpse in the process rather than put in limbo
	}

	// Player is offline. If rez timer is enabled, disable it and save corpse.
	if (m_is_rezzable && m_check_rezzable_timer.Check()) {
		if (!m_is_owner_online) {
			if (m_corpse_rezzable_timer.Enabled()) {
				m_remaining_rez_time = m_corpse_rezzable_timer.GetRemainingTime();
				m_corpse_rezzable_timer.Disable();
				m_is_corpse_changed = true;
				Save();
			}
		}
		else { //Player is online. If rez timer is disabled, enable it.
			if (m_corpse_rezzable_timer.Enabled()) {
				m_remaining_rez_time = m_corpse_rezzable_timer.GetRemainingTime();
			}
			else {
				SetRezTimer();
			}
		}
	}

	if (m_corpse_rezzable_timer.Check()) {
		CompleteResurrection(true);
	}

	/* This is when a corpse hits decay timer and does checks*/
	if (m_corpse_decay_timer.Check()) {
		/* NPC */
		if (IsNPCCorpse()) {
			m_corpse_decay_timer.Disable();
			return false;
		}
		/* Client */
		if (!RuleB(Zone, EnableShadowrest)) {
			Delete();
		}
		else {
			if (database.BuryCharacterCorpse(m_corpse_db_id)) {
				Save();
				m_player_corpse_depop = true;
				m_corpse_db_id        = 0;
				LogCorpses("Tagged [{}] player corpse has buried", GetName());
			}
			else {
				LogError("Unable to bury [{}] player corpse", GetName());
				return true;
			}
		}
		m_corpse_decay_timer.Disable();
		return false;
	}

	return true;
}

void Corpse::ResetDecayTimer()
{
	int decay_ms = level > 54 ? RuleI(NPC, MajorNPCCorpseDecayTime) : RuleI(NPC, MinorNPCCorpseDecayTime);

	if (IsPlayerCorpse()) {
		decay_ms = RuleI(Character, CorpseDecayTime);
	}
	else if (IsEmpty()) {
		decay_ms = RuleI(NPC, EmptyNPCCorpseDecayTime) + 1000;
	}
	else {
		for (const npcDecayTimes_Struct &decay_time: npcCorpseDecayTimes) {
			if (level >= decay_time.minlvl && level <= decay_time.maxlvl) {
				decay_ms = decay_time.seconds * 1000;
				break;
			}
		}
	}

	m_corpse_decay_timer.SetTimer(decay_ms);
}

void Corpse::SetDecayTimer(uint32 decay_time)
{
	if (decay_time == 0) {
		m_corpse_decay_timer.Trigger();
	}
	else {
		m_corpse_decay_timer.Start(decay_time);
	}
}

bool Corpse::CanPlayerLoot(int character_id)
{
	uint8 looters = 0;
	for (int allowed_looter: m_allowed_looters) {
		if (allowed_looter != 0) {
			looters++;
		}

		if (allowed_looter == character_id) {
			return true;
		}
	}

	return looters == 0;
}

void Corpse::AllowPlayerLoot(Mob *them, uint8 slot)
{
	if (slot >= MAX_LOOTERS) {
		return;
	}
	if (them == nullptr || !them->IsClient()) {
		return;
	}

	m_allowed_looters[slot] = them->CastToClient()->CharacterID();
}

void Corpse::MakeLootRequestPackets(Client *c, const EQApplicationPacket *app)
{
	if (!c) {
		return;
	}

	// Added 12/08. Started compressing loot struct on live.
	if (m_player_corpse_depop) {
		SendLootReqErrorPacket(c, LootResponse::SomeoneElse);
		return;
	}

	if (IsPlayerCorpse() && !m_corpse_db_id) { // really should try to resave in this case
		// SendLootReqErrorPacket(client, 0);
		c->Message(Chat::Red, "Warning: Corpse's dbid = 0! Corpse will not survive zone shutdown!");
		std::cout << "Error: PlayerCorpse::MakeLootRequestPackets: dbid = 0!" << std::endl;
		// return;
	}

	if (m_is_locked && c->Admin() < AccountStatus::GMAdmin) {
		SendLootReqErrorPacket(c, LootResponse::SomeoneElse);
		c->Message(Chat::Red, "Error: Corpse locked by GM.");
		return;
	}

	if (!m_being_looted_by_entity_id ||
		(m_being_looted_by_entity_id != 0xFFFFFFFF && !entity_list.GetID(m_being_looted_by_entity_id))) {
		m_being_looted_by_entity_id = 0xFFFFFFFF;
	}

	if (DistanceSquaredNoZ(c->GetPosition(), m_Position) > 625) {
		SendLootReqErrorPacket(c, LootResponse::TooFar);
		return;
	}

	if (m_being_looted_by_entity_id != 0xFFFFFFFF && m_being_looted_by_entity_id != c->GetID()) {
		SendLootReqErrorPacket(c, LootResponse::SomeoneElse);
		return;
	}

	// all loot session disqualifiers should occur before this point as not to interfere with any current looter
	m_loot_request_type = LootRequestType::Forbidden;

	// loot_request_type is scoped to class Corpse and reset on a per-loot session basis
	if (c->GetGM()) {
		if (c->Admin() >= AccountStatus::GMAdmin) {
			m_loot_request_type = LootRequestType::GMAllowed;
			c->Message(Chat::White, "Your GM Status allows you to loot any items on this corpse.");
		} else {
			m_loot_request_type = LootRequestType::GMPeek;
			c->Message(Chat::White, "Your GM flag allows you to look at the items on this corpse.");
		}
	}
	else {
		if (IsPlayerCorpse()) {
			if (m_character_id == c->CharacterID()) {
				m_loot_request_type = LootRequestType::Self;
			}

			else if (CanPlayerLoot(c->CharacterID())) {
				if (GetPlayerKillItem() == -1) {
					m_loot_request_type = LootRequestType::AllowedPVPAll;

				}
				else if (GetPlayerKillItem() == 1) {
					m_loot_request_type = LootRequestType::AllowedPVPSingle;

				}
				else if (GetPlayerKillItem() > 1) {
					m_loot_request_type = LootRequestType::AllowedPVPDefined;
				}
			}
		}
		else if ((IsNPCCorpse() || m_become_npc) && CanPlayerLoot(c->CharacterID())) {
			m_loot_request_type = LootRequestType::AllowedPVE;
		}

	}

	LogInventory(
		"m_loot_request_type [{}] for [{}]",
		(int) m_loot_request_type,
		c->GetName()
	);

	if (m_loot_request_type == LootRequestType::Forbidden) {
		SendLootReqErrorPacket(c, LootResponse::NotAtThisTime);
		return;
	}

	m_being_looted_by_entity_id = c->GetID();
	c->CommonBreakInvisible(); // we should be "all good" so lets break invis now instead of earlier before all error checking is done

	// process coin
	bool        loot_coin = false;
	std::string tmp;
	if (database.GetVariable("LootCoin", tmp)) {
		loot_coin = (tmp[0] == 1 && tmp[1] == '\0');
	}

	if (m_loot_request_type == LootRequestType::GMPeek || m_loot_request_type == LootRequestType::GMAllowed) {
		if (GetPlatinum() || GetGold() || GetSilver() || GetCopper()) {
			c->Message(
				Chat::Yellow,
				fmt::format(
					"This corpse contains {}.",
					Strings::Money(
						GetPlatinum(),
						GetGold(),
						GetSilver(),
						GetCopper()
					)
				).c_str()
			);
		}
		else {
			c->Message(Chat::Yellow, "This corpse contains no money.");
		}

		auto outapp = new EQApplicationPacket(OP_MoneyOnCorpse, sizeof(moneyOnCorpseStruct));
		auto *d     = (moneyOnCorpseStruct *) outapp->pBuffer;

		d->response = static_cast<uint8>(LootResponse::Normal);
		d->unknown1 = 0x42;
		d->unknown2 = 0xef;

		d->copper   = 0;
		d->silver   = 0;
		d->gold     = 0;
		d->platinum = 0;

		outapp->priority = 6;
		c->QueuePacket(outapp);

		safe_delete(outapp);
	}
	else {
		auto outapp = new EQApplicationPacket(OP_MoneyOnCorpse, sizeof(moneyOnCorpseStruct));
		auto *d     = (moneyOnCorpseStruct *) outapp->pBuffer;

		d->response = static_cast<uint8>(LootResponse::Normal);
		d->unknown1 = 0x42;
		d->unknown2 = 0xef;

		Group *cgroup = c->GetGroup();

		// this can be reworked into a switch and/or massaged to include specialized pve loot rules based on 'LootRequestType'
		if (!IsPlayerCorpse() && c->IsGrouped() && c->AutoSplitEnabled() && cgroup) {
			d->copper   = 0;
			d->silver   = 0;
			d->gold     = 0;
			d->platinum = 0;
			cgroup->SplitMoney(GetCopper(), GetSilver(), GetGold(), GetPlatinum(), c);
		}
		else {
			d->copper   = GetCopper();
			d->silver   = GetSilver();
			d->gold     = GetGold();
			d->platinum = GetPlatinum();
			c->AddMoneyToPP(GetCopper(), GetSilver(), GetGold(), GetPlatinum());
		}

		RemoveCash();
		Save();

		outapp->priority = 6;
		c->QueuePacket(outapp);

		safe_delete(outapp);
	}

	// process items
	auto timestamps = database.GetItemRecastTimestamps(c->CharacterID());

	if (m_loot_request_type == LootRequestType::AllowedPVPDefined) {
		auto pkitemid = GetPlayerKillItem();
		auto pkitem   = database.GetItem(pkitemid);
		auto pkinst   = database.CreateItem(pkitem, pkitem->MaxCharges);

		if (pkinst) {
			if (pkitem->RecastDelay) {
				if (pkitem->RecastType != RECAST_TYPE_UNLINKED_ITEM) {
					pkinst->SetRecastTimestamp(
						timestamps.count(pkitem->RecastType) ? timestamps.at(pkitem->RecastType) : 0
					);
				}
				else {
					pkinst->SetRecastTimestamp(timestamps.count(pkitem->ID) ? timestamps.at(pkitem->ID) : 0);
				}
			}

			LogInventory("MakeLootRequestPackets() Slot [{}], Item [{}]", EQ::invslot::CORPSE_BEGIN, pkitem->Name);

			c->SendItemPacket(EQ::invslot::CORPSE_BEGIN, pkinst, ItemPacketLoot);
			safe_delete(pkinst);
		}
		else {
			LogInventory("MakeLootRequestPackets() PlayerKillItem [{}] not found", pkitemid);

			c->Message(Chat::Red, "PlayerKillItem (id: %i) could not be found!", pkitemid);
		}

		c->QueuePacket(app);
		return;
	}

	auto loot_slot   = EQ::invslot::CORPSE_BEGIN;
	auto corpse_mask = c->GetInv().GetLookup()->CorpseBitmask;

	for (auto i: m_item_list) {
		// every loot session must either set all items' lootslots to 'invslot::SLOT_INVALID'
		// or to a valid enumerated client-versioned corpse slot (lootslot is not equip_slot)
		i->lootslot = 0xFFFF;

		// align server and client corpse slot mappings so translators can function properly
		while (loot_slot <= EQ::invslot::CORPSE_END && (((uint64) 1 << loot_slot) & corpse_mask) == 0)
			++loot_slot;
		if (loot_slot > EQ::invslot::CORPSE_END) {
			continue;
		}

		if (IsPlayerCorpse()) {
			if (m_loot_request_type == LootRequestType::AllowedPVPSingle && loot_slot != EQ::invslot::CORPSE_BEGIN) {
				continue;
			}

			if (i->equip_slot < EQ::invslot::POSSESSIONS_BEGIN ||
				i->equip_slot > EQ::invslot::POSSESSIONS_END) {
				continue;
			}
		}

		const auto *item = database.GetItem(i->item_id);

		auto inst = database.CreateItem(
			item,
			i->charges,
			i->aug_1,
			i->aug_2,
			i->aug_3,
			i->aug_4,
			i->aug_5,
			i->aug_6,
			i->attuned,
			i->custom_data,
			i->ornamenticon,
			i->ornamentidfile,
			i->ornament_hero_model
		);
		if (!inst) {
			continue;
		}

		if (item->RecastDelay) {
			if (item->RecastType != RECAST_TYPE_UNLINKED_ITEM) {
				inst->SetRecastTimestamp(timestamps.count(item->RecastType) ? timestamps.at(item->RecastType) : 0);
			}
			else {
				inst->SetRecastTimestamp(timestamps.count(item->ID) ? timestamps.at(item->ID) : 0);
			}
		}

		LogInventory("MakeLootRequestPackets() Slot [{}], Item [{}]", loot_slot, item->Name);

		c->SendItemPacket(loot_slot, inst, ItemPacketLoot);
		safe_delete(inst);

		i->lootslot = loot_slot++;
	}

	// Disgrace: Client seems to require that we send the packet back...
	c->QueuePacket(app);

	// This is required for the 'Loot All' feature to work for SoD clients. I expect it is to tell the client that the
	// server has now sent all the items on the corpse.
	if (c->ClientVersion() >= EQ::versions::ClientVersion::SoD) {
		SendLootReqErrorPacket(c, LootResponse::LootAll);
	}
}

void Corpse::LootCorpseItem(Client *c, const EQApplicationPacket *app)
{
	if (!c) {
		return;
	}

	auto lootitem = (LootingItem_Struct *) app->pBuffer;

	LogInventory(
		"m_loot_request_type [{}] slot_id [{}] for [{}]",
		(int) m_loot_request_type,
		lootitem->slot_id,
		c->GetName()
	);

	if (m_loot_request_type < LootRequestType::GMAllowed) { // LootRequestType::Forbidden and LootRequestType::GMPeek
		c->QueuePacket(app);
		SendEndLootErrorPacket(c);
		// unlock corpse for others

		if (IsBeingLootedBy(c)) {
			ResetLooter();
		}
		return;
	}

	if (!m_loot_cooldown_timer.Check()) {
		c->QueuePacket(app);
		SendEndLootErrorPacket(c);
		// unlock corpse for others
		if (IsBeingLootedBy(c)) {
			ResetLooter();
		}
		return;
	}

	/* To prevent item loss for a player using 'Loot All' who doesn't have inventory space for all their items. */
	if (RuleB(Character, CheckCursorEmptyWhenLooting) && !c->GetInv().CursorEmpty()) {
		c->Message(Chat::Red, "You may not loot an item while you have an item on your cursor.");
		c->QueuePacket(app);
		SendEndLootErrorPacket(c);
		/* Unlock corpse for others */
		if (IsBeingLootedBy(c)) {
			ResetLooter();
		}
		return;
	}

	if (!IsBeingLootedBy(c)) {
		c->QueuePacket(app);
		SendEndLootErrorPacket(c);
		return;
	}

	if (IsPlayerCorpse() && !CanPlayerLoot(c->CharacterID()) && !m_become_npc &&
		(m_character_id != c->CharacterID() && c->Admin() < AccountStatus::GMLeadAdmin)) {
		c->Message(Chat::Red, "Error: This is a player corpse and you dont own it.");
		c->QueuePacket(app);
		SendEndLootErrorPacket(c);
		return;
	}

	if (m_is_locked && c->Admin() < AccountStatus::GMAdmin) {
		c->QueuePacket(app);
		SendLootReqErrorPacket(c, LootResponse::SomeoneElse);
		c->Message(Chat::Red, "Error: Corpse locked by GM.");
		return;
	}

	if (IsPlayerCorpse() && (m_character_id != c->CharacterID()) && CanPlayerLoot(c->CharacterID()) &&
		GetPlayerKillItem() == 0) {
		c->Message(Chat::Red, "Error: You cannot loot any more items from this corpse.");
		c->QueuePacket(app);
		SendEndLootErrorPacket(c);
		ResetLooter();
		return;
	}

	const EQ::ItemData *item      = nullptr;
	EQ::ItemInstance   *inst      = nullptr;
	LootItem           *item_data = nullptr, *bag_item_data[EQ::invbag::SLOT_COUNT] = {};

	memset(bag_item_data, 0, sizeof(bag_item_data));
	if (GetPlayerKillItem() > 1) {
		item = database.GetItem(GetPlayerKillItem());
	}
	else if (GetPlayerKillItem() == -1 || GetPlayerKillItem() == 1) {
		item_data = GetItem(lootitem->slot_id); // dont allow them to loot entire bags of items as pvp reward
	}
	else {
		item_data = GetItem(lootitem->slot_id, bag_item_data);
	}

	if (GetPlayerKillItem() <= 1 && item_data != 0) {
		item = database.GetItem(item_data->item_id);
	}

	if (item) {
		if (item_data) {
			inst = database.CreateItem(
				item, item_data ? item_data->charges : 0, item_data->aug_1,
				item_data->aug_2, item_data->aug_3, item_data->aug_4,
				item_data->aug_5, item_data->aug_6, item_data->attuned,
				item_data->custom_data, item_data->ornamenticon,
				item_data->ornamentidfile, item_data->ornament_hero_model
			);
		}
		else {
			inst = database.CreateItem(item);
		}
	}

	if (c && inst) {
		if (c->CheckLoreConflict(item)) {
			c->MessageString(Chat::White, LOOT_LORE_ERROR);
			c->QueuePacket(app);
			SendEndLootErrorPacket(c);
			ResetLooter();
			delete inst;
			return;
		}

		if (inst->IsAugmented()) {
			for (int i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++) {
				EQ::ItemInstance *itm = inst->GetAugment(i);
				if (itm) {
					if (c->CheckLoreConflict(itm->GetItem())) {
						c->MessageString(Chat::White, LOOT_LORE_ERROR);
						c->QueuePacket(app);
						SendEndLootErrorPacket(c);
						ResetLooter();
						delete inst;
						return;
					}
				}
			}
		}

		auto prevent_loot = false;

		if (RuleB(Zone, UseZoneController)) {
			auto controller = entity_list.GetNPCByNPCTypeID(ZONE_CONTROLLER_NPC_ID);
			if (controller) {
				if (parse->HasQuestSub(ZONE_CONTROLLER_NPC_ID, EVENT_LOOT_ZONE)) {
					const auto &export_string = fmt::format(
						"{} {} {} {}",
						inst->GetItem()->ID,
						inst->GetCharges(),
						EntityList::RemoveNumbers(corpse_name),
						GetID()
					);

					std::vector<std::any> args = {inst, this};
					if (parse->EventNPC(EVENT_LOOT_ZONE, controller, c, export_string, 0, &args) != 0) {
						prevent_loot = true;
					}
				}
			}
		}

		if (parse->PlayerHasQuestSub(EVENT_LOOT)) {
			const auto &export_string = fmt::format(
				"{} {} {} {}",
				inst->GetItem()->ID,
				inst->GetCharges(),
				EntityList::RemoveNumbers(corpse_name),
				GetID()
			);

			std::vector<std::any> args = {inst, this};
			if (parse->EventPlayer(EVENT_LOOT, c, export_string, 0, &args) != 0) {
				prevent_loot = true;
			}
		}

		if (parse->ZoneHasQuestSub(EVENT_LOOT_ZONE)) {
			const auto &export_string = fmt::format(
				"{} {} {} {}",
				inst->GetItem()->ID,
				inst->GetCharges(),
				EntityList::RemoveNumbers(corpse_name),
				GetID()
			);

			std::vector<std::any> args = {inst, this, c};
			if (parse->EventZone(EVENT_LOOT_ZONE, zone, export_string, 0, &args) != 0) {
				prevent_loot = true;
			}
		}

		if (inst && PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::LOOT_ITEM) && !IsPlayerCorpse()) {
			auto e = PlayerEvent::LootItemEvent{
				.item_id      = inst->GetItem()->ID,
				.item_name    = inst->GetItem()->Name,
				.charges      = inst->GetCharges(),
				.augment_1_id = inst->GetAugmentItemID(0),
				.augment_2_id = inst->GetAugmentItemID(1),
				.augment_3_id = inst->GetAugmentItemID(2),
				.augment_4_id = inst->GetAugmentItemID(3),
				.augment_5_id = inst->GetAugmentItemID(4),
				.augment_6_id = inst->GetAugmentItemID(5),
				.npc_id       = GetNPCTypeID(),
				.corpse_name  = EntityList::RemoveNumbers(corpse_name)
			};

			RecordPlayerEventLogWithClient(c, PlayerEvent::LOOT_ITEM, e);
		}

		if (!IsPlayerCorpse()) {
			// dynamic zones may prevent looting by non-members or based on lockouts
			auto dz = zone->GetDynamicZone();
			if (dz && !dz->CanClientLootCorpse(c, GetNPCTypeID(), GetID())) {
				prevent_loot = true;
				// note on live this message is only sent once on the first loot attempt of an open corpse
				c->MessageString(Chat::Loot, LOOT_NOT_ALLOWED, inst->GetItem()->Name);
			}
		}

		if (parse->ItemHasQuestSub(inst, EVENT_LOOT)) {
			const auto &export_string = fmt::format(
				"{} {} {} {}",
				inst->GetItem()->ID,
				inst->GetCharges(),
				EntityList::RemoveNumbers(corpse_name),
				GetID()
			);

			std::vector<std::any> args = {inst, this};
			if (parse->EventItem(EVENT_LOOT, c, inst, this, export_string, 0, &args) != 0) {
				prevent_loot = true;
			}
		}

		if (prevent_loot) {
			lootitem->auto_loot = -1;
			c->QueuePacket(app);
			safe_delete(inst);
			return;
		}


		// safe to ACK now
		c->QueuePacket(app);

		if (!IsPlayerCorpse() && c) {
			c->CheckItemDiscoverability(inst->GetID());
		}

		if (zone->adv_data) {
			auto *ad = (ServerZoneAdventureDataReply_Struct *) zone->adv_data;
			if (ad->type == Adventure_Collect && !IsPlayerCorpse()) {
				if (ad->data_id == inst->GetItem()->ID) {
					zone->DoAdventureCountIncrease();
				}
			}
		}

		// get count for task update before it's mutated by AutoPutLootInInventory
		int        count      = inst->IsStackable() ? inst->GetCharges() : 1;
		//Set recast on item when looting it!
		auto       timestamps = database.GetItemRecastTimestamps(c->CharacterID());
		const auto *d         = inst->GetItem();
		if (d->RecastDelay) {
			if (d->RecastType != RECAST_TYPE_UNLINKED_ITEM) {
				inst->SetRecastTimestamp(timestamps.count(d->RecastType) ? timestamps.at(d->RecastType) : 0);
			}
			else {
				inst->SetRecastTimestamp(timestamps.count(d->ID) ? timestamps.at(d->ID) : 0);
			}
		}

		/* First add it to the looter - this will do the bag contents too */
		if (lootitem->auto_loot > 0) {
			if (!c->AutoPutLootInInventory(*inst, true, true, bag_item_data)) {
				c->PutLootInInventory(EQ::invslot::slotCursor, *inst, bag_item_data);
			}
		}
		else {
			c->PutLootInInventory(EQ::invslot::slotCursor, *inst, bag_item_data);
		}

		/* Update any tasks that have an activity to loot this item */
		if (RuleB(TaskSystem, EnableTaskSystem) && IsNPCCorpse()) {
			c->UpdateTasksOnLoot(this, item->ID, count);
		}

		/* Remove it from Corpse */
		if (item_data) {
			/* Delete needs to be before RemoveItem because its deletes the pointer for
			* item_data/bag_item_data */
			database.DeleteItemOffCharacterCorpse(
				m_corpse_db_id, item_data->equip_slot,
				item_data->item_id
			);
			/* Delete Item Instance */
			RemoveItem(item_data->lootslot);
		}

		/* Remove Bag Contents */
		if (item->IsClassBag() && (GetPlayerKillItem() != -1 || GetPlayerKillItem() != 1)) {
			for (int i = EQ::invbag::SLOT_BEGIN; i <= EQ::invbag::SLOT_END; i++) {
				if (bag_item_data[i]) {
					/* Delete needs to be before RemoveItem because its deletes the pointer for
					* item_data/bag_item_data */
					database.DeleteItemOffCharacterCorpse(
						m_corpse_db_id,
						bag_item_data[i]->equip_slot,
						bag_item_data[i]->item_id
					);
					/* Delete Item Instance */
					RemoveItem(bag_item_data[i]);
				}
			}
		}

		if (GetPlayerKillItem() != -1) {
			SetPlayerKillItemID(0);
		}

		/* Send message with item link to groups and such */
		EQ::SayLinkEngine linker;
		linker.SetLinkType(EQ::saylink::SayLinkItemInst);
		linker.SetItemInst(inst);

		linker.GenerateLink();

		c->MessageString(Chat::Loot, LOOTED_MESSAGE, linker.Link().c_str());

		if (!IsPlayerCorpse()) {
			Group *g = c->GetGroup();
			if (g != nullptr) {
				g->GroupMessageString(
					c, Chat::Loot,
					OTHER_LOOTED_MESSAGE,
					c->GetName(),
					linker.Link().c_str()
				);
			}
			else {
				Raid *r = c->GetRaid();
				if (r != nullptr) {
					r->RaidMessageString(
						c,
						Chat::Loot,
						OTHER_LOOTED_MESSAGE,
						c->GetName(),
						linker.Link().c_str()
					);
				}
			}
		}
	}
	else {
		SendEndLootErrorPacket(c);
		safe_delete(inst);
		return;
	}

	if (IsPlayerCorpse()) {
		c->SendItemLink(inst);
	}
	else {
		c->SendItemLink(inst, true);
	}

	safe_delete(inst);
}

void Corpse::EndLoot(Client *c, const EQApplicationPacket *app)
{
	auto outapp = new EQApplicationPacket;
	outapp->SetOpcode(OP_LootComplete);
	outapp->size = 0;
	c->QueuePacket(outapp);
	safe_delete(outapp);

	m_being_looted_by_entity_id = 0xFFFFFFFF;
	if (IsEmpty()) {
		Delete();
	}
	else {
		Save();
	}
}

void Corpse::FillSpawnStruct(NewSpawn_Struct *ns, Mob *ForWho)
{
	Mob::FillSpawnStruct(ns, ForWho);

	ns->spawn.max_hp = 120;
	ns->spawn.NPC    = 2;

	UpdateActiveLight();
	ns->spawn.light = m_Light.Type[EQ::lightsource::LightActive];
}

void Corpse::QueryLoot(Client *to)
{
	if (!m_item_list.empty()) {
		int player_corpse_limit = to->GetInv().GetLookup()->InventoryTypeSize.Corpse;
		to->Message(
			Chat::White,
			fmt::format(
				"Loot | Name: {} ID: {}",
				GetName(),
				GetNPCTypeID()
			).c_str()
		);

		int       item_count = 0;
		for (auto current_item: m_item_list) {
			int item_number = (item_count + 1);
			if (!current_item) {
				LogError("Corpse::QueryLoot() - ItemList error, null item.");
				continue;
			}

			if (!current_item->item_id || !database.GetItem(current_item->item_id)) {
				LogError("Corpse::QueryLoot() - Database error, invalid item.");
				continue;
			}

			EQ::SayLinkEngine linker;
			linker.SetLinkType(EQ::saylink::SayLinkLootItem);
			linker.SetLootData(current_item);

			to->Message(
				Chat::White,
				fmt::format(
					"Item {} | Name: {} ({}){}",
					item_number,
					linker.GenerateLink().c_str(),
					current_item->item_id,
					(
						current_item->charges > 1 ?
							fmt::format(
								" Amount: {}",
								current_item->charges
							) :
							""
					)
				).c_str()
			);
			item_count++;
		}
	}

	if (m_platinum || m_gold || m_silver || m_copper) {
		to->Message(
			Chat::White,
			fmt::format(
				"Money | {}",
				Strings::Money(
					m_platinum,
					m_gold,
					m_silver,
					m_copper
				)
			).c_str()
		);
	}
}

bool Corpse::HasItem(uint32 item_id)
{
	if (!database.GetItem(item_id)) {
		return false;
	}

	for (auto i: m_item_list) {
		if (!i) {
			LogError("Corpse::HasItem() - ItemList error, null item");
			continue;
		}

		if (!i->item_id || !database.GetItem(i->item_id)) {
			LogError("Corpse::HasItem() - Database error, invalid item");
			continue;
		}

		if (i->item_id == item_id) {
			return true;
		}
	}
	return false;
}

uint32 Corpse::CountItem(uint32 item_id)
{
	uint32 item_count = 0;

	if (!database.GetItem(item_id)) {
		return item_count;
	}

	for (auto i: m_item_list) {
		if (!i) {
			LogError("Corpse::CountItem() - ItemList error, null item");
			continue;
		}

		if (!i->item_id || !database.GetItem(i->item_id)) {
			LogError("Corpse::CountItem() - Database error, invalid item");
			continue;
		}

		if (i->item_id == item_id) {
			item_count += i->charges > 0 ? i->charges : 1;
		}
	}

	return item_count;
}

uint32 Corpse::GetItemIDBySlot(uint16 loot_slot)
{
	for (auto i: m_item_list) {
		if (i->lootslot == loot_slot) {
			return i->item_id;
		}
	}

	return 0;
}

uint16 Corpse::GetFirstLootSlotByItemID(uint32 item_id)
{
	for (auto i: m_item_list) {
		if (i->item_id == item_id) {
			return i->lootslot;
		}
	}

	return 0;
}

bool Corpse::Summon(Client *c, bool spell, bool CheckDistance)
{
	uint32 dist2 = 10000; // pow(100, 2);
	if (!spell) {
		if (GetCharID() == c->CharacterID()) {
			if (IsLocked() && c->Admin() < AccountStatus::GMAdmin) {
				c->Message(Chat::Red, "That corpse is locked by a GM.");
				return false;
			}
			if (!CheckDistance || (DistanceSquaredNoZ(m_Position, c->GetPosition()) <= dist2)) {
				GMMove(c->GetX(), c->GetY(), c->GetZ());
				m_is_corpse_changed = true;
			}
			else {
				c->MessageString(Chat::Red, CORPSE_TOO_FAR);
				return false;
			}
		}
		else {
			bool            consented = false;
			for (const auto &consented_player_name: m_consented_player_names) {
				if (strcasecmp(c->GetName(), consented_player_name.c_str()) == 0) {
					consented = true;
					break;
				}
			}

			if (!consented && m_consented_guild_id && m_consented_guild_id != GUILD_NONE) {
				if (c->GuildID() == m_consented_guild_id) {
					consented = true;
				}
			}
			if (!consented && m_consented_group_id) {
				Group *grp = c->GetGroup();
				if (grp && grp->GetID() == m_consented_group_id) {
					consented = true;
				}
			}
			if (!consented && m_consented_raid_id) {
				Raid *raid = c->GetRaid();
				if (raid && raid->GetID() == m_consented_raid_id) {
					consented = true;
				}
			}

			if (consented) {
				if (!CheckDistance || (DistanceSquaredNoZ(m_Position, c->GetPosition()) <= dist2)) {
					GMMove(c->GetX(), c->GetY(), c->GetZ());
					m_is_corpse_changed = true;
				}
				else {
					c->MessageString(Chat::Red, CORPSE_TOO_FAR);
					return false;
				}
			}
			else {
				c->MessageString(Chat::Red, CONSENT_DENIED);
				return false;
			}
		}
	}
	else {
		GMMove(c->GetX(), c->GetY(), c->GetZ());
		m_is_corpse_changed = true;
	}
	Save();
	return true;
}

void Corpse::CompleteResurrection(bool timer_expired)
{
	LogCorpses("Corpse [{}] has been rezzed.", GetName());

	m_remaining_rez_time = m_corpse_rezzable_timer.GetRemainingTime();

	if (timer_expired) {
		m_remaining_rez_time = 0;
		m_is_rezzable        = false; // Players can no longer rez this corpse.
		m_corpse_rezzable_timer.Disable();
	}

	IsRezzed(true); // Players can rez this corpse for no XP (corpse gate) provided rezzable is true.
	m_rezzed_experience = 0;
	m_is_corpse_changed = true;
	Save();
}

void Corpse::Spawn()
{
	auto app = new EQApplicationPacket;
	CreateSpawnPacket(app, this);
	entity_list.QueueClients(this, app);
	safe_delete(app);
}

uint32 Corpse::GetEquippedItemFromTextureSlot(uint8 material_slot) const
{
	int16 invslot;

	if (material_slot > EQ::textures::LastTexture) {
		return 0;
	}

	invslot = EQ::InventoryProfile::CalcSlotFromMaterial(material_slot);
	if (invslot == INVALID_INDEX) { // GetWornItem() should be returning a 0 for any invalid index...
		return 0;
	}

	return GetWornItem(invslot);
}

uint32 Corpse::GetEquipmentColor(uint8 material_slot) const
{
	const EQ::ItemData *item = nullptr;

	if (material_slot > EQ::textures::LastTexture) {
		return 0;
	}

	item = database.GetItem(GetEquippedItemFromTextureSlot(material_slot));
	if (item) {
		return (m_item_tint.Slot[material_slot].UseTint ? m_item_tint.Slot[material_slot].Color : item->Color);
	}

	return 0;
}

void Corpse::UpdateEquipmentLight()
{
	m_Light.Type[EQ::lightsource::LightEquipment]  = 0;
	m_Light.Level[EQ::lightsource::LightEquipment] = 0;

	for (auto &i: m_item_list) {
		if (i->equip_slot < EQ::invslot::EQUIPMENT_BEGIN || i->equip_slot > EQ::invslot::EQUIPMENT_END) {
			continue;
		}
		if (i->equip_slot == EQ::invslot::slotAmmo) {
			continue;
		}

		auto item = database.GetItem(i->item_id);
		if (item == nullptr) {
			continue;
		}

		if (EQ::lightsource::IsLevelGreater(item->Light, m_Light.Type[EQ::lightsource::LightEquipment])) {
			m_Light.Type[EQ::lightsource::LightEquipment] = item->Light;
		}
	}

	uint8 general_light_type = 0;

	for (auto &i: m_item_list) {
		if (i->equip_slot < EQ::invslot::GENERAL_BEGIN || i->equip_slot > EQ::invslot::GENERAL_END) {
			continue;
		}

		auto item = database.GetItem(i->item_id);
		if (item == nullptr) { continue; }

		if (!item->IsClassCommon()) { continue; }
		if (item->Light < 9 || item->Light > 13) { continue; }

		if (EQ::lightsource::TypeToLevel(item->Light)) {
			general_light_type = item->Light;
		}
	}

	if (EQ::lightsource::IsLevelGreater(general_light_type, m_Light.Type[EQ::lightsource::LightEquipment])) {
		m_Light.Type[EQ::lightsource::LightEquipment] = general_light_type;
	}

	m_Light.Level[EQ::lightsource::LightEquipment] = EQ::lightsource::TypeToLevel(m_Light.Type[EQ::lightsource::LightEquipment]);
}

void Corpse::AddLooter(Mob *who)
{
	for (int &looter: m_allowed_looters) {
		if (looter == 0) {
			looter = who->CastToClient()->CharacterID();
			break;
		}
	}
}

void Corpse::LoadPlayerCorpseDecayTime(uint32 corpse_db_id)
{
	if (!corpse_db_id) {
		return;
	}

	uint32 active_corpse_decay_timer = database.GetCharacterCorpseDecayTimer(corpse_db_id);
	if (active_corpse_decay_timer > 0 && RuleI(Character, CorpseDecayTime) > (active_corpse_decay_timer * 1000)) {
		m_corpse_decay_timer.SetTimer(RuleI(Character, CorpseDecayTime) - (active_corpse_decay_timer * 1000));
	}
	else {
		m_corpse_decay_timer.SetTimer(2000);
	}
	if (active_corpse_decay_timer > 0 && RuleI(Zone, GraveyardTimeMS) > (active_corpse_decay_timer * 1000)) {
		m_corpse_graveyard_timer.SetTimer(RuleI(Zone, GraveyardTimeMS) - (active_corpse_decay_timer * 1000));
	}
	else {
		m_corpse_graveyard_timer.SetTimer(3000);
	}
}

void Corpse::SendWorldSpawnPlayerCorpseInZone(uint32_t zone_id)
{
	auto pack = std::make_unique<ServerPacket>(
		ServerOP_SpawnPlayerCorpse,
		sizeof(SpawnPlayerCorpse_Struct)
	);
	auto *spc = reinterpret_cast<SpawnPlayerCorpse_Struct *>(pack->pBuffer);
	spc->player_corpse_id = m_corpse_db_id;
	spc->zone_id          = zone_id;
	worldserver.SendPacket(pack.get());
}

bool Corpse::MovePlayerCorpseToGraveyard()
{
	if (IsPlayerCorpse() && zone && zone->HasGraveyard()) {
		Save();

		glm::vec4 graveyard_point = zone->GetGraveyardPoint();

		uint16_t instance_id = (zone->GetZoneID() == zone->graveyard_zoneid()) ? zone->GetInstanceID() : 0;
		database.SendCharacterCorpseToGraveyard(m_corpse_db_id, zone->graveyard_zoneid(), instance_id, graveyard_point);
		SendWorldSpawnPlayerCorpseInZone(zone->graveyard_zoneid());

		m_corpse_db_id        = 0;
		m_player_corpse_depop = true;
		m_corpse_graveyard_timer.Disable();

		LogCorpses(
			"Moved [{}] player corpse to the designated graveyard in zone [{}]",
			GetName(),
			ZoneName(zone->graveyard_zoneid())
		);

		return true;
	}

	return false;
}

bool Corpse::MovePlayerCorpseToNonInstance()
{
	if (IsPlayerCorpse() && zone && zone->GetInstanceID() != 0) {
		Save();

		database.SendCharacterCorpseToNonInstance(m_corpse_db_id);
		SendWorldSpawnPlayerCorpseInZone(zone->GetZoneID());

		m_corpse_db_id        = 0;
		m_player_corpse_depop = true;
		m_corpse_graveyard_timer.Disable();

		LogCorpses(
			"Moved [{}] player corpse to non-instance version of zone [{}]",
			GetName(),
			ZoneName(zone->GetZoneID())
		);

		return true;
	}

	return false;
}

std::vector<int> Corpse::GetLootList()
{
	std::vector<int> corpse_items;

	for (auto i: m_item_list) {
		if (!i) {
			LogError("Corpse::GetLootList() - ItemList error, null item");
			continue;
		}

		if (std::find(corpse_items.begin(), corpse_items.end(), i->item_id) != corpse_items.end()) {
			continue;
		}

		corpse_items.push_back(i->item_id);
	}
	return corpse_items;
}

void Corpse::SetRezTimer(bool initial_timer)
{
	LogCorpsesDetail("Checking for rezzable corpse [{}]", GetName());

	if (!m_is_rezzable) {
		if (m_corpse_rezzable_timer.Enabled()) {
			m_corpse_rezzable_timer.Disable();
		}
		return;
	}

	CheckIsOwnerOnline();

	if (!m_is_owner_online && !initial_timer) {
		if (m_corpse_rezzable_timer.Enabled()) {
			m_corpse_rezzable_timer.Disable();
		}
		return;
	}

	if (m_corpse_rezzable_timer.Enabled() && !initial_timer) {
		return;
	}

	if (initial_timer) {
		uint32 timer         = RuleI(Character, CorpseResTime);
		if (static_cast<KilledByTypes>(m_killed_by_type) == KilledByTypes::Killed_DUEL) {
			timer = RuleI(Character, DuelCorpseResTime);
		}
		m_remaining_rez_time = timer;
	}

	if (m_remaining_rez_time < 1) {
		// Corpse is no longer rezzable
		CompleteResurrection(true);
		return;
	}

	m_corpse_rezzable_timer.SetTimer(m_remaining_rez_time);
}

void Corpse::CheckIsOwnerOnline()
{
	LogCorpsesDetail("Checking if owner is online for corpse [{}]", GetOwnerName());

	Client *c = entity_list.GetClientByCharID(GetCharID());
	if (!c) {
		// Client is not in the corpse's zone, send a packet to world to have it check.
		auto pack = new ServerPacket(ServerOP_IsOwnerOnline, sizeof(ServerIsOwnerOnline_Struct));
		auto *o   = (ServerIsOwnerOnline_Struct *) pack->pBuffer;
		strncpy(o->name, GetOwnerName(), sizeof(o->name));
		o->corpse_id  = GetID();
		o->zone_id    = zone->GetZoneID();
		o->online     = 0;
		o->account_id = m_account_id;
		worldserver.SendPacket(pack);
		safe_delete(pack);
		LogCorpsesDetail("Sent IsOwnerOnline packet to world for [{}]", GetName());
	}
	else {
		SetOwnerOnline(true);
	}
}

void Corpse::CastRezz(uint16 spell_id, Mob *caster)
{
	LogSpells(
		"spell_id [{}] IsRezzed() [{}], rez_experience [{}], rez_timer enabled [{}]",
		spell_id,
		IsRezzed(),
		m_rezzed_experience,
		m_corpse_rezzable_timer.Enabled()
	);

	// refresh rezzed state from database
	const auto &e = CharacterCorpsesRepository::FindOne(database, m_corpse_db_id);
	if (!e.id) {
		caster->MessageString(Chat::White, REZZ_ALREADY_PENDING);
		return;
	}

	m_rezzed_experience    = e.exp;
	m_gm_rezzed_experience = e.gm_exp;
	m_is_rezzable          = e.rezzable;
	IsRezzed(e.is_rezzed);

	// Rez timer has expired, only GMs can rez at this point. (uses rezzable)
	if (!IsRezzable()) {
		if (caster && caster->IsClient()) {
			if (!caster->CastToClient()->GetGM()) {
				caster->MessageString(Chat::White, REZZ_ALREADY_PENDING);
				caster->MessageString(Chat::White, CORPSE_TOO_OLD);
				return;
			}

			caster->Message(Chat::White, "Your GM flag allows you to resurrect this corpse.");
		}
	}

	// Corpse has been rezzed, but timer is still active. Players can corpse gate, GMs can rez for XP. (uses is_rezzed)
	if (IsRezzed()) {
		auto c = caster && caster->IsClient() ? caster->CastToClient() : nullptr;
		if (c) {
			m_rezzed_experience = 0;
			if (c->GetGM()) {
				m_rezzed_experience    = m_gm_rezzed_experience;
				m_gm_rezzed_experience = 0;

				c->Message(Chat::White, "Your GM flag allows you to resurrect this corpse and return experience.");
			}
		}
	}

	auto outapp = new EQApplicationPacket(OP_RezzRequest, sizeof(Resurrect_Struct));
	auto *r     = (Resurrect_Struct *) outapp->pBuffer;

	strn0cpy(r->your_name, corpse_name, 64);
	strn0cpy(r->corpse_name, name, 64);
	strn0cpy(r->rezzer_name, caster->GetName(), 64);

	r->zone_id     = zone->GetZoneID();
	r->instance_id = zone->GetInstanceID();
	r->spellid     = spell_id;
	r->x           = m_Position.x;
	r->y           = m_Position.y;
	r->z           = GetFixedZ(m_Position);
	r->unknown000  = 0;
	r->unknown020  = 0;
	r->unknown088  = 0;

	// We send this to world, because it needs to go to the player who may not be in this zone.
	worldserver.RezzPlayer(outapp, m_rezzed_experience, m_corpse_db_id, OP_RezzRequest);
	safe_delete(outapp);
}

Corpse *Corpse::LoadCharacterCorpse(
	const CharacterCorpsesRepository::CharacterCorpses &cc,
	const glm::vec4 &position
)
{
	if (!cc.id) {
		LogCorpses("Unable to create a corpse entity for character corpse_id [{}]", cc.id);
		return nullptr;
	}

	const auto &items = CharacterCorpseItemsRepository::GetWhere(
		database,
		fmt::format(
			"`corpse_id` = {}",
			cc.id
		)
	);

	LootItems item_list;
	for (auto &i: items) {
		item_list.push_back(
			new LootItem{
				.item_id    = i.item_id,
				.equip_slot = static_cast<int16>(i.equip_slot),
				.charges    = static_cast<uint16>(i.charges),
				.lootslot   = 0,
				.aug_1      = i.aug_1,
				.aug_2      = i.aug_2,
				.aug_3      = i.aug_3,
				.aug_4      = i.aug_4,
				.aug_5      = i.aug_5,
				.aug_6      = static_cast<uint32>(i.aug_6),
				.attuned    = static_cast<bool>(i.attuned)
			}
		);
	}

	auto c = new Corpse(
		cc.id,
		cc.charid,
		cc.charname.c_str(),
		&item_list,
		cc.copper,
		cc.silver,
		cc.gold,
		cc.platinum,
		position,
		cc.size,
		cc.gender,
		cc.race,
		cc.class_,
		cc.deity,
		cc.level,
		cc.texture,
		cc.helm_texture,
		cc.exp,
		cc.gm_exp,
		static_cast<KilledByTypes>(cc.killed_by),
		cc.rezzable,
		cc.rez_time,
		RuleB(Zone, EnableShadowrest) ? false : cc.was_at_graveyard
	);

	if (cc.is_locked) {
		c->Lock();
	}

	// item tints and appearance
	c->m_item_tint.Head.Color      = cc.wc_1;
	c->m_item_tint.Chest.Color     = cc.wc_2;
	c->m_item_tint.Arms.Color      = cc.wc_3;
	c->m_item_tint.Wrist.Color     = cc.wc_4;
	c->m_item_tint.Hands.Color     = cc.wc_5;
	c->m_item_tint.Legs.Color      = cc.wc_6;
	c->m_item_tint.Feet.Color      = cc.wc_7;
	c->m_item_tint.Primary.Color   = cc.wc_8;
	c->m_item_tint.Secondary.Color = cc.wc_9;
	c->haircolor                   = cc.hair_color;
	c->beardcolor                  = cc.beard_color;
	c->eyecolor1                   = cc.eye_color_1;
	c->eyecolor2                   = cc.eye_color_2;
	c->hairstyle                   = cc.hair_style;
	c->luclinface                  = cc.face;
	c->beard                       = cc.beard;
	c->drakkin_heritage            = cc.drakkin_heritage;
	c->drakkin_tattoo              = cc.drakkin_tattoo;
	c->drakkin_details             = cc.drakkin_details;
	c->m_become_npc                = false;
	c->m_consented_guild_id        = cc.guild_consent_id;

	try {
		if (Strings::IsValidJson(cc.entity_variables)) {
			json j = json::parse(cc.entity_variables);
			for (auto& el : j.items()) {
				c->SetEntityVariable(el.key(), el.value().get<std::string>());
			}
		}
	} catch (const std::exception& ex) {
		LogError("Failed to parse entity_variables JSON for corpse ID %u: %s", cc.id, ex.what());
	}

	c->IsRezzed(cc.is_rezzed);

	c->UpdateEquipmentLight();

	return c;
}

void Corpse::SyncEntityVariablesToCorpseDB()
{
	if (!m_is_player_corpse || m_corpse_db_id == 0) {
		return;
	}

	json j;
	for (const auto& [key, value] : m_EntityVariables) {
		j[key] = value;
	}

	std::string serialized = j.dump();

	CharacterCorpsesRepository::UpdateEntityVariables(database, m_corpse_db_id, serialized);
}
