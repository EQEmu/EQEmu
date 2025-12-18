#include "zone/client.h"

void command_wp(Client *c, const Seperator *sep)
{
	auto command_type = sep->arg[1];
	auto grid_id      = Strings::ToInt(sep->arg[2]);
	if (grid_id != 0) {
		auto pause    = Strings::ToInt(sep->arg[3]);
		auto waypoint = Strings::ToInt(sep->arg[4]);
		auto zone_id  = zone->GetZoneID();
		if (strcasecmp("add", command_type) == 0) {
			if (waypoint == 0) { // Default to highest if it's left blank, or we enter 0
				waypoint = (content_db.GetHighestWaypoint(zone_id, grid_id) + 1);
			}

			if (strcasecmp("-h", sep->arg[5]) == 0) {
				content_db.AddWaypoint(c, grid_id, waypoint, c->GetPosition(), pause, zone_id);
			}
			else {
				auto position = c->GetPosition();
				position.w = -1;
				content_db.AddWaypoint(c, grid_id, waypoint, position, pause, zone_id);
			}
			c->Message(
				Chat::White,
				fmt::format(
					"Waypoint {} added to grid {} with a pause of {} {}.",
					waypoint,
					grid_id,
					pause,
					(pause == 1 ? "second" : "seconds")
				).c_str()
			);
		}
		else if (strcasecmp("delete", command_type) == 0) {
			content_db.DeleteWaypoint(c, grid_id, waypoint, zone_id);
			c->Message(
				Chat::White,
				fmt::format(
					"Waypoint {} deleted from grid {}.",
					waypoint,
					grid_id
				).c_str()
			);
		}
	}
	else {
		c->Message(Chat::White, "Usage: #wp [add|delete] [grid_id] [pause] [waypoint_id] [-h]");
	}
}

