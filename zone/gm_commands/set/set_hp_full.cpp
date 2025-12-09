#include "../../client.h"

void SetHPFull(Client *c, const Seperator *sep)
{
	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	t->RestoreHealth();

	c->Message(
		Chat::White,
		fmt::format(
			"Set {} to full Health ({}).",
			c->GetTargetDescription(t),
			Strings::Commify(t->GetMaxHP())
		).c_str()
	);
}
