#include "zone/client.h"
#include "zone/dialogue_window.h"

void ShowKeyring(Client *c, const Seperator *sep)
{
	Client* t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	t->KeyRingList(c);
}
