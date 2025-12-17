#pragma once

#include "common/repositories/base/base_rule_sets_repository.h"

#include "common/database.h"
#include "common/strings.h"

class RuleSetsRepository: public BaseRuleSetsRepository {
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
     * RuleSetsRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * RuleSetsRepository::GetWhereNeverExpires()
     * RuleSetsRepository::GetWhereXAndY()
     * RuleSetsRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here
	static int GetRuleSetID(Database& db, std::string rule_set_name)
	{
		const auto query = fmt::format(
			"SELECT ruleset_id FROM {} WHERE `name` = '{}'",
			TableName(),
			Strings::Escape(rule_set_name)
		);
		auto results = db.QueryDatabase(query);
		if (!results.Success() || !results.RowCount()) {
			return -1;
		}

		auto row = results.begin();
		return std::stoi(row[0]);
	}

	static int CreateNewRuleSet(Database& db, std::string rule_set_name)
	{
		const auto query = fmt::format(
			"INSERT INTO {} (`name`) VALUES ('{}')",
			TableName(),
			rule_set_name
		);
		auto results = db.QueryDatabase(query);
		if (!results.Success() || !results.RowsAffected()) {
			return -1;
		}

		return static_cast<int>(results.LastInsertedID());
	}

	static std::string GetRuleSetName(Database& db, int rule_set_id)
	{
		const auto query = fmt::format(
			"SELECT `name` FROM {} WHERE ruleset_id = {}",
			TableName(),
			rule_set_id
		);
		auto results = db.QueryDatabase(query);
		if (!results.Success() || !results.RowsAffected()) {
			return std::string();
		}

		auto row = results.begin();

		return std::string(row[0]);
	}
};
