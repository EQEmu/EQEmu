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

#include "pathfinder_interface.h"

struct PathFileHeader;
struct Node;

class PathfinderWaypoint : public IPathfinder
{
public:
	PathfinderWaypoint(const std::string &path);
	virtual ~PathfinderWaypoint();

	virtual IPath FindRoute(const glm::vec3 &start, const glm::vec3 &end, bool &partial, bool &stuck, int flags = PathingNotDisabled);
	virtual glm::vec3 GetRandomLocation(const glm::vec3 &start, int flags = PathingNotDisabled);
	virtual void DebugCommand(Client *c, const Seperator *sep);

private:
	void Load(const std::string &filename);
	void LoadV2(FILE *f, const PathFileHeader &header);
	void LoadV3(FILE *f, const PathFileHeader &header);
	void ShowNodes();
	void ShowPath(Client *c, const glm::vec3 &start, const glm::vec3 &end);
	void NodeInfo(Client *c);
	Node *FindPathNodeByCoordinates(float x, float y, float z);
	void BuildGraph();
	void ShowNode(const Node &n);

	struct Implementation;
	std::unique_ptr<Implementation> m_impl;
};
