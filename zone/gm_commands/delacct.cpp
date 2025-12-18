#include "common/repositories/account_repository.h"
#include "zone/client.h"

void command_delacct(Client *c, const Seperator *sep)
{
	auto arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Format: #delacct [Account ID|Account Name]");
		return;
	}

	uint32 account_id = 0;
	std::string account_name;

	if (sep->IsNumber(1)) {
		account_id = Strings::ToUnsignedInt(sep->arg[1]);
		auto a = AccountRepository::FindOne(content_db, account_id);
		if (!a.id) {
			c->Message(
				Chat::White,
				fmt::format(
					"Account ID {} does not exist or is invalid.",
					account_id
				).c_str()
			);
			return;
		}

		account_name = a.name;
	} else {
		account_name = sep->arg[1];
		auto a = AccountRepository::GetWhere(
			content_db,
			fmt::format(
				"`name` = '{}'",
				account_name
			)
		);

		if (a.empty() || !a[0].id) {
			c->Message(
				Chat::White,
				fmt::format(
					"Account {} does not exist or is invalid.",
					account_name
				).c_str()
			);
			return;
		}

		account_id = a[0].id;
	}

	if (!AccountRepository::DeleteOne(content_db, account_id)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to delete account {} ({}).",
				account_name,
				account_id
			).c_str()
		);
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Successfully deleted account {} ({}).",
			account_name,
			account_id
		).c_str()
	);
}

