#include "../bot_command.h"

void bot_command_inventory(Client *c, const Seperator *sep)
{
	std::vector<const char*> subcommand_list = {
		"inventorygive",
		"inventorylist",
		"inventoryremove",
		"inventorywindow"
	};

	if (helper_command_alias_fail(c, "bot_command_inventory", sep->arg[0], "inventory"))
		return;

	helper_send_available_subcommands(c, "bot inventory", subcommand_list);
}

void bot_command_inventory_give(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_inventory_give", sep->arg[0], "inventorygive")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: {} ([actionable: target | byname] ([actionable_name])) [optional: slot ID]",
				sep->arg[0]
			).c_str()
		);
		return;
	}

	int ab_mask = (ActionableBots::ABM_Target | ActionableBots::ABM_ByName);
	int ab_arg = 1;
	int slot_arg = 1;
	int16 chosen_slot = INVALID_INDEX;
	bool byname = false;

	std::string byname_arg = sep->arg[ab_arg];

	if (!byname_arg.compare("byname")) {
		byname = true;
		slot_arg = ab_arg + 2;
	}

	if (sep->IsNumber(slot_arg)) {
		chosen_slot = atoi(sep->arg[slot_arg]);

		if (!EQ::ValueWithin(chosen_slot, EQ::invslot::EQUIPMENT_BEGIN, EQ::invslot::EQUIPMENT_END)) {
			c->Message(Chat::Yellow, "Please enter a valid inventory slot.");

			return;
		}

		if (!byname) {
			++ab_arg;
		}	
	}

	std::vector<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, sep->arg[ab_arg + 1]) == ActionableBots::ABT_None) {
		return;
	}

	auto my_bot = sbl.front();
	if (!my_bot) {
		c->Message(Chat::Yellow, "ActionableBots returned 'nullptr'");

		return;
	}

	my_bot->FinishTrade(c, Bot::BotTradeClientNoDropNoTrade, chosen_slot);
}

void bot_command_inventory_list(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_inventory_list", sep->arg[0], "inventorylist")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: {} ([actionable: target | byname] ([actionable_name]))",
				sep->arg[0]
			).c_str()
		);
		return;
	}

	int ab_mask = (ActionableBots::ABM_Target | ActionableBots::ABM_ByName);

	std::vector<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[1], sbl, ab_mask, sep->arg[2]) == ActionableBots::ABT_None) {
		return;
	}

	auto my_bot = sbl.front();
	if (!my_bot) {
		c->Message(Chat::White, "ActionableBots returned 'nullptr'");
		return;
	}

	const EQ::ItemInstance* inst = nullptr;
	const EQ::ItemData    * item = nullptr;
	bool is_2h_weapon = false;

	EQ::SayLinkEngine linker;
	linker.SetLinkType(EQ::saylink::SayLinkItemInst);

	uint32      inventory_count = 0;
	for (uint16 slot_id         = EQ::invslot::EQUIPMENT_BEGIN; slot_id <= EQ::invslot::EQUIPMENT_END; ++slot_id) {
		if (slot_id == EQ::invslot::slotSecondary && is_2h_weapon) {
			continue;
		}

		inst = my_bot->CastToBot()->GetBotItem(slot_id);
		if (!inst || !inst->GetItem()) {
			c->Message(
				Chat::White,
				fmt::format(
					"Slot {} ({}) | Empty",
					slot_id,
					EQ::invslot::GetInvPossessionsSlotName(slot_id)
				).c_str()
			);
			continue;
		}

		item = inst->GetItem();
		if (slot_id == EQ::invslot::slotPrimary && item->IsType2HWeapon()) {
			is_2h_weapon = true;
		}

		linker.SetItemInst(inst);
		c->Message(
			Chat::White,
			fmt::format(
				"Slot {} ({}) | {} | {}",
				slot_id,
				EQ::invslot::GetInvPossessionsSlotName(slot_id),
				linker.GenerateLink(),
				Saylink::Silent(
					fmt::format("^inventoryremove {}", slot_id),
					"Remove"
				)
			).c_str()
		);

		++inventory_count;
	}

	uint32 database_count = 0;
	database.botdb.QueryInventoryCount(my_bot->GetBotID(), database_count);

	if (inventory_count != database_count) {
		c->Message(
			Chat::White,
			fmt::format(
				"Inventory-database item count mismatch, inventory has {} item{} and the database has {} item{}.",
				inventory_count,
				inventory_count != 1 ? "s" : "",
				database_count,
				database_count != 1 ? "s" : ""
			).c_str()
		);
	}
}

