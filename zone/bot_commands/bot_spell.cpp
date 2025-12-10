#include "../bot_command.h"

void bot_command_spell_list(Client* c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_spell_list", sep->arg[0], "spells")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: {} [Min Level] (Level is optional)",
				sep->arg[0]
			).c_str()
		);
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must target a bot that you own to use this command.");
		return;
	}

	uint8 min_level = 0;

	if (sep->IsNumber(1)) {
		min_level = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[1]));
	}

	my_bot->ListBotSpells(min_level);
}

void bot_command_spell_settings_add(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_spell_settings_add", sep->arg[0], "spellsettingsadd")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: {} [Spell ID] [Priority] [Min HP] [Max HP]",
				sep->arg[0]
			).c_str()
		);
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must target a bot that you own to use this command.");
		return;
	}

	auto arguments = sep->argnum;
	if (
		arguments < 4 ||
		!sep->IsNumber(1) ||
		!sep->IsNumber(2) ||
		!sep->IsNumber(3) ||
		!sep->IsNumber(4)
		) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: {} [Spell ID] [Priority] [Min HP] [Max HP]",
				sep->arg[0]
			).c_str()
		);
		return;
	}

	auto spell_id = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[1]));

	if (!IsValidSpell(spell_id)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Spell ID {} is invalid or could not be found.",
				spell_id
			).c_str()
		);
		return;
	}

	if (my_bot->GetBotSpellSetting(spell_id)) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} already has a spell setting for {} ({}), trying using {} instead.",
				my_bot->GetCleanName(),
				spells[spell_id].name,
				spell_id,
				Saylink::Silent("^spellsettingsupdate")
			).c_str()
		);
		return;
	}

	auto priority = static_cast<int16>(Strings::ToInt(sep->arg[2]));
	auto min_hp = static_cast<int8>(EQ::Clamp(Strings::ToInt(sep->arg[3]), -1, 99));
	auto max_hp = static_cast<int8>(EQ::Clamp(Strings::ToInt(sep->arg[4]), -1, 100));

	BotSpellSetting bs;

	bs.priority = priority;
	bs.min_hp = min_hp;
	bs.max_hp = max_hp;

	if (!my_bot->AddBotSpellSetting(spell_id, &bs)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to add spell setting for {}.",
				my_bot->GetCleanName()
			).c_str()
		);
		return;
	}

	my_bot->AI_AddBotSpells(my_bot->GetBotSpellID());

	c->Message(
		Chat::White,
		fmt::format(
			"Successfully added spell setting for {}.",
			my_bot->GetCleanName()
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Spell Setting Added | Spell: {} ({}) ",
			spells[spell_id].name,
			spell_id
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Spell Setting Added | Priority: {} Health: {}",
			priority,
			my_bot->GetHPString(min_hp, max_hp)
		).c_str()
	);
}

void bot_command_spell_settings_delete(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_spell_settings_delete", sep->arg[0], "spellsettingsdelete")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: {} [Spell ID]",
				sep->arg[0]
			).c_str()
		);
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must target a bot that you own to use this command.");
		return;
	}

	auto arguments = sep->argnum;
	if (
		arguments < 1 ||
		!sep->IsNumber(1)
		) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: {} [Spell ID]",
				sep->arg[0]
			).c_str()
		);
		return;
	}

	auto spell_id = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[1]));

	if (!IsValidSpell(spell_id)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Spell ID {} is invalid or could not be found.",
				spell_id
			).c_str()
		);
		return;
	}

	if (!my_bot->DeleteBotSpellSetting(spell_id)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to delete spell setting for {}.",
				my_bot->GetCleanName()
			).c_str()
		);
		return;
	}

	my_bot->AI_AddBotSpells(my_bot->GetBotSpellID());

	c->Message(
		Chat::White,
		fmt::format(
			"Successfully deleted spell setting for {}.",
			my_bot->GetCleanName()
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Spell Setting Deleted | Spell: {} ({})",
			spells[spell_id].name,
			spell_id
		).c_str()
	);
}

void bot_command_spell_settings_list(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_spell_settings_list", sep->arg[0], "spellsettings")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: {}",
				sep->arg[0]
			).c_str()
		);
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must target a bot that you own to use this command.");
		return;
	}

	my_bot->ListBotSpellSettings();
}

void bot_command_spell_settings_toggle(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_spell_settings_toggle", sep->arg[0], "spellsettingstoggle")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: {} [Spell ID] [Toggle]",
				sep->arg[0]
			).c_str()
		);
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must target a bot that you own to use this command.");
		return;
	}

	auto arguments = sep->argnum;
	if (
		arguments < 2 ||
		!sep->IsNumber(1)
		) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: {} [Spell ID] [Toggle]",
				sep->arg[0]
			).c_str()
		);
		return;
	}

	auto spell_id = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[1]));
	if (!IsValidSpell(spell_id)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Spell ID {} is invalid or could not be found.",
				spell_id
			).c_str()
		);
		return;
	}

	bool toggle = (
		sep->IsNumber(2) ?
			Strings::ToInt(sep->arg[2]) != 0 :
			atobool(sep->arg[2])
	);

	auto obs = my_bot->GetBotSpellSetting(spell_id);
	if (!obs) {
		return;
	}

	BotSpellSetting bs;

	bs.priority = obs->priority;
	bs.min_hp = obs->min_hp;
	bs.max_hp = obs->max_hp;
	bs.is_enabled = toggle;

	if (!my_bot->UpdateBotSpellSetting(spell_id, &bs)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to {}able spell for {}.",
				toggle ? "en" : "dis",
				my_bot->GetCleanName()
			).c_str()
		);
		return;
	}

	my_bot->AI_AddBotSpells(my_bot->GetBotSpellID());

	c->Message(
		Chat::White,
		fmt::format(
			"Successfully {}abled spell for {}.",
			toggle ? "en" : "dis",
			my_bot->GetCleanName()
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Spell {}abled | Spell: {} ({})",
			toggle ? "En" : "Dis",
			spells[spell_id].name,
			spell_id
		).c_str()
	);
}

