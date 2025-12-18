#include "zone/client.h"

void command_exptoggle(Client *c, const Seperator *sep)
{
	auto arguments = sep->argnum;
	if (!arguments || arguments > 1) {
		c->Message(Chat::White, "Usage: #exptoggle [Toggle] - Toggle your or your target's experience gain.");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM()) {
		t = c->GetTarget()->CastToClient();
	}

	const auto is_exp_enabled = Strings::ToBool(sep->arg[1]);

	t->SetEXPEnabled(is_exp_enabled);

	c->Message(
		Chat::White,
		fmt::format(
			"Experience gain for {} is now {}abled.",
			c->GetTargetDescription(t),
			is_exp_enabled ? "en" : "dis"
		).c_str()
	);
}
