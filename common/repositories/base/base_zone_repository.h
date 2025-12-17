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

class BaseZoneRepository {
public:
	struct Zone {
		int32_t     id;
		int32_t     zoneidnumber;
		uint8_t     version;
		std::string short_name;
		std::string long_name;
		uint8_t     min_status;
		std::string map_file_name;
		std::string note;
		int8_t      min_expansion;
		int8_t      max_expansion;
		std::string content_flags;
		std::string content_flags_disabled;
		int8_t      expansion;
		std::string file_name;
		float       safe_x;
		float       safe_y;
		float       safe_z;
		float       safe_heading;
		float       graveyard_id;
		uint8_t     min_level;
		uint8_t     max_level;
		int32_t     timezone;
		int32_t     maxclients;
		uint32_t    ruleset;
		float       underworld;
		float       minclip;
		float       maxclip;
		float       fog_minclip;
		float       fog_maxclip;
		uint8_t     fog_blue;
		uint8_t     fog_red;
		uint8_t     fog_green;
		uint8_t     sky;
		uint8_t     ztype;
		float       zone_exp_multiplier;
		float       walkspeed;
		uint8_t     time_type;
		uint8_t     fog_red1;
		uint8_t     fog_green1;
		uint8_t     fog_blue1;
		float       fog_minclip1;
		float       fog_maxclip1;
		uint8_t     fog_red2;
		uint8_t     fog_green2;
		uint8_t     fog_blue2;
		float       fog_minclip2;
		float       fog_maxclip2;
		uint8_t     fog_red3;
		uint8_t     fog_green3;
		uint8_t     fog_blue3;
		float       fog_minclip3;
		float       fog_maxclip3;
		uint8_t     fog_red4;
		uint8_t     fog_green4;
		uint8_t     fog_blue4;
		float       fog_minclip4;
		float       fog_maxclip4;
		float       fog_density;
		std::string flag_needed;
		int8_t      canbind;
		int8_t      cancombat;
		int8_t      canlevitate;
		int8_t      castoutdoor;
		uint8_t     hotzone;
		uint8_t     insttype;
		uint64_t    shutdowndelay;
		int8_t      peqzone;
		int8_t      bypass_expansion_check;
		uint8_t     suspendbuffs;
		int32_t     rain_chance1;
		int32_t     rain_chance2;
		int32_t     rain_chance3;
		int32_t     rain_chance4;
		int32_t     rain_duration1;
		int32_t     rain_duration2;
		int32_t     rain_duration3;
		int32_t     rain_duration4;
		int32_t     snow_chance1;
		int32_t     snow_chance2;
		int32_t     snow_chance3;
		int32_t     snow_chance4;
		int32_t     snow_duration1;
		int32_t     snow_duration2;
		int32_t     snow_duration3;
		int32_t     snow_duration4;
		float       gravity;
		int32_t     type;
		int8_t      skylock;
		int32_t     fast_regen_hp;
		int32_t     fast_regen_mana;
		int32_t     fast_regen_endurance;
		int32_t     npc_max_aggro_dist;
		uint32_t    client_update_range;
		int32_t     underworld_teleport_index;
		int32_t     lava_damage;
		int32_t     min_lava_damage;
		uint8_t     idle_when_empty;
		uint32_t    seconds_before_idle;
		int32_t     shard_at_player_count;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"zoneidnumber",
			"version",
			"short_name",
			"long_name",
			"min_status",
			"map_file_name",
			"note",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
			"expansion",
			"file_name",
			"safe_x",
			"safe_y",
			"safe_z",
			"safe_heading",
			"graveyard_id",
			"min_level",
			"max_level",
			"timezone",
			"maxclients",
			"ruleset",
			"underworld",
			"minclip",
			"maxclip",
			"fog_minclip",
			"fog_maxclip",
			"fog_blue",
			"fog_red",
			"fog_green",
			"sky",
			"ztype",
			"zone_exp_multiplier",
			"walkspeed",
			"time_type",
			"fog_red1",
			"fog_green1",
			"fog_blue1",
			"fog_minclip1",
			"fog_maxclip1",
			"fog_red2",
			"fog_green2",
			"fog_blue2",
			"fog_minclip2",
			"fog_maxclip2",
			"fog_red3",
			"fog_green3",
			"fog_blue3",
			"fog_minclip3",
			"fog_maxclip3",
			"fog_red4",
			"fog_green4",
			"fog_blue4",
			"fog_minclip4",
			"fog_maxclip4",
			"fog_density",
			"flag_needed",
			"canbind",
			"cancombat",
			"canlevitate",
			"castoutdoor",
			"hotzone",
			"insttype",
			"shutdowndelay",
			"peqzone",
			"bypass_expansion_check",
			"suspendbuffs",
			"rain_chance1",
			"rain_chance2",
			"rain_chance3",
			"rain_chance4",
			"rain_duration1",
			"rain_duration2",
			"rain_duration3",
			"rain_duration4",
			"snow_chance1",
			"snow_chance2",
			"snow_chance3",
			"snow_chance4",
			"snow_duration1",
			"snow_duration2",
			"snow_duration3",
			"snow_duration4",
			"gravity",
			"type",
			"skylock",
			"fast_regen_hp",
			"fast_regen_mana",
			"fast_regen_endurance",
			"npc_max_aggro_dist",
			"client_update_range",
			"underworld_teleport_index",
			"lava_damage",
			"min_lava_damage",
			"idle_when_empty",
			"seconds_before_idle",
			"shard_at_player_count",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"zoneidnumber",
			"version",
			"short_name",
			"long_name",
			"min_status",
			"map_file_name",
			"note",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
			"expansion",
			"file_name",
			"safe_x",
			"safe_y",
			"safe_z",
			"safe_heading",
			"graveyard_id",
			"min_level",
			"max_level",
			"timezone",
			"maxclients",
			"ruleset",
			"underworld",
			"minclip",
			"maxclip",
			"fog_minclip",
			"fog_maxclip",
			"fog_blue",
			"fog_red",
			"fog_green",
			"sky",
			"ztype",
			"zone_exp_multiplier",
			"walkspeed",
			"time_type",
			"fog_red1",
			"fog_green1",
			"fog_blue1",
			"fog_minclip1",
			"fog_maxclip1",
			"fog_red2",
			"fog_green2",
			"fog_blue2",
			"fog_minclip2",
			"fog_maxclip2",
			"fog_red3",
			"fog_green3",
			"fog_blue3",
			"fog_minclip3",
			"fog_maxclip3",
			"fog_red4",
			"fog_green4",
			"fog_blue4",
			"fog_minclip4",
			"fog_maxclip4",
			"fog_density",
			"flag_needed",
			"canbind",
			"cancombat",
			"canlevitate",
			"castoutdoor",
			"hotzone",
			"insttype",
			"shutdowndelay",
			"peqzone",
			"bypass_expansion_check",
			"suspendbuffs",
			"rain_chance1",
			"rain_chance2",
			"rain_chance3",
			"rain_chance4",
			"rain_duration1",
			"rain_duration2",
			"rain_duration3",
			"rain_duration4",
			"snow_chance1",
			"snow_chance2",
			"snow_chance3",
			"snow_chance4",
			"snow_duration1",
			"snow_duration2",
			"snow_duration3",
			"snow_duration4",
			"gravity",
			"type",
			"skylock",
			"fast_regen_hp",
			"fast_regen_mana",
			"fast_regen_endurance",
			"npc_max_aggro_dist",
			"client_update_range",
			"underworld_teleport_index",
			"lava_damage",
			"min_lava_damage",
			"idle_when_empty",
			"seconds_before_idle",
			"shard_at_player_count",
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
		return std::string("zone");
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

