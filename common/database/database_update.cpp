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
#include "database_update.h"

#include "common/database.h"
#include "common/database/database_dump_service.h"
#include "common/database/database_update_manifest_bots.h"
#include "common/database/database_update_manifest_custom.h"
#include "common/database/database_update_manifest.h"
#include "common/eqemu_logsys.h"
#include "common/http/httplib.h"
#include "common/rulesys.h"
#include "common/strings.h"

#include <filesystem>
#include <ranges>


constexpr int BREAK_LENGTH = 70;

DatabaseVersion DatabaseUpdate::GetDatabaseVersions()
{
	auto results = m_database->QueryDatabase("SELECT `version`, `bots_version`, `custom_version` FROM `db_version` LIMIT 1");
	if (!results.Success() || !results.RowCount()) {
		LogError("Failed to read from [db_version] table!");
		return DatabaseVersion{};
	}

	auto r = results.begin();

	return DatabaseVersion{
		.server_database_version = Strings::ToInt(r[0]),
		.bots_database_version = Strings::ToInt(r[1]),
		.custom_database_version = Strings::ToInt(r[2]),
	};
}

DatabaseVersion DatabaseUpdate::GetBinaryDatabaseVersions()
{
	return DatabaseVersion{
		.server_database_version = CURRENT_BINARY_DATABASE_VERSION,
		.bots_database_version = (RuleB(Bots, Enabled) ? CURRENT_BINARY_BOTS_DATABASE_VERSION : 0),
		.custom_database_version = CUSTOM_BINARY_DATABASE_VERSION,
	};
}

// the amount of versions we look-back to ensure we have all migrations
// we may not want to force these, but just warn about the look-backs
constexpr int LOOK_BACK_AMOUNT = 10;

// this check will take action
void DatabaseUpdate::CheckDbUpdates()
{
	InjectCustomVersionColumn();
	InjectBotsVersionColumn();
	auto v = GetDatabaseVersions();
	auto b = GetBinaryDatabaseVersions();
	if (CheckVersionsUpToDate(v, b)) {
		return;
	}

	int server_database_version = UpdateManifest(manifest_entries, v.server_database_version, b.server_database_version);
	if (server_database_version > v.server_database_version) {
		LogInfo(
			"Updates ran successfully, setting database version to [{}] from [{}]",
			server_database_version,
			v.server_database_version
		);
		m_database->QueryDatabase(fmt::format("UPDATE `db_version` SET `version` = {}", server_database_version));
	}

	int custom_database_version = UpdateManifest(manifest_entries_custom, v.custom_database_version, b.custom_database_version);
	if (custom_database_version > v.server_database_version) {
		LogInfo(
			"Updates ran successfully, setting database version to [{}] from [{}]",
			custom_database_version,
			v.custom_database_version
		);
		m_database->QueryDatabase(fmt::format("UPDATE `db_version` SET `custom_version` = {}", custom_database_version));
	}

	if (b.bots_database_version > 0) {
		int bots_database_version = UpdateManifest(bot_manifest_entries, v.bots_database_version, b.bots_database_version);
		if (bots_database_version > v.bots_database_version) {
			LogInfo(
				"Updates ran successfully, setting database version to [{}] from [{}]",
				bots_database_version,
				v.bots_database_version
			);
			m_database->QueryDatabase(
				fmt::format(
					"UPDATE `db_version` SET `bots_version` = {}",
					bots_database_version
				)
			);
		}
	}
}

std::string DatabaseUpdate::GetQueryResult(const ManifestEntry& e)
{
	auto results = (e.content_schema_update ? m_content_database : m_database)->QueryDatabase(e.check);

	std::vector<std::string> result_lines = {};

	for (auto row = results.begin(); row != results.end(); ++row) {
		std::vector<std::string> cols;

		int field_count = results.ColumnCount();
		cols.reserve(field_count);
		for (int i = 0; i < field_count; ++i) {
			if (row[i] != nullptr) {
				cols.emplace_back(row[i]);
			}
		}

		result_lines.emplace_back(Strings::Join(cols, " "));
	}

	return Strings::Join(result_lines, "\n");
}

