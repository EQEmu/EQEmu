#include "zone/fellowship_manager.h"

#include "common/bodytypes.h"
#include "common/classes.h"
#include "common/data_bucket.h"
#include "common/eq_packet.h"
#include "common/eqemu_logsys.h"
#include "common/fellowship.h"
#include "common/races.h"
#include "common/servertalk.h"
#include "common/strings.h"
#include "zone/client.h"
#include "zone/entity.h"
#include "zone/npc.h"
#include "zone/worldserver.h"
#include "zone/zone.h"
#include "zone/zonedb.h"

#include <algorithm>
#include <cstring>
#include <ctime>
#include <iterator>

extern WorldServer worldserver;

namespace {
	constexpr char CAMPFIRE_NAME[] = "Fellowship_Campfire";

	constexpr int32 CAMPFIRE_HP    = 4000000;
	constexpr uint8 CAMPFIRE_LEVEL = 200;
	constexpr uint8 CAMPFIRE_DEITY = 1;
	constexpr float CAMPFIRE_SIZE  = 5.0f;

	// the Live campfire is a plain, untouchable NPC; same immunities as the zone point node NPCs
	constexpr char CAMPFIRE_SPECIAL_ABILITIES[] = "12,1^13,1^14,1^15,1^16,1^17,1^19,1^22,1^24,1^25,1^28,1^31,1^35,1^39,1^42,1";
}

ZoneFellowshipManager *ZoneFellowshipManager::Instance()
{
	// never destroyed: ~Client runs when the global entity list is torn down, after function statics are gone
	static auto *instance = new ZoneFellowshipManager;

	return instance;
}

void ZoneFellowshipManager::ProcessWorldPacket(ServerPacket *pack)
{
	if (!pack) {
		return;
	}

	switch (pack->opcode) {
		case ServerOP_FellowshipRefresh: {
			if (pack->size != sizeof(ServerFellowshipRefresh_Struct)) {
				LogError(
					"ServerOP_FellowshipRefresh size mismatch: got [{}], expected [{}]",
					pack->size,
					sizeof(ServerFellowshipRefresh_Struct)
				);
				return;
			}

			ProcessRefresh(*reinterpret_cast<const ServerFellowshipRefresh_Struct *>(pack->pBuffer));
			break;
		}
		case ServerOP_FellowshipUpdate: {
			if (pack->size != sizeof(ServerFellowshipUpdate_Struct)) {
				LogError(
					"ServerOP_FellowshipUpdate size mismatch: got [{}], expected [{}]",
					pack->size,
					sizeof(ServerFellowshipUpdate_Struct)
				);
				return;
			}

			ProcessUpdate(*reinterpret_cast<const ServerFellowshipUpdate_Struct *>(pack->pBuffer));
			break;
		}
		case ServerOP_FellowshipCampsite: {
			if (pack->size != sizeof(ServerFellowshipCampsite_Struct)) {
				LogError(
					"ServerOP_FellowshipCampsite size mismatch: got [{}], expected [{}]",
					pack->size,
					sizeof(ServerFellowshipCampsite_Struct)
				);
				return;
			}

			ProcessCampsite(*reinterpret_cast<const ServerFellowshipCampsite_Struct *>(pack->pBuffer));
			break;
		}
		default: {
			break;
		}
	}
}

void ZoneFellowshipManager::ProcessRefresh(const ServerFellowshipRefresh_Struct &in)
{
	ApplyRefresh(in);

	// the packet's member list is the pre-change one when the fellowship was deleted
	const uint32 current_id  = in.deleted ? 0 : in.fellowship_id;
	const uint32 member_count = std::min<uint32>(in.member_count, std::size(in.members));
	for (uint32 i = 0; i < member_count; i++) {
		auto *client = entity_list.GetClientByCharID(in.members[i].character_id);
		if (client) {
			client->SetFellowshipID(current_id);
		}
	}

	const uint32 removed_count = std::min<uint32>(in.removed_count, std::size(in.removed_character_ids));
	for (uint32 i = 0; i < removed_count; i++) {
		auto *client = entity_list.GetClientByCharID(in.removed_character_ids[i]);
		if (client) {
			client->SetFellowshipID(0);
		}
	}

	// world just wrote the rejoin cooldown bucket; this zone may hold a cached miss for it
	if (in.joined_character_id && entity_list.GetClientByCharID(in.joined_character_id)) {
		DataBucket::DeleteCachedBuckets(DataBucketLoadType::Client, in.joined_character_id);
	}

	if (in.deleted) {
		for (auto it = m_invites.begin(); it != m_invites.end();) {
			if (it->second.fellowship_id == in.fellowship_id) {
				it = m_invites.erase(it);
			} else {
				++it;
			}
		}
	}

	LogFellowshipsDetail(
		"Refresh for fellowship [{}] deleted [{}] members [{}] removed [{}]",
		in.fellowship_id,
		in.deleted,
		member_count,
		removed_count
	);
}

