#include "../../client.h"

void SetGM(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #set gm [on|off]");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const bool gm_flag = Strings::ToBool(sep->arg[2]);

	t->SetGM(gm_flag);

	if (c != t) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} is {} flagged as a GM.",
				c->GetTargetDescription(t),
				gm_flag ? "now" : "no longer"
			).c_str()
		);
	}
}
