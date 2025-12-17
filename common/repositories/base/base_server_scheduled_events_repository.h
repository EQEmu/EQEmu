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

class BaseServerScheduledEventsRepository {
public:
	struct ServerScheduledEvents {
		int32_t     id;
		std::string description;
		std::string event_type;
		std::string event_data;
		int32_t     minute_start;
		int32_t     hour_start;
		int32_t     day_start;
		int32_t     month_start;
		int32_t     year_start;
		int32_t     minute_end;
		int32_t     hour_end;
		int32_t     day_end;
		int32_t     month_end;
		int32_t     year_end;
		std::string cron_expression;
		time_t      created_at;
		time_t      deleted_at;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"description",
			"event_type",
			"event_data",
			"minute_start",
			"hour_start",
			"day_start",
			"month_start",
			"year_start",
			"minute_end",
			"hour_end",
			"day_end",
			"month_end",
			"year_end",
			"cron_expression",
			"created_at",
			"deleted_at",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"description",
			"event_type",
			"event_data",
			"minute_start",
			"hour_start",
			"day_start",
			"month_start",
			"year_start",
			"minute_end",
			"hour_end",
			"day_end",
			"month_end",
			"year_end",
			"cron_expression",
			"UNIX_TIMESTAMP(created_at)",
			"UNIX_TIMESTAMP(deleted_at)",
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
		return std::string("server_scheduled_events");
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

	static ServerScheduledEvents NewEntity()
	{
		ServerScheduledEvents e{};

		e.id              = 0;
		e.description     = "";
		e.event_type      = "";
		e.event_data      = "";
		e.minute_start    = 0;
		e.hour_start      = 0;
		e.day_start       = 0;
		e.month_start     = 0;
		e.year_start      = 0;
		e.minute_end      = 0;
		e.hour_end        = 0;
		e.day_end         = 0;
		e.month_end       = 0;
		e.year_end        = 0;
		e.cron_expression = "";
		e.created_at      = 0;
		e.deleted_at      = 0;

		return e;
	}

	static ServerScheduledEvents GetServerScheduledEvents(
		const std::vector<ServerScheduledEvents> &server_scheduled_eventss,
		int server_scheduled_events_id
	)
	{
		for (auto &server_scheduled_events : server_scheduled_eventss) {
			if (server_scheduled_events.id == server_scheduled_events_id) {
				return server_scheduled_events;
			}
		}

		return NewEntity();
	}

	static ServerScheduledEvents FindOne(
		Database& db,
		int server_scheduled_events_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				server_scheduled_events_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			ServerScheduledEvents e{};

			e.id              = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.description     = row[1] ? row[1] : "";
			e.event_type      = row[2] ? row[2] : "";
			e.event_data      = row[3] ? row[3] : "";
			e.minute_start    = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.hour_start      = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.day_start       = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.month_start     = row[7] ? static_cast<int32_t>(atoi(row[7])) : 0;
			e.year_start      = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.minute_end      = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.hour_end        = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.day_end         = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;
			e.month_end       = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.year_end        = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.cron_expression = row[14] ? row[14] : "";
			e.created_at      = strtoll(row[15] ? row[15] : "-1", nullptr, 10);
			e.deleted_at      = strtoll(row[16] ? row[16] : "-1", nullptr, 10);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int server_scheduled_events_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				server_scheduled_events_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const ServerScheduledEvents &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.description) + "'");
		v.push_back(columns[2] + " = '" + Strings::Escape(e.event_type) + "'");
		v.push_back(columns[3] + " = '" + Strings::Escape(e.event_data) + "'");
		v.push_back(columns[4] + " = " + std::to_string(e.minute_start));
		v.push_back(columns[5] + " = " + std::to_string(e.hour_start));
		v.push_back(columns[6] + " = " + std::to_string(e.day_start));
		v.push_back(columns[7] + " = " + std::to_string(e.month_start));
		v.push_back(columns[8] + " = " + std::to_string(e.year_start));
		v.push_back(columns[9] + " = " + std::to_string(e.minute_end));
		v.push_back(columns[10] + " = " + std::to_string(e.hour_end));
		v.push_back(columns[11] + " = " + std::to_string(e.day_end));
		v.push_back(columns[12] + " = " + std::to_string(e.month_end));
		v.push_back(columns[13] + " = " + std::to_string(e.year_end));
		v.push_back(columns[14] + " = '" + Strings::Escape(e.cron_expression) + "'");
		v.push_back(columns[15] + " = FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");
		v.push_back(columns[16] + " = FROM_UNIXTIME(" + (e.deleted_at > 0 ? std::to_string(e.deleted_at) : "null") + ")");

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

