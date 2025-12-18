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

#include "dynamic_zone.h"

#include "common/repositories/character_expedition_lockouts_repository.h"
#include "common/repositories/dynamic_zone_lockouts_repository.h"
#include "zone/client.h"
#include "zone/expedition_request.h"
#include "zone/string_ids.h"
#include "zone/worldserver.h"

#include "cereal/types/utility.hpp"

extern WorldServer worldserver;

// various expeditions use these strings when locking
constexpr char LockClose[] = "Your expedition is nearing its close. You cannot bring any additional people into your expedition at this time.";
constexpr char LockBegin[] = "The trial has begun. You cannot bring any additional people into your expedition at this time.";

DynamicZone::DynamicZone(uint32_t zone_id, uint32_t version, uint32_t duration, DynamicZoneType type)
{
	m_zone_id = zone_id;
	m_zone_version = version;
	m_duration = std::chrono::seconds(duration);
	m_type = type;
}

Database& DynamicZone::GetDatabase()
{
	return database;
}

bool DynamicZone::SendServerPacket(ServerPacket* packet)
{
	return worldserver.SendPacket(packet);
}

uint16_t DynamicZone::GetCurrentInstanceID() const
{
	return zone ? static_cast<uint16_t>(zone->GetInstanceID()) : 0;
}

uint16_t DynamicZone::GetCurrentZoneID() const
{
	return zone ? static_cast<uint16_t>(zone->GetZoneID()) : 0;
}

DynamicZone* DynamicZone::TryCreate(Client& client, DynamicZone& dzinfo, bool silent)
{
	// only expedition types are currently created in zone
	if (!zone || dzinfo.GetID() != 0 || !dzinfo.IsExpedition())
	{
		return nullptr;
	}

	// request parses leader, members list, and lockouts while validating
	ExpeditionRequest request(dzinfo, client, silent);
	if (!request.Validate())
	{
		LogExpeditions("[{}] request by [{}] denied", dzinfo.GetName(), client.GetName());
		return nullptr;
	}

	dzinfo.SetLeader({ request.GetLeaderID(), request.GetLeaderName(), DynamicZoneMemberStatus::Online });

	// this creates a new dz instance and saves it to both db and cache
	uint32_t dz_id = dzinfo.Create();
	if (dz_id == 0)
	{
		// live uses this message when trying to enter an instance that isn't ready
		// for now we can use it as a client error message if instance creation fails
		client.MessageString(Chat::Red, DZ_PREVENT_ENTERING);
		LogDynamicZones("Failed to create dynamic zone for zone [{}]", dzinfo.GetZoneID());
		return nullptr;
	}

	auto [it, ok] = zone->dynamic_zone_cache.try_emplace(dz_id, std::make_unique<DynamicZone>(dzinfo));

	DynamicZone* dz = it->second.get();
	dz->SaveMembers(request.GetMembers());
	dz->SaveLockouts(request.GetLockouts());

	dz->SendLeaderMessage(request.GetLeaderClient(), Chat::System, DZ_AVAILABLE, { dz->GetName() });
	if (dz->GetMemberCount() < request.GetMembers().size())
	{
		dz->SendLeaderMessage(request.GetLeaderClient(), Chat::System, fmt::format(DzNotAllAdded,
			request.IsRaid() ? "raid" : "group", "expedition", dz->GetMaxPlayers(), request.GetMembers().size()));
	}

	// world must be notified before we request async member updates
	auto pack = dz->CreateServerPacket(zone->GetZoneID(), zone->GetInstanceID());
	worldserver.SendPacket(pack.get());

	dz->UpdateMembers();

	LogDynamicZones("Created new dz [{}] for zone [{}]", dz_id, dz->GetZoneID());

	return dz;
}

void DynamicZone::CacheNewDynamicZone(ServerPacket* pack)
{
	auto buf = reinterpret_cast<ServerDzCreate_Struct*>(pack->pBuffer);

	// caching new dz created in world or another zone (has member statuses set by world)
	auto [it, ok] = zone->dynamic_zone_cache.try_emplace(buf->dz_id, std::make_unique<DynamicZone>());
	it->second->Unserialize({ buf->cereal_data, buf->cereal_size });
	it->second->UpdateMembers();

	LogDynamicZones("Cached new dynamic zone [{}]", buf->dz_id);
}

void DynamicZone::CacheAllFromDatabase()
{
	if (!zone)
	{
		return;
	}

	BenchTimer bench;

	auto dzs = DynamicZonesRepository::AllWithInstanceNotExpired(database);
	auto members = DynamicZoneMembersRepository::AllWithNames(database);
	auto lockouts = DynamicZoneLockoutsRepository::All(database);

	zone->dynamic_zone_cache.clear();
	zone->dynamic_zone_cache.reserve(dzs.size());

	for (auto& entry : dzs)
	{
		uint32_t dz_id = entry.id;
		auto dz = std::make_unique<DynamicZone>(std::move(entry));

		for (auto& member : members)
		{
			if (member.dynamic_zone_id == dz_id)
			{
				dz->AddMemberFromRepositoryResult(std::move(member));
			}
		}

		for (auto& lockout : lockouts)
		{
			if (lockout.dynamic_zone_id == dz->GetID())
			{
				dz->m_lockouts.emplace_back(dz->GetName(), std::move(lockout));
			}
		}

		zone->dynamic_zone_cache.emplace(dz_id, std::move(dz));
	}

	if (!zone->dynamic_zone_cache.empty())
	{
		RequestMemberStatuses();
	}

	LogInfo("Loaded [{}] dynamic zone(s)", Strings::Commify(zone->dynamic_zone_cache.size()));
	LogDynamicZones("Caching [{}] dynamic zone(s) took [{}s]", zone->dynamic_zone_cache.size(), bench.elapsed());
}

void DynamicZone::RequestMemberStatuses()
{
	ServerPacket pack(ServerOP_DzGetBulkMemberStatuses, sizeof(ServerDzCerealData_Struct));
	auto buf = reinterpret_cast<ServerDzCerealData_Struct*>(pack.pBuffer);
	buf->zone_id = static_cast<uint16_t>(zone->GetZoneID());
	buf->inst_id = static_cast<uint16_t>(zone->GetInstanceID());

	worldserver.SendPacket(&pack);
}

template <typename T>
DynamicZone* FindDynamicZone(T pred)
{
	if (zone)
	{
		for (const auto& [id_, dz] : zone->dynamic_zone_cache)
		{
			if (pred(*dz.get()))
			{
				return dz.get();
			}
		}
	}
	return nullptr;
}

DynamicZone* DynamicZone::FindDynamicZoneByID(uint32_t dz_id, DynamicZoneType type)
{
	if (zone)
	{
		auto it = zone->dynamic_zone_cache.find(dz_id);
		if (it != zone->dynamic_zone_cache.end() && (type == DynamicZoneType::None || it->second->GetType() == type))
		{
			return it->second.get();
		}
	}
	return nullptr;
}

DynamicZone* DynamicZone::FindExpeditionByCharacter(uint32_t char_id)
{
	return FindDynamicZone([&](const DynamicZone& dz) { return dz.IsExpedition() && dz.HasMember(char_id); });
}

