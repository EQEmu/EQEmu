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

void SetGMSpeed(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #set gmspeed [on|off]");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const bool gm_speed = Strings::ToBool(sep->arg[2]);

	database.SetGMSpeed(
		t->AccountID(),
		gm_speed ? 1 : 0
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Turning GM Speed {} for {}.",
			gm_speed ? "on" : "off",
			c->GetTargetDescription(t)
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Note: {} must zone for it to take effect.",
			c->GetTargetDescription(t, TargetDescriptionType::UCYou)
		).c_str()
	);
}
