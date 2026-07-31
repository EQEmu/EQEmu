-- aotv4_dungeon.sql -- the scaling dungeon ("Delve") system: tasks + the reward chest.
-- =============================================================================================
-- FIFTEEN layers running level 1 to 70, cycling the six Dragons of Norrath instanced zones. Each is
-- a real INSTANCE of a stock DoN MISSION version, with every mob in it scaled by npc:ScaleNPC to the
-- layer's level and then to the player's own gear (aotv4_dungeon_scale.lua).
--
--   levels  1  5 10 15 20 25 30 35 40 45 50 55 60 65 70   (the ladder starts at 1, not 50)
--
-- ⚠️ The zone is REUSED across difficulties -- 6 zones, 15 layers -- which is why the tasks below are
-- per ZONE and their titles carry no level. The authoritative layer table is M.LAYERS in
-- lua_modules/aotv4_dungeon.lua; this file only supplies the six tasks and the chest.
--
-- ⚠️⚠️ THE VERSION IS NEVER 0. Version 0 is the OPEN WORLD spawn set, so instancing it gives a private
-- copy of the ordinary zone -- properly instanced, but identical in layout and population to just
-- walking in, which is not what an instanced dungeon should feel like. The non-zero versions are the
-- real DoN mission layouts. They are SMALLER (154 against 262 for delvea), and every kill goal above
-- is set against the mission version's count, not version 0's -- the highest is 60 of 144.
--
-- Layers unlock in order -- layer N+1 only appears in the window once layer N has been cleared.
-- That is the progression gate, and it is also what stops the level picker being a power-level
-- machine: you cannot jump to the level 70 layer without clearing the five below it.
--
-- ⚠️⚠️ THE TASKS ARE MODELLED FIELD-FOR-FIELD ON THE NATIVE DoN MISSIONS (301 Lavaspinner Hunting,
-- 1130 Infested) so they read as native content in the quest journal: 360 minute duration,
-- min_level/max_level tiering, and a Kill objective as step 1. The one deliberate difference is that
-- our Kill activity has an EMPTY npc_match_list.
--
-- ⚠️ AN EMPTY npc_match_list MATCHES EVERY NPC -- that is not an oversight, it is the mechanic.
-- task_client_state.cpp:528 guards the whole name test with `if (!activity.npc_match_list.empty()
-- && ...)`, so leaving it empty makes any kill in the listed zone count. A named list would break
-- the moment a layer pointed at a different zone, and this system is explicitly about killing lots
-- of whatever is in front of you. The `zones` column still scopes it to the right dungeon.
--
-- ⚠️ `zones` holds the BASE zone id, not the instance id. Instances keep their base zone id, so one
-- task row works for every instance of that zone.
--
-- ⚠️ Task ids 2000300-2000305, activity ids start at 0 per task. Chosen well clear of the ranges in
-- CLAUDE.md section 15 (Epics 710001+, Zone Slayer 700001+, ...) and of the stock task ids, which
-- top out in the 8000s here.
--
--   mysql -h127.0.0.1 -upeq -ppeqpass peq < .devcontainer/custom/sql/aotv4_dungeon.sql
--
-- ⚠️ npc_types IS NOT in shared memory (section 10) -- a zone restart picks the chest up. The tasks
-- are loaded by the zone at boot as well, so: restart zones, no ./shared_memory needed.
-- =============================================================================================

-- ---------------------------------------------------------------- the reward chest
-- Cloned from stock 119179 a_gilded_chest: race 378 is the treasure-chest model, bodytype 33 keeps
-- it out of normal combat handling, runspeed 0 so it cannot wander off the spot it spawned on.
-- ⚠️ It is spawned by Lua AT THE PLAYER'S POSITION when the task completes, never from a spawn2
-- row -- "where we took the last action to finish the quest" cannot be expressed as a fixed point.
-- ⚠️ CLONED VIA A TEMP TABLE so all ~90 npc_types columns stay byte-identical to stock. Never
-- hand-list them: a first pass did, and `aggro_spell_id` does not exist on this schema -- the insert
-- died on a column that only some EQEmu versions carry. Same rule as the custom spell lines.
DELETE FROM npc_types WHERE id = 2000300;

DROP TEMPORARY TABLE IF EXISTS aotv4_npc_tmpl;
CREATE TEMPORARY TABLE aotv4_npc_tmpl LIKE npc_types;
INSERT INTO aotv4_npc_tmpl SELECT * FROM npc_types WHERE id = 119179;
UPDATE aotv4_npc_tmpl SET
  id             = 2000300,
  name           = 'an_ornate_delve_chest',
  level          = 1,
  hp             = 100,
  loottable_id   = 0,      -- loot is out of scope for now; the chest is the object, not its contents
  npc_faction_id = 0,
  runspeed       = 0,      -- must not wander off the spot it was spawned on
  walkspeed      = 0,
  -- ⚠️ FINDABILITY IS THE POINT. It spawns wherever the last blow landed, which can be in a corridor,
  -- behind a corpse pile or off to one side, and a default-size chest on the floor of a DoN zone is
  -- genuinely easy to walk past. Three things stack here, and none of them is decoration:
  --   texture 1  the gilded (golden) chest rather than the plain wooden one
  --   size    12 roughly twice normal, so it reads as a landmark from across a room
  --   light    10 self-illuminating, which is what makes it visible in the dark interiors
  -- The Lua adds a nimbus aura and an emote on top (aotv4_dungeon.M.on_task_complete).
  texture        = 1,
  size           = 12,
  light          = 10;
