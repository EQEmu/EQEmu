#include "zone/client.h"

void command_serverrules(Client *c, const Seperator *sep)
{
	c->SendRules();
}

