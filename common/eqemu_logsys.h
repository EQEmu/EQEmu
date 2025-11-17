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

#ifndef EQEMU_LOGSYS_H
#define EQEMU_LOGSYS_H

#include <iostream>
#include <fstream>
#include <cstdio>
#include <functional>
#include <algorithm>

#ifdef _WIN32
#ifdef utf16_to_utf8
#undef utf16_to_utf8
#endif
#endif

#include <fmt/format.h>
#include <fmt/ranges.h>
#include "types.h"

namespace Logs {
	enum DebugLevel {
		General = 1,    // 1 - Low-Level general debugging, useful info on single line
		Detail          // 2 - Use this for very chatty logging you want to leave in but don't want on by default
	};

	/**
	 * If you add to this, make sure you update LogCategoryName
	 *
	 * NOTE: Only add to the bottom of the enum because that is the type ID assignment
	 */
	enum LogCategory {
		None = 0,
		AA,
		AI,
		Aggro,
		Attack,
		DeprecatedCS, // deprecated
		Combat,
		Commands,
		Crash,
		Debug,
		Doors,
		Error,
		Guilds,
		Inventory,
		Launcher,
		Netcode,
		Normal, // deprecated
		Object,
		Pathing,
		QSServer, // deprecated
		Quests,
		Rules,
		Skills,
		Spawns,
		Spells,
		Status, // deprecated
		TCPConnection, // deprecated
		Tasks,
		Tradeskills,
		Trading,
		Tribute,
		UCSServer, // deprecated
		WebInterfaceServer, // deprecated
		WorldServer, // deprecated
		ZoneServer, // deprecated
		MySQLError,
		MySQLQuery,
		Mercenaries,
		QuestDebug,
		DeprecatedSC, // deprecated
		DeprecatedCSU, // deprecated
		DeprecatedSCD, // deprecated
		DeprecatedCSD, // deprecated
		Loginserver, // deprecated
		ClientLogin,
		HeadlessClient, // deprecated
		HPUpdate,
		FixZ,
		Food,
		Traps,
		NPCRoamBox,
		NPCScaling,
		MobAppearance,
		Info,
		Warning,
		Critical, // deprecated
		Emergency, // deprecated
		Alert, // deprecated
		Notice, // deprecated
		AIScanClose,
		AIYellForHelp,
		AICastBeneficialClose,
		AoeCast,
		EntityManagement,
		Flee,
		Aura,
		HotReload,
		Merchants,
		ZonePoints,
		Loot,
		Expeditions,
		DynamicZones,
		Scheduler,
		Cheat,
		ClientList,
		DiaWind,
		HTTP,
		Saylink,
		ChecksumVerification,
		CombatRecord,
		Hate,
		Discord,
		Faction,
		PacketServerClient,
		PacketClientServer,
		PacketServerToServer,
		Bugs,
		QuestErrors,
		PlayerEvents,
		DataBuckets,
		Zoning,
		EqTime,
		Corpses,
		XTargets,
		EvolveItem,
		PositionUpdate,
		KSM,
		BotSettings,
		BotSpellChecks,
		BotSpellTypeChecks,
		NpcHandin,
		ZoneState,
		NetClient,
		NetTCP,
		MaxCategoryID /* Don't Remove this */
	};

