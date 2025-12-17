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

class BaseAccountIpRepository {
public:
	struct AccountIp {
		int32_t     accid;
		std::string ip;
		int32_t     count;
		time_t      lastused;
	};

	static std::string PrimaryKey()
	{
		return std::string("accid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"accid",
			"ip",
			"count",
			"lastused",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"accid",
			"ip",
			"count",
			"UNIX_TIMESTAMP(lastused)",
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
		return std::string("account_ip");
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

	static AccountIp NewEntity()
	{
		AccountIp e{};

		e.accid    = 0;
		e.ip       = "";
		e.count    = 1;
		e.lastused = std::time(nullptr);

		return e;
	}

	static AccountIp GetAccountIp(
		const std::vector<AccountIp> &account_ips,
		int account_ip_id
	)
	{
		for (auto &account_ip : account_ips) {
			if (account_ip.accid == account_ip_id) {
				return account_ip;
			}
		}

		return NewEntity();
	}

	static AccountIp FindOne(
		Database& db,
		int account_ip_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				account_ip_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			AccountIp e{};

			e.accid    = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.ip       = row[1] ? row[1] : "";
			e.count    = row[2] ? static_cast<int32_t>(atoi(row[2])) : 1;
			e.lastused = strtoll(row[3] ? row[3] : "-1", nullptr, 10);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int account_ip_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				account_ip_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const AccountIp &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.accid));
		v.push_back(columns[1] + " = '" + Strings::Escape(e.ip) + "'");
		v.push_back(columns[2] + " = " + std::to_string(e.count));
		v.push_back(columns[3] + " = FROM_UNIXTIME(" + (e.lastused > 0 ? std::to_string(e.lastused) : "null") + ")");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.accid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static AccountIp InsertOne(
		Database& db,
		AccountIp e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.accid));
		v.push_back("'" + Strings::Escape(e.ip) + "'");
		v.push_back(std::to_string(e.count));
		v.push_back("FROM_UNIXTIME(" + (e.lastused > 0 ? std::to_string(e.lastused) : "null") + ")");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.accid = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<AccountIp> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.accid));
			v.push_back("'" + Strings::Escape(e.ip) + "'");
			v.push_back(std::to_string(e.count));
			v.push_back("FROM_UNIXTIME(" + (e.lastused > 0 ? std::to_string(e.lastused) : "null") + ")");

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

	static std::vector<AccountIp> All(Database& db)
	{
		std::vector<AccountIp> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AccountIp e{};

			e.accid    = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.ip       = row[1] ? row[1] : "";
			e.count    = row[2] ? static_cast<int32_t>(atoi(row[2])) : 1;
			e.lastused = strtoll(row[3] ? row[3] : "-1", nullptr, 10);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<AccountIp> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<AccountIp> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AccountIp e{};

			e.accid    = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.ip       = row[1] ? row[1] : "";
			e.count    = row[2] ? static_cast<int32_t>(atoi(row[2])) : 1;
			e.lastused = strtoll(row[3] ? row[3] : "-1", nullptr, 10);

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
		const AccountIp &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.accid));
		v.push_back("'" + Strings::Escape(e.ip) + "'");
		v.push_back(std::to_string(e.count));
		v.push_back("FROM_UNIXTIME(" + (e.lastused > 0 ? std::to_string(e.lastused) : "null") + ")");

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
		const std::vector<AccountIp> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.accid));
			v.push_back("'" + Strings::Escape(e.ip) + "'");
			v.push_back(std::to_string(e.count));
			v.push_back("FROM_UNIXTIME(" + (e.lastused > 0 ? std::to_string(e.lastused) : "null") + ")");

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
