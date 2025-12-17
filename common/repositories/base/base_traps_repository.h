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

class BaseTrapsRepository {
public:
	struct Traps {
		int32_t     id;
		std::string zone;
		uint16_t    version;
		int32_t     x;
		int32_t     y;
		int32_t     z;
		int8_t      chance;
		float       maxzdiff;
		float       radius;
		int32_t     effect;
		int32_t     effectvalue;
		int32_t     effectvalue2;
		std::string message;
		int32_t     skill;
		uint32_t    level;
		uint32_t    respawn_time;
		uint32_t    respawn_var;
		int8_t      triggered_number;
		int8_t      group_;
		int8_t      despawn_when_triggered;
		int8_t      undetectable;
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
			"zone",
			"version",
			"x",
			"y",
			"z",
			"chance",
			"maxzdiff",
			"radius",
			"effect",
			"effectvalue",
			"effectvalue2",
			"message",
			"skill",
			"level",
			"respawn_time",
			"respawn_var",
			"triggered_number",
			"`group`",
			"despawn_when_triggered",
			"undetectable",
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
			"zone",
			"version",
			"x",
			"y",
			"z",
			"chance",
			"maxzdiff",
			"radius",
			"effect",
			"effectvalue",
			"effectvalue2",
			"message",
			"skill",
			"level",
			"respawn_time",
			"respawn_var",
			"triggered_number",
			"`group`",
			"despawn_when_triggered",
			"undetectable",
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
		return std::string("traps");
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

	static Traps NewEntity()
	{
		Traps e{};

		e.id                     = 0;
		e.zone                   = "";
		e.version                = 0;
		e.x                      = 0;
		e.y                      = 0;
		e.z                      = 0;
		e.chance                 = 0;
		e.maxzdiff               = 0;
		e.radius                 = 0;
		e.effect                 = 0;
		e.effectvalue            = 0;
		e.effectvalue2           = 0;
		e.message                = "";
		e.skill                  = 0;
		e.level                  = 1;
		e.respawn_time           = 60;
		e.respawn_var            = 0;
		e.triggered_number       = 0;
		e.group_                 = 0;
		e.despawn_when_triggered = 0;
		e.undetectable           = 0;
		e.min_expansion          = -1;
		e.max_expansion          = -1;
		e.content_flags          = "";
		e.content_flags_disabled = "";

		return e;
	}

	static Traps GetTraps(
		const std::vector<Traps> &trapss,
		int traps_id
	)
	{
		for (auto &traps : trapss) {
			if (traps.id == traps_id) {
				return traps;
			}
		}

		return NewEntity();
	}

	static Traps FindOne(
		Database& db,
		int traps_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				traps_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Traps e{};

			e.id                     = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.zone                   = row[1] ? row[1] : "";
			e.version                = row[2] ? static_cast<uint16_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.x                      = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.y                      = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.z                      = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.chance                 = row[6] ? static_cast<int8_t>(atoi(row[6])) : 0;
			e.maxzdiff               = row[7] ? strtof(row[7], nullptr) : 0;
			e.radius                 = row[8] ? strtof(row[8], nullptr) : 0;
			e.effect                 = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.effectvalue            = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.effectvalue2           = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;
			e.message                = row[12] ? row[12] : "";
			e.skill                  = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.level                  = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 1;
			e.respawn_time           = row[15] ? static_cast<uint32_t>(strtoul(row[15], nullptr, 10)) : 60;
			e.respawn_var            = row[16] ? static_cast<uint32_t>(strtoul(row[16], nullptr, 10)) : 0;
			e.triggered_number       = row[17] ? static_cast<int8_t>(atoi(row[17])) : 0;
			e.group_                 = row[18] ? static_cast<int8_t>(atoi(row[18])) : 0;
			e.despawn_when_triggered = row[19] ? static_cast<int8_t>(atoi(row[19])) : 0;
			e.undetectable           = row[20] ? static_cast<int8_t>(atoi(row[20])) : 0;
			e.min_expansion          = row[21] ? static_cast<int8_t>(atoi(row[21])) : -1;
			e.max_expansion          = row[22] ? static_cast<int8_t>(atoi(row[22])) : -1;
			e.content_flags          = row[23] ? row[23] : "";
			e.content_flags_disabled = row[24] ? row[24] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int traps_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				traps_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Traps &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.zone) + "'");
		v.push_back(columns[2] + " = " + std::to_string(e.version));
		v.push_back(columns[3] + " = " + std::to_string(e.x));
		v.push_back(columns[4] + " = " + std::to_string(e.y));
		v.push_back(columns[5] + " = " + std::to_string(e.z));
		v.push_back(columns[6] + " = " + std::to_string(e.chance));
		v.push_back(columns[7] + " = " + std::to_string(e.maxzdiff));
		v.push_back(columns[8] + " = " + std::to_string(e.radius));
		v.push_back(columns[9] + " = " + std::to_string(e.effect));
		v.push_back(columns[10] + " = " + std::to_string(e.effectvalue));
		v.push_back(columns[11] + " = " + std::to_string(e.effectvalue2));
		v.push_back(columns[12] + " = '" + Strings::Escape(e.message) + "'");
		v.push_back(columns[13] + " = " + std::to_string(e.skill));
		v.push_back(columns[14] + " = " + std::to_string(e.level));
		v.push_back(columns[15] + " = " + std::to_string(e.respawn_time));
		v.push_back(columns[16] + " = " + std::to_string(e.respawn_var));
		v.push_back(columns[17] + " = " + std::to_string(e.triggered_number));
		v.push_back(columns[18] + " = " + std::to_string(e.group_));
		v.push_back(columns[19] + " = " + std::to_string(e.despawn_when_triggered));
		v.push_back(columns[20] + " = " + std::to_string(e.undetectable));
		v.push_back(columns[21] + " = " + std::to_string(e.min_expansion));
		v.push_back(columns[22] + " = " + std::to_string(e.max_expansion));
		v.push_back(columns[23] + " = '" + Strings::Escape(e.content_flags) + "'");
		v.push_back(columns[24] + " = '" + Strings::Escape(e.content_flags_disabled) + "'");

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

