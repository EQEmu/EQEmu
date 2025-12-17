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

class BaseTasksRepository {
public:
	struct Tasks {
		uint32_t    id;
		int8_t      type;
		uint32_t    duration;
		int8_t      duration_code;
		std::string title;
		std::string description;
		std::string reward_text;
		std::string reward_id_list;
		uint32_t    cash_reward;
		int32_t     exp_reward;
		uint8_t     reward_method;
		int32_t     reward_points;
		int32_t     reward_point_type;
		uint8_t     min_level;
		uint8_t     max_level;
		uint32_t    level_spread;
		uint32_t    min_players;
		uint32_t    max_players;
		uint8_t     repeatable;
		int32_t     faction_reward;
		std::string completion_emote;
		uint32_t    replay_timer_group;
		uint32_t    replay_timer_seconds;
		uint32_t    request_timer_group;
		uint32_t    request_timer_seconds;
		uint32_t    dz_template_id;
		int32_t     lock_activity_id;
		int32_t     faction_amount;
		int16_t     enabled;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"type",
			"duration",
			"duration_code",
			"title",
			"description",
			"reward_text",
			"reward_id_list",
			"cash_reward",
			"exp_reward",
			"reward_method",
			"reward_points",
			"reward_point_type",
			"min_level",
			"max_level",
			"level_spread",
			"min_players",
			"max_players",
			"repeatable",
			"faction_reward",
			"completion_emote",
			"replay_timer_group",
			"replay_timer_seconds",
			"request_timer_group",
			"request_timer_seconds",
			"dz_template_id",
			"lock_activity_id",
			"faction_amount",
			"enabled",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"type",
			"duration",
			"duration_code",
			"title",
			"description",
			"reward_text",
			"reward_id_list",
			"cash_reward",
			"exp_reward",
			"reward_method",
			"reward_points",
			"reward_point_type",
			"min_level",
			"max_level",
			"level_spread",
			"min_players",
			"max_players",
			"repeatable",
			"faction_reward",
			"completion_emote",
			"replay_timer_group",
			"replay_timer_seconds",
			"request_timer_group",
			"request_timer_seconds",
			"dz_template_id",
			"lock_activity_id",
			"faction_amount",
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
		return std::string("tasks");
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

	static Tasks NewEntity()
	{
		Tasks e{};

		e.id                    = 0;
		e.type                  = 0;
		e.duration              = 0;
		e.duration_code         = 0;
		e.title                 = "";
		e.description           = "";
		e.reward_text           = "";
		e.reward_id_list        = "";
		e.cash_reward           = 0;
		e.exp_reward            = 0;
		e.reward_method         = 0;
		e.reward_points         = 0;
		e.reward_point_type     = 0;
		e.min_level             = 0;
		e.max_level             = 0;
		e.level_spread          = 0;
		e.min_players           = 0;
		e.max_players           = 0;
		e.repeatable            = 1;
		e.faction_reward        = 0;
		e.completion_emote      = "";
		e.replay_timer_group    = 0;
		e.replay_timer_seconds  = 0;
		e.request_timer_group   = 0;
		e.request_timer_seconds = 0;
		e.dz_template_id        = 0;
		e.lock_activity_id      = -1;
		e.faction_amount        = 0;
		e.enabled               = 1;

		return e;
	}

	static Tasks GetTasks(
		const std::vector<Tasks> &taskss,
		int tasks_id
	)
	{
		for (auto &tasks : taskss) {
			if (tasks.id == tasks_id) {
				return tasks;
			}
		}

		return NewEntity();
	}

