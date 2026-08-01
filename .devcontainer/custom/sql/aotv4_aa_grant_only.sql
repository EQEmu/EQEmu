-- AoTv4 -- hide UNTRAINED alternate abilities from the client's native AA window.
-- =====================================================================================
-- Symptom this fixes: the AA window listed every ability in the pool at 0/N, so a player could
-- browse (and, with points in the native pool, BUY) the whole catalogue directly -- bypassing the
-- death picker entirely and making the random offer pointless.
--
-- grant_only = 1 makes the native window list an ability only once it has been TRAINED, and refuse
-- to sell it there. Progression then runs exclusively through the picker, which is the design.
--
-- ⚠️⚠️ THIS DOES NOT BLOCK OUR OWN PICKER, and that is the whole reason it is safe.
-- aa_choice.lua line 294 calls GrantAlternateAdvancementAbility(aa_id, cur + 1, true) -- the third
-- argument is ignore_cost, which makes CanPurchase run with check_grant = false, and grant_only is
-- only ever consulted under check_grant = true. So the picker grants these normally while the
-- native window cannot sell them. Change that call to ignore_cost = false and every grant in the
-- game silently starts failing with no message (see CLAUDE.md section 10, "AA grant rejected
-- silently"), so the two must be read together.
--
-- ⚠️ Scoped to enabled = 1 on purpose. Disabled abilities are already invisible, and flagging them
-- would only make the eventual re-enable of one behave differently from the rest of the pool for
-- no reason.
--
-- ⚠️ AAs are NOT in shared memory -- the zone loads aa_ability/aa_ranks straight from the DB at
-- boot. So this needs a ZONE RESTART, not a ./shared_memory rebuild, and not #reloadquest.
--
-- Re-runnable: a plain idempotent UPDATE, it creates nothing and deletes nothing.

UPDATE aa_ability
SET    grant_only = 1
WHERE  enabled = 1;

-- Report what the window will now show. trained_only should cover the entire enabled pool; the
-- "visible untrained" count is what the player was complaining about and must read 0.
SELECT COUNT(*)                                        AS enabled_abilities,
       SUM(grant_only = 1)                             AS trained_only,
       SUM(grant_only = 0)                             AS visible_untrained
FROM   aa_ability
WHERE  enabled = 1;
