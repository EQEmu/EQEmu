#include "../../client.h"

void FindFaction(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(2)) {
		const auto  faction_id   = Strings::ToInt(sep->arg[2]);
		const auto& faction_name = content_db.GetFactionName(faction_id);
		if (!faction_name.empty()) {
			c->Message(
				Chat::White,
				fmt::format(
					"Faction {} | {}",
					faction_id,
					faction_name
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Faction ID {} was not found.",
				faction_id
			).c_str()
		);

		return;
	}

	const auto& search_criteria = Strings::ToLower(sep->argplus[2]);
	auto        found_count     = 0;
	const auto  max_faction_id  = content_db.GetMaxFaction();

	for (uint32 faction_id = 0; faction_id < max_faction_id; faction_id++) {
		const auto& faction_name       = content_db.GetFactionName(faction_id);
		const auto& faction_name_lower = Strings::ToLower(faction_name);
		if (faction_name.empty()) {
			continue;
		}

		if (!Strings::Contains(faction_name_lower, search_criteria)) {
			continue;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Faction {} | {}",
				faction_id,
				faction_name
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
				"50 Factions found matching '{}', max reached.",
				sep->argplus[2]
			).c_str()
		);

		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Faction{} found matching '{}'.",
			found_count,
			found_count != 1 ? "s" : "",
			sep->argplus[2]
		).c_str()
	);
}
