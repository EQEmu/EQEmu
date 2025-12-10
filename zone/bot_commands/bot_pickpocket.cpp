#include "../bot_command.h"

void bot_command_pickpocket(Client *c, const Seperator *sep)
{
	if (helper_command_disabled(c, RuleB(Bots, AllowPickpocketCommand), "pickpocket")) {
		return;
	}

	if (helper_command_alias_fail(c, "bot_command_pickpocket", sep->arg[0], "pickpocket")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: <enemy_target>", sep->arg[0]);
		return;
	}

	std::vector<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	// Check for capable rogue
	ActionableBots::Filter_ByClasses(c, sbl, player_class_bitmasks[Class::Rogue]);
	Bot *my_bot = ActionableBots::AsSpawned_ByMinLevelAndClass(c, sbl, 7, Class::Rogue);
	if (!my_bot) {
		c->Message(Chat::White, "No bots are capable of performing this action");
		return;
	}

	// Make sure a mob is targetted and a valid NPC
	Mob *target_mob = ActionableTarget::AsSingle_ByAttackable(c);
	if (!target_mob || !target_mob->IsNPC()) {
		c->Message(Chat::White, "You must <target> an enemy to use this command");
		return;
	}

	NPC *target_npc = ActionableTarget::AsSingle_ByAttackable(c)->CastToNPC();

	// Check if mob is close enough
	glm::vec4 mob_distance    = (c->GetPosition() - target_mob->GetPosition());
	float     mob_xy_distance = ((mob_distance.x * mob_distance.x) + (mob_distance.y * mob_distance.y));
	float     mob_z_distance  = (mob_distance.z * mob_distance.z);
	float     z_offset_diff   = target_mob->GetZOffset() - c->GetZOffset();

	if (mob_z_distance >= (35-z_offset_diff) || mob_xy_distance > 250) {
		c->Message(Chat::White, "You must be closer to an enemy to use this command");
		return;
	}

	// Adapted from pickpock skill in npc.cpp
	// Make sure we are allowed to target them
	uint8 over_level = target_mob->GetLevel();
	if (over_level > (my_bot->GetLevel() + THIEF_PICKPOCKET_OVER)) {
		c->Message(Chat::Red, "You are too inexperienced to pick pocket this target");
		return;
	}

	// Random fail roll
	if (zone->random.Roll(5)) {
		if (zone->CanDoCombat()) {
			target_mob->AddToHateList(c, 50);
		}
		target_mob->Say("Stop thief!");
		c->Message(Chat::Red, "You are noticed trying to steal!");
		return;
	}

	// Setup variables for calcs
	bool steal_skill  = my_bot->GetSkill(EQ::skills::SkillPickPockets);
	bool steal_chance = steal_skill * 100 / (5 * over_level + 5);

	// Determine whether to steal money or an item.
	uint32 money[6] = {
		0,
		((steal_skill >= 125) ? (target_npc->GetPlatinum()) : (0)),
		((steal_skill >= 60) ? (target_npc->GetGold()) : (0)),
		target_npc->GetSilver(),
		target_npc->GetCopper(),
		0
	};

	bool   has_coin   = ((money[PickPocketPlatinum] | money[PickPocketGold] | money[PickPocketSilver] | money[PickPocketCopper]) != 0);
	bool   steal_item = (steal_skill >= steal_chance && (zone->random.Roll(50) || !has_coin));

	// Steal item
	while (steal_item) {
		std::vector<std::pair<const EQ::ItemData *, uint16>> loot_selection; // <const ItemData*, charges>
		for (auto                                            item_iter: target_npc->GetLootItems()) {
			if (!item_iter || !item_iter->item_id) {
				continue;
			}
			auto item_test = database.GetItem(item_iter->item_id);
			if (item_test->Magic || !item_test->NoDrop || item_test->IsClassBag() || c->CheckLoreConflict(item_test) ||
				item_iter->equip_slot != EQ::invslot::SLOT_INVALID) {
				continue;
			}
			loot_selection.emplace_back(
				std::make_pair(
					item_test,
					((item_test->Stackable) ? (1) : (item_iter->charges))
				)
			);
		}
		if (loot_selection.empty()) {
			steal_item = false;
			break;
		}

		int random = zone->random.Int(0, (loot_selection.size() - 1));

		int16 slot_id = c->GetInv().FindFreeSlot(
			false,
			true,
			(loot_selection[random].first->Size),
			(loot_selection[random].first->ItemType == EQ::item::ItemTypeArrow)
		);
		if (slot_id == INVALID_INDEX) {
			steal_item = false;
			break;
		}

		auto item_inst = database.CreateItem(loot_selection[random].first, loot_selection[random].second);
		if (item_inst == nullptr) {
			steal_item = false;
			break;
		}

		// Successful item pickpocket
		if (item_inst->IsStackable() && RuleB(Character, UseStackablePickPocketing)) {
			if (!c->TryStacking(item_inst, ItemPacketTrade, false, false)) {
				c->PutItemInInventory(slot_id, *item_inst);
				c->SendItemPacket(slot_id, item_inst, ItemPacketTrade);
			}
		}
		else {
			c->PutItemInInventory(slot_id, *item_inst);
			c->SendItemPacket(slot_id, item_inst, ItemPacketTrade);
		}
		target_npc->RemoveItem(item_inst->GetID());
		c->Message(Chat::White, "You stole an item.");
		safe_delete(item_inst);
		return;
	}

	// no items, try money
	while (!steal_item && has_coin) {
		uint32 coin_amount = zone->random.Int(1, (steal_skill / 25) + 1);

		int coin_type = PickPocketPlatinum;
		while (coin_type <= PickPocketCopper) {
			if (money[coin_type]) {
				if (coin_amount > money[coin_type]) {
					coin_amount = money[coin_type];
				}
				break;
			}
			++coin_type;
		}
		if (coin_type > PickPocketCopper) {
			break;
		}

		memset(money, 0, (sizeof(int) * 6));
		money[coin_type] = coin_amount;

		if (zone->random.Roll(steal_chance)) { // Successful coin pickpocket
			switch (coin_type) {
				case PickPocketPlatinum:
					target_npc->SetPlatinum(target_npc->GetPlatinum() - coin_amount);
					break;
				case PickPocketGold:
					target_npc->SetGold(target_npc->GetGold() - coin_amount);
					break;
				case PickPocketSilver:
					target_npc->SetSilver(target_npc->GetSilver() - coin_amount);
					break;
				case PickPocketCopper:
					target_npc->SetCopper(target_npc->GetCopper() - coin_amount);
					break;
				default: // has_coin..but, doesn't have coin?
					c->Message(Chat::Red, "You failed to pickpocket.");
					return;
			}
			c->Message(Chat::White, "You stole money.");
			c->AddMoneyToPP(
				money[PickPocketCopper],
				money[PickPocketSilver],
				money[PickPocketGold],
				money[PickPocketPlatinum],
				true
			);
			return;
		}

		c->Message(Chat::Red, "You failed to pickpocket.");
		return;
	}
	c->Message(Chat::White, "This target's pockets are empty");
}
