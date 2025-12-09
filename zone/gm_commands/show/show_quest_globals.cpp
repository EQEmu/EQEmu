#include "../../client.h"

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
