-- aotv4_tank_stun.sql -- Sanctified Blow, the Tank tree's activated stun.
-- =============================================================================================
-- A stun in the shape of the native Paladin AA (ability 73 Divine Stun, spell 2190), which lands a
-- chance at a protective ward. The stun is the basis; the ward is what makes it a tank ability
-- rather than just interrupt utility.
--
-- HOSTED ON DIVINE STUN ITSELF (ability 73). That is the ideal host: it is already an activated AA,
-- so it already has the hotkey sids that let it be dragged to a hotbar, and its ranks already carry
-- a stun spell. Chain walked, not assumed: 188 - 1277 - 5044 - 7339 - 7662 (it runs to 7 ranks
-- natively; we terminate at five).
--
-- ⚠️⚠️ SPELL_TYPE MOVED FROM 2 TO 6. spell_type is a TIMER SLOT, not a category -- the recast is
-- keyed on `rank->spell_type + pTimerAAStart` (Client::ActivateAlternateAdvancementAbility). Divine
-- Stun ships on slot 2, which is the slot Sanguine Frenzy inherited from Cannibalization, so leaving
-- it would have made the two activated AAs SHARE ONE COOLDOWN. Any future activated AA needs its own
-- distinct value here.
--
-- Requirements for an activated AA, all enforced by the engine:
--   a. a valid spell on every rank            -> 43406-43410
--   b. NO aa_rank_effects on any rank         -> deleted below; this is how the engine tells an
--                                                activated AA from a passive one
--   c. recast_time in SECONDS                 -> 30, matching the native Divine Stun
--
-- ⚠️ THE WARD IS NOT IN THE SPELL. The chance and the size are in zone/aotv4_tank_aa.cpp, because
-- neither can be expressed here: the size is a share of the striker's own maximum health, and there
-- is no SPA for "on a successful stun". Rank id 188 is the ONLY join between this file and that code.
--
--   mysql -h127.0.0.1 -upeq -ppeqpass peq < .devcontainer/custom/sql/aotv4_tank_stun.sql
--
-- ⚠️ spells_new IS in shared memory: stop world + zones, run ./shared_memory, restart.
-- AAs are not -- a zone restart is enough for the AA rows.
-- =============================================================================================

DELETE FROM spells_new WHERE id BETWEEN 43406 AND 43411;
DELETE FROM db_str     WHERE id BETWEEN 43406 AND 43411 AND type = 6;

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;
CREATE TEMPORARY TABLE aotv4_tmpl LIKE spells_new;

-- --------------------------------------------------------------------- 43406-43410 the stun
-- Cloned from 2190 Divine Stun: SPA 21, single target, instant cast, magic resist.
--
-- ⚠️ SPA 21 FIELD MEANINGS (zone/spell_effects.cpp, case Stun): base = stun length in MILLISECONDS,
-- max = the highest target LEVEL it can stun. A max of 0 does NOT mean "no limit" -- it falls back
-- to RuleI(Spells, BaseImmunityLevel), which is 55. Stock 2190 sets 70, so it must be set explicitly
-- or the stun silently stops working on anything past the mid levels.
-- Targets with SpecialAbility::StunImmunity are immune regardless; most raid bosses have it.
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 2190;
UPDATE aotv4_tmpl SET
  id = 43406, name = 'Sanctified Blow', descnum = 43406, spellgroup = 43406, `rank` = 1, mana = 0,
  effect_base_value1 = 2000, max1 = 70,
  cast_time = 0, recovery_time = 0, recast_time = 0,
  spell_category = -99,
  classes1=255, classes2=255, classes3=255, classes4=255, classes5=255, classes6=255,
  classes7=255, classes8=255, classes9=255, classes10=255, classes11=255, classes12=255,
  classes13=255, classes14=255, classes15=255, classes16=255;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43406;
UPDATE aotv4_tmpl SET id=43407, descnum=43407, spellgroup=43407, `rank`=2,
  effect_base_value1 = 2000, max1 = 70;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

-- Rank 3 is the quality rank: a longer stun that also holds tougher targets.
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43406;
UPDATE aotv4_tmpl SET id=43408, descnum=43408, spellgroup=43408, `rank`=3,
  effect_base_value1 = 3000, max1 = 75;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43406;
