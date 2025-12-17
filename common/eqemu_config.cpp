/*	 EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "eqemu_config.h"

#include "common/eqemu_logsys.h"
#include "common/global_define.h"
#include "common/json/json.hpp"
#include "common/misc_functions.h"
#include "common/strings.h"

#include <filesystem>
#include <iostream>
#include <sstream>

std::string EQEmuConfig::ConfigFile = "eqemu_config.json";
EQEmuConfig *EQEmuConfig::_config = nullptr;

void EQEmuConfig::parse_config()
{

	ShortName    = _root["server"]["world"].get("shortname", "").asString();
	LongName     = _root["server"]["world"].get("longname", "").asString();
	WorldAddress = _root["server"]["world"].get("address", "").asString();
	LocalAddress = _root["server"]["world"].get("localaddress", "").asString();
	MaxClients   = Strings::ToInt(_root["server"]["world"].get("maxclients", "-1").asString());
	SharedKey    = _root["server"]["world"].get("key", "").asString();
	LoginCount   = 0;

	if (_root["server"]["world"]["loginserver"].isObject()) {
		LoginHost   = _root["server"]["world"]["loginserver"].get("host", "login.eqemulator.net").asString();
		LoginPort   = Strings::ToUnsignedInt(_root["server"]["world"]["loginserver"].get("port", "5998").asString());
		LoginLegacy = false;
		if (_root["server"]["world"]["loginserver"].get("legacy", "0").asString() == "1") { LoginLegacy = true; }
		LoginAccount  = _root["server"]["world"]["loginserver"].get("account", "").asString();
		LoginPassword = _root["server"]["world"]["loginserver"].get("password", "").asString();

		// at least today, this is wrong a majority of the time
		// remove this if eqemulator ever upgrades its loginserver
		if (LoginHost.find("login.eqemulator.net") != std::string::npos) {
			LoginLegacy = true;
		}
	}
	else {
		char str[32];
		loginlist.Clear();
		do {
			sprintf(str, "loginserver%i", ++LoginCount);
			if (!_root["server"]["world"][str].isObject()) {
				break;
			}

			auto loginconfig = new LoginConfig;
			loginconfig->LoginHost     = _root["server"]["world"][str].get("host", "login.eqemulator.net").asString();
			loginconfig->LoginPort     = Strings::ToUnsignedInt(_root["server"]["world"][str].get("port", "5998").asString());
			loginconfig->LoginAccount  = _root["server"]["world"][str].get("account", "").asString();
			loginconfig->LoginPassword = _root["server"]["world"][str].get("password", "").asString();

			loginconfig->LoginLegacy = false;
			if (_root["server"]["world"][str].get("legacy", "0").asString() == "1") { loginconfig->LoginLegacy = true; }

			// at least today, this is wrong a majority of the time
			// remove this if eqemulator ever upgrades its loginserver
			if (loginconfig->LoginHost.find("login.eqemulator.net") != std::string::npos) {
				loginconfig->LoginLegacy = true;
			}

			loginlist.Insert(loginconfig);
		} while (LoginCount < 100);
	}


	//<locked> from xml converts to json as locked: "", so i default to "false".
	//The only way to enable locked is by switching to true, meaning this value is always false until manually set true
	Locked = false;
	if (_root["server"]["world"].get("locked", "false").asString() == "true") { Locked = true; }

	auto_database_updates = false;
	if (_root["server"].get("auto_database_updates", "true").asString() == "true") {
		auto_database_updates = true;
	}

	WorldIP      = _root["server"]["world"]["tcp"].get("ip", "127.0.0.1").asString();
	WorldTCPPort = Strings::ToUnsignedInt(_root["server"]["world"]["tcp"].get("port", "9000").asString());

	TelnetIP      = _root["server"]["world"]["telnet"].get("ip", "127.0.0.1").asString();
	TelnetTCPPort = Strings::ToUnsignedInt(_root["server"]["world"]["telnet"].get("port", "9001").asString());
	TelnetEnabled = false;
	if (_root["server"]["world"]["telnet"].get("enabled", "false").asString() == "true") { TelnetEnabled = true; }

	WorldHTTPMimeFile = _root["server"]["world"]["http"].get("mimefile", "mime.types").asString();
	WorldHTTPPort     = Strings::ToUnsignedInt(_root["server"]["world"]["http"].get("port", "9080").asString());
	WorldHTTPEnabled  = false;

	if (_root["server"]["world"]["http"].get("enabled", "false").asString() == "true") {
		WorldHTTPEnabled = true;
	}

	if (_root["server"].get("disable_config_checks", "false").asString() == "true") {
		DisableConfigChecks = true;
	}



	CheckUcsConfigConversion();

	m_ucs_host = _root["server"]["ucs"].get("host", "eqchat.eqemulator.net").asString();
	m_ucs_port = Strings::ToUnsignedInt(_root["server"]["ucs"].get("port", "7778").asString());

	/**
	 * Database
	 */
	DatabaseUsername = _root["server"]["database"].get("username", "eq").asString();
	DatabasePassword = _root["server"]["database"].get("password", "eq").asString();
	DatabaseHost     = _root["server"]["database"].get("host", "localhost").asString();
	DatabasePort     = Strings::ToUnsignedInt(_root["server"]["database"].get("port", "3306").asString());
	DatabaseDB       = _root["server"]["database"].get("db", "eq").asString();

	/**
	 * Content Database
	 */
	ContentDbUsername = _root["server"]["content_database"].get("username", "").asString();
	ContentDbPassword = _root["server"]["content_database"].get("password", "").asString();
	ContentDbHost     = _root["server"]["content_database"].get("host", "").asString();
	ContentDbPort     = Strings::ToUnsignedInt(_root["server"]["content_database"].get("port", 0).asString());
	ContentDbName     = _root["server"]["content_database"].get("db", "").asString();

	/**
	 * QS
	 */
	QSDatabaseHost     = _root["server"]["qsdatabase"].get("host", "localhost").asString();
	QSDatabasePort     = Strings::ToUnsignedInt(_root["server"]["qsdatabase"].get("port", "3306").asString());
	QSDatabaseUsername = _root["server"]["qsdatabase"].get("username", "eq").asString();
	QSDatabasePassword = _root["server"]["qsdatabase"].get("password", "eq").asString();
	QSDatabaseDB       = _root["server"]["qsdatabase"].get("db", "eq").asString();
	QSHost             = _root["server"]["queryserver"].get("host", "localhost").asString();
	QSPort             = Strings::ToUnsignedInt(_root["server"]["queryserver"].get("port", "9500").asString());

	/**
	 * Zones
	 */
	DefaultStatus = Strings::ToUnsignedInt(_root["server"]["zones"].get("defaultstatus", 0).asString());
	ZonePortLow   = Strings::ToUnsignedInt(_root["server"]["zones"]["ports"].get("low", "7000").asString());
	ZonePortHigh  = Strings::ToUnsignedInt(_root["server"]["zones"]["ports"].get("high", "7999").asString());

	/**
	 * Files
	 */
	SpellsFile      = _root["server"]["files"].get("spells", "spells_us.txt").asString();
	OpCodesFile     = _root["server"]["files"].get("opcodes", "opcodes.conf").asString();
	MailOpCodesFile = _root["server"]["files"].get("mail_opcodes", "mail_opcodes.conf").asString();
	PluginPlFile    = _root["server"]["files"].get("plugin.pl", "plugin.pl").asString();

	/**
	 * Directories
	 */
	MapDir       = _root["server"]["directories"].get("maps", "Maps/").asString();
	QuestDir     = _root["server"]["directories"].get("quests", "quests/").asString();
	PluginDir    = _root["server"]["directories"].get("plugins", "plugins/").asString();
	LuaModuleDir = _root["server"]["directories"].get("lua_modules", "lua_modules/").asString();
	PatchDir     = _root["server"]["directories"].get("patches", "./").asString();
	OpcodeDir    = _root["server"]["directories"].get("opcodes", "./").asString();
	SharedMemDir = _root["server"]["directories"].get("shared_memory", "shared/").asString();
	LogDir       = _root["server"]["directories"].get("logs", "logs/").asString();

	auto load_paths = [&](const std::string& key, std::vector<std::string>& target) {
		const auto& paths = _root["server"]["directories"][key];
		if (paths.isArray()) {
			for (const auto& dir : paths) {
				if (dir.isString()) {
					target.push_back(dir.asString());
				}
			}
		}
	};

	load_paths("quest_paths", m_quest_directories);
	load_paths("plugin_paths", m_plugin_directories);
	load_paths("lua_module_paths", m_lua_module_directories);

	/**
	 * Logs
	 */
	LogPrefix = _root["server"]["launcher"].get("logprefix", "logs/zone-").asString();
	LogSuffix = _root["server"]["launcher"].get("logsuffix", ".log").asString();

	/**
	 * Launcher
	 */
	RestartWait      = Strings::ToInt(_root["server"]["launcher"]["timers"].get("restart", "10000").asString());
	TerminateWait    = Strings::ToInt(_root["server"]["launcher"]["timers"].get("reterminate", "10000").asString());
	InitialBootWait  = Strings::ToInt(_root["server"]["launcher"]["timers"].get("initial", "20000").asString());
	ZoneBootInterval = Strings::ToInt(_root["server"]["launcher"]["timers"].get("interval", "2000").asString());
