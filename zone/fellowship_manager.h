#pragma once

#include "common/fellowship_base.h"
#include "common/timer.h"
#include "common/types.h"

#include <map>
#include <string>
#include <unordered_map>

class ServerPacket;
struct ServerFellowshipAction_Struct;
struct ServerFellowshipCampsite_Struct;
struct ServerFellowshipRefresh_Struct;
struct ServerFellowshipUpdate_Struct;

struct PendingFellowshipInvite {
	uint32 fellowship_id;
	Timer  timer;      // Fellowship::INVITE_TIMEOUT_MS
};

// TOB+ fellowships in the zone: a copy of world's state, the pending invites and the campfires
class ZoneFellowshipManager : public BaseFellowshipManager {
public:
	static ZoneFellowshipManager *Instance();

	// Refresh, Update and Campsite from world
	void ProcessWorldPacket(ServerPacket *pack);

	// ServerOP_FellowshipAction to world
	void SendAction(const ServerFellowshipAction_Struct &action);
	void SendCampsiteExpired(uint32 fellowship_id);

	// pending invites, keyed by the invited character; a newer invite replaces an older one
	void RecordInvite(uint32 target_character_id, uint32 fellowship_id);
	void ClearInvite(uint32 target_character_id);
	// erases the invite and returns its fellowship id, 0 when there is none or it timed out
	uint32 TakeInvite(uint32 target_character_id);

	// campfire NPCs: one per fellowship with a campsite in this zone
	void SpawnCampsitesForThisZone();
	// a repop depops every NPC, the campfires included
	void RespawnCampfires();

private:
	ZoneFellowshipManager() = default;

	void ProcessRefresh(const ServerFellowshipRefresh_Struct &in);
	void ProcessUpdate(const ServerFellowshipUpdate_Struct &in);
	void ProcessCampsite(const ServerFellowshipCampsite_Struct &in);

	void SpawnCampfire(uint32 fellowship_id, const FellowshipCampsite &campsite, const std::string &leader_name);
	void DespawnCampfire(uint32 fellowship_id);

	std::map<uint32, PendingFellowshipInvite> m_invites;          // target character id -> invite
	struct Campfire {
		uint16      entity_id;
		std::string leader_name;
	};

	std::unordered_map<uint32, Campfire>      m_campfires; // fellowship id -> campfire NPC, this zone only
};
