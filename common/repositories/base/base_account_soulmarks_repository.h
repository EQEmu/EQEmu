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

class BaseAccountSoulmarksRepository {
public:
	struct AccountSoulmarks {
		uint64_t    id;
		int32_t     account_id;
		uint32_t    character_id;
		std::string character_name;
		int32_t     gm_account_id;
		uint32_t    gm_character_id;
		std::string gm_character_name;
		uint8_t     type;
		std::string text;
		time_t      created_at;
		time_t      updated_at;
		time_t      deleted_at;
		uint32_t    revision;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"account_id",
			"character_id",
			"character_name",
			"gm_account_id",
			"gm_character_id",
			"gm_character_name",
			"type",
			"text",
			"created_at",
			"updated_at",
			"deleted_at",
			"revision",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"account_id",
			"character_id",
			"character_name",
			"gm_account_id",
			"gm_character_id",
			"gm_character_name",
			"type",
			"text",
			"UNIX_TIMESTAMP(created_at)",
			"UNIX_TIMESTAMP(updated_at)",
			"UNIX_TIMESTAMP(deleted_at)",
			"revision",
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
		return std::string("account_soulmarks");
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

	static AccountSoulmarks NewEntity()
	{
		AccountSoulmarks e{};

		e.id                = 0;
		e.account_id        = 0;
		e.character_id      = 0;
		e.character_name    = "";
		e.gm_account_id     = 0;
		e.gm_character_id   = 0;
		e.gm_character_name = "";
		e.type              = 0;
		e.text              = "";
		e.created_at        = 0;
		e.updated_at        = 0;
		e.deleted_at        = 0;
		e.revision          = 1;

		return e;
	}

	static AccountSoulmarks GetAccountSoulmarks(
		const std::vector<AccountSoulmarks> &account_soulmarkss,
		int account_soulmarks_id
	)
	{
		for (auto &account_soulmarks : account_soulmarkss) {
			if (account_soulmarks.id == account_soulmarks_id) {
				return account_soulmarks;
			}
		}

		return NewEntity();
	}

	static AccountSoulmarks FindOne(
		Database& db,
		int account_soulmarks_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				account_soulmarks_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			AccountSoulmarks e{};

			e.id                = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.account_id        = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.character_id      = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.character_name    = row[3] ? row[3] : "";
			e.gm_account_id     = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.gm_character_id   = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.gm_character_name = row[6] ? row[6] : "";
			e.type              = row[7] ? static_cast<uint8_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.text              = row[8] ? row[8] : "";
			e.created_at        = strtoll(row[9] ? row[9] : "-1", nullptr, 10);
			e.updated_at        = strtoll(row[10] ? row[10] : "-1", nullptr, 10);
			e.deleted_at        = strtoll(row[11] ? row[11] : "-1", nullptr, 10);
			e.revision          = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 1;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int account_soulmarks_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				account_soulmarks_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const AccountSoulmarks &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.account_id));
		v.push_back(columns[2] + " = " + std::to_string(e.character_id));
		v.push_back(columns[3] + " = '" + Strings::Escape(e.character_name) + "'");
		v.push_back(columns[4] + " = " + std::to_string(e.gm_account_id));
		v.push_back(columns[5] + " = " + std::to_string(e.gm_character_id));
		v.push_back(columns[6] + " = '" + Strings::Escape(e.gm_character_name) + "'");
		v.push_back(columns[7] + " = " + std::to_string(e.type));
		v.push_back(columns[8] + " = '" + Strings::Escape(e.text) + "'");
		v.push_back(columns[9] + " = FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");
		v.push_back(columns[10] + " = FROM_UNIXTIME(" + (e.updated_at > 0 ? std::to_string(e.updated_at) : "null") + ")");
		v.push_back(columns[11] + " = FROM_UNIXTIME(" + (e.deleted_at > 0 ? std::to_string(e.deleted_at) : "null") + ")");
		v.push_back(columns[12] + " = " + std::to_string(e.revision));

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

