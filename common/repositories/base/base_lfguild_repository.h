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

class BaseLfguildRepository {
public:
	struct Lfguild {
		uint8_t     type;
		std::string name;
		std::string comment;
		uint8_t     fromlevel;
		uint8_t     tolevel;
		uint32_t    classes;
		uint32_t    aacount;
		uint32_t    timezone;
		uint32_t    timeposted;
	};

	static std::string PrimaryKey()
	{
		return std::string("type");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"type",
			"name",
			"comment",
			"fromlevel",
			"tolevel",
			"classes",
			"aacount",
			"timezone",
			"timeposted",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"type",
			"name",
			"comment",
			"fromlevel",
			"tolevel",
			"classes",
			"aacount",
			"timezone",
			"timeposted",
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
		return std::string("lfguild");
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

	static Lfguild NewEntity()
	{
		Lfguild e{};

		e.type       = 0;
		e.name       = "";
		e.comment    = "";
		e.fromlevel  = 0;
		e.tolevel    = 0;
		e.classes    = 0;
		e.aacount    = 0;
		e.timezone   = 0;
		e.timeposted = 0;

		return e;
	}

	static Lfguild GetLfguild(
		const std::vector<Lfguild> &lfguilds,
		int lfguild_id
	)
	{
		for (auto &lfguild : lfguilds) {
			if (lfguild.type == lfguild_id) {
				return lfguild;
			}
		}

		return NewEntity();
	}

	static Lfguild FindOne(
		Database& db,
		int lfguild_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				lfguild_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Lfguild e{};

			e.type       = row[0] ? static_cast<uint8_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.name       = row[1] ? row[1] : "";
			e.comment    = row[2] ? row[2] : "";
			e.fromlevel  = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.tolevel    = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.classes    = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.aacount    = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.timezone   = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.timeposted = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int lfguild_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				lfguild_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Lfguild &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.type));
		v.push_back(columns[1] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[2] + " = '" + Strings::Escape(e.comment) + "'");
		v.push_back(columns[3] + " = " + std::to_string(e.fromlevel));
		v.push_back(columns[4] + " = " + std::to_string(e.tolevel));
		v.push_back(columns[5] + " = " + std::to_string(e.classes));
		v.push_back(columns[6] + " = " + std::to_string(e.aacount));
		v.push_back(columns[7] + " = " + std::to_string(e.timezone));
		v.push_back(columns[8] + " = " + std::to_string(e.timeposted));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.type
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Lfguild InsertOne(
		Database& db,
		Lfguild e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.type));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back("'" + Strings::Escape(e.comment) + "'");
		v.push_back(std::to_string(e.fromlevel));
		v.push_back(std::to_string(e.tolevel));
		v.push_back(std::to_string(e.classes));
		v.push_back(std::to_string(e.aacount));
		v.push_back(std::to_string(e.timezone));
		v.push_back(std::to_string(e.timeposted));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.type = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<Lfguild> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.type));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back("'" + Strings::Escape(e.comment) + "'");
			v.push_back(std::to_string(e.fromlevel));
			v.push_back(std::to_string(e.tolevel));
			v.push_back(std::to_string(e.classes));
			v.push_back(std::to_string(e.aacount));
			v.push_back(std::to_string(e.timezone));
			v.push_back(std::to_string(e.timeposted));

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

	static std::vector<Lfguild> All(Database& db)
	{
		std::vector<Lfguild> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Lfguild e{};

			e.type       = row[0] ? static_cast<uint8_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.name       = row[1] ? row[1] : "";
			e.comment    = row[2] ? row[2] : "";
			e.fromlevel  = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.tolevel    = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.classes    = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.aacount    = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.timezone   = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.timeposted = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Lfguild> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Lfguild> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Lfguild e{};

			e.type       = row[0] ? static_cast<uint8_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.name       = row[1] ? row[1] : "";
			e.comment    = row[2] ? row[2] : "";
			e.fromlevel  = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.tolevel    = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.classes    = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.aacount    = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.timezone   = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.timeposted = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;

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
		const Lfguild &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.type));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back("'" + Strings::Escape(e.comment) + "'");
		v.push_back(std::to_string(e.fromlevel));
		v.push_back(std::to_string(e.tolevel));
		v.push_back(std::to_string(e.classes));
		v.push_back(std::to_string(e.aacount));
		v.push_back(std::to_string(e.timezone));
		v.push_back(std::to_string(e.timeposted));

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
		const std::vector<Lfguild> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.type));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back("'" + Strings::Escape(e.comment) + "'");
			v.push_back(std::to_string(e.fromlevel));
			v.push_back(std::to_string(e.tolevel));
			v.push_back(std::to_string(e.classes));
			v.push_back(std::to_string(e.aacount));
			v.push_back(std::to_string(e.timezone));
			v.push_back(std::to_string(e.timeposted));

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
