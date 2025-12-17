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

#include "timeoutmgr.h"

#include "common/global_define.h"

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




