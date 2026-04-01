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

void command_takeplatinum(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) { //must be a number
		c->Message(Chat::Red, "Usage: #takeplatinum [Platinum]");
		return;
	}

	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	uint32 platinum = Strings::ToUnsignedInt(sep->arg[1]);
	if (!platinum) {
		c->Message(Chat::Red, "Usage: #takeplatinum [Platinum]");
		return;
	}

	bool success = target->TakePlatinum(
		platinum,
		true
	);

	if (success) {
		c->Message(
			Chat::White,
			fmt::format(
				"Subtracted {} from {}.",
				Strings::Money(
					platinum,
					0,
					0,
					0
				),
				c->GetTargetDescription(target)
			).c_str()
		);
	}
	else {
		c->Message(
			Chat::Red,
			fmt::format(
				"Unable to subtract {} from {}.",
				Strings::Money(
					platinum,
					0,
					0,
					0
				),
				c->GetTargetDescription(target)
			).c_str()
		);
	}
}
