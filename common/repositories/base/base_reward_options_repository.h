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

#ifndef EQEMU_BASE_REWARD_OPTIONS_REPOSITORY_H
#define EQEMU_BASE_REWARD_OPTIONS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseRewardOptionsRepository {
public:
	struct RewardOptions {
		uint32_t    reward_set_id;
		uint32_t    option_id;
		uint32_t    sequence;
		std::string label;
		uint8_t     common_to_all;
		uint8_t     flags;
		uint8_t     enabled;
	};

	static std::string PrimaryKey()
	{
		return std::string("reward_set_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"reward_set_id",
			"option_id",
			"sequence",
			"label",
			"common_to_all",
			"flags",
			"enabled",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"reward_set_id",
			"option_id",
			"sequence",
			"label",
			"common_to_all",
			"flags",
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
		return std::string("reward_options");
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

	static RewardOptions NewEntity()
	{
		RewardOptions e{};

		e.reward_set_id = 0;
		e.option_id     = 0;
		e.sequence      = 0;
		e.label         = "";
		e.common_to_all = 0;
		e.flags         = 0;
		e.enabled       = 1;

		return e;
	}

	static RewardOptions GetRewardOptions(
		const std::vector<RewardOptions> &reward_optionss,
		int reward_options_id
	)
	{
		for (auto &reward_options : reward_optionss) {
			if (reward_options.reward_set_id == reward_options_id) {
				return reward_options;
			}
		}

		return NewEntity();
	}

	static RewardOptions FindOne(
		Database& db,
		int reward_options_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				reward_options_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			RewardOptions e{};

			e.reward_set_id = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.option_id     = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.sequence      = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.label         = row[3] ? row[3] : "";
			e.common_to_all = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.flags         = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.enabled       = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 1;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int reward_options_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				reward_options_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const RewardOptions &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.reward_set_id));
		v.push_back(columns[1] + " = " + std::to_string(e.option_id));
		v.push_back(columns[2] + " = " + std::to_string(e.sequence));
		v.push_back(columns[3] + " = '" + Strings::Escape(e.label) + "'");
		v.push_back(columns[4] + " = " + std::to_string(e.common_to_all));
		v.push_back(columns[5] + " = " + std::to_string(e.flags));
		v.push_back(columns[6] + " = " + std::to_string(e.enabled));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.reward_set_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static RewardOptions InsertOne(
		Database& db,
		RewardOptions e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.reward_set_id));
		v.push_back(std::to_string(e.option_id));
		v.push_back(std::to_string(e.sequence));
		v.push_back("'" + Strings::Escape(e.label) + "'");
		v.push_back(std::to_string(e.common_to_all));
		v.push_back(std::to_string(e.flags));
		v.push_back(std::to_string(e.enabled));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.reward_set_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<RewardOptions> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.reward_set_id));
			v.push_back(std::to_string(e.option_id));
			v.push_back(std::to_string(e.sequence));
			v.push_back("'" + Strings::Escape(e.label) + "'");
			v.push_back(std::to_string(e.common_to_all));
			v.push_back(std::to_string(e.flags));
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

	static std::vector<RewardOptions> All(Database& db)
	{
		std::vector<RewardOptions> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			RewardOptions e{};

			e.reward_set_id = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.option_id     = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.sequence      = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.label         = row[3] ? row[3] : "";
			e.common_to_all = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.flags         = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.enabled       = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 1;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<RewardOptions> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<RewardOptions> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			RewardOptions e{};

			e.reward_set_id = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.option_id     = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.sequence      = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.label         = row[3] ? row[3] : "";
			e.common_to_all = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.flags         = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.enabled       = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 1;

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
		const RewardOptions &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.reward_set_id));
		v.push_back(std::to_string(e.option_id));
		v.push_back(std::to_string(e.sequence));
		v.push_back("'" + Strings::Escape(e.label) + "'");
		v.push_back(std::to_string(e.common_to_all));
		v.push_back(std::to_string(e.flags));
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
		const std::vector<RewardOptions> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.reward_set_id));
			v.push_back(std::to_string(e.option_id));
			v.push_back(std::to_string(e.sequence));
			v.push_back("'" + Strings::Escape(e.label) + "'");
			v.push_back(std::to_string(e.common_to_all));
			v.push_back(std::to_string(e.flags));
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

#endif //EQEMU_BASE_REWARD_OPTIONS_REPOSITORY_H
