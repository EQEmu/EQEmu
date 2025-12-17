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

class BaseRespawnTimesRepository {
public:
	struct RespawnTimes {
		int32_t  id;
		int32_t  start;
		int32_t  duration;
		uint32_t expire_at;
		int16_t  instance_id;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"start",
			"duration",
			"expire_at",
			"instance_id",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"start",
			"duration",
			"expire_at",
			"instance_id",
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
		return std::string("respawn_times");
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

	static RespawnTimes NewEntity()
	{
		RespawnTimes e{};

		e.id          = 0;
		e.start       = 0;
		e.duration    = 0;
		e.expire_at   = 0;
		e.instance_id = 0;

		return e;
	}

	static RespawnTimes GetRespawnTimes(
		const std::vector<RespawnTimes> &respawn_timess,
		int respawn_times_id
	)
	{
		for (auto &respawn_times : respawn_timess) {
			if (respawn_times.id == respawn_times_id) {
				return respawn_times;
			}
		}

		return NewEntity();
	}

	static RespawnTimes FindOne(
		Database& db,
		int respawn_times_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				respawn_times_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			RespawnTimes e{};

			e.id          = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.start       = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.duration    = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.expire_at   = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.instance_id = row[4] ? static_cast<int16_t>(atoi(row[4])) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int respawn_times_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				respawn_times_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const RespawnTimes &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.id));
		v.push_back(columns[1] + " = " + std::to_string(e.start));
		v.push_back(columns[2] + " = " + std::to_string(e.duration));
		v.push_back(columns[3] + " = " + std::to_string(e.expire_at));
		v.push_back(columns[4] + " = " + std::to_string(e.instance_id));

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

	static RespawnTimes InsertOne(
		Database& db,
		RespawnTimes e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.start));
		v.push_back(std::to_string(e.duration));
		v.push_back(std::to_string(e.expire_at));
		v.push_back(std::to_string(e.instance_id));

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
		const std::vector<RespawnTimes> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.start));
			v.push_back(std::to_string(e.duration));
			v.push_back(std::to_string(e.expire_at));
			v.push_back(std::to_string(e.instance_id));

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

	static std::vector<RespawnTimes> All(Database& db)
	{
		std::vector<RespawnTimes> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			RespawnTimes e{};

			e.id          = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.start       = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.duration    = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.expire_at   = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.instance_id = row[4] ? static_cast<int16_t>(atoi(row[4])) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<RespawnTimes> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<RespawnTimes> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			RespawnTimes e{};

			e.id          = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.start       = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.duration    = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.expire_at   = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.instance_id = row[4] ? static_cast<int16_t>(atoi(row[4])) : 0;

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
		const RespawnTimes &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.start));
		v.push_back(std::to_string(e.duration));
		v.push_back(std::to_string(e.expire_at));
		v.push_back(std::to_string(e.instance_id));

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
		const std::vector<RespawnTimes> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.start));
			v.push_back(std::to_string(e.duration));
			v.push_back(std::to_string(e.expire_at));
			v.push_back(std::to_string(e.instance_id));

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
