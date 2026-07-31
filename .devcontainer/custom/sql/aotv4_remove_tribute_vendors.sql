-- ==========================================================================================
-- AoTv4: remove every tribute vendor from the world.
--
-- Tribute Masters (npc_types.class 63) and Guild Tribute Masters (class 64) are the ONLY way a
-- player can open the tribute window -- Handle_OP_OpenTributeMaster / Handle_OP_OpenGuildTributeMaster
-- are both reached by clicking one. Despawning them therefore removes access to the whole system
-- without touching the tribute code, the tribute tables, or anything a character already owns.
--
-- ⚠️ THIS DESPAWNS, IT DOES NOT DELETE THE NPCs. npc_types rows for all 30 are left intact: they are
-- stock PEQ content, some are referenced by quests, and keeping them means restoring a vendor is one
-- INSERT rather than a re-import. What is removed is spawn2 / spawnentry / spawngroup.
--
-- ⚠️ Checked before writing this: all 29 spawngroups involved hold ONLY tribute NPCs (verified with a
-- NOT EXISTS against non-tribute classes in the same group), so deleting whole groups cannot take an
-- unrelated NPC down with them. RE-CHECK THAT if this is ever re-run against a changed DB.
--
-- ⚠️ spawn2 is read at ZONE BOOT and is not in shared memory -- restart zones, no ./shared_memory.
-- Reversible: see the restore block at the bottom.
-- ==========================================================================================

-- ---- keep the rows so this can be undone ----------------------------------------------------
CREATE TABLE IF NOT EXISTS aotv4_tribute_spawn2_backup   LIKE spawn2;
CREATE TABLE IF NOT EXISTS aotv4_tribute_spawnentry_backup LIKE spawnentry;
CREATE TABLE IF NOT EXISTS aotv4_tribute_spawngroup_backup LIKE spawngroup;

INSERT IGNORE INTO aotv4_tribute_spawn2_backup
  SELECT s.* FROM spawn2 s
  WHERE EXISTS (SELECT 1 FROM spawnentry se JOIN npc_types n ON n.id = se.npcID
                WHERE se.spawngroupID = s.spawngroupID AND n.class IN (63, 64));

INSERT IGNORE INTO aotv4_tribute_spawnentry_backup
  SELECT se.* FROM spawnentry se JOIN npc_types n ON n.id = se.npcID
  WHERE n.class IN (63, 64);

INSERT IGNORE INTO aotv4_tribute_spawngroup_backup
  SELECT g.* FROM spawngroup g
  WHERE EXISTS (SELECT 1 FROM spawnentry se JOIN npc_types n ON n.id = se.npcID
                WHERE se.spawngroupID = g.id AND n.class IN (63, 64));

-- ---- despawn ---------------------------------------------------------------------------------
-- ⚠️ ORDER MATTERS: spawn2 and spawngroup are both identified THROUGH spawnentry, so spawnentry has
-- to be deleted LAST or the other two statements match nothing.
DELETE s FROM spawn2 s
  WHERE EXISTS (SELECT 1 FROM spawnentry se JOIN npc_types n ON n.id = se.npcID
                WHERE se.spawngroupID = s.spawngroupID AND n.class IN (63, 64));

DELETE g FROM spawngroup g
  WHERE EXISTS (SELECT 1 FROM spawnentry se JOIN npc_types n ON n.id = se.npcID
                WHERE se.spawngroupID = g.id AND n.class IN (63, 64));

DELETE se FROM spawnentry se JOIN npc_types n ON n.id = se.npcID
  WHERE n.class IN (63, 64);

-- ---- restore (manual) ------------------------------------------------------------------------
-- INSERT IGNORE INTO spawngroup SELECT * FROM aotv4_tribute_spawngroup_backup;
-- INSERT IGNORE INTO spawnentry SELECT * FROM aotv4_tribute_spawnentry_backup;
-- INSERT IGNORE INTO spawn2     SELECT * FROM aotv4_tribute_spawn2_backup;
