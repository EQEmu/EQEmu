-- aotv4_reptile_line.sql -- low-level tiers for the "Skin of the Reptile" defensive-proc line.
-- =============================================================================================
-- Stock EQ only starts this line at level 68 (8008 Skin of the Reptile) and then climbs:
--   68 Skin -> 79 Scales -> 84 Carapace -> 89 Shell -> 94 Hide -> 99 Husk
-- Nothing fills levels 1-67, so this adds six lower rungs, one per decade:
--
--     8  Skin of the Newt          heal  20   mana  26
--    18  Skin of the Lizard        heal  45   mana  58
--    28  Skin of the Serpent       heal  85   mana 110
--    38  Skin of the Crocodile     heal 140   mana 182
--    48  Skin of the Basilisk      heal 215   mana 279
--    58  Skin of the Drake         heal 300   mana 390
--   (68  Skin of the Reptile       heal 393   mana 512   <- stock, unchanged)
--
-- Heal scales on a gentle curve up to the stock 393 at 68; mana holds the stock line's ratio of
-- ~0.77 healed per point of mana, so no tier is a better deal than the one above it.
--
-- HOW IT WORKS: the buff carries SPA 323 "add defensive proc" in slot 4, whose base is the id of a
-- TRIGGER spell; the trigger is a SPA 79 heal on self. Proc rate (effect_limit_value4 = 400) and
-- max (5) are kept from stock, so only the heal size differs between tiers.
--
--   buffs    43300-43305   offered by the level-up reward picker
--   triggers 43350-43355   never offered (gen_stock_pool.pl only pulls 43300-43349)
--
-- Rows are CLONED from 8008/8009 through a temporary table, so every column we do not explicitly
-- set is byte-identical to the stock spell. spells_new has ~236 columns; listing them by hand is
-- how subtle breakage gets in.
--
-- Classes: all 16 columns are set to the tier's level, matching the all-classes design (section 14)
-- and giving the reward picker its index (classes8 = offer level, section 5).
--
--   mysql -h127.0.0.1 -upeq -ppeqpass peq < .devcontainer/custom/sql/aotv4_reptile_line.sql
--
-- ⚠️ spells_new IS in shared memory (section 10): stop world + zones, run ./shared_memory, restart.
-- Then regenerate the pool so the picker can offer them:
--   perl .devcontainer/custom/spells/gen_stock_pool.pl
--   perl aotv4_client_install/gen_choice_xml.pl
-- =============================================================================================

-- Idempotent: re-running replaces the rows rather than erroring.
DELETE FROM spells_new WHERE id BETWEEN 43300 AND 43305 OR id BETWEEN 43350 AND 43355;
DELETE FROM db_str    WHERE (id BETWEEN 43300 AND 43305 OR id BETWEEN 43350 AND 43355) AND type = 6;

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;
CREATE TEMPORARY TABLE aotv4_tmpl LIKE spells_new;

-- ---------------------------------------------------------------- triggers (the heal on proc)
-- Cloned from 8009 "Skin of the Rep. Trigger": SPA 79 self-heal, duration 1, never offered.
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 8009;
UPDATE aotv4_tmpl SET id=43350, name='Skin of the Newt Trigger',      descnum=43350, effect_base_value1=20;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 8009;
UPDATE aotv4_tmpl SET id=43351, name='Skin of the Lizard Trigger',    descnum=43351, effect_base_value1=45;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 8009;
UPDATE aotv4_tmpl SET id=43352, name='Skin of the Serpent Trigger',   descnum=43352, effect_base_value1=85;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 8009;
UPDATE aotv4_tmpl SET id=43353, name='Skin of the Crocodile Trigger', descnum=43353, effect_base_value1=140;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 8009;
UPDATE aotv4_tmpl SET id=43354, name='Skin of the Basilisk Trigger',  descnum=43354, effect_base_value1=215;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 8009;
UPDATE aotv4_tmpl SET id=43355, name='Skin of the Drake Trigger',     descnum=43355, effect_base_value1=300;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

