/**
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to
 * the repository extending the base. Any modifications to base repositories are to
 * be made by the generator only
 */

#pragma once

#include "common/database.h"
#include "common/strings.h"

class BaseLauncherRepository {
public:
	struct Launcher {
		std::string name;
		int8        dynamics;
	};

	static std::string PrimaryKey()
	{
		return std::string("name");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"name",
			"dynamics",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(Strings::Implode(", ", Columns()));
	}

	static std::string InsertColumnsRaw()
	{
		std::vector<std::string> insert_columns;

		for (auto &column : Columns()) {
			if (column == PrimaryKey()) {
				continue;
			}

			insert_columns.push_back(column);
		}

		return std::string(Strings::Implode(", ", insert_columns));
	}

	static std::string TableName()
	{
		return std::string("launcher");
	}

	static std::string BaseSelect()
	{
		return fmt::format(
			"SELECT {} FROM {}",
			ColumnsRaw(),
			TableName()
		);
	}

	static std::string BaseInsert()
	{
		return fmt::format(
			"INSERT INTO {} ({}) ",
			TableName(),
			InsertColumnsRaw()
		);
	}

	static Launcher NewEntity()
	{
		Launcher entry{};

		entry.name     = "";
		entry.dynamics = 0;

		return entry;
	}

	static Launcher GetLauncherEntry(
		const std::vector<Launcher> &launchers,
		int launcher_id
	)
	{
		for (auto &launcher : launchers) {
			if (launcher.name == launcher_id) {
				return launcher;
			}
		}

		return NewEntity();
	}

	static Launcher FindOne(
		int launcher_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				launcher_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Launcher entry{};

			entry.name     = row[0] ? row[0] : "";
			entry.dynamics = atoi(row[1]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int launcher_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				launcher_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Launcher launcher_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = '" + Strings::Escape(launcher_entry.name) + "'");
		update_values.push_back(columns[1] + " = " + std::to_string(launcher_entry.dynamics));

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				launcher_entry.name
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Launcher InsertOne(
		Launcher launcher_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back("'" + Strings::Escape(launcher_entry.name) + "'");
		insert_values.push_back(std::to_string(launcher_entry.dynamics));

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			launcher_entry.name = results.LastInsertedID();
			return launcher_entry;
		}

		launcher_entry = NewEntity();

		return launcher_entry;
	}

	static int InsertMany(
		std::vector<Launcher> launcher_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &launcher_entry: launcher_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back("'" + Strings::Escape(launcher_entry.name) + "'");
			insert_values.push_back(std::to_string(launcher_entry.dynamics));

			insert_chunks.push_back("(" + Strings::Implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<Launcher> All()
	{
		std::vector<Launcher> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Launcher entry{};

			entry.name     = row[0] ? row[0] : "";
			entry.dynamics = atoi(row[1]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Launcher> GetWhere(std::string where_filter)
	{
		std::vector<Launcher> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Launcher entry{};

			entry.name     = row[0] ? row[0] : "";
			entry.dynamics = atoi(row[1]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static int DeleteWhere(std::string where_filter)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {}",
				TableName(),
				PrimaryKey(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};
