-- ==========================================================================================
-- AoTv4 respawn times, server wide:
--   raid targets      -> 21600 s (6 hours)
--   everything else   ->   600 s (10 minutes)
--
-- "Raid mob" is npc_types.raid_target AND a NAMED mob. The flag alone is too broad: it is set on
-- raid ADDS as well as bosses, so Innoruuk and a_forsaken_revenant are both raid_target and only one
-- of them should be on a 6 hour timer. Of the 4,609 spawn points the flag alone selects, 3,001 hold
-- nothing but lowercase adds.
--
-- ⚠️⚠️ NAMED = "the first character is not a lowercase letter", tested with BINARY.
--   * `BINARY` is REQUIRED. MySQL's REGEXP and LIKE are CASE INSENSITIVE under the default
--     collation, so `name REGEXP '^[a-z]'` matches `Lady_Vox` and quietly classifies every boss in
--     the game as trash. Nothing about the result looks wrong until you read the names.
--   * ⚠️ This is NOT the "starts with #" rule CLAUDE.md section 15 uses for the Hunter achievements.
--     That convention does not hold here: only 456 of these 4,609 spawn points have a `#` name,
--     while Lady_Vox, Innoruuk, Gorenaire and Phinigel_Autropos have none. Do not "fix" one to
--     match the other -- they are answering different questions on different data.
-- The result (1,608 points) lands close to the 1,560 that were already on a 6 hour timer before any
-- of this ran, which is the best evidence available that it matches the original intent.
--
-- ⚠️ A SPAWN POINT is set to 6 hours if ANY npc in its spawngroup is a named raid target. A
-- spawngroup is a weighted list (spawnentry), so a point that can roll a boss has to be treated as
-- the boss's -- keying off "the first entry" would leave a raid mob on a 10 minute timer whenever it
-- shared a point with trash.
--
-- ⚠️⚠️ respawntime = 0 IS NOT "instant" -- IT MEANS THE POINT DOES NOT RESPAWN, and 5,123 rows use
-- it. Those are one time and quest/script controlled spawns. Setting them to 600 would make quest
-- NPCs reappear every ten minutes and would break scripted encounters, so they are LEFT ALONE.
--
-- ⚠️ `variance` is deliberately untouched. It is the random spread around respawntime; the request
-- was about the timer, and zeroing or inventing a spread is a separate decision.
--
-- spawn2 is read from the DB at ZONE BOOT (it is not in shared memory -- only items and spells are
-- here), so restart zones after this. No ./shared_memory rebuild needed.
-- Idempotent: re-running sets the same values.
-- ==========================================================================================

-- ---- reversible: keep the pre change values once, the FIRST time this is run -----------------
CREATE TABLE IF NOT EXISTS aotv4_spawn2_respawn_backup (
  id          INT PRIMARY KEY,
  respawntime INT NOT NULL,
  variance    INT NOT NULL
);
INSERT IGNORE INTO aotv4_spawn2_respawn_backup (id, respawntime, variance)
  SELECT id, respawntime, variance FROM spawn2;

-- ---- 1. raid targets -> 6 hours --------------------------------------------------------------
UPDATE spawn2 s
SET s.respawntime = 21600
WHERE s.respawntime <> 0
  AND EXISTS (
    SELECT 1 FROM spawnentry se
    JOIN npc_types n ON n.id = se.npcID
    WHERE se.spawngroupID = s.spawngroupID AND n.raid_target = 1
      AND NOT (BINARY LEFT(n.name, 1) BETWEEN 'a' AND 'z')   -- named only, NOT adds
  );

-- ---- 2. everything else -> 10 minutes --------------------------------------------------------
-- ⚠️ THE NAMED TEST MUST BE REPEATED HERE, IDENTICALLY. The two statements have to partition the
-- table between them: if this one only asked "no raid_target in the group", a point holding nothing
-- but ADDS would satisfy neither statement and would be silently left on whatever timer it already
-- had -- which, having just run step 1 in an earlier pass, is 6 hours. That is exactly the case this
-- change exists to fix, so it would look like the edit did nothing.
UPDATE spawn2 s
SET s.respawntime = 600
WHERE s.respawntime <> 0
  AND NOT EXISTS (
    SELECT 1 FROM spawnentry se
    JOIN npc_types n ON n.id = se.npcID
    WHERE se.spawngroupID = s.spawngroupID AND n.raid_target = 1
      AND NOT (BINARY LEFT(n.name, 1) BETWEEN 'a' AND 'z')   -- named only, NOT adds
  );

-- ---- restore (manual) ------------------------------------------------------------------------
-- UPDATE spawn2 s JOIN aotv4_spawn2_respawn_backup b ON b.id = s.id
--   SET s.respawntime = b.respawntime, s.variance = b.variance;
