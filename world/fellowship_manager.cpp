#include "world/fellowship_manager.h"

#include "common/data_bucket.h"
#include "common/eq_constants.h"
#include "common/eqemu_logsys.h"
#include "common/fellowship.h"
#include "common/repositories/character_data_repository.h"
#include "common/repositories/fellowship_campsites_repository.h"
#include "common/repositories/fellowship_members_repository.h"
#include "common/repositories/fellowships_repository.h"
#include "common/rulesys.h"
#include "common/servertalk.h"
#include "common/strings.h"
#include "world/cliententry.h"
#include "world/clientlist.h"
#include "world/worlddb.h"
#include "world/zonelist.h"
#include "world/zoneserver.h"

#include <algorithm>
#include <cstring>
#include <ctime>
#include <iterator>
#include <unordered_set>

namespace {
	// eqstr ids, zone/string_ids.h is not visible from world
	constexpr uint32 FELLOWSHIP_NO_LONGER_MEMBER = 4590;
	constexpr uint32 FELLOWSHIP_DISBANDED        = 4591;
	constexpr uint32 FELLOWSHIP_MEMBER_LEFT      = 4592;
	constexpr uint32 FELLOWSHIP_FULL             = 11011;

	constexpr uint32 PROCESS_INTERVAL_MS = 10000;

	uint32 Now()
	{
		return static_cast<uint32>(std::time(nullptr));
	}

	FellowshipMember MakeMember(const CharacterDataRepository::CharacterData &character, uint32 joined_at)
	{
		FellowshipMember member{};
		member.character_id   = character.id;
		member.joined_at      = joined_at;
		member.is_exp_sharing = false;
		member.name           = character.name;
		member.level          = character.level;
		member.class_         = character.class_;
		member.last_login     = character.last_login;

		return member;
	}

	bool IsCharacterValid(const CharacterDataRepository::CharacterData &character)
	{
		return character.id != 0 && character.deleted_at <= 0;
	}

	// join order, the order the database and the zones use
	void SortMembers(std::vector<FellowshipMember> &members)
	{
		std::sort(
			members.begin(),
			members.end(),
			[](const FellowshipMember &a, const FellowshipMember &b) {
				if (a.joined_at != b.joined_at) {
					return a.joined_at < b.joined_at;
				}

				return a.character_id < b.character_id;
			}
		);
	}
}

WorldFellowshipManager::WorldFellowshipManager()
	: m_process_timer(PROCESS_INTERVAL_MS)
{
}

WorldFellowshipManager *WorldFellowshipManager::Instance()
{
	// never destroyed: CLEs report their logout to it while the client list is torn down at shutdown
	static auto *instance = new WorldFellowshipManager;

	return instance;
}

bool WorldFellowshipManager::IsSameSnapshot(const RosterSnapshot &a, const RosterSnapshot &b)
{
	return a.zone_id == b.zone_id &&
		   a.instance_id == b.instance_id &&
		   a.level == b.level &&
		   a.class_ == b.class_ &&
		   a.is_online == b.is_online;
}

std::vector<uint32> WorldFellowshipManager::GetMemberIDs(const FellowshipInfo &info)
{
	std::vector<uint32> ids;
	ids.reserve(info.members.size());

	for (const auto &m : info.members) {
		ids.push_back(m.character_id);
	}

	return ids;
}

ClientListEntry *WorldFellowshipManager::FindOnlineEntry(uint32 character_id) const
{
	if (!character_id) {
		return nullptr;
	}

	auto *cle = ClientList::Instance()->FindCLEByCharacterID(character_id);
	if (!cle) {
		return nullptr;
	}

	const auto status = cle->GetOnline();
	if (status != CLE_Status::InZone && status != CLE_Status::Zoning) {
		return nullptr;
	}

	return cle;
}

WorldFellowshipManager::RosterSnapshot WorldFellowshipManager::GetSnapshot(uint32 character_id) const
{
	RosterSnapshot snapshot{};

	const auto *cle = FindOnlineEntry(character_id);
	if (!cle) {
		return snapshot;
	}

	snapshot.is_online = true;
	snapshot.level     = cle->level();
	snapshot.class_    = cle->class_();

	if (cle->GetOnline() == CLE_Status::InZone) {
		snapshot.zone_id     = cle->zone();
		snapshot.instance_id = cle->instance();
	}

	return snapshot;
}

std::string WorldFellowshipManager::GetMemberName(const FellowshipMember &member) const
{
	const auto *cle = FindOnlineEntry(member.character_id);

	return cle ? std::string(cle->name()) : member.name;
}

FellowshipUpdate_Struct WorldFellowshipManager::BuildRoster(const FellowshipInfo &info) const
{
	FellowshipUpdate_Struct roster{};
	roster.fellowship_id = info.id;
	strn0cpy(roster.motd, info.motd.c_str(), sizeof(roster.motd));
	roster.campsite = CampsiteToStruct(info.campsite);

	// leader first, then join order
	std::vector<const FellowshipMember *> ordered;
	ordered.reserve(info.members.size());

	const auto *leader = info.FindMember(info.leader_id);
	if (leader) {
		ordered.push_back(leader);
	}

	for (const auto &m : info.members) {
		if (m.character_id != info.leader_id) {
			ordered.push_back(&m);
		}
	}

	const uint32 now   = Now();
	const uint32 count = std::min<uint32>(ordered.size(), std::size(roster.members));
	for (uint32 i = 0; i < count; i++) {
		const auto &member = *ordered[i];
		auto &out          = roster.members[i];

		out.character_id   = member.character_id;
		out.is_exp_sharing = member.is_exp_sharing ? 1 : 0;

		const auto *cle = FindOnlineEntry(member.character_id);
		if (!cle) {
			strn0cpy(out.name, member.name.c_str(), sizeof(out.name));
			out.level   = member.level;
			out.class_  = member.class_;
			out.last_on = member.last_login;
			continue;
		}

		strn0cpy(out.name, cle->name(), sizeof(out.name));
		out.level   = cle->level();
		out.class_  = cle->class_();
		out.last_on = now;

		if (cle->GetOnline() == CLE_Status::InZone) {
			out.zone_id     = cle->zone();
			out.instance_id = cle->instance();
		}
	}

	roster.member_count = count;

	return roster;
}

