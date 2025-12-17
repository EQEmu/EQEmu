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

class BasePlayerEventNpcHandinRepository {
public:
	struct PlayerEventNpcHandin {
		uint64_t    id;
		uint32_t    npc_id;
		std::string npc_name;
		uint64_t    handin_copper;
		uint64_t    handin_silver;
		uint64_t    handin_gold;
		uint64_t    handin_platinum;
		uint64_t    return_copper;
		uint64_t    return_silver;
		uint64_t    return_gold;
		uint64_t    return_platinum;
		uint8_t     is_quest_handin;
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
			"npc_id",
			"npc_name",
			"handin_copper",
			"handin_silver",
			"handin_gold",
			"handin_platinum",
			"return_copper",
			"return_silver",
			"return_gold",
			"return_platinum",
			"is_quest_handin",
			"created_at",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"npc_id",
			"npc_name",
			"handin_copper",
			"handin_silver",
			"handin_gold",
			"handin_platinum",
			"return_copper",
			"return_silver",
			"return_gold",
			"return_platinum",
			"is_quest_handin",
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
		return std::string("player_event_npc_handin");
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

	static PlayerEventNpcHandin NewEntity()
	{
		PlayerEventNpcHandin e{};

		e.id              = 0;
		e.npc_id          = 0;
		e.npc_name        = "";
		e.handin_copper   = 0;
		e.handin_silver   = 0;
		e.handin_gold     = 0;
		e.handin_platinum = 0;
		e.return_copper   = 0;
		e.return_silver   = 0;
		e.return_gold     = 0;
		e.return_platinum = 0;
		e.is_quest_handin = 0;
		e.created_at      = 0;

		return e;
	}

	static PlayerEventNpcHandin GetPlayerEventNpcHandin(
		const std::vector<PlayerEventNpcHandin> &player_event_npc_handins,
		int player_event_npc_handin_id
	)
	{
		for (auto &player_event_npc_handin : player_event_npc_handins) {
			if (player_event_npc_handin.id == player_event_npc_handin_id) {
				return player_event_npc_handin;
			}
		}

		return NewEntity();
	}

	static PlayerEventNpcHandin FindOne(
		Database& db,
		int player_event_npc_handin_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				player_event_npc_handin_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			PlayerEventNpcHandin e{};

			e.id              = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.npc_id          = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.npc_name        = row[2] ? row[2] : "";
			e.handin_copper   = row[3] ? strtoull(row[3], nullptr, 10) : 0;
			e.handin_silver   = row[4] ? strtoull(row[4], nullptr, 10) : 0;
			e.handin_gold     = row[5] ? strtoull(row[5], nullptr, 10) : 0;
			e.handin_platinum = row[6] ? strtoull(row[6], nullptr, 10) : 0;
			e.return_copper   = row[7] ? strtoull(row[7], nullptr, 10) : 0;
			e.return_silver   = row[8] ? strtoull(row[8], nullptr, 10) : 0;
			e.return_gold     = row[9] ? strtoull(row[9], nullptr, 10) : 0;
			e.return_platinum = row[10] ? strtoull(row[10], nullptr, 10) : 0;
			e.is_quest_handin = row[11] ? static_cast<uint8_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.created_at      = strtoll(row[12] ? row[12] : "-1", nullptr, 10);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int player_event_npc_handin_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				player_event_npc_handin_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const PlayerEventNpcHandin &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.npc_id));
		v.push_back(columns[2] + " = '" + Strings::Escape(e.npc_name) + "'");
		v.push_back(columns[3] + " = " + std::to_string(e.handin_copper));
		v.push_back(columns[4] + " = " + std::to_string(e.handin_silver));
		v.push_back(columns[5] + " = " + std::to_string(e.handin_gold));
		v.push_back(columns[6] + " = " + std::to_string(e.handin_platinum));
		v.push_back(columns[7] + " = " + std::to_string(e.return_copper));
		v.push_back(columns[8] + " = " + std::to_string(e.return_silver));
		v.push_back(columns[9] + " = " + std::to_string(e.return_gold));
		v.push_back(columns[10] + " = " + std::to_string(e.return_platinum));
		v.push_back(columns[11] + " = " + std::to_string(e.is_quest_handin));
		v.push_back(columns[12] + " = FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");

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

