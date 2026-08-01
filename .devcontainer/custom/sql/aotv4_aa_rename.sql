-- =============================================================================================
-- AoTv4 -- rename the 50 reused native AAs                                       2026-07-31
--
-- The 50 Luclin/PoP passives we re-enabled keep their mechanics, ranks, costs and descriptions
-- exactly as they are; only the NAME the client shows changes, so the AA window reads as this
-- server's own content rather than as stock EQ abilities.
--
-- ⚠️ The 40 hand-built AoTv4 trees are NOT touched -- they already carry their own names.
--
-- ⚠️⚠️ AA NAMES ARE RESOLVED CLIENT SIDE. `db_str` type 1 is the title; the client reads it out of
-- its OWN dbstr_us.txt, NOT from the server at runtime. So this change is invisible in game until
-- `./export_client_files` is run and the regenerated dbstr_us.txt is installed in the EQ folder.
-- Until then the client keeps showing the old stock names, with no error anywhere (section 10).
--
-- ⚠️ Every rank of an ability shares ONE title_sid here -- verified: 50 abilities, 379 ranks, 50
-- distinct title_sids, none shared between abilities. So one UPDATE per ability renames the whole
-- chain and cannot bleed into another AA. Do not assume that holds for arbitrary AAs: `title_sid`
-- and `desc_sid` are independent and neither is guaranteed to equal first_rank_id.
-- ⚠️ Type 1 is the TITLE and type 4 the DESCRIPTION. Writing a name to the desc_sid gives an AA with
-- the right name and the wrong description, with no error -- that has bitten this project before.
-- ⚠️ Re-runnable.
-- =============================================================================================
-- Acrobatics
UPDATE db_str SET value = 'Tumbler''s Instinct' WHERE id = 283 AND type = 1;
-- Alchemy Mastery
UPDATE db_str SET value = 'Alembic' WHERE id = 150 AND type = 1;
-- Ambidexterity
UPDATE db_str SET value = 'Offhand Grace' WHERE id = 198 AND type = 1;
-- Bestial Frenzy
UPDATE db_str SET value = 'Feral Cadence' WHERE id = 551 AND type = 1;
-- Body and Mind Rejuvenation
UPDATE db_str SET value = 'Quiet Recovery' WHERE id = 278 AND type = 1;
-- Chaotic Stab
UPDATE db_str SET value = 'Wild Thrust' WHERE id = 287 AND type = 1;
-- Combat Fury
UPDATE db_str SET value = 'Killing Edge' WHERE id = 113 AND type = 1;
-- Dead Aim
UPDATE db_str SET value = 'True Cast' WHERE id = 1140 AND type = 1;
-- Embrace of the Keepers
UPDATE db_str SET value = 'Open Vessel' WHERE id = 1368 AND type = 1;
-- Fear Resistance
UPDATE db_str SET value = 'Steady Nerve' WHERE id = 116 AND type = 1;
-- Fearless
UPDATE db_str SET value = 'Nothing Left to Fear' WHERE id = 195 AND type = 1;
-- Ferocity
UPDATE db_str SET value = 'Redoubled' WHERE id = 564 AND type = 1;
-- Foraging
UPDATE db_str SET value = 'Forager''s Eye' WHERE id = 7062 AND type = 1;
-- Fury of Magic
UPDATE db_str SET value = 'Ruinous Focus' WHERE id = 637 AND type = 1;
-- Gift of the Keepers
UPDATE db_str SET value = 'Tempered Frame' WHERE id = 1366 AND type = 1;
-- Harmonious Attack
UPDATE db_str SET value = 'Song of Blades' WHERE id = 556 AND type = 1;
-- Hunter's Attack Power
UPDATE db_str SET value = 'Hunter''s Weight' WHERE id = 6546 AND type = 1;
-- Ingenuity
UPDATE db_str SET value = 'Clever Working' WHERE id = 625 AND type = 1;
-- Innate Charisma
UPDATE db_str SET value = 'Command Presence' WHERE id = 32 AND type = 1;
-- Innate Enlightenment
UPDATE db_str SET value = 'Tranquil Mind' WHERE id = 955 AND type = 1;
-- Innate Intelligence
UPDATE db_str SET value = 'Sharpened Wit' WHERE id = 22 AND type = 1;
-- Innate Metabolism
UPDATE db_str SET value = 'Lean Rations' WHERE id = 68 AND type = 1;
-- Innate Regeneration
UPDATE db_str SET value = 'Slow Knitting' WHERE id = 65 AND type = 1;
-- Innate Strength
UPDATE db_str SET value = 'Brawn' WHERE id = 2 AND type = 1;
-- Jewel Craft Mastery
UPDATE db_str SET value = 'Gemcutter' WHERE id = 4675 AND type = 1;
-- Knight's Advantage
UPDATE db_str SET value = 'Oathblade' WHERE id = 561 AND type = 1;
-- Mastery of the Past
UPDATE db_str SET value = 'Word Perfect' WHERE id = 446 AND type = 1;
-- Mental Clarity
UPDATE db_str SET value = 'Wellspring' WHERE id = 658 AND type = 1;
-- New Tanaan Crafting Mastery
UPDATE db_str SET value = 'Tanaan Schooling' WHERE id = 412 AND type = 1;
-- Nimble Evasion
UPDATE db_str SET value = 'Slip Away' WHERE id = 606 AND type = 1;
-- Physical Enhancement
UPDATE db_str SET value = 'Hardened Frame' WHERE id = 279 AND type = 1;
-- Planar Durability
UPDATE db_str SET value = 'Planeforged' WHERE id = 423 AND type = 1;
-- Poison Mastery
UPDATE db_str SET value = 'Steady Hands' WHERE id = 244 AND type = 1;
-- Power of the Keepers
UPDATE db_str SET value = 'Malefic Reach' WHERE id = 1369 AND type = 1;
-- Punishing Blade
UPDATE db_str SET value = 'Cleaving Arc' WHERE id = 599 AND type = 1;
-- Quick Buff
UPDATE db_str SET value = 'Ready Blessing' WHERE id = 147 AND type = 1;
-- Quick Evacuation
UPDATE db_str SET value = 'Quick Exit' WHERE id = 137 AND type = 1;
-- Quick Summoning
UPDATE db_str SET value = 'Ready Conjuring' WHERE id = 655 AND type = 1;
-- Sanctity of the Keepers
UPDATE db_str SET value = 'Warded Soul' WHERE id = 1370 AND type = 1;
-- Speed of the Knight
UPDATE db_str SET value = 'Follow Through' WHERE id = 602 AND type = 1;
-- Spell Casting Deftness
UPDATE db_str SET value = 'Deft Incantation' WHERE id = 104 AND type = 1;
-- Spell Casting Expertise
UPDATE db_str SET value = 'Flawless Recital' WHERE id = 101 AND type = 1;
-- Spell Casting Fury
UPDATE db_str SET value = 'Ruinous Cadence' WHERE id = 92 AND type = 1;
-- Spell Casting Mastery
UPDATE db_str SET value = 'Frugal Casting' WHERE id = 83 AND type = 1;
-- Stalwart Endurance
UPDATE db_str SET value = 'Refuse to Fall' WHERE id = 652 AND type = 1;
-- Tactical Mastery
UPDATE db_str SET value = 'Read the Fight' WHERE id = 631 AND type = 1;
-- Theft of Life
UPDATE db_str SET value = 'Stolen Vitality' WHERE id = 634 AND type = 1;
-- Thief's Intuition
UPDATE db_str SET value = 'Sixth Sense' WHERE id = 1641 AND type = 1;
-- Throwing Mastery
UPDATE db_str SET value = 'Heavy Throw' WHERE id = 1131 AND type = 1;
-- Valor of the Keepers
UPDATE db_str SET value = 'Bulwark Within' WHERE id = 1367 AND type = 1;