std::map<ZoneServer *, std::vector<uint32>> WorldFellowshipManager::GroupByZoneServer(
	const std::vector<uint32> &character_ids
) const
{
	std::map<ZoneServer *, std::vector<uint32>> groups;

	for (const auto character_id : character_ids) {
		const auto *cle = FindOnlineEntry(character_id);
		if (!cle) {
			continue;
		}

		auto *zone_server = cle->Server();
		if (!zone_server) {
			continue;
		}

		groups[zone_server].push_back(character_id);
	}

	return groups;
}

std::unique_ptr<ServerPacket> WorldFellowshipManager::BuildRefreshPacket(
	const FellowshipInfo &state,
	bool deleted,
	uint32 joined_character_id,
	const std::vector<uint32> &before_ids
) const
{
	auto pack = std::make_unique<ServerPacket>(ServerOP_FellowshipRefresh, sizeof(ServerFellowshipRefresh_Struct));
	auto *out = reinterpret_cast<ServerFellowshipRefresh_Struct *>(pack->pBuffer);

	out->fellowship_id = state.id;
	out->deleted       = deleted ? 1 : 0;
	out->leader_id     = state.leader_id;
	out->created_at    = state.created_at;
	strn0cpy(out->motd, state.motd.c_str(), sizeof(out->motd));

	const uint32 count = std::min<uint32>(state.members.size(), std::size(out->members));
	for (uint32 i = 0; i < count; i++) {
		out->members[i].character_id   = state.members[i].character_id;
		out->members[i].joined_at      = state.members[i].joined_at;
		out->members[i].is_exp_sharing = state.members[i].is_exp_sharing ? 1 : 0;
	}
	out->member_count = count;

	if (!deleted) {
		out->campsite         = CampsiteToStruct(state.campsite);
		out->campsite_heading = state.campsite ? state.campsite->heading : 0.0f;
	}

	out->joined_character_id = joined_character_id;

	for (const auto character_id : before_ids) {
		if (out->removed_count >= std::size(out->removed_character_ids)) {
			break;
		}

		if (!deleted && state.FindMember(character_id)) {
			continue;
		}

		out->removed_character_ids[out->removed_count++] = character_id;
	}

	return pack;
}

void WorldFellowshipManager::SendRefresh(
	const FellowshipInfo &state,
	bool deleted,
	uint32 joined_character_id,
	const std::vector<uint32> &before_ids
)
{
	const auto pack = BuildRefreshPacket(state, deleted, joined_character_id, before_ids);

	// zones holding a member before or after the change
	std::vector<uint32> ids = before_ids;
	for (const auto &m : state.members) {
		if (std::find(ids.begin(), ids.end(), m.character_id) == ids.end()) {
			ids.push_back(m.character_id);
		}
	}

	const auto groups = GroupByZoneServer(ids);
	for (const auto &group : groups) {
		group.first->SendPacket(pack.get());
	}

	LogFellowshipsDetail(
		"Sent refresh of fellowship [{}] deleted [{}] to [{}] zone servers",
		state.id,
		deleted,
		groups.size()
	);
}

void WorldFellowshipManager::SendUpdate(
	const std::vector<uint32> &recipients,
	uint32 fellowship_id,
	const FellowshipUpdate_Struct &roster,
	uint32 string_id,
	uint32 string_type,
	const std::string &string_argument
)
{
	const auto groups = GroupByZoneServer(recipients);
	if (groups.empty()) {
		LogFellowshipsDetail("No online recipients for update of fellowship [{}]", fellowship_id);
		return;
	}

	for (const auto &group : groups) {
		ServerPacket pack(ServerOP_FellowshipUpdate, sizeof(ServerFellowshipUpdate_Struct));
		auto *out = reinterpret_cast<ServerFellowshipUpdate_Struct *>(pack.pBuffer);

		out->fellowship_id = fellowship_id;
		out->string_id     = string_id;
		out->string_type   = string_type;
		strn0cpy(out->string_argument, string_argument.c_str(), sizeof(out->string_argument));

		out->recipient_count = std::min<uint32>(group.second.size(), std::size(out->recipients));
		for (uint32 i = 0; i < out->recipient_count; i++) {
			out->recipients[i] = group.second[i];
		}

		out->roster = roster;

		group.first->SendPacket(&pack);

		LogFellowshipsDetail(
			"Sent update of fellowship [{}] string [{}] to [{}] recipients on zone server [{}]",
			fellowship_id,
			string_id,
			out->recipient_count,
			group.first->GetID()
		);
	}
}

void WorldFellowshipManager::SendRosterToOnlineMembers(const FellowshipInfo &info)
{
	SendUpdate(GetMemberIDs(info), info.id, BuildRoster(info), 0, 0, "");
	RecordSnapshots(info);
}

