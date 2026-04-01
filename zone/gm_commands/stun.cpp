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

void command_stun(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #stun [Duration]");
		return;
	}

	Mob *target  = c;
	int duration = static_cast<int>(std::min(Strings::ToBigInt(sep->arg[1]), (int64) 2000000000));

	if (duration < 0) {
		duration = 0;
	}

	if (c->GetTarget()) {
		target = c->GetTarget();
		if (target->IsClient()) {
			target->CastToClient()->Stun(duration);
		} else if (target->IsNPC()) {
			target->CastToNPC()->Stun(duration);
		}
	} else {
		c->Stun(duration);
	}

	std::string stun_message = (
		duration ?
			fmt::format(
				"You stunned {} for {}.",
				c->GetTargetDescription(target),
				Strings::MillisecondsToTime(duration)
			) :
			fmt::format(
				"You unstunned {}.",
				c->GetTargetDescription(target)
			)
	);
	c->Message(
		Chat::White,
		stun_message.c_str()
	);
}


