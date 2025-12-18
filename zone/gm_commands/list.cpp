#include "zone/client.h"
#include "zone/command.h"
#include "zone/corpse.h"
#include "zone/doors.h"
#include "zone/object.h"

struct UniqueEntity {
	uint16      entity_id;
	std::string entity_name;
	uint32      unique_id;
	glm::vec4   position;
};

void command_list(Client *c, const Seperator *sep)
{
	const uint16 arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #list [bots|corpses|doors|npcs|objects|players] [search]");
		c->Message(Chat::White, "Example: #list npcs (Blank for all)");
		return;
	}

	const bool can_kill = c->Admin() >= GetCommandStatus("kill");

	const bool is_bots    = !strcasecmp(sep->arg[1], "bots");
	const bool is_corpses = !strcasecmp(sep->arg[1], "corpses");
	const bool is_doors   = !strcasecmp(sep->arg[1], "doors");
	const bool is_npcs    = !strcasecmp(sep->arg[1], "npcs");
	const bool is_objects = !strcasecmp(sep->arg[1], "objects");
	const bool is_players = !strcasecmp(sep->arg[1], "players");

	if (
		!is_bots &&
		!is_corpses &&
		!is_doors &&
		!is_npcs &&
		!is_objects &&
		!is_players
	) {
		c->Message(Chat::White, "Usage: #list [bots|corpses|doors|npcs|objects|players] [search]");
		c->Message(Chat::White, "Example: #list npcs (Blank for all)");
		return;
	}

	std::string search_type;
	std::string unique_type;

	if (is_bots) {
		search_type = "bot";
		unique_type = "Bot ID";
	} else if (is_corpses) {
		search_type = "corpse";
		unique_type = "Corpse ID";
	} else if (is_doors) {
		search_type = "door";
		unique_type = "Door ID";
	} else if (is_npcs) {
		search_type = "NPC";
		unique_type = "NPC ID";
	} else if (is_objects) {
		search_type = "object";
		unique_type = "Object ID";
	} else if (is_players) {
		search_type = "player";
		unique_type = "Character ID";
	}

	uint32 entity_count = 0;
	uint32 found_count  = 0;

	const std::string &search_string = sep->arg[2] ? Strings::ToLower(sep->arg[2]) : std::string();

	std::vector<UniqueEntity> unique_entities;

	if (is_bots) {
		const auto& l = entity_list.GetBotList();

		for (const auto& e : l) {
			Bot* entity = e.second;

			entity_count++;

			const std::string& entity_name = Strings::ToLower(entity->GetName());
			if (!search_string.empty() && !Strings::Contains(entity_name, search_string)) {
				continue;
			}

			unique_entities.emplace_back(
				UniqueEntity{
					.entity_id = entity->GetID(),
					.entity_name = entity->GetName(),
					.unique_id = entity->GetBotID(),
					.position = entity->GetPosition()
				}
			);

			found_count++;
		}
	} else if (is_corpses) {
		const auto& l = entity_list.GetCorpseList();

		for (const auto& e: l) {
			Corpse* entity = e.second;

			entity_count++;

			const std::string& entity_name = Strings::ToLower(entity->GetName());
			if (!search_string.empty() && !Strings::Contains(entity_name, search_string)) {
				continue;
			}

			unique_entities.emplace_back(
				UniqueEntity{
					.entity_id = entity->GetID(),
					.entity_name = entity->GetName(),
					.unique_id = entity->GetCorpseDBID(),
					.position = entity->GetPosition()
				}
			);

			found_count++;
		}
	} else if (is_doors) {
		const auto& l = entity_list.GetDoorsList();

		for (const auto& e: l) {
			Doors* entity = e.second;

			entity_count++;

			const std::string& entity_name = Strings::ToLower(entity->GetDoorName());
			if (!search_string.empty() && !Strings::Contains(entity_name, search_string)) {
				continue;
			}

			unique_entities.emplace_back(
				UniqueEntity{
					.entity_id = entity->GetID(),
					.entity_name = entity->GetDoorName(),
					.unique_id = entity->GetDoorID(),
					.position = entity->GetPosition()
				}
			);

			found_count++;
		}
	} else if (is_npcs) {
		const auto& l = entity_list.GetMobList();

		for (const auto& e: l) {
			Mob* entity = e.second;
			if (!entity->IsNPC()) {
				continue;
			}

			entity_count++;

			const std::string& entity_name = Strings::ToLower(entity->GetName());
			if (!search_string.empty() && !Strings::Contains(entity_name, search_string)) {
				continue;
			}

			unique_entities.emplace_back(
				UniqueEntity{
					.entity_id = entity->GetID(),
					.entity_name = entity->GetName(),
					.unique_id = entity->GetNPCTypeID(),
					.position = entity->GetPosition()
				}
			);

			found_count++;
		}
	} else if (is_objects) {
		const auto& l = entity_list.GetObjectList();

		for (const auto& e: l) {
			Object* entity = e.second;

			entity_count++;

			const std::string& entity_name = Strings::ToLower(entity->GetModelName());
			if (!search_string.empty() && !Strings::Contains(entity_name, search_string)) {
				continue;
			}

			unique_entities.emplace_back(
				UniqueEntity{
					.entity_id = entity->GetID(),
					.entity_name = entity->GetModelName(),
					.unique_id = entity->GetDBID(),
					.position = glm::vec4(entity->GetX(), entity->GetY(), entity->GetZ(), 0.0f)
				}
			);

			found_count++;
		}
	} else if (is_players) {
		const auto& l = entity_list.GetClientList();

		for (const auto& e: l) {
			Client* entity = e.second;

			entity_count++;

			const std::string& entity_name = Strings::ToLower(entity->GetName());
			if (!search_string.empty() && !Strings::Contains(entity_name, search_string)) {
				continue;
			}

			unique_entities.emplace_back(
				UniqueEntity{
					.entity_id = entity->GetID(),
					.entity_name = entity->GetName(),
					.unique_id = entity->CharacterID(),
					.position = entity->GetPosition()
				}
			);

			found_count++;
		}
	}

	if (!found_count) {
		c->Message(
			Chat::White,
			fmt::format(
				"Could not find any {}{}.",
				search_type,
				(
					!search_string.empty() ?
						fmt::format(
							" matching '{}'",
							search_string
						) :
						""
				)
			).c_str()
		);
		return;
	}

	std::sort(
		unique_entities.begin(),
		unique_entities.end(),
		[](UniqueEntity a, UniqueEntity b) {
			if (a.entity_id && b.entity_id) {
				return a.entity_id < b.entity_id;
			} else {
				return a.unique_id < b.unique_id;
			}
		}
	);

	for (const auto& e : unique_entities) {
		const std::string& saylink = Saylink::Silent(
			fmt::format(
				"#goto {:.2f} {:.2f} {:.2f}",
				e.position.x,
				e.position.y,
				e.position.z
			),
			"Goto"
		);

		c->Message(
			Chat::White,
			fmt::format(
				"| {}{}{} | {} |{}",
				saylink,
				(
					e.entity_id ?
					fmt::format(
						" | ID {}",
						e.entity_id
					) :
					""
				),
				(
					e.unique_id ?
					fmt::format(
						" | {} {}",
						unique_type,
						e.unique_id
					) :
					""
				),
				e.entity_name,
				(
					(can_kill && (is_bots || is_npcs || is_players)) ?
					fmt::format(
						" {} |",
						Saylink::Silent(
							fmt::format(
								"#kill {}",
								e.entity_id
							),
							"Kill"
						)
					) :
					""
				)
			).c_str()
		);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Found {} {}{}{}, {} total.",
			found_count,
			search_type,
			found_count != 1 ? "s" : "",
			(
				!search_string.empty() ?
				fmt::format(
					" matching '{}'",
					search_string
				) :
				""
			),
			entity_count
		).c_str()
	);
}
