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

class BaseCharacterDataRepository {
public:
	struct CharacterData {
		uint32_t    id;
		int32_t     account_id;
		std::string name;
		std::string last_name;
		std::string title;
		std::string suffix;
		uint32_t    zone_id;
		uint32_t    zone_instance;
		float       y;
		float       x;
		float       z;
		float       heading;
		uint8_t     gender;
		uint16_t    race;
		uint8_t     class_;
		uint32_t    level;
		uint32_t    deity;
		uint32_t    birthday;
		uint32_t    last_login;
		uint32_t    time_played;
		uint8_t     level2;
		uint8_t     anon;
		uint8_t     gm;
		uint32_t    face;
		uint8_t     hair_color;
		uint8_t     hair_style;
		uint8_t     beard;
		uint8_t     beard_color;
		uint8_t     eye_color_1;
		uint8_t     eye_color_2;
		uint32_t    drakkin_heritage;
		uint32_t    drakkin_tattoo;
		uint32_t    drakkin_details;
		uint8_t     ability_time_seconds;
		uint8_t     ability_number;
		uint8_t     ability_time_minutes;
		uint8_t     ability_time_hours;
		uint32_t    exp;
		uint8_t     exp_enabled;
		uint32_t    aa_points_spent;
		uint32_t    aa_exp;
		uint32_t    aa_points;
		uint32_t    group_leadership_exp;
		uint32_t    raid_leadership_exp;
		uint32_t    group_leadership_points;
		uint32_t    raid_leadership_points;
		uint32_t    points;
		uint32_t    cur_hp;
		uint32_t    mana;
		uint32_t    endurance;
		uint32_t    intoxication;
		uint32_t    str;
		uint32_t    sta;
		uint32_t    cha;
		uint32_t    dex;
		uint32_t    int_;
		uint32_t    agi;
		uint32_t    wis;
		int32_t     extra_haste;
		uint32_t    zone_change_count;
		uint32_t    toxicity;
		uint32_t    hunger_level;
		uint32_t    thirst_level;
		uint32_t    ability_up;
		uint32_t    ldon_points_guk;
		uint32_t    ldon_points_mir;
		uint32_t    ldon_points_mmc;
		uint32_t    ldon_points_ruj;
		uint32_t    ldon_points_tak;
		uint32_t    ldon_points_available;
		uint32_t    tribute_time_remaining;
		uint32_t    career_tribute_points;
		uint32_t    tribute_points;
		uint32_t    tribute_active;
		uint8_t     pvp_status;
		uint32_t    pvp_kills;
		uint32_t    pvp_deaths;
		uint32_t    pvp_current_points;
		uint32_t    pvp_career_points;
		uint32_t    pvp_best_kill_streak;
		uint32_t    pvp_worst_death_streak;
		uint32_t    pvp_current_kill_streak;
		uint32_t    pvp2;
		uint32_t    pvp_type;
		uint32_t    show_helm;
		uint8_t     group_auto_consent;
		uint8_t     raid_auto_consent;
		uint8_t     guild_auto_consent;
		uint8_t     leadership_exp_on;
		uint32_t    RestTimer;
		uint32_t    air_remaining;
		uint32_t    autosplit_enabled;
		uint8_t     lfp;
		uint8_t     lfg;
		std::string mailkey;
		uint8_t     xtargets;
		uint8_t     ingame;
		uint32_t    first_login;
		uint32_t    e_aa_effects;
		uint32_t    e_percent_to_aa;
		uint32_t    e_expended_aa_spent;
		uint32_t    aa_points_spent_old;
		uint32_t    aa_points_old;
		uint32_t    e_last_invsnapshot;
		time_t      deleted_at;
		uint8_t     illusion_block;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"account_id",
			"name",
			"last_name",
			"title",
			"suffix",
			"zone_id",
			"zone_instance",
			"y",
			"x",
			"z",
			"heading",
			"gender",
			"race",
			"`class`",
			"level",
			"deity",
			"birthday",
			"last_login",
			"time_played",
			"level2",
			"anon",
			"gm",
			"face",
			"hair_color",
			"hair_style",
			"beard",
			"beard_color",
			"eye_color_1",
			"eye_color_2",
			"drakkin_heritage",
			"drakkin_tattoo",
			"drakkin_details",
			"ability_time_seconds",
			"ability_number",
			"ability_time_minutes",
			"ability_time_hours",
			"exp",
			"exp_enabled",
			"aa_points_spent",
			"aa_exp",
			"aa_points",
			"group_leadership_exp",
			"raid_leadership_exp",
			"group_leadership_points",
			"raid_leadership_points",
			"points",
			"cur_hp",
			"mana",
			"endurance",
			"intoxication",
			"str",
			"sta",
			"cha",
			"dex",
			"`int`",
			"agi",
			"wis",
			"extra_haste",
			"zone_change_count",
			"toxicity",
			"hunger_level",
			"thirst_level",
			"ability_up",
			"ldon_points_guk",
			"ldon_points_mir",
			"ldon_points_mmc",
			"ldon_points_ruj",
			"ldon_points_tak",
			"ldon_points_available",
			"tribute_time_remaining",
			"career_tribute_points",
			"tribute_points",
			"tribute_active",
			"pvp_status",
			"pvp_kills",
			"pvp_deaths",
			"pvp_current_points",
			"pvp_career_points",
			"pvp_best_kill_streak",
			"pvp_worst_death_streak",
			"pvp_current_kill_streak",
			"pvp2",
			"pvp_type",
			"show_helm",
			"group_auto_consent",
			"raid_auto_consent",
			"guild_auto_consent",
			"leadership_exp_on",
			"RestTimer",
			"air_remaining",
			"autosplit_enabled",
			"lfp",
			"lfg",
			"mailkey",
			"xtargets",
			"ingame",
			"first_login",
			"e_aa_effects",
			"e_percent_to_aa",
			"e_expended_aa_spent",
			"aa_points_spent_old",
			"aa_points_old",
			"e_last_invsnapshot",
			"deleted_at",
			"illusion_block",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"account_id",
			"name",
			"last_name",
			"title",
			"suffix",
			"zone_id",
			"zone_instance",
			"y",
			"x",
			"z",
			"heading",
			"gender",
			"race",
			"`class`",
			"level",
			"deity",
			"birthday",
			"last_login",
			"time_played",
			"level2",
			"anon",
			"gm",
			"face",
			"hair_color",
			"hair_style",
			"beard",
			"beard_color",
			"eye_color_1",
			"eye_color_2",
			"drakkin_heritage",
			"drakkin_tattoo",
			"drakkin_details",
			"ability_time_seconds",
			"ability_number",
			"ability_time_minutes",
			"ability_time_hours",
			"exp",
			"exp_enabled",
			"aa_points_spent",
			"aa_exp",
			"aa_points",
			"group_leadership_exp",
			"raid_leadership_exp",
			"group_leadership_points",
			"raid_leadership_points",
			"points",
			"cur_hp",
			"mana",
			"endurance",
			"intoxication",
			"str",
			"sta",
			"cha",
			"dex",
			"`int`",
			"agi",
			"wis",
			"extra_haste",
			"zone_change_count",
			"toxicity",
			"hunger_level",
			"thirst_level",
			"ability_up",
			"ldon_points_guk",
			"ldon_points_mir",
			"ldon_points_mmc",
			"ldon_points_ruj",
			"ldon_points_tak",
			"ldon_points_available",
			"tribute_time_remaining",
			"career_tribute_points",
			"tribute_points",
			"tribute_active",
			"pvp_status",
			"pvp_kills",
			"pvp_deaths",
			"pvp_current_points",
			"pvp_career_points",
			"pvp_best_kill_streak",
			"pvp_worst_death_streak",
			"pvp_current_kill_streak",
			"pvp2",
			"pvp_type",
			"show_helm",
			"group_auto_consent",
			"raid_auto_consent",
			"guild_auto_consent",
			"leadership_exp_on",
			"RestTimer",
			"air_remaining",
			"autosplit_enabled",
			"lfp",
			"lfg",
			"mailkey",
			"xtargets",
			"ingame",
			"first_login",
			"e_aa_effects",
			"e_percent_to_aa",
			"e_expended_aa_spent",
			"aa_points_spent_old",
			"aa_points_old",
			"e_last_invsnapshot",
			"UNIX_TIMESTAMP(deleted_at)",
			"illusion_block",
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
		return std::string("character_data");
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

