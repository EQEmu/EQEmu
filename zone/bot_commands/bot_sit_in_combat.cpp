#include "../bot_command.h"

void bot_command_sit_in_combat(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_sit_in_combat", sep->arg[0], "sitincombat")) {
		c->Message(Chat::White, "note: Toggles whether or not bots will sit in combat to heal or med.");

		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		BotCommandHelpParams p;

		p.description = { "Toggles whether or not bots will sit in combat to heal or med." };
		p.example_format = { fmt::format("{} [value] [actionable]", sep->arg[0]) };
		p.examples_one =
		{
			"To set Clerics to sit in combat:",
			fmt::format(
				"{} 1 byclass {}",
				sep->arg[0],
				Class::Cleric
			)
		};
		p.examples_two = 
		{
			"To set all bots to sit/med in combat:",
			fmt::format(
				"{} 1 spawned",
				sep->arg[0]
			)
		};
		p.examples_three =
		{
			"To check the sit in combat state for all bots:",
			fmt::format(
				"{} current spawned",
				sep->arg[0]
			)
		};
		p.actionables = { "target, byname, ownergroup, ownerraid, targetgroup, namesgroup, healrotationtargets, mmr, byclass, byrace, spawned" };

		std::string popup_text = c->SendBotCommandHelpWindow(p);
		popup_text = DialogueWindow::Table(popup_text);

		c->SendPopupToClient(sep->arg[0], popup_text.c_str());

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

	int ab_arg = 1;
	bool current_check = false;
	uint32 type_value = 0;

	if (sep->IsNumber(1)) {
		type_value = atoi(sep->arg[1]);
		++ab_arg;
		if (type_value < 0 || type_value > 1) {
			c->Message(Chat::Yellow, "You must enter either 0 for disabled or 1 for enabled.");

			return;
		}
	}
	else if (!arg1.compare("current")) {
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
					"{} says, 'I {} sit in combat.'",
					my_bot->GetCleanName(),
					my_bot->GetMedInCombat() ? "will" : "will not"
				).c_str()
			);
		}
		else {
			my_bot->SetMedInCombat(type_value);
			++success_count;
		}
	}
	if (!current_check) {
		if (success_count == 1 && first_found) {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} says, 'I {} sit in combat.'",
					first_found->GetCleanName(),
					first_found->GetMedInCombat() ? "will now" : "will no longer"
				).c_str()
			);
		}
		else {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} of your bots {} sit in combat.",
					success_count,
					type_value ? "will now" : "will no longer"
				).c_str()
			);
		}
	}
}
