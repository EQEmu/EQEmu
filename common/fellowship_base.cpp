#include "common/fellowship_base.h"

#include "common/database.h"
#include "common/eqemu_logsys.h"
#include "common/repositories/fellowship_campsites_repository.h"
#include "common/repositories/fellowship_members_repository.h"
#include "common/repositories/fellowships_repository.h"
#include "common/servertalk.h"

#include <algorithm>
#include <cstring>
#include <iterator>

const FellowshipMember *FellowshipInfo::FindMember(uint32 character_id) const
{
	for (const auto &m : members) {
		if (m.character_id == character_id) {
			return &m;
		}
	}

	return nullptr;
}

BaseFellowshipManager *BaseFellowshipManager::SetDatabase(Database *db)
{
	m_db = db;

	return this;
}

bool BaseFellowshipManager::LoadFellowships()
{
	if (!m_db) {
		return false;
	}

	m_fellowships.clear();
	m_fellowship_by_character.clear();

	std::unordered_map<uint32, FellowshipInfo> loaded;
	for (const auto &e : FellowshipsRepository::All(*m_db)) {
		FellowshipInfo info{};
		info.id         = e.id;
		info.leader_id  = e.leader_id;
		info.motd       = e.motd;
		info.created_at = e.created_at;

		loaded.emplace(e.id, std::move(info));
	}

	uint32 skipped_members = 0;
	for (const auto &e : FellowshipMembersRepository::GetWhereWithCharacters(*m_db, "TRUE")) {
		auto it = loaded.find(e.member.fellowship_id);
		if (it == loaded.end() || !e.is_character_valid) {
			LogFellowshipsDetail(
				"Skipping member [{}] of fellowship [{}]: [{}]",
				e.member.character_id,
				e.member.fellowship_id,
				it == loaded.end() ? "no such fellowship" : "character missing or deleted"
			);
			skipped_members++;
			continue;
		}

		FellowshipMember m{};
		m.character_id   = e.member.character_id;
		m.joined_at      = e.member.joined_at;
		m.is_exp_sharing = e.member.is_exp_sharing != 0;
		m.name           = e.name;
		m.level          = e.level;
		m.class_         = e.class_;
		m.last_login     = e.last_login;

		it->second.members.push_back(std::move(m));
	}

	for (const auto &e : FellowshipCampsitesRepository::All(*m_db)) {
		auto it = loaded.find(e.fellowship_id);
		if (it == loaded.end()) {
			LogFellowshipsDetail("Skipping campsite of missing fellowship [{}]", e.fellowship_id);
			continue;
		}

		FellowshipCampsite c{};
		c.zone_id       = e.zone_id;
		c.instance_id   = e.instance_id;
		c.x             = e.x;
		c.y             = e.y;
		c.z             = e.z;
		c.heading       = e.heading;
		c.campfire_type = e.campfire_type;
		c.placed_at     = e.placed_at;
		c.expires_at    = e.expires_at;

		it->second.campsite = c;
	}

	uint32 member_count   = 0;
	uint32 campsite_count = 0;
	uint32 skipped        = 0;
	for (auto &[id, info] : loaded) {
		if (!info.FindMember(info.leader_id)) {
			LogFellowshipsDetail("Skipping fellowship [{}]: leader [{}] is not a member", id, info.leader_id);
			skipped++;
			continue;
		}

		member_count += info.members.size();
		campsite_count += info.campsite ? 1 : 0;
		CacheFellowship(std::move(info));
	}

	LogFellowships(
		"Loaded [{}] fellowships, [{}] members, [{}] campsites (skipped [{}] fellowships, [{}] members)",
		m_fellowships.size(),
		member_count,
		campsite_count,
		skipped,
		skipped_members
	);

	return true;
}

void BaseFellowshipManager::ApplyRefresh(const ServerFellowshipRefresh_Struct &in)
{
	EraseFellowship(in.fellowship_id);

	if (in.deleted) {
		return;
	}

	FellowshipInfo info{};
	info.id         = in.fellowship_id;
	info.leader_id  = in.leader_id;
	info.motd       = std::string(in.motd, strnlen(in.motd, sizeof(in.motd)));
	info.created_at = in.created_at;

	const uint32 count = std::min<uint32>(in.member_count, std::size(in.members));
	for (uint32 i = 0; i < count; i++) {
		FellowshipMember m{};
		m.character_id   = in.members[i].character_id;
		m.joined_at      = in.members[i].joined_at;
		m.is_exp_sharing = in.members[i].is_exp_sharing != 0;

		info.members.push_back(std::move(m));
	}

	if (in.campsite.expires_at) {
		FellowshipCampsite c{};
		c.zone_id       = in.campsite.zone_id;
		c.instance_id   = in.campsite.instance_id;
		c.x             = in.campsite.x;
		c.y             = in.campsite.y;
		c.z             = in.campsite.z;
		c.heading       = in.campsite_heading;
		c.campfire_type = in.campsite.campfire_type;
		c.expires_at    = in.campsite.expires_at;

		info.campsite = c;
	}

	CacheFellowship(std::move(info));
}

const FellowshipInfo *BaseFellowshipManager::GetFellowship(uint32 fellowship_id) const
{
	auto it = m_fellowships.find(fellowship_id);

	return it != m_fellowships.end() ? &it->second : nullptr;
}

uint32 BaseFellowshipManager::GetFellowshipIDByCharacterID(uint32 character_id) const
{
	auto it = m_fellowship_by_character.find(character_id);

	return it != m_fellowship_by_character.end() ? it->second : 0;
}

bool BaseFellowshipManager::IsLeader(uint32 fellowship_id, uint32 character_id) const
{
	const auto *f = GetFellowship(fellowship_id);

	return f && f->leader_id == character_id;
}

FellowshipCampsite_Struct BaseFellowshipManager::CampsiteToStruct(const std::optional<FellowshipCampsite> &campsite)
{
	FellowshipCampsite_Struct out{};
	if (!campsite) {
		return out;
	}

	out.x             = campsite->x;
	out.y             = campsite->y;
	out.z             = campsite->z;
	out.zone_id       = campsite->zone_id;
	out.instance_id   = campsite->instance_id;
	out.expires_at    = campsite->expires_at;
	out.campfire_type = campsite->campfire_type;

	return out;
}

void BaseFellowshipManager::CacheFellowship(FellowshipInfo &&info)
{
	for (const auto &m : info.members) {
		m_fellowship_by_character[m.character_id] = info.id;
	}

	const uint32 id = info.id;
	m_fellowships[id] = std::move(info);
}

void BaseFellowshipManager::EraseFellowship(uint32 fellowship_id)
{
	auto it = m_fellowships.find(fellowship_id);
	if (it == m_fellowships.end()) {
		return;
	}

	for (const auto &m : it->second.members) {
		auto c = m_fellowship_by_character.find(m.character_id);
		if (c != m_fellowship_by_character.end() && c->second == fellowship_id) {
			m_fellowship_by_character.erase(c);
		}
	}

	m_fellowships.erase(it);
}
