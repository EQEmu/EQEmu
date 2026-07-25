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

class BaseCharacterRegionsUnlockedRepository {
public:
	struct CharacterRegionsUnlocked {
		uint32_t    region_id;
		uint32_t    char_id;
		std::string char_name;
		std::string region_name;
	};

	static std::string PrimaryKey()
	{
		return std::string("region_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"region_id",
			"char_id",
			"char_name",
			"region_name",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"region_id",
			"char_id",
			"char_name",
			"region_name",
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
		return std::string("character_regions_unlocked");
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

	static CharacterRegionsUnlocked NewEntity()
	{
		CharacterRegionsUnlocked e{};

		e.region_id   = 0;
		e.char_id     = 0;
		e.char_name   = "";
		e.region_name = "";

		return e;
	}

	static CharacterRegionsUnlocked GetCharacterRegionsUnlocked(
		const std::vector<CharacterRegionsUnlocked> &character_regions_unlockeds,
		int character_regions_unlocked_id
	)
	{
		for (auto &character_regions_unlocked : character_regions_unlockeds) {
			if (character_regions_unlocked.region_id == character_regions_unlocked_id) {
				return character_regions_unlocked;
			}
		}

		return NewEntity();
	}

	static CharacterRegionsUnlocked FindOne(
		Database& db,
		int character_regions_unlocked_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				character_regions_unlocked_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterRegionsUnlocked e{};

			e.region_id   = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.char_id     = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.char_name   = row[2] ? row[2] : "";
			e.region_name = row[3] ? row[3] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_regions_unlocked_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_regions_unlocked_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CharacterRegionsUnlocked &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.region_id));
		v.push_back(columns[1] + " = " + std::to_string(e.char_id));
		v.push_back(columns[2] + " = '" + Strings::Escape(e.char_name) + "'");
		v.push_back(columns[3] + " = '" + Strings::Escape(e.region_name) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.region_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterRegionsUnlocked InsertOne(
		Database& db,
		CharacterRegionsUnlocked e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.region_id));
		v.push_back(std::to_string(e.char_id));
		v.push_back("'" + Strings::Escape(e.char_name) + "'");
		v.push_back("'" + Strings::Escape(e.region_name) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.region_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<CharacterRegionsUnlocked> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.region_id));
			v.push_back(std::to_string(e.char_id));
			v.push_back("'" + Strings::Escape(e.char_name) + "'");
			v.push_back("'" + Strings::Escape(e.region_name) + "'");

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

	static std::vector<CharacterRegionsUnlocked> All(Database& db)
	{
		std::vector<CharacterRegionsUnlocked> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterRegionsUnlocked e{};

			e.region_id   = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.char_id     = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.char_name   = row[2] ? row[2] : "";
			e.region_name = row[3] ? row[3] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CharacterRegionsUnlocked> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CharacterRegionsUnlocked> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterRegionsUnlocked e{};

			e.region_id   = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.char_id     = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.char_name   = row[2] ? row[2] : "";
			e.region_name = row[3] ? row[3] : "";

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
		const CharacterRegionsUnlocked &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.region_id));
		v.push_back(std::to_string(e.char_id));
		v.push_back("'" + Strings::Escape(e.char_name) + "'");
		v.push_back("'" + Strings::Escape(e.region_name) + "'");

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
		const std::vector<CharacterRegionsUnlocked> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.region_id));
			v.push_back(std::to_string(e.char_id));
			v.push_back("'" + Strings::Escape(e.char_name) + "'");
			v.push_back("'" + Strings::Escape(e.region_name) + "'");

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
