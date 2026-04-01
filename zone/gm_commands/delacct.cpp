/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
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

