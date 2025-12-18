/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2020 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#pragma once

#include "common/dynamic_zone_lockout.h"
#include "zone/dynamic_zone.h"

#include <cstdint>
#include <string>
#include <vector>

class Client;
class Group;
class Raid;

class ExpeditionRequest
{
public:
	ExpeditionRequest(const DynamicZone& dz, Client& client, bool silent = false);

	bool Validate();

	Client* GetLeaderClient() const { return m_leader; }
	uint32_t GetLeaderID() const { return m_leader_id; }
	const std::string& GetLeaderName() const { return m_leader_name; }
	const std::vector<DynamicZoneMember>& GetMembers() const { return m_members; }
	const std::vector<DzLockout>& GetLockouts() const { return m_lockouts; }
	bool IsRaid() const { return m_is_raid; }

private:
	bool CanMembersJoin(const std::vector<std::string>& member_names);
	bool CanRaidRequest(Raid* raid);
	bool CanGroupRequest(Group* group);
	bool CheckMembersForConflicts(const std::vector<std::string>& member_names);
	bool IsPlayerCountValidated();
	bool SaveLeaderLockouts(const std::vector<DzLockout>& leader_lockouts);
	void SendLeaderMemberInExpedition(const std::string& name, bool is_solo);
	void SendLeaderMemberReplayLockout(const std::string& name, const DzLockout& lockout, bool is_solo);
	void SendLeaderMemberEventLockout(const std::string& name, const DzLockout& lockout);
	void SendLeaderMessage(uint16_t chat_type, uint32_t string_id, std::initializer_list<std::string> args = {});

	const DynamicZone* m_dz         = nullptr;
	Client*  m_requester            = nullptr;
	Client*  m_leader               = nullptr;
	uint32_t m_leader_id            = 0;
	bool     m_silent               = false;
	bool     m_is_raid              = false;
	std::string m_leader_name;
	std::vector<DynamicZoneMember> m_members;
	std::vector<DzLockout> m_lockouts;
};
