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

class BaseObjectContentsRepository {
public:
	struct ObjectContents {
		uint32_t zoneid;
		uint32_t parentid;
		uint32_t bagidx;
		uint32_t itemid;
		int16_t  charges;
		time_t   droptime;
		uint32_t augslot1;
		uint32_t augslot2;
		uint32_t augslot3;
		uint32_t augslot4;
		uint32_t augslot5;
		int32_t  augslot6;
	};

	static std::string PrimaryKey()
	{
		return std::string("parentid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"zoneid",
			"parentid",
			"bagidx",
			"itemid",
			"charges",
			"droptime",
			"augslot1",
			"augslot2",
			"augslot3",
			"augslot4",
			"augslot5",
			"augslot6",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"zoneid",
			"parentid",
			"bagidx",
			"itemid",
			"charges",
			"UNIX_TIMESTAMP(droptime)",
			"augslot1",
			"augslot2",
			"augslot3",
			"augslot4",
			"augslot5",
			"augslot6",
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
		return std::string("object_contents");
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

	static ObjectContents NewEntity()
	{
		ObjectContents e{};

		e.zoneid   = 0;
		e.parentid = 0;
		e.bagidx   = 0;
		e.itemid   = 0;
		e.charges  = 0;
		e.droptime = std::time(nullptr);
		e.augslot1 = 0;
		e.augslot2 = 0;
		e.augslot3 = 0;
		e.augslot4 = 0;
		e.augslot5 = 0;
		e.augslot6 = 0;

		return e;
	}

	static ObjectContents GetObjectContents(
		const std::vector<ObjectContents> &object_contentss,
		int object_contents_id
	)
	{
		for (auto &object_contents : object_contentss) {
			if (object_contents.parentid == object_contents_id) {
				return object_contents;
			}
		}

		return NewEntity();
	}

	static ObjectContents FindOne(
		Database& db,
		int object_contents_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				object_contents_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			ObjectContents e{};

			e.zoneid   = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.parentid = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.bagidx   = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.itemid   = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.charges  = row[4] ? static_cast<int16_t>(atoi(row[4])) : 0;
			e.droptime = strtoll(row[5] ? row[5] : "-1", nullptr, 10);
			e.augslot1 = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.augslot2 = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.augslot3 = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.augslot4 = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.augslot5 = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.augslot6 = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int object_contents_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				object_contents_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const ObjectContents &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.zoneid));
		v.push_back(columns[1] + " = " + std::to_string(e.parentid));
		v.push_back(columns[2] + " = " + std::to_string(e.bagidx));
		v.push_back(columns[3] + " = " + std::to_string(e.itemid));
		v.push_back(columns[4] + " = " + std::to_string(e.charges));
		v.push_back(columns[5] + " = FROM_UNIXTIME(" + (e.droptime > 0 ? std::to_string(e.droptime) : "null") + ")");
		v.push_back(columns[6] + " = " + std::to_string(e.augslot1));
		v.push_back(columns[7] + " = " + std::to_string(e.augslot2));
		v.push_back(columns[8] + " = " + std::to_string(e.augslot3));
		v.push_back(columns[9] + " = " + std::to_string(e.augslot4));
		v.push_back(columns[10] + " = " + std::to_string(e.augslot5));
		v.push_back(columns[11] + " = " + std::to_string(e.augslot6));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.parentid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static ObjectContents InsertOne(
		Database& db,
		ObjectContents e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.zoneid));
		v.push_back(std::to_string(e.parentid));
		v.push_back(std::to_string(e.bagidx));
		v.push_back(std::to_string(e.itemid));
		v.push_back(std::to_string(e.charges));
		v.push_back("FROM_UNIXTIME(" + (e.droptime > 0 ? std::to_string(e.droptime) : "null") + ")");
		v.push_back(std::to_string(e.augslot1));
		v.push_back(std::to_string(e.augslot2));
		v.push_back(std::to_string(e.augslot3));
		v.push_back(std::to_string(e.augslot4));
		v.push_back(std::to_string(e.augslot5));
		v.push_back(std::to_string(e.augslot6));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.parentid = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<ObjectContents> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.zoneid));
			v.push_back(std::to_string(e.parentid));
			v.push_back(std::to_string(e.bagidx));
			v.push_back(std::to_string(e.itemid));
			v.push_back(std::to_string(e.charges));
			v.push_back("FROM_UNIXTIME(" + (e.droptime > 0 ? std::to_string(e.droptime) : "null") + ")");
			v.push_back(std::to_string(e.augslot1));
			v.push_back(std::to_string(e.augslot2));
			v.push_back(std::to_string(e.augslot3));
			v.push_back(std::to_string(e.augslot4));
			v.push_back(std::to_string(e.augslot5));
			v.push_back(std::to_string(e.augslot6));

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

	static std::vector<ObjectContents> All(Database& db)
	{
		std::vector<ObjectContents> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ObjectContents e{};

			e.zoneid   = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.parentid = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.bagidx   = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.itemid   = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.charges  = row[4] ? static_cast<int16_t>(atoi(row[4])) : 0;
			e.droptime = strtoll(row[5] ? row[5] : "-1", nullptr, 10);
			e.augslot1 = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.augslot2 = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.augslot3 = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.augslot4 = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.augslot5 = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.augslot6 = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<ObjectContents> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<ObjectContents> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ObjectContents e{};

			e.zoneid   = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.parentid = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.bagidx   = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.itemid   = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.charges  = row[4] ? static_cast<int16_t>(atoi(row[4])) : 0;
			e.droptime = strtoll(row[5] ? row[5] : "-1", nullptr, 10);
			e.augslot1 = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.augslot2 = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.augslot3 = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.augslot4 = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.augslot5 = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.augslot6 = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;

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
		const ObjectContents &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.zoneid));
		v.push_back(std::to_string(e.parentid));
		v.push_back(std::to_string(e.bagidx));
		v.push_back(std::to_string(e.itemid));
		v.push_back(std::to_string(e.charges));
		v.push_back("FROM_UNIXTIME(" + (e.droptime > 0 ? std::to_string(e.droptime) : "null") + ")");
		v.push_back(std::to_string(e.augslot1));
		v.push_back(std::to_string(e.augslot2));
		v.push_back(std::to_string(e.augslot3));
		v.push_back(std::to_string(e.augslot4));
		v.push_back(std::to_string(e.augslot5));
		v.push_back(std::to_string(e.augslot6));

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
		const std::vector<ObjectContents> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.zoneid));
			v.push_back(std::to_string(e.parentid));
			v.push_back(std::to_string(e.bagidx));
			v.push_back(std::to_string(e.itemid));
			v.push_back(std::to_string(e.charges));
			v.push_back("FROM_UNIXTIME(" + (e.droptime > 0 ? std::to_string(e.droptime) : "null") + ")");
			v.push_back(std::to_string(e.augslot1));
			v.push_back(std::to_string(e.augslot2));
			v.push_back(std::to_string(e.augslot3));
			v.push_back(std::to_string(e.augslot4));
			v.push_back(std::to_string(e.augslot5));
			v.push_back(std::to_string(e.augslot6));

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
