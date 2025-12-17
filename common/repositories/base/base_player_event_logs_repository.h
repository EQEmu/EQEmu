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
#include <cereal/cereal.hpp>
class BasePlayerEventLogsRepository {
public:
	struct PlayerEventLogs {
		int64_t     id;
		int64_t     account_id;
		int64_t     character_id;
		int32_t     zone_id;
		int32_t     instance_id;
		float       x;
		float       y;
		float       z;
		float       heading;
		int32_t     event_type_id;
		std::string event_type_name;
		std::string event_data;
		int64_t     etl_table_id;
		time_t      created_at;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(id),
				CEREAL_NVP(account_id),
				CEREAL_NVP(character_id),
				CEREAL_NVP(zone_id),
				CEREAL_NVP(instance_id),
				CEREAL_NVP(x),
				CEREAL_NVP(y),
				CEREAL_NVP(z),
				CEREAL_NVP(heading),
				CEREAL_NVP(event_type_id),
				CEREAL_NVP(event_type_name),
				CEREAL_NVP(event_data),
				CEREAL_NVP(etl_table_id),
				CEREAL_NVP(created_at)
			);
		}
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"account_id",
			"character_id",
			"zone_id",
			"instance_id",
			"x",
			"y",
			"z",
			"heading",
			"event_type_id",
			"event_type_name",
			"event_data",
			"etl_table_id",
			"created_at",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"account_id",
			"character_id",
			"zone_id",
			"instance_id",
			"x",
			"y",
			"z",
			"heading",
			"event_type_id",
			"event_type_name",
			"event_data",
			"etl_table_id",
			"UNIX_TIMESTAMP(created_at)",
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
		return std::string("player_event_logs");
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

	static PlayerEventLogs NewEntity()
	{
		PlayerEventLogs e{};

		e.id              = 0;
		e.account_id      = 0;
		e.character_id    = 0;
		e.zone_id         = 0;
		e.instance_id     = 0;
		e.x               = 0;
		e.y               = 0;
		e.z               = 0;
		e.heading         = 0;
		e.event_type_id   = 0;
		e.event_type_name = "";
		e.event_data      = "";
		e.etl_table_id    = 0;
		e.created_at      = 0;

		return e;
	}

	static PlayerEventLogs GetPlayerEventLogs(
		const std::vector<PlayerEventLogs> &player_event_logss,
		int player_event_logs_id
	)
	{
		for (auto &player_event_logs : player_event_logss) {
			if (player_event_logs.id == player_event_logs_id) {
				return player_event_logs;
			}
		}

		return NewEntity();
	}

	static PlayerEventLogs FindOne(
		Database& db,
		int player_event_logs_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				player_event_logs_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			PlayerEventLogs e{};

			e.id              = row[0] ? strtoll(row[0], nullptr, 10) : 0;
			e.account_id      = row[1] ? strtoll(row[1], nullptr, 10) : 0;
			e.character_id    = row[2] ? strtoll(row[2], nullptr, 10) : 0;
			e.zone_id         = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.instance_id     = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.x               = row[5] ? strtof(row[5], nullptr) : 0;
			e.y               = row[6] ? strtof(row[6], nullptr) : 0;
			e.z               = row[7] ? strtof(row[7], nullptr) : 0;
			e.heading         = row[8] ? strtof(row[8], nullptr) : 0;
			e.event_type_id   = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.event_type_name = row[10] ? row[10] : "";
			e.event_data      = row[11] ? row[11] : "";
			e.etl_table_id    = row[12] ? strtoll(row[12], nullptr, 10) : 0;
			e.created_at      = strtoll(row[13] ? row[13] : "-1", nullptr, 10);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int player_event_logs_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				player_event_logs_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const PlayerEventLogs &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.account_id));
		v.push_back(columns[2] + " = " + std::to_string(e.character_id));
		v.push_back(columns[3] + " = " + std::to_string(e.zone_id));
		v.push_back(columns[4] + " = " + std::to_string(e.instance_id));
		v.push_back(columns[5] + " = " + std::to_string(e.x));
		v.push_back(columns[6] + " = " + std::to_string(e.y));
		v.push_back(columns[7] + " = " + std::to_string(e.z));
		v.push_back(columns[8] + " = " + std::to_string(e.heading));
		v.push_back(columns[9] + " = " + std::to_string(e.event_type_id));
		v.push_back(columns[10] + " = '" + Strings::Escape(e.event_type_name) + "'");
		v.push_back(columns[11] + " = '" + Strings::Escape(e.event_data) + "'");
		v.push_back(columns[12] + " = " + std::to_string(e.etl_table_id));
		v.push_back(columns[13] + " = FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");

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

	static PlayerEventLogs InsertOne(
		Database& db,
		PlayerEventLogs e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.account_id));
		v.push_back(std::to_string(e.character_id));
		v.push_back(std::to_string(e.zone_id));
		v.push_back(std::to_string(e.instance_id));
		v.push_back(std::to_string(e.x));
		v.push_back(std::to_string(e.y));
		v.push_back(std::to_string(e.z));
		v.push_back(std::to_string(e.heading));
		v.push_back(std::to_string(e.event_type_id));
		v.push_back("'" + Strings::Escape(e.event_type_name) + "'");
		v.push_back("'" + Strings::Escape(e.event_data) + "'");
		v.push_back(std::to_string(e.etl_table_id));
		v.push_back("FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");

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
		const std::vector<PlayerEventLogs> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.account_id));
			v.push_back(std::to_string(e.character_id));
			v.push_back(std::to_string(e.zone_id));
			v.push_back(std::to_string(e.instance_id));
			v.push_back(std::to_string(e.x));
			v.push_back(std::to_string(e.y));
			v.push_back(std::to_string(e.z));
			v.push_back(std::to_string(e.heading));
			v.push_back(std::to_string(e.event_type_id));
			v.push_back("'" + Strings::Escape(e.event_type_name) + "'");
			v.push_back("'" + Strings::Escape(e.event_data) + "'");
			v.push_back(std::to_string(e.etl_table_id));
			v.push_back("FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");

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

	static std::vector<PlayerEventLogs> All(Database& db)
	{
		std::vector<PlayerEventLogs> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PlayerEventLogs e{};

			e.id              = row[0] ? strtoll(row[0], nullptr, 10) : 0;
			e.account_id      = row[1] ? strtoll(row[1], nullptr, 10) : 0;
			e.character_id    = row[2] ? strtoll(row[2], nullptr, 10) : 0;
			e.zone_id         = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.instance_id     = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.x               = row[5] ? strtof(row[5], nullptr) : 0;
			e.y               = row[6] ? strtof(row[6], nullptr) : 0;
			e.z               = row[7] ? strtof(row[7], nullptr) : 0;
			e.heading         = row[8] ? strtof(row[8], nullptr) : 0;
			e.event_type_id   = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.event_type_name = row[10] ? row[10] : "";
			e.event_data      = row[11] ? row[11] : "";
			e.etl_table_id    = row[12] ? strtoll(row[12], nullptr, 10) : 0;
			e.created_at      = strtoll(row[13] ? row[13] : "-1", nullptr, 10);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<PlayerEventLogs> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<PlayerEventLogs> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PlayerEventLogs e{};

			e.id              = row[0] ? strtoll(row[0], nullptr, 10) : 0;
			e.account_id      = row[1] ? strtoll(row[1], nullptr, 10) : 0;
			e.character_id    = row[2] ? strtoll(row[2], nullptr, 10) : 0;
			e.zone_id         = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.instance_id     = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.x               = row[5] ? strtof(row[5], nullptr) : 0;
			e.y               = row[6] ? strtof(row[6], nullptr) : 0;
			e.z               = row[7] ? strtof(row[7], nullptr) : 0;
			e.heading         = row[8] ? strtof(row[8], nullptr) : 0;
			e.event_type_id   = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.event_type_name = row[10] ? row[10] : "";
			e.event_data      = row[11] ? row[11] : "";
			e.etl_table_id    = row[12] ? strtoll(row[12], nullptr, 10) : 0;
			e.created_at      = strtoll(row[13] ? row[13] : "-1", nullptr, 10);

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
		const PlayerEventLogs &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.account_id));
		v.push_back(std::to_string(e.character_id));
		v.push_back(std::to_string(e.zone_id));
		v.push_back(std::to_string(e.instance_id));
		v.push_back(std::to_string(e.x));
		v.push_back(std::to_string(e.y));
		v.push_back(std::to_string(e.z));
		v.push_back(std::to_string(e.heading));
		v.push_back(std::to_string(e.event_type_id));
		v.push_back("'" + Strings::Escape(e.event_type_name) + "'");
		v.push_back("'" + Strings::Escape(e.event_data) + "'");
		v.push_back(std::to_string(e.etl_table_id));
		v.push_back("FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");

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
		const std::vector<PlayerEventLogs> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.account_id));
			v.push_back(std::to_string(e.character_id));
			v.push_back(std::to_string(e.zone_id));
			v.push_back(std::to_string(e.instance_id));
			v.push_back(std::to_string(e.x));
			v.push_back(std::to_string(e.y));
			v.push_back(std::to_string(e.z));
			v.push_back(std::to_string(e.heading));
			v.push_back(std::to_string(e.event_type_id));
			v.push_back("'" + Strings::Escape(e.event_type_name) + "'");
			v.push_back("'" + Strings::Escape(e.event_data) + "'");
			v.push_back(std::to_string(e.etl_table_id));
			v.push_back("FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");

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
