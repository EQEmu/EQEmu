-- AoTv4 region-locked progression
-- ============================================================================================
-- Zones are grouped into REGIONS. A character may only enter a zone whose region they have
-- unlocked, and a region carries a max_level which is the level ceiling while that region is the
-- best one you hold.
--
-- NOTE ON zone_regions.zone_id: this holds the runtime zone id (zone.zoneidnumber), NOT zone.id.
-- A hard FOREIGN KEY to zone.id is not possible and would be wrong here: `zone` has one row per
-- zone VERSION, so zoneidnumber is not unique in it (618 rows, ids 1..999). The zoning code checks
-- the id the client is travelling to, which is zoneidnumber, so that is what has to be stored.
-- The region_id side IS a real foreign key.

CREATE TABLE IF NOT EXISTS `regions` (
  `id`        INT UNSIGNED     NOT NULL AUTO_INCREMENT,
  `name`      VARCHAR(64)      NOT NULL DEFAULT '',
  `max_level` SMALLINT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `zone_regions` (
  `zone_id`   INT UNSIGNED NOT NULL COMMENT 'zone.zoneidnumber, not zone.id',
  `region_id` INT UNSIGNED NOT NULL,
  `name`      VARCHAR(64)  NOT NULL DEFAULT '' COMMENT 'zone short name, for readability only',
  PRIMARY KEY (`zone_id`, `region_id`),
  KEY `idx_zone_regions_region` (`region_id`),
  CONSTRAINT `fk_zone_regions_region`
    FOREIGN KEY (`region_id`) REFERENCES `regions` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `character_regions_unlocked` (
  `region_id`   INT UNSIGNED NOT NULL,
  `char_id`     INT UNSIGNED NOT NULL,
  `char_name`   VARCHAR(64)  NOT NULL DEFAULT '' COMMENT 'debugging only',
  `region_name` VARCHAR(64)  NOT NULL DEFAULT '' COMMENT 'debugging only',
  PRIMARY KEY (`region_id`, `char_id`),
  KEY `idx_character_regions_char` (`char_id`),
  CONSTRAINT `fk_character_regions_region`
    FOREIGN KEY (`region_id`) REFERENCES `regions` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
