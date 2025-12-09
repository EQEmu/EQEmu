#include "../../client.h"

void SetGMSpeed(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #set gmspeed [on|off]");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const bool gm_speed = Strings::ToBool(sep->arg[2]);

	database.SetGMSpeed(
		t->AccountID(),
		gm_speed ? 1 : 0
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Turning GM Speed {} for {}.",
			gm_speed ? "on" : "off",
			c->GetTargetDescription(t)
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Note: {} must zone for it to take effect.",
			c->GetTargetDescription(t, TargetDescriptionType::UCYou)
		).c_str()
	);
}
