#ifndef REGIONS_H
#define REGIONS_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

class Client;

// ================================================================= AoTv4 region-locked progression
// Zones are grouped into REGIONS (table `regions`). `zone_regions` maps zone -> region, one region
// to many zones. `character_regions_unlocked` records which regions a character has earned.
//
// Rules:
//   * A character may only enter a zone whose region they have unlocked.
//   * A zone with NO region mapping is UNRESTRICTED. This is deliberate: it means adding the feature
//     cannot accidentally lock players out of the entire world, and content becomes gated only once
//     somebody explicitly assigns it to a region.
//   * A region carries a max_level. A character's level ceiling is the HIGHEST max_level among the
//     regions they hold, cached in the data bucket `region_max_level_<char_id>`.
//
// The zone->region map is small and changes only when content is edited, so it is loaded once per
// zone process and kept in memory; the per-character unlock check goes to the DB so an unlock granted
// in another zone is seen immediately.

class RegionManager {
public:
	void LoadZoneRegions();                       // called at zone boot

	// The region a zone belongs to, or 0 when the zone is not mapped (= unrestricted).
	uint32_t GetZoneRegion(uint32_t zone_id) const;

	// Region metadata
	std::string GetRegionName(uint32_t region_id) const;
	uint32_t    GetRegionMaxLevel(uint32_t region_id) const;

	// Does this character hold this region?
	bool HasRegion(uint32_t char_id, uint32_t region_id) const;

	// Grant a region. Returns false if the region does not exist or the character already had it.
	bool UnlockRegion(Client *c, uint32_t region_id);

	// May this character enter this zone? Fills `reason` for the player when not.
	bool CanEnterZone(Client *c, uint32_t zone_id, std::string &reason) const;

	// Highest max_level across the character's unlocked regions, cached in a data bucket.
	uint32_t GetMaxLevel(Client *c) const;
	void     RecalculateMaxLevel(Client *c) const;

private:
	struct RegionInfo {
		std::string name;
		uint32_t    max_level = 0;
	};

	std::map<uint32_t, uint32_t>   m_zone_to_region;   // zoneidnumber -> region id
	std::map<uint32_t, RegionInfo> m_regions;          // region id -> metadata
	bool                           m_loaded = false;
};

extern RegionManager region_manager;

#endif
