-- aotv4_aa_ranged.sql -- the four remaining Ranged AAs, and the spells they need.
-- =============================================================================================
-- Kindred Bond (host 32) is the fifth and lives in aotv4_aa_ranged_kindred.sql.
--
--   host 44  Quick Damage       ranks 141,142,143,12863,15396  -> Overload          [MARKER]
--   host 50  Rabid Bear         ranks 153,1519,5068,6101,7468  -> Second Wind       [ACTIVATED]
--   host 33  Combat Stability   ranks 122,123,124,454,455      -> Corrosion         [MARKER]
--   host 34  Combat Agility     ranks 125,126,127,449,450      -> Concussive Burst  [MARKER]
--
-- Chains walked, not assumed. ⚠️ Marker rank ids 141, 153, 122 and 125 are the ONLY join to
-- zone/aotv4_ranged_aa.cpp and nothing checks them.
--
-- ⚠️⚠️ SECOND WIND TAKES TIMER SLOT 3. spell_type is a TIMER SLOT, not a category -- the recast is
-- keyed on `rank->spell_type + pTimerAAStart`, so two activated AAs sharing a value share ONE
-- cooldown. Slots now in use: 2 Sanguine Frenzy, 3 Second Wind, 6 Sanctified Blow, 14 Iron Will.
-- Rabid Bear already ships on 3, so it is left alone -- but check this list before adding a fifth.
--
--   mysql -h127.0.0.1 -upeq -ppeqpass peq < .devcontainer/custom/sql/aotv4_aa_ranged.sql
--
-- ⚠️ spells_new IS in shared memory: stop world + zones, run ./shared_memory, restart.
-- =============================================================================================

DELETE FROM spells_new WHERE id BETWEEN 43430 AND 43437;
DELETE FROM db_str     WHERE id BETWEEN 43430 AND 43437 AND type = 6;

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;
CREATE TEMPORARY TABLE aotv4_tmpl LIKE spells_new;

-- --------------------------------------------------------- 43430-43435 Corrosion's resist debuffs
-- One per resist type. The code picks whichever matches the damage-over-time that ticked, read from
-- that spell's OWN resisttype, so a fire line erodes fire resistance and a poison line erodes poison.
--
-- ⚠️ RESIST DEBUFFS USE A NEGATIVE BASE. SPA 46 fire / 47 cold / 48 poison / 49 disease / 50 magic /
-- 111 all. A positive value would BUFF the target's resistance, which is a silent own-goal.
-- ⚠️ resisttype 0 on these rows: the debuff itself must not be resistable, or the thing you are
-- trying to make easier to hit gets a save against being made easier to hit.
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 4088;
UPDATE aotv4_tmpl SET
  id = 43430, name = 'Corroded Wards', descnum = 43430, spellgroup = 43430, `rank` = 1, mana = 0,
  effectid1 = 50, effect_base_value1 = -20, effect_limit_value1 = 0, max1 = 0, formula1 = 100,
  effectid2 = 254, effect_base_value2 = 0,
  buffduration = 4, buffdurationformula = 10, cast_time = 0, resisttype = 0,
  targettype = 5, goodEffect = 0, new_icon = 39, spell_category = -99,
  classes1=255, classes2=255, classes3=255, classes4=255, classes5=255, classes6=255,
  classes7=255, classes8=255, classes9=255, classes10=255, classes11=255, classes12=255,
  classes13=255, classes14=255, classes15=255, classes16=255;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id=43430;
UPDATE aotv4_tmpl SET id=43431, name='Scorched Wards', descnum=43431, spellgroup=43431, effectid1=46;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id=43430;
UPDATE aotv4_tmpl SET id=43432, name='Frostbitten Wards', descnum=43432, spellgroup=43432, effectid1=47;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id=43430;
UPDATE aotv4_tmpl SET id=43433, name='Envenomed Wards', descnum=43433, spellgroup=43433, effectid1=48;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id=43430;
UPDATE aotv4_tmpl SET id=43434, name='Festering Wards', descnum=43434, spellgroup=43434, effectid1=49;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;
-- The catch-all, for chromatic / prismatic / corruption lines. Weaker, since it hits everything.
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id=43430;
UPDATE aotv4_tmpl SET id=43435, name='Sundered Wards', descnum=43435, spellgroup=43435,
  effectid1=111, effect_base_value1=-10;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

-- ------------------------------------------------------------------ 43436 Concussive Burst's stun
-- ⚠️ targettype 4 = ST_AECaster: point blank, centred on the caster. That is the whole point --
-- it is for shaking off things that have surrounded you.
-- ⚠️ SPA 21's MAX field is the highest target LEVEL it can stun, and 0 does NOT mean unlimited --
-- it silently falls back to RuleI(Spells, BaseImmunityLevel), which is 55. Set explicitly or this
-- quietly stops working in the fifties. Targets with StunImmunity are immune regardless, which is
-- why this is an escape from adds rather than a raid tool.
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43406;
UPDATE aotv4_tmpl SET
  id = 43436, name = 'Concussive Burst', descnum = 43436, spellgroup = 43436, `rank` = 1,
  effect_base_value1 = 3000, max1 = 70, formula1 = 100,
  targettype = 4, aoerange = 40, cast_time = 0, resisttype = 0;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

