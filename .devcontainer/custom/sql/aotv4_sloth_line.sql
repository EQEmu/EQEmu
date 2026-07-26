-- aotv4_sloth_line.sql -- low-level tiers for the Shaman "Lingering Sloth" retaliation-slow line.
-- =============================================================================================
-- Stock EQ has exactly ONE spell in this line, 8015 Lingering Sloth at Shaman 68, with nothing
-- below it and nothing above it. This adds six lower rungs, one per decade, matching what
-- aotv4_reptile_line.sql does for the Druid Skin-of-the-Reptile line.
--
--            level  name              slow      counters  mana
--              8    Faint Sloth        5 pct        2       25
--             18    Creeping Sloth     8 pct        4       57
--             28    Nagging Sloth     11 pct        6      107
--             38    Weighted Sloth    14 pct        9      178
--             48    Clinging Sloth    17 pct       11      273
--             58    Enduring Sloth    21 pct       13      381
--            (68    Lingering Sloth   25 pct       16      500   <- stock, unchanged)
--
-- HOW IT WORKS: the buff carries SPA 323 "add defensive proc" whose base is a TRIGGER spell id;
-- when the buffed target is struck, the trigger lands on the ATTACKER carrying SPA 11 (melee speed)
-- and SPA 35 (disease counters) for 3 ticks. So it is a retaliation slow, not a damage ward.
--
-- ⚠️ SPA 11 base is the attacker's resulting melee SPEED, not the slow amount: 75 means "attack at
-- 75 percent speed", i.e. a 25 percent slow. LOWER base = STRONGER. The tiers below therefore run
-- 95 down to 79, ending just above the stock 75.
--
-- ⚠️ The proc sits in effect slot **3** on this line (8015), whereas the reptile line puts it in
-- slot 4 (8008). Do not copy the slot number between them.
--
--   buffs    43306-43311   offered by the level-up reward picker (band 43300-43349)
--   triggers 43356-43361   never offered (helper band 43350+)
--
-- Rows are CLONED from 8015/8016 through a temporary table so every column we do not explicitly set
-- is byte-identical to stock. spells_new has ~236 columns; hand-listing them is how breakage gets in.
--
--   mysql -h127.0.0.1 -upeq -ppeqpass peq < .devcontainer/custom/sql/aotv4_sloth_line.sql
--
-- ⚠️ spells_new IS in shared memory (section 10): stop world + zones, run ./shared_memory, restart.
-- Then: perl .devcontainer/custom/spells/gen_stock_pool.pl && perl aotv4_client_install/gen_choice_xml.pl
-- =============================================================================================

-- Idempotent: re-running replaces the rows rather than erroring.
DELETE FROM spells_new WHERE id BETWEEN 43306 AND 43311 OR id BETWEEN 43356 AND 43361;
DELETE FROM db_str    WHERE (id BETWEEN 43306 AND 43311 OR id BETWEEN 43356 AND 43361) AND type = 6;

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;
CREATE TEMPORARY TABLE aotv4_tmpl LIKE spells_new;

-- ---------------------------------------------------------------- triggers (land on the attacker)
-- Cloned from 8016: slot2 = SPA 11 melee speed, slot3 = SPA 35 disease counters, 3 ticks.
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 8016;
UPDATE aotv4_tmpl SET id=43356, name='Faint Sloth Trigger',     descnum=43356, effect_base_value2=95, effect_base_value3=2;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 8016;
UPDATE aotv4_tmpl SET id=43357, name='Creeping Sloth Trigger',  descnum=43357, effect_base_value2=92, effect_base_value3=4;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 8016;
UPDATE aotv4_tmpl SET id=43358, name='Nagging Sloth Trigger',   descnum=43358, effect_base_value2=89, effect_base_value3=6;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 8016;
UPDATE aotv4_tmpl SET id=43359, name='Weighted Sloth Trigger',  descnum=43359, effect_base_value2=86, effect_base_value3=9;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 8016;
UPDATE aotv4_tmpl SET id=43360, name='Clinging Sloth Trigger',  descnum=43360, effect_base_value2=83, effect_base_value3=11;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 8016;
UPDATE aotv4_tmpl SET id=43361, name='Enduring Sloth Trigger',  descnum=43361, effect_base_value2=79, effect_base_value3=13;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

