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

class BaseGroundSpawnsRepository {
public:
	struct GroundSpawns {
		uint32_t    id;
		uint32_t    zoneid;
		int16_t     version;
		float       max_x;
		float       max_y;
		float       max_z;
		float       min_x;
		float       min_y;
		float       heading;
		std::string name;
		uint32_t    item;
		uint32_t    max_allowed;
		std::string comment;
		uint32_t    respawn_timer;
		uint8_t     fix_z;
		int8_t      min_expansion;
		int8_t      max_expansion;
		std::string content_flags;
		std::string content_flags_disabled;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"zoneid",
			"version",
			"max_x",
			"max_y",
			"max_z",
			"min_x",
			"min_y",
			"heading",
			"name",
			"item",
			"max_allowed",
			"comment",
			"respawn_timer",
			"fix_z",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"zoneid",
			"version",
			"max_x",
			"max_y",
			"max_z",
			"min_x",
			"min_y",
			"heading",
			"name",
			"item",
			"max_allowed",
			"comment",
			"respawn_timer",
			"fix_z",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
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
		return std::string("ground_spawns");
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

	static GroundSpawns NewEntity()
	{
		GroundSpawns e{};

		e.id                     = 0;
		e.zoneid                 = 0;
		e.version                = 0;
		e.max_x                  = 2000;
		e.max_y                  = 2000;
		e.max_z                  = 10000;
		e.min_x                  = -2000;
		e.min_y                  = -2000;
		e.heading                = 0;
		e.name                   = "";
		e.item                   = 0;
		e.max_allowed            = 1;
		e.comment                = "";
		e.respawn_timer          = 300;
		e.fix_z                  = 1;
		e.min_expansion          = -1;
		e.max_expansion          = -1;
		e.content_flags          = "";
		e.content_flags_disabled = "";

		return e;
	}

	static GroundSpawns GetGroundSpawns(
		const std::vector<GroundSpawns> &ground_spawnss,
		int ground_spawns_id
	)
	{
		for (auto &ground_spawns : ground_spawnss) {
			if (ground_spawns.id == ground_spawns_id) {
				return ground_spawns;
			}
		}

		return NewEntity();
	}

	static GroundSpawns FindOne(
		Database& db,
		int ground_spawns_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				ground_spawns_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			GroundSpawns e{};

			e.id                     = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.zoneid                 = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.version                = row[2] ? static_cast<int16_t>(atoi(row[2])) : 0;
			e.max_x                  = row[3] ? strtof(row[3], nullptr) : 2000;
			e.max_y                  = row[4] ? strtof(row[4], nullptr) : 2000;
			e.max_z                  = row[5] ? strtof(row[5], nullptr) : 10000;
			e.min_x                  = row[6] ? strtof(row[6], nullptr) : -2000;
			e.min_y                  = row[7] ? strtof(row[7], nullptr) : -2000;
			e.heading                = row[8] ? strtof(row[8], nullptr) : 0;
			e.name                   = row[9] ? row[9] : "";
			e.item                   = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.max_allowed            = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 1;
			e.comment                = row[12] ? row[12] : "";
			e.respawn_timer          = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 300;
			e.fix_z                  = row[14] ? static_cast<uint8_t>(strtoul(row[14], nullptr, 10)) : 1;
			e.min_expansion          = row[15] ? static_cast<int8_t>(atoi(row[15])) : -1;
			e.max_expansion          = row[16] ? static_cast<int8_t>(atoi(row[16])) : -1;
			e.content_flags          = row[17] ? row[17] : "";
			e.content_flags_disabled = row[18] ? row[18] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int ground_spawns_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				ground_spawns_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const GroundSpawns &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.zoneid));
		v.push_back(columns[2] + " = " + std::to_string(e.version));
		v.push_back(columns[3] + " = " + std::to_string(e.max_x));
		v.push_back(columns[4] + " = " + std::to_string(e.max_y));
		v.push_back(columns[5] + " = " + std::to_string(e.max_z));
		v.push_back(columns[6] + " = " + std::to_string(e.min_x));
		v.push_back(columns[7] + " = " + std::to_string(e.min_y));
		v.push_back(columns[8] + " = " + std::to_string(e.heading));
		v.push_back(columns[9] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[10] + " = " + std::to_string(e.item));
		v.push_back(columns[11] + " = " + std::to_string(e.max_allowed));
		v.push_back(columns[12] + " = '" + Strings::Escape(e.comment) + "'");
		v.push_back(columns[13] + " = " + std::to_string(e.respawn_timer));
		v.push_back(columns[14] + " = " + std::to_string(e.fix_z));
		v.push_back(columns[15] + " = " + std::to_string(e.min_expansion));
		v.push_back(columns[16] + " = " + std::to_string(e.max_expansion));
		v.push_back(columns[17] + " = '" + Strings::Escape(e.content_flags) + "'");
		v.push_back(columns[18] + " = '" + Strings::Escape(e.content_flags_disabled) + "'");

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

