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

class BaseChatchannelsRepository {
public:
	struct Chatchannels {
		int32_t     id;
		std::string name;
		std::string owner;
		std::string password;
		int32_t     minstatus;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"name",
			"owner",
			"password",
			"minstatus",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"name",
			"owner",
			"password",
			"minstatus",
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
		return std::string("chatchannels");
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

	static Chatchannels NewEntity()
	{
		Chatchannels e{};

		e.id        = 0;
		e.name      = "";
		e.owner     = "";
		e.password  = "";
		e.minstatus = 0;

		return e;
	}

	static Chatchannels GetChatchannels(
		const std::vector<Chatchannels> &chatchannelss,
		int chatchannels_id
	)
	{
		for (auto &chatchannels : chatchannelss) {
			if (chatchannels.id == chatchannels_id) {
				return chatchannels;
			}
		}

		return NewEntity();
	}

	static Chatchannels FindOne(
		Database& db,
		int chatchannels_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				chatchannels_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Chatchannels e{};

			e.id        = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.name      = row[1] ? row[1] : "";
			e.owner     = row[2] ? row[2] : "";
			e.password  = row[3] ? row[3] : "";
			e.minstatus = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int chatchannels_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				chatchannels_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Chatchannels &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[2] + " = '" + Strings::Escape(e.owner) + "'");
		v.push_back(columns[3] + " = '" + Strings::Escape(e.password) + "'");
		v.push_back(columns[4] + " = " + std::to_string(e.minstatus));

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

	static Chatchannels InsertOne(
		Database& db,
		Chatchannels e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back("'" + Strings::Escape(e.owner) + "'");
		v.push_back("'" + Strings::Escape(e.password) + "'");
		v.push_back(std::to_string(e.minstatus));

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
		const std::vector<Chatchannels> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back("'" + Strings::Escape(e.owner) + "'");
			v.push_back("'" + Strings::Escape(e.password) + "'");
			v.push_back(std::to_string(e.minstatus));

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

	static std::vector<Chatchannels> All(Database& db)
	{
		std::vector<Chatchannels> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Chatchannels e{};

			e.id        = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.name      = row[1] ? row[1] : "";
			e.owner     = row[2] ? row[2] : "";
			e.password  = row[3] ? row[3] : "";
			e.minstatus = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Chatchannels> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Chatchannels> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Chatchannels e{};

			e.id        = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.name      = row[1] ? row[1] : "";
			e.owner     = row[2] ? row[2] : "";
			e.password  = row[3] ? row[3] : "";
			e.minstatus = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;

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
		const Chatchannels &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back("'" + Strings::Escape(e.owner) + "'");
		v.push_back("'" + Strings::Escape(e.password) + "'");
		v.push_back(std::to_string(e.minstatus));

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
		const std::vector<Chatchannels> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back("'" + Strings::Escape(e.owner) + "'");
			v.push_back("'" + Strings::Escape(e.password) + "'");
			v.push_back(std::to_string(e.minstatus));

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
