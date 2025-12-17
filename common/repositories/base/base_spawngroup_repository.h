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

class BaseSpawngroupRepository {
public:
	struct Spawngroup {
		int32_t     id;
		std::string name;
		int8_t      spawn_limit;
		float       dist;
		float       max_x;
		float       min_x;
		float       max_y;
		float       min_y;
		int32_t     delay;
		int32_t     mindelay;
		int8_t      despawn;
		int32_t     despawn_timer;
		uint8_t     wp_spawns;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"name",
			"spawn_limit",
			"dist",
			"max_x",
			"min_x",
			"max_y",
			"min_y",
			"delay",
			"mindelay",
			"despawn",
			"despawn_timer",
			"wp_spawns",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"name",
			"spawn_limit",
			"dist",
			"max_x",
			"min_x",
			"max_y",
			"min_y",
			"delay",
			"mindelay",
			"despawn",
			"despawn_timer",
			"wp_spawns",
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
		return std::string("spawngroup");
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

	static Spawngroup NewEntity()
	{
		Spawngroup e{};

		e.id            = 0;
		e.name          = "";
		e.spawn_limit   = 0;
		e.dist          = 0;
		e.max_x         = 0;
		e.min_x         = 0;
		e.max_y         = 0;
		e.min_y         = 0;
		e.delay         = 45000;
		e.mindelay      = 15000;
		e.despawn       = 0;
		e.despawn_timer = 100;
		e.wp_spawns     = 0;

		return e;
	}

	static Spawngroup GetSpawngroup(
		const std::vector<Spawngroup> &spawngroups,
		int spawngroup_id
	)
	{
		for (auto &spawngroup : spawngroups) {
			if (spawngroup.id == spawngroup_id) {
				return spawngroup;
			}
		}

		return NewEntity();
	}

	static Spawngroup FindOne(
		Database& db,
		int spawngroup_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				spawngroup_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Spawngroup e{};

			e.id            = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.name          = row[1] ? row[1] : "";
			e.spawn_limit   = row[2] ? static_cast<int8_t>(atoi(row[2])) : 0;
			e.dist          = row[3] ? strtof(row[3], nullptr) : 0;
			e.max_x         = row[4] ? strtof(row[4], nullptr) : 0;
			e.min_x         = row[5] ? strtof(row[5], nullptr) : 0;
			e.max_y         = row[6] ? strtof(row[6], nullptr) : 0;
			e.min_y         = row[7] ? strtof(row[7], nullptr) : 0;
			e.delay         = row[8] ? static_cast<int32_t>(atoi(row[8])) : 45000;
			e.mindelay      = row[9] ? static_cast<int32_t>(atoi(row[9])) : 15000;
			e.despawn       = row[10] ? static_cast<int8_t>(atoi(row[10])) : 0;
			e.despawn_timer = row[11] ? static_cast<int32_t>(atoi(row[11])) : 100;
			e.wp_spawns     = row[12] ? static_cast<uint8_t>(strtoul(row[12], nullptr, 10)) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int spawngroup_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				spawngroup_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Spawngroup &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[2] + " = " + std::to_string(e.spawn_limit));
		v.push_back(columns[3] + " = " + std::to_string(e.dist));
		v.push_back(columns[4] + " = " + std::to_string(e.max_x));
		v.push_back(columns[5] + " = " + std::to_string(e.min_x));
		v.push_back(columns[6] + " = " + std::to_string(e.max_y));
		v.push_back(columns[7] + " = " + std::to_string(e.min_y));
		v.push_back(columns[8] + " = " + std::to_string(e.delay));
		v.push_back(columns[9] + " = " + std::to_string(e.mindelay));
		v.push_back(columns[10] + " = " + std::to_string(e.despawn));
		v.push_back(columns[11] + " = " + std::to_string(e.despawn_timer));
		v.push_back(columns[12] + " = " + std::to_string(e.wp_spawns));

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

