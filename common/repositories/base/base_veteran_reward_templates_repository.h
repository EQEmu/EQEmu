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

class BaseVeteranRewardTemplatesRepository {
public:
	struct VeteranRewardTemplates {
		uint32_t    claim_id;
		std::string name;
		uint32_t    item_id;
		uint16_t    charges;
		uint8_t     reward_slot;
	};

	static std::string PrimaryKey()
	{
		return std::string("claim_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"claim_id",
			"name",
			"item_id",
			"charges",
			"reward_slot",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"claim_id",
			"name",
			"item_id",
			"charges",
			"reward_slot",
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
		return std::string("veteran_reward_templates");
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

	static VeteranRewardTemplates NewEntity()
	{
		VeteranRewardTemplates e{};

		e.claim_id    = 0;
		e.name        = "";
		e.item_id     = 0;
		e.charges     = 0;
		e.reward_slot = 0;

		return e;
	}

	static VeteranRewardTemplates GetVeteranRewardTemplates(
		const std::vector<VeteranRewardTemplates> &veteran_reward_templatess,
		int veteran_reward_templates_id
	)
	{
		for (auto &veteran_reward_templates : veteran_reward_templatess) {
			if (veteran_reward_templates.claim_id == veteran_reward_templates_id) {
				return veteran_reward_templates;
			}
		}

		return NewEntity();
	}

	static VeteranRewardTemplates FindOne(
		Database& db,
		int veteran_reward_templates_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				veteran_reward_templates_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			VeteranRewardTemplates e{};

			e.claim_id    = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.name        = row[1] ? row[1] : "";
			e.item_id     = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.charges     = row[3] ? static_cast<uint16_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.reward_slot = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int veteran_reward_templates_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				veteran_reward_templates_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const VeteranRewardTemplates &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.claim_id));
		v.push_back(columns[1] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[2] + " = " + std::to_string(e.item_id));
		v.push_back(columns[3] + " = " + std::to_string(e.charges));
		v.push_back(columns[4] + " = " + std::to_string(e.reward_slot));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.claim_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static VeteranRewardTemplates InsertOne(
		Database& db,
		VeteranRewardTemplates e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.claim_id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.charges));
		v.push_back(std::to_string(e.reward_slot));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.claim_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<VeteranRewardTemplates> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.claim_id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.charges));
			v.push_back(std::to_string(e.reward_slot));

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

	static std::vector<VeteranRewardTemplates> All(Database& db)
	{
		std::vector<VeteranRewardTemplates> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			VeteranRewardTemplates e{};

			e.claim_id    = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.name        = row[1] ? row[1] : "";
			e.item_id     = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.charges     = row[3] ? static_cast<uint16_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.reward_slot = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<VeteranRewardTemplates> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<VeteranRewardTemplates> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			VeteranRewardTemplates e{};

			e.claim_id    = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.name        = row[1] ? row[1] : "";
			e.item_id     = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.charges     = row[3] ? static_cast<uint16_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.reward_slot = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;

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
		const VeteranRewardTemplates &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.claim_id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.charges));
		v.push_back(std::to_string(e.reward_slot));

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
		const std::vector<VeteranRewardTemplates> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.claim_id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.charges));
			v.push_back(std::to_string(e.reward_slot));

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
