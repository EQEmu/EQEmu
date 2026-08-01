-- =============================================================================================
-- AoTv4 -- Luclin/PoP passive AAs: 55 abilities, all usable at level 1              2026-07-31
--
-- Re-enables 54 NATIVE passive abilities that already exist in `aa_ability` but ship disabled.
-- No new rows and no new ids. Classic flat innates only: Innate Strength / Charisma / Intelligence /
-- Regeneration / Enlightenment / Metabolism, Mental Clarity, Body and Mind Rejuvenation, Combat
-- Fury, Ferocity, Knight's Advantage, Fury of Magic, Ingenuity, the Spell Casting lines, Physical
-- Enhancement, Planar Durability, Punishing Blade, Ambidexterity, Dead Aim, Fear Resistance,
-- Stalwart Endurance, Theft of Life and the tradeskill masteries.
--
-- â ï¸â ï¸ THE TARGET IS **POINTS OF SINK**, NOT A COUNT OF ABILITIES. The brief was "enough AAs to
-- spend 200 points on". These abilities carry hundreds of ranks worth well over a thousand points --
-- many times the 200 required. Reading it as "200 abilities" is what drove three earlier attempts
-- out into Gates of Discord, Omens of War and beyond, dragging in exactly the material that was then
-- rejected. Luclin and Planes of Power alone are oversupplied for this purpose.
--
-- â ï¸ FOUR EXCLUSIONS, all applied by hand because none is detectable by SPA alone:
--   1. Abilities that enhance ONE named ability instead of adding a flat stat -- Consumption of the
--      Soul ("enhance their Leech Touch ability"), Blur of Axes, Archery Mastery, Technique of
--      Master Wu, Endless Quiver, Headshot, Pet Discipline, Jam Fest, Shroud of Stealth.
--   2. Reuse-timer reducers INCLUDING THE ONES NOT CALLED "Hastened" -- Blessing of the Devoted,
--      Fervent Blessing, Hasty Exit, Rush to Judgment and Touch of the Wicked are all SPA 264, the
--      same effect as the Hastened family under different names. A name filter misses them.
--   3. Mirror-pair duplicates. The Keepers and Dark Reign versions of Gift, Power, Embrace, Sanctity
--      and Valor are identical in SPA and base value, differing only in faction flavour text.
--   4. Bandage healing -- First Aid, Bandage Wound, Mithaniel's Binding.
-- ⚠ Chaotic Stab is an EXCEPTION to rule 1 and is included by request: it modifies the Backstab
-- skill rather than adding a flat stat, but what it actually removes is a POSITIONAL requirement
-- (backstab from the front for reduced damage), which plays as quality of life rather than scaling.
-- â ï¸ Class-limited is NOT an exclusion and must not become one: Combat Fury is melee-only and Spell
-- Casting Mastery caster-only by native design, and Thief's Intuition is two rows because Bards and
-- Rogues get different values. `classes` was also touched by the older AA migration (section 6), so
-- it is not a reliable signal of anything.
--
-- â ï¸â ï¸ PREREQUISITES ARE CLEARED AND THEN SELECTIVELY RESTORED, and the restore is the important
-- half. Clearing ALL of them (the first attempt) broke native progressions and produced AAs that
-- strictly dominate each other: **Fearless grants 100 percent fear immunity and natively REQUIRED
-- Fear Resistance at 3 points**, so with the prereq gone it was directly buyable and the three ranks
-- of Fear Resistance became worthless. Same for Mastery of the Past over Spell Casting Expertise and
-- Fury of Magic over Spell Casting Fury. Only prereqs pointing at abilities that are ALSO enabled
-- here are restored -- a prereq aimed at a still-disabled ability is what makes a rank appear in the
-- window and then refuse to train with no message at all (section 10).
--
-- â ï¸â ï¸ EVERY RANK IS SET TO level_req 1, NOT JUST THE FIRST. Nearly all were gated above level 35;
-- lowering only `first_rank_id` leaves every later rank unreachable, which reads as the AA being
-- broken because the window shows the ranks and simply refuses them.
-- â ï¸ Zero-cost ranks are raised to 1 -- a free AA is not a decision.
-- â ï¸ AAs are NOT in shared memory: they load at ZONE boot, so this needs a zone restart only.
-- â ï¸ Stock rows with stock `db_str` text, so nothing needs re-exporting to the client.
-- â ï¸ Re-runnable. Never touches the 40 hand-built AoTv4 AAs.
-- =============================================================================================

