-- Development-only examples for the RoF2 achievement reward preview.
--
-- Run this manually against the content database after the achievement reward
-- schema migrations. This is intentionally outside utils/sql/git so it is not
-- installed as production content by the database migration path.
--
-- Mastering Achievements and Norrathian Seeker are fixed/common bundles.
-- Omenslayer is deliberately configured as the multi-choice test: its title
-- unlock is common to both choices, while items 70994 and 68489 are separate
-- selectable alternatives.
--
-- IMPORTANT: unmapped achievement_rewards are automatic grants. On the next
-- achievement reload/login, completed characters without matching ledger rows
-- will receive the fixed Mastering/Norrathian rows when
-- Achievements:GrantRewards is enabled. Omenslayer's mapped rows wait for a
-- validated selection. Disable that rule or use a disposable character if the
-- fixed examples are only a rendering test.
--
-- Reserved high sequence values keep these examples separate from authored
-- rows while allowing idempotent ON DUPLICATE KEY UPDATE statements.

SET @mastering_achievements_id := (
	SELECT MIN(`id`)
	FROM `achievements`
	WHERE `name` = 'Mastering Achievements'
);
SET @norrathian_seeker_id := (
	SELECT MIN(`id`)
	FROM `achievements`
	WHERE `name` = 'Norrathian Seeker'
);
SET @omenslayer_id := (
	SELECT MIN(`id`)
	FROM `achievements`
	WHERE `name` = 'Omenslayer'
);

-- Choose an existing unrestricted title set that exposes at least one prefix
-- and one suffix. Reward type 5 consumes titles.title_set, not titles.id.
-- Replace this query with a known title_set when content and character tables
-- live in separate schemas.
SET @example_title_set := (
	SELECT `candidate`.`title_set`
	FROM (
		SELECT `title_set`
		FROM `titles`
		WHERE
			`title_set` > 0
			AND `skill_id` = -1
			AND `min_skill_value` = -1
			AND `max_skill_value` = -1
			AND `min_aa_points` = -1
			AND `max_aa_points` = -1
			AND `class` = -1
			AND `gender` = -1
			AND `char_id` = -1
			AND `status` = -1
			AND `item_id` = -1
		GROUP BY `title_set`
		HAVING
			MAX(`prefix` <> '') = 1
			AND MAX(`suffix` <> '') = 1
		ORDER BY `title_set`
		LIMIT 1
	) AS `candidate`
);

SET @example_title_reward_description := (
	SELECT COALESCE(
		NULLIF(
			CONCAT(
				'Unlocks the prefix and suffix titles ',
				CONCAT_WS(
					' and ',
					NULLIF(
						GROUP_CONCAT(
							DISTINCT NULLIF(`prefix`, '')
							ORDER BY `prefix`
							SEPARATOR ', '
						),
						''
					),
					NULLIF(
						GROUP_CONCAT(
							DISTINCT NULLIF(`suffix`, '')
							ORDER BY `suffix`
							SEPARATOR ', '
						),
						''
					)
				)
			),
			'Unlocks the prefix and suffix titles '
		),
		'Unlocks a prefix and suffix title'
	)
	FROM `titles`
	WHERE `title_set` = @example_title_set
);

-- Mastering Achievements:
-- The live Experience quantity was not supplied, so amount 1 is the smallest
-- valid preview/grant value. Coin is stored in copper: 10 silver = 100 copper.
INSERT INTO `achievement_rewards`
	(
		`achievement_id`,
		`sequence`,
		`reward_type`,
		`reward_data_id`,
		`amount`,
		`description`,
		`enabled`
	)
SELECT
	@mastering_achievements_id,
	4000000000,
	1,
	0,
	1,
	'Experience',
	1
WHERE @mastering_achievements_id IS NOT NULL
ON DUPLICATE KEY UPDATE
	`reward_type` = VALUES(`reward_type`),
	`reward_data_id` = VALUES(`reward_data_id`),
	`amount` = VALUES(`amount`),
	`description` = VALUES(`description`),
	`enabled` = VALUES(`enabled`);

