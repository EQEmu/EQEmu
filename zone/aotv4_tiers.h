#ifndef AOTV4_TIERS_H
#define AOTV4_TIERS_H

#include <cstdint>

// AoTv4 gear tiers -- id arithmetic, in ONE place.
//
// Hallowed = base id + 300,000 and Mythic = base id + 600,000, so the reserved band is
// [300000, 900000) and a tier item reduces to its base with id % 300000. Base ids top out at
// 147,494, which is why the step is 300,000 and not something rounder: RoF2 item LINKS encode the id
// in a 5-hex-digit field masked to 0xFFFFF (1,048,575), and the old +1,000,000/+2,000,000 scheme
// pushed Mythic past that ceiling so every Mythic chat link rendered as garbage.
//
// ⚠️ THE STEP IS ALSO HARDCODED IN FIVE OLDER PLACES -- loot.cpp, npc.cpp, questmgr.cpp, attack.cpp
// and tradeskills.cpp (AOTV4_TIER_STEP). Those predate this header and were deliberately NOT churned
// when it was added, because they work and rewriting them buys nothing but risk. New code should use
// this; if one of those five is ever touched for another reason, migrate it then.

constexpr uint32_t AOTV4_TIER_STEP  = 300000;
constexpr uint32_t AOTV4_TIER_FIRST = AOTV4_TIER_STEP;       // lowest Hallowed id
constexpr uint32_t AOTV4_TIER_END   = 3 * AOTV4_TIER_STEP;   // one past the highest Mythic id

// A tier item's base id. Native ids and anything outside the reserved band come back unchanged.
inline uint32_t AoTv4TierBaseId(uint32_t id)
{
	return (id >= AOTV4_TIER_FIRST && id < AOTV4_TIER_END) ? (id % AOTV4_TIER_STEP) : id;
}

// Is this id inside the reserved tier band at all?
inline bool AoTv4IsTierId(uint32_t id)
{
	return id >= AOTV4_TIER_FIRST && id < AOTV4_TIER_END;
}

// ---------------------------------------------------------------------------------------------
// AoTv4 DELVE AUGMENTS -- three tiers in three contiguous id blocks.
//
// The delve chest drops one (aotv4_dungeon.lua); FOUR of a tier combine in the Refining Crucible
// into ONE RANDOM augment of the next tier (zone/tradeskills.cpp, AoTv4RefineCombine). They are
// ordinary items, NOT native evolving items -- see the header of gen_delve_augs.pl for why.
//
// ⚠️⚠️ THESE BOUNDARIES MIRROR THE GENERATOR AND NOTHING ENFORCES THAT. They are produced by
// .devcontainer/custom/tools/gen_delve_augs.pl (16 tier-1 variants, then 40 each for tiers 2 and 3,
// laid out contiguously from 147600). Change the variant counts there and these must change too:
// nothing will fail to compile, the crucible will simply start reading the wrong tier and handing
// out the wrong items. The generated SQL prints the block boundaries in its header -- check them
// against this after any regen.
constexpr uint32_t AOTV4_AUG_T1_FIRST = 147600;
constexpr uint32_t AOTV4_AUG_T1_LAST  = 147615;   // 16 single-stat variants, one per stat line
constexpr uint32_t AOTV4_AUG_T2_FIRST = 147616;
constexpr uint32_t AOTV4_AUG_T2_LAST  = 147715;   // 100 rolled variants, all stat-distinct
constexpr uint32_t AOTV4_AUG_T3_FIRST = 147716;
constexpr uint32_t AOTV4_AUG_T3_LAST  = 147915;   // 200 rolled variants, all stat-distinct, top tier

// 1, 2 or 3 for a delve augment; 0 for anything else.
inline int AoTv4AugTier(uint32_t id)
{
	if (id >= AOTV4_AUG_T1_FIRST && id <= AOTV4_AUG_T1_LAST) { return 1; }
	if (id >= AOTV4_AUG_T2_FIRST && id <= AOTV4_AUG_T2_LAST) { return 2; }
	if (id >= AOTV4_AUG_T3_FIRST && id <= AOTV4_AUG_T3_LAST) { return 3; }
	return 0;
}

// The id block a tier's variants occupy. Returns false for a tier with no block (0, or above 3).
inline bool AoTv4AugTierBlock(int tier, uint32_t &first, uint32_t &last)
{
	switch (tier) {
		case 1: first = AOTV4_AUG_T1_FIRST; last = AOTV4_AUG_T1_LAST; return true;
		case 2: first = AOTV4_AUG_T2_FIRST; last = AOTV4_AUG_T2_LAST; return true;
		case 3: first = AOTV4_AUG_T3_FIRST; last = AOTV4_AUG_T3_LAST; return true;
		default: return false;
	}
}

#endif // AOTV4_TIERS_H
