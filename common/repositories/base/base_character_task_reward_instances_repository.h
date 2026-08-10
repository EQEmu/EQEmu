/**
 * DO NOT MODIFY THIS FILE
 *
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to the repository extending the base.
 * Any modifications to base repositories are to be made by the generator only
 *
 * @generator ./utils/scripts/generators/repository-generator.pl
 * @docs https://docs.eqemu.io/developer/repositories
 */

#ifndef EQEMU_BASE_CHARACTER_TASK_REWARD_INSTANCES_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_TASK_REWARD_INSTANCES_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseCharacterTaskRewardInstancesRepository {
public:
	struct CharacterTaskRewardInstances {
		uint64_t occurrence_id;
		uint32_t character_id;
		uint32_t task_id;
		uint32_t accepted_time;
	};

	static std::string PrimaryKey()
	{
		return std::string("occurrence_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"occurrence_id",
			"character_id",
			"task_id",
			"accepted_time",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"occurrence_id",
			"character_id",
			"task_id",
			"accepted_time",
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
		return std::string("character_task_reward_instances");
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

	static CharacterTaskRewardInstances NewEntity()
	{
		CharacterTaskRewardInstances e{};

		e.occurrence_id = 0;
		e.character_id  = 0;
		e.task_id       = 0;
		e.accepted_time = 0;

		return e;
	}

	static CharacterTaskRewardInstances GetCharacterTaskRewardInstances(
		const std::vector<CharacterTaskRewardInstances> &character_task_reward_instancess,
		int character_task_reward_instances_id
	)
	{
		for (auto &character_task_reward_instances : character_task_reward_instancess) {
			if (character_task_reward_instances.occurrence_id == character_task_reward_instances_id) {
				return character_task_reward_instances;
			}
		}

		return NewEntity();
	}

	static CharacterTaskRewardInstances FindOne(
		Database& db,
		int character_task_reward_instances_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				character_task_reward_instances_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterTaskRewardInstances e{};

			e.occurrence_id = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.character_id  = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.task_id       = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.accepted_time = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_task_reward_instances_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_task_reward_instances_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CharacterTaskRewardInstances &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.character_id));
		v.push_back(columns[2] + " = " + std::to_string(e.task_id));
		v.push_back(columns[3] + " = " + std::to_string(e.accepted_time));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.occurrence_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterTaskRewardInstances InsertOne(
		Database& db,
		CharacterTaskRewardInstances e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.occurrence_id));
		v.push_back(std::to_string(e.character_id));
		v.push_back(std::to_string(e.task_id));
		v.push_back(std::to_string(e.accepted_time));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.occurrence_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<CharacterTaskRewardInstances> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.occurrence_id));
			v.push_back(std::to_string(e.character_id));
			v.push_back(std::to_string(e.task_id));
			v.push_back(std::to_string(e.accepted_time));

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

	static std::vector<CharacterTaskRewardInstances> All(Database& db)
	{
		std::vector<CharacterTaskRewardInstances> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterTaskRewardInstances e{};

			e.occurrence_id = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.character_id  = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.task_id       = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.accepted_time = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CharacterTaskRewardInstances> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CharacterTaskRewardInstances> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterTaskRewardInstances e{};

			e.occurrence_id = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.character_id  = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.task_id       = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.accepted_time = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;

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
		const CharacterTaskRewardInstances &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.occurrence_id));
		v.push_back(std::to_string(e.character_id));
		v.push_back(std::to_string(e.task_id));
		v.push_back(std::to_string(e.accepted_time));

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
		const std::vector<CharacterTaskRewardInstances> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.occurrence_id));
			v.push_back(std::to_string(e.character_id));
			v.push_back(std::to_string(e.task_id));
			v.push_back(std::to_string(e.accepted_time));

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

#endif //EQEMU_BASE_CHARACTER_TASK_REWARD_INSTANCES_REPOSITORY_H
