#ifndef BOT_STAT_MODEL_H
#define BOT_STAT_MODEL_H

// =========================================================================
// Theo-and-Co Phase 3 Group A — Bot stat model
// =========================================================================
// Equipped gear is cosmetic-only for bots; combat stats come from this
// level x class x role formula instead. This header holds the *data*
// (creation-base table now; Defiant tier anchors, role multipliers and the
// AA-compensation curve are added in later Group-A steps). The behavior
// (the curve + recalc-layer override) is wired in bot.cpp separately.
//
// Design source of truth: docs/phase3_group_a_proposal.md (theo-and-co repo).
//
// -------------------------------------------------------------------------
// CREATION-BASE TABLE
// -------------------------------------------------------------------------
// The "creation base" formula term = what a normally-created character of a
// given class+race has at level 1 (the pre-gear floor + the default point
// allocation the create screen auto-fills). Used directly for L1-19 (no gear
// stats below 20) and as the additive base under the L20+ Defiant anchors.
//
// Authoritative source (verified 2026-05-16):
//   engine world/client.cpp:1900-1994 (CheckCharCreateInfo* validates against
//   RaceClassAllocation.BaseStats[] + the allocatable pool), data from the
//   live DB tables char_create_combinations + char_create_point_allocations.
//
// Regenerate (run against the peq DB) — keep this in sync if char_create_* changes:
//   SELECT cc.class, cc.race,
//     pa.base_str+pa.alloc_str, pa.base_sta+pa.alloc_sta,
//     pa.base_agi+pa.alloc_agi, pa.base_dex+pa.alloc_dex,
//     pa.base_int+pa.alloc_int, pa.base_wis+pa.alloc_wis,
//     pa.base_cha+pa.alloc_cha
//   FROM (SELECT DISTINCT class, race, allocation_id FROM char_create_combinations) cc
//   JOIN char_create_point_allocations pa ON pa.id = cc.allocation_id
//   ORDER BY cc.class, cc.race;
//
// class: 1 WAR 2 CLR 3 PAL 4 RNG 5 SHD 6 DRU 7 MNK 8 BRD 9 ROG 10 SHM
//        11 NEC 12 WIZ 13 MAG 14 ENC 15 BST 16 BER
// race : 1 Human 2 Barbarian 3 Erudite 4 WoodElf 5 HighElf 6 DarkElf
//        7 HalfElf 8 Dwarf 9 Troll 10 Ogre 11 Halfling 12 Gnome
//        128 Iksar 130 VahShir 330 Froglok 522 Drakkin
// =========================================================================

#include <cstdint>

struct BotCreationBaseStats {
	uint8_t  class_;
	uint16_t race;
	uint16_t str;
	uint16_t sta;
	uint16_t agi;
	uint16_t dex;
	uint16_t intel;
	uint16_t wis;
	uint16_t cha;
};