DynamicZone* DynamicZone::FindExpeditionByZone(uint32_t zone_id, uint32_t instance_id)
{
	return FindDynamicZone([&](const DynamicZone& dz) { return dz.IsExpedition() && dz.IsSameDz(zone_id, instance_id); });
}

void DynamicZone::StartAllClientRemovalTimers()
{
	for (const auto& client_iter : entity_list.GetClientList())
	{
		if (client_iter.second)
		{
			client_iter.second->SetDzRemovalTimer(true);
		}
	}
}

bool DynamicZone::IsCurrentZoneDz() const
{
	return zone && zone->GetInstanceID() != 0 && zone->GetInstanceID() == GetInstanceID();
}

void DynamicZone::SetSecondsRemaining(uint32_t seconds_remaining)
{
	// async
	constexpr uint32_t pack_size = sizeof(ServerDzSetDuration_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_DzSetSecondsRemaining, pack_size);
	auto buf = reinterpret_cast<ServerDzSetDuration_Struct*>(pack->pBuffer);
	buf->dz_id = GetID();
	buf->seconds = seconds_remaining;
	worldserver.SendPacket(pack.get());
}

void DynamicZone::SetUpdatedDuration(uint32_t new_duration)
{
	// preserves original start time, just modifies duration and expire time
	m_duration = std::chrono::seconds(new_duration);
	m_expire_time = m_start_time + m_duration;

	LogDynamicZones("Updated dz [{}] zone [{}]:[{}] seconds remaining: [{}]",
		m_id, m_zone_id, m_instance_id, GetSecondsRemaining());

	if (zone && IsCurrentZoneDz())
	{
		zone->SetInstanceTimer(GetSecondsRemaining());
	}
}

void DynamicZone::SendClientInvite(Client* client, const std::string& inviter, const std::string& swap_name)
{
	if (!client)
	{
		return;
	}

	LogExpeditions("Invite [{}] to [{}] by [{}] swap [{}]", client->GetName(), GetID(), inviter, swap_name);

	client->SetPendingDzInvite({ GetID(), inviter, swap_name });
	client->MessageString(Chat::System, DZ_INVITED, GetLeaderName().c_str(), GetName().c_str());

	// live (as of March 11 2020 patch) warns for lockouts added during current
	// expedition that client would receive upon entering (sent in invite packet)
	std::string events;
	for (const auto& lockout : m_lockouts)
	{
		// live doesn't issue a warning for the dz's replay timer
		if (!lockout.IsReplay() && !lockout.IsExpired() && lockout.IsUUID(GetUUID()) &&
		    !client->HasDzLockout(GetName(), lockout.Event()))
		{
			auto time = lockout.GetTimeRemainingStrs();
			events += fmt::format("\n{} - {}D:{}H:{}M", lockout.Event(), time.days, time.hours, time.mins);
		}
	}

	if (!events.empty())
	{
		client->SendColoredText(Chat::System, fmt::format(
			"Warning! You will be given replay timers for the following events if you enter {}:{}", GetName(), events));
	}

	auto outapp = CreateInvitePacket(inviter, swap_name).release();
	client->FastQueuePacket(&outapp);
}

bool DynamicZone::ConfirmLeaderCommand(Client* client)
{
	if (!client)
	{
		return false;
	}

	if (!GetLeader().IsValid())
	{
		client->MessageString(Chat::Red, DZ_NO_LEADER_INFO); // unconfirmed message
		return false;
	}

	if (GetLeaderID() != client->CharacterID())
	{
		client->MessageString(Chat::System, DZ_NOT_LEADER, GetLeaderName().c_str());
		return false;
	}

	return true;
}

void DynamicZone::SendLeaderMessage(Client* leader, uint16_t type, const std::string& msg)
{
	Client::SendCrossZoneMessage(leader, GetLeaderName(), type, msg);
}

void DynamicZone::SendLeaderMessage(Client* leader, uint16_t type, uint32_t str_id, std::initializer_list<std::string> args)
{
	Client::SendCrossZoneMessageString(leader, GetLeaderName(), type, str_id, args);
}

bool DynamicZone::ProcessAddConflicts(Client* leader, Client* client, bool swapping)
{
	if (!client) // a null leader client is handled by SendLeaderMessage fallback
	{
		return true;
	}

	bool has_conflict = false;

	if (IsCurrentZoneDz())
	{
		SendLeaderMessage(leader, Chat::Red, DZADD_LEAVE_ZONE, { client->GetName() });
		has_conflict = true;
	}

	auto dz_id = client->GetExpeditionID();
	if (dz_id)
	{
		int string_id = dz_id == GetID() ? DZADD_ALREADY_PART : DZADD_ALREADY_OTHER;
		SendLeaderMessage(leader, Chat::Red, string_id, { client->GetName() });
		has_conflict = true;
	}

	// check any extra event lockouts for this expedition that the client has and expedition doesn't
	auto lockouts = client->GetDzLockouts(GetName());
	for (const auto& lockout : lockouts)
	{
		// client with a replay lockout is allowed only if the replay timer was from this expedition
		if (lockout.IsReplay() && lockout.UUID() != m_uuid)
		{
			has_conflict = true;

			auto time = lockout.GetTimeRemainingStrs();
			SendLeaderMessage(leader, Chat::Red, DZADD_REPLAY_TIMER, { client->GetName(), time.days, time.hours, time.mins });
		}
		else if (!lockout.IsReplay() && !HasLockout(lockout.Event()))
		{
			has_conflict = true;

			auto time = lockout.GetTimeRemainingStrs();
			SendLeaderMessage(leader, Chat::Red, DZADD_EVENT_TIMER, { client->GetName(), lockout.Event(), time.days, time.hours, time.mins });
		}
	}

	// member swapping integrity is handled by invite response
	if (!swapping)
	{
		auto member_count = GetDatabaseMemberCount();
		if (member_count == 0)
		{
			has_conflict = true;
		}
		else if (member_count >= GetMaxPlayers())
		{
			SendLeaderMessage(leader, Chat::Red, DZADD_EXCEED_MAX, { fmt::format_int(GetMaxPlayers()).str() });
			has_conflict = true;
		}
	}

	auto invite_id = client->GetPendingDzInviteID();
	if (invite_id)
	{
		int string_id = invite_id == GetID() ? DZADD_PENDING : DZADD_PENDING_OTHER;
		SendLeaderMessage(leader, Chat::Red, string_id, { client->GetName() });
		has_conflict = true;
	}

	return has_conflict;
}

void DynamicZone::TryAddClient(Client* client, const std::string& inviter, const std::string& swap_name, Client* leader)
{
	if (!client)
	{
		return;
	}

	LogExpeditions("Adding [{}] to [{}] by [{}] swap [{}]", client->GetName(), GetID(), inviter, swap_name);

	// null leader client handled by ProcessAddConflicts/SendLeaderMessage fallbacks
	if (!leader)
	{
		leader = entity_list.GetClientByName(inviter.c_str());
	}

	bool has_conflicts = ProcessAddConflicts(leader, client, !swap_name.empty());
	if (!has_conflicts)
	{
		// live uses the original unsanitized input string in invite messages
		uint32_t string_id = swap_name.empty() ? DZADD_INVITE : DZSWAP_INVITE;
		SendLeaderMessage(leader, Chat::Yellow, string_id, { client->GetName() });
		SendClientInvite(client, inviter, swap_name);
	}
	else if (swap_name.empty()) // swap command doesn't result in this message
	{
		SendLeaderMessage(leader, Chat::Red, DZADD_INVITE_FAIL, { client->GetName() });
	}
}

