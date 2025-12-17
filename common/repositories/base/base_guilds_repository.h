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


class BaseGuildsRepository {
public:
	struct Guilds {
		int32_t     id;
		std::string name;
		int32_t     leader;
		int16_t     minstatus;
		std::string motd;
		uint32_t    tribute;
		std::string motd_setter;
		std::string channel;
		std::string url;
		uint32_t    favor;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"name",
			"leader",
			"minstatus",
			"motd",
			"tribute",
			"motd_setter",
			"channel",
			"url",
			"favor",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"name",
			"leader",
			"minstatus",
			"motd",
			"tribute",
			"motd_setter",
			"channel",
			"url",
			"favor",
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
		return std::string("guilds");
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

	static Guilds NewEntity()
	{
		Guilds e{};

		e.id          = 0;
		e.name        = "";
		e.leader      = 0;
		e.minstatus   = 0;
		e.motd        = "";
		e.tribute     = 0;
		e.motd_setter = "";
		e.channel     = "";
		e.url         = "";
		e.favor       = 0;

		return e;
	}

	static Guilds GetGuilds(
		const std::vector<Guilds> &guildss,
		int guilds_id
	)
	{
		for (auto &guilds : guildss) {
			if (guilds.id == guilds_id) {
				return guilds;
			}
		}

		return NewEntity();
	}

	static Guilds FindOne(
		Database& db,
		int guilds_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				guilds_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Guilds e{};

			e.id          = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.name        = row[1] ? row[1] : "";
			e.leader      = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.minstatus   = row[3] ? static_cast<int16_t>(atoi(row[3])) : 0;
			e.motd        = row[4] ? row[4] : "";
			e.tribute     = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.motd_setter = row[6] ? row[6] : "";
			e.channel     = row[7] ? row[7] : "";
			e.url         = row[8] ? row[8] : "";
			e.favor       = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int guilds_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				guilds_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Guilds &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[2] + " = " + std::to_string(e.leader));
		v.push_back(columns[3] + " = " + std::to_string(e.minstatus));
		v.push_back(columns[4] + " = '" + Strings::Escape(e.motd) + "'");
		v.push_back(columns[5] + " = " + std::to_string(e.tribute));
		v.push_back(columns[6] + " = '" + Strings::Escape(e.motd_setter) + "'");
		v.push_back(columns[7] + " = '" + Strings::Escape(e.channel) + "'");
		v.push_back(columns[8] + " = '" + Strings::Escape(e.url) + "'");
		v.push_back(columns[9] + " = " + std::to_string(e.favor));

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

	static Guilds InsertOne(
		Database& db,
		Guilds e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.leader));
		v.push_back(std::to_string(e.minstatus));
		v.push_back("'" + Strings::Escape(e.motd) + "'");
		v.push_back(std::to_string(e.tribute));
		v.push_back("'" + Strings::Escape(e.motd_setter) + "'");
		v.push_back("'" + Strings::Escape(e.channel) + "'");
		v.push_back("'" + Strings::Escape(e.url) + "'");
		v.push_back(std::to_string(e.favor));

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
		const std::vector<Guilds> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.leader));
			v.push_back(std::to_string(e.minstatus));
			v.push_back("'" + Strings::Escape(e.motd) + "'");
			v.push_back(std::to_string(e.tribute));
			v.push_back("'" + Strings::Escape(e.motd_setter) + "'");
			v.push_back("'" + Strings::Escape(e.channel) + "'");
			v.push_back("'" + Strings::Escape(e.url) + "'");
			v.push_back(std::to_string(e.favor));

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

	static std::vector<Guilds> All(Database& db)
	{
		std::vector<Guilds> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Guilds e{};

			e.id          = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.name        = row[1] ? row[1] : "";
			e.leader      = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.minstatus   = row[3] ? static_cast<int16_t>(atoi(row[3])) : 0;
			e.motd        = row[4] ? row[4] : "";
			e.tribute     = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.motd_setter = row[6] ? row[6] : "";
			e.channel     = row[7] ? row[7] : "";
			e.url         = row[8] ? row[8] : "";
			e.favor       = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Guilds> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Guilds> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Guilds e{};

			e.id          = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.name        = row[1] ? row[1] : "";
			e.leader      = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.minstatus   = row[3] ? static_cast<int16_t>(atoi(row[3])) : 0;
			e.motd        = row[4] ? row[4] : "";
			e.tribute     = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.motd_setter = row[6] ? row[6] : "";
			e.channel     = row[7] ? row[7] : "";
			e.url         = row[8] ? row[8] : "";
			e.favor       = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;

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
		const Guilds &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.leader));
		v.push_back(std::to_string(e.minstatus));
		v.push_back("'" + Strings::Escape(e.motd) + "'");
		v.push_back(std::to_string(e.tribute));
		v.push_back("'" + Strings::Escape(e.motd_setter) + "'");
		v.push_back("'" + Strings::Escape(e.channel) + "'");
		v.push_back("'" + Strings::Escape(e.url) + "'");
		v.push_back(std::to_string(e.favor));

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
		const std::vector<Guilds> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.leader));
			v.push_back(std::to_string(e.minstatus));
			v.push_back("'" + Strings::Escape(e.motd) + "'");
			v.push_back(std::to_string(e.tribute));
			v.push_back("'" + Strings::Escape(e.motd_setter) + "'");
			v.push_back("'" + Strings::Escape(e.channel) + "'");
			v.push_back("'" + Strings::Escape(e.url) + "'");
			v.push_back(std::to_string(e.favor));

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
