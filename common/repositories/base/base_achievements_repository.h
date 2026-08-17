/**
 * DO NOT MODIFY THIS FILE
 *
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to the repository extending the base.
 * Any modifications to base repositories are to be made by the generator only
 *
 * @generator ./utils/scripts/generators/repository-generator.pl
 * @docs https://docs.eqemu.io/developer/repositories
 */

#ifndef EQEMU_BASE_ACHIEVEMENTS_REPOSITORY_H
#define EQEMU_BASE_ACHIEVEMENTS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseAchievementsRepository {
public:
	struct Achievements {
		uint32_t    id;
		std::string name;
		std::string description;
		uint32_t    icon_id;
		uint32_t    points;
		uint8_t     has_reward;
		uint8_t     client_flag;
		uint32_t    version;
		uint8_t     reset_on_version_change;
		uint8_t     enabled;
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
			"description",
			"icon_id",
			"points",
			"has_reward",
			"client_flag",
			"version",
			"reset_on_version_change",
			"enabled",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"name",
			"description",
			"icon_id",
			"points",
			"has_reward",
			"client_flag",
			"version",
			"reset_on_version_change",
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
		return std::string("achievements");
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

	static Achievements NewEntity()
	{
		Achievements e{};

		e.id                      = 0;
		e.name                    = "";
		e.description             = "";
		e.icon_id                 = 0;
		e.points                  = 0;
		e.has_reward              = 0;
		e.client_flag             = 0;
		e.version                 = 0;
		e.reset_on_version_change = 1;
		e.enabled                 = 1;

		return e;
	}

	static Achievements GetAchievements(
		const std::vector<Achievements> &achievementss,
		int achievements_id
	)
	{
		for (auto &achievements : achievementss) {
			if (achievements.id == achievements_id) {
				return achievements;
			}
		}

		return NewEntity();
	}

	static Achievements FindOne(
		Database& db,
		int achievements_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				achievements_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Achievements e{};

			e.id                      = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.name                    = row[1] ? row[1] : "";
			e.description             = row[2] ? row[2] : "";
			e.icon_id                 = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.points                  = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.has_reward              = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.client_flag             = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.version                 = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.reset_on_version_change = row[8] ? static_cast<uint8_t>(strtoul(row[8], nullptr, 10)) : 1;
			e.enabled                 = row[9] ? static_cast<uint8_t>(strtoul(row[9], nullptr, 10)) : 1;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int achievements_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				achievements_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Achievements &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.id));
		v.push_back(columns[1] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[2] + " = '" + Strings::Escape(e.description) + "'");
		v.push_back(columns[3] + " = " + std::to_string(e.icon_id));
		v.push_back(columns[4] + " = " + std::to_string(e.points));
		v.push_back(columns[5] + " = " + std::to_string(e.has_reward));
		v.push_back(columns[6] + " = " + std::to_string(e.client_flag));
		v.push_back(columns[7] + " = " + std::to_string(e.version));
		v.push_back(columns[8] + " = " + std::to_string(e.reset_on_version_change));
		v.push_back(columns[9] + " = " + std::to_string(e.enabled));

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

	static Achievements InsertOne(
		Database& db,
		Achievements e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back("'" + Strings::Escape(e.description) + "'");
		v.push_back(std::to_string(e.icon_id));
		v.push_back(std::to_string(e.points));
		v.push_back(std::to_string(e.has_reward));
		v.push_back(std::to_string(e.client_flag));
		v.push_back(std::to_string(e.version));
		v.push_back(std::to_string(e.reset_on_version_change));
		v.push_back(std::to_string(e.enabled));

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
		const std::vector<Achievements> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back("'" + Strings::Escape(e.description) + "'");
			v.push_back(std::to_string(e.icon_id));
			v.push_back(std::to_string(e.points));
			v.push_back(std::to_string(e.has_reward));
			v.push_back(std::to_string(e.client_flag));
			v.push_back(std::to_string(e.version));
			v.push_back(std::to_string(e.reset_on_version_change));
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

	static std::vector<Achievements> All(Database& db)
	{
		std::vector<Achievements> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Achievements e{};

			e.id                      = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.name                    = row[1] ? row[1] : "";
			e.description             = row[2] ? row[2] : "";
			e.icon_id                 = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.points                  = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.has_reward              = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.client_flag             = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.version                 = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.reset_on_version_change = row[8] ? static_cast<uint8_t>(strtoul(row[8], nullptr, 10)) : 1;
			e.enabled                 = row[9] ? static_cast<uint8_t>(strtoul(row[9], nullptr, 10)) : 1;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Achievements> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Achievements> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Achievements e{};

			e.id                      = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.name                    = row[1] ? row[1] : "";
			e.description             = row[2] ? row[2] : "";
			e.icon_id                 = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.points                  = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.has_reward              = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.client_flag             = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.version                 = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.reset_on_version_change = row[8] ? static_cast<uint8_t>(strtoul(row[8], nullptr, 10)) : 1;
			e.enabled                 = row[9] ? static_cast<uint8_t>(strtoul(row[9], nullptr, 10)) : 1;

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
		const Achievements &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back("'" + Strings::Escape(e.description) + "'");
		v.push_back(std::to_string(e.icon_id));
		v.push_back(std::to_string(e.points));
		v.push_back(std::to_string(e.has_reward));
		v.push_back(std::to_string(e.client_flag));
		v.push_back(std::to_string(e.version));
		v.push_back(std::to_string(e.reset_on_version_change));
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
		const std::vector<Achievements> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back("'" + Strings::Escape(e.description) + "'");
			v.push_back(std::to_string(e.icon_id));
			v.push_back(std::to_string(e.points));
			v.push_back(std::to_string(e.has_reward));
			v.push_back(std::to_string(e.client_flag));
			v.push_back(std::to_string(e.version));
			v.push_back(std::to_string(e.reset_on_version_change));
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

#endif //EQEMU_BASE_ACHIEVEMENTS_REPOSITORY_H
