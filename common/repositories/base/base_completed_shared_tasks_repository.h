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

class BaseCompletedSharedTasksRepository {
public:
	struct CompletedSharedTasks {
		int64_t id;
		int32_t task_id;
		time_t  accepted_time;
		time_t  expire_time;
		time_t  completion_time;
		int8_t  is_locked;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"task_id",
			"accepted_time",
			"expire_time",
			"completion_time",
			"is_locked",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"task_id",
			"UNIX_TIMESTAMP(accepted_time)",
			"UNIX_TIMESTAMP(expire_time)",
			"UNIX_TIMESTAMP(completion_time)",
			"is_locked",
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
		return std::string("completed_shared_tasks");
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

	static CompletedSharedTasks NewEntity()
	{
		CompletedSharedTasks e{};

		e.id              = 0;
		e.task_id         = 0;
		e.accepted_time   = 0;
		e.expire_time     = 0;
		e.completion_time = 0;
		e.is_locked       = 0;

		return e;
	}

	static CompletedSharedTasks GetCompletedSharedTasks(
		const std::vector<CompletedSharedTasks> &completed_shared_taskss,
		int completed_shared_tasks_id
	)
	{
		for (auto &completed_shared_tasks : completed_shared_taskss) {
			if (completed_shared_tasks.id == completed_shared_tasks_id) {
				return completed_shared_tasks;
			}
		}

		return NewEntity();
	}

	static CompletedSharedTasks FindOne(
		Database& db,
		int completed_shared_tasks_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				completed_shared_tasks_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CompletedSharedTasks e{};

			e.id              = row[0] ? strtoll(row[0], nullptr, 10) : 0;
			e.task_id         = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.accepted_time   = strtoll(row[2] ? row[2] : "-1", nullptr, 10);
			e.expire_time     = strtoll(row[3] ? row[3] : "-1", nullptr, 10);
			e.completion_time = strtoll(row[4] ? row[4] : "-1", nullptr, 10);
			e.is_locked       = row[5] ? static_cast<int8_t>(atoi(row[5])) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int completed_shared_tasks_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				completed_shared_tasks_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CompletedSharedTasks &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.id));
		v.push_back(columns[1] + " = " + std::to_string(e.task_id));
		v.push_back(columns[2] + " = FROM_UNIXTIME(" + (e.accepted_time > 0 ? std::to_string(e.accepted_time) : "null") + ")");
		v.push_back(columns[3] + " = FROM_UNIXTIME(" + (e.expire_time > 0 ? std::to_string(e.expire_time) : "null") + ")");
		v.push_back(columns[4] + " = FROM_UNIXTIME(" + (e.completion_time > 0 ? std::to_string(e.completion_time) : "null") + ")");
		v.push_back(columns[5] + " = " + std::to_string(e.is_locked));

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

	static CompletedSharedTasks InsertOne(
		Database& db,
		CompletedSharedTasks e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.task_id));
		v.push_back("FROM_UNIXTIME(" + (e.accepted_time > 0 ? std::to_string(e.accepted_time) : "null") + ")");
		v.push_back("FROM_UNIXTIME(" + (e.expire_time > 0 ? std::to_string(e.expire_time) : "null") + ")");
		v.push_back("FROM_UNIXTIME(" + (e.completion_time > 0 ? std::to_string(e.completion_time) : "null") + ")");
		v.push_back(std::to_string(e.is_locked));

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
		const std::vector<CompletedSharedTasks> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.task_id));
			v.push_back("FROM_UNIXTIME(" + (e.accepted_time > 0 ? std::to_string(e.accepted_time) : "null") + ")");
			v.push_back("FROM_UNIXTIME(" + (e.expire_time > 0 ? std::to_string(e.expire_time) : "null") + ")");
			v.push_back("FROM_UNIXTIME(" + (e.completion_time > 0 ? std::to_string(e.completion_time) : "null") + ")");
			v.push_back(std::to_string(e.is_locked));

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

	static std::vector<CompletedSharedTasks> All(Database& db)
	{
		std::vector<CompletedSharedTasks> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CompletedSharedTasks e{};

			e.id              = row[0] ? strtoll(row[0], nullptr, 10) : 0;
			e.task_id         = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.accepted_time   = strtoll(row[2] ? row[2] : "-1", nullptr, 10);
			e.expire_time     = strtoll(row[3] ? row[3] : "-1", nullptr, 10);
			e.completion_time = strtoll(row[4] ? row[4] : "-1", nullptr, 10);
			e.is_locked       = row[5] ? static_cast<int8_t>(atoi(row[5])) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CompletedSharedTasks> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CompletedSharedTasks> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CompletedSharedTasks e{};

			e.id              = row[0] ? strtoll(row[0], nullptr, 10) : 0;
			e.task_id         = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.accepted_time   = strtoll(row[2] ? row[2] : "-1", nullptr, 10);
			e.expire_time     = strtoll(row[3] ? row[3] : "-1", nullptr, 10);
			e.completion_time = strtoll(row[4] ? row[4] : "-1", nullptr, 10);
			e.is_locked       = row[5] ? static_cast<int8_t>(atoi(row[5])) : 0;

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
		const CompletedSharedTasks &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.task_id));
		v.push_back("FROM_UNIXTIME(" + (e.accepted_time > 0 ? std::to_string(e.accepted_time) : "null") + ")");
		v.push_back("FROM_UNIXTIME(" + (e.expire_time > 0 ? std::to_string(e.expire_time) : "null") + ")");
		v.push_back("FROM_UNIXTIME(" + (e.completion_time > 0 ? std::to_string(e.completion_time) : "null") + ")");
		v.push_back(std::to_string(e.is_locked));

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
		const std::vector<CompletedSharedTasks> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.task_id));
			v.push_back("FROM_UNIXTIME(" + (e.accepted_time > 0 ? std::to_string(e.accepted_time) : "null") + ")");
			v.push_back("FROM_UNIXTIME(" + (e.expire_time > 0 ? std::to_string(e.expire_time) : "null") + ")");
			v.push_back("FROM_UNIXTIME(" + (e.completion_time > 0 ? std::to_string(e.completion_time) : "null") + ")");
			v.push_back(std::to_string(e.is_locked));

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
