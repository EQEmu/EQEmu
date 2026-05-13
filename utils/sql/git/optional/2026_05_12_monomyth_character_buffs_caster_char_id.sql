ALTER TABLE `character_buffs`
	ADD COLUMN `caster_char_id` INT(10) UNSIGNED NOT NULL DEFAULT 0 AFTER `caster_level`;
