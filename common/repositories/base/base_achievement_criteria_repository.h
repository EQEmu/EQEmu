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

#ifndef EQEMU_BASE_ACHIEVEMENT_CRITERIA_REPOSITORY_H
#define EQEMU_BASE_ACHIEVEMENT_CRITERIA_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseAchievementCriteriaRepository {
public:
	struct AchievementCriteria {
		uint64_t id;
		uint32_t achievement_id;
		uint8_t  component_type;
		uint32_t component_sequence;
		uint32_t component_id;
		uint8_t  event_type;
		uint8_t  progress_mode;
		uint8_t  behavior;
		uint32_t target_id;
		uint32_t target_id2;
		int64_t  target_value;
		uint32_t required_count;
		uint8_t  enabled;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"achievement_id",
			"component_type",
			"component_sequence",
			"component_id",
			"event_type",
			"progress_mode",
			"behavior",
			"target_id",
			"target_id2",
			"target_value",
			"required_count",
			"enabled",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"achievement_id",
			"component_type",
			"component_sequence",
			"component_id",
			"event_type",
			"progress_mode",
			"behavior",
			"target_id",
			"target_id2",
			"target_value",
			"required_count",
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
		return std::string("achievement_criteria");
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

	static AchievementCriteria NewEntity()
	{
		AchievementCriteria e{};

		e.id                 = 0;
		e.achievement_id     = 0;
		e.component_type     = 0;
		e.component_sequence = 0;
		e.component_id       = 0;
		e.event_type         = 0;
		e.progress_mode      = 0;
		e.behavior           = 0;
		e.target_id          = 0;
		e.target_id2         = 0;
		e.target_value       = 0;
		e.required_count     = 1;
		e.enabled            = 1;

		return e;
	}

	static AchievementCriteria GetAchievementCriteria(
		const std::vector<AchievementCriteria> &achievement_criterias,
		int achievement_criteria_id
	)
	{
		for (auto &achievement_criteria : achievement_criterias) {
			if (achievement_criteria.id == achievement_criteria_id) {
				return achievement_criteria;
			}
		}

		return NewEntity();
	}

	static AchievementCriteria FindOne(
		Database& db,
		int achievement_criteria_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				achievement_criteria_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			AchievementCriteria e{};

			e.id                 = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.achievement_id     = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.component_type     = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.component_sequence = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.component_id       = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.event_type         = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.progress_mode      = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.behavior           = row[7] ? static_cast<uint8_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.target_id          = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.target_id2         = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.target_value       = row[10] ? strtoll(row[10], nullptr, 10) : 0;
			e.required_count     = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 1;
			e.enabled            = row[12] ? static_cast<uint8_t>(strtoul(row[12], nullptr, 10)) : 1;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int achievement_criteria_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				achievement_criteria_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const AchievementCriteria &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.achievement_id));
		v.push_back(columns[2] + " = " + std::to_string(e.component_type));
		v.push_back(columns[3] + " = " + std::to_string(e.component_sequence));
		v.push_back(columns[4] + " = " + std::to_string(e.component_id));
		v.push_back(columns[5] + " = " + std::to_string(e.event_type));
		v.push_back(columns[6] + " = " + std::to_string(e.progress_mode));
		v.push_back(columns[7] + " = " + std::to_string(e.behavior));
		v.push_back(columns[8] + " = " + std::to_string(e.target_id));
		v.push_back(columns[9] + " = " + std::to_string(e.target_id2));
		v.push_back(columns[10] + " = " + std::to_string(e.target_value));
		v.push_back(columns[11] + " = " + std::to_string(e.required_count));
		v.push_back(columns[12] + " = " + std::to_string(e.enabled));

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

	static AchievementCriteria InsertOne(
		Database& db,
		AchievementCriteria e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.achievement_id));
		v.push_back(std::to_string(e.component_type));
		v.push_back(std::to_string(e.component_sequence));
		v.push_back(std::to_string(e.component_id));
		v.push_back(std::to_string(e.event_type));
		v.push_back(std::to_string(e.progress_mode));
		v.push_back(std::to_string(e.behavior));
		v.push_back(std::to_string(e.target_id));
		v.push_back(std::to_string(e.target_id2));
		v.push_back(std::to_string(e.target_value));
		v.push_back(std::to_string(e.required_count));
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
		const std::vector<AchievementCriteria> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.achievement_id));
			v.push_back(std::to_string(e.component_type));
			v.push_back(std::to_string(e.component_sequence));
			v.push_back(std::to_string(e.component_id));
			v.push_back(std::to_string(e.event_type));
			v.push_back(std::to_string(e.progress_mode));
			v.push_back(std::to_string(e.behavior));
			v.push_back(std::to_string(e.target_id));
			v.push_back(std::to_string(e.target_id2));
			v.push_back(std::to_string(e.target_value));
			v.push_back(std::to_string(e.required_count));
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

	static std::vector<AchievementCriteria> All(Database& db)
	{
		std::vector<AchievementCriteria> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AchievementCriteria e{};

			e.id                 = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.achievement_id     = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.component_type     = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.component_sequence = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.component_id       = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.event_type         = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.progress_mode      = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.behavior           = row[7] ? static_cast<uint8_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.target_id          = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.target_id2         = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.target_value       = row[10] ? strtoll(row[10], nullptr, 10) : 0;
			e.required_count     = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 1;
			e.enabled            = row[12] ? static_cast<uint8_t>(strtoul(row[12], nullptr, 10)) : 1;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<AchievementCriteria> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<AchievementCriteria> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AchievementCriteria e{};

			e.id                 = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.achievement_id     = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.component_type     = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.component_sequence = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.component_id       = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.event_type         = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.progress_mode      = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.behavior           = row[7] ? static_cast<uint8_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.target_id          = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.target_id2         = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.target_value       = row[10] ? strtoll(row[10], nullptr, 10) : 0;
			e.required_count     = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 1;
			e.enabled            = row[12] ? static_cast<uint8_t>(strtoul(row[12], nullptr, 10)) : 1;

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
		const AchievementCriteria &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.achievement_id));
		v.push_back(std::to_string(e.component_type));
		v.push_back(std::to_string(e.component_sequence));
		v.push_back(std::to_string(e.component_id));
		v.push_back(std::to_string(e.event_type));
		v.push_back(std::to_string(e.progress_mode));
		v.push_back(std::to_string(e.behavior));
		v.push_back(std::to_string(e.target_id));
		v.push_back(std::to_string(e.target_id2));
		v.push_back(std::to_string(e.target_value));
		v.push_back(std::to_string(e.required_count));
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
		const std::vector<AchievementCriteria> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.achievement_id));
			v.push_back(std::to_string(e.component_type));
			v.push_back(std::to_string(e.component_sequence));
			v.push_back(std::to_string(e.component_id));
			v.push_back(std::to_string(e.event_type));
			v.push_back(std::to_string(e.progress_mode));
			v.push_back(std::to_string(e.behavior));
			v.push_back(std::to_string(e.target_id));
			v.push_back(std::to_string(e.target_id2));
			v.push_back(std::to_string(e.target_value));
			v.push_back(std::to_string(e.required_count));
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

#endif //EQEMU_BASE_ACHIEVEMENT_CRITERIA_REPOSITORY_H
