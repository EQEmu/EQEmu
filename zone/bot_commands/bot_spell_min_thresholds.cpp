#include "../bot_command.h"

void bot_command_spell_min_thresholds(Client* c, const Seperator* sep) {
	if (helper_command_alias_fail(c, "bot_command_spell_min_thresholds", sep->arg[0], "spellminthresholds")) {
		c->Message(Chat::White, "note: Controls at what target HP % the bot will stop casting different spell types.");

		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		BotCommandHelpParams p;

		p.description = { "Controls at what target HP % the bot will stop casting different spell types." };
		p.notes =
		{
			"- Targeting yourself for this command will allow you to control your own settings for how bots cast on you",
			"- All pet types are based off the pet's owner's setting",
			"- Any remaining types use the owner's setting when a pet is the target",
			"- All Heals, Cures, Buffs (DS and resists included) are based off the target's setting, not the caster",
			"- e.g., BotA is healing BotB using BotB's settings",
		};
		p.example_format =
		{
			fmt::format("{} [Type Shortname] [value] [actionable, default: target]", sep->arg[0]),
			fmt::format("{} [Type ID] [value] [actionable, default: target]", sep->arg[0])
		};
		p.examples_one =
		{
			"To set all bots to stop debuffing at 10%:",
			fmt::format(
				"{} {} 10 spawned",
				sep->arg[0],
				Bot::GetSpellTypeShortNameByID(BotSpellTypes::Debuff)
			),
			fmt::format(
				"{} {} 10 spawned",
				sep->arg[0],
				BotSpellTypes::Debuff
			)
		};
		p.examples_two =
		{
			"To set all Druids to stop casting DoTs at 15%:",
			fmt::format(
				"{} {} 15 byclass {}",
				sep->arg[0],
				Bot::GetSpellTypeShortNameByID(BotSpellTypes::DOT),
				Class::Druid
			),
			fmt::format(
				"{} {} 15 byclass {}",
				sep->arg[0],
				BotSpellTypes::DOT,
				Class::Druid
			)
		};
		p.examples_three =
		{
			"To check the current Fast Heal min threshold on all bots:",
			fmt::format(
				"{} {} current spawned",
				sep->arg[0],
				Bot::GetSpellTypeShortNameByID(BotSpellTypes::FastHeals)
			),
			fmt::format(
				"{} {} current spawned",
				sep->arg[0],
				BotSpellTypes::FastHeals
			)
		};
		p.actionables = { "target, byname, ownergroup, ownerraid, targetgroup, namesgroup, healrotationtargets, mmr, byclass, byrace, spawned" };

		std::string popup_text = c->SendBotCommandHelpWindow(p);
		popup_text = DialogueWindow::Table(popup_text);

		c->SendPopupToClient(sep->arg[0], popup_text.c_str());
		c->SendSpellTypePrompts();
		c->SendSpellTypePrompts(false, true);

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

	std::string arg1 = sep->arg[1];
	std::string arg2 = sep->arg[2];
	int ab_arg = 2;
	bool current_check = false;
	uint16 spell_type = 0;
	uint32 type_value = 0;
	Mob* target = c->GetTarget();
	bool clientSetting = (target && target == c);

	if (sep->IsNumber(1)) {
		spell_type = atoi(sep->arg[1]);

		if (
			(clientSetting && !IsClientBotSpellType(spell_type)) ||
			(!clientSetting && !EQ::ValueWithin(spell_type, BotSpellTypes::START, BotSpellTypes::END))
		) {
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{}. Use {} for information regarding this command.",
					(!clientSetting ? "You must choose a valid spell type" : "You must choose a valid client spell type"),
					Saylink::Silent(
						fmt::format("{} help", sep->arg[0])
					)
				).c_str()
			);

			if (clientSetting) {
				c->SendSpellTypePrompts(false, true);
			}

			return;
		}
	}
	else {
		if (Bot::GetSpellTypeIDByShortName(arg1) != UINT16_MAX) {
			spell_type = Bot::GetSpellTypeIDByShortName(arg1);

			if (clientSetting && !IsClientBotSpellType(spell_type)) {
				c->Message(Chat::Yellow, "Invalid spell type for clients.");
				c->SendSpellTypePrompts(false, true);
			}
		}
		else {
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
	}

	if (sep->IsNumber(2)) {
		type_value = atoi(sep->arg[2]);
		++ab_arg;
		if (type_value < 0 || type_value > 100) {
			c->Message(Chat::Yellow, "You must enter a value between 0-100 (0%% to 100%% of health).");

			return;
		}
	}
	else if (!arg2.compare("current")) {
		++ab_arg;
		current_check = true;
	}
	else {
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

	const int ab_mask = ActionableBots::ABM_Type1;
	std::string actionable_arg = sep->arg[ab_arg];

	if (actionable_arg.empty()) {
		actionable_arg = "target";
	}

	if (!clientSetting) {

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

		Bot* first_found = nullptr;
		int success_count = 0;

		for (auto my_bot : sbl) {
			if (my_bot->BotPassiveCheck()) {
				continue;
			}

			if (!first_found) {
				first_found = my_bot;
			}

			if (current_check) {
				c->Message(
					Chat::Green,
					fmt::format(
						"{} says, 'My [{}] minimum threshold is currently [{}%%].'",
						my_bot->GetCleanName(),
						Bot::GetSpellTypeNameByID(spell_type),
						my_bot->GetSpellTypeMinThreshold(spell_type)
					).c_str()
				);
			}
			else {
				my_bot->SetSpellTypeMinThreshold(spell_type, type_value);
				++success_count;
			}
		}

		if (!current_check) {
			if (success_count == 1 && first_found) {
				c->Message(
					Chat::Green,
					fmt::format(
						"{} says, 'My [{}] minimum threshold was set to [{}%%].'",
						first_found->GetCleanName(),
						Bot::GetSpellTypeNameByID(spell_type),
						first_found->GetSpellTypeMinThreshold(spell_type)
					).c_str()
				);
			}
			else {
				c->Message(
					Chat::Green,
					fmt::format(
						"{} of your bots set their [{}] minimum threshold to [{}%%].",
						success_count,
						Bot::GetSpellTypeNameByID(spell_type),
						type_value
					).c_str()
				);
			}
		}
	}
	else {
		if (current_check) {
			c->Message(
				Chat::Green,
				fmt::format(
					"Your [{}] minimum threshold is currently [{}%%].",
					Bot::GetSpellTypeNameByID(spell_type),
					c->GetSpellTypeMinThreshold(spell_type)
				).c_str()
			);
		}
		else {
			c->SetSpellTypeMinThreshold(spell_type, type_value);

			c->Message(
				Chat::Green,
				fmt::format(
					"Your [{}] minimum threshold was set to [{}%%].",
					Bot::GetSpellTypeNameByID(spell_type),
					c->GetSpellTypeMinThreshold(spell_type)
				).c_str()
			);
		}
	}
}
