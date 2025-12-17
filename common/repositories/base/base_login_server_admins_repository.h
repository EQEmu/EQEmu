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

class BaseLoginServerAdminsRepository {
public:
	struct LoginServerAdmins {
		uint32_t    id;
		std::string account_name;
		std::string account_password;
		std::string first_name;
		std::string last_name;
		std::string email;
		time_t      registration_date;
		std::string registration_ip_address;
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
			"first_name",
			"last_name",
			"email",
			"registration_date",
			"registration_ip_address",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"account_name",
			"account_password",
			"first_name",
			"last_name",
			"email",
			"UNIX_TIMESTAMP(registration_date)",
			"registration_ip_address",
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
		return std::string("login_server_admins");
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

	static LoginServerAdmins NewEntity()
	{
		LoginServerAdmins e{};

		e.id                      = 0;
		e.account_name            = "";
		e.account_password        = "";
		e.first_name              = "";
		e.last_name               = "";
		e.email                   = "";
		e.registration_date       = 0;
		e.registration_ip_address = "";

		return e;
	}

	static LoginServerAdmins GetLoginServerAdmins(
		const std::vector<LoginServerAdmins> &login_server_adminss,
		int login_server_admins_id
	)
	{
		for (auto &login_server_admins : login_server_adminss) {
			if (login_server_admins.id == login_server_admins_id) {
				return login_server_admins;
			}
		}

		return NewEntity();
	}

	static LoginServerAdmins FindOne(
		Database& db,
		int login_server_admins_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				login_server_admins_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			LoginServerAdmins e{};

			e.id                      = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.account_name            = row[1] ? row[1] : "";
			e.account_password        = row[2] ? row[2] : "";
			e.first_name              = row[3] ? row[3] : "";
			e.last_name               = row[4] ? row[4] : "";
			e.email                   = row[5] ? row[5] : "";
			e.registration_date       = strtoll(row[6] ? row[6] : "-1", nullptr, 10);
			e.registration_ip_address = row[7] ? row[7] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int login_server_admins_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				login_server_admins_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const LoginServerAdmins &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.account_name) + "'");
		v.push_back(columns[2] + " = '" + Strings::Escape(e.account_password) + "'");
		v.push_back(columns[3] + " = '" + Strings::Escape(e.first_name) + "'");
		v.push_back(columns[4] + " = '" + Strings::Escape(e.last_name) + "'");
		v.push_back(columns[5] + " = '" + Strings::Escape(e.email) + "'");
		v.push_back(columns[6] + " = FROM_UNIXTIME(" + (e.registration_date > 0 ? std::to_string(e.registration_date) : "null") + ")");
		v.push_back(columns[7] + " = '" + Strings::Escape(e.registration_ip_address) + "'");

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

	static LoginServerAdmins InsertOne(
		Database& db,
		LoginServerAdmins e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.account_name) + "'");
		v.push_back("'" + Strings::Escape(e.account_password) + "'");
		v.push_back("'" + Strings::Escape(e.first_name) + "'");
		v.push_back("'" + Strings::Escape(e.last_name) + "'");
		v.push_back("'" + Strings::Escape(e.email) + "'");
		v.push_back("FROM_UNIXTIME(" + (e.registration_date > 0 ? std::to_string(e.registration_date) : "null") + ")");
		v.push_back("'" + Strings::Escape(e.registration_ip_address) + "'");

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
		const std::vector<LoginServerAdmins> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.account_name) + "'");
			v.push_back("'" + Strings::Escape(e.account_password) + "'");
			v.push_back("'" + Strings::Escape(e.first_name) + "'");
			v.push_back("'" + Strings::Escape(e.last_name) + "'");
			v.push_back("'" + Strings::Escape(e.email) + "'");
			v.push_back("FROM_UNIXTIME(" + (e.registration_date > 0 ? std::to_string(e.registration_date) : "null") + ")");
			v.push_back("'" + Strings::Escape(e.registration_ip_address) + "'");

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

	static std::vector<LoginServerAdmins> All(Database& db)
	{
		std::vector<LoginServerAdmins> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LoginServerAdmins e{};

			e.id                      = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.account_name            = row[1] ? row[1] : "";
			e.account_password        = row[2] ? row[2] : "";
			e.first_name              = row[3] ? row[3] : "";
			e.last_name               = row[4] ? row[4] : "";
			e.email                   = row[5] ? row[5] : "";
			e.registration_date       = strtoll(row[6] ? row[6] : "-1", nullptr, 10);
			e.registration_ip_address = row[7] ? row[7] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<LoginServerAdmins> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<LoginServerAdmins> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LoginServerAdmins e{};

			e.id                      = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.account_name            = row[1] ? row[1] : "";
			e.account_password        = row[2] ? row[2] : "";
			e.first_name              = row[3] ? row[3] : "";
			e.last_name               = row[4] ? row[4] : "";
			e.email                   = row[5] ? row[5] : "";
			e.registration_date       = strtoll(row[6] ? row[6] : "-1", nullptr, 10);
			e.registration_ip_address = row[7] ? row[7] : "";

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
		const LoginServerAdmins &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.account_name) + "'");
		v.push_back("'" + Strings::Escape(e.account_password) + "'");
		v.push_back("'" + Strings::Escape(e.first_name) + "'");
		v.push_back("'" + Strings::Escape(e.last_name) + "'");
		v.push_back("'" + Strings::Escape(e.email) + "'");
		v.push_back("FROM_UNIXTIME(" + (e.registration_date > 0 ? std::to_string(e.registration_date) : "null") + ")");
		v.push_back("'" + Strings::Escape(e.registration_ip_address) + "'");

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
		const std::vector<LoginServerAdmins> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.account_name) + "'");
			v.push_back("'" + Strings::Escape(e.account_password) + "'");
			v.push_back("'" + Strings::Escape(e.first_name) + "'");
			v.push_back("'" + Strings::Escape(e.last_name) + "'");
			v.push_back("'" + Strings::Escape(e.email) + "'");
			v.push_back("FROM_UNIXTIME(" + (e.registration_date > 0 ? std::to_string(e.registration_date) : "null") + ")");
			v.push_back("'" + Strings::Escape(e.registration_ip_address) + "'");

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
