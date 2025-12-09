#include "../../client.h"

void SetPassword(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 3) {
		c->Message(Chat::White, "Usage: #set password [Account Name] [Password] [Loginserver]");
		return;
	}

	std::string account_name = sep->arg[2];
	std::string password     = sep->arg[3];
	std::string loginserver  = arguments >= 4 ? sep->arg[4] : "eqemu";

	int16 status = 0;

	const uint32 account_id = database.GetAccountIDByName(account_name, loginserver, &status);
	if (!account_id) {
		c->Message(
			Chat::White,
			fmt::format(
				"Account {} was not found.",
				account_name
			).c_str()
		);
		return;
	}

	if (status > c->Admin()) {
		c->Message(
			Chat::White,
			fmt::format(
				"You cannot change the password for Account {} as its status is higher than yours.",
				account_name
			).c_str()
		);
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Password {} changed for Account {}.",
			(
				database.SetLocalPassword(account_id, password.c_str()) ?
				"successfully" :
				"failed"
			),
			account_name
		).c_str()
	);
}
