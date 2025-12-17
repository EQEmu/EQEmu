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


class BaseMercTypesRepository {
public:
	struct MercTypes {
		uint32_t    merc_type_id;
		uint32_t    race_id;
		uint8_t     proficiency_id;
		std::string dbstring;
		uint32_t    clientversion;
	};

	static std::string PrimaryKey()
	{
		return std::string("merc_type_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"merc_type_id",
			"race_id",
			"proficiency_id",
			"dbstring",
			"clientversion",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"merc_type_id",
			"race_id",
			"proficiency_id",
			"dbstring",
			"clientversion",
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
		return std::string("merc_types");
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

	static MercTypes NewEntity()
	{
		MercTypes e{};

		e.merc_type_id   = 0;
		e.race_id        = 0;
		e.proficiency_id = 0;
		e.dbstring       = "";
		e.clientversion  = 0;

		return e;
	}

	static MercTypes GetMercTypes(
		const std::vector<MercTypes> &merc_typess,
		int merc_types_id
	)
	{
		for (auto &merc_types : merc_typess) {
			if (merc_types.merc_type_id == merc_types_id) {
				return merc_types;
			}
		}

		return NewEntity();
	}

	static MercTypes FindOne(
		Database& db,
		int merc_types_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				merc_types_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			MercTypes e{};

			e.merc_type_id   = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.race_id        = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.proficiency_id = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.dbstring       = row[3] ? row[3] : "";
			e.clientversion  = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int merc_types_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				merc_types_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const MercTypes &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.race_id));
		v.push_back(columns[2] + " = " + std::to_string(e.proficiency_id));
		v.push_back(columns[3] + " = '" + Strings::Escape(e.dbstring) + "'");
		v.push_back(columns[4] + " = " + std::to_string(e.clientversion));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.merc_type_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static MercTypes InsertOne(
		Database& db,
		MercTypes e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.merc_type_id));
		v.push_back(std::to_string(e.race_id));
		v.push_back(std::to_string(e.proficiency_id));
		v.push_back("'" + Strings::Escape(e.dbstring) + "'");
		v.push_back(std::to_string(e.clientversion));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.merc_type_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<MercTypes> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.merc_type_id));
			v.push_back(std::to_string(e.race_id));
			v.push_back(std::to_string(e.proficiency_id));
			v.push_back("'" + Strings::Escape(e.dbstring) + "'");
			v.push_back(std::to_string(e.clientversion));

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

	static std::vector<MercTypes> All(Database& db)
	{
		std::vector<MercTypes> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MercTypes e{};

			e.merc_type_id   = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.race_id        = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.proficiency_id = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.dbstring       = row[3] ? row[3] : "";
			e.clientversion  = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<MercTypes> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<MercTypes> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MercTypes e{};

			e.merc_type_id   = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.race_id        = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.proficiency_id = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.dbstring       = row[3] ? row[3] : "";
			e.clientversion  = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));

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
		const MercTypes &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.merc_type_id));
		v.push_back(std::to_string(e.race_id));
		v.push_back(std::to_string(e.proficiency_id));
		v.push_back("'" + Strings::Escape(e.dbstring) + "'");
		v.push_back(std::to_string(e.clientversion));

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
		const std::vector<MercTypes> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.merc_type_id));
			v.push_back(std::to_string(e.race_id));
			v.push_back(std::to_string(e.proficiency_id));
			v.push_back("'" + Strings::Escape(e.dbstring) + "'");
			v.push_back(std::to_string(e.clientversion));

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
