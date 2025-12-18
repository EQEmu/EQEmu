#pragma once

#include "common/repositories/spawn2_disabled_repository.h"
#include "common/repositories/zone_state_spawns_repository.h"
#include "zone/corpse.h"
#include "zone/npc.h"
#include "zone/zone.h"
#include "cereal/archives/json.hpp"
#include "cereal/types/map.hpp"
#include <string>

struct LootEntryStateData {
	uint32   item_id     = 0;
	uint32_t lootdrop_id = 0;
	uint16   charges     = 0; // used in dynamically added loot (AddItem)

	// cereal
	template<class Archive>
	void serialize(Archive &ar)
	{
		ar(
			CEREAL_NVP(item_id),
			CEREAL_NVP(lootdrop_id),
			CEREAL_NVP(charges)
		);
	}
};

struct LootStateData {
	uint32                          copper   = 0;
	uint32                          silver   = 0;
	uint32                          gold     = 0;
	uint32                          platinum = 0;
	std::vector<LootEntryStateData> entries  = {};

	// cereal
	template<class Archive>
	void serialize(Archive &ar)
	{
		ar(
			CEREAL_NVP(copper),
			CEREAL_NVP(silver),
			CEREAL_NVP(gold),
			CEREAL_NVP(platinum),
			CEREAL_NVP(entries)
		);
	}
};
