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

class BasePlayerEventKilledNpcRepository {
public:
	struct PlayerEventKilledNpc {
		uint64_t    id;
		uint32_t    npc_id;
		std::string npc_name;
		uint32_t    combat_time_seconds;
		uint64_t    total_damage_per_second_taken;
		uint64_t    total_heal_per_second_taken;
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
			"combat_time_seconds",
			"total_damage_per_second_taken",
			"total_heal_per_second_taken",
			"created_at",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"npc_id",
			"npc_name",
			"combat_time_seconds",
			"total_damage_per_second_taken",
			"total_heal_per_second_taken",
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
		return std::string("player_event_killed_npc");
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

	static PlayerEventKilledNpc NewEntity()
	{
		PlayerEventKilledNpc e{};

		e.id                            = 0;
		e.npc_id                        = 0;
		e.npc_name                      = "";
		e.combat_time_seconds           = 0;
		e.total_damage_per_second_taken = 0;
		e.total_heal_per_second_taken   = 0;
		e.created_at                    = 0;

		return e;
	}

	static PlayerEventKilledNpc GetPlayerEventKilledNpc(
		const std::vector<PlayerEventKilledNpc> &player_event_killed_npcs,
		int player_event_killed_npc_id
	)
	{
		for (auto &player_event_killed_npc : player_event_killed_npcs) {
			if (player_event_killed_npc.id == player_event_killed_npc_id) {
				return player_event_killed_npc;
			}
		}

		return NewEntity();
	}

	static PlayerEventKilledNpc FindOne(
		Database& db,
		int player_event_killed_npc_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				player_event_killed_npc_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			PlayerEventKilledNpc e{};

			e.id                            = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.npc_id                        = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.npc_name                      = row[2] ? row[2] : "";
			e.combat_time_seconds           = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.total_damage_per_second_taken = row[4] ? strtoull(row[4], nullptr, 10) : 0;
			e.total_heal_per_second_taken   = row[5] ? strtoull(row[5], nullptr, 10) : 0;
			e.created_at                    = strtoll(row[6] ? row[6] : "-1", nullptr, 10);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int player_event_killed_npc_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				player_event_killed_npc_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const PlayerEventKilledNpc &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.npc_id));
		v.push_back(columns[2] + " = '" + Strings::Escape(e.npc_name) + "'");
		v.push_back(columns[3] + " = " + std::to_string(e.combat_time_seconds));
		v.push_back(columns[4] + " = " + std::to_string(e.total_damage_per_second_taken));
		v.push_back(columns[5] + " = " + std::to_string(e.total_heal_per_second_taken));
		v.push_back(columns[6] + " = FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");

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

	static PlayerEventKilledNpc InsertOne(
		Database& db,
		PlayerEventKilledNpc e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.npc_id));
		v.push_back("'" + Strings::Escape(e.npc_name) + "'");
		v.push_back(std::to_string(e.combat_time_seconds));
		v.push_back(std::to_string(e.total_damage_per_second_taken));
		v.push_back(std::to_string(e.total_heal_per_second_taken));
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
		const std::vector<PlayerEventKilledNpc> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.npc_id));
			v.push_back("'" + Strings::Escape(e.npc_name) + "'");
			v.push_back(std::to_string(e.combat_time_seconds));
			v.push_back(std::to_string(e.total_damage_per_second_taken));
			v.push_back(std::to_string(e.total_heal_per_second_taken));
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

	static std::vector<PlayerEventKilledNpc> All(Database& db)
	{
		std::vector<PlayerEventKilledNpc> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PlayerEventKilledNpc e{};

			e.id                            = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.npc_id                        = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.npc_name                      = row[2] ? row[2] : "";
			e.combat_time_seconds           = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.total_damage_per_second_taken = row[4] ? strtoull(row[4], nullptr, 10) : 0;
			e.total_heal_per_second_taken   = row[5] ? strtoull(row[5], nullptr, 10) : 0;
			e.created_at                    = strtoll(row[6] ? row[6] : "-1", nullptr, 10);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<PlayerEventKilledNpc> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<PlayerEventKilledNpc> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PlayerEventKilledNpc e{};

			e.id                            = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.npc_id                        = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.npc_name                      = row[2] ? row[2] : "";
			e.combat_time_seconds           = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.total_damage_per_second_taken = row[4] ? strtoull(row[4], nullptr, 10) : 0;
			e.total_heal_per_second_taken   = row[5] ? strtoull(row[5], nullptr, 10) : 0;
			e.created_at                    = strtoll(row[6] ? row[6] : "-1", nullptr, 10);

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
		const PlayerEventKilledNpc &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.npc_id));
		v.push_back("'" + Strings::Escape(e.npc_name) + "'");
		v.push_back(std::to_string(e.combat_time_seconds));
		v.push_back(std::to_string(e.total_damage_per_second_taken));
		v.push_back(std::to_string(e.total_heal_per_second_taken));
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
		const std::vector<PlayerEventKilledNpc> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.npc_id));
			v.push_back("'" + Strings::Escape(e.npc_name) + "'");
			v.push_back(std::to_string(e.combat_time_seconds));
			v.push_back(std::to_string(e.total_damage_per_second_taken));
			v.push_back(std::to_string(e.total_heal_per_second_taken));
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
