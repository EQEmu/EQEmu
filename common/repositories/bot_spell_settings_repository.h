#pragma once

#include "common/repositories/base/base_bot_spell_settings_repository.h"

#include "common/database.h"
#include "common/strings.h"

class BotSpellSettingsRepository: public BaseBotSpellSettingsRepository {
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
     * BotSpellSettingsRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * BotSpellSettingsRepository::GetWhereNeverExpires()
     * BotSpellSettingsRepository::GetWhereXAndY()
     * BotSpellSettingsRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here

	static bool UpdateSpellSetting(
		Database& db,
		const BotSpellSettings &e
	) {
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[3] + " = " + std::to_string(e.priority));
		v.push_back(columns[4] + " = " + std::to_string(e.min_hp));
		v.push_back(columns[5] + " = " + std::to_string(e.max_hp));
		v.push_back(columns[6] + " = " + std::to_string(e.is_enabled));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE `bot_id` = {} AND `spell_id` = {}",
				TableName(),
				Strings::Implode(", ", v),
				e.bot_id,
				e.spell_id
			)
		);

		return (results.Success() ? true : false);
	}
};
