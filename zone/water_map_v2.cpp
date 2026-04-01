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
#include "water_map_v2.h"

WaterMapV2::WaterMapV2() {
}

WaterMapV2::~WaterMapV2() {
}

WaterRegionType WaterMapV2::ReturnRegionType(const glm::vec3& location) const {
	size_t sz = regions.size();
	for(size_t i = 0; i < sz; ++i) {
		auto const &region = regions[i];
		if (region.second.ContainsPoint(glm::vec3(location.y, location.x, location.z))) {
			return region.first;
		}
	}
	return RegionTypeNormal;
}

bool WaterMapV2::InWater(const glm::vec3& location) const {
	return ReturnRegionType(location) == RegionTypeWater;
}

bool WaterMapV2::InVWater(const glm::vec3& location) const {
	return ReturnRegionType(location) == RegionTypeVWater;
}

bool WaterMapV2::InLava(const glm::vec3& location) const {
	return ReturnRegionType(location) == RegionTypeLava;
}

bool WaterMapV2::InLiquid(const glm::vec3& location) const {
	auto rt = ReturnRegionType(location);
	return rt == RegionTypeWater || rt == RegionTypeVWater || rt == RegionTypeLava;
}

bool WaterMapV2::InPvP(const glm::vec3& location) const {
	return ReturnRegionType(location) == RegionTypePVP;
}

bool WaterMapV2::InZoneLine(const glm::vec3& location) const {
	return ReturnRegionType(location) == RegionTypeZoneLine;
}

bool WaterMapV2::Load(FILE *fp) {
	uint32 region_count;
	if (fread(&region_count, sizeof(region_count), 1, fp) != 1) {
		return false;
	}

	for(uint32_t i = 0; i < region_count; ++i) {
		uint32_t region_type;
		float x;
		float y;
		float z;
		float x_rot;
		float y_rot;
		float z_rot;
		float x_scale;
		float y_scale;
		float z_scale;
		float x_extent;
		float y_extent;
		float z_extent;

		if (fread(&region_type, sizeof(region_type), 1, fp) != 1) {
			return false;
		}

		if (fread(&x, sizeof(x), 1, fp) != 1) {
			return false;
		}

		if (fread(&y, sizeof(y), 1, fp) != 1) {
			return false;
		}

		if (fread(&z, sizeof(z), 1, fp) != 1) {
			return false;
		}

		if (fread(&x_rot, sizeof(x_rot), 1, fp) != 1) {
			return false;
		}

		if (fread(&y_rot, sizeof(y_rot), 1, fp) != 1) {
			return false;
		}

		if (fread(&z_rot, sizeof(z_rot), 1, fp) != 1) {
			return false;
		}

		if (fread(&x_scale, sizeof(x_scale), 1, fp) != 1) {
			return false;
		}

		if (fread(&y_scale, sizeof(y_scale), 1, fp) != 1) {
			return false;
		}

		if (fread(&z_scale, sizeof(z_scale), 1, fp) != 1) {
			return false;
		}

		if (fread(&x_extent, sizeof(x_extent), 1, fp) != 1) {
			return false;
		}

		if (fread(&y_extent, sizeof(y_extent), 1, fp) != 1) {
			return false;
		}

		if (fread(&z_extent, sizeof(z_extent), 1, fp) != 1) {
			return false;
		}

		regions.push_back(std::make_pair((WaterRegionType)region_type,
			OrientedBoundingBox(glm::vec3(x, y, z), glm::vec3(x_rot, y_rot, z_rot), glm::vec3(x_scale, y_scale, z_scale), glm::vec3(x_extent, y_extent, z_extent))));
	}

	return true;
}
