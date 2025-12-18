#include "zone/client.h"

void CatchSignal(int sig_num);

void command_shutdown(Client *c, const Seperator *sep)
{
	const int arguments = sep->argnum;
	if (!arguments) {
		c->Message(
			Chat::White,
			fmt::format(
				"Using this command will shut down your current zone. Please {} this is what you want to do.",
				Saylink::Silent("#shutdown confirm", "confirm")
			).c_str()
		);
		return;
	}

	const bool is_confirm = !strcasecmp(sep->arg[1], "confirm");

	if (is_confirm) {
		c->Message(Chat::White, "Shutting down your current zone.");
		CatchSignal(2);
	}
}
