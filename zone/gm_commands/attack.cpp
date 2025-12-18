#include "zone/client.h"

void command_attack(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	std::string entity_name = sep->argplus[1];
	if (entity_name.empty()) {
		c->Message(Chat::White, "Usage: #attack [Entity Name]");
		return;
	}

	auto entity = entity_list.GetMob(entity_name.c_str());
	if (entity) {
		c->GetTarget()->AddToHateList(entity, 1);
		c->Message(
			Chat::EchoChat1,
			fmt::format(
				"{} whispers, 'Attacking {}.'",
				c->GetTarget()->GetCleanName(),
				c->GetTargetDescription(entity)
			).c_str()
		);
	} else {
		c->Message(
			Chat::White,
			fmt::format(
				"No entity by the name of '{}' could be found.",
				entity_name
			).c_str()
		);
	}
}