// Exactly the 112 valid (class,race) combinations. Bots can only be valid
// creatable race/class combos, so this table is complete for any bot.
static const BotCreationBaseStats kBotCreationBaseStats[] = {
	// class, race,  STR  STA  AGI  DEX  INT  WIS  CHA
	{  1,   1,   85, 110,  80,  75,  75,  75,  75 },
	{  1,   2,  113, 130,  87,  70,  60,  70,  55 },
	{  1,   4,   75, 100, 100,  80,  75,  80,  75 },
	{  1,   6,   70, 100,  95,  75,  99,  83,  60 },
	{  1,   7,   80, 105,  95,  85,  75,  60,  75 },
	{  1,   8,  100, 125,  75,  90,  60,  83,  45 },
	{  1,   9,  118, 144,  88,  75,  52,  60,  40 },
	{  1,  10,  147, 150,  75,  70,  60,  67,  37 },
	{  1,  11,   80, 110, 100,  90,  67,  80,  50 },
	{  1,  12,   70, 105,  90,  85,  98,  67,  60 },
	{  1, 128,   80, 105,  95,  85,  75,  80,  55 },
	{  1, 130,  100, 110,  95,  70,  65,  70,  65 },
	{  1, 330,   80, 115, 105, 100,  75,  75,  50 },
	{  1, 522,   80, 115,  90,  75,  85,  80,  75 },
	{  2,   1,   80,  85,  75,  75,  75, 110,  75 },
	{  2,   3,   65,  80,  70,  70, 107, 118,  70 },
	{  2,   5,   60,  75,  85,  70,  92, 130,  80 },
	{  2,   6,   65,  75,  90,  75,  99, 118,  60 },
	{  2,   8,   95, 100,  70,  90,  60, 118,  45 },
	{  2,  11,   75,  85,  95,  90,  67, 115,  50 },
	{  2,  12,   65,  80,  85,  85,  98, 102,  60 },
	{  2, 330,   75,  90, 100, 100,  75, 110,  50 },
	{  2, 522,   75,  90,  85,  75,  85, 115,  75 },
	{  3,   1,   85, 100,  75,  75,  75,  80,  85 },
	{  3,   3,   70,  95,  70,  70, 107,  88,  80 },
	{  3,   5,   65,  90,  85,  70,  92, 100,  90 },
	{  3,   7,   80,  95,  90,  85,  75,  65,  85 },
	{  3,   8,  100, 115,  70,  90,  60,  88,  55 },
	{  3,  11,   80, 100,  95,  90,  67,  85,  60 },
	{  3,  12,   70,  95,  85,  85,  98,  72,  70 },
	{  3, 330,   80, 105, 100, 100,  75,  80,  60 },
	{  3, 522,   80, 105,  85,  75,  85,  85,  85 },
	{  4,   1,   80,  85,  85,  95,  75,  80,  75 },
	{  4,   4,   70,  75, 105, 100,  75,  85,  75 },
	{  4,   7,   75,  80, 100, 105,  75,  65,  75 },
	{  4,  11,   75,  85, 105, 110,  67,  85,  50 },
	{  4, 522,   75,  90,  95,  95,  85,  85,  75 },
	{  5,   1,   85, 100,  75,  75,  85,  75,  80 },
	{  5,   3,   70,  95,  70,  70, 117,  83,  75 },
	{  5,   6,   70,  90,  90,  75, 109,  83,  65 },
	{  5,   9,  118, 134,  83,  75,  62,  60,  45 },
	{  5,  10,  140, 147,  70,  70,  70,  67,  42 },
	{  5,  12,   70,  95,  85,  85, 108,  67,  65 },
	{  5, 128,   80,  95,  90,  85,  85,  80,  60 },
	{  5, 330,   80, 105, 100, 100,  85,  75,  55 },
	{  5, 522,   80, 105,  85,  75,  95,  80,  80 },
	{  6,   1,   75,  90,  75,  75,  75, 110,  75 },
	{  6,   4,   65,  80,  95,  80,  75, 115,  75 },
	{  6,   7,   70,  85,  90,  85,  75,  95,  75 },
	{  6,  11,   70,  90,  95,  90,  67, 115,  50 },
	{  6, 522,   70,  95,  85,  75,  85, 115,  75 },
	{  7,   1,   80,  80, 105,  85,  75,  75,  75 },
	{  7, 128,   75,  75, 120,  95,  75,  80,  55 },
	{  7, 522,   75,  85, 115,  85,  85,  80,  75 },
	{  8,   1,   80,  75,  75,  85,  75,  75, 110 },
	{  8,   4,   70,  65,  95,  90,  75,  80, 110 },
	{  8,   7,   75,  70,  90,  95,  75,  60, 110 },
	{  8, 130,   95,  75,  90,  80,  65,  70, 100 },
	{  8, 522,   75,  80,  85,  85,  85,  80, 110 },
	{  9,   1,  100,  75,  85,  90,  75,  75,  75 },
	{  9,   2,  128,  95,  92,  85,  60,  70,  55 },
	{  9,   4,   90,  65, 105,  95,  75,  80,  75 },
	{  9,   6,   85,  65, 100,  90,  99,  83,  60 },
	{  9,   7,   95,  70, 100, 100,  75,  60,  75 },
	{  9,   8,  115,  90,  80, 105,  60,  83,  45 },
	{  9,  11,   95,  75, 105, 105,  67,  80,  50 },
	{  9,  12,   85,  70,  95, 100,  98,  67,  60 },
	{  9, 130,  115,  75, 100,  85,  65,  70,  65 },
	{  9, 330,   95,  80, 110, 115,  75,  75,  50 },
	{  9, 522,   95,  80,  95,  90,  85,  80,  75 },
	{ 10,   2,  103, 105,  82,  70,  60, 105,  60 },
	{ 10,   9,  108, 119,  83,  75,  52,  95,  45 },
	{ 10,  10,  130, 132,  70,  70,  60, 102,  42 },
	{ 10, 128,   70,  80,  90,  85,  75, 115,  60 },
	{ 10, 130,   90,  85,  90,  70,  65, 105,  70 },
	{ 10, 330,   70,  90, 100, 100,  75, 110,  55 },
	{ 11,   1,   75,  80,  75,  85, 110,  75,  75 },
	{ 11,   3,   60,  75,  70,  80, 142,  83,  70 },
	{ 11,   6,   60,  70,  90,  85, 134,  83,  60 },
	{ 11,  12,   60,  75,  85,  95, 133,  67,  60 },
	{ 11, 128,   70,  75,  90,  95, 110,  80,  55 },
	{ 11, 330,   70,  85, 100, 110, 110,  75,  50 },
	{ 11, 522,   70,  85,  85,  85, 120,  80,  75 },
	{ 12,   1,   75,  90,  75,  75, 110,  75,  75 },
	{ 12,   3,   60,  85,  70,  70, 142,  83,  70 },
	{ 12,   5,   55,  80,  85,  70, 127,  95,  80 },
	{ 12,   6,   60,  80,  90,  75, 134,  83,  60 },
	{ 12,  12,   60,  85,  85,  85, 133,  67,  60 },
	{ 12, 330,   70,  95, 100, 100, 110,  75,  50 },
	{ 12, 522,   70,  95,  85,  75, 120,  80,  75 },
	{ 13,   1,   75,  90,  75,  75, 110,  75,  75 },
	{ 13,   3,   60,  85,  70,  70, 142,  83,  70 },
	{ 13,   5,   55,  80,  85,  70, 127,  95,  80 },
	{ 13,   6,   60,  80,  90,  75, 134,  83,  60 },
	{ 13,  12,   60,  85,  85,  85, 133,  67,  60 },
	{ 13, 522,   70,  95,  85,  75, 120,  80,  75 },
	{ 14,   1,   75,  75,  75,  75,  90,  75, 110 },
	{ 14,   3,   60,  70,  70,  70, 122,  83, 105 },
	{ 14,   5,   55,  65,  85,  70, 107,  95, 115 },
	{ 14,   6,   60,  65,  90,  75, 114,  83,  95 },
	{ 14,  12,   60,  70,  85,  85, 113,  67,  95 },
	{ 14, 522,   70,  80,  85,  75, 100,  80, 110 },
	{ 15,   2,  103, 110,  92,  75,  60,  85,  60 },
	{ 15,   9,  108, 124,  93,  80,  52,  75,  45 },
	{ 15,  10,  130, 137,  80,  75,  60,  82,  42 },
	{ 15, 128,   70,  85, 100,  90,  75,  95,  60 },
	{ 15, 130,   90,  90, 100,  75,  65,  85,  70 },
	{ 16,   2,  138, 100,  82,  80,  60,  70,  55 },
	{ 16,   8,  125,  95,  70, 100,  60,  83,  45 },
	{ 16,   9,  143, 114,  83,  85,  52,  60,  40 },
	{ 16,  10,  150, 142,  70,  80,  60,  67,  37 },
	{ 16, 130,  125,  80,  90,  80,  65,  70,  65 },
};

