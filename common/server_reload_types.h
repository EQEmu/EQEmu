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

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace ServerReload {
	// Reload IDs are sent between world and zone, so existing numeric values
	// remain stable even though the declarations and display list are sorted.
	enum Type {
		ReloadTypeNone = 0,
		AAData = 1,
		Achievements = 36,
		AlternateCurrencies = 2,
		BaseData = 3,
		BlockedSpells = 4,
		Commands = 5,
		ContentFlags = 6,
		DataBucketsCache = 7,
		Doors = 8,
		DzTemplates = 9,
		Factions = 10,
		GroundSpawns = 11,
		LevelEXPMods = 12,
		Logs = 13,
		Loot = 14,
		Maps = 15,
		Merchants = 16,
		NPCEmotes = 17,
		NPCSpells = 18,
		Objects = 19,
		Opcodes = 20,
		PerlExportSettings = 21,
		Quests = 22,
		QuestsTimerReset = 23,
		Rules = 24,
		SkillCaps = 25,
		StaticZoneData = 26,
		Tasks = 27,
		Titles = 28,
		Traps = 29,
		Variables = 30,
		VeteranRewards = 31,
		WorldRepop = 32,
		WorldWithRespawn = 33,
		ZoneData = 34,
		ZonePoints = 35,
		Max = 37
	};

	struct TypeName {
		Type type;
		const char *name;
	};

	inline constexpr std::array<TypeName, Max - 1> Types{{
		{AAData, "AA Data"},
		{Achievements, "Achievements"},
		{AlternateCurrencies, "Alternate Currencies"},
		{BaseData, "Base Data"},
		{BlockedSpells, "Blocked Spells"},
		{Commands, "Commands"},
		{ContentFlags, "Content Flags"},
		{DataBucketsCache, "Data Buckets Cache"},
		{Doors, "Doors"},
		{DzTemplates, "DZ Templates"},
		{Factions, "Factions"},
		{GroundSpawns, "Ground Spawns"},
		{LevelEXPMods, "Level EXP Mods"},
		{Logs, "Logs"},
		{Loot, "Loot"},
		{Maps, "Maps"},
		{Merchants, "Merchants"},
		{NPCEmotes, "NPC Emotes"},
		{NPCSpells, "NPC Spells"},
		{Objects, "Objects"},
		{Opcodes, "Opcodes"},
		{PerlExportSettings, "Perl Event Export Settings"},
		{Quests, "Quest"},
		{QuestsTimerReset, "Quests With Timer (Resets timer events)"},
		{Rules, "Rules"},
		{SkillCaps, "Skill Caps"},
		{StaticZoneData, "Static Zone Data"},
		{Tasks, "Tasks"},
		{Titles, "Titles"},
		{Traps, "Traps"},
		{Variables, "Variables"},
		{VeteranRewards, "Veteran Rewards"},
		{WorldRepop, "World Repop"},
		{WorldWithRespawn, "World Repop Timers (Clear Respawn Timers)"},
		{ZoneData, "Zone Data"},
		{ZonePoints, "Zone Points"}
	}};

	inline std::string GetName(int reload_type)
	{
		if (reload_type == ReloadTypeNone) {
			return "None";
		}

		for (const auto &entry : Types) {
			if (entry.type == reload_type) {
				return entry.name;
			}
		}

		return "Unknown";
	}

	// Get a clean name without spaces or special characters
	inline std::string GetNameClean(int reload_type)
	{
		if (reload_type < 0 || reload_type >= ServerReload::Type::Max) {
			return "Unknown";
		}

		// get the name before parentheses
		std::string name = GetName(reload_type);
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
		types.reserve(Types.size());
		for (const auto &entry : Types) {
			types.push_back(entry.type);
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
