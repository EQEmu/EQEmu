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

#include "common/events/player_event_logs.h"
#include "common/raid.h"
#include "common/repositories/character_expedition_lockouts_repository.h"
#include "common/repositories/raid_details_repository.h"
#include "common/repositories/raid_members_repository.h"
#include "common/strings.h"
#include "zone/bot.h"
#include "zone/client.h"
#include "zone/dynamic_zone.h"
#include "zone/entity.h"
#include "zone/groups.h"
#include "zone/mob.h"
#include "zone/queryserv.h"
#include "zone/raids.h"
#include "zone/string_ids.h"
#include "zone/worldserver.h"

extern EntityList  entity_list;
extern WorldServer worldserver;
extern QueryServ  *QServ;

Raid::Raid(uint32 raidID)
: GroupIDConsumer(raidID)
{
	for (auto& gm : group_mentor) {
		gm.mentor_percent = 0;
		gm.mentoree = nullptr;
	}
	leader = nullptr;
	memset(leadername, 0, 64);
	locked = false;
	LootType = 4;

	m_autohatermgr.SetOwner(nullptr, nullptr, this);

	for (int i = 0; i < MAX_NO_RAID_MAIN_ASSISTERS; i++) {
		memset(main_assister_pcs[i], 0, 64);
		memset(main_marker_pcs[i], 0, 64);
		marked_npcs[i].entity_id = 0;
		marked_npcs[i].zone_id = 0;
		marked_npcs[i].instance_id = 0;
	}
}

Raid::Raid(Client* nLeader)
: GroupIDConsumer()
{
	for (auto& gm : group_mentor) {
		gm.mentor_percent = 0;
		gm.mentoree = nullptr;
	}
	leader = nLeader;
	memset(leadername, 0, 64);
	strn0cpy(leadername, nLeader->GetName(), 64);
	locked = false;
	LootType = 4;

	m_autohatermgr.SetOwner(nullptr, nullptr, this);

	for (int i = 0; i < MAX_NO_RAID_MAIN_ASSISTERS; i++) {
		memset(main_assister_pcs[i], 0, 64);
		memset(main_marker_pcs[i], 0, 64);
		marked_npcs[i].entity_id = 0;
		marked_npcs[i].zone_id = 0;
		marked_npcs[i].instance_id = 0;
	}
}

Raid::~Raid()
{
}

bool Raid::Process()
{
	if (forceDisband) {
		return false;
	}

	if (disbandCheck) {
		int count = 0;
		for (const auto& m : members) {
			if (strlen(m.member_name) == 0) {
				continue;
			}
			else {
				count++;
			}
		}
		if (count == 0) {
			return false;
		}
	}
	return true;
}

void Raid::AddMember(Client *c, uint32 group, bool rleader, bool groupleader, bool looter)
{
	if (!c) {
		return;
	}

	const auto query = fmt::format(
		"REPLACE INTO raid_members SET raidid = {}, charid = {}, bot_id = 0, "
		"groupid = {}, _class = {}, level = {}, name = '{}', "
		"isgroupleader = {}, israidleader = {}, islooter = {}",
		GetID(),
		c->CharacterID(),
		group,
		c->GetClass(),
		c->GetLevel(),
		c->GetName(),
		groupleader,
		rleader,
		looter
	);

	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		LogError("Error inserting into raid members: [{}]", results.ErrorMessage().c_str());
	}

	LearnMembers();
	VerifyRaid();

	if (rleader && group != RAID_GROUPLESS) {
		database.SetRaidGroupLeaderInfo(group, GetID());
		UpdateRaidAAs();
	}
	else if (rleader) {
		database.SetRaidGroupLeaderInfo(RAID_GROUPLESS, GetID());
		UpdateRaidAAs();
	}

	if (group != RAID_GROUPLESS && groupleader) {
		database.SetRaidGroupLeaderInfo(group, GetID());
		UpdateGroupAAs(group);
	}

	if (group < MAX_RAID_GROUPS) {
		GroupUpdate(group);
	}
	else { // get raid AAs, GroupUpdate will handles it otherwise
		SendGroupLeadershipAA(c, RAID_GROUPLESS);
	}

	SendRaidAddAll(c->GetName());

	c->SetRaidGrouped(true);
	SendRaidMOTD(c);

	if (group == RAID_GROUPLESS) {
		if (rleader) {
			GetXTargetAutoMgr()->merge(*c->GetXTargetAutoMgr());
			c->GetXTargetAutoMgr()->clear();
			c->SetXTargetAutoMgr(GetXTargetAutoMgr());
		}
		else {
			if (!c->GetXTargetAutoMgr()->empty()) {
				GetXTargetAutoMgr()->merge(*c->GetXTargetAutoMgr());
				c->GetXTargetAutoMgr()->clear();
				c->RemoveAutoXTargets();
			}

			c->SetXTargetAutoMgr(GetXTargetAutoMgr());

			if (!c->GetXTargetAutoMgr()->empty()) {
				c->SetDirtyAutoHaters();
			}
		}
	}

	if (auto* raid_update = c->GetRaid()) {
		raid_update->SendHPManaEndPacketsTo(c);
		raid_update->SendHPManaEndPacketsFrom(c);
	}

	auto pack = new ServerPacket(ServerOP_RaidAdd, sizeof(ServerRaidGeneralAction_Struct));
	auto* rga = (ServerRaidGeneralAction_Struct*) pack->pBuffer;
	strn0cpy(rga->playername, c->GetName(), sizeof(rga->playername));
	rga->rid         = GetID();
	rga->zoneid      = zone->GetZoneID();
	rga->instance_id = zone->GetInstanceID();
	worldserver.SendPacket(pack);
	safe_delete(pack);

	SendAssistTarget(c);

}

void Raid::AddBot(Bot* b, uint32 group, bool raid_leader, bool group_leader, bool looter)
{
	if (!b) {
		return;
	}

	const auto query = fmt::format(
		"REPLACE INTO raid_members SET raidid = {}, "
		"charid = 0, bot_id = {}, groupid = {}, _class = {}, level = {}, name = '{}', "
		"isgroupleader = {}, israidleader = {}, islooter = {}",
		GetID(),
		b->GetBotID(),
		group,
		b->GetClass(),
		b->GetLevel(),
		b->GetName(),
		group_leader,
		raid_leader,
		looter
	);

	auto results = database.QueryDatabase(query);

	LearnMembers();
	VerifyRaid();

	if (group < MAX_RAID_GROUPS) {
		GroupUpdate(group);
	} else { // get raid AAs, GroupUpdate will handle it otherwise
		SendGroupLeadershipAA(b->GetOwner()->CastToClient(), RAID_GROUPLESS);
	}

	SendRaidAddAll(b->GetName());

	b->SetRaidGrouped(true);

	auto pack = new ServerPacket(ServerOP_RaidAdd, sizeof(ServerRaidGeneralAction_Struct));
	auto* rga = (ServerRaidGeneralAction_Struct*) pack->pBuffer;
	strn0cpy(rga->playername, b->GetName(), sizeof(rga->playername));
	rga->rid         = GetID();
	rga->zoneid      = zone->GetZoneID();
	rga->instance_id = zone->GetInstanceID();
	worldserver.SendPacket(pack);
	safe_delete(pack);
}


