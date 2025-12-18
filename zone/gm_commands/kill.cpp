#include "zone/client.h"

void command_kill(Client *c, const Seperator *sep)
{
	if (!sep->IsNumber(1) && !c->GetTarget()) {
		c->Message(Chat::White, "#kill - Kills your target");
		c->Message(Chat::White, "#kill [entity_id] - Kills the entity ID you provided");
		return;
	}

	Mob* t = nullptr;
	uint16 entity_id = 0;

	if (sep->IsNumber(1)) {
		entity_id = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[1]));
		t         = entity_list.GetMob(entity_id);
	} else {
		t = c->GetTarget();
	}

	if (!t) {
		c->Message(Chat::White, "You must have a target or supply an entity ID to use this command.");
		return;
	}

	if (!t->IsClient() || t->CastToClient()->Admin() <= c->Admin()) {
		if (c != t) {
			c->Message(
				Chat::White,
				fmt::format(
					"Killing {}{}.",
					c->GetTargetDescription(t),
					entity_id ? " by entity id" : ""
				).c_str()
			);
		}

		t->Kill();
	}
}

