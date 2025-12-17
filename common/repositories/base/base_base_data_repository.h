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

class BaseBaseDataRepository {
public:
	struct BaseData {
		uint8_t level;
		uint8_t class_;
		double  hp;
		double  mana;
		double  end;
		double  hp_regen;
		double  end_regen;
		double  hp_fac;
		double  mana_fac;
		double  end_fac;
	};

	static std::string PrimaryKey()
	{
		return std::string("level");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"level",
			"`class`",
			"hp",
			"mana",
			"end",
			"hp_regen",
			"end_regen",
			"hp_fac",
			"mana_fac",
			"end_fac",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"level",
			"`class`",
			"hp",
			"mana",
			"end",
			"hp_regen",
			"end_regen",
			"hp_fac",
			"mana_fac",
			"end_fac",
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
		return std::string("base_data");
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

	static BaseData NewEntity()
	{
		BaseData e{};

		e.level     = 0;
		e.class_    = 0;
		e.hp        = 0;
		e.mana      = 0;
		e.end       = 0;
		e.hp_regen  = 0;
		e.end_regen = 0;
		e.hp_fac    = 0;
		e.mana_fac  = 0;
		e.end_fac   = 0;

		return e;
	}

	static BaseData GetBaseData(
		const std::vector<BaseData> &base_datas,
		int base_data_id
	)
	{
		for (auto &base_data : base_datas) {
			if (base_data.level == base_data_id) {
				return base_data;
			}
		}

		return NewEntity();
	}

	static BaseData FindOne(
		Database& db,
		int base_data_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				base_data_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			BaseData e{};

			e.level     = row[0] ? static_cast<uint8_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.class_    = row[1] ? static_cast<uint8_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.hp        = row[2] ? strtod(row[2], nullptr) : 0;
			e.mana      = row[3] ? strtod(row[3], nullptr) : 0;
			e.end       = row[4] ? strtod(row[4], nullptr) : 0;
			e.hp_regen  = row[5] ? strtod(row[5], nullptr) : 0;
			e.end_regen = row[6] ? strtod(row[6], nullptr) : 0;
			e.hp_fac    = row[7] ? strtod(row[7], nullptr) : 0;
			e.mana_fac  = row[8] ? strtod(row[8], nullptr) : 0;
			e.end_fac   = row[9] ? strtod(row[9], nullptr) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int base_data_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				base_data_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const BaseData &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.level));
		v.push_back(columns[1] + " = " + std::to_string(e.class_));
		v.push_back(columns[2] + " = " + std::to_string(e.hp));
		v.push_back(columns[3] + " = " + std::to_string(e.mana));
		v.push_back(columns[4] + " = " + std::to_string(e.end));
		v.push_back(columns[5] + " = " + std::to_string(e.hp_regen));
		v.push_back(columns[6] + " = " + std::to_string(e.end_regen));
		v.push_back(columns[7] + " = " + std::to_string(e.hp_fac));
		v.push_back(columns[8] + " = " + std::to_string(e.mana_fac));
		v.push_back(columns[9] + " = " + std::to_string(e.end_fac));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.level
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static BaseData InsertOne(
		Database& db,
		BaseData e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.level));
		v.push_back(std::to_string(e.class_));
		v.push_back(std::to_string(e.hp));
		v.push_back(std::to_string(e.mana));
		v.push_back(std::to_string(e.end));
		v.push_back(std::to_string(e.hp_regen));
		v.push_back(std::to_string(e.end_regen));
		v.push_back(std::to_string(e.hp_fac));
		v.push_back(std::to_string(e.mana_fac));
		v.push_back(std::to_string(e.end_fac));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.level = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<BaseData> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.level));
			v.push_back(std::to_string(e.class_));
			v.push_back(std::to_string(e.hp));
			v.push_back(std::to_string(e.mana));
			v.push_back(std::to_string(e.end));
			v.push_back(std::to_string(e.hp_regen));
			v.push_back(std::to_string(e.end_regen));
			v.push_back(std::to_string(e.hp_fac));
			v.push_back(std::to_string(e.mana_fac));
			v.push_back(std::to_string(e.end_fac));

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

	static std::vector<BaseData> All(Database& db)
	{
		std::vector<BaseData> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BaseData e{};

			e.level     = row[0] ? static_cast<uint8_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.class_    = row[1] ? static_cast<uint8_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.hp        = row[2] ? strtod(row[2], nullptr) : 0;
			e.mana      = row[3] ? strtod(row[3], nullptr) : 0;
			e.end       = row[4] ? strtod(row[4], nullptr) : 0;
			e.hp_regen  = row[5] ? strtod(row[5], nullptr) : 0;
			e.end_regen = row[6] ? strtod(row[6], nullptr) : 0;
			e.hp_fac    = row[7] ? strtod(row[7], nullptr) : 0;
			e.mana_fac  = row[8] ? strtod(row[8], nullptr) : 0;
			e.end_fac   = row[9] ? strtod(row[9], nullptr) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<BaseData> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<BaseData> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BaseData e{};

			e.level     = row[0] ? static_cast<uint8_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.class_    = row[1] ? static_cast<uint8_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.hp        = row[2] ? strtod(row[2], nullptr) : 0;
			e.mana      = row[3] ? strtod(row[3], nullptr) : 0;
			e.end       = row[4] ? strtod(row[4], nullptr) : 0;
			e.hp_regen  = row[5] ? strtod(row[5], nullptr) : 0;
			e.end_regen = row[6] ? strtod(row[6], nullptr) : 0;
			e.hp_fac    = row[7] ? strtod(row[7], nullptr) : 0;
			e.mana_fac  = row[8] ? strtod(row[8], nullptr) : 0;
			e.end_fac   = row[9] ? strtod(row[9], nullptr) : 0;

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
		const BaseData &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.level));
		v.push_back(std::to_string(e.class_));
		v.push_back(std::to_string(e.hp));
		v.push_back(std::to_string(e.mana));
		v.push_back(std::to_string(e.end));
		v.push_back(std::to_string(e.hp_regen));
		v.push_back(std::to_string(e.end_regen));
		v.push_back(std::to_string(e.hp_fac));
		v.push_back(std::to_string(e.mana_fac));
		v.push_back(std::to_string(e.end_fac));

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
		const std::vector<BaseData> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.level));
			v.push_back(std::to_string(e.class_));
			v.push_back(std::to_string(e.hp));
			v.push_back(std::to_string(e.mana));
			v.push_back(std::to_string(e.end));
			v.push_back(std::to_string(e.hp_regen));
			v.push_back(std::to_string(e.end_regen));
			v.push_back(std::to_string(e.hp_fac));
			v.push_back(std::to_string(e.mana_fac));
			v.push_back(std::to_string(e.end_fac));

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
