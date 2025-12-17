/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2020 EQEmulator Development Team (https://github.com/EQEmu/Server)
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
 
#include "zone_store.h"

#include "common/content/world_content_service.h"
#include "common/stacktrace/backward.hpp"


#define DEFAULT_MINIMUM_CLIP 50.0f
#define DEFAULT_MAXIMUM_CLIP 175.0f
#define DEFAULT_ZONE_FAST_REGEN 180
#define DEFAULT_ZONE_GRAVITY 0.4f
#define DEFAULT_ZONE_MIN_MAX_EXPANSION -1
#define DEFAULT_ZONE_MAX_AGGRO_DISTANCE 600
#define DEFAULT_ZONE_MAX_MOVEMENT_UPDATE_RANGE 600
#define DEFAULT_ZONE_RUNSPEED 0.4f
#define DEFAULT_ZONE_SHUTDOWN_DELAY 3600000
#define DEFAULT_ZONE_TYPE 1
#define FOG_SLOT_ONE 1
#define FOG_SLOT_TWO 2
#define FOG_SLOT_THREE 3
#define FOG_SLOT_FOUR 4
#define RAIN_SLOT_ONE 1
#define RAIN_SLOT_TWO 2
#define RAIN_SLOT_THREE 3
#define RAIN_SLOT_FOUR 4
#define SNOW_SLOT_ONE 1
#define SNOW_SLOT_TWO 2
#define SNOW_SLOT_THREE 3
#define SNOW_SLOT_FOUR 4

ZoneStore::ZoneStore() = default;
ZoneStore::~ZoneStore() = default;

// cache record of zones for fast successive retrieval
void ZoneStore::LoadZones(Database &db)
{
	m_zones = ZoneRepository::All(db);

	LogInfo("Loaded [{}] zones", m_zones.size());
}

/**
 * @param in_zone_name
 * @return
 */
uint32 ZoneStore::GetZoneID(const char *in_zone_name)
{
	if (in_zone_name == nullptr) {
		return 0;
	}

	std::string zone_name = Strings::ToLower(in_zone_name);

	return GetZoneID(zone_name);
}

/**
 * @param zone_name
 * @return
 */
uint32 ZoneStore::GetZoneID(std::string zone_name)
{
	for (auto &z: m_zones) {
		if (z.short_name == zone_name) {
			return z.zoneidnumber;
		}
	}

	LogInfo("Failed to get zone_name [{}]", zone_name);

	return 0;
}

/**
 * @param zone_id
 * @param error_unknown
 * @return
 */
const char *ZoneStore::GetZoneName(uint32 zone_id, bool error_unknown)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id) {
			return z.short_name.c_str();
		}
	}

	if (error_unknown) {
		return "UNKNOWN";
	}

	return nullptr;
}

/**
 * @param zone_id
 * @param error_unknown
 * @return
 */
const char *ZoneStore::GetZoneLongName(uint32 zone_id, bool error_unknown)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id) {
			return z.long_name.c_str();
		}
	}

	if (error_unknown) {
		return "UNKNOWN";
	}

	return nullptr;
}

/**
 * @param zone_id
 * @return
 */
std::string ZoneStore::GetZoneName(uint32 zone_id)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id) {
			return z.short_name;
		}
	}

	LogInfo("Failed to get zone long name by zone_id [{}]", zone_id);

	return {};
}

/**
 * @param zone_id
 * @return
 */
std::string ZoneStore::GetZoneLongName(uint32 zone_id)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id) {
			return z.long_name;
		}
	}

	LogInfo("Failed to get zone long name by zone_id [{}]", zone_id);

	return {};
}

std::string ZoneStore::GetZoneShortNameByLongName(const std::string& zone_long_name)
{
	for (const auto& z : m_zones) {
		if (z.long_name == zone_long_name) {
			return z.short_name;
		}
	}

	LogInfo("Failed to get zone short name by zone_long_name [{}]", zone_long_name);

	return {};
}

uint32 ZoneStore::GetZoneIDByLongName(const std::string& zone_long_name)
{
	for (const auto& z : m_zones) {
		if (z.long_name == zone_long_name) {
			return z.zoneidnumber;
		}
	}

	LogInfo("Failed to get zone ID by zone_long_name [{}]", zone_long_name);

	return 0;
}

/**
 * @param zone_id
 * @param version
 * @return
 */
ZoneRepository::Zone *ZoneStore::GetZone(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return &z;
		}
	}

	LogInfo("Failed to get zone by zone_id [{}] version [{}]", zone_id, version);

	return nullptr;
}

/**
 * @param in_zone_name
 * @return
 */
ZoneRepository::Zone *ZoneStore::GetZone(const char *in_zone_name)
{
	for (auto &z: m_zones) {
		if (z.short_name == in_zone_name) {
			return &z;
		}
	}

	LogInfo("Failed to get zone by zone_name [{}]", in_zone_name);

	return nullptr;
}

ZoneRepository::Zone *ZoneStore::GetZone(const std::string& in_zone_name)
{
	for (auto &z: m_zones) {
		if (z.short_name == in_zone_name) {
			return &z;
		}
	}

	return nullptr;
}

