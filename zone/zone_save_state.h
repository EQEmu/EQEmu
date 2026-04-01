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
