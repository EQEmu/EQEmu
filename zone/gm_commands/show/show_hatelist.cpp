#include "../../client.h"

void ShowHateList(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	const auto t = c->GetTarget();

	t->PrintHateListToClient(c);
}
