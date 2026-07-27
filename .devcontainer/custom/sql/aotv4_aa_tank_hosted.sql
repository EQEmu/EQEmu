-- aotv4_aa_tank_hosted.sql -- the Tank tree, HOSTED ON NATIVE AA ROWS.
-- Rewritten 2026-07-26 against the live DB. The previous version wrote to ranks 110/111 (which
-- belong to a DIFFERENT ability) and still created Bulwark, which was removed; re-running it would
-- have re-broken both. This file now reproduces exactly what is live, and is safe to re-run.
-- =============================================================================================
-- WHY THIS EXISTS. Custom AAs created with NEW ids are silently discarded by the client. The
-- server resolves them, passes every gate in CanUseAlternateAdvancementRank and sends the packet
-- (proven with logging) -- the client just never renders them. Ability ids above the native maximum
-- (30195) and rank ids above 65535 both fail this way, with no error anywhere.
--
-- The fix is to stop minting ids: TAKE OVER an existing native AA. Keep its ability id, rank ids,
-- title_sid and rank chain -- everything the client already accepts -- and replace only the payload
-- (aa_rank_effects) and the strings (db_str).
--
-- WARNING, THE TWO THINGS THAT COST REAL TIME:
--
--  1. RANK IDS ARE NOT CONTIGUOUS. aa_ranks has no aa_id column: the only way to know an ability's
--     ranks is first_rank_id, then follow next_id. Shield Oath's host chain is
--     107 - 108 - 109 - 7541 - 7542, NOT 107..111. Assuming a block wrote onto another ability.
--     Terminate the chain with next_id = -1 or the window counts the host's leftover ranks
--     and shows something like "0/15".
--
--  2. db_str changes need `./export_client_files` and dbstr_us.txt copied to the EQ root, or the
--     names stay whatever the host was called. title_sid = -1 renders NO ROW AT ALL; it does not
--     fall back to aa_ability.name.
--
-- The tab an AA appears on is aa_ability.type, relabelled client-side in EQUI_AAWindow.xml:
--     1 = Tank (this file)   2 = Healer   3 = Ranged   4 = Melee
--
--   host 28  Natural Durability   ranks 107,108,109,7541,7542  -> Shield Oath     [native SPA]
--   host  2  Innate Stamina       ranks   7,  8,  9,  10,  11  -> Stonestride     [native SPA]
--   host  3  Innate Agility       ranks  12, 13, 14,  15,  16  -> Unyielding      [native SPA]
--   host  4  Innate Dexterity     ranks  17, 18, 19,  20,  21  -> Bloodied Bash   [MARKER, Lua]
--   host  6  Innate Wisdom        ranks  27, 28, 29,  30,  31  -> Aegis Reflex    [MARKER, C++]
--
-- Host 5 (Innate Intelligence) was consumed by Bulwark and is RESTORED at the bottom of this file.
-- =============================================================================================

-- ---------------------------------------------------------------------------- the five abilities
-- type = 1 puts them on the tab labelled "Tank". classes = 65535: these are roles, not class locks.
UPDATE aa_ability SET name='Shield Oath',   classes=65535, enabled=1, type=1 WHERE id=28;
UPDATE aa_ability SET name='Stonestride',   classes=65535, enabled=1, type=1 WHERE id=2;
UPDATE aa_ability SET name='Unyielding',    classes=65535, enabled=1, type=1 WHERE id=3;
UPDATE aa_ability SET name='Bloodied Bash', classes=65535, enabled=1, type=1 WHERE id=4;
UPDATE aa_ability SET name='Aegis Reflex',  classes=65535, enabled=1, type=1 WHERE id=6;

-- ---------------------------------------------------------------------------- five buyable ranks
-- Levels 5/15/25/35/45, cost 3/4/5/6/8. Note rank 4 and 5 of Shield Oath are 7541/7542, not 110/111.
UPDATE aa_ranks SET level_req=5,  cost=3 WHERE id IN (107,   7, 12, 17, 27);
UPDATE aa_ranks SET level_req=15, cost=4 WHERE id IN (108,   8, 13, 18, 28);
UPDATE aa_ranks SET level_req=25, cost=5 WHERE id IN (109,   9, 14, 19, 29);
UPDATE aa_ranks SET level_req=35, cost=6 WHERE id IN (7541, 10, 15, 20, 30);
UPDATE aa_ranks SET level_req=45, cost=8 WHERE id IN (7542, 11, 16, 21, 31);

-- Stop each chain at rank 5 so the window reads "0/5" and not the host's full native rank count.
UPDATE aa_ranks SET next_id=-1 WHERE id IN (7542, 11, 16, 21, 31);

