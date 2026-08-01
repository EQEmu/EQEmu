-- aotv4_dungeon.lua -- the scaling dungeon ("Delve") system.
--
-- Pick a level in a window, get dropped into a private INSTANCE of a Dragons of Norrath zone with
-- every mob in it scaled to that level, with a real quest in the journal telling you what to do.
-- Finish the quest and a reward chest drops where you finished it. Loot the chest or press Exit and
-- the instance closes behind you.
--
-- Layers unlock in ORDER: layer N+1 only appears once layer N has been cleared. That is both the
-- progression and the guard rail -- with loot out of scope the reward is XP, and a freely pickable
-- level would just be a power-level machine.
--
-- WHAT IS NATIVE HERE, AND WHY THAT MATTERS
--   client:CreateExpedition(zone, version, secs, name, min, max)  a REAL dynamic zone
--   client:MovePCDynamicZone(zoneid)                              entry, via the DZ
--   eq.destroy_instance(dz:GetInstanceID())                       teardown
--   npc:ScaleNPC(level)                                          scaling, off npc_scale_global_base
--   client:AssignTask(id) + the task system                      the quest journal
-- All four are stock EQEmu and Lua-bound, so this whole system is Lua + SQL with no C++ at all.
--
-- ⚠️ ScaleNPC's Lua binding passes always_scale = true (lua_npc.cpp:782), so it scales mobs that
-- are not flagged for scaling -- which is every stock DoN mob. That is exactly what we want here and
-- is why no npc_types editing is needed.
--
-- See custom/sql/aotv4_dungeon.sql for the tasks and the chest, and CLAUDE.md section 24.

local scale = require("aotv4_dungeon_scale")   -- player-relative creature scaling + the difficulty ledger

local M = {}

M.SCALE = scale        -- so global_player/global_npc can reach the ledger without a second require

-- How often a player inside a delve is re-measured, in seconds. Catches somebody who entered
-- stripped and then geared up. ⚠️ Per CLIENT (see the timer in global_player), so it only ever walks
-- the instance that one player is standing in.
M.RESCALE_SECS = 10

-- ---------------------------------------------------------------- layers
-- One per DoN instanced zone, ordered by difficulty.
--
-- ⚠️⚠️ `version` IS A REAL DoN MISSION VERSION, NEVER 0. Version 0 is the OPEN WORLD spawn set, so
-- instancing it produces a private copy of the ordinary zone -- which is genuinely instanced (the
-- zone boots with its own inst_id and nobody else is in it) but looks and plays exactly like walking
-- into the zone normally, because it is the same layout and the same mobs. The non-zero versions are
-- the actual instanced mission layouts that Dragons of Norrath shipped, with their own spawn sets.
--
-- ⚠️ x/y/z/h come from `dynamic_zone_templates.zone_in_*` for that exact (zone, version) pair -- the
-- entry point DoN itself uses for that mission -- NOT from zone.safe_x/y/z. A version 0 safe point
-- can sit in a part of the map the mission layout does not even populate.
--
--   lvl  zone          ver  spawn pts  npc types  the mission it is
--   50   delvea         4      154        11      Lavaspinner's Lair: Lavaspinner's Locals
--   55   delveb         1       99        15      Tirranun's Delve: Storming the Goblin Palace
--   60   stillmoona     1      111        32      Stillmoon Temple
--   65   stillmoonb     7      141        40      The Ascent: Signal Fires
--   68   thundercrest  15      561         ?      Thundercrest Isles: Secret of the Storm
--   70   thenest       16      144        31      The Nest: Stopping Firiona's Henchmen
--
-- Each is the best-populated non-zero version of its zone. ⚠️ They are SMALLER than version 0 (154
-- against 262 for delvea) -- that is the cost of a real mission layout, and the kill goals in
-- aotv4_dungeon.sql are set against these numbers, not against version 0's.
-- The six dungeons, each entered at its own DoN mission version and entry point.
local D = {
    delvea       = { zone = "delvea",       zoneid = 341, version = 4,  task = 2000300, race = 432,
                     name = "Lavaspinner's Lair",  x = -673,    y = -407,   z = -22,    h = 280 },
    delveb       = { zone = "delveb",       zoneid = 342, version = 1,  task = 2000301, race = 433,
                     name = "Tirranun's Delve",    x = -85,     y = -388,   z = 19,     h = 429 },
    stillmoona   = { zone = "stillmoona",   zoneid = 338, version = 1,  task = 2000302, race = 433,
                     name = "Stillmoon Temple",    x = -9,      y = -78,    z = -30,    h = 0   },
    stillmoonb   = { zone = "stillmoonb",   zoneid = 339, version = 7,  task = 2000303, race = 433,
                     name = "Stillmoon Ascent",    x = 866,     y = 6863,   z = 334,    h = 293 },
    thundercrest = { zone = "thundercrest", zoneid = 340, version = 15, task = 2000304, race = 433,
                     name = "Thundercrest Isles",  x = 56,      y = -1757,  z = 113,    h = 484 },
    thenest      = { zone = "thenest",      zoneid = 343, version = 16, task = 2000305, race = 432,
                     name = "The Nest",            x = -154.99, y = 270.13, z = -79.62, h = 324 },
}

-- ⚠️⚠️ THE LADDER STARTS AT LEVEL 1 AND CLIMBS TO 70. It used to start at 50, which made the first
-- delve unreachable for a new character and meant clearing it jumped straight to 55 -- there was no
-- ladder at all, just the top of one. Fifteen rungs, five levels apart, is what makes "unlock the
-- next once you beat the previous" a progression rather than a gate.
--
-- ⚠️ Six dungeons across fifteen rungs, so each zone is REUSED at several difficulties. That is fine
-- because ScaleNPC covers 1..90 and the mobs are rescaled to the rung (and then to the player), but
-- it is why the tasks are per ZONE and their titles carry no level -- see aotv4_dungeon.sql.
--
-- ⚠️ Order is deliberate: it cycles so consecutive rungs are never the same dungeon, and it ENDS on
-- The Nest, the densest of the six (144 spawn points in its mission version).
local function rung(level, d) local L = {} for k, v in pairs(d) do L[k] = v end L.level = level return L end

-- ⚠️⚠️ ONE RUNG PER LEVEL, 1 to 70 -- NOT every fifth level. It ran in fives originally (15 rungs)
-- and that is too coarse: clearing a delve jumped the next one five levels, so a character was always
-- either over or under geared for the rung in front of them and the ladder read as a series of walls
-- rather than a climb. Seventy rungs one level apart is what makes "clear one, unlock the next" a
-- smooth progression.
--
-- ⚠️ THIS COSTS NO NEW TASKS. The tasks are per ZONE (six of them, times six modes), never per rung --
-- the rung supplies only the scaling level. Going from 15 rungs to 70 therefore touches no SQL at all;
-- it is why the tasks were built per zone with no level in the title in the first place.
--
-- ⚠️ The zone still CYCLES so consecutive rungs are never the same dungeon. With 6 zones and 70 rungs
-- each dungeon comes round roughly a dozen times at steadily higher scaling.
-- ⚠️ M.LAYERS is indexed 1..70 and the index IS the unlock high-water mark, so the order of this
-- table is load bearing: reordering it silently reassigns what every stored `delve_cleared_<id>` means.
local CYCLE = { D.delvea, D.delveb, D.stillmoona, D.stillmoonb, D.thundercrest, D.thenest }

M.MAX_LEVEL = 70

