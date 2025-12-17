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

class BaseCompletedSharedTaskMembersRepository {
public:
	struct CompletedSharedTaskMembers {
		int64_t shared_task_id;
		int64_t character_id;
		int8_t  is_leader;
	};

	static std::string PrimaryKey()
	{
		return std::string("shared_task_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"shared_task_id",
			"character_id",
			"is_leader",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"shared_task_id",
			"character_id",
			"is_leader",
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
		return std::string("completed_shared_task_members");
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

	static CompletedSharedTaskMembers NewEntity()
	{
		CompletedSharedTaskMembers e{};

		e.shared_task_id = 0;
		e.character_id   = 0;
		e.is_leader      = 0;

		return e;
	}

	static CompletedSharedTaskMembers GetCompletedSharedTaskMembers(
		const std::vector<CompletedSharedTaskMembers> &completed_shared_task_memberss,
		int completed_shared_task_members_id
	)
	{
		for (auto &completed_shared_task_members : completed_shared_task_memberss) {
			if (completed_shared_task_members.shared_task_id == completed_shared_task_members_id) {
				return completed_shared_task_members;
			}
		}

		return NewEntity();
	}

	static CompletedSharedTaskMembers FindOne(
		Database& db,
		int completed_shared_task_members_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				completed_shared_task_members_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CompletedSharedTaskMembers e{};

			e.shared_task_id = row[0] ? strtoll(row[0], nullptr, 10) : 0;
			e.character_id   = row[1] ? strtoll(row[1], nullptr, 10) : 0;
			e.is_leader      = row[2] ? static_cast<int8_t>(atoi(row[2])) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int completed_shared_task_members_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				completed_shared_task_members_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CompletedSharedTaskMembers &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.shared_task_id));
		v.push_back(columns[1] + " = " + std::to_string(e.character_id));
		v.push_back(columns[2] + " = " + std::to_string(e.is_leader));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.shared_task_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CompletedSharedTaskMembers InsertOne(
		Database& db,
		CompletedSharedTaskMembers e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.shared_task_id));
		v.push_back(std::to_string(e.character_id));
		v.push_back(std::to_string(e.is_leader));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.shared_task_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<CompletedSharedTaskMembers> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.shared_task_id));
			v.push_back(std::to_string(e.character_id));
			v.push_back(std::to_string(e.is_leader));

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

	static std::vector<CompletedSharedTaskMembers> All(Database& db)
	{
		std::vector<CompletedSharedTaskMembers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CompletedSharedTaskMembers e{};

			e.shared_task_id = row[0] ? strtoll(row[0], nullptr, 10) : 0;
			e.character_id   = row[1] ? strtoll(row[1], nullptr, 10) : 0;
			e.is_leader      = row[2] ? static_cast<int8_t>(atoi(row[2])) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CompletedSharedTaskMembers> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CompletedSharedTaskMembers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CompletedSharedTaskMembers e{};

			e.shared_task_id = row[0] ? strtoll(row[0], nullptr, 10) : 0;
			e.character_id   = row[1] ? strtoll(row[1], nullptr, 10) : 0;
			e.is_leader      = row[2] ? static_cast<int8_t>(atoi(row[2])) : 0;

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
		const CompletedSharedTaskMembers &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.shared_task_id));
		v.push_back(std::to_string(e.character_id));
		v.push_back(std::to_string(e.is_leader));

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
		const std::vector<CompletedSharedTaskMembers> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.shared_task_id));
			v.push_back(std::to_string(e.character_id));
			v.push_back(std::to_string(e.is_leader));

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
