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

#include "npc_scale_manager.h"

#include "common/repositories/npc_scale_global_base_repository.h"
#include "common/repositories/npc_types_repository.h"

/**
 * @param npc
 */
void NpcScaleManager::ScaleNPC(
	NPC *npc,
	bool always_scale,
	bool override_special_abilities
) {
	if (npc->IsSkipAutoScale() || npc->GetNPCTypeID() == 0) {
		return;
	}

	auto npc_type         = GetNPCScalingType(npc);
	auto npc_level        = npc->GetLevel();
	auto is_auto_scaled   = IsAutoScaled(npc);
	auto zone_id          = zone->GetZoneID();
	auto instance_version = zone->GetInstanceVersion();

	global_npc_scale scale_data = GetGlobalScaleDataForTypeLevel(
		npc_type,
		npc_level,
		zone_id,
		instance_version
	);

	if (!scale_data.level) {
		LogNPCScaling(
			"NPC: [{}] - scaling data not found for type [{}] level [{}]",
			npc->GetCleanName(),
			npc_type,
			npc_level
		);

		return;
	}

	if (always_scale || (npc->GetAC() == 0 && is_auto_scaled)) {
		npc->ModifyNPCStat("ac", std::to_string(scale_data.ac));
	}

	if (always_scale || npc->GetMaxHP() == 0) {
		npc->ModifyNPCStat("max_hp", std::to_string(scale_data.hp));
		npc->RestoreHealth();
	}

	if (always_scale || npc->GetAccuracyRating() == 0) {
		npc->ModifyNPCStat("accuracy", std::to_string(scale_data.accuracy));
	}

	if (always_scale || npc->GetSlowMitigation() == 0) {
		npc->ModifyNPCStat("slow_mitigation", std::to_string(scale_data.slow_mitigation));
	}

	if (always_scale || npc->GetATK() == 0) {
		npc->ModifyNPCStat("atk", std::to_string(scale_data.attack));
	}

	if (always_scale || npc->GetSTR() == 0) {
		npc->ModifyNPCStat("str", std::to_string(scale_data.strength));
	}

	if (always_scale || npc->GetSTA() == 0) {
		npc->ModifyNPCStat("sta", std::to_string(scale_data.stamina));
	}

	if (always_scale || npc->GetDEX() == 0) {
		npc->ModifyNPCStat("dex", std::to_string(scale_data.dexterity));
	}

	if (always_scale || npc->GetAGI() == 0) {
		npc->ModifyNPCStat("agi", std::to_string(scale_data.agility));
	}

	if (always_scale || npc->GetINT() == 0) {
		npc->ModifyNPCStat("int", std::to_string(scale_data.intelligence));
	}

	if (always_scale || npc->GetWIS() == 0) {
		npc->ModifyNPCStat("wis", std::to_string(scale_data.wisdom));
	}

	if (always_scale || npc->GetCHA() == 0) {
		npc->ModifyNPCStat("cha", std::to_string(scale_data.charisma));
	}

	if (always_scale || npc->GetMR() == 0) {
		npc->ModifyNPCStat("mr", std::to_string(scale_data.magic_resist));
	}

	if (always_scale || npc->GetCR() == 0) {
		npc->ModifyNPCStat("cr", std::to_string(scale_data.cold_resist));
	}

	if (always_scale || npc->GetFR() == 0) {
		npc->ModifyNPCStat("fr", std::to_string(scale_data.fire_resist));
	}

	if (always_scale || npc->GetPR() == 0) {
		npc->ModifyNPCStat("pr", std::to_string(scale_data.poison_resist));
	}

	if (always_scale || npc->GetDR() == 0) {
		npc->ModifyNPCStat("dr", std::to_string(scale_data.disease_resist));
	}

	if (always_scale || (npc->GetCorrup() == 0 && is_auto_scaled)) {
		npc->ModifyNPCStat("cor", std::to_string(scale_data.corruption_resist));
	}

	if (always_scale || (npc->GetPhR() == 0 && is_auto_scaled)) {
		npc->ModifyNPCStat("phr", std::to_string(scale_data.physical_resist));
	}

	// If either is scaled, both need to be.  The values for base_damage and min_damage will be in flux until
	// both are complete.

	if (always_scale || npc->GetMinDMG() == 0 || npc->GetMaxDMG() == 0) {
		int64 min_dmg = scale_data.min_dmg;
		int64 max_dmg = scale_data.max_dmg;

		if (RuleB(Combat, UseNPCDamageClassLevelMods)) {
			uint32 class_level_damage_mod = GetClassLevelDamageMod(npc->GetLevel(), npc->GetClass());
			min_dmg = (min_dmg * class_level_damage_mod) / 220;
			max_dmg = (max_dmg * class_level_damage_mod) / 220;
		}

		npc->ModifyNPCStat("min_hit", std::to_string(min_dmg));
		npc->ModifyNPCStat("max_hit", std::to_string(max_dmg));

		LogNPCScaling("ClassLevelDamageMod::min_dmg base: [{}] calc: [{}]", scale_data.min_dmg, min_dmg);
		LogNPCScaling("ClassLevelDamageMod::max_dmg base: [{}] calc: [{}]", scale_data.max_dmg, max_dmg);
	}

	if (always_scale || (npc->GetHPRegen() == 0 && is_auto_scaled)) {
		npc->ModifyNPCStat("hp_regen", std::to_string(scale_data.hp_regen_rate));
	}

	if (always_scale || (npc->GetHPRegenPerSecond() == 0 && is_auto_scaled)) {
		npc->ModifyNPCStat("hp_regen_per_second", std::to_string(scale_data.hp_regen_per_second));
	}

	if (always_scale || npc->GetAttackDelay() == 0) {
		npc->ModifyNPCStat("attack_delay", std::to_string(scale_data.attack_delay));
	}

	if (always_scale || npc->GetSpellScale() == 0) {
		npc->ModifyNPCStat("spellscale", std::to_string(scale_data.spell_scale));
	}

	if (always_scale || npc->GetHealScale() == 0) {
		npc->ModifyNPCStat("healscale", std::to_string(scale_data.heal_scale));
	}

	if (always_scale || npc->GetAvoidanceRating() == 0) {
		npc->ModifyNPCStat("avoidance", std::to_string(scale_data.avoidance));
	}

	if (always_scale || npc->GetHeroicStrikethrough() == 0) {
		npc->ModifyNPCStat("heroic_strikethrough", std::to_string(scale_data.heroic_strikethrough));
	}

	if (override_special_abilities || (!npc->HasSpecialAbilities() && is_auto_scaled)) {
		npc->ModifyNPCStat("special_abilities", scale_data.special_abilities);
	}

	if (EQEmuLogSys::Instance()->log_settings[Logs::NPCScaling].is_category_enabled == 1) {
		std::string scale_log;

		for (const auto &stat : scaling_stats) {
			auto v = fmt::format("modify_stat_{}", stat);
			if (npc->EntityVariableExists(v)) {
				scale_log += fmt::format("{}: {} ", stat, npc->GetEntityVariable(v));
			}
		}

		LogNPCScaling(
			"([{}]) level: [{}] type: [{}] Auto: [{}] Setting: [{}]",
			npc->GetCleanName(),
			npc_level,
			npc_type,
			(is_auto_scaled ? "true" : "false"),
			scale_log
		);
	}
}

