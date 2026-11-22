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
#include "collision_observation.h"
#include "common/file.h"
#include "common/misc_functions.h"
#include "zone/client.h"
#include "zone/map.h"
#include "zone/map_geometry.h"
#include "zone/raycast_mesh.h"
#include "zone/rust_raycast_mesh.h"
#include "zone/zone.h"

#include <algorithm>
#include <map>
#include <memory>
#include <tuple>
#include <vector>

struct Map::impl
{
	RaycastMesh *rm;
};

Map::Map() {
	imp = nullptr;
}

Map::~Map() {
	if(imp) {
		imp->rm->release();
		safe_delete(imp);
	}
}

float Map::FindBestZ(glm::vec3 &start, glm::vec3 *result) const {
	if (!imp) {
		return BEST_Z_INVALID;
	}

	glm::vec3 tmp;
	if (!result) {
		result = &tmp;
	}

	start.z += RuleI(Map, FindBestZHeightAdjust);
	glm::vec3 from(start.x, start.y, start.z);
	glm::vec3 to(start.x, start.y, BEST_Z_INVALID);
	float hit_distance;
	bool hit = false;

	hit = imp->rm->raycast((const RmReal*)&from, (const RmReal*)&to, (RmReal*)result, nullptr, &hit_distance);
	if (hit && zone && zone->newzone_data.underworld != 0.0f && result->z < zone->newzone_data.underworld) {
		hit = false;
	}

	if (hit) {
		CollisionObservation::Query("FindBestZ", from, to, result->z);
		return result->z;
	}

	// Find nearest Z above us
	to.z = -BEST_Z_INVALID;
	hit = imp->rm->raycast((const RmReal*)&from, (const RmReal*)&to, (RmReal*)result, nullptr, &hit_distance);
	if (hit && zone && zone->newzone_data.max_z != 0.0f && result->z > zone->newzone_data.max_z) {
		hit = false;
	}

	if (hit) {
		CollisionObservation::Query("FindBestZ", from, to, result->z);
		return result->z;
	}

	CollisionObservation::Query("FindBestZ", from, to, BEST_Z_INVALID);
	return BEST_Z_INVALID;
}

float Map::FindClosestZ(glm::vec3 &start, glm::vec3 *result) const {
	// Unlike FindBestZ, this method finds the closest Z value above or below the specified point.
	//
	if (!imp)
		return false;

	float ClosestZ = BEST_Z_INVALID;

	glm::vec3 tmp;
	if (!result)
		result = &tmp;

	glm::vec3 from(start.x, start.y, start.z);
	glm::vec3 to(start.x, start.y, BEST_Z_INVALID);
	float hit_distance;
	bool hit = false;

	// first check is below us
	hit = imp->rm->raycast((const RmReal*)&from, (const RmReal*)&to, (RmReal*)result, nullptr, &hit_distance);
	if (hit) {
		ClosestZ = result->z;

	}

	// Find nearest Z above us
	to.z = -BEST_Z_INVALID;
	hit = imp->rm->raycast((const RmReal*)&from, (const RmReal*)&to, (RmReal*)result, nullptr, &hit_distance);
	if (hit) {
		if (std::abs(from.z - result->z) < std::abs(ClosestZ - from.z))
			return result->z;
	}

	return ClosestZ;
}

float Map::FindCeiling(glm::vec3 &start, glm::vec3 *result) const {
	// Unlike FindBestZ, this method finds the closest Z above point.

	if (!imp) {
		return false;
	}

	glm::vec3 tmp;
	if (!result) {
		result = &tmp;
	}

	glm::vec3 from(start.x, start.y, start.z);
	glm::vec3 to(start.x, start.y, -BEST_Z_INVALID);
	float hit_distance;
	bool hit = false;

	// Find nearest Z above us
	hit = imp->rm->raycast((const RmReal*)&from, (const RmReal*)&to, (RmReal*)result, nullptr, &hit_distance);
	if (hit) {
		return result->z;
	}

	return BEST_Z_INVALID;
}

