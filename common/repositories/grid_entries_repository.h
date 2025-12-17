#pragma once

#include "common/repositories/base/base_grid_entries_repository.h"

#include "common/database.h"
#include "common/strings.h"

class GridEntriesRepository: public BaseGridEntriesRepository {
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
     * GridEntriesRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * GridEntriesRepository::GetWhereNeverExpires()
     * GridEntriesRepository::GetWhereXAndY()
     * GridEntriesRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here

	static std::vector<GridEntries> GetZoneGridEntries(Database& db, int zone_id)
	{
		std::vector<GridEntries> grid_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE zoneid = {} ORDER BY gridid, number",
				BaseSelect(),
				zone_id
			)
		);

		for (auto row = results.begin(); row != results.end(); ++row) {
			GridEntries entry{};

			entry.gridid      = atoi(row[0]);
			entry.zoneid      = atoi(row[1]);
			entry.number      = atoi(row[2]);
			entry.x           = static_cast<float>(atof(row[3]));
			entry.y           = static_cast<float>(atof(row[4]));
			entry.z           = static_cast<float>(atof(row[5]));
			entry.heading     = static_cast<float>(atof(row[6]));
			entry.pause       = atoi(row[7]);
			entry.centerpoint = atoi(row[8]);

			grid_entries.push_back(entry);
		}

		return grid_entries;
	}

	static int GetHighestWaypoint(Database& db, uint32 zone_id, uint32 grid_id)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				SQL(
					SELECT COALESCE(MAX(`number`), 0) FROM `{}`
					WHERE `zoneid` = {} AND `gridid` = {}
				),
				TableName(),
				zone_id,
				grid_id
			)
		);

		if (!results.Success() || !results.RowCount()) {
			return 0;
		}

		auto row = results.begin();

		return Strings::ToInt(row[0]);
	}

	static int GetNextWaypoint(Database& db, uint32 zone_id, uint32 grid_id)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COALESCE(MAX(`number`), 0) FROM `{}` WHERE `zoneid` = {} AND `gridid` = {}",
				TableName(),
				zone_id,
				grid_id
			)
		);

		if (!results.Success() || !results.RowCount()) {
			return 1;
		}

		auto row = results.begin();

		return Strings::ToInt(row[0]) + 1;
	}
};
