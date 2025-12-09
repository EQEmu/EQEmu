#include "../../client.h"

void ShowAAs(Client *c, const Seperator *sep)
{
	Client *t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const std::string& search_criteria = sep->argnum >= 2 ? sep->argplus[2] : std::string();

	t->ListPurchasedAAs(c, search_criteria);
}
