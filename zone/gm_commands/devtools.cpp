#include "common/data_bucket.h"
#include "zone/client.h"

void command_devtools(Client *c, const Seperator *sep)
{
	const uint16 arguments = sep->argnum;
	if (arguments != 2) {
		c->ShowDevToolsMenu();
		return;
	}

	const std::string& type = sep->arg[1];
	const bool toggle = Strings::ToBool(sep->arg[2]);

	if (Strings::EqualFold(type, "menu")) {
		c->SetDevToolsEnabled(toggle);
	} else if (Strings::EqualFold(type, "window")) {
		c->SetDisplayMobInfoWindow(toggle);
	}

	c->ShowDevToolsMenu();
}

