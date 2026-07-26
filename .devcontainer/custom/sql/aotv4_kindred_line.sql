-- aotv4_kindred_line.sql -- a melee proc buff that heals YOUR GROUP on a swing.
-- =============================================================================================
-- Self buff. While it holds, your melee swings have a chance to proc a small heal on your whole
-- group. Offensive proc (on your swing), not a defensive one (on being hit) -- so it is the mirror
-- of the sloth line rather than a copy of it.
--
--   lvl  id     name               group heal   mana   duration
--    8   43330  Kindred Spark            12       30    10 min
--   18   43331  Kindred Ember            25       60    10 min
--   28   43332  Kindred Flame            45      100    10 min
--   38   43333  Kindred Blaze            70      145    10 min
--   48   43334  Kindred Radiance        100      195    10 min
--   58   43335  Kindred Beacon          150      260    10 min
--
-- MODELLED ON NATIVE CONTENT: 1976 Blessing of the Blackstar, the proc on 31348 Blackstar, Mace of
-- Night -- targettype 41, goodEffect 1, SPA 0 base +100. 39661 Soothing Wave (Sword of the Sanative,
-- +550) is the same shape. So group-heal-on-proc is proven live behaviour, not new ground.
-- (Hopebringer is NOT a usable guide: its proc 3606 "Light of Marr" is a stub in this DB --
--  targettype 5, goodEffect 0, SPA 0 base -1, i.e. one point of DAMAGE. Its recourse 4119 is empty.)
--
-- The engine routes it like this:
-- Mob::ExecWeaponProc (zone/mob.cpp:5358) branches on whether the PROC spell is beneficial:
--     if (IsBeneficialSpell(spell_id) && ...) SpellFinished(spell_id, this, ...);   // the SWINGER
--     else                                    SpellFinished(spell_id, on,   ...);   // the VICTIM
-- so a beneficial proc is cast with the proc's owner as the target, and SpellFinished then applies
-- normal target-type resolution -- including `case ST_Group` (zone/spells.cpp:2162). Give the proc
-- spell targettype 41 and it lands on the swinger's group.
--
-- Two fields are therefore LOad-BEARING on the proc spell and must not be "tidied":
--   goodEffect = 1   picks the beneficial branch above. Set it to 0 and the heal lands on the mob
--                    you just hit instead of your group.
--   targettype = 41  ST_Group. Without it the heal only touches the swinger.
-- The proc spells mirror 1976 exactly: SPA 0 with a positive base (not SPA 79), range 100,
-- resist none, no duration, spell_category -99.
--
-- Heals are deliberately SMALL. A proc on swing fires far more often than a cast, it multiplies
-- across up to six people, and it scales with attack rate rather than with mana -- so this is
-- balanced as steady chip healing, not as a heal button.
--
-- Cloned from 1463 Call of Fire (the classic SPA 85 proc buff) and its strike 3131.
--
--   buffs  43330-43335   offered by the picker (band 43300-43349)
--   procs  43374-43379   never offered (helper band 43350+)
--
--   mysql -h127.0.0.1 -upeq -ppeqpass peq < .devcontainer/custom/sql/aotv4_kindred_line.sql
--
-- ⚠️ spells_new IS in shared memory (section 10): stop world + zones, run ./shared_memory, restart.
-- Then: perl .devcontainer/custom/spells/gen_stock_pool.pl && perl aotv4_client_install/gen_choice_xml.pl
-- =============================================================================================

DELETE FROM spells_new WHERE id BETWEEN 43330 AND 43335 OR id BETWEEN 43374 AND 43379;
DELETE FROM db_str    WHERE (id BETWEEN 43330 AND 43335 OR id BETWEEN 43374 AND 43379) AND type = 6;

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;
CREATE TEMPORARY TABLE aotv4_tmpl LIKE spells_new;

-- ---------------------------------------------------------------- the procs (the group heal)
-- Cloned from 3131 Call of Fire Strike, then reshaped to match 1976 Blessing of the Blackstar:
-- SPA 0 positive heal, goodEffect 1, targettype 41. See the header for why those decide who heals.
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 3131;
UPDATE aotv4_tmpl SET id=43374, name='Kindred Spark Effect',    descnum=43374, spellgroup=43374,
  effectid1=0, effect_base_value1=12, goodEffect=1, targettype=41, resisttype=0, `range`=100, new_icon=99, spell_category=-99;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 3131;
UPDATE aotv4_tmpl SET id=43375, name='Kindred Ember Effect',    descnum=43375, spellgroup=43375,
  effectid1=0, effect_base_value1=25, goodEffect=1, targettype=41, resisttype=0, `range`=100, new_icon=99, spell_category=-99;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 3131;
UPDATE aotv4_tmpl SET id=43376, name='Kindred Flame Effect',    descnum=43376, spellgroup=43376,
  effectid1=0, effect_base_value1=45, goodEffect=1, targettype=41, resisttype=0, `range`=100, new_icon=99, spell_category=-99;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 3131;
