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

class BaseBotInventoriesRepository {
public:
	struct BotInventories {
		uint32_t    inventories_index;
		uint32_t    bot_id;
		uint32_t    slot_id;
		uint32_t    item_id;
		uint16_t    inst_charges;
		uint32_t    inst_color;
		uint8_t     inst_no_drop;
		std::string inst_custom_data;
		uint32_t    ornament_icon;
		uint32_t    ornament_id_file;
		int32_t     ornament_hero_model;
		uint32_t    augment_1;
		uint32_t    augment_2;
		uint32_t    augment_3;
		uint32_t    augment_4;
		uint32_t    augment_5;
		uint32_t    augment_6;
	};

	static std::string PrimaryKey()
	{
		return std::string("inventories_index");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"inventories_index",
			"bot_id",
			"slot_id",
			"item_id",
			"inst_charges",
			"inst_color",
			"inst_no_drop",
			"inst_custom_data",
			"ornament_icon",
			"ornament_id_file",
			"ornament_hero_model",
			"augment_1",
			"augment_2",
			"augment_3",
			"augment_4",
			"augment_5",
			"augment_6",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"inventories_index",
			"bot_id",
			"slot_id",
			"item_id",
			"inst_charges",
			"inst_color",
			"inst_no_drop",
			"inst_custom_data",
			"ornament_icon",
			"ornament_id_file",
			"ornament_hero_model",
			"augment_1",
			"augment_2",
			"augment_3",
			"augment_4",
			"augment_5",
			"augment_6",
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
		return std::string("bot_inventories");
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

	static BotInventories NewEntity()
	{
		BotInventories e{};

		e.inventories_index   = 0;
		e.bot_id              = 0;
		e.slot_id             = 0;
		e.item_id             = 0;
		e.inst_charges        = 0;
		e.inst_color          = 0;
		e.inst_no_drop        = 0;
		e.inst_custom_data    = "";
		e.ornament_icon       = 0;
		e.ornament_id_file    = 0;
		e.ornament_hero_model = 0;
		e.augment_1           = 0;
		e.augment_2           = 0;
		e.augment_3           = 0;
		e.augment_4           = 0;
		e.augment_5           = 0;
		e.augment_6           = 0;

		return e;
	}

	static BotInventories GetBotInventories(
		const std::vector<BotInventories> &bot_inventoriess,
		int bot_inventories_id
	)
	{
		for (auto &bot_inventories : bot_inventoriess) {
			if (bot_inventories.inventories_index == bot_inventories_id) {
				return bot_inventories;
			}
		}

		return NewEntity();
	}

	static BotInventories FindOne(
		Database& db,
		int bot_inventories_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				bot_inventories_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			BotInventories e{};

			e.inventories_index   = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.bot_id              = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.slot_id             = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.item_id             = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.inst_charges        = row[4] ? static_cast<uint16_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.inst_color          = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.inst_no_drop        = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.inst_custom_data    = row[7] ? row[7] : "";
			e.ornament_icon       = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.ornament_id_file    = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.ornament_hero_model = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.augment_1           = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.augment_2           = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.augment_3           = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.augment_4           = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.augment_5           = row[15] ? static_cast<uint32_t>(strtoul(row[15], nullptr, 10)) : 0;
			e.augment_6           = row[16] ? static_cast<uint32_t>(strtoul(row[16], nullptr, 10)) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int bot_inventories_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				bot_inventories_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const BotInventories &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.bot_id));
		v.push_back(columns[2] + " = " + std::to_string(e.slot_id));
		v.push_back(columns[3] + " = " + std::to_string(e.item_id));
		v.push_back(columns[4] + " = " + std::to_string(e.inst_charges));
		v.push_back(columns[5] + " = " + std::to_string(e.inst_color));
		v.push_back(columns[6] + " = " + std::to_string(e.inst_no_drop));
		v.push_back(columns[7] + " = '" + Strings::Escape(e.inst_custom_data) + "'");
		v.push_back(columns[8] + " = " + std::to_string(e.ornament_icon));
		v.push_back(columns[9] + " = " + std::to_string(e.ornament_id_file));
		v.push_back(columns[10] + " = " + std::to_string(e.ornament_hero_model));
		v.push_back(columns[11] + " = " + std::to_string(e.augment_1));
		v.push_back(columns[12] + " = " + std::to_string(e.augment_2));
		v.push_back(columns[13] + " = " + std::to_string(e.augment_3));
		v.push_back(columns[14] + " = " + std::to_string(e.augment_4));
		v.push_back(columns[15] + " = " + std::to_string(e.augment_5));
		v.push_back(columns[16] + " = " + std::to_string(e.augment_6));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.inventories_index
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static BotInventories InsertOne(
		Database& db,
		BotInventories e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.inventories_index));
		v.push_back(std::to_string(e.bot_id));
		v.push_back(std::to_string(e.slot_id));
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.inst_charges));
		v.push_back(std::to_string(e.inst_color));
		v.push_back(std::to_string(e.inst_no_drop));
		v.push_back("'" + Strings::Escape(e.inst_custom_data) + "'");
		v.push_back(std::to_string(e.ornament_icon));
		v.push_back(std::to_string(e.ornament_id_file));
		v.push_back(std::to_string(e.ornament_hero_model));
		v.push_back(std::to_string(e.augment_1));
		v.push_back(std::to_string(e.augment_2));
		v.push_back(std::to_string(e.augment_3));
		v.push_back(std::to_string(e.augment_4));
		v.push_back(std::to_string(e.augment_5));
		v.push_back(std::to_string(e.augment_6));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.inventories_index = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<BotInventories> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.inventories_index));
			v.push_back(std::to_string(e.bot_id));
			v.push_back(std::to_string(e.slot_id));
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.inst_charges));
			v.push_back(std::to_string(e.inst_color));
			v.push_back(std::to_string(e.inst_no_drop));
			v.push_back("'" + Strings::Escape(e.inst_custom_data) + "'");
			v.push_back(std::to_string(e.ornament_icon));
			v.push_back(std::to_string(e.ornament_id_file));
			v.push_back(std::to_string(e.ornament_hero_model));
			v.push_back(std::to_string(e.augment_1));
			v.push_back(std::to_string(e.augment_2));
			v.push_back(std::to_string(e.augment_3));
			v.push_back(std::to_string(e.augment_4));
			v.push_back(std::to_string(e.augment_5));
			v.push_back(std::to_string(e.augment_6));

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

	static std::vector<BotInventories> All(Database& db)
	{
		std::vector<BotInventories> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotInventories e{};

			e.inventories_index   = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.bot_id              = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.slot_id             = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.item_id             = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.inst_charges        = row[4] ? static_cast<uint16_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.inst_color          = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.inst_no_drop        = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.inst_custom_data    = row[7] ? row[7] : "";
			e.ornament_icon       = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.ornament_id_file    = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.ornament_hero_model = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.augment_1           = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.augment_2           = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.augment_3           = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.augment_4           = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.augment_5           = row[15] ? static_cast<uint32_t>(strtoul(row[15], nullptr, 10)) : 0;
			e.augment_6           = row[16] ? static_cast<uint32_t>(strtoul(row[16], nullptr, 10)) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<BotInventories> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<BotInventories> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotInventories e{};

			e.inventories_index   = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.bot_id              = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.slot_id             = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.item_id             = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.inst_charges        = row[4] ? static_cast<uint16_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.inst_color          = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.inst_no_drop        = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.inst_custom_data    = row[7] ? row[7] : "";
			e.ornament_icon       = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.ornament_id_file    = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.ornament_hero_model = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.augment_1           = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.augment_2           = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.augment_3           = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.augment_4           = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.augment_5           = row[15] ? static_cast<uint32_t>(strtoul(row[15], nullptr, 10)) : 0;
			e.augment_6           = row[16] ? static_cast<uint32_t>(strtoul(row[16], nullptr, 10)) : 0;

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
		const BotInventories &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.inventories_index));
		v.push_back(std::to_string(e.bot_id));
		v.push_back(std::to_string(e.slot_id));
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.inst_charges));
		v.push_back(std::to_string(e.inst_color));
		v.push_back(std::to_string(e.inst_no_drop));
		v.push_back("'" + Strings::Escape(e.inst_custom_data) + "'");
		v.push_back(std::to_string(e.ornament_icon));
		v.push_back(std::to_string(e.ornament_id_file));
		v.push_back(std::to_string(e.ornament_hero_model));
		v.push_back(std::to_string(e.augment_1));
		v.push_back(std::to_string(e.augment_2));
		v.push_back(std::to_string(e.augment_3));
		v.push_back(std::to_string(e.augment_4));
		v.push_back(std::to_string(e.augment_5));
		v.push_back(std::to_string(e.augment_6));

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
		const std::vector<BotInventories> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.inventories_index));
			v.push_back(std::to_string(e.bot_id));
			v.push_back(std::to_string(e.slot_id));
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.inst_charges));
			v.push_back(std::to_string(e.inst_color));
			v.push_back(std::to_string(e.inst_no_drop));
			v.push_back("'" + Strings::Escape(e.inst_custom_data) + "'");
			v.push_back(std::to_string(e.ornament_icon));
			v.push_back(std::to_string(e.ornament_id_file));
			v.push_back(std::to_string(e.ornament_hero_model));
			v.push_back(std::to_string(e.augment_1));
			v.push_back(std::to_string(e.augment_2));
			v.push_back(std::to_string(e.augment_3));
			v.push_back(std::to_string(e.augment_4));
			v.push_back(std::to_string(e.augment_5));
			v.push_back(std::to_string(e.augment_6));

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
