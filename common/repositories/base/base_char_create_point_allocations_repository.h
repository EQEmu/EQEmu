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

class BaseCharCreatePointAllocationsRepository {
public:
	struct CharCreatePointAllocations {
		uint32_t id;
		uint32_t base_str;
		uint32_t base_sta;
		uint32_t base_dex;
		uint32_t base_agi;
		uint32_t base_int;
		uint32_t base_wis;
		uint32_t base_cha;
		uint32_t alloc_str;
		uint32_t alloc_sta;
		uint32_t alloc_dex;
		uint32_t alloc_agi;
		uint32_t alloc_int;
		uint32_t alloc_wis;
		uint32_t alloc_cha;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"base_str",
			"base_sta",
			"base_dex",
			"base_agi",
			"base_int",
			"base_wis",
			"base_cha",
			"alloc_str",
			"alloc_sta",
			"alloc_dex",
			"alloc_agi",
			"alloc_int",
			"alloc_wis",
			"alloc_cha",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"base_str",
			"base_sta",
			"base_dex",
			"base_agi",
			"base_int",
			"base_wis",
			"base_cha",
			"alloc_str",
			"alloc_sta",
			"alloc_dex",
			"alloc_agi",
			"alloc_int",
			"alloc_wis",
			"alloc_cha",
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
		return std::string("char_create_point_allocations");
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

	static CharCreatePointAllocations NewEntity()
	{
		CharCreatePointAllocations e{};

		e.id        = 0;
		e.base_str  = 0;
		e.base_sta  = 0;
		e.base_dex  = 0;
		e.base_agi  = 0;
		e.base_int  = 0;
		e.base_wis  = 0;
		e.base_cha  = 0;
		e.alloc_str = 0;
		e.alloc_sta = 0;
		e.alloc_dex = 0;
		e.alloc_agi = 0;
		e.alloc_int = 0;
		e.alloc_wis = 0;
		e.alloc_cha = 0;

		return e;
	}

	static CharCreatePointAllocations GetCharCreatePointAllocations(
		const std::vector<CharCreatePointAllocations> &char_create_point_allocationss,
		int char_create_point_allocations_id
	)
	{
		for (auto &char_create_point_allocations : char_create_point_allocationss) {
			if (char_create_point_allocations.id == char_create_point_allocations_id) {
				return char_create_point_allocations;
			}
		}

		return NewEntity();
	}

	static CharCreatePointAllocations FindOne(
		Database& db,
		int char_create_point_allocations_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				char_create_point_allocations_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharCreatePointAllocations e{};

			e.id        = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.base_str  = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.base_sta  = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.base_dex  = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.base_agi  = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.base_int  = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.base_wis  = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.base_cha  = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.alloc_str = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.alloc_sta = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.alloc_dex = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.alloc_agi = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.alloc_int = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.alloc_wis = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.alloc_cha = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int char_create_point_allocations_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				char_create_point_allocations_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CharCreatePointAllocations &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.id));
		v.push_back(columns[1] + " = " + std::to_string(e.base_str));
		v.push_back(columns[2] + " = " + std::to_string(e.base_sta));
		v.push_back(columns[3] + " = " + std::to_string(e.base_dex));
		v.push_back(columns[4] + " = " + std::to_string(e.base_agi));
		v.push_back(columns[5] + " = " + std::to_string(e.base_int));
		v.push_back(columns[6] + " = " + std::to_string(e.base_wis));
		v.push_back(columns[7] + " = " + std::to_string(e.base_cha));
		v.push_back(columns[8] + " = " + std::to_string(e.alloc_str));
		v.push_back(columns[9] + " = " + std::to_string(e.alloc_sta));
		v.push_back(columns[10] + " = " + std::to_string(e.alloc_dex));
		v.push_back(columns[11] + " = " + std::to_string(e.alloc_agi));
		v.push_back(columns[12] + " = " + std::to_string(e.alloc_int));
		v.push_back(columns[13] + " = " + std::to_string(e.alloc_wis));
		v.push_back(columns[14] + " = " + std::to_string(e.alloc_cha));

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