void DynamicZone::DzAddPlayer(Client* client, const std::string& add_name, const std::string& swap_name)
{
	if (!client || !ConfirmLeaderCommand(client))
	{
		return;
	}

	bool invite_failed = false;

	if (IsLocked())
	{
		client->MessageString(Chat::Red, DZADD_NOT_ALLOWING);
		invite_failed = true;
	}
	else if (add_name.empty())
	{
		client->MessageString(Chat::Red, DZADD_NOT_ONLINE, add_name.c_str());
		invite_failed = true;
	}
	else
	{
		auto member_data = GetMemberData(add_name);
		if (member_data.IsValid())
		{
			// live prioritizes offline message before already a member message
			if (member_data.status == DynamicZoneMemberStatus::Offline)
			{
				client->MessageString(Chat::Red, DZADD_NOT_ONLINE, add_name.c_str());
			}
			else
			{
				client->MessageString(Chat::Red, DZADD_ALREADY_PART, add_name.c_str());
			}
			invite_failed = true;
		}
	}

	if (invite_failed)
	{
		client->MessageString(Chat::Red, DZADD_INVITE_FAIL, FormatName(add_name).c_str());
		return;
	}

	if (Client* add_client = entity_list.GetClientByName(add_name.c_str()))
	{
		// client is online in this zone
		TryAddClient(add_client, client->GetName(), swap_name, client);
	}
	else
	{
		// forward to world to check if client is online and perform cross-zone invite
		SendWorldPlayerInvite(client->GetName(), swap_name, FormatName(add_name));
	}
}

void DynamicZone::DzAddPlayerContinue(std::string inviter, std::string add_name, std::string swap_name)
{
	// continuing expedition invite from leader in another zone
	if (Client* add_client = entity_list.GetClientByName(add_name.c_str()))
	{
		TryAddClient(add_client, inviter, swap_name);
	}
}

void DynamicZone::DzInviteResponse(Client* client, bool accepted, const std::string& swap_name)
{
	if (!client)
	{
		return;
	}

	LogExpeditions("[{}] accepted invite [{}] swap [{}]", client->GetName(), accepted, swap_name);

	// a null leader client is handled by SendLeaderMessage fallbacks
	// note current leader receives invite reply messages (if leader changed)
	Client* leader = entity_list.GetClientByCharID(GetLeaderID());

	if (!accepted)
	{
		SendLeaderMessage(leader, Chat::Red, DZ_INVITE_DECLINED, { client->GetName() });
		return;
	}

	bool is_swap = !swap_name.empty();
	bool has_conflicts = IsLocked();

	if (IsLocked())
	{
		SendLeaderMessage(leader, Chat::Red, DZADD_NOT_ALLOWING);
	}
	else
	{
		has_conflicts = ProcessAddConflicts(leader, client, is_swap);
	}

	// error if swapping and character was already removed before the accept
	if (is_swap)
	{
		auto swap_member = GetMemberData(swap_name);
		if (!swap_member.IsValid() || !HasDatabaseMember(swap_member.id))
		{
			has_conflicts = true;
		}
	}

	if (has_conflicts)
	{
		SendLeaderMessage(leader, Chat::Red, DZ_INVITE_ERROR, { client->GetName() });
	}
	else
	{
		SendLeaderMessage(leader, Chat::Yellow, DZ_INVITE_ACCEPTED, { client->GetName() });

		// replay timers are optionally added to new members on join with fresh expire time
		if (m_add_replay)
		{
			auto it = std::ranges::find_if(m_lockouts, [&](const auto& l) { return l.IsReplay(); });
			if (it != m_lockouts.end() && it->IsUUID(GetUUID()) && !client->HasDzLockout(GetName(), DzLockout::ReplayTimer))
			{
				DzLockout replay_timer = *it; // copy
				replay_timer.Reset();
				client->AddDzLockout(replay_timer, true);
			}
		}

		DynamicZoneMember add_member(client->CharacterID(), client->GetName(), DynamicZoneMemberStatus::Online);

		bool success = is_swap ? SwapMember(add_member, swap_name) : AddMember(add_member);
		if (success)
		{
			SendLeaderMessage(leader, Chat::Yellow, DZ_ADDED, { client->GetName(), GetName() });
		}
	}
}

void DynamicZone::DzMakeLeader(Client* client, std::string leader_name)
{
	if (!client || !ConfirmLeaderCommand(client))
	{
		return;
	}

	if (leader_name.empty())
	{
		client->MessageString(Chat::Red, DZ_LEADER_OFFLINE, leader_name.c_str());
		return;
	}

	// leader can only be changed by world
	SendWorldMakeLeaderRequest(client->CharacterID(), FormatName(leader_name));
}

void DynamicZone::DzRemovePlayer(Client* client, std::string name)
{
	if (!client || !ConfirmLeaderCommand(client))
	{
		return;
	}

	// live only seems to enforce min_players for requesting expeditions, no need to check here
	// note: on live members removed when inside a dz instance remain "temporary"
	// members for kick timer duration and still receive lockouts across zones (unimplemented)
	bool removed = RemoveMember(name);
	if (!removed)
	{
		client->MessageString(Chat::Red, DZ_NOT_MEMBER, FormatName(name).c_str());
	}
	else
	{
		client->MessageString(Chat::Yellow, DZ_REMOVED, FormatName(name).c_str(), m_name.c_str());
	}
}

void DynamicZone::DzQuit(Client* client)
{
	if (client)
	{
		RemoveMember(client->GetName());
	}
}

void DynamicZone::DzSwapPlayer(Client* client, std::string rem_name, std::string add_name)
{
	if (!client || !ConfirmLeaderCommand(client))
	{
		return;
	}

	if (rem_name.empty() || !HasMember(rem_name))
	{
		client->MessageString(Chat::Red, DZSWAP_CANNOT_REMOVE, FormatName(rem_name).c_str());
		return;
	}

	DzAddPlayer(client, add_name, rem_name);
}

void DynamicZone::DzPlayerList(Client* client)
{
	if (client)
	{
		client->MessageString(Chat::Yellow, DZ_LEADER, GetLeaderName().c_str());

		std::vector<std::string> names;
		for (const auto& member : m_members)
		{
			names.push_back(member.name);
		}

		client->MessageString(Chat::Yellow, DZ_MEMBERS, fmt::format("{}", fmt::join(names, ", ")).c_str());
	}
}

void DynamicZone::DzKickPlayers(Client* client)
{
	if (!client || !ConfirmLeaderCommand(client))
	{
		return;
	}

	RemoveAllMembers();
	client->MessageString(Chat::Red, DZ_REMOVED, "Everyone", m_name.c_str());
}

