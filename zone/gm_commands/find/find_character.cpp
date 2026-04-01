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
#include "common/repositories/character_data_repository.h"
#include "zone/client.h"

void FindCharacter(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(2)) {
		const uint32 character_id = Strings::ToUnsignedInt(sep->arg[2]);

		const auto& e = CharacterDataRepository::FindOne(database, character_id);
		if (!e.id) {
			c->Message(
				Chat::White,
				fmt::format(
					"Character ID {} does not exist or is invalid.",
					character_id
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Character ID {} | {}",
				character_id,
				e.name
			).c_str()
		);

		return;
	}

	const std::string& search_criteria = Strings::ToLower(sep->argplus[2]);

	const auto& l = CharacterDataRepository::GetWhere(
		database,
		fmt::format(
			"LOWER(`name`) LIKE '%%{}%%' AND `name` NOT LIKE '%-deleted-%' ORDER BY `id` ASC LIMIT 50",
			search_criteria
		)
	);

	if (l.empty()) {
		c->Message(
			Chat::White,
			fmt::format(
				"No characters found matching '{}'.",
				sep->argplus[2]
			).c_str()
		);
	}

	uint32 found_count = 0;

	for (const auto& e : l) {
		c->Message(
			Chat::White,
			fmt::format(
				"Character ID {} | {}",
				Strings::Commify(e.id),
				e.name
			).c_str()
		);

		found_count++;

		if (found_count == 50) {
			break;
		}
	}

	if (found_count == 50) {
		c->Message(
			Chat::White,
			fmt::format(
				"50 Characters found matching '{}', max reached.",
				sep->argplus[2]
			).c_str()
		);

		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Character{} found matching '{}'.",
			found_count,
			found_count != 1 ? "s" : "",
			sep->argplus[2]
		).c_str()
	);
}