-- ---------------------------------------------------------------------------------- the payloads
-- ⚠️ CLEAR THE INHERITED PREREQUISITES TOO. aa_rank_prereqs is a SEPARATE table from
-- aa_rank_effects, and a hosted AA inherits whatever its host required -- so the ability shows in
-- the window but refuses to train, with no explanation. Missing this made eight AAs untrainable
-- until 2026-07-27.
DELETE FROM aa_rank_prereqs WHERE rank_id IN
 (107,108,109,7541,7542, 7,8,9,10,11, 12,13,14,15,16, 17,18,19,20,21, 27,28,29,30,31);

DELETE FROM aa_rank_effects WHERE rank_id IN
 (107,108,109,7541,7542, 7,8,9,10,11, 12,13,14,15,16, 17,18,19,20,21, 27,28,29,30,31);

-- Shield Oath : SPA 192 Hate + SPA 185 DamageModifier
INSERT INTO aa_rank_effects (rank_id,slot,effect_id,base1,base2) VALUES
 (107, 1,192, 8,0),(107, 2,185, 4,0),
 (108, 1,192,12,0),(108, 2,185, 7,0),
 (109, 1,192,16,0),(109, 2,185,10,0),
 (7541,1,192,20,0),(7541,2,185,13,0),
 (7542,1,192,25,0),(7542,2,185,15,0),
-- Unyielding  : SPA 172 evasion, feeds the defence roll directly
 (12,1,172,2,0),(13,1,172,4,0),(14,1,172,6,0),(15,1,172,8,0),(16,1,172,11,0);

-- ⚠️⚠️ STONESTRIDE HAS NO EFFECT ROWS -- IT IS A MARKER. Do NOT restore the SPA 162 rows that used
-- to sit here:
--     (7,1,162,100,2),(8,1,162,100,4),(9,1,162,100,7),(10,1,162,100,10),(11,1,162,100,14)
-- MitigateMeleeDamage with base 100 + limit N really is the documented way to get a flat per-hit
-- reduction, but only from a SPELL. On an AA it is inert twice over: Mob::ApplyAABonuses has no
-- case for it, so aabonuses.MitigateMeleeRune is never filled; and the consumer (attack.cpp:3880)
-- reads spellbonuses only, paying out of buffs[slot].melee_rune -- a buff slot an AA does not have.
-- It was live and doing nothing from the day it was written until 2026-07-27, which is invisible in
-- game because a passive that quietly reduces damage looks exactly like one that does not.
-- Contrast Weathered (SPA 161): ApplyAABonuses DOES handle that one (bonuses.cpp:1754) and the
-- consumer sums aabonuses (attack.cpp:3980), so it works. Do not assume the two behave alike.
-- The reduction now lives in Mob::AoTv4Stonestride, read off rank id 7.

-- Hosts 2, 4 and 6 are MARKERS: no effect rows at all, read at runtime by code off the FIRST rank id.
--   Stonestride    rank  7 -> Mob::AoTv4Stonestride, called from Mob::MeleeMitigation (GetAA(7))
--   Bloodied Bash  rank 17 -> quests/lua_modules/aotv4_aa_tank.lua  (client:GetAA(17))
--   Aegis Reflex   rank 27 -> Mob::MeleeMitigation + Mob::HealDamage (GetAA(27))
-- That rank id is the only join between SQL and code and nothing checks it: a wrong id reads 0
-- forever, so the AA looks bought and quietly does nothing.

-- ------------------------------------------------------------------------- names + descriptions
UPDATE db_str SET value='Shield Oath'   WHERE id=107 AND type=1;
UPDATE db_str SET value='Stonestride'   WHERE id=7   AND type=1;
UPDATE db_str SET value='Unyielding'    WHERE id=12  AND type=1;
UPDATE db_str SET value='Bloodied Bash' WHERE id=17  AND type=1;
UPDATE db_str SET value='Aegis Reflex'  WHERE id=27  AND type=1;

UPDATE db_str SET value='Your defensive stance sharpens your blows and your presence. Increases melee damage by 4, 7, 10, 13 and 15 percent, and hate generated by 8, 12, 16, 20 and 25 percent.' WHERE id=107 AND type=4;
UPDATE db_str SET value='Your footing turns aside the worst of a blow. Reduces the damage of each melee hit against you by 2, 4, 7, 10 and 14 points.' WHERE id=7 AND type=4;
UPDATE db_str SET value='You are harder to land a blow upon. Increases your chance to avoid melee attacks by 2, 4, 6, 8 and 11.' WHERE id=12 AND type=4;
UPDATE db_str SET value='You drink from the wounds your shield opens. Bash and Slam heal you for 25, 40, 55, 70 and 85 percent of the damage they deal.' WHERE id=17 AND type=4;
UPDATE db_str SET value='Every blow your armour turns aside is remembered. Deflections build a reserve, and when a blow finally lands it strengthens the next heal you receive by up to 10, 18, 28, 40 and 60 percent.' WHERE id=27 AND type=4;

