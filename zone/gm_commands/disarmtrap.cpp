#include "zone/client.h"

void command_disarmtrap(Client *c, const Seperator *sep)
{
	Mob *t = c->GetTarget();
	if (!t || !t->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	if (!c->HasSkill(EQ::skills::SkillDisarmTraps)) {
		c->Message(Chat::White, "You do not have the Disarm Trap skill.");
		return;
	}

	if (DistanceSquaredNoZ(c->GetPosition(), t->GetPosition()) > RuleI(Adventure, LDoNTrapDistanceUse)) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} is too far away.",
				t->GetCleanName()
			).c_str()
		);
		return;
	}

	c->HandleLDoNDisarm(t->CastToNPC(), c->GetSkill(EQ::skills::SkillDisarmTraps), LDoNTypeMechanical);
}
