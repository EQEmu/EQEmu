#include "../../client.h"

void ShowStats(Client *c, const Seperator *sep)
{
	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	t->ShowStats(c);
}
