#include "../../client.h"

void ShowBuffs(Client *c, const Seperator *sep)
{
	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	t->ShowBuffs(c);
}
