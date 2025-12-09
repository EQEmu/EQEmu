#include "../../client.h"

void SetEnduranceFull(Client *c, const Seperator *sep)
{
	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	int endurance;

	if (t->IsClient()) {
		endurance = t->CastToClient()->GetMaxEndurance();
		t->CastToClient()->SetEndurance(endurance);
	} else {
		endurance = t->GetMaxEndurance();
		t->SetEndurance(endurance);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Set {} to full Endurance ({}).",
			c->GetTargetDescription(t),
			Strings::Commify(endurance)
		).c_str()
	);
}

