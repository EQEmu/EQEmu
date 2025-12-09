#include "../../client.h"

void ShowSpawnStatus(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #show spawn_status all - Show all spawn statuses for your current zone");
		c->Message(Chat::White, "Usage: #show spawn_status disabled - Show all disabled spawn statuses for your current zone");
		c->Message(Chat::White, "Usage: #show spawn_status enabled - Show all enabled spawn statuses for your current zone");
		c->Message(Chat::White, "Usage: #show spawn_status [Spawn ID] - Show spawn status by ID for your current zone");
		return;
	}

	const bool is_all      = !strcasecmp(sep->arg[2], "all");
	const bool is_disabled = !strcasecmp(sep->arg[2], "disabled");
	const bool is_enabled  = !strcasecmp(sep->arg[2], "enabled");
	const bool is_search   = sep->IsNumber(2);

	if (
		!is_all &&
		!is_disabled &&
		!is_enabled &&
		!is_search
	) {
		c->Message(Chat::White, "Usage: #show spawn_status all - Show all spawn statuses for your current zone");
		c->Message(Chat::White, "Usage: #show spawn_status disabled - Show all disabled spawn statuses for your current zone");
		c->Message(Chat::White, "Usage: #show spawn_status enabled - Show all enabled spawn statuses for your current zone");
		c->Message(Chat::White, "Usage: #show spawn_status [Spawn ID] - Show spawn status by ID for your current zone");
		return;
	}

	std::string filter_type;
	if (is_disabled) {
		filter_type = "Disabled";
	} else if (is_enabled) {
		filter_type = "Enabled";
	}

	const uint32 spawn_id = (
		is_search ?
		Strings::ToUnsignedInt(sep->arg[2]) :
		0
	);

	LinkedListIterator<Spawn2*> iterator(zone->spawn2_list);
	iterator.Reset();

	uint32 filtered_count = 0;
	uint32 spawn_count    = 0;
	uint32 spawn_number   = 1;

	while (iterator.MoreElements()) {
		const auto& e = iterator.GetData();

		const uint32 time_remaining = e->GetTimer().GetRemainingTime();

		if (
			is_all ||
			(
				is_disabled &&
				time_remaining == UINT32_MAX
			) ||
			(
				is_enabled &&
				time_remaining != UINT32_MAX
			) ||
			(
				is_search &&
				e->GetID() == spawn_id
			)
		) {
			c->Message(
				Chat::White,
				fmt::format(
					"Spawn {} | ID: {} Coordinates: {:.2f}, {:.2f}, {:.2f}, {:.2f}",
					spawn_number,
					e->GetID(),
					e->GetX(),
					e->GetY(),
					e->GetZ(),
					e->GetHeading()
				).c_str()
			);

			if (time_remaining != UINT32_MAX) {
				const uint32 seconds_remaining = (time_remaining / 1000);

				c->Message(
					Chat::White,
					fmt::format(
						"Spawn {} | Respawn: {}",
						spawn_number,
						Strings::SecondsToTime(seconds_remaining)
					).c_str()
				);
			}

			filtered_count++;
			spawn_number++;
		}

		spawn_count++;
		iterator.Advance();
	}

	if (!spawn_count) {
		c->Message(Chat::White, "No spawns were found.");
		return;
	}

	if (
		(is_disabled || is_enabled) &&
		!filtered_count
	) {
		c->Message(
			Chat::White,
			fmt::format(
				"No {} spawns were found.",
				filter_type
			).c_str()
		);
		return;
	}

	if (is_all) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} Spawn{} found.",
				spawn_count,
				spawn_count != 1 ? "s" : ""
			).c_str()
		);

		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} of {} spawn{} found.",
			filtered_count,
			spawn_count,
			spawn_count != 1 ? "s" : ""
		).c_str()
	);
}
