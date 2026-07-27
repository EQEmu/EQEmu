/*	EQEMu: Everquest Server Emulator

	AoTv4 -- the endurance-funded emergency abilities, one per tree, plus the utility AAs that
	came with them. See custom/sql/aotv4_aa_outs.sql.

	  Last Stand  (tank)    you cannot be reduced below a floor for a few seconds
	  Reprieve    (healer)  a large instant self heal, and snare and root break
	  Fade        (ranged)  everything forgets you
	  Disengage   (melee)   a burst of speed, and nothing can hold you
	  Rally       (tank)    cures fear, root and snare on the whole group
	  Convalesce  (healer)  the group recovers far faster out of combat

	Quickening (ranged) is not here: cast time reduction is native SPA 127 and needs no code.

	⚠️ WHY THE ENDURANCE THRESHOLD EXISTS. A 45 minute recast is the real brake -- endurance
	regenerates in minutes, so on that timer the cost alone would be decorative. What makes it a
	decision is that the SAME resource funds Iron Will and Second Wind: spend your endurance
	converting it to absorption or mana and you have disarmed your emergency exit until it comes
	back. Requiring a MINIMUM as well as consuming it is what creates that tension.

	⚠️ Recast timers are PERSISTENT -- p_timers.Store on save, p_timers.Load on connect -- so
	camping or zoning cannot reset one. That is the whole anti-abuse story and the engine gives it
	to us free.

	⚠️ THE RANK IDS BELOW ARE THE ONLY JOIN TO THE SQL AND NOTHING CHECKS THEM.
*/

#include "../common/spdat.h"
#include "client.h"
#include "entity.h"
#include "groups.h"
#include "mob.h"
#include "zone.h"

extern Zone *zone;

namespace
{
	constexpr int AA_LASTSTAND = 517;   // host 173 Eldritch Rune,      517,518,519,1440,1441
	constexpr int AA_RALLY     = 260;   // host 111 War Cry,            260,261,262,8309,8310
	constexpr int AA_REPRIEVE  = 534;   // host 180 Hand of Piety,      534,535,536,715,716
	constexpr int AA_FADE      = 155;   // host  52 Improved Familiar,  155,533,1344,5279,5289
	constexpr int AA_DISENGAGE = 510;   // host 170 Wrath of the Wild,  510,511,512,7425,7426
	constexpr int AA_CONVALESCE= 77;    // host  18 Healing Adept,       77, 78, 79, 434, 435

	constexpr int RANKS = 5;

	// Shared gate. Every "out" needs at least this share of your endurance and then spends all of it.
	constexpr int OUT_ENDURANCE_MIN_PCT = 50;

	constexpr uint16 SPELL_LASTSTAND = 43440;
	constexpr uint16 SPELL_REPRIEVE  = 43441;
	constexpr uint16 SPELL_FADE      = 43442;
	constexpr uint16 SPELL_DISENGAGE = 43443;
	constexpr uint16 SPELL_RALLY     = 43444;

	// Last Stand: how long the floor holds, and where the floor sits.
	constexpr uint32 LASTSTAND_MS[RANKS]  = { 8000, 10000, 12000, 14000, 15000 };
	constexpr int    LASTSTAND_FLOOR_PCT  = 20;

	// Reprieve: instant heal as a share of maximum health.
	constexpr int REPRIEVE_HEAL_PCT[RANKS] = { 20, 27, 34, 42, 50 };

	// Convalesce: extra regeneration OUT OF COMBAT ONLY, per rank, for the whole group.
	constexpr int CONVALESCE_HP[RANKS]   = { 4, 8, 12, 16, 20 };
	constexpr int CONVALESCE_MANA[RANKS] = { 3, 6,  9, 12, 15 };

	inline int RankIndex(int rank)
	{
		if (rank > RANKS) {
			rank = RANKS;
		}
		return rank - 1;
	}
}

// Shared entry test for the four outs. Returns false and explains itself if the character cannot
// pay. Spends the whole pool on success.
bool Mob::AoTv4SpendOutEndurance()
{
	if (!IsClient()) {
		return false;
	}

	const int64 have = GetEndurance();
	const int64 need = GetMaxEndurance() * OUT_ENDURANCE_MIN_PCT / 100;

	if (have < need) {
		Message(Chat::Red, "You are too spent for that.");
		return false;
	}

	SetEndurance(0);
	return true;
}

// =================================================================================================
// Last Stand (tank). For a few seconds nothing can reduce you below a fraction of your health.
// The floor itself is enforced in Mob::Damage, beside Borrowed Breath's death save.
// =================================================================================================
void Mob::AoTv4LastStand(int rank)
{
	if (rank < 1 || !AoTv4SpendOutEndurance()) {
		return;
	}
	m_aotv4_laststand_until = Timer::GetCurrentTime() + LASTSTAND_MS[RankIndex(rank)];
	SpellOnTarget(SPELL_LASTSTAND, this);
	Message(Chat::Emote, "You set your feet. This is far enough.");
}