	static Zone NewEntity()
	{
		Zone e{};

		e.id                        = 0;
		e.zoneidnumber              = 0;
		e.version                   = 0;
		e.short_name                = "";
		e.long_name                 = "";
		e.min_status                = 0;
		e.map_file_name             = "";
		e.note                      = "";
		e.min_expansion             = -1;
		e.max_expansion             = -1;
		e.content_flags             = "";
		e.content_flags_disabled    = "";
		e.expansion                 = 0;
		e.file_name                 = "";
		e.safe_x                    = 0;
		e.safe_y                    = 0;
		e.safe_z                    = 0;
		e.safe_heading              = 0;
		e.graveyard_id              = 0;
		e.min_level                 = 0;
		e.max_level                 = 255;
		e.timezone                  = 0;
		e.maxclients                = 0;
		e.ruleset                   = 0;
		e.underworld                = 0;
		e.minclip                   = 450;
		e.maxclip                   = 450;
		e.fog_minclip               = 450;
		e.fog_maxclip               = 450;
		e.fog_blue                  = 0;
		e.fog_red                   = 0;
		e.fog_green                 = 0;
		e.sky                       = 1;
		e.ztype                     = 1;
		e.zone_exp_multiplier       = 0.00;
		e.walkspeed                 = 0.4;
		e.time_type                 = 2;
		e.fog_red1                  = 0;
		e.fog_green1                = 0;
		e.fog_blue1                 = 0;
		e.fog_minclip1              = 450;
		e.fog_maxclip1              = 450;
		e.fog_red2                  = 0;
		e.fog_green2                = 0;
		e.fog_blue2                 = 0;
		e.fog_minclip2              = 450;
		e.fog_maxclip2              = 450;
		e.fog_red3                  = 0;
		e.fog_green3                = 0;
		e.fog_blue3                 = 0;
		e.fog_minclip3              = 450;
		e.fog_maxclip3              = 450;
		e.fog_red4                  = 0;
		e.fog_green4                = 0;
		e.fog_blue4                 = 0;
		e.fog_minclip4              = 450;
		e.fog_maxclip4              = 450;
		e.fog_density               = 0;
		e.flag_needed               = "";
		e.canbind                   = 1;
		e.cancombat                 = 1;
		e.canlevitate               = 1;
		e.castoutdoor               = 1;
		e.hotzone                   = 0;
		e.insttype                  = 0;
		e.shutdowndelay             = 5000;
		e.peqzone                   = 1;
		e.bypass_expansion_check    = 0;
		e.suspendbuffs              = 0;
		e.rain_chance1              = 0;
		e.rain_chance2              = 0;
		e.rain_chance3              = 0;
		e.rain_chance4              = 0;
		e.rain_duration1            = 0;
		e.rain_duration2            = 0;
		e.rain_duration3            = 0;
		e.rain_duration4            = 0;
		e.snow_chance1              = 0;
		e.snow_chance2              = 0;
		e.snow_chance3              = 0;
		e.snow_chance4              = 0;
		e.snow_duration1            = 0;
		e.snow_duration2            = 0;
		e.snow_duration3            = 0;
		e.snow_duration4            = 0;
		e.gravity                   = 0.4;
		e.type                      = 0;
		e.skylock                   = 0;
		e.fast_regen_hp             = 180;
		e.fast_regen_mana           = 180;
		e.fast_regen_endurance      = 180;
		e.npc_max_aggro_dist        = 600;
		e.client_update_range       = 600;
		e.underworld_teleport_index = 0;
		e.lava_damage               = 50;
		e.min_lava_damage           = 10;
		e.idle_when_empty           = 1;
		e.seconds_before_idle       = 60;
		e.shard_at_player_count     = 0;

		return e;
	}

	static Zone GetZone(
		const std::vector<Zone> &zones,
		int zone_id
	)
	{
		for (auto &zone : zones) {
			if (zone.id == zone_id) {
				return zone;
			}
		}

		return NewEntity();
	}

