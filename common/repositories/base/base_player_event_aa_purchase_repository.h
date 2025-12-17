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

class BasePlayerEventAaPurchaseRepository {
public:
	struct PlayerEventAaPurchase {
		uint64_t id;
		int32_t  aa_ability_id;
		int32_t  cost;
		int32_t  previous_id;
		int32_t  next_id;
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
			"aa_ability_id",
			"cost",
			"previous_id",
			"next_id",
			"created_at",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"aa_ability_id",
			"cost",
			"previous_id",
			"next_id",
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
		return std::string("player_event_aa_purchase");
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

	static PlayerEventAaPurchase NewEntity()
	{
		PlayerEventAaPurchase e{};

		e.id            = 0;
		e.aa_ability_id = 0;
		e.cost          = 0;
		e.previous_id   = 0;
		e.next_id       = 0;
		e.created_at    = 0;

		return e;
	}

	static PlayerEventAaPurchase GetPlayerEventAaPurchase(
		const std::vector<PlayerEventAaPurchase> &player_event_aa_purchases,
		int player_event_aa_purchase_id
	)
	{
		for (auto &player_event_aa_purchase : player_event_aa_purchases) {
			if (player_event_aa_purchase.id == player_event_aa_purchase_id) {
				return player_event_aa_purchase;
			}
		}

		return NewEntity();
	}

	static PlayerEventAaPurchase FindOne(
		Database& db,
		int player_event_aa_purchase_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				player_event_aa_purchase_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			PlayerEventAaPurchase e{};

			e.id            = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.aa_ability_id = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.cost          = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.previous_id   = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.next_id       = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.created_at    = strtoll(row[5] ? row[5] : "-1", nullptr, 10);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int player_event_aa_purchase_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				player_event_aa_purchase_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const PlayerEventAaPurchase &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.aa_ability_id));
		v.push_back(columns[2] + " = " + std::to_string(e.cost));
		v.push_back(columns[3] + " = " + std::to_string(e.previous_id));
		v.push_back(columns[4] + " = " + std::to_string(e.next_id));
		v.push_back(columns[5] + " = FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");

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

	static PlayerEventAaPurchase InsertOne(
		Database& db,
		PlayerEventAaPurchase e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.aa_ability_id));
		v.push_back(std::to_string(e.cost));
		v.push_back(std::to_string(e.previous_id));
		v.push_back(std::to_string(e.next_id));
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
		const std::vector<PlayerEventAaPurchase> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.aa_ability_id));
			v.push_back(std::to_string(e.cost));
			v.push_back(std::to_string(e.previous_id));
			v.push_back(std::to_string(e.next_id));
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

	static std::vector<PlayerEventAaPurchase> All(Database& db)
	{
		std::vector<PlayerEventAaPurchase> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PlayerEventAaPurchase e{};

			e.id            = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.aa_ability_id = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.cost          = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.previous_id   = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.next_id       = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.created_at    = strtoll(row[5] ? row[5] : "-1", nullptr, 10);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<PlayerEventAaPurchase> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<PlayerEventAaPurchase> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PlayerEventAaPurchase e{};

			e.id            = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.aa_ability_id = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.cost          = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.previous_id   = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.next_id       = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.created_at    = strtoll(row[5] ? row[5] : "-1", nullptr, 10);

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
		const PlayerEventAaPurchase &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.aa_ability_id));
		v.push_back(std::to_string(e.cost));
		v.push_back(std::to_string(e.previous_id));
		v.push_back(std::to_string(e.next_id));
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
		const std::vector<PlayerEventAaPurchase> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.aa_ability_id));
			v.push_back(std::to_string(e.cost));
			v.push_back(std::to_string(e.previous_id));
			v.push_back(std::to_string(e.next_id));
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
