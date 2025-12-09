#include "../../client.h"

void SetHP(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set hp [Amount]");
		return;
	}

	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	int64 health = Strings::ToBigInt(sep->arg[2]);

	if (health >= t->GetMaxHP()) {
		health = t->GetMaxHP();
	}

	t->SetHP(health);
	t->SendHPUpdate();

	c->Message(
		Chat::White,
		fmt::format(
			"Set {} to {} Health.",
			c->GetTargetDescription(t),
			Strings::Commify(health)
		).c_str()
	);
}
