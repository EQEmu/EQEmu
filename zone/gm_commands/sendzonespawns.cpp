#include "zone/client.h"

void command_sendzonespawns(Client *c, const Seperator *sep)
{
	entity_list.SendZoneSpawns(c);
}

