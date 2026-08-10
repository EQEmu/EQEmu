-- Character achievement state, reward delivery state, and queued mutations.

CREATE TABLE IF NOT EXISTS `character_achievements` (
	`character_id` INT(10) UNSIGNED NOT NULL,
	`achievement_id` INT(10) UNSIGNED NOT NULL,
	`version` INT(10) UNSIGNED NOT NULL DEFAULT 0,
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
	`version` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`updated_at` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	PRIMARY KEY (`character_id`, `achievement_id`, `component_type`, `component_id`),
	KEY `character_achievement_progress_achievement` (`achievement_id`, `component_type`, `component_sequence`, `component_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- status: 0=claimed/in-flight, 1=granted, 2=explicit delivery failure.
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

-- status: 0=pending/in progress, 1=fully granted, 2=retryable failure,
-- 3=ambiguous delivery.
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
	KEY `character_achievement_reward_selections_status` (`status`, `character_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `character_task_reward_instances` (
	`occurrence_id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
	`character_id` INT(10) UNSIGNED NOT NULL,
	`task_id` INT(10) UNSIGNED NOT NULL,
	`accepted_time` INT(10) UNSIGNED NOT NULL,
	PRIMARY KEY (`occurrence_id`),
	UNIQUE KEY `character_task_reward_instances_source` (`character_id`, `task_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- status: 0=pending/in progress, 1=fully granted, 2=retryable failure,
-- 3=ambiguous delivery.
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
	UNIQUE KEY `character_task_reward_selections_source` (`character_id`, `source_instance_id`),
	KEY `character_task_reward_selections_status` (`status`, `character_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

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
	`version` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`status` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	`attempt_count` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`created_at` INT(10) UNSIGNED NOT NULL,
	`last_attempt_at` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`last_error` VARCHAR(255) NOT NULL DEFAULT '',
	PRIMARY KEY (`mutation_id`),
	KEY `character_achievement_pending_character` (`character_id`, `status`, `mutation_id`),
	KEY `character_achievement_pending_status` (`status`, `character_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