// Lookup the creation-base stats for a class+race. Returns nullptr if the
// combo is not a valid creatable combination (should never happen for a real
// bot — bots are constrained to valid race/class at creation). Caller decides
// the fallback; do NOT silently substitute zeros into combat math.
inline const BotCreationBaseStats* GetBotCreationBaseStats(uint8_t class_, uint16_t race) {
	for (const auto &row : kBotCreationBaseStats) {
		if (row.class_ == class_ && row.race == race) {
			return &row;
		}
	}
	return nullptr;
}

// =========================================================================
// DEFIANT TIER ANCHORS — exact shipped CHEST values (live merchantlist pull
// 2026-05-16; merchants 99010 Simple / 99012 Ornate / 99013 Intricate).
// Full-set itemized total = chest * kArmorFactor (locked 4.75).
// Order: ac, hp, mana, str, sta, agi, dex, intel, wis, cha.
// Materials: 0 Plate, 1 Chain, 2 Leather, 3 Cloth (Intricate cloth = Silk).
// =========================================================================
struct DefiantChestAnchor {
	uint16_t ac, hp, mana, str, sta, agi, dex, intel, wis, cha;
};

// [tier 0=Simple(L20) 1=Ornate(L40) 2=Intricate(L60)][material 0..3]
static const DefiantChestAnchor kDefiantChestAnchor[3][4] = {
	{ // Simple (reqlevel 20)
		/*Plate  50039*/ { 18,  0,  0,  4,  3, 0, 0,  1,  1, 0 },
		/*Chain  50046*/ { 14,  0,  0,  1,  3, 1, 1,  0,  3, 1 },
		/*Leath  50053*/ {  9,  0,  0,  1,  2, 0, 1,  0,  3, 0 },
		/*Cloth  50060*/ {  5,  0, 10,  0,  2, 0, 0,  3,  0, 1 },
	},
	{ // Ornate (reqlevel 40)
		/*Plate  50095*/ { 24, 15,  0,  7,  7, 0, 0,  2,  2, 0 },
		/*Chain  50102*/ { 20,  0,  0,  2,  6, 2, 2,  0,  6, 2 },
		/*Leath  50109*/ { 16,  0, 14,  2,  4, 0, 2,  0,  7, 0 },
		/*Cloth  50116*/ { 10,  0, 25,  0,  3, 0, 0,  6,  0, 2 },
	},
	{ // Intricate (reqlevel 60)
		/*Plate  50153*/ { 30, 40,  0, 12, 12, 0, 0,  3,  3, 0 },
		/*Chain  50161*/ { 28, 35,  0,  3, 10, 3, 3,  0, 10, 3 },
		/*Leath  50169*/ { 24, 30, 22,  3,  6, 0, 3,  0, 12, 0 },
		/*Silk   50178*/ { 14, 16, 45,  0,  3, 0, 0, 12,  0, 3 },
	},
};

