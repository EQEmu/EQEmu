#include "zone/client.h"

void command_fleeinfo(Client *c, const Seperator *sep)
{
	if (c->GetTarget() && c->GetTarget()->IsNPC()) {
		Mob* client = entity_list.GetMob(c->GetID());
		if (client) {
			c->GetTarget()->FleeInfo(client);
		}
	} else {
		c->Message(Chat::Red, "Please target a NPC to use this command on.");
	}

}
