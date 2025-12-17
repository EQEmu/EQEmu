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

class BaseBotSpellsEntriesRepository {
public:
	struct BotSpellsEntries {
		uint32_t    id;
		int32_t     npc_spells_id;
		uint16_t    spell_id;
		uint32_t    type;
		uint8_t     minlevel;
		uint8_t     maxlevel;
		int16_t     manacost;
		int32_t     recast_delay;
		int16_t     priority;
		int32_t     resist_adjust;
		int16_t     min_hp;
		int16_t     max_hp;
		std::string bucket_name;
		std::string bucket_value;
		uint8_t     bucket_comparison;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"npc_spells_id",
			"spell_id",
			"type",
			"minlevel",
			"maxlevel",
			"manacost",
			"recast_delay",
			"priority",
			"resist_adjust",
			"min_hp",
			"max_hp",
			"bucket_name",
			"bucket_value",
			"bucket_comparison",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"npc_spells_id",
			"spell_id",
			"type",
			"minlevel",
			"maxlevel",
			"manacost",
			"recast_delay",
			"priority",
			"resist_adjust",
			"min_hp",
			"max_hp",
			"bucket_name",
			"bucket_value",
			"bucket_comparison",
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
		return std::string("bot_spells_entries");
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

	static BotSpellsEntries NewEntity()
	{
		BotSpellsEntries e{};

		e.id                = 0;
		e.npc_spells_id     = 0;
		e.spell_id          = 0;
		e.type              = 0;
		e.minlevel          = 0;
		e.maxlevel          = 255;
		e.manacost          = -1;
		e.recast_delay      = -1;
		e.priority          = 0;
		e.resist_adjust     = 0;
		e.min_hp            = 0;
		e.max_hp            = 0;
		e.bucket_name       = "";
		e.bucket_value      = "";
		e.bucket_comparison = 0;

		return e;
	}

	static BotSpellsEntries GetBotSpellsEntries(
		const std::vector<BotSpellsEntries> &bot_spells_entriess,
		int bot_spells_entries_id
	)
	{
		for (auto &bot_spells_entries : bot_spells_entriess) {
			if (bot_spells_entries.id == bot_spells_entries_id) {
				return bot_spells_entries;
			}
		}

		return NewEntity();
	}

	static BotSpellsEntries FindOne(
		Database& db,
		int bot_spells_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				bot_spells_entries_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			BotSpellsEntries e{};

			e.id                = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.npc_spells_id     = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.spell_id          = row[2] ? static_cast<uint16_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.type              = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.minlevel          = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.maxlevel          = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 255;
			e.manacost          = row[6] ? static_cast<int16_t>(atoi(row[6])) : -1;
			e.recast_delay      = row[7] ? static_cast<int32_t>(atoi(row[7])) : -1;
			e.priority          = row[8] ? static_cast<int16_t>(atoi(row[8])) : 0;
			e.resist_adjust     = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.min_hp            = row[10] ? static_cast<int16_t>(atoi(row[10])) : 0;
			e.max_hp            = row[11] ? static_cast<int16_t>(atoi(row[11])) : 0;
			e.bucket_name       = row[12] ? row[12] : "";
			e.bucket_value      = row[13] ? row[13] : "";
			e.bucket_comparison = row[14] ? static_cast<uint8_t>(strtoul(row[14], nullptr, 10)) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int bot_spells_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				bot_spells_entries_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const BotSpellsEntries &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.npc_spells_id));
		v.push_back(columns[2] + " = " + std::to_string(e.spell_id));
		v.push_back(columns[3] + " = " + std::to_string(e.type));
		v.push_back(columns[4] + " = " + std::to_string(e.minlevel));
		v.push_back(columns[5] + " = " + std::to_string(e.maxlevel));
		v.push_back(columns[6] + " = " + std::to_string(e.manacost));
		v.push_back(columns[7] + " = " + std::to_string(e.recast_delay));
		v.push_back(columns[8] + " = " + std::to_string(e.priority));
		v.push_back(columns[9] + " = " + std::to_string(e.resist_adjust));
		v.push_back(columns[10] + " = " + std::to_string(e.min_hp));
		v.push_back(columns[11] + " = " + std::to_string(e.max_hp));
		v.push_back(columns[12] + " = '" + Strings::Escape(e.bucket_name) + "'");
		v.push_back(columns[13] + " = '" + Strings::Escape(e.bucket_value) + "'");
		v.push_back(columns[14] + " = " + std::to_string(e.bucket_comparison));

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

