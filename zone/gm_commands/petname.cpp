#include "zone/client.h"

void command_petname(Client *c, const Seperator *sep)
{
	Mob *t = nullptr;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	if (!t) {
		c->Message(Chat::White, "You must target your pet to use this command.");
		return;
	}

	if (!t->IsPet()) {
		c->Message(Chat::White, "You must target your pet to use this command.");
		return;
	}

	if (t->GetOwnerID() != c->GetID()) {
		c->Message(Chat::White, "You must target your pet to use this command.");
		return;
	}

	if (sep->arg[1]) {
		const std::string& old_name = t->GetCleanName();
		const std::string& new_name = sep->arg[1];

		t->TempName(new_name.c_str());

		c->Message(
			Chat::White,
			fmt::format(
				"Renamed your pet from {} to {}.",
				old_name,
				new_name
			).c_str()
		);

		return;
	}

	t->TempName();
	c->Message(Chat::White, "Restored the original name.");
}
