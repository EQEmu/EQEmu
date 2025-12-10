#include "../bot_command.h"

void bot_command_default_settings(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_default_settings", sep->arg[0], "defaultsettings")) {
		c->Message(Chat::White, "note: Restores a bot's setting(s) to defaults.");

		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		BotCommandHelpParams p;

		p.description = { "Restores a bot's setting(s) to defaults." };
		p.notes = { "- You can put a spell type ID or shortname after any option except [all], [misc] and [spellsettings] to restore that specifc spell type only"};
		p.example_format = { fmt::format("{} [option] [optional: spelltype id/short name] [actionable]", sep->arg[0]) };
		p.examples_one =
		{
			"To restore delays for Clerics:",
			fmt::format(
				"{} delays byclass {}",
				sep->arg[0],
				Class::Cleric
			)
		};
		p.examples_two =
		{
			"To restore only Snare delays for BotA:",
			fmt::format(
				"{} delays {} byname BotA",
				sep->arg[0],
				Bot::GetSpellTypeShortNameByID(BotSpellTypes::Snare)
			),
			fmt::format(
				"{} delays {} byname BotA",
				sep->arg[0],
				BotSpellTypes::Snare
			)
		};
		p.actionables = { "target, byname, ownergroup, ownerraid, targetgroup, namesgroup, healrotationtargets, mmr, byclass, byrace, spawned" };
		p.options = { "all, misc, spellsettings, spelltypesettings, spellholds, spelldelays, spellminthresholds, spellmaxthresholds, spellresistlimits, spellaggrocheck, spellminmanapct, spellmaxmanapct, spellminhppct, spellmaxhppct, spellidlepriority, spellengagedpriority, spellpursuepriority, spelltargetcounts, spellannouncecasts" };
		p.options_one =
		{
			"[spellsettings] will restore ^spellsettings options",
			"[spelltypesettings] restores all spell type settings",
			"[all] restores all settings"
		};
		p.options_two =
		{
			"[misc] restores all miscellaneous options such as:",
			"- ^showhelm, ^followd, ^stopmeleelevel, ^enforcespellsettings, ^bottoggleranged, ^petsettype, ^behindmob, ^distanceranged, ^illusionblock, ^sitincombat, ^sithppercent and ^sitmanapercent",
		};
		p.options_three = 
		{ 
			"<br>",
			"**The remaining options restore that specific type**" 
		};

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
		"spellannouncecasts"
	};

	std::string option_arg = Strings::ToLower(sep->arg[1]);
	bool valid_option = false;
	bool default_all = false;
	bool default_misc = false;
	bool default_spell_settings = false;
	bool default_spell_type_settings = false;
	uint16 setting_type = UINT16_MAX;

	for (int i = 0; i < options.size(); i++) {
		if (option_arg == options[i]) {
			if (option_arg == "all") {
				default_all = true;
				valid_option = true;

				break;
			}
			else if (option_arg == "misc") {
				default_misc = true;
				valid_option = true;

				break;
			}
			else if (option_arg == "spellsettings") {
				default_spell_settings = true;
				valid_option = true;

				break;
			}
			else if (option_arg == "spelltypesettings") {
				default_spell_type_settings = true;
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

	int ab_arg = 2;
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
			if (!default_all && !default_misc && !default_spell_settings) {
				c->Message(Chat::Yellow, "If you are trying to specify a spell type, you must enter a valid spell type. Otherwise you can ignore this message.");
			}
		}
	}

	const int ab_mask = ActionableBots::ABM_Type1;
	std::string class_race_arg = sep->arg[ab_arg];
	bool class_race_check = false;

	if (!class_race_arg.compare("byclass") || !class_race_arg.compare("byrace")) {
		class_race_check = true;
	}

	std::vector<Bot*> sbl;

	if (ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, !class_race_check ? sep->arg[ab_arg + 1] : nullptr, class_race_check ? atoi(sep->arg[ab_arg + 1]) : 0) == ActionableBots::ABT_None) {
		return;
	}

	sbl.erase(std::remove(sbl.begin(), sbl.end(), nullptr), sbl.end());

	Bot* first_found = nullptr;
	int success_count = 0;
	std::string output = "";
	uint8 bot_stance = 2;

	for (auto my_bot : sbl) {
		if (!first_found) {
			first_found = my_bot;
		}

		bot_stance = my_bot->GetBotStance();
		
		if (default_all) {
			for (uint16 i = BotBaseSettings::START; i <= BotBaseSettings::END; ++i) {
				my_bot->SetBotBaseSetting(i, my_bot->GetDefaultBotBaseSetting(i, bot_stance));
			}
			
			for (uint16 i = BotSettingCategories::START_NO_BASE; i <= BotSettingCategories::END; ++i) {
				for (uint16 x = BotSpellTypes::START; x <= BotSpellTypes::END; ++x) {
					my_bot->SetBotSetting(i, x, my_bot->GetDefaultSetting(i, x, bot_stance));
				}
			}

			my_bot->ResetBotSpellSettings();
			my_bot->ClearBotBlockedBuffs();
			output = "settings were reset";
		}
		else if (default_misc) {
			for (uint16 i = BotBaseSettings::START; i <= BotBaseSettings::END; ++i) {
				my_bot->SetBotBaseSetting(i, my_bot->GetDefaultBotBaseSetting(i, bot_stance));
				output = "miscellaneous settings were reset";
			}
		}
		else if (default_spell_settings) {
			my_bot->ResetBotSpellSettings();
			output = "^spellsettings were reset";
		}
		else if (default_spell_type_settings) {
			if (spell_type != UINT16_MAX) {
				for (uint16 i = BotSettingCategories::START_NO_BASE; i <= BotSettingCategories::END; ++i) {
					my_bot->SetBotSetting(i, spell_type, my_bot->GetDefaultSetting(i, spell_type, bot_stance));
				}

				output = fmt::format(
					"[{}] settings were reset",
					Bot::GetSpellTypeNameByID(spell_type)
				);
			}
			else {
				for (uint16 i = BotSettingCategories::START_NO_BASE; i <= BotSettingCategories::END; ++i) {
					for (uint16 x = BotSpellTypes::START; x <= BotSpellTypes::END; ++x) {
						my_bot->SetBotSetting(i, x, my_bot->GetDefaultSetting(i, x, bot_stance));
					}
				}
				
				output = "spell type settings were reset";
			}
		}
		else if (setting_type != UINT16_MAX) {
			if (spell_type != UINT16_MAX) {
				my_bot->SetBotSetting(setting_type, spell_type, my_bot->GetDefaultSetting(setting_type, spell_type, bot_stance));
				output = fmt::format(
					"[{}] {} were reset",
					Bot::GetSpellTypeNameByID(spell_type),
					Bot::GetBotSpellCategoryName(Bot::GetBotSpellCategoryIDByShortName(option_arg))
				);
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					my_bot->SetBotSetting(setting_type, i, my_bot->GetDefaultSetting(setting_type, i, bot_stance));
				}
				
				output = fmt::format(
					"{} settings were reset for all spell types",
					Bot::GetBotSpellCategoryName(Bot::GetBotSpellCategoryIDByShortName(option_arg))
				);
			}
		}

		my_bot->Save();
		++success_count;
	}

	if (success_count == 1) {
		c->Message(
			Chat::Green,
			fmt::format(
				"{} says, 'My {}.'",
				first_found->GetCleanName(),
				output
			).c_str()
		);
	}
	else {
		c->Message(
			Chat::Green,
			fmt::format(
				"{} of your bots' {}.",
				success_count,
				output
			).c_str()
		);
	}
}
