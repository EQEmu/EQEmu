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

class BaseSpellGlobalsRepository {
public:
	struct SpellGlobals {
		int32_t     spellid;
		std::string spell_name;
		std::string qglobal;
		std::string value;
	};

	static std::string PrimaryKey()
	{
		return std::string("spellid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"spellid",
			"spell_name",
			"qglobal",
			"value",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"spellid",
			"spell_name",
			"qglobal",
			"value",
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
		return std::string("spell_globals");
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

	static SpellGlobals NewEntity()
	{
		SpellGlobals e{};

		e.spellid    = 0;
		e.spell_name = "";
		e.qglobal    = "";
		e.value      = "";

		return e;
	}

	static SpellGlobals GetSpellGlobals(
		const std::vector<SpellGlobals> &spell_globalss,
		int spell_globals_id
	)
	{
		for (auto &spell_globals : spell_globalss) {
			if (spell_globals.spellid == spell_globals_id) {
				return spell_globals;
			}
		}

		return NewEntity();
	}

	static SpellGlobals FindOne(
		Database& db,
		int spell_globals_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				spell_globals_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			SpellGlobals e{};

			e.spellid    = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.spell_name = row[1] ? row[1] : "";
			e.qglobal    = row[2] ? row[2] : "";
			e.value      = row[3] ? row[3] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int spell_globals_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				spell_globals_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const SpellGlobals &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.spellid));
		v.push_back(columns[1] + " = '" + Strings::Escape(e.spell_name) + "'");
		v.push_back(columns[2] + " = '" + Strings::Escape(e.qglobal) + "'");
		v.push_back(columns[3] + " = '" + Strings::Escape(e.value) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.spellid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static SpellGlobals InsertOne(
		Database& db,
		SpellGlobals e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.spellid));
		v.push_back("'" + Strings::Escape(e.spell_name) + "'");
		v.push_back("'" + Strings::Escape(e.qglobal) + "'");
		v.push_back("'" + Strings::Escape(e.value) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.spellid = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<SpellGlobals> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.spellid));
			v.push_back("'" + Strings::Escape(e.spell_name) + "'");
			v.push_back("'" + Strings::Escape(e.qglobal) + "'");
			v.push_back("'" + Strings::Escape(e.value) + "'");

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

	static std::vector<SpellGlobals> All(Database& db)
	{
		std::vector<SpellGlobals> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SpellGlobals e{};

			e.spellid    = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.spell_name = row[1] ? row[1] : "";
			e.qglobal    = row[2] ? row[2] : "";
			e.value      = row[3] ? row[3] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<SpellGlobals> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<SpellGlobals> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SpellGlobals e{};

			e.spellid    = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.spell_name = row[1] ? row[1] : "";
			e.qglobal    = row[2] ? row[2] : "";
			e.value      = row[3] ? row[3] : "";

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
		const SpellGlobals &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.spellid));
		v.push_back("'" + Strings::Escape(e.spell_name) + "'");
		v.push_back("'" + Strings::Escape(e.qglobal) + "'");
		v.push_back("'" + Strings::Escape(e.value) + "'");

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
		const std::vector<SpellGlobals> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.spellid));
			v.push_back("'" + Strings::Escape(e.spell_name) + "'");
			v.push_back("'" + Strings::Escape(e.qglobal) + "'");
			v.push_back("'" + Strings::Escape(e.value) + "'");

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
