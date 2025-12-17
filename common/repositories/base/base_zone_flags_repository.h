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

class BaseZoneFlagsRepository {
public:
	struct ZoneFlags {
		int32_t charID;
		int32_t zoneID;
	};

	static std::string PrimaryKey()
	{
		return std::string("charID");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"charID",
			"zoneID",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"charID",
			"zoneID",
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
		return std::string("zone_flags");
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

	static ZoneFlags NewEntity()
	{
		ZoneFlags e{};

		e.charID = 0;
		e.zoneID = 0;

		return e;
	}

	static ZoneFlags GetZoneFlags(
		const std::vector<ZoneFlags> &zone_flagss,
		int zone_flags_id
	)
	{
		for (auto &zone_flags : zone_flagss) {
			if (zone_flags.charID == zone_flags_id) {
				return zone_flags;
			}
		}

		return NewEntity();
	}

	static ZoneFlags FindOne(
		Database& db,
		int zone_flags_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				zone_flags_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			ZoneFlags e{};

			e.charID = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.zoneID = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int zone_flags_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				zone_flags_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const ZoneFlags &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.charID));
		v.push_back(columns[1] + " = " + std::to_string(e.zoneID));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.charID
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static ZoneFlags InsertOne(
		Database& db,
		ZoneFlags e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.charID));
		v.push_back(std::to_string(e.zoneID));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.charID = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<ZoneFlags> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.charID));
			v.push_back(std::to_string(e.zoneID));

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

	static std::vector<ZoneFlags> All(Database& db)
	{
		std::vector<ZoneFlags> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ZoneFlags e{};

			e.charID = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.zoneID = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<ZoneFlags> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<ZoneFlags> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ZoneFlags e{};

			e.charID = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.zoneID = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;

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
		const ZoneFlags &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.charID));
		v.push_back(std::to_string(e.zoneID));

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
		const std::vector<ZoneFlags> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.charID));
			v.push_back(std::to_string(e.zoneID));

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
