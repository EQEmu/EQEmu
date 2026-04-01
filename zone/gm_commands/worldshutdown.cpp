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

void command_worldshutdown(Client *c, const Seperator *sep)
{
	// GM command to shutdown world server and all zone servers
	uint32 time     = 0;
	uint32 interval = 0;
	if (worldserver.Connected()) {
		if (
			sep->IsNumber(1) &&
			sep->IsNumber(2) &&
			((time     = Strings::ToInt(sep->arg[1])) > 0) &&
			((interval = Strings::ToInt(sep->arg[2])) > 0)
			) {
			int time_minutes = (time / 60);
			quest_manager.WorldWideMessage(
				Chat::System,
				fmt::format(
					"[SYSTEM] World will be shutting down in {} minutes.",
					time_minutes
				).c_str()
			);
			c->Message(
				Chat::White,
				fmt::format(
					"World will be shutting down in {} minutes, notifying every {} seconds",
					time_minutes,
					interval
				).c_str()
			);
			auto                 pack = new ServerPacket(ServerOP_ShutdownAll, sizeof(WorldShutDown_Struct));
			WorldShutDown_Struct *wsd = (WorldShutDown_Struct *) pack->pBuffer;
			wsd->time     = (time * 1000);
			wsd->interval = (interval * 1000);
			worldserver.SendPacket(pack);
			safe_delete(pack);
		}
		else if (!strcasecmp(sep->arg[1], "now")) {
			quest_manager.WorldWideMessage(
				Chat::Yellow,
				"[SYSTEM] World is shutting down now."
			);
			c->Message(Chat::White, "World is shutting down now.");
			auto pack = new ServerPacket;
			pack->opcode = ServerOP_ShutdownAll;
			pack->size   = 0;
			worldserver.SendPacket(pack);
			safe_delete(pack);
		}
		else if (!strcasecmp(sep->arg[1], "disable")) {
			c->Message(Chat::White, "World shutdown has been aborted.");
			auto                 pack = new ServerPacket(ServerOP_ShutdownAll, sizeof(WorldShutDown_Struct));
			WorldShutDown_Struct *wsd = (WorldShutDown_Struct *) pack->pBuffer;
			wsd->time     = 0;
			wsd->interval = 0;
			worldserver.SendPacket(pack);
			safe_delete(pack);
		}
		else {
			c->Message(Chat::White, "#worldshutdown - Shuts down the server and all zones.");
			c->Message(Chat::White, "Usage: #worldshutdown now - Shuts down the server and all zones immediately.");
			c->Message(
				Chat::White,
				"Usage: #worldshutdown disable - Stops the server from a previously scheduled shut down."
			);
			c->Message(
				Chat::White,
				"Usage: #worldshutdown [timer] [interval] - Shuts down the server and all zones after [timer] seconds and notifies players every [interval] seconds."
			);
		}
	}
	else {
		c->Message(Chat::White, "Error: World server is disconnected.");
	}
}

