-- AoTv4 -- the Reforger: race and class change, at level 1 only.
-- =====================================================================================
-- Stands in Resplendent beside the Wayfinder. The roguelite puts a character back to level 1 on
-- every death, so "level 1 only" is the start of every run rather than a one-time window -- this is
-- a between-runs decision.
--
-- ⚠️⚠️ THE CLIENT'S /loc REPORTS Y, X, Z -- NOT X, Y, Z. "Your Location is 684.31, -100.49, -18.22"
-- therefore means x = -100.49, y = 684.31, z = -18.22. The SERVER's #loc is the other way round and
-- labels itself "XYZ:", so the wording is the only tell and a verbatim entry lands ~700 units out.
-- ⚠️ Corroborated: x = -100.49 and y = 684.31 sit near the Wayfinder at -30.70, 696.62.
--
-- ⚠️⚠️ THE QUEST SCRIPT FILENAME MUST INCLUDE THE LEADING '#' -- quests/resplendent/#Reforger_Vael.lua.
-- Named without it the npc spawns perfectly and never responds, with nothing in any log.
-- ⚠️ A ZONE RESTART IS ENOUGH: npc_types and spawn2 are read at zone boot, NOT shared memory
-- (shared_memory builds only the items and spells blobs).
-- ⚠️ min_expansion / max_expansion MUST be -1 or the Classic content filter removes her at boot.

DELETE FROM spawnentry WHERE spawngroupID = 2000401;
DELETE FROM spawn2     WHERE spawngroupID = 2000401;
DELETE FROM spawngroup WHERE id           = 2000401;
DELETE FROM npc_types  WHERE id           = 2000401;

CREATE TEMPORARY TABLE aotv4_tmp_npc AS SELECT * FROM npc_types WHERE id = 2000400;
UPDATE aotv4_tmp_npc SET
    id       = 2000401,
    name     = '#Reforger_Vael',
    lastname = 'Shaper of Forms',
    race     = 12, gender = 0,     -- a Gnome, so she reads as a tinkerer rather than a second Alessa
    size     = 4;
INSERT INTO npc_types SELECT * FROM aotv4_tmp_npc;
DROP TEMPORARY TABLE aotv4_tmp_npc;

INSERT INTO spawngroup (id, name, spawn_limit, dist, max_x, min_x, max_y, min_y, delay, mindelay, despawn, despawn_timer)
    VALUES (2000401, 'aotv4_reforger', 0, 0, 0, 0, 0, 0, 0, 0, 0, 100);
INSERT INTO spawnentry (spawngroupID, npcID, chance) VALUES (2000401, 2000401, 100);
INSERT INTO spawn2 (spawngroupID, zone, version, x, y, z, heading, respawntime, variance,
                    min_expansion, max_expansion)
    VALUES (2000401, 'resplendent', 0, -100.49, 684.31, -18.22, 128, 300, 0, -1, -1);

SELECT (SELECT COUNT(*) FROM npc_types WHERE id = 2000401)                       AS npc,
       (SELECT CONCAT(x,', ',y,', ',z) FROM spawn2 WHERE spawngroupID = 2000401) AS placed_at;
