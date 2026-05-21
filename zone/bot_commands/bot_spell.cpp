/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "zone/bot_command.h"

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

	// S39 fix #4: auto-create a neutral default row when none exists so
	// `^spellsettingstoggle <id> 0` works on first use instead of silently
	// no-opping. Defaults match `^spellsettingsadd <id> 0 0 100`.
	auto obs = my_bot->GetBotSpellSetting(spell_id);
	BotSpellSetting bs;

	if (obs) {
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
	}
	else {
		bs.priority = 0;
		bs.min_hp = 0;
		bs.max_hp = 100;
		bs.is_enabled = toggle;

		if (!my_bot->AddBotSpellSetting(spell_id, &bs)) {
			c->Message(
				Chat::White,
				fmt::format(
					"Failed to create spell setting for {}.",
					my_bot->GetCleanName()
				).c_str()
			);
			return;
		}
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

// S39 fix #4: shared helper for spelldisable/spellenable/spellsettingstoggle.
// Auto-creates a neutral default row if none exists; otherwise updates the
// existing row's is_enabled while preserving priority/hp thresholds.
// Caller is responsible for spell_id validity and target-bot ownership.
static void apply_spell_enable_toggle(Client *c, Bot *my_bot, uint16 spell_id, bool enable)
{
	auto obs = my_bot->GetBotSpellSetting(spell_id);
	BotSpellSetting bs;

	if (obs) {
		bs.priority = obs->priority;
		bs.min_hp = obs->min_hp;
		bs.max_hp = obs->max_hp;
		bs.is_enabled = enable;

		if (!my_bot->UpdateBotSpellSetting(spell_id, &bs)) {
			c->Message(
				Chat::White,
				fmt::format(
					"Failed to {}able spell for {}.",
					enable ? "en" : "dis",
					my_bot->GetCleanName()
				).c_str()
			);
			return;
		}
	}
	else {
		bs.priority = 0;
		bs.min_hp = 0;
		bs.max_hp = 100;
		bs.is_enabled = enable;

		if (!my_bot->AddBotSpellSetting(spell_id, &bs)) {
			c->Message(
				Chat::White,
				fmt::format(
					"Failed to create spell setting for {}.",
					my_bot->GetCleanName()
				).c_str()
			);
			return;
		}
	}

	my_bot->AI_AddBotSpells(my_bot->GetBotSpellID());

	c->Message(
		Chat::White,
		fmt::format(
			"Spell {}abled | Spell: {} ({}) on {}.",
			enable ? "En" : "Dis",
			spells[spell_id].name,
			spell_id,
			my_bot->GetCleanName()
		).c_str()
	);
}

void bot_command_spell_disable(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_spell_disable", sep->arg[0], "spelldisable")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			fmt::format("Usage: {} [Spell ID] -- disables a spell for the targeted bot.", sep->arg[0]).c_str()
		);
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must target a bot that you own to use this command.");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, fmt::format("Usage: {} [Spell ID]", sep->arg[0]).c_str());
		return;
	}

	auto spell_id = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[1]));
	if (!IsValidSpell(spell_id)) {
		c->Message(
			Chat::White,
			fmt::format("Spell ID {} is invalid or could not be found.", spell_id).c_str()
		);
		return;
	}

	apply_spell_enable_toggle(c, my_bot, spell_id, false);
}

void bot_command_spell_enable(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_spell_enable", sep->arg[0], "spellenable")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			fmt::format("Usage: {} [Spell ID] -- re-enables a previously disabled spell for the targeted bot.", sep->arg[0]).c_str()
		);
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must target a bot that you own to use this command.");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, fmt::format("Usage: {} [Spell ID]", sep->arg[0]).c_str());
		return;
	}

	auto spell_id = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[1]));
	if (!IsValidSpell(spell_id)) {
		c->Message(
			Chat::White,
			fmt::format("Spell ID {} is invalid or could not be found.", spell_id).c_str()
		);
		return;
	}

	apply_spell_enable_toggle(c, my_bot, spell_id, true);
}
