#include "../../client.h"

void ShowDistance(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || c->GetTarget() == c) {
		c->Message(Chat::White, "You must have a target to use this command.");
		return;
	}

	const auto t = c->GetTarget();

	c->Message(
		Chat::White,
		fmt::format(
			"{} is {:.2f} units from you.",
			c->GetTargetDescription(t),
			Distance(
				c->GetPosition(),
				t->GetPosition()
			)
		).c_str()
	);
}
