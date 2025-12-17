/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemu.org)

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

enum {	//raid packet types:
	raidAdd = 0,
	raidRemove2 = 1,	//parameter=0
	raidMemberNameChange = 2,
	raidRemove1 = 3,	//parameter=0xFFFFFFFF
	raidNoLongerLeader = 4,
	raidDisband = 5,
	raidMembers = 6,	//len 395+, details + members list
	raidNoAssignLeadership = 7,
	raidCreate = 8,		//len 72
	raidUnknown = 9, // unused?
	raidNoRaid = 10,		//parameter=0
	raidChangeLootType = 11,
	raidStringID = 12,
	raidChangeGroupLeader = 13,	//136 raid leader, new group leader, group_id?
	raidSetLeaderAbilities = 14,	//472
	raidSetLeaderData = 15,	// 14,15 SoE names, not sure on difference, 14 packet has 0x100 bytes 15 0x214 in addition to raid general
	raidChangeGroup = 16,	//?? len 136 old leader, new leader, 0 (preceeded with a remove2)
	raidLock = 17,		//len 136 leader?, leader, 0
	raidUnlock = 18,		//len 136 leader?, leader, 0
	raidRedStringID = 19,
	raidSetLeader = 20,	//len 388, contains 'details' struct without members; also used for "invite to raid"
	raidMakeLeader = 30,
	raidSetMotd = 35,
	raidSetNote = 36,
};


enum { //raid command types
	RaidCommandInviteIntoExisting = 0, //in use
	RaidCommandAcceptInvite = 1, //in use
	RaidCommandInvite = 3, //in use
	RaidCommandDisband = 5, //in use
	RaidCommandMoveGroup = 6, //in use
	RaidCommandRemoveGroupLeader = 7,
	RaidCommandRaidLock = 8, //in use
	RaidCommandRaidUnlock = 9, //in use
	RaidCommandLootType = 20, //in use
	RaidCommandAddLooter = 21, //in use
	RaidCommandRemoveLooter = 22, //in use
	RaidCommandMakeLeader = 30,
	RaidCommandInviteFail = 31, //already in raid, waiting on invite from other raid, etc
	RaidCommandLootType2 = 32, //in use
	RaidCommandAddLooter2 = 33, //in use
	RaidCommandRemoveLooter2 = 34, //in use
	RaidCommandSetMotd = 35,
	RaidCommandSetNote = 36,
};
