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

class BaseNpcSpellsRepository {
public:
	struct NpcSpells {
		uint32_t    id;
		std::string name;
		uint32_t    parent_list;
		int16_t     attack_proc;
		int8_t      proc_chance;
		int16_t     range_proc;
		int16_t     rproc_chance;
		int16_t     defensive_proc;
		int16_t     dproc_chance;
		uint32_t    fail_recast;
		uint32_t    engaged_no_sp_recast_min;
		uint32_t    engaged_no_sp_recast_max;
		uint8_t     engaged_b_self_chance;
		uint8_t     engaged_b_other_chance;
		uint8_t     engaged_d_chance;
		uint32_t    pursue_no_sp_recast_min;
		uint32_t    pursue_no_sp_recast_max;
		uint8_t     pursue_d_chance;
		uint32_t    idle_no_sp_recast_min;
		uint32_t    idle_no_sp_recast_max;
		uint8_t     idle_b_chance;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"name",
			"parent_list",
			"attack_proc",
			"proc_chance",
			"range_proc",
			"rproc_chance",
			"defensive_proc",
			"dproc_chance",
			"fail_recast",
			"engaged_no_sp_recast_min",
			"engaged_no_sp_recast_max",
			"engaged_b_self_chance",
			"engaged_b_other_chance",
			"engaged_d_chance",
			"pursue_no_sp_recast_min",
			"pursue_no_sp_recast_max",
			"pursue_d_chance",
			"idle_no_sp_recast_min",
			"idle_no_sp_recast_max",
			"idle_b_chance",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"name",
			"parent_list",
			"attack_proc",
			"proc_chance",
			"range_proc",
			"rproc_chance",
			"defensive_proc",
			"dproc_chance",
			"fail_recast",
			"engaged_no_sp_recast_min",
			"engaged_no_sp_recast_max",
			"engaged_b_self_chance",
			"engaged_b_other_chance",
			"engaged_d_chance",
			"pursue_no_sp_recast_min",
			"pursue_no_sp_recast_max",
			"pursue_d_chance",
			"idle_no_sp_recast_min",
			"idle_no_sp_recast_max",
			"idle_b_chance",
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
		return std::string("npc_spells");
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

	static NpcSpells NewEntity()
	{
		NpcSpells e{};

		e.id                       = 0;
		e.name                     = "";
		e.parent_list              = 0;
		e.attack_proc              = -1;
		e.proc_chance              = 3;
		e.range_proc               = -1;
		e.rproc_chance             = 0;
		e.defensive_proc           = -1;
		e.dproc_chance             = 0;
		e.fail_recast              = 0;
		e.engaged_no_sp_recast_min = 0;
		e.engaged_no_sp_recast_max = 0;
		e.engaged_b_self_chance    = 0;
		e.engaged_b_other_chance   = 0;
		e.engaged_d_chance         = 0;
		e.pursue_no_sp_recast_min  = 0;
		e.pursue_no_sp_recast_max  = 0;
		e.pursue_d_chance          = 0;
		e.idle_no_sp_recast_min    = 0;
		e.idle_no_sp_recast_max    = 0;
		e.idle_b_chance            = 0;

		return e;
	}

	static NpcSpells GetNpcSpells(
		const std::vector<NpcSpells> &npc_spellss,
		int npc_spells_id
	)
	{
		for (auto &npc_spells : npc_spellss) {
			if (npc_spells.id == npc_spells_id) {
				return npc_spells;
			}
		}

		return NewEntity();
	}

