-- AoTv4 -- five achievements that each open one region of the player's choosing.
-- =====================================================================================
-- Earned by DYING AT THE LEVEL CAP. The first capped death completes the first, the second the
-- second, and so on to five -- which, with the region granted at character creation, opens all six.
--
-- ⚠️⚠️ THE CHAIN IS EXPRESSED IN DATA, NOT IN CODE. Each capped death credits ONE point of progress
-- to every `death_at_max` objective (AchievementManager::ProcessDeathAtMaxLevel), and the five rows
-- below simply ask for 1, 2, 3, 4 and 5. There is no per-achievement bookkeeping anywhere, and a
-- sixth would be one more row.
-- ⚠️ That only works because the objective is credited with absolute_progress = FALSE, so the count
-- ACCUMULATES. True would overwrite it with 1 on every death and nothing past the first would
-- ever complete.
--
-- ⚠️⚠️ THE REWARD IS A CREDIT, NOT A REGION. `region_choice` adds one to `region_credits_<charid>`;
-- the player spends it on whichever region they want (aotv4_regions.lua). Naming a region id here
-- would fix the order and make the five achievements interchangeable with a plain counter -- the
-- choice IS the reward.
--
-- ⚠️ Achievement ids are NOT category-bound (CLAUDE.md section 15): 9000001-9000016 is Class Mastery,
-- 700001-700448 is Zone Slayer, and an ON DUPLICATE KEY UPDATE into an occupied range silently
-- hijacks somebody else's rows. 9100001-9100005 is free.
-- ⚠️ Achievements are read from the DB at ZONE BOOT, not shared memory -- zone restart, no
-- ./shared_memory.

DELETE FROM custom_achievement_rewards    WHERE achievement_id BETWEEN 9100001 AND 9100005;
DELETE FROM custom_achievement_objectives WHERE achievement_id BETWEEN 9100001 AND 9100005;
DELETE FROM custom_achievements           WHERE id             BETWEEN 9100001 AND 9100005;

-- ⚠️ The `slug` column is UNIQUE and NOT NULL with no default -- omitting it makes every row default
-- to the empty string and the SECOND insert fails with a duplicate-key error on uk_slug.
INSERT INTO custom_achievements (id, category_id, slug, name, description, enabled, hidden, sort_order) VALUES
 (9100001, 8, 'region_unlock_1', 'The First Ending',  'Fall once at the height of your power.',        1, 0, 1),
 (9100002, 8, 'region_unlock_2', 'The Second Ending', 'Fall twice at the height of your power.',       1, 0, 2),
 (9100003, 8, 'region_unlock_3', 'The Third Ending',  'Fall three times at the height of your power.', 1, 0, 3),
 (9100004, 8, 'region_unlock_4', 'The Fourth Ending', 'Fall four times at the height of your power.',  1, 0, 4),
 (9100005, 8, 'region_unlock_5', 'The Fifth Ending',  'Fall five times at the height of your power.',  1, 0, 5);

-- ⚠️ required_count is the CUMULATIVE number of capped deaths, which is why the five differ only in
-- that column. class_mask 0 = any class.
-- ⚠️⚠️ `custom_achievement_objectives.id` has NO auto_increment (unlike the rewards table, which
-- does), so ids must be supplied. Omitting them fails with "Field 'id' doesn't have a default value"
-- -- and only on this table, which makes it look like the row itself is malformed.
INSERT INTO custom_achievement_objectives
      (id, achievement_id, objective_index, objective_type, target_id, target_name, required_count, zone_id, class_mask, optional) VALUES
 (91000011, 9100001, 0, 'death_at_max', 0, 'Die at the level cap',       1, 0, 0, 0),
 (91000021, 9100002, 0, 'death_at_max', 0, 'Die at the level cap twice', 2, 0, 0, 0),
 (91000031, 9100003, 0, 'death_at_max', 0, 'Die at the level cap x3',    3, 0, 0, 0),
 (91000041, 9100004, 0, 'death_at_max', 0, 'Die at the level cap x4',    4, 0, 0, 0),
 (91000051, 9100005, 0, 'death_at_max', 0, 'Die at the level cap x5',    5, 0, 0, 0);

-- ⚠️ auto_claim = 1: the credit should land the moment the achievement completes. Leaving it for a
-- manual claim would mean dying at cap and appearing to get nothing.
-- ⚠️ reward_id is unused for `region_choice` -- there is no region to name yet, which is the point.
INSERT INTO custom_achievement_rewards
      (achievement_id, reward_type, reward_id, amount, chance, tier, claim_once, auto_claim, preview_text) VALUES
 (9100001, 'region_choice', 0, 1, 100, 'standard', 1, 1, 'Open a region of your choosing'),
 (9100002, 'region_choice', 0, 1, 100, 'standard', 1, 1, 'Open a region of your choosing'),
 (9100003, 'region_choice', 0, 1, 100, 'standard', 1, 1, 'Open a region of your choosing'),
 (9100004, 'region_choice', 0, 1, 100, 'standard', 1, 1, 'Open a region of your choosing'),
 (9100005, 'region_choice', 0, 1, 100, 'standard', 1, 1, 'Open a region of your choosing');

SELECT (SELECT COUNT(*) FROM custom_achievements           WHERE id BETWEEN 9100001 AND 9100005) AS achievements,
       (SELECT COUNT(*) FROM custom_achievement_objectives WHERE achievement_id BETWEEN 9100001 AND 9100005) AS objectives,
       (SELECT COUNT(*) FROM custom_achievement_rewards    WHERE achievement_id BETWEEN 9100001 AND 9100005) AS rewards,
       (SELECT COUNT(*) FROM regions) AS regions_available;
