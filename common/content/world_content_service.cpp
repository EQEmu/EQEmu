#include "world_content_service.h"

#include "common/database.h"
#include "common/eqemu_logsys.h"
#include "common/repositories/instance_list_repository.h"
#include "common/rulesys.h"
#include "common/zone_store.h"

#include "glm/vec3.hpp"
#include <utility>

WorldContentService::WorldContentService()
{
	SetCurrentExpansion(Expansion::EXPANSION_ALL);
}

int WorldContentService::GetCurrentExpansion() const
{
	return current_expansion;
}

WorldContentService *WorldContentService::SetExpansionContext()
{
	// do a rule manager reload until where we store expansion is changed to somewhere else
	RuleManager::Instance()->LoadRules(GetDatabase(), "default", true);

	// pull expansion from rules
	int expansion = RuleI(Expansion, CurrentExpansion);
	if (expansion >= Expansion::Classic && expansion <= Expansion::MaxId) {
		WorldContentService::Instance()->SetCurrentExpansion(expansion);
	}

	LogInfo(
		"Current expansion is [{}] ({})",
		GetCurrentExpansion(),
		GetCurrentExpansionName()
	);

	return this;
}

std::string WorldContentService::GetCurrentExpansionName()
{
	if (WorldContentService::Instance()->GetCurrentExpansion() == Expansion::EXPANSION_ALL) {
		return "All Expansions";
	}

	if (current_expansion >= Expansion::Classic && current_expansion <= Expansion::MaxId) {
		return Expansion::ExpansionName[WorldContentService::Instance()->GetCurrentExpansion()];
	}

	return "Unknown Expansion";
}

/**
 * @param current_expansion
 */
void WorldContentService::SetCurrentExpansion(int current_expansion)
{
	WorldContentService::current_expansion = current_expansion;
}

/**
 * @return
 */
const std::vector<ContentFlagsRepository::ContentFlags> &WorldContentService::GetContentFlags() const
{
	return content_flags;
}

/**
 * @return
 */
std::vector<std::string> WorldContentService::GetContentFlagsEnabled()
{
	std::vector<std::string> enabled_flags;

	for (auto &f: GetContentFlags()) {
		if (f.enabled) {
			enabled_flags.emplace_back(f.flag_name);
		}
	}

	return enabled_flags;
}

/**
 * @return
 */
std::vector<std::string> WorldContentService::GetContentFlagsDisabled()
{
	std::vector<std::string> disabled_flags;

	for (auto &f: GetContentFlags()) {
		if (!f.enabled) {
			disabled_flags.emplace_back(f.flag_name);
		}
	}

	return disabled_flags;
}

/**
 * @param content_flags
 */
void WorldContentService::SetContentFlags(const std::vector<ContentFlagsRepository::ContentFlags> &content_flags)
{
	WorldContentService::content_flags = content_flags;
}

/**
 * @param content_flag
 * @return
 */
bool WorldContentService::IsContentFlagEnabled(const std::string &content_flag)
{
	for (auto &f: GetContentFlags()) {
		if (f.flag_name == content_flag && f.enabled == true) {
			return true;
		}
	}

	return false;
}

/**
 * @param content_flag
 * @return
 */
bool WorldContentService::IsContentFlagDisabled(const std::string &content_flag)
{
	for (auto &f: GetContentFlags()) {
		if (f.flag_name == content_flag && f.enabled == false) {
			return true;
		}
	}

	return false;
}

bool WorldContentService::DoesPassContentFiltering(const ContentFlags &f)
{
	// if we're not set to (-1 All) then fail when we aren't within minimum expansion
	if (f.min_expansion > Expansion::EXPANSION_ALL && current_expansion < f.min_expansion && current_expansion != -1) {
		return false;
	}

	// if we're not set to (-1 All) then fail when we aren't within max expansion
	if (f.max_expansion > Expansion::EXPANSION_ALL && current_expansion > f.max_expansion && current_expansion != -1) {
		return false;
	}

	// if we don't have any enabled flag in enabled flags, we fail
	for (const auto &flag: Strings::Split(f.content_flags)) {
		if (!Strings::Contains(GetContentFlagsEnabled(), flag)) {
			return false;
		}
	}

	// if we don't have any disabled flag in disabled flags, we fail
	for (const auto &flag: Strings::Split(f.content_flags_disabled)) {
		if (!Strings::Contains(GetContentFlagsDisabled(), flag)) {
			return false;
		}
	}

	return true;
}

void WorldContentService::ReloadContentFlags()
{
	std::vector<ContentFlagsRepository::ContentFlags> set_content_flags;
	auto                                              flags = ContentFlagsRepository::All(*GetDatabase());

	set_content_flags.reserve(flags.size());
	for (auto &f: flags) {
		set_content_flags.push_back(f);

		LogInfo(
			"Loaded content flag [{}] [{}]",
			f.flag_name,
			(f.enabled ? "enabled" : "disabled")
		);
	}

	SetContentFlags(set_content_flags);
	LoadStaticGlobalZoneInstances();
	ZoneStore::Instance()->LoadZones(*m_content_database);
}

