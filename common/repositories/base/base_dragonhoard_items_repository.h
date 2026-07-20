/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
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

class BaseDragonhoardItemsRepository {
public:
	struct DragonhoardItems {
		uint32_t    id;
		int32_t     account_id;
		int32_t     slot_id;
		int32_t     item_id;
		std::string item_name;
		uint32_t    stack_count;
		int32_t     charges;
		int64_t     serial;
		int32_t     augslot1;
		int32_t     augslot2;
		int32_t     augslot3;
		int32_t     augslot4;
		int32_t     augslot5;
		int32_t     augslot6;
		std::string custom_data;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"account_id",
			"slot_id",
			"item_id",
			"item_name",
			"stack_count",
			"charges",
			"serial",
			"augslot1",
			"augslot2",
			"augslot3",
			"augslot4",
			"augslot5",
			"augslot6",
			"custom_data",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"account_id",
			"slot_id",
			"item_id",
			"item_name",
			"stack_count",
			"charges",
			"serial",
			"augslot1",
			"augslot2",
			"augslot3",
			"augslot4",
			"augslot5",
			"augslot6",
			"custom_data",
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
		return std::string("dragonhoard_items");
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

	static DragonhoardItems NewEntity()
	{
		DragonhoardItems e{};

		e.id          = 0;
		e.account_id  = 0;
		e.slot_id     = 0;
		e.item_id     = 0;
		e.item_name   = "";
		e.stack_count = 1;
		e.charges     = 0;
		e.serial      = 0;
		e.augslot1    = 0;
		e.augslot2    = 0;
		e.augslot3    = 0;
		e.augslot4    = 0;
		e.augslot5    = 0;
		e.augslot6    = 0;
		e.custom_data = "";

		return e;
	}

	static DragonhoardItems GetDragonhoardItems(
		const std::vector<DragonhoardItems> &dragonhoard_itemss,
		int dragonhoard_items_id
	)
	{
		for (auto &dragonhoard_items : dragonhoard_itemss) {
			if (dragonhoard_items.id == dragonhoard_items_id) {
				return dragonhoard_items;
			}
		}

		return NewEntity();
	}

	static DragonhoardItems FindOne(
		Database& db,
		int dragonhoard_items_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				dragonhoard_items_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			DragonhoardItems e{};

			e.id          = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.account_id  = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.slot_id     = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.item_id     = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.item_name   = row[4] ? row[4] : "";
			e.stack_count = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 1;
			e.charges     = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.serial      = row[7] ? strtoll(row[7], nullptr, 10) : 0;
			e.augslot1    = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.augslot2    = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.augslot3    = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.augslot4    = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;
			e.augslot5    = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.augslot6    = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.custom_data = row[14] ? row[14] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int dragonhoard_items_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				dragonhoard_items_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const DragonhoardItems &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.account_id));
		v.push_back(columns[2] + " = " + std::to_string(e.slot_id));
		v.push_back(columns[3] + " = " + std::to_string(e.item_id));
		v.push_back(columns[4] + " = '" + Strings::Escape(e.item_name) + "'");
		v.push_back(columns[5] + " = " + std::to_string(e.stack_count));
		v.push_back(columns[6] + " = " + std::to_string(e.charges));
		v.push_back(columns[7] + " = " + std::to_string(e.serial));
		v.push_back(columns[8] + " = " + std::to_string(e.augslot1));
		v.push_back(columns[9] + " = " + std::to_string(e.augslot2));
		v.push_back(columns[10] + " = " + std::to_string(e.augslot3));
		v.push_back(columns[11] + " = " + std::to_string(e.augslot4));
		v.push_back(columns[12] + " = " + std::to_string(e.augslot5));
		v.push_back(columns[13] + " = " + std::to_string(e.augslot6));
		v.push_back(columns[14] + " = '" + Strings::Escape(e.custom_data) + "'");

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