	static NpcSpells FindOne(
		Database& db,
		int npc_spells_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				npc_spells_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			NpcSpells e{};

			e.id                       = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.name                     = row[1] ? row[1] : "";
			e.parent_list              = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.attack_proc              = row[3] ? static_cast<int16_t>(atoi(row[3])) : -1;
			e.proc_chance              = row[4] ? static_cast<int8_t>(atoi(row[4])) : 3;
			e.range_proc               = row[5] ? static_cast<int16_t>(atoi(row[5])) : -1;
			e.rproc_chance             = row[6] ? static_cast<int16_t>(atoi(row[6])) : 0;
			e.defensive_proc           = row[7] ? static_cast<int16_t>(atoi(row[7])) : -1;
			e.dproc_chance             = row[8] ? static_cast<int16_t>(atoi(row[8])) : 0;
			e.fail_recast              = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.engaged_no_sp_recast_min = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.engaged_no_sp_recast_max = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.engaged_b_self_chance    = row[12] ? static_cast<uint8_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.engaged_b_other_chance   = row[13] ? static_cast<uint8_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.engaged_d_chance         = row[14] ? static_cast<uint8_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.pursue_no_sp_recast_min  = row[15] ? static_cast<uint32_t>(strtoul(row[15], nullptr, 10)) : 0;
			e.pursue_no_sp_recast_max  = row[16] ? static_cast<uint32_t>(strtoul(row[16], nullptr, 10)) : 0;
			e.pursue_d_chance          = row[17] ? static_cast<uint8_t>(strtoul(row[17], nullptr, 10)) : 0;
			e.idle_no_sp_recast_min    = row[18] ? static_cast<uint32_t>(strtoul(row[18], nullptr, 10)) : 0;
			e.idle_no_sp_recast_max    = row[19] ? static_cast<uint32_t>(strtoul(row[19], nullptr, 10)) : 0;
			e.idle_b_chance            = row[20] ? static_cast<uint8_t>(strtoul(row[20], nullptr, 10)) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int npc_spells_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				npc_spells_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const NpcSpells &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[2] + " = " + std::to_string(e.parent_list));
		v.push_back(columns[3] + " = " + std::to_string(e.attack_proc));
		v.push_back(columns[4] + " = " + std::to_string(e.proc_chance));
		v.push_back(columns[5] + " = " + std::to_string(e.range_proc));
		v.push_back(columns[6] + " = " + std::to_string(e.rproc_chance));
		v.push_back(columns[7] + " = " + std::to_string(e.defensive_proc));
		v.push_back(columns[8] + " = " + std::to_string(e.dproc_chance));
		v.push_back(columns[9] + " = " + std::to_string(e.fail_recast));
		v.push_back(columns[10] + " = " + std::to_string(e.engaged_no_sp_recast_min));
		v.push_back(columns[11] + " = " + std::to_string(e.engaged_no_sp_recast_max));
		v.push_back(columns[12] + " = " + std::to_string(e.engaged_b_self_chance));
		v.push_back(columns[13] + " = " + std::to_string(e.engaged_b_other_chance));
		v.push_back(columns[14] + " = " + std::to_string(e.engaged_d_chance));
		v.push_back(columns[15] + " = " + std::to_string(e.pursue_no_sp_recast_min));
		v.push_back(columns[16] + " = " + std::to_string(e.pursue_no_sp_recast_max));
		v.push_back(columns[17] + " = " + std::to_string(e.pursue_d_chance));
		v.push_back(columns[18] + " = " + std::to_string(e.idle_no_sp_recast_min));
		v.push_back(columns[19] + " = " + std::to_string(e.idle_no_sp_recast_max));
		v.push_back(columns[20] + " = " + std::to_string(e.idle_b_chance));

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

