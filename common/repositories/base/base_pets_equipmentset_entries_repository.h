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

class BasePetsEquipmentsetEntriesRepository {
public:
	struct PetsEquipmentsetEntries {
		int32_t set_id;
		int32_t slot;
		int32_t item_id;
	};

	static std::string PrimaryKey()
	{
		return std::string("set_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"set_id",
			"slot",
			"item_id",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"set_id",
			"slot",
			"item_id",
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
		return std::string("pets_equipmentset_entries");
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

	static PetsEquipmentsetEntries NewEntity()
	{
		PetsEquipmentsetEntries e{};

		e.set_id  = 0;
		e.slot    = 0;
		e.item_id = 0;

		return e;
	}

	static PetsEquipmentsetEntries GetPetsEquipmentsetEntries(
		const std::vector<PetsEquipmentsetEntries> &pets_equipmentset_entriess,
		int pets_equipmentset_entries_id
	)
	{
		for (auto &pets_equipmentset_entries : pets_equipmentset_entriess) {
			if (pets_equipmentset_entries.set_id == pets_equipmentset_entries_id) {
				return pets_equipmentset_entries;
			}
		}

		return NewEntity();
	}

	static PetsEquipmentsetEntries FindOne(
		Database& db,
		int pets_equipmentset_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				pets_equipmentset_entries_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			PetsEquipmentsetEntries e{};

			e.set_id  = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.slot    = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.item_id = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int pets_equipmentset_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				pets_equipmentset_entries_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const PetsEquipmentsetEntries &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.set_id));
		v.push_back(columns[1] + " = " + std::to_string(e.slot));
		v.push_back(columns[2] + " = " + std::to_string(e.item_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.set_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static PetsEquipmentsetEntries InsertOne(
		Database& db,
		PetsEquipmentsetEntries e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.set_id));
		v.push_back(std::to_string(e.slot));
		v.push_back(std::to_string(e.item_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.set_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<PetsEquipmentsetEntries> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.set_id));
			v.push_back(std::to_string(e.slot));
			v.push_back(std::to_string(e.item_id));

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

	static std::vector<PetsEquipmentsetEntries> All(Database& db)
	{
		std::vector<PetsEquipmentsetEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PetsEquipmentsetEntries e{};

			e.set_id  = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.slot    = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.item_id = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<PetsEquipmentsetEntries> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<PetsEquipmentsetEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PetsEquipmentsetEntries e{};

			e.set_id  = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.slot    = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.item_id = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;

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
		const PetsEquipmentsetEntries &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.set_id));
		v.push_back(std::to_string(e.slot));
		v.push_back(std::to_string(e.item_id));

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
		const std::vector<PetsEquipmentsetEntries> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.set_id));
			v.push_back(std::to_string(e.slot));
			v.push_back(std::to_string(e.item_id));

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
