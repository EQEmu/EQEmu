#include "zone/client.h"

void command_npceditmass(Client *c, const Seperator *sep)
{
	if (strcasecmp(sep->arg[1], "usage") == 0) {
		c->Message(
			Chat::White,
			"#npceditmass search_column [exact_match: =]search_value change_column change_value (apply)"
		);
		return;
	}

	std::string query = SQL(
		SELECT
			COLUMN_NAME
		FROM
		INFORMATION_SCHEMA.COLUMNS
		WHERE
			table_name = 'npc_types'
		AND
		COLUMN_NAME != 'id'
	);

	std::string search_column, search_value, change_column, change_value;

	if (sep->arg[1]) {
		search_column = sep->arg[1];
	}

	if (sep->arg[2]) {
		search_value = sep->arg[2];
	}

	if (sep->arg[3]) {
		change_column = sep->arg[3];
	}

	if (sep->arg[4]) {
		change_value = sep->arg[4];
	}

	bool valid_change_column = false;
	bool valid_search_column = false;
	auto results = content_db.QueryDatabase(query);

	std::vector<std::string> possible_column_options;

	for (auto row : results) {
		if (row[0] == change_column) {
			valid_change_column = true;
		}

		if (row[0] == search_column) {
			valid_search_column = true;
		}

		possible_column_options.push_back(row[0]);
	}

	std::string options_glue = ", ";

	if (!valid_search_column) {
		c->Message(
			Chat::Red,
			fmt::format(
				"You must specify a valid search column. [{}] is not valid",
				search_column
			).c_str()
		);

		c->Message(
			Chat::Yellow,
			fmt::format(
				"Possible columns [{}]",
				Strings::Implode(options_glue, possible_column_options)
			).c_str()
		);
		return;
	}

	if (!valid_change_column) {
		c->Message(
			Chat::Red,
			fmt::format(
				"You must specify a valid change column. [{}] is not valid",
				change_column
			).c_str()
		);

		c->Message(
			Chat::Yellow,
			fmt::format(
				"Possible columns [{}]",
				Strings::Implode(options_glue, possible_column_options)
			).c_str()
		);
		return;
	}

	if (!valid_search_column || !valid_change_column) {
		c->Message(Chat::Red, "One requested column is invalid.");
		return;
	}

	query = fmt::format(
		SQL(
			SELECT id, name, {}, {}
			FROM npc_types
			WHERE id IN(
				SELECT spawnentry.npcID
				FROM spawnentry
				JOIN spawn2
				ON spawn2.spawngroupID = spawnentry.spawngroupID
				WHERE spawn2.zone = '{}' AND spawn2.version = {}
			)
		),
		search_column,
		change_column,
		zone->GetShortName(),
		zone->GetInstanceVersion()
	);

	std::string status = "(Searching)";

	if (!strcasecmp(sep->arg[5], "apply")) {
		status = "(Applying)";
	}

	std::vector<std::string> npc_ids;

	bool exact_match = false;
	if (search_value[0] == '=') {
		exact_match  = true;
		search_value = search_value.substr(1);
	}

	int found_count = 0;
	results = content_db.QueryDatabase(query);
	for (auto row : results) {
		std::string npc_id = row[0];
		std::string npc_name = row[1];
		std::string search_column_value = Strings::ToLower(row[2]);
		std::string change_column_current_value = row[3];

		if (exact_match) {
			if (search_column_value.compare(search_value)) {
				continue;
			}
		}
		else {
			if (search_column_value.find(search_value) == std::string::npos) {
				continue;
			}
		}

		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ({}) [{}] ({}) [{}] Current ({}) [{}] New [{}] {}",
				npc_id,
				npc_name,
				search_column,
				search_column_value,
				change_column,
				change_column_current_value,
				change_value,
				status
			).c_str()
		);

		npc_ids.push_back(npc_id);

		found_count++;
	}

	std::string saylink = fmt::format(
		"#npceditmass {} {}{} {} {} apply",
		search_column,
		(exact_match ? "=" : ""),
		search_value,
		change_column,
		change_value
	);

	if (strcasecmp(sep->arg[5], "apply") == 0) {
		std::string npc_ids_string = Strings::Implode(",", npc_ids);
		if (npc_ids_string.empty()) {
			c->Message(Chat::Red, "Error: Ran into an unknown error compiling NPC IDs");
			return;
		}

		content_db.QueryDatabase(
			fmt::format(
				"UPDATE `npc_types` SET {} = '{}' WHERE id IN ({})",
				change_column,
				change_value,
				npc_ids_string
			)
		);

		c->Message(
			Chat::Yellow,
			fmt::format(
				"Changes applied to {} NPC{}.",
				found_count,
				found_count != 1 ? "s" : ""
			).c_str()
		);
		zone->Repop();
	}
	else {
		if (found_count > 0) {
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} NPC{} match your search.",
					found_count,
					found_count != 1 ? "s" : ""
				).c_str()
			);

			c->Message(
				Chat::Yellow,
				fmt::format(
					"Would you like to {} these changes?",
					Saylink::Silent(saylink, "apply")
				).c_str()
			);

			c->Message(
				Chat::Yellow,
				fmt::format(
					"You can also use '{}'.",
					saylink
				).c_str()
			);
		} else {
			c->Message(Chat::Yellow, "No NPCs match your search.");
		}
	}
}

