# World Boss — test plan

A boss is armed for a random classic dungeon, the server is told, and **everyone who damages it can
roll on the corpse**. There is no raid — loot rights are granted directly.

Status: **written, never run.** Everything below is untested. This document exists to be worked
through the first time it is fought.

---

## Before you can test

The system needs a **zone restart** — it adds Lua modules (which `#reloadquest` does *not* reload)
and a new `npc_types` row.

```bash
# from /src/build/bin
for p in $(pgrep -x zone); do kill -9 $p; done      # eqlaunch respawns them on demand
```

If the SQL has not been applied yet:

```bash
mysql -h127.0.0.1 -upeq -ppeqpass peq < /src/.devcontainer/custom/sql/aotv4_worldboss.sql
```

No `./shared_memory` run is needed for this feature alone — `npc_types` and loot tables are read at
zone boot. (Other pending work in the deploy backlog does need it.)

---

## Commands

| Command | Effect |
|---|---|
| `#worldboss` | Arm in a random classic dungeon |
| `#worldboss <zone>` | Arm in a named zone (`befallen`, `unrest`, `najena`, …) |
| `#worldboss status` | What is armed, and how long is left |
| `#worldboss clear` | Cancel it |

GM access level 200. Valid zones are listed back at you if you mistype one.

---

## Test 1 — arming and announcing

1. `#worldboss unrest`
2. Expect a **world-wide** emote naming The Estate of Unrest, seen by every character online.
3. `#worldboss status` should report the zone and roughly 60 minutes remaining.

**Check:** the announcement reaches a character in a *different* zone. It uses `eq.world_emote`, so
if it only appears locally, the announcement is broken rather than the encounter.

## Test 2 — the lazy spawn

The boss does **not** exist yet after arming. It spawns when the first player enters.

1. With a boss armed for `unrest`, zone in.
2. Expect a zone-wide emote and `#The_Nameless` standing **on top of you**.
3. `#worldboss status` should now report nothing armed — arming is consumed by the spawn.

**Check:** spawning on the arriving player is intentional (it guarantees valid coordinates in any
zone), but confirm it is not *inside* geometry in a tight dungeon.

## Test 3 — the fight

1. Kill it. Take a group; it has 120,000 HP, summons, enrages and rampages, and is slow- and
   mez-immune.
2. Watch for it being trivial or impossible — both are likely on the first attempt.

## Test 4 — shared loot (the important one)

1. Have **two or more characters, in different groups or ungrouped**, each land at least one hit.
2. On death each should get: *"You have earned a share of the spoils."*
3. A world emote should announce the kill and the number of champions.
4. **Every one of them** should be able to loot the corpse and roll through `/advl`.

**This is the test most likely to fail** — see the first risk below.

---

## Most likely failure points

**1. The hate list may be empty by the time loot rights are granted.**
`on_death` runs from `event_death_complete` and reads `e.self:GetHateList()`. If EQEmu has already
torn the hate list down as part of death processing, the loop grants rights to *nobody* and only the
killer can loot. **Symptom:** no "share of the spoils" message, kill emote says `0 champions`.
**Fix if so:** accumulate damagers into a bucket during the fight (`event_damage_taken` on the NPC)
instead of reading the hate list at the end.

**2. The corpse may not exist yet.**
`on_death` looks it up with `GetCorpseByName`. If the corpse is created after this event fires, the
lookup returns nil and the function silently does nothing. **Symptom:** identical to the above — no
messages at all. Distinguish the two by whether the kill emote appears (it only prints after a
successful corpse lookup).

**3. Two bosses at once.**
Running `#worldboss` twice arms the second over the first *in the bucket*, but if the first already
spawned it is still alive in its zone. Nothing tracks spawned bosses. Use `#worldboss status` before
arming, and `#kill` any stray.

**4. It may never despawn.**
If nobody kills it, the boss persists until its zone empties and the zone process shuts down. There
is no cleanup timer. Not harmful, but it means a half-killed boss lingers.

**5. It looks like a dragon.**
Cloned from Lord Nagafen, so it inherits his race and model. Expect a dragon until the appearance is
chosen deliberately.

**6. Faction.**
Set to `npc_faction_id = 0`. Confirm it actually aggroes players on sight rather than standing
inert.

**7. Region locking.**
A boss armed for a region nobody has unlocked is unreachable — players simply cannot zone in. Keep
`M.ZONES` inside regions that are open.

---

## Tuning

| What | Where | Current |
|---|---|---|
| HP, level, abilities | `custom/sql/aotv4_worldboss.sql` | 120,000 / 50 / summon+enrage+rampage |
| Loot pool | same file, `lootdrop_entries` | 3 rolls from 5 random Hallowed/Mythic items |
| Arm window | `aotv4_worldboss.lua` → `WINDOW_MINS` | 60 minutes |
| Zone pool | `aotv4_worldboss.lua` → `M.ZONES` | 14 classic dungeons |
| Looter cap | engine (`MAX_LOOTERS`) | 72 |

The name `#The_Nameless` is a placeholder. Nothing matches on it — only npc id **2000200** — so
renaming is a single `UPDATE npc_types SET name = '...' WHERE id = 2000200;`.

---

## Not built yet

- **Automatic scheduling.** GM command only, by choice. The hot-zone daemon
  (`custom/sql/aotv4_hotzone_daemon.sh`) is the pattern to copy when the fight is proven.
- **Raid invites.** Deliberately skipped — `Lua_Raid` has no `AddMember` binding, and loot rights
  achieve the same goal without pulling anyone out of their existing group.
- **Multiple bosses / a boss table.** One NPC id, one encounter at a time.
- **Curated loot.** The current pool is five randomly-chosen items, picked to prove the plumbing.
