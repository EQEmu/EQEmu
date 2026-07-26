-- aotv4_healer_buffs.sql -- the buff rows the Healer AA tree applies from code.
-- =============================================================================================
-- These are NOT inert markers. Every one of them does real work through a native SPA; the C++ only
-- decides WHEN to apply them and, for the shield, how big it is. That distinction matters -- the
-- marker-buff pattern used elsewhere (43022 Divine Aura, 43035 Blade Turn, the Thirst line, the
-- Shielded buffs) is a known placeholder we want to move away from, not extend.
--
-- ⚠️ HELPER BAND. 43390-43397 sit above 43350, so gen_stock_pool.pl never pulls them into the
-- reward pool. Anything in 43300-43349 WOULD be offered as a level-up reward, which would be
-- useless here -- none of these are castable by a player.
--
--   43390  Overflowing Grace   SPA 55  Rune          absorb pool written from code
--   43391  Borrowed Breath     SPA 162 flat mitigation, 3/hit, 1 tick     (AA rank 1)
--   43392  Borrowed Breath     SPA 162 flat mitigation, 5/hit, 1 tick     (AA rank 2)
--   43393  Borrowed Breath     SPA 162 flat mitigation, 6/hit, 2 ticks    (AA rank 3)
--   43394  Borrowed Breath     SPA 162 flat mitigation, 8/hit, 2 ticks    (AA rank 4)
--   43395  Borrowed Breath     SPA 162 flat mitigation, 10/hit, 2 ticks   (AA rank 5)
--   43396  Cleansing Renewal   SPA 0   regen                              (AA rank 3+)
--   43397  Grace Renewed       SPA 0   regen, paid when a shield is spent (AA rank 5)
--
-- ⚠️ WHY FIVE ROWS FOR BORROWED BREATH. The flat amount lives in the spell's LIMIT field and is
-- read during bonus calculation, so it cannot be varied per rank from code the way the shield's
-- size can -- bonuses are recalculated constantly and any poke would be reverted. One row per rank
-- is the robust way to express a per-rank amount for a native SPA.
--
-- ⚠️ SPA 162 SEMANTICS (zone/attack.cpp:3863). base = PERCENT of the hit to remove, limit = MAX
-- absorbed per hit, max = a depleting HP pool. base 100 + limit N + max 0 therefore means
-- "subtract exactly N from every hit, no pool, until it expires" -- a FLAT reduction. That is
-- deliberate: percentage mitigation is imperceptible against this server's post-mitigation numbers
-- (CLAUDE.md section 14), which is the same trap Passive Protection hit.
--
-- ⚠️ NO SPA 232 HERE. The death save is hand-rolled in C++; see the rank 5 note below.
-- (The short version: Divine Aura blocks all incoming heals, so the native save is a trap.)



--
--   mysql -h127.0.0.1 -upeq -ppeqpass peq < .devcontainer/custom/sql/aotv4_healer_buffs.sql
--
-- ⚠️ spells_new IS in shared memory: stop world + zones, run ./shared_memory, restart.
-- No pool regen needed -- nothing here is offerable.
-- =============================================================================================

DELETE FROM spells_new WHERE id BETWEEN 43390 AND 43397;
DELETE FROM db_str     WHERE id BETWEEN 43390 AND 43397 AND type = 6;

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;
CREATE TEMPORARY TABLE aotv4_tmpl LIKE spells_new;

-- Clone through a temp table so all ~236 columns stay byte-identical to a working stock row.
-- Never hand-list the columns: a missed one is a silently malformed spell.
-- Template 4088 Ward of Vie: single target, beneficial, buff, already a mitigation spell.

-- ---------------------------------------------------------------- 43390 Overflowing Grace (rune)
-- base 1 is a PLACEHOLDER. Mob::HealDamage overwrites buffs[slot].melee_rune with the real absorb
-- pool right after applying this, because the pool is a share of the overheal and so is only known
-- at cast time. The base still has to be non-zero: RuneAbsorb checks the BONUS value (from the row)
-- before it ever looks at the per-buff pool, so a base of 0 would make the shield inert.
-- Duration is 3 ticks here and extended to 5 from code at rank 3+.
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 4088;
UPDATE aotv4_tmpl SET
  id = 43390, name = 'Overflowing Grace', descnum = 43390, spellgroup = 43390, `rank` = 1, mana = 0,
  effectid1 = 55, effect_base_value1 = 1, effect_limit_value1 = 0, max1 = 0,
  effectid2 = 254, effect_base_value2 = 0, effect_limit_value2 = 0, max2 = 0,
  buffduration = 3, buffdurationformula = 10,
  targettype = 5, goodEffect = 1, new_icon = 77, spell_category = -99,
  classes1=255, classes2=255, classes3=255, classes4=255, classes5=255, classes6=255,
  classes7=255, classes8=255, classes9=255, classes10=255, classes11=255, classes12=255,
  classes13=255, classes14=255, classes15=255, classes16=255;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

