/*	EQEMu: Everquest Server Emulator

	AoTv4 Melee AA tree -- the behaviour behind custom/sql/aotv4_aa_melee_hosted.sql.

	⚠️ THE RANK IDS BELOW ARE THE ONLY JOIN TO THE SQL, AND NOTHING CHECKS THEM. A wrong id reads 0
	forever, which in game looks exactly like an AA you bought that quietly does nothing.

	WHAT SHAPED THIS TREE. Two facts about combat here, both verified rather than assumed:

	  1. A deflected swing is a TOTAL loss. This server replaces stock AC mitigation with an
	     AC-versus-offense roll, and a full-mitigation roll zeroes the damage outright. Because
	     Mob::CommonOutgoingHitSuccess only runs when damage_done > 0 (attack.cpp), a deflected
	     swing gets no crit, no bonus damage, no proc -- nothing at all.
	  2. Anything added inside CommonOutgoingHitSuccess is POST-mitigation and is never reduced.

	So flat damage beats percentages here by a wide margin, and reliability beats size. That is the
	same lesson the tank tree learned with Stonestride, for the same underlying reason.

	⚠️ EVERY AA IN THIS TREE MUST BE WORTH ITS POINTS TO ALL SIXTEEN CLASSES. AA points are one
	shared currency and every ability is classes=65535 by design. An earlier draft had an AA that
	boosted the cross-class combat skills from skill_pool.lua -- but those come from the RANDOM
	level-up picker, so its value would have been decided by a dice roll. It was cut. Relentless
	likewise uses SPA 225 GiveDoubleAttack rather than SPA 177 DoubleAttackChance, because only 8 of
	16 classes have a Double Attack skill cap and SPA 177 does nothing without the skill.
*/

#include "../common/spdat.h"
#include "client.h"
#include "entity.h"
#include "mob.h"
#include "zone.h"

extern Zone *zone;

namespace
{
	// ---------------------------------------------------------------- joins to the SQL
	constexpr int AA_SUNDER  = 113;  // host 30, ranks 113,114,115,443,444
	constexpr int AA_RHYTHM  = 80;   // host 19, ranks  80, 81, 82,437,438
	constexpr int AA_RELENT  = 86;   // host 21, ranks  86, 87, 88,266,10467
	constexpr int AA_BLEED   = 98;   // host 25, ranks  98, 99,100,4767,4768
	constexpr int AA_EXECUTE = 71;   // host 16, ranks  71, 72, 73,676,677
	constexpr int AA_FRENZY  = 146;  // host 47, ranks 146,5069,6102,7466,7691  (ACTIVATED)
	constexpr int AA_BACKS   = 247;  // host 104 Double Riposte, ranks 247,248,249,504,505
	constexpr int AA_BRACING = 210;  // host  89 Soul Abrasion,  ranks 210,211,212,1316,1317
	constexpr int AA_RUNDOWN = 255;  // host 108 Flurry,         ranks 255,256,257,542,543

	constexpr uint16 SPELL_BLEED_RANK1 = 43400;   // 43400..43404, one row per rank
	constexpr uint16 SPELL_FRENZY      = 43405;

	constexpr int RANKS = 5;

	// A run against one target lapses if you stop hitting it. Long enough to survive a miss or a
	// deflection, short enough that it does not carry between pulls.
	constexpr uint32 RUN_LAPSE_MS = 8000;

	// ---------------------------------------------------------------- Sunder
	// Absolute reduction of the mitigation roll per stack. rolled_mit runs 0..1 where 1.0 is a full
	// deflection, so 0.010 is one percentage point of mitigation removed.
	constexpr double SUNDER_PER_STACK[RANKS] = { 0.010, 0.015, 0.015, 0.020, 0.020 };
	constexpr int    SUNDER_MAX_STACKS       = 5;

	// ---------------------------------------------------------------- Killing Rhythm
	constexpr int64 RHYTHM_FLAT[RANKS]     = { 2, 4, 4, 6, 6 };
	constexpr int64 RHYTHM_PER_STACK       = 1;
	constexpr int   RHYTHM_STACK_CAP[RANKS]= { 0, 0, 5, 5, 10 };   // ramp arrives at rank 3

	// ---------------------------------------------------------------- Executioner
	constexpr float EXECUTE_THRESHOLD[RANKS] = { 25.0f, 25.0f, 35.0f, 35.0f, 35.0f };
	constexpr int64 EXECUTE_FLAT[RANKS]      = { 3, 5, 5, 8, 8 };
	constexpr float EXECUTE_CLEAVE_RANGE     = 25.0f;   // rank 5 overkill splash

