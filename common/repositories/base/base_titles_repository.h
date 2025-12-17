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

class BaseTitlesRepository {
public:
	struct Titles {
		uint32_t    id;
		int8_t      skill_id;
		int32_t     min_skill_value;
		int32_t     max_skill_value;
		int32_t     min_aa_points;
		int32_t     max_aa_points;
		int8_t      class_;
		int8_t      gender;
		int32_t     char_id;
		int32_t     status;
		int32_t     item_id;
		std::string prefix;
		std::string suffix;
		int32_t     title_set;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"skill_id",
			"min_skill_value",
			"max_skill_value",
			"min_aa_points",
			"max_aa_points",
			"`class`",
			"gender",
			"char_id",
			"status",
			"item_id",
			"prefix",
			"suffix",
			"title_set",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"skill_id",
			"min_skill_value",
			"max_skill_value",
			"min_aa_points",
			"max_aa_points",
			"`class`",
			"gender",
			"char_id",
			"status",
			"item_id",
			"prefix",
			"suffix",
			"title_set",
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
		return std::string("titles");
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

	static Titles NewEntity()
	{
		Titles e{};

		e.id              = 0;
		e.skill_id        = -1;
		e.min_skill_value = -1;
		e.max_skill_value = -1;
		e.min_aa_points   = -1;
		e.max_aa_points   = -1;
		e.class_          = -1;
		e.gender          = -1;
		e.char_id         = -1;
		e.status          = -1;
		e.item_id         = -1;
		e.prefix          = "";
		e.suffix          = "";
		e.title_set       = 0;

		return e;
	}

	static Titles GetTitles(
		const std::vector<Titles> &titless,
		int titles_id
	)
	{
		for (auto &titles : titless) {
			if (titles.id == titles_id) {
				return titles;
			}
		}

		return NewEntity();
	}

	static Titles FindOne(
		Database& db,
		int titles_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				titles_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Titles e{};

			e.id              = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.skill_id        = row[1] ? static_cast<int8_t>(atoi(row[1])) : -1;
			e.min_skill_value = row[2] ? static_cast<int32_t>(atoi(row[2])) : -1;
			e.max_skill_value = row[3] ? static_cast<int32_t>(atoi(row[3])) : -1;
			e.min_aa_points   = row[4] ? static_cast<int32_t>(atoi(row[4])) : -1;
			e.max_aa_points   = row[5] ? static_cast<int32_t>(atoi(row[5])) : -1;
			e.class_          = row[6] ? static_cast<int8_t>(atoi(row[6])) : -1;
			e.gender          = row[7] ? static_cast<int8_t>(atoi(row[7])) : -1;
			e.char_id         = row[8] ? static_cast<int32_t>(atoi(row[8])) : -1;
			e.status          = row[9] ? static_cast<int32_t>(atoi(row[9])) : -1;
			e.item_id         = row[10] ? static_cast<int32_t>(atoi(row[10])) : -1;
			e.prefix          = row[11] ? row[11] : "";
			e.suffix          = row[12] ? row[12] : "";
			e.title_set       = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int titles_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				titles_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Titles &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.skill_id));
		v.push_back(columns[2] + " = " + std::to_string(e.min_skill_value));
		v.push_back(columns[3] + " = " + std::to_string(e.max_skill_value));
		v.push_back(columns[4] + " = " + std::to_string(e.min_aa_points));
		v.push_back(columns[5] + " = " + std::to_string(e.max_aa_points));
		v.push_back(columns[6] + " = " + std::to_string(e.class_));
		v.push_back(columns[7] + " = " + std::to_string(e.gender));
		v.push_back(columns[8] + " = " + std::to_string(e.char_id));
		v.push_back(columns[9] + " = " + std::to_string(e.status));
		v.push_back(columns[10] + " = " + std::to_string(e.item_id));
		v.push_back(columns[11] + " = '" + Strings::Escape(e.prefix) + "'");
		v.push_back(columns[12] + " = '" + Strings::Escape(e.suffix) + "'");
		v.push_back(columns[13] + " = " + std::to_string(e.title_set));

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