-- ---------------------------------------------------------------- buffs (what the player scribes)
-- Cloned from 8008 "Skin of the Reptile". effect_base_value4 is the SPA 323 trigger id.
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 8008;
UPDATE aotv4_tmpl SET id=43300, name='Skin of the Newt',      descnum=43300, mana=26,  effect_base_value4=43350,
  classes1=8,  classes2=8,  classes3=8,  classes4=8,  classes5=8,  classes6=8,  classes7=8,  classes8=8,
  classes9=8,  classes10=8, classes11=8, classes12=8, classes13=8, classes14=8, classes15=8, classes16=8;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 8008;
UPDATE aotv4_tmpl SET id=43301, name='Skin of the Lizard',    descnum=43301, mana=58,  effect_base_value4=43351,
  classes1=18, classes2=18, classes3=18, classes4=18, classes5=18, classes6=18, classes7=18, classes8=18,
  classes9=18, classes10=18,classes11=18,classes12=18,classes13=18,classes14=18,classes15=18,classes16=18;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 8008;
UPDATE aotv4_tmpl SET id=43302, name='Skin of the Serpent',   descnum=43302, mana=110, effect_base_value4=43352,
  classes1=28, classes2=28, classes3=28, classes4=28, classes5=28, classes6=28, classes7=28, classes8=28,
  classes9=28, classes10=28,classes11=28,classes12=28,classes13=28,classes14=28,classes15=28,classes16=28;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 8008;
UPDATE aotv4_tmpl SET id=43303, name='Skin of the Crocodile', descnum=43303, mana=182, effect_base_value4=43353,
  classes1=38, classes2=38, classes3=38, classes4=38, classes5=38, classes6=38, classes7=38, classes8=38,
  classes9=38, classes10=38,classes11=38,classes12=38,classes13=38,classes14=38,classes15=38,classes16=38;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 8008;
UPDATE aotv4_tmpl SET id=43304, name='Skin of the Basilisk',  descnum=43304, mana=279, effect_base_value4=43354,
  classes1=48, classes2=48, classes3=48, classes4=48, classes5=48, classes6=48, classes7=48, classes8=48,
  classes9=48, classes10=48,classes11=48,classes12=48,classes13=48,classes14=48,classes15=48,classes16=48;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 8008;
UPDATE aotv4_tmpl SET id=43305, name='Skin of the Drake',     descnum=43305, mana=390, effect_base_value4=43355,
  classes1=58, classes2=58, classes3=58, classes4=58, classes5=58, classes6=58, classes7=58, classes8=58,
  classes9=58, classes10=58,classes11=58,classes12=58,classes13=58,classes14=58,classes15=58,classes16=58;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;

-- ---------------------------------------------------------------- descriptions
-- type 6 = spell description, keyed by descnum (which we set = spell id, the custom-set convention).
-- No literal percent sign: Client::Message is printf-style and eats it as a format token.
INSERT INTO db_str (id, type, value) VALUES
 (43300, 6, 'Toughens your target''s skin. When the target is struck in combat, the ward may knit the wound, healing 20 hit points.'),
 (43301, 6, 'Toughens your target''s skin. When the target is struck in combat, the ward may knit the wound, healing 45 hit points.'),
 (43302, 6, 'Toughens your target''s skin. When the target is struck in combat, the ward may knit the wound, healing 85 hit points.'),
 (43303, 6, 'Toughens your target''s skin. When the target is struck in combat, the ward may knit the wound, healing 140 hit points.'),
 (43304, 6, 'Toughens your target''s skin. When the target is struck in combat, the ward may knit the wound, healing 215 hit points.'),
 (43305, 6, 'Toughens your target''s skin. When the target is struck in combat, the ward may knit the wound, healing 300 hit points.'),
 (43350, 6, 'Knits a wound closed.'),
 (43351, 6, 'Knits a wound closed.'),
 (43352, 6, 'Knits a wound closed.'),
 (43353, 6, 'Knits a wound closed.'),
 (43354, 6, 'Knits a wound closed.'),
 (43355, 6, 'Knits a wound closed.');