void NpcScaleManager::ResetNPCScaling(NPC* npc)
{
	for (const auto &scaling_stat : scaling_stats) {
		auto stat_name   = fmt::format("modify_stat_{}", scaling_stat);
		auto reset_value = std::to_string(0);
		if (npc->EntityVariableExists(stat_name)) {
			npc->ModifyNPCStat(scaling_stat, reset_value);
		}
	}
}

bool NpcScaleManager::LoadScaleData()
{
	auto rows = NpcScaleGlobalBaseRepository::All(content_db);
	for (const auto &s : rows) {
		if (
			s.zone_id_list.empty() ||
			s.instance_version_list.empty()
		) {
			continue;
		}

		global_npc_scale scale_data;

		scale_data.type                 = s.type;
		scale_data.level                = s.level;
		scale_data.ac                   = s.ac;
		scale_data.hp                   = s.hp;
		scale_data.accuracy             = s.accuracy;
		scale_data.slow_mitigation      = s.slow_mitigation;
		scale_data.attack               = s.attack;
		scale_data.strength             = s.strength;
		scale_data.stamina              = s.stamina;
		scale_data.dexterity            = s.dexterity;
		scale_data.agility              = s.agility;
		scale_data.intelligence         = s.intelligence;
		scale_data.wisdom               = s.wisdom;
		scale_data.charisma             = s.charisma;
		scale_data.magic_resist         = s.magic_resist;
		scale_data.cold_resist          = s.cold_resist;
		scale_data.fire_resist          = s.fire_resist;
		scale_data.poison_resist        = s.poison_resist;
		scale_data.disease_resist       = s.disease_resist;
		scale_data.corruption_resist    = s.corruption_resist;
		scale_data.physical_resist      = s.physical_resist;
		scale_data.min_dmg              = s.min_dmg;
		scale_data.max_dmg              = s.max_dmg;
		scale_data.hp_regen_rate        = s.hp_regen_rate;
		scale_data.hp_regen_per_second  = s.hp_regen_per_second;
		scale_data.attack_delay         = s.attack_delay;
		scale_data.spell_scale          = s.spell_scale;
		scale_data.heal_scale           = s.heal_scale;
		scale_data.avoidance            = s.avoidance;
		scale_data.heroic_strikethrough = s.heroic_strikethrough;

		if (!s.special_abilities.empty()) {
			scale_data.special_abilities = s.special_abilities;
		}

		const auto has_multiple_zones    = Strings::Contains(s.zone_id_list, "|");
		const auto has_multiple_versions = Strings::Contains(s.instance_version_list, "|");

		if (!has_multiple_zones && !has_multiple_versions) {
			scale_data.zone_id          = Strings::ToUnsignedInt(s.zone_id_list);
			scale_data.instance_version = static_cast<uint16>(Strings::ToUnsignedInt(s.instance_version_list));

			npc_global_base_scaling_data.emplace(
				std::make_pair(
					std::make_tuple(
						scale_data.type,
						scale_data.level,
						scale_data.zone_id,
						scale_data.instance_version
					),
					scale_data
				)
			);
		} else if (has_multiple_zones && !has_multiple_versions) {
			scale_data.instance_version = static_cast<uint16>(Strings::ToUnsignedInt(s.instance_version_list));

			const auto zones = Strings::Split(s.zone_id_list, "|");

			for (const auto &z : zones) {
				scale_data.zone_id = Strings::ToUnsignedInt(z);

				npc_global_base_scaling_data.emplace(
					std::make_pair(
						std::make_tuple(
							scale_data.type,
							scale_data.level,
							scale_data.zone_id,
							scale_data.instance_version
						),
						scale_data
					)
				);
			}
		} else if (!has_multiple_zones) {
			scale_data.zone_id = Strings::ToUnsignedInt(s.zone_id_list);

			const auto versions = Strings::Split(s.instance_version_list, "|");

			for (const auto &v : versions) {
				scale_data.instance_version = static_cast<uint16>(Strings::ToUnsignedInt(v));

				npc_global_base_scaling_data.emplace(
					std::make_pair(
						std::make_tuple(
							scale_data.type,
							scale_data.level,
							scale_data.zone_id,
							scale_data.instance_version
						),
						scale_data
					)
				);
			}
		} else {
			const auto zones    = Strings::Split(s.zone_id_list, "|");
			const auto versions = Strings::Split(s.instance_version_list, "|");

			for (const auto &z : zones) {
				scale_data.zone_id = Strings::ToUnsignedInt(z);

				for (const auto &v : versions) {
					scale_data.instance_version = static_cast<uint16>(Strings::ToUnsignedInt(v));

					npc_global_base_scaling_data.emplace(
						std::make_pair(
							std::make_tuple(
								scale_data.type,
								scale_data.level,
								scale_data.zone_id,
								scale_data.instance_version
							),
							scale_data
						)
					);
				}
			}
		}
	}

	LogInfo("Loaded [{}] global scaling data entries", Strings::Commify(rows.size()));

	return true;
}

