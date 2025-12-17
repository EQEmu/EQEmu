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

class BaseCharacterStatsRecordRepository {
public:
	struct CharacterStatsRecord {
		int32_t     character_id;
		std::string name;
		int32_t     status;
		int32_t     level;
		int32_t     class_;
		int32_t     race;
		int32_t     aa_points;
		int64_t     hp;
		int64_t     mana;
		int64_t     endurance;
		int32_t     ac;
		int32_t     strength;
		int32_t     stamina;
		int32_t     dexterity;
		int32_t     agility;
		int32_t     intelligence;
		int32_t     wisdom;
		int32_t     charisma;
		int32_t     magic_resist;
		int32_t     fire_resist;
		int32_t     cold_resist;
		int32_t     poison_resist;
		int32_t     disease_resist;
		int32_t     corruption_resist;
		int32_t     heroic_strength;
		int32_t     heroic_stamina;
		int32_t     heroic_dexterity;
		int32_t     heroic_agility;
		int32_t     heroic_intelligence;
		int32_t     heroic_wisdom;
		int32_t     heroic_charisma;
		int32_t     heroic_magic_resist;
		int32_t     heroic_fire_resist;
		int32_t     heroic_cold_resist;
		int32_t     heroic_poison_resist;
		int32_t     heroic_disease_resist;
		int32_t     heroic_corruption_resist;
		int32_t     haste;
		int32_t     accuracy;
		int32_t     attack;
		int32_t     avoidance;
		int32_t     clairvoyance;
		int32_t     combat_effects;
		int32_t     damage_shield_mitigation;
		int32_t     damage_shield;
		int32_t     dot_shielding;
		int32_t     hp_regen;
		int32_t     mana_regen;
		int32_t     endurance_regen;
		int32_t     shielding;
		int32_t     spell_damage;
		int32_t     heal_amount;
		int32_t     spell_shielding;
		int32_t     strikethrough;
		int32_t     stun_resist;
		int32_t     backstab;
		int32_t     wind;
		int32_t     brass;
		int32_t     string;
		int32_t     percussion;
		int32_t     singing;
		int32_t     baking;
		int32_t     alchemy;
		int32_t     tailoring;
		int32_t     blacksmithing;
		int32_t     fletching;
		int32_t     brewing;
		int32_t     jewelry;
		int32_t     pottery;
		int32_t     research;
		int32_t     alcohol;
		int32_t     fishing;
		int32_t     tinkering;
		time_t      created_at;
		time_t      updated_at;
	};