const std::vector<ZoneRepository::Zone> &ZoneStore::GetZones() const
{
	return m_zones;
}

// gets zone data by using explicit version and falling back to version 0 if not found
ZoneRepository::Zone *ZoneStore::GetZoneWithFallback(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return &z;
		}
	}

	// second pass, default to version 0 if specific doesn't exist
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return &z;
		}
	}

	LogInfo("Failed to get zone by zone_id [{}] version [{}]", zone_id, version);

	return nullptr;
}

glm::vec4 ZoneStore::GetZoneSafeCoordinates(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? glm::vec4(z->safe_x, z->safe_y, z->safe_z, z->safe_heading) : glm::vec4(0.f);
}

float ZoneStore::GetZoneGraveyardID(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->graveyard_id : 0;
}

uint8 ZoneStore::GetZoneMinimumLevel(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->min_level : 0;
}

uint8 ZoneStore::GetZoneMaximumLevel(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->max_level : UINT8_MAX;
}

uint8 ZoneStore::GetZoneMinimumStatus(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->min_status : 0;
}

int ZoneStore::GetZoneTimeZone(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->timezone : 0;
}

int ZoneStore::GetZoneMaximumPlayers(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->maxclients : 0;
}

uint32 ZoneStore::GetZoneRuleSet(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->ruleset : 0;
}

const std::string ZoneStore::GetZoneNote(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->note : "";
}

float ZoneStore::GetZoneUnderworld(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->underworld : 0.0f;
}

float ZoneStore::GetZoneMinimumClip(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->minclip : DEFAULT_MINIMUM_CLIP;
}

float ZoneStore::GetZoneMaximumClip(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->maxclip : DEFAULT_MAXIMUM_CLIP;
}

float ZoneStore::GetZoneFogMinimumClip(uint32 zone_id, uint8 slot, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);

	if (!z) {
		return DEFAULT_MINIMUM_CLIP;
	}

	switch (slot) {
		case FOG_SLOT_ONE:
			return z->fog_minclip1;
		case FOG_SLOT_TWO:
			return z->fog_minclip2;
		case FOG_SLOT_THREE:
			return z->fog_minclip3;
		case FOG_SLOT_FOUR:
			return z->fog_minclip4;
		default:
			return z->fog_minclip;
	}
}

float ZoneStore::GetZoneFogMaximumClip(uint32 zone_id, uint8 slot, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);

	if (!z) {
		return DEFAULT_MAXIMUM_CLIP;
	}

	switch (slot) {
		case FOG_SLOT_ONE:
			return z->fog_maxclip1;
		case FOG_SLOT_TWO:
			return z->fog_maxclip2;
		case FOG_SLOT_THREE:
			return z->fog_maxclip3;
		case FOG_SLOT_FOUR:
			return z->fog_maxclip4;
		default:
			return z->fog_maxclip;
	}
}

uint8 ZoneStore::GetZoneFogRed(uint32 zone_id, uint8 slot, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);

	if (!z) {
		return 0;
	}

	switch (slot) {
		case FOG_SLOT_ONE:
			return z->fog_red1;
		case FOG_SLOT_TWO:
			return z->fog_red2;
		case FOG_SLOT_THREE:
			return z->fog_red3;
		case FOG_SLOT_FOUR:
			return z->fog_red4;
		default:
			return z->fog_red;
	}
}

uint8 ZoneStore::GetZoneFogGreen(uint32 zone_id, uint8 slot, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);

	if (!z) {
		return 0;
	}

	switch (slot) {
		case FOG_SLOT_ONE:
			return z->fog_green1;
		case FOG_SLOT_TWO:
			return z->fog_green2;
		case FOG_SLOT_THREE:
			return z->fog_green3;
		case FOG_SLOT_FOUR:
			return z->fog_green4;
		default:
			return z->fog_green;
	}
}

uint8 ZoneStore::GetZoneFogBlue(uint32 zone_id, uint8 slot, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);

	if (!z) {
		return 0;
	}

	switch (slot) {
		case FOG_SLOT_ONE:
			return z->fog_blue1;
		case FOG_SLOT_TWO:
			return z->fog_blue2;
		case FOG_SLOT_THREE:
			return z->fog_blue3;
		case FOG_SLOT_FOUR:
			return z->fog_blue4;
		default:
			return z->fog_blue;
	}
}

uint8 ZoneStore::GetZoneSky(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->sky : 0;
}

uint8 ZoneStore::GetZoneZType(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->ztype : 0;
}

float ZoneStore::GetZoneExperienceMultiplier(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->zone_exp_multiplier : 1.0f;
}

float ZoneStore::GetZoneWalkSpeed(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->walkspeed : DEFAULT_ZONE_RUNSPEED;
}

uint8 ZoneStore::GetZoneTimeType(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->time_type : 0;
}

float ZoneStore::GetZoneFogDensity(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->fog_density : 0.0f;
}

const std::string ZoneStore::GetZoneFlagNeeded(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->flag_needed : "";
}

