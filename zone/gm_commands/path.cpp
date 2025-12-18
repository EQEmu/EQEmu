#include "zone/client.h"

void command_path(Client *c, const Seperator *sep)
{
	if (zone->pathing) {
		zone->pathing->DebugCommand(c, sep);
	}
}

