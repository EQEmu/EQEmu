-- =============================================================================================
-- AoTv4 -- AA repricing: custom trees 1/2/2/3/3 by rank, passives 1 or 2      2026-07-31
--
--   AoTv4 custom trees (40 abilities, all exactly 5 ranks):
--       rank 1 = 1 point, ranks 2-3 = 2 points, ranks 4-5 = 3 points  -> 11 each, 440 total
--   Luclin/PoP passives (379 ranks): every rank becomes 1 or 2 points -> 560 total
--   Whole AA window = 1,000 points exactly.
--
-- ⚠️ THE 1-OR-2 SPLIT FOR PASSIVES IS BY THEIR ORIGINAL COST, AT THE MEDIAN. Rank costs ran 1-15
-- and the median was exactly 3, so "3 or under becomes 1, 4 or over becomes 2" divides them 198/181
-- -- near enough half and half without inventing a threshold, and it keeps the native sense of which
-- ranks were the expensive ones.
--
-- ⚠️⚠️ THIS SCRIPT IS NOT IDEMPOTENT AND GUARDS ITSELF. The passive rule reads `cost` to decide the
-- new `cost`, so running it a second time would map the already-repriced 1s and 2s both to 1 and
-- silently flatten the split. The guard below aborts unless it can still see original pricing
-- (any passive rank costing more than 2). To reprice again, restore costs from a dump first.
--
-- ⚠️ Ranks are walked by next_id because rank ids are NOT contiguous (section 10). Pricing the
-- custom trees by RANK POSITION rather than by id is why: their rank ids are not sequential, so
-- "first rank" cannot be inferred from the number.
-- ⚠️ AAs are NOT in shared memory -- they load at ZONE boot, so this needs a zone restart only.
-- =============================================================================================

-- Abort if this has already been applied (see the note above).
SELECT
  CASE WHEN COUNT(*) = 0
    THEN 'ALREADY APPLIED -- aborting. No passive rank costs above 2 remain to reprice.'
    ELSE CONCAT('ok, ', COUNT(*), ' passive ranks still at original pricing')
  END AS guard
FROM aa_ranks r
JOIN aa_ability a ON a.first_rank_id = r.id
WHERE a.enabled = 1 AND a.id NOT IN (2,3,4,6,8,9,10,11,12,13,16,18,19,21,25,28,30,32,33,34,44,47,50,52,60,73,75,89,97,104,108,111,114,128,142,144,154,170,173,180) AND r.cost > 2;

DROP TEMPORARY TABLE IF EXISTS tmp_reprice;
CREATE TEMPORARY TABLE tmp_reprice (rank_id INT PRIMARY KEY, new_cost INT);

-- custom trees: priced by RANK POSITION in the chain
INSERT INTO tmp_reprice (rank_id, new_cost)
WITH RECURSIVE chain AS (
    SELECT r.id AS rank_id, r.next_id, 1 AS pos
    FROM aa_ability a JOIN aa_ranks r ON r.id = a.first_rank_id
    WHERE a.enabled = 1 AND a.id IN (2,3,4,6,8,9,10,11,12,13,16,18,19,21,25,28,30,32,33,34,44,47,50,52,60,73,75,89,97,104,108,111,114,128,142,144,154,170,173,180)
  UNION ALL
    SELECT r2.id, r2.next_id, c.pos + 1
    FROM chain c JOIN aa_ranks r2 ON r2.id = c.next_id WHERE c.pos < 40
)
SELECT rank_id, CASE WHEN pos = 1 THEN 1 WHEN pos <= 3 THEN 2 ELSE 3 END FROM chain;

-- passives: priced by ORIGINAL cost, split at the median
INSERT INTO tmp_reprice (rank_id, new_cost)
WITH RECURSIVE chain AS (
    SELECT r.id AS rank_id, r.next_id, r.cost AS orig_cost, 1 AS pos
    FROM aa_ability a JOIN aa_ranks r ON r.id = a.first_rank_id
    WHERE a.enabled = 1 AND a.id NOT IN (2,3,4,6,8,9,10,11,12,13,16,18,19,21,25,28,30,32,33,34,44,47,50,52,60,73,75,89,97,104,108,111,114,128,142,144,154,170,173,180)
  UNION ALL
    SELECT r2.id, r2.next_id, r2.cost, c.pos + 1
    FROM chain c JOIN aa_ranks r2 ON r2.id = c.next_id WHERE c.pos < 40
)
SELECT rank_id, CASE WHEN orig_cost <= 3 THEN 1 ELSE 2 END FROM chain;

UPDATE aa_ranks r JOIN tmp_reprice t ON t.rank_id = r.id SET r.cost = t.new_cost;

-- ---------------------------------------------------------------- verify
SELECT 'ranks repriced' AS what, COUNT(*) n FROM tmp_reprice
UNION ALL SELECT 'any cost outside 1-3 (must be 0)', COUNT(*)
  FROM aa_ranks r JOIN tmp_reprice t ON t.rank_id = r.id WHERE r.cost NOT BETWEEN 1 AND 3
UNION ALL SELECT 'TOTAL points in the AA window', SUM(r.cost)
  FROM aa_ranks r JOIN tmp_reprice t ON t.rank_id = r.id;

DROP TEMPORARY TABLE tmp_reprice;
