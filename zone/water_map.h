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
