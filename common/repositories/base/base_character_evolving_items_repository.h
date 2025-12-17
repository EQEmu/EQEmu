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

class BaseCharacterEvolvingItemsRepository {
public:
	struct CharacterEvolvingItems {
		uint64_t id;
		uint32_t character_id;
		uint32_t item_id;
		uint8_t  activated;
		uint8_t  equipped;
		int64_t  current_amount;
		double   progression;
		uint32_t final_item_id;
		time_t   deleted_at;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"character_id",
			"item_id",
			"activated",
			"equipped",
			"current_amount",
			"progression",
			"final_item_id",
			"deleted_at",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"character_id",
			"item_id",
			"activated",
			"equipped",
			"current_amount",
			"progression",
			"final_item_id",
			"UNIX_TIMESTAMP(deleted_at)",
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
		return std::string("character_evolving_items");
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

	static CharacterEvolvingItems NewEntity()
	{
		CharacterEvolvingItems e{};

		e.id             = 0;
		e.character_id   = 0;
		e.item_id        = 0;
		e.activated      = 0;
		e.equipped       = 0;
		e.current_amount = 0;
		e.progression    = 0;
		e.final_item_id  = 0;
		e.deleted_at     = 0;

		return e;
	}

	static CharacterEvolvingItems GetCharacterEvolvingItems(
		const std::vector<CharacterEvolvingItems> &character_evolving_itemss,
		int character_evolving_items_id
	)
	{
		for (auto &character_evolving_items : character_evolving_itemss) {
			if (character_evolving_items.id == character_evolving_items_id) {
				return character_evolving_items;
			}
		}

		return NewEntity();
	}

	static CharacterEvolvingItems FindOne(
		Database& db,
		int character_evolving_items_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				character_evolving_items_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterEvolvingItems e{};

			e.id             = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.character_id   = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.item_id        = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.activated      = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.equipped       = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.current_amount = row[5] ? strtoll(row[5], nullptr, 10) : 0;
			e.progression    = row[6] ? strtod(row[6], nullptr) : 0;
			e.final_item_id  = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.deleted_at     = strtoll(row[8] ? row[8] : "-1", nullptr, 10);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_evolving_items_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_evolving_items_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CharacterEvolvingItems &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.character_id));
		v.push_back(columns[2] + " = " + std::to_string(e.item_id));
		v.push_back(columns[3] + " = " + std::to_string(e.activated));
		v.push_back(columns[4] + " = " + std::to_string(e.equipped));
		v.push_back(columns[5] + " = " + std::to_string(e.current_amount));
		v.push_back(columns[6] + " = " + std::to_string(e.progression));
		v.push_back(columns[7] + " = " + std::to_string(e.final_item_id));
		v.push_back(columns[8] + " = FROM_UNIXTIME(" + (e.deleted_at > 0 ? std::to_string(e.deleted_at) : "null") + ")");

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

	static CharacterEvolvingItems InsertOne(
		Database& db,
		CharacterEvolvingItems e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.character_id));
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.activated));
		v.push_back(std::to_string(e.equipped));
		v.push_back(std::to_string(e.current_amount));
		v.push_back(std::to_string(e.progression));
		v.push_back(std::to_string(e.final_item_id));
		v.push_back("FROM_UNIXTIME(" + (e.deleted_at > 0 ? std::to_string(e.deleted_at) : "null") + ")");

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
		const std::vector<CharacterEvolvingItems> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.character_id));
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.activated));
			v.push_back(std::to_string(e.equipped));
			v.push_back(std::to_string(e.current_amount));
			v.push_back(std::to_string(e.progression));
			v.push_back(std::to_string(e.final_item_id));
			v.push_back("FROM_UNIXTIME(" + (e.deleted_at > 0 ? std::to_string(e.deleted_at) : "null") + ")");

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

	static std::vector<CharacterEvolvingItems> All(Database& db)
	{
		std::vector<CharacterEvolvingItems> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterEvolvingItems e{};

			e.id             = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.character_id   = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.item_id        = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.activated      = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.equipped       = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.current_amount = row[5] ? strtoll(row[5], nullptr, 10) : 0;
			e.progression    = row[6] ? strtod(row[6], nullptr) : 0;
			e.final_item_id  = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.deleted_at     = strtoll(row[8] ? row[8] : "-1", nullptr, 10);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CharacterEvolvingItems> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CharacterEvolvingItems> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterEvolvingItems e{};

			e.id             = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.character_id   = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.item_id        = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.activated      = row[3] ? static_cast<uint8_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.equipped       = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.current_amount = row[5] ? strtoll(row[5], nullptr, 10) : 0;
			e.progression    = row[6] ? strtod(row[6], nullptr) : 0;
			e.final_item_id  = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.deleted_at     = strtoll(row[8] ? row[8] : "-1", nullptr, 10);

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
		const CharacterEvolvingItems &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.character_id));
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.activated));
		v.push_back(std::to_string(e.equipped));
		v.push_back(std::to_string(e.current_amount));
		v.push_back(std::to_string(e.progression));
		v.push_back(std::to_string(e.final_item_id));
		v.push_back("FROM_UNIXTIME(" + (e.deleted_at > 0 ? std::to_string(e.deleted_at) : "null") + ")");

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
		const std::vector<CharacterEvolvingItems> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.character_id));
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.activated));
			v.push_back(std::to_string(e.equipped));
			v.push_back(std::to_string(e.current_amount));
			v.push_back(std::to_string(e.progression));
			v.push_back(std::to_string(e.final_item_id));
			v.push_back("FROM_UNIXTIME(" + (e.deleted_at > 0 ? std::to_string(e.deleted_at) : "null") + ")");

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
