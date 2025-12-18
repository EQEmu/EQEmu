#include "zone/client.h"

void command_forage(Client *c, const Seperator *sep)
{
	c->ForageItem(true);
}
