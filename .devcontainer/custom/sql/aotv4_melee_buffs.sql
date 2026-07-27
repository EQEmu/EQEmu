-- aotv4_melee_buffs.sql -- the spell rows the Melee AA tree needs.
-- =============================================================================================
-- ⚠️ HELPER BAND. 43400-43405 sit above 43350, so gen_stock_pool.pl never pulls them into the
-- reward pool. Anything in 43300-43349 WOULD be offered as a level-up reward.
--
--   43400-43404  Bloodletting     SPA 0 damage over time, one row per AA rank
--   43405        Sanguine Frenzy  the activated AA's spell -- visual only, see below
--
-- ⚠️ WHY ONE BLEED ROW PER RANK. EQ will not stack the same spell id from one caster, so the bleed
-- refreshes rather than stacks, and per-rank damage therefore has to live in separate rows. Same
-- reason Borrowed Breath needed five.
--
-- ⚠️ THE ORIGINAL DESIGN SAID RANK 5 "TICKS TWICE AS FAST". It cannot: buff tics are driven by a
-- fixed 6 second timer (Mob::tic_timer, BuffProcess), so tick RATE is not something a spell row can
-- change. Rank 5 doubles the damage per tick instead, which is the same damage per second.
--
-- ⚠️ 43405 IS DELIBERATELY ALMOST INERT. Client::ActivateAlternateAdvancementAbility refuses to fire
-- an AA whose spell fails IsValidSpell, so the AA needs a real castable spell -- but the actual
-- effect (a capped share of weapon damage returned as health for exactly 4 seconds) is enforced in
-- C++, because a 4 second buff is not expressible: durations are whole 6 second tics and the timer
-- free-runs, so a one-tic buff can expire almost immediately. It carries SPA 178 at 1 percent so it
-- is a genuine, valid buff rather than an all-254 shell, and so the player can see the window is up.
-- The 1 percent is noise next to the code's own contribution.
--
--   mysql -h127.0.0.1 -upeq -ppeqpass peq < .devcontainer/custom/sql/aotv4_melee_buffs.sql
--
-- ⚠️ spells_new IS in shared memory: stop world + zones, run ./shared_memory, restart.
-- =============================================================================================

DELETE FROM spells_new WHERE id BETWEEN 43400 AND 43405;
DELETE FROM db_str     WHERE id BETWEEN 43400 AND 43405 AND type = 6;

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;
CREATE TEMPORARY TABLE aotv4_tmpl LIKE spells_new;

-- Clone through a temp table so all ~236 columns stay byte-identical to a working stock row.
-- Never hand-list the columns: a missed one is a silently malformed spell.

-- ------------------------------------------------------------------ 43400-43404 Bloodletting
-- Template 4088 Ward of Vie: single target, buff, sane duration handling. Overridden to a
-- detrimental damage-over-time. goodEffect 0 and a NEGATIVE base is what makes SPA 0 damage.
--
-- NB: BuffProcess strips detrimental buffs from PLAYER-SIDE entities that are out of combat
-- (cleanse-on-peace), but wild NPCs are explicitly excluded, so a bleed on a monster runs normally.
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 4088;
UPDATE aotv4_tmpl SET
  id = 43400, name = 'Bloodletting', descnum = 43400, spellgroup = 43400, `rank` = 1, mana = 0,
  effectid1 = 0, effect_base_value1 = -4, effect_limit_value1 = 0, max1 = 0,
  effectid2 = 254, effect_base_value2 = 0, effect_limit_value2 = 0, max2 = 0,
  buffduration = 3, buffdurationformula = 10,
  targettype = 5, goodEffect = 0, new_icon = 68, spell_category = -99,
  classes1=255, classes2=255, classes3=255, classes4=255, classes5=255, classes6=255,
  classes7=255, classes8=255, classes9=255, classes10=255, classes11=255, classes12=255,
  classes13=255, classes14=255, classes15=255, classes16=255;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43400;
UPDATE aotv4_tmpl SET id = 43401, descnum = 43401, spellgroup = 43401, `rank` = 2,
  effect_base_value1 = -7, buffduration = 3;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43400;
UPDATE aotv4_tmpl SET id = 43402, descnum = 43402, spellgroup = 43402, `rank` = 3,
  effect_base_value1 = -7, buffduration = 5;   -- rank 3 lengthens it
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43400;
UPDATE aotv4_tmpl SET id = 43403, descnum = 43403, spellgroup = 43403, `rank` = 4,
  effect_base_value1 = -11, buffduration = 5;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43400;
UPDATE aotv4_tmpl SET id = 43404, descnum = 43404, spellgroup = 43404, `rank` = 5,
  effect_base_value1 = -22, buffduration = 5;  -- "twice as fast" expressed as twice the damage
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

-- ------------------------------------------------------------------------ 43405 Sanguine Frenzy
-- Self buff, instant cast, no mana. cast_time 0 matters: the AA fires it through CastSpell and a
-- cast bar on a 4 second window would eat most of the window.
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 4088;
UPDATE aotv4_tmpl SET
  id = 43405, name = 'Sanguine Frenzy', descnum = 43405, spellgroup = 43405, `rank` = 1, mana = 0,
  effectid1 = 178, effect_base_value1 = 1, effect_limit_value1 = 0, max1 = 0,
  effectid2 = 254, effect_base_value2 = 0, effect_limit_value2 = 0, max2 = 0,
  buffduration = 1, buffdurationformula = 10,
  cast_time = 0, recovery_time = 0, recast_time = 0,
  targettype = 6, goodEffect = 1, new_icon = 61, spell_category = -99,
  classes1=255, classes2=255, classes3=255, classes4=255, classes5=255, classes6=255,
  classes7=255, classes8=255, classes9=255, classes10=255, classes11=255, classes12=255,
  classes13=255, classes14=255, classes15=255, classes16=255;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

-- ------------------------------------------------------------------------------- spell hover text
-- ⚠️ Client::Message is printf style, so a literal % is eaten as a format token. Spell out "percent".
INSERT INTO db_str (id,type,value) VALUES
 (43400,6,'A wound that will not close.'),
 (43401,6,'A wound that will not close.'),
 (43402,6,'A wound that will not close.'),
 (43403,6,'A wound that will not close.'),
 (43404,6,'A wound that will not close.'),
 (43405,6,'Your weapons thirst, and what they take, they give back to you.');

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;
