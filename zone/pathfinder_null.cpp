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
#include "pathfinder_null.h"

IPathfinder::IPath PathfinderNull::FindRoute(const glm::vec3 &start, const glm::vec3 &end, bool &partial, bool &stuck, int flags)
{
	partial = false;
	stuck = false;
	IPath ret;
	ret.push_back(start);
	ret.push_back(end);
	return ret;
}

IPathfinder::IPath PathfinderNull::FindPath(const glm::vec3 &start, const glm::vec3 &end, bool &partial, bool &stuck, const PathfinderOptions &opts)
{
	partial = false;
	stuck = false;
	IPath ret;
	ret.push_back(start);
	ret.push_back(end);
	return ret;
}

glm::vec3 PathfinderNull::GetRandomLocation(const glm::vec3 &start, int flags)
{
	return glm::vec3(0.0f);
}
