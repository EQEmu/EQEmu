#include "dynamic_zone.h"

#include "common/eqemu_logsys.h"
#include "common/repositories/instance_list_repository.h"
#include "world/cliententry.h"
#include "world/clientlist.h"
#include "world/dynamic_zone_manager.h"
#include "world/worlddb.h"
#include "world/zonelist.h"
#include "world/zoneserver.h"

Database& DynamicZone::GetDatabase()
{
	return database;
}

bool DynamicZone::SendServerPacket(ServerPacket* packet)
{
	return ZSList::Instance()->SendPacket(packet);
}

DynamicZone* DynamicZone::FindDynamicZoneByID(uint32_t dz_id)
{
	auto dz = dynamic_zone_manager.dynamic_zone_cache.find(dz_id);
	if (dz != dynamic_zone_manager.dynamic_zone_cache.end())
	{
		return dz->second.get();
	}
	return nullptr;
}

void DynamicZone::ChooseNewLeader()
{
	if (m_members.empty() || !m_choose_leader_cooldown_timer.Check())
	{
		m_choose_leader_needed = true;
		return;
	}

	auto it = std::find_if(m_members.begin(), m_members.end(), [&](const DynamicZoneMember& member) {
		if (member.id != GetLeaderID() && member.IsOnline()) {
			auto member_cle = ClientList::Instance()->FindCLEByCharacterID(member.id);
			return (member_cle && member_cle->GetOnline() == CLE_Status::InZone);
		}
		return false;
	});

	if (it == m_members.end())
	{
		// no online members found, fallback to choosing any member
		it = std::find_if(m_members.begin(), m_members.end(),
			[&](const DynamicZoneMember& member) { return member.id != GetLeaderID(); });
	}

	if (it != m_members.end() && SetNewLeader(it->id))
	{
		m_choose_leader_needed = false;
	}
}

bool DynamicZone::SetNewLeader(uint32_t member_id)
{
	auto new_leader = GetMemberData(member_id);
	if (!new_leader.IsValid())
	{
		return false;
	}

	LogDynamicZonesDetail("Replacing dz [{}] leader [{}] with [{}]", GetID(), GetLeaderName(), new_leader.name);
	SetLeader(new_leader, true);
	SendZonesLeaderChanged();
	return true;
}

void DynamicZone::CheckLeader()
{
	if (m_choose_leader_needed)
	{
		ChooseNewLeader();
	}
}

DynamicZoneStatus DynamicZone::Process()
{
	DynamicZoneStatus status = DynamicZoneStatus::Normal;

	// force expire if no members
	if (!HasMembers() || IsExpired())
	{
		status = DynamicZoneStatus::Expired;

		auto dz_zoneserver = ZSList::Instance()->FindByInstanceID(GetInstanceID());
		if (!dz_zoneserver || dz_zoneserver->NumPlayers() == 0) // no clients inside dz
		{
			status = DynamicZoneStatus::ExpiredEmpty;

			if (!HasMembers() && !m_is_pending_early_shutdown && RuleB(DynamicZone, EmptyShutdownEnabled))
			{
				SetSecondsRemaining(RuleI(DynamicZone, EmptyShutdownDelaySeconds));
				m_is_pending_early_shutdown = true;
			}
		}
	}

	if (GetType() == DynamicZoneType::Expedition && status != DynamicZoneStatus::ExpiredEmpty)
	{
		CheckExpireWarning();
		CheckLeader();
	}

	return status;
}

void DynamicZone::SendZonesDynamicZoneDeleted()
{
	uint32_t pack_size = sizeof(ServerDzID_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_DzDeleted, pack_size);
	auto buf = reinterpret_cast<ServerDzID_Struct*>(pack->pBuffer);
	buf->dz_id = GetID();
	ZSList::Instance()->SendPacket(pack.get());
}

void DynamicZone::SetSecondsRemaining(uint32_t seconds_remaining)
{
	auto now = std::chrono::system_clock::now();
	auto new_remaining = std::chrono::seconds(seconds_remaining);

	auto current_remaining = m_expire_time - now;
	if (current_remaining > new_remaining) // reduce only
	{
		LogDynamicZonesDetail("Updating dynamic zone [{}] instance [{}] seconds remaining to [{}]s",
			GetID(), GetInstanceID(), seconds_remaining);

		// preserve original start time and adjust duration instead
		m_expire_time = now + new_remaining;
		m_duration = std::chrono::duration_cast<std::chrono::seconds>(m_expire_time - m_start_time);

		InstanceListRepository::UpdateDuration(
			database,
			GetInstanceID(),
			static_cast<uint32_t>(m_duration.count())
		);

		SendZonesDurationUpdate(); // update zone caches and actual instance's timer
	}
}

