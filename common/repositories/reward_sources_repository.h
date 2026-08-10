#ifndef EQEMU_REWARD_SOURCES_REPOSITORY_H
#define EQEMU_REWARD_SOURCES_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_reward_sources_repository.h"

class RewardSourcesRepository: public BaseRewardSourcesRepository {
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
     * RewardSourcesRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * RewardSourcesRepository::GetWhereNeverExpires()
     * RewardSourcesRepository::GetWhereXAndY()
     * RewardSourcesRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	static bool GetAll(Database &db, std::vector<RewardSources> &entries)
	{
		entries.clear();
		auto results = db.QueryDatabase(BaseSelect());
		if (!results.Success()) {
			return false;
		}

		entries.reserve(results.RowCount());
		for (auto row = results.begin(); row != results.end(); ++row) {
			entries.push_back({
				.source_type = row[0]
					? static_cast<uint8_t>(strtoul(row[0], nullptr, 10))
					: 0,
				.source_id = row[1] ? strtoull(row[1], nullptr, 10) : 0,
				.reward_set_id = row[2]
					? static_cast<uint32_t>(strtoul(row[2], nullptr, 10))
					: 0,
				.enabled = row[3]
					? static_cast<uint8_t>(strtoul(row[3], nullptr, 10))
					: 1
			});
		}
		return true;
	}

};

#endif //EQEMU_REWARD_SOURCES_REPOSITORY_H