	static CharacterData NewEntity()
	{
		CharacterData e{};

		e.id                      = 0;
		e.account_id              = 0;
		e.name                    = "";
		e.last_name               = "";
		e.title                   = "";
		e.suffix                  = "";
		e.zone_id                 = 0;
		e.zone_instance           = 0;
		e.y                       = 0;
		e.x                       = 0;
		e.z                       = 0;
		e.heading                 = 0;
		e.gender                  = 0;
		e.race                    = 0;
		e.class_                  = 0;
		e.level                   = 0;
		e.deity                   = 0;
		e.birthday                = 0;
		e.last_login              = 0;
		e.time_played             = 0;
		e.level2                  = 0;
		e.anon                    = 0;
		e.gm                      = 0;
		e.face                    = 0;
		e.hair_color              = 0;
		e.hair_style              = 0;
		e.beard                   = 0;
		e.beard_color             = 0;
		e.eye_color_1             = 0;
		e.eye_color_2             = 0;
		e.drakkin_heritage        = 0;
		e.drakkin_tattoo          = 0;
		e.drakkin_details         = 0;
		e.ability_time_seconds    = 0;
		e.ability_number          = 0;
		e.ability_time_minutes    = 0;
		e.ability_time_hours      = 0;
		e.exp                     = 0;
		e.exp_enabled             = 1;
		e.aa_points_spent         = 0;
		e.aa_exp                  = 0;
		e.aa_points               = 0;
		e.group_leadership_exp    = 0;
		e.raid_leadership_exp     = 0;
		e.group_leadership_points = 0;
		e.raid_leadership_points  = 0;
		e.points                  = 0;
		e.cur_hp                  = 0;
		e.mana                    = 0;
		e.endurance               = 0;
		e.intoxication            = 0;
		e.str                     = 0;
		e.sta                     = 0;
		e.cha                     = 0;
		e.dex                     = 0;
		e.int_                    = 0;
		e.agi                     = 0;
		e.wis                     = 0;
		e.extra_haste             = 0;
		e.zone_change_count       = 0;
		e.toxicity                = 0;
		e.hunger_level            = 0;
		e.thirst_level            = 0;
		e.ability_up              = 0;
		e.ldon_points_guk         = 0;
		e.ldon_points_mir         = 0;
		e.ldon_points_mmc         = 0;
		e.ldon_points_ruj         = 0;
		e.ldon_points_tak         = 0;
		e.ldon_points_available   = 0;
		e.tribute_time_remaining  = 0;
		e.career_tribute_points   = 0;
		e.tribute_points          = 0;
		e.tribute_active          = 0;
		e.pvp_status              = 0;
		e.pvp_kills               = 0;
		e.pvp_deaths              = 0;
		e.pvp_current_points      = 0;
		e.pvp_career_points       = 0;
		e.pvp_best_kill_streak    = 0;
		e.pvp_worst_death_streak  = 0;
		e.pvp_current_kill_streak = 0;
		e.pvp2                    = 0;
		e.pvp_type                = 0;
		e.show_helm               = 0;
		e.group_auto_consent      = 0;
		e.raid_auto_consent       = 0;
		e.guild_auto_consent      = 0;
		e.leadership_exp_on       = 0;
		e.RestTimer               = 0;
		e.air_remaining           = 0;
		e.autosplit_enabled       = 0;
		e.lfp                     = 0;
		e.lfg                     = 0;
		e.mailkey                 = "";
		e.xtargets                = 5;
		e.ingame                  = 0;
		e.first_login             = 0;
		e.e_aa_effects            = 0;
		e.e_percent_to_aa         = 0;
		e.e_expended_aa_spent     = 0;
		e.aa_points_spent_old     = 0;
		e.aa_points_old           = 0;
		e.e_last_invsnapshot      = 0;
		e.deleted_at              = 0;
		e.illusion_block          = 0;

		return e;
	}

	static CharacterData GetCharacterData(
		const std::vector<CharacterData> &character_datas,
		int character_data_id
	)
	{
		for (auto &character_data : character_datas) {
			if (character_data.id == character_data_id) {
				return character_data;
			}
		}

		return NewEntity();
	}

