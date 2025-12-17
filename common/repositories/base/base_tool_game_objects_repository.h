/**
 * DO NOT MODIFY THIS FILE
 *
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to the repository extending the base.
 * Any modifications to base repositories are to be made by the generator only
 *
 * @generator ./utils/scripts/generators/repository-generator.pl
 * @docs https://eqemu.gitbook.io/server/in-development/developer-area/repositories
 */

#pragma once

#include "common/database.h"
#include "common/strings.h"

#include <ctime>

class BaseToolGameObjectsRepository {
public:
	struct ToolGameObjects {
		int         id;
		int         zoneid;
		std::string zonesn;
		std::string object_name;
		std::string file_from;
		int         is_global;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"zoneid",
			"zonesn",
			"object_name",
			"file_from",
			"is_global",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"zoneid",
			"zonesn",
			"object_name",
			"file_from",
			"is_global",
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
		return std::string("tool_game_objects");
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

	static ToolGameObjects NewEntity()
	{
		ToolGameObjects entry{};

		entry.id          = 0;
		entry.zoneid      = 0;
		entry.zonesn      = "";
		entry.object_name = "";
		entry.file_from   = "";
		entry.is_global   = 0;

		return entry;
	}

	static ToolGameObjects GetToolGameObjectsEntry(
		const std::vector<ToolGameObjects> &tool_game_objectss,
		int tool_game_objects_id
	)
	{
		for (auto &tool_game_objects : tool_game_objectss) {
			if (tool_game_objects.id == tool_game_objects_id) {
				return tool_game_objects;
			}
		}

		return NewEntity();
	}

	static ToolGameObjects FindOne(
		Database& db,
		int tool_game_objects_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				tool_game_objects_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			ToolGameObjects entry{};

			entry.id          = atoi(row[0]);
			entry.zoneid      = atoi(row[1]);
			entry.zonesn      = row[2] ? row[2] : "";
			entry.object_name = row[3] ? row[3] : "";
			entry.file_from   = row[4] ? row[4] : "";
			entry.is_global   = atoi(row[5]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int tool_game_objects_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				tool_game_objects_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		ToolGameObjects tool_game_objects_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(tool_game_objects_entry.zoneid));
		update_values.push_back(columns[2] + " = '" + Strings::Escape(tool_game_objects_entry.zonesn) + "'");
		update_values.push_back(columns[3] + " = '" + Strings::Escape(tool_game_objects_entry.object_name) + "'");
		update_values.push_back(columns[4] + " = '" + Strings::Escape(tool_game_objects_entry.file_from) + "'");
		update_values.push_back(columns[5] + " = " + std::to_string(tool_game_objects_entry.is_global));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				tool_game_objects_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static ToolGameObjects InsertOne(
		Database& db,
		ToolGameObjects tool_game_objects_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(tool_game_objects_entry.id));
		insert_values.push_back(std::to_string(tool_game_objects_entry.zoneid));
		insert_values.push_back("'" + Strings::Escape(tool_game_objects_entry.zonesn) + "'");
		insert_values.push_back("'" + Strings::Escape(tool_game_objects_entry.object_name) + "'");
		insert_values.push_back("'" + Strings::Escape(tool_game_objects_entry.file_from) + "'");
		insert_values.push_back(std::to_string(tool_game_objects_entry.is_global));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			tool_game_objects_entry.id = results.LastInsertedID();
			return tool_game_objects_entry;
		}

		tool_game_objects_entry = NewEntity();

		return tool_game_objects_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<ToolGameObjects> tool_game_objects_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &tool_game_objects_entry: tool_game_objects_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(tool_game_objects_entry.id));
			insert_values.push_back(std::to_string(tool_game_objects_entry.zoneid));
			insert_values.push_back("'" + Strings::Escape(tool_game_objects_entry.zonesn) + "'");
			insert_values.push_back("'" + Strings::Escape(tool_game_objects_entry.object_name) + "'");
			insert_values.push_back("'" + Strings::Escape(tool_game_objects_entry.file_from) + "'");
			insert_values.push_back(std::to_string(tool_game_objects_entry.is_global));

			insert_chunks.push_back("(" + Strings::Implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<ToolGameObjects> All(Database& db)
	{
		std::vector<ToolGameObjects> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ToolGameObjects entry{};

			entry.id          = atoi(row[0]);
			entry.zoneid      = atoi(row[1]);
			entry.zonesn      = row[2] ? row[2] : "";
			entry.object_name = row[3] ? row[3] : "";
			entry.file_from   = row[4] ? row[4] : "";
			entry.is_global   = atoi(row[5]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<ToolGameObjects> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<ToolGameObjects> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ToolGameObjects entry{};

			entry.id          = atoi(row[0]);
			entry.zoneid      = atoi(row[1]);
			entry.zonesn      = row[2] ? row[2] : "";
			entry.object_name = row[3] ? row[3] : "";
			entry.file_from   = row[4] ? row[4] : "";
			entry.is_global   = atoi(row[5]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static int DeleteWhere(Database& db, std::string where_filter)
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

};
