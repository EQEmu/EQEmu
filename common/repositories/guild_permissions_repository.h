#pragma once

#include "common/repositories/base/base_guild_permissions_repository.h"

#include "common/database.h"
#include "common/strings.h"

class GuildPermissionsRepository: public BaseGuildPermissionsRepository {
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
     * GuildPermissionsRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * GuildPermissionsRepository::GetWhereNeverExpires()
     * GuildPermissionsRepository::GetWhereXAndY()
     * GuildPermissionsRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here

	static std::map<std::string, GuildPermissions> LoadAll(Database &db)
	{
		std::map<std::string, GuildPermissions> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE `guild_id` < {}",
				BaseSelect(),
				RoF2::constants::MAX_GUILD_ID
			));

		for (auto row = results.begin(); row != results.end(); ++row) {
			GuildPermissions e{};

			e.id         = static_cast<int32_t>(atoi(row[0]));
			e.perm_id    = static_cast<int32_t>(atoi(row[1]));
			e.guild_id   = static_cast<int32_t>(atoi(row[2]));
			e.permission = static_cast<int32_t>(atoi(row[3]));

			auto key = fmt::format("{}-{}", e.guild_id, e.perm_id);
			all_entries.emplace(key, e);
		}

		return all_entries;
	}
};
