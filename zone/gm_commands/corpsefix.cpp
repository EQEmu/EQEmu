#include "zone/client.h"
#include "zone/corpse.h"

void command_corpsefix(Client *c, const Seperator *sep)
{
	entity_list.CorpseFix(c);
}