	static NpcSpells InsertOne(
		Database& db,
		NpcSpells e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.parent_list));
		v.push_back(std::to_string(e.attack_proc));
		v.push_back(std::to_string(e.proc_chance));
		v.push_back(std::to_string(e.range_proc));
		v.push_back(std::to_string(e.rproc_chance));
		v.push_back(std::to_string(e.defensive_proc));
		v.push_back(std::to_string(e.dproc_chance));
		v.push_back(std::to_string(e.fail_recast));
		v.push_back(std::to_string(e.engaged_no_sp_recast_min));
		v.push_back(std::to_string(e.engaged_no_sp_recast_max));
		v.push_back(std::to_string(e.engaged_b_self_chance));
		v.push_back(std::to_string(e.engaged_b_other_chance));
		v.push_back(std::to_string(e.engaged_d_chance));
		v.push_back(std::to_string(e.pursue_no_sp_recast_min));
		v.push_back(std::to_string(e.pursue_no_sp_recast_max));
		v.push_back(std::to_string(e.pursue_d_chance));
		v.push_back(std::to_string(e.idle_no_sp_recast_min));
		v.push_back(std::to_string(e.idle_no_sp_recast_max));
		v.push_back(std::to_string(e.idle_b_chance));

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
		const std::vector<NpcSpells> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.parent_list));
			v.push_back(std::to_string(e.attack_proc));
			v.push_back(std::to_string(e.proc_chance));
			v.push_back(std::to_string(e.range_proc));
			v.push_back(std::to_string(e.rproc_chance));
			v.push_back(std::to_string(e.defensive_proc));
			v.push_back(std::to_string(e.dproc_chance));
			v.push_back(std::to_string(e.fail_recast));
			v.push_back(std::to_string(e.engaged_no_sp_recast_min));
			v.push_back(std::to_string(e.engaged_no_sp_recast_max));
			v.push_back(std::to_string(e.engaged_b_self_chance));
			v.push_back(std::to_string(e.engaged_b_other_chance));
			v.push_back(std::to_string(e.engaged_d_chance));
			v.push_back(std::to_string(e.pursue_no_sp_recast_min));
			v.push_back(std::to_string(e.pursue_no_sp_recast_max));
			v.push_back(std::to_string(e.pursue_d_chance));
			v.push_back(std::to_string(e.idle_no_sp_recast_min));
			v.push_back(std::to_string(e.idle_no_sp_recast_max));
			v.push_back(std::to_string(e.idle_b_chance));

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

	static std::vector<NpcSpells> All(Database& db)
	{
		std::vector<NpcSpells> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcSpells e{};

			e.id                       = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.name                     = row[1] ? row[1] : "";
			e.parent_list              = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.attack_proc              = row[3] ? static_cast<int16_t>(atoi(row[3])) : -1;
			e.proc_chance              = row[4] ? static_cast<int8_t>(atoi(row[4])) : 3;
			e.range_proc               = row[5] ? static_cast<int16_t>(atoi(row[5])) : -1;
			e.rproc_chance             = row[6] ? static_cast<int16_t>(atoi(row[6])) : 0;
			e.defensive_proc           = row[7] ? static_cast<int16_t>(atoi(row[7])) : -1;
			e.dproc_chance             = row[8] ? static_cast<int16_t>(atoi(row[8])) : 0;
			e.fail_recast              = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.engaged_no_sp_recast_min = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.engaged_no_sp_recast_max = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.engaged_b_self_chance    = row[12] ? static_cast<uint8_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.engaged_b_other_chance   = row[13] ? static_cast<uint8_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.engaged_d_chance         = row[14] ? static_cast<uint8_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.pursue_no_sp_recast_min  = row[15] ? static_cast<uint32_t>(strtoul(row[15], nullptr, 10)) : 0;
			e.pursue_no_sp_recast_max  = row[16] ? static_cast<uint32_t>(strtoul(row[16], nullptr, 10)) : 0;
			e.pursue_d_chance          = row[17] ? static_cast<uint8_t>(strtoul(row[17], nullptr, 10)) : 0;
			e.idle_no_sp_recast_min    = row[18] ? static_cast<uint32_t>(strtoul(row[18], nullptr, 10)) : 0;
			e.idle_no_sp_recast_max    = row[19] ? static_cast<uint32_t>(strtoul(row[19], nullptr, 10)) : 0;
			e.idle_b_chance            = row[20] ? static_cast<uint8_t>(strtoul(row[20], nullptr, 10)) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<NpcSpells> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<NpcSpells> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcSpells e{};

			e.id                       = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.name                     = row[1] ? row[1] : "";
			e.parent_list              = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.attack_proc              = row[3] ? static_cast<int16_t>(atoi(row[3])) : -1;
			e.proc_chance              = row[4] ? static_cast<int8_t>(atoi(row[4])) : 3;
			e.range_proc               = row[5] ? static_cast<int16_t>(atoi(row[5])) : -1;
			e.rproc_chance             = row[6] ? static_cast<int16_t>(atoi(row[6])) : 0;
			e.defensive_proc           = row[7] ? static_cast<int16_t>(atoi(row[7])) : -1;
			e.dproc_chance             = row[8] ? static_cast<int16_t>(atoi(row[8])) : 0;
			e.fail_recast              = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.engaged_no_sp_recast_min = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.engaged_no_sp_recast_max = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.engaged_b_self_chance    = row[12] ? static_cast<uint8_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.engaged_b_other_chance   = row[13] ? static_cast<uint8_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.engaged_d_chance         = row[14] ? static_cast<uint8_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.pursue_no_sp_recast_min  = row[15] ? static_cast<uint32_t>(strtoul(row[15], nullptr, 10)) : 0;
			e.pursue_no_sp_recast_max  = row[16] ? static_cast<uint32_t>(strtoul(row[16], nullptr, 10)) : 0;
			e.pursue_d_chance          = row[17] ? static_cast<uint8_t>(strtoul(row[17], nullptr, 10)) : 0;
			e.idle_no_sp_recast_min    = row[18] ? static_cast<uint32_t>(strtoul(row[18], nullptr, 10)) : 0;
			e.idle_no_sp_recast_max    = row[19] ? static_cast<uint32_t>(strtoul(row[19], nullptr, 10)) : 0;
			e.idle_b_chance            = row[20] ? static_cast<uint8_t>(strtoul(row[20], nullptr, 10)) : 0;

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
		const NpcSpells &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.parent_list));
		v.push_back(std::to_string(e.attack_proc));
		v.push_back(std::to_string(e.proc_chance));
		v.push_back(std::to_string(e.range_proc));
		v.push_back(std::to_string(e.rproc_chance));
		v.push_back(std::to_string(e.defensive_proc));
		v.push_back(std::to_string(e.dproc_chance));
		v.push_back(std::to_string(e.fail_recast));
		v.push_back(std::to_string(e.engaged_no_sp_recast_min));
		v.push_back(std::to_string(e.engaged_no_sp_recast_max));
		v.push_back(std::to_string(e.engaged_b_self_chance));
		v.push_back(std::to_string(e.engaged_b_other_chance));
		v.push_back(std::to_string(e.engaged_d_chance));
		v.push_back(std::to_string(e.pursue_no_sp_recast_min));
		v.push_back(std::to_string(e.pursue_no_sp_recast_max));
		v.push_back(std::to_string(e.pursue_d_chance));
		v.push_back(std::to_string(e.idle_no_sp_recast_min));
		v.push_back(std::to_string(e.idle_no_sp_recast_max));
		v.push_back(std::to_string(e.idle_b_chance));

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
		const std::vector<NpcSpells> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.parent_list));
			v.push_back(std::to_string(e.attack_proc));
			v.push_back(std::to_string(e.proc_chance));
			v.push_back(std::to_string(e.range_proc));
			v.push_back(std::to_string(e.rproc_chance));
			v.push_back(std::to_string(e.defensive_proc));
			v.push_back(std::to_string(e.dproc_chance));
			v.push_back(std::to_string(e.fail_recast));
			v.push_back(std::to_string(e.engaged_no_sp_recast_min));
			v.push_back(std::to_string(e.engaged_no_sp_recast_max));
			v.push_back(std::to_string(e.engaged_b_self_chance));
			v.push_back(std::to_string(e.engaged_b_other_chance));
			v.push_back(std::to_string(e.engaged_d_chance));
			v.push_back(std::to_string(e.pursue_no_sp_recast_min));
			v.push_back(std::to_string(e.pursue_no_sp_recast_max));
			v.push_back(std::to_string(e.pursue_d_chance));
			v.push_back(std::to_string(e.idle_no_sp_recast_min));
			v.push_back(std::to_string(e.idle_no_sp_recast_max));
			v.push_back(std::to_string(e.idle_b_chance));

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
