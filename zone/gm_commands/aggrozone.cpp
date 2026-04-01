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

void command_aggrozone(Client *c, const Seperator *sep)
{
	Mob *target = c;
	if (c->GetTarget()) {
		target = c->GetTarget();
	}

	int64 hate = 0;
	if (sep->IsNumber(1)) {
		hate = std::strtoll(sep->arg[1], nullptr, 10);
	}

	entity_list.AggroZone(target, hate);
	c->Message(
		Chat::White,
		fmt::format(
			"Aggroing zone on {}.",
			c->GetTargetDescription(target)
		).c_str()
	);
}