static const float    kArmorFactor          = 4.75f; // LOCKED — full set = chest * this
static const uint8_t  kAnchorLevels[4]       = { 20, 40, 60, 65 }; // Defiant ramp; <20 = creation-base only
// L65 = Intricate + 5 * (Intricate - Ornate)/20  (slope continues, locked)

enum class BotMaterial : uint8_t { Plate = 0, Chain = 1, Leather = 2, Cloth = 3, MonkCustom = 255 };
enum class BotRole     : uint8_t { Tank, Healer, DPS, CC, Support };

// Role multipliers — LOCKED starting values (§5.3); Group E play-test tunes.
// Redistribute the budget, do not inflate it.
struct RoleMultipliers { float hp, ac, melee, caster; };
inline RoleMultipliers GetRoleMultipliers(BotRole r) {
	switch (r) {
		case BotRole::Tank:    return { 1.20f, 1.15f, 0.90f, 1.00f };
		case BotRole::Healer:  return { 1.00f, 1.00f, 0.80f, 1.20f };
		case BotRole::DPS:     return { 0.90f, 0.90f, 1.15f, 1.00f };
		case BotRole::CC:      return { 0.95f, 0.95f, 0.95f, 1.10f };
		case BotRole::Support: return { 0.95f, 0.95f, 0.95f, 1.10f };
	}
	return { 1.00f, 1.00f, 1.00f, 1.00f };
}