	static CharCreatePointAllocations InsertOne(
		Database& db,
		CharCreatePointAllocations e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.base_str));
		v.push_back(std::to_string(e.base_sta));
		v.push_back(std::to_string(e.base_dex));
		v.push_back(std::to_string(e.base_agi));
		v.push_back(std::to_string(e.base_int));
		v.push_back(std::to_string(e.base_wis));
		v.push_back(std::to_string(e.base_cha));
		v.push_back(std::to_string(e.alloc_str));
		v.push_back(std::to_string(e.alloc_sta));
		v.push_back(std::to_string(e.alloc_dex));
		v.push_back(std::to_string(e.alloc_agi));
		v.push_back(std::to_string(e.alloc_int));
		v.push_back(std::to_string(e.alloc_wis));
		v.push_back(std::to_string(e.alloc_cha));

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
		const std::vector<CharCreatePointAllocations> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.base_str));
			v.push_back(std::to_string(e.base_sta));
			v.push_back(std::to_string(e.base_dex));
			v.push_back(std::to_string(e.base_agi));
			v.push_back(std::to_string(e.base_int));
			v.push_back(std::to_string(e.base_wis));
			v.push_back(std::to_string(e.base_cha));
			v.push_back(std::to_string(e.alloc_str));
			v.push_back(std::to_string(e.alloc_sta));
			v.push_back(std::to_string(e.alloc_dex));
			v.push_back(std::to_string(e.alloc_agi));
			v.push_back(std::to_string(e.alloc_int));
			v.push_back(std::to_string(e.alloc_wis));
			v.push_back(std::to_string(e.alloc_cha));

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

	static std::vector<CharCreatePointAllocations> All(Database& db)
	{
		std::vector<CharCreatePointAllocations> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharCreatePointAllocations e{};

			e.id        = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.base_str  = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.base_sta  = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.base_dex  = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.base_agi  = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.base_int  = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.base_wis  = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.base_cha  = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.alloc_str = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.alloc_sta = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.alloc_dex = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.alloc_agi = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.alloc_int = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.alloc_wis = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.alloc_cha = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CharCreatePointAllocations> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CharCreatePointAllocations> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharCreatePointAllocations e{};

			e.id        = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.base_str  = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.base_sta  = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.base_dex  = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.base_agi  = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.base_int  = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.base_wis  = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.base_cha  = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.alloc_str = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.alloc_sta = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.alloc_dex = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.alloc_agi = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.alloc_int = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.alloc_wis = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.alloc_cha = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 0;

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
		const CharCreatePointAllocations &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.base_str));
		v.push_back(std::to_string(e.base_sta));
		v.push_back(std::to_string(e.base_dex));
		v.push_back(std::to_string(e.base_agi));
		v.push_back(std::to_string(e.base_int));
		v.push_back(std::to_string(e.base_wis));
		v.push_back(std::to_string(e.base_cha));
		v.push_back(std::to_string(e.alloc_str));
		v.push_back(std::to_string(e.alloc_sta));
		v.push_back(std::to_string(e.alloc_dex));
		v.push_back(std::to_string(e.alloc_agi));
		v.push_back(std::to_string(e.alloc_int));
		v.push_back(std::to_string(e.alloc_wis));
		v.push_back(std::to_string(e.alloc_cha));

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
		const std::vector<CharCreatePointAllocations> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.base_str));
			v.push_back(std::to_string(e.base_sta));
			v.push_back(std::to_string(e.base_dex));
			v.push_back(std::to_string(e.base_agi));
			v.push_back(std::to_string(e.base_int));
			v.push_back(std::to_string(e.base_wis));
			v.push_back(std::to_string(e.base_cha));
			v.push_back(std::to_string(e.alloc_str));
			v.push_back(std::to_string(e.alloc_sta));
			v.push_back(std::to_string(e.alloc_dex));
			v.push_back(std::to_string(e.alloc_agi));
			v.push_back(std::to_string(e.alloc_int));
			v.push_back(std::to_string(e.alloc_wis));
			v.push_back(std::to_string(e.alloc_cha));

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