	// ---------------------------------------------------------------- Relentless
	constexpr int RELENT_THIRD_SWING_PCT = 15;   // rank 5 only: chance of a third swing

	// ---------------------------------------------------------------- Backs to the Wall
	// FLAT reduction per enemy BEYOND the first. Percentage mitigation is imperceptible here.
	constexpr int BACKS_PER_ENEMY[RANKS] = { 1, 2, 2, 3, 3 };
	constexpr int BACKS_MAX_ENEMIES      = 4;   // stops a huge train becoming immunity

	// ---------------------------------------------------------------- Sanguine Frenzy
	constexpr uint32 FRENZY_WINDOW_MS  = 4000;
	constexpr int    FRENZY_PCT        = 300;                        // of weapon damage dealt
	constexpr int    FRENZY_CAP_PCT[RANKS] = { 8, 11, 14, 17, 20 };  // ...capped, as a share of max HP

	inline int RankIndex(int rank)
	{
		if (rank > RANKS) {
			rank = RANKS;
		}
		return rank - 1;
	}
}

// =================================================================================================
// Sunder. Called from Mob::MeleeMitigation, so `this` is the DEFENDER and `attacker` is the one
// with the AA. Returns the mitigation roll to use.
//
// This is the most valuable AA in the tree because it attacks the thing that actually costs melee
// damage here: a deflection is a total loss, so removing deflections is worth more than adding to
// the hits that already land.
// =================================================================================================
double Mob::AoTv4SunderMitigation(Mob *attacker, double rolled_mit)
{
	if (!attacker) {
		return rolled_mit;
	}

	const int rank = static_cast<int>(attacker->GetAA(AA_SUNDER));
	if (rank < 1) {
		return rolled_mit;
	}
	const int i = RankIndex(rank);

	// Stacks live on the ATTACKER and belong to one target. Switching targets, or letting the run
	// lapse, starts over -- otherwise you would carry a full set of stacks into every new pull.
	const uint32 now = Timer::GetCurrentTime();
	if (attacker->m_aotv4_sunder_target != GetID() || attacker->m_aotv4_sunder_expire <= now) {
		attacker->m_aotv4_sunder_target = GetID();
		attacker->m_aotv4_sunder_stacks = 0;
	}
	attacker->m_aotv4_sunder_expire = now + RUN_LAPSE_MS;

	const int stacks = attacker->m_aotv4_sunder_stacks;

	// Rank 5: a full set of stacks guarantees the next blow lands. Spent whether or not this
	// particular swing would have been deflected anyway -- it is the certainty being bought.
	if (rank >= RANKS && stacks >= SUNDER_MAX_STACKS) {
		attacker->m_aotv4_sunder_stacks = 0;
		return std::min(rolled_mit, 0.95);   // anything under 1.0 cannot be a full deflection
	}

	rolled_mit -= SUNDER_PER_STACK[i] * stacks;
	if (rolled_mit < 0.0) {
		rolled_mit = 0.0;
	}

	// Rank 3 lets a DEFLECTED swing build a stack too, so a whiff still makes progress. Without it
	// the AA is weakest exactly when deflections are most common, which is when you need it.
	const bool deflected = (rolled_mit >= 1.0);
	if (!deflected || rank >= 3) {
		if (attacker->m_aotv4_sunder_stacks < SUNDER_MAX_STACKS) {
			++attacker->m_aotv4_sunder_stacks;
		}
	}

	return rolled_mit;
}

