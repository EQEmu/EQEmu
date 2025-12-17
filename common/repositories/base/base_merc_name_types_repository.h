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


class BaseMercNameTypesRepository {
public:
	struct MercNameTypes {
		uint32_t    name_type_id;
		uint32_t    class_id;
		std::string prefix;
		std::string suffix;
	};

	static std::string PrimaryKey()
	{
		return std::string("name_type_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"name_type_id",
			"class_id",
			"prefix",
			"suffix",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"name_type_id",
			"class_id",
			"prefix",
			"suffix",
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
		return std::string("merc_name_types");
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

	static MercNameTypes NewEntity()
	{
		MercNameTypes e{};

		e.name_type_id = 0;
		e.class_id     = 0;
		e.prefix       = "";
		e.suffix       = "";

		return e;
	}

	static MercNameTypes GetMercNameTypes(
		const std::vector<MercNameTypes> &merc_name_typess,
		int merc_name_types_id
	)
	{
		for (auto &merc_name_types : merc_name_typess) {
			if (merc_name_types.name_type_id == merc_name_types_id) {
				return merc_name_types;
			}
		}

		return NewEntity();
	}

	static MercNameTypes FindOne(
		Database& db,
		int merc_name_types_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				merc_name_types_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			MercNameTypes e{};

			e.name_type_id = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.class_id     = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.prefix       = row[2] ? row[2] : "";
			e.suffix       = row[3] ? row[3] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int merc_name_types_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				merc_name_types_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const MercNameTypes &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.name_type_id));
		v.push_back(columns[1] + " = " + std::to_string(e.class_id));
		v.push_back(columns[2] + " = '" + Strings::Escape(e.prefix) + "'");
		v.push_back(columns[3] + " = '" + Strings::Escape(e.suffix) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.name_type_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static MercNameTypes InsertOne(
		Database& db,
		MercNameTypes e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.name_type_id));
		v.push_back(std::to_string(e.class_id));
		v.push_back("'" + Strings::Escape(e.prefix) + "'");
		v.push_back("'" + Strings::Escape(e.suffix) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.name_type_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<MercNameTypes> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.name_type_id));
			v.push_back(std::to_string(e.class_id));
			v.push_back("'" + Strings::Escape(e.prefix) + "'");
			v.push_back("'" + Strings::Escape(e.suffix) + "'");

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

	static std::vector<MercNameTypes> All(Database& db)
	{
		std::vector<MercNameTypes> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MercNameTypes e{};

			e.name_type_id = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.class_id     = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.prefix       = row[2] ? row[2] : "";
			e.suffix       = row[3] ? row[3] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<MercNameTypes> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<MercNameTypes> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MercNameTypes e{};

			e.name_type_id = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.class_id     = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.prefix       = row[2] ? row[2] : "";
			e.suffix       = row[3] ? row[3] : "";

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
		const MercNameTypes &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.name_type_id));
		v.push_back(std::to_string(e.class_id));
		v.push_back("'" + Strings::Escape(e.prefix) + "'");
		v.push_back("'" + Strings::Escape(e.suffix) + "'");

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
		const std::vector<MercNameTypes> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.name_type_id));
			v.push_back(std::to_string(e.class_id));
			v.push_back("'" + Strings::Escape(e.prefix) + "'");
			v.push_back("'" + Strings::Escape(e.suffix) + "'");

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
