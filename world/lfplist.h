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

#include "common/eq_packet_structs.h"
#include "common/linked_list.h"
#include "common/servertalk.h"
#include "common/timer.h"

class GroupLFP {

public:
	GroupLFP(uint32 LeaderID);

	void	SetDetails(ServerLFPUpdate_Struct *Update);
	inline	GroupLFPMemberEntry* GetMembers() { return Members; }
	inline	uint32 GetID() { return LeaderID; }
	void	RemoveMember(int Index);

	friend class GroupLFPList;

private:
	uint32	LeaderID;
	uint8	MatchFilter;
	uint32	FromLevel;
	uint32	ToLevel;
	uint32	Classes;
	char	Comments[64];
	GroupLFPMemberEntry Members[MAX_GROUP_MEMBERS];

};

class GroupLFPList {

public:
	GroupLFPList();
	void UpdateGroup(ServerLFPUpdate_Struct *Update);
	void RemoveGroup(ServerLFPUpdate_Struct *Update);
	void SendLFPMatches(ServerLFPMatchesRequest_Struct* smrs);
	void Process();

private:
	LinkedList<GroupLFP*> LFPGroupList;
	Timer LFPStaleTimer;
};