INSERT INTO `achievement_rewards`
	(
		`achievement_id`,
		`sequence`,
		`reward_type`,
		`reward_data_id`,
		`amount`,
		`description`,
		`enabled`
	)
SELECT
	@mastering_achievements_id,
	4000000001,
	3,
	0,
	100,
	'0p, 0g, 10s, 0c',
	1
WHERE @mastering_achievements_id IS NOT NULL
ON DUPLICATE KEY UPDATE
	`reward_type` = VALUES(`reward_type`),
	`reward_data_id` = VALUES(`reward_data_id`),
	`amount` = VALUES(`amount`),
	`description` = VALUES(`description`),
	`enabled` = VALUES(`enabled`);

-- Norrathian Seeker:
-- This definition is absent from the supplied ToB resource snapshot, so the
-- name lookup safely inserts no rows unless it exists in the active database.
INSERT INTO `achievement_rewards`
	(
		`achievement_id`,
		`sequence`,
		`reward_type`,
		`reward_data_id`,
		`amount`,
		`description`,
		`enabled`
	)
SELECT
	@norrathian_seeker_id,
	4000000000,
	0,
	68489,
	1,
	'Apprentice Collector''s Rucksack',
	1
WHERE @norrathian_seeker_id IS NOT NULL
ON DUPLICATE KEY UPDATE
	`reward_type` = VALUES(`reward_type`),
	`reward_data_id` = VALUES(`reward_data_id`),
	`amount` = VALUES(`amount`),
	`description` = VALUES(`description`),
	`enabled` = VALUES(`enabled`);

-- reward_data_id 1 selects fixed normal-only XP: no AA allocation and no XP
-- multipliers. The server's base formula gives:
--   level 111 start = 4,126,100,000
--   level 112 start = 4,239,656,100
--   2% of the 113,556,100 difference = 2,271,122
INSERT INTO `achievement_rewards`
	(
		`achievement_id`,
		`sequence`,
		`reward_type`,
		`reward_data_id`,
		`amount`,
		`description`,
		`enabled`
	)
SELECT
	@norrathian_seeker_id,
	4000000001,
	1,
	1,
	2271122,
	'2% of the experience required to go from level 111 to 112 (No AA Experience)',
	1
WHERE @norrathian_seeker_id IS NOT NULL
ON DUPLICATE KEY UPDATE
	`reward_type` = VALUES(`reward_type`),
	`reward_data_id` = VALUES(`reward_data_id`),
	`amount` = VALUES(`amount`),
	`description` = VALUES(`description`),
	`enabled` = VALUES(`enabled`);

-- Omenslayer:
INSERT INTO `achievement_rewards`
	(
		`achievement_id`,
		`sequence`,
		`reward_type`,
		`reward_data_id`,
		`amount`,
		`description`,
		`enabled`
	)
SELECT
	@omenslayer_id,
	4000000000,
	0,
	70994,
	1,
	'Omenslayer''s Chest',
	1
WHERE @omenslayer_id IS NOT NULL
ON DUPLICATE KEY UPDATE
	`reward_type` = VALUES(`reward_type`),
	`reward_data_id` = VALUES(`reward_data_id`),
	`amount` = VALUES(`amount`),
	`description` = VALUES(`description`),
	`enabled` = VALUES(`enabled`);

INSERT INTO `achievement_rewards`
	(
		`achievement_id`,
		`sequence`,
		`reward_type`,
		`reward_data_id`,
		`amount`,
		`description`,
		`enabled`
	)
SELECT
	@omenslayer_id,
	4000000001,
	5,
	@example_title_set,
	1,
	@example_title_reward_description,
	1
WHERE
	@omenslayer_id IS NOT NULL
	AND @example_title_set IS NOT NULL
ON DUPLICATE KEY UPDATE
	`reward_type` = VALUES(`reward_type`),
	`reward_data_id` = VALUES(`reward_data_id`),
	`amount` = VALUES(`amount`),
	`description` = VALUES(`description`),
	`enabled` = VALUES(`enabled`);

-- Alternate item used to exercise the left-hand Reward Choices list.
INSERT INTO `achievement_rewards`
	(
		`achievement_id`,
		`sequence`,
		`reward_type`,
		`reward_data_id`,
		`amount`,
		`description`,
		`enabled`
	)
