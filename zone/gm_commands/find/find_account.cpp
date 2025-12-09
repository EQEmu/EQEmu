#include "../../client.h"
#include "../../../common/repositories/account_repository.h"

void FindAccount(Client *c, const Seperator *sep)
{
	const uint16 arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #find account [Character Name]");
		c->Message(Chat::White, "Note: Used to print the account ID and name of the account a character belongs to.");
		return;
	}

	const std::string& character_name = sep->arg[2];

	const auto& e = CharacterDataRepository::FindByName(database, character_name);

	if (!e.id) {
		c->Message(
			Chat::White,
			fmt::format(
				"Character '{}' does not exist.",
				character_name
			).c_str()
		);
		return;
	}

	auto a = AccountRepository::FindOne(database, e.account_id);

	if (!a.id) {
		c->Message(
			Chat::White,
			fmt::format(
				"Character '{}' is not attached to an account.",
				character_name
			).c_str()
		);
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Account {} ({}) owns the character {}.",
			a.name,
			a.id,
			character_name
		).c_str()
	);
}
