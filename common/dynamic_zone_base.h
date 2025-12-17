#pragma once

#include "common/dynamic_zone_lockout.h"
#include "common/eq_constants.h"
#include "common/repositories/dynamic_zone_members_repository.h"
#include "common/repositories/dynamic_zone_templates_repository.h"
#include "common/repositories/dynamic_zones_repository.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <vector>

class Database;
class ServerPacket;

// message string 8312 added in September 08 2020 Test patch (used by both dz and shared tasks)
inline constexpr char DzNotAllAdded[] = "Not all players in your {0} were added to the {1}. The {1} can take a maximum of {2} players, and your {0} has {3}.";

enum class DzLockMsg : uint8_t
{
	None = 0, Close, Begin
};

enum class DynamicZoneType
{
	None = 0,
	Expedition,
	Tutorial,
	Task,
	Mission, // Shared Task
	Quest
};

enum class DynamicZoneMemberStatus
{
	Unknown = 0,
	Online,
	Offline,
	InDynamicZone,
	LinkDead
};

struct DynamicZoneMember
{
	uint32_t id = 0;
	std::string name;
	DynamicZoneMemberStatus status = DynamicZoneMemberStatus::Unknown;

	DynamicZoneMember() = default;
	DynamicZoneMember(uint32_t id, std::string name_)
		: id(id), name{std::move(name_)} {}
	DynamicZoneMember(uint32_t id, std::string name_, DynamicZoneMemberStatus status_)
		: id(id), name{std::move(name_)}, status(status_) {}

	bool IsOnline() const { return status == DynamicZoneMemberStatus::Online ||
	                               status == DynamicZoneMemberStatus::InDynamicZone; }
	bool IsValid() const { return id != 0 && !name.empty(); }

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(id, name, status);
	}
};

struct DynamicZoneLocation
{
	uint32_t zone_id = 0;
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float heading = 0.0f;

	DynamicZoneLocation() = default;
	DynamicZoneLocation(uint32_t zone_id_, float x_, float y_, float z_, float heading_)
		: zone_id(zone_id_), x(x_), y(y_), z(z_), heading(heading_) {}

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(zone_id, x, y, z, heading);
	}
};

class DynamicZoneBase
{
public:
	virtual ~DynamicZoneBase() = default;
	DynamicZoneBase(const DynamicZoneBase&) = default;
	DynamicZoneBase(DynamicZoneBase&&) = default;
	DynamicZoneBase& operator=(const DynamicZoneBase&) = default;
	DynamicZoneBase& operator=(DynamicZoneBase&&) = default;
	DynamicZoneBase() = default;
	DynamicZoneBase(uint32_t dz_id) : m_id(dz_id) {}
	DynamicZoneBase(DynamicZoneType type) : m_type(type) {}
	DynamicZoneBase(DynamicZonesRepository::DynamicZoneInstance&& entry);

	static std::string GetDynamicZoneTypeName(DynamicZoneType dz_type);

	virtual void SetSecondsRemaining(uint32_t seconds_remaining) = 0;

	int GetDuration() const { return static_cast<int>(m_duration.count()); }
	uint64_t GetExpireTime() const { return std::chrono::system_clock::to_time_t(m_expire_time); }
	uint32_t GetID() const { return m_id; }
	uint16_t GetInstanceID() const { return static_cast<uint16_t>(m_instance_id); }
	uint32_t GetLeaderID() const { return m_leader.id; }
	uint32_t GetMaxPlayers() const { return m_max_players; }
	uint32_t GetMemberCount() const { return static_cast<uint32_t>(m_members.size()); }
	uint32_t GetMinPlayers() const { return m_min_players; }
	uint32_t GetSecondsRemaining() const;
	uint16_t GetZoneID() const { return static_cast<uint16_t>(m_zone_id); }
	uint32_t GetZoneIndex() const { return (m_instance_id << 16) | (m_zone_id & 0xffff); }
	uint32_t GetZoneVersion() const { return m_zone_version; }
	int GetSwitchID() const { return m_dz_switch_id; }
	DynamicZoneType GetType() const { return m_type; }
	const std::string& GetLeaderName() const { return m_leader.name; }
	const std::string& GetName() const { return m_name; }
	const std::string& GetUUID() const { return m_uuid; }
	const DynamicZoneMember& GetLeader() const { return m_leader; }
	const std::vector<DzLockout>& GetLockouts() const { return m_lockouts; }
	const std::vector<DynamicZoneMember>& GetMembers() const { return m_members; }
	const DynamicZoneLocation& GetCompassLocation() const { return m_compass; }
	const DynamicZoneLocation& GetSafeReturnLocation() const { return m_safereturn; }
	const DynamicZoneLocation& GetZoneInLocation() const { return m_zonein; }
	std::chrono::system_clock::duration GetDurationRemaining() const { return m_expire_time - std::chrono::system_clock::now(); }

