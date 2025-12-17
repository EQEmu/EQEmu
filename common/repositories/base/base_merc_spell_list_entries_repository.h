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


class BaseMercSpellListEntriesRepository {
public:
	struct MercSpellListEntries {
		uint32_t merc_spell_list_entry_id;
		uint32_t merc_spell_list_id;
		uint32_t spell_id;
		uint32_t spell_type;
		uint8_t  stance_id;
		uint8_t  minlevel;
		uint8_t  maxlevel;
		int8_t   slot;
		uint8_t  procChance;
	};

	static std::string PrimaryKey()
	{
		return std::string("merc_spell_list_entry_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"merc_spell_list_entry_id",
			"merc_spell_list_id",
			"spell_id",
			"spell_type",
			"stance_id",
			"minlevel",
			"maxlevel",
			"slot",
			"procChance",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"merc_spell_list_entry_id",
			"merc_spell_list_id",
			"spell_id",
			"spell_type",
			"stance_id",
			"minlevel",
			"maxlevel",
			"slot",
			"procChance",
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
		return std::string("merc_spell_list_entries");
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

	static MercSpellListEntries NewEntity()
	{
		MercSpellListEntries e{};

		e.merc_spell_list_entry_id = 0;
		e.merc_spell_list_id       = 0;
		e.spell_id                 = 0;
		e.spell_type               = 0;
		e.stance_id                = 0;
		e.minlevel                 = 1;
		e.maxlevel                 = 255;
		e.slot                     = -1;
		e.procChance               = 0;

		return e;
	}

	static MercSpellListEntries GetMercSpellListEntries(
		const std::vector<MercSpellListEntries> &merc_spell_list_entriess,
		int merc_spell_list_entries_id
	)
	{
		for (auto &merc_spell_list_entries : merc_spell_list_entriess) {
			if (merc_spell_list_entries.merc_spell_list_entry_id == merc_spell_list_entries_id) {
				return merc_spell_list_entries;
			}
		}

		return NewEntity();
	}

	static MercSpellListEntries FindOne(
		Database& db,
		int merc_spell_list_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				merc_spell_list_entries_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			MercSpellListEntries e{};

			e.merc_spell_list_entry_id = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.merc_spell_list_id       = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.spell_id                 = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.spell_type               = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));
			e.stance_id                = static_cast<uint8_t>(strtoul(row[4], nullptr, 10));
			e.minlevel                 = static_cast<uint8_t>(strtoul(row[5], nullptr, 10));
			e.maxlevel                 = static_cast<uint8_t>(strtoul(row[6], nullptr, 10));
			e.slot                     = static_cast<int8_t>(atoi(row[7]));
			e.procChance               = static_cast<uint8_t>(strtoul(row[8], nullptr, 10));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int merc_spell_list_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				merc_spell_list_entries_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const MercSpellListEntries &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.merc_spell_list_id));
		v.push_back(columns[2] + " = " + std::to_string(e.spell_id));
		v.push_back(columns[3] + " = " + std::to_string(e.spell_type));
		v.push_back(columns[4] + " = " + std::to_string(e.stance_id));
		v.push_back(columns[5] + " = " + std::to_string(e.minlevel));
		v.push_back(columns[6] + " = " + std::to_string(e.maxlevel));
		v.push_back(columns[7] + " = " + std::to_string(e.slot));
		v.push_back(columns[8] + " = " + std::to_string(e.procChance));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.merc_spell_list_entry_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static MercSpellListEntries InsertOne(
		Database& db,
		MercSpellListEntries e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.merc_spell_list_entry_id));
		v.push_back(std::to_string(e.merc_spell_list_id));
		v.push_back(std::to_string(e.spell_id));
		v.push_back(std::to_string(e.spell_type));
		v.push_back(std::to_string(e.stance_id));
		v.push_back(std::to_string(e.minlevel));
		v.push_back(std::to_string(e.maxlevel));
		v.push_back(std::to_string(e.slot));
		v.push_back(std::to_string(e.procChance));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.merc_spell_list_entry_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<MercSpellListEntries> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.merc_spell_list_entry_id));
			v.push_back(std::to_string(e.merc_spell_list_id));
			v.push_back(std::to_string(e.spell_id));
			v.push_back(std::to_string(e.spell_type));
			v.push_back(std::to_string(e.stance_id));
			v.push_back(std::to_string(e.minlevel));
			v.push_back(std::to_string(e.maxlevel));
			v.push_back(std::to_string(e.slot));
			v.push_back(std::to_string(e.procChance));

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

	static std::vector<MercSpellListEntries> All(Database& db)
	{
		std::vector<MercSpellListEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MercSpellListEntries e{};

			e.merc_spell_list_entry_id = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.merc_spell_list_id       = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.spell_id                 = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.spell_type               = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));
			e.stance_id                = static_cast<uint8_t>(strtoul(row[4], nullptr, 10));
			e.minlevel                 = static_cast<uint8_t>(strtoul(row[5], nullptr, 10));
			e.maxlevel                 = static_cast<uint8_t>(strtoul(row[6], nullptr, 10));
			e.slot                     = static_cast<int8_t>(atoi(row[7]));
			e.procChance               = static_cast<uint8_t>(strtoul(row[8], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<MercSpellListEntries> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<MercSpellListEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MercSpellListEntries e{};

			e.merc_spell_list_entry_id = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.merc_spell_list_id       = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.spell_id                 = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.spell_type               = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));
			e.stance_id                = static_cast<uint8_t>(strtoul(row[4], nullptr, 10));
			e.minlevel                 = static_cast<uint8_t>(strtoul(row[5], nullptr, 10));
			e.maxlevel                 = static_cast<uint8_t>(strtoul(row[6], nullptr, 10));
			e.slot                     = static_cast<int8_t>(atoi(row[7]));
			e.procChance               = static_cast<uint8_t>(strtoul(row[8], nullptr, 10));

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
		const MercSpellListEntries &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.merc_spell_list_entry_id));
		v.push_back(std::to_string(e.merc_spell_list_id));
		v.push_back(std::to_string(e.spell_id));
		v.push_back(std::to_string(e.spell_type));
		v.push_back(std::to_string(e.stance_id));
		v.push_back(std::to_string(e.minlevel));
		v.push_back(std::to_string(e.maxlevel));
		v.push_back(std::to_string(e.slot));
		v.push_back(std::to_string(e.procChance));

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
		const std::vector<MercSpellListEntries> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.merc_spell_list_entry_id));
			v.push_back(std::to_string(e.merc_spell_list_id));
			v.push_back(std::to_string(e.spell_id));
			v.push_back(std::to_string(e.spell_type));
			v.push_back(std::to_string(e.stance_id));
			v.push_back(std::to_string(e.minlevel));
			v.push_back(std::to_string(e.maxlevel));
			v.push_back(std::to_string(e.slot));
			v.push_back(std::to_string(e.procChance));

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
