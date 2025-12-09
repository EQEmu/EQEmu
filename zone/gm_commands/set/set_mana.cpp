#include "../../client.h"

void SetMana(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set mana [Amount]");
		return;
	}

	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	int64 mana = Strings::ToBigInt(sep->arg[2]);

	if (t->IsClient()) {
		if (mana >= t->CastToClient()->CalcMaxMana()) {
			mana = t->CastToClient()->CalcMaxMana();
		}

		t->CastToClient()->SetMana(mana);
	} else {
		if (mana >= t->CalcMaxMana()) {
			mana = t->CalcMaxMana();
		}

		t->SetMana(mana);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Set {} to {} Mana.",
			c->GetTargetDescription(t),
			Strings::Commify(mana)
		).c_str()
	);
}