// Class -> role : LOCKED (proposal §3, Alex 2026-05-16).
inline BotRole GetBotRole(uint8_t class_) {
	switch (class_) {
		case 1: case 3: case 5:                     return BotRole::Tank;    // WAR PAL SK
		case 2: case 6: case 10:                    return BotRole::Healer;  // CLR DRU SHM
		case 14:                                    return BotRole::CC;      // ENC
		case 8:                                     return BotRole::Support; // BRD
		default:                                    return BotRole::DPS;     // RNG MNK ROG NEC WIZ MAG BST BER
	}
}

// Class -> base material : from PHASE3_BOTS.md §2; gaps RESOLVED by Alex
// 2026-05-16 — Monk=Leather analog, Rogue=Chain, Berserker=Plate. The §2
// "+X" modifiers are realized numerically below (GetClassPlusMod), scheme
// LOCKED: 0.5 x the donor Defiant material's stat, same tier.
inline BotMaterial GetBotBaseMaterial(uint8_t class_) {
	switch (class_) {
		case 1: case 3: case 5:  return BotMaterial::Plate;       // WAR PAL SK
		case 4: case 8: case 10: return BotMaterial::Chain;       // RNG BRD SHM
		case 2: case 6: case 15: return BotMaterial::Leather;     // CLR DRU BST
		case 11: case 12: case 13: case 14: return BotMaterial::Cloth; // NEC WIZ MAG ENC
		case 7:  return BotMaterial::Leather;                      // MNK — Leather analog (Alex 2026-05-16)
		case 9:  return BotMaterial::Chain;                        // ROG — Chain (Alex 2026-05-16)
		case 16: return BotMaterial::Plate;                        // BER — Plate (Alex 2026-05-16)
		default: return BotMaterial::Leather;
	}
}

// Per-class "+X" modifier (§2) — scheme LOCKED (Alex 2026-05-16): each
// flagged stat gets + frac * (that stat on the DONOR Defiant material, same
// interpolated tier, full-set i.e. * kArmorFactor). Applied L20+ only, on
// top of the class material anchor, BEFORE role multipliers. Classes whose
// own material already supplies the stat (CLR/DRU/BST Leather+WIS, casters
// Cloth+INT, WAR/BER Plate, MNK Leather) get NO modifier — no double-count.
// SK is "+INT" only per §2 exact text (INT drives SK mana in CalcMaxMana).
struct ClassPlusMod {
	BotMaterial donor;
	bool add_wis, add_mana, add_int, add_dex, add_cha;
	float frac;
};
inline ClassPlusMod GetClassPlusMod(uint8_t class_) {
	switch (class_) {
		case 3:  return { BotMaterial::Leather, true,  true,  false, false, false, 0.5f }; // PAL +WIS/Mana
		case 10: return { BotMaterial::Leather, true,  true,  false, false, false, 0.5f }; // SHM +WIS/Mana (Chain mat)
		case 5:  return { BotMaterial::Cloth,   false, false, true,  false, false, 0.5f }; // SK  +INT
		case 4:  return { BotMaterial::Leather, true,  false, false, false, false, 0.5f }; // RNG +WIS
		case 9:  return { BotMaterial::Chain,   false, false, false, true,  false, 0.5f }; // ROG +DEX
		case 8:  return { BotMaterial::Chain,   false, false, false, false, true,  0.5f }; // BRD +CHA
		default: return { BotMaterial::Plate,   false, false, false, false, false, 0.0f }; // none
	}
}

// AA-compensation uplift (§5.2a) — scaffold. Magnitude = Group E play-test;
// returns extra-fraction (currently 0). Hooked from v1 so wiring + the
// diagnostic exist now (feedback_diagnostics_from_v1); only the number is
// pending. AA-start level TBD vs Character: rules (classic gate 51). NOT guessed.
inline float GetAACompFactor(uint8_t /*class_*/, uint8_t /*level*/) {
	return 0.0f; // PLACEHOLDER magnitude — Group E
}

