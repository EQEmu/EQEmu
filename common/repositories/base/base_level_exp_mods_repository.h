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

class BaseLevelExpModsRepository {
public:
	struct LevelExpMods {
		int32_t level;
		float   exp_mod;
		float   aa_exp_mod;
	};

	static std::string PrimaryKey()
	{
		return std::string("level");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"level",
			"exp_mod",
			"aa_exp_mod",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"level",
			"exp_mod",
			"aa_exp_mod",
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
		return std::string("level_exp_mods");
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

	static LevelExpMods NewEntity()
	{
		LevelExpMods e{};

		e.level      = 0;
		e.exp_mod    = 0;
		e.aa_exp_mod = 0;

		return e;
	}

	static LevelExpMods GetLevelExpMods(
		const std::vector<LevelExpMods> &level_exp_modss,
		int level_exp_mods_id
	)
	{
		for (auto &level_exp_mods : level_exp_modss) {
			if (level_exp_mods.level == level_exp_mods_id) {
				return level_exp_mods;
			}
		}

		return NewEntity();
	}

	static LevelExpMods FindOne(
		Database& db,
		int level_exp_mods_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				level_exp_mods_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			LevelExpMods e{};

			e.level      = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.exp_mod    = row[1] ? strtof(row[1], nullptr) : 0;
			e.aa_exp_mod = row[2] ? strtof(row[2], nullptr) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int level_exp_mods_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				level_exp_mods_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const LevelExpMods &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.level));
		v.push_back(columns[1] + " = " + std::to_string(e.exp_mod));
		v.push_back(columns[2] + " = " + std::to_string(e.aa_exp_mod));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.level
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static LevelExpMods InsertOne(
		Database& db,
		LevelExpMods e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.level));
		v.push_back(std::to_string(e.exp_mod));
		v.push_back(std::to_string(e.aa_exp_mod));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.level = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<LevelExpMods> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.level));
			v.push_back(std::to_string(e.exp_mod));
			v.push_back(std::to_string(e.aa_exp_mod));

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

	static std::vector<LevelExpMods> All(Database& db)
	{
		std::vector<LevelExpMods> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LevelExpMods e{};

			e.level      = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.exp_mod    = row[1] ? strtof(row[1], nullptr) : 0;
			e.aa_exp_mod = row[2] ? strtof(row[2], nullptr) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<LevelExpMods> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<LevelExpMods> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LevelExpMods e{};

			e.level      = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.exp_mod    = row[1] ? strtof(row[1], nullptr) : 0;
			e.aa_exp_mod = row[2] ? strtof(row[2], nullptr) : 0;

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
		const LevelExpMods &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.level));
		v.push_back(std::to_string(e.exp_mod));
		v.push_back(std::to_string(e.aa_exp_mod));

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
		const std::vector<LevelExpMods> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.level));
			v.push_back(std::to_string(e.exp_mod));
			v.push_back(std::to_string(e.aa_exp_mod));

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
