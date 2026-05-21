# Progressive Dungeons — Working MVP Handoff

Branch: `progressive-dungeons-working-mvp`

## Why this design

The native EQEmu LDoN recruiter system was attempted first but abandoned: the
LDoN recruiter packet flow is tightly bound to its hosting zone/context and
does not generalize to arbitrary dungeon entries the way our progression
system needs.

The current runtime instead uses a **custom Lua NPC menu plus the EQEmu
expeditions/tasks** subsystems. The recruiter is a normal scripted NPC; the
instance lifecycle is owned by an Expedition; per-player progress is tracked
in Data Buckets; per-NPC scaling is performed in `event_spawn` from a global
hook.

## Components

### Recruiter NPC

- Zone: `draniksscar`
- NPC id: **302093** (`server/quests/draniksscar/302093.lua`)
- Menu-driven; offers a `[Start Mission]` option per dungeon, gated by tier.
- On mission start it creates an Expedition (TTL `2h`) and writes per-instance
  markers consumed by `global_npc.lua` when mobs spawn.
- The `[Abandon Mission]` chat link was removed in this MVP; players use the
  built-in expedition window to leave/abandon.

### Per-NPC scaling and clean-up

- File: `server/quests/global/global_npc.lua`
- Function: `pd_scale_npc()` — invoked by `event_spawn` for every NPC.
- Behavior inside a PD instance (zone short name is in `PD_DUNGEON_SHORTS`):
  1. **Raid-flagged NPCs are depopped** as a defensive measure. These mobs
     trigger `Map::FindCeiling` null-deref crashes in instances with no
     loaded map. See "Known issues" below.
  2. **All NPCs are stripped of loot/cash**: `loottable_id=0`,
     `ClearItemList()`, `AddCash(0,0,0,0)`. Loot is intentionally disabled
     for this MVP until the reward overhaul lands.
  3. **Stats are rescaled to the active tier** (`level = tier`, HP/damage
     scaled by ratio, fixed atk/ac/resists/stats, `npc_spells_id=0`,
     `special_abilities=''`).

### Progression bookkeeping

- File: `server/quests/lua_modules/progressive_dungeons.lua`
- Player data-bucket keys (per-player):
  - `pd_highest_completed` — highest tier ever completed
  - `pd_total_completed`   — total completions across all dungeons
  - `pd_active_tier`       — tier of the currently active mission
  - `pd_active_instance_id`
  - `pd_active_zone`
- Per-instance markers (TTL `D2`):
  - `pd_inst_<instance_id>_tier`
  - `pd_inst_<instance_id>_zone`
- Constants: `MAX_TIER = 65`, safe-return = `draniksscar (-447, 1722, -247, 324)`,
  expedition TTL `2h`.

## Tasks

- IDs **990201 – 990217** (one task per dungeon).
- All are `type = 2` (shared task), single kill activity, `target_name = Enemies`.
- `npc_match_list` is the full list of NPC type ids for the dungeon zone.
- `goalcount` is scaled by `clamp(round(spawn2_count / 8), 20, 100)`:

| Task ID | Dungeon          | Short        | goalcount |
|--------:|------------------|--------------|----------:|
| 990201  | Blackburrow      | blackburrow  |        30 |
| 990202  | Befallen         | befallen     |        33 |
| 990203  | Crushbone        | crushbone    |        26 |
| 990204  | Runnyeye         | runnyeye     |        38 |
| 990205  | Solusek's Eye    | soldunga     |        46 |
| 990206  | Upper Guk        | guktop       |        60 |
| 990207  | Lower Guk        | gukbottom    |        57 |
| 990208  | Crypt of Dalnir  | dalnir       |        38 |
| 990209  | Mistmoore Castle | mistmoore    |        44 |
| 990210  | Crystal Caverns  | crystal      |        30 |
| 990211  | Frozen Shadow    | frozenshadow |        59 |
| 990212  | Chardok          | chardok      |        93 |
| 990213  | Old Sebilis      | sebilis      |        62 |
| 990214  | Howling Stones   | charasis     |        57 |
| 990215  | City of Mist     | citymist     |        42 |
| 990216  | Akheva Ruins     | akheva       |        86 |
| 990217  | Sanctus Seru     | sseru        |       100 |

## Warlord's Challenge Horn (click item)

- `items.id = 990001`
- itemtype `11` (no weapon skill — `items` has no `skill` column, weapon skill
  is derived from itemtype)
- `damage = 0`, `delay = 0`, `slots = 8388608` (Inventory only)
- All augslots disabled (`augslot{1..6}{type,visible} = 0`)
- `clickeffect = 42603`, `clicktype = 5`, `nodrop = 0`, `norent = 1`,
  `loregroup = -1`, `maxcharges = -1`
- `spells_new.id = 42603` "Warlord's Challenge" is a silent shim:
  `effectid1 = 254` (no-op), `cast_time = 0`,
  `descnum = 11657` / `typedescnum = 0` / `effectdescnum = 0`.

### Client export files (NOT in this repo)

The client-side display text was generated as part of the same change set
but is not committed here (these are client assets, not server data):

- `spells_us.txt` — row 42603 cloned from spell 3 with id/name swapped,
  SPA `91 → 254`, descnum block `3^125^64 → 42603^0^0`, string refs
  `2512/2106/17355 → -1`.
- `dbstr_us.txt` — appended:
  `42603^6^Issue a challenge to nearby foes, drawing their ire. Tempt fate.^0`

End users need to copy these into their EverQuest client folder and relog
for the click text to render correctly.

## Known issues / risks

- **fearpath null-deref crash risk.** When mobs flee inside a PD expedition
  instance whose zone map is not loaded, `Map::FindCeiling` dereferences a
  null map pointer (path: NPC weapon proc → fear spell →
  `Mob::CalculateNewFearpoint` → `Map::FindCeiling`,
  `zone/fearpath.cpp` around line 376). This was hit in practice via the
  Sebilis instance.
  - Current mitigation: raid-flagged NPCs are depopped on spawn inside PD
    instances (the original crash trigger was a raid mob's weapon proc).
  - The real fix is a C++ null-check in `fearpath.cpp:~376`, deferred.
- **No loot or rewards.** All NPCs in PD instances have their loot tables
  zeroed and cash drops suppressed. Mission completion has no reward beyond
  the tier-progress bookkeeping. A reward overhaul is pending.
- **Doors still require keys / picking.** No pass over `doors` has been made
  yet for the PD zone set. A future migration should run, for each PD zone
  id with `version = 1`:
  `UPDATE doors SET keyitem = 0, lockpick = 0 WHERE zone IN (...) AND version = 1;`

## Pending follow-up work (not in this branch)

- `event_task_fail` handler in `progressive_dungeons.lua` to clear the
  `pd_active_*` data buckets when a task is failed or abandoned.
- Door key/lockpick removal SQL for the 17 PD zone ids (`version = 1`).
- Reward overhaul for PD instances (loot tables, completion bonus, currency).
- C++ null-check in `zone/fearpath.cpp` around line 376 to prevent the
  `Map::FindCeiling` crash in mapless instances.
