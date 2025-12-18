#include "zone/client.h"

void command_flagedit(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		auto flags_link = Saylink::Silent("#flags");
		c->Message(
			Chat::White,
			"Usage: #flagedit lock [Zone ID|Zone Short Name] [Flag Name] - Set the specified flag name on the zone, locking the zone"
		);
		c->Message(
			Chat::White,
			"Usage: #flagedit unlock [Zone ID|Zone Short Name] - Removes the flag requirement from the specified zone"
		);
		c->Message(
			Chat::White,
			"Usage: #flagedit list - List all zones which require a flag, and their flag's name"
		);
		c->Message(
			Chat::White,
			"Usage: #flagedit give [Zone ID|Zone Short Name] - Give your target the zone flag for the specified zone."
		);
		c->Message(
			Chat::White,
			"Usage: #flagedit take [Zone ID|Zone Short Name] - Take the zone flag for the specified zone away from your target"
		);
		c->Message(
			Chat::White,
			fmt::format(
				"Note: Use {} to view the flags a player has.",
				flags_link
			).c_str()
		);
		return;
	}

	bool is_give = !strcasecmp(sep->arg[1], "give");
	bool is_list = !strcasecmp(sep->arg[1], "list");
	bool is_lock = !strcasecmp(sep->arg[1], "lock");
	bool is_take = !strcasecmp(sep->arg[1], "take");
	bool is_unlock = !strcasecmp(sep->arg[1], "unlock");

	if (
		!is_give &&
		!is_list &&
		!is_lock &&
		!is_take &&
		!is_unlock
	) {
		auto flags_link = Saylink::Silent("#flags");
		c->Message(
			Chat::White,
			"Usage: #flagedit lock [Zone ID|Zone Short Name] [Flag Name] - Set the specified flag name on the zone, locking the zone"
		);
		c->Message(
			Chat::White,
			"Usage: #flagedit unlock [Zone ID|Zone Short Name] - Removes the flag requirement from the specified zone"
		);
		c->Message(
			Chat::White,
			"Usage: #flagedit list - List all zones which require a flag, and their flag's name"
		);
		c->Message(
			Chat::White,
			"Usage: #flagedit give [Zone ID|Zone Short Name] - Give your target the zone flag for the specified zone."
		);
		c->Message(
			Chat::White,
			"Usage: #flagedit take [Zone ID|Zone Short Name] - Take the zone flag for the specified zone away from your target"
		);
		c->Message(
			Chat::White,
			fmt::format(
				"Note: Use {} to view the flags a player has.",
				flags_link
			).c_str()
		);
		return;
	}

	if (is_give) {
		uint32 zone_id = (
			sep->IsNumber(2) ?
			Strings::ToUnsignedInt(sep->arg[2]) :
			ZoneID(sep->arg[2])
		);
		std::string zone_short_name = Strings::ToLower(ZoneName(zone_id, true));
		bool is_unknown_zone = zone_short_name.find("unknown") != std::string::npos;
		if (zone_id && !is_unknown_zone) {
			std::string zone_long_name = ZoneLongName(zone_id);
			auto target = c;
			if (c->GetTarget() && c->GetTarget()->IsClient()) {
				target = c->GetTarget()->CastToClient();
			}

			target->SetZoneFlag(zone_id);
			c->Message(
				Chat::White,
				fmt::format(
					"{} now {} the flag for {} ({}).",
					c->GetTargetDescription(target, TargetDescriptionType::UCYou),
					c == target ? "have" : "has",
					zone_long_name,
					zone_id
				).c_str()
			);
			return;
		}
	} else if (is_list) {
		std::string query = SQL(
			SELECT long_name, zoneidnumber, version, flag_needed
			FROM zone
			WHERE flag_needed != ''
			ORDER BY long_name ASC
		);
		auto results = content_db.QueryDatabase(query);
		if (!results.Success()) {
			return;
		}

		std::string popup_text = "<table>";

		popup_text += "<tr><td>Zone</td><td>Flag Required</td></tr>";

		for (auto row : results) {
			popup_text += fmt::format(
				"<tr><td>{} ({}){}</td><td>{}</td></tr>",
				row[0],
				row[1],
				(
					Strings::ToInt(row[2]) != 0 ?
					fmt::format(
						"[Version {}]",
						row[2]
					) :
					""
				),
				row[3]
			);
		}

		popup_text += "</table>";

		c->SendPopupToClient(
			"Zone Flags",
			popup_text.c_str()
		);

		return;
	} else if (is_lock) {
		uint32 zone_id = (
			sep->IsNumber(2) ?
			Strings::ToUnsignedInt(sep->arg[2]) :
			ZoneID(sep->arg[2])
		);
		std::string zone_short_name = Strings::ToLower(ZoneName(zone_id, true));
		bool is_unknown_zone = zone_short_name.find("unknown") != std::string::npos;
		if (zone_id && !is_unknown_zone) {
			if (arguments < 3) {
				c->Message(
					Chat::White,
					"Usage: #flagedit lock [Zone ID|Zone Short Name] [Flag Name] - Set the specified flag name on the zone, locking the zone"
				);
				return;
			}

			std::string flag_name = Strings::Escape(sep->argplus[3]);
			std::string zone_long_name = ZoneLongName(zone_id);

			auto query = fmt::format(
				SQL(
					UPDATE zone
					SET flag_needed = '{}'
					WHERE zoneidnumber = {} AND version = {}
				),
				flag_name,
				zone_id,
				zone->GetInstanceVersion()
			);

			auto results = content_db.QueryDatabase(query);
			if (!results.Success()) {
				c->Message(
					Chat::White,
					fmt::format(
						"Error updating zone flag for {} ({}).",
						zone_long_name,
						zone_id
					).c_str()
				);
				return;
			}

			c->Message(
				Chat::White,
				fmt::format(
					"{} ({}) now requires a flag, named {}.",
					zone_long_name,
					zone_id,
					flag_name
				).c_str()
			);
			return;
		}
	} else if (is_take) {
		uint32 zone_id = (
			sep->IsNumber(2) ?
			Strings::ToUnsignedInt(sep->arg[2]) :
			ZoneID(sep->arg[2])
		);
		std::string zone_short_name = Strings::ToLower(ZoneName(zone_id, true));
		bool is_unknown_zone = zone_short_name.find("unknown") != std::string::npos;
		if (zone_id && !is_unknown_zone) {
			std::string zone_long_name = ZoneLongName(zone_id);
			auto target = c;
			if (c->GetTarget() && c->GetTarget()->IsClient()) {
				target = c->GetTarget()->CastToClient();
			}

			target->ClearZoneFlag(zone_id);
			c->Message(
				Chat::White,
				fmt::format(
					"{} no longer {} the flag for {} ({}).",
					c->GetTargetDescription(target, TargetDescriptionType::UCYou),
					c == target ? "have" : "has",
					zone_long_name,
					zone_id
				).c_str()
			);
			return;
		}
	} else if (is_unlock) {
		uint32 zone_id = (
			sep->IsNumber(2) ?
			Strings::ToUnsignedInt(sep->arg[2]) :
			ZoneID(sep->arg[2])
		);
		std::string zone_short_name = Strings::ToLower(ZoneName(zone_id, true));
		bool is_unknown_zone = zone_short_name.find("unknown") != std::string::npos;
		if (zone_id && !is_unknown_zone) {
			std::string zone_long_name = ZoneLongName(zone_id);
			auto query = fmt::format(
				SQL(
					UPDATE zone
					SET flag_needed = ''
					WHERE zoneidnumber = {} AND version = {}
				),
				zone_id,
				zone->GetInstanceVersion()
			);
			auto results = content_db.QueryDatabase(query);
			if (!results.Success()) {
				c->Message(
					Chat::White,
					fmt::format(
						"Error updating zone flag for {} ({}).",
						zone_long_name,
						zone_id
					).c_str()
				);
				return;
			}

			c->Message(
				Chat::White,
				fmt::format(
					"{} ({}) no longer requires a flag.",
					zone_long_name,
					zone_id
				).c_str()
			);
			return;
		}
	}
}