#ifdef WIN32
	ZoneExe = _root["server"]["launcher"].get("exe", "zone.exe").asString();
#else
	ZoneExe = _root["server"]["launcher"].get("exe", "./zone").asString();
#endif

}
std::string EQEmuConfig::GetByName(const std::string &var_name) const
{
	if (var_name == "ShortName") {
		return (ShortName);
	}
	if (var_name == "LongName") {
		return (LongName);
	}
	if (var_name == "WorldAddress") {
		return (WorldAddress);
	}
	if (var_name == "LoginHost") {
		return (LoginHost);
	}
	if (var_name == "LoginAccount") {
		return (LoginAccount);
	}
	if (var_name == "LoginPassword") {
		return (LoginPassword);
	}
	if (var_name == "LoginPort") {
		return (itoa(LoginPort));
	}
	if (var_name == "LoginLegacy") {
		return (itoa(LoginLegacy ? 1 : 0));
	}
	if (var_name == "Locked") {
		return (Locked ? "true" : "false");
	}
	if (var_name == "WorldTCPPort") {
		return (itoa(WorldTCPPort));
	}
	if (var_name == "WorldIP") {
		return (WorldIP);
	}
	if (var_name == "TelnetTCPPort") {
		return (itoa(TelnetTCPPort));
	}
	if (var_name == "TelnetIP") {
		return (TelnetIP);
	}
	if (var_name == "TelnetEnabled") {
		return (TelnetEnabled ? "true" : "false");
	}
	if (var_name == "WorldHTTPPort") {
		return (itoa(WorldHTTPPort));
	}
	if (var_name == "WorldHTTPMimeFile") {
		return (WorldHTTPMimeFile);
	}
	if (var_name == "WorldHTTPEnabled") {
		return (WorldHTTPEnabled ? "true" : "false");
	}
	if (var_name == "ChatHost") {
		return (m_ucs_host);
	}
	if (var_name == "ChatPort") {
		return (itoa(m_ucs_port));
	}
	if (var_name == "MailHost") {
		return (m_ucs_host);
	}
	if (var_name == "MailPort") {
		return (itoa(m_ucs_port));
	}
	if (var_name == "DatabaseHost") {
		return (DatabaseHost);
	}
	if (var_name == "DatabaseUsername") {
		return (DatabaseUsername);
	}
	if (var_name == "DatabasePassword") {
		return (DatabasePassword);
	}
	if (var_name == "DatabaseDB") {
		return (DatabaseDB);
	}
	if (var_name == "DatabasePort") {
		return (itoa(DatabasePort));
	}
	if (var_name == "QSDatabaseHost") {
		return (QSDatabaseHost);
	}
	if (var_name == "QSDatabaseUsername") {
		return (QSDatabaseUsername);
	}
	if (var_name == "QSDatabasePassword") {
		return (QSDatabasePassword);
	}
	if (var_name == "QSDatabaseDB") {
		return (QSDatabaseDB);
	}
	if (var_name == "QSDatabasePort") {
		return (itoa(QSDatabasePort));
	}
	if (var_name == "SpellsFile") {
		return (SpellsFile);
	}
	if (var_name == "OpCodesFile") {
		return (OpCodesFile);
	}
	if (var_name == "MailOpCodesFile") {
		return (MailOpCodesFile);
	}
	if (var_name == "PluginPlFile") {
		return (PluginPlFile);
	}
	if (var_name == "MapDir") {
		return (MapDir);
	}
	if (var_name == "QuestDir") {
		return (QuestDir);
	}
	if (var_name == "PluginDir") {
		return (PluginDir);
	}
	if (var_name == "LuaModuleDir") {
		return (LuaModuleDir);
	}
	if (var_name == "PatchDir") {
		return (PatchDir);
	}
	if (var_name == "SharedMemDir") {
		return (SharedMemDir);
	}
	if (var_name == "LogDir") {
		return (LogDir);
	}
	if (var_name == "LogPrefix") {
		return (LogPrefix);
	}
	if (var_name == "LogSuffix") {
		return (LogSuffix);
	}
	if (var_name == "ZoneExe") {
		return (ZoneExe);
	}
	if (var_name == "ZonePortLow") {
		return (itoa(ZonePortLow));
	}
	if (var_name == "ZonePortHigh") {
		return (itoa(ZonePortHigh));
	}
	if (var_name == "DefaultStatus") {
		return (itoa(DefaultStatus));
	}
//	if(var_name == "DynamicCount")
//		return(itoa(DynamicCount));
	return ("");
}

