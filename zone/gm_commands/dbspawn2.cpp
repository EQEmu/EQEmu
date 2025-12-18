#include "zone/client.h"

void command_dbspawn2(Client *c, const Seperator *sep)
{
	auto arguments = sep->argnum;
	if (
		!arguments ||
		!sep->IsNumber(1) ||
		!sep->IsNumber(2) ||
		!sep->IsNumber(3)
	) {
		c->Message(Chat::White, "Usage: #dbspawn2 [Spawngroup ID] [Respawn] [Variance] [Condition ID] [Condition Minimum]");
		c->Message(Chat::White, "Note: Respawn and Variance are in Seconds.");
		return;
	}

	auto position = c->GetPosition();

	auto spawngroup_id = Strings::ToUnsignedInt(sep->arg[1]);
	auto respawn = Strings::ToUnsignedInt(sep->arg[2]);
	auto variance = Strings::ToUnsignedInt(sep->arg[3]);

	auto condition_id = sep->IsNumber(4) ? static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[4])) : static_cast<uint16>(0);
	auto condition_minimum = sep->IsNumber(5) ? static_cast<int16>(Strings::ToUnsignedInt(sep->arg[5])) : static_cast<int16>(0);

	if (!database.CreateSpawn2(
		c,
		spawngroup_id,
		zone->GetShortName(),
		position,
		respawn,
		variance,
		condition_id,
		condition_minimum
	)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to add Spawngroup ID {} to {} at {:.2f}, {:.2f}, {:.2f}, {:.2f}.",
				spawngroup_id,
				zone->GetZoneDescription(),
				position.x,
				position.y,
				position.z,
				position.w
			).c_str()
		);
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Spawngroup Added | ID: {}",
			spawngroup_id
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Spawngroup Added | Zone: {}",
			zone->GetZoneDescription()
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Spawngroup Added | Position: {:.2f}, {:.2f}, {:.2f}, {:.2f}",
			position.x,
			position.y,
			position.z,
			position.w
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Spawngroup Added | Respawn: {} ({})",
			Strings::SecondsToTime(respawn),
			respawn
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Spawngroup Added | Variance: {} ({})",
			Strings::SecondsToTime(variance),
			variance
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Spawngroup Added | Condition ID: {} Condition Minimum: {}",
			condition_id,
			condition_minimum
		).c_str()
	);
}
