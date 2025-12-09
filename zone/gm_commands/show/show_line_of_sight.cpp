#include "../../client.h"

void ShowLineOfSight(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || c->GetTarget() == c) {
		c->Message(Chat::White, "You must have a target to use this command.");
		return;
	}

	const auto t = c->GetTarget();

	const bool has_los = c->CheckLosFN(t);

	c->Message(
		Chat::White,
		fmt::format(
			"You {}have line of sight to {}.",
			has_los ? "" : "do not ",
			c->GetTargetDescription(t)
		).c_str()
	);
}