void DynamicZone::HandleWorldMessage(ServerPacket* pack)
{
	switch (pack->opcode)
	{
	case ServerOP_DzCreated:
	{
		auto buf = reinterpret_cast<ServerDzCreate_Struct*>(pack->pBuffer);
		if (zone && !zone->IsZone(buf->origin_zone_id, buf->origin_instance_id))
		{
			DynamicZone::CacheNewDynamicZone(pack);
		}
		break;
	}
	case ServerOP_DzDeleted:
	{
		// sent by world when it deletes an expired or empty dz
		// any system that held a reference to the dz should have already been notified
		auto buf = reinterpret_cast<ServerDzID_Struct*>(pack->pBuffer);
		auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
		if (zone && dz)
		{
			LogDynamicZonesDetail("Deleting dynamic zone [{}] from zone cache", buf->dz_id);
			dz->SendUpdatesToZoneMembers(true, true); // members silently removed
			zone->dynamic_zone_cache.erase(buf->dz_id);
		}
		break;
	}
	case ServerOP_DzAddPlayer:
	{
		auto buf = reinterpret_cast<ServerDzCommand_Struct*>(pack->pBuffer);
		if (buf->is_char_online)
		{
			if (auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id))
			{
				dz->DzAddPlayerContinue(buf->requester_name, buf->target_name, buf->remove_name);
			}
		}
		else if (Client* leader = entity_list.GetClientByName(buf->requester_name))
		{
			std::string target_name = FormatName(buf->target_name);
			leader->MessageString(Chat::Red, DZADD_NOT_ONLINE, target_name.c_str());
			leader->MessageString(Chat::Red, DZADD_INVITE_FAIL, target_name.c_str());
		}
		break;
	}
	case ServerOP_DzMakeLeader:
	{
		auto buf = reinterpret_cast<ServerDzCommandMakeLeader_Struct*>(pack->pBuffer);
		auto old_leader = entity_list.GetClientByCharID(buf->requester_id);

		// success flag is set by world to indicate new leader set to an online member
		if (old_leader && buf->is_success)
		{
			old_leader->MessageString(Chat::Yellow, DZ_LEADER_NAME, buf->new_leader_name);
		}
		else if (old_leader)
		{
			uint32_t str_id = buf->is_online ? DZ_NOT_MEMBER : DZ_LEADER_OFFLINE;
			old_leader->MessageString(Chat::Red, str_id, buf->new_leader_name);
		}

		if (buf->is_success && !RuleB(Expedition, AlwaysNotifyNewLeaderOnChange))
		{
			if (auto new_leader = entity_list.GetClientByName(buf->new_leader_name))
			{
				new_leader->MessageString(Chat::Yellow, DZ_LEADER_YOU);
			}
		}
		break;
	}
	case ServerOP_DzAddRemoveMember:
	{
		auto buf = reinterpret_cast<ServerDzMember_Struct*>(pack->pBuffer);
		if (zone && !zone->IsZone(buf->sender_zone_id, buf->sender_instance_id))
		{
			auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
			if (dz)
			{
				auto status = static_cast<DynamicZoneMemberStatus>(buf->character_status);
				dz->ProcessMemberAddRemove({ buf->character_id, buf->character_name, status }, buf->removed);
			}
		}

		if (zone && zone->IsZone(buf->dz_zone_id, buf->dz_instance_id))
		{
			// cache independent redundancy to kick removed members from dz's instance
			Client* client = entity_list.GetClientByCharID(buf->character_id);
			if (client)
			{
				client->SetDzRemovalTimer(buf->removed);
			}
		}
		break;
	}
	case ServerOP_DzSwapMembers:
	{
		auto buf = reinterpret_cast<ServerDzMemberSwap_Struct*>(pack->pBuffer);
		if (zone && !zone->IsZone(buf->sender_zone_id, buf->sender_instance_id))
		{
			auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
			if (dz)
			{
				auto status = static_cast<DynamicZoneMemberStatus>(buf->add_character_status);
				dz->ProcessMemberAddRemove({ buf->remove_character_id, buf->remove_character_name }, true);
				dz->ProcessMemberAddRemove({ buf->add_character_id, buf->add_character_name, status }, false);
			}
		}

		if (zone && zone->IsZone(buf->dz_zone_id, buf->dz_instance_id))
		{
			// cache independent redundancy to kick removed members from dz's instance
			Client* removed_client = entity_list.GetClientByCharID(buf->remove_character_id);
			if (removed_client)
			{
				removed_client->SetDzRemovalTimer(true);
			}

			Client* added_client = entity_list.GetClientByCharID(buf->add_character_id);
			if (added_client)
			{
				added_client->SetDzRemovalTimer(false);
			}
		}
		break;
	}
	case ServerOP_DzRemoveAllMembers:
	{
		auto buf = reinterpret_cast<ServerDzID_Struct*>(pack->pBuffer);
		if (zone && !zone->IsZone(buf->sender_zone_id, buf->sender_instance_id))
		{
			auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
			if (dz)
			{
				dz->ProcessRemoveAllMembers();
			}
		}

		if (zone && zone->IsZone(buf->dz_zone_id, buf->dz_instance_id))
		{
			// cache independent redundancy to kick removed members from dz's instance
			DynamicZone::StartAllClientRemovalTimers();
		}
		break;
	}
	case ServerOP_DzDurationUpdate:
	{
		auto buf = reinterpret_cast<ServerDzSetDuration_Struct*>(pack->pBuffer);
		auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
		if (dz)
		{
			dz->SetUpdatedDuration(buf->seconds);
		}
		break;
	}
	case ServerOP_DzSetCompass:
	case ServerOP_DzSetSafeReturn:
	case ServerOP_DzSetZoneIn:
	{
		auto buf = reinterpret_cast<ServerDzLocation_Struct*>(pack->pBuffer);
		if (zone && !zone->IsZone(buf->sender_zone_id, buf->sender_instance_id))
		{
			auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
			if (dz)
			{
				if (pack->opcode == ServerOP_DzSetCompass)
				{
					dz->SetCompass(buf->zone_id, buf->x, buf->y, buf->z, false);
				}
				else if (pack->opcode == ServerOP_DzSetSafeReturn)
				{
					dz->SetSafeReturn(buf->zone_id, buf->x, buf->y, buf->z, buf->heading, false);
				}
				else if (pack->opcode == ServerOP_DzSetZoneIn)
				{
					dz->SetZoneInLocation(buf->x, buf->y, buf->z, buf->heading, false);
				}
			}
		}
		break;
	}
	case ServerOP_DzSetSwitchID:
	{
		auto buf = reinterpret_cast<ServerDzSwitchID_Struct*>(pack->pBuffer);
		auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
		if (dz)
		{
			dz->ProcessSetSwitchID(buf->dz_switch_id);
		}
		break;
	}
	case ServerOP_DzGetMemberStatuses:
	{
		// reply from world for online member statuses request for async zone member updates
		auto buf = reinterpret_cast<ServerDzMemberStatuses_Struct*>(pack->pBuffer);
		auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
		if (dz)
		{
			for (uint32_t i = 0; i < buf->count; ++i)
			{
				auto status = static_cast<DynamicZoneMemberStatus>(buf->entries[i].online_status);
				dz->SetInternalMemberStatus(buf->entries[i].character_id, status);
			}
			dz->m_has_member_statuses = true;
			dz->SendUpdatesToZoneMembers(false, true);
		}
		break;
	}
	case ServerOP_DzGetBulkMemberStatuses:
	{
		if (zone)
		{
			std::vector<std::pair<uint32_t, std::vector<DynamicZoneMember>>> dzs;
			dzs.reserve(zone->dynamic_zone_cache.size());

			auto buf = reinterpret_cast<ServerDzCerealData_Struct*>(pack->pBuffer);
			EQ::Util::MemoryStreamReader ss(buf->cereal_data, buf->cereal_size);
			{
				cereal::BinaryInputArchive archive(ss);
				archive(dzs);
			}

			for (const auto& [dz_id, members] : dzs)
			{
				if (auto dz = DynamicZone::FindDynamicZoneByID(dz_id))
				{
					for (const auto& member : members)
					{
						dz->SetInternalMemberStatus(member.id, member.status);
					}
					dz->m_has_member_statuses = true;
				}
			}
		}
		break;
	}
	case ServerOP_DzUpdateMemberStatus:
	{
		auto buf = reinterpret_cast<ServerDzMemberStatus_Struct*>(pack->pBuffer);
		if (zone && !zone->IsZone(buf->sender_zone_id, buf->sender_instance_id))
		{
			auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
			if (dz)
			{
				auto status = static_cast<DynamicZoneMemberStatus>(buf->status);
				dz->ProcessMemberStatusChange(buf->character_id, status);
			}
		}
		break;
	}
	case ServerOP_DzLeaderChanged:
	{
		auto buf = reinterpret_cast<ServerDzLeaderID_Struct*>(pack->pBuffer);
		auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
		if (dz)
		{
			dz->ProcessLeaderChanged(buf->leader_id);
		}
		break;
	}
	case ServerOP_DzExpireWarning:
	{
		auto buf = reinterpret_cast<ServerDzExpireWarning_Struct*>(pack->pBuffer);
		auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
		if (dz)
		{
			dz->SendMembersExpireWarning(buf->minutes_remaining);
		}
		break;
	}
	case ServerOP_DzMovePC:
	{
		auto buf = reinterpret_cast<ServerDzMovePC_Struct*>(pack->pBuffer);
		auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
		if (dz)
		{
			Client* client = entity_list.GetClientByCharID(buf->character_id);
			if (client)
			{
				dz->MovePCInto(client, false);
			}
		}
		break;
	}
	case ServerOP_DzLock:
	{
		auto buf = reinterpret_cast<ServerDzLock_Struct*>(pack->pBuffer);
		if (zone && !zone->IsZone(buf->sender_zone_id, buf->sender_instance_id))
		{
			if (auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id))
			{
				dz->SetLocked(buf->lock, false, static_cast<DzLockMsg>(buf->lock_msg), buf->color);
			}
		}
		break;
	}
	case ServerOP_DzReplayOnJoin:
	{
		auto buf = reinterpret_cast<ServerDzBool_Struct*>(pack->pBuffer);
		if (zone && !zone->IsZone(buf->sender_zone_id, buf->sender_instance_id))
		{
			if (auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id))
			{
				dz->SetReplayOnJoin(buf->enabled);
			}
		}
		break;
	}
	case ServerOP_DzLockout:
	case ServerOP_DzLockoutDuration:
	{
		auto buf = reinterpret_cast<ServerDzLockout_Struct*>(pack->pBuffer);
		if (zone && !zone->IsZone(buf->sender_zone_id, buf->sender_instance_id))
		{
			if (DynamicZone* dz = DynamicZone::FindDynamicZoneByID(buf->dz_id))
			{
				DzLockout lockout{ dz->GetUUID(), dz->GetName(), buf->event_name, buf->expire_time, buf->duration };
				if (pack->opcode == ServerOP_DzLockout)
				{
					dz->HandleLockoutUpdate(lockout, buf->remove, buf->members_only);
				}
				else if (pack->opcode == ServerOP_DzLockoutDuration)
				{
					dz->HandleLockoutDuration(lockout, buf->seconds, buf->members_only, false);
				}
			}
		}
		break;
	}
	case ServerOP_DzCharacterLockout:
	{
		auto buf = reinterpret_cast<ServerDzCharacterLockout_Struct*>(pack->pBuffer);
		if (Client* client = entity_list.GetClientByCharID(buf->char_id))
		{
			if (!buf->remove)
			{
				client->AddDzLockout(DzLockout{ buf->uuid, buf->expedition, buf->event, buf->expire_time, buf->duration });
			}
			else if (buf->event[0] != '\0')
			{
				client->RemoveDzLockout(buf->expedition, buf->event);
			}
			else
			{
				client->RemoveDzLockouts(buf->expedition);
			}
		}
		break;
	}
	default:
		break;
	}
}

