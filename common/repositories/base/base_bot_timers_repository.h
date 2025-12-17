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

class BaseBotTimersRepository {
public:
	struct BotTimers {
		uint32_t bot_id;
		uint32_t timer_id;
		uint32_t timer_value;
		uint32_t recast_time;
		uint8_t  is_spell;
		uint8_t  is_disc;
		uint32_t spell_id;
		uint8_t  is_item;
		uint32_t item_id;
	};

	static std::string PrimaryKey()
	{
		return std::string("bot_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"bot_id",
			"timer_id",
			"timer_value",
			"recast_time",
			"is_spell",
			"is_disc",
			"spell_id",
			"is_item",
			"item_id",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"bot_id",
			"timer_id",
			"timer_value",
			"recast_time",
			"is_spell",
			"is_disc",
			"spell_id",
			"is_item",
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
		return std::string("bot_timers");
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

	static BotTimers NewEntity()
	{
		BotTimers e{};

		e.bot_id      = 0;
		e.timer_id    = 0;
		e.timer_value = 0;
		e.recast_time = 0;
		e.is_spell    = 0;
		e.is_disc     = 0;
		e.spell_id    = 0;
		e.is_item     = 0;
		e.item_id     = 0;

		return e;
	}

	static BotTimers GetBotTimers(
		const std::vector<BotTimers> &bot_timerss,
		int bot_timers_id
	)
	{
		for (auto &bot_timers : bot_timerss) {
			if (bot_timers.bot_id == bot_timers_id) {
				return bot_timers;
			}
		}

		return NewEntity();
	}

	static BotTimers FindOne(
		Database& db,
		int bot_timers_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				bot_timers_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			BotTimers e{};

			e.bot_id      = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.timer_id    = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.timer_value = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.recast_time = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.is_spell    = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.is_disc     = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.spell_id    = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.is_item     = row[7] ? static_cast<uint8_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.item_id     = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int bot_timers_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				bot_timers_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const BotTimers &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.bot_id));
		v.push_back(columns[1] + " = " + std::to_string(e.timer_id));
		v.push_back(columns[2] + " = " + std::to_string(e.timer_value));
		v.push_back(columns[3] + " = " + std::to_string(e.recast_time));
		v.push_back(columns[4] + " = " + std::to_string(e.is_spell));
		v.push_back(columns[5] + " = " + std::to_string(e.is_disc));
		v.push_back(columns[6] + " = " + std::to_string(e.spell_id));
		v.push_back(columns[7] + " = " + std::to_string(e.is_item));
		v.push_back(columns[8] + " = " + std::to_string(e.item_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.bot_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static BotTimers InsertOne(
		Database& db,
		BotTimers e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.bot_id));
		v.push_back(std::to_string(e.timer_id));
		v.push_back(std::to_string(e.timer_value));
		v.push_back(std::to_string(e.recast_time));
		v.push_back(std::to_string(e.is_spell));
		v.push_back(std::to_string(e.is_disc));
		v.push_back(std::to_string(e.spell_id));
		v.push_back(std::to_string(e.is_item));
		v.push_back(std::to_string(e.item_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.bot_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<BotTimers> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.bot_id));
			v.push_back(std::to_string(e.timer_id));
			v.push_back(std::to_string(e.timer_value));
			v.push_back(std::to_string(e.recast_time));
			v.push_back(std::to_string(e.is_spell));
			v.push_back(std::to_string(e.is_disc));
			v.push_back(std::to_string(e.spell_id));
			v.push_back(std::to_string(e.is_item));
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

	static std::vector<BotTimers> All(Database& db)
	{
		std::vector<BotTimers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotTimers e{};

			e.bot_id      = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.timer_id    = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.timer_value = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.recast_time = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.is_spell    = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.is_disc     = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.spell_id    = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.is_item     = row[7] ? static_cast<uint8_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.item_id     = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<BotTimers> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<BotTimers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotTimers e{};

			e.bot_id      = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.timer_id    = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.timer_value = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.recast_time = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.is_spell    = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.is_disc     = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.spell_id    = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.is_item     = row[7] ? static_cast<uint8_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.item_id     = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;

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
		const BotTimers &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.bot_id));
		v.push_back(std::to_string(e.timer_id));
		v.push_back(std::to_string(e.timer_value));
		v.push_back(std::to_string(e.recast_time));
		v.push_back(std::to_string(e.is_spell));
		v.push_back(std::to_string(e.is_disc));
		v.push_back(std::to_string(e.spell_id));
		v.push_back(std::to_string(e.is_item));
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
		const std::vector<BotTimers> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.bot_id));
			v.push_back(std::to_string(e.timer_id));
			v.push_back(std::to_string(e.timer_value));
			v.push_back(std::to_string(e.recast_time));
			v.push_back(std::to_string(e.is_spell));
			v.push_back(std::to_string(e.is_disc));
			v.push_back(std::to_string(e.spell_id));
			v.push_back(std::to_string(e.is_item));
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
