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

class BaseBotBlockedBuffsRepository {
public:
	struct BotBlockedBuffs {
		uint32_t bot_id;
		uint32_t spell_id;
		uint8_t  blocked;
		uint8_t  blocked_pet;
	};

	static std::string PrimaryKey()
	{
		return std::string("bot_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"bot_id",
			"spell_id",
			"blocked",
			"blocked_pet",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"bot_id",
			"spell_id",
			"blocked",
			"blocked_pet",
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
		return std::string("bot_blocked_buffs");
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

	static BotBlockedBuffs NewEntity()
	{
		BotBlockedBuffs e{};

		e.bot_id      = 0;
		e.spell_id    = 0;
		e.blocked     = 0;
		e.blocked_pet = 0;

		return e;
	}

	static BotBlockedBuffs GetBotBlockedBuffs(
		const std::vector<BotBlockedBuffs> &bot_blocked_buffss,
		int bot_blocked_buffs_id
	)
	{
		for (auto &bot_blocked_buffs : bot_blocked_buffss) {
			if (bot_blocked_buffs.bot_id == bot_blocked_buffs_id) {
				return bot_blocked_buffs;
			}
		}

		return NewEntity();
	}

	static BotBlockedBuffs FindOne(
		Database& db,
		int bot_blocked_buffs_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				bot_blocked_buffs_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			BotBlockedBuffs e{};

			e.bot_id      = row[0] ? static_cast<uint32_t>(atoi(row[0])) : 0;
			e.spell_id    = row[1] ? static_cast<uint32_t>(atoi(row[1])) : 0;
			e.blocked     = row[2] ? static_cast<uint8_t>(atoi(row[2])) : 0;
			e.blocked_pet = row[3] ? static_cast<uint8_t>(atoi(row[3])) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int bot_blocked_buffs_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				bot_blocked_buffs_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const BotBlockedBuffs &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.bot_id));
		v.push_back(columns[1] + " = " + std::to_string(e.spell_id));
		v.push_back(columns[2] + " = " + std::to_string(e.blocked));
		v.push_back(columns[3] + " = " + std::to_string(e.blocked_pet));

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

	static BotBlockedBuffs InsertOne(
		Database& db,
		BotBlockedBuffs e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.bot_id));
		v.push_back(std::to_string(e.spell_id));
		v.push_back(std::to_string(e.blocked));
		v.push_back(std::to_string(e.blocked_pet));

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
		const std::vector<BotBlockedBuffs> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.bot_id));
			v.push_back(std::to_string(e.spell_id));
			v.push_back(std::to_string(e.blocked));
			v.push_back(std::to_string(e.blocked_pet));

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

	static std::vector<BotBlockedBuffs> All(Database& db)
	{
		std::vector<BotBlockedBuffs> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotBlockedBuffs e{};

			e.bot_id      = row[0] ? static_cast<uint32_t>(atoi(row[0])) : 0;
			e.spell_id    = row[1] ? static_cast<uint32_t>(atoi(row[1])) : 0;
			e.blocked     = row[2] ? static_cast<uint8_t>(atoi(row[2])) : 0;
			e.blocked_pet = row[3] ? static_cast<uint8_t>(atoi(row[3])) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<BotBlockedBuffs> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<BotBlockedBuffs> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotBlockedBuffs e{};

			e.bot_id      = row[0] ? static_cast<uint32_t>(atoi(row[0])) : 0;
			e.spell_id    = row[1] ? static_cast<uint32_t>(atoi(row[1])) : 0;
			e.blocked     = row[2] ? static_cast<uint8_t>(atoi(row[2])) : 0;
			e.blocked_pet = row[3] ? static_cast<uint8_t>(atoi(row[3])) : 0;

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
		const BotBlockedBuffs &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.bot_id));
		v.push_back(std::to_string(e.spell_id));
		v.push_back(std::to_string(e.blocked));
		v.push_back(std::to_string(e.blocked_pet));

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
		const std::vector<BotBlockedBuffs> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.bot_id));
			v.push_back(std::to_string(e.spell_id));
			v.push_back(std::to_string(e.blocked));
			v.push_back(std::to_string(e.blocked_pet));

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
