#include "../client.h"
#include "../../common/data_bucket.h"

void command_gmzone(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #gmzone [Zone ID|Zone Short Name] [Version] [Instance Identifier]");
		return;
	}

	std::string zone_short_name = Strings::ToLower(
		sep->IsNumber(1) ?
			ZoneName(Strings::ToUnsignedInt(sep->arg[1]), true) :
			sep->arg[1]
	);
	bool        is_unknown_zone = zone_short_name.find("unknown") != std::string::npos;
	if (is_unknown_zone) {
		c->Message(
			Chat::White,
			fmt::format(
				"Zone {} could not be found.",
				zone_short_name
			).c_str()
		);
	}

	auto zone_id = ZoneID(zone_short_name);
	if (!zone_id) {
		c->Message(
			Chat::White,
			fmt::format(
				"Zone ID {} could not be found.",
				zone_id
			).c_str()
		);
		return;
	}

	std::string zone_long_name = ZoneLongName(zone_id);

	auto zone_version = (
		sep->IsNumber(2) ?
			Strings::ToUnsignedInt(sep->arg[2]) :
			0
	);

	std::string instance_identifier = (
		sep->arg[3] ?
			sep->arg[3] :
			"gmzone"
	);

	auto bucket_key = fmt::format(
		"{}-{}-{}-instance",
		zone_short_name,
		instance_identifier,
		zone_version
	);

	auto   existing_zone_instance = DataBucket::GetData(&database, bucket_key);
	uint16 instance_id            = 0;
	uint32 duration               = 100000000;

	if (!existing_zone_instance.empty()) {
		instance_id = Strings::ToInt(existing_zone_instance);
		c->Message(
			Chat::White,
			fmt::format(
				"You already have an Instance ID of {} for Version {} of {} ({}).",
				instance_id,
				zone_version,
				zone_long_name,
				zone_short_name
			).c_str()
		);
	}

	if (!instance_id) {
		if (!database.GetUnusedInstanceID(instance_id)) {
			c->Message(Chat::White, "Failed to find an unused Instance ID.");
			return;
		}

		if (!database.CreateInstance(instance_id, zone_id, zone_version, duration)) {
			c->Message(Chat::White, "Failed to create an Instance.");
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"New GM Instance ID {} for Version {} of {} ({}) was created.",
				instance_id,
				zone_version,
				zone_long_name,
				zone_short_name
			).c_str()
		);

		DataBucket::SetData(
			&database,
			bucket_key,
			std::to_string(instance_id)
		);
	}

	if (instance_id) {
		float target_x = -1, target_y = -1, target_z = -1, target_heading = -1;

		auto z = GetZoneVersionWithFallback(
			ZoneID(zone_short_name.c_str()),
			zone_version
		);
		if (!z) {
			c->Message(
				Chat::White,
				fmt::format(
					"Failed to find the safe coordinates for Version {} of {} ({}).",
					zone_version,
					zone_long_name,
					zone_short_name
				).c_str()
			);
			return;
		}

		target_x       = z->safe_x;
		target_y       = z->safe_y;
		target_z       = z->safe_z;
		target_heading = z->safe_heading;

		c->Message(
			Chat::White,
			fmt::format(
				"Zoning to GM Instance ID {} for Version {} of {} ({}).",
				instance_id,
				zone_version,
				zone_long_name,
				zone_short_name
			).c_str()
		);

		c->AssignToInstance(instance_id);
		c->MovePC(
			zone_id,
			instance_id,
			target_x,
			target_y,
			target_z,
			target_heading,
			1
		);
	}
}

