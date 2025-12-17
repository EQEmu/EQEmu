/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2018 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
*/

#include "eqemu_logsys.h"

#include "common/file.h"
#include "common/path_manager.h"
#include "common/platform.h"
#include "common/repositories/discord_webhooks_repository.h"
#include "common/repositories/logsys_categories_repository.h"
#include "common/rulesys.h"
#include "common/strings.h"
#include "common/termcolor/rang.hpp"

#include <filesystem>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <time.h>

#ifdef _WINDOWS
#include <conio.h>
#include <direct.h>
#include <dos.h>
#include <iostream>
#include <process.h>
#include <windows.h>
#else
#include <sys/stat.h>
#include <thread>
#include <unistd.h>
#endif

std::ofstream process_log;

/**
 * EQEmuLogSys Constructor
 */
EQEmuLogSys::EQEmuLogSys()
{
	m_on_log_gmsay_hook   = [](uint16 log_type, const char *func, const std::string &) {};
	m_on_log_console_hook = [](uint16 log_type, const std::string &) {};
}

/**
 * EQEmuLogSys Deconstructor
 */
EQEmuLogSys::~EQEmuLogSys() = default;

EQEmuLogSys *EQEmuLogSys::LoadLogSettingsDefaults()
{
	/**
	 * Get Executable platform currently running this code (Zone/World/etc)
	 */
	m_log_platform = GetExecutablePlatformInt();

	for (int log_category_id = Logs::AA; log_category_id != Logs::MaxCategoryID; log_category_id++) {
		log_settings[log_category_id].log_to_console      = 0;
		log_settings[log_category_id].log_to_file         = 0;
		log_settings[log_category_id].log_to_gmsay        = 0;
		log_settings[log_category_id].log_to_discord      = 0;
		log_settings[log_category_id].is_category_enabled = 0;
	}

	m_file_logs_enabled = false;

	/**
	 * Set Defaults
	 */
	log_settings[Logs::Crash].log_to_console                = static_cast<uint8>(Logs::General);
	log_settings[Logs::Crash].log_to_file                   = static_cast<uint8>(Logs::General);
	log_settings[Logs::MySQLError].log_to_console           = static_cast<uint8>(Logs::General);
	log_settings[Logs::NPCScaling].log_to_gmsay             = static_cast<uint8>(Logs::General);
	log_settings[Logs::HotReload].log_to_gmsay              = static_cast<uint8>(Logs::General);
	log_settings[Logs::HotReload].log_to_console            = static_cast<uint8>(Logs::General);
	log_settings[Logs::Loot].log_to_gmsay                   = static_cast<uint8>(Logs::General);
	log_settings[Logs::Scheduler].log_to_console            = static_cast<uint8>(Logs::General);
	log_settings[Logs::Cheat].log_to_console                = static_cast<uint8>(Logs::General);
	log_settings[Logs::HTTP].log_to_console                 = static_cast<uint8>(Logs::General);
	log_settings[Logs::HTTP].log_to_gmsay                   = static_cast<uint8>(Logs::General);
	log_settings[Logs::ChecksumVerification].log_to_console = static_cast<uint8>(Logs::General);
	log_settings[Logs::ChecksumVerification].log_to_gmsay   = static_cast<uint8>(Logs::General);
	log_settings[Logs::CombatRecord].log_to_gmsay           = static_cast<uint8>(Logs::General);
	log_settings[Logs::Discord].log_to_console              = static_cast<uint8>(Logs::General);
	log_settings[Logs::QuestErrors].log_to_gmsay            = static_cast<uint8>(Logs::General);
	log_settings[Logs::QuestErrors].log_to_console          = static_cast<uint8>(Logs::General);
	log_settings[Logs::EqTime].log_to_console               = static_cast<uint8>(Logs::General);
	log_settings[Logs::EqTime].log_to_gmsay                 = static_cast<uint8>(Logs::General);
	log_settings[Logs::NpcHandin].log_to_console            = static_cast<uint8>(Logs::General);
	log_settings[Logs::NpcHandin].log_to_gmsay              = static_cast<uint8>(Logs::General);

	/**
	 * RFC 5424
	 */
	log_settings[Logs::Error].log_to_console   = static_cast<uint8>(Logs::General);
	log_settings[Logs::Warning].log_to_console = static_cast<uint8>(Logs::General);
	log_settings[Logs::Info].log_to_console    = static_cast<uint8>(Logs::General);

	/**
	 * Set Category enabled status on defaults
	 */
	for (int log_category_id = Logs::AA; log_category_id != Logs::MaxCategoryID; log_category_id++) {
		const bool log_to_console      = log_settings[log_category_id].log_to_console > 0;
		const bool log_to_file         = log_settings[log_category_id].log_to_file > 0;
		const bool log_to_gmsay        = log_settings[log_category_id].log_to_gmsay > 0;
		const bool log_to_discord      = log_settings[log_category_id].log_to_discord > 0;
		const bool is_category_enabled = log_to_console || log_to_file || log_to_gmsay || log_to_discord;
		if (is_category_enabled) {
			log_settings[log_category_id].is_category_enabled = 1;
		}
	}

	/**
	 * Declare process file names for log writing=
	 */
	if (EQEmuLogSys::m_log_platform == EQEmuExePlatform::ExePlatformWorld) {
		m_platform_file_name = "world";
	}
	else if (EQEmuLogSys::m_log_platform == EQEmuExePlatform::ExePlatformQueryServ) {
		m_platform_file_name = "query_server";
	}
	else if (EQEmuLogSys::m_log_platform == EQEmuExePlatform::ExePlatformZone) {
		m_platform_file_name = "zone";
	}
	else if (EQEmuLogSys::m_log_platform == EQEmuExePlatform::ExePlatformUCS) {
		m_platform_file_name = "ucs";
	}
	else if (EQEmuLogSys::m_log_platform == EQEmuExePlatform::ExePlatformLogin) {
		m_platform_file_name = "login";
	}
	else if (EQEmuLogSys::m_log_platform == EQEmuExePlatform::ExePlatformLaunch) {
		m_platform_file_name = "launcher";
	}
	else if (EQEmuLogSys::m_log_platform == EQEmuExePlatform::ExePlatformHC) {
		m_platform_file_name = "hc";
	}

	return this;
}

