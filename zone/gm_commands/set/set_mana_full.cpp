#include "../../client.h"

void SetManaFull(Client *c, const Seperator *sep)
{
	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	int64 mana;

	if (t->IsClient()) {
		mana = t->CastToClient()->CalcMaxMana();
		t->CastToClient()->SetMana(mana);
	} else {
		mana = t->CalcMaxMana();
		t->SetMana(mana);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Set {} to full Mana ({}).",
			c->GetTargetDescription(t),
			Strings::Commify(mana)
		).c_str()
	);
}
