#include "zone/bot.h"
#include "zone/client.h"

void command_mystats(Client *c, const Seperator *sep)
{
	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	if (
		(t->IsPet() && t == c->GetPet()) ||
		(t->IsBot() && t->CastToBot()->GetOwner() && t->CastToBot()->GetOwner() == c)
	) {
		t->ShowStats(c);
		return;
	}

	c->ShowStats(c);
}

