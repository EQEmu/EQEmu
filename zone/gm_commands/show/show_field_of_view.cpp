#include "../../client.h"

void ShowFieldOfView(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || c->GetTarget() == c) {
		c->Message(Chat::White, "You must have a target to use this command.");
		return;
	}

	const auto t = c->GetTarget();

	const bool is_behind = c->BehindMob(t, c->GetX(), c->GetY());

	c->Message(
		Chat::White,
		fmt::format(
			"You are {}behind {}, they have a heading of {}.",
			is_behind ? "" : "not ",
			c->GetTargetDescription(t),
			t->GetHeading()
		).c_str()
	);
}
