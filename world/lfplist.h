/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2008 EQEMu Development Team (http://eqemulator.net)

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
