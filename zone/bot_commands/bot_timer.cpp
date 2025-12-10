#include "../bot_command.h"

void bot_command_timer(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_timer", sep->arg[0], "timer")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s [clear | has | set] [disc | item | spell] [timer ID | item ID | spell ID | all] [optional ms for set] ([actionable: target | byname | ownergroup | ownerraid | targetgroup | namesgroup | healrotationtargets | mmr | byclass | byrace | spawned] ([actionable_name])).", sep->arg[0]);
		c->Message(Chat::White, "When setting, you can leave the value blank to use the default for the item or specify a value in ms to set the timer to.");
		c->Message(Chat::White, "Returns or sets the provided timer(s) for the selected bot(s) or clears the selected timer(s) for the selected bot(s).");
		return;
	}

	const int ab_mask = ActionableBots::ABM_Type1;

	std::string arg1 = sep->arg[1];
	std::string arg2 = sep->arg[2];
	std::string arg3 = sep->arg[3];
	int ab_arg = 4;
	bool clear = false;
	bool has = false;
	bool set = false;
	bool disc = false;
	bool item = false;
	bool spell = false;
	uint32 timer_id = 0;
	uint32 timer_value = 0;
	bool all = false;

	if (!arg1.compare("clear")) {
		clear = true;
	}
	else if (!arg1.compare("has")) {
		has = true;
	}
	else if (!arg1.compare("set")) {
		set = true;
	}
	else {
		c->Message(Chat::White, "Incorrect argument, use %s help for a list of options.", sep->arg[0]);
		return;
	}

	if (!arg2.compare("disc")) {
		disc = true;
	}
	else if (!arg2.compare("item")) {
		item = true;
	}
	else if (!arg2.compare("spell")) {
		spell = true;
	}
	else {
		c->Message(Chat::White, "Incorrect timer type, use %s help for a list of options.", sep->arg[0]);
		return;
	}

	if (sep->IsNumber(3)) {
		timer_id = atoi(sep->arg[3]);
		if (timer_id < 0) {
			c->Message(Chat::White, "You cannot use negative numbers.");
			return;
		}
	}
	else if (!arg3.compare("all")) {
		if (has || set) {
			c->Message(Chat::White, "You can only use 'all' for clearing timers.");
			return;
		}

		all = true;
	}
	else {
		c->Message(Chat::White, "Incorrect ID option, use %s help for a list of options.", sep->arg[0]);
		return;
	}

	if (set) {
		if (sep->IsNumber(4)) {
			ab_arg = 5;
			timer_value = atoi(sep->arg[4]);
			if (timer_value <= 0) {
				c->Message(Chat::White, "You cannot use 0 or negative numbers.");
				return;
			}
		}
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

	for (auto my_bot : sbl) {
		bool found = false;

		if (clear) {
			c->Message(
				Chat::White,
				fmt::format(
					"{} says, 'Clearing {} timer{}'",
					my_bot->GetCleanName(),
					disc ? "Discipline" : item ? "Item" : "Spell",
					(all ? "s." : ".")
				).c_str()
			);

			if (disc) {
				my_bot->ClearDisciplineReuseTimer(timer_id);
			}
			else if (item) {
				my_bot->ClearItemReuseTimer(timer_id);
			}
			else if (spell) {
				my_bot->ClearSpellRecastTimer(timer_id);
			}
		}
		else if (has) {
			uint32 remaining_time;
			std::string time_string = "";

			if (disc) {
				if (!my_bot->CheckDisciplineReuseTimer(timer_id)) {
					remaining_time = my_bot->GetDisciplineReuseRemainingTime(timer_id) / 1000;
					time_string = Strings::SecondsToTime(remaining_time);
					found = true;
				}
			}
			else if (item) {
				if (!my_bot->CheckItemReuseTimer(timer_id)) {
					remaining_time = my_bot->GetItemReuseRemainingTime(timer_id) / 1000;
					time_string = Strings::SecondsToTime(remaining_time);
					found = true;
				}
			}
			else if (spell) {
				if (!my_bot->CheckSpellRecastTimer(timer_id)) {
					remaining_time = my_bot->GetSpellRecastRemainingTime(timer_id) / 1000;
					time_string = Strings::SecondsToTime(remaining_time);
					found = true;
				}
			}

			c->Message(
				Chat::White,
				fmt::format(
					"{} says, 'I {}{}{}'",
					my_bot->GetCleanName(),
					(!found ? " do not have that timer currently" : " have "),
					(!found ? "" : time_string),
					(!found ? "." : " remaining.")
				).c_str()
			);
		}
		else if (set) {
			c->Message(
				Chat::White,
				fmt::format(
					"{} says, 'Setting {} timer{} for {} to {}.'",
					my_bot->GetCleanName(),
					disc ? "Discipline" : item ? "Item" : "Spell",
					(all ? "s" : ""),
					timer_id,
					timer_value ? std::to_string(timer_value) : "the default value"
				).c_str()
			);

			if (disc) {
				my_bot->ClearDisciplineReuseTimer(timer_id);
				my_bot->SetDisciplineReuseTimer(timer_id, timer_value);
			}
			else if (item) {
				my_bot->ClearItemReuseTimer(timer_id);
				my_bot->SetItemReuseTimer(timer_id, timer_value);
			}
			else if (spell) {
				my_bot->ClearSpellRecastTimer(timer_id);
				my_bot->SetSpellRecastTimer(timer_id, timer_value);
			}
		}
	}
}
