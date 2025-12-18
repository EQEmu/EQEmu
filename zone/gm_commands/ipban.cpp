#include "zone/client.h"

void command_ipban(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #ipban [IP]");
		return;
	}

	std::string ip = sep->arg[1];
	if (ip.empty()) {
		c->Message(Chat::White, "Usage: #ipban [IP]");
		return;
	}

	if (database.AddBannedIP(ip, c->GetName())) {
		c->Message(
			Chat::White,
			fmt::format(
				"IP '{}' has been successfully banned.",
				ip
			).c_str()
		);
	} else {
		c->Message(
			Chat::White,
			fmt::format(
				"IP '{}' has failed to be banned, the IP address may already be in the table.",
				ip
			).c_str()
		);
	}
}

