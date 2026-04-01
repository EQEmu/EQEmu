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
#include "zone/corpse.h"

void command_depop(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	auto start_spawn_timer = false;

	if (sep->IsNumber(1)) {
		start_spawn_timer = Strings::ToInt(sep->arg[1]) ? true : false;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Depopping {}{}.",
			c->GetTargetDescription(c->GetTarget()),
			start_spawn_timer ? " and starting their spawn timer" : ""
		).c_str()
	);

	c->GetTarget()->Depop(start_spawn_timer);
}