-- ------------------------------------------------------------------- 43437 Second Wind's vehicle
-- Activation demands a valid spell; the conversion itself is in code. Almost inert on purpose --
-- SPA 15 at 1 mana per tick, just enough to be a real buff rather than an all-254 shell, and to
-- show the player something happened.
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43430;
UPDATE aotv4_tmpl SET
  id = 43437, name = 'Second Wind', descnum = 43437, spellgroup = 43437,
  effectid1 = 15, effect_base_value1 = 1, max1 = 0, formula1 = 100,
  buffduration = 1, targettype = 6, goodEffect = 1, new_icon = 63;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

INSERT INTO db_str (id,type,value) VALUES
 (43430,6,'Its defences against magic are eaten away.'),
 (43431,6,'Its defences against fire are eaten away.'),
 (43432,6,'Its defences against cold are eaten away.'),
 (43433,6,'Its defences against poison are eaten away.'),
 (43434,6,'Its defences against disease are eaten away.'),
 (43435,6,'Its defences are eaten away.'),
 (43436,6,'The air cracks outward, and everything near you reels.'),
 (43437,6,'Weariness spent, and turned into will.');

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;

-- =============================================================================================
UPDATE aa_ability SET name='Overload',         classes=65535, enabled=1, type=3 WHERE id=44;
UPDATE aa_ability SET name='Second Wind',      classes=65535, enabled=1, type=3 WHERE id=50;
UPDATE aa_ability SET name='Corrosion',        classes=65535, enabled=1, type=3 WHERE id=33;
UPDATE aa_ability SET name='Concussive Burst', classes=65535, enabled=1, type=3 WHERE id=34;

UPDATE aa_ranks SET level_req=5,  cost=3 WHERE id IN (141, 153,  122, 125);
UPDATE aa_ranks SET level_req=15, cost=4 WHERE id IN (142, 1519, 123, 126);
UPDATE aa_ranks SET level_req=25, cost=5 WHERE id IN (143, 5068, 124, 127);
UPDATE aa_ranks SET level_req=35, cost=6 WHERE id IN (12863, 6101, 454, 449);
UPDATE aa_ranks SET level_req=45, cost=8 WHERE id IN (15396, 7468, 455, 450);

UPDATE aa_ranks SET next_id=-1 WHERE id IN (15396, 7468, 455, 450);

DELETE FROM aa_rank_effects WHERE rank_id IN
 (141,142,143,12863,15396, 153,1519,5068,6101,7468, 122,123,124,454,455, 125,126,127,449,450);

-- Second Wind is the only activated one. 5 minute recast; endurance regenerates, so a shorter one
-- would make it a mana battery rather than a decision.
UPDATE aa_ranks SET spell=43437, recast_time=300, spell_type=3 WHERE id IN (153,1519,5068,6101,7468);

-- ⚠️⚠️ title_sid AND desc_sid ARE INDEPENDENT, AND NEITHER IS GUARANTEED TO EQUAL first_rank_id.
-- Every other host in every tree happens to have title_sid = desc_sid = first_rank_id, which made
-- that look like a rule. Quick Damage does not: its ranks carry title_sid 141 but desc_sid **12863**.
-- The result was an AA that showed the right NAME and the WRONG DESCRIPTION -- it kept Quick Damage's
-- "reduces the casting time on your damage spells" text, because that is what lives at 12863, while
-- the text written to 141 was never read by anything.
-- Repointed here so the description comes from the same sid as the name, matching every other AA.
-- ⚠️ Check `SELECT title_sid, desc_sid FROM aa_ranks` on any NEW host before writing its db_str.
UPDATE aa_ranks SET desc_sid=141 WHERE id IN (141,142,143,12863,15396);

UPDATE db_str SET value='Overload'         WHERE id=141 AND type=1;
UPDATE db_str SET value='Second Wind'      WHERE id=153 AND type=1;
UPDATE db_str SET value='Corrosion'        WHERE id=122 AND type=1;
UPDATE db_str SET value='Concussive Burst' WHERE id=125 AND type=1;

UPDATE db_str SET value='Now and then the magic gets away from you. Direct damage spells have a 5, 10, 15, 20 and 25 percent chance to land for half as much again. This is its own roll and stacks with a critical rather than replacing one.' WHERE id=141 AND type=4;
UPDATE db_str SET value='Spend the last of your stamina and turn it into will. Consumes all of your endurance and returns 40, 55, 70, 85 and 100 percent of it as mana. Mana over your maximum is lost, so do not use it while full. Usable once every 5 minutes.' WHERE id=153 AND type=4;
UPDATE db_str SET value='What you inflict makes the next thing easier. Each tick of your damage over time has a 10, 15, 20, 25 and 30 percent chance to erode whichever resistance that spell is checked against, so fire wears down fire and poison wears down poison.' WHERE id=122 AND type=4;
UPDATE db_str SET value='When you are hurt badly the air cracks outward from you, stunning everything nearby. Triggers when you fall below 30 percent health, once every 5, 4, 3, 2 and a half, and 2 minutes. It will not hold the largest creatures.' WHERE id=125 AND type=4;