	static ServerScheduledEvents InsertOne(
		Database& db,
		ServerScheduledEvents e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.description) + "'");
		v.push_back("'" + Strings::Escape(e.event_type) + "'");
		v.push_back("'" + Strings::Escape(e.event_data) + "'");
		v.push_back(std::to_string(e.minute_start));
		v.push_back(std::to_string(e.hour_start));
		v.push_back(std::to_string(e.day_start));
		v.push_back(std::to_string(e.month_start));
		v.push_back(std::to_string(e.year_start));
		v.push_back(std::to_string(e.minute_end));
		v.push_back(std::to_string(e.hour_end));
		v.push_back(std::to_string(e.day_end));
		v.push_back(std::to_string(e.month_end));
		v.push_back(std::to_string(e.year_end));
		v.push_back("'" + Strings::Escape(e.cron_expression) + "'");
		v.push_back("FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");
		v.push_back("FROM_UNIXTIME(" + (e.deleted_at > 0 ? std::to_string(e.deleted_at) : "null") + ")");

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
		const std::vector<ServerScheduledEvents> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.description) + "'");
			v.push_back("'" + Strings::Escape(e.event_type) + "'");
			v.push_back("'" + Strings::Escape(e.event_data) + "'");
			v.push_back(std::to_string(e.minute_start));
			v.push_back(std::to_string(e.hour_start));
			v.push_back(std::to_string(e.day_start));
			v.push_back(std::to_string(e.month_start));
			v.push_back(std::to_string(e.year_start));
			v.push_back(std::to_string(e.minute_end));
			v.push_back(std::to_string(e.hour_end));
			v.push_back(std::to_string(e.day_end));
			v.push_back(std::to_string(e.month_end));
			v.push_back(std::to_string(e.year_end));
			v.push_back("'" + Strings::Escape(e.cron_expression) + "'");
			v.push_back("FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");
			v.push_back("FROM_UNIXTIME(" + (e.deleted_at > 0 ? std::to_string(e.deleted_at) : "null") + ")");

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

	static std::vector<ServerScheduledEvents> All(Database& db)
	{
		std::vector<ServerScheduledEvents> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ServerScheduledEvents e{};

			e.id              = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.description     = row[1] ? row[1] : "";
			e.event_type      = row[2] ? row[2] : "";
			e.event_data      = row[3] ? row[3] : "";
			e.minute_start    = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.hour_start      = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.day_start       = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.month_start     = row[7] ? static_cast<int32_t>(atoi(row[7])) : 0;
			e.year_start      = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.minute_end      = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.hour_end        = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.day_end         = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;
			e.month_end       = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.year_end        = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.cron_expression = row[14] ? row[14] : "";
			e.created_at      = strtoll(row[15] ? row[15] : "-1", nullptr, 10);
			e.deleted_at      = strtoll(row[16] ? row[16] : "-1", nullptr, 10);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<ServerScheduledEvents> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<ServerScheduledEvents> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ServerScheduledEvents e{};

			e.id              = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.description     = row[1] ? row[1] : "";
			e.event_type      = row[2] ? row[2] : "";
			e.event_data      = row[3] ? row[3] : "";
			e.minute_start    = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.hour_start      = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.day_start       = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.month_start     = row[7] ? static_cast<int32_t>(atoi(row[7])) : 0;
			e.year_start      = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.minute_end      = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.hour_end        = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.day_end         = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;
			e.month_end       = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.year_end        = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.cron_expression = row[14] ? row[14] : "";
			e.created_at      = strtoll(row[15] ? row[15] : "-1", nullptr, 10);
			e.deleted_at      = strtoll(row[16] ? row[16] : "-1", nullptr, 10);

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
		const ServerScheduledEvents &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.description) + "'");
		v.push_back("'" + Strings::Escape(e.event_type) + "'");
		v.push_back("'" + Strings::Escape(e.event_data) + "'");
		v.push_back(std::to_string(e.minute_start));
		v.push_back(std::to_string(e.hour_start));
		v.push_back(std::to_string(e.day_start));
		v.push_back(std::to_string(e.month_start));
		v.push_back(std::to_string(e.year_start));
		v.push_back(std::to_string(e.minute_end));
		v.push_back(std::to_string(e.hour_end));
		v.push_back(std::to_string(e.day_end));
		v.push_back(std::to_string(e.month_end));
		v.push_back(std::to_string(e.year_end));
		v.push_back("'" + Strings::Escape(e.cron_expression) + "'");
		v.push_back("FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");
		v.push_back("FROM_UNIXTIME(" + (e.deleted_at > 0 ? std::to_string(e.deleted_at) : "null") + ")");

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
		const std::vector<ServerScheduledEvents> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.description) + "'");
			v.push_back("'" + Strings::Escape(e.event_type) + "'");
			v.push_back("'" + Strings::Escape(e.event_data) + "'");
			v.push_back(std::to_string(e.minute_start));
			v.push_back(std::to_string(e.hour_start));
			v.push_back(std::to_string(e.day_start));
			v.push_back(std::to_string(e.month_start));
			v.push_back(std::to_string(e.year_start));
			v.push_back(std::to_string(e.minute_end));
			v.push_back(std::to_string(e.hour_end));
			v.push_back(std::to_string(e.day_end));
			v.push_back(std::to_string(e.month_end));
			v.push_back(std::to_string(e.year_end));
			v.push_back("'" + Strings::Escape(e.cron_expression) + "'");
			v.push_back("FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");
			v.push_back("FROM_UNIXTIME(" + (e.deleted_at > 0 ? std::to_string(e.deleted_at) : "null") + ")");

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
