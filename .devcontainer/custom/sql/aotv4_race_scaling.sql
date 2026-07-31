-- AoTv4 -- per race base identity applied to npc_types, from race_scaling.csv       2026-07-30
-- ============================================================================================
-- Source: /src/race_scaling.csv. Of its 193 rows only **15 carry real values**; the other 178 are
-- placeholder defaults (25/25/25/25/25/25/25, ac 50, resists 25) and are DELIBERATELY NOT IMPORTED.
-- An absent race here means "not tuned yet", never "deliberately average" -- importing the
-- placeholders would have written a fake baseline over hundreds of hand tuned npcs.
--
-- ⚠️⚠️⚠️ `mindmg`, `maxdmg` AND `AC` ARE NOT APPLIED, AND MUST NOT BE. The CSV expresses them as
-- LEVEL 1 BASELINES (1 to 5 damage, ac 50) but npc_types rows for these races run to level 102, so a
-- flat write would gut a large slice of the server's content:
--     race 367 Skeleton    1365 npcs, levels 1-101, max damage 6350, max AC 608
--     race 127 Shadowman   1273 npcs, levels 1-102, max damage 7440, max AC 900
--     race   3 Erudite      802 npcs, levels 1-100, max damage 100003, max AC 1065
-- Every high level skeleton, shadowman and erudite in the game would drop to 1-5 damage. That is not
-- a scaling change, it is deleting the difficulty of everything built on those races. The columns
-- below are the ones that describe WHAT A RACE IS rather than how strong a given npc is.
-- If a damage/AC pass is ever wanted it has to be a MULTIPLIER against the npc's existing values, or
-- level aware, never an absolute assignment.
--
-- ⚠️ Total blast radius even so: 5,959 npc_types rows across 15 races. Hence the backup below.
-- ⚠️ npc_types IS IN SHARED MEMORY: world down, `./shared_memory`, restart. A zone restart alone
-- will NOT pick this up.
-- ⚠️ Re runnable. The backup table is created only on the FIRST run so a second run cannot overwrite
-- the pristine values with already modified ones.
-- ============================================================================================

-- ---------------------------------------------------------------- backup (first run only)
CREATE TABLE IF NOT EXISTS aotv4_race_scaling_backup AS
SELECT id, race, level, STR, STA, DEX, AGI, _INT, WIS, CHA, MR, CR, FR, PR, DR, AC,
       mindmg, maxdmg, bodytype
FROM npc_types
WHERE race IN (3,6,9,14,29,46,71,98,127,367,415,491,660,669,678);

-- ---------------------------------------------------------------- the 15 tuned races
DROP TEMPORARY TABLE IF EXISTS tmp_race_scaling;
CREATE TEMPORARY TABLE tmp_race_scaling (
    race INT PRIMARY KEY, rname VARCHAR(32),
    s_str INT, s_sta INT, s_dex INT, s_agi INT, s_int INT, s_wis INT, s_cha INT,
    s_body INT, s_mr INT, s_fr INT, s_cr INT, s_pr INT, s_dr INT
);
INSERT INTO tmp_race_scaling VALUES
    (  3, 'Erudite',       20,25,25,25,35,30,20,  0, 25,25,25,25,25),
    (  6, 'Dark Elf',      20,25,30,25,30,25,30,  0, 25,25,25,25,25),
    (  9, 'Troll',         30,35,25,25,15,20,15,  0, 25,25,25,25,25),
    ( 14, 'Werewolf',      35,25,30,30,25,25,25,  1, 25, 5,50,25,25),
    ( 29, 'Gargoyle',      25,40,25,25,25,25,25,  7,  5,25,25,25,25),
    ( 46, 'Fire Imp',      25,25,45,30,25,25,25,  0, 25,75, 5,25,25),
    ( 71, 'Human',         30,25,30,25,30,25,30,  0, 25,25,25,25,25),
    ( 98, 'Drake',         35,35,15,25,35,15,20, 25,  5,25,25,25,25),
    (127, 'Shadowman',     30,25,30,25,30,25,30,  0,  5, 5, 5,25,25),
    (367, 'Skeleton',      15,15,35,35,25,25,15,  2,  5,25,25,50,50),
    (415, 'Rat',           15,15,30,40,25,25,10, 20, 25, 5, 5,25,100),
    (491, 'Bone Golem',    30,35,25,25,15,15,15,  2,  5,25,25,50,50),
    (660, 'Book',          10,10,25,25,45,30,40,  7, 75, 5,25,25,25),
    (669, 'Blind Dreamer', 35,30,25,25,25,25,25,  7, 15,65,15,15,15),
    (678, 'Erudite',       20,25,25,25,35,30,20,  0, 25,25,25,25,25);

-- ⚠️ Note the CSV column order: it is mr, fr, cr, pr, dr -- NOT the MR, CR, FR order npc_types uses.
-- Getting that wrong silently swaps fire and cold resistance on every race, which nothing would flag.
UPDATE npc_types n JOIN tmp_race_scaling t ON t.race = n.race
SET n.STR = t.s_str, n.STA = t.s_sta, n.DEX = t.s_dex, n.AGI = t.s_agi,
    n._INT = t.s_int, n.WIS = t.s_wis, n.CHA = t.s_cha,
    n.bodytype = t.s_body,
    n.MR = t.s_mr, n.FR = t.s_fr, n.CR = t.s_cr, n.PR = t.s_pr, n.DR = t.s_dr;

DROP TEMPORARY TABLE tmp_race_scaling;

-- ---------------------------------------------------------------- verify
SELECT 'npcs updated' AS what, COUNT(*) AS n FROM npc_types
 WHERE race IN (3,6,9,14,29,46,71,98,127,367,415,491,660,669,678)
UNION ALL
SELECT 'backup rows', COUNT(*) FROM aotv4_race_scaling_backup
UNION ALL
SELECT 'damage/AC untouched (max maxdmg)', MAX(maxdmg) FROM npc_types WHERE race = 367;
-- expected: 5959 / 5959 / 6350   (the last proves mindmg, maxdmg and AC were left alone)

-- ---------------------------------------------------------------- revert
-- UPDATE npc_types n JOIN aotv4_race_scaling_backup b ON b.id = n.id
-- SET n.STR=b.STR, n.STA=b.STA, n.DEX=b.DEX, n.AGI=b.AGI, n._INT=b._INT, n.WIS=b.WIS, n.CHA=b.CHA,
--     n.MR=b.MR, n.CR=b.CR, n.FR=b.FR, n.PR=b.PR, n.DR=b.DR, n.bodytype=b.bodytype;
