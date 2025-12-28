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

#include "common/types.h"

#include <list>
#include <string>

struct LootItem {
	uint32      item_id;
	int16       equip_slot;
	uint16      charges;
	uint16      lootslot;
	uint32      aug_1;
	uint32      aug_2;
	uint32      aug_3;
	uint32      aug_4;
	uint32      aug_5;
	uint32      aug_6;
	bool        attuned;
	std::string custom_data;
	uint32      ornamenticon{};
	uint32      ornamentidfile{};
	uint32      ornament_hero_model{};
	uint16      trivial_min_level;
	uint16      trivial_max_level;
	uint16      npc_min_level;
	uint16      npc_max_level;
	uint32      lootdrop_id; // required for zone state referencing
};

using LootItems = std::list<LootItem*>;
