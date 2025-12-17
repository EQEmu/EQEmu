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

class BaseSpawn2Repository {
public:
	struct Spawn2 {
		int32_t     id;
		int32_t     spawngroupID;
		std::string zone;
		int16_t     version;
		float       x;
		float       y;
		float       z;
		float       heading;
		int32_t     respawntime;
		int32_t     variance;
		int32_t     pathgrid;
		int8_t      path_when_zone_idle;
		uint32_t    _condition;
		int32_t     cond_value;
		uint8_t     animation;
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
			"spawngroupID",
			"zone",
			"version",
			"x",
			"y",
			"z",
			"heading",
			"respawntime",
			"variance",
			"pathgrid",
			"path_when_zone_idle",
			"_condition",
			"cond_value",
			"animation",
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
			"spawngroupID",
			"zone",
			"version",
			"x",
			"y",
			"z",
			"heading",
			"respawntime",
			"variance",
			"pathgrid",
			"path_when_zone_idle",
			"_condition",
			"cond_value",
			"animation",
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
		return std::string("spawn2");
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

	static Spawn2 NewEntity()
	{
		Spawn2 e{};

		e.id                     = 0;
		e.spawngroupID           = 0;
		e.zone                   = "";
		e.version                = 0;
		e.x                      = 0.000000;
		e.y                      = 0.000000;
		e.z                      = 0.000000;
		e.heading                = 0.000000;
		e.respawntime            = 0;
		e.variance               = 0;
		e.pathgrid               = 0;
		e.path_when_zone_idle    = 0;
		e._condition             = 0;
		e.cond_value             = 1;
		e.animation              = 0;
		e.min_expansion          = -1;
		e.max_expansion          = -1;
		e.content_flags          = "";
		e.content_flags_disabled = "";

		return e;
	}

	static Spawn2 GetSpawn2(
		const std::vector<Spawn2> &spawn2s,
		int spawn2_id
	)
	{
		for (auto &spawn2 : spawn2s) {
			if (spawn2.id == spawn2_id) {
				return spawn2;
			}
		}

		return NewEntity();
	}

	static Spawn2 FindOne(
		Database& db,
		int spawn2_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				spawn2_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Spawn2 e{};

			e.id                     = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.spawngroupID           = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.zone                   = row[2] ? row[2] : "";
			e.version                = row[3] ? static_cast<int16_t>(atoi(row[3])) : 0;
			e.x                      = row[4] ? strtof(row[4], nullptr) : 0.000000;
			e.y                      = row[5] ? strtof(row[5], nullptr) : 0.000000;
			e.z                      = row[6] ? strtof(row[6], nullptr) : 0.000000;
			e.heading                = row[7] ? strtof(row[7], nullptr) : 0.000000;
			e.respawntime            = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.variance               = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.pathgrid               = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.path_when_zone_idle    = row[11] ? static_cast<int8_t>(atoi(row[11])) : 0;
			e._condition             = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.cond_value             = row[13] ? static_cast<int32_t>(atoi(row[13])) : 1;
			e.animation              = row[14] ? static_cast<uint8_t>(strtoul(row[14], nullptr, 10)) : 0;
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
		int spawn2_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				spawn2_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Spawn2 &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.spawngroupID));
		v.push_back(columns[2] + " = '" + Strings::Escape(e.zone) + "'");
		v.push_back(columns[3] + " = " + std::to_string(e.version));
		v.push_back(columns[4] + " = " + std::to_string(e.x));
		v.push_back(columns[5] + " = " + std::to_string(e.y));
		v.push_back(columns[6] + " = " + std::to_string(e.z));
		v.push_back(columns[7] + " = " + std::to_string(e.heading));
		v.push_back(columns[8] + " = " + std::to_string(e.respawntime));
		v.push_back(columns[9] + " = " + std::to_string(e.variance));
		v.push_back(columns[10] + " = " + std::to_string(e.pathgrid));
		v.push_back(columns[11] + " = " + std::to_string(e.path_when_zone_idle));
		v.push_back(columns[12] + " = " + std::to_string(e._condition));
		v.push_back(columns[13] + " = " + std::to_string(e.cond_value));
		v.push_back(columns[14] + " = " + std::to_string(e.animation));
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

