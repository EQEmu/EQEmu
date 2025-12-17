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

class BaseDynamicZoneLockoutsRepository {
public:
	struct DynamicZoneLockouts {
		uint32_t    id;
		uint32_t    dynamic_zone_id;
		std::string event_name;
		time_t      expire_time;
		uint32_t    duration;
		std::string from_expedition_uuid;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"dynamic_zone_id",
			"event_name",
			"expire_time",
			"duration",
			"from_expedition_uuid",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"dynamic_zone_id",
			"event_name",
			"UNIX_TIMESTAMP(expire_time)",
			"duration",
			"from_expedition_uuid",
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
		return std::string("dynamic_zone_lockouts");
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

	static DynamicZoneLockouts NewEntity()
	{
		DynamicZoneLockouts e{};

		e.id                   = 0;
		e.dynamic_zone_id      = 0;
		e.event_name           = "";
		e.expire_time          = std::time(nullptr);
		e.duration             = 0;
		e.from_expedition_uuid = "";

		return e;
	}

	static DynamicZoneLockouts GetDynamicZoneLockouts(
		const std::vector<DynamicZoneLockouts> &dynamic_zone_lockoutss,
		int dynamic_zone_lockouts_id
	)
	{
		for (auto &dynamic_zone_lockouts : dynamic_zone_lockoutss) {
			if (dynamic_zone_lockouts.id == dynamic_zone_lockouts_id) {
				return dynamic_zone_lockouts;
			}
		}

		return NewEntity();
	}

	static DynamicZoneLockouts FindOne(
		Database& db,
		int dynamic_zone_lockouts_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				dynamic_zone_lockouts_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			DynamicZoneLockouts e{};

			e.id                   = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.dynamic_zone_id      = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.event_name           = row[2] ? row[2] : "";
			e.expire_time          = strtoll(row[3] ? row[3] : "-1", nullptr, 10);
			e.duration             = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.from_expedition_uuid = row[5] ? row[5] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int dynamic_zone_lockouts_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				dynamic_zone_lockouts_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const DynamicZoneLockouts &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.dynamic_zone_id));
		v.push_back(columns[2] + " = '" + Strings::Escape(e.event_name) + "'");
		v.push_back(columns[3] + " = FROM_UNIXTIME(" + (e.expire_time > 0 ? std::to_string(e.expire_time) : "null") + ")");
		v.push_back(columns[4] + " = " + std::to_string(e.duration));
		v.push_back(columns[5] + " = '" + Strings::Escape(e.from_expedition_uuid) + "'");

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

	static DynamicZoneLockouts InsertOne(
		Database& db,
		DynamicZoneLockouts e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.dynamic_zone_id));
		v.push_back("'" + Strings::Escape(e.event_name) + "'");
		v.push_back("FROM_UNIXTIME(" + (e.expire_time > 0 ? std::to_string(e.expire_time) : "null") + ")");
		v.push_back(std::to_string(e.duration));
		v.push_back("'" + Strings::Escape(e.from_expedition_uuid) + "'");

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
		const std::vector<DynamicZoneLockouts> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.dynamic_zone_id));
			v.push_back("'" + Strings::Escape(e.event_name) + "'");
			v.push_back("FROM_UNIXTIME(" + (e.expire_time > 0 ? std::to_string(e.expire_time) : "null") + ")");
			v.push_back(std::to_string(e.duration));
			v.push_back("'" + Strings::Escape(e.from_expedition_uuid) + "'");

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

	static std::vector<DynamicZoneLockouts> All(Database& db)
	{
		std::vector<DynamicZoneLockouts> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			DynamicZoneLockouts e{};

			e.id                   = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.dynamic_zone_id      = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.event_name           = row[2] ? row[2] : "";
			e.expire_time          = strtoll(row[3] ? row[3] : "-1", nullptr, 10);
			e.duration             = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.from_expedition_uuid = row[5] ? row[5] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<DynamicZoneLockouts> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<DynamicZoneLockouts> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			DynamicZoneLockouts e{};

			e.id                   = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.dynamic_zone_id      = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.event_name           = row[2] ? row[2] : "";
			e.expire_time          = strtoll(row[3] ? row[3] : "-1", nullptr, 10);
			e.duration             = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.from_expedition_uuid = row[5] ? row[5] : "";

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
		const DynamicZoneLockouts &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.dynamic_zone_id));
		v.push_back("'" + Strings::Escape(e.event_name) + "'");
		v.push_back("FROM_UNIXTIME(" + (e.expire_time > 0 ? std::to_string(e.expire_time) : "null") + ")");
		v.push_back(std::to_string(e.duration));
		v.push_back("'" + Strings::Escape(e.from_expedition_uuid) + "'");

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
		const std::vector<DynamicZoneLockouts> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.dynamic_zone_id));
			v.push_back("'" + Strings::Escape(e.event_name) + "'");
			v.push_back("FROM_UNIXTIME(" + (e.expire_time > 0 ? std::to_string(e.expire_time) : "null") + ")");
			v.push_back(std::to_string(e.duration));
			v.push_back("'" + Strings::Escape(e.from_expedition_uuid) + "'");

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
