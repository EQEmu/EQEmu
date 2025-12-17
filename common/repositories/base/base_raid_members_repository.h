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

class BaseRaidMembersRepository {
public:
	struct RaidMembers {
		uint64_t    id;
		int32_t     raidid;
		int32_t     charid;
		int32_t     bot_id;
		uint32_t    groupid;
		int8_t      _class;
		int8_t      level;
		std::string name;
		int8_t      isgroupleader;
		int8_t      israidleader;
		int8_t      islooter;
		uint8_t     is_marker;
		uint8_t     is_assister;
		std::string note;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"raidid",
			"charid",
			"bot_id",
			"groupid",
			"_class",
			"level",
			"name",
			"isgroupleader",
			"israidleader",
			"islooter",
			"is_marker",
			"is_assister",
			"note",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"raidid",
			"charid",
			"bot_id",
			"groupid",
			"_class",
			"level",
			"name",
			"isgroupleader",
			"israidleader",
			"islooter",
			"is_marker",
			"is_assister",
			"note",
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
		return std::string("raid_members");
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

	static RaidMembers NewEntity()
	{
		RaidMembers e{};

		e.id            = 0;
		e.raidid        = 0;
		e.charid        = 0;
		e.bot_id        = 0;
		e.groupid       = 0;
		e._class        = 0;
		e.level         = 0;
		e.name          = "";
		e.isgroupleader = 0;
		e.israidleader  = 0;
		e.islooter      = 0;
		e.is_marker     = 0;
		e.is_assister   = 0;
		e.note          = "";

		return e;
	}

	static RaidMembers GetRaidMembers(
		const std::vector<RaidMembers> &raid_memberss,
		int raid_members_id
	)
	{
		for (auto &raid_members : raid_memberss) {
			if (raid_members.id == raid_members_id) {
				return raid_members;
			}
		}

		return NewEntity();
	}

	static RaidMembers FindOne(
		Database& db,
		int raid_members_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				raid_members_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			RaidMembers e{};

			e.id            = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.raidid        = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.charid        = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.bot_id        = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.groupid       = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e._class        = row[5] ? static_cast<int8_t>(atoi(row[5])) : 0;
			e.level         = row[6] ? static_cast<int8_t>(atoi(row[6])) : 0;
			e.name          = row[7] ? row[7] : "";
			e.isgroupleader = row[8] ? static_cast<int8_t>(atoi(row[8])) : 0;
			e.israidleader  = row[9] ? static_cast<int8_t>(atoi(row[9])) : 0;
			e.islooter      = row[10] ? static_cast<int8_t>(atoi(row[10])) : 0;
			e.is_marker     = row[11] ? static_cast<uint8_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.is_assister   = row[12] ? static_cast<uint8_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.note          = row[13] ? row[13] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int raid_members_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				raid_members_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const RaidMembers &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.raidid));
		v.push_back(columns[2] + " = " + std::to_string(e.charid));
		v.push_back(columns[3] + " = " + std::to_string(e.bot_id));
		v.push_back(columns[4] + " = " + std::to_string(e.groupid));
		v.push_back(columns[5] + " = " + std::to_string(e._class));
		v.push_back(columns[6] + " = " + std::to_string(e.level));
		v.push_back(columns[7] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[8] + " = " + std::to_string(e.isgroupleader));
		v.push_back(columns[9] + " = " + std::to_string(e.israidleader));
		v.push_back(columns[10] + " = " + std::to_string(e.islooter));
		v.push_back(columns[11] + " = " + std::to_string(e.is_marker));
		v.push_back(columns[12] + " = " + std::to_string(e.is_assister));
		v.push_back(columns[13] + " = '" + Strings::Escape(e.note) + "'");

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