void EQEmuConfig::Dump() const
{
	std::cout << "ShortName = " << ShortName << std::endl;
	std::cout << "LongName = " << LongName << std::endl;
	std::cout << "WorldAddress = " << WorldAddress << std::endl;
	std::cout << "LoginHost = " << LoginHost << std::endl;
	std::cout << "LoginAccount = " << LoginAccount << std::endl;
	std::cout << "LoginPassword = " << LoginPassword << std::endl;
	std::cout << "LoginPort = " << LoginPort << std::endl;
	std::cout << "LoginLegacy = " << LoginLegacy << std::endl;
	std::cout << "Locked = " << Locked << std::endl;
	std::cout << "WorldTCPPort = " << WorldTCPPort << std::endl;
	std::cout << "WorldIP = " << WorldIP << std::endl;
	std::cout << "TelnetTCPPort = " << TelnetTCPPort << std::endl;
	std::cout << "TelnetIP = " << TelnetIP << std::endl;
	std::cout << "TelnetEnabled = " << TelnetEnabled << std::endl;
	std::cout << "WorldHTTPPort = " << WorldHTTPPort << std::endl;
	std::cout << "WorldHTTPMimeFile = " << WorldHTTPMimeFile << std::endl;
	std::cout << "WorldHTTPEnabled = " << WorldHTTPEnabled << std::endl;
	std::cout << "UCSHost = " << m_ucs_host << std::endl;
	std::cout << "UCSPort = " << m_ucs_port << std::endl;
	std::cout << "DatabaseHost = " << DatabaseHost << std::endl;
	std::cout << "DatabaseUsername = " << DatabaseUsername << std::endl;
	std::cout << "DatabasePassword = " << DatabasePassword << std::endl;
	std::cout << "DatabaseDB = " << DatabaseDB << std::endl;
	std::cout << "DatabasePort = " << DatabasePort << std::endl;
	std::cout << "QSDatabaseHost = " << QSDatabaseHost << std::endl;
	std::cout << "QSDatabaseUsername = " << QSDatabaseUsername << std::endl;
	std::cout << "QSDatabasePassword = " << QSDatabasePassword << std::endl;
	std::cout << "QSDatabaseDB = " << QSDatabaseDB << std::endl;
	std::cout << "QSDatabasePort = " << QSDatabasePort << std::endl;
	std::cout << "SpellsFile = " << SpellsFile << std::endl;
	std::cout << "OpCodesFile = " << OpCodesFile << std::endl;
	std::cout << "MailOpcodesFile = " << MailOpCodesFile << std::endl;
	std::cout << "PluginPlFile = " << PluginPlFile << std::endl;
	std::cout << "MapDir = " << MapDir << std::endl;
	std::cout << "QuestDir = " << QuestDir << std::endl;
	std::cout << "PluginDir = " << PluginDir << std::endl;
	std::cout << "LuaModuleDir = " << LuaModuleDir << std::endl;
	std::cout << "PatchDir = " << PatchDir << std::endl;
	std::cout << "SharedMemDir = " << SharedMemDir << std::endl;
	std::cout << "LogDir = " << LogDir << std::endl;
	std::cout << "ZonePortLow = " << ZonePortLow << std::endl;
	std::cout << "ZonePortHigh = " << ZonePortHigh << std::endl;
	std::cout << "DefaultStatus = " << (int) DefaultStatus << std::endl;
//	std::cout << "DynamicCount = " << DynamicCount << std::endl;
}

