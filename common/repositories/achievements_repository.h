#ifndef EQEMU_ACHIEVEMENTS_REPOSITORY_H
#define EQEMU_ACHIEVEMENTS_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_achievements_repository.h"

class AchievementsRepository: public BaseAchievementsRepository {
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
     * AchievementsRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * AchievementsRepository::GetWhereNeverExpires()
     * AchievementsRepository::GetWhereXAndY()
     * AchievementsRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	static bool GetAll(Database &db, std::vector<Achievements> &entries)
	{
		entries.clear();
		auto results = db.QueryDatabase(BaseSelect());
		if (!results.Success()) {
			return false;
		}

		entries.reserve(results.RowCount());
		for (auto row = results.begin(); row != results.end(); ++row) {
			entries.push_back({
				.id = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0,
				.name = row[1] ? row[1] : "",
				.description = row[2] ? row[2] : "",
				.icon_id = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0,
				.points = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0,
				.has_reward = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0,
				.client_flag = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 0,
				.version = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0,
				.reset_on_version_change = row[8]
					? static_cast<uint8_t>(strtoul(row[8], nullptr, 10))
					: 1,
				.enabled = row[9] ? static_cast<uint8_t>(strtoul(row[9], nullptr, 10)) : 1
			});
		}
		return true;
	}

};

#endif //EQEMU_ACHIEVEMENTS_REPOSITORY_H
