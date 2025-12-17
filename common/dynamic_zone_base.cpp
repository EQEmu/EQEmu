#include "dynamic_zone_base.h"

#include "common/database.h"
#include "common/eqemu_logsys.h"
#include "common/repositories/character_expedition_lockouts_repository.h"
#include "common/repositories/dynamic_zone_lockouts_repository.h"
#include "common/repositories/instance_list_player_repository.h"
#include "common/repositories/instance_list_repository.h"
#include "common/rulesys.h"
#include "common/servertalk.h"
#include "common/util/uuid.h"

DynamicZoneBase::DynamicZoneBase(DynamicZonesRepository::DynamicZoneInstance&& entry)
{
	LoadRepositoryResult(std::move(entry));
}

uint32_t DynamicZoneBase::Create()
{
	if (GetInstanceID() == 0)
	{
		CreateInstance();
	}

	m_uuid = EQ::Util::UUID::Generate().ToString();
	m_id = SaveToDatabase();

	return m_id;
}

uint32_t DynamicZoneBase::CreateInstance()
{
	if (m_instance_id)
	{
		LogDynamicZones("CreateInstance failed, instance id [{}] already created", m_instance_id);
		return 0;
	}

	if (!m_zone_id)
	{
		LogDynamicZones("CreateInstance failed, invalid zone id [{}]", m_zone_id);
		return 0;
	}

	uint16_t unused_instance_id = 0;
	if (!GetDatabase().GetUnusedInstanceID(unused_instance_id)) // todo: doesn't this race with insert?
	{
		LogDynamicZones("Failed to find unused instance id");
		return 0;
	}

	m_start_time = std::chrono::system_clock::now();
	m_expire_time = m_start_time + m_duration;

	auto insert_instance = InstanceListRepository::NewEntity();
	insert_instance.id = unused_instance_id;
	insert_instance.zone = m_zone_id;
	insert_instance.version = m_zone_version;
	insert_instance.start_time = static_cast<int>(std::chrono::system_clock::to_time_t(m_start_time));
	insert_instance.duration = static_cast<int>(m_duration.count());
	insert_instance.never_expires = m_never_expires;
	insert_instance.expire_at = insert_instance.start_time + insert_instance.duration;

	auto instance = InstanceListRepository::ReplaceOne(GetDatabase(), insert_instance);
	if (!instance)
	{
		LogDynamicZones("Failed to create instance [{}] for zone [{}]", unused_instance_id, m_zone_id);
		return 0;
	}

	m_instance_id = unused_instance_id;

	return m_instance_id;
}

void DynamicZoneBase::LoadRepositoryResult(DynamicZonesRepository::DynamicZoneInstance&& dz_entry)
{
	m_id                 = dz_entry.id;
	m_uuid               = std::move(dz_entry.uuid);
	m_name               = std::move(dz_entry.name);
	m_leader.id          = dz_entry.leader_id;
	m_min_players        = dz_entry.min_players;
	m_max_players        = dz_entry.max_players;
	m_instance_id        = dz_entry.instance_id;
	m_type               = static_cast<DynamicZoneType>(dz_entry.type);
	m_dz_switch_id       = dz_entry.dz_switch_id;
	m_compass.zone_id    = dz_entry.compass_zone_id;
	m_compass.x          = dz_entry.compass_x;
	m_compass.y          = dz_entry.compass_y;
	m_compass.z          = dz_entry.compass_z;
	m_safereturn.zone_id = dz_entry.safe_return_zone_id;
	m_safereturn.x       = dz_entry.safe_return_x;
	m_safereturn.y       = dz_entry.safe_return_y;
	m_safereturn.z       = dz_entry.safe_return_z;
	m_safereturn.heading = dz_entry.safe_return_heading;
	m_zonein.x           = dz_entry.zone_in_x;
	m_zonein.y           = dz_entry.zone_in_y;
	m_zonein.z           = dz_entry.zone_in_z;
	m_zonein.heading     = dz_entry.zone_in_heading;
	m_has_zonein         = dz_entry.has_zone_in != 0;
	m_is_locked          = dz_entry.is_locked;
	m_add_replay         = dz_entry.add_replay;
	// instance_list portion
	m_zone_id            = dz_entry.zone;
	m_zone_version       = dz_entry.version;
	m_start_time         = std::chrono::system_clock::from_time_t(dz_entry.start_time);
	m_duration           = std::chrono::seconds(dz_entry.duration);
	m_never_expires      = dz_entry.never_expires != 0;
	m_expire_time        = m_start_time + m_duration;
}

