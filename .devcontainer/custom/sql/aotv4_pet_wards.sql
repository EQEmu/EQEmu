-- aotv4_pet_wards.sql -- every pet now carries a standing self-buff suited to what it is.
-- =============================================================================================
-- Stock EQ gives only the Magician fire pet anything of the sort (its damage shield). This gives
-- every family its own, so a pet is identifiably a fire pet or a skeleton or a warder beyond its
-- model, and so the Ranged AA "Kindred Bond" has something worth sharing with its owner.
--
-- ⚠️ HELPER BAND. 43420-43429 sit above 43350, so gen_stock_pool.pl never pulls them into the
-- level-up reward pool. None of these are castable by a player.
--
-- The family is read from the `pets`.`type` string at summon time, NOT from the owner's class --
-- a Magician has four thematically different pets and they must not all get the same buff.
-- Matching is by PREFIX, so every rank of a line (SumFireR10 .. SumFireR37) resolves together.
-- Mapping lives in zone/aotv4_pet_aa.cpp; the prefixes there and the ids here must agree.
--
-- ⚠️⚠️ EVERYTHING HERE SCALES WITH THE PET'S LEVEL, AND THAT IS NOT OPTIONAL. Pets arrive at level
-- ONE -- 285 Pendril's Animation and 338 Cavorting Bones are both level 1 spells, and the Magician
-- elementalkin line starts at 2. A flat value tuned for the mid levels is not merely strong down
-- there, it is decisive: 5 points of damage reduction against a mob that hits for 6, or a 150 point
-- absorb against a mob with 40 health, simply ends the fight. The first draft of this file made
-- exactly that mistake.
--
-- The scaling is done by the spell FORMULA field, so the engine does the work and no extra rows are
-- needed (Mob::CalcSpellEffectValue_formula):
--     100 = flat            101 = base + level/2      103 = base + level*2
--     109 = base + level/4
-- max_value is the ceiling and IS enforced in both directions; a negative base forces a negative
-- result, which is what lets the damage shield scale downward correctly.
--
--   id     ward              family            effect                       lvl 1   lvl 25  lvl 50
--   43420  Cinder Aura       SumFire*          SPA 59  damage shield          -1      -7     -13
--   43421  Tidal Renewal     SumWater*         SPA 0   health regen            1       7      13
--   43422  Stoneflesh        SumEarth*         SPA 69  maximum health         12      60     110
--   43423  Gale Fervor       SumAir*           SPA 11  attack speed          110pct  110pct 110pct
--   43424  Graveborn Hunger  skel_pet_*, etc.  SPA 178 melee lifetap           8pct    8pct   8pct
--   43425  Feral Ferocity    BLpet*            SPA 185 melee damage            8pct    8pct   8pct
--   43426  Arcane Weave      Animation*        SPA 55  absorb rune             7      55     105
--   43427  Spirits Grace     SpiritWolf*       SPA 172 avoidance               1       7      13
--   43428  Kindred Insight   Familiar*         SPA 15  mana regen              1       7      13
--   43429  Bound Servant     anything else     SPA 69  maximum health          5      17      30
--
-- The three PERCENTAGE effects (11, 178, 185) are left flat on purpose: a percentage is already
-- level-appropriate, because it is a share of output that grows on its own. They were still tuned
-- DOWN from the first draft -- 25 percent haste on a level 2 pet is proportionate and still absurd.
--
-- ⚠️ EARTH AND THE FALLBACK USE SPA 69 (max health), NOT SPA 162 (flat damage reduction), even
-- though 162 fits "stoneskin" better. SPA 162 keeps its magnitude in the LIMIT field, and the
-- formula system only scales BASE -- so a 162 ward cannot be made to grow with level without one
-- row per level band. Max health scales cleanly and is just as much "this pet is hard to kill".
--
-- ⚠️ SPA 11 IS THE RESULTING SPEED, NOT THE BONUS. 110 means "attacks at 110 percent"; a value
-- BELOW 100 is a slow. This is the field that reads backwards, so it is easy to build a debuff by
-- accident.
--
-- ⚠️ SPA 59 damage shields use a NEGATIVE base, which is the native convention.
--
-- Durations are effectively permanent (6000 ticks). The lifetime is owned by the code: the ward
-- goes on at summon and the owner's copy is stripped when the pet dies.
--
--   mysql -h127.0.0.1 -upeq -ppeqpass peq < .devcontainer/custom/sql/aotv4_pet_wards.sql
--
-- ⚠️ spells_new IS in shared memory: stop world + zones, run ./shared_memory, restart.
-- =============================================================================================

