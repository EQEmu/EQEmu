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

class BaseSpawnEventsRepository {
public:
	struct SpawnEvents {
		uint32_t    id;
		std::string zone;
		uint32_t    cond_id;
		std::string name;
		uint32_t    period;
		uint8_t     next_minute;
		uint8_t     next_hour;
		uint8_t     next_day;
		uint8_t     next_month;
		uint32_t    next_year;
		int8_t      enabled;
		uint8_t     action;
		int32_t     argument;
		int8_t      strict;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"zone",
			"cond_id",
			"name",
			"period",
			"next_minute",
			"next_hour",
			"next_day",
			"next_month",
			"next_year",
			"enabled",
			"action",
			"argument",
			"strict",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"zone",
			"cond_id",
			"name",
			"period",
			"next_minute",
			"next_hour",
			"next_day",
			"next_month",
			"next_year",
			"enabled",
			"action",
			"argument",
			"strict",
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
		return std::string("spawn_events");
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

	static SpawnEvents NewEntity()
	{
		SpawnEvents e{};

		e.id          = 0;
		e.zone        = "";
		e.cond_id     = 0;
		e.name        = "";
		e.period      = 0;
		e.next_minute = 0;
		e.next_hour   = 0;
		e.next_day    = 0;
		e.next_month  = 0;
		e.next_year   = 0;
		e.enabled     = 1;
		e.action      = 0;
		e.argument    = 0;
		e.strict      = 0;

		return e;
	}

	static SpawnEvents GetSpawnEvents(
		const std::vector<SpawnEvents> &spawn_eventss,
		int spawn_events_id
	)
	{
		for (auto &spawn_events : spawn_eventss) {
			if (spawn_events.id == spawn_events_id) {
				return spawn_events;
			}
		}

		return NewEntity();
	}

	static SpawnEvents FindOne(
		Database& db,
		int spawn_events_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				spawn_events_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			SpawnEvents e{};

			e.id          = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.zone        = row[1] ? row[1] : "";
			e.cond_id     = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.name        = row[3] ? row[3] : "";
			e.period      = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.next_minute = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.next_hour   = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.next_day    = row[7] ? static_cast<uint8_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.next_month  = row[8] ? static_cast<uint8_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.next_year   = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.enabled     = row[10] ? static_cast<int8_t>(atoi(row[10])) : 1;
			e.action      = row[11] ? static_cast<uint8_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.argument    = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.strict      = row[13] ? static_cast<int8_t>(atoi(row[13])) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int spawn_events_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				spawn_events_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const SpawnEvents &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.zone) + "'");
		v.push_back(columns[2] + " = " + std::to_string(e.cond_id));
		v.push_back(columns[3] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[4] + " = " + std::to_string(e.period));
		v.push_back(columns[5] + " = " + std::to_string(e.next_minute));
		v.push_back(columns[6] + " = " + std::to_string(e.next_hour));
		v.push_back(columns[7] + " = " + std::to_string(e.next_day));
		v.push_back(columns[8] + " = " + std::to_string(e.next_month));
		v.push_back(columns[9] + " = " + std::to_string(e.next_year));
		v.push_back(columns[10] + " = " + std::to_string(e.enabled));
		v.push_back(columns[11] + " = " + std::to_string(e.action));
		v.push_back(columns[12] + " = " + std::to_string(e.argument));
		v.push_back(columns[13] + " = " + std::to_string(e.strict));

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

	static SpawnEvents InsertOne(
		Database& db,
		SpawnEvents e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.zone) + "'");
		v.push_back(std::to_string(e.cond_id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.period));
		v.push_back(std::to_string(e.next_minute));
		v.push_back(std::to_string(e.next_hour));
		v.push_back(std::to_string(e.next_day));
		v.push_back(std::to_string(e.next_month));
		v.push_back(std::to_string(e.next_year));
		v.push_back(std::to_string(e.enabled));
		v.push_back(std::to_string(e.action));
		v.push_back(std::to_string(e.argument));
		v.push_back(std::to_string(e.strict));

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
		const std::vector<SpawnEvents> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.zone) + "'");
			v.push_back(std::to_string(e.cond_id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.period));
			v.push_back(std::to_string(e.next_minute));
			v.push_back(std::to_string(e.next_hour));
			v.push_back(std::to_string(e.next_day));
			v.push_back(std::to_string(e.next_month));
			v.push_back(std::to_string(e.next_year));
			v.push_back(std::to_string(e.enabled));
			v.push_back(std::to_string(e.action));
			v.push_back(std::to_string(e.argument));
			v.push_back(std::to_string(e.strict));

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

	static std::vector<SpawnEvents> All(Database& db)
	{
		std::vector<SpawnEvents> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SpawnEvents e{};

			e.id          = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.zone        = row[1] ? row[1] : "";
			e.cond_id     = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.name        = row[3] ? row[3] : "";
			e.period      = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.next_minute = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.next_hour   = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.next_day    = row[7] ? static_cast<uint8_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.next_month  = row[8] ? static_cast<uint8_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.next_year   = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.enabled     = row[10] ? static_cast<int8_t>(atoi(row[10])) : 1;
			e.action      = row[11] ? static_cast<uint8_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.argument    = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.strict      = row[13] ? static_cast<int8_t>(atoi(row[13])) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<SpawnEvents> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<SpawnEvents> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SpawnEvents e{};

			e.id          = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.zone        = row[1] ? row[1] : "";
			e.cond_id     = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.name        = row[3] ? row[3] : "";
			e.period      = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.next_minute = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.next_hour   = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.next_day    = row[7] ? static_cast<uint8_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.next_month  = row[8] ? static_cast<uint8_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.next_year   = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.enabled     = row[10] ? static_cast<int8_t>(atoi(row[10])) : 1;
			e.action      = row[11] ? static_cast<uint8_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.argument    = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.strict      = row[13] ? static_cast<int8_t>(atoi(row[13])) : 0;

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
		const SpawnEvents &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.zone) + "'");
		v.push_back(std::to_string(e.cond_id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.period));
		v.push_back(std::to_string(e.next_minute));
		v.push_back(std::to_string(e.next_hour));
		v.push_back(std::to_string(e.next_day));
		v.push_back(std::to_string(e.next_month));
		v.push_back(std::to_string(e.next_year));
		v.push_back(std::to_string(e.enabled));
		v.push_back(std::to_string(e.action));
		v.push_back(std::to_string(e.argument));
		v.push_back(std::to_string(e.strict));

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
		const std::vector<SpawnEvents> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.zone) + "'");
			v.push_back(std::to_string(e.cond_id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.period));
			v.push_back(std::to_string(e.next_minute));
			v.push_back(std::to_string(e.next_hour));
			v.push_back(std::to_string(e.next_day));
			v.push_back(std::to_string(e.next_month));
			v.push_back(std::to_string(e.next_year));
			v.push_back(std::to_string(e.enabled));
			v.push_back(std::to_string(e.action));
			v.push_back(std::to_string(e.argument));
			v.push_back(std::to_string(e.strict));

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