void DynamicZone::SendZonesDurationUpdate()
{
	constexpr uint32_t packsize = sizeof(ServerDzSetDuration_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_DzDurationUpdate, packsize);
	auto packbuf = reinterpret_cast<ServerDzSetDuration_Struct*>(pack->pBuffer);
	packbuf->dz_id = GetID();
	packbuf->seconds = static_cast<uint32_t>(m_duration.count());
	ZSList::Instance()->SendPacket(pack.get());
}

void DynamicZone::SendZonesLeaderChanged()
{
	uint32_t pack_size = sizeof(ServerDzLeaderID_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_DzLeaderChanged, pack_size);
	auto buf = reinterpret_cast<ServerDzLeaderID_Struct*>(pack->pBuffer);
	buf->dz_id = GetID();
	buf->leader_id = GetLeaderID();
	ZSList::Instance()->SendPacket(pack.get());
}

void DynamicZone::ProcessMemberAddRemove(const DynamicZoneMember& member, bool removed)
{
	DynamicZoneBase::ProcessMemberAddRemove(member, removed);

	if (GetType() == DynamicZoneType::Expedition && removed && member.id == GetLeaderID())
	{
		ChooseNewLeader();
	}
}

bool DynamicZone::ProcessMemberStatusChange(uint32_t character_id, DynamicZoneMemberStatus status)
{
	bool changed = DynamicZoneBase::SetInternalMemberStatus(character_id, status);
	if (changed && GetType() == DynamicZoneType::Expedition)
	{
		// any member status update will trigger a leader fix if leader was offline
		if (GetLeader().status == DynamicZoneMemberStatus::Offline && GetMemberCount() > 1)
		{
			ChooseNewLeader();
		}
	}
	return changed;
}

void DynamicZone::SendZonesExpireWarning(uint32_t minutes_remaining)
{
	uint32_t pack_size = sizeof(ServerDzExpireWarning_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_DzExpireWarning, pack_size);
	auto buf = reinterpret_cast<ServerDzExpireWarning_Struct*>(pack->pBuffer);
	buf->dz_id = GetID();
	buf->minutes_remaining = minutes_remaining;
	ZSList::Instance()->SendPacket(pack.get());
}

void DynamicZone::SendZoneMemberStatuses(uint16_t zone_id, uint16_t instance_id)
{
	uint32_t members_count = static_cast<uint32_t>(m_members.size());
	uint32_t entries_size = sizeof(ServerDzMemberStatusEntry_Struct) * members_count;
	uint32_t pack_size = sizeof(ServerDzMemberStatuses_Struct) + entries_size;
	auto pack = std::make_unique<ServerPacket>(ServerOP_DzGetMemberStatuses, pack_size);
	auto buf = reinterpret_cast<ServerDzMemberStatuses_Struct*>(pack->pBuffer);
	buf->dz_id = GetID();
	buf->count = members_count;

	for (int i = 0; i < m_members.size(); ++i)
	{
		buf->entries[i].character_id = m_members[i].id;
		buf->entries[i].online_status = static_cast<uint8_t>(m_members[i].status);
	}

	ZSList::Instance()->SendPacket(zone_id, instance_id, pack.get());
}

void DynamicZone::CacheMemberStatuses()
{
	if (m_has_member_statuses || m_members.empty())
	{
		return;
	}

	// called when a new dz is cached to fill member statuses
	std::string zone_name;
	std::vector<ClientListEntry*> all_clients;
	all_clients.reserve(ClientList::Instance()->GetClientCount());
	ClientList::Instance()->GetClients(zone_name.c_str(), all_clients);

	for (const auto& member : m_members)
	{
		auto it = std::find_if(all_clients.begin(), all_clients.end(),
			[&](const ClientListEntry* cle) { return (cle && cle->CharID() == member.id); });

		auto status = DynamicZoneMemberStatus::Offline;
		if (it != all_clients.end())
		{
			status = DynamicZoneMemberStatus::Online;
			if (IsSameDz((*it)->zone(), (*it)->instance()))
			{
				status = DynamicZoneMemberStatus::InDynamicZone;
			}
		}

		SetInternalMemberStatus(member.id, status);
	}

	m_has_member_statuses = true;
}

void DynamicZone::CheckExpireWarning()
{
	if (m_warning_cooldown_timer.Check(false))
	{
		using namespace std::chrono_literals;
		auto remaining = GetDurationRemaining();
		if ((remaining > 14min && remaining < 15min) ||
		    (remaining > 4min && remaining < 5min) ||
		    (remaining > 0min && remaining < 1min))
		{
			int minutes = std::chrono::duration_cast<std::chrono::minutes>(remaining).count() + 1;
			SendZonesExpireWarning(minutes);
			m_warning_cooldown_timer.Start(120000); // 2 minute cooldown after a warning
		}
	}
}