-- =============================================================================================
-- RESTORE host 5, which Bulwark consumed before it was cut.
-- Bulwark was dropped because /shield already covers shared damage, so its host goes back to being
-- Innate Intelligence. Left disabled (enabled=0) like the rest of the native set, but no longer
-- carrying Bulwark's levels, costs, effects or strings. Pristine values are from
-- utils/sql/peq_aa_tables_post_rework.sql; the db_str text is from the 2026-07-24 DB dump
-- (peq_recovered.sql.gz), which predates this work.
UPDATE aa_ability SET name='Innate Intelligence', classes=65535, enabled=0, type=1 WHERE id=5;
UPDATE aa_ranks SET level_req=51, cost=1, expansion=3 WHERE id IN (22,23,24,25,26);
UPDATE aa_ranks SET next_id=23  WHERE id=22;
UPDATE aa_ranks SET next_id=24  WHERE id=23;
UPDATE aa_ranks SET next_id=25  WHERE id=24;
UPDATE aa_ranks SET next_id=26  WHERE id=25;
UPDATE aa_ranks SET next_id=332 WHERE id=26;   -- the native line continues past rank 5
DELETE FROM aa_rank_effects WHERE rank_id IN (22,23,24,25,26);
INSERT INTO aa_rank_effects (rank_id,slot,effect_id,base1,base2) VALUES
 (22,1,8,2,0),(23,1,8,4,0),(24,1,8,6,0),(25,1,8,8,0),(26,1,8,10,0);
UPDATE db_str SET value='Innate Intelligence' WHERE id=22 AND type=1;
UPDATE db_str SET value='This ability raises your base Intelligence by 2 points for each ability level.' WHERE id=22 AND type=4;

-- ---------------------------------------------------------------------------------------------
-- RESTORE ranks 110-111, collateral damage from the contiguous-rank assumption.
-- Shield Oath's 4th and 5th ranks were written to 110/111 on the belief that its chain was 107..111.
-- Those ranks belong to ability 29 Natural Healing (chain 110-111-112, title_sid 110). A later pass
-- "restored" them to level 59/61 cost 5 by inference, which is also wrong -- pristine is level 55
-- for all three, costs 2/4/6. Values from utils/sql/peq_aa_tables_post_rework.sql:
--   (110,-1,-1,110,110,2,55,-1,0,0,3,-1,111)
--   (111,-1,-1,110,110,4,55,-1,0,0,3,110,112)
UPDATE aa_ranks SET cost=2, level_req=55, title_sid=110, desc_sid=110, expansion=3,
                    prev_id=-1,  next_id=111 WHERE id=110;
UPDATE aa_ranks SET cost=4, level_req=55, title_sid=110, desc_sid=110, expansion=3,
                    prev_id=110, next_id=112 WHERE id=111;
-- rank 112 was never touched and already matches pristine.

-- Retire the unreachable custom-id attempt (new ids the client discards; see the header).
DELETE FROM aa_rank_effects WHERE rank_id IN (SELECT id FROM (SELECT id FROM aa_ranks WHERE id BETWEEN 50000 AND 50029) t);
DELETE FROM aa_ranks        WHERE id BETWEEN 50000 AND 50029;
DELETE FROM aa_ability      WHERE id BETWEEN 30500 AND 30505;

-- ...and its orphaned strings. Verified before deleting: nothing in aa_ranks references sids
-- 40000-40005, and the 2026-07-24 dump has no rows there, so they are ours and nothing else's.
-- ⚠️ SCOPE THIS TIGHTLY. db_str 50000-50029 looks like part of the same attempt and is NOT --
-- those are NATIVE type-6 item descriptions (Book of Bad Poetry, the amnesia potions, mercenary
-- contracts). Only ids 40000-40005 types 1 and 4 came from us.
DELETE FROM db_str WHERE id BETWEEN 40000 AND 40005 AND type IN (1,4);

-- AAs are NOT in shared memory -- a zone restart applies this, no ./shared_memory rebuild.
-- db_str changes additionally need ./export_client_files + dbstr_us.txt copied to the EQ root.
