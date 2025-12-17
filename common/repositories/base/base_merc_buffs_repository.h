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

class BaseMercBuffsRepository {
public:
	struct MercBuffs {
		uint32_t MercBuffId;
		uint32_t MercId;
		uint32_t SpellId;
		uint32_t CasterLevel;
		uint32_t DurationFormula;
		int32_t  TicsRemaining;
		uint32_t PoisonCounters;
		uint32_t DiseaseCounters;
		uint32_t CurseCounters;
		uint32_t CorruptionCounters;
		uint32_t HitCount;
		uint32_t MeleeRune;
		uint32_t MagicRune;
		int32_t  dot_rune;
		int32_t  caston_x;
		int8_t   Persistent;
		int32_t  caston_y;
		int32_t  caston_z;
		int32_t  ExtraDIChance;
	};

	static std::string PrimaryKey()
	{
		return std::string("MercBuffId");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"MercBuffId",
			"MercId",
			"SpellId",
			"CasterLevel",
			"DurationFormula",
			"TicsRemaining",
			"PoisonCounters",
			"DiseaseCounters",
			"CurseCounters",
			"CorruptionCounters",
			"HitCount",
			"MeleeRune",
			"MagicRune",
			"dot_rune",
			"caston_x",
			"Persistent",
			"caston_y",
			"caston_z",
			"ExtraDIChance",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"MercBuffId",
			"MercId",
			"SpellId",
			"CasterLevel",
			"DurationFormula",
			"TicsRemaining",
			"PoisonCounters",
			"DiseaseCounters",
			"CurseCounters",
			"CorruptionCounters",
			"HitCount",
			"MeleeRune",
			"MagicRune",
			"dot_rune",
			"caston_x",
			"Persistent",
			"caston_y",
			"caston_z",
			"ExtraDIChance",
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
		return std::string("merc_buffs");
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

	static MercBuffs NewEntity()
	{
		MercBuffs e{};

		e.MercBuffId         = 0;
		e.MercId             = 0;
		e.SpellId            = 0;
		e.CasterLevel        = 0;
		e.DurationFormula    = 0;
		e.TicsRemaining      = 0;
		e.PoisonCounters     = 0;
		e.DiseaseCounters    = 0;
		e.CurseCounters      = 0;
		e.CorruptionCounters = 0;
		e.HitCount           = 0;
		e.MeleeRune          = 0;
		e.MagicRune          = 0;
		e.dot_rune           = 0;
		e.caston_x           = 0;
		e.Persistent         = 0;
		e.caston_y           = 0;
		e.caston_z           = 0;
		e.ExtraDIChance      = 0;

		return e;
	}

	static MercBuffs GetMercBuffs(
		const std::vector<MercBuffs> &merc_buffss,
		int merc_buffs_id
	)
	{
		for (auto &merc_buffs : merc_buffss) {
			if (merc_buffs.MercBuffId == merc_buffs_id) {
				return merc_buffs;
			}
		}

		return NewEntity();
	}

	static MercBuffs FindOne(
		Database& db,
		int merc_buffs_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				merc_buffs_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			MercBuffs e{};

			e.MercBuffId         = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.MercId             = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.SpellId            = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.CasterLevel        = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.DurationFormula    = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.TicsRemaining      = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.PoisonCounters     = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.DiseaseCounters    = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.CurseCounters      = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.CorruptionCounters = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.HitCount           = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.MeleeRune          = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.MagicRune          = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.dot_rune           = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.caston_x           = row[14] ? static_cast<int32_t>(atoi(row[14])) : 0;
			e.Persistent         = row[15] ? static_cast<int8_t>(atoi(row[15])) : 0;
			e.caston_y           = row[16] ? static_cast<int32_t>(atoi(row[16])) : 0;
			e.caston_z           = row[17] ? static_cast<int32_t>(atoi(row[17])) : 0;
			e.ExtraDIChance      = row[18] ? static_cast<int32_t>(atoi(row[18])) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int merc_buffs_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				merc_buffs_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const MercBuffs &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.MercId));
		v.push_back(columns[2] + " = " + std::to_string(e.SpellId));
		v.push_back(columns[3] + " = " + std::to_string(e.CasterLevel));
		v.push_back(columns[4] + " = " + std::to_string(e.DurationFormula));
		v.push_back(columns[5] + " = " + std::to_string(e.TicsRemaining));
		v.push_back(columns[6] + " = " + std::to_string(e.PoisonCounters));
		v.push_back(columns[7] + " = " + std::to_string(e.DiseaseCounters));
		v.push_back(columns[8] + " = " + std::to_string(e.CurseCounters));
		v.push_back(columns[9] + " = " + std::to_string(e.CorruptionCounters));
		v.push_back(columns[10] + " = " + std::to_string(e.HitCount));
		v.push_back(columns[11] + " = " + std::to_string(e.MeleeRune));
		v.push_back(columns[12] + " = " + std::to_string(e.MagicRune));
		v.push_back(columns[13] + " = " + std::to_string(e.dot_rune));
		v.push_back(columns[14] + " = " + std::to_string(e.caston_x));
		v.push_back(columns[15] + " = " + std::to_string(e.Persistent));
		v.push_back(columns[16] + " = " + std::to_string(e.caston_y));
		v.push_back(columns[17] + " = " + std::to_string(e.caston_z));
		v.push_back(columns[18] + " = " + std::to_string(e.ExtraDIChance));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.MercBuffId
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static MercBuffs InsertOne(
		Database& db,
		MercBuffs e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.MercBuffId));
		v.push_back(std::to_string(e.MercId));
		v.push_back(std::to_string(e.SpellId));
		v.push_back(std::to_string(e.CasterLevel));
		v.push_back(std::to_string(e.DurationFormula));
		v.push_back(std::to_string(e.TicsRemaining));
		v.push_back(std::to_string(e.PoisonCounters));
		v.push_back(std::to_string(e.DiseaseCounters));
		v.push_back(std::to_string(e.CurseCounters));
		v.push_back(std::to_string(e.CorruptionCounters));
		v.push_back(std::to_string(e.HitCount));
		v.push_back(std::to_string(e.MeleeRune));
		v.push_back(std::to_string(e.MagicRune));
		v.push_back(std::to_string(e.dot_rune));
		v.push_back(std::to_string(e.caston_x));
		v.push_back(std::to_string(e.Persistent));
		v.push_back(std::to_string(e.caston_y));
		v.push_back(std::to_string(e.caston_z));
		v.push_back(std::to_string(e.ExtraDIChance));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.MercBuffId = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<MercBuffs> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.MercBuffId));
			v.push_back(std::to_string(e.MercId));
			v.push_back(std::to_string(e.SpellId));
			v.push_back(std::to_string(e.CasterLevel));
			v.push_back(std::to_string(e.DurationFormula));
			v.push_back(std::to_string(e.TicsRemaining));
			v.push_back(std::to_string(e.PoisonCounters));
			v.push_back(std::to_string(e.DiseaseCounters));
			v.push_back(std::to_string(e.CurseCounters));
			v.push_back(std::to_string(e.CorruptionCounters));
			v.push_back(std::to_string(e.HitCount));
			v.push_back(std::to_string(e.MeleeRune));
			v.push_back(std::to_string(e.MagicRune));
			v.push_back(std::to_string(e.dot_rune));
			v.push_back(std::to_string(e.caston_x));
			v.push_back(std::to_string(e.Persistent));
			v.push_back(std::to_string(e.caston_y));
			v.push_back(std::to_string(e.caston_z));
			v.push_back(std::to_string(e.ExtraDIChance));

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

	static std::vector<MercBuffs> All(Database& db)
	{
		std::vector<MercBuffs> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MercBuffs e{};

			e.MercBuffId         = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.MercId             = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.SpellId            = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.CasterLevel        = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.DurationFormula    = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.TicsRemaining      = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.PoisonCounters     = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.DiseaseCounters    = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.CurseCounters      = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.CorruptionCounters = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.HitCount           = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.MeleeRune          = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.MagicRune          = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.dot_rune           = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.caston_x           = row[14] ? static_cast<int32_t>(atoi(row[14])) : 0;
			e.Persistent         = row[15] ? static_cast<int8_t>(atoi(row[15])) : 0;
			e.caston_y           = row[16] ? static_cast<int32_t>(atoi(row[16])) : 0;
			e.caston_z           = row[17] ? static_cast<int32_t>(atoi(row[17])) : 0;
			e.ExtraDIChance      = row[18] ? static_cast<int32_t>(atoi(row[18])) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<MercBuffs> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<MercBuffs> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MercBuffs e{};

			e.MercBuffId         = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.MercId             = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.SpellId            = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.CasterLevel        = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.DurationFormula    = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.TicsRemaining      = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.PoisonCounters     = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.DiseaseCounters    = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.CurseCounters      = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.CorruptionCounters = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.HitCount           = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.MeleeRune          = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.MagicRune          = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.dot_rune           = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.caston_x           = row[14] ? static_cast<int32_t>(atoi(row[14])) : 0;
			e.Persistent         = row[15] ? static_cast<int8_t>(atoi(row[15])) : 0;
			e.caston_y           = row[16] ? static_cast<int32_t>(atoi(row[16])) : 0;
			e.caston_z           = row[17] ? static_cast<int32_t>(atoi(row[17])) : 0;
			e.ExtraDIChance      = row[18] ? static_cast<int32_t>(atoi(row[18])) : 0;

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
		const MercBuffs &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.MercBuffId));
		v.push_back(std::to_string(e.MercId));
		v.push_back(std::to_string(e.SpellId));
		v.push_back(std::to_string(e.CasterLevel));
		v.push_back(std::to_string(e.DurationFormula));
		v.push_back(std::to_string(e.TicsRemaining));
		v.push_back(std::to_string(e.PoisonCounters));
		v.push_back(std::to_string(e.DiseaseCounters));
		v.push_back(std::to_string(e.CurseCounters));
		v.push_back(std::to_string(e.CorruptionCounters));
		v.push_back(std::to_string(e.HitCount));
		v.push_back(std::to_string(e.MeleeRune));
		v.push_back(std::to_string(e.MagicRune));
		v.push_back(std::to_string(e.dot_rune));
		v.push_back(std::to_string(e.caston_x));
		v.push_back(std::to_string(e.Persistent));
		v.push_back(std::to_string(e.caston_y));
		v.push_back(std::to_string(e.caston_z));
		v.push_back(std::to_string(e.ExtraDIChance));

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
		const std::vector<MercBuffs> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.MercBuffId));
			v.push_back(std::to_string(e.MercId));
			v.push_back(std::to_string(e.SpellId));
			v.push_back(std::to_string(e.CasterLevel));
			v.push_back(std::to_string(e.DurationFormula));
			v.push_back(std::to_string(e.TicsRemaining));
			v.push_back(std::to_string(e.PoisonCounters));
			v.push_back(std::to_string(e.DiseaseCounters));
			v.push_back(std::to_string(e.CurseCounters));
			v.push_back(std::to_string(e.CorruptionCounters));
			v.push_back(std::to_string(e.HitCount));
			v.push_back(std::to_string(e.MeleeRune));
			v.push_back(std::to_string(e.MagicRune));
			v.push_back(std::to_string(e.dot_rune));
			v.push_back(std::to_string(e.caston_x));
			v.push_back(std::to_string(e.Persistent));
			v.push_back(std::to_string(e.caston_y));
			v.push_back(std::to_string(e.caston_z));
			v.push_back(std::to_string(e.ExtraDIChance));

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
