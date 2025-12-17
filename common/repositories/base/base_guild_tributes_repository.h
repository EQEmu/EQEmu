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


class BaseGuildTributesRepository {
public:
	struct GuildTributes {
		uint32_t guild_id;
		uint32_t tribute_id_1;
		uint32_t tribute_id_1_tier;
		uint32_t tribute_id_2;
		uint32_t tribute_id_2_tier;
		uint32_t time_remaining;
		uint32_t enabled;
	};

	static std::string PrimaryKey()
	{
		return std::string("guild_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"guild_id",
			"tribute_id_1",
			"tribute_id_1_tier",
			"tribute_id_2",
			"tribute_id_2_tier",
			"time_remaining",
			"enabled",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"guild_id",
			"tribute_id_1",
			"tribute_id_1_tier",
			"tribute_id_2",
			"tribute_id_2_tier",
			"time_remaining",
			"enabled",
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
		return std::string("guild_tributes");
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

	static GuildTributes NewEntity()
	{
		GuildTributes e{};

		e.guild_id          = 0;
		e.tribute_id_1      = 0;
		e.tribute_id_1_tier = 0;
		e.tribute_id_2      = 0;
		e.tribute_id_2_tier = 0;
		e.time_remaining    = 0;
		e.enabled           = 0;

		return e;
	}

	static GuildTributes GetGuildTributes(
		const std::vector<GuildTributes> &guild_tributess,
		int guild_tributes_id
	)
	{
		for (auto &guild_tributes : guild_tributess) {
			if (guild_tributes.guild_id == guild_tributes_id) {
				return guild_tributes;
			}
		}

		return NewEntity();
	}

	static GuildTributes FindOne(
		Database& db,
		int guild_tributes_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				guild_tributes_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			GuildTributes e{};

			e.guild_id          = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.tribute_id_1      = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.tribute_id_1_tier = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.tribute_id_2      = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));
			e.tribute_id_2_tier = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));
			e.time_remaining    = static_cast<uint32_t>(strtoul(row[5], nullptr, 10));
			e.enabled           = static_cast<uint32_t>(strtoul(row[6], nullptr, 10));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int guild_tributes_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				guild_tributes_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const GuildTributes &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.guild_id));
		v.push_back(columns[1] + " = " + std::to_string(e.tribute_id_1));
		v.push_back(columns[2] + " = " + std::to_string(e.tribute_id_1_tier));
		v.push_back(columns[3] + " = " + std::to_string(e.tribute_id_2));
		v.push_back(columns[4] + " = " + std::to_string(e.tribute_id_2_tier));
		v.push_back(columns[5] + " = " + std::to_string(e.time_remaining));
		v.push_back(columns[6] + " = " + std::to_string(e.enabled));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.guild_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static GuildTributes InsertOne(
		Database& db,
		GuildTributes e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.guild_id));
		v.push_back(std::to_string(e.tribute_id_1));
		v.push_back(std::to_string(e.tribute_id_1_tier));
		v.push_back(std::to_string(e.tribute_id_2));
		v.push_back(std::to_string(e.tribute_id_2_tier));
		v.push_back(std::to_string(e.time_remaining));
		v.push_back(std::to_string(e.enabled));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.guild_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<GuildTributes> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.guild_id));
			v.push_back(std::to_string(e.tribute_id_1));
			v.push_back(std::to_string(e.tribute_id_1_tier));
			v.push_back(std::to_string(e.tribute_id_2));
			v.push_back(std::to_string(e.tribute_id_2_tier));
			v.push_back(std::to_string(e.time_remaining));
			v.push_back(std::to_string(e.enabled));

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

	static std::vector<GuildTributes> All(Database& db)
	{
		std::vector<GuildTributes> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GuildTributes e{};

			e.guild_id          = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.tribute_id_1      = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.tribute_id_1_tier = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.tribute_id_2      = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));
			e.tribute_id_2_tier = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));
			e.time_remaining    = static_cast<uint32_t>(strtoul(row[5], nullptr, 10));
			e.enabled           = static_cast<uint32_t>(strtoul(row[6], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<GuildTributes> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<GuildTributes> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GuildTributes e{};

			e.guild_id          = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.tribute_id_1      = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.tribute_id_1_tier = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.tribute_id_2      = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));
			e.tribute_id_2_tier = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));
			e.time_remaining    = static_cast<uint32_t>(strtoul(row[5], nullptr, 10));
			e.enabled           = static_cast<uint32_t>(strtoul(row[6], nullptr, 10));

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
		const GuildTributes &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.guild_id));
		v.push_back(std::to_string(e.tribute_id_1));
		v.push_back(std::to_string(e.tribute_id_1_tier));
		v.push_back(std::to_string(e.tribute_id_2));
		v.push_back(std::to_string(e.tribute_id_2_tier));
		v.push_back(std::to_string(e.time_remaining));
		v.push_back(std::to_string(e.enabled));

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
		const std::vector<GuildTributes> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.guild_id));
			v.push_back(std::to_string(e.tribute_id_1));
			v.push_back(std::to_string(e.tribute_id_1_tier));
			v.push_back(std::to_string(e.tribute_id_2));
			v.push_back(std::to_string(e.tribute_id_2_tier));
			v.push_back(std::to_string(e.time_remaining));
			v.push_back(std::to_string(e.enabled));

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
