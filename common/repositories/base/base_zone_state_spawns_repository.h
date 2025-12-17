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

class BaseZoneStateSpawnsRepository {
public:
	struct ZoneStateSpawns {
		int64_t     id;
		uint32_t    zone_id;
		uint32_t    instance_id;
		int8_t      is_corpse;
		int8_t      is_zone;
		int32_t     decay_in_seconds;
		uint32_t    npc_id;
		uint32_t    spawn2_id;
		uint32_t    spawngroup_id;
		float       x;
		float       y;
		float       z;
		float       heading;
		uint32_t    respawn_time;
		uint32_t    variance;
		uint32_t    grid;
		int32_t     current_waypoint;
		int16_t     path_when_zone_idle;
		uint16_t    condition_id;
		int16_t     condition_min_value;
		int16_t     enabled;
		uint16_t    anim;
		std::string loot_data;
		std::string entity_variables;
		std::string buffs;
		int64_t     hp;
		int64_t     mana;
		int64_t     endurance;
		time_t      created_at;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"zone_id",
			"instance_id",
			"is_corpse",
			"is_zone",
			"decay_in_seconds",
			"npc_id",
			"spawn2_id",
			"spawngroup_id",
			"x",
			"y",
			"z",
			"heading",
			"respawn_time",
			"variance",
			"grid",
			"current_waypoint",
			"path_when_zone_idle",
			"condition_id",
			"condition_min_value",
			"enabled",
			"anim",
			"loot_data",
			"entity_variables",
			"buffs",
			"hp",
			"mana",
			"endurance",
			"created_at",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"zone_id",
			"instance_id",
			"is_corpse",
			"is_zone",
			"decay_in_seconds",
			"npc_id",
			"spawn2_id",
			"spawngroup_id",
			"x",
			"y",
			"z",
			"heading",
			"respawn_time",
			"variance",
			"grid",
			"current_waypoint",
			"path_when_zone_idle",
			"condition_id",
			"condition_min_value",
			"enabled",
			"anim",
			"loot_data",
			"entity_variables",
			"buffs",
			"hp",
			"mana",
			"endurance",
			"UNIX_TIMESTAMP(created_at)",
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
		return std::string("zone_state_spawns");
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

	static ZoneStateSpawns NewEntity()
	{
		ZoneStateSpawns e{};

		e.id                  = 0;
		e.zone_id             = 0;
		e.instance_id         = 0;
		e.is_corpse           = 0;
		e.is_zone             = 0;
		e.decay_in_seconds    = 0;
		e.npc_id              = 0;
		e.spawn2_id           = 0;
		e.spawngroup_id       = 0;
		e.x                   = 0;
		e.y                   = 0;
		e.z                   = 0;
		e.heading             = 0;
		e.respawn_time        = 0;
		e.variance            = 0;
		e.grid                = 0;
		e.current_waypoint    = 0;
		e.path_when_zone_idle = 0;
		e.condition_id        = 0;
		e.condition_min_value = 0;
		e.enabled             = 1;
		e.anim                = 0;
		e.loot_data           = "";
		e.entity_variables    = "";
		e.buffs               = "";
		e.hp                  = 0;
		e.mana                = 0;
		e.endurance           = 0;
		e.created_at          = 0;

		return e;
	}

	static ZoneStateSpawns GetZoneStateSpawns(
		const std::vector<ZoneStateSpawns> &zone_state_spawnss,
		int zone_state_spawns_id
	)
	{
		for (auto &zone_state_spawns : zone_state_spawnss) {
			if (zone_state_spawns.id == zone_state_spawns_id) {
				return zone_state_spawns;
			}
		}

		return NewEntity();
	}