const std::string &EQEmuConfig::GetUCSHost() const
{
	return m_ucs_host;
}

uint16 EQEmuConfig::GetUCSPort() const
{
	return m_ucs_port;
}

void EQEmuConfig::CheckUcsConfigConversion()
{
	std::string chat_host = _root["server"]["chatserver"].get("host", "").asString();
	uint32      chat_port = Strings::ToUnsignedInt(_root["server"]["chatserver"].get("port", "0").asString());
	std::string mail_host = _root["server"]["mailserver"].get("host", "").asString();
	uint32      mail_port = Strings::ToUnsignedInt(_root["server"]["mailserver"].get("port", "0").asString());
	std::string ucs_host  = _root["server"]["ucs"].get("host", "").asString();

	// automatic ucs legacy configuration migration
	// if old configuration values are set, let's backup the existing configuration
	// and migrate to to use the new fields and write the new config
	if ((!chat_host.empty() || !mail_host.empty()) && ucs_host.empty()) {
		LogInfo("Migrating old [eqemu_config] UCS configuration to new configuration");

		std::string config_file_path = std::filesystem::path{
			PathManager::Instance()->GetServerPath() + "/eqemu_config.json"
		}.string();

		std::string config_file_bak_path = std::filesystem::path{
			PathManager::Instance()->GetServerPath() + "/eqemu_config.ucs-migrate-json.bak"
		}.string();

		// copy eqemu_config.json to eqemu_config.json.bak
		std::ifstream src(config_file_path, std::ios::binary);
		std::ofstream dst(config_file_bak_path, std::ios::binary);
		dst << src.rdbuf();
		src.close();

		LogInfo("Old configuration backed up to [{}]", config_file_bak_path);

		// read eqemu_config.json, transplant new fields and write to eqemu_config.json
		Json::Value   root;
		Json::Reader  reader;
		std::ifstream file(config_file_path);
		if (!reader.parse(file, root)) {
			LogError("Failed to parse configuration file");
			return;
		}
		file.close();

		// get old fields
		std::string host = !chat_host.empty() ? chat_host : mail_host;
		if (host.empty()) {
			host = "eqchat.eqemulator.net";
		}
		std::string port = chat_port > 0 ? std::to_string(chat_port) : std::to_string(mail_port);
		if (port.empty()) {
			port = "7778";
		}

		// set new fields
		root["server"]["ucs"]["host"] = host;
		root["server"]["ucs"]["port"] = port;

		// unset old fields
		root["server"].removeMember("chatserver");
		root["server"].removeMember("mailserver");

		// get Json::Value raw string
		std::string config = root.toStyledString();

		// format using more modern json library
		nlohmann::json data = nlohmann::json::parse(config);

		// write to file
		std::ofstream o(config_file_path);
		o << std::setw(1) << data << std::endl;
		o.close();

		// write new config
		LogInfo("New configuration written to [{}]", config_file_path);
		LogInfo("Migration complete, please review the new configuration file");

		// reload config internally
		try {
			std::ifstream fconfig(config_file_path, std::ifstream::binary);
			fconfig >> _config->_root;
			_config->parse_config();
		}
		catch (std::exception &) {
			return;
		}
	}
}