void WorldFellowshipManager::SendCampsite(const FellowshipInfo &info, const FellowshipCampsite &campsite, bool removed)
{
	ServerPacket pack(ServerOP_FellowshipCampsite, sizeof(ServerFellowshipCampsite_Struct));
	auto *out = reinterpret_cast<ServerFellowshipCampsite_Struct *>(pack.pBuffer);

	out->fellowship_id    = info.id;
	out->removed          = removed ? 1 : 0;
	out->campsite         = CampsiteToStruct(campsite);
	out->campsite_heading = campsite.heading;

	const auto *leader = info.FindMember(info.leader_id);
	if (leader) {
		strn0cpy(out->leader_name, GetMemberName(*leader).c_str(), sizeof(out->leader_name));
	}

	const bool sent = ZSList::Instance()->SendPacket(campsite.zone_id, campsite.instance_id, &pack);
	LogFellowshipsDetail(
		"Sent campsite of fellowship [{}] removed [{}] to zone [{}] instance [{}] delivered [{}]",
		info.id,
		removed,
		campsite.zone_id,
		campsite.instance_id,
		sent
	);
}

void WorldFellowshipManager::RecordSnapshots(const FellowshipInfo &info)
{
	for (const auto &m : info.members) {
		m_last_sent[m.character_id] = GetSnapshot(m.character_id);
	}
}

void WorldFellowshipManager::ReplaceFellowship(FellowshipInfo &&info)
{
	EraseFellowship(info.id);
	CacheFellowship(std::move(info));
}

bool WorldFellowshipManager::RunTransaction(const std::function<bool()> &write, const std::string &description)
{
	m_db->TransactionBegin();

	if (write()) {
		const auto commit = m_db->TransactionCommit();
		if (commit.Success()) {
			return true;
		}
	}

	m_db->TransactionRollback();
	LogFellowships("Fellowship change [{}] failed and was rolled back", description);

	return false;
}

void WorldFellowshipManager::WriteRejoinBucket(uint32 character_id)
{
	const int minutes = RuleI(Character, FellowshipRejoinCooldownMinutes);
	if (minutes <= 0) {
		return;
	}

	DataBucket::SetData(
		&database,
		DataBucketKey{
			.key          = Fellowship::REJOIN_BUCKET,
			.value        = "1",
			.expires      = fmt::format("{}s", minutes * 60),
			.character_id = character_id
		}
	);

	// world never reads the bucket, keep its cache from growing
	DataBucket::DeleteCachedBuckets(DataBucketLoadType::Client, character_id);
}

bool WorldFellowshipManager::RefreshOfflineMember(uint32 character_id)
{
	const auto rows = FellowshipMembersRepository::GetWhereWithCharacters(
		*m_db,
		fmt::format("m.`character_id` = {}", character_id)
	);

	if (rows.empty()) {
		LogFellowshipsDetail("No member row for character [{}] while refreshing offline fields", character_id);
		return true;
	}

	const auto &row = rows.front();
	if (!row.is_character_valid) {
		LogFellowships("Character [{}] no longer exists, removing from fellowship", character_id);
		RemoveCharacter(character_id);
		return false;
	}

	auto fellowship = m_fellowships.find(row.member.fellowship_id);
	if (fellowship == m_fellowships.end()) {
		return true;
	}

	for (auto &m : fellowship->second.members) {
		if (m.character_id != character_id) {
			continue;
		}

		m.name       = row.name;
		m.level      = row.level;
		m.class_     = row.class_;
		m.last_login = row.last_login;
	}

	return true;
}

void WorldFellowshipManager::AnnounceRepairedFellowship(uint32 fellowship_id, uint32 character_id)
{
	auto announced = m_unannounced.find(fellowship_id);
	if (announced == m_unannounced.end()) {
		return;
	}

	const auto *info = GetFellowship(fellowship_id);
	if (!info) {
		m_unannounced.erase(announced);
		return;
	}

	const auto *cle = FindOnlineEntry(character_id);
	if (!cle || !cle->Server()) {
		return;
	}

	auto *zone_server = cle->Server();
	if (!announced->second.count(zone_server->GetID())) {
		const auto pack = BuildRefreshPacket(*info, false, 0, {});
		zone_server->SendPacket(pack.get());
		announced->second.insert(zone_server->GetID());
	}

	// done once every zone that holds a member has heard it
	bool any_online = false;
	bool all_told   = true;
	for (const auto &group : GroupByZoneServer(GetMemberIDs(*info))) {
		any_online = true;
		if (!announced->second.count(group.first->GetID())) {
			all_told = false;
		}
	}

	if (any_online && all_told) {
		LogFellowships("Announced repair of fellowship [{}] to the zones of its members", fellowship_id);
		m_unannounced.erase(announced);
	}
}