	static DragonhoardItems InsertOne(
		Database& db,
		DragonhoardItems e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.account_id));
		v.push_back(std::to_string(e.slot_id));
		v.push_back(std::to_string(e.item_id));
		v.push_back("'" + Strings::Escape(e.item_name) + "'");
		v.push_back(std::to_string(e.stack_count));
		v.push_back(std::to_string(e.charges));
		v.push_back(std::to_string(e.serial));
		v.push_back(std::to_string(e.augslot1));
		v.push_back(std::to_string(e.augslot2));
		v.push_back(std::to_string(e.augslot3));
		v.push_back(std::to_string(e.augslot4));
		v.push_back(std::to_string(e.augslot5));
		v.push_back(std::to_string(e.augslot6));
		v.push_back("'" + Strings::Escape(e.custom_data) + "'");

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
		const std::vector<DragonhoardItems> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.account_id));
			v.push_back(std::to_string(e.slot_id));
			v.push_back(std::to_string(e.item_id));
			v.push_back("'" + Strings::Escape(e.item_name) + "'");
			v.push_back(std::to_string(e.stack_count));
			v.push_back(std::to_string(e.charges));
			v.push_back(std::to_string(e.serial));
			v.push_back(std::to_string(e.augslot1));
			v.push_back(std::to_string(e.augslot2));
			v.push_back(std::to_string(e.augslot3));
			v.push_back(std::to_string(e.augslot4));
			v.push_back(std::to_string(e.augslot5));
			v.push_back(std::to_string(e.augslot6));
			v.push_back("'" + Strings::Escape(e.custom_data) + "'");

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

	static std::vector<DragonhoardItems> All(Database& db)
	{
		std::vector<DragonhoardItems> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			DragonhoardItems e{};

			e.id          = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.account_id  = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.slot_id     = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.item_id     = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.item_name   = row[4] ? row[4] : "";
			e.stack_count = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 1;
			e.charges     = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.serial      = row[7] ? strtoll(row[7], nullptr, 10) : 0;
			e.augslot1    = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.augslot2    = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.augslot3    = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.augslot4    = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;
			e.augslot5    = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.augslot6    = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.custom_data = row[14] ? row[14] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<DragonhoardItems> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<DragonhoardItems> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			DragonhoardItems e{};

			e.id          = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.account_id  = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.slot_id     = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.item_id     = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.item_name   = row[4] ? row[4] : "";
			e.stack_count = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 1;
			e.charges     = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.serial      = row[7] ? strtoll(row[7], nullptr, 10) : 0;
			e.augslot1    = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;
			e.augslot2    = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.augslot3    = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.augslot4    = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;
			e.augslot5    = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.augslot6    = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.custom_data = row[14] ? row[14] : "";

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
		const DragonhoardItems &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.account_id));
		v.push_back(std::to_string(e.slot_id));
		v.push_back(std::to_string(e.item_id));
		v.push_back("'" + Strings::Escape(e.item_name) + "'");
		v.push_back(std::to_string(e.stack_count));
		v.push_back(std::to_string(e.charges));
		v.push_back(std::to_string(e.serial));
		v.push_back(std::to_string(e.augslot1));
		v.push_back(std::to_string(e.augslot2));
		v.push_back(std::to_string(e.augslot3));
		v.push_back(std::to_string(e.augslot4));
		v.push_back(std::to_string(e.augslot5));
		v.push_back(std::to_string(e.augslot6));
		v.push_back("'" + Strings::Escape(e.custom_data) + "'");

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
		const std::vector<DragonhoardItems> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.account_id));
			v.push_back(std::to_string(e.slot_id));
			v.push_back(std::to_string(e.item_id));
			v.push_back("'" + Strings::Escape(e.item_name) + "'");
			v.push_back(std::to_string(e.stack_count));
			v.push_back(std::to_string(e.charges));
			v.push_back(std::to_string(e.serial));
			v.push_back(std::to_string(e.augslot1));
			v.push_back(std::to_string(e.augslot2));
			v.push_back(std::to_string(e.augslot3));
			v.push_back(std::to_string(e.augslot4));
			v.push_back(std::to_string(e.augslot5));
			v.push_back(std::to_string(e.augslot6));
			v.push_back("'" + Strings::Escape(e.custom_data) + "'");

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
