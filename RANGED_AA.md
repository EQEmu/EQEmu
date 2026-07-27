# Ranged AA tree — build notes and test plan

Five AAs on the **Ranged** tab (`aa_ability.type = 3`). Built 2026-07-26. **Untested in game.**

| AA | Host | Ranks | Kind |
|---|---|---|---|
| Overload | 44 Quick Damage | 141,142,143,12863,15396 | marker → `GetActSpellDamage` |
| Second Wind | 50 Rabid Bear | 153,1519,5068,6101,7468 | **ACTIVATED**, 5 min recast |
| Corrosion | 33 Combat Stability | 122,123,124,454,455 | marker → `DoBuffTic` |
| Concussive Burst | 34 Combat Agility | 125,126,127,449,450 | marker → `Mob::Damage` |
| Kindred Bond | 32 Finishing Blow | 119,120,121,440,441 | see `PET_WARDS.md` |

Marker rank ids **141, 153, 122, 125** are the only join to `zone/aotv4_ranged_aa.cpp`.

## What each does

**Overload** — 5/10/15/20/25% chance for a direct spell to land for **half as much again**.
Deliberately *not* a crit-chance modifier: the engine already has spell criticals in the same
function (SPA 294 + `BaseCritChance`), and adding to those would have been free — but a crit is a
doubling on its own roll. This is a separate, smaller roll that **stacks** with criticals.

⚠️ It is a **percentage of the spell's own damage**, so it needs no level scaling — unlike a flat
bonus, which would have tripled a level-5 nuke and been a rounding error at 50.

⚠️ **Direct damage only, for free.** `GetActSpellDamage` handles direct spells; damage-over-time
goes through `GetActDoTDamage`. No gating was needed — a flat or percentage bonus firing once per
tick would have been six times the intended value.

**Second Wind** — consumes **all** endurance, returns 40/55/70/85/100% of it as mana. Overfill is
lost, so using it at full mana wastes the pool. 5-minute recast (endurance regenerates; a shorter
one makes it a mana battery rather than a decision).

⚠️ **Known interim imbalance, accepted knowingly.** Endurance and mana pools are the same size
(900 base each at level 50) and a pure caster has no other use for endurance — so today this is
close to free, roughly doubling a caster's mana. It becomes a genuine trade once endurance is the
currency for defensive AAs. Until then the recast is the only thing holding it down.

**Corrosion** — 10/15/20/25/30% chance per DoT tick to erode the resist **that DoT is checked
against**, read from its own `resisttype`. Fire wears down fire, poison wears down poison. That is
what makes it worth taking whatever kind of caster you are rather than rewarding one element.

⚠️ It **refreshes rather than stacks** — which is what EQ does with a repeated spell from one caster
anyway. Accumulating erosion would need a counter keyed on caster *and* target; this gets the same
feel with none of that.

⚠️ Resist debuffs need a **negative base** (SPA 46 fire / 47 cold / 48 poison / 49 disease / 50
magic / 111 all). A positive value silently *buffs* the target's resistance.

⚠️ The debuff rows are `resisttype 0` (unresistable) on purpose — otherwise the thing you are trying
to make easier to hit gets a saving throw against being made easier to hit.

**Concussive Burst** — falling below 30% health stuns everything within 40 units. Cooldown
5/4/3/2.5/2 minutes. Uses the same crossing test the engine already uses for `TryDeathSave` at 16%,
so it fires once on the way down rather than on every hit while low.

⚠️ **This is an escape from adds, not a raid tool, and the engine enforces that** — most raid bosses
carry `SpecialAbility::StunImmunity` and simply will not be held.

⚠️ **SPA 21's `max` field is the highest target level it can stun, and 0 does NOT mean unlimited** —
it silently falls back to `BaseImmunityLevel` (55). Set to 70 explicitly. This is the trap that
would make it quietly stop working in the fifties.

✅ **Stun is not classed as crowd control here.** `IsCrowdControlSpell` deliberately omits it "so
stun-nukes keep working", and the 30-second CC-immunity window only triggers on an **NPC's** CC
landing on a **player**. So this cannot burn that window or interfere with an enchanter's mez.
(An earlier review of this idea claimed otherwise — that was wrong.)

## ⚠️ Timer slots — check before adding a fifth activated AA

`spell_type` is a **timer slot**, not a category: the recast is keyed on
`rank->spell_type + pTimerAAStart`, so two activated AAs sharing a value **share one cooldown**.

| Slot | AA | Recast |
|---|---|---|
| 2 | Sanguine Frenzy (melee) | 45 s |
| 3 | Second Wind (ranged) | 5 min |
| 6 | Sanctified Blow (tank) | 30 s |
| 14 | Iron Will (tank) | 30 min |

## Test plan

1. **Overload** — nuke repeatedly at rank 1 and watch for "Your magic overloads!". Confirm the
   damage is ~1.5×, and that it can occur **on the same cast as a crit** (they stack).
2. **Overload does not fire on DoTs** — cast a DoT and confirm no overload messages per tick.
3. **Second Wind** — at low mana and full endurance, activate. Endurance should empty and mana rise
   by the rank's share. Repeat at **full mana** and confirm the endurance is spent for nothing.
4. **Corrosion** — apply a fire DoT and watch for "Scorched Wards" on the target; a poison DoT
   should give "Envenomed Wards". Wrong-element debuffs mean the `resisttype` mapping is off.
5. **Concussive Burst** — take damage past 30% with several trash mobs on you; they should all reel.
   Confirm it does **not** re-fire while you stay below 30%, and that it respects the cooldown.
6. **Boss check** — confirm Concussive Burst does nothing to a stun-immune NPC. That is correct.

## Files

| File | What |
|---|---|
| `custom/sql/aotv4_aa_ranged.sql` | four AAs + spells 43430-43437 |
| `custom/sql/aotv4_aa_ranged_kindred.sql` | Kindred Bond |
| `zone/aotv4_ranged_aa.cpp` | all four behaviours |
| `zone/effects.cpp` | `GetActSpellDamage` — Overload |
| `zone/spell_effects.cpp` | `DoBuffTic` — Corrosion |
| `zone/attack.cpp` | `Mob::Damage` — Concussive Burst trigger |
| `zone/aa.cpp` | activation hook — Second Wind |
