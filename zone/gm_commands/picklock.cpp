#include "zone/client.h"

void command_picklock(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	auto t = c->GetTarget()->CastToNPC();

	if (c->HasSkill(EQ::skills::SkillPickLock)) {
		if (DistanceSquaredNoZ(c->GetPosition(), t->GetPosition()) > RuleI(Adventure, LDoNTrapDistanceUse)) {
			c->Message(
				Chat::White,
				fmt::format(
					"{} is too far away.",
					c->GetTargetDescription(t)
				).c_str()
			);
			return;
		}

		c->HandleLDoNPickLock(t, c->GetSkill(EQ::skills::SkillPickLock), LDoNTypeMechanical);
	} else {
		c->Message(Chat::White, "You do not have the Pick Lock skill.");
	}
}

