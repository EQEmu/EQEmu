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

#include "common/types.h"
#include "zone/groups.h"
#include "zone/xtargetautohaters.h"
#include "zone/client.h"

class Client;
class EQApplicationPacket;
class Mob;

enum {
	FindNextMarkerSlot     = 1,
	FindNextAssisterSlot   = 2,
	RaidDelegateMainAssist = 3,
	RaidDelegateMainMarker = 4
};

typedef enum {
	MAIN_ASSIST_1_SLOT = 0,
	MAIN_ASSIST_2_SLOT = 1,
	MAIN_ASSIST_3_SLOT = 2,
	MAIN_ASSIST_1 = 1,
	MAIN_ASSIST_2 = 2,
	MAIN_ASSIST_3 = 3,
} MainAssistType;

typedef enum {
	MAIN_MARKER_1_SLOT = 0,
	MAIN_MARKER_2_SLOT = 1,
	MAIN_MARKER_3_SLOT = 2,
	MAIN_MARKER_1 = 1,
	MAIN_MARKER_2 = 2,
	MAIN_MARKER_3 = 3,
} MainMarkerType;

enum {
	ClearDelegate = 1,
	SetDelegate = 0,
	FindNextRaidMainMarkerSlot = 1,
	FindNextRaidMainAssisterSlot = 2,
	DELEGATE_OFF = 0,
	DELEGATE_ON  = 1
};

struct Raid_Marked_NPC {
	uint32	entity_id;
	uint32	zone_id;
	uint32	instance_id;
};


constexpr uint8_t MAX_RAID_GROUPS = 12;
constexpr uint8_t MAX_RAID_MEMBERS = 72;
const uint32 RAID_GROUPLESS = 0xFFFFFFFF;
#define MAX_NO_RAID_MAIN_ASSISTERS 3
#define MAX_NO_RAID_MAIN_MARKERS 3

struct RaidMember{
	char member_name[64]{ 0 };
	Client* member{ nullptr };
	uint32 group_number{ RAID_GROUPLESS };
	uint8 _class{ 0 };
	uint8 level{ 0 };
	std::string note{};
	bool is_group_leader{ false };
	bool is_raid_leader{ false };
	bool is_looter{ false };
	uint8 main_marker{ 0 };
	uint8 main_assister{ 0 };
	bool is_bot{ false };
	bool is_raid_main_assist_one{false};
};

struct GroupMentor {
	std::string name;
	Client *mentoree;
	int mentor_percent;
};

class Raid : public GroupIDConsumer {
public:
	Raid(Client *nLeader);
	Raid(uint32 raidID);
	~Raid();

	void SetLeader(Client* c) { leader = c; }
	Client* GetLeader() { return leader; }
	std::string GetLeaderName() { return leadername; }
	bool IsLeader(Client* c) { return c == leader; }
	bool IsLeader(const char* name) { return !strcmp(leadername, name); }
	void SetRaidLeader(const char *wasLead, const char *name);

	bool	Process();

	void	AddMember(Client *c, uint32 group = 0xFFFFFFFF, bool rleader=false, bool groupleader=false, bool looter=false);
	void	AddBot(Bot* b, uint32 group = 0xFFFFFFFF, bool raid_leader=false, bool group_leader=false, bool looter=false);
	void	RaidGroupSay(const char* msg, const char* from, uint8 language, uint8 lang_skill);
	void	RaidSay(const char* msg, const char* from, uint8 language, uint8 lang_skill);
	bool	IsEngaged();
	Mob*	GetRaidMainAssistOne();
	void	RemoveMember(const char *character_name);
	void	DisbandRaid();
	void	MoveMember(const char *name, uint32 newGroup);
	void	SetGroupLeader(const char *who, bool glFlag = true);
	Client	*GetGroupLeader(uint32 group_id);
	void	RemoveGroupLeader(const char *who);
	bool	IsGroupLeader(const char* name);
	bool	IsGroupLeader(Client *c);
	bool	IsRaidMember(const char* name);
	bool	IsRaidMember(Client *c);
	void	UpdateLevel(const char *name, int newLevel);
	void	SetNewRaidLeader(uint32 i);
	bool    IsAssister(const char* who);
	bool    IsMarker(const char* who);
	void    EmptyRaidMembers();

	uint32	GetFreeGroup();
	uint8	GroupCount(uint32 gid);
	uint8	RaidCount();
	uint32	GetHighestLevel();
	uint32	GetLowestLevel();
	uint32	GetGroup(const char *name);
	uint32	GetGroup(Client *c);
	uint16	GetAvgLevel();

