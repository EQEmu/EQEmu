#pragma once

#include "common/repositories/base/base_char_recipe_list_repository.h"

#include "common/database.h"
#include "common/strings.h"

class CharRecipeListRepository: public BaseCharRecipeListRepository {
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
     * CharRecipeListRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * CharRecipeListRepository::GetWhereNeverExpires()
     * CharRecipeListRepository::GetWhereXAndY()
     * CharRecipeListRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here

	static CharRecipeList GetCharRecipeListEntry(
		const std::vector<CharRecipeList> &recipe_list,
		uint32 recipe_id
	)
	{
		for (auto &e : recipe_list) {
			if (e.recipe_id == recipe_id) {
				return e;
			}
		}

		return NewEntity();
	}

	// insert with ON DUPLICATE KEY UPDATE to leave rows that exist unchanged
	static int InsertUpdateMany(Database& db, const std::vector<CharRecipeList>& entries)
	{
		std::vector<std::string> values;
		values.reserve(entries.size());

		for (const auto& e: entries)
		{
			values.emplace_back(fmt::format("({},{},{})", e.char_id, e.recipe_id, e.madecount));
		}

		auto results = db.QueryDatabase(fmt::format(
			"INSERT INTO {0} (char_id, recipe_id, madecount) VALUES {1} ON DUPLICATE KEY UPDATE {2}={2}",
			TableName(), fmt::join(values, ","), PrimaryKey()));

		return results.Success() ? results.RowsAffected() : 0;
	}

};
