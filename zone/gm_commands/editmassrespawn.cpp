#include "zone/client.h"

void command_editmassrespawn(Client *c, const Seperator *sep)
{
	if (strcasecmp(sep->arg[1], "usage") == 0) {
		c->Message(Chat::White, "#editmassrespawn [exact_match: =]npc_type_name new_respawn_seconds (apply)");
		return;
	}

	std::string search_npc_type;
	if (sep->arg[1]) {
		search_npc_type = sep->arg[1];
	}

	int change_respawn_seconds = 0;
	if (sep->arg[2] && sep->IsNumber(2)) {
		change_respawn_seconds = Strings::ToInt(sep->arg[2]);
	}

	bool change_apply = false;
	if (sep->arg[3] && strcasecmp(sep->arg[3], "apply") == 0) {
		change_apply = true;
	}

	std::string search_encapsulator = "%";
	if (search_npc_type[0] == '=') {

		search_npc_type     = search_npc_type.substr(1);
		search_encapsulator = "";
	}

	std::string query = fmt::format(
		SQL(
			SELECT npc_types.id, spawn2.spawngroupID, spawn2.id, npc_types.name, spawn2.respawntime
			FROM spawn2
			INNER JOIN spawnentry ON spawn2.spawngroupID = spawnentry.spawngroupID
			INNER JOIN npc_types ON spawnentry.npcID = npc_types.id
			WHERE spawn2.zone LIKE '{}'
			AND spawn2.version = '{}'
			AND npc_types.name LIKE '{}{}{}'
			ORDER BY npc_types.id, spawn2.spawngroupID, spawn2.id
		),
		zone->GetShortName(),
		zone->GetInstanceVersion(),
		search_encapsulator,
		search_npc_type,
		search_encapsulator
	);

	std::string status = "(Searching)";
	if (change_apply) {
		status = "(Applying)";
	}

	int results_count = 0;

	auto results = content_db.QueryDatabase(query);
	if (results.Success() && results.RowCount()) {

		results_count = results.RowCount();

		for (auto row : results) {
			c->Message(
				Chat::Yellow,
				fmt::format(
					"NPC (npcid:{}) (sgid:{}) (s2id:{}) [{}] Respawn: Current [{}] New [{}] {}",
					row[0],
					row[1],
					row[2],
					row[3],
					row[4],
					change_respawn_seconds,
					status
				).c_str()
			);
		}

		c->Message(Chat::Yellow, "Found (%i) NPC's that match this search...", results_count);

		if (change_respawn_seconds > 0) {

			if (change_apply) {

				results = content_db.QueryDatabase(
					fmt::format(
						SQL(
							UPDATE spawn2
							SET    respawntime = '{}'
							WHERE id IN(
							SELECT spawn2.id
							FROM spawn2
							INNER JOIN spawnentry ON spawn2.spawngroupID = spawnentry.spawngroupID
							INNER JOIN npc_types ON spawnentry.npcID = npc_types.id
							WHERE spawn2.zone LIKE '{}'
							AND spawn2.version = '{}'
							AND npc_types.name LIKE '{}{}{}'
						)
						),
						change_respawn_seconds,
						zone->GetShortName(),
						zone->GetInstanceVersion(),
						search_encapsulator,
						search_npc_type,
						search_encapsulator
					)
				);

				if (results.Success()) {

					c->Message(Chat::Yellow, "Changes applied to (%i) NPC 'Spawn2' entries", results_count);
					zone->Repop();
				}
				else {

					c->Message(Chat::Yellow, "Found (0) NPC's that match this search...");
				}
			}
			else {

				std::string saylink = fmt::format(
					"#editmassrespawn {}{} {} apply",
					(search_encapsulator.empty() ? "=" : ""),
					search_npc_type,
					change_respawn_seconds
				);

				c->Message(
					Chat::Yellow, "To apply these changes, click <%s> or type [%s]",
					Saylink::Silent(saylink, "Apply").c_str(),
					saylink.c_str()
				);
			}
		}
	}
	else {

		c->Message(Chat::Yellow, "Found (0) NPC's that match this search...");
	}
}

