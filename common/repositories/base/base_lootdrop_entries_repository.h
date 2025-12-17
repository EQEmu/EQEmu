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

class BaseLootdropEntriesRepository {
public:
	struct LootdropEntries {
		uint32_t    lootdrop_id;
		int32_t     item_id;
		uint16_t    item_charges;
		uint8_t     equip_item;
		float       chance;
		float       disabled_chance;
		uint16_t    trivial_min_level;
		uint16_t    trivial_max_level;
		uint8_t     multiplier;
		uint16_t    npc_min_level;
		uint16_t    npc_max_level;
		int8_t      min_expansion;
		int8_t      max_expansion;
		std::string content_flags;
		std::string content_flags_disabled;
	};

	static std::string PrimaryKey()
	{
		return std::string("lootdrop_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"lootdrop_id",
			"item_id",
			"item_charges",
			"equip_item",
			"chance",
			"disabled_chance",
			"trivial_min_level",
			"trivial_max_level",
			"multiplier",
			"npc_min_level",
			"npc_max_level",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"lootdrop_id",
			"item_id",
			"item_charges",
			"equip_item",
			"chance",
			"disabled_chance",
			"trivial_min_level",
			"trivial_max_level",
			"multiplier",
			"npc_min_level",
			"npc_max_level",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
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
		return std::string("lootdrop_entries");
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

	static LootdropEntries NewEntity()
	{
		LootdropEntries e{};

		e.lootdrop_id            = 0;
		e.item_id                = 0;
		e.item_charges           = 1;
		e.equip_item             = 0;
		e.chance                 = 1;
		e.disabled_chance        = 0;
		e.trivial_min_level      = 0;
		e.trivial_max_level      = 0;
		e.multiplier             = 1;
		e.npc_min_level          = 0;
		e.npc_max_level          = 0;
		e.min_expansion          = -1;
		e.max_expansion          = -1;
		e.content_flags          = "";
		e.content_flags_disabled = "";

		return e;
	}

	static LootdropEntries GetLootdropEntries(
		const std::vector<LootdropEntries> &lootdrop_entriess,
		int lootdrop_entries_id
	)
	{
		for (auto &lootdrop_entries : lootdrop_entriess) {
			if (lootdrop_entries.lootdrop_id == lootdrop_entries_id) {
				return lootdrop_entries;
			}
		}

		return NewEntity();
	}

	static LootdropEntries FindOne(
		Database& db,
		int lootdrop_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				lootdrop_entries_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			LootdropEntries e{};

			e.lootdrop_id            = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.item_id                = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.item_charges           = row[2] ? static_cast<uint16_t>(strtoul(row[2], nullptr, 10)) : 1;
			e.equip_item             = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.chance                 = row[4] ? strtof(row[4], nullptr) : 1;
			e.disabled_chance        = row[5] ? strtof(row[5], nullptr) : 0;
			e.trivial_min_level      = row[6] ? static_cast<uint16_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.trivial_max_level      = row[7] ? static_cast<uint16_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.multiplier             = row[8] ? static_cast<uint8_t>(strtoul(row[8], nullptr, 10)) : 1;
			e.npc_min_level          = row[9] ? static_cast<uint16_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.npc_max_level          = row[10] ? static_cast<uint16_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.min_expansion          = row[11] ? static_cast<int8_t>(atoi(row[11])) : -1;
			e.max_expansion          = row[12] ? static_cast<int8_t>(atoi(row[12])) : -1;
			e.content_flags          = row[13] ? row[13] : "";
			e.content_flags_disabled = row[14] ? row[14] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int lootdrop_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				lootdrop_entries_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const LootdropEntries &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.lootdrop_id));
		v.push_back(columns[1] + " = " + std::to_string(e.item_id));
		v.push_back(columns[2] + " = " + std::to_string(e.item_charges));
		v.push_back(columns[3] + " = " + std::to_string(e.equip_item));
		v.push_back(columns[4] + " = " + std::to_string(e.chance));
		v.push_back(columns[5] + " = " + std::to_string(e.disabled_chance));
		v.push_back(columns[6] + " = " + std::to_string(e.trivial_min_level));
		v.push_back(columns[7] + " = " + std::to_string(e.trivial_max_level));
		v.push_back(columns[8] + " = " + std::to_string(e.multiplier));
		v.push_back(columns[9] + " = " + std::to_string(e.npc_min_level));
		v.push_back(columns[10] + " = " + std::to_string(e.npc_max_level));
		v.push_back(columns[11] + " = " + std::to_string(e.min_expansion));
		v.push_back(columns[12] + " = " + std::to_string(e.max_expansion));
		v.push_back(columns[13] + " = '" + Strings::Escape(e.content_flags) + "'");
		v.push_back(columns[14] + " = '" + Strings::Escape(e.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.lootdrop_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static LootdropEntries InsertOne(
		Database& db,
		LootdropEntries e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.lootdrop_id));
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.item_charges));
		v.push_back(std::to_string(e.equip_item));
		v.push_back(std::to_string(e.chance));
		v.push_back(std::to_string(e.disabled_chance));
		v.push_back(std::to_string(e.trivial_min_level));
		v.push_back(std::to_string(e.trivial_max_level));
		v.push_back(std::to_string(e.multiplier));
		v.push_back(std::to_string(e.npc_min_level));
		v.push_back(std::to_string(e.npc_max_level));
		v.push_back(std::to_string(e.min_expansion));
		v.push_back(std::to_string(e.max_expansion));
		v.push_back("'" + Strings::Escape(e.content_flags) + "'");
		v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.lootdrop_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<LootdropEntries> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.lootdrop_id));
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.item_charges));
			v.push_back(std::to_string(e.equip_item));
			v.push_back(std::to_string(e.chance));
			v.push_back(std::to_string(e.disabled_chance));
			v.push_back(std::to_string(e.trivial_min_level));
			v.push_back(std::to_string(e.trivial_max_level));
			v.push_back(std::to_string(e.multiplier));
			v.push_back(std::to_string(e.npc_min_level));
			v.push_back(std::to_string(e.npc_max_level));
			v.push_back(std::to_string(e.min_expansion));
			v.push_back(std::to_string(e.max_expansion));
			v.push_back("'" + Strings::Escape(e.content_flags) + "'");
			v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

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

	static std::vector<LootdropEntries> All(Database& db)
	{
		std::vector<LootdropEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LootdropEntries e{};

			e.lootdrop_id            = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.item_id                = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.item_charges           = row[2] ? static_cast<uint16_t>(strtoul(row[2], nullptr, 10)) : 1;
			e.equip_item             = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.chance                 = row[4] ? strtof(row[4], nullptr) : 1;
			e.disabled_chance        = row[5] ? strtof(row[5], nullptr) : 0;
			e.trivial_min_level      = row[6] ? static_cast<uint16_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.trivial_max_level      = row[7] ? static_cast<uint16_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.multiplier             = row[8] ? static_cast<uint8_t>(strtoul(row[8], nullptr, 10)) : 1;
			e.npc_min_level          = row[9] ? static_cast<uint16_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.npc_max_level          = row[10] ? static_cast<uint16_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.min_expansion          = row[11] ? static_cast<int8_t>(atoi(row[11])) : -1;
			e.max_expansion          = row[12] ? static_cast<int8_t>(atoi(row[12])) : -1;
			e.content_flags          = row[13] ? row[13] : "";
			e.content_flags_disabled = row[14] ? row[14] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<LootdropEntries> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<LootdropEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LootdropEntries e{};

			e.lootdrop_id            = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.item_id                = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.item_charges           = row[2] ? static_cast<uint16_t>(strtoul(row[2], nullptr, 10)) : 1;
			e.equip_item             = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.chance                 = row[4] ? strtof(row[4], nullptr) : 1;
			e.disabled_chance        = row[5] ? strtof(row[5], nullptr) : 0;
			e.trivial_min_level      = row[6] ? static_cast<uint16_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.trivial_max_level      = row[7] ? static_cast<uint16_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.multiplier             = row[8] ? static_cast<uint8_t>(strtoul(row[8], nullptr, 10)) : 1;
			e.npc_min_level          = row[9] ? static_cast<uint16_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.npc_max_level          = row[10] ? static_cast<uint16_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.min_expansion          = row[11] ? static_cast<int8_t>(atoi(row[11])) : -1;
			e.max_expansion          = row[12] ? static_cast<int8_t>(atoi(row[12])) : -1;
			e.content_flags          = row[13] ? row[13] : "";
			e.content_flags_disabled = row[14] ? row[14] : "";

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
		const LootdropEntries &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.lootdrop_id));
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.item_charges));
		v.push_back(std::to_string(e.equip_item));
		v.push_back(std::to_string(e.chance));
		v.push_back(std::to_string(e.disabled_chance));
		v.push_back(std::to_string(e.trivial_min_level));
		v.push_back(std::to_string(e.trivial_max_level));
		v.push_back(std::to_string(e.multiplier));
		v.push_back(std::to_string(e.npc_min_level));
		v.push_back(std::to_string(e.npc_max_level));
		v.push_back(std::to_string(e.min_expansion));
		v.push_back(std::to_string(e.max_expansion));
		v.push_back("'" + Strings::Escape(e.content_flags) + "'");
		v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

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
		const std::vector<LootdropEntries> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.lootdrop_id));
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.item_charges));
			v.push_back(std::to_string(e.equip_item));
			v.push_back(std::to_string(e.chance));
			v.push_back(std::to_string(e.disabled_chance));
			v.push_back(std::to_string(e.trivial_min_level));
			v.push_back(std::to_string(e.trivial_max_level));
			v.push_back(std::to_string(e.multiplier));
			v.push_back(std::to_string(e.npc_min_level));
			v.push_back(std::to_string(e.npc_max_level));
			v.push_back(std::to_string(e.min_expansion));
			v.push_back(std::to_string(e.max_expansion));
			v.push_back("'" + Strings::Escape(e.content_flags) + "'");
			v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

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
