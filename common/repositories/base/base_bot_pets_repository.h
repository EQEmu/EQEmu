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

class BaseBotPetsRepository {
public:
	struct BotPets {
		uint32_t    pets_index;
		uint32_t    spell_id;
		uint32_t    bot_id;
		std::string name;
		int32_t     mana;
		int32_t     hp;
	};

	static std::string PrimaryKey()
	{
		return std::string("pets_index");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"pets_index",
			"spell_id",
			"bot_id",
			"name",
			"mana",
			"hp",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"pets_index",
			"spell_id",
			"bot_id",
			"name",
			"mana",
			"hp",
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
		return std::string("bot_pets");
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

	static BotPets NewEntity()
	{
		BotPets e{};

		e.pets_index = 0;
		e.spell_id   = 0;
		e.bot_id     = 0;
		e.name       = "";
		e.mana       = 0;
		e.hp         = 0;

		return e;
	}

	static BotPets GetBotPets(
		const std::vector<BotPets> &bot_petss,
		int bot_pets_id
	)
	{
		for (auto &bot_pets : bot_petss) {
			if (bot_pets.pets_index == bot_pets_id) {
				return bot_pets;
			}
		}

		return NewEntity();
	}

	static BotPets FindOne(
		Database& db,
		int bot_pets_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				bot_pets_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			BotPets e{};

			e.pets_index = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.spell_id   = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.bot_id     = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.name       = row[3] ? row[3] : "";
			e.mana       = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.hp         = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int bot_pets_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				bot_pets_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const BotPets &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.spell_id));
		v.push_back(columns[2] + " = " + std::to_string(e.bot_id));
		v.push_back(columns[3] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[4] + " = " + std::to_string(e.mana));
		v.push_back(columns[5] + " = " + std::to_string(e.hp));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.pets_index
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static BotPets InsertOne(
		Database& db,
		BotPets e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.pets_index));
		v.push_back(std::to_string(e.spell_id));
		v.push_back(std::to_string(e.bot_id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.mana));
		v.push_back(std::to_string(e.hp));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.pets_index = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<BotPets> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.pets_index));
			v.push_back(std::to_string(e.spell_id));
			v.push_back(std::to_string(e.bot_id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.mana));
			v.push_back(std::to_string(e.hp));

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

	static std::vector<BotPets> All(Database& db)
	{
		std::vector<BotPets> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotPets e{};

			e.pets_index = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.spell_id   = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.bot_id     = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.name       = row[3] ? row[3] : "";
			e.mana       = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.hp         = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<BotPets> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<BotPets> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotPets e{};

			e.pets_index = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.spell_id   = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.bot_id     = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.name       = row[3] ? row[3] : "";
			e.mana       = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.hp         = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;

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
		const BotPets &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.pets_index));
		v.push_back(std::to_string(e.spell_id));
		v.push_back(std::to_string(e.bot_id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.mana));
		v.push_back(std::to_string(e.hp));

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
		const std::vector<BotPets> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.pets_index));
			v.push_back(std::to_string(e.spell_id));
			v.push_back(std::to_string(e.bot_id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.mana));
			v.push_back(std::to_string(e.hp));

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
