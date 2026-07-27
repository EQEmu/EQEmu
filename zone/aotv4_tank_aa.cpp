/*	EQEMu: Everquest Server Emulator

	AoTv4 Tank AA tree -- behaviour that does not fit inside an existing function.

	Most of the tank tree lives elsewhere: Shield Oath and Unyielding are native SPAs with no code at
	all, Bloodied Bash is in quests/lua_modules/aotv4_aa_tank.lua, and Aegis Reflex is inline in
	Mob::MeleeMitigation because that is the only place a deflection is visible.

	This file holds Sanctified Blow's ward, Iron Will, and Stonestride -- which WAS a native SPA with
	no code, and was inert for exactly that reason. See its comment below before touching it.
	See custom/sql/aotv4_tank_stun.sql.

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
	constexpr int AA_SANCTIFIED  = 188;   // host 73 Divine Stun, ranks 188,1277,5044,7339,7662
	constexpr int AA_IRONWILL    = 167;   // host 60 Frenzied Burnout, ranks 167,5879,7249,8343,12955
	constexpr int AA_STONESTRIDE = 7;     // host  2 Innate Stamina, ranks 7,8,9,10,11

	constexpr uint16 SPELL_STUN_RANK1 = 43406;   // 43406..43410, one per rank
	constexpr uint16 SPELL_WARD       = 43411;
	constexpr uint16 SPELL_IRONWILL       = 43412;   // melee only
	constexpr uint16 SPELL_IRONWILL_BOTH  = 43413;   // melee and spell, rank 4+

	constexpr int RANKS = 5;

	// ---------------------------------------------------------------- Iron Will
	// What share of the resources it burns comes back as absorption. Rank 2 is where endurance
	// starts being consumed at all; before that only mana is touched.
	constexpr int  IRONWILL_CONVERT_PCT[RANKS] = { 50, 50, 100, 100, 100 };
	constexpr bool IRONWILL_USES_END[RANKS]    = { false, true, true, true, true };
	constexpr int  IRONWILL_TICKS              = 5;    // 30 seconds; rank 5 doubles it
	// ⚠️ BACKSTOP, NOT A BALANCE LEVER. At a THIRTY MINUTE recast this is meant to be enormous --
	// once a fight at best, and it costs every point of mana and endurance you have. So the ceiling
	// sits well above what a normal character can reach and exists only to stop a pathological pool
	// (deep mana plus deep endurance on the same character) producing several health bars of
	// absorption at once. Expressed as a share of the user's OWN maximum health so gear cannot
	// inflate it. Set to 0 to remove the ceiling entirely.
	constexpr int IRONWILL_CAP_PCT_OF_MAXHP = 150;

	// Chance the stun leaves a ward, and how big the ward is as a share of the striker's own maximum
	// health.
	//
	// ⚠️ A SHARE OF MAX HP, not a figure derived from level. That is deliberate and it is the
	// opposite of the trap Sanguine Frenzy fell into. A percentage of DAMAGE DEALT compounds across
	// several multiplicative axes at once -- gear damage, offense, hit count -- and has to be capped.
	// A percentage of your own health compounds with nothing: it is SELF-NORMALISING, because the
	// blows it absorbs scale alongside the health pool it is measured against. Ten percent buys the
	// same few seconds of survival in starter gear and in Mythic, which is exactly what an absorb
	// should do.
	constexpr int WARD_CHANCE[RANKS]      = { 20, 30, 30, 40, 100 };
	constexpr int WARD_PCT_OF_MAXHP[RANKS]= {  2,  4,  6,  8,  10 };

	// ---------------------------------------------------------------- Stonestride
	// Flat points shaved off every melee hit that lands. The figures are the ones the AA has always
	// advertised in its description; only the delivery changed.
	constexpr int STONESTRIDE_FLAT[RANKS] = { 2, 4, 7, 10, 14 };

	inline int RankIndex(int rank)
	{
		if (rank > RANKS) {
			rank = RANKS;
		}
		return rank - 1;
	}
}

// =================================================================================================
// Stonestride. Called from Mob::MeleeMitigation once the mitigation roll has produced a figure;
// `this` is the DEFENDER.
//
// ⚠️⚠️ THIS USED TO BE SPA 162 AND IT DID NOTHING AT ALL. The rank rows carried
// MitigateMeleeDamage (base 100, limit N), which is the documented trick for a flat per-hit
// reduction -- and it genuinely works, but ONLY FOR A SPELL. It is dead twice over on an AA:
//
//   1. Mob::ApplyAABonuses (bonuses.cpp:612-1854) has NO case for MitigateMeleeDamage, so
//      aabonuses.MitigateMeleeRune is never populated. Compare MitigateSpellDamage at
//      bonuses.cpp:1754, which IS handled -- which is why Weathered works and this did not.
//   2. Even populated it would not be read: the consumer at attack.cpp:3880 tests
//      spellbonuses.MitigateMeleeRune only, and pays out of buffs[slot].melee_rune -- a BUFF SLOT,
//      which an AA structurally does not have.
//
// So this is not a case of a missing switch arm; the whole SPA is buff-shaped. Hence a marker AA
// with the reduction applied here in code, exactly like the melee tree's Backs to the Wall, which
// trims the finished figure three lines further down for the same reason.
//
// ⚠️ NEVER TO ZERO. Reaching zero is deflection's job -- it prints its own message and feeds Aegis
// Reflex's stack counter. A flat reduction that silently zeroed a hit would fake a deflection
// without either of those, so it floors at 1 the same way Backs to the Wall does.
// =================================================================================================
int64 Mob::AoTv4Stonestride(int64 damage)
{
	if (damage <= 1) {
		return damage;
	}

	const int rank = static_cast<int>(GetAA(AA_STONESTRIDE));
	if (rank < 1) {
		return damage;
	}

	damage -= STONESTRIDE_FLAT[RankIndex(rank)];
	return damage < 1 ? 1 : damage;
}

// =================================================================================================
// Sanctified Blow's ward.
//
// Called from the SPA 21 handler in spell_effects.cpp, at the point the stun has actually landed --
// after the immunity test and after the resist roll, so a stun that was shrugged off pays nothing.
// `this` is the one who STRUCK; the stunned target is not involved beyond having been stunned.
//
// ⚠️ Gated to OUR stun spells on purpose, not to stuns in general. Firing on any stun would make the
// AA worth more to whoever happened to be handed Bash or a stun spell by the random reward picker,
// and every AA in these trees has to be worth its points to all sixteen classes. Same reasoning that
// cut the combat-skill AA from the melee tree.
// =================================================================================================
void Mob::AoTv4WardOnStun(uint16 spell_id)
{
	if (spell_id < SPELL_STUN_RANK1 || spell_id > (SPELL_STUN_RANK1 + RANKS - 1)) {
		return;
	}

	const int rank = static_cast<int>(GetAA(AA_SANCTIFIED));
	if (rank < 1) {
		return;
	}
	const int i = RankIndex(rank);

	if (!zone || !zone->random.Roll(WARD_CHANCE[i])) {
		return;
	}

	const int64 pool = GetMaxHP() * WARD_PCT_OF_MAXHP[i] / 100;
	if (pool < 1) {
		return;
	}

	SpellOnTarget(SPELL_WARD, this);

	// SPA 55 keeps its absorb pool per buff INSTANCE (spell_effects.cpp), so the row carries a
	// placeholder and the real figure is written here -- it depends on the striker's maximum health
	// and so cannot live in the spell. Replaced rather than added to: this fires on a 30 second
	// recast, and letting successive wards stack would turn an occasional cushion into permanent
	// absorption.
	const uint32 slots = GetMaxTotalSlots();
	for (uint32 s = 0; s < slots; ++s) {
		if (buffs[s].spellid == SPELL_WARD) {
			buffs[s].melee_rune = static_cast<uint32>(pool);
			break;
		}
	}

	Message(Chat::Emote, "A ward settles about you.");
}

// =================================================================================================
// Iron Will -- burn mana and endurance, get absorption.
//
// Called from Client::ActivateAlternateAdvancementAbility once the activation has passed every
// check and the recast timer is set. `this` is the one who used it.
//
// ⚠️ READING OF THE BRIEF. "Convert 1/2 mana into rune ... convert both into rune at 100 percent of
// consumed" is taken to mean the ability spends the WHOLE pool and the rank sets the exchange rate,
// not that it spends half. That is the only reading under which all three ranks describe the same
// mechanic; "100 percent of consumed" makes no sense as a fraction of the pool spent.
//
// ⚠️ It is deliberately all-or-nothing, like Mana Burn: there is no partial spend and no refund if
// the rune goes unused. That is what makes it a decision rather than a rotation button.
// =================================================================================================
void Mob::AoTv4IronWill(int rank)
{
	if (rank < 1 || !IsClient()) {
		return;
	}
	const int i = RankIndex(rank);

	const int64 mana_spent = GetMana();
	const int64 end_spent  = IRONWILL_USES_END[i] ? GetEndurance() : 0;

	if (mana_spent <= 0 && end_spent <= 0) {
		Message(Chat::Red, "You have nothing left to spend.");
		return;
	}

	int64 pool = (mana_spent + end_spent) * IRONWILL_CONVERT_PCT[i] / 100;

	// See IRONWILL_CAP_PCT_OF_MAXHP. A share of the user's own health, so gear decides nothing.
	const int64 cap = GetMaxHP() * IRONWILL_CAP_PCT_OF_MAXHP / 100;
	if (pool > cap) {
		pool = cap;
	}
	if (pool < 1) {
		return;
	}

	SetMana(GetMana() - mana_spent);
	if (end_spent > 0) {
		SetEndurance(static_cast<int32>(GetEndurance() - end_spent));
	}

	// Rank 4 turns aside spell damage as well, which needs the second row -- SPA 78 keeps its pool
	// in a different field from SPA 55, so one row cannot be made to do both on demand.
	const uint16 spell_id = (rank >= 4) ? SPELL_IRONWILL_BOTH : SPELL_IRONWILL;
	const int    ticks    = (rank >= RANKS) ? (IRONWILL_TICKS * 2) : IRONWILL_TICKS;

	BuffFadeBySpellID(SPELL_IRONWILL);
	BuffFadeBySpellID(SPELL_IRONWILL_BOTH);
	SpellOnTarget(spell_id, this, 0, false, 0, false, -1, ticks);

	// Both rows carry a placeholder base; the real pools are written per buff instance because they
	// depend on what was just spent. Same trick as Overflowing Grace and Sanctified Ward.
	const uint32 slots = GetMaxTotalSlots();
	for (uint32 s = 0; s < slots; ++s) {
		if (buffs[s].spellid == spell_id) {
			buffs[s].melee_rune = static_cast<uint32>(pool);
			if (rank >= 4) {
				buffs[s].magic_rune = static_cast<uint32>(pool);
			}
			break;
		}
	}

	Message(Chat::Emote, "You spend everything you have, and it hardens about you.");
}
