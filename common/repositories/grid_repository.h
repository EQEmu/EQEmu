#pragma once

#include "common/repositories/base/base_grid_repository.h"

#include "common/database.h"
#include "common/strings.h"

class GridRepository: public BaseGridRepository {
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
     * GridRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * GridRepository::GetWhereNeverExpires()
     * GridRepository::GetWhereXAndY()
     * GridRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here

	static std::vector<Grid> GetZoneGrids(Database& db, int zone_id)
	{
		std::vector<Grid> grids;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE zoneid = {}",
				BaseSelect(),
				zone_id
			)
		);

		for (auto row = results.begin(); row != results.end(); ++row) {
			Grid e{};

			e.id     = atoi(row[0]);
			e.zoneid = atoi(row[1]);
			e.type   = atoi(row[2]);
			e.type2  = atoi(row[3]);

			grids.push_back(e);
		}

		return grids;
	}

	static int GetHighestGrid(Database& db, uint32 zone_id)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				SQL(
					SELECT COALESCE(MAX(`id`), 0) FROM `{}`
					WHERE `zoneid` = {}
				),
				TableName(),
				zone_id
			)
		);

		if (!results.Success() || !results.RowCount()) {
			return 0;
		}

		auto row = results.begin();

		return Strings::ToInt(row[0]);
	}
};
