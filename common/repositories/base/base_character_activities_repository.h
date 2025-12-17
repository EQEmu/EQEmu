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

class BaseCharacterActivitiesRepository {
public:
	struct CharacterActivities {
		uint32_t charid;
		uint32_t taskid;
		uint32_t activityid;
		uint32_t donecount;
		int8_t   completed;
	};

	static std::string PrimaryKey()
	{
		return std::string("charid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"charid",
			"taskid",
			"activityid",
			"donecount",
			"completed",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"charid",
			"taskid",
			"activityid",
			"donecount",
			"completed",
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
		return std::string("character_activities");
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

	static CharacterActivities NewEntity()
	{
		CharacterActivities e{};

		e.charid     = 0;
		e.taskid     = 0;
		e.activityid = 0;
		e.donecount  = 0;
		e.completed  = 0;

		return e;
	}

	static CharacterActivities GetCharacterActivities(
		const std::vector<CharacterActivities> &character_activitiess,
		int character_activities_id
	)
	{
		for (auto &character_activities : character_activitiess) {
			if (character_activities.charid == character_activities_id) {
				return character_activities;
			}
		}

		return NewEntity();
	}

	static CharacterActivities FindOne(
		Database& db,
		int character_activities_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				character_activities_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterActivities e{};

			e.charid     = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.taskid     = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.activityid = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.donecount  = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.completed  = row[4] ? static_cast<int8_t>(atoi(row[4])) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_activities_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_activities_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CharacterActivities &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.charid));
		v.push_back(columns[1] + " = " + std::to_string(e.taskid));
		v.push_back(columns[2] + " = " + std::to_string(e.activityid));
		v.push_back(columns[3] + " = " + std::to_string(e.donecount));
		v.push_back(columns[4] + " = " + std::to_string(e.completed));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.charid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterActivities InsertOne(
		Database& db,
		CharacterActivities e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.charid));
		v.push_back(std::to_string(e.taskid));
		v.push_back(std::to_string(e.activityid));
		v.push_back(std::to_string(e.donecount));
		v.push_back(std::to_string(e.completed));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.charid = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<CharacterActivities> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.charid));
			v.push_back(std::to_string(e.taskid));
			v.push_back(std::to_string(e.activityid));
			v.push_back(std::to_string(e.donecount));
			v.push_back(std::to_string(e.completed));

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

	static std::vector<CharacterActivities> All(Database& db)
	{
		std::vector<CharacterActivities> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterActivities e{};

			e.charid     = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.taskid     = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.activityid = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.donecount  = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.completed  = row[4] ? static_cast<int8_t>(atoi(row[4])) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CharacterActivities> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CharacterActivities> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterActivities e{};

			e.charid     = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.taskid     = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.activityid = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.donecount  = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.completed  = row[4] ? static_cast<int8_t>(atoi(row[4])) : 0;

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
		const CharacterActivities &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.charid));
		v.push_back(std::to_string(e.taskid));
		v.push_back(std::to_string(e.activityid));
		v.push_back(std::to_string(e.donecount));
		v.push_back(std::to_string(e.completed));

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
		const std::vector<CharacterActivities> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.charid));
			v.push_back(std::to_string(e.taskid));
			v.push_back(std::to_string(e.activityid));
			v.push_back(std::to_string(e.donecount));
			v.push_back(std::to_string(e.completed));

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
