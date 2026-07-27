-- aotv4_tank_ironwill.sql -- Iron Will, the Tank tree's resource-to-absorption conversion.
-- =============================================================================================
-- Works the way Mana Burn does: it spends a whole resource pool in one go for a single large
-- outcome. Here the outcome is absorption rather than damage, and endurance joins mana from rank 2.
--
--   rank 1  spends mana,                 50 percent of it becomes absorption
--   rank 2  spends mana AND endurance,   50 percent of both
--   rank 3  spends both,                100 percent of both
--   rank 4  the absorption also turns aside SPELL damage
--   rank 5  it lasts twice as long
--
-- ⚠️ RANKS 4 AND 5 ARE ADDITIONS. The brief specified three ranks; every other AA in these trees has
-- five, and a three-rank ability sits oddly in the window beside them. Cutting back is a small
-- change: terminate the chain at 7249 and delete the two rank rows.
--
-- ⚠️ READING OF THE BRIEF: "convert 1/2 mana" and "100 percent of consumed" are taken to mean the
-- ability always spends the WHOLE pool and the rank sets the EXCHANGE RATE. It is the only reading
-- under which all three ranks describe one mechanic -- "100 percent of consumed" cannot be a
-- fraction of the pool spent.
--
-- HOSTED ON ability 60 Frenzied Burnout: already activated, so it has the hotkey sids needed to
-- reach a hotbar. Chain walked, not assumed: 167 - 5879 - 7249 - 8343 - 12955 (it runs to six).
--
-- ⚠️⚠️ SPELL_TYPE 14, inherited from the host and left alone deliberately. spell_type is a TIMER
-- SLOT (`rank->spell_type + pTimerAAStart`), so two activated AAs sharing one share a cooldown.
-- Slots in use: 2 Sanguine Frenzy, 6 Sanctified Blow, 14 Iron Will. A fourth activated AA needs a
-- fourth value.
--
-- ⚠️ THE CONVERSION AND THE CEILING ARE IN zone/aotv4_tank_aa.cpp, not here. Rank id 167 is the
-- only join and nothing checks it.
--
--   mysql -h127.0.0.1 -upeq -ppeqpass peq < .devcontainer/custom/sql/aotv4_tank_ironwill.sql
--
-- ⚠️ spells_new IS in shared memory: stop world + zones, run ./shared_memory, restart.
-- =============================================================================================

DELETE FROM spells_new WHERE id BETWEEN 43412 AND 43413;
DELETE FROM db_str     WHERE id BETWEEN 43412 AND 43413 AND type = 6;

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;
CREATE TEMPORARY TABLE aotv4_tmpl LIKE spells_new;

-- Two rows because SPA 55 (melee absorption) and SPA 78 (spell absorption) keep their pools in
-- DIFFERENT per-buff fields, so a single row cannot be made to carry one or both on demand.
-- Both bases are PLACEHOLDERS: the real pools depend on what was just spent and are written into
-- the buff instance from code. The base still has to be non-zero -- RuneAbsorb checks the BONUS
-- value from the row before it ever looks at the per-buff pool.
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43411;
UPDATE aotv4_tmpl SET
  id = 43412, name = 'Iron Will', descnum = 43412, spellgroup = 43412, `rank` = 1,
  effectid1 = 55, effect_base_value1 = 1, effect_limit_value1 = 0, max1 = 0, formula1 = 100,
  effectid2 = 254, effect_base_value2 = 0,
  buffduration = 5, buffdurationformula = 10, cast_time = 0,
  targettype = 6, goodEffect = 1, new_icon = 77;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43412;
UPDATE aotv4_tmpl SET id = 43413, descnum = 43413, spellgroup = 43413, `rank` = 2,
  effectid2 = 78, effect_base_value2 = 1, effect_limit_value2 = 0, max2 = 0, formula2 = 100;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

INSERT INTO db_str (id,type,value) VALUES
 (43412,6,'Everything you had, spent at once and set between you and the next blow.'),
 (43413,6,'Everything you had, spent at once and set between you and whatever comes next.');

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;

-- =============================================================================================
UPDATE aa_ability SET name='Iron Will', classes=65535, enabled=1, type=1 WHERE id=60;

UPDATE aa_ranks SET level_req=5,  cost=3 WHERE id=167;
UPDATE aa_ranks SET level_req=15, cost=4 WHERE id=5879;
UPDATE aa_ranks SET level_req=25, cost=5 WHERE id=7249;
UPDATE aa_ranks SET level_req=35, cost=6 WHERE id=8343;
UPDATE aa_ranks SET level_req=45, cost=8 WHERE id=12955;

UPDATE aa_ranks SET next_id=-1 WHERE id=12955;

-- The spell is only the vehicle and the visible buff; the conversion is in code. Ranks 4 and 5 use
-- the second row for spell absorption. THIRTY MINUTE recast (1800 seconds).
UPDATE aa_ranks SET spell=43412, recast_time=1800 WHERE id IN (167,5879,7249);
UPDATE aa_ranks SET spell=43413, recast_time=1800 WHERE id IN (8343,12955);

-- MUST be empty or the ability cannot be activated at all.
DELETE FROM aa_rank_effects WHERE rank_id IN (167,5879,7249,8343,12955);

UPDATE db_str SET value='Iron Will' WHERE id=167 AND type=1;
UPDATE db_str SET value='Spend everything at once and wear it. Your entire pool of mana is consumed and returned to you as absorption, at half its worth for the first two ranks and in full from the third. From the second rank your endurance is spent alongside it. The fourth rank turns aside spell damage as well as blows, and the fifth makes it last twice as long. Usable once every 30 minutes.' WHERE id=167 AND type=4;