std::unique_ptr<EQApplicationPacket> DynamicZone::CreateExpireWarningPacket(uint32_t minutes_remaining)
{
	uint32_t outsize = sizeof(ExpeditionExpireWarning);
	auto outapp = std::make_unique<EQApplicationPacket>(OP_DzExpeditionEndsWarning, outsize);
	auto buf = reinterpret_cast<ExpeditionExpireWarning*>(outapp->pBuffer);
	buf->minutes_remaining = minutes_remaining;
	return outapp;
}

std::unique_ptr<EQApplicationPacket> DynamicZone::CreateInfoPacket(bool clear)
{
	constexpr uint32_t outsize = sizeof(DynamicZoneInfo_Struct);
	auto outapp = std::make_unique<EQApplicationPacket>(OP_DzExpeditionInfo, outsize);
	if (!clear)
	{
		auto info = reinterpret_cast<DynamicZoneInfo_Struct*>(outapp->pBuffer);
		info->assigned = true;
		strn0cpy(info->dz_name, m_name.c_str(), sizeof(info->dz_name));
		strn0cpy(info->leader_name, m_leader.name.c_str(), sizeof(info->leader_name));
		info->max_players = m_max_players;
	}
	return outapp;
}

std::unique_ptr<EQApplicationPacket> DynamicZone::CreateMemberListPacket(bool clear)
{
	uint32_t member_count = clear ? 0 : static_cast<uint32_t>(m_members.size());
	uint32_t member_entries_size = sizeof(DynamicZoneMemberEntry_Struct) * member_count;
	uint32_t outsize = sizeof(DynamicZoneMemberList_Struct) + member_entries_size;
	auto outapp = std::make_unique<EQApplicationPacket>(OP_DzMemberList, outsize);
	auto buf = reinterpret_cast<DynamicZoneMemberList_Struct*>(outapp->pBuffer);

	buf->member_count = member_count;

	if (!clear)
	{
		for (auto i = 0; i < m_members.size(); ++i)
		{
			strn0cpy(buf->members[i].name, m_members[i].name.c_str(), sizeof(buf->members[i].name));
			buf->members[i].online_status = static_cast<uint8_t>(m_members[i].status);
		}
	}

	return outapp;
}

std::unique_ptr<EQApplicationPacket> DynamicZone::CreateMemberNamePacket(const std::string& name, bool remove)
{
	constexpr uint32_t outsize = sizeof(DynamicZoneMemberListName_Struct);
	auto outapp = std::make_unique<EQApplicationPacket>(OP_DzMemberListName, outsize);
	auto buf = reinterpret_cast<DynamicZoneMemberListName_Struct*>(outapp->pBuffer);
	buf->add_name = !remove;
	strn0cpy(buf->name, name.c_str(), sizeof(buf->name));
	return outapp;
}

