-- aotv4_moonfire_line.sql -- the Moonfire line: a LIFETAP weighted 25 pct damage / 75 pct heal.
-- =============================================================================================
-- Redesign of the one-off 43107 Moonfire into a six-tier line on the same 8/18/28/38/48/58 cadence
-- as aotv4_reptile_line.sql and aotv4_sloth_line.sql.
--
-- These are REAL TAPS: targettype 13 (ST_Tap), which is exactly what IsLifetapSpell keys on
-- (common/spdat.cpp:108). One cast, healing is intrinsic -- no trigger spell, no second spell to
-- cast by choice. Being a genuine tap also matters beyond the heal: IsLifetapSpell excludes them
-- from IsDamageSpell / IsAnyDamageSpell / IsDamageOverTimeSpell, drives the "beams a smile at"
-- emote, and changes how bots treat them.
--
--   lvl  id     name        damage   heal   mana    (necro lifetap damage at that level)
--    8   43312  Moonspark       25     75     40      ~53   Lifespike 7 @3 -> Lifedraw 90 @12
--   18   43313  Moonflare       45    135     75      ~97   Lifedraw 90 @12 -> Siphon Life 100 @20
--   28   43314  Moonblaze       80    240    130     ~165   Spirit Tap 150 @26
--   38   43315  Moonstorm      100    300    165     ~195   Drain Spirit 200 @39
--   48   43316  Moonfury       125    375    205      250   Drain Soul 250 @48
--   58   43317  Moonwrath      300    900    480     ~640   Deflux 535 @54 -> Touch of Night 708 @59
--
-- Damage is roughly HALF the native lifetap for the level; healing is 3x the damage. A lifetap
-- returns exactly what it deals (heal = damage, a 50/50 split of its output); this returns three
-- times, so the same power budget lands 25 percent on damage and 75 percent on healing.
--
-- ⚠️⚠️ THE 3x SPLIT IS NOT EXPRESSIBLE IN THE DB. EQEmu hardcodes the tap ratio -- Mob::Damage does
--   int64 healed = damage;                                   (zone/attack.cpp:4287)
-- and spells_new has no ratio column. So the engine pays the first 1x of healing itself and
-- quests/global/spells/433xx.lua adds the remaining 2x through lua_modules/aotv4_moonfire.lua.
-- CHANGE A DAMAGE VALUE HERE AND YOU MUST CHANGE THE BONUS IN THAT TIER'S SCRIPT (it is 2x damage).
--
-- ⚠️ NAMING: no tier is called "Moonfire". Stock spell 2877 Moonfire is a Druid 60 nuke already IN
-- the reward pool, so a custom spell of that name would be a real duplicate in the picker.
--
--   taps  43312-43317   offered by the level-up reward picker (band 43300-43349)
--
-- 43107 / 43159 are left in place as the clone templates -- unoffered, scribed by nobody -- which
-- is what keeps this script re-runnable.
--
--   mysql -h127.0.0.1 -upeq -ppeqpass peq < .devcontainer/custom/sql/aotv4_moonfire_line.sql
--
-- ⚠️ spells_new IS in shared memory (section 10): stop world + zones, run ./shared_memory, restart.
-- Then: perl .devcontainer/custom/spells/gen_stock_pool.pl && perl aotv4_client_install/gen_choice_xml.pl
-- =============================================================================================

-- Idempotent. 43318-43323 were the separate castable heals and 43362-43367 the trigger spells,
-- both from earlier passes at this line; drop them, nothing points at them any more.
DELETE FROM spells_new WHERE id BETWEEN 43312 AND 43323 OR id BETWEEN 43362 AND 43367;
DELETE FROM db_str    WHERE (id BETWEEN 43312 AND 43323 OR id BETWEEN 43362 AND 43367) AND type = 6;

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;
CREATE TEMPORARY TABLE aotv4_tmpl LIKE spells_new;

