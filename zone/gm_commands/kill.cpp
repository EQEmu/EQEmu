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

void command_kill(Client *c, const Seperator *sep)
{
	if (!sep->IsNumber(1) && !c->GetTarget()) {
		c->Message(Chat::White, "#kill - Kills your target");
		c->Message(Chat::White, "#kill [entity_id] - Kills the entity ID you provided");
		return;
	}

	Mob* t = nullptr;
	uint16 entity_id = 0;

	if (sep->IsNumber(1)) {
		entity_id = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[1]));
		t         = entity_list.GetMob(entity_id);
	} else {
		t = c->GetTarget();
	}

	if (!t) {
		c->Message(Chat::White, "You must have a target or supply an entity ID to use this command.");
		return;
	}

	if (!t->IsClient() || t->CastToClient()->Admin() <= c->Admin()) {
		if (c != t) {
			c->Message(
				Chat::White,
				fmt::format(
					"Killing {}{}.",
					c->GetTargetDescription(t),
					entity_id ? " by entity id" : ""
				).c_str()
			);
		}

		t->Kill();
	}
}