-- Turn off everything the earlier, wider passes enabled. Never touches the AoTv4 40.
UPDATE aa_ability SET enabled = 0 WHERE id IN (1,3,5,7,15,17,20,21,23,26,27,28,31,42,45,48,49,55,56,67,75,78,79,81,82,84,90,94,97,100,101,103,112,114,118,119,120,121,122,123,124,125,140,141,142,143,144,150,154,166,181,186,187,188,189,190,193,194,195,196,197,198,200,201,202,203,204,205,206,210,211,213,214,215,218,221,222,223,224,225,227,229,233,234,235,236,237,238,248,249,250,251,252,258,259,262,263,264,266,267,268,269,270,271,272,273,274,275,278,281,283,284,287,288,292,293,294,296,297,299,301,302,309,310,311,312,314,315,316,319,324,325,326,327,328,329,330,333,334,335,336,338,339,341,347,348,352,353,354,358,360,364,366,367,368,370,371,375,377,378,379,380,381,382,388,389,397,398,406,410,419,427,434,435,437,438,439,440,442,448,457,460,466,471,472,473,474,475,476,477,478,479,480,488,495,496,497,498,501,504,507,508,509,516,522,525,526,530,532,537,539,540,545,546,549,555,557,560,562,563,568,569,575,576,579,581,582,600,637,663,667,703,704,713,716,718,737,738,765,790,807,823,842,843,862,875,878,918,928,930,942,989,991,995,1022,1115,1177,1204,1206,1207,1208,1214,1218,1219,1220,1221,1222,1223,1232,1237,1245,1246,1247,1250,1258,1271,1281,1300,1302,1304,1305,1311,1433,1662,1665,1667,2020,2025,2032,2036,2041,2042,2056,2059,2208,3511,3513,3517,3525,3600,3716,3724,3725,3727,3733,3813,3815,3819,3821,3823,3824,3832,3838,3843,3865,4001,5005,6002,7060,7105,7106,7695,7699,8203,8331,8335,8502,8802,9201,9205,9206,9207,9301,9404,9602,10434) AND id NOT IN (2,3,4,6,8,9,10,11,12,13,16,18,19,21,25,28,30,32,33,34,44,47,50,52,60,73,75,89,97,104,108,111,114,128,142,144,154,170,173,180);

DROP TEMPORARY TABLE IF EXISTS tmp_bloat_aa;
CREATE TEMPORARY TABLE tmp_bloat_aa (aa_id INT PRIMARY KEY);
INSERT INTO tmp_bloat_aa (aa_id) VALUES (1),(5),(7),(15),(20),(23),(26),(27),(31),(42),(48),(49),(56),(78),(81),(103),(119),(120),(122),(124),(141),(143),(150),(186),(187),(188),(189),(202),(203),(205),(210),(213),(214),(215),(222),(223),(224),(225),(309),(310),(311),(315),(316),(367),(370),(476),(477),(478),(479),(480),(568),(569),(576),(5005),(6002);

-- Ranks are walked by next_id because rank ids are NOT contiguous (section 10).
DROP TEMPORARY TABLE IF EXISTS tmp_bloat_ranks;
CREATE TEMPORARY TABLE tmp_bloat_ranks (rank_id INT PRIMARY KEY);
INSERT INTO tmp_bloat_ranks (rank_id)
WITH RECURSIVE chain AS (
    SELECT r.id AS rank_id, r.next_id, 1 AS depth
    FROM tmp_bloat_aa t JOIN aa_ability a ON a.id = t.aa_id JOIN aa_ranks r ON r.id = a.first_rank_id
  UNION ALL
    SELECT r2.id, r2.next_id, c.depth + 1
    FROM chain c JOIN aa_ranks r2 ON r2.id = c.next_id WHERE c.depth < 40
)
SELECT DISTINCT rank_id FROM chain;

