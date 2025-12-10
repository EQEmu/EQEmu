#include "../bot_command.h"

void bot_command_spell_resist_limits(Client* c, const Seperator* sep) {
	if (helper_command_alias_fail(c, "bot_command_spell_resist_limits", sep->arg[0], "spellresistlimits")) {
		c->Message(Chat::White, "note: Sets the limit of a target's resists to where a bot won't attempt to cast due to resist chances.");

		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		BotCommandHelpParams p;

		p.description = { "Sets the limit of a target's resists to where a bot won't attempt to cast due to resist chances." };
		p.example_format =
		{
			fmt::format("{} [Type Shortname] [value] [actionable]", sep->arg[0]),
			fmt::format("{} [Type ID] [value] [actionable]", sep->arg[0])
		};
		p.examples_one =
		{
			"To set all bots' slow resist limit to 250:",
			fmt::format(
				"{} {} 250 spawned",
				sep->arg[0],
				Bot::GetSpellTypeShortNameByID(BotSpellTypes::Slow)
			),
			fmt::format(
				"{} {} 250 spawned",
				sep->arg[0],
				BotSpellTypes::Slow
			)
		};
		p.examples_two =
		{
			"To set Magicians to limit their resist to 175 for nukes:",
			fmt::format(
				"{} {} 175 byclass {}",
				sep->arg[0],
				Bot::GetSpellTypeShortNameByID(BotSpellTypes::Nuke),
				Class::Magician
			),
			fmt::format(
				"{} {} 175 byclass {}",
				sep->arg[0],
				BotSpellTypes::Nuke,
				Class::Magician

			)
		};
		p.examples_three =
		{
			"To check the current debuff resist limit on all bots:",
			fmt::format(
				"{} {} current spawned",
				sep->arg[0],
				Bot::GetSpellTypeShortNameByID(BotSpellTypes::Debuff)
			),
			fmt::format(
				"{} {} current spawned",
				sep->arg[0],
				BotSpellTypes::Debuff
			)
		};
		p.actionables = { "target, byname, ownergroup, ownerraid, targetgroup, namesgroup, healrotationtargets, mmr, byclass, byrace, spawned" };

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

	std::string arg1 = sep->arg[1];
	std::string arg2 = sep->arg[2];
	int ab_arg = 2;
	bool current_check = false;
	uint16 spell_type = 0;
	uint32 type_value = 0;

	// String/Int type checks
	if (sep->IsNumber(1)) {
		spell_type = atoi(sep->arg[1]);

		if (!EQ::ValueWithin(spell_type, BotSpellTypes::START, BotSpellTypes::END)) {
			c->Message(Chat::Yellow, "You must choose a valid spell type. Spell types range from %i to %i", BotSpellTypes::START, BotSpellTypes::END);

			return;
		}
	}
	else {
		if (Bot::GetSpellTypeIDByShortName(arg1) != UINT16_MAX) {
			spell_type = Bot::GetSpellTypeIDByShortName(arg1);
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

		if (type_value < 0 || type_value > 1000) {
			c->Message(Chat::Yellow, "You must enter a value between 1-1000.");

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
					"{} says, 'My [{}] resist limit is currently [{}].'",
					my_bot->GetCleanName(),
					Bot::GetSpellTypeNameByID(spell_type),
					my_bot->GetSpellTypeResistLimit(spell_type)
				).c_str()
			);
		}
		else {
			my_bot->SetSpellTypeResistLimit(spell_type, type_value);
			++success_count;
		}
	}
	if (!current_check) {
		if (success_count == 1 && first_found) {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} says, 'My [{}] resist limit was set to [{}].'",
					first_found->GetCleanName(),
					Bot::GetSpellTypeNameByID(spell_type),
					first_found->GetSpellTypeResistLimit(spell_type)
				).c_str()
			);
		}
		else {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} of your bots set their [{}] resist limit to [{}].",
					success_count,
					Bot::GetSpellTypeNameByID(spell_type),
					type_value
				).c_str()
			);
		}
	}
}