	static AccountSoulmarks InsertOne(
		Database& db,
		AccountSoulmarks e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.account_id));
		v.push_back(std::to_string(e.character_id));
		v.push_back("'" + Strings::Escape(e.character_name) + "'");
		v.push_back(std::to_string(e.gm_account_id));
		v.push_back(std::to_string(e.gm_character_id));
		v.push_back("'" + Strings::Escape(e.gm_character_name) + "'");
		v.push_back(std::to_string(e.type));
		v.push_back("'" + Strings::Escape(e.text) + "'");
		v.push_back("FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");
		v.push_back("FROM_UNIXTIME(" + (e.updated_at > 0 ? std::to_string(e.updated_at) : "null") + ")");
		v.push_back("FROM_UNIXTIME(" + (e.deleted_at > 0 ? std::to_string(e.deleted_at) : "null") + ")");
		v.push_back(std::to_string(e.revision));

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
		const std::vector<AccountSoulmarks> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.account_id));
			v.push_back(std::to_string(e.character_id));
			v.push_back("'" + Strings::Escape(e.character_name) + "'");
			v.push_back(std::to_string(e.gm_account_id));
			v.push_back(std::to_string(e.gm_character_id));
			v.push_back("'" + Strings::Escape(e.gm_character_name) + "'");
			v.push_back(std::to_string(e.type));
			v.push_back("'" + Strings::Escape(e.text) + "'");
			v.push_back("FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");
			v.push_back("FROM_UNIXTIME(" + (e.updated_at > 0 ? std::to_string(e.updated_at) : "null") + ")");
			v.push_back("FROM_UNIXTIME(" + (e.deleted_at > 0 ? std::to_string(e.deleted_at) : "null") + ")");
			v.push_back(std::to_string(e.revision));

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

	static std::vector<AccountSoulmarks> All(Database& db)
	{
		std::vector<AccountSoulmarks> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AccountSoulmarks e{};

			e.id                = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.account_id        = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.character_id      = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.character_name    = row[3] ? row[3] : "";
			e.gm_account_id     = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.gm_character_id   = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.gm_character_name = row[6] ? row[6] : "";
			e.type              = row[7] ? static_cast<uint8_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.text              = row[8] ? row[8] : "";
			e.created_at        = strtoll(row[9] ? row[9] : "-1", nullptr, 10);
			e.updated_at        = strtoll(row[10] ? row[10] : "-1", nullptr, 10);
			e.deleted_at        = strtoll(row[11] ? row[11] : "-1", nullptr, 10);
			e.revision          = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 1;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<AccountSoulmarks> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<AccountSoulmarks> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AccountSoulmarks e{};

			e.id                = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.account_id        = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.character_id      = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.character_name    = row[3] ? row[3] : "";
			e.gm_account_id     = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.gm_character_id   = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.gm_character_name = row[6] ? row[6] : "";
			e.type              = row[7] ? static_cast<uint8_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.text              = row[8] ? row[8] : "";
			e.created_at        = strtoll(row[9] ? row[9] : "-1", nullptr, 10);
			e.updated_at        = strtoll(row[10] ? row[10] : "-1", nullptr, 10);
			e.deleted_at        = strtoll(row[11] ? row[11] : "-1", nullptr, 10);
			e.revision          = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 1;

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
		const AccountSoulmarks &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.account_id));
		v.push_back(std::to_string(e.character_id));
		v.push_back("'" + Strings::Escape(e.character_name) + "'");
		v.push_back(std::to_string(e.gm_account_id));
		v.push_back(std::to_string(e.gm_character_id));
		v.push_back("'" + Strings::Escape(e.gm_character_name) + "'");
		v.push_back(std::to_string(e.type));
		v.push_back("'" + Strings::Escape(e.text) + "'");
		v.push_back("FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");
		v.push_back("FROM_UNIXTIME(" + (e.updated_at > 0 ? std::to_string(e.updated_at) : "null") + ")");
		v.push_back("FROM_UNIXTIME(" + (e.deleted_at > 0 ? std::to_string(e.deleted_at) : "null") + ")");
		v.push_back(std::to_string(e.revision));

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
		const std::vector<AccountSoulmarks> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.account_id));
			v.push_back(std::to_string(e.character_id));
			v.push_back("'" + Strings::Escape(e.character_name) + "'");
			v.push_back(std::to_string(e.gm_account_id));
			v.push_back(std::to_string(e.gm_character_id));
			v.push_back("'" + Strings::Escape(e.gm_character_name) + "'");
			v.push_back(std::to_string(e.type));
			v.push_back("'" + Strings::Escape(e.text) + "'");
			v.push_back("FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");
			v.push_back("FROM_UNIXTIME(" + (e.updated_at > 0 ? std::to_string(e.updated_at) : "null") + ")");
			v.push_back("FROM_UNIXTIME(" + (e.deleted_at > 0 ? std::to_string(e.deleted_at) : "null") + ")");
			v.push_back(std::to_string(e.revision));

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
