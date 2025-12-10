#include "../bot_command.h"

void bot_command_spell_holds(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_spell_holds", sep->arg[0], "spellholds")) {
		c->Message(Chat::White, "note: Toggles whether or not bots can cast certain spell types.");

		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		BotCommandHelpParams p;

		p.description = { "Toggles whether or not bots can cast certain spell types." };
		p.notes = { "- All pet types are based off the pet owner's setting when a pet is the target" };
		p.example_format =
		{
			fmt::format("{} [Type Shortname] [value] [actionable]", sep->arg[0]),
			fmt::format("{} [Type ID] [value] [actionable]", sep->arg[0])
		};
		p.examples_one =
		{ 
			"To set all bots to hold DoTs:", 
			fmt::format(
				"{} {} 1 spawned",
				sep->arg[0],
				Bot::GetSpellTypeShortNameByID(BotSpellTypes::DOT)
			),
			fmt::format(
				"{} {} 1 spawned",
				sep->arg[0],
				BotSpellTypes::DOT
			)
		};
		p.examples_two =
		{
			"To check the current DoT settings on all bots:",
			fmt::format(
				"{} {} current spawned",
				sep->arg[0],
				Bot::GetSpellTypeShortNameByID(BotSpellTypes::DOT)
			),
			fmt::format(
				"{} {} current spawned",
				sep->arg[0],
				BotSpellTypes::DOT
			)
		};
		p.actionables = { "target, byname, ownergroup, ownerraid, targetgroup, namesgroup, healrotationtargets, mmr, byclass, byrace, spawned"  };

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
		if (type_value < 0 || type_value > 1) {
			c->Message(Chat::Yellow, "You must enter either 0 for disabled or 1 for enabled.");

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
		if (!first_found) {
			first_found = my_bot;
		}
		if (current_check) {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} says, 'My [{}] spell hold is currently [{}].'",
					my_bot->GetCleanName(),
					Bot::GetSpellTypeNameByID(spell_type),
					my_bot->GetSpellTypeHold(spell_type) ? "enabled" : "disabled"
				).c_str()
			);
		}
		else {
			my_bot->SetSpellTypeHold(spell_type, type_value);
			++success_count;
		}
	}
	if (!current_check) {
		if (success_count == 1 && first_found) {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} says, 'My [{}] spell hold was [{}].'",
					first_found->GetCleanName(),
					Bot::GetSpellTypeNameByID(spell_type),
					first_found->GetSpellTypeHold(spell_type) ? "enabled" : "disabled"
				).c_str()
			);
		}
		else {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} of your bots [{}] their [{}] spell hold.",
					success_count,
					type_value ? "enabled" : "disabled",
					Bot::GetSpellTypeNameByID(spell_type)
				).c_str()
			);
		}
	}
}
