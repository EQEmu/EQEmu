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

class BaseTaskActivitiesRepository {
public:
	struct TaskActivities {
		uint32_t    taskid;
		uint32_t    activityid;
		int32_t     req_activity_id;
		int32_t     step;
		uint8_t     activitytype;
		std::string target_name;
		uint32_t    goalmethod;
		int32_t     goalcount;
		std::string description_override;
		std::string npc_match_list;
		std::string item_id_list;
		std::string item_list;
		int32_t     dz_switch_id;
		float       min_x;
		float       min_y;
		float       min_z;
		float       max_x;
		float       max_y;
		float       max_z;
		std::string skill_list;
		std::string spell_list;
		std::string zones;
		int32_t     zone_version;
		int8_t      optional;
		uint8_t     list_group;
	};

	static std::string PrimaryKey()
	{
		return std::string("taskid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"taskid",
			"activityid",
			"req_activity_id",
			"step",
			"activitytype",
			"target_name",
			"goalmethod",
			"goalcount",
			"description_override",
			"npc_match_list",
			"item_id_list",
			"item_list",
			"dz_switch_id",
			"min_x",
			"min_y",
			"min_z",
			"max_x",
			"max_y",
			"max_z",
			"skill_list",
			"spell_list",
			"zones",
			"zone_version",
			"optional",
			"list_group",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"taskid",
			"activityid",
			"req_activity_id",
			"step",
			"activitytype",
			"target_name",
			"goalmethod",
			"goalcount",
			"description_override",
			"npc_match_list",
			"item_id_list",
			"item_list",
			"dz_switch_id",
			"min_x",
			"min_y",
			"min_z",
			"max_x",
			"max_y",
			"max_z",
			"skill_list",
			"spell_list",
			"zones",
			"zone_version",
			"optional",
			"list_group",
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
		return std::string("task_activities");
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

	static TaskActivities NewEntity()
	{
		TaskActivities e{};

		e.taskid               = 0;
		e.activityid           = 0;
		e.req_activity_id      = -1;
		e.step                 = 0;
		e.activitytype         = 0;
		e.target_name          = "";
		e.goalmethod           = 0;
		e.goalcount            = 1;
		e.description_override = "";
		e.npc_match_list       = "";
		e.item_id_list         = "";
		e.item_list            = "";
		e.dz_switch_id         = 0;
		e.min_x                = 0;
		e.min_y                = 0;
		e.min_z                = 0;
		e.max_x                = 0;
		e.max_y                = 0;
		e.max_z                = 0;
		e.skill_list           = "-1";
		e.spell_list           = "0";
		e.zones                = "";
		e.zone_version         = -1;
		e.optional             = 0;
		e.list_group           = 0;

		return e;
	}

	static TaskActivities GetTaskActivities(
		const std::vector<TaskActivities> &task_activitiess,
		int task_activities_id
	)
	{
		for (auto &task_activities : task_activitiess) {
			if (task_activities.taskid == task_activities_id) {
				return task_activities;
			}
		}

		return NewEntity();
	}

	static TaskActivities FindOne(
		Database& db,
		int task_activities_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				task_activities_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			TaskActivities e{};

			e.taskid               = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.activityid           = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.req_activity_id      = row[2] ? static_cast<int32_t>(atoi(row[2])) : -1;
			e.step                 = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.activitytype         = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.target_name          = row[5] ? row[5] : "";
			e.goalmethod           = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.goalcount            = row[7] ? static_cast<int32_t>(atoi(row[7])) : 1;
			e.description_override = row[8] ? row[8] : "";
			e.npc_match_list       = row[9] ? row[9] : "";
			e.item_id_list         = row[10] ? row[10] : "";
			e.item_list            = row[11] ? row[11] : "";
			e.dz_switch_id         = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.min_x                = row[13] ? strtof(row[13], nullptr) : 0;
			e.min_y                = row[14] ? strtof(row[14], nullptr) : 0;
			e.min_z                = row[15] ? strtof(row[15], nullptr) : 0;
			e.max_x                = row[16] ? strtof(row[16], nullptr) : 0;
			e.max_y                = row[17] ? strtof(row[17], nullptr) : 0;
			e.max_z                = row[18] ? strtof(row[18], nullptr) : 0;
			e.skill_list           = row[19] ? row[19] : "-1";
			e.spell_list           = row[20] ? row[20] : "0";
			e.zones                = row[21] ? row[21] : "";
			e.zone_version         = row[22] ? static_cast<int32_t>(atoi(row[22])) : -1;
			e.optional             = row[23] ? static_cast<int8_t>(atoi(row[23])) : 0;
			e.list_group           = row[24] ? static_cast<uint8_t>(strtoul(row[24], nullptr, 10)) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int task_activities_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				task_activities_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const TaskActivities &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.taskid));
		v.push_back(columns[1] + " = " + std::to_string(e.activityid));
		v.push_back(columns[2] + " = " + std::to_string(e.req_activity_id));
		v.push_back(columns[3] + " = " + std::to_string(e.step));
		v.push_back(columns[4] + " = " + std::to_string(e.activitytype));
		v.push_back(columns[5] + " = '" + Strings::Escape(e.target_name) + "'");
		v.push_back(columns[6] + " = " + std::to_string(e.goalmethod));
		v.push_back(columns[7] + " = " + std::to_string(e.goalcount));
		v.push_back(columns[8] + " = '" + Strings::Escape(e.description_override) + "'");
		v.push_back(columns[9] + " = '" + Strings::Escape(e.npc_match_list) + "'");
		v.push_back(columns[10] + " = '" + Strings::Escape(e.item_id_list) + "'");
		v.push_back(columns[11] + " = '" + Strings::Escape(e.item_list) + "'");
		v.push_back(columns[12] + " = " + std::to_string(e.dz_switch_id));
		v.push_back(columns[13] + " = " + std::to_string(e.min_x));
		v.push_back(columns[14] + " = " + std::to_string(e.min_y));
		v.push_back(columns[15] + " = " + std::to_string(e.min_z));
		v.push_back(columns[16] + " = " + std::to_string(e.max_x));
		v.push_back(columns[17] + " = " + std::to_string(e.max_y));
		v.push_back(columns[18] + " = " + std::to_string(e.max_z));
		v.push_back(columns[19] + " = '" + Strings::Escape(e.skill_list) + "'");
		v.push_back(columns[20] + " = '" + Strings::Escape(e.spell_list) + "'");
		v.push_back(columns[21] + " = '" + Strings::Escape(e.zones) + "'");
		v.push_back(columns[22] + " = " + std::to_string(e.zone_version));
		v.push_back(columns[23] + " = " + std::to_string(e.optional));
		v.push_back(columns[24] + " = " + std::to_string(e.list_group));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.taskid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static TaskActivities InsertOne(
		Database& db,
		TaskActivities e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.taskid));
		v.push_back(std::to_string(e.activityid));
		v.push_back(std::to_string(e.req_activity_id));
		v.push_back(std::to_string(e.step));
		v.push_back(std::to_string(e.activitytype));
		v.push_back("'" + Strings::Escape(e.target_name) + "'");
		v.push_back(std::to_string(e.goalmethod));
		v.push_back(std::to_string(e.goalcount));
		v.push_back("'" + Strings::Escape(e.description_override) + "'");
		v.push_back("'" + Strings::Escape(e.npc_match_list) + "'");
		v.push_back("'" + Strings::Escape(e.item_id_list) + "'");
		v.push_back("'" + Strings::Escape(e.item_list) + "'");
		v.push_back(std::to_string(e.dz_switch_id));
		v.push_back(std::to_string(e.min_x));
		v.push_back(std::to_string(e.min_y));
		v.push_back(std::to_string(e.min_z));
		v.push_back(std::to_string(e.max_x));
		v.push_back(std::to_string(e.max_y));
		v.push_back(std::to_string(e.max_z));
		v.push_back("'" + Strings::Escape(e.skill_list) + "'");
		v.push_back("'" + Strings::Escape(e.spell_list) + "'");
		v.push_back("'" + Strings::Escape(e.zones) + "'");
		v.push_back(std::to_string(e.zone_version));
		v.push_back(std::to_string(e.optional));
		v.push_back(std::to_string(e.list_group));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.taskid = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<TaskActivities> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.taskid));
			v.push_back(std::to_string(e.activityid));
			v.push_back(std::to_string(e.req_activity_id));
			v.push_back(std::to_string(e.step));
			v.push_back(std::to_string(e.activitytype));
			v.push_back("'" + Strings::Escape(e.target_name) + "'");
			v.push_back(std::to_string(e.goalmethod));
			v.push_back(std::to_string(e.goalcount));
			v.push_back("'" + Strings::Escape(e.description_override) + "'");
			v.push_back("'" + Strings::Escape(e.npc_match_list) + "'");
			v.push_back("'" + Strings::Escape(e.item_id_list) + "'");
			v.push_back("'" + Strings::Escape(e.item_list) + "'");
			v.push_back(std::to_string(e.dz_switch_id));
			v.push_back(std::to_string(e.min_x));
			v.push_back(std::to_string(e.min_y));
			v.push_back(std::to_string(e.min_z));
			v.push_back(std::to_string(e.max_x));
			v.push_back(std::to_string(e.max_y));
			v.push_back(std::to_string(e.max_z));
			v.push_back("'" + Strings::Escape(e.skill_list) + "'");
			v.push_back("'" + Strings::Escape(e.spell_list) + "'");
			v.push_back("'" + Strings::Escape(e.zones) + "'");
			v.push_back(std::to_string(e.zone_version));
			v.push_back(std::to_string(e.optional));
			v.push_back(std::to_string(e.list_group));

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

	static std::vector<TaskActivities> All(Database& db)
	{
		std::vector<TaskActivities> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			TaskActivities e{};

			e.taskid               = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.activityid           = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.req_activity_id      = row[2] ? static_cast<int32_t>(atoi(row[2])) : -1;
			e.step                 = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.activitytype         = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.target_name          = row[5] ? row[5] : "";
			e.goalmethod           = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.goalcount            = row[7] ? static_cast<int32_t>(atoi(row[7])) : 1;
			e.description_override = row[8] ? row[8] : "";
			e.npc_match_list       = row[9] ? row[9] : "";
			e.item_id_list         = row[10] ? row[10] : "";
			e.item_list            = row[11] ? row[11] : "";
			e.dz_switch_id         = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.min_x                = row[13] ? strtof(row[13], nullptr) : 0;
			e.min_y                = row[14] ? strtof(row[14], nullptr) : 0;
			e.min_z                = row[15] ? strtof(row[15], nullptr) : 0;
			e.max_x                = row[16] ? strtof(row[16], nullptr) : 0;
			e.max_y                = row[17] ? strtof(row[17], nullptr) : 0;
			e.max_z                = row[18] ? strtof(row[18], nullptr) : 0;
			e.skill_list           = row[19] ? row[19] : "-1";
			e.spell_list           = row[20] ? row[20] : "0";
			e.zones                = row[21] ? row[21] : "";
			e.zone_version         = row[22] ? static_cast<int32_t>(atoi(row[22])) : -1;
			e.optional             = row[23] ? static_cast<int8_t>(atoi(row[23])) : 0;
			e.list_group           = row[24] ? static_cast<uint8_t>(strtoul(row[24], nullptr, 10)) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<TaskActivities> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<TaskActivities> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			TaskActivities e{};

			e.taskid               = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.activityid           = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.req_activity_id      = row[2] ? static_cast<int32_t>(atoi(row[2])) : -1;
			e.step                 = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.activitytype         = row[4] ? static_cast<uint8_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.target_name          = row[5] ? row[5] : "";
			e.goalmethod           = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.goalcount            = row[7] ? static_cast<int32_t>(atoi(row[7])) : 1;
			e.description_override = row[8] ? row[8] : "";
			e.npc_match_list       = row[9] ? row[9] : "";
			e.item_id_list         = row[10] ? row[10] : "";
			e.item_list            = row[11] ? row[11] : "";
			e.dz_switch_id         = row[12] ? static_cast<int32_t>(atoi(row[12])) : 0;
			e.min_x                = row[13] ? strtof(row[13], nullptr) : 0;
			e.min_y                = row[14] ? strtof(row[14], nullptr) : 0;
			e.min_z                = row[15] ? strtof(row[15], nullptr) : 0;
			e.max_x                = row[16] ? strtof(row[16], nullptr) : 0;
			e.max_y                = row[17] ? strtof(row[17], nullptr) : 0;
			e.max_z                = row[18] ? strtof(row[18], nullptr) : 0;
			e.skill_list           = row[19] ? row[19] : "-1";
			e.spell_list           = row[20] ? row[20] : "0";
			e.zones                = row[21] ? row[21] : "";
			e.zone_version         = row[22] ? static_cast<int32_t>(atoi(row[22])) : -1;
			e.optional             = row[23] ? static_cast<int8_t>(atoi(row[23])) : 0;
			e.list_group           = row[24] ? static_cast<uint8_t>(strtoul(row[24], nullptr, 10)) : 0;

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
		const TaskActivities &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.taskid));
		v.push_back(std::to_string(e.activityid));
		v.push_back(std::to_string(e.req_activity_id));
		v.push_back(std::to_string(e.step));
		v.push_back(std::to_string(e.activitytype));
		v.push_back("'" + Strings::Escape(e.target_name) + "'");
		v.push_back(std::to_string(e.goalmethod));
		v.push_back(std::to_string(e.goalcount));
		v.push_back("'" + Strings::Escape(e.description_override) + "'");
		v.push_back("'" + Strings::Escape(e.npc_match_list) + "'");
		v.push_back("'" + Strings::Escape(e.item_id_list) + "'");
		v.push_back("'" + Strings::Escape(e.item_list) + "'");
		v.push_back(std::to_string(e.dz_switch_id));
		v.push_back(std::to_string(e.min_x));
		v.push_back(std::to_string(e.min_y));
		v.push_back(std::to_string(e.min_z));
		v.push_back(std::to_string(e.max_x));
		v.push_back(std::to_string(e.max_y));
		v.push_back(std::to_string(e.max_z));
		v.push_back("'" + Strings::Escape(e.skill_list) + "'");
		v.push_back("'" + Strings::Escape(e.spell_list) + "'");
		v.push_back("'" + Strings::Escape(e.zones) + "'");
		v.push_back(std::to_string(e.zone_version));
		v.push_back(std::to_string(e.optional));
		v.push_back(std::to_string(e.list_group));

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
		const std::vector<TaskActivities> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.taskid));
			v.push_back(std::to_string(e.activityid));
			v.push_back(std::to_string(e.req_activity_id));
			v.push_back(std::to_string(e.step));
			v.push_back(std::to_string(e.activitytype));
			v.push_back("'" + Strings::Escape(e.target_name) + "'");
			v.push_back(std::to_string(e.goalmethod));
			v.push_back(std::to_string(e.goalcount));
			v.push_back("'" + Strings::Escape(e.description_override) + "'");
			v.push_back("'" + Strings::Escape(e.npc_match_list) + "'");
			v.push_back("'" + Strings::Escape(e.item_id_list) + "'");
			v.push_back("'" + Strings::Escape(e.item_list) + "'");
			v.push_back(std::to_string(e.dz_switch_id));
			v.push_back(std::to_string(e.min_x));
			v.push_back(std::to_string(e.min_y));
			v.push_back(std::to_string(e.min_z));
			v.push_back(std::to_string(e.max_x));
			v.push_back(std::to_string(e.max_y));
			v.push_back(std::to_string(e.max_z));
			v.push_back("'" + Strings::Escape(e.skill_list) + "'");
			v.push_back("'" + Strings::Escape(e.spell_list) + "'");
			v.push_back("'" + Strings::Escape(e.zones) + "'");
			v.push_back(std::to_string(e.zone_version));
			v.push_back(std::to_string(e.optional));
			v.push_back(std::to_string(e.list_group));

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
