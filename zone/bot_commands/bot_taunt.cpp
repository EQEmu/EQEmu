#include "../bot_command.h"

void bot_command_taunt(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_taunt", sep->arg[0], "taunt")) {
		c->Message(Chat::White, "note: TAllows you to turn on/off the taunting state of your bots and/or their pets.");

		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		BotCommandHelpParams p;

		p.description = { "Allows you to turn on/off the taunting state of your bots and/or their pets." };
		p.notes = { "- If a taunting class is set to Aggressive (5) stance, they will automatically taunt and don't need to have it enabled manually." };
		p.example_format = { fmt::format("{} [on / off / pet] [optional: pet]  [actionable, default: target]", sep->arg[0]) };
		p.examples_one =
		{
			"To turn off taunt on all bots:",
			fmt::format(
				"{} off spawned",
				sep->arg[0]
			)
		};
		p.examples_two =
		{
			"To turn on taunt on all bots' pets:",
			fmt::format(
				"{} on pet spawned",
				sep->arg[0]
			)
		};
		p.examples_three =
		{
			"To turn on taunt for all ShadowKnights:",
			fmt::format(
				"{} on byclass {}",
				sep->arg[0],
				Class::ShadowKnight
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
	std::string arg2 = sep->arg[2];

	bool taunt_state = false;
	bool pet_taunt = false;
	bool valid_option = false;

	int ab_arg = 1;

	if (!arg1.compare("on")) {
		taunt_state = true;
		valid_option = true;
		++ab_arg;
	}
	else if (!arg1.compare("off")) {
		valid_option = true;
		++ab_arg;
	}

	if (!arg2.compare("pet")) {
		pet_taunt = true;
		valid_option = true;
		++ab_arg;
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

	const int ab_mask = ActionableBots::ABM_Type1;
	std::string actionable_arg = sep->arg[ab_arg];

	if (actionable_arg.empty()) {
		actionable_arg = "target";
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

	int bot_taunting_count = 0;
	int pet_taunting_count = 0;

	if (!pet_taunt) {
		for (auto bot_iter : sbl) {
			if (!bot_iter->GetSkill(EQ::skills::SkillTaunt)) {
				continue;
			}

			bot_iter->SetTaunting(taunt_state);

			bot_iter->RaidGroupSay(
				fmt::format(
					"I am {} taunting.",
					bot_iter->IsTaunting() ? "now" : "no longer"
				).c_str()
			);

			++bot_taunting_count;
		}
	}

	if (pet_taunt) {
		for (auto bot_iter : sbl) {
			if (!bot_iter->HasPet()) {
				continue;
			}

			if (!bot_iter->GetPet()->GetSkill(EQ::skills::SkillTaunt)) {
				continue;
			}

			bot_iter->GetPet()->CastToNPC()->SetTaunting(taunt_state);

			bot_iter->RaidGroupSay(
				fmt::format(
					"My Pet is {} taunting.",
					bot_iter->GetPet()->CastToNPC()->IsTaunting() ? "now" : "no longer"
				).c_str()
			);

			++pet_taunting_count;
		}
	}

	if (bot_taunting_count || pet_taunting_count) {
		c->Message(
			Chat::Green,
			fmt::format(
				"{} of your {} are {} taunting.",
				(bot_taunting_count ? bot_taunting_count : pet_taunting_count),
				(bot_taunting_count ? "bots" : "bots' pets"),
				taunt_state ? "now" : "no longer"
			).c_str()
		);
	}
	else {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"None of your {} are capable of taunting.",
				!pet_taunt ? "bots" : "bots' pets"
			).c_str()
		);
	}
}
