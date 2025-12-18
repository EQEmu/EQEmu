#include "zone/client.h"
#include "zone/dynamic_zone.h"

void command_dzkickplayers(Client *c, const Seperator *sep)
{
	if (c) {
		auto dz = c->GetExpedition();
		if (dz) {
			dz->DzKickPlayers(c);
		}
	}
}
