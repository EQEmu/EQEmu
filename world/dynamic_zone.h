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

#include "common/dynamic_zone_base.h"
#include "common/rulesys.h"
#include "common/timer.h"

class Database;
class ServerPacket;

enum class DynamicZoneStatus
{
	Unknown = 0,
	Normal,
	Expired,
	ExpiredEmpty,
};

class DynamicZone : public DynamicZoneBase
{
public:
	using DynamicZoneBase::DynamicZoneBase; // inherit base constructors

	static DynamicZone* FindDynamicZoneByID(uint32_t dz_id);

	void SetSecondsRemaining(uint32_t seconds_remaining) override;

	void CacheMemberStatuses();
	DynamicZoneStatus Process();
	bool SetNewLeader(uint32_t member_id);

protected:
	Database& GetDatabase() override;
	void ProcessMemberAddRemove(const DynamicZoneMember& member, bool removed) override;
	bool ProcessMemberStatusChange(uint32_t character_id, DynamicZoneMemberStatus status) override;
	bool SendServerPacket(ServerPacket* packet) override;

private:
	friend class DynamicZoneManager;

	void CheckExpireWarning();
	void CheckLeader();
	void ChooseNewLeader();
	void SendZoneMemberStatuses(uint16_t zone_id, uint16_t instance_id);
	void SendZonesDurationUpdate();
	void SendZonesDynamicZoneDeleted();
	void SendZonesExpireWarning(uint32_t minutes_remaining);
	void SendZonesLeaderChanged();

	bool m_is_pending_early_shutdown = false;
	bool m_choose_leader_needed = false;
	Timer m_choose_leader_cooldown_timer{ static_cast<uint32_t>(RuleI(Expedition, ChooseLeaderCooldownTime)) };
	Timer m_warning_cooldown_timer{ 1 }; // non-zero so it's enabled initially
};
