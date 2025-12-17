#pragma once

#include "common/repositories/base/base_inventory_snapshots_repository.h"

#include "common/database.h"
#include "common/strings.h"

class InventorySnapshotsRepository: public BaseInventorySnapshotsRepository {
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
     * InventorySnapshotsRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * InventorySnapshotsRepository::GetWhereNeverExpires()
     * InventorySnapshotsRepository::GetWhereXAndY()
     * InventorySnapshotsRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here
	static int64 CountInventorySnapshots(Database& db)
	{
		const std::string& query = "SELECT COUNT(*) FROM (SELECT * FROM `inventory_snapshots` a GROUP BY `charid`, `time_index`) b";

		auto results = db.QueryDatabase(query);

		if (!results.Success() || !results.RowCount()) {
			return -1;
		}

		auto row = results.begin();

		const int64 count = Strings::ToBigInt(row[0]);

		if (count > std::numeric_limits<int>::max()) {
			return -2;
		}

		if (count < 0) {
			return -3;
		}

		return count;
	}
};