SELECT
	@omenslayer_id,
	4000000002,
	0,
	68489,
	1,
	'Apprentice Collector''s Rucksack (Alternate Test Choice)',
	1
WHERE @omenslayer_id IS NOT NULL
ON DUPLICATE KEY UPDATE
	`reward_type` = VALUES(`reward_type`),
	`reward_data_id` = VALUES(`reward_data_id`),
	`amount` = VALUES(`amount`),
	`description` = VALUES(`description`),
	`enabled` = VALUES(`enabled`);

-- Reuse an existing Omenslayer set when present. Otherwise install this
-- reserved development-only set identity.
SET @omenslayer_reward_set_id := COALESCE(
	(
		SELECT MIN(`reward_set_id`)
		FROM `achievement_reward_sets`
		WHERE `achievement_id` = @omenslayer_id
	),
	3900901003
);

INSERT INTO `achievement_reward_sets`
	(
		`reward_set_id`,
		`achievement_id`,
		`title`,
		`enabled`
	)
SELECT
	@omenslayer_reward_set_id,
	@omenslayer_id,
	'Omenslayer - Example Item Choice',
	1
WHERE @omenslayer_id IS NOT NULL
ON DUPLICATE KEY UPDATE
	`title` = VALUES(`title`),
	`enabled` = VALUES(`enabled`);

SET @omenslayer_chest_reward_id := (
	SELECT MIN(`reward_id`)
	FROM `achievement_rewards`
	WHERE
		`achievement_id` = @omenslayer_id
		AND `sequence` = 4000000000
);
SET @omenslayer_title_reward_id := (
	SELECT MIN(`reward_id`)
	FROM `achievement_rewards`
	WHERE
		`achievement_id` = @omenslayer_id
		AND `sequence` = 4000000001
);
SET @omenslayer_alternate_reward_id := (
	SELECT MIN(`reward_id`)
	FROM `achievement_rewards`
	WHERE
		`achievement_id` = @omenslayer_id
		AND `sequence` = 4000000002
);

-- Option 4000000000 is common and is not displayed as a selectable item.
-- It is installed only when the example title-set reward resolved.
INSERT INTO `achievement_reward_options`
	(
		`reward_set_id`,
		`option_id`,
		`sequence`,
		`label`,
		`common_to_all`,
		`flags`,
		`enabled`
	)
SELECT
	@omenslayer_reward_set_id,
	4000000000,
	0,
	'Player Flags (Included with Either Item)',
	1,
	0,
	1
WHERE
	@omenslayer_id IS NOT NULL
	AND @omenslayer_title_reward_id IS NOT NULL
ON DUPLICATE KEY UPDATE
	`sequence` = VALUES(`sequence`),
	`label` = VALUES(`label`),
	`common_to_all` = VALUES(`common_to_all`),
	`flags` = VALUES(`flags`),
	`enabled` = VALUES(`enabled`);

INSERT INTO `achievement_reward_options`
	(
		`reward_set_id`,
		`option_id`,
		`sequence`,
		`label`,
		`common_to_all`,
		`flags`,
		`enabled`
	)
SELECT
	@omenslayer_reward_set_id,
	4000000001,
	1,
	'Omenslayer''s Chest',
	0,
	0,
	1
WHERE
	@omenslayer_id IS NOT NULL
	AND @omenslayer_chest_reward_id IS NOT NULL
ON DUPLICATE KEY UPDATE
	`sequence` = VALUES(`sequence`),
	`label` = VALUES(`label`),
	`common_to_all` = VALUES(`common_to_all`),
	`flags` = VALUES(`flags`),
	`enabled` = VALUES(`enabled`);

INSERT INTO `achievement_reward_options`
	(
		`reward_set_id`,
		`option_id`,
		`sequence`,
		`label`,
		`common_to_all`,
		`flags`,
		`enabled`
	)
SELECT
	@omenslayer_reward_set_id,
	4000000002,
	2,
	'Apprentice Collector''s Rucksack (Test)',
	0,
	0,
	1
