-- aotv4_aa_revert.sql -- put Alternate Advancement back to BASE EQ (2026-07-25)
-- =============================================================================================
-- Undoes the three schema edits aotv4_aa.sql made to serve the random-AA-on-death picker, which
-- is now retired.  With the picker gone those edits are actively harmful: grant_only hides every
-- AA from the native window, the Bard class tag hands every class's AAs to Bards, and the
-- level_req flattening lets a level 1 character buy raid-tier ranks.
--
-- What aotv4_aa.sql did, and what this reverses:
--   1. aa_ranks.level_req  1..70 -> 1            (so a post-death level 1 char could buy anything)
--   2. aa_ability.classes  cleared the Bard bit everywhere, then set it on the tagged 353-AA set
--   3. aa_ability.grant_only = 1 on that set     (hide untrained AAs from the native AA window)
--
-- The original values were OVERWRITTEN in place, so they cannot be computed back -- they are
-- restored from the pristine PEQ dump that ships with the server source.
--
-- RUN IT LIKE THIS (the scratch database is what makes the restore possible):
--
--   mysql -h127.0.0.1 -upeq -ppeqpass -e "CREATE DATABASE IF NOT EXISTS peq_aa_base"
--   mysql -h127.0.0.1 -upeq -ppeqpass peq_aa_base < /src/utils/sql/peq_aa_tables_post_rework.sql
--   mysql -h127.0.0.1 -upeq -ppeqpass peq         < /src/.devcontainer/custom/sql/aotv4_aa_revert.sql
--   mysql -h127.0.0.1 -upeq -ppeqpass -e "DROP DATABASE peq_aa_base"
--
-- AAs are read from the DB at ZONE BOOT (they are not in shared memory), so restart the zones
-- afterwards.  No shared_memory rebuild needed.
-- =============================================================================================

-- 1. level_req: restore every rank's real level gate.
UPDATE aa_ranks r
  JOIN peq_aa_base.aa_ranks b ON b.id = r.id
   SET r.level_req = b.level_req;

-- 2. classes: restore the real per-class bitmask, undoing both the blanket Bard-bit clear and the
--    Bard tagging of the 353-AA pool.  Each class gets its own AAs again.
UPDATE aa_ability a
  JOIN peq_aa_base.aa_ability b ON b.id = a.id
   SET a.classes = b.classes;

-- 3. grant_only: back to the stock value so untrained AAs are visible and purchasable in the
--    client's own AA window.  (The picker needed them hidden because it granted with
--    ignore_cost=true; nothing grants AAs behind the player's back any more.)
UPDATE aa_ability a
  JOIN peq_aa_base.aa_ability b ON b.id = a.id
   SET a.grant_only = b.grant_only;

-- Expansion:UseCurrentExpansionAAOnly stays FALSE.  It is not part of the picker: with the server
-- themed Classic (CurrentExpansion=0) a true value blocks ALL AAs outright, because every AA is
-- Luclin or later (aa_ranks.expansion >= 3).  Leaving it false is what makes AA exist at all here.

-- Sanity check after running (expect: no rows tagged grant_only from our pool, and a spread of
-- level_req values rather than 1855 ranks all sitting at 1):
--   SELECT COUNT(*) FROM aa_ability WHERE grant_only = 1;
--   SELECT level_req, COUNT(*) FROM aa_ranks GROUP BY level_req ORDER BY level_req LIMIT 10;