/**
 * @param debug_level
 * @param log_category
 * @param message
 */
void EQEmuLogSys::ProcessLogWrite(
	uint16 log_category,
	const std::string &message
)
{
	if (log_category == Logs::Crash) {
		char time_stamp[80];
		EQEmuLogSys::SetCurrentTimeStamp(time_stamp);
		std::ofstream crash_log;
		EQEmuLogSys::MakeDirectory("logs/crashes");
		crash_log.open(
			StringFormat("logs/crashes/crash_%s_%i.log", m_platform_file_name.c_str(), getpid()),
			std::ios_base::app | std::ios_base::out
		);
		crash_log << time_stamp << " " << message << "\n";
		crash_log.close();
	}

	if (process_log) {
		char time_stamp[80];
		EQEmuLogSys::SetCurrentTimeStamp(time_stamp);
		process_log << time_stamp << " " << message << std::endl;
	}
}

/**
 * @param log_category
 * @return
 */
uint16 EQEmuLogSys::GetGMSayColorFromCategory(uint16 log_category)
{
	switch (log_category) {
		case Logs::Crash:
		case Logs::Error:
		case Logs::MySQLError:
		case Logs::QuestErrors:
			return Chat::Red;
		case Logs::MySQLQuery:
		case Logs::Debug:
			return Chat::Lime;
		case Logs::Quests:
			return Chat::Group;
		case Logs::Commands:
		case Logs::Mercenaries:
			return Chat::Magenta;
		default:
			return Chat::Yellow;
	}
}


/**
 * @param debug_level
 * @param log_category
 * @param message
 */
