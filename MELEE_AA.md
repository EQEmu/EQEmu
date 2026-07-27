# Melee AA tree — build notes and test plan

Six AAs on the **Melee** tab (`aa_ability.type = 4`). Built 2026-07-26. **Nothing below has been
exercised in game** — it compiles, deploys, and the data reads back correctly, and that is all.

---

## 1. What shaped this tree

Two verified facts about combat here drove every decision:

1. **A deflected swing is a total loss.** This server replaces stock AC mitigation with an
   AC-versus-offense roll, and a full-mitigation roll zeroes the damage outright. Because
   `CommonOutgoingHitSuccess` only runs when `damage_done > 0` ([attack.cpp:1690](zone/attack.cpp#L1690)),
   a deflected swing gets no crit, no bonus damage, no proc — nothing at all.
2. **Anything added inside `CommonOutgoingHitSuccess` is post-mitigation** and is never reduced.

So **flat beats percentage**, and **reliability beats size**. Two of the six AAs exist purely to
attack the deflection problem, from opposite directions.

## 2. The tree

| AA | Host | Ranks | Kind | Where |
|---|---|---|---|---|
| Sunder | 30 Combat Fury | 113,114,115,443,444 | marker | `MeleeMitigation` |
| Killing Rhythm | 19 Healing Gift | 80,81,82,437,438 | marker | `CommonOutgoingHitSuccess` |
| Relentless | 21 Spell Casting Reinf. | 86,87,88,266,10467 | **native SPA 225** | — (r5 in code) |
| Bloodletting | 25 Spell Casting Subtlety | 98,99,100,4767,4768 | marker | `CommonOutgoingHitSuccess` |
| Executioner | 16 Innate Lung Capacity | 71,72,73,676,677 | marker | `CommonOutgoingHitSuccess` + `Damage` |
| Sanguine Frenzy | 47 Cannibalization | 146,5069,6102,7466,7691 | **activated** | `MeleeLifeTap` |

Marker rank ids — **113, 80, 86, 98, 71, 146** — are the only join to `zone/aotv4_melee_aa.cpp`,
and nothing checks them.

## 3. Every AA must be worth its points to all 16 classes

AA points are one shared currency and every ability is `classes = 65535`. Two design changes came
out of holding that line, and both are worth remembering:

- **A "boost your combat skills" AA was cut entirely.** Those skills (Backstab, Kick, Frenzy, the
  monk strikes) come from `skill_pool.lua` through the **random** level-up picker, so its value
  would have been decided by a dice roll. Two identical characters could buy it and get completely
  different value.
- **Relentless uses SPA 225 `GiveDoubleAttack`, not SPA 177 `DoubleAttackChance`.** Only **8 of 16**
  classes have a Double Attack skill cap, and `Client::CheckDoubleAttack` returns false immediately
  without the skill — SPA 177 would have been dead weight for half the roster. SPA 225 is documented
  as *"Allow any class to double attack with set chance"* and explicitly bypasses that check, while
  still stacking for classes that do have the skill. It also unlocks off-hand doubles, which
  otherwise need skill 150+.

## 4. Sanguine Frenzy — the activated one

4-second window, 45-second cooldown, weapon damage returned as health at 300%, **capped per use** at
8/11/14/17/20% of your own max health by rank.

**Why the cap exists.** A percentage of damage dealt compounds with every multiplier in the game:
the upper gear tiers double weapon damage, Mythic's stat conversions raise offense (so more hits
land *and* fewer are deflected), dual wield and double attack multiply the hit count inside the
window, and Killing Rhythm and Sunder from this very tree feed straight into `damage_done`. An
uncapped 400% would have been a full heal for a fast dual-wielder and a rounding error for a slow
two-hander — same cost in points, wildly different value. Capping as a share of the user's **own**
max health means gear decides how *fast* you reach the cap, never how much you get, and it is immune
to future gear inflation.

Verified: both upper tiers set `damage = damage*2` from the **base**, so Mythic does *not* double
again over Hallowed (Frostwrath 24 → 48 → 48). Its edge is hp/mana/end, AC and heroics.

**Requirements for an activated AA** (`Client::ActivateAlternateAdvancementAbility`, [aa.cpp:1254](zone/aa.cpp#L1254)):

- must have a **valid spell** — 43405
- must have **no `aa_rank_effects`** — that is how the engine tells passive from activated; our
  marker AAs already satisfy it
- `recast_time` is in **seconds**
- the host must already be an activated AA so it has **hotkey sids**; a passive host has −1 and can
  never be dragged to a hotbar

⚠️ **`spell_type` is a timer slot, not a category.** The cooldown is keyed on
`rank->spell_type + pTimerAAStart`, so two activated AAs sharing a `spell_type` **share a cooldown**.
This is the only activated AA enabled, so slot 2 is safe — any future one must take a different slot.

⚠️ **The 4 seconds cannot come from the buff.** Buff durations are whole 6-second tics on a
free-running timer (`tic_timer(6000)`, `BuffProcess`), so a 1-tic buff can expire almost immediately
depending on when you cast it. The window is a timestamp in code; spell 43405 is the visual, and
carries SPA 178 at 1% only so it is a genuine valid buff rather than an all-254 shell.

## 5. Test plan

Nothing here has run. Buy **one AA at a time** — Sunder, Killing Rhythm and Executioner all modify
the same damage number and you will not be able to tell them apart otherwise.

### Sunder (build this understanding first — it is the most tunable)
1. Rank 1. Attack something with high AC and count deflection messages over ~30 swings.
2. Keep attacking the same target — deflections should get **less frequent** as marks build.
3. Switch targets mid-fight; the run resets, so deflections should return to baseline.
4. Rank 3: deflected swings now build marks too, so the effect should ramp even against something
   that is deflecting most of your attacks.
5. Rank 5: at 5 marks the next blow should never be deflected.
   - ⚠️ **This is the tuning dial for the whole tree.** It moves `rolled_mit` directly. If melee
     starts trivialising armoured targets, reduce `SUNDER_PER_STACK` before touching anything else.

### Killing Rhythm
1. Rank 1: hits land for exactly 2 more. Post-mitigation numbers here are small, so this is visible.
2. Rank 3: staying on one target should add another point per consecutive hit, to +5.
3. Switch targets — the ramp resets.

### Relentless
1. Rank 1 on a **caster** (wizard, cleric — a class with no Double Attack cap). It should still
   double attack. **This is the whole point of using SPA 225**; if it does nothing on a caster, the
   effect id is wrong.
2. Rank 5: watch for occasional three-swing rounds.

### Bloodletting
1. Rank 1: a melee hit should apply a "Bloodletting" DoT that ticks for 4.
2. It re-applies only once the previous one expires, not on every swing.
3. ⚠️ Bleeds on **NPCs** survive; `BuffProcess` cleanse-on-peace only strips detrimental buffs from
   player-side entities.

### Executioner
1. Rank 1: bonus damage below 25% target health; rank 3 widens it to 35%.
2. Rank 5: kill something with a large overkill while another mob is **already fighting you** —
   the excess should carry to it.
   - ⚠️ Deliberately restricted to NPCs that already have you on their hate list. Splashing onto
     unaggroed mobs would mean a damage AA silently pulls extra mobs.

### Sanguine Frenzy
1. Confirm it can be dragged to a hotbar at all — that depends on the host's hotkey sids.
2. Activate while fighting. You should see "Your weapons thirst" and be healed as you swing.
3. **Cap check:** at rank 1 you should never heal more than 8% of max health from one activation,
   no matter how hard you hit. This is the most important test in the file.
4. **Cooldown check:** it should refuse for 45 seconds.
5. **Gear check, if you can:** repeat with a Mythic weapon. You should reach the cap *sooner*, not
   heal *more*.

## 6. Known risks

- **Sunder is the risky one.** It moves the mitigation roll, which nothing else in any tree does.
- **Sixteen native AAs are now consumed** across the three trees. All disabled server-wide, so
  nothing is lost today — but each is gone if the native set returns. Restore notes: TURNOVER.md §4.
- **Shield Wall, the world boss, and the tank and healer trees are all still untested**, so a
  session that touches several at once will be hard to read.

## 7. Files

| File | What |
|---|---|
| `custom/sql/aotv4_aa_melee_hosted.sql` | the six AAs, ranks, names, descriptions |
| `custom/sql/aotv4_melee_buffs.sql` | spells 43400-43405 |
| `zone/aotv4_melee_aa.cpp` | all six behaviours |
| `zone/attack.cpp` | `MeleeMitigation`, `CommonOutgoingHitSuccess`, `Damage`, `DoAttackRounds` |
| `zone/mob.cpp` | `MeleeLifeTap` — Sanguine Frenzy |
| `zone/aa.cpp` | `ActivateAlternateAdvancementAbility` — opens the Frenzy window |

Deploy: both SQL files → `ninja zone` → world down → `./shared_memory` → world up → `eqlaunch` →
`./export_client_files`, copy `dbstr_us.txt` + `spells_us.txt` to the EQ root.
