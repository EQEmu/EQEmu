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

class BaseBuyerTradeItemsRepository {
public:
	struct BuyerTradeItems {
		uint64_t    id;
		uint64_t    buyer_buy_lines_id;
		int32_t     item_id;
		int32_t     item_qty;
		int32_t     item_icon;
		std::string item_name;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"buyer_buy_lines_id",
			"item_id",
			"item_qty",
			"item_icon",
			"item_name",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"buyer_buy_lines_id",
			"item_id",
			"item_qty",
			"item_icon",
			"item_name",
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
		return std::string("buyer_trade_items");
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

	static BuyerTradeItems NewEntity()
	{
		BuyerTradeItems e{};

		e.id                 = 0;
		e.buyer_buy_lines_id = 0;
		e.item_id            = 0;
		e.item_qty           = 0;
		e.item_icon          = 0;
		e.item_name          = "0";

		return e;
	}

	static BuyerTradeItems GetBuyerTradeItems(
		const std::vector<BuyerTradeItems> &buyer_trade_itemss,
		int buyer_trade_items_id
	)
	{
		for (auto &buyer_trade_items : buyer_trade_itemss) {
			if (buyer_trade_items.id == buyer_trade_items_id) {
				return buyer_trade_items;
			}
		}

		return NewEntity();
	}

	static BuyerTradeItems FindOne(
		Database& db,
		int buyer_trade_items_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				buyer_trade_items_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			BuyerTradeItems e{};

			e.id                 = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.buyer_buy_lines_id = row[1] ? strtoull(row[1], nullptr, 10) : 0;
			e.item_id            = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.item_qty           = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.item_icon          = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.item_name          = row[5] ? row[5] : "0";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int buyer_trade_items_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				buyer_trade_items_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const BuyerTradeItems &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.buyer_buy_lines_id));
		v.push_back(columns[2] + " = " + std::to_string(e.item_id));
		v.push_back(columns[3] + " = " + std::to_string(e.item_qty));
		v.push_back(columns[4] + " = " + std::to_string(e.item_icon));
		v.push_back(columns[5] + " = '" + Strings::Escape(e.item_name) + "'");

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

	static BuyerTradeItems InsertOne(
		Database& db,
		BuyerTradeItems e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.buyer_buy_lines_id));
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.item_qty));
		v.push_back(std::to_string(e.item_icon));
		v.push_back("'" + Strings::Escape(e.item_name) + "'");

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
		const std::vector<BuyerTradeItems> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.buyer_buy_lines_id));
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.item_qty));
			v.push_back(std::to_string(e.item_icon));
			v.push_back("'" + Strings::Escape(e.item_name) + "'");

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

	static std::vector<BuyerTradeItems> All(Database& db)
	{
		std::vector<BuyerTradeItems> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BuyerTradeItems e{};

			e.id                 = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.buyer_buy_lines_id = row[1] ? strtoull(row[1], nullptr, 10) : 0;
			e.item_id            = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.item_qty           = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.item_icon          = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.item_name          = row[5] ? row[5] : "0";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<BuyerTradeItems> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<BuyerTradeItems> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BuyerTradeItems e{};

			e.id                 = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.buyer_buy_lines_id = row[1] ? strtoull(row[1], nullptr, 10) : 0;
			e.item_id            = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.item_qty           = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.item_icon          = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.item_name          = row[5] ? row[5] : "0";

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
		const BuyerTradeItems &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.buyer_buy_lines_id));
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.item_qty));
		v.push_back(std::to_string(e.item_icon));
		v.push_back("'" + Strings::Escape(e.item_name) + "'");

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
		const std::vector<BuyerTradeItems> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.buyer_buy_lines_id));
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.item_qty));
			v.push_back(std::to_string(e.item_icon));
			v.push_back("'" + Strings::Escape(e.item_name) + "'");

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
