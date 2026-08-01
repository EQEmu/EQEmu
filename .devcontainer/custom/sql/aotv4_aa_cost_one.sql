-- AoTv4 -- every alternate ability rank costs 1 point.
-- =====================================================================================
-- Follows the level cap moving 35 -> 30. A capped death banks experience, not a fixed number of
-- points, so a shorter climb pays less: a full run is now 495,000 experience rather than 665,000,
-- which at 200,000 per point is ~2.48 points instead of ~3.32. Flattening every rank to 1 restores
-- the number of ABILITIES a death buys rather than the number of points.
--
-- Was: 238 ranks at 1, 261 at 2, 80 at 3  =  1,000 points to own everything.
-- Now: 579 ranks at 1                     =    579 points.
--
-- ⚠️⚠️ THIS FLATTENS A DELIBERATE CURVE. The costs were tiered on purpose -- customs at 1/2/3 by
-- rank, natives at 1 or 2 -- so a strong ability cost more than a filler one. With everything at 1
-- that distinction is gone: the picker's only remaining lever is WHICH abilities it offers, not what
-- they cost. If a cost curve is ever wanted back, this script is the thing to stop running; the
-- original values are in aotv4_aa_repricing.sql.
--
-- ⚠️ `aa_choice.lua` spends the private bank at the rank's cost via GrantAlternateAdvancementAbility
-- (ignore_cost = true), so it reads this column too -- there is no second copy of the price to keep
-- in step.
-- ⚠️ RANK_BUDGET in aa_choice.lua is 0 (disabled), so nothing filters offers by total investment.
-- If it is ever re-enabled it is expressed in POINTS and would need re-tuning against these costs.
--
-- ⚠️ Scoped to enabled = 1. Disabled abilities are unreachable anyway, and repricing them would make
-- a later re-enable behave differently from the rest of the pool for no reason.
-- ⚠️ AAs load from the DB at ZONE BOOT, not shared memory: zone restart, no ./shared_memory.
-- ⚠️ Re-runnable: a plain idempotent UPDATE.

-- Backup of the original costs, FIRST RUN ONLY, so a second run cannot overwrite the pristine values
-- with the already-flattened ones. Same pattern as aotv4_race_scaling.sql.
-- ⚠️ EVERY rank, not just the enabled pool. A first version tried to scope this with
--     JOIN aa_ability a ON a.first_rank_id = r.id OR r.id IN (SELECT id FROM aa_ranks)
-- whose second clause is ALWAYS TRUE, so it joined every rank to every enabled ability and only the
-- GROUP BY made the result look sane. Backing up the whole table is simpler, cannot be wrong, and
-- costs nothing -- a restore is still exact because only the 579 were ever modified.
CREATE TABLE IF NOT EXISTS aotv4_aa_cost_backup AS
SELECT id AS rank_id, cost FROM aa_ranks;

-- ⚠️ Walk the chain via next_id -- rank ids are NOT contiguous (CLAUDE.md section 10), and
-- aa_ranks has no aa_id column, so "all ranks of an enabled ability" cannot be expressed as a join.
UPDATE aa_ranks r
JOIN (
    WITH RECURSIVE chain AS (
        SELECT a.id AS aa_id, rr.id AS rank_id, rr.next_id
        FROM   aa_ability a JOIN aa_ranks rr ON rr.id = a.first_rank_id
        WHERE  a.enabled = 1
        UNION ALL
        SELECT c.aa_id, rr.id, rr.next_id
        FROM   chain c JOIN aa_ranks rr ON rr.id = c.next_id
        WHERE  c.next_id > 0
    )
    SELECT DISTINCT rank_id FROM chain
) x ON x.rank_id = r.id
SET r.cost = 1;

SELECT cost, COUNT(*) AS ranks FROM aa_ranks r
WHERE r.id IN (
  WITH RECURSIVE chain AS (
    SELECT a.id AS aa_id, rr.id AS rank_id, rr.next_id
    FROM aa_ability a JOIN aa_ranks rr ON rr.id = a.first_rank_id WHERE a.enabled = 1
    UNION ALL
    SELECT c.aa_id, rr.id, rr.next_id FROM chain c JOIN aa_ranks rr ON rr.id = c.next_id WHERE c.next_id > 0
  ) SELECT rank_id FROM chain)
GROUP BY cost ORDER BY cost;
