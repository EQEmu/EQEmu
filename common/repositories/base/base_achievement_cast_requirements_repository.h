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

#ifndef EQEMU_BASE_ACHIEVEMENT_CAST_REQUIREMENTS_REPOSITORY_H
#define EQEMU_BASE_ACHIEVEMENT_CAST_REQUIREMENTS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseAchievementCastRequirementsRepository {
public:
	struct AchievementCastRequirements {
		uint32_t restriction_id;
		uint32_t achievement_id;
		uint8_t  requires_completed;
	};

	static std::string PrimaryKey()
	{
		return std::string("restriction_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"restriction_id",
			"achievement_id",
			"requires_completed",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"restriction_id",
			"achievement_id",
			"requires_completed",
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
		return std::string("achievement_cast_requirements");
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

	static AchievementCastRequirements NewEntity()
	{
		AchievementCastRequirements e{};

		e.restriction_id     = 0;
		e.achievement_id     = 0;
		e.requires_completed = 1;

		return e;
	}

	static AchievementCastRequirements GetAchievementCastRequirements(
		const std::vector<AchievementCastRequirements> &achievement_cast_requirementss,
		int achievement_cast_requirements_id
	)
	{
		for (auto &achievement_cast_requirements : achievement_cast_requirementss) {
			if (achievement_cast_requirements.restriction_id == achievement_cast_requirements_id) {
				return achievement_cast_requirements;
			}
		}

		return NewEntity();
	}

	static AchievementCastRequirements FindOne(
		Database& db,
		int achievement_cast_requirements_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				achievement_cast_requirements_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			AchievementCastRequirements e{};

			e.restriction_id     = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.achievement_id     = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.requires_completed = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 1;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int achievement_cast_requirements_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				achievement_cast_requirements_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const AchievementCastRequirements &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.restriction_id));
		v.push_back(columns[1] + " = " + std::to_string(e.achievement_id));
		v.push_back(columns[2] + " = " + std::to_string(e.requires_completed));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.restriction_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static AchievementCastRequirements InsertOne(
		Database& db,
		AchievementCastRequirements e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.restriction_id));
		v.push_back(std::to_string(e.achievement_id));
		v.push_back(std::to_string(e.requires_completed));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.restriction_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<AchievementCastRequirements> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.restriction_id));
			v.push_back(std::to_string(e.achievement_id));
			v.push_back(std::to_string(e.requires_completed));

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

	static std::vector<AchievementCastRequirements> All(Database& db)
	{
		std::vector<AchievementCastRequirements> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AchievementCastRequirements e{};

			e.restriction_id     = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.achievement_id     = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.requires_completed = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 1;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<AchievementCastRequirements> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<AchievementCastRequirements> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AchievementCastRequirements e{};

			e.restriction_id     = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.achievement_id     = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.requires_completed = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 1;

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
		const AchievementCastRequirements &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.restriction_id));
		v.push_back(std::to_string(e.achievement_id));
		v.push_back(std::to_string(e.requires_completed));

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
		const std::vector<AchievementCastRequirements> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.restriction_id));
			v.push_back(std::to_string(e.achievement_id));
			v.push_back(std::to_string(e.requires_completed));

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

#endif //EQEMU_BASE_ACHIEVEMENT_CAST_REQUIREMENTS_REPOSITORY_H
