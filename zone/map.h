/*
	EQEMu:  Everquest Server Emulator

	Copyright (C) 2001-2014 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#pragma once

#include "zone/position.h"
#include "zone/zone_config.h"

#include <cstdio>

#define BEST_Z_INVALID -99999

extern const ZoneConfig *Config;

class Map
{
public:
	Map();
	~Map();

	float FindBestZ(glm::vec3 &start, glm::vec3 *result) const;
	float FindClosestZ(glm::vec3 &start, glm::vec3 *result) const;
	float FindCeiling(glm::vec3 &start, glm::vec3 *result) const;
	float FindGround(glm::vec3 &start, glm::vec3 *result) const;
	bool LineIntersectsZone(glm::vec3 start, glm::vec3 end, float step, glm::vec3 *result) const;
	bool LineIntersectsZoneNoZLeaps(glm::vec3 start, glm::vec3 end, float step_mag, glm::vec3 *result) const;
	bool CheckLoS(glm::vec3 myloc, glm::vec3 oloc) const;
	bool DoCollisionCheck(glm::vec3 myloc, glm::vec3 oloc, glm::vec3 &outnorm, float &distance) const;

#ifdef USE_MAP_MMFS
	bool Load(std::string filename, bool force_mmf_overwrite = false);
#else
	bool Load(const std::string& filename);
#endif

	static Map *LoadMapFile(std::string file);
private:
	void RotateVertex(glm::vec3 &v, float rx, float ry, float rz);
	void ScaleVertex(glm::vec3 &v, float sx, float sy, float sz);
	void TranslateVertex(glm::vec3 &v, float tx, float ty, float tz);
	bool LoadV1(FILE *f);
	bool LoadV2(FILE *f);

#ifdef USE_MAP_MMFS
	bool LoadMMF(const std::string& map_file_name, bool force_mmf_overwrite);
	bool SaveMMF(const std::string& map_file_name, bool force_mmf_overwrite);
#endif /*USE_MAP_MMFS*/

	struct impl;
	impl *imp;
};
