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

#ifndef EQEMU_BASE_REWARDS_REPOSITORY_H
#define EQEMU_BASE_REWARDS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseRewardsRepository {
public:
	struct Rewards {
		uint32_t    reward_id;
		uint8_t     reward_type;
		uint32_t    reward_data_id;
		uint64_t    amount;
		std::string description;
		uint8_t     enabled;
	};

	static std::string PrimaryKey()
	{
		return std::string("reward_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"reward_id",
			"reward_type",
			"reward_data_id",
			"amount",
			"description",
			"enabled",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"reward_id",
			"reward_type",
			"reward_data_id",
			"amount",
			"description",
			"enabled",
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
		return std::string("rewards");
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

	static Rewards NewEntity()
	{
		Rewards e{};

		e.reward_id      = 0;
		e.reward_type    = 0;
		e.reward_data_id = 0;
		e.amount         = 1;
		e.description    = "";
		e.enabled        = 1;

		return e;
	}

	static Rewards GetRewards(
		const std::vector<Rewards> &rewardss,
		int rewards_id
	)
	{
		for (auto &rewards : rewardss) {
			if (rewards.reward_id == rewards_id) {
				return rewards;
			}
		}

		return NewEntity();
	}

	static Rewards FindOne(
		Database& db,
		int rewards_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				rewards_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Rewards e{};

			e.reward_id      = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.reward_type    = row[1] ? static_cast<uint8_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.reward_data_id = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.amount         = row[3] ? strtoull(row[3], nullptr, 10) : 1;
			e.description    = row[4] ? row[4] : "";
			e.enabled        = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 1;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int rewards_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				rewards_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Rewards &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.reward_type));
		v.push_back(columns[2] + " = " + std::to_string(e.reward_data_id));
		v.push_back(columns[3] + " = " + std::to_string(e.amount));
		v.push_back(columns[4] + " = '" + Strings::Escape(e.description) + "'");
		v.push_back(columns[5] + " = " + std::to_string(e.enabled));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.reward_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Rewards InsertOne(
		Database& db,
		Rewards e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.reward_id));
		v.push_back(std::to_string(e.reward_type));
		v.push_back(std::to_string(e.reward_data_id));
		v.push_back(std::to_string(e.amount));
		v.push_back("'" + Strings::Escape(e.description) + "'");
		v.push_back(std::to_string(e.enabled));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.reward_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<Rewards> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.reward_id));
			v.push_back(std::to_string(e.reward_type));
			v.push_back(std::to_string(e.reward_data_id));
			v.push_back(std::to_string(e.amount));
			v.push_back("'" + Strings::Escape(e.description) + "'");
			v.push_back(std::to_string(e.enabled));

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

	static std::vector<Rewards> All(Database& db)
	{
		std::vector<Rewards> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Rewards e{};

			e.reward_id      = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.reward_type    = row[1] ? static_cast<uint8_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.reward_data_id = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.amount         = row[3] ? strtoull(row[3], nullptr, 10) : 1;
			e.description    = row[4] ? row[4] : "";
			e.enabled        = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 1;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Rewards> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Rewards> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Rewards e{};

			e.reward_id      = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.reward_type    = row[1] ? static_cast<uint8_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.reward_data_id = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.amount         = row[3] ? strtoull(row[3], nullptr, 10) : 1;
			e.description    = row[4] ? row[4] : "";
			e.enabled        = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 1;

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
		const Rewards &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.reward_id));
		v.push_back(std::to_string(e.reward_type));
		v.push_back(std::to_string(e.reward_data_id));
		v.push_back(std::to_string(e.amount));
		v.push_back("'" + Strings::Escape(e.description) + "'");
		v.push_back(std::to_string(e.enabled));

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
		const std::vector<Rewards> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.reward_id));
			v.push_back(std::to_string(e.reward_type));
			v.push_back(std::to_string(e.reward_data_id));
			v.push_back(std::to_string(e.amount));
			v.push_back("'" + Strings::Escape(e.description) + "'");
			v.push_back(std::to_string(e.enabled));

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

#endif //EQEMU_BASE_REWARDS_REPOSITORY_H
