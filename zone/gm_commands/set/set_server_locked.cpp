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
#include "zone/worldserver.h"

extern WorldServer worldserver;

void SetServerLocked(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #set server_locked [on|off]");
		return;
	}

	bool is_locked = Strings::ToBool(sep->arg[2]);

	if (c->EntityVariableExists("old_command")) {
		const std::string& old_command = c->GetEntityVariable("old_command");
		if (old_command == "lock" || old_command == "serverlock") {
			is_locked = true;
		} else if (old_command == "unlock" || old_command == "serverunlock") {
			is_locked = false;
		}
	}

	auto pack = new ServerPacket(ServerOP_Lock, sizeof(ServerLock_Struct));

	auto l = (ServerLock_Struct *) pack->pBuffer;
	strn0cpy(l->character_name, c->GetCleanName(), sizeof(l->character_name));
	l->is_locked = is_locked;

	worldserver.SendPacket(pack);
	safe_delete(pack);
}
