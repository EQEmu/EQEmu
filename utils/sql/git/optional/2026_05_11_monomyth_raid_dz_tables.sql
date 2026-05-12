-- Monomyth Raid Dynamic Zone configuration tables

-- Raid DZ enabled zones
CREATE TABLE IF NOT EXISTS `monomyth_raid_dz_zones` (
  `zone_id`      SMALLINT UNSIGNED NOT NULL,
  `version`      SMALLINT UNSIGNED NOT NULL DEFAULT 0,
  `min_level`    TINYINT UNSIGNED NOT NULL DEFAULT 1,
  `max_level`    TINYINT UNSIGNED NOT NULL DEFAULT 65,
  `min_players`  TINYINT UNSIGNED NOT NULL DEFAULT 1,
  `max_players`  TINYINT UNSIGNED NOT NULL DEFAULT 54,
  `safe_return_zone_id` SMALLINT UNSIGNED NOT NULL DEFAULT 0,
  `safe_return_x`       FLOAT NOT NULL DEFAULT 0,
  `safe_return_y`       FLOAT NOT NULL DEFAULT 0,
  `safe_return_z`       FLOAT NOT NULL DEFAULT 0,
  `safe_return_heading` FLOAT NOT NULL DEFAULT 0,
  `zone_in_x`    FLOAT NOT NULL DEFAULT 0,
  `zone_in_y`    FLOAT NOT NULL DEFAULT 0,
  `zone_in_z`    FLOAT NOT NULL DEFAULT 0,
  `zone_in_heading` FLOAT NOT NULL DEFAULT 0,
  `compass_zone_id` SMALLINT UNSIGNED NOT NULL DEFAULT 0,
  `compass_x`   FLOAT NOT NULL DEFAULT 0,
  `compass_y`   FLOAT NOT NULL DEFAULT 0,
  `compass_z`   FLOAT NOT NULL DEFAULT 0,
  PRIMARY KEY (`zone_id`, `version`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Raid target NPC IDs guaranteed to be present in Raid DZ instances
-- (used by the raid DZ zone controller to verify/respawn targets)
CREATE TABLE IF NOT EXISTS `monomyth_raid_dz_targets` (
  `zone_id`     SMALLINT UNSIGNED NOT NULL,
  `npc_type_id` INT UNSIGNED NOT NULL,
  PRIMARY KEY (`zone_id`, `npc_type_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