bool DatabaseUpdate::ShouldRunMigration(const ManifestEntry& e, std::string& query_result)
{
	std::string r = Strings::Trim(query_result);
	if (e.condition == "contains") {
		return Strings::Contains(r, e.match);
	}
	else if (e.condition == "match") {
		return r == e.match;
	}
	else if (e.condition == "missing") {
		return !Strings::Contains(r, e.match);
	}
	else if (e.condition == "empty") {
		return r.empty();
	}
	else if (e.condition == "not_empty") {
		return !r.empty();
	}

	return false;
}

// check if we are running in a terminal
bool is_atty()
{
#ifdef _WINDOWS
	return ::_isatty(_fileno(stdin));
#else
	return isatty(fileno(stdin));
#endif
}

std::string DisplayPrompt(const std::string& prompt)
{
	std::string input;
	if (is_atty()) {
		LogInfo("{} (Timeout 60s)", prompt);

		// user input
		bool gave_input = false;
		time_t start_time = time(nullptr);
		time_t wait_time_seconds = 60;

		// spawn a concurrent thread that waits for input from std::cin
		std::thread t1(
			[&]() {
				std::cin >> input;
				gave_input = true;
			}
		);
		t1.detach();

		// check the inputReceived flag once every 50ms for 10 seconds
		while (time(nullptr) < start_time + wait_time_seconds && !gave_input) {
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	}

	return Strings::Trim(input);
}

// return last successful version updated
int DatabaseUpdate::UpdateManifest(
	std::vector<ManifestEntry>& entries,
	int version_low,
	int version_high
)
{
	int latest_version = version_low;
	if (version_low != version_high) {
		// assume at this point that we have a migration to do because there is a version number difference. If a
		// migration for a specific manifest entry does not happen because of a missing test, then log it and
		// continue (the assumption here is that the user has manually fixed the database at this point). If a force
		// interactive flag is set, then stop for each query. Fail the migration if the user says no or it times out
		// because it means the database isn't going to have a correct state to continue. Start with backing up the
		// database as per user options.

		if (!m_skip_backup) {
			LogInfo("Automatically backing up database before applying updates");
			LogInfo("{}", Strings::Repeat("-", BREAK_LENGTH));
			auto s = DatabaseDumpService();
			s.SetDumpAllTables(true);
			s.SetDumpWithCompression(true);
			s.DatabaseDump();
			LogInfo("{}", Strings::Repeat("-", BREAK_LENGTH));
		} else {
			LogInfo("Skipping database backup");
		}

		LogInfo("Running database migrations. Please wait...");
		LogInfo("{}", Strings::Repeat("-", BREAK_LENGTH));

		auto filtered_entries = entries | std::views::filter(
			[version_low, version_high](const ManifestEntry& entry)
			{ return entry.version > version_low && entry.version <= version_high; });

		std::vector<ManifestEntry> sorted_entries{filtered_entries.begin(), filtered_entries.end()};
		std::ranges::sort(sorted_entries, {}, &ManifestEntry::version);

		for (const auto& entry : sorted_entries) {
			// this is the test to run this individual migration. If the test fails, then it is safe to assume
			// that this migration has already happened manually or otherwise and it's safe to skip
			// suppress error messages here, it's all tested in the following function
			EQEmuLogSys::Instance()->DisableMySQLErrorLogs();
			std::string result = GetQueryResult(entry);
			EQEmuLogSys::Instance()->EnableMySQLErrorLogs();

			if (ShouldRunMigration(entry, result)) {
				if (entry.force_interactive && !std::getenv("FORCE_INTERACTIVE")) {
					LogInfo("{}", Strings::Repeat("-", BREAK_LENGTH));
					LogInfo("This migration requires user input. Running interactively");
					LogInfo("This is usually due to a major change that could cause data loss");
					LogInfo("Your server is automatically backed up before these updates are applied");
					LogInfo("but you should also make sure you take a backup prior to running this update");
					LogInfo("{}", Strings::Repeat("-", BREAK_LENGTH));

					// prompt for user skip
					if (DisplayPrompt("Would you like to run this update? [y/n]") != "y") {
						LogInfo("Exiting due to user input");
						return latest_version;
					}
				}

				auto r = (entry.content_schema_update ? m_content_database : m_database)->QueryDatabaseMulti(entry.sql);

				// ignore empty query result "errors"
				if (r.ErrorNumber() != 1065 && !r.ErrorMessage().empty()) {
					LogError("(#{}) [{}]", r.ErrorNumber(), r.ErrorMessage());
					LogInfo("Required database update failed.");

					// if terminal attached then prompt for skip
					if (DisplayPrompt("Would you like to skip this update? [y/n]") == "y") {
						LogInfo("Skipping update [{}] [{}]", entry.version, entry.description);
					} else {
						LogError("Fatal | Database migration [{}] failed to run", entry.description);
						LogError("Fatal | Shutting down");
						return latest_version;
					}

					LogInfo("[{}] [{}] [error]", entry.version, entry.description);
				} else {
					LogInfo("[{}] [{}] [ok]", entry.version, entry.description);
				}

				latest_version = entry.version;
			}
		}

		LogInfo("{}", Strings::Repeat("-", BREAK_LENGTH));
	}

	return latest_version;
}

DatabaseUpdate *DatabaseUpdate::SetDatabase(Database *db)
{
	m_database = db;

	return this;
}

DatabaseUpdate *DatabaseUpdate::SetContentDatabase(Database *db)
{
	m_content_database = db;

	return this;
}

DatabaseUpdate *DatabaseUpdate::SetSkipBackup(bool skip)
{
	m_skip_backup = skip;

	return this;
}

bool DatabaseUpdate::CheckVersionsUpToDate(DatabaseVersion v, DatabaseVersion b)
{
	LogInfo("{}", Strings::Repeat("-", BREAK_LENGTH));

	LogInfo(
		"{:>8} | database [{}] binary [{}] {}",
		"Server",
		v.server_database_version,
		b.server_database_version,
		(v.server_database_version == b.server_database_version) ? "up to date" : "checking updates"
	);

	if (RuleB(Bots, Enabled) && b.bots_database_version > 0) {
		LogInfo(
			"{:>8} | database [{}] binary [{}] {}",
			"Bots",
			v.bots_database_version,
			b.bots_database_version,
			(v.bots_database_version == b.bots_database_version) ? "up to date" : "checking updates"
		);
	}

	if (b.custom_database_version > 0) {
		LogInfo(
			"{:>8} | database [{}] binary [{}] {}",
			"Custom",
			v.custom_database_version,
			b.custom_database_version,
			(v.custom_database_version == b.custom_database_version) ? "up to date" : "checking updates"
		);
	}

	LogInfo("{:>8} | [server.auto_database_updates] [<green>true]", "Config");

	LogInfo("{}", Strings::Repeat("-", BREAK_LENGTH));

	// server database version is required
	bool server_up_to_date = v.server_database_version >= b.server_database_version;
	// bots database version is optional, if not enabled then it is always up-to-date
	bool bots_up_to_date   = RuleB(Bots, Enabled) ? v.bots_database_version >= b.bots_database_version : true;

	// custom database version is optional, if not enabled then it is always up-to-date
	bool custom_up_to_date = v.custom_database_version >= b.custom_database_version;

	return server_up_to_date && bots_up_to_date && custom_up_to_date;
}

// checks to see if there are pending updates
// used by zone to prevent launch or boot loop until updates are applied
bool DatabaseUpdate::HasPendingUpdates()
{
	auto v = GetDatabaseVersions();
	auto b = GetBinaryDatabaseVersions();

	return !CheckVersionsUpToDate(v, b);
}

void DatabaseUpdate::InjectBotsVersionColumn()
{
	auto r = m_database->QueryDatabase("show columns from db_version where Field like '%bots_version%'");
	if (r.RowCount() == 0) {
		m_database->QueryDatabase("ALTER TABLE db_version ADD bots_version int(11) DEFAULT '0' AFTER version");
	}
}

void DatabaseUpdate::InjectCustomVersionColumn()
{
	auto results = m_database->QueryDatabase("SHOW COLUMNS FROM `db_version` LIKE 'custom_version'");
	if (!results.Success() || results.RowCount() == 0) {
		LogInfo("Adding custom_version column to db_version table");
		m_database->QueryDatabase("ALTER TABLE `db_version` ADD COLUMN `custom_version` INT(11) UNSIGNED NOT NULL DEFAULT 0");
	}
}
