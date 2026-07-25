#include "regions.h"

#include "client.h"
#include "zonedb.h"
#include "../common/data_bucket.h"
#include "../common/repositories/regions_repository.h"
#include "../common/repositories/zone_regions_repository.h"
#include "../common/repositories/character_regions_unlocked_repository.h"
#include "../common/strings.h"

RegionManager region_manager;

void RegionManager::LoadZoneRegions()
{
	m_zone_to_region.clear();
	m_regions.clear();

	for (const auto &r : RegionsRepository::All(content_db)) {
		m_regions[r.id] = RegionInfo{r.name, (uint32_t) r.max_level};
	}
	for (const auto &zr : ZoneRegionsRepository::All(content_db)) {
		m_zone_to_region[zr.zone_id] = zr.region_id;
	}

	m_loaded = true;
	LogInfo("Loaded [{}] regions covering [{}] zones", m_regions.size(), m_zone_to_region.size());
}

uint32_t RegionManager::GetZoneRegion(uint32_t zone_id) const
{
	const auto it = m_zone_to_region.find(zone_id);
	return (it == m_zone_to_region.end()) ? 0 : it->second;
}

std::string RegionManager::GetRegionName(uint32_t region_id) const
{
	const auto it = m_regions.find(region_id);
	return (it == m_regions.end()) ? std::string() : it->second.name;
}

uint32_t RegionManager::GetRegionMaxLevel(uint32_t region_id) const
{
	const auto it = m_regions.find(region_id);
	return (it == m_regions.end()) ? 0 : it->second.max_level;
}

// Queried live rather than cached: an unlock granted while the character is in another zone (or by a
// quest running elsewhere) has to be visible here immediately, or they would be bounced at a zone
// line they were just told they could use.
bool RegionManager::HasRegion(uint32_t char_id, uint32_t region_id) const
{
	if (!char_id || !region_id) {
		return false;
	}
	return !CharacterRegionsUnlockedRepository::GetWhere(
		database,
		fmt::format("`char_id` = {} AND `region_id` = {} LIMIT 1", char_id, region_id)
	).empty();
}

bool RegionManager::UnlockRegion(Client *c, uint32_t region_id)
{
	if (!c || !region_id) {
		return false;
	}
	if (m_regions.find(region_id) == m_regions.end()) {
		LogInfo("UnlockRegion: character [{}] asked for region [{}] which does not exist", c->GetCleanName(), region_id);
		return false;
	}
	if (HasRegion(c->CharacterID(), region_id)) {
		return false;   // already held; nothing to do
	}

	CharacterRegionsUnlockedRepository::CharacterRegionsUnlocked e{};
	e.region_id   = region_id;
	e.char_id     = c->CharacterID();
	e.char_name   = c->GetCleanName();
	e.region_name = GetRegionName(region_id);

	if (CharacterRegionsUnlockedRepository::InsertOne(database, e).char_id == 0) {
		return false;
	}

	RecalculateMaxLevel(c);
	c->Message(
		Chat::Yellow,
		"You have unlocked the region of %s.",
		e.region_name.empty() ? "an unknown land" : e.region_name.c_str()
	);
	return true;
}

bool RegionManager::CanEnterZone(Client *c, uint32_t zone_id, std::string &reason) const
{
	if (!c) {
		return false;
	}
	// GMs are never region-gated; they need to be able to reach content to fix it.
	if (c->GetGM()) {
		return true;
	}

	const uint32_t region_id = GetZoneRegion(zone_id);
	if (!region_id) {
		return true;   // unmapped zone: unrestricted by design (see regions.h)
	}
	if (HasRegion(c->CharacterID(), region_id)) {
		return true;
	}

	const std::string name = GetRegionName(region_id);
	reason = fmt::format(
		"You have not yet unlocked {}.",
		name.empty() ? "that region" : name
	);
	return false;
}

uint32_t RegionManager::GetMaxLevel(Client *c) const
{
	if (!c) {
		return 0;
	}
	const std::string v =
		DataBucket::GetData(&database, fmt::format("region_max_level_{}", c->CharacterID()));
	if (v.empty()) {
		RecalculateMaxLevel(c);
		return (uint32_t) Strings::ToInt(
			DataBucket::GetData(&database, fmt::format("region_max_level_{}", c->CharacterID()))
		);
	}
	return (uint32_t) Strings::ToInt(v);
}

// The ceiling is the best region the character holds, not the sum -- unlocking a lower region later
// must never reduce it.
void RegionManager::RecalculateMaxLevel(Client *c) const
{
	if (!c) {
		return;
	}
	uint32_t best = 0;
	for (const auto &e : CharacterRegionsUnlockedRepository::GetWhere(
		database, fmt::format("`char_id` = {}", c->CharacterID()))) {
		const auto it = m_regions.find(e.region_id);
		if (it != m_regions.end() && it->second.max_level > best) {
			best = it->second.max_level;
		}
	}
	DataBucket::SetData(
		&database,
		fmt::format("region_max_level_{}", c->CharacterID()),
		std::to_string(best)
	);
}

// ================================================================= Client thin wrappers
// These exist so Lua (and the rest of the zone) has a Client-scoped API, per the spec's
// Client:UnlockRegion(region_id) / Client:HasRegion(region_id).
bool Client::UnlockRegion(uint32 region_id)
{
	return region_manager.UnlockRegion(this, region_id);
}

bool Client::HasRegion(uint32 region_id)
{
	return region_manager.HasRegion(CharacterID(), region_id);
}

uint32 Client::GetRegionMaxLevel()
{
	return region_manager.GetMaxLevel(this);
}