DELETE FROM spells_new WHERE id BETWEEN 43420 AND 43429;
DELETE FROM db_str     WHERE id BETWEEN 43420 AND 43429 AND type = 6;

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;
CREATE TEMPORARY TABLE aotv4_tmpl LIKE spells_new;

-- Template 4088 Ward of Vie: single target, beneficial, buff. targettype 5 rather than self,
-- because the same row is applied to the pet AND handed to the owner by the AA.
-- One INSERT per row through the temp table so all ~236 columns stay identical to a working spell.

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 4088;
UPDATE aotv4_tmpl SET
  id = 43420, name = 'Cinder Aura', descnum = 43420, spellgroup = 43420, `rank` = 1, mana = 0,
  effectid1 = 59, effect_base_value1 = -1, effect_limit_value1 = 0, max1 = -25, formula1 = 109,
  effectid2 = 254, effect_base_value2 = 0, effect_limit_value2 = 0, max2 = 0,
  buffduration = 6000, buffdurationformula = 10, cast_time = 0,
  targettype = 5, goodEffect = 1, new_icon = 106, spell_category = -99,
  classes1=255, classes2=255, classes3=255, classes4=255, classes5=255, classes6=255,
  classes7=255, classes8=255, classes9=255, classes10=255, classes11=255, classes12=255,
  classes13=255, classes14=255, classes15=255, classes16=255;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43420;
UPDATE aotv4_tmpl SET id=43421, name='Tidal Renewal', descnum=43421, spellgroup=43421,
  effectid1 = 0, effect_base_value1 = 1, max1 = 40, formula1 = 109, new_icon = 99;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43420;
UPDATE aotv4_tmpl SET id=43422, name='Stoneflesh', descnum=43422, spellgroup=43422,
  effectid1 = 69, effect_base_value1 = 10, effect_limit_value1 = 0, max1 = 600, formula1 = 103, new_icon = 128;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43420;
UPDATE aotv4_tmpl SET id=43423, name='Gale Fervor', descnum=43423, spellgroup=43423,
  effectid1 = 11, effect_base_value1 = 110, max1 = 110, formula1 = 100, new_icon = 61;   -- 110 = attacks at 110 percent
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43420;
UPDATE aotv4_tmpl SET id=43424, name='Graveborn Hunger', descnum=43424, spellgroup=43424,
  effectid1 = 178, effect_base_value1 = 8, max1 = 8, formula1 = 100, new_icon = 68;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43420;
UPDATE aotv4_tmpl SET id=43425, name='Feral Ferocity', descnum=43425, spellgroup=43425,
  effectid1 = 185, effect_base_value1 = 8, max1 = 8, formula1 = 100, new_icon = 49;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

-- Arcane Weave carries a real absorb pool. Unlike Sanctified Ward the size is NOT rewritten from
-- code, so the base here is the actual figure.
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43420;
UPDATE aotv4_tmpl SET id=43426, name='Arcane Weave', descnum=43426, spellgroup=43426,
  effectid1 = 55, effect_base_value1 = 5, max1 = 300, formula1 = 103, new_icon = 77;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43420;
UPDATE aotv4_tmpl SET id=43427, name='Spirits Grace', descnum=43427, spellgroup=43427,
  effectid1 = 172, effect_base_value1 = 1, max1 = 15, formula1 = 109, new_icon = 87;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43420;
UPDATE aotv4_tmpl SET id=43428, name='Kindred Insight', descnum=43428, spellgroup=43428,
  effectid1 = 15, effect_base_value1 = 1, max1 = 20, formula1 = 109, new_icon = 63;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43420;
UPDATE aotv4_tmpl SET id=43429, name='Bound Servant', descnum=43429, spellgroup=43429,
  effectid1 = 69, effect_base_value1 = 5, effect_limit_value1 = 0, max1 = 300, formula1 = 101, new_icon = 128;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

INSERT INTO db_str (id,type,value) VALUES
 (43420,6,'Heat rolls off it in waves, and whatever strikes it is burned for the trouble.'),
 (43421,6,'What is wounded closes over, the way water closes over a stone.'),
 (43422,6,'Its hide is not hide at all, and blows land on it like blows land on a wall.'),
 (43423,6,'It moves the way weather moves, and strikes far more often than its size suggests.'),
 (43424,6,'It takes what it kills, and is a little less dead for every wound it opens.'),
 (43425,6,'There is nothing tame in it. Its blows land with a hunter conviction.'),
 (43426,6,'Threads of the spell that made it hang about it still, and turn aside what would unmake it.'),
 (43427,6,'It is only half here. Blows meant for it find rather less than they expected.'),
 (43428,6,'Its attention is your attention, and what it notices returns to you as clarity.'),
 (43429,6,'Bound in service, and bound a little against harm.');

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;