void EQEmuLogSys::ProcessConsoleMessage(
	uint16 log_category,
	const std::string &message,
	const char *file,
	const char *func,
	int line
)
{
	bool is_error   = (
		log_category == Logs::LogCategory::Error ||
		log_category == Logs::LogCategory::MySQLError ||
		log_category == Logs::LogCategory::Crash ||
		log_category == Logs::LogCategory::QuestErrors
	);
	bool is_warning = (
		log_category == Logs::LogCategory::Warning
	);

	(!is_error ? std::cout : std::cerr)
		<< ""
		<< rang::fgB::black
		<< rang::style::bold
		<< fmt::format("{:>6}", GetPlatformName().substr(0, 6))
		<< rang::style::reset
		<< rang::fgB::gray
		<< " | "
		<< ((is_error || is_warning) ? rang::fgB::red : rang::fgB::gray)
		<< rang::style::bold
		<< fmt::format("{:^10}", fmt::format("{}", Logs::LogCategoryName[log_category]).substr(0, 10))
		<< rang::style::reset
		<< rang::fgB::gray
		<< " | "
		<< rang::fgB::gray
		<< rang::style::bold
		<< fmt::format("{}", func)
		<< rang::style::reset
		<< rang::fgB::gray
		<< " ";

	if (RuleB(Logging, PrintFileFunctionAndLine)) {
		(!is_error ? std::cout : std::cerr)
			<< ""
			<< rang::fgB::green
			<< rang::style::bold
			<< fmt::format("{:}", fmt::format("{}:{}:{}", std::filesystem::path(file).filename().string(), func, line))
			<< rang::style::reset
			<< " | ";
	}

	if (log_category == Logs::LogCategory::MySQLQuery) {
		auto s = Strings::Split(message, "--");
		if (s.size() > 1) {
			std::string query = Strings::Trim(s[0]);
			std::string meta  = Strings::Trim(s[1]);

			std::cout <<
					  rang::fgB::green
					  <<
					  query
					  <<
					  rang::style::reset;

			std::cout <<
					  rang::fgB::black
					  <<
					  " -- "
					  <<
					  meta
					  <<
					  rang::style::reset;
		}
	}
	else if (Strings::Contains(message, "[")) {
		for (auto &e: Strings::Split(message, " ")) {
			if (Strings::Contains(e, "[") && Strings::Contains(e, "]")) {
				e = Strings::Replace(e, "[", "");
				e = Strings::Replace(e, "]", "");

				bool is_upper = false;

				for (int i = 0; i < strlen(e.c_str()); i++) {
					if (isupper(e[i])) {
						is_upper = true;
					}
				}

				// color matching in []
				// ex: [<red>variable] would produce [variable] with red inside brackets
				std::map<std::string, rang::fgB> colors = {
					{"<black>",   rang::fgB::black},
					{"<green>",   rang::fgB::green},
					{"<yellow>",  rang::fgB::yellow},
					{"<blue>",    rang::fgB::blue},
					{"<magenta>", rang::fgB::magenta},
					{"<cyan>",    rang::fgB::cyan},
					{"<gray>",    rang::fgB::gray},
					{"<red>",     rang::fgB::red},
				};

				bool      match_color = false;
				for (auto &c: colors) {
					if (Strings::Contains(e, c.first)) {
						e = Strings::Replace(e, c.first, "");
						(!is_error ? std::cout : std::cerr)
							<< rang::fgB::gray
							<< "["
							<< rang::style::bold
							<< c.second
							<< e
							<< rang::style::reset
							<< rang::fgB::gray
							<< "] ";
						match_color = true;
					}
				}

				// string match to colors
				std::map<std::string, rang::fgB> matches = {
					{"missing", rang::fgB::red},
					{"error",   rang::fgB::red},
					{"ok",      rang::fgB::green},
				};

				for (auto &c: matches) {
					if (Strings::Contains(e, c.first)) {
						(!is_error ? std::cout : std::cerr)
							<< rang::fgB::gray
							<< "["
							<< rang::style::bold
							<< c.second
							<< e
							<< rang::style::reset
							<< rang::fgB::gray
							<< "] ";
						match_color = true;
					}
				}

				// if we don't match a color in either the string matching or
				// the color tag matching, we default to yellow inside brackets
				// if uppercase, does not get colored
				if (!match_color) {
					if (!is_upper) {
						(!is_error ? std::cout : std::cerr)
							<< rang::fgB::gray
							<< "["
							<< rang::style::bold
							<< rang::fgB::yellow
							<< e
							<< rang::style::reset
							<< rang::fgB::gray
							<< "] ";
					}
					else {
						(!is_error ? std::cout : std::cerr) << rang::fgB::gray << "[" << e << "] ";
					}
				}
			}
			else {
				(!is_error ? std::cout : std::cerr)
					<< (is_error ? rang::fgB::red : rang::fgB::gray)
					<< e
					<< " ";
			}
		}
	}
	else {
		(!is_error ? std::cout : std::cerr)
			<< (is_error ? rang::fgB::red : rang::fgB::gray)
			<< message
			<< " ";
	}

	if (!origination_info.zone_short_name.empty()) {
		(!is_error ? std::cout : std::cerr)
			<<
			rang::fgB::black
			<<
			"-- "
			<<
			fmt::format(
				"[{}] ({}) inst_id [{}]",
				origination_info.zone_short_name,
				origination_info.zone_long_name,
				origination_info.instance_id
			);
	}

	(!is_error ? std::cout : std::cerr) << rang::style::reset << std::endl;

	m_on_log_console_hook(log_category, message);
}

