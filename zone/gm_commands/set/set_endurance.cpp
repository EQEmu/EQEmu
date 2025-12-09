#include "../../client.h"

void SetEndurance(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set endurance [Endurance]");
		return;
	}

	int endurance = Strings::ToInt(sep->arg[2]);

	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	if (t->IsClient()) {
		if (endurance >= t->CastToClient()->GetMaxEndurance()) {
			endurance = t->CastToClient()->GetMaxEndurance();
		}

		t->CastToClient()->SetEndurance(endurance);
	} else {
		if (endurance >= t->GetMaxEndurance()) {
			endurance = t->GetMaxEndurance();
		}

		t->SetEndurance(endurance);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Set {} to {} Endurance.",
			c->GetTargetDescription(t),
			Strings::Commify(endurance)
		).c_str()
	);
}
