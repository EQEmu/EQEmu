-- aotv4_aa_melee_hosted.sql -- the Melee tree, HOSTED ON NATIVE AA ROWS.
-- =============================================================================================
-- Read the header of aotv4_aa_tank_hosted.sql first. Same two rules, same reasons:
--   1. A custom AA with a NEW id never reaches the client (ability > 30195, rank > 65535).
--   2. Rank ids are NOT contiguous -- follow first_rank_id then next_id. Every chain below was
--      walked, not assumed, and the activated hosts are the worst offenders: Cannibalization runs
--      146 - 5069 - 6102 - 7466 - 7691. Terminate with next_id = -1.
--
-- ⚠️ type = 4 puts these on the tab labelled "Melee" in EQUI_AAWindow.xml.
--
--   host 30  Combat Fury                 ranks 113,114,115,443,444    -> Sunder           [MARKER]
--   host 19  Healing Gift                ranks  80, 81, 82,437,438    -> Killing Rhythm   [MARKER]
--   host 21  Spell Casting Reinforcement ranks  86, 87, 88,266,10467  -> Relentless       [native SPA]
--   host 25  Spell Casting Subtlety      ranks  98, 99,100,4767,4768  -> Bloodletting     [MARKER]
--   host 16  Innate Lung Capacity        ranks  71, 72, 73,676,677    -> Executioner      [MARKER]
--   host 47  Cannibalization             ranks 146,5069,6102,7466,7691-> Sanguine Frenzy  [ACTIVATED]
--
-- That is six more native abilities consumed, sixteen across the three trees. All are disabled
-- server-wide today, so nothing is lost now, but each is gone if the native set is turned back on.
--
-- ⚠️ MARKER RANK IDS -- 113, 80, 86, 98, 71, 146 -- are the ONLY join to zone/aotv4_melee_aa.cpp and
-- nothing checks them. A wrong id reads rank 0 forever: the AA looks bought and does nothing.
--
-- =============================================================================================
-- SANGUINE FRENZY IS AN ACTIVATED AA. Three requirements, all enforced by
-- Client::ActivateAlternateAdvancementAbility:
--
--   a. It MUST have a valid spell (IsValidSpell(rank->spell)) -- 43405 from aotv4_melee_buffs.sql.
--   b. It MUST have NO aa_rank_effects. The function returns early on a rank with effects, since
--      that is how it tells a passive from an activated one. Every marker AA already satisfies this.
--   c. recast_time is in SECONDS -- 45.
--
-- ⚠️ spell_type IS A TIMER SLOT, NOT A CATEGORY. The cooldown is keyed on
-- `rank->spell_type + pTimerAAStart`, so two activated AAs sharing a spell_type SHARE a cooldown.
-- This is the only activated AA we enable, so slot 2 (inherited from Cannibalization) is safe --
-- but any future activated AA must take a different one.
--
-- The host is used because it already has upper/lower hotkey sids set, which is what lets the
-- ability be dragged to a hotbar. A passive host has -1 there and cannot be activated at all.
-- =============================================================================================

UPDATE aa_ability SET name='Sunder',          classes=65535, enabled=1, type=4 WHERE id=30;
UPDATE aa_ability SET name='Killing Rhythm',  classes=65535, enabled=1, type=4 WHERE id=19;
UPDATE aa_ability SET name='Relentless',      classes=65535, enabled=1, type=4 WHERE id=21;
UPDATE aa_ability SET name='Bloodletting',    classes=65535, enabled=1, type=4 WHERE id=25;
UPDATE aa_ability SET name='Executioner',     classes=65535, enabled=1, type=4 WHERE id=16;
UPDATE aa_ability SET name='Sanguine Frenzy', classes=65535, enabled=1, type=4 WHERE id=47;

-- Same ladder as the other two trees so the cost of a role is comparable.
UPDATE aa_ranks SET level_req=5,  cost=3 WHERE id IN (113,  80, 86,  98,  71, 146);
UPDATE aa_ranks SET level_req=15, cost=4 WHERE id IN (114,  81, 87,  99,  72, 5069);
UPDATE aa_ranks SET level_req=25, cost=5 WHERE id IN (115,  82, 88, 100,  73, 6102);
UPDATE aa_ranks SET level_req=35, cost=6 WHERE id IN (443, 437, 266, 4767, 676, 7466);
UPDATE aa_ranks SET level_req=45, cost=8 WHERE id IN (444, 438, 10467, 4768, 677, 7691);

