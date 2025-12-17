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

class BaseDiscordWebhooksRepository {
public:
	struct DiscordWebhooks {
		int32_t     id;
		std::string webhook_name;
		std::string webhook_url;
		time_t      created_at;
		time_t      deleted_at;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"webhook_name",
			"webhook_url",
			"created_at",
			"deleted_at",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"webhook_name",
			"webhook_url",
			"UNIX_TIMESTAMP(created_at)",
			"UNIX_TIMESTAMP(deleted_at)",
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
		return std::string("discord_webhooks");
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

	static DiscordWebhooks NewEntity()
	{
		DiscordWebhooks e{};

		e.id           = 0;
		e.webhook_name = "";
		e.webhook_url  = "";
		e.created_at   = 0;
		e.deleted_at   = 0;

		return e;
	}

	static DiscordWebhooks GetDiscordWebhooks(
		const std::vector<DiscordWebhooks> &discord_webhookss,
		int discord_webhooks_id
	)
	{
		for (auto &discord_webhooks : discord_webhookss) {
			if (discord_webhooks.id == discord_webhooks_id) {
				return discord_webhooks;
			}
		}

		return NewEntity();
	}

	static DiscordWebhooks FindOne(
		Database& db,
		int discord_webhooks_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				discord_webhooks_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			DiscordWebhooks e{};

			e.id           = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.webhook_name = row[1] ? row[1] : "";
			e.webhook_url  = row[2] ? row[2] : "";
			e.created_at   = strtoll(row[3] ? row[3] : "-1", nullptr, 10);
			e.deleted_at   = strtoll(row[4] ? row[4] : "-1", nullptr, 10);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int discord_webhooks_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				discord_webhooks_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const DiscordWebhooks &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.webhook_name) + "'");
		v.push_back(columns[2] + " = '" + Strings::Escape(e.webhook_url) + "'");
		v.push_back(columns[3] + " = FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");
		v.push_back(columns[4] + " = FROM_UNIXTIME(" + (e.deleted_at > 0 ? std::to_string(e.deleted_at) : "null") + ")");

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

	static DiscordWebhooks InsertOne(
		Database& db,
		DiscordWebhooks e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.webhook_name) + "'");
		v.push_back("'" + Strings::Escape(e.webhook_url) + "'");
		v.push_back("FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");
		v.push_back("FROM_UNIXTIME(" + (e.deleted_at > 0 ? std::to_string(e.deleted_at) : "null") + ")");

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
		const std::vector<DiscordWebhooks> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.webhook_name) + "'");
			v.push_back("'" + Strings::Escape(e.webhook_url) + "'");
			v.push_back("FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");
			v.push_back("FROM_UNIXTIME(" + (e.deleted_at > 0 ? std::to_string(e.deleted_at) : "null") + ")");

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

	static std::vector<DiscordWebhooks> All(Database& db)
	{
		std::vector<DiscordWebhooks> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			DiscordWebhooks e{};

			e.id           = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.webhook_name = row[1] ? row[1] : "";
			e.webhook_url  = row[2] ? row[2] : "";
			e.created_at   = strtoll(row[3] ? row[3] : "-1", nullptr, 10);
			e.deleted_at   = strtoll(row[4] ? row[4] : "-1", nullptr, 10);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<DiscordWebhooks> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<DiscordWebhooks> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			DiscordWebhooks e{};

			e.id           = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.webhook_name = row[1] ? row[1] : "";
			e.webhook_url  = row[2] ? row[2] : "";
			e.created_at   = strtoll(row[3] ? row[3] : "-1", nullptr, 10);
			e.deleted_at   = strtoll(row[4] ? row[4] : "-1", nullptr, 10);

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
		const DiscordWebhooks &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.webhook_name) + "'");
		v.push_back("'" + Strings::Escape(e.webhook_url) + "'");
		v.push_back("FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");
		v.push_back("FROM_UNIXTIME(" + (e.deleted_at > 0 ? std::to_string(e.deleted_at) : "null") + ")");

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
		const std::vector<DiscordWebhooks> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.webhook_name) + "'");
			v.push_back("'" + Strings::Escape(e.webhook_url) + "'");
			v.push_back("FROM_UNIXTIME(" + (e.created_at > 0 ? std::to_string(e.created_at) : "null") + ")");
			v.push_back("FROM_UNIXTIME(" + (e.deleted_at > 0 ? std::to_string(e.deleted_at) : "null") + ")");

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
