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
#include "zone/fastmath.h"

extern FastMath g_Math;

void command_push(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #push [Back Push] [Up Push]");
		return;
	}

	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	auto target = c->GetTarget();
	auto back = Strings::ToFloat(sep->arg[1]);
	auto up = 0.0f;

	if (arguments == 2 && sep->IsNumber(2)) {
		up = Strings::ToFloat(sep->arg[2]);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Pushing {} with a push back of {:.2f} and a push up of {:.2f}.",
			c->GetTargetDescription(target),
			back,
			up
		).c_str()
	);

	target->IncDeltaX(back * g_Math.FastSin(c->GetHeading()));
	target->IncDeltaY(back * g_Math.FastCos(c->GetHeading()));
	target->IncDeltaZ(up);
	target->SetForcedMovement(6);
}