void DynamicZoneBase::AddMemberFromRepositoryResult(
	DynamicZoneMembersRepository::MemberWithName&& entry)
{
	auto status = DynamicZoneMemberStatus::Unknown;

	if (m_leader.id == entry.character_id)
	{
		m_leader.name = entry.character_name;
	}

	AddInternalMember({ entry.character_id, std::move(entry.character_name), status });
}

uint32_t DynamicZoneBase::SaveToDatabase()
{
	LogDynamicZonesDetail("Saving dz instance [{}] to database", m_instance_id);
	if (m_instance_id == 0)
	{
		return 0;
	}

	auto dz = DynamicZonesRepository::NewEntity();
	dz.uuid                = m_uuid;
	dz.name                = m_name;
	dz.leader_id           = m_leader.id;
	dz.min_players         = m_min_players;
	dz.max_players         = m_max_players;
	dz.instance_id         = static_cast<int32_t>(m_instance_id),
	dz.type                = static_cast<uint8_t>(m_type);
	dz.dz_switch_id        = m_dz_switch_id;
	dz.compass_zone_id     = m_compass.zone_id;
	dz.compass_x           = m_compass.x;
	dz.compass_y           = m_compass.y;
	dz.compass_z           = m_compass.z;
	dz.safe_return_zone_id = m_safereturn.zone_id;
	dz.safe_return_x       = m_safereturn.x;
	dz.safe_return_y       = m_safereturn.y;
	dz.safe_return_z       = m_safereturn.z;
	dz.safe_return_heading = m_safereturn.heading;
	dz.zone_in_x           = m_zonein.x;
	dz.zone_in_y           = m_zonein.y;
	dz.zone_in_z           = m_zonein.z;
	dz.zone_in_heading     = m_zonein.heading;
	dz.has_zone_in         = static_cast<uint8_t>(m_has_zonein);
	dz.is_locked           = static_cast<int8_t>(m_is_locked);
	dz.add_replay          = static_cast<int8_t>(m_add_replay);

	dz = DynamicZonesRepository::InsertOne(GetDatabase(), std::move(dz));

	return dz.id;
}

bool DynamicZoneBase::AddMember(const DynamicZoneMember& add_member)
{
	if (HasMember(add_member.id))
	{
		return false;
	}

	DynamicZoneMembersRepository::AddMember(GetDatabase(), m_id, add_member.id);
	GetDatabase().AddClientToInstance(m_instance_id, add_member.id);

	ProcessMemberAddRemove(add_member, false);
	SendServerPacket(CreateServerMemberAddRemovePacket(add_member, false).get());

	return true;
}

bool DynamicZoneBase::RemoveMember(uint32_t character_id)
{
	auto remove_member = GetMemberData(character_id);
	return RemoveMember(remove_member);
}

bool DynamicZoneBase::RemoveMember(const std::string& character_name)
{
	auto remove_member = GetMemberData(character_name);
	return RemoveMember(remove_member);
}

bool DynamicZoneBase::RemoveMember(const DynamicZoneMember& remove_member)
{
	if (remove_member.id == 0)
	{
		return false;
	}

	DynamicZoneMembersRepository::RemoveMember(GetDatabase(), m_id, remove_member.id);
	GetDatabase().RemoveClientFromInstance(m_instance_id, remove_member.id);

	ProcessMemberAddRemove(remove_member, true);
	SendServerPacket(CreateServerMemberAddRemovePacket(remove_member, true).get());

	return true;
}

bool DynamicZoneBase::SwapMember(const DynamicZoneMember& add_member, const std::string& remove_name)
{
	auto remove_member = GetMemberData(remove_name);
	if (!add_member.IsValid() || !remove_member.IsValid())
	{
		return false;
	}

	// make remove and add atomic to avoid racing with separate world messages
	DynamicZoneMembersRepository::RemoveMember(GetDatabase(), m_id, remove_member.id);
	GetDatabase().RemoveClientFromInstance(m_instance_id, remove_member.id);

	DynamicZoneMembersRepository::AddMember(GetDatabase(), m_id, add_member.id);
	GetDatabase().AddClientToInstance(m_instance_id, add_member.id);

	ProcessMemberAddRemove(remove_member, true);
	ProcessMemberAddRemove(add_member, false);
	SendServerPacket(CreateServerMemberSwapPacket(remove_member, add_member).get());

	return true;
}