// Called from Mob::Damage. Returns the damage to actually apply.
// ⚠️ A FLOOR, NOT INVULNERABILITY: damage still lands and you can still be whittled to the floor,
// you simply cannot be taken past it. It also does nothing about anything that bypasses Damage.
int64 Mob::AoTv4LastStandFloor(int64 damage)
{
	if (m_aotv4_laststand_until <= Timer::GetCurrentTime()) {
		return damage;
	}

	const int64 floor_hp = GetMaxHP() * LASTSTAND_FLOOR_PCT / 100;
	const int64 current  = GetHP();

	if (current <= floor_hp) {
		return 0;               // already at the floor: nothing gets through at all
	}
	if ((current - damage) < floor_hp) {
		return current - floor_hp;   // trim the blow so it stops exactly on the floor
	}
	return damage;
}

// =================================================================================================
// Reprieve (healer). Health back and the ground under your feet -- it reduces no damage at all,
// so it buys an escape rather than a stand.
// =================================================================================================
void Mob::AoTv4Reprieve(int rank)
{
	if (rank < 1 || !AoTv4SpendOutEndurance()) {
		return;
	}

	const int64 heal = GetMaxHP() * REPRIEVE_HEAL_PCT[RankIndex(rank)] / 100;
	if (heal > 0) {
		HealDamage(heal);
	}

	BuffFadeByEffect(SpellEffect::Root);
	BuffFadeByEffect(SpellEffect::MovementSpeed);   // snares

	SpellOnTarget(SPELL_REPRIEVE, this);
	Message(Chat::Emote, "You find your second breath, and your footing with it.");
}

// =================================================================================================
// Fade (ranged). Everything currently angry at you forgets.
//
// ⚠️ IN A GROUP THIS HANDS YOUR ATTACKER TO SOMEBODY ELSE. That is the trade, not a fault, and it
// is stated in the ability description so nobody is surprised by it.
// =================================================================================================
void Mob::AoTv4Fade(int rank)
{
	if (rank < 1 || !AoTv4SpendOutEndurance()) {
		return;
	}

	entity_list.RemoveFromHateLists(this);
	SpellOnTarget(SPELL_FADE, this);
	Message(Chat::Emote, "You slip out of the world's attention.");
}

// =================================================================================================
// Disengage (melee). No mitigation whatsoever -- speed, and nothing may hold you. You have to
// actually leave. The weakest of the four on purpose: melee chose to be in contact.
// =================================================================================================
void Mob::AoTv4Disengage(int rank)
{
	if (rank < 1 || !AoTv4SpendOutEndurance()) {
		return;
	}

	BuffFadeByEffect(SpellEffect::Root);
	BuffFadeByEffect(SpellEffect::MovementSpeed);

	SpellOnTarget(SPELL_DISENGAGE, this);
	Message(Chat::Emote, "You break contact and run.");
}

// =================================================================================================
// Rally (tank utility). Frees the whole group of fear, root and snare.
//
// Not an "out" -- no endurance threshold, and a shorter recast. Tanks had no group utility at all.
// =================================================================================================
void Mob::AoTv4Rally(int rank)
{
	if (rank < 1 || !IsClient()) {
		return;
	}

	auto free_one = [&](Mob *m) {
		if (!m) {
			return;
		}
		m->BuffFadeByEffect(SpellEffect::Fear);
		m->BuffFadeByEffect(SpellEffect::Root);
		m->BuffFadeByEffect(SpellEffect::MovementSpeed);
		SpellOnTarget(SPELL_RALLY, m);
	};

	free_one(this);
	if (Group *g = entity_list.GetGroupByMob(this)) {
		for (int i = 0; i < MAX_GROUP_MEMBERS; ++i) {
			if (g->members[i] && g->members[i] != this) {
				free_one(g->members[i]);
			}
		}
	}

	Message(Chat::Emote, "Your voice cuts through the panic.");
}

// =================================================================================================
// Convalesce (healer utility). Added to CalcHPRegen / CalcManaRegen, and ONLY out of combat.
//
// Unglamorous, and probably the most used AA in any tree: this server has region locking and no
// ports, so a great deal of play is walking somewhere and recovering when you get there.
//
// `this` is the character regenerating -- the bonus is read from whoever in their GROUP has the AA,
// so one healer carries it for everybody.
// =================================================================================================
int64 Mob::AoTv4ConvalesceBonus(bool mana)
{
	int best = 0;

	auto consider = [&](Mob *m) {
		if (!m) {
			return;
		}
		const int r = static_cast<int>(m->GetAA(AA_CONVALESCE));
		if (r > best) {
			best = r;
		}
	};

	consider(this);
	if (Group *g = entity_list.GetGroupByMob(this)) {
		for (int i = 0; i < MAX_GROUP_MEMBERS; ++i) {
			consider(g->members[i]);
		}
	}

	if (best < 1) {
		return 0;
	}
	const int i = RankIndex(best);
	return mana ? CONVALESCE_MANA[i] : CONVALESCE_HP[i];
}
