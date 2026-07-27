/*	EQEMu: Everquest Server Emulator

	AoTv4 Ranged AA tree -- see custom/sql/aotv4_aa_ranged.sql.

	Kindred Bond, the fifth member of this tree, lives in aotv4_pet_aa.cpp with the pet wards it
	shares. The four here are:

	  Overload          a chance for a direct spell to land for half as much again
	  Second Wind       ACTIVATED: burn endurance, get mana
	  Corrosion         a chance for your damage-over-time to erode the matching resist
	  Concussive Burst  dropping below 30 percent health stuns everything around you

	⚠️ THE RANK IDS BELOW ARE THE ONLY JOIN TO THE SQL AND NOTHING CHECKS THEM. A wrong id reads 0
	forever, which in game looks exactly like an AA you bought that quietly does nothing.
*/

#include "../common/spdat.h"
#include "client.h"
#include "entity.h"
#include "mob.h"
#include "zone.h"

extern Zone *zone;

namespace
{
	constexpr int AA_OVERLOAD   = 141;   // host 44 Quick Damage,      ranks 141,142,143,12863,15396
	constexpr int AA_SECONDWIND = 153;   // host 50 Rabid Bear,        ranks 153,1519,5068,6101,7468
	constexpr int AA_CORROSION  = 122;   // host 33 Combat Stability,  ranks 122,123,124,454,455
	constexpr int AA_BURST      = 125;   // host 34 Combat Agility,    ranks 125,126,127,449,450
	constexpr int AA_UNBROKEN   = 267;   // host 114 Spell Casting Fury Mastery, 267,268,269,640,641

	constexpr int RANKS = 5;

	// ---------------------------------------------------------------- Overload
	// The bonus is a SHARE of the spell's own damage, so it needs no level scaling: a percentage of
	// output is proportionate at every level by construction. Only the chance is ranked.
	constexpr int OVERLOAD_CHANCE[RANKS] = { 5, 10, 15, 20, 25 };
	constexpr int OVERLOAD_BONUS_PCT     = 50;

	// ---------------------------------------------------------------- Second Wind
	// What share of the endurance burned comes back as mana.
	constexpr int SECONDWIND_PCT[RANKS] = { 40, 55, 70, 85, 100 };
	constexpr uint16 SPELL_SECONDWIND   = 43437;

	// ---------------------------------------------------------------- Corrosion
	constexpr int CORROSION_CHANCE[RANKS] = { 10, 15, 20, 25, 30 };
	// One debuff per resist type, matched to whatever the damage-over-time is checked against.
	constexpr uint16 SPELL_RESIST_MAGIC   = 43430;
	constexpr uint16 SPELL_RESIST_FIRE    = 43431;
	constexpr uint16 SPELL_RESIST_COLD    = 43432;
	constexpr uint16 SPELL_RESIST_POISON  = 43433;
	constexpr uint16 SPELL_RESIST_DISEASE = 43434;
	constexpr uint16 SPELL_RESIST_ALL     = 43435;

	// ---------------------------------------------------------------- Concussive Burst
	constexpr uint16 SPELL_BURST          = 43436;
	constexpr float  BURST_HP_PCT         = 30.0f;
	constexpr uint32 BURST_COOLDOWN_MS[RANKS] = { 300000, 240000, 180000, 150000, 120000 };

	inline int RankIndex(int rank)
	{
		if (rank > RANKS) {
			rank = RANKS;
		}
		return rank - 1;
	}

	// spells_new.resisttype: 1 magic, 2 fire, 3 cold, 4 poison, 5 disease. 0 is unresistable, and
	// 6-9 are the chromatic / prismatic / physical / corruption oddities -- those fall back to the
	// all-resist debuff rather than being skipped, so an exotic damage-over-time still does
	// something. 0 returns nothing: there is no resist to erode.
	uint16 ResistDebuffFor(int resist_type)
	{
		switch (resist_type) {
			case 1:  return SPELL_RESIST_MAGIC;
			case 2:  return SPELL_RESIST_FIRE;
			case 3:  return SPELL_RESIST_COLD;
			case 4:  return SPELL_RESIST_POISON;
			case 5:  return SPELL_RESIST_DISEASE;
			case 0:  return 0;
			default: return SPELL_RESIST_ALL;
		}
	}
}

// =================================================================================================
// Overload. Called from Mob::GetActSpellDamage, which handles DIRECT damage only -- damage over
// time goes through GetActDoTDamage instead, so this cannot accidentally fire once per tick.
// `this` is the caster. Returns the bonus to add, 0 for nothing.
//
// ⚠️ Deliberately NOT a critical chance. The engine already has spell criticals in this same
// function (SPA 294 CriticalSpellChance and the BaseCritChance rule), and adding to those would
// have been free -- but a crit is a doubling on its own roll. This is a separate, smaller roll that
// stacks with criticals rather than replacing them.
// =================================================================================================
int64 Mob::AoTv4OverloadDamage(uint16 spell_id, int64 value)
{
	// ⚠️ SPELL DAMAGE IS CARRIED NEGATIVE through this function (see the final return of
	// GetActSpellDamage, which negates twice). The bonus is therefore a share of `value` WITH its
	// sign preserved -- guarding on "value <= 0" would have disabled the whole ability.
	if (value == 0 || !IsValidSpell(spell_id)) {
		return 0;
	}

	const int rank = static_cast<int>(GetAA(AA_OVERLOAD));
	if (rank < 1) {
		return 0;
	}

	if (!zone || !zone->random.Roll(OVERLOAD_CHANCE[RankIndex(rank)])) {
		return 0;
	}

	Message(Chat::SpellCrit, "Your magic overloads!");
	return value * OVERLOAD_BONUS_PCT / 100;
}

