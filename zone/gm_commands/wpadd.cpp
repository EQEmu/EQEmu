#include "zone/client.h"

void command_wpadd(Client *c, const Seperator *sep)
{
	int type1   = 0, type2 = 0, pause = 0; // Defaults for a new grid
	Mob *target = c->GetTarget();
	if (target && target->IsNPC()) {
		Spawn2 *s2info = target->CastToNPC()->respawn2;
		if (s2info == nullptr) {
			c->Message(
				Chat::White,
				"#wpadd Failed, you must target a valid spawn."
			);
			return;
		}

		if (sep->arg[1][0]) {
			if (Strings::ToInt(sep->arg[1]) >= 0) {
				pause = Strings::ToInt(sep->arg[1]);
			}
			else {
				c->Message(Chat::White, "Usage: #wpadd [pause] [-h]");
				return;
			}
		}
		auto position = c->GetPosition();
		if (strcmp("-h", sep->arg[2]) != 0) {
			position.w = -1;
		}

		auto   zone_id  = zone->GetZoneID();
		uint32 tmp_grid = content_db.AddWaypointForSpawn(c, s2info->GetID(), position, pause, type1, type2, zone_id);
		if (tmp_grid) {
			target->CastToNPC()->SetGrid(tmp_grid);
		}

		auto grid_id = target->CastToNPC()->GetGrid();
		target->CastToNPC()->AssignWaypoints(grid_id);
		c->Message(
			Chat::White,
			fmt::format(
				"Waypoint added to grid {} in zone ID {}. Use #wpinfo to see waypoints for this NPC (may need to #repop first).",
				grid_id,
				zone_id
			).c_str()
		);
	}
	else {
		c->Message(Chat::White, "You must target an NPC to use this.");
	}
}

