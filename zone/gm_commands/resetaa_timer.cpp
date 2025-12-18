#include "zone/client.h"

void command_resetaa_timer(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #resetaa_timer all - Reset all Alternate Advancement timers");
		c->Message(Chat::White, "Usage: #resetaa_timer [Timer ID] - Reset Alternate Advancement timer by ID");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const bool is_all = !strcasecmp(sep->arg[1], "all");

	if (is_all) {
		c->Message(
			Chat::White,
			fmt::format(
				"Reset all Alternate Advancement timers for {}.",
				c->GetTargetDescription(t)
			).c_str()
		);

		t->ResetAlternateAdvancementTimers();

		return;
	} else if (sep->IsNumber(1)) {
		const auto timer_id = Strings::ToInt(sep->arg[1]);

		c->Message(
			Chat::White,
			fmt::format(
				"Reset Alternate Advancement timer {} for {}.",
				timer_id,
				c->GetTargetDescription(t)
			).c_str()
		);

		t->ResetAlternateAdvancementTimer(timer_id);
	}
}
