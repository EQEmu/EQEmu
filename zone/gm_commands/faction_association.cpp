#include "zone/client.h"

void command_faction_association(Client *c, const Seperator *sep)
{
	if (sep->argnum < 2 || !sep->IsNumber(1) || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #factionassociation <factionid> <amount>");
		c->Message(Chat::White, "  Defaults to self, unless target is also a client.");
		return;
	}

	// default to self unless target is also a client
	auto target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	target->RewardFaction(Strings::ToInt(sep->arg[1]), Strings::ToInt(sep->arg[2]));
}