	static std::string PrimaryKey()
	{
		return std::string("character_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"character_id",
			"name",
			"status",
			"level",
			"`class`",
			"race",
			"aa_points",
			"hp",
			"mana",
			"endurance",
			"ac",
			"strength",
			"stamina",
			"dexterity",
			"agility",
			"intelligence",
			"wisdom",
			"charisma",
			"magic_resist",
			"fire_resist",
			"cold_resist",
			"poison_resist",
			"disease_resist",
			"corruption_resist",
			"heroic_strength",
			"heroic_stamina",
			"heroic_dexterity",
			"heroic_agility",
			"heroic_intelligence",
			"heroic_wisdom",
			"heroic_charisma",
			"heroic_magic_resist",
			"heroic_fire_resist",
			"heroic_cold_resist",
			"heroic_poison_resist",
			"heroic_disease_resist",
			"heroic_corruption_resist",
			"haste",
			"accuracy",
			"attack",
			"avoidance",
			"clairvoyance",
			"combat_effects",
			"damage_shield_mitigation",
			"damage_shield",
			"dot_shielding",
			"hp_regen",
			"mana_regen",
			"endurance_regen",
			"shielding",
			"spell_damage",
			"heal_amount",
			"spell_shielding",
			"strikethrough",
			"stun_resist",
			"backstab",
			"wind",
			"brass",
			"string",
			"percussion",
			"singing",
			"baking",
			"alchemy",
			"tailoring",
			"blacksmithing",
			"fletching",
			"brewing",
			"jewelry",
			"pottery",
			"research",
			"alcohol",
			"fishing",
			"tinkering",
			"created_at",
			"updated_at",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"character_id",
			"name",
			"status",
			"level",
			"`class`",
			"race",
			"aa_points",
			"hp",
			"mana",
			"endurance",
			"ac",
			"strength",
			"stamina",
			"dexterity",
			"agility",
			"intelligence",
			"wisdom",
			"charisma",
			"magic_resist",
			"fire_resist",
			"cold_resist",
			"poison_resist",
			"disease_resist",
			"corruption_resist",
			"heroic_strength",
			"heroic_stamina",
			"heroic_dexterity",
			"heroic_agility",
			"heroic_intelligence",
			"heroic_wisdom",
			"heroic_charisma",
			"heroic_magic_resist",
			"heroic_fire_resist",
			"heroic_cold_resist",
			"heroic_poison_resist",
			"heroic_disease_resist",
			"heroic_corruption_resist",
			"haste",
			"accuracy",
			"attack",
			"avoidance",
			"clairvoyance",
			"combat_effects",
			"damage_shield_mitigation",
			"damage_shield",
			"dot_shielding",
			"hp_regen",
			"mana_regen",
			"endurance_regen",
			"shielding",
			"spell_damage",
			"heal_amount",
			"spell_shielding",
			"strikethrough",
			"stun_resist",
			"backstab",
			"wind",
			"brass",
			"string",
			"percussion",
			"singing",
			"baking",
			"alchemy",
			"tailoring",
			"blacksmithing",
			"fletching",
			"brewing",
			"jewelry",
			"pottery",
			"research",
			"alcohol",
			"fishing",
			"tinkering",
			"UNIX_TIMESTAMP(created_at)",
			"UNIX_TIMESTAMP(updated_at)",
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
		return std::string("character_stats_record");
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

	static CharacterStatsRecord NewEntity()
	{
		CharacterStatsRecord e{};

		e.character_id             = 0;
		e.name                     = "";
		e.status                   = 0;
		e.level                    = 0;
		e.class_                   = 0;
		e.race                     = 0;
		e.aa_points                = 0;
		e.hp                       = 0;
		e.mana                     = 0;
		e.endurance                = 0;
		e.ac                       = 0;
		e.strength                 = 0;
		e.stamina                  = 0;
		e.dexterity                = 0;
		e.agility                  = 0;
		e.intelligence             = 0;
		e.wisdom                   = 0;
		e.charisma                 = 0;
		e.magic_resist             = 0;
		e.fire_resist              = 0;
		e.cold_resist              = 0;
		e.poison_resist            = 0;
		e.disease_resist           = 0;
		e.corruption_resist        = 0;
		e.heroic_strength          = 0;
		e.heroic_stamina           = 0;
		e.heroic_dexterity         = 0;
		e.heroic_agility           = 0;
		e.heroic_intelligence      = 0;
		e.heroic_wisdom            = 0;
		e.heroic_charisma          = 0;
		e.heroic_magic_resist      = 0;
		e.heroic_fire_resist       = 0;
		e.heroic_cold_resist       = 0;
		e.heroic_poison_resist     = 0;
		e.heroic_disease_resist    = 0;
		e.heroic_corruption_resist = 0;
		e.haste                    = 0;
		e.accuracy                 = 0;
		e.attack                   = 0;
		e.avoidance                = 0;
		e.clairvoyance             = 0;
		e.combat_effects           = 0;
		e.damage_shield_mitigation = 0;
		e.damage_shield            = 0;
		e.dot_shielding            = 0;
		e.hp_regen                 = 0;
		e.mana_regen               = 0;
		e.endurance_regen          = 0;
		e.shielding                = 0;
		e.spell_damage             = 0;
		e.heal_amount              = 0;
		e.spell_shielding          = 0;
		e.strikethrough            = 0;
		e.stun_resist              = 0;
		e.backstab                 = 0;
		e.wind                     = 0;
		e.brass                    = 0;
		e.string                   = 0;
		e.percussion               = 0;
		e.singing                  = 0;
		e.baking                   = 0;
		e.alchemy                  = 0;
		e.tailoring                = 0;
		e.blacksmithing            = 0;
		e.fletching                = 0;
		e.brewing                  = 0;
		e.jewelry                  = 0;
		e.pottery                  = 0;
		e.research                 = 0;
		e.alcohol                  = 0;
		e.fishing                  = 0;
		e.tinkering                = 0;
		e.created_at               = 0;
		e.updated_at               = 0;

		return e;
	}

	static CharacterStatsRecord GetCharacterStatsRecord(
		const std::vector<CharacterStatsRecord> &character_stats_records,
		int character_stats_record_id
	)
	{
		for (auto &character_stats_record : character_stats_records) {
			if (character_stats_record.character_id == character_stats_record_id) {
				return character_stats_record;
			}
		}

		return NewEntity();
	}

	static CharacterStatsRecord FindOne(
		Database& db,
		int character_stats_record_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				character_stats_record_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterStatsRecord e{};

			e.character_id             = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.name                     = row[1] ? row[1] : "";
			e.status                   = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.level                    = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.class_                   = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.race                     = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.aa_points                = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.hp                       = row[7] ? strtoll(row[7], nullptr, 10) : 0;
			e.mana                     = row[8] ? strtoll(row[8], nullptr, 10) : 0;
			e.endurance                = row[9] ? strtoll(row[9], nullptr, 10) : 0;
			e.ac                       = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.strength                 = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;
			e.stamina                  = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.dexterity                = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.agility                  = row[14] ? static_cast<int32_t>(atoi(row[14])) : 0;
			e.intelligence             = row[15] ? static_cast<int32_t>(atoi(row[15])) : 0;
			e.wisdom                   = row[16] ? static_cast<int32_t>(atoi(row[16])) : 0;
			e.charisma                 = row[17] ? static_cast<int32_t>(atoi(row[17])) : 0;
			e.magic_resist             = row[18] ? static_cast<int32_t>(atoi(row[18])) : 0;
			e.fire_resist              = row[19] ? static_cast<int32_t>(atoi(row[19])) : 0;
			e.cold_resist              = row[20] ? static_cast<int32_t>(atoi(row[20])) : 0;
			e.poison_resist            = row[21] ? static_cast<int32_t>(atoi(row[21])) : 0;
			e.disease_resist           = row[22] ? static_cast<int32_t>(atoi(row[22])) : 0;
			e.corruption_resist        = row[23] ? static_cast<int32_t>(atoi(row[23])) : 0;
			e.heroic_strength          = row[24] ? static_cast<int32_t>(atoi(row[24])) : 0;
			e.heroic_stamina           = row[25] ? static_cast<int32_t>(atoi(row[25])) : 0;
			e.heroic_dexterity         = row[26] ? static_cast<int32_t>(atoi(row[26])) : 0;
			e.heroic_agility           = row[27] ? static_cast<int32_t>(atoi(row[27])) : 0;
			e.heroic_intelligence      = row[28] ? static_cast<int32_t>(atoi(row[28])) : 0;
			e.heroic_wisdom            = row[29] ? static_cast<int32_t>(atoi(row[29])) : 0;
			e.heroic_charisma          = row[30] ? static_cast<int32_t>(atoi(row[30])) : 0;
			e.heroic_magic_resist      = row[31] ? static_cast<int32_t>(atoi(row[31])) : 0;
			e.heroic_fire_resist       = row[32] ? static_cast<int32_t>(atoi(row[32])) : 0;
			e.heroic_cold_resist       = row[33] ? static_cast<int32_t>(atoi(row[33])) : 0;
			e.heroic_poison_resist     = row[34] ? static_cast<int32_t>(atoi(row[34])) : 0;
			e.heroic_disease_resist    = row[35] ? static_cast<int32_t>(atoi(row[35])) : 0;
			e.heroic_corruption_resist = row[36] ? static_cast<int32_t>(atoi(row[36])) : 0;
			e.haste                    = row[37] ? static_cast<int32_t>(atoi(row[37])) : 0;
			e.accuracy                 = row[38] ? static_cast<int32_t>(atoi(row[38])) : 0;
			e.attack                   = row[39] ? static_cast<int32_t>(atoi(row[39])) : 0;
			e.avoidance                = row[40] ? static_cast<int32_t>(atoi(row[40])) : 0;
			e.clairvoyance             = row[41] ? static_cast<int32_t>(atoi(row[41])) : 0;
			e.combat_effects           = row[42] ? static_cast<int32_t>(atoi(row[42])) : 0;
			e.damage_shield_mitigation = row[43] ? static_cast<int32_t>(atoi(row[43])) : 0;
			e.damage_shield            = row[44] ? static_cast<int32_t>(atoi(row[44])) : 0;
			e.dot_shielding            = row[45] ? static_cast<int32_t>(atoi(row[45])) : 0;
			e.hp_regen                 = row[46] ? static_cast<int32_t>(atoi(row[46])) : 0;
			e.mana_regen               = row[47] ? static_cast<int32_t>(atoi(row[47])) : 0;
			e.endurance_regen          = row[48] ? static_cast<int32_t>(atoi(row[48])) : 0;
			e.shielding                = row[49] ? static_cast<int32_t>(atoi(row[49])) : 0;
			e.spell_damage             = row[50] ? static_cast<int32_t>(atoi(row[50])) : 0;
			e.heal_amount              = row[51] ? static_cast<int32_t>(atoi(row[51])) : 0;
			e.spell_shielding          = row[52] ? static_cast<int32_t>(atoi(row[52])) : 0;
			e.strikethrough            = row[53] ? static_cast<int32_t>(atoi(row[53])) : 0;
			e.stun_resist              = row[54] ? static_cast<int32_t>(atoi(row[54])) : 0;
			e.backstab                 = row[55] ? static_cast<int32_t>(atoi(row[55])) : 0;
			e.wind                     = row[56] ? static_cast<int32_t>(atoi(row[56])) : 0;
			e.brass                    = row[57] ? static_cast<int32_t>(atoi(row[57])) : 0;
			e.string                   = row[58] ? static_cast<int32_t>(atoi(row[58])) : 0;
			e.percussion               = row[59] ? static_cast<int32_t>(atoi(row[59])) : 0;
			e.singing                  = row[60] ? static_cast<int32_t>(atoi(row[60])) : 0;
			e.baking                   = row[61] ? static_cast<int32_t>(atoi(row[61])) : 0;
			e.alchemy                  = row[62] ? static_cast<int32_t>(atoi(row[62])) : 0;
			e.tailoring                = row[63] ? static_cast<int32_t>(atoi(row[63])) : 0;
			e.blacksmithing            = row[64] ? static_cast<int32_t>(atoi(row[64])) : 0;
			e.fletching                = row[65] ? static_cast<int32_t>(atoi(row[65])) : 0;
			e.brewing                  = row[66] ? static_cast<int32_t>(atoi(row[66])) : 0;
			e.jewelry                  = row[67] ? static_cast<int32_t>(atoi(row[67])) : 0;
			e.pottery                  = row[68] ? static_cast<int32_t>(atoi(row[68])) : 0;
			e.research                 = row[69] ? static_cast<int32_t>(atoi(row[69])) : 0;
			e.alcohol                  = row[70] ? static_cast<int32_t>(atoi(row[70])) : 0;
			e.fishing                  = row[71] ? static_cast<int32_t>(atoi(row[71])) : 0;
			e.tinkering                = row[72] ? static_cast<int32_t>(atoi(row[72])) : 0;
			e.created_at               = strtoll(row[73] ? row[73] : "-1", nullptr, 10);
			e.updated_at               = strtoll(row[74] ? row[74] : "-1", nullptr, 10);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_stats_record_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_stats_record_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CharacterStatsRecord &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.character_id));
		v.push_back(columns[1] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[2] + " = " + std::to_string(e.status));
		v.push_back(columns[3] + " = " + std::to_string(e.level));
		v.push_back(columns[4] + " = " + std::to_string(e.class_));
		v.push_back(columns[5] + " = " + std::to_string(e.race));
		v.push_back(columns[6] + " = " + std::to_string(e.aa_points));
		v.push_back(columns[7] + " = " + std::to_string(e.hp));
		v.push_back(columns[8] + " = " + std::to_string(e.mana));
		v.push_back(columns[9] + " = " + std::to_string(e.endurance));
		v.push_back(columns[10] + " = " + std::to_string(e.ac));
		v.push_back(columns[11] + " = " + std::to_string(e.strength));
		v.push_back(columns[12] + " = " + std::to_string(e.stamina));
		v.push_back(columns[13] + " = " + std::to_string(e.dexterity));
		v.push_back(columns[14] + " = " + std::to_string(e.agility));
		v.push_back(columns[15] + " = " + std::to_string(e.intelligence));
		v.push_back(columns[16] + " = " + std::to_string(e.wisdom));
		v.push_back(columns[17] + " = " + std::to_string(e.charisma));
		v.push_back(columns[18] + " = " + std::to_string(e.magic_resist));
		v.push_back(columns[19] + " = " + std::to_string(e.fire_resist));
		v.push_back(columns[20] + " = " + std::to_string(e.cold_resist));
		v.push_back(columns[21] + " = " + std::to_string(e.poison_resist));
		v.push_back(columns[22] + " = " + std::to_string(e.disease_resist));
		v.push_back(columns[23] + " = " + std::to_string(e.corruption_resist));
		v.push_back(columns[24] + " = " + std::to_string(e.heroic_strength));
		v.push_back(columns[25] + " = " + std::to_string(e.heroic_stamina));
		v.push_back(columns[26] + " = " + std::to_string(e.heroic_dexterity));
		v.push_back(columns[27] + " = " + std::to_string(e.heroic_agility));
		v.push_back(columns[28] + " = " + std::to_string(e.heroic_intelligence));
		v.push_back(columns[29] + " = " + std::to_string(e.heroic_wisdom));
		v.push_back(columns[30] + " = " + std::to_string(e.heroic_charisma));
		v.push_back(columns[31] + " = " + std::to_string(e.heroic_magic_resist));
		v.push_back(columns[32] + " = " + std::to_string(e.heroic_fire_resist));
		v.push_back(columns[33] + " = " + std::to_string(e.heroic_cold_resist));
		v.push_back(columns[34] + " = " + std::to_string(e.heroic_poison_resist));
		v.push_back(columns[35] + " = " + std::to_string(e.heroic_disease_resist));
		v.push_back(columns[36] + " = " + std::to_string(e.heroic_corruption_resist));
		v.push_back(columns[37] + " = " + std::to_string(e.haste));
		v.push_back(columns[38] + " = " + std::to_string(e.accuracy));
		v.push_back(columns[39] + " = " + std::to_string(e.attack));
		v.push_back(columns[40] + " = " + std::to_string(e.avoidance));
		v.push_back(columns[41] + " = " + std::to_string(e.clairvoyance));
		v.push_back(columns[42] + " = " + std::to_string(e.combat_effects));
		v.push_back(columns[43] + " = " + std::to_string(e.damage_shield_mitigation));
		v.push_back(columns[44] + " = " + std::to_string(e.damage_shield));
		v.push_back(columns[45] + " = " + std::to_string(e.dot_shielding));
		v.push_back(columns[46] + " = " + std::to_string(e.hp_regen));
		v.push_back(columns[47] + " = " + std::to_string(e.mana_regen));
		v.push_back(columns[48] + " = " + std::to_string(e.endurance_regen));
		v.push_back(columns[49] + " = " + std::to_string(e.shielding));
		v.push_back(columns[50] + " = " + std::to_string(e.spell_damage));
		v.push_back(columns[51] + " = " + std::to_string(e.heal_amount));
		v.push_back(columns[52] + " = " + std::to_string(e.spell_shielding));
		v.push_back(columns[53] + " = " + std::to_string(e.strikethrough));
		v.push_back(columns[54] + " = " + std::to_string(e.stun_resist));
		v.push_back(columns[55] + " = " + std::to_string(e.backstab));
		v.push_back(columns[56] + " = " + std::to_string(e.wind));
		v.push_back(columns[57] + " = " + std::to_string(e.brass));
		v.push_back(columns[58] + " = " + std::to_string(e.string));
		v.push_back(columns[59] + " = " + std::to_string(e.percussion));
		v.push_back(columns[60] + " = " + std::to_string(e.singing));
		v.push_back(columns[61] + " = " + std::to_string(e.baking));
		v.push_back(columns[62] + " = " + std::to_string(e.alchemy));
		v.push_back(columns[63] + " = " + std::to_string(e.tailoring));
		v.push_back(columns[64] + " = " + std::to_string(e.blacksmithing));
		v.push_back(columns[65] + " = " + std::to_string(e.fletching));
		v.push_back(columns[66] + " = " + std::to_string(e.brewing));
		v.push_back(columns[67] + " = " + std::to_string(e.jewelry));
		v.push_back(columns[68] + " = " + std::to_string(e.pottery));
		v.push_back(columns[69] + " = " + std::to_string(e.research));
		v.push_back(columns[70] + " = " + std::to_string(e.alcohol));
		v.push_back(columns[71] + " = " + std::to_string(e.fishing));
		v.push_back(columns[72] + " = " + std::to_string(e.tinkering));
		v.push_back(columns[73] + " = FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");
		v.push_back(columns[74] + " = FROM_UNIXTIME(" + (e.updated_at > 0 ? std::to_string(e.updated_at) : "null") + ")");

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

	static CharacterStatsRecord InsertOne(
		Database& db,
		CharacterStatsRecord e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.character_id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.status));
		v.push_back(std::to_string(e.level));
		v.push_back(std::to_string(e.class_));
		v.push_back(std::to_string(e.race));
		v.push_back(std::to_string(e.aa_points));
		v.push_back(std::to_string(e.hp));
		v.push_back(std::to_string(e.mana));
		v.push_back(std::to_string(e.endurance));
		v.push_back(std::to_string(e.ac));
		v.push_back(std::to_string(e.strength));
		v.push_back(std::to_string(e.stamina));
		v.push_back(std::to_string(e.dexterity));
		v.push_back(std::to_string(e.agility));
		v.push_back(std::to_string(e.intelligence));
		v.push_back(std::to_string(e.wisdom));
		v.push_back(std::to_string(e.charisma));
		v.push_back(std::to_string(e.magic_resist));
		v.push_back(std::to_string(e.fire_resist));
		v.push_back(std::to_string(e.cold_resist));
		v.push_back(std::to_string(e.poison_resist));
		v.push_back(std::to_string(e.disease_resist));
		v.push_back(std::to_string(e.corruption_resist));
		v.push_back(std::to_string(e.heroic_strength));
		v.push_back(std::to_string(e.heroic_stamina));
		v.push_back(std::to_string(e.heroic_dexterity));
		v.push_back(std::to_string(e.heroic_agility));
		v.push_back(std::to_string(e.heroic_intelligence));
		v.push_back(std::to_string(e.heroic_wisdom));
		v.push_back(std::to_string(e.heroic_charisma));
		v.push_back(std::to_string(e.heroic_magic_resist));
		v.push_back(std::to_string(e.heroic_fire_resist));
		v.push_back(std::to_string(e.heroic_cold_resist));
		v.push_back(std::to_string(e.heroic_poison_resist));
		v.push_back(std::to_string(e.heroic_disease_resist));
		v.push_back(std::to_string(e.heroic_corruption_resist));
		v.push_back(std::to_string(e.haste));
		v.push_back(std::to_string(e.accuracy));
		v.push_back(std::to_string(e.attack));
		v.push_back(std::to_string(e.avoidance));
		v.push_back(std::to_string(e.clairvoyance));
		v.push_back(std::to_string(e.combat_effects));
		v.push_back(std::to_string(e.damage_shield_mitigation));
		v.push_back(std::to_string(e.damage_shield));
		v.push_back(std::to_string(e.dot_shielding));
		v.push_back(std::to_string(e.hp_regen));
		v.push_back(std::to_string(e.mana_regen));
		v.push_back(std::to_string(e.endurance_regen));
		v.push_back(std::to_string(e.shielding));
		v.push_back(std::to_string(e.spell_damage));
		v.push_back(std::to_string(e.heal_amount));
		v.push_back(std::to_string(e.spell_shielding));
		v.push_back(std::to_string(e.strikethrough));
		v.push_back(std::to_string(e.stun_resist));
		v.push_back(std::to_string(e.backstab));
		v.push_back(std::to_string(e.wind));
		v.push_back(std::to_string(e.brass));
		v.push_back(std::to_string(e.string));
		v.push_back(std::to_string(e.percussion));
		v.push_back(std::to_string(e.singing));
		v.push_back(std::to_string(e.baking));
		v.push_back(std::to_string(e.alchemy));
		v.push_back(std::to_string(e.tailoring));
		v.push_back(std::to_string(e.blacksmithing));
		v.push_back(std::to_string(e.fletching));
		v.push_back(std::to_string(e.brewing));
		v.push_back(std::to_string(e.jewelry));
		v.push_back(std::to_string(e.pottery));
		v.push_back(std::to_string(e.research));
		v.push_back(std::to_string(e.alcohol));
		v.push_back(std::to_string(e.fishing));
		v.push_back(std::to_string(e.tinkering));
		v.push_back("FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");
		v.push_back("FROM_UNIXTIME(" + (e.updated_at > 0 ? std::to_string(e.updated_at) : "null") + ")");

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
		const std::vector<CharacterStatsRecord> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.character_id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.status));
			v.push_back(std::to_string(e.level));
			v.push_back(std::to_string(e.class_));
			v.push_back(std::to_string(e.race));
			v.push_back(std::to_string(e.aa_points));
			v.push_back(std::to_string(e.hp));
			v.push_back(std::to_string(e.mana));
			v.push_back(std::to_string(e.endurance));
			v.push_back(std::to_string(e.ac));
			v.push_back(std::to_string(e.strength));
			v.push_back(std::to_string(e.stamina));
			v.push_back(std::to_string(e.dexterity));
			v.push_back(std::to_string(e.agility));
			v.push_back(std::to_string(e.intelligence));
			v.push_back(std::to_string(e.wisdom));
			v.push_back(std::to_string(e.charisma));
			v.push_back(std::to_string(e.magic_resist));
			v.push_back(std::to_string(e.fire_resist));
			v.push_back(std::to_string(e.cold_resist));
			v.push_back(std::to_string(e.poison_resist));
			v.push_back(std::to_string(e.disease_resist));
			v.push_back(std::to_string(e.corruption_resist));
			v.push_back(std::to_string(e.heroic_strength));
			v.push_back(std::to_string(e.heroic_stamina));
			v.push_back(std::to_string(e.heroic_dexterity));
			v.push_back(std::to_string(e.heroic_agility));
			v.push_back(std::to_string(e.heroic_intelligence));
			v.push_back(std::to_string(e.heroic_wisdom));
			v.push_back(std::to_string(e.heroic_charisma));
			v.push_back(std::to_string(e.heroic_magic_resist));
			v.push_back(std::to_string(e.heroic_fire_resist));
			v.push_back(std::to_string(e.heroic_cold_resist));
			v.push_back(std::to_string(e.heroic_poison_resist));
			v.push_back(std::to_string(e.heroic_disease_resist));
			v.push_back(std::to_string(e.heroic_corruption_resist));
			v.push_back(std::to_string(e.haste));
			v.push_back(std::to_string(e.accuracy));
			v.push_back(std::to_string(e.attack));
			v.push_back(std::to_string(e.avoidance));
			v.push_back(std::to_string(e.clairvoyance));
			v.push_back(std::to_string(e.combat_effects));
			v.push_back(std::to_string(e.damage_shield_mitigation));
			v.push_back(std::to_string(e.damage_shield));
			v.push_back(std::to_string(e.dot_shielding));
			v.push_back(std::to_string(e.hp_regen));
			v.push_back(std::to_string(e.mana_regen));
			v.push_back(std::to_string(e.endurance_regen));
			v.push_back(std::to_string(e.shielding));
			v.push_back(std::to_string(e.spell_damage));
			v.push_back(std::to_string(e.heal_amount));
			v.push_back(std::to_string(e.spell_shielding));
			v.push_back(std::to_string(e.strikethrough));
			v.push_back(std::to_string(e.stun_resist));
			v.push_back(std::to_string(e.backstab));
			v.push_back(std::to_string(e.wind));
			v.push_back(std::to_string(e.brass));
			v.push_back(std::to_string(e.string));
			v.push_back(std::to_string(e.percussion));
			v.push_back(std::to_string(e.singing));
			v.push_back(std::to_string(e.baking));
			v.push_back(std::to_string(e.alchemy));
			v.push_back(std::to_string(e.tailoring));
			v.push_back(std::to_string(e.blacksmithing));
			v.push_back(std::to_string(e.fletching));
			v.push_back(std::to_string(e.brewing));
			v.push_back(std::to_string(e.jewelry));
			v.push_back(std::to_string(e.pottery));
			v.push_back(std::to_string(e.research));
			v.push_back(std::to_string(e.alcohol));
			v.push_back(std::to_string(e.fishing));
			v.push_back(std::to_string(e.tinkering));
			v.push_back("FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");
			v.push_back("FROM_UNIXTIME(" + (e.updated_at > 0 ? std::to_string(e.updated_at) : "null") + ")");

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

	static std::vector<CharacterStatsRecord> All(Database& db)
	{
		std::vector<CharacterStatsRecord> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterStatsRecord e{};

			e.character_id             = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.name                     = row[1] ? row[1] : "";
			e.status                   = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.level                    = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.class_                   = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.race                     = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.aa_points                = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.hp                       = row[7] ? strtoll(row[7], nullptr, 10) : 0;
			e.mana                     = row[8] ? strtoll(row[8], nullptr, 10) : 0;
			e.endurance                = row[9] ? strtoll(row[9], nullptr, 10) : 0;
			e.ac                       = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.strength                 = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;
			e.stamina                  = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.dexterity                = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.agility                  = row[14] ? static_cast<int32_t>(atoi(row[14])) : 0;
			e.intelligence             = row[15] ? static_cast<int32_t>(atoi(row[15])) : 0;
			e.wisdom                   = row[16] ? static_cast<int32_t>(atoi(row[16])) : 0;
			e.charisma                 = row[17] ? static_cast<int32_t>(atoi(row[17])) : 0;
			e.magic_resist             = row[18] ? static_cast<int32_t>(atoi(row[18])) : 0;
			e.fire_resist              = row[19] ? static_cast<int32_t>(atoi(row[19])) : 0;
			e.cold_resist              = row[20] ? static_cast<int32_t>(atoi(row[20])) : 0;
			e.poison_resist            = row[21] ? static_cast<int32_t>(atoi(row[21])) : 0;
			e.disease_resist           = row[22] ? static_cast<int32_t>(atoi(row[22])) : 0;
			e.corruption_resist        = row[23] ? static_cast<int32_t>(atoi(row[23])) : 0;
			e.heroic_strength          = row[24] ? static_cast<int32_t>(atoi(row[24])) : 0;
			e.heroic_stamina           = row[25] ? static_cast<int32_t>(atoi(row[25])) : 0;
			e.heroic_dexterity         = row[26] ? static_cast<int32_t>(atoi(row[26])) : 0;
			e.heroic_agility           = row[27] ? static_cast<int32_t>(atoi(row[27])) : 0;
			e.heroic_intelligence      = row[28] ? static_cast<int32_t>(atoi(row[28])) : 0;
			e.heroic_wisdom            = row[29] ? static_cast<int32_t>(atoi(row[29])) : 0;
			e.heroic_charisma          = row[30] ? static_cast<int32_t>(atoi(row[30])) : 0;
			e.heroic_magic_resist      = row[31] ? static_cast<int32_t>(atoi(row[31])) : 0;
			e.heroic_fire_resist       = row[32] ? static_cast<int32_t>(atoi(row[32])) : 0;
			e.heroic_cold_resist       = row[33] ? static_cast<int32_t>(atoi(row[33])) : 0;
			e.heroic_poison_resist     = row[34] ? static_cast<int32_t>(atoi(row[34])) : 0;
			e.heroic_disease_resist    = row[35] ? static_cast<int32_t>(atoi(row[35])) : 0;
			e.heroic_corruption_resist = row[36] ? static_cast<int32_t>(atoi(row[36])) : 0;
			e.haste                    = row[37] ? static_cast<int32_t>(atoi(row[37])) : 0;
			e.accuracy                 = row[38] ? static_cast<int32_t>(atoi(row[38])) : 0;
			e.attack                   = row[39] ? static_cast<int32_t>(atoi(row[39])) : 0;
			e.avoidance                = row[40] ? static_cast<int32_t>(atoi(row[40])) : 0;
			e.clairvoyance             = row[41] ? static_cast<int32_t>(atoi(row[41])) : 0;
			e.combat_effects           = row[42] ? static_cast<int32_t>(atoi(row[42])) : 0;
			e.damage_shield_mitigation = row[43] ? static_cast<int32_t>(atoi(row[43])) : 0;
			e.damage_shield            = row[44] ? static_cast<int32_t>(atoi(row[44])) : 0;
			e.dot_shielding            = row[45] ? static_cast<int32_t>(atoi(row[45])) : 0;
			e.hp_regen                 = row[46] ? static_cast<int32_t>(atoi(row[46])) : 0;
			e.mana_regen               = row[47] ? static_cast<int32_t>(atoi(row[47])) : 0;
			e.endurance_regen          = row[48] ? static_cast<int32_t>(atoi(row[48])) : 0;
			e.shielding                = row[49] ? static_cast<int32_t>(atoi(row[49])) : 0;
			e.spell_damage             = row[50] ? static_cast<int32_t>(atoi(row[50])) : 0;
			e.heal_amount              = row[51] ? static_cast<int32_t>(atoi(row[51])) : 0;
			e.spell_shielding          = row[52] ? static_cast<int32_t>(atoi(row[52])) : 0;
			e.strikethrough            = row[53] ? static_cast<int32_t>(atoi(row[53])) : 0;
			e.stun_resist              = row[54] ? static_cast<int32_t>(atoi(row[54])) : 0;
			e.backstab                 = row[55] ? static_cast<int32_t>(atoi(row[55])) : 0;
			e.wind                     = row[56] ? static_cast<int32_t>(atoi(row[56])) : 0;
			e.brass                    = row[57] ? static_cast<int32_t>(atoi(row[57])) : 0;
			e.string                   = row[58] ? static_cast<int32_t>(atoi(row[58])) : 0;
			e.percussion               = row[59] ? static_cast<int32_t>(atoi(row[59])) : 0;
			e.singing                  = row[60] ? static_cast<int32_t>(atoi(row[60])) : 0;
			e.baking                   = row[61] ? static_cast<int32_t>(atoi(row[61])) : 0;
			e.alchemy                  = row[62] ? static_cast<int32_t>(atoi(row[62])) : 0;
			e.tailoring                = row[63] ? static_cast<int32_t>(atoi(row[63])) : 0;
			e.blacksmithing            = row[64] ? static_cast<int32_t>(atoi(row[64])) : 0;
			e.fletching                = row[65] ? static_cast<int32_t>(atoi(row[65])) : 0;
			e.brewing                  = row[66] ? static_cast<int32_t>(atoi(row[66])) : 0;
			e.jewelry                  = row[67] ? static_cast<int32_t>(atoi(row[67])) : 0;
			e.pottery                  = row[68] ? static_cast<int32_t>(atoi(row[68])) : 0;
			e.research                 = row[69] ? static_cast<int32_t>(atoi(row[69])) : 0;
			e.alcohol                  = row[70] ? static_cast<int32_t>(atoi(row[70])) : 0;
			e.fishing                  = row[71] ? static_cast<int32_t>(atoi(row[71])) : 0;
			e.tinkering                = row[72] ? static_cast<int32_t>(atoi(row[72])) : 0;
			e.created_at               = strtoll(row[73] ? row[73] : "-1", nullptr, 10);
			e.updated_at               = strtoll(row[74] ? row[74] : "-1", nullptr, 10);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CharacterStatsRecord> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CharacterStatsRecord> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterStatsRecord e{};

			e.character_id             = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.name                     = row[1] ? row[1] : "";
			e.status                   = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.level                    = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.class_                   = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.race                     = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.aa_points                = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.hp                       = row[7] ? strtoll(row[7], nullptr, 10) : 0;
			e.mana                     = row[8] ? strtoll(row[8], nullptr, 10) : 0;
			e.endurance                = row[9] ? strtoll(row[9], nullptr, 10) : 0;
			e.ac                       = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.strength                 = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;
			e.stamina                  = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.dexterity                = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.agility                  = row[14] ? static_cast<int32_t>(atoi(row[14])) : 0;
			e.intelligence             = row[15] ? static_cast<int32_t>(atoi(row[15])) : 0;
			e.wisdom                   = row[16] ? static_cast<int32_t>(atoi(row[16])) : 0;
			e.charisma                 = row[17] ? static_cast<int32_t>(atoi(row[17])) : 0;
			e.magic_resist             = row[18] ? static_cast<int32_t>(atoi(row[18])) : 0;
			e.fire_resist              = row[19] ? static_cast<int32_t>(atoi(row[19])) : 0;
			e.cold_resist              = row[20] ? static_cast<int32_t>(atoi(row[20])) : 0;
			e.poison_resist            = row[21] ? static_cast<int32_t>(atoi(row[21])) : 0;
			e.disease_resist           = row[22] ? static_cast<int32_t>(atoi(row[22])) : 0;
			e.corruption_resist        = row[23] ? static_cast<int32_t>(atoi(row[23])) : 0;
			e.heroic_strength          = row[24] ? static_cast<int32_t>(atoi(row[24])) : 0;
			e.heroic_stamina           = row[25] ? static_cast<int32_t>(atoi(row[25])) : 0;
			e.heroic_dexterity         = row[26] ? static_cast<int32_t>(atoi(row[26])) : 0;
			e.heroic_agility           = row[27] ? static_cast<int32_t>(atoi(row[27])) : 0;
			e.heroic_intelligence      = row[28] ? static_cast<int32_t>(atoi(row[28])) : 0;
			e.heroic_wisdom            = row[29] ? static_cast<int32_t>(atoi(row[29])) : 0;
			e.heroic_charisma          = row[30] ? static_cast<int32_t>(atoi(row[30])) : 0;
			e.heroic_magic_resist      = row[31] ? static_cast<int32_t>(atoi(row[31])) : 0;
			e.heroic_fire_resist       = row[32] ? static_cast<int32_t>(atoi(row[32])) : 0;
			e.heroic_cold_resist       = row[33] ? static_cast<int32_t>(atoi(row[33])) : 0;
			e.heroic_poison_resist     = row[34] ? static_cast<int32_t>(atoi(row[34])) : 0;
			e.heroic_disease_resist    = row[35] ? static_cast<int32_t>(atoi(row[35])) : 0;
			e.heroic_corruption_resist = row[36] ? static_cast<int32_t>(atoi(row[36])) : 0;
			e.haste                    = row[37] ? static_cast<int32_t>(atoi(row[37])) : 0;
			e.accuracy                 = row[38] ? static_cast<int32_t>(atoi(row[38])) : 0;
			e.attack                   = row[39] ? static_cast<int32_t>(atoi(row[39])) : 0;
			e.avoidance                = row[40] ? static_cast<int32_t>(atoi(row[40])) : 0;
			e.clairvoyance             = row[41] ? static_cast<int32_t>(atoi(row[41])) : 0;
			e.combat_effects           = row[42] ? static_cast<int32_t>(atoi(row[42])) : 0;
			e.damage_shield_mitigation = row[43] ? static_cast<int32_t>(atoi(row[43])) : 0;
			e.damage_shield            = row[44] ? static_cast<int32_t>(atoi(row[44])) : 0;
			e.dot_shielding            = row[45] ? static_cast<int32_t>(atoi(row[45])) : 0;
			e.hp_regen                 = row[46] ? static_cast<int32_t>(atoi(row[46])) : 0;
			e.mana_regen               = row[47] ? static_cast<int32_t>(atoi(row[47])) : 0;
			e.endurance_regen          = row[48] ? static_cast<int32_t>(atoi(row[48])) : 0;
			e.shielding                = row[49] ? static_cast<int32_t>(atoi(row[49])) : 0;
			e.spell_damage             = row[50] ? static_cast<int32_t>(atoi(row[50])) : 0;
			e.heal_amount              = row[51] ? static_cast<int32_t>(atoi(row[51])) : 0;
			e.spell_shielding          = row[52] ? static_cast<int32_t>(atoi(row[52])) : 0;
			e.strikethrough            = row[53] ? static_cast<int32_t>(atoi(row[53])) : 0;
			e.stun_resist              = row[54] ? static_cast<int32_t>(atoi(row[54])) : 0;
			e.backstab                 = row[55] ? static_cast<int32_t>(atoi(row[55])) : 0;
			e.wind                     = row[56] ? static_cast<int32_t>(atoi(row[56])) : 0;
			e.brass                    = row[57] ? static_cast<int32_t>(atoi(row[57])) : 0;
			e.string                   = row[58] ? static_cast<int32_t>(atoi(row[58])) : 0;
			e.percussion               = row[59] ? static_cast<int32_t>(atoi(row[59])) : 0;
			e.singing                  = row[60] ? static_cast<int32_t>(atoi(row[60])) : 0;
			e.baking                   = row[61] ? static_cast<int32_t>(atoi(row[61])) : 0;
			e.alchemy                  = row[62] ? static_cast<int32_t>(atoi(row[62])) : 0;
			e.tailoring                = row[63] ? static_cast<int32_t>(atoi(row[63])) : 0;
			e.blacksmithing            = row[64] ? static_cast<int32_t>(atoi(row[64])) : 0;
			e.fletching                = row[65] ? static_cast<int32_t>(atoi(row[65])) : 0;
			e.brewing                  = row[66] ? static_cast<int32_t>(atoi(row[66])) : 0;
			e.jewelry                  = row[67] ? static_cast<int32_t>(atoi(row[67])) : 0;
			e.pottery                  = row[68] ? static_cast<int32_t>(atoi(row[68])) : 0;
			e.research                 = row[69] ? static_cast<int32_t>(atoi(row[69])) : 0;
			e.alcohol                  = row[70] ? static_cast<int32_t>(atoi(row[70])) : 0;
			e.fishing                  = row[71] ? static_cast<int32_t>(atoi(row[71])) : 0;
			e.tinkering                = row[72] ? static_cast<int32_t>(atoi(row[72])) : 0;
			e.created_at               = strtoll(row[73] ? row[73] : "-1", nullptr, 10);
			e.updated_at               = strtoll(row[74] ? row[74] : "-1", nullptr, 10);

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
		const CharacterStatsRecord &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.character_id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.status));
		v.push_back(std::to_string(e.level));
		v.push_back(std::to_string(e.class_));
		v.push_back(std::to_string(e.race));
		v.push_back(std::to_string(e.aa_points));
		v.push_back(std::to_string(e.hp));
		v.push_back(std::to_string(e.mana));
		v.push_back(std::to_string(e.endurance));
		v.push_back(std::to_string(e.ac));
		v.push_back(std::to_string(e.strength));
		v.push_back(std::to_string(e.stamina));
		v.push_back(std::to_string(e.dexterity));
		v.push_back(std::to_string(e.agility));
		v.push_back(std::to_string(e.intelligence));
		v.push_back(std::to_string(e.wisdom));
		v.push_back(std::to_string(e.charisma));
		v.push_back(std::to_string(e.magic_resist));
		v.push_back(std::to_string(e.fire_resist));
		v.push_back(std::to_string(e.cold_resist));
		v.push_back(std::to_string(e.poison_resist));
		v.push_back(std::to_string(e.disease_resist));
		v.push_back(std::to_string(e.corruption_resist));
		v.push_back(std::to_string(e.heroic_strength));
		v.push_back(std::to_string(e.heroic_stamina));
		v.push_back(std::to_string(e.heroic_dexterity));
		v.push_back(std::to_string(e.heroic_agility));
		v.push_back(std::to_string(e.heroic_intelligence));
		v.push_back(std::to_string(e.heroic_wisdom));
		v.push_back(std::to_string(e.heroic_charisma));
		v.push_back(std::to_string(e.heroic_magic_resist));
		v.push_back(std::to_string(e.heroic_fire_resist));
		v.push_back(std::to_string(e.heroic_cold_resist));
		v.push_back(std::to_string(e.heroic_poison_resist));
		v.push_back(std::to_string(e.heroic_disease_resist));
		v.push_back(std::to_string(e.heroic_corruption_resist));
		v.push_back(std::to_string(e.haste));
		v.push_back(std::to_string(e.accuracy));
		v.push_back(std::to_string(e.attack));
		v.push_back(std::to_string(e.avoidance));
		v.push_back(std::to_string(e.clairvoyance));
		v.push_back(std::to_string(e.combat_effects));
		v.push_back(std::to_string(e.damage_shield_mitigation));
		v.push_back(std::to_string(e.damage_shield));
		v.push_back(std::to_string(e.dot_shielding));
		v.push_back(std::to_string(e.hp_regen));
		v.push_back(std::to_string(e.mana_regen));
		v.push_back(std::to_string(e.endurance_regen));
		v.push_back(std::to_string(e.shielding));
		v.push_back(std::to_string(e.spell_damage));
		v.push_back(std::to_string(e.heal_amount));
		v.push_back(std::to_string(e.spell_shielding));
		v.push_back(std::to_string(e.strikethrough));
		v.push_back(std::to_string(e.stun_resist));
		v.push_back(std::to_string(e.backstab));
		v.push_back(std::to_string(e.wind));
		v.push_back(std::to_string(e.brass));
		v.push_back(std::to_string(e.string));
		v.push_back(std::to_string(e.percussion));
		v.push_back(std::to_string(e.singing));
		v.push_back(std::to_string(e.baking));
		v.push_back(std::to_string(e.alchemy));
		v.push_back(std::to_string(e.tailoring));
		v.push_back(std::to_string(e.blacksmithing));
		v.push_back(std::to_string(e.fletching));
		v.push_back(std::to_string(e.brewing));
		v.push_back(std::to_string(e.jewelry));
		v.push_back(std::to_string(e.pottery));
		v.push_back(std::to_string(e.research));
		v.push_back(std::to_string(e.alcohol));
		v.push_back(std::to_string(e.fishing));
		v.push_back(std::to_string(e.tinkering));
		v.push_back("FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");
		v.push_back("FROM_UNIXTIME(" + (e.updated_at > 0 ? std::to_string(e.updated_at) : "null") + ")");

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
		const std::vector<CharacterStatsRecord> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.character_id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.status));
			v.push_back(std::to_string(e.level));
			v.push_back(std::to_string(e.class_));
			v.push_back(std::to_string(e.race));
			v.push_back(std::to_string(e.aa_points));
			v.push_back(std::to_string(e.hp));
			v.push_back(std::to_string(e.mana));
			v.push_back(std::to_string(e.endurance));
			v.push_back(std::to_string(e.ac));
			v.push_back(std::to_string(e.strength));
			v.push_back(std::to_string(e.stamina));
			v.push_back(std::to_string(e.dexterity));
			v.push_back(std::to_string(e.agility));
			v.push_back(std::to_string(e.intelligence));
			v.push_back(std::to_string(e.wisdom));
			v.push_back(std::to_string(e.charisma));
			v.push_back(std::to_string(e.magic_resist));
			v.push_back(std::to_string(e.fire_resist));
			v.push_back(std::to_string(e.cold_resist));
			v.push_back(std::to_string(e.poison_resist));
			v.push_back(std::to_string(e.disease_resist));
			v.push_back(std::to_string(e.corruption_resist));
			v.push_back(std::to_string(e.heroic_strength));
			v.push_back(std::to_string(e.heroic_stamina));
			v.push_back(std::to_string(e.heroic_dexterity));
			v.push_back(std::to_string(e.heroic_agility));
			v.push_back(std::to_string(e.heroic_intelligence));
			v.push_back(std::to_string(e.heroic_wisdom));
			v.push_back(std::to_string(e.heroic_charisma));
			v.push_back(std::to_string(e.heroic_magic_resist));
			v.push_back(std::to_string(e.heroic_fire_resist));
			v.push_back(std::to_string(e.heroic_cold_resist));
			v.push_back(std::to_string(e.heroic_poison_resist));
			v.push_back(std::to_string(e.heroic_disease_resist));
			v.push_back(std::to_string(e.heroic_corruption_resist));
			v.push_back(std::to_string(e.haste));
			v.push_back(std::to_string(e.accuracy));
			v.push_back(std::to_string(e.attack));
			v.push_back(std::to_string(e.avoidance));
			v.push_back(std::to_string(e.clairvoyance));
			v.push_back(std::to_string(e.combat_effects));
			v.push_back(std::to_string(e.damage_shield_mitigation));
			v.push_back(std::to_string(e.damage_shield));
			v.push_back(std::to_string(e.dot_shielding));
			v.push_back(std::to_string(e.hp_regen));
			v.push_back(std::to_string(e.mana_regen));
			v.push_back(std::to_string(e.endurance_regen));
			v.push_back(std::to_string(e.shielding));
			v.push_back(std::to_string(e.spell_damage));
			v.push_back(std::to_string(e.heal_amount));
			v.push_back(std::to_string(e.spell_shielding));
			v.push_back(std::to_string(e.strikethrough));
			v.push_back(std::to_string(e.stun_resist));
			v.push_back(std::to_string(e.backstab));
			v.push_back(std::to_string(e.wind));
			v.push_back(std::to_string(e.brass));
			v.push_back(std::to_string(e.string));
			v.push_back(std::to_string(e.percussion));
			v.push_back(std::to_string(e.singing));
			v.push_back(std::to_string(e.baking));
			v.push_back(std::to_string(e.alchemy));
			v.push_back(std::to_string(e.tailoring));
			v.push_back(std::to_string(e.blacksmithing));
			v.push_back(std::to_string(e.fletching));
			v.push_back(std::to_string(e.brewing));
			v.push_back(std::to_string(e.jewelry));
			v.push_back(std::to_string(e.pottery));
			v.push_back(std::to_string(e.research));
			v.push_back(std::to_string(e.alcohol));
			v.push_back(std::to_string(e.fishing));
			v.push_back(std::to_string(e.tinkering));
			v.push_back("FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");
			v.push_back("FROM_UNIXTIME(" + (e.updated_at > 0 ? std::to_string(e.updated_at) : "null") + ")");

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