	bool AddMember(const DynamicZoneMember& add_member);
	void AddMemberFromRepositoryResult(DynamicZoneMembersRepository::MemberWithName&& entry);
	uint32_t GetDatabaseMemberCount();
	DynamicZoneMember GetMemberData(uint32_t character_id);
	DynamicZoneMember GetMemberData(const std::string& character_name);
	std::vector<uint32_t> GetMemberIds();
	std::ostringstream GetSerialized();
	bool HasDatabaseMember(uint32_t character_id);
	bool HasMember(uint32_t character_id) const;
	bool HasMember(const std::string& character_name) const;
	bool HasMembers() const { return !m_members.empty(); }
	bool HasZoneInLocation() const { return m_has_zonein; }
	bool IsExpedition() const { return m_type == DynamicZoneType::Expedition; }
	bool IsExpired() const { return m_expire_time < std::chrono::system_clock::now(); }
	bool IsInstanceID(uint32_t instance_id) const { return (m_instance_id != 0 && m_instance_id == instance_id); }
	bool IsLocked() const { return m_is_locked; }
	bool IsValid() const { return m_instance_id != 0; }
	bool IsSameDz(uint32_t zone_id, uint32_t instance_id) const { return zone_id == m_zone_id && instance_id == m_instance_id; }
	void LoadTemplate(const DynamicZoneTemplatesRepository::DynamicZoneTemplates& dz_template);
	void RemoveAllMembers();
	bool RemoveMember(uint32_t character_id);
	bool RemoveMember(const std::string& character_name);
	bool RemoveMember(const DynamicZoneMember& remove_member);
	void SetCompass(const DynamicZoneLocation& location, bool update_db = false);
	void SetCompass(uint32_t zone_id, float x, float y, float z, bool update_db = false);
	void SetDuration(uint32_t seconds) { m_duration = std::chrono::seconds(seconds); }
	void SetLeader(const DynamicZoneMember& leader, bool update_db = false);
	void SetLocked(bool lock, bool update_db = false, DzLockMsg lock_msg = DzLockMsg::None, uint32_t color = Chat::Yellow);
	void SetMaxPlayers(uint32_t max_players) { m_max_players = max_players; }
	void SetMemberStatus(uint32_t character_id, DynamicZoneMemberStatus status);
	void SetMinPlayers(uint32_t min_players) { m_min_players = min_players; }
	void SetName(const std::string& name) { m_name = name; }
	void SetReplayOnJoin(bool enabled, bool update_db = false);
	void SetSafeReturn(const DynamicZoneLocation& location, bool update_db = false);
	void SetSafeReturn(uint32_t zone_id, float x, float y, float z, float heading, bool update_db = false);
	void SetSwitchID(int dz_switch_id, bool update_db = false);
	void SetType(DynamicZoneType type) { m_type = type; }
	void SetUUID(std::string uuid) { m_uuid = std::move(uuid); }
	void SetZoneInLocation(const DynamicZoneLocation& location, bool update_db = false);
	void SetZoneInLocation(float x, float y, float z, float heading, bool update_db = false);
	bool SwapMember(const DynamicZoneMember& add_member, const std::string& remove_name);

