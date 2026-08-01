-- AoTv4 -- the Herald: a guide NPC in Resplendent who explains the server on hail.
-- =====================================================================================
-- ⚠️⚠️ CLIENT /loc REPORTS Y, X, Z. "Your Location is 688.80, 7.29, -23.42" means
-- x = 7.29, y = 688.80, z = -23.42. The SERVER's #loc is the other way round and labels itself
-- "XYZ:", so the wording is the only tell; entering a /loc reading verbatim lands ~690 units out.
-- ⚠️ Corroborated: this sits between the Wayfinder (-30.7, 696.6) and the Reforger (-100.5, 684.3).
--
-- ⚠️⚠️ THE QUEST SCRIPT FILENAME MUST INCLUDE THE LEADING '#' -- quests/resplendent/#Herald_Coren.lua.
-- Without it he spawns perfectly and never responds, with nothing in any log.
-- ⚠️ A zone restart is enough; npc_types and spawn2 are read at zone boot, not shared memory.
-- ⚠️ min_expansion / max_expansion -1 or the Classic content filter removes him at boot.

DELETE FROM spawnentry WHERE spawngroupID = 2000402;
DELETE FROM spawn2     WHERE spawngroupID = 2000402;
DELETE FROM spawngroup WHERE id           = 2000402;
DELETE FROM npc_types  WHERE id           = 2000402;

CREATE TEMPORARY TABLE aotv4_tmp_npc AS SELECT * FROM npc_types WHERE id = 2000400;
UPDATE aotv4_tmp_npc SET
    id       = 2000402,
    name     = '#Herald_Coren',
    lastname = 'Voice of the Temple',
    race     = 1, gender = 0,
    size     = 6;
INSERT INTO npc_types SELECT * FROM aotv4_tmp_npc;
DROP TEMPORARY TABLE aotv4_tmp_npc;

INSERT INTO spawngroup (id, name, spawn_limit, dist, max_x, min_x, max_y, min_y, delay, mindelay, despawn, despawn_timer)
    VALUES (2000402, 'aotv4_herald', 0, 0, 0, 0, 0, 0, 0, 0, 0, 100);
INSERT INTO spawnentry (spawngroupID, npcID, chance) VALUES (2000402, 2000402, 100);
INSERT INTO spawn2 (spawngroupID, zone, version, x, y, z, heading, respawntime, variance,
                    min_expansion, max_expansion)
    VALUES (2000402, 'resplendent', 0, 7.29, 688.80, -23.42, 128, 300, 0, -1, -1);

SELECT (SELECT COUNT(*) FROM npc_types WHERE id = 2000402)                       AS npc,
       (SELECT CONCAT(x,', ',y,', ',z) FROM spawn2 WHERE spawngroupID = 2000402) AS placed_at;
