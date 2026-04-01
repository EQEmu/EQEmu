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

#include <list>
#include <map>
#include <memory>

class SpawnEntry {
public:
	SpawnEntry(uint32 in_NPCType, int in_chance, uint16 in_filter, uint8 in_npc_spawn_limit, uint8 in_min_time, uint8 in_max_time);
	~SpawnEntry() {}
	uint32 NPCType;
	int    chance;
	uint8 min_time;
	uint8 max_time;
	uint16 condition_value_filter;

	//this is a cached value from npc_types, for speed
	uint8 npc_spawn_limit; //max # of this entry which can be spawned in this zone
};

class SpawnGroup {
public:
	SpawnGroup(
		uint32 in_id,
		char *name,
		int in_group_spawn_limit,
		float dist,
		float maxx,
		float minx,
		float maxy,
		float miny,
		int delay_in,
		int despawn_in,
		uint32 despawn_timer_in,
		int min_delay_in,
		bool wp_spawns_in
	);

	~SpawnGroup();
	uint32 GetNPCType(uint16 condition_value_filter=1);
	void AddSpawnEntry(std::unique_ptr<SpawnEntry> &newEntry);
	uint32 id;
	bool wp_spawns;			// if true, spawn NPCs at a random waypoint location (if spawnpoint has a grid) instead of the spawnpoint's loc
	float  roamdist;
	float  roambox[4];
	int    min_delay;
	int    delay;
	int    despawn;
	uint32 despawn_timer;
private:
	char name_[120];
	std::list<std::unique_ptr<SpawnEntry>> list_;
	uint8 group_spawn_limit; //max # of this entry which can be spawned by this group
};

class SpawnGroupList {
public:
	SpawnGroupList() {}
	~SpawnGroupList();

	void AddSpawnGroup(std::unique_ptr<SpawnGroup> &new_group);
	SpawnGroup *GetSpawnGroup(uint32 id);
	void ClearSpawnGroups();
	void ReloadSpawnGroups();
private:
	std::map<uint32, std::unique_ptr<SpawnGroup>> m_spawn_groups;
};
