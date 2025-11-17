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
#ifndef __EQEmuConfig_H
#define __EQEmuConfig_H

#include "json/json.h"
#include "linked_list.h"
#include "path_manager.h"
#include <fstream>
#include <fmt/format.h>
#include <fmt/ranges.h>

struct LoginConfig {
	std::string LoginHost;
	std::string LoginAccount;
	std::string LoginPassword;
	uint16 LoginPort;
	bool LoginLegacy;
};

class EQEmuConfig
{
	public:
		virtual std::string GetByName(const std::string &var_name) const;

		// From <world/>
		std::string ShortName;
		std::string LongName;
		std::string WorldAddress;
		std::string LocalAddress;
		std::string LoginHost;
		std::string LoginAccount;
		std::string LoginPassword;
		uint16 LoginPort;
		bool LoginLegacy;
		uint32 LoginCount;
		LinkedList<LoginConfig*> loginlist;
		bool Locked;
		uint16 WorldTCPPort;
		std::string WorldIP;
		uint16 TelnetTCPPort;
		std::string TelnetIP;
		bool TelnetEnabled;
		int32 MaxClients;
		bool WorldHTTPEnabled;
		uint16 WorldHTTPPort;
		std::string WorldHTTPMimeFile;
		std::string SharedKey;
		bool DisableConfigChecks;

		// From <database/>
		std::string DatabaseHost;
		std::string DatabaseUsername;
		std::string DatabasePassword;
		std::string DatabaseDB;
		uint16 DatabasePort;

		// From <content_database/>
		std::string ContentDbHost;
		std::string ContentDbUsername;
		std::string ContentDbPassword;
		std::string ContentDbName;
		uint16      ContentDbPort;

		// From <qsdatabase> // QueryServ
		std::string QSDatabaseHost;
		std::string QSDatabaseUsername;
		std::string QSDatabasePassword;
		std::string QSDatabaseDB;
		uint16      QSDatabasePort;
		std::string QSHost;
		int         QSPort;

		// From <files/>
		std::string SpellsFile;
		std::string OpCodesFile;
		std::string MailOpCodesFile;
		std::string PluginPlFile;

		// From <directories/>
		std::string MapDir;
		std::string QuestDir;
		std::string PluginDir;
		std::string LuaModuleDir;
		std::string PatchDir;
		std::string OpcodeDir;
		std::string SharedMemDir;
		std::string LogDir;

		// From <launcher/>
		std::string LogPrefix;
		std::string LogSuffix;
		std::string ZoneExe;
		uint32 RestartWait;
		uint32 TerminateWait;
		uint32 InitialBootWait;
		uint32 ZoneBootInterval;

		// From <zones/>
		uint16 ZonePortLow;
		uint16 ZonePortHigh;
		uint8 DefaultStatus;

		bool auto_database_updates;

		const std::string &GetUCSHost() const;
		uint16 GetUCSPort() const;

		std::vector<std::string> GetQuestDirectories() const
		{
			return m_quest_directories;
		}

		std::vector<std::string> GetPluginsDirectories() const
		{
			return m_plugin_directories;
		}

		std::vector<std::string> GetLuaModuleDirectories() const
		{
			return m_lua_module_directories;
		}


//	uint16 DynamicCount;

//	map<string,uint16> StaticZones;

	protected:

		std::string m_ucs_host;
		uint16      m_ucs_port;

		static EQEmuConfig *_config;
		Json::Value _root;
		static std::string ConfigFile;

		std::vector<std::string> m_quest_directories = {};
		std::vector<std::string> m_plugin_directories = {};
		std::vector<std::string> m_lua_module_directories = {};

	protected:
		void parse_config();

		EQEmuConfig()
		{

		}

	public:
		virtual ~EQEmuConfig() {}

		// Produce a const singleton
		static const EQEmuConfig *get()
		{
			LoadConfig();
			return (_config);
		}

		// Load the config
		static bool LoadConfig(const std::string& path = "")
		{
			if (_config != nullptr) {
				return true;
			}
			_config = new EQEmuConfig;

			return parseFile(path);
		}

		// Load config file and parse data
		static bool parseFile(const std::string& file_path = ".")
		{
			if (_config == nullptr) {
				return LoadConfig(file_path);
			}

			std::string file = fmt::format(
				"{}/{}",
				(file_path.empty() ? PathManager::Instance()->GetServerPath() : file_path),
				EQEmuConfig::ConfigFile
			);

			std::ifstream fconfig(file, std::ifstream::binary);

			try {
				fconfig >> _config->_root;
				_config->parse_config();
			}
			catch (std::exception &) {
				return false;
			}
			return true;
		}

		void Dump() const;
		void CheckUcsConfigConversion();
};

#endif