struct BotComputedStats {
	int str, sta, agi, dex, intel, wis, cha; // final attributes
	int hp_bonus, mana_bonus, ac_bonus;      // gear-equivalent vitals (on top of engine native)
};

// Interpolated chest anchor for a material at a level (>=20). 20/40/60 =
// Simple/Ornate/Intricate; 60..65 continues the 40->60 slope (locked).
inline DefiantChestAnchor AnchorAtLevel(int material, uint8_t level) {
	const DefiantChestAnchor &S = kDefiantChestAnchor[0][material];
	const DefiantChestAnchor &O = kDefiantChestAnchor[1][material];
	const DefiantChestAnchor &I = kDefiantChestAnchor[2][material];
	DefiantChestAnchor r{};
	auto mix = [&](float s, float o, float i) -> uint16_t {
		float v;
		if (level <= 40)      v = s + (o - s) * (level - 20) / 20.0f;
		else if (level <= 60) v = o + (i - o) * (level - 40) / 20.0f;
		else                  v = i + ((i - o) / 20.0f) * (level - 60); // 60..65
		return (uint16_t)(v < 0 ? 0 : v + 0.5f);
	};
	r.ac=mix(S.ac,O.ac,I.ac);   r.hp=mix(S.hp,O.hp,I.hp);   r.mana=mix(S.mana,O.mana,I.mana);
	r.str=mix(S.str,O.str,I.str); r.sta=mix(S.sta,O.sta,I.sta); r.agi=mix(S.agi,O.agi,I.agi);
	r.dex=mix(S.dex,O.dex,I.dex); r.intel=mix(S.intel,O.intel,I.intel);
	r.wis=mix(S.wis,O.wis,I.wis); r.cha=mix(S.cha,O.cha,I.cha);
	return r;
}

// Group A bot stat formula. Returns final attributes + gear-equivalent
// vitals. <20: creation-base only (no gear) per §5.2. Role multipliers
// redistribute the GEAR-EQUIVALENT only (NOT the innate creation-base — a
// Tank's natural STR is not nerfed). sta/agi/cha are not in the §5.3 role
// table -> x1.0. Invalid class/race (never for a real bot) falls back to
// the EQ-neutral 75, never 0, so combat math is never silently zeroed.
inline BotComputedStats ComputeBotStats(uint8_t class_, uint16_t race, uint8_t level) {
	const BotCreationBaseStats* cb = GetBotCreationBaseStats(class_, race);
	int bSTR=cb?cb->str:75, bSTA=cb?cb->sta:75, bAGI=cb?cb->agi:75,
	    bDEX=cb?cb->dex:75, bINT=cb?cb->intel:75, bWIS=cb?cb->wis:75, bCHA=cb?cb->cha:75;

	BotComputedStats out{};
	out.str=bSTR; out.sta=bSTA; out.agi=bAGI; out.dex=bDEX;
	out.intel=bINT; out.wis=bWIS; out.cha=bCHA;
	out.hp_bonus=0; out.mana_bonus=0; out.ac_bonus=0;
	if (level < 20) return out; // §5.2: zero gear stats below L20

	int mat = (int)GetBotBaseMaterial(class_);
	if (mat == (int)BotMaterial::MonkCustom) mat = (int)BotMaterial::Leather;
	DefiantChestAnchor a = AnchorAtLevel(mat, level);
	const float F = kArmorFactor;
	float gSTR=a.str*F, gSTA=a.sta*F, gAGI=a.agi*F, gDEX=a.dex*F,
	      gINT=a.intel*F, gWIS=a.wis*F, gCHA=a.cha*F;
	float gHP=a.hp*F, gMANA=a.mana*F, gAC=a.ac*F;

	ClassPlusMod pm = GetClassPlusMod(class_);
	if (pm.frac > 0.0f) {
		DefiantChestAnchor d = AnchorAtLevel((int)pm.donor, level);
		if (pm.add_wis)  gWIS  += pm.frac * d.wis   * F;
		if (pm.add_int)  gINT  += pm.frac * d.intel * F;
		if (pm.add_dex)  gDEX  += pm.frac * d.dex   * F;
		if (pm.add_cha)  gCHA  += pm.frac * d.cha   * F;
		if (pm.add_mana) gMANA += pm.frac * d.mana  * F;
	}

	RoleMultipliers rm = GetRoleMultipliers(GetBotRole(class_));
	gSTR*=rm.melee; gDEX*=rm.melee;
	gINT*=rm.caster; gWIS*=rm.caster; gMANA*=rm.caster;
	gHP*=rm.hp; gAC*=rm.ac;
	// sta/agi/cha: not in §5.3 role table -> x1.0

	float aac = 1.0f + GetAACompFactor(class_, level);
	gSTR*=aac; gSTA*=aac; gAGI*=aac; gDEX*=aac; gINT*=aac; gWIS*=aac; gCHA*=aac;
	gHP*=aac; gMANA*=aac; gAC*=aac;

	out.str=bSTR+(int)(gSTR+0.5f); out.sta=bSTA+(int)(gSTA+0.5f);
	out.agi=bAGI+(int)(gAGI+0.5f); out.dex=bDEX+(int)(gDEX+0.5f);
	out.intel=bINT+(int)(gINT+0.5f); out.wis=bWIS+(int)(gWIS+0.5f);
	out.cha=bCHA+(int)(gCHA+0.5f);
	out.hp_bonus=(int)(gHP+0.5f); out.mana_bonus=(int)(gMANA+0.5f); out.ac_bonus=(int)(gAC+0.5f);
	return out;
}

