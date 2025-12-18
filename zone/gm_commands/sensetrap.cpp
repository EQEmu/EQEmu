#include "zone/client.h"

void command_sensetrap(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	auto target = c->GetTarget()->CastToNPC();

	if (!c->HasSkill(EQ::skills::SkillSenseTraps)) {
		c->Message(Chat::White, "You do not have the Sense Traps skill.");
		return;
	}
	
	if (DistanceSquaredNoZ(c->GetPosition(), target->GetPosition()) > RuleI(Adventure, LDoNTrapDistanceUse)) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} is too far away.",
				c->GetTargetDescription(target)
			).c_str()
		);
	}
	
	c->HandleLDoNSenseTraps(target, c->GetSkill(EQ::skills::SkillSenseTraps), LDoNTypeMechanical);
}

