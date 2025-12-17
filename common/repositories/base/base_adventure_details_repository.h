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

class BaseAdventureDetailsRepository {
public:
	struct AdventureDetails {
		uint32_t id;
		uint16_t adventure_id;
		int32_t  instance_id;
		uint16_t count;
		uint16_t assassinate_count;
		uint8_t  status;
		uint32_t time_created;
		uint32_t time_zoned;
		uint32_t time_completed;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"adventure_id",
			"instance_id",
			"count",
			"assassinate_count",
			"status",
			"time_created",
			"time_zoned",
			"time_completed",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"adventure_id",
			"instance_id",
			"count",
			"assassinate_count",
			"status",
			"time_created",
			"time_zoned",
			"time_completed",
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
		return std::string("adventure_details");
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

	static AdventureDetails NewEntity()
	{
		AdventureDetails e{};

		e.id                = 0;
		e.adventure_id      = 0;
		e.instance_id       = -1;
		e.count             = 0;
		e.assassinate_count = 0;
		e.status            = 0;
		e.time_created      = 0;
		e.time_zoned        = 0;
		e.time_completed    = 0;

		return e;
	}

	static AdventureDetails GetAdventureDetails(
		const std::vector<AdventureDetails> &adventure_detailss,
		int adventure_details_id
	)
	{
		for (auto &adventure_details : adventure_detailss) {
			if (adventure_details.id == adventure_details_id) {
				return adventure_details;
			}
		}

		return NewEntity();
	}

	static AdventureDetails FindOne(
		Database& db,
		int adventure_details_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				adventure_details_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			AdventureDetails e{};

			e.id                = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.adventure_id      = row[1] ? static_cast<uint16_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.instance_id       = row[2] ? static_cast<int32_t>(atoi(row[2])) : -1;
			e.count             = row[3] ? static_cast<uint16_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.assassinate_count = row[4] ? static_cast<uint16_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.status            = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.time_created      = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.time_zoned        = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.time_completed    = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int adventure_details_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				adventure_details_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const AdventureDetails &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.adventure_id));
		v.push_back(columns[2] + " = " + std::to_string(e.instance_id));
		v.push_back(columns[3] + " = " + std::to_string(e.count));
		v.push_back(columns[4] + " = " + std::to_string(e.assassinate_count));
		v.push_back(columns[5] + " = " + std::to_string(e.status));
		v.push_back(columns[6] + " = " + std::to_string(e.time_created));
		v.push_back(columns[7] + " = " + std::to_string(e.time_zoned));
		v.push_back(columns[8] + " = " + std::to_string(e.time_completed));

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

	static AdventureDetails InsertOne(
		Database& db,
		AdventureDetails e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.adventure_id));
		v.push_back(std::to_string(e.instance_id));
		v.push_back(std::to_string(e.count));
		v.push_back(std::to_string(e.assassinate_count));
		v.push_back(std::to_string(e.status));
		v.push_back(std::to_string(e.time_created));
		v.push_back(std::to_string(e.time_zoned));
		v.push_back(std::to_string(e.time_completed));

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
		const std::vector<AdventureDetails> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.adventure_id));
			v.push_back(std::to_string(e.instance_id));
			v.push_back(std::to_string(e.count));
			v.push_back(std::to_string(e.assassinate_count));
			v.push_back(std::to_string(e.status));
			v.push_back(std::to_string(e.time_created));
			v.push_back(std::to_string(e.time_zoned));
			v.push_back(std::to_string(e.time_completed));

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

	static std::vector<AdventureDetails> All(Database& db)
	{
		std::vector<AdventureDetails> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AdventureDetails e{};

			e.id                = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.adventure_id      = row[1] ? static_cast<uint16_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.instance_id       = row[2] ? static_cast<int32_t>(atoi(row[2])) : -1;
			e.count             = row[3] ? static_cast<uint16_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.assassinate_count = row[4] ? static_cast<uint16_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.status            = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.time_created      = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.time_zoned        = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.time_completed    = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<AdventureDetails> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<AdventureDetails> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AdventureDetails e{};

			e.id                = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.adventure_id      = row[1] ? static_cast<uint16_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.instance_id       = row[2] ? static_cast<int32_t>(atoi(row[2])) : -1;
			e.count             = row[3] ? static_cast<uint16_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.assassinate_count = row[4] ? static_cast<uint16_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.status            = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.time_created      = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.time_zoned        = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.time_completed    = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;

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
		const AdventureDetails &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.adventure_id));
		v.push_back(std::to_string(e.instance_id));
		v.push_back(std::to_string(e.count));
		v.push_back(std::to_string(e.assassinate_count));
		v.push_back(std::to_string(e.status));
		v.push_back(std::to_string(e.time_created));
		v.push_back(std::to_string(e.time_zoned));
		v.push_back(std::to_string(e.time_completed));

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
		const std::vector<AdventureDetails> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.adventure_id));
			v.push_back(std::to_string(e.instance_id));
			v.push_back(std::to_string(e.count));
			v.push_back(std::to_string(e.assassinate_count));
			v.push_back(std::to_string(e.status));
			v.push_back(std::to_string(e.time_created));
			v.push_back(std::to_string(e.time_zoned));
			v.push_back(std::to_string(e.time_completed));

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
