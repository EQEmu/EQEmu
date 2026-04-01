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

void SetTimeZone(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set time_zone [Hour] [Minute]");
		c->Message(
			Chat::White,
			fmt::format(
				"Current time zone is {}.",
				Strings::ZoneTime(zone->zone_time.getEQTimeZoneHr(), zone->zone_time.getEQTimeZoneMin())
			).c_str()
		);
		return;
	}

	uint8 hours   = Strings::ToUnsignedInt(sep->arg[2]);

	EQ::Clamp(hours, static_cast<uint8>(0), static_cast<uint8>(24));

	uint8 minutes = sep->IsNumber(3) ? Strings::ToUnsignedInt(sep->arg[3]) : 0;

	EQ::Clamp(minutes, static_cast<uint8>(0), static_cast<uint8>(59));

	uint8 real_hours = (
		(hours - 1) > 0 ?
		(hours - 1) :
		0
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Setting timezone to {}.",
			Strings::ZoneTime(hours, minutes)
		).c_str()
	);

	const int new_timezone = ((hours * 60) + minutes);
	zone->zone_time.setEQTimeZone(new_timezone);
	content_db.SetZoneTimezone(zone->GetZoneID(), zone->GetInstanceVersion(), new_timezone);

	auto outapp = new EQApplicationPacket(OP_TimeOfDay, sizeof(TimeOfDay_Struct));

	auto t = (TimeOfDay_Struct *) outapp->pBuffer;
	zone->zone_time.GetCurrentEQTimeOfDay(time(0), t);

	entity_list.QueueClients(c, outapp);
	safe_delete(outapp);
}