-- Stop each chain at rank 5, or the window counts the host's remaining native ranks.
UPDATE aa_ranks SET next_id=-1 WHERE id IN (444, 438, 10467, 4768, 677, 7691);

-- Clear every effect row first. Five of the six are markers, and Sanguine Frenzy MUST have none or
-- it cannot be activated at all.
DELETE FROM aa_rank_effects WHERE rank_id IN
 (113,114,115,443,444, 80,81,82,437,438, 86,87,88,266,10467,
  98,99,100,4767,4768, 71,72,73,676,677, 146,5069,6102,7466,7691);

-- Relentless is the one NON-marker: SPA 225 GiveDoubleAttack.
-- ⚠️ 225, NOT 177 DoubleAttackChance. Only 8 of the 16 classes have a Double Attack skill cap, and
-- Client::CheckDoubleAttack returns false immediately without the skill, so SPA 177 would be dead
-- weight for half the roster. SPA 225 is documented as "Allow any class to double attack with set
-- chance" and explicitly bypasses that check, while still stacking for those who do have the skill.
-- It also unlocks OFF-HAND doubles, which otherwise need skill 150+.
INSERT INTO aa_rank_effects (rank_id,slot,effect_id,base1,base2) VALUES
 (86,1,225,3,0),(87,1,225,6,0),(88,1,225,9,0),(266,1,225,12,0),(10467,1,225,15,0);

-- Sanguine Frenzy: the activation spell and the 45 second recast, on every rank.
UPDATE aa_ranks SET spell=43405, recast_time=45 WHERE id IN (146,5069,6102,7466,7691);

-- ------------------------------------------------------------------------- names + descriptions
UPDATE db_str SET value='Sunder'          WHERE id=113 AND type=1;
UPDATE db_str SET value='Killing Rhythm'  WHERE id=80  AND type=1;
UPDATE db_str SET value='Relentless'      WHERE id=86  AND type=1;
UPDATE db_str SET value='Bloodletting'    WHERE id=98  AND type=1;
UPDATE db_str SET value='Executioner'     WHERE id=71  AND type=1;
UPDATE db_str SET value='Sanguine Frenzy' WHERE id=146 AND type=1;

UPDATE db_str SET value='Armour gives, if you keep hitting it. Each blow you land wears down your target guard against you, making your attacks less likely to be turned aside. From the third rank even a deflected swing wears it down. At the fifth rank a full set of marks guarantees your next blow lands. Wearing down a new target starts over.' WHERE id=113 AND type=4;

UPDATE db_str SET value='Violence rewards a steady hand. Every melee blow you land strikes for 2, 4, 4, 6 and 6 additional points. From the third rank, staying on one target adds a further point for each blow in a row, and the fifth rank doubles how far that can climb.' WHERE id=80 AND type=4;

UPDATE db_str SET value='You do not stop. Increases your chance to attack twice by 3, 6, 9, 12 and 15 percent, whether or not your class was ever taught how. At the fifth rank a double strike can carry into a third.' WHERE id=86 AND type=4;

UPDATE db_str SET value='Your blows leave wounds that keep working after you have moved on. Melee hits cause bleeding. The third rank makes it last longer and the fifth rank doubles its severity. Bleeding ignores armour entirely.' WHERE id=98 AND type=4;

UPDATE db_str SET value='You finish what you start. Blows against a target below 25 percent health strike for 3, 5, 5, 8 and 8 additional points, and the third rank widens that to 35 percent. At the fifth rank damage spent on an already dying foe carries to another enemy fighting you.' WHERE id=71 AND type=4;

-- ⚠️ The window, the conversion rate and the cap are all enforced in zone/aotv4_melee_aa.cpp, not by
-- anything in the DB. Keep this text, FRENZY_WINDOW_MS, FRENZY_PCT and FRENZY_CAP_PCT in step.
UPDATE db_str SET value='For 4 seconds, the damage your weapons deal is returned to you as health, three times over. The healing is capped at 8, 11, 14, 17 and 20 percent of your maximum health each time you use it, so no weapon is too fine and none too crude. Reusable every 45 seconds.' WHERE id=146 AND type=4;
