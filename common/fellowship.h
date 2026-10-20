#pragma once

#include "common/types.h"

// TOB+ fellowships: values fixed by the client
namespace Fellowship {
	constexpr char   REJOIN_BUCKET[]    = "fellowship_rejoin";
	constexpr uint32 MAX_MEMBERS        = 12;
	constexpr uint32 INVITE_TIMEOUT_MS  = 60000;
	constexpr uint32 LIST_RATE_LIMIT_MS = 2000;
	constexpr float  CAMPSITE_RANGE     = 100.0f; // guess: how close members must be to the member placing the campsite

	struct CampfireType {
		uint32     id;
		uint32     description_id; // dbstr
		const char *name;
	};

	// the only campfire without a kit; ids and name match the client's dbstr_us.txt
	constexpr CampfireType CAMPFIRE_TYPES[] = {
		{41200, 41200, "Fellowship of Honor"},
	};

	inline const CampfireType *FindCampfireType(uint32 id)
	{
		for (const auto &e : CAMPFIRE_TYPES) {
			if (e.id == id) {
				return &e;
			}
		}

		return nullptr;
	}
}

// OP_FellowshipAction, as the client numbers them
enum FellowshipAction : uint32 {
	FellowshipCreate          = 1,
	FellowshipSetMotd         = 2,
	FellowshipAcceptInvite    = 3,
	FellowshipDeclineInvite   = 4,
	FellowshipInvite          = 5,
	FellowshipRemoveMember    = 6,
	FellowshipPromoteLeader   = 7,
	FellowshipCreateCampsite  = 8,
	FellowshipDestroyCampsite = 9,
	FellowshipLeave           = 10,
	FellowshipDisband         = 11,
	FellowshipToggleExpShare  = 12
};

// zone -> world actions no client sends
enum FellowshipServerAction : uint32 {
	FellowshipCampsiteExpired = 100 // the campsite's instance is gone
};
