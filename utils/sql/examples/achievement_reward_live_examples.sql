-- Development-only examples for the RoF2 achievement reward preview.
--
-- Run this manually against the content database after the reward catalog
-- migration. This file is intentionally outside utils/sql/git and is not
-- installed as production content.
--
-- Mastering Achievements and Norrathian Seeker demonstrate automatic rewards.
-- Omenslayer demonstrates one common title unlock plus two item choices.
-- These reserved IDs are examples; replace them if they overlap local content.
-- With Achievements:GrantRewards enabled, a reload or login may deliver the
-- automatic examples to completed characters that lack matching ledger rows.
-- Use disposable characters when this file is only a presentation test.

SET @achievement_source_type := 1;

SET @mastering_achievements_id := (
	SELECT MIN(`id`) FROM `achievements`
	WHERE `name` = 'Mastering Achievements'
);
SET @norrathian_seeker_id := (
	SELECT MIN(`id`) FROM `achievements`
	WHERE `name` = 'Norrathian Seeker'
);
SET @omenslayer_id := (
	SELECT MIN(`id`) FROM `achievements`
	WHERE `name` = 'Omenslayer'
);

SET @mastering_xp_reward_id := 9921001;
SET @mastering_coin_reward_id := 9921002;
SET @seeker_bag_reward_id := 9921003;
SET @seeker_xp_reward_id := 9921004;
SET @omenslayer_chest_reward_id := 9921005;
SET @omenslayer_title_reward_id := 9921006;
SET @omenslayer_alternate_reward_id := 9921007;

-- Choose an unrestricted title set that contains both a prefix and a suffix.
-- Reward type 5 consumes titles.title_set, not titles.id. Replace this lookup
-- with a known title_set when content and character tables use separate schemas.
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
		HAVING MAX(`prefix` <> '') = 1 AND MAX(`suffix` <> '') = 1
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
					NULLIF(GROUP_CONCAT(DISTINCT NULLIF(`prefix`, '') ORDER BY `prefix` SEPARATOR ', '), ''),
					NULLIF(GROUP_CONCAT(DISTINCT NULLIF(`suffix`, '') ORDER BY `suffix` SEPARATOR ', '), '')
				)
			),
			'Unlocks the prefix and suffix titles '
		),
		'Unlocks a prefix and suffix title'
	)
	FROM `titles`
	WHERE `title_set` = @example_title_set
);

-- Canonical provider-neutral grants. Norrathian Seeker uses normal-only XP
-- mode 1; 2,271,122 is 2% of the server's level 111-to-112 base interval.
INSERT INTO `rewards`
	(`reward_id`, `reward_type`, `reward_data_id`, `amount`, `description`, `enabled`)
VALUES
	(@mastering_xp_reward_id, 1, 0, 1, 'Experience', 1),
	(@mastering_coin_reward_id, 3, 0, 100, '0p, 0g, 10s, 0c', 1),
	(@seeker_bag_reward_id, 0, 68489, 1, 'Apprentice Collector''s Rucksack', 1),
	(@seeker_xp_reward_id, 1, 1, 2271122,
	 '2% of the experience required to go from level 111 to 112 (No AA Experience)', 1),
	(@omenslayer_chest_reward_id, 0, 70994, 1, 'Omenslayer''s Chest', 1),
	(@omenslayer_alternate_reward_id, 0, 68489, 1,
	 'Apprentice Collector''s Rucksack (Alternate Test Choice)', 1)
ON DUPLICATE KEY UPDATE
	`reward_type` = VALUES(`reward_type`),
	`reward_data_id` = VALUES(`reward_data_id`),
	`amount` = VALUES(`amount`),
	`description` = VALUES(`description`),
	`enabled` = VALUES(`enabled`);

INSERT INTO `rewards`
	(`reward_id`, `reward_type`, `reward_data_id`, `amount`, `description`, `enabled`)
SELECT
	@omenslayer_title_reward_id,
	5,
	@example_title_set,
	1,
	@example_title_reward_description,
	1
