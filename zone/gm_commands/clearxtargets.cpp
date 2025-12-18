#include "zone/client.h"

void command_clearxtargets(Client *c, const Seperator *sep)
{
	const int reuse_timer = RuleI(Character, ClearXTargetDelay);

	const int arguments = sep->argnum;
	if (arguments) {
		const bool is_help = !strcasecmp(sep->arg[1], "help");

		if (is_help) {
			c->Message(Chat::White, "Usage: #clearxtargets");
			c->Message(
				Chat::White,
				"Note: Use this if your Extended Target window is bugged or has lingering targets that are invalid."
			);
			if (reuse_timer) {
				c->Message(
					Chat::White,
					fmt::format(
						"Note: This can only be used every {}.",
						Strings::SecondsToTime(reuse_timer)
					).c_str()
				);
			}
			return;
		}
	}

	if (reuse_timer) {
		const uint32 time_left = c->GetPTimers().GetRemainingTime(pTimerClearXTarget);
		if (!c->GetPTimers().Expired(&database, pTimerClearXTarget, false)) {
			c->Message(
				Chat::White,
				fmt::format(
					"You must wait {} before using this command again.",
					Strings::SecondsToTime(time_left)
				).c_str()
			);
			return;
		}
	}

	c->ClearXTargets();
	c->Message(Chat::White, "Extended Target window has been cleared.");

	if (reuse_timer) {
		c->GetPTimers().Start(pTimerClearXTarget, reuse_timer);
	}
}
