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

class BaseDynamicZonesRepository {
public:
	struct DynamicZones {
		uint32_t    id;
		int32_t     instance_id;
		uint8_t     type;
		std::string uuid;
		std::string name;
		uint32_t    leader_id;
		uint32_t    min_players;
		uint32_t    max_players;
		int32_t     dz_switch_id;
		uint32_t    compass_zone_id;
		float       compass_x;
		float       compass_y;
		float       compass_z;
		uint32_t    safe_return_zone_id;
		float       safe_return_x;
		float       safe_return_y;
		float       safe_return_z;
		float       safe_return_heading;
		float       zone_in_x;
		float       zone_in_y;
		float       zone_in_z;
		float       zone_in_heading;
		uint8_t     has_zone_in;
		int8_t      is_locked;
		int8_t      add_replay;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"instance_id",
			"type",
			"uuid",
			"name",
			"leader_id",
			"min_players",
			"max_players",
			"dz_switch_id",
			"compass_zone_id",
			"compass_x",
			"compass_y",
			"compass_z",
			"safe_return_zone_id",
			"safe_return_x",
			"safe_return_y",
			"safe_return_z",
			"safe_return_heading",
			"zone_in_x",
			"zone_in_y",
			"zone_in_z",
			"zone_in_heading",
			"has_zone_in",
			"is_locked",
			"add_replay",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"instance_id",
			"type",
			"uuid",
			"name",
			"leader_id",
			"min_players",
			"max_players",
			"dz_switch_id",
			"compass_zone_id",
			"compass_x",
			"compass_y",
			"compass_z",
			"safe_return_zone_id",
			"safe_return_x",
			"safe_return_y",
			"safe_return_z",
			"safe_return_heading",
			"zone_in_x",
			"zone_in_y",
			"zone_in_z",
			"zone_in_heading",
			"has_zone_in",
			"is_locked",
			"add_replay",
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
		return std::string("dynamic_zones");
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

	static DynamicZones NewEntity()
	{
		DynamicZones e{};

		e.id                  = 0;
		e.instance_id         = 0;
		e.type                = 0;
		e.uuid                = "";
		e.name                = "";
		e.leader_id           = 0;
		e.min_players         = 0;
		e.max_players         = 0;
		e.dz_switch_id        = 0;
		e.compass_zone_id     = 0;
		e.compass_x           = 0;
		e.compass_y           = 0;
		e.compass_z           = 0;
		e.safe_return_zone_id = 0;
		e.safe_return_x       = 0;
		e.safe_return_y       = 0;
		e.safe_return_z       = 0;
		e.safe_return_heading = 0;
		e.zone_in_x           = 0;
		e.zone_in_y           = 0;
		e.zone_in_z           = 0;
		e.zone_in_heading     = 0;
		e.has_zone_in         = 0;
		e.is_locked           = 0;
		e.add_replay          = 1;

		return e;
	}

	static DynamicZones GetDynamicZones(
		const std::vector<DynamicZones> &dynamic_zoness,
		int dynamic_zones_id
	)
	{
		for (auto &dynamic_zones : dynamic_zoness) {
			if (dynamic_zones.id == dynamic_zones_id) {
				return dynamic_zones;
			}
		}

		return NewEntity();
	}

	static DynamicZones FindOne(
		Database& db,
		int dynamic_zones_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				dynamic_zones_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			DynamicZones e{};

			e.id                  = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.instance_id         = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.type                = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.uuid                = row[3] ? row[3] : "";
			e.name                = row[4] ? row[4] : "";
			e.leader_id           = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.min_players         = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.max_players         = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.dz_switch_id        = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.compass_zone_id     = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.compass_x           = row[10] ? strtof(row[10], nullptr) : 0;
			e.compass_y           = row[11] ? strtof(row[11], nullptr) : 0;
			e.compass_z           = row[12] ? strtof(row[12], nullptr) : 0;
			e.safe_return_zone_id = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.safe_return_x       = row[14] ? strtof(row[14], nullptr) : 0;
			e.safe_return_y       = row[15] ? strtof(row[15], nullptr) : 0;
			e.safe_return_z       = row[16] ? strtof(row[16], nullptr) : 0;
			e.safe_return_heading = row[17] ? strtof(row[17], nullptr) : 0;
			e.zone_in_x           = row[18] ? strtof(row[18], nullptr) : 0;
			e.zone_in_y           = row[19] ? strtof(row[19], nullptr) : 0;
			e.zone_in_z           = row[20] ? strtof(row[20], nullptr) : 0;
			e.zone_in_heading     = row[21] ? strtof(row[21], nullptr) : 0;
			e.has_zone_in         = row[22] ? static_cast<uint8_t>(strtoul(row[22], nullptr, 10)) : 0;
			e.is_locked           = row[23] ? static_cast<int8_t>(atoi(row[23])) : 0;
			e.add_replay          = row[24] ? static_cast<int8_t>(atoi(row[24])) : 1;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int dynamic_zones_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				dynamic_zones_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const DynamicZones &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.instance_id));
		v.push_back(columns[2] + " = " + std::to_string(e.type));
		v.push_back(columns[3] + " = '" + Strings::Escape(e.uuid) + "'");
		v.push_back(columns[4] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[5] + " = " + std::to_string(e.leader_id));
		v.push_back(columns[6] + " = " + std::to_string(e.min_players));
		v.push_back(columns[7] + " = " + std::to_string(e.max_players));
		v.push_back(columns[8] + " = " + std::to_string(e.dz_switch_id));
		v.push_back(columns[9] + " = " + std::to_string(e.compass_zone_id));
		v.push_back(columns[10] + " = " + std::to_string(e.compass_x));
		v.push_back(columns[11] + " = " + std::to_string(e.compass_y));
		v.push_back(columns[12] + " = " + std::to_string(e.compass_z));
		v.push_back(columns[13] + " = " + std::to_string(e.safe_return_zone_id));
		v.push_back(columns[14] + " = " + std::to_string(e.safe_return_x));
		v.push_back(columns[15] + " = " + std::to_string(e.safe_return_y));
		v.push_back(columns[16] + " = " + std::to_string(e.safe_return_z));
		v.push_back(columns[17] + " = " + std::to_string(e.safe_return_heading));
		v.push_back(columns[18] + " = " + std::to_string(e.zone_in_x));
		v.push_back(columns[19] + " = " + std::to_string(e.zone_in_y));
		v.push_back(columns[20] + " = " + std::to_string(e.zone_in_z));
		v.push_back(columns[21] + " = " + std::to_string(e.zone_in_heading));
		v.push_back(columns[22] + " = " + std::to_string(e.has_zone_in));
		v.push_back(columns[23] + " = " + std::to_string(e.is_locked));
		v.push_back(columns[24] + " = " + std::to_string(e.add_replay));

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

