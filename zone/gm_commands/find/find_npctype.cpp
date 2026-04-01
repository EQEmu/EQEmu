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
#include "zone/command.h"

void FindNPCType(Client *c, const Seperator *sep)
{
	std::string query = "SELECT `id`, `name` FROM npc_types WHERE ";
	const std::string& search_criteria = sep->argplus[2];
	if (sep->IsNumber(2)) {
		const auto npc_id = Strings::ToUnsignedInt(sep->arg[2]);

		query += fmt::format(
			"id = {}",
			npc_id
		);
	} else {
		query += fmt::format(
			"`name` LIKE '%%{}%%'",
			Strings::Escape(search_criteria)
		);
	}

	query += " ORDER BY `id` ASC LIMIT 50";

	auto results = content_db.QueryDatabase(query);
	if (!results.Success() || !results.RowCount()) {
		c->Message(
			Chat::White,
			fmt::format(
				"No NPCs matching '{}' were found.",
				search_criteria
			).c_str()
		);

		return;
	}

	const auto can_spawn_npcs = c->Admin() >= GetCommandStatus("#npctypespawn");

	auto found_count = 0;

	for (auto row : results) {
		auto found_number = (found_count + 1);
		if (found_count == 50) {
			break;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"NPC {} | {}{}",
				row[0],
				row[1],
				(
					can_spawn_npcs ?
					fmt::format(
						" | {}",
						Saylink::Silent(
							fmt::format(
								"#npctypespawn {}",
								row[0]
							),
							"Spawn"
						)
					) :
					""
				)
			).c_str()
		);

		found_count++;
	}

	if (found_count == 50) {
		c->Message(
			Chat::White,
			fmt::format(
				"50 NPCs found matching '{}', max reached.",
				search_criteria
			).c_str()
		);

		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} NPC{} found matching '{}'.",
			found_count,
			found_count != 1 ? "s" : "",
			search_criteria
		).c_str()
	);
}

