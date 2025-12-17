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

class BaseBlockedSpellsRepository {
public:
	struct BlockedSpells {
		int32_t     id;
		uint32_t    spellid;
		int8_t      type;
		int32_t     zoneid;
		float       x;
		float       y;
		float       z;
		float       x_diff;
		float       y_diff;
		float       z_diff;
		std::string message;
		std::string description;
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
			"spellid",
			"type",
			"zoneid",
			"x",
			"y",
			"z",
			"x_diff",
			"y_diff",
			"z_diff",
			"message",
			"description",
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
			"spellid",
			"type",
			"zoneid",
			"x",
			"y",
			"z",
			"x_diff",
			"y_diff",
			"z_diff",
			"message",
			"description",
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
		return std::string("blocked_spells");
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

	static BlockedSpells NewEntity()
	{
		BlockedSpells e{};

		e.id                     = 0;
		e.spellid                = 0;
		e.type                   = 0;
		e.zoneid                 = 0;
		e.x                      = 0;
		e.y                      = 0;
		e.z                      = 0;
		e.x_diff                 = 0;
		e.y_diff                 = 0;
		e.z_diff                 = 0;
		e.message                = "";
		e.description            = "";
		e.min_expansion          = -1;
		e.max_expansion          = -1;
		e.content_flags          = "";
		e.content_flags_disabled = "";

		return e;
	}

	static BlockedSpells GetBlockedSpells(
		const std::vector<BlockedSpells> &blocked_spellss,
		int blocked_spells_id
	)
	{
		for (auto &blocked_spells : blocked_spellss) {
			if (blocked_spells.id == blocked_spells_id) {
				return blocked_spells;
			}
		}

		return NewEntity();
	}

	static BlockedSpells FindOne(
		Database& db,
		int blocked_spells_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				blocked_spells_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			BlockedSpells e{};

			e.id                     = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.spellid                = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.type                   = row[2] ? static_cast<int8_t>(atoi(row[2])) : 0;
			e.zoneid                 = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.x                      = row[4] ? strtof(row[4], nullptr) : 0;
			e.y                      = row[5] ? strtof(row[5], nullptr) : 0;
			e.z                      = row[6] ? strtof(row[6], nullptr) : 0;
			e.x_diff                 = row[7] ? strtof(row[7], nullptr) : 0;
			e.y_diff                 = row[8] ? strtof(row[8], nullptr) : 0;
			e.z_diff                 = row[9] ? strtof(row[9], nullptr) : 0;
			e.message                = row[10] ? row[10] : "";
			e.description            = row[11] ? row[11] : "";
			e.min_expansion          = row[12] ? static_cast<int8_t>(atoi(row[12])) : -1;
			e.max_expansion          = row[13] ? static_cast<int8_t>(atoi(row[13])) : -1;
			e.content_flags          = row[14] ? row[14] : "";
			e.content_flags_disabled = row[15] ? row[15] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int blocked_spells_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				blocked_spells_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const BlockedSpells &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.spellid));
		v.push_back(columns[2] + " = " + std::to_string(e.type));
		v.push_back(columns[3] + " = " + std::to_string(e.zoneid));
		v.push_back(columns[4] + " = " + std::to_string(e.x));
		v.push_back(columns[5] + " = " + std::to_string(e.y));
		v.push_back(columns[6] + " = " + std::to_string(e.z));
		v.push_back(columns[7] + " = " + std::to_string(e.x_diff));
		v.push_back(columns[8] + " = " + std::to_string(e.y_diff));
		v.push_back(columns[9] + " = " + std::to_string(e.z_diff));
		v.push_back(columns[10] + " = '" + Strings::Escape(e.message) + "'");
		v.push_back(columns[11] + " = '" + Strings::Escape(e.description) + "'");
		v.push_back(columns[12] + " = " + std::to_string(e.min_expansion));
		v.push_back(columns[13] + " = " + std::to_string(e.max_expansion));
		v.push_back(columns[14] + " = '" + Strings::Escape(e.content_flags) + "'");
		v.push_back(columns[15] + " = '" + Strings::Escape(e.content_flags_disabled) + "'");

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

