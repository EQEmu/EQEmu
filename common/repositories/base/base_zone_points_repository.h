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

class BaseZonePointsRepository {
public:
	struct ZonePoints {
		int32_t     id;
		std::string zone;
		int32_t     version;
		uint16_t    number;
		float       y;
		float       x;
		float       z;
		float       heading;
		float       target_y;
		float       target_x;
		float       target_z;
		float       target_heading;
		uint16_t    zoneinst;
		uint32_t    target_zone_id;
		uint32_t    target_instance;
		float       buffer;
		uint32_t    client_version_mask;
		int8_t      min_expansion;
		int8_t      max_expansion;
		std::string content_flags;
		std::string content_flags_disabled;
		int8_t      is_virtual;
		int32_t     height;
		int32_t     width;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"zone",
			"version",
			"number",
			"y",
			"x",
			"z",
			"heading",
			"target_y",
			"target_x",
			"target_z",
			"target_heading",
			"zoneinst",
			"target_zone_id",
			"target_instance",
			"buffer",
			"client_version_mask",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
			"is_virtual",
			"height",
			"width",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"zone",
			"version",
			"number",
			"y",
			"x",
			"z",
			"heading",
			"target_y",
			"target_x",
			"target_z",
			"target_heading",
			"zoneinst",
			"target_zone_id",
			"target_instance",
			"buffer",
			"client_version_mask",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
			"is_virtual",
			"height",
			"width",
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
		return std::string("zone_points");
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

	static ZonePoints NewEntity()
	{
		ZonePoints e{};

		e.id                     = 0;
		e.zone                   = "";
		e.version                = 0;
		e.number                 = 1;
		e.y                      = 0;
		e.x                      = 0;
		e.z                      = 0;
		e.heading                = 0;
		e.target_y               = 0;
		e.target_x               = 0;
		e.target_z               = 0;
		e.target_heading         = 0;
		e.zoneinst               = 0;
		e.target_zone_id         = 0;
		e.target_instance        = 0;
		e.buffer                 = 0;
		e.client_version_mask    = 4294967295;
		e.min_expansion          = -1;
		e.max_expansion          = -1;
		e.content_flags          = "";
		e.content_flags_disabled = "";
		e.is_virtual             = 0;
		e.height                 = 0;
		e.width                  = 0;

		return e;
	}

	static ZonePoints GetZonePoints(
		const std::vector<ZonePoints> &zone_pointss,
		int zone_points_id
	)
	{
		for (auto &zone_points : zone_pointss) {
			if (zone_points.id == zone_points_id) {
				return zone_points;
			}
		}

		return NewEntity();
	}

	static ZonePoints FindOne(
		Database& db,
		int zone_points_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				zone_points_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			ZonePoints e{};

			e.id                     = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.zone                   = row[1] ? row[1] : "";
			e.version                = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.number                 = row[3] ? static_cast<uint16_t>(strtoul(row[3], nullptr, 10)) : 1;
			e.y                      = row[4] ? strtof(row[4], nullptr) : 0;
			e.x                      = row[5] ? strtof(row[5], nullptr) : 0;
			e.z                      = row[6] ? strtof(row[6], nullptr) : 0;
			e.heading                = row[7] ? strtof(row[7], nullptr) : 0;
			e.target_y               = row[8] ? strtof(row[8], nullptr) : 0;
			e.target_x               = row[9] ? strtof(row[9], nullptr) : 0;
			e.target_z               = row[10] ? strtof(row[10], nullptr) : 0;
			e.target_heading         = row[11] ? strtof(row[11], nullptr) : 0;
			e.zoneinst               = row[12] ? static_cast<uint16_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.target_zone_id         = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.target_instance        = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.buffer                 = row[15] ? strtof(row[15], nullptr) : 0;
			e.client_version_mask    = row[16] ? static_cast<uint32_t>(strtoul(row[16], nullptr, 10)) : 4294967295;
			e.min_expansion          = row[17] ? static_cast<int8_t>(atoi(row[17])) : -1;
			e.max_expansion          = row[18] ? static_cast<int8_t>(atoi(row[18])) : -1;
			e.content_flags          = row[19] ? row[19] : "";
			e.content_flags_disabled = row[20] ? row[20] : "";
			e.is_virtual             = row[21] ? static_cast<int8_t>(atoi(row[21])) : 0;
			e.height                 = row[22] ? static_cast<int32_t>(atoi(row[22])) : 0;
			e.width                  = row[23] ? static_cast<int32_t>(atoi(row[23])) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int zone_points_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				zone_points_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const ZonePoints &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.zone) + "'");
		v.push_back(columns[2] + " = " + std::to_string(e.version));
		v.push_back(columns[3] + " = " + std::to_string(e.number));
		v.push_back(columns[4] + " = " + std::to_string(e.y));
		v.push_back(columns[5] + " = " + std::to_string(e.x));
		v.push_back(columns[6] + " = " + std::to_string(e.z));
		v.push_back(columns[7] + " = " + std::to_string(e.heading));
		v.push_back(columns[8] + " = " + std::to_string(e.target_y));
		v.push_back(columns[9] + " = " + std::to_string(e.target_x));
		v.push_back(columns[10] + " = " + std::to_string(e.target_z));
		v.push_back(columns[11] + " = " + std::to_string(e.target_heading));
		v.push_back(columns[12] + " = " + std::to_string(e.zoneinst));
		v.push_back(columns[13] + " = " + std::to_string(e.target_zone_id));
		v.push_back(columns[14] + " = " + std::to_string(e.target_instance));
		v.push_back(columns[15] + " = " + std::to_string(e.buffer));
		v.push_back(columns[16] + " = " + std::to_string(e.client_version_mask));
		v.push_back(columns[17] + " = " + std::to_string(e.min_expansion));
		v.push_back(columns[18] + " = " + std::to_string(e.max_expansion));
		v.push_back(columns[19] + " = '" + Strings::Escape(e.content_flags) + "'");
		v.push_back(columns[20] + " = '" + Strings::Escape(e.content_flags_disabled) + "'");
		v.push_back(columns[21] + " = " + std::to_string(e.is_virtual));
		v.push_back(columns[22] + " = " + std::to_string(e.height));
		v.push_back(columns[23] + " = " + std::to_string(e.width));

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

