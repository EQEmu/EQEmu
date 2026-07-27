/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
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

class BaseRaidLeadersRepository {
public:
	struct RaidLeaders {
		uint32_t    id;
		uint32_t    gid;
		uint32_t    rid;
		std::string marknpc;
		std::string maintank;
		std::string assist;
		std::string puller;
		std::string masterlooter;
		std::string leadershipaa;
		std::string mentoree;
		int32_t     mentor_percent;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"gid",
			"rid",
			"marknpc",
			"maintank",
			"assist",
			"puller",
			"masterlooter",
			"leadershipaa",
			"mentoree",
			"mentor_percent",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"gid",
			"rid",
			"marknpc",
			"maintank",
			"assist",
			"puller",
			"masterlooter",
			"leadershipaa",
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
		return std::string("raid_leaders");
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

	static RaidLeaders NewEntity()
	{
		RaidLeaders e{};

		e.id             = 0;
		e.gid            = 0;
		e.rid            = 0;
		e.marknpc        = "";
		e.maintank       = "";
		e.assist         = "";
		e.puller         = "";
		e.masterlooter   = "";
		e.leadershipaa   = "";
		e.mentoree       = "";
		e.mentor_percent = 0;

		return e;
	}

	static RaidLeaders GetRaidLeaders(
		const std::vector<RaidLeaders> &raid_leaderss,
		int raid_leaders_id
	)
	{
		for (auto &raid_leaders : raid_leaderss) {
			if (raid_leaders.id == raid_leaders_id) {
				return raid_leaders;
			}
		}

		return NewEntity();
	}

	static RaidLeaders FindOne(
		Database& db,
		int raid_leaders_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				raid_leaders_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			RaidLeaders e{};

			e.id             = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.gid            = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.rid            = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.marknpc        = row[3] ? row[3] : "";
			e.maintank       = row[4] ? row[4] : "";
			e.assist         = row[5] ? row[5] : "";
			e.puller         = row[6] ? row[6] : "";
			e.masterlooter   = row[7] ? row[7] : "";
			e.leadershipaa   = row[8] ? row[8] : "";
			e.mentoree       = row[9] ? row[9] : "";
			e.mentor_percent = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int raid_leaders_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				raid_leaders_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const RaidLeaders &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.gid));
		v.push_back(columns[2] + " = " + std::to_string(e.rid));
		v.push_back(columns[3] + " = '" + Strings::Escape(e.marknpc) + "'");
		v.push_back(columns[4] + " = '" + Strings::Escape(e.maintank) + "'");
		v.push_back(columns[5] + " = '" + Strings::Escape(e.assist) + "'");
		v.push_back(columns[6] + " = '" + Strings::Escape(e.puller) + "'");
		v.push_back(columns[7] + " = '" + Strings::Escape(e.masterlooter) + "'");
		v.push_back(columns[8] + " = '" + e.leadershipaa + "'");
		v.push_back(columns[9] + " = '" + Strings::Escape(e.mentoree) + "'");
		v.push_back(columns[10] + " = " + std::to_string(e.mentor_percent));

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

	static RaidLeaders InsertOne(
		Database& db,
		RaidLeaders e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.gid));
		v.push_back(std::to_string(e.rid));
		v.push_back("'" + Strings::Escape(e.marknpc) + "'");
		v.push_back("'" + Strings::Escape(e.maintank) + "'");
		v.push_back("'" + Strings::Escape(e.assist) + "'");
		v.push_back("'" + Strings::Escape(e.puller) + "'");
		v.push_back("'" + Strings::Escape(e.masterlooter) + "'");
		v.push_back("'" + e.leadershipaa + "'");
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
			e.id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<RaidLeaders> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.gid));
			v.push_back(std::to_string(e.rid));
			v.push_back("'" + Strings::Escape(e.marknpc) + "'");
			v.push_back("'" + Strings::Escape(e.maintank) + "'");
			v.push_back("'" + Strings::Escape(e.assist) + "'");
			v.push_back("'" + Strings::Escape(e.puller) + "'");
			v.push_back("'" + Strings::Escape(e.masterlooter) + "'");
			v.push_back("'" + e.leadershipaa + "'");
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

	static std::vector<RaidLeaders> All(Database& db)
	{
		std::vector<RaidLeaders> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			RaidLeaders e{};

			e.id             = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.gid            = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.rid            = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.marknpc        = row[3] ? row[3] : "";
			e.maintank       = row[4] ? row[4] : "";
			e.assist         = row[5] ? row[5] : "";
			e.puller         = row[6] ? row[6] : "";
			e.masterlooter   = row[7] ? row[7] : "";
			e.leadershipaa   = row[8] ? row[8] : "";
			e.mentoree       = row[9] ? row[9] : "";
			e.mentor_percent = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<RaidLeaders> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<RaidLeaders> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			RaidLeaders e{};

			e.id             = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.gid            = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.rid            = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.marknpc        = row[3] ? row[3] : "";
			e.maintank       = row[4] ? row[4] : "";
			e.assist         = row[5] ? row[5] : "";
			e.puller         = row[6] ? row[6] : "";
			e.masterlooter   = row[7] ? row[7] : "";
			e.leadershipaa   = row[8] ? row[8] : "";
			e.mentoree       = row[9] ? row[9] : "";
			e.mentor_percent = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;

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
		const RaidLeaders &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.gid));
		v.push_back(std::to_string(e.rid));
		v.push_back("'" + Strings::Escape(e.marknpc) + "'");
		v.push_back("'" + Strings::Escape(e.maintank) + "'");
		v.push_back("'" + Strings::Escape(e.assist) + "'");
		v.push_back("'" + Strings::Escape(e.puller) + "'");
		v.push_back("'" + Strings::Escape(e.masterlooter) + "'");
		v.push_back("'" + e.leadershipaa + "'");
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
		const std::vector<RaidLeaders> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.gid));
			v.push_back(std::to_string(e.rid));
			v.push_back("'" + Strings::Escape(e.marknpc) + "'");
			v.push_back("'" + Strings::Escape(e.maintank) + "'");
			v.push_back("'" + Strings::Escape(e.assist) + "'");
			v.push_back("'" + Strings::Escape(e.puller) + "'");
			v.push_back("'" + Strings::Escape(e.masterlooter) + "'");
			v.push_back("'" + e.leadershipaa + "'");
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