	static Zone FindOne(
		Database& db,
		int zone_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				zone_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Zone e{};

			e.id                        = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.zoneidnumber              = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.version                   = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.short_name                = row[3] ? row[3] : "";
			e.long_name                 = row[4] ? row[4] : "";
			e.min_status                = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.map_file_name             = row[6] ? row[6] : "";
			e.note                      = row[7] ? row[7] : "";
			e.min_expansion             = row[8] ? static_cast<int8_t>(atoi(row[8])) : -1;
			e.max_expansion             = row[9] ? static_cast<int8_t>(atoi(row[9])) : -1;
			e.content_flags             = row[10] ? row[10] : "";
			e.content_flags_disabled    = row[11] ? row[11] : "";
			e.expansion                 = row[12] ? static_cast<int8_t>(atoi(row[12])) : 0;
			e.file_name                 = row[13] ? row[13] : "";
			e.safe_x                    = row[14] ? strtof(row[14], nullptr) : 0;
			e.safe_y                    = row[15] ? strtof(row[15], nullptr) : 0;
			e.safe_z                    = row[16] ? strtof(row[16], nullptr) : 0;
			e.safe_heading              = row[17] ? strtof(row[17], nullptr) : 0;
			e.graveyard_id              = row[18] ? strtof(row[18], nullptr) : 0;
			e.min_level                 = row[19] ? static_cast<uint8_t>(strtoul(row[19], nullptr, 10)) : 0;
			e.max_level                 = row[20] ? static_cast<uint8_t>(strtoul(row[20], nullptr, 10)) : 255;
			e.timezone                  = row[21] ? static_cast<int32_t>(atoi(row[21])) : 0;
			e.maxclients                = row[22] ? static_cast<int32_t>(atoi(row[22])) : 0;
			e.ruleset                   = row[23] ? static_cast<uint32_t>(strtoul(row[23], nullptr, 10)) : 0;
			e.underworld                = row[24] ? strtof(row[24], nullptr) : 0;
			e.minclip                   = row[25] ? strtof(row[25], nullptr) : 450;
			e.maxclip                   = row[26] ? strtof(row[26], nullptr) : 450;
			e.fog_minclip               = row[27] ? strtof(row[27], nullptr) : 450;
			e.fog_maxclip               = row[28] ? strtof(row[28], nullptr) : 450;
			e.fog_blue                  = row[29] ? static_cast<uint8_t>(strtoul(row[29], nullptr, 10)) : 0;
			e.fog_red                   = row[30] ? static_cast<uint8_t>(strtoul(row[30], nullptr, 10)) : 0;
			e.fog_green                 = row[31] ? static_cast<uint8_t>(strtoul(row[31], nullptr, 10)) : 0;
			e.sky                       = row[32] ? static_cast<uint8_t>(strtoul(row[32], nullptr, 10)) : 1;
			e.ztype                     = row[33] ? static_cast<uint8_t>(strtoul(row[33], nullptr, 10)) : 1;
			e.zone_exp_multiplier       = row[34] ? strtof(row[34], nullptr) : 0.00;
			e.walkspeed                 = row[35] ? strtof(row[35], nullptr) : 0.4;
			e.time_type                 = row[36] ? static_cast<uint8_t>(strtoul(row[36], nullptr, 10)) : 2;
			e.fog_red1                  = row[37] ? static_cast<uint8_t>(strtoul(row[37], nullptr, 10)) : 0;
			e.fog_green1                = row[38] ? static_cast<uint8_t>(strtoul(row[38], nullptr, 10)) : 0;
			e.fog_blue1                 = row[39] ? static_cast<uint8_t>(strtoul(row[39], nullptr, 10)) : 0;
			e.fog_minclip1              = row[40] ? strtof(row[40], nullptr) : 450;
			e.fog_maxclip1              = row[41] ? strtof(row[41], nullptr) : 450;
			e.fog_red2                  = row[42] ? static_cast<uint8_t>(strtoul(row[42], nullptr, 10)) : 0;
			e.fog_green2                = row[43] ? static_cast<uint8_t>(strtoul(row[43], nullptr, 10)) : 0;
			e.fog_blue2                 = row[44] ? static_cast<uint8_t>(strtoul(row[44], nullptr, 10)) : 0;
			e.fog_minclip2              = row[45] ? strtof(row[45], nullptr) : 450;
			e.fog_maxclip2              = row[46] ? strtof(row[46], nullptr) : 450;
			e.fog_red3                  = row[47] ? static_cast<uint8_t>(strtoul(row[47], nullptr, 10)) : 0;
			e.fog_green3                = row[48] ? static_cast<uint8_t>(strtoul(row[48], nullptr, 10)) : 0;
			e.fog_blue3                 = row[49] ? static_cast<uint8_t>(strtoul(row[49], nullptr, 10)) : 0;
			e.fog_minclip3              = row[50] ? strtof(row[50], nullptr) : 450;
			e.fog_maxclip3              = row[51] ? strtof(row[51], nullptr) : 450;
			e.fog_red4                  = row[52] ? static_cast<uint8_t>(strtoul(row[52], nullptr, 10)) : 0;
			e.fog_green4                = row[53] ? static_cast<uint8_t>(strtoul(row[53], nullptr, 10)) : 0;
			e.fog_blue4                 = row[54] ? static_cast<uint8_t>(strtoul(row[54], nullptr, 10)) : 0;
			e.fog_minclip4              = row[55] ? strtof(row[55], nullptr) : 450;
			e.fog_maxclip4              = row[56] ? strtof(row[56], nullptr) : 450;
			e.fog_density               = row[57] ? strtof(row[57], nullptr) : 0;
			e.flag_needed               = row[58] ? row[58] : "";
			e.canbind                   = row[59] ? static_cast<int8_t>(atoi(row[59])) : 1;
			e.cancombat                 = row[60] ? static_cast<int8_t>(atoi(row[60])) : 1;
			e.canlevitate               = row[61] ? static_cast<int8_t>(atoi(row[61])) : 1;
			e.castoutdoor               = row[62] ? static_cast<int8_t>(atoi(row[62])) : 1;
			e.hotzone                   = row[63] ? static_cast<uint8_t>(strtoul(row[63], nullptr, 10)) : 0;
			e.insttype                  = row[64] ? static_cast<uint8_t>(strtoul(row[64], nullptr, 10)) : 0;
			e.shutdowndelay             = row[65] ? strtoull(row[65], nullptr, 10) : 5000;
			e.peqzone                   = row[66] ? static_cast<int8_t>(atoi(row[66])) : 1;
			e.bypass_expansion_check    = row[67] ? static_cast<int8_t>(atoi(row[67])) : 0;
			e.suspendbuffs              = row[68] ? static_cast<uint8_t>(strtoul(row[68], nullptr, 10)) : 0;
			e.rain_chance1              = row[69] ? static_cast<int32_t>(atoi(row[69])) : 0;
			e.rain_chance2              = row[70] ? static_cast<int32_t>(atoi(row[70])) : 0;
			e.rain_chance3              = row[71] ? static_cast<int32_t>(atoi(row[71])) : 0;
			e.rain_chance4              = row[72] ? static_cast<int32_t>(atoi(row[72])) : 0;
			e.rain_duration1            = row[73] ? static_cast<int32_t>(atoi(row[73])) : 0;
			e.rain_duration2            = row[74] ? static_cast<int32_t>(atoi(row[74])) : 0;
			e.rain_duration3            = row[75] ? static_cast<int32_t>(atoi(row[75])) : 0;
			e.rain_duration4            = row[76] ? static_cast<int32_t>(atoi(row[76])) : 0;
			e.snow_chance1              = row[77] ? static_cast<int32_t>(atoi(row[77])) : 0;
			e.snow_chance2              = row[78] ? static_cast<int32_t>(atoi(row[78])) : 0;
			e.snow_chance3              = row[79] ? static_cast<int32_t>(atoi(row[79])) : 0;
			e.snow_chance4              = row[80] ? static_cast<int32_t>(atoi(row[80])) : 0;
			e.snow_duration1            = row[81] ? static_cast<int32_t>(atoi(row[81])) : 0;
			e.snow_duration2            = row[82] ? static_cast<int32_t>(atoi(row[82])) : 0;
			e.snow_duration3            = row[83] ? static_cast<int32_t>(atoi(row[83])) : 0;
			e.snow_duration4            = row[84] ? static_cast<int32_t>(atoi(row[84])) : 0;
			e.gravity                   = row[85] ? strtof(row[85], nullptr) : 0.4;
			e.type                      = row[86] ? static_cast<int32_t>(atoi(row[86])) : 0;
			e.skylock                   = row[87] ? static_cast<int8_t>(atoi(row[87])) : 0;
			e.fast_regen_hp             = row[88] ? static_cast<int32_t>(atoi(row[88])) : 180;
			e.fast_regen_mana           = row[89] ? static_cast<int32_t>(atoi(row[89])) : 180;
			e.fast_regen_endurance      = row[90] ? static_cast<int32_t>(atoi(row[90])) : 180;
			e.npc_max_aggro_dist        = row[91] ? static_cast<int32_t>(atoi(row[91])) : 600;
			e.client_update_range       = row[92] ? static_cast<uint32_t>(strtoul(row[92], nullptr, 10)) : 600;
			e.underworld_teleport_index = row[93] ? static_cast<int32_t>(atoi(row[93])) : 0;
			e.lava_damage               = row[94] ? static_cast<int32_t>(atoi(row[94])) : 50;
			e.min_lava_damage           = row[95] ? static_cast<int32_t>(atoi(row[95])) : 10;
			e.idle_when_empty           = row[96] ? static_cast<uint8_t>(strtoul(row[96], nullptr, 10)) : 1;
			e.seconds_before_idle       = row[97] ? static_cast<uint32_t>(strtoul(row[97], nullptr, 10)) : 60;
			e.shard_at_player_count     = row[98] ? static_cast<int32_t>(atoi(row[98])) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int zone_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				zone_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Zone &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.zoneidnumber));
		v.push_back(columns[2] + " = " + std::to_string(e.version));
		v.push_back(columns[3] + " = '" + Strings::Escape(e.short_name) + "'");
		v.push_back(columns[4] + " = '" + Strings::Escape(e.long_name) + "'");
		v.push_back(columns[5] + " = " + std::to_string(e.min_status));
		v.push_back(columns[6] + " = '" + Strings::Escape(e.map_file_name) + "'");
		v.push_back(columns[7] + " = '" + Strings::Escape(e.note) + "'");
		v.push_back(columns[8] + " = " + std::to_string(e.min_expansion));
		v.push_back(columns[9] + " = " + std::to_string(e.max_expansion));
		v.push_back(columns[10] + " = '" + Strings::Escape(e.content_flags) + "'");
		v.push_back(columns[11] + " = '" + Strings::Escape(e.content_flags_disabled) + "'");
		v.push_back(columns[12] + " = " + std::to_string(e.expansion));
		v.push_back(columns[13] + " = '" + Strings::Escape(e.file_name) + "'");
		v.push_back(columns[14] + " = " + std::to_string(e.safe_x));
		v.push_back(columns[15] + " = " + std::to_string(e.safe_y));
		v.push_back(columns[16] + " = " + std::to_string(e.safe_z));
		v.push_back(columns[17] + " = " + std::to_string(e.safe_heading));
		v.push_back(columns[18] + " = " + std::to_string(e.graveyard_id));
		v.push_back(columns[19] + " = " + std::to_string(e.min_level));
		v.push_back(columns[20] + " = " + std::to_string(e.max_level));
		v.push_back(columns[21] + " = " + std::to_string(e.timezone));
		v.push_back(columns[22] + " = " + std::to_string(e.maxclients));
		v.push_back(columns[23] + " = " + std::to_string(e.ruleset));
		v.push_back(columns[24] + " = " + std::to_string(e.underworld));
		v.push_back(columns[25] + " = " + std::to_string(e.minclip));
		v.push_back(columns[26] + " = " + std::to_string(e.maxclip));
		v.push_back(columns[27] + " = " + std::to_string(e.fog_minclip));
		v.push_back(columns[28] + " = " + std::to_string(e.fog_maxclip));
		v.push_back(columns[29] + " = " + std::to_string(e.fog_blue));
		v.push_back(columns[30] + " = " + std::to_string(e.fog_red));
		v.push_back(columns[31] + " = " + std::to_string(e.fog_green));
		v.push_back(columns[32] + " = " + std::to_string(e.sky));
		v.push_back(columns[33] + " = " + std::to_string(e.ztype));
		v.push_back(columns[34] + " = " + std::to_string(e.zone_exp_multiplier));
		v.push_back(columns[35] + " = " + std::to_string(e.walkspeed));
		v.push_back(columns[36] + " = " + std::to_string(e.time_type));
		v.push_back(columns[37] + " = " + std::to_string(e.fog_red1));
		v.push_back(columns[38] + " = " + std::to_string(e.fog_green1));
		v.push_back(columns[39] + " = " + std::to_string(e.fog_blue1));
		v.push_back(columns[40] + " = " + std::to_string(e.fog_minclip1));
		v.push_back(columns[41] + " = " + std::to_string(e.fog_maxclip1));
		v.push_back(columns[42] + " = " + std::to_string(e.fog_red2));
		v.push_back(columns[43] + " = " + std::to_string(e.fog_green2));
		v.push_back(columns[44] + " = " + std::to_string(e.fog_blue2));
		v.push_back(columns[45] + " = " + std::to_string(e.fog_minclip2));
		v.push_back(columns[46] + " = " + std::to_string(e.fog_maxclip2));
		v.push_back(columns[47] + " = " + std::to_string(e.fog_red3));
		v.push_back(columns[48] + " = " + std::to_string(e.fog_green3));
		v.push_back(columns[49] + " = " + std::to_string(e.fog_blue3));
		v.push_back(columns[50] + " = " + std::to_string(e.fog_minclip3));
		v.push_back(columns[51] + " = " + std::to_string(e.fog_maxclip3));
		v.push_back(columns[52] + " = " + std::to_string(e.fog_red4));
		v.push_back(columns[53] + " = " + std::to_string(e.fog_green4));
		v.push_back(columns[54] + " = " + std::to_string(e.fog_blue4));
		v.push_back(columns[55] + " = " + std::to_string(e.fog_minclip4));
		v.push_back(columns[56] + " = " + std::to_string(e.fog_maxclip4));
		v.push_back(columns[57] + " = " + std::to_string(e.fog_density));
		v.push_back(columns[58] + " = '" + Strings::Escape(e.flag_needed) + "'");
		v.push_back(columns[59] + " = " + std::to_string(e.canbind));
		v.push_back(columns[60] + " = " + std::to_string(e.cancombat));
		v.push_back(columns[61] + " = " + std::to_string(e.canlevitate));
		v.push_back(columns[62] + " = " + std::to_string(e.castoutdoor));
		v.push_back(columns[63] + " = " + std::to_string(e.hotzone));
		v.push_back(columns[64] + " = " + std::to_string(e.insttype));
		v.push_back(columns[65] + " = " + std::to_string(e.shutdowndelay));
		v.push_back(columns[66] + " = " + std::to_string(e.peqzone));
		v.push_back(columns[67] + " = " + std::to_string(e.bypass_expansion_check));
		v.push_back(columns[68] + " = " + std::to_string(e.suspendbuffs));
		v.push_back(columns[69] + " = " + std::to_string(e.rain_chance1));
		v.push_back(columns[70] + " = " + std::to_string(e.rain_chance2));
		v.push_back(columns[71] + " = " + std::to_string(e.rain_chance3));
		v.push_back(columns[72] + " = " + std::to_string(e.rain_chance4));
		v.push_back(columns[73] + " = " + std::to_string(e.rain_duration1));
		v.push_back(columns[74] + " = " + std::to_string(e.rain_duration2));
		v.push_back(columns[75] + " = " + std::to_string(e.rain_duration3));
		v.push_back(columns[76] + " = " + std::to_string(e.rain_duration4));
		v.push_back(columns[77] + " = " + std::to_string(e.snow_chance1));
		v.push_back(columns[78] + " = " + std::to_string(e.snow_chance2));
		v.push_back(columns[79] + " = " + std::to_string(e.snow_chance3));
		v.push_back(columns[80] + " = " + std::to_string(e.snow_chance4));
		v.push_back(columns[81] + " = " + std::to_string(e.snow_duration1));
		v.push_back(columns[82] + " = " + std::to_string(e.snow_duration2));
		v.push_back(columns[83] + " = " + std::to_string(e.snow_duration3));
		v.push_back(columns[84] + " = " + std::to_string(e.snow_duration4));
		v.push_back(columns[85] + " = " + std::to_string(e.gravity));
		v.push_back(columns[86] + " = " + std::to_string(e.type));
		v.push_back(columns[87] + " = " + std::to_string(e.skylock));
		v.push_back(columns[88] + " = " + std::to_string(e.fast_regen_hp));
		v.push_back(columns[89] + " = " + std::to_string(e.fast_regen_mana));
		v.push_back(columns[90] + " = " + std::to_string(e.fast_regen_endurance));
		v.push_back(columns[91] + " = " + std::to_string(e.npc_max_aggro_dist));
		v.push_back(columns[92] + " = " + std::to_string(e.client_update_range));
		v.push_back(columns[93] + " = " + std::to_string(e.underworld_teleport_index));
		v.push_back(columns[94] + " = " + std::to_string(e.lava_damage));
		v.push_back(columns[95] + " = " + std::to_string(e.min_lava_damage));
		v.push_back(columns[96] + " = " + std::to_string(e.idle_when_empty));
		v.push_back(columns[97] + " = " + std::to_string(e.seconds_before_idle));
		v.push_back(columns[98] + " = " + std::to_string(e.shard_at_player_count));

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