// =================================================================================================
// Killing Rhythm, Executioner and Bloodletting. Called from Mob::CommonOutgoingHitSuccess, so
// `this` is the ATTACKER and the hit has already survived mitigation -- everything added here is
// unmitigated.
//
// ⚠️ The flat numbers look tiny and are not. Post-mitigation hits on this server land in the single
// and low double digits, so +6 flat is a large proportion. These want tuning down, not up.
// =================================================================================================
void Mob::AoTv4MeleeOnHit(Mob *defender, DamageHitInfo &hit)
{
	if (!defender || hit.damage_done <= 0) {
		return;
	}

	const uint32 now = Timer::GetCurrentTime();

	// ------------------------------------------------------------------ Killing Rhythm
	{
		const int rank = static_cast<int>(GetAA(AA_RHYTHM));
		if (rank > 0) {
			const int i = RankIndex(rank);

			if (m_aotv4_rhythm_target != defender->GetID() || m_aotv4_rhythm_expire <= now) {
				m_aotv4_rhythm_target = defender->GetID();
				m_aotv4_rhythm_stacks = 0;
			}
			m_aotv4_rhythm_expire = now + RUN_LAPSE_MS;

			hit.damage_done += RHYTHM_FLAT[i] + (RHYTHM_PER_STACK * m_aotv4_rhythm_stacks);

			if (m_aotv4_rhythm_stacks < RHYTHM_STACK_CAP[i]) {
				++m_aotv4_rhythm_stacks;
			}
		}
	}

	// ------------------------------------------------------------------ Executioner
	{
		const int rank = static_cast<int>(GetAA(AA_EXECUTE));
		if (rank > 0) {
			const int i = RankIndex(rank);
			if (defender->GetHPRatio() < EXECUTE_THRESHOLD[i]) {
				hit.damage_done += EXECUTE_FLAT[i];
			}
		}
	}

	// ------------------------------------------------------------------ Bloodletting
	// Damage that the mitigation roll never sees -- a second answer to the deflection problem,
	// paid out of hit FREQUENCY rather than hit size.
	//
	// ⚠️ Re-applied only once the previous bleed has run out, not on every swing. EQ will not stack
	// a spell from one caster anyway, and casting on every hit would be a great deal of traffic for
	// no gain.
	{
		const int rank = static_cast<int>(GetAA(AA_BLEED));
		if (rank > 0) {
			const uint16 bleed = static_cast<uint16>(SPELL_BLEED_RANK1 + RankIndex(rank));
			if (!defender->FindBuff(bleed, GetID())) {
				SpellOnTarget(bleed, defender);
			}
		}
	}
}

// =================================================================================================
// Executioner rank 5: damage past what the target had left carries to something else nearby.
// Called from Mob::Damage once the victim is known to have died. `this` is the ATTACKER.
// =================================================================================================
void Mob::AoTv4ExecutionerCleave(Mob *victim, int64 overkill)
{
	if (!victim || overkill <= 0) {
		return;
	}
	if (static_cast<int>(GetAA(AA_EXECUTE)) < RANKS) {
		return;
	}

	// Players only, and only onto NPCs. Splashing overkill between players would turn a kill into
	// an attack on a bystander, and letting NPCs do it would silently change every mob in the game.
	if (!IsClient() || !victim->IsNPC()) {
		return;
	}

	Mob *next = nullptr;
	for (auto &e : entity_list.GetNPCList()) {
		NPC *npc = e.second;
		if (!npc || npc == victim || npc->GetHP() <= 0) {
			continue;
		}
		if (npc->CalculateDistance(victim->GetX(), victim->GetY(), victim->GetZ()) > EXECUTE_CLEAVE_RANGE) {
			continue;
		}
		// Only things already fighting us. Otherwise a kill would pull unrelated mobs, which is a
		// nasty surprise to hand someone for buying a damage AA.
		if (!npc->CheckAggro(this)) {
			continue;
		}
		next = npc;
		break;
	}

	if (next) {
		next->Damage(this, overkill, SPELL_UNKNOWN, EQ::skills::SkillHandtoHand, false);
	}
}

// =================================================================================================
// Sanguine Frenzy. Called from Mob::MeleeLifeTap, which already runs on every successful weapon
// swing with the POST-mitigation damage -- exactly "weapon damage done". `this` is the ATTACKER.
// Returns how much health to restore, 0 when the window is shut.
//
// ⚠️ WHY THERE IS A CAP. A percentage of damage dealt compounds with every multiplier in the game:
// the upper gear tiers double weapon damage, Mythic's stat conversions raise offense (so more hits
// land AND fewer are deflected), dual wield and double attack multiply the hit count inside the
// window, and Killing Rhythm and Sunder from this very tree feed straight into damage_done. A flat
// 400 percent would have been a full heal for a fast dual-wielder and a rounding error for a slow
// two-hander -- same cost in points, wildly different value.
//
// Capping the total as a share of the user's OWN max health fixes all of that at once: gear decides
// how FAST you reach the cap, never how much you get, and the AA is worth the same to everyone who
// presses it. It is also immune to future gear inflation.
// =================================================================================================
int64 Mob::AoTv4FrenzyLifetap(int64 damage)
{
	if (damage <= 0 || m_aotv4_frenzy_until <= Timer::GetCurrentTime()) {
		return 0;
	}

	int64 heal = damage * m_aotv4_frenzy_pct / 100;

	const int64 left = m_aotv4_frenzy_cap - m_aotv4_frenzy_healed;
	if (left <= 0) {
		return 0;
	}
	if (heal > left) {
		heal = left;
	}

	m_aotv4_frenzy_healed += heal;
	return heal;
}