M.LAYERS = {}
for lvl = 1, M.MAX_LEVEL do
    M.LAYERS[lvl] = rung(lvl, CYCLE[((lvl - 1) % #CYCLE) + 1])
end

-- ---------------------------------------------------------------- the evolving charm
-- The Delver's Sigil (custom/sql/aotv4_delve_charm.sql, items 147500-147509) is a NATIVE evolving
-- item whose progress is delve SCORE rather than experience. Ten levels costing `4400 * n^2` each, so
-- it advances at roughly ten runs per level AT AN APPROPRIATE RUNG and takes tens of thousands of runs
-- if farmed at the bottom of the ladder.
-- ⚠️ It is `n^2`, NOT "double the previous" -- this comment said doubling for a while and that was
-- describing the ORIGINAL curve, which was the bug (exponential requirement against quadratic income;
-- see the header of custom/sql/aotv4_delve_charm.sql). The constant is DERIVED from
-- aotv4_dungeon_scale.M.kill_value and the reachable rung ceiling -- retuned 24000 -> 4400 on
-- 2026-08-01 when the character level cap became 30. Never edit it here; it lives in that SQL file.
--
-- ⚠️⚠️ IT MUST BE WORN IN THE CHARM SLOT to gain anything -- that is native
-- (EvolvingItemsManager::DoLootChecks only flags equipment slots), and there is a 30 second arming
-- delay after equipping (rule EvolvingItems:DelayUponEquipping). A sigil in a bag earns nothing, and
-- silently: there is no message and no log line.
-- ⚠️ Slot 0 is the charm slot.
M.CHARM_SLOT       = 0
M.CHARM_ITEM_FIRST = 147500
M.CHARM_ITEM_LAST  = 147509

-- ---------------------------------------------------------------- the delve augments
-- custom/sql/aotv4_delve_augs.sql, generated by custom/tools/gen_delve_augs.pl. Three tiers in three
-- contiguous id blocks: 16 single stat tier 1 rolls, then 40 randomly rolled variants each for tiers
-- 2 and 3. The chest drops one, and FOUR OF A TIER COMBINE INTO ONE RANDOM AUGMENT OF THE NEXT TIER
-- in the Refining Crucible (item 2000060, zone/tradeskills.cpp).
--
-- ⚠️⚠️ THEY ARE ORDINARY ITEMS AND DO NOT EVOLVE. An earlier version made them native evolving items
-- fed by delve score, split across everything worn. It was abandoned because an augment can never
-- appear in the client's evolving-item window -- that list is built CLIENT side and does not descend
-- into augment sockets -- so the sigil would have shown natively while augments needed a second,
-- custom window. Combining removes the problem instead of working around it: the sigil is now the
-- only evolving item on the server and there is exactly ONE evolve window.
-- ⚠️ Delve score therefore feeds the SIGIL ONLY. There is no split any more.
--
-- ⚠️⚠️ THESE BLOCKS MIRROR THE GENERATOR AND ALSO zone/aotv4_tiers.h, AND NOTHING ENFORCES EITHER.
-- Change the variant counts in gen_delve_augs.pl and all three have to move together; nothing will
-- error, the chest and the crucible will just hand out the wrong tier.
M.AUG_ITEM_FIRST   = 147600
M.AUG_ITEM_LAST    = 147915
M.AUG_TIER_BLOCK   = { { 147600, 147615 },    -- tier 1:  16 single stat variants
                       { 147616, 147715 },    -- tier 2: 100 rolled variants
                       { 147716, 147915 } }   -- tier 3: 200 rolled variants

-- ⚠️ Rung bands the chest rolls a tier in, and the chance of one tier above. Kept beside the ids so
-- the two cannot drift; the generator's per level evolve COST is calibrated to the MIDDLE of each of
-- these bands (see %HOME_RUNG there), so moving a band without moving that makes augs evolve at the
-- wrong pace rather than simply dropping in the wrong place.
M.AUG_TIER_MAXRUNG = { 23, 46, 70 }
M.AUG_UPGRADE_PCT  = 15

-- ⚠️ RoF2 exposes six augment sockets; slot indexes are 0-5.
M.AUG_SOCKETS      = 5


M.CHEST_NPC       = 2000300
M.BOSS_NPC        = 2000301
M.DURATION_SECS   = 21600      -- 360 minutes, the same as every native DoN mission
M.SAY_TRIGGER     = "delve"

-- ---------------------------------------------------------------- difficulty modes
-- ⚠️⚠️ EACH MODE NEEDS ITS OWN TASK ROWS, and that is not a style choice: `task_activities.goalcount`
-- is a STATIC DB COLUMN. Nothing can change a kill target at runtime, so a mode that alters the count
-- (Swarm x3, Gauntlet /3) can only be expressed as a different task. Hence `taskoff`: the task id is
-- `<zone base task> + taskoff`, giving six parallel families over the same six zones. It is also what
-- makes Onslaught free -- `tasks.duration` is per task, so its countdown is native with no code.
--
-- ⚠️ `hp` / `dmg` / `lvl` are applied AFTER `scale.apply` in on_npc_spawn, never before: ScaleNPC
-- rewrites stats wholesale from npc_scale_global_base and would discard anything set first (§24).
--
-- ⚠️ `score` multiplies the ledger, which is how difficulty is meant to pay. The ledger's per-kill
-- value is already quadratic in effective level, so Hard (which raises HP/damage but NOT level) would
-- otherwise score identically to Standard -- the multiplier is what records that it was harder.
--
-- ⚠️ Descriptions are sent to the client over DUNGMODES rather than hardcoded in the dll. Same reason
-- the picker's icon set is read from the pool instead of a kIcons[] array (§3): two copies drift, and
-- the one in C++ is the copy nobody remembers to update.
M.MODES = {
    { id = "standard", name = "Standard", taskoff = 0, goalmul = 1,
      hp = 1.00, dmg = 1.00, lvl = 0, swarm = 0, bosses = 1, score = 1.00,
      desc = "The delve as intended. Clear the dungeon, then kill what comes for you at the end." },

    { id = "hard", name = "Hard", taskoff = 10, goalmul = 1,
      hp = 2.00, dmg = 2.00, lvl = 0, swarm = 0, bosses = 1, score = 2.00,
      desc = "Everything hits twice as hard and takes twice as long to kill. Same numbers, twice the fight." },

    { id = "swarm", name = "Swarm", taskoff = 20, goalmul = 3,
      hp = 0.50, dmg = 0.75, lvl = 0, swarm = 3, bosses = 1, score = 1.50,
      desc = "Every kill splits into three more. Individually weaker, endlessly many: ninety of them before the boss." },

    -- ⚠️ `thin` DEPOPULATES the dungeon; goalmul only lowers the KILL GOAL. Gauntlet needs both, and
    -- shipping it with goalmul alone was a real bug: the kill target dropped to 10 but all ~150 mobs
    -- were still standing there, so the mode looked and played exactly like Standard.
    { id = "gauntlet", name = "Gauntlet", taskoff = 30, goalmul = 0.34, thin = 0.75,
      hp = 1.00, dmg = 1.00, lvl = 0, swarm = 0, bosses = 3, score = 1.75,
      desc = "Barely any trash, but three bosses instead, one after another, each angrier than the last." },

    { id = "onslaught", name = "Onslaught", taskoff = 40, goalmul = 1,
      hp = 1.00, dmg = 1.00, lvl = 0, swarm = 0, bosses = 1, score = 1.50,
      desc = "The same delve against a clock. Run out of time and the run is failed where it stands." },

    { id = "fragile", name = "Fragile", taskoff = 50, goalmul = 1,
      hp = 0.50, dmg = 2.00, lvl = 0, swarm = 0, bosses = 1, score = 1.50,
      desc = "Everything dies in half the time and kills you in half the time. Win the trade or lose the run." },
}

M.MODE_DEFAULT = "standard"

function M.mode_by_id(id)
    for _, m in ipairs(M.MODES) do if m.id == id then return m end end
    return nil
end

-- The task row a (layer, mode) pair actually runs on.
function M.task_for(L, mode) return L.task + mode.taskoff end

-- ⚠️ M.mode_allowed is deliberately NOT here -- it needs `get_cleared`, which is a `local function`
-- declared further down, and in Lua a reference to a local made BEFORE its declaration compiles to a
-- global lookup instead. It would have been nil at call time with no error until somebody picked a
-- mode. It lives immediately after get_cleared for that reason.

-- Live population cap for Swarm. Each kill is net +2 creatures, so ninety kills would otherwise try
-- to put 270 extra bodies in the zone; spawns above the cap are silently skipped rather than queued.
M.SWARM_CAP = 40

-- ---------------------------------------------------------------- buckets
-- cleared_<charid>  highest layer index cleared        (permanent -- this is the unlock chain)
-- run_<charid>      "<layer>|<instid>|<zone>"          (the run in progress; cleared on exit)
-- back_<charid>     "<zone>|<x>|<y>|<z>"               (where to put them back)
local function bkey(c, what) return "delve_" .. what .. "_" .. c:CharacterID() end

local function get_cleared(c)
    return tonumber(eq.get_data(bkey(c, "cleared"))) or 0
end

local function set_cleared(c, n)
    if n > get_cleared(c) then eq.set_data(bkey(c, "cleared"), tostring(n)) end
end


-- ⚠️⚠️ THE CHARM FUNCTIONS LIVE HERE, NOT BESIDE M.CHARM_SLOT ABOVE, because grant_sigil_if_first
-- needs `bkey` -- a `local function` declared just above. A reference made BEFORE a local is declared
-- compiles to a GLOBAL lookup in Lua, so it was nil at call time and threw
--     attempt to call global bkey (a nil value)
-- from inside on_task_complete, aborting the clear after set_cleared but before the chest, the score,
-- the history row and the charm award. It only fired on a FIRST clear, which is why it survived
-- luacheck and several test runs. Same trap as M.mode_allowed below; keep declaration order in mind.
-- The sigil is the reward for FINISHING YOUR FIRST DELVE -- there is no vendor and no drop. That
-- makes it the thing that introduces the whole progression: you clear one dungeon and are handed the
-- object that every later run feeds.
--
-- ⚠️ Awarded ONCE EVER, tracked in `delve_sigil_<charid>`. Not once per layer and not once per mode:
-- the ten evolve levels ARE the long tail, so a second sigil would be worthless anyway -- and it
-- could not even be carried, since all ten share loregroup 20000.
--
-- ⚠️ THE BUCKET IS SET BEFORE THE SUMMON, deliberately. SummonItem pushes to the cursor and reports
-- nothing back, so there is no way to detect a failure and retry; marking first means a clear can
-- never award two. The trade is that a player who destroys their sigil does not get another -- which
-- is the right way round, because the alternative silently re-grants a maxed item back to level 1.
function M.grant_sigil_if_first(c)
    if not c or not c.valid then return end

    local key = bkey(c, "sigil")
    if (tonumber(eq.get_data(key)) or 0) > 0 then return end
    eq.set_data(key, "1")

    c:SummonItem(M.CHARM_ITEM_FIRST)
    c:Message(MT.Yellow,
        "Something in the dungeon marks you. A Cracked Delver's Sigil settles onto your cursor.")
    c:Message(MT.LightBlue,
        "Wear it in your charm slot. Every delve you finish will feed it, and it can grow ten times.")
end

-- Feed a finished run's score into the sigil, if one is worn.
-- ⚠️ The award is applied through the CUSTOM binding c:AddEvolveProgress, which runs the whole
-- native sequence and evolves on the spot; see the block on the call below for why the obvious
-- inst:AddEvolveAmount is not enough. (An earlier version of this comment said there was no Lua
-- binding for the progression check and that the swap could only happen on a later experience tick.
-- That was true before the binding existed -- it is not any more, so do not "restore" it.)
function M.award_charm(c, score)
    if not c or not c.valid or not score or score <= 0 then return end

    local inv = c:GetInventory()
    if not inv then return end
    local inst = inv:GetItem(M.CHARM_SLOT)
    if not inst or not inst.valid then return end

    local id = inst:GetID()
    if id < M.CHARM_ITEM_FIRST or id > M.CHARM_ITEM_LAST then return end

    -- ⚠️⚠️ c:AddEvolveProgress, NOT inst:AddEvolveAmount. The ItemInst binding only bumps an
    -- IN-MEMORY counter: it does not recompute progression, does not write character_evolving_items,
    -- and does not tell the client -- and there is NO save-on-zone for evolving progress anywhere in
    -- the server, so the award was silently discarded the moment the player left the instance. The
    -- symptom was a charm reading 0 percent after cleared runs, with current_amount 0 in the DB.
    -- AddEvolveProgress (custom binding, zone/lua_client.cpp) does the whole native sequence: add,
    -- recompute, persist, push the update packet, and evolve immediately if it crossed 100 percent.
    -- ⚠️⚠️ READ THE LEVEL BEFORE THE CALL, NEVER AFTER. If this award crosses 100 percent the item
    -- evolves inside AddEvolveProgress: the old instance is pulled out of the charm slot and the new
    -- one is put on the CURSOR. `inst` still points at the old item (its free is deferred to the next
    -- CleanDirty, so reading it does not crash) -- it just reports the level the sigil has already
    -- stopped being, which is exactly the moment the message most needs to be right.
    local lvl, maxlvl = inst:GetEvolveLevel(), inst:GetMaxEvolveLvl()
    local pct = c:AddEvolveProgress(M.CHARM_SLOT, score) or 0

    if lvl >= maxlvl then
        c:Message(MT.Yellow, string.format("Your sigil drinks in %d more, though it can grow no further.", score))
    elseif pct >= 100 then
        -- ⚠️ The engine already sends its own EVOLVE_ITEM_EVOLVED line here, so this one only reports
        -- the delve's part and does NOT restate the new name -- `inst` is the old item and could not
        -- name it correctly anyway. ⚠️ Do not print `pct` on this branch: it is the progression
        -- measured BEFORE the swap and reads over 100 (it can be several hundred on a big run).
        c:Message(MT.Yellow, string.format(
            "Your sigil absorbs %d from the delve and reshapes itself. Look to your cursor.", score))
    else
        c:Message(MT.Yellow, string.format(
            "Your sigil absorbs %d from the delve. (%d percent toward its next form.)", score, math.floor(pct)))
    end
end

-- ---------------------------------------------------------------- the chest's augment
-- One augment per cleared delve, tier chosen by the rung with a small chance of one better.
-- ⚠️ The tier decides the STARTING weight (2/4/6 of a single stat) and, through the generator's
-- HOME_RUNG table, how expensive that augment is to evolve. A tier 3 handed out at rung 1 would not
-- merely be strong, it would be priced to evolve at rung 58 income and so would effectively never
-- grow -- which is why the upgrade chance is small and only ever moves ONE tier.
function M.roll_aug_tier(layer_level)
    local tier = 3
    for i = 1, 3 do
        if layer_level <= M.AUG_TIER_MAXRUNG[i] then tier = i break end
    end
    if tier < 3 and math.random(100) <= M.AUG_UPGRADE_PCT then tier = tier + 1 end
    return tier
end

-- ⚠️ Coin: ten platinum per rung, DOUBLED when an affix is running. "Affix" is any mode other than
-- Standard -- they are the difficulty modifiers, and every one of them makes the run materially
-- harder (Hard doubles hp and damage, Swarm triples the bodies, Fragile halves your margin).
M.CHEST_PLAT_PER_LEVEL = 10
M.CHEST_AFFIX_MULT     = 2

-- Put the run's reward INTO the chest, before it is killed.
--
-- ⚠️⚠️ THE AUGMENT GOES ON THE CHEST, NOT ONTO THE PLAYER'S CURSOR. An earlier version summoned it
-- directly, which meant the chest was pure theatre -- it existed to be clicked and gave nothing. Put
-- on the npc, the item and the coin go through the ordinary loot path: the corpse is looted like any
-- other, which is also the only way the coin can be handed over at all (there is no "give platinum"
-- that reads as loot).
-- ⚠️ Stocked at SPAWN rather than on death, because loot has to be on the npc before it dies -- a
-- corpse built from an npc with no loot has nothing to add to.
-- ⚠️⚠️ `chest` ARRIVES AS A Lua_Mob AND MUST BE CAST TO Lua_NPC. `eq.spawn2` returns Lua_Mob
-- (`Lua_Mob lua_spawn2(...)`, zone/lua_general.cpp:306), and AddItem/AddCash/ScaleNPC/ModifyNPCStat
-- are bound on Lua_NPC ONLY -- so calling one on a spawn2 result is *"attempt to call method
-- 'AddItem' (a nil value)"* at runtime, with nothing to warn you at write time. It bit exactly here
-- on 2026-07-30, and the cost was far wider than the chest: the error aborted the rest of
-- on_task_complete, so the clear also paid no score and wrote no history. That is why the accounting
-- now runs BEFORE this function is called.
-- ⚠️ `CastToNPC` IS bound (on Lua_Entity, zone/lua_entity.cpp:178, which Lua_Mob derives from) --
-- do NOT reach for the entity-list lookup here. That workaround belongs to §24's `e.other` case,
-- which is about CastToCLIENT; the two are not interchangeable. M.spawn_boss already casts this way.
local function as_npc(mob)
    if not mob or not mob.valid then return nil end
    local npc = mob:CastToNPC()
    if npc and npc.valid then return npc end
    return nil
end

function M.stock_chest(c, chest, run, L)
    if not c or not c.valid then return end

    local npc = as_npc(chest)
    if not npc then
        -- ⚠️ Loud, not silent: an unstocked chest looks identical to a stocked one until it is opened
        -- and turns out to be empty, which reads as "the reward system is broken" rather than as one
        -- failed lookup.
        c:Message(MT.Red, "[delve] the chest could not be stocked; report this.")
        return
    end
    chest = npc

    local layer_level = (L and L.level) or (run and run.layer) or 1

    local tier  = M.roll_aug_tier(layer_level)
    local block = M.AUG_TIER_BLOCK[tier]
    -- ⚠️ Any member of the tier's block is a legal drop -- unlike the old evolving set there is no
    -- "starting form" to be careful of, because a tier is a flat pool of equally valid rolls.
    local id = math.random(block[1], block[2])
    chest:AddItem(id, 1)

    local affix = run and run.modedef and run.modedef.id ~= M.MODE_DEFAULT
    local plat  = layer_level * M.CHEST_PLAT_PER_LEVEL * (affix and M.CHEST_AFFIX_MULT or 1)
    -- ⚠️ AddCash takes copper, silver, gold, platinum IN THAT ORDER -- passing the amount first
    -- would pay out in copper and look like the reward silently failed.
    chest:AddCash(0, 0, 0, plat)

    c:Message(MT.Yellow, string.format(
        "The chest holds a tier %d augment and %d platinum%s.",
        tier, plat, affix and string.format(" (doubled for %s)", run.modedef.name) or ""))
    if tier < 3 then
        c:Message(MT.Yellow, string.format(
            "Four tier %d augments combine in a Refining Crucible into one tier %d.", tier, tier + 1))
    end
end

-- ⚠️ Modes other than Standard are gated on having CLEARED that layer on Standard. The test is just
-- `idx <= cleared` and needs no extra bookkeeping: a layer's FIRST clear can only ever be Standard,
-- because nothing else is offered for it until it is cleared. So the existing high-water mark already
-- means "cleared on Standard" -- do NOT add a second bucket for this.
-- ⚠️ Defined here rather than beside M.MODES because it needs get_cleared above; see the note there.
function M.mode_allowed(c, idx, mode)
    if not mode then return false end
    if mode.id == M.MODE_DEFAULT then return true end
    return idx <= get_cleared(c)
end

-- The run a character is inside right now, as a table, or nil.
-- ⚠️ The MODE is part of the run record, not just of the entry call: every hook that runs inside the
-- dungeon (scaling, swarm splitting, the boss chain, the score sheet) has to know which mode is being
-- played, and they all run in the delve zone with nothing but the character to go on.
-- ⚠️ A run written by an older build has no mode field -- it falls back to Standard rather than
-- returning nil, so an in-flight run at upgrade time degrades instead of stranding the player.
function M.current_run(c)
    local raw = eq.get_data(bkey(c, "run"))
    if not raw or raw == "" then return nil end
    local layer, inst, zone, mode = raw:match("^(%d+)|(%d+)|(%w+)|?(%a*)$")
    if not layer then return nil end
    if not mode or mode == "" then mode = M.MODE_DEFAULT end
    return {
        layer    = tonumber(layer),
        instance = tonumber(inst),
        zone     = zone,
        mode     = mode,
        modedef  = M.mode_by_id(mode) or M.mode_by_id(M.MODE_DEFAULT),
    }
end

local function set_run(c, layer, inst, zone, mode)
    eq.set_data(bkey(c, "run"), string.format("%d|%d|%s|%s", layer, inst, zone, mode))
end

local function clear_run(c) eq.delete_data(bkey(c, "run")) end

-- ⚠️⚠️ TRANSIT FLAG -- the thing that makes entering a delve possible at all.
--
-- `EVENT_DISCONNECT` FIRES ON EVERY ZONE TRANSFER, not just on camping or going link-dead. In
-- `zone/client_process.cpp` the surrounding function tests `bZoning` for other work --
--     if (!bZoning) { SetDynamicZoneMemberStatus(DynamicZoneMemberStatus::Offline); }   (:735)
-- -- and then fires the event UNCONDITIONALLY thirty lines later (:765). So the moment we move a
-- player into their delve, the source zone raises event_disconnect, `M.on_disconnect` decides the
-- run is over, and it deletes the run bucket, the back bucket and the instance registration WHILE
-- THE PLAYER IS STILL IN THE LOADING SCREEN. World then reaches its own entry check
-- (world/client.cpp:929 VerifyInstanceAlive -> CheckInstanceByCharID), finds no membership row, and
-- redirects the player to the safe return. Measured, polling five times a second:
--     .934  instance + membership row + both buckets + task all present and correct
--     .368  membership row and BOTH buckets gone (434ms later; instance and task survive)
--     .029  world evicts to the bazaar
-- The surviving task is the fingerprint: on_disconnect is the one teardown path that deliberately
-- leaves the journal entry alone, which is how this was finally identified.
--
-- `Client::IsZoning()` exists (zone/client.h:855) but is NOT Lua-bound, so the flag stands in for it:
-- set just before the move, cleared on arrival, and treated as expired after TRANSIT_GRACE seconds so
-- a genuine camp mid-transit still tears down on the next login instead of leaking an instance.
local TRANSIT_GRACE = 60

local function set_transit(c) eq.set_data(bkey(c, "transit"), tostring(os.time())) end
local function clear_transit(c) eq.delete_data(bkey(c, "transit")) end

local function in_transit(c)
    local raw = eq.get_data(bkey(c, "transit"))
    if not raw or raw == "" then return false end
    local started = tonumber(raw)
    if not started then return false end
    return (os.time() - started) < TRANSIT_GRACE
end

-- ---------------------------------------------------------------- run history (the score sheet)
-- Every finished run is appended to `delve_hist_<charid>`, newest LAST, capped at HISTORY_MAX. This
-- is the permanent record of how hard the things you killed actually were.
--
-- One entry, '|' separated, entries separated by '^':
--   when | layer_index | kills | score | avg | lo | hi | outcome | bands
-- outcome: C cleared, F failed (died), A abandoned (walked out)
-- bands:   "45-12,50-20,55-15" -- kills per five-level band, so a run reads as a SHAPE and not just
--          an average. This is what the history tab expands to show.
--
-- ⚠️ Capped, and the OLDEST is dropped. A data bucket is one string; letting it grow unbounded would
-- eventually produce a value too long to store and a chat line too long to send (§20 records that an
-- oversized line is silently TRUNCATED, which looks like a short list rather than an error).
M.HISTORY_MAX = 20

local function hkey(c) return "delve_hist_" .. c:CharacterID() end

function M.record_history(c, layer_idx, outcome)
    if not c or not c.valid then return end
    local led = scale.ledger(c)
    if not led or led.kills <= 0 then return end   -- nothing was killed; not worth a row

    local entry = string.format("%d|%d|%d|%d|%.1f|%d|%d|%s|%s",
        os.time(), layer_idx, led.kills, led.score, led.avg, led.lo, led.hi, outcome, led.bands)

    local raw = eq.get_data(hkey(c)) or ""
    local list = {}
    for e in raw:gmatch("[^%^]+") do list[#list + 1] = e end
    list[#list + 1] = entry
    while #list > M.HISTORY_MAX do table.remove(list, 1) end   -- drop the oldest
    eq.set_data(hkey(c), table.concat(list, "^"))
end

-- DLVHIST <n>^when|level|dungeon|kills|score|avg|lo|hi|outcome|bands^...
-- ⚠️ Sent NEWEST FIRST, because that is the order the window shows it in and sorting client side
-- would mean the dll owning an ordering rule the server already knows.
-- ⚠️ The layer's NAME is resolved here rather than sent as an index: the client only ever learns the
-- names of layers it has UNLOCKED, so an old run in a layer it cannot currently see would otherwise
-- render as a blank row.
function M.send_history(c)
    if not c or not c.valid then return end
    local raw = eq.get_data(hkey(c)) or ""

    local list = {}
    for e in raw:gmatch("[^%^]+") do list[#list + 1] = e end

    local out = {}
    for i = #list, 1, -1 do
        local when, idx, kills, score, avg, lo, hi, outcome, bands =
            list[i]:match("^(%d+)|(%d+)|(%d+)|(%d+)|([%d.]+)|(%d+)|(%d+)|(%a)|(.*)$")
        if when then
            local L = M.LAYERS[tonumber(idx)]
            -- 10 fields: when, level, dungeon, kills, score, avg, lo, hi, outcome, bands
            out[#out + 1] = string.format("%s|%d|%s|%s|%s|%s|%s|%s|%s|%s",
                when, L and L.level or 0, L and L.name or "unknown",
                kills, score, avg, lo, hi, outcome, bands)
        end
    end

    c:Message(MT.NPCQuestSay, "DLVHIST " .. #out .. "^" .. table.concat(out, "^"))
end

-- ---------------------------------------------------------------- unlock state
-- A layer is offerable if it is the first, or the one below it has been cleared.
function M.unlocked_count(c)
    local n = get_cleared(c) + 1
    if n > #M.LAYERS then n = #M.LAYERS end
    return n
end

-- ---------------------------------------------------------------- the window feed
-- DUNGDATA <unlocked>^level|name|cleared^level|name|cleared^...
-- ⚠️ Only unlocked layers are sent AT ALL. The window cannot show a locked layer it was never told
-- about, and the enter path re-checks anyway, so a modified client gains nothing by asking.
function M.send_list(c)
    if not c or not c.valid then return end
    local unlocked, cleared = M.unlocked_count(c), get_cleared(c)
    local parts = {}
    for i = 1, unlocked do
        local L = M.LAYERS[i]
        parts[#parts + 1] = string.format("%d|%s|%d", L.level, L.name, i <= cleared and 1 or 0)
    end
    c:Message(MT.NPCQuestSay, "DUNGDATA " .. unlocked .. "^" .. table.concat(parts, "^"))
end

-- DUNGMODES <n>^id|name|desc^id|name|desc^...
-- ⚠️ The mode list and its descriptions come FROM THE SERVER rather than being hardcoded in the dll.
-- Same reason the reward picker reads its icon set out of the pool instead of a kIcons[] array (§3):
-- two copies drift, and the C++ one is the copy nobody remembers to update. Adding a seventh mode
-- should mean editing M.MODES and nothing else.
-- ⚠️ Per-layer availability is NOT sent here -- the client already has `cleared` for every layer from
-- DUNGDATA and gates the dropdown off that. M.enter re-checks regardless.
-- ⚠️ '|' and '^' are the field separators, so a description must contain neither. They are authored
-- in M.MODES, so this is a rule for whoever edits that table rather than something to escape here.
function M.send_modes(c)
    if not c or not c.valid then return end
    local parts = {}
    for _, m in ipairs(M.MODES) do
        parts[#parts + 1] = string.format("%s|%s|%s", m.id, m.name, m.desc)
    end
    c:Message(MT.NPCQuestSay, "DUNGMODES " .. #M.MODES .. "^" .. table.concat(parts, "^"))
end

-- ---------------------------------------------------------------- entering
-- ⚠️ ORDER IS LOAD-BEARING: validate -> create -> assign to instance -> assign task -> move.
-- Creating the instance before the checks would leak an instance every time somebody was refused,
-- and moving before AssignTask means the player can be standing in the dungeon for a frame with no
-- journal entry, which reads as the quest having failed to appear.
function M.enter(c, level, mode_id)
    if not c or not c.valid then return end

    local idx
    for i, L in ipairs(M.LAYERS) do if L.level == level then idx = i break end end
    if not idx then
        c:Message(MT.Red, "There is no delve at that level.")
        return
    end
    if idx > M.unlocked_count(c) then
        c:Message(MT.Red, "You have not cleared the delve below that one yet.")
        return
    end

    -- ⚠️ The mode is re-validated HERE, not trusted from the window. The dll only ever grays a mode
    -- out; it is the same rule as the reward picker sending an index rather than a spell id -- a
    -- modified client must not be able to open Gauntlet on a layer it has never cleared.
    -- ⚠️ Missing/unknown mode falls back to Standard rather than refusing, so an older dll that still
    -- sends `delveenter <level>` keeps working against the new server.
    local mode = M.mode_by_id(mode_id or M.MODE_DEFAULT) or M.mode_by_id(M.MODE_DEFAULT)
    -- ⚠️ Only reachable if M.MODES has no "standard" row at all, i.e. somebody edited the table and
    -- removed the default. Guarded rather than assumed because every line below dereferences `mode`,
    -- and a nil here would be an "attempt to index a nil value" thrown from inside the Enter button.
    if not mode then
        c:Message(MT.Red, "Delve difficulties are misconfigured on this server.")
        return
    end
    if not M.mode_allowed(c, idx, mode) then
        c:Message(MT.Red, string.format(
            "%s is not open here yet -- clear this delve on Standard first.", mode.name))
        return
    end
    if M.current_run(c) then
        c:Message(MT.Red, "You are already inside a delve. Finish it or use Exit first.")
        return
    end

    local L = M.LAYERS[idx]

    -- Remember where they came from BEFORE anything else, so both Exit and the DZ's own safe return
    -- can put them back there.
    -- ⚠️ Store the zone ID, not the short name. `eq.get_zone_id()` takes NO ARGUMENTS -- it returns
    -- the CURRENT zone -- so the obvious `eq.get_zone_id(name)` on the way back out is a hard Lua
    -- error ("int get_zone_id()", stack traceback, exit silently fails). There is an
    -- `eq.get_zone_id_by_name()`, but recording the id here means no lookup is needed at all.
    eq.set_data(bkey(c, "back"), string.format("%d|%d|%d|%d",
        eq.get_zone_id(), math.floor(c:GetX()), math.floor(c:GetY()), math.floor(c:GetZ())))

    -- ⚠️⚠️⚠️ A PLAIN INSTANCE, NOT AN EXPEDITION/DZ. `c:CreateExpedition(...)` was tried and it
    -- CANNOT WORK for this design. Do not "restore" it for the compass and the Ctrl+Z entry.
    --
    -- World reaps a dynamic zone the moment it looks empty (world/dynamic_zone.cpp:104):
    --     if (!HasMembers() || IsExpired()) {
    --         auto dz_zoneserver = ZSList::Instance()->FindByInstanceID(GetInstanceID());
    --         if (!dz_zoneserver || dz_zoneserver->NumPlayers() == 0)   // no clients inside dz
    --             status = DynamicZoneStatus::ExpiredEmpty;             // -> deleted
    --     }
    -- We create the DZ and zone into it IMMEDIATELY, so for the second or so the player is in
    -- transit there is nobody inside the instance yet and world deletes the DZ underneath them --
    -- taking the `instance_list_player` rows with it. World then reaches its own entry check
    -- (world/client.cpp:929 `VerifyInstanceAlive` -> `CheckInstanceByCharID`), finds no membership
    -- row, and sends the player to the safe return instead. Measured on a live attempt, polling the
    -- DB four times a second:
    --     .957  instance + member row + dz created
    --     .225  character saved into the delve
    --     .496  DZ and member row DELETED   (~270ms later, instance_list survives)
    --     .296  world evicts to the bazaar
    -- ⚠️ THE RACE IS CAUSED BY MOVING IMMEDIATELY, and that is worth being precise about: expeditions
    -- are NOT broken on this build. AoTv3's Progressive Dungeons uses them successfully
    -- (server/quests/draniksscar/302093.lua) -- but it creates the expedition and then STOPS, telling
    -- the player "Click the dungeon portal to enter your active mission." Entry is a separate act,
    -- seconds later, through the engine's own DZ portal. The expedition sits with a member attached,
    -- so world never sees it memberless. Live EQ behaves the same way: make it from the window, enter
    -- afterwards. We create and zone in one breath, which tears the source-zone Client down mid
    -- handshake -- before world's copy of the DZ has its member list -- and world reaps it as empty.
    --
    -- So a DZ is recoverable IF the move is decoupled from creation (a portal, a confirm step, or a
    -- deferred timer). That is a design change, not a bug fix: this ladder's whole point is that the
    -- window's Enter button puts you in the dungeon. Plain instances give that immediately, so they
    -- are the right trade here. Revisit only if the Ctrl+Z entry and compass are wanted enough to
    -- add a second step.
    --
    -- The plain-instance path below is exactly what the stock `#instance create` + `#instance add` +
    -- `#zoneinstance` sequence does, and that sequence was verified working in game on this build.
    -- Cost of dropping the DZ: no Ctrl+Z entry, no compass arrow, no engine-managed safe return. The
    -- run bucket already records where to send people back to, so exit is unaffected.
    --
    -- ⚠️ A FRESH INSTANCE EVERY TIME (never `eq.get_instance_id`) -- reusing one would hand back a
    -- dungeon still full of the last run's corpses and an emptied spawn table.
    local inst = eq.create_instance(L.zone, L.version, M.DURATION_SECS)
    if not inst or inst == 0 then
        c:Message(MT.Red, "The delve could not be opened. Try again shortly.")
        return
    end

    set_run(c, idx, inst, L.zone, mode.id)

    -- ⚠️⚠️ REGISTER THE CHARACTER ON THE INSTANCE EXPLICITLY. Without this the client is told to zone,
    -- world records the destination, the delve zone boots and approves the transfer -- and then the
    -- client NEVER RECONNECTS TO WORLD to collect the address. It hangs on the loading screen forever,
    -- with no error and nothing further in any log. That was the second silent failure of this entry
    -- path, and it looks nothing like a permissions problem from the outside.
    --
    -- The stock `#zoneinstance` command does exactly this before its move
    -- (zone/gm_commands/zone_instance.cpp: `if (!CheckInstanceByCharID(...)) AddClientToInstance(...)`)
    -- and that command demonstrably works on this build, moving through the SAME
    -- `MovePC(zone, instance, x, y, z, 0, ZoneSolicited)` call we use. Registration was the only
    -- difference between the two paths.
    --
    -- ⚠️ With the DZ gone this is the ONLY thing that registers the character on the instance --
    -- `eq.create_instance` creates the instance but adds nobody to it. It is not optional and it has
    -- no fallback: without it every entry ends in an eviction to the safe return.
    -- ⚠️ Argument order is (instance_id, character_id), NOT the reverse.
    eq.assign_to_instance_by_char_id(inst, c:CharacterID())

    -- ⚠️ Wipe the difficulty ledger BEFORE entering. It is per run, and a stale one would credit this
    -- run with the kills of the last -- which is exactly the accounting a gear-swapper wants.
    scale.reset_ledger(c)

    -- The journal entry. ⚠️ enforce_level_requirement = false: the task's min_level tiers exist so it
    -- READS like a native DoN mission, but the unlock chain is the real gate and a level 45 character
    -- who has cleared layer 1 must still be able to take layer 2.
    -- ⚠️ The MODE picks the task row. Each mode is a parallel family over the same six zones
    -- (`L.task + mode.taskoff`) because goalcount and duration are static DB columns -- see M.MODES.
    -- ⚠️ Clear ANY delve task, not just this mode's: switching modes between runs would otherwise
    -- leave the previous mode's journal entry sitting alongside the new one.
    local task_id = M.task_for(L, mode)
    for _, m in ipairs(M.MODES) do
        local t = M.task_for(L, m)
        if c:IsTaskActive(t) then c:RemoveTaskByTaskID(t) end
    end
    c:AssignTask(task_id, 0, false)

    if mode.id == M.MODE_DEFAULT then
        c:Message(MT.Yellow, string.format("The way into %s opens before you.", L.name))
    else
        c:Message(MT.Yellow, string.format("The way into %s opens before you -- %s.", L.name, mode.name))
    end

    -- ⚠️⚠️ MovePCInstance, NOT MovePCDynamicZone -- AND THAT IS NOT A STYLE CHOICE. Using the DZ
    -- form here is a SILENT NO-OP, which is exactly how it failed on the first live run: the DZ was
    -- created, showed up in Ctrl+Z with a working compass, and the player was simply never moved,
    -- with no error and no log line.
    --
    -- The reason is that DZ MEMBERSHIP IS NOT SET SYNCHRONOUSLY. `CreateExpedition` ends in
    -- `DynamicZone::TryCreate` -> `dz->UpdateMembers()`, and for a brand-new DZ `m_has_member_statuses`
    -- is false, so that function ships a `ServerOP_DzGetMemberStatuses` packet to WORLD and returns
    -- (zone/dynamic_zone.cpp:1297). Only when world REPLIES, a tick or more later, does
    -- `SendUpdatesToZoneMembers` run `client->AddDynamicZoneID(...)` and populate the client's
    -- `m_dynamic_zone_ids`. So on the very next line of Lua that list is still EMPTY.
    -- `Client::MovePCDynamicZone` looks the player's DZs up in exactly that list, finds nothing, and
    -- -- because `msg_if_invalid` DEFAULTS TO FALSE on this codebase (zone/client.h:1689) -- returns
    -- without moving anyone and without saying a word. `MovePC` is never reached, so not even its
    -- unconditional LogInfo appears. Invisible to every check we have.
    --
    -- MovePCInstance goes straight to (zone, instance) with no membership lookup, and it is the SAME
    -- move: its binding calls `MovePC(zone, instance, x, y, z, h)`, whose default zone mode is
    -- `ZoneSolicited` (zone/client.h:832) -- the identical mode `MovePCDynamicZone` would have used.
    -- Nothing about the DZ is lost: membership lives in the DB, the compass and safe return are
    -- already set on the dz object above, and `IsCurrentZoneDz` matches on zone id + instance id, so
    -- the player still arrives as a member of a DZ the client knows about.
    --
    -- ⚠️ Deferring the DZ call behind a timer instead would also "work" and is the wrong fix: it
    -- races the world round-trip, and there is no upper bound on that reply.
    -- ⚠️ MUST be set before the move: the move is what raises event_disconnect in this zone.
    set_transit(c)
    c:MovePCInstance(L.zoneid, inst, L.x, L.y, L.z, L.h or 0)
end

-- ---------------------------------------------------------------- scaling
-- Called from global_npc.event_spawn for every NPC that spawns anywhere. Scales it only if it is
-- spawning inside a live delve instance.
--
-- ⚠️ THIS RUNS FOR EVERY NPC SPAWN ON THE SERVER, so the cheap early-outs come first: the instance
-- id test rejects the entire normal world before anything else is looked at.
-- ⚠️ The chest is skipped explicitly -- scaling it to level 70 would give it 70th-level HP and turn
-- the reward into a fight.
function M.on_npc_spawn(e)
    local inst = eq.get_zone_instance_id()
    if not inst or inst == 0 then return end

    local L = M.layer_for_zone()
    if not L then return end

    local npc = e.self
    if not npc or not npc.valid then return end
    -- ⚠️ The chest AND the boss are both excluded from everything below. The boss is spawned by
    -- M.spawn_boss, which scales it itself -- and if it were not excluded from the thinning below it
    -- could be DESPAWNED THE INSTANT IT ARRIVED, making Gauntlet unfinishable at random.
    local npcid = npc:GetNPCTypeID()
    if npcid == M.CHEST_NPC or npcid == M.BOSS_NPC then return end

    -- ⚠️⚠️ STRIP THE STOCK LOOT. These are Dragons of Norrath zones, so the mobs carry DoN drops --
    -- an expansion the server does not even have unlocked (era caps at OoW, section 12). A level 3
    -- delve was handing out gear from six expansions past anything else in the game, which made the
    -- delve the best source of loot in the world for reasons nobody designed.
    -- ⚠️ The REWARD is the chest: an evolving augment and coin, both scaled to the rung. Trash drops
    -- would compete with that and drown it.
    -- ⚠️ Done HERE rather than by editing loottable rows, because these are instances of REAL zones
    -- and npc_types/loottable rows are shared with the open world -- stripping them in the database
    -- would empty those zones for everyone, everywhere, forever.
    -- ⚠️ Placed before the thinning and reference-client early-returns below on purpose: a mob that
    -- spawns while the instance is momentarily empty must still lose its loot.
    npc:ClearItemList()
    npc:RemoveCash()

    -- Scaled to the PLAYER, not just to the layer. The layer level is the floor; how far above the
    -- naked expectation for their level the player is decides the rest (aotv4_dungeon_scale.lua).
    -- ⚠️ Whoever is in the instance is the reference. With one occupant that is unambiguous; with a
    -- group it takes the FIRST client found, which is a known simplification -- see section 24.
    local ref
    local clients = eq.get_entity_list():GetClientList()
    for cl in clients.entries do
        if cl and cl.valid then ref = cl break end
    end

    -- ⚠️⚠️ AN EMPTY INSTANCE MUST STILL BE SCALED, TO THE LAYER. This used to `return` here, and that
    -- was the single worst bug in the delve: the instance BOOTS AND POPULATES BEFORE THE PLAYER
    -- FINISHES ZONING IN, so on a cold instance there is no reference client for most of the spawn
    -- wave and those mobs were left at their NATIVE Dragons of Norrath level of 60-70. In a level 3
    -- delve that is a one-shot for anything that walks into it, and it fed the score ledger
    -- end-game numbers (see the fallback in scale.record_kill).
    -- ⚠️ Scaling to the layer with power 1.0 is the SAFE default, not a guess at the player: the rung
    -- is a known quantity and the sweep in M.rescale_zone refines it to the real player within a
    -- tick of them arriving. The rule is that a delve mob is NEVER left at its native level.
    if not ref then
        scale.apply(npc, L.level, 1.0)
        return
    end

    -- ⚠️ Read the run's mode BEFORE any scaling: thinning has to happen first, and the multipliers
    -- below need it too. Read from the reference client's run, not a module level "current mode" --
    -- several instances of different modes run at once on one zone process.
    local run  = M.current_run(ref)
    local mode = run and run.modedef or nil

    -- ⚠️⚠️ THINNING RUNS BEFORE SCALING, and returns. Scaling a mob we are about to despawn is wasted
    -- work on every spawn in the zone, and Depop after ScaleNPC would briefly show the mob.
    -- ⚠️ Trash only -- the chest and boss returned above. Depopping the boss would make the mode
    -- impossible to finish, which is the one failure the player could never diagnose.
    -- ⚠️ The kill goal must stay reachable: delvea has ~154 spawn points and Gauntlet wants 10 kills,
    -- so at 75 percent thinned there are still ~38 left. Any future mode raising `thin` toward 1.0
    -- has to be checked against its own goalcount or the run becomes uncompletable.
    if mode and (mode.thin or 0) > 0 and math.random() < mode.thin then
        npc:Depop()
        return
    end

    local eff, power = scale.effective_level(L.level, ref)
    scale.apply(npc:CastToNPC(), eff, power)

    -- ⚠️⚠️ MODE MULTIPLIERS GO LAST, AFTER scale.apply -- which calls ScaleNPC, and ScaleNPC rewrites
    -- stats wholesale from npc_scale_global_base. Anything applied before it is silently discarded.
    -- This is the same ordering rule already recorded for ModifyNPCStat in §24.
    if mode then
        local n = npc:CastToNPC()
        if (mode.hp or 1) ~= 1 then
            local hp = n:GetMaxHP() * mode.hp
            if hp < 1 then hp = 1 end
            n:ModifyNPCStat("max_hp", tostring(math.floor(hp)))
            n:SetHP(math.floor(hp))    -- or it spawns at the OLD hp and reads as damaged
        end
        if (mode.dmg or 1) ~= 1 then
            n:ModifyNPCStat("min_hit", tostring(math.floor(n:GetMinDMG() * mode.dmg)))
            n:ModifyNPCStat("max_hit", tostring(math.floor(n:GetMaxDMG() * mode.dmg)))
        end
    end
end

-- Which layer is the zone we are currently running in? Identified by SHORT NAME rather than by
-- instance id: the instance was created from a known zone, so the name is sufficient, and it means
-- the mapping survives a zone restart without needing a bucket to remember it.
-- ⚠️ Callers must already have established that this is an instance -- on its own this would also
-- match the ordinary open-world copy of the zone.
function M.layer_for_zone()
    local zone = eq.get_zone_short_name()
    for _, L in ipairs(M.LAYERS) do
        if L.zone == zone then return L end
    end
    return nil
end

-- ---------------------------------------------------------------- the boss
-- ⚠️⚠️ WITHOUT THIS THE DELVE CANNOT BE FINISHED. Every task carries a second activity (step 2, kill
-- npc 2000301) that the task system only opens once the trash goal is met -- so if nothing spawns the
-- boss, the player clears the dungeon and then stands in an empty instance with an objective that can
-- never tick. The step ordering is native; the SPAWNING is ours and is not optional.
--
-- Fired from global_player.event_task_stage_complete. ⚠️ That event has a real argument builder
-- (lua_parser.cpp registers handle_player_task_stage_complete) and gives `task_id` + `activity_id` --
-- checked before relying on it, because an unregistered spell/player event silently hands back an
-- EMPTY table rather than erroring (§10).
--
-- ⚠️ "Randomly generated" is done by varying what we do to ONE npc row, never by picking a different
-- row: `task_activities.npc_match_list` is static, so the task can only ever match a fixed npc id.
-- ⚠️ A GIVEN NAME plus a title, never the dungeon's name plus a title. Building it from `L.name`
-- produced "StillmoonAscent the Devourer": the zone names contain spaces and apostrophes, which have
-- to be stripped because the client renders underscores as spaces, and what survives is a run
-- together word that reads like a bug. A proper noun is also just what a named mob looks like in EQ.
-- ⚠️ ASCII ONLY, and one word each. TempName goes onto the wire as a mob name and underscores render
-- as spaces, so anything else here shows up mangled in game rather than failing visibly.
local BOSS_NAMES = {
    "Kaeloth", "Vaskar", "Morthain", "Zhurel", "Draegoth", "Ilvaren", "Xanthos", "Thelgrin",
    "Ruvaak", "Sythrel", "Ombrach", "Kelvorn", "Nazareth", "Ythera", "Grimvault", "Aszhalor",
}
local BOSS_TITLES = {
    "the Gorged", "the Unbroken", "Bloodmaw", "the Hollow", "Rendfang", "the Sunless",
    "Ashjaw", "the Patient", "Gravewind", "the Devourer", "Nightscale", "the Unfed",
}

-- ⚠️ Scaled to the SAME effective level as the trash, then made a real fight on top. That split is
-- deliberate: matching the trash level keeps the score sheet honest (the ledger banks effective level
-- per kill, and the '#' name already pays the 3x named multiplier), while the HP and damage
-- multipliers supply the named-versus-trash difference the player actually feels.
-- ⚠️⚠️ ADD A RACE HERE ONLY IF IT IS ALREADY LOADED IN EVERY ZONE. There is no client file to edit
-- and no archive to ship -- see the selection rule below. A race that is not globally loaded renders
-- as a humanoid placeholder, and it fails ONLY on the client: the server reports the correct race
-- the whole time and nothing appears in any log.
--
-- ⚠️⚠️ EVERY RACE HERE IS POST DRAGONS OF NORRATH and is force loaded by the client's
-- Resources\GlobalLoad_chr.txt. That file and this list are ONE UNIT: a race added here whose archive
-- is not listed there renders as a HUMANOID PLACEHOLDER, and it fails only on the client -- the
-- server reports the right race the whole time and nothing appears in any log.
--
-- ⚠️⚠️ ONLY EXPANSION 10-11 ARCHIVES WORK. This was established by testing, not assumed, and it is
-- the single most expensive thing learned here. A first attempt listed five archives from expansions
-- 13-19 (sepulcher, beastdomain, pillarsalra, shardslanding, thulehouse2); race 700 was present in
-- THREE of them and still drew as a humanoid every time. Retested with expansion 10-11 archives and
-- races 461, 496 and 488 all rendered correctly on the first try.
-- The reason is almost certainly the archive FORMAT: the stock 13 entries are all classic S3D era
-- globals, expansions 10-11 are the last of the S3D zones, and expansion 13+ zones ship as EQG --
-- which this loader appears not to handle. It fails SILENTLY, so a bad entry looks exactly like a
-- correct one.
-- 📌 DO NOT ADD AN EXPANSION 12+ ARCHIVE without spawning one of its races via `/say delveboss <race>`
-- first. That command exists for precisely this test.
--
-- Archives currently listed in GlobalLoad_chr.txt and the races each one buys:
--     illsalin_chr      exp 10   460, 461, 462, 465, 467
--     elddar_chr        exp 11   473, 489, 490, 493, 494, 496, 519
--     theater_chr       exp 11   473, 488, 491, 494, 496, 497, 518, 519
--     devastation_chr   exp 11   461, 467, 469, 486, 500
--     westkorlachc_chr  exp 10   461, 465, 467, 474
-- ⚠️ None of these are 431/432/433/450, the only races the DoN delve zones use, so every warden is
-- visually unique inside a delve as well as being from a later era than the dungeon itself.
-- ⚠️ This is the exact union of the five archives above, computed from the DB rather than typed by
-- hand. 470 was in an earlier draft and is NOT here: it comes from corathus_chr, which is not listed,
-- so it would have drawn as a humanoid roughly one warden in twenty -- rare enough to look like bad
-- luck rather than a bug. If an archive is added or removed, recompute this list, do not edit it.
M.BOSS_RACES = {
    460, 461, 462, 465, 467, 469, 473, 474, 486, 488,
    489, 490, 491, 493, 494, 496, 497, 500, 518, 519,
}

-- 📌 Tuning. The multipliers are the real shape; the floors only bite at the bottom of the ladder,
-- where ScaleNPC's own numbers are too small for a multiplier to mean anything. Raise the floors if
-- early wardens still die too fast; they stop applying once ScaleNPC overtakes them.
M.BOSS_HP_FLOOR_PER_LEVEL  = 60   -- level 3 warden floors at 180 hp instead of 13
M.BOSS_DMG_FLOOR_PER_LEVEL = 3    -- and hits for up to 9 instead of 1

M.BOSS_HP_MULT  = 5.0
M.BOSS_DMG_MULT = 1.6
M.BOSS_LVL_BONUS = 2

function M.spawn_boss(c, L, run, ordinal)
    if not c or not c.valid then return end

    local boss = eq.spawn2(M.BOSS_NPC, 0, 0, c:GetX(), c:GetY(), c:GetZ(), c:GetHeading())
    if not boss or not boss.valid then return end

    local mode = run.modedef or M.mode_by_id(M.MODE_DEFAULT)
    local eff  = scale.effective_level(L.level, c)

    -- ⚠️ ScaleNPC FIRST, always. It rewrites stats wholesale from npc_scale_global_base and would
    -- discard anything applied before it (§24).
    -- ⚠️⚠️ FLOORED AT THE RUNG'S OWN LEVEL, not just at 1. `eff` is the PLAYER RELATIVE level and can
    -- come out BELOW the rung -- or negative -- for an undergeared character: at layer 1 a power of
    -- 0.83 gives eff = 1 + 12*(0.83-1) = -1, which clamped to 1 and produced a level 1 warden with
    -- THIRTEEN hit points. Proportionally that was still 5x the trash, and completely meaningless as
    -- a boss. The rung is the floor because a warden should never be weaker than the dungeon it is
    -- guarding; the power adjustment can only ever make it harder, never softer.
    local blvl = math.max(eff, L.level) + M.BOSS_LVL_BONUS
    if blvl < 1 then blvl = 1 end
    boss:CastToNPC():ScaleNPC(blvl)

    -- Then the boss multipliers, then the MODE multipliers on top of those, so Hard's boss is twice
    -- the boss rather than twice the trash.
    -- ⚠️ GetMinDMG / GetMaxDMG are the no-argument accessors. `GetMaxDamage` also exists on Lua_NPC
    -- but takes an int and is a DIFFERENT function, and there is no `GetMinDamage` binding at all --
    -- calling the wrong one is a runtime nil, not a compile error.
    local npc = boss:CastToNPC()
    local hp  = npc:GetMaxHP()  * M.BOSS_HP_MULT  * (mode.hp  or 1)
    local lo  = npc:GetMinDMG() * M.BOSS_DMG_MULT * (mode.dmg or 1)
    local hi  = npc:GetMaxDMG() * M.BOSS_DMG_MULT * (mode.dmg or 1)

    -- ⚠️ ABSOLUTE FLOORS, because a MULTIPLIER OF A TINY NUMBER IS STILL TINY. The level floor above
    -- fixes most of this, but ScaleNPC at the bottom of the ladder still yields single digit hp and
    -- 1 point hits, and 5x of that is not a fight. Scaled per level so they fade out naturally --
    -- by the middle of the ladder ScaleNPC already exceeds them and these never apply.
    -- ⚠️ Applied AFTER the mode multipliers, so Hard and Fragile still move a floored boss; putting
    -- them before would let the floor erase the mode's effect entirely at low rungs.
    local hp_floor = blvl * M.BOSS_HP_FLOOR_PER_LEVEL
    local hi_floor = blvl * M.BOSS_DMG_FLOOR_PER_LEVEL
    if hp < hp_floor then hp = hp_floor end
    if hi < hi_floor then hi = hi_floor end
    if lo < hi * 0.4 then lo = hi * 0.4 end   -- keep the spread sane rather than 1-to-N

    npc:ModifyNPCStat("max_hp",  tostring(math.floor(hp)))
    npc:ModifyNPCStat("min_hit", tostring(math.floor(lo)))
    npc:ModifyNPCStat("max_hit", tostring(math.floor(hi)))
    npc:SetHP(math.floor(hp))

    -- ⚠️ The '#' prefix is load-bearing and must survive the rename: the ledger's named test is the
    -- same lowercase-is-trash heuristic used in §17c and quest_difficulty.pl, and a named kill is
    -- worth 3x in M.kill_value. A lowercase boss name silently scores as a rat.
    -- ⚠️ The leading '#' is load bearing and must survive any change here: the ledger's named test is
    -- the lowercase-is-trash heuristic from §17c, and a named kill is worth 3x in M.kill_value.
    -- ⚠️ Underscores become spaces client side, which is why the title's spaces are converted rather
    -- than left as-is.
    local title = BOSS_TITLES[math.random(#BOSS_TITLES)]
    local given = BOSS_NAMES[math.random(#BOSS_NAMES)]
    boss:TempName("#" .. given .. "_" .. title:gsub("%s", "_"))

    -- ⚠️⚠️ THE BOSS MUST WEAR A RACE THE ZONE ALREADY LOADS, or it renders as a placeholder -- an
    -- untextured model, or a generic human. The client only loads character models present in that
    -- zone's own _chr archive; a race that never spawns there HAS NO MODEL TO DRAW, and there is no
    -- server-side way to change that. It is a client asset decision, not a DB flag: the only lever is
    -- globalload.txt in the EQ folder, and force-loading every race in every zone costs memory
    -- everywhere to fix one mob.
    --
    -- The boss row is cloned from a Frostgiant template (race 188), which no delve zone spawns --
    -- exactly how this broke. The race is taken from the DUNGEON instead, which reads better anyway:
    -- the warden looks like a bigger, angrier version of whatever lives there rather than a frost
    -- giant standing in a lava cave.
    --
    -- ⚠️ The race comes from M.BOSS_RACES, which is restricted to models the client ALREADY loads in
    -- every zone -- see the selection rule and the failed GlobalLoad_chr.txt attempt documented up
    -- there. None of them is a race the DoN delve zones use, so a warden is still visually unique
    -- inside a delve without needing any client file at all.
    -- ⚠️⚠️ IT TAKES **BOTH** CALLS, AND SetRace IS THE ONE THAT MATTERS. `ChangeRace` sounds like the
    -- real one and is not -- it is a one line inline setter that BROADCASTS NOTHING:
    --     inline void ChangeRace(uint16 in) { race = in; }        zone/mob.h:588
    -- The spawn packet has already gone out by the time we get here, so on its own the server thinks
    -- the boss is race 496 while every client in the zone is still drawing the npc_types race (188,
    -- the Frostgiant clone template) -- which has no model in a delve, so it renders as a HUMAN.
    -- That is the bug this replaced, and it is invisible server side: GetRace() reports the new race
    -- quite happily while the player looks at something else entirely.
    -- `SetRace` is the opposite of what its name suggests too -- it sends an illusion packet
    -- (Lua_Mob::SetRace -> SendIllusionPacket), which is exactly the appearance broadcast needed.
    -- ChangeRace is kept so the SERVER side value agrees with what is drawn; SetRace makes clients
    -- actually draw it. Dropping either one leaves the two out of step.
    local race = M.BOSS_RACES[math.random(#M.BOSS_RACES)]
    if race and race > 0 then
        boss:ChangeRace(race)   -- server side truth
        boss:SetRace(race)      -- tells the clients; without this nothing visibly changes
    end

    -- ⚠️ DIAGNOSTIC, GM ONLY. A boss that renders as a plain human looks EXACTLY the same whether the
    -- race change never happened or the race is set correctly but the client has no model for it --
    -- and the second is by far the more likely, because the model only exists if that race's archive
    -- globally loaded. Reporting the id server side is the only way to tell
    -- those two apart from in game. Remove once the model set is settled.
    if c:GetGMStatus() > 0 then
        c:Message(MT.LightBlue, string.format(
            "[delve] warden race %d. If it renders as a humanoid, that race is NOT globally loaded and must come out of M.BOSS_RACES.",
            boss:GetRace()))
    end

    -- ⚠️ Size is re-applied AFTER the race change: every race carries its own default model scale, so
    -- changing race leaves the warden whatever size that race normally is -- i.e. the same size as the
    -- trash it is supposed to tower over.
    -- ⚠️ ChangeSize, not SetSize. `SetSize` is NOT bound on Lua_Mob at all (only on Lua_Door), so it
    -- would be a runtime nil rather than a compile error.
    boss:ChangeSize(8)

    -- ⚠️ Nimbus 412 is the ONE id proven to work in this codebase (guildhall/#Healer.lua); a wrong
    -- nimbus id fails silently. The boss lands wherever the last trash died, which can be a corner.
    boss:AddNimbusEffect(412)

    if (mode.bosses or 1) > 1 then
        c:Message(MT.Red, string.format("%s of the delve answers your intrusion. (%d of %d)",
            title, ordinal or 1, mode.bosses))
    else
        c:Message(MT.Red, string.format("Something far worse than the rest stirs: %s.", title))
    end
end

-- Trash goal met -> the boss arrives. ⚠️ activity_id 0 is the trash step; activity 1 IS the boss and
-- must not re-trigger a spawn when its own stage completes.
function M.on_task_stage_complete(e)
    local c = e.self
    if not c or not c.valid then return end
    if e.activity_id ~= 0 then return end

    local run = M.current_run(c)
    if not run then return end
    local L = M.LAYERS[run.layer]
    if not L or e.task_id ~= M.task_for(L, run.modedef) then return end

    M.spawn_boss(c, L, run, 1)
end

-- ---------------------------------------------------------------- completion
-- Called from global_player.event_task_complete. Drops the chest exactly where the player was
-- standing when the last objective ticked over -- "where we took the last action to finish it".
function M.on_task_complete(e)
    local c = e.self
    if not c or not c.valid then return end

    local run = M.current_run(c)
    if not run then return end

    local L = M.LAYERS[run.layer]
    -- ⚠️ Compare against the task for the run's MODE, not the layer's base task -- each mode is its
    -- own task family, so a Hard run completes task base+10 and would never match L.task.
    if not L or e.task_id ~= M.task_for(L, run.modedef) then return end

    set_cleared(c, run.layer)

    -- ⚠️ The sigil arrives ON THE CURSOR, so the run that earns it does NOT feed it -- M.award_charm
    -- reads the charm slot, and the native system only accrues on equipped items anyway. The first
    -- delve buys the charm; the second is the first one that grows it. That is intentional and worth
    -- knowing before someone reports the first run as "not counting".
    M.grant_sigil_if_first(c)

    c:Message(MT.Yellow, string.format("%s is cleared.", L.name))

    -- ⚠️⚠️ THE ACCOUNTING RUNS BEFORE THE CHEST, AND THAT ORDER IS DELIBERATE. A Lua error anywhere
    -- in this handler aborts the REST of it, and on 2026-07-30 a bad call while stocking the chest
    -- did exactly that: the run was cleared and the chest spawned, but the score award, the history
    -- row, the unlock and the list refresh all never ran -- a completed delve paid nothing and left
    -- no record. Everything irreplaceable is therefore settled first; the chest is the last thing
    -- touched, because it is the only part a player can still be handed on a later run.
    --
    -- What the run was actually worth. ⚠️ AVERAGE, not final gear and not the peak: clearing the
    -- dungeon stripped and then putting gear on shows up here as a low average with a high maximum.
    local led = scale.ledger(c)
    if led and led.kills > 0 then
        c:Message(MT.Yellow, string.format("Score: %d", led.score))
        c:Message(MT.Yellow, string.format(
            "  %d creatures%s, average level %.1f (weakest %d, strongest %d).",
            led.kills, led.named > 0 and string.format(" including %d named", led.named) or "",
            led.avg, led.lo, led.hi))
        if led.hi - led.lo >= 8 then
            c:Message(MT.Yellow,
                "Your power swung a long way during that run; the score follows what you actually fought.")
        end

        -- ⚠️ Evolving gear is fed HERE, on the clear, and deliberately NOT on death or abandon. The
        -- score sheet records those outcomes because a failed run still says something about what
        -- you fought, but this is a reward for FINISHING -- paying out on a wipe would make farming
        -- a partial run the efficient way to grow it.
        -- ⚠️ The SIGIL is the only thing delve score feeds. Augments are ordinary items now and are
        -- upgraded by combining four of a tier in the Refining Crucible, so there is nothing to
        -- split the score between any more.
        M.award_charm(c, led.score)
    end

    -- ⚠️ Recorded here, on the CLEAR, and again on death/abandon in their own paths -- never in one
    -- shared place, because the outcome letter differs and it is the outcome that makes the row
    -- meaningful. M.record_history no-ops on a run with no kills.
    M.record_history(c, run.layer, "C")
    if run.layer < #M.LAYERS then
        c:Message(MT.Yellow, string.format("The delve at level %d is now open to you.",
            M.LAYERS[run.layer + 1].level))
    end
    M.send_list(c)

    -- ⚠️ Spawned at the PLAYER, not at a spawn2 row: the finish position is wherever the last kill
    -- happened and cannot be known in advance. runspeed 0 on the npc keeps it there.
    -- ⚠️ No AddLooter call: that binding is on Lua_CORPSE, not Lua_NPC, so it cannot be applied to a
    -- living chest -- the reward is placed on the chest NPC by M.stock_chest below, so it arrives
    -- through the ordinary loot path once the chest is opened.
    local chest = eq.spawn2(M.CHEST_NPC, 0, 0, c:GetX(), c:GetY(), c:GetZ(), c:GetHeading())

    -- ⚠️ MAKE IT OBVIOUS. It lands wherever the last blow happened -- a corridor, a corner, on top of
    -- a corpse pile -- and a chest on the floor of a dark DoN interior is genuinely easy to walk past.
    -- The npc row already carries the gilded texture, double size and a light source; the nimbus is
    -- the part that has to be applied at runtime.
    -- ⚠️ Nimbus 412 is used because it is the ONE id proven to work in this codebase
    -- (guildhall/#Healer.lua). Any other value is a guess, and a wrong nimbus id fails silently.
    if chest and chest.valid then
        chest:AddNimbusEffect(412)
        M.stock_chest(c, chest, run, L)
    end

    -- Say where it is, for the case where it still ends up behind something.
    c:Message(MT.Yellow, "A heavy, glowing chest settles where you struck the last blow.")
end

-- ---------------------------------------------------------------- leaving
-- One path out, whether it was the chest or the Exit button, so the teardown cannot diverge.
function M.leave(c, reason)
    if not c or not c.valid then return end
    local run = M.current_run(c)
    if not run then
        c:Message(MT.Red, "You are not inside a delve.")
        return
    end

    local L = M.LAYERS[run.layer]

    -- ⚠️ Kill the close countdown on EVERY way out, not just the chest's. Exit, death and the
    -- zone-line failure all funnel through here, and a surviving "delveclose" timer would fire
    -- minutes later against a character standing in the ordinary world. M.on_close_timer re-checks
    -- the run and would no-op, but a timer that outlives its run is a trap waiting for the next
    -- person to add a side effect to that handler.
    c:StopTimer("delveclose")

    -- ⚠️ Drop the unfinished journal entry. Leaving early with the task still live would leave a
    -- permanent "Delve: ..." row in the journal for a dungeon that no longer exists.
    -- RemoveTaskByTaskID (custom, lua_client.cpp) clears the DB rows too, unlike CancelAllTasks.
    -- ⚠️ ASK FIRST, THEN REMOVE. A live task here means the run was ABANDONED; looting the chest also
    -- routes through M.leave, but by then on_task_complete has finished the task and already written
    -- its "C" row. Testing after the removal would always read false and never record an abandon at
    -- all -- and testing without the removal ordering in mind would double-count every clear.
    local run_task = L and M.task_for(L, run.modedef) or nil
    local abandoned = (run_task and c:IsTaskActive(run_task)) and true or false
    if abandoned then c:RemoveTaskByTaskID(run_task) end
    if abandoned then M.record_history(c, run.layer, "A") end

    local back = eq.get_data(bkey(c, "back")) or ""
    local zid, x, y, z = back:match("^(%d+)|(-?%d+)|(-?%d+)|(-?%d+)$")

    clear_run(c)
    eq.delete_data(bkey(c, "back"))

    c:Message(MT.Yellow, reason or "You step back out of the delve.")

    -- ⚠️ There must ALWAYS be somewhere to go: the instance is about to be destroyed, and a player
    -- left standing in a destroyed instance is stuck in a zone that no longer exists. The bind point
    -- is the fallback (there is no MoveToSafeCoords binding on Lua_Client -- only MovePC and the
    -- MoveZone family).
    if zid then
        c:MovePC(tonumber(zid), tonumber(x), tonumber(y), tonumber(z), 0)
    else
        c:MovePC(c:GetBindZoneID(), c:GetBindX(), c:GetBindY(), c:GetBindZ(), c:GetBindHeading())
    end

    -- ⚠️ Destroy LAST, after the player is out. Destroying an instance somebody is standing in is
    -- how you get a client stuck in a zone that no longer exists.
    if run.instance and run.instance > 0 then
        eq.destroy_instance(run.instance)
    end
end

-- ⚠️⚠️ HOW LONG THE DELVE STAYS OPEN AFTER THE CHEST IS BROKEN OPEN. This is not a nicety: the
-- reward is ON THE CORPSE now (M.stock_chest), and this hook fires on the chest's DEATH -- i.e. the
-- instant the corpse appears and before anybody could possibly have looted it. Closing here, which
-- is what it used to do, destroyed the instance out from under the player and took the augment and
-- the platinum with it. The run's actual reward was unobtainable.
M.CLOSE_DELAY_SECS = 120

-- Chest opened -> start the countdown. Called from global_npc on the chest's death.
-- ⚠️ The augment and coin are NOT granted here; they were placed on the chest at spawn so they
-- arrive by ordinary looting. This hook only schedules the close.
function M.on_chest_looted(c)
    if not c or not c.valid then return end
    if not M.current_run(c) then return end

    -- ⚠️ A CLIENT timer, not an NPC one: the chest is dead, and timers on a corpse-bound npc are not
    -- something to rely on. global_player.event_timer routes "delveclose" back to M.on_close_timer.
    c:SetTimer("delveclose", M.CLOSE_DELAY_SECS)
    c:Message(MT.Yellow, string.format(
        "The chest bursts open. The delve will close in %d seconds -- take what is inside.",
        M.CLOSE_DELAY_SECS))
end

-- The countdown expired.
-- ⚠️ Re-checks the run: the player may have pressed Exit, died, or walked a zone line in the
-- meantime, and every one of those paths already tore the run down. Without this the timer would
-- fire M.leave against a character standing in the ordinary world.
function M.on_close_timer(c)
    if not c or not c.valid then return end
    if not M.current_run(c) then return end
    M.leave(c, "The delve closes behind you.")
end

-- ---------------------------------------------------------------- per-run upkeep
-- From global_player.event_timer, name "delvescale". Re-measures the player and re-scales anything in
-- the instance that is not already fighting. No-op unless they are actually inside a delve, so the
-- timer is harmless to leave running for everybody.
function M.on_tick(c)
    if not c or not c.valid then return end
    local run = M.current_run(c)
    if not run then return end
    local L = M.LAYERS[run.layer]
    if not L then return end
    -- ⚠️ Guard on the ZONE as well as the bucket. The run bucket survives zoning out by any means the
    -- exit path did not handle (a crash, a GM port), and without this the sweep would rescale mobs in
    -- whatever ordinary zone the player is standing in.
    if eq.get_zone_short_name() ~= L.zone then return end
    if (eq.get_zone_instance_id() or 0) == 0 then return end

    scale.rescale_zone(c, L.level)
end

-- ⚠️⚠️ `e.other` IS A Lua_Mob, NOT A Lua_Client. `IsClient()` returns true on it, but CharacterID is
-- only defined on Lua_Client -- so passing it straight through was "attempt to call method
-- 'CharacterID' (a nil value)" on EVERY kill in a delve. `CastToClient` is not reliably bound here
-- either, so resolve through the entity list by id, which is the approach aotv4_sinewtap.lua already
-- uses for exactly this reason.
function M.as_client(mob)
    if not mob or not mob.valid or not mob:IsClient() then return nil end
    local c = eq.get_entity_list():GetClientByID(mob:GetID())
    if not c or not c.valid then return nil end
    return c
end

-- A creature died in a delve: bank what it was worth. Called from global_npc.event_death_complete.
function M.on_npc_death(e)
    local c = M.as_client(e.other)
    if not c then return end
    local run = M.current_run(c)
    if not run then return end
    -- ⚠️ The rung is passed so an UNSCALED mob scores as the dungeon rather than as its own native
    -- Dragons of Norrath level (60-70). See the fallback in record_kill for what that cost.
    scale.record_kill(c, e.self, run.layer)

    local mode = run.modedef or M.mode_by_id(M.MODE_DEFAULT)
    local L    = M.LAYERS[run.layer]
    if not L or not mode then return end

    local was_boss = (e.self and e.self.valid and e.self:GetNPCTypeID() == M.BOSS_NPC)

    -- ⚠️⚠️ GAUNTLET CHAINS ITS BOSSES HERE, and without it that mode is UNCOMPLETABLE: its task wants
    -- three kills of npc 2000301 but only one is ever spawned by the stage hook. Each death summons
    -- the next until the mode's quota has been spawned.
    -- ⚠️ Counted in a bucket rather than by asking the task how many are done: the activity's done
    -- count is not readable from Lua here, and re-deriving it wrongly would either stall the run one
    -- boss short or spawn an endless chain.
    if was_boss then
        local key = bkey(c, "boss")
        local n   = (tonumber(eq.get_data(key)) or 1)
        local want = mode.bosses or 1
        if n < want then
            eq.set_data(key, tostring(n + 1))
            M.spawn_boss(c, L, run, n + 1)
        else
            eq.delete_data(key)
        end
        return   -- a boss never splits, whatever the mode
    end

    -- ⚠️⚠️ SWARM: every trash death puts three more in its place. Net +2 per kill, so ninety kills
    -- would try to add 270 bodies -- the live cap is what stops the zone melting, and spawns above it
    -- are SKIPPED rather than queued (a queue would just move the melt later).
    -- ⚠️ Only trash splits, and only in a mode that asks for it; the chest and the boss are excluded
    -- by id so a cleared dungeon does not erupt.
    if (mode.swarm or 0) > 0 and e.self and e.self.valid then
        local id = e.self:GetNPCTypeID()
        if id ~= M.CHEST_NPC and id ~= M.BOSS_NPC then
            local alive = 0
            local list = eq.get_entity_list():GetNPCList()
            for _ in list.entries do alive = alive + 1 end
            if alive < M.SWARM_CAP then
                local x, y, z, h = e.self:GetX(), e.self:GetY(), e.self:GetZ(), e.self:GetHeading()
                local room = M.SWARM_CAP - alive
                local n = mode.swarm < room and mode.swarm or room
                for _ = 1, n do
                    -- ⚠️ Spawned at a small offset, not exactly on the corpse: three mobs sharing one
                    -- coordinate stack into a single unclickable pile.
                    eq.spawn2(id, 0, 0, x + math.random(-6, 6), y + math.random(-6, 6), z, h)
                end
            end
        end
    end
end

-- ---------------------------------------------------------------- leaving by a zone line
-- ⚠️⚠️ DoN ZONES HAVE REAL ZONE LINES OUT OF THEM. Lavaspinner's Lair connects onward to places that
-- have nothing to do with the delve, and walking one of those lines drops you into the ordinary
-- world with the run bucket still set, the task still live and the instance still open. So crossing
-- one FAILS the run, exactly like dying: the mission was abandoned, whatever the intent.
--
-- Called from global_player.event_enter_zone, which also fires on the way IN -- so the test has to be
-- "am I somewhere other than my delve", not merely "did I zone".
function M.on_enter_zone(e)
    local c = e.self
    if not c or not c.valid then return end

    local run = M.current_run(c)
    if not run then return end

    local L = M.LAYERS[run.layer]
    if not L then return end

    -- Still inside the right instance of the right zone: this is the entry itself, nothing to do.
    -- ⚠️ Arrival is also where the transit flag is retired -- from here on a disconnect really is a
    -- camp and SHOULD end the run.
    if eq.get_zone_short_name() == L.zone and (eq.get_zone_instance_id() or 0) == run.instance then
        clear_transit(c)
        return
    end

    -- ⚠️ Landed somewhere else: the entry failed or they walked out. Retire the flag here too, or a
    -- camp within the grace window would be mistaken for a transit and leak the instance.
    clear_transit(c)

    if L then
        local t = M.task_for(L, run.modedef)
        if c:IsTaskActive(t) then c:FailTask(t) end
    end
    M.record_history(c, run.layer, "F")

    -- ⚠️ Read the return point BEFORE clearing the run, then send them there -- "drop me off where I
    -- started it". They are already out of the instance by the time this runs (the zone change has
    -- happened), so unlike the death path this one DOES move them.
    local back = eq.get_data(bkey(c, "back")) or ""
    local zid, x, y, z = back:match("^(%d+)|(-?%d+)|(-?%d+)|(-?%d+)$")

    local inst = run.instance
    clear_run(c)
    eq.delete_data(bkey(c, "back"))

    c:Message(MT.Red, string.format("You left %s. The delve is closed and the mission is failed.", L.name))

    if zid then
        c:MovePC(tonumber(zid), tonumber(x), tonumber(y), tonumber(z), 0)
    end
    if inst and inst > 0 then
        eq.destroy_instance(inst)
    end
end

-- ---------------------------------------------------------------- camping out ends the run
-- ⚠️⚠️ CAMPING (/q) INSIDE A DELVE IS THE THIRD WAY OUT, and it needs handling for the same reason as
-- the other two: the character is SAVED standing inside the instance. If that instance is later gone
-- (destroyed, or its 6 hour timer lapsed) the next login asks world to place them somewhere that no
-- longer exists. The engine does fall back gracefully -- the world log shows delvea(105) then
-- tutorialb -- but it leaves an orphaned instance behind every time and a run bucket that says the
-- player is still inside a dungeon they are not in.
--
-- So camping ends the run, consistently with dying and with walking a zone line.
--
-- ⚠️ This does NOT move the player and does not touch the task: the client is already leaving, and
-- calling into it here is asking for trouble. The stale journal entry is cleaned up on the way back
-- IN instead -- see M.on_connect.
function M.on_disconnect(c)
    if not c or not c.valid then return end

    -- ⚠️⚠️ ZONING RAISES THIS EVENT TOO. Without this guard, moving a player INTO their delve
    -- immediately tears the run down from under them and they get evicted back out -- see the
    -- TRANSIT FLAG note by clear_run for the measured sequence. Do not remove it: every delve entry
    -- passes through here, and the failure is silent and looks nothing like a disconnect bug.
    if in_transit(c) then return end

    local run = M.current_run(c)
    if not run then return end

    M.record_history(c, run.layer, "A")
    clear_run(c)
    eq.delete_data(bkey(c, "back"))

    if run.instance and run.instance > 0 then
        eq.destroy_instance(run.instance)
    end
end

-- The other half: on the way back in, drop any delve task left over from a camp or a client crash.
-- ⚠️ Guarded on there being NO active run -- a player who camped and came back while their instance is
-- still alive is legitimately mid delve, and this must not rip the journal entry out from under them.
function M.on_connect(c)
    if not c or not c.valid then return end
    if M.current_run(c) then return end
    -- ⚠️ DISTINCT task ids, not one per rung: 15 rungs share only 6 base tasks, so walking M.LAYERS
    -- directly would test the same task up to three times.
    -- ⚠️ EVERY MODE'S VARIANT has to be swept, not just the base row. A camp during a Hard run leaves
    -- task base+10 behind, and checking only base would walk straight past it -- leaving a permanent
    -- journal entry for a dungeon that no longer exists.
    local seen = {}
    for _, L in ipairs(M.LAYERS) do
        for _, m in ipairs(M.MODES) do
            local t = M.task_for(L, m)
            if not seen[t] then
                seen[t] = true
                if c:IsTaskActive(t) then c:RemoveTaskByTaskID(t) end
            end
        end
    end
end

-- ---------------------------------------------------------------- death ends the run
-- Called from global_player.event_death. The roguelite reset puts the character back to LEVEL 1, so
-- there is no sense in which the run can continue: the mission is failed, the journal entry goes, and
-- the instance closes.
--
-- ⚠️⚠️ THIS DOES NOT MOVE THE PLAYER, unlike every other teardown path. They are dead, and the engine
-- is already sending them to their bind point as part of the death flow -- issuing a competing MovePC
-- from inside event_death fights that, and the one thing worse than a stale instance is a broken
-- respawn. The instance is destroyed and the engine's own respawn takes them out of it.
-- ⚠️ Order still matters within what we DO control: drop the task before destroying the instance, or
-- a failed run can leave a live journal entry pointing at a dungeon that no longer exists.
--
-- ⚠️ FailTask, not RemoveTaskByTaskID: this is a real failure and should read as one. FailTask gives
-- the client the "task failed" treatment; RemoveTaskByTaskID (used by M.leave) removes it silently,
-- which is right for walking out voluntarily and wrong for dying.
function M.on_death(e)
    local c = e.self
    if not c or not c.valid then return end

    local run = M.current_run(c)
    if not run then return end

    local L = M.LAYERS[run.layer]

    if L then
        local t = M.task_for(L, run.modedef)
        if c:IsTaskActive(t) then c:FailTask(t) end
    end

    -- A failed run still earned whatever it killed on the way down -- bank the score sheet before the
    -- ledger is lost, or dying would erase the record of a genuinely hard fight.
    M.record_history(c, run.layer, "F")

    clear_run(c)
    eq.delete_data(bkey(c, "back"))

    c:Message(MT.Red, string.format("You have died. %s is lost, and the way in has closed.",
        L and L.name or "The delve"))

    if run.instance and run.instance > 0 then
        eq.destroy_instance(run.instance)
    end
end

-- ---------------------------------------------------------------- chat protocol
-- All of it is swallowed by the dll; these are also typeable by hand for testing.
--   /say delve                 open the window
--   /say delveenter <level>    enter that layer
--   /say delveexit             leave
function M.handle_say(e)
    local msg = (e.message or ""):lower()

    if msg == M.SAY_TRIGGER then
        M.send_modes(e.self)     -- modes first: the window builds its dropdown before the layer list
        M.send_list(e.self)
        M.send_history(e.self)   -- every tab is filled by opening the window; one round trip, not three
        return true
    end

    if msg == "delvehist" then
        M.send_history(e.self)
        return true
    end

    -- ⚠️ TWO patterns, and the bare one must stay: `delveenter <level>` is what an un-rebuilt dll
    -- still sends, and it has to keep working (it falls back to Standard in M.enter). A single
    -- pattern with an optional group would also match, but keeping them separate makes the
    -- backward-compatible case explicit rather than accidental.
    local lvl, mode_id = msg:match("^delveenter%s+(%d+)%s+(%a+)$")
    if lvl then
        M.enter(e.self, tonumber(lvl), mode_id)
        return true
    end

    lvl = msg:match("^delveenter%s+(%d+)$")
    if lvl then
        M.enter(e.self, tonumber(lvl), M.MODE_DEFAULT)
        return true
    end

    if msg == "delveexit" then
        M.leave(e.self, "You step back out of the delve.")
        return true
    end

    -- Diagnostic: what the scaler currently thinks of you, axis by axis. Exists because one number
    -- ("power 1.42") is untunable on its own -- you need to see WHICH axis is carrying it.
    -- ⚠️ GM ONLY TEST SPAWN. Exists purely so the boss MODEL can be iterated on without clearing the
    -- trash goal first -- otherwise every look at a warden costs 10 to 90 kills, which is what makes
    -- a client side asset problem so slow to chase. Optional race argument spawns a SPECIFIC race
    -- (`delveboss 496`) so a single archive can be confirmed instead of waiting for the random pick
    -- to land on it. Does not touch the task, the ledger or the run: it is a look-at-it spawn only.
    local br = msg:match("^delveboss%s*(%d*)$")
    if br then
        local c = e.self
        if c:GetGMStatus() <= 0 then return true end
        local run = M.current_run(c)
        if not run then
            c:Message(MT.Red, "delveboss: you must be inside a delve (it needs the layer to scale to).")
            return true
        end
        local L = M.LAYERS[run.layer]
        local forced = tonumber(br)
        if forced and forced > 0 then
            -- ⚠️ Temporarily narrow the pool rather than adding a parameter to spawn_boss: the real
            -- path must stay exactly the code being tested, or the test proves nothing about it.
            local saved = M.BOSS_RACES
            M.BOSS_RACES = { forced }
            M.spawn_boss(c, L, run, 1)
            M.BOSS_RACES = saved
        else
            M.spawn_boss(c, L, run, 1)
        end
        return true
    end

    if msg == "delvepower" then
        local c = e.self
        local power, axes = scale.power(c)
        c:Message(MT.Yellow, string.format("Power %.3f  (1.00 = naked expectation for level %d %s)",
            power, c:GetLevel(), tostring(c:GetClassName())))
        if axes then
            for _, a in ipairs({ "hp", "mana", "ac", "stats", "resists" }) do
                c:Message(MT.LightBlue, string.format("  %-8s %s", a,
                    axes[a] and string.format("%.3f", axes[a]) or "n/a (does not apply to this class)"))
            end
        end
        local run = M.current_run(c)
        if run then
            local L = M.LAYERS[run.layer]
            local eff = scale.effective_level(L.level, c)
            c:Message(MT.Yellow, string.format("In %s: layer level %d, creatures scaling to %d.",
                L.name, L.level, eff))
        end
        local led = scale.ledger(c)
        if led and led.kills > 0 then
            c:Message(MT.Yellow, string.format("Ledger: %d kills, average level %.1f (%d to %d).",
                led.kills, led.avg, led.lo, led.hi))
        end
        return true
    end

    return false
end

return M