// =========================================================================
// SYNTHETIC WEAPON (step 3) — equipped weapon is cosmetic; bot melee damage
// + attack delay come from class weapon-type, mirroring the armor curve.
// Scheme CONFIRMED (Alex 2026-05-16). Damage = L1 real baseline ramping to
// Simple@20, then Defiant S/O/I @20/40/60, slope 60->65, x role melee mult.
// Delay = fixed per weapon archetype. No weapon +stat (gear cosmetic).
// L1 anchors + Defiant weapon dmg/delay = real DB pulls (proposal item 10).
// Class->archetype from proposal §6.2 epic-weapon-type map.
// =========================================================================
enum class BotWeaponArchetype : uint8_t { OneHand, Pierce, TwoHand, HandToHand, Bow, CasterLowMelee };

inline BotWeaponArchetype GetBotWeaponArchetype(uint8_t class_) {
	switch (class_) {
		case 1: case 16: return BotWeaponArchetype::TwoHand;        // WAR BER
		case 9:          return BotWeaponArchetype::Pierce;         // ROG (dagger)
		case 7: case 15: return BotWeaponArchetype::HandToHand;     // MNK BST
		case 11: case 12: case 13: case 14:
		                 return BotWeaponArchetype::CasterLowMelee; // NEC WIZ MAG ENC
		default:         return BotWeaponArchetype::OneHand;        // PAL SK RNG DRU CLR SHM BRD
	}
}

// l1 = real basic-vendor anchor; s/o/i = real Defiant weapon dmg by tier;
// delay = real Defiant per-type delay (proposal item 10).
struct WeaponArchetypeData { int l1_dmg, s_dmg, o_dmg, i_dmg, delay; };
inline WeaponArchetypeData GetWeaponArchetypeData(BotWeaponArchetype a) {
	switch (a) {
		case BotWeaponArchetype::OneHand:        return {  4,  7, 12, 18, 22 };
		case BotWeaponArchetype::Pierce:         return {  3,  7, 12, 18, 20 };
		case BotWeaponArchetype::TwoHand:        return {  9, 11, 18, 26, 33 };
		case BotWeaponArchetype::HandToHand:     return {  4,  6, 11, 15, 18 };
		case BotWeaponArchetype::Bow:            return {  3,  6, 11, 15, 25 };
		case BotWeaponArchetype::CasterLowMelee: return {  2,  4,  6,  9, 28 };
	}
	return { 4, 7, 12, 18, 22 };
}

