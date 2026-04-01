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

void ShowQuestGlobals(Client *c, const Seperator *sep)
{
	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	QGlobalCache* char_cache = c->GetQGlobals();
	QGlobalCache* npc_cache  = t->IsNPC() ? t->CastToNPC()->GetQGlobals() : nullptr;
	QGlobalCache* zone_cache = zone->GetQGlobals();

	std::list<QGlobal> global_map;

	uint32 character_id = c->CharacterID();
	uint32 npc_id       = t->IsNPC() ? t->CastToNPC()->GetNPCTypeID() : 0;
	uint32 zone_id      = zone->GetZoneID();

	if (npc_cache) {
		QGlobalCache::Combine(
			global_map,
			npc_cache->GetBucket(),
			npc_id,
			character_id,
			zone_id
		);
	}

	if (char_cache) {
		QGlobalCache::Combine(
			global_map,
			char_cache->GetBucket(),
			npc_id,
			character_id,
			zone_id
		);
	}

	if (zone_cache) {
		QGlobalCache::Combine(
			global_map,
			zone_cache->GetBucket(),
			npc_id,
			character_id,
			zone_id
		);
	}

	uint32 global_count  = 0;
	uint32 global_number = 1;

	for (const auto& g : global_map) {
		c->Message(
			Chat::White,
			fmt::format(
				"Quest Global {} | Name: {} Value: {}",
				global_number,
				g.name,
				g.value
			).c_str()
		);

		global_count++;
		global_number++;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Quest Global{} found.",
			global_count,
			global_count != 1 ? "s" : ""
		).c_str()
	);
}
