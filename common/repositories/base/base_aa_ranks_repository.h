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

class BaseAaRanksRepository {
public:
	struct AaRanks {
		uint32_t id;
		int32_t  upper_hotkey_sid;
		int32_t  lower_hotkey_sid;
		int32_t  title_sid;
		int32_t  desc_sid;
		int32_t  cost;
		int32_t  level_req;
		int32_t  spell;
		int32_t  spell_type;
		int32_t  recast_time;
		int32_t  expansion;
		int32_t  prev_id;
		int32_t  next_id;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"upper_hotkey_sid",
			"lower_hotkey_sid",
			"title_sid",
			"desc_sid",
			"cost",
			"level_req",
			"spell",
			"spell_type",
			"recast_time",
			"expansion",
			"prev_id",
			"next_id",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"upper_hotkey_sid",
			"lower_hotkey_sid",
			"title_sid",
			"desc_sid",
			"cost",
			"level_req",
			"spell",
			"spell_type",
			"recast_time",
			"expansion",
			"prev_id",
			"next_id",
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
		return std::string("aa_ranks");
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

	static AaRanks NewEntity()
	{
		AaRanks e{};

		e.id               = 0;
		e.upper_hotkey_sid = -1;
		e.lower_hotkey_sid = -1;
		e.title_sid        = -1;
		e.desc_sid         = -1;
		e.cost             = 1;
		e.level_req        = 51;
		e.spell            = -1;
		e.spell_type       = 0;
		e.recast_time      = 0;
		e.expansion        = 0;
		e.prev_id          = -1;
		e.next_id          = -1;

		return e;
	}

	static AaRanks GetAaRanks(
		const std::vector<AaRanks> &aa_rankss,
		int aa_ranks_id
	)
	{
		for (auto &aa_ranks : aa_rankss) {
			if (aa_ranks.id == aa_ranks_id) {
				return aa_ranks;
			}
		}

		return NewEntity();
	}

	static AaRanks FindOne(
		Database& db,
		int aa_ranks_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				aa_ranks_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			AaRanks e{};

			e.id               = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.upper_hotkey_sid = row[1] ? static_cast<int32_t>(atoi(row[1])) : -1;
			e.lower_hotkey_sid = row[2] ? static_cast<int32_t>(atoi(row[2])) : -1;
			e.title_sid        = row[3] ? static_cast<int32_t>(atoi(row[3])) : -1;
			e.desc_sid         = row[4] ? static_cast<int32_t>(atoi(row[4])) : -1;
			e.cost             = row[5] ? static_cast<int32_t>(atoi(row[5])) : 1;
			e.level_req        = row[6] ? static_cast<int32_t>(atoi(row[6])) : 51;
			e.spell            = row[7] ? static_cast<int32_t>(atoi(row[7])) : -1;
			e.spell_type       = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.recast_time      = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.expansion        = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.prev_id          = row[11] ? static_cast<int32_t>(atoi(row[11])) : -1;
			e.next_id          = row[12] ? static_cast<int32_t>(atoi(row[12])) : -1;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int aa_ranks_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				aa_ranks_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const AaRanks &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.id));
		v.push_back(columns[1] + " = " + std::to_string(e.upper_hotkey_sid));
		v.push_back(columns[2] + " = " + std::to_string(e.lower_hotkey_sid));
		v.push_back(columns[3] + " = " + std::to_string(e.title_sid));
		v.push_back(columns[4] + " = " + std::to_string(e.desc_sid));
		v.push_back(columns[5] + " = " + std::to_string(e.cost));
		v.push_back(columns[6] + " = " + std::to_string(e.level_req));
		v.push_back(columns[7] + " = " + std::to_string(e.spell));
		v.push_back(columns[8] + " = " + std::to_string(e.spell_type));
		v.push_back(columns[9] + " = " + std::to_string(e.recast_time));
		v.push_back(columns[10] + " = " + std::to_string(e.expansion));
		v.push_back(columns[11] + " = " + std::to_string(e.prev_id));
		v.push_back(columns[12] + " = " + std::to_string(e.next_id));

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

