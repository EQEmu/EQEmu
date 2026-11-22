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

// Interface derived from John W. Ratcliff's 2011 MIT-licensed raycastmesh.
// Query interface owned by Map and implemented by the Rust adapter.
typedef float RmReal;

class RaycastMesh
{
public:
	virtual bool raycast(const RmReal *from,const RmReal *to,RmReal *hitLocation,RmReal *hitNormal,RmReal *hitDistance) = 0;
	virtual bool bruteForceRaycast(const RmReal *from,const RmReal *to,RmReal *hitLocation,RmReal *hitNormal,RmReal *hitDistance) = 0;

	virtual const RmReal * getBoundMin(void) const = 0; // return the minimum bounding box
	virtual const RmReal * getBoundMax(void) const = 0; // return the maximum bounding box.
	virtual void release(void) = 0;
protected:
	virtual ~RaycastMesh(void) { };
};
