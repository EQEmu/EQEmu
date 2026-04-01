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

