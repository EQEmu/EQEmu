#include "../bot_command.h"

void bot_command_item_use(Client* c, const Seperator* sep)
{
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: [%s empty] will display only bots that can use the item in an empty slot.", sep->arg[0]);
		c->Message(Chat::White, "usage: [%s caster] will display only caster bots that can use the item.", sep->arg[0]);
		c->Message(Chat::White, "usage: [%s hybrid] will display only hybrid bots that can use the item.", sep->arg[0]);
		c->Message(Chat::White, "usage: [%s melee] will display only melee bots that can use the item.", sep->arg[0]);
		c->Message(Chat::White, "usage: [%s wiscaster] will display only Wisdom-based Caster bots that can use the item.", sep->arg[0]);
		c->Message(Chat::White, "usage: [%s intcaster] will display only Intelligence-based Caster bots that can use the item.", sep->arg[0]);
		c->Message(Chat::White, "usage: [%s plate] will display only Plate-wearing bots that can use the item.", sep->arg[0]);
		c->Message(Chat::White, "usage: [%s chain] will display only Chain-wearing bots that can use the item.", sep->arg[0]);
		c->Message(Chat::White, "usage: [%s leather] will display only Leather-wearing bots that can use the item.", sep->arg[0]);
		c->Message(Chat::White, "usage: [%s cloth] will display only Cloth-wearing bots that can use the item.", sep->arg[0]);
		c->Message(Chat::White, "usage: [%s haste] will display bots that have no or lesser haste than the item.", sep->arg[0]);
		c->Message(Chat::White, "usage: You can also use empty or haste as an argument to narrow down further, for example [%s caster empty], [%s plate haste] or even [%s empty haste]", sep->arg[0], sep->arg[0], sep->arg[0]);
		return;
	}

	bool empty_only = false;
	int8 class_mask = 0;
	bool caster_only = false;
	bool hybrid_only = false;
	bool melee_only = false;
	bool wis_caster_only = false;
	bool int_caster_only = false;
	bool plate_only = false;
	bool chain_only = false;
	bool leather_only = false;
	bool cloth_only = false;
	bool haste_only = false;
	int haste_value = 0;

	int ab_arg = 2;
	std::string arg1 = sep->arg[1];
	std::string arg2 = sep->arg[2];

	if (arg1.compare("empty") == 0 || arg2.compare("empty") == 0) {
		empty_only = true;

		if (arg2.compare("empty") == 0) {
			++ab_arg;
		}
	}

	if (arg1.compare("haste") == 0 || arg2.compare("haste") == 0) {
		haste_only = true;

		if (arg2.compare("haste") == 0) {
			++ab_arg;
		}
	}

	if (arg1.compare("caster") == 0) {
		caster_only = true;
	}
	else if (arg1.compare("hybrid") == 0) {
		hybrid_only = true;
	}
	else if (arg1.compare("melee") == 0) {
		melee_only = true;
	}
	else if (arg1.compare("wiscaster") == 0) {
		wis_caster_only = true;
	}
	else if (arg1.compare("intcaster") == 0) {
		int_caster_only = true;
	}
	else if (arg1.compare("plate") == 0) {
		plate_only = true;
	}
	else if (arg1.compare("chain") == 0) {
		chain_only = true;
	}
	else if (arg1.compare("leather") == 0) {
		leather_only = true;
	}
	else if (arg1.compare("cloth") == 0) {
		cloth_only = true;
	}
	else {
		if (arg1.empty()) {
			--ab_arg;
		}
		else {
			if (!(arg1.compare("empty") == 0) && !(arg1.compare("haste") == 0)) {
				c->Message(Chat::White, "Please choose the correct subtype. For help use %s help.", sep->arg[0]);

				return;
			}
		}
	}

	const auto item_instance = c->GetInv().GetItem(EQ::invslot::slotCursor);

	if (!item_instance) {
		c->Message(Chat::Yellow, "No item found on cursor! For help use %s help.", sep->arg[0]);

		return;
	}

	auto item_data = item_instance->GetItem();

	if (!item_data) {
		c->Message(Chat::Yellow, "No data found for cursor item!");

		return;
	}

	if (item_data->ItemClass != EQ::item::ItemClassCommon || item_data->Slots == 0) {
		c->Message(Chat::Yellow, "'%s' is not an equipable item!", item_data->Name);

		return;
	}

	std::vector<int16> equipable_slot_list;

	for (int16 equipable_slot = EQ::invslot::EQUIPMENT_BEGIN; equipable_slot <= EQ::invslot::EQUIPMENT_END; ++equipable_slot) {
		if (item_data->Slots & (1 << equipable_slot)) {
			equipable_slot_list.emplace_back(equipable_slot);
		}
	}

	const int ab_mask = ActionableBots::ABM_Type1;
	std::string actionable_arg = sep->arg[ab_arg];

	if (actionable_arg.empty()) {
		actionable_arg = "spawned";
	}

	std::string class_race_arg = sep->arg[ab_arg];
	bool class_race_check = false;

	if (!class_race_arg.compare("byclass") || !class_race_arg.compare("byrace")) {
		class_race_check = true;
	}

	std::vector<Bot*> sbl;

	if (ActionableBots::PopulateSBL(c, actionable_arg, sbl, ab_mask, !class_race_check ? sep->arg[ab_arg + 1] : nullptr, class_race_check ? atoi(sep->arg[ab_arg + 1]) : 0) == ActionableBots::ABT_None) {
		return;
	}

	sbl.erase(std::remove(sbl.begin(), sbl.end(), nullptr), sbl.end());

	EQ::SayLinkEngine linker;
	linker.SetLinkType(EQ::saylink::SayLinkItemData);

	for (const auto& bot_iter : sbl) {
		if (!bot_iter) {
			continue;
		}

		if (caster_only && !IsCasterClass(bot_iter->GetClass())) {
			continue;
		}

		if (hybrid_only && !IsSpellFighterClass(bot_iter->GetClass())) {
			continue;
		}

		if (melee_only && !IsNonSpellFighterClass(bot_iter->GetClass())) {
			continue;
		}

		if (wis_caster_only && !IsWISCasterClass(bot_iter->GetClass())) {
			continue;
		}

		if (int_caster_only && !IsINTCasterClass(bot_iter->GetClass())) {
			continue;
		}

		if (plate_only && !IsPlateClass(bot_iter->GetClass())) {
			continue;
		}

		if (chain_only && !IsChainClass(bot_iter->GetClass())) {
			continue;
		}

		if (leather_only && !IsLeatherClass(bot_iter->GetClass())) {
			continue;
		}

		if (cloth_only && !IsClothClass(bot_iter->GetClass())) {
			continue;
		}

		if (
			(!RuleB(Bots, AllowBotEquipAnyRaceGear) && ((~item_data->Races) & GetPlayerRaceBit(bot_iter->GetRace()))) ||
			(!RuleB(Bots, AllowBotEquipAnyClassGear) && ((~item_data->Classes) & GetPlayerClassBit(bot_iter->GetClass())))
		) {
			continue;
		}

		std::list<int16> refined_equipable_slot_list;
		bool skip_bot = false;
		const EQ::ItemData* equipped_item = nullptr;
		const EQ::ItemInstance* equipped_inst = nullptr;

		for (const auto& slot_iter : equipable_slot_list) {
			// needs more failure criteria - this should cover the bulk for now
			if (slot_iter == EQ::invslot::slotSecondary && item_data->Damage && !bot_iter->CanThisClassDualWield()) {
				continue;
			}

			if (item_data->ReqLevel > bot_iter->GetLevel()) {
				continue;
			}

			haste_value = 0;
			equipped_item = nullptr;
			equipped_inst = nullptr;

			
			for (int16 equipable_slot = EQ::invslot::EQUIPMENT_BEGIN; equipable_slot <= EQ::invslot::EQUIPMENT_END; ++equipable_slot) {
				equipped_inst = bot_iter->GetInv()[equipable_slot];
				if (equipped_inst && equipped_inst->GetItem()) {
					equipped_item = equipped_inst->GetItem();

					if (item_data->CheckLoreConflict(equipped_item)) {
						skip_bot = true;
						break;
					}

					if (haste_only) {
						if (equipped_item->Haste > haste_value) {
							haste_value = equipped_item->Haste;
						}
					}
				}
			}

			if (skip_bot) {
				break;
			}

			if (haste_only && item_data->Haste < haste_value) {
				continue;
			}

			equipped_inst = bot_iter->GetInv()[slot_iter];

			if (equipped_inst && empty_only) {
				continue;
			}

			refined_equipable_slot_list.push_back(slot_iter);
		}

		if (skip_bot) {
			continue;
		}

		if (refined_equipable_slot_list.empty()) {
			continue;
		}

		for (auto slot_iter : refined_equipable_slot_list) {
			equipped_item = nullptr;
			equipped_inst = nullptr;

			equipped_inst = bot_iter->GetInv()[slot_iter];

			if (equipped_inst && equipped_inst->GetItem()) {
				equipped_item = equipped_inst->GetItem();
			}

			if (equipped_item) {
				linker.SetItemData(equipped_item);

				c->Message(
					Chat::Say,
					fmt::format(
						"{} says, 'I can use that for my {} instead of my {}! Would you like to {}?'",
						Saylink::Silent(
							fmt::format(
								"^inventorygive byname {}",
								bot_iter->GetCleanName()
							),
							bot_iter->GetCleanName()
						),
						EQ::invslot::GetInvPossessionsSlotName(slot_iter),
						linker.GenerateLink(),
						Saylink::Silent(
							fmt::format(
								"^inventoryremove {} byname {}",
								slot_iter,
								bot_iter->GetCleanName()
							),
							"remove my item"
						)
					).c_str()
				);
			}
			else {
				c->Message(
					Chat::Say,
					fmt::format(
						"{} says, 'I can use that for my {}! Would you like to {}?'",
						Saylink::Silent(
							fmt::format(
								"^inventorygive byname {}",
								bot_iter->GetCleanName()
							),
							bot_iter->GetCleanName()
						),
						EQ::invslot::GetInvPossessionsSlotName(slot_iter),
						Saylink::Silent(
							fmt::format(
								"^inventorygive byname {}",
								bot_iter->GetCleanName()
							),
							"give it to me"
						)
					).c_str()
				);
			}

			if (RuleB(Bots, DoResponseAnimations)) {
				bot_iter->DoAnim(29);
			}
		}
	}
}
