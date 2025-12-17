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

class BaseNpcScaleGlobalBaseRepository {
public:
	struct NpcScaleGlobalBase {
		int32_t     type;
		int32_t     level;
		std::string zone_id_list;
		std::string instance_version_list;
		int32_t     ac;
		int64_t     hp;
		int32_t     accuracy;
		int32_t     slow_mitigation;
		int32_t     attack;
		int32_t     strength;
		int32_t     stamina;
		int32_t     dexterity;
		int32_t     agility;
		int32_t     intelligence;
		int32_t     wisdom;
		int32_t     charisma;
		int32_t     magic_resist;
		int32_t     cold_resist;
		int32_t     fire_resist;
		int32_t     poison_resist;
		int32_t     disease_resist;
		int32_t     corruption_resist;
		int32_t     physical_resist;
		int32_t     min_dmg;
		int32_t     max_dmg;
		int64_t     hp_regen_rate;
		int64_t     hp_regen_per_second;
		int32_t     attack_delay;
		int32_t     spell_scale;
		int32_t     heal_scale;
		uint32_t    avoidance;
		int32_t     heroic_strikethrough;
		std::string special_abilities;
	};

	static std::string PrimaryKey()
	{
		return std::string("type");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"type",
			"level",
			"zone_id_list",
			"instance_version_list",
			"ac",
			"hp",
			"accuracy",
			"slow_mitigation",
			"attack",
			"strength",
			"stamina",
			"dexterity",
			"agility",
			"intelligence",
			"wisdom",
			"charisma",
			"magic_resist",
			"cold_resist",
			"fire_resist",
			"poison_resist",
			"disease_resist",
			"corruption_resist",
			"physical_resist",
			"min_dmg",
			"max_dmg",
			"hp_regen_rate",
			"hp_regen_per_second",
			"attack_delay",
			"spell_scale",
			"heal_scale",
			"avoidance",
			"heroic_strikethrough",
			"special_abilities",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"type",
			"level",
			"zone_id_list",
			"instance_version_list",
			"ac",
			"hp",
			"accuracy",
			"slow_mitigation",
			"attack",
			"strength",
			"stamina",
			"dexterity",
			"agility",
			"intelligence",
			"wisdom",
			"charisma",
			"magic_resist",
			"cold_resist",
			"fire_resist",
			"poison_resist",
			"disease_resist",
			"corruption_resist",
			"physical_resist",
			"min_dmg",
			"max_dmg",
			"hp_regen_rate",
			"hp_regen_per_second",
			"attack_delay",
			"spell_scale",
			"heal_scale",
			"avoidance",
			"heroic_strikethrough",
			"special_abilities",
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
		return std::string("npc_scale_global_base");
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

	static NpcScaleGlobalBase NewEntity()
	{
		NpcScaleGlobalBase e{};

		e.type                  = 0;
		e.level                 = 0;
		e.zone_id_list          = "";
		e.instance_version_list = "";
		e.ac                    = 0;
		e.hp                    = 0;
		e.accuracy              = 0;
		e.slow_mitigation       = 0;
		e.attack                = 0;
		e.strength              = 0;
		e.stamina               = 0;
		e.dexterity             = 0;
		e.agility               = 0;
		e.intelligence          = 0;
		e.wisdom                = 0;
		e.charisma              = 0;
		e.magic_resist          = 0;
		e.cold_resist           = 0;
		e.fire_resist           = 0;
		e.poison_resist         = 0;
		e.disease_resist        = 0;
		e.corruption_resist     = 0;
		e.physical_resist       = 0;
		e.min_dmg               = 0;
		e.max_dmg               = 0;
		e.hp_regen_rate         = 0;
		e.hp_regen_per_second   = 0;
		e.attack_delay          = 0;
		e.spell_scale           = 100;
		e.heal_scale            = 100;
		e.avoidance             = 0;
		e.heroic_strikethrough  = 0;
		e.special_abilities     = "";

		return e;
	}

	static NpcScaleGlobalBase GetNpcScaleGlobalBase(
		const std::vector<NpcScaleGlobalBase> &npc_scale_global_bases,
		int npc_scale_global_base_id
	)
	{
		for (auto &npc_scale_global_base : npc_scale_global_bases) {
			if (npc_scale_global_base.type == npc_scale_global_base_id) {
				return npc_scale_global_base;
			}
		}

		return NewEntity();
	}

	static NpcScaleGlobalBase FindOne(
		Database& db,
		int npc_scale_global_base_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				npc_scale_global_base_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			NpcScaleGlobalBase e{};

			e.type                  = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.level                 = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.zone_id_list          = row[2] ? row[2] : "";
			e.instance_version_list = row[3] ? row[3] : "";
			e.ac                    = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.hp                    = row[5] ? strtoll(row[5], nullptr, 10) : 0;
			e.accuracy              = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.slow_mitigation       = row[7] ? static_cast<int32_t>(atoi(row[7])) : 0;
			e.attack                = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.strength              = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.stamina               = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.dexterity             = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;
			e.agility               = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.intelligence          = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.wisdom                = row[14] ? static_cast<int32_t>(atoi(row[14])) : 0;
			e.charisma              = row[15] ? static_cast<int32_t>(atoi(row[15])) : 0;
			e.magic_resist          = row[16] ? static_cast<int32_t>(atoi(row[16])) : 0;
			e.cold_resist           = row[17] ? static_cast<int32_t>(atoi(row[17])) : 0;
			e.fire_resist           = row[18] ? static_cast<int32_t>(atoi(row[18])) : 0;
			e.poison_resist         = row[19] ? static_cast<int32_t>(atoi(row[19])) : 0;
			e.disease_resist        = row[20] ? static_cast<int32_t>(atoi(row[20])) : 0;
			e.corruption_resist     = row[21] ? static_cast<int32_t>(atoi(row[21])) : 0;
			e.physical_resist       = row[22] ? static_cast<int32_t>(atoi(row[22])) : 0;
			e.min_dmg               = row[23] ? static_cast<int32_t>(atoi(row[23])) : 0;
			e.max_dmg               = row[24] ? static_cast<int32_t>(atoi(row[24])) : 0;
			e.hp_regen_rate         = row[25] ? strtoll(row[25], nullptr, 10) : 0;
			e.hp_regen_per_second   = row[26] ? strtoll(row[26], nullptr, 10) : 0;
			e.attack_delay          = row[27] ? static_cast<int32_t>(atoi(row[27])) : 0;
			e.spell_scale           = row[28] ? static_cast<int32_t>(atoi(row[28])) : 100;
			e.heal_scale            = row[29] ? static_cast<int32_t>(atoi(row[29])) : 100;
			e.avoidance             = row[30] ? static_cast<uint32_t>(strtoul(row[30], nullptr, 10)) : 0;
			e.heroic_strikethrough  = row[31] ? static_cast<int32_t>(atoi(row[31])) : 0;
			e.special_abilities     = row[32] ? row[32] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int npc_scale_global_base_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				npc_scale_global_base_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const NpcScaleGlobalBase &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.type));
		v.push_back(columns[1] + " = " + std::to_string(e.level));
		v.push_back(columns[2] + " = '" + Strings::Escape(e.zone_id_list) + "'");
		v.push_back(columns[3] + " = '" + Strings::Escape(e.instance_version_list) + "'");
		v.push_back(columns[4] + " = " + std::to_string(e.ac));
		v.push_back(columns[5] + " = " + std::to_string(e.hp));
		v.push_back(columns[6] + " = " + std::to_string(e.accuracy));
		v.push_back(columns[7] + " = " + std::to_string(e.slow_mitigation));
		v.push_back(columns[8] + " = " + std::to_string(e.attack));
		v.push_back(columns[9] + " = " + std::to_string(e.strength));
		v.push_back(columns[10] + " = " + std::to_string(e.stamina));
		v.push_back(columns[11] + " = " + std::to_string(e.dexterity));
		v.push_back(columns[12] + " = " + std::to_string(e.agility));
		v.push_back(columns[13] + " = " + std::to_string(e.intelligence));
		v.push_back(columns[14] + " = " + std::to_string(e.wisdom));
		v.push_back(columns[15] + " = " + std::to_string(e.charisma));
		v.push_back(columns[16] + " = " + std::to_string(e.magic_resist));
		v.push_back(columns[17] + " = " + std::to_string(e.cold_resist));
		v.push_back(columns[18] + " = " + std::to_string(e.fire_resist));
		v.push_back(columns[19] + " = " + std::to_string(e.poison_resist));
		v.push_back(columns[20] + " = " + std::to_string(e.disease_resist));
		v.push_back(columns[21] + " = " + std::to_string(e.corruption_resist));
		v.push_back(columns[22] + " = " + std::to_string(e.physical_resist));
		v.push_back(columns[23] + " = " + std::to_string(e.min_dmg));
		v.push_back(columns[24] + " = " + std::to_string(e.max_dmg));
		v.push_back(columns[25] + " = " + std::to_string(e.hp_regen_rate));
		v.push_back(columns[26] + " = " + std::to_string(e.hp_regen_per_second));
		v.push_back(columns[27] + " = " + std::to_string(e.attack_delay));
		v.push_back(columns[28] + " = " + std::to_string(e.spell_scale));
		v.push_back(columns[29] + " = " + std::to_string(e.heal_scale));
		v.push_back(columns[30] + " = " + std::to_string(e.avoidance));
		v.push_back(columns[31] + " = " + std::to_string(e.heroic_strikethrough));
		v.push_back(columns[32] + " = '" + Strings::Escape(e.special_abilities) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.type
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static NpcScaleGlobalBase InsertOne(
		Database& db,
		NpcScaleGlobalBase e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.level));
		v.push_back("'" + Strings::Escape(e.zone_id_list) + "'");
		v.push_back("'" + Strings::Escape(e.instance_version_list) + "'");
		v.push_back(std::to_string(e.ac));
		v.push_back(std::to_string(e.hp));
		v.push_back(std::to_string(e.accuracy));
		v.push_back(std::to_string(e.slow_mitigation));
		v.push_back(std::to_string(e.attack));
		v.push_back(std::to_string(e.strength));
		v.push_back(std::to_string(e.stamina));
		v.push_back(std::to_string(e.dexterity));
		v.push_back(std::to_string(e.agility));
		v.push_back(std::to_string(e.intelligence));
		v.push_back(std::to_string(e.wisdom));
		v.push_back(std::to_string(e.charisma));
		v.push_back(std::to_string(e.magic_resist));
		v.push_back(std::to_string(e.cold_resist));
		v.push_back(std::to_string(e.fire_resist));
		v.push_back(std::to_string(e.poison_resist));
		v.push_back(std::to_string(e.disease_resist));
		v.push_back(std::to_string(e.corruption_resist));
		v.push_back(std::to_string(e.physical_resist));
		v.push_back(std::to_string(e.min_dmg));
		v.push_back(std::to_string(e.max_dmg));
		v.push_back(std::to_string(e.hp_regen_rate));
		v.push_back(std::to_string(e.hp_regen_per_second));
		v.push_back(std::to_string(e.attack_delay));
		v.push_back(std::to_string(e.spell_scale));
		v.push_back(std::to_string(e.heal_scale));
		v.push_back(std::to_string(e.avoidance));
		v.push_back(std::to_string(e.heroic_strikethrough));
		v.push_back("'" + Strings::Escape(e.special_abilities) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.type = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<NpcScaleGlobalBase> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.level));
			v.push_back("'" + Strings::Escape(e.zone_id_list) + "'");
			v.push_back("'" + Strings::Escape(e.instance_version_list) + "'");
			v.push_back(std::to_string(e.ac));
			v.push_back(std::to_string(e.hp));
			v.push_back(std::to_string(e.accuracy));
			v.push_back(std::to_string(e.slow_mitigation));
			v.push_back(std::to_string(e.attack));
			v.push_back(std::to_string(e.strength));
			v.push_back(std::to_string(e.stamina));
			v.push_back(std::to_string(e.dexterity));
			v.push_back(std::to_string(e.agility));
			v.push_back(std::to_string(e.intelligence));
			v.push_back(std::to_string(e.wisdom));
			v.push_back(std::to_string(e.charisma));
			v.push_back(std::to_string(e.magic_resist));
			v.push_back(std::to_string(e.cold_resist));
			v.push_back(std::to_string(e.fire_resist));
			v.push_back(std::to_string(e.poison_resist));
			v.push_back(std::to_string(e.disease_resist));
			v.push_back(std::to_string(e.corruption_resist));
			v.push_back(std::to_string(e.physical_resist));
			v.push_back(std::to_string(e.min_dmg));
			v.push_back(std::to_string(e.max_dmg));
			v.push_back(std::to_string(e.hp_regen_rate));
			v.push_back(std::to_string(e.hp_regen_per_second));
			v.push_back(std::to_string(e.attack_delay));
			v.push_back(std::to_string(e.spell_scale));
			v.push_back(std::to_string(e.heal_scale));
			v.push_back(std::to_string(e.avoidance));
			v.push_back(std::to_string(e.heroic_strikethrough));
			v.push_back("'" + Strings::Escape(e.special_abilities) + "'");

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

	static std::vector<NpcScaleGlobalBase> All(Database& db)
	{
		std::vector<NpcScaleGlobalBase> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcScaleGlobalBase e{};

			e.type                  = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.level                 = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.zone_id_list          = row[2] ? row[2] : "";
			e.instance_version_list = row[3] ? row[3] : "";
			e.ac                    = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.hp                    = row[5] ? strtoll(row[5], nullptr, 10) : 0;
			e.accuracy              = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.slow_mitigation       = row[7] ? static_cast<int32_t>(atoi(row[7])) : 0;
			e.attack                = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.strength              = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.stamina               = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.dexterity             = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;
			e.agility               = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.intelligence          = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.wisdom                = row[14] ? static_cast<int32_t>(atoi(row[14])) : 0;
			e.charisma              = row[15] ? static_cast<int32_t>(atoi(row[15])) : 0;
			e.magic_resist          = row[16] ? static_cast<int32_t>(atoi(row[16])) : 0;
			e.cold_resist           = row[17] ? static_cast<int32_t>(atoi(row[17])) : 0;
			e.fire_resist           = row[18] ? static_cast<int32_t>(atoi(row[18])) : 0;
			e.poison_resist         = row[19] ? static_cast<int32_t>(atoi(row[19])) : 0;
			e.disease_resist        = row[20] ? static_cast<int32_t>(atoi(row[20])) : 0;
			e.corruption_resist     = row[21] ? static_cast<int32_t>(atoi(row[21])) : 0;
			e.physical_resist       = row[22] ? static_cast<int32_t>(atoi(row[22])) : 0;
			e.min_dmg               = row[23] ? static_cast<int32_t>(atoi(row[23])) : 0;
			e.max_dmg               = row[24] ? static_cast<int32_t>(atoi(row[24])) : 0;
			e.hp_regen_rate         = row[25] ? strtoll(row[25], nullptr, 10) : 0;
			e.hp_regen_per_second   = row[26] ? strtoll(row[26], nullptr, 10) : 0;
			e.attack_delay          = row[27] ? static_cast<int32_t>(atoi(row[27])) : 0;
			e.spell_scale           = row[28] ? static_cast<int32_t>(atoi(row[28])) : 100;
			e.heal_scale            = row[29] ? static_cast<int32_t>(atoi(row[29])) : 100;
			e.avoidance             = row[30] ? static_cast<uint32_t>(strtoul(row[30], nullptr, 10)) : 0;
			e.heroic_strikethrough  = row[31] ? static_cast<int32_t>(atoi(row[31])) : 0;
			e.special_abilities     = row[32] ? row[32] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<NpcScaleGlobalBase> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<NpcScaleGlobalBase> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcScaleGlobalBase e{};

			e.type                  = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.level                 = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.zone_id_list          = row[2] ? row[2] : "";
			e.instance_version_list = row[3] ? row[3] : "";
			e.ac                    = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.hp                    = row[5] ? strtoll(row[5], nullptr, 10) : 0;
			e.accuracy              = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.slow_mitigation       = row[7] ? static_cast<int32_t>(atoi(row[7])) : 0;
			e.attack                = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.strength              = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.stamina               = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.dexterity             = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;
			e.agility               = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.intelligence          = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.wisdom                = row[14] ? static_cast<int32_t>(atoi(row[14])) : 0;
			e.charisma              = row[15] ? static_cast<int32_t>(atoi(row[15])) : 0;
			e.magic_resist          = row[16] ? static_cast<int32_t>(atoi(row[16])) : 0;
			e.cold_resist           = row[17] ? static_cast<int32_t>(atoi(row[17])) : 0;
			e.fire_resist           = row[18] ? static_cast<int32_t>(atoi(row[18])) : 0;
			e.poison_resist         = row[19] ? static_cast<int32_t>(atoi(row[19])) : 0;
			e.disease_resist        = row[20] ? static_cast<int32_t>(atoi(row[20])) : 0;
			e.corruption_resist     = row[21] ? static_cast<int32_t>(atoi(row[21])) : 0;
			e.physical_resist       = row[22] ? static_cast<int32_t>(atoi(row[22])) : 0;
			e.min_dmg               = row[23] ? static_cast<int32_t>(atoi(row[23])) : 0;
			e.max_dmg               = row[24] ? static_cast<int32_t>(atoi(row[24])) : 0;
			e.hp_regen_rate         = row[25] ? strtoll(row[25], nullptr, 10) : 0;
			e.hp_regen_per_second   = row[26] ? strtoll(row[26], nullptr, 10) : 0;
			e.attack_delay          = row[27] ? static_cast<int32_t>(atoi(row[27])) : 0;
			e.spell_scale           = row[28] ? static_cast<int32_t>(atoi(row[28])) : 100;
			e.heal_scale            = row[29] ? static_cast<int32_t>(atoi(row[29])) : 100;
			e.avoidance             = row[30] ? static_cast<uint32_t>(strtoul(row[30], nullptr, 10)) : 0;
			e.heroic_strikethrough  = row[31] ? static_cast<int32_t>(atoi(row[31])) : 0;
			e.special_abilities     = row[32] ? row[32] : "";

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
		const NpcScaleGlobalBase &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.level));
		v.push_back("'" + Strings::Escape(e.zone_id_list) + "'");
		v.push_back("'" + Strings::Escape(e.instance_version_list) + "'");
		v.push_back(std::to_string(e.ac));
		v.push_back(std::to_string(e.hp));
		v.push_back(std::to_string(e.accuracy));
		v.push_back(std::to_string(e.slow_mitigation));
		v.push_back(std::to_string(e.attack));
		v.push_back(std::to_string(e.strength));
		v.push_back(std::to_string(e.stamina));
		v.push_back(std::to_string(e.dexterity));
		v.push_back(std::to_string(e.agility));
		v.push_back(std::to_string(e.intelligence));
		v.push_back(std::to_string(e.wisdom));
		v.push_back(std::to_string(e.charisma));
		v.push_back(std::to_string(e.magic_resist));
		v.push_back(std::to_string(e.cold_resist));
		v.push_back(std::to_string(e.fire_resist));
		v.push_back(std::to_string(e.poison_resist));
		v.push_back(std::to_string(e.disease_resist));
		v.push_back(std::to_string(e.corruption_resist));
		v.push_back(std::to_string(e.physical_resist));
		v.push_back(std::to_string(e.min_dmg));
		v.push_back(std::to_string(e.max_dmg));
		v.push_back(std::to_string(e.hp_regen_rate));
		v.push_back(std::to_string(e.hp_regen_per_second));
		v.push_back(std::to_string(e.attack_delay));
		v.push_back(std::to_string(e.spell_scale));
		v.push_back(std::to_string(e.heal_scale));
		v.push_back(std::to_string(e.avoidance));
		v.push_back(std::to_string(e.heroic_strikethrough));
		v.push_back("'" + Strings::Escape(e.special_abilities) + "'");

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
		const std::vector<NpcScaleGlobalBase> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.level));
			v.push_back("'" + Strings::Escape(e.zone_id_list) + "'");
			v.push_back("'" + Strings::Escape(e.instance_version_list) + "'");
			v.push_back(std::to_string(e.ac));
			v.push_back(std::to_string(e.hp));
			v.push_back(std::to_string(e.accuracy));
			v.push_back(std::to_string(e.slow_mitigation));
			v.push_back(std::to_string(e.attack));
			v.push_back(std::to_string(e.strength));
			v.push_back(std::to_string(e.stamina));
			v.push_back(std::to_string(e.dexterity));
			v.push_back(std::to_string(e.agility));
			v.push_back(std::to_string(e.intelligence));
			v.push_back(std::to_string(e.wisdom));
			v.push_back(std::to_string(e.charisma));
			v.push_back(std::to_string(e.magic_resist));
			v.push_back(std::to_string(e.cold_resist));
			v.push_back(std::to_string(e.fire_resist));
			v.push_back(std::to_string(e.poison_resist));
			v.push_back(std::to_string(e.disease_resist));
			v.push_back(std::to_string(e.corruption_resist));
			v.push_back(std::to_string(e.physical_resist));
			v.push_back(std::to_string(e.min_dmg));
			v.push_back(std::to_string(e.max_dmg));
			v.push_back(std::to_string(e.hp_regen_rate));
			v.push_back(std::to_string(e.hp_regen_per_second));
			v.push_back(std::to_string(e.attack_delay));
			v.push_back(std::to_string(e.spell_scale));
			v.push_back(std::to_string(e.heal_scale));
			v.push_back(std::to_string(e.avoidance));
			v.push_back(std::to_string(e.heroic_strikethrough));
			v.push_back("'" + Strings::Escape(e.special_abilities) + "'");

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
