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

#include "common/repositories/base/base_bot_inventories_repository.h"

#include "common/database.h"
#include "common/strings.h"

class BotInventoriesRepository: public BaseBotInventoriesRepository {
public:

    /**
     * This file was auto generated and can be modified and extended upon
     *
     * Base repository methods are automatically
     * generated in the "base" version of this repository. The base repository
     * is immutable and to be left untouched, while methods in this class
     * are used as extension methods for more specific persistence-layer
     * accessors or mutators.
     *
     * Base Methods (Subject to be expanded upon in time)
     *
     * Note: Not all tables are designed appropriately to fit functionality with all base methods
     *
     * InsertOne
     * UpdateOne
     * DeleteOne
     * FindOne
     * GetWhere(std::string where_filter)
     * DeleteWhere(std::string where_filter)
     * InsertMany
     * All
     *
     * Example custom methods in a repository
     *
     * BotInventoriesRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * BotInventoriesRepository::GetWhereNeverExpires()
     * BotInventoriesRepository::GetWhereXAndY()
     * BotInventoriesRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here
	static bool UpdateItemColors(Database& db, const uint32 bot_id, const uint32 color, const std::string& where_clause)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE `{}` SET `inst_color` = {} WHERE `bot_id` = {} AND `slot_id` {}",
				TableName(),
				color,
				bot_id,
				where_clause
			)
		);

		return results.Success();
	}

	static bool SaveAllArmorColors(Database& db, const uint32 owner_id, const uint32 color)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE `{}` SET `inst_color` = {} WHERE `slot_id` IN ({}, {}, {}, {}, {}, {}, {}) AND `bot_id` IN (SELECT `bot_id` FROM `bot_data` WHERE `owner_id` = {})",
				TableName(),
				color,
				EQ::invslot::slotHead,
				EQ::invslot::slotChest,
				EQ::invslot::slotArms,
				EQ::invslot::slotWrist1,
				EQ::invslot::slotWrist2,
				EQ::invslot::slotHands,
				EQ::invslot::slotLegs,
				EQ::invslot::slotFeet,
				owner_id
			)
		);

		return results.Success();
	}

	static bool SaveAllArmorColorsBySlot(Database& db, const uint32 owner_id, const int16 slot_id, const uint32 color)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE `{}` SET `inst_color` = {} WHERE `slot_id` = {} AND `bot_id` IN (SELECT `bot_id` FROM `bot_data` WHERE `owner_id` = {})",
				TableName(),
				color,
				slot_id,
				owner_id
			)
		);

		return results.Success();
	}
};