	static CharacterData FindOne(
		Database& db,
		int character_data_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				character_data_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterData e{};

			e.id                      = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.account_id              = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.name                    = row[2] ? row[2] : "";
			e.last_name               = row[3] ? row[3] : "";
			e.title                   = row[4] ? row[4] : "";
			e.suffix                  = row[5] ? row[5] : "";
			e.zone_id                 = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.zone_instance           = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.y                       = row[8] ? strtof(row[8], nullptr) : 0;
			e.x                       = row[9] ? strtof(row[9], nullptr) : 0;
			e.z                       = row[10] ? strtof(row[10], nullptr) : 0;
			e.heading                 = row[11] ? strtof(row[11], nullptr) : 0;
			e.gender                  = row[12] ? static_cast<uint8_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.race                    = row[13] ? static_cast<uint16_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.class_                  = row[14] ? static_cast<uint8_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.level                   = row[15] ? static_cast<uint32_t>(strtoul(row[15], nullptr, 10)) : 0;
			e.deity                   = row[16] ? static_cast<uint32_t>(strtoul(row[16], nullptr, 10)) : 0;
			e.birthday                = row[17] ? static_cast<uint32_t>(strtoul(row[17], nullptr, 10)) : 0;
			e.last_login              = row[18] ? static_cast<uint32_t>(strtoul(row[18], nullptr, 10)) : 0;
			e.time_played             = row[19] ? static_cast<uint32_t>(strtoul(row[19], nullptr, 10)) : 0;
			e.level2                  = row[20] ? static_cast<uint8_t>(strtoul(row[20], nullptr, 10)) : 0;
			e.anon                    = row[21] ? static_cast<uint8_t>(strtoul(row[21], nullptr, 10)) : 0;
			e.gm                      = row[22] ? static_cast<uint8_t>(strtoul(row[22], nullptr, 10)) : 0;
			e.face                    = row[23] ? static_cast<uint32_t>(strtoul(row[23], nullptr, 10)) : 0;
			e.hair_color              = row[24] ? static_cast<uint8_t>(strtoul(row[24], nullptr, 10)) : 0;
			e.hair_style              = row[25] ? static_cast<uint8_t>(strtoul(row[25], nullptr, 10)) : 0;
			e.beard                   = row[26] ? static_cast<uint8_t>(strtoul(row[26], nullptr, 10)) : 0;
			e.beard_color             = row[27] ? static_cast<uint8_t>(strtoul(row[27], nullptr, 10)) : 0;
			e.eye_color_1             = row[28] ? static_cast<uint8_t>(strtoul(row[28], nullptr, 10)) : 0;
			e.eye_color_2             = row[29] ? static_cast<uint8_t>(strtoul(row[29], nullptr, 10)) : 0;
			e.drakkin_heritage        = row[30] ? static_cast<uint32_t>(strtoul(row[30], nullptr, 10)) : 0;
			e.drakkin_tattoo          = row[31] ? static_cast<uint32_t>(strtoul(row[31], nullptr, 10)) : 0;
			e.drakkin_details         = row[32] ? static_cast<uint32_t>(strtoul(row[32], nullptr, 10)) : 0;
			e.ability_time_seconds    = row[33] ? static_cast<uint8_t>(strtoul(row[33], nullptr, 10)) : 0;
			e.ability_number          = row[34] ? static_cast<uint8_t>(strtoul(row[34], nullptr, 10)) : 0;
			e.ability_time_minutes    = row[35] ? static_cast<uint8_t>(strtoul(row[35], nullptr, 10)) : 0;
			e.ability_time_hours      = row[36] ? static_cast<uint8_t>(strtoul(row[36], nullptr, 10)) : 0;
			e.exp                     = row[37] ? static_cast<uint32_t>(strtoul(row[37], nullptr, 10)) : 0;
			e.exp_enabled             = row[38] ? static_cast<uint8_t>(strtoul(row[38], nullptr, 10)) : 1;
			e.aa_points_spent         = row[39] ? static_cast<uint32_t>(strtoul(row[39], nullptr, 10)) : 0;
			e.aa_exp                  = row[40] ? static_cast<uint32_t>(strtoul(row[40], nullptr, 10)) : 0;
			e.aa_points               = row[41] ? static_cast<uint32_t>(strtoul(row[41], nullptr, 10)) : 0;
			e.group_leadership_exp    = row[42] ? static_cast<uint32_t>(strtoul(row[42], nullptr, 10)) : 0;
			e.raid_leadership_exp     = row[43] ? static_cast<uint32_t>(strtoul(row[43], nullptr, 10)) : 0;
			e.group_leadership_points = row[44] ? static_cast<uint32_t>(strtoul(row[44], nullptr, 10)) : 0;
			e.raid_leadership_points  = row[45] ? static_cast<uint32_t>(strtoul(row[45], nullptr, 10)) : 0;
			e.points                  = row[46] ? static_cast<uint32_t>(strtoul(row[46], nullptr, 10)) : 0;
			e.cur_hp                  = row[47] ? static_cast<uint32_t>(strtoul(row[47], nullptr, 10)) : 0;
			e.mana                    = row[48] ? static_cast<uint32_t>(strtoul(row[48], nullptr, 10)) : 0;
			e.endurance               = row[49] ? static_cast<uint32_t>(strtoul(row[49], nullptr, 10)) : 0;
			e.intoxication            = row[50] ? static_cast<uint32_t>(strtoul(row[50], nullptr, 10)) : 0;
			e.str                     = row[51] ? static_cast<uint32_t>(strtoul(row[51], nullptr, 10)) : 0;
			e.sta                     = row[52] ? static_cast<uint32_t>(strtoul(row[52], nullptr, 10)) : 0;
			e.cha                     = row[53] ? static_cast<uint32_t>(strtoul(row[53], nullptr, 10)) : 0;
			e.dex                     = row[54] ? static_cast<uint32_t>(strtoul(row[54], nullptr, 10)) : 0;
			e.int_                    = row[55] ? static_cast<uint32_t>(strtoul(row[55], nullptr, 10)) : 0;
			e.agi                     = row[56] ? static_cast<uint32_t>(strtoul(row[56], nullptr, 10)) : 0;
			e.wis                     = row[57] ? static_cast<uint32_t>(strtoul(row[57], nullptr, 10)) : 0;
			e.extra_haste             = row[58] ? static_cast<int32_t>(atoi(row[58])) : 0;
			e.zone_change_count       = row[59] ? static_cast<uint32_t>(strtoul(row[59], nullptr, 10)) : 0;
			e.toxicity                = row[60] ? static_cast<uint32_t>(strtoul(row[60], nullptr, 10)) : 0;
			e.hunger_level            = row[61] ? static_cast<uint32_t>(strtoul(row[61], nullptr, 10)) : 0;
			e.thirst_level            = row[62] ? static_cast<uint32_t>(strtoul(row[62], nullptr, 10)) : 0;
			e.ability_up              = row[63] ? static_cast<uint32_t>(strtoul(row[63], nullptr, 10)) : 0;
			e.ldon_points_guk         = row[64] ? static_cast<uint32_t>(strtoul(row[64], nullptr, 10)) : 0;
			e.ldon_points_mir         = row[65] ? static_cast<uint32_t>(strtoul(row[65], nullptr, 10)) : 0;
			e.ldon_points_mmc         = row[66] ? static_cast<uint32_t>(strtoul(row[66], nullptr, 10)) : 0;
			e.ldon_points_ruj         = row[67] ? static_cast<uint32_t>(strtoul(row[67], nullptr, 10)) : 0;
			e.ldon_points_tak         = row[68] ? static_cast<uint32_t>(strtoul(row[68], nullptr, 10)) : 0;
			e.ldon_points_available   = row[69] ? static_cast<uint32_t>(strtoul(row[69], nullptr, 10)) : 0;
			e.tribute_time_remaining  = row[70] ? static_cast<uint32_t>(strtoul(row[70], nullptr, 10)) : 0;
			e.career_tribute_points   = row[71] ? static_cast<uint32_t>(strtoul(row[71], nullptr, 10)) : 0;
			e.tribute_points          = row[72] ? static_cast<uint32_t>(strtoul(row[72], nullptr, 10)) : 0;
			e.tribute_active          = row[73] ? static_cast<uint32_t>(strtoul(row[73], nullptr, 10)) : 0;
			e.pvp_status              = row[74] ? static_cast<uint8_t>(strtoul(row[74], nullptr, 10)) : 0;
			e.pvp_kills               = row[75] ? static_cast<uint32_t>(strtoul(row[75], nullptr, 10)) : 0;
			e.pvp_deaths              = row[76] ? static_cast<uint32_t>(strtoul(row[76], nullptr, 10)) : 0;
			e.pvp_current_points      = row[77] ? static_cast<uint32_t>(strtoul(row[77], nullptr, 10)) : 0;
			e.pvp_career_points       = row[78] ? static_cast<uint32_t>(strtoul(row[78], nullptr, 10)) : 0;
			e.pvp_best_kill_streak    = row[79] ? static_cast<uint32_t>(strtoul(row[79], nullptr, 10)) : 0;
			e.pvp_worst_death_streak  = row[80] ? static_cast<uint32_t>(strtoul(row[80], nullptr, 10)) : 0;
			e.pvp_current_kill_streak = row[81] ? static_cast<uint32_t>(strtoul(row[81], nullptr, 10)) : 0;
			e.pvp2                    = row[82] ? static_cast<uint32_t>(strtoul(row[82], nullptr, 10)) : 0;
			e.pvp_type                = row[83] ? static_cast<uint32_t>(strtoul(row[83], nullptr, 10)) : 0;
			e.show_helm               = row[84] ? static_cast<uint32_t>(strtoul(row[84], nullptr, 10)) : 0;
			e.group_auto_consent      = row[85] ? static_cast<uint8_t>(strtoul(row[85], nullptr, 10)) : 0;
			e.raid_auto_consent       = row[86] ? static_cast<uint8_t>(strtoul(row[86], nullptr, 10)) : 0;
			e.guild_auto_consent      = row[87] ? static_cast<uint8_t>(strtoul(row[87], nullptr, 10)) : 0;
			e.leadership_exp_on       = row[88] ? static_cast<uint8_t>(strtoul(row[88], nullptr, 10)) : 0;
			e.RestTimer               = row[89] ? static_cast<uint32_t>(strtoul(row[89], nullptr, 10)) : 0;
			e.air_remaining           = row[90] ? static_cast<uint32_t>(strtoul(row[90], nullptr, 10)) : 0;
			e.autosplit_enabled       = row[91] ? static_cast<uint32_t>(strtoul(row[91], nullptr, 10)) : 0;
			e.lfp                     = row[92] ? static_cast<uint8_t>(strtoul(row[92], nullptr, 10)) : 0;
			e.lfg                     = row[93] ? static_cast<uint8_t>(strtoul(row[93], nullptr, 10)) : 0;
			e.mailkey                 = row[94] ? row[94] : "";
			e.xtargets                = row[95] ? static_cast<uint8_t>(strtoul(row[95], nullptr, 10)) : 5;
			e.ingame                  = row[96] ? static_cast<uint8_t>(strtoul(row[96], nullptr, 10)) : 0;
			e.first_login             = row[97] ? static_cast<uint32_t>(strtoul(row[97], nullptr, 10)) : 0;
			e.e_aa_effects            = row[98] ? static_cast<uint32_t>(strtoul(row[98], nullptr, 10)) : 0;
			e.e_percent_to_aa         = row[99] ? static_cast<uint32_t>(strtoul(row[99], nullptr, 10)) : 0;
			e.e_expended_aa_spent     = row[100] ? static_cast<uint32_t>(strtoul(row[100], nullptr, 10)) : 0;
			e.aa_points_spent_old     = row[101] ? static_cast<uint32_t>(strtoul(row[101], nullptr, 10)) : 0;
			e.aa_points_old           = row[102] ? static_cast<uint32_t>(strtoul(row[102], nullptr, 10)) : 0;
			e.e_last_invsnapshot      = row[103] ? static_cast<uint32_t>(strtoul(row[103], nullptr, 10)) : 0;
			e.deleted_at              = strtoll(row[104] ? row[104] : "-1", nullptr, 10);
			e.illusion_block          = row[105] ? static_cast<uint8_t>(strtoul(row[105], nullptr, 10)) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_data_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_data_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CharacterData &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.account_id));
		v.push_back(columns[2] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[3] + " = '" + Strings::Escape(e.last_name) + "'");
		v.push_back(columns[4] + " = '" + Strings::Escape(e.title) + "'");
		v.push_back(columns[5] + " = '" + Strings::Escape(e.suffix) + "'");
		v.push_back(columns[6] + " = " + std::to_string(e.zone_id));
		v.push_back(columns[7] + " = " + std::to_string(e.zone_instance));
		v.push_back(columns[8] + " = " + std::to_string(e.y));
		v.push_back(columns[9] + " = " + std::to_string(e.x));
		v.push_back(columns[10] + " = " + std::to_string(e.z));
		v.push_back(columns[11] + " = " + std::to_string(e.heading));
		v.push_back(columns[12] + " = " + std::to_string(e.gender));
		v.push_back(columns[13] + " = " + std::to_string(e.race));
		v.push_back(columns[14] + " = " + std::to_string(e.class_));
		v.push_back(columns[15] + " = " + std::to_string(e.level));
		v.push_back(columns[16] + " = " + std::to_string(e.deity));
		v.push_back(columns[17] + " = " + std::to_string(e.birthday));
		v.push_back(columns[18] + " = " + std::to_string(e.last_login));
		v.push_back(columns[19] + " = " + std::to_string(e.time_played));
		v.push_back(columns[20] + " = " + std::to_string(e.level2));
		v.push_back(columns[21] + " = " + std::to_string(e.anon));
		v.push_back(columns[22] + " = " + std::to_string(e.gm));
		v.push_back(columns[23] + " = " + std::to_string(e.face));
		v.push_back(columns[24] + " = " + std::to_string(e.hair_color));
		v.push_back(columns[25] + " = " + std::to_string(e.hair_style));
		v.push_back(columns[26] + " = " + std::to_string(e.beard));
		v.push_back(columns[27] + " = " + std::to_string(e.beard_color));
		v.push_back(columns[28] + " = " + std::to_string(e.eye_color_1));
		v.push_back(columns[29] + " = " + std::to_string(e.eye_color_2));
		v.push_back(columns[30] + " = " + std::to_string(e.drakkin_heritage));
		v.push_back(columns[31] + " = " + std::to_string(e.drakkin_tattoo));
		v.push_back(columns[32] + " = " + std::to_string(e.drakkin_details));
		v.push_back(columns[33] + " = " + std::to_string(e.ability_time_seconds));
		v.push_back(columns[34] + " = " + std::to_string(e.ability_number));
		v.push_back(columns[35] + " = " + std::to_string(e.ability_time_minutes));
		v.push_back(columns[36] + " = " + std::to_string(e.ability_time_hours));
		v.push_back(columns[37] + " = " + std::to_string(e.exp));
		v.push_back(columns[38] + " = " + std::to_string(e.exp_enabled));
		v.push_back(columns[39] + " = " + std::to_string(e.aa_points_spent));
		v.push_back(columns[40] + " = " + std::to_string(e.aa_exp));
		v.push_back(columns[41] + " = " + std::to_string(e.aa_points));
		v.push_back(columns[42] + " = " + std::to_string(e.group_leadership_exp));
		v.push_back(columns[43] + " = " + std::to_string(e.raid_leadership_exp));
		v.push_back(columns[44] + " = " + std::to_string(e.group_leadership_points));
		v.push_back(columns[45] + " = " + std::to_string(e.raid_leadership_points));
		v.push_back(columns[46] + " = " + std::to_string(e.points));
		v.push_back(columns[47] + " = " + std::to_string(e.cur_hp));
		v.push_back(columns[48] + " = " + std::to_string(e.mana));
		v.push_back(columns[49] + " = " + std::to_string(e.endurance));
		v.push_back(columns[50] + " = " + std::to_string(e.intoxication));
		v.push_back(columns[51] + " = " + std::to_string(e.str));
		v.push_back(columns[52] + " = " + std::to_string(e.sta));
		v.push_back(columns[53] + " = " + std::to_string(e.cha));
		v.push_back(columns[54] + " = " + std::to_string(e.dex));
		v.push_back(columns[55] + " = " + std::to_string(e.int_));
		v.push_back(columns[56] + " = " + std::to_string(e.agi));
		v.push_back(columns[57] + " = " + std::to_string(e.wis));
		v.push_back(columns[58] + " = " + std::to_string(e.extra_haste));
		v.push_back(columns[59] + " = " + std::to_string(e.zone_change_count));
		v.push_back(columns[60] + " = " + std::to_string(e.toxicity));
		v.push_back(columns[61] + " = " + std::to_string(e.hunger_level));
		v.push_back(columns[62] + " = " + std::to_string(e.thirst_level));
		v.push_back(columns[63] + " = " + std::to_string(e.ability_up));
		v.push_back(columns[64] + " = " + std::to_string(e.ldon_points_guk));
		v.push_back(columns[65] + " = " + std::to_string(e.ldon_points_mir));
		v.push_back(columns[66] + " = " + std::to_string(e.ldon_points_mmc));
		v.push_back(columns[67] + " = " + std::to_string(e.ldon_points_ruj));
		v.push_back(columns[68] + " = " + std::to_string(e.ldon_points_tak));
		v.push_back(columns[69] + " = " + std::to_string(e.ldon_points_available));
		v.push_back(columns[70] + " = " + std::to_string(e.tribute_time_remaining));
		v.push_back(columns[71] + " = " + std::to_string(e.career_tribute_points));
		v.push_back(columns[72] + " = " + std::to_string(e.tribute_points));
		v.push_back(columns[73] + " = " + std::to_string(e.tribute_active));
		v.push_back(columns[74] + " = " + std::to_string(e.pvp_status));
		v.push_back(columns[75] + " = " + std::to_string(e.pvp_kills));
		v.push_back(columns[76] + " = " + std::to_string(e.pvp_deaths));
		v.push_back(columns[77] + " = " + std::to_string(e.pvp_current_points));
		v.push_back(columns[78] + " = " + std::to_string(e.pvp_career_points));
		v.push_back(columns[79] + " = " + std::to_string(e.pvp_best_kill_streak));
		v.push_back(columns[80] + " = " + std::to_string(e.pvp_worst_death_streak));
		v.push_back(columns[81] + " = " + std::to_string(e.pvp_current_kill_streak));
		v.push_back(columns[82] + " = " + std::to_string(e.pvp2));
		v.push_back(columns[83] + " = " + std::to_string(e.pvp_type));
		v.push_back(columns[84] + " = " + std::to_string(e.show_helm));
		v.push_back(columns[85] + " = " + std::to_string(e.group_auto_consent));
		v.push_back(columns[86] + " = " + std::to_string(e.raid_auto_consent));
		v.push_back(columns[87] + " = " + std::to_string(e.guild_auto_consent));
		v.push_back(columns[88] + " = " + std::to_string(e.leadership_exp_on));
		v.push_back(columns[89] + " = " + std::to_string(e.RestTimer));
		v.push_back(columns[90] + " = " + std::to_string(e.air_remaining));
		v.push_back(columns[91] + " = " + std::to_string(e.autosplit_enabled));
		v.push_back(columns[92] + " = " + std::to_string(e.lfp));
		v.push_back(columns[93] + " = " + std::to_string(e.lfg));
		v.push_back(columns[94] + " = '" + Strings::Escape(e.mailkey) + "'");
		v.push_back(columns[95] + " = " + std::to_string(e.xtargets));
		v.push_back(columns[96] + " = " + std::to_string(e.ingame));
		v.push_back(columns[97] + " = " + std::to_string(e.first_login));
		v.push_back(columns[98] + " = " + std::to_string(e.e_aa_effects));
		v.push_back(columns[99] + " = " + std::to_string(e.e_percent_to_aa));
		v.push_back(columns[100] + " = " + std::to_string(e.e_expended_aa_spent));
		v.push_back(columns[101] + " = " + std::to_string(e.aa_points_spent_old));
		v.push_back(columns[102] + " = " + std::to_string(e.aa_points_old));
		v.push_back(columns[103] + " = " + std::to_string(e.e_last_invsnapshot));
		v.push_back(columns[104] + " = FROM_UNIXTIME(" + (e.deleted_at > 0 ? std::to_string(e.deleted_at) : "null") + ")");
		v.push_back(columns[105] + " = " + std::to_string(e.illusion_block));

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

