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

class BaseLoottableEntriesRepository {
public:
	struct LoottableEntries {
		uint32_t loottable_id;
		uint32_t lootdrop_id;
		uint8_t  multiplier;
		uint8_t  droplimit;
		uint8_t  mindrop;
		float    probability;
	};

	static std::string PrimaryKey()
	{
		return std::string("loottable_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"loottable_id",
			"lootdrop_id",
			"multiplier",
			"droplimit",
			"mindrop",
			"probability",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"loottable_id",
			"lootdrop_id",
			"multiplier",
			"droplimit",
			"mindrop",
			"probability",
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
		return std::string("loottable_entries");
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

	static LoottableEntries NewEntity()
	{
		LoottableEntries e{};

		e.loottable_id = 0;
		e.lootdrop_id  = 0;
		e.multiplier   = 1;
		e.droplimit    = 0;
		e.mindrop      = 0;
		e.probability  = 100;

		return e;
	}

	static LoottableEntries GetLoottableEntries(
		const std::vector<LoottableEntries> &loottable_entriess,
		int loottable_entries_id
	)
	{
		for (auto &loottable_entries : loottable_entriess) {
			if (loottable_entries.loottable_id == loottable_entries_id) {
				return loottable_entries;
			}
		}

		return NewEntity();
	}

	static LoottableEntries FindOne(
		Database& db,
		int loottable_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				loottable_entries_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			LoottableEntries e{};

			e.loottable_id = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.lootdrop_id  = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.multiplier   = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 1;
			e.droplimit    = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.mindrop      = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.probability  = row[5] ? strtof(row[5], nullptr) : 100;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int loottable_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				loottable_entries_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const LoottableEntries &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.loottable_id));
		v.push_back(columns[1] + " = " + std::to_string(e.lootdrop_id));
		v.push_back(columns[2] + " = " + std::to_string(e.multiplier));
		v.push_back(columns[3] + " = " + std::to_string(e.droplimit));
		v.push_back(columns[4] + " = " + std::to_string(e.mindrop));
		v.push_back(columns[5] + " = " + std::to_string(e.probability));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.loottable_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static LoottableEntries InsertOne(
		Database& db,
		LoottableEntries e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.loottable_id));
		v.push_back(std::to_string(e.lootdrop_id));
		v.push_back(std::to_string(e.multiplier));
		v.push_back(std::to_string(e.droplimit));
		v.push_back(std::to_string(e.mindrop));
		v.push_back(std::to_string(e.probability));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.loottable_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<LoottableEntries> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.loottable_id));
			v.push_back(std::to_string(e.lootdrop_id));
			v.push_back(std::to_string(e.multiplier));
			v.push_back(std::to_string(e.droplimit));
			v.push_back(std::to_string(e.mindrop));
			v.push_back(std::to_string(e.probability));

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

	static std::vector<LoottableEntries> All(Database& db)
	{
		std::vector<LoottableEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LoottableEntries e{};

			e.loottable_id = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.lootdrop_id  = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.multiplier   = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 1;
			e.droplimit    = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.mindrop      = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.probability  = row[5] ? strtof(row[5], nullptr) : 100;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<LoottableEntries> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<LoottableEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LoottableEntries e{};

			e.loottable_id = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.lootdrop_id  = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.multiplier   = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 1;
			e.droplimit    = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.mindrop      = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.probability  = row[5] ? strtof(row[5], nullptr) : 100;

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
		const LoottableEntries &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.loottable_id));
		v.push_back(std::to_string(e.lootdrop_id));
		v.push_back(std::to_string(e.multiplier));
		v.push_back(std::to_string(e.droplimit));
		v.push_back(std::to_string(e.mindrop));
		v.push_back(std::to_string(e.probability));

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
		const std::vector<LoottableEntries> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.loottable_id));
			v.push_back(std::to_string(e.lootdrop_id));
			v.push_back(std::to_string(e.multiplier));
			v.push_back(std::to_string(e.droplimit));
			v.push_back(std::to_string(e.mindrop));
			v.push_back(std::to_string(e.probability));

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
