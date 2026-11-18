#ifndef EQEMU_REWARD_OPTIONS_REPOSITORY_H
#define EQEMU_REWARD_OPTIONS_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_reward_options_repository.h"

class RewardOptionsRepository: public BaseRewardOptionsRepository {
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
     * RewardOptionsRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * RewardOptionsRepository::GetWhereNeverExpires()
     * RewardOptionsRepository::GetWhereXAndY()
     * RewardOptionsRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	static bool GetAll(Database &db, std::vector<RewardOptions> &entries)
	{
		entries.clear();
		auto results = db.QueryDatabase(BaseSelect());
		if (!results.Success()) {
			return false;
		}

		entries.reserve(results.RowCount());
		for (auto row = results.begin(); row != results.end(); ++row) {
			entries.push_back({
				.reward_set_id = row[0]
					? static_cast<uint32_t>(strtoul(row[0], nullptr, 10))
					: 0,
				.option_id = row[1]
					? static_cast<uint32_t>(strtoul(row[1], nullptr, 10))
					: 0,
				.sequence = row[2]
					? static_cast<uint32_t>(strtoul(row[2], nullptr, 10))
					: 0,
				.label = row[3] ? row[3] : "",
				.common_to_all = static_cast<uint8_t>(
					row[4] ? strtoul(row[4], nullptr, 10) : 0
				),
				.flags = static_cast<uint8_t>(
					row[5] ? strtoul(row[5], nullptr, 10) : 0
				),
				.enabled = static_cast<uint8_t>(
					row[6] ? strtoul(row[6], nullptr, 10) : 1
				)
			});
		}
		return true;
	}

};

#endif //EQEMU_REWARD_OPTIONS_REPOSITORY_H
