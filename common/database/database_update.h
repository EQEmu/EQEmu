/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include "common/database.h"

struct ManifestEntry {
	int         version{};     // database version of the migration
	std::string description{}; // description of the migration ex: "add_new_table" or "add_index_to_table"
	std::string check{};       // query that checks against the condition
	std::string condition{};   // condition or "match_type" - Possible values [contains|match|missing|empty|not_empty]
	std::string match{};       // match field that is not always used, but works in conjunction with "condition" values [missing|match|contains]
	std::string sql{};         // the SQL DDL that gets ran when the condition is true
	bool        content_schema_update{}; // if true, this migration is a content schema update and should be ran against the content database
	bool        force_interactive; // if true, this migration will always be run interactively
};

struct DatabaseVersion {
	int server_database_version;
	int bots_database_version;
	int custom_database_version;
};

class DatabaseUpdate {
public:
	DatabaseVersion GetDatabaseVersions();
	DatabaseVersion GetBinaryDatabaseVersions();
	void CheckDbUpdates();
	std::string GetQueryResult(const ManifestEntry& e);
	static bool ShouldRunMigration(ManifestEntry &e, std::string query_result);
	bool UpdateManifest(std::vector<ManifestEntry> entries, int version_low, int version_high);

	DatabaseUpdate *SetDatabase(Database *db);
	DatabaseUpdate *SetContentDatabase(Database *db);
	DatabaseUpdate *SetSkipBackup(bool skip);
	bool HasPendingUpdates();

	static DatabaseUpdate* Instance()
	{
		static DatabaseUpdate instance;
		return &instance;
	}

private:
	bool m_skip_backup = false;
	Database *m_database;
	Database *m_content_database;
	static bool CheckVersionsUpToDate(DatabaseVersion v, DatabaseVersion b);
	void InjectBotsVersionColumn();
	void InjectCustomVersionColumn();
};
