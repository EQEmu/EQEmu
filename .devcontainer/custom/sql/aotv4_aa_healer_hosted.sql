-- aotv4_aa_healer_hosted.sql -- the Healer tree, HOSTED ON NATIVE AA ROWS.
-- =============================================================================================
-- Read the header of aotv4_aa_tank_hosted.sql first. The same two rules apply and they are the two
-- things that have cost real time here:
--
--   1. A custom AA with a NEW id never reaches the client. Ability ids above 30195 and rank ids
--      above 65535 are silently discarded, so we take over native rows instead.
--   2. Rank ids are NOT contiguous. aa_ranks has no aa_id column; follow first_rank_id then next_id.
--      Every chain below was walked, not assumed. Terminate with next_id = -1 or the window shows
--      the host's full native rank count (e.g. "0/12").
--
-- ⚠️ type = 2 puts these on the tab labelled "Healer" in EQUI_AAWindow.xml. The tab is chosen by
-- aa_ability.type, NOT by category. 1 Tank / 2 Healer / 3 Ranged / 4 Melee.
--
--   host  8  Innate Fire Protection      ranks 37,38,39,40,41  -> Overflowing Grace
--   host  9  Innate Cold Protection      ranks 42,43,44,45,46  -> Triage Instinct
--   host 10  Innate Magic Protection     ranks 47,48,49,50,51  -> Mender's Echo
--   host 11  Innate Poison Protection    ranks 52,53,54,55,56  -> Cleansing Renewal
--   host 12  Innate Disease Protection   ranks 57,58,59,60,61  -> Borrowed Breath
--
-- That is five more native abilities consumed, ten in total with the Tank tree. They are all
-- currently disabled server-wide, so nothing is lost today, but each one is gone if the native set
-- is ever turned back on. Restore instructions are in TURNOVER.md.
--
-- ⚠️ ALL FIVE ARE MARKER AAs: no aa_rank_effects at all. Every effect is conditional on something no
-- SPA can express (the target's health, an overheal, a successful cure), so the behaviour lives in
-- C++ and reads the rank with GetAA(<first rank id>). Those five ids -- 37, 42, 47, 52, 57 -- are
-- the ONLY join between this file and the code, and nothing checks them. A wrong id reads rank 0
-- forever, which looks exactly like a bought AA that does nothing.
--
--   mysql -h127.0.0.1 -upeq -ppeqpass peq < .devcontainer/custom/sql/aotv4_aa_healer_hosted.sql
--
-- AAs are NOT in shared memory -- a zone restart applies this. db_str changes additionally need
-- ./export_client_files and dbstr_us.txt copied to the EQ root, or the names stay as the host's.
-- =============================================================================================

UPDATE aa_ability SET name='Overflowing Grace', classes=65535, enabled=1, type=2 WHERE id=8;
UPDATE aa_ability SET name='Triage Instinct',   classes=65535, enabled=1, type=2 WHERE id=9;
UPDATE aa_ability SET name='Mender''s Echo',      classes=65535, enabled=1, type=2 WHERE id=10;
UPDATE aa_ability SET name='Cleansing Renewal', classes=65535, enabled=1, type=2 WHERE id=11;
UPDATE aa_ability SET name='Borrowed Breath',   classes=65535, enabled=1, type=2 WHERE id=12;

-- Five buyable ranks each, same ladder as the Tank tree so the trees stay comparable in cost.
UPDATE aa_ranks SET level_req=5,  cost=3 WHERE id IN (37,42,47,52,57);
UPDATE aa_ranks SET level_req=15, cost=4 WHERE id IN (38,43,48,53,58);
UPDATE aa_ranks SET level_req=25, cost=5 WHERE id IN (39,44,49,54,59);
UPDATE aa_ranks SET level_req=35, cost=6 WHERE id IN (40,45,50,55,60);
UPDATE aa_ranks SET level_req=45, cost=8 WHERE id IN (41,46,51,56,61);

-- Stop each chain at rank 5. Without this they run on into the host's later ranks (362, 372, 382,
-- 392, 402 respectively) and the window counts those too.
UPDATE aa_ranks SET next_id=-1 WHERE id IN (41,46,51,56,61);

-- No effect rows: every one of these is a marker read from code.
-- ⚠️ CLEAR THE INHERITED PREREQUISITES TOO. aa_rank_prereqs is a SEPARATE table from
-- aa_rank_effects, and a hosted AA inherits whatever its host required -- so the ability shows in
-- the window but refuses to train, with no explanation. Missing this made eight AAs untrainable
-- until 2026-07-27.
DELETE FROM aa_rank_prereqs WHERE rank_id IN
 (37,38,39,40,41, 42,43,44,45,46, 47,48,49,50,51, 52,53,54,55,56, 57,58,59,60,61);

DELETE FROM aa_rank_effects WHERE rank_id IN
 (37,38,39,40,41, 42,43,44,45,46, 47,48,49,50,51, 52,53,54,55,56, 57,58,59,60,61);

-- ------------------------------------------------------------------------- names + descriptions
UPDATE db_str SET value='Overflowing Grace' WHERE id=37 AND type=1;
UPDATE db_str SET value='Triage Instinct'   WHERE id=42 AND type=1;
UPDATE db_str SET value='Mender''s Echo'      WHERE id=47 AND type=1;
UPDATE db_str SET value='Cleansing Renewal' WHERE id=52 AND type=1;
UPDATE db_str SET value='Borrowed Breath'   WHERE id=57 AND type=1;

UPDATE db_str SET value='Healing that would have been wasted is not wasted. Healing past a target maximum health wraps them in a shield that absorbs melee damage, converting 10, 18, 18, 25 and 25 percent of the excess. The third rank makes the shield last longer, and at the fifth rank a shield that is spent entirely leaves behind a brief renewal.' WHERE id=37 AND type=4;

UPDATE db_str SET value='You heal hardest where it is needed most. Heals cast on a target below 35 percent health are increased by 6, 10, 10, 15 and 15 percent. From the third rank those heals are also far more likely to critical, and at the fifth rank a critical heal on a dying target returns a portion of its mana.' WHERE id=42 AND type=4;

UPDATE db_str SET value='Your healing carries. A direct heal echoes onto the most wounded ally near your target for 8, 12, 12, 18 and 18 percent of the amount healed. The third rank reaches a second ally at reduced strength, and at the fifth rank the echo itself can critical.' WHERE id=47 AND type=4;

UPDATE db_str SET value='Curing is itself a kindness. Removing a poison, disease or curse also heals the target. The third rank leaves a short renewal behind, and at the fifth rank your next heal on that target lands harder.' WHERE id=52 AND type=4;

-- ⚠️ The two minute limit is enforced in code (Mob::AoTv4TryBorrowedBreath), not by anything in the
-- DB, and it is spent when a death is actually averted rather than when the save is armed. Keep this
-- text and BREATH_SAVE_COOLDOWN_MS in step.
UPDATE db_str SET value='You buy the dying a moment more. Healing a target below 15 percent health steadies them, reducing each melee hit against them by 3, 5, 6, 8 and 10 points. The third rank makes it last longer. At the fifth rank a blow that would have killed them instead leaves them standing, restored to a sliver of life and still able to be healed. You can avert death this way only once every 2 minutes.' WHERE id=57 AND type=4;
