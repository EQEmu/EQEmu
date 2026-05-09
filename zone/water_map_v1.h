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

#include "zone/water_map.h"

#pragma pack(push)
#pragma pack(1)

struct ZBSP_Node {
	int32 node_number;
	float normal[3], splitdistance;
	int32 region;
	int32 special;
	int32 left, right;
};

#pragma pack(pop)

class WaterMapV1 : public WaterMap
{
public:
	WaterMapV1();
	~WaterMapV1();
	
	virtual WaterRegionType ReturnRegionType(const glm::vec3& location) const;
	virtual bool InWater(const glm::vec3& location) const;
	virtual bool InVWater(const glm::vec3& location) const;
	virtual bool InLava(const glm::vec3& location) const;
	virtual bool InLiquid(const glm::vec3& location) const;
	virtual bool InPvP(const glm::vec3& location) const;
	virtual bool InZoneLine(const glm::vec3& location) const;
	
protected:
	virtual bool Load(FILE *fp);

private:
	WaterRegionType BSPReturnRegionType(int32 node_number, const glm::vec3& location) const;
	ZBSP_Node* BSP_Root;

	friend class WaterMap;
};
