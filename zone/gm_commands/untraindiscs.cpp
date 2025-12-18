#include "zone/client.h"

void command_untraindiscs(Client *c, const Seperator *sep)
{
	Client *t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM()) {
		t = c->GetTarget()->CastToClient();
	}

	t->UntrainDiscAll();
	t->Message(Chat::Yellow, "All disciplines removed.");
}