void bot_command_inventory_remove(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_inventory_remove", sep->arg[0], "inventoryremove")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: {} [Slot ID: 0-22] ([actionable: target | byname] ([actionable_name]))",
				sep->arg[0]
			).c_str()
		);
		return;
	}

	int ab_mask = (ActionableBots::ABM_Target | ActionableBots::ABM_ByName);

	if (c->GetTradeskillObject() || (c->trade->state == Trading)) {
		c->MessageString(Chat::Tell, MERCHANT_BUSY);
		return;
	}

	std::vector<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[2], sbl, ab_mask, sep->arg[3]) == ActionableBots::ABT_None) {
		return;
	}

	auto my_bot = sbl.front();
	if (!my_bot) {
		c->Message(Chat::White, "ActionableBots returned 'nullptr'");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "Slot ID must be a number.");
		return;
	}

	auto slot_id = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[1]));
	if (slot_id > EQ::invslot::EQUIPMENT_END || slot_id < EQ::invslot::EQUIPMENT_BEGIN) {
		c->Message(Chat::White, "Valid slots are 0 to 22.");
		return;
	}

	auto* inst = my_bot->GetBotItem(slot_id);
	if (!inst) {
		std::string slot_message = "is";
		switch (slot_id) {
			case EQ::invslot::slotShoulders:
			case EQ::invslot::slotArms:
			case EQ::invslot::slotHands:
			case EQ::invslot::slotLegs:
			case EQ::invslot::slotFeet:
				slot_message = "are";
				break;
			default:
				break;
		}

		my_bot->OwnerMessage(
			fmt::format(
				"My {} (Slot {}) {} already unequipped.",
				EQ::invslot::GetInvPossessionsSlotName(slot_id),
				slot_id,
				slot_message
			)
		);
		return;
	}

	const auto* itm = inst->GetItem();
	EQ::SayLinkEngine linker;
	linker.SetLinkType(EQ::saylink::SayLinkItemInst);
	linker.SetItemInst(inst);

	if (inst && itm && c->CheckLoreConflict(itm)) {
		c->Message(
			Chat::White,
			fmt::format(
				"You cannot pick up {} because it is a lore item you already possess.",
				linker.GenerateLink()
			).c_str()
		);
		return;
	}

	for (int m = EQ::invaug::SOCKET_BEGIN; m <= EQ::invaug::SOCKET_END; ++m) {
		EQ::ItemInstance* augment = inst->GetAugment(m);
		if (!augment) {
			continue;
		}

		if (!c->CheckLoreConflict(augment->GetItem())) {
			continue;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"You cannot pick up {} because it is a lore item you already possess.",
				linker.GenerateLink()
			).c_str()
		);
		return;
	}

	if (itm) {
		EQ::SayLinkEngine linker;
		linker.SetLinkType(EQ::saylink::SayLinkItemInst);
		linker.SetItemInst(inst);

		c->PushItemOnCursor(*inst, true);
		if (
			slot_id == EQ::invslot::slotRange ||
			slot_id == EQ::invslot::slotAmmo
			) {
			my_bot->SetBotRangedSetting(false);
		}

		my_bot->RemoveBotItemBySlot(slot_id);
		my_bot->BotRemoveEquipItem(slot_id);
		my_bot->CalcBotStats(c->GetBotOption(Client::booStatsUpdate));

		my_bot->OwnerMessage(
			fmt::format(
				"I have unequipped {} from my {} (Slot {}).",
				linker.GenerateLink(),
				EQ::invslot::GetInvPossessionsSlotName(slot_id),
				slot_id
			)
		);

		if (parse->BotHasQuestSub(EVENT_UNEQUIP_ITEM_BOT)) {
			const auto& export_string = fmt::format(
				"{} {}",
				inst->IsStackable() ? inst->GetCharges() : 1,
				slot_id
			);

			std::vector<std::any> args = { inst };

			parse->EventBot(EVENT_UNEQUIP_ITEM_BOT, my_bot, nullptr, export_string, inst->GetID(), &args);
		}
	}
}

void bot_command_inventory_window(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_inventory_window", sep->arg[0], "inventorywindow")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: {} [actionable: target]",
				sep->arg[0]
			).c_str()
		);
		return;
	}

	int ab_mask = ActionableBots::ABM_Target;

	std::vector<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[1], sbl, ab_mask, sep->arg[2]) == ActionableBots::ABT_None) {
		return;
	}

	auto my_bot = sbl.front();
	if (!my_bot) {
		c->Message(Chat::White, "ActionableBots returned 'nullptr'");
		return;
	}

	std::string window_title = fmt::format(
		"{}'s Inventory",
		my_bot->GetCleanName()
	);

	std::string window_text = "<table>";
	for (uint16 slot_id     = EQ::invslot::EQUIPMENT_BEGIN; slot_id <= EQ::invslot::EQUIPMENT_END; ++slot_id) {
		const EQ::ItemData    * item = nullptr;
		const EQ::ItemInstance* inst = my_bot->CastToBot()->GetBotItem(slot_id);
		if (inst) {
			item = inst->GetItem();
		}

		window_text.append(
			fmt::format(
				"<tr><td>{}</td><td>{}{}</c></td></tr>",
				EQ::invslot::GetInvPossessionsSlotName(slot_id),
				item ? "<c \"#00FF00\">" : "<c \"#FFFF00\">",
				item ? item->Name : "Empty"
			)
		);
	}
	window_text.append("</table>");

	c->SendPopupToClient(
		window_title.c_str(),
		window_text.c_str()
	);
}
