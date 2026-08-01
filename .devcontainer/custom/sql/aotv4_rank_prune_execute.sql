-- AoTv4 -- STEP 2 of 2: remove the native rank variants identified by aotv4_rank_prune_analyze.sql.
-- =====================================================================================
-- ⚠️⚠️ RUN aotv4_rank_prune_analyze.sql FIRST, IN THE SAME SESSION OR BEFORE THIS. It builds the
-- `aotv4_rank_prune` / `aotv4_scroll_prune` staging tables this script deletes from. Without them
-- this does nothing at all (the joins match no rows) -- it fails safe, not loudly.
--
-- ⚠️⚠️ TAKE A FULL DUMP FIRST. This removes 6,126 spells, 4,582 items and ~38,724 loot rows. The
-- per-table backups below make it reversible in principle, but a mysqldump is the real safety net:
--     mysqldump -h127.0.0.1 -upeq -ppeqpass --single-transaction --routines --events \
--       --databases peq | gzip > peq_pre_rank_prune.sql.gz
--
-- ⚠️⚠️ BOTH `spells_new` AND `items` ARE SHARED MEMORY. Afterwards: world down, ./shared_memory,
-- restart. A zone restart is NOT enough. Then ./export_client_files and reinstall spells_us.txt, or
-- the client keeps offering scrolls for spells the server no longer has.
--
-- Verified before writing this (all read 0): nothing in the prune set is scribed, buffed, in a
-- player's bags, in shared bank, on a corpse, listed in a shop, a ground spawn, or a starting item.
-- Exactly ONE lootdrop becomes empty, out of 51,944.

-- ---------------------------------------------------------------- backups (first run only)
-- ⚠️ CREATE TABLE IF NOT EXISTS, so a second run cannot overwrite the pristine copy with the
-- already-pruned data. Same pattern as aotv4_race_scaling.sql.
CREATE TABLE IF NOT EXISTS aotv4_prune_backup_spells   AS SELECT s.* FROM spells_new s JOIN aotv4_rank_prune p ON p.id=s.id;
CREATE TABLE IF NOT EXISTS aotv4_prune_backup_items    AS SELECT i.* FROM items i      JOIN aotv4_scroll_prune s ON s.id=i.id;
CREATE TABLE IF NOT EXISTS aotv4_prune_backup_lootdrop AS SELECT l.* FROM lootdrop_entries l JOIN aotv4_scroll_prune s ON s.id=l.item_id;
CREATE TABLE IF NOT EXISTS aotv4_prune_backup_merchant AS SELECT m.* FROM merchantlist m     JOIN aotv4_scroll_prune s ON s.id=m.item;

-- ---------------------------------------------------------------- delete, children first
-- Loot and merchant rows go before the items they point at, so nothing dangles even briefly.
DELETE l FROM lootdrop_entries l JOIN aotv4_scroll_prune s ON s.id = l.item_id;
DELETE m FROM merchantlist     m JOIN aotv4_scroll_prune s ON s.id = m.item;
DELETE i FROM items            i JOIN aotv4_scroll_prune s ON s.id = i.id;
DELETE s FROM spells_new       s JOIN aotv4_rank_prune   p ON p.id = s.id;

-- ---------------------------------------------------------------- verify
SELECT (SELECT COUNT(*) FROM spells_new)                                   AS spells_now,
       (SELECT 45000 - COUNT(*) FROM spells_new WHERE id < 45000)          AS free_ids_now,
       (SELECT COUNT(*) FROM spells_new s JOIN aotv4_rank_prune p ON p.id=s.id)     AS spells_left_behind,
       (SELECT COUNT(*) FROM items i      JOIN aotv4_scroll_prune s ON s.id=i.id)   AS scrolls_left_behind,
       (SELECT COUNT(*) FROM lootdrop_entries l JOIN aotv4_scroll_prune s ON s.id=l.item_id) AS loot_left_behind;

-- ⚠️ The real test: nothing that SURVIVED may reference something that was deleted. All must be 0.
SELECT (SELECT COUNT(*) FROM items i JOIN aotv4_prune_backup_spells b ON b.id IN
          (i.clickeffect,i.proceffect,i.worneffect,i.focuseffect,i.bardeffect,i.scrolleffect)) AS items_pointing_at_deleted_spells,
       (SELECT COUNT(*) FROM npc_spells_entries n JOIN aotv4_prune_backup_spells b ON b.id=n.spellid) AS npcs_casting_deleted_spells,
       (SELECT COUNT(*) FROM aa_ranks a JOIN aotv4_prune_backup_spells b ON b.id=a.spell)             AS aa_using_deleted_spells,
       (SELECT COUNT(*) FROM character_spells c JOIN aotv4_prune_backup_spells b ON b.id=c.spell_id)  AS players_holding_deleted_spells;
