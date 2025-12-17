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

class BaseMerchantlistTempRepository {
public:
	struct MerchantlistTemp {
		uint32_t npcid;
		uint32_t slot;
		int32_t  zone_id;
		int32_t  instance_id;
		uint32_t itemid;
		uint32_t charges;
	};

	static std::string PrimaryKey()
	{
		return std::string("npcid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"npcid",
			"slot",
			"zone_id",
			"instance_id",
			"itemid",
			"charges",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"npcid",
			"slot",
			"zone_id",
			"instance_id",
			"itemid",
			"charges",
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
		return std::string("merchantlist_temp");
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

	static MerchantlistTemp NewEntity()
	{
		MerchantlistTemp e{};

		e.npcid       = 0;
		e.slot        = 0;
		e.zone_id     = 0;
		e.instance_id = 0;
		e.itemid      = 0;
		e.charges     = 1;

		return e;
	}

	static MerchantlistTemp GetMerchantlistTemp(
		const std::vector<MerchantlistTemp> &merchantlist_temps,
		int merchantlist_temp_id
	)
	{
		for (auto &merchantlist_temp : merchantlist_temps) {
			if (merchantlist_temp.npcid == merchantlist_temp_id) {
				return merchantlist_temp;
			}
		}

		return NewEntity();
	}

	static MerchantlistTemp FindOne(
		Database& db,
		int merchantlist_temp_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				merchantlist_temp_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			MerchantlistTemp e{};

			e.npcid       = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.slot        = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.zone_id     = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.instance_id = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.itemid      = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.charges     = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 1;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int merchantlist_temp_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				merchantlist_temp_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const MerchantlistTemp &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.npcid));
		v.push_back(columns[1] + " = " + std::to_string(e.slot));
		v.push_back(columns[2] + " = " + std::to_string(e.zone_id));
		v.push_back(columns[3] + " = " + std::to_string(e.instance_id));
		v.push_back(columns[4] + " = " + std::to_string(e.itemid));
		v.push_back(columns[5] + " = " + std::to_string(e.charges));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.npcid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static MerchantlistTemp InsertOne(
		Database& db,
		MerchantlistTemp e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.npcid));
		v.push_back(std::to_string(e.slot));
		v.push_back(std::to_string(e.zone_id));
		v.push_back(std::to_string(e.instance_id));
		v.push_back(std::to_string(e.itemid));
		v.push_back(std::to_string(e.charges));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.npcid = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<MerchantlistTemp> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.npcid));
			v.push_back(std::to_string(e.slot));
			v.push_back(std::to_string(e.zone_id));
			v.push_back(std::to_string(e.instance_id));
			v.push_back(std::to_string(e.itemid));
			v.push_back(std::to_string(e.charges));

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

	static std::vector<MerchantlistTemp> All(Database& db)
	{
		std::vector<MerchantlistTemp> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MerchantlistTemp e{};

			e.npcid       = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.slot        = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.zone_id     = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.instance_id = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.itemid      = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.charges     = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 1;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<MerchantlistTemp> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<MerchantlistTemp> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MerchantlistTemp e{};

			e.npcid       = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.slot        = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.zone_id     = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.instance_id = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.itemid      = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.charges     = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 1;

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
		const MerchantlistTemp &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.npcid));
		v.push_back(std::to_string(e.slot));
		v.push_back(std::to_string(e.zone_id));
		v.push_back(std::to_string(e.instance_id));
		v.push_back(std::to_string(e.itemid));
		v.push_back(std::to_string(e.charges));

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
		const std::vector<MerchantlistTemp> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.npcid));
			v.push_back(std::to_string(e.slot));
			v.push_back(std::to_string(e.zone_id));
			v.push_back(std::to_string(e.instance_id));
			v.push_back(std::to_string(e.itemid));
			v.push_back(std::to_string(e.charges));

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
