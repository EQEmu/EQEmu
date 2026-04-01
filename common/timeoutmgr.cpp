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
#include "timeoutmgr.h"

//#define TIMEOUT_DEBUG

Timeoutable::Timeoutable(uint32 check_frequency)
 : next_check(check_frequency)
{
	timeout_manager.AddMember(this);
}

Timeoutable::~Timeoutable() {
	timeout_manager.DeleteMember(this);
}


TimeoutManager::TimeoutManager() {
}

void TimeoutManager::CheckTimeouts() {
	std::vector<Timeoutable *>::iterator cur,end;
	cur = members.begin();
	end = members.end();
	for(; cur != end; ++cur) {
		Timeoutable *it = *cur;
		if(it->next_check.Check()) {
			it->CheckTimeout();
		}
	}
}

//methods called by Timeoutable objects:
void TimeoutManager::AddMember(Timeoutable *who) {
	if(who == nullptr)
		return;

	DeleteMember(who);	//just in case... prolly not needed.
	members.push_back(who);
}

void TimeoutManager::DeleteMember(Timeoutable *who) {
	std::vector<Timeoutable *>::iterator cur,end;
	cur = members.begin();
	end = members.end();
	for(; cur != end; ++cur) {
		if(*cur == who) {
			members.erase(cur);
			return;
		}
	}
}




