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

class BaseAurasRepository {
public:
	struct Auras {
		int32_t     type;
		int32_t     npc_type;
		std::string name;
		int32_t     spell_id;
		int32_t     distance;
		int32_t     aura_type;
		int32_t     spawn_type;
		int32_t     movement;
		int32_t     duration;
		int32_t     icon;
		int32_t     cast_time;
	};

	static std::string PrimaryKey()
	{
		return std::string("type");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"type",
			"npc_type",
			"name",
			"spell_id",
			"distance",
			"aura_type",
			"spawn_type",
			"movement",
			"duration",
			"icon",
			"cast_time",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"type",
			"npc_type",
			"name",
			"spell_id",
			"distance",
			"aura_type",
			"spawn_type",
			"movement",
			"duration",
			"icon",
			"cast_time",
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
		return std::string("auras");
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

	static Auras NewEntity()
	{
		Auras e{};

		e.type       = 0;
		e.npc_type   = 0;
		e.name       = "";
		e.spell_id   = 0;
		e.distance   = 60;
		e.aura_type  = 1;
		e.spawn_type = 0;
		e.movement   = 0;
		e.duration   = 5400;
		e.icon       = -1;
		e.cast_time  = 0;

		return e;
	}

	static Auras GetAuras(
		const std::vector<Auras> &aurass,
		int auras_id
	)
	{
		for (auto &auras : aurass) {
			if (auras.type == auras_id) {
				return auras;
			}
		}

		return NewEntity();
	}

	static Auras FindOne(
		Database& db,
		int auras_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				auras_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Auras e{};

			e.type       = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.npc_type   = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.name       = row[2] ? row[2] : "";
			e.spell_id   = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.distance   = row[4] ? static_cast<int32_t>(atoi(row[4])) : 60;
			e.aura_type  = row[5] ? static_cast<int32_t>(atoi(row[5])) : 1;
			e.spawn_type = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.movement   = row[7] ? static_cast<int32_t>(atoi(row[7])) : 0;
			e.duration   = row[8] ? static_cast<int32_t>(atoi(row[8])) : 5400;
			e.icon       = row[9] ? static_cast<int32_t>(atoi(row[9])) : -1;
			e.cast_time  = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int auras_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				auras_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Auras &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.type));
		v.push_back(columns[1] + " = " + std::to_string(e.npc_type));
		v.push_back(columns[2] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[3] + " = " + std::to_string(e.spell_id));
		v.push_back(columns[4] + " = " + std::to_string(e.distance));
		v.push_back(columns[5] + " = " + std::to_string(e.aura_type));
		v.push_back(columns[6] + " = " + std::to_string(e.spawn_type));
		v.push_back(columns[7] + " = " + std::to_string(e.movement));
		v.push_back(columns[8] + " = " + std::to_string(e.duration));
		v.push_back(columns[9] + " = " + std::to_string(e.icon));
		v.push_back(columns[10] + " = " + std::to_string(e.cast_time));

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

	static Auras InsertOne(
		Database& db,
		Auras e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.npc_type));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.spell_id));
		v.push_back(std::to_string(e.distance));
		v.push_back(std::to_string(e.aura_type));
		v.push_back(std::to_string(e.spawn_type));
		v.push_back(std::to_string(e.movement));
		v.push_back(std::to_string(e.duration));
		v.push_back(std::to_string(e.icon));
		v.push_back(std::to_string(e.cast_time));

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
		const std::vector<Auras> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.npc_type));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.spell_id));
			v.push_back(std::to_string(e.distance));
			v.push_back(std::to_string(e.aura_type));
			v.push_back(std::to_string(e.spawn_type));
			v.push_back(std::to_string(e.movement));
			v.push_back(std::to_string(e.duration));
			v.push_back(std::to_string(e.icon));
			v.push_back(std::to_string(e.cast_time));

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

	static std::vector<Auras> All(Database& db)
	{
		std::vector<Auras> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Auras e{};

			e.type       = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.npc_type   = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.name       = row[2] ? row[2] : "";
			e.spell_id   = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.distance   = row[4] ? static_cast<int32_t>(atoi(row[4])) : 60;
			e.aura_type  = row[5] ? static_cast<int32_t>(atoi(row[5])) : 1;
			e.spawn_type = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.movement   = row[7] ? static_cast<int32_t>(atoi(row[7])) : 0;
			e.duration   = row[8] ? static_cast<int32_t>(atoi(row[8])) : 5400;
			e.icon       = row[9] ? static_cast<int32_t>(atoi(row[9])) : -1;
			e.cast_time  = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Auras> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Auras> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Auras e{};

			e.type       = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.npc_type   = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.name       = row[2] ? row[2] : "";
			e.spell_id   = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.distance   = row[4] ? static_cast<int32_t>(atoi(row[4])) : 60;
			e.aura_type  = row[5] ? static_cast<int32_t>(atoi(row[5])) : 1;
			e.spawn_type = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.movement   = row[7] ? static_cast<int32_t>(atoi(row[7])) : 0;
			e.duration   = row[8] ? static_cast<int32_t>(atoi(row[8])) : 5400;
			e.icon       = row[9] ? static_cast<int32_t>(atoi(row[9])) : -1;
			e.cast_time  = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;

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
		const Auras &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.npc_type));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.spell_id));
		v.push_back(std::to_string(e.distance));
		v.push_back(std::to_string(e.aura_type));
		v.push_back(std::to_string(e.spawn_type));
		v.push_back(std::to_string(e.movement));
		v.push_back(std::to_string(e.duration));
		v.push_back(std::to_string(e.icon));
		v.push_back(std::to_string(e.cast_time));

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
		const std::vector<Auras> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.npc_type));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.spell_id));
			v.push_back(std::to_string(e.distance));
			v.push_back(std::to_string(e.aura_type));
			v.push_back(std::to_string(e.spawn_type));
			v.push_back(std::to_string(e.movement));
			v.push_back(std::to_string(e.duration));
			v.push_back(std::to_string(e.icon));
			v.push_back(std::to_string(e.cast_time));

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