void DynamicZoneBase::RemoveAllMembers()
{
	DynamicZoneMembersRepository::RemoveAllMembers(GetDatabase(), m_id);
	GetDatabase().RemoveClientsFromInstance(GetInstanceID());

	ProcessRemoveAllMembers();
	SendServerPacket(CreateServerRemoveAllMembersPacket().get());
}

void DynamicZoneBase::SaveMembers(const std::vector<DynamicZoneMember>& members)
{
	if (members.empty())
	{
		return;
	}

	LogDynamicZonesDetail("Saving [{}] member(s) for dz [{}]", members.size(), m_id);

	m_members = members;
	if (m_members.size() > m_max_players)
	{
		m_members.resize(m_max_players);
	}

	// the lower level instance_list_players needs to be kept updated as well
	std::vector<DynamicZoneMembersRepository::DynamicZoneMembers> insert_members;
	std::vector<InstanceListPlayerRepository::InstanceListPlayer> insert_players;
	for (const auto& member : m_members)
	{
		DynamicZoneMembersRepository::DynamicZoneMembers member_entry{};
		member_entry.dynamic_zone_id = m_id;
		member_entry.character_id = member.id;
		insert_members.push_back(member_entry);

		InstanceListPlayerRepository::InstanceListPlayer player_entry{};
		player_entry.id = m_instance_id;
		player_entry.charid = member.id;
		insert_players.push_back(player_entry);
	}

	DynamicZoneMembersRepository::InsertOrUpdateMany(GetDatabase(), insert_members);
	InstanceListPlayerRepository::InsertOrUpdateMany(GetDatabase(), insert_players);
}

void DynamicZoneBase::SetCompass(const DynamicZoneLocation& location, bool update_db)
{
	ProcessCompassChange(location);

	if (update_db)
	{
		LogDynamicZonesDetail("Saving [{}] compass zone: [{}] xyz: ([{}], [{}], [{}])",
			m_id, m_compass.zone_id, m_compass.x, m_compass.y, m_compass.z);

		DynamicZonesRepository::UpdateCompass(GetDatabase(),
			m_id, m_compass.zone_id, m_compass.x, m_compass.y, m_compass.z);

		SendServerPacket(CreateServerDzLocationPacket(ServerOP_DzSetCompass, location).get());
	}
}

void DynamicZoneBase::SetCompass(uint32_t zone_id, float x, float y, float z, bool update_db)
{
	SetCompass({ zone_id, x, y, z, 0.0f }, update_db);
}

void DynamicZoneBase::SetSafeReturn(const DynamicZoneLocation& location, bool update_db)
{
	m_safereturn = location;

	if (update_db)
	{
		LogDynamicZonesDetail("Saving [{}] safereturn zone: [{}] xyzh: ([{}], [{}], [{}], [{}])",
			m_id, m_safereturn.zone_id, m_safereturn.x, m_safereturn.y, m_safereturn.z, m_safereturn.heading);

		DynamicZonesRepository::UpdateSafeReturn(GetDatabase(), m_id, m_safereturn.zone_id,
			m_safereturn.x, m_safereturn.y, m_safereturn.z, m_safereturn.heading);

		SendServerPacket(CreateServerDzLocationPacket(ServerOP_DzSetSafeReturn, location).get());
	}
}

void DynamicZoneBase::SetSafeReturn(uint32_t zone_id, float x, float y, float z, float heading, bool update_db)
{
	SetSafeReturn({ zone_id, x, y, z, heading }, update_db);
}

void DynamicZoneBase::SetZoneInLocation(const DynamicZoneLocation& location, bool update_db)
{
	m_zonein = location;
	m_has_zonein = true;

	if (update_db)
	{
		LogDynamicZonesDetail("Saving [{}] zonein zone: [{}] xyzh: ([{}], [{}], [{}], [{}])",
			m_id, m_zone_id, m_zonein.x, m_zonein.y, m_zonein.z, m_zonein.heading);

		DynamicZonesRepository::UpdateZoneIn(GetDatabase(), m_id, m_zone_id,
			m_zonein.x, m_zonein.y, m_zonein.z, m_zonein.heading, m_has_zonein);

		SendServerPacket(CreateServerDzLocationPacket(ServerOP_DzSetZoneIn, location).get());
	}
}