	static Tasks FindOne(
		Database& db,
		int tasks_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				tasks_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Tasks e{};

			e.id                    = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.type                  = row[1] ? static_cast<int8_t>(atoi(row[1])) : 0;
			e.duration              = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.duration_code         = row[3] ? static_cast<int8_t>(atoi(row[3])) : 0;
			e.title                 = row[4] ? row[4] : "";
			e.description           = row[5] ? row[5] : "";
			e.reward_text           = row[6] ? row[6] : "";
			e.reward_id_list        = row[7] ? row[7] : "";
			e.cash_reward           = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.exp_reward            = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.reward_method         = row[10] ? static_cast<uint8_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.reward_points         = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;
			e.reward_point_type     = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.min_level             = row[13] ? static_cast<uint8_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.max_level             = row[14] ? static_cast<uint8_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.level_spread          = row[15] ? static_cast<uint32_t>(strtoul(row[15], nullptr, 10)) : 0;
			e.min_players           = row[16] ? static_cast<uint32_t>(strtoul(row[16], nullptr, 10)) : 0;
			e.max_players           = row[17] ? static_cast<uint32_t>(strtoul(row[17], nullptr, 10)) : 0;
			e.repeatable            = row[18] ? static_cast<uint8_t>(strtoul(row[18], nullptr, 10)) : 1;
			e.faction_reward        = row[19] ? static_cast<int32_t>(atoi(row[19])) : 0;
			e.completion_emote      = row[20] ? row[20] : "";
			e.replay_timer_group    = row[21] ? static_cast<uint32_t>(strtoul(row[21], nullptr, 10)) : 0;
			e.replay_timer_seconds  = row[22] ? static_cast<uint32_t>(strtoul(row[22], nullptr, 10)) : 0;
			e.request_timer_group   = row[23] ? static_cast<uint32_t>(strtoul(row[23], nullptr, 10)) : 0;
			e.request_timer_seconds = row[24] ? static_cast<uint32_t>(strtoul(row[24], nullptr, 10)) : 0;
			e.dz_template_id        = row[25] ? static_cast<uint32_t>(strtoul(row[25], nullptr, 10)) : 0;
			e.lock_activity_id      = row[26] ? static_cast<int32_t>(atoi(row[26])) : -1;
			e.faction_amount        = row[27] ? static_cast<int32_t>(atoi(row[27])) : 0;
			e.enabled               = row[28] ? static_cast<int16_t>(atoi(row[28])) : 1;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int tasks_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				tasks_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Tasks &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.id));
		v.push_back(columns[1] + " = " + std::to_string(e.type));
		v.push_back(columns[2] + " = " + std::to_string(e.duration));
		v.push_back(columns[3] + " = " + std::to_string(e.duration_code));
		v.push_back(columns[4] + " = '" + Strings::Escape(e.title) + "'");
		v.push_back(columns[5] + " = '" + Strings::Escape(e.description) + "'");
		v.push_back(columns[6] + " = '" + Strings::Escape(e.reward_text) + "'");
		v.push_back(columns[7] + " = '" + Strings::Escape(e.reward_id_list) + "'");
		v.push_back(columns[8] + " = " + std::to_string(e.cash_reward));
		v.push_back(columns[9] + " = " + std::to_string(e.exp_reward));
		v.push_back(columns[10] + " = " + std::to_string(e.reward_method));
		v.push_back(columns[11] + " = " + std::to_string(e.reward_points));
		v.push_back(columns[12] + " = " + std::to_string(e.reward_point_type));
		v.push_back(columns[13] + " = " + std::to_string(e.min_level));
		v.push_back(columns[14] + " = " + std::to_string(e.max_level));
		v.push_back(columns[15] + " = " + std::to_string(e.level_spread));
		v.push_back(columns[16] + " = " + std::to_string(e.min_players));
		v.push_back(columns[17] + " = " + std::to_string(e.max_players));
		v.push_back(columns[18] + " = " + std::to_string(e.repeatable));
		v.push_back(columns[19] + " = " + std::to_string(e.faction_reward));
		v.push_back(columns[20] + " = '" + Strings::Escape(e.completion_emote) + "'");
		v.push_back(columns[21] + " = " + std::to_string(e.replay_timer_group));
		v.push_back(columns[22] + " = " + std::to_string(e.replay_timer_seconds));
		v.push_back(columns[23] + " = " + std::to_string(e.request_timer_group));
		v.push_back(columns[24] + " = " + std::to_string(e.request_timer_seconds));
		v.push_back(columns[25] + " = " + std::to_string(e.dz_template_id));
		v.push_back(columns[26] + " = " + std::to_string(e.lock_activity_id));
		v.push_back(columns[27] + " = " + std::to_string(e.faction_amount));
		v.push_back(columns[28] + " = " + std::to_string(e.enabled));

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

