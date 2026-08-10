#ifndef EQEMU_ACHIEVEMENT_CATEGORY_ASSOCIATIONS_REPOSITORY_H
#define EQEMU_ACHIEVEMENT_CATEGORY_ASSOCIATIONS_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_achievement_category_associations_repository.h"

class AchievementCategoryAssociationsRepository: public BaseAchievementCategoryAssociationsRepository {
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
     * AchievementCategoryAssociationsRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * AchievementCategoryAssociationsRepository::GetWhereNeverExpires()
     * AchievementCategoryAssociationsRepository::GetWhereXAndY()
     * AchievementCategoryAssociationsRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	static bool GetAll(
		Database &db,
		std::vector<AchievementCategoryAssociations> &entries
	)
	{
		entries.clear();
		auto results = db.QueryDatabase(BaseSelect());
		if (!results.Success()) {
			return false;
		}

		entries.reserve(results.RowCount());
		for (auto row = results.begin(); row != results.end(); ++row) {
			entries.push_back({
				.category_id = row[0]
					? static_cast<uint32_t>(strtoul(row[0], nullptr, 10))
					: 0,
				.sequence = row[1]
					? static_cast<uint32_t>(strtoul(row[1], nullptr, 10))
					: 0,
				.achievement_id = row[2]
					? static_cast<uint32_t>(strtoul(row[2], nullptr, 10))
					: 0,
				.display_text = row[3] ? row[3] : ""
			});
		}
		return true;
	}

};

#endif //EQEMU_ACHIEVEMENT_CATEGORY_ASSOCIATIONS_REPOSITORY_H