void DynamicZoneBase::SetZoneInLocation(float x, float y, float z, float heading, bool update_db)
{
	SetZoneInLocation({ 0, x, y, z, heading }, update_db);
}

void DynamicZoneBase::SetSwitchID(int dz_switch_id, bool update_db)
{
	m_dz_switch_id = dz_switch_id;

	if (update_db)
	{
		DynamicZonesRepository::UpdateSwitchID(GetDatabase(), m_id, dz_switch_id);
		SendServerPacket(CreateServerDzSwitchIDPacket().get());
	}
}

void DynamicZoneBase::SetLeader(const DynamicZoneMember& new_leader, bool update_db)
{
	m_leader = new_leader;

	if (update_db)
	{
		DynamicZonesRepository::UpdateLeaderID(GetDatabase(), m_id, new_leader.id);
	}
}

void DynamicZoneBase::SetLocked(bool lock, bool update_db, DzLockMsg lock_msg, uint32_t color)
{
	m_is_locked = lock;

	if (update_db)
	{
		DynamicZonesRepository::UpdateLocked(GetDatabase(), m_id, lock);

		ServerPacket pack(ServerOP_DzLock, sizeof(ServerDzLock_Struct));
		auto buf = reinterpret_cast<ServerDzLock_Struct*>(pack.pBuffer);
		buf->dz_id = GetID();
		buf->sender_zone_id = GetCurrentZoneID();
		buf->sender_instance_id = GetCurrentInstanceID();
		buf->lock = m_is_locked;
		buf->lock_msg = static_cast<uint8_t>(lock_msg);
		buf->color = color;
		SendServerPacket(&pack);
	}
}

void DynamicZoneBase::SetReplayOnJoin(bool enabled, bool update_db)
{
	m_add_replay = enabled;

	if (update_db)
	{
		DynamicZonesRepository::UpdateReplayOnJoin(GetDatabase(), m_id, enabled);

		ServerPacket pack(ServerOP_DzReplayOnJoin, sizeof(ServerDzBool_Struct));
		auto buf = reinterpret_cast<ServerDzBool_Struct*>(pack.pBuffer);
		buf->dz_id = GetID();
		buf->sender_zone_id = GetCurrentZoneID();
		buf->sender_instance_id = GetCurrentInstanceID();
		buf->enabled = enabled;
		SendServerPacket(&pack);
	}
}

uint32_t DynamicZoneBase::GetSecondsRemaining() const
{
	auto remaining = std::chrono::duration_cast<std::chrono::seconds>(GetDurationRemaining()).count();
	return std::max(0, static_cast<int>(remaining));
}

std::unique_ptr<ServerPacket> DynamicZoneBase::CreateServerMemberAddRemovePacket(
	const DynamicZoneMember& member, bool removed)
{
	constexpr uint32_t pack_size = sizeof(ServerDzMember_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_DzAddRemoveMember, pack_size);
	auto buf = reinterpret_cast<ServerDzMember_Struct*>(pack->pBuffer);
	buf->dz_id = GetID();
	buf->dz_zone_id = GetZoneID();
	buf->dz_instance_id = GetInstanceID();
	buf->sender_zone_id = GetCurrentZoneID();
	buf->sender_instance_id = GetCurrentInstanceID();
	buf->removed = removed;
	buf->character_id = member.id;
	buf->character_status = static_cast<uint8_t>(member.status);
	strn0cpy(buf->character_name, member.name.c_str(), sizeof(buf->character_name));

	return pack;
}

