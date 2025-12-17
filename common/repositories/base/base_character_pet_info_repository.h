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

class BaseCharacterPetInfoRepository {
public:
	struct CharacterPetInfo {
		int32_t     char_id;
		int32_t     pet;
		std::string petname;
		int32_t     petpower;
		int32_t     spell_id;
		int32_t     hp;
		int32_t     mana;
		float       size;
		int8_t      taunting;
	};

	static std::string PrimaryKey()
	{
		return std::string("char_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"char_id",
			"pet",
			"petname",
			"petpower",
			"spell_id",
			"hp",
			"mana",
			"size",
			"taunting",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"char_id",
			"pet",
			"petname",
			"petpower",
			"spell_id",
			"hp",
			"mana",
			"size",
			"taunting",
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
		return std::string("character_pet_info");
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

	static CharacterPetInfo NewEntity()
	{
		CharacterPetInfo e{};

		e.char_id  = 0;
		e.pet      = 0;
		e.petname  = "";
		e.petpower = 0;
		e.spell_id = 0;
		e.hp       = 0;
		e.mana     = 0;
		e.size     = 0;
		e.taunting = 1;

		return e;
	}

	static CharacterPetInfo GetCharacterPetInfo(
		const std::vector<CharacterPetInfo> &character_pet_infos,
		int character_pet_info_id
	)
	{
		for (auto &character_pet_info : character_pet_infos) {
			if (character_pet_info.char_id == character_pet_info_id) {
				return character_pet_info;
			}
		}

		return NewEntity();
	}

	static CharacterPetInfo FindOne(
		Database& db,
		int character_pet_info_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				character_pet_info_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterPetInfo e{};

			e.char_id  = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.pet      = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.petname  = row[2] ? row[2] : "";
			e.petpower = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.spell_id = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.hp       = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.mana     = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.size     = row[7] ? strtof(row[7], nullptr) : 0;
			e.taunting = row[8] ? static_cast<int8_t>(atoi(row[8])) : 1;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_pet_info_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_pet_info_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CharacterPetInfo &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.char_id));
		v.push_back(columns[1] + " = " + std::to_string(e.pet));
		v.push_back(columns[2] + " = '" + Strings::Escape(e.petname) + "'");
		v.push_back(columns[3] + " = " + std::to_string(e.petpower));
		v.push_back(columns[4] + " = " + std::to_string(e.spell_id));
		v.push_back(columns[5] + " = " + std::to_string(e.hp));
		v.push_back(columns[6] + " = " + std::to_string(e.mana));
		v.push_back(columns[7] + " = " + std::to_string(e.size));
		v.push_back(columns[8] + " = " + std::to_string(e.taunting));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.char_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterPetInfo InsertOne(
		Database& db,
		CharacterPetInfo e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.char_id));
		v.push_back(std::to_string(e.pet));
		v.push_back("'" + Strings::Escape(e.petname) + "'");
		v.push_back(std::to_string(e.petpower));
		v.push_back(std::to_string(e.spell_id));
		v.push_back(std::to_string(e.hp));
		v.push_back(std::to_string(e.mana));
		v.push_back(std::to_string(e.size));
		v.push_back(std::to_string(e.taunting));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.char_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<CharacterPetInfo> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.char_id));
			v.push_back(std::to_string(e.pet));
			v.push_back("'" + Strings::Escape(e.petname) + "'");
			v.push_back(std::to_string(e.petpower));
			v.push_back(std::to_string(e.spell_id));
			v.push_back(std::to_string(e.hp));
			v.push_back(std::to_string(e.mana));
			v.push_back(std::to_string(e.size));
			v.push_back(std::to_string(e.taunting));

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

	static std::vector<CharacterPetInfo> All(Database& db)
	{
		std::vector<CharacterPetInfo> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterPetInfo e{};

			e.char_id  = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.pet      = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.petname  = row[2] ? row[2] : "";
			e.petpower = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.spell_id = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.hp       = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.mana     = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.size     = row[7] ? strtof(row[7], nullptr) : 0;
			e.taunting = row[8] ? static_cast<int8_t>(atoi(row[8])) : 1;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CharacterPetInfo> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CharacterPetInfo> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterPetInfo e{};

			e.char_id  = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.pet      = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.petname  = row[2] ? row[2] : "";
			e.petpower = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.spell_id = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.hp       = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.mana     = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.size     = row[7] ? strtof(row[7], nullptr) : 0;
			e.taunting = row[8] ? static_cast<int8_t>(atoi(row[8])) : 1;

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
		const CharacterPetInfo &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.char_id));
		v.push_back(std::to_string(e.pet));
		v.push_back("'" + Strings::Escape(e.petname) + "'");
		v.push_back(std::to_string(e.petpower));
		v.push_back(std::to_string(e.spell_id));
		v.push_back(std::to_string(e.hp));
		v.push_back(std::to_string(e.mana));
		v.push_back(std::to_string(e.size));
		v.push_back(std::to_string(e.taunting));

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
		const std::vector<CharacterPetInfo> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.char_id));
			v.push_back(std::to_string(e.pet));
			v.push_back("'" + Strings::Escape(e.petname) + "'");
			v.push_back(std::to_string(e.petpower));
			v.push_back(std::to_string(e.spell_id));
			v.push_back(std::to_string(e.hp));
			v.push_back(std::to_string(e.mana));
			v.push_back(std::to_string(e.size));
			v.push_back(std::to_string(e.taunting));

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
