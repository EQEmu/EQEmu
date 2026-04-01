/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
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