std::unique_ptr<EQApplicationPacket> DynamicZone::CreateMemberStatusPacket(const std::string& name, DynamicZoneMemberStatus status)
{
	// member list status uses member list struct with a single entry
	constexpr uint32_t outsize = sizeof(DynamicZoneMemberList_Struct) + sizeof(DynamicZoneMemberEntry_Struct);
	auto outapp = std::make_unique<EQApplicationPacket>(OP_DzMemberListStatus, outsize);
	auto buf = reinterpret_cast<DynamicZoneMemberList_Struct*>(outapp->pBuffer);
	buf->member_count = 1;

	auto entry = static_cast<DynamicZoneMemberEntry_Struct*>(buf->members);
	strn0cpy(entry->name, name.c_str(), sizeof(entry->name));
	entry->online_status = static_cast<uint8_t>(status);

	return outapp;
}

std::unique_ptr<EQApplicationPacket> DynamicZone::CreateLeaderNamePacket()
{
	constexpr uint32_t outsize = sizeof(DynamicZoneLeaderName_Struct);
	auto outapp = std::make_unique<EQApplicationPacket>(OP_DzSetLeaderName, outsize);
	auto buf = reinterpret_cast<DynamicZoneLeaderName_Struct*>(outapp->pBuffer);
	strn0cpy(buf->leader_name, m_leader.name.c_str(), sizeof(buf->leader_name));
	return outapp;
}

void DynamicZone::ProcessCompassChange(const DynamicZoneLocation& location)
{
	DynamicZoneBase::ProcessCompassChange(location);
	SendCompassUpdateToZoneMembers();
}

void DynamicZone::SendCompassUpdateToZoneMembers()
{
	for (const auto& member : m_members)
	{
		Client* member_client = entity_list.GetClientByCharID(member.id);
		if (member_client)
		{
			member_client->SendDzCompassUpdate();
		}
	}
}

void DynamicZone::ProcessSetSwitchID(int dz_switch_id)
{
	DynamicZoneBase::ProcessSetSwitchID(dz_switch_id);
	SendCompassUpdateToZoneMembers();
}

void DynamicZone::SendLeaderNameToZoneMembers()
{
	auto outapp_leader = CreateLeaderNamePacket();

	for (const auto& member : m_members)
	{
		Client* member_client = entity_list.GetClientByCharID(member.id);
		if (member_client)
		{
			member_client->QueuePacket(outapp_leader.get());

			if (member.id == m_leader.id && RuleB(Expedition, AlwaysNotifyNewLeaderOnChange))
			{
				member_client->MessageString(Chat::Yellow, DZ_LEADER_YOU);
			}
		}
	}
}

void DynamicZone::SendMembersExpireWarning(uint32_t minutes)
{
	// expeditions warn members in all zones not just the dz
	auto outapp = CreateExpireWarningPacket(minutes);
	for (const auto& member : GetMembers())
	{
		Client* client = entity_list.GetClientByCharID(member.id);
		if (client)
		{
			client->QueuePacket(outapp.get());

			// live doesn't actually send the chat message with it
			client->MessageString(Chat::Yellow, DZ_MINUTES_REMAIN, fmt::format_int(minutes).c_str());
		}
	}
}

void DynamicZone::SendMemberListToZoneMembers()
{
	auto outapp_members = CreateMemberListPacket(false);

	for (const auto& member : m_members)
	{
		Client* member_client = entity_list.GetClientByCharID(member.id);
		if (member_client)
		{
			member_client->QueuePacket(outapp_members.get());
		}
	}
}

void DynamicZone::SendMemberNameToZoneMembers(const std::string& char_name, bool remove)
{
	auto outapp = CreateMemberNamePacket(char_name, remove);

	for (const auto& member : m_members)
	{
		Client* member_client = entity_list.GetClientByCharID(member.id);
		if (member_client)
		{
			member_client->QueuePacket(outapp.get());
		}
	}
}

void DynamicZone::SendMemberStatusToZoneMembers(const DynamicZoneMember& update)
{
	auto outapp = CreateMemberStatusPacket(update.name, update.status);

	for (const auto& member : m_members)
	{
		Client* member_client = entity_list.GetClientByCharID(member.id);
		if (member_client)
		{
			member_client->QueuePacket(outapp.get());
		}
	}
}

void DynamicZone::SendClientWindowUpdate(Client* client)
{
	if (client)
	{
		client->QueuePacket(CreateInfoPacket().get());
		client->QueuePacket(CreateMemberListPacket().get());
	}
}

void DynamicZone::SendUpdatesToZoneMembers(bool removing_all, bool silent)
{
	// performs a full update on all members (usually for dz creation or removing all)
	if (!HasMembers())
	{
		return;
	}

	std::unique_ptr<EQApplicationPacket> outapp_info = nullptr;
	std::unique_ptr<EQApplicationPacket> outapp_members = nullptr;

	// only expeditions use the dz window. on live the window is filled by non
	// expeditions when first created but never kept updated. that behavior could
	// be replicated in the future by flagging this as a creation update
	if (m_type == DynamicZoneType::Expedition)
	{
		// clearing info also clears member list, no need to send both when removing
		outapp_info = CreateInfoPacket(removing_all);
		outapp_members = removing_all ? nullptr : CreateMemberListPacket();
	}

	for (const auto& member : GetMembers())
	{
		Client* client = entity_list.GetClientByCharID(member.id);
		if (client)
		{
			if (removing_all) {
				client->RemoveDynamicZoneID(GetID());
			} else {
				client->AddDynamicZoneID(GetID());
			}

			client->SendDzCompassUpdate();

			if (outapp_info)
			{
				client->QueuePacket(outapp_info.get());
			}

			if (outapp_members)
			{
				client->QueuePacket(outapp_members.get());
			}

			if (m_type == DynamicZoneType::Expedition && removing_all && !silent)
			{
				client->MessageString(Chat::Yellow, DZ_REMOVED, client->GetCleanName(), GetName().c_str());
			}
		}
	}
}

void DynamicZone::ProcessMemberAddRemove(const DynamicZoneMember& member, bool removed)
{
	DynamicZoneBase::ProcessMemberAddRemove(member, removed);

	// the affected client always gets a full compass update. for expeditions
	// client also gets window info update and all members get a member list update
	Client* client = entity_list.GetClientByCharID(member.id);
	if (client)
	{
		if (!removed) {
			client->AddDynamicZoneID(GetID());
		} else {
			client->RemoveDynamicZoneID(GetID());
		}

		client->SendDzCompassUpdate();

		if (m_type == DynamicZoneType::Expedition)
		{
			// sending clear info also clears member list for removed members
			client->QueuePacket(CreateInfoPacket(removed).get());
			client->MessageString(Chat::Yellow, removed ? DZ_REMOVED : DZ_ADDED, client->GetCleanName(), GetName().c_str());
		}
	}

	if (m_type == DynamicZoneType::Expedition)
	{
		// send full list when adding (MemberListName adds with "unknown" status)
		if (!removed) {
			SendMemberListToZoneMembers();
		} else {
			SendMemberNameToZoneMembers(member.name, true);
		}
	}
}

void DynamicZone::ProcessRemoveAllMembers()
{
	SendUpdatesToZoneMembers(true, false);
	DynamicZoneBase::ProcessRemoveAllMembers();
}

