#pragma once

#include "common/fellowship_base.h"
#include "common/timer.h"
#include "common/types.h"

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

class ClientListEntry;
class ServerPacket;
class ZoneServer;
struct ServerFellowshipAction_Struct;

// the single writer of fellowships: validates zone requests, writes in transactions, tells the zones
class WorldFellowshipManager : public BaseFellowshipManager {
public:
	static WorldFellowshipManager *Instance();

	void RepairAfterLoad();
	void ProcessZonePacket(ServerPacket *pack);
	void Process();

	void OnClientListChange(uint32 character_id);
	void OnZoneBootup(ZoneServer *zone_server);
	void RemoveCharacter(uint32 character_id);
	void SendChannelMessage(ServerPacket *pack, uint32 fellowship_id);

private:
	struct RosterSnapshot {
		uint32 zone_id;
		uint16 instance_id;
		uint8  level;
		uint8  class_;
		bool   is_online;
	};

	WorldFellowshipManager();

	static bool IsSameSnapshot(const RosterSnapshot &a, const RosterSnapshot &b);
	static std::vector<uint32> GetMemberIDs(const FellowshipInfo &info);

	ClientListEntry *FindOnlineEntry(uint32 character_id) const;
	RosterSnapshot GetSnapshot(uint32 character_id) const;
	std::string GetMemberName(const FellowshipMember &member) const;
	FellowshipUpdate_Struct BuildRoster(const FellowshipInfo &info) const;
	std::map<ZoneServer *, std::vector<uint32>> GroupByZoneServer(const std::vector<uint32> &character_ids) const;

	std::unique_ptr<ServerPacket> BuildRefreshPacket(
		const FellowshipInfo &state,
		bool deleted,
		uint32 joined_character_id,
		const std::vector<uint32> &before_ids
	) const;
	void SendRefresh(
		const FellowshipInfo &state,
		bool deleted,
		uint32 joined_character_id,
		const std::vector<uint32> &before_ids
	);
	void SendUpdate(
		const std::vector<uint32> &recipients,
		uint32 fellowship_id,
		const FellowshipUpdate_Struct &roster,
		uint32 string_id,
		uint32 string_type,
		const std::string &string_argument
	);
	void SendRosterToOnlineMembers(const FellowshipInfo &info);
	void SendCampsite(const FellowshipInfo &info, const FellowshipCampsite &campsite, bool removed);
	void RecordSnapshots(const FellowshipInfo &info);

	void ReplaceFellowship(FellowshipInfo &&info);
	bool RunTransaction(const std::function<bool()> &write, const std::string &description);
	void WriteRejoinBucket(uint32 character_id);
	bool RefreshOfflineMember(uint32 character_id);
	void AnnounceRepairedFellowship(uint32 fellowship_id, uint32 character_id);

	uint32 GetActorFellowshipID(const ServerFellowshipAction_Struct &in, bool must_be_leader, const char *action_name) const;

	bool CreateFellowship(uint32 character_id);
	bool AddMember(uint32 character_id, uint32 fellowship_id);
	bool RemoveMember(uint32 fellowship_id, uint32 character_id);
	bool SetLeader(uint32 fellowship_id, uint32 character_id);
	bool SetMotd(uint32 fellowship_id, const std::string &motd);
	bool SetExpSharing(uint32 fellowship_id, uint32 character_id);
	bool PlaceCampsite(uint32 fellowship_id, const ServerFellowshipAction_Struct &in);
	bool RemoveCampsite(uint32 fellowship_id);
	bool Disband(uint32 fellowship_id);

	Timer                                      m_process_timer;
	std::unordered_map<uint32, RosterSnapshot> m_last_sent;   // character_id -> what the members last saw
	std::map<uint32, std::set<uint32>>         m_unannounced; // repaired fellowship_id -> zone server ids told
};
