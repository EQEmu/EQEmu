#include "zone/client.h"

void command_killallnpcs(Client *c, const Seperator *sep)
{
	std::string search_string;
	if (sep->arg[1]) {
		search_string = Strings::ToLower(sep->arg[1]);
	}

	int killed_count = 0;

	for (auto &e: entity_list.GetMobList()) {
		auto *entity = e.second;
		if (!entity || !entity->IsNPC()) {
			continue;
		}

		std::string entity_name = Strings::ToLower(entity->GetName());
		if ((!search_string.empty() && entity_name.find(search_string) == std::string::npos) ||
			!entity->IsAttackAllowed(c)) {
			continue;
		}

		entity->Damage(
			c,
			entity->GetHP() + 1000,
			SPELL_UNKNOWN,
			EQ::skills::SkillDragonPunch
		);

		killed_count++;
	}

	if (killed_count) {
		c->Message(
			Chat::White,
			fmt::format(
				"Killed {} NPC{}{}.",
				killed_count,
				killed_count != 1 ? "s" : "",
				(
					!search_string.empty() ?
						fmt::format(
							" that matched '{}'",
							search_string
						) :
						""
				)
			).c_str()
		);
	}
	else {
		c->Message(
			Chat::White,
			fmt::format(
				"There were no NPCs to kill{}.",
				(
					!search_string.empty() ?
						fmt::format(
							" that matched '{}'",
							search_string
						) :
						""
				)
			).c_str()
		);
	}
}
