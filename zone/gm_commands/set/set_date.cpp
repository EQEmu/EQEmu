#include "../../client.h"

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
