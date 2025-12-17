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

class BaseQuestGlobalsRepository {
public:
	struct QuestGlobals {
		int32_t     charid;
		int32_t     npcid;
		int32_t     zoneid;
		std::string name;
		std::string value;
		int32_t     expdate;
	};

	static std::string PrimaryKey()
	{
		return std::string("charid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"charid",
			"npcid",
			"zoneid",
			"name",
			"value",
			"expdate",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"charid",
			"npcid",
			"zoneid",
			"name",
			"value",
			"expdate",
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
		return std::string("quest_globals");
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

	static QuestGlobals NewEntity()
	{
		QuestGlobals e{};

		e.charid  = 0;
		e.npcid   = 0;
		e.zoneid  = 0;
		e.name    = "";
		e.value   = "?";
		e.expdate = 0;

		return e;
	}

	static QuestGlobals GetQuestGlobals(
		const std::vector<QuestGlobals> &quest_globalss,
		int quest_globals_id
	)
	{
		for (auto &quest_globals : quest_globalss) {
			if (quest_globals.charid == quest_globals_id) {
				return quest_globals;
			}
		}

		return NewEntity();
	}

	static QuestGlobals FindOne(
		Database& db,
		int quest_globals_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				quest_globals_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			QuestGlobals e{};

			e.charid  = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.npcid   = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.zoneid  = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.name    = row[3] ? row[3] : "";
			e.value   = row[4] ? row[4] : "?";
			e.expdate = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int quest_globals_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				quest_globals_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const QuestGlobals &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.charid));
		v.push_back(columns[1] + " = " + std::to_string(e.npcid));
		v.push_back(columns[2] + " = " + std::to_string(e.zoneid));
		v.push_back(columns[3] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[4] + " = '" + Strings::Escape(e.value) + "'");
		v.push_back(columns[5] + " = " + std::to_string(e.expdate));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.charid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static QuestGlobals InsertOne(
		Database& db,
		QuestGlobals e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.charid));
		v.push_back(std::to_string(e.npcid));
		v.push_back(std::to_string(e.zoneid));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back("'" + Strings::Escape(e.value) + "'");
		v.push_back(std::to_string(e.expdate));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.charid = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<QuestGlobals> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.charid));
			v.push_back(std::to_string(e.npcid));
			v.push_back(std::to_string(e.zoneid));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back("'" + Strings::Escape(e.value) + "'");
			v.push_back(std::to_string(e.expdate));

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

	static std::vector<QuestGlobals> All(Database& db)
	{
		std::vector<QuestGlobals> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			QuestGlobals e{};

			e.charid  = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.npcid   = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.zoneid  = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.name    = row[3] ? row[3] : "";
			e.value   = row[4] ? row[4] : "?";
			e.expdate = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<QuestGlobals> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<QuestGlobals> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			QuestGlobals e{};

			e.charid  = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.npcid   = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.zoneid  = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.name    = row[3] ? row[3] : "";
			e.value   = row[4] ? row[4] : "?";
			e.expdate = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;

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
		const QuestGlobals &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.charid));
		v.push_back(std::to_string(e.npcid));
		v.push_back(std::to_string(e.zoneid));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back("'" + Strings::Escape(e.value) + "'");
		v.push_back(std::to_string(e.expdate));

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
		const std::vector<QuestGlobals> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.charid));
			v.push_back(std::to_string(e.npcid));
			v.push_back(std::to_string(e.zoneid));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back("'" + Strings::Escape(e.value) + "'");
			v.push_back(std::to_string(e.expdate));

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
