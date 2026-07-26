-- aotv4_promised_line.sql -- low-level tiers for the "Promised" delayed-bloom heal line.
-- =============================================================================================
-- Cast it on yourself (or an ally), wait, and it blooms into a heal when the buff expires.
--
-- WHAT ALREADY EXISTS: the native mechanic is the Promised line, driven by SPA 289
-- CastOnFadeEffect ("triggers only if fades after natural duration", common/spdat.h:1352). But it
-- is unreachable here:
--   * the whole family starts at level 73 (9755 Promised Renewal, Cleric 73), and
--   * every tier ABOVE it is id >= 10000, which the reward pool filter (id < 10000) excludes.
-- So exactly ONE bloom heal is in pool range, at level 73, against a level cap of 50. In practice
-- the mechanic does not exist in play at all.
--
-- This adds six low tiers on the same 8/18/28/38/48/58 cadence as the reptile / sloth / moonfire
-- lines, using the NATIVE SPA 289 rather than the Lua event_spell_fade approach that 43017 Languid
-- Healing uses -- the engine already implements exactly this, so no script is needed.
--
--   lvl  id     name                     heal   mana   delay
--    8   43324  Promised Salve            110     55   3 ticks (18s)
--   18   43325  Promised Poultice         200    100   3 ticks
--   28   43326  Promised Balm             300    150   3 ticks
--   38   43327  Promised Suture           420    210   3 ticks
--   48   43328  Promised Convalescence    560    280   3 ticks
--   58   43329  Promised Regrowth         900    450   3 ticks
--   (73  9755   Promised Renewal         8500    500   3 ticks  <- stock, unchanged)
--
-- Heals run ~1.5x the stock DIRECT heal of the same level (Healing 95 @10, Greater Healing 140 @20,
-- Superior Healing 200 @30), which is the premium for waiting out the delay.
--
-- STRUCTURE, cloned from 9755/9758 -- three effects, and all three matter:
--   slot 1  SPA 289 CastOnFadeEffect, base = the bloom spell id
--   slot 2  SPA 44  base = the heal amount. This is the STACKING MARKER, not a real effect.
--   slot 3  SPA 148 StackingCommand_Block, base = 44. Together with slot 2 this is how Promised
--           heals refuse to stack: a weaker promise cannot overwrite a stronger one. Keep both,
--           and keep slot 2's base equal to the actual heal, or the tiers will stack on each other.
-- All six also share spellgroup 43324 with ascending rank, so a higher tier replaces a lower one.
--
--   blooms  43324-43329   offered by the picker (band 43300-43349)
--   heals   43368-43373   never offered (helper band 43350+)
--
--   mysql -h127.0.0.1 -upeq -ppeqpass peq < .devcontainer/custom/sql/aotv4_promised_line.sql
--
-- ⚠️ spells_new IS in shared memory (section 10): stop world + zones, run ./shared_memory, restart.
-- Then: perl .devcontainer/custom/spells/gen_stock_pool.pl && perl aotv4_client_install/gen_choice_xml.pl
-- =============================================================================================

DELETE FROM spells_new WHERE id BETWEEN 43324 AND 43329 OR id BETWEEN 43368 AND 43373;
DELETE FROM db_str    WHERE (id BETWEEN 43324 AND 43329 OR id BETWEEN 43368 AND 43373) AND type = 6;

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;
CREATE TEMPORARY TABLE aotv4_tmpl LIKE spells_new;

-- ---------------------------------------------------------------- the blooms (the actual heal)
-- Cloned from 9758: targettype 6 (self, i.e. whoever held the buff), SPA 0 positive = instant heal.
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 9758;
UPDATE aotv4_tmpl SET id=43368, name='Promised Salve Bloom',          descnum=43368, spellgroup=43368, effect_base_value1=110;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 9758;
UPDATE aotv4_tmpl SET id=43369, name='Promised Poultice Bloom',       descnum=43369, spellgroup=43369, effect_base_value1=200;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 9758;
UPDATE aotv4_tmpl SET id=43370, name='Promised Balm Bloom',           descnum=43370, spellgroup=43370, effect_base_value1=300;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 9758;
UPDATE aotv4_tmpl SET id=43371, name='Promised Suture Bloom',         descnum=43371, spellgroup=43371, effect_base_value1=420;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 9758;
UPDATE aotv4_tmpl SET id=43372, name='Promised Convalescence Bloom',  descnum=43372, spellgroup=43372, effect_base_value1=560;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 9758;
UPDATE aotv4_tmpl SET id=43373, name='Promised Regrowth Bloom',       descnum=43373, spellgroup=43373, effect_base_value1=900;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

