#include "../bot_command.h"

void bot_command_copy_settings(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_copy_settings", sep->arg[0], "copysettings")) {
		c->Message(Chat::White, "note: Copies settings from one bot to another.");

		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		BotCommandHelpParams p;

		p.description =
		{
			"Copies settings from one bot to another."
		};
		p.notes =
		{
			"- You can put a spell type ID or shortname after any option except [all], [misc] and [spellsettings] to restore that specifc spell type only"
		};
		p.example_format = { fmt::format("{} [from] [to] [option] [optional: spelltype id/short name]", sep->arg[0]) };
		p.examples_one =
		{
			"To copy all settings from BotA to BotB:",
			fmt::format("{} BotA BotB all", sep->arg[0])
		};
		p.examples_two =
		{
			"To copy only Nuke spelltypesettings from BotA to BotB:",
			fmt::format(
				"{} BotA BotB spelltypesettings {}",
				sep->arg[0],
				Bot::GetSpellTypeShortNameByID(BotSpellTypes::Nuke)
			),
			fmt::format(
				"{} BotA BotB spelltypesettings {}",
				sep->arg[0],
				Bot::GetSpellTypeShortNameByID(BotSpellTypes::Nuke)
			),
		};
		p.examples_three =
		{
			"To copy only spellsettings from BotA to BotB:",
			fmt::format(
				"{} BotA BotB spellsettings",
				sep->arg[0]
			),
			fmt::format(
				"{} BotA BotB spellsettings",
				sep->arg[0]
			),
		};
		p.actionables = { "target, byname, ownergroup, ownerraid, targetgroup, namesgroup, healrotationtargets, mmr, byclass, byrace, spawned" };
		p.options = { "all, misc, spellsettings, spelltypesettings, spellholds, spelldelays, spellminthresholds, spellmaxthresholds, spellresistlimits, spellaggrochecks, spellminmanapct, spellmaxmanapct, spellminhppct, spellmaxhppct, spellidlepriority, spellengagedpriority, spellpursuepriority, spelltargetcounts, spellannouncecasts, blockedbuffs, blockedpetbuffs" };
		p.options_one =
		{
			"[spellsettings] will copy ^spellsettings options",
			"[spelltypesettings] copies all spell type settings",
			"[all] copies all settings" 
		};
		p.options_two =
		{
			"[misc] copies all miscellaneous options such as:",
			"- ^showhelm, ^followd, ^stopmeleelevel, ^enforcespellsettings, ^bottoggleranged, ^petsettype, ^behindmob, ^distanceranged, ^illusionblock, ^sitincombat, ^sithppercent, ^sitmanapercent, ^blockedbuffs, ^blockedpetbuffs"
		};
		p.options_three = { "The remaining options copy that specific type" };

		std::string popup_text = c->SendBotCommandHelpWindow(p);
		popup_text = DialogueWindow::Table(popup_text);

		c->SendPopupToClient(sep->arg[0], popup_text.c_str());
		c->SendSpellTypePrompts();

		if (RuleB(Bots, SendClassRaceOnHelp)) {
			c->Message(
				Chat::Yellow,
				fmt::format(
					"Use {} for information about race/class IDs.",
					Saylink::Silent("^classracelist")
				).c_str()
			);
		}

		return;
	}

	std::vector<std::string> options =
	{
		"all",
		"misc",
		"spellsettings", 
		"spelltypesettings", 
		"spellholds", 
		"spelldelays", 
		"spellminthresholds", 
		"spellmaxthresholds",
		"spellresistlimits",
		"spellaggrochecks",
		"spellminmanapct", 
		"spellmaxmanapct", 
		"spellminhppct", 
		"spellmaxhppct", 
		"spellidlepriority", 
		"spellengagedpriority",
		"spellpursuepriority",
		"spelltargetcounts",
		"spellannouncecasts",
		"blockedbuffs", 
		"blockedpetbuffs"
	};

	std::string option_arg = Strings::ToLower(sep->arg[3]);
	bool valid_option = false;
	bool copy_all = false;
	bool copy_misc = false;
	bool copy_spell_settings = false;
	bool copy_spell_type_settings = false;
	bool copy_blocked_buffs = false;
	bool copy_blocked_pet_buffs = false;
	uint16 setting_type = UINT16_MAX;

	for (int i = 0; i < options.size(); i++) {
		if (option_arg == options[i]) {
			if (option_arg == "all") {
				copy_all = true;
				valid_option = true;

				break;
			}
			else if (option_arg == "misc") {
				copy_misc = true;
				valid_option = true;

				break;
			}
			else if (option_arg == "spellsettings") {
				copy_spell_settings = true;
				valid_option = true;

				break;
			}
			else if (option_arg == "spelltypesettings") {
				copy_spell_type_settings = true;
				valid_option = true;

				break;
			}
			else if (option_arg == "blockedbuffs") {
				copy_blocked_buffs = true;
				valid_option = true;

				break;
			}
			else if (option_arg == "blockedpetbuffs") {
				copy_blocked_pet_buffs = true;
				valid_option = true;

				break;
			}
			else {
				setting_type = Bot::GetBotSpellCategoryIDByShortName(option_arg);

				if (setting_type != UINT16_MAX) {
					valid_option = true;

					break;
				}
			}
		}
	}

	if (!valid_option) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"Incorrect argument, use {} for information regarding this command.",
				Saylink::Silent(
					fmt::format("{} help", sep->arg[0])
				)
			).c_str()
		);

		return;
	}

	auto from = entity_list.GetBotByBotName(sep->arg[1]);

	if (!from) {
		c->Message(Chat::Yellow, "Could not find %s.", sep->arg[1]);
		return;
	}

	if (!from->IsBot()) {
		c->Message(Chat::Yellow, "%s is not a bot.", from->GetCleanName());
		return;
	}

	if (!from->GetOwner()) {
		c->Message(Chat::Yellow, "Could not find %s's owner.", from->GetCleanName());
	}

	if (RuleB(Bots, CopySettingsOwnBotsOnly) && from->GetOwner() != c) {
		c->Message(Chat::Yellow, "You name a bot you own to use this command.");
		return;
	}

	if (!RuleB(Bots, AllowCopySettingsAnon) && from->GetOwner() != c && from->GetOwner()->CastToClient()->GetAnon()) {
		c->Message(Chat::Yellow, "You name a bot you own to use this command.");
		return;
	}

	auto to = ActionableBots::AsNamed_ByBot(c, sep->arg[2]);

	if (!to) {
		c->Message(Chat::Yellow, "Could not find %s.", sep->arg[1]);
		return;
	}
	
	if (!to->IsBot()) {
		c->Message(Chat::Yellow, "%s is not a bot.", to->GetCleanName());
		return;
	}

	if (!to->GetOwner()) {
		c->Message(Chat::Yellow, "Could not find %s's owner.", to->GetCleanName());
	}
	
	if (to->GetOwner() != c) {
		c->Message(Chat::Yellow, "You must name a spawned bot that you own to use this command.");
		return;
	}

	if (to == from) {
		c->Message(Chat::Yellow, "You cannot copy to the same bot that you're copying from.");
		return;
	}

	int ab_arg = 4;
	std::string spell_type_arg = sep->arg[ab_arg];
	uint16 spell_type = UINT16_MAX;

	if (sep->IsNumber(ab_arg)) {
		spell_type = atoi(sep->arg[ab_arg]);

		if (!EQ::ValueWithin(spell_type, BotSpellTypes::START, BotSpellTypes::END)) {
			c->Message(
				Chat::Yellow,
				fmt::format(
					"You must choose a valid spell type. Use {} for information regarding this command.",
					Saylink::Silent(
						fmt::format("{} help", sep->arg[0])
					)
				).c_str()
			);

			return;
		}

		++ab_arg;
	}
	else if (!spell_type_arg.empty()) {
		if (Bot::GetSpellTypeIDByShortName(spell_type_arg) != UINT16_MAX) {
			spell_type = Bot::GetSpellTypeIDByShortName(spell_type_arg);
			++ab_arg;
		}
		else {
			if (
				!copy_all &&
				!copy_misc &&
				!copy_spell_settings &&
				!copy_blocked_buffs &&
				!copy_blocked_pet_buffs
			) {
				c->Message(Chat::Yellow, "If you are trying to specify a spell type, you must enter a valid spell type. Otherwise you can ignore this message.");
			}
		}
	}

	std::string output = "";

	if (copy_all) {
		for (uint16 i = BotBaseSettings::START; i <= BotBaseSettings::END; ++i) {
			to->SetBotBaseSetting(i, from->GetBotBaseSetting(i));
		}

		for (uint16 i = BotSettingCategories::START_NO_BASE; i <= BotSettingCategories::END; ++i) {
			for (uint16 x = BotSpellTypes::START; x <= BotSpellTypes::END; ++x) {
				to->SetBotSetting(i, x, from->GetSetting(i, x));
			}
		}

		from->CopyBotSpellSettings(to);
		from->CopyBotBlockedBuffs(to);
		from->CopyBotBlockedPetBuffs(to);
		output = "settings were";
	}
	else if (copy_misc) {
		for (uint16 i = BotBaseSettings::START; i <= BotBaseSettings::END; ++i) {
			to->SetBotBaseSetting(i, from->GetBotBaseSetting(i));
			output = "miscellaneous settings were";
		}
	}
	else if (copy_spell_settings) {
		from->ResetBotSpellSettings();
		output = "^spellsettings were";
	}
	else if (copy_spell_type_settings) {
		if (spell_type != UINT16_MAX) {
			for (uint16 i = BotSettingCategories::START_NO_BASE; i <= BotSettingCategories::END; ++i) {
				to->SetBotSetting(i, spell_type, from->GetSetting(i, spell_type));
			}

			output = fmt::format(
				"[{}] settings were",
				Bot::GetSpellTypeNameByID(spell_type)
			);
		}
		else {
			for (uint16 i = BotSettingCategories::START_NO_BASE; i <= BotSettingCategories::END; ++i) {
				for (uint16 x = BotSpellTypes::START; x <= BotSpellTypes::END; ++x) {
					to->SetBotSetting(i, x, from->GetSetting(i, x));
				}
			}

			output = "spell type settings were";
		}
	}
	else if (copy_blocked_buffs) {
		from->CopyBotBlockedBuffs(to);
	}
	else if (copy_blocked_pet_buffs) {
		from->CopyBotBlockedPetBuffs(to);
	}
	else if (setting_type != UINT16_MAX) {
		if (spell_type != UINT16_MAX) {
			to->SetBotSetting(setting_type, spell_type, from->GetSetting(setting_type, spell_type));
			output = fmt::format(
				"[{}] {} were",
				Bot::GetSpellTypeNameByID(spell_type),
				Bot::GetBotSpellCategoryName(Bot::GetBotSpellCategoryIDByShortName(option_arg))
			);
		}
		else {
			for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
				to->SetBotSetting(setting_type, i, from->GetSetting(setting_type, i));
			}

			output = fmt::format(
				"{} settings for all spell types",
				Bot::GetBotSpellCategoryName(Bot::GetBotSpellCategoryIDByShortName(option_arg))
			);
		}
	}

	to->Save();

	c->Message(
		Chat::Green,
		fmt::format(
			"{}'s {} copied to {}.",
			from->GetCleanName(),
			output,
			to->GetCleanName()
		).c_str()
	);
}
