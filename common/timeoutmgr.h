/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2005 EQEMu Development Team (http://eqemulator.net)

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