	static ZonePoints InsertOne(
		Database& db,
		ZonePoints e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.zone) + "'");
		v.push_back(std::to_string(e.version));
		v.push_back(std::to_string(e.number));
		v.push_back(std::to_string(e.y));
		v.push_back(std::to_string(e.x));
		v.push_back(std::to_string(e.z));
		v.push_back(std::to_string(e.heading));
		v.push_back(std::to_string(e.target_y));
		v.push_back(std::to_string(e.target_x));
		v.push_back(std::to_string(e.target_z));
		v.push_back(std::to_string(e.target_heading));
		v.push_back(std::to_string(e.zoneinst));
		v.push_back(std::to_string(e.target_zone_id));
		v.push_back(std::to_string(e.target_instance));
		v.push_back(std::to_string(e.buffer));
		v.push_back(std::to_string(e.client_version_mask));
		v.push_back(std::to_string(e.min_expansion));
		v.push_back(std::to_string(e.max_expansion));
		v.push_back("'" + Strings::Escape(e.content_flags) + "'");
		v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");
		v.push_back(std::to_string(e.is_virtual));
		v.push_back(std::to_string(e.height));
		v.push_back(std::to_string(e.width));

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
		const std::vector<ZonePoints> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.zone) + "'");
			v.push_back(std::to_string(e.version));
			v.push_back(std::to_string(e.number));
			v.push_back(std::to_string(e.y));
			v.push_back(std::to_string(e.x));
			v.push_back(std::to_string(e.z));
			v.push_back(std::to_string(e.heading));
			v.push_back(std::to_string(e.target_y));
			v.push_back(std::to_string(e.target_x));
			v.push_back(std::to_string(e.target_z));
			v.push_back(std::to_string(e.target_heading));
			v.push_back(std::to_string(e.zoneinst));
			v.push_back(std::to_string(e.target_zone_id));
			v.push_back(std::to_string(e.target_instance));
			v.push_back(std::to_string(e.buffer));
			v.push_back(std::to_string(e.client_version_mask));
			v.push_back(std::to_string(e.min_expansion));
			v.push_back(std::to_string(e.max_expansion));
			v.push_back("'" + Strings::Escape(e.content_flags) + "'");
			v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");
			v.push_back(std::to_string(e.is_virtual));
			v.push_back(std::to_string(e.height));
			v.push_back(std::to_string(e.width));

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

	static std::vector<ZonePoints> All(Database& db)
	{
		std::vector<ZonePoints> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ZonePoints e{};

			e.id                     = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.zone                   = row[1] ? row[1] : "";
			e.version                = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.number                 = row[3] ? static_cast<uint16_t>(strtoul(row[3], nullptr, 10)) : 1;
			e.y                      = row[4] ? strtof(row[4], nullptr) : 0;
			e.x                      = row[5] ? strtof(row[5], nullptr) : 0;
			e.z                      = row[6] ? strtof(row[6], nullptr) : 0;
			e.heading                = row[7] ? strtof(row[7], nullptr) : 0;
			e.target_y               = row[8] ? strtof(row[8], nullptr) : 0;
			e.target_x               = row[9] ? strtof(row[9], nullptr) : 0;
			e.target_z               = row[10] ? strtof(row[10], nullptr) : 0;
			e.target_heading         = row[11] ? strtof(row[11], nullptr) : 0;
			e.zoneinst               = row[12] ? static_cast<uint16_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.target_zone_id         = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.target_instance        = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.buffer                 = row[15] ? strtof(row[15], nullptr) : 0;
			e.client_version_mask    = row[16] ? static_cast<uint32_t>(strtoul(row[16], nullptr, 10)) : 4294967295;
			e.min_expansion          = row[17] ? static_cast<int8_t>(atoi(row[17])) : -1;
			e.max_expansion          = row[18] ? static_cast<int8_t>(atoi(row[18])) : -1;
			e.content_flags          = row[19] ? row[19] : "";
			e.content_flags_disabled = row[20] ? row[20] : "";
			e.is_virtual             = row[21] ? static_cast<int8_t>(atoi(row[21])) : 0;
			e.height                 = row[22] ? static_cast<int32_t>(atoi(row[22])) : 0;
			e.width                  = row[23] ? static_cast<int32_t>(atoi(row[23])) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<ZonePoints> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<ZonePoints> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ZonePoints e{};

			e.id                     = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.zone                   = row[1] ? row[1] : "";
			e.version                = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.number                 = row[3] ? static_cast<uint16_t>(strtoul(row[3], nullptr, 10)) : 1;
			e.y                      = row[4] ? strtof(row[4], nullptr) : 0;
			e.x                      = row[5] ? strtof(row[5], nullptr) : 0;
			e.z                      = row[6] ? strtof(row[6], nullptr) : 0;
			e.heading                = row[7] ? strtof(row[7], nullptr) : 0;
			e.target_y               = row[8] ? strtof(row[8], nullptr) : 0;
			e.target_x               = row[9] ? strtof(row[9], nullptr) : 0;
			e.target_z               = row[10] ? strtof(row[10], nullptr) : 0;
			e.target_heading         = row[11] ? strtof(row[11], nullptr) : 0;
			e.zoneinst               = row[12] ? static_cast<uint16_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.target_zone_id         = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.target_instance        = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.buffer                 = row[15] ? strtof(row[15], nullptr) : 0;
			e.client_version_mask    = row[16] ? static_cast<uint32_t>(strtoul(row[16], nullptr, 10)) : 4294967295;
			e.min_expansion          = row[17] ? static_cast<int8_t>(atoi(row[17])) : -1;
			e.max_expansion          = row[18] ? static_cast<int8_t>(atoi(row[18])) : -1;
			e.content_flags          = row[19] ? row[19] : "";
			e.content_flags_disabled = row[20] ? row[20] : "";
			e.is_virtual             = row[21] ? static_cast<int8_t>(atoi(row[21])) : 0;
			e.height                 = row[22] ? static_cast<int32_t>(atoi(row[22])) : 0;
			e.width                  = row[23] ? static_cast<int32_t>(atoi(row[23])) : 0;

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
		const ZonePoints &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.zone) + "'");
		v.push_back(std::to_string(e.version));
		v.push_back(std::to_string(e.number));
		v.push_back(std::to_string(e.y));
		v.push_back(std::to_string(e.x));
		v.push_back(std::to_string(e.z));
		v.push_back(std::to_string(e.heading));
		v.push_back(std::to_string(e.target_y));
		v.push_back(std::to_string(e.target_x));
		v.push_back(std::to_string(e.target_z));
		v.push_back(std::to_string(e.target_heading));
		v.push_back(std::to_string(e.zoneinst));
		v.push_back(std::to_string(e.target_zone_id));
		v.push_back(std::to_string(e.target_instance));
		v.push_back(std::to_string(e.buffer));
		v.push_back(std::to_string(e.client_version_mask));
		v.push_back(std::to_string(e.min_expansion));
		v.push_back(std::to_string(e.max_expansion));
		v.push_back("'" + Strings::Escape(e.content_flags) + "'");
		v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");
		v.push_back(std::to_string(e.is_virtual));
		v.push_back(std::to_string(e.height));
		v.push_back(std::to_string(e.width));

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
		const std::vector<ZonePoints> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.zone) + "'");
			v.push_back(std::to_string(e.version));
			v.push_back(std::to_string(e.number));
			v.push_back(std::to_string(e.y));
			v.push_back(std::to_string(e.x));
			v.push_back(std::to_string(e.z));
			v.push_back(std::to_string(e.heading));
			v.push_back(std::to_string(e.target_y));
			v.push_back(std::to_string(e.target_x));
			v.push_back(std::to_string(e.target_z));
			v.push_back(std::to_string(e.target_heading));
			v.push_back(std::to_string(e.zoneinst));
			v.push_back(std::to_string(e.target_zone_id));
			v.push_back(std::to_string(e.target_instance));
			v.push_back(std::to_string(e.buffer));
			v.push_back(std::to_string(e.client_version_mask));
			v.push_back(std::to_string(e.min_expansion));
			v.push_back(std::to_string(e.max_expansion));
			v.push_back("'" + Strings::Escape(e.content_flags) + "'");
			v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");
			v.push_back(std::to_string(e.is_virtual));
			v.push_back(std::to_string(e.height));
			v.push_back(std::to_string(e.width));

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