// Opens the window. Called from Client::ActivateAlternateAdvancementAbility once the activation has
// passed every check and the recast timer has been set.
void Mob::AoTv4ActivateFrenzy(int rank)
{
	if (rank < 1) {
		return;
	}
	const int i = RankIndex(rank);

	m_aotv4_frenzy_until  = Timer::GetCurrentTime() + FRENZY_WINDOW_MS;
	m_aotv4_frenzy_pct    = FRENZY_PCT;
	m_aotv4_frenzy_cap    = GetMaxHP() * FRENZY_CAP_PCT[i] / 100;
	m_aotv4_frenzy_healed = 0;

	Message(Chat::Emote, "Your weapons thirst.");
}

// =================================================================================================
// Relentless rank 5: a chance at a third swing after a double connects.
// Called from Client::DoAttackRounds. Ranks 1-4 are native SPA 225 and need no code at all.
// =================================================================================================
bool Mob::AoTv4RelentlessExtraSwing()
{
	if (static_cast<int>(GetAA(AA_RELENT)) < RANKS) {
		return false;
	}
	return zone && zone->random.Roll(RELENT_THIRD_SWING_PCT);
}

// =================================================================================================
// Backs to the Wall. Flat damage reduction that scales with how many things are actually on you.
// Called from Mob::MeleeMitigation once the blow's damage is known; `this` is the DEFENDER.
//
// ⚠️ FLAT, not a percentage -- percentage mitigation is imperceptible against this server's
// post-mitigation numbers (CLAUDE.md section 14), the trap Passive Protection and the first draft of
// Borrowed Breath both fell into.
//
// ⚠️ It gives NOTHING against a single attacker, on purpose. A defensive passive that pays out when
// things are going well can be farmed by pulling carefully; this one only rewards the situation
// melee actually ends up in and cannot be arranged deliberately without genuine risk.
// =================================================================================================
int64 Mob::AoTv4BacksToTheWall(int64 damage)
{
	if (damage <= 0 || !IsClient()) {
		return damage;
	}

	const int rank = static_cast<int>(GetAA(AA_BACKS));
	if (rank < 1) {
		return damage;
	}

	// Everything currently angry at us, minus the one whose blow this is.
	const int extra = static_cast<int>(CastToClient()->GetAggroCount()) - 1;
	if (extra < 1) {
		return damage;
	}

	int64 reduce = static_cast<int64>(BACKS_PER_ENEMY[RankIndex(rank)]) *
	               (extra > BACKS_MAX_ENEMIES ? BACKS_MAX_ENEMIES : extra);

	damage -= reduce;
	return damage < 1 ? 1 : damage;   // never to zero: that is deflection's job, not this
}

// =================================================================================================
// Bracing. Called from Mob::CommonDamage where melee push is applied; `this` is the one being
// shoved. Returns true if the shove should be ignored.
//
// ⚠️ MELEE PUSH ONLY. Spell knockback takes a different path and is NOT covered -- worth saying in
// the description rather than letting someone discover it mid-fight.
// =================================================================================================
bool Mob::AoTv4Braced()
{
	return static_cast<int>(GetAA(AA_BRACING)) >= 1;
}

// =================================================================================================
// Run Them Down. Called from Mob::CheckFlee; `this` is the NPC thinking about running.
// Returns true if somebody currently fighting it is holding it in place.
//
// ⚠️ The hate list is walked on every flee check, so the cheap tests come first and it stops at the
// first holder found. Hate lists are short in practice, but this runs for every wounded NPC in the
// zone, so it must stay cheap.
// =================================================================================================
bool Mob::AoTv4HeldInPlace()
{
	auto &haters = GetHateList();
	if (haters.empty()) {
		return false;
	}

	for (auto *h : haters) {
		if (!h || !h->entity_on_hatelist) {
			continue;
		}
		Mob *m = h->entity_on_hatelist;
		if (!m->IsClient()) {
			continue;
		}
		if (static_cast<int>(m->GetAA(AA_RUNDOWN)) >= 1) {
			return true;
		}
	}
	return false;
}
