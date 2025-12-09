#include "../../client.h"

void ShowAggro(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #show aggro [Distance] [-v] (-v is verbose Faction Information)");
		return;
	}

	if (!c->GetTarget() ||  !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	const auto t = c->GetTarget()->CastToNPC();

	const float distance   = Strings::ToFloat(sep->arg[2]);
	const bool  is_verbose = Strings::EqualFold(sep->arg[3], "-v");

	entity_list.DescribeAggro(c, t, distance, is_verbose);
}
