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

void command_camerashake(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1) || !sep->IsNumber(2)) {
		c->Message(Chat::Red, "Usage: #camerashake [Duration (Milliseconds)] [Intensity (1-10)]");
		return;
	}

	auto duration = Strings::ToInt(sep->arg[1]);
	auto intensity = Strings::ToInt(sep->arg[2]);

	auto pack = new ServerPacket(ServerOP_CameraShake, sizeof(ServerCameraShake_Struct));
	ServerCameraShake_Struct *camera_shake = (ServerCameraShake_Struct *) pack->pBuffer;
	camera_shake->duration = duration;
	camera_shake->intensity = intensity;
	worldserver.SendPacket(pack);
	c->Message(
		Chat::White,
		fmt::format(
			"Sending camera shake to world with a duration of {} ({}) and an intensity of {}.",
			Strings::MillisecondsToTime(duration),
			duration,
			intensity
		).c_str()
	);
	safe_delete(pack);
}

