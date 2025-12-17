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
};