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

class BaseLoginApiTokensRepository {
public:
	struct LoginApiTokens {
		int32_t     id;
		std::string token;
		int32_t     can_write;
		int32_t     can_read;
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
			"token",
			"can_write",
			"can_read",
			"created_at",
			"updated_at",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"token",
			"can_write",
			"can_read",
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
		return std::string("login_api_tokens");
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

	static LoginApiTokens NewEntity()
	{
		LoginApiTokens e{};

		e.id         = 0;
		e.token      = "";
		e.can_write  = 0;
		e.can_read   = 0;
		e.created_at = 0;
		e.updated_at = std::time(nullptr);

		return e;
	}

	static LoginApiTokens GetLoginApiTokens(
		const std::vector<LoginApiTokens> &login_api_tokenss,
		int login_api_tokens_id
	)
	{
		for (auto &login_api_tokens : login_api_tokenss) {
			if (login_api_tokens.id == login_api_tokens_id) {
				return login_api_tokens;
			}
		}

		return NewEntity();
	}

	static LoginApiTokens FindOne(
		Database& db,
		int login_api_tokens_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				login_api_tokens_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			LoginApiTokens e{};

			e.id         = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.token      = row[1] ? row[1] : "";
			e.can_write  = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.can_read   = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.created_at = strtoll(row[4] ? row[4] : "-1", nullptr, 10);
			e.updated_at = strtoll(row[5] ? row[5] : "-1", nullptr, 10);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int login_api_tokens_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				login_api_tokens_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const LoginApiTokens &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.token) + "'");
		v.push_back(columns[2] + " = " + std::to_string(e.can_write));
		v.push_back(columns[3] + " = " + std::to_string(e.can_read));
		v.push_back(columns[4] + " = FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");
		v.push_back(columns[5] + " = FROM_UNIXTIME(" + (e.updated_at > 0 ? std::to_string(e.updated_at) : "null") + ")");

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

	static LoginApiTokens InsertOne(
		Database& db,
		LoginApiTokens e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.token) + "'");
		v.push_back(std::to_string(e.can_write));
		v.push_back(std::to_string(e.can_read));
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
		const std::vector<LoginApiTokens> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.token) + "'");
			v.push_back(std::to_string(e.can_write));
			v.push_back(std::to_string(e.can_read));
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

	static std::vector<LoginApiTokens> All(Database& db)
	{
		std::vector<LoginApiTokens> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LoginApiTokens e{};

			e.id         = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.token      = row[1] ? row[1] : "";
			e.can_write  = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.can_read   = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.created_at = strtoll(row[4] ? row[4] : "-1", nullptr, 10);
			e.updated_at = strtoll(row[5] ? row[5] : "-1", nullptr, 10);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<LoginApiTokens> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<LoginApiTokens> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LoginApiTokens e{};

			e.id         = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.token      = row[1] ? row[1] : "";
			e.can_write  = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.can_read   = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.created_at = strtoll(row[4] ? row[4] : "-1", nullptr, 10);
			e.updated_at = strtoll(row[5] ? row[5] : "-1", nullptr, 10);

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
		const LoginApiTokens &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.token) + "'");
		v.push_back(std::to_string(e.can_write));
		v.push_back(std::to_string(e.can_read));
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
		const std::vector<LoginApiTokens> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.token) + "'");
			v.push_back(std::to_string(e.can_write));
			v.push_back(std::to_string(e.can_read));
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