void WorldFellowshipManager::RepairAfterLoad()
{
	if (!m_db) {
		return;
	}

	const auto fellowships = FellowshipsRepository::All(*m_db);
	const auto members     = FellowshipMembersRepository::GetWhereWithCharacters(*m_db, "TRUE");
	const auto campsites   = FellowshipCampsitesRepository::All(*m_db);

	std::unordered_set<uint32> fellowship_ids;
	for (const auto &f : fellowships) {
		fellowship_ids.insert(f.id);
	}

	uint32 removed_members     = 0;
	uint32 promoted_leaders    = 0;
	uint32 deleted_fellowships = 0;
	uint32 deleted_campsites   = 0;

	std::set<uint32>                                repaired_ids;
	std::unordered_set<uint32>                      deleted_ids;
	std::unordered_map<uint32, std::vector<uint32>> valid_members; // join order

	for (const auto &e : members) {
		const bool has_fellowship = fellowship_ids.count(e.member.fellowship_id) > 0;
		if (has_fellowship && e.is_character_valid) {
			valid_members[e.member.fellowship_id].push_back(e.member.character_id);
			continue;
		}

		const bool fixed = RunTransaction(
			[&]() {
				return FellowshipMembersRepository::DeleteOne(*m_db, e.member.character_id) == 1;
			},
			fmt::format("repair: delete member [{}]", e.member.character_id)
		);
		if (!fixed) {
			continue;
		}

		LogFellowships(
			"Repair: deleted member [{}] of fellowship [{}] ([{}])",
			e.member.character_id,
			e.member.fellowship_id,
			has_fellowship ? "character missing or deleted" : "no such fellowship"
		);

		removed_members++;
		if (has_fellowship) {
			repaired_ids.insert(e.member.fellowship_id);
		}
	}

	for (const auto &f : fellowships) {
		const auto valid = valid_members.find(f.id);
		if (valid == valid_members.end() || valid->second.empty()) {
			const bool fixed = RunTransaction(
				[&]() {
					FellowshipCampsitesRepository::DeleteOne(*m_db, f.id);
					return FellowshipsRepository::DeleteOne(*m_db, f.id) == 1;
				},
				fmt::format("repair: delete fellowship [{}]", f.id)
			);
			if (!fixed) {
				continue;
			}

			LogFellowships("Repair: deleted fellowship [{}] with no valid members", f.id);
			deleted_fellowships++;
			deleted_ids.insert(f.id);
			repaired_ids.erase(f.id);
			continue;
		}

		const auto &ids = valid->second;
		if (std::find(ids.begin(), ids.end(), f.leader_id) != ids.end()) {
			continue;
		}

		auto fixed_fellowship      = f;
		fixed_fellowship.leader_id = ids.front();

		const bool fixed = RunTransaction(
			[&]() {
				return FellowshipsRepository::UpdateOne(*m_db, fixed_fellowship) == 1;
			},
			fmt::format("repair: promote leader of fellowship [{}]", f.id)
		);
		if (!fixed) {
			continue;
		}

		LogFellowships(
			"Repair: leader [{}] of fellowship [{}] is not a member, promoted [{}]",
			f.leader_id,
			f.id,
			fixed_fellowship.leader_id
		);
		promoted_leaders++;
		repaired_ids.insert(f.id);
	}

	for (const auto &c : campsites) {
		if (fellowship_ids.count(c.fellowship_id) || deleted_ids.count(c.fellowship_id)) {
			continue;
		}

		const bool fixed = RunTransaction(
			[&]() {
				return FellowshipCampsitesRepository::DeleteOne(*m_db, c.fellowship_id) == 1;
			},
			fmt::format("repair: delete campsite [{}]", c.fellowship_id)
		);
		if (!fixed) {
			continue;
		}

		LogFellowships("Repair: deleted campsite of missing fellowship [{}]", c.fellowship_id);
		deleted_campsites++;
	}

	LogFellowships(
		"Repair finished: [{}] member rows deleted, [{}] leaders promoted, [{}] fellowships deleted, [{}] campsites deleted",
		removed_members,
		promoted_leaders,
		deleted_fellowships,
		deleted_campsites
	);

	if (!removed_members && !promoted_leaders && !deleted_fellowships && !deleted_campsites) {
		return;
	}

	LoadFellowships();

	// no zone is connected yet, announce when one boots or a member appears
	for (const auto fellowship_id : repaired_ids) {
		if (GetFellowship(fellowship_id)) {
			m_unannounced[fellowship_id] = {};
		}
	}
}

void WorldFellowshipManager::OnClientListChange(uint32 character_id)
{
	if (!character_id) {
		return;
	}

	const uint32 fellowship_id = GetFellowshipIDByCharacterID(character_id);
	if (!fellowship_id) {
		return;
	}

	AnnounceRepairedFellowship(fellowship_id, character_id);

	if (!RuleB(Character, EnableFellowships)) {
		return;
	}

	const auto current = GetSnapshot(character_id);
	const auto last    = m_last_sent.find(character_id);
	if (last != m_last_sent.end() && IsSameSnapshot(last->second, current)) {
		LogFellowshipsDetail("Skipped roster for fellowship [{}]: nothing changed for [{}]", fellowship_id, character_id);
		return;
	}

	const bool went_offline = !current.is_online && (last == m_last_sent.end() || last->second.is_online);
	m_last_sent[character_id] = current;

	if (went_offline && !RefreshOfflineMember(character_id)) {
		return;
	}

	const auto *info = GetFellowship(fellowship_id);
	if (!info) {
		return;
	}

	// the zone the member arrived in may not have seen the fellowship change
	const auto *cle = FindOnlineEntry(character_id);
	if (cle && cle->Server()) {
		const auto pack = BuildRefreshPacket(*info, false, 0, {});
		cle->Server()->SendPacket(pack.get());
	}

	SendRosterToOnlineMembers(*info);
}

void WorldFellowshipManager::OnZoneBootup(ZoneServer *zone_server)
{
	if (!zone_server || m_unannounced.empty()) {
		return;
	}

	for (auto announced = m_unannounced.begin(); announced != m_unannounced.end();) {
		const auto *info = GetFellowship(announced->first);
		if (!info) {
			announced = m_unannounced.erase(announced);
			continue;
		}

		const auto pack = BuildRefreshPacket(*info, false, 0, {});
		zone_server->SendPacket(pack.get());
		announced->second.insert(zone_server->GetID());

		LogFellowshipsDetail(
			"Announced repaired fellowship [{}] to booting zone server [{}]",
			announced->first,
			zone_server->GetID()
		);

		++announced;
	}
}

void WorldFellowshipManager::RemoveCharacter(uint32 character_id)
{
	const uint32 fellowship_id = GetFellowshipIDByCharacterID(character_id);
	if (!fellowship_id) {
		return;
	}

	LogFellowships("Removing deleted character [{}] from fellowship [{}]", character_id, fellowship_id);
	RemoveMember(fellowship_id, character_id);
}

