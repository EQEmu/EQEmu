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

#include "common/timer.h"
#include "common/types.h"

#include <vector>

//ms between checking all timeouts
//timeouts are generally somewhat large, so its safe to use a
//value on the order of seconds here.
#define TIMEOUT_GRANULARITY 1000

//timeoutable objects automatically register themselves
//with the global TimeoutManager object
class TimeoutManager;
class Timeoutable {
	friend class TimeoutManager;
public:
	//this frequency should generally be a multiple of TIMEOUT_GRANULARITY
	Timeoutable(uint32 check_frequency);
	virtual ~Timeoutable();

	virtual void CheckTimeout() = 0;

private:
	//accessed directly by TimeoutManager
	Timer next_check;
};

class TimeoutManager {
	friend class Timeoutable;
public:
	TimeoutManager();

	void CheckTimeouts();

protected:

	//methods called by Timeoutable objects:
	void AddMember(Timeoutable *who);
	void DeleteMember(Timeoutable *who);

	std::vector<Timeoutable *> members;
};

extern TimeoutManager timeout_manager;