	static Spawngroup InsertOne(
		Database& db,
		Spawngroup e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.spawn_limit));
		v.push_back(std::to_string(e.dist));
		v.push_back(std::to_string(e.max_x));
		v.push_back(std::to_string(e.min_x));
		v.push_back(std::to_string(e.max_y));
		v.push_back(std::to_string(e.min_y));
		v.push_back(std::to_string(e.delay));
		v.push_back(std::to_string(e.mindelay));
		v.push_back(std::to_string(e.despawn));
		v.push_back(std::to_string(e.despawn_timer));
		v.push_back(std::to_string(e.wp_spawns));

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
		const std::vector<Spawngroup> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.spawn_limit));
			v.push_back(std::to_string(e.dist));
			v.push_back(std::to_string(e.max_x));
			v.push_back(std::to_string(e.min_x));
			v.push_back(std::to_string(e.max_y));
			v.push_back(std::to_string(e.min_y));
			v.push_back(std::to_string(e.delay));
			v.push_back(std::to_string(e.mindelay));
			v.push_back(std::to_string(e.despawn));
			v.push_back(std::to_string(e.despawn_timer));
			v.push_back(std::to_string(e.wp_spawns));

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

	static std::vector<Spawngroup> All(Database& db)
	{
		std::vector<Spawngroup> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Spawngroup e{};

			e.id            = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.name          = row[1] ? row[1] : "";
			e.spawn_limit   = row[2] ? static_cast<int8_t>(atoi(row[2])) : 0;
			e.dist          = row[3] ? strtof(row[3], nullptr) : 0;
			e.max_x         = row[4] ? strtof(row[4], nullptr) : 0;
			e.min_x         = row[5] ? strtof(row[5], nullptr) : 0;
			e.max_y         = row[6] ? strtof(row[6], nullptr) : 0;
			e.min_y         = row[7] ? strtof(row[7], nullptr) : 0;
			e.delay         = row[8] ? static_cast<int32_t>(atoi(row[8])) : 45000;
			e.mindelay      = row[9] ? static_cast<int32_t>(atoi(row[9])) : 15000;
			e.despawn       = row[10] ? static_cast<int8_t>(atoi(row[10])) : 0;
			e.despawn_timer = row[11] ? static_cast<int32_t>(atoi(row[11])) : 100;
			e.wp_spawns     = row[12] ? static_cast<uint8_t>(strtoul(row[12], nullptr, 10)) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Spawngroup> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Spawngroup> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Spawngroup e{};

			e.id            = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.name          = row[1] ? row[1] : "";
			e.spawn_limit   = row[2] ? static_cast<int8_t>(atoi(row[2])) : 0;
			e.dist          = row[3] ? strtof(row[3], nullptr) : 0;
			e.max_x         = row[4] ? strtof(row[4], nullptr) : 0;
			e.min_x         = row[5] ? strtof(row[5], nullptr) : 0;
			e.max_y         = row[6] ? strtof(row[6], nullptr) : 0;
			e.min_y         = row[7] ? strtof(row[7], nullptr) : 0;
			e.delay         = row[8] ? static_cast<int32_t>(atoi(row[8])) : 45000;
			e.mindelay      = row[9] ? static_cast<int32_t>(atoi(row[9])) : 15000;
			e.despawn       = row[10] ? static_cast<int8_t>(atoi(row[10])) : 0;
			e.despawn_timer = row[11] ? static_cast<int32_t>(atoi(row[11])) : 100;
			e.wp_spawns     = row[12] ? static_cast<uint8_t>(strtoul(row[12], nullptr, 10)) : 0;

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
		const Spawngroup &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.spawn_limit));
		v.push_back(std::to_string(e.dist));
		v.push_back(std::to_string(e.max_x));
		v.push_back(std::to_string(e.min_x));
		v.push_back(std::to_string(e.max_y));
		v.push_back(std::to_string(e.min_y));
		v.push_back(std::to_string(e.delay));
		v.push_back(std::to_string(e.mindelay));
		v.push_back(std::to_string(e.despawn));
		v.push_back(std::to_string(e.despawn_timer));
		v.push_back(std::to_string(e.wp_spawns));

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
		const std::vector<Spawngroup> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.spawn_limit));
			v.push_back(std::to_string(e.dist));
			v.push_back(std::to_string(e.max_x));
			v.push_back(std::to_string(e.min_x));
			v.push_back(std::to_string(e.max_y));
			v.push_back(std::to_string(e.min_y));
			v.push_back(std::to_string(e.delay));
			v.push_back(std::to_string(e.mindelay));
			v.push_back(std::to_string(e.despawn));
			v.push_back(std::to_string(e.despawn_timer));
			v.push_back(std::to_string(e.wp_spawns));

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