	static Traps InsertOne(
		Database& db,
		Traps e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.zone) + "'");
		v.push_back(std::to_string(e.version));
		v.push_back(std::to_string(e.x));
		v.push_back(std::to_string(e.y));
		v.push_back(std::to_string(e.z));
		v.push_back(std::to_string(e.chance));
		v.push_back(std::to_string(e.maxzdiff));
		v.push_back(std::to_string(e.radius));
		v.push_back(std::to_string(e.effect));
		v.push_back(std::to_string(e.effectvalue));
		v.push_back(std::to_string(e.effectvalue2));
		v.push_back("'" + Strings::Escape(e.message) + "'");
		v.push_back(std::to_string(e.skill));
		v.push_back(std::to_string(e.level));
		v.push_back(std::to_string(e.respawn_time));
		v.push_back(std::to_string(e.respawn_var));
		v.push_back(std::to_string(e.triggered_number));
		v.push_back(std::to_string(e.group_));
		v.push_back(std::to_string(e.despawn_when_triggered));
		v.push_back(std::to_string(e.undetectable));
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
		const std::vector<Traps> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.zone) + "'");
			v.push_back(std::to_string(e.version));
			v.push_back(std::to_string(e.x));
			v.push_back(std::to_string(e.y));
			v.push_back(std::to_string(e.z));
			v.push_back(std::to_string(e.chance));
			v.push_back(std::to_string(e.maxzdiff));
			v.push_back(std::to_string(e.radius));
			v.push_back(std::to_string(e.effect));
			v.push_back(std::to_string(e.effectvalue));
			v.push_back(std::to_string(e.effectvalue2));
			v.push_back("'" + Strings::Escape(e.message) + "'");
			v.push_back(std::to_string(e.skill));
			v.push_back(std::to_string(e.level));
			v.push_back(std::to_string(e.respawn_time));
			v.push_back(std::to_string(e.respawn_var));
			v.push_back(std::to_string(e.triggered_number));
			v.push_back(std::to_string(e.group_));
			v.push_back(std::to_string(e.despawn_when_triggered));
			v.push_back(std::to_string(e.undetectable));
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

	static std::vector<Traps> All(Database& db)
	{
		std::vector<Traps> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Traps e{};

			e.id                     = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.zone                   = row[1] ? row[1] : "";
			e.version                = row[2] ? static_cast<uint16_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.x                      = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.y                      = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.z                      = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.chance                 = row[6] ? static_cast<int8_t>(atoi(row[6])) : 0;
			e.maxzdiff               = row[7] ? strtof(row[7], nullptr) : 0;
			e.radius                 = row[8] ? strtof(row[8], nullptr) : 0;
			e.effect                 = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.effectvalue            = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.effectvalue2           = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;
			e.message                = row[12] ? row[12] : "";
			e.skill                  = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.level                  = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 1;
			e.respawn_time           = row[15] ? static_cast<uint32_t>(strtoul(row[15], nullptr, 10)) : 60;
			e.respawn_var            = row[16] ? static_cast<uint32_t>(strtoul(row[16], nullptr, 10)) : 0;
			e.triggered_number       = row[17] ? static_cast<int8_t>(atoi(row[17])) : 0;
			e.group_                 = row[18] ? static_cast<int8_t>(atoi(row[18])) : 0;
			e.despawn_when_triggered = row[19] ? static_cast<int8_t>(atoi(row[19])) : 0;
			e.undetectable           = row[20] ? static_cast<int8_t>(atoi(row[20])) : 0;
			e.min_expansion          = row[21] ? static_cast<int8_t>(atoi(row[21])) : -1;
			e.max_expansion          = row[22] ? static_cast<int8_t>(atoi(row[22])) : -1;
			e.content_flags          = row[23] ? row[23] : "";
			e.content_flags_disabled = row[24] ? row[24] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Traps> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Traps> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Traps e{};

			e.id                     = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.zone                   = row[1] ? row[1] : "";
			e.version                = row[2] ? static_cast<uint16_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.x                      = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.y                      = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.z                      = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.chance                 = row[6] ? static_cast<int8_t>(atoi(row[6])) : 0;
			e.maxzdiff               = row[7] ? strtof(row[7], nullptr) : 0;
			e.radius                 = row[8] ? strtof(row[8], nullptr) : 0;
			e.effect                 = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.effectvalue            = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.effectvalue2           = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;
			e.message                = row[12] ? row[12] : "";
			e.skill                  = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.level                  = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 1;
			e.respawn_time           = row[15] ? static_cast<uint32_t>(strtoul(row[15], nullptr, 10)) : 60;
			e.respawn_var            = row[16] ? static_cast<uint32_t>(strtoul(row[16], nullptr, 10)) : 0;
			e.triggered_number       = row[17] ? static_cast<int8_t>(atoi(row[17])) : 0;
			e.group_                 = row[18] ? static_cast<int8_t>(atoi(row[18])) : 0;
			e.despawn_when_triggered = row[19] ? static_cast<int8_t>(atoi(row[19])) : 0;
			e.undetectable           = row[20] ? static_cast<int8_t>(atoi(row[20])) : 0;
			e.min_expansion          = row[21] ? static_cast<int8_t>(atoi(row[21])) : -1;
			e.max_expansion          = row[22] ? static_cast<int8_t>(atoi(row[22])) : -1;
			e.content_flags          = row[23] ? row[23] : "";
			e.content_flags_disabled = row[24] ? row[24] : "";

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
		const Traps &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.zone) + "'");
		v.push_back(std::to_string(e.version));
		v.push_back(std::to_string(e.x));
		v.push_back(std::to_string(e.y));
		v.push_back(std::to_string(e.z));
		v.push_back(std::to_string(e.chance));
		v.push_back(std::to_string(e.maxzdiff));
		v.push_back(std::to_string(e.radius));
		v.push_back(std::to_string(e.effect));
		v.push_back(std::to_string(e.effectvalue));
		v.push_back(std::to_string(e.effectvalue2));
		v.push_back("'" + Strings::Escape(e.message) + "'");
		v.push_back(std::to_string(e.skill));
		v.push_back(std::to_string(e.level));
		v.push_back(std::to_string(e.respawn_time));
		v.push_back(std::to_string(e.respawn_var));
		v.push_back(std::to_string(e.triggered_number));
		v.push_back(std::to_string(e.group_));
		v.push_back(std::to_string(e.despawn_when_triggered));
		v.push_back(std::to_string(e.undetectable));
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
		const std::vector<Traps> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.zone) + "'");
			v.push_back(std::to_string(e.version));
			v.push_back(std::to_string(e.x));
			v.push_back(std::to_string(e.y));
			v.push_back(std::to_string(e.z));
			v.push_back(std::to_string(e.chance));
			v.push_back(std::to_string(e.maxzdiff));
			v.push_back(std::to_string(e.radius));
			v.push_back(std::to_string(e.effect));
			v.push_back(std::to_string(e.effectvalue));
			v.push_back(std::to_string(e.effectvalue2));
			v.push_back("'" + Strings::Escape(e.message) + "'");
			v.push_back(std::to_string(e.skill));
			v.push_back(std::to_string(e.level));
			v.push_back(std::to_string(e.respawn_time));
			v.push_back(std::to_string(e.respawn_var));
			v.push_back(std::to_string(e.triggered_number));
			v.push_back(std::to_string(e.group_));
			v.push_back(std::to_string(e.despawn_when_triggered));
			v.push_back(std::to_string(e.undetectable));
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