	static ZoneStateSpawns FindOne(
		Database& db,
		int zone_state_spawns_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				zone_state_spawns_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			ZoneStateSpawns e{};

			e.id                  = row[0] ? strtoll(row[0], nullptr, 10) : 0;
			e.zone_id             = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.instance_id         = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.is_corpse           = row[3] ? static_cast<int8_t>(atoi(row[3])) : 0;
			e.is_zone             = row[4] ? static_cast<int8_t>(atoi(row[4])) : 0;
			e.decay_in_seconds    = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.npc_id              = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.spawn2_id           = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.spawngroup_id       = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.x                   = row[9] ? strtof(row[9], nullptr) : 0;
			e.y                   = row[10] ? strtof(row[10], nullptr) : 0;
			e.z                   = row[11] ? strtof(row[11], nullptr) : 0;
			e.heading             = row[12] ? strtof(row[12], nullptr) : 0;
			e.respawn_time        = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.variance            = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.grid                = row[15] ? static_cast<uint32_t>(strtoul(row[15], nullptr, 10)) : 0;
			e.current_waypoint    = row[16] ? static_cast<int32_t>(atoi(row[16])) : 0;
			e.path_when_zone_idle = row[17] ? static_cast<int16_t>(atoi(row[17])) : 0;
			e.condition_id        = row[18] ? static_cast<uint16_t>(strtoul(row[18], nullptr, 10)) : 0;
			e.condition_min_value = row[19] ? static_cast<int16_t>(atoi(row[19])) : 0;
			e.enabled             = row[20] ? static_cast<int16_t>(atoi(row[20])) : 1;
			e.anim                = row[21] ? static_cast<uint16_t>(strtoul(row[21], nullptr, 10)) : 0;
			e.loot_data           = row[22] ? row[22] : "";
			e.entity_variables    = row[23] ? row[23] : "";
			e.buffs               = row[24] ? row[24] : "";
			e.hp                  = row[25] ? strtoll(row[25], nullptr, 10) : 0;
			e.mana                = row[26] ? strtoll(row[26], nullptr, 10) : 0;
			e.endurance           = row[27] ? strtoll(row[27], nullptr, 10) : 0;
			e.created_at          = strtoll(row[28] ? row[28] : "-1", nullptr, 10);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int zone_state_spawns_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				zone_state_spawns_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const ZoneStateSpawns &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.zone_id));
		v.push_back(columns[2] + " = " + std::to_string(e.instance_id));
		v.push_back(columns[3] + " = " + std::to_string(e.is_corpse));
		v.push_back(columns[4] + " = " + std::to_string(e.is_zone));
		v.push_back(columns[5] + " = " + std::to_string(e.decay_in_seconds));
		v.push_back(columns[6] + " = " + std::to_string(e.npc_id));
		v.push_back(columns[7] + " = " + std::to_string(e.spawn2_id));
		v.push_back(columns[8] + " = " + std::to_string(e.spawngroup_id));
		v.push_back(columns[9] + " = " + std::to_string(e.x));
		v.push_back(columns[10] + " = " + std::to_string(e.y));
		v.push_back(columns[11] + " = " + std::to_string(e.z));
		v.push_back(columns[12] + " = " + std::to_string(e.heading));
		v.push_back(columns[13] + " = " + std::to_string(e.respawn_time));
		v.push_back(columns[14] + " = " + std::to_string(e.variance));
		v.push_back(columns[15] + " = " + std::to_string(e.grid));
		v.push_back(columns[16] + " = " + std::to_string(e.current_waypoint));
		v.push_back(columns[17] + " = " + std::to_string(e.path_when_zone_idle));
		v.push_back(columns[18] + " = " + std::to_string(e.condition_id));
		v.push_back(columns[19] + " = " + std::to_string(e.condition_min_value));
		v.push_back(columns[20] + " = " + std::to_string(e.enabled));
		v.push_back(columns[21] + " = " + std::to_string(e.anim));
		v.push_back(columns[22] + " = '" + Strings::Escape(e.loot_data) + "'");
		v.push_back(columns[23] + " = '" + Strings::Escape(e.entity_variables) + "'");
		v.push_back(columns[24] + " = '" + Strings::Escape(e.buffs) + "'");
		v.push_back(columns[25] + " = " + std::to_string(e.hp));
		v.push_back(columns[26] + " = " + std::to_string(e.mana));
		v.push_back(columns[27] + " = " + std::to_string(e.endurance));
		v.push_back(columns[28] + " = FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");

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