	static BlockedSpells InsertOne(
		Database& db,
		BlockedSpells e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.spellid));
		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.zoneid));
		v.push_back(std::to_string(e.x));
		v.push_back(std::to_string(e.y));
		v.push_back(std::to_string(e.z));
		v.push_back(std::to_string(e.x_diff));
		v.push_back(std::to_string(e.y_diff));
		v.push_back(std::to_string(e.z_diff));
		v.push_back("'" + Strings::Escape(e.message) + "'");
		v.push_back("'" + Strings::Escape(e.description) + "'");
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
		const std::vector<BlockedSpells> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.spellid));
			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.zoneid));
			v.push_back(std::to_string(e.x));
			v.push_back(std::to_string(e.y));
			v.push_back(std::to_string(e.z));
			v.push_back(std::to_string(e.x_diff));
			v.push_back(std::to_string(e.y_diff));
			v.push_back(std::to_string(e.z_diff));
			v.push_back("'" + Strings::Escape(e.message) + "'");
			v.push_back("'" + Strings::Escape(e.description) + "'");
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

	static std::vector<BlockedSpells> All(Database& db)
	{
		std::vector<BlockedSpells> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BlockedSpells e{};

			e.id                     = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.spellid                = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.type                   = row[2] ? static_cast<int8_t>(atoi(row[2])) : 0;
			e.zoneid                 = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.x                      = row[4] ? strtof(row[4], nullptr) : 0;
			e.y                      = row[5] ? strtof(row[5], nullptr) : 0;
			e.z                      = row[6] ? strtof(row[6], nullptr) : 0;
			e.x_diff                 = row[7] ? strtof(row[7], nullptr) : 0;
			e.y_diff                 = row[8] ? strtof(row[8], nullptr) : 0;
			e.z_diff                 = row[9] ? strtof(row[9], nullptr) : 0;
			e.message                = row[10] ? row[10] : "";
			e.description            = row[11] ? row[11] : "";
			e.min_expansion          = row[12] ? static_cast<int8_t>(atoi(row[12])) : -1;
			e.max_expansion          = row[13] ? static_cast<int8_t>(atoi(row[13])) : -1;
			e.content_flags          = row[14] ? row[14] : "";
			e.content_flags_disabled = row[15] ? row[15] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<BlockedSpells> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<BlockedSpells> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BlockedSpells e{};

			e.id                     = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.spellid                = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.type                   = row[2] ? static_cast<int8_t>(atoi(row[2])) : 0;
			e.zoneid                 = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.x                      = row[4] ? strtof(row[4], nullptr) : 0;
			e.y                      = row[5] ? strtof(row[5], nullptr) : 0;
			e.z                      = row[6] ? strtof(row[6], nullptr) : 0;
			e.x_diff                 = row[7] ? strtof(row[7], nullptr) : 0;
			e.y_diff                 = row[8] ? strtof(row[8], nullptr) : 0;
			e.z_diff                 = row[9] ? strtof(row[9], nullptr) : 0;
			e.message                = row[10] ? row[10] : "";
			e.description            = row[11] ? row[11] : "";
			e.min_expansion          = row[12] ? static_cast<int8_t>(atoi(row[12])) : -1;
			e.max_expansion          = row[13] ? static_cast<int8_t>(atoi(row[13])) : -1;
			e.content_flags          = row[14] ? row[14] : "";
			e.content_flags_disabled = row[15] ? row[15] : "";

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
		const BlockedSpells &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.spellid));
		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.zoneid));
		v.push_back(std::to_string(e.x));
		v.push_back(std::to_string(e.y));
		v.push_back(std::to_string(e.z));
		v.push_back(std::to_string(e.x_diff));
		v.push_back(std::to_string(e.y_diff));
		v.push_back(std::to_string(e.z_diff));
		v.push_back("'" + Strings::Escape(e.message) + "'");
		v.push_back("'" + Strings::Escape(e.description) + "'");
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
		const std::vector<BlockedSpells> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.spellid));
			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.zoneid));
			v.push_back(std::to_string(e.x));
			v.push_back(std::to_string(e.y));
			v.push_back(std::to_string(e.z));
			v.push_back(std::to_string(e.x_diff));
			v.push_back(std::to_string(e.y_diff));
			v.push_back(std::to_string(e.z_diff));
			v.push_back("'" + Strings::Escape(e.message) + "'");
			v.push_back("'" + Strings::Escape(e.description) + "'");
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