	static Tasks InsertOne(
		Database& db,
		Tasks e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.duration));
		v.push_back(std::to_string(e.duration_code));
		v.push_back("'" + Strings::Escape(e.title) + "'");
		v.push_back("'" + Strings::Escape(e.description) + "'");
		v.push_back("'" + Strings::Escape(e.reward_text) + "'");
		v.push_back("'" + Strings::Escape(e.reward_id_list) + "'");
		v.push_back(std::to_string(e.cash_reward));
		v.push_back(std::to_string(e.exp_reward));
		v.push_back(std::to_string(e.reward_method));
		v.push_back(std::to_string(e.reward_points));
		v.push_back(std::to_string(e.reward_point_type));
		v.push_back(std::to_string(e.min_level));
		v.push_back(std::to_string(e.max_level));
		v.push_back(std::to_string(e.level_spread));
		v.push_back(std::to_string(e.min_players));
		v.push_back(std::to_string(e.max_players));
		v.push_back(std::to_string(e.repeatable));
		v.push_back(std::to_string(e.faction_reward));
		v.push_back("'" + Strings::Escape(e.completion_emote) + "'");
		v.push_back(std::to_string(e.replay_timer_group));
		v.push_back(std::to_string(e.replay_timer_seconds));
		v.push_back(std::to_string(e.request_timer_group));
		v.push_back(std::to_string(e.request_timer_seconds));
		v.push_back(std::to_string(e.dz_template_id));
		v.push_back(std::to_string(e.lock_activity_id));
		v.push_back(std::to_string(e.faction_amount));
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
		const std::vector<Tasks> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.duration));
			v.push_back(std::to_string(e.duration_code));
			v.push_back("'" + Strings::Escape(e.title) + "'");
			v.push_back("'" + Strings::Escape(e.description) + "'");
			v.push_back("'" + Strings::Escape(e.reward_text) + "'");
			v.push_back("'" + Strings::Escape(e.reward_id_list) + "'");
			v.push_back(std::to_string(e.cash_reward));
			v.push_back(std::to_string(e.exp_reward));
			v.push_back(std::to_string(e.reward_method));
			v.push_back(std::to_string(e.reward_points));
			v.push_back(std::to_string(e.reward_point_type));
			v.push_back(std::to_string(e.min_level));
			v.push_back(std::to_string(e.max_level));
			v.push_back(std::to_string(e.level_spread));
			v.push_back(std::to_string(e.min_players));
			v.push_back(std::to_string(e.max_players));
			v.push_back(std::to_string(e.repeatable));
			v.push_back(std::to_string(e.faction_reward));
			v.push_back("'" + Strings::Escape(e.completion_emote) + "'");
			v.push_back(std::to_string(e.replay_timer_group));
			v.push_back(std::to_string(e.replay_timer_seconds));
			v.push_back(std::to_string(e.request_timer_group));
			v.push_back(std::to_string(e.request_timer_seconds));
			v.push_back(std::to_string(e.dz_template_id));
			v.push_back(std::to_string(e.lock_activity_id));
			v.push_back(std::to_string(e.faction_amount));
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

	static std::vector<Tasks> All(Database& db)
	{
		std::vector<Tasks> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Tasks e{};

			e.id                    = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.type                  = row[1] ? static_cast<int8_t>(atoi(row[1])) : 0;
			e.duration              = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.duration_code         = row[3] ? static_cast<int8_t>(atoi(row[3])) : 0;
			e.title                 = row[4] ? row[4] : "";
			e.description           = row[5] ? row[5] : "";
			e.reward_text           = row[6] ? row[6] : "";
			e.reward_id_list        = row[7] ? row[7] : "";
			e.cash_reward           = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.exp_reward            = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.reward_method         = row[10] ? static_cast<uint8_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.reward_points         = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;
			e.reward_point_type     = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.min_level             = row[13] ? static_cast<uint8_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.max_level             = row[14] ? static_cast<uint8_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.level_spread          = row[15] ? static_cast<uint32_t>(strtoul(row[15], nullptr, 10)) : 0;
			e.min_players           = row[16] ? static_cast<uint32_t>(strtoul(row[16], nullptr, 10)) : 0;
			e.max_players           = row[17] ? static_cast<uint32_t>(strtoul(row[17], nullptr, 10)) : 0;
			e.repeatable            = row[18] ? static_cast<uint8_t>(strtoul(row[18], nullptr, 10)) : 1;
			e.faction_reward        = row[19] ? static_cast<int32_t>(atoi(row[19])) : 0;
			e.completion_emote      = row[20] ? row[20] : "";
			e.replay_timer_group    = row[21] ? static_cast<uint32_t>(strtoul(row[21], nullptr, 10)) : 0;
			e.replay_timer_seconds  = row[22] ? static_cast<uint32_t>(strtoul(row[22], nullptr, 10)) : 0;
			e.request_timer_group   = row[23] ? static_cast<uint32_t>(strtoul(row[23], nullptr, 10)) : 0;
			e.request_timer_seconds = row[24] ? static_cast<uint32_t>(strtoul(row[24], nullptr, 10)) : 0;
			e.dz_template_id        = row[25] ? static_cast<uint32_t>(strtoul(row[25], nullptr, 10)) : 0;
			e.lock_activity_id      = row[26] ? static_cast<int32_t>(atoi(row[26])) : -1;
			e.faction_amount        = row[27] ? static_cast<int32_t>(atoi(row[27])) : 0;
			e.enabled               = row[28] ? static_cast<int16_t>(atoi(row[28])) : 1;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Tasks> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Tasks> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Tasks e{};

			e.id                    = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.type                  = row[1] ? static_cast<int8_t>(atoi(row[1])) : 0;
			e.duration              = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.duration_code         = row[3] ? static_cast<int8_t>(atoi(row[3])) : 0;
			e.title                 = row[4] ? row[4] : "";
			e.description           = row[5] ? row[5] : "";
			e.reward_text           = row[6] ? row[6] : "";
			e.reward_id_list        = row[7] ? row[7] : "";
			e.cash_reward           = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.exp_reward            = row[9] ? static_cast<int32_t>(atoi(row[9])) : 0;
			e.reward_method         = row[10] ? static_cast<uint8_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.reward_points         = row[11] ? static_cast<int32_t>(atoi(row[11])) : 0;
			e.reward_point_type     = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.min_level             = row[13] ? static_cast<uint8_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.max_level             = row[14] ? static_cast<uint8_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.level_spread          = row[15] ? static_cast<uint32_t>(strtoul(row[15], nullptr, 10)) : 0;
			e.min_players           = row[16] ? static_cast<uint32_t>(strtoul(row[16], nullptr, 10)) : 0;
			e.max_players           = row[17] ? static_cast<uint32_t>(strtoul(row[17], nullptr, 10)) : 0;
			e.repeatable            = row[18] ? static_cast<uint8_t>(strtoul(row[18], nullptr, 10)) : 1;
			e.faction_reward        = row[19] ? static_cast<int32_t>(atoi(row[19])) : 0;
			e.completion_emote      = row[20] ? row[20] : "";
			e.replay_timer_group    = row[21] ? static_cast<uint32_t>(strtoul(row[21], nullptr, 10)) : 0;
			e.replay_timer_seconds  = row[22] ? static_cast<uint32_t>(strtoul(row[22], nullptr, 10)) : 0;
			e.request_timer_group   = row[23] ? static_cast<uint32_t>(strtoul(row[23], nullptr, 10)) : 0;
			e.request_timer_seconds = row[24] ? static_cast<uint32_t>(strtoul(row[24], nullptr, 10)) : 0;
			e.dz_template_id        = row[25] ? static_cast<uint32_t>(strtoul(row[25], nullptr, 10)) : 0;
			e.lock_activity_id      = row[26] ? static_cast<int32_t>(atoi(row[26])) : -1;
			e.faction_amount        = row[27] ? static_cast<int32_t>(atoi(row[27])) : 0;
			e.enabled               = row[28] ? static_cast<int16_t>(atoi(row[28])) : 1;

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
		const Tasks &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.duration));
		v.push_back(std::to_string(e.duration_code));
		v.push_back("'" + Strings::Escape(e.title) + "'");
		v.push_back("'" + Strings::Escape(e.description) + "'");
		v.push_back("'" + Strings::Escape(e.reward_text) + "'");
		v.push_back("'" + Strings::Escape(e.reward_id_list) + "'");
		v.push_back(std::to_string(e.cash_reward));
		v.push_back(std::to_string(e.exp_reward));
		v.push_back(std::to_string(e.reward_method));
		v.push_back(std::to_string(e.reward_points));
		v.push_back(std::to_string(e.reward_point_type));
		v.push_back(std::to_string(e.min_level));
		v.push_back(std::to_string(e.max_level));
		v.push_back(std::to_string(e.level_spread));
		v.push_back(std::to_string(e.min_players));
		v.push_back(std::to_string(e.max_players));
		v.push_back(std::to_string(e.repeatable));
		v.push_back(std::to_string(e.faction_reward));
		v.push_back("'" + Strings::Escape(e.completion_emote) + "'");
		v.push_back(std::to_string(e.replay_timer_group));
		v.push_back(std::to_string(e.replay_timer_seconds));
		v.push_back(std::to_string(e.request_timer_group));
		v.push_back(std::to_string(e.request_timer_seconds));
		v.push_back(std::to_string(e.dz_template_id));
		v.push_back(std::to_string(e.lock_activity_id));
		v.push_back(std::to_string(e.faction_amount));
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
		const std::vector<Tasks> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.duration));
			v.push_back(std::to_string(e.duration_code));
			v.push_back("'" + Strings::Escape(e.title) + "'");
			v.push_back("'" + Strings::Escape(e.description) + "'");
			v.push_back("'" + Strings::Escape(e.reward_text) + "'");
			v.push_back("'" + Strings::Escape(e.reward_id_list) + "'");
			v.push_back(std::to_string(e.cash_reward));
			v.push_back(std::to_string(e.exp_reward));
			v.push_back(std::to_string(e.reward_method));
			v.push_back(std::to_string(e.reward_points));
			v.push_back(std::to_string(e.reward_point_type));
			v.push_back(std::to_string(e.min_level));
			v.push_back(std::to_string(e.max_level));
			v.push_back(std::to_string(e.level_spread));
			v.push_back(std::to_string(e.min_players));
			v.push_back(std::to_string(e.max_players));
			v.push_back(std::to_string(e.repeatable));
			v.push_back(std::to_string(e.faction_reward));
			v.push_back("'" + Strings::Escape(e.completion_emote) + "'");
			v.push_back(std::to_string(e.replay_timer_group));
			v.push_back(std::to_string(e.replay_timer_seconds));
			v.push_back(std::to_string(e.request_timer_group));
			v.push_back(std::to_string(e.request_timer_seconds));
			v.push_back(std::to_string(e.dz_template_id));
			v.push_back(std::to_string(e.lock_activity_id));
			v.push_back(std::to_string(e.faction_amount));
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
