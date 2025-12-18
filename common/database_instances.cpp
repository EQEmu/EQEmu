/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2015 EQEMu Development Team (http://eqemulator.net)

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

#include "database.h"

#include "common/platform/inet.h"
#include "common/platform/platform.h"
#include "common/platform/win/include_windows.h"
#include "common/repositories/character_corpses_repository.h"
#include "common/repositories/data_buckets_repository.h"
#include "common/repositories/dynamic_zone_members_repository.h"
#include "common/repositories/dynamic_zones_repository.h"
#include "common/repositories/group_id_repository.h"
#include "common/repositories/instance_list_player_repository.h"
#include "common/repositories/instance_list_repository.h"
#include "common/repositories/raid_members_repository.h"
#include "common/repositories/respawn_times_repository.h"
#include "common/repositories/spawn_condition_values_repository.h"
#include "common/repositories/spawn2_disabled_repository.h"
#include "common/repositories/zone_state_spawns_repository.h"
#include "common/rulesys.h"
#include "common/strings.h"
#include "common/timer.h"
#include "common/unix.h"
#include "zone/zonedb.h"



bool Database::AddClientToInstance(uint16 instance_id, uint32 character_id)
{
	auto e = InstanceListPlayerRepository::NewEntity();

	e.id = instance_id;
	e.charid = character_id;

	return InstanceListPlayerRepository::ReplaceOne(*this, e);
}

bool Database::CheckInstanceByCharID(uint16 instance_id, uint32 character_id)
{
	if (!instance_id) {
		return false;
	}

	auto l = InstanceListPlayerRepository::GetWhere(
		*this,
		fmt::format(
			"id = {} AND charid = {}",
			instance_id,
			character_id
		)
	);
	if (l.empty()) {
		return false;
	}

	return true;
}

bool Database::CheckInstanceExists(uint16 instance_id)
{
	if (!instance_id) {
		return false;
	}

	auto i = InstanceListRepository::FindOne(*this, instance_id);
	if (!i.id) {
		return false;
	}

	return true;
}

bool Database::CheckInstanceExpired(uint16 instance_id)
{
	if (!instance_id) {
		return true;
	}

	auto i = InstanceListRepository::FindOne(*this, instance_id);
	if (!i.id) {
		return true;
	}

	if (i.never_expires) {
		return false;
	}

	timeval tv{};
	gettimeofday(&tv, nullptr);

	// Use uint64_t for the addition to prevent overflow
	uint64_t expiration_time = static_cast<uint64_t>(i.start_time) + static_cast<uint64_t>(i.duration);
	return expiration_time <= tv.tv_sec;
}

bool Database::CreateInstance(uint16 instance_id, uint32 zone_id, uint32 version, uint32 duration)
{
	auto e = InstanceListRepository::NewEntity();

	e.id = instance_id;
	e.zone = zone_id;
	e.version = version;
	e.start_time = std::time(nullptr);
	e.duration = duration;
	e.expire_at = e.start_time + duration;

	RespawnTimesRepository::ClearInstanceTimers(*this, e.id);
	InstanceListRepository::ReplaceOne(*this, e);
	return instance_id > 0 && e.id;
}

bool Database::GetUnusedInstanceID(uint16 &instance_id)
{
	// attempt to get an unused instance id
	for (int a = 0; a < 10; a++) {
		uint16 attempted_id = 0;
		if (TryGetUnusedInstanceID(attempted_id)) {
			auto i = InstanceListRepository::NewEntity();
			i.id    = attempted_id;
			i.notes = "Prefetching";
			auto n = InstanceListRepository::InsertOne(*this, i);
			if (n.id > 0) {
				instance_id = n.id;
				return true;
			}
		}
	}

	instance_id = 0;
	return false;
}

bool Database::TryGetUnusedInstanceID(uint16 &instance_id)
{
	uint32 max_reserved_instance_id = RuleI(Instances, ReservedInstances);
	uint32 max_instance_id          = 32000;

	// sanity check reserved
	if (max_reserved_instance_id >= max_instance_id) {
		instance_id = 0;
		return false;
	}

	// recycle instances
	if (RuleB(Instances, RecycleInstanceIds)) {

		//query to get first unused id above reserved
		auto query = fmt::format(
			SQL(
				SELECT id
				FROM instance_list
				WHERE id = {};
			),
			max_reserved_instance_id + 1
		);

		auto results = QueryDatabase(query);

		// could not successfully query - bail out
		if (!results.Success()) {
			instance_id = 0;
			return false;
		}

		// first id is available
		if (results.RowCount() == 0) {
			instance_id = max_reserved_instance_id + 1;
			return true;
		}

		// now look for next available above reserved
		query = fmt::format(
			SQL(
				SELECT MIN(i.id + 1) AS next_available
				FROM instance_list i
				LEFT JOIN instance_list i2 ON i.id + 1 = i2.id
				WHERE i.id >= {}
				AND i2.id IS NULL;
			),
			max_reserved_instance_id
		);

		results = QueryDatabase(query);

		// could not successfully query - bail out
		if (!results.Success()) {
			instance_id = 0;
			return false;
		}

		// did not retrieve any rows - bail out
		if (results.RowCount() == 0) {
			instance_id = 0;
			return false;
		}

		auto row = results.begin();

		// check that id is within limits
		if (row[0] && Strings::ToInt(row[0]) <= max_instance_id) {
			instance_id = Strings::ToInt(row[0]);
			return true;
		}

		// no available instance ids
		instance_id = 0;
		return false;
	}

	// get max unused id above reserved
	auto query = fmt::format(
		"SELECT IFNULL(MAX(id), {}) + 1 FROM instance_list WHERE id > {}",
		max_reserved_instance_id,
		max_reserved_instance_id
	);

	auto results = QueryDatabase(query);

	// could not successfully query - bail out
	if (!results.Success()) {
		instance_id = 0;
		return false;
	}

	// did not retrieve any rows - bail out
	if (results.RowCount() == 0) {
		instance_id = 0;
		return false;
	}

	auto row = results.begin();

	// no instances currently used
	if (!row[0]) {
		instance_id = max_reserved_instance_id + 1;
		return true;
	}

	// check that id is within limits
	if (Strings::ToInt(row[0]) <= max_instance_id) {
		instance_id = Strings::ToInt(row[0]);
		return true;
	}

	// no available instance ids
	instance_id = 0;
	return false;
}

bool Database::IsGlobalInstance(uint16 instance_id)
{
	if (!instance_id) {
		return false;
	}

	auto i = InstanceListRepository::FindOne(*this, instance_id);
	if (!i.id) {
		return false;
	}

	return i.is_global;
}

bool Database::RemoveClientFromInstance(uint16 instance_id, uint32 char_id)
{
	return InstanceListPlayerRepository::DeleteWhere(
		*this,
		fmt::format(
			"id = {} AND charid = {}",
			instance_id,
			char_id
		)
	);
}

bool Database::RemoveClientsFromInstance(uint16 instance_id)
{
	return InstanceListPlayerRepository::DeleteOne(*this, instance_id);
}

bool Database::VerifyInstanceAlive(uint16 instance_id, uint32 character_id)
{
	//we are not saved to this instance so set our instance to 0
	if (!IsGlobalInstance(instance_id) && !CheckInstanceByCharID(instance_id, character_id)) {
		return false;
	}

	if (CheckInstanceExpired(instance_id)) {
		DeleteInstance(instance_id);
		return false;
	}

	return true;
}

bool Database::VerifyZoneInstance(uint32 zone_id, uint16 instance_id)
{
	auto l = InstanceListRepository::GetWhere(
		*this,
		fmt::format(
			"id = {} AND zone = {}",
			instance_id,
			zone_id
		)
	);
	if (l.empty()) {
		return false;
	}

	return true;
}

uint16 Database::GetInstanceID(uint32 zone_id, uint32 character_id, int16 version)
{
	if (!zone_id) {
		return 0;
	}

	const auto query = fmt::format(
		"SELECT instance_list.id FROM "
		"instance_list, instance_list_player WHERE "
		"instance_list.zone = {} AND "
		"instance_list.version = {} AND "
		"instance_list.id = instance_list_player.id AND "
		"instance_list_player.charid = {} "
		"LIMIT 1;",
		zone_id,
		version,
		character_id
	);
	auto results = QueryDatabase(query);

	if (!results.Success() || !results.RowCount()) {
		return 0;
	}

	auto row = results.begin();

	return static_cast<uint16>(Strings::ToUnsignedInt(row[0]));
}

std::vector<uint16> Database::GetInstanceIDs(uint32 zone_id, uint32 character_id)
{
	std::vector<uint16> l;

	if (!zone_id) {
		return l;
	}

	const auto query = fmt::format(
		"SELECT instance_list.id FROM "
		"instance_list, instance_list_player WHERE "
		"instance_list.zone = {} AND "
		"instance_list.id = instance_list_player.id AND "
		"instance_list_player.charid = {}",
		zone_id,
		character_id
	);
	auto results = QueryDatabase(query);

	if (!results.Success() || !results.RowCount()) {
		return l;
	}

	for (auto row : results) {
		l.push_back(static_cast<uint16>(Strings::ToUnsignedInt(row[0])));
	}

	return l;
}

uint8_t Database::GetInstanceVersion(uint16 instance_id) {
	if (!instance_id) {
		return 0;
	}

	auto i = InstanceListRepository::FindOne(*this, instance_id);
	if (!i.id) {
		return 0;
	}

	return i.version;
}

uint32 Database::GetTimeRemainingInstance(uint16 instance_id, bool &is_perma)
{
	auto i = InstanceListRepository::FindOne(*this, instance_id);
	if (!i.id) {
		is_perma = false;
		return 0;
	}

	if (i.never_expires) {
		is_perma = true;
		return 0;
	}

	is_perma = false;

	timeval tv;
	gettimeofday(&tv, nullptr);
	return ((i.start_time + i.duration) - tv.tv_sec);
}

uint32 Database::GetInstanceZoneID(uint16 instance_id)
{
	if (!instance_id) {
		return 0;
	}

	auto i = InstanceListRepository::FindOne(*this, instance_id);
	if (!i.id) {
		return 0;
	}

	return i.zone;
}

void Database::AssignGroupToInstance(uint32 group_id, uint32 instance_id)
{
	auto zone_id = GetInstanceZoneID(instance_id);
	auto version = GetInstanceVersion(instance_id);

	const auto& l = GroupIdRepository::GetWhere(
		*this,
		fmt::format(
			"`group_id` = {}",
			group_id
		)
	);

	if (l.empty()) {
		return;
	}

	for (const auto& e : l) {
		if (!e.character_id) {
			continue;
		}

		if (!GetInstanceID(zone_id, e.character_id, version)) {
			AddClientToInstance(instance_id, e.character_id);
		}
	}
}

void Database::AssignRaidToInstance(uint32 raid_id, uint32 instance_id)
{
	auto zone_id = GetInstanceZoneID(instance_id);
	auto version = GetInstanceVersion(instance_id);

	auto l = RaidMembersRepository::GetWhere(
		*this,
		fmt::format(
			"raidid = {}",
			raid_id
		)
	);
	if (l.empty()) {
		return;
	}

	for (const auto& e : l) {
		if (!GetInstanceID(zone_id, e.charid, version)) {
			AddClientToInstance(instance_id, e.charid);
		}
	}
}

void Database::DeleteInstance(uint16 instance_id)
{
	// I'm not sure why this isn't in here but we should add it in a later change and make sure it's tested
	// InstanceListRepository::DeleteWhere(*this, fmt::format("id = {}", instance_id));
	InstanceListPlayerRepository::DeleteWhere(*this, fmt::format("id = {}", instance_id));
	RespawnTimesRepository::DeleteWhere(*this, fmt::format("instance_id = {}", instance_id));
	SpawnConditionValuesRepository::DeleteWhere(*this, fmt::format("instance_id = {}", instance_id));
	DynamicZoneMembersRepository::DeleteByInstance(*this, instance_id);
	DynamicZonesRepository::DeleteWhere(*this, fmt::format("instance_id = {}", instance_id));
	CharacterCorpsesRepository::BuryInstance(*this, instance_id);
	DataBucketsRepository::DeleteWhere(*this, fmt::format("instance_id = {}", instance_id));
	if (RuleB(Zone, StateSavingOnShutdown)) {
		ZoneStateSpawnsRepository::DeleteWhere(*this, fmt::format("`instance_id` = {}", instance_id));
	}
}

void Database::FlagInstanceByGroupLeader(uint32 zone_id, int16 version, uint32 character_id, uint32 group_id)
{
	auto instance_id = GetInstanceID(zone_id, character_id, version);
	if (instance_id) {
		return;
	}

	char ln[128];
	memset(ln, 0, 128);
	GetGroupLeadershipInfo(group_id, ln);

	auto group_leader_id = GetCharacterID((const char*)ln);
	auto group_leader_instance_id = GetInstanceID(zone_id, group_leader_id, version);

	if (!group_leader_instance_id) {
		return;
	}

	AddClientToInstance(group_leader_instance_id, character_id);
}

void Database::FlagInstanceByRaidLeader(uint32 zone_id, int16 version, uint32 character_id, uint32 raid_id)
{
	uint16 instance_id = GetInstanceID(zone_id, character_id, version);
	if (instance_id) {
		return;
	}

	auto raid_leader_id = GetCharacterID(GetRaidLeaderName(raid_id).c_str());
	auto raid_leader_instance_id = GetInstanceID(zone_id, raid_leader_id, version);

	if (!raid_leader_instance_id) {
		return;
	}

	AddClientToInstance(raid_leader_instance_id, character_id);
}

void Database::GetCharactersInInstance(uint16 instance_id, std::list<uint32> &character_ids)
{
	auto l = InstanceListPlayerRepository::GetWhere(*this, fmt::format("id = {}", instance_id));
	if (l.empty()) {
		return;
	}

	for (const auto& e : l) {
		character_ids.push_back(e.charid);
	}
}

void Database::PurgeExpiredInstances()
{
	auto l = InstanceListRepository::GetWhere(
		*this,
		fmt::format(
			"expire_at <= (UNIX_TIMESTAMP() - {}) and expire_at != 0 AND never_expires = 0",
			RuleI(Instances, ExpireOffsetTimeSeconds)
		)
	);
	if (l.empty()) {
		return;
	}

	std::vector<std::string> instance_ids;
	for (const auto& e : l) {
		instance_ids.emplace_back(std::to_string(e.id));
	}

	const auto ids = Strings::Implode(",", instance_ids);

	TransactionBegin();
	InstanceListRepository::DeleteWhere(*this, fmt::format("id IN ({})", ids));
	InstanceListPlayerRepository::DeleteWhere(*this, fmt::format("id IN ({})", ids));
	RespawnTimesRepository::DeleteWhere(*this, fmt::format("instance_id IN ({})", ids));
	SpawnConditionValuesRepository::DeleteWhere(*this, fmt::format("instance_id IN ({})", ids));
	CharacterCorpsesRepository::BuryInstances(*this, ids);
	DynamicZoneMembersRepository::DeleteByManyInstances(*this, ids);
	DynamicZonesRepository::DeleteWhere(*this, fmt::format("instance_id IN ({})", ids));
	Spawn2DisabledRepository::DeleteWhere(*this, fmt::format("instance_id IN ({})", ids));
	DataBucketsRepository::DeleteWhere(*this, fmt::format("instance_id != 0 and instance_id IN ({})", ids));
	if (RuleB(Zone, StateSavingOnShutdown)) {
		ZoneStateSpawnsRepository::DeleteWhere(*this, fmt::format("`instance_id` IN ({})", ids));
	}
	TransactionCommit();

	LogInfo("Purged [{}] expired instances", l.size());
}

void Database::SetInstanceDuration(uint16 instance_id, uint32 new_duration)
{
	auto i = InstanceListRepository::FindOne(*this, instance_id);
	if (!i.id) {
		return;
	}

	i.start_time = std::time(nullptr);
	i.duration = new_duration;
	i.expire_at = i.start_time + i.duration;

	InstanceListRepository::UpdateOne(*this, i);
}

void Database::CleanupInstanceCorpses() {
	auto l = InstanceListRepository::GetWhere(
		*this,
		"never_expires = 0"
	);

	if (l.empty()) {
		return;
	}

	std::vector<std::string> instance_ids;
	for (const auto& e : l) {
		instance_ids.emplace_back(std::to_string(e.id));
	}

	const auto imploded_instance_ids = Strings::Implode(",", instance_ids);

	CharacterCorpsesRepository::BuryInstances(*this, imploded_instance_ids);
}
