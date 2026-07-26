# Healer AA tree — build notes and test plan

Five AAs on the **Healer** tab (`aa_ability.type = 2`). Built 2026-07-26. **Nothing below has been
exercised in game** — it compiles, deploys, and the data reads back correctly, and that is all.

---

## 1. What each one does

| AA | Host | Ranks | Where the work happens |
|---|---|---|---|
| Overflowing Grace | 8 (Innate Fire Protection) | 37-41 | `HealDamage` + `RuneAbsorb` |
| Triage Instinct | 9 (Innate Cold Protection) | 42-46 | `GetActSpellHealing` |
| Mender's Echo | 10 (Innate Magic Protection) | 47-51 | `HealDamage` |
| Cleansing Renewal | 11 (Innate Poison Protection) | 52-56 | the four cure sites |
| Borrowed Breath | 12 (Innate Disease Protection) | 57-61 | `HealDamage` |

All are **marker AAs** — no `aa_rank_effects`. Every one is conditional on something no SPA can
express (the target's health, an overheal, a cure landing), so the behaviour is in
`zone/aotv4_healer_aa.cpp` and reads its rank with `GetAA(<first rank id>)`.

**Rank 5 of Mender's Echo was changed** from the original design. It read "echo healing can trigger
your heal procs or secondary healing effects", which is a request to remove the reentry guard that
stops an echo echoing itself — and combined with Overflowing Grace it would have looped
echo → overheal → shield → echo. It is now "the echo itself can critically heal": bounded, one extra
multiplier, no recursion.

**Borrowed Breath ranks 1-4 were changed** from 3/5/8 *percent* damage reduction to **flat points
per hit** (3/5/6/8/10). Percentage mitigation is imperceptible against this server's post-mitigation
numbers — the same trap Passive Protection hit and the reason Stonestride is flat. See CLAUDE.md §14.

## 2. The buffs are real, not markers

`custom/sql/aotv4_healer_buffs.sql`, ids **43390-43397** (helper band, never offered):

| id | Spell | SPA | Notes |
|---|---|---|---|
| 43390 | Overflowing Grace | 55 Rune | absorb pool written from code |
| 43391-43395 | Borrowed Breath | 162 | flat 3/5/6/8/10 per hit. **No SPA 232** — see §5 |
| 43396 | Cleansing Renewal | 0 | regen, rank 3+ |
| 43397 | Grace Renewed | 0 | regen, paid when a shield is spent |

These do real work through native SPAs. That is deliberately different from the marker *buffs*
elsewhere (43022 Divine Aura, 43035 Blade Turn, the Thirst line, the Shielded buffs), which exist
only to be looked at and are a known placeholder.

**Five rows for Borrowed Breath** because the flat amount lives in the spell's limit field and is
read during bonus calculation — it cannot be poked from code the way the shield's size can, since
bonuses are recalculated constantly and any poke would be reverted.

## 3. Anti-farm gates — check these still hold if the code is touched

The design's biggest hole was **overheal being farmable**: heal an already-full target and 100% of
it is overheal, so Overflowing Grace would mint shields forever for free. Three gates close it, all
in `AoTv4HealerPostHeal`:

1. **`acthealed == 0` returns immediately.** A heal that healed nothing feeds nothing. This is the
   one that actually closes the hole.
2. **Heal-over-time tics are excluded** (`IsBuffSpell`). Otherwise a HoT on a nearly-full target
   trickles out shields, echoes and death saves for one cast.
3. **The shield is capped at 20% of the target's max HP**, so repeated topping-up cannot run away.

## 4. Test plan

Nothing here has run. Suggested order — each step is cheap and isolates one mechanism.

### Setup
`#ach` is unrelated; use a GM character. Grant AA points and buy rank 1 of one AA at a time —
**do not buy several at once**, because Triage, Borrowed Breath and Overflowing Grace all fire on
the same emergency heal and you will not be able to tell which produced what.

### Triage Instinct (start here — simplest, no buffs involved)
1. Buy rank 1. Damage a target below 35% health, heal it, note the amount.
2. Heal the same target above 35% health. **The heal should be visibly smaller.**
3. Buy ranks 2-3, repeat. Rank 3 should start producing frequent crit-heal messages *only* on
   low-health targets.
4. Rank 5: crit-heal a target below 35% and watch mana. Refund is 15% of the spell's mana cost.
   - ⚠️ If nothing changes at all, the likely cause is the **rank id join** — `GetAA(42)` must match
     `first_rank_id` of ability 9. A wrong id reads 0 forever and looks exactly like this.

### Cleansing Renewal
1. Get poisoned or diseased (356 counter-applying debuffs exist below id 10000; poison and disease
   are the common ones — **corruption is not worth testing**, there are 10 such debuffs server-wide
   and it is a DoD-era mechanic).
2. Cure it. You should be healed for `level × 2` at rank 1.
3. Rank 3: a "Cleansing Renewal" regen buff should appear on the cured target.
4. Rank 5: cure, then immediately heal that same target — the heal should be 20% larger. The offer
   lapses after 10 seconds and is consumed by the first heal.

### Borrowed Breath
1. Rank 1, heal a target below 15% health → a "Borrowed Breath" buff should appear on them.
2. Have something melee them. Each hit should land for 3 less than normal.
3. Rank 5: heal a target below 15% (this **arms** the save for 20 seconds), then let it take a
   killing blow. It should survive at **15% of max health**, print "You draw one more breath", and
   still be attackable, castable and healable.
4. **Cooldown check:** immediately repeat step 3 on a different target. The second one should
   **die** — the limit is *once every 2 minutes per healer*, not per target. Wait 2 minutes and it
   should work again.
   - The save is hand-rolled, not a SPA — see §5.
   - ⚠️ The cooldown is spent when a death is **actually averted**, not when the save is armed.
     Arming is free, so a healer can have saves standing on several people at once and only the
     first death is prevented. Healing someone who then survives on their own costs nothing.
   - ⚠️ If the target dies when it should not have, check the arming window: the save is only
     placed when the heal lands on someone **already below 15%**. Healing them at 20% arms nothing.
   - If the healer has zoned or died before the save fires, the save still lands but no cooldown is
     recorded — there is nobody left to record it against.

### Overflowing Grace
1. Rank 1, heal a damaged target for far more than it needs. An "Overflowing Grace" buff appears.
2. Have it take melee damage — the first hits should be absorbed.
3. **Farm check:** heal a target already at full health, repeatedly. **No shield should ever
   appear.** If one does, gate 1 above has been broken.
4. **HoT check:** put a HoT on a nearly-full target. No shield should appear from the tics.
5. Rank 5: let a shield be consumed entirely → a "Grace Renewed" regen should appear.

### Mender's Echo
1. Group up, wound two people, heal one. The other should receive ~8% of the heal.
2. **Solo check:** ungrouped, heal yourself while wounded. The echo falls back to the healer, so it
   should still do something — this was added because a group-only AA is dead weight on a server
   with this much solo play.
3. Rank 3: with three wounded group members, two should receive an echo, the second at half.
4. **Recursion check:** watch for runaway healing or a zone hang. The guard is on the healer, so two
   healers can echo independently but neither can echo its own echo.

## 5. Why the death save is hand-rolled

Borrowed Breath rank 5 does **not** use a spell effect. All three native death hooks were checked
and none of them work here — worth recording so nobody re-treads it.

| Hook | When it fires | Why it is unusable |
|---|---|---|
| `TryDivineSave` — **SPA 232** | on the killing blow | drags 4789 Touch of the Divine with it |
| `TryDeathSave` — **SPA 150** | crossing below 16% **and surviving** | never runs on a killing blow |
| `TrySpellOnDeath` — SpellOnDeath | on the killing blow | **cannot save you, by design** |

**SPA 232 is the trap.** `Mob::TryDivineSave` unconditionally casts 4789 *Touch of the Divine* on
top of the save. That is SPA 40 Divine Aura for up to **36 seconds**, during which you cannot attack
(`attack.cpp:1743`), cannot cast (`spells.cpp:566`), and **no other caster can land any spell on
you, heals included** — `spells.cpp:4147`, whose own comment reads *"invuln mobs can't be affected
by any spells, good or bad"*. You are also unattackable, so a boss drops the tank it just failed to
kill and turns on the healer who saved them. It keeps you alive by removing you from the fight.

This is also exactly what the **"Second Chance" tribute** does — its worn effect is spells
5612/5613/5614 (*Divine Res I/II/III*), which are SPA 232 with a 2/4/6% chance. Tributes have no
death-save mechanism of their own: `CalcBonuses` resolves a tribute to an item and runs it through
`AddItemBonuses(..., is_tribute = true)` (`bonuses.cpp:177`), so a tribute is only ever a delivery
mechanism for the same SPAs.

**SPA 150 is misnamed for what people expect.** Divine Intervention and Death Pact are SPA 150, but
`TryDeathSave` is called from the *else* branch of the death check — only when you drop below 16%
health **and live**. It is an emergency heal on crossing a threshold, not a death save. It also
rolls off the *saved player's* Charisma, so it fires roughly 19% of the time on a 60-CHA warrior and
76% on a 255-CHA enchanter, which is backwards for a role-agnostic AA.

**SpellOnDeath looks like a way in and is not.** `Mob::TrySpellOnDeath` always returns false, and
says why (`mob.cpp:6636`): *"attempting to place a heal in these effects will still result in death
because the heal will not register before the script kills you."*

So `Mob::AoTv4TryBorrowedBreath` runs in `Mob::Damage`'s `HasDied()` branch, before the native
saves. No invulnerability, no lockout, no aggro drop, and the return health is a tunable constant
(`BREATH_SAVE_HP_PCT`) rather than a hardcoded 1 HP — at 1 HP the save is a formality that the next
tick of anything undoes.

The **2-minute limit** (`BREATH_SAVE_COOLDOWN_MS`) is enforced here too, and it lives on the
*healer*, not the saved player. Arming is deliberately free: the cooldown is only checked and spent
at the moment a death is actually averted, so a healer who tops up three dying people has armed
three saves but will still only prevent one death. Doing it the other way — charging the cooldown at
arming time — meant a heal on someone who then survived unaided burned the full window for nothing,
and made the description untrue of anything the player could observe.

⚠️ Keep `BREATH_SAVE_COOLDOWN_MS` and the AA's `db_str` description in step; nothing checks them
against each other.

## 6. Known risks

- **All five interact on one cast.** A fully trained healer's clutch heal is bigger, crits more,
  refunds mana, grants damage reduction, leaves a shield, and echoes. Tune individual numbers down
  from what they look like in isolation.
- **The `#worldboss` and Shield Wall systems are also untested**, so a test session that touches
  several at once will be hard to read.
- **Ten native AAs are now consumed** across the Tank and Healer trees. They are all disabled
  server-wide so nothing is lost today, but each is gone if the native set is turned back on.
  Restore instructions are in TURNOVER.md §4.

## 7. Files

| File | What |
|---|---|
| `custom/sql/aotv4_aa_healer_hosted.sql` | the five AAs, ranks, names, descriptions |
| `custom/sql/aotv4_healer_buffs.sql` | spells 43390-43397 |
| `zone/aotv4_healer_aa.cpp` | all five behaviours |
| `zone/mob.h` | the marker state, and the method declarations |
| `zone/effects.cpp` | `GetActSpellHealing` — Triage + Cleansing Renewal payoff |
| `zone/attack.cpp` | `HealDamage` hook, `RuneAbsorb` shield-spent hook |
| `zone/spell_effects.cpp` | the four cure sites |

Deploy is: apply both SQL files → `ninja zone` → world down → `./shared_memory` → world up →
`eqlaunch` → `./export_client_files` and copy `dbstr_us.txt` + `spells_us.txt` to the EQ root.
**Without the client files the AAs render under the host's old name.**