-- ---------------------------------------------------------------- the taps
-- Cloned from 43107, with slot 2 (its old SPA 374 trigger) CLEARED to 254 so nothing auto-fires,
-- and targettype switched to 13 so the engine treats it as a lifetap.
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43107;
UPDATE aotv4_tmpl SET id=43312, name='Moonspark', descnum=43312, mana=40,  effect_base_value1=-25,
  targettype=13, effectid2=254, effect_base_value2=0, effect_limit_value2=0, recast_time=1500,
  classes1=8,  classes2=8,  classes3=8,  classes4=8,  classes5=8,  classes6=8,  classes7=8,  classes8=8,
  classes9=8,  classes10=8, classes11=8, classes12=8, classes13=8, classes14=8, classes15=8, classes16=8;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43107;
UPDATE aotv4_tmpl SET id=43313, name='Moonflare', descnum=43313, mana=75,  effect_base_value1=-45,
  targettype=13, effectid2=254, effect_base_value2=0, effect_limit_value2=0, recast_time=1500,
  classes1=18, classes2=18, classes3=18, classes4=18, classes5=18, classes6=18, classes7=18, classes8=18,
  classes9=18, classes10=18,classes11=18,classes12=18,classes13=18,classes14=18,classes15=18,classes16=18;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43107;
UPDATE aotv4_tmpl SET id=43314, name='Moonblaze', descnum=43314, mana=130, effect_base_value1=-80,
  targettype=13, effectid2=254, effect_base_value2=0, effect_limit_value2=0, recast_time=1500,
  classes1=28, classes2=28, classes3=28, classes4=28, classes5=28, classes6=28, classes7=28, classes8=28,
  classes9=28, classes10=28,classes11=28,classes12=28,classes13=28,classes14=28,classes15=28,classes16=28;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43107;
UPDATE aotv4_tmpl SET id=43315, name='Moonstorm', descnum=43315, mana=165, effect_base_value1=-100,
  targettype=13, effectid2=254, effect_base_value2=0, effect_limit_value2=0, recast_time=1500,
  classes1=38, classes2=38, classes3=38, classes4=38, classes5=38, classes6=38, classes7=38, classes8=38,
  classes9=38, classes10=38,classes11=38,classes12=38,classes13=38,classes14=38,classes15=38,classes16=38;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43107;
UPDATE aotv4_tmpl SET id=43316, name='Moonfury',  descnum=43316, mana=205, effect_base_value1=-125,
  targettype=13, effectid2=254, effect_base_value2=0, effect_limit_value2=0, recast_time=1500,
  classes1=48, classes2=48, classes3=48, classes4=48, classes5=48, classes6=48, classes7=48, classes8=48,
  classes9=48, classes10=48,classes11=48,classes12=48,classes13=48,classes14=48,classes15=48,classes16=48;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43107;
UPDATE aotv4_tmpl SET id=43317, name='Moonwrath', descnum=43317, mana=480, effect_base_value1=-300,
  targettype=13, effectid2=254, effect_base_value2=0, effect_limit_value2=0, recast_time=1500,
  classes1=58, classes2=58, classes3=58, classes4=58, classes5=58, classes6=58, classes7=58, classes8=58,
  classes9=58, classes10=58,classes11=58,classes12=58,classes13=58,classes14=58,classes15=58,classes16=58;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;

-- ---------------------------------------------------------------- descriptions
-- type 6 = spell description, keyed by descnum (= spell id). No literal percent sign:
-- Client::Message is printf-style and eats it as a format token.
INSERT INTO db_str (id, type, value) VALUES
 (43312, 6, 'Draws the life from your target for 25 damage, knitting 75 points of your own wounds closed.'),
 (43313, 6, 'Draws the life from your target for 45 damage, knitting 135 points of your own wounds closed.'),
 (43314, 6, 'Draws the life from your target for 80 damage, knitting 240 points of your own wounds closed.'),
 (43315, 6, 'Draws the life from your target for 100 damage, knitting 300 points of your own wounds closed.'),
 (43316, 6, 'Draws the life from your target for 125 damage, knitting 375 points of your own wounds closed.'),
 (43317, 6, 'Draws the life from your target for 300 damage, knitting 900 points of your own wounds closed.');
