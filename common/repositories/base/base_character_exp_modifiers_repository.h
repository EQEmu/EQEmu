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

class BaseCharacterExpModifiersRepository {
public:
	struct CharacterExpModifiers {
		int32_t character_id;
		int32_t zone_id;
		int32_t instance_version;
		float   aa_modifier;
		float   exp_modifier;
	};

	static std::string PrimaryKey()
	{
		return std::string("character_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"character_id",
			"zone_id",
			"instance_version",
			"aa_modifier",
			"exp_modifier",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"character_id",
			"zone_id",
			"instance_version",
			"aa_modifier",
			"exp_modifier",
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
		return std::string("character_exp_modifiers");
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

	static CharacterExpModifiers NewEntity()
	{
		CharacterExpModifiers e{};

		e.character_id     = 0;
		e.zone_id          = 0;
		e.instance_version = -1;
		e.aa_modifier      = 0;
		e.exp_modifier     = 0;

		return e;
	}

	static CharacterExpModifiers GetCharacterExpModifiers(
		const std::vector<CharacterExpModifiers> &character_exp_modifierss,
		int character_exp_modifiers_id
	)
	{
		for (auto &character_exp_modifiers : character_exp_modifierss) {
			if (character_exp_modifiers.character_id == character_exp_modifiers_id) {
				return character_exp_modifiers;
			}
		}

		return NewEntity();
	}

	static CharacterExpModifiers FindOne(
		Database& db,
		int character_exp_modifiers_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				character_exp_modifiers_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterExpModifiers e{};

			e.character_id     = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.zone_id          = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.instance_version = row[2] ? static_cast<int32_t>(atoi(row[2])) : -1;
			e.aa_modifier      = row[3] ? strtof(row[3], nullptr) : 0;
			e.exp_modifier     = row[4] ? strtof(row[4], nullptr) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_exp_modifiers_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_exp_modifiers_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CharacterExpModifiers &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.character_id));
		v.push_back(columns[1] + " = " + std::to_string(e.zone_id));
		v.push_back(columns[2] + " = " + std::to_string(e.instance_version));
		v.push_back(columns[3] + " = " + std::to_string(e.aa_modifier));
		v.push_back(columns[4] + " = " + std::to_string(e.exp_modifier));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.character_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterExpModifiers InsertOne(
		Database& db,
		CharacterExpModifiers e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.character_id));
		v.push_back(std::to_string(e.zone_id));
		v.push_back(std::to_string(e.instance_version));
		v.push_back(std::to_string(e.aa_modifier));
		v.push_back(std::to_string(e.exp_modifier));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.character_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<CharacterExpModifiers> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.character_id));
			v.push_back(std::to_string(e.zone_id));
			v.push_back(std::to_string(e.instance_version));
			v.push_back(std::to_string(e.aa_modifier));
			v.push_back(std::to_string(e.exp_modifier));

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

	static std::vector<CharacterExpModifiers> All(Database& db)
	{
		std::vector<CharacterExpModifiers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterExpModifiers e{};

			e.character_id     = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.zone_id          = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.instance_version = row[2] ? static_cast<int32_t>(atoi(row[2])) : -1;
			e.aa_modifier      = row[3] ? strtof(row[3], nullptr) : 0;
			e.exp_modifier     = row[4] ? strtof(row[4], nullptr) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CharacterExpModifiers> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CharacterExpModifiers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterExpModifiers e{};

			e.character_id     = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.zone_id          = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.instance_version = row[2] ? static_cast<int32_t>(atoi(row[2])) : -1;
			e.aa_modifier      = row[3] ? strtof(row[3], nullptr) : 0;
			e.exp_modifier     = row[4] ? strtof(row[4], nullptr) : 0;

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
		const CharacterExpModifiers &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.character_id));
		v.push_back(std::to_string(e.zone_id));
		v.push_back(std::to_string(e.instance_version));
		v.push_back(std::to_string(e.aa_modifier));
		v.push_back(std::to_string(e.exp_modifier));

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
		const std::vector<CharacterExpModifiers> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.character_id));
			v.push_back(std::to_string(e.zone_id));
			v.push_back(std::to_string(e.instance_version));
			v.push_back(std::to_string(e.aa_modifier));
			v.push_back(std::to_string(e.exp_modifier));

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
