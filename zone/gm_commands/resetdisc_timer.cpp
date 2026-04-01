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

