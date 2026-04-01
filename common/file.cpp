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
#include "file.h"

#include "common/platform/platform.h"
#include "fmt/format.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <vector>


namespace fs = std::filesystem;


bool File::Exists(const std::string &name)
{
	struct stat sb{};
	if (stat(name.c_str(), &sb) == 0) {
		return true;
	}

	return false;
}

void File::Makedir(const std::string &directory_name)
{
	try {
		fs::create_directory(directory_name);
		fs::permissions(directory_name, fs::perms::owner_all);
	}
	catch (const fs::filesystem_error &ex) {
		std::cout << "Failed to create directory: " << directory_name << std::endl;
		std::cout << ex.what() << std::endl;
	}
}

std::string File::FindEqemuConfigPath()
{
	if (File::Exists(fs::path{File::GetCwd() + "/eqemu_config.json"}.string())) {
		return File::GetCwd();
	}
	else if (File::Exists(fs::path{File::GetCwd() + "/../eqemu_config.json"}.string())) {
		return canonical(fs::path{File::GetCwd() + "/../"}).string();
	}
	else if (File::Exists(fs::path{File::GetCwd() + "/login.json"}.string())) {
		return File::GetCwd();
	}
	else if (File::Exists(fs::path{File::GetCwd() + "/../login.json"}.string())) {
		return canonical(fs::path{File::GetCwd() + "/../"}).string();
	}

	return {};
}

std::string File::GetCwd()
{
	return fs::current_path().string();
}

FileContentsResult File::GetContents(const std::string &file_name)
{
	std::ifstream f(file_name, std::ios::in | std::ios::binary);
	if (!f) {
		return { .error = fmt::format("Couldn't open file [{}]", file_name) };
	}

	constexpr size_t CHUNK_SIZE = 4096;  // Read 4KB chunks
	std::string lines;
	std::vector<char> buffer(CHUNK_SIZE);

	while (f.read(buffer.data(), CHUNK_SIZE) || f.gcount() > 0) {
		lines.append(buffer.data(), f.gcount());
	}

	return FileContentsResult{
		.contents = lines,
		.error = {}
	};
}
