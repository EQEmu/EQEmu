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


class BaseMercStatsRepository {
public:
	struct MercStats {
		uint32_t    merc_npc_type_id;
		uint8_t     clientlevel;
		uint8_t     level;
		int32_t     hp;
		int32_t     mana;
		int16_t     AC;
		int32_t     ATK;
		uint32_t    STR;
		uint32_t    STA;
		uint32_t    DEX;
		uint32_t    AGI;
		uint32_t    _INT;
		uint32_t    WIS;
		uint32_t    CHA;
		int16_t     MR;
		int16_t     CR;
		int16_t     DR;
		int16_t     FR;
		int16_t     PR;
		int16_t     Corrup;
		uint32_t    mindmg;
		uint32_t    maxdmg;
		int16_t     attack_count;
		int8_t      attack_speed;
		uint8_t     attack_delay;
		std::string special_abilities;
		int32_t     Accuracy;
		uint32_t    hp_regen_rate;
		uint32_t    mana_regen_rate;
		float       runspeed;
		int32_t     statscale;
		float       spellscale;
		float       healscale;
	};

	static std::string PrimaryKey()
	{
		return std::string("merc_npc_type_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"merc_npc_type_id",
			"clientlevel",
			"level",
			"hp",
			"mana",
			"AC",
			"ATK",
			"STR",
			"STA",
			"DEX",
			"AGI",
			"_INT",
			"WIS",
			"CHA",
			"MR",
			"CR",
			"DR",
			"FR",
			"PR",
			"Corrup",
			"mindmg",
			"maxdmg",
			"attack_count",
			"attack_speed",
			"attack_delay",
			"special_abilities",
			"Accuracy",
			"hp_regen_rate",
			"mana_regen_rate",
			"runspeed",
			"statscale",
			"spellscale",
			"healscale",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"merc_npc_type_id",
			"clientlevel",
			"level",
			"hp",
			"mana",
			"AC",
			"ATK",
			"STR",
			"STA",
			"DEX",
			"AGI",
			"_INT",
			"WIS",
			"CHA",
			"MR",
			"CR",
			"DR",
			"FR",
			"PR",
			"Corrup",
			"mindmg",
			"maxdmg",
			"attack_count",
			"attack_speed",
			"attack_delay",
			"special_abilities",
			"Accuracy",
			"hp_regen_rate",
			"mana_regen_rate",
			"runspeed",
			"statscale",
			"spellscale",
			"healscale",
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
		return std::string("merc_stats");
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

	static MercStats NewEntity()
	{
		MercStats e{};

		e.merc_npc_type_id  = 0;
		e.clientlevel       = 1;
		e.level             = 1;
		e.hp                = 1;
		e.mana              = 0;
		e.AC                = 1;
		e.ATK               = 1;
		e.STR               = 75;
		e.STA               = 75;
		e.DEX               = 75;
		e.AGI               = 75;
		e._INT              = 80;
		e.WIS               = 80;
		e.CHA               = 75;
		e.MR                = 15;
		e.CR                = 15;
		e.DR                = 15;
		e.FR                = 15;
		e.PR                = 15;
		e.Corrup            = 15;
		e.mindmg            = 1;
		e.maxdmg            = 1;
		e.attack_count      = 0;
		e.attack_speed      = 0;
		e.attack_delay      = 30;
		e.special_abilities = "";
		e.Accuracy          = 0;
		e.hp_regen_rate     = 1;
		e.mana_regen_rate   = 1;
		e.runspeed          = 0;
		e.statscale         = 100;
		e.spellscale        = 100;
		e.healscale         = 100;

		return e;
	}

	static MercStats GetMercStats(
		const std::vector<MercStats> &merc_statss,
		int merc_stats_id
	)
	{
		for (auto &merc_stats : merc_statss) {
			if (merc_stats.merc_npc_type_id == merc_stats_id) {
				return merc_stats;
			}
		}

		return NewEntity();
	}

	static MercStats FindOne(
		Database& db,
		int merc_stats_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				merc_stats_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			MercStats e{};

			e.merc_npc_type_id  = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.clientlevel       = static_cast<uint8_t>(strtoul(row[1], nullptr, 10));
			e.level             = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.hp                = static_cast<int32_t>(atoi(row[3]));
			e.mana              = static_cast<int32_t>(atoi(row[4]));
			e.AC                = static_cast<int16_t>(atoi(row[5]));
			e.ATK               = static_cast<int32_t>(atoi(row[6]));
			e.STR               = static_cast<uint32_t>(strtoul(row[7], nullptr, 10));
			e.STA               = static_cast<uint32_t>(strtoul(row[8], nullptr, 10));
			e.DEX               = static_cast<uint32_t>(strtoul(row[9], nullptr, 10));
			e.AGI               = static_cast<uint32_t>(strtoul(row[10], nullptr, 10));
			e._INT              = static_cast<uint32_t>(strtoul(row[11], nullptr, 10));
			e.WIS               = static_cast<uint32_t>(strtoul(row[12], nullptr, 10));
			e.CHA               = static_cast<uint32_t>(strtoul(row[13], nullptr, 10));
			e.MR                = static_cast<int16_t>(atoi(row[14]));
			e.CR                = static_cast<int16_t>(atoi(row[15]));
			e.DR                = static_cast<int16_t>(atoi(row[16]));
			e.FR                = static_cast<int16_t>(atoi(row[17]));
			e.PR                = static_cast<int16_t>(atoi(row[18]));
			e.Corrup            = static_cast<int16_t>(atoi(row[19]));
			e.mindmg            = static_cast<uint32_t>(strtoul(row[20], nullptr, 10));
			e.maxdmg            = static_cast<uint32_t>(strtoul(row[21], nullptr, 10));
			e.attack_count      = static_cast<int16_t>(atoi(row[22]));
			e.attack_speed      = static_cast<int8_t>(atoi(row[23]));
			e.attack_delay      = static_cast<uint8_t>(strtoul(row[24], nullptr, 10));
			e.special_abilities = row[25] ? row[25] : "";
			e.Accuracy          = static_cast<int32_t>(atoi(row[26]));
			e.hp_regen_rate     = static_cast<uint32_t>(strtoul(row[27], nullptr, 10));
			e.mana_regen_rate   = static_cast<uint32_t>(strtoul(row[28], nullptr, 10));
			e.runspeed          = strtof(row[29], nullptr);
			e.statscale         = static_cast<int32_t>(atoi(row[30]));
			e.spellscale        = strtof(row[31], nullptr);
			e.healscale         = strtof(row[32], nullptr);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int merc_stats_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				merc_stats_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const MercStats &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.merc_npc_type_id));
		v.push_back(columns[1] + " = " + std::to_string(e.clientlevel));
		v.push_back(columns[2] + " = " + std::to_string(e.level));
		v.push_back(columns[3] + " = " + std::to_string(e.hp));
		v.push_back(columns[4] + " = " + std::to_string(e.mana));
		v.push_back(columns[5] + " = " + std::to_string(e.AC));
		v.push_back(columns[6] + " = " + std::to_string(e.ATK));
		v.push_back(columns[7] + " = " + std::to_string(e.STR));
		v.push_back(columns[8] + " = " + std::to_string(e.STA));
		v.push_back(columns[9] + " = " + std::to_string(e.DEX));
		v.push_back(columns[10] + " = " + std::to_string(e.AGI));
		v.push_back(columns[11] + " = " + std::to_string(e._INT));
		v.push_back(columns[12] + " = " + std::to_string(e.WIS));
		v.push_back(columns[13] + " = " + std::to_string(e.CHA));
		v.push_back(columns[14] + " = " + std::to_string(e.MR));
		v.push_back(columns[15] + " = " + std::to_string(e.CR));
		v.push_back(columns[16] + " = " + std::to_string(e.DR));
		v.push_back(columns[17] + " = " + std::to_string(e.FR));
		v.push_back(columns[18] + " = " + std::to_string(e.PR));
		v.push_back(columns[19] + " = " + std::to_string(e.Corrup));
		v.push_back(columns[20] + " = " + std::to_string(e.mindmg));
		v.push_back(columns[21] + " = " + std::to_string(e.maxdmg));
		v.push_back(columns[22] + " = " + std::to_string(e.attack_count));
		v.push_back(columns[23] + " = " + std::to_string(e.attack_speed));
		v.push_back(columns[24] + " = " + std::to_string(e.attack_delay));
		v.push_back(columns[25] + " = '" + Strings::Escape(e.special_abilities) + "'");
		v.push_back(columns[26] + " = " + std::to_string(e.Accuracy));
		v.push_back(columns[27] + " = " + std::to_string(e.hp_regen_rate));
		v.push_back(columns[28] + " = " + std::to_string(e.mana_regen_rate));
		v.push_back(columns[29] + " = " + std::to_string(e.runspeed));
		v.push_back(columns[30] + " = " + std::to_string(e.statscale));
		v.push_back(columns[31] + " = " + std::to_string(e.spellscale));
		v.push_back(columns[32] + " = " + std::to_string(e.healscale));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.merc_npc_type_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static MercStats InsertOne(
		Database& db,
		MercStats e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.merc_npc_type_id));
		v.push_back(std::to_string(e.clientlevel));
		v.push_back(std::to_string(e.level));
		v.push_back(std::to_string(e.hp));
		v.push_back(std::to_string(e.mana));
		v.push_back(std::to_string(e.AC));
		v.push_back(std::to_string(e.ATK));
		v.push_back(std::to_string(e.STR));
		v.push_back(std::to_string(e.STA));
		v.push_back(std::to_string(e.DEX));
		v.push_back(std::to_string(e.AGI));
		v.push_back(std::to_string(e._INT));
		v.push_back(std::to_string(e.WIS));
		v.push_back(std::to_string(e.CHA));
		v.push_back(std::to_string(e.MR));
		v.push_back(std::to_string(e.CR));
		v.push_back(std::to_string(e.DR));
		v.push_back(std::to_string(e.FR));
		v.push_back(std::to_string(e.PR));
		v.push_back(std::to_string(e.Corrup));
		v.push_back(std::to_string(e.mindmg));
		v.push_back(std::to_string(e.maxdmg));
		v.push_back(std::to_string(e.attack_count));
		v.push_back(std::to_string(e.attack_speed));
		v.push_back(std::to_string(e.attack_delay));
		v.push_back("'" + Strings::Escape(e.special_abilities) + "'");
		v.push_back(std::to_string(e.Accuracy));
		v.push_back(std::to_string(e.hp_regen_rate));
		v.push_back(std::to_string(e.mana_regen_rate));
		v.push_back(std::to_string(e.runspeed));
		v.push_back(std::to_string(e.statscale));
		v.push_back(std::to_string(e.spellscale));
		v.push_back(std::to_string(e.healscale));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.merc_npc_type_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<MercStats> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.merc_npc_type_id));
			v.push_back(std::to_string(e.clientlevel));
			v.push_back(std::to_string(e.level));
			v.push_back(std::to_string(e.hp));
			v.push_back(std::to_string(e.mana));
			v.push_back(std::to_string(e.AC));
			v.push_back(std::to_string(e.ATK));
			v.push_back(std::to_string(e.STR));
			v.push_back(std::to_string(e.STA));
			v.push_back(std::to_string(e.DEX));
			v.push_back(std::to_string(e.AGI));
			v.push_back(std::to_string(e._INT));
			v.push_back(std::to_string(e.WIS));
			v.push_back(std::to_string(e.CHA));
			v.push_back(std::to_string(e.MR));
			v.push_back(std::to_string(e.CR));
			v.push_back(std::to_string(e.DR));
			v.push_back(std::to_string(e.FR));
			v.push_back(std::to_string(e.PR));
			v.push_back(std::to_string(e.Corrup));
			v.push_back(std::to_string(e.mindmg));
			v.push_back(std::to_string(e.maxdmg));
			v.push_back(std::to_string(e.attack_count));
			v.push_back(std::to_string(e.attack_speed));
			v.push_back(std::to_string(e.attack_delay));
			v.push_back("'" + Strings::Escape(e.special_abilities) + "'");
			v.push_back(std::to_string(e.Accuracy));
			v.push_back(std::to_string(e.hp_regen_rate));
			v.push_back(std::to_string(e.mana_regen_rate));
			v.push_back(std::to_string(e.runspeed));
			v.push_back(std::to_string(e.statscale));
			v.push_back(std::to_string(e.spellscale));
			v.push_back(std::to_string(e.healscale));

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

	static std::vector<MercStats> All(Database& db)
	{
		std::vector<MercStats> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MercStats e{};

			e.merc_npc_type_id  = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.clientlevel       = static_cast<uint8_t>(strtoul(row[1], nullptr, 10));
			e.level             = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.hp                = static_cast<int32_t>(atoi(row[3]));
			e.mana              = static_cast<int32_t>(atoi(row[4]));
			e.AC                = static_cast<int16_t>(atoi(row[5]));
			e.ATK               = static_cast<int32_t>(atoi(row[6]));
			e.STR               = static_cast<uint32_t>(strtoul(row[7], nullptr, 10));
			e.STA               = static_cast<uint32_t>(strtoul(row[8], nullptr, 10));
			e.DEX               = static_cast<uint32_t>(strtoul(row[9], nullptr, 10));
			e.AGI               = static_cast<uint32_t>(strtoul(row[10], nullptr, 10));
			e._INT              = static_cast<uint32_t>(strtoul(row[11], nullptr, 10));
			e.WIS               = static_cast<uint32_t>(strtoul(row[12], nullptr, 10));
			e.CHA               = static_cast<uint32_t>(strtoul(row[13], nullptr, 10));
			e.MR                = static_cast<int16_t>(atoi(row[14]));
			e.CR                = static_cast<int16_t>(atoi(row[15]));
			e.DR                = static_cast<int16_t>(atoi(row[16]));
			e.FR                = static_cast<int16_t>(atoi(row[17]));
			e.PR                = static_cast<int16_t>(atoi(row[18]));
			e.Corrup            = static_cast<int16_t>(atoi(row[19]));
			e.mindmg            = static_cast<uint32_t>(strtoul(row[20], nullptr, 10));
			e.maxdmg            = static_cast<uint32_t>(strtoul(row[21], nullptr, 10));
			e.attack_count      = static_cast<int16_t>(atoi(row[22]));
			e.attack_speed      = static_cast<int8_t>(atoi(row[23]));
			e.attack_delay      = static_cast<uint8_t>(strtoul(row[24], nullptr, 10));
			e.special_abilities = row[25] ? row[25] : "";
			e.Accuracy          = static_cast<int32_t>(atoi(row[26]));
			e.hp_regen_rate     = static_cast<uint32_t>(strtoul(row[27], nullptr, 10));
			e.mana_regen_rate   = static_cast<uint32_t>(strtoul(row[28], nullptr, 10));
			e.runspeed          = strtof(row[29], nullptr);
			e.statscale         = static_cast<int32_t>(atoi(row[30]));
			e.spellscale        = strtof(row[31], nullptr);
			e.healscale         = strtof(row[32], nullptr);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<MercStats> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<MercStats> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MercStats e{};

			e.merc_npc_type_id  = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.clientlevel       = static_cast<uint8_t>(strtoul(row[1], nullptr, 10));
			e.level             = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.hp                = static_cast<int32_t>(atoi(row[3]));
			e.mana              = static_cast<int32_t>(atoi(row[4]));
			e.AC                = static_cast<int16_t>(atoi(row[5]));
			e.ATK               = static_cast<int32_t>(atoi(row[6]));
			e.STR               = static_cast<uint32_t>(strtoul(row[7], nullptr, 10));
			e.STA               = static_cast<uint32_t>(strtoul(row[8], nullptr, 10));
			e.DEX               = static_cast<uint32_t>(strtoul(row[9], nullptr, 10));
			e.AGI               = static_cast<uint32_t>(strtoul(row[10], nullptr, 10));
			e._INT              = static_cast<uint32_t>(strtoul(row[11], nullptr, 10));
			e.WIS               = static_cast<uint32_t>(strtoul(row[12], nullptr, 10));
			e.CHA               = static_cast<uint32_t>(strtoul(row[13], nullptr, 10));
			e.MR                = static_cast<int16_t>(atoi(row[14]));
			e.CR                = static_cast<int16_t>(atoi(row[15]));
			e.DR                = static_cast<int16_t>(atoi(row[16]));
			e.FR                = static_cast<int16_t>(atoi(row[17]));
			e.PR                = static_cast<int16_t>(atoi(row[18]));
			e.Corrup            = static_cast<int16_t>(atoi(row[19]));
			e.mindmg            = static_cast<uint32_t>(strtoul(row[20], nullptr, 10));
			e.maxdmg            = static_cast<uint32_t>(strtoul(row[21], nullptr, 10));
			e.attack_count      = static_cast<int16_t>(atoi(row[22]));
			e.attack_speed      = static_cast<int8_t>(atoi(row[23]));
			e.attack_delay      = static_cast<uint8_t>(strtoul(row[24], nullptr, 10));
			e.special_abilities = row[25] ? row[25] : "";
			e.Accuracy          = static_cast<int32_t>(atoi(row[26]));
			e.hp_regen_rate     = static_cast<uint32_t>(strtoul(row[27], nullptr, 10));
			e.mana_regen_rate   = static_cast<uint32_t>(strtoul(row[28], nullptr, 10));
			e.runspeed          = strtof(row[29], nullptr);
			e.statscale         = static_cast<int32_t>(atoi(row[30]));
			e.spellscale        = strtof(row[31], nullptr);
			e.healscale         = strtof(row[32], nullptr);

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
		const MercStats &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.merc_npc_type_id));
		v.push_back(std::to_string(e.clientlevel));
		v.push_back(std::to_string(e.level));
		v.push_back(std::to_string(e.hp));
		v.push_back(std::to_string(e.mana));
		v.push_back(std::to_string(e.AC));
		v.push_back(std::to_string(e.ATK));
		v.push_back(std::to_string(e.STR));
		v.push_back(std::to_string(e.STA));
		v.push_back(std::to_string(e.DEX));
		v.push_back(std::to_string(e.AGI));
		v.push_back(std::to_string(e._INT));
		v.push_back(std::to_string(e.WIS));
		v.push_back(std::to_string(e.CHA));
		v.push_back(std::to_string(e.MR));
		v.push_back(std::to_string(e.CR));
		v.push_back(std::to_string(e.DR));
		v.push_back(std::to_string(e.FR));
		v.push_back(std::to_string(e.PR));
		v.push_back(std::to_string(e.Corrup));
		v.push_back(std::to_string(e.mindmg));
		v.push_back(std::to_string(e.maxdmg));
		v.push_back(std::to_string(e.attack_count));
		v.push_back(std::to_string(e.attack_speed));
		v.push_back(std::to_string(e.attack_delay));
		v.push_back("'" + Strings::Escape(e.special_abilities) + "'");
		v.push_back(std::to_string(e.Accuracy));
		v.push_back(std::to_string(e.hp_regen_rate));
		v.push_back(std::to_string(e.mana_regen_rate));
		v.push_back(std::to_string(e.runspeed));
		v.push_back(std::to_string(e.statscale));
		v.push_back(std::to_string(e.spellscale));
		v.push_back(std::to_string(e.healscale));

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
		const std::vector<MercStats> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.merc_npc_type_id));
			v.push_back(std::to_string(e.clientlevel));
			v.push_back(std::to_string(e.level));
			v.push_back(std::to_string(e.hp));
			v.push_back(std::to_string(e.mana));
			v.push_back(std::to_string(e.AC));
			v.push_back(std::to_string(e.ATK));
			v.push_back(std::to_string(e.STR));
			v.push_back(std::to_string(e.STA));
			v.push_back(std::to_string(e.DEX));
			v.push_back(std::to_string(e.AGI));
			v.push_back(std::to_string(e._INT));
			v.push_back(std::to_string(e.WIS));
			v.push_back(std::to_string(e.CHA));
			v.push_back(std::to_string(e.MR));
			v.push_back(std::to_string(e.CR));
			v.push_back(std::to_string(e.DR));
			v.push_back(std::to_string(e.FR));
			v.push_back(std::to_string(e.PR));
			v.push_back(std::to_string(e.Corrup));
			v.push_back(std::to_string(e.mindmg));
			v.push_back(std::to_string(e.maxdmg));
			v.push_back(std::to_string(e.attack_count));
			v.push_back(std::to_string(e.attack_speed));
			v.push_back(std::to_string(e.attack_delay));
			v.push_back("'" + Strings::Escape(e.special_abilities) + "'");
			v.push_back(std::to_string(e.Accuracy));
			v.push_back(std::to_string(e.hp_regen_rate));
			v.push_back(std::to_string(e.mana_regen_rate));
			v.push_back(std::to_string(e.runspeed));
			v.push_back(std::to_string(e.statscale));
			v.push_back(std::to_string(e.spellscale));
			v.push_back(std::to_string(e.healscale));

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