	static ZoneStateSpawns InsertOne(
		Database& db,
		ZoneStateSpawns e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.zone_id));
		v.push_back(std::to_string(e.instance_id));
		v.push_back(std::to_string(e.is_corpse));
		v.push_back(std::to_string(e.is_zone));
		v.push_back(std::to_string(e.decay_in_seconds));
		v.push_back(std::to_string(e.npc_id));
		v.push_back(std::to_string(e.spawn2_id));
		v.push_back(std::to_string(e.spawngroup_id));
		v.push_back(std::to_string(e.x));
		v.push_back(std::to_string(e.y));
		v.push_back(std::to_string(e.z));
		v.push_back(std::to_string(e.heading));
		v.push_back(std::to_string(e.respawn_time));
		v.push_back(std::to_string(e.variance));
		v.push_back(std::to_string(e.grid));
		v.push_back(std::to_string(e.current_waypoint));
		v.push_back(std::to_string(e.path_when_zone_idle));
		v.push_back(std::to_string(e.condition_id));
		v.push_back(std::to_string(e.condition_min_value));
		v.push_back(std::to_string(e.enabled));
		v.push_back(std::to_string(e.anim));
		v.push_back("'" + Strings::Escape(e.loot_data) + "'");
		v.push_back("'" + Strings::Escape(e.entity_variables) + "'");
		v.push_back("'" + Strings::Escape(e.buffs) + "'");
		v.push_back(std::to_string(e.hp));
		v.push_back(std::to_string(e.mana));
		v.push_back(std::to_string(e.endurance));
		v.push_back("FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");

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
		const std::vector<ZoneStateSpawns> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.zone_id));
			v.push_back(std::to_string(e.instance_id));
			v.push_back(std::to_string(e.is_corpse));
			v.push_back(std::to_string(e.is_zone));
			v.push_back(std::to_string(e.decay_in_seconds));
			v.push_back(std::to_string(e.npc_id));
			v.push_back(std::to_string(e.spawn2_id));
			v.push_back(std::to_string(e.spawngroup_id));
			v.push_back(std::to_string(e.x));
			v.push_back(std::to_string(e.y));
			v.push_back(std::to_string(e.z));
			v.push_back(std::to_string(e.heading));
			v.push_back(std::to_string(e.respawn_time));
			v.push_back(std::to_string(e.variance));
			v.push_back(std::to_string(e.grid));
			v.push_back(std::to_string(e.current_waypoint));
			v.push_back(std::to_string(e.path_when_zone_idle));
			v.push_back(std::to_string(e.condition_id));
			v.push_back(std::to_string(e.condition_min_value));
			v.push_back(std::to_string(e.enabled));
			v.push_back(std::to_string(e.anim));
			v.push_back("'" + Strings::Escape(e.loot_data) + "'");
			v.push_back("'" + Strings::Escape(e.entity_variables) + "'");
			v.push_back("'" + Strings::Escape(e.buffs) + "'");
			v.push_back(std::to_string(e.hp));
			v.push_back(std::to_string(e.mana));
			v.push_back(std::to_string(e.endurance));
			v.push_back("FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");

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

	static std::vector<ZoneStateSpawns> All(Database& db)
	{
		std::vector<ZoneStateSpawns> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ZoneStateSpawns e{};

			e.id                  = row[0] ? strtoll(row[0], nullptr, 10) : 0;
			e.zone_id             = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.instance_id         = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.is_corpse           = row[3] ? static_cast<int8_t>(atoi(row[3])) : 0;
			e.is_zone             = row[4] ? static_cast<int8_t>(atoi(row[4])) : 0;
			e.decay_in_seconds    = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.npc_id              = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.spawn2_id           = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.spawngroup_id       = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.x                   = row[9] ? strtof(row[9], nullptr) : 0;
			e.y                   = row[10] ? strtof(row[10], nullptr) : 0;
			e.z                   = row[11] ? strtof(row[11], nullptr) : 0;
			e.heading             = row[12] ? strtof(row[12], nullptr) : 0;
			e.respawn_time        = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.variance            = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.grid                = row[15] ? static_cast<uint32_t>(strtoul(row[15], nullptr, 10)) : 0;
			e.current_waypoint    = row[16] ? static_cast<int32_t>(atoi(row[16])) : 0;
			e.path_when_zone_idle = row[17] ? static_cast<int16_t>(atoi(row[17])) : 0;
			e.condition_id        = row[18] ? static_cast<uint16_t>(strtoul(row[18], nullptr, 10)) : 0;
			e.condition_min_value = row[19] ? static_cast<int16_t>(atoi(row[19])) : 0;
			e.enabled             = row[20] ? static_cast<int16_t>(atoi(row[20])) : 1;
			e.anim                = row[21] ? static_cast<uint16_t>(strtoul(row[21], nullptr, 10)) : 0;
			e.loot_data           = row[22] ? row[22] : "";
			e.entity_variables    = row[23] ? row[23] : "";
			e.buffs               = row[24] ? row[24] : "";
			e.hp                  = row[25] ? strtoll(row[25], nullptr, 10) : 0;
			e.mana                = row[26] ? strtoll(row[26], nullptr, 10) : 0;
			e.endurance           = row[27] ? strtoll(row[27], nullptr, 10) : 0;
			e.created_at          = strtoll(row[28] ? row[28] : "-1", nullptr, 10);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<ZoneStateSpawns> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<ZoneStateSpawns> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ZoneStateSpawns e{};

			e.id                  = row[0] ? strtoll(row[0], nullptr, 10) : 0;
			e.zone_id             = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.instance_id         = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.is_corpse           = row[3] ? static_cast<int8_t>(atoi(row[3])) : 0;
			e.is_zone             = row[4] ? static_cast<int8_t>(atoi(row[4])) : 0;
			e.decay_in_seconds    = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.npc_id              = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.spawn2_id           = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.spawngroup_id       = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.x                   = row[9] ? strtof(row[9], nullptr) : 0;
			e.y                   = row[10] ? strtof(row[10], nullptr) : 0;
			e.z                   = row[11] ? strtof(row[11], nullptr) : 0;
			e.heading             = row[12] ? strtof(row[12], nullptr) : 0;
			e.respawn_time        = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.variance            = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.grid                = row[15] ? static_cast<uint32_t>(strtoul(row[15], nullptr, 10)) : 0;
			e.current_waypoint    = row[16] ? static_cast<int32_t>(atoi(row[16])) : 0;
			e.path_when_zone_idle = row[17] ? static_cast<int16_t>(atoi(row[17])) : 0;
			e.condition_id        = row[18] ? static_cast<uint16_t>(strtoul(row[18], nullptr, 10)) : 0;
			e.condition_min_value = row[19] ? static_cast<int16_t>(atoi(row[19])) : 0;
			e.enabled             = row[20] ? static_cast<int16_t>(atoi(row[20])) : 1;
			e.anim                = row[21] ? static_cast<uint16_t>(strtoul(row[21], nullptr, 10)) : 0;
			e.loot_data           = row[22] ? row[22] : "";
			e.entity_variables    = row[23] ? row[23] : "";
			e.buffs               = row[24] ? row[24] : "";
			e.hp                  = row[25] ? strtoll(row[25], nullptr, 10) : 0;
			e.mana                = row[26] ? strtoll(row[26], nullptr, 10) : 0;
			e.endurance           = row[27] ? strtoll(row[27], nullptr, 10) : 0;
			e.created_at          = strtoll(row[28] ? row[28] : "-1", nullptr, 10);

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
		const ZoneStateSpawns &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.zone_id));
		v.push_back(std::to_string(e.instance_id));
		v.push_back(std::to_string(e.is_corpse));
		v.push_back(std::to_string(e.is_zone));
		v.push_back(std::to_string(e.decay_in_seconds));
		v.push_back(std::to_string(e.npc_id));
		v.push_back(std::to_string(e.spawn2_id));
		v.push_back(std::to_string(e.spawngroup_id));
		v.push_back(std::to_string(e.x));
		v.push_back(std::to_string(e.y));
		v.push_back(std::to_string(e.z));
		v.push_back(std::to_string(e.heading));
		v.push_back(std::to_string(e.respawn_time));
		v.push_back(std::to_string(e.variance));
		v.push_back(std::to_string(e.grid));
		v.push_back(std::to_string(e.current_waypoint));
		v.push_back(std::to_string(e.path_when_zone_idle));
		v.push_back(std::to_string(e.condition_id));
		v.push_back(std::to_string(e.condition_min_value));
		v.push_back(std::to_string(e.enabled));
		v.push_back(std::to_string(e.anim));
		v.push_back("'" + Strings::Escape(e.loot_data) + "'");
		v.push_back("'" + Strings::Escape(e.entity_variables) + "'");
		v.push_back("'" + Strings::Escape(e.buffs) + "'");
		v.push_back(std::to_string(e.hp));
		v.push_back(std::to_string(e.mana));
		v.push_back(std::to_string(e.endurance));
		v.push_back("FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");

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
		const std::vector<ZoneStateSpawns> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.zone_id));
			v.push_back(std::to_string(e.instance_id));
			v.push_back(std::to_string(e.is_corpse));
			v.push_back(std::to_string(e.is_zone));
			v.push_back(std::to_string(e.decay_in_seconds));
			v.push_back(std::to_string(e.npc_id));
			v.push_back(std::to_string(e.spawn2_id));
			v.push_back(std::to_string(e.spawngroup_id));
			v.push_back(std::to_string(e.x));
			v.push_back(std::to_string(e.y));
			v.push_back(std::to_string(e.z));
			v.push_back(std::to_string(e.heading));
			v.push_back(std::to_string(e.respawn_time));
			v.push_back(std::to_string(e.variance));
			v.push_back(std::to_string(e.grid));
			v.push_back(std::to_string(e.current_waypoint));
			v.push_back(std::to_string(e.path_when_zone_idle));
			v.push_back(std::to_string(e.condition_id));
			v.push_back(std::to_string(e.condition_min_value));
			v.push_back(std::to_string(e.enabled));
			v.push_back(std::to_string(e.anim));
			v.push_back("'" + Strings::Escape(e.loot_data) + "'");
			v.push_back("'" + Strings::Escape(e.entity_variables) + "'");
			v.push_back("'" + Strings::Escape(e.buffs) + "'");
			v.push_back(std::to_string(e.hp));
			v.push_back(std::to_string(e.mana));
			v.push_back(std::to_string(e.endurance));
			v.push_back("FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");

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