/**
 * @param str
 * @return
 */
constexpr const char *str_end(const char *str)
{
	return *str ? str_end(str + 1) : str;
}

/**
 * Core logging function
 *
 * @param debug_level
 * @param log_category
 * @param message
 * @param ...
 */
void EQEmuLogSys::Out(
	Logs::DebugLevel debug_level,
	uint16 log_category,
	const char *file,
	const char *func,
	int line,
	const char *message,
	...
)
{
	auto l = GetLogsEnabled(debug_level, log_category);

	// bail out if nothing to log
	if (!l.log_enabled) {
		return;
	}

	std::string prefix;
	if (RuleB(Logging, PrintFileFunctionAndLine)) {
		prefix = fmt::format("[{0}::{1}:{2}] ", std::filesystem::path(file).filename().string(), func, line);
	}

	// remove this when we remove all legacy logs
	bool ignore_log_legacy_format = (
		log_category == Logs::Netcode ||
		log_category == Logs::MySQLQuery ||
		log_category == Logs::MySQLError ||
		log_category == Logs::PacketServerClient ||
		log_category == Logs::PacketClientServer ||
		log_category == Logs::PacketServerToServer
	);

	// remove this when we remove all legacy logs
	std::string output_message = message;
	if (!ignore_log_legacy_format) {
		va_list args;
		va_start(args, message);
		output_message = vStringFormat(message, args);
		va_end(args);
	}

	if (l.log_to_console_enabled) {
		EQEmuLogSys::ProcessConsoleMessage(
			log_category,
			output_message,
			file,
			func,
			line
		);
	}
	if (l.log_to_gmsay_enabled) {
		m_on_log_gmsay_hook(log_category, func, output_message);
	}
	if (l.log_to_file_enabled) {
		EQEmuLogSys::ProcessLogWrite(
			log_category,
			fmt::format("[{}] [{}] {}", GetPlatformName(), Logs::LogCategoryName[log_category], prefix + output_message)
		);
	}
	if (l.log_to_discord_enabled && m_on_log_discord_hook) {
		m_on_log_discord_hook(log_category, log_settings[log_category].discord_webhook_id, output_message);
	}
}

/**
 * @param time_stamp
 */
void EQEmuLogSys::SetCurrentTimeStamp(char *time_stamp)
{
	time_t    raw_time;
	struct tm *time_info;
	time(&raw_time);
	time_info = localtime(&raw_time);
	strftime(time_stamp, 80, "[%m-%d-%Y %H:%M:%S]", time_info);
}

/**
 * @param directory_name
 */
void EQEmuLogSys::MakeDirectory(const std::string &directory_name)
{
#ifdef _WINDOWS
	struct _stat st;
	if (_stat(directory_name.c_str(), &st) == 0) // exists
		return;
	_mkdir(directory_name.c_str());
#else
	struct stat st{};
	if (stat(directory_name.c_str(), &st) == 0) { // exists
		return;
	}
	mkdir(directory_name.c_str(), 0755);
#endif
}

void EQEmuLogSys::CloseFileLogs()
{
	if (process_log.is_open()) {
		process_log.close();
	}
}

/**
 * @param log_name
 */
