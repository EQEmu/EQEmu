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

class BaseDynamicZoneTemplatesRepository {
public:
	struct DynamicZoneTemplates {
		uint32_t    id;
		int32_t     zone_id;
		int32_t     zone_version;
		std::string name;
		int32_t     min_players;
		int32_t     max_players;
		int32_t     duration_seconds;
		int32_t     dz_switch_id;
		int32_t     compass_zone_id;
		float       compass_x;
		float       compass_y;
		float       compass_z;
		int32_t     return_zone_id;
		float       return_x;
		float       return_y;
		float       return_z;
		float       return_h;
		int8_t      override_zone_in;
		float       zone_in_x;
		float       zone_in_y;
		float       zone_in_z;
		float       zone_in_h;
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
			"zone_version",
			"name",
			"min_players",
			"max_players",
			"duration_seconds",
			"dz_switch_id",
			"compass_zone_id",
			"compass_x",
			"compass_y",
			"compass_z",
			"return_zone_id",
			"return_x",
			"return_y",
			"return_z",
			"return_h",
			"override_zone_in",
			"zone_in_x",
			"zone_in_y",
			"zone_in_z",
			"zone_in_h",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"zone_id",
			"zone_version",
			"name",
			"min_players",
			"max_players",
			"duration_seconds",
			"dz_switch_id",
			"compass_zone_id",
			"compass_x",
			"compass_y",
			"compass_z",
			"return_zone_id",
			"return_x",
			"return_y",
			"return_z",
			"return_h",
			"override_zone_in",
			"zone_in_x",
			"zone_in_y",
			"zone_in_z",
			"zone_in_h",
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
		return std::string("dynamic_zone_templates");
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

	static DynamicZoneTemplates NewEntity()
	{
		DynamicZoneTemplates e{};

		e.id               = 0;
		e.zone_id          = 0;
		e.zone_version     = 0;
		e.name             = "";
		e.min_players      = 0;
		e.max_players      = 0;
		e.duration_seconds = 0;
		e.dz_switch_id     = 0;
		e.compass_zone_id  = 0;
		e.compass_x        = 0;
		e.compass_y        = 0;
		e.compass_z        = 0;
		e.return_zone_id   = 0;
		e.return_x         = 0;
		e.return_y         = 0;
		e.return_z         = 0;
		e.return_h         = 0;
		e.override_zone_in = 0;
		e.zone_in_x        = 0;
		e.zone_in_y        = 0;
		e.zone_in_z        = 0;
		e.zone_in_h        = 0;

		return e;
	}

	static DynamicZoneTemplates GetDynamicZoneTemplates(
		const std::vector<DynamicZoneTemplates> &dynamic_zone_templatess,
		int dynamic_zone_templates_id
	)
	{
		for (auto &dynamic_zone_templates : dynamic_zone_templatess) {
			if (dynamic_zone_templates.id == dynamic_zone_templates_id) {
				return dynamic_zone_templates;
			}
		}

		return NewEntity();
	}

