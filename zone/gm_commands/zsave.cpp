#include "zone/client.h"

void command_zsave(Client *c, const Seperator *sep)
{
	c->Message(
		Chat::White,
		fmt::format(
			"Zone header {}.",
			(
				zone->SaveZoneCFG() ?
				"saved successfully" :
				"failed to save"
			)
		).c_str()
	);
}
