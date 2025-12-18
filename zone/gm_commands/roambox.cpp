#include "zone/client.h"
#include "zone/groups.h"

void command_roambox(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You need a valid NPC target for this command");
		return;
	}

	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "#roambox remove - Remove a roambox from an NPC");
		c->Message(Chat::White, "#roambox set [Box Size] [Delay] - Set a roambox for an NPC");
		return;
	}

	auto target = c->GetTarget()->CastToNPC();
	int spawn_group_id = target->GetSpawnGroupId();
	if (spawn_group_id <= 0) {
		c->Message(Chat::Red, "NPC needs a valid SpawnGroup!");
		return;
	}

	bool is_remove = !strcasecmp(sep->arg[1], "remove");
	bool is_set = !strcasecmp(sep->arg[1], "set");
	if (!is_remove && !is_set) {
		c->Message(Chat::White, "#roambox remove - Remove a roambox from an NPC");
		c->Message(Chat::White, "#roambox set [Box Size] [Delay] - Set a roambox for an NPC");
		return;
	}

	if (is_remove) {
		auto query = fmt::format(
			"UPDATE spawngroup SET	dist = 0, min_x = 0, max_x = 0, min_y = 0, max_y = 0, delay = 0 WHERE id = {}",
			spawn_group_id
		);

		database.QueryDatabase(query);

		c->Message(
			Chat::White,
			fmt::format(
				"Roambox has been removed from {} ({}) on spawn group ID {}.",
				target->GetCleanName(),
				target->GetID(),
				spawn_group_id
			).c_str()
		);
	} else if (is_set) {
		float box_size = 0.0f;
		int delay = 15000;

		if (arguments >= 2) {
			box_size = Strings::ToFloat(sep->arg[2]);

			if (arguments == 3) {
				delay = Strings::ToInt(sep->arg[3]);
			}
		}

		if (box_size) {
			auto query = fmt::format(
				"UPDATE spawngroup SET dist = {:2f}, min_x = {:2f}, max_x = {:.2f}, min_y = {:2f}, max_y = {:2f}, delay = {} WHERE id = {}",
				(box_size / 2.0f),
				(target->GetX() - (box_size / 2.0f)),
				(target->GetX() + (box_size / 2.0f)),
				(target->GetY() - (box_size / 2.0f)),
				(target->GetY() + (box_size / 2.0f)),
				delay,
				spawn_group_id
			);

			auto results = database.QueryDatabase(query);

			if (!results.RowsAffected()) {
				c->Message(
					Chat::White,
					fmt::format(
						"Failed to set roambox for {}.",
						c->GetTargetDescription(target)
					).c_str()
				);
				return;
			}

			c->Message(
				Chat::White,
				fmt::format(
					"Roambox set to box size of {} for {} on spawn group ID {} with a delay of {} ({}).",
					box_size,
					c->GetTargetDescription(target),
					spawn_group_id,
					Strings::MillisecondsToTime(delay),
					delay
				).c_str()
			);
		} else {
			c->Message(Chat::White, "Box size must be greater than 0.");
		}
	}
}

