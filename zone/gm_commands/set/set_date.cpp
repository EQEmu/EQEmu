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

void SetDate(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (
		arguments < 2 ||
		!sep->IsNumber(2) ||
		!sep->IsNumber(3) ||
		!sep->IsNumber(4)
	) {
		c->Message(Chat::White, "Usage: #date [Year] [Month] [Day] [Hour] [Minute]");
		c->Message(Chat::White, "Hour and Minute are optional");
		return;
	}

	TimeOfDay_Struct t{};
	zone->zone_time.GetCurrentEQTimeOfDay(time(0), &t);

	const uint16 year   = Strings::ToUnsignedInt(sep->arg[2]);
	const uint8  month  = Strings::ToUnsignedInt(sep->arg[3]);
	const uint8  day    = Strings::ToUnsignedInt(sep->arg[4]);
	const uint8  hour   = !sep->IsNumber(5) ? t.hour : Strings::ToUnsignedInt(sep->arg[5]) + 1;
	const uint8  minute = !sep->IsNumber(6) ? t.minute : Strings::ToUnsignedInt(sep->arg[6]);

	c->Message(
		Chat::White,
		fmt::format("Setting world time to {}/{}/{} {}.",
			year,
			month,
			day,
			Strings::ZoneTime(hour, minute)
		).c_str()
	);

	zone->SetDate(year, month, day, hour, minute);

	LogInfo(
		"{} :: Setting world time to {}/{}/{} {}.",
		c->GetCleanName(),
		year,
		month,
		day,
		Strings::ZoneTime(hour, minute)
	);
}