std::unique_ptr<ServerPacket> DynamicZoneBase::CreateServerMemberSwapPacket(
	const DynamicZoneMember& remove_member, const DynamicZoneMember& add_member)
{
	constexpr uint32_t pack_size = sizeof(ServerDzMemberSwap_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_DzSwapMembers, pack_size);
	auto buf = reinterpret_cast<ServerDzMemberSwap_Struct*>(pack->pBuffer);
	buf->dz_id = GetID();
	buf->dz_zone_id = GetZoneID();
	buf->dz_instance_id = GetInstanceID();
	buf->sender_zone_id = GetCurrentZoneID();
	buf->sender_instance_id = GetCurrentInstanceID();
	buf->add_character_status = static_cast<uint8_t>(add_member.status);
	buf->add_character_id = add_member.id;
	buf->remove_character_id = remove_member.id;
	strn0cpy(buf->add_character_name, add_member.name.c_str(), sizeof(buf->add_character_name));
	strn0cpy(buf->remove_character_name, remove_member.name.c_str(), sizeof(buf->remove_character_name));

	return pack;
}

std::unique_ptr<ServerPacket> DynamicZoneBase::CreateServerRemoveAllMembersPacket()
{
	constexpr uint32_t pack_size = sizeof(ServerDzID_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_DzRemoveAllMembers, pack_size);
	auto buf = reinterpret_cast<ServerDzID_Struct*>(pack->pBuffer);
	buf->dz_id = GetID();
	buf->dz_zone_id = GetZoneID();
	buf->dz_instance_id = GetInstanceID();
	buf->sender_zone_id = GetCurrentZoneID();
	buf->sender_instance_id = GetCurrentInstanceID();

	return pack;
}

std::unique_ptr<ServerPacket> DynamicZoneBase::CreateServerDzLocationPacket(
	uint16_t server_opcode, const DynamicZoneLocation& location)
{
	constexpr uint32_t pack_size = sizeof(ServerDzLocation_Struct);
	auto pack = std::make_unique<ServerPacket>(server_opcode, pack_size);
	auto buf = reinterpret_cast<ServerDzLocation_Struct*>(pack->pBuffer);
	buf->dz_id = GetID();
	buf->sender_zone_id = GetCurrentZoneID();
	buf->sender_instance_id = GetCurrentInstanceID();
	buf->zone_id = location.zone_id;
	buf->x = location.x;
	buf->y = location.y;
	buf->z = location.z;
	buf->heading = location.heading;

	return pack;
}

std::unique_ptr<ServerPacket> DynamicZoneBase::CreateServerDzSwitchIDPacket()
{
	constexpr uint32_t pack_size = sizeof(ServerDzSwitchID_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_DzSetSwitchID, pack_size);
	auto buf = reinterpret_cast<ServerDzSwitchID_Struct*>(pack->pBuffer);
	buf->dz_id = GetID();
	buf->dz_switch_id = GetSwitchID();

	return pack;
}

std::unique_ptr<ServerPacket> DynamicZoneBase::CreateServerMemberStatusPacket(
	uint32_t character_id, DynamicZoneMemberStatus status)
{
	constexpr uint32_t pack_size = sizeof(ServerDzMemberStatus_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_DzUpdateMemberStatus, pack_size);
	auto buf = reinterpret_cast<ServerDzMemberStatus_Struct*>(pack->pBuffer);
	buf->dz_id = GetID();
	buf->sender_zone_id = GetCurrentZoneID();
	buf->sender_instance_id = GetCurrentInstanceID();
	buf->status = static_cast<uint8_t>(status);
	buf->character_id = character_id;

	return pack;
}

uint32_t DynamicZoneBase::GetDatabaseMemberCount()
{
	return DynamicZoneMembersRepository::GetCountWhere(GetDatabase(),
		fmt::format("dynamic_zone_id = {}", m_id));
}

bool DynamicZoneBase::HasDatabaseMember(uint32_t character_id)
{
	if (character_id == 0)
	{
		return false;
	}

	auto entries = DynamicZoneMembersRepository::GetWhere(GetDatabase(), fmt::format(
		"dynamic_zone_id = {} AND character_id = {}",
		m_id, character_id
	));

	return entries.size() != 0;
}

void DynamicZoneBase::AddInternalMember(const DynamicZoneMember& member)
{
	if (!HasMember(member.id))
	{
		m_members.emplace_back(member);
	}
}

void DynamicZoneBase::RemoveInternalMember(uint32_t character_id)
{
	m_members.erase(std::remove_if(m_members.begin(), m_members.end(),
		[&](const DynamicZoneMember& member) { return member.id == character_id; }
	), m_members.end());
}