void Raid::RemoveMember(const char *character_name)
{
	std::string query = StringFormat("DELETE FROM raid_members where name='%s'", character_name);
	auto results = database.QueryDatabase(query);

	auto* b = entity_list.GetBotByBotName(character_name);
	auto* c = entity_list.GetClientByName(character_name);

	if (RuleB(Bots, Enabled) && b) {
		b = entity_list.GetBotByBotName(character_name);
		b->SetFollowID(b->GetOwner()->CastToClient()->GetID());
		b->SetTarget(nullptr);
		b->SetRaidGrouped(false);
		b->p_raid_instance = nullptr;
		b->SetStoredRaid(nullptr);
		b->SetVerifiedRaid(false);
	}

	disbandCheck = true;
	SendRaidRemoveAll(character_name);
	SendRaidDisband(c);
	LearnMembers();
	VerifyRaid();

	if (c) {
		c->SetRaidGrouped(false);
		c->LeaveRaidXTargets(this);
		c->p_raid_instance = nullptr;
	}

	auto pack = new ServerPacket(ServerOP_RaidRemove, sizeof(ServerRaidGeneralAction_Struct));
	auto* rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
	rga->rid         = GetID();
	rga->instance_id = zone->GetInstanceID();
	rga->zoneid      = zone->GetZoneID();
	strn0cpy(rga->playername, character_name, sizeof(rga->playername));
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Raid::DisbandRaid()
{
	const auto query = fmt::format(
		"DELETE FROM raid_members WHERE raidid = {}",
		GetID()
	);
	auto results = database.QueryDatabase(query);

	LearnMembers();
	VerifyRaid();
	SendRaidDisbandAll();

	auto pack = new ServerPacket(ServerOP_RaidDisband, sizeof(ServerRaidGeneralAction_Struct));
	auto* rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
	rga->rid         = GetID();
	rga->zoneid      = zone->GetZoneID();
	rga->instance_id = zone->GetInstanceID();
	strn0cpy(rga->playername, " ", sizeof(rga->playername));
	worldserver.SendPacket(pack);
	safe_delete(pack);

	forceDisband = true;
}

void Raid::MoveMember(const char *name, uint32 newGroup)
{

	const auto query = fmt::format(
		"UPDATE raid_members SET groupid = {} WHERE name = '{}'",
		newGroup,
		name
		);
	auto results = database.QueryDatabase(query);

	LearnMembers();
	VerifyRaid();
	SendRaidMoveAll(name);

	auto pack = new ServerPacket(ServerOP_RaidChangeGroup, sizeof(ServerRaidGeneralAction_Struct));
	auto* rga = (ServerRaidGeneralAction_Struct*) pack->pBuffer;
	strn0cpy(rga->playername, name, sizeof(rga->playername));
	rga->rid         = GetID();
	rga->zoneid      = zone->GetZoneID();
	rga->instance_id = zone->GetInstanceID();
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Raid::SetGroupLeader(const char *who, bool glFlag)
{
	const auto query = fmt::format(
		"UPDATE raid_members SET isgroupleader = {} WHERE name = '{}'",
		glFlag,
		who
	);
	auto results = database.QueryDatabase(query);

	LearnMembers();
	VerifyRaid();

	auto pack = new ServerPacket(ServerOP_RaidGroupLeader, sizeof(ServerRaidGeneralAction_Struct));
	auto* rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
	strn0cpy(rga->playername, who, sizeof(rga->playername));
	rga->rid         = GetID();
	rga->zoneid      = zone->GetZoneID();
	rga->instance_id = zone->GetInstanceID();
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

Client *Raid::GetGroupLeader(uint32 group_id)
{
	if (group_id == RAID_GROUPLESS) {
		return nullptr;
	}

	for (const auto& m : members) {
		if (m.member && m.is_group_leader && m.group_number == group_id) {
			return m.member;
		}
	}

	return nullptr;
}

void Raid::SetRaidLeader(const char *wasLead, const char *name)
{
	std::string query = StringFormat("UPDATE raid_members SET israidleader = 0 WHERE name = '%s'", wasLead);
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		LogError("Set Raid Leader error: [{}]\n", results.ErrorMessage().c_str());
	}

	query = StringFormat("UPDATE raid_members SET israidleader = 1 WHERE name = '%s'", name);
	results = database.QueryDatabase(query);
	if (!results.Success()) {
		LogError("Set Raid Leader error: [{}]\n", results.ErrorMessage().c_str());
	}

	strn0cpy(leadername, name, 64);

	Client *c = entity_list.GetClientByName(name);
	if (c) {
		SetLeader(c);
	}
	else {
		SetLeader(nullptr);
	} //sanity check, should never get hit but we want to prefer to NOT crash if we do VerifyRaid and leader never gets set there (raid without a leader?)

	LearnMembers();
	VerifyRaid();
	SendMakeLeaderPacket(name);

	auto pack = new ServerPacket(ServerOP_RaidLeader, sizeof(ServerRaidGeneralAction_Struct));
	auto rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
	rga->rid = GetID();
	strn0cpy(rga->playername, name, 64);
	rga->zoneid = zone->GetZoneID();
	rga->instance_id = zone->GetInstanceID();
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Raid::SaveGroupLeaderAA(uint32 gid)
{
	auto queryBuffer = new char[sizeof(GroupLeadershipAA_Struct) * 2 + 1];
	database.DoEscapeString(queryBuffer, (char*)&group_aa[gid], sizeof(GroupLeadershipAA_Struct));

	std::string query = "UPDATE raid_leaders SET leadershipaa = '";
	query += queryBuffer;
	query +=  StringFormat("' WHERE gid = %lu AND rid = %lu LIMIT 1", gid, GetID());
	safe_delete_array(queryBuffer);
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		LogError("Unable to store LeadershipAA: [{}]\n", results.ErrorMessage().c_str());
	}
}

void Raid::SaveRaidLeaderAA()
{
	auto queryBuffer = new char[sizeof(RaidLeadershipAA_Struct) * 2 + 1];
	database.DoEscapeString(queryBuffer, (char*)&raid_aa, sizeof(RaidLeadershipAA_Struct));

	std::string query = "UPDATE raid_leaders SET leadershipaa = '";
	query += queryBuffer;
	query +=  StringFormat("' WHERE gid = %lu AND rid = %lu LIMIT 1", RAID_GROUPLESS, GetID());
	safe_delete_array(queryBuffer);
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		LogError("Unable to store LeadershipAA: [{}]\n", results.ErrorMessage().c_str());
	}
}

void Raid::UpdateGroupAAs(uint32 gid)
{

	if (gid > MAX_RAID_GROUPS || gid == RAID_GROUPLESS || gid < 0) {
		return;
	}

	Client *gl = GetGroupLeader(gid);

	if (gl && gl->IsClient()) {
		gl->GetGroupAAs(&group_aa[gid]);
	} else {
		memset(&group_aa[gid], 0, sizeof(GroupLeadershipAA_Struct));
	}

	SaveGroupLeaderAA(gid);
}

void Raid::UpdateRaidAAs()
{
	Client *rl = GetLeader();

	if (rl) {
		rl->GetRaidAAs(&raid_aa);
	} else {
		memset(&raid_aa, 0, sizeof(RaidLeadershipAA_Struct));
	}

	SaveRaidLeaderAA();
}

bool Raid::IsGroupLeader(const char* name)
{
	if (name) {
		for (const auto &m: members) {
			if (!strcmp(m.member_name, name)) {
				return m.is_group_leader;
			}
		}
	}

	return false;
}

bool Raid::IsGroupLeader(Client *c)
{
	if (c) {
		for (const auto &m: members) {
			if (m.member == c) {
				return true;
			}
		}
	}

	return false;
}

void Raid::UpdateLevel(const char *name, int newLevel)
{
	std::string query = StringFormat("UPDATE raid_members SET level = %lu WHERE name = '%s'",
                                    (unsigned long)newLevel, name);
    auto results = database.QueryDatabase(query);

	LearnMembers();
	VerifyRaid();
}

uint32 Raid::GetFreeGroup()
{
	//check each group return the first one with 0 members assigned...
	for (int x = 0; x < MAX_RAID_GROUPS; x++) {
		int count = 0;
		for (const auto& m : members) {
			if (m.group_number == x && (strlen(m.member_name) > 0)) {
				count++;
			}
		}
		if (count == 0) {
			return x;
		}
	}
	//if we get to here then there were no free groups so we added the group as free floating members.
	return RAID_GROUPLESS;
}

uint8 Raid::GroupCount(uint32 gid)
{
	uint8 count = 0;
	if (gid < MAX_RAID_GROUPS) {
		for (const auto& m : members) {
			if (m.group_number == gid && strlen(m.member_name) > 0) {
				count++;
			}
		}
	}
	else {
		for (const auto& m : members) {
			if (m.group_number > 11 && strlen(m.member_name) > 0) {
				count++;
			}
		}
	}
	return count;
}

uint8 Raid::RaidCount()
{
	int count = 0;
	for (const auto& m : members) {
		if (strlen(m.member_name) > 0) {
			count++;
		}
	}
	return count;
}

uint32 Raid::GetGroup(const char *name)
{
	for (const auto& m : members) {
		if (strcmp(m.member_name, name) == 0) {
			return m.group_number;
		}
	}
	return RAID_GROUPLESS;
}

uint32 Raid::GetGroup(Client *c)
{
	for (const auto& m : members) {
		if (m.member == c) {
			return m.group_number;
		}
	}
	return RAID_GROUPLESS;
}

void Raid::RaidSay(const char *msg, Client *c, uint8 language, uint8 lang_skill)
{
	if (!c) {
		return;
	}

	auto pack = new ServerPacket(ServerOP_RaidSay, sizeof(ServerRaidMessage_Struct) + strlen(msg) + 1);
	ServerRaidMessage_Struct *rga = (ServerRaidMessage_Struct*)pack->pBuffer;
	rga->rid = GetID();
	rga->gid = RAID_GROUPLESS;
	rga->language = language;
	rga->lang_skill = lang_skill;
	strn0cpy(rga->from, c->GetName(), 64);

	strcpy(rga->message, msg); // this is safe because we are allocating enough space for the entire msg above

	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Raid::RaidGroupSay(const char *msg, Client *c, uint8 language, uint8 lang_skill)
{
	if (!c) {
		return;
	}

	uint32 group_id_to_use = GetGroup(c->GetName());

	if (group_id_to_use >= MAX_RAID_GROUPS) {
		return;
	}

	auto pack = new ServerPacket(ServerOP_RaidGroupSay, sizeof(ServerRaidMessage_Struct) + strlen(msg) + 1);
	auto rga = (ServerRaidMessage_Struct*)pack->pBuffer;
	rga->rid = GetID();
	rga->gid = group_id_to_use;
	rga->language = language;
	rga->lang_skill = lang_skill;
	strn0cpy(rga->from, c->GetName(), 64);

	strcpy(rga->message, msg); // this is safe because we are allocating enough space for the entire msg above

	worldserver.SendPacket(pack);
	safe_delete(pack);
}

uint32 Raid::GetPlayerIndex(const char *name)
{
	for (int x = 0; x < MAX_RAID_MEMBERS; x++) {
		if (strcmp(name, members[x].member_name) == 0) {
			return x;
		}
	}
	return 0; //should never get to here if we do everything else right, set it to 0 so we never crash things that rely on it.
}

uint32 Raid::GetPlayerIndex(Client *c)
{
	for (int i = 0; i < MAX_RAID_MEMBERS; ++i) {
		if (c == members[i].member) {
			return i;
		}
	}
	return 0xFFFFFFFF; // return sentinel value, make sure you check it unlike the above function
}

Client *Raid::GetClientByIndex(uint16 index)
{
	if (index >= MAX_RAID_MEMBERS) {
		return nullptr;
	}

	return members[index].member;
}

void Raid::CastGroupSpell(Mob* caster, uint16 spellid, uint32 gid)
{
	float range, distance;

	if (!caster) {
		return;
	}

	range = caster->GetAOERange(spellid);

	float range2 = range*range;

	for (const auto& m : members) {
		if (m.member == caster) {
			caster->SpellOnTarget(spellid, caster);
#ifdef GROUP_BUFF_PETS
			if (spells[spellid].target_type != ST_GroupNoPets && caster->GetPet() && caster->HasPetAffinity() && !caster->GetPet()->IsCharmed()) {
				caster->SpellOnTarget(spellid, caster->GetPet());
			}
#endif
		}
		else if (m.member != nullptr && m.group_number == gid) {
			distance = DistanceSquared(caster->GetPosition(), m.member->GetPosition());
			if (distance <= range2) {
				caster->SpellOnTarget(spellid, m.member);

#ifdef GROUP_BUFF_PETS
				if (spells[spellid].target_type != ST_GroupNoPets && m.member->GetPet() && m.member->HasPetAffinity() &&
					!m.member->GetPet()->IsCharmed()) {
					caster->SpellOnTarget(spellid, m.member->GetPet());
				}
#endif
			}
			else {
				LogSpells("Raid spell: [{}] is out of range [{}] at distance [{}] from [{}]", m.member->GetName(), range, distance, caster->GetName());
			}
		}
	}
}


uint32 Raid::GetTotalRaidDamage(Mob* other)
{
	uint32 total = 0;

	for (const auto& m : members) {
		if (!m.member) {
			continue;
		}

		if (other->CheckAggro(m.member)) {
			total += other->GetHateAmount(m.member, true);
		}
	}

	return total;
}

void Raid::HealGroup(uint32 heal_amount, Mob* caster, uint32 group_id, float range)
{
	if (!caster) {
		return;
	}

	if (!range) {
		range = 200;
	}

	float distance;
	float range_squared = range * range;

	int member_count = 0;

	for (const auto& m : members) {
		if (m.member && m.group_number == group_id) {
			distance = DistanceSquared(caster->GetPosition(), m.member->GetPosition());

			if (distance <= range_squared) {
				member_count += 1;
			}
		}
	}

	if (member_count > 0) {
		heal_amount /= member_count;
	}

	for (const auto& m : members) {
		if (m.member && m.group_number == group_id) {
			distance = DistanceSquared(caster->GetPosition(), m.member->GetPosition());

			if (distance <= range_squared) {
				m.member->SetHP(m.member->GetHP() + heal_amount);
				m.member->SendHPUpdate();
			}
		}
	}
}


void Raid::BalanceHP(int32 penalty, uint32 group_id, float range, Mob* caster, int32 limit)
{
	if (!caster) {
		return;
	}

	if (!range) {
		range = 200;
	}

	int damage_taken           = 0;
	int damage_taken_temporary = 0;
	int member_count           = 0;

	float distance;
	float range_squared = range * range;

	for (const auto& m : members) {
		if (m.member && m.group_number == group_id) {
			distance = DistanceSquared(caster->GetPosition(), m.member->GetPosition());

			if (distance <= range_squared) {
				damage_taken_temporary = m.member->GetMaxHP() - m.member->GetHP();

				if (limit && (damage_taken_temporary > limit)) {
					damage_taken_temporary = limit;
				}

				damage_taken += damage_taken_temporary;
				member_count += 1;
			}
		}
	}

	damage_taken += damage_taken * penalty / 100;

	if (member_count > 0) {
		damage_taken /= member_count;
	}

	for (const auto& m : members) {
		if (m.member && m.group_number == group_id) {
			distance = DistanceSquared(caster->GetPosition(), m.member->GetPosition());

			//this way the ability will never kill someone
			//but it will come darn close
			if (distance <= range_squared) {
				if ((m.member->GetMaxHP() - damage_taken) < 1) {
					m.member->SetHP(1);
					m.member->SendHPUpdate();
				} else {
					m.member->SetHP(m.member->GetMaxHP() - damage_taken);
					m.member->SendHPUpdate();
				}
			}
		}
	}
}

void Raid::BalanceMana(int32 penalty, uint32 group_id, float range, Mob* caster, int32 limit)
{
	if (!caster) {
		return;
	}

	if (!range) {
		range = 200;
	}

	float distance;
	float range_squared = range * range;

	int mana_taken           = 0;
	int mana_taken_temporary = 0;
	int member_count         = 0;

	for (const auto& m : members) {
		if (m.is_bot) {
			continue;
		}

		if (m.member && m.group_number == group_id && m.member->GetMaxMana() > 0) {
			distance = DistanceSquared(caster->GetPosition(), m.member->GetPosition());

			if (distance <= range_squared) {
				mana_taken_temporary = m.member->GetMaxMana() - m.member->GetMana();

				if (limit && (mana_taken_temporary > limit)) {
					mana_taken_temporary = limit;
				}

				mana_taken += mana_taken_temporary;
				member_count += 1;
			}
		}
	}

	mana_taken += mana_taken * penalty / 100;

	if (member_count > 0) {
		mana_taken /= member_count;
	}

	for (const auto& m : members) {
		if (m.is_bot) {
			continue;
		}

		if (m.member && m.group_number == group_id) {
			distance = DistanceSquared(caster->GetPosition(), m.member->GetPosition());

			if (distance <= range_squared) {
				if ((m.member->GetMaxMana() - mana_taken) < 1) {
					m.member->SetMana(1);

					if (m.member->IsClient()) {
						m.member->CastToClient()->SendManaUpdate();
					}
				} else {
					m.member->SetMana(m.member->GetMaxMana() - mana_taken);

					if (m.member->IsClient()) {
						m.member->CastToClient()->SendManaUpdate();
					}
				}
			}
		}
	}
}

//basically the same as Group's version just with more people like a lot of non group specific raid stuff
//this only functions if the member has a group in the raid. This does not support /autosplit?
void Raid::SplitMoney(uint32 gid, uint32 copper, uint32 silver, uint32 gold, uint32 platinum, Client *splitter)
{
	//avoid unneeded work
	if (gid == RAID_GROUPLESS) {
		return;
	}

	if (
		!copper &&
		!silver &&
		!gold &&
		!platinum
	) {
		return;
	}

	uint8 member_count = 0;
	for (const auto& m : members) {
		if (m.is_bot) {
			continue;
		}

		if (m.member && m.group_number == gid && m.member->IsClient()) {
			member_count++;
		}
	}

	if (!member_count) {
		return;
	}

	uint32 modifier;
	if (member_count > 1) {
		modifier = platinum % member_count;

		if (modifier) {
			platinum -= modifier;
			gold += 10 * modifier;
		}

		modifier = gold % member_count;

		if (modifier) {
			gold -= modifier;
			silver += 10 * modifier;
		}

		modifier = silver % member_count;

		if (modifier) {
			silver -= modifier;
			copper += 10 * modifier;
		}
	}

	auto copper_split = copper / member_count;
	auto silver_split = silver / member_count;
	auto gold_split = gold / member_count;
	auto platinum_split = platinum / member_count;

	for (const auto& m : members) {
		if (m.is_bot) {
			continue;
		}

		if (m.member && m.group_number == gid && m.member->IsClient()) { // If Group Member is Client
			m.member->AddMoneyToPP(
				copper_split,
				silver_split,
				gold_split,
				platinum_split,
				true
			);

			if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::SPLIT_MONEY)) {
				auto e = PlayerEvent::SplitMoneyEvent{
					.copper = copper_split,
					.silver = silver_split,
					.gold = gold_split,
					.platinum = platinum_split,
					.player_money_balance = m.member->GetCarriedMoney(),
				};

				RecordPlayerEventLogWithClient(m.member, PlayerEvent::SPLIT_MONEY, e);
			}

			m.member->MessageString(
				Chat::MoneySplit,
				YOU_RECEIVE_AS_SPLIT,
				Strings::Money(
					platinum_split,
					gold_split,
					silver_split,
					copper_split
				).c_str()
			);
		}
	}
}

