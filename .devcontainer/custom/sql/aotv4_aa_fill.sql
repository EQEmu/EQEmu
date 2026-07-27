-- aotv4_aa_fill.sql -- the seven AAs that bring every tree to ten.
-- =============================================================================================
--   host 142 Planar Power                418,419,420,421,422    -> Weathered      TANK    native
--   host 128 Paragon of Spirit           291,1123,1124,1125,4969-> Communion      HEALER  ACTIVATED
--   host 154 Hastened Divinity           462,463,464,7994,7995  -> Practiced Grace HEALER native
--   host  97 Critical Mend               230,231,232,539,540    -> Enduring Grace HEALER  native
--   host 144 Innate Enlightenment        426,427,428,429,430    -> Far Sight      RANGED  native
--   host 114 Spell Casting Fury Mastery  267,268,269,640,641    -> Unbroken Conc. RANGED  native+code
--   host  75 Slay Undead                 190,191,192,1524,1526  -> Mana Shroud    RANGED  native
--
-- Chains walked; title_sid and desc_sid confirmed equal to first_rank_id on all seven.
--
-- ⚠️ SIX OF THESE SEVEN ARE PURE DATA -- native SPAs in aa_rank_effects, no code at all, the same
-- way Relentless (SPA 225) and Quickening (SPA 127) work. Only Unbroken Concentration needs code,
-- and only for its top rank.
--
-- ⚠️ COMMUNION TAKES TIMER SLOT 5. spell_type is a TIMER SLOT: two activated AAs sharing one share
-- a cooldown. Now in use: 2 Sanguine Frenzy, 3 Second Wind, 4 Reprieve, 5 Communion,
-- 6 Sanctified Blow, 8 Fade, 12 Last Stand, 14 Iron Will, 20 Rally, 37 Disengage.
--
--   mysql -h127.0.0.1 -upeq -ppeqpass peq < .devcontainer/custom/sql/aotv4_aa_fill.sql
--
-- ⚠️ spells_new IS in shared memory: stop world + zones, run ./shared_memory, restart.
-- =============================================================================================

DELETE FROM spells_new WHERE id BETWEEN 43450 AND 43454;
DELETE FROM db_str     WHERE id BETWEEN 43450 AND 43454 AND type = 6;

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;
CREATE TEMPORARY TABLE aotv4_tmpl LIKE spells_new;

-- --------------------------------------------------------------------- 43450-43454 Communion
-- A REAL group heal, one row per rank -- so Communion needs no code whatsoever. targettype 3 is
-- ST_Group, and the engine resolves it exactly as it does for any group heal spell.
-- Scaled with the caster's level by formula 103 (base + level*2) so it stays relevant across the
-- whole range without a row per level; max is the ceiling.
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43441;
UPDATE aotv4_tmpl SET
  id = 43450, name = 'Communion', descnum = 43450, spellgroup = 43450, `rank` = 1, mana = 0,
  effectid1 = 0, effect_base_value1 = 40, effect_limit_value1 = 0, max1 = 800, formula1 = 103,
  effectid2 = 254, effect_base_value2 = 0,
  buffduration = 0, buffdurationformula = 0, cast_time = 0, recovery_time = 0, recast_time = 0,
  targettype = 3, goodEffect = 1, new_icon = 99, spell_category = -99, resisttype = 0;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id=43450;
UPDATE aotv4_tmpl SET id=43451, descnum=43451, spellgroup=43451, `rank`=2, effect_base_value1=80;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id=43450;
UPDATE aotv4_tmpl SET id=43452, descnum=43452, spellgroup=43452, `rank`=3, effect_base_value1=120;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id=43450;
UPDATE aotv4_tmpl SET id=43453, descnum=43453, spellgroup=43453, `rank`=4, effect_base_value1=160;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id=43450;
UPDATE aotv4_tmpl SET id=43454, descnum=43454, spellgroup=43454, `rank`=5, effect_base_value1=200;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

INSERT INTO db_str (id,type,value) VALUES
 (43450,6,'Strength shared out among those standing with you.'),
 (43451,6,'Strength shared out among those standing with you.'),
 (43452,6,'Strength shared out among those standing with you.'),
 (43453,6,'Strength shared out among those standing with you.'),
 (43454,6,'Strength shared out among those standing with you.');

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;

-- =============================================================================================
UPDATE aa_ability SET name='Weathered',              classes=65535, enabled=1, type=1 WHERE id=142;
UPDATE aa_ability SET name='Communion',              classes=65535, enabled=1, type=2 WHERE id=128;
UPDATE aa_ability SET name='Practiced Grace',        classes=65535, enabled=1, type=2 WHERE id=154;
UPDATE aa_ability SET name='Enduring Grace',         classes=65535, enabled=1, type=2 WHERE id=97;
UPDATE aa_ability SET name='Far Sight',              classes=65535, enabled=1, type=3 WHERE id=144;
UPDATE aa_ability SET name='Unbroken Concentration', classes=65535, enabled=1, type=3 WHERE id=114;
UPDATE aa_ability SET name='Mana Shroud',            classes=65535, enabled=1, type=3 WHERE id=75;

