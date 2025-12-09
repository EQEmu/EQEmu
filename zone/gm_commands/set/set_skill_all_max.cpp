#include "../../client.h"

void SetSkillAllMax(Client *c, const Seperator *sep)
{
	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	t->MaxSkills();

	c->Message(
		Chat::White,
		fmt::format(
			"Maxed skills for {}.",
			c->GetTargetDescription(t)
		).c_str()
	);
}