void Raid::TeleportGroup(Mob* sender, uint32 zoneID, uint16 instance_id, float x, float y, float z, float heading, uint32 gid)
{
	for (const auto& m : members) {
		if (m.is_bot) {
			continue;
		}

		if (m.member && m.group_number == gid && m.member->IsClient()) {
			m.member->MovePC(zoneID, instance_id, x, y, z, heading, 0, ZoneSolicited);
		}
	}
}

void Raid::TeleportRaid(Mob* sender, uint32 zoneID, uint16 instance_id, float x, float y, float z, float heading)
{
	for (const auto& m : members) {
		if (m.is_bot) {
			continue;
		}

		if (m.member && m.member->IsClient()) {
			m.member->MovePC(zoneID, instance_id, x, y, z, heading, 0, ZoneSolicited);
		}
	}
}

void Raid::ChangeLootType(uint32 type)
{
	std::string query = StringFormat("UPDATE raid_details SET loottype = %lu WHERE raidid = %lu",
									 (unsigned long)type, (unsigned long)GetID());
	auto results = database.QueryDatabase(query);
	LootType = type;
}

void Raid::AddRaidLooter(const char* looter)
{
	std::string query = StringFormat("UPDATE raid_members SET islooter = 1 WHERE name = '%s'", looter);
	auto results = database.QueryDatabase(query);

	for (auto& m : members) {
		if (m.is_bot) {
			continue;
		}

		if (strcmp(looter, m.member_name) == 0) {
			m.is_looter = true;
			break;
		}
	}

	auto pack = new ServerPacket(ServerOP_DetailsChange, sizeof(ServerRaidGeneralAction_Struct));
	auto rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
	rga->rid = GetID();
	rga->zoneid = zone->GetZoneID();
	rga->instance_id = zone->GetInstanceID();
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Raid::RemoveRaidLooter(const char* looter)
{
	std::string query = StringFormat("UPDATE raid_members SET islooter = 0 WHERE name = '%s'", looter);
	auto results = database.QueryDatabase(query);

	for (auto& m: members) {
		if (m.is_bot) {
			continue;
		}

		if (strcmp(looter, m.member_name) == 0) {
			m.is_looter = false;
			break;
		}
	}

	auto pack = new ServerPacket(ServerOP_DetailsChange, sizeof(ServerRaidGeneralAction_Struct));
	auto rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
	rga->rid = GetID();
	rga->zoneid = zone->GetZoneID();
	rga->instance_id = zone->GetInstanceID();
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

bool Raid::IsRaidMember(const char *name)
{
	if (name) {
		for (const auto &m: members) {
			if (!strcmp(m.member_name, name)) {
				return true;
			}
		}
	}

	return false;
}

bool Raid::IsRaidMember(Client* c)
{
	if (c) {
		for (const auto &m: members) {
			if (m.member == c) {
				return true;
			}
		}
	}

	return false;
}

uint32 Raid::GetHighestLevel()
{
	uint32 highlvl = 0;
	for (const auto& m : members) {
		if (strlen(m.member_name) && m.level > highlvl) {
			highlvl = m.level;
		}
	}

	return highlvl;
}

uint32 Raid::GetLowestLevel()
{
	uint32 lowlvl = 1000;
	for (const auto& m : members) {
		if (strlen(m.member_name) && m.level < lowlvl) {
			lowlvl = m.level;
		}
	}

	return lowlvl;
}

/*
 * Packet Functions Start
 */
void Raid::SendRaidCreate(Client *to)
{
	if (!to) {
		return;
	}

	auto outapp = new EQApplicationPacket(OP_RaidJoin, sizeof(RaidCreate_Struct));
	auto rc = (RaidCreate_Struct*)outapp->pBuffer;
	rc->action = raidCreate;
	strn0cpy(rc->leader_name, leadername, 64);
	rc->leader_id = (GetLeader()?GetLeader()->GetID():0);
	to->QueuePacket(outapp);
	safe_delete(outapp);
}

void Raid::SendRaidAdd(const char *who, Client *to)
{
	if (!to) {
		return;
	}

	std::vector<RaidMember> rm = GetMembers();

	for (const auto& m : rm) {
		if (strcmp(m.member_name, who) == 0) {
			auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidAddMember_Struct));
			auto ram = (RaidAddMember_Struct*)outapp->pBuffer;
			ram->raidGen.action = raidAdd;
			ram->raidGen.parameter = m.group_number;
			strn0cpy(ram->raidGen.leader_name, m.member_name, sizeof(ram->raidGen.leader_name));
			strn0cpy(ram->raidGen.player_name, m.member_name, sizeof(ram->raidGen.player_name));
			ram->_class = m._class;
			ram->level = m.level;
			ram->isGroupLeader = m.is_group_leader;
			to->QueuePacket(outapp);
			safe_delete(outapp);

			if (IsAssister(m.member_name)) {
				SendRaidAssisterTo(m.member_name, to);
			}

			if (IsMarker(m.member_name)) {
				SendRaidMarkerTo(m.member_name, to);
			}

			return;
		}
	}
}

void Raid::SendRaidAddAll(const char *who)
{
	std::vector<RaidMember> rm = GetMembers();

	for (const auto& m : rm) {
		if (strcmp(m.member_name, who) == 0) {
			auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidAddMember_Struct));
			auto ram = (RaidAddMember_Struct*)outapp->pBuffer;
			ram->raidGen.action = raidAdd;
			ram->raidGen.parameter = m.group_number;
			strn0cpy(ram->raidGen.leader_name, m.member_name, sizeof(ram->raidGen.leader_name));
			strn0cpy(ram->raidGen.player_name, m.member_name, sizeof(ram->raidGen.player_name));
			ram->isGroupLeader = m.is_group_leader;
			ram->_class = m._class;
			ram->level = m.level;

			QueuePacket(outapp);
			safe_delete(outapp);

			if (IsAssister(m.member_name)) {
				SendRaidAssister(m.member_name);
			}

			if (IsMarker(m.member_name)) {
				SendRaidMarker(m.member_name);
			}

			return;
		}
	}
}

void Raid::SendRaidRemove(const char *who, Client *to)
{
	if (!to) {
		return;
	}

	for (const auto& m : members) {
		if (strcmp(m.member_name, who) == 0) {
			auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
			auto rg = (RaidGeneral_Struct*)outapp->pBuffer;
			rg->action = raidRemove2;
			strn0cpy(rg->leader_name, who, 64);
			strn0cpy(rg->player_name, who, 64);
			rg->parameter = 0;
			to->QueuePacket(outapp);
			safe_delete(outapp);
			return;
		}
	}
}

void Raid::SendRaidRemoveAll(const char *who)
{
	for (const auto& m : members) {
		if (strcmp(m.member_name, who) == 0) {
			auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
			auto rg = (RaidGeneral_Struct*)outapp->pBuffer;
			rg->action = raidRemove2;
			strn0cpy(rg->leader_name, who, 64);
			strn0cpy(rg->player_name, who, 64);
			rg->parameter = 0;
			QueuePacket(outapp);
			safe_delete(outapp);
			return;
		}
	}
}