/**
 * @param npc_type
 * @param npc_level
 * @return NpcScaleManager::global_npc_scale
 */
NpcScaleManager::global_npc_scale NpcScaleManager::GetGlobalScaleDataForTypeLevel(
	int8 npc_type,
	uint8 npc_level,
	uint32 zone_id,
	uint16 instance_version
) {
	auto iter = npc_global_base_scaling_data.find(
		std::make_tuple(
			npc_type,
			npc_level,
			zone_id,
			instance_version
		)
	);

	if (iter != npc_global_base_scaling_data.end()) {
		return iter->second;
	}

	iter = npc_global_base_scaling_data.find(
		std::make_tuple(
			npc_type,
			npc_level,
			zone_id,
			0
		)
	);
	if (iter != npc_global_base_scaling_data.end()) {
		return iter->second;
	}

	iter = npc_global_base_scaling_data.find(
		std::make_tuple(
			npc_type,
			npc_level,
			0,
			0
		)
	);
	if (iter != npc_global_base_scaling_data.end()) {
		return iter->second;
	}

	return {};
}

/**
 * @param level
 * @param npc_class
 * @return
 */
uint32 NpcScaleManager::GetClassLevelDamageMod(uint32 level, uint32 npc_class)
{
	uint32 multiplier;

	switch (npc_class) {
		case Class::Warrior: {
			if (level < 20) {
				multiplier = 220;
			}
			else if (level < 30) {
				multiplier = 230;
			}
			else if (level < 40) {
				multiplier = 250;
			}
			else if (level < 53) {
				multiplier = 270;
			}
			else if (level < 57) {
				multiplier = 280;
			}
			else if (level < 60) {
				multiplier = 290;
			}
			else if (level < 70) {
				multiplier = 300;
			}
			else {
				multiplier = 311;
			}
			break;
		}
		case Class::Druid:
		case Class::Cleric:
		case Class::Shaman: {
			if (level < 70) {
				multiplier = 150;
			}
			else {
				multiplier = 157;
			}
			break;
		}
		case Class::Berserker:
		case Class::Paladin:
		case Class::ShadowKnight: {
			if (level < 35) {
				multiplier = 210;
			}
			else if (level < 45) {
				multiplier = 220;
			}
			else if (level < 51) {
				multiplier = 230;
			}
			else if (level < 56) {
				multiplier = 240;
			}
			else if (level < 60) {
				multiplier = 250;
			}
			else if (level < 68) {
				multiplier = 260;
			}
			else {
				multiplier = 270;
			}
			break;
		}
		case Class::Monk:
		case Class::Bard:
		case Class::Rogue:
		case Class::Beastlord: {
			if (level < 51) {
				multiplier = 180;
			}
			else if (level < 58) {
				multiplier = 190;
			}
			else if (level < 70) {
				multiplier = 200;
			}
			else {
				multiplier = 210;
			}
			break;
		}
		case Class::Ranger: {
			if (level < 58) {
				multiplier = 200;
			}
			else if (level < 70) {
				multiplier = 210;
			}
			else {
				multiplier = 220;
			}
			break;
		}
		case Class::Magician:
		case Class::Wizard:
		case Class::Necromancer:
		case Class::Enchanter: {
			if (level < 70) {
				multiplier = 120;
			}
			else {
				multiplier = 127;
			}
			break;
		}
		default: {
			if (level < 35) {
				multiplier = 210;
			}
			else if (level < 45) {
				multiplier = 220;
			}
			else if (level < 51) {
				multiplier = 230;
			}
			else if (level < 56) {
				multiplier = 240;
			}
			else if (level < 60) {
				multiplier = 250;
			}
			else {
				multiplier = 260;
			}
			break;
		}
	}

	return multiplier;
}

