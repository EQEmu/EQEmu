-- AoTv4 -- the Wayfinder: the NPC in Resplendent who opens regions.
-- =====================================================================================
-- Every character starts with ONE region credit, spent on whichever of the six regions they want.
-- After that a credit only comes from an "Ending" achievement (dying at the level cap), so the
-- second region is earned rather than chosen at the door.
--
-- ⚠️⚠️ REGION LOCKING IS INVERTED FROM WHAT IT LOOKS LIKE. RegionManager::CanEnterZone ALLOWS any
-- zone not in `zone_regions` and BLOCKS a mapped zone whose region you lack. So the 51 organised
-- zones are the gated ones. Without this NPC a fresh character holds no region at all and cannot
-- reach Crushbone, Blackburrow, the Commonlands or the Karanas -- which is why the starting choice
-- exists and why it is granted at the door rather than earned.
--
-- ⚠️ Cloned VIA A TEMP TABLE from the Parcel Courier (2000220), which is already the shape we want:
-- level 70, immovable, no faction, non-aggro. Hand-listing npc_types columns is the section 24 trap
-- (`aggro_spell_id` does not exist on this schema).
-- ⚠️⚠️ A ZONE RESTART IS ENOUGH -- npc_types is NOT shared memory, despite what an earlier version of
-- this comment said. `shared_memory` builds exactly two blobs, `items` and `spells` (shared_memory/
-- main.cpp calls only LoadItems and LoadSpells); everything else, npc_types and spawn2 included, is
-- read from the database when a zone boots. Believing otherwise costs a needless world-down cycle.
--
-- ⚠️⚠️ THE QUEST SCRIPT FILENAME MUST INCLUDE THE LEADING '#'. The npc is `#Wayfinder_Alessa`, so the
-- script is quests/resplendent/#Wayfinder_Alessa.lua -- the file is matched on the npc's NAME, hash
-- and all, which is why 1,161 existing scripts are named that way. Named `Wayfinder_Alessa.lua` she
-- spawns perfectly and simply never responds, with nothing in any log to say why.

DELETE FROM spawnentry WHERE spawngroupID = 2000400;
DELETE FROM spawn2     WHERE spawngroupID = 2000400;
DELETE FROM spawngroup WHERE id           = 2000400;
DELETE FROM npc_types  WHERE id           = 2000400;

CREATE TEMPORARY TABLE aotv4_tmp_npc AS SELECT * FROM npc_types WHERE id = 2000220;
UPDATE aotv4_tmp_npc SET
    id       = 2000400,
    name     = '#Wayfinder_Alessa',
    lastname = 'Keeper of Ways',
    level    = 70,
    race     = 1, gender = 1, class = 41,     -- class 41 = a plain quest/merchant style humanoid
    size     = 6,
    -- ⚠️ Not attackable and not aggro: this is a service NPC standing in the starting hub, and a
    -- player who kills it would lose the only way to open a region.
    npc_aggro = 0, always_aggro = 0,
    bodytype = 11,                            -- untargetable-for-attack body type
    runspeed = 0, walkspeed = 0,
    loottable_id = 0, merchant_id = 0,
    -- Gives it presence in a hub where everything else is scenery.
    light = 10, texture = 1;
INSERT INTO npc_types SELECT * FROM aotv4_tmp_npc;
DROP TEMPORARY TABLE aotv4_tmp_npc;

-- ---------------------------------------------------------------- placement
-- Position picked in game and given as a client /loc reading.
--
-- ⚠️⚠️ THE CLIENT'S /loc REPORTS Y, X, Z -- NOT X, Y, Z. "Your Location is 696.62, -30.70, -26.25"
-- therefore means x = -30.70, y = 696.62, z = -26.25, which is what is stored below. The SERVER's
-- #loc is the other way round and labels itself "XYZ:", so the two commands disagree and the wording
-- is the only tell. Entering a /loc reading verbatim puts the npc ~700 units away on the wrong axis.
-- ⚠️ Corroborated by the geometry rather than trusted: x = -30.70 sits beside the arrival point's
-- x = -22 (aotv4_start_resplendent.sql) and the zone safe point's x = -33, whereas reading it as
-- x = 696 would place her nowhere near either.
INSERT INTO spawngroup (id, name, spawn_limit, dist, max_x, min_x, max_y, min_y, delay, mindelay, despawn, despawn_timer)
    VALUES (2000400, 'aotv4_wayfinder', 0, 0, 0, 0, 0, 0, 0, 0, 0, 100);
INSERT INTO spawnentry (spawngroupID, npcID, chance) VALUES (2000400, 2000400, 100);
-- ⚠️ spawn2 has NO `enabled` column on this schema.
-- ⚠️⚠️ min_expansion / max_expansion MUST be -1. The server runs at Expansion:CurrentExpansion = 0
-- (Classic), and anything expansion-tagged above that is content-filtered OUT at zone boot -- which
-- is exactly how the Plane of Knowledge books disappeared (CLAUDE.md section 11). She would simply
-- never spawn, with nothing in any log to say why.
INSERT INTO spawn2 (spawngroupID, zone, version, x, y, z, heading, respawntime, variance,
                    min_expansion, max_expansion)
    VALUES (2000400, 'resplendent', 0, -30.70, 696.62, -26.25, 128, 300, 0, -1, -1);

SELECT (SELECT COUNT(*) FROM npc_types WHERE id = 2000400)                       AS npc,
       (SELECT COUNT(*) FROM spawn2 WHERE spawngroupID = 2000400)                AS spawn_point,
       (SELECT CONCAT(x,', ',y,', ',z) FROM spawn2 WHERE spawngroupID = 2000400) AS placed_at;