void bot_command_spell_settings_update(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_spell_settings_update", sep->arg[0], "spellsettingsupdate")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: {} [Spell ID] [Priority] [Min HP] [Max HP]",
				sep->arg[0]
			).c_str()
		);
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must target a bot that you own to use this command.");
		return;
	}

	auto arguments = sep->argnum;
	if (
		arguments < 4 ||
		!sep->IsNumber(1) ||
		!sep->IsNumber(2) ||
		!sep->IsNumber(3) ||
		!sep->IsNumber(4)
		) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: {} [Spell ID] [Priority] [Min HP] [Max HP]",
				sep->arg[0]
			).c_str()
		);
		return;
	}

	auto spell_id = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[1]));

	if (!IsValidSpell(spell_id)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Spell ID {} is invalid or could not be found.",
				spell_id
			).c_str()
		);
		return;
	}

	auto priority = static_cast<int16>(Strings::ToInt(sep->arg[2]));
	auto min_hp = static_cast<int8>(EQ::Clamp(Strings::ToInt(sep->arg[3]), -1, 99));
	auto max_hp = static_cast<int8>(EQ::Clamp(Strings::ToInt(sep->arg[4]), -1, 100));

	BotSpellSetting bs;

	bs.priority = priority;
	bs.min_hp = min_hp;
	bs.max_hp = max_hp;

	if (!my_bot->UpdateBotSpellSetting(spell_id, &bs)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to update spell setting for {}.",
				my_bot->GetCleanName()
			).c_str()
		);
		return;
	}

	my_bot->AI_AddBotSpells(my_bot->GetBotSpellID());

	c->Message(
		Chat::White,
		fmt::format(
			"Successfully updated spell setting for {}.",
			my_bot->GetCleanName()
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Spell Setting Updated | Spell: {} ({})",
			spells[spell_id].name,
			spell_id
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Spell Setting Updated | Priority: {} Health: {}",
			priority,
			my_bot->GetHPString(min_hp, max_hp)
		).c_str()
	);
}

void bot_spell_info_dialogue_window(Client* c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_spell_info_dialogue_window", sep->arg[0], "spellinfo")) {
		return;
	}

	auto arguments = sep->argnum;
	if (
		arguments < 1 ||
		!sep->IsNumber(1)
		) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: {} [Spell ID]",
				sep->arg[0]
			).c_str()
		);
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must target a bot that you own to use this command.");
		return;
	}

	auto spell_id = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[1]));
	auto min_level = spells[spell_id].classes;
	auto class_level = min_level[my_bot->GetBotClass() - 1];

	if (class_level > my_bot->GetLevel()) {
		c->Message(Chat::White, "This is not a usable spell by your bot.");
		return;
	}

	auto results = database.QueryDatabase(
		fmt::format(
			"SELECT value FROM db_str WHERE id = {} and type = 6 LIMIT 1",
			spells[spell_id].description_id
		)
	);

	if (!results.Success() || !results.RowCount()) {
		c->Message(Chat::White, "No Spell Information Available for this.");
		return;
	}

	auto row = results.begin();
	std::string spell_desc = row[0];

	auto m = DialogueWindow::TableRow(
		DialogueWindow::TableCell("Spell Effect: ") +
		DialogueWindow::TableCell(spell_desc)
	);

	m += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Spell Level: ") +
		DialogueWindow::TableCell(fmt::format("{}", class_level))
	);

	c->SendPopupToClient(
		fmt::format(
			"Spell: {}", spells[spell_id].name
		).c_str(),
		DialogueWindow::Table(m).c_str()
	);
}

void bot_command_enforce_spell_list(Client* c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_enforce_spell_list", sep->arg[0], "enforcespellsettings")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: {} [True|False] (Blank to toggle]",
				sep->arg[0]
			).c_str()
		);
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must target a bot that you own to use this command.");
		return;
	}

	bool enforce_state = (sep->argnum > 0) ? Strings::ToBool(sep->arg[1]) : !my_bot->GetBotEnforceSpellSetting();
	my_bot->SetBotEnforceSpellSetting(enforce_state);

	c->Message(
		Chat::White,
		fmt::format(
			"{}'s Spell Settings List entries are now {}.",
			my_bot->GetCleanName(),
			my_bot->GetBotEnforceSpellSetting() ? "enforced" : "optional"
		).c_str()
	);
}
