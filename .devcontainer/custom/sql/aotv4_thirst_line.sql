-- aotv4_thirst_line.sql -- your melee hits heal you for a SET AMOUNT.
-- =============================================================================================
-- ⚠️ READ THIS FIRST -- two things about this line are not what they look like.
--
-- 1. IT IS A SELF BUFF, NOT A DEBUFF ON THE TARGET.
--    The ask was "a debuff that heals you when you hit it". That shape does not exist: the damage
--    shield family covers damage both ways (SPA 59 = buff on defender hurts attacker, SPA 121 =
--    debuff on attacker hurts attacker) but has NO healing counterpart in either direction, and
--    Mob::MeleeLifeTap (zone/mob.cpp:6852) reads the bonus off the ATTACKER and heals the ATTACKER.
--    The bonus has to sit on whoever is swinging, so the spell is cast on yourself. It therefore
--    covers every fight for its duration instead of being applied per target.
--
-- 2. THE HEAL IS PAID BY LUA, NOT BY THE SPELL ROW. These rows are INERT MARKER BUFFS -- every
--    effect slot is 254 and the only real fields are the duration and the icon. The native
--    mechanic (SPA 178 MeleeLifetap) is PERCENTAGE-ONLY by construction --
--        lifetap_amt = damage * (melee_lifetap_mod / 100.0f);       (zone/mob.cpp:6860)
--    -- and nothing else in the engine fires on every successful melee hit, so a flat per-hit
--    amount cannot be expressed in spells_new at all. lua_modules/aotv4_thirst.lua does the healing
--    off global_player event_damage_given. This is the same inert-marker pattern the existing
--    reaction abilities use (43022 Divine Aura, 43035 Blade Turn, 43056 Counterattack).
--    ⚠️ THE AMOUNTS LIVE IN THAT LUA TABLE. The numbers below are documentation; editing them here
--    changes nothing. Edit lua_modules/aotv4_thirst.lua and keep the two in step.
--
--   lvl  id     name                  heal per hit   mana   duration
--    8   43342  Faint Thirst                2         25    10 min
--   18   43343  Rising Thirst               4         50    10 min
--   28   43344  Keen Thirst                 6         85    10 min
--   38   43345  Ravening Thirst             8        130    10 min
--   48   43346  Savage Thirst              10        180    10 min
--   58   43347  Unquenchable Thirst        12        240    10 min
--
-- Flat healing does not scale with weapon or haste the way a percentage would, so the tiers climb
-- steeply to stay relevant. For contrast the only native user of SPA 178 is 4504 Leechcurse
-- Discipline (Shadowknight 60): 100 percent of damage, but for 4 ticks -- a burst cooldown, where
-- this is a small trickle sustained for ten minutes.
--
-- All six share spellgroup 43342 with ascending rank so a higher tier replaces a lower one.
--
--   thirsts  43342-43347   offered by the picker (band 43300-43349)
--
--   mysql -h127.0.0.1 -upeq -ppeqpass peq < .devcontainer/custom/sql/aotv4_thirst_line.sql
--
-- ⚠️ spells_new IS in shared memory (section 10): stop world + zones, run ./shared_memory, restart.
-- Zone restart also reloads the Lua module. Then regenerate the pool:
--   perl .devcontainer/custom/spells/gen_stock_pool.pl && perl aotv4_client_install/gen_choice_xml.pl
-- =============================================================================================

DELETE FROM spells_new WHERE id BETWEEN 43342 AND 43347;
DELETE FROM db_str    WHERE id BETWEEN 43342 AND 43347 AND type = 6;

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;
CREATE TEMPORARY TABLE aotv4_tmpl LIKE spells_new;

-- Cloned from 1463 Call of Fire -- a plain self buff with a 10 minute duration -- rather than from
-- Leechcurse, which is a 4-tick discipline and would drag its timer semantics along with it.
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 1463;
UPDATE aotv4_tmpl SET id=43342, name='Faint Thirst',        descnum=43342, mana=25,  numhits=50, numhitstype=5,  spellgroup=43342, rank=1,
  effectid1=254, effect_base_value1=0,  effect_limit_value1=0, max1=0, new_icon=161, spell_category=-99,
  classes1=8,  classes2=8,  classes3=8,  classes4=8,  classes5=8,  classes6=8,  classes7=8,  classes8=8,
  classes9=8,  classes10=8, classes11=8, classes12=8, classes13=8, classes14=8, classes15=8, classes16=8;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 1463;
