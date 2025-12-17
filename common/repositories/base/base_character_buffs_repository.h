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

class BaseCharacterBuffsRepository {
public:
	struct CharacterBuffs {
		uint32_t    character_id;
		uint8_t     slot_id;
		uint16_t    spell_id;
		uint8_t     caster_level;
		std::string caster_name;
		int32_t     ticsremaining;
		uint32_t    counters;
		uint32_t    numhits;
		uint32_t    melee_rune;
		uint32_t    magic_rune;
		uint8_t     persistent;
		int32_t     dot_rune;
		int32_t     caston_x;
		int32_t     caston_y;
		int32_t     caston_z;
		int32_t     ExtraDIChance;
		int32_t     instrument_mod;
	};

	static std::string PrimaryKey()
	{
		return std::string("character_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"character_id",
			"slot_id",
			"spell_id",
			"caster_level",
			"caster_name",
			"ticsremaining",
			"counters",
			"numhits",
			"melee_rune",
			"magic_rune",
			"persistent",
			"dot_rune",
			"caston_x",
			"caston_y",
			"caston_z",
			"ExtraDIChance",
			"instrument_mod",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"character_id",
			"slot_id",
			"spell_id",
			"caster_level",
			"caster_name",
			"ticsremaining",
			"counters",
			"numhits",
			"melee_rune",
			"magic_rune",
			"persistent",
			"dot_rune",
			"caston_x",
			"caston_y",
			"caston_z",
			"ExtraDIChance",
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
		return std::string("character_buffs");
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

	static CharacterBuffs NewEntity()
	{
		CharacterBuffs e{};

		e.character_id   = 0;
		e.slot_id        = 0;
		e.spell_id       = 0;
		e.caster_level   = 0;
		e.caster_name    = "";
		e.ticsremaining  = 0;
		e.counters       = 0;
		e.numhits        = 0;
		e.melee_rune     = 0;
		e.magic_rune     = 0;
		e.persistent     = 0;
		e.dot_rune       = 0;
		e.caston_x       = 0;
		e.caston_y       = 0;
		e.caston_z       = 0;
		e.ExtraDIChance  = 0;
		e.instrument_mod = 10;

		return e;
	}

	static CharacterBuffs GetCharacterBuffs(
		const std::vector<CharacterBuffs> &character_buffss,
		int character_buffs_id
	)
	{
		for (auto &character_buffs : character_buffss) {
			if (character_buffs.character_id == character_buffs_id) {
				return character_buffs;
			}
		}

		return NewEntity();
	}

	static CharacterBuffs FindOne(
		Database& db,
		int character_buffs_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				character_buffs_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterBuffs e{};

			e.character_id   = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.slot_id        = row[1] ? static_cast<uint8_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.spell_id       = row[2] ? static_cast<uint16_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.caster_level   = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.caster_name    = row[4] ? row[4] : "";
			e.ticsremaining  = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.counters       = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.numhits        = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.melee_rune     = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.magic_rune     = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.persistent     = row[10] ? static_cast<uint8_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.dot_rune       = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;
			e.caston_x       = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.caston_y       = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.caston_z       = row[14] ? static_cast<int32_t>(atoi(row[14])) : 0;
			e.ExtraDIChance  = row[15] ? static_cast<int32_t>(atoi(row[15])) : 0;
			e.instrument_mod = row[16] ? static_cast<int32_t>(atoi(row[16])) : 10;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_buffs_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_buffs_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CharacterBuffs &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.character_id));
		v.push_back(columns[1] + " = " + std::to_string(e.slot_id));
		v.push_back(columns[2] + " = " + std::to_string(e.spell_id));
		v.push_back(columns[3] + " = " + std::to_string(e.caster_level));
		v.push_back(columns[4] + " = '" + Strings::Escape(e.caster_name) + "'");
		v.push_back(columns[5] + " = " + std::to_string(e.ticsremaining));
		v.push_back(columns[6] + " = " + std::to_string(e.counters));
		v.push_back(columns[7] + " = " + std::to_string(e.numhits));
		v.push_back(columns[8] + " = " + std::to_string(e.melee_rune));
		v.push_back(columns[9] + " = " + std::to_string(e.magic_rune));
		v.push_back(columns[10] + " = " + std::to_string(e.persistent));
		v.push_back(columns[11] + " = " + std::to_string(e.dot_rune));
		v.push_back(columns[12] + " = " + std::to_string(e.caston_x));
		v.push_back(columns[13] + " = " + std::to_string(e.caston_y));
		v.push_back(columns[14] + " = " + std::to_string(e.caston_z));
		v.push_back(columns[15] + " = " + std::to_string(e.ExtraDIChance));
		v.push_back(columns[16] + " = " + std::to_string(e.instrument_mod));

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

	static CharacterBuffs InsertOne(
		Database& db,
		CharacterBuffs e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.character_id));
		v.push_back(std::to_string(e.slot_id));
		v.push_back(std::to_string(e.spell_id));
		v.push_back(std::to_string(e.caster_level));
		v.push_back("'" + Strings::Escape(e.caster_name) + "'");
		v.push_back(std::to_string(e.ticsremaining));
		v.push_back(std::to_string(e.counters));
		v.push_back(std::to_string(e.numhits));
		v.push_back(std::to_string(e.melee_rune));
		v.push_back(std::to_string(e.magic_rune));
		v.push_back(std::to_string(e.persistent));
		v.push_back(std::to_string(e.dot_rune));
		v.push_back(std::to_string(e.caston_x));
		v.push_back(std::to_string(e.caston_y));
		v.push_back(std::to_string(e.caston_z));
		v.push_back(std::to_string(e.ExtraDIChance));
		v.push_back(std::to_string(e.instrument_mod));

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
		const std::vector<CharacterBuffs> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.character_id));
			v.push_back(std::to_string(e.slot_id));
			v.push_back(std::to_string(e.spell_id));
			v.push_back(std::to_string(e.caster_level));
			v.push_back("'" + Strings::Escape(e.caster_name) + "'");
			v.push_back(std::to_string(e.ticsremaining));
			v.push_back(std::to_string(e.counters));
			v.push_back(std::to_string(e.numhits));
			v.push_back(std::to_string(e.melee_rune));
			v.push_back(std::to_string(e.magic_rune));
			v.push_back(std::to_string(e.persistent));
			v.push_back(std::to_string(e.dot_rune));
			v.push_back(std::to_string(e.caston_x));
			v.push_back(std::to_string(e.caston_y));
			v.push_back(std::to_string(e.caston_z));
			v.push_back(std::to_string(e.ExtraDIChance));
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

	static std::vector<CharacterBuffs> All(Database& db)
	{
		std::vector<CharacterBuffs> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterBuffs e{};

			e.character_id   = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.slot_id        = row[1] ? static_cast<uint8_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.spell_id       = row[2] ? static_cast<uint16_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.caster_level   = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.caster_name    = row[4] ? row[4] : "";
			e.ticsremaining  = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.counters       = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.numhits        = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.melee_rune     = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.magic_rune     = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.persistent     = row[10] ? static_cast<uint8_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.dot_rune       = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;
			e.caston_x       = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.caston_y       = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.caston_z       = row[14] ? static_cast<int32_t>(atoi(row[14])) : 0;
			e.ExtraDIChance  = row[15] ? static_cast<int32_t>(atoi(row[15])) : 0;
			e.instrument_mod = row[16] ? static_cast<int32_t>(atoi(row[16])) : 10;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CharacterBuffs> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CharacterBuffs> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterBuffs e{};

			e.character_id   = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.slot_id        = row[1] ? static_cast<uint8_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.spell_id       = row[2] ? static_cast<uint16_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.caster_level   = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.caster_name    = row[4] ? row[4] : "";
			e.ticsremaining  = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.counters       = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.numhits        = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.melee_rune     = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.magic_rune     = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.persistent     = row[10] ? static_cast<uint8_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.dot_rune       = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;
			e.caston_x       = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.caston_y       = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.caston_z       = row[14] ? static_cast<int32_t>(atoi(row[14])) : 0;
			e.ExtraDIChance  = row[15] ? static_cast<int32_t>(atoi(row[15])) : 0;
			e.instrument_mod = row[16] ? static_cast<int32_t>(atoi(row[16])) : 10;

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
		const CharacterBuffs &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.character_id));
		v.push_back(std::to_string(e.slot_id));
		v.push_back(std::to_string(e.spell_id));
		v.push_back(std::to_string(e.caster_level));
		v.push_back("'" + Strings::Escape(e.caster_name) + "'");
		v.push_back(std::to_string(e.ticsremaining));
		v.push_back(std::to_string(e.counters));
		v.push_back(std::to_string(e.numhits));
		v.push_back(std::to_string(e.melee_rune));
		v.push_back(std::to_string(e.magic_rune));
		v.push_back(std::to_string(e.persistent));
		v.push_back(std::to_string(e.dot_rune));
		v.push_back(std::to_string(e.caston_x));
		v.push_back(std::to_string(e.caston_y));
		v.push_back(std::to_string(e.caston_z));
		v.push_back(std::to_string(e.ExtraDIChance));
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
		const std::vector<CharacterBuffs> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.character_id));
			v.push_back(std::to_string(e.slot_id));
			v.push_back(std::to_string(e.spell_id));
			v.push_back(std::to_string(e.caster_level));
			v.push_back("'" + Strings::Escape(e.caster_name) + "'");
			v.push_back(std::to_string(e.ticsremaining));
			v.push_back(std::to_string(e.counters));
			v.push_back(std::to_string(e.numhits));
			v.push_back(std::to_string(e.melee_rune));
			v.push_back(std::to_string(e.magic_rune));
			v.push_back(std::to_string(e.persistent));
			v.push_back(std::to_string(e.dot_rune));
			v.push_back(std::to_string(e.caston_x));
			v.push_back(std::to_string(e.caston_y));
			v.push_back(std::to_string(e.caston_z));
			v.push_back(std::to_string(e.ExtraDIChance));
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
