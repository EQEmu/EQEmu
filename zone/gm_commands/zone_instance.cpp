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

void command_zone_instance(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #zoneinstance [Instance ID] [X] [Y] [Z]");
		return;
	}

	auto instance_id = Strings::ToUnsignedInt(sep->arg[1]);
	if (!instance_id) {
		c->Message(Chat::White, "You must enter a valid Instance ID.");
		return;
	}

	if (!database.CheckInstanceExists(instance_id)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Instance ID {} does not exist.",
				instance_id
			).c_str()
		);
		return;
	}

	auto zone_id = database.GetInstanceZoneID(instance_id);
	if (!zone_id) {
		c->Message(
			Chat::White,
			fmt::format(
				"Instance ID {} not found or zone is set to null.",
				instance_id
			).c_str()
		);
		return;
	}

	if (!database.CheckInstanceByCharID(instance_id, c->CharacterID())) {
		database.AddClientToInstance(instance_id, c->CharacterID());
	}

	if (!database.VerifyInstanceAlive(instance_id, c->CharacterID())) {
		c->Message(
			Chat::White,
			fmt::format(
				"Instance ID {} expired.",
				instance_id
			).c_str()
		);
		return;
	}

	auto x = sep->IsNumber(2) ? Strings::ToFloat(sep->arg[2]) : 0.0f;
	auto y = sep->IsNumber(3) ? Strings::ToFloat(sep->arg[3]) : 0.0f;
	auto z = sep->IsNumber(4) ? Strings::ToFloat(sep->arg[4]) : 0.0f;
	auto zone_mode = sep->IsNumber(2) ? ZoneSolicited : ZoneToSafeCoords;

	c->MovePC(
		zone_id,
		instance_id,
		x,
		y,
		z,
		0.0f,
		0,
		zone_mode
	);
}
