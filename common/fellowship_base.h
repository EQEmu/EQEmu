#pragma once

#include "common/eq_packet_structs.h"
#include "common/types.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class Database;
struct ServerFellowshipRefresh_Struct;

struct FellowshipMember {
	uint32      character_id;
	uint32      joined_at;
	bool        is_exp_sharing;
	// shown for offline members; the client list wins while the member is online
	std::string name;
	uint32      level;
	uint32      class_;
	uint32      last_login;
};

struct FellowshipCampsite {
	uint16 zone_id;
	uint16 instance_id;
	float  x;
	float  y;
	float  z;
	float  heading;
	uint32 campfire_type;
	uint32 placed_at;
	uint32 expires_at;
};

struct FellowshipInfo {
	uint32                            id;
	uint32                            leader_id;
	std::string                       motd;
	uint32                            created_at;
	std::vector<FellowshipMember>     members; // join order
	std::optional<FellowshipCampsite> campsite;

	const FellowshipMember *FindMember(uint32 character_id) const;
};

// TOB+ fellowships, cached in world and every zone; only world writes
class BaseFellowshipManager {
public:
	virtual ~BaseFellowshipManager() = default;

	BaseFellowshipManager *SetDatabase(Database *db);

	// read-only: rows that don't add up are skipped and logged, world repairs them
	bool LoadFellowships();

	// replaces (or erases) one cached fellowship with world's copy
	void ApplyRefresh(const ServerFellowshipRefresh_Struct &in);

	const FellowshipInfo *GetFellowship(uint32 fellowship_id) const;
	uint32 GetFellowshipIDByCharacterID(uint32 character_id) const;
	bool IsLeader(uint32 fellowship_id, uint32 character_id) const;

	static FellowshipCampsite_Struct CampsiteToStruct(const std::optional<FellowshipCampsite> &campsite);

protected:
	void CacheFellowship(FellowshipInfo &&info);
	void EraseFellowship(uint32 fellowship_id);

	std::unordered_map<uint32, FellowshipInfo> m_fellowships;
	std::unordered_map<uint32, uint32>         m_fellowship_by_character; // character_id -> fellowship_id
	Database                                   *m_db{};
};
