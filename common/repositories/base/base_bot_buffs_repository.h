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

class BaseBotBuffsRepository {
public:
	struct BotBuffs {
		uint32_t buffs_index;
		uint32_t bot_id;
		uint32_t spell_id;
		uint8_t  caster_level;
		uint32_t duration_formula;
		uint32_t tics_remaining;
		uint32_t poison_counters;
		uint32_t disease_counters;
		uint32_t curse_counters;
		uint32_t corruption_counters;
		uint32_t numhits;
		uint32_t melee_rune;
		uint32_t magic_rune;
		uint32_t dot_rune;
		int8_t   persistent;
		int32_t  caston_x;
		int32_t  caston_y;
		int32_t  caston_z;
		uint32_t extra_di_chance;
		int32_t  instrument_mod;
	};

	static std::string PrimaryKey()
	{
		return std::string("buffs_index");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"buffs_index",
			"bot_id",
			"spell_id",
			"caster_level",
			"duration_formula",
			"tics_remaining",
			"poison_counters",
			"disease_counters",
			"curse_counters",
			"corruption_counters",
			"numhits",
			"melee_rune",
			"magic_rune",
			"dot_rune",
			"persistent",
			"caston_x",
			"caston_y",
			"caston_z",
			"extra_di_chance",
			"instrument_mod",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"buffs_index",
			"bot_id",
			"spell_id",
			"caster_level",
			"duration_formula",
			"tics_remaining",
			"poison_counters",
			"disease_counters",
			"curse_counters",
			"corruption_counters",
			"numhits",
			"melee_rune",
			"magic_rune",
			"dot_rune",
			"persistent",
			"caston_x",
			"caston_y",
			"caston_z",
			"extra_di_chance",
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
		return std::string("bot_buffs");
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

	static BotBuffs NewEntity()
	{
		BotBuffs e{};

		e.buffs_index         = 0;
		e.bot_id              = 0;
		e.spell_id            = 0;
		e.caster_level        = 0;
		e.duration_formula    = 0;
		e.tics_remaining      = 0;
		e.poison_counters     = 0;
		e.disease_counters    = 0;
		e.curse_counters      = 0;
		e.corruption_counters = 0;
		e.numhits             = 0;
		e.melee_rune          = 0;
		e.magic_rune          = 0;
		e.dot_rune            = 0;
		e.persistent          = 0;
		e.caston_x            = 0;
		e.caston_y            = 0;
		e.caston_z            = 0;
		e.extra_di_chance     = 0;
		e.instrument_mod      = 10;

		return e;
	}

	static BotBuffs GetBotBuffs(
		const std::vector<BotBuffs> &bot_buffss,
		int bot_buffs_id
	)
	{
		for (auto &bot_buffs : bot_buffss) {
			if (bot_buffs.buffs_index == bot_buffs_id) {
				return bot_buffs;
			}
		}

		return NewEntity();
	}

	static BotBuffs FindOne(
		Database& db,
		int bot_buffs_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				bot_buffs_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			BotBuffs e{};

			e.buffs_index         = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.bot_id              = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.spell_id            = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.caster_level        = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.duration_formula    = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.tics_remaining      = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.poison_counters     = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.disease_counters    = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.curse_counters      = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.corruption_counters = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.numhits             = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.melee_rune          = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.magic_rune          = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.dot_rune            = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.persistent          = row[14] ? static_cast<int8_t>(atoi(row[14])) : 0;
			e.caston_x            = row[15] ? static_cast<int32_t>(atoi(row[15])) : 0;
			e.caston_y            = row[16] ? static_cast<int32_t>(atoi(row[16])) : 0;
			e.caston_z            = row[17] ? static_cast<int32_t>(atoi(row[17])) : 0;
			e.extra_di_chance     = row[18] ? static_cast<uint32_t>(strtoul(row[18], nullptr, 10)) : 0;
			e.instrument_mod      = row[19] ? static_cast<int32_t>(atoi(row[19])) : 10;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int bot_buffs_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				bot_buffs_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const BotBuffs &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.bot_id));
		v.push_back(columns[2] + " = " + std::to_string(e.spell_id));
		v.push_back(columns[3] + " = " + std::to_string(e.caster_level));
		v.push_back(columns[4] + " = " + std::to_string(e.duration_formula));
		v.push_back(columns[5] + " = " + std::to_string(e.tics_remaining));
		v.push_back(columns[6] + " = " + std::to_string(e.poison_counters));
		v.push_back(columns[7] + " = " + std::to_string(e.disease_counters));
		v.push_back(columns[8] + " = " + std::to_string(e.curse_counters));
		v.push_back(columns[9] + " = " + std::to_string(e.corruption_counters));
		v.push_back(columns[10] + " = " + std::to_string(e.numhits));
		v.push_back(columns[11] + " = " + std::to_string(e.melee_rune));
		v.push_back(columns[12] + " = " + std::to_string(e.magic_rune));
		v.push_back(columns[13] + " = " + std::to_string(e.dot_rune));
		v.push_back(columns[14] + " = " + std::to_string(e.persistent));
		v.push_back(columns[15] + " = " + std::to_string(e.caston_x));
		v.push_back(columns[16] + " = " + std::to_string(e.caston_y));
		v.push_back(columns[17] + " = " + std::to_string(e.caston_z));
		v.push_back(columns[18] + " = " + std::to_string(e.extra_di_chance));
		v.push_back(columns[19] + " = " + std::to_string(e.instrument_mod));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.buffs_index
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static BotBuffs InsertOne(
		Database& db,
		BotBuffs e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.buffs_index));
		v.push_back(std::to_string(e.bot_id));
		v.push_back(std::to_string(e.spell_id));
		v.push_back(std::to_string(e.caster_level));
		v.push_back(std::to_string(e.duration_formula));
		v.push_back(std::to_string(e.tics_remaining));
		v.push_back(std::to_string(e.poison_counters));
		v.push_back(std::to_string(e.disease_counters));
		v.push_back(std::to_string(e.curse_counters));
		v.push_back(std::to_string(e.corruption_counters));
		v.push_back(std::to_string(e.numhits));
		v.push_back(std::to_string(e.melee_rune));
		v.push_back(std::to_string(e.magic_rune));
		v.push_back(std::to_string(e.dot_rune));
		v.push_back(std::to_string(e.persistent));
		v.push_back(std::to_string(e.caston_x));
		v.push_back(std::to_string(e.caston_y));
		v.push_back(std::to_string(e.caston_z));
		v.push_back(std::to_string(e.extra_di_chance));
		v.push_back(std::to_string(e.instrument_mod));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.buffs_index = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<BotBuffs> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.buffs_index));
			v.push_back(std::to_string(e.bot_id));
			v.push_back(std::to_string(e.spell_id));
			v.push_back(std::to_string(e.caster_level));
			v.push_back(std::to_string(e.duration_formula));
			v.push_back(std::to_string(e.tics_remaining));
			v.push_back(std::to_string(e.poison_counters));
			v.push_back(std::to_string(e.disease_counters));
			v.push_back(std::to_string(e.curse_counters));
			v.push_back(std::to_string(e.corruption_counters));
			v.push_back(std::to_string(e.numhits));
			v.push_back(std::to_string(e.melee_rune));
			v.push_back(std::to_string(e.magic_rune));
			v.push_back(std::to_string(e.dot_rune));
			v.push_back(std::to_string(e.persistent));
			v.push_back(std::to_string(e.caston_x));
			v.push_back(std::to_string(e.caston_y));
			v.push_back(std::to_string(e.caston_z));
			v.push_back(std::to_string(e.extra_di_chance));
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

	static std::vector<BotBuffs> All(Database& db)
	{
		std::vector<BotBuffs> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotBuffs e{};

			e.buffs_index         = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.bot_id              = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.spell_id            = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.caster_level        = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.duration_formula    = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.tics_remaining      = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.poison_counters     = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.disease_counters    = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.curse_counters      = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.corruption_counters = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.numhits             = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.melee_rune          = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.magic_rune          = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.dot_rune            = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.persistent          = row[14] ? static_cast<int8_t>(atoi(row[14])) : 0;
			e.caston_x            = row[15] ? static_cast<int32_t>(atoi(row[15])) : 0;
			e.caston_y            = row[16] ? static_cast<int32_t>(atoi(row[16])) : 0;
			e.caston_z            = row[17] ? static_cast<int32_t>(atoi(row[17])) : 0;
			e.extra_di_chance     = row[18] ? static_cast<uint32_t>(strtoul(row[18], nullptr, 10)) : 0;
			e.instrument_mod      = row[19] ? static_cast<int32_t>(atoi(row[19])) : 10;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<BotBuffs> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<BotBuffs> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotBuffs e{};

			e.buffs_index         = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.bot_id              = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.spell_id            = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.caster_level        = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.duration_formula    = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.tics_remaining      = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.poison_counters     = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.disease_counters    = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.curse_counters      = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.corruption_counters = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.numhits             = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.melee_rune          = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.magic_rune          = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.dot_rune            = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.persistent          = row[14] ? static_cast<int8_t>(atoi(row[14])) : 0;
			e.caston_x            = row[15] ? static_cast<int32_t>(atoi(row[15])) : 0;
			e.caston_y            = row[16] ? static_cast<int32_t>(atoi(row[16])) : 0;
			e.caston_z            = row[17] ? static_cast<int32_t>(atoi(row[17])) : 0;
			e.extra_di_chance     = row[18] ? static_cast<uint32_t>(strtoul(row[18], nullptr, 10)) : 0;
			e.instrument_mod      = row[19] ? static_cast<int32_t>(atoi(row[19])) : 10;

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
		const BotBuffs &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.buffs_index));
		v.push_back(std::to_string(e.bot_id));
		v.push_back(std::to_string(e.spell_id));
		v.push_back(std::to_string(e.caster_level));
		v.push_back(std::to_string(e.duration_formula));
		v.push_back(std::to_string(e.tics_remaining));
		v.push_back(std::to_string(e.poison_counters));
		v.push_back(std::to_string(e.disease_counters));
		v.push_back(std::to_string(e.curse_counters));
		v.push_back(std::to_string(e.corruption_counters));
		v.push_back(std::to_string(e.numhits));
		v.push_back(std::to_string(e.melee_rune));
		v.push_back(std::to_string(e.magic_rune));
		v.push_back(std::to_string(e.dot_rune));
		v.push_back(std::to_string(e.persistent));
		v.push_back(std::to_string(e.caston_x));
		v.push_back(std::to_string(e.caston_y));
		v.push_back(std::to_string(e.caston_z));
		v.push_back(std::to_string(e.extra_di_chance));
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
		const std::vector<BotBuffs> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.buffs_index));
			v.push_back(std::to_string(e.bot_id));
			v.push_back(std::to_string(e.spell_id));
			v.push_back(std::to_string(e.caster_level));
			v.push_back(std::to_string(e.duration_formula));
			v.push_back(std::to_string(e.tics_remaining));
			v.push_back(std::to_string(e.poison_counters));
			v.push_back(std::to_string(e.disease_counters));
			v.push_back(std::to_string(e.curse_counters));
			v.push_back(std::to_string(e.corruption_counters));
			v.push_back(std::to_string(e.numhits));
			v.push_back(std::to_string(e.melee_rune));
			v.push_back(std::to_string(e.magic_rune));
			v.push_back(std::to_string(e.dot_rune));
			v.push_back(std::to_string(e.persistent));
			v.push_back(std::to_string(e.caston_x));
			v.push_back(std::to_string(e.caston_y));
			v.push_back(std::to_string(e.caston_z));
			v.push_back(std::to_string(e.extra_di_chance));
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