void Raid::SendRaidDisband(Client *to)
{
	if (!to) {
		return;
	}

	auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
	auto rg = (RaidGeneral_Struct*)outapp->pBuffer;
	rg->action = raidDisband;
	strn0cpy(rg->leader_name, to->GetName(), 64);
	strn0cpy(rg->player_name, to->GetName(), 64);
	rg->parameter = 0;
	to->QueuePacket(outapp);
	safe_delete(outapp);
}

void Raid::SendRaidDisbandAll()
{
	auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
	auto rg = (RaidGeneral_Struct*)outapp->pBuffer;
	rg->action = raidDisband;
	strn0cpy(rg->leader_name, "RaidMember", 64);
	strn0cpy(rg->player_name, "RaidMember", 64);
	rg->parameter = 0;
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Raid::SendRaidMove(const char* who, Client *to)
{
	if (!to) {
		return;
	}

	Client *c = entity_list.GetClientByName(who);
	if (c && c == to) {
		SendRaidCreate(c);
		SendMakeLeaderPacketTo(leadername, c);
	}

	SendRaidRemove(who, to);
	SendRaidAdd(who, to);
	if (c && c == to) {
		SendBulkRaid(c);
		if (IsLocked()) {
			SendRaidLockTo(c);
		}
	}
}

void Raid::SendRaidMoveAll(const char* who)
{
	Client *c = entity_list.GetClientByName(who);
	SendRaidRemoveAll(who);

	if (c) {
		SendRaidCreate(c);
	}

	if (c) {
		SendBulkRaid(c);
		if (IsLocked()) {
			SendRaidLockTo(c);
		}
	}

	SendRaidAddAll(who);
	SendMakeLeaderPacket(leadername);
}

void Raid::SendBulkRaid(Client *to)
{
	if (!to) {
		return;
	}

	for (const auto& m : members) {
		if (strlen(m.member_name) > 0 && (strcmp(m.member_name, to->GetName()) != 0)) {
			SendRaidAdd(m.member_name, to);
		}
	}
	SendRaidNotes();
}

void Raid::QueuePacket(const EQApplicationPacket *app, bool ack_req)
{
	for (const auto& m : members) {
		if (m.is_bot) {
			continue;
		}

		if (m.member && m.member->IsClient()) {
			m.member->QueuePacket(app, ack_req);
		}
	}
}

void Raid::SendMakeLeaderPacket(const char *who)
{

	if (RuleB(Bots, Enabled) && entity_list.GetBotByBotName(who)) {
		return;
	}

	auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidLeadershipUpdate_Struct));
	auto rg = (RaidLeadershipUpdate_Struct*)outapp->pBuffer;
	rg->action = raidMakeLeader;
	strn0cpy(rg->leader_name, who, 64);
	strn0cpy(rg->player_name, who, 64);
	memcpy(&rg->raid, &raid_aa, sizeof(RaidLeadershipAA_Struct));
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Raid::SendMakeLeaderPacketTo(const char *who, Client *to)
{
	if (!to) {
		return;
	}

	if (RuleB(Bots, Enabled) && entity_list.GetBotByBotName(who)) {
		return;
	}

	auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidLeadershipUpdate_Struct));
	auto rg = (RaidLeadershipUpdate_Struct*)outapp->pBuffer;
	rg->action = raidMakeLeader;
	strn0cpy(rg->leader_name, who, 64);
	strn0cpy(rg->player_name, who, 64);
	memcpy(&rg->raid, &raid_aa, sizeof(RaidLeadershipAA_Struct));
	to->QueuePacket(outapp);
	safe_delete(outapp);
}

//nyi
void Raid::SendMakeGroupLeaderPacketAll()
{
}

void Raid::SendMakeGroupLeaderPacket(const char *who) //13
{
}

void Raid::SendMakeGroupLeaderPacketTo(const char *who, Client *to)
{
	if (!to) {
		return;
	}
}

void Raid::SendGroupUpdate(Client *to)
{
	if (!to) {
		return;
	}

	if (RuleB(Bots, Enabled) && entity_list.GetBotByBotName(to->GetName())) {
		return;
	}

	auto outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupUpdate2_Struct));
	auto gu = (GroupUpdate2_Struct*)outapp->pBuffer;
	gu->action = groupActUpdate;
	uint32 grp = GetGroup(to->GetName());
	if (grp >= MAX_RAID_GROUPS) {
		safe_delete(outapp);
		return;
	}

	int i = 0;
	for (const auto& m : members) {
		if (m.group_number == grp && strlen(m.member_name) > 0) {
			if (m.is_group_leader) {
				strn0cpy(gu->leadersname, m.member_name, sizeof(gu->leadersname));
				if (strcmp(to->GetName(), m.member_name) != 0) {
					strn0cpy(gu->membername[i], m.member_name, sizeof(gu->membername[i]));
					++i;
				}
			}
			else if (strcmp(to->GetName(), m.member_name) != 0) {
				strn0cpy(gu->membername[i], m.member_name, sizeof(gu->membername[i]));
				++i;
			}
		}
	}

	if (strlen(gu->leadersname) < 1) {
		strn0cpy(gu->leadersname, to->GetName(), sizeof(gu->leadersname));
	}

	strn0cpy(gu->yourname, to->GetName(), 64);
	memcpy(&gu->leader_aas, &group_aa[grp], sizeof(GroupLeadershipAA_Struct));

	to->FastQueuePacket(&outapp);
}

void Raid::GroupUpdate(uint32 gid, bool initial)
{
	//ungrouped member doesn't need grouping.
	if (gid >= MAX_RAID_GROUPS) {
		return;
	}

	for (const auto& m : members) {
		if (m.member && m.group_number == gid && strlen(m.member_name) > 0) {
			SendGroupUpdate(m.member);
			SendGroupLeadershipAA(m.member, gid);
		}
	}
	if (initial) {
		auto pack = new ServerPacket(ServerOP_UpdateGroup, sizeof(ServerRaidGeneralAction_Struct));
		auto  rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
		rga->gid = gid;
		rga->rid = GetID();
		rga->zoneid = zone->GetZoneID();
		rga->instance_id = zone->GetInstanceID();
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void Raid::SendRaidLock()
{
	auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
	auto rg = (RaidGeneral_Struct*)outapp->pBuffer;
	rg->action = raidLock;
	strn0cpy(rg->leader_name, leadername, 64);
	strn0cpy(rg->player_name, leadername, 64);
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Raid::SendRaidUnlock()
{
	auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
	auto rg = (RaidGeneral_Struct*)outapp->pBuffer;
	rg->action = raidUnlock;
	strn0cpy(rg->leader_name, leadername, 64);
	strn0cpy(rg->player_name, leadername, 64);
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Raid::SendRaidLockTo(Client *c)
{
	if (!c) {
		return;
	}

	auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
	auto rg = (RaidGeneral_Struct*)outapp->pBuffer;
	rg->action = raidLock;
	strn0cpy(rg->leader_name, c->GetName(), 64);
	strn0cpy(rg->player_name, c->GetName(), 64);
	c->QueuePacket(outapp);
	safe_delete(outapp);
}

void Raid::SendRaidUnlockTo(Client *c)
{
	if (!c) {
		return;
	}

	auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
	auto rg = (RaidGeneral_Struct*)outapp->pBuffer;
	rg->action = raidUnlock;
	strn0cpy(rg->leader_name, c->GetName(), 64);
	strn0cpy(rg->player_name, c->GetName(), 64);
	c->QueuePacket(outapp);
	safe_delete(outapp);
}

void Raid::SendGroupDisband(Client *to)
{
	if (!to) {
		return;
	}

	auto outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupUpdate_Struct));
	auto gu = (GroupUpdate_Struct*) outapp->pBuffer;
	gu->action = groupActDisband;
	strn0cpy(gu->leadersname, leadername, 64);
	strn0cpy(gu->yourname, to->GetName(), 64);
	to->FastQueuePacket(&outapp);
}

void Raid::SendRaidGroupAdd(const char *who, uint32 gid)
{
	auto pack = new ServerPacket(ServerOP_RaidGroupAdd, sizeof(ServerRaidGroupAction_Struct));
	auto rga = (ServerRaidGroupAction_Struct*)pack->pBuffer;
	rga->rid = GetID();
	rga->gid = gid;
	strn0cpy(rga->membername, who, 64);
	safe_delete(pack);
}

void Raid::SendRaidGroupRemove(const char *who, uint32 gid)
{
	auto pack = new ServerPacket(ServerOP_RaidGroupRemove, sizeof(ServerRaidGroupAction_Struct));
	auto rga = (ServerRaidGroupAction_Struct*)pack->pBuffer;
	rga->rid = GetID();
	rga->gid = gid;
	strn0cpy(rga->membername, who, 64);
	safe_delete(pack);
}

void Raid::SendRaidMOTD(Client *c)
{
	if (!c || motd.empty() || c->IsBot()) {
		return;
	}

	auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidMOTD_Struct));
	auto data = (RaidMOTD_Struct*)outapp->pBuffer;

	data->general.action = raidSetMotd;
	data->general.parameter = 0;
	data->general.unknown1 = 0;
	strn0cpy(data->general.leader_name, c->GetName(), sizeof(c->GetName()));
	strn0cpy(data->general.player_name, GetLeaderName().c_str(), 64);
	strn0cpy(data->motd, motd.c_str(), sizeof(data->motd));

	c->QueuePacket(outapp);
	safe_delete(outapp);
}

void Raid::SendRaidMOTD()
{
	if (motd.empty()) {
		return;
	}

	for (const auto& m: members) {
		if (m.is_bot) {
			continue;
		}

		if (m.member) {
			SendRaidMOTD(m.member);
		}
	}
}

void Raid::SendRaidMOTDToWorld()
{
	if (motd.empty()) {
		return;
	}

	auto pack = new ServerPacket(ServerOP_RaidMOTD, sizeof(ServerRaidMOTD_Struct));
	auto smotd = (ServerRaidMOTD_Struct *)pack->pBuffer;
	smotd->rid = GetID();
	strn0cpy(smotd->motd, motd.c_str(), sizeof(smotd->motd));
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Raid::SendGroupLeadershipAA(Client *c, uint32 gid)
{
	if (RuleB(Bots, Enabled) && entity_list.GetBotByBotName(c->GetName())) {
		return;
	}

	auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidLeadershipUpdate_Struct));
	auto rlaa = (RaidLeadershipUpdate_Struct *)outapp->pBuffer;
	rlaa->action = raidSetLeaderAbilities;
	strn0cpy(rlaa->leader_name, c->GetName(), 64);
	strn0cpy(rlaa->player_name, c->GetName(), 64);
	if (gid != RAID_GROUPLESS)
		memcpy(&rlaa->group, &group_aa[gid], sizeof(GroupLeadershipAA_Struct));
	memcpy(&rlaa->raid, &raid_aa, sizeof(RaidLeadershipAA_Struct));
	c->QueuePacket(outapp);
	safe_delete(outapp);
}

void Raid::SendGroupLeadershipAA(uint32 gid)
{
	for (const auto& m : members) {
		if (m.is_bot) {
			continue;
		}

		if (m.member && m.group_number == gid) {
			SendGroupLeadershipAA(m.member, gid);
		}
	}
}

