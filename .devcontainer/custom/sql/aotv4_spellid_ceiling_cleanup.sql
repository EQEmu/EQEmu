-- AoTv4 -- remove the spell-id ceiling diagnostic (see SPELL_RANKS.md for the result).
-- ⚠️ 46000 is scribed on Ashrem in book slot 0 and is INVISIBLE to the client -- it holds a slot
-- the player cannot see or clear, which is exactly the failure mode the test was proving.
-- ⚠️⚠️ THE CHARACTER MUST BE AT CHARACTER SELECT. A live zone holds the spellbook in memory and
-- saves over direct edits (CLAUDE.md section 7).
-- ⚠️ spells_new is SHARED MEMORY: world down, ./shared_memory, restart.
DELETE FROM character_spells WHERE spell_id IN (44000, 46000);
DELETE FROM spells_new       WHERE id       IN (44000, 46000);
SELECT (SELECT COUNT(*) FROM spells_new WHERE id IN (44000,46000))       AS test_spells_left,
       (SELECT COUNT(*) FROM character_spells WHERE spell_id IN (44000,46000)) AS scribed_left;
