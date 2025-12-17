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

class BaseBuyerRepository {
public:
	struct Buyer {
		uint64_t    id;
		uint32_t    char_id;
		uint32_t    char_entity_id;
		std::string char_name;
		uint32_t    char_zone_id;
		uint32_t    char_zone_instance_id;
		time_t      transaction_date;
		std::string welcome_message;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"char_id",
			"char_entity_id",
			"char_name",
			"char_zone_id",
			"char_zone_instance_id",
			"transaction_date",
			"welcome_message",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"char_id",
			"char_entity_id",
			"char_name",
			"char_zone_id",
			"char_zone_instance_id",
			"UNIX_TIMESTAMP(transaction_date)",
			"welcome_message",
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
		return std::string("buyer");
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

	static Buyer NewEntity()
	{
		Buyer e{};

		e.id                    = 0;
		e.char_id               = 0;
		e.char_entity_id        = 0;
		e.char_name             = "";
		e.char_zone_id          = 0;
		e.char_zone_instance_id = 0;
		e.transaction_date      = 0;
		e.welcome_message       = "";

		return e;
	}

	static Buyer GetBuyer(
		const std::vector<Buyer> &buyers,
		int buyer_id
	)
	{
		for (auto &buyer : buyers) {
			if (buyer.id == buyer_id) {
				return buyer;
			}
		}

		return NewEntity();
	}

	static Buyer FindOne(
		Database& db,
		int buyer_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				buyer_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Buyer e{};

			e.id                    = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.char_id               = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.char_entity_id        = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.char_name             = row[3] ? row[3] : "";
			e.char_zone_id          = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.char_zone_instance_id = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.transaction_date      = strtoll(row[6] ? row[6] : "-1", nullptr, 10);
			e.welcome_message       = row[7] ? row[7] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int buyer_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				buyer_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Buyer &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.char_id));
		v.push_back(columns[2] + " = " + std::to_string(e.char_entity_id));
		v.push_back(columns[3] + " = '" + Strings::Escape(e.char_name) + "'");
		v.push_back(columns[4] + " = " + std::to_string(e.char_zone_id));
		v.push_back(columns[5] + " = " + std::to_string(e.char_zone_instance_id));
		v.push_back(columns[6] + " = FROM_UNIXTIME(" + (e.transaction_date > 0 ? std::to_string(e.transaction_date) : "null") + ")");
		v.push_back(columns[7] + " = '" + Strings::Escape(e.welcome_message) + "'");

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

	static Buyer InsertOne(
		Database& db,
		Buyer e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.char_id));
		v.push_back(std::to_string(e.char_entity_id));
		v.push_back("'" + Strings::Escape(e.char_name) + "'");
		v.push_back(std::to_string(e.char_zone_id));
		v.push_back(std::to_string(e.char_zone_instance_id));
		v.push_back("FROM_UNIXTIME(" + (e.transaction_date > 0 ? std::to_string(e.transaction_date) : "null") + ")");
		v.push_back("'" + Strings::Escape(e.welcome_message) + "'");

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
		const std::vector<Buyer> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.char_id));
			v.push_back(std::to_string(e.char_entity_id));
			v.push_back("'" + Strings::Escape(e.char_name) + "'");
			v.push_back(std::to_string(e.char_zone_id));
			v.push_back(std::to_string(e.char_zone_instance_id));
			v.push_back("FROM_UNIXTIME(" + (e.transaction_date > 0 ? std::to_string(e.transaction_date) : "null") + ")");
			v.push_back("'" + Strings::Escape(e.welcome_message) + "'");

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

	static std::vector<Buyer> All(Database& db)
	{
		std::vector<Buyer> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Buyer e{};

			e.id                    = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.char_id               = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.char_entity_id        = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.char_name             = row[3] ? row[3] : "";
			e.char_zone_id          = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.char_zone_instance_id = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.transaction_date      = strtoll(row[6] ? row[6] : "-1", nullptr, 10);
			e.welcome_message       = row[7] ? row[7] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Buyer> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Buyer> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Buyer e{};

			e.id                    = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.char_id               = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.char_entity_id        = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.char_name             = row[3] ? row[3] : "";
			e.char_zone_id          = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.char_zone_instance_id = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.transaction_date      = strtoll(row[6] ? row[6] : "-1", nullptr, 10);
			e.welcome_message       = row[7] ? row[7] : "";

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
		const Buyer &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.char_id));
		v.push_back(std::to_string(e.char_entity_id));
		v.push_back("'" + Strings::Escape(e.char_name) + "'");
		v.push_back(std::to_string(e.char_zone_id));
		v.push_back(std::to_string(e.char_zone_instance_id));
		v.push_back("FROM_UNIXTIME(" + (e.transaction_date > 0 ? std::to_string(e.transaction_date) : "null") + ")");
		v.push_back("'" + Strings::Escape(e.welcome_message) + "'");

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
		const std::vector<Buyer> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.char_id));
			v.push_back(std::to_string(e.char_entity_id));
			v.push_back("'" + Strings::Escape(e.char_name) + "'");
			v.push_back(std::to_string(e.char_zone_id));
			v.push_back(std::to_string(e.char_zone_instance_id));
			v.push_back("FROM_UNIXTIME(" + (e.transaction_date > 0 ? std::to_string(e.transaction_date) : "null") + ")");
			v.push_back("'" + Strings::Escape(e.welcome_message) + "'");

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
