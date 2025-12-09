#include "../../client.h"

void ShowVariable(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #show variable [Variable Name]");
		return;
	}

	const std::string& variable = sep->argplus[2];

	std::string value;
	if (!database.GetVariable(variable, value)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Variable '{}' was not found.",
				variable
			).c_str()
		);
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Variable {} | {}",
			variable,
			value
		).c_str()
	);
}
