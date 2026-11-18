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

#ifndef EQEMU_BASE_CHARACTER_ACHIEVEMENT_PROGRESS_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_ACHIEVEMENT_PROGRESS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseCharacterAchievementProgressRepository {
public:
	struct CharacterAchievementProgress {
		uint32_t character_id;
		uint32_t achievement_id;
		uint8_t  component_type;
		uint32_t component_sequence;
		uint32_t component_id;
		uint64_t current_count;
		uint8_t  completed;
		uint32_t version;
		uint32_t updated_at;
	};

	static std::string PrimaryKey()
	{
		return std::string("character_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"character_id",
			"achievement_id",
			"component_type",
			"component_sequence",
			"component_id",
			"current_count",
			"completed",
			"version",
			"updated_at",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"character_id",
			"achievement_id",
			"component_type",
			"component_sequence",
			"component_id",
			"current_count",
			"completed",
			"version",
			"updated_at",
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
		return std::string("character_achievement_progress");
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

	static CharacterAchievementProgress NewEntity()
	{
		CharacterAchievementProgress e{};

		e.character_id       = 0;
		e.achievement_id     = 0;
		e.component_type     = 0;
		e.component_sequence = 0;
		e.component_id       = 0;
		e.current_count      = 0;
		e.completed          = 0;
		e.version            = 0;
		e.updated_at         = 0;

		return e;
	}

	static CharacterAchievementProgress GetCharacterAchievementProgress(
		const std::vector<CharacterAchievementProgress> &character_achievement_progresss,
		int character_achievement_progress_id
	)
	{
		for (auto &character_achievement_progress : character_achievement_progresss) {
			if (character_achievement_progress.character_id == character_achievement_progress_id) {
				return character_achievement_progress;
			}
		}

		return NewEntity();
	}

	static CharacterAchievementProgress FindOne(
		Database& db,
		int character_achievement_progress_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				character_achievement_progress_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterAchievementProgress e{};

			e.character_id       = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.achievement_id     = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.component_type     = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.component_sequence = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.component_id       = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.current_count      = row[5] ? strtoull(row[5], nullptr, 10) : 0;
			e.completed          = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.version            = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.updated_at         = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_achievement_progress_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_achievement_progress_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CharacterAchievementProgress &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.character_id));
		v.push_back(columns[1] + " = " + std::to_string(e.achievement_id));
		v.push_back(columns[2] + " = " + std::to_string(e.component_type));
		v.push_back(columns[3] + " = " + std::to_string(e.component_sequence));
		v.push_back(columns[4] + " = " + std::to_string(e.component_id));
		v.push_back(columns[5] + " = " + std::to_string(e.current_count));
		v.push_back(columns[6] + " = " + std::to_string(e.completed));
		v.push_back(columns[7] + " = " + std::to_string(e.version));
		v.push_back(columns[8] + " = " + std::to_string(e.updated_at));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.character_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterAchievementProgress InsertOne(
		Database& db,
		CharacterAchievementProgress e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.character_id));
		v.push_back(std::to_string(e.achievement_id));
		v.push_back(std::to_string(e.component_type));
		v.push_back(std::to_string(e.component_sequence));
		v.push_back(std::to_string(e.component_id));
		v.push_back(std::to_string(e.current_count));
		v.push_back(std::to_string(e.completed));
		v.push_back(std::to_string(e.version));
		v.push_back(std::to_string(e.updated_at));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.character_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<CharacterAchievementProgress> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.character_id));
			v.push_back(std::to_string(e.achievement_id));
			v.push_back(std::to_string(e.component_type));
			v.push_back(std::to_string(e.component_sequence));
			v.push_back(std::to_string(e.component_id));
			v.push_back(std::to_string(e.current_count));
			v.push_back(std::to_string(e.completed));
			v.push_back(std::to_string(e.version));
			v.push_back(std::to_string(e.updated_at));

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

	static std::vector<CharacterAchievementProgress> All(Database& db)
	{
		std::vector<CharacterAchievementProgress> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterAchievementProgress e{};

			e.character_id       = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.achievement_id     = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.component_type     = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.component_sequence = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.component_id       = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.current_count      = row[5] ? strtoull(row[5], nullptr, 10) : 0;
			e.completed          = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.version            = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.updated_at         = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CharacterAchievementProgress> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CharacterAchievementProgress> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterAchievementProgress e{};

			e.character_id       = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.achievement_id     = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.component_type     = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.component_sequence = row[3] ? static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.component_id       = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.current_count      = row[5] ? strtoull(row[5], nullptr, 10) : 0;
			e.completed          = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.version            = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.updated_at         = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;

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
		const CharacterAchievementProgress &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.character_id));
		v.push_back(std::to_string(e.achievement_id));
		v.push_back(std::to_string(e.component_type));
		v.push_back(std::to_string(e.component_sequence));
		v.push_back(std::to_string(e.component_id));
		v.push_back(std::to_string(e.current_count));
		v.push_back(std::to_string(e.completed));
		v.push_back(std::to_string(e.version));
		v.push_back(std::to_string(e.updated_at));

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
		const std::vector<CharacterAchievementProgress> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.character_id));
			v.push_back(std::to_string(e.achievement_id));
			v.push_back(std::to_string(e.component_type));
			v.push_back(std::to_string(e.component_sequence));
			v.push_back(std::to_string(e.component_id));
			v.push_back(std::to_string(e.current_count));
			v.push_back(std::to_string(e.completed));
			v.push_back(std::to_string(e.version));
			v.push_back(std::to_string(e.updated_at));

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

#endif //EQEMU_BASE_CHARACTER_ACHIEVEMENT_PROGRESS_REPOSITORY_H
