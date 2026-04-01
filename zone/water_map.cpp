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
#include "water_map.h"

#include "common/eqemu_logsys.h"
#include "zone/water_map_v1.h"
#include "zone/water_map_v2.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>

/**
 * @param zone_name
 * @return
 */
WaterMap* WaterMap::LoadWaterMapfile(std::string zone_name) {
	std::transform(zone_name.begin(), zone_name.end(), zone_name.begin(), ::tolower);

	std::string file_path = fmt::format("{}/water/{}.wtr", PathManager::Instance()->GetMapsPath(), zone_name);
	LogDebug("Attempting to load water map with path [{}]", file_path.c_str());
	FILE *f = fopen(file_path.c_str(), "rb");
	if(f) {
		char magic[10];
		uint32 version;
		if(fread(magic, 10, 1, f) != 1) {
			LogDebug("Failed to load water map, error reading magic string in header");
			fclose(f);
			return nullptr;
		}

		if(strncmp(magic, "EQEMUWATER", 10)) {
			LogDebug("Failed to load water map, bad magic string in header");
			fclose(f);
			return nullptr;
		}

		if(fread(&version, sizeof(version), 1, f) != 1) {
			LogDebug("Failed to load water map, error reading version");
			fclose(f);
			return nullptr;
		}

		LogDebug("Attempting to V[{}] load water map [{}]", version, file_path.c_str());
		if(version == 1) {
			auto wm = new WaterMapV1();
			if(!wm->Load(f)) {
				delete wm;
				wm = nullptr;
			}

			LogInfo("Loaded Water Map V[{}] file [{}]", version, file_path.c_str());

			fclose(f);
			return wm;
		} else if(version == 2) {
			auto wm = new WaterMapV2();
			if(!wm->Load(f)) {
				delete wm;
				wm = nullptr;
			}

			LogInfo("Loaded Water Map V[{}] file [{}]", version, file_path.c_str());

			fclose(f);
			return wm;
		} else {
			LogDebug("Failed to load water map, unsupported version V[{}]", version);
			fclose(f);
			return nullptr;
		}
	}

	LogDebug("Failed to load water map, could not open file for reading [{}]", file_path.c_str());
	return nullptr;
}
