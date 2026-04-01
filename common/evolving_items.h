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

#include "common/events/player_events.h"
#include "common/repositories/items_evolving_details_repository.h"
#include "common/shareddb.h"

namespace EQ {
	class ItemInstance;
}

namespace EvolvingItems {
	namespace Actions {
		constexpr int8 UPDATE_ITEMS            = 0;
		constexpr int8 TRANSFER_WINDOW_OPEN    = 1;
		constexpr int8 TRANSFER_WINDOW_DETAILS = 2;
		constexpr int8 TRANSFER_XP             = 3;
		constexpr int8 FINAL_RESULT            = 4;
	}

	namespace Types {
		constexpr int8 AMOUNT_OF_EXP     = 1;
		constexpr int8 NUMBER_OF_KILLS   = 2;
		constexpr int8 SPECIFIC_MOB_RACE = 3;
		constexpr int8 SPECIFIC_ZONE_ID  = 4;
	}

	namespace SubTypes {
		constexpr int8 ALL_EXP   = 0;
		constexpr int8 SOLO_EXP  = 1;
		constexpr int8 GROUP_EXP = 2;
		constexpr int8 RAID_EXP  = 3;
	}
}

class EvolvingItemsManager
{
public:
	EvolvingItemsManager();
	void SetDatabase(Database *db);
	void SetContentDatabase(Database *db);

	void LoadEvolvingItems() const;
	void DoLootChecks(uint32 char_id, uint16 slot_id, const EQ::ItemInstance &inst) const;
	uint32 GetFinalItemID(const EQ::ItemInstance &inst) const;
	uint32 GetNextEvolveItemID(const EQ::ItemInstance &inst) const;
	uint32 GetFirstItemInLoreGroup(uint32 lore_id);
	uint32 GetFirstItemInLoreGroupByItemID(uint32 item_id);
	uint64 GetTotalEarnedXP(const EQ::ItemInstance &inst);
	static double CalculateProgression(uint64 current_amount, uint32 item_id);
	static void LoadPlayerEvent(const EQ::ItemInstance &inst, PlayerEvent::EvolveItem &e);

	ItemsEvolvingDetailsRepository::ItemsEvolvingDetails GetEvolveItemDetails(uint64 id);
	EvolveTransfer DetermineTransferResults(const EQ::ItemInstance& inst_from, const EQ::ItemInstance& inst_to);
	EvolveGetNextItem GetNextItemByXP(const EQ::ItemInstance &inst_in, int64 in_xp);
	std::map<uint32, ItemsEvolvingDetailsRepository::ItemsEvolvingDetails>& GetEvolvingItemsCache() { return m_evolving_items_cache; }
	std::vector<ItemsEvolvingDetailsRepository::ItemsEvolvingDetails> GetEvolveIDItems(uint32 evolve_id);

	static EvolvingItemsManager* Instance()
	{
		static EvolvingItemsManager instance;
		return &instance;
	}
private:
	std::map<uint32, ItemsEvolvingDetailsRepository::ItemsEvolvingDetails> m_evolving_items_cache;
	Database *                                                             m_db;
	Database *                                                             m_content_db;
};
