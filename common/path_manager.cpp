#include "path_manager.h"

#include "common/eqemu_config.h"
#include "common/eqemu_logsys.h"
#include "common/file.h"
#include "common/strings.h"

#include <filesystem>

namespace fs = std::filesystem;

void PathManager::Init()
{
	m_server_path = File::FindEqemuConfigPath();

	if (m_server_path.empty()) {
		LogInfo("Failed to load server path");
		return;
	}

	std::filesystem::current_path(m_server_path);

	if (!EQEmuConfig::LoadConfig()) {
		LogError("Failed to load eqemu config");
		return;
	}

	const auto c = EQEmuConfig::get();

	auto resolve_path = [&](const std::string& dir, const std::vector<std::string>& fallback_dirs = {}) -> std::string {
		// relative
		if (File::Exists((fs::path{m_server_path} / dir).string())) {
			return fs::relative(fs::path{m_server_path} / dir).lexically_normal().string();
		}

		// absolute
		if (File::Exists(fs::path{dir}.string())) {
			return fs::absolute(fs::path{dir}).string();
		}

		// fallback search options if specified
		for (const auto& fallback : fallback_dirs) {
			if (File::Exists((fs::path{m_server_path} / fallback).string())) {
				return fs::relative(fs::path{m_server_path} / fallback).lexically_normal().string();
			}
		}

		// if all else fails, just set it to the config value
		return dir;
	};

	auto load_many_paths_fallback = [&](const std::vector<std::string>& dirs, const std::string& fallback, std::vector<std::string>& target) {
		target.clear();
		if (!dirs.empty()) {
			for (const auto& path : dirs) {
				target.push_back(resolve_path(path));
			}
		} else {
			target.push_back(resolve_path(fallback));
		}
	};

	load_many_paths_fallback(c->GetQuestDirectories(), c->QuestDir, m_quests_paths);
	load_many_paths_fallback(c->GetPluginsDirectories(), c->PluginDir, m_plugin_paths);
	load_many_paths_fallback(c->GetLuaModuleDirectories(), c->LuaModuleDir, m_lua_module_paths);

	// resolve all paths
	m_maps_path          = resolve_path(c->MapDir, {"maps", "Maps"});
	m_lua_mods_path      = resolve_path("mods");
	m_patch_path         = resolve_path(c->PatchDir);
	m_opcode_path        = resolve_path(c->OpcodeDir);
	m_shared_memory_path = resolve_path(c->SharedMemDir);
	m_log_path           = resolve_path(c->LogDir, {"logs"});

	// Log all paths in a loop
	std::vector<std::pair<std::string, std::string>> paths = {
		{"server", m_server_path},
		{"logs", m_log_path},
		{"maps", m_maps_path},
		{"lua mods", m_lua_mods_path},
		{"patches", m_patch_path},
		{"opcode", m_opcode_path},
		{"shared_memory", m_shared_memory_path}
	};

	constexpr int name_width   = 15;
	constexpr int path_width   = 0;
	constexpr int break_length = 70;

	LogInfo("Loading server paths");
	LogInfo("{}", Strings::Repeat("-", break_length));
	for (const auto& [name, in_path] : paths) {
		if (!in_path.empty()) {
			LogInfo("{:>{}} > [{:<{}}]", name, name_width, in_path, path_width);
		}
	}

	auto log_paths = [&](const std::string& label, const std::vector<std::string>& paths) {
		if (!paths.empty()) {
			LogInfo("{:>{}} > [{:<{}}]", label, name_width - 1, Strings::Join(paths, ";"), path_width);
		}
	};

	log_paths("quests", m_quests_paths);
	log_paths("plugins", m_plugin_paths);
	log_paths("lua_modules", m_lua_module_paths);

	LogInfo("{}", Strings::Repeat("-", break_length));
}

const std::string &PathManager::GetServerPath() const
{
	return m_server_path;
}

const std::string &PathManager::GetMapsPath() const
{
	return m_maps_path;
}

const std::string &PathManager::GetSharedMemoryPath() const
{
	return m_shared_memory_path;
}

std::vector<std::string> PathManager::GetQuestPaths() const
{
	return m_quests_paths;
}

std::vector<std::string> PathManager::GetPluginPaths() const
{
	return m_plugin_paths;
}

std::vector<std::string> PathManager::GetLuaModulePaths() const
{
	return m_lua_module_paths;
}

const std::string &PathManager::GetLogPath() const
{
	return m_log_path;
}

const std::string &PathManager::GetPatchPath() const
{
	return m_patch_path;
}

const std::string &PathManager::GetOpcodePath() const
{
	return m_opcode_path;
}

const std::string &PathManager::GetLuaModsPath() const
{
	return m_lua_mods_path;
}
