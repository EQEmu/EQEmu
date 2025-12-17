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


class BaseMercSpellListsRepository {
public:
	struct MercSpellLists {
		uint32_t    merc_spell_list_id;
		uint32_t    class_id;
		uint8_t     proficiency_id;
		std::string name;
	};

	static std::string PrimaryKey()
	{
		return std::string("merc_spell_list_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"merc_spell_list_id",
			"class_id",
			"proficiency_id",
			"name",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"merc_spell_list_id",
			"class_id",
			"proficiency_id",
			"name",
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
		return std::string("merc_spell_lists");
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

	static MercSpellLists NewEntity()
	{
		MercSpellLists e{};

		e.merc_spell_list_id = 0;
		e.class_id           = 0;
		e.proficiency_id     = 0;
		e.name               = "";

		return e;
	}

	static MercSpellLists GetMercSpellLists(
		const std::vector<MercSpellLists> &merc_spell_listss,
		int merc_spell_lists_id
	)
	{
		for (auto &merc_spell_lists : merc_spell_listss) {
			if (merc_spell_lists.merc_spell_list_id == merc_spell_lists_id) {
				return merc_spell_lists;
			}
		}

		return NewEntity();
	}

	static MercSpellLists FindOne(
		Database& db,
		int merc_spell_lists_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				merc_spell_lists_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			MercSpellLists e{};

			e.merc_spell_list_id = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.class_id           = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.proficiency_id     = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.name               = row[3] ? row[3] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int merc_spell_lists_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				merc_spell_lists_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const MercSpellLists &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.class_id));
		v.push_back(columns[2] + " = " + std::to_string(e.proficiency_id));
		v.push_back(columns[3] + " = '" + Strings::Escape(e.name) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.merc_spell_list_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static MercSpellLists InsertOne(
		Database& db,
		MercSpellLists e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.merc_spell_list_id));
		v.push_back(std::to_string(e.class_id));
		v.push_back(std::to_string(e.proficiency_id));
		v.push_back("'" + Strings::Escape(e.name) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.merc_spell_list_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<MercSpellLists> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.merc_spell_list_id));
			v.push_back(std::to_string(e.class_id));
			v.push_back(std::to_string(e.proficiency_id));
			v.push_back("'" + Strings::Escape(e.name) + "'");

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

	static std::vector<MercSpellLists> All(Database& db)
	{
		std::vector<MercSpellLists> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MercSpellLists e{};

			e.merc_spell_list_id = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.class_id           = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.proficiency_id     = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.name               = row[3] ? row[3] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<MercSpellLists> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<MercSpellLists> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MercSpellLists e{};

			e.merc_spell_list_id = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.class_id           = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.proficiency_id     = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.name               = row[3] ? row[3] : "";

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
		const MercSpellLists &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.merc_spell_list_id));
		v.push_back(std::to_string(e.class_id));
		v.push_back(std::to_string(e.proficiency_id));
		v.push_back("'" + Strings::Escape(e.name) + "'");

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
		const std::vector<MercSpellLists> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.merc_spell_list_id));
			v.push_back(std::to_string(e.class_id));
			v.push_back(std::to_string(e.proficiency_id));
			v.push_back("'" + Strings::Escape(e.name) + "'");

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