int8 ZoneStore::GetZoneCanBind(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->canbind : 0;
}

int8 ZoneStore::GetZoneCanCombat(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->cancombat : 0;
}

int8 ZoneStore::GetZoneCanLevitate(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->canlevitate : 0;
}

int8 ZoneStore::GetZoneCastOutdoor(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->castoutdoor : 0;
}

uint8 ZoneStore::GetZoneHotzone(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->hotzone : 0;
}

uint8 ZoneStore::GetZoneInstanceType(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->insttype : 0;
}

uint64 ZoneStore::GetZoneShutdownDelay(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->shutdowndelay : DEFAULT_ZONE_SHUTDOWN_DELAY;
}

int8 ZoneStore::GetZonePEQZone(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->peqzone : 0;
}

int8 ZoneStore::GetZoneExpansion(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->expansion : 0;
}

int8 ZoneStore::GetZoneBypassExpansionCheck(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->bypass_expansion_check : 0;
}

uint8 ZoneStore::GetZoneSuspendBuffs(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->suspendbuffs : 0;
}

int ZoneStore::GetZoneRainChance(uint32 zone_id, uint8 slot, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);

	if (!z) {
		return 0;
	}

	switch (slot) {
		case RAIN_SLOT_TWO:
			return z->rain_chance2;
		case RAIN_SLOT_THREE:
			return z->rain_chance3;
		case RAIN_SLOT_FOUR:
			return z->rain_chance4;
		case RAIN_SLOT_ONE:
		default:
			return z->rain_chance1;
	}
}

int ZoneStore::GetZoneRainDuration(uint32 zone_id, uint8 slot, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);

	if (!z) {
		return 0;
	}

	switch (slot) {
		case RAIN_SLOT_TWO:
			return z->rain_duration2;
		case RAIN_SLOT_THREE:
			return z->rain_duration3;
		case RAIN_SLOT_FOUR:
			return z->rain_duration4;
		case RAIN_SLOT_ONE:
		default:
			return z->rain_duration1;
	}
}

int ZoneStore::GetZoneSnowChance(uint32 zone_id, uint8 slot, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);

	if (!z) {
		return 0;
	}

	switch (slot) {
		case SNOW_SLOT_TWO:
			return z->snow_chance2;
		case SNOW_SLOT_THREE:
			return z->snow_chance3;
		case SNOW_SLOT_FOUR:
			return z->snow_chance4;
		case SNOW_SLOT_ONE:
		default:
			return z->snow_chance1;
	}
}

int ZoneStore::GetZoneSnowDuration(uint32 zone_id, uint8 slot, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);

	if (!z) {
		return 0;
	}

	switch (slot) {
		case SNOW_SLOT_TWO:
			return z->snow_duration2;
		case SNOW_SLOT_THREE:
			return z->snow_duration3;
		case SNOW_SLOT_FOUR:
			return z->snow_duration4;
		case SNOW_SLOT_ONE:
		default:
			return z->snow_duration1;
	}
}

float ZoneStore::GetZoneGravity(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->gravity : DEFAULT_ZONE_GRAVITY;
}

int ZoneStore::GetZoneType(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->type : DEFAULT_ZONE_TYPE;
}

int8 ZoneStore::GetZoneSkyLock(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->skylock : 0;
}

int ZoneStore::GetZoneFastRegenHP(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->fast_regen_hp : DEFAULT_ZONE_FAST_REGEN;
}

int ZoneStore::GetZoneFastRegenMana(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->fast_regen_mana : DEFAULT_ZONE_FAST_REGEN;
}

int ZoneStore::GetZoneFastRegenEndurance(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->fast_regen_endurance : DEFAULT_ZONE_FAST_REGEN;
}

int ZoneStore::GetZoneNPCMaximumAggroDistance(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->npc_max_aggro_dist : DEFAULT_ZONE_MAX_AGGRO_DISTANCE;
}

int8 ZoneStore::GetZoneMinimumExpansion(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->min_expansion : DEFAULT_ZONE_MIN_MAX_EXPANSION;
}

int8 ZoneStore::GetZoneMaximumExpansion(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->max_expansion : DEFAULT_ZONE_MIN_MAX_EXPANSION;
}

const std::string ZoneStore::GetZoneContentFlags(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->content_flags : "";
}

const std::string ZoneStore::GetZoneContentFlagsDisabled(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->content_flags_disabled : "";
}

int ZoneStore::GetZoneUnderworldTeleportIndex(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->underworld_teleport_index : 0;
}

int ZoneStore::GetZoneLavaDamage(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->lava_damage : 0;
}

int ZoneStore::GetZoneMinimumLavaDamage(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->min_lava_damage : 0;
}

uint8 ZoneStore::GetZoneIdleWhenEmpty(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->idle_when_empty : 1;
}

uint32 ZoneStore::GetZoneSecondsBeforeIdle(uint32 zone_id, int version)
{
	const auto& z = GetZoneVersionWithFallback(zone_id, version);
	return z ? z->seconds_before_idle : 60;
}
