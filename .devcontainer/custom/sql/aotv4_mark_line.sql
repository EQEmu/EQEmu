-- aotv4_mark_line.sql -- low-level tiers for the "Mark of" reverse damage shield line.
-- =============================================================================================
-- A DEBUFF you cast on an ENEMY. While it holds, the marked creature wounds itself every time it
-- lands a melee hit. The native line is Cleric-only and starts at 54:
--
--   54  Mark of Retribution    -15      65  Mark of the Righteous  -34
--   69  Mark of the Blameless  -45      74  Mark of the Martyr     -54
--
-- Nothing fills levels 1-53, so this adds six lower rungs on the same 8/18/28/38/48/58 cadence as
-- the reptile / sloth / moonfire / promised / kindred lines.
--
--   lvl  id     name                  self-damage per hit   mana
--    8   43336  Mark of Reproof                3            15
--   18   43337  Mark of Rebuke                 5            30
--   28   43338  Mark of Reprisal               8            50
--   38   43339  Mark of Reckoning             11            75
--   48   43340  Mark of Requital              14           100
--   58   43341  Mark of Recompense            17           125
--   (54  2507   Mark of Retribution           15           115   <- stock, unchanged)
--
-- The level 58 tier lands just above the native level 54 one, which is the intent: four levels
-- higher for two more damage, and still far below Mark of the Righteous at 65.
--
-- ⚠️ IT IS A DEBUFF, NOT A BUFF -- goodEffect = 0, resisttype 1 (magic), cast on the target. The
-- "Mark of" naming reads like a blessing and every tier here is the opposite. Getting this backwards
-- would put the reverse shield on the PLAYER, who would then hurt themselves on every swing.
--
-- ⚠️ SPA 121 ReverseDS base is NEGATIVE, like a damage shield (section 14). Mob::DamageShield reads
-- it off the ATTACKER's own bonuses and damages the attacker (zone/attack.cpp:3476 and 3591):
--     rev_ds = attacker->spellbonuses.ReverseDamageShield;
--     if (rev_ds < 0) attacker->Damage(this, -rev_ds, ...);
-- A POSITIVE base does nothing at all -- the `rev_ds < 0` test simply never fires.
--
-- No helper spells: SPA 121 is self-contained, so unlike the other lines there are no trigger rows.
-- All six share spellgroup 43336 with ascending rank so a higher tier replaces a lower one.
--
--   marks  43336-43341   offered by the picker (band 43300-43349)
--
--   mysql -h127.0.0.1 -upeq -ppeqpass peq < .devcontainer/custom/sql/aotv4_mark_line.sql
--
-- ⚠️ spells_new IS in shared memory (section 10): stop world + zones, run ./shared_memory, restart.
-- Then: perl .devcontainer/custom/spells/gen_stock_pool.pl && perl aotv4_client_install/gen_choice_xml.pl
-- =============================================================================================

DELETE FROM spells_new WHERE id BETWEEN 43336 AND 43341;
DELETE FROM db_str    WHERE id BETWEEN 43336 AND 43341 AND type = 6;

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;
CREATE TEMPORARY TABLE aotv4_tmpl LIKE spells_new;

-- Cloned from 2507 Mark of Retribution: 150 ticks, single target, magic resist, detrimental.
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 2507;
UPDATE aotv4_tmpl SET id=43336, name='Mark of Reproof',    descnum=43336, mana=15,  spellgroup=43336, rank=1,
  effect_base_value1=-3,
  classes1=8,  classes2=8,  classes3=8,  classes4=8,  classes5=8,  classes6=8,  classes7=8,  classes8=8,
  classes9=8,  classes10=8, classes11=8, classes12=8, classes13=8, classes14=8, classes15=8, classes16=8;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 2507;
UPDATE aotv4_tmpl SET id=43337, name='Mark of Rebuke',     descnum=43337, mana=30,  spellgroup=43336, rank=2,
  effect_base_value1=-5,
  classes1=18, classes2=18, classes3=18, classes4=18, classes5=18, classes6=18, classes7=18, classes8=18,
  classes9=18, classes10=18,classes11=18,classes12=18,classes13=18,classes14=18,classes15=18,classes16=18;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 2507;
UPDATE aotv4_tmpl SET id=43338, name='Mark of Reprisal',   descnum=43338, mana=50,  spellgroup=43336, rank=3,
  effect_base_value1=-8,
  classes1=28, classes2=28, classes3=28, classes4=28, classes5=28, classes6=28, classes7=28, classes8=28,
  classes9=28, classes10=28,classes11=28,classes12=28,classes13=28,classes14=28,classes15=28,classes16=28;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 2507;
UPDATE aotv4_tmpl SET id=43339, name='Mark of Reckoning',  descnum=43339, mana=75,  spellgroup=43336, rank=4,
  effect_base_value1=-11,
  classes1=38, classes2=38, classes3=38, classes4=38, classes5=38, classes6=38, classes7=38, classes8=38,
  classes9=38, classes10=38,classes11=38,classes12=38,classes13=38,classes14=38,classes15=38,classes16=38;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 2507;
UPDATE aotv4_tmpl SET id=43340, name='Mark of Requital',   descnum=43340, mana=100, spellgroup=43336, rank=5,
  effect_base_value1=-14,
  classes1=48, classes2=48, classes3=48, classes4=48, classes5=48, classes6=48, classes7=48, classes8=48,
  classes9=48, classes10=48,classes11=48,classes12=48,classes13=48,classes14=48,classes15=48,classes16=48;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 2507;
UPDATE aotv4_tmpl SET id=43341, name='Mark of Recompense', descnum=43341, mana=125, spellgroup=43336, rank=6,
  effect_base_value1=-17,
  classes1=58, classes2=58, classes3=58, classes4=58, classes5=58, classes6=58, classes7=58, classes8=58,
  classes9=58, classes10=58,classes11=58,classes12=58,classes13=58,classes14=58,classes15=58,classes16=58;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;

-- ---------------------------------------------------------------- descriptions
INSERT INTO db_str (id, type, value) VALUES
 (43336, 6, 'Marks your target. Each time the marked creature lands a melee hit, it wounds itself for 3 damage.'),
 (43337, 6, 'Marks your target. Each time the marked creature lands a melee hit, it wounds itself for 5 damage.'),
 (43338, 6, 'Marks your target. Each time the marked creature lands a melee hit, it wounds itself for 8 damage.'),
 (43339, 6, 'Marks your target. Each time the marked creature lands a melee hit, it wounds itself for 11 damage.'),
 (43340, 6, 'Marks your target. Each time the marked creature lands a melee hit, it wounds itself for 14 damage.'),
 (43341, 6, 'Marks your target. Each time the marked creature lands a melee hit, it wounds itself for 17 damage.');
