#pragma once

#include "common/repositories/base/base_adventure_stats_repository.h"

#include "common/database.h"
#include "common/strings.h"

class AdventureStatsRepository: public BaseAdventureStatsRepository {
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
     * AdventureStatsRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * AdventureStatsRepository::GetWhereNeverExpires()
     * AdventureStatsRepository::GetWhereXAndY()
     * AdventureStatsRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here
	static void UpdateAdventureStatsEntry(Database& db, uint32 character_id, uint8 theme_id, bool is_win, bool is_remove)
	{
		std::string field;

		switch (theme_id) {
			case LDoNTheme::GUK: {
				field = "guk_";
				break;
			}
			case LDoNTheme::MIR: {
				field = "mir_";
				break;
			}
			case LDoNTheme::MMC: {
				field = "mmc_";
				break;
			}
			case LDoNTheme::RUJ: {
				field = "ruj_";
				break;
			}
			case LDoNTheme::TAK: {
				field = "tak_";
				break;
			}
		}

		field += is_win ? "wins" : "losses";

		auto e = FindOne(db, character_id);

		if (!e.player_id && !is_remove) {
			const std::string& query = fmt::format(
				"INSERT INTO `{}` SET `{}` = 1, `{}` = {}",
				TableName(),
				field,
				PrimaryKey(),
				character_id
			);

			db.QueryDatabase(query);

			return;
		}

		const std::string& field_operation = is_remove ? "-" : "+";

		const std::string& query = fmt::format(
			"UPDATE `{}` SET `{}` = {} {} 1 WHERE `{}` = {}",
			TableName(),
			field,
			field,
			field_operation,
			PrimaryKey(),
			character_id
		);

		db.QueryDatabase(query);
	}
};
