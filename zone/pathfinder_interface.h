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

#include "zone/map.h"

#include <list>

class Client;
class Seperator;

enum PathingPolyFlags
{
	PathingNormal = 1,
	PathingWater = 2,
	PathingLava = 4,
	PathingZoneLine = 8,
	PathingPvP = 16,
	PathingSlime = 32,
	PathingIce = 64,
	PathingVWater = 128,
	PathingGeneralArea = 256,
	PathingPortal = 512,
	PathingPrefer = 1024,
	PathingDisabled = 2048,
	PathingAll = 65535,
	PathingNotDisabled = PathingAll ^ PathingDisabled
};

struct PathfinderOptions
{
	PathfinderOptions() {
		flags = PathingNotDisabled;
		smooth_path = true;
		step_size = 10.0f;
		flag_cost[0] = 1.0f;
		flag_cost[1] = 3.0f;
		flag_cost[2] = 5.0f;
		flag_cost[3] = 1.0f;
		flag_cost[4] = 2.0f;
		flag_cost[5] = 2.0f;
		flag_cost[6] = 4.0f;
		flag_cost[7] = 1.0f;
		flag_cost[8] = 0.1f;
		flag_cost[9] = 0.1f;
		offset = 3.25f;
	}

	int flags;
	bool smooth_path;
	float step_size;
	float flag_cost[10];
	float offset;
};

class IPathfinder
{
public:
	struct IPathNode
	{
		IPathNode(const glm::vec3 &p) {
			pos = p;
			teleport = false;
		}

		IPathNode(bool tp) {
			teleport = tp;
		}

		glm::vec3 pos;
		bool teleport;
	};

	typedef std::list<IPathNode> IPath;

	IPathfinder() { }
	virtual ~IPathfinder() { }

	virtual IPath FindRoute(const glm::vec3 &start, const glm::vec3 &end, bool &partial, bool &stuck, int flags = PathingNotDisabled) = 0;
	virtual IPath FindPath(const glm::vec3 &start, const glm::vec3 &end, bool &partial, bool &stuck, const PathfinderOptions& opts) = 0;
	virtual glm::vec3 GetRandomLocation(const glm::vec3 &start, int flags = PathingNotDisabled) = 0;
	virtual void DebugCommand(Client *c, const Seperator *sep) = 0;

	static IPathfinder *Load(const std::string &zone);
};
