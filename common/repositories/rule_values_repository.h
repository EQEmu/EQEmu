#pragma once

#include "common/repositories/base/base_rule_values_repository.h"

#include "common/database.h"
#include "common/strings.h"

class RuleValuesRepository: public BaseRuleValuesRepository {
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
     * RuleValuesRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * RuleValuesRepository::GetWhereNeverExpires()
     * RuleValuesRepository::GetWhereXAndY()
     * RuleValuesRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	template<typename T1, typename T2, typename T3, typename T4>
	static std::vector<std::string> join_tuple(
		const std::string &glue,
		const std::pair<char, char> &encapsulation,
		const std::vector<std::tuple<T1, T2, T3, T4>> &src
	)
	{
		if (src.empty()) {
			return {};
		}

		std::vector<std::string> output;

		for (const std::tuple<T1, T2, T3, T4> &src_iter: src) {

			output.emplace_back(

				fmt::format(
					"{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}",
					encapsulation.first,
					std::get<0>(src_iter),
					encapsulation.second,
					glue,
					encapsulation.first,
					std::get<1>(src_iter),
					encapsulation.second,
					glue,
					encapsulation.first,
					std::get<2>(src_iter),
					encapsulation.second,
					glue,
					encapsulation.first,
					std::get<3>(src_iter),
					encapsulation.second
				)
			);
		}

		return output;
	}

	// Custom extended repository methods here
	static std::vector<std::string> GetRuleNames(Database &db, int rule_set_id)
	{
		std::vector<std::string> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT rule_name FROM {} WHERE ruleset_id = {}",
				TableName(),
				rule_set_id
			)
		);
		if (!results.Success() || !results.RowCount()) {
			return v;
		}

		for (auto row : results) {
			v.push_back(row[0]);
		}

		return v;
	}

	static std::vector<std::string> GetGroupedRules(Database &db)
	{
		std::vector <std::string> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT rule_name FROM {} GROUP BY rule_name",
				TableName()
			)
		);
		if (!results.Success() || !results.RowCount()) {
			return v;
		}

		for (auto row : results) {
			v.push_back(row[0]);
		}

		return v;
	}

	template<typename T>
	static std::string
	ImplodePair(const std::string &glue, const std::pair<char, char> &encapsulation, const std::vector<T> &src)
	{
		if (src.empty()) {
			return {};
		}
		std::ostringstream oss;
		for (const T &src_iter: src) {
			oss << encapsulation.first << src_iter << encapsulation.second << glue;
		}
		std::string output(oss.str());
		output.resize(output.size() - glue.size());
		return output;
	}

	static bool DeleteOrphanedRules(Database& db, std::vector<std::string>& v)
	{
		const auto query = fmt::format(
			"DELETE FROM {} WHERE rule_name IN ({})",
			TableName(),
			ImplodePair(",", std::pair<char, char>('\'', '\''), v)
		);

		return db.QueryDatabase(query).Success();
	}

	static bool InjectRules(Database& db, std::vector<std::tuple<int, std::string, std::string, std::string>>& v)
	{
		const auto query = fmt::format(
			"REPLACE INTO {} (`ruleset_id`, `rule_name`, `rule_value`, `notes`) VALUES {}",
			TableName(),
			ImplodePair(
				",",
				std::pair<char, char>('(', ')'),
				join_tuple(",", std::pair<char, char>('\'', '\''), v)
			)
		);

		return db.QueryDatabase(query).Success();
	}

	static bool UpdateRuleNote(Database& db, int rule_set_id, std::string rule_name, std::string notes)
	{
		const auto query = fmt::format(
			"UPDATE {} SET notes = '{}' WHERE ruleset_id = {} AND rule_name = '{}'",
			TableName(),
			Strings::Escape(notes),
			rule_set_id,
			rule_name
		);

		return db.QueryDatabase(query).Success();
	}

};