Database *WorldContentService::GetDatabase() const
{
	return m_database;
}

WorldContentService *WorldContentService::SetDatabase(Database *database)
{
	WorldContentService::m_database = database;

	return this;
}

Database *WorldContentService::GetContentDatabase() const
{
	return m_content_database;
}

WorldContentService *WorldContentService::SetContentDatabase(Database *database)
{
	WorldContentService::m_content_database = database;

	return this;
}

void WorldContentService::SetContentFlag(const std::string &content_flag_name, bool enabled)
{
	auto flags = ContentFlagsRepository::GetWhere(
		*GetDatabase(),
		fmt::format("flag_name = '{}'", content_flag_name)
	);

	auto f = ContentFlagsRepository::NewEntity();
	if (!flags.empty()) {
		f = flags.front();
	}

	f.enabled   = enabled ? 1 : 0;
	f.flag_name = content_flag_name;

	if (!flags.empty()) {
		ContentFlagsRepository::UpdateOne(*GetDatabase(), f);
	}
	else {
		ContentFlagsRepository::InsertOne(*GetDatabase(), f);
	}

	ReloadContentFlags();
}

void WorldContentService::HandleZoneRoutingMiddleware(ZoneChange_Struct *zc)
{
	auto r = FindZone(zc->zoneID, zc->instanceID);
	if (r.zone_id == 0) {
		return;
	}

	zc->instanceID = r.instance.id;
}

// LoadStaticGlobalZoneInstances loads all static global zone instances
// these are zones that are never set to expire and are global
// these are used commonly in v1/v2/v3 versions of the same zone for expansion routing
WorldContentService *WorldContentService::LoadStaticGlobalZoneInstances()
{
	m_zone_static_instances = InstanceListRepository::GetWhere(
		*GetDatabase(),
		fmt::format("never_expires = 1 AND is_global = 1")
	);

	LogInfo("Loaded [{}] zone_instances", m_zone_static_instances.size());

	return this;
}

// FindZone handles content and context aware zone routing (middleware)
//
// this is a middleware function that is meant to be used in the zone change process
// this hooks all core zone changes within the server and routes the player to the correct zone
// returning a zone_id of non-zero means the middleware will route the player
// returning a zone_id of 0 means the middleware will not route the player
// this is useful for handling multiple versions of the same zone
//
// implementation >
// the zoning and process spawning logic already is handled by two keys "zone_id" and "instance_id"
// we leverage static, never expires instances to handle this and client still sees it as a normal zone
//
// content awareness >
// simply use the zone_id, server content settings and the middleware will handle the rest
// you don't have to think about instances in any data tables (use instance_id 0)
// you don't have to keep track of instance ids in scripts (use instance_id 0)
// the versions of zones are represented by two zone entries that have potentially different min/max expansion and/or different content flags
// we decide to route the client to the correct version of the zone based on the current server side expansion
//
// example >
// we want to route players to the correct version of lavastorm based on the current server side expansion (DoesZonePassContentFiltering)
// lavastorm (pre-don) version 0 (classic)
//   zone table entry for version = 0, min_expansion = 0, max_expansion = 8
//   instance_list table entry for lavastorm has version = 0, is_global = 1, never_expires = 1
// lavastorm (don) version 1
//   zone table entry for version = 1, min_expansion = 9, max_expansion = 99
//   instance_list table entry for lavastorm has version = 1, is_global = 1, never_expires = 1
WorldContentService::FindZoneResult WorldContentService::FindZone(uint32 zone_id, uint32 instance_id)
{
	for (const auto &z: ZoneStore::Instance()->GetZones()) {
		for (auto &i: m_zone_static_instances) {
			if (
				z.zoneidnumber == zone_id &&
				DoesZonePassContentFiltering(z) &&
				i.zone == zone_id &&
				i.version == z.version) {

				if (instance_id > 0 && i.id != instance_id) {
					continue;
				}

				LogInfo(
					"Routed player to public static instance [{}] of zone [{}] ({}) version [{}] long_name [{}] notes [{}]",
					i.id,
					z.short_name,
					z.zoneidnumber,
					z.version,
					z.long_name,
					i.notes
				);

				return WorldContentService::FindZoneResult{
					.zone_id = static_cast<uint32>(z.zoneidnumber),
					.instance = i,
					.zone = z
				};
			}
		}
	}

	return WorldContentService::FindZoneResult{.zone_id = 0};
}

bool WorldContentService::IsInPublicStaticInstance(uint32 instance_id)
{
	for (auto &i: m_zone_static_instances) {
		if (i.id == instance_id) {
			return true;
		}
	}

	return false;
}

bool WorldContentService::DoesZonePassContentFiltering(const ZoneRepository::Zone &z)
{
	auto f = ContentFlags{
		.min_expansion = z.min_expansion,
		.max_expansion = z.max_expansion,
		.content_flags = z.content_flags,
		.content_flags_disabled = z.content_flags_disabled
	};

	return DoesPassContentFiltering(f);
}
