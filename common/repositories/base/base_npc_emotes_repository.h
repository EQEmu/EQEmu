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

class BaseNpcEmotesRepository {
public:
	struct NpcEmotes {
		int32_t     id;
		uint32_t    emoteid;
		int8_t      event_;
		int8_t      type;
		std::string text;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"emoteid",
			"event_",
			"type",
			"text",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"emoteid",
			"event_",
			"type",
			"text",
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
		return std::string("npc_emotes");
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

	static NpcEmotes NewEntity()
	{
		NpcEmotes e{};

		e.id      = 0;
		e.emoteid = 0;
		e.event_  = 0;
		e.type    = 0;
		e.text    = "";

		return e;
	}

	static NpcEmotes GetNpcEmotes(
		const std::vector<NpcEmotes> &npc_emotess,
		int npc_emotes_id
	)
	{
		for (auto &npc_emotes : npc_emotess) {
			if (npc_emotes.id == npc_emotes_id) {
				return npc_emotes;
			}
		}

		return NewEntity();
	}

	static NpcEmotes FindOne(
		Database& db,
		int npc_emotes_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				npc_emotes_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			NpcEmotes e{};

			e.id      = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.emoteid = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.event_  = row[2] ? static_cast<int8_t>(atoi(row[2])) : 0;
			e.type    = row[3] ? static_cast<int8_t>(atoi(row[3])) : 0;
			e.text    = row[4] ? row[4] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int npc_emotes_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				npc_emotes_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const NpcEmotes &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.emoteid));
		v.push_back(columns[2] + " = " + std::to_string(e.event_));
		v.push_back(columns[3] + " = " + std::to_string(e.type));
		v.push_back(columns[4] + " = '" + Strings::Escape(e.text) + "'");

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

	static NpcEmotes InsertOne(
		Database& db,
		NpcEmotes e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.emoteid));
		v.push_back(std::to_string(e.event_));
		v.push_back(std::to_string(e.type));
		v.push_back("'" + Strings::Escape(e.text) + "'");

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
		const std::vector<NpcEmotes> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.emoteid));
			v.push_back(std::to_string(e.event_));
			v.push_back(std::to_string(e.type));
			v.push_back("'" + Strings::Escape(e.text) + "'");

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

	static std::vector<NpcEmotes> All(Database& db)
	{
		std::vector<NpcEmotes> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcEmotes e{};

			e.id      = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.emoteid = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.event_  = row[2] ? static_cast<int8_t>(atoi(row[2])) : 0;
			e.type    = row[3] ? static_cast<int8_t>(atoi(row[3])) : 0;
			e.text    = row[4] ? row[4] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<NpcEmotes> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<NpcEmotes> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcEmotes e{};

			e.id      = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.emoteid = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.event_  = row[2] ? static_cast<int8_t>(atoi(row[2])) : 0;
			e.type    = row[3] ? static_cast<int8_t>(atoi(row[3])) : 0;
			e.text    = row[4] ? row[4] : "";

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
		const NpcEmotes &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.emoteid));
		v.push_back(std::to_string(e.event_));
		v.push_back(std::to_string(e.type));
		v.push_back("'" + Strings::Escape(e.text) + "'");

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
		const std::vector<NpcEmotes> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.emoteid));
			v.push_back(std::to_string(e.event_));
			v.push_back(std::to_string(e.type));
			v.push_back("'" + Strings::Escape(e.text) + "'");

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