bool DynamicZoneBase::HasMember(uint32_t character_id) const
{
	return std::any_of(m_members.begin(), m_members.end(),
		[&](const DynamicZoneMember& member) { return member.id == character_id; });
}

bool DynamicZoneBase::HasMember(const std::string& character_name) const
{
	return std::any_of(m_members.begin(), m_members.end(),
		[&](const DynamicZoneMember& member) {
			return strcasecmp(member.name.c_str(), character_name.c_str()) == 0;
		});
}

DynamicZoneMember DynamicZoneBase::GetMemberData(uint32_t character_id)
{
	auto it = std::find_if(m_members.begin(), m_members.end(),
		[&](const DynamicZoneMember& member) { return member.id == character_id; });

	DynamicZoneMember member_data;
	if (it != m_members.end())
	{
		member_data = *it;
	}
	return member_data;
}

DynamicZoneMember DynamicZoneBase::GetMemberData(const std::string& character_name)
{
	auto it = std::find_if(m_members.begin(), m_members.end(),
		[&](const DynamicZoneMember& member) {
			return strcasecmp(member.name.c_str(), character_name.c_str()) == 0;
		});

	DynamicZoneMember member_data;
	if (it != m_members.end())
	{
		member_data = *it;
	}
	return member_data;
}

bool DynamicZoneBase::SetInternalMemberStatus(uint32_t character_id, DynamicZoneMemberStatus status)
{
	if (status == DynamicZoneMemberStatus::InDynamicZone && !RuleB(DynamicZone, EnableInDynamicZoneStatus))
	{
		status = DynamicZoneMemberStatus::Online;
	}

	if (character_id == m_leader.id)
	{
		m_leader.status = status;
	}

	auto it = std::find_if(m_members.begin(), m_members.end(),
		[&](const DynamicZoneMember& member) { return member.id == character_id; });

	if (it != m_members.end() && it->status != status)
	{
		it->status = status;
		return true;
	}

	return false;
}

void DynamicZoneBase::SetMemberStatus(uint32_t character_id, DynamicZoneMemberStatus status)
{
	auto update_member = GetMemberData(character_id);
	if (update_member.IsValid())
	{
		ProcessMemberStatusChange(character_id, status);
		SendServerPacket(CreateServerMemberStatusPacket(character_id, status).get());
	}
}

void DynamicZoneBase::ProcessMemberAddRemove(const DynamicZoneMember& member, bool removed)
{
	if (!removed)
	{
		AddInternalMember(member);
	}
	else
	{
		RemoveInternalMember(member.id);
	}
}

bool DynamicZoneBase::ProcessMemberStatusChange(uint32_t character_id, DynamicZoneMemberStatus status)
{
	return SetInternalMemberStatus(character_id, status);
}

std::string DynamicZoneBase::GetDynamicZoneTypeName(DynamicZoneType dz_type)
{
	switch (dz_type)
	{
		case DynamicZoneType::Expedition:
			return "Expedition";
		case DynamicZoneType::Tutorial:
			return "Tutorial";
		case DynamicZoneType::Task:
			return "Task";
		case DynamicZoneType::Mission:
			return "Mission";
		case DynamicZoneType::Quest:
			return "Quest";
		default:
			return "Unknown";
	}
}

std::unique_ptr<ServerPacket> DynamicZoneBase::CreateServerPacket(uint16_t zone_id, uint16_t instance_id)
{
	std::ostringstream ss = GetSerialized();
	std::string_view sv = ss.view();

	auto pack_size = sizeof(ServerDzCreate_Struct) + sv.size();
	auto pack = std::make_unique<ServerPacket>(ServerOP_DzCreated, static_cast<uint32_t>(pack_size));
	auto buf = reinterpret_cast<ServerDzCreate_Struct*>(pack->pBuffer);
	buf->origin_zone_id = zone_id;
	buf->origin_instance_id = instance_id;
	buf->dz_id = GetID();
	buf->cereal_size = static_cast<uint32_t>(sv.size());
	memcpy(buf->cereal_data, sv.data(), sv.size());

	return pack;
}

std::ostringstream DynamicZoneBase::GetSerialized()
{
	std::ostringstream ss;
	cereal::BinaryOutputArchive archive(ss);
	archive(*this);
	return ss;
}

