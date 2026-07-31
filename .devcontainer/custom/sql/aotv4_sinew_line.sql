-- aotv4_sinew_line.sql -- the Sinew line: a DAMAGE tap weighted 75 pct damage / 25 pct ENDURANCE.
-- =============================================================================================
-- The structural mirror of aotv4_moonfire_line.sql. Moonfire splits its output 25 damage / 75
-- HEALTH; this splits it 75 damage / 25 ENDURANCE, so a tier that deals 120 hands the caster 40
-- endurance back. Same six-tier 8/18/28/38/48/58 cadence as the reptile, sloth and moonfire lines.
--
-- WHY THIS LINE EXISTS: section 22 made every damaging combat special cost endurance in proportion
-- to the damage it dealt (AoT:SpecialEndurancePct, default 50). That gave endurance a real sink for
-- the first time; this is the tap that refills it. The two are meant to be read together -- a tier
-- returning 175 endurance buys roughly 350 damage worth of specials at the default rate.
--
--   lvl  id     name        damage   end    mana   dpm    (avg native single-target nuke at level)
--    8   43318  Sinewbite       27     9      24   1.13     ~30 dmg  / 22 mana
--   18   43319  Sinewdraw       90    30      70   1.29     ~73-130  / 45-83 mana
--   28   43320  Sinewdrain     120    40     105   1.14    ~100-160  / 124 mana
--   38   43321  Sinewrend      225    75     165   1.36    ~224-263  / 133-178 mana
--   48   43322  Sinewfeast     360   120     235   1.53       ~413   / 162 mana
--   58   43323  Sinewreave     525   175     330   1.59       ~586   / 281 mana
--
-- Damage sits a little UNDER the average native single-target nuke for the level (those averages
-- were read out of this database, not from memory) because the cast also returns a resource.
-- Every damage value is divisible by 3 so the endurance return is a clean integer.
--
-- ⚠️⚠️ THESE ARE NOT LIFETAPS AND MUST NOT BE "FIXED" INTO ONE. targettype stays 5 (single target).
-- Setting it to 13 (ST_Tap) is what IsLifetapSpell keys on (common/spdat.cpp:108) and it makes
-- Mob::Damage heal the caster 1x the damage in HEALTH -- `int64 healed = damage;`
-- (zone/attack.cpp:4287). That is free health nobody asked for, handed out on top of the
-- endurance. Moonfire wants that engine-paid 1x and is a real tap; this line does not and is not.
--
-- ⚠️⚠️ THE ENDURANCE RETURN IS NOT EXPRESSIBLE IN THE DB AT ALL. There is no endurance-tap SPA.
-- SPA 189 (SE_CurrentEndurance) applies to the SPELL'S TARGET, and the target of a nuke is the
-- thing you are hitting -- it would hand the monster the endurance. A recourse spell could aim it
-- at the caster but only for a FLAT amount unrelated to the tier. So the rows are plain nukes and
-- quests/global/spells/433xx.lua pays the whole return through lua_modules/aotv4_sinewtap.lua.
-- CHANGE A DAMAGE VALUE HERE AND YOU MUST CHANGE THAT TIER'S SCRIPT (its return is damage / 3).
--
-- ⚠️ Cloned from stock 466 Lightning Shock via a temp table so all ~236 columns stay byte-identical
-- to stock -- never hand-list the columns. It is a clean single-effect ST nuke (skill 24 Evocation,
-- resisttype 1 magic, effectid1 0, effectid2 254). Only new_icon is overridden, to 47: that is the
-- icon 44 native lifetaps use, so the line reads as draining rather than as lightning.
--
-- ⚠️ NAMING: "Sinew" appears in no other spell in this database (checked). Deliberately NOT
-- "Marrow" -- stock 8608 Marrow Drain and 8803 Marrow Bend would sit next to it in the picker.
--
--   taps  43318-43323   offered by the level-up reward picker (band 43300-43349)
--
--   mysql -h127.0.0.1 -upeq -ppeqpass peq < .devcontainer/custom/sql/aotv4_sinew_line.sql
--
-- ⚠️ spells_new IS in shared memory (section 10): stop world + zones, run ./shared_memory, restart.
-- Then: perl .devcontainer/custom/spells/gen_stock_pool.pl && perl aotv4_client_install/gen_choice_xml.pl
-- =============================================================================================

-- Idempotent. Scoped to exactly this line's ids -- see the note in aotv4_moonfire_line.sql, whose
-- DELETE used to run to 43323 and would have taken this whole line with it.
DELETE FROM spells_new WHERE id BETWEEN 43318 AND 43323;
DELETE FROM db_str    WHERE  id BETWEEN 43318 AND 43323 AND type = 6;

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;
CREATE TEMPORARY TABLE aotv4_tmpl LIKE spells_new;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 466;
UPDATE aotv4_tmpl SET id=43318, name='Sinewbite',  descnum=43318, mana=24,  effect_base_value1=-27,
  new_icon=47, targettype=5, recast_time=1500,
  classes1=8,  classes2=8,  classes3=8,  classes4=8,  classes5=8,  classes6=8,  classes7=8,  classes8=8,
  classes9=8,  classes10=8, classes11=8, classes12=8, classes13=8, classes14=8, classes15=8, classes16=8;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 466;
