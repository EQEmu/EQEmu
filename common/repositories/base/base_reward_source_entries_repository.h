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

#ifndef EQEMU_BASE_REWARD_SOURCE_ENTRIES_REPOSITORY_H
#define EQEMU_BASE_REWARD_SOURCE_ENTRIES_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseRewardSourceEntriesRepository {
public:
	struct RewardSourceEntries {
		uint8_t  source_type;
		uint64_t source_id;
		uint32_t sequence;
		uint32_t reward_id;
	};

	static std::string PrimaryKey()
	{
		return std::string("source_type");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"source_type",
			"source_id",
			"sequence",
			"reward_id",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"source_type",
			"source_id",
			"sequence",
			"reward_id",
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
		return std::string("reward_source_entries");
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

	static RewardSourceEntries NewEntity()
	{
		RewardSourceEntries e{};

		e.source_type = 0;
		e.source_id   = 0;
		e.sequence    = 0;
		e.reward_id   = 0;

		return e;
	}

	static RewardSourceEntries GetRewardSourceEntries(
		const std::vector<RewardSourceEntries> &reward_source_entriess,
		int reward_source_entries_id
	)
	{
		for (auto &reward_source_entries : reward_source_entriess) {
			if (reward_source_entries.source_type == reward_source_entries_id) {
				return reward_source_entries;
			}
		}

		return NewEntity();
	}

	static RewardSourceEntries FindOne(
		Database& db,
		int reward_source_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				reward_source_entries_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			RewardSourceEntries e{};

			e.source_type = row[0] ? static_cast<uint8_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.source_id   = row[1] ? strtoull(row[1], nullptr, 10) : 0;
			e.sequence    = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.reward_id   = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int reward_source_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				reward_source_entries_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const RewardSourceEntries &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.source_type));
		v.push_back(columns[1] + " = " + std::to_string(e.source_id));
		v.push_back(columns[2] + " = " + std::to_string(e.sequence));
		v.push_back(columns[3] + " = " + std::to_string(e.reward_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.source_type
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static RewardSourceEntries InsertOne(
		Database& db,
		RewardSourceEntries e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.source_type));
		v.push_back(std::to_string(e.source_id));
		v.push_back(std::to_string(e.sequence));
		v.push_back(std::to_string(e.reward_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.source_type = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<RewardSourceEntries> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.source_type));
			v.push_back(std::to_string(e.source_id));
			v.push_back(std::to_string(e.sequence));
			v.push_back(std::to_string(e.reward_id));

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

	static std::vector<RewardSourceEntries> All(Database& db)
	{
		std::vector<RewardSourceEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			RewardSourceEntries e{};

			e.source_type = row[0] ? static_cast<uint8_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.source_id   = row[1] ? strtoull(row[1], nullptr, 10) : 0;
			e.sequence    = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.reward_id   = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<RewardSourceEntries> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<RewardSourceEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			RewardSourceEntries e{};

			e.source_type = row[0] ? static_cast<uint8_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.source_id   = row[1] ? strtoull(row[1], nullptr, 10) : 0;
			e.sequence    = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.reward_id   = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;

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
		const RewardSourceEntries &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.source_type));
		v.push_back(std::to_string(e.source_id));
		v.push_back(std::to_string(e.sequence));
		v.push_back(std::to_string(e.reward_id));

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
		const std::vector<RewardSourceEntries> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.source_type));
			v.push_back(std::to_string(e.source_id));
			v.push_back(std::to_string(e.sequence));
			v.push_back(std::to_string(e.reward_id));

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

#endif //EQEMU_BASE_REWARD_SOURCE_ENTRIES_REPOSITORY_H
