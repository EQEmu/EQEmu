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

#include "common/repositories/base/base_lootdrop_entries_repository.h"

#include "common/database.h"
#include "common/strings.h"

class LootdropEntriesRepository: public BaseLootdropEntriesRepository {
public:

	static LootdropEntries NewNpcEntity()
	{
		LootdropEntries e{};

		e.lootdrop_id       = 0;
		e.item_id           = 0;
		e.item_charges      = 1;
		e.equip_item        = 1;
		e.chance            = 0;
		e.disabled_chance   = 0;
		e.trivial_min_level = 0;
		e.trivial_max_level = 0;
		e.multiplier        = 0;
		e.npc_min_level     = 0;
		e.npc_max_level     = 0;

		return e;
	}

};