	static AaRanks InsertOne(
		Database& db,
		AaRanks e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.upper_hotkey_sid));
		v.push_back(std::to_string(e.lower_hotkey_sid));
		v.push_back(std::to_string(e.title_sid));
		v.push_back(std::to_string(e.desc_sid));
		v.push_back(std::to_string(e.cost));
		v.push_back(std::to_string(e.level_req));
		v.push_back(std::to_string(e.spell));
		v.push_back(std::to_string(e.spell_type));
		v.push_back(std::to_string(e.recast_time));
		v.push_back(std::to_string(e.expansion));
		v.push_back(std::to_string(e.prev_id));
		v.push_back(std::to_string(e.next_id));

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
		const std::vector<AaRanks> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.upper_hotkey_sid));
			v.push_back(std::to_string(e.lower_hotkey_sid));
			v.push_back(std::to_string(e.title_sid));
			v.push_back(std::to_string(e.desc_sid));
			v.push_back(std::to_string(e.cost));
			v.push_back(std::to_string(e.level_req));
			v.push_back(std::to_string(e.spell));
			v.push_back(std::to_string(e.spell_type));
			v.push_back(std::to_string(e.recast_time));
			v.push_back(std::to_string(e.expansion));
			v.push_back(std::to_string(e.prev_id));
			v.push_back(std::to_string(e.next_id));

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

	static std::vector<AaRanks> All(Database& db)
	{
		std::vector<AaRanks> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AaRanks e{};

			e.id               = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.upper_hotkey_sid = row[1] ? static_cast<int32_t>(atoi(row[1])) : -1;
			e.lower_hotkey_sid = row[2] ? static_cast<int32_t>(atoi(row[2])) : -1;
			e.title_sid        = row[3] ? static_cast<int32_t>(atoi(row[3])) : -1;
			e.desc_sid         = row[4] ? static_cast<int32_t>(atoi(row[4])) : -1;
			e.cost             = row[5] ? static_cast<int32_t>(atoi(row[5])) : 1;
			e.level_req        = row[6] ? static_cast<int32_t>(atoi(row[6])) : 51;
			e.spell            = row[7] ? static_cast<int32_t>(atoi(row[7])) : -1;
			e.spell_type       = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.recast_time      = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.expansion        = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.prev_id          = row[11] ? static_cast<int32_t>(atoi(row[11])) : -1;
			e.next_id          = row[12] ? static_cast<int32_t>(atoi(row[12])) : -1;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<AaRanks> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<AaRanks> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AaRanks e{};

			e.id               = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.upper_hotkey_sid = row[1] ? static_cast<int32_t>(atoi(row[1])) : -1;
			e.lower_hotkey_sid = row[2] ? static_cast<int32_t>(atoi(row[2])) : -1;
			e.title_sid        = row[3] ? static_cast<int32_t>(atoi(row[3])) : -1;
			e.desc_sid         = row[4] ? static_cast<int32_t>(atoi(row[4])) : -1;
			e.cost             = row[5] ? static_cast<int32_t>(atoi(row[5])) : 1;
			e.level_req        = row[6] ? static_cast<int32_t>(atoi(row[6])) : 51;
			e.spell            = row[7] ? static_cast<int32_t>(atoi(row[7])) : -1;
			e.spell_type       = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.recast_time      = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.expansion        = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.prev_id          = row[11] ? static_cast<int32_t>(atoi(row[11])) : -1;
			e.next_id          = row[12] ? static_cast<int32_t>(atoi(row[12])) : -1;

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
		const AaRanks &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.upper_hotkey_sid));
		v.push_back(std::to_string(e.lower_hotkey_sid));
		v.push_back(std::to_string(e.title_sid));
		v.push_back(std::to_string(e.desc_sid));
		v.push_back(std::to_string(e.cost));
		v.push_back(std::to_string(e.level_req));
		v.push_back(std::to_string(e.spell));
		v.push_back(std::to_string(e.spell_type));
		v.push_back(std::to_string(e.recast_time));
		v.push_back(std::to_string(e.expansion));
		v.push_back(std::to_string(e.prev_id));
		v.push_back(std::to_string(e.next_id));

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
		const std::vector<AaRanks> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.upper_hotkey_sid));
			v.push_back(std::to_string(e.lower_hotkey_sid));
			v.push_back(std::to_string(e.title_sid));
			v.push_back(std::to_string(e.desc_sid));
			v.push_back(std::to_string(e.cost));
			v.push_back(std::to_string(e.level_req));
			v.push_back(std::to_string(e.spell));
			v.push_back(std::to_string(e.spell_type));
			v.push_back(std::to_string(e.recast_time));
			v.push_back(std::to_string(e.expansion));
			v.push_back(std::to_string(e.prev_id));
			v.push_back(std::to_string(e.next_id));

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