	/**
	 * If you add to this, make sure you update LogCategory
	 */
	static const char *LogCategoryName[LogCategory::MaxCategoryID] = {
		"",
		"AA",
		"AI",
		"Aggro",
		"Attack",
		"Deprecated",
		"Combat",
		"Commands",
		"Crash",
		"Debug",
		"Doors",
		"Error",
		"Guilds",
		"Inventory",
		"Launcher",
		"Netcode",
		"Normal (Deprecated)",
		"Object",
		"Pathing",
		"QS Server (Deprecated)",
		"Quests",
		"Rules",
		"Skills",
		"Spawns",
		"Spells",
		"Status (Deprecated)",
		"TCP Connection (Deprecated)",
		"Tasks",
		"Tradeskills",
		"Trading",
		"Tribute",
		"UCS Server (Deprecated)",
		"Web Interface (Deprecated)",
		"World Server (Deprecated)",
		"Zone Server (Deprecated)",
		"MySQL Error",
		"MySQL Query",
		"Mercenaries",
		"Quest Debug",
		"Legacy Packet Logging (Deprecated)",
		"Legacy Packet Logging (Deprecated)",
		"Legacy Packet Logging (Deprecated)",
		"Legacy Packet Logging (Deprecated)",
		"Login Server (Deprecated)",
		"Client Login",
		"Headless Client (Deprecated)",
		"HP Update",
		"FixZ",
		"Food",
		"Traps",
		"NPC Roam Box",
		"NPC Scaling",
		"Mob Appearance",
		"Info",
		"Warning",
		"Critical (Deprecated)",
		"Emergency (Deprecated)",
		"Alert (Deprecated)",
		"Notice (Deprecated)",
		"AI Scan Close",
		"AI Yell For Help",
		"AI CastBeneficial",
		"AOE Cast",
		"Entity Management",
		"Flee",
		"Aura",
		"HotReload",
		"Merchants",
		"ZonePoints",
		"Loot",
		"Expeditions",
		"DynamicZones",
		"Scheduler",
		"Cheat",
		"ClientList",
		"DialogueWindow",
		"HTTP",
		"Saylink",
		"Checksum Verification",
		"CombatRecord",
		"Hate",
		"Discord",
		"Faction",
		"Packet S->C",
		"Packet C->S",
		"Packet S->S",
		"Bugs",
		"QuestErrors",
		"PlayerEvents",
		"DataBuckets",
		"Zoning",
		"EqTime",
		"Corpses",
		"XTargets",
		"EvolveItem",
		"PositionUpdate",
		"KSM", // Kernel Samepage Merging
		"Bot Settings",
		"Bot Spell Checks",
		"Bot Spell Type Checks",
		"NpcHandin",
		"ZoneState",
		"Net Server <-> Client",
		"Net TCP"
	};
}

class Database;

constexpr uint16 MAX_DISCORD_WEBHOOK_ID = 300;

class EQEmuLogSys {
public:
	EQEmuLogSys();
	~EQEmuLogSys();

	/**
	 * Close File Logs wherever necessary, either at zone shutdown or entire process shutdown for everything else.
	 * This should be handled on deconstructor but to be safe we use it anyways.
	 */
	void CloseFileLogs();
	EQEmuLogSys *LoadLogSettingsDefaults();
	EQEmuLogSys *LoadLogDatabaseSettings(bool silent_load = false);

	static EQEmuLogSys *Instance()
	{
		static EQEmuLogSys instance;
		return &instance;
	}

	/**
	 * @param directory_name
	 */
	void MakeDirectory(const std::string &directory_name);

	/**
	 *	The one and only Logging function that uses a debug level as a parameter, as well as a log_category
	 *		log_category - defined in Logs::LogCategory::[]
	 *		log_category name resolution works by passing the enum int ID to Logs::LogCategoryName[category_id]
	 *
	 *	Example: EQEmuLogSys::Out(Logs::General, Logs::Guilds, "This guild has no leader present");
	 *		- This would pipe the same category and debug level to all output formats, but the internal memory reference of log_settings would
	 *			be checked against to see if that piped output is set to actually process it for the category and debug level
	*/
	void Out(
		Logs::DebugLevel debug_level,
		uint16 log_category,
		const char *file,
		const char *func,
		int line,
		const char *message,
		...
	);

	/**
	 * Used in file logs to prepend a timestamp entry for logs
	 * @param time_stamp
	 */
	void SetCurrentTimeStamp(char *time_stamp);

	/**
	 * @param log_name
	 */
	void StartFileLogs(const std::string &log_name = "");

	/**
     * LogSettings Struct
     *
     * This struct is the master reference for all settings for each category, and for each output
     *
     * log_to_file[category_id]    = [1-3] - Sets debug level for category to output to file
     * log_to_console[category_id] = [1-3] - Sets debug level for category to output to console
     * log_to_gmsay[category_id]   = [1-3] - Sets debug level for category to output to gmsay
     *
	*/
	struct LogSettings {
		uint8 log_to_file;
		uint8 log_to_console;
		uint8 log_to_gmsay;
		uint8 log_to_discord;
		int   discord_webhook_id;
		uint8 is_category_enabled; /* When any log output in a category > 0, set this to 1 as (Enabled) */
	};

