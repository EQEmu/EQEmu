#pragma once

#include "common/types.h"
#include "zone/position.h"
#include "zone/zone_config.h"

#include <string>

extern const ZoneConfig *Config;

enum WaterRegionType : int {
	RegionTypeUnsupported = -2,
	RegionTypeUntagged = -1,
	RegionTypeNormal = 0,
	RegionTypeWater = 1,
	RegionTypeLava = 2,
	RegionTypeZoneLine = 3,
	RegionTypePVP = 4,
	RegionTypeSlime = 5,
	RegionTypeIce = 6,
	RegionTypeVWater = 7
};

class WaterMap
{
public:
	WaterMap() { }
	virtual ~WaterMap() { }

	static WaterMap* LoadWaterMapfile(std::string zone_name);
	virtual WaterRegionType ReturnRegionType(const glm::vec3& location) const = 0;
	virtual bool InWater(const glm::vec3& location) const = 0;
	virtual bool InVWater(const glm::vec3& location) const = 0;
	virtual bool InLava(const glm::vec3& location) const = 0;
	virtual bool InLiquid(const glm::vec3& location) const = 0;
	virtual bool InPvP(const glm::vec3& location) const = 0;
	virtual bool InZoneLine(const glm::vec3& location) const = 0;

protected:
	virtual bool Load(FILE *fp) { return false; }
};
