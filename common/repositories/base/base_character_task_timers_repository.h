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

class BaseCharacterTaskTimersRepository {
public:
	struct CharacterTaskTimers {
		uint32_t id;
		uint32_t character_id;
		uint32_t task_id;
		int32_t  timer_type;
		int32_t  timer_group;
		time_t   expire_time;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"character_id",
			"task_id",
			"timer_type",
			"timer_group",
			"expire_time",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"character_id",
			"task_id",
			"timer_type",
			"timer_group",
			"UNIX_TIMESTAMP(expire_time)",
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
		return std::string("character_task_timers");
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

	static CharacterTaskTimers NewEntity()
	{
		CharacterTaskTimers e{};

		e.id           = 0;
		e.character_id = 0;
		e.task_id      = 0;
		e.timer_type   = 0;
		e.timer_group  = 0;
		e.expire_time  = std::time(nullptr);

		return e;
	}

	static CharacterTaskTimers GetCharacterTaskTimers(
		const std::vector<CharacterTaskTimers> &character_task_timerss,
		int character_task_timers_id
	)
	{
		for (auto &character_task_timers : character_task_timerss) {
			if (character_task_timers.id == character_task_timers_id) {
				return character_task_timers;
			}
		}

		return NewEntity();
	}

	static CharacterTaskTimers FindOne(
		Database& db,
		int character_task_timers_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				character_task_timers_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterTaskTimers e{};

			e.id           = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.character_id = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.task_id      = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.timer_type   = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.timer_group  = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.expire_time  = strtoll(row[5] ? row[5] : "-1", nullptr, 10);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_task_timers_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_task_timers_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CharacterTaskTimers &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.character_id));
		v.push_back(columns[2] + " = " + std::to_string(e.task_id));
		v.push_back(columns[3] + " = " + std::to_string(e.timer_type));
		v.push_back(columns[4] + " = " + std::to_string(e.timer_group));
		v.push_back(columns[5] + " = FROM_UNIXTIME(" + (e.expire_time > 0 ? std::to_string(e.expire_time) : "null") + ")");

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

	static CharacterTaskTimers InsertOne(
		Database& db,
		CharacterTaskTimers e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.character_id));
		v.push_back(std::to_string(e.task_id));
		v.push_back(std::to_string(e.timer_type));
		v.push_back(std::to_string(e.timer_group));
		v.push_back("FROM_UNIXTIME(" + (e.expire_time > 0 ? std::to_string(e.expire_time) : "null") + ")");

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
		const std::vector<CharacterTaskTimers> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.character_id));
			v.push_back(std::to_string(e.task_id));
			v.push_back(std::to_string(e.timer_type));
			v.push_back(std::to_string(e.timer_group));
			v.push_back("FROM_UNIXTIME(" + (e.expire_time > 0 ? std::to_string(e.expire_time) : "null") + ")");

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

	static std::vector<CharacterTaskTimers> All(Database& db)
	{
		std::vector<CharacterTaskTimers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterTaskTimers e{};

			e.id           = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.character_id = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.task_id      = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.timer_type   = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.timer_group  = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.expire_time  = strtoll(row[5] ? row[5] : "-1", nullptr, 10);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CharacterTaskTimers> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CharacterTaskTimers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterTaskTimers e{};

			e.id           = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.character_id = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.task_id      = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.timer_type   = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.timer_group  = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.expire_time  = strtoll(row[5] ? row[5] : "-1", nullptr, 10);

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
		const CharacterTaskTimers &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.character_id));
		v.push_back(std::to_string(e.task_id));
		v.push_back(std::to_string(e.timer_type));
		v.push_back(std::to_string(e.timer_group));
		v.push_back("FROM_UNIXTIME(" + (e.expire_time > 0 ? std::to_string(e.expire_time) : "null") + ")");

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
		const std::vector<CharacterTaskTimers> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.character_id));
			v.push_back(std::to_string(e.task_id));
			v.push_back(std::to_string(e.timer_type));
			v.push_back(std::to_string(e.timer_group));
			v.push_back("FROM_UNIXTIME(" + (e.expire_time > 0 ? std::to_string(e.expire_time) : "null") + ")");

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
