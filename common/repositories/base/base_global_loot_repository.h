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

class BaseGlobalLootRepository {
public:
	struct GlobalLoot {
		int32_t     id;
		std::string description;
		int32_t     loottable_id;
		int8_t      enabled;
		int32_t     min_level;
		int32_t     max_level;
		int8_t      rare;
		int8_t      raid;
		std::string race;
		std::string class_;
		std::string bodytype;
		std::string zone;
		int8_t      hot_zone;
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
			"description",
			"loottable_id",
			"enabled",
			"min_level",
			"max_level",
			"rare",
			"raid",
			"race",
			"`class`",
			"bodytype",
			"zone",
			"hot_zone",
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
			"description",
			"loottable_id",
			"enabled",
			"min_level",
			"max_level",
			"rare",
			"raid",
			"race",
			"`class`",
			"bodytype",
			"zone",
			"hot_zone",
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
		return std::string("global_loot");
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

	static GlobalLoot NewEntity()
	{
		GlobalLoot e{};

		e.id                     = 0;
		e.description            = "";
		e.loottable_id           = 0;
		e.enabled                = 1;
		e.min_level              = 0;
		e.max_level              = 0;
		e.rare                   = 0;
		e.raid                   = 0;
		e.race                   = "";
		e.class_                 = "";
		e.bodytype               = "";
		e.zone                   = "";
		e.hot_zone               = 0;
		e.min_expansion          = -1;
		e.max_expansion          = -1;
		e.content_flags          = "";
		e.content_flags_disabled = "";

		return e;
	}

	static GlobalLoot GetGlobalLoot(
		const std::vector<GlobalLoot> &global_loots,
		int global_loot_id
	)
	{
		for (auto &global_loot : global_loots) {
			if (global_loot.id == global_loot_id) {
				return global_loot;
			}
		}

		return NewEntity();
	}

	static GlobalLoot FindOne(
		Database& db,
		int global_loot_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				global_loot_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			GlobalLoot e{};

			e.id                     = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.description            = row[1] ? row[1] : "";
			e.loottable_id           = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.enabled                = row[3] ? static_cast<int8_t>(atoi(row[3])) : 1;
			e.min_level              = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.max_level              = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.rare                   = row[6] ? static_cast<int8_t>(atoi(row[6])) : 0;
			e.raid                   = row[7] ? static_cast<int8_t>(atoi(row[7])) : 0;
			e.race                   = row[8] ? row[8] : "";
			e.class_                 = row[9] ? row[9] : "";
			e.bodytype               = row[10] ? row[10] : "";
			e.zone                   = row[11] ? row[11] : "";
			e.hot_zone               = row[12] ? static_cast<int8_t>(atoi(row[12])) : 0;
			e.min_expansion          = row[13] ? static_cast<int8_t>(atoi(row[13])) : -1;
			e.max_expansion          = row[14] ? static_cast<int8_t>(atoi(row[14])) : -1;
			e.content_flags          = row[15] ? row[15] : "";
			e.content_flags_disabled = row[16] ? row[16] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int global_loot_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				global_loot_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const GlobalLoot &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.description) + "'");
		v.push_back(columns[2] + " = " + std::to_string(e.loottable_id));
		v.push_back(columns[3] + " = " + std::to_string(e.enabled));
		v.push_back(columns[4] + " = " + std::to_string(e.min_level));
		v.push_back(columns[5] + " = " + std::to_string(e.max_level));
		v.push_back(columns[6] + " = " + std::to_string(e.rare));
		v.push_back(columns[7] + " = " + std::to_string(e.raid));
		v.push_back(columns[8] + " = '" + Strings::Escape(e.race) + "'");
		v.push_back(columns[9] + " = '" + Strings::Escape(e.class_) + "'");
		v.push_back(columns[10] + " = '" + Strings::Escape(e.bodytype) + "'");
		v.push_back(columns[11] + " = '" + Strings::Escape(e.zone) + "'");
		v.push_back(columns[12] + " = " + std::to_string(e.hot_zone));
		v.push_back(columns[13] + " = " + std::to_string(e.min_expansion));
		v.push_back(columns[14] + " = " + std::to_string(e.max_expansion));
		v.push_back(columns[15] + " = '" + Strings::Escape(e.content_flags) + "'");
		v.push_back(columns[16] + " = '" + Strings::Escape(e.content_flags_disabled) + "'");

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

