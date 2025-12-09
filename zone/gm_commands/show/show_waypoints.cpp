#include "../../client.h"

void ShowWaypoints(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	auto t = c->GetTarget()->CastToNPC();

	if (!t->GetGrid()) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} is not a part of any grid.",
				c->GetTargetDescription(t)
			).c_str()
		);

		return;
	}

	t->DisplayWaypointInfo(c);
}
