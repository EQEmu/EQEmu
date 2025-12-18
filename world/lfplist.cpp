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

#include "lfplist.h"

#include "common/classes.h"
#include "common/misc_functions.h"
#include "world/cliententry.h"
#include "world/clientlist.h"
#include "world/zonelist.h"
#include "world/zoneserver.h"

GroupLFP::GroupLFP(uint32 inLeaderID) {

	LeaderID = inLeaderID;
	for (auto &member : Members) {
		member.Name[0] = '\0';
		member.Class = Class::None;
		member.Level = 0;
		member.Zone = 0;
	}
	FromLevel = 1;
	ToLevel = 100;
	Classes = 0x01FFFE;
	Comments[0] = '\0';
}

void GroupLFP::SetDetails(ServerLFPUpdate_Struct *Update) {

	ClientListEntry *CLE;

	// Update->Action can be 1 or 255.
	// If it is 255, we update the members only, not the level/class filters.
	//
	if(Update->Action != 255) {
		MatchFilter = Update->MatchFilter;
		FromLevel = Update->FromLevel;
		ToLevel = Update->ToLevel;
		Classes = Update->Classes;
		strcpy(Comments, Update->Comments);
	}

	for(unsigned int i=0; i<MAX_GROUP_MEMBERS; i++) {
		strcpy(Members[i].Name,Update->Members[i].Name);
		// If we were passed the class/level and zone information, use that.
		if(Update->Members[i].Class && Update->Members[i].Level && Update->Members[i].Zone) {
			Members[i].Class = Update->Members[i].Class;
			Members[i].Level = Update->Members[i].Level;
			Members[i].Zone = Update->Members[i].Zone;
			Members[i].GuildID = Update->Members[i].GuildID;
		}
		// Otherwise try and find the information ourselves.
		else {
			CLE = ClientList::Instance()->FindCharacter(Members[i].Name);
			if(CLE) {
				Members[i].Class = CLE->class_();
				Members[i].Level = CLE->level();
				Members[i].Zone = CLE->zone();
				Members[i].GuildID = CLE->GuildID();
			}
			else {
				Members[i].Class = Class::None;
				Members[i].Level = 0;
				Members[i].Zone = 0;
				Members[i].GuildID = 0xFFFF;
			}
		}
	}
}

void GroupLFP::RemoveMember(int Index) {

	Members[Index].Name[0] = '\0';
}

GroupLFPList::GroupLFPList() : LFPStaleTimer(60000) {

}

void GroupLFPList::Process() {

	// Once a minute, check for clients in a LFP group who are no longer connected, and remove them.
	// If the client that posted the LFP group has gone, remove the entire LFP entry.
	//
	// We also update the level, class and zone for each member of the group. Their class will usually
	// never change, but if the LFP group is posted while a member is zoning, it will initially be
	// 'Unknown Class', so we will fill it in here.

	if(!LFPStaleTimer.Check())
		return;

	GroupLFP* Group;

	LinkedListIterator<GroupLFP*> Iterator(LFPGroupList);
	Iterator.Reset();

	while(Iterator.MoreElements()) {
		Group = Iterator.GetData();
		int MemberCount = 0;
		if(Group) {
			GroupLFPMemberEntry* GroupMembers = Group->Members;
			if(!GroupMembers) {
				Iterator.Advance();
				continue;
			}
			for(unsigned int i=0; i<MAX_GROUP_MEMBERS; i++) {
				if(GroupMembers[i].Name[0] == '\0')
					continue;

				ClientListEntry *CLE = ClientList::Instance()->FindCharacter(GroupMembers[i].Name);
				if(!CLE) {
					// The first member entry is always the person who posted the LFP group, either
					// a single ungrouped player, or the leader of the group. If (s)he is gone, remove
					// the group from LFP.
					if(i==0) break;
					Group->RemoveMember(i);
				}
				else {
					// If the level/class/zone information we have is valid, update
					// the player's information.
					//
					if(CLE->level() > 0)
						GroupMembers[i].Level = CLE->level();

					if(CLE->class_() > 0)
						GroupMembers[i].Class = CLE->class_();

					if(CLE->zone() > 0)
						GroupMembers[i].Zone = CLE->zone();

					MemberCount++;
				}
			}
			if(MemberCount == 0) {
				// If the leader or all the members are not online, remove the entry.
				Iterator.RemoveCurrent();
				continue;
			}
		}
		Iterator.Advance();
	}
}

