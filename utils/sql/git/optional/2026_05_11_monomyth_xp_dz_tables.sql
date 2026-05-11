-- Monomyth XP Dynamic Zone configuration tables

-- XP DZ enabled zones
CREATE TABLE IF NOT EXISTS `monomyth_xp_dz_zones` (
  `zone_id`      SMALLINT UNSIGNED NOT NULL,
  `version`      SMALLINT UNSIGNED NOT NULL DEFAULT 0,
  `min_level`    TINYINT UNSIGNED NOT NULL DEFAULT 1,
  `max_level`    TINYINT UNSIGNED NOT NULL DEFAULT 65,
  `min_players`  TINYINT UNSIGNED NOT NULL DEFAULT 1,
  `max_players`  TINYINT UNSIGNED NOT NULL DEFAULT 6,
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

-- Raid target NPC suppression list for XP DZ instances
CREATE TABLE IF NOT EXISTS `monomyth_xp_dz_raid_suppress` (
  `zone_id`     SMALLINT UNSIGNED NOT NULL,
  `npc_type_id` INT UNSIGNED NOT NULL,
  PRIMARY KEY (`zone_id`, `npc_type_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Insert default Monomyth DZ rule values if not present
INSERT IGNORE INTO `rule_values` (`rule_name`, `rule_value`, `notes`) VALUES
  ('Monomyth.DynamicZonesEnabled', '0', 'Enable Monomyth dynamic zone assignment systems.'),
  ('Monomyth.XPDZLockoutSeconds', '3600', 'Lockout in seconds for Monomyth XP dynamic zones.'),
  ('Monomyth.RaidDZLockoutSeconds', '86400', 'Lockout in seconds for Monomyth raid dynamic zones.'),
  ('Monomyth.XPDZLifetimeSeconds', '86400', 'Lifetime in seconds for Monomyth XP dynamic zones.'),
  ('Monomyth.RaidDZLifetimeSeconds', '86400', 'Lifetime in seconds for Monomyth raid dynamic zones.'),
  ('Monomyth.MaxActiveXPDZAssignments', '1', 'Maximum active Monomyth XP dynamic zone assignments per character.'),
  ('Monomyth.MaxActiveRaidDZAssignments', '1', 'Maximum active Monomyth raid dynamic zone assignments per character.');