void DynamicZone::UpdateMembers()
{
	// gets member statuses from world and performs zone member updates on reply
	// if we've already received member statuses we can just update immediately
	if (m_has_member_statuses)
	{
		SendUpdatesToZoneMembers();
		return;
	}

	constexpr uint32_t pack_size = sizeof(ServerDzID_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_DzGetMemberStatuses, pack_size);
	auto buf = reinterpret_cast<ServerDzID_Struct*>(pack->pBuffer);
	buf->dz_id = GetID();
	buf->sender_zone_id = zone ? zone->GetZoneID() : 0;
	buf->sender_instance_id = zone ? zone->GetInstanceID() : 0;
	worldserver.SendPacket(pack.get());
}

bool DynamicZone::ProcessMemberStatusChange(uint32_t character_id, DynamicZoneMemberStatus status)
{
	bool changed = DynamicZoneBase::ProcessMemberStatusChange(character_id, status);

	if (changed && m_type == DynamicZoneType::Expedition)
	{
		auto member = GetMemberData(character_id);
		if (member.IsValid())
		{
			SendMemberStatusToZoneMembers(member);
		}
	}

	return changed;
}

void DynamicZone::ProcessLeaderChanged(uint32_t new_leader_id)
{
	auto new_leader = GetMemberData(new_leader_id);
	if (!new_leader.IsValid())
	{
		LogDynamicZones("Processed invalid new leader id [{}] for dz [{}]", new_leader_id, m_id);
		return;
	}

	LogDynamicZones("Replaced [{}] leader [{}] with [{}]", m_id, GetLeaderName(), new_leader.name);

	SetLeader(new_leader);
	if (GetType() == DynamicZoneType::Expedition)
	{
		SendLeaderNameToZoneMembers();
	}
}

bool DynamicZone::CanClientLootCorpse(Client* client, uint32_t npc_type_id, uint32_t entity_id)
{
	// non-members of a dz cannot loot corpses inside the dz
	if (!HasMember(client->CharacterID()))
	{
		return false;
	}

	// expeditions may prevent looting based on client's lockouts
	if (GetType() == DynamicZoneType::Expedition)
	{
		// entity id takes priority, falls back to checking by npc type if not set
		std::string event = GetLootEvent(entity_id, DzLootEvent::Type::Entity);
		if (event.empty())
		{
			event = GetLootEvent(npc_type_id, DzLootEvent::Type::NpcType);
		}

		if (!event.empty())
		{
			auto lockout = client->GetDzLockout(GetName(), event);
			if (!lockout || lockout->UUID() != m_uuid)
			{
				// client lockout not received in this expedition, prevent looting
				LogExpeditions("Character [{}] denied looting npc [{}] for [{}]", client->CharacterID(), npc_type_id, event);
				return false;
			}
		}
	}

	return true;
}

void DynamicZone::MovePCInto(Client* client, bool world_verify) const
{
	if (!world_verify)
	{
		DynamicZoneLocation zonein = GetZoneInLocation();
		ZoneMode zone_mode = HasZoneInLocation() ? ZoneMode::ZoneSolicited : ZoneMode::ZoneToSafeCoords;
		client->MovePC(GetZoneID(), GetInstanceID(), zonein.x, zonein.y, zonein.z, zonein.heading, 0, zone_mode);
	}
	else
	{
		ServerPacket pack(ServerOP_DzMovePC, sizeof(ServerDzMovePC_Struct));
		auto buf = reinterpret_cast<ServerDzMovePC_Struct*>(pack.pBuffer);
		buf->dz_id = GetID();
		buf->sender_zone_id = static_cast<uint16_t>(zone->GetZoneID());
		buf->sender_instance_id = static_cast<uint16_t>(zone->GetInstanceID());
		buf->character_id = client->CharacterID();
		worldserver.SendPacket(&pack);
	}
}

void DynamicZone::SetLocked(bool lock, bool update_db, DzLockMsg lock_msg, uint32_t color)
{
	DynamicZoneBase::SetLocked(lock, update_db, lock_msg, color);

	if (m_is_locked && lock_msg != DzLockMsg::None && IsCurrentZoneDz())
	{
		auto msg = lock_msg == DzLockMsg::Close ? LockClose : LockBegin;
		for (const auto& it : entity_list.GetClientList())
		{
			if (it.second)
			{
				it.second->Message(color, msg);
			}
		}
	}
}

void DynamicZone::SaveLockouts(const std::vector<DzLockout>& lockouts)
{
	m_lockouts = lockouts;
	DynamicZoneLockoutsRepository::InsertLockouts(database, m_id, m_lockouts);
}

static void SendWorldCharacterLockout(uint32_t char_id, const DzLockout& lockout, bool remove)
{
	uint32_t pack_size = sizeof(ServerDzCharacterLockout_Struct);
	ServerPacket pack(ServerOP_DzCharacterLockout, pack_size);
	auto buf = reinterpret_cast<ServerDzCharacterLockout_Struct*>(pack.pBuffer);
	buf->remove = remove;
	buf->char_id = char_id;
	buf->expire_time = lockout.GetExpireTime();
	buf->duration = lockout.GetDuration();
	strn0cpy(buf->uuid, lockout.UUID().c_str(), sizeof(buf->uuid));
	strn0cpy(buf->expedition, lockout.DzName().c_str(), sizeof(buf->expedition));
	strn0cpy(buf->event, lockout.Event().c_str(), sizeof(buf->event));
	worldserver.SendPacket(&pack);
}

void DynamicZone::AddCharacterLockout(
	uint32_t char_id, const std::string& expedition, const std::string& event, uint32_t seconds, const std::string& uuid)
{
	if (char_id)
	{
		auto lockout = DzLockout::Create(expedition, event, seconds, uuid);
		CharacterExpeditionLockoutsRepository::InsertLockouts(database, char_id, { lockout });
		SendWorldCharacterLockout(char_id, lockout, false);
	}
}

void DynamicZone::AddCharacterLockout(
	const std::string& char_name, const std::string& expedition, const std::string& event, uint32_t seconds, const std::string& uuid)
{
	if (!char_name.empty())
	{
		uint32_t char_id = database.GetCharacterID(char_name);
		AddCharacterLockout(char_id, expedition, event, seconds, uuid);
	}
}

bool DynamicZone::HasCharacterLockout(uint32_t char_id, const std::string& expedition, const std::string& event)
{
	auto lockouts = GetCharacterLockouts(char_id);
	return std::any_of(lockouts.begin(), lockouts.end(),
		[&](const auto& l) { return !l.IsExpired() && l.IsSame(expedition, event); });
}

bool DynamicZone::HasCharacterLockout(const std::string& char_name, const std::string& expedition, const std::string& event)
{
	if (!char_name.empty())
	{
		return HasCharacterLockout(database.GetCharacterID(char_name), expedition, event);
	}
	return false;
}

void DynamicZone::RemoveCharacterLockouts(uint32_t char_id, const std::string& expedition, const std::string& event)
{
	if (char_id)
	{
		std::string where = fmt::format("character_id = {}", char_id);
		if (!event.empty())
		{
			where += fmt::format(" AND expedition_name = '{}' AND event_name = '{}'", Strings::Escape(expedition), Strings::Escape(event));
		}
		else if (!expedition.empty())
		{
			where += fmt::format(" AND expedition_name = '{}'", Strings::Escape(expedition));
		}
		CharacterExpeditionLockoutsRepository::DeleteWhere(database, where);

		DzLockout lockout{{}, expedition, event, 0, 0};
		SendWorldCharacterLockout(char_id, lockout, true);
	}
}

