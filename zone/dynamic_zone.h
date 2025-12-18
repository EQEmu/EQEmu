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

#include "common/dynamic_zone_base.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

class Client;
class Database;
class EQApplicationPacket;
class ServerPacket;
struct ExpeditionInvite;

struct DzLootEvent
{
	enum class Type { NpcType = 0, Entity };
	uint32_t id = 0;
	std::string event;
	Type type = Type::NpcType;
};

class DynamicZone : public DynamicZoneBase
{
public:
	using DynamicZoneBase::DynamicZoneBase; // inherit base constructors

	DynamicZone() = default;
	DynamicZone(uint32_t zone_id, uint32_t version, uint32_t duration, DynamicZoneType type);

	static constexpr int32_t EventTimerID = 1;
	static constexpr int32_t ReplayTimerID = -1;

	static void CacheAllFromDatabase();
	static void CacheNewDynamicZone(ServerPacket* pack);
	static DynamicZone* TryCreate(Client& client, DynamicZone& dzinfo, bool silent = false);
	static DynamicZone* FindDynamicZoneByID(uint32_t dz_id, DynamicZoneType type = DynamicZoneType::None);
	static DynamicZone* FindExpeditionByCharacter(uint32_t char_id);
 	static DynamicZone* FindExpeditionByZone(uint32_t zone_id, uint32_t instance_id);
	static void HandleWorldMessage(ServerPacket* pack);

	static void AddClientsLockout(const DzLockout& lockout);
	static void AddCharacterLockout(uint32_t char_id, const std::string& expedition, const std::string& event, uint32_t seconds, const std::string& uuid = {});
	static void AddCharacterLockout(const std::string& char_name, const std::string& expedition, const std::string& event, uint32_t seconds, const std::string& uuid = {});
	static bool HasCharacterLockout(uint32_t char_id, const std::string& expedition, const std::string& event);
	static bool HasCharacterLockout(const std::string& char_name, const std::string& expedition, const std::string& event);
	static void RemoveCharacterLockouts(uint32_t char_id, const std::string& expedition = {}, const std::string& event = {});
	static void RemoveCharacterLockouts(const std::string& char_name, const std::string& expedition = {}, const std::string& event = {});
	static std::vector<DzLockout> GetCharacterLockouts(uint32_t char_id);

	void DzAddPlayer(Client* client, const std::string& add_name, const std::string& swap_name = {});
	void DzAddPlayerContinue(std::string inviter, std::string add_name, std::string swap_name = {});
	void DzInviteResponse(Client* client, bool accepted, const std::string& swap_name);
	void DzMakeLeader(Client* client, std::string leader_name);
	void DzPlayerList(Client* client);
	void DzRemovePlayer(Client* client, std::string name);
	void DzSwapPlayer(Client* client, std::string rem_name, std::string add_name);
	void DzQuit(Client* client);
	void DzKickPlayers(Client* client);
	void SendWorldMakeLeaderRequest(uint32_t char_id, const std::string& leader_name);
	void SendWorldPendingInvite(const ExpeditionInvite& invite, const std::string& add_name);

	void SetSecondsRemaining(uint32_t seconds_remaining) override;

	bool CanClientLootCorpse(Client* client, uint32_t npc_type_id, uint32_t entity_id);
	bool IsCurrentZoneDz() const;
	void MovePCInto(Client* client, bool world_verify = false) const;
	void SendClientWindowUpdate(Client* client);
	void SendLeaderNameToZoneMembers();
	void SendMemberListToZoneMembers();
	void SendMemberNameToZoneMembers(const std::string& char_name, bool remove);
	void SendMemberStatusToZoneMembers(const DynamicZoneMember& member);
	void SetLocked(bool lock, bool update_db = false, DzLockMsg lock_msg = DzLockMsg::None, uint32_t color = Chat::Yellow);

	std::string GetLootEvent(uint32_t id, DzLootEvent::Type type) const;
	void SetLootEvent(uint32_t id, const std::string& event, DzLootEvent::Type type);

private:
	static void StartAllClientRemovalTimers();
	static void RequestMemberStatuses();

	uint16_t GetCurrentInstanceID() const override;
	uint16_t GetCurrentZoneID() const override;
	Database& GetDatabase() override;
	void HandleLockoutDuration(const DzLockout& lockout, int seconds, bool members_only, bool insert_db) override;
	void HandleLockoutUpdate(const DzLockout& lockout, bool remove, bool members_only) override;
	void ProcessCompassChange(const DynamicZoneLocation& location) override;
	void ProcessMemberAddRemove(const DynamicZoneMember& member, bool removed) override;
	bool ProcessMemberStatusChange(uint32_t character_id, DynamicZoneMemberStatus status) override;
	void ProcessRemoveAllMembers() override;
	void ProcessSetSwitchID(int dz_switch_id) override;
	bool SendServerPacket(ServerPacket* packet) override;

	bool ConfirmLeaderCommand(Client* client);
	bool ProcessAddConflicts(Client* leader, Client* client, bool swapping);
	void ProcessLeaderChanged(uint32_t new_leader_id);
	void SaveLockouts(const std::vector<DzLockout>& lockouts);
	void SendClientInvite(Client* client, const std::string& inviter, const std::string& swap_name);
	void SendCompassUpdateToZoneMembers();
	void SendLeaderMessage(Client* leader, uint16_t type, const std::string& msg);
	void SendLeaderMessage(Client* leader, uint16_t type, uint32_t str_id, std::initializer_list<std::string> args = {});
	void SendMembersExpireWarning(uint32_t minutes);
	void SendUpdatesToZoneMembers(bool removing_all = false, bool silent = true);
	void SendWorldPlayerInvite(const std::string& inviter, const std::string& swap_name, const std::string& add_name, bool pending = false);
	void SetUpdatedDuration(uint32_t seconds);
	void TryAddClient(Client* add_client, const std::string& inviter, const std::string& swap_name, Client* leader = nullptr);
	void UpdateMembers();

	std::unique_ptr<EQApplicationPacket> CreateExpireWarningPacket(uint32_t minutes_remaining);
	std::unique_ptr<EQApplicationPacket> CreateInfoPacket(bool clear = false);
	std::unique_ptr<EQApplicationPacket> CreateInvitePacket(const std::string& inviter, const std::string& swap_name);
	std::unique_ptr<EQApplicationPacket> CreateLeaderNamePacket();
	std::unique_ptr<EQApplicationPacket> CreateMemberListPacket(bool clear = false);
	std::unique_ptr<EQApplicationPacket> CreateMemberNamePacket(const std::string& name, bool remove);
	std::unique_ptr<EQApplicationPacket> CreateMemberStatusPacket(const std::string& name, DynamicZoneMemberStatus status);

	std::vector<DzLootEvent> m_loot_events; // only valid inside dz zone
};