WHERE @example_title_set IS NOT NULL
ON DUPLICATE KEY UPDATE
	`reward_type` = VALUES(`reward_type`),
	`reward_data_id` = VALUES(`reward_data_id`),
	`amount` = VALUES(`amount`),
	`description` = VALUES(`description`),
	`enabled` = VALUES(`enabled`);

-- Automatic achievement grants. The provider-specific ledger still controls
-- at-most-once delivery for each character.
INSERT INTO `reward_source_entries`
	(`source_type`, `source_id`, `sequence`, `reward_id`)
SELECT @achievement_source_type, @mastering_achievements_id, 4000000000,
	@mastering_xp_reward_id
WHERE @mastering_achievements_id IS NOT NULL
ON DUPLICATE KEY UPDATE
	`sequence` = VALUES(`sequence`),
	`reward_id` = VALUES(`reward_id`);

INSERT INTO `reward_source_entries`
	(`source_type`, `source_id`, `sequence`, `reward_id`)
SELECT @achievement_source_type, @mastering_achievements_id, 4000000001,
	@mastering_coin_reward_id
WHERE @mastering_achievements_id IS NOT NULL
ON DUPLICATE KEY UPDATE
	`sequence` = VALUES(`sequence`),
	`reward_id` = VALUES(`reward_id`);

INSERT INTO `reward_source_entries`
	(`source_type`, `source_id`, `sequence`, `reward_id`)
SELECT @achievement_source_type, @norrathian_seeker_id, 4000000000,
	@seeker_bag_reward_id
WHERE @norrathian_seeker_id IS NOT NULL
ON DUPLICATE KEY UPDATE
	`sequence` = VALUES(`sequence`),
	`reward_id` = VALUES(`reward_id`);

INSERT INTO `reward_source_entries`
	(`source_type`, `source_id`, `sequence`, `reward_id`)
SELECT @achievement_source_type, @norrathian_seeker_id, 4000000001,
	@seeker_xp_reward_id
WHERE @norrathian_seeker_id IS NOT NULL
ON DUPLICATE KEY UPDATE
	`sequence` = VALUES(`sequence`),
	`reward_id` = VALUES(`reward_id`);

-- Reuse an existing Omenslayer set when present. Otherwise use the reserved
-- development set identity.
SET @omenslayer_reward_set_id := COALESCE(
	(
		SELECT `reward_set_id`
		FROM `reward_sources`
		WHERE
			`source_type` = @achievement_source_type
			AND `source_id` = @omenslayer_id
		LIMIT 1
	),
	9941001
);

INSERT INTO `reward_sets` (`reward_set_id`, `title`, `enabled`)
SELECT @omenslayer_reward_set_id, 'Omenslayer - Example Item Choice', 1
WHERE @omenslayer_id IS NOT NULL
ON DUPLICATE KEY UPDATE
	`title` = VALUES(`title`),
	`enabled` = VALUES(`enabled`);

INSERT INTO `reward_sources`
	(`source_type`, `source_id`, `reward_set_id`, `enabled`)
SELECT @achievement_source_type, @omenslayer_id, @omenslayer_reward_set_id, 1
WHERE @omenslayer_id IS NOT NULL
ON DUPLICATE KEY UPDATE
	`reward_set_id` = VALUES(`reward_set_id`),
	`enabled` = VALUES(`enabled`);

INSERT INTO `reward_options`
	(`reward_set_id`, `option_id`, `sequence`, `label`, `common_to_all`, `flags`, `enabled`)
SELECT @omenslayer_reward_set_id, 4000000000, 0,
	'Player Flags (Included with Either Item)', 1, 0, 1
WHERE @omenslayer_id IS NOT NULL AND @example_title_set IS NOT NULL
ON DUPLICATE KEY UPDATE
	`sequence` = VALUES(`sequence`),
	`label` = VALUES(`label`),
	`common_to_all` = VALUES(`common_to_all`),
	`flags` = VALUES(`flags`),
	`enabled` = VALUES(`enabled`);

INSERT INTO `reward_options`
	(`reward_set_id`, `option_id`, `sequence`, `label`, `common_to_all`, `flags`, `enabled`)
