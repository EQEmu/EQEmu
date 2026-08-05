-- Achievement definitions and selectable reward metadata.

CREATE TABLE IF NOT EXISTS `achievement_categories` (
	`id` INT(10) UNSIGNED NOT NULL,
	`parent_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`sequence` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`name` VARCHAR(255) NOT NULL DEFAULT '',
	`description` TEXT NOT NULL,
	`icon` VARCHAR(255) NOT NULL DEFAULT '',
	PRIMARY KEY (`id`),
	KEY `achievement_categories_parent_sequence` (`parent_id`, `sequence`, `id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `achievements` (
	`id` INT(10) UNSIGNED NOT NULL,
	`name` VARCHAR(255) NOT NULL DEFAULT '',
	`description` TEXT NOT NULL,
	`icon_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`points` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`reward_display` INT(10) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Sixth field from AchievementsClient.txt; sent in the reward display wire position',
	`world_display_flag` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Seventh field from AchievementsClient.txt; newer-client styling value not sent to RoF2',
	`definition_version` INT(10) UNSIGNED NOT NULL DEFAULT 1,
	`reset_on_version_change` TINYINT(1) UNSIGNED NOT NULL DEFAULT 1,
	`enabled` TINYINT(1) UNSIGNED NOT NULL DEFAULT 1,
	PRIMARY KEY (`id`),
	KEY `achievements_enabled` (`enabled`, `id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `achievement_category_associations` (
	`category_id` INT(10) UNSIGNED NOT NULL,
	`sequence` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`achievement_id` INT(10) UNSIGNED NOT NULL,
	`display_text` VARCHAR(255) NOT NULL DEFAULT '',
	PRIMARY KEY (`category_id`, `achievement_id`),
	KEY `achievement_category_associations_sequence` (`category_id`, `sequence`, `achievement_id`),
	KEY `achievement_category_associations_achievement` (`achievement_id`, `category_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `achievement_components` (
	`achievement_id` INT(10) UNSIGNED NOT NULL,
	`component_type` TINYINT(3) UNSIGNED NOT NULL,
	`sequence` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`component_id` INT(10) UNSIGNED NOT NULL,
	`description` TEXT NOT NULL,
	`description_2` TEXT NOT NULL,
	PRIMARY KEY (`achievement_id`, `component_type`, `component_id`),
	KEY `achievement_components_component` (`component_id`),
	KEY `achievement_components_achievement_sequence` (`achievement_id`, `component_type`, `sequence`, `component_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `achievement_component_counts` (
	`component_id` INT(10) UNSIGNED NOT NULL,
	`required_count` INT(10) UNSIGNED NOT NULL DEFAULT 1,
	PRIMARY KEY (`component_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Server-authored evaluation metadata. Numeric enum values are defined in
-- common/achievements.h and are intentionally not constrained by SQL ENUMs.
CREATE TABLE IF NOT EXISTS `achievement_criteria` (
	`id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
	`achievement_id` INT(10) UNSIGNED NOT NULL,
	`component_type` TINYINT(3) UNSIGNED NOT NULL,
	`component_sequence` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`component_id` INT(10) UNSIGNED NOT NULL,
	`event_type` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	`progress_mode` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	`behavior` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	`target_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`target_id2` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`target_value` BIGINT(20) NOT NULL DEFAULT 0,
	`required_count` INT(10) UNSIGNED NOT NULL DEFAULT 1 COMMENT 'Explicit server-authored evaluation threshold; zero is invalid',
	`enabled` TINYINT(1) UNSIGNED NOT NULL DEFAULT 1,
	PRIMARY KEY (`id`),
	UNIQUE KEY `achievement_criteria_definition` (`achievement_id`, `component_type`, `component_id`, `event_type`, `target_id`, `target_id2`),
	KEY `achievement_criteria_component` (`achievement_id`, `component_type`, `component_sequence`, `component_id`, `enabled`),
	KEY `achievement_criteria_event_target` (`event_type`, `target_id`, `target_id2`, `enabled`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `achievement_rewards` (
	`reward_id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
	`achievement_id` INT(10) UNSIGNED NOT NULL,
	`sequence` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`reward_type` TINYINT(3) UNSIGNED NOT NULL,
	`reward_data_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`amount` BIGINT(20) UNSIGNED NOT NULL DEFAULT 1,
	`description` VARCHAR(255) NOT NULL DEFAULT '',
	`enabled` TINYINT(1) UNSIGNED NOT NULL DEFAULT 1,
	PRIMARY KEY (`reward_id`),
	UNIQUE KEY `achievement_rewards_achievement_sequence` (`achievement_id`, `sequence`),
	KEY `achievement_rewards_enabled` (`achievement_id`, `enabled`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- All rows for one restriction_id must pass. Use another restriction_id to
-- express an alternative achievement requirement.
CREATE TABLE IF NOT EXISTS `achievement_cast_restrictions` (
	`restriction_id` INT(10) UNSIGNED NOT NULL,
	`achievement_id` INT(10) UNSIGNED NOT NULL,
	`requires_completed` TINYINT(1) UNSIGNED NOT NULL DEFAULT 1,
	PRIMARY KEY (`restriction_id`, `achievement_id`),
	KEY `achievement_cast_restrictions_achievement` (`achievement_id`, `restriction_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Ensure the unique identities required by the runtime are present.
SET @achievement_key_exists = (
	SELECT COUNT(*) FROM (
		SELECT `index_name`, MIN(`non_unique`) AS `non_unique`,
			GROUP_CONCAT(`column_name` ORDER BY `seq_in_index`) AS `key_columns`
		FROM `information_schema`.`statistics`
		WHERE `table_schema` = DATABASE()
			AND `table_name` = 'achievement_categories'
		GROUP BY `index_name`
	) AS `candidate_keys`
	WHERE `non_unique` = 0 AND `key_columns` = 'id'
);
SET @achievement_key_sql = IF(
	@achievement_key_exists > 0,
	'SELECT 1',
	'ALTER TABLE `achievement_categories` ADD UNIQUE KEY `achievement_categories_identity` (`id`)'
);
PREPARE achievement_key_statement FROM @achievement_key_sql;
EXECUTE achievement_key_statement;
DEALLOCATE PREPARE achievement_key_statement;

SET @achievement_key_exists = (
	SELECT COUNT(*) FROM (
		SELECT `index_name`, MIN(`non_unique`) AS `non_unique`,
			GROUP_CONCAT(`column_name` ORDER BY `seq_in_index`) AS `key_columns`
		FROM `information_schema`.`statistics`
		WHERE `table_schema` = DATABASE() AND `table_name` = 'achievements'
		GROUP BY `index_name`
	) AS `candidate_keys`
	WHERE `non_unique` = 0 AND `key_columns` = 'id'
);
SET @achievement_key_sql = IF(
	@achievement_key_exists > 0,
	'SELECT 1',
	'ALTER TABLE `achievements` ADD UNIQUE KEY `achievements_identity` (`id`)'
);
PREPARE achievement_key_statement FROM @achievement_key_sql;
EXECUTE achievement_key_statement;
DEALLOCATE PREPARE achievement_key_statement;

SET @achievement_key_exists = (
	SELECT COUNT(*) FROM (
		SELECT `index_name`, MIN(`non_unique`) AS `non_unique`,
			GROUP_CONCAT(`column_name` ORDER BY `seq_in_index`) AS `key_columns`
		FROM `information_schema`.`statistics`
		WHERE `table_schema` = DATABASE()
			AND `table_name` = 'achievement_category_associations'
		GROUP BY `index_name`
	) AS `candidate_keys`
	WHERE `non_unique` = 0
		AND `key_columns` = 'category_id,achievement_id'
);
SET @achievement_key_sql = IF(
	@achievement_key_exists > 0,
	'SELECT 1',
	'ALTER TABLE `achievement_category_associations` ADD UNIQUE KEY `achievement_category_associations_identity` (`category_id`, `achievement_id`)'
);
PREPARE achievement_key_statement FROM @achievement_key_sql;
EXECUTE achievement_key_statement;
DEALLOCATE PREPARE achievement_key_statement;

SET @achievement_key_exists = (
	SELECT COUNT(*) FROM (
		SELECT `index_name`, MIN(`non_unique`) AS `non_unique`,
			GROUP_CONCAT(`column_name` ORDER BY `seq_in_index`) AS `key_columns`
		FROM `information_schema`.`statistics`
		WHERE `table_schema` = DATABASE()
			AND `table_name` = 'achievement_components'
		GROUP BY `index_name`
	) AS `candidate_keys`
	WHERE `non_unique` = 0
		AND `key_columns` = 'achievement_id,component_type,component_id'
);
SET @achievement_key_sql = IF(
	@achievement_key_exists > 0,
	'SELECT 1',
	'ALTER TABLE `achievement_components` ADD UNIQUE KEY `achievement_components_identity` (`achievement_id`, `component_type`, `component_id`)'
);
PREPARE achievement_key_statement FROM @achievement_key_sql;
EXECUTE achievement_key_statement;
DEALLOCATE PREPARE achievement_key_statement;

SET @achievement_key_exists = (
	SELECT COUNT(*) FROM (
		SELECT `index_name`, MIN(`non_unique`) AS `non_unique`,
			GROUP_CONCAT(`column_name` ORDER BY `seq_in_index`) AS `key_columns`
		FROM `information_schema`.`statistics`
		WHERE `table_schema` = DATABASE()
			AND `table_name` = 'achievement_component_counts'
		GROUP BY `index_name`
	) AS `candidate_keys`
	WHERE `non_unique` = 0 AND `key_columns` = 'component_id'
);
SET @achievement_key_sql = IF(
	@achievement_key_exists > 0,
	'SELECT 1',
	'ALTER TABLE `achievement_component_counts` ADD UNIQUE KEY `achievement_component_counts_identity` (`component_id`)'
);
PREPARE achievement_key_statement FROM @achievement_key_sql;
EXECUTE achievement_key_statement;
DEALLOCATE PREPARE achievement_key_statement;

SET @achievement_key_exists = (
	SELECT COUNT(*) FROM (
		SELECT `index_name`, MIN(`non_unique`) AS `non_unique`,
			GROUP_CONCAT(`column_name` ORDER BY `seq_in_index`) AS `key_columns`
		FROM `information_schema`.`statistics`
		WHERE `table_schema` = DATABASE()
			AND `table_name` = 'achievement_criteria'
		GROUP BY `index_name`
	) AS `candidate_keys`
	WHERE `non_unique` = 0
		AND `key_columns` =
			'achievement_id,component_type,component_id,event_type,target_id,target_id2'
);
SET @achievement_key_sql = IF(
	@achievement_key_exists > 0,
	'SELECT 1',
	'ALTER TABLE `achievement_criteria` ADD UNIQUE KEY `achievement_criteria_definition` (`achievement_id`, `component_type`, `component_id`, `event_type`, `target_id`, `target_id2`)'
);
PREPARE achievement_key_statement FROM @achievement_key_sql;
EXECUTE achievement_key_statement;
DEALLOCATE PREPARE achievement_key_statement;

SET @achievement_key_exists = (
	SELECT COUNT(*) FROM (
		SELECT `index_name`, MIN(`non_unique`) AS `non_unique`,
			GROUP_CONCAT(`column_name` ORDER BY `seq_in_index`) AS `key_columns`
		FROM `information_schema`.`statistics`
		WHERE `table_schema` = DATABASE()
			AND `table_name` = 'achievement_rewards'
		GROUP BY `index_name`
	) AS `candidate_keys`
	WHERE `non_unique` = 0 AND `key_columns` = 'reward_id'
);
SET @achievement_key_sql = IF(
	@achievement_key_exists > 0,
	'SELECT 1',
	'ALTER TABLE `achievement_rewards` ADD UNIQUE KEY `achievement_rewards_identity` (`reward_id`)'
);
PREPARE achievement_key_statement FROM @achievement_key_sql;
EXECUTE achievement_key_statement;
DEALLOCATE PREPARE achievement_key_statement;

SET @achievement_key_exists = (
	SELECT COUNT(*) FROM (
		SELECT `index_name`, MIN(`non_unique`) AS `non_unique`,
			GROUP_CONCAT(`column_name` ORDER BY `seq_in_index`) AS `key_columns`
		FROM `information_schema`.`statistics`
		WHERE `table_schema` = DATABASE()
			AND `table_name` = 'achievement_cast_restrictions'
		GROUP BY `index_name`
	) AS `candidate_keys`
	WHERE `non_unique` = 0
		AND `key_columns` = 'restriction_id,achievement_id'
);
SET @achievement_key_sql = IF(
	@achievement_key_exists > 0,
	'SELECT 1',
	'ALTER TABLE `achievement_cast_restrictions` ADD UNIQUE KEY `achievement_cast_restrictions_identity` (`restriction_id`, `achievement_id`)'
);
PREPARE achievement_key_statement FROM @achievement_key_sql;
EXECUTE achievement_key_statement;
DEALLOCATE PREPARE achievement_key_statement;

-- Verified client restrictions for Legendary Answerer (achievement 2100243).
INSERT INTO `achievement_cast_restrictions`
	(`restriction_id`, `achievement_id`, `requires_completed`)
VALUES
	(39281, 2100243, 0),
	(42280, 2100243, 1)
ON DUPLICATE KEY UPDATE
	`requires_completed` = VALUES(`requires_completed`);

-- Fail the migration instead of accepting pre-existing tables that only share
-- these names but are missing fields required by the achievement runtime.
SELECT `id`, `parent_id`, `sequence`, `name`, `description`, `icon`
FROM `achievement_categories` LIMIT 0;
SELECT `id`, `name`, `description`, `icon_id`, `points`, `reward_display`,
	`world_display_flag`, `definition_version`, `reset_on_version_change`, `enabled`
FROM `achievements` LIMIT 0;
SELECT `category_id`, `sequence`, `achievement_id`, `display_text`
FROM `achievement_category_associations` LIMIT 0;
SELECT `achievement_id`, `component_type`, `sequence`, `component_id`,
	`description`, `description_2`
FROM `achievement_components` LIMIT 0;
SELECT `component_id`, `required_count`
FROM `achievement_component_counts` LIMIT 0;
SELECT `id`, `achievement_id`, `component_type`, `component_sequence`,
	`component_id`, `event_type`, `progress_mode`, `behavior`, `target_id`,
	`target_id2`, `target_value`, `required_count`, `enabled`
FROM `achievement_criteria` LIMIT 0;
SELECT `reward_id`, `achievement_id`, `sequence`, `reward_type`,
	`reward_data_id`, `amount`, `description`, `enabled`
FROM `achievement_rewards` LIMIT 0;
SELECT `restriction_id`, `achievement_id`, `requires_completed`
FROM `achievement_cast_restrictions` LIMIT 0;

-- Normalize criterion thresholds on an existing schema.
UPDATE `achievement_criteria`
SET `required_count` = 1
WHERE `required_count` = 0;

ALTER TABLE `achievement_criteria`
	MODIFY COLUMN `required_count` INT(10) UNSIGNED NOT NULL DEFAULT 1
	COMMENT 'Explicit server-authored evaluation threshold; zero is invalid';

-- Server-authored selectable achievement rewards. Client resource files define
-- the achievement window, but do not contain reward-set or option contents.
CREATE TABLE IF NOT EXISTS `achievement_reward_sets` (
	`reward_set_id` INT(10) UNSIGNED NOT NULL,
	`achievement_id` INT(10) UNSIGNED NOT NULL,
	`title` VARCHAR(255) NOT NULL DEFAULT '',
	`enabled` TINYINT(1) UNSIGNED NOT NULL DEFAULT 1,
	PRIMARY KEY (`reward_set_id`),
	UNIQUE KEY `achievement_reward_sets_achievement` (`achievement_id`),
	KEY `achievement_reward_sets_enabled` (`enabled`, `achievement_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `achievement_reward_options` (
	`reward_set_id` INT(10) UNSIGNED NOT NULL,
	`option_id` INT(10) UNSIGNED NOT NULL,
	`sequence` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`label` VARCHAR(255) NOT NULL DEFAULT '',
	`common_to_all` TINYINT(1) UNSIGNED NOT NULL DEFAULT 0,
	`flags` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	`enabled` TINYINT(1) UNSIGNED NOT NULL DEFAULT 1,
	PRIMARY KEY (`reward_set_id`, `option_id`),
	KEY `achievement_reward_options_sequence`
		(`reward_set_id`, `sequence`, `option_id`),
	KEY `achievement_reward_options_enabled`
		(`reward_set_id`, `enabled`, `sequence`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- The canonical grant row remains in achievement_rewards, preserving its
-- existing per-character idempotency key. Mapping a row here changes it from
-- an automatic completion grant into part of a common or selectable bundle.
CREATE TABLE IF NOT EXISTS `achievement_reward_option_entries` (
	`reward_set_id` INT(10) UNSIGNED NOT NULL,
	`option_id` INT(10) UNSIGNED NOT NULL,
	`reward_id` BIGINT(20) UNSIGNED NOT NULL,
	PRIMARY KEY (`reward_set_id`, `option_id`, `reward_id`),
	UNIQUE KEY `achievement_reward_option_entries_reward` (`reward_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Verify the columns used by the runtime.
SELECT `reward_set_id`, `achievement_id`, `title`, `enabled`
FROM `achievement_reward_sets` LIMIT 0;
SELECT `reward_set_id`, `option_id`, `sequence`, `label`, `common_to_all`,
	`flags`, `enabled`
FROM `achievement_reward_options` LIMIT 0;
SELECT `reward_set_id`, `option_id`, `reward_id`
FROM `achievement_reward_option_entries` LIMIT 0;

-- Server-authored selectable task rewards. Existing task reward methods and
-- reward_id_list remain unchanged; tasks opt in with reward_method = 3.
CREATE TABLE IF NOT EXISTS `task_reward_sets` (
	`reward_set_id` INT(10) UNSIGNED NOT NULL,
	`task_id` INT(10) UNSIGNED NOT NULL,
	`title` VARCHAR(255) NOT NULL DEFAULT '',
	`enabled` TINYINT(1) UNSIGNED NOT NULL DEFAULT 1,
	PRIMARY KEY (`reward_set_id`),
	UNIQUE KEY `task_reward_sets_task` (`task_id`),
	KEY `task_reward_sets_enabled` (`enabled`, `task_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `task_reward_options` (
	`reward_set_id` INT(10) UNSIGNED NOT NULL,
	`option_id` INT(10) UNSIGNED NOT NULL,
	`sequence` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`label` VARCHAR(255) NOT NULL DEFAULT '',
	`common_to_all` TINYINT(1) UNSIGNED NOT NULL DEFAULT 0,
	`flags` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	`enabled` TINYINT(1) UNSIGNED NOT NULL DEFAULT 1,
	PRIMARY KEY (`reward_set_id`, `option_id`),
	KEY `task_reward_options_sequence`
		(`reward_set_id`, `sequence`, `option_id`),
	KEY `task_reward_options_enabled`
		(`reward_set_id`, `enabled`, `sequence`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `task_rewards` (
	`reward_id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
	`task_id` INT(10) UNSIGNED NOT NULL,
	`sequence` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`reward_type` TINYINT(3) UNSIGNED NOT NULL,
	`reward_data_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`amount` BIGINT(20) UNSIGNED NOT NULL DEFAULT 1,
	`description` VARCHAR(255) NOT NULL DEFAULT '',
	`enabled` TINYINT(1) UNSIGNED NOT NULL DEFAULT 1,
	PRIMARY KEY (`reward_id`),
	UNIQUE KEY `task_rewards_task_sequence` (`task_id`, `sequence`),
	KEY `task_rewards_enabled` (`task_id`, `enabled`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `task_reward_option_entries` (
	`reward_set_id` INT(10) UNSIGNED NOT NULL,
	`option_id` INT(10) UNSIGNED NOT NULL,
	`reward_id` BIGINT(20) UNSIGNED NOT NULL,
	PRIMARY KEY (`reward_set_id`, `option_id`, `reward_id`),
	UNIQUE KEY `task_reward_option_entries_reward` (`reward_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- CREATE TABLE IF NOT EXISTS does not repair partially-created tables. Verify
-- the exact unique signatures used for ownership and idempotency, and add only
-- those that are absent. Duplicate data deliberately makes ALTER fail.
SET @reward_uq_sql = IF(
	EXISTS (
		SELECT 1 FROM information_schema.statistics
		WHERE table_schema = DATABASE() AND table_name = 'task_reward_sets'
		GROUP BY index_name
		HAVING MAX(non_unique) = 0 AND COUNT(*) = 1
			AND COUNT(column_name) = COUNT(*)
			AND SUM(CASE WHEN sub_part IS NULL THEN 0 ELSE 1 END) = 0
			AND GROUP_CONCAT(column_name ORDER BY seq_in_index SEPARATOR ',') =
				'reward_set_id'
	),
	'SELECT 1',
	'ALTER TABLE `task_reward_sets` ADD UNIQUE INDEX (`reward_set_id`)'
);
PREPARE reward_uq_stmt FROM @reward_uq_sql;
EXECUTE reward_uq_stmt;
DEALLOCATE PREPARE reward_uq_stmt;

SET @reward_uq_sql = IF(
	EXISTS (
		SELECT 1 FROM information_schema.statistics
		WHERE table_schema = DATABASE() AND table_name = 'task_reward_sets'
		GROUP BY index_name
		HAVING MAX(non_unique) = 0 AND COUNT(*) = 1
			AND COUNT(column_name) = COUNT(*)
			AND SUM(CASE WHEN sub_part IS NULL THEN 0 ELSE 1 END) = 0
			AND GROUP_CONCAT(column_name ORDER BY seq_in_index SEPARATOR ',') =
				'task_id'
	),
	'SELECT 1',
	'ALTER TABLE `task_reward_sets` ADD UNIQUE INDEX (`task_id`)'
);
PREPARE reward_uq_stmt FROM @reward_uq_sql;
EXECUTE reward_uq_stmt;
DEALLOCATE PREPARE reward_uq_stmt;

SET @reward_uq_sql = IF(
	EXISTS (
		SELECT 1 FROM information_schema.statistics
		WHERE table_schema = DATABASE() AND table_name = 'task_reward_options'
		GROUP BY index_name
		HAVING MAX(non_unique) = 0 AND COUNT(*) = 2
			AND COUNT(column_name) = COUNT(*)
			AND SUM(CASE WHEN sub_part IS NULL THEN 0 ELSE 1 END) = 0
			AND GROUP_CONCAT(column_name ORDER BY seq_in_index SEPARATOR ',') =
				'reward_set_id,option_id'
	),
	'SELECT 1',
	'ALTER TABLE `task_reward_options` ADD UNIQUE INDEX (`reward_set_id`, `option_id`)'
);
PREPARE reward_uq_stmt FROM @reward_uq_sql;
EXECUTE reward_uq_stmt;
DEALLOCATE PREPARE reward_uq_stmt;

SET @reward_uq_sql = IF(
	EXISTS (
		SELECT 1 FROM information_schema.statistics
		WHERE table_schema = DATABASE() AND table_name = 'task_rewards'
		GROUP BY index_name
		HAVING MAX(non_unique) = 0 AND COUNT(*) = 1
			AND COUNT(column_name) = COUNT(*)
			AND SUM(CASE WHEN sub_part IS NULL THEN 0 ELSE 1 END) = 0
			AND GROUP_CONCAT(column_name ORDER BY seq_in_index SEPARATOR ',') =
				'reward_id'
	),
	'SELECT 1',
	'ALTER TABLE `task_rewards` ADD UNIQUE INDEX (`reward_id`)'
);
PREPARE reward_uq_stmt FROM @reward_uq_sql;
EXECUTE reward_uq_stmt;
DEALLOCATE PREPARE reward_uq_stmt;

SET @reward_uq_sql = IF(
	EXISTS (
		SELECT 1 FROM information_schema.statistics
		WHERE table_schema = DATABASE() AND table_name = 'task_rewards'
		GROUP BY index_name
		HAVING MAX(non_unique) = 0 AND COUNT(*) = 2
			AND COUNT(column_name) = COUNT(*)
			AND SUM(CASE WHEN sub_part IS NULL THEN 0 ELSE 1 END) = 0
			AND GROUP_CONCAT(column_name ORDER BY seq_in_index SEPARATOR ',') =
				'task_id,sequence'
	),
	'SELECT 1',
	'ALTER TABLE `task_rewards` ADD UNIQUE INDEX (`task_id`, `sequence`)'
);
PREPARE reward_uq_stmt FROM @reward_uq_sql;
EXECUTE reward_uq_stmt;
DEALLOCATE PREPARE reward_uq_stmt;

SET @reward_uq_sql = IF(
	EXISTS (
		SELECT 1 FROM information_schema.statistics
		WHERE table_schema = DATABASE()
			AND table_name = 'task_reward_option_entries'
		GROUP BY index_name
		HAVING MAX(non_unique) = 0 AND COUNT(*) = 3
			AND COUNT(column_name) = COUNT(*)
			AND SUM(CASE WHEN sub_part IS NULL THEN 0 ELSE 1 END) = 0
			AND GROUP_CONCAT(column_name ORDER BY seq_in_index SEPARATOR ',') =
				'reward_set_id,option_id,reward_id'
	),
	'SELECT 1',
	'ALTER TABLE `task_reward_option_entries` ADD UNIQUE INDEX (`reward_set_id`, `option_id`, `reward_id`)'
);
PREPARE reward_uq_stmt FROM @reward_uq_sql;
EXECUTE reward_uq_stmt;
DEALLOCATE PREPARE reward_uq_stmt;

SET @reward_uq_sql = IF(
	EXISTS (
		SELECT 1 FROM information_schema.statistics
		WHERE table_schema = DATABASE()
			AND table_name = 'task_reward_option_entries'
		GROUP BY index_name
		HAVING MAX(non_unique) = 0 AND COUNT(*) = 1
			AND COUNT(column_name) = COUNT(*)
			AND SUM(CASE WHEN sub_part IS NULL THEN 0 ELSE 1 END) = 0
			AND GROUP_CONCAT(column_name ORDER BY seq_in_index SEPARATOR ',') =
				'reward_id'
	),
	'SELECT 1',
	'ALTER TABLE `task_reward_option_entries` ADD UNIQUE INDEX (`reward_id`)'
);
PREPARE reward_uq_stmt FROM @reward_uq_sql;
EXECUTE reward_uq_stmt;
DEALLOCATE PREPARE reward_uq_stmt;

-- Verify the columns used by the runtime.
SELECT `reward_set_id`, `task_id`, `title`, `enabled`
FROM `task_reward_sets` LIMIT 0;
SELECT `reward_set_id`, `option_id`, `sequence`, `label`, `common_to_all`,
	`flags`, `enabled`
FROM `task_reward_options` LIMIT 0;
SELECT `reward_id`, `task_id`, `sequence`, `reward_type`, `reward_data_id`,
	`amount`, `description`, `enabled`
FROM `task_rewards` LIMIT 0;
SELECT `reward_set_id`, `option_id`, `reward_id`
FROM `task_reward_option_entries` LIMIT 0;