void EQEmuLogSys::StartFileLogs(const std::string &log_name)
{
	EQEmuLogSys::CloseFileLogs();

	if (!File::Exists(PathManager::Instance()->GetLogPath())) {
		LogInfo("Logs directory not found, creating [{}]", PathManager::Instance()->GetLogPath());
		File::Makedir(PathManager::Instance()->GetLogPath());
	}

	/**
	 * When loading settings, we must have been given a reason in category based logging to output to a file in order to even create or open one...
	 */
	if (!m_file_logs_enabled) {
		return;
	}

	/**
	 * Zone
	 */
	if (EQEmuLogSys::m_log_platform == EQEmuExePlatform::ExePlatformZone) {
		if (!log_name.empty()) {
			m_platform_file_name = log_name;
		}

		if (m_platform_file_name.empty()) {
			return;
		}

		LogInfo("Starting File Log [{}/zone/{}_{}.log]", GetLogPath(), m_platform_file_name.c_str(), getpid());

		// Make directory if not exists
		EQEmuLogSys::MakeDirectory(fmt::format("{}/zone", GetLogPath()));

		// Open file pointer
		process_log.open(
			fmt::format("{}/zone/{}_{}.log", GetLogPath(), m_platform_file_name, getpid()),
			std::ios_base::app | std::ios_base::out
		);
	}
	else {

		// All other processes
		if (m_platform_file_name.empty()) {
			return;
		}

		LogInfo("Starting File Log [{}/{}_{}.log]", GetLogPath(), m_platform_file_name.c_str(), getpid());

		// Open file pointer
		process_log.open(
			fmt::format("{}/{}_{}.log", GetLogPath(), m_platform_file_name.c_str(), getpid()),
			std::ios_base::app | std::ios_base::out
		);
	}
}

/**
 * Silence console logging
 */
void EQEmuLogSys::SilenceConsoleLogging()
{
	std::copy(std::begin(log_settings), std::end(log_settings), std::begin(pre_silence_settings));

	for (int log_index = Logs::AA; log_index != Logs::MaxCategoryID; log_index++) {
		log_settings[log_index].log_to_console      = 0;
		log_settings[log_index].is_category_enabled = 0;
	}

	log_settings[Logs::MySQLError].log_to_console = static_cast<uint8>(Logs::MySQLError);
	log_settings[Logs::Error].log_to_console = static_cast<uint8>(Logs::Error);
	log_settings[Logs::Crash].log_to_console = static_cast<uint8>(Logs::General);
}

/**
 * Enables console logging
 */
void EQEmuLogSys::EnableConsoleLogging()
{
	std::copy(std::begin(pre_silence_settings), std::end(pre_silence_settings), std::begin(log_settings));
}