	static BotSpellsEntries InsertOne(
		Database& db,
		BotSpellsEntries e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.npc_spells_id));
		v.push_back(std::to_string(e.spell_id));
		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.minlevel));
		v.push_back(std::to_string(e.maxlevel));
		v.push_back(std::to_string(e.manacost));
		v.push_back(std::to_string(e.recast_delay));
		v.push_back(std::to_string(e.priority));
		v.push_back(std::to_string(e.resist_adjust));
		v.push_back(std::to_string(e.min_hp));
		v.push_back(std::to_string(e.max_hp));
		v.push_back("'" + Strings::Escape(e.bucket_name) + "'");
		v.push_back("'" + Strings::Escape(e.bucket_value) + "'");
		v.push_back(std::to_string(e.bucket_comparison));

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
		const std::vector<BotSpellsEntries> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.npc_spells_id));
			v.push_back(std::to_string(e.spell_id));
			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.minlevel));
			v.push_back(std::to_string(e.maxlevel));
			v.push_back(std::to_string(e.manacost));
			v.push_back(std::to_string(e.recast_delay));
			v.push_back(std::to_string(e.priority));
			v.push_back(std::to_string(e.resist_adjust));
			v.push_back(std::to_string(e.min_hp));
			v.push_back(std::to_string(e.max_hp));
			v.push_back("'" + Strings::Escape(e.bucket_name) + "'");
			v.push_back("'" + Strings::Escape(e.bucket_value) + "'");
			v.push_back(std::to_string(e.bucket_comparison));

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

	static std::vector<BotSpellsEntries> All(Database& db)
	{
		std::vector<BotSpellsEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotSpellsEntries e{};

			e.id                = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.npc_spells_id     = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.spell_id          = row[2] ? static_cast<uint16_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.type              = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.minlevel          = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.maxlevel          = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 255;
			e.manacost          = row[6] ? static_cast<int16_t>(atoi(row[6])) : -1;
			e.recast_delay      = row[7] ? static_cast<int32_t>(atoi(row[7])) : -1;
			e.priority          = row[8] ? static_cast<int16_t>(atoi(row[8])) : 0;
			e.resist_adjust     = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.min_hp            = row[10] ? static_cast<int16_t>(atoi(row[10])) : 0;
			e.max_hp            = row[11] ? static_cast<int16_t>(atoi(row[11])) : 0;
			e.bucket_name       = row[12] ? row[12] : "";
			e.bucket_value      = row[13] ? row[13] : "";
			e.bucket_comparison = row[14] ? static_cast<uint8_t>(strtoul(row[14], nullptr, 10)) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<BotSpellsEntries> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<BotSpellsEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotSpellsEntries e{};

			e.id                = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.npc_spells_id     = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.spell_id          = row[2] ? static_cast<uint16_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.type              = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.minlevel          = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.maxlevel          = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 255;
			e.manacost          = row[6] ? static_cast<int16_t>(atoi(row[6])) : -1;
			e.recast_delay      = row[7] ? static_cast<int32_t>(atoi(row[7])) : -1;
			e.priority          = row[8] ? static_cast<int16_t>(atoi(row[8])) : 0;
			e.resist_adjust     = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.min_hp            = row[10] ? static_cast<int16_t>(atoi(row[10])) : 0;
			e.max_hp            = row[11] ? static_cast<int16_t>(atoi(row[11])) : 0;
			e.bucket_name       = row[12] ? row[12] : "";
			e.bucket_value      = row[13] ? row[13] : "";
			e.bucket_comparison = row[14] ? static_cast<uint8_t>(strtoul(row[14], nullptr, 10)) : 0;

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
		const BotSpellsEntries &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.npc_spells_id));
		v.push_back(std::to_string(e.spell_id));
		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.minlevel));
		v.push_back(std::to_string(e.maxlevel));
		v.push_back(std::to_string(e.manacost));
		v.push_back(std::to_string(e.recast_delay));
		v.push_back(std::to_string(e.priority));
		v.push_back(std::to_string(e.resist_adjust));
		v.push_back(std::to_string(e.min_hp));
		v.push_back(std::to_string(e.max_hp));
		v.push_back("'" + Strings::Escape(e.bucket_name) + "'");
		v.push_back("'" + Strings::Escape(e.bucket_value) + "'");
		v.push_back(std::to_string(e.bucket_comparison));

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
		const std::vector<BotSpellsEntries> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.npc_spells_id));
			v.push_back(std::to_string(e.spell_id));
			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.minlevel));
			v.push_back(std::to_string(e.maxlevel));
			v.push_back(std::to_string(e.manacost));
			v.push_back(std::to_string(e.recast_delay));
			v.push_back(std::to_string(e.priority));
			v.push_back(std::to_string(e.resist_adjust));
			v.push_back(std::to_string(e.min_hp));
			v.push_back(std::to_string(e.max_hp));
			v.push_back("'" + Strings::Escape(e.bucket_name) + "'");
			v.push_back("'" + Strings::Escape(e.bucket_value) + "'");
			v.push_back(std::to_string(e.bucket_comparison));

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
