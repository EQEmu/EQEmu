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

class BaseGroupLeadersRepository {
public:
	struct GroupLeaders {
		int32_t     gid;
		std::string leadername;
		std::string marknpc;
		std::string leadershipaa;
		std::string maintank;
		std::string assist;
		std::string puller;
		std::string mentoree;
		int32_t     mentor_percent;
	};

	static std::string PrimaryKey()
	{
		return std::string("gid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"gid",
			"leadername",
			"marknpc",
			"leadershipaa",
			"maintank",
			"assist",
			"puller",
			"mentoree",
			"mentor_percent",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"gid",
			"leadername",
			"marknpc",
			"leadershipaa",
			"maintank",
			"assist",
			"puller",
			"mentoree",
			"mentor_percent",
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
		return std::string("group_leaders");
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

	static GroupLeaders NewEntity()
	{
		GroupLeaders e{};

		e.gid            = 0;
		e.leadername     = "";
		e.marknpc        = "";
		e.leadershipaa   = "";
		e.maintank       = "";
		e.assist         = "";
		e.puller         = "";
		e.mentoree       = "";
		e.mentor_percent = 0;

		return e;
	}

	static GroupLeaders GetGroupLeaders(
		const std::vector<GroupLeaders> &group_leaderss,
		int group_leaders_id
	)
	{
		for (auto &group_leaders : group_leaderss) {
			if (group_leaders.gid == group_leaders_id) {
				return group_leaders;
			}
		}

		return NewEntity();
	}

	static GroupLeaders FindOne(
		Database& db,
		int group_leaders_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				group_leaders_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			GroupLeaders e{};

			e.gid            = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.leadername     = row[1] ? row[1] : "";
			e.marknpc        = row[2] ? row[2] : "";
			e.leadershipaa   = row[3] ? row[3] : "";
			e.maintank       = row[4] ? row[4] : "";
			e.assist         = row[5] ? row[5] : "";
			e.puller         = row[6] ? row[6] : "";
			e.mentoree       = row[7] ? row[7] : "";
			e.mentor_percent = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int group_leaders_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				group_leaders_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const GroupLeaders &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.gid));
		v.push_back(columns[1] + " = '" + Strings::Escape(e.leadername) + "'");
		v.push_back(columns[2] + " = '" + Strings::Escape(e.marknpc) + "'");
		v.push_back(columns[3] + " = '" + e.leadershipaa + "'");
		v.push_back(columns[4] + " = '" + Strings::Escape(e.maintank) + "'");
		v.push_back(columns[5] + " = '" + Strings::Escape(e.assist) + "'");
		v.push_back(columns[6] + " = '" + Strings::Escape(e.puller) + "'");
		v.push_back(columns[7] + " = '" + Strings::Escape(e.mentoree) + "'");
		v.push_back(columns[8] + " = " + std::to_string(e.mentor_percent));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.gid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static GroupLeaders InsertOne(
		Database& db,
		GroupLeaders e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.gid));
		v.push_back("'" + Strings::Escape(e.leadername) + "'");
		v.push_back("'" + Strings::Escape(e.marknpc) + "'");
		v.push_back("'" + e.leadershipaa + "'");
		v.push_back("'" + Strings::Escape(e.maintank) + "'");
		v.push_back("'" + Strings::Escape(e.assist) + "'");
		v.push_back("'" + Strings::Escape(e.puller) + "'");
		v.push_back("'" + Strings::Escape(e.mentoree) + "'");
		v.push_back(std::to_string(e.mentor_percent));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.gid = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<GroupLeaders> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.gid));
			v.push_back("'" + Strings::Escape(e.leadername) + "'");
			v.push_back("'" + Strings::Escape(e.marknpc) + "'");
			v.push_back("'" + e.leadershipaa + "'");
			v.push_back("'" + Strings::Escape(e.maintank) + "'");
			v.push_back("'" + Strings::Escape(e.assist) + "'");
			v.push_back("'" + Strings::Escape(e.puller) + "'");
			v.push_back("'" + Strings::Escape(e.mentoree) + "'");
			v.push_back(std::to_string(e.mentor_percent));

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

	static std::vector<GroupLeaders> All(Database& db)
	{
		std::vector<GroupLeaders> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GroupLeaders e{};

			e.gid            = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.leadername     = row[1] ? row[1] : "";
			e.marknpc        = row[2] ? row[2] : "";
			e.leadershipaa   = row[3] ? row[3] : "";
			e.maintank       = row[4] ? row[4] : "";
			e.assist         = row[5] ? row[5] : "";
			e.puller         = row[6] ? row[6] : "";
			e.mentoree       = row[7] ? row[7] : "";
			e.mentor_percent = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<GroupLeaders> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<GroupLeaders> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GroupLeaders e{};

			e.gid            = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.leadername     = row[1] ? row[1] : "";
			e.marknpc        = row[2] ? row[2] : "";
			e.leadershipaa   = row[3] ? row[3] : "";
			e.maintank       = row[4] ? row[4] : "";
			e.assist         = row[5] ? row[5] : "";
			e.puller         = row[6] ? row[6] : "";
			e.mentoree       = row[7] ? row[7] : "";
			e.mentor_percent = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;

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
		const GroupLeaders &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.gid));
		v.push_back("'" + Strings::Escape(e.leadername) + "'");
		v.push_back("'" + Strings::Escape(e.marknpc) + "'");
		v.push_back("'" + e.leadershipaa + "'");
		v.push_back("'" + Strings::Escape(e.maintank) + "'");
		v.push_back("'" + Strings::Escape(e.assist) + "'");
		v.push_back("'" + Strings::Escape(e.puller) + "'");
		v.push_back("'" + Strings::Escape(e.mentoree) + "'");
		v.push_back(std::to_string(e.mentor_percent));

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
		const std::vector<GroupLeaders> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.gid));
			v.push_back("'" + Strings::Escape(e.leadername) + "'");
			v.push_back("'" + Strings::Escape(e.marknpc) + "'");
			v.push_back("'" + e.leadershipaa + "'");
			v.push_back("'" + Strings::Escape(e.maintank) + "'");
			v.push_back("'" + Strings::Escape(e.assist) + "'");
			v.push_back("'" + Strings::Escape(e.puller) + "'");
			v.push_back("'" + Strings::Escape(e.mentoree) + "'");
			v.push_back(std::to_string(e.mentor_percent));

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