/**
 * @param npc
 * @return int8
 */
int8 NpcScaleManager::GetNPCScalingType(NPC *&npc)
{
	std::string npc_name = npc->GetName();

	if (npc->IsRaidTarget()) {
		return 2;
	}

	if (npc->IsRareSpawn() || npc_name.find('#') != std::string::npos || isupper(npc_name[0])) {
		return 1;
	}

	return 0;
}

/**
 * @param npc
 * @return std::string
 */
std::string NpcScaleManager::GetNPCScalingTypeName(NPC *&npc)
{

	if (int8 scaling_type = GetNPCScalingType(npc); scaling_type == 1) {
		return "Named";
	}

	if (npc->IsRaidTarget()) {
		return "Raid";
	}

	return "Trash";
}

/**
 * Determines based on minimum criteria if NPC is auto scaled for certain things to be scaled like
 * special abilities. We use this so we don't blindly assume we want things to be applied
 *
 * @param npc
 * @return
 */
bool NpcScaleManager::IsAutoScaled(NPC* npc)
{
	return (
		npc->GetHP() == 0 &&
		npc->GetMaxDMG() == 0 &&
		npc->GetMinDMG() == 0 &&
		npc->GetSTR() == 0 &&
		npc->GetSTA() == 0 &&
		npc->GetDEX() == 0 &&
		npc->GetAGI() == 0 &&
		npc->GetINT() == 0 &&
		npc->GetWIS() == 0 &&
		npc->GetCHA() == 0 &&
		npc->GetMR() == 0 &&
		npc->GetFR() == 0 &&
		npc->GetCR() == 0 &&
		npc->GetPR() == 0 &&
		npc->GetDR() == 0
	);
}