void GroupLFPList::RemoveGroup(ServerLFPUpdate_Struct *Update) {

	GroupLFP* Group;

	LinkedListIterator<GroupLFP*> Iterator(LFPGroupList);
	Iterator.Reset();

	while(Iterator.MoreElements()) {
		Group = Iterator.GetData();
		if(Group && (Group->GetID() == Update->LeaderID)) {
			Iterator.RemoveCurrent();
			return;
		}
		Iterator.Advance();
	}
}

void GroupLFPList::UpdateGroup(ServerLFPUpdate_Struct *Update) {

	GroupLFP* Group;

	LinkedListIterator<GroupLFP*> Iterator(LFPGroupList);
	Iterator.Reset();

	while(Iterator.MoreElements()) {
		Group = Iterator.GetData();
		if(Group && (Group->GetID() == Update->LeaderID)) {
			Group->SetDetails(Update);
			return;
		}
		Iterator.Advance();
	}

	Group = new GroupLFP(Update->LeaderID);

	if(Group) {
		Group->SetDetails(Update);
		LFPGroupList.Append(Group);
	}

}

void GroupLFPList::SendLFPMatches(ServerLFPMatchesRequest_Struct* smrs) {

	int Matches = 0;
	GroupLFP* Group;

	LinkedListIterator<GroupLFP*> Iterator(LFPGroupList);
	Iterator.Reset();

	while(Iterator.MoreElements()) {
		Group = Iterator.GetData();
		Iterator.Advance();

		if(Group) {
			// Just check if the leader is within the requested level range.
			if((Group->Members[0].Level < smrs->FromLevel) || (Group->Members[0].Level > smrs->ToLevel))
				continue;

			// If the Player putting up the LFP request specified MatchFilter = true, then anyone
			// searching for groups LFP must meet the specified criteria to see this group.
			if(Group->MatchFilter) {
				unsigned int BitMask = 1 << smrs->QuerierClass;
				if(!(BitMask & Group->Classes)) continue;
				if(!((smrs->QuerierLevel >= Group->FromLevel) && (smrs->QuerierLevel <= Group->ToLevel))) continue;
			}
			Matches++;
		}
	}

	auto Pack = new ServerPacket(ServerOP_LFPMatches, (sizeof(ServerLFPMatchesResponse_Struct) * Matches) + 4);

	char *Buf = (char *)Pack->pBuffer;

	VARSTRUCT_ENCODE_TYPE(uint32, Buf, smrs->FromID);

	ServerLFPMatchesResponse_Struct* Buffer = (ServerLFPMatchesResponse_Struct*)Buf;

	Iterator.Reset();

	if(Matches) {
		while(Iterator.MoreElements() && (Matches > 0)) {
			Group = Iterator.GetData();
			Iterator.Advance();

			if(Group) {
				if((Group->Members[0].Level < smrs->FromLevel) || (Group->Members[0].Level > smrs->ToLevel))
					continue;

				if(Group->MatchFilter) {
					unsigned int BitMask = 1 << smrs->QuerierClass;
					if(!(BitMask & Group->Classes)) continue;
					if(!((smrs->QuerierLevel >= Group->FromLevel) && (smrs->QuerierLevel <= Group->ToLevel))) continue;
				}
				Buffer->FromLevel = Group->FromLevel;
				Buffer->ToLevel = Group->ToLevel;
				Buffer->Classes = Group->Classes;
				memcpy(Buffer->Members, Group->Members, 64 * MAX_GROUP_MEMBERS);
				strcpy(Buffer->Comments, Group->Comments);
				Matches--;
				Buffer++;
			}
		}
	}

	ClientListEntry* CLE = ClientList::Instance()->FindCharacter(smrs->FromName);
	if (CLE != nullptr) {
		if (CLE->Server() != nullptr)
			CLE->Server()->SendPacket(Pack);
	}
	else {
		ZoneServer* zs = ZSList::Instance()->FindByName(smrs->FromName);
		if (zs != nullptr)
			zs->SendPacket(Pack);
	}
	safe_delete(Pack);
}

