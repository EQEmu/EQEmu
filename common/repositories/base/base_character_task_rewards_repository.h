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

#ifndef EQEMU_BASE_CHARACTER_TASK_REWARDS_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_TASK_REWARDS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseCharacterTaskRewardsRepository {
public:
	struct CharacterTaskRewards {
		uint32_t    character_id;
		uint32_t    pending_reward_id;
		uint64_t    reward_id;
		uint8_t     status;
		uint32_t    attempt_count;
		uint32_t    granted_at;
		uint32_t    last_attempt_at;
		std::string last_error;
	};

	static std::string PrimaryKey()
	{
		return std::string("pending_reward_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"character_id",
			"pending_reward_id",
			"reward_id",
			"status",
			"attempt_count",
			"granted_at",
			"last_attempt_at",
			"last_error",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"character_id",
			"pending_reward_id",
			"reward_id",
			"status",
			"attempt_count",
			"granted_at",
			"last_attempt_at",
			"last_error",
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
		return std::string("character_task_rewards");
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

	static CharacterTaskRewards NewEntity()
	{
		CharacterTaskRewards e{};

		e.character_id      = 0;
		e.pending_reward_id = 0;
		e.reward_id         = 0;
		e.status            = 0;
		e.attempt_count     = 0;
		e.granted_at        = 0;
		e.last_attempt_at   = 0;
		e.last_error        = "";

		return e;
	}

	static CharacterTaskRewards GetCharacterTaskRewards(
		const std::vector<CharacterTaskRewards> &character_task_rewardss,
		int character_task_rewards_id
	)
	{
		for (auto &character_task_rewards : character_task_rewardss) {
			if (character_task_rewards.pending_reward_id == character_task_rewards_id) {
				return character_task_rewards;
			}
		}

		return NewEntity();
	}

	static CharacterTaskRewards FindOne(
		Database& db,
		int character_task_rewards_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				character_task_rewards_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterTaskRewards e{};

			e.character_id      = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.pending_reward_id = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.reward_id         = row[2] ? strtoull(row[2], nullptr, 10) : 0;
			e.status            = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.attempt_count     = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.granted_at        = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.last_attempt_at   = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.last_error        = row[7] ? row[7] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_task_rewards_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_task_rewards_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CharacterTaskRewards &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.character_id));
		v.push_back(columns[1] + " = " + std::to_string(e.pending_reward_id));
		v.push_back(columns[2] + " = " + std::to_string(e.reward_id));
		v.push_back(columns[3] + " = " + std::to_string(e.status));
		v.push_back(columns[4] + " = " + std::to_string(e.attempt_count));
		v.push_back(columns[5] + " = " + std::to_string(e.granted_at));
		v.push_back(columns[6] + " = " + std::to_string(e.last_attempt_at));
		v.push_back(columns[7] + " = '" + Strings::Escape(e.last_error) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.pending_reward_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterTaskRewards InsertOne(
		Database& db,
		CharacterTaskRewards e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.character_id));
		v.push_back(std::to_string(e.pending_reward_id));
		v.push_back(std::to_string(e.reward_id));
		v.push_back(std::to_string(e.status));
		v.push_back(std::to_string(e.attempt_count));
		v.push_back(std::to_string(e.granted_at));
		v.push_back(std::to_string(e.last_attempt_at));
		v.push_back("'" + Strings::Escape(e.last_error) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.pending_reward_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<CharacterTaskRewards> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.character_id));
			v.push_back(std::to_string(e.pending_reward_id));
			v.push_back(std::to_string(e.reward_id));
			v.push_back(std::to_string(e.status));
			v.push_back(std::to_string(e.attempt_count));
			v.push_back(std::to_string(e.granted_at));
			v.push_back(std::to_string(e.last_attempt_at));
			v.push_back("'" + Strings::Escape(e.last_error) + "'");

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

	static std::vector<CharacterTaskRewards> All(Database& db)
	{
		std::vector<CharacterTaskRewards> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterTaskRewards e{};

			e.character_id      = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.pending_reward_id = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.reward_id         = row[2] ? strtoull(row[2], nullptr, 10) : 0;
			e.status            = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.attempt_count     = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.granted_at        = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.last_attempt_at   = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.last_error        = row[7] ? row[7] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CharacterTaskRewards> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CharacterTaskRewards> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterTaskRewards e{};

			e.character_id      = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.pending_reward_id = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.reward_id         = row[2] ? strtoull(row[2], nullptr, 10) : 0;
			e.status            = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.attempt_count     = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.granted_at        = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.last_attempt_at   = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.last_error        = row[7] ? row[7] : "";

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
		const CharacterTaskRewards &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.character_id));
		v.push_back(std::to_string(e.pending_reward_id));
		v.push_back(std::to_string(e.reward_id));
		v.push_back(std::to_string(e.status));
		v.push_back(std::to_string(e.attempt_count));
		v.push_back(std::to_string(e.granted_at));
		v.push_back(std::to_string(e.last_attempt_at));
		v.push_back("'" + Strings::Escape(e.last_error) + "'");

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
		const std::vector<CharacterTaskRewards> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.character_id));
			v.push_back(std::to_string(e.pending_reward_id));
			v.push_back(std::to_string(e.reward_id));
			v.push_back(std::to_string(e.status));
			v.push_back(std::to_string(e.attempt_count));
			v.push_back(std::to_string(e.granted_at));
			v.push_back(std::to_string(e.last_attempt_at));
			v.push_back("'" + Strings::Escape(e.last_error) + "'");

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

#endif //EQEMU_BASE_CHARACTER_TASK_REWARDS_REPOSITORY_H
