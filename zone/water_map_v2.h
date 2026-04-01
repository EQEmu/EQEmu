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

#include "zone/oriented_bounding_box.h"
#include "zone/water_map.h"

#include <utility>
#include <vector>

class WaterMapV2 : public WaterMap
{
public:
	WaterMapV2();
	~WaterMapV2();

	virtual WaterRegionType ReturnRegionType(const glm::vec3& location) const;
	virtual bool InWater(const glm::vec3& location) const;
	virtual bool InVWater(const glm::vec3& location) const;
	virtual bool InLava(const glm::vec3& location) const;
	virtual bool InLiquid(const glm::vec3& location) const;
	virtual bool InPvP(const glm::vec3& location) const;
	virtual bool InZoneLine(const glm::vec3& location) const;

protected:
	virtual bool Load(FILE *fp);

	std::vector<std::pair<WaterRegionType, OrientedBoundingBox>> regions;
	friend class WaterMap;
};
