-- AoTv4 -- every race may be every class: all 256 combinations playable.
-- =====================================================================================
-- Stock EQ allows only 112 of the 256 race/class pairs (no Ogre Paladin, no Troll Wizard). This
-- opens the remaining 144, which the Reforger NPC also needs -- there is no point offering a form
-- change the creation rules would refuse.
--
-- ⚠️⚠️ `char_create_combinations` DRIVES BOTH THE CLIENT AND THE SERVER. World loads it at boot to
-- grey out disallowed classes on the create screen AND to validate the create request (CLAUDE.md
-- section 14), so one table opens both halves. It needs a WORLD restart, not just a zone restart.
--
-- ⚠️⚠️ `allocation_id` POINTS AT char_create_point_allocations -- the starting stat spread. It is
-- unique per race/class pair (112 pairs, 112 allocations), so a new pair has no allocation of its
-- own. Each new row borrows the allocation of the SAME CLASS from an existing race: the spread is
-- overwhelmingly a function of class (a Paladin's points look like a Paladin's), and an invalid
-- allocation_id would leave a new character with no starting stats at all.
-- 📌 That means, say, an Ogre Paladin starts with a Human Paladin's spread rather than a bespoke
-- one. Approximate but coherent; a hand-tuned table per new pair would be 144 rows of guesswork.
--
-- ⚠️ Deities are taken from what that CLASS already permits, not from the race. Deity restrictions
-- in EQ are religious rather than anatomical -- a Paladin's gods are a Paladin's gods whoever is
-- swearing to them -- and copying the race's list instead would deny the new pair every deity its
-- class actually needs.
--
-- ⚠️ start_zone is copied along but is IRRELEVANT here: aotv4_start_resplendent.sql points every
-- start_zones row at Resplendent, and that table -- not this one -- is what places a new character.
-- ⚠️ expansions_req 0 so nothing is gated behind an expansion flag.
--
-- ⚠️ Re-runnable: INSERTs are keyed on what is missing.

-- ---------------------------------------------------------------- what is missing
DROP TEMPORARY TABLE IF EXISTS aotv4_missing_pairs;
CREATE TEMPORARY TABLE aotv4_missing_pairs (race INT, class INT, PRIMARY KEY (race, class));

-- ⚠️ The race list is the 16 REAL race ids, not 1..16 -- Iksar/Vah Shir/Froglok/Drakkin are
-- 128/130/330/522. Generating with a 1..16 loop would silently create combinations for races that
-- do not exist and skip four that do.
INSERT INTO aotv4_missing_pairs (race, class)
SELECT r.race, c.class
FROM   (SELECT DISTINCT race FROM char_create_combinations) r
CROSS  JOIN (SELECT DISTINCT class FROM char_create_combinations) c
LEFT   JOIN (SELECT DISTINCT race, class FROM char_create_combinations) have
         ON have.race = r.race AND have.class = c.class
WHERE  have.race IS NULL;

-- ---------------------------------------------------------------- one allocation per class
DROP TEMPORARY TABLE IF EXISTS aotv4_class_alloc;
CREATE TEMPORARY TABLE aotv4_class_alloc (class INT PRIMARY KEY, allocation_id INT);
INSERT INTO aotv4_class_alloc (class, allocation_id)
SELECT class, MIN(allocation_id) FROM char_create_combinations GROUP BY class;

-- ---------------------------------------------------------------- the deities each class allows
DROP TEMPORARY TABLE IF EXISTS aotv4_class_deity;
CREATE TEMPORARY TABLE aotv4_class_deity (class INT, deity INT, PRIMARY KEY (class, deity));
INSERT INTO aotv4_class_deity (class, deity)
SELECT DISTINCT class, deity FROM char_create_combinations;

-- ---------------------------------------------------------------- open them
INSERT INTO char_create_combinations (allocation_id, race, class, deity, start_zone, expansions_req)
SELECT a.allocation_id, m.race, m.class, d.deity, 0, 0
FROM   aotv4_missing_pairs m
JOIN   aotv4_class_alloc  a ON a.class = m.class
JOIN   aotv4_class_deity  d ON d.class = m.class;

SELECT (SELECT COUNT(*) FROM (SELECT DISTINCT race, class FROM char_create_combinations) x) AS pairs_now,
       (SELECT COUNT(*) FROM aotv4_missing_pairs)                                           AS pairs_added,
       (SELECT COUNT(*) FROM char_create_combinations)                                      AS total_rows;

DROP TEMPORARY TABLE IF EXISTS aotv4_missing_pairs;
DROP TEMPORARY TABLE IF EXISTS aotv4_class_alloc;
DROP TEMPORARY TABLE IF EXISTS aotv4_class_deity;
