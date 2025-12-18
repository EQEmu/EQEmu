#include "zone/client.h"

void command_aggrozone(Client *c, const Seperator *sep)
{
	Mob *target = c;
	if (c->GetTarget()) {
		target = c->GetTarget();
	}

	int64 hate = 0;
	if (sep->IsNumber(1)) {
		hate = std::strtoll(sep->arg[1], nullptr, 10);
	}

	entity_list.AggroZone(target, hate);
	c->Message(
		Chat::White,
		fmt::format(
			"Aggroing zone on {}.",
			c->GetTargetDescription(target)
		).c_str()
	);
}