float Map::FindGround(glm::vec3 &start, glm::vec3 *result) const {
	// Unlike FindBestZ, this method finds the closest Z below point.

	if (!imp) {
		return false;
	}

	glm::vec3 tmp;

	if (!result) {
		result = &tmp;
	}

	glm::vec3 from(start.x, start.y, start.z);
	glm::vec3 to(start.x, start.y, BEST_Z_INVALID);
	float hit_distance;
	bool hit = false;

	// Find nearest Z below us
	hit = imp->rm->raycast((const RmReal*)&from, (const RmReal*)&to, (RmReal*)result, nullptr, &hit_distance);

	if (hit && zone && zone->newzone_data.underworld != 0.0f && result->z < zone->newzone_data.underworld) {
		hit = false;
	}

	if (hit) {
		return result->z;
	}

	return BEST_Z_INVALID;
}

bool Map::LineIntersectsZone(glm::vec3 start, glm::vec3 end, float step, glm::vec3 *result) const {
	if(!imp)
		return false;
	return imp->rm->raycast((const RmReal*)&start, (const RmReal*)&end, (RmReal*)result, nullptr, nullptr);
}

bool Map::LineIntersectsZoneNoZLeaps(glm::vec3 start, glm::vec3 end, float step_mag, glm::vec3 *result) const {
	if (!imp)
		return false;

	float z = BEST_Z_INVALID;
	glm::vec3 step;
	glm::vec3 cur;
	cur.x = start.x;
	cur.y = start.y;
	cur.z = start.z;

	step.x = end.x - start.x;
	step.y = end.y - start.y;
	step.z = end.z - start.z;
	float factor = step_mag / sqrt(step.x*step.x + step.y*step.y + step.z*step.z);

	step.x *= factor;
	step.y *= factor;
	step.z *= factor;

	int steps = 0;

	if (step.x > 0 && step.x < 0.001f)
		step.x = 0.001f;
	if (step.y > 0 && step.y < 0.001f)
		step.y = 0.001f;
	if (step.z > 0 && step.z < 0.001f)
		step.z = 0.001f;
	if (step.x < 0 && step.x > -0.001f)
		step.x = -0.001f;
	if (step.y < 0 && step.y > -0.001f)
		step.y = -0.001f;
	if (step.z < 0 && step.z > -0.001f)
		step.z = -0.001f;

	//while we are not past end
	//always do this once, even if start == end.
	while(cur.x != end.x || cur.y != end.y || cur.z != end.z)
	{
		steps++;
		glm::vec3 me;
		me.x = cur.x;
		me.y = cur.y;
		me.z = cur.z;
		glm::vec3 hit;

		float best_z = FindBestZ(me, &hit);
		float diff = best_z - z;
		diff = diff < 0 ? -diff : diff;

		if (z <= BEST_Z_INVALID || best_z <= BEST_Z_INVALID || diff < 12.0)
			z = best_z;
		else
			return true;

		//look at current location
		if(LineIntersectsZone(start, end, step_mag, result))
		{
			return true;
		}

		//move 1 step
		if (cur.x != end.x)
			cur.x += step.x;
		if (cur.y != end.y)
			cur.y += step.y;
		if (cur.z != end.z)
			cur.z += step.z;

		//watch for end conditions
		if ( (cur.x > end.x && end.x >= start.x) || (cur.x < end.x && end.x <= start.x) || (step.x == 0) ) {
			cur.x = end.x;
		}
		if ( (cur.y > end.y && end.y >= start.y) || (cur.y < end.y && end.y <= start.y) || (step.y == 0) ) {
			cur.y = end.y;
		}
		if ( (cur.z > end.z && end.z >= start.z) || (cur.z < end.z && end.z < start.z) || (step.z == 0) ) {
			cur.z = end.z;
		}
	}

	//walked entire line and didnt run into anything...
	return false;
}

