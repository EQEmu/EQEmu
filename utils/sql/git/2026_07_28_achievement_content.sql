-- Achievement definitions and selectable reward metadata.

CREATE TABLE IF NOT EXISTS `achievement_categories` (
	`id` INT(10) UNSIGNED NOT NULL,
	`parent_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`sequence` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`name` VARCHAR(255) NOT NULL DEFAULT '',
	`description` TEXT NOT NULL,
	`icon` VARCHAR(255) NOT NULL DEFAULT '',
	PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `achievements` (
	`id` INT(10) UNSIGNED NOT NULL,
	`name` VARCHAR(255) NOT NULL DEFAULT '',
	`description` TEXT NOT NULL,
	`icon_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`points` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`has_reward` TINYINT(1) UNSIGNED NOT NULL DEFAULT 0,
	`client_flag` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	`version` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`reset_on_version_change` TINYINT(1) UNSIGNED NOT NULL DEFAULT 1,
	`enabled` TINYINT(1) UNSIGNED NOT NULL DEFAULT 1,
	PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `achievement_category_associations` (
	`category_id` INT(10) UNSIGNED NOT NULL,
	`sequence` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`achievement_id` INT(10) UNSIGNED NOT NULL,
	`display_text` VARCHAR(255) NOT NULL DEFAULT '',
	PRIMARY KEY (`category_id`, `achievement_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `achievement_components` (
	`achievement_id` INT(10) UNSIGNED NOT NULL,
	`component_type` TINYINT(3) UNSIGNED NOT NULL,
	`sequence` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`component_id` INT(10) UNSIGNED NOT NULL,
	`name` TEXT NOT NULL,
	`description` TEXT NOT NULL,
	PRIMARY KEY (`achievement_id`, `component_type`, `component_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `achievement_associations` (
	`component_id` INT(10) UNSIGNED NOT NULL,
	`required_count` INT(10) UNSIGNED NOT NULL DEFAULT 1,
	PRIMARY KEY (`component_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Server-authored event bindings. Numeric enum values are defined in
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
	`required_count` INT(10) UNSIGNED NOT NULL DEFAULT 1,
	`enabled` TINYINT(1) UNSIGNED NOT NULL DEFAULT 1,
	PRIMARY KEY (`id`),
	UNIQUE KEY `achievement_criteria_definition` (`achievement_id`, `component_type`, `component_id`, `event_type`, `target_id`, `target_id2`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- All rows for one restriction_id must pass.
CREATE TABLE IF NOT EXISTS `achievement_cast_requirements` (
	`restriction_id` INT(10) UNSIGNED NOT NULL,
	`achievement_id` INT(10) UNSIGNED NOT NULL,
	`requires_completed` TINYINT(1) UNSIGNED NOT NULL DEFAULT 1,
	PRIMARY KEY (`restriction_id`, `achievement_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Reward definitions are provider-independent. reward_sources links a
-- selectable set to an achievement, task, or future source. Automatic rewards
-- use reward_source_entries instead.
CREATE TABLE IF NOT EXISTS `reward_sets` (
	`reward_set_id` INT(10) UNSIGNED NOT NULL,
	`title` VARCHAR(255) NOT NULL DEFAULT '',
	`enabled` TINYINT(1) UNSIGNED NOT NULL DEFAULT 1,
	PRIMARY KEY (`reward_set_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `reward_options` (
	`reward_set_id` INT(10) UNSIGNED NOT NULL,
	`option_id` INT(10) UNSIGNED NOT NULL,
	`sequence` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`label` VARCHAR(255) NOT NULL DEFAULT '',
	`common_to_all` TINYINT(1) UNSIGNED NOT NULL DEFAULT 0,
	`flags` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	`enabled` TINYINT(1) UNSIGNED NOT NULL DEFAULT 1,
	PRIMARY KEY (`reward_set_id`, `option_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `rewards` (
	`reward_id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`reward_type` TINYINT(3) UNSIGNED NOT NULL,
	`reward_data_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`amount` BIGINT(20) UNSIGNED NOT NULL DEFAULT 1,
	`description` VARCHAR(255) NOT NULL DEFAULT '',
	`enabled` TINYINT(1) UNSIGNED NOT NULL DEFAULT 1,
	PRIMARY KEY (`reward_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `reward_option_entries` (
	`reward_set_id` INT(10) UNSIGNED NOT NULL,
	`option_id` INT(10) UNSIGNED NOT NULL,
	`sequence` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`reward_id` INT(10) UNSIGNED NOT NULL,
	PRIMARY KEY (`reward_set_id`, `option_id`, `reward_id`),
	UNIQUE KEY `reward_option_entries_set_reward` (`reward_set_id`, `reward_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `reward_sources` (
	`source_type` TINYINT(3) UNSIGNED NOT NULL,
	`source_id` BIGINT(20) UNSIGNED NOT NULL,
	`reward_set_id` INT(10) UNSIGNED NOT NULL,
	`enabled` TINYINT(1) UNSIGNED NOT NULL DEFAULT 1,
	PRIMARY KEY (`source_type`, `source_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `reward_source_entries` (
	`source_type` TINYINT(3) UNSIGNED NOT NULL,
	`source_id` BIGINT(20) UNSIGNED NOT NULL,
	`sequence` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`reward_id` INT(10) UNSIGNED NOT NULL,
	PRIMARY KEY (`source_type`, `source_id`, `reward_id`),
	UNIQUE KEY `reward_source_entries_source_sequence` (`source_type`, `source_id`, `sequence`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