void ZoneFellowshipManager::ProcessUpdate(const ServerFellowshipUpdate_Struct &in)
{
	const uint32 recipient_count = std::min<uint32>(in.recipient_count, std::size(in.recipients));
	const std::string argument(in.string_argument, strnlen(in.string_argument, sizeof(in.string_argument)));

	for (uint32 i = 0; i < recipient_count; i++) {
		auto *client = entity_list.GetClientByCharID(in.recipients[i]);
		if (!client || !client->IsFellowshipEnabled()) {
			continue;
		}

		if (in.string_id) {
			if (argument.empty()) {
				client->MessageString(in.string_type, in.string_id);
			} else {
				client->MessageString(in.string_type, in.string_id, argument.c_str());
			}
		}

		if (!in.fellowship_id) {
			continue;
		}

		auto outapp = new EQApplicationPacket(OP_FellowshipUpdate, sizeof(FellowshipUpdate_Struct));
		auto out    = reinterpret_cast<FellowshipUpdate_Struct *>(outapp->pBuffer);
		memcpy(out, &in.roster, sizeof(FellowshipUpdate_Struct));

		// world builds these, but they go to a client: never trust a count or a string terminator
		out->fellowship_id = in.fellowship_id;
		out->member_count  = std::min<uint32>(out->member_count, std::size(out->members));
		out->motd[sizeof(out->motd) - 1] = '\0';
		for (auto &m : out->members) {
			m.name[sizeof(m.name) - 1] = '\0';
		}

		client->FastQueuePacket(&outapp);
	}

	LogFellowshipsDetail(
		"Update for fellowship [{}] string [{}] to [{}] recipients",
		in.fellowship_id,
		in.string_id,
		recipient_count
	);
}

void ZoneFellowshipManager::ProcessCampsite(const ServerFellowshipCampsite_Struct &in)
{
	if (in.removed) {
		DespawnCampfire(in.fellowship_id);
		return;
	}

	if (!zone || in.campsite.zone_id != zone->GetZoneID() || in.campsite.instance_id != zone->GetInstanceID()) {
		LogFellowshipsDetail(
			"Ignoring campsite of fellowship [{}] for zone [{}] instance [{}]: not this zone",
			in.fellowship_id,
			in.campsite.zone_id,
			in.campsite.instance_id
		);
		return;
	}

	FellowshipCampsite campsite{};
	campsite.zone_id       = in.campsite.zone_id;
	campsite.instance_id   = in.campsite.instance_id;
	campsite.x             = in.campsite.x;
	campsite.y             = in.campsite.y;
	campsite.z             = in.campsite.z;
	campsite.heading       = in.campsite_heading;
	campsite.campfire_type = in.campsite.campfire_type;
	campsite.expires_at    = in.campsite.expires_at;

	const std::string leader_name(in.leader_name, strnlen(in.leader_name, sizeof(in.leader_name)));
	SpawnCampfire(in.fellowship_id, campsite, leader_name);
}

void ZoneFellowshipManager::SendAction(const ServerFellowshipAction_Struct &action)
{
	ServerPacket pack(ServerOP_FellowshipAction, sizeof(ServerFellowshipAction_Struct));
	memcpy(pack.pBuffer, &action, sizeof(ServerFellowshipAction_Struct));
	worldserver.SendPacket(&pack);
}

void ZoneFellowshipManager::SendCampsiteExpired(uint32 fellowship_id)
{
	ServerFellowshipAction_Struct action{};
	action.action        = FellowshipCampsiteExpired;
	action.fellowship_id = fellowship_id;

	SendAction(action);
}

void ZoneFellowshipManager::RecordInvite(uint32 target_character_id, uint32 fellowship_id)
{
	m_invites.insert_or_assign(
		target_character_id,
		PendingFellowshipInvite{fellowship_id, Timer(Fellowship::INVITE_TIMEOUT_MS)}
	);
}

void ZoneFellowshipManager::ClearInvite(uint32 target_character_id)
{
	m_invites.erase(target_character_id);
}

uint32 ZoneFellowshipManager::TakeInvite(uint32 target_character_id)
{
	auto it = m_invites.find(target_character_id);
	if (it == m_invites.end()) {
		LogFellowshipsDetail("Character [{}] answered an invite that does not exist", target_character_id);
		return 0;
	}

	const uint32 fellowship_id = it->second.fellowship_id;
	const bool   timed_out     = it->second.timer.Check(false);
	m_invites.erase(it);

	if (timed_out) {
		LogFellowshipsDetail("Character [{}] answered a timed out invite to fellowship [{}]", target_character_id, fellowship_id);
		return 0;
	}

	return fellowship_id;
}

