#include "../../client.h"

void SetPVP(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #set pvp [on|off]");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const bool pvp_state = Strings::ToBool(sep->arg[2]);

	t->SetPVP(pvp_state);

	if (c != t) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} now follows the ways of {}.",
				c->GetTargetDescription(t),
				pvp_state ? "Discord" : "Order"
			).c_str()
		);
	}
}
