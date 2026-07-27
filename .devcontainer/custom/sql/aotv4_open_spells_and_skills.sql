-- aotv4_open_spells_and_skills.sql -- put back the all-classes access that the Bard pivot lost.
-- =============================================================================================
-- When every character was forced to Bard, "all classes can use everything" was true for free: the
-- reward pool only ever had to be legal for Bard. Opening the server to all 16 classes (CLAUDE.md
-- section 14) removed that, and the class gating on the STOCK spell set was never reopened -- so of
-- 2,707 pool spells, ZERO were castable by all classes. Only the custom 43xxx set was opened, which
-- is why the problem was invisible while the custom set WAS the pool.
--
-- Three separate gates all had to be reopened. Any one of them left shut makes the reward useless:
--
--   1. spells_new.classes1..16   -- can this class SCRIBE and CAST it at all
--   2. skill_caps (singing)      -- can this class have Singing and the four instrument skills
--   3. skill_caps (combat)       -- can this class have the activated combat specials
--
--   mysql -h127.0.0.1 -upeq -ppeqpass peq < .devcontainer/custom/sql/aotv4_open_spells_and_skills.sql
--
-- ⚠️ spells_new AND skill_caps are BOTH read into shared memory. Stop world + zones, run
-- ./shared_memory, restart. The client also needs a fresh SkillCaps.txt and spells_us.txt from
-- ./export_client_files, or its skill window and spellbook will disagree with the server.
-- =============================================================================================

-- 1. ------------------------------------------------------------------- spell class access
-- classes8 is this server's "minimum level to learn" (set to LEAST of the real class columns when
-- the pool was built), so copying it into every class column opens the spell to everyone AT THE
-- SAME LEVEL it was already offered at. Nothing becomes available earlier than it already was.
--
-- ⚠️ This is what makes BARD SONGS castable by other classes. A song carries a real level only in
-- classes8 and 255 everywhere else, so before this every song in the reward pool was unlearnable by
-- 15 of the 16 classes -- it would be offered, picked, and then simply not work.
--
-- Scoped to the reward pool (id < 10000 with a sane learn level). Spells outside it are untouched.
UPDATE spells_new
   SET classes1  = classes8, classes2  = classes8, classes3  = classes8, classes4  = classes8,
       classes5  = classes8, classes6  = classes8, classes7  = classes8, classes9  = classes8,
       classes10 = classes8, classes11 = classes8, classes12 = classes8, classes13 = classes8,
       classes14 = classes8, classes15 = classes8, classes16 = classes8
 WHERE id < 10000 AND classes8 BETWEEN 1 AND 100;

-- 2. ------------------------------------------------------------------- singing + instruments
-- Skills 12 Singing, 41 Percussion, 49 Stringed, 54 Wind, 70 Brass. Bard is the only class with any
-- cap for these, so nobody else could raise them -- and a song sung with no skill is a song that
-- fails. Copy Bard's whole curve to every other class, level for level.
--
-- ⚠️ Copies from class 8 rather than inventing numbers: the curve is per level (96 rows per skill)
-- and has to line up with what the client's SkillCaps.txt will say.
INSERT INTO skill_caps (skill_id, class_id, level, cap, class_)
SELECT b.skill_id, c.class_id, b.level, b.cap, 0
  FROM skill_caps b
  JOIN (SELECT 1 AS class_id UNION SELECT 2 UNION SELECT 3 UNION SELECT 4 UNION SELECT 5
        UNION SELECT 6 UNION SELECT 7 UNION SELECT 9 UNION SELECT 10 UNION SELECT 11
        UNION SELECT 12 UNION SELECT 13 UNION SELECT 14 UNION SELECT 15 UNION SELECT 16) c
 WHERE b.class_id = 8 AND b.skill_id IN (12, 41, 49, 54, 70)
   AND NOT EXISTS (SELECT 1 FROM skill_caps x
                    WHERE x.skill_id = b.skill_id AND x.class_id = c.class_id AND x.level = b.level);

-- 3. ------------------------------------------------------------------- combat specials
-- The twelve activated specials the reward picker rotates (skill_pool.lua). Every class needs a CAP
-- for these or Client::CanHaveSkill refuses, the grant silently does nothing, and the client hides
-- the ability -- which is the whole reason a picked reward could look like it did nothing.
--
-- ⚠️ A CAP IS NOT THE SAME AS HAVING THE SKILL. This only makes it POSSIBLE. Which ones a character
-- actually gets is decided in Lua: natively-available ones are granted automatically on connect,
-- and the rest are what the reward picker offers. See global_player.lua.
--
-- ⚠️⚠️ RUN THIS AFTER capturing the native map, never before. Once every class has a cap there is
-- no way left to ask "which classes had this natively" -- the answer is now "all of them". The map
-- as it stood on 2026-07-27 is recorded in lua_modules/skill_pool.lua and is the only copy.
--
-- 72 Berserking has NO cap for any class in this database, so there is nothing to copy from; it is
-- given Bard's Frenzy curve instead so it is at least reachable.
INSERT INTO skill_caps (skill_id, class_id, level, cap, class_)
SELECT b.skill_id, c.class_id, b.level, b.cap, 0
  FROM skill_caps b
  JOIN (SELECT 1 AS class_id UNION SELECT 2 UNION SELECT 3 UNION SELECT 4 UNION SELECT 5
        UNION SELECT 6 UNION SELECT 7 UNION SELECT 9 UNION SELECT 10 UNION SELECT 11
        UNION SELECT 12 UNION SELECT 13 UNION SELECT 14 UNION SELECT 15 UNION SELECT 16) c
 WHERE b.class_id = 8 AND b.skill_id IN (8, 10, 16, 21, 23, 26, 30, 38, 52, 73, 74)
   AND NOT EXISTS (SELECT 1 FROM skill_caps x
                    WHERE x.skill_id = b.skill_id AND x.class_id = c.class_id AND x.level = b.level);

-- Berserking (72): no source row anywhere, so clone Bard's Frenzy (74) curve under skill 72 for
-- every class including Bard.
INSERT INTO skill_caps (skill_id, class_id, level, cap, class_)
SELECT 72, c.class_id, b.level, b.cap, 0
  FROM skill_caps b
  JOIN (SELECT 1 AS class_id UNION SELECT 2 UNION SELECT 3 UNION SELECT 4 UNION SELECT 5
        UNION SELECT 6 UNION SELECT 7 UNION SELECT 8 UNION SELECT 9 UNION SELECT 10
        UNION SELECT 11 UNION SELECT 12 UNION SELECT 13 UNION SELECT 14 UNION SELECT 15
        UNION SELECT 16) c
 WHERE b.class_id = 8 AND b.skill_id = 74
   AND NOT EXISTS (SELECT 1 FROM skill_caps x
                    WHERE x.skill_id = 72 AND x.class_id = c.class_id AND x.level = b.level);
