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
