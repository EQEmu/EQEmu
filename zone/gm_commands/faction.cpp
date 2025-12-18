#include "zone/client.h"

void command_faction(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(
			Chat::White,
			"Usage: #faction review [Search Criteria | All] - Review Targeted Player's Faction Hits"
		);
		c->Message(
			Chat::White,
			"Usage: #faction reset [Faction ID] - Reset Targeted Player's Faction to Base Faction Value"
		);
		c->Message(Chat::White, "Usage: #faction view - Displays Target NPC's Primary Faction");
		return;
	}

	std::string faction_filter;
	if (sep->arg[2]) {
		faction_filter = Strings::ToLower(sep->arg[2]);
	}

	if (!strcasecmp(sep->arg[1], "review")) {
		if (!(c->GetTarget() && c->GetTarget()->IsClient())) {
			c->Message(Chat::Red, "Player Target Required for faction review");
			return;
		}

		Client      *target      = c->GetTarget()->CastToClient();
		uint32      character_id = target->CharacterID();
		std::string query;
		if (!strcasecmp(faction_filter.c_str(), "all")) {
			query = fmt::format(
				"SELECT id, `name`, current_value FROM faction_list INNER JOIN faction_values ON faction_list.id = faction_values.faction_id WHERE char_id = {}",
				character_id
			);
		}
		else {
			query = fmt::format(
				"SELECT id, `name`, current_value FROM faction_list INNER JOIN faction_values ON faction_list.id = faction_values.faction_id WHERE `name` like '%{}%' and char_id = {}",
				faction_filter.c_str(),
				character_id
			);
		}

		auto results = content_db.QueryDatabase(query);
		if (!results.Success() || !results.RowCount()) {
			c->Message(Chat::Yellow, "No faction hits found. All are at base level.");
			return;
		}

		uint32    found_count = 0;
		for (auto row : results) {
			uint32      faction_number = (found_count + 1);
			auto        faction_id     = Strings::ToUnsignedInt(row[0]);
			std::string faction_name   = row[1];
			std::string faction_value  = row[2];
			std::string reset_link     = Saylink::Silent(
				fmt::format("#faction reset {}", faction_id),
				"Reset"
			);

			c->Message(
				Chat::White,
				fmt::format(
					"Faction {} | Name: {} ({}) Value: {} [{}]",
					faction_number,
					faction_name,
					faction_id,
					faction_value,
					reset_link
				).c_str()
			);
			found_count++;
		}

		auto faction_message = (
			found_count > 0 ?
				(
					found_count == 1 ?
						"A Faction was" :
						fmt::format("{} Factions were", found_count)
				) :
				"No Factions were"
		);
		c->Message(
			Chat::White,
			fmt::format(
				"{} found.",
				faction_message
			).c_str()
		);
	}
	else if (!strcasecmp(sep->arg[1], "reset")) {
		if (strlen(faction_filter.c_str()) > 0) {
			if (c->GetTarget() && c->GetTarget()->IsClient()) {
				Client *target = c->GetTarget()->CastToClient();
				if (
					(
						!c->GetFeigned() &&
						c->GetAggroCount() == 0
					) ||
					(
						!target->GetFeigned() &&
						target->GetAggroCount() == 0
					)
					) {
					uint32 character_id = target->CharacterID();
					uint32 faction_id   = Strings::ToUnsignedInt(faction_filter);
					if (target->ReloadCharacterFaction(target, faction_id, character_id)) {
						c->Message(
							Chat::White,
							fmt::format(
								"Faction Reset | {} ({}) was reset for {}.",
								content_db.GetFactionName(faction_id),
								faction_id,
								target->GetCleanName()
							).c_str()
						);
					}
					else {
						c->Message(
							Chat::White,
							fmt::format(
								"Faction Reset Failed | {} ({}) was unable to be reset for {}.",
								content_db.GetFactionName(faction_id),
								faction_id,
								target->GetCleanName()
							).c_str()
						);
					}
				}
				else {
					c->Message(
						Chat::White,
						"You cannot reset factions while you or your target is in combat or feigned."
					);
					return;
				}
			}
			else {
				c->Message(Chat::White, "You must target a PC for this command.");
				return;
			}
		}
		else {
			c->Message(
				Chat::White,
				"Usage: #faction reset [Faction ID] - Reset Targeted Player's Faction to Base Faction Value"
			);
		}
	}
	else if (!strcasecmp(sep->arg[1], "view")) {
		if (c->GetTarget() && c->GetTarget()->IsNPC()) {
			auto target = c->GetTarget();
			auto npc_faction_id = target->CastToNPC()->GetPrimaryFaction();
			c->Message(
				Chat::White,
				fmt::format(
					"{} has a Primary Faction of {} ({}).",
					c->GetTargetDescription(target),
					content_db.GetFactionName(npc_faction_id),
					npc_faction_id
				).c_str()
			);
		}
	}
}