void WorldFellowshipManager::SendChannelMessage(ServerPacket *pack, uint32 fellowship_id)
{
	const auto *info = GetFellowship(fellowship_id);
	if (!info) {
		LogFellowshipsDetail("Dropped chat for unknown fellowship [{}]", fellowship_id);
		return;
	}

	const auto groups = GroupByZoneServer(GetMemberIDs(*info));
	for (const auto &group : groups) {
		group.first->SendPacket(pack);
	}

	LogFellowshipsDetail("Routed chat of fellowship [{}] to [{}] zone servers", fellowship_id, groups.size());
}

void WorldFellowshipManager::Process()
{
	if (!m_process_timer.Check()) {
		return;
	}

	const uint32 now = Now();

	std::vector<uint32> expired;
	for (const auto &[fellowship_id, info] : m_fellowships) {
		if (info.campsite && info.campsite->expires_at <= now) {
			expired.push_back(fellowship_id);
		}
	}

	for (const auto fellowship_id : expired) {
		LogFellowships("Campsite of fellowship [{}] expired", fellowship_id);
		RemoveCampsite(fellowship_id);
	}
}

uint32 WorldFellowshipManager::GetActorFellowshipID(
	const ServerFellowshipAction_Struct &in,
	bool must_be_leader,
	const char *action_name
) const
{
	const uint32 fellowship_id = GetFellowshipIDByCharacterID(in.character_id);
	if (!fellowship_id) {
		LogFellowshipsDetail("Refused [{}] by [{}]: not in a fellowship", action_name, in.character_id);
		return 0;
	}

	if (in.fellowship_id && in.fellowship_id != fellowship_id) {
		LogFellowshipsDetail(
			"Refused [{}] by [{}]: zone sent fellowship [{}], world has [{}]",
			action_name,
			in.character_id,
			in.fellowship_id,
			fellowship_id
		);
		return 0;
	}

	if (must_be_leader && !IsLeader(fellowship_id, in.character_id)) {
		LogFellowshipsDetail("Refused [{}] by [{}]: not the leader of [{}]", action_name, in.character_id, fellowship_id);
		return 0;
	}

	return fellowship_id;
}

void WorldFellowshipManager::ProcessZonePacket(ServerPacket *pack)
{
	if (!pack || pack->opcode != ServerOP_FellowshipAction) {
		return;
	}

	if (!RuleB(Character, EnableFellowships)) {
		LogFellowshipsDetail("Ignored fellowship action: fellowships are disabled");
		return;
	}

	if (pack->size != sizeof(ServerFellowshipAction_Struct)) {
		LogError(
			"Received ServerOP_FellowshipAction of incorrect size [{}], expected [{}]",
			pack->size,
			sizeof(ServerFellowshipAction_Struct)
		);
		return;
	}

	const auto &in = *reinterpret_cast<const ServerFellowshipAction_Struct *>(pack->pBuffer);

	switch (in.action) {
		case FellowshipCreate: {
			CreateFellowship(in.character_id);
			break;
		}
		case FellowshipAcceptInvite: {
			AddMember(in.character_id, in.fellowship_id);
			break;
		}
		case FellowshipSetMotd: {
			const uint32 fellowship_id = GetActorFellowshipID(in, true, "set motd");
			if (!fellowship_id) {
				break;
			}

			const size_t length = strnlen(in.motd, sizeof(in.motd));
			if (length == sizeof(in.motd)) {
				LogFellowshipsDetail("Refused set motd by [{}]: motd is not terminated", in.character_id);
				break;
			}

			SetMotd(fellowship_id, std::string(in.motd, length));
			break;
		}
		case FellowshipRemoveMember: {
			const uint32 fellowship_id = GetActorFellowshipID(in, true, "remove member");
			if (!fellowship_id) {
				break;
			}

			if (in.target_character_id == in.character_id || GetFellowshipIDByCharacterID(in.target_character_id) != fellowship_id) {
				LogFellowshipsDetail(
					"Refused remove member by [{}]: target [{}] is the leader or not in fellowship [{}]",
					in.character_id,
					in.target_character_id,
					fellowship_id
				);
				break;
			}

			RemoveMember(fellowship_id, in.target_character_id);
			break;
		}
		case FellowshipPromoteLeader: {
			const uint32 fellowship_id = GetActorFellowshipID(in, true, "promote");
			if (!fellowship_id) {
				break;
			}

			if (in.target_character_id == in.character_id || GetFellowshipIDByCharacterID(in.target_character_id) != fellowship_id) {
				LogFellowshipsDetail(
					"Refused promote by [{}]: target [{}] is the leader or not in fellowship [{}]",
					in.character_id,
					in.target_character_id,
					fellowship_id
				);
				break;
			}

			SetLeader(fellowship_id, in.target_character_id);
			break;
		}
		case FellowshipCreateCampsite: {
			const uint32 fellowship_id = GetActorFellowshipID(in, false, "create campsite");
			if (!fellowship_id) {
				break;
			}

			PlaceCampsite(fellowship_id, in);
			break;
		}
		case FellowshipDestroyCampsite: {
			const uint32 fellowship_id = GetActorFellowshipID(in, false, "destroy campsite");
			if (!fellowship_id) {
				break;
			}

			RemoveCampsite(fellowship_id);
			break;
		}
		case FellowshipLeave: {
			const uint32 fellowship_id = GetActorFellowshipID(in, false, "leave");
			if (!fellowship_id) {
				break;
			}

			RemoveMember(fellowship_id, in.character_id);
			break;
		}
		case FellowshipDisband: {
			const uint32 fellowship_id = GetActorFellowshipID(in, true, "disband");
			if (!fellowship_id) {
				break;
			}

			Disband(fellowship_id);
			break;
		}
		case FellowshipToggleExpShare: {
			const uint32 fellowship_id = GetActorFellowshipID(in, false, "toggle exp sharing");
			if (!fellowship_id) {
				break;
			}

			const bool is_self = in.target_character_id == in.character_id;
			if (
				GetFellowshipIDByCharacterID(in.target_character_id) != fellowship_id ||
				(!is_self && !IsLeader(fellowship_id, in.character_id))
			) {
				LogFellowshipsDetail(
					"Refused toggle exp sharing by [{}]: target [{}] is not in fellowship [{}] or the actor is not its leader",
					in.character_id,
					in.target_character_id,
					fellowship_id
				);
				break;
			}

			SetExpSharing(fellowship_id, in.target_character_id);
			break;
		}
		case FellowshipCampsiteExpired: {
			RemoveCampsite(in.fellowship_id);
			break;
		}
		default: {
			LogFellowshipsDetail("Ignored fellowship action [{}] from [{}]", in.action, in.character_id);
			break;
		}
	}
}

