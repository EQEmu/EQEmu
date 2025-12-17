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

class BasePlayerEventNpcHandinEntriesRepository {
public:
	struct PlayerEventNpcHandinEntries {
		uint64_t id;
		uint64_t player_event_npc_handin_id;
		uint32_t type;
		uint32_t item_id;
		int32_t  charges;
		uint32_t evolve_level;
		uint64_t evolve_amount;
		uint32_t augment_1_id;
		uint32_t augment_2_id;
		uint32_t augment_3_id;
		uint32_t augment_4_id;
		uint32_t augment_5_id;
		uint32_t augment_6_id;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"player_event_npc_handin_id",
			"type",
			"item_id",
			"charges",
			"evolve_level",
			"evolve_amount",
			"augment_1_id",
			"augment_2_id",
			"augment_3_id",
			"augment_4_id",
			"augment_5_id",
			"augment_6_id",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"player_event_npc_handin_id",
			"type",
			"item_id",
			"charges",
			"evolve_level",
			"evolve_amount",
			"augment_1_id",
			"augment_2_id",
			"augment_3_id",
			"augment_4_id",
			"augment_5_id",
			"augment_6_id",
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
		return std::string("player_event_npc_handin_entries");
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

	static PlayerEventNpcHandinEntries NewEntity()
	{
		PlayerEventNpcHandinEntries e{};

		e.id                         = 0;
		e.player_event_npc_handin_id = 0;
		e.type                       = 0;
		e.item_id                    = 0;
		e.charges                    = 0;
		e.evolve_level               = 0;
		e.evolve_amount              = 0;
		e.augment_1_id               = 0;
		e.augment_2_id               = 0;
		e.augment_3_id               = 0;
		e.augment_4_id               = 0;
		e.augment_5_id               = 0;
		e.augment_6_id               = 0;

		return e;
	}

	static PlayerEventNpcHandinEntries GetPlayerEventNpcHandinEntries(
		const std::vector<PlayerEventNpcHandinEntries> &player_event_npc_handin_entriess,
		int player_event_npc_handin_entries_id
	)
	{
		for (auto &player_event_npc_handin_entries : player_event_npc_handin_entriess) {
			if (player_event_npc_handin_entries.id == player_event_npc_handin_entries_id) {
				return player_event_npc_handin_entries;
			}
		}

		return NewEntity();
	}

	static PlayerEventNpcHandinEntries FindOne(
		Database& db,
		int player_event_npc_handin_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				player_event_npc_handin_entries_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			PlayerEventNpcHandinEntries e{};

			e.id                         = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.player_event_npc_handin_id = row[1] ? strtoull(row[1], nullptr, 10) : 0;
			e.type                       = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.item_id                    = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.charges                    = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.evolve_level               = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.evolve_amount              = row[6] ? strtoull(row[6], nullptr, 10) : 0;
			e.augment_1_id               = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.augment_2_id               = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.augment_3_id               = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.augment_4_id               = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.augment_5_id               = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.augment_6_id               = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int player_event_npc_handin_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				player_event_npc_handin_entries_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const PlayerEventNpcHandinEntries &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.player_event_npc_handin_id));
		v.push_back(columns[2] + " = " + std::to_string(e.type));
		v.push_back(columns[3] + " = " + std::to_string(e.item_id));
		v.push_back(columns[4] + " = " + std::to_string(e.charges));
		v.push_back(columns[5] + " = " + std::to_string(e.evolve_level));
		v.push_back(columns[6] + " = " + std::to_string(e.evolve_amount));
		v.push_back(columns[7] + " = " + std::to_string(e.augment_1_id));
		v.push_back(columns[8] + " = " + std::to_string(e.augment_2_id));
		v.push_back(columns[9] + " = " + std::to_string(e.augment_3_id));
		v.push_back(columns[10] + " = " + std::to_string(e.augment_4_id));
		v.push_back(columns[11] + " = " + std::to_string(e.augment_5_id));
		v.push_back(columns[12] + " = " + std::to_string(e.augment_6_id));

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

	static PlayerEventNpcHandinEntries InsertOne(
		Database& db,
		PlayerEventNpcHandinEntries e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.player_event_npc_handin_id));
		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.charges));
		v.push_back(std::to_string(e.evolve_level));
		v.push_back(std::to_string(e.evolve_amount));
		v.push_back(std::to_string(e.augment_1_id));
		v.push_back(std::to_string(e.augment_2_id));
		v.push_back(std::to_string(e.augment_3_id));
		v.push_back(std::to_string(e.augment_4_id));
		v.push_back(std::to_string(e.augment_5_id));
		v.push_back(std::to_string(e.augment_6_id));

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
		const std::vector<PlayerEventNpcHandinEntries> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.player_event_npc_handin_id));
			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.charges));
			v.push_back(std::to_string(e.evolve_level));
			v.push_back(std::to_string(e.evolve_amount));
			v.push_back(std::to_string(e.augment_1_id));
			v.push_back(std::to_string(e.augment_2_id));
			v.push_back(std::to_string(e.augment_3_id));
			v.push_back(std::to_string(e.augment_4_id));
			v.push_back(std::to_string(e.augment_5_id));
			v.push_back(std::to_string(e.augment_6_id));

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

	static std::vector<PlayerEventNpcHandinEntries> All(Database& db)
	{
		std::vector<PlayerEventNpcHandinEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PlayerEventNpcHandinEntries e{};

			e.id                         = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.player_event_npc_handin_id = row[1] ? strtoull(row[1], nullptr, 10) : 0;
			e.type                       = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.item_id                    = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.charges                    = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.evolve_level               = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.evolve_amount              = row[6] ? strtoull(row[6], nullptr, 10) : 0;
			e.augment_1_id               = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.augment_2_id               = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.augment_3_id               = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.augment_4_id               = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.augment_5_id               = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.augment_6_id               = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<PlayerEventNpcHandinEntries> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<PlayerEventNpcHandinEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PlayerEventNpcHandinEntries e{};

			e.id                         = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.player_event_npc_handin_id = row[1] ? strtoull(row[1], nullptr, 10) : 0;
			e.type                       = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.item_id                    = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.charges                    = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.evolve_level               = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.evolve_amount              = row[6] ? strtoull(row[6], nullptr, 10) : 0;
			e.augment_1_id               = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.augment_2_id               = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.augment_3_id               = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.augment_4_id               = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.augment_5_id               = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.augment_6_id               = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;

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
		const PlayerEventNpcHandinEntries &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.player_event_npc_handin_id));
		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.charges));
		v.push_back(std::to_string(e.evolve_level));
		v.push_back(std::to_string(e.evolve_amount));
		v.push_back(std::to_string(e.augment_1_id));
		v.push_back(std::to_string(e.augment_2_id));
		v.push_back(std::to_string(e.augment_3_id));
		v.push_back(std::to_string(e.augment_4_id));
		v.push_back(std::to_string(e.augment_5_id));
		v.push_back(std::to_string(e.augment_6_id));

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
		const std::vector<PlayerEventNpcHandinEntries> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.player_event_npc_handin_id));
			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.charges));
			v.push_back(std::to_string(e.evolve_level));
			v.push_back(std::to_string(e.evolve_amount));
			v.push_back(std::to_string(e.augment_1_id));
			v.push_back(std::to_string(e.augment_2_id));
			v.push_back(std::to_string(e.augment_3_id));
			v.push_back(std::to_string(e.augment_4_id));
			v.push_back(std::to_string(e.augment_5_id));
			v.push_back(std::to_string(e.augment_6_id));

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