UPDATE aotv4_tmpl SET id=43319, name='Sinewdraw',  descnum=43319, mana=70,  effect_base_value1=-90,
  new_icon=47, targettype=5, recast_time=1500,
  classes1=18, classes2=18, classes3=18, classes4=18, classes5=18, classes6=18, classes7=18, classes8=18,
  classes9=18, classes10=18,classes11=18,classes12=18,classes13=18,classes14=18,classes15=18,classes16=18;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 466;
UPDATE aotv4_tmpl SET id=43320, name='Sinewdrain', descnum=43320, mana=105, effect_base_value1=-120,
  new_icon=47, targettype=5, recast_time=1500,
  classes1=28, classes2=28, classes3=28, classes4=28, classes5=28, classes6=28, classes7=28, classes8=28,
  classes9=28, classes10=28,classes11=28,classes12=28,classes13=28,classes14=28,classes15=28,classes16=28;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 466;
UPDATE aotv4_tmpl SET id=43321, name='Sinewrend',  descnum=43321, mana=165, effect_base_value1=-225,
  new_icon=47, targettype=5, recast_time=1500,
  classes1=38, classes2=38, classes3=38, classes4=38, classes5=38, classes6=38, classes7=38, classes8=38,
  classes9=38, classes10=38,classes11=38,classes12=38,classes13=38,classes14=38,classes15=38,classes16=38;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 466;
UPDATE aotv4_tmpl SET id=43322, name='Sinewfeast', descnum=43322, mana=235, effect_base_value1=-360,
  new_icon=47, targettype=5, recast_time=1500,
  classes1=48, classes2=48, classes3=48, classes4=48, classes5=48, classes6=48, classes7=48, classes8=48,
  classes9=48, classes10=48,classes11=48,classes12=48,classes13=48,classes14=48,classes15=48,classes16=48;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 466;
UPDATE aotv4_tmpl SET id=43323, name='Sinewreave', descnum=43323, mana=330, effect_base_value1=-525,
  new_icon=47, targettype=5, recast_time=1500,
  classes1=58, classes2=58, classes3=58, classes4=58, classes5=58, classes6=58, classes7=58, classes8=58,
  classes9=58, classes10=58,classes11=58,classes12=58,classes13=58,classes14=58,classes15=58,classes16=58;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;

-- ---------------------------------------------------------------- static damage
-- ⚠️⚠️ THE CLONE INHERITS THE TEMPLATE'S DAMAGE FORMULA, AND OVERRIDING effect_base_value1 IS NOT
-- ENOUGH. 466 Lightning Shock ships formula1 = 5 and max1 = 570. Formula values 1-99 mean
-- "base + caster_level * formula" (zone/spell_effects.cpp, the default branch of the formula
-- switch: `if (formula < 100) result = ubase + (caster_level * formula)`), so Sinewbite was dealing
-- 27 + 8*5 = 67 at level 8 instead of 27, and would have reached 27 + 250 = 277 by level 50.
--
-- That does not just make the numbers wrong, it breaks the DESIGN: the endurance return is a FLAT
-- amount paid by Lua, so a damage value that climbs with level while the return stays put destroys
-- the 75/25 split the whole line exists to express.
--
-- ⚠️ Every tier is meant to be a hand-tuned constant, so force formula 100 ("= base", the static
-- case) and max 0 (no cap; 570 would also have clipped the top tier). This is what the sibling
-- Moonfire line already had -- it cloned a custom that was static, which is why it never showed
-- this and why the audit of 43300-43347 found the Sinew line to be the only offender.
-- ⚠️ Check formula1/max1 on ANY future clone of a stock spell before trusting its damage.
UPDATE spells_new SET formula1 = 100, max1 = 0 WHERE id BETWEEN 43318 AND 43323;

-- ---------------------------------------------------------------- cast messages
-- ⚠️ The clone inherits 466 Lightning Shock's flavour text, which is "You feel your skin ignite."
-- -- a FIRE message on a spell that drains sinew. These two columns are rendered by the CLIENT out
-- of its own spells_us.txt, so fixing them here only takes effect once that file is re-exported
-- (./export_client_files) and reinstalled in the EQ root; the shared-memory copy is not what the
-- player reads. ⚠️ cast_on_other is appended to the target's name, so it must start mid-sentence.
UPDATE spells_new SET
  cast_on_you   = 'You feel your sinew tear away.',
  cast_on_other = '''s sinew tears away.'
WHERE id BETWEEN 43318 AND 43323;

-- ---------------------------------------------------------------- descriptions
-- type 6 = spell description, keyed by descnum (= spell id). No literal percent sign:
-- Client::Message is printf-style and eats it as a format token.
INSERT INTO db_str (id, type, value) VALUES
 (43318, 6, 'Tears the sinew from your target for 27 damage, drawing 9 endurance into your own limbs.'),
 (43319, 6, 'Tears the sinew from your target for 90 damage, drawing 30 endurance into your own limbs.'),
 (43320, 6, 'Tears the sinew from your target for 120 damage, drawing 40 endurance into your own limbs.'),
 (43321, 6, 'Tears the sinew from your target for 225 damage, drawing 75 endurance into your own limbs.'),
 (43322, 6, 'Tears the sinew from your target for 360 damage, drawing 120 endurance into your own limbs.'),
 (43323, 6, 'Tears the sinew from your target for 525 damage, drawing 175 endurance into your own limbs.');
