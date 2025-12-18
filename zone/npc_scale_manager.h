/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2018 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#pragma once

#include "zone/npc.h"
#include "zone/zone.h"

extern Zone* zone;

class NpcScaleManager {
public:
	struct global_npc_scale {
		int8   type;
		uint8  level;
		uint32 zone_id;
		uint16 instance_version;
		int    ac;
		int64  hp;
		int    accuracy;
		int    slow_mitigation;
		int    attack;
		int    strength;
		int    stamina;
		int    dexterity;
		int    agility;
		int    intelligence;
		int    wisdom;
		int    charisma;
		int    magic_resist;
		int    cold_resist;
		int    fire_resist;
		int    poison_resist;
		int    disease_resist;
		int    corruption_resist;
		int    physical_resist;
		int    min_dmg;
		int    max_dmg;
		int64  hp_regen_rate;
		int64  hp_regen_per_second;
		int    attack_delay;
		int    spell_scale;
		int    heal_scale;
		uint32 avoidance;
		int    heroic_strikethrough;

		std::string special_abilities;
	};

	std::vector<std::string> scaling_stats = {
		"ac",
		"max_hp",
		"accuracy",
		"slow_mitigation",
		"atk",
		"str",
		"sta",
		"dex",
		"agi",
		"int",
		"wis",
		"cha",
		"mr",
		"cr",
		"fr",
		"pr",
		"dr",
		"cr",
		"pr",
		"min_hit",
		"max_hit",
		"hp_regen",
		"hp_regen_per_second",
		"attack_delay",
		"spell_scale",
		"heal_scale",
		"avoidance",
		"heroic_strikethrough",
		"special_abilities"
	};

	void ScaleNPC(NPC* npc, bool always_scale = false, bool override_special_abilities = false);
	void ResetNPCScaling(NPC* npc);
	bool IsAutoScaled(NPC* npc);
	bool LoadScaleData();

	global_npc_scale GetGlobalScaleDataForTypeLevel(
		int8 npc_type,
		uint8 npc_level,
		uint32 zone_id,
		uint16 instance_version
	);

	std::map<std::tuple<int8, uint8, uint32, uint16>, global_npc_scale> npc_global_base_scaling_data;

	int8 GetNPCScalingType(NPC * &npc);
	std::string GetNPCScalingTypeName(NPC * &npc);
	bool ApplyGlobalBaseScalingToNPCStatically(NPC * &npc);
	bool ApplyGlobalBaseScalingToNPCDynamically(NPC * &npc);

	uint32 GetClassLevelDamageMod(uint32 level, uint32 npc_class);
};

extern NpcScaleManager *npc_scale_manager;