	void AddLockout(const std::string& event, uint32_t seconds);
	void AddLockoutDuration(const std::string& event, int seconds, bool members_only = true);
	bool HasLockout(const std::string& event);
	bool HasReplayLockout();
	void RemoveLockout(const std::string& event);
	void SyncCharacterLockouts(uint32_t char_id, std::vector<DzLockout>& lockouts);
	void UpdateLockoutDuration(const std::string& event, uint32_t seconds, bool members_only = true);

protected:
	virtual uint16_t GetCurrentInstanceID() const { return 0; }
	virtual uint16_t GetCurrentZoneID() const { return 0; }
	virtual Database& GetDatabase() = 0;
	virtual void HandleLockoutDuration(const DzLockout& lockout, int seconds, bool members_only, bool insert_db);
	virtual void HandleLockoutUpdate(const DzLockout& lockout, bool remove, bool members_only);
	virtual void ProcessCompassChange(const DynamicZoneLocation& location) { m_compass = location; }
	virtual void ProcessMemberAddRemove(const DynamicZoneMember& member, bool removed);
	virtual bool ProcessMemberStatusChange(uint32_t character_id, DynamicZoneMemberStatus status);
	virtual void ProcessRemoveAllMembers() { m_members.clear(); }
	virtual void ProcessSetSwitchID(int dz_switch_id) { m_dz_switch_id = dz_switch_id; }
	virtual bool SendServerPacket(ServerPacket* packet) = 0;

	void AddLockout(const DzLockout& lockout, bool members_only = false);
	void AddInternalMember(const DynamicZoneMember& member);
	uint32_t Create();
	uint32_t CreateInstance();
	void LoadRepositoryResult(DynamicZonesRepository::DynamicZoneInstance&& dz_entry);
	void RemoveInternalMember(uint32_t character_id);
	void SaveMembers(const std::vector<DynamicZoneMember>& members);
	uint32_t SaveToDatabase();
	bool SetInternalMemberStatus(uint32_t character_id, DynamicZoneMemberStatus status);

	std::unique_ptr<ServerPacket> CreateServerPacket(uint16_t zone_id, uint16_t instance_id);
	std::unique_ptr<ServerPacket> CreateServerDzLocationPacket(uint16_t server_opcode, const DynamicZoneLocation& location);
	std::unique_ptr<ServerPacket> CreateServerDzSwitchIDPacket();
	std::unique_ptr<ServerPacket> CreateServerMemberAddRemovePacket(const DynamicZoneMember& member, bool removed);
	std::unique_ptr<ServerPacket> CreateServerMemberStatusPacket(uint32_t character_id, DynamicZoneMemberStatus status);
	std::unique_ptr<ServerPacket> CreateServerMemberSwapPacket(const DynamicZoneMember& remove_member, const DynamicZoneMember& add_member);
	std::unique_ptr<ServerPacket> CreateServerRemoveAllMembersPacket();
	std::unique_ptr<ServerPacket> CreateLockoutPacket(const DzLockout& lockout, bool remove, bool members_only = false) const;
	std::unique_ptr<ServerPacket> CreateLockoutDurationPacket(const DzLockout& lockout, int seconds, bool members_only = false) const;

	uint32_t m_id = 0;
	uint32_t m_zone_id = 0;
	uint32_t m_instance_id = 0;
	uint32_t m_zone_version = 0;
	uint32_t m_min_players = 0;
	uint32_t m_max_players = 0;
	int m_dz_switch_id = 0;
	bool m_never_expires = false;
	bool m_has_zonein = false;
	bool m_has_member_statuses = false;
	bool m_is_locked = false;
	bool m_add_replay = true;
	std::string m_name;
	std::string m_uuid;
	DynamicZoneMember m_leader;
	DynamicZoneType m_type{ DynamicZoneType::None };
	DynamicZoneLocation m_compass;
	DynamicZoneLocation m_safereturn;
	DynamicZoneLocation m_zonein;
	std::chrono::seconds m_duration = {};
	std::chrono::time_point<std::chrono::system_clock> m_start_time;
	std::chrono::time_point<std::chrono::system_clock> m_expire_time;
	std::vector<DynamicZoneMember> m_members;
	std::vector<DzLockout> m_lockouts;

public:
	void Unserialize(std::span<char> buf);

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(
			m_id,
			m_zone_id,
			m_instance_id,
			m_zone_version,
			m_min_players,
			m_max_players,
			m_dz_switch_id,
			m_never_expires,
			m_has_zonein,
			m_has_member_statuses,
			m_is_locked,
			m_add_replay,
			m_name,
			m_uuid,
			m_leader,
			m_type,
			m_compass,
			m_safereturn,
			m_zonein,
			m_duration,
			m_start_time,
			m_expire_time,
			m_members,
			m_lockouts
		);
	}
};