void DynamicZoneBase::Unserialize(std::span<char> buf)
{
	EQ::Util::MemoryStreamReader ss(buf.data(), buf.size());
	cereal::BinaryInputArchive archive(ss);
	archive(*this);
}

void DynamicZoneBase::LoadTemplate(const DynamicZoneTemplatesRepository::DynamicZoneTemplates& dz_template)
{
	m_zone_id            = dz_template.zone_id;
	m_zone_version       = dz_template.zone_version;
	m_name               = dz_template.name;
	m_min_players        = dz_template.min_players;
	m_max_players        = dz_template.max_players;
	m_duration           = std::chrono::seconds(dz_template.duration_seconds);
	m_dz_switch_id       = dz_template.dz_switch_id;
	m_compass.zone_id    = dz_template.compass_zone_id;
	m_compass.x          = dz_template.compass_x;
	m_compass.y          = dz_template.compass_y;
	m_compass.z          = dz_template.compass_z;
	m_safereturn.zone_id = dz_template.return_zone_id;
	m_safereturn.x       = dz_template.return_x;
	m_safereturn.y       = dz_template.return_y;
	m_safereturn.z       = dz_template.return_z;
	m_safereturn.heading = dz_template.return_h;
	m_has_zonein         = dz_template.override_zone_in;
	m_zonein.x           = dz_template.zone_in_x;
	m_zonein.y           = dz_template.zone_in_y;
	m_zonein.z           = dz_template.zone_in_z;
	m_zonein.heading     = dz_template.zone_in_h;
}

std::vector<uint32_t> DynamicZoneBase::GetMemberIds()
{
	std::vector<uint32_t> ids;
	ids.reserve(m_members.size());
	for (const auto& member : m_members)
	{
		ids.push_back(member.id);
	}
	return ids;
}

bool DynamicZoneBase::HasLockout(const std::string& event)
{
	return std::ranges::any_of(m_lockouts, [&](const auto& l) { return l.IsEvent(event); });
}

bool DynamicZoneBase::HasReplayLockout()
{
	return HasLockout(DzLockout::ReplayTimer);
}

void DynamicZoneBase::AddLockout(const std::string& event, uint32_t seconds)
{
	auto lockout = DzLockout::Create(m_name, event, seconds, m_uuid);
	AddLockout(lockout);
}

void DynamicZoneBase::AddLockout(const DzLockout& lockout, bool members_only)
{
	if (!members_only)
	{
		DynamicZoneLockoutsRepository::InsertLockouts(GetDatabase(), GetID(), { lockout });
	}

	CharacterExpeditionLockoutsRepository::InsertLockout(GetDatabase(), GetMemberIds(), lockout);

	HandleLockoutUpdate(lockout, false, members_only);
	SendServerPacket(CreateLockoutPacket(lockout, false, members_only).get());
}

void DynamicZoneBase::AddLockoutDuration(const std::string& event, int seconds, bool members_only)
{
	auto lockout = DzLockout::Create(m_name, event, std::max(0, seconds), m_uuid);

	// lockout has unsigned duration, pass original seconds to support reducing existing timers
	int secs = static_cast<int>(seconds * RuleR(Expedition, LockoutDurationMultiplier));
	CharacterExpeditionLockoutsRepository::AddLockoutDuration(GetDatabase(), GetMemberIds(), lockout, secs);

	HandleLockoutDuration(lockout, seconds, members_only, true);
	SendServerPacket(CreateLockoutDurationPacket(lockout, seconds, members_only).get());
}

void DynamicZoneBase::UpdateLockoutDuration(const std::string& event, uint32_t seconds, bool members_only)
{
	// some live expeditions update existing lockout timers during progression
	auto it = std::ranges::find_if(m_lockouts, [&](const auto& l) { return l.IsEvent(event); });
	if (it != m_lockouts.end())
	{
		seconds = static_cast<uint32_t>(seconds * RuleR(Expedition, LockoutDurationMultiplier));
		DzLockout lockout(m_uuid, m_name, event, it->GetStartTime() + seconds, seconds);
		AddLockout(lockout, members_only);
	}
}