	uint32	GetLootType() { return LootType; }
	void	ChangeLootType(uint32 type);
	void	AddRaidLooter(const char* looter);
	void	RemoveRaidLooter(const char* looter);

	inline void	SetRaidMOTD(const std::string& in_motd) { motd = in_motd; };

	//util func
	//keeps me from having to keep iterating through the list
	//when I want lots of data from the same entry
	uint32	GetPlayerIndex(const char *name);
	uint32	GetPlayerIndex(Client *c);
	//for perl interface
	Client *GetClientByIndex(uint16 index);
	const char *GetClientNameByIndex(uint8 index);

	void	LockRaid(bool lockFlag);
	bool	IsLocked() { return locked; }

	//Actual Implementation Stuff

	void	RaidMessageString(Mob* sender, uint32 type, uint32 string_id, const char* message,const char* message2=0,const char* message3=0,const char* message4=0,const char* message5=0,const char* message6=0,const char* message7=0,const char* message8=0,const char* message9=0, uint32 distance = 0);
	void	CastGroupSpell(Mob* caster,uint16 spellid, uint32 gid);
	void	SplitExp(ExpSource exp_source, const uint64 exp, Mob* other);
	uint32	GetTotalRaidDamage(Mob* other);
	void	BalanceHP(int32 penalty, uint32 group_id, float range = 0, Mob* caster = nullptr, int32 limit = 0);
	void	BalanceMana(int32 penalty, uint32 group_id, float range = 0, Mob* caster = nullptr, int32 limit = 0);
	void	HealGroup(uint32 heal_amount, Mob* caster, uint32 group_id, float range = 0);
	void	SplitMoney(uint32 gid, uint32 copper, uint32 silver, uint32 gold, uint32 platinum, Client *splitter = nullptr);

	void	TeleportGroup(Mob* sender, uint32 zoneID, uint16 instance_id, float x, float y, float z, float heading, uint32 gid);
	void	TeleportRaid(Mob* sender, uint32 zoneID, uint16 instance_id, float x, float y, float z, float heading);

	//updates the list of Client* objects based on who's in and not in the zone.
	//also learns raid structure based on db.
	void	SetRaidDetails();
	void	GetRaidDetails();
	void	SaveRaidMOTD();
	bool	LearnMembers();
	void	VerifyRaid();
	void	MemberZoned(Client *c);
	void	SendHPManaEndPacketsTo(Client *c);
	void	SendHPManaEndPacketsFrom(Mob *mob);
	void	SendManaPacketFrom(Mob *mob);
	void	SendEndurancePacketFrom(Mob *mob);
	void	RaidSay(const char *msg, Client *c, uint8 language, uint8 lang_skill);
	void	RaidGroupSay(const char *msg, Client *c, uint8 language, uint8 lang_skill);
	void    SaveRaidNote(std::string who, std::string note);
	std::vector<RaidMember> GetMembersWithNotes();
	void	DelegateAbilityAssist(Mob* mob, const char* who);
	void	DelegateAbilityMark(Mob* mob, const char* who);
	void    RaidMarkNPC(Mob* mob, uint32 parameter);
	void    UpdateXTargetType(XTargetType Type, Mob* m, const char* name = (const char*)nullptr);
	int     FindNextRaidDelegateSlot(int option);
	void    UpdateXtargetMarkedNPC();
	void    RaidClearNPCMarks(Client* c);
	void    RemoveRaidDelegates(const char* delegatee);
	void	UpdateRaidXTargets();