// Bot melee base ("max") damage. <20: L1 baseline ramps to Simple@20
// (accurate-for-L1, continuous — unlike armor which is flat-0 <20, a L1
// player does swing a real weapon). 20/40/60 = Defiant S/O/I; slope->65;
// x role melee multiplier. Never < 1.
inline int ComputeBotMeleeDamage(uint8_t class_, uint8_t level, bool is_ranged) {
	BotWeaponArchetype a = is_ranged ? BotWeaponArchetype::Bow : GetBotWeaponArchetype(class_);
	WeaponArchetypeData w = GetWeaponArchetypeData(a);
	float dmg;
	if (level < 20)       dmg = w.l1_dmg + (float)(w.s_dmg - w.l1_dmg) * (level - 1) / 19.0f;
	else if (level <= 40) dmg = w.s_dmg + (float)(w.o_dmg - w.s_dmg) * (level - 20) / 20.0f;
	else if (level <= 60) dmg = w.o_dmg + (float)(w.i_dmg - w.o_dmg) * (level - 40) / 20.0f;
	else                  dmg = w.i_dmg + ((float)(w.i_dmg - w.o_dmg) / 20.0f) * (level - 60);
	dmg *= GetRoleMultipliers(GetBotRole(class_)).melee;
	if (dmg < 1.0f) dmg = 1.0f;
	return (int)(dmg + 0.5f);
}

inline int ComputeBotWeaponDelay(uint8_t class_, bool is_ranged) {
	BotWeaponArchetype a = is_ranged ? BotWeaponArchetype::Bow : GetBotWeaponArchetype(class_);
	return GetWeaponArchetypeData(a).delay;
}

// =========================================================================
// COSMETIC ARMOR APPEARANCE (step 5a, LOCKED Alex 2026-05-16) — bots show
// class-appropriate armor via per-slot material, NO fake inventory items;
// a real player-equipped item's own material overrides it naturally.
// material: 1 leather, 2 chain, 3 plate(bronze); 0 = none (invisible).
// Casters wear a distinct robe via a chest robe-texture (robe model covers
// the body); Monk = leather so it is not naked.
// =========================================================================
inline int GetBotCosmeticArmorMaterial(uint8_t class_) {
	switch (class_) {
		case 1: case 2: case 3: case 5: case 8:  return 3; // WAR CLR PAL SK BRD -> plate(bronze)
		case 4: case 9: case 10: case 16:        return 2; // RNG ROG SHM BER    -> chain
		case 6: case 7: case 15:                 return 1; // DRU MNK BST        -> leather
		default:                                 return 0; // casters: robe (separate)
	}
}
inline int GetBotCosmeticRobeTexture(uint8_t class_) {
	switch (class_) {
		case 11: return 11; // Necromancer
		case 12: return 12; // Wizard
		case 13: return 13; // Magician
		case 14: return 14; // Enchanter
		default: return 0;
	}
}

// =========================================================================
// COSMETIC WEAPON MODEL (step 5b, LOCKED Alex 2026-05-16) — bots show a
// class-appropriate basic weapon model on an empty primary slot, NO fake
// item (set via FillSpawnStruct Primary.Material, same trick as armor);
// a real player-equipped weapon overrides it. Model numbers = the integer
// in real basic weapons' IDFile ("IT<n>"). H2H -> 0 = bare fists (correct
// classic look for Monk/Beastlord). Caster -> staff (the "robe and staff").
// =========================================================================
inline int GetBotCosmeticWeaponModel(uint8_t class_) {
	switch (GetBotWeaponArchetype(class_)) {
		case BotWeaponArchetype::OneHand:        return 10653; // Short Sword
		case BotWeaponArchetype::Pierce:         return 10650; // Dagger
		case BotWeaponArchetype::TwoHand:        return 10648; // Two-Handed Sword
		case BotWeaponArchetype::HandToHand:     return 0;     // bare fists (Monk/BST)
		case BotWeaponArchetype::Bow:            return 10614; // Short Bow (range slot; not primary)
		case BotWeaponArchetype::CasterLowMelee: return 8;     // Staff
	}
	return 0;
}

#endif // BOT_STAT_MODEL_H
