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

void command_givemoney(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) { //as long as the first one is a number, we'll just let atoi convert the rest to 0 or a number
		c->Message(Chat::Red, "Usage: #givemoney [Platinum] [Gold] [Silver] [Copper]");
		return;
	}


	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	uint32 platinum = Strings::ToUnsignedInt(sep->arg[1]);
	uint32 gold = sep->IsNumber(2) ? Strings::ToUnsignedInt(sep->arg[2]) : 0;
	uint32 silver = sep->IsNumber(3) ? Strings::ToUnsignedInt(sep->arg[3]) : 0;
	uint32 copper = sep->IsNumber(4) ? Strings::ToUnsignedInt(sep->arg[4]) : 0;
	if (!platinum && !gold && !silver && !copper) {
		c->Message(Chat::Red, "Usage: #givemoney [Platinum] [Gold] [Silver] [Copper]");
		return;
	}

	target->AddMoneyToPP(
		copper,
		silver,
		gold,
		platinum,
		true
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Added {} to {}.",
			Strings::Money(
				platinum,
				gold,
				silver,
				copper
			),
			c->GetTargetDescription(target)
		).c_str()
	);
}
