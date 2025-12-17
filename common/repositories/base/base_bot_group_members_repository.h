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

class BaseBotGroupMembersRepository {
public:
	struct BotGroupMembers {
		uint32_t group_members_index;
		uint32_t groups_index;
		uint32_t bot_id;
	};

	static std::string PrimaryKey()
	{
		return std::string("group_members_index");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"group_members_index",
			"groups_index",
			"bot_id",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"group_members_index",
			"groups_index",
			"bot_id",
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
		return std::string("bot_group_members");
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

	static BotGroupMembers NewEntity()
	{
		BotGroupMembers e{};

		e.group_members_index = 0;
		e.groups_index        = 0;
		e.bot_id              = 0;

		return e;
	}

	static BotGroupMembers GetBotGroupMembers(
		const std::vector<BotGroupMembers> &bot_group_memberss,
		int bot_group_members_id
	)
	{
		for (auto &bot_group_members : bot_group_memberss) {
			if (bot_group_members.group_members_index == bot_group_members_id) {
				return bot_group_members;
			}
		}

		return NewEntity();
	}

	static BotGroupMembers FindOne(
		Database& db,
		int bot_group_members_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				bot_group_members_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			BotGroupMembers e{};

			e.group_members_index = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.groups_index        = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.bot_id              = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int bot_group_members_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				bot_group_members_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const BotGroupMembers &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.groups_index));
		v.push_back(columns[2] + " = " + std::to_string(e.bot_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.group_members_index
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static BotGroupMembers InsertOne(
		Database& db,
		BotGroupMembers e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.group_members_index));
		v.push_back(std::to_string(e.groups_index));
		v.push_back(std::to_string(e.bot_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.group_members_index = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<BotGroupMembers> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.group_members_index));
			v.push_back(std::to_string(e.groups_index));
			v.push_back(std::to_string(e.bot_id));

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

	static std::vector<BotGroupMembers> All(Database& db)
	{
		std::vector<BotGroupMembers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotGroupMembers e{};

			e.group_members_index = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.groups_index        = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.bot_id              = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<BotGroupMembers> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<BotGroupMembers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotGroupMembers e{};

			e.group_members_index = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.groups_index        = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.bot_id              = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));

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
