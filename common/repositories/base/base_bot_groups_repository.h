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

class BaseBotGroupsRepository {
public:
	struct BotGroups {
		uint32_t    groups_index;
		uint32_t    group_leader_id;
		std::string group_name;
		uint8_t     auto_spawn;
	};

	static std::string PrimaryKey()
	{
		return std::string("groups_index");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"groups_index",
			"group_leader_id",
			"group_name",
			"auto_spawn",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"groups_index",
			"group_leader_id",
			"group_name",
			"auto_spawn",
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
		return std::string("bot_groups");
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

	static BotGroups NewEntity()
	{
		BotGroups e{};

		e.groups_index    = 0;
		e.group_leader_id = 0;
		e.group_name      = "";
		e.auto_spawn      = 0;

		return e;
	}

	static BotGroups GetBotGroups(
		const std::vector<BotGroups> &bot_groupss,
		int bot_groups_id
	)
	{
		for (auto &bot_groups : bot_groupss) {
			if (bot_groups.groups_index == bot_groups_id) {
				return bot_groups;
			}
		}

		return NewEntity();
	}

	static BotGroups FindOne(
		Database& db,
		int bot_groups_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				bot_groups_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			BotGroups e{};

			e.groups_index    = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.group_leader_id = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.group_name      = row[2] ? row[2] : "";
			e.auto_spawn      = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int bot_groups_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				bot_groups_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const BotGroups &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.group_leader_id));
		v.push_back(columns[2] + " = '" + Strings::Escape(e.group_name) + "'");
		v.push_back(columns[3] + " = " + std::to_string(e.auto_spawn));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.groups_index
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static BotGroups InsertOne(
		Database& db,
		BotGroups e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.groups_index));
		v.push_back(std::to_string(e.group_leader_id));
		v.push_back("'" + Strings::Escape(e.group_name) + "'");
		v.push_back(std::to_string(e.auto_spawn));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.groups_index = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<BotGroups> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.groups_index));
			v.push_back(std::to_string(e.group_leader_id));
			v.push_back("'" + Strings::Escape(e.group_name) + "'");
			v.push_back(std::to_string(e.auto_spawn));

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

	static std::vector<BotGroups> All(Database& db)
	{
		std::vector<BotGroups> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotGroups e{};

			e.groups_index    = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.group_leader_id = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.group_name      = row[2] ? row[2] : "";
			e.auto_spawn      = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<BotGroups> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<BotGroups> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotGroups e{};

			e.groups_index    = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.group_leader_id = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.group_name      = row[2] ? row[2] : "";
			e.auto_spawn      = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));

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

};