void DynamicZone::RemoveCharacterLockouts(const std::string& char_name, const std::string& expedition, const std::string& event)
{
	if (!char_name.empty())
	{
		uint32_t char_id = database.GetCharacterID(char_name);
		RemoveCharacterLockouts(char_id, expedition, event);
	}
}

std::vector<DzLockout> DynamicZone::GetCharacterLockouts(uint32_t char_id)
{
	std::vector<DzLockout> lockouts;
	if (char_id == 0)
	{
		return lockouts;
	}

	auto client = entity_list.GetClientByCharID(char_id);
	if (client)
	{
		lockouts = client->GetDzLockouts();
	}
	else
	{
		lockouts = CharacterExpeditionLockoutsRepository::GetLockouts(database, char_id);
	}

	return lockouts;
}

void DynamicZone::AddClientsLockout(const DzLockout& lockout)
{
	std::vector<uint32_t> char_ids;
	for (const auto& it : entity_list.GetClientList())
	{
		char_ids.push_back(it.second->CharacterID());
		it.second->AddDzLockout(lockout);
	}

	if (!char_ids.empty())
	{
		CharacterExpeditionLockoutsRepository::InsertLockout(database, char_ids, lockout);
	}
}

void DynamicZone::HandleLockoutUpdate(const DzLockout& lockout, bool remove, bool members_only)
{
	DynamicZoneBase::HandleLockoutUpdate(lockout, remove, members_only);

	std::vector<uint32_t> char_ids;
	for (const auto& it : entity_list.GetClientList())
	{
		Client* client = it.second;
		if (std::ranges::any_of(m_members, [&](const auto& m) { return m.id == client->CharacterID(); }))
		{
			if (!remove)
			{
				client->AddDzLockout(lockout);
			}
			else
			{
				client->RemoveDzLockout(GetName(), lockout.Event());
			}
		}
		else if (!remove && IsCurrentZoneDz()) // non-member client inside the dz
		{
			// all clients inside the dz instance receive added lockouts to avoid exploits
			// where members quit the expedition but haven't been kicked from zone yet
			char_ids.push_back(client->CharacterID());
			client->AddDzLockout(lockout);
		}
	}

	if (!char_ids.empty())
	{
		CharacterExpeditionLockoutsRepository::InsertLockout(database, char_ids, lockout);
	}
}

void DynamicZone::HandleLockoutDuration(const DzLockout& lockout, int seconds, bool members_only, bool insert_db)
{
	DynamicZoneBase::HandleLockoutDuration(lockout, seconds, members_only, insert_db);

	std::vector<uint32_t> char_ids;
	for (const auto& it : entity_list.GetClientList())
	{
		Client* client = it.second;
		if (std::ranges::any_of(m_members, [&](const auto& m) { return m.id == client->CharacterID(); }))
		{
			client->AddDzLockoutDuration(lockout, seconds, m_uuid);
		}
		else if (IsCurrentZoneDz()) // non-member client inside the dz
		{
			char_ids.push_back(client->CharacterID());
			client->AddDzLockoutDuration(lockout, seconds, m_uuid);
		}
	}

	if (!char_ids.empty()) // always update db for non-members in dz (call may be from another zone)
	{
		int secs = static_cast<int>(seconds * RuleR(Expedition, LockoutDurationMultiplier));
		CharacterExpeditionLockoutsRepository::AddLockoutDuration(database, char_ids, lockout, secs);
	}
}

void DynamicZone::SetLootEvent(uint32_t id, const std::string& event, DzLootEvent::Type type)
{
	if (id != 0 && IsCurrentZoneDz())
	{
		LogExpeditions("Setting loot event [{}] for id [{}] type [{}]", event, id, static_cast<int>(type));
		auto it = std::ranges::find_if(m_loot_events, [&](const auto& le) { return le.id == id && le.type == type; });
		if (it != m_loot_events.end())
		{
			it->event = event;
		}
		else
		{
			m_loot_events.push_back({id, event, type});
		}
	}
}

std::string DynamicZone::GetLootEvent(uint32_t id, DzLootEvent::Type type) const
{
	std::string event_name;
	if (id != 0 && IsCurrentZoneDz())
	{
		auto it = std::ranges::find_if(m_loot_events, [&](const auto& le) { return le.id == id && le.type == type; });
		if (it != m_loot_events.end())
		{
			event_name = it->event;
		}
	}
	return event_name;
}

std::unique_ptr<EQApplicationPacket> DynamicZone::CreateInvitePacket(const std::string& inviter, const std::string& swap_name)
{
	uint32_t outsize = sizeof(ExpeditionInvite_Struct);
	auto outapp = std::make_unique<EQApplicationPacket>(OP_DzExpeditionInvite, outsize);
	auto outbuf = reinterpret_cast<ExpeditionInvite_Struct*>(outapp->pBuffer);
	strn0cpy(outbuf->inviter_name, inviter.c_str(), sizeof(outbuf->inviter_name));
	strn0cpy(outbuf->expedition_name, GetName().c_str(), sizeof(outbuf->expedition_name));
	strn0cpy(outbuf->swap_name, swap_name.c_str(), sizeof(outbuf->swap_name));
	outbuf->swapping = !swap_name.empty();
	outbuf->dz_zone_id = GetZoneID();
	outbuf->dz_instance_id = GetInstanceID();
	return outapp;
}

void DynamicZone::SendWorldPendingInvite(const ExpeditionInvite& invite, const std::string& add_name)
{
	LogExpeditions("[{}] saving invite from [{}] to [{}]", add_name, invite.inviter_name, invite.dz_id);
	SendWorldPlayerInvite(invite.inviter_name, invite.swap_name, add_name, true);
}

void DynamicZone::SendWorldPlayerInvite(const std::string& inviter, const std::string& swap_name, const std::string& add_name, bool pending)
{
	auto opcode = pending ? ServerOP_DzSaveInvite : ServerOP_DzAddPlayer;
	ServerPacket pack(opcode, static_cast<uint32_t>(sizeof(ServerDzCommand_Struct)));
	auto buf = reinterpret_cast<ServerDzCommand_Struct*>(pack.pBuffer);
	buf->dz_id = GetID();
	buf->is_char_online = false;
	strn0cpy(buf->requester_name, inviter.c_str(), sizeof(buf->requester_name));
	strn0cpy(buf->target_name, add_name.c_str(), sizeof(buf->target_name));
	strn0cpy(buf->remove_name, swap_name.c_str(), sizeof(buf->remove_name));
	worldserver.SendPacket(&pack);
}

void DynamicZone::SendWorldMakeLeaderRequest(uint32_t char_id, const std::string& leader_name)
{
	ServerPacket pack(ServerOP_DzMakeLeader, static_cast<uint32_t>(sizeof(ServerDzCommandMakeLeader_Struct)));
	auto buf = reinterpret_cast<ServerDzCommandMakeLeader_Struct*>(pack.pBuffer);
	buf->dz_id = GetID();
	buf->requester_id = char_id;
	strn0cpy(buf->new_leader_name, leader_name.c_str(), sizeof(buf->new_leader_name));
	worldserver.SendPacket(&pack);
}