	//Packet Functions
	void	SendRaidCreate(Client *to);
	void	SendRaidAdd(const char *who, Client *to);
	void	SendRaidAddAll(const char *who);
	void	SendRaidRemove(const char *who, Client *to);
	void	SendRaidRemoveAll(const char *who);
	void	SendRaidDisband(Client *to);
	void	SendRaidDisbandAll();
	void	SendRaidMove(const char* who, Client *to);
	void	SendRaidMoveAll(const char* who);
	void	SendBulkRaid(Client *to);
	void    SendRaidNotes();
	void    SendRaidNotesToWorld();
	void    SendRemoveRaidXTargets(XTargetType Type);
	void    SendRemoveAllRaidXTargets();
	void    SendRemoveAllRaidXTargets(const char* client_name);
	void    SendRaidAssistTarget();
	void    SendAssistTarget(Client* c);
	void	GroupUpdate(uint32 gid, bool initial = true);
	void	SendGroupUpdate(Client *to);
	void	SendGroupDisband(Client *to);
	void	SendRaidLock();
	void	SendRaidUnlock();
	void	SendRaidLockTo(Client *c);
	void	SendRaidUnlockTo(Client *c);
	void	SendRaidGroupAdd(const char *who, uint32 gid);
	void	SendRaidGroupRemove(const char *who, uint32 gid);
	void	SendMakeLeaderPacket(const char *who); //30
	void	SendMakeLeaderPacketTo(const char *who, Client *to);
	void	SendMakeGroupLeaderPacketAll();
	void	SendMakeGroupLeaderPacket(const char *who); //13
	void	SendMakeGroupLeaderPacketTo(const char *who, Client *to);
	void	SendRaidMOTD(Client *c);
	void	SendRaidMOTD();
	void	SendRaidMOTDToWorld();
	void    SendRaidAssisterTo(const char* assister, Client* to);
	void    SendRaidAssister(const char* assister);
	void    SendRaidMarkerTo(const char* marker, Client* to);
	void    SendRaidMarker(const char* marker);
	void    SendMarkTargets(Client* c);

	void	QueuePacket(const EQApplicationPacket *app, bool ack_req = true);

	// Leadership
	void	UpdateGroupAAs(uint32 gid);
	void	SaveGroupLeaderAA(uint32 gid);
	void	UpdateRaidAAs();
	void	SaveRaidLeaderAA();
	void	SendGroupLeadershipAA(Client *c, uint32 gid);
	void	SendGroupLeadershipAA(uint32 gid);
	void	SendAllRaidLeadershipAA();
	void	LoadLeadership();
	inline int GetLeadershipAA(int AAID, uint32 gid = 0)
		{ if (AAID >= 16) return raid_aa.ranks[AAID - 16]; else return group_aa[gid].ranks[AAID]; }
	inline void SetGroupAAs(uint32 gid, GroupLeadershipAA_Struct *glaa)
		{ memcpy(&group_aa[gid], glaa, sizeof(GroupLeadershipAA_Struct)); }
	inline void SetRaidAAs(RaidLeadershipAA_Struct *rlaa)
		{ memcpy(&raid_aa, rlaa, sizeof(RaidLeadershipAA_Struct)); }

	void	SetGroupMentor(uint32 group_id, int percent, char *name);
	void	ClearGroupMentor(uint32 group_id);
	void	CheckGroupMentor(uint32 group_id, Client *c); // this just checks if we should be fixing the pointer in group mentor struct on zone
	inline int GetMentorPercent(uint32 group_id) { return group_mentor[group_id].mentor_percent; }
	inline Client *GetMentoree(uint32 group_id) { return group_mentor[group_id].mentoree; }

	void SetDirtyAutoHaters();
	inline XTargetAutoHaters *GetXTargetAutoMgr() { return &m_autohatermgr; }

	void	QueueClients(Mob *sender, const EQApplicationPacket *app, bool ack_required = true, bool ignore_sender = true, float distance = 0, bool group_only = true);

	bool AnyMemberHasDzLockout(const std::string& expedition, const std::string& event);

	std::vector<RaidMember> GetMembers() const;
	std::vector<RaidMember> GetRaidGroupMembers(uint32 gid);
	std::vector<Bot*> GetRaidGroupBotMembers(uint32 gid);
	std::vector<Bot*> GetRaidBotMembers(uint32 owner = 0);
	void HandleBotGroupDisband(uint32 owner, uint32 gid = RAID_GROUPLESS);
	void HandleOfflineBots(uint32 owner);

	RaidMember members[MAX_RAID_MEMBERS];
	char leadername[64];
	char main_assister_pcs[MAX_NO_RAID_MAIN_ASSISTERS][64];
	char main_marker_pcs[MAX_NO_RAID_MAIN_MARKERS][64];
	Raid_Marked_NPC	marked_npcs[MAX_MARKED_NPCS];
protected:
	Client *leader;
	bool locked;
	uint32 LootType;
	bool disbandCheck;
	bool forceDisband;
	std::string motd;
	RaidLeadershipAA_Struct raid_aa{};
	GroupLeadershipAA_Struct group_aa[MAX_RAID_GROUPS]{};

	GroupMentor group_mentor[MAX_RAID_GROUPS];

	XTargetAutoHaters m_autohatermgr;
};