-- ---------------------------------------------------------------- buffs (what the player scribes)
-- Cloned from 8015. effect_base_value3 (slot THREE) is the SPA 323 trigger id.
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 8015;
UPDATE aotv4_tmpl SET id=43306, name='Faint Sloth',     descnum=43306, mana=25,  effect_base_value3=43356,
  classes1=8,  classes2=8,  classes3=8,  classes4=8,  classes5=8,  classes6=8,  classes7=8,  classes8=8,
  classes9=8,  classes10=8, classes11=8, classes12=8, classes13=8, classes14=8, classes15=8, classes16=8;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 8015;
UPDATE aotv4_tmpl SET id=43307, name='Creeping Sloth',  descnum=43307, mana=57,  effect_base_value3=43357,
  classes1=18, classes2=18, classes3=18, classes4=18, classes5=18, classes6=18, classes7=18, classes8=18,
  classes9=18, classes10=18,classes11=18,classes12=18,classes13=18,classes14=18,classes15=18,classes16=18;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 8015;
UPDATE aotv4_tmpl SET id=43308, name='Nagging Sloth',   descnum=43308, mana=107, effect_base_value3=43358,
  classes1=28, classes2=28, classes3=28, classes4=28, classes5=28, classes6=28, classes7=28, classes8=28,
  classes9=28, classes10=28,classes11=28,classes12=28,classes13=28,classes14=28,classes15=28,classes16=28;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 8015;
UPDATE aotv4_tmpl SET id=43309, name='Weighted Sloth',  descnum=43309, mana=178, effect_base_value3=43359,
  classes1=38, classes2=38, classes3=38, classes4=38, classes5=38, classes6=38, classes7=38, classes8=38,
  classes9=38, classes10=38,classes11=38,classes12=38,classes13=38,classes14=38,classes15=38,classes16=38;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 8015;
UPDATE aotv4_tmpl SET id=43310, name='Clinging Sloth',  descnum=43310, mana=273, effect_base_value3=43360,
  classes1=48, classes2=48, classes3=48, classes4=48, classes5=48, classes6=48, classes7=48, classes8=48,
  classes9=48, classes10=48,classes11=48,classes12=48,classes13=48,classes14=48,classes15=48,classes16=48;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 8015;
UPDATE aotv4_tmpl SET id=43311, name='Enduring Sloth',  descnum=43311, mana=381, effect_base_value3=43361,
  classes1=58, classes2=58, classes3=58, classes4=58, classes5=58, classes6=58, classes7=58, classes8=58,
  classes9=58, classes10=58,classes11=58,classes12=58,classes13=58,classes14=58,classes15=58,classes16=58;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;

-- ---------------------------------------------------------------- descriptions
-- type 6 = spell description, keyed by descnum (= spell id, the custom-set convention).
-- "percent" is spelled out: Client::Message is printf-style and eats a literal percent sign.
INSERT INTO db_str (id, type, value) VALUES
 (43306, 6, 'Wraps your target in a sluggish haze. Attackers who strike them are slowed by 5 percent for a short time.'),
 (43307, 6, 'Wraps your target in a sluggish haze. Attackers who strike them are slowed by 8 percent for a short time.'),
 (43308, 6, 'Wraps your target in a sluggish haze. Attackers who strike them are slowed by 11 percent for a short time.'),
 (43309, 6, 'Wraps your target in a sluggish haze. Attackers who strike them are slowed by 14 percent for a short time.'),
 (43310, 6, 'Wraps your target in a sluggish haze. Attackers who strike them are slowed by 17 percent for a short time.'),
 (43311, 6, 'Wraps your target in a sluggish haze. Attackers who strike them are slowed by 21 percent for a short time.'),
 (43356, 6, 'Your limbs grow heavy.'),
 (43357, 6, 'Your limbs grow heavy.'),
 (43358, 6, 'Your limbs grow heavy.'),
 (43359, 6, 'Your limbs grow heavy.'),
 (43360, 6, 'Your limbs grow heavy.'),
 (43361, 6, 'Your limbs grow heavy.');