bool Map::CheckLoS(glm::vec3 myloc, glm::vec3 oloc) const {
	if(!imp)
		return false;

	const bool clear = !imp->rm->raycast((const RmReal*)&myloc, (const RmReal*)&oloc, nullptr, nullptr, nullptr);
	CollisionObservation::Query("CheckLoS", myloc, oloc, clear);
	return clear;
}

// returns true if a collision happens
bool Map::DoCollisionCheck(glm::vec3 myloc, glm::vec3 oloc, glm::vec3 &outnorm, float &distance) const {
	if(!imp)
		return false;

	return imp->rm->raycast((const RmReal*)&myloc, (const RmReal*)&oloc, nullptr, (RmReal *)&outnorm, (RmReal *)&distance);
}

Map *Map::LoadMapFile(std::string file) {
	std::transform(file.begin(), file.end(), file.begin(), ::tolower);
	std::string filename = fmt::format("{}/base/{}.map", PathManager::Instance()->GetMapsPath(), file);

	LogInfo("Attempting to load Map File [{}]", filename.c_str());

	auto m = new Map();
	if (m->Load(filename)) {
		return m;
	}

	delete m;
	return nullptr;
}

bool Map::Load(const std::string &filename)
{

	FILE *map_file = fopen(filename.c_str(), "rb");
	if (map_file) {
		uint32 version;
		if (fread(&version, sizeof(version), 1, map_file) != 1) {
			fclose(map_file);
			return false;
		}

		if (version == 0x01000000) {
			LogInfo("Loaded V1 Map File [{}]", filename.c_str());
			bool loaded_map_file = LoadV1(map_file);
			fclose(map_file);

			if (loaded_map_file) {
				LogInfo("Loaded V1 Map File [{}]", filename.c_str());
			} else {
				LogError("Failed to load V1 Map File [{}]", filename.c_str());
			}

			return loaded_map_file;
		}
		else if (version == 0x02000000) {
			LogInfo("Loading V2 Map File [{}]", filename.c_str());
			bool loaded_map_file = LoadV2(map_file);
			fclose(map_file);

			if (loaded_map_file) {
				LogInfo("Loaded V2 Map File [{}]", filename.c_str());
			} else {
				LogError("Failed to load V2 Map File [{}]", filename.c_str());
			}

			return loaded_map_file;
		}
		else {
			fclose(map_file);
			return false;
		}
	}

	return false;
}

bool Map::LoadV1(FILE *f) {
	std::vector<glm::vec3> verts;
	std::vector<uint32> indices;
	if (!MapGeometry::LoadV1(f, verts, indices)) {
		return false;
	}
	return InstallGeometry(verts, indices);
}

bool Map::LoadV2(FILE *f) {
	std::vector<glm::vec3> verts;
	std::vector<uint32> indices;
	if (!MapGeometry::LoadV2(f, verts, indices)) return false;
	return InstallGeometry(verts, indices);
}

bool Map::InstallGeometry(const std::vector<glm::vec3>& verts, const std::vector<uint32>& indices) {
	RaycastMesh* replacement = nullptr;
	if (verts.size() > 8000000 || indices.size() > 12000000 || indices.size() % 3) {
		LogError("Rust map load failed: invalid geometry counts");
		return false;
	}
	try {
		std::vector<std::array<float,3>> points;
		points.reserve(verts.size());
		for (const auto& v : verts) points.push_back({v.x,v.y,v.z});
		std::string error;
		replacement = createRustRaycastMesh(points, indices, error);
		if (!replacement) LogError("Rust map load failed: [{}]", error);
	} catch (const std::exception& e) {
		LogError("Rust map load failed: [{}]", e.what());
		return false;
	}
	if (!replacement) return false;
	// Allocate the holder before transferring ownership. Failed reload keeps the old engine.
	if (!imp) {
		try { imp = new impl{replacement}; }
		catch (...) { replacement->release(); throw; }
	} else {
		auto* previous = imp->rm;
		imp->rm = replacement;
		previous->release();
	}
	return true;
}