-- ---------------------------------------------------------------- verify
SELECT 'renamed titles' AS what, COUNT(*) n FROM db_str d
JOIN aa_ranks r ON r.title_sid = d.id AND d.type = 1
JOIN aa_ability a ON a.first_rank_id = r.id
WHERE a.enabled = 1 AND d.value NOT LIKE '%Innate%' AND d.value NOT LIKE '%Mastery%'
  AND d.value NOT LIKE '%Keepers%' AND d.value NOT LIKE '%Spell Casting%';

-- ---------------------------------------------------------------- title_sid collision fix
-- ⚠️⚠️ COMBAT FURY AND THE CUSTOM AA "SUNDER" SHARED title_sid 113, so renaming one renamed BOTH.
-- This was NOT created by the rename -- it was already true, and enabling Combat Fury simply made it
-- visible: two abilities in the window displaying the same name. The rename turned a silent
-- duplicate into an obvious one.
--
-- ⚠️ The uniqueness check that missed it only compared the 50 natives against EACH OTHER. A native
-- can share a title_sid with one of the 40 hand-built AoTv4 trees, because those are hosted on
-- native rows and inherit whatever title_sid the host had. Any future rename must check against
-- every ENABLED ability, not just the set being renamed.
--
-- Fix: give Combat Fury its own string id so the two names are independent.
-- ⚠️ 990113 is chosen from an empty band (nothing exists in db_str between 990000 and 990100) and is
-- far below the 1,458,120,305 maximum already in use, so it is safely inside the range the exporter
-- and client already handle.
UPDATE db_str SET value = 'Sunder' WHERE id = 113 AND type = 1;

INSERT INTO db_str (id, type, value) VALUES (990113, 1, 'Killing Edge')
  ON DUPLICATE KEY UPDATE value = VALUES(value);

UPDATE aa_ranks r
JOIN aa_ability a ON a.id = 309
SET r.title_sid = 990113
WHERE r.id IN (
  SELECT rank_id FROM (
    WITH RECURSIVE chain AS (
      SELECT r2.id AS rank_id, r2.next_id, 1 AS pos
      FROM aa_ability a2 JOIN aa_ranks r2 ON r2.id = a2.first_rank_id WHERE a2.id = 309
      UNION ALL
      SELECT r3.id, r3.next_id, c.pos + 1 FROM chain c JOIN aa_ranks r3 ON r3.id = c.next_id WHERE c.pos < 40
    ) SELECT rank_id FROM chain
  ) x
);

-- verify: no two ENABLED abilities may display the same name
SELECT 'duplicate display names (must be 0)' AS what, COUNT(*) n FROM (
  SELECT TRIM(t.value) v FROM aa_ability a JOIN aa_ranks r ON r.id = a.first_rank_id
  JOIN db_str t ON t.id = r.title_sid AND t.type = 1
  WHERE a.enabled = 1 GROUP BY TRIM(t.value) HAVING COUNT(*) > 1
) d;
