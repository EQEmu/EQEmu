-- Character achievement state, reward delivery state, and queued mutations.

CREATE TABLE IF NOT EXISTS `character_achievements` (
	`character_id` INT(10) UNSIGNED NOT NULL,
	`achievement_id` INT(10) UNSIGNED NOT NULL,
	`definition_version` INT(10) UNSIGNED NOT NULL DEFAULT 1,
	`completed_at` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	PRIMARY KEY (`character_id`, `achievement_id`),
	KEY `character_achievements_achievement` (`achievement_id`, `character_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `character_achievement_progress` (
	`character_id` INT(10) UNSIGNED NOT NULL,
	`achievement_id` INT(10) UNSIGNED NOT NULL,
	`component_type` TINYINT(3) UNSIGNED NOT NULL,
	`component_sequence` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`component_id` INT(10) UNSIGNED NOT NULL,
	`current_count` BIGINT(20) UNSIGNED NOT NULL DEFAULT 0,
	`completed` TINYINT(1) UNSIGNED NOT NULL DEFAULT 0,
	`definition_version` INT(10) UNSIGNED NOT NULL DEFAULT 1,
	`updated_at` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	PRIMARY KEY (`character_id`, `achievement_id`, `component_type`, `component_id`),
	KEY `character_achievement_progress_achievement` (`achievement_id`, `component_type`, `component_sequence`, `component_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- status: 0=claimed/in-flight, 1=granted, 2=explicit delivery failure.
-- Failed rows are retryable; in-flight rows remain at-most-once after an
-- ambiguous process interruption. The primary key is the idempotency boundary.
CREATE TABLE IF NOT EXISTS `character_achievement_rewards` (
	`character_id` INT(10) UNSIGNED NOT NULL,
	`achievement_id` INT(10) UNSIGNED NOT NULL,
	`reward_id` BIGINT(20) UNSIGNED NOT NULL,
	`status` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	`attempt_count` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`granted_at` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`last_attempt_at` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`last_error` VARCHAR(255) NOT NULL DEFAULT '',
	PRIMARY KEY (`character_id`, `achievement_id`, `reward_id`),
	KEY `character_achievement_rewards_status` (`status`, `character_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Ensure the unique identities required for idempotent character state.
SET @achievement_key_exists = (
	SELECT COUNT(*) FROM (
		SELECT `index_name`, MIN(`non_unique`) AS `non_unique`,
			GROUP_CONCAT(`column_name` ORDER BY `seq_in_index`) AS `key_columns`
		FROM `information_schema`.`statistics`
		WHERE `table_schema` = DATABASE()
			AND `table_name` = 'character_achievements'
		GROUP BY `index_name`
	) AS `candidate_keys`
	WHERE `non_unique` = 0
		AND `key_columns` = 'character_id,achievement_id'
);
SET @achievement_key_sql = IF(
	@achievement_key_exists > 0,
	'SELECT 1',
	'ALTER TABLE `character_achievements` ADD UNIQUE KEY `character_achievements_identity` (`character_id`, `achievement_id`)'
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
			AND `table_name` = 'character_achievement_progress'
		GROUP BY `index_name`
	) AS `candidate_keys`
	WHERE `non_unique` = 0
		AND `key_columns` =
			'character_id,achievement_id,component_type,component_id'
);
SET @achievement_key_sql = IF(
	@achievement_key_exists > 0,
	'SELECT 1',
	'ALTER TABLE `character_achievement_progress` ADD UNIQUE KEY `character_achievement_progress_identity` (`character_id`, `achievement_id`, `component_type`, `component_id`)'
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
			AND `table_name` = 'character_achievement_rewards'
		GROUP BY `index_name`
	) AS `candidate_keys`
	WHERE `non_unique` = 0
		AND `key_columns` = 'character_id,achievement_id,reward_id'
);
SET @achievement_key_sql = IF(
	@achievement_key_exists > 0,
	'SELECT 1',
	'ALTER TABLE `character_achievement_rewards` ADD UNIQUE KEY `character_achievement_rewards_identity` (`character_id`, `achievement_id`, `reward_id`)'
);
PREPARE achievement_key_statement FROM @achievement_key_sql;
EXECUTE achievement_key_statement;
DEALLOCATE PREPARE achievement_key_statement;

-- Verify the columns used by the runtime.
SELECT `character_id`, `achievement_id`, `definition_version`, `completed_at`
FROM `character_achievements` LIMIT 0;
SELECT `character_id`, `achievement_id`, `component_type`,
	`component_sequence`, `component_id`, `current_count`, `completed`,
	`definition_version`, `updated_at`
FROM `character_achievement_progress` LIMIT 0;
SELECT `character_id`, `achievement_id`, `reward_id`, `status`,
	`attempt_count`, `granted_at`, `last_attempt_at`, `last_error`
FROM `character_achievement_rewards` LIMIT 0;

-- status: 0 with option 0=pending; 0 with an option=claim in progress;
-- 1=fully granted; 2=explicit/retryable delivery failure; 3=ambiguous
-- delivery. Per-entry delivery remains guarded by
-- character_achievement_rewards, so resuming a ledger-safe claim cannot
-- duplicate an entry that was durably finalized.
CREATE TABLE IF NOT EXISTS `character_achievement_reward_selections` (
	`character_id` INT(10) UNSIGNED NOT NULL,
	`achievement_id` INT(10) UNSIGNED NOT NULL,
	`reward_set_id` INT(10) UNSIGNED NOT NULL,
	`selected_option_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`status` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	`attempt_count` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`claimed_at` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`last_attempt_at` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`last_error` VARCHAR(255) NOT NULL DEFAULT '',
	PRIMARY KEY (`character_id`, `achievement_id`, `reward_set_id`),
	KEY `character_achievement_reward_selections_status`
		(`status`, `character_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

SELECT `character_id`, `achievement_id`, `reward_set_id`,
	`selected_option_id`, `status`, `attempt_count`, `claimed_at`,
	`last_attempt_at`, `last_error`
FROM `character_achievement_reward_selections` LIMIT 0;

-- A task acceptance reserves a server-owned occurrence token without changing
-- character_tasks.accepted_time. REPLACE intentionally allocates a new token
-- when the same character accepts the same task again.
CREATE TABLE IF NOT EXISTS `character_task_reward_instances` (
	`occurrence_id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
	`character_id` INT(10) UNSIGNED NOT NULL,
	`task_id` INT(10) UNSIGNED NOT NULL,
	`accepted_time` INT(10) UNSIGNED NOT NULL,
	PRIMARY KEY (`occurrence_id`),
	UNIQUE KEY `character_task_reward_instances_source`
		(`character_id`, `task_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- A pending row is created before a completed task is removed from
-- character_tasks. source_instance_id identifies the source task occurrence;
-- accepted_time is retained as diagnostic metadata only.
-- status: 0=claim pending/in progress, 1=fully granted,
-- 2=explicit/retryable delivery failure, 3=ambiguous delivery.
CREATE TABLE IF NOT EXISTS `character_task_reward_selections` (
	`pending_reward_id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`character_id` INT(10) UNSIGNED NOT NULL,
	`task_id` INT(10) UNSIGNED NOT NULL,
	`accepted_time` INT(10) UNSIGNED NOT NULL,
	`source_instance_id` BIGINT(20) UNSIGNED NOT NULL,
	`reward_set_id` INT(10) UNSIGNED NOT NULL,
	`selected_option_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`status` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	`attempt_count` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`claimed_at` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`last_attempt_at` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`last_error` VARCHAR(255) NOT NULL DEFAULT '',
	PRIMARY KEY (`pending_reward_id`),
	UNIQUE KEY `character_task_reward_selections_source`
		(`character_id`, `source_instance_id`),
	KEY `character_task_reward_selections_status`
		(`status`, `character_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Per-entry delivery is the idempotency boundary for claim retries.
CREATE TABLE IF NOT EXISTS `character_task_rewards` (
	`character_id` INT(10) UNSIGNED NOT NULL,
	`pending_reward_id` INT(10) UNSIGNED NOT NULL,
	`reward_id` BIGINT(20) UNSIGNED NOT NULL,
	`status` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	`attempt_count` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`granted_at` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`last_attempt_at` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`last_error` VARCHAR(255) NOT NULL DEFAULT '',
	PRIMARY KEY (`pending_reward_id`, `reward_id`),
	KEY `character_task_rewards_character` (`character_id`, `pending_reward_id`),
	KEY `character_task_rewards_status` (`status`, `character_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Normalize existing task reward rows to the occurrence-token schema.
SET @reward_schema_sql = IF(
	EXISTS (
		SELECT 1 FROM information_schema.columns
		WHERE table_schema = DATABASE()
			AND table_name = 'character_task_reward_selections'
			AND column_name = 'source_instance_id'
	),
	'SELECT 1',
	'ALTER TABLE `character_task_reward_selections` ADD COLUMN `source_instance_id` BIGINT(20) UNSIGNED NOT NULL DEFAULT 0 AFTER `accepted_time`'
);
PREPARE reward_schema_stmt FROM @reward_schema_sql;
EXECUTE reward_schema_stmt;
DEALLOCATE PREPARE reward_schema_stmt;

UPDATE `character_task_reward_selections`
SET `source_instance_id` = 9223372036854775808 + `pending_reward_id`
WHERE `source_instance_id` = 0;

-- Ensure task reward idempotency constraints are present.
SET @reward_uq_sql = IF(
	EXISTS (
		SELECT 1 FROM information_schema.statistics
		WHERE table_schema = DATABASE()
			AND table_name = 'character_task_reward_instances'
		GROUP BY index_name
		HAVING MAX(non_unique) = 0 AND COUNT(*) = 1
			AND COUNT(column_name) = COUNT(*)
			AND SUM(CASE WHEN sub_part IS NULL THEN 0 ELSE 1 END) = 0
			AND GROUP_CONCAT(column_name ORDER BY seq_in_index SEPARATOR ',') =
				'occurrence_id'
	),
	'SELECT 1',
	'ALTER TABLE `character_task_reward_instances` ADD UNIQUE INDEX (`occurrence_id`)'
);
PREPARE reward_uq_stmt FROM @reward_uq_sql;
EXECUTE reward_uq_stmt;
DEALLOCATE PREPARE reward_uq_stmt;

SET @reward_schema_sql = IF(
	EXISTS (
		SELECT 1 FROM information_schema.columns
		WHERE table_schema = DATABASE()
			AND table_name = 'character_task_reward_instances'
			AND column_name = 'occurrence_id'
			AND LOWER(extra) LIKE '%auto_increment%'
	),
	'SELECT 1',
	'ALTER TABLE `character_task_reward_instances` MODIFY COLUMN `occurrence_id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT'
);
PREPARE reward_schema_stmt FROM @reward_schema_sql;
EXECUTE reward_schema_stmt;
DEALLOCATE PREPARE reward_schema_stmt;

SET @reward_uq_sql = IF(
	EXISTS (
		SELECT 1 FROM information_schema.statistics
		WHERE table_schema = DATABASE()
			AND table_name = 'character_task_reward_instances'
		GROUP BY index_name
		HAVING MAX(non_unique) = 0 AND COUNT(*) = 2
			AND COUNT(column_name) = COUNT(*)
			AND SUM(CASE WHEN sub_part IS NULL THEN 0 ELSE 1 END) = 0
			AND GROUP_CONCAT(column_name ORDER BY seq_in_index SEPARATOR ',') =
				'character_id,task_id'
	),
	'SELECT 1',
	'ALTER TABLE `character_task_reward_instances` ADD UNIQUE INDEX (`character_id`, `task_id`)'
);
PREPARE reward_uq_stmt FROM @reward_uq_sql;
EXECUTE reward_uq_stmt;
DEALLOCATE PREPARE reward_uq_stmt;

SET @reward_uq_sql = IF(
	EXISTS (
		SELECT 1 FROM information_schema.statistics
		WHERE table_schema = DATABASE()
			AND table_name = 'character_task_reward_selections'
		GROUP BY index_name
		HAVING MAX(non_unique) = 0 AND COUNT(*) = 1
			AND COUNT(column_name) = COUNT(*)
			AND SUM(CASE WHEN sub_part IS NULL THEN 0 ELSE 1 END) = 0
			AND GROUP_CONCAT(column_name ORDER BY seq_in_index SEPARATOR ',') =
				'pending_reward_id'
	),
	'SELECT 1',
	'ALTER TABLE `character_task_reward_selections` ADD UNIQUE INDEX (`pending_reward_id`)'
);
PREPARE reward_uq_stmt FROM @reward_uq_sql;
EXECUTE reward_uq_stmt;
DEALLOCATE PREPARE reward_uq_stmt;

SET @reward_schema_sql = IF(
	EXISTS (
		SELECT 1 FROM information_schema.columns
		WHERE table_schema = DATABASE()
			AND table_name = 'character_task_reward_selections'
			AND column_name = 'pending_reward_id'
			AND LOWER(extra) LIKE '%auto_increment%'
	),
	'SELECT 1',
	'ALTER TABLE `character_task_reward_selections` MODIFY COLUMN `pending_reward_id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT'
);
PREPARE reward_schema_stmt FROM @reward_schema_sql;
EXECUTE reward_schema_stmt;
DEALLOCATE PREPARE reward_schema_stmt;

SET @reward_uq_sql = IF(
	EXISTS (
		SELECT 1 FROM information_schema.statistics
		WHERE table_schema = DATABASE()
			AND table_name = 'character_task_reward_selections'
		GROUP BY index_name
		HAVING MAX(non_unique) = 0 AND COUNT(*) = 2
			AND COUNT(column_name) = COUNT(*)
			AND SUM(CASE WHEN sub_part IS NULL THEN 0 ELSE 1 END) = 0
			AND GROUP_CONCAT(column_name ORDER BY seq_in_index SEPARATOR ',') =
				'character_id,source_instance_id'
	),
	'SELECT 1',
	'ALTER TABLE `character_task_reward_selections` ADD UNIQUE INDEX (`character_id`, `source_instance_id`)'
);
PREPARE reward_uq_stmt FROM @reward_uq_sql;
EXECUTE reward_uq_stmt;
DEALLOCATE PREPARE reward_uq_stmt;

SET @reward_uq_sql = IF(
	EXISTS (
		SELECT 1 FROM information_schema.statistics
		WHERE table_schema = DATABASE()
			AND table_name = 'character_task_rewards'
		GROUP BY index_name
		HAVING MAX(non_unique) = 0 AND COUNT(*) = 2
			AND COUNT(column_name) = COUNT(*)
			AND SUM(CASE WHEN sub_part IS NULL THEN 0 ELSE 1 END) = 0
			AND GROUP_CONCAT(column_name ORDER BY seq_in_index SEPARATOR ',') =
				'pending_reward_id,reward_id'
	),
	'SELECT 1',
	'ALTER TABLE `character_task_rewards` ADD UNIQUE INDEX (`pending_reward_id`, `reward_id`)'
);
PREPARE reward_uq_stmt FROM @reward_uq_sql;
EXECUTE reward_uq_stmt;
DEALLOCATE PREPARE reward_uq_stmt;

SET @reward_old_source_index = (
	SELECT index_name
	FROM (
		SELECT index_name, MAX(non_unique) AS is_non_unique,
			COUNT(*) AS index_columns,
			COUNT(column_name) AS named_columns,
			SUM(CASE WHEN sub_part IS NULL THEN 0 ELSE 1 END) AS prefix_columns,
			GROUP_CONCAT(column_name ORDER BY seq_in_index SEPARATOR ',')
				AS column_signature
		FROM information_schema.statistics
		WHERE table_schema = DATABASE()
			AND table_name = 'character_task_reward_selections'
		GROUP BY index_name
	) AS existing_indexes
	WHERE is_non_unique = 0
		AND index_columns = 3
		AND named_columns = index_columns
		AND prefix_columns = 0
		AND column_signature = 'character_id,task_id,accepted_time'
		AND index_name <> 'PRIMARY'
	LIMIT 1
);
SET @reward_schema_sql = IF(
	@reward_old_source_index IS NULL,
	'SELECT 1',
	CONCAT(
		'ALTER TABLE `character_task_reward_selections` DROP INDEX `',
		REPLACE(@reward_old_source_index, '`', '``'),
		'`'
	)
);
PREPARE reward_schema_stmt FROM @reward_schema_sql;
EXECUTE reward_schema_stmt;
DEALLOCATE PREPARE reward_schema_stmt;

-- Verify the columns used by the runtime.
SELECT `occurrence_id`, `character_id`, `task_id`, `accepted_time`
FROM `character_task_reward_instances` LIMIT 0;
SELECT `pending_reward_id`, `character_id`, `task_id`, `accepted_time`,
	`source_instance_id`, `reward_set_id`, `selected_option_id`, `status`,
	`attempt_count`, `claimed_at`, `last_attempt_at`, `last_error`
FROM `character_task_reward_selections` LIMIT 0;
SELECT `character_id`, `pending_reward_id`, `reward_id`, `status`,
	`attempt_count`, `granted_at`, `last_attempt_at`, `last_error`
FROM `character_task_rewards` LIMIT 0;

-- status: 0=pending, 1=blocked, 2=processing under a bounded lease.
CREATE TABLE IF NOT EXISTS `character_achievement_pending_mutations` (
	`mutation_id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
	`character_id` INT(10) UNSIGNED NOT NULL,
	`source_target_type` TINYINT(3) UNSIGNED NOT NULL,
	`source_target_id` BIGINT(20) UNSIGNED NOT NULL,
	`operation` TINYINT(3) UNSIGNED NOT NULL,
	`achievement_id` INT(10) UNSIGNED NOT NULL,
	`component_type` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	`component_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`requested_value` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`definition_version` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`status` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	`attempt_count` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`created_at` INT(10) UNSIGNED NOT NULL,
	`last_attempt_at` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`last_error` VARCHAR(255) NOT NULL DEFAULT '',
	PRIMARY KEY (`mutation_id`),
	KEY `character_achievement_pending_character`
		(`character_id`, `status`, `mutation_id`),
	KEY `character_achievement_pending_status`
		(`status`, `character_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

SET @achievement_mutation_schema_sql = IF(
	EXISTS (
		SELECT 1 FROM information_schema.statistics
		WHERE table_schema = DATABASE()
			AND table_name = 'character_achievement_pending_mutations'
			AND index_name = 'PRIMARY'
		GROUP BY index_name
		HAVING MAX(non_unique) = 0
			AND COUNT(*) = 1
			AND COUNT(column_name) = COUNT(*)
			AND SUM(CASE WHEN sub_part IS NULL THEN 0 ELSE 1 END) = 0
			AND GROUP_CONCAT(column_name ORDER BY seq_in_index SEPARATOR ',') =
				'mutation_id'
	),
	'SELECT 1',
	'ALTER TABLE `character_achievement_pending_mutations` ADD PRIMARY KEY (`mutation_id`)'
);
PREPARE achievement_mutation_schema_stmt FROM @achievement_mutation_schema_sql;
EXECUTE achievement_mutation_schema_stmt;
DEALLOCATE PREPARE achievement_mutation_schema_stmt;

ALTER TABLE `character_achievement_pending_mutations`
	ENGINE = InnoDB,
	DEFAULT CHARACTER SET = utf8mb4,
	MODIFY COLUMN `mutation_id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
	MODIFY COLUMN `character_id` INT(10) UNSIGNED NOT NULL,
	MODIFY COLUMN `source_target_type` TINYINT(3) UNSIGNED NOT NULL,
	MODIFY COLUMN `source_target_id` BIGINT(20) UNSIGNED NOT NULL,
	MODIFY COLUMN `operation` TINYINT(3) UNSIGNED NOT NULL,
	MODIFY COLUMN `achievement_id` INT(10) UNSIGNED NOT NULL,
	MODIFY COLUMN `component_type` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	MODIFY COLUMN `component_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	MODIFY COLUMN `requested_value` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	MODIFY COLUMN `definition_version` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	MODIFY COLUMN `status` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	MODIFY COLUMN `attempt_count` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	MODIFY COLUMN `created_at` INT(10) UNSIGNED NOT NULL,
	MODIFY COLUMN `last_attempt_at` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	MODIFY COLUMN `last_error` VARCHAR(255) CHARACTER SET utf8mb4 NOT NULL DEFAULT '';

-- Recreate the pending-mutation lookup indexes when their shape is invalid.
SET @achievement_mutation_bad_indexes = (
	SELECT GROUP_CONCAT(
		CONCAT('DROP INDEX `', REPLACE(index_name, '`', '``'), '`')
		ORDER BY index_name SEPARATOR ', '
	)
	FROM (
		SELECT
			index_name,
			MAX(non_unique) AS non_unique,
			COUNT(*) AS column_count,
			COUNT(column_name) AS named_column_count,
			SUM(CASE WHEN sub_part IS NULL THEN 0 ELSE 1 END) AS prefix_count,
			GROUP_CONCAT(column_name ORDER BY seq_in_index SEPARATOR ',') AS key_columns
		FROM information_schema.statistics
		WHERE table_schema = DATABASE()
			AND table_name = 'character_achievement_pending_mutations'
		GROUP BY index_name
	) AS existing_indexes
	WHERE index_name <> 'PRIMARY'
		AND (
			non_unique = 0
			OR (
				index_name = 'character_achievement_pending_character'
				AND NOT (
					non_unique = 1
					AND column_count = 3
					AND named_column_count = column_count
					AND prefix_count = 0
					AND key_columns = 'character_id,status,mutation_id'
				)
			)
		)
);
SET @achievement_mutation_schema_sql = IF(
	@achievement_mutation_bad_indexes IS NULL,
	'SELECT 1',
	CONCAT(
		'ALTER TABLE `character_achievement_pending_mutations` ',
		@achievement_mutation_bad_indexes
	)
);
PREPARE achievement_mutation_schema_stmt FROM @achievement_mutation_schema_sql;
EXECUTE achievement_mutation_schema_stmt;
DEALLOCATE PREPARE achievement_mutation_schema_stmt;

SET @achievement_mutation_schema_sql = IF(
	EXISTS (
		SELECT 1 FROM information_schema.statistics
		WHERE table_schema = DATABASE()
			AND table_name = 'character_achievement_pending_mutations'
		GROUP BY index_name
		HAVING MAX(non_unique) = 1
			AND COUNT(*) = 3
			AND COUNT(column_name) = COUNT(*)
			AND SUM(CASE WHEN sub_part IS NULL THEN 0 ELSE 1 END) = 0
			AND GROUP_CONCAT(column_name ORDER BY seq_in_index SEPARATOR ',') =
				'character_id,status,mutation_id'
	),
	'SELECT 1',
	'ALTER TABLE `character_achievement_pending_mutations` ADD INDEX `character_achievement_pending_character` (`character_id`, `status`, `mutation_id`)'
);
PREPARE achievement_mutation_schema_stmt FROM @achievement_mutation_schema_sql;
EXECUTE achievement_mutation_schema_stmt;
DEALLOCATE PREPARE achievement_mutation_schema_stmt;

SET @achievement_mutation_bad_indexes = (
	SELECT GROUP_CONCAT(
		CONCAT('DROP INDEX `', REPLACE(index_name, '`', '``'), '`')
		ORDER BY index_name SEPARATOR ', '
	)
	FROM (
		SELECT
			index_name,
			MAX(non_unique) AS non_unique,
			COUNT(*) AS column_count,
			COUNT(column_name) AS named_column_count,
			SUM(CASE WHEN sub_part IS NULL THEN 0 ELSE 1 END) AS prefix_count,
			GROUP_CONCAT(column_name ORDER BY seq_in_index SEPARATOR ',') AS key_columns
		FROM information_schema.statistics
		WHERE table_schema = DATABASE()
			AND table_name = 'character_achievement_pending_mutations'
		GROUP BY index_name
	) AS existing_indexes
	WHERE index_name <> 'PRIMARY'
		AND (
			non_unique = 0
			OR (
				index_name = 'character_achievement_pending_status'
				AND NOT (
					non_unique = 1
					AND column_count = 2
					AND named_column_count = column_count
					AND prefix_count = 0
					AND key_columns = 'status,character_id'
				)
			)
		)
);
SET @achievement_mutation_schema_sql = IF(
	@achievement_mutation_bad_indexes IS NULL,
	'SELECT 1',
	CONCAT(
		'ALTER TABLE `character_achievement_pending_mutations` ',
		@achievement_mutation_bad_indexes
	)
);
PREPARE achievement_mutation_schema_stmt FROM @achievement_mutation_schema_sql;
EXECUTE achievement_mutation_schema_stmt;
DEALLOCATE PREPARE achievement_mutation_schema_stmt;

SET @achievement_mutation_schema_sql = IF(
	EXISTS (
		SELECT 1 FROM information_schema.statistics
		WHERE table_schema = DATABASE()
			AND table_name = 'character_achievement_pending_mutations'
		GROUP BY index_name
		HAVING MAX(non_unique) = 1
			AND COUNT(*) = 2
			AND COUNT(column_name) = COUNT(*)
			AND SUM(CASE WHEN sub_part IS NULL THEN 0 ELSE 1 END) = 0
			AND GROUP_CONCAT(column_name ORDER BY seq_in_index SEPARATOR ',') =
				'status,character_id'
	),
	'SELECT 1',
	'ALTER TABLE `character_achievement_pending_mutations` ADD INDEX `character_achievement_pending_status` (`status`, `character_id`)'
);
PREPARE achievement_mutation_schema_stmt FROM @achievement_mutation_schema_sql;
EXECUTE achievement_mutation_schema_stmt;
DEALLOCATE PREPARE achievement_mutation_schema_stmt;

SELECT `mutation_id`, `character_id`, `source_target_type`, `source_target_id`,
	`operation`, `achievement_id`, `component_type`, `component_id`,
	`requested_value`, `definition_version`, `status`, `attempt_count`,
	`created_at`, `last_attempt_at`, `last_error`
FROM `character_achievement_pending_mutations` LIMIT 0;