void DynamicZoneBase::RemoveLockout(const std::string& event)
{
	DynamicZoneLockoutsRepository::DeleteWhere(GetDatabase(), fmt::format(
		"dynamic_zone_id = {} AND event_name = '{}'", GetID(), Strings::Escape(event)));

	CharacterExpeditionLockoutsRepository::DeleteWhere(GetDatabase(), fmt::format(
		"character_id IN ({}) AND expedition_name = '{}' AND event_name = '{}'",
		fmt::join(GetMemberIds(), ","), Strings::Escape(m_name), Strings::Escape(event)));

	DzLockout lockout{m_uuid, m_name, event, 0, 0};
	HandleLockoutUpdate(lockout, true, false);
	SendServerPacket(CreateLockoutPacket(lockout, true).get());
}

void DynamicZoneBase::HandleLockoutUpdate(const DzLockout& lockout, bool remove, bool members_only)
{
	if (!members_only)
	{
		std::erase_if(m_lockouts, [&](const auto& l) { return l.IsEvent(lockout.Event()); });
		if (!remove)
		{
			m_lockouts.push_back(lockout);
		}
	}
}

void DynamicZoneBase::HandleLockoutDuration(const DzLockout& lockout, int seconds, bool members_only, bool insert_db)
{
	if (!members_only)
	{
		auto it = std::ranges::find_if(m_lockouts, [&](const auto& l) { return l.IsEvent(lockout.Event()); });
		if (it != m_lockouts.end())
		{
			it->AddLockoutTime(seconds);
		}
		else
		{
			it = m_lockouts.insert(m_lockouts.end(), lockout);
		}

		if (insert_db)
		{
			DynamicZoneLockoutsRepository::InsertLockouts(GetDatabase(), GetID(), { *it });
		}
	}
}

std::unique_ptr<ServerPacket> DynamicZoneBase::CreateLockoutPacket(const DzLockout& lockout, bool remove, bool members_only) const
{
	uint32_t pack_size = sizeof(ServerDzLockout_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_DzLockout, pack_size);
	auto buf = reinterpret_cast<ServerDzLockout_Struct*>(pack->pBuffer);
	buf->dz_id = GetID();
	buf->expire_time = lockout.GetExpireTime();
	buf->duration = lockout.GetDuration();
	buf->sender_zone_id = GetCurrentZoneID();
	buf->sender_instance_id = GetCurrentInstanceID();
	buf->remove = remove;
	buf->members_only = members_only;
	strn0cpy(buf->event_name, lockout.Event().c_str(), sizeof(buf->event_name));
	return pack;
}

std::unique_ptr<ServerPacket> DynamicZoneBase::CreateLockoutDurationPacket(const DzLockout& lockout, int seconds, bool members_only) const
{
	uint32_t pack_size = sizeof(ServerDzLockout_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_DzLockoutDuration, pack_size);
	auto buf = reinterpret_cast<ServerDzLockout_Struct*>(pack->pBuffer);
	buf->dz_id = GetID();
	buf->expire_time = lockout.GetExpireTime();
	buf->duration = lockout.GetDuration();
	buf->sender_zone_id = GetCurrentZoneID();
	buf->sender_instance_id = GetCurrentInstanceID();
	buf->members_only = members_only;
	buf->seconds = seconds;
	strn0cpy(buf->event_name, lockout.Event().c_str(), sizeof(buf->event_name));
	return pack;
}

void DynamicZoneBase::SyncCharacterLockouts(uint32_t char_id, std::vector<DzLockout>& lockouts)
{
	// adds missing event lockouts to client for this expedition and updates
	// client timers that are both shorter and from another expedition
	bool modified = false;

	for (const auto& lockout : m_lockouts)
	{
		if (lockout.IsReplay() || lockout.IsExpired() || lockout.UUID() != m_uuid)
		{
			continue;
		}

		auto it = std::find_if(lockouts.begin(), lockouts.end(), [&](const DzLockout& l) { return l.IsSame(lockout); });
		if (it == lockouts.end())
		{
			modified = true;
			lockouts.push_back(lockout); // insert missing
		}
		else if (it->GetSecondsRemaining() < lockout.GetSecondsRemaining() && it->UUID() != m_uuid)
		{
			// only update lockout timer not uuid so loot event apis still work
			modified = true;
			it->SetDuration(lockout.GetDuration());
			it->SetExpireTime(lockout.GetExpireTime());
		}
	}

	if (modified)
	{
		CharacterExpeditionLockoutsRepository::InsertLockouts(GetDatabase(), char_id, lockouts);
	}
}
