#include "../../client.h"

void SetZoneData(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 3) {
		c->Message(
			Chat::White,
			"Usage: #set zone clipping [Minimum Clip] [Maximum Clip] [Fog Minimum Clip] [Fog Maximum Clip] [Permanent (0 = False, 1 = True)] (Permanent is optional)"
		);
		c->Message(
			Chat::White,
			"Usage: #set zone color [Red] [Green] [Blue] [Permanent (0 = False, 1 = True)] (Permanent is optional)"
		);
		c->Message(
			Chat::White,
			"Usage: #set zone header [Zone ID|Zone Short Name] [Version]"
		);
		c->Message(
			Chat::White,
			"Usage: #set zone locked [on|off] [Zone ID|Zone Short Name]"
		);
		c->Message(
			Chat::White,
			"Usage: #set zone safe_coordinates [X] [Y] [Z] [Heading] [Permanent (0 = False, 1 = True)] (Permanent is optional)"
		);
		c->Message(
			Chat::White,
			"Usage: #set zone sky [Sky Type] [Permanent (0 = False, 1 = True)] (Permanent is optional)"
		);
		c->Message(
			Chat::White,
			"Usage: #set zone underworld [Z] [Permanent (0 = False, 1 = True)] (Permanent is optional)"
		);
		return;
	}

	const bool is_clipping         = Strings::EqualFold(sep->arg[2], "clipping");
	const bool is_color            = Strings::EqualFold(sep->arg[2], "color");
	const bool is_header           = Strings::EqualFold(sep->arg[2], "header");
	const bool is_locked           = Strings::EqualFold(sep->arg[2], "locked");
	const bool is_safe_coordinates = Strings::EqualFold(sep->arg[2], "safe_coordinates");
	const bool is_sky              = Strings::EqualFold(sep->arg[2], "sky");
	const bool is_underworld       = Strings::EqualFold(sep->arg[2], "underworld");

	if (
		!is_clipping &&
		!is_color &&
		!is_header &&
		!is_locked &&
		!is_safe_coordinates &&
		!is_sky &&
		!is_underworld
	) {
		c->Message(
			Chat::White,
			"Usage: #set zone clipping [Minimum Clip] [Maximum Clip] [Fog Minimum Clip] [Fog Maximum Clip] [Permanent (0 = False, 1 = True)] (Permanent is optional)"
		);
		c->Message(
			Chat::White,
			"Usage: #set zone color [Red] [Green] [Blue] [Permanent (0 = False, 1 = True)] (Permanent is optional)"
		);
		c->Message(
			Chat::White,
			"Usage: #set zone header [Zone ID|Zone Short Name] [Version]"
		);
		c->Message(
			Chat::White,
			"Usage: #set zone locked [on|off] [Zone ID|Zone Short Name]"
		);
		c->Message(
			Chat::White,
			"Usage: #set zone safe_coordinates [X] [Y] [Z] [Heading] [Permanent (0 = False, 1 = True)] (Permanent is optional)"
		);
		c->Message(
			Chat::White,
			"Usage: #set zone sky [Sky Type] [Permanent (0 = False, 1 = True)] (Permanent is optional)"
		);
		c->Message(
			Chat::White,
			"Usage: #set zone underworld [Z] [Permanent (0 = False, 1 = True)] (Permanent is optional)"
		);
		return;
	}

	if (is_clipping) {
		const float minimum_clip     = sep->arg[3] ? Strings::ToFloat(sep->arg[3]) : 0.0f;
		const float maximum_clip     = sep->arg[4] ? Strings::ToFloat(sep->arg[4]) : 0.0f;
		const float minimum_fog_clip = sep->arg[5] ? Strings::ToFloat(sep->arg[5]) : 0.0f;
		const float maximum_fog_clip = sep->arg[6] ? Strings::ToFloat(sep->arg[6]) : 0.0f;
		const bool  permanent        = sep->arg[7] ? Strings::ToBool(sep->arg[7]) : false;

		if (minimum_clip < 0.0f || maximum_clip < 0.0f) {
			c->Message(Chat::White, "Minimum Clip and Maximum Clip must be greater than or equal 0.0.");
			return;
		} else if (minimum_clip > maximum_clip) {
			c->Message(Chat::White, "Minimum Clip must be less than or equal to Maximum Clip!");
			return;
		} else {
			zone->newzone_data.minclip = minimum_clip;
			zone->newzone_data.maxclip = maximum_clip;

			if (minimum_fog_clip) {
				for (float &fog_index: zone->newzone_data.fog_minclip) {
					fog_index = minimum_fog_clip;
				}
			}

			if (maximum_fog_clip) {
				for (float &fog_index: zone->newzone_data.fog_maxclip) {
					fog_index = maximum_fog_clip;
				}
			}

			if (permanent) {
				std::string query = fmt::format(
					"UPDATE zone SET minclip = {:.2f}, maxclip = {:.2f} WHERE zoneidnumber = {} AND version = {}",
					minimum_clip,
					maximum_clip,
					zone->GetZoneID(),
					zone->GetInstanceVersion()
				);

				database.QueryDatabase(query);

				if (minimum_fog_clip) {
					query = fmt::format(
						"UPDATE zone SET fog_minclip = {:.2f} WHERE zoneidnumber = {} AND version = {}",
						minimum_fog_clip,
						zone->GetZoneID(),
						zone->GetInstanceVersion()
					);
					database.QueryDatabase(query);
				}

				if (maximum_fog_clip) {
					query = fmt::format(
						"UPDATE zone SET fog_maxclip = {:.2f} WHERE zoneidnumber = {} AND version = {}",
						maximum_fog_clip,
						zone->GetZoneID(),
						zone->GetInstanceVersion()
					);
					database.QueryDatabase(query);
				}
			}

			auto outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
			memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
			entity_list.QueueClients(c, outapp);
			safe_delete(outapp);

			c->Message(
				Chat::White,
				fmt::format(
					"Clipping Changed | Zone: {} Permanent: {}",
					zone->GetZoneDescription(),
					permanent ? "Yes" : "No"
				).c_str()
			);

			c->Message(
				Chat::White,
				fmt::format(
					"Clipping Changed | Minimum Clip: {:.2f} Maximum Clip: {:.2f}",
					minimum_clip,
					maximum_clip
				).c_str()
			);

			if (minimum_fog_clip || maximum_fog_clip) {
				c->Message(
					Chat::White,
					fmt::format(
						"Clipping Changed | Fog Minimum Clip: {:.2f} Fog Maximum Clip: {:.2f}",
						minimum_fog_clip,
						maximum_fog_clip
					).c_str()
				);
			}
		}
	} else if (is_color) {
		if (
			arguments < 3 ||
			!sep->IsNumber(3) ||
			!sep->IsNumber(4) ||
			!sep->IsNumber(5)
		) {
			c->Message(Chat::White, "Usage: #set zone color [Red] [Green] [Blue] [Permanent (0 = False, 1 = True)]");
			return;
		}

		const uint8 red       = Strings::ToUnsignedInt(sep->arg[3]);
		const uint8 green     = Strings::ToUnsignedInt(sep->arg[4]);
		const uint8 blue      = Strings::ToUnsignedInt(sep->arg[5]);
		const bool  permanent = sep->arg[6] ? Strings::ToBool(sep->arg[6]) : false;

		if (
			!EQ::ValueWithin(red, 0, UINT8_MAX) ||
			!EQ::ValueWithin(green, 0, UINT8_MAX) ||
			!EQ::ValueWithin(blue, 0, UINT8_MAX)
		) {
			c->Message(Chat::White, "Colors cannot be less than 0 or greater than 255.");
			return;
		}

		if (permanent) {
			const std::string& query = fmt::format(
				"UPDATE zone SET fog_red = {}, fog_green = {}, fog_blue = {} "
				"WHERE zoneidnumber = {} AND version = {}",
				red,
				green,
				blue,
				zone->GetZoneID(),
				zone->GetInstanceVersion()
			);
			database.QueryDatabase(query);
		}

		for (uint8 fog_index = 0; fog_index < 4; fog_index++) {
			zone->newzone_data.fog_red[fog_index]   = red;
			zone->newzone_data.fog_green[fog_index] = green;
			zone->newzone_data.fog_blue[fog_index]  = blue;
		}

		auto outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
		memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
		entity_list.QueueClients(c, outapp);
		safe_delete(outapp);

		c->Message(
			Chat::White,
			fmt::format(
				"Fog Color Changed | Zone: {} Red: {} Green: {} Blue: {} Permanent: {}",
				zone->GetZoneDescription(),
				red,
				green,
				blue,
				permanent ? "Yes" : "No"
			).c_str()
		);
	} else if (is_header) {
		if (arguments < 3) {
			c->Message(Chat::White, "Usage: #set zone header [Zone ID|Zone Short Name] [Version]");
			return;
		}

		const auto zone_id = (
			sep->IsNumber(3) ?
			Strings::ToUnsignedInt(sep->arg[3]) :
			ZoneID(sep->arg[3])
		);
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

		const std::string& zone_short_name = ZoneName(zone_id);
		const std::string& zone_long_name = ZoneLongName(zone_id);
		const auto version = (
			sep->IsNumber(4) ?
			Strings::ToUnsignedInt(sep->arg[4]) :
			0
		);

		auto outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
		memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
		entity_list.QueueClients(c, outapp);
		safe_delete(outapp);

		ZoneStore::Instance()->LoadZones(content_db);

		c->Message(
			Chat::White,
			fmt::format(
				"Zone Header Load {} | Zone: {} ({}){}",
				(
					zone->LoadZoneCFG(zone_short_name.c_str(), version) ?
					"Succeeded" :
					"Failed"
				),
				zone_long_name,
				zone_short_name,
				(
					version ?
						fmt::format(
						" Version: {}",
						version
					) :
					""
				)
			).c_str()
		);
	} else if (is_locked) {
		if (arguments < 4) {
			c->Message(Chat::White, "Usage: #set zone locked [on|off] [Zone ID|Zone Short Name]");
			return;
		}

		const bool is_locked = Strings::ToBool(sep->arg[3]);

		const uint32 zone_id = (
			sep->IsNumber(4) ?
			Strings::ToUnsignedInt(sep->arg[4]) :
			ZoneID(sep->arg[4])
		);
		const std::string& zone_short_name = Strings::ToLower(ZoneName(zone_id, true));

		const bool is_unknown_zone = Strings::EqualFold(zone_short_name, "unknown");
		if (!zone_id || is_unknown_zone) {
			c->Message(Chat::White, "Usage: #set zone locked [on|off] [Zone ID|Zone Short Name]");
			return;
		}

		auto pack = new ServerPacket(ServerOP_LockZone, sizeof(ServerLockZone_Struct));

		auto l = (ServerLockZone_Struct *) pack->pBuffer;
		strn0cpy(l->adminname, c->GetName(), sizeof(l->adminname));
		l->op     = is_locked ? ServerLockType::Lock : ServerLockType::Unlock;
		l->zoneID = zone_id;

		worldserver.SendPacket(pack);
		safe_delete(pack);
	} else if (is_safe_coordinates) {
		if (
			arguments < 3 ||
			!sep->IsNumber(3) ||
			!sep->IsNumber(4) ||
			!sep->IsNumber(5)
		) {
			c->Message(Chat::White, "Usage: #set zone safe_coordinates [X] [Y] [Z] [Heading] [Permanent (0 = False, 1 = True)]");
			c->Message(Chat::White, "Not sending Heading defaults to current Heading and the change is temporary.");
			return;
		}

		const float x         = Strings::ToFloat(sep->arg[3]);
		const float y         = Strings::ToFloat(sep->arg[4]);
		const float z         = Strings::ToFloat(sep->arg[5]);
		const float heading   = sep->arg[6] ? Strings::ToFloat(sep->arg[6]) : c->GetHeading();
		const bool  permanent = sep->arg[7] ? Strings::ToBool(sep->arg[7]) : false;

		if (permanent) {
			const std::string& query = fmt::format(
				"UPDATE zone SET safe_x = {:.2f}, safe_y = {:.2f}, safe_z = {:.2f}, safe_heading = {:.2f} WHERE zoneidnumber = {} AND version = {}",
				x,
				y,
				z,
				heading,
				zone->GetZoneID(),
				zone->GetInstanceVersion()
			);
			database.QueryDatabase(query);
		}

		zone->newzone_data.safe_x       = x;
		zone->newzone_data.safe_y       = y;
		zone->newzone_data.safe_z       = z;
		zone->newzone_data.safe_heading = heading;

		auto outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
		memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
		entity_list.QueueClients(c, outapp);
		safe_delete(outapp);

		c->Message(
			Chat::White,
			fmt::format(
				"Safe Coordinates Changed | Zone: {} XYZ: {:.2f}, {:.2f}, {:.2f} Heading: {:.2f} Permanent: {} ",
				zone->GetZoneDescription(),
				x,
				y,
				z,
				heading,
				permanent ? "Yes" : "No"
			).c_str()
		);
	} else if (is_sky) {
		if (arguments < 3 || !sep->IsNumber(3)) {
			c->Message(Chat::White, "Usage: #set zone sky [Sky Type] [Permanent (0 = False, 1 = True)]");
			return;
		}

		const uint8 sky_type  = Strings::ToUnsignedInt(sep->arg[3]);
		const bool  permanent = sep->arg[4] ? Strings::ToBool(sep->arg[4]) : false;

		if (!EQ::ValueWithin(sky_type, 0, UINT8_MAX)) {
			c->Message(Chat::White, "Sky Type cannot be less than 0 or greater than 255!");
			return;
		}

		if (permanent) {
			const std::string& query = fmt::format(
				"UPDATE zone SET sky = {} WHERE zoneidnumber = {} AND version = {}",
				sky_type,
				zone->GetZoneID(),
				zone->GetInstanceVersion()
			);
			database.QueryDatabase(query);
		}

		zone->newzone_data.sky = sky_type;
		auto outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
		memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
		entity_list.QueueClients(c, outapp);
		safe_delete(outapp);

		c->Message(
			Chat::White,
			fmt::format(
				"Sky Changed | Zone: {} Sky Type: {} Permanent: {}",
				zone->GetZoneDescription(),
				sky_type,
				permanent ? "Yes" : "No"
			).c_str()
		);
	} else if (is_underworld) {
		if (arguments < 3 || !sep->IsNumber(3)) {
			c->Message(Chat::White, "Usage: #set zone underworld [Z] [Permanent (0 = False, 1 = True)]");
			return;
		}

		const float z = Strings::ToFloat(sep->arg[3]);
		const bool permanent = sep->arg[4] ? Strings::ToBool(sep->arg[4]) : false;

		if (permanent) {
			auto query = fmt::format(
				"UPDATE zone SET underworld = {:.2f} WHERE zoneidnumber = {} AND version = {}",
				z,
				zone->GetZoneID(),
				zone->GetInstanceVersion()
			);
			database.QueryDatabase(query);
		}

		zone->newzone_data.underworld = z;
		auto outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
		memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
		entity_list.QueueClients(c, outapp);
		safe_delete(outapp);

		c->Message(
			Chat::White,
			fmt::format(
				"Underworld Z Changed | Zone: {} Z: {:.2f} Permanent: {}",
				zone->GetZoneDescription(),
				z,
				permanent ? "Yes" : "No"
			).c_str()
		);
	}
}