	static Spawn2 InsertOne(
		Database& db,
		Spawn2 e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.spawngroupID));
		v.push_back("'" + Strings::Escape(e.zone) + "'");
		v.push_back(std::to_string(e.version));
		v.push_back(std::to_string(e.x));
		v.push_back(std::to_string(e.y));
		v.push_back(std::to_string(e.z));
		v.push_back(std::to_string(e.heading));
		v.push_back(std::to_string(e.respawntime));
		v.push_back(std::to_string(e.variance));
		v.push_back(std::to_string(e.pathgrid));
		v.push_back(std::to_string(e.path_when_zone_idle));
		v.push_back(std::to_string(e._condition));
		v.push_back(std::to_string(e.cond_value));
		v.push_back(std::to_string(e.animation));
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
		const std::vector<Spawn2> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.spawngroupID));
			v.push_back("'" + Strings::Escape(e.zone) + "'");
			v.push_back(std::to_string(e.version));
			v.push_back(std::to_string(e.x));
			v.push_back(std::to_string(e.y));
			v.push_back(std::to_string(e.z));
			v.push_back(std::to_string(e.heading));
			v.push_back(std::to_string(e.respawntime));
			v.push_back(std::to_string(e.variance));
			v.push_back(std::to_string(e.pathgrid));
			v.push_back(std::to_string(e.path_when_zone_idle));
			v.push_back(std::to_string(e._condition));
			v.push_back(std::to_string(e.cond_value));
			v.push_back(std::to_string(e.animation));
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

	static std::vector<Spawn2> All(Database& db)
	{
		std::vector<Spawn2> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Spawn2 e{};

			e.id                     = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.spawngroupID           = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.zone                   = row[2] ? row[2] : "";
			e.version                = row[3] ? static_cast<int16_t>(atoi(row[3])) : 0;
			e.x                      = row[4] ? strtof(row[4], nullptr) : 0.000000;
			e.y                      = row[5] ? strtof(row[5], nullptr) : 0.000000;
			e.z                      = row[6] ? strtof(row[6], nullptr) : 0.000000;
			e.heading                = row[7] ? strtof(row[7], nullptr) : 0.000000;
			e.respawntime            = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.variance               = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.pathgrid               = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.path_when_zone_idle    = row[11] ? static_cast<int8_t>(atoi(row[11])) : 0;
			e._condition             = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.cond_value             = row[13] ? static_cast<int32_t>(atoi(row[13])) : 1;
			e.animation              = row[14] ? static_cast<uint8_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.min_expansion          = row[15] ? static_cast<int8_t>(atoi(row[15])) : -1;
			e.max_expansion          = row[16] ? static_cast<int8_t>(atoi(row[16])) : -1;
			e.content_flags          = row[17] ? row[17] : "";
			e.content_flags_disabled = row[18] ? row[18] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Spawn2> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Spawn2> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Spawn2 e{};

			e.id                     = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.spawngroupID           = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.zone                   = row[2] ? row[2] : "";
			e.version                = row[3] ? static_cast<int16_t>(atoi(row[3])) : 0;
			e.x                      = row[4] ? strtof(row[4], nullptr) : 0.000000;
			e.y                      = row[5] ? strtof(row[5], nullptr) : 0.000000;
			e.z                      = row[6] ? strtof(row[6], nullptr) : 0.000000;
			e.heading                = row[7] ? strtof(row[7], nullptr) : 0.000000;
			e.respawntime            = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.variance               = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.pathgrid               = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.path_when_zone_idle    = row[11] ? static_cast<int8_t>(atoi(row[11])) : 0;
			e._condition             = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.cond_value             = row[13] ? static_cast<int32_t>(atoi(row[13])) : 1;
			e.animation              = row[14] ? static_cast<uint8_t>(strtoul(row[14], nullptr, 10)) : 0;
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
		const Spawn2 &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.spawngroupID));
		v.push_back("'" + Strings::Escape(e.zone) + "'");
		v.push_back(std::to_string(e.version));
		v.push_back(std::to_string(e.x));
		v.push_back(std::to_string(e.y));
		v.push_back(std::to_string(e.z));
		v.push_back(std::to_string(e.heading));
		v.push_back(std::to_string(e.respawntime));
		v.push_back(std::to_string(e.variance));
		v.push_back(std::to_string(e.pathgrid));
		v.push_back(std::to_string(e.path_when_zone_idle));
		v.push_back(std::to_string(e._condition));
		v.push_back(std::to_string(e.cond_value));
		v.push_back(std::to_string(e.animation));
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
		const std::vector<Spawn2> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.spawngroupID));
			v.push_back("'" + Strings::Escape(e.zone) + "'");
			v.push_back(std::to_string(e.version));
			v.push_back(std::to_string(e.x));
			v.push_back(std::to_string(e.y));
			v.push_back(std::to_string(e.z));
			v.push_back(std::to_string(e.heading));
			v.push_back(std::to_string(e.respawntime));
			v.push_back(std::to_string(e.variance));
			v.push_back(std::to_string(e.pathgrid));
			v.push_back(std::to_string(e.path_when_zone_idle));
			v.push_back(std::to_string(e._condition));
			v.push_back(std::to_string(e.cond_value));
			v.push_back(std::to_string(e.animation));
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
