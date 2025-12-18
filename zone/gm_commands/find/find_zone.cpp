#include "common/content/world_content_service.h"
#include "zone/client.h"

void FindZone(Client* c, const Seperator* sep)
{
	std::string where_filter = std::string();

	const bool is_expansion_search  = !strcasecmp(sep->arg[2], "expansion");
	const bool is_id_search         = Strings::IsNumber(sep->arg[2]);
	const bool is_short_name_search = !is_expansion_search && !is_id_search;

	std::string search_string;
	std::string search_type;

	if (is_expansion_search) {
		int expansion_id = Strings::ToInt(sep->arg[3]);

		if (
			!EQ::ValueWithin(expansion_id, Expansion::Classic, Expansion::MaxId - 1) &&
			expansion_id != Expansion::EXPANSION_FILTER_MAX
		) {
			c->Message(
				Chat::White,
				fmt::format(
					"Invalid expansion ID: {}. Please enter a value between 0 and {}, or 99.",
					expansion_id,
					Expansion::MaxId - 1
				).c_str()
			);
			return;
		}

		where_filter = fmt::format("expansion = {}", expansion_id);

		if (expansion_id == Expansion::EXPANSION_FILTER_MAX) {
			search_string = "Test Zones";
		} else {
			search_string = Expansion::ExpansionName[expansion_id];
		}

		search_type = "expansion";
	} else if (is_id_search) {
		where_filter = fmt::format("zoneidnumber = {}", Strings::ToUnsignedInt(sep->arg[2]));

		search_string = sep->arg[2];
		search_type   = "ID";
	} else if (is_short_name_search) {
		where_filter = fmt::format(
			"LOWER(`long_name`) LIKE '%%{}%%' OR LOWER(`short_name`) LIKE '%%{}%%'",
			Strings::Escape(Strings::ToLower(sep->argplus[2])),
			Strings::Escape(Strings::ToLower(sep->argplus[2]))
		);

		search_string = sep->argplus[2];
		search_type   = "name";
	}

	where_filter += " ORDER BY `zoneidnumber` ASC LIMIT 50";

	const auto& l = ZoneRepository::GetWhere(content_db, where_filter);

	if (l.empty()) {
		c->Message(Chat::White, "No zones were found matching your search criteria.");
		return;
	}

	uint32 found_count = 0;

	for (const auto& e : l) {
		c->Message(
			Chat::White,
			fmt::format(
				"{}{} {} ({}) (ID {}){}",
				(
					e.version == 0 ?
					fmt::format(
						"{} | ",
						Saylink::Silent(
							fmt::format(
								"#zone {}",
								e.short_name
							),
							"Zone"
						)
					) :
					""
				),
				fmt::format(
					"{} |",
					Saylink::Silent(
						fmt::format(
							"#gmzone {} {}",
							e.short_name,
							e.version
						),
						"GM Zone"
					)
				),
				e.long_name,
				e.short_name,
				e.zoneidnumber,
				(
					e.version != 0 ?
					fmt::format(
						" (Version {})",
						e.version
					) :
					""
				)
			).c_str()
		);

		found_count++;

		if (found_count == 50) {
			break;
		}
	}

	if (found_count == 50) {
		c->Message(
			Chat::White,
			fmt::format(
				"50 Zones found matching '{}' of '{}', max reached.",
				search_type,
				search_string
			).c_str()
		);

		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Zone{} found matching '{}' of '{}'.",
			found_count,
			found_count != 1 ? "s" : "",
			search_type,
			search_string
		).c_str()
	);
}
