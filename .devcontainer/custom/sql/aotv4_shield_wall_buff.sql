-- aotv4_shield_wall_buff.sql -- the visible "Shielded" buff for the /shield damage-splitting system.
-- =============================================================================================
-- Without this the only sign you are being shielded is the one-off chat line when it starts. The
-- buff gives the shielded player something they can look at mid-fight to confirm it is still up,
-- and it disappears the moment the pairing breaks (walking apart, death, zoning, toggling off) --
-- which is exactly when you most need to notice.
--
-- ⚠️ THE BUFF CANNOT NAME THE SHIELDER. EQ takes a buff's name from the spell row, so it is the same
-- text for everyone; there is no way to render "Being shielded by Ashrem" in the buff bar. Who is
-- shielding you comes from the chat message the ability already prints on start and stop
-- (START_SHIELDING / END_SHIELDING). This is deliberate, not an oversight.
--
-- ⚠️ ID 43380 is in the HELPER band (43350+), NOT the offered band (43300-43349). If it lived in the
-- offered band, gen_stock_pool.pl would pull it into the reward pool and players could be handed
-- "Shielded" as a level-up reward, which does nothing when cast.
--
-- Inert marker: every effect slot is 254. The damage split is done in C++
-- (Mob::ApplyShieldWall, zone/attack.cpp); this row exists purely to be visible. Same pattern as
-- 43022 Divine Aura / 43035 Blade Turn / the Thirst line.
--
-- Applied and removed from Mob::ShieldWallAdd / ShieldWallRemove via ApplySpellBuff() and
-- BuffFadeBySpellID(). The long duration is a backstop only -- the C++ owns the lifetime.
--
--   mysql -h127.0.0.1 -upeq -ppeqpass peq < .devcontainer/custom/sql/aotv4_shield_wall_buff.sql
--
-- ⚠️ spells_new IS in shared memory (section 10): stop world + zones, run ./shared_memory, restart.
-- No pool regen needed -- 43380-43382 are never offered.
-- =============================================================================================

DELETE FROM spells_new WHERE id BETWEEN 43380 AND 43382;
DELETE FROM db_str    WHERE id BETWEEN 43380 AND 43382 AND type = 6;

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;
CREATE TEMPORARY TABLE aotv4_tmpl LIKE spells_new;

-- Cloned from 43342 Faint Thirst: already an inert marker buff with a real duration and icon.
-- ONE SPELL PER SHIELDER (43380/81/82). EQ will not stack the same spell id from two casters, so a
-- single row could only ever display one name; three distinct ids let all three shielders appear as
-- separate buffs, each carrying its own caster. They are all named "Shielded" on purpose -- the
-- player sees three identical buffs and inspects each to find out who. Distinct spellgroups keep
-- them from overwriting one another. Cap matches AoT:ShieldWallMaxSharers 4 (holder + 3).
DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43342;
UPDATE aotv4_tmpl SET
  id = 43380, name = 'Shielded', descnum = 43380, spellgroup = 43380, rank = 1,
  mana = 0, buffduration = 6000, buffdurationformula = 10,
  numhits = 0, numhitstype = 0,          -- charges belong to the Thirst line, not here
  targettype = 6, goodEffect = 1, new_icon = 158, spell_category = -99,
  classes1=255, classes2=255, classes3=255, classes4=255, classes5=255, classes6=255,
  classes7=255, classes8=255, classes9=255, classes10=255, classes11=255, classes12=255,
  classes13=255, classes14=255, classes15=255, classes16=255;   -- not learnable by anyone
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43342;
UPDATE aotv4_tmpl SET
  id = 43381, name = 'Shielded', descnum = 43381, spellgroup = 43381, rank = 1,
  mana = 0, buffduration = 6000, buffdurationformula = 10,
  numhits = 0, numhitstype = 0,
  targettype = 6, goodEffect = 1, new_icon = 158, spell_category = -99,
  classes1=255, classes2=255, classes3=255, classes4=255, classes5=255, classes6=255,
  classes7=255, classes8=255, classes9=255, classes10=255, classes11=255, classes12=255,
  classes13=255, classes14=255, classes15=255, classes16=255;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DELETE FROM aotv4_tmpl; INSERT INTO aotv4_tmpl SELECT * FROM spells_new WHERE id = 43342;
UPDATE aotv4_tmpl SET
  id = 43382, name = 'Shielded', descnum = 43382, spellgroup = 43382, rank = 1,
  mana = 0, buffduration = 6000, buffdurationformula = 10,
  numhits = 0, numhitstype = 0,
  targettype = 6, goodEffect = 1, new_icon = 158, spell_category = -99,
  classes1=255, classes2=255, classes3=255, classes4=255, classes5=255, classes6=255,
  classes7=255, classes8=255, classes9=255, classes10=255, classes11=255, classes12=255,
  classes13=255, classes14=255, classes15=255, classes16=255;
INSERT INTO spells_new SELECT * FROM aotv4_tmpl;

DROP TEMPORARY TABLE IF EXISTS aotv4_tmpl;

INSERT INTO db_str (id, type, value) VALUES
 (43380, 6, 'Allies are absorbing part of the melee damage you take. Stay close to them or the shield will break.'),
 (43381, 6, 'Allies are absorbing part of the melee damage you take. Stay close to them or the shield will break.'),
 (43382, 6, 'Allies are absorbing part of the melee damage you take. Stay close to them or the shield will break.');
