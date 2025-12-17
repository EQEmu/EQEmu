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

class BaseLdonTrapTemplatesRepository {
public:
	struct LdonTrapTemplates {
		uint32_t id;
		uint8_t  type;
		uint16_t spell_id;
		uint16_t skill;
		uint8_t  locked;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"type",
			"spell_id",
			"skill",
			"locked",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"type",
			"spell_id",
			"skill",
			"locked",
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
		return std::string("ldon_trap_templates");
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

	static LdonTrapTemplates NewEntity()
	{
		LdonTrapTemplates e{};

		e.id       = 0;
		e.type     = 1;
		e.spell_id = 0;
		e.skill    = 0;
		e.locked   = 0;

		return e;
	}

	static LdonTrapTemplates GetLdonTrapTemplates(
		const std::vector<LdonTrapTemplates> &ldon_trap_templatess,
		int ldon_trap_templates_id
	)
	{
		for (auto &ldon_trap_templates : ldon_trap_templatess) {
			if (ldon_trap_templates.id == ldon_trap_templates_id) {
				return ldon_trap_templates;
			}
		}

		return NewEntity();
	}

	static LdonTrapTemplates FindOne(
		Database& db,
		int ldon_trap_templates_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				ldon_trap_templates_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			LdonTrapTemplates e{};

			e.id       = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.type     = row[1] ? static_cast<uint8_t>(strtoul(row[1], nullptr, 10)) : 1;
			e.spell_id = row[2] ? static_cast<uint16_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.skill    = row[3] ? static_cast<uint16_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.locked   = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int ldon_trap_templates_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				ldon_trap_templates_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const LdonTrapTemplates &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.id));
		v.push_back(columns[1] + " = " + std::to_string(e.type));
		v.push_back(columns[2] + " = " + std::to_string(e.spell_id));
		v.push_back(columns[3] + " = " + std::to_string(e.skill));
		v.push_back(columns[4] + " = " + std::to_string(e.locked));

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

	static LdonTrapTemplates InsertOne(
		Database& db,
		LdonTrapTemplates e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.spell_id));
		v.push_back(std::to_string(e.skill));
		v.push_back(std::to_string(e.locked));

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
		const std::vector<LdonTrapTemplates> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.spell_id));
			v.push_back(std::to_string(e.skill));
			v.push_back(std::to_string(e.locked));

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

	static std::vector<LdonTrapTemplates> All(Database& db)
	{
		std::vector<LdonTrapTemplates> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LdonTrapTemplates e{};

			e.id       = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.type     = row[1] ? static_cast<uint8_t>(strtoul(row[1], nullptr, 10)) : 1;
			e.spell_id = row[2] ? static_cast<uint16_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.skill    = row[3] ? static_cast<uint16_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.locked   = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<LdonTrapTemplates> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<LdonTrapTemplates> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LdonTrapTemplates e{};

			e.id       = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.type     = row[1] ? static_cast<uint8_t>(strtoul(row[1], nullptr, 10)) : 1;
			e.spell_id = row[2] ? static_cast<uint16_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.skill    = row[3] ? static_cast<uint16_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.locked   = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;

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
		const LdonTrapTemplates &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.spell_id));
		v.push_back(std::to_string(e.skill));
		v.push_back(std::to_string(e.locked));

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
		const std::vector<LdonTrapTemplates> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.spell_id));
			v.push_back(std::to_string(e.skill));
			v.push_back(std::to_string(e.locked));

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
