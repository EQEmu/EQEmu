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

void command_npctypespawn(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #npctypespawn [NPC ID] [Faction ID]");
		return;
	}

	auto npc_id = Strings::ToUnsignedInt(sep->arg[1]);
	int faction_id = 0;

	auto npc_type = content_db.LoadNPCTypesData(npc_id);
	if (npc_type) {
		auto npc = new NPC(npc_type, 0, c->GetPosition(), GravityBehavior::Water);
		if (npc) {
			if (sep->IsNumber(2)) {
				faction_id = Strings::ToInt(sep->arg[2]);
				npc->SetNPCFactionID(faction_id);
			}

			npc->AddLootTable();
			if (npc->DropsGlobalLoot()) {
				npc->CheckGlobalLootTables();
			}
			entity_list.AddNPC(npc);

			c->Message(
				Chat::White,
				fmt::format(
					"Spawned {} ({}){}.",
					npc->GetCleanName(),
					npc_id,
					(
						faction_id ?
						fmt::format(
							" on the {} Faction ({})",
							content_db.GetFactionName(faction_id),
							faction_id
						) :
						""
					)
				).c_str()
			);
		} else {
			c->Message(
				Chat::White,
				fmt::format(
					"Failed to spawn NPC ID {}.",
					npc_id
				).c_str()
			);
		}
	} else {
		c->Message(
			Chat::White,
			fmt::format(
				"NPC ID {} was not found.",
				npc_id
			).c_str()
		);
	}
}

