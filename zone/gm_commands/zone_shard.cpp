#include "zone/client.h"

void command_zone_shard(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		if (!RuleB(Zone, ZoneShardQuestMenuOnly)) {
			c->ShowZoneShardMenu();
		}

		return;
	}

	if (c->GetAggroCount() > 0) {
		c->Message(Chat::White, "You cannot request a shard change while in combat.");
		return;
	}

	std::string zone_input = sep->arg[1];
	uint32      zone_id    = 0;

	// if input is id
	if (Strings::IsNumber(zone_input)) {
		zone_id = Strings::ToInt(zone_input);

		// validate
		if (zone_id != 0 && !GetZone(zone_id)) {
			c->Message(Chat::White, fmt::format("Could not find zone by id [{}]", zone_id).c_str());
			return;
		}
	}
	else {
		// validate
		if (!ZoneStore::Instance()->GetZone(zone_input)) {
			c->Message(Chat::White, fmt::format("Could not find zone by short_name [{}]", zone_input).c_str());
			return;
		}

		// validate we got id
		zone_id = ZoneID(zone_input);
		if (zone_id == 0) {
			c->Message(Chat::White, fmt::format("Could not find zone id by short_name [{}]", zone_input).c_str());
			return;
		}
	}

	auto z = GetZone(zone_id);
	if (z && z->shard_at_player_count == 0) {
		c->Message(Chat::White, "Zone does not have sharding enabled.");
		return;
	}

	auto instance_id = sep->arg[2] ? Strings::ToBigInt(sep->arg[2]) : 0;
	if (instance_id < -1) {
		c->Message(Chat::White, "You must enter a valid Instance ID.");
		return;
	}

	if (zone_id == c->GetZoneID() && c->GetInstanceID() == instance_id) {
		c->Message(Chat::White, "You are already in this shard.");
		return;
	}

	if (zone_id != c->GetZoneID()) {
		c->Message(Chat::White, "You must request a shard change from the zone you are currently in.");
		return;
	}

	auto results = CharacterDataRepository::GetInstanceZonePlayerCounts(database, c->GetZoneID());
	if (results.size() <= 1) {
		c->Message(Chat::White, "No shards found.");
		return;
	}

	if (instance_id > 0) {
		if (!database.CheckInstanceExists(instance_id)) {
			c->Message(
				Chat::White,
				fmt::format(
					"Instance ID {} does not exist.",
					instance_id
				).c_str()
			);
			return;
		}

		auto instance_zone_id = database.GetInstanceZoneID(instance_id);
		if (!instance_zone_id) {
			c->Message(
				Chat::White,
				fmt::format(
					"Instance ID {} not found or zone is set to null.",
					instance_id
				).c_str()
			);
			return;
		}

		if (instance_zone_id != zone_id) {
			c->Message(
				Chat::White,
				fmt::format(
					"Instance Zone ID {} does not match zone ID {}.",
					instance_id,
					zone_id
				).c_str()
			);
			return;
		}

		if (!database.CheckInstanceByCharID(instance_id, c->CharacterID())) {
			database.AddClientToInstance(instance_id, c->CharacterID());
		}

		if (!database.VerifyInstanceAlive(instance_id, c->CharacterID())) {
			c->Message(
				Chat::White,
				fmt::format(
					"Instance ID {} expired.",
					instance_id
				).c_str()
			);
			return;
		}
	}

	c->MovePC(
		zone_id,
		instance_id,
		c->GetX(),
		c->GetY(),
		c->GetZ(),
		c->GetHeading(),
		0,
		ZoneSolicited
	);
}
