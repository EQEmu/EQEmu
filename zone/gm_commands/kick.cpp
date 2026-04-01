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

void command_kick(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #kick [Character Name]");
		return;
	}
	
	std::string character_name = sep->arg[1];
	auto client = entity_list.GetClientByName(character_name.c_str());
	if (client) {
		if (client->Admin() <= c->Admin()) {
			auto outapp = new EQApplicationPacket(OP_GMKick, 0);
			client->QueuePacket(outapp);
			safe_delete(outapp);
			client->Kick("Ordered kicked by command");
			c->Message(
				Chat::White,
				fmt::format(
					"{} has been kicked from the server.",
					character_name
				).c_str()
			);
		}
	} else if (!worldserver.Connected()) {
		c->Message(Chat::White, "The world server is currently disconnected.");
	} else {
		auto pack = new ServerPacket(ServerOP_KickPlayer, sizeof(ServerKickPlayer_Struct));
		ServerKickPlayer_Struct *skp = (ServerKickPlayer_Struct *) pack->pBuffer;
		strcpy(skp->adminname, c->GetName());
		strcpy(skp->name, character_name.c_str());
		skp->adminrank = c->Admin();
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