UPDATE aotv4_tmpl SET id=43343, name='Rising Thirst',       descnum=43343, mana=50,  numhits=60, numhitstype=5,  spellgroup=43342, rank=2,
  effectid1=254, effect_base_value1=0,  effect_limit_value1=0, max1=0, new_icon=161, spell_category=-99,
  classes1=18, classes2=18, classes3=18, classes4=18, classes5=18, classes6=18, classes7=18, classes8=18,
  classes9=18, classes10=18,classes11=18,classes12=18,classes13=18,classes14=18,classes15=18,classes16=18;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 1463;
UPDATE aotv4_tmpl SET id=43344, name='Keen Thirst',         descnum=43344, mana=85,  numhits=70, numhitstype=5,  spellgroup=43342, rank=3,
  effectid1=254, effect_base_value1=0,  effect_limit_value1=0, max1=0, new_icon=161, spell_category=-99,
  classes1=28, classes2=28, classes3=28, classes4=28, classes5=28, classes6=28, classes7=28, classes8=28,
  classes9=28, classes10=28,classes11=28,classes12=28,classes13=28,classes14=28,classes15=28,classes16=28;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 1463;
UPDATE aotv4_tmpl SET id=43345, name='Ravening Thirst',     descnum=43345, mana=130, numhits=80, numhitstype=5, spellgroup=43342, rank=4,
  effectid1=254, effect_base_value1=0,  effect_limit_value1=0, max1=0, new_icon=161, spell_category=-99,
  classes1=38, classes2=38, classes3=38, classes4=38, classes5=38, classes6=38, classes7=38, classes8=38,
  classes9=38, classes10=38,classes11=38,classes12=38,classes13=38,classes14=38,classes15=38,classes16=38;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 1463;
UPDATE aotv4_tmpl SET id=43346, name='Savage Thirst',       descnum=43346, mana=180, numhits=90, numhitstype=5, spellgroup=43342, rank=5,
  effectid1=254, effect_base_value1=0,  effect_limit_value1=0, max1=0, new_icon=161, spell_category=-99,
  classes1=48, classes2=48, classes3=48, classes4=48, classes5=48, classes6=48, classes7=48, classes8=48,
  classes9=48, classes10=48,classes11=48,classes12=48,classes13=48,classes14=48,classes15=48,classes16=48;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 1463;
UPDATE aotv4_tmpl SET id=43347, name='Unquenchable Thirst', descnum=43347, mana=240, numhits=100, numhitstype=5, spellgroup=43342, rank=6,
  effectid1=254, effect_base_value1=0, effect_limit_value1=0, max1=0, new_icon=161, spell_category=-99,
  classes1=58, classes2=58, classes3=58, classes4=58, classes5=58, classes6=58, classes7=58, classes8=58,
  classes9=58, classes10=58,classes11=58,classes12=58,classes13=58,classes14=58,classes15=58,classes16=58;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;

-- ---------------------------------------------------------------- descriptions
INSERT INTO db_str (id, type, value) VALUES
 (43342, 6, 'Your melee attacks drink from the wounds they open, healing you for 2 hit points, and fades after 50 hits.'),
 (43343, 6, 'Your melee attacks drink from the wounds they open, healing you for 4 hit points, and fades after 60 hits.'),
 (43344, 6, 'Your melee attacks drink from the wounds they open, healing you for 6 hit points, and fades after 70 hits.'),
 (43345, 6, 'Your melee attacks drink from the wounds they open, healing you for 8 hit points, and fades after 80 hits.'),
 (43346, 6, 'Your melee attacks drink from the wounds they open, healing you for 10 hit points, and fades after 90 hits.'),
 (43347, 6, 'Your melee attacks drink from the wounds they open, healing you for 12 hit points, and fades after 100 hits.');