	static RaidMembers InsertOne(
		Database& db,
		RaidMembers e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.raidid));
		v.push_back(std::to_string(e.charid));
		v.push_back(std::to_string(e.bot_id));
		v.push_back(std::to_string(e.groupid));
		v.push_back(std::to_string(e._class));
		v.push_back(std::to_string(e.level));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.isgroupleader));
		v.push_back(std::to_string(e.israidleader));
		v.push_back(std::to_string(e.islooter));
		v.push_back(std::to_string(e.is_marker));
		v.push_back(std::to_string(e.is_assister));
		v.push_back("'" + Strings::Escape(e.note) + "'");

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
		const std::vector<RaidMembers> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.raidid));
			v.push_back(std::to_string(e.charid));
			v.push_back(std::to_string(e.bot_id));
			v.push_back(std::to_string(e.groupid));
			v.push_back(std::to_string(e._class));
			v.push_back(std::to_string(e.level));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.isgroupleader));
			v.push_back(std::to_string(e.israidleader));
			v.push_back(std::to_string(e.islooter));
			v.push_back(std::to_string(e.is_marker));
			v.push_back(std::to_string(e.is_assister));
			v.push_back("'" + Strings::Escape(e.note) + "'");

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

	static std::vector<RaidMembers> All(Database& db)
	{
		std::vector<RaidMembers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			RaidMembers e{};

			e.id            = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.raidid        = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.charid        = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.bot_id        = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.groupid       = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e._class        = row[5] ? static_cast<int8_t>(atoi(row[5])) : 0;
			e.level         = row[6] ? static_cast<int8_t>(atoi(row[6])) : 0;
			e.name          = row[7] ? row[7] : "";
			e.isgroupleader = row[8] ? static_cast<int8_t>(atoi(row[8])) : 0;
			e.israidleader  = row[9] ? static_cast<int8_t>(atoi(row[9])) : 0;
			e.islooter      = row[10] ? static_cast<int8_t>(atoi(row[10])) : 0;
			e.is_marker     = row[11] ? static_cast<uint8_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.is_assister   = row[12] ? static_cast<uint8_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.note          = row[13] ? row[13] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<RaidMembers> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<RaidMembers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			RaidMembers e{};

			e.id            = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.raidid        = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.charid        = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.bot_id        = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.groupid       = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e._class        = row[5] ? static_cast<int8_t>(atoi(row[5])) : 0;
			e.level         = row[6] ? static_cast<int8_t>(atoi(row[6])) : 0;
			e.name          = row[7] ? row[7] : "";
			e.isgroupleader = row[8] ? static_cast<int8_t>(atoi(row[8])) : 0;
			e.israidleader  = row[9] ? static_cast<int8_t>(atoi(row[9])) : 0;
			e.islooter      = row[10] ? static_cast<int8_t>(atoi(row[10])) : 0;
			e.is_marker     = row[11] ? static_cast<uint8_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.is_assister   = row[12] ? static_cast<uint8_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.note          = row[13] ? row[13] : "";

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
		const RaidMembers &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.raidid));
		v.push_back(std::to_string(e.charid));
		v.push_back(std::to_string(e.bot_id));
		v.push_back(std::to_string(e.groupid));
		v.push_back(std::to_string(e._class));
		v.push_back(std::to_string(e.level));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.isgroupleader));
		v.push_back(std::to_string(e.israidleader));
		v.push_back(std::to_string(e.islooter));
		v.push_back(std::to_string(e.is_marker));
		v.push_back(std::to_string(e.is_assister));
		v.push_back("'" + Strings::Escape(e.note) + "'");

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
		const std::vector<RaidMembers> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.raidid));
			v.push_back(std::to_string(e.charid));
			v.push_back(std::to_string(e.bot_id));
			v.push_back(std::to_string(e.groupid));
			v.push_back(std::to_string(e._class));
			v.push_back(std::to_string(e.level));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.isgroupleader));
			v.push_back(std::to_string(e.israidleader));
			v.push_back(std::to_string(e.islooter));
			v.push_back(std::to_string(e.is_marker));
			v.push_back(std::to_string(e.is_assister));
			v.push_back("'" + Strings::Escape(e.note) + "'");

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
