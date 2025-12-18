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

#include "common/eq_packet_structs.h"
#include "common/types.h"
#include "zone/mob.h"
#include "zone/xtargetautohaters.h"

class Client;
class EQApplicationPacket;
class Mob;

#define MAX_MARKED_NPCS 3

enum {
	RoleAssist = 1,
	RoleTank   = 2,
	RolePuller = 4,
	RoleLeader = 8
};

class GroupIDConsumer {
public:
	GroupIDConsumer() { id = 0; }
	GroupIDConsumer(uint32 gid) { id = gid; }
	inline const uint32 GetID()	const { return id; }

protected:
	friend class EntityList;
	//use of this function is highly discouraged
	inline void SetID(uint32 set_id) { id = set_id; }
private:
	uint32 id;
};

class Group : public GroupIDConsumer {
public:
	Group(Mob* leader);
	Group(uint32 gid);
	~Group();

	struct AddToGroupRequest {
		Mob* mob = nullptr;
		// Only used cross-zone, otherwise use Mob* mob
		std::string member_name  = std::string();
		uint32      character_id = 0;
	};

	bool	AddMember(Mob* new_member, std::string new_member_name = std::string(), uint32 character_id = 0, bool is_merc = false);
	void	AddMember(const std::string& new_member_name);
	void	SendUpdate(uint32 type,Mob* member);
	void	SendLeadershipAAUpdate();
	bool	DelMemberOOZ(const char *Name);
	bool	DelMember(Mob* oldmember,bool ignoresender = false);
	void	DisbandGroup(bool joinraid = false);
	void	GetMemberList(std::list<Mob*>& member_list, bool clear_list = true);
	void	GetClientList(std::list<Client*>& client_list, bool clear_list = true);
	void	GetBotList(std::list<Bot*>& bot_list, bool clear_list = true);
	std::list<uint32> GetRawBotList();
	bool	IsGroupMember(Mob* c);
	bool	IsGroupMember(const char* name);
	bool	Process();
	void	SendGroupJoinOOZ(Mob* NewMember);
	void	CastGroupSpell(Mob* caster,uint16 spellid);
	void	SplitExp(ExpSource exp_source, const uint64 exp, Mob* other);
	void	GroupMessage(Mob* sender,uint8 language,uint8 lang_skill,const char* message);
	void	GroupMessageString(Mob* sender, uint32 type, uint32 string_id, const char* message,const char* message2=0,const char* message3=0,const char* message4=0,const char* message5=0,const char* message6=0,const char* message7=0,const char* message8=0,const char* message9=0, uint32 distance = 0);
	uint32	GetTotalGroupDamage(Mob* other);
	void	SplitMoney(uint32 copper, uint32 silver, uint32 gold, uint32 platinum, Client *splitter = nullptr, bool share = false);
	inline	void SetLeader(Mob* c){ leader = c; };
	inline	Mob* GetLeader() { return leader; };
	std::string	GetLeaderName();
	void	SendHPManaEndPacketsTo(Mob* newmember);
	void	SendHPPacketsFrom(Mob* member);
	void	SendManaPacketFrom(Mob* member);
	void	SendEndurancePacketFrom(Mob* member);
	bool	UpdatePlayer(Mob* update);
	void	MemberZoned(Mob* removemob);
	bool	IsLeader(const char* name);
	inline bool IsLeader(Mob* m) { return m == leader; };
	uint8	GroupCount();
	uint32	GetHighestLevel();
	uint32	GetLowestLevel();
	void	QueuePacket(const EQApplicationPacket *app, bool ack_req = true);
	void	TeleportGroup(Mob* sender, uint32 zoneID, uint16 instance_id, float x, float y, float z, float heading);
	uint16	GetAvgLevel();
	bool	LearnMembers();
	void	VerifyGroup();
	void	BalanceHP(int32 penalty, float range = 0, Mob* caster = nullptr, int32 limit = 0);
	void	BalanceMana(int32 penalty, float range = 0, Mob* caster = nullptr, int32 limit = 0);
	void	HealGroup(uint32 heal_amt, Mob* caster, float range = 0);
	inline	void SetGroupAAs(GroupLeadershipAA_Struct *From) { memcpy(&LeaderAbilities, From, sizeof(GroupLeadershipAA_Struct)); }
	inline	void GetGroupAAs(GroupLeadershipAA_Struct *Into) { memcpy(Into, &LeaderAbilities, sizeof(GroupLeadershipAA_Struct)); }
	void	UpdateGroupAAs();
	void	SaveGroupLeaderAA();
	void	MarkNPC(Mob* Target, int Number);
	int8	GetNumberNeedingHealedInGroup(int8 hpr, bool include_pets);
	void	DelegateMainTank(const char *NewMainAssistName, uint8 toggle = 0);
	void	DelegateMainAssist(const char *NewMainAssistName, uint8 toggle = 0);
	void	DelegatePuller(const char *NewMainAssistName, uint8 toggle = 0);
	void	UnDelegateMainTank(const char *OldMainAssistName, uint8 toggle = 0);
	void	UnDelegateMainAssist(const char *OldMainAssistName, uint8 toggle = 0);
	void	UnDelegatePuller(const char *OldMainAssistName, uint8 toggle = 0);
	bool	IsNPCMarker(Client *c);
	void	SetGroupAssistTarget(Mob *m);
	void	SetGroupTankTarget(Mob *m);
	void	SetGroupPullerTarget(Mob *m);
	bool	HasRole(Mob *m, uint8 Role);
	uint8	GetMemberRole(Mob* m);
	uint8	GetMemberRole(const char* name);
	void	NotifyAssistTarget(Client *c);
	void	NotifyTankTarget(Client *c);
	void	NotifyPullerTarget(Client *c);
	void	DelegateMarkNPC(const char *NewNPCMarkerName);
	void	UnDelegateMarkNPC(const char *OldNPCMarkerName);
	void	NotifyMainTank(Client *c, uint8 toggle = 0);
	void	NotifyMainAssist(Client *c, uint8 toggle = 0);
	void	NotifyPuller(Client *c, uint8 toggle = 0);
	void	NotifyMarkNPC(Client *c);
	inline	uint32 GetNPCMarkerID() { return NPCMarkerID; }
	void	SetMainTank(const char *NewMainTankName);
	void	SetMainAssist(const char *NewMainAssistName);
	void	SetPuller(const char *NewPullerName);
	const char *GetMainTankName() { return MainTankName.c_str(); }
	const char *GetMainAssistName() { return MainAssistName.c_str(); }
	const char *GetPullerName() { return PullerName.c_str(); }
	bool	AmIMainTank(const char *mob_name);
	bool	AmIMainAssist(const char *mob_name);
	bool	AmIPuller(const char *mob_name);
	void	SetNPCMarker(const char *NewNPCMarkerName);
	void	UnMarkNPC(uint16 ID);
	void	SendMarkedNPCsToMember(Client *c, bool Clear = false);
	inline int GetLeadershipAA(int AAID) { return LeaderAbilities.ranks[AAID]; }
	void	ClearAllNPCMarks();
	void	QueueHPPacketsForNPCHealthAA(Mob* sender, const EQApplicationPacket* app);
	void	QueueClients(Mob *sender, const EQApplicationPacket *app, bool ack_required = true, bool ignore_sender = true, float distance = 0);
	void	ChangeLeader(Mob* newleader);
	const char *GetClientNameByIndex(uint8 index);
	void	UpdateXTargetMarkedNPC(uint32 Number, Mob *m);
	void	SetDirtyAutoHaters();
	inline XTargetAutoHaters *GetXTargetAutoMgr() { return &m_autohatermgr; }
	void	JoinRaidXTarget(Raid *raid, bool first = false);
	void	AddToGroup(AddToGroupRequest r);
	void	AddToGroup(Mob* m);
	static void	RemoveFromGroup(Mob* m);
	void RemoveClientsBots(Client* c);

	void SetGroupMentor(int percent, char *name);
	void ClearGroupMentor();
	inline int GetMentorPercent() { return mentor_percent; }
	inline Client *GetMentoree() { return mentoree; }

	bool AnyMemberHasDzLockout(const std::string& expedition, const std::string& event);

	Mob*	members[MAX_GROUP_MEMBERS] {nullptr};
	char	membername[MAX_GROUP_MEMBERS][64] {""};
	uint8	MemberRoles[MAX_GROUP_MEMBERS] {0};
	bool	disbandcheck {false};
	bool	castspell {false};

private:
	Mob*	leader;
	GroupLeadershipAA_Struct LeaderAbilities;
	std::string	MainTankName;
	std::string	MainAssistName;
	std::string	PullerName;
	std::string	NPCMarkerName;
	uint16	NPCMarkerID;
	uint16	AssistTargetID;
	uint16	TankTargetID;
	uint16	PullerTargetID;
	uint16	MarkedNPCs[MAX_MARKED_NPCS];

	std::string mentoree_name;
	Client *mentoree;
	int mentor_percent;

	XTargetAutoHaters m_autohatermgr;
};
