#include "zone/client.h"
#include "zone/corpse.h"

void command_depop(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	auto start_spawn_timer = false;

	if (sep->IsNumber(1)) {
		start_spawn_timer = Strings::ToInt(sep->arg[1]) ? true : false;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Depopping {}{}.",
			c->GetTargetDescription(c->GetTarget()),
			start_spawn_timer ? " and starting their spawn timer" : ""
		).c_str()
	);

	c->GetTarget()->Depop(start_spawn_timer);
}
