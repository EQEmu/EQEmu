/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#pragma once

#include "common/types.h"

#include "glm/geometric.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include <string>

std::string to_string(const glm::vec4 &position);
std::string to_string(const glm::vec3 &position);
std::string to_string(const glm::vec2 &position);

bool IsWithinAxisAlignedBox(const glm::vec3 &position, const glm::vec3 &minimum, const glm::vec3 &maximum);
bool IsWithinAxisAlignedBox(const glm::vec2 &position, const glm::vec2 &minimum, const glm::vec2 &maximum);

bool IsOrigin(const glm::vec3 &position);
bool IsOrigin(const glm::vec4 &position);

float DistanceSquared(const glm::vec2& point1, const glm::vec2& point2);
float Distance(const glm::vec2& point1, const glm::vec2& point2);
float DistanceSquared(const glm::vec3& point1, const glm::vec3& point2);
float Distance(const glm::vec3& point1, const glm::vec3& point2);
float DistanceNoZ(const glm::vec3& point1, const glm::vec3& point2);
float DistanceSquaredNoZ(const glm::vec3& point1, const glm::vec3& point2);

float DistanceSquared(const glm::vec4& point1, const glm::vec4& point2);
float Distance(const glm::vec4& point1, const glm::vec4& point2);
float DistanceNoZ(const glm::vec4& point1, const glm::vec4& point2);
float DistanceSquaredNoZ(const glm::vec4& point1, const glm::vec4& point2);

float GetReciprocalHeading(const glm::vec4& point1);
float GetReciprocalHeading(const float heading);

bool IsHeadingEqual(const float h1, const float h2);

bool IsPositionEqual(const glm::vec2 &p1, const glm::vec2 &p2);
bool IsPositionEqual(const glm::vec3 &p1, const glm::vec3 &p2);
bool IsPositionEqual(const glm::vec4 &p1, const glm::vec4 &p2);
bool IsPositionEqualWithinCertainZ(const glm::vec3 &p1, const glm::vec3 &p2, float z_eps);
bool IsPositionEqualWithinCertainZ(const glm::vec4 &p1, const glm::vec4 &p2, float z_eps);

bool IsPositionWithinSimpleCylinder(const glm::vec3 &p1, const glm::vec3 &cylinder_center, float cylinder_radius, float cylinder_height);
bool IsPositionWithinSimpleCylinder(const glm::vec4 &p1, const glm::vec4 &cylinder_center, float cylinder_radius, float cylinder_height);

float CalculateHeadingAngleBetweenPositions(float x1, float y1, float x2, float y2);

bool IsWithinCircularArc(glm::vec4 arc_center, glm::vec4 point, uint32 arc_offset, uint32 arc_radius, uint32 arc_radius_limit);
bool IsWithinSquare(glm::vec4 center, uint32 area, glm::vec4 position);