void Raid::SendAllRaidLeadershipAA()
{
	for (const auto& m : members) {
		if (m.is_bot) {
			continue;
		}

		if (m.member) {
			SendGroupLeadershipAA(m.member, m.group_number);
		}
	}
}


void Raid::LockRaid(bool lockFlag)
{
	std::string query = StringFormat("UPDATE raid_details SET locked = %d WHERE raidid = %lu", lockFlag, (unsigned long)GetID());
	auto results = database.QueryDatabase(query);

	locked = lockFlag;

	if (lockFlag) {
		SendRaidLock();
	}
	else {
		SendRaidUnlock();
	}

	auto pack = new ServerPacket(ServerOP_RaidLockFlag, sizeof(ServerRaidGeneralAction_Struct));
	auto rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
	rga->rid = GetID();
	rga->zoneid = zone->GetZoneID();
	rga->gid = lockFlag;
	rga->instance_id = zone->GetInstanceID();
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Raid::SetRaidDetails()
{
	std::string query = StringFormat("INSERT INTO raid_details SET raidid = %lu, loottype = 4, locked = 0, motd = ''",
									 (unsigned long)GetID());
	auto results = database.QueryDatabase(query);
}

void Raid::GetRaidDetails()
{
	auto raid_details = RaidDetailsRepository::FindOne(database, GetID());
	if (raid_details.raidid == 0) {
		return;
	}

	locked   = raid_details.locked;
	LootType = raid_details.loottype;
	motd     = raid_details.motd;
	marked_npcs[0].entity_id   = raid_details.marked_npc_1_entity_id;
	marked_npcs[0].zone_id     = raid_details.marked_npc_1_zone_id;
	marked_npcs[0].instance_id = raid_details.marked_npc_1_instance_id;
	marked_npcs[1].entity_id   = raid_details.marked_npc_2_entity_id;
	marked_npcs[1].zone_id     = raid_details.marked_npc_2_zone_id;
	marked_npcs[1].instance_id = raid_details.marked_npc_2_instance_id;
	marked_npcs[2].entity_id   = raid_details.marked_npc_3_entity_id;
	marked_npcs[2].zone_id     = raid_details.marked_npc_3_zone_id;
	marked_npcs[2].instance_id = raid_details.marked_npc_3_instance_id;
}

void Raid::SaveRaidMOTD()
{
	std::string query = StringFormat("UPDATE raid_details SET motd = '%s' WHERE raidid = %lu",
			Strings::Escape(motd).c_str(), (unsigned long)GetID());

	auto results = database.QueryDatabase(query);
}

bool Raid::LearnMembers()
{
	EmptyRaidMembers();

	auto raid_members = RaidMembersRepository::GetWhere(
			database,
			fmt::format(
					"raidid = {}",
					GetID()
			)
	);

	if (raid_members.empty()) {
		disbandCheck = true;
		return false;
	}

	int i = 0;
	for (const auto &e: raid_members) {
		if (e.name.empty()) {
			continue;
		}

		members[i].member = nullptr;
		strn0cpy(members[i].member_name, e.name.c_str(), sizeof(members[i].member_name));
		uint32 group_id = e.groupid;
		if (!e.note.empty()) {
			members[i].note = e.note;
		}

		if (group_id >= MAX_RAID_GROUPS) {
			members[i].group_number = RAID_GROUPLESS;
		}
		else {
			members[i].group_number = group_id;
		}

		members[i]._class          = e._class;
		members[i].level           = e.level;
		members[i].is_group_leader = e.isgroupleader;
		members[i].is_raid_leader  = e.israidleader;
		members[i].is_looter       = e.islooter;
		members[i].main_marker     = e.is_marker;
		members[i].main_assister   = e.is_assister;
		members[i].is_bot          = e.bot_id > 0;
		++i;
	}
	return true;
}

void Raid::VerifyRaid()
{
	for (auto& m : members) {
		if (strlen(m.member_name) == 0) {
			m.member = nullptr;
		}
		else {
			auto* c = entity_list.GetClientByName(m.member_name);
			auto* b = entity_list.GetBotByBotName(m.member_name);

			if (c) {
				m.member = c;
				m.is_bot = false;
			}
			else if (RuleB(Bots, Enabled) && b) {
				//Raid requires client* we are forcing it here to be a BOT.  Care is needed here as any client function that
				//does not exist within the Bot Class will likely corrupt memory for the member object. Good practice to test the is_bot
				//attribute before calling a client function or casting to client.
				b = entity_list.GetBotByBotName(m.member_name);
				m.member = b->CastToClient();
				m.is_bot = true;
			}
			else {
				m.member = nullptr;
			}

			for (int i = 0; i < MAX_NO_RAID_MAIN_MARKERS; i++) {
				if (m.main_marker == i + 1) {
					strcpy(main_marker_pcs[i], m.member_name);
				}
			}

			for (int i = 0; i < MAX_NO_RAID_MAIN_ASSISTERS; i++) {
				if (m.main_assister == i + 1) {
					strcpy(main_assister_pcs[i], m.member_name);
				}
			}
		}

		if (m.is_raid_leader) {
			if (strlen(m.member_name) > 0) {
				SetLeader(m.member);
				strn0cpy(leadername, m.member_name, sizeof(leadername));
			}
			else {
				SetLeader(nullptr);
			}
		}
	}
}

void Raid::MemberZoned(Client *c)
{
	if (!c) {
		return;
	}

	if (leader == c) {
		leader = nullptr;
	}

	// Raid::GetGroup() goes over the members as well, this way we go over once
	uint32 gid = RAID_GROUPLESS;
	for (auto& m : members) {
		if (m.member == c) {
			m.member = nullptr;
			gid = m.group_number;
		}
	}

	if (gid < MAX_RAID_GROUPS && group_mentor[gid].mentoree == c) {
		group_mentor[gid].mentoree = nullptr;
	}
}

void Raid::SendHPManaEndPacketsTo(Client *client)
{
	if (!client) {
		return;
	}

	uint32 group_id = GetGroup(client);

	EQApplicationPacket hp_packet;
	EQApplicationPacket outapp(OP_MobManaUpdate, sizeof(MobManaUpdate_Struct));

	for (const auto& m : members) {
		if (m.is_bot) {
			continue;
		}

		if (m.member && (m.member != client) && (m.group_number == group_id)) {
			m.member->CreateHPPacket(&hp_packet);
			client->QueuePacket(&hp_packet, false);
			safe_delete_array(hp_packet.pBuffer);
			hp_packet.size = 0;

			if (client->ClientVersion() >= EQ::versions::ClientVersion::SoD) {
				outapp.SetOpcode(OP_MobManaUpdate);
				auto mana_update = (MobManaUpdate_Struct *)outapp.pBuffer;
				mana_update->spawn_id = m.member->GetID();
				mana_update->mana = m.member->GetManaPercent();
				client->QueuePacket(&outapp, false);

				outapp.SetOpcode(OP_MobEnduranceUpdate);
				auto endurance_update = (MobEnduranceUpdate_Struct *)outapp.pBuffer;
				endurance_update->endurance = m.member->GetEndurancePercent();
				client->QueuePacket(&outapp, false);
			}
		}
	}
}

void Raid::SendHPManaEndPacketsFrom(Mob *mob)
{
	if (!mob) {
		return;
	}

	uint32 group_id = 0;

	if (mob->IsClient()) {
		group_id = GetGroup(mob->CastToClient());
	}

	EQApplicationPacket hpapp;
	EQApplicationPacket outapp(OP_MobManaUpdate, sizeof(MobManaUpdate_Struct));

	mob->CreateHPPacket(&hpapp);

	for (const auto& m : members) {
		if (m.is_bot) {
			continue;
		}

		if (m.member && (!mob->IsClient() || ((m.member != mob->CastToClient()) && (m.group_number == group_id)))) {
			m.member->QueuePacket(&hpapp, false);

			if (m.member->ClientVersion() >= EQ::versions::ClientVersion::SoD) {
				outapp.SetOpcode(OP_MobManaUpdate);
				MobManaUpdate_Struct *mana_update = (MobManaUpdate_Struct *)outapp.pBuffer;
				mana_update->spawn_id = mob->GetID();
				mana_update->mana = mob->GetManaPercent();
				m.member->QueuePacket(&outapp, false);

				outapp.SetOpcode(OP_MobEnduranceUpdate);
				MobEnduranceUpdate_Struct *endurance_update = (MobEnduranceUpdate_Struct *)outapp.pBuffer;
				endurance_update->endurance = mob->GetEndurancePercent();
				m.member->QueuePacket(&outapp, false);
			}
		}
	}
}

void Raid::SendManaPacketFrom(Mob *mob)
{
	if (!mob) {
		return;
	}

	uint32 group_id = 0;

	if (mob->IsClient()) {
		group_id = GetGroup(mob->CastToClient());
	}

	EQApplicationPacket outapp(OP_MobManaUpdate, sizeof(MobManaUpdate_Struct));

	for (const auto& m : members) {
		if (m.is_bot) {
			continue;
		}

		if (m.member && (!mob->IsClient() || ((m.member != mob->CastToClient()) && (m.group_number == group_id))) &&
			m.member->ClientVersion() >= EQ::versions::ClientVersion::SoD
		) {
			outapp.SetOpcode(OP_MobManaUpdate);
			MobManaUpdate_Struct *mana_update = (MobManaUpdate_Struct *)outapp.pBuffer;
			mana_update->spawn_id = mob->GetID();
			mana_update->mana = mob->GetManaPercent();
			m.member->QueuePacket(&outapp, false);
		}
	}
}

void Raid::SendEndurancePacketFrom(Mob *mob)
{
	if (!mob) {
		return;
	}

	uint32 group_id = 0;

	if (mob->IsClient()) {
		group_id = GetGroup(mob->CastToClient());
	}

	EQApplicationPacket outapp(OP_MobManaUpdate, sizeof(MobManaUpdate_Struct));

	for (const auto& m : members) {
		if (m.is_bot) {
			continue;
		}

		if (m.member && (!mob->IsClient() || ((m.member != mob->CastToClient()) && (m.group_number == group_id))) &&
			m.member->ClientVersion() >= EQ::versions::ClientVersion::SoD
		) {
			outapp.SetOpcode(OP_MobEnduranceUpdate);
			auto endurance_update = (MobEnduranceUpdate_Struct *)outapp.pBuffer;
			endurance_update->spawn_id = mob->GetID();
			endurance_update->endurance = mob->GetEndurancePercent();
			m.member->QueuePacket(&outapp, false);
		}
	}
}

uint16 Raid::GetAvgLevel()
{
	double levelHolder = 0;
	uint8 i = 0;
	uint8 numMem = 0;
	while (i < MAX_RAID_MEMBERS) {
		if (strlen(members[i].member_name)) {
			levelHolder = levelHolder + members[i].level;
			numMem++;
		}
		i++;
	}
	levelHolder = ((levelHolder/(numMem))+.5); // total levels divided by num of characters
	return uint16(levelHolder);
}

const char *Raid::GetClientNameByIndex(uint8 index)
{
	return members[index].member_name;
}

void Raid::RaidMessageString(Mob* sender, uint32 type, uint32 string_id, const char* message,const char* message2,const char* message3,const char* message4,const char* message5,const char* message6,const char* message7,const char* message8,const char* message9, uint32 distance)
{
	for (const auto& m : members) {
		if (m.is_bot) {
			continue;
		}

		if (m.member && m.member->IsClient() && m.member != sender) {
			m.member->MessageString(type, string_id, message, message2, message3, message4, message5, message6,
									message7, message8, message9, distance);
		}
	}
}

void Raid::LoadLeadership()
{
	database.GetRaidLeadershipInfo(GetID(), nullptr, nullptr, nullptr, nullptr, &raid_aa);

	char mentor_name[64];
	for (uint32 group_id = 0; group_id < MAX_RAID_GROUPS; group_id++) {
		database.GetGroupLeadershipInfo(group_id, GetID(), nullptr, nullptr, nullptr, nullptr,
				mentor_name, &group_mentor[group_id].mentor_percent, &group_aa[group_id]);

		if (strlen(mentor_name)) {
			group_mentor[group_id].name = mentor_name;
			mentor_name[0] = '\0';
		}
	}
}

void Raid::SetGroupMentor(uint32 group_id, int percent, char *name)
{
	if (group_id >= MAX_RAID_GROUPS) {
		return;
	}

	group_mentor[group_id].name = name;
	group_mentor[group_id].mentor_percent = percent;
	Client *client = entity_list.GetClientByName(name);
	group_mentor[group_id].mentoree = client ? client : nullptr;

	std::string query = StringFormat("UPDATE raid_leaders SET mentoree = '%s', mentor_percent = %i WHERE gid = %i AND rid = %i LIMIT 1",
			name, percent, group_id, GetID());
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		LogError("Unable to set raid group mentor: [{}]\n", results.ErrorMessage().c_str());
}

void Raid::ClearGroupMentor(uint32 group_id)
{
	if (group_id >= MAX_RAID_GROUPS) {
		return;
	}
	group_mentor[group_id].name.clear();
	group_mentor[group_id].mentor_percent = 0;
	group_mentor[group_id].mentoree = nullptr;

	std::string query = StringFormat("UPDATE raid_leaders SET mentoree = '', mentor_percent = 0 WHERE gid = %i AND rid = %i LIMIT 1",
			group_id, GetID());
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		LogError("Unable to clear raid group mentor: [{}]\n", results.ErrorMessage().c_str());
	}
}

