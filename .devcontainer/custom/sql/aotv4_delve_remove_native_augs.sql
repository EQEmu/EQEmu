-- =============================================================================================
-- AoTv4 -- take the stock augments out of the Delve zones                             2026-07-30
--
-- The Delve hands out its own evolving augment set (aotv4_delve_augs.sql, 147600-148007), which
-- starts at 2/4/6 points of a single stat. The stock augments that drop in these six DoN zones give
-- 15-60 hp/mana and 8 ac outright, so a first clear could hand a player something several times
-- stronger than the reward line is ever meant to reach at tier 3.
--
-- ⚠️⚠️ REMOVED FROM `lootdrop_entries`, WHICH IS GLOBAL -- so this is only safe because every one of
-- these nine items was CHECKED to drop nowhere else. Each resolves to exactly one zone:
--     71374/71375/71376  Volcanic Shard (Broken/Dusty/Bright)   delvea
--     71402              Stone of the Defensive Arts            stillmoona
--     71403              Mark of the Focused Life               stillmoona
--     71404/71405        Jewel of the Enlightened Body/Spirit   stillmoona
--     71472              Graycloud Gem of Health                thundercrest
--     71515              Sparkling Nest Ornament                thenest
-- Verify before re-running against a changed database; a shared lootdrop would silently strip the
-- item from the rest of the game as well.
--
-- ⚠️ The ITEM ROWS ARE LEFT ALONE. Only the drop is removed, so anything a player already owns keeps
-- working and nothing that references these ids breaks. 71402 in particular is the CLONE TEMPLATE
-- for the whole custom augment set (gen_delve_augs.pl) -- deleting it would break a regen.
--
-- ⚠️ THE TWO COSMETIC ORNAMENTS ARE DELIBERATELY KEPT: 85412 Mummy Head Ornament and 85414
-- Jack O' Lantern Head Ornament are augtype 1048576 (ornamentation) with no stats at all. They are
-- appearance, not power, so they fail the reason this file exists.
--
-- ⚠️ `items` is shared memory but `lootdrop_entries` is NOT -- it is read per NPC at spawn, so this
-- takes effect on a zone restart with no ./shared_memory rebuild.
-- ⚠️ Re-runnable: a DELETE of rows that are already gone is a no-op.
-- =============================================================================================

DELETE FROM lootdrop_entries
WHERE item_id IN (71374, 71375, 71376, 71402, 71403, 71404, 71405, 71472, 71515);

-- ---------------------------------------------------------------- verify
-- Every stat augment still reachable from a delve zone. Expected: the 2 cosmetic ornaments only.
SELECT DISTINCT i.id, i.Name, i.augtype
FROM zone z
JOIN spawn2 s2            ON s2.zone         = z.short_name
JOIN spawnentry se        ON se.spawngroupID = s2.spawngroupID
JOIN npc_types n          ON n.id            = se.npcID
JOIN loottable_entries lte ON lte.loottable_id = n.loottable_id
JOIN lootdrop_entries lde ON lde.lootdrop_id  = lte.lootdrop_id
JOIN items i              ON i.id            = lde.item_id
WHERE z.short_name IN ('delvea','delveb','stillmoona','stillmoonb','thundercrest','thenest')
  AND i.augtype > 0
ORDER BY i.id;
