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


class BaseMercMerchantEntriesRepository {
public:
	struct MercMerchantEntries {
		uint32_t merc_merchant_entry_id;
		uint32_t merc_merchant_template_id;
		uint32_t merchant_id;
	};

	static std::string PrimaryKey()
	{
		return std::string("merc_merchant_entry_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"merc_merchant_entry_id",
			"merc_merchant_template_id",
			"merchant_id",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"merc_merchant_entry_id",
			"merc_merchant_template_id",
			"merchant_id",
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
		return std::string("merc_merchant_entries");
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

	static MercMerchantEntries NewEntity()
	{
		MercMerchantEntries e{};

		e.merc_merchant_entry_id    = 0;
		e.merc_merchant_template_id = 0;
		e.merchant_id               = 0;

		return e;
	}

	static MercMerchantEntries GetMercMerchantEntries(
		const std::vector<MercMerchantEntries> &merc_merchant_entriess,
		int merc_merchant_entries_id
	)
	{
		for (auto &merc_merchant_entries : merc_merchant_entriess) {
			if (merc_merchant_entries.merc_merchant_entry_id == merc_merchant_entries_id) {
				return merc_merchant_entries;
			}
		}

		return NewEntity();
	}

	static MercMerchantEntries FindOne(
		Database& db,
		int merc_merchant_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				merc_merchant_entries_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			MercMerchantEntries e{};

			e.merc_merchant_entry_id    = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.merc_merchant_template_id = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.merchant_id               = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int merc_merchant_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				merc_merchant_entries_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const MercMerchantEntries &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.merc_merchant_template_id));
		v.push_back(columns[2] + " = " + std::to_string(e.merchant_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.merc_merchant_entry_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static MercMerchantEntries InsertOne(
		Database& db,
		MercMerchantEntries e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.merc_merchant_entry_id));
		v.push_back(std::to_string(e.merc_merchant_template_id));
		v.push_back(std::to_string(e.merchant_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.merc_merchant_entry_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<MercMerchantEntries> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.merc_merchant_entry_id));
			v.push_back(std::to_string(e.merc_merchant_template_id));
			v.push_back(std::to_string(e.merchant_id));

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

	static std::vector<MercMerchantEntries> All(Database& db)
	{
		std::vector<MercMerchantEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MercMerchantEntries e{};

			e.merc_merchant_entry_id    = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.merc_merchant_template_id = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.merchant_id               = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<MercMerchantEntries> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<MercMerchantEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MercMerchantEntries e{};

			e.merc_merchant_entry_id    = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.merc_merchant_template_id = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.merchant_id               = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));

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
		const MercMerchantEntries &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.merc_merchant_entry_id));
		v.push_back(std::to_string(e.merc_merchant_template_id));
		v.push_back(std::to_string(e.merchant_id));

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
		const std::vector<MercMerchantEntries> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.merc_merchant_entry_id));
			v.push_back(std::to_string(e.merc_merchant_template_id));
			v.push_back(std::to_string(e.merchant_id));

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