// there isn't a nice place to add this in another function, unlike groups
// so we do it here instead
void Raid::CheckGroupMentor(uint32 group_id, Client *c)
{
	if (!c || group_id >= MAX_RAID_GROUPS) {
		return;
	}

	if (group_mentor[group_id].name == c->GetName())
		group_mentor[group_id].mentoree = c;
}

void Raid::SetDirtyAutoHaters()
{
	for (const auto& m: members) {
		if (m.member && m.is_bot) {
			m.member->CastToBot()->SetDirtyAutoHaters();
		}
		else if (m.member) {
			m.member->SetDirtyAutoHaters();
		}
	}
}

void Raid::QueueClients(Mob *sender, const EQApplicationPacket *app, bool ack_required /*= true*/, bool ignore_sender /*= true*/, float distance /*= 0*/, bool group_only /*= true*/)
{
	if (sender && sender->IsClient()) {
		uint32 group_id = GetGroup(sender->CastToClient());

		/* If this is a group only packet and we're not in a group -- return */
		if (group_id == RAID_GROUPLESS && group_only) {
			return;
		}

		for (const auto& m : members) {
			if (!m.member) {
				continue;
			}

			if (m.is_bot) {
				continue;
			}

			if (m.member->IsClient()) {
				continue;
			}

			if (ignore_sender && m.member == sender) {
				continue;
			}

			if (group_only && m.group_number != group_id) {
				continue;
			}

			/* If we don't have a distance requirement - send to all members */
			if (distance == 0) {
				m.member->CastToClient()->QueuePacket(app, ack_required);
			}

			else {
				/* If negative distance - we check if current distance is greater than X */
				if (distance <= 0 && DistanceSquared(sender->GetPosition(), m.member->GetPosition()) >= (distance * distance)) {
					m.member->CastToClient()->QueuePacket(app, ack_required);
				}

				/* If positive distance - we check if current distance is less than X */
				else if (distance >= 0 && DistanceSquared(sender->GetPosition(), m.member->GetPosition()) <= (distance * distance)) {
					m.member->CastToClient()->QueuePacket(app, ack_required);
				}
			}
		}
	}
}

std::vector<RaidMember> Raid::GetMembers() const
{
	std::vector<RaidMember> raid_members;
	for (const auto& m : members) {
		if (m.member_name[0]) {
			raid_members.emplace_back(m);
		}
	}
	return raid_members;
}

bool Raid::AnyMemberHasDzLockout(const std::string& expedition, const std::string& event)
{
	std::vector<std::string> names;
	for (const auto& mbr : members)
	{
		if (!mbr.member && !mbr.is_bot && mbr.member_name[0])
		{
			names.emplace_back(mbr.member_name); // out of zone member
		}
		else if (mbr.member && !mbr.is_bot && mbr.member->HasDzLockout(expedition, event))
		{
			return true;
		}
	}
	return !CharacterExpeditionLockoutsRepository::GetLockouts(database, names, expedition, event).empty();
}

Mob* Raid::GetRaidMainAssistOne()
{
	for (int i = MAIN_ASSIST_1_SLOT; i < MAX_NO_RAID_MAIN_ASSISTERS; i++) {
		if (strlen(main_assister_pcs[i]) > 0) {
			auto ma = entity_list.GetMob(main_assister_pcs[i]);
			if (ma) {
				return ma;
			}
		}
	}
	return nullptr;
}

bool Raid::IsEngaged()
{

	for (const auto& m : GetMembers()) {
		if (m.is_bot) {
			continue;
		}

		if (m.member && (m.member->IsEngaged() || m.member->GetAggroCount() > 0)) {
			return true;
		}
	}
	return false;
}