bool WorldFellowshipManager::CreateFellowship(uint32 character_id)
{
	const uint32 current_id = GetFellowshipIDByCharacterID(character_id);
	if (current_id) {
		LogFellowshipsDetail("Refused create by [{}]: already in fellowship [{}]", character_id, current_id);
		return false;
	}

	const auto character = CharacterDataRepository::FindOne(*m_db, character_id);
	if (!IsCharacterValid(character)) {
		LogFellowshipsDetail("Refused create by [{}]: character missing or deleted", character_id);
		return false;
	}

	const uint32 now = Now();
	uint32 fellowship_id = 0;

	const bool written = RunTransaction(
		[&]() {
			FellowshipsRepository::Fellowships fellowship{};
			fellowship.leader_id  = character_id;
			fellowship.created_at = now;

			fellowship = FellowshipsRepository::InsertOne(*m_db, fellowship);
			if (!fellowship.id) {
				return false;
			}

			fellowship_id = fellowship.id;

			FellowshipMembersRepository::FellowshipMembers member{};
			member.character_id   = character_id;
			member.fellowship_id  = fellowship_id;
			member.joined_at      = now;
			member.is_exp_sharing = 0;

			return FellowshipMembersRepository::InsertMany(*m_db, {member}) == 1;
		},
		fmt::format("create by [{}]", character_id)
	);
	if (!written) {
		return false;
	}

	FellowshipInfo info{};
	info.id         = fellowship_id;
	info.leader_id  = character_id;
	info.created_at = now;
	info.members.push_back(MakeMember(character, now));
	CacheFellowship(std::move(info));

	WriteRejoinBucket(character_id);

	LogFellowships("Character [{}] created fellowship [{}]", character_id, fellowship_id);

	const auto *created = GetFellowship(fellowship_id);
	SendRefresh(*created, false, character_id, {});
	SendRosterToOnlineMembers(*created);

	return true;
}

bool WorldFellowshipManager::AddMember(uint32 character_id, uint32 fellowship_id)
{
	const auto *info = GetFellowship(fellowship_id);
	if (!info) {
		LogFellowshipsDetail("Refused accept by [{}]: fellowship [{}] does not exist", character_id, fellowship_id);
		return false;
	}

	const uint32 current_id = GetFellowshipIDByCharacterID(character_id);
	if (current_id) {
		LogFellowshipsDetail("Refused accept by [{}]: already in fellowship [{}]", character_id, current_id);
		return false;
	}

	if (info->members.size() >= Fellowship::MAX_MEMBERS) {
		LogFellowshipsDetail("Refused accept by [{}]: fellowship [{}] is full", character_id, fellowship_id);
		SendUpdate({character_id}, 0, FellowshipUpdate_Struct{}, FELLOWSHIP_FULL, Chat::Red, "");
		return false;
	}

	const auto character = CharacterDataRepository::FindOne(*m_db, character_id);
	if (!IsCharacterValid(character)) {
		LogFellowshipsDetail("Refused accept by [{}]: character missing or deleted", character_id);
		return false;
	}

	const uint32 now = Now();

	const bool written = RunTransaction(
		[&]() {
			FellowshipMembersRepository::FellowshipMembers member{};
			member.character_id   = character_id;
			member.fellowship_id  = fellowship_id;
			member.joined_at      = now;
			member.is_exp_sharing = 0;

			return FellowshipMembersRepository::InsertMany(*m_db, {member}) == 1;
		},
		fmt::format("join of [{}] to fellowship [{}]", character_id, fellowship_id)
	);
	if (!written) {
		return false;
	}

	const std::vector<uint32> before_ids = GetMemberIDs(*info);

	FellowshipInfo after = *info;
	after.members.push_back(MakeMember(character, now));
	SortMembers(after.members);
	ReplaceFellowship(std::move(after));

	WriteRejoinBucket(character_id);

	LogFellowships("Character [{}] joined fellowship [{}]", character_id, fellowship_id);

	const auto *joined = GetFellowship(fellowship_id);
	SendRefresh(*joined, false, character_id, before_ids);
	SendRosterToOnlineMembers(*joined);

	return true;
}

