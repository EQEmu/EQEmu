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


class BaseGuildPermissionsRepository {
public:
	struct GuildPermissions {
		int32_t id;
		int32_t perm_id;
		int32_t guild_id;
		int32_t permission;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"perm_id",
			"guild_id",
			"permission",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"perm_id",
			"guild_id",
			"permission",
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
		return std::string("guild_permissions");
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

	static GuildPermissions NewEntity()
	{
		GuildPermissions e{};

		e.id         = 0;
		e.perm_id    = 0;
		e.guild_id   = 0;
		e.permission = 0;

		return e;
	}

	static GuildPermissions GetGuildPermissions(
		const std::vector<GuildPermissions> &guild_permissionss,
		int guild_permissions_id
	)
	{
		for (auto &guild_permissions : guild_permissionss) {
			if (guild_permissions.id == guild_permissions_id) {
				return guild_permissions;
			}
		}

		return NewEntity();
	}

	static GuildPermissions FindOne(
		Database& db,
		int guild_permissions_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				guild_permissions_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			GuildPermissions e{};

			e.id         = static_cast<int32_t>(atoi(row[0]));
			e.perm_id    = static_cast<int32_t>(atoi(row[1]));
			e.guild_id   = static_cast<int32_t>(atoi(row[2]));
			e.permission = static_cast<int32_t>(atoi(row[3]));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int guild_permissions_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				guild_permissions_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const GuildPermissions &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.perm_id));
		v.push_back(columns[2] + " = " + std::to_string(e.guild_id));
		v.push_back(columns[3] + " = " + std::to_string(e.permission));

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

	static GuildPermissions InsertOne(
		Database& db,
		GuildPermissions e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.perm_id));
		v.push_back(std::to_string(e.guild_id));
		v.push_back(std::to_string(e.permission));

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
		const std::vector<GuildPermissions> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.perm_id));
			v.push_back(std::to_string(e.guild_id));
			v.push_back(std::to_string(e.permission));

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

	static std::vector<GuildPermissions> All(Database& db)
	{
		std::vector<GuildPermissions> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GuildPermissions e{};

			e.id         = static_cast<int32_t>(atoi(row[0]));
			e.perm_id    = static_cast<int32_t>(atoi(row[1]));
			e.guild_id   = static_cast<int32_t>(atoi(row[2]));
			e.permission = static_cast<int32_t>(atoi(row[3]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<GuildPermissions> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<GuildPermissions> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GuildPermissions e{};

			e.id         = static_cast<int32_t>(atoi(row[0]));
			e.perm_id    = static_cast<int32_t>(atoi(row[1]));
			e.guild_id   = static_cast<int32_t>(atoi(row[2]));
			e.permission = static_cast<int32_t>(atoi(row[3]));

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
		const GuildPermissions &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.perm_id));
		v.push_back(std::to_string(e.guild_id));
		v.push_back(std::to_string(e.permission));

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
		const std::vector<GuildPermissions> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.perm_id));
			v.push_back(std::to_string(e.guild_id));
			v.push_back(std::to_string(e.permission));

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
