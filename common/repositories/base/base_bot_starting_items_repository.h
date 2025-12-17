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

class BaseBotStartingItemsRepository {
public:
	struct BotStartingItems {
		uint32_t    id;
		uint32_t    races;
		uint32_t    classes;
		uint32_t    item_id;
		uint8_t     item_charges;
		uint32_t    augment_one;
		uint32_t    augment_two;
		uint32_t    augment_three;
		uint32_t    augment_four;
		uint32_t    augment_five;
		uint32_t    augment_six;
		uint8_t     min_status;
		int32_t     slot_id;
		int8_t      min_expansion;
		int8_t      max_expansion;
		std::string content_flags;
		std::string content_flags_disabled;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"races",
			"classes",
			"item_id",
			"item_charges",
			"augment_one",
			"augment_two",
			"augment_three",
			"augment_four",
			"augment_five",
			"augment_six",
			"min_status",
			"slot_id",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"races",
			"classes",
			"item_id",
			"item_charges",
			"augment_one",
			"augment_two",
			"augment_three",
			"augment_four",
			"augment_five",
			"augment_six",
			"min_status",
			"slot_id",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
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
		return std::string("bot_starting_items");
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

	static BotStartingItems NewEntity()
	{
		BotStartingItems e{};

		e.id                     = 0;
		e.races                  = 0;
		e.classes                = 0;
		e.item_id                = 0;
		e.item_charges           = 1;
		e.augment_one            = 0;
		e.augment_two            = 0;
		e.augment_three          = 0;
		e.augment_four           = 0;
		e.augment_five           = 0;
		e.augment_six            = 0;
		e.min_status             = 0;
		e.slot_id                = -1;
		e.min_expansion          = -1;
		e.max_expansion          = -1;
		e.content_flags          = "";
		e.content_flags_disabled = "";

		return e;
	}

	static BotStartingItems GetBotStartingItems(
		const std::vector<BotStartingItems> &bot_starting_itemss,
		int bot_starting_items_id
	)
	{
		for (auto &bot_starting_items : bot_starting_itemss) {
			if (bot_starting_items.id == bot_starting_items_id) {
				return bot_starting_items;
			}
		}

		return NewEntity();
	}

	static BotStartingItems FindOne(
		Database& db,
		int bot_starting_items_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				bot_starting_items_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			BotStartingItems e{};

			e.id                     = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.races                  = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.classes                = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.item_id                = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.item_charges           = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 1;
			e.augment_one            = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.augment_two            = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.augment_three          = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.augment_four           = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.augment_five           = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.augment_six            = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.min_status             = row[11] ? static_cast<uint8_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.slot_id                = row[12] ? static_cast<int32_t>(atoi(row[12])) : -1;
			e.min_expansion          = row[13] ? static_cast<int8_t>(atoi(row[13])) : -1;
			e.max_expansion          = row[14] ? static_cast<int8_t>(atoi(row[14])) : -1;
			e.content_flags          = row[15] ? row[15] : "";
			e.content_flags_disabled = row[16] ? row[16] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int bot_starting_items_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				bot_starting_items_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const BotStartingItems &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.races));
		v.push_back(columns[2] + " = " + std::to_string(e.classes));
		v.push_back(columns[3] + " = " + std::to_string(e.item_id));
		v.push_back(columns[4] + " = " + std::to_string(e.item_charges));
		v.push_back(columns[5] + " = " + std::to_string(e.augment_one));
		v.push_back(columns[6] + " = " + std::to_string(e.augment_two));
		v.push_back(columns[7] + " = " + std::to_string(e.augment_three));
		v.push_back(columns[8] + " = " + std::to_string(e.augment_four));
		v.push_back(columns[9] + " = " + std::to_string(e.augment_five));
		v.push_back(columns[10] + " = " + std::to_string(e.augment_six));
		v.push_back(columns[11] + " = " + std::to_string(e.min_status));
		v.push_back(columns[12] + " = " + std::to_string(e.slot_id));
		v.push_back(columns[13] + " = " + std::to_string(e.min_expansion));
		v.push_back(columns[14] + " = " + std::to_string(e.max_expansion));
		v.push_back(columns[15] + " = '" + Strings::Escape(e.content_flags) + "'");
		v.push_back(columns[16] + " = '" + Strings::Escape(e.content_flags_disabled) + "'");

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

