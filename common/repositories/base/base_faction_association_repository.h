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

class BaseFactionAssociationRepository {
public:
	struct FactionAssociation {
		int32_t id;
		int32_t id_1;
		float   mod_1;
		int32_t id_2;
		float   mod_2;
		int32_t id_3;
		float   mod_3;
		int32_t id_4;
		float   mod_4;
		int32_t id_5;
		float   mod_5;
		int32_t id_6;
		float   mod_6;
		int32_t id_7;
		float   mod_7;
		int32_t id_8;
		float   mod_8;
		int32_t id_9;
		float   mod_9;
		int32_t id_10;
		float   mod_10;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"id_1",
			"mod_1",
			"id_2",
			"mod_2",
			"id_3",
			"mod_3",
			"id_4",
			"mod_4",
			"id_5",
			"mod_5",
			"id_6",
			"mod_6",
			"id_7",
			"mod_7",
			"id_8",
			"mod_8",
			"id_9",
			"mod_9",
			"id_10",
			"mod_10",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"id_1",
			"mod_1",
			"id_2",
			"mod_2",
			"id_3",
			"mod_3",
			"id_4",
			"mod_4",
			"id_5",
			"mod_5",
			"id_6",
			"mod_6",
			"id_7",
			"mod_7",
			"id_8",
			"mod_8",
			"id_9",
			"mod_9",
			"id_10",
			"mod_10",
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
		return std::string("faction_association");
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

	static FactionAssociation NewEntity()
	{
		FactionAssociation e{};

		e.id     = 0;
		e.id_1   = 0;
		e.mod_1  = 0;
		e.id_2   = 0;
		e.mod_2  = 0;
		e.id_3   = 0;
		e.mod_3  = 0;
		e.id_4   = 0;
		e.mod_4  = 0;
		e.id_5   = 0;
		e.mod_5  = 0;
		e.id_6   = 0;
		e.mod_6  = 0;
		e.id_7   = 0;
		e.mod_7  = 0;
		e.id_8   = 0;
		e.mod_8  = 0;
		e.id_9   = 0;
		e.mod_9  = 0;
		e.id_10  = 0;
		e.mod_10 = 0;

		return e;
	}

	static FactionAssociation GetFactionAssociation(
		const std::vector<FactionAssociation> &faction_associations,
		int faction_association_id
	)
	{
		for (auto &faction_association : faction_associations) {
			if (faction_association.id == faction_association_id) {
				return faction_association;
			}
		}

		return NewEntity();
	}

	static FactionAssociation FindOne(
		Database& db,
		int faction_association_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				faction_association_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			FactionAssociation e{};

			e.id     = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.id_1   = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.mod_1  = row[2] ? strtof(row[2], nullptr) : 0;
			e.id_2   = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.mod_2  = row[4] ? strtof(row[4], nullptr) : 0;
			e.id_3   = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.mod_3  = row[6] ? strtof(row[6], nullptr) : 0;
			e.id_4   = row[7] ? static_cast<int32_t>(atoi(row[7])) : 0;
			e.mod_4  = row[8] ? strtof(row[8], nullptr) : 0;
			e.id_5   = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.mod_5  = row[10] ? strtof(row[10], nullptr) : 0;
			e.id_6   = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;
			e.mod_6  = row[12] ? strtof(row[12], nullptr) : 0;
			e.id_7   = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.mod_7  = row[14] ? strtof(row[14], nullptr) : 0;
			e.id_8   = row[15] ? static_cast<int32_t>(atoi(row[15])) : 0;
			e.mod_8  = row[16] ? strtof(row[16], nullptr) : 0;
			e.id_9   = row[17] ? static_cast<int32_t>(atoi(row[17])) : 0;
			e.mod_9  = row[18] ? strtof(row[18], nullptr) : 0;
			e.id_10  = row[19] ? static_cast<int32_t>(atoi(row[19])) : 0;
			e.mod_10 = row[20] ? strtof(row[20], nullptr) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int faction_association_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				faction_association_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const FactionAssociation &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.id));
		v.push_back(columns[1] + " = " + std::to_string(e.id_1));
		v.push_back(columns[2] + " = " + std::to_string(e.mod_1));
		v.push_back(columns[3] + " = " + std::to_string(e.id_2));
		v.push_back(columns[4] + " = " + std::to_string(e.mod_2));
		v.push_back(columns[5] + " = " + std::to_string(e.id_3));
		v.push_back(columns[6] + " = " + std::to_string(e.mod_3));
		v.push_back(columns[7] + " = " + std::to_string(e.id_4));
		v.push_back(columns[8] + " = " + std::to_string(e.mod_4));
		v.push_back(columns[9] + " = " + std::to_string(e.id_5));
		v.push_back(columns[10] + " = " + std::to_string(e.mod_5));
		v.push_back(columns[11] + " = " + std::to_string(e.id_6));
		v.push_back(columns[12] + " = " + std::to_string(e.mod_6));
		v.push_back(columns[13] + " = " + std::to_string(e.id_7));
		v.push_back(columns[14] + " = " + std::to_string(e.mod_7));
		v.push_back(columns[15] + " = " + std::to_string(e.id_8));
		v.push_back(columns[16] + " = " + std::to_string(e.mod_8));
		v.push_back(columns[17] + " = " + std::to_string(e.id_9));
		v.push_back(columns[18] + " = " + std::to_string(e.mod_9));
		v.push_back(columns[19] + " = " + std::to_string(e.id_10));
		v.push_back(columns[20] + " = " + std::to_string(e.mod_10));

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