WHERE
	@omenslayer_id IS NOT NULL
	AND @omenslayer_alternate_reward_id IS NOT NULL
ON DUPLICATE KEY UPDATE
	`sequence` = VALUES(`sequence`),
	`label` = VALUES(`label`),
	`common_to_all` = VALUES(`common_to_all`),
	`flags` = VALUES(`flags`),
	`enabled` = VALUES(`enabled`);

INSERT INTO `achievement_reward_option_entries`
	(`reward_set_id`, `option_id`, `reward_id`)
SELECT
	@omenslayer_reward_set_id,
	4000000000,
	@omenslayer_title_reward_id
WHERE
	@omenslayer_id IS NOT NULL
	AND @omenslayer_title_reward_id IS NOT NULL
ON DUPLICATE KEY UPDATE
	`reward_set_id` = VALUES(`reward_set_id`),
	`option_id` = VALUES(`option_id`);

INSERT INTO `achievement_reward_option_entries`
	(`reward_set_id`, `option_id`, `reward_id`)
SELECT
	@omenslayer_reward_set_id,
	4000000001,
	@omenslayer_chest_reward_id
WHERE
	@omenslayer_id IS NOT NULL
	AND @omenslayer_chest_reward_id IS NOT NULL
ON DUPLICATE KEY UPDATE
	`reward_set_id` = VALUES(`reward_set_id`),
	`option_id` = VALUES(`option_id`);

INSERT INTO `achievement_reward_option_entries`
	(`reward_set_id`, `option_id`, `reward_id`)
SELECT
	@omenslayer_reward_set_id,
	4000000002,
	@omenslayer_alternate_reward_id
WHERE
	@omenslayer_id IS NOT NULL
	AND @omenslayer_alternate_reward_id IS NOT NULL
ON DUPLICATE KEY UPDATE
	`reward_set_id` = VALUES(`reward_set_id`),
	`option_id` = VALUES(`option_id`);

-- Review resolution and any enabled selectable sets. When a set exists, its
-- mapped choices own the preview; these unmapped fixed rows remain automatic
-- grants and are intentionally not merged into that selectable preview.
SELECT
	@mastering_achievements_id AS `mastering_achievements_id`,
	@norrathian_seeker_id AS `norrathian_seeker_id`,
	@omenslayer_id AS `omenslayer_id`,
	@example_title_set AS `example_title_set`;

SELECT
	`achievement_id`,
	`reward_set_id`,
	`title`
FROM `achievement_reward_sets`
WHERE
	`enabled` = 1
	AND `achievement_id` IN (
		COALESCE(@mastering_achievements_id, 0),
		COALESCE(@norrathian_seeker_id, 0),
		COALESCE(@omenslayer_id, 0)
	);

SELECT
	`reward_id`,
	`achievement_id`,
	`sequence`,
	`reward_type`,
	`reward_data_id`,
	`amount`,
	`description`,
	`enabled`
FROM `achievement_rewards`
WHERE
	`achievement_id` IN (
		COALESCE(@mastering_achievements_id, 0),
	COALESCE(@norrathian_seeker_id, 0),
		COALESCE(@omenslayer_id, 0)
	)
	AND `sequence` BETWEEN 4000000000 AND 4000000002
ORDER BY `achievement_id`, `sequence`;

SELECT
	`o`.`reward_set_id`,
	`o`.`option_id`,
	`o`.`sequence`,
	`o`.`label`,
	`o`.`common_to_all`,
	`m`.`reward_id`,
	`r`.`reward_type`,
	`r`.`reward_data_id`,
	`r`.`amount`,
	`r`.`description`
FROM `achievement_reward_options` AS `o`
LEFT JOIN `achievement_reward_option_entries` AS `m`
	ON
		`m`.`reward_set_id` = `o`.`reward_set_id`
		AND `m`.`option_id` = `o`.`option_id`
LEFT JOIN `achievement_rewards` AS `r`
	ON `r`.`reward_id` = `m`.`reward_id`
WHERE `o`.`reward_set_id` = @omenslayer_reward_set_id
ORDER BY `o`.`sequence`, `o`.`option_id`, `m`.`reward_id`;