	static DynamicZoneTemplates FindOne(
		Database& db,
		int dynamic_zone_templates_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				dynamic_zone_templates_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			DynamicZoneTemplates e{};

			e.id               = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.zone_id          = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.zone_version     = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.name             = row[3] ? row[3] : "";
			e.min_players      = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.max_players      = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.duration_seconds = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.dz_switch_id     = row[7] ? static_cast<int32_t>(atoi(row[7])) : 0;
			e.compass_zone_id  = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.compass_x        = row[9] ? strtof(row[9], nullptr) : 0;
			e.compass_y        = row[10] ? strtof(row[10], nullptr) : 0;
			e.compass_z        = row[11] ? strtof(row[11], nullptr) : 0;
			e.return_zone_id   = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.return_x         = row[13] ? strtof(row[13], nullptr) : 0;
			e.return_y         = row[14] ? strtof(row[14], nullptr) : 0;
			e.return_z         = row[15] ? strtof(row[15], nullptr) : 0;
			e.return_h         = row[16] ? strtof(row[16], nullptr) : 0;
			e.override_zone_in = row[17] ? static_cast<int8_t>(atoi(row[17])) : 0;
			e.zone_in_x        = row[18] ? strtof(row[18], nullptr) : 0;
			e.zone_in_y        = row[19] ? strtof(row[19], nullptr) : 0;
			e.zone_in_z        = row[20] ? strtof(row[20], nullptr) : 0;
			e.zone_in_h        = row[21] ? strtof(row[21], nullptr) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int dynamic_zone_templates_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				dynamic_zone_templates_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const DynamicZoneTemplates &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.zone_id));
		v.push_back(columns[2] + " = " + std::to_string(e.zone_version));
		v.push_back(columns[3] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[4] + " = " + std::to_string(e.min_players));
		v.push_back(columns[5] + " = " + std::to_string(e.max_players));
		v.push_back(columns[6] + " = " + std::to_string(e.duration_seconds));
		v.push_back(columns[7] + " = " + std::to_string(e.dz_switch_id));
		v.push_back(columns[8] + " = " + std::to_string(e.compass_zone_id));
		v.push_back(columns[9] + " = " + std::to_string(e.compass_x));
		v.push_back(columns[10] + " = " + std::to_string(e.compass_y));
		v.push_back(columns[11] + " = " + std::to_string(e.compass_z));
		v.push_back(columns[12] + " = " + std::to_string(e.return_zone_id));
		v.push_back(columns[13] + " = " + std::to_string(e.return_x));
		v.push_back(columns[14] + " = " + std::to_string(e.return_y));
		v.push_back(columns[15] + " = " + std::to_string(e.return_z));
		v.push_back(columns[16] + " = " + std::to_string(e.return_h));
		v.push_back(columns[17] + " = " + std::to_string(e.override_zone_in));
		v.push_back(columns[18] + " = " + std::to_string(e.zone_in_x));
		v.push_back(columns[19] + " = " + std::to_string(e.zone_in_y));
		v.push_back(columns[20] + " = " + std::to_string(e.zone_in_z));
		v.push_back(columns[21] + " = " + std::to_string(e.zone_in_h));

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

