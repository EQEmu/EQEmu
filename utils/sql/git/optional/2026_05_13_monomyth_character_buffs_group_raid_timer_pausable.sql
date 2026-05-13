ALTER TABLE `character_buffs`
	ADD COLUMN `group_raid_timer_pausable` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0 AFTER `instrument_mod`;