void ZoneFellowshipManager::SpawnCampsitesForThisZone()
{
	// a new zone has a new entity list: nothing from before is still there
	m_campfires.clear();

	if (!zone) {
		return;
	}

	const uint32 now = static_cast<uint32>(std::time(nullptr));
	for (const auto &[fellowship_id, fellowship] : m_fellowships) {
		if (!fellowship.campsite) {
			continue;
		}

		const auto &campsite = *fellowship.campsite;
		if (campsite.zone_id != zone->GetZoneID() || campsite.instance_id != zone->GetInstanceID()) {
			continue;
		}

		if (campsite.expires_at <= now) {
			LogFellowshipsDetail("Not spawning the campfire of fellowship [{}]: it has expired", fellowship_id);
			continue;
		}

		// the one place an instance that is gone is noticed at boot, world removes the row
		if (campsite.instance_id && database.CheckInstanceExpired(campsite.instance_id)) {
			LogFellowshipsDetail(
				"Not spawning the campfire of fellowship [{}]: instance [{}] has expired",
				fellowship_id,
				campsite.instance_id
			);
			SendCampsiteExpired(fellowship_id);
			continue;
		}

		const auto *leader = fellowship.FindMember(fellowship.leader_id);
		SpawnCampfire(fellowship_id, campsite, leader ? leader->name : std::string());
	}
}

void ZoneFellowshipManager::SpawnCampfire(
	uint32 fellowship_id,
	const FellowshipCampsite &campsite,
	const std::string &leader_name
)
{
	DespawnCampfire(fellowship_id);

	auto npc_type = new NPCType;
	memset(npc_type, 0, sizeof(NPCType));

	// no unique suffix: the Find window and spawn lookups go by entity id
	strn0cpy(npc_type->name, CAMPFIRE_NAME, sizeof(npc_type->name));

	const std::string last_name = fmt::format("{}'s Fellowship", leader_name.empty() ? "Your" : leader_name);
	strn0cpy(npc_type->lastname, last_name.c_str(), sizeof(npc_type->lastname));

	npc_type->current_hp   = CAMPFIRE_HP;
	npc_type->max_hp       = CAMPFIRE_HP;
	npc_type->race         = Race::Campfire;
	npc_type->gender       = Gender::Neuter;
	npc_type->class_       = Class::Rogue;
	npc_type->deity        = CAMPFIRE_DEITY;
	npc_type->level        = CAMPFIRE_LEVEL;
	npc_type->npc_id       = 0;
	npc_type->loottable_id = 0;
	npc_type->texture      = 1;
	npc_type->light        = 1;
	npc_type->size         = CAMPFIRE_SIZE;
	npc_type->runspeed     = 0;
	npc_type->merchanttype = 0;
	npc_type->bodytype     = BodyType::Humanoid;
	npc_type->show_name    = true;
	npc_type->findable     = true; // the Find window lists it

	strn0cpy(npc_type->special_abilities, CAMPFIRE_SPECIAL_ABILITIES, sizeof(npc_type->special_abilities));

	const glm::vec4 position(campsite.x, campsite.y, campsite.z, campsite.heading);
	auto npc = new NPC(npc_type, nullptr, position, GravityBehavior::Flying);

	npc->GiveNPCTypeData(npc_type);

	entity_list.AddNPC(npc);

	m_campfires[fellowship_id] = Campfire{npc->GetID(), leader_name};

	LogFellowships(
		"Spawned the campfire of fellowship [{}] at [{:.1f}, {:.1f}, {:.1f}] entity [{}]",
		fellowship_id,
		campsite.x,
		campsite.y,
		campsite.z,
		npc->GetID()
	);
}

void ZoneFellowshipManager::DespawnCampfire(uint32 fellowship_id)
{
	auto it = m_campfires.find(fellowship_id);
	if (it == m_campfires.end()) {
		return;
	}

	const uint16 entity_id = it->second.entity_id;
	m_campfires.erase(it);

	// the entity id may have been reused if the NPC went away by other means
	auto *npc = entity_list.GetNPCByID(entity_id);
	if (!npc || npc->GetRace() != Race::Campfire) {
		return;
	}

	// Depop alone leaves the Find window entry behind
	entity_list.UpdateFindableNPCState(npc, true);
	npc->Depop();

	LogFellowships("Despawned the campfire of fellowship [{}]", fellowship_id);
}

void ZoneFellowshipManager::RespawnCampfires()
{
	const auto campfires = m_campfires;
	const uint32 now     = static_cast<uint32>(std::time(nullptr));

	for (const auto &[fellowship_id, campfire] : campfires) {
		const auto *fellowship = GetFellowship(fellowship_id);
		if (!fellowship || !fellowship->campsite || fellowship->campsite->expires_at <= now) {
			m_campfires.erase(fellowship_id);
			continue;
		}

		SpawnCampfire(fellowship_id, *fellowship->campsite, campfire.leader_name);
	}
}
