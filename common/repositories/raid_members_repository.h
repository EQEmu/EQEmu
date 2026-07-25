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

#include "common/repositories/base/base_raid_members_repository.h"

#include "common/database.h"
#include "common/strings.h"

class RaidMembersRepository: public BaseRaidMembersRepository {
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
     * RaidMembersRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * RaidMembersRepository::GetWhereNeverExpires()
     * RaidMembersRepository::GetWhereXAndY()
     * RaidMembersRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here
	static int UpdateRaidNote(
		Database& db,
		int32_t raid_id,
		const std::string& note,
		const std::string& character_name
	) {
		auto results = db.QueryDatabase(
			fmt::format("UPDATE `{}` SET `note` = '{}' WHERE raidid = '{}' AND name = '{}';",
				TableName(),
				Strings::Escape(note),
				raid_id,
				Strings::Escape(character_name)
			)
		);
		return results.Success() ? results.RowsAffected() : 0;
	}

	static int UpdateRaidAssister(
		Database& db,
		int32_t raid_id,
		const std::string& character_name,
		uint8_t value
	) {
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE `{}` SET `is_assister` = '{}' WHERE raidid = '{}' AND `name` = '{}';",
				TableName(),
				value,
				raid_id,
				Strings::Escape(character_name)
			)
		);

		return results.Success() ? results.RowsAffected() : 0;
	}

	static int UpdateRaidMarker(
		Database& db,
		int32_t raid_id,
		const std::string& character_name,
		uint8_t value
	) {
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE `{}` SET `is_marker` = '{}' WHERE raidid = '{}' AND `name` = '{}';",
				TableName(),
				value,
				raid_id,
				Strings::Escape(character_name)
			)
		);

		return results.Success() ? results.RowsAffected() : 0;
	}

	static void ClearAllRaids(Database& db)
	{
		db.QueryDatabase(
			fmt::format(
				"DELETE FROM `{}`",
				TableName()
			)
		);
	}

	static int ReplaceMember(
		Database& db,
		int32_t raid_id,
		int32_t character_id,
		int32_t bot_id,
		uint32_t group_id,
		int8_t class_,
		int8_t level,
		const std::string& character_name,
		int8_t is_group_leader,
		int8_t is_raid_leader,
		int8_t is_looter
	) {
		auto results = db.QueryDatabase(
			fmt::format(
				"REPLACE INTO `{}` SET raidid = {}, charid = {}, bot_id = {}, "
				"groupid = {}, _class = {}, level = {}, name = '{}', "
				"isgroupleader = {}, israidleader = {}, islooter = {};",
				TableName(),
				raid_id,
				character_id,
				bot_id,
				group_id,
				class_,
				level,
				Strings::Escape(character_name),
				is_group_leader,
				is_raid_leader,
				is_looter
			)
		);

		return results.Success() ? results.RowsAffected() : 0;
	}

	static int UpdateGroupId(
		Database& db,
		const std::string& character_name,
		uint32_t group_id
	) {
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE `{}` SET `groupid` = {} WHERE `name` = '{}';",
				TableName(),
				group_id,
				Strings::Escape(character_name)
			)
		);

		return results.Success() ? results.RowsAffected() : 0;
	}

	static int UpdateGroupLeaderFlag(
		Database& db,
		const std::string& character_name,
		int8_t value
	) {
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE `{}` SET `isgroupleader` = {} WHERE `name` = '{}';",
				TableName(),
				value,
				Strings::Escape(character_name)
			)
		);

		return results.Success() ? results.RowsAffected() : 0;
	}

	static int UpdateRaidLeaderFlag(
		Database& db,
		const std::string& character_name,
		int8_t value
	) {
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE `{}` SET `israidleader` = {} WHERE `name` = '{}';",
				TableName(),
				value,
				Strings::Escape(character_name)
			)
		);

		return results.Success() ? results.RowsAffected() : 0;
	}

	static int UpdateMemberLevel(
		Database& db,
		const std::string& character_name,
		int8_t level
	) {
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE `{}` SET `level` = {} WHERE `name` = '{}';",
				TableName(),
				level,
				Strings::Escape(character_name)
			)
		);

		return results.Success() ? results.RowsAffected() : 0;
	}

	static int UpdateLooterFlag(
		Database& db,
		const std::string& character_name,
		int8_t value
	) {
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE `{}` SET `islooter` = {} WHERE `name` = '{}';",
				TableName(),
				value,
				Strings::Escape(character_name)
			)
		);

		return results.Success() ? results.RowsAffected() : 0;
	}

	static int ClearAllLooters(Database& db, int32_t raid_id)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE `{}` SET `islooter` = 0 WHERE `raidid` = {};",
				TableName(),
				raid_id
			)
		);

		return results.Success() ? results.RowsAffected() : 0;
	}

	static int ClearMasterLooter(Database& db, int32_t raid_id)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE `{}` SET `is_master_looter` = 0 WHERE `raidid` = {};",
				TableName(),
				raid_id
			)
		);

		return results.Success() ? results.RowsAffected() : 0;
	}

	static int UpdateMasterLooterFlag(
		Database& db,
		int32_t raid_id,
		const std::string& character_name,
		int8_t value
	) {
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE `{}` SET `is_master_looter` = {} WHERE `raidid` = {} AND `name` = '{}';",
				TableName(),
				value,
				raid_id,
				Strings::Escape(character_name)
			)
		);

		return results.Success() ? results.RowsAffected() : 0;
	}
};