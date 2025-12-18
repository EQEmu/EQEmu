#include "zone/client.h"

void command_fish(Client *c, const Seperator *sep)
{
	c->GoFish(true, false);
}