	static GlobalLoot InsertOne(
		Database& db,
		GlobalLoot e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.description) + "'");
		v.push_back(std::to_string(e.loottable_id));
		v.push_back(std::to_string(e.enabled));
		v.push_back(std::to_string(e.min_level));
		v.push_back(std::to_string(e.max_level));
		v.push_back(std::to_string(e.rare));
		v.push_back(std::to_string(e.raid));
		v.push_back("'" + Strings::Escape(e.race) + "'");
		v.push_back("'" + Strings::Escape(e.class_) + "'");
		v.push_back("'" + Strings::Escape(e.bodytype) + "'");
		v.push_back("'" + Strings::Escape(e.zone) + "'");
		v.push_back(std::to_string(e.hot_zone));
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
		const std::vector<GlobalLoot> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.description) + "'");
			v.push_back(std::to_string(e.loottable_id));
			v.push_back(std::to_string(e.enabled));
			v.push_back(std::to_string(e.min_level));
			v.push_back(std::to_string(e.max_level));
			v.push_back(std::to_string(e.rare));
			v.push_back(std::to_string(e.raid));
			v.push_back("'" + Strings::Escape(e.race) + "'");
			v.push_back("'" + Strings::Escape(e.class_) + "'");
			v.push_back("'" + Strings::Escape(e.bodytype) + "'");
			v.push_back("'" + Strings::Escape(e.zone) + "'");
			v.push_back(std::to_string(e.hot_zone));
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

	static std::vector<GlobalLoot> All(Database& db)
	{
		std::vector<GlobalLoot> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GlobalLoot e{};

			e.id                     = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.description            = row[1] ? row[1] : "";
			e.loottable_id           = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.enabled                = row[3] ? static_cast<int8_t>(atoi(row[3])) : 1;
			e.min_level              = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.max_level              = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.rare                   = row[6] ? static_cast<int8_t>(atoi(row[6])) : 0;
			e.raid                   = row[7] ? static_cast<int8_t>(atoi(row[7])) : 0;
			e.race                   = row[8] ? row[8] : "";
			e.class_                 = row[9] ? row[9] : "";
			e.bodytype               = row[10] ? row[10] : "";
			e.zone                   = row[11] ? row[11] : "";
			e.hot_zone               = row[12] ? static_cast<int8_t>(atoi(row[12])) : 0;
			e.min_expansion          = row[13] ? static_cast<int8_t>(atoi(row[13])) : -1;
			e.max_expansion          = row[14] ? static_cast<int8_t>(atoi(row[14])) : -1;
			e.content_flags          = row[15] ? row[15] : "";
			e.content_flags_disabled = row[16] ? row[16] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<GlobalLoot> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<GlobalLoot> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GlobalLoot e{};

			e.id                     = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.description            = row[1] ? row[1] : "";
			e.loottable_id           = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.enabled                = row[3] ? static_cast<int8_t>(atoi(row[3])) : 1;
			e.min_level              = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.max_level              = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.rare                   = row[6] ? static_cast<int8_t>(atoi(row[6])) : 0;
			e.raid                   = row[7] ? static_cast<int8_t>(atoi(row[7])) : 0;
			e.race                   = row[8] ? row[8] : "";
			e.class_                 = row[9] ? row[9] : "";
			e.bodytype               = row[10] ? row[10] : "";
			e.zone                   = row[11] ? row[11] : "";
			e.hot_zone               = row[12] ? static_cast<int8_t>(atoi(row[12])) : 0;
			e.min_expansion          = row[13] ? static_cast<int8_t>(atoi(row[13])) : -1;
			e.max_expansion          = row[14] ? static_cast<int8_t>(atoi(row[14])) : -1;
			e.content_flags          = row[15] ? row[15] : "";
			e.content_flags_disabled = row[16] ? row[16] : "";

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
		const GlobalLoot &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.description) + "'");
		v.push_back(std::to_string(e.loottable_id));
		v.push_back(std::to_string(e.enabled));
		v.push_back(std::to_string(e.min_level));
		v.push_back(std::to_string(e.max_level));
		v.push_back(std::to_string(e.rare));
		v.push_back(std::to_string(e.raid));
		v.push_back("'" + Strings::Escape(e.race) + "'");
		v.push_back("'" + Strings::Escape(e.class_) + "'");
		v.push_back("'" + Strings::Escape(e.bodytype) + "'");
		v.push_back("'" + Strings::Escape(e.zone) + "'");
		v.push_back(std::to_string(e.hot_zone));
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
		const std::vector<GlobalLoot> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.description) + "'");
			v.push_back(std::to_string(e.loottable_id));
			v.push_back(std::to_string(e.enabled));
			v.push_back(std::to_string(e.min_level));
			v.push_back(std::to_string(e.max_level));
			v.push_back(std::to_string(e.rare));
			v.push_back(std::to_string(e.raid));
			v.push_back("'" + Strings::Escape(e.race) + "'");
			v.push_back("'" + Strings::Escape(e.class_) + "'");
			v.push_back("'" + Strings::Escape(e.bodytype) + "'");
			v.push_back("'" + Strings::Escape(e.zone) + "'");
			v.push_back(std::to_string(e.hot_zone));
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