	static GroundSpawns InsertOne(
		Database& db,
		GroundSpawns e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.zoneid));
		v.push_back(std::to_string(e.version));
		v.push_back(std::to_string(e.max_x));
		v.push_back(std::to_string(e.max_y));
		v.push_back(std::to_string(e.max_z));
		v.push_back(std::to_string(e.min_x));
		v.push_back(std::to_string(e.min_y));
		v.push_back(std::to_string(e.heading));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.item));
		v.push_back(std::to_string(e.max_allowed));
		v.push_back("'" + Strings::Escape(e.comment) + "'");
		v.push_back(std::to_string(e.respawn_timer));
		v.push_back(std::to_string(e.fix_z));
		v.push_back(std::to_string(e.min_expansion));
		v.push_back(std::to_string(e.max_expansion));
		v.push_back("'" + Strings::Escape(e.content_flags) + "'");
		v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

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
		const std::vector<GroundSpawns> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.zoneid));
			v.push_back(std::to_string(e.version));
			v.push_back(std::to_string(e.max_x));
			v.push_back(std::to_string(e.max_y));
			v.push_back(std::to_string(e.max_z));
			v.push_back(std::to_string(e.min_x));
			v.push_back(std::to_string(e.min_y));
			v.push_back(std::to_string(e.heading));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.item));
			v.push_back(std::to_string(e.max_allowed));
			v.push_back("'" + Strings::Escape(e.comment) + "'");
			v.push_back(std::to_string(e.respawn_timer));
			v.push_back(std::to_string(e.fix_z));
			v.push_back(std::to_string(e.min_expansion));
			v.push_back(std::to_string(e.max_expansion));
			v.push_back("'" + Strings::Escape(e.content_flags) + "'");
			v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

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

	static std::vector<GroundSpawns> All(Database& db)
	{
		std::vector<GroundSpawns> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GroundSpawns e{};

			e.id                     = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.zoneid                 = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.version                = row[2] ? static_cast<int16_t>(atoi(row[2])) : 0;
			e.max_x                  = row[3] ? strtof(row[3], nullptr) : 2000;
			e.max_y                  = row[4] ? strtof(row[4], nullptr) : 2000;
			e.max_z                  = row[5] ? strtof(row[5], nullptr) : 10000;
			e.min_x                  = row[6] ? strtof(row[6], nullptr) : -2000;
			e.min_y                  = row[7] ? strtof(row[7], nullptr) : -2000;
			e.heading                = row[8] ? strtof(row[8], nullptr) : 0;
			e.name                   = row[9] ? row[9] : "";
			e.item                   = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.max_allowed            = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 1;
			e.comment                = row[12] ? row[12] : "";
			e.respawn_timer          = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 300;
			e.fix_z                  = row[14] ? static_cast<uint8_t>(strtoul(row[14], nullptr, 10)) : 1;
			e.min_expansion          = row[15] ? static_cast<int8_t>(atoi(row[15])) : -1;
			e.max_expansion          = row[16] ? static_cast<int8_t>(atoi(row[16])) : -1;
			e.content_flags          = row[17] ? row[17] : "";
			e.content_flags_disabled = row[18] ? row[18] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<GroundSpawns> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<GroundSpawns> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GroundSpawns e{};

			e.id                     = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.zoneid                 = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.version                = row[2] ? static_cast<int16_t>(atoi(row[2])) : 0;
			e.max_x                  = row[3] ? strtof(row[3], nullptr) : 2000;
			e.max_y                  = row[4] ? strtof(row[4], nullptr) : 2000;
			e.max_z                  = row[5] ? strtof(row[5], nullptr) : 10000;
			e.min_x                  = row[6] ? strtof(row[6], nullptr) : -2000;
			e.min_y                  = row[7] ? strtof(row[7], nullptr) : -2000;
			e.heading                = row[8] ? strtof(row[8], nullptr) : 0;
			e.name                   = row[9] ? row[9] : "";
			e.item                   = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.max_allowed            = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 1;
			e.comment                = row[12] ? row[12] : "";
			e.respawn_timer          = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 300;
			e.fix_z                  = row[14] ? static_cast<uint8_t>(strtoul(row[14], nullptr, 10)) : 1;
			e.min_expansion          = row[15] ? static_cast<int8_t>(atoi(row[15])) : -1;
			e.max_expansion          = row[16] ? static_cast<int8_t>(atoi(row[16])) : -1;
			e.content_flags          = row[17] ? row[17] : "";
			e.content_flags_disabled = row[18] ? row[18] : "";

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
		const GroundSpawns &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.zoneid));
		v.push_back(std::to_string(e.version));
		v.push_back(std::to_string(e.max_x));
		v.push_back(std::to_string(e.max_y));
		v.push_back(std::to_string(e.max_z));
		v.push_back(std::to_string(e.min_x));
		v.push_back(std::to_string(e.min_y));
		v.push_back(std::to_string(e.heading));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.item));
		v.push_back(std::to_string(e.max_allowed));
		v.push_back("'" + Strings::Escape(e.comment) + "'");
		v.push_back(std::to_string(e.respawn_timer));
		v.push_back(std::to_string(e.fix_z));
		v.push_back(std::to_string(e.min_expansion));
		v.push_back(std::to_string(e.max_expansion));
		v.push_back("'" + Strings::Escape(e.content_flags) + "'");
		v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

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
		const std::vector<GroundSpawns> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.zoneid));
			v.push_back(std::to_string(e.version));
			v.push_back(std::to_string(e.max_x));
			v.push_back(std::to_string(e.max_y));
			v.push_back(std::to_string(e.max_z));
			v.push_back(std::to_string(e.min_x));
			v.push_back(std::to_string(e.min_y));
			v.push_back(std::to_string(e.heading));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.item));
			v.push_back(std::to_string(e.max_allowed));
			v.push_back("'" + Strings::Escape(e.comment) + "'");
			v.push_back(std::to_string(e.respawn_timer));
			v.push_back(std::to_string(e.fix_z));
			v.push_back(std::to_string(e.min_expansion));
			v.push_back(std::to_string(e.max_expansion));
			v.push_back("'" + Strings::Escape(e.content_flags) + "'");
			v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

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
