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


class BaseMercSubtypesRepository {
public:
	struct MercSubtypes {
		uint32_t merc_subtype_id;
		uint32_t class_id;
		uint8_t  tier_id;
		uint8_t  confidence_id;
	};

	static std::string PrimaryKey()
	{
		return std::string("merc_subtype_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"merc_subtype_id",
			"class_id",
			"tier_id",
			"confidence_id",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"merc_subtype_id",
			"class_id",
			"tier_id",
			"confidence_id",
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
		return std::string("merc_subtypes");
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

	static MercSubtypes NewEntity()
	{
		MercSubtypes e{};

		e.merc_subtype_id = 0;
		e.class_id        = 0;
		e.tier_id         = 0;
		e.confidence_id   = 0;

		return e;
	}

	static MercSubtypes GetMercSubtypes(
		const std::vector<MercSubtypes> &merc_subtypess,
		int merc_subtypes_id
	)
	{
		for (auto &merc_subtypes : merc_subtypess) {
			if (merc_subtypes.merc_subtype_id == merc_subtypes_id) {
				return merc_subtypes;
			}
		}

		return NewEntity();
	}

	static MercSubtypes FindOne(
		Database& db,
		int merc_subtypes_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				merc_subtypes_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			MercSubtypes e{};

			e.merc_subtype_id = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.class_id        = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.tier_id         = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.confidence_id   = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int merc_subtypes_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				merc_subtypes_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const MercSubtypes &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.class_id));
		v.push_back(columns[2] + " = " + std::to_string(e.tier_id));
		v.push_back(columns[3] + " = " + std::to_string(e.confidence_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.merc_subtype_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static MercSubtypes InsertOne(
		Database& db,
		MercSubtypes e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.merc_subtype_id));
		v.push_back(std::to_string(e.class_id));
		v.push_back(std::to_string(e.tier_id));
		v.push_back(std::to_string(e.confidence_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.merc_subtype_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<MercSubtypes> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.merc_subtype_id));
			v.push_back(std::to_string(e.class_id));
			v.push_back(std::to_string(e.tier_id));
			v.push_back(std::to_string(e.confidence_id));

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

	static std::vector<MercSubtypes> All(Database& db)
	{
		std::vector<MercSubtypes> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MercSubtypes e{};

			e.merc_subtype_id = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.class_id        = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.tier_id         = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.confidence_id   = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<MercSubtypes> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<MercSubtypes> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MercSubtypes e{};

			e.merc_subtype_id = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.class_id        = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.tier_id         = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.confidence_id   = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));

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
		const MercSubtypes &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.merc_subtype_id));
		v.push_back(std::to_string(e.class_id));
		v.push_back(std::to_string(e.tier_id));
		v.push_back(std::to_string(e.confidence_id));

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
		const std::vector<MercSubtypes> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.merc_subtype_id));
			v.push_back(std::to_string(e.class_id));
			v.push_back(std::to_string(e.tier_id));
			v.push_back(std::to_string(e.confidence_id));

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
