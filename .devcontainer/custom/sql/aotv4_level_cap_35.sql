-- AoTv4 -- set the character level cap to 35.
-- =====================================================================================
-- The cap is the highest `max_level` among the regions a character has unlocked
-- (RegionManager::GetMaxLevel), so the regions table IS the cap. The region export that
-- introduced this system shipped every region at 30; the rest of the server was built for 35.
--
-- ⚠️⚠️ THE EXPERIENCE CURVE WAS ALWAYS WRITTEN FOR 35, so 30 was the outlier, not this.
-- zone/exp.cpp states it outright: "629,000 at level 35 (665,000 to complete 35)". The AA
-- conversion, the balance sheet (exp.xlsx) and the delve ladder are all calibrated against that
-- 665,000 figure. Leaving the regions at 30 silently capped the server 5 levels below everything
-- that had been tuned for it.
--
-- ⚠️ There is a SECOND floor in Lua: era_system.M.HARD_CAP. Both must say 35 or the lower wins --
-- era_system deliberately takes the MINIMUM of the era cap, the hard cap and the region ceiling, so
-- changing this table alone would do nothing.
--
-- ⚠️ Regions are loaded by RegionManager::LoadZoneRegions at ZONE BOOT, not from shared memory, so
-- this needs a zone restart. No ./shared_memory rebuild.
--
-- ⚠️ `character_regions_unlocked` is still EMPTY -- nothing grants a starting region yet -- so in
-- practice every character currently falls back to the Lua HARD_CAP. This UPDATE matters the moment
-- regions start being granted, and is set now so the two cannot disagree later.
--
-- Re-runnable: a plain idempotent UPDATE.

UPDATE regions SET max_level = 35;

SELECT id, name, max_level FROM regions ORDER BY id;
