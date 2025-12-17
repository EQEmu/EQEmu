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

class BaseVariablesRepository {
public:
	struct Variables {
		int32_t     id;
		std::string varname;
		std::string value;
		std::string information;
		time_t      ts;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"varname",
			"value",
			"information",
			"ts",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"varname",
			"value",
			"information",
			"UNIX_TIMESTAMP(ts)",
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
		return std::string("variables");
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

	static Variables NewEntity()
	{
		Variables e{};

		e.id          = 0;
		e.varname     = "";
		e.value       = "";
		e.information = "";
		e.ts          = std::time(nullptr);

		return e;
	}

	static Variables GetVariables(
		const std::vector<Variables> &variabless,
		int variables_id
	)
	{
		for (auto &variables : variabless) {
			if (variables.id == variables_id) {
				return variables;
			}
		}

		return NewEntity();
	}

	static Variables FindOne(
		Database& db,
		int variables_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				variables_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Variables e{};

			e.id          = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.varname     = row[1] ? row[1] : "";
			e.value       = row[2] ? row[2] : "";
			e.information = row[3] ? row[3] : "";
			e.ts          = strtoll(row[4] ? row[4] : "-1", nullptr, 10);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int variables_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				variables_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Variables &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.varname) + "'");
		v.push_back(columns[2] + " = '" + Strings::Escape(e.value) + "'");
		v.push_back(columns[3] + " = '" + Strings::Escape(e.information) + "'");
		v.push_back(columns[4] + " = FROM_UNIXTIME(" + (e.ts > 0 ? std::to_string(e.ts) : "null") + ")");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Variables InsertOne(
		Database& db,
		Variables e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.varname) + "'");
		v.push_back("'" + Strings::Escape(e.value) + "'");
		v.push_back("'" + Strings::Escape(e.information) + "'");
		v.push_back("FROM_UNIXTIME(" + (e.ts > 0 ? std::to_string(e.ts) : "null") + ")");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<Variables> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.varname) + "'");
			v.push_back("'" + Strings::Escape(e.value) + "'");
			v.push_back("'" + Strings::Escape(e.information) + "'");
			v.push_back("FROM_UNIXTIME(" + (e.ts > 0 ? std::to_string(e.ts) : "null") + ")");

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

	static std::vector<Variables> All(Database& db)
	{
		std::vector<Variables> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Variables e{};

			e.id          = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.varname     = row[1] ? row[1] : "";
			e.value       = row[2] ? row[2] : "";
			e.information = row[3] ? row[3] : "";
			e.ts          = strtoll(row[4] ? row[4] : "-1", nullptr, 10);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Variables> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Variables> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Variables e{};

			e.id          = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.varname     = row[1] ? row[1] : "";
			e.value       = row[2] ? row[2] : "";
			e.information = row[3] ? row[3] : "";
			e.ts          = strtoll(row[4] ? row[4] : "-1", nullptr, 10);

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
		const Variables &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.varname) + "'");
		v.push_back("'" + Strings::Escape(e.value) + "'");
		v.push_back("'" + Strings::Escape(e.information) + "'");
		v.push_back("FROM_UNIXTIME(" + (e.ts > 0 ? std::to_string(e.ts) : "null") + ")");

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
		const std::vector<Variables> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.varname) + "'");
			v.push_back("'" + Strings::Escape(e.value) + "'");
			v.push_back("'" + Strings::Escape(e.information) + "'");
			v.push_back("FROM_UNIXTIME(" + (e.ts > 0 ? std::to_string(e.ts) : "null") + ")");

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
