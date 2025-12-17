#pragma once

#include "common/repositories/base/base_character_peqzone_flags_repository.h"

#include "common/database.h"
#include "common/strings.h"

class CharacterPeqzoneFlagsRepository: public BaseCharacterPeqzoneFlagsRepository {
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
     * CharacterPeqzoneFlagsRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * CharacterPeqzoneFlagsRepository::GetWhereNeverExpires()
     * CharacterPeqzoneFlagsRepository::GetWhereXAndY()
     * CharacterPeqzoneFlagsRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here
	static int DeleteFlag(Database& db, uint32 character_id, uint32 zone_id)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE id = {} AND zone_id = {}",
				TableName(),
				character_id,
				zone_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}
};