UPDATE aotv4_tmpl SET id=43377, name='Kindred Blaze Effect',    descnum=43377, spellgroup=43377,
  effectid1=0, effect_base_value1=70, goodEffect=1, targettype=41, resisttype=0, `range`=100, new_icon=99, spell_category=-99;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 3131;
UPDATE aotv4_tmpl SET id=43378, name='Kindred Radiance Effect', descnum=43378, spellgroup=43378,
  effectid1=0, effect_base_value1=100, goodEffect=1, targettype=41, resisttype=0, `range`=100, new_icon=99, spell_category=-99;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 3131;
UPDATE aotv4_tmpl SET id=43379, name='Kindred Beacon Effect',   descnum=43379, spellgroup=43379,
  effectid1=0, effect_base_value1=150, goodEffect=1, targettype=41, resisttype=0, `range`=100, new_icon=99, spell_category=-99;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

-- ---------------------------------------------------------------- the buffs (what is scribed)
-- Cloned from 1463 Call of Fire: SPA 85 WeaponProc, base = the proc spell id, self buff, 10 min.
-- All six share spellgroup 43330 with ascending rank so a higher tier replaces a lower one.
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 1463;
UPDATE aotv4_tmpl SET id=43330, name='Kindred Spark',    descnum=43330, mana=30,  spellgroup=43330, rank=1,
  effect_base_value1=43374,
  classes1=8,  classes2=8,  classes3=8,  classes4=8,  classes5=8,  classes6=8,  classes7=8,  classes8=8,
  classes9=8,  classes10=8, classes11=8, classes12=8, classes13=8, classes14=8, classes15=8, classes16=8;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 1463;
UPDATE aotv4_tmpl SET id=43331, name='Kindred Ember',    descnum=43331, mana=60,  spellgroup=43330, rank=2,
  effect_base_value1=43375,
  classes1=18, classes2=18, classes3=18, classes4=18, classes5=18, classes6=18, classes7=18, classes8=18,
  classes9=18, classes10=18,classes11=18,classes12=18,classes13=18,classes14=18,classes15=18,classes16=18;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 1463;
UPDATE aotv4_tmpl SET id=43332, name='Kindred Flame',    descnum=43332, mana=100, spellgroup=43330, rank=3,
  effect_base_value1=43376,
  classes1=28, classes2=28, classes3=28, classes4=28, classes5=28, classes6=28, classes7=28, classes8=28,
  classes9=28, classes10=28,classes11=28,classes12=28,classes13=28,classes14=28,classes15=28,classes16=28;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 1463;
UPDATE aotv4_tmpl SET id=43333, name='Kindred Blaze',    descnum=43333, mana=145, spellgroup=43330, rank=4,
  effect_base_value1=43377,
  classes1=38, classes2=38, classes3=38, classes4=38, classes5=38, classes6=38, classes7=38, classes8=38,
  classes9=38, classes10=38,classes11=38,classes12=38,classes13=38,classes14=38,classes15=38,classes16=38;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 1463;
UPDATE aotv4_tmpl SET id=43334, name='Kindred Radiance', descnum=43334, mana=195, spellgroup=43330, rank=5,
  effect_base_value1=43378,
  classes1=48, classes2=48, classes3=48, classes4=48, classes5=48, classes6=48, classes7=48, classes8=48,
  classes9=48, classes10=48,classes11=48,classes12=48,classes13=48,classes14=48,classes15=48,classes16=48;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 1463;
UPDATE aotv4_tmpl SET id=43335, name='Kindred Beacon',   descnum=43335, mana=260, spellgroup=43330, rank=6,
  effect_base_value1=43379,
  classes1=58, classes2=58, classes3=58, classes4=58, classes5=58, classes6=58, classes7=58, classes8=58,
  classes9=58, classes10=58,classes11=58,classes12=58,classes13=58,classes14=58,classes15=58,classes16=58;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;

-- ---------------------------------------------------------------- descriptions
INSERT INTO db_str (id, type, value) VALUES
 (43330, 6, 'Binds your fate to your group. Your melee attacks may heal every group member for 12 points.'),
 (43331, 6, 'Binds your fate to your group. Your melee attacks may heal every group member for 25 points.'),
 (43332, 6, 'Binds your fate to your group. Your melee attacks may heal every group member for 45 points.'),
 (43333, 6, 'Binds your fate to your group. Your melee attacks may heal every group member for 70 points.'),
 (43334, 6, 'Binds your fate to your group. Your melee attacks may heal every group member for 100 points.'),
 (43335, 6, 'Binds your fate to your group. Your melee attacks may heal every group member for 150 points.'),
 (43374, 6, 'Kindred light washes over your group.'),
 (43375, 6, 'Kindred light washes over your group.'),
 (43376, 6, 'Kindred light washes over your group.'),
 (43377, 6, 'Kindred light washes over your group.'),
 (43378, 6, 'Kindred light washes over your group.'),
 (43379, 6, 'Kindred light washes over your group.');
