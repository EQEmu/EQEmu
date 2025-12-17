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

class BaseSharedTaskActivityStateRepository {
public:
	struct SharedTaskActivityState {
		int64_t shared_task_id;
		int32_t activity_id;
		int32_t done_count;
		time_t  updated_time;
		time_t  completed_time;
	};

	static std::string PrimaryKey()
	{
		return std::string("shared_task_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"shared_task_id",
			"activity_id",
			"done_count",
			"updated_time",
			"completed_time",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"shared_task_id",
			"activity_id",
			"done_count",
			"UNIX_TIMESTAMP(updated_time)",
			"UNIX_TIMESTAMP(completed_time)",
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
		return std::string("shared_task_activity_state");
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

	static SharedTaskActivityState NewEntity()
	{
		SharedTaskActivityState e{};

		e.shared_task_id = 0;
		e.activity_id    = 0;
		e.done_count     = 0;
		e.updated_time   = 0;
		e.completed_time = 0;

		return e;
	}

	static SharedTaskActivityState GetSharedTaskActivityState(
		const std::vector<SharedTaskActivityState> &shared_task_activity_states,
		int shared_task_activity_state_id
	)
	{
		for (auto &shared_task_activity_state : shared_task_activity_states) {
			if (shared_task_activity_state.shared_task_id == shared_task_activity_state_id) {
				return shared_task_activity_state;
			}
		}

		return NewEntity();
	}

	static SharedTaskActivityState FindOne(
		Database& db,
		int shared_task_activity_state_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				shared_task_activity_state_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			SharedTaskActivityState e{};

			e.shared_task_id = row[0] ? strtoll(row[0], nullptr, 10) : 0;
			e.activity_id    = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.done_count     = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.updated_time   = strtoll(row[3] ? row[3] : "-1", nullptr, 10);
			e.completed_time = strtoll(row[4] ? row[4] : "-1", nullptr, 10);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int shared_task_activity_state_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				shared_task_activity_state_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const SharedTaskActivityState &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.shared_task_id));
		v.push_back(columns[1] + " = " + std::to_string(e.activity_id));
		v.push_back(columns[2] + " = " + std::to_string(e.done_count));
		v.push_back(columns[3] + " = FROM_UNIXTIME(" + (e.updated_time > 0 ? std::to_string(e.updated_time) : "null") + ")");
		v.push_back(columns[4] + " = FROM_UNIXTIME(" + (e.completed_time > 0 ? std::to_string(e.completed_time) : "null") + ")");

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

	static SharedTaskActivityState InsertOne(
		Database& db,
		SharedTaskActivityState e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.shared_task_id));
		v.push_back(std::to_string(e.activity_id));
		v.push_back(std::to_string(e.done_count));
		v.push_back("FROM_UNIXTIME(" + (e.updated_time > 0 ? std::to_string(e.updated_time) : "null") + ")");
		v.push_back("FROM_UNIXTIME(" + (e.completed_time > 0 ? std::to_string(e.completed_time) : "null") + ")");

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
		const std::vector<SharedTaskActivityState> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.shared_task_id));
			v.push_back(std::to_string(e.activity_id));
			v.push_back(std::to_string(e.done_count));
			v.push_back("FROM_UNIXTIME(" + (e.updated_time > 0 ? std::to_string(e.updated_time) : "null") + ")");
			v.push_back("FROM_UNIXTIME(" + (e.completed_time > 0 ? std::to_string(e.completed_time) : "null") + ")");

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

	static std::vector<SharedTaskActivityState> All(Database& db)
	{
		std::vector<SharedTaskActivityState> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SharedTaskActivityState e{};

			e.shared_task_id = row[0] ? strtoll(row[0], nullptr, 10) : 0;
			e.activity_id    = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.done_count     = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.updated_time   = strtoll(row[3] ? row[3] : "-1", nullptr, 10);
			e.completed_time = strtoll(row[4] ? row[4] : "-1", nullptr, 10);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<SharedTaskActivityState> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<SharedTaskActivityState> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SharedTaskActivityState e{};

			e.shared_task_id = row[0] ? strtoll(row[0], nullptr, 10) : 0;
			e.activity_id    = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.done_count     = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.updated_time   = strtoll(row[3] ? row[3] : "-1", nullptr, 10);
			e.completed_time = strtoll(row[4] ? row[4] : "-1", nullptr, 10);

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
		const SharedTaskActivityState &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.shared_task_id));
		v.push_back(std::to_string(e.activity_id));
		v.push_back(std::to_string(e.done_count));
		v.push_back("FROM_UNIXTIME(" + (e.updated_time > 0 ? std::to_string(e.updated_time) : "null") + ")");
		v.push_back("FROM_UNIXTIME(" + (e.completed_time > 0 ? std::to_string(e.completed_time) : "null") + ")");

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
		const std::vector<SharedTaskActivityState> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.shared_task_id));
			v.push_back(std::to_string(e.activity_id));
			v.push_back(std::to_string(e.done_count));
			v.push_back("FROM_UNIXTIME(" + (e.updated_time > 0 ? std::to_string(e.updated_time) : "null") + ")");
			v.push_back("FROM_UNIXTIME(" + (e.completed_time > 0 ? std::to_string(e.completed_time) : "null") + ")");

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
