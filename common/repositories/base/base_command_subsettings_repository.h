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

class BaseCommandSubsettingsRepository {
public:
	struct CommandSubsettings {
		uint32_t    id;
		std::string parent_command;
		std::string sub_command;
		uint32_t    access_level;
		std::string top_level_aliases;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"parent_command",
			"sub_command",
			"access_level",
			"top_level_aliases",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"parent_command",
			"sub_command",
			"access_level",
			"top_level_aliases",
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
		return std::string("command_subsettings");
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

	static CommandSubsettings NewEntity()
	{
		CommandSubsettings e{};

		e.id                = 0;
		e.parent_command    = "";
		e.sub_command       = "";
		e.access_level      = 0;
		e.top_level_aliases = "";

		return e;
	}

	static CommandSubsettings GetCommandSubsettings(
		const std::vector<CommandSubsettings> &command_subsettingss,
		int command_subsettings_id
	)
	{
		for (auto &command_subsettings : command_subsettingss) {
			if (command_subsettings.id == command_subsettings_id) {
				return command_subsettings;
			}
		}

		return NewEntity();
	}

	static CommandSubsettings FindOne(
		Database& db,
		int command_subsettings_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				command_subsettings_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CommandSubsettings e{};

			e.id                = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.parent_command    = row[1] ? row[1] : "";
			e.sub_command       = row[2] ? row[2] : "";
			e.access_level      = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.top_level_aliases = row[4] ? row[4] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int command_subsettings_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				command_subsettings_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CommandSubsettings &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.parent_command) + "'");
		v.push_back(columns[2] + " = '" + Strings::Escape(e.sub_command) + "'");
		v.push_back(columns[3] + " = " + std::to_string(e.access_level));
		v.push_back(columns[4] + " = '" + Strings::Escape(e.top_level_aliases) + "'");

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

	static CommandSubsettings InsertOne(
		Database& db,
		CommandSubsettings e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.parent_command) + "'");
		v.push_back("'" + Strings::Escape(e.sub_command) + "'");
		v.push_back(std::to_string(e.access_level));
		v.push_back("'" + Strings::Escape(e.top_level_aliases) + "'");

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
		const std::vector<CommandSubsettings> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.parent_command) + "'");
			v.push_back("'" + Strings::Escape(e.sub_command) + "'");
			v.push_back(std::to_string(e.access_level));
			v.push_back("'" + Strings::Escape(e.top_level_aliases) + "'");

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

	static std::vector<CommandSubsettings> All(Database& db)
	{
		std::vector<CommandSubsettings> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CommandSubsettings e{};

			e.id                = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.parent_command    = row[1] ? row[1] : "";
			e.sub_command       = row[2] ? row[2] : "";
			e.access_level      = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.top_level_aliases = row[4] ? row[4] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CommandSubsettings> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CommandSubsettings> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CommandSubsettings e{};

			e.id                = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.parent_command    = row[1] ? row[1] : "";
			e.sub_command       = row[2] ? row[2] : "";
			e.access_level      = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.top_level_aliases = row[4] ? row[4] : "";

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
		const CommandSubsettings &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.parent_command) + "'");
		v.push_back("'" + Strings::Escape(e.sub_command) + "'");
		v.push_back(std::to_string(e.access_level));
		v.push_back("'" + Strings::Escape(e.top_level_aliases) + "'");

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
		const std::vector<CommandSubsettings> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.parent_command) + "'");
			v.push_back("'" + Strings::Escape(e.sub_command) + "'");
			v.push_back(std::to_string(e.access_level));
			v.push_back("'" + Strings::Escape(e.top_level_aliases) + "'");

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