	static DynamicZones InsertOne(
		Database& db,
		DynamicZones e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.instance_id));
		v.push_back(std::to_string(e.type));
		v.push_back("'" + Strings::Escape(e.uuid) + "'");
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.leader_id));
		v.push_back(std::to_string(e.min_players));
		v.push_back(std::to_string(e.max_players));
		v.push_back(std::to_string(e.dz_switch_id));
		v.push_back(std::to_string(e.compass_zone_id));
		v.push_back(std::to_string(e.compass_x));
		v.push_back(std::to_string(e.compass_y));
		v.push_back(std::to_string(e.compass_z));
		v.push_back(std::to_string(e.safe_return_zone_id));
		v.push_back(std::to_string(e.safe_return_x));
		v.push_back(std::to_string(e.safe_return_y));
		v.push_back(std::to_string(e.safe_return_z));
		v.push_back(std::to_string(e.safe_return_heading));
		v.push_back(std::to_string(e.zone_in_x));
		v.push_back(std::to_string(e.zone_in_y));
		v.push_back(std::to_string(e.zone_in_z));
		v.push_back(std::to_string(e.zone_in_heading));
		v.push_back(std::to_string(e.has_zone_in));
		v.push_back(std::to_string(e.is_locked));
		v.push_back(std::to_string(e.add_replay));

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
		const std::vector<DynamicZones> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.instance_id));
			v.push_back(std::to_string(e.type));
			v.push_back("'" + Strings::Escape(e.uuid) + "'");
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.leader_id));
			v.push_back(std::to_string(e.min_players));
			v.push_back(std::to_string(e.max_players));
			v.push_back(std::to_string(e.dz_switch_id));
			v.push_back(std::to_string(e.compass_zone_id));
			v.push_back(std::to_string(e.compass_x));
			v.push_back(std::to_string(e.compass_y));
			v.push_back(std::to_string(e.compass_z));
			v.push_back(std::to_string(e.safe_return_zone_id));
			v.push_back(std::to_string(e.safe_return_x));
			v.push_back(std::to_string(e.safe_return_y));
			v.push_back(std::to_string(e.safe_return_z));
			v.push_back(std::to_string(e.safe_return_heading));
			v.push_back(std::to_string(e.zone_in_x));
			v.push_back(std::to_string(e.zone_in_y));
			v.push_back(std::to_string(e.zone_in_z));
			v.push_back(std::to_string(e.zone_in_heading));
			v.push_back(std::to_string(e.has_zone_in));
			v.push_back(std::to_string(e.is_locked));
			v.push_back(std::to_string(e.add_replay));

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

	static std::vector<DynamicZones> All(Database& db)
	{
		std::vector<DynamicZones> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			DynamicZones e{};

			e.id                  = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.instance_id         = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.type                = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.uuid                = row[3] ? row[3] : "";
			e.name                = row[4] ? row[4] : "";
			e.leader_id           = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.min_players         = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.max_players         = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.dz_switch_id        = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.compass_zone_id     = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.compass_x           = row[10] ? strtof(row[10], nullptr) : 0;
			e.compass_y           = row[11] ? strtof(row[11], nullptr) : 0;
			e.compass_z           = row[12] ? strtof(row[12], nullptr) : 0;
			e.safe_return_zone_id = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.safe_return_x       = row[14] ? strtof(row[14], nullptr) : 0;
			e.safe_return_y       = row[15] ? strtof(row[15], nullptr) : 0;
			e.safe_return_z       = row[16] ? strtof(row[16], nullptr) : 0;
			e.safe_return_heading = row[17] ? strtof(row[17], nullptr) : 0;
			e.zone_in_x           = row[18] ? strtof(row[18], nullptr) : 0;
			e.zone_in_y           = row[19] ? strtof(row[19], nullptr) : 0;
			e.zone_in_z           = row[20] ? strtof(row[20], nullptr) : 0;
			e.zone_in_heading     = row[21] ? strtof(row[21], nullptr) : 0;
			e.has_zone_in         = row[22] ? static_cast<uint8_t>(strtoul(row[22], nullptr, 10)) : 0;
			e.is_locked           = row[23] ? static_cast<int8_t>(atoi(row[23])) : 0;
			e.add_replay          = row[24] ? static_cast<int8_t>(atoi(row[24])) : 1;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<DynamicZones> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<DynamicZones> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			DynamicZones e{};

			e.id                  = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.instance_id         = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.type                = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.uuid                = row[3] ? row[3] : "";
			e.name                = row[4] ? row[4] : "";
			e.leader_id           = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.min_players         = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.max_players         = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.dz_switch_id        = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.compass_zone_id     = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.compass_x           = row[10] ? strtof(row[10], nullptr) : 0;
			e.compass_y           = row[11] ? strtof(row[11], nullptr) : 0;
			e.compass_z           = row[12] ? strtof(row[12], nullptr) : 0;
			e.safe_return_zone_id = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.safe_return_x       = row[14] ? strtof(row[14], nullptr) : 0;
			e.safe_return_y       = row[15] ? strtof(row[15], nullptr) : 0;
			e.safe_return_z       = row[16] ? strtof(row[16], nullptr) : 0;
			e.safe_return_heading = row[17] ? strtof(row[17], nullptr) : 0;
			e.zone_in_x           = row[18] ? strtof(row[18], nullptr) : 0;
			e.zone_in_y           = row[19] ? strtof(row[19], nullptr) : 0;
			e.zone_in_z           = row[20] ? strtof(row[20], nullptr) : 0;
			e.zone_in_heading     = row[21] ? strtof(row[21], nullptr) : 0;
			e.has_zone_in         = row[22] ? static_cast<uint8_t>(strtoul(row[22], nullptr, 10)) : 0;
			e.is_locked           = row[23] ? static_cast<int8_t>(atoi(row[23])) : 0;
			e.add_replay          = row[24] ? static_cast<int8_t>(atoi(row[24])) : 1;

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
		const DynamicZones &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.instance_id));
		v.push_back(std::to_string(e.type));
		v.push_back("'" + Strings::Escape(e.uuid) + "'");
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.leader_id));
		v.push_back(std::to_string(e.min_players));
		v.push_back(std::to_string(e.max_players));
		v.push_back(std::to_string(e.dz_switch_id));
		v.push_back(std::to_string(e.compass_zone_id));
		v.push_back(std::to_string(e.compass_x));
		v.push_back(std::to_string(e.compass_y));
		v.push_back(std::to_string(e.compass_z));
		v.push_back(std::to_string(e.safe_return_zone_id));
		v.push_back(std::to_string(e.safe_return_x));
		v.push_back(std::to_string(e.safe_return_y));
		v.push_back(std::to_string(e.safe_return_z));
		v.push_back(std::to_string(e.safe_return_heading));
		v.push_back(std::to_string(e.zone_in_x));
		v.push_back(std::to_string(e.zone_in_y));
		v.push_back(std::to_string(e.zone_in_z));
		v.push_back(std::to_string(e.zone_in_heading));
		v.push_back(std::to_string(e.has_zone_in));
		v.push_back(std::to_string(e.is_locked));
		v.push_back(std::to_string(e.add_replay));

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
		const std::vector<DynamicZones> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.instance_id));
			v.push_back(std::to_string(e.type));
			v.push_back("'" + Strings::Escape(e.uuid) + "'");
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.leader_id));
			v.push_back(std::to_string(e.min_players));
			v.push_back(std::to_string(e.max_players));
			v.push_back(std::to_string(e.dz_switch_id));
			v.push_back(std::to_string(e.compass_zone_id));
			v.push_back(std::to_string(e.compass_x));
			v.push_back(std::to_string(e.compass_y));
			v.push_back(std::to_string(e.compass_z));
			v.push_back(std::to_string(e.safe_return_zone_id));
			v.push_back(std::to_string(e.safe_return_x));
			v.push_back(std::to_string(e.safe_return_y));
			v.push_back(std::to_string(e.safe_return_z));
			v.push_back(std::to_string(e.safe_return_heading));
			v.push_back(std::to_string(e.zone_in_x));
			v.push_back(std::to_string(e.zone_in_y));
			v.push_back(std::to_string(e.zone_in_z));
			v.push_back(std::to_string(e.zone_in_heading));
			v.push_back(std::to_string(e.has_zone_in));
			v.push_back(std::to_string(e.is_locked));
			v.push_back(std::to_string(e.add_replay));

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