INSERT INTO npc_types SELECT * FROM aotv4_npc_tmpl;
DROP TEMPORARY TABLE IF EXISTS aotv4_npc_tmpl;

-- ---------------------------------------------------------------- the six layer tasks
-- duration 21600s = 360 minutes, exactly what every DoN mission uses.
-- reward_method 0 / reward_id_list '' -- loot is explicitly out of scope for now; the chest is the
-- reward object and its contents are a later job.
DELETE FROM task_activities WHERE taskid BETWEEN 2000300 AND 2000305;
DELETE FROM tasks          WHERE id     BETWEEN 2000300 AND 2000305;

-- ⚠️⚠️ ONE TASK PER ZONE, NOT PER LAYER, and the titles carry NO level. There are 15 layers running
-- from level 1 to 70 across only 6 zones, so a zone is reused at several difficulties -- a title of
-- "[50]" would be a lie at 11 of the 15. The window shows the level; the journal shows the dungeon.
-- ⚠️ min_level is 1 for the same reason: the unlock chain is the real gate (and M.enter passes
-- enforce_level_requirement = false anyway), so a level tier here would only block the low layers.
INSERT INTO tasks (id, type, duration, duration_code, title, description, reward_text,
                   reward_id_list, cash_reward, exp_reward, reward_method, reward_points,
                   min_level, max_level, level_spread, min_players, max_players, repeatable,
                   faction_reward, completion_emote, enabled)
VALUES
 (2000300, 0, 21600, 3, 'Delve: Lavaspinner''s Lair',
  'The lavaspinners have overrun the lair. Cut them down, then claim what they were guarding.',
  'A reward chest', '', 0, 0, 0, 0, 1, 200, 0, 1, 6, 1, 0,
  'The lair falls silent. Something glitters where the last of them dropped.', 1),
 (2000301, 0, 21600, 3, 'Delve: Tirranun''s Delve',
  'Tirranun''s brood has grown bold in the deep. Thin them out and take the hoard.',
  'A reward chest', '', 0, 0, 0, 0, 1, 200, 0, 1, 6, 1, 0,
  'The delve goes quiet. Something glitters where the last of them dropped.', 1),
 (2000302, 0, 21600, 3, 'Delve: Stillmoon Temple',
  'The temple guardians no longer answer to anyone. Put them down and search the halls.',
  'A reward chest', '', 0, 0, 0, 0, 1, 200, 0, 1, 6, 1, 0,
  'The temple stills. Something glitters where the last of them dropped.', 1),
 (2000303, 0, 21600, 3, 'Delve: Stillmoon Ascent',
  'Cut a path up the ascent. Nothing that walks it now should be left standing.',
  'A reward chest', '', 0, 0, 0, 0, 1, 200, 0, 1, 6, 1, 0,
  'The ascent is clear. Something glitters where the last of them dropped.', 1),
 (2000304, 0, 21600, 3, 'Delve: Thundercrest Isles',
  'The isles swarm with drakes. Break them, and the storm they nest in is yours.',
  'A reward chest', '', 0, 0, 0, 0, 1, 200, 0, 1, 6, 1, 0,
  'The isles fall quiet. Something glitters where the last of them dropped.', 1),
 (2000305, 0, 21600, 3, 'Delve: The Nest',
  'The Nest is the deepest of them. Kill everything in it.',
  'A reward chest', '', 0, 0, 0, 0, 1, 200, 0, 1, 6, 1, 0,
  'The Nest is emptied. Something glitters where the last of them dropped.', 1);

-- One Kill objective per layer. goalcount climbs with the layer; `zones` is the BASE zone id so the
-- activity counts kills in any instance of it. target_name is what the journal actually displays.
INSERT INTO task_activities
  (taskid, activityid, step, activitytype, target_name, goalmethod, goalcount,
   description_override, npc_match_list, item_id_list, zones, optional)
VALUES
 (2000300, 0, 1, 2, 'the lair''s defenders', 0, 30, 'Slay the defenders of Lavaspinner''s Lair', '', '', '341', 0),
 (2000301, 0, 1, 2, 'Tirranun''s brood',      0, 35, 'Slay the brood of Tirranun''s Delve',      '', '', '342', 0),
 (2000302, 0, 1, 2, 'the temple guardians',   0, 40, 'Slay the guardians of Stillmoon Temple',   '', '', '338', 0),
 (2000303, 0, 1, 2, 'the ascent''s wardens',  0, 45, 'Slay the wardens of the Stillmoon Ascent', '', '', '339', 0),
 (2000304, 0, 1, 2, 'the storm drakes',       0, 50, 'Slay the drakes of Thundercrest Isles',    '', '', '340', 0),
 (2000305, 0, 1, 2, 'the brood of the Nest',  0, 60, 'Slay the brood of The Nest',               '', '', '343', 0);