/**
 * Returns false if scaling data not found
 * @param npc
 * @return
 */
bool NpcScaleManager::ApplyGlobalBaseScalingToNPCStatically(NPC *&npc)
{
	auto npc_type         = GetNPCScalingType(npc);
	auto npc_level        = npc->GetLevel();
	auto zone_id          = zone->GetZoneID();
	auto instance_version = zone->GetInstanceVersion();

	global_npc_scale g = GetGlobalScaleDataForTypeLevel(
		npc_type,
		npc_level,
		zone_id,
		instance_version
	);

	if (!g.level) {
		LogNPCScaling(
			"NPC: [{}] - scaling data not found for type [{}] level [{}]",
			npc->GetCleanName(),
			npc_type,
			npc_level
		);

		return false;
	}

	auto n = NpcTypesRepository::FindOne(content_db, static_cast<int>(npc->GetNPCTypeID()));
	if (n.id > 0) {
		n.AC                   = g.ac;
		n.hp                   = g.hp;
		n.Accuracy             = g.accuracy;
		n.slow_mitigation      = g.slow_mitigation;
		n.ATK                  = g.attack;
		n.STR                  = g.strength;
		n.STA                  = g.stamina;
		n.DEX                  = g.dexterity;
		n.AGI                  = g.agility;
		n._INT                 = g.intelligence;
		n.WIS                  = g.wisdom;
		n.CHA                  = g.charisma;
		n.MR                   = g.magic_resist;
		n.CR                   = g.cold_resist;
		n.FR                   = g.fire_resist;
		n.PR                   = g.poison_resist;
		n.DR                   = g.disease_resist;
		n.Corrup               = g.corruption_resist;
		n.PhR                  = g.physical_resist;
		n.mindmg               = g.min_dmg;
		n.maxdmg               = g.max_dmg;
		n.hp_regen_rate        = g.hp_regen_rate;
		n.hp_regen_per_second  = g.hp_regen_per_second;
		n.attack_delay         = g.attack_delay;
		n.spellscale           = static_cast<float>(g.spell_scale);
		n.healscale            = static_cast<float>(g.heal_scale);
		n.Avoidance            = g.avoidance;
		n.heroic_strikethrough = g.heroic_strikethrough;
		n.special_abilities    = g.special_abilities;

		return NpcTypesRepository::UpdateOne(content_db, n);
	}

	return false;
}

/**
 * Returns false if scaling data not found
 * @param npc
 * @return
 */
bool NpcScaleManager::ApplyGlobalBaseScalingToNPCDynamically(NPC *&npc)
{
	auto npc_type         = GetNPCScalingType(npc);
	auto npc_level        = npc->GetLevel();
	auto zone_id          = zone->GetZoneID();
	auto instance_version = zone->GetInstanceVersion();

	global_npc_scale d = GetGlobalScaleDataForTypeLevel(
		npc_type,
		npc_level,
		zone_id,
		instance_version
	);

	if (!d.level) {
		LogNPCScaling(
			"NPC: [{}] - scaling data not found for type [{}] level [{}]",
			npc->GetCleanName(),
			npc_type,
			npc_level
		);

		return false;
	}

	auto n = NpcTypesRepository::FindOne(content_db, static_cast<int>(npc->GetNPCTypeID()));
	if (n.id > 0) {
		n.AC                   = 0;
		n.hp                   = 0;
		n.Accuracy             = 0;
		n.slow_mitigation      = 0;
		n.ATK                  = 0;
		n.STR                  = 0;
		n.STA                  = 0;
		n.DEX                  = 0;
		n.AGI                  = 0;
		n._INT                 = 0;
		n.WIS                  = 0;
		n.CHA                  = 0;
		n.MR                   = 0;
		n.CR                   = 0;
		n.FR                   = 0;
		n.PR                   = 0;
		n.DR                   = 0;
		n.Corrup               = 0;
		n.PhR                  = 0;
		n.mindmg               = 0;
		n.maxdmg               = 0;
		n.hp_regen_rate        = 0;
		n.hp_regen_per_second  = 0;
		n.attack_delay         = 0;
		n.spellscale           = 0;
		n.healscale            = 0;
		n.Avoidance	           = 0;
		n.heroic_strikethrough = 0;
		n.special_abilities    = "";

		return NpcTypesRepository::UpdateOne(content_db, n);
	}

	return false;
}
