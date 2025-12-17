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

class BaseBugsRepository {
public:
	struct Bugs {
		uint32_t    id;
		std::string zone;
		std::string name;
		std::string ui;
		float       x;
		float       y;
		float       z;
		std::string type;
		uint8_t     flag;
		std::string target;
		std::string bug;
		std::string date;
		uint8_t     status;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"zone",
			"name",
			"ui",
			"x",
			"y",
			"z",
			"type",
			"flag",
			"target",
			"bug",
			"date",
			"status",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"zone",
			"name",
			"ui",
			"x",
			"y",
			"z",
			"type",
			"flag",
			"target",
			"bug",
			"date",
			"status",
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
		return std::string("bugs");
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

	static Bugs NewEntity()
	{
		Bugs e{};

		e.id     = 0;
		e.zone   = "";
		e.name   = "";
		e.ui     = "";
		e.x      = 0;
		e.y      = 0;
		e.z      = 0;
		e.type   = "";
		e.flag   = 0;
		e.target = "";
		e.bug    = "";
		e.date   = 0;
		e.status = 0;

		return e;
	}

	static Bugs GetBugs(
		const std::vector<Bugs> &bugss,
		int bugs_id
	)
	{
		for (auto &bugs : bugss) {
			if (bugs.id == bugs_id) {
				return bugs;
			}
		}

		return NewEntity();
	}

	static Bugs FindOne(
		Database& db,
		int bugs_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				bugs_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Bugs e{};

			e.id     = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.zone   = row[1] ? row[1] : "";
			e.name   = row[2] ? row[2] : "";
			e.ui     = row[3] ? row[3] : "";
			e.x      = row[4] ? strtof(row[4], nullptr) : 0;
			e.y      = row[5] ? strtof(row[5], nullptr) : 0;
			e.z      = row[6] ? strtof(row[6], nullptr) : 0;
			e.type   = row[7] ? row[7] : "";
			e.flag   = row[8] ? static_cast<uint8_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.target = row[9] ? row[9] : "";
			e.bug    = row[10] ? row[10] : "";
			e.date   = row[11] ? row[11] : 0;
			e.status = row[12] ? static_cast<uint8_t>(strtoul(row[12], nullptr, 10)) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int bugs_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				bugs_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Bugs &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.zone) + "'");
		v.push_back(columns[2] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[3] + " = '" + Strings::Escape(e.ui) + "'");
		v.push_back(columns[4] + " = " + std::to_string(e.x));
		v.push_back(columns[5] + " = " + std::to_string(e.y));
		v.push_back(columns[6] + " = " + std::to_string(e.z));
		v.push_back(columns[7] + " = '" + Strings::Escape(e.type) + "'");
		v.push_back(columns[8] + " = " + std::to_string(e.flag));
		v.push_back(columns[9] + " = '" + Strings::Escape(e.target) + "'");
		v.push_back(columns[10] + " = '" + Strings::Escape(e.bug) + "'");
		v.push_back(columns[11] + " = '" + Strings::Escape(e.date) + "'");
		v.push_back(columns[12] + " = " + std::to_string(e.status));

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

	static Bugs InsertOne(
		Database& db,
		Bugs e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.zone) + "'");
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back("'" + Strings::Escape(e.ui) + "'");
		v.push_back(std::to_string(e.x));
		v.push_back(std::to_string(e.y));
		v.push_back(std::to_string(e.z));
		v.push_back("'" + Strings::Escape(e.type) + "'");
		v.push_back(std::to_string(e.flag));
		v.push_back("'" + Strings::Escape(e.target) + "'");
		v.push_back("'" + Strings::Escape(e.bug) + "'");
		v.push_back("'" + Strings::Escape(e.date) + "'");
		v.push_back(std::to_string(e.status));

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
		const std::vector<Bugs> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.zone) + "'");
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back("'" + Strings::Escape(e.ui) + "'");
			v.push_back(std::to_string(e.x));
			v.push_back(std::to_string(e.y));
			v.push_back(std::to_string(e.z));
			v.push_back("'" + Strings::Escape(e.type) + "'");
			v.push_back(std::to_string(e.flag));
			v.push_back("'" + Strings::Escape(e.target) + "'");
			v.push_back("'" + Strings::Escape(e.bug) + "'");
			v.push_back("'" + Strings::Escape(e.date) + "'");
			v.push_back(std::to_string(e.status));

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

	static std::vector<Bugs> All(Database& db)
	{
		std::vector<Bugs> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Bugs e{};

			e.id     = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.zone   = row[1] ? row[1] : "";
			e.name   = row[2] ? row[2] : "";
			e.ui     = row[3] ? row[3] : "";
			e.x      = row[4] ? strtof(row[4], nullptr) : 0;
			e.y      = row[5] ? strtof(row[5], nullptr) : 0;
			e.z      = row[6] ? strtof(row[6], nullptr) : 0;
			e.type   = row[7] ? row[7] : "";
			e.flag   = row[8] ? static_cast<uint8_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.target = row[9] ? row[9] : "";
			e.bug    = row[10] ? row[10] : "";
			e.date   = row[11] ? row[11] : 0;
			e.status = row[12] ? static_cast<uint8_t>(strtoul(row[12], nullptr, 10)) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Bugs> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Bugs> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Bugs e{};

			e.id     = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.zone   = row[1] ? row[1] : "";
			e.name   = row[2] ? row[2] : "";
			e.ui     = row[3] ? row[3] : "";
			e.x      = row[4] ? strtof(row[4], nullptr) : 0;
			e.y      = row[5] ? strtof(row[5], nullptr) : 0;
			e.z      = row[6] ? strtof(row[6], nullptr) : 0;
			e.type   = row[7] ? row[7] : "";
			e.flag   = row[8] ? static_cast<uint8_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.target = row[9] ? row[9] : "";
			e.bug    = row[10] ? row[10] : "";
			e.date   = row[11] ? row[11] : 0;
			e.status = row[12] ? static_cast<uint8_t>(strtoul(row[12], nullptr, 10)) : 0;

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
		const Bugs &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.zone) + "'");
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back("'" + Strings::Escape(e.ui) + "'");
		v.push_back(std::to_string(e.x));
		v.push_back(std::to_string(e.y));
		v.push_back(std::to_string(e.z));
		v.push_back("'" + Strings::Escape(e.type) + "'");
		v.push_back(std::to_string(e.flag));
		v.push_back("'" + Strings::Escape(e.target) + "'");
		v.push_back("'" + Strings::Escape(e.bug) + "'");
		v.push_back("'" + Strings::Escape(e.date) + "'");
		v.push_back(std::to_string(e.status));

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
		const std::vector<Bugs> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.zone) + "'");
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back("'" + Strings::Escape(e.ui) + "'");
			v.push_back(std::to_string(e.x));
			v.push_back(std::to_string(e.y));
			v.push_back(std::to_string(e.z));
			v.push_back("'" + Strings::Escape(e.type) + "'");
			v.push_back(std::to_string(e.flag));
			v.push_back("'" + Strings::Escape(e.target) + "'");
			v.push_back("'" + Strings::Escape(e.bug) + "'");
			v.push_back("'" + Strings::Escape(e.date) + "'");
			v.push_back(std::to_string(e.status));

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