SELECT @omenslayer_reward_set_id, 4000000001, 1,
	'Omenslayer''s Chest', 0, 0, 1
WHERE @omenslayer_id IS NOT NULL
ON DUPLICATE KEY UPDATE
	`sequence` = VALUES(`sequence`),
	`label` = VALUES(`label`),
	`common_to_all` = VALUES(`common_to_all`),
	`flags` = VALUES(`flags`),
	`enabled` = VALUES(`enabled`);

INSERT INTO `reward_options`
	(`reward_set_id`, `option_id`, `sequence`, `label`, `common_to_all`, `flags`, `enabled`)
SELECT @omenslayer_reward_set_id, 4000000002, 2,
	'Apprentice Collector''s Rucksack (Test)', 0, 0, 1
WHERE @omenslayer_id IS NOT NULL
ON DUPLICATE KEY UPDATE
	`sequence` = VALUES(`sequence`),
	`label` = VALUES(`label`),
	`common_to_all` = VALUES(`common_to_all`),
	`flags` = VALUES(`flags`),
	`enabled` = VALUES(`enabled`);

INSERT INTO `reward_option_entries`
	(`reward_set_id`, `option_id`, `sequence`, `reward_id`)
SELECT @omenslayer_reward_set_id, 4000000000, 0, @omenslayer_title_reward_id
WHERE @omenslayer_id IS NOT NULL AND @example_title_set IS NOT NULL
ON DUPLICATE KEY UPDATE
	`option_id` = VALUES(`option_id`),
	`sequence` = VALUES(`sequence`);

INSERT INTO `reward_option_entries`
	(`reward_set_id`, `option_id`, `sequence`, `reward_id`)
SELECT @omenslayer_reward_set_id, 4000000001, 0, @omenslayer_chest_reward_id
WHERE @omenslayer_id IS NOT NULL
ON DUPLICATE KEY UPDATE
	`option_id` = VALUES(`option_id`),
	`sequence` = VALUES(`sequence`);

INSERT INTO `reward_option_entries`
	(`reward_set_id`, `option_id`, `sequence`, `reward_id`)
SELECT @omenslayer_reward_set_id, 4000000002, 0, @omenslayer_alternate_reward_id
WHERE @omenslayer_id IS NOT NULL
ON DUPLICATE KEY UPDATE
	`option_id` = VALUES(`option_id`),
	`sequence` = VALUES(`sequence`);

-- Review the provider mappings and rendered choice contents.
SELECT
	`source_type`, `source_id`, `reward_set_id`, `enabled`
FROM `reward_sources`
WHERE
	`source_type` = @achievement_source_type
	AND `source_id` IN (
		COALESCE(@mastering_achievements_id, 0),
		COALESCE(@norrathian_seeker_id, 0),
		COALESCE(@omenslayer_id, 0)
	);

SELECT
	`e`.`source_id`, `e`.`sequence`, `r`.*
FROM `reward_source_entries` AS `e`
JOIN `rewards` AS `r` ON `r`.`reward_id` = `e`.`reward_id`
WHERE
	`e`.`source_type` = @achievement_source_type
	AND `e`.`source_id` IN (
		COALESCE(@mastering_achievements_id, 0),
		COALESCE(@norrathian_seeker_id, 0)
	)
ORDER BY `e`.`source_id`, `e`.`sequence`;

SELECT
	`o`.`reward_set_id`, `o`.`option_id`, `o`.`sequence`, `o`.`label`,
	`o`.`common_to_all`, `m`.`sequence` AS `reward_sequence`, `r`.*
FROM `reward_options` AS `o`
LEFT JOIN `reward_option_entries` AS `m`
	ON `m`.`reward_set_id` = `o`.`reward_set_id`
	AND `m`.`option_id` = `o`.`option_id`
LEFT JOIN `rewards` AS `r` ON `r`.`reward_id` = `m`.`reward_id`
WHERE `o`.`reward_set_id` = @omenslayer_reward_set_id
ORDER BY `o`.`sequence`, `o`.`option_id`, `m`.`sequence`, `m`.`reward_id`;
