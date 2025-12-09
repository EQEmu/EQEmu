#include "../../client.h"

void ShowPEQZoneFlags(Client *c, const Seperator *sep)
{
	Client *t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t =  c->GetTarget()->CastToClient();
	}

	t->SendPEQZoneFlagInfo(c);
}
