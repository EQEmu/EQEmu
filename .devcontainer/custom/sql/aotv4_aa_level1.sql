-- =============================================================================================
-- AoTv4 -- every rank of every ENABLED AA is purchasable at level 1              2026-07-31
--
-- One rule, applied to the whole AA window rather than to a particular set: if an ability is
-- enabled here, none of its ranks carries a level requirement.
--
-- ⚠️⚠️ THIS EXISTS BECAUSE A PER-SET SCRIPT MISSED HALF THE PROBLEM. aotv4_aa_bloat.sql lowers
-- level_req for the abilities IT enables, so the Luclin/PoP passives were correct while the 40
-- hand-built AoTv4 trees still had **155 of their 200 ranks gated, the highest at level 45** --
-- above the level cap, so those ranks were unreachable outright. Scoping the fix to "the AAs this
-- script owns" is what let that survive; scoping it to "everything enabled" cannot.
--
-- ⚠️⚠️ EVERY RANK, NOT JUST first_rank_id. Ranks are walked by `next_id` because rank ids are NOT
-- contiguous (section 10). Lowering only the first rank lets a player buy rank 1 and leaves the rest
-- permanently unreachable -- which reads as the AA being broken rather than as a level gate, because
-- the window still lists the ranks and simply refuses them.
-- ⚠️ Cost is floored at 1 in the same pass: a free rank is not a decision.
--
-- ⚠️ AAs are NOT in shared memory -- they load at ZONE boot, so this needs a zone restart only.
-- ⚠️ Re-runnable, and safe to re-run after enabling any further AAs.
-- =============================================================================================

DROP TEMPORARY TABLE IF EXISTS tmp_all_enabled_ranks;
CREATE TEMPORARY TABLE tmp_all_enabled_ranks (rank_id INT PRIMARY KEY);
INSERT INTO tmp_all_enabled_ranks (rank_id)
WITH RECURSIVE chain AS (
    SELECT r.id AS rank_id, r.next_id, 1 AS depth
    FROM aa_ability a JOIN aa_ranks r ON r.id = a.first_rank_id
    WHERE a.enabled = 1
  UNION ALL
    SELECT r2.id, r2.next_id, c.depth + 1
    FROM chain c JOIN aa_ranks r2 ON r2.id = c.next_id
    WHERE c.depth < 40
)
SELECT DISTINCT rank_id FROM chain;

UPDATE aa_ranks SET level_req = 1 WHERE id IN (SELECT rank_id FROM tmp_all_enabled_ranks);
UPDATE aa_ranks SET cost = 1 WHERE cost < 1 AND id IN (SELECT rank_id FROM tmp_all_enabled_ranks);

-- ---------------------------------------------------------------- verify
SELECT 'enabled abilities' AS what, COUNT(*) n FROM aa_ability WHERE enabled = 1
UNION ALL SELECT 'ranks across them', COUNT(*) FROM tmp_all_enabled_ranks
UNION ALL SELECT 'ranks above level 1 (must be 0)', COUNT(*) FROM aa_ranks
  WHERE id IN (SELECT rank_id FROM tmp_all_enabled_ranks) AND level_req > 1
UNION ALL SELECT 'free ranks (must be 0)', COUNT(*) FROM aa_ranks
  WHERE id IN (SELECT rank_id FROM tmp_all_enabled_ranks) AND cost < 1
UNION ALL SELECT 'abilities not class-agnostic (must be 0)', COUNT(*) FROM aa_ability
  WHERE enabled = 1 AND classes <> 65535;

DROP TEMPORARY TABLE tmp_all_enabled_ranks;
