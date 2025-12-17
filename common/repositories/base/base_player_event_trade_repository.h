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

class BasePlayerEventTradeRepository {
public:
	struct PlayerEventTrade {
		uint32_t id;
		uint32_t char1_id;
		uint32_t char2_id;
		uint64_t char1_copper;
		uint64_t char1_silver;
		uint64_t char1_gold;
		uint64_t char1_platinum;
		uint64_t char2_copper;
		uint64_t char2_silver;
		uint64_t char2_gold;
		uint64_t char2_platinum;
		time_t   created_at;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"char1_id",
			"char2_id",
			"char1_copper",
			"char1_silver",
			"char1_gold",
			"char1_platinum",
			"char2_copper",
			"char2_silver",
			"char2_gold",
			"char2_platinum",
			"created_at",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"char1_id",
			"char2_id",
			"char1_copper",
			"char1_silver",
			"char1_gold",
			"char1_platinum",
			"char2_copper",
			"char2_silver",
			"char2_gold",
			"char2_platinum",
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
		return std::string("player_event_trade");
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

	static PlayerEventTrade NewEntity()
	{
		PlayerEventTrade e{};

		e.id             = 0;
		e.char1_id       = 0;
		e.char2_id       = 0;
		e.char1_copper   = 0;
		e.char1_silver   = 0;
		e.char1_gold     = 0;
		e.char1_platinum = 0;
		e.char2_copper   = 0;
		e.char2_silver   = 0;
		e.char2_gold     = 0;
		e.char2_platinum = 0;
		e.created_at     = 0;

		return e;
	}

	static PlayerEventTrade GetPlayerEventTrade(
		const std::vector<PlayerEventTrade> &player_event_trades,
		int player_event_trade_id
	)
	{
		for (auto &player_event_trade : player_event_trades) {
			if (player_event_trade.id == player_event_trade_id) {
				return player_event_trade;
			}
		}

		return NewEntity();
	}

	static PlayerEventTrade FindOne(
		Database& db,
		int player_event_trade_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				player_event_trade_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			PlayerEventTrade e{};

			e.id             = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.char1_id       = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.char2_id       = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.char1_copper   = row[3] ? strtoull(row[3], nullptr, 10) : 0;
			e.char1_silver   = row[4] ? strtoull(row[4], nullptr, 10) : 0;
			e.char1_gold     = row[5] ? strtoull(row[5], nullptr, 10) : 0;
			e.char1_platinum = row[6] ? strtoull(row[6], nullptr, 10) : 0;
			e.char2_copper   = row[7] ? strtoull(row[7], nullptr, 10) : 0;
			e.char2_silver   = row[8] ? strtoull(row[8], nullptr, 10) : 0;
			e.char2_gold     = row[9] ? strtoull(row[9], nullptr, 10) : 0;
			e.char2_platinum = row[10] ? strtoull(row[10], nullptr, 10) : 0;
			e.created_at     = strtoll(row[11] ? row[11] : "-1", nullptr, 10);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int player_event_trade_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				player_event_trade_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const PlayerEventTrade &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.char1_id));
		v.push_back(columns[2] + " = " + std::to_string(e.char2_id));
		v.push_back(columns[3] + " = " + std::to_string(e.char1_copper));
		v.push_back(columns[4] + " = " + std::to_string(e.char1_silver));
		v.push_back(columns[5] + " = " + std::to_string(e.char1_gold));
		v.push_back(columns[6] + " = " + std::to_string(e.char1_platinum));
		v.push_back(columns[7] + " = " + std::to_string(e.char2_copper));
		v.push_back(columns[8] + " = " + std::to_string(e.char2_silver));
		v.push_back(columns[9] + " = " + std::to_string(e.char2_gold));
		v.push_back(columns[10] + " = " + std::to_string(e.char2_platinum));
		v.push_back(columns[11] + " = FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");

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

