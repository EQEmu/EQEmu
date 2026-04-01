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

void command_zoneshutdown(Client *c, const Seperator *sep)
{
	const int arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #zoneshutdown instance [Instance ID]");
		c->Message(Chat::White, "Usage: #zoneshutdown zone [Zone ID|Zone Short Name]");
		return;
	}

	bool is_instance = !strcasecmp(sep->arg[1], "instance");
	bool is_zone     = !strcasecmp(sep->arg[1], "zone");

	if (!is_instance && !is_zone) {
		c->Message(Chat::White, "Usage: #zoneshutdown instance [Instance ID]");
		c->Message(Chat::White, "Usage: #zoneshutdown zone [Zone ID|Zone Short Name]");
		return;
	}

	if (!worldserver.Connected()) {
		c->Message(Chat::White, "World server disconnected.");
		return;
	}

	uint32      zone_id     = 0;
	uint16      instance_id = 0;
	std::string message     = "";

	if (is_instance) {
		instance_id = sep->IsNumber(2) ? Strings::ToUnsignedInt(sep->arg[2]) : 0;

		if (!database.CheckInstanceExists(instance_id)) {
			c->Message(
				Chat::White,
				fmt::format(
					"Instance ID '{}' does not exist.",
					instance_id
				).c_str()
			);
			return;
		}

		message = fmt::format("Instance ID {}", instance_id);
	} else if (is_zone) {
		zone_id = sep->IsNumber(2) ? Strings::ToUnsignedInt(sep->arg[2]) : ZoneID(sep->arg[2]);

		if (!zone_id) {
			c->Message(
				Chat::White,
				fmt::format(
					"Zone '{}' does not exist.",
					sep->arg[1]
				).c_str()
			);
			return;
		}

		message = fmt::format("{} (ID {})", ZoneLongName(zone_id), zone_id);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Attempting to shut down {}.",
			message
		).c_str()
	);

	auto pack = new ServerPacket(ServerOP_ZoneShutdown, sizeof(ServerZoneStateChange_Struct));
	auto *s   = (ServerZoneStateChange_Struct *) pack->pBuffer;

	s->zone_id     = zone_id;
	s->instance_id = instance_id;

	strn0cpy(s->admin_name, c->GetName(), sizeof(s->admin_name));

	worldserver.SendPacket(pack);
	safe_delete(pack);
}
