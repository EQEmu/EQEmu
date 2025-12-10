#include "../bot_command.h"

void bot_command_spell_pursue_priority(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_spell_pursue_priority", sep->arg[0], "spellpursuepriority")) {
		c->Message(Chat::White, "note: Sets the order of spell casts when the mob is fleeing in combat by spell type.");

		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		BotCommandHelpParams p;

		p.description = { "Sets the order of spell casts when the mob is fleeing in combat by spell type." };
		p.notes =
		{
			"- Setting a spell type to 0 will prevent that type from being cast.",
			"- If 2 or more are set to the same priority they will sort by spell type ID."
		};
		p.example_format =
		{
			fmt::format("{} [Type Shortname] [value] [actionable]", sep->arg[0]),
			fmt::format("{} [Type ID] [value] [actionable]", sep->arg[0])
		};
		p.examples_one =
		{
			"To list your targeted bot's priorities:",
			fmt::format(
				"{} list",
				sep->arg[0]
			)
		};
		p.examples_two =
		{
			"To set all Shaman to not cast cures:",
			fmt::format(
				"{} {} 0 byclass {}",
				sep->arg[0],
				Bot::GetSpellTypeShortNameByID(BotSpellTypes::Cure),
				Class::Shaman
			),
			fmt::format(
				"{} {} 0 byclass {}",
				sep->arg[0],
				BotSpellTypes::Cure,
				Class::Shaman
			)
		};
		p.examples_three =
		{
			"To check the current pursue priority of buffs on all bots:",
			fmt::format(
				"{} {} current spawned",
				sep->arg[0],
				Bot::GetSpellTypeShortNameByID(BotSpellTypes::Buff)
			),
			fmt::format(
				"{} {} current spawned",
				sep->arg[0],
				BotSpellTypes::Buff
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
	bool list_check = false;
	uint16 spell_type = 0;
	uint32 type_value = 0;
	int ab_mask = ActionableBots::ABM_Type1;

	// String/Int type checks
	if (sep->IsNumber(1)) {
		spell_type = atoi(sep->arg[1]);

		if (!EQ::ValueWithin(spell_type, BotSpellTypes::START, BotSpellTypes::END)) {
			c->Message(Chat::Yellow, "You must choose a valid spell type. Spell types range from %i to %i", BotSpellTypes::START, BotSpellTypes::END);

			return;
		}
	}
	else {
		if (!arg1.compare("list")) {
			++ab_arg;
			list_check = true;
			ab_mask = ActionableBots::ABM_Target;

			if (
				!c->GetTarget() ||
				!c->GetTarget()->IsBot() ||
				c->GetTarget()->GetOwner() != c
			) {
				c->Message(Chat::Yellow, "You must target your own bot to list priorities.");
				return;
			}
		}
		else if (Bot::GetSpellTypeIDByShortName(arg1) != UINT16_MAX) {
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
		if (!EQ::ValueWithin(type_value, BotSpellTypes::START, BotSpellTypes::END)) {
			c->Message(Chat::Yellow, "You must enter a value between %u and %u.", BotSpellTypes::START, BotSpellTypes::END);

			return;
		}
	}
	else if (!arg2.compare("current")) {
		++ab_arg;
		current_check = true;
	}
	else if (!list_check) {
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
					"{} says, 'My [{}] pursue cast priority is currently [{}].'",
					my_bot->GetCleanName(),
					Bot::GetSpellTypeNameByID(spell_type),
					my_bot->GetSpellTypePriority(spell_type, BotPriorityCategories::Pursue)
				).c_str()
			);
		}
		else if (list_check) {
			auto cast_order = my_bot->GetSpellTypesPrioritized(BotPriorityCategories::Pursue);

			BotCommandHelpParams p;
			p.description = {
				"Anything not listed is currently disabled (0)",
				"----------",
				"Spell Type - Priority"
			};
			p.notes       = { };

			for (auto& current_cast : cast_order) {
				p.notes.push_back(
					fmt::format(
						"{}s - {}",
						Bot::GetSpellTypeNameByID(current_cast.spellType),
						(current_cast.priority == 0 ? "disabled (0)" : std::to_string(current_cast.priority))
					).c_str()
				);
			}

			std::string popup_text = c->SendBotCommandHelpWindow(p);
			popup_text = DialogueWindow::Table(popup_text);

			c->SendPopupToClient(sep->arg[0], popup_text.c_str());

			return;
		}
		else {
			my_bot->SetSpellTypePriority(spell_type, BotPriorityCategories::Pursue, type_value);
			++success_count;
		}
	}
	if (!current_check) {
		if (success_count == 1 && first_found) {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} says, 'My [{}] pursue cast priority was set to [{}].'",
					first_found->GetCleanName(),
					Bot::GetSpellTypeNameByID(spell_type),
					first_found->GetSpellTypePriority(spell_type, BotPriorityCategories::Pursue)
				).c_str()
			);
		}
		else {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} of your bots set their [{}] pursue cast priority to [{}].",
					success_count,
					Bot::GetSpellTypeNameByID(spell_type),
					type_value
				).c_str()
			);
		}
	}
}
