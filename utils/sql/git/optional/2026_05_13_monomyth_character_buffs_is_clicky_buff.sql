ALTER TABLE `character_buffs`
	ADD COLUMN `is_clicky_buff` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0 AFTER `group_raid_timer_pausable`;
