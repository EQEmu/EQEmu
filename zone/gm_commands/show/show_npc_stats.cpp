#include "../../client.h"

void ShowNPCStats(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	const auto t = c->GetTarget()->CastToNPC();

	// Stats
	t->ShowStats(c);

	// Loot Data
	t->QueryLoot(c);
}
