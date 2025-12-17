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

class BaseLoginWorldServersRepository {
public:
	struct LoginWorldServers {
		uint32_t    id;
		std::string long_name;
		std::string short_name;
		std::string tag_description;
		int32_t     login_server_list_type_id;
		time_t      last_login_date;
		std::string last_ip_address;
		int32_t     login_server_admin_id;
		int32_t     is_server_trusted;
		std::string note;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"long_name",
			"short_name",
			"tag_description",
			"login_server_list_type_id",
			"last_login_date",
			"last_ip_address",
			"login_server_admin_id",
			"is_server_trusted",
			"note",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"long_name",
			"short_name",
			"tag_description",
			"login_server_list_type_id",
			"UNIX_TIMESTAMP(last_login_date)",
			"last_ip_address",
			"login_server_admin_id",
			"is_server_trusted",
			"note",
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
		return std::string("login_world_servers");
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

	static LoginWorldServers NewEntity()
	{
		LoginWorldServers e{};

		e.id                        = 0;
		e.long_name                 = "";
		e.short_name                = "";
		e.tag_description           = "";
		e.login_server_list_type_id = 0;
		e.last_login_date           = 0;
		e.last_ip_address           = "";
		e.login_server_admin_id     = 0;
		e.is_server_trusted         = 0;
		e.note                      = "";

		return e;
	}

	static LoginWorldServers GetLoginWorldServers(
		const std::vector<LoginWorldServers> &login_world_serverss,
		int login_world_servers_id
	)
	{
		for (auto &login_world_servers : login_world_serverss) {
			if (login_world_servers.id == login_world_servers_id) {
				return login_world_servers;
			}
		}

		return NewEntity();
	}

	static LoginWorldServers FindOne(
		Database& db,
		int login_world_servers_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				login_world_servers_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			LoginWorldServers e{};

			e.id                        = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.long_name                 = row[1] ? row[1] : "";
			e.short_name                = row[2] ? row[2] : "";
			e.tag_description           = row[3] ? row[3] : "";
			e.login_server_list_type_id = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.last_login_date           = strtoll(row[5] ? row[5] : "-1", nullptr, 10);
			e.last_ip_address           = row[6] ? row[6] : "";
			e.login_server_admin_id     = row[7] ? static_cast<int32_t>(atoi(row[7])) : 0;
			e.is_server_trusted         = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.note                      = row[9] ? row[9] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int login_world_servers_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				login_world_servers_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const LoginWorldServers &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.long_name) + "'");
		v.push_back(columns[2] + " = '" + Strings::Escape(e.short_name) + "'");
		v.push_back(columns[3] + " = '" + Strings::Escape(e.tag_description) + "'");
		v.push_back(columns[4] + " = " + std::to_string(e.login_server_list_type_id));
		v.push_back(columns[5] + " = FROM_UNIXTIME(" + (e.last_login_date > 0 ? std::to_string(e.last_login_date) : "null") + ")");
		v.push_back(columns[6] + " = '" + Strings::Escape(e.last_ip_address) + "'");
		v.push_back(columns[7] + " = " + std::to_string(e.login_server_admin_id));
		v.push_back(columns[8] + " = " + std::to_string(e.is_server_trusted));
		v.push_back(columns[9] + " = '" + Strings::Escape(e.note) + "'");

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

	static LoginWorldServers InsertOne(
		Database& db,
		LoginWorldServers e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.long_name) + "'");
		v.push_back("'" + Strings::Escape(e.short_name) + "'");
		v.push_back("'" + Strings::Escape(e.tag_description) + "'");
		v.push_back(std::to_string(e.login_server_list_type_id));
		v.push_back("FROM_UNIXTIME(" + (e.last_login_date > 0 ? std::to_string(e.last_login_date) : "null") + ")");
		v.push_back("'" + Strings::Escape(e.last_ip_address) + "'");
		v.push_back(std::to_string(e.login_server_admin_id));
		v.push_back(std::to_string(e.is_server_trusted));
		v.push_back("'" + Strings::Escape(e.note) + "'");

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
		const std::vector<LoginWorldServers> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.long_name) + "'");
			v.push_back("'" + Strings::Escape(e.short_name) + "'");
			v.push_back("'" + Strings::Escape(e.tag_description) + "'");
			v.push_back(std::to_string(e.login_server_list_type_id));
			v.push_back("FROM_UNIXTIME(" + (e.last_login_date > 0 ? std::to_string(e.last_login_date) : "null") + ")");
			v.push_back("'" + Strings::Escape(e.last_ip_address) + "'");
			v.push_back(std::to_string(e.login_server_admin_id));
			v.push_back(std::to_string(e.is_server_trusted));
			v.push_back("'" + Strings::Escape(e.note) + "'");

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

	static std::vector<LoginWorldServers> All(Database& db)
	{
		std::vector<LoginWorldServers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LoginWorldServers e{};

			e.id                        = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.long_name                 = row[1] ? row[1] : "";
			e.short_name                = row[2] ? row[2] : "";
			e.tag_description           = row[3] ? row[3] : "";
			e.login_server_list_type_id = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.last_login_date           = strtoll(row[5] ? row[5] : "-1", nullptr, 10);
			e.last_ip_address           = row[6] ? row[6] : "";
			e.login_server_admin_id     = row[7] ? static_cast<int32_t>(atoi(row[7])) : 0;
			e.is_server_trusted         = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.note                      = row[9] ? row[9] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<LoginWorldServers> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<LoginWorldServers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LoginWorldServers e{};

			e.id                        = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.long_name                 = row[1] ? row[1] : "";
			e.short_name                = row[2] ? row[2] : "";
			e.tag_description           = row[3] ? row[3] : "";
			e.login_server_list_type_id = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.last_login_date           = strtoll(row[5] ? row[5] : "-1", nullptr, 10);
			e.last_ip_address           = row[6] ? row[6] : "";
			e.login_server_admin_id     = row[7] ? static_cast<int32_t>(atoi(row[7])) : 0;
			e.is_server_trusted         = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.note                      = row[9] ? row[9] : "";

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
		const LoginWorldServers &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.long_name) + "'");
		v.push_back("'" + Strings::Escape(e.short_name) + "'");
		v.push_back("'" + Strings::Escape(e.tag_description) + "'");
		v.push_back(std::to_string(e.login_server_list_type_id));
		v.push_back("FROM_UNIXTIME(" + (e.last_login_date > 0 ? std::to_string(e.last_login_date) : "null") + ")");
		v.push_back("'" + Strings::Escape(e.last_ip_address) + "'");
		v.push_back(std::to_string(e.login_server_admin_id));
		v.push_back(std::to_string(e.is_server_trusted));
		v.push_back("'" + Strings::Escape(e.note) + "'");

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
		const std::vector<LoginWorldServers> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.long_name) + "'");
			v.push_back("'" + Strings::Escape(e.short_name) + "'");
			v.push_back("'" + Strings::Escape(e.tag_description) + "'");
			v.push_back(std::to_string(e.login_server_list_type_id));
			v.push_back("FROM_UNIXTIME(" + (e.last_login_date > 0 ? std::to_string(e.last_login_date) : "null") + ")");
			v.push_back("'" + Strings::Escape(e.last_ip_address) + "'");
			v.push_back(std::to_string(e.login_server_admin_id));
			v.push_back(std::to_string(e.is_server_trusted));
			v.push_back("'" + Strings::Escape(e.note) + "'");

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