	static Titles InsertOne(
		Database& db,
		Titles e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.skill_id));
		v.push_back(std::to_string(e.min_skill_value));
		v.push_back(std::to_string(e.max_skill_value));
		v.push_back(std::to_string(e.min_aa_points));
		v.push_back(std::to_string(e.max_aa_points));
		v.push_back(std::to_string(e.class_));
		v.push_back(std::to_string(e.gender));
		v.push_back(std::to_string(e.char_id));
		v.push_back(std::to_string(e.status));
		v.push_back(std::to_string(e.item_id));
		v.push_back("'" + Strings::Escape(e.prefix) + "'");
		v.push_back("'" + Strings::Escape(e.suffix) + "'");
		v.push_back(std::to_string(e.title_set));

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
		const std::vector<Titles> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.skill_id));
			v.push_back(std::to_string(e.min_skill_value));
			v.push_back(std::to_string(e.max_skill_value));
			v.push_back(std::to_string(e.min_aa_points));
			v.push_back(std::to_string(e.max_aa_points));
			v.push_back(std::to_string(e.class_));
			v.push_back(std::to_string(e.gender));
			v.push_back(std::to_string(e.char_id));
			v.push_back(std::to_string(e.status));
			v.push_back(std::to_string(e.item_id));
			v.push_back("'" + Strings::Escape(e.prefix) + "'");
			v.push_back("'" + Strings::Escape(e.suffix) + "'");
			v.push_back(std::to_string(e.title_set));

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

	static std::vector<Titles> All(Database& db)
	{
		std::vector<Titles> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Titles e{};

			e.id              = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.skill_id        = row[1] ? static_cast<int8_t>(atoi(row[1])) : -1;
			e.min_skill_value = row[2] ? static_cast<int32_t>(atoi(row[2])) : -1;
			e.max_skill_value = row[3] ? static_cast<int32_t>(atoi(row[3])) : -1;
			e.min_aa_points   = row[4] ? static_cast<int32_t>(atoi(row[4])) : -1;
			e.max_aa_points   = row[5] ? static_cast<int32_t>(atoi(row[5])) : -1;
			e.class_          = row[6] ? static_cast<int8_t>(atoi(row[6])) : -1;
			e.gender          = row[7] ? static_cast<int8_t>(atoi(row[7])) : -1;
			e.char_id         = row[8] ? static_cast<int32_t>(atoi(row[8])) : -1;
			e.status          = row[9] ? static_cast<int32_t>(atoi(row[9])) : -1;
			e.item_id         = row[10] ? static_cast<int32_t>(atoi(row[10])) : -1;
			e.prefix          = row[11] ? row[11] : "";
			e.suffix          = row[12] ? row[12] : "";
			e.title_set       = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Titles> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Titles> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Titles e{};

			e.id              = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.skill_id        = row[1] ? static_cast<int8_t>(atoi(row[1])) : -1;
			e.min_skill_value = row[2] ? static_cast<int32_t>(atoi(row[2])) : -1;
			e.max_skill_value = row[3] ? static_cast<int32_t>(atoi(row[3])) : -1;
			e.min_aa_points   = row[4] ? static_cast<int32_t>(atoi(row[4])) : -1;
			e.max_aa_points   = row[5] ? static_cast<int32_t>(atoi(row[5])) : -1;
			e.class_          = row[6] ? static_cast<int8_t>(atoi(row[6])) : -1;
			e.gender          = row[7] ? static_cast<int8_t>(atoi(row[7])) : -1;
			e.char_id         = row[8] ? static_cast<int32_t>(atoi(row[8])) : -1;
			e.status          = row[9] ? static_cast<int32_t>(atoi(row[9])) : -1;
			e.item_id         = row[10] ? static_cast<int32_t>(atoi(row[10])) : -1;
			e.prefix          = row[11] ? row[11] : "";
			e.suffix          = row[12] ? row[12] : "";
			e.title_set       = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;

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
		const Titles &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.skill_id));
		v.push_back(std::to_string(e.min_skill_value));
		v.push_back(std::to_string(e.max_skill_value));
		v.push_back(std::to_string(e.min_aa_points));
		v.push_back(std::to_string(e.max_aa_points));
		v.push_back(std::to_string(e.class_));
		v.push_back(std::to_string(e.gender));
		v.push_back(std::to_string(e.char_id));
		v.push_back(std::to_string(e.status));
		v.push_back(std::to_string(e.item_id));
		v.push_back("'" + Strings::Escape(e.prefix) + "'");
		v.push_back("'" + Strings::Escape(e.suffix) + "'");
		v.push_back(std::to_string(e.title_set));

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
		const std::vector<Titles> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.skill_id));
			v.push_back(std::to_string(e.min_skill_value));
			v.push_back(std::to_string(e.max_skill_value));
			v.push_back(std::to_string(e.min_aa_points));
			v.push_back(std::to_string(e.max_aa_points));
			v.push_back(std::to_string(e.class_));
			v.push_back(std::to_string(e.gender));
			v.push_back(std::to_string(e.char_id));
			v.push_back(std::to_string(e.status));
			v.push_back(std::to_string(e.item_id));
			v.push_back("'" + Strings::Escape(e.prefix) + "'");
			v.push_back("'" + Strings::Escape(e.suffix) + "'");
			v.push_back(std::to_string(e.title_set));

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
