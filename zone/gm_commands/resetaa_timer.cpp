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
