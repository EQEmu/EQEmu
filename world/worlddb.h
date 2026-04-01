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

#include "common/eq_packet.h"
#include "common/repositories/character_data_repository.h"
#include "common/repositories/inventory_repository.h"
#include "common/shareddb.h"

struct PlayerProfile_Struct;
struct CharCreate_Struct;
struct CharacterSelect_Struct;


class WorldDatabase : public SharedDatabase {
public:
	bool GetStartZone(PlayerProfile_Struct* pp, CharCreate_Struct* p_char_create_struct, bool is_titanium);
	void GetCharSelectInfo(uint32 account_id, EQApplicationPacket **out_app, uint32 client_version_bit);
	int MoveCharacterToBind(int character_id, uint8 bind_number = 0);
	int MoveCharacterToInstanceSafeReturn(int character_id, int instance_zone_id, int instance_id);

	void GetLauncherList(std::vector<std::string> &result);
	bool GetCharacterLevel(const char *name, int &level);

	bool LoadCharacterCreateAllocations();
	bool LoadCharacterCreateCombos();

private:
	void SetTitaniumDefaultStartZone(PlayerProfile_Struct* in_pp, CharCreate_Struct* in_cc);
	void SetSoFDefaultStartZone(PlayerProfile_Struct* in_pp, CharCreate_Struct* in_cc);

	bool GetCharSelInventory(
		const std::vector<InventoryRepository::Inventory> &inventories,
		const CharacterDataRepository::CharacterData &character,
		EQ::InventoryProfile *inv
	);
};

extern WorldDatabase database;
extern WorldDatabase content_db;