EQEmuLogSys *EQEmuLogSys::LoadLogDatabaseSettings(bool silent_load)
{
	InjectTablesIfNotExist();

	auto categories = LogsysCategoriesRepository::GetWhere(
		*m_database,
		"TRUE ORDER BY log_category_id"
	);

	// keep track of categories
	std::vector<int> db_categories{};
	db_categories.reserve(categories.size());

	// loop through database categories
	for (auto &c: categories) {
		if (c.log_category_id <= Logs::None || c.log_category_id >= Logs::MaxCategoryID) {
			continue;
		}

		log_settings[c.log_category_id].log_to_console     = static_cast<uint8>(c.log_to_console);
		log_settings[c.log_category_id].log_to_file        = static_cast<uint8>(c.log_to_file);
		log_settings[c.log_category_id].log_to_gmsay       = static_cast<uint8>(c.log_to_gmsay);
		log_settings[c.log_category_id].log_to_discord     = static_cast<uint8>(c.log_to_discord);
		log_settings[c.log_category_id].discord_webhook_id = c.discord_webhook_id;

		// Determine if any output method is enabled for the category
		// and set it to 1 so it can used to check if category is enabled
		const bool log_to_console      = log_settings[c.log_category_id].log_to_console > 0;
		const bool log_to_file         = log_settings[c.log_category_id].log_to_file > 0;
		const bool log_to_gmsay        = log_settings[c.log_category_id].log_to_gmsay > 0;
		const bool log_to_discord      = log_settings[c.log_category_id].log_to_discord > 0 &&
										 log_settings[c.log_category_id].discord_webhook_id > 0;
		const bool is_category_enabled = log_to_console || log_to_file || log_to_gmsay || log_to_discord;

		if (is_category_enabled) {
			log_settings[c.log_category_id].is_category_enabled = 1;
		}

		// This determines whether or not the process needs to actually file log anything.
		// If we go through this whole loop and nothing is set to any debug level, there
		// is no point to create a file or keep anything open
		if (log_settings[c.log_category_id].log_to_file > 0) {
			m_file_logs_enabled = true;
		}

		db_categories.emplace_back(c.log_category_id);
	}

	// Auto inject categories that don't exist in the database...

	std::vector<LogsysCategoriesRepository::LogsysCategories> db_categories_to_add{};

	for (int i = Logs::AA; i != Logs::MaxCategoryID; i++) {

		bool is_missing_in_database = std::find(db_categories.begin(), db_categories.end(), i) == db_categories.end();
		bool is_deprecated_category = Strings::Contains(fmt::format("{}", Logs::LogCategoryName[i]), "Deprecated");
		if (!is_missing_in_database && is_deprecated_category) {
			LogInfo(
				"Logging category [{}] ({}) is now deprecated, deleting from database",
				Logs::LogCategoryName[i],
				i
			);
			LogsysCategoriesRepository::DeleteOne(*m_database, i);
		}

		if (is_missing_in_database && !is_deprecated_category) {
			LogInfo("Automatically adding new log category [{}] ({})", Logs::LogCategoryName[i], i);

			auto e = LogsysCategoriesRepository::NewEntity();
			e.log_category_id          = i;
			e.log_category_description = Strings::Escape(Logs::LogCategoryName[i]);
			e.log_to_console           = log_settings[i].log_to_console;
			e.log_to_gmsay             = log_settings[i].log_to_gmsay;
			e.log_to_file              = log_settings[i].log_to_file;
			e.log_to_discord           = log_settings[i].log_to_discord;
			db_categories_to_add.emplace_back(e);
		}

		// look to see if the category name is different in the database
		auto it = std::find_if(
			categories.begin(),
			categories.end(),
			[i](const auto &c) { return c.log_category_id == i; }
		);
		if (it != categories.end()) {
			if (it->log_category_description != Logs::LogCategoryName[i]) {
				LogInfo(
					"Updating log category [{}] ({}) to new name [{}]",
					it->log_category_description,
					i,
					Logs::LogCategoryName[i]
				);
				it->log_category_description = Logs::LogCategoryName[i];
				LogsysCategoriesRepository::ReplaceOne(*m_database, *it);
			}
		}
	}

	if (!db_categories_to_add.empty()) {
		LogsysCategoriesRepository::ReplaceMany(*m_database, db_categories_to_add);
		LoadLogDatabaseSettings();
		return this;
	}

	if (silent_load) {
		SilenceConsoleLogging();
	}

	LogInfo("Loaded [{}] log categories", categories.size());

	auto webhooks = DiscordWebhooksRepository::GetWhere(*m_database, fmt::format("id < {}", MAX_DISCORD_WEBHOOK_ID));
	if (!webhooks.empty()) {
		for (auto &w: webhooks) {
			m_discord_webhooks[w.id] = {w.id, w.webhook_name, w.webhook_url};
		}
		LogInfo("Loaded [{}] Discord webhooks", webhooks.size());
	}

	// force override this setting
	log_settings[Logs::Crash].log_to_console = static_cast<uint8>(Logs::General);
	log_settings[Logs::Crash].log_to_gmsay   = static_cast<uint8>(Logs::General);
	log_settings[Logs::Crash].log_to_file    = static_cast<uint8>(Logs::General);
	log_settings[Logs::Info].log_to_file     = static_cast<uint8>(Logs::General);
	log_settings[Logs::Info].log_to_console  = static_cast<uint8>(Logs::General);

	if (silent_load) {
		SilenceConsoleLogging();
	}

	return this;
}

EQEmuLogSys *EQEmuLogSys::SetDatabase(Database *db)
{
	m_database = db;

	return this;
}

