#include "../bot_command.h"

void bot_command_guard(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_guard", sep->arg[0], "guard")) {
		return;
	}
	if (helper_is_help_or_usage(sep->arg[1])) {

		c->Message(Chat::White, "usage: %s ([option: clear]) [actionable: byname | ownergroup | ownerraid | namesgroup | healrotation | mmr | byclass | byrace | default: spawned] ([actionable_name])", sep->arg[0]);
		return;
	}
	const int ab_mask = (ActionableBots::ABM_Target | ActionableBots::ABM_Type2);

	bool clear = false;
	int ab_arg = 1;
	int name_arg = 2;

	std::string clear_arg = sep->arg[1];
	if (!clear_arg.compare("clear")) {

		clear = true;
		++ab_arg;
		++name_arg;
	}

	std::string class_race_arg = sep->arg[ab_arg];
	bool class_race_check = false;
	if (!class_race_arg.compare("byclass") || !class_race_arg.compare("byrace")) {
		class_race_check = true;
	}

	std::vector<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, !class_race_check ? sep->arg[name_arg] : nullptr, class_race_check ? atoi(sep->arg[name_arg]) : 0) == ActionableBots::ABT_None) {
		return;
	}

	sbl.erase(std::remove(sbl.begin(), sbl.end(), nullptr), sbl.end());
	for (auto bot_iter : sbl) {

		if (clear) {
			bot_iter->SetGuardFlag(false);
		}
		else {
			bot_iter->SetGuardMode();
		}
	}

	if (sbl.size() == 1) {
		sbl.front()->RaidGroupSay(
			fmt::format(
				"{}uarding this position.",
				clear ? "No longer g" : "G"
			).c_str()
		);
	} else {
		c->Message(Chat::White, "%i of your bots are %sguarding their positions.", sbl.size(), (clear ? "no longer " : ""));
	}
}
