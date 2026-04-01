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

#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

class OrientedBoundingBox
{
public:
	OrientedBoundingBox() = default;
	OrientedBoundingBox(const glm::vec3 &pos, const glm::vec3 &rot, const glm::vec3 &scale, const glm::vec3 &extents);
	~OrientedBoundingBox() = default;

	bool ContainsPoint(const glm::vec3 &p) const;
private:
	float min_x, max_x;
	float min_y, max_y;
	float min_z, max_z;
	glm::mat4 transformation;
	glm::mat4 inverted_transformation;
};
