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

#ifndef EQEMU_BASE_CHARACTER_ACHIEVEMENT_PENDING_UPDATES_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_ACHIEVEMENT_PENDING_UPDATES_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseCharacterAchievementPendingUpdatesRepository {
public:
	struct CharacterAchievementPendingUpdates {
		uint64_t    update_id;
		uint32_t    character_id;
		uint8_t     source_target_type;
		uint64_t    source_target_id;
		uint8_t     operation;
		uint32_t    achievement_id;
		uint8_t     component_type;
		uint32_t    component_id;
		uint32_t    requested_value;
		uint32_t    version;
		uint8_t     status;
		uint32_t    attempt_count;
		uint32_t    created_at;
		uint32_t    last_attempt_at;
		std::string last_error;
	};

	static std::string PrimaryKey()
	{
		return std::string("update_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"update_id",
			"character_id",
			"source_target_type",
			"source_target_id",
			"operation",
			"achievement_id",
			"component_type",
			"component_id",
			"requested_value",
			"version",
			"status",
			"attempt_count",
			"created_at",
			"last_attempt_at",
			"last_error",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"update_id",
			"character_id",
			"source_target_type",
			"source_target_id",
			"operation",
			"achievement_id",
			"component_type",
			"component_id",
			"requested_value",
			"version",
			"status",
			"attempt_count",
			"created_at",
			"last_attempt_at",
			"last_error",
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
		return std::string("character_achievement_pending_updates");
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

	static CharacterAchievementPendingUpdates NewEntity()
	{
		CharacterAchievementPendingUpdates e{};

		e.update_id          = 0;
		e.character_id       = 0;
		e.source_target_type = 0;
		e.source_target_id   = 0;
		e.operation          = 0;
		e.achievement_id     = 0;
		e.component_type     = 0;
		e.component_id       = 0;
		e.requested_value    = 0;
		e.version            = 0;
		e.status             = 0;
		e.attempt_count      = 0;
		e.created_at         = 0;
		e.last_attempt_at    = 0;
		e.last_error         = "";

		return e;
	}

	static CharacterAchievementPendingUpdates GetCharacterAchievementPendingUpdates(
		const std::vector<CharacterAchievementPendingUpdates> &character_achievement_pending_updatess,
		int character_achievement_pending_updates_id
	)
	{
		for (auto &character_achievement_pending_updates : character_achievement_pending_updatess) {
			if (character_achievement_pending_updates.update_id == character_achievement_pending_updates_id) {
				return character_achievement_pending_updates;
			}
		}

		return NewEntity();
	}

	static CharacterAchievementPendingUpdates FindOne(
		Database& db,
		int character_achievement_pending_updates_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				character_achievement_pending_updates_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterAchievementPendingUpdates e{};

			e.update_id          = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.character_id       = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.source_target_type = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.source_target_id   = row[3] ? strtoull(row[3], nullptr, 10) : 0;
			e.operation          = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.achievement_id     = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.component_type     = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.component_id       = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.requested_value    = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.version            = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.status             = row[10] ? static_cast<uint8_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.attempt_count      = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.created_at         = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.last_attempt_at    = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.last_error         = row[14] ? row[14] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_achievement_pending_updates_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_achievement_pending_updates_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CharacterAchievementPendingUpdates &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.character_id));
		v.push_back(columns[2] + " = " + std::to_string(e.source_target_type));
		v.push_back(columns[3] + " = " + std::to_string(e.source_target_id));
		v.push_back(columns[4] + " = " + std::to_string(e.operation));
		v.push_back(columns[5] + " = " + std::to_string(e.achievement_id));
		v.push_back(columns[6] + " = " + std::to_string(e.component_type));
		v.push_back(columns[7] + " = " + std::to_string(e.component_id));
		v.push_back(columns[8] + " = " + std::to_string(e.requested_value));
		v.push_back(columns[9] + " = " + std::to_string(e.version));
		v.push_back(columns[10] + " = " + std::to_string(e.status));
		v.push_back(columns[11] + " = " + std::to_string(e.attempt_count));
		v.push_back(columns[12] + " = " + std::to_string(e.created_at));
		v.push_back(columns[13] + " = " + std::to_string(e.last_attempt_at));
		v.push_back(columns[14] + " = '" + Strings::Escape(e.last_error) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.update_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterAchievementPendingUpdates InsertOne(
		Database& db,
		CharacterAchievementPendingUpdates e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.update_id));
		v.push_back(std::to_string(e.character_id));
		v.push_back(std::to_string(e.source_target_type));
		v.push_back(std::to_string(e.source_target_id));
		v.push_back(std::to_string(e.operation));
		v.push_back(std::to_string(e.achievement_id));
		v.push_back(std::to_string(e.component_type));
		v.push_back(std::to_string(e.component_id));
		v.push_back(std::to_string(e.requested_value));
		v.push_back(std::to_string(e.version));
		v.push_back(std::to_string(e.status));
		v.push_back(std::to_string(e.attempt_count));
		v.push_back(std::to_string(e.created_at));
		v.push_back(std::to_string(e.last_attempt_at));
		v.push_back("'" + Strings::Escape(e.last_error) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.update_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<CharacterAchievementPendingUpdates> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.update_id));
			v.push_back(std::to_string(e.character_id));
			v.push_back(std::to_string(e.source_target_type));
			v.push_back(std::to_string(e.source_target_id));
			v.push_back(std::to_string(e.operation));
			v.push_back(std::to_string(e.achievement_id));
			v.push_back(std::to_string(e.component_type));
			v.push_back(std::to_string(e.component_id));
			v.push_back(std::to_string(e.requested_value));
			v.push_back(std::to_string(e.version));
			v.push_back(std::to_string(e.status));
			v.push_back(std::to_string(e.attempt_count));
			v.push_back(std::to_string(e.created_at));
			v.push_back(std::to_string(e.last_attempt_at));
			v.push_back("'" + Strings::Escape(e.last_error) + "'");

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

	static std::vector<CharacterAchievementPendingUpdates> All(Database& db)
	{
		std::vector<CharacterAchievementPendingUpdates> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterAchievementPendingUpdates e{};

			e.update_id          = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.character_id       = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.source_target_type = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.source_target_id   = row[3] ? strtoull(row[3], nullptr, 10) : 0;
			e.operation          = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.achievement_id     = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.component_type     = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.component_id       = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.requested_value    = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.version            = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.status             = row[10] ? static_cast<uint8_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.attempt_count      = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.created_at         = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.last_attempt_at    = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.last_error         = row[14] ? row[14] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CharacterAchievementPendingUpdates> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CharacterAchievementPendingUpdates> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterAchievementPendingUpdates e{};

			e.update_id          = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.character_id       = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.source_target_type = row[2] ? static_cast<uint8_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.source_target_id   = row[3] ? strtoull(row[3], nullptr, 10) : 0;
			e.operation          = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.achievement_id     = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.component_type     = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.component_id       = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.requested_value    = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.version            = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.status             = row[10] ? static_cast<uint8_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.attempt_count      = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.created_at         = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.last_attempt_at    = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.last_error         = row[14] ? row[14] : "";

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
		const CharacterAchievementPendingUpdates &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.update_id));
		v.push_back(std::to_string(e.character_id));
		v.push_back(std::to_string(e.source_target_type));
		v.push_back(std::to_string(e.source_target_id));
		v.push_back(std::to_string(e.operation));
		v.push_back(std::to_string(e.achievement_id));
		v.push_back(std::to_string(e.component_type));
		v.push_back(std::to_string(e.component_id));
		v.push_back(std::to_string(e.requested_value));
		v.push_back(std::to_string(e.version));
		v.push_back(std::to_string(e.status));
		v.push_back(std::to_string(e.attempt_count));
		v.push_back(std::to_string(e.created_at));
		v.push_back(std::to_string(e.last_attempt_at));
		v.push_back("'" + Strings::Escape(e.last_error) + "'");

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
		const std::vector<CharacterAchievementPendingUpdates> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.update_id));
			v.push_back(std::to_string(e.character_id));
			v.push_back(std::to_string(e.source_target_type));
			v.push_back(std::to_string(e.source_target_id));
			v.push_back(std::to_string(e.operation));
			v.push_back(std::to_string(e.achievement_id));
			v.push_back(std::to_string(e.component_type));
			v.push_back(std::to_string(e.component_id));
			v.push_back(std::to_string(e.requested_value));
			v.push_back(std::to_string(e.version));
			v.push_back(std::to_string(e.status));
			v.push_back(std::to_string(e.attempt_count));
			v.push_back(std::to_string(e.created_at));
			v.push_back(std::to_string(e.last_attempt_at));
			v.push_back("'" + Strings::Escape(e.last_error) + "'");

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

#endif //EQEMU_BASE_CHARACTER_ACHIEVEMENT_PENDING_UPDATES_REPOSITORY_H