bool WorldFellowshipManager::RemoveMember(uint32 fellowship_id, uint32 character_id)
{
	const auto *cached = GetFellowship(fellowship_id);
	if (!cached || !cached->FindMember(character_id)) {
		LogFellowshipsDetail("Refused remove of [{}]: not in fellowship [{}]", character_id, fellowship_id);
		return false;
	}

	const FellowshipInfo before = *cached;
	const bool was_leader = before.leader_id == character_id;
	const std::string removed_name = GetMemberName(*before.FindMember(character_id));

	FellowshipInfo after = before;
	after.members.erase(
		std::remove_if(
			after.members.begin(),
			after.members.end(),
			[&](const FellowshipMember &m) {
				return m.character_id == character_id;
			}
		),
		after.members.end()
	);

	const bool is_gone = after.members.empty();
	if (is_gone) {
		after.campsite.reset();
	}
	else if (was_leader) {
		after.leader_id = after.members.front().character_id;
	}

	const bool written = RunTransaction(
		[&]() {
			if (FellowshipMembersRepository::DeleteOne(*m_db, character_id) != 1) {
				return false;
			}

			if (is_gone) {
				FellowshipCampsitesRepository::DeleteOne(*m_db, fellowship_id);
				return FellowshipsRepository::DeleteOne(*m_db, fellowship_id) == 1;
			}

			if (!was_leader) {
				return true;
			}

			FellowshipsRepository::Fellowships fellowship{};
			fellowship.id         = fellowship_id;
			fellowship.leader_id  = after.leader_id;
			fellowship.motd       = after.motd;
			fellowship.created_at = after.created_at;

			return FellowshipsRepository::UpdateOne(*m_db, fellowship) == 1;
		},
		fmt::format("removal of [{}] from fellowship [{}]", character_id, fellowship_id)
	);
	if (!written) {
		return false;
	}

	const std::vector<uint32> before_ids = GetMemberIDs(before);

	m_last_sent.erase(character_id);
	if (is_gone) {
		EraseFellowship(fellowship_id);
		LogFellowships("Character [{}] left fellowship [{}], which is now empty and deleted", character_id, fellowship_id);
	}
	else {
		LogFellowships("Character [{}] left fellowship [{}]", character_id, fellowship_id);
		if (was_leader) {
			LogFellowships("Promoted [{}] to leader of fellowship [{}]", after.leader_id, fellowship_id);
		}
		ReplaceFellowship(FellowshipInfo(after));
	}

	SendRefresh(is_gone ? before : after, is_gone, 0, before_ids);

	// the leaver sees the roster without them, which is how the client leaves
	const auto roster = BuildRoster(after);
	SendUpdate({character_id}, fellowship_id, roster, FELLOWSHIP_NO_LONGER_MEMBER, Chat::System, "");

	if (!is_gone) {
		SendUpdate(GetMemberIDs(after), fellowship_id, roster, FELLOWSHIP_MEMBER_LEFT, Chat::System, removed_name);
		RecordSnapshots(after);
	}

	if (is_gone && before.campsite) {
		SendCampsite(before, *before.campsite, true);
	}

	return true;
}

bool WorldFellowshipManager::SetLeader(uint32 fellowship_id, uint32 character_id)
{
	const auto *info = GetFellowship(fellowship_id);
	if (!info || !info->FindMember(character_id) || info->leader_id == character_id) {
		LogFellowshipsDetail("Refused promote of [{}] in fellowship [{}]", character_id, fellowship_id);
		return false;
	}

	const std::vector<uint32> before_ids = GetMemberIDs(*info);

	FellowshipInfo after = *info;
	after.leader_id      = character_id;

	const bool written = RunTransaction(
		[&]() {
			FellowshipsRepository::Fellowships fellowship{};
			fellowship.id         = fellowship_id;
			fellowship.leader_id  = after.leader_id;
			fellowship.motd       = after.motd;
			fellowship.created_at = after.created_at;

			return FellowshipsRepository::UpdateOne(*m_db, fellowship) == 1;
		},
		fmt::format("promote of [{}] in fellowship [{}]", character_id, fellowship_id)
	);
	if (!written) {
		return false;
	}

	ReplaceFellowship(std::move(after));

	LogFellowships("Promoted [{}] to leader of fellowship [{}]", character_id, fellowship_id);

	const auto *updated = GetFellowship(fellowship_id);
	SendRefresh(*updated, false, 0, before_ids);
	SendRosterToOnlineMembers(*updated);

	return true;
}

bool WorldFellowshipManager::SetMotd(uint32 fellowship_id, const std::string &motd)
{
	const auto *info = GetFellowship(fellowship_id);
	if (!info) {
		return false;
	}

	const std::vector<uint32> before_ids = GetMemberIDs(*info);

	FellowshipInfo after = *info;
	after.motd           = motd;

	const bool written = RunTransaction(
		[&]() {
			FellowshipsRepository::Fellowships fellowship{};
			fellowship.id         = fellowship_id;
			fellowship.leader_id  = after.leader_id;
			fellowship.motd       = after.motd;
			fellowship.created_at = after.created_at;

			return FellowshipsRepository::UpdateOne(*m_db, fellowship) == 1;
		},
		fmt::format("motd of fellowship [{}]", fellowship_id)
	);
	if (!written) {
		return false;
	}

	ReplaceFellowship(std::move(after));

	LogFellowshipsDetail("Fellowship [{}] motd set", fellowship_id);

	const auto *updated = GetFellowship(fellowship_id);
	SendRefresh(*updated, false, 0, before_ids);
	SendRosterToOnlineMembers(*updated);

	return true;
}

bool WorldFellowshipManager::SetExpSharing(uint32 fellowship_id, uint32 character_id)
{
	const auto *info = GetFellowship(fellowship_id);
	if (!info) {
		return false;
	}

	const auto *member = info->FindMember(character_id);
	if (!member) {
		return false;
	}

	const std::vector<uint32> before_ids = GetMemberIDs(*info);
	const bool is_exp_sharing            = !member->is_exp_sharing;
	const uint32 joined_at               = member->joined_at;

	const bool written = RunTransaction(
		[&]() {
			FellowshipMembersRepository::FellowshipMembers row{};
			row.character_id   = character_id;
			row.fellowship_id  = fellowship_id;
			row.joined_at      = joined_at;
			row.is_exp_sharing = is_exp_sharing ? 1 : 0;

			return FellowshipMembersRepository::UpdateOne(*m_db, row) == 1;
		},
		fmt::format("exp sharing of [{}] in fellowship [{}]", character_id, fellowship_id)
	);
	if (!written) {
		return false;
	}

	FellowshipInfo after = *info;
	for (auto &m : after.members) {
		if (m.character_id == character_id) {
			m.is_exp_sharing = is_exp_sharing;
		}
	}
	ReplaceFellowship(std::move(after));

	LogFellowshipsDetail("Fellowship [{}] member [{}] exp sharing [{}]", fellowship_id, character_id, is_exp_sharing);

	const auto *updated = GetFellowship(fellowship_id);
	SendRefresh(*updated, false, 0, before_ids);
	SendRosterToOnlineMembers(*updated);

	return true;
}