UPDATE aotv4_tmpl SET id=43409, descnum=43409, spellgroup=43409, `rank`=4,
  effect_base_value1 = 3000, max1 = 75;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43406;
UPDATE aotv4_tmpl SET id=43410, descnum=43410, spellgroup=43410, `rank`=5,
  effect_base_value1 = 3000, max1 = 75;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

-- ------------------------------------------------------------------------- 43411 the ward itself
-- SPA 55 Rune. base 1 is a PLACEHOLDER: Mob::AoTv4WardOnStun overwrites buffs[slot].melee_rune with
-- the real pool, because it scales with the caster's level and so is only known at cast time. The
-- base still has to be non-zero -- RuneAbsorb checks the BONUS value (from this row) before it ever
-- looks at the per-buff pool, so a base of 0 would make the ward inert. Same trick as 43390.
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43390;
UPDATE aotv4_tmpl SET
  id = 43411, name = 'Sanctified Ward', descnum = 43411, spellgroup = 43411, `rank` = 1,
  buffduration = 5, buffdurationformula = 10,
  targettype = 6, new_icon = 77;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

INSERT INTO db_str (id,type,value) VALUES
 (43406,6,'A blow struck with conviction, and the sense to stop what it hits.'),
 (43407,6,'A blow struck with conviction, and the sense to stop what it hits.'),
 (43408,6,'A blow struck with conviction, and the sense to stop what it hits.'),
 (43409,6,'A blow struck with conviction, and the sense to stop what it hits.'),
 (43410,6,'A blow struck with conviction, and the sense to stop what it hits.'),
 (43411,6,'Faith answered in kind. Absorbs damage until it is spent.');

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;

-- =============================================================================================
-- The AA itself.
-- =============================================================================================
UPDATE aa_ability SET name='Sanctified Blow', classes=65535, enabled=1, type=1 WHERE id=73;

-- Same ladder as every other tree, so a role costs the same wherever you buy into it.
UPDATE aa_ranks SET level_req=5,  cost=3 WHERE id=188;
UPDATE aa_ranks SET level_req=15, cost=4 WHERE id=1277;
UPDATE aa_ranks SET level_req=25, cost=5 WHERE id=5044;
UPDATE aa_ranks SET level_req=35, cost=6 WHERE id=7339;
UPDATE aa_ranks SET level_req=45, cost=8 WHERE id=7662;

-- Stop at rank 5; natively this line runs to seven (10647, 13586).
UPDATE aa_ranks SET next_id=-1 WHERE id=7662;

-- One stun spell per rank, a 30 second recast, and OFF the shared timer slot.
UPDATE aa_ranks SET spell=43406, recast_time=30, spell_type=6 WHERE id=188;
UPDATE aa_ranks SET spell=43407, recast_time=30, spell_type=6 WHERE id=1277;
UPDATE aa_ranks SET spell=43408, recast_time=30, spell_type=6 WHERE id=5044;
UPDATE aa_ranks SET spell=43409, recast_time=30, spell_type=6 WHERE id=7339;
UPDATE aa_ranks SET spell=43410, recast_time=30, spell_type=6 WHERE id=7662;

-- MUST be empty or the ability cannot be activated at all.
DELETE FROM aa_rank_effects WHERE rank_id IN (188,1277,5044,7339,7662);

UPDATE db_str SET value='Sanctified Blow' WHERE id=188 AND type=1;
-- ⚠️ The ward's chance and size live in zone/aotv4_tank_aa.cpp (WARD_CHANCE, WARD_PCT_OF_MAXHP).
-- Nothing checks this text against them; keep them in step by hand.
UPDATE db_str SET value='Strike your target senseless for a moment. Each blow that lands has a 20, 30, 30, 40 and 100 percent chance to leave a ward about you, absorbing melee damage equal to 2, 4, 6, 8 and 10 percent of your maximum health until it is spent. The third rank stuns for longer and holds tougher opponents, and at the fifth rank the ward is certain. Reusable every 30 seconds.' WHERE id=188 AND type=4;
