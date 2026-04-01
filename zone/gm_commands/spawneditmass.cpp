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

void command_spawneditmass(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (arguments < 3) {
		c->Message(Chat::Yellow, "Usage: #spawneditmass [Search Criteria] [Edit Option] [Edit Value] [Apply]");
		return;
	}

	std::string search_criteria = sep->arg[1];
	std::string edit_option = sep->arg[2];
	std::string edit_value = sep->arg[3];
	bool is_apply = sep->arg[4] ? !strcasecmp(sep->arg[4], "apply") : false;

	if (edit_option != "respawn_time") { // Remove this when other options are added, here so people know there's only one option at the moment.
		c->Message(Chat::Yellow, "The only edit option currently implemented is \"respawn_time\", more will be added in the future.");
		return;
	}

	auto query = fmt::format(
		SQL(
			SELECT
				npc_types.id,
				npc_types.name,
				spawn2.respawntime,
				spawn2.id
			FROM
				npc_types
			JOIN spawnentry ON spawnentry.npcID = npc_types.id
			JOIN spawn2 ON spawn2.spawngroupID = spawnentry.spawngroupID
			WHERE
				spawn2.zone = '{}' AND spawn2.version = {}
			GROUP BY npc_types.id
		),
		zone->GetShortName(),
		zone->GetInstanceVersion()
	);

	std::vector<std::string> spawn2_ids;

	auto results = database.QueryDatabase(query);

	c->Message(
		Chat::Yellow,
		fmt::format(
			"Spawn Edit Mass ({})",
			is_apply ? "Applying" : "Searching"
		).c_str()
	);

	for (auto row : results) {
		std::string npc_id = row[0];
		std::string npc_name = row[1];
		std::string respawn_time = row[2];
		std::string spawn2_id = row[3];

		if (npc_name.find(search_criteria) == std::string::npos) {
			continue;
		}

		c->Message(
			Chat::Yellow,
			fmt::format(
				"Spawn2 ID: {} NPC ID: {} Name: {} Respawn Time: {} ({})",
				spawn2_id,
				npc_id,
				npc_name,
				Strings::SecondsToTime(Strings::ToInt(respawn_time)),
				Strings::Commify(respawn_time)
			).c_str()
		);

		spawn2_ids.push_back(spawn2_id);
	}

	if (!spawn2_ids.size()) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"No NPC Spawn2 entries were found matching '{}' in this zone.",
				search_criteria
			).c_str()
		);
		return;
	}

	c->Message(
		Chat::Yellow,
		fmt::format(
			"Found {} NPC Spawn2 entr{} matching '{}' in this zone",
			spawn2_ids.size(),
			spawn2_ids.size() != 1 ? "ies" : "y",
			search_criteria
		).c_str()
	);

	if (!is_apply) {
		auto edit_link = fmt::format(
			"#spawneditmass {} {} {} apply",
			search_criteria,
			edit_option,
			edit_value
		);

		c->Message(
			Chat::Yellow,
			fmt::format(
				"To apply these changes, click {} or type \"{}\".",
				Saylink::Silent(edit_link, "apply"),
				edit_link
			).c_str()
		);
	}

	if (edit_option == "respawn_time" && is_apply) {
		auto spawn2_ids_string = Strings::Implode(", ", spawn2_ids);
		if (spawn2_ids_string.empty()) {
			c->Message(
				Chat::Yellow,
				fmt::format(
					"No NPC Spawn2 entries were found matching '{}' in this zone.",
					search_criteria
				).c_str()
			);
			return;
		}

		database.QueryDatabase(
			fmt::format(
				SQL(
					UPDATE spawn2 SET respawntime = {} WHERE id IN ({})
				),
				Strings::ToInt(edit_value),
				spawn2_ids_string
			)
		);

		c->Message(
			Chat::Yellow,
			fmt::format(
				"Updated {} NPC spawn{} in this zone.",
				spawn2_ids.size(),
				spawn2_ids.size() != 1 ? "s" : ""
			).c_str()
		);
	}
}
