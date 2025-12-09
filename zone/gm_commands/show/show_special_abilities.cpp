#include "../../client.h"

void ShowSpecialAbilities(Client* c, const Seperator* sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	NPC* t = c->GetTarget()->CastToNPC();

	t->DescribeSpecialAbilities(c);
}
