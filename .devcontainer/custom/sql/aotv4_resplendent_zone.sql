-- AoTv4 -- Resplendent Temple: Classic expansion, no combat, low gravity.
-- =====================================================================================
-- The starting hub. Three NPCs stand here (Herald, Wayfinder, Reforger) and nothing else should
-- happen in it.
--
-- ⚠️ `expansion` 18 -> 0. That column is metadata used for grouping and display; the fields that
-- actually CONTENT-FILTER a zone are min_expansion / max_expansion, both already -1 (always spawns).
-- Setting it to 0 makes the row say Classic, consistent with Expansion:CurrentExpansion = 0.
--
-- ⚠️⚠️ `cancombat` IS GENUINELY ENFORCED -- do not mistake it for a display flag. Zone::CanDoCombat()
-- reads it and is the FIRST test in Mob::IsAttackAllowed (zone/aggro.cpp:730), with 19 call sites
-- across melee, detrimental spells and bot logic. Setting it 0 makes the whole zone unattackable
-- rather than merely labelling it.
-- 📌 The method is CanDoCombat, NOT CanCombat -- grepping the column name finds only the Perl/Lua
-- accessor and makes the flag look inert, which is exactly the wrong conclusion.
--
-- ⚠️ `gravity` 0.4 -> 0.5. Zone-wide fall/jump behaviour.
--
-- ⚠️ Zone rows are read at ZONE BOOT, not shared memory: a zone restart is enough. But the hub is
-- usually already booted with players in it, so those zone processes must be killed to pick this up.

UPDATE zone
SET    expansion = 0,
       cancombat = 0,
       gravity   = 0.5
WHERE  zoneidnumber = 729;

SELECT zoneidnumber, short_name, expansion, min_expansion, max_expansion,
       cancombat, gravity, canbind, canlevitate
FROM   zone WHERE zoneidnumber = 729;