	static FactionAssociation InsertOne(
		Database& db,
		FactionAssociation e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.id_1));
		v.push_back(std::to_string(e.mod_1));
		v.push_back(std::to_string(e.id_2));
		v.push_back(std::to_string(e.mod_2));
		v.push_back(std::to_string(e.id_3));
		v.push_back(std::to_string(e.mod_3));
		v.push_back(std::to_string(e.id_4));
		v.push_back(std::to_string(e.mod_4));
		v.push_back(std::to_string(e.id_5));
		v.push_back(std::to_string(e.mod_5));
		v.push_back(std::to_string(e.id_6));
		v.push_back(std::to_string(e.mod_6));
		v.push_back(std::to_string(e.id_7));
		v.push_back(std::to_string(e.mod_7));
		v.push_back(std::to_string(e.id_8));
		v.push_back(std::to_string(e.mod_8));
		v.push_back(std::to_string(e.id_9));
		v.push_back(std::to_string(e.mod_9));
		v.push_back(std::to_string(e.id_10));
		v.push_back(std::to_string(e.mod_10));

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
		const std::vector<FactionAssociation> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.id_1));
			v.push_back(std::to_string(e.mod_1));
			v.push_back(std::to_string(e.id_2));
			v.push_back(std::to_string(e.mod_2));
			v.push_back(std::to_string(e.id_3));
			v.push_back(std::to_string(e.mod_3));
			v.push_back(std::to_string(e.id_4));
			v.push_back(std::to_string(e.mod_4));
			v.push_back(std::to_string(e.id_5));
			v.push_back(std::to_string(e.mod_5));
			v.push_back(std::to_string(e.id_6));
			v.push_back(std::to_string(e.mod_6));
			v.push_back(std::to_string(e.id_7));
			v.push_back(std::to_string(e.mod_7));
			v.push_back(std::to_string(e.id_8));
			v.push_back(std::to_string(e.mod_8));
			v.push_back(std::to_string(e.id_9));
			v.push_back(std::to_string(e.mod_9));
			v.push_back(std::to_string(e.id_10));
			v.push_back(std::to_string(e.mod_10));

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

	static std::vector<FactionAssociation> All(Database& db)
	{
		std::vector<FactionAssociation> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			FactionAssociation e{};

			e.id     = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.id_1   = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.mod_1  = row[2] ? strtof(row[2], nullptr) : 0;
			e.id_2   = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.mod_2  = row[4] ? strtof(row[4], nullptr) : 0;
			e.id_3   = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.mod_3  = row[6] ? strtof(row[6], nullptr) : 0;
			e.id_4   = row[7] ? static_cast<int32_t>(atoi(row[7])) : 0;
			e.mod_4  = row[8] ? strtof(row[8], nullptr) : 0;
			e.id_5   = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.mod_5  = row[10] ? strtof(row[10], nullptr) : 0;
			e.id_6   = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;
			e.mod_6  = row[12] ? strtof(row[12], nullptr) : 0;
			e.id_7   = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.mod_7  = row[14] ? strtof(row[14], nullptr) : 0;
			e.id_8   = row[15] ? static_cast<int32_t>(atoi(row[15])) : 0;
			e.mod_8  = row[16] ? strtof(row[16], nullptr) : 0;
			e.id_9   = row[17] ? static_cast<int32_t>(atoi(row[17])) : 0;
			e.mod_9  = row[18] ? strtof(row[18], nullptr) : 0;
			e.id_10  = row[19] ? static_cast<int32_t>(atoi(row[19])) : 0;
			e.mod_10 = row[20] ? strtof(row[20], nullptr) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<FactionAssociation> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<FactionAssociation> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			FactionAssociation e{};

			e.id     = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.id_1   = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.mod_1  = row[2] ? strtof(row[2], nullptr) : 0;
			e.id_2   = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.mod_2  = row[4] ? strtof(row[4], nullptr) : 0;
			e.id_3   = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.mod_3  = row[6] ? strtof(row[6], nullptr) : 0;
			e.id_4   = row[7] ? static_cast<int32_t>(atoi(row[7])) : 0;
			e.mod_4  = row[8] ? strtof(row[8], nullptr) : 0;
			e.id_5   = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.mod_5  = row[10] ? strtof(row[10], nullptr) : 0;
			e.id_6   = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;
			e.mod_6  = row[12] ? strtof(row[12], nullptr) : 0;
			e.id_7   = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.mod_7  = row[14] ? strtof(row[14], nullptr) : 0;
			e.id_8   = row[15] ? static_cast<int32_t>(atoi(row[15])) : 0;
			e.mod_8  = row[16] ? strtof(row[16], nullptr) : 0;
			e.id_9   = row[17] ? static_cast<int32_t>(atoi(row[17])) : 0;
			e.mod_9  = row[18] ? strtof(row[18], nullptr) : 0;
			e.id_10  = row[19] ? static_cast<int32_t>(atoi(row[19])) : 0;
			e.mod_10 = row[20] ? strtof(row[20], nullptr) : 0;

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
		const FactionAssociation &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.id_1));
		v.push_back(std::to_string(e.mod_1));
		v.push_back(std::to_string(e.id_2));
		v.push_back(std::to_string(e.mod_2));
		v.push_back(std::to_string(e.id_3));
		v.push_back(std::to_string(e.mod_3));
		v.push_back(std::to_string(e.id_4));
		v.push_back(std::to_string(e.mod_4));
		v.push_back(std::to_string(e.id_5));
		v.push_back(std::to_string(e.mod_5));
		v.push_back(std::to_string(e.id_6));
		v.push_back(std::to_string(e.mod_6));
		v.push_back(std::to_string(e.id_7));
		v.push_back(std::to_string(e.mod_7));
		v.push_back(std::to_string(e.id_8));
		v.push_back(std::to_string(e.mod_8));
		v.push_back(std::to_string(e.id_9));
		v.push_back(std::to_string(e.mod_9));
		v.push_back(std::to_string(e.id_10));
		v.push_back(std::to_string(e.mod_10));

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
		const std::vector<FactionAssociation> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.id_1));
			v.push_back(std::to_string(e.mod_1));
			v.push_back(std::to_string(e.id_2));
			v.push_back(std::to_string(e.mod_2));
			v.push_back(std::to_string(e.id_3));
			v.push_back(std::to_string(e.mod_3));
			v.push_back(std::to_string(e.id_4));
			v.push_back(std::to_string(e.mod_4));
			v.push_back(std::to_string(e.id_5));
			v.push_back(std::to_string(e.mod_5));
			v.push_back(std::to_string(e.id_6));
			v.push_back(std::to_string(e.mod_6));
			v.push_back(std::to_string(e.id_7));
			v.push_back(std::to_string(e.mod_7));
			v.push_back(std::to_string(e.id_8));
			v.push_back(std::to_string(e.mod_8));
			v.push_back(std::to_string(e.id_9));
			v.push_back(std::to_string(e.mod_9));
			v.push_back(std::to_string(e.id_10));
			v.push_back(std::to_string(e.mod_10));

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
