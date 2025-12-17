/**
 * DO NOT MODIFY THIS FILE
 *
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to the repository extending the base.
 * Any modifications to base repositories are to be made by the generator only
 *
 * @generator ./utils/scripts/generators/repository-generator.pl
 * @docs https://docs.eqemu.dev/developer/repositories
 */

#pragma once

#include "common/database.h"
#include "common/strings.h"

#include <ctime>

class BaseRuleValuesRepository {
public:
	struct RuleValues {
		uint8_t     ruleset_id;
		std::string rule_name;
		std::string rule_value;
		std::string notes;
	};

	static std::string PrimaryKey()
	{
		return std::string("ruleset_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"ruleset_id",
			"rule_name",
			"rule_value",
			"notes",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"ruleset_id",
			"rule_name",
			"rule_value",
			"notes",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(Strings::Implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(Strings::Implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("rule_values");
	}

	static std::string BaseSelect()
	{
		return fmt::format(
			"SELECT {} FROM {}",
			SelectColumnsRaw(),
			TableName()
		);
	}

	static std::string BaseInsert()
	{
		return fmt::format(
			"INSERT INTO {} ({}) ",
			TableName(),
			ColumnsRaw()
		);
	}

	static RuleValues NewEntity()
	{
		RuleValues e{};

		e.ruleset_id = 0;
		e.rule_name  = "";
		e.rule_value = "";
		e.notes      = "";

		return e;
	}

	static RuleValues GetRuleValues(
		const std::vector<RuleValues> &rule_valuess,
		int rule_values_id
	)
	{
		for (auto &rule_values : rule_valuess) {
			if (rule_values.ruleset_id == rule_values_id) {
				return rule_values;
			}
		}

		return NewEntity();
	}

	static RuleValues FindOne(
		Database& db,
		int rule_values_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				rule_values_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			RuleValues e{};

			e.ruleset_id = row[0] ? static_cast<uint8_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.rule_name  = row[1] ? row[1] : "";
			e.rule_value = row[2] ? row[2] : "";
			e.notes      = row[3] ? row[3] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int rule_values_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				rule_values_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const RuleValues &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.ruleset_id));
		v.push_back(columns[1] + " = '" + Strings::Escape(e.rule_name) + "'");
		v.push_back(columns[2] + " = '" + Strings::Escape(e.rule_value) + "'");
		v.push_back(columns[3] + " = '" + Strings::Escape(e.notes) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.ruleset_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static RuleValues InsertOne(
		Database& db,
		RuleValues e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.ruleset_id));
		v.push_back("'" + Strings::Escape(e.rule_name) + "'");
		v.push_back("'" + Strings::Escape(e.rule_value) + "'");
		v.push_back("'" + Strings::Escape(e.notes) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.ruleset_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<RuleValues> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.ruleset_id));
			v.push_back("'" + Strings::Escape(e.rule_name) + "'");
			v.push_back("'" + Strings::Escape(e.rule_value) + "'");
			v.push_back("'" + Strings::Escape(e.notes) + "'");

			insert_chunks.push_back("(" + Strings::Implode(",", v) + ")");
		}

		std::vector<std::string> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<RuleValues> All(Database& db)
	{
		std::vector<RuleValues> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			RuleValues e{};

			e.ruleset_id = row[0] ? static_cast<uint8_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.rule_name  = row[1] ? row[1] : "";
			e.rule_value = row[2] ? row[2] : "";
			e.notes      = row[3] ? row[3] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<RuleValues> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<RuleValues> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			RuleValues e{};

			e.ruleset_id = row[0] ? static_cast<uint8_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.rule_name  = row[1] ? row[1] : "";
			e.rule_value = row[2] ? row[2] : "";
			e.notes      = row[3] ? row[3] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static int DeleteWhere(Database& db, const std::string &where_filter)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {}",
				TableName(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int Truncate(Database& db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"TRUNCATE TABLE {}",
				TableName()
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int64 GetMaxId(Database& db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COALESCE(MAX({}), 0) FROM {}",
				PrimaryKey(),
				TableName()
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static int64 Count(Database& db, const std::string &where_filter = "")
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COUNT(*) FROM {} {}",
				TableName(),
				(where_filter.empty() ? "" : "WHERE " + where_filter)
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static std::string BaseReplace()
	{
		return fmt::format(
			"REPLACE INTO {} ({}) ",
			TableName(),
			ColumnsRaw()
		);
	}

	static int ReplaceOne(
		Database& db,
		const RuleValues &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.ruleset_id));
		v.push_back("'" + Strings::Escape(e.rule_name) + "'");
		v.push_back("'" + Strings::Escape(e.rule_value) + "'");
		v.push_back("'" + Strings::Escape(e.notes) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseReplace(),
				Strings::Implode(",", v)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int ReplaceMany(
		Database& db,
		const std::vector<RuleValues> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.ruleset_id));
			v.push_back("'" + Strings::Escape(e.rule_name) + "'");
			v.push_back("'" + Strings::Escape(e.rule_value) + "'");
			v.push_back("'" + Strings::Escape(e.notes) + "'");

			insert_chunks.push_back("(" + Strings::Implode(",", v) + ")");
		}

		std::vector<std::string> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseReplace(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}
};