-- ---------------------------------------------------------------- the promises (what is scribed)
-- Cloned from 9755. slot1 base = bloom id, slot2 base = heal amount (stacking marker), slot3 = the
-- block that reads slot 2. rank ascends so a higher tier replaces a lower one within spellgroup.
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 9755;
UPDATE aotv4_tmpl SET id=43324, name='Promised Salve',         descnum=43324, mana=55,  spellgroup=43324, rank=1,
  effect_base_value1=43368, effect_base_value2=110,
  classes1=8,  classes2=8,  classes3=8,  classes4=8,  classes5=8,  classes6=8,  classes7=8,  classes8=8,
  classes9=8,  classes10=8, classes11=8, classes12=8, classes13=8, classes14=8, classes15=8, classes16=8;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 9755;
UPDATE aotv4_tmpl SET id=43325, name='Promised Poultice',      descnum=43325, mana=100, spellgroup=43324, rank=2,
  effect_base_value1=43369, effect_base_value2=200,
  classes1=18, classes2=18, classes3=18, classes4=18, classes5=18, classes6=18, classes7=18, classes8=18,
  classes9=18, classes10=18,classes11=18,classes12=18,classes13=18,classes14=18,classes15=18,classes16=18;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 9755;
UPDATE aotv4_tmpl SET id=43326, name='Promised Balm',          descnum=43326, mana=150, spellgroup=43324, rank=3,
  effect_base_value1=43370, effect_base_value2=300,
  classes1=28, classes2=28, classes3=28, classes4=28, classes5=28, classes6=28, classes7=28, classes8=28,
  classes9=28, classes10=28,classes11=28,classes12=28,classes13=28,classes14=28,classes15=28,classes16=28;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 9755;
UPDATE aotv4_tmpl SET id=43327, name='Promised Suture',        descnum=43327, mana=210, spellgroup=43324, rank=4,
  effect_base_value1=43371, effect_base_value2=420,
  classes1=38, classes2=38, classes3=38, classes4=38, classes5=38, classes6=38, classes7=38, classes8=38,
  classes9=38, classes10=38,classes11=38,classes12=38,classes13=38,classes14=38,classes15=38,classes16=38;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 9755;
UPDATE aotv4_tmpl SET id=43328, name='Promised Convalescence', descnum=43328, mana=280, spellgroup=43324, rank=5,
  effect_base_value1=43372, effect_base_value2=560,
  classes1=48, classes2=48, classes3=48, classes4=48, classes5=48, classes6=48, classes7=48, classes8=48,
  classes9=48, classes10=48,classes11=48,classes12=48,classes13=48,classes14=48,classes15=48,classes16=48;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 9755;
UPDATE aotv4_tmpl SET id=43329, name='Promised Regrowth',      descnum=43329, mana=450, spellgroup=43324, rank=6,
  effect_base_value1=43373, effect_base_value2=900,
  classes1=58, classes2=58, classes3=58, classes4=58, classes5=58, classes6=58, classes7=58, classes8=58,
  classes9=58, classes10=58,classes11=58,classes12=58,classes13=58,classes14=58,classes15=58,classes16=58;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;

-- ---------------------------------------------------------------- descriptions
INSERT INTO db_str (id, type, value) VALUES
 (43324, 6, 'Promises aid to your target. After 18 seconds the promise blooms, healing 110 points.'),
 (43325, 6, 'Promises aid to your target. After 18 seconds the promise blooms, healing 200 points.'),
 (43326, 6, 'Promises aid to your target. After 18 seconds the promise blooms, healing 300 points.'),
 (43327, 6, 'Promises aid to your target. After 18 seconds the promise blooms, healing 420 points.'),
 (43328, 6, 'Promises aid to your target. After 18 seconds the promise blooms, healing 560 points.'),
 (43329, 6, 'Promises aid to your target. After 18 seconds the promise blooms, healing 900 points.'),
 (43368, 6, 'The promise blooms.'),
 (43369, 6, 'The promise blooms.'),
 (43370, 6, 'The promise blooms.'),
 (43371, 6, 'The promise blooms.'),
 (43372, 6, 'The promise blooms.'),
 (43373, 6, 'The promise blooms.');
