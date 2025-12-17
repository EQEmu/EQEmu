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
class BasePlayerEventLogSettingsRepository {
public:
	struct PlayerEventLogSettings {
		int64_t     id;
		std::string event_name;
		int8_t      event_enabled;
		int32_t     retention_days;
		int32_t     discord_webhook_id;
		uint8_t     etl_enabled;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(id),
				CEREAL_NVP(event_name),
				CEREAL_NVP(event_enabled),
				CEREAL_NVP(retention_days),
				CEREAL_NVP(discord_webhook_id),
				CEREAL_NVP(etl_enabled)
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
			"event_name",
			"event_enabled",
			"retention_days",
			"discord_webhook_id",
			"etl_enabled",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"event_name",
			"event_enabled",
			"retention_days",
			"discord_webhook_id",
			"etl_enabled",
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
		return std::string("player_event_log_settings");
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

	static PlayerEventLogSettings NewEntity()
	{
		PlayerEventLogSettings e{};

		e.id                 = 0;
		e.event_name         = "";
		e.event_enabled      = 0;
		e.retention_days     = 0;
		e.discord_webhook_id = 0;
		e.etl_enabled        = 0;

		return e;
	}

	static PlayerEventLogSettings GetPlayerEventLogSettings(
		const std::vector<PlayerEventLogSettings> &player_event_log_settingss,
		int player_event_log_settings_id
	)
	{
		for (auto &player_event_log_settings : player_event_log_settingss) {
			if (player_event_log_settings.id == player_event_log_settings_id) {
				return player_event_log_settings;
			}
		}

		return NewEntity();
	}

	static PlayerEventLogSettings FindOne(
		Database& db,
		int player_event_log_settings_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				player_event_log_settings_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			PlayerEventLogSettings e{};

			e.id                 = row[0] ? strtoll(row[0], nullptr, 10) : 0;
			e.event_name         = row[1] ? row[1] : "";
			e.event_enabled      = row[2] ? static_cast<int8_t>(atoi(row[2])) : 0;
			e.retention_days     = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.discord_webhook_id = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.etl_enabled        = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int player_event_log_settings_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				player_event_log_settings_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const PlayerEventLogSettings &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.id));
		v.push_back(columns[1] + " = '" + Strings::Escape(e.event_name) + "'");
		v.push_back(columns[2] + " = " + std::to_string(e.event_enabled));
		v.push_back(columns[3] + " = " + std::to_string(e.retention_days));
		v.push_back(columns[4] + " = " + std::to_string(e.discord_webhook_id));
		v.push_back(columns[5] + " = " + std::to_string(e.etl_enabled));

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

	static PlayerEventLogSettings InsertOne(
		Database& db,
		PlayerEventLogSettings e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.event_name) + "'");
		v.push_back(std::to_string(e.event_enabled));
		v.push_back(std::to_string(e.retention_days));
		v.push_back(std::to_string(e.discord_webhook_id));
		v.push_back(std::to_string(e.etl_enabled));

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
		const std::vector<PlayerEventLogSettings> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.event_name) + "'");
			v.push_back(std::to_string(e.event_enabled));
			v.push_back(std::to_string(e.retention_days));
			v.push_back(std::to_string(e.discord_webhook_id));
			v.push_back(std::to_string(e.etl_enabled));

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

	static std::vector<PlayerEventLogSettings> All(Database& db)
	{
		std::vector<PlayerEventLogSettings> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PlayerEventLogSettings e{};

			e.id                 = row[0] ? strtoll(row[0], nullptr, 10) : 0;
			e.event_name         = row[1] ? row[1] : "";
			e.event_enabled      = row[2] ? static_cast<int8_t>(atoi(row[2])) : 0;
			e.retention_days     = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.discord_webhook_id = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.etl_enabled        = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<PlayerEventLogSettings> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<PlayerEventLogSettings> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PlayerEventLogSettings e{};

			e.id                 = row[0] ? strtoll(row[0], nullptr, 10) : 0;
			e.event_name         = row[1] ? row[1] : "";
			e.event_enabled      = row[2] ? static_cast<int8_t>(atoi(row[2])) : 0;
			e.retention_days     = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.discord_webhook_id = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.etl_enabled        = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;

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
		const PlayerEventLogSettings &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.event_name) + "'");
		v.push_back(std::to_string(e.event_enabled));
		v.push_back(std::to_string(e.retention_days));
		v.push_back(std::to_string(e.discord_webhook_id));
		v.push_back(std::to_string(e.etl_enabled));

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
		const std::vector<PlayerEventLogSettings> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.event_name) + "'");
			v.push_back(std::to_string(e.event_enabled));
			v.push_back(std::to_string(e.retention_days));
			v.push_back(std::to_string(e.discord_webhook_id));
			v.push_back(std::to_string(e.etl_enabled));

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