	static PlayerEventTrade InsertOne(
		Database& db,
		PlayerEventTrade e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.char1_id));
		v.push_back(std::to_string(e.char2_id));
		v.push_back(std::to_string(e.char1_copper));
		v.push_back(std::to_string(e.char1_silver));
		v.push_back(std::to_string(e.char1_gold));
		v.push_back(std::to_string(e.char1_platinum));
		v.push_back(std::to_string(e.char2_copper));
		v.push_back(std::to_string(e.char2_silver));
		v.push_back(std::to_string(e.char2_gold));
		v.push_back(std::to_string(e.char2_platinum));
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
		const std::vector<PlayerEventTrade> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.char1_id));
			v.push_back(std::to_string(e.char2_id));
			v.push_back(std::to_string(e.char1_copper));
			v.push_back(std::to_string(e.char1_silver));
			v.push_back(std::to_string(e.char1_gold));
			v.push_back(std::to_string(e.char1_platinum));
			v.push_back(std::to_string(e.char2_copper));
			v.push_back(std::to_string(e.char2_silver));
			v.push_back(std::to_string(e.char2_gold));
			v.push_back(std::to_string(e.char2_platinum));
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

	static std::vector<PlayerEventTrade> All(Database& db)
	{
		std::vector<PlayerEventTrade> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PlayerEventTrade e{};

			e.id             = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.char1_id       = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.char2_id       = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.char1_copper   = row[3] ? strtoull(row[3], nullptr, 10) : 0;
			e.char1_silver   = row[4] ? strtoull(row[4], nullptr, 10) : 0;
			e.char1_gold     = row[5] ? strtoull(row[5], nullptr, 10) : 0;
			e.char1_platinum = row[6] ? strtoull(row[6], nullptr, 10) : 0;
			e.char2_copper   = row[7] ? strtoull(row[7], nullptr, 10) : 0;
			e.char2_silver   = row[8] ? strtoull(row[8], nullptr, 10) : 0;
			e.char2_gold     = row[9] ? strtoull(row[9], nullptr, 10) : 0;
			e.char2_platinum = row[10] ? strtoull(row[10], nullptr, 10) : 0;
			e.created_at     = strtoll(row[11] ? row[11] : "-1", nullptr, 10);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<PlayerEventTrade> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<PlayerEventTrade> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PlayerEventTrade e{};

			e.id             = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.char1_id       = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.char2_id       = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.char1_copper   = row[3] ? strtoull(row[3], nullptr, 10) : 0;
			e.char1_silver   = row[4] ? strtoull(row[4], nullptr, 10) : 0;
			e.char1_gold     = row[5] ? strtoull(row[5], nullptr, 10) : 0;
			e.char1_platinum = row[6] ? strtoull(row[6], nullptr, 10) : 0;
			e.char2_copper   = row[7] ? strtoull(row[7], nullptr, 10) : 0;
			e.char2_silver   = row[8] ? strtoull(row[8], nullptr, 10) : 0;
			e.char2_gold     = row[9] ? strtoull(row[9], nullptr, 10) : 0;
			e.char2_platinum = row[10] ? strtoull(row[10], nullptr, 10) : 0;
			e.created_at     = strtoll(row[11] ? row[11] : "-1", nullptr, 10);

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
		const PlayerEventTrade &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.char1_id));
		v.push_back(std::to_string(e.char2_id));
		v.push_back(std::to_string(e.char1_copper));
		v.push_back(std::to_string(e.char1_silver));
		v.push_back(std::to_string(e.char1_gold));
		v.push_back(std::to_string(e.char1_platinum));
		v.push_back(std::to_string(e.char2_copper));
		v.push_back(std::to_string(e.char2_silver));
		v.push_back(std::to_string(e.char2_gold));
		v.push_back(std::to_string(e.char2_platinum));
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
		const std::vector<PlayerEventTrade> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.char1_id));
			v.push_back(std::to_string(e.char2_id));
			v.push_back(std::to_string(e.char1_copper));
			v.push_back(std::to_string(e.char1_silver));
			v.push_back(std::to_string(e.char1_gold));
			v.push_back(std::to_string(e.char1_platinum));
			v.push_back(std::to_string(e.char2_copper));
			v.push_back(std::to_string(e.char2_silver));
			v.push_back(std::to_string(e.char2_gold));
			v.push_back(std::to_string(e.char2_platinum));
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
