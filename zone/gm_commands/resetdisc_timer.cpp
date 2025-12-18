#include "zone/client.h"

void command_resetdisc_timer(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #resetdisc_timer all - Reset all Discipline timers");
		c->Message(Chat::White, "Usage: #resetdisc_timer [Timer ID] - Reset Discipline timer by ID");
		return;
	}

	auto target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	bool is_all = !strcasecmp(sep->arg[1], "all");

	if (is_all) {
		c->Message(
			Chat::White,
			fmt::format(
				"Reset all Discipline timers for {}.",
				c->GetTargetDescription(target)
			).c_str()
		);
		target->ResetAllDisciplineTimers();
		return;
	}

	if (sep->IsNumber(1)) {
		auto timer_id = Strings::ToUnsignedInt(sep->arg[1]);
		c->Message(
			Chat::White,
			fmt::format(
				"Reset Discipline timer {} for {}.",
				timer_id,
				c->GetTargetDescription(target)
			).c_str()
		);
		target->ResetDisciplineTimer(timer_id);
	}
}

