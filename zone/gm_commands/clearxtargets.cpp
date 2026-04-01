/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
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
