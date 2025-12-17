#pragma once

#include "common/repositories/base/base_npc_types_repository.h"

#include "common/database.h"
#include "common/strings.h"

class NpcTypesRepository: public BaseNpcTypesRepository {
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
     * NpcTypesRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * NpcTypesRepository::GetWhereNeverExpires()
     * NpcTypesRepository::GetWhereXAndY()
     * NpcTypesRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here
	static uint32 GetOpenIDInZoneRange(Database& db, uint32 zone_id)
	{
		const uint32 min_id = zone_id * 1000;
		const uint32 max_id = min_id + 999;

		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT MAX({}) FROM `{}` WHERE `{}` BETWEEN {} AND {}",
				PrimaryKey(),
				TableName(),
				PrimaryKey(),
				min_id,
				max_id
			)
		);

		if (!results.Success() || !results.RowCount()) {
			return 0;
		}

		auto row = results.begin();

		const uint32 npc_id = row[0] ? Strings::ToUnsignedInt(row[0]) + 1 : 0;

		return npc_id < max_id ? npc_id : 0;
	}
};