UPDATE aa_ranks SET level_req = 1 WHERE id IN (SELECT rank_id FROM tmp_bloat_ranks);
UPDATE aa_ranks SET cost = 1 WHERE cost < 1 AND id IN (SELECT rank_id FROM tmp_bloat_ranks);

-- clear, then restore only the satisfiable ones (see the warning above)
DELETE FROM aa_rank_prereqs WHERE rank_id IN (SELECT rank_id FROM tmp_bloat_ranks);
INSERT IGNORE INTO aa_rank_prereqs (rank_id, aa_id, points) VALUES (195,31,3),(446,26,3),(447,26,3),(448,26,3),(637,23,3),(638,23,3),(639,23,3),(770,23,3),(771,23,3),(772,23,3),(4749,23,3),(4750,23,3),(4751,23,3),(5571,23,3),(5572,23,3),(5573,23,3),(7050,26,3),(7051,26,3),(7052,26,3),(7053,26,3),(7054,26,3),(7055,26,3),(7063,26,3),(7064,26,3),(7065,26,3),(7622,26,3),(7623,26,3),(7624,26,3),(10473,26,3),(10474,26,3),(10475,26,3),(10476,26,3),(10477,26,3),(12435,23,3),(12436,23,3),(12437,23,3),(12553,23,3),(12554,23,3),(12555,23,3),(13308,26,3),(14361,23,3);

UPDATE aa_ability SET enabled = 1 WHERE id IN (SELECT aa_id FROM tmp_bloat_aa);

-- ---------------------------------------------------------------- verify
SELECT 'abilities enabled by this script' AS what, COUNT(*) n FROM aa_ability
  WHERE enabled = 1 AND id IN (SELECT aa_id FROM tmp_bloat_aa)
UNION ALL SELECT 'ranks to purchase', COUNT(*) FROM tmp_bloat_ranks
UNION ALL SELECT 'TOTAL POINTS OF SINK (target was 200)', SUM(cost) FROM aa_ranks
  WHERE id IN (SELECT rank_id FROM tmp_bloat_ranks)
UNION ALL SELECT 'progression prereqs restored', COUNT(*) FROM aa_rank_prereqs
  WHERE rank_id IN (SELECT rank_id FROM tmp_bloat_ranks)
UNION ALL SELECT 'unsatisfiable prereqs (must be 0)', COUNT(*) FROM aa_rank_prereqs p
  WHERE p.rank_id IN (SELECT rank_id FROM tmp_bloat_ranks)
    AND p.aa_id NOT IN (SELECT aa_id FROM tmp_bloat_aa)
UNION ALL SELECT 'ranks still above level 1 (must be 0)', COUNT(*) FROM aa_ranks
  WHERE id IN (SELECT rank_id FROM tmp_bloat_ranks) AND level_req > 1
UNION ALL SELECT 'bandage AAs enabled (must be 0)', COUNT(*) FROM aa_ability
  WHERE enabled = 1 AND name IN ('First Aid','Bandage Wound','Mithaniel''s Binding')
UNION ALL SELECT 'AoTv4 designed AAs still enabled (must be 40)', COUNT(*) FROM aa_ability
  WHERE enabled = 1 AND id IN (2,3,4,6,8,9,10,11,12,13,16,18,19,21,25,28,30,32,33,34,44,47,50,52,60,73,75,89,97,104,108,111,114,128,142,144,154,170,173,180);

DROP TEMPORARY TABLE tmp_bloat_aa;
DROP TEMPORARY TABLE tmp_bloat_ranks;
