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

class BaseBotSpellSettingsRepository {
public:
	struct BotSpellSettings {
		uint32_t id;
		int32_t  bot_id;
		int16_t  spell_id;
		int16_t  priority;
		int16_t  min_hp;
		int16_t  max_hp;
		uint8_t  is_enabled;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"bot_id",
			"spell_id",
			"priority",
			"min_hp",
			"max_hp",
			"is_enabled",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"bot_id",
			"spell_id",
			"priority",
			"min_hp",
			"max_hp",
			"is_enabled",
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
		return std::string("bot_spell_settings");
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

	static BotSpellSettings NewEntity()
	{
		BotSpellSettings e{};

		e.id         = 0;
		e.bot_id     = 0;
		e.spell_id   = 0;
		e.priority   = 0;
		e.min_hp     = 0;
		e.max_hp     = 0;
		e.is_enabled = 1;

		return e;
	}

	static BotSpellSettings GetBotSpellSettings(
		const std::vector<BotSpellSettings> &bot_spell_settingss,
		int bot_spell_settings_id
	)
	{
		for (auto &bot_spell_settings : bot_spell_settingss) {
			if (bot_spell_settings.id == bot_spell_settings_id) {
				return bot_spell_settings;
			}
		}

		return NewEntity();
	}

	static BotSpellSettings FindOne(
		Database& db,
		int bot_spell_settings_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				bot_spell_settings_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			BotSpellSettings e{};

			e.id         = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.bot_id     = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.spell_id   = row[2] ? static_cast<int16_t>(atoi(row[2])) : 0;
			e.priority   = row[3] ? static_cast<int16_t>(atoi(row[3])) : 0;
			e.min_hp     = row[4] ? static_cast<int16_t>(atoi(row[4])) : 0;
			e.max_hp     = row[5] ? static_cast<int16_t>(atoi(row[5])) : 0;
			e.is_enabled = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 1;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int bot_spell_settings_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				bot_spell_settings_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const BotSpellSettings &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.bot_id));
		v.push_back(columns[2] + " = " + std::to_string(e.spell_id));
		v.push_back(columns[3] + " = " + std::to_string(e.priority));
		v.push_back(columns[4] + " = " + std::to_string(e.min_hp));
		v.push_back(columns[5] + " = " + std::to_string(e.max_hp));
		v.push_back(columns[6] + " = " + std::to_string(e.is_enabled));

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

	static BotSpellSettings InsertOne(
		Database& db,
		BotSpellSettings e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.bot_id));
		v.push_back(std::to_string(e.spell_id));
		v.push_back(std::to_string(e.priority));
		v.push_back(std::to_string(e.min_hp));
		v.push_back(std::to_string(e.max_hp));
		v.push_back(std::to_string(e.is_enabled));

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
		const std::vector<BotSpellSettings> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.bot_id));
			v.push_back(std::to_string(e.spell_id));
			v.push_back(std::to_string(e.priority));
			v.push_back(std::to_string(e.min_hp));
			v.push_back(std::to_string(e.max_hp));
			v.push_back(std::to_string(e.is_enabled));

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

	static std::vector<BotSpellSettings> All(Database& db)
	{
		std::vector<BotSpellSettings> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotSpellSettings e{};

			e.id         = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.bot_id     = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.spell_id   = row[2] ? static_cast<int16_t>(atoi(row[2])) : 0;
			e.priority   = row[3] ? static_cast<int16_t>(atoi(row[3])) : 0;
			e.min_hp     = row[4] ? static_cast<int16_t>(atoi(row[4])) : 0;
			e.max_hp     = row[5] ? static_cast<int16_t>(atoi(row[5])) : 0;
			e.is_enabled = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 1;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<BotSpellSettings> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<BotSpellSettings> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotSpellSettings e{};

			e.id         = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.bot_id     = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.spell_id   = row[2] ? static_cast<int16_t>(atoi(row[2])) : 0;
			e.priority   = row[3] ? static_cast<int16_t>(atoi(row[3])) : 0;
			e.min_hp     = row[4] ? static_cast<int16_t>(atoi(row[4])) : 0;
			e.max_hp     = row[5] ? static_cast<int16_t>(atoi(row[5])) : 0;
			e.is_enabled = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 1;

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
		const BotSpellSettings &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.bot_id));
		v.push_back(std::to_string(e.spell_id));
		v.push_back(std::to_string(e.priority));
		v.push_back(std::to_string(e.min_hp));
		v.push_back(std::to_string(e.max_hp));
		v.push_back(std::to_string(e.is_enabled));

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
		const std::vector<BotSpellSettings> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.bot_id));
			v.push_back(std::to_string(e.spell_id));
			v.push_back(std::to_string(e.priority));
			v.push_back(std::to_string(e.min_hp));
			v.push_back(std::to_string(e.max_hp));
			v.push_back(std::to_string(e.is_enabled));

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
