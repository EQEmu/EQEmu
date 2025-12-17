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

class BaseSpawnConditionsRepository {
public:
	struct SpawnConditions {
		std::string zone;
		uint32_t    id;
		int32_t     value;
		uint8_t     onchange;
		std::string name;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"zone",
			"id",
			"value",
			"onchange",
			"name",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"zone",
			"id",
			"value",
			"onchange",
			"name",
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
		return std::string("spawn_conditions");
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

	static SpawnConditions NewEntity()
	{
		SpawnConditions e{};

		e.zone     = "";
		e.id       = 1;
		e.value    = 0;
		e.onchange = 0;
		e.name     = "";

		return e;
	}

	static SpawnConditions GetSpawnConditions(
		const std::vector<SpawnConditions> &spawn_conditionss,
		int spawn_conditions_id
	)
	{
		for (auto &spawn_conditions : spawn_conditionss) {
			if (spawn_conditions.id == spawn_conditions_id) {
				return spawn_conditions;
			}
		}

		return NewEntity();
	}

	static SpawnConditions FindOne(
		Database& db,
		int spawn_conditions_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				spawn_conditions_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			SpawnConditions e{};

			e.zone     = row[0] ? row[0] : "";
			e.id       = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 1;
			e.value    = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.onchange = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.name     = row[4] ? row[4] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int spawn_conditions_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				spawn_conditions_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const SpawnConditions &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = '" + Strings::Escape(e.zone) + "'");
		v.push_back(columns[1] + " = " + std::to_string(e.id));
		v.push_back(columns[2] + " = " + std::to_string(e.value));
		v.push_back(columns[3] + " = " + std::to_string(e.onchange));
		v.push_back(columns[4] + " = '" + Strings::Escape(e.name) + "'");

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

	static SpawnConditions InsertOne(
		Database& db,
		SpawnConditions e
	)
	{
		std::vector<std::string> v;

		v.push_back("'" + Strings::Escape(e.zone) + "'");
		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.value));
		v.push_back(std::to_string(e.onchange));
		v.push_back("'" + Strings::Escape(e.name) + "'");

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
		const std::vector<SpawnConditions> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back("'" + Strings::Escape(e.zone) + "'");
			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.value));
			v.push_back(std::to_string(e.onchange));
			v.push_back("'" + Strings::Escape(e.name) + "'");

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

	static std::vector<SpawnConditions> All(Database& db)
	{
		std::vector<SpawnConditions> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SpawnConditions e{};

			e.zone     = row[0] ? row[0] : "";
			e.id       = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 1;
			e.value    = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.onchange = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.name     = row[4] ? row[4] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<SpawnConditions> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<SpawnConditions> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SpawnConditions e{};

			e.zone     = row[0] ? row[0] : "";
			e.id       = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 1;
			e.value    = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.onchange = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.name     = row[4] ? row[4] : "";

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
		const SpawnConditions &e
	)
	{
		std::vector<std::string> v;

		v.push_back("'" + Strings::Escape(e.zone) + "'");
		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.value));
		v.push_back(std::to_string(e.onchange));
		v.push_back("'" + Strings::Escape(e.name) + "'");

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
		const std::vector<SpawnConditions> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back("'" + Strings::Escape(e.zone) + "'");
			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.value));
			v.push_back(std::to_string(e.onchange));
			v.push_back("'" + Strings::Escape(e.name) + "'");

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
