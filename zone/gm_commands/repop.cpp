#include "zone/client.h"

void command_repop(Client *c, const Seperator *sep)
{
	const bool is_forced = sep->argnum > 0 ? !strcasecmp(sep->arg[1], "force") : false;

	c->Message(
		Chat::White,
		fmt::format(
			"Zone depopped, {}repopping now.",
			is_forced ? "forcefully " : ""
		).c_str()
	);

	entity_list.ClearAreas();
	zone->Repop(is_forced);
}

