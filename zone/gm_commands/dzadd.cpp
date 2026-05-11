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
#include "zone/dynamic_zone.h"

void command_dzadd(Client *c, const Seperator *sep)
{
	if (!c) {
		return;
	}

	if (sep->arg[1][0] == '\0') {
		c->Message(Chat::Yellow, "Usage: #dzadd <player_name>");
		return;
	}

	auto dz = c->GetExpedition();
	if (!dz) {
		c->Message(Chat::Red, "You are not a member of an expedition or dynamic zone.");
		return;
	}

	dz->DzAddPlayer(c, sep->arg[1]);
}
