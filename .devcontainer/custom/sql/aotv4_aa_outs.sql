-- aotv4_aa_outs.sql -- an endurance-funded emergency ability for every tree, plus three utility AAs.
-- =============================================================================================
--   host 173 Eldritch Rune       517,518,519,1440,1441  -> Last Stand  (tank)   ACTIVATED st=12
--   host 111 War Cry             260,261,262,8309,8310  -> Rally       (tank)   ACTIVATED st=20
--   host 180 Hand of Piety       534,535,536,715,716    -> Reprieve    (healer) ACTIVATED st=4
--   host  18 Healing Adept        77, 78, 79,434,435     -> Convalesce  (healer) passive
--   host  52 Improved Familiar   155,533,1344,5279,5289 -> Fade        (ranged) ACTIVATED st=8
--   host  13 Innate Run Speed     62, 63, 64,672,673     -> Quickening  (ranged) passive, NATIVE SPA
--   host 170 Wrath of the Wild   510,511,512,7425,7426  -> Disengage   (melee)  ACTIVATED st=37
--
-- Chains walked, and title_sid/desc_sid confirmed equal to first_rank_id on all seven (the trap
-- that made Overload show Quick Damage's description).
--
-- ⚠️⚠️ TIMER SLOTS. spell_type is a TIMER SLOT: two activated AAs sharing one share a cooldown.
-- Now in use: 2 Sanguine Frenzy, 3 Second Wind, 4 Reprieve, 6 Sanctified Blow, 8 Fade,
-- 12 Last Stand, 14 Iron Will, 20 Rally, 37 Disengage. War Cry ships on 3 and is MOVED to 20.
--
-- ⚠️ WHY 45 MINUTES AND NOT AN ENDURANCE COST ALONE. Endurance regenerates in minutes, so on a
-- long timer the cost by itself is decorative. What makes it a decision is that the same resource
-- funds Iron Will and Second Wind -- spend it and your emergency exit is disarmed until it returns.
-- The code requires 50 percent of maximum endurance as well as consuming it.
--
-- ⚠️ Recast timers are PERSISTENT (p_timers.Store/Load), so camping or zoning cannot reset one.
--
--   mysql -h127.0.0.1 -upeq -ppeqpass peq < .devcontainer/custom/sql/aotv4_aa_outs.sql
--
-- ⚠️ spells_new IS in shared memory: stop world + zones, run ./shared_memory, restart.
-- =============================================================================================

DELETE FROM spells_new WHERE id BETWEEN 43440 AND 43444;
DELETE FROM db_str     WHERE id BETWEEN 43440 AND 43444 AND type = 6;

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;
CREATE TEMPORARY TABLE aotv4_tmpl LIKE spells_new;

-- Four of these five are VEHICLES: an activated AA is refused outright unless rank->spell passes
-- IsValidSpell, but the behaviour is in code. They carry a token real effect rather than an
-- all-254 shell so they are valid buffs and the player can see something happened.
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43412;
UPDATE aotv4_tmpl SET
  id = 43440, name = 'Last Stand', descnum = 43440, spellgroup = 43440,
  effectid1 = 162, effect_base_value1 = 100, effect_limit_value1 = 1, max1 = 0, formula1 = 100,
  buffduration = 3, targettype = 6, goodEffect = 1, new_icon = 128;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43440;
UPDATE aotv4_tmpl SET id=43441, name='Reprieve', descnum=43441, spellgroup=43441,
  effectid1=0, effect_base_value1=1, effect_limit_value1=0, max1=0, buffduration=2, new_icon=99;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

-- Fade carries a REAL effect: invisibility, so slipping away actually works.
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43440;
UPDATE aotv4_tmpl SET id=43442, name='Fade', descnum=43442, spellgroup=43442,
  effectid1=18, effect_base_value1=1, effect_limit_value1=0, max1=0, buffduration=2, new_icon=21;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

-- Disengage carries a REAL effect too: SPA 3 movement speed. ⚠️ POSITIVE base = faster; a negative
-- value here would be a snare, which is the exact opposite of the point.
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43440;
UPDATE aotv4_tmpl SET id=43443, name='Disengage', descnum=43443, spellgroup=43443,
  effectid1=3, effect_base_value1=60, effect_limit_value1=0, max1=0, buffduration=2, new_icon=57;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43440;
UPDATE aotv4_tmpl SET id=43444, name='Rally', descnum=43444, spellgroup=43444,
  effectid1=0, effect_base_value1=1, effect_limit_value1=0, max1=0, buffduration=1,
  targettype=5, new_icon=54;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

INSERT INTO db_str (id,type,value) VALUES
 (43440,6,'Your feet are set. This is far enough.'),
 (43441,6,'A second breath, and your footing with it.'),
 (43442,6,'The world has lost interest in you.'),
 (43443,6,'Nothing can hold you.'),
 (43444,6,'A voice cutting through the panic.');

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;

-- =============================================================================================
UPDATE aa_ability SET name='Last Stand', classes=65535, enabled=1, type=1 WHERE id=173;
UPDATE aa_ability SET name='Rally',      classes=65535, enabled=1, type=1 WHERE id=111;
UPDATE aa_ability SET name='Reprieve',   classes=65535, enabled=1, type=2 WHERE id=180;
UPDATE aa_ability SET name='Convalesce', classes=65535, enabled=1, type=2 WHERE id=18;
UPDATE aa_ability SET name='Fade',       classes=65535, enabled=1, type=3 WHERE id=52;
UPDATE aa_ability SET name='Quickening', classes=65535, enabled=1, type=3 WHERE id=13;
UPDATE aa_ability SET name='Disengage',  classes=65535, enabled=1, type=4 WHERE id=170;

UPDATE aa_ranks SET level_req=5,  cost=3 WHERE id IN (517, 260, 534, 77,  155,  62,  510);
UPDATE aa_ranks SET level_req=15, cost=4 WHERE id IN (518, 261, 535, 78,  533,  63,  511);
UPDATE aa_ranks SET level_req=25, cost=5 WHERE id IN (519, 262, 536, 79,  1344, 64,  512);
UPDATE aa_ranks SET level_req=35, cost=6 WHERE id IN (1440,8309,715, 434, 5279, 672, 7425);
UPDATE aa_ranks SET level_req=45, cost=8 WHERE id IN (1441,8310,716, 435, 5289, 673, 7426);

UPDATE aa_ranks SET next_id=-1 WHERE id IN (1441,8310,716,435,5289,673,7426);

DELETE FROM aa_rank_effects WHERE rank_id IN
 (517,518,519,1440,1441, 260,261,262,8309,8310, 534,535,536,715,716,
  77,78,79,434,435, 155,533,1344,5279,5289, 62,63,64,672,673, 510,511,512,7425,7426);

-- The four outs: 45 minutes, each on its own timer slot.
UPDATE aa_ranks SET spell=43440, recast_time=2700, spell_type=12 WHERE id IN (517,518,519,1440,1441);
UPDATE aa_ranks SET spell=43441, recast_time=2700, spell_type=4  WHERE id IN (534,535,536,715,716);
UPDATE aa_ranks SET spell=43442, recast_time=2700, spell_type=8  WHERE id IN (155,533,1344,5279,5289);
UPDATE aa_ranks SET spell=43443, recast_time=2700, spell_type=37 WHERE id IN (510,511,512,7425,7426);
-- Rally is NOT an out: no endurance threshold, and 3 minutes rather than 45.
UPDATE aa_ranks SET spell=43444, recast_time=180,  spell_type=20 WHERE id IN (260,261,262,8309,8310);

-- Quickening is entirely native -- SPA 127 IncreaseSpellHaste, "cast time mod pct". No code at all,
-- the same way Relentless uses SPA 225.
INSERT INTO aa_rank_effects (rank_id,slot,effect_id,base1,base2) VALUES
 (62,1,127,2,0),(63,1,127,4,0),(64,1,127,6,0),(672,1,127,9,0),(673,1,127,12,0);

UPDATE db_str SET value='Last Stand' WHERE id=517 AND type=1;
UPDATE db_str SET value='Rally'      WHERE id=260 AND type=1;
UPDATE db_str SET value='Reprieve'   WHERE id=534 AND type=1;
UPDATE db_str SET value='Convalesce' WHERE id=77  AND type=1;
UPDATE db_str SET value='Fade'       WHERE id=155 AND type=1;
UPDATE db_str SET value='Quickening' WHERE id=62  AND type=1;
UPDATE db_str SET value='Disengage'  WHERE id=510 AND type=1;

UPDATE db_str SET value='Set your feet and refuse to fall. For 8, 10, 12, 14 and 15 seconds nothing can reduce you below a fifth of your maximum health. Requires and consumes at least half your endurance, so spending that endurance elsewhere leaves you without it. Usable once every 45 minutes.' WHERE id=517 AND type=4;
UPDATE db_str SET value='Cut through the panic. Frees you and your whole group of fear, root and snare. Usable once every 3 minutes.' WHERE id=260 AND type=4;
UPDATE db_str SET value='Find a second breath. Restores 20, 27, 34, 42 and 50 percent of your maximum health and frees you of root and snare. It turns aside no damage at all, so it buys you an escape rather than a stand. Requires and consumes at least half your endurance. Usable once every 45 minutes.' WHERE id=534 AND type=4;
UPDATE db_str SET value='Rest is worth more in company. While out of combat, you and your group recover an additional 4, 8, 12, 16 and 20 health and 3, 6, 9, 12 and 15 mana each tick.' WHERE id=77 AND type=4;
UPDATE db_str SET value='Slip out of the world attention. Everything currently angry at you forgets, and you are briefly unseen. In a group this hands whatever was hitting you to somebody else. Requires and consumes at least half your endurance. Usable once every 45 minutes.' WHERE id=155 AND type=4;
UPDATE db_str SET value='Your casting quickens. Reduces the casting time of your spells by 2, 4, 6, 9 and 12 percent.' WHERE id=62 AND type=4;
UPDATE db_str SET value='Break contact and run. Frees you of root and snare and greatly increases your movement for a few seconds. It turns aside no damage whatsoever, so you must actually leave. Requires and consumes at least half your endurance. Usable once every 45 minutes.' WHERE id=510 AND type=4;