// =================================================================================================
// Corrosion. Called from Mob::DoBuffTic where a damage-over-time tick is applied -- `this` is the
// CASTER and `target` is the thing rotting.
//
// The debuff is chosen from the DAMAGE-OVER-TIME'S OWN RESIST TYPE, so a fire line erodes fire
// resistance and a poison line erodes poison. That is what makes it worth taking whatever kind of
// caster you are, rather than rewarding one element.
//
// ⚠️ It REFRESHES rather than stacks, which is what EQ does with a repeated spell from one caster
// anyway. That was a deliberate simplification -- accumulating erosion needs a counter keyed on
// caster and target, and this achieves the same feel without one.
// =================================================================================================
void Mob::AoTv4CorrodeResists(Mob *target, uint16 spell_id)
{
	if (!target || !IsValidSpell(spell_id)) {
		return;
	}

	const int rank = static_cast<int>(GetAA(AA_CORROSION));
	if (rank < 1) {
		return;
	}

	if (!zone || !zone->random.Roll(CORROSION_CHANCE[RankIndex(rank)])) {
		return;
	}

	const uint16 debuff = ResistDebuffFor(spells[spell_id].resist_type);
	if (!debuff) {
		return;   // unresistable damage has no resist to erode
	}

	SpellOnTarget(debuff, target);
}

// =================================================================================================
// Second Wind -- burn endurance, get mana.
//
// Called from Client::ActivateAlternateAdvancementAbility once the activation is committed.
// `this` is the one who used it. The mirror of the tank tree's Iron Will, and it spends the whole
// pool for the same reason: it is meant to be a decision, not a rotation button.
//
// ⚠️ BALANCE NOTE FOR LATER. Endurance and mana pools are the same size at level 50 (900 base
// each), so a full conversion roughly DOUBLES a caster's mana, and a pure caster has no other use
// for endurance -- today this is close to free. That is accepted knowingly: endurance is intended
// to become the currency for defensive AAs, at which point this becomes a real trade of survival
// for output. Until that exists, the recast is what holds it down.
// =================================================================================================
void Mob::AoTv4SecondWind(int rank)
{
	if (rank < 1 || !IsClient()) {
		return;
	}

	const int64 spent = GetEndurance();
	if (spent <= 0) {
		Message(Chat::Red, "You have no endurance left to spend.");
		return;
	}

	int64 gained = spent * SECONDWIND_PCT[RankIndex(rank)] / 100;

	// Never overfill: the excess is simply lost, so using it at full mana wastes the whole pool.
	const int64 room = GetMaxMana() - GetMana();
	if (gained > room) {
		gained = room;
	}

	SetEndurance(static_cast<int32>(GetEndurance() - spent));
	if (gained > 0) {
		SetMana(GetMana() + gained);
	}

	Message(Chat::Emote, "You drive yourself onward, and the weariness becomes will.");
}

// =================================================================================================
// Concussive Burst. Called from Mob::Damage when the caster's health CROSSES below the threshold,
// the same shape the engine already uses for TryDeathSave at 16 percent. `this` is the one who was
// hurt.
//
// ⚠️ THIS IS AN ESCAPE FROM ADDS, NOT A RAID TOOL, and the engine enforces that for us: most raid
// bosses carry SpecialAbility::StunImmunity, so it simply will not hold them. That is the intended
// shape rather than a shortcoming.
//
// ⚠️ Stun is NOT classed as crowd control on this server -- IsCrowdControlSpell deliberately omits
// it so stun-nukes keep working -- so this does not touch the 30 second CC-immunity window and
// cannot interfere with an enchanter's mez.
// =================================================================================================
void Mob::AoTv4ConcussiveBurst()
{
	const int rank = static_cast<int>(GetAA(AA_BURST));
	if (rank < 1 || !IsClient()) {
		return;
	}
	const int i = RankIndex(rank);

	if (m_aotv4_burst_ready > Timer::GetCurrentTime()) {
		return;
	}
	m_aotv4_burst_ready = Timer::GetCurrentTime() + BURST_COOLDOWN_MS[i];

	// Point blank, centred on the caster -- the spell row carries ST_AECaster.
	SpellOnTarget(SPELL_BURST, this);
	Message(Chat::Emote, "The air cracks outward from you.");
}

// =================================================================================================
// Unbroken Concentration. Called from Mob::CastSpell's channel check; `this` is the caster.
// Returns true if this character simply cannot be interrupted.
//
// ⚠️ WHY THIS NEEDS CODE AT ALL. SPA 235 ChannelChanceSpells looked like it would do the whole job,
// but it does NOT set a chance -- it multiplies one (zone/spells.cpp):
//
//     channelchance = 30 + GetSkill(SkillChanneling) / 400.0f * 100;   // caps around 93
//     channelchance -= attacked_count * 2;
//     channelchance += channelchance * channelbonuses / 100.0f;
//
// The base is capped below 100 and falls with every attacker, so no value of SPA 235 reaches
// immunity. Ranks 1-4 therefore use the native effect (a real, useful multiplier) and rank 5 is a
// marker that short-circuits the roll entirely.
// =================================================================================================
bool Mob::AoTv4CannotBeInterrupted()
{
	return static_cast<int>(GetAA(AA_UNBROKEN)) >= RANKS;
}