	struct OriginationInfo {
		std::string zone_short_name;
		std::string zone_long_name;
		int         instance_id;
	};

	OriginationInfo origination_info{};

	/**
	 * Internally used memory reference for all log settings per category
	 * These are loaded via DB and have defaults loaded in LoadLogSettingsDefaults
	 * Database loaded via EQEmuLogSys::Instance()->SetDatabase(&database)->LoadLogDatabaseSettings();
	*/
	LogSettings log_settings[Logs::LogCategory::MaxCategoryID]{};

	// temporary bucket to re-load after silencing
	LogSettings pre_silence_settings[Logs::LogCategory::MaxCategoryID]{};

	struct LogEnabled {
		bool log_to_file_enabled;
		bool log_to_console_enabled;
		bool log_to_gmsay_enabled;
		bool log_to_discord_enabled;
		bool log_enabled;
	};

	LogEnabled GetLogsEnabled(const Logs::DebugLevel &debug_level, const uint16 &log_category);
	bool IsLogEnabled(const Logs::DebugLevel &debug_level, const uint16 &log_category);

	struct DiscordWebhooks {
		int         id;
		std::string webhook_name;
		std::string webhook_url;
	};

	const DiscordWebhooks *GetDiscordWebhooks() const;

	// gmsay
	uint16 GetGMSayColorFromCategory(uint16 log_category);

	EQEmuLogSys *SetGMSayHandler(const std::function<void(uint16 log_type, const char *func, const std::string &)>& f)
	{
		m_on_log_gmsay_hook = f;
		return this;
	}

	EQEmuLogSys *SetDiscordHandler(std::function<void(uint16 log_category, int webhook_id, const std::string &)> f)
	{
		m_on_log_discord_hook = f;
		return this;
	}

	// console
	void SetConsoleHandler(
		std::function<void(
			uint16 log_type,
			const std::string &
		)> f
	) { m_on_log_console_hook = f; }
	void SilenceConsoleLogging();
	void EnableConsoleLogging();

	// database
	EQEmuLogSys *SetDatabase(Database *db);

	[[nodiscard]] const std::string &GetLogPath() const;
	EQEmuLogSys * SetLogPath(const std::string &log_path);

	void DisableMySQLErrorLogs();
	void EnableMySQLErrorLogs();

private:

	// reference to database
	Database                                                                        *m_database;
	std::function<void(uint16 log_category, const char *func, const std::string &)> m_on_log_gmsay_hook;
	std::function<void(uint16 log_category, int webhook_id, const std::string &)>   m_on_log_discord_hook;
	std::function<void(uint16 log_category, const std::string &)>                   m_on_log_console_hook;
	DiscordWebhooks                                                                 m_discord_webhooks[MAX_DISCORD_WEBHOOK_ID]{};
	bool                                                                            m_file_logs_enabled = false;
	int                                                                             m_log_platform      = 0;
	std::string                                                                     m_platform_file_name;
	std::string                                                                     m_log_path;

	void ProcessConsoleMessage(
		uint16 log_category,
		const std::string &message,
		const char *file,
		const char *func,
		int line
	);
	void ProcessLogWrite(uint16 log_category, const std::string &message);
	void InjectTablesIfNotExist();
};

#include "eqemu_logsys_log_aliases.h"

/**
template<typename... Args>
void OutF(
	EQEmuLogSys &ls,
	Logs::DebugLevel debug_level,
	uint16 log_category,
	const char *file,
	const char *func,
	int line,
	const char *fmt,
	const Args &... args
)
{
	std::string log_str = fmt::format(fmt, args...);
	ls.Out(debug_level, log_category, file, func, line, log_str.c_str());
}
 **/

#define OutF(ls, debug_level, log_category, file, func, line, formatStr, ...) \
do { \
    ls->Out(debug_level, log_category, file, func, line, fmt::format(formatStr, ##__VA_ARGS__).c_str()); \
} while(0)

#endif