	static BotStartingItems InsertOne(
		Database& db,
		BotStartingItems e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.races));
		v.push_back(std::to_string(e.classes));
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.item_charges));
		v.push_back(std::to_string(e.augment_one));
		v.push_back(std::to_string(e.augment_two));
		v.push_back(std::to_string(e.augment_three));
		v.push_back(std::to_string(e.augment_four));
		v.push_back(std::to_string(e.augment_five));
		v.push_back(std::to_string(e.augment_six));
		v.push_back(std::to_string(e.min_status));
		v.push_back(std::to_string(e.slot_id));
		v.push_back(std::to_string(e.min_expansion));
		v.push_back(std::to_string(e.max_expansion));
		v.push_back("'" + Strings::Escape(e.content_flags) + "'");
		v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

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
		const std::vector<BotStartingItems> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.races));
			v.push_back(std::to_string(e.classes));
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.item_charges));
			v.push_back(std::to_string(e.augment_one));
			v.push_back(std::to_string(e.augment_two));
			v.push_back(std::to_string(e.augment_three));
			v.push_back(std::to_string(e.augment_four));
			v.push_back(std::to_string(e.augment_five));
			v.push_back(std::to_string(e.augment_six));
			v.push_back(std::to_string(e.min_status));
			v.push_back(std::to_string(e.slot_id));
			v.push_back(std::to_string(e.min_expansion));
			v.push_back(std::to_string(e.max_expansion));
			v.push_back("'" + Strings::Escape(e.content_flags) + "'");
			v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

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

	static std::vector<BotStartingItems> All(Database& db)
	{
		std::vector<BotStartingItems> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotStartingItems e{};

			e.id                     = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.races                  = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.classes                = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.item_id                = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.item_charges           = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 1;
			e.augment_one            = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.augment_two            = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.augment_three          = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.augment_four           = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.augment_five           = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.augment_six            = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.min_status             = row[11] ? static_cast<uint8_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.slot_id                = row[12] ? static_cast<int32_t>(atoi(row[12])) : -1;
			e.min_expansion          = row[13] ? static_cast<int8_t>(atoi(row[13])) : -1;
			e.max_expansion          = row[14] ? static_cast<int8_t>(atoi(row[14])) : -1;
			e.content_flags          = row[15] ? row[15] : "";
			e.content_flags_disabled = row[16] ? row[16] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<BotStartingItems> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<BotStartingItems> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotStartingItems e{};

			e.id                     = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.races                  = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.classes                = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.item_id                = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.item_charges           = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 1;
			e.augment_one            = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.augment_two            = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.augment_three          = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.augment_four           = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.augment_five           = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.augment_six            = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.min_status             = row[11] ? static_cast<uint8_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.slot_id                = row[12] ? static_cast<int32_t>(atoi(row[12])) : -1;
			e.min_expansion          = row[13] ? static_cast<int8_t>(atoi(row[13])) : -1;
			e.max_expansion          = row[14] ? static_cast<int8_t>(atoi(row[14])) : -1;
			e.content_flags          = row[15] ? row[15] : "";
			e.content_flags_disabled = row[16] ? row[16] : "";

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
		const BotStartingItems &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.races));
		v.push_back(std::to_string(e.classes));
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.item_charges));
		v.push_back(std::to_string(e.augment_one));
		v.push_back(std::to_string(e.augment_two));
		v.push_back(std::to_string(e.augment_three));
		v.push_back(std::to_string(e.augment_four));
		v.push_back(std::to_string(e.augment_five));
		v.push_back(std::to_string(e.augment_six));
		v.push_back(std::to_string(e.min_status));
		v.push_back(std::to_string(e.slot_id));
		v.push_back(std::to_string(e.min_expansion));
		v.push_back(std::to_string(e.max_expansion));
		v.push_back("'" + Strings::Escape(e.content_flags) + "'");
		v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

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
		const std::vector<BotStartingItems> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.races));
			v.push_back(std::to_string(e.classes));
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.item_charges));
			v.push_back(std::to_string(e.augment_one));
			v.push_back(std::to_string(e.augment_two));
			v.push_back(std::to_string(e.augment_three));
			v.push_back(std::to_string(e.augment_four));
			v.push_back(std::to_string(e.augment_five));
			v.push_back(std::to_string(e.augment_six));
			v.push_back(std::to_string(e.min_status));
			v.push_back(std::to_string(e.slot_id));
			v.push_back(std::to_string(e.min_expansion));
			v.push_back(std::to_string(e.max_expansion));
			v.push_back("'" + Strings::Escape(e.content_flags) + "'");
			v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

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