-- ------------------------------------------------------------- 43391-43395 Borrowed Breath (flat)
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 4088;
UPDATE aotv4_tmpl SET
  id = 43391, name = 'Borrowed Breath', descnum = 43391, spellgroup = 43391, `rank` = 1, mana = 0,
  effectid1 = 162, effect_base_value1 = 100, effect_limit_value1 = 3, max1 = 0,
  effectid2 = 254, effect_base_value2 = 0, effect_limit_value2 = 0, max2 = 0,
  buffduration = 1, buffdurationformula = 10,
  targettype = 5, goodEffect = 1, new_icon = 128, spell_category = -99,
  classes1=255, classes2=255, classes3=255, classes4=255, classes5=255, classes6=255,
  classes7=255, classes8=255, classes9=255, classes10=255, classes11=255, classes12=255,
  classes13=255, classes14=255, classes15=255, classes16=255;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43391;
UPDATE aotv4_tmpl SET id = 43392, descnum = 43392, spellgroup = 43392, `rank` = 2,
  effect_limit_value1 = 5, buffduration = 1;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43391;
UPDATE aotv4_tmpl SET id = 43393, descnum = 43393, spellgroup = 43393, `rank` = 3,
  effect_limit_value1 = 6, buffduration = 2;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43391;
UPDATE aotv4_tmpl SET id = 43394, descnum = 43394, spellgroup = 43394, `rank` = 4,
  effect_limit_value1 = 8, buffduration = 2;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

-- Rank 5 is mitigation only. The death save that goes with it is NOT a spell effect -- see
-- Mob::AoTv4TryBorrowedBreath. It briefly carried SPA 232 DivineSave here, which was wrong:
-- Mob::TryDivineSave always casts 4789 Touch of the Divine on top of the save, and Divine Aura
-- blocks every incoming spell from every other caster (spells.cpp:4147) as well as your own
-- attacks and casts. It keeps you alive by taking you out of the fight for up to 36 seconds, and
-- makes you unattackable so the boss turns on your healer. The hand-rolled save has none of that.
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43391;
UPDATE aotv4_tmpl SET id = 43395, descnum = 43395, spellgroup = 43395, `rank` = 5,
  effect_limit_value1 = 10, buffduration = 2;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

-- ------------------------------------------------------------------------ 43396/43397 the regens
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 4088;
UPDATE aotv4_tmpl SET
  id = 43396, name = 'Cleansing Renewal', descnum = 43396, spellgroup = 43396, `rank` = 1, mana = 0,
  effectid1 = 0, effect_base_value1 = 8, effect_limit_value1 = 0, max1 = 0,
  effectid2 = 254, effect_base_value2 = 0, effect_limit_value2 = 0, max2 = 0,
  buffduration = 3, buffdurationformula = 10,
  targettype = 5, goodEffect = 1, new_icon = 99, spell_category = -99,
  classes1=255, classes2=255, classes3=255, classes4=255, classes5=255, classes6=255,
  classes7=255, classes8=255, classes9=255, classes10=255, classes11=255, classes12=255,
  classes13=255, classes14=255, classes15=255, classes16=255;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43396;
UPDATE aotv4_tmpl SET id = 43397, name = 'Grace Renewed', descnum = 43397, spellgroup = 43397,
  effect_base_value1 = 10, buffduration = 3;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

-- ------------------------------------------------------------------------------- spell hover text
-- ⚠️ Client::Message is printf style, so a literal % is eaten as a format token. Spell out "percent".
INSERT INTO db_str (id,type,value) VALUES
 (43390,6,'A shield of unspent healing, woven from grace that had nowhere else to go. Absorbs melee damage until it is spent.'),
 (43391,6,'Borrowed strength steadies you. Reduces the damage of each melee hit against you.'),
 (43392,6,'Borrowed strength steadies you. Reduces the damage of each melee hit against you.'),
 (43393,6,'Borrowed strength steadies you. Reduces the damage of each melee hit against you.'),
 (43394,6,'Borrowed strength steadies you. Reduces the damage of each melee hit against you.'),
 (43395,6,'Borrowed strength steadies you. Reduces the damage of each melee hit against you, and a blow that would kill you leaves you standing instead.'),
 (43396,6,'The cure lingers, knitting what the affliction tore.'),
 (43397,6,'The shield gave everything it had, and what remained returns to you.');

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;
