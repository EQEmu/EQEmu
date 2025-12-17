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

class BaseCharacterCorpseItemsRepository {
public:
	struct CharacterCorpseItems {
		uint32_t    corpse_id;
		uint32_t    equip_slot;
		uint32_t    item_id;
		uint32_t    charges;
		uint32_t    aug_1;
		uint32_t    aug_2;
		uint32_t    aug_3;
		uint32_t    aug_4;
		uint32_t    aug_5;
		int32_t     aug_6;
		int16_t     attuned;
		std::string custom_data;
		uint32_t    ornamenticon;
		uint32_t    ornamentidfile;
		uint32_t    ornament_hero_model;
	};

	static std::string PrimaryKey()
	{
		return std::string("corpse_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"corpse_id",
			"equip_slot",
			"item_id",
			"charges",
			"aug_1",
			"aug_2",
			"aug_3",
			"aug_4",
			"aug_5",
			"aug_6",
			"attuned",
			"custom_data",
			"ornamenticon",
			"ornamentidfile",
			"ornament_hero_model",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"corpse_id",
			"equip_slot",
			"item_id",
			"charges",
			"aug_1",
			"aug_2",
			"aug_3",
			"aug_4",
			"aug_5",
			"aug_6",
			"attuned",
			"custom_data",
			"ornamenticon",
			"ornamentidfile",
			"ornament_hero_model",
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
		return std::string("character_corpse_items");
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

	static CharacterCorpseItems NewEntity()
	{
		CharacterCorpseItems e{};

		e.corpse_id           = 0;
		e.equip_slot          = 0;
		e.item_id             = 0;
		e.charges             = 0;
		e.aug_1               = 0;
		e.aug_2               = 0;
		e.aug_3               = 0;
		e.aug_4               = 0;
		e.aug_5               = 0;
		e.aug_6               = 0;
		e.attuned             = 0;
		e.custom_data         = "";
		e.ornamenticon        = 0;
		e.ornamentidfile      = 0;
		e.ornament_hero_model = 0;

		return e;
	}

	static CharacterCorpseItems GetCharacterCorpseItems(
		const std::vector<CharacterCorpseItems> &character_corpse_itemss,
		int character_corpse_items_id
	)
	{
		for (auto &character_corpse_items : character_corpse_itemss) {
			if (character_corpse_items.corpse_id == character_corpse_items_id) {
				return character_corpse_items;
			}
		}

		return NewEntity();
	}

	static CharacterCorpseItems FindOne(
		Database& db,
		int character_corpse_items_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				character_corpse_items_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterCorpseItems e{};

			e.corpse_id           = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.equip_slot          = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.item_id             = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.charges             = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.aug_1               = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.aug_2               = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.aug_3               = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.aug_4               = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.aug_5               = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.aug_6               = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.attuned             = row[10] ? static_cast<int16_t>(atoi(row[10])) : 0;
			e.custom_data         = row[11] ? row[11] : "";
			e.ornamenticon        = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.ornamentidfile      = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.ornament_hero_model = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_corpse_items_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_corpse_items_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CharacterCorpseItems &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.corpse_id));
		v.push_back(columns[1] + " = " + std::to_string(e.equip_slot));
		v.push_back(columns[2] + " = " + std::to_string(e.item_id));
		v.push_back(columns[3] + " = " + std::to_string(e.charges));
		v.push_back(columns[4] + " = " + std::to_string(e.aug_1));
		v.push_back(columns[5] + " = " + std::to_string(e.aug_2));
		v.push_back(columns[6] + " = " + std::to_string(e.aug_3));
		v.push_back(columns[7] + " = " + std::to_string(e.aug_4));
		v.push_back(columns[8] + " = " + std::to_string(e.aug_5));
		v.push_back(columns[9] + " = " + std::to_string(e.aug_6));
		v.push_back(columns[10] + " = " + std::to_string(e.attuned));
		v.push_back(columns[11] + " = '" + Strings::Escape(e.custom_data) + "'");
		v.push_back(columns[12] + " = " + std::to_string(e.ornamenticon));
		v.push_back(columns[13] + " = " + std::to_string(e.ornamentidfile));
		v.push_back(columns[14] + " = " + std::to_string(e.ornament_hero_model));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.corpse_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterCorpseItems InsertOne(
		Database& db,
		CharacterCorpseItems e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.corpse_id));
		v.push_back(std::to_string(e.equip_slot));
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.charges));
		v.push_back(std::to_string(e.aug_1));
		v.push_back(std::to_string(e.aug_2));
		v.push_back(std::to_string(e.aug_3));
		v.push_back(std::to_string(e.aug_4));
		v.push_back(std::to_string(e.aug_5));
		v.push_back(std::to_string(e.aug_6));
		v.push_back(std::to_string(e.attuned));
		v.push_back("'" + Strings::Escape(e.custom_data) + "'");
		v.push_back(std::to_string(e.ornamenticon));
		v.push_back(std::to_string(e.ornamentidfile));
		v.push_back(std::to_string(e.ornament_hero_model));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.corpse_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<CharacterCorpseItems> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.corpse_id));
			v.push_back(std::to_string(e.equip_slot));
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.charges));
			v.push_back(std::to_string(e.aug_1));
			v.push_back(std::to_string(e.aug_2));
			v.push_back(std::to_string(e.aug_3));
			v.push_back(std::to_string(e.aug_4));
			v.push_back(std::to_string(e.aug_5));
			v.push_back(std::to_string(e.aug_6));
			v.push_back(std::to_string(e.attuned));
			v.push_back("'" + Strings::Escape(e.custom_data) + "'");
			v.push_back(std::to_string(e.ornamenticon));
			v.push_back(std::to_string(e.ornamentidfile));
			v.push_back(std::to_string(e.ornament_hero_model));

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

	static std::vector<CharacterCorpseItems> All(Database& db)
	{
		std::vector<CharacterCorpseItems> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterCorpseItems e{};

			e.corpse_id           = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.equip_slot          = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.item_id             = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.charges             = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.aug_1               = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.aug_2               = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.aug_3               = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.aug_4               = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.aug_5               = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.aug_6               = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.attuned             = row[10] ? static_cast<int16_t>(atoi(row[10])) : 0;
			e.custom_data         = row[11] ? row[11] : "";
			e.ornamenticon        = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.ornamentidfile      = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.ornament_hero_model = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CharacterCorpseItems> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CharacterCorpseItems> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterCorpseItems e{};

			e.corpse_id           = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.equip_slot          = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.item_id             = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.charges             = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.aug_1               = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.aug_2               = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.aug_3               = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.aug_4               = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.aug_5               = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.aug_6               = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.attuned             = row[10] ? static_cast<int16_t>(atoi(row[10])) : 0;
			e.custom_data         = row[11] ? row[11] : "";
			e.ornamenticon        = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.ornamentidfile      = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.ornament_hero_model = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 0;

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
		const CharacterCorpseItems &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.corpse_id));
		v.push_back(std::to_string(e.equip_slot));
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.charges));
		v.push_back(std::to_string(e.aug_1));
		v.push_back(std::to_string(e.aug_2));
		v.push_back(std::to_string(e.aug_3));
		v.push_back(std::to_string(e.aug_4));
		v.push_back(std::to_string(e.aug_5));
		v.push_back(std::to_string(e.aug_6));
		v.push_back(std::to_string(e.attuned));
		v.push_back("'" + Strings::Escape(e.custom_data) + "'");
		v.push_back(std::to_string(e.ornamenticon));
		v.push_back(std::to_string(e.ornamentidfile));
		v.push_back(std::to_string(e.ornament_hero_model));

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
		const std::vector<CharacterCorpseItems> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.corpse_id));
			v.push_back(std::to_string(e.equip_slot));
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.charges));
			v.push_back(std::to_string(e.aug_1));
			v.push_back(std::to_string(e.aug_2));
			v.push_back(std::to_string(e.aug_3));
			v.push_back(std::to_string(e.aug_4));
			v.push_back(std::to_string(e.aug_5));
			v.push_back(std::to_string(e.aug_6));
			v.push_back(std::to_string(e.attuned));
			v.push_back("'" + Strings::Escape(e.custom_data) + "'");
			v.push_back(std::to_string(e.ornamenticon));
			v.push_back(std::to_string(e.ornamentidfile));
			v.push_back(std::to_string(e.ornament_hero_model));

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
