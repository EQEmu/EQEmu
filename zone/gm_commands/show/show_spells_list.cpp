#include "../../client.h"

void ShowSpellsList(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	const auto t = c->GetTarget()->CastToNPC();

	t->AISpellsList(c);
}
