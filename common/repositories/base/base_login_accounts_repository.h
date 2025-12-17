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

class BaseLoginAccountsRepository {
public:
	struct LoginAccounts {
		uint32_t    id;
		std::string account_name;
		std::string account_password;
		std::string account_email;
		std::string source_loginserver;
		std::string last_ip_address;
		time_t      last_login_date;
		time_t      created_at;
		time_t      updated_at;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"account_name",
			"account_password",
			"account_email",
			"source_loginserver",
			"last_ip_address",
			"last_login_date",
			"created_at",
			"updated_at",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"account_name",
			"account_password",
			"account_email",
			"source_loginserver",
			"last_ip_address",
			"UNIX_TIMESTAMP(last_login_date)",
			"UNIX_TIMESTAMP(created_at)",
			"UNIX_TIMESTAMP(updated_at)",
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
		return std::string("login_accounts");
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

	static LoginAccounts NewEntity()
	{
		LoginAccounts e{};

		e.id                 = 0;
		e.account_name       = "";
		e.account_password   = "";
		e.account_email      = "";
		e.source_loginserver = "";
		e.last_ip_address    = "";
		e.last_login_date    = 0;
		e.created_at         = 0;
		e.updated_at         = std::time(nullptr);

		return e;
	}

	static LoginAccounts GetLoginAccounts(
		const std::vector<LoginAccounts> &login_accountss,
		int login_accounts_id
	)
	{
		for (auto &login_accounts : login_accountss) {
			if (login_accounts.id == login_accounts_id) {
				return login_accounts;
			}
		}

		return NewEntity();
	}

	static LoginAccounts FindOne(
		Database& db,
		int login_accounts_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				login_accounts_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			LoginAccounts e{};

			e.id                 = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.account_name       = row[1] ? row[1] : "";
			e.account_password   = row[2] ? row[2] : "";
			e.account_email      = row[3] ? row[3] : "";
			e.source_loginserver = row[4] ? row[4] : "";
			e.last_ip_address    = row[5] ? row[5] : "";
			e.last_login_date    = strtoll(row[6] ? row[6] : "-1", nullptr, 10);
			e.created_at         = strtoll(row[7] ? row[7] : "-1", nullptr, 10);
			e.updated_at         = strtoll(row[8] ? row[8] : "-1", nullptr, 10);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int login_accounts_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				login_accounts_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const LoginAccounts &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.id));
		v.push_back(columns[1] + " = '" + Strings::Escape(e.account_name) + "'");
		v.push_back(columns[2] + " = '" + Strings::Escape(e.account_password) + "'");
		v.push_back(columns[3] + " = '" + Strings::Escape(e.account_email) + "'");
		v.push_back(columns[4] + " = '" + Strings::Escape(e.source_loginserver) + "'");
		v.push_back(columns[5] + " = '" + Strings::Escape(e.last_ip_address) + "'");
		v.push_back(columns[6] + " = FROM_UNIXTIME(" + (e.last_login_date > 0 ? std::to_string(e.last_login_date) : "null") + ")");
		v.push_back(columns[7] + " = FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");
		v.push_back(columns[8] + " = FROM_UNIXTIME(" + (e.updated_at > 0 ? std::to_string(e.updated_at) : "null") + ")");

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

	static LoginAccounts InsertOne(
		Database& db,
		LoginAccounts e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.account_name) + "'");
		v.push_back("'" + Strings::Escape(e.account_password) + "'");
		v.push_back("'" + Strings::Escape(e.account_email) + "'");
		v.push_back("'" + Strings::Escape(e.source_loginserver) + "'");
		v.push_back("'" + Strings::Escape(e.last_ip_address) + "'");
		v.push_back("FROM_UNIXTIME(" + (e.last_login_date > 0 ? std::to_string(e.last_login_date) : "null") + ")");
		v.push_back("FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");
		v.push_back("FROM_UNIXTIME(" + (e.updated_at > 0 ? std::to_string(e.updated_at) : "null") + ")");

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
		const std::vector<LoginAccounts> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.account_name) + "'");
			v.push_back("'" + Strings::Escape(e.account_password) + "'");
			v.push_back("'" + Strings::Escape(e.account_email) + "'");
			v.push_back("'" + Strings::Escape(e.source_loginserver) + "'");
			v.push_back("'" + Strings::Escape(e.last_ip_address) + "'");
			v.push_back("FROM_UNIXTIME(" + (e.last_login_date > 0 ? std::to_string(e.last_login_date) : "null") + ")");
			v.push_back("FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");
			v.push_back("FROM_UNIXTIME(" + (e.updated_at > 0 ? std::to_string(e.updated_at) : "null") + ")");

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

	static std::vector<LoginAccounts> All(Database& db)
	{
		std::vector<LoginAccounts> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LoginAccounts e{};

			e.id                 = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.account_name       = row[1] ? row[1] : "";
			e.account_password   = row[2] ? row[2] : "";
			e.account_email      = row[3] ? row[3] : "";
			e.source_loginserver = row[4] ? row[4] : "";
			e.last_ip_address    = row[5] ? row[5] : "";
			e.last_login_date    = strtoll(row[6] ? row[6] : "-1", nullptr, 10);
			e.created_at         = strtoll(row[7] ? row[7] : "-1", nullptr, 10);
			e.updated_at         = strtoll(row[8] ? row[8] : "-1", nullptr, 10);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<LoginAccounts> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<LoginAccounts> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LoginAccounts e{};

			e.id                 = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.account_name       = row[1] ? row[1] : "";
			e.account_password   = row[2] ? row[2] : "";
			e.account_email      = row[3] ? row[3] : "";
			e.source_loginserver = row[4] ? row[4] : "";
			e.last_ip_address    = row[5] ? row[5] : "";
			e.last_login_date    = strtoll(row[6] ? row[6] : "-1", nullptr, 10);
			e.created_at         = strtoll(row[7] ? row[7] : "-1", nullptr, 10);
			e.updated_at         = strtoll(row[8] ? row[8] : "-1", nullptr, 10);

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
		const LoginAccounts &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.account_name) + "'");
		v.push_back("'" + Strings::Escape(e.account_password) + "'");
		v.push_back("'" + Strings::Escape(e.account_email) + "'");
		v.push_back("'" + Strings::Escape(e.source_loginserver) + "'");
		v.push_back("'" + Strings::Escape(e.last_ip_address) + "'");
		v.push_back("FROM_UNIXTIME(" + (e.last_login_date > 0 ? std::to_string(e.last_login_date) : "null") + ")");
		v.push_back("FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");
		v.push_back("FROM_UNIXTIME(" + (e.updated_at > 0 ? std::to_string(e.updated_at) : "null") + ")");

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
		const std::vector<LoginAccounts> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.account_name) + "'");
			v.push_back("'" + Strings::Escape(e.account_password) + "'");
			v.push_back("'" + Strings::Escape(e.account_email) + "'");
			v.push_back("'" + Strings::Escape(e.source_loginserver) + "'");
			v.push_back("'" + Strings::Escape(e.last_ip_address) + "'");
			v.push_back("FROM_UNIXTIME(" + (e.last_login_date > 0 ? std::to_string(e.last_login_date) : "null") + ")");
			v.push_back("FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");
			v.push_back("FROM_UNIXTIME(" + (e.updated_at > 0 ? std::to_string(e.updated_at) : "null") + ")");

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
