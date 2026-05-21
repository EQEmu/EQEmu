// Theo-and-Co Phase 3 / S38 — meditate-curve OOC fast-regen helper.
//
// Out-of-combat HP/mana/endurance regen speed scales with the player's
// Meditate skill, per a curve Alex locked S38:
//
//   - Pure caster classes (CLR/DRU/SHM/NEC/WIZ/MAG/ENC; PoP-era cap 350):
//     20s 0->100% at meditate 1, linear down to 10s at meditate 350.
//   - Hybrid classes (PAL/RNG/SK/BST; PoP-era cap 325):
//     20s at meditate 1, linear down to 10s at meditate 325.
//   - Melee + Bard (WAR/MNK/ROG/BER/BRD; skill never trainable beyond 1):
//     flat 15s regardless of skill value.
//
// The dial set via the Chromie NPC (data_bucket "regen_mult") then scales
// the per-second pool fraction on top: per_sec_frac = dial / sec_to_full.
//
// Bots inherit the OWNER's class + meditate skill + dial (cached refresh
// every 6s tic via Bot::RefreshOwnerRegenMult, identical pattern to
// Bot::RefreshOwnerDrillMults). The owner's class is used in the curve --
// a Warrior owner means his Wizard bot regens at the melee-middle 15s, and
// a Cleric owner means his Warrior bot regens at the Cleric's meditate
// curve. "They are identical" -- Alex S38.
//
// Class cap values verified S38 against live peq.skill_caps (skill_id=31
// Meditate, level 65). See memory/session_log.md S38.

#pragma once

#include "../common/classes.h"

#include <cstdint>

namespace TheoRegen {

// Returns the OOC fast-regen "seconds to full pool" for the given class and
// meditate skill value. Always in [10.0, 20.0]. Melee+Bard always 15.0.
//
// Callers multiply 1.0 / result by the Chromie dial and by the entity's
// max pool to get per-second regen amount.
inline float ComputeOOCSecondsToFull(uint8 class_id, uint16 meditate_skill) {
	int cap = 0;
	switch (class_id) {
	case Class::Cleric:
	case Class::Druid:
	case Class::Shaman:
	case Class::Necromancer:
	case Class::Wizard:
	case Class::Magician:
	case Class::Enchanter:
		cap = 350;
		break;
	case Class::Paladin:
	case Class::Ranger:
	case Class::ShadowKnight:
	case Class::Beastlord:
		cap = 325;
		break;
	default:
		// Warrior, Monk, Rogue, Berserker, Bard: meditate not trainable
		// past 1 on this engine. Flat 15s (the middle of the 10-20s
		// curve, Alex-locked S38).
		return 15.0f;
	}

	int skill = static_cast<int>(meditate_skill);
	if (skill <= 1) {
		return 20.0f;
	}
	if (skill >= cap) {
		return 10.0f;
	}

	// Linear: at skill 1 -> 20s; at skill cap -> 10s.
	float frac = static_cast<float>(skill - 1) / static_cast<float>(cap - 1);
	return 20.0f - (10.0f * frac);
}

// Default dial when the data_bucket "regen_mult" is unset / empty.
// Alex S38: default 1.0 (today-ish OOC regen feel, ~20s at low meditate,
// ~10s at high meditate). Players opt UP (faster, easier) or DOWN
// (slower, harder) via the Chromie NPC.
constexpr float kDefaultRegenDial = 1.0f;

// Minimum / maximum dial values offered by the Chromie NPC. Used for
// defensive clamping on the engine side in case a bucket holds an
// out-of-band value.
constexpr float kMinRegenDial = 0.10f;
constexpr float kMaxRegenDial = 2.00f;

} // namespace TheoRegen