void Raid::RaidGroupSay(const char* msg, const char* from, uint8 language, uint8 lang_skill)
{
	if (!from) {
		return;
	}

	uint32 group_id_to_use = GetGroup(from);

	if (group_id_to_use >= MAX_RAID_GROUPS) {
		return;
	}

	auto pack = new ServerPacket(ServerOP_RaidGroupSay, sizeof(ServerRaidMessage_Struct) + strlen(msg) + 1);
	auto rga = (ServerRaidMessage_Struct*)pack->pBuffer;
	rga->rid = GetID();
	rga->gid = group_id_to_use;
	rga->language = language;
	rga->lang_skill = lang_skill;
	strn0cpy(rga->from, from, 64);

	strcpy(rga->message, msg); // this is safe because we are allocating enough space for the entire msg above

	worldserver.SendPacket(pack);
	safe_delete(pack);
}
void Raid::RaidSay(const char* msg, const char* from, uint8 language, uint8 lang_skill)
{
	if (!from) {
		return;
	}

	auto pack = new ServerPacket(ServerOP_RaidSay, sizeof(ServerRaidMessage_Struct) + strlen(msg) + 1);
	auto rga = (ServerRaidMessage_Struct*)pack->pBuffer;
	rga->rid = GetID();
	rga->gid = RAID_GROUPLESS;
	rga->language = language;
	rga->lang_skill = lang_skill;
	strn0cpy(rga->from, from, 64);

	strcpy(rga->message, msg);

	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Raid::SetNewRaidLeader(uint32 i)
{
	if (members[i].is_raid_leader) {
		for (const auto& m : members) {
			if (m.is_bot) {
				continue;
			}

			if (m.member && strlen(m.member_name) > 0 && strcmp(m.member_name, members[i].member_name) != 0) {
				SetRaidLeader(members[i].member_name, m.member_name);
				UpdateRaidAAs();
				SendAllRaidLeadershipAA();
				break;
			}
		}
	}
}

void Raid::SaveRaidNote(std::string who, std::string note)
{
	if (who.empty() || note.empty()) {
		return;
	}

	auto result = RaidMembersRepository::UpdateRaidNote(database, GetID(), note, who);
	if (!result) {
		LogError("Unable to update the raid note for player [{}] in guild [{}].",
			who,
			GetID()
		);
	}
}

std::vector<RaidMember> Raid::GetMembersWithNotes()
{
	std::vector<RaidMember> raid_members;
	for (const auto& m : members) {
		if (!m.note.empty()) {
			raid_members.emplace_back(m);
		}
	}
	return raid_members;
}

void Raid::SendRaidNotes()
{
	LearnMembers();
	VerifyRaid();

	for (const auto& c : GetMembersWithNotes()) {

		auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidNote_Struct));
		auto data = (RaidNote_Struct*)outapp->pBuffer;

		data->general.action = raidSetNote;
		data->general.parameter = 0;
		data->general.unknown1 = 0;
		strn0cpy(data->general.leader_name, c.member_name, sizeof(c.member_name));
		strn0cpy(data->general.player_name, GetLeaderName().c_str(), GetLeaderName().length());
		strn0cpy(data->note, c.note.c_str(), sizeof(data->note));

		QueuePacket(outapp);
		safe_delete(outapp);
	}
}
void Raid::SendRaidNotesToWorld()
{
	auto pack = new ServerPacket(ServerOP_RaidNote, sizeof(ServerRaidNote_Struct));
	auto snote = (ServerRaidNote_Struct*)pack->pBuffer;
	snote->rid = GetID();
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Raid::DelegateAbilityAssist(Mob* delegator, const char* delegatee)
{
	auto raid_delegatee = entity_list.GetRaidByName(delegatee);
	if (!raid_delegatee) {
		delegator->CastToClient()->MessageString(Chat::Cyan, NOT_IN_YOUR_RAID, delegatee);
		return;
	}
	uint32 raid_delegatee_id = raid_delegatee->GetID();
	uint32 raid_delegator_id = GetID();
	if (raid_delegatee_id != raid_delegator_id) {
		delegator->CastToClient()->MessageString(Chat::Cyan, NOT_IN_YOUR_RAID, delegatee);
		return;
	}

	auto rm = &members[GetPlayerIndex(delegatee)];
	if (!rm) {
		return;
	}
	auto c = rm->member;
	if (!c) {
		return;
	}

	auto slot = FindNextRaidDelegateSlot(FindNextAssisterSlot);
	auto ma = rm->main_assister;
	if (slot == -1 && !ma) {
		delegator->CastToClient()->MessageString(Chat::Cyan, MAX_MAIN_RAID_ASSISTERS);
		return;
	}

	auto outapp = new EQApplicationPacket(OP_RaidDelegateAbility, sizeof(DelegateAbility_Struct));
	DelegateAbility_Struct* das = (DelegateAbility_Struct*)outapp->pBuffer;
	if (ma) {
		das->Action = ClearDelegate;
		memset(main_assister_pcs[ma - 1], 0, 64);
		rm->main_assister = DELEGATE_OFF;
		auto result = RaidMembersRepository::UpdateRaidAssister(
			database,
			GetID(),
			delegatee,
			DELEGATE_OFF
		);
		if (!result) {
			LogError("Unable to clear raid main assister for player: [{}].",
					 delegatee
			);
		}
	}
	else {
		if (slot >= MAIN_ASSIST_1_SLOT) {
			strcpy(main_assister_pcs[slot], delegatee);
			rm->main_assister = slot + 1;
			das->Action = SetDelegate;
			auto result = RaidMembersRepository::UpdateRaidAssister(
				database,
				GetID(),
				delegatee,
				slot + 1
			);
			if (!result) {
				LogError("Unable to set raid main assister for player: [{}] to [{}].",
						 delegatee,
						 slot + 1
				);
			}
		}
	}
	das->DelegateAbility = RaidDelegateMainAssist;
	das->MemberNumber = slot + 1;
	das->EntityID = c->GetID();
	strcpy(das->Name, delegatee);
	QueuePacket(outapp);
	safe_delete(outapp);
	UpdateRaidXTargets();
}

void Raid::UpdateRaidXTargets()
{
	struct AssistUpdate {
		XTargetType assist_type;
		XTargetType assist_target_type;
		int32       slot;
	};

	std::vector<AssistUpdate> assist_updates = {
		AssistUpdate{.assist_type = RaidAssist1, .assist_target_type = RaidAssist1Target, .slot = MAIN_ASSIST_1_SLOT},
		AssistUpdate{.assist_type = RaidAssist2, .assist_target_type = RaidAssist2Target, .slot = MAIN_ASSIST_2_SLOT},
		AssistUpdate{.assist_type = RaidAssist3, .assist_target_type = RaidAssist3Target, .slot = MAIN_ASSIST_3_SLOT},
	};

	for (const auto& u : assist_updates) {
		if (strlen(main_assister_pcs[u.slot]) > 0) {
			auto m = entity_list.GetMob(main_assister_pcs[u.slot]);
			if (m) {
				UpdateXTargetType(u.assist_type, m, m->GetName());
				auto n = m->GetTarget();
				if (n && n->GetHP() > 0) {
					UpdateXTargetType(u.assist_target_type, n, n->GetName());
				}
				else {
					UpdateXTargetType(u.assist_target_type, nullptr);
				}
			}
		}
		else {
			UpdateXTargetType(u.assist_type, nullptr);
			UpdateXTargetType(u.assist_target_type, nullptr);
		}
	}

	struct MarkedUpdate {
		XTargetType mark_target;
		int32       slot;
	};

	std::vector<MarkedUpdate> marked_updates = {
		MarkedUpdate{.mark_target = RaidMarkTarget1, .slot = MAIN_MARKER_1_SLOT},
		MarkedUpdate{.mark_target = RaidMarkTarget2, .slot = MAIN_MARKER_2_SLOT},
		MarkedUpdate{.mark_target = RaidMarkTarget3, .slot = MAIN_MARKER_3_SLOT},
	};

	for (auto& u : marked_updates) {
		if (marked_npcs[u.slot].entity_id) {
			auto m = entity_list.GetMob(marked_npcs[u.slot].entity_id);
			if (m && m->GetHP() > 0) {
				UpdateXTargetType(u.mark_target, m, m->GetName());
			}
			else {
				UpdateXTargetType(u.mark_target, nullptr);
			}
		}
		else {
			UpdateXTargetType(u.mark_target, nullptr);
		}
	}
}

void Raid::DelegateAbilityMark(Mob* delegator, const char* delegatee)
{
	auto raid_delegatee = entity_list.GetRaidByName(delegatee);
	if (!raid_delegatee) {
		delegator->CastToClient()->MessageString(Chat::Cyan, NOT_IN_YOUR_RAID, delegatee);
		return;
	}
	uint32 raid_delegatee_id = raid_delegatee->GetID();
	uint32 raid_delegator_id = GetID();
	if (raid_delegatee_id != raid_delegator_id) {
		delegator->CastToClient()->MessageString(Chat::Cyan, NOT_IN_YOUR_RAID, delegatee);
		return;
	}

	auto rm = &members[GetPlayerIndex(delegatee)];
	if (!rm) {
		return;
	}
	auto c = rm->member;
	if (!c) {
		return;
	}

	auto slot = FindNextRaidDelegateSlot(FindNextMarkerSlot);
	auto mm = rm->main_marker;

	if (slot == -1 && !mm) {
		delegator->CastToClient()->MessageString(Chat::Cyan, MAX_MAIN_RAID_MARKERS);
		return;
	}

	auto outapp = new EQApplicationPacket(OP_RaidDelegateAbility, sizeof(DelegateAbility_Struct));
	DelegateAbility_Struct* das = (DelegateAbility_Struct*)outapp->pBuffer;
	if (mm) {
		das->Action = ClearDelegate;
		memset(main_marker_pcs[mm - 1], 0, 64);
		rm->main_marker = DELEGATE_OFF;
		auto result = RaidMembersRepository::UpdateRaidMarker(
			database,
			GetID(),
			delegatee,
			DELEGATE_OFF
		);
		if (!result) {
			LogError("Unable to clear rain main marker for player: [{}].", delegatee);
		}
	}
	else {
		if (slot >= 0) {
			strcpy(main_marker_pcs[slot], c->GetName());
			rm->main_marker = slot + 1;
			das->Action = SetDelegate;
			auto result = RaidMembersRepository::UpdateRaidMarker(
				database,
				GetID(),
				delegatee,
				slot + 1
			);
			if (!result) {
				LogError("Unable to set raid main marker for player: [{}] to [{}].", delegatee, slot + 1);
			}
		}
	}
	das->DelegateAbility = RaidDelegateMainMarker;
	das->MemberNumber = 0;
	das->EntityID = c->GetID();
	strcpy(das->Name, delegatee);
	QueuePacket(outapp);
	safe_delete(outapp);
}

int Raid::FindNextRaidDelegateSlot(int option)
{
	if (option == FindNextRaidMainMarkerSlot) {
		for (int i = 0; i < MAX_NO_RAID_MAIN_MARKERS; i++) {
			if (strlen(main_marker_pcs[i]) == 0) {
				return i;
			}
		}
	}
	else if (option == FindNextRaidMainAssisterSlot) {
		for (int i = 0; i < MAX_NO_RAID_MAIN_ASSISTERS; i++) {
			if (strlen(main_assister_pcs[i]) == 0) {
				return i;
			}
		}
	}

	return -1;
}

void Raid::UpdateXTargetType(XTargetType Type, Mob *m, const char *name)
{
	for (const auto &rm: members) {
		if (!rm.member || rm.is_bot || !rm.member->XTargettingAvailable()) {
			continue;
		}

		for (int i = 0; i < rm.member->GetMaxXTargets(); ++i) {
			if (rm.member->XTargets[i].Type == Type) {
				if (m) {
					rm.member->XTargets[i].ID = m->GetID();
				}
				else {
					rm.member->XTargets[i].ID = 0;
				}

				if (name) {
					strn0cpy(rm.member->XTargets[i].Name, name, 64);
				}

				rm.member->SendXTargetPacket(i, m);
			}
		}
	}
}

void Raid::RaidMarkNPC(Mob* mob, uint32 parameter)
{
	Client* c = mob->CastToClient();
	if (!c || !c->GetTarget() || parameter < 1 || parameter > 3) {
		LogDebug("RaidMarkNPC Failed sanity checks.");
		return;
	}

	for (int i = 0; i < MAX_NO_RAID_MAIN_MARKERS; i++) {
		auto cname = c->GetCleanName();
		if (strcasecmp(main_marker_pcs[i], cname) == 0 || strcasecmp(leadername, cname) == 0) {
			marked_npcs[parameter - 1].entity_id = c->GetTarget()->GetID();
			marked_npcs[parameter - 1].zone_id = c->GetTarget()->GetZoneID();
			marked_npcs[parameter - 1].instance_id = c->GetTarget()->GetInstanceVersion();
			auto result = RaidDetailsRepository::UpdateRaidMarkedNPC(
				database,
				GetID(),
				marked_npcs[parameter - 1].entity_id,
				marked_npcs[parameter - 1].zone_id,
				marked_npcs[parameter - 1].instance_id,
				parameter
				);
			if (!result) {
				LogError("Unable to set MarkedNPC{} from slot: [{}] for guild [{}].",
					parameter,
					parameter - 1,
					GetID()
				);
			}

			auto outapp = new EQApplicationPacket(OP_MarkRaidNPC, sizeof(MarkNPC_Struct));
			MarkNPC_Struct* mnpcs = (MarkNPC_Struct*)outapp->pBuffer;
			mnpcs->TargetID = marked_npcs[parameter - 1].entity_id;
			mnpcs->Number = parameter;
			strcpy(mnpcs->Name, c->GetTarget()->GetCleanName());
			QueuePacket(outapp);
			safe_delete(outapp);
			UpdateXtargetMarkedNPC();
			return;
		}
	}
		//client is not delegated the mark ability
		c->MessageString(Chat::Cyan, NOT_DELEGATED_MARKER);
		return;
}

void Raid::UpdateXtargetMarkedNPC()
{
	for (int i = 0; i < MAX_MARKED_NPCS; i++) {
		auto mm = entity_list.GetNPCByID(marked_npcs[i].entity_id);
		if (mm) {
			UpdateXTargetType(static_cast<XTargetType>(RaidMarkTarget1 + i), mm->CastToMob(), mm->CastToMob()->GetName());
		}
		else {
			UpdateXTargetType(static_cast<XTargetType>(RaidMarkTarget1 + i), nullptr);
		}
	}
}

void Raid::RaidClearNPCMarks(Client* c)
{
	auto mob_id = c->GetID();

	if (Strings::EqualFold(main_marker_pcs[MAIN_MARKER_1_SLOT], c->GetCleanName()) ||
		Strings::EqualFold(main_marker_pcs[MAIN_MARKER_2_SLOT], c->GetCleanName()) ||
		Strings::EqualFold(main_marker_pcs[MAIN_MARKER_3_SLOT], c->GetCleanName())) {
		for (int i = 0; i < MAX_MARKED_NPCS; i++) {
			if (marked_npcs[i].entity_id > 0 && marked_npcs[i].zone_id == c->GetZoneID()
				&& marked_npcs[i].instance_id == c->GetInstanceID())
			{
				auto npc_name = entity_list.GetNPCByID(marked_npcs[i].entity_id)->GetCleanName();
				RaidMessageString(nullptr, Chat::Cyan, RAID_NO_LONGER_MARKED, npc_name);
				marked_npcs[i].entity_id = 0;
				marked_npcs[i].zone_id = 0;
				marked_npcs[i].instance_id = 0;
				auto result = RaidDetailsRepository::UpdateRaidMarkedNPC(
					database,
					GetID(),
					0,
					0,
					0,
					i + 1
				);
				if (!result) {
					LogError("Unable to clear MarkedNPC{} from slot: [{}] for guild [{}].", i + 1, i, GetID());
				}
			}
		}

		auto outapp = new EQApplicationPacket(OP_RaidClearNPCMarks, sizeof(MarkNPC_Struct));
		MarkNPC_Struct* mnpcs = (MarkNPC_Struct*)outapp->pBuffer;
		mnpcs->TargetID = 0;
		mnpcs->Number = 0;
		QueuePacket(outapp);
		safe_delete(outapp);
		UpdateXtargetMarkedNPC();
	}
	else {
		c->MessageString(Chat::Cyan, NOT_DELEGATED_MARKER);
	}
}

void Raid::RemoveRaidDelegates(const char* delegatee)
{
	auto ma = members[GetPlayerIndex(delegatee)].main_assister;
	auto mm = members[GetPlayerIndex(delegatee)].main_marker;

	if (ma) {
		SendRemoveRaidXTargets(static_cast<XTargetType>(RaidAssist1 + ma - 1));
		SendRemoveRaidXTargets(static_cast<XTargetType>(RaidAssist1Target + ma - 1));
		DelegateAbilityAssist(leader->CastToMob(), delegatee);
	}

	if (mm) {
		SendRemoveRaidXTargets(static_cast<XTargetType>(RaidMarkTarget1 + mm - 1));
		DelegateAbilityMark(leader->CastToMob(), delegatee);
	}
}

void Raid::SendRemoveAllRaidXTargets(const char* client_name)
{

	auto c = entity_list.GetClientByName(client_name);

	for (int i = 0; i < c->GetMaxXTargets(); ++i)
	{
		if ((c->XTargets[i].Type == RaidAssist1) ||
			(c->XTargets[i].Type == RaidAssist2) ||
			(c->XTargets[i].Type == RaidAssist3) ||
			(c->XTargets[i].Type == RaidAssist1Target) ||
			(c->XTargets[i].Type == RaidAssist2Target) ||
			(c->XTargets[i].Type == RaidAssist3Target) ||
			(c->XTargets[i].Type == RaidMarkTarget1) ||
			(c->XTargets[i].Type == RaidMarkTarget2) ||
			(c->XTargets[i].Type == RaidMarkTarget3))
		{
			c->XTargets[i].ID = 0;
			c->XTargets[i].Name[0] = 0;
			c->SendXTargetPacket(i, nullptr);
		}
	}
}

void Raid::SendRemoveRaidXTargets(XTargetType Type)
{
	for (const auto &m: members) {
		if (m.member && !m.is_bot) {
			for (int i = 0; i < m.member->GetMaxXTargets(); ++i) {
				if (m.member->XTargets[i].Type == Type) {
					m.member->XTargets[i].ID = 0;
					m.member->XTargets[i].Name[0] = 0;
					m.member->SendXTargetPacket(i, nullptr);
				}
			}
		}
	}
}

void Raid::SendRemoveAllRaidXTargets()
{
	for (const auto &m: members) {
		if (m.member && !m.is_bot) {
			for (int i = 0; i < m.member->GetMaxXTargets(); ++i) {
				if ((m.member->XTargets[i].Type == RaidAssist1) ||
					(m.member->XTargets[i].Type == RaidAssist2) ||
					(m.member->XTargets[i].Type == RaidAssist3) ||
					(m.member->XTargets[i].Type == RaidAssist1Target) ||
					(m.member->XTargets[i].Type == RaidAssist2Target) ||
					(m.member->XTargets[i].Type == RaidAssist3Target) ||
					(m.member->XTargets[i].Type == RaidMarkTarget1) ||
					(m.member->XTargets[i].Type == RaidMarkTarget2) ||
					(m.member->XTargets[i].Type == RaidMarkTarget3)) {
					m.member->XTargets[i].ID = 0;
					m.member->XTargets[i].Name[0] = 0;
					m.member->SendXTargetPacket(i, nullptr);
				}
			}
		}
	}
}

// Send a packet to the entire raid notifying them of the group target selected by the Main Assist.
void Raid::SendRaidAssistTarget()
{
	uint16 assist_target_id = 0;
	uint16 number = 0;
	Mob* target = nullptr;

	struct AssistTypes {
		MainAssistType main_assist_type_slot;
		MainAssistType main_assist_number;
	};

	std::vector<AssistTypes> assist_types = {
		{.main_assist_type_slot = MAIN_ASSIST_1_SLOT, .main_assist_number = MAIN_ASSIST_1},
		{.main_assist_type_slot = MAIN_ASSIST_2_SLOT, .main_assist_number = MAIN_ASSIST_2},
		{.main_assist_type_slot = MAIN_ASSIST_3_SLOT, .main_assist_number = MAIN_ASSIST_3}
	};

	for (auto &a: assist_types) {
		if (strlen(main_assister_pcs[a.main_assist_type_slot]) > 0) {
			auto player = entity_list.GetMob(main_assister_pcs[a.main_assist_type_slot]);
			if (player) {
				target = player->GetTarget();
				if (target) {
					assist_target_id = target->GetID();
					number           = a.main_assist_number;
					break;
				}
			}
		}
	}

	if (assist_target_id) {
		auto outapp = new EQApplicationPacket(OP_SetGroupTarget, sizeof(MarkNPC_Struct));
		MarkNPC_Struct* mnpcs = (MarkNPC_Struct*)outapp->pBuffer;
		mnpcs->TargetID = assist_target_id;
		mnpcs->Number = number;

		for (const auto& m : members) {
			if (m.member && !m.is_bot) {
				m.member->QueuePacket(outapp);
			}
		}
		safe_delete(outapp);
	}
}

void Raid::SendAssistTarget(Client *c)
{
	if (!c || c->IsBot()) {
		return;
	}

	uint16 assist_target_id = 0;
	uint16 number           = 0;
	Mob *target = nullptr;

	struct AssistTypes {
		MainAssistType main_assist_type_slot;
		MainAssistType main_assist_number;
	};

	std::vector<AssistTypes> assist_types = {
		{.main_assist_type_slot = MAIN_ASSIST_1_SLOT, .main_assist_number = MAIN_ASSIST_1},
		{.main_assist_type_slot = MAIN_ASSIST_2_SLOT, .main_assist_number = MAIN_ASSIST_2},
		{.main_assist_type_slot = MAIN_ASSIST_3_SLOT, .main_assist_number = MAIN_ASSIST_3}
	};

	for (auto &a: assist_types) {
		if (strlen(main_assister_pcs[a.main_assist_type_slot]) > 0) {
			auto player = entity_list.GetMob(main_assister_pcs[a.main_assist_type_slot]);
			if (player) {
				target = player->GetTarget();
				if (target) {
					assist_target_id = target->GetID();
					number           = a.main_assist_number;
					break;
				}
			}
		}
	}

	if (assist_target_id) {
		auto outapp = new EQApplicationPacket(OP_SetGroupTarget, sizeof(MarkNPC_Struct));
		MarkNPC_Struct *mnpcs = (MarkNPC_Struct *) outapp->pBuffer;
		mnpcs->TargetID = assist_target_id;
		mnpcs->Number   = number;
		c->QueuePacket(outapp);
		safe_delete(outapp);
	}
}

bool Raid::IsAssister(const char* who)
{
	for (auto & main_assister_pc : main_assister_pcs) {
		if (strcasecmp(main_assister_pc, who) == 0) {
			return true;
		}
	}

	return false;
}

void Raid::SendRaidAssisterTo(const char* assister, Client* to)
{
	if (strlen(assister) == 0 || !to || to->IsBot()) {
		return;
	}

	auto mob = entity_list.GetMob(assister);
	if (mob) {
		auto m_id = mob->GetID();
		if (m_id) {
			auto outapp = new EQApplicationPacket(OP_RaidDelegateAbility, sizeof(DelegateAbility_Struct));
			DelegateAbility_Struct* das = (DelegateAbility_Struct*)outapp->pBuffer;
			das->Action = SetDelegate;
			das->DelegateAbility = RaidDelegateMainAssist;
			das->MemberNumber = 0;
			das->EntityID = m_id;
			strcpy(das->Name, assister);
			to->QueuePacket(outapp);
			safe_delete(outapp);
		}
	}
}

void Raid::SendRaidAssister(const char* assister)
{
	if (strlen(assister) == 0) {
		return;
	}

	auto mob = entity_list.GetMob(assister);

	if (mob) {
		auto m_id = mob->GetID();
		if (m_id) {
			auto outapp = new EQApplicationPacket(OP_RaidDelegateAbility, sizeof(DelegateAbility_Struct));
			DelegateAbility_Struct* das = (DelegateAbility_Struct*)outapp->pBuffer;
			das->Action = SetDelegate;
			das->DelegateAbility = RaidDelegateMainAssist;
			das->MemberNumber = 0;
			das->EntityID = m_id;
			strcpy(das->Name, assister);
			QueuePacket(outapp);
			safe_delete(outapp);
		}
	}
}
bool Raid::IsMarker(const char* who)
{
	for (int i = 0; i < MAX_NO_RAID_MAIN_MARKERS; i++) {
		if (Strings::EqualFold(main_marker_pcs[i], who)) {
			return 1;
		}
	}
	return 0;
}

void Raid::SendRaidMarkerTo(const char* marker, Client* to)
{
	if (strlen(marker) == 0 || !to || to->IsBot()) {
		return;
	}

	auto mob = entity_list.GetMob(marker);
	if (mob) {
		auto m_id = mob->GetID();
		if (m_id) {
			auto outapp = new EQApplicationPacket(OP_RaidDelegateAbility, sizeof(DelegateAbility_Struct));
			DelegateAbility_Struct* das = (DelegateAbility_Struct*)outapp->pBuffer;
			das->Action = SetDelegate;
			das->DelegateAbility = RaidDelegateMainMarker;
			das->MemberNumber = 0;
			das->EntityID = m_id;
			strcpy(das->Name, marker);
			to->QueuePacket(outapp);
			safe_delete(outapp);
		}
	}
}

void Raid::SendRaidMarker(const char* marker)
{
	if (strlen(marker) == 0) {
		return;
	}

	auto mob = entity_list.GetMob(marker);
	if (mob) {
		auto m_id = mob->GetID();
		if (m_id) {
			auto outapp = new EQApplicationPacket(OP_RaidDelegateAbility, sizeof(DelegateAbility_Struct));
			DelegateAbility_Struct* das = (DelegateAbility_Struct*)outapp->pBuffer;
			das->Action = SetDelegate;
			das->DelegateAbility = RaidDelegateMainMarker;
			das->MemberNumber = 0;
			das->EntityID = m_id;
			strcpy(das->Name, marker);
			QueuePacket(outapp);
			safe_delete(outapp);
		}
	}
}

void Raid::SendMarkTargets(Client* c)
{
	if (!c || c->IsBot()) {
		return;
	}

	for (int i = 0; i < MAX_MARKED_NPCS; i++) {
		if (marked_npcs[i].entity_id > 0 && marked_npcs[i].zone_id == c->GetZoneID()
			&& marked_npcs[i].instance_id == c->GetInstanceID()) {
			auto marked_mob = entity_list.GetMob(marked_npcs[i].entity_id);
			if (marked_mob) {
				auto outapp = new EQApplicationPacket(OP_MarkRaidNPC, sizeof(MarkNPC_Struct));
				MarkNPC_Struct* mnpcs = (MarkNPC_Struct*)outapp->pBuffer;
				mnpcs->TargetID = marked_mob->GetID();
				mnpcs->Number = i + 1;
				strcpy(mnpcs->Name, marked_mob->GetCleanName());
				QueuePacket(outapp);
				safe_delete(outapp);
			}
		}
	}
	UpdateXtargetMarkedNPC();
}

void Raid::EmptyRaidMembers()
{
	for (int i = 0; i < MAX_RAID_MEMBERS; i++) {
		members[i].group_number    = RAID_GROUPLESS;
		members[i].is_group_leader = 0;
		members[i].level           = 0;
		members[i].main_assister   = 0;
		members[i].main_marker     = 0;
		members[i]._class          = 0;
		members[i].is_bot          = false;
		members[i].is_looter       = false;
		members[i].is_raid_leader  = false;
		members[i].is_raid_main_assist_one = false;
		members[i].member          = nullptr;
		members[i].note.clear();
		members[i].member_name[0]  = '\0';
	}
}
