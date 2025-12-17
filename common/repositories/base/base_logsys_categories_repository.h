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

class BaseLogsysCategoriesRepository {
public:
	struct LogsysCategories {
		int32_t     log_category_id;
		std::string log_category_description;
		int16_t     log_to_console;
		int16_t     log_to_file;
		int16_t     log_to_gmsay;
		int16_t     log_to_discord;
		int32_t     discord_webhook_id;
	};

	static std::string PrimaryKey()
	{
		return std::string("log_category_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"log_category_id",
			"log_category_description",
			"log_to_console",
			"log_to_file",
			"log_to_gmsay",
			"log_to_discord",
			"discord_webhook_id",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"log_category_id",
			"log_category_description",
			"log_to_console",
			"log_to_file",
			"log_to_gmsay",
			"log_to_discord",
			"discord_webhook_id",
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
		return std::string("logsys_categories");
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

	static LogsysCategories NewEntity()
	{
		LogsysCategories e{};

		e.log_category_id          = 0;
		e.log_category_description = "";
		e.log_to_console           = 0;
		e.log_to_file              = 0;
		e.log_to_gmsay             = 0;
		e.log_to_discord           = 0;
		e.discord_webhook_id       = 0;

		return e;
	}

	static LogsysCategories GetLogsysCategories(
		const std::vector<LogsysCategories> &logsys_categoriess,
		int logsys_categories_id
	)
	{
		for (auto &logsys_categories : logsys_categoriess) {
			if (logsys_categories.log_category_id == logsys_categories_id) {
				return logsys_categories;
			}
		}

		return NewEntity();
	}

	static LogsysCategories FindOne(
		Database& db,
		int logsys_categories_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				logsys_categories_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			LogsysCategories e{};

			e.log_category_id          = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.log_category_description = row[1] ? row[1] : "";
			e.log_to_console           = row[2] ? static_cast<int16_t>(atoi(row[2])) : 0;
			e.log_to_file              = row[3] ? static_cast<int16_t>(atoi(row[3])) : 0;
			e.log_to_gmsay             = row[4] ? static_cast<int16_t>(atoi(row[4])) : 0;
			e.log_to_discord           = row[5] ? static_cast<int16_t>(atoi(row[5])) : 0;
			e.discord_webhook_id       = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int logsys_categories_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				logsys_categories_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const LogsysCategories &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.log_category_id));
		v.push_back(columns[1] + " = '" + Strings::Escape(e.log_category_description) + "'");
		v.push_back(columns[2] + " = " + std::to_string(e.log_to_console));
		v.push_back(columns[3] + " = " + std::to_string(e.log_to_file));
		v.push_back(columns[4] + " = " + std::to_string(e.log_to_gmsay));
		v.push_back(columns[5] + " = " + std::to_string(e.log_to_discord));
		v.push_back(columns[6] + " = " + std::to_string(e.discord_webhook_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.log_category_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static LogsysCategories InsertOne(
		Database& db,
		LogsysCategories e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.log_category_id));
		v.push_back("'" + Strings::Escape(e.log_category_description) + "'");
		v.push_back(std::to_string(e.log_to_console));
		v.push_back(std::to_string(e.log_to_file));
		v.push_back(std::to_string(e.log_to_gmsay));
		v.push_back(std::to_string(e.log_to_discord));
		v.push_back(std::to_string(e.discord_webhook_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.log_category_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<LogsysCategories> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.log_category_id));
			v.push_back("'" + Strings::Escape(e.log_category_description) + "'");
			v.push_back(std::to_string(e.log_to_console));
			v.push_back(std::to_string(e.log_to_file));
			v.push_back(std::to_string(e.log_to_gmsay));
			v.push_back(std::to_string(e.log_to_discord));
			v.push_back(std::to_string(e.discord_webhook_id));

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

	static std::vector<LogsysCategories> All(Database& db)
	{
		std::vector<LogsysCategories> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LogsysCategories e{};

			e.log_category_id          = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.log_category_description = row[1] ? row[1] : "";
			e.log_to_console           = row[2] ? static_cast<int16_t>(atoi(row[2])) : 0;
			e.log_to_file              = row[3] ? static_cast<int16_t>(atoi(row[3])) : 0;
			e.log_to_gmsay             = row[4] ? static_cast<int16_t>(atoi(row[4])) : 0;
			e.log_to_discord           = row[5] ? static_cast<int16_t>(atoi(row[5])) : 0;
			e.discord_webhook_id       = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<LogsysCategories> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<LogsysCategories> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LogsysCategories e{};

			e.log_category_id          = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.log_category_description = row[1] ? row[1] : "";
			e.log_to_console           = row[2] ? static_cast<int16_t>(atoi(row[2])) : 0;
			e.log_to_file              = row[3] ? static_cast<int16_t>(atoi(row[3])) : 0;
			e.log_to_gmsay             = row[4] ? static_cast<int16_t>(atoi(row[4])) : 0;
			e.log_to_discord           = row[5] ? static_cast<int16_t>(atoi(row[5])) : 0;
			e.discord_webhook_id       = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;

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
		const LogsysCategories &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.log_category_id));
		v.push_back("'" + Strings::Escape(e.log_category_description) + "'");
		v.push_back(std::to_string(e.log_to_console));
		v.push_back(std::to_string(e.log_to_file));
		v.push_back(std::to_string(e.log_to_gmsay));
		v.push_back(std::to_string(e.log_to_discord));
		v.push_back(std::to_string(e.discord_webhook_id));

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
		const std::vector<LogsysCategories> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.log_category_id));
			v.push_back("'" + Strings::Escape(e.log_category_description) + "'");
			v.push_back(std::to_string(e.log_to_console));
			v.push_back(std::to_string(e.log_to_file));
			v.push_back(std::to_string(e.log_to_gmsay));
			v.push_back(std::to_string(e.log_to_discord));
			v.push_back(std::to_string(e.discord_webhook_id));

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
