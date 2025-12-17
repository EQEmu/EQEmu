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

class BaseDiscoveredItemsRepository {
public:
	struct DiscoveredItems {
		uint32_t    item_id;
		std::string char_name;
		uint32_t    discovered_date;
		int32_t     account_status;
	};

	static std::string PrimaryKey()
	{
		return std::string("item_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"item_id",
			"char_name",
			"discovered_date",
			"account_status",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"item_id",
			"char_name",
			"discovered_date",
			"account_status",
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
		return std::string("discovered_items");
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

	static DiscoveredItems NewEntity()
	{
		DiscoveredItems e{};

		e.item_id         = 0;
		e.char_name       = "";
		e.discovered_date = 0;
		e.account_status  = 0;

		return e;
	}

	static DiscoveredItems GetDiscoveredItems(
		const std::vector<DiscoveredItems> &discovered_itemss,
		int discovered_items_id
	)
	{
		for (auto &discovered_items : discovered_itemss) {
			if (discovered_items.item_id == discovered_items_id) {
				return discovered_items;
			}
		}

		return NewEntity();
	}

	static DiscoveredItems FindOne(
		Database& db,
		int discovered_items_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				discovered_items_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			DiscoveredItems e{};

			e.item_id         = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.char_name       = row[1] ? row[1] : "";
			e.discovered_date = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.account_status  = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int discovered_items_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				discovered_items_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const DiscoveredItems &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.item_id));
		v.push_back(columns[1] + " = '" + Strings::Escape(e.char_name) + "'");
		v.push_back(columns[2] + " = " + std::to_string(e.discovered_date));
		v.push_back(columns[3] + " = " + std::to_string(e.account_status));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.item_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static DiscoveredItems InsertOne(
		Database& db,
		DiscoveredItems e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.item_id));
		v.push_back("'" + Strings::Escape(e.char_name) + "'");
		v.push_back(std::to_string(e.discovered_date));
		v.push_back(std::to_string(e.account_status));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.item_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<DiscoveredItems> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.item_id));
			v.push_back("'" + Strings::Escape(e.char_name) + "'");
			v.push_back(std::to_string(e.discovered_date));
			v.push_back(std::to_string(e.account_status));

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

	static std::vector<DiscoveredItems> All(Database& db)
	{
		std::vector<DiscoveredItems> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			DiscoveredItems e{};

			e.item_id         = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.char_name       = row[1] ? row[1] : "";
			e.discovered_date = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.account_status  = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<DiscoveredItems> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<DiscoveredItems> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			DiscoveredItems e{};

			e.item_id         = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.char_name       = row[1] ? row[1] : "";
			e.discovered_date = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.account_status  = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;

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
		const DiscoveredItems &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.item_id));
		v.push_back("'" + Strings::Escape(e.char_name) + "'");
		v.push_back(std::to_string(e.discovered_date));
		v.push_back(std::to_string(e.account_status));

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
		const std::vector<DiscoveredItems> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.item_id));
			v.push_back("'" + Strings::Escape(e.char_name) + "'");
			v.push_back(std::to_string(e.discovered_date));
			v.push_back(std::to_string(e.account_status));

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