	static DynamicZoneTemplates InsertOne(
		Database& db,
		DynamicZoneTemplates e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.zone_id));
		v.push_back(std::to_string(e.zone_version));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.min_players));
		v.push_back(std::to_string(e.max_players));
		v.push_back(std::to_string(e.duration_seconds));
		v.push_back(std::to_string(e.dz_switch_id));
		v.push_back(std::to_string(e.compass_zone_id));
		v.push_back(std::to_string(e.compass_x));
		v.push_back(std::to_string(e.compass_y));
		v.push_back(std::to_string(e.compass_z));
		v.push_back(std::to_string(e.return_zone_id));
		v.push_back(std::to_string(e.return_x));
		v.push_back(std::to_string(e.return_y));
		v.push_back(std::to_string(e.return_z));
		v.push_back(std::to_string(e.return_h));
		v.push_back(std::to_string(e.override_zone_in));
		v.push_back(std::to_string(e.zone_in_x));
		v.push_back(std::to_string(e.zone_in_y));
		v.push_back(std::to_string(e.zone_in_z));
		v.push_back(std::to_string(e.zone_in_h));

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
		const std::vector<DynamicZoneTemplates> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.zone_id));
			v.push_back(std::to_string(e.zone_version));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.min_players));
			v.push_back(std::to_string(e.max_players));
			v.push_back(std::to_string(e.duration_seconds));
			v.push_back(std::to_string(e.dz_switch_id));
			v.push_back(std::to_string(e.compass_zone_id));
			v.push_back(std::to_string(e.compass_x));
			v.push_back(std::to_string(e.compass_y));
			v.push_back(std::to_string(e.compass_z));
			v.push_back(std::to_string(e.return_zone_id));
			v.push_back(std::to_string(e.return_x));
			v.push_back(std::to_string(e.return_y));
			v.push_back(std::to_string(e.return_z));
			v.push_back(std::to_string(e.return_h));
			v.push_back(std::to_string(e.override_zone_in));
			v.push_back(std::to_string(e.zone_in_x));
			v.push_back(std::to_string(e.zone_in_y));
			v.push_back(std::to_string(e.zone_in_z));
			v.push_back(std::to_string(e.zone_in_h));

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

	static std::vector<DynamicZoneTemplates> All(Database& db)
	{
		std::vector<DynamicZoneTemplates> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			DynamicZoneTemplates e{};

			e.id               = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.zone_id          = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.zone_version     = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.name             = row[3] ? row[3] : "";
			e.min_players      = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.max_players      = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.duration_seconds = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.dz_switch_id     = row[7] ? static_cast<int32_t>(atoi(row[7])) : 0;
			e.compass_zone_id  = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.compass_x        = row[9] ? strtof(row[9], nullptr) : 0;
			e.compass_y        = row[10] ? strtof(row[10], nullptr) : 0;
			e.compass_z        = row[11] ? strtof(row[11], nullptr) : 0;
			e.return_zone_id   = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.return_x         = row[13] ? strtof(row[13], nullptr) : 0;
			e.return_y         = row[14] ? strtof(row[14], nullptr) : 0;
			e.return_z         = row[15] ? strtof(row[15], nullptr) : 0;
			e.return_h         = row[16] ? strtof(row[16], nullptr) : 0;
			e.override_zone_in = row[17] ? static_cast<int8_t>(atoi(row[17])) : 0;
			e.zone_in_x        = row[18] ? strtof(row[18], nullptr) : 0;
			e.zone_in_y        = row[19] ? strtof(row[19], nullptr) : 0;
			e.zone_in_z        = row[20] ? strtof(row[20], nullptr) : 0;
			e.zone_in_h        = row[21] ? strtof(row[21], nullptr) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<DynamicZoneTemplates> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<DynamicZoneTemplates> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			DynamicZoneTemplates e{};

			e.id               = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.zone_id          = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.zone_version     = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.name             = row[3] ? row[3] : "";
			e.min_players      = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.max_players      = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.duration_seconds = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.dz_switch_id     = row[7] ? static_cast<int32_t>(atoi(row[7])) : 0;
			e.compass_zone_id  = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.compass_x        = row[9] ? strtof(row[9], nullptr) : 0;
			e.compass_y        = row[10] ? strtof(row[10], nullptr) : 0;
			e.compass_z        = row[11] ? strtof(row[11], nullptr) : 0;
			e.return_zone_id   = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.return_x         = row[13] ? strtof(row[13], nullptr) : 0;
			e.return_y         = row[14] ? strtof(row[14], nullptr) : 0;
			e.return_z         = row[15] ? strtof(row[15], nullptr) : 0;
			e.return_h         = row[16] ? strtof(row[16], nullptr) : 0;
			e.override_zone_in = row[17] ? static_cast<int8_t>(atoi(row[17])) : 0;
			e.zone_in_x        = row[18] ? strtof(row[18], nullptr) : 0;
			e.zone_in_y        = row[19] ? strtof(row[19], nullptr) : 0;
			e.zone_in_z        = row[20] ? strtof(row[20], nullptr) : 0;
			e.zone_in_h        = row[21] ? strtof(row[21], nullptr) : 0;

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
		const DynamicZoneTemplates &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.zone_id));
		v.push_back(std::to_string(e.zone_version));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.min_players));
		v.push_back(std::to_string(e.max_players));
		v.push_back(std::to_string(e.duration_seconds));
		v.push_back(std::to_string(e.dz_switch_id));
		v.push_back(std::to_string(e.compass_zone_id));
		v.push_back(std::to_string(e.compass_x));
		v.push_back(std::to_string(e.compass_y));
		v.push_back(std::to_string(e.compass_z));
		v.push_back(std::to_string(e.return_zone_id));
		v.push_back(std::to_string(e.return_x));
		v.push_back(std::to_string(e.return_y));
		v.push_back(std::to_string(e.return_z));
		v.push_back(std::to_string(e.return_h));
		v.push_back(std::to_string(e.override_zone_in));
		v.push_back(std::to_string(e.zone_in_x));
		v.push_back(std::to_string(e.zone_in_y));
		v.push_back(std::to_string(e.zone_in_z));
		v.push_back(std::to_string(e.zone_in_h));

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
		const std::vector<DynamicZoneTemplates> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.zone_id));
			v.push_back(std::to_string(e.zone_version));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.min_players));
			v.push_back(std::to_string(e.max_players));
			v.push_back(std::to_string(e.duration_seconds));
			v.push_back(std::to_string(e.dz_switch_id));
			v.push_back(std::to_string(e.compass_zone_id));
			v.push_back(std::to_string(e.compass_x));
			v.push_back(std::to_string(e.compass_y));
			v.push_back(std::to_string(e.compass_z));
			v.push_back(std::to_string(e.return_zone_id));
			v.push_back(std::to_string(e.return_x));
			v.push_back(std::to_string(e.return_y));
			v.push_back(std::to_string(e.return_z));
			v.push_back(std::to_string(e.return_h));
			v.push_back(std::to_string(e.override_zone_in));
			v.push_back(std::to_string(e.zone_in_x));
			v.push_back(std::to_string(e.zone_in_y));
			v.push_back(std::to_string(e.zone_in_z));
			v.push_back(std::to_string(e.zone_in_h));

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
