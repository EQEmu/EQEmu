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

#include <string>
#include <vector>

class PathManager {
public:
	void Init();

	static PathManager *Instance()
	{
		static PathManager instance;
		return &instance;
	}

	[[nodiscard]] const std::string &GetLogPath() const;
	[[nodiscard]] const std::string &GetLuaModsPath() const;
	[[nodiscard]] const std::string &GetLuaModulesPath() const;
	[[nodiscard]] const std::string &GetMapsPath() const;
	[[nodiscard]] const std::string &GetPatchPath() const;
	[[nodiscard]] const std::string &GetOpcodePath() const;
	[[nodiscard]] const std::string &GetServerPath() const;
	[[nodiscard]] const std::string &GetSharedMemoryPath() const;
	[[nodiscard]] std::vector<std::string> GetQuestPaths() const;
	[[nodiscard]] std::vector<std::string> GetPluginPaths() const;
	[[nodiscard]] std::vector<std::string> GetLuaModulePaths() const;

private:
	std::string              m_log_path;
	std::string              m_lua_mods_path;
	std::string              m_maps_path;
	std::string              m_patch_path;
	std::string              m_opcode_path;
	std::string              m_quests_path;
	std::vector<std::string> m_quests_paths;
	std::vector<std::string> m_plugin_paths;
	std::vector<std::string> m_lua_module_paths;


private:
	std::string              m_server_path;
	std::string              m_shared_memory_path;
};