bool WorldFellowshipManager::PlaceCampsite(uint32 fellowship_id, const ServerFellowshipAction_Struct &in)
{
	const auto *info = GetFellowship(fellowship_id);
	if (!info) {
		return false;
	}

	if (info->campsite) {
		LogFellowshipsDetail("Refused campsite by [{}]: fellowship [{}] already has one", in.character_id, fellowship_id);
		return false;
	}

	if (!Fellowship::FindCampfireType(in.campfire_type)) {
		LogFellowshipsDetail("Refused campsite by [{}]: unknown campfire type [{}]", in.character_id, in.campfire_type);
		return false;
	}

	const uint32 now = Now();

	FellowshipCampsite campsite{};
	campsite.zone_id       = in.zone_id;
	campsite.instance_id   = in.instance_id;
	campsite.x             = in.x;
	campsite.y             = in.y;
	campsite.z             = in.z;
	campsite.heading       = in.heading;
	campsite.campfire_type = in.campfire_type;
	campsite.placed_at     = now;
	campsite.expires_at    = now + RuleI(Character, FellowshipCampsiteMinutes) * 60;

	const bool written = RunTransaction(
		[&]() {
			FellowshipCampsitesRepository::FellowshipCampsites row{};
			row.fellowship_id = fellowship_id;
			row.zone_id       = campsite.zone_id;
			row.instance_id   = campsite.instance_id;
			row.x             = campsite.x;
			row.y             = campsite.y;
			row.z             = campsite.z;
			row.heading       = campsite.heading;
			row.campfire_type = campsite.campfire_type;
			row.placed_at     = campsite.placed_at;
			row.expires_at    = campsite.expires_at;

			return FellowshipCampsitesRepository::InsertMany(*m_db, {row}) == 1;
		},
		fmt::format("campsite of fellowship [{}]", fellowship_id)
	);
	if (!written) {
		return false;
	}

	const std::vector<uint32> before_ids = GetMemberIDs(*info);

	FellowshipInfo after = *info;
	after.campsite       = campsite;
	ReplaceFellowship(std::move(after));

	LogFellowships(
		"Fellowship [{}] placed a campsite in zone [{}] instance [{}], expires at [{}]",
		fellowship_id,
		campsite.zone_id,
		campsite.instance_id,
		campsite.expires_at
	);

	const auto *updated = GetFellowship(fellowship_id);
	SendRefresh(*updated, false, 0, before_ids);
	SendRosterToOnlineMembers(*updated);
	SendCampsite(*updated, campsite, false);

	return true;
}

bool WorldFellowshipManager::RemoveCampsite(uint32 fellowship_id)
{
	const auto *info = GetFellowship(fellowship_id);
	if (!info || !info->campsite) {
		LogFellowshipsDetail("Refused campsite removal: fellowship [{}] has none", fellowship_id);
		return false;
	}

	const FellowshipCampsite campsite    = *info->campsite;
	const std::vector<uint32> before_ids = GetMemberIDs(*info);

	const bool written = RunTransaction(
		[&]() {
			return FellowshipCampsitesRepository::DeleteOne(*m_db, fellowship_id) == 1;
		},
		fmt::format("campsite removal of fellowship [{}]", fellowship_id)
	);
	if (!written) {
		return false;
	}

	FellowshipInfo after = *info;
	after.campsite.reset();
	ReplaceFellowship(std::move(after));

	LogFellowships("Fellowship [{}] campsite removed", fellowship_id);

	const auto *updated = GetFellowship(fellowship_id);
	SendRefresh(*updated, false, 0, before_ids);
	SendRosterToOnlineMembers(*updated);
	SendCampsite(*updated, campsite, true);

	return true;
}

bool WorldFellowshipManager::Disband(uint32 fellowship_id)
{
	const auto *cached = GetFellowship(fellowship_id);
	if (!cached) {
		return false;
	}

	const FellowshipInfo before          = *cached;
	const std::vector<uint32> before_ids = GetMemberIDs(before);

	const bool written = RunTransaction(
		[&]() {
			FellowshipCampsitesRepository::DeleteOne(*m_db, fellowship_id);

			if (FellowshipMembersRepository::DeleteWhere(*m_db, fmt::format("`fellowship_id` = {}", fellowship_id)) < 1) {
				return false;
			}

			return FellowshipsRepository::DeleteOne(*m_db, fellowship_id) == 1;
		},
		fmt::format("disband of fellowship [{}]", fellowship_id)
	);
	if (!written) {
		return false;
	}

	EraseFellowship(fellowship_id);
	for (const auto character_id : before_ids) {
		m_last_sent.erase(character_id);
	}

	LogFellowships("Fellowship [{}] disbanded", fellowship_id);

	SendRefresh(before, true, 0, before_ids);

	// a roster without members makes each client leave
	FellowshipInfo empty{};
	empty.id = fellowship_id;
	SendUpdate(before_ids, fellowship_id, BuildRoster(empty), FELLOWSHIP_DISBANDED, Chat::System, "");

	if (before.campsite) {
		SendCampsite(before, *before.campsite, true);
	}

	return true;
}
