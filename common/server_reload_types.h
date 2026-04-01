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

#include "common/types.h"

#include <cstdint>
#include <string>
#include <vector>

namespace ServerReload {
	enum Type {
		ReloadTypeNone = 0,
		AAData,
		AlternateCurrencies,
		BaseData,
		BlockedSpells,
		Commands,
		ContentFlags,
		DataBucketsCache,
		Doors,
		DzTemplates,
		Factions,
		GroundSpawns,
		LevelEXPMods,
		Logs,
		Loot,
		Maps,
		Merchants,
		NPCEmotes,
		NPCSpells,
		Objects,
		Opcodes,
		PerlExportSettings,
		Quests,
		QuestsTimerReset,
		Rules,
		SkillCaps,
		StaticZoneData,
		Tasks,
		Titles,
		Traps,
		Variables,
		VeteranRewards,
		WorldRepop,
		WorldWithRespawn,
		ZoneData,
		ZonePoints,
		Max
	};

	static const char *Name[ServerReload::Max] = {
		"None",
		"AA Data",
		"Alternate Currencies",
		"Base Data",
		"Blocked Spells",
		"Commands",
		"Content Flags",
		"Data Buckets Cache",
		"Doors",
		"DZ Templates",
		"Factions",
		"Ground Spawns",
		"Level EXP Mods",
		"Logs",
		"Loot",
		"Maps",
		"Merchants",
		"NPC Emotes",
		"NPC Spells",
		"Objects",
		"Opcodes",
		"Perl Event Export Settings",
		"Quest",
		"Quests With Timer (Resets timer events)",
		"Rules",
		"Skill Caps",
		"Static Zone Data",
		"Tasks",
		"Titles",
		"Traps",
		"Variables",
		"Veteran Rewards",
		"World Repop",
		"World Repop Timers (Clear Respawn Timers)",
		"Zone Data",
		"Zone Points"
	};

	inline std::string GetName(int reload_type)
	{
		if (reload_type < 0 || reload_type >= ServerReload::Type::Max) {
			return "Unknown";
		}

		return ServerReload::Name[reload_type];
	}

	// Get a clean name without spaces or special characters
	inline std::string GetNameClean(int reload_type)
	{
		if (reload_type < 0 || reload_type >= ServerReload::Type::Max) {
			return "Unknown";
		}

		// get the name before parentheses
		std::string name = ServerReload::Name[reload_type];
		size_t pos = name.find('(');
		if (pos != std::string::npos) {
			name = name.substr(0, pos);
		}

		// Trim leading spaces
		size_t start = name.find_first_not_of(' ');
		if (start == std::string::npos) {
			return ""; // If all spaces, return empty string
		}

		// Trim trailing spaces
		size_t end = name.find_last_not_of(' ');

		// Extract trimmed substring
		return name.substr(start, end - start + 1);

		return name;
	}

	inline std::vector<ServerReload::Type> GetTypes()
	{
		std::vector<ServerReload::Type> types;
		types.reserve(ServerReload::Type::Max);
		for (int i = 1; i < ServerReload::Type::Max; i++) {
			types.push_back(static_cast<ServerReload::Type>(i));
		}
		return types;
	}

	struct Request {
		int      type                 = 0;
		bool     requires_zone_booted = false;
		int64    reload_at_unix       = 0;
		int32    opt_param            = 0;
		uint32_t zone_server_id       = 0;
	};
}
