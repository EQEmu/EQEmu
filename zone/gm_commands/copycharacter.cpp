#include "zone/client.h"

void command_copycharacter(Client *c, const Seperator *sep)
{
	if (
		sep->argnum < 3 ||
		sep->IsNumber(1) ||
		sep->IsNumber(2) ||
		sep->IsNumber(3)
	) {
		c->Message(
			Chat::White,
			"Usage: #copycharacter [source_character_name] [destination_character_name] [destination_account_name]"
		);
		return;
	}

	const std::string& source_character_name      = sep->arg[1];
	const std::string& destination_character_name = sep->arg[2];
	const std::string& destination_account_name   = sep->arg[3];

	const bool result = database.CopyCharacter(
		source_character_name,
		destination_character_name,
		destination_account_name
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Character Copy [{}] to [{}] via account [{}] [{}]",
			source_character_name,
			destination_character_name,
			destination_account_name,
			result ? "Success" : "Failed"
		).c_str()
	);
}

