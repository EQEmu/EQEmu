#include "common/repositories/grid_entries_repository.h"
#include "zone/client.h"

void command_grid(Client *c, const Seperator *sep)
{
	const uint16 arguments = sep->argnum;

	if (!arguments) {
		c->Message(Chat::White, "Usage: #grid add [Grid ID] [Wander Type] [Pause Type] - Add a grid with the specified wander and pause type");
		c->Message(Chat::White, "Usage: #grid delete [Grid ID] - Delete a grid");
		c->Message(Chat::White, "Usage: #grid hide - Hides waypoint nodes from targeted NPC's grid");
		c->Message(Chat::White, "Usage: #grid max - Displays the highest grid ID used in this zone (for add)");
		c->Message(Chat::White, "Usage: #grid show - Displays waypoint nodes for targeted NPC's grid");
		return;
	}

	const bool is_add    = !strcasecmp(sep->arg[1], "add");
	const bool is_delete = !strcasecmp(sep->arg[1], "delete");
	const bool is_hide   = !strcasecmp(sep->arg[1], "hide");
	const bool is_max    = !strcasecmp(sep->arg[1], "max");
	const bool is_show   = !strcasecmp(sep->arg[1], "show");

	if (!is_add && !is_delete && !is_hide && !is_max && !is_show) {
		c->Message(Chat::White, "Usage: #grid add [Grid ID] [Wander Type] [Pause Type] - Add a grid with the specified wander and pause type");
		c->Message(Chat::White, "Usage: #grid delete [Grid ID] - Delete a grid");
		c->Message(Chat::White, "Usage: #grid hide - Hides waypoint nodes from targeted NPC's grid");
		c->Message(Chat::White, "Usage: #grid max - Displays the highest grid ID used in this zone (for add)");
		c->Message(Chat::White, "Usage: #grid show - Displays waypoint nodes for targeted NPC's grid");
		return;
	}

	if (is_add) {
		const uint32 grid_id     = Strings::ToUnsignedInt(sep->arg[2]);
		const uint8  wander_type = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[3]));
		const uint8  pause_type  = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[4]));

		if (!grid_id) {
			c->Message(Chat::White, "You must specify a valid grid ID.");
			return;
		}

		if (!content_db.GridExistsInZone(zone->GetZoneID(), grid_id)) {
			content_db.ModifyGrid(c, false, grid_id, wander_type, pause_type, zone->GetZoneID());

			c->Message(
				Chat::White,
				fmt::format(
					"Grid {} added to zone ID {} with wander type {} and pause type {}.",
					grid_id,
					zone->GetZoneID(),
					wander_type,
					pause_type
				).c_str()
			);
		} else {
			c->Message(
				Chat::White,
				fmt::format(
					"Grid {} already exists in zone ID {}.",
					grid_id,
					zone->GetZoneID()
				).c_str()
			);
		}
	} else if (is_delete) {
		const uint32 grid_id = Strings::ToUnsignedInt(sep->arg[2]);

		content_db.ModifyGrid(c, true, grid_id, 0, 0, zone->GetZoneID());

		c->Message(
			Chat::White,
			fmt::format(
				"Grid {} deleted from zone ID {}.",
				grid_id,
				zone->GetZoneID()
			).c_str()
		);
	} else if (is_hide) {
		Mob* t = c->GetTarget();
		if (!t || !t->IsNPC()) {
			c->Message(Chat::White, "You must target an NPC to use this command.");
			return;
		}

		const uint32 grid_id = t->CastToNPC()->GetGrid();

		entity_list.DespawnGridNodes(grid_id);

		c->Message(
			Chat::White,
			fmt::format(
				"Depawning nodes for grid {}.",
				grid_id
			).c_str()
		);
	} else if (is_max) {
		c->Message(
			Chat::White,
			fmt::format(
				"Highest grid ID in this zone is {}.",
				content_db.GetHighestGrid(zone->GetZoneID())
			).c_str()
		);
	} else if (is_show) {
		Mob* t = c->GetTarget();
		if (!t || !t->IsNPC()) {
			c->Message(Chat::White, "You must target an NPC to use this command.");
			return;
		}

		const uint32 grid_id = t->CastToNPC()->GetGrid();

		const auto& l = GridEntriesRepository::GetWhere(
			content_db,
			fmt::format(
				"`zoneid` = {} AND `gridid` = {} ORDER BY `number`",
				zone->GetZoneID(),
				grid_id
			)
		);

		if (l.empty()) {
			c->Message(Chat::White, "No grid found.");
			return;
		}

		// Depop any grid nodes already spawned
		entity_list.DespawnGridNodes(grid_id);

		// Spawn grid nodes
		std::map<std::vector<float>, int32> zoffset;

		for (const auto& e : l) {
			glm::vec4 node_position = glm::vec4(e.x, e.y, e.z, e.heading);

			std::vector<float> node_loc{
				node_position.x,
				node_position.y,
				node_position.z
			};

			// If we already have a node at this location, set the z offset
			// higher from the existing one, so we can see it.  Adjust so if
			// there is another at the same spot we adjust again.
			auto search = zoffset.find(node_loc);
			if (search != zoffset.end()) {
				search->second += 3;
			} else {
				zoffset[node_loc] = 0.0;
			}

			node_position.z += zoffset[node_loc];

			NPC::SpawnGridNodeNPC(node_position, grid_id, e.number, zoffset[node_loc]);
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Spawning nodes for grid {}.",
				grid_id
			).c_str()
		);
	}
}
