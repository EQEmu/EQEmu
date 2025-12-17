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

class BaseCharacterPetBuffsRepository {
public:
	struct CharacterPetBuffs {
		int32_t     char_id;
		int32_t     pet;
		int32_t     slot;
		int32_t     spell_id;
		int8_t      caster_level;
		std::string castername;
		int32_t     ticsremaining;
		int32_t     counters;
		int32_t     numhits;
		int32_t     rune;
		uint8_t     instrument_mod;
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
			"slot",
			"spell_id",
			"caster_level",
			"castername",
			"ticsremaining",
			"counters",
			"numhits",
			"rune",
			"instrument_mod",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"char_id",
			"pet",
			"slot",
			"spell_id",
			"caster_level",
			"castername",
			"ticsremaining",
			"counters",
			"numhits",
			"rune",
			"instrument_mod",
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
		return std::string("character_pet_buffs");
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

	static CharacterPetBuffs NewEntity()
	{
		CharacterPetBuffs e{};

		e.char_id        = 0;
		e.pet            = 0;
		e.slot           = 0;
		e.spell_id       = 0;
		e.caster_level   = 0;
		e.castername     = "";
		e.ticsremaining  = 0;
		e.counters       = 0;
		e.numhits        = 0;
		e.rune           = 0;
		e.instrument_mod = 10;

		return e;
	}

	static CharacterPetBuffs GetCharacterPetBuffs(
		const std::vector<CharacterPetBuffs> &character_pet_buffss,
		int character_pet_buffs_id
	)
	{
		for (auto &character_pet_buffs : character_pet_buffss) {
			if (character_pet_buffs.char_id == character_pet_buffs_id) {
				return character_pet_buffs;
			}
		}

		return NewEntity();
	}

	static CharacterPetBuffs FindOne(
		Database& db,
		int character_pet_buffs_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				character_pet_buffs_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterPetBuffs e{};

			e.char_id        = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.pet            = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.slot           = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.spell_id       = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.caster_level   = row[4] ? static_cast<int8_t>(atoi(row[4])) : 0;
			e.castername     = row[5] ? row[5] : "";
			e.ticsremaining  = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.counters       = row[7] ? static_cast<int32_t>(atoi(row[7])) : 0;
			e.numhits        = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.rune           = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.instrument_mod = row[10] ? static_cast<uint8_t>(strtoul(row[10], nullptr, 10)) : 10;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_pet_buffs_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_pet_buffs_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CharacterPetBuffs &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.char_id));
		v.push_back(columns[1] + " = " + std::to_string(e.pet));
		v.push_back(columns[2] + " = " + std::to_string(e.slot));
		v.push_back(columns[3] + " = " + std::to_string(e.spell_id));
		v.push_back(columns[4] + " = " + std::to_string(e.caster_level));
		v.push_back(columns[5] + " = '" + Strings::Escape(e.castername) + "'");
		v.push_back(columns[6] + " = " + std::to_string(e.ticsremaining));
		v.push_back(columns[7] + " = " + std::to_string(e.counters));
		v.push_back(columns[8] + " = " + std::to_string(e.numhits));
		v.push_back(columns[9] + " = " + std::to_string(e.rune));
		v.push_back(columns[10] + " = " + std::to_string(e.instrument_mod));

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

	static CharacterPetBuffs InsertOne(
		Database& db,
		CharacterPetBuffs e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.char_id));
		v.push_back(std::to_string(e.pet));
		v.push_back(std::to_string(e.slot));
		v.push_back(std::to_string(e.spell_id));
		v.push_back(std::to_string(e.caster_level));
		v.push_back("'" + Strings::Escape(e.castername) + "'");
		v.push_back(std::to_string(e.ticsremaining));
		v.push_back(std::to_string(e.counters));
		v.push_back(std::to_string(e.numhits));
		v.push_back(std::to_string(e.rune));
		v.push_back(std::to_string(e.instrument_mod));

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
		const std::vector<CharacterPetBuffs> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.char_id));
			v.push_back(std::to_string(e.pet));
			v.push_back(std::to_string(e.slot));
			v.push_back(std::to_string(e.spell_id));
			v.push_back(std::to_string(e.caster_level));
			v.push_back("'" + Strings::Escape(e.castername) + "'");
			v.push_back(std::to_string(e.ticsremaining));
			v.push_back(std::to_string(e.counters));
			v.push_back(std::to_string(e.numhits));
			v.push_back(std::to_string(e.rune));
			v.push_back(std::to_string(e.instrument_mod));

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

	static std::vector<CharacterPetBuffs> All(Database& db)
	{
		std::vector<CharacterPetBuffs> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterPetBuffs e{};

			e.char_id        = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.pet            = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.slot           = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.spell_id       = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.caster_level   = row[4] ? static_cast<int8_t>(atoi(row[4])) : 0;
			e.castername     = row[5] ? row[5] : "";
			e.ticsremaining  = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.counters       = row[7] ? static_cast<int32_t>(atoi(row[7])) : 0;
			e.numhits        = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.rune           = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.instrument_mod = row[10] ? static_cast<uint8_t>(strtoul(row[10], nullptr, 10)) : 10;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CharacterPetBuffs> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CharacterPetBuffs> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterPetBuffs e{};

			e.char_id        = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.pet            = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.slot           = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.spell_id       = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.caster_level   = row[4] ? static_cast<int8_t>(atoi(row[4])) : 0;
			e.castername     = row[5] ? row[5] : "";
			e.ticsremaining  = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.counters       = row[7] ? static_cast<int32_t>(atoi(row[7])) : 0;
			e.numhits        = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.rune           = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.instrument_mod = row[10] ? static_cast<uint8_t>(strtoul(row[10], nullptr, 10)) : 10;

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
		const CharacterPetBuffs &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.char_id));
		v.push_back(std::to_string(e.pet));
		v.push_back(std::to_string(e.slot));
		v.push_back(std::to_string(e.spell_id));
		v.push_back(std::to_string(e.caster_level));
		v.push_back("'" + Strings::Escape(e.castername) + "'");
		v.push_back(std::to_string(e.ticsremaining));
		v.push_back(std::to_string(e.counters));
		v.push_back(std::to_string(e.numhits));
		v.push_back(std::to_string(e.rune));
		v.push_back(std::to_string(e.instrument_mod));

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
		const std::vector<CharacterPetBuffs> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.char_id));
			v.push_back(std::to_string(e.pet));
			v.push_back(std::to_string(e.slot));
			v.push_back(std::to_string(e.spell_id));
			v.push_back(std::to_string(e.caster_level));
			v.push_back("'" + Strings::Escape(e.castername) + "'");
			v.push_back(std::to_string(e.ticsremaining));
			v.push_back(std::to_string(e.counters));
			v.push_back(std::to_string(e.numhits));
			v.push_back(std::to_string(e.rune));
			v.push_back(std::to_string(e.instrument_mod));

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
