#pragma once

#include "common/repositories/base/base_spawn2_repository.h"

#include "common/database.h"
#include "common/strings.h"

class Spawn2Repository: public BaseSpawn2Repository {
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
     * Spawn2Repository::GetByZoneAndVersion(int zone_id, int zone_version)
     * Spawn2Repository::GetWhereNeverExpires()
     * Spawn2Repository::GetWhereXAndY()
     * Spawn2Repository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here
	static uint32 GetPathGridBySpawn2ID(Database& db, uint32 spawn2_id)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT `pathgrid` FROM `{}` WHERE `id` = {}",
				TableName(),
				spawn2_id
			)
		);

		if (!results.Success() || !results.RowCount()) {
			return 0;
		}

		auto row = results.begin();

		return Strings::ToUnsignedInt(row[0]);
	}

	static void SetPathGridBySpawn2ID(Database& db, uint32 spawn2_id, uint32 grid_id)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE `spawn2` SET `pathgrid` = {} WHERE `id` = {}",
				grid_id,
				spawn2_id
			)
		);
	}
};