void EQEmuLogSys::InjectTablesIfNotExist()
{
	// do not run injections for zone as its unnecessary hits every time a zone boots
	// other processes less frequently ran can pick up injection
	if (m_log_platform == EQEmuExePlatform::ExePlatformZone) {
		return;
	}

	// inject discord_webhooks
	if (!m_database->DoesTableExist("discord_webhooks")) {
		LogInfo("Creating table [discord_webhooks]");
		m_database->QueryDatabase(
			SQL(
				CREATE TABLE discord_webhooks
				(
					id INT auto_increment primary key NULL,
				webhook_name varchar(100) NULL,
				webhook_url varchar(255) NULL,
				created_at DATETIME NULL,
				deleted_at DATETIME NULL
			) ENGINE=InnoDB
				DEFAULT CHARSET=utf8mb4
				COLLATE=utf8mb4_general_ci;
			)
		);
	}

	// inject logsys_categories
	if (!m_database->DoesTableExist("logsys_categories")) {
		LogInfo("Creating table [logsys_categories]");
		m_database->QueryDatabase(
			SQL(
				CREATE TABLE `logsys_categories` (
				`log_category_id` int(11) NOT NULL,
				`log_category_description` varchar(150) DEFAULT NULL,
				`log_to_console` smallint(11) DEFAULT 0,
				`log_to_file` smallint(11) DEFAULT 0,
				`log_to_gmsay` smallint(11) DEFAULT 0,
				`log_to_discord` smallint(11) DEFAULT 0,
				`discord_webhook_id` int(11) DEFAULT 0,
				PRIMARY KEY (`log_category_id`)
			) ENGINE=InnoDB DEFAULT CHARSET=latin1
			)
		);
	}
}

const EQEmuLogSys::DiscordWebhooks *EQEmuLogSys::GetDiscordWebhooks() const
{
	return m_discord_webhooks;
}

EQEmuLogSys::LogEnabled EQEmuLogSys::GetLogsEnabled(const Logs::DebugLevel &debug_level, const uint16 &log_category)
{
	auto e = LogEnabled{};

	e.log_to_console_enabled = log_settings[log_category].log_to_console > 0 &&
							   log_settings[log_category].log_to_console >= debug_level;
	e.log_to_file_enabled    = log_settings[log_category].log_to_file > 0 &&
							   log_settings[log_category].log_to_file >= debug_level;
	e.log_to_gmsay_enabled   = log_settings[log_category].log_to_gmsay > 0 &&
							   log_settings[log_category].log_to_gmsay >= debug_level &&
							   log_category != Logs::LogCategory::Netcode &&
							   (EQEmuLogSys::m_log_platform == EQEmuExePlatform::ExePlatformZone ||
								EQEmuLogSys::m_log_platform == EQEmuExePlatform::ExePlatformWorld);
	e.log_to_discord_enabled = EQEmuLogSys::m_log_platform == EQEmuExePlatform::ExePlatformZone &&
							   log_settings[log_category].log_to_discord > 0 &&
							   log_settings[log_category].log_to_discord >= debug_level &&
							   log_settings[log_category].discord_webhook_id > 0 &&
							   log_settings[log_category].discord_webhook_id < MAX_DISCORD_WEBHOOK_ID;
	e.log_enabled =
		e.log_to_console_enabled || e.log_to_file_enabled || e.log_to_gmsay_enabled || e.log_to_discord_enabled;

	return e;
}

bool EQEmuLogSys::IsLogEnabled(const Logs::DebugLevel &debug_level, const uint16 &log_category)
{
	return GetLogsEnabled(debug_level, log_category).log_enabled;
}

const std::string &EQEmuLogSys::GetLogPath() const
{
	return m_log_path;
}

EQEmuLogSys *EQEmuLogSys::SetLogPath(const std::string &log_path)
{
	EQEmuLogSys::m_log_path = log_path;

	return this;
}

void EQEmuLogSys::DisableMySQLErrorLogs()
{
	log_settings[Logs::MySQLError].log_to_file    = 0;
	log_settings[Logs::MySQLError].log_to_console = 0;
	log_settings[Logs::MySQLError].log_to_gmsay   = 0;
}

void EQEmuLogSys::EnableMySQLErrorLogs()
{
	log_settings[Logs::MySQLError].log_to_file    = 1;
	log_settings[Logs::MySQLError].log_to_console = 1;
	log_settings[Logs::MySQLError].log_to_gmsay   = 1;
}
