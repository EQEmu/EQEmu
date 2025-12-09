#include "../../client.h"

void SetInvulnerable(Client *c, const Seperator *sep) {
	const auto arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #set invulnerable [on|off]");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const bool invulnerable = Strings::ToBool(sep->arg[2]);

	t->SetInvul(invulnerable);

	const uint32 account_id = t->AccountID();

	database.SetGMInvul(account_id, invulnerable);

	c->Message(
		Chat::White,
		fmt::format(
			"{} {} now {}vulnerable.",
			c->GetTargetDescription(t, TargetDescriptionType::UCYou),
			c == t ? "are" : "is",
			invulnerable ? "in" : ""
		).c_str()
	);
}