	static Zone InsertOne(
		Database& db,
		Zone e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.zoneidnumber));
		v.push_back(std::to_string(e.version));
		v.push_back("'" + Strings::Escape(e.short_name) + "'");
		v.push_back("'" + Strings::Escape(e.long_name) + "'");
		v.push_back(std::to_string(e.min_status));
		v.push_back("'" + Strings::Escape(e.map_file_name) + "'");
		v.push_back("'" + Strings::Escape(e.note) + "'");
		v.push_back(std::to_string(e.min_expansion));
		v.push_back(std::to_string(e.max_expansion));
		v.push_back("'" + Strings::Escape(e.content_flags) + "'");
		v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");
		v.push_back(std::to_string(e.expansion));
		v.push_back("'" + Strings::Escape(e.file_name) + "'");
		v.push_back(std::to_string(e.safe_x));
		v.push_back(std::to_string(e.safe_y));
		v.push_back(std::to_string(e.safe_z));
		v.push_back(std::to_string(e.safe_heading));
		v.push_back(std::to_string(e.graveyard_id));
		v.push_back(std::to_string(e.min_level));
		v.push_back(std::to_string(e.max_level));
		v.push_back(std::to_string(e.timezone));
		v.push_back(std::to_string(e.maxclients));
		v.push_back(std::to_string(e.ruleset));
		v.push_back(std::to_string(e.underworld));
		v.push_back(std::to_string(e.minclip));
		v.push_back(std::to_string(e.maxclip));
		v.push_back(std::to_string(e.fog_minclip));
		v.push_back(std::to_string(e.fog_maxclip));
		v.push_back(std::to_string(e.fog_blue));
		v.push_back(std::to_string(e.fog_red));
		v.push_back(std::to_string(e.fog_green));
		v.push_back(std::to_string(e.sky));
		v.push_back(std::to_string(e.ztype));
		v.push_back(std::to_string(e.zone_exp_multiplier));
		v.push_back(std::to_string(e.walkspeed));
		v.push_back(std::to_string(e.time_type));
		v.push_back(std::to_string(e.fog_red1));
		v.push_back(std::to_string(e.fog_green1));
		v.push_back(std::to_string(e.fog_blue1));
		v.push_back(std::to_string(e.fog_minclip1));
		v.push_back(std::to_string(e.fog_maxclip1));
		v.push_back(std::to_string(e.fog_red2));
		v.push_back(std::to_string(e.fog_green2));
		v.push_back(std::to_string(e.fog_blue2));
		v.push_back(std::to_string(e.fog_minclip2));
		v.push_back(std::to_string(e.fog_maxclip2));
		v.push_back(std::to_string(e.fog_red3));
		v.push_back(std::to_string(e.fog_green3));
		v.push_back(std::to_string(e.fog_blue3));
		v.push_back(std::to_string(e.fog_minclip3));
		v.push_back(std::to_string(e.fog_maxclip3));
		v.push_back(std::to_string(e.fog_red4));
		v.push_back(std::to_string(e.fog_green4));
		v.push_back(std::to_string(e.fog_blue4));
		v.push_back(std::to_string(e.fog_minclip4));
		v.push_back(std::to_string(e.fog_maxclip4));
		v.push_back(std::to_string(e.fog_density));
		v.push_back("'" + Strings::Escape(e.flag_needed) + "'");
		v.push_back(std::to_string(e.canbind));
		v.push_back(std::to_string(e.cancombat));
		v.push_back(std::to_string(e.canlevitate));
		v.push_back(std::to_string(e.castoutdoor));
		v.push_back(std::to_string(e.hotzone));
		v.push_back(std::to_string(e.insttype));
		v.push_back(std::to_string(e.shutdowndelay));
		v.push_back(std::to_string(e.peqzone));
		v.push_back(std::to_string(e.bypass_expansion_check));
		v.push_back(std::to_string(e.suspendbuffs));
		v.push_back(std::to_string(e.rain_chance1));
		v.push_back(std::to_string(e.rain_chance2));
		v.push_back(std::to_string(e.rain_chance3));
		v.push_back(std::to_string(e.rain_chance4));
		v.push_back(std::to_string(e.rain_duration1));
		v.push_back(std::to_string(e.rain_duration2));
		v.push_back(std::to_string(e.rain_duration3));
		v.push_back(std::to_string(e.rain_duration4));
		v.push_back(std::to_string(e.snow_chance1));
		v.push_back(std::to_string(e.snow_chance2));
		v.push_back(std::to_string(e.snow_chance3));
		v.push_back(std::to_string(e.snow_chance4));
		v.push_back(std::to_string(e.snow_duration1));
		v.push_back(std::to_string(e.snow_duration2));
		v.push_back(std::to_string(e.snow_duration3));
		v.push_back(std::to_string(e.snow_duration4));
		v.push_back(std::to_string(e.gravity));
		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.skylock));
		v.push_back(std::to_string(e.fast_regen_hp));
		v.push_back(std::to_string(e.fast_regen_mana));
		v.push_back(std::to_string(e.fast_regen_endurance));
		v.push_back(std::to_string(e.npc_max_aggro_dist));
		v.push_back(std::to_string(e.client_update_range));
		v.push_back(std::to_string(e.underworld_teleport_index));
		v.push_back(std::to_string(e.lava_damage));
		v.push_back(std::to_string(e.min_lava_damage));
		v.push_back(std::to_string(e.idle_when_empty));
		v.push_back(std::to_string(e.seconds_before_idle));
		v.push_back(std::to_string(e.shard_at_player_count));

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
		const std::vector<Zone> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.zoneidnumber));
			v.push_back(std::to_string(e.version));
			v.push_back("'" + Strings::Escape(e.short_name) + "'");
			v.push_back("'" + Strings::Escape(e.long_name) + "'");
			v.push_back(std::to_string(e.min_status));
			v.push_back("'" + Strings::Escape(e.map_file_name) + "'");
			v.push_back("'" + Strings::Escape(e.note) + "'");
			v.push_back(std::to_string(e.min_expansion));
			v.push_back(std::to_string(e.max_expansion));
			v.push_back("'" + Strings::Escape(e.content_flags) + "'");
			v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");
			v.push_back(std::to_string(e.expansion));
			v.push_back("'" + Strings::Escape(e.file_name) + "'");
			v.push_back(std::to_string(e.safe_x));
			v.push_back(std::to_string(e.safe_y));
			v.push_back(std::to_string(e.safe_z));
			v.push_back(std::to_string(e.safe_heading));
			v.push_back(std::to_string(e.graveyard_id));
			v.push_back(std::to_string(e.min_level));
			v.push_back(std::to_string(e.max_level));
			v.push_back(std::to_string(e.timezone));
			v.push_back(std::to_string(e.maxclients));
			v.push_back(std::to_string(e.ruleset));
			v.push_back(std::to_string(e.underworld));
			v.push_back(std::to_string(e.minclip));
			v.push_back(std::to_string(e.maxclip));
			v.push_back(std::to_string(e.fog_minclip));
			v.push_back(std::to_string(e.fog_maxclip));
			v.push_back(std::to_string(e.fog_blue));
			v.push_back(std::to_string(e.fog_red));
			v.push_back(std::to_string(e.fog_green));
			v.push_back(std::to_string(e.sky));
			v.push_back(std::to_string(e.ztype));
			v.push_back(std::to_string(e.zone_exp_multiplier));
			v.push_back(std::to_string(e.walkspeed));
			v.push_back(std::to_string(e.time_type));
			v.push_back(std::to_string(e.fog_red1));
			v.push_back(std::to_string(e.fog_green1));
			v.push_back(std::to_string(e.fog_blue1));
			v.push_back(std::to_string(e.fog_minclip1));
			v.push_back(std::to_string(e.fog_maxclip1));
			v.push_back(std::to_string(e.fog_red2));
			v.push_back(std::to_string(e.fog_green2));
			v.push_back(std::to_string(e.fog_blue2));
			v.push_back(std::to_string(e.fog_minclip2));
			v.push_back(std::to_string(e.fog_maxclip2));
			v.push_back(std::to_string(e.fog_red3));
			v.push_back(std::to_string(e.fog_green3));
			v.push_back(std::to_string(e.fog_blue3));
			v.push_back(std::to_string(e.fog_minclip3));
			v.push_back(std::to_string(e.fog_maxclip3));
			v.push_back(std::to_string(e.fog_red4));
			v.push_back(std::to_string(e.fog_green4));
			v.push_back(std::to_string(e.fog_blue4));
			v.push_back(std::to_string(e.fog_minclip4));
			v.push_back(std::to_string(e.fog_maxclip4));
			v.push_back(std::to_string(e.fog_density));
			v.push_back("'" + Strings::Escape(e.flag_needed) + "'");
			v.push_back(std::to_string(e.canbind));
			v.push_back(std::to_string(e.cancombat));
			v.push_back(std::to_string(e.canlevitate));
			v.push_back(std::to_string(e.castoutdoor));
			v.push_back(std::to_string(e.hotzone));
			v.push_back(std::to_string(e.insttype));
			v.push_back(std::to_string(e.shutdowndelay));
			v.push_back(std::to_string(e.peqzone));
			v.push_back(std::to_string(e.bypass_expansion_check));
			v.push_back(std::to_string(e.suspendbuffs));
			v.push_back(std::to_string(e.rain_chance1));
			v.push_back(std::to_string(e.rain_chance2));
			v.push_back(std::to_string(e.rain_chance3));
			v.push_back(std::to_string(e.rain_chance4));
			v.push_back(std::to_string(e.rain_duration1));
			v.push_back(std::to_string(e.rain_duration2));
			v.push_back(std::to_string(e.rain_duration3));
			v.push_back(std::to_string(e.rain_duration4));
			v.push_back(std::to_string(e.snow_chance1));
			v.push_back(std::to_string(e.snow_chance2));
			v.push_back(std::to_string(e.snow_chance3));
			v.push_back(std::to_string(e.snow_chance4));
			v.push_back(std::to_string(e.snow_duration1));
			v.push_back(std::to_string(e.snow_duration2));
			v.push_back(std::to_string(e.snow_duration3));
			v.push_back(std::to_string(e.snow_duration4));
			v.push_back(std::to_string(e.gravity));
			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.skylock));
			v.push_back(std::to_string(e.fast_regen_hp));
			v.push_back(std::to_string(e.fast_regen_mana));
			v.push_back(std::to_string(e.fast_regen_endurance));
			v.push_back(std::to_string(e.npc_max_aggro_dist));
			v.push_back(std::to_string(e.client_update_range));
			v.push_back(std::to_string(e.underworld_teleport_index));
			v.push_back(std::to_string(e.lava_damage));
			v.push_back(std::to_string(e.min_lava_damage));
			v.push_back(std::to_string(e.idle_when_empty));
			v.push_back(std::to_string(e.seconds_before_idle));
			v.push_back(std::to_string(e.shard_at_player_count));

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

	static std::vector<Zone> All(Database& db)
	{
		std::vector<Zone> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Zone e{};

			e.id                        = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.zoneidnumber              = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.version                   = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.short_name                = row[3] ? row[3] : "";
			e.long_name                 = row[4] ? row[4] : "";
			e.min_status                = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.map_file_name             = row[6] ? row[6] : "";
			e.note                      = row[7] ? row[7] : "";
			e.min_expansion             = row[8] ? static_cast<int8_t>(atoi(row[8])) : -1;
			e.max_expansion             = row[9] ? static_cast<int8_t>(atoi(row[9])) : -1;
			e.content_flags             = row[10] ? row[10] : "";
			e.content_flags_disabled    = row[11] ? row[11] : "";
			e.expansion                 = row[12] ? static_cast<int8_t>(atoi(row[12])) : 0;
			e.file_name                 = row[13] ? row[13] : "";
			e.safe_x                    = row[14] ? strtof(row[14], nullptr) : 0;
			e.safe_y                    = row[15] ? strtof(row[15], nullptr) : 0;
			e.safe_z                    = row[16] ? strtof(row[16], nullptr) : 0;
			e.safe_heading              = row[17] ? strtof(row[17], nullptr) : 0;
			e.graveyard_id              = row[18] ? strtof(row[18], nullptr) : 0;
			e.min_level                 = row[19] ? static_cast<uint8_t>(strtoul(row[19], nullptr, 10)) : 0;
			e.max_level                 = row[20] ? static_cast<uint8_t>(strtoul(row[20], nullptr, 10)) : 255;
			e.timezone                  = row[21] ? static_cast<int32_t>(atoi(row[21])) : 0;
			e.maxclients                = row[22] ? static_cast<int32_t>(atoi(row[22])) : 0;
			e.ruleset                   = row[23] ? static_cast<uint32_t>(strtoul(row[23], nullptr, 10)) : 0;
			e.underworld                = row[24] ? strtof(row[24], nullptr) : 0;
			e.minclip                   = row[25] ? strtof(row[25], nullptr) : 450;
			e.maxclip                   = row[26] ? strtof(row[26], nullptr) : 450;
			e.fog_minclip               = row[27] ? strtof(row[27], nullptr) : 450;
			e.fog_maxclip               = row[28] ? strtof(row[28], nullptr) : 450;
			e.fog_blue                  = row[29] ? static_cast<uint8_t>(strtoul(row[29], nullptr, 10)) : 0;
			e.fog_red                   = row[30] ? static_cast<uint8_t>(strtoul(row[30], nullptr, 10)) : 0;
			e.fog_green                 = row[31] ? static_cast<uint8_t>(strtoul(row[31], nullptr, 10)) : 0;
			e.sky                       = row[32] ? static_cast<uint8_t>(strtoul(row[32], nullptr, 10)) : 1;
			e.ztype                     = row[33] ? static_cast<uint8_t>(strtoul(row[33], nullptr, 10)) : 1;
			e.zone_exp_multiplier       = row[34] ? strtof(row[34], nullptr) : 0.00;
			e.walkspeed                 = row[35] ? strtof(row[35], nullptr) : 0.4;
			e.time_type                 = row[36] ? static_cast<uint8_t>(strtoul(row[36], nullptr, 10)) : 2;
			e.fog_red1                  = row[37] ? static_cast<uint8_t>(strtoul(row[37], nullptr, 10)) : 0;
			e.fog_green1                = row[38] ? static_cast<uint8_t>(strtoul(row[38], nullptr, 10)) : 0;
			e.fog_blue1                 = row[39] ? static_cast<uint8_t>(strtoul(row[39], nullptr, 10)) : 0;
			e.fog_minclip1              = row[40] ? strtof(row[40], nullptr) : 450;
			e.fog_maxclip1              = row[41] ? strtof(row[41], nullptr) : 450;
			e.fog_red2                  = row[42] ? static_cast<uint8_t>(strtoul(row[42], nullptr, 10)) : 0;
			e.fog_green2                = row[43] ? static_cast<uint8_t>(strtoul(row[43], nullptr, 10)) : 0;
			e.fog_blue2                 = row[44] ? static_cast<uint8_t>(strtoul(row[44], nullptr, 10)) : 0;
			e.fog_minclip2              = row[45] ? strtof(row[45], nullptr) : 450;
			e.fog_maxclip2              = row[46] ? strtof(row[46], nullptr) : 450;
			e.fog_red3                  = row[47] ? static_cast<uint8_t>(strtoul(row[47], nullptr, 10)) : 0;
			e.fog_green3                = row[48] ? static_cast<uint8_t>(strtoul(row[48], nullptr, 10)) : 0;
			e.fog_blue3                 = row[49] ? static_cast<uint8_t>(strtoul(row[49], nullptr, 10)) : 0;
			e.fog_minclip3              = row[50] ? strtof(row[50], nullptr) : 450;
			e.fog_maxclip3              = row[51] ? strtof(row[51], nullptr) : 450;
			e.fog_red4                  = row[52] ? static_cast<uint8_t>(strtoul(row[52], nullptr, 10)) : 0;
			e.fog_green4                = row[53] ? static_cast<uint8_t>(strtoul(row[53], nullptr, 10)) : 0;
			e.fog_blue4                 = row[54] ? static_cast<uint8_t>(strtoul(row[54], nullptr, 10)) : 0;
			e.fog_minclip4              = row[55] ? strtof(row[55], nullptr) : 450;
			e.fog_maxclip4              = row[56] ? strtof(row[56], nullptr) : 450;
			e.fog_density               = row[57] ? strtof(row[57], nullptr) : 0;
			e.flag_needed               = row[58] ? row[58] : "";
			e.canbind                   = row[59] ? static_cast<int8_t>(atoi(row[59])) : 1;
			e.cancombat                 = row[60] ? static_cast<int8_t>(atoi(row[60])) : 1;
			e.canlevitate               = row[61] ? static_cast<int8_t>(atoi(row[61])) : 1;
			e.castoutdoor               = row[62] ? static_cast<int8_t>(atoi(row[62])) : 1;
			e.hotzone                   = row[63] ? static_cast<uint8_t>(strtoul(row[63], nullptr, 10)) : 0;
			e.insttype                  = row[64] ? static_cast<uint8_t>(strtoul(row[64], nullptr, 10)) : 0;
			e.shutdowndelay             = row[65] ? strtoull(row[65], nullptr, 10) : 5000;
			e.peqzone                   = row[66] ? static_cast<int8_t>(atoi(row[66])) : 1;
			e.bypass_expansion_check    = row[67] ? static_cast<int8_t>(atoi(row[67])) : 0;
			e.suspendbuffs              = row[68] ? static_cast<uint8_t>(strtoul(row[68], nullptr, 10)) : 0;
			e.rain_chance1              = row[69] ? static_cast<int32_t>(atoi(row[69])) : 0;
			e.rain_chance2              = row[70] ? static_cast<int32_t>(atoi(row[70])) : 0;
			e.rain_chance3              = row[71] ? static_cast<int32_t>(atoi(row[71])) : 0;
			e.rain_chance4              = row[72] ? static_cast<int32_t>(atoi(row[72])) : 0;
			e.rain_duration1            = row[73] ? static_cast<int32_t>(atoi(row[73])) : 0;
			e.rain_duration2            = row[74] ? static_cast<int32_t>(atoi(row[74])) : 0;
			e.rain_duration3            = row[75] ? static_cast<int32_t>(atoi(row[75])) : 0;
			e.rain_duration4            = row[76] ? static_cast<int32_t>(atoi(row[76])) : 0;
			e.snow_chance1              = row[77] ? static_cast<int32_t>(atoi(row[77])) : 0;
			e.snow_chance2              = row[78] ? static_cast<int32_t>(atoi(row[78])) : 0;
			e.snow_chance3              = row[79] ? static_cast<int32_t>(atoi(row[79])) : 0;
			e.snow_chance4              = row[80] ? static_cast<int32_t>(atoi(row[80])) : 0;
			e.snow_duration1            = row[81] ? static_cast<int32_t>(atoi(row[81])) : 0;
			e.snow_duration2            = row[82] ? static_cast<int32_t>(atoi(row[82])) : 0;
			e.snow_duration3            = row[83] ? static_cast<int32_t>(atoi(row[83])) : 0;
			e.snow_duration4            = row[84] ? static_cast<int32_t>(atoi(row[84])) : 0;
			e.gravity                   = row[85] ? strtof(row[85], nullptr) : 0.4;
			e.type                      = row[86] ? static_cast<int32_t>(atoi(row[86])) : 0;
			e.skylock                   = row[87] ? static_cast<int8_t>(atoi(row[87])) : 0;
			e.fast_regen_hp             = row[88] ? static_cast<int32_t>(atoi(row[88])) : 180;
			e.fast_regen_mana           = row[89] ? static_cast<int32_t>(atoi(row[89])) : 180;
			e.fast_regen_endurance      = row[90] ? static_cast<int32_t>(atoi(row[90])) : 180;
			e.npc_max_aggro_dist        = row[91] ? static_cast<int32_t>(atoi(row[91])) : 600;
			e.client_update_range       = row[92] ? static_cast<uint32_t>(strtoul(row[92], nullptr, 10)) : 600;
			e.underworld_teleport_index = row[93] ? static_cast<int32_t>(atoi(row[93])) : 0;
			e.lava_damage               = row[94] ? static_cast<int32_t>(atoi(row[94])) : 50;
			e.min_lava_damage           = row[95] ? static_cast<int32_t>(atoi(row[95])) : 10;
			e.idle_when_empty           = row[96] ? static_cast<uint8_t>(strtoul(row[96], nullptr, 10)) : 1;
			e.seconds_before_idle       = row[97] ? static_cast<uint32_t>(strtoul(row[97], nullptr, 10)) : 60;
			e.shard_at_player_count     = row[98] ? static_cast<int32_t>(atoi(row[98])) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Zone> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Zone> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Zone e{};

			e.id                        = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.zoneidnumber              = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.version                   = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.short_name                = row[3] ? row[3] : "";
			e.long_name                 = row[4] ? row[4] : "";
			e.min_status                = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.map_file_name             = row[6] ? row[6] : "";
			e.note                      = row[7] ? row[7] : "";
			e.min_expansion             = row[8] ? static_cast<int8_t>(atoi(row[8])) : -1;
			e.max_expansion             = row[9] ? static_cast<int8_t>(atoi(row[9])) : -1;
			e.content_flags             = row[10] ? row[10] : "";
			e.content_flags_disabled    = row[11] ? row[11] : "";
			e.expansion                 = row[12] ? static_cast<int8_t>(atoi(row[12])) : 0;
			e.file_name                 = row[13] ? row[13] : "";
			e.safe_x                    = row[14] ? strtof(row[14], nullptr) : 0;
			e.safe_y                    = row[15] ? strtof(row[15], nullptr) : 0;
			e.safe_z                    = row[16] ? strtof(row[16], nullptr) : 0;
			e.safe_heading              = row[17] ? strtof(row[17], nullptr) : 0;
			e.graveyard_id              = row[18] ? strtof(row[18], nullptr) : 0;
			e.min_level                 = row[19] ? static_cast<uint8_t>(strtoul(row[19], nullptr, 10)) : 0;
			e.max_level                 = row[20] ? static_cast<uint8_t>(strtoul(row[20], nullptr, 10)) : 255;
			e.timezone                  = row[21] ? static_cast<int32_t>(atoi(row[21])) : 0;
			e.maxclients                = row[22] ? static_cast<int32_t>(atoi(row[22])) : 0;
			e.ruleset                   = row[23] ? static_cast<uint32_t>(strtoul(row[23], nullptr, 10)) : 0;
			e.underworld                = row[24] ? strtof(row[24], nullptr) : 0;
			e.minclip                   = row[25] ? strtof(row[25], nullptr) : 450;
			e.maxclip                   = row[26] ? strtof(row[26], nullptr) : 450;
			e.fog_minclip               = row[27] ? strtof(row[27], nullptr) : 450;
			e.fog_maxclip               = row[28] ? strtof(row[28], nullptr) : 450;
			e.fog_blue                  = row[29] ? static_cast<uint8_t>(strtoul(row[29], nullptr, 10)) : 0;
			e.fog_red                   = row[30] ? static_cast<uint8_t>(strtoul(row[30], nullptr, 10)) : 0;
			e.fog_green                 = row[31] ? static_cast<uint8_t>(strtoul(row[31], nullptr, 10)) : 0;
			e.sky                       = row[32] ? static_cast<uint8_t>(strtoul(row[32], nullptr, 10)) : 1;
			e.ztype                     = row[33] ? static_cast<uint8_t>(strtoul(row[33], nullptr, 10)) : 1;
			e.zone_exp_multiplier       = row[34] ? strtof(row[34], nullptr) : 0.00;
			e.walkspeed                 = row[35] ? strtof(row[35], nullptr) : 0.4;
			e.time_type                 = row[36] ? static_cast<uint8_t>(strtoul(row[36], nullptr, 10)) : 2;
			e.fog_red1                  = row[37] ? static_cast<uint8_t>(strtoul(row[37], nullptr, 10)) : 0;
			e.fog_green1                = row[38] ? static_cast<uint8_t>(strtoul(row[38], nullptr, 10)) : 0;
			e.fog_blue1                 = row[39] ? static_cast<uint8_t>(strtoul(row[39], nullptr, 10)) : 0;
			e.fog_minclip1              = row[40] ? strtof(row[40], nullptr) : 450;
			e.fog_maxclip1              = row[41] ? strtof(row[41], nullptr) : 450;
			e.fog_red2                  = row[42] ? static_cast<uint8_t>(strtoul(row[42], nullptr, 10)) : 0;
			e.fog_green2                = row[43] ? static_cast<uint8_t>(strtoul(row[43], nullptr, 10)) : 0;
			e.fog_blue2                 = row[44] ? static_cast<uint8_t>(strtoul(row[44], nullptr, 10)) : 0;
			e.fog_minclip2              = row[45] ? strtof(row[45], nullptr) : 450;
			e.fog_maxclip2              = row[46] ? strtof(row[46], nullptr) : 450;
			e.fog_red3                  = row[47] ? static_cast<uint8_t>(strtoul(row[47], nullptr, 10)) : 0;
			e.fog_green3                = row[48] ? static_cast<uint8_t>(strtoul(row[48], nullptr, 10)) : 0;
			e.fog_blue3                 = row[49] ? static_cast<uint8_t>(strtoul(row[49], nullptr, 10)) : 0;
			e.fog_minclip3              = row[50] ? strtof(row[50], nullptr) : 450;
			e.fog_maxclip3              = row[51] ? strtof(row[51], nullptr) : 450;
			e.fog_red4                  = row[52] ? static_cast<uint8_t>(strtoul(row[52], nullptr, 10)) : 0;
			e.fog_green4                = row[53] ? static_cast<uint8_t>(strtoul(row[53], nullptr, 10)) : 0;
			e.fog_blue4                 = row[54] ? static_cast<uint8_t>(strtoul(row[54], nullptr, 10)) : 0;
			e.fog_minclip4              = row[55] ? strtof(row[55], nullptr) : 450;
			e.fog_maxclip4              = row[56] ? strtof(row[56], nullptr) : 450;
			e.fog_density               = row[57] ? strtof(row[57], nullptr) : 0;
			e.flag_needed               = row[58] ? row[58] : "";
			e.canbind                   = row[59] ? static_cast<int8_t>(atoi(row[59])) : 1;
			e.cancombat                 = row[60] ? static_cast<int8_t>(atoi(row[60])) : 1;
			e.canlevitate               = row[61] ? static_cast<int8_t>(atoi(row[61])) : 1;
			e.castoutdoor               = row[62] ? static_cast<int8_t>(atoi(row[62])) : 1;
			e.hotzone                   = row[63] ? static_cast<uint8_t>(strtoul(row[63], nullptr, 10)) : 0;
			e.insttype                  = row[64] ? static_cast<uint8_t>(strtoul(row[64], nullptr, 10)) : 0;
			e.shutdowndelay             = row[65] ? strtoull(row[65], nullptr, 10) : 5000;
			e.peqzone                   = row[66] ? static_cast<int8_t>(atoi(row[66])) : 1;
			e.bypass_expansion_check    = row[67] ? static_cast<int8_t>(atoi(row[67])) : 0;
			e.suspendbuffs              = row[68] ? static_cast<uint8_t>(strtoul(row[68], nullptr, 10)) : 0;
			e.rain_chance1              = row[69] ? static_cast<int32_t>(atoi(row[69])) : 0;
			e.rain_chance2              = row[70] ? static_cast<int32_t>(atoi(row[70])) : 0;
			e.rain_chance3              = row[71] ? static_cast<int32_t>(atoi(row[71])) : 0;
			e.rain_chance4              = row[72] ? static_cast<int32_t>(atoi(row[72])) : 0;
			e.rain_duration1            = row[73] ? static_cast<int32_t>(atoi(row[73])) : 0;
			e.rain_duration2            = row[74] ? static_cast<int32_t>(atoi(row[74])) : 0;
			e.rain_duration3            = row[75] ? static_cast<int32_t>(atoi(row[75])) : 0;
			e.rain_duration4            = row[76] ? static_cast<int32_t>(atoi(row[76])) : 0;
			e.snow_chance1              = row[77] ? static_cast<int32_t>(atoi(row[77])) : 0;
			e.snow_chance2              = row[78] ? static_cast<int32_t>(atoi(row[78])) : 0;
			e.snow_chance3              = row[79] ? static_cast<int32_t>(atoi(row[79])) : 0;
			e.snow_chance4              = row[80] ? static_cast<int32_t>(atoi(row[80])) : 0;
			e.snow_duration1            = row[81] ? static_cast<int32_t>(atoi(row[81])) : 0;
			e.snow_duration2            = row[82] ? static_cast<int32_t>(atoi(row[82])) : 0;
			e.snow_duration3            = row[83] ? static_cast<int32_t>(atoi(row[83])) : 0;
			e.snow_duration4            = row[84] ? static_cast<int32_t>(atoi(row[84])) : 0;
			e.gravity                   = row[85] ? strtof(row[85], nullptr) : 0.4;
			e.type                      = row[86] ? static_cast<int32_t>(atoi(row[86])) : 0;
			e.skylock                   = row[87] ? static_cast<int8_t>(atoi(row[87])) : 0;
			e.fast_regen_hp             = row[88] ? static_cast<int32_t>(atoi(row[88])) : 180;
			e.fast_regen_mana           = row[89] ? static_cast<int32_t>(atoi(row[89])) : 180;
			e.fast_regen_endurance      = row[90] ? static_cast<int32_t>(atoi(row[90])) : 180;
			e.npc_max_aggro_dist        = row[91] ? static_cast<int32_t>(atoi(row[91])) : 600;
			e.client_update_range       = row[92] ? static_cast<uint32_t>(strtoul(row[92], nullptr, 10)) : 600;
			e.underworld_teleport_index = row[93] ? static_cast<int32_t>(atoi(row[93])) : 0;
			e.lava_damage               = row[94] ? static_cast<int32_t>(atoi(row[94])) : 50;
			e.min_lava_damage           = row[95] ? static_cast<int32_t>(atoi(row[95])) : 10;
			e.idle_when_empty           = row[96] ? static_cast<uint8_t>(strtoul(row[96], nullptr, 10)) : 1;
			e.seconds_before_idle       = row[97] ? static_cast<uint32_t>(strtoul(row[97], nullptr, 10)) : 60;
			e.shard_at_player_count     = row[98] ? static_cast<int32_t>(atoi(row[98])) : 0;

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
		const Zone &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.zoneidnumber));
		v.push_back(std::to_string(e.version));
		v.push_back("'" + Strings::Escape(e.short_name) + "'");
		v.push_back("'" + Strings::Escape(e.long_name) + "'");
		v.push_back(std::to_string(e.min_status));
		v.push_back("'" + Strings::Escape(e.map_file_name) + "'");
		v.push_back("'" + Strings::Escape(e.note) + "'");
		v.push_back(std::to_string(e.min_expansion));
		v.push_back(std::to_string(e.max_expansion));
		v.push_back("'" + Strings::Escape(e.content_flags) + "'");
		v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");
		v.push_back(std::to_string(e.expansion));
		v.push_back("'" + Strings::Escape(e.file_name) + "'");
		v.push_back(std::to_string(e.safe_x));
		v.push_back(std::to_string(e.safe_y));
		v.push_back(std::to_string(e.safe_z));
		v.push_back(std::to_string(e.safe_heading));
		v.push_back(std::to_string(e.graveyard_id));
		v.push_back(std::to_string(e.min_level));
		v.push_back(std::to_string(e.max_level));
		v.push_back(std::to_string(e.timezone));
		v.push_back(std::to_string(e.maxclients));
		v.push_back(std::to_string(e.ruleset));
		v.push_back(std::to_string(e.underworld));
		v.push_back(std::to_string(e.minclip));
		v.push_back(std::to_string(e.maxclip));
		v.push_back(std::to_string(e.fog_minclip));
		v.push_back(std::to_string(e.fog_maxclip));
		v.push_back(std::to_string(e.fog_blue));
		v.push_back(std::to_string(e.fog_red));
		v.push_back(std::to_string(e.fog_green));
		v.push_back(std::to_string(e.sky));
		v.push_back(std::to_string(e.ztype));
		v.push_back(std::to_string(e.zone_exp_multiplier));
		v.push_back(std::to_string(e.walkspeed));
		v.push_back(std::to_string(e.time_type));
		v.push_back(std::to_string(e.fog_red1));
		v.push_back(std::to_string(e.fog_green1));
		v.push_back(std::to_string(e.fog_blue1));
		v.push_back(std::to_string(e.fog_minclip1));
		v.push_back(std::to_string(e.fog_maxclip1));
		v.push_back(std::to_string(e.fog_red2));
		v.push_back(std::to_string(e.fog_green2));
		v.push_back(std::to_string(e.fog_blue2));
		v.push_back(std::to_string(e.fog_minclip2));
		v.push_back(std::to_string(e.fog_maxclip2));
		v.push_back(std::to_string(e.fog_red3));
		v.push_back(std::to_string(e.fog_green3));
		v.push_back(std::to_string(e.fog_blue3));
		v.push_back(std::to_string(e.fog_minclip3));
		v.push_back(std::to_string(e.fog_maxclip3));
		v.push_back(std::to_string(e.fog_red4));
		v.push_back(std::to_string(e.fog_green4));
		v.push_back(std::to_string(e.fog_blue4));
		v.push_back(std::to_string(e.fog_minclip4));
		v.push_back(std::to_string(e.fog_maxclip4));
		v.push_back(std::to_string(e.fog_density));
		v.push_back("'" + Strings::Escape(e.flag_needed) + "'");
		v.push_back(std::to_string(e.canbind));
		v.push_back(std::to_string(e.cancombat));
		v.push_back(std::to_string(e.canlevitate));
		v.push_back(std::to_string(e.castoutdoor));
		v.push_back(std::to_string(e.hotzone));
		v.push_back(std::to_string(e.insttype));
		v.push_back(std::to_string(e.shutdowndelay));
		v.push_back(std::to_string(e.peqzone));
		v.push_back(std::to_string(e.bypass_expansion_check));
		v.push_back(std::to_string(e.suspendbuffs));
		v.push_back(std::to_string(e.rain_chance1));
		v.push_back(std::to_string(e.rain_chance2));
		v.push_back(std::to_string(e.rain_chance3));
		v.push_back(std::to_string(e.rain_chance4));
		v.push_back(std::to_string(e.rain_duration1));
		v.push_back(std::to_string(e.rain_duration2));
		v.push_back(std::to_string(e.rain_duration3));
		v.push_back(std::to_string(e.rain_duration4));
		v.push_back(std::to_string(e.snow_chance1));
		v.push_back(std::to_string(e.snow_chance2));
		v.push_back(std::to_string(e.snow_chance3));
		v.push_back(std::to_string(e.snow_chance4));
		v.push_back(std::to_string(e.snow_duration1));
		v.push_back(std::to_string(e.snow_duration2));
		v.push_back(std::to_string(e.snow_duration3));
		v.push_back(std::to_string(e.snow_duration4));
		v.push_back(std::to_string(e.gravity));
		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.skylock));
		v.push_back(std::to_string(e.fast_regen_hp));
		v.push_back(std::to_string(e.fast_regen_mana));
		v.push_back(std::to_string(e.fast_regen_endurance));
		v.push_back(std::to_string(e.npc_max_aggro_dist));
		v.push_back(std::to_string(e.client_update_range));
		v.push_back(std::to_string(e.underworld_teleport_index));
		v.push_back(std::to_string(e.lava_damage));
		v.push_back(std::to_string(e.min_lava_damage));
		v.push_back(std::to_string(e.idle_when_empty));
		v.push_back(std::to_string(e.seconds_before_idle));
		v.push_back(std::to_string(e.shard_at_player_count));

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
		const std::vector<Zone> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.zoneidnumber));
			v.push_back(std::to_string(e.version));
			v.push_back("'" + Strings::Escape(e.short_name) + "'");
			v.push_back("'" + Strings::Escape(e.long_name) + "'");
			v.push_back(std::to_string(e.min_status));
			v.push_back("'" + Strings::Escape(e.map_file_name) + "'");
			v.push_back("'" + Strings::Escape(e.note) + "'");
			v.push_back(std::to_string(e.min_expansion));
			v.push_back(std::to_string(e.max_expansion));
			v.push_back("'" + Strings::Escape(e.content_flags) + "'");
			v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");
			v.push_back(std::to_string(e.expansion));
			v.push_back("'" + Strings::Escape(e.file_name) + "'");
			v.push_back(std::to_string(e.safe_x));
			v.push_back(std::to_string(e.safe_y));
			v.push_back(std::to_string(e.safe_z));
			v.push_back(std::to_string(e.safe_heading));
			v.push_back(std::to_string(e.graveyard_id));
			v.push_back(std::to_string(e.min_level));
			v.push_back(std::to_string(e.max_level));
			v.push_back(std::to_string(e.timezone));
			v.push_back(std::to_string(e.maxclients));
			v.push_back(std::to_string(e.ruleset));
			v.push_back(std::to_string(e.underworld));
			v.push_back(std::to_string(e.minclip));
			v.push_back(std::to_string(e.maxclip));
			v.push_back(std::to_string(e.fog_minclip));
			v.push_back(std::to_string(e.fog_maxclip));
			v.push_back(std::to_string(e.fog_blue));
			v.push_back(std::to_string(e.fog_red));
			v.push_back(std::to_string(e.fog_green));
			v.push_back(std::to_string(e.sky));
			v.push_back(std::to_string(e.ztype));
			v.push_back(std::to_string(e.zone_exp_multiplier));
			v.push_back(std::to_string(e.walkspeed));
			v.push_back(std::to_string(e.time_type));
			v.push_back(std::to_string(e.fog_red1));
			v.push_back(std::to_string(e.fog_green1));
			v.push_back(std::to_string(e.fog_blue1));
			v.push_back(std::to_string(e.fog_minclip1));
			v.push_back(std::to_string(e.fog_maxclip1));
			v.push_back(std::to_string(e.fog_red2));
			v.push_back(std::to_string(e.fog_green2));
			v.push_back(std::to_string(e.fog_blue2));
			v.push_back(std::to_string(e.fog_minclip2));
			v.push_back(std::to_string(e.fog_maxclip2));
			v.push_back(std::to_string(e.fog_red3));
			v.push_back(std::to_string(e.fog_green3));
			v.push_back(std::to_string(e.fog_blue3));
			v.push_back(std::to_string(e.fog_minclip3));
			v.push_back(std::to_string(e.fog_maxclip3));
			v.push_back(std::to_string(e.fog_red4));
			v.push_back(std::to_string(e.fog_green4));
			v.push_back(std::to_string(e.fog_blue4));
			v.push_back(std::to_string(e.fog_minclip4));
			v.push_back(std::to_string(e.fog_maxclip4));
			v.push_back(std::to_string(e.fog_density));
			v.push_back("'" + Strings::Escape(e.flag_needed) + "'");
			v.push_back(std::to_string(e.canbind));
			v.push_back(std::to_string(e.cancombat));
			v.push_back(std::to_string(e.canlevitate));
			v.push_back(std::to_string(e.castoutdoor));
			v.push_back(std::to_string(e.hotzone));
			v.push_back(std::to_string(e.insttype));
			v.push_back(std::to_string(e.shutdowndelay));
			v.push_back(std::to_string(e.peqzone));
			v.push_back(std::to_string(e.bypass_expansion_check));
			v.push_back(std::to_string(e.suspendbuffs));
			v.push_back(std::to_string(e.rain_chance1));
			v.push_back(std::to_string(e.rain_chance2));
			v.push_back(std::to_string(e.rain_chance3));
			v.push_back(std::to_string(e.rain_chance4));
			v.push_back(std::to_string(e.rain_duration1));
			v.push_back(std::to_string(e.rain_duration2));
			v.push_back(std::to_string(e.rain_duration3));
			v.push_back(std::to_string(e.rain_duration4));
			v.push_back(std::to_string(e.snow_chance1));
			v.push_back(std::to_string(e.snow_chance2));
			v.push_back(std::to_string(e.snow_chance3));
			v.push_back(std::to_string(e.snow_chance4));
			v.push_back(std::to_string(e.snow_duration1));
			v.push_back(std::to_string(e.snow_duration2));
			v.push_back(std::to_string(e.snow_duration3));
			v.push_back(std::to_string(e.snow_duration4));
			v.push_back(std::to_string(e.gravity));
			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.skylock));
			v.push_back(std::to_string(e.fast_regen_hp));
			v.push_back(std::to_string(e.fast_regen_mana));
			v.push_back(std::to_string(e.fast_regen_endurance));
			v.push_back(std::to_string(e.npc_max_aggro_dist));
			v.push_back(std::to_string(e.client_update_range));
			v.push_back(std::to_string(e.underworld_teleport_index));
			v.push_back(std::to_string(e.lava_damage));
			v.push_back(std::to_string(e.min_lava_damage));
			v.push_back(std::to_string(e.idle_when_empty));
			v.push_back(std::to_string(e.seconds_before_idle));
			v.push_back(std::to_string(e.shard_at_player_count));

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
