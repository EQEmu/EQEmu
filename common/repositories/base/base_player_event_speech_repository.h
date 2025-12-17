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

class BasePlayerEventSpeechRepository {
public:
	struct PlayerEventSpeech {
		uint64_t    id;
		std::string to_char_id;
		std::string from_char_id;
		uint32_t    guild_id;
		uint32_t    type;
		uint32_t    min_status;
		std::string message;
		time_t      created_at;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"to_char_id",
			"from_char_id",
			"guild_id",
			"type",
			"min_status",
			"message",
			"created_at",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"to_char_id",
			"from_char_id",
			"guild_id",
			"type",
			"min_status",
			"message",
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
		return std::string("player_event_speech");
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

	static PlayerEventSpeech NewEntity()
	{
		PlayerEventSpeech e{};

		e.id           = 0;
		e.to_char_id   = "";
		e.from_char_id = "";
		e.guild_id     = 0;
		e.type         = 0;
		e.min_status   = 0;
		e.message      = "";
		e.created_at   = 0;

		return e;
	}

	static PlayerEventSpeech GetPlayerEventSpeech(
		const std::vector<PlayerEventSpeech> &player_event_speechs,
		int player_event_speech_id
	)
	{
		for (auto &player_event_speech : player_event_speechs) {
			if (player_event_speech.id == player_event_speech_id) {
				return player_event_speech;
			}
		}

		return NewEntity();
	}

	static PlayerEventSpeech FindOne(
		Database& db,
		int player_event_speech_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				player_event_speech_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			PlayerEventSpeech e{};

			e.id           = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.to_char_id   = row[1] ? row[1] : "";
			e.from_char_id = row[2] ? row[2] : "";
			e.guild_id     = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.type         = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.min_status   = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.message      = row[6] ? row[6] : "";
			e.created_at   = strtoll(row[7] ? row[7] : "-1", nullptr, 10);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int player_event_speech_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				player_event_speech_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const PlayerEventSpeech &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.to_char_id) + "'");
		v.push_back(columns[2] + " = '" + Strings::Escape(e.from_char_id) + "'");
		v.push_back(columns[3] + " = " + std::to_string(e.guild_id));
		v.push_back(columns[4] + " = " + std::to_string(e.type));
		v.push_back(columns[5] + " = " + std::to_string(e.min_status));
		v.push_back(columns[6] + " = '" + Strings::Escape(e.message) + "'");
		v.push_back(columns[7] + " = FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");

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

	static PlayerEventSpeech InsertOne(
		Database& db,
		PlayerEventSpeech e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.to_char_id) + "'");
		v.push_back("'" + Strings::Escape(e.from_char_id) + "'");
		v.push_back(std::to_string(e.guild_id));
		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.min_status));
		v.push_back("'" + Strings::Escape(e.message) + "'");
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
		const std::vector<PlayerEventSpeech> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.to_char_id) + "'");
			v.push_back("'" + Strings::Escape(e.from_char_id) + "'");
			v.push_back(std::to_string(e.guild_id));
			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.min_status));
			v.push_back("'" + Strings::Escape(e.message) + "'");
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

	static std::vector<PlayerEventSpeech> All(Database& db)
	{
		std::vector<PlayerEventSpeech> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PlayerEventSpeech e{};

			e.id           = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.to_char_id   = row[1] ? row[1] : "";
			e.from_char_id = row[2] ? row[2] : "";
			e.guild_id     = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.type         = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.min_status   = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.message      = row[6] ? row[6] : "";
			e.created_at   = strtoll(row[7] ? row[7] : "-1", nullptr, 10);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<PlayerEventSpeech> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<PlayerEventSpeech> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PlayerEventSpeech e{};

			e.id           = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.to_char_id   = row[1] ? row[1] : "";
			e.from_char_id = row[2] ? row[2] : "";
			e.guild_id     = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.type         = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.min_status   = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.message      = row[6] ? row[6] : "";
			e.created_at   = strtoll(row[7] ? row[7] : "-1", nullptr, 10);

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
		const PlayerEventSpeech &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.to_char_id) + "'");
		v.push_back("'" + Strings::Escape(e.from_char_id) + "'");
		v.push_back(std::to_string(e.guild_id));
		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.min_status));
		v.push_back("'" + Strings::Escape(e.message) + "'");
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
		const std::vector<PlayerEventSpeech> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.to_char_id) + "'");
			v.push_back("'" + Strings::Escape(e.from_char_id) + "'");
			v.push_back(std::to_string(e.guild_id));
			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.min_status));
			v.push_back("'" + Strings::Escape(e.message) + "'");
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