UPDATE aa_ranks SET level_req=5,  cost=3 WHERE id IN (418, 291,  462,  230, 426, 267, 190);
UPDATE aa_ranks SET level_req=15, cost=4 WHERE id IN (419, 1123, 463,  231, 427, 268, 191);
UPDATE aa_ranks SET level_req=25, cost=5 WHERE id IN (420, 1124, 464,  232, 428, 269, 192);
UPDATE aa_ranks SET level_req=35, cost=6 WHERE id IN (421, 1125, 7994, 539, 429, 640, 1524);
UPDATE aa_ranks SET level_req=45, cost=8 WHERE id IN (422, 4969, 7995, 540, 430, 641, 1526);

UPDATE aa_ranks SET next_id=-1 WHERE id IN (422, 4969, 7995, 540, 430, 641, 1526);

DELETE FROM aa_rank_effects WHERE rank_id IN
 (418,419,420,421,422, 291,1123,1124,1125,4969, 462,463,464,7994,7995,
  230,231,232,539,540, 426,427,428,429,430, 267,268,269,640,641, 190,191,192,1524,1526);

-- Communion: the group heal spell per rank, on its own timer slot. 5 minutes.
-- ⚠️ MUST have no effect rows (deleted above) or the engine refuses to activate it at all.
UPDATE aa_ranks SET spell=43450, recast_time=300, spell_type=5 WHERE id=291;
UPDATE aa_ranks SET spell=43451, recast_time=300, spell_type=5 WHERE id=1123;
UPDATE aa_ranks SET spell=43452, recast_time=300, spell_type=5 WHERE id=1124;
UPDATE aa_ranks SET spell=43453, recast_time=300, spell_type=5 WHERE id=1125;
UPDATE aa_ranks SET spell=43454, recast_time=300, spell_type=5 WHERE id=4969;

-- The six native ones. No code touches any of these.
--   SPA 161 MitigateSpellDamage -- on an AA this is stackable percent reduction of SPELL damage.
--           Fills the only gap in the tank tree: everything else there is melee-only.
--   SPA 132 ReduceManaCost      -- base is the minimum percent, limit the maximum.
--   SPA 319 CriticalHealOverTime-- read in the buff_duration >= 1 branch of GetActSpellHealing.
--           Fills the only gap in the healer tree: every other healer AA skips heals-over-time.
--   SPA 129 IncreaseRange       -- spell range, percent.
--   SPA 235 ChannelChanceSpells -- MULTIPLIES an existing channel chance; it cannot reach immunity,
--           which is why rank 5 is handled in code instead (Mob::AoTv4CannotBeInterrupted).
--   SPA 329 ManaAbsorbPercentDamage -- damage comes off mana rather than health.
INSERT INTO aa_rank_effects (rank_id,slot,effect_id,base1,base2) VALUES
 (418,1,161,2,0),(419,1,161,4,0),(420,1,161,6,0),(421,1,161,8,0),(422,1,161,11,0),
 (462,1,132,3,15),(463,1,132,5,15),(464,1,132,7,15),(7994,1,132,9,15),(7995,1,132,11,15),
 (230,1,319,10,0),(231,1,319,18,0),(232,1,319,26,0),(539,1,319,34,0),(540,1,319,42,0),
 (426,1,129,5,0),(427,1,129,10,0),(428,1,129,15,0),(429,1,129,20,0),(430,1,129,25,0),
 (267,1,235,20,0),(268,1,235,40,0),(269,1,235,60,0),(640,1,235,80,0),
 (190,1,329,4,0),(191,1,329,8,0),(192,1,329,12,0),(1524,1,329,16,0),(1526,1,329,20,0);
-- ⚠️ rank 641 (Unbroken Concentration rank 5) gets NO effect row on purpose -- it is the marker the
-- code reads for outright immunity, and the native multiplier is redundant once you cannot be
-- interrupted at all.

UPDATE db_str SET value='Weathered'              WHERE id=418 AND type=1;
UPDATE db_str SET value='Communion'              WHERE id=291 AND type=1;
UPDATE db_str SET value='Practiced Grace'        WHERE id=462 AND type=1;
UPDATE db_str SET value='Enduring Grace'         WHERE id=230 AND type=1;
UPDATE db_str SET value='Far Sight'              WHERE id=426 AND type=1;
UPDATE db_str SET value='Unbroken Concentration' WHERE id=267 AND type=1;
UPDATE db_str SET value='Mana Shroud'            WHERE id=190 AND type=1;

UPDATE db_str SET value='Armour is no help against fire. Reduces the damage you take from spells and from anything that strikes an area by 2, 4, 6, 8 and 11 percent.' WHERE id=418 AND type=4;
UPDATE db_str SET value='Pour out your strength for everyone standing with you. Heals your entire group at once, for an amount that grows with your level. Usable once every 5 minutes.' WHERE id=291 AND type=4;
UPDATE db_str SET value='Practice tells. Your spells cost 3, 5, 7, 9 and 11 percent less mana.' WHERE id=462 AND type=4;
UPDATE db_str SET value='Even slow mercy can be sudden. Your healing over time gains a 10, 18, 26, 34 and 42 percent chance to critically heal, which no other ability of yours affects.' WHERE id=230 AND type=4;
UPDATE db_str SET value='Distance is its own armour. Increases the range of your spells by 5, 10, 15, 20 and 25 percent.' WHERE id=426 AND type=4;
UPDATE db_str SET value='Hold the thought whatever happens to the body. Greatly improves your chance to keep casting when struck, and at the fifth rank you cannot be interrupted at all.' WHERE id=267 AND type=4;
UPDATE db_str SET value='Let the magic take the blow instead. 4, 8, 12, 16 and 20 percent of the damage you take is drawn from your mana rather than your health.' WHERE id=190 AND type=4;