	static PlayerEventNpcHandin InsertOne(
		Database& db,
		PlayerEventNpcHandin e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.npc_id));
		v.push_back("'" + Strings::Escape(e.npc_name) + "'");
		v.push_back(std::to_string(e.handin_copper));
		v.push_back(std::to_string(e.handin_silver));
		v.push_back(std::to_string(e.handin_gold));
		v.push_back(std::to_string(e.handin_platinum));
		v.push_back(std::to_string(e.return_copper));
		v.push_back(std::to_string(e.return_silver));
		v.push_back(std::to_string(e.return_gold));
		v.push_back(std::to_string(e.return_platinum));
		v.push_back(std::to_string(e.is_quest_handin));
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
		const std::vector<PlayerEventNpcHandin> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.npc_id));
			v.push_back("'" + Strings::Escape(e.npc_name) + "'");
			v.push_back(std::to_string(e.handin_copper));
			v.push_back(std::to_string(e.handin_silver));
			v.push_back(std::to_string(e.handin_gold));
			v.push_back(std::to_string(e.handin_platinum));
			v.push_back(std::to_string(e.return_copper));
			v.push_back(std::to_string(e.return_silver));
			v.push_back(std::to_string(e.return_gold));
			v.push_back(std::to_string(e.return_platinum));
			v.push_back(std::to_string(e.is_quest_handin));
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

	static std::vector<PlayerEventNpcHandin> All(Database& db)
	{
		std::vector<PlayerEventNpcHandin> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PlayerEventNpcHandin e{};

			e.id              = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.npc_id          = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.npc_name        = row[2] ? row[2] : "";
			e.handin_copper   = row[3] ? strtoull(row[3], nullptr, 10) : 0;
			e.handin_silver   = row[4] ? strtoull(row[4], nullptr, 10) : 0;
			e.handin_gold     = row[5] ? strtoull(row[5], nullptr, 10) : 0;
			e.handin_platinum = row[6] ? strtoull(row[6], nullptr, 10) : 0;
			e.return_copper   = row[7] ? strtoull(row[7], nullptr, 10) : 0;
			e.return_silver   = row[8] ? strtoull(row[8], nullptr, 10) : 0;
			e.return_gold     = row[9] ? strtoull(row[9], nullptr, 10) : 0;
			e.return_platinum = row[10] ? strtoull(row[10], nullptr, 10) : 0;
			e.is_quest_handin = row[11] ? static_cast<uint8_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.created_at      = strtoll(row[12] ? row[12] : "-1", nullptr, 10);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<PlayerEventNpcHandin> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<PlayerEventNpcHandin> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PlayerEventNpcHandin e{};

			e.id              = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.npc_id          = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.npc_name        = row[2] ? row[2] : "";
			e.handin_copper   = row[3] ? strtoull(row[3], nullptr, 10) : 0;
			e.handin_silver   = row[4] ? strtoull(row[4], nullptr, 10) : 0;
			e.handin_gold     = row[5] ? strtoull(row[5], nullptr, 10) : 0;
			e.handin_platinum = row[6] ? strtoull(row[6], nullptr, 10) : 0;
			e.return_copper   = row[7] ? strtoull(row[7], nullptr, 10) : 0;
			e.return_silver   = row[8] ? strtoull(row[8], nullptr, 10) : 0;
			e.return_gold     = row[9] ? strtoull(row[9], nullptr, 10) : 0;
			e.return_platinum = row[10] ? strtoull(row[10], nullptr, 10) : 0;
			e.is_quest_handin = row[11] ? static_cast<uint8_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.created_at      = strtoll(row[12] ? row[12] : "-1", nullptr, 10);

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
		const PlayerEventNpcHandin &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.npc_id));
		v.push_back("'" + Strings::Escape(e.npc_name) + "'");
		v.push_back(std::to_string(e.handin_copper));
		v.push_back(std::to_string(e.handin_silver));
		v.push_back(std::to_string(e.handin_gold));
		v.push_back(std::to_string(e.handin_platinum));
		v.push_back(std::to_string(e.return_copper));
		v.push_back(std::to_string(e.return_silver));
		v.push_back(std::to_string(e.return_gold));
		v.push_back(std::to_string(e.return_platinum));
		v.push_back(std::to_string(e.is_quest_handin));
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
		const std::vector<PlayerEventNpcHandin> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.npc_id));
			v.push_back("'" + Strings::Escape(e.npc_name) + "'");
			v.push_back(std::to_string(e.handin_copper));
			v.push_back(std::to_string(e.handin_silver));
			v.push_back(std::to_string(e.handin_gold));
			v.push_back(std::to_string(e.handin_platinum));
			v.push_back(std::to_string(e.return_copper));
			v.push_back(std::to_string(e.return_silver));
			v.push_back(std::to_string(e.return_gold));
			v.push_back(std::to_string(e.return_platinum));
			v.push_back(std::to_string(e.is_quest_handin));
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
