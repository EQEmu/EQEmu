#include "zone/client.h"

void command_makepet(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #makepet [Pet Name]");
		return;
	}

	std::string pet_name = sep->arg[1];
	if (pet_name.empty()) {
		c->Message(Chat::White, "Usage: #makepet [Pet Name]");
		return;
	}

	c->MakePet(0, pet_name.c_str());
}

