#include "zone/client.h"

void command_depopzone(Client *c, const Seperator *sep)
{
	auto start_spawn_timers = false;

	if (sep->IsNumber(1)) {
		start_spawn_timers = Strings::ToInt(sep->arg[1]) ? true : false;
	}

	zone->Depop(start_spawn_timers);

	c->Message(
		Chat::White,
		fmt::format(
			"Zone depopped{}.",
			start_spawn_timers ? " and spawn timers started" : ""
		).c_str()
	);
}

