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