	static CharacterData InsertOne(
		Database& db,
		CharacterData e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.account_id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back("'" + Strings::Escape(e.last_name) + "'");
		v.push_back("'" + Strings::Escape(e.title) + "'");
		v.push_back("'" + Strings::Escape(e.suffix) + "'");
		v.push_back(std::to_string(e.zone_id));
		v.push_back(std::to_string(e.zone_instance));
		v.push_back(std::to_string(e.y));
		v.push_back(std::to_string(e.x));
		v.push_back(std::to_string(e.z));
		v.push_back(std::to_string(e.heading));
		v.push_back(std::to_string(e.gender));
		v.push_back(std::to_string(e.race));
		v.push_back(std::to_string(e.class_));
		v.push_back(std::to_string(e.level));
		v.push_back(std::to_string(e.deity));
		v.push_back(std::to_string(e.birthday));
		v.push_back(std::to_string(e.last_login));
		v.push_back(std::to_string(e.time_played));
		v.push_back(std::to_string(e.level2));
		v.push_back(std::to_string(e.anon));
		v.push_back(std::to_string(e.gm));
		v.push_back(std::to_string(e.face));
		v.push_back(std::to_string(e.hair_color));
		v.push_back(std::to_string(e.hair_style));
		v.push_back(std::to_string(e.beard));
		v.push_back(std::to_string(e.beard_color));
		v.push_back(std::to_string(e.eye_color_1));
		v.push_back(std::to_string(e.eye_color_2));
		v.push_back(std::to_string(e.drakkin_heritage));
		v.push_back(std::to_string(e.drakkin_tattoo));
		v.push_back(std::to_string(e.drakkin_details));
		v.push_back(std::to_string(e.ability_time_seconds));
		v.push_back(std::to_string(e.ability_number));
		v.push_back(std::to_string(e.ability_time_minutes));
		v.push_back(std::to_string(e.ability_time_hours));
		v.push_back(std::to_string(e.exp));
		v.push_back(std::to_string(e.exp_enabled));
		v.push_back(std::to_string(e.aa_points_spent));
		v.push_back(std::to_string(e.aa_exp));
		v.push_back(std::to_string(e.aa_points));
		v.push_back(std::to_string(e.group_leadership_exp));
		v.push_back(std::to_string(e.raid_leadership_exp));
		v.push_back(std::to_string(e.group_leadership_points));
		v.push_back(std::to_string(e.raid_leadership_points));
		v.push_back(std::to_string(e.points));
		v.push_back(std::to_string(e.cur_hp));
		v.push_back(std::to_string(e.mana));
		v.push_back(std::to_string(e.endurance));
		v.push_back(std::to_string(e.intoxication));
		v.push_back(std::to_string(e.str));
		v.push_back(std::to_string(e.sta));
		v.push_back(std::to_string(e.cha));
		v.push_back(std::to_string(e.dex));
		v.push_back(std::to_string(e.int_));
		v.push_back(std::to_string(e.agi));
		v.push_back(std::to_string(e.wis));
		v.push_back(std::to_string(e.extra_haste));
		v.push_back(std::to_string(e.zone_change_count));
		v.push_back(std::to_string(e.toxicity));
		v.push_back(std::to_string(e.hunger_level));
		v.push_back(std::to_string(e.thirst_level));
		v.push_back(std::to_string(e.ability_up));
		v.push_back(std::to_string(e.ldon_points_guk));
		v.push_back(std::to_string(e.ldon_points_mir));
		v.push_back(std::to_string(e.ldon_points_mmc));
		v.push_back(std::to_string(e.ldon_points_ruj));
		v.push_back(std::to_string(e.ldon_points_tak));
		v.push_back(std::to_string(e.ldon_points_available));
		v.push_back(std::to_string(e.tribute_time_remaining));
		v.push_back(std::to_string(e.career_tribute_points));
		v.push_back(std::to_string(e.tribute_points));
		v.push_back(std::to_string(e.tribute_active));
		v.push_back(std::to_string(e.pvp_status));
		v.push_back(std::to_string(e.pvp_kills));
		v.push_back(std::to_string(e.pvp_deaths));
		v.push_back(std::to_string(e.pvp_current_points));
		v.push_back(std::to_string(e.pvp_career_points));
		v.push_back(std::to_string(e.pvp_best_kill_streak));
		v.push_back(std::to_string(e.pvp_worst_death_streak));
		v.push_back(std::to_string(e.pvp_current_kill_streak));
		v.push_back(std::to_string(e.pvp2));
		v.push_back(std::to_string(e.pvp_type));
		v.push_back(std::to_string(e.show_helm));
		v.push_back(std::to_string(e.group_auto_consent));
		v.push_back(std::to_string(e.raid_auto_consent));
		v.push_back(std::to_string(e.guild_auto_consent));
		v.push_back(std::to_string(e.leadership_exp_on));
		v.push_back(std::to_string(e.RestTimer));
		v.push_back(std::to_string(e.air_remaining));
		v.push_back(std::to_string(e.autosplit_enabled));
		v.push_back(std::to_string(e.lfp));
		v.push_back(std::to_string(e.lfg));
		v.push_back("'" + Strings::Escape(e.mailkey) + "'");
		v.push_back(std::to_string(e.xtargets));
		v.push_back(std::to_string(e.ingame));
		v.push_back(std::to_string(e.first_login));
		v.push_back(std::to_string(e.e_aa_effects));
		v.push_back(std::to_string(e.e_percent_to_aa));
		v.push_back(std::to_string(e.e_expended_aa_spent));
		v.push_back(std::to_string(e.aa_points_spent_old));
		v.push_back(std::to_string(e.aa_points_old));
		v.push_back(std::to_string(e.e_last_invsnapshot));
		v.push_back("FROM_UNIXTIME(" + (e.deleted_at > 0 ? std::to_string(e.deleted_at) : "null") + ")");
		v.push_back(std::to_string(e.illusion_block));

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
		const std::vector<CharacterData> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.account_id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back("'" + Strings::Escape(e.last_name) + "'");
			v.push_back("'" + Strings::Escape(e.title) + "'");
			v.push_back("'" + Strings::Escape(e.suffix) + "'");
			v.push_back(std::to_string(e.zone_id));
			v.push_back(std::to_string(e.zone_instance));
			v.push_back(std::to_string(e.y));
			v.push_back(std::to_string(e.x));
			v.push_back(std::to_string(e.z));
			v.push_back(std::to_string(e.heading));
			v.push_back(std::to_string(e.gender));
			v.push_back(std::to_string(e.race));
			v.push_back(std::to_string(e.class_));
			v.push_back(std::to_string(e.level));
			v.push_back(std::to_string(e.deity));
			v.push_back(std::to_string(e.birthday));
			v.push_back(std::to_string(e.last_login));
			v.push_back(std::to_string(e.time_played));
			v.push_back(std::to_string(e.level2));
			v.push_back(std::to_string(e.anon));
			v.push_back(std::to_string(e.gm));
			v.push_back(std::to_string(e.face));
			v.push_back(std::to_string(e.hair_color));
			v.push_back(std::to_string(e.hair_style));
			v.push_back(std::to_string(e.beard));
			v.push_back(std::to_string(e.beard_color));
			v.push_back(std::to_string(e.eye_color_1));
			v.push_back(std::to_string(e.eye_color_2));
			v.push_back(std::to_string(e.drakkin_heritage));
			v.push_back(std::to_string(e.drakkin_tattoo));
			v.push_back(std::to_string(e.drakkin_details));
			v.push_back(std::to_string(e.ability_time_seconds));
			v.push_back(std::to_string(e.ability_number));
			v.push_back(std::to_string(e.ability_time_minutes));
			v.push_back(std::to_string(e.ability_time_hours));
			v.push_back(std::to_string(e.exp));
			v.push_back(std::to_string(e.exp_enabled));
			v.push_back(std::to_string(e.aa_points_spent));
			v.push_back(std::to_string(e.aa_exp));
			v.push_back(std::to_string(e.aa_points));
			v.push_back(std::to_string(e.group_leadership_exp));
			v.push_back(std::to_string(e.raid_leadership_exp));
			v.push_back(std::to_string(e.group_leadership_points));
			v.push_back(std::to_string(e.raid_leadership_points));
			v.push_back(std::to_string(e.points));
			v.push_back(std::to_string(e.cur_hp));
			v.push_back(std::to_string(e.mana));
			v.push_back(std::to_string(e.endurance));
			v.push_back(std::to_string(e.intoxication));
			v.push_back(std::to_string(e.str));
			v.push_back(std::to_string(e.sta));
			v.push_back(std::to_string(e.cha));
			v.push_back(std::to_string(e.dex));
			v.push_back(std::to_string(e.int_));
			v.push_back(std::to_string(e.agi));
			v.push_back(std::to_string(e.wis));
			v.push_back(std::to_string(e.extra_haste));
			v.push_back(std::to_string(e.zone_change_count));
			v.push_back(std::to_string(e.toxicity));
			v.push_back(std::to_string(e.hunger_level));
			v.push_back(std::to_string(e.thirst_level));
			v.push_back(std::to_string(e.ability_up));
			v.push_back(std::to_string(e.ldon_points_guk));
			v.push_back(std::to_string(e.ldon_points_mir));
			v.push_back(std::to_string(e.ldon_points_mmc));
			v.push_back(std::to_string(e.ldon_points_ruj));
			v.push_back(std::to_string(e.ldon_points_tak));
			v.push_back(std::to_string(e.ldon_points_available));
			v.push_back(std::to_string(e.tribute_time_remaining));
			v.push_back(std::to_string(e.career_tribute_points));
			v.push_back(std::to_string(e.tribute_points));
			v.push_back(std::to_string(e.tribute_active));
			v.push_back(std::to_string(e.pvp_status));
			v.push_back(std::to_string(e.pvp_kills));
			v.push_back(std::to_string(e.pvp_deaths));
			v.push_back(std::to_string(e.pvp_current_points));
			v.push_back(std::to_string(e.pvp_career_points));
			v.push_back(std::to_string(e.pvp_best_kill_streak));
			v.push_back(std::to_string(e.pvp_worst_death_streak));
			v.push_back(std::to_string(e.pvp_current_kill_streak));
			v.push_back(std::to_string(e.pvp2));
			v.push_back(std::to_string(e.pvp_type));
			v.push_back(std::to_string(e.show_helm));
			v.push_back(std::to_string(e.group_auto_consent));
			v.push_back(std::to_string(e.raid_auto_consent));
			v.push_back(std::to_string(e.guild_auto_consent));
			v.push_back(std::to_string(e.leadership_exp_on));
			v.push_back(std::to_string(e.RestTimer));
			v.push_back(std::to_string(e.air_remaining));
			v.push_back(std::to_string(e.autosplit_enabled));
			v.push_back(std::to_string(e.lfp));
			v.push_back(std::to_string(e.lfg));
			v.push_back("'" + Strings::Escape(e.mailkey) + "'");
			v.push_back(std::to_string(e.xtargets));
			v.push_back(std::to_string(e.ingame));
			v.push_back(std::to_string(e.first_login));
			v.push_back(std::to_string(e.e_aa_effects));
			v.push_back(std::to_string(e.e_percent_to_aa));
			v.push_back(std::to_string(e.e_expended_aa_spent));
			v.push_back(std::to_string(e.aa_points_spent_old));
			v.push_back(std::to_string(e.aa_points_old));
			v.push_back(std::to_string(e.e_last_invsnapshot));
			v.push_back("FROM_UNIXTIME(" + (e.deleted_at > 0 ? std::to_string(e.deleted_at) : "null") + ")");
			v.push_back(std::to_string(e.illusion_block));

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

	static std::vector<CharacterData> All(Database& db)
	{
		std::vector<CharacterData> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterData e{};

			e.id                      = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.account_id              = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.name                    = row[2] ? row[2] : "";
			e.last_name               = row[3] ? row[3] : "";
			e.title                   = row[4] ? row[4] : "";
			e.suffix                  = row[5] ? row[5] : "";
			e.zone_id                 = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.zone_instance           = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.y                       = row[8] ? strtof(row[8], nullptr) : 0;
			e.x                       = row[9] ? strtof(row[9], nullptr) : 0;
			e.z                       = row[10] ? strtof(row[10], nullptr) : 0;
			e.heading                 = row[11] ? strtof(row[11], nullptr) : 0;
			e.gender                  = row[12] ? static_cast<uint8_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.race                    = row[13] ? static_cast<uint16_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.class_                  = row[14] ? static_cast<uint8_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.level                   = row[15] ? static_cast<uint32_t>(strtoul(row[15], nullptr, 10)) : 0;
			e.deity                   = row[16] ? static_cast<uint32_t>(strtoul(row[16], nullptr, 10)) : 0;
			e.birthday                = row[17] ? static_cast<uint32_t>(strtoul(row[17], nullptr, 10)) : 0;
			e.last_login              = row[18] ? static_cast<uint32_t>(strtoul(row[18], nullptr, 10)) : 0;
			e.time_played             = row[19] ? static_cast<uint32_t>(strtoul(row[19], nullptr, 10)) : 0;
			e.level2                  = row[20] ? static_cast<uint8_t>(strtoul(row[20], nullptr, 10)) : 0;
			e.anon                    = row[21] ? static_cast<uint8_t>(strtoul(row[21], nullptr, 10)) : 0;
			e.gm                      = row[22] ? static_cast<uint8_t>(strtoul(row[22], nullptr, 10)) : 0;
			e.face                    = row[23] ? static_cast<uint32_t>(strtoul(row[23], nullptr, 10)) : 0;
			e.hair_color              = row[24] ? static_cast<uint8_t>(strtoul(row[24], nullptr, 10)) : 0;
			e.hair_style              = row[25] ? static_cast<uint8_t>(strtoul(row[25], nullptr, 10)) : 0;
			e.beard                   = row[26] ? static_cast<uint8_t>(strtoul(row[26], nullptr, 10)) : 0;
			e.beard_color             = row[27] ? static_cast<uint8_t>(strtoul(row[27], nullptr, 10)) : 0;
			e.eye_color_1             = row[28] ? static_cast<uint8_t>(strtoul(row[28], nullptr, 10)) : 0;
			e.eye_color_2             = row[29] ? static_cast<uint8_t>(strtoul(row[29], nullptr, 10)) : 0;
			e.drakkin_heritage        = row[30] ? static_cast<uint32_t>(strtoul(row[30], nullptr, 10)) : 0;
			e.drakkin_tattoo          = row[31] ? static_cast<uint32_t>(strtoul(row[31], nullptr, 10)) : 0;
			e.drakkin_details         = row[32] ? static_cast<uint32_t>(strtoul(row[32], nullptr, 10)) : 0;
			e.ability_time_seconds    = row[33] ? static_cast<uint8_t>(strtoul(row[33], nullptr, 10)) : 0;
			e.ability_number          = row[34] ? static_cast<uint8_t>(strtoul(row[34], nullptr, 10)) : 0;
			e.ability_time_minutes    = row[35] ? static_cast<uint8_t>(strtoul(row[35], nullptr, 10)) : 0;
			e.ability_time_hours      = row[36] ? static_cast<uint8_t>(strtoul(row[36], nullptr, 10)) : 0;
			e.exp                     = row[37] ? static_cast<uint32_t>(strtoul(row[37], nullptr, 10)) : 0;
			e.exp_enabled             = row[38] ? static_cast<uint8_t>(strtoul(row[38], nullptr, 10)) : 1;
			e.aa_points_spent         = row[39] ? static_cast<uint32_t>(strtoul(row[39], nullptr, 10)) : 0;
			e.aa_exp                  = row[40] ? static_cast<uint32_t>(strtoul(row[40], nullptr, 10)) : 0;
			e.aa_points               = row[41] ? static_cast<uint32_t>(strtoul(row[41], nullptr, 10)) : 0;
			e.group_leadership_exp    = row[42] ? static_cast<uint32_t>(strtoul(row[42], nullptr, 10)) : 0;
			e.raid_leadership_exp     = row[43] ? static_cast<uint32_t>(strtoul(row[43], nullptr, 10)) : 0;
			e.group_leadership_points = row[44] ? static_cast<uint32_t>(strtoul(row[44], nullptr, 10)) : 0;
			e.raid_leadership_points  = row[45] ? static_cast<uint32_t>(strtoul(row[45], nullptr, 10)) : 0;
			e.points                  = row[46] ? static_cast<uint32_t>(strtoul(row[46], nullptr, 10)) : 0;
			e.cur_hp                  = row[47] ? static_cast<uint32_t>(strtoul(row[47], nullptr, 10)) : 0;
			e.mana                    = row[48] ? static_cast<uint32_t>(strtoul(row[48], nullptr, 10)) : 0;
			e.endurance               = row[49] ? static_cast<uint32_t>(strtoul(row[49], nullptr, 10)) : 0;
			e.intoxication            = row[50] ? static_cast<uint32_t>(strtoul(row[50], nullptr, 10)) : 0;
			e.str                     = row[51] ? static_cast<uint32_t>(strtoul(row[51], nullptr, 10)) : 0;
			e.sta                     = row[52] ? static_cast<uint32_t>(strtoul(row[52], nullptr, 10)) : 0;
			e.cha                     = row[53] ? static_cast<uint32_t>(strtoul(row[53], nullptr, 10)) : 0;
			e.dex                     = row[54] ? static_cast<uint32_t>(strtoul(row[54], nullptr, 10)) : 0;
			e.int_                    = row[55] ? static_cast<uint32_t>(strtoul(row[55], nullptr, 10)) : 0;
			e.agi                     = row[56] ? static_cast<uint32_t>(strtoul(row[56], nullptr, 10)) : 0;
			e.wis                     = row[57] ? static_cast<uint32_t>(strtoul(row[57], nullptr, 10)) : 0;
			e.extra_haste             = row[58] ? static_cast<int32_t>(atoi(row[58])) : 0;
			e.zone_change_count       = row[59] ? static_cast<uint32_t>(strtoul(row[59], nullptr, 10)) : 0;
			e.toxicity                = row[60] ? static_cast<uint32_t>(strtoul(row[60], nullptr, 10)) : 0;
			e.hunger_level            = row[61] ? static_cast<uint32_t>(strtoul(row[61], nullptr, 10)) : 0;
			e.thirst_level            = row[62] ? static_cast<uint32_t>(strtoul(row[62], nullptr, 10)) : 0;
			e.ability_up              = row[63] ? static_cast<uint32_t>(strtoul(row[63], nullptr, 10)) : 0;
			e.ldon_points_guk         = row[64] ? static_cast<uint32_t>(strtoul(row[64], nullptr, 10)) : 0;
			e.ldon_points_mir         = row[65] ? static_cast<uint32_t>(strtoul(row[65], nullptr, 10)) : 0;
			e.ldon_points_mmc         = row[66] ? static_cast<uint32_t>(strtoul(row[66], nullptr, 10)) : 0;
			e.ldon_points_ruj         = row[67] ? static_cast<uint32_t>(strtoul(row[67], nullptr, 10)) : 0;
			e.ldon_points_tak         = row[68] ? static_cast<uint32_t>(strtoul(row[68], nullptr, 10)) : 0;
			e.ldon_points_available   = row[69] ? static_cast<uint32_t>(strtoul(row[69], nullptr, 10)) : 0;
			e.tribute_time_remaining  = row[70] ? static_cast<uint32_t>(strtoul(row[70], nullptr, 10)) : 0;
			e.career_tribute_points   = row[71] ? static_cast<uint32_t>(strtoul(row[71], nullptr, 10)) : 0;
			e.tribute_points          = row[72] ? static_cast<uint32_t>(strtoul(row[72], nullptr, 10)) : 0;
			e.tribute_active          = row[73] ? static_cast<uint32_t>(strtoul(row[73], nullptr, 10)) : 0;
			e.pvp_status              = row[74] ? static_cast<uint8_t>(strtoul(row[74], nullptr, 10)) : 0;
			e.pvp_kills               = row[75] ? static_cast<uint32_t>(strtoul(row[75], nullptr, 10)) : 0;
			e.pvp_deaths              = row[76] ? static_cast<uint32_t>(strtoul(row[76], nullptr, 10)) : 0;
			e.pvp_current_points      = row[77] ? static_cast<uint32_t>(strtoul(row[77], nullptr, 10)) : 0;
			e.pvp_career_points       = row[78] ? static_cast<uint32_t>(strtoul(row[78], nullptr, 10)) : 0;
			e.pvp_best_kill_streak    = row[79] ? static_cast<uint32_t>(strtoul(row[79], nullptr, 10)) : 0;
			e.pvp_worst_death_streak  = row[80] ? static_cast<uint32_t>(strtoul(row[80], nullptr, 10)) : 0;
			e.pvp_current_kill_streak = row[81] ? static_cast<uint32_t>(strtoul(row[81], nullptr, 10)) : 0;
			e.pvp2                    = row[82] ? static_cast<uint32_t>(strtoul(row[82], nullptr, 10)) : 0;
			e.pvp_type                = row[83] ? static_cast<uint32_t>(strtoul(row[83], nullptr, 10)) : 0;
			e.show_helm               = row[84] ? static_cast<uint32_t>(strtoul(row[84], nullptr, 10)) : 0;
			e.group_auto_consent      = row[85] ? static_cast<uint8_t>(strtoul(row[85], nullptr, 10)) : 0;
			e.raid_auto_consent       = row[86] ? static_cast<uint8_t>(strtoul(row[86], nullptr, 10)) : 0;
			e.guild_auto_consent      = row[87] ? static_cast<uint8_t>(strtoul(row[87], nullptr, 10)) : 0;
			e.leadership_exp_on       = row[88] ? static_cast<uint8_t>(strtoul(row[88], nullptr, 10)) : 0;
			e.RestTimer               = row[89] ? static_cast<uint32_t>(strtoul(row[89], nullptr, 10)) : 0;
			e.air_remaining           = row[90] ? static_cast<uint32_t>(strtoul(row[90], nullptr, 10)) : 0;
			e.autosplit_enabled       = row[91] ? static_cast<uint32_t>(strtoul(row[91], nullptr, 10)) : 0;
			e.lfp                     = row[92] ? static_cast<uint8_t>(strtoul(row[92], nullptr, 10)) : 0;
			e.lfg                     = row[93] ? static_cast<uint8_t>(strtoul(row[93], nullptr, 10)) : 0;
			e.mailkey                 = row[94] ? row[94] : "";
			e.xtargets                = row[95] ? static_cast<uint8_t>(strtoul(row[95], nullptr, 10)) : 5;
			e.ingame                  = row[96] ? static_cast<uint8_t>(strtoul(row[96], nullptr, 10)) : 0;
			e.first_login             = row[97] ? static_cast<uint32_t>(strtoul(row[97], nullptr, 10)) : 0;
			e.e_aa_effects            = row[98] ? static_cast<uint32_t>(strtoul(row[98], nullptr, 10)) : 0;
			e.e_percent_to_aa         = row[99] ? static_cast<uint32_t>(strtoul(row[99], nullptr, 10)) : 0;
			e.e_expended_aa_spent     = row[100] ? static_cast<uint32_t>(strtoul(row[100], nullptr, 10)) : 0;
			e.aa_points_spent_old     = row[101] ? static_cast<uint32_t>(strtoul(row[101], nullptr, 10)) : 0;
			e.aa_points_old           = row[102] ? static_cast<uint32_t>(strtoul(row[102], nullptr, 10)) : 0;
			e.e_last_invsnapshot      = row[103] ? static_cast<uint32_t>(strtoul(row[103], nullptr, 10)) : 0;
			e.deleted_at              = strtoll(row[104] ? row[104] : "-1", nullptr, 10);
			e.illusion_block          = row[105] ? static_cast<uint8_t>(strtoul(row[105], nullptr, 10)) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CharacterData> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CharacterData> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterData e{};

			e.id                      = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.account_id              = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.name                    = row[2] ? row[2] : "";
			e.last_name               = row[3] ? row[3] : "";
			e.title                   = row[4] ? row[4] : "";
			e.suffix                  = row[5] ? row[5] : "";
			e.zone_id                 = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.zone_instance           = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.y                       = row[8] ? strtof(row[8], nullptr) : 0;
			e.x                       = row[9] ? strtof(row[9], nullptr) : 0;
			e.z                       = row[10] ? strtof(row[10], nullptr) : 0;
			e.heading                 = row[11] ? strtof(row[11], nullptr) : 0;
			e.gender                  = row[12] ? static_cast<uint8_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.race                    = row[13] ? static_cast<uint16_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.class_                  = row[14] ? static_cast<uint8_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.level                   = row[15] ? static_cast<uint32_t>(strtoul(row[15], nullptr, 10)) : 0;
			e.deity                   = row[16] ? static_cast<uint32_t>(strtoul(row[16], nullptr, 10)) : 0;
			e.birthday                = row[17] ? static_cast<uint32_t>(strtoul(row[17], nullptr, 10)) : 0;
			e.last_login              = row[18] ? static_cast<uint32_t>(strtoul(row[18], nullptr, 10)) : 0;
			e.time_played             = row[19] ? static_cast<uint32_t>(strtoul(row[19], nullptr, 10)) : 0;
			e.level2                  = row[20] ? static_cast<uint8_t>(strtoul(row[20], nullptr, 10)) : 0;
			e.anon                    = row[21] ? static_cast<uint8_t>(strtoul(row[21], nullptr, 10)) : 0;
			e.gm                      = row[22] ? static_cast<uint8_t>(strtoul(row[22], nullptr, 10)) : 0;
			e.face                    = row[23] ? static_cast<uint32_t>(strtoul(row[23], nullptr, 10)) : 0;
			e.hair_color              = row[24] ? static_cast<uint8_t>(strtoul(row[24], nullptr, 10)) : 0;
			e.hair_style              = row[25] ? static_cast<uint8_t>(strtoul(row[25], nullptr, 10)) : 0;
			e.beard                   = row[26] ? static_cast<uint8_t>(strtoul(row[26], nullptr, 10)) : 0;
			e.beard_color             = row[27] ? static_cast<uint8_t>(strtoul(row[27], nullptr, 10)) : 0;
			e.eye_color_1             = row[28] ? static_cast<uint8_t>(strtoul(row[28], nullptr, 10)) : 0;
			e.eye_color_2             = row[29] ? static_cast<uint8_t>(strtoul(row[29], nullptr, 10)) : 0;
			e.drakkin_heritage        = row[30] ? static_cast<uint32_t>(strtoul(row[30], nullptr, 10)) : 0;
			e.drakkin_tattoo          = row[31] ? static_cast<uint32_t>(strtoul(row[31], nullptr, 10)) : 0;
			e.drakkin_details         = row[32] ? static_cast<uint32_t>(strtoul(row[32], nullptr, 10)) : 0;
			e.ability_time_seconds    = row[33] ? static_cast<uint8_t>(strtoul(row[33], nullptr, 10)) : 0;
			e.ability_number          = row[34] ? static_cast<uint8_t>(strtoul(row[34], nullptr, 10)) : 0;
			e.ability_time_minutes    = row[35] ? static_cast<uint8_t>(strtoul(row[35], nullptr, 10)) : 0;
			e.ability_time_hours      = row[36] ? static_cast<uint8_t>(strtoul(row[36], nullptr, 10)) : 0;
			e.exp                     = row[37] ? static_cast<uint32_t>(strtoul(row[37], nullptr, 10)) : 0;
			e.exp_enabled             = row[38] ? static_cast<uint8_t>(strtoul(row[38], nullptr, 10)) : 1;
			e.aa_points_spent         = row[39] ? static_cast<uint32_t>(strtoul(row[39], nullptr, 10)) : 0;
			e.aa_exp                  = row[40] ? static_cast<uint32_t>(strtoul(row[40], nullptr, 10)) : 0;
			e.aa_points               = row[41] ? static_cast<uint32_t>(strtoul(row[41], nullptr, 10)) : 0;
			e.group_leadership_exp    = row[42] ? static_cast<uint32_t>(strtoul(row[42], nullptr, 10)) : 0;
			e.raid_leadership_exp     = row[43] ? static_cast<uint32_t>(strtoul(row[43], nullptr, 10)) : 0;
			e.group_leadership_points = row[44] ? static_cast<uint32_t>(strtoul(row[44], nullptr, 10)) : 0;
			e.raid_leadership_points  = row[45] ? static_cast<uint32_t>(strtoul(row[45], nullptr, 10)) : 0;
			e.points                  = row[46] ? static_cast<uint32_t>(strtoul(row[46], nullptr, 10)) : 0;
			e.cur_hp                  = row[47] ? static_cast<uint32_t>(strtoul(row[47], nullptr, 10)) : 0;
			e.mana                    = row[48] ? static_cast<uint32_t>(strtoul(row[48], nullptr, 10)) : 0;
			e.endurance               = row[49] ? static_cast<uint32_t>(strtoul(row[49], nullptr, 10)) : 0;
			e.intoxication            = row[50] ? static_cast<uint32_t>(strtoul(row[50], nullptr, 10)) : 0;
			e.str                     = row[51] ? static_cast<uint32_t>(strtoul(row[51], nullptr, 10)) : 0;
			e.sta                     = row[52] ? static_cast<uint32_t>(strtoul(row[52], nullptr, 10)) : 0;
			e.cha                     = row[53] ? static_cast<uint32_t>(strtoul(row[53], nullptr, 10)) : 0;
			e.dex                     = row[54] ? static_cast<uint32_t>(strtoul(row[54], nullptr, 10)) : 0;
			e.int_                    = row[55] ? static_cast<uint32_t>(strtoul(row[55], nullptr, 10)) : 0;
			e.agi                     = row[56] ? static_cast<uint32_t>(strtoul(row[56], nullptr, 10)) : 0;
			e.wis                     = row[57] ? static_cast<uint32_t>(strtoul(row[57], nullptr, 10)) : 0;
			e.extra_haste             = row[58] ? static_cast<int32_t>(atoi(row[58])) : 0;
			e.zone_change_count       = row[59] ? static_cast<uint32_t>(strtoul(row[59], nullptr, 10)) : 0;
			e.toxicity                = row[60] ? static_cast<uint32_t>(strtoul(row[60], nullptr, 10)) : 0;
			e.hunger_level            = row[61] ? static_cast<uint32_t>(strtoul(row[61], nullptr, 10)) : 0;
			e.thirst_level            = row[62] ? static_cast<uint32_t>(strtoul(row[62], nullptr, 10)) : 0;
			e.ability_up              = row[63] ? static_cast<uint32_t>(strtoul(row[63], nullptr, 10)) : 0;
			e.ldon_points_guk         = row[64] ? static_cast<uint32_t>(strtoul(row[64], nullptr, 10)) : 0;
			e.ldon_points_mir         = row[65] ? static_cast<uint32_t>(strtoul(row[65], nullptr, 10)) : 0;
			e.ldon_points_mmc         = row[66] ? static_cast<uint32_t>(strtoul(row[66], nullptr, 10)) : 0;
			e.ldon_points_ruj         = row[67] ? static_cast<uint32_t>(strtoul(row[67], nullptr, 10)) : 0;
			e.ldon_points_tak         = row[68] ? static_cast<uint32_t>(strtoul(row[68], nullptr, 10)) : 0;
			e.ldon_points_available   = row[69] ? static_cast<uint32_t>(strtoul(row[69], nullptr, 10)) : 0;
			e.tribute_time_remaining  = row[70] ? static_cast<uint32_t>(strtoul(row[70], nullptr, 10)) : 0;
			e.career_tribute_points   = row[71] ? static_cast<uint32_t>(strtoul(row[71], nullptr, 10)) : 0;
			e.tribute_points          = row[72] ? static_cast<uint32_t>(strtoul(row[72], nullptr, 10)) : 0;
			e.tribute_active          = row[73] ? static_cast<uint32_t>(strtoul(row[73], nullptr, 10)) : 0;
			e.pvp_status              = row[74] ? static_cast<uint8_t>(strtoul(row[74], nullptr, 10)) : 0;
			e.pvp_kills               = row[75] ? static_cast<uint32_t>(strtoul(row[75], nullptr, 10)) : 0;
			e.pvp_deaths              = row[76] ? static_cast<uint32_t>(strtoul(row[76], nullptr, 10)) : 0;
			e.pvp_current_points      = row[77] ? static_cast<uint32_t>(strtoul(row[77], nullptr, 10)) : 0;
			e.pvp_career_points       = row[78] ? static_cast<uint32_t>(strtoul(row[78], nullptr, 10)) : 0;
			e.pvp_best_kill_streak    = row[79] ? static_cast<uint32_t>(strtoul(row[79], nullptr, 10)) : 0;
			e.pvp_worst_death_streak  = row[80] ? static_cast<uint32_t>(strtoul(row[80], nullptr, 10)) : 0;
			e.pvp_current_kill_streak = row[81] ? static_cast<uint32_t>(strtoul(row[81], nullptr, 10)) : 0;
			e.pvp2                    = row[82] ? static_cast<uint32_t>(strtoul(row[82], nullptr, 10)) : 0;
			e.pvp_type                = row[83] ? static_cast<uint32_t>(strtoul(row[83], nullptr, 10)) : 0;
			e.show_helm               = row[84] ? static_cast<uint32_t>(strtoul(row[84], nullptr, 10)) : 0;
			e.group_auto_consent      = row[85] ? static_cast<uint8_t>(strtoul(row[85], nullptr, 10)) : 0;
			e.raid_auto_consent       = row[86] ? static_cast<uint8_t>(strtoul(row[86], nullptr, 10)) : 0;
			e.guild_auto_consent      = row[87] ? static_cast<uint8_t>(strtoul(row[87], nullptr, 10)) : 0;
			e.leadership_exp_on       = row[88] ? static_cast<uint8_t>(strtoul(row[88], nullptr, 10)) : 0;
			e.RestTimer               = row[89] ? static_cast<uint32_t>(strtoul(row[89], nullptr, 10)) : 0;
			e.air_remaining           = row[90] ? static_cast<uint32_t>(strtoul(row[90], nullptr, 10)) : 0;
			e.autosplit_enabled       = row[91] ? static_cast<uint32_t>(strtoul(row[91], nullptr, 10)) : 0;
			e.lfp                     = row[92] ? static_cast<uint8_t>(strtoul(row[92], nullptr, 10)) : 0;
			e.lfg                     = row[93] ? static_cast<uint8_t>(strtoul(row[93], nullptr, 10)) : 0;
			e.mailkey                 = row[94] ? row[94] : "";
			e.xtargets                = row[95] ? static_cast<uint8_t>(strtoul(row[95], nullptr, 10)) : 5;
			e.ingame                  = row[96] ? static_cast<uint8_t>(strtoul(row[96], nullptr, 10)) : 0;
			e.first_login             = row[97] ? static_cast<uint32_t>(strtoul(row[97], nullptr, 10)) : 0;
			e.e_aa_effects            = row[98] ? static_cast<uint32_t>(strtoul(row[98], nullptr, 10)) : 0;
			e.e_percent_to_aa         = row[99] ? static_cast<uint32_t>(strtoul(row[99], nullptr, 10)) : 0;
			e.e_expended_aa_spent     = row[100] ? static_cast<uint32_t>(strtoul(row[100], nullptr, 10)) : 0;
			e.aa_points_spent_old     = row[101] ? static_cast<uint32_t>(strtoul(row[101], nullptr, 10)) : 0;
			e.aa_points_old           = row[102] ? static_cast<uint32_t>(strtoul(row[102], nullptr, 10)) : 0;
			e.e_last_invsnapshot      = row[103] ? static_cast<uint32_t>(strtoul(row[103], nullptr, 10)) : 0;
			e.deleted_at              = strtoll(row[104] ? row[104] : "-1", nullptr, 10);
			e.illusion_block          = row[105] ? static_cast<uint8_t>(strtoul(row[105], nullptr, 10)) : 0;

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
		const CharacterData &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.account_id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back("'" + Strings::Escape(e.last_name) + "'");
		v.push_back("'" + Strings::Escape(e.title) + "'");
		v.push_back("'" + Strings::Escape(e.suffix) + "'");
		v.push_back(std::to_string(e.zone_id));
		v.push_back(std::to_string(e.zone_instance));
		v.push_back(std::to_string(e.y));
		v.push_back(std::to_string(e.x));
		v.push_back(std::to_string(e.z));
		v.push_back(std::to_string(e.heading));
		v.push_back(std::to_string(e.gender));
		v.push_back(std::to_string(e.race));
		v.push_back(std::to_string(e.class_));
		v.push_back(std::to_string(e.level));
		v.push_back(std::to_string(e.deity));
		v.push_back(std::to_string(e.birthday));
		v.push_back(std::to_string(e.last_login));
		v.push_back(std::to_string(e.time_played));
		v.push_back(std::to_string(e.level2));
		v.push_back(std::to_string(e.anon));
		v.push_back(std::to_string(e.gm));
		v.push_back(std::to_string(e.face));
		v.push_back(std::to_string(e.hair_color));
		v.push_back(std::to_string(e.hair_style));
		v.push_back(std::to_string(e.beard));
		v.push_back(std::to_string(e.beard_color));
		v.push_back(std::to_string(e.eye_color_1));
		v.push_back(std::to_string(e.eye_color_2));
		v.push_back(std::to_string(e.drakkin_heritage));
		v.push_back(std::to_string(e.drakkin_tattoo));
		v.push_back(std::to_string(e.drakkin_details));
		v.push_back(std::to_string(e.ability_time_seconds));
		v.push_back(std::to_string(e.ability_number));
		v.push_back(std::to_string(e.ability_time_minutes));
		v.push_back(std::to_string(e.ability_time_hours));
		v.push_back(std::to_string(e.exp));
		v.push_back(std::to_string(e.exp_enabled));
		v.push_back(std::to_string(e.aa_points_spent));
		v.push_back(std::to_string(e.aa_exp));
		v.push_back(std::to_string(e.aa_points));
		v.push_back(std::to_string(e.group_leadership_exp));
		v.push_back(std::to_string(e.raid_leadership_exp));
		v.push_back(std::to_string(e.group_leadership_points));
		v.push_back(std::to_string(e.raid_leadership_points));
		v.push_back(std::to_string(e.points));
		v.push_back(std::to_string(e.cur_hp));
		v.push_back(std::to_string(e.mana));
		v.push_back(std::to_string(e.endurance));
		v.push_back(std::to_string(e.intoxication));
		v.push_back(std::to_string(e.str));
		v.push_back(std::to_string(e.sta));
		v.push_back(std::to_string(e.cha));
		v.push_back(std::to_string(e.dex));
		v.push_back(std::to_string(e.int_));
		v.push_back(std::to_string(e.agi));
		v.push_back(std::to_string(e.wis));
		v.push_back(std::to_string(e.extra_haste));
		v.push_back(std::to_string(e.zone_change_count));
		v.push_back(std::to_string(e.toxicity));
		v.push_back(std::to_string(e.hunger_level));
		v.push_back(std::to_string(e.thirst_level));
		v.push_back(std::to_string(e.ability_up));
		v.push_back(std::to_string(e.ldon_points_guk));
		v.push_back(std::to_string(e.ldon_points_mir));
		v.push_back(std::to_string(e.ldon_points_mmc));
		v.push_back(std::to_string(e.ldon_points_ruj));
		v.push_back(std::to_string(e.ldon_points_tak));
		v.push_back(std::to_string(e.ldon_points_available));
		v.push_back(std::to_string(e.tribute_time_remaining));
		v.push_back(std::to_string(e.career_tribute_points));
		v.push_back(std::to_string(e.tribute_points));
		v.push_back(std::to_string(e.tribute_active));
		v.push_back(std::to_string(e.pvp_status));
		v.push_back(std::to_string(e.pvp_kills));
		v.push_back(std::to_string(e.pvp_deaths));
		v.push_back(std::to_string(e.pvp_current_points));
		v.push_back(std::to_string(e.pvp_career_points));
		v.push_back(std::to_string(e.pvp_best_kill_streak));
		v.push_back(std::to_string(e.pvp_worst_death_streak));
		v.push_back(std::to_string(e.pvp_current_kill_streak));
		v.push_back(std::to_string(e.pvp2));
		v.push_back(std::to_string(e.pvp_type));
		v.push_back(std::to_string(e.show_helm));
		v.push_back(std::to_string(e.group_auto_consent));
		v.push_back(std::to_string(e.raid_auto_consent));
		v.push_back(std::to_string(e.guild_auto_consent));
		v.push_back(std::to_string(e.leadership_exp_on));
		v.push_back(std::to_string(e.RestTimer));
		v.push_back(std::to_string(e.air_remaining));
		v.push_back(std::to_string(e.autosplit_enabled));
		v.push_back(std::to_string(e.lfp));
		v.push_back(std::to_string(e.lfg));
		v.push_back("'" + Strings::Escape(e.mailkey) + "'");
		v.push_back(std::to_string(e.xtargets));
		v.push_back(std::to_string(e.ingame));
		v.push_back(std::to_string(e.first_login));
		v.push_back(std::to_string(e.e_aa_effects));
		v.push_back(std::to_string(e.e_percent_to_aa));
		v.push_back(std::to_string(e.e_expended_aa_spent));
		v.push_back(std::to_string(e.aa_points_spent_old));
		v.push_back(std::to_string(e.aa_points_old));
		v.push_back(std::to_string(e.e_last_invsnapshot));
		v.push_back("FROM_UNIXTIME(" + (e.deleted_at > 0 ? std::to_string(e.deleted_at) : "null") + ")");
		v.push_back(std::to_string(e.illusion_block));

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
		const std::vector<CharacterData> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.account_id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back("'" + Strings::Escape(e.last_name) + "'");
			v.push_back("'" + Strings::Escape(e.title) + "'");
			v.push_back("'" + Strings::Escape(e.suffix) + "'");
			v.push_back(std::to_string(e.zone_id));
			v.push_back(std::to_string(e.zone_instance));
			v.push_back(std::to_string(e.y));
			v.push_back(std::to_string(e.x));
			v.push_back(std::to_string(e.z));
			v.push_back(std::to_string(e.heading));
			v.push_back(std::to_string(e.gender));
			v.push_back(std::to_string(e.race));
			v.push_back(std::to_string(e.class_));
			v.push_back(std::to_string(e.level));
			v.push_back(std::to_string(e.deity));
			v.push_back(std::to_string(e.birthday));
			v.push_back(std::to_string(e.last_login));
			v.push_back(std::to_string(e.time_played));
			v.push_back(std::to_string(e.level2));
			v.push_back(std::to_string(e.anon));
			v.push_back(std::to_string(e.gm));
			v.push_back(std::to_string(e.face));
			v.push_back(std::to_string(e.hair_color));
			v.push_back(std::to_string(e.hair_style));
			v.push_back(std::to_string(e.beard));
			v.push_back(std::to_string(e.beard_color));
			v.push_back(std::to_string(e.eye_color_1));
			v.push_back(std::to_string(e.eye_color_2));
			v.push_back(std::to_string(e.drakkin_heritage));
			v.push_back(std::to_string(e.drakkin_tattoo));
			v.push_back(std::to_string(e.drakkin_details));
			v.push_back(std::to_string(e.ability_time_seconds));
			v.push_back(std::to_string(e.ability_number));
			v.push_back(std::to_string(e.ability_time_minutes));
			v.push_back(std::to_string(e.ability_time_hours));
			v.push_back(std::to_string(e.exp));
			v.push_back(std::to_string(e.exp_enabled));
			v.push_back(std::to_string(e.aa_points_spent));
			v.push_back(std::to_string(e.aa_exp));
			v.push_back(std::to_string(e.aa_points));
			v.push_back(std::to_string(e.group_leadership_exp));
			v.push_back(std::to_string(e.raid_leadership_exp));
			v.push_back(std::to_string(e.group_leadership_points));
			v.push_back(std::to_string(e.raid_leadership_points));
			v.push_back(std::to_string(e.points));
			v.push_back(std::to_string(e.cur_hp));
			v.push_back(std::to_string(e.mana));
			v.push_back(std::to_string(e.endurance));
			v.push_back(std::to_string(e.intoxication));
			v.push_back(std::to_string(e.str));
			v.push_back(std::to_string(e.sta));
			v.push_back(std::to_string(e.cha));
			v.push_back(std::to_string(e.dex));
			v.push_back(std::to_string(e.int_));
			v.push_back(std::to_string(e.agi));
			v.push_back(std::to_string(e.wis));
			v.push_back(std::to_string(e.extra_haste));
			v.push_back(std::to_string(e.zone_change_count));
			v.push_back(std::to_string(e.toxicity));
			v.push_back(std::to_string(e.hunger_level));
			v.push_back(std::to_string(e.thirst_level));
			v.push_back(std::to_string(e.ability_up));
			v.push_back(std::to_string(e.ldon_points_guk));
			v.push_back(std::to_string(e.ldon_points_mir));
			v.push_back(std::to_string(e.ldon_points_mmc));
			v.push_back(std::to_string(e.ldon_points_ruj));
			v.push_back(std::to_string(e.ldon_points_tak));
			v.push_back(std::to_string(e.ldon_points_available));
			v.push_back(std::to_string(e.tribute_time_remaining));
			v.push_back(std::to_string(e.career_tribute_points));
			v.push_back(std::to_string(e.tribute_points));
			v.push_back(std::to_string(e.tribute_active));
			v.push_back(std::to_string(e.pvp_status));
			v.push_back(std::to_string(e.pvp_kills));
			v.push_back(std::to_string(e.pvp_deaths));
			v.push_back(std::to_string(e.pvp_current_points));
			v.push_back(std::to_string(e.pvp_career_points));
			v.push_back(std::to_string(e.pvp_best_kill_streak));
			v.push_back(std::to_string(e.pvp_worst_death_streak));
			v.push_back(std::to_string(e.pvp_current_kill_streak));
			v.push_back(std::to_string(e.pvp2));
			v.push_back(std::to_string(e.pvp_type));
			v.push_back(std::to_string(e.show_helm));
			v.push_back(std::to_string(e.group_auto_consent));
			v.push_back(std::to_string(e.raid_auto_consent));
			v.push_back(std::to_string(e.guild_auto_consent));
			v.push_back(std::to_string(e.leadership_exp_on));
			v.push_back(std::to_string(e.RestTimer));
			v.push_back(std::to_string(e.air_remaining));
			v.push_back(std::to_string(e.autosplit_enabled));
			v.push_back(std::to_string(e.lfp));
			v.push_back(std::to_string(e.lfg));
			v.push_back("'" + Strings::Escape(e.mailkey) + "'");
			v.push_back(std::to_string(e.xtargets));
			v.push_back(std::to_string(e.ingame));
			v.push_back(std::to_string(e.first_login));
			v.push_back(std::to_string(e.e_aa_effects));
			v.push_back(std::to_string(e.e_percent_to_aa));
			v.push_back(std::to_string(e.e_expended_aa_spent));
			v.push_back(std::to_string(e.aa_points_spent_old));
			v.push_back(std::to_string(e.aa_points_old));
			v.push_back(std::to_string(e.e_last_invsnapshot));
			v.push_back("FROM_UNIXTIME(" + (e.deleted_at > 0 ? std::to_string(e.deleted_at) : "null") + ")");
			v.push_back(std::to_string(e.illusion_block));

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
