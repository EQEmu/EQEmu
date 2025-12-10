#pragma once

std::vector<ManifestEntry> manifest_entries = {
	ManifestEntry{
		.version = 9000,
		.description = "2013_02_18_merc_rules_and_tables.sql",
		.check = "SELECT * FROM `rule_values` WHERE `rule_name` LIKE '%Mercs:ResurrectRadius%'",
		.condition = "empty",
		.match = "",
		.sql = R"(
INSERT INTO `rule_values` (`ruleset_id`, `rule_name`, `rule_value`, `notes`) VALUES (1, 'Mercs:ResurrectRadius', '50', 'Determines the distance from which a healer merc will attempt to resurrect a corpse');

DROP TABLE IF EXISTS mercsbuffs;
DROP TABLE IF EXISTS mercs;

CREATE TABLE  mercs (
	MercID 				int(10) unsigned NOT NULL AUTO_INCREMENT,
	OwnerCharacterID 	int(10) unsigned NOT NULL,
	Slot 	    		tinyint(1) unsigned NOT NULL DEFAULT '0',
	Name 				varchar(64) NOT NULL,
	TemplateID			int(10) unsigned NOT NULL DEFAULT '0',
	SuspendedTime		int(11) unsigned NOT NULL DEFAULT '0',
	IsSuspended			tinyint(1) unsigned NOT NULL default '0',
	TimerRemaining		int(11) unsigned NOT NULL DEFAULT '0',
	Gender 				tinyint unsigned NOT NULL DEFAULT '0',
	StanceID			tinyint unsigned NOT NULL DEFAULT '0',
	HP 					int(11) unsigned NOT NULL DEFAULT '0',
	Mana 				int(11) unsigned NOT NULL DEFAULT '0',
	Endurance			int(11) unsigned NOT NULL DEFAULT '0',
	Face 				int(10) unsigned NOT NULL DEFAULT '1',
	LuclinHairStyle 	int(10) unsigned NOT NULL DEFAULT '1',
	LuclinHairColor 	int(10) unsigned NOT NULL DEFAULT '1',
	LuclinEyeColor 		int(10) unsigned NOT NULL DEFAULT '1',
	LuclinEyeColor2 	int(10) unsigned NOT NULL DEFAULT '1',
	LuclinBeardColor 	int(10) unsigned NOT NULL DEFAULT '1',
	LuclinBeard 		int(10) unsigned NOT NULL DEFAULT '0',
	DrakkinHeritage 	int(10) unsigned NOT NULL DEFAULT '0',
	DrakkinTattoo 		int(10) unsigned NOT NULL DEFAULT '0',
	DrakkinDetails 		int(10) unsigned NOT NULL DEFAULT '0',
	PRIMARY KEY (MercID)
);

CREATE TABLE mercbuffs (
	MercBuffId        	int(10) unsigned NOT NULL auto_increment,
	MercId            	int(10) unsigned NOT NULL default '0',
	SpellId           	int(10) unsigned NOT NULL default '0',
	CasterLevel       	int(10) unsigned NOT NULL default '0',
	DurationFormula   	int(10) unsigned NOT NULL default '0',
	TicsRemaining     	int(11) unsigned NOT NULL default '0',
	PoisonCounters    	int(11) unsigned NOT NULL default '0',
	DiseaseCounters   	int(11) unsigned NOT NULL default '0',
	CurseCounters     	int(11) unsigned NOT NULL default '0',
	CorruptionCounters 	int(11) unsigned NOT NULL default '0',
	HitCount          	int(10) unsigned NOT NULL default '0',
	MeleeRune         	int(10) unsigned NOT NULL default '0',
	MagicRune         	int(10) unsigned NOT NULL default '0',
	DeathSaveSuccessChance int(10) unsigned NOT NULL default '0',
	CasterAARank      	int(10) unsigned NOT NULL default '0',
	Persistent        	tinyint(1) NOT NULL default '0',
	PRIMARY KEY  (MercBuffId),
	KEY FK_mercbuff_1 (MercId),
	CONSTRAINT FK_mercbuff_1 FOREIGN KEY (MercId) REFERENCES mercs (MercID)
);
)",
	},
	ManifestEntry{
		.version = 9001,
		.description = "2013_02_25_impr_ht_lt.sql",
		.check = "SHOW TABLES LIKE 'merc_inventory'",
		.condition = "empty",
		.match = "",
		.sql = R"(
/* SK AA Touch of the Wicked should reduce reuse timers for */
/* Improved Harm Touch & Leech Touch as well as regular HT */
update aa_actions set redux_aa=596, redux_rate=17 where aaid=207;
update aa_actions set redux_aa=596, redux_rate=17 where aaid=208;

)",
	},
	ManifestEntry{
		.version = 9002,
		.description = "2013_03_1_merc_rules_and_equipment.sql",
		.check = "SHOW TABLES LIKE 'merc_inventory'",
		.condition = "empty",
		.match = "",
		.sql = R"(
INSERT INTO `rule_values` (`ruleset_id`, `rule_name`, `rule_value`, `notes`) VALUES (1, 'Mercs:ChargeMercPurchaseCost', 'false', 'Turns Mercenary purchase costs on or off.');
INSERT INTO `rule_values` (`ruleset_id`, `rule_name`, `rule_value`, `notes`) VALUES (1, 'Mercs:ChargeMercUpkeepCost', 'false', 'Turns Mercenary upkeep costs on or off.');

UPDATE merc_stats SET spellscale = 100, healscale = 100;

ALTER TABLE mercbuffs RENAME TO  merc_buffs;

DROP TABLE IF EXISTS merc_inventory;

CREATE TABLE merc_inventory (
	merc_inventory_id      	int(10) unsigned NOT NULL auto_increment,
	merc_subtype_id       	int(10) unsigned NOT NULL default '0',
	item_id           	int(11) unsigned NOT NULL default '0',
	min_level       	int(10) unsigned NOT NULL default '0',
	max_level   		int(10) unsigned NOT NULL default '0',
	PRIMARY KEY  (merc_inventory_id),
	KEY FK_merc_inventory_1 (merc_subtype_id),
	CONSTRAINT FK_merc_inventory_1 FOREIGN KEY (merc_subtype_id) REFERENCES merc_subtypes (merc_subtype_id)
);

)",
	},
	ManifestEntry{
		.version = 9005,
		.description = "2013_04_08_salvage.sql",
		.check = "SHOW COLUMNS FROM `tradeskill_recipe_entries` LIKE 'salvagecount'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- Add row to tre
ALTER TABLE `tradeskill_recipe_entries` ADD `salvagecount` tinyint(2) DEFAULT '0' NOT NULL AFTER `componentcount`;

-- Fix level req on Salvage
UPDATE `altadv_vars` SET `level_inc` = '5' WHERE `skill_id` = '997';

-- Set aa_effects for Salvage
INSERT INTO `aa_effects` (`id`, `aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('2374', '997', '1', '313', '5', '0');
INSERT INTO `aa_effects` (`id`, `aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('2375', '998', '1', '313', '15', '0');
INSERT INTO `aa_effects` (`id`, `aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('2376', '999', '1', '313', '25', '0');


)",
	},
	ManifestEntry{
		.version = 9006,
		.description = "2013_05_05_account_flags.sql",
		.check = "SHOW TABLES LIKE 'account_flags'",
		.condition = "empty",
		.match = "",
		.sql = R"(
--
-- Table structure for table `account_flags`
--

CREATE TABLE IF NOT EXISTS `account_flags` (
  `p_accid` int(10) unsigned NOT NULL,
  `p_flag` varchar(50) NOT NULL,
  `p_value` varchar(80) NOT NULL,
  PRIMARY KEY (`p_accid`,`p_flag`),
  KEY `p_accid` (`p_accid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

)",
	},
	ManifestEntry{
		.version = 9007,
		.description = "2013_05_05_item_tick.sql",
		.check = "SHOW TABLES LIKE 'item_tick'",
		.condition = "empty",
		.match = "",
		.sql = R"(
--
-- Table structure for table `item_tick`
--

CREATE TABLE IF NOT EXISTS `item_tick` (
  `it_itemid` int(11) NOT NULL,
  `it_chance` int(11) NOT NULL,
  `it_level` int(11) NOT NULL,
  `it_id` int(11) NOT NULL AUTO_INCREMENT,
  `it_qglobal` varchar(50) NOT NULL,
  `it_bagslot` tinyint(4) NOT NULL,
  PRIMARY KEY (`it_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1;

)",
	},
	ManifestEntry{
		.version = 9008,
		.description = "2013_07_11_npc_special_abilities.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'special_abilities'",
		.condition = "empty",
		.match = "",
		.sql =R"(
ALTER TABLE `npc_types`  ADD COLUMN `special_abilities` TEXT NULL AFTER `npcspecialattks`;
ALTER TABLE `npc_types` MODIFY COLUMN `special_abilities` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL;

UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "1,1^") WHERE npcspecialattks LIKE BINARY '%S%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "2,1^") WHERE npcspecialattks LIKE BINARY '%E%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "3,1^") WHERE npcspecialattks LIKE BINARY '%R%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "4,1^") WHERE npcspecialattks LIKE BINARY '%r%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "5,1^") WHERE npcspecialattks LIKE BINARY '%F%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "6,1^") WHERE npcspecialattks LIKE BINARY '%T%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "7,1^") WHERE npcspecialattks LIKE BINARY '%Q%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "8,1^") WHERE npcspecialattks LIKE BINARY '%L%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "9,1^") WHERE npcspecialattks LIKE BINARY '%b%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "10,1^") WHERE npcspecialattks LIKE BINARY '%m%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "11,1^") WHERE npcspecialattks LIKE BINARY '%Y%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "12,1^") WHERE npcspecialattks LIKE BINARY '%U%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "13,1^") WHERE npcspecialattks LIKE BINARY '%M%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "14,1^") WHERE npcspecialattks LIKE BINARY '%C%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "15,1^") WHERE npcspecialattks LIKE BINARY '%N%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "16,1^") WHERE npcspecialattks LIKE BINARY '%I%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "17,1^") WHERE npcspecialattks LIKE BINARY '%D%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "18,1^") WHERE npcspecialattks LIKE BINARY '%K%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "19,1^") WHERE npcspecialattks LIKE BINARY '%A%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "20,1^") WHERE npcspecialattks LIKE BINARY '%B%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "21,1^") WHERE npcspecialattks LIKE BINARY '%f%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "22,1^") WHERE npcspecialattks LIKE BINARY '%O%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "23,1^") WHERE npcspecialattks LIKE BINARY '%W%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "24,1^") WHERE npcspecialattks LIKE BINARY '%H%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "25,1^") WHERE npcspecialattks LIKE BINARY '%G%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "26,1^") WHERE npcspecialattks LIKE BINARY '%g%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "27,1^") WHERE npcspecialattks LIKE BINARY '%d%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "28,1^") WHERE npcspecialattks LIKE BINARY '%i%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "29,1^") WHERE npcspecialattks LIKE BINARY '%t%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "30,1^") WHERE npcspecialattks LIKE BINARY '%n%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "31,1^") WHERE npcspecialattks LIKE BINARY '%p%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "32,1^") WHERE npcspecialattks LIKE BINARY '%J%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "33,1^") WHERE npcspecialattks LIKE BINARY '%j%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "34,1^") WHERE npcspecialattks LIKE BINARY '%o%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "35,1^") WHERE npcspecialattks LIKE BINARY '%Z%';
UPDATE npc_types SET special_abilities = TRIM(TRAILING '^' FROM special_abilities);

ALTER TABLE `npc_types`  DROP COLUMN `npcspecialattks`;
)",
	},
	ManifestEntry{
		.version = 9009,
		.description = "2013_10_12_merc_special_abilities.sql",
		.check = "SHOW COLUMNS FROM `merc_stats` LIKE 'special_abilities'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `merc_stats`  ADD COLUMN `special_abilities` TEXT NULL AFTER `specialattks`;
ALTER TABLE `merc_stats` MODIFY COLUMN `special_abilities`  text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL;

UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "1,1^") WHERE specialattks LIKE BINARY '%S%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "2,1^") WHERE specialattks LIKE BINARY '%E%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "3,1^") WHERE specialattks LIKE BINARY '%R%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "4,1^") WHERE specialattks LIKE BINARY '%r%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "5,1^") WHERE specialattks LIKE BINARY '%F%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "6,1^") WHERE specialattks LIKE BINARY '%T%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "7,1^") WHERE specialattks LIKE BINARY '%Q%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "8,1^") WHERE specialattks LIKE BINARY '%L%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "9,1^") WHERE specialattks LIKE BINARY '%b%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "10,1^") WHERE specialattks LIKE BINARY '%m%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "11,1^") WHERE specialattks LIKE BINARY '%Y%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "12,1^") WHERE specialattks LIKE BINARY '%U%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "13,1^") WHERE specialattks LIKE BINARY '%M%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "14,1^") WHERE specialattks LIKE BINARY '%C%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "15,1^") WHERE specialattks LIKE BINARY '%N%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "16,1^") WHERE specialattks LIKE BINARY '%I%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "17,1^") WHERE specialattks LIKE BINARY '%D%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "18,1^") WHERE specialattks LIKE BINARY '%K%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "19,1^") WHERE specialattks LIKE BINARY '%A%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "20,1^") WHERE specialattks LIKE BINARY '%B%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "21,1^") WHERE specialattks LIKE BINARY '%f%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "22,1^") WHERE specialattks LIKE BINARY '%O%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "23,1^") WHERE specialattks LIKE BINARY '%W%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "24,1^") WHERE specialattks LIKE BINARY '%H%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "25,1^") WHERE specialattks LIKE BINARY '%G%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "26,1^") WHERE specialattks LIKE BINARY '%g%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "27,1^") WHERE specialattks LIKE BINARY '%d%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "28,1^") WHERE specialattks LIKE BINARY '%i%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "29,1^") WHERE specialattks LIKE BINARY '%t%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "30,1^") WHERE specialattks LIKE BINARY '%n%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "31,1^") WHERE specialattks LIKE BINARY '%p%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "32,1^") WHERE specialattks LIKE BINARY '%J%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "33,1^") WHERE specialattks LIKE BINARY '%j%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "34,1^") WHERE specialattks LIKE BINARY '%o%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "35,1^") WHERE specialattks LIKE BINARY '%Z%';
UPDATE merc_stats SET special_abilities = TRIM(TRAILING '^' FROM special_abilities);

ALTER TABLE `merc_stats`  DROP COLUMN `specialattks`;
)",
	},
	ManifestEntry{
		.version = 9011,
		.description = "2013_10_31_recipe_disabling.sql",
		.check = "SHOW COLUMNS FROM `tradeskill_recipe` LIKE 'enabled'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `tradeskill_recipe` ADD `enabled` tinyint(1) NOT NULL DEFAULT '1';

)",
	},
	ManifestEntry{
		.version = 9014,
		.description = "2013_11_18_assistradius.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'assistradius'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE  `npc_types` ADD  `assistradius` INT( 10 ) UNSIGNED NOT NULL DEFAULT  '0' AFTER `aggroradius`;

)",
	},
	ManifestEntry{
		.version = 9015,
		.description = "2013_12_26_merchantlist_class_required.sql",
		.check = "SHOW COLUMNS FROM `merchantlist` LIKE 'classes_required'",
		.condition = "empty",
		.match = "",
		.sql = R"(
 ALTER TABLE `merchantlist` ADD COLUMN `classes_required` INT(11) NOT NULL DEFAULT '65535';


)",
	},
	ManifestEntry{
		.version = 9017,
		.description = "2014_01_08_spellsnewadditions.sql",
		.check = "SHOW COLUMNS FROM `spells_new` LIKE 'persistdeath'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `spells_new` CHANGE `field200` `suspendable` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field202` `songcap` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `field215` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `field216` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `field217` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `field218` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `maxtargets` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `field220` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `field221` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `field222` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `field223` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `persistdeath` INT(11) DEFAULT '0';

)",
	},
	ManifestEntry{
		.version = 9018,
		.description = "2014_01_09_preservepetsize.sql",
		.check = "SHOW COLUMNS FROM `character_pet_info` LIKE 'size'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `character_pet_info` ADD `size` FLOAT NOT NULL DEFAULT '0';

)",
	},
	ManifestEntry{
		.version = 9020,
		.description = "2014_01_20_not_extendable.sql",
		.check = "SHOW COLUMNS FROM `spells_new` LIKE 'not_extendable'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `spells_new` CHANGE `field197` `not_extendable` INT(11) NOT NULL DEFAULT '0';

)",
	},
	ManifestEntry{
		.version = 9022,
		.description = "2014_01_20_weather.sql",
		.check = "SHOW COLUMNS FROM `zone` LIKE 'rain_chance1'",
		.condition = "empty",
		.match = "",
		.sql = R"(
alter table zone drop column `weather`;
alter table zone add column `rain_chance1` int(4) not null default 0;
alter table zone add column `rain_chance2` int(4) not null default 0;
alter table zone add column `rain_chance3` int(4) not null default 0;
alter table zone add column `rain_chance4` int(4) not null default 0;
alter table zone add column `rain_duration1` int(4) not null default 0;
alter table zone add column `rain_duration2` int(4) not null default 0;
alter table zone add column `rain_duration3` int(4) not null default 0;
alter table zone add column `rain_duration4` int(4) not null default 0;
alter table zone add column `snow_chance1` int(4) not null default 0;
alter table zone add column `snow_chance2` int(4) not null default 0;
alter table zone add column `snow_chance3` int(4) not null default 0;
alter table zone add column `snow_chance4` int(4) not null default 0;
alter table zone add column `snow_duration1` int(4) not null default 0;
alter table zone add column `snow_duration2` int(4) not null default 0;
alter table zone add column `snow_duration3` int(4) not null default 0;
alter table zone add column `snow_duration4` int(4) not null default 0;
)",
	},
	ManifestEntry{
		.version = 9025,
		.description = "2014_02_13_rename_instance_lockout_tables.sql",
		.check = "SHOW TABLES LIKE 'instance_list'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- rename the instance_lockout tables to instance_list. They have nothing to do with lockouts.
ALTER TABLE `instance_lockout` RENAME TO  `instance_list` ;
ALTER TABLE `instance_lockout_player` RENAME TO  `instance_list_player` ;
)",
	},
	ManifestEntry{
		.version = 9026,
		.description = "2014_02_13_spells_new_update.sql",
		.check = "SHOW COLUMNS FROM `spells_new` LIKE 'ConeStartAngle'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `spells_new` CHANGE `field161` `not_reflectable` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field151` `no_partial_resist` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field189` `MinResist` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field190` `MaxResist` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field194` `ConeStartAngle` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field195` `ConeStopAngle` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field208` `rank` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field159` `npc_no_los` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field213` `NotOutofCombat` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field214` `NotInCombat` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field168` `IsDiscipline` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field211` `CastRestriction` INT(11) NOT NULL DEFAULT '0';

UPDATE altadv_vars SET sof_next_id = 8261 WHERE skill_id = 8232;
UPDATE altadv_vars SET sof_next_id = 0 WHERE skill_id = 8261;
UPDATE altadv_vars SET sof_current_level = 3 WHERE skill_id = 8261;

)",
	},
	ManifestEntry{
		.version = 9027,
		.description = "2014_02_20_buff_update.sql",
		.check = "SHOW COLUMNS FROM `character_buffs` LIKE 'caston_y'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- UPDATE BUFF TABLES
ALTER TABLE `character_buffs` CHANGE `death_save_chance` `dot_rune` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `merc_buffs` CHANGE `DeathSaveSuccessChance` `dot_rune` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `botbuffs` CHANGE `DeathSaveSuccessChance` `dot_rune` INT(10) NOT NULL DEFAULT '0';

ALTER TABLE `character_buffs` CHANGE `death_save_aa_chance` `caston_x` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `merc_buffs` CHANGE `CasterAARank` `caston_x` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `botbuffs` CHANGE `CasterAARank` `caston_x` INT(10) NOT NULL DEFAULT '0';

ALTER TABLE `character_buffs` ADD `caston_y` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `merc_buffs` ADD `caston_y` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `botbuffs` ADD `caston_y` INT(10) NOT NULL DEFAULT '0';

ALTER TABLE `character_buffs` ADD `caston_z` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `merc_buffs` ADD `caston_z` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `botbuffs` ADD `caston_z` INT(10) NOT NULL DEFAULT '0';

ALTER TABLE `character_buffs` ADD `ExtraDIChance` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `merc_buffs` ADD `ExtraDIChance` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `botbuffs` ADD `ExtraDIChance` INT(10) NOT NULL DEFAULT '0';

ALTER TABLE `spells_new` CHANGE `not_reflectable` `reflectable` INT(11) NOT NULL DEFAULT '0';
)",
	},
	ManifestEntry{
		.version = 9028,
		.description = "2014_02_26_roambox_update.sql",
		.check = "SHOW COLUMNS FROM `spawngroup` LIKE 'mindelay'",
		.condition = "empty",
		.match = "",
		.sql = R"(
alter table `spawngroup` add column `mindelay` int(11) not null default 15000 AFTER delay;
alter table `spawngroup` change `delay` `delay` int(11) not null default 45000;
)",
	},
	ManifestEntry{
		.version = 9030,
		.description = "2014_04_04_physicalresist.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'PhR'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE  `npc_types` ADD  `PhR` smallint( 5 ) UNSIGNED NOT NULL DEFAULT  '0' AFTER `Corrup`;

-- Approximate baseline live npc values based on extensive parsing.
UPDATE npc_types SET PhR = 10 WHERE PhR = 0 AND level <= 50;
UPDATE npc_types SET PhR = (10 + (level - 50))  WHERE PhR = 0 AND (level > 50 AND level <= 60);
UPDATE npc_types SET PhR = (20 + ((level - 60)*4))  WHERE PhR = 0 AND level > 60;

)",
	},
	ManifestEntry{
		.version = 9031,
		.description = "2014_04_10_no_target_with_hotkey.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'no_target_hotkey'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE  `npc_types` ADD  `no_target_hotkey` tinyint( 1 ) UNSIGNED NOT NULL DEFAULT  '0' AFTER `healscale`;

)",
	},
	ManifestEntry{
		.version = 9032,
		.description = "2014_04_12_slowmitigation.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'slow_mitigation'",
		.condition = "contains",
		.match = "float",
		.sql = R"(
-- Convert all values from FLOAT to INT
UPDATE npc_types SET slow_mitigation = slow_mitigation * 100;

-- Change variable type from FLOAT TO INT
ALTER TABLE npc_types MODIFY slow_mitigation smallint(4) NOT NULL DEFAULT  '0';



)",
	},
	ManifestEntry{
		.version = 9034,
		.description = "2014_04_25_spawn_events.sql",
		.check = "SHOW COLUMNS FROM `spawn_events` LIKE 'strict'",
		.condition = "empty",
		.match = "",
		.sql = R"(
alter table spawn_events add column `strict` tinyint(4) not null default 0;
)",
	},
	ManifestEntry{
		.version = 9035,
		.description = "2014_04_27_aispelleffects.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'npc_spells_effects_id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- Note: The data entered into the new table are only examples and can be deleted/modified as needed.

ALTER TABLE  `npc_types` ADD  `npc_spells_effects_id` int( 11 ) UNSIGNED NOT NULL DEFAULT  '0' AFTER `npc_spells_id`;

SET FOREIGN_KEY_CHECKS=0;

DROP TABLE IF EXISTS `npc_spells_effects`;
CREATE TABLE `npc_spells_effects` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` tinytext,
  `parent_list` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1080 DEFAULT CHARSET=latin1;


SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `npc_spells_effects_entries`
-- ----------------------------
DROP TABLE IF EXISTS `npc_spells_effects_entries`;
CREATE TABLE `npc_spells_effects_entries` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `npc_spells_effects_id` int(11) NOT NULL DEFAULT '0',
  `spell_effect_id` smallint(5) NOT NULL DEFAULT '0',
  `minlevel` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `maxlevel` tinyint(3) unsigned NOT NULL DEFAULT '255',
  `se_base` int(11) NOT NULL DEFAULT '0',
  `se_limit` int(11) NOT NULL DEFAULT '0',
  `se_max` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `spellsid_spellid` (`npc_spells_effects_id`,`spell_effect_id`)
) ENGINE=InnoDB AUTO_INCREMENT=18374 DEFAULT CHARSET=latin1;

)",
	},
	ManifestEntry{
		.version = 9036,
		.description = "2014_05_04_slowmitigationfix.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'slow_mitigation'",
		.condition = "contains",
		.match = "float",
		.sql = R"(
ALTER TABLE npc_types MODIFY slow_mitigation smallint(4) NOT NULL DEFAULT  '0';
)",
	},
	ManifestEntry{
		.version = 9040,
		.description = "2014_07_10_npc_spells.sql",
		.check = "SHOW COLUMNS FROM `npc_spells` LIKE 'engaged_no_sp_recast_min'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- npc_types
ALTER TABLE  `npc_types` ADD  `ammo_idfile` varchar( 30 ) NOT NULL DEFAULT  'IT10' AFTER `d_meele_texture2`;
ALTER TABLE  `npc_types` ADD  `ranged_type` tinyint( 4 ) UNSIGNED NOT NULL DEFAULT  '7' AFTER `sec_melee_type`;
ALTER TABLE  `npc_types` ADD  `Avoidance` mediumint(9) UNSIGNED NOT NULL DEFAULT  '0' AFTER `Accuracy`;

-- npc spells
ALTER TABLE  `npc_spells` ADD  `range_proc` smallint(5) NOT NULL DEFAULT '-1';
ALTER TABLE  `npc_spells` ADD  `rproc_chance` smallint(5) NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD  `defensive_proc` smallint(5) NOT NULL DEFAULT '-1';
ALTER TABLE  `npc_spells` ADD  `dproc_chance` smallint(5) NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD  `fail_recast` int(11) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD `engaged_no_sp_recast_min` int(11) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD `engaged_no_sp_recast_max` int(11) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD `engaged_b_self_chance` tinyint(3) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD  `engaged_b_other_chance` tinyint(3) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD `engaged_d_chance` tinyint(3) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD  `pursue_no_sp_recast_min` int(3) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD  `pursue_no_sp_recast_max` int(11) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD  `pursue_d_chance` tinyint(3) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD  `idle_no_sp_recast_min` int(11) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD  `idle_no_sp_recast_max` int(11) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD  `idle_b_chance` tinyint(11) unsigned NOT NULL DEFAULT '0';
)",
	},
	ManifestEntry{
		.version = 9041,
		.description = "2014_08_02_spells_new.sql",
		.check = "SHOW COLUMNS FROM `spells_new` LIKE 'viral_range'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- spells new talbe update
ALTER TABLE `spells_new` CHANGE `NotOutofCombat` `InCombat` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `NotInCombat` `OutofCombat` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field201` `viral_range` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field218` `aemaxtargets` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE  `spells_new` ADD  `field225` int( 11 ) NOT NULL DEFAULT  '0' AFTER `persistdeath`;
ALTER TABLE  `spells_new` ADD  `field226` int( 11 ) NOT NULL DEFAULT  '0' AFTER `field225`;
ALTER TABLE  `spells_new` ADD  `min_dist` float( 0 ) NOT NULL DEFAULT  '0' AFTER `field226`;
ALTER TABLE  `spells_new` ADD  `min_dist_mod` float( 0 ) NOT NULL DEFAULT  '0' AFTER `min_dist`;
ALTER TABLE  `spells_new` ADD  `max_dist` float( 0 ) NOT NULL DEFAULT  '0' AFTER `min_dist_mod`;
ALTER TABLE  `spells_new` ADD  `max_dist_mod` float( 0 ) NOT NULL DEFAULT  '0' AFTER `max_dist`;
ALTER TABLE  `spells_new` ADD  `min_range` int( 11 ) NOT NULL DEFAULT  '0' AFTER `max_dist_mod`;
ALTER TABLE  `spells_new` ADD  `field232` int( 11 ) NOT NULL DEFAULT  '0' AFTER `min_range`;
ALTER TABLE  `spells_new` ADD  `field233` int( 11 ) NOT NULL DEFAULT  '0' AFTER `field232`;
ALTER TABLE  `spells_new` ADD  `field234` int( 11 ) NOT NULL DEFAULT  '0' AFTER `field233`;
ALTER TABLE  `spells_new` ADD  `field235` int( 11 ) NOT NULL DEFAULT  '0' AFTER `field234`;
ALTER TABLE  `spells_new` ADD  `field236` int( 11 ) NOT NULL DEFAULT  '0' AFTER `field235`;
)",
	},
	ManifestEntry{
		.version = 9042,
		.description = "2014_08_12_npc_raid_targets.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'raid_target'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types` ADD `raid_target` TINYINT(1) UNSIGNED NOT NULL DEFAULT '0' AFTER `no_target_hotkey`;

)",
	},
	ManifestEntry{
		.version = 9043,
		.description = "2014_08_18_spells_new_update.sql",
		.check = "SHOW COLUMNS FROM `spells_new` LIKE 'viral_targets'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `spells_new` CHANGE `field191` `viral_targets` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field192` `viral_timer` INT(11) NOT NULL DEFAULT '0';
)",
	},
	ManifestEntry{
		.version = 9044,
		.description = "2014_08_20_merchantlist_probability.sql",
		.check = "SHOW COLUMNS FROM `merchantlist` LIKE 'probability'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `merchantlist` ADD `probability` INT(3) NOT NULL DEFAULT '100' AFTER `classes_required`;
)",
	},
	ManifestEntry{
		.version = 9045,
		.description = "2014_08_23_complete_queryserv_table_structures.sql",
		.check = "SHOW TABLES LIKE 'qs_player_aa_rate_hourly'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- QS Table Structures --

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for qs_merchant_transaction_record
-- ----------------------------
DROP TABLE IF EXISTS `qs_merchant_transaction_record`;
CREATE TABLE `qs_merchant_transaction_record` (
  `transaction_id` int(11) NOT NULL AUTO_INCREMENT,
  `time` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,
  `zone_id` int(11) DEFAULT '0',
  `merchant_id` int(11) DEFAULT '0',
  `merchant_pp` int(11) DEFAULT '0',
  `merchant_gp` int(11) DEFAULT '0',
  `merchant_sp` int(11) DEFAULT '0',
  `merchant_cp` int(11) DEFAULT '0',
  `merchant_items` mediumint(7) DEFAULT '0',
  `char_id` int(11) DEFAULT '0',
  `char_pp` int(11) DEFAULT '0',
  `char_gp` int(11) DEFAULT '0',
  `char_sp` int(11) DEFAULT '0',
  `char_cp` int(11) DEFAULT '0',
  `char_items` mediumint(7) DEFAULT '0',
  PRIMARY KEY (`transaction_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for qs_merchant_transaction_record_entries
-- ----------------------------
DROP TABLE IF EXISTS `qs_merchant_transaction_record_entries`;
CREATE TABLE `qs_merchant_transaction_record_entries` (
  `event_id` int(11) DEFAULT '0',
  `char_slot` mediumint(7) DEFAULT '0',
  `item_id` int(11) DEFAULT '0',
  `charges` mediumint(7) DEFAULT '0',
  `aug_1` int(11) DEFAULT '0',
  `aug_2` int(11) DEFAULT '0',
  `aug_3` int(11) DEFAULT '0',
  `aug_4` int(11) DEFAULT '0',
  `aug_5` int(11) DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for qs_player_aa_rate_hourly
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_aa_rate_hourly`;
CREATE TABLE `qs_player_aa_rate_hourly` (
  `char_id` int(11) NOT NULL DEFAULT '0',
  `hour_time` int(11) NOT NULL,
  `aa_count` varchar(11) DEFAULT NULL,
  PRIMARY KEY (`char_id`,`hour_time`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for qs_player_delete_record
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_delete_record`;
CREATE TABLE `qs_player_delete_record` (
  `delete_id` int(11) NOT NULL AUTO_INCREMENT,
  `time` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,
  `char_id` int(11) DEFAULT '0',
  `stack_size` mediumint(7) DEFAULT '0',
  `char_items` mediumint(7) DEFAULT '0',
  PRIMARY KEY (`delete_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for qs_player_delete_record_entries
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_delete_record_entries`;
CREATE TABLE `qs_player_delete_record_entries` (
  `event_id` int(11) DEFAULT '0',
  `char_slot` mediumint(7) DEFAULT '0',
  `item_id` int(11) DEFAULT '0',
  `charges` mediumint(7) DEFAULT '0',
  `aug_1` int(11) DEFAULT '0',
  `aug_2` int(11) DEFAULT '0',
  `aug_3` int(11) DEFAULT '0',
  `aug_4` int(11) DEFAULT '0',
  `aug_5` int(11) DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for qs_player_events
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_events`;
CREATE TABLE `qs_player_events` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `char_id` int(11) DEFAULT '0',
  `event` int(11) unsigned DEFAULT '0',
  `event_desc` varchar(255) DEFAULT NULL,
  `time` int(11) unsigned DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for qs_player_handin_record
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_handin_record`;
CREATE TABLE `qs_player_handin_record` (
  `handin_id` int(11) NOT NULL AUTO_INCREMENT,
  `time` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,
  `quest_id` int(11) DEFAULT '0',
  `char_id` int(11) DEFAULT '0',
  `char_pp` int(11) DEFAULT '0',
  `char_gp` int(11) DEFAULT '0',
  `char_sp` int(11) DEFAULT '0',
  `char_cp` int(11) DEFAULT '0',
  `char_items` mediumint(7) DEFAULT '0',
  `npc_id` int(11) DEFAULT '0',
  `npc_pp` int(11) DEFAULT '0',
  `npc_gp` int(11) DEFAULT '0',
  `npc_sp` int(11) DEFAULT '0',
  `npc_cp` int(11) DEFAULT '0',
  `npc_items` mediumint(7) DEFAULT '0',
  PRIMARY KEY (`handin_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for qs_player_handin_record_entries
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_handin_record_entries`;
CREATE TABLE `qs_player_handin_record_entries` (
  `event_id` int(11) DEFAULT '0',
  `action_type` char(6) DEFAULT 'action',
  `char_slot` mediumint(7) DEFAULT '0',
  `item_id` int(11) DEFAULT '0',
  `charges` mediumint(7) DEFAULT '0',
  `aug_1` int(11) DEFAULT '0',
  `aug_2` int(11) DEFAULT '0',
  `aug_3` int(11) DEFAULT '0',
  `aug_4` int(11) DEFAULT '0',
  `aug_5` int(11) DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for qs_player_move_record
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_move_record`;
CREATE TABLE `qs_player_move_record` (
  `move_id` int(11) NOT NULL AUTO_INCREMENT,
  `time` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,
  `char_id` int(11) DEFAULT '0',
  `from_slot` mediumint(7) DEFAULT '0',
  `to_slot` mediumint(7) DEFAULT '0',
  `stack_size` mediumint(7) DEFAULT '0',
  `char_items` mediumint(7) DEFAULT '0',
  `postaction` tinyint(1) DEFAULT '0',
  PRIMARY KEY (`move_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for qs_player_move_record_entries
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_move_record_entries`;
CREATE TABLE `qs_player_move_record_entries` (
  `event_id` int(11) DEFAULT '0',
  `from_slot` mediumint(7) DEFAULT '0',
  `to_slot` mediumint(7) DEFAULT '0',
  `item_id` int(11) DEFAULT '0',
  `charges` mediumint(7) DEFAULT '0',
  `aug_1` int(11) DEFAULT '0',
  `aug_2` int(11) DEFAULT '0',
  `aug_3` int(11) DEFAULT '0',
  `aug_4` int(11) DEFAULT '0',
  `aug_5` int(11) DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for qs_player_npc_kill_record
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_npc_kill_record`;
CREATE TABLE `qs_player_npc_kill_record` (
  `fight_id` int(11) NOT NULL AUTO_INCREMENT,
  `npc_id` int(11) DEFAULT NULL,
  `type` int(11) DEFAULT NULL,
  `zone_id` int(11) DEFAULT NULL,
  `time` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`fight_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for qs_player_npc_kill_record_entries
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_npc_kill_record_entries`;
CREATE TABLE `qs_player_npc_kill_record_entries` (
  `event_id` int(11) DEFAULT '0',
  `char_id` int(11) DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for qs_player_speech
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_speech`;
CREATE TABLE `qs_player_speech` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `from` varchar(64) NOT NULL,
  `to` varchar(64) NOT NULL,
  `message` varchar(256) NOT NULL,
  `minstatus` smallint(5) NOT NULL,
  `guilddbid` int(11) NOT NULL,
  `type` tinyint(3) NOT NULL,
  `timerecorded` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for qs_player_trade_record
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_trade_record`;
CREATE TABLE `qs_player_trade_record` (
  `trade_id` int(11) NOT NULL AUTO_INCREMENT,
  `time` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,
  `char1_id` int(11) DEFAULT '0',
  `char1_pp` int(11) DEFAULT '0',
  `char1_gp` int(11) DEFAULT '0',
  `char1_sp` int(11) DEFAULT '0',
  `char1_cp` int(11) DEFAULT '0',
  `char1_items` mediumint(7) DEFAULT '0',
  `char2_id` int(11) DEFAULT '0',
  `char2_pp` int(11) DEFAULT '0',
  `char2_gp` int(11) DEFAULT '0',
  `char2_sp` int(11) DEFAULT '0',
  `char2_cp` int(11) DEFAULT '0',
  `char2_items` mediumint(7) DEFAULT '0',
  PRIMARY KEY (`trade_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for qs_player_trade_record_entries
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_trade_record_entries`;
CREATE TABLE `qs_player_trade_record_entries` (
  `event_id` int(11) DEFAULT '0',
  `from_id` int(11) DEFAULT '0',
  `from_slot` mediumint(7) DEFAULT '0',
  `to_id` int(11) DEFAULT '0',
  `to_slot` mediumint(7) DEFAULT '0',
  `item_id` int(11) DEFAULT '0',
  `charges` mediumint(7) DEFAULT '0',
  `aug_1` int(11) DEFAULT '0',
  `aug_2` int(11) DEFAULT '0',
  `aug_3` int(11) DEFAULT '0',
  `aug_4` int(11) DEFAULT '0',
  `aug_5` int(11) DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

)",
	},
	ManifestEntry{
		.version = 9046,
		.description = "2014_08_23_player_events_and_player_aa_rate_hourly.sql",
		.check = "SHOW TABLES LIKE 'qs_player_events'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- ----------------------------
-- Table structure for qs_player_events
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_events`;
CREATE TABLE `qs_player_events` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `char_id` int(11) DEFAULT '0',
  `event` int(11) unsigned DEFAULT '0',
  `event_desc` varchar(255) DEFAULT NULL,
  `time` int(11) unsigned DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for qs_player_aa_rate_hourly
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_aa_rate_hourly`;
CREATE TABLE `qs_player_aa_rate_hourly` (
  `char_id` int(11) NOT NULL DEFAULT '0',
  `hour_time` int(11) NOT NULL,
  `aa_count` varchar(11) DEFAULT NULL,
  PRIMARY KEY (`char_id`,`hour_time`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
)",
	},
	ManifestEntry{
		.version = 9048,
		.description = "2014_09_09_attack_delay.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'attack_delay'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types` ADD `attack_delay` TINYINT(3) UNSIGNED DEFAULT '30' NOT NULL AFTER `attack_speed`;
UPDATE `npc_types` SET `attack_delay` = 36 + 36 * (`attack_speed` / 100);
UPDATE `npc_types` SET `attack_delay` = 30 WHERE `attack_speed` = 0;

)",
	},
	ManifestEntry{
		.version = 9050,
		.description = "2014_09_20_ban_messages.sql",
		.check = "SHOW COLUMNS FROM `account` LIKE 'ban_reason'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `account` ADD COLUMN `ban_reason` TEXT NULL DEFAULT NULL, ADD COLUMN `suspend_reason` TEXT NULL DEFAULT NULL AFTER `ban_reason`;

)",
	},
	ManifestEntry{
		.version = 9051,
		.description = "2014_10_11_raidmotd.sql",
		.check = "SHOW COLUMNS FROM `raid_details` LIKE 'motd'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `raid_details` ADD `motd` varchar(1024);

)",
	},
	ManifestEntry{
		.version = 9052,
		.description = "2014_10_13_raidleadership.sql",
		.check = "SHOW TABLES LIKE 'raid_leaders'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `raid_leaders` (
	`gid` int(4) unsigned NOT NULL,
	`rid` int(4) unsigned NOT NULL,
	`marknpc` varchar(64) NOT NULL,
	`maintank` varchar(64) NOT NULL,
	`assist` varchar(64) NOT NULL,
	`puller` varchar(64) NOT NULL,
	`leadershipaa` tinyblob NOT NULL
);

)",
	},
	ManifestEntry{
		.version = 9053,
		.description = "2014_10_18_group_mentor.sql",
		.check = "SHOW COLUMNS FROM `group_leaders` LIKE 'mentoree'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `group_leaders` ADD `mentoree` VARCHAR(64) NOT NULL;
ALTER TABLE `group_leaders` ADD `mentor_percent` INT(4) DEFAULT 0 NOT NULL;

)",
	},
	ManifestEntry{
		.version = 9054,
		.description = "2014_10_19_raid_group_mentor.sql",
		.check = "SHOW COLUMNS FROM `raid_leaders` LIKE 'mentoree'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `raid_leaders` ADD `mentoree` VARCHAR(64) NOT NULL;
ALTER TABLE `raid_leaders` ADD `mentor_percent` INT(4) DEFAULT 0 NOT NULL;

)",
	},
	ManifestEntry{
		.version = 9055,
		.description = "2014_10_30_special_abilities_null.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'special_abilities'",
		.condition = "contains",
		.match = "NO",
		.sql = R"(
ALTER TABLE `merc_stats` MODIFY COLUMN `special_abilities`  text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL;

ALTER TABLE `npc_types` MODIFY COLUMN `special_abilities`  text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL;


)",
	},
	ManifestEntry{
		.version = 9056,
		.description = "2014_11_08_raidmembers.sql",
		.check = "SHOW COLUMNS FROM `raid_members` LIKE 'groupid'",
		.condition = "missing",
		.match = "unsigned",
		.sql = R"(
ALTER TABLE `raid_members` CHANGE COLUMN `groupid` `groupid` INT(4) UNSIGNED NOT NULL DEFAULT '0' AFTER `charid`;
)",
	},
	ManifestEntry{
		.version = 9057,
		.description = "2014_11_13_spells_new_updates.sql",
		.check = "SHOW COLUMNS FROM `spells_new` LIKE 'disallow_sit'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- spells new table update
ALTER TABLE `spells_new` CHANGE `field124` `disallow_sit` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field125` `deities0` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field196` `sneaking` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field158` `effectdescnum2` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field165` `ldon_trap` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field205` `no_block` INT(11) NOT NULL DEFAULT '0';



)",
	},
	ManifestEntry{
		.version = 9058,
		.description = "2014_11_26_inventorytableupdate.sql",
		.check = "SHOW COLUMNS FROM `inventory` LIKE 'ornamenticon'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- Inventory table update
ALTER TABLE `inventory`
	ADD COLUMN `ornamenticon` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `custom_data`,
	ADD COLUMN `ornamentidfile` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `ornamenticon`;

)",
	},
	ManifestEntry{
		.version = 9059,
		.description = "2014_12_01_mercs_table_update.sql",
		.check = "SHOW COLUMNS FROM `mercs` LIKE 'MercSize'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `mercs` ADD `MercSize` float( 0 ) NOT NULL DEFAULT '5' AFTER `Gender`;

)",
	},
	ManifestEntry{
		.version = 9060,
		.description = "2014_12_09_items_table_update.sql",
		.check = "SHOW COLUMNS FROM `items` LIKE 'herosforgemodel'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `items` ADD `herosforgemodel` int( 11 ) NOT NULL DEFAULT '0' AFTER `material`;

)",
	},
	ManifestEntry{
		.version = 9061,
		.description = "2014_12_13_inventory_table_update.sql",
		.check = "SHOW COLUMNS FROM `inventory` LIKE 'ornament_hero_model'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `inventory` ADD `ornament_hero_model` int( 11 ) NOT NULL DEFAULT '0' AFTER `ornamentidfile`;
)",
	},
	ManifestEntry{
		.version = 9062,
		.description = "2014_12_15_multiple_table_updates.sql",
		.check = "SHOW COLUMNS FROM `items` LIKE 'augslot6type'",
		.condition = "empty",
		.match = "",
		.sql = R"(
/* Add the new Aug Slot 6 Fields to the items table */
ALTER TABLE `items` ADD `augslot6type` tinyint( 3 ) NOT NULL DEFAULT '0' AFTER `augslot5visible`;
ALTER TABLE `items` ADD `augslot6visible` tinyint( 3 ) NOT NULL DEFAULT '0' AFTER `augslot6type`;
ALTER TABLE `items` ADD `augslot6unk2` int( 11 ) NOT NULL DEFAULT '0' AFTER `augslot5unk2`;

/* Add the new Aug Slot 6 Field to the inventory table */
ALTER TABLE `inventory` ADD `augslot6` mediumint( 7 ) NOT NULL DEFAULT '0' AFTER `augslot5`;

/* Add the new Aug Slot 6 Field to the sharedbank table */
ALTER TABLE `sharedbank` ADD `augslot6` mediumint( 7 ) NOT NULL DEFAULT '0' AFTER `augslot5`;

/* Add the new Aug Slot 6 Field to the object_contents table */
ALTER TABLE `object_contents` ADD `augslot6` mediumint( 7 ) NOT NULL DEFAULT '0' AFTER `augslot5`;

/* Add the new Aug Slot 6 Field to the sharedbank table */
ALTER TABLE `character_corpse_items` ADD `aug_6` int( 11 ) NOT NULL DEFAULT '0' AFTER `aug_5`;
)",
	},
	ManifestEntry{
		.version = 9063,
		.description = "2014_12_24_npc_types_update.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'd_melee_texture1'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types` CHANGE `d_meele_texture1` `d_melee_texture1` INT(11) DEFAULT NULL;
ALTER TABLE `npc_types` CHANGE `d_meele_texture2` `d_melee_texture2` INT(11) DEFAULT NULL;
)",
	},
	ManifestEntry{
		.version = 9064,
		.description = "2014_12_24_npc_types_table_update.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'herosforgemodel'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types` ADD `herosforgemodel` int( 11 ) NOT NULL DEFAULT '0' AFTER `helmtexture`;
)",
	},
	ManifestEntry{
		.version = 9066,
		.description = "2014_12_31_npc_types_default_values_update.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'bodytype'",
		.condition = "contains",
		.match = "YES",
		.sql = R"(
UPDATE `npc_types` SET `bodytype` = 0 WHERE `bodytype` IS NULL;
ALTER TABLE `npc_types` MODIFY `bodytype` INT(11) NOT NULL DEFAULT '1';
UPDATE `npc_types` SET `d_melee_texture1` = 0 WHERE `d_melee_texture1` IS NULL;
ALTER TABLE `npc_types` MODIFY `d_melee_texture1` INT(11) NOT NULL DEFAULT '0';
UPDATE `npc_types` SET `d_melee_texture2` = 0 WHERE `d_melee_texture2` IS NULL;
ALTER TABLE `npc_types` MODIFY `d_melee_texture2` INT(11) NOT NULL DEFAULT '0';

)",
	},
	ManifestEntry{
		.version = 9067,
		.description = "2015_01_21_npc_types_update.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'light'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types` ADD `light` tinyint( 2 ) NOT NULL DEFAULT '0';
)",
	},
	ManifestEntry{
		.version = 9068,
		.description = "2015_01_15_logsys_categories_table.sql",
		.check = "SHOW TABLES LIKE 'logsys_categories'",
		.condition = "empty",
		.match = "",
		.sql = R"(
SET FOREIGN_KEY_CHECKS=0;

DROP TABLE IF EXISTS `logsys_categories`;
CREATE TABLE `logsys_categories` (
  `log_category_id` int(11) NOT NULL,
  `log_category_description` varchar(150) DEFAULT NULL,
  `log_to_console` smallint(11) DEFAULT '0',
  `log_to_file` smallint(11) DEFAULT '0',
  `log_to_gmsay` smallint(11) DEFAULT '0',
  PRIMARY KEY (`log_category_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
)",
	},
	ManifestEntry{
		.version = 9069,
		.description = "2015_01_25_logsys_mercenaries_category.sql",
		.check = "SELECT * FROM `logsys_categories` WHERE `log_category_description` LIKE 'Mercenaries'",
		.condition = "empty",
		.match = "",
		.sql = R"(
INSERT INTO `logsys_categories` (`log_category_id`,`log_category_description`,`log_to_console`,`log_to_file`,`log_to_gmsay`) VALUES ('37', 'Mercenaries', '0', '0', '0');
)",
	},
	ManifestEntry{
		.version = 9070,
		.description = "2015_01_28_quest_debug_log_category.sql",
		.check = "SELECT * FROM `logsys_categories` WHERE `log_category_description` LIKE 'Quest Debug'",
		.condition = "empty",
		.match = "",
		.sql = R"(
INSERT INTO `logsys_categories` (`log_category_id`, `log_category_description`, `log_to_gmsay`) VALUES ('38', 'Quest Debug', '1');

)",
	},
	ManifestEntry{
		.version = 9071,
		.description = "2015_01_29_merc_stats_table_update.sql",
		.check = "SHOW COLUMNS FROM `merc_stats` LIKE 'statscale'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `merc_stats` ADD `statscale` int( 11 ) NOT NULL DEFAULT '100' AFTER `runspeed`;
)",
	},
	ManifestEntry{
		.version = 9072,
		.description = "2015_01_30_merc_attack_delay.sql",
		.check = "SHOW COLUMNS FROM `merc_stats` LIKE 'attack_delay'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `merc_stats` ADD `attack_delay` TINYINT(3) UNSIGNED DEFAULT '30' NOT NULL AFTER `attack_speed`;
UPDATE `merc_stats` SET `attack_delay` = 36 + 36 * (`attack_speed` / 100);
UPDATE `merc_stats` SET `attack_delay` = 30 WHERE `attack_speed` = 0;

)",
	},
	ManifestEntry{
		.version = 9073,
		.description = "2015_01_31_character_item_recast.sql",
		.check = "SHOW TABLES LIKE 'character_item_recast'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `character_item_recast` (
    `id` int(11) UNSIGNED NOT NULL DEFAULT 0,
    `recast_type` smallint(11) UNSIGNED NOT NULL DEFAULT 0,
    `timestamp` int(11) UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY(`id`, `recast_type`),
    KEY `id` (`id`)
) ENGINE = InnoDB DEFAULT CHARSET = latin1;

)",
	},
	ManifestEntry{
		.version = 9074,
		.description = "2015_02_01_logsys_packet_logs.sql",
		.check = "SELECT * FROM `logsys_categories` WHERE `log_category_description` LIKE 'Packet: Server -> Client'",
		.condition = "empty",
		.match = "",
		.sql = R"(
REPLACE INTO `logsys_categories` (`log_category_id`, `log_category_description`) VALUES ('39', 'Packet: Server -> Client');
REPLACE INTO `logsys_categories` (`log_category_id`, `log_category_description`) VALUES ('5', 'Packet: Client -> Server');
REPLACE INTO `logsys_categories` (`log_category_id`, `log_category_description`) VALUES ('40', 'Packet: Client -> Server Unhandled');
)",
	},
	ManifestEntry{
		.version = 9075,
		.description = "2015_02_02_logsys_packet_logs_with_dump.sql",
		.check = "SELECT * FROM `logsys_categories` WHERE `log_category_description` LIKE 'Packet: Server -> Client With Dump'",
		.condition = "empty",
		.match = "",
		.sql = R"(
REPLACE INTO `logsys_categories` (`log_category_id`, `log_category_description`) VALUES ('41', 'Packet: Server -> Client With Dump');
REPLACE INTO `logsys_categories` (`log_category_id`, `log_category_description`) VALUES ('42', 'Packet: Client -> Server With Dump');

)",
	},
	ManifestEntry{
		.version = 9076,
		.description = "2015_02_04_average_coin.sql",
		.check = "SHOW COLUMNS FROM `loottable` WHERE Field = 'avgcoin'",
		.condition = "contains",
		.match = "smallint",
		.sql = R"(
ALTER TABLE `loottable` CHANGE COLUMN `avgcoin` `avgcoin` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `maxcash`;
UPDATE `loottable` SET avgcoin = 0;

)",
	},
	ManifestEntry{
		.version = 9077,
		.description = "2015_02_12_zone_gravity.sql",
		.check = "SHOW COLUMNS FROM `zone` LIKE 'gravity'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `zone`
ADD COLUMN `gravity`  float NOT NULL DEFAULT .4 AFTER `snow_duration4`;
)",
	},
	ManifestEntry{
		.version = 9078,
		.description = "2015_05_20_buffinstrumentmod.sql",
		.check = "SHOW COLUMNS FROM `character_buffs` LIKE 'instrument_mod'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `character_buffs` ADD COLUMN `instrument_mod` int(10) DEFAULT 10 NOT NULL;

)",
	},
	ManifestEntry{
		.version = 9079,
		.description = "2015_05_23_buffdurations.sql",
		.check = "SHOW COLUMNS FROM `character_buffs` LIKE 'ticsremaining'",
		.condition = "contains",
		.match = "unsigned",
		.sql = R"(
ALTER TABLE `character_buffs` CHANGE COLUMN `ticsremaining` `ticsremaining` INT(11) SIGNED NOT NULL;
ALTER TABLE `merc_buffs` CHANGE COLUMN `TicsRemaining` `TicsRemaining` INT(11) SIGNED NOT NULL DEFAULT 0;

)",
	},
	ManifestEntry{
		.version = 9080,
		.description = "2015_05_23_petbuffinstrumentmod.sql",
		.check = "SHOW COLUMNS FROM `character_pet_buffs` LIKE 'instrument_mod'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `character_pet_buffs` ADD COLUMN `instrument_mod` tinyint UNSIGNED DEFAULT 10 NOT NULL;

)",
	},
	ManifestEntry{
		.version = 9081,
		.description = "2015_05_23_dbstr_us.sql",
		.check = "SHOW TABLES LIKE 'db_str'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `db_str` (
	`id` INT(10) NOT NULL,
	`type` INT(10) NOT NULL,
	`value` TEXT NOT NULL,
	PRIMARY KEY (`id`, `type`)
);

)",
	},
	ManifestEntry{
		.version = 9082,
		.description = "2015_05_25_npc_types_texture_fields.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'armtexture'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE npc_types
ADD COLUMN `armtexture` tinyint(2) NOT NULL DEFAULT '0' AFTER `raid_target`,
ADD COLUMN `bracertexture` tinyint(2) NOT NULL DEFAULT '0' AFTER `armtexture`,
ADD COLUMN `handtexture` tinyint(2) NOT NULL DEFAULT '0' AFTER `bracertexture`,
ADD COLUMN `legtexture` tinyint(2) NOT NULL DEFAULT '0' AFTER `handtexture`,
ADD COLUMN `feettexture` tinyint(2) NOT NULL DEFAULT '0' AFTER `legtexture`;
)",
	},
	ManifestEntry{
		.version = 9083,
		.description = "2015_06_07_aa_update.sql",
		.check = "SHOW COLUMNS FROM `character_alternate_abilities` LIKE 'charges'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE character_alternate_abilities ADD COLUMN charges SMALLINT(11) UNSIGNED NOT NULL DEFAULT 0;

)",
	},
	ManifestEntry{
		.version = 9084,
		.description = "2015_06_30_runspeed_adjustments.sql",
		.check = "SELECT `runspeed` FROM `npc_types` WHERE `runspeed` > 3",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
/* This rescales the old peq runspeeds which were about 80 percent too high to new values */
/* This section should only ever be run once */
UPDATE npc_types SET npc_types.runspeed = 1.050 WHERE (npc_types.runspeed > 0 and npc_types.runspeed < 1.2);
UPDATE npc_types SET npc_types.runspeed = 1.325 WHERE (npc_types.runspeed > 1.19 and npc_types.runspeed < 1.75 and race != 73 and race != 72);
UPDATE npc_types SET npc_types.runspeed = 1.575 WHERE (npc_types.runspeed > 1.69 and npc_types.runspeed < 2.2);
UPDATE npc_types SET npc_types.runspeed = 1.850 WHERE (npc_types.runspeed > 2.19 and npc_types.runspeed < 3);
UPDATE npc_types SET npc_types.runspeed = 3 WHERE npc_types.runspeed > 3;
)",
	},
	ManifestEntry{
		.version = 9085,
		.description = "2015_07_01_marquee_rule.sql",
		.check = "SELECT * FROM `rule_values` WHERE `rule_name` LIKE '%Character:MarqueeHPUpdates%'",
		.condition = "empty",
		.match = "",
		.sql = R"(
INSERT INTO `rule_values` (`rule_name`, `rule_value`, `notes`) VALUES ('Character:MarqueeHPUpdates', 'false', 'Will show Health % in center of screen < 100%');
)",
	},
	ManifestEntry{
		.version = 9086,
		.description = "2015_07_02_aa_rework.sql",
		.check = "SHOW TABLES LIKE 'aa_ranks'",
		.condition = "empty",
		.match = "",
		.sql = R"(
DROP TABLE IF EXISTS `aa_ability`;
CREATE TABLE IF NOT EXISTS `aa_ability` (
  `id` int(10) unsigned NOT NULL,
  `name` text NOT NULL,
  `category` int(10) NOT NULL DEFAULT '-1',
  `classes` int(10) NOT NULL DEFAULT '65535',
  `races` int(10) NOT NULL DEFAULT '65535',
  `drakkin_heritage` int(10) NOT NULL DEFAULT '127',
  `deities` int(10) NOT NULL DEFAULT '131071',
  `status` int(10) NOT NULL DEFAULT '0',
  `type` int(10) NOT NULL DEFAULT '0',
  `charges` int(11) NOT NULL DEFAULT '0',
  `grant_only` tinyint(4) NOT NULL DEFAULT '0',
  `first_rank_id` int(10) NOT NULL DEFAULT '-1',
  `enabled` tinyint(3) unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `aa_ranks`;
CREATE TABLE IF NOT EXISTS `aa_ranks` (
  `id` int(10) unsigned NOT NULL,
  `upper_hotkey_sid` int(10) NOT NULL DEFAULT '-1',
  `lower_hotkey_sid` int(10) NOT NULL DEFAULT '-1',
  `title_sid` int(10) NOT NULL DEFAULT '-1',
  `desc_sid` int(10) NOT NULL DEFAULT '-1',
  `cost` int(10) NOT NULL DEFAULT '1',
  `level_req` int(10) NOT NULL DEFAULT '51',
  `spell` int(10) NOT NULL DEFAULT '-1',
  `spell_type` int(10) NOT NULL DEFAULT '0',
  `recast_time` int(10) NOT NULL DEFAULT '0',
  `expansion` int(10) NOT NULL DEFAULT '0',
  `prev_id` int(10) NOT NULL DEFAULT '-1',
  `next_id` int(10) NOT NULL DEFAULT '-1',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Dumping structure for table eqdb.aa_rank_effects
DROP TABLE IF EXISTS `aa_rank_effects`;
CREATE TABLE IF NOT EXISTS `aa_rank_effects` (
  `rank_id` int(10) unsigned NOT NULL,
  `slot` int(10) unsigned NOT NULL DEFAULT '1',
  `effect_id` int(10) NOT NULL DEFAULT '0',
  `base1` int(10) NOT NULL DEFAULT '0',
  `base2` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`rank_id`,`slot`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `aa_rank_prereqs`;
CREATE TABLE IF NOT EXISTS `aa_rank_prereqs` (
  `rank_id` int(10) unsigned NOT NULL,
  `aa_id` int(10) NOT NULL,
  `points` int(10) NOT NULL,
  PRIMARY KEY (`rank_id`,`aa_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

RENAME TABLE `character_alternate_abilities` TO `character_alternate_abilities_old`;
DROP TABLE IF EXISTS `character_alternate_abilities`;
CREATE TABLE IF NOT EXISTS `character_alternate_abilities` (
  `id` int(11) unsigned NOT NULL DEFAULT '0',
  `aa_id` smallint(11) unsigned NOT NULL DEFAULT '0',
  `aa_value` smallint(11) unsigned NOT NULL DEFAULT '0',
  `charges` smallint(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`,`aa_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

ALTER TABLE `character_data` ADD COLUMN `aa_points_spent_old` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `aa_points_spent`;
ALTER TABLE `character_data` ADD COLUMN `aa_points_old` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `aa_points`;

UPDATE `character_data` SET `aa_points_spent_old` = `aa_points_spent`, `aa_points_old` = `aa_points`;

 -- sanity checks since if someone never logged in after the db conversion there is junk data
 -- I don't have a good way of addressing this so I keep the old data in aa_points_spent_old and aa_points_old and character_alternate_abilities_old
 -- for anyone who wants to personally polish up their player data
UPDATE `character_data` SET `aa_points_spent` = 2700 WHERE `aa_points_spent` > 2700;
UPDATE `character_data` SET `aa_points` = 5000 WHERE `aa_points` > 5000;

 -- another sanity check, give people a few levels below 51 to keep their points
UPDATE `character_data` SET `aa_points_spent` = 0 WHERE `level` < 48;
UPDATE `character_data` SET `aa_points` = 0 WHERE `level` < 48;

 -- aa refund here
UPDATE `character_data` SET `aa_points` = `aa_points_spent` + `aa_points`;
UPDATE `character_data` SET `aa_points_spent` = 0;

)",
	},
	ManifestEntry{
		.version = 9087,
		.description = "2015_09_25_inventory_snapshots.sql",
		.check = "SHOW TABLES LIKE 'inventory_snapshots'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `inventory_snapshots` (
	`time_index` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`charid` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`slotid` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`itemid` INT(11) UNSIGNED NULL DEFAULT '0',
	`charges` SMALLINT(3) UNSIGNED NULL DEFAULT '0',
	`color` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`augslot1` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`augslot2` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`augslot3` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`augslot4` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`augslot5` MEDIUMINT(7) UNSIGNED NULL DEFAULT '0',
	`augslot6` MEDIUMINT(7) NOT NULL DEFAULT '0',
	`instnodrop` TINYINT(1) UNSIGNED NOT NULL DEFAULT '0',
	`custom_data` TEXT NULL,
	`ornamenticon` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`ornamentidfile` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`ornament_hero_model` INT(11) NOT NULL DEFAULT '0',
	PRIMARY KEY (`time_index`, `charid`, `slotid`)
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB;

ALTER TABLE `character_data` ADD COLUMN `e_last_invsnapshot` INT(11) UNSIGNED NOT NULL DEFAULT '0';

)",
	},
	ManifestEntry{
		.version = 9088,
		.description = "2015_11_01_perl_event_export_settings.sql",
		.check = "SHOW TABLES LIKE 'perl_event_export_settings'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `perl_event_export_settings` (
  `event_id` int(11) NOT NULL,
  `event_description` varchar(150) DEFAULT NULL,
  `export_qglobals` smallint(11) DEFAULT '0',
  `export_mob` smallint(11) DEFAULT '0',
  `export_zone` smallint(11) DEFAULT '0',
  `export_item` smallint(11) DEFAULT '0',
  `export_event` smallint(11) DEFAULT '0',
  PRIMARY KEY (`event_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of perl_event_export_settings
-- ----------------------------
INSERT INTO `perl_event_export_settings` VALUES ('0', 'EVENT_SAY', '1', '1', '1', '1', '1');
INSERT INTO `perl_event_export_settings` VALUES ('1', 'EVENT_ITEM', '1', '1', '1', '1', '1');
INSERT INTO `perl_event_export_settings` VALUES ('2', 'EVENT_DEATH', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('3', 'EVENT_SPAWN', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('4', 'EVENT_ATTACK', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('5', 'EVENT_COMBAT', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('6', 'EVENT_AGGRO', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('7', 'EVENT_SLAY', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('8', 'EVENT_NPC_SLAY', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('9', 'EVENT_WAYPOINT_ARRIVE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('10', 'EVENT_WAYPOINT_DEPART', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('11', 'EVENT_TIMER', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('12', 'EVENT_SIGNAL', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('13', 'EVENT_HP', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('14', 'EVENT_ENTER', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('15', 'EVENT_EXIT', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('16', 'EVENT_ENTERZONE', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('17', 'EVENT_CLICKDOOR', '1', '1', '1', '1', '1');
INSERT INTO `perl_event_export_settings` VALUES ('18', 'EVENT_LOOT', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('19', 'EVENT_ZONE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('20', 'EVENT_LEVEL_UP', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('21', 'EVENT_KILLED_MERIT', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('22', 'EVENT_CAST_ON', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('23', 'EVENT_TASKACCEPTED', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('24', 'EVENT_TASK_STAGE_COMPLETE', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('25', 'EVENT_TASK_UPDATE', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('26', 'EVENT_TASK_COMPLETE', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('27', 'EVENT_TASK_FAIL', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('28', 'EVENT_AGGRO_SAY', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('29', 'EVENT_PLAYER_PICKUP', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('30', 'EVENT_POPUPRESPONSE', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('31', 'EVENT_ENVIRONMENTAL_DAMAGE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('32', 'EVENT_PROXIMITY_SAY', '1', '1', '1', '1', '1');
INSERT INTO `perl_event_export_settings` VALUES ('33', 'EVENT_CAST', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('34', 'EVENT_CAST_BEGIN', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('35', 'EVENT_SCALE_CALC', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('36', 'EVENT_ITEM_ENTER_ZONE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('37', 'EVENT_TARGET_CHANGE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('38', 'EVENT_HATE_LIST', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('39', 'EVENT_SPELL_EFFECT_CLIENT', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('40', 'EVENT_SPELL_EFFECT_NPC', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('41', 'EVENT_SPELL_EFFECT_BUFF_TIC_CLIENT', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('42', 'EVENT_SPELL_EFFECT_BUFF_TIC_NPC', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('43', 'EVENT_SPELL_FADE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('44', 'EVENT_SPELL_EFFECT_TRANSLOCATE_COMPLETE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('45', 'EVENT_COMBINE_SUCCESS', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('46', 'EVENT_COMBINE_FAILURE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('47', 'EVENT_ITEM_CLICK', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('48', 'EVENT_ITEM_CLICK_CAST', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('49', 'EVENT_GROUP_CHANGE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('50', 'EVENT_FORAGE_SUCCESS', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('51', 'EVENT_FORAGE_FAILURE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('52', 'EVENT_FISH_START', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('53', 'EVENT_FISH_SUCCESS', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('54', 'EVENT_FISH_FAILURE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('55', 'EVENT_CLICK_OBJECT', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('56', 'EVENT_DISCOVER_ITEM', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('57', 'EVENT_DISCONNECT', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('58', 'EVENT_CONNECT', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('59', 'EVENT_ITEM_TICK', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('60', 'EVENT_DUEL_WIN', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('61', 'EVENT_DUEL_LOSE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('62', 'EVENT_ENCOUNTER_LOAD', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('63', 'EVENT_ENCOUNTER_UNLOAD', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('64', 'EVENT_SAY', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('65', 'EVENT_DROP_ITEM', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('66', 'EVENT_DESTROY_ITEM', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('67', 'EVENT_FEIGN_DEATH', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('68', 'EVENT_WEAPON_PROC', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('69', 'EVENT_EQUIP_ITEM', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('70', 'EVENT_UNEQUIP_ITEM', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('71', 'EVENT_AUGMENT_ITEM', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('72', 'EVENT_UNAUGMENT_ITEM', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('73', 'EVENT_AUGMENT_INSERT', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('74', 'EVENT_AUGMENT_REMOVE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('75', 'EVENT_ENTER_AREA', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('76', 'EVENT_LEAVE_AREA', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('77', 'EVENT_RESPAWN', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('78', 'EVENT_DEATH_COMPLETE', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('79', 'EVENT_UNHANDLED_OPCODE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('80', 'EVENT_TICK', '0', '1', '1', '0', '1');
)",
	},
	ManifestEntry{
		.version = 9089,
		.description = "2015_11_02_ai_idle_no_spell_recast_default_changes.sql",
		.check = "SELECT * FROM `rule_values` WHERE `rule_name` LIKE '%Spells:AI_IdleNoSpellMinRecast%' AND `rule_value` = '500'",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
UPDATE `rule_values` SET `rule_value` = '6000' WHERE `rule_value` = '500' AND `rule_name` = 'Spells:AI_IdleNoSpellMinRecast';
UPDATE `rule_values` SET `rule_value` = '60000' WHERE `rule_value` = '2000' AND `rule_name` = 'Spells:AI_IdleNoSpellMaxRecast';
)",
	},
	ManifestEntry{
		.version = 9091,
		.description = "2015_12_07_command_settings.sql",
		.check = "SHOW TABLES LIKE 'command_settings'",
		.condition = "empty",
		.match = "",
		.sql = R"(
RENAME TABLE `commands` to `commands_old`;

CREATE TABLE `command_settings` (
	`command` varchar(128) NOT NULL DEFAULT '',
	`access` int(11) NOT NULL DEFAULT '0',
	`aliases` varchar(256) NOT NULL DEFAULT '',
	PRIMARY KEY (`command`),
	UNIQUE KEY `UK_command_settings_1` (`command`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

INSERT INTO `command_settings` VALUES ('acceptrules', '90', ''),('advnpcspawn', '150', 'advnpc'),('aggro', '80', ''),('aggrozone', '200', ''),('ai', '100', ''),('appearance', '150', ''),('apply_shared_memory', '250', ''),('attack', '150', ''),('augmentitem', '250', 'aug'),('ban', '200', ''),('beard', '80', ''),('beardcolor', '80', ''),('bestz', '80', ''),('bind', '80', ''),('camerashake', '80', ''),('castspell', '90', 'cast'),('chat', '200', ''),('checklos', '50', 'los'),('clearinvsnapshots', '200', ''),('connectworldserver', '200', 'connectworld'),('corpse', '90', ''),('crashtest', '201', ''),('cvs', '80', ''),('damage', '150', ''),('date', '150', ''),('dbspawn2', '100', ''),('delacct', '200', ''),('deletegraveyard', '200', ''),('delpetition', '80', ''),('depop', '100', ''),('depopzone', '100', ''),('details', '80', ''),('disablerecipe', '80', ''),('disarmtrap', '80', ''),('distance', '80', ''),('doanim', '50', ''),('emote', '150', ''),('emotesearch', '80', ''),('emoteview', '80', ''),('enablerecipe', '80', ''),('equipitem', '50', ''),('face', '80', ''),('findnpctype', '90', 'fn'),('findspell', '90', 'fs|spfind'),('findzone', '1', 'fz'),('fixmob', '150', ''),('flag', '201', ''),('flagedit', '150', ''),('flags', '80', ''),('flymode', '80', ''),('fov', '80', ''),('freeze', '100', ''),('gassign', '150', ''),('gender', '90', ''),('getplayerburiedcorpsecount', '100', ''),('getvariable', '200', ''),('ginfo', '20', ''),('giveitem', '150', 'gi'),('givemoney', '150', ''),('globalview', '80', ''),('gm', '80', ''),('gmspeed', '80', ''),('goto', '80', ''),('grid', '150', ''),('guild', '80', 'guilds'),('guildapprove', '0', ''),('guildcreate', '0', ''),('guildlist', '0', ''),('hair', '80', ''),('haircolor', '80', ''),('haste', '100', ''),('hatelist', '80', ''),('heal', '100', ''),('helm', '80', ''),('help', '0', ''),('heritage', '80', ''),('heromodel', '200', 'hm'),('hideme', '80', 'gmhideme'),('hotfix', '250', ''),('hp', '90', ''),('incstat', '200', ''),('instance', '80', ''),('interrogateinv', '0', ''),('interrupt', '50', ''),('invsnapshot', '80', ''),('invul', '80', 'invulnerable'),('ipban', '201', ''),('iplookup', '200', ''),('iteminfo', '10', ''),('itemsearch', '90', 'fi|finditem|search'),('kick', '80', ''),('kill', '80', ''),('lastname', '80', ''),('level', '150', ''),('listnpcs', '90', ''),('listpetition', '80', ''),('load_shared_memory', '250', ''),('loc', '0', ''),('lock', '200', ''),('logs', '250', ''),('logtest', '250', ''),('makepet', '150', ''),('mana', '100', ''),('maxskills', '90', ''),('memspell', '100', ''),('merchant_close_shop', '100', 'close_shop'),('merchant_open_shop', '100', 'open_shop'),('modifynpcstat', '150', ''),('motd', '200', ''),('movechar', '80', ''),('myskills', '0', ''),('mysqltest', '250', ''),('mysql', '255', ''),('mystats', '50', ''),('name', '100', ''),('netstats', '200', ''),('npccast', '90', ''),('npcedit', '150', ''),('npcemote', '80', ''),('npcloot', '150', ''),('npcsay', '80', ''),('npcshout', '90', ''),('npcspawn', '100', ''),('npcspecialattk', '150', 'npcspecialatk|npcspecialattack'),('npcstats', '90', ''),('npctype_cache', '250', ''),('npctypespawn', '90', 'dbspawn'),('nukebuffs', '100', ''),('nukeitem', '150', ''),('object', '100', ''),('oocmute', '200', ''),('opcode', '250', ''),('path', '200', ''),('peekinv', '80', ''),('peqzone', '2', ''),('permaclass', '150', ''),('permagender', '150', ''),('permarace', '150', ''),('petitioninfo', '20', ''),('pf', '0', ''),('picklock', '0', ''),('pvp', '80', ''),('qglobal', '150', ''),('questerrors', '0', ''),('race', '90', ''),('raidloot', '0', ''),('randomfeatures', '90', ''),('refreshgroup', '0', ''),('reloadaa', '200', ''),('reloadallrules', '80', ''),('reloademote', '80', ''),('reloadlevelmods', '255', ''),('reloadperlexportsettings', '255', ''),('reloadqst', '80', 'reloadquest|rq'),('reloadrulesworld', '80', ''),('reloadstatic', '150', ''),('reloadtitles', '150', ''),('reloadworld', '255', ''),('reloadzps', '150', 'reloadzonepoints'),('repop', '90', ''),('repopclose', '100', ''),('resetaa', '100', ''),('resetaa_timer', '200', ''),('revoke', '80', ''),('rules', '200', ''),('save', '80', ''),('scribespell', '90', ''),('scribespells', '100', ''),('sendzonespawns', '200', ''),('sensetrap', '0', ''),('serverinfo', '201', ''),('serverrules', '90', ''),('setaapts', '100', 'setaapoints'),('setaaxp', '100', 'setaaexp'),('setadventurepoints', '200', ''),('setanim', '200', ''),('setcrystals', '100', ''),('setfaction', '170', ''),('setgraveyard', '200', ''),('setlanguage', '50', ''),('setlsinfo', '0', ''),('setpass', '150', ''),('setpvppoints', '100', ''),('setskill', '90', ''),('setskillall', '100', 'setallskill|setallskills'),('setstartzone', '80', ''),('setstat', '255', ''),('setxp', '100', 'setexp'),('showbonusstats', '50', ''),('showbuffs', '80', ''),('shownumhits', '0', ''),('showskills', '50', ''),('showspellslist', '100', ''),('showstats', '80', ''),('shutdown', '200', ''),('size', '90', ''),('spawn', '150', ''),('spawnfix', '80', ''),('spawnstatus', '150', ''),('spellinfo', '10', ''),('spoff', '0', ''),('spon', '0', ''),('stun', '100', ''),('summon', '80', ''),('summonburiedplayercorpse', '100', ''),('summonitem', '150', 'si'),('suspend', '100', ''),('task', '150', ''),('tattoo', '80', ''),('tempname', '100', ''),('texture', '150', ''),('time', '90', ''),('timers', '200', ''),('timezone', '90', ''),('title', '100', ''),('titlesuffix', '50', ''),('traindisc', '100', ''),('tune', '100', ''),('undyeme', '0', ''),('unfreeze', '100', ''),('unlock', '150', ''),('unscribespell', '90', ''),('unscribespells', '100', ''),('untraindisc', '180', ''),('untraindiscs', '180', ''),('uptime', '10', ''),('version', '0', ''),('viewnpctype', '100', ''),('viewpetition', '80', ''),('wc', '200', ''),('weather', '90', ''),('worldshutdown', '200', ''),('wp', '150', ''),('wpadd', '150', ''),('wpinfo', '150', ''),('xtargets', '250', ''),('zclip', '150', ''),('zcolor', '150', ''),('zheader', '150', ''),('zone', '80', ''),('zonebootup', '100', ''),('zoneinstance', '80', ''),('zonelock', '200', ''),('zoneshutdown', '200', ''),('zonespawn', '250', ''),('zonestatus', '150', ''),('zopp', '250', ''),('zsafecoords', '150', ''),('zsave', '200', ''),	('zsky', '150', ''),('zstats', '80', ''),('zunderworld', '80', ''),('zuwcoords', '80', '');

)",
	},
	ManifestEntry{
		.version = 9092,
		.description = "2015_12_17_eqtime.sql",
		.check = "SHOW TABLES LIKE 'eqtime'",
		.condition = "empty",
		.match = "",
		.sql = R"(
DROP TABLE IF EXISTS `eqtime`;
CREATE TABLE `eqtime` (
  `minute` tinyint(4) not null default 0,
  `hour` tinyint(4) not null default 0,
  `day` tinyint(4) not null default 0,
  `month` tinyint(4) not null default 0,
  `year` int(4) not null default 0,
  `realtime` int(11) not null default 0
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

INSERT INTO eqtime values (0,1,28,12,3766,1444035661);
)",
	},
	ManifestEntry{
		.version = 9093,
		.description = "2015_12_21_items_updates_evoitem.sql",
		.check = "SHOW COLUMNS FROM `items` LIKE 'evoitem'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `items`
	ADD COLUMN `evoitem` INT(11) NOT NULL DEFAULT '0' AFTER `purity`,
	ADD COLUMN `evoid` INT(11) NOT NULL DEFAULT '0' AFTER `evoitem`,
	ADD COLUMN `evomax` INT(11) NOT NULL DEFAULT '0' AFTER `evolvinglevel`,
	CHANGE `UNK038` `skillmodmax` INT(11) NOT NULL DEFAULT '0',
	CHANGE `UNK222` `heirloom` INT(11) NOT NULL DEFAULT '0',
	CHANGE `UNK235` `placeable` INT(11) NOT NULL DEFAULT '0',
	CHANGE `UNK242` `epicitem` INT(11) NOT NULL DEFAULT '0';
)",
	},
	ManifestEntry{
		.version = 9094,
		.description = "2015_12_29_quest_zone_events.sql",
		.check = "SELECT * FROM perl_event_export_settings WHERE event_description = 'EVENT_SPAWN_ZONE'",
		.condition = "empty",
		.match = "",
		.sql = R"(
INSERT INTO `perl_event_export_settings` (`event_id`, `event_description`, `export_qglobals`, `export_mob`, `export_zone`, `export_item`, `export_event`) VALUES (81, 'EVENT_SPAWN_ZONE', 0, 0, 0, 0, 1);
INSERT INTO `perl_event_export_settings` (`event_id`, `event_description`, `export_qglobals`, `export_mob`, `export_zone`, `export_item`, `export_event`) VALUES (82, 'EVENT_DEATH_ZONE', 0, 0, 0, 0, 1);
ALTER TABLE `rule_values`
MODIFY COLUMN `notes`  text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL AFTER `rule_value`;
)",
	},
	ManifestEntry{
		.version = 9095,
		.description = "2016_01_08_command_find_aliases.sql",
		.check = "SELECT * FROM `command_settings` WHERE `command` LIKE 'findaliases'",
		.condition = "empty",
		.match = "",
		.sql = R"(
INSERT INTO `command_settings` VALUES ('findaliases', 0, 'fa');

)",
	},
	ManifestEntry{
		.version = 9096,
		.description = "2016_03_05_secondary_recall.sql",
		.check = "SHOW COLUMNS FROM `character_bind` LIKE 'slot'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `character_bind` ADD `slot` int(4) AFTER `id`;
UPDATE `character_bind` SET `slot`='0' WHERE `is_home`=0;
UPDATE `character_bind` SET `slot`='4' WHERE `is_home`=1;
ALTER TABLE `character_bind` DROP PRIMARY KEY, ADD PRIMARY KEY(`id`, `slot`);
ALTER TABLE `character_bind` DROP COLUMN `is_home`;


)",
	},
	ManifestEntry{
		.version = 9097,
		.description = "2016_07_03_npc_class_as_last_name.sql",
		.check = "SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'NPC:UseClassAsLastName'",
		.condition = "empty",
		.match = "",
		.sql = R"(
INSERT INTO `rule_values` (`ruleset_id`, `rule_name`, `rule_value`, `notes`) VALUES
(1, 'NPC:UseClassAsLastName', 'true', 'Uses class archetype as LastName for npcs with none');

)",
	},
	ManifestEntry{
		.version = 9098,
		.description = "2016_08_26_object_size_tilt.sql",
		.check = "SHOW COLUMNS FROM `object` LIKE 'size'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `object`
	ADD COLUMN `size` FLOAT NOT NULL DEFAULT '100' AFTER `unknown84`,
	ADD COLUMN `tilt_x` FLOAT NOT NULL DEFAULT '0' AFTER `size`,
	ADD COLUMN `tilt_y` FLOAT NOT NULL DEFAULT '0' AFTER `tilt_x`;
)",
	},
	ManifestEntry{
		.version = 9099,
		.description = "2016_08_27_ip_exemptions.sql",
		.check = "SHOW TABLES LIKE 'ip_exemptions'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- IP Exemptions table structure
DROP TABLE IF EXISTS `ip_exemptions`;
CREATE TABLE `ip_exemptions` (
  `exemption_id` int(11) NOT NULL AUTO_INCREMENT,
  `exemption_ip` varchar(255) DEFAULT NULL,
  `exemption_amount` int(11) DEFAULT NULL,
  PRIMARY KEY (`exemption_id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=latin1;

-- Rule Value Entry, Default to false
INSERT INTO `rule_values` (`ruleset_id`, `rule_name`, `rule_value`, `notes`) VALUES ('1', 'World:EnableIPExemptions', 'false', 'notation');

-- Logging Category Entry
INSERT INTO `logsys_categories` (`log_category_id`, `log_category_description`, `log_to_console`, `log_to_file`, `log_to_gmsay`) VALUES ('44', 'Client Login', '1', '1', '1');
)",
	},
	ManifestEntry{
		.version = 9100,
		.description = "2016_08_27_object_display_name.sql",
		.check = "SHOW COLUMNS FROM `object` LIKE 'display_name'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `object` ADD COLUMN `display_name` VARCHAR(64);

)",
	},
	ManifestEntry{
		.version = 9101,
		.description = "2016_12_01_pcnpc_only.sql",
		.check = "SHOW COLUMNS FROM `spells_new` LIKE 'pcnpc_only_flag'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `spells_new` CHANGE `field183` `pcnpc_only_flag` INT(11) DEFAULT 0;
ALTER TABLE `spells_new` CHANGE `field184` `cast_not_standing` INT(11) DEFAULT 0;

)",
	},
	ManifestEntry{
		.version = 9102,
		.description = "2017_01_10_book_languages.sql",
		.check = "SHOW COLUMNS FROM `books` LIKE 'language'",
		.condition = "empty",
		.match = "",
		.sql = R"(
alter table books add language int not null default 0;

drop table if exists reading_is_fundamental;

create table reading_is_fundamental
(
filename varchar(32),
language int
);

insert into reading_is_fundamental (select items.filename, items.booktype from items where items.filename  != "" group by filename);

update books set books.language = (select language from reading_is_fundamental r where r.filename = books.name);

drop table reading_is_fundamental;

)",
	},
	ManifestEntry{
		.version = 9103,
		.description = "2017_01_30_book_languages_fix.sql",
		.check = "SELECT `language` from `books` WHERE `language` IS NULL",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
UPDATE `books` SET `language` = '0' WHERE `language` IS NULL;

ALTER TABLE `books` MODIFY COLUMN `language` INT NOT NULL DEFAULT '0';


)",
	},
	ManifestEntry{
		.version = 9104,
		.description = "2017_02_09_npc_spells_entries_type_update.sql",
		.check = "SHOW COLUMNS IN `npc_spells_entries` LIKE 'type'",
		.condition = "contains",
		.match = "smallint(5) unsigned",
		.sql = R"(
ALTER TABLE `npc_spells_entries` MODIFY COLUMN `type` INT(10) UNSIGNED NOT NULL DEFAULT '0';

)",
	},
	ManifestEntry{
		.version = 9105,
		.description = "2017_02_15_bot_spells_entries.sql",
		.check = "SELECT `id` FROM `npc_spells_entries` WHERE `npc_spells_id` >= 701 AND `npc_spells_id` <= 712",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
-- Delete any existing `bots_spells_entries` table
DROP TABLE IF EXISTS `bots_spells_entries`;

-- Create new bot spells entries table (new table does not have spells_id_spellid constraint)
CREATE TABLE `bot_spells_entries` (
	`id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
	`npc_spells_id` INT(11) NOT NULL DEFAULT '0',
	`spellid` SMALLINT(5) NOT NULL DEFAULT '0',
	`type` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`minlevel` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
	`maxlevel` TINYINT(3) UNSIGNED NOT NULL DEFAULT '255',
	`manacost` SMALLINT(5) NOT NULL DEFAULT '-1',
	`recast_delay` INT(11) NOT NULL DEFAULT '-1',
	`priority` SMALLINT(5) NOT NULL DEFAULT '0',
	`resist_adjust` INT(11) NULL DEFAULT NULL,
	PRIMARY KEY (`id`)
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
AUTO_INCREMENT=1
;

-- Copy bots spells into new table
INSERT INTO `bot_spells_entries` (`npc_spells_id`, `spellid`, `type`, `minlevel`, `maxlevel`, `manacost`, `recast_delay`, `priority`, `resist_adjust`)
SELECT `npc_spells_id`, `spellid`, `type`, `minlevel`, `maxlevel`, `manacost`, `recast_delay`, `priority`, `resist_adjust`
FROM `npc_spells_entries` WHERE `npc_spells_id` >= '701' AND `npc_spells_id` <= '712';

-- Delete bot spells from old table
DELETE FROM `npc_spells_entries` WHERE `npc_spells_id` >= '701' AND `npc_spells_id` <= '712';

-- Admins can remove this new table if they are 100% certain they will never use bots

)",
	},
	ManifestEntry{
		.version = 9107,
		.description = "2017_03_09_inventory_version.sql",
		.check = "SHOW TABLES LIKE 'inventory_version'",
		.condition = "empty",
		.match = "",
		.sql = R"(
DROP TABLE IF EXISTS `inventory_version`;

CREATE TABLE `inventory_version` (
	`version` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`step` INT(11) UNSIGNED NOT NULL DEFAULT '0'
)
COLLATE='latin1_swedish_ci'
ENGINE=MyISAM
;

INSERT INTO `inventory_version` VALUES (2, 0);

)",
	},
	ManifestEntry{
		.version = 9108,
		.description = "2017_04_07_ignore_despawn.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'ignore_despawn'",
		.condition = "empty",
		.match = "",
		.sql = R"(
alter table npc_types add column `ignore_despawn` tinyint(2) not null default 0;
)",
	},
	ManifestEntry{
		.version = 9109,
		.description = "2017_04_08_doors_disable_timer.sql",
		.check = "SHOW COLUMNS FROM `doors` LIKE 'disable_timer'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `doors` ADD COLUMN `disable_timer` TINYINT(2) NOT NULL DEFAULT '0' AFTER `triggertype`;
)",
	},
	ManifestEntry{
		.version = 9110,
		.description = "2017_04_10_graveyard.sql",
		.check = "show index from graveyard WHERE key_name = 'zone_id_nonunique'",
		.condition = "empty",
		.match = "",
		.sql = R"(
alter table graveyard drop index zone_id;
create index zone_id_nonunique on graveyard(zone_id);

)",
	},
	ManifestEntry{
		.version = 9111,
		.description = "2017_06_24_saylink_index.sql",
		.check = "SHOW INDEX FROM `saylink` WHERE `key_name` = 'phrase_index'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `saylink`
ADD INDEX `phrase_index` (`phrase`) USING BTREE ;
)",
	},
	ManifestEntry{
		.version = 9112,
		.description = "2017_06_24_rule_values_expand.sql",
		.check = "SHOW COLUMNS FROM rule_values WHERE Field = 'rule_value' and Type = 'varchar(30)'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `rule_values`
MODIFY COLUMN `rule_value`  varchar(30) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '' AFTER `rule_name`;

)",
	},
	ManifestEntry{
		.version = 9113,
		.description = "2017_07_19_show_name.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'show_name'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types` ADD COLUMN `show_name` TINYINT(2) NOT NULL DEFAULT 1;
ALTER TABLE `npc_types` ADD COLUMN `untargetable` TINYINT(2) NOT NULL DEFAULT 0;
UPDATE `npc_types` SET `show_name` = 0, `untargetable` = 1 WHERE `bodytype` >= 66;

)",
	},
	ManifestEntry{
		.version = 9114,
		.description = "2017_07_22_aura.sql",
		.check = "SHOW TABLES LIKE 'auras'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `auras` (
	`type` INT(10) NOT NULL,
	`npc_type` INT(10) NOT NULL,
	`name` VARCHAR(64) NOT NULL,
	`spell_id` INT(10) NOT NULL,
	`distance` INT(10) NOT NULL DEFAULT 60,
	`aura_type` INT(10) NOT NULL DEFAULT 1,
	`spawn_type` INT(10) NOT NULL DEFAULT 0,
	`movement` INT(10) NOT NULL DEFAULT 0,
	`duration` INT(10) NOT NULL DEFAULT 5400,
	`icon` INT(10) NOT NULL DEFAULT -1,
	`cast_time` INT(10) NOT NULL DEFAULT 0,
	PRIMARY KEY(`type`)
);

CREATE TABLE `character_auras` (
	`id` INT(10) NOT NULL,
	`slot` TINYINT(10) NOT NULL,
	`spell_id` INT(10) NOT NULL,
	PRIMARY KEY (`id`, `slot`)
);

)",
	},
	ManifestEntry{
		.version = 9115,
		.description = "2017_10_28_traps.sql",
		.check = "SHOW COLUMNS FROM `traps` LIKE 'triggered_number'",
		.condition = "empty",
		.match = "",
		.sql = R"(
alter table `traps` add column `triggered_number` tinyint(4) not null default 0;
alter table `traps` add column `group` tinyint(4) not null default 0;
alter table `traps` add column `despawn_when_triggered` tinyint(4) not null default 0;
alter table `traps` add column `undetectable` tinyint(4) not null default 0;

)",
	},
	ManifestEntry{
		.version = 9116,
		.description = "2017_12_16_groundspawn_respawn_timer.sql",
		.check = "SHOW COLUMNS FROM `ground_spawns` WHERE Field = 'respawn_timer' AND Type = 'int(11) unsigned'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `ground_spawns` MODIFY `respawn_timer` int(11) unsigned NOT NULL default 300;
UPDATE `ground_spawns` SET `respawn_timer` = `respawn_timer` / 1000;

)",
	},
	ManifestEntry{
		.version = 9117,
		.description = "2018_02_01_npc_spells_min_max_hp.sql",
		.check = "SHOW COLUMNS FROM `npc_spells_entries` LIKE 'min_hp'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_spells_entries` ADD `min_hp` SMALLINT(5) DEFAULT '0';
ALTER TABLE `npc_spells_entries` ADD `max_hp` SMALLINT(5) DEFAULT '0';

)",
	},
	ManifestEntry{
		.version = 9118,
		.description = "2018_02_04_charm_stats.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'charm_ac'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types` ADD `charm_ac` SMALLINT(5) DEFAULT '0';
ALTER TABLE `npc_types` ADD `charm_min_dmg` INT(10) DEFAULT '0';
ALTER TABLE `npc_types` ADD `charm_max_dmg` INT(10) DEFAULT '0';
ALTER TABLE `npc_types` ADD `charm_attack_delay` TINYINT(3) DEFAULT '0';
ALTER TABLE `npc_types` ADD `charm_accuracy_rating` MEDIUMINT(9) DEFAULT '0';
ALTER TABLE `npc_types` ADD `charm_avoidance_rating` MEDIUMINT(9) DEFAULT '0';
ALTER TABLE `npc_types` ADD `charm_atk` MEDIUMINT(9) DEFAULT '0';

)",
	},
	ManifestEntry{
		.version = 9119,
		.description = "2018_02_10_globalloot.sql",
		.check = "SHOW TABLES LIKE 'global_loot'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types` ADD `skip_global_loot` TINYINT DEFAULT '0';
ALTER TABLE `npc_types` ADD `rare_spawn` TINYINT DEFAULT '0';

CREATE TABLE global_loot (
	id INT NOT NULL AUTO_INCREMENT,
	description varchar(255),
	loottable_id INT NOT NULL,
	enabled TINYINT NOT NULL DEFAULT 1,
	min_level INT NOT NULL DEFAULT 0,
	max_level INT NOT NULL DEFAULT 0,
	rare TINYINT NULL,
	raid TINYINT NULL,
	race MEDIUMTEXT NULL,
	class MEDIUMTEXT NULL,
	bodytype MEDIUMTEXT NULL,
	zone MEDIUMTEXT NULL,
	PRIMARY KEY (id)
);


)",
	},
	ManifestEntry{
		.version = 9120,
		.description = "2018_02_13_heading.sql",
		.check = "SELECT value FROM variables WHERE varname = 'fixed_heading'",
		.condition = "empty",
		.match = "",
		.sql = R"(
UPDATE spawn2 SET heading = heading * 8.0 / 4.0;
UPDATE grid_entries SET heading = heading * 8.0 / 4.0 WHERE heading <> -1;
INSERT INTO variables (varname, value, information) VALUES ('fixed_heading', 1, 'manifest heading fix hack'); -- hack

)",
	},
	ManifestEntry{
		.version = 9121,
		.description = "2018_02_18_bug_reports.sql",
		.check = "SHOW TABLES LIKE 'bug_reports'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `bug_reports` (
	`id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
	`zone` VARCHAR(32) NOT NULL DEFAULT 'Unknown',
	`client_version_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`client_version_name` VARCHAR(24) NOT NULL DEFAULT 'Unknown',
	`account_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`character_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`character_name` VARCHAR(64) NOT NULL DEFAULT 'Unknown',
	`reporter_spoof` TINYINT(1) NOT NULL DEFAULT '1',
	`category_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`category_name` VARCHAR(64) NOT NULL DEFAULT 'Other',
	`reporter_name` VARCHAR(64) NOT NULL DEFAULT 'Unknown',
	`ui_path` VARCHAR(128) NOT NULL DEFAULT 'Unknown',
	`pos_x` FLOAT NOT NULL DEFAULT '0',
	`pos_y` FLOAT NOT NULL DEFAULT '0',
	`pos_z` FLOAT NOT NULL DEFAULT '0',
	`heading` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`time_played` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`target_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`target_name` VARCHAR(64) NOT NULL DEFAULT 'Unknown',
	`optional_info_mask` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`_can_duplicate` TINYINT(1) NOT NULL DEFAULT '0',
	`_crash_bug` TINYINT(1) NOT NULL DEFAULT '0',
	`_target_info` TINYINT(1) NOT NULL DEFAULT '0',
	`_character_flags` TINYINT(1) NOT NULL DEFAULT '0',
	`_unknown_value` TINYINT(1) NOT NULL DEFAULT '0',
	`bug_report` VARCHAR(1024) NOT NULL DEFAULT '',
	`system_info` VARCHAR(1024) NOT NULL DEFAULT '',
	`report_datetime` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
	`bug_status` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
	`last_review` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	`last_reviewer` VARCHAR(64) NOT NULL DEFAULT 'None',
	`reviewer_notes` VARCHAR(1024) NOT NULL DEFAULT '',
	PRIMARY KEY (`id`),
	UNIQUE INDEX `id` (`id`)
)
COLLATE='utf8_general_ci'
ENGINE=InnoDB
;

INSERT INTO `rule_values` (`ruleset_id`, `rule_name`, `rule_value`, `notes`) VALUES
(1, 'Bugs:ReportingSystemActive', 'true', 'Activates bug reporting'),
(1, 'Bugs:UseOldReportingMethod', 'true', 'Forces the use of the old bug reporting system'),
(1, 'Bugs:DumpTargetEntity', 'false', 'Dumps the target entity, if one is provided');

)",
	},
	ManifestEntry{
		.version = 9122,
		.description = "2018_03_07_ucs_command.sql",
		.check = "SELECT * FROM `command_settings` WHERE `command` LIKE 'ucs'",
		.condition = "empty",
		.match = "",
		.sql = R"(
INSERT INTO `command_settings` VALUES ('ucs', '0', '');

)",
	},
	ManifestEntry{
		.version = 9123,
		.description = "2018_07_07_data_buckets.sql",
		.check = "SHOW TABLES LIKE 'data_buckets'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `data_buckets` (
  `id` bigint(11) unsigned NOT NULL AUTO_INCREMENT,
  `key` varchar(100) DEFAULT NULL,
  `value` text,
  `expires` int(11) unsigned DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `key_index` (`key`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4;
)",
	},
	ManifestEntry{
		.version = 9124,
		.description = "2018_07_09_tasks.sql",
		.check = "SHOW COLUMNS FROM `tasks` LIKE 'type'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `tasks` ADD `type` TINYINT NOT NULL DEFAULT '0' AFTER `id`;
ALTER TABLE `tasks` ADD `duration_code` TINYINT NOT NULL DEFAULT '0' AFTER `duration`;
UPDATE `tasks` SET `type` = '2'; -- we were treating them all as quests
ALTER TABLE `character_tasks` ADD `type` TINYINT NOT NULL DEFAULT '0' AFTER `slot`;
UPDATE `character_tasks` SET `type` = '2'; -- we were treating them all as quests
ALTER TABLE `activities` ADD `target_name` VARCHAR(64) NOT NULL DEFAULT '' AFTER `activitytype`;
ALTER TABLE `activities` ADD `item_list` VARCHAR(128) NOT NULL DEFAULT '' AFTER `target_name`;
ALTER TABLE `activities` ADD `skill_list` VARCHAR(64) NOT NULL DEFAULT '-1' AFTER `item_list`;
ALTER TABLE `activities` ADD `spell_list` VARCHAR(64) NOT NULL DEFAULT '0' AFTER `skill_list`;
ALTER TABLE `activities` ADD `description_override` VARCHAR(128) NOT NULL DEFAULT '' AFTER `spell_list`;
ALTER TABLE `activities` ADD `zones` VARCHAR(64) NOT NULL DEFAULT '' AFTER `zoneid`;
UPDATE `activities` SET `description_override` = `text3`;
UPDATE `activities` SET `target_name` = `text1`;
UPDATE `activities` SET `item_list` = `text2`;
UPDATE `activities` SET `zones` = `zoneid`; -- should be safe for us ...
ALTER TABLE `activities` DROP COLUMN `text1`;
ALTER TABLE `activities` DROP COLUMN `text2`;
ALTER TABLE `activities` DROP COLUMN `text3`;
ALTER TABLE `activities` DROP COLUMN `zoneid`;
ALTER TABLE `tasks` DROP COLUMN `startzone`;
ALTER TABLE `tasks` ADD `faction_reward` INT(10) NOT NULL DEFAULT '0';
RENAME TABLE `activities` TO `task_activities`;
)",
	},
	ManifestEntry{
		.version = 9125,
		.description = "2018_07_20_task_emote.sql",
		.check = "SHOW COLUMNS FROM `tasks` LIKE 'completion_emote'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `tasks` ADD `completion_emote` VARCHAR(128) NOT NULL DEFAULT '';

)",
	},
	ManifestEntry{
		.version = 9126,
		.description = "2018_09_07_fastregen.sql",
		.check = "SHOW COLUMNS FROM `zone` LIKE 'fast_regen_hp'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `zone` ADD `fast_regen_hp` INT NOT NULL DEFAULT '180';
ALTER TABLE `zone` ADD `fast_regen_mana` INT NOT NULL DEFAULT '180';
ALTER TABLE `zone` ADD `fast_regen_endurance` INT NOT NULL DEFAULT '180';

)",
	},
	ManifestEntry{
		.version = 9127,
		.description = "2018_09_07_npcmaxaggrodist.sql",
		.check = "SHOW COLUMNS FROM `zone` LIKE 'npc_max_aggro_dist'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `zone` ADD `npc_max_aggro_dist` INT NOT NULL DEFAULT '600';

)",
	},
	ManifestEntry{
		.version = 9128,
		.description = "2018_08_13_inventory_version_update.sql",
		.check = "SHOW TABLES LIKE 'inventory_version'",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
DROP TABLE IF EXISTS `inventory_version`;

)",
	},
	ManifestEntry{
		.version = 9130,
		.description = "2018_11_25_name_filter_update.sql",
		.check = "SHOW COLUMNS FROM `name_filter` LIKE 'id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `name_filter`
ADD COLUMN `id` INT(11) NULL AUTO_INCREMENT FIRST,
DROP PRIMARY KEY,
ADD PRIMARY KEY (`id`) USING BTREE,
ADD INDEX `name_search_index`(`name`);

)",
	},
	ManifestEntry{
		.version = 9131,
		.description = "2018_12_13_spell_buckets.sql",
		.check = "SHOW TABLES LIKE 'spell_buckets'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `spell_buckets` (
  `spellid` bigint(11) unsigned NOT NULL,
  `key` varchar(100) DEFAULT NULL,
  `value` text,
  PRIMARY KEY (`spellid`),
  KEY `key_index` (`key`) USING BTREE
) ENGINE=InnoDB  DEFAULT CHARSET=utf8mb4;

INSERT INTO `rule_values` (`ruleset_id`, `rule_name`, `rule_value`, `notes`) VALUES (1, 'Spells:EnableSpellBuckets', 'false', 'Enables spell buckets');
INSERT INTO `rule_values` (`ruleset_id`, `rule_name`, `rule_value`, `notes`) VALUES (1, 'Character:PerCharacterBucketMaxLevel', 'false', 'Enables data bucket-based max level.');
)",
	},
	ManifestEntry{
		.version = 9132,
		.description = "2018_12_16_global_base_scaling.sql",
		.check = "SHOW TABLES LIKE 'npc_scale_global_base'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- INSERT #devtools / #dev command

INSERT INTO `command_settings` (`command`, `access`, `aliases`)
VALUES
	('devtools', 200, 'dev');

-- CREATE 'npc_scale_global_base'

CREATE TABLE `npc_scale_global_base` (
  `type` int(11) NOT NULL DEFAULT '0',
  `level` int(11) NOT NULL,
  `ac` int(11) DEFAULT NULL,
  `hp` int(11) DEFAULT NULL,
  `accuracy` int(11) DEFAULT NULL,
  `slow_mitigation` int(11) DEFAULT NULL,
  `attack` int(11) DEFAULT NULL,
  `strength` int(11) DEFAULT NULL,
  `stamina` int(11) DEFAULT NULL,
  `dexterity` int(11) DEFAULT NULL,
  `agility` int(11) DEFAULT NULL,
  `intelligence` int(11) DEFAULT NULL,
  `wisdom` int(11) DEFAULT NULL,
  `charisma` int(11) DEFAULT NULL,
  `magic_resist` int(11) DEFAULT NULL,
  `cold_resist` int(11) DEFAULT NULL,
  `fire_resist` int(11) DEFAULT NULL,
  `poison_resist` int(11) DEFAULT NULL,
  `disease_resist` int(11) DEFAULT NULL,
  `corruption_resist` int(11) DEFAULT NULL,
  `physical_resist` int(11) DEFAULT NULL,
  `min_dmg` int(11) DEFAULT NULL,
  `max_dmg` int(11) DEFAULT NULL,
  `hp_regen_rate` int(11) DEFAULT NULL,
  `attack_delay` int(11) DEFAULT NULL,
  `spell_scale` int(11) DEFAULT '100',
  `heal_scale` int(11) DEFAULT '100',
  `special_abilities` text,
  PRIMARY KEY (`type`,`level`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

)",
	},
	ManifestEntry{
		.version = 9133,
		.description = "2018_11_25_stuckbehavior.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'stuck_behavior'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types` ADD COLUMN `stuck_behavior` TINYINT(4) NOT NULL DEFAULT '0' AFTER `rare_spawn`;
UPDATE `npc_types` SET `stuck_behavior`=2 WHERE `underwater`=1;

)",
	},
	ManifestEntry{
		.version = 9135,
		.description = "2019_01_10_multi_version_spawns.sql",
		.check = "SHOW COLUMNS FROM `spawn2` LIKE 'version'",
		.condition = "contains",
		.match = "unsigned",
		.sql = R"(
ALTER TABLE `spawn2` MODIFY `version` SMALLINT(5) SIGNED NOT NULL DEFAULT '0';
)",
	},
	ManifestEntry{
		.version = 9136,
		.description = "2019_02_04_profanity_command.sql",
		.check = "SHOW TABLES LIKE 'profanity_list'",
		.condition = "empty",
		.match = "",
		.sql = R"(
DROP TABLE IF EXISTS `profanity_list`;

CREATE TABLE `profanity_list` (
	`word` VARCHAR(16) NOT NULL
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
;

REPLACE INTO `command_settings` VALUES ('profanity', 150, 'prof');

)",
	},
	ManifestEntry{
		.version = 9137,
		.description = "2018_12_12_client_faction_tables.sql",
		.check = "SHOW TABLES LIKE 'faction_base_data'",
		.condition = "empty",
		.match = "",
		.sql = R"(
--
-- Table structure for table `client_faction_associations`
--

DROP TABLE IF EXISTS `client_faction_associations`;

CREATE TABLE `client_faction_associations` (
  `faction_id` int(11) NOT NULL,
  `other_faction_id` int(11) NOT NULL,
  `mod` int(11) DEFAULT NULL,
  PRIMARY KEY (`faction_id`,`other_faction_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
)",
	},
	ManifestEntry{
		.version = 9139,
		.description = "2019_03_25_optional_npc_model.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'model'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types` ADD COLUMN `model` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `stuck_behavior`;
)",
	},
	ManifestEntry{
		.version = 9140,
		.description = "2019_07_03_update_range.sql",
		.check = "SHOW COLUMNS FROM `zone` LIKE 'max_movement_update_range'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `zone` ADD COLUMN `max_movement_update_range` INT(11) UNSIGNED NOT NULL DEFAULT '600' AFTER `npc_max_aggro_dist`;
)",
	},
	ManifestEntry{
		.version = 9141,
		.description = "2019_07_10_npc_flymode.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'flymode'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types` ADD COLUMN `flymode` tinyint(4) NOT NULL DEFAULT -1;
)",
	},
	ManifestEntry{
		.version = 9142,
		.description = "2019_09_02_required_spawn_filter.sql",
		.check = "SHOW COLUMNS FROM `spawnentry` LIKE 'condition_value_filter'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `spawnentry` ADD COLUMN `condition_value_filter` MEDIUMINT(9) NOT NULL DEFAULT '1' AFTER `chance`;

)",
	},
	ManifestEntry{
		.version = 9143,
		.description = "2019_09_16_account_table_changes.sql",
		.check = "SHOW COLUMNS FROM `account` LIKE 'ls_id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `account`
	DROP INDEX `name`,
	DROP INDEX `lsaccount_id`;

ALTER TABLE `account`
	ADD COLUMN `ls_id` VARCHAR(64) NULL DEFAULT 'eqemu' AFTER `status`;

ALTER TABLE `account`
	ADD UNIQUE INDEX `name_ls_id` (`name`, `ls_id`),
	ADD UNIQUE INDEX `ls_id_lsaccount_id` (`ls_id`, `lsaccount_id`);

)",
	},
	ManifestEntry{
		.version = 9145,
		.description = "2019_12_24_banned_ips_update.sql",
		.check = "SHOW TABLES LIKE 'Banned_IPs'",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
RENAME TABLE `Banned_IPs` TO `Banned_IPs_`;

CREATE TABLE `banned_ips` (PRIMARY KEY (`ip_address`)) SELECT `ip_address`, `notes` FROM `Banned_IPs_`;

DROP TABLE IF EXISTS `Banned_IPs_`;

)",
	},
	ManifestEntry{
		.version = 9146,
		.description = "2020_01_10_character_soft_deletes.sql",
		.check = "SHOW COLUMNS FROM `character_data` LIKE 'deleted_at'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `character_data` ADD COLUMN `deleted_at` datetime NULL DEFAULT NULL;
)",
	},
	ManifestEntry{
		.version = 9147,
		.description = "2020_01_24_grid_centerpoint_wp.sql",
		.check = "SHOW COLUMNS FROM `grid_entries` LIKE 'centerpoint'",
		.condition = "empty",
		.match = "",
		.sql = R"(
alter table grid_entries add column `centerpoint` tinyint(4) not null default 0;
alter table spawngroup add column `wp_spawns` tinyint(1) unsigned not null default 0;
)",
	},
	ManifestEntry{
		.version = 9148,
		.description = "2020_01_28_corpse_guild_consent_id.sql",
		.check = "SHOW COLUMNS FROM `character_corpses` LIKE 'guild_consent_id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `character_corpses` ADD COLUMN `guild_consent_id` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `time_of_death`;

)",
	},
	ManifestEntry{
		.version = 9149,
		.description = "2020_02_06_globalloot.sql",
		.check = "SHOW COLUMNS FROM `global_loot` LIKE 'hot_zone'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `global_loot` ADD `hot_zone` TINYINT NULL;


)",
	},
	ManifestEntry{
		.version = 9150,
		.description = "2020_02_06_aa_reset_on_death.sql",
		.check = "SHOW COLUMNS FROM `aa_ability` LIKE 'reset_on_death'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `aa_ability` ADD `reset_on_death` TINYINT(4) NOT NULL DEFAULT '0';
UPDATE `aa_ability` SET `reset_on_death` = '1' WHERE `id` = 6001;

)",
	},
	ManifestEntry{
		.version = 9151,
		.description = "2020_03_05_npc_always_aggro.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'always_aggro'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types` ADD COLUMN `always_aggro` tinyint(1) NOT NULL DEFAULT 0;

)",
	},
	ManifestEntry{
		.version = 9152,
		.description = "2020_03_09_convert_myisam_to_innodb.sql",
		.check = "SELECT * FROM db_version WHERE version >= 9152",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `account_flags` ENGINE=InnoDB;
ALTER TABLE `account_ip` ENGINE=InnoDB;
ALTER TABLE `account` ENGINE=InnoDB;
ALTER TABLE `adventure_template_entry_flavor` ENGINE=InnoDB;
ALTER TABLE `adventure_template_entry` ENGINE=InnoDB;
ALTER TABLE `altadv_vars` ENGINE=InnoDB;
ALTER TABLE `alternate_currency` ENGINE=InnoDB;
ALTER TABLE `banned_ips` ENGINE=InnoDB;
ALTER TABLE `base_data` ENGINE=InnoDB;
ALTER TABLE `blocked_spells` ENGINE=InnoDB;
ALTER TABLE `buyer` ENGINE=InnoDB;
ALTER TABLE `char_create_combinations` ENGINE=InnoDB;
ALTER TABLE `char_create_point_allocations` ENGINE=InnoDB;
ALTER TABLE `character_activities` ENGINE=InnoDB;
ALTER TABLE `character_enabledtasks` ENGINE=InnoDB;
ALTER TABLE `character_tasks` ENGINE=InnoDB;
ALTER TABLE `chatchannels` ENGINE=InnoDB;
ALTER TABLE `completed_tasks` ENGINE=InnoDB;
ALTER TABLE `damageshieldtypes` ENGINE=InnoDB;
ALTER TABLE `discovered_items` ENGINE=InnoDB;
ALTER TABLE `eqtime` ENGINE=InnoDB;
ALTER TABLE `eventlog` ENGINE=InnoDB;
ALTER TABLE `faction_list_mod` ENGINE=InnoDB;
ALTER TABLE `faction_list` ENGINE=InnoDB;
ALTER TABLE `faction_values` ENGINE=InnoDB;
ALTER TABLE `friends` ENGINE=InnoDB;
ALTER TABLE `goallists` ENGINE=InnoDB;
ALTER TABLE `guild_bank` ENGINE=InnoDB;
ALTER TABLE `guild_members` ENGINE=InnoDB;
ALTER TABLE `guild_ranks` ENGINE=InnoDB;
ALTER TABLE `guild_relations` ENGINE=InnoDB;
ALTER TABLE `guilds` ENGINE=InnoDB;
ALTER TABLE `hackers` ENGINE=InnoDB;
ALTER TABLE `horses` ENGINE=InnoDB;
ALTER TABLE `inventory_versions` ENGINE=InnoDB;
ALTER TABLE `item_tick` ENGINE=InnoDB;
ALTER TABLE `items` ENGINE=InnoDB;
ALTER TABLE `keyring` ENGINE=InnoDB;
ALTER TABLE `launcher_zones` ENGINE=InnoDB;
ALTER TABLE `launcher` ENGINE=InnoDB;
ALTER TABLE `ldon_trap_entries` ENGINE=InnoDB;
ALTER TABLE `ldon_trap_templates` ENGINE=InnoDB;
ALTER TABLE `lfguild` ENGINE=InnoDB;
ALTER TABLE `lootdrop_entries` ENGINE=InnoDB;
ALTER TABLE `lootdrop` ENGINE=InnoDB;
ALTER TABLE `loottable_entries` ENGINE=InnoDB;
ALTER TABLE `loottable` ENGINE=InnoDB;
ALTER TABLE `mail` ENGINE=InnoDB;
ALTER TABLE `merc_armorinfo` ENGINE=InnoDB;
ALTER TABLE `merc_buffs` ENGINE=InnoDB;
ALTER TABLE `merc_inventory` ENGINE=InnoDB;
ALTER TABLE `merc_merchant_entries` ENGINE=InnoDB;
ALTER TABLE `merc_merchant_template_entries` ENGINE=InnoDB;
ALTER TABLE `merc_merchant_templates` ENGINE=InnoDB;
ALTER TABLE `merc_name_types` ENGINE=InnoDB;
ALTER TABLE `merc_npc_types` ENGINE=InnoDB;
ALTER TABLE `merc_spell_list_entries` ENGINE=InnoDB;
ALTER TABLE `merc_spell_lists` ENGINE=InnoDB;
ALTER TABLE `merc_stance_entries` ENGINE=InnoDB;
ALTER TABLE `merc_stats` ENGINE=InnoDB;
ALTER TABLE `merc_subtypes` ENGINE=InnoDB;
ALTER TABLE `merc_templates` ENGINE=InnoDB;
ALTER TABLE `merc_types` ENGINE=InnoDB;
ALTER TABLE `merc_weaponinfo` ENGINE=InnoDB;
ALTER TABLE `mercs` ENGINE=InnoDB;
ALTER TABLE `name_filter` ENGINE=InnoDB;
ALTER TABLE `npc_types` ENGINE=InnoDB;
ALTER TABLE `object_contents` ENGINE=InnoDB;
ALTER TABLE `petitions` ENGINE=InnoDB;
ALTER TABLE `pets_equipmentset_entries` ENGINE=InnoDB;
ALTER TABLE `pets_equipmentset` ENGINE=InnoDB;
ALTER TABLE `player_titlesets` ENGINE=InnoDB;
ALTER TABLE `proximities` ENGINE=InnoDB;
ALTER TABLE `races` ENGINE=InnoDB;
ALTER TABLE `raid_details` ENGINE=InnoDB;
ALTER TABLE `raid_leaders` ENGINE=InnoDB;
ALTER TABLE `raid_members` ENGINE=InnoDB;
ALTER TABLE `rule_sets` ENGINE=InnoDB;
ALTER TABLE `rule_values` ENGINE=InnoDB;
ALTER TABLE `saylink` ENGINE=InnoDB;
ALTER TABLE `sharedbank` ENGINE=InnoDB;
ALTER TABLE `skill_caps` ENGINE=InnoDB;
ALTER TABLE `spell_globals` ENGINE=InnoDB;
ALTER TABLE `spells_new` ENGINE=InnoDB;
ALTER TABLE `task_activities` ENGINE=InnoDB;
ALTER TABLE `tasks` ENGINE=InnoDB;
ALTER TABLE `tasksets` ENGINE=InnoDB;
ALTER TABLE `timers` ENGINE=InnoDB;
ALTER TABLE `titles` ENGINE=InnoDB;
ALTER TABLE `trader_audit` ENGINE=InnoDB;
ALTER TABLE `trader` ENGINE=InnoDB;
ALTER TABLE `tradeskill_recipe_entries` ENGINE=InnoDB;
ALTER TABLE `tradeskill_recipe` ENGINE=InnoDB;
ALTER TABLE `variables` ENGINE=InnoDB;
ALTER TABLE `veteran_reward_templates` ENGINE=InnoDB;
)",
	},
	ManifestEntry{
		.version = 9153,
		.description = "2020_05_09_items_subtype.sql",
		.check = "SHOW COLUMNS from `items` LIKE 'UNK219'",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
ALTER TABLE `items` CHANGE `UNK219` `subtype` int(11) not null default '0';

)",
	},
	ManifestEntry{
		.version = 9154,
		.description = "2020_04_11_expansions_content_filters.sql",
		.check = "SHOW COLUMNS from `zone` LIKE 'min_expansion'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- zone
ALTER TABLE `zone` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `zone` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `zone` ADD `content_flags` varchar(100) NULL;

-- doors
ALTER TABLE `doors` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `doors` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `doors` ADD `content_flags` varchar(100) NULL;

-- object
ALTER TABLE `object` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `object` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `object` ADD `content_flags` varchar(100) NULL;

-- spawn2
ALTER TABLE `spawn2` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `spawn2` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `spawn2` ADD `content_flags` varchar(100) NULL;

-- tradeskill_recipe
ALTER TABLE `tradeskill_recipe` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `tradeskill_recipe` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `tradeskill_recipe` ADD `content_flags` varchar(100) NULL;

-- merchantlist
ALTER TABLE `merchantlist` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `merchantlist` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `merchantlist` ADD `content_flags` varchar(100) NULL;

-- global_loot
ALTER TABLE `global_loot` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `global_loot` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `global_loot` ADD `content_flags` varchar(100) NULL;

-- fishing
ALTER TABLE `fishing` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `fishing` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `fishing` ADD `content_flags` varchar(100) NULL;

-- forage
ALTER TABLE `forage` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `forage` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `forage` ADD `content_flags` varchar(100) NULL;

-- ground_spawns
ALTER TABLE `ground_spawns` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `ground_spawns` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `ground_spawns` ADD `content_flags` varchar(100) NULL;

-- loottable
ALTER TABLE `loottable` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `loottable` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `loottable` ADD `content_flags` varchar(100) NULL;

-- lootdrop
ALTER TABLE `lootdrop` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `lootdrop` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `lootdrop` ADD `content_flags` varchar(100) NULL;

-- starting_items
ALTER TABLE `starting_items` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `starting_items` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `starting_items` ADD `content_flags` varchar(100) NULL;

-- start_zones
ALTER TABLE `start_zones` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `start_zones` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `start_zones` ADD `content_flags` varchar(100) NULL;

-- traps
ALTER TABLE `traps` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `traps` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `traps` ADD `content_flags` varchar(100) NULL;

-- zone_points
ALTER TABLE `zone_points` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `zone_points` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `zone_points` ADD `content_flags` varchar(100) NULL;

-- pok books
update doors set min_expansion = 4 where name like '%POKTELE%';

-- content flags
CREATE TABLE `content_flags` (`id` int AUTO_INCREMENT,`flag_name` varchar(75),`enabled` tinyint,`notes` text, PRIMARY KEY (id));

-- content flags disabled

ALTER TABLE `doors` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `fishing` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `forage` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `global_loot` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `ground_spawns` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `lootdrop` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `loottable` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `merchantlist` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `object` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `spawn2` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `start_zones` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `starting_items` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `tradeskill_recipe` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `traps` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `zone` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `zone_points` ADD `content_flags_disabled` varchar(100) NULL;
)",
	},
	ManifestEntry{
		.version = 9155,
		.description = "2020_08_15_lootdrop_level_filtering.sql",
		.check = "SHOW COLUMNS from `lootdrop_entries` LIKE 'trivial_min_level'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `lootdrop_entries` CHANGE `minlevel` `trivial_min_level` tinyint(3) unsigned NOT NULL DEFAULT 0 COMMENT '';
ALTER TABLE `lootdrop_entries` CHANGE `maxlevel` `trivial_max_level` tinyint(3) unsigned NOT NULL DEFAULT 0 COMMENT '';
ALTER TABLE `lootdrop_entries` ADD COLUMN `npc_min_level` smallint unsigned NOT NULL DEFAULT '0' COMMENT '';
ALTER TABLE `lootdrop_entries` ADD COLUMN `npc_max_level` smallint unsigned NOT NULL DEFAULT '0' COMMENT '';
ALTER TABLE `lootdrop_entries` CHANGE `trivial_min_level` `trivial_min_level` smallint(5) unsigned NOT NULL DEFAULT 0 COMMENT '';
ALTER TABLE `lootdrop_entries` CHANGE `trivial_max_level` `trivial_max_level` smallint(5) unsigned NOT NULL DEFAULT 0 COMMENT '';
UPDATE `lootdrop_entries` SET `trivial_max_level` = 0 WHERE `trivial_max_level` = 127;
)",
	},
	ManifestEntry{
		.version = 9156,
		.description = "2020_08_16_virtual_zonepoints.sql",
		.check = "SHOW COLUMNS from `zone_points` LIKE 'is_virtual'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `zone_points` ADD COLUMN `is_virtual` tinyint NOT NULL DEFAULT '0' COMMENT '' AFTER `content_flags_disabled`;
ALTER TABLE `zone_points` ADD COLUMN `height` int NOT NULL DEFAULT '0' COMMENT '';
ALTER TABLE `zone_points` ADD COLUMN `width` int NOT NULL DEFAULT '0' COMMENT '';
)",
	},
	ManifestEntry{
		.version = 9157,
		.description = "2020_09_02_pet_taunting.sql",
		.check = "SHOW COLUMNS from `character_pet_info` LIKE 'taunting'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `character_pet_info` ADD COLUMN `taunting` tinyint(1) NOT NULL DEFAULT '1' COMMENT '';

)",
	},
	ManifestEntry{
		.version = 9158,
		.description = "2020_12_09_underworld.sql",
		.check = "SHOW COLUMNS from `zone` LIKE 'underworld_teleport_index'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `zone` ADD COLUMN `underworld_teleport_index` INT(4) NOT NULL DEFAULT '0';
UPDATE `zone` SET `underworld` = '-2030' WHERE `zoneidnumber` = '71';
UPDATE `zone` SET `underworld_teleport_index` = '11' WHERE `zoneidnumber` = '71';
UPDATE `zone` SET `underworld_teleport_index` = '-1' WHERE `zoneidnumber` = '75';
UPDATE `zone` SET `underworld_teleport_index` = '-1' WHERE `zoneidnumber` = '150';


)",
	},
	ManifestEntry{
		.version = 9159,
		.description = "2020_12_22_expedition_system.sql",
		.check = "SELECT * FROM db_version WHERE version >= 9159",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `expeditions` (
	`id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`uuid` VARCHAR(36) NOT NULL,
	`dynamic_zone_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`expedition_name` VARCHAR(128) NOT NULL,
	`leader_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`min_players` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	`max_players` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	`add_replay_on_join` TINYINT(3) UNSIGNED NOT NULL DEFAULT 1,
	`is_locked` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	PRIMARY KEY (`id`),
	UNIQUE INDEX `dynamic_zone_id` (`dynamic_zone_id`)
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
;

CREATE TABLE `expedition_lockouts` (
	`id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`expedition_id` INT(10) UNSIGNED NOT NULL,
	`event_name` VARCHAR(256) NOT NULL,
	`expire_time` DATETIME NOT NULL DEFAULT current_timestamp(),
	`duration` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`from_expedition_uuid` VARCHAR(36) NOT NULL,
	PRIMARY KEY (`id`),
	UNIQUE INDEX `expedition_id_event_name` (`expedition_id`, `event_name`)
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
;

CREATE TABLE `expedition_members` (
	`id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`expedition_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`character_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`is_current_member` TINYINT(3) UNSIGNED NOT NULL DEFAULT 1,
	PRIMARY KEY (`id`),
	UNIQUE INDEX `expedition_id_character_id` (`expedition_id`, `character_id`)
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
;

CREATE TABLE `character_expedition_lockouts` (
	`id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`character_id` INT(10) UNSIGNED NOT NULL,
	`expedition_name` VARCHAR(128) NOT NULL,
	`event_name` VARCHAR(256) NOT NULL,
	`expire_time` DATETIME NOT NULL DEFAULT current_timestamp(),
	`duration` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`from_expedition_uuid` VARCHAR(36) NOT NULL,
	PRIMARY KEY (`id`),
	UNIQUE INDEX `character_id_expedition_name_event_name` (`character_id`, `expedition_name`, `event_name`)
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
;

CREATE TABLE `dynamic_zones` (
	`id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`instance_id` INT(10) NOT NULL DEFAULT 0,
	`type` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	`compass_zone_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`compass_x` FLOAT NOT NULL DEFAULT 0,
	`compass_y` FLOAT NOT NULL DEFAULT 0,
	`compass_z` FLOAT NOT NULL DEFAULT 0,
	`safe_return_zone_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`safe_return_x` FLOAT NOT NULL DEFAULT 0,
	`safe_return_y` FLOAT NOT NULL DEFAULT 0,
	`safe_return_z` FLOAT NOT NULL DEFAULT 0,
	`safe_return_heading` FLOAT NOT NULL DEFAULT 0,
	`zone_in_x` FLOAT NOT NULL DEFAULT 0,
	`zone_in_y` FLOAT NOT NULL DEFAULT 0,
	`zone_in_z` FLOAT NOT NULL DEFAULT 0,
	`zone_in_heading` FLOAT NOT NULL DEFAULT 0,
	`has_zone_in` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	PRIMARY KEY (`id`),
	UNIQUE INDEX `instance_id` (`instance_id`)
)
COLLATE='utf8mb4_general_ci'
ENGINE=InnoDB
;

)",
	},
	ManifestEntry{
		.version = 9160,
		.description = "2021_02_14_npc_exp_mod.sql",
		.check = "SHOW COLUMNS from `npc_types` LIKE 'exp_mod'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types` ADD COLUMN `exp_mod` INT NOT NULL DEFAULT '100' AFTER `always_aggro`;

)",
	},
	ManifestEntry{
		.version = 9161,
		.description = "2021_02_15_npc_spell_entries_unsigned.sql",
		.check = "SELECT * FROM db_version WHERE version >= 9161",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_spells_entries` MODIFY `spellid` SMALLINT(5) UNSIGNED NOT NULL DEFAULT 0;

)",
	},
	ManifestEntry{
		.version = 9162,
		.description = "2021_02_17_server_scheduled_events.sql",
		.check = "SELECT * FROM db_version WHERE version >= 9162",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `server_scheduled_events`
(
    `id`              int(11) NOT NULL AUTO_INCREMENT,
    `description`     varchar(255) DEFAULT NULL,
    `event_type`      varchar(100) DEFAULT NULL,
    `event_data`      text         DEFAULT NULL,
    `minute_start`    int(11) DEFAULT 0,
    `hour_start`      int(11) DEFAULT 0,
    `day_start`       int(11) DEFAULT 0,
    `month_start`     int(11) DEFAULT 0,
    `year_start`      int(11) DEFAULT 0,
    `minute_end`      int(11) DEFAULT 0,
    `hour_end`        int(11) DEFAULT 0,
    `day_end`         int(11) DEFAULT 0,
    `month_end`       int(11) DEFAULT 0,
    `year_end`        int(11) DEFAULT 0,
    `cron_expression` varchar(100) DEFAULT NULL,
    `created_at`      datetime     DEFAULT NULL,
    `deleted_at`      datetime     DEFAULT NULL,
    PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

)",
	},
	ManifestEntry{
		.version = 9163,
		.description = "2021_04_17_zone_safe_heading_changes.sql",
		.check = "SHOW COLUMNS FROM `zone` LIKE 'safe_heading'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE zone ADD COLUMN safe_heading float NOT NULL DEFAULT 0 AFTER safe_z;

)",
	},
	ManifestEntry{
		.version = 9164,
		.description = "2021_04_23_character_exp_modifiers.sql",
		.check = "SHOW TABLES LIKE 'character_exp_modifiers'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `character_exp_modifiers`  (
  `character_id` int NOT NULL,
  `zone_id` int NOT NULL,
  `aa_modifier` float NOT NULL,
  `exp_modifier` float NOT NULL,
  PRIMARY KEY (`character_id`, `zone_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

)",
	},
	ManifestEntry{
		.version = 9165,
		.description = "2021_04_28_idle_pathing.sql",
		.check = "SHOW COLUMNS FROM `spawn2` LIKE 'path_when_zone_idle'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- Add new path_when_zone_idle flag to allow some spawns to path in empty zones
ALTER TABLE spawn2 ADD COLUMN path_when_zone_idle tinyint(1) NOT NULL DEFAULT 0 AFTER pathgrid;

-- Update spawns that used to path in empty zones because of their grid type
-- to behave the same using the new mechanism.  The code that checked path grid
-- types has been removed as it was coincidentally coupled to idle movement.
-- The new flag path_when_zone_idle is the new mechanism, and allows any moving
-- mob, not just those on grids, to path while the zone is idle.
UPDATE spawn2 s
LEFT JOIN zone z ON z.short_name = s.zone
LEFT JOIN grid g ON g.id = s.pathgrid AND g.zoneid = z.zoneidnumber
SET path_when_zone_idle = 1
WHERE pathgrid != 0 AND g.type IN (4, 6);

)",
	},
	ManifestEntry{
		.version = 9166,
		.description = "2021_02_12_dynamic_zone_members.sql",
		.check = "SHOW TABLES LIKE 'dynamic_zone_members'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `dynamic_zone_members` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `dynamic_zone_id` int(10) unsigned NOT NULL DEFAULT 0,
  `character_id` int(10) unsigned NOT NULL DEFAULT 0,
  `is_current_member` tinyint(3) unsigned NOT NULL DEFAULT 1,
  PRIMARY KEY (`id`),
  UNIQUE KEY `dynamic_zone_id_character_id` (`dynamic_zone_id`,`character_id`),
  KEY `character_id` (`character_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE `expedition_members`;

)",
	},
	ManifestEntry{
		.version = 9167,
		.description = "2021_06_06_beastlord_pets.sql",
		.check = "SHOW TABLES LIKE 'pets_beastlord_data'",
		.condition = "empty",
		.match = "",
		.sql = R"(
SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

DROP TABLE IF EXISTS `pets_beastlord_data`;
CREATE TABLE `pets_beastlord_data`  (
  `player_race` int UNSIGNED NOT NULL DEFAULT 1,
  `pet_race` int UNSIGNED NOT NULL DEFAULT 42,
  `texture` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `helm_texture` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `gender` tinyint UNSIGNED NOT NULL DEFAULT 2,
  `size_modifier` float UNSIGNED NULL DEFAULT 1,
  `face` tinyint UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`player_race`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

INSERT INTO `pets_beastlord_data` VALUES (2, 42, 2, 0, 2, 1, 0); -- Barbarian
INSERT INTO `pets_beastlord_data` VALUES (9, 91, 0, 0, 2, 2.5, 0); -- Troll
INSERT INTO `pets_beastlord_data` VALUES (10, 43, 3, 0, 2, 1, 0); -- Ogre
INSERT INTO `pets_beastlord_data` VALUES (128, 42, 0, 0, 1, 2, 0); -- Iksar
INSERT INTO `pets_beastlord_data` VALUES (130, 63, 0, 0, 2, 0.8, 0); -- Vah Shir

SET FOREIGN_KEY_CHECKS = 1;
)",
	},
	ManifestEntry{
		.version = 9168,
		.description = "2021_08_31_pvp_duration.sql",
		.check = "SHOW COLUMNS FROM `spells_new` LIKE 'pvp_duration'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `spells_new` CHANGE `field181` `pvp_duration` int(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field182` `pvp_duration_cap` int(11) NOT NULL DEFAULT '0';

)",
	},
	ManifestEntry{
		.version = 9169,
		.description = "2021_06_06_dynamic_zone_moved_columns.sql",
		.check = "SELECT * FROM db_version WHERE version >= 9169",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `dynamic_zones`
	ADD COLUMN `uuid` VARCHAR(36) NOT NULL COLLATE 'latin1_swedish_ci' AFTER `type`,
	ADD COLUMN `name` VARCHAR(128) NOT NULL DEFAULT '' COLLATE 'latin1_swedish_ci' AFTER `uuid`,
	ADD COLUMN `leader_id` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `name`,
	ADD COLUMN `min_players` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `leader_id`,
	ADD COLUMN `max_players` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `min_players`;

-- migrate any currently active expeditions
UPDATE dynamic_zones
INNER JOIN expeditions ON expeditions.dynamic_zone_id = dynamic_zones.id
SET
  dynamic_zones.uuid = expeditions.uuid,
  dynamic_zones.name = expeditions.expedition_name,
  dynamic_zones.leader_id = expeditions.leader_id,
  dynamic_zones.min_players = expeditions.min_players,
  dynamic_zones.max_players = expeditions.max_players;

ALTER TABLE `expeditions`
  DROP COLUMN `uuid`,
  DROP COLUMN `expedition_name`,
  DROP COLUMN `leader_id`,
  DROP COLUMN `min_players`,
  DROP COLUMN `max_players`;

)",
	},
	ManifestEntry{
		.version = 9170,
		.description = "2021_03_03_instance_safereturns.sql",
		.check = "SHOW TABLES LIKE 'character_instance_safereturns'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `character_instance_safereturns` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `character_id` int(10) unsigned NOT NULL,
  `instance_zone_id` int(11) NOT NULL DEFAULT 0,
  `instance_id` int(11) NOT NULL DEFAULT 0,
  `safe_zone_id` int(11) NOT NULL DEFAULT 0,
  `safe_x` float NOT NULL DEFAULT 0,
  `safe_y` float NOT NULL DEFAULT 0,
  `safe_z` float NOT NULL DEFAULT 0,
  `safe_heading` float NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  UNIQUE KEY `character_id` (`character_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

)",
	},
	ManifestEntry{
		.version = 9171,
		.description = "2021_03_30_remove_dz_is_current_member.sql",
		.check = "SHOW COLUMNS FROM `dynamic_zone_members` LIKE 'is_current_member'",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
-- remove any non-current members for new behavior
DELETE FROM `dynamic_zone_members`
WHERE is_current_member = 0;

ALTER TABLE `dynamic_zone_members`
  DROP COLUMN `is_current_member`;

)",
	},
	ManifestEntry{
		.version = 9172,
		.description = "2021_05_21_shared_tasks.sql",
		.check = "SHOW TABLES LIKE 'shared_tasks'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- shared task tables
CREATE TABLE `shared_tasks`
(
    `id`              bigint(20) NOT NULL AUTO_INCREMENT,
    `task_id`         int(11) DEFAULT NULL,
    `accepted_time`   datetime DEFAULT NULL,
    `expire_time`     datetime DEFAULT NULL,
    `completion_time` datetime DEFAULT NULL,
    `is_locked`       tinyint(1) DEFAULT NULL,
    PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

CREATE TABLE `shared_task_members`
(
    `shared_task_id` bigint(20) NOT NULL,
    `character_id`   bigint(20) NOT NULL,
    `is_leader`      tinyint(4) DEFAULT NULL,
    PRIMARY KEY (`shared_task_id`, `character_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `shared_task_activity_state`
(
    `shared_task_id` bigint(20) NOT NULL,
    `activity_id`    int(11) NOT NULL,
    `done_count`     int(11) DEFAULT NULL,
    `updated_time`   datetime DEFAULT NULL,
    `completed_time` datetime DEFAULT NULL,
    PRIMARY KEY (`shared_task_id`, `activity_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `shared_task_dynamic_zones`
(
    `shared_task_id`  bigint(20) NOT NULL,
    `dynamic_zone_id` int(10) unsigned NOT NULL,
    PRIMARY KEY (`shared_task_id`, `dynamic_zone_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- completed shared task tables - simply stores completed for reporting and logging

CREATE TABLE `completed_shared_tasks`
(
    `id`              bigint(20) NOT NULL,
    `task_id`         int(11) DEFAULT NULL,
    `accepted_time`   datetime DEFAULT NULL,
    `expire_time`     datetime DEFAULT NULL,
    `completion_time` datetime DEFAULT NULL,
    `is_locked`       tinyint(1) DEFAULT NULL,
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `completed_shared_task_members`
(
    `shared_task_id` bigint(20) NOT NULL,
    `character_id`   bigint(20) NOT NULL,
    `is_leader`      tinyint(4) DEFAULT NULL,
    PRIMARY KEY (`shared_task_id`, `character_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `completed_shared_task_activity_state`
(
    `shared_task_id` bigint(20) NOT NULL,
    `activity_id`    int(11) NOT NULL,
    `done_count`     int(11) DEFAULT NULL,
    `updated_time`   datetime DEFAULT NULL,
    `completed_time` datetime DEFAULT NULL,
    PRIMARY KEY (`shared_task_id`, `activity_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- tasks

ALTER TABLE `tasks`
    ADD COLUMN `level_spread` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `maxlevel`,
  ADD COLUMN `min_players` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `level_spread`,
  ADD COLUMN `max_players` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `min_players`,
  ADD COLUMN `replay_timer_seconds` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `completion_emote`,
  ADD COLUMN `request_timer_seconds` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `replay_timer_seconds`;

-- character timers

CREATE TABLE `character_task_timers`
(
    `id`           int(10) unsigned NOT NULL AUTO_INCREMENT,
    `character_id` int(10) unsigned NOT NULL DEFAULT 0,
    `task_id`      int(10) unsigned NOT NULL DEFAULT 0,
    `timer_type`   int(11) NOT NULL DEFAULT 0,
    `expire_time`  datetime NOT NULL DEFAULT current_timestamp(),
    PRIMARY KEY (`id`),
    KEY            `character_id` (`character_id`),
    KEY            `task_id` (`task_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

ALTER TABLE `tasks`
    CHANGE COLUMN `completion_emote` `completion_emote` VARCHAR (512) NOT NULL DEFAULT '' COLLATE 'latin1_swedish_ci' AFTER `faction_reward`;

ALTER TABLE `tasks`
    ADD COLUMN `reward_radiant_crystals` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `rewardmethod`,
  ADD COLUMN `reward_ebon_crystals` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `reward_radiant_crystals`;

)",
	},
	ManifestEntry{
		.version = 9173,
		.description = "2021_09_14_zone_lava_damage.sql",
		.check = "SHOW COLUMNS FROM `zone` LIKE 'lava_damage'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE zone ADD lava_damage INT(11) NULL DEFAULT '50' AFTER underworld_teleport_index, ADD min_lava_damage INT(11) NOT NULL DEFAULT '10' AFTER lava_damage;

)",
	},
	ManifestEntry{
		.version = 9174,
		.description = "2021_10_09_not_null_door_columns.sql",
		.check = "SELECT * FROM db_version WHERE version >= 9174",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- update any null columns to non-null value first to avoid data truncation errors
-- this will likely only affect the buffer column
update `doors` set `doors`.`dest_x` = 0 where `doors`.`dest_x` is null;
update `doors` set `doors`.`dest_y` = 0 where `doors`.`dest_y` is null;
update `doors` set `doors`.`dest_z` = 0 where `doors`.`dest_z` is null;
update `doors` set `doors`.`dest_heading` = 0 where `doors`.`dest_heading` is null;
update `doors` set `doors`.`invert_state` = 0 where `doors`.`invert_state` is null;
update `doors` set `doors`.`incline` = 0 where `doors`.`incline` is null;
update `doors` set `doors`.`buffer` = 0 where `doors`.`buffer` is null;

ALTER TABLE `doors`
	CHANGE COLUMN `dest_x` `dest_x` FLOAT NOT NULL DEFAULT '0' AFTER `dest_instance`,
	CHANGE COLUMN `dest_y` `dest_y` FLOAT NOT NULL DEFAULT '0' AFTER `dest_x`,
	CHANGE COLUMN `dest_z` `dest_z` FLOAT NOT NULL DEFAULT '0' AFTER `dest_y`,
	CHANGE COLUMN `dest_heading` `dest_heading` FLOAT NOT NULL DEFAULT '0' AFTER `dest_z`,
	CHANGE COLUMN `invert_state` `invert_state` INT(11) NOT NULL DEFAULT '0' AFTER `dest_heading`,
	CHANGE COLUMN `incline` `incline` INT(11) NOT NULL DEFAULT '0' AFTER `invert_state`,
	CHANGE COLUMN `buffer` `buffer` FLOAT NOT NULL DEFAULT '0' AFTER `size`;

)",
	},
	ManifestEntry{
		.version = 9175,
		.description = "2022_01_02_expansion_default_value_all.sql",
		.check = "SHOW COLUMNS FROM `forage` LIKE 'min_expansion'",
		.condition = "contains",
		.match = "unsigned",
		.sql = R"(
-- forage

ALTER TABLE `forage` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `forage` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE forage set min_expansion = -1 where min_expansion = 0;
UPDATE forage set max_expansion = -1 where max_expansion = 0;

-- tradeskill_recipe

ALTER TABLE `tradeskill_recipe` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `tradeskill_recipe` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE tradeskill_recipe set min_expansion = -1 where min_expansion = 0;
UPDATE tradeskill_recipe set max_expansion = -1 where max_expansion = 0;

-- fishing

ALTER TABLE `fishing` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `fishing` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE fishing set min_expansion = -1 where min_expansion = 0;
UPDATE fishing set max_expansion = -1 where max_expansion = 0;

-- zone

ALTER TABLE `zone` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `zone` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE zone set min_expansion = -1 where min_expansion = 0;
UPDATE zone set max_expansion = -1 where max_expansion = 0;

-- traps

ALTER TABLE `traps` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `traps` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE traps set min_expansion = -1 where min_expansion = 0;
UPDATE traps set max_expansion = -1 where max_expansion = 0;

-- loottable

ALTER TABLE `loottable` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `loottable` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE loottable set min_expansion = -1 where min_expansion = 0;
UPDATE loottable set max_expansion = -1 where max_expansion = 0;

-- ground_spawns

ALTER TABLE `ground_spawns` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `ground_spawns` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE ground_spawns set min_expansion = -1 where min_expansion = 0;
UPDATE ground_spawns set max_expansion = -1 where max_expansion = 0;

-- starting_items

ALTER TABLE `starting_items` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `starting_items` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE starting_items set min_expansion = -1 where min_expansion = 0;
UPDATE starting_items set max_expansion = -1 where max_expansion = 0;

-- spawn2

ALTER TABLE `spawn2` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `spawn2` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE spawn2 set min_expansion = -1 where min_expansion = 0;
UPDATE spawn2 set max_expansion = -1 where max_expansion = 0;

-- zone_points

ALTER TABLE `zone_points` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `zone_points` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE zone_points set min_expansion = -1 where min_expansion = 0;
UPDATE zone_points set max_expansion = -1 where max_expansion = 0;

-- lootdrop

ALTER TABLE `lootdrop` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `lootdrop` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE lootdrop set min_expansion = -1 where min_expansion = 0;
UPDATE lootdrop set max_expansion = -1 where max_expansion = 0;

-- global_loot

ALTER TABLE `global_loot` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `global_loot` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE global_loot set min_expansion = -1 where min_expansion = 0;
UPDATE global_loot set max_expansion = -1 where max_expansion = 0;

-- doors

ALTER TABLE `doors` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `doors` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE doors set min_expansion = -1 where min_expansion = 0;
UPDATE doors set max_expansion = -1 where max_expansion = 0;

-- object

ALTER TABLE `object` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `object` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE object set min_expansion = -1 where min_expansion = 0;
UPDATE object set max_expansion = -1 where max_expansion = 0;

-- start_zones

ALTER TABLE `start_zones` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `start_zones` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE start_zones set min_expansion = -1 where min_expansion = 0;
UPDATE start_zones set max_expansion = -1 where max_expansion = 0;

-- merchantlist

ALTER TABLE `merchantlist` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `merchantlist` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE merchantlist set min_expansion = -1 where min_expansion = 0;
UPDATE merchantlist set max_expansion = -1 where max_expansion = 0;

-- spawnentry
ALTER TABLE `spawnentry` ADD `min_expansion` tinyint(4)  NOT NULL DEFAULT -1;
ALTER TABLE `spawnentry` ADD `max_expansion` tinyint(4)  NOT NULL DEFAULT -1;
ALTER TABLE `spawnentry` ADD `content_flags` varchar(100) NULL;
ALTER TABLE `spawnentry` ADD `content_flags_disabled` varchar(100) NULL;

)",
	},
	ManifestEntry{
		.version = 9176,
		.description = "2022_01_10_checksum_verification.sql",
		.check = "SHOW COLUMNS FROM `account` LIKE 'crc_eqgame'",
		.condition = "empty",
		.match = "",
		.sql = R"(
INSERT INTO `variables` (`varname`, `value`, `information`, `ts`) VALUES ('crc_eqgame', '0', 'Client CRC64 Checksum on: eqgame.exe', '2021-09-23 14:16:27');
INSERT INTO `variables` (`varname`, `value`, `information`, `ts`) VALUES ('crc_skillcaps', '0', 'Client CRC64 Checksum on: SkillCaps.txt', '2021-09-23 14:16:21');
INSERT INTO `variables` (`varname`, `value`, `information`, `ts`) VALUES ('crc_basedata', '0', 'Client CRC64 Checksum on: BaseData.txt','2021-09-23 14:16:21');

ALTER TABLE `account`
	ADD COLUMN `crc_eqgame` TEXT NULL AFTER `suspend_reason`,
	ADD COLUMN `crc_skillcaps` TEXT NULL AFTER `crc_eqgame`,
	ADD COLUMN `crc_basedata` TEXT NULL AFTER `crc_skillcaps`;

ALTER TABLE `account` CHANGE `suspendeduntil` `suspendeduntil` datetime  NULL  COMMENT '';

)",
	},
	ManifestEntry{
		.version = 9177,
		.description = "2022_03_06_table_structure_changes.sql",
		.check = "SHOW COLUMNS FROM `pets` LIKE 'id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `pets` DROP PRIMARY KEY;
ALTER TABLE `pets` ADD `id` int(20) PRIMARY KEY NOT NULL AUTO_INCREMENT FIRST;
CREATE UNIQUE INDEX `type_petpower` ON pets (type, petpower);

ALTER TABLE `horses` DROP PRIMARY KEY;
ALTER TABLE `horses` ADD `id` int(20) PRIMARY KEY NOT NULL AUTO_INCREMENT FIRST;
CREATE UNIQUE INDEX `filename` ON horses (filename);

ALTER TABLE books DROP INDEX `id`;
ALTER TABLE `books` ADD `id` int(20) PRIMARY KEY NOT NULL AUTO_INCREMENT FIRST;
CREATE UNIQUE INDEX `filename` ON books (name);

)",
	},
	ManifestEntry{
		.version = 9178,
		.description = "2022_03_07_saylink_collation.sql",
		.check = "SELECT * FROM db_version WHERE version >= 9178",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE saylink CONVERT TO CHARACTER SET utf8 COLLATE utf8_bin;

)",
	},
	ManifestEntry{
		.version = 9179,
		.description = "2022_04_30_hp_regen_per_second.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'hp_regen_per_second'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE npc_types ADD COLUMN hp_regen_per_second bigint DEFAULT 0 AFTER hp_regen_rate;

)",
	},
	ManifestEntry{
		.version = 9180,
		.description = "2022_05_01_character_peqzone_flags.sql",
		.check = "SHOW TABLES LIKE 'character_peqzone_flags'",
		.condition = "empty",
		.match = "",
		.sql = R"(
SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for character_peqzone_flags
-- ----------------------------
DROP TABLE IF EXISTS `character_peqzone_flags`;
CREATE TABLE `character_peqzone_flags`  (
  `id` int NOT NULL DEFAULT 0,
  `zone_id` int NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`, `zone_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

SET FOREIGN_KEY_CHECKS = 1;

)",
	},
	ManifestEntry{
		.version = 9181,
		.description = "2022_05_03_task_activity_goal_match_list.sql",
		.check = "SHOW COLUMNS FROM `task_activities` LIKE 'goal_match_list'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE task_activities ADD goal_match_list text AFTER goalid;

)",
	},
	ManifestEntry{
		.version = 9182,
		.description = "2022_05_02_npc_types_int64.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'hp'",
		.condition = "missing",
		.match = "bigint",
		.sql = R"(
ALTER TABLE npc_types MODIFY COLUMN hp BIGINT NOT NULL DEFAULT 0;
ALTER TABLE npc_types MODIFY COLUMN mana BIGINT NOT NULL DEFAULT 0;
ALTER TABLE npc_types MODIFY COLUMN hp_regen_rate BIGINT NOT NULL DEFAULT 0;
ALTER TABLE npc_types MODIFY COLUMN mana_regen_rate BIGINT NOT NULL DEFAULT 0;

)",
	},
	ManifestEntry{
		.version = 9183,
		.description = "2022_05_07_merchant_data_buckets.sql",
		.check = "SHOW COLUMNS FROM `merchantlist` LIKE 'bucket_comparison'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `merchantlist`
ADD COLUMN `bucket_name` varchar(100) NOT NULL DEFAULT '' AFTER `probability`,
ADD COLUMN `bucket_value` varchar(100) NOT NULL DEFAULT '' AFTER `bucket_name`,
ADD COLUMN `bucket_comparison` tinyint UNSIGNED NULL DEFAULT 0 AFTER `bucket_value`;
)",
	},
	ManifestEntry{
		.version = 9184,
		.description = "2022_05_21_schema_consistency.sql",
		.check = "SELECT * FROM db_version WHERE version >= 9184",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE npc_types MODIFY COLUMN hp BIGINT NOT NULL DEFAULT 0;
ALTER TABLE npc_types MODIFY COLUMN mana BIGINT NOT NULL DEFAULT 0;
ALTER TABLE npc_types MODIFY COLUMN hp_regen_rate BIGINT NOT NULL DEFAULT 0;
ALTER TABLE npc_types MODIFY COLUMN mana_regen_rate BIGINT NOT NULL DEFAULT 0;
ALTER TABLE npc_types MODIFY COLUMN hp_regen_per_second BIGINT NOT NULL DEFAULT 0;

)",
	},
	ManifestEntry{
		.version = 9185,
		.description = "2022_05_07_discord_webhooks.sql",
		.check = "SHOW TABLES LIKE 'discord_webhooks'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE discord_webhooks
(
    id           INT auto_increment primary key NULL,
    webhook_name varchar(100) NULL,
    webhook_url  varchar(255) NULL,
    created_at   DATETIME NULL,
    deleted_at   DATETIME NULL
) ENGINE=InnoDB
DEFAULT CHARSET=utf8mb4
COLLATE=utf8mb4_general_ci;

ALTER TABLE logsys_categories
    ADD log_to_discord smallint(11) default 0 AFTER log_to_gmsay;
ALTER TABLE logsys_categories
    ADD discord_webhook_id int(11) default 0 AFTER log_to_discord;

)",
	},
	ManifestEntry{
		.version = 9186,
		.description = "2022_07_09_zone_expansion_deprecate.sql",
		.check = "SHOW COLUMNS FROM `zone` LIKE 'expansion'",
		.condition = "not_empty",
		.match = "",
		.sql = R"(

)",
	},
	ManifestEntry{
		.version = 9187,
		.description = "2022_07_09_task_zone_version_matching.sql",
		.check = "SHOW COLUMNS FROM `task_activities` LIKE 'zone_version'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `task_activities` ADD COLUMN `zone_version` int(11) default -1 AFTER zones;

)",
	},
	ManifestEntry{
		.version = 9189,
		.description = "2022_07_10_character_task_rewarded.sql",
		.check = "SHOW COLUMNS FROM `character_tasks` LIKE 'was_rewarded'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `character_tasks`
	ADD COLUMN `was_rewarded` TINYINT NOT NULL DEFAULT '0' AFTER `acceptedtime`;

)",
	},
	ManifestEntry{
		.version = 9190,
		.description = "2022_07_13_task_reward_points.sql",
		.check = "SHOW COLUMNS FROM `tasks` LIKE 'reward_points'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `tasks`
	ADD COLUMN `reward_points` INT NOT NULL DEFAULT '0' AFTER `rewardmethod`,
	ADD COLUMN `reward_point_type` INT NOT NULL DEFAULT '0' AFTER `reward_points`;

-- convert don crystal points to new fields
UPDATE tasks SET reward_point_type = 4 WHERE reward_radiant_crystals > 0;
UPDATE tasks SET reward_point_type = 5 WHERE reward_ebon_crystals > 0;
UPDATE tasks SET reward_points = reward_radiant_crystals WHERE reward_radiant_crystals > 0;
UPDATE tasks SET reward_points = reward_ebon_crystals WHERE reward_ebon_crystals > 0;

ALTER TABLE `tasks`
	DROP COLUMN `reward_radiant_crystals`,
	DROP COLUMN `reward_ebon_crystals`;

)",
	},
	ManifestEntry{
		.version = 9191,
		.description = "2022_07_28_gm_state_changes.sql",
		.check = "SHOW COLUMNS FROM `account` LIKE 'invulnerable'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `account`
    ADD COLUMN `invulnerable` TINYINT(4) NULL DEFAULT '0' AFTER `gmspeed`,
    ADD COLUMN `flymode` TINYINT(4) NULL DEFAULT '0' AFTER `invulnerable`,
    ADD COLUMN `ignore_tells` TINYINT(4) NULL DEFAULT '0' AFTER `flymode`;

)",
	},
	ManifestEntry{
		.version = 9192,
		.description = "2022_07_13_task_lock_activity.sql",
		.check = "SHOW COLUMNS FROM `tasks` LIKE 'lock_activity_id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `tasks`
  ADD COLUMN `lock_activity_id` INT NOT NULL DEFAULT '-1' AFTER `request_timer_seconds`;

)",
	},
	ManifestEntry{
		.version = 9193,
		.description = "2022_07_16_task_timer_groups.sql",
		.check = "SHOW COLUMNS FROM `tasks` LIKE 'replay_timer_group'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `tasks`
  ADD COLUMN `replay_timer_group` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `completion_emote`,
  ADD COLUMN `request_timer_group` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `replay_timer_seconds`;

ALTER TABLE `character_task_timers`
  ADD COLUMN `timer_group` INT NOT NULL DEFAULT '0' AFTER `timer_type`;

)",
	},
	ManifestEntry{
		.version = 9194,
		.description = "2022_07_23_dz_switch_id.sql",
		.check = "SHOW COLUMNS FROM `doors` LIKE 'dz_switch_id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `doors`
  ADD COLUMN `dz_switch_id` INT NOT NULL DEFAULT '0' AFTER `is_ldon_door`;

ALTER TABLE `dynamic_zones`
  ADD COLUMN `dz_switch_id` INT NOT NULL DEFAULT '0' AFTER `max_players`;

)",
	},
	ManifestEntry{
		.version = 9195,
		.description = "2022_07_23_dz_templates.sql",
		.check = "SHOW TABLES like 'dynamic_zone_templates'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `dynamic_zone_templates` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `zone_id` int(11) NOT NULL DEFAULT 0,
  `zone_version` int(11) NOT NULL DEFAULT 0,
  `name` varchar(128) NOT NULL DEFAULT '',
  `min_players` int(11) NOT NULL DEFAULT 0,
  `max_players` int(11) NOT NULL DEFAULT 0,
  `duration_seconds` int(11) NOT NULL DEFAULT 0,
  `dz_switch_id` int(11) NOT NULL DEFAULT 0,
  `compass_zone_id` int(11) NOT NULL DEFAULT 0,
  `compass_x` float NOT NULL DEFAULT 0,
  `compass_y` float NOT NULL DEFAULT 0,
  `compass_z` float NOT NULL DEFAULT 0,
  `return_zone_id` int(11) NOT NULL DEFAULT 0,
  `return_x` float NOT NULL DEFAULT 0,
  `return_y` float NOT NULL DEFAULT 0,
  `return_z` float NOT NULL DEFAULT 0,
  `return_h` float NOT NULL DEFAULT 0,
  `override_zone_in` tinyint(4) NOT NULL DEFAULT 0,
  `zone_in_x` float NOT NULL DEFAULT 0,
  `zone_in_y` float NOT NULL DEFAULT 0,
  `zone_in_z` float NOT NULL DEFAULT 0,
  `zone_in_h` float NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

ALTER TABLE `tasks`
  ADD COLUMN `dz_template_id` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `request_timer_seconds`;

)",
	},
	ManifestEntry{
		.version = 9196,
		.description = "2022_07_30_merchantlist_temp.sql",
		.check = "SHOW COLUMNS FROM `merchantlist_temp` LIKE 'zone_id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `merchantlist_temp` ADD COLUMN `zone_id` INT(11) NOT NULL DEFAULT '0' AFTER `slot`;
ALTER TABLE `merchantlist_temp` ADD COLUMN `instance_id` INT(11) NOT NULL DEFAULT '0' AFTER `zone_id`;
ALTER TABLE `merchantlist_temp` DROP PRIMARY KEY, ADD PRIMARY KEY (`npcid`, `slot`, `zone_id`, `instance_id`);

)",
	},
	ManifestEntry{
		.version = 9197,
		.description = "2022_08_01_drop_expansion_account.sql",
		.check = "SHOW COLUMNS FROM `account` LIKE 'expansion'",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
ALTER TABLE `account` DROP `expansion`;

)",
	},
	ManifestEntry{
		.version = 9198,
		.description = "2022_08_14_exp_modifier_instance_versions.sql",
		.check = "SHOW COLUMNS FROM `character_exp_modifiers` LIKE 'instance_version'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE character_exp_modifiers
ADD COLUMN instance_version int NOT NULL DEFAULT -1 AFTER zone_id,
DROP PRIMARY KEY,
ADD PRIMARY KEY (character_id, zone_id, instance_version) USING BTREE;
)",
	},
	ManifestEntry{
		.version = 9199,
		.description = "2022_08_08_task_req_activity_id.sql",
		.check = "SHOW COLUMNS FROM `task_activities` LIKE 'req_activity_id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `task_activities`
  ADD COLUMN `req_activity_id` INT SIGNED NOT NULL DEFAULT '-1' AFTER `activityid`;

)",
	},
	ManifestEntry{
		.version = 9200,
		.description = "2022_08_19_zone_expansion_consistency.sql",
		.check = "SELECT * FROM db_version WHERE version >= 9200",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `zone`
    ADD COLUMN `bypass_expansion_check` tinyint(3) NOT NULL DEFAULT 0 AFTER `expansion`;

UPDATE `zone` SET `bypass_expansion_check` = 1 WHERE `short_name`
IN (
    'befallenb',
    'commonlands',
    'freeportacademy',
    'freeportarena',
    'freeportcityhall',
    'freeporteast',
    'freeporthall',
    'freeportmilitia',
    'freeportsewers',
    'freeportwest',
    'guildhall',
    'guildlobby',
    'highpasshold',
    'highpasskeep',
    'innothuleb',
    'kithforest',
    'mistythicket',
    'moors',
    'nektulosa',
    'northro',
    'oceanoftears',
    'southro',
    'steamfontmts',
    'toxxulia'
);

)",
	},
	ManifestEntry{
		.version = 9201,
		.description = "2022_08_22_npc_types_heroic_strikethrough.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'heroic_strikethrough'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types`
ADD COLUMN `heroic_strikethrough` INT NOT NULL DEFAULT 0 AFTER `exp_mod`;
)",
	},
	ManifestEntry{
		.version = 9202,
		.description = "2022_08_24_task_activities_step.sql",
		.check = "SHOW COLUMNS FROM `task_activities` LIKE 'step'",
		.condition = "contains",
		.match = "unsigned",
		.sql = R"(
ALTER TABLE `task_activities` MODIFY `step` INT(11) NOT NULL DEFAULT '0';

)",
	},
	ManifestEntry{
		.version = 9203,
		.description = "2022_08_07_replace_task_goals.sql",
		.check = "SHOW COLUMNS FROM `task_activities` LIKE 'item_id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- backup original since this is a complex migration
CREATE TABLE `task_activities_backup_9203` LIKE `task_activities`;
INSERT INTO `task_activities_backup_9203` SELECT * FROM `task_activities`;

ALTER TABLE `task_activities`
  CHANGE COLUMN `description_override` `description_override` VARCHAR(128) NOT NULL DEFAULT '' COLLATE 'latin1_swedish_ci' AFTER `goalcount`,
  ADD COLUMN `npc_id` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `description_override`,
  ADD COLUMN `npc_goal_id` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `npc_id`,
  ADD COLUMN `npc_match_list` TEXT NULL DEFAULT NULL AFTER `npc_goal_id`,
  ADD COLUMN `item_id` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `npc_match_list`,
  ADD COLUMN `item_goal_id` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `item_id`,
  ADD COLUMN `item_id_list` TEXT NULL DEFAULT NULL AFTER `item_goal_id`,
  CHANGE COLUMN `item_list` `item_list` VARCHAR(128) NOT NULL DEFAULT '' COLLATE 'latin1_swedish_ci' AFTER `item_id_list`,
  ADD COLUMN `dz_switch_id` INT(11) NOT NULL DEFAULT '0' AFTER `delivertonpc`,
  ADD COLUMN `min_x` FLOAT NOT NULL DEFAULT 0 AFTER `dz_switch_id`,
  ADD COLUMN `min_y` FLOAT NOT NULL DEFAULT 0 AFTER `min_x`,
  ADD COLUMN `min_z` FLOAT NOT NULL DEFAULT 0 AFTER `min_y`,
  ADD COLUMN `max_x` FLOAT NOT NULL DEFAULT 0 AFTER `min_z`,
  ADD COLUMN `max_y` FLOAT NOT NULL DEFAULT 0 AFTER `max_x`,
  ADD COLUMN `max_z` FLOAT NOT NULL DEFAULT 0 AFTER `max_y`,
  CHANGE COLUMN `skill_list` `skill_list` VARCHAR(64) NOT NULL DEFAULT '-1' COLLATE 'latin1_swedish_ci' AFTER `max_z`,
  CHANGE COLUMN `spell_list` `spell_list` VARCHAR(64) NOT NULL DEFAULT '0' COLLATE 'latin1_swedish_ci' AFTER `skill_list`;

-- move Explore (5) goalid proximities to the new location fields
-- does not migrate where zone was different and ignores lists (unsupported)
UPDATE `task_activities`
INNER JOIN `proximities`
  ON `task_activities`.`goalid` = `proximities`.`exploreid`
  AND CAST(`task_activities`.`zones` AS INT) = `proximities`.`zoneid`
SET
  `task_activities`.`goalid` = 0,
  `task_activities`.`min_x` = `proximities`.`minx`,
  `task_activities`.`min_y` = `proximities`.`miny`,
  `task_activities`.`min_z` = `proximities`.`minz`,
  `task_activities`.`max_x` = `proximities`.`maxx`,
  `task_activities`.`max_y` = `proximities`.`maxy`,
  `task_activities`.`max_z` = `proximities`.`maxz`
WHERE
  `task_activities`.`goalmethod` = 0
  AND `task_activities`.`activitytype` = 5;

-- dz_switch_id for Touch (11)
UPDATE `task_activities`
SET `task_activities`.`dz_switch_id` = `task_activities`.`goalid`
WHERE `task_activities`.`goalmethod` = 0
  AND `task_activities`.`activitytype` = 11;

-- single item ids for Deliver (1), Loot (3), TradeSkill (6), Fish (7), Forage (8)
UPDATE `task_activities`
SET `task_activities`.`item_id` = `task_activities`.`goalid`
WHERE `task_activities`.`goalmethod` = 0
  AND `task_activities`.`activitytype` IN (1, 3, 6, 7, 8);

-- item goallist id
UPDATE `task_activities`
SET `task_activities`.`item_goal_id` = `task_activities`.`goalid`
WHERE `task_activities`.`goalmethod` = 1
  AND `task_activities`.`activitytype` IN (1, 3, 6, 7, 8);

-- item id match list
UPDATE `task_activities`
SET `task_activities`.`item_id_list` = `task_activities`.`goal_match_list`
WHERE `task_activities`.`goalmethod` = 1
  AND `task_activities`.`activitytype` IN (1, 3, 6, 7, 8);

-- single npc ids for Kill (2), SpeakWith (4)
UPDATE `task_activities`
SET `task_activities`.`npc_id` = `task_activities`.`goalid`
WHERE `task_activities`.`goalmethod` = 0
  AND `task_activities`.`activitytype` IN (2, 4);

-- npc goallist id
UPDATE `task_activities`
SET `task_activities`.`npc_goal_id` = `task_activities`.`goalid`
WHERE `task_activities`.`goalmethod` = 1
  AND `task_activities`.`activitytype` IN (2, 4);

-- npc match list
UPDATE `task_activities`
SET `task_activities`.`npc_match_list` = `task_activities`.`goal_match_list`
WHERE `task_activities`.`goalmethod` = 1
  AND `task_activities`.`activitytype` IN (2, 4);

-- delivertonpc npc_ids for Deliver (1), GiveCash (100)
UPDATE `task_activities`
SET `task_activities`.`npc_id` = `task_activities`.`delivertonpc`
WHERE `task_activities`.`activitytype` IN (1, 100);

ALTER TABLE `task_activities`
  DROP COLUMN `goalid`,
  DROP COLUMN `goal_match_list`,
  DROP COLUMN `delivertonpc`;

-- leave proximities table backup in case of regressions
ALTER TABLE `proximities` RENAME `proximities_backup_9203`;

)",
	},
	ManifestEntry{
		.version = 9204,
		.description = "2022_09_02_faction_association.sql",
		.check = "SHOW TABLES LIKE 'faction_association'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `faction_association` (
	`id` INT(10) NOT NULL,
	`id_1` INT(10) NOT NULL DEFAULT 0,
	`mod_1` FLOAT NOT NULL DEFAULT 0,
	`id_2` INT(10) NOT NULL DEFAULT 0,
	`mod_2` FLOAT NOT NULL DEFAULT 0,
	`id_3` INT(10) NOT NULL DEFAULT 0,
	`mod_3` FLOAT NOT NULL DEFAULT 0,
	`id_4` INT(10) NOT NULL DEFAULT 0,
	`mod_4` FLOAT NOT NULL DEFAULT 0,
	`id_5` INT(10) NOT NULL DEFAULT 0,
	`mod_5` FLOAT NOT NULL DEFAULT 0,
	`id_6` INT(10) NOT NULL DEFAULT 0,
	`mod_6` FLOAT NOT NULL DEFAULT 0,
	`id_7` INT(10) NOT NULL DEFAULT 0,
	`mod_7` FLOAT NOT NULL DEFAULT 0,
	`id_8` INT(10) NOT NULL DEFAULT 0,
	`mod_8` FLOAT NOT NULL DEFAULT 0,
	`id_9` INT(10) NOT NULL DEFAULT 0,
	`mod_9` FLOAT NOT NULL DEFAULT 0,
	`id_10` INT(10) NOT NULL DEFAULT 0,
	`mod_10` FLOAT NOT NULL DEFAULT 0,
	PRIMARY KEY(`id`)
);

ALTER TABLE `npc_types` ADD `faction_amount` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `tasks` ADD `faction_amount` INT(10) NOT NULL DEFAULT '0';

)",
	},
	ManifestEntry{
		.version = 9208,
		.description = "2022_09_25_task_concat_matchlists.sql",
		.check = "SHOW COLUMNS FROM `task_activities` LIKE 'npc_id'",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
SET SESSION group_concat_max_len = 1048576;
SET collation_connection = latin1_swedish_ci;

-- backup original(s)
CREATE TABLE `goallists_backup_9_25_2022` LIKE `goallists`;
INSERT INTO `goallists_backup_9_25_2022` SELECT * FROM `goallists`;
CREATE TABLE `tasks_backup_9_25_2022` LIKE `tasks`;
INSERT INTO `tasks_backup_9_25_2022` SELECT * FROM `tasks`;

-- npc id
UPDATE `task_activities`
SET `task_activities`.`npc_match_list` = CONCAT_WS('|', `npc_match_list`, `npc_id`)
WHERE npc_id != 0;

-- npc_goal_id goallists
UPDATE `task_activities`
INNER JOIN
(
  SELECT `goallists`.`listid`, GROUP_CONCAT(`goallists`.`entry` ORDER BY `goallists`.`entry` SEPARATOR '|') AS `goallist_ids`
  FROM `goallists`
  GROUP BY `goallists`.`listid`
) AS `goallist_group`
  ON `task_activities`.`npc_goal_id` = `goallist_group`.`listid`
SET `task_activities`.`npc_match_list` = CONCAT_WS('|', `npc_match_list`, `goallist_ids`)
WHERE npc_goal_id != 0;

-- item id
UPDATE `task_activities`
SET `task_activities`.`item_id_list` = CONCAT_WS('|', `item_id_list`, `item_id`)
WHERE item_id != 0;

-- item_goal_id goallists
UPDATE `task_activities`
INNER JOIN
(
  SELECT `goallists`.`listid`, GROUP_CONCAT(`goallists`.`entry` ORDER BY `goallists`.`entry` SEPARATOR '|') AS `goallist_ids`
  FROM `goallists`
  GROUP BY `goallists`.`listid`
) AS `goallist_group`
  ON `task_activities`.`item_goal_id` = `goallist_group`.`listid`
SET `task_activities`.`item_id_list` = CONCAT_WS('|', `item_id_list`, `goallist_ids`)
WHERE item_goal_id != 0;

ALTER TABLE `task_activities`
  DROP COLUMN `npc_id`,
  DROP COLUMN `npc_goal_id`,
  DROP COLUMN `item_id`,
  DROP COLUMN `item_goal_id`;


-- Reward cleanup and task table cleanup

ALTER TABLE `tasks`
    CHANGE COLUMN `reward` `reward_text` varchar(64) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '' AFTER `description`,
    CHANGE COLUMN `rewardid` `reward_id_list` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL AFTER `reward_text`,
    CHANGE COLUMN `cashreward` `cash_reward` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `reward_id_list`,
    CHANGE COLUMN `rewardmethod` `reward_method` tinyint(3) UNSIGNED NOT NULL DEFAULT 0 AFTER `xpreward`,
    CHANGE COLUMN `minlevel` `min_level` tinyint(3) UNSIGNED NOT NULL DEFAULT 0 AFTER `reward_point_type`,
    CHANGE COLUMN `maxlevel` `max_level` tinyint(3) UNSIGNED NOT NULL DEFAULT 0 AFTER `min_level`;

ALTER Table `tasks` CHANGE COLUMN `xpreward` `exp_reward` int(10) NOT NULL DEFAULT 0 AFTER `cash_reward`;

UPDATE tasks SET reward_id_list =
 (
     SELECT GROUP_CONCAT(`goallists`.`entry` ORDER BY `goallists`.`entry` SEPARATOR '|') AS `goallist_ids` FROM `goallists` WHERE listid = reward_id_list)
WHERE
reward_method = 1;

-- deprecated table
DROP table goallists;

)",
	},
	ManifestEntry{
		.version = 9209,
		.description = "2022_09_28_discord_webhooks.sql",
		.check = "SHOW COLUMNS FROM `logsys_categories` LIKE 'log_to_discord'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE logsys_categories
    ADD log_to_discord smallint(11) default 0 AFTER log_to_gmsay;
ALTER TABLE logsys_categories
    ADD discord_webhook_id int(11) default 0 AFTER log_to_discord;

)",
	},
	ManifestEntry{
		.version = 9213,
		.description = "2022_12_24_npc_keeps_sold_items.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'keeps_sold_items'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types`
ADD COLUMN `keeps_sold_items` tinyint(1) UNSIGNED NOT NULL DEFAULT 1 AFTER `faction_amount`;
)",
	},
	ManifestEntry{
		.version = 9214,
		.description = "2022_12_24_character_exp_toggle.sql",
		.check = "SHOW COLUMNS FROM `character_data` LIKE 'exp_enabled'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `character_data`
ADD COLUMN `exp_enabled` tinyint(1) UNSIGNED NOT NULL DEFAULT 1 AFTER `exp`;

)",
	},
	ManifestEntry{
		.version = 9215,
		.description = "2023_01_08_zone_max_level.sql",
		.check = "SHOW COLUMNS FROM `zone` LIKE 'max_level'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `zone`
ADD COLUMN `max_level` tinyint(3) UNSIGNED NOT NULL DEFAULT 255 AFTER `min_level`;
)",
	},
	ManifestEntry{
		.version = 9216,
		.description = "2023_01_15_merc_data.sql",
		.check = "SHOW TABLES LIKE 'mercs'",
		.condition = "empty",
		.match = "",
		.sql = R"(
SET NAMES utf8;
SET
FOREIGN_KEY_CHECKS = 0;

DROP TABLE IF EXISTS `merc_armorinfo`;
CREATE TABLE `merc_armorinfo`
(
    `id`               int(11) NOT NULL AUTO_INCREMENT,
    `merc_npc_type_id` int(11) UNSIGNED NOT NULL,
    `minlevel`         tinyint(2) UNSIGNED NOT NULL DEFAULT 1,
    `maxlevel`         tinyint(2) UNSIGNED NOT NULL DEFAULT 255,
    `texture`          tinyint(2) UNSIGNED NOT NULL DEFAULT 0,
    `helmtexture`      tinyint(2) UNSIGNED NOT NULL DEFAULT 0,
    `armortint_id`     int(10) UNSIGNED NOT NULL DEFAULT 0,
    `armortint_red`    tinyint(3) UNSIGNED NOT NULL DEFAULT 0,
    `armortint_green`  tinyint(3) UNSIGNED NOT NULL DEFAULT 0,
    `armortint_blue`   tinyint(3) UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 41 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_buffs`;
CREATE TABLE `merc_buffs`
(
    `MercBuffId`         int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
    `MercId`             int(10) UNSIGNED NOT NULL DEFAULT 0,
    `SpellId`            int(10) UNSIGNED NOT NULL DEFAULT 0,
    `CasterLevel`        int(10) UNSIGNED NOT NULL DEFAULT 0,
    `DurationFormula`    int(10) UNSIGNED NOT NULL DEFAULT 0,
    `TicsRemaining`      int(11) NOT NULL DEFAULT 0,
    `PoisonCounters`     int(11) UNSIGNED NOT NULL DEFAULT 0,
    `DiseaseCounters`    int(11) UNSIGNED NOT NULL DEFAULT 0,
    `CurseCounters`      int(11) UNSIGNED NOT NULL DEFAULT 0,
    `CorruptionCounters` int(11) UNSIGNED NOT NULL DEFAULT 0,
    `HitCount`           int(10) UNSIGNED NOT NULL DEFAULT 0,
    `MeleeRune`          int(10) UNSIGNED NOT NULL DEFAULT 0,
    `MagicRune`          int(10) UNSIGNED NOT NULL DEFAULT 0,
    `dot_rune`           int(10) NOT NULL DEFAULT 0,
    `caston_x`           int(10) NOT NULL DEFAULT 0,
    `Persistent`         tinyint(1) NOT NULL DEFAULT 0,
    `caston_y`           int(10) NOT NULL DEFAULT 0,
    `caston_z`           int(10) NOT NULL DEFAULT 0,
    `ExtraDIChance`      int(10) NOT NULL DEFAULT 0,
    PRIMARY KEY (`MercBuffId`) USING BTREE,
    INDEX                `FK_mercbuff_1`(`MercId`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 1 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_inventory`;
CREATE TABLE `merc_inventory`
(
    `merc_inventory_id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
    `merc_subtype_id`   int(10) UNSIGNED NOT NULL DEFAULT 0,
    `item_id`           int(11) UNSIGNED NOT NULL DEFAULT 0,
    `min_level`         int(10) UNSIGNED NOT NULL DEFAULT 0,
    `max_level`         int(10) UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`merc_inventory_id`) USING BTREE,
    INDEX               `FK_merc_inventory_1`(`merc_subtype_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 42 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_merchant_entries`;
CREATE TABLE `merc_merchant_entries`
(
    `merc_merchant_entry_id`    int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
    `merc_merchant_template_id` int(10) UNSIGNED NOT NULL,
    `merchant_id`               int(11) UNSIGNED NOT NULL,
    PRIMARY KEY (`merc_merchant_entry_id`) USING BTREE,
    INDEX                       `FK_merc_merchant_entries_1`(`merc_merchant_template_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 57 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_merchant_template_entries`;
CREATE TABLE `merc_merchant_template_entries`
(
    `merc_merchant_template_entry_id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
    `merc_merchant_template_id`       int(10) UNSIGNED NOT NULL,
    `merc_template_id`                int(10) UNSIGNED NOT NULL,
    PRIMARY KEY (`merc_merchant_template_entry_id`) USING BTREE,
    INDEX                             `FK_merc_merchant_template_entries_1`(`merc_merchant_template_id`) USING BTREE,
    INDEX                             `FK_merc_merchant_template_entries_2`(`merc_template_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 554 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_merchant_templates`;
CREATE TABLE `merc_merchant_templates`
(
    `merc_merchant_template_id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
    `name`                      varchar(50) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
    `qglobal`                   varchar(255) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
    PRIMARY KEY (`merc_merchant_template_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 25 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_name_types`;
CREATE TABLE `merc_name_types`
(
    `name_type_id` int(10) UNSIGNED NOT NULL,
    `class_id`     int(10) UNSIGNED NOT NULL,
    `prefix`       varchar(25) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
    `suffix`       varchar(25) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
    PRIMARY KEY (`name_type_id`, `class_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_npc_types`;
CREATE TABLE `merc_npc_types`
(
    `merc_npc_type_id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
    `proficiency_id`   tinyint(3) UNSIGNED NOT NULL,
    `tier_id`          tinyint(3) UNSIGNED NOT NULL,
    `class_id`         int(10) UNSIGNED NOT NULL,
    `name`             varchar(255) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
    PRIMARY KEY (`merc_npc_type_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 41 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_spell_list_entries`;
CREATE TABLE `merc_spell_list_entries`
(
    `merc_spell_list_entry_id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
    `merc_spell_list_id`       int(10) UNSIGNED NOT NULL,
    `spell_id`                 int(10) UNSIGNED NOT NULL,
    `spell_type`               int(10) UNSIGNED NOT NULL DEFAULT 0,
    `stance_id`                tinyint(3) UNSIGNED NOT NULL DEFAULT 0,
    `minlevel`                 tinyint(3) UNSIGNED NOT NULL DEFAULT 1,
    `maxlevel`                 tinyint(3) UNSIGNED NOT NULL DEFAULT 255,
    `slot`                     tinyint(4) NOT NULL DEFAULT -1,
    `procChance`               tinyint(3) UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`merc_spell_list_entry_id`) USING BTREE,
    INDEX                      `FK_merc_spell_lists_1`(`merc_spell_list_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 730 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_spell_lists`;
CREATE TABLE `merc_spell_lists`
(
    `merc_spell_list_id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
    `class_id`           int(10) UNSIGNED NOT NULL,
    `proficiency_id`     tinyint(3) UNSIGNED NOT NULL,
    `name`               varchar(50) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
    PRIMARY KEY (`merc_spell_list_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 9 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_stance_entries`;
CREATE TABLE `merc_stance_entries`
(
    `merc_stance_entry_id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
    `class_id`             int(10) UNSIGNED NOT NULL,
    `proficiency_id`       tinyint(3) UNSIGNED NOT NULL,
    `stance_id`            tinyint(3) UNSIGNED NOT NULL,
    `isdefault`            tinyint(1) NOT NULL,
    PRIMARY KEY (`merc_stance_entry_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 23 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_stats`;
CREATE TABLE `merc_stats`
(
    `merc_npc_type_id`  int(11) UNSIGNED NOT NULL,
    `clientlevel`       tinyint(2) UNSIGNED NOT NULL DEFAULT 1,
    `level`             tinyint(2) UNSIGNED NOT NULL DEFAULT 1,
    `hp`                int(11) NOT NULL DEFAULT 1,
    `mana`              int(11) NOT NULL DEFAULT 0,
    `AC`                smallint(5) NOT NULL DEFAULT 1,
    `ATK`               mediumint(9) NOT NULL DEFAULT 1,
    `STR`               mediumint(8) UNSIGNED NOT NULL DEFAULT 75,
    `STA`               mediumint(8) UNSIGNED NOT NULL DEFAULT 75,
    `DEX`               mediumint(8) UNSIGNED NOT NULL DEFAULT 75,
    `AGI`               mediumint(8) UNSIGNED NOT NULL DEFAULT 75,
    `_INT`              mediumint(8) UNSIGNED NOT NULL DEFAULT 80,
    `WIS`               mediumint(8) UNSIGNED NOT NULL DEFAULT 80,
    `CHA`               mediumint(8) UNSIGNED NOT NULL DEFAULT 75,
    `MR`                smallint(5) NOT NULL DEFAULT 15,
    `CR`                smallint(5) NOT NULL DEFAULT 15,
    `DR`                smallint(5) NOT NULL DEFAULT 15,
    `FR`                smallint(5) NOT NULL DEFAULT 15,
    `PR`                smallint(5) NOT NULL DEFAULT 15,
    `Corrup`            smallint(5) NOT NULL DEFAULT 15,
    `mindmg`            int(10) UNSIGNED NOT NULL DEFAULT 1,
    `maxdmg`            int(10) UNSIGNED NOT NULL DEFAULT 1,
    `attack_count`      smallint(6) NOT NULL DEFAULT 0,
    `attack_speed`      tinyint(3) NOT NULL DEFAULT 0,
    `attack_delay`      tinyint(3) UNSIGNED NOT NULL DEFAULT 30,
    `special_abilities` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL,
    `Accuracy`          mediumint(9) NOT NULL DEFAULT 0,
    `hp_regen_rate`     int(11) UNSIGNED NOT NULL DEFAULT 1,
    `mana_regen_rate`   int(11) UNSIGNED NOT NULL DEFAULT 1,
    `runspeed`          float NOT NULL DEFAULT 0,
    `statscale`         int(11) NOT NULL DEFAULT 100,
    `spellscale`        float NOT NULL DEFAULT 100,
    `healscale`         float NOT NULL DEFAULT 100,
    PRIMARY KEY (`merc_npc_type_id`, `clientlevel`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_subtypes`;
CREATE TABLE `merc_subtypes`
(
    `merc_subtype_id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
    `class_id`        int(10) UNSIGNED NOT NULL,
    `tier_id`         tinyint(3) UNSIGNED NOT NULL,
    `confidence_id`   tinyint(3) UNSIGNED NOT NULL,
    PRIMARY KEY (`merc_subtype_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 21 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_templates`;
CREATE TABLE `merc_templates`
(
    `merc_template_id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
    `merc_type_id`     int(10) UNSIGNED NOT NULL,
    `merc_subtype_id`  int(10) UNSIGNED NOT NULL,
    `merc_npc_type_id` int(11) UNSIGNED NOT NULL,
    `dbstring`         varchar(12) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
    `name_type_id`     tinyint(4) NOT NULL DEFAULT 0,
    `clientversion`    int(10) UNSIGNED NOT NULL,
    PRIMARY KEY (`merc_template_id`) USING BTREE,
    INDEX              `FK_merc_templates_1`(`merc_type_id`) USING BTREE,
    INDEX              `FK_merc_templates_2`(`merc_subtype_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 554 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_types`;
CREATE TABLE `merc_types`
(
    `merc_type_id`   int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
    `race_id`        int(10) UNSIGNED NOT NULL,
    `proficiency_id` tinyint(3) UNSIGNED NOT NULL,
    `dbstring`       varchar(12) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
    `clientversion`  int(10) UNSIGNED NOT NULL,
    PRIMARY KEY (`merc_type_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 49 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_weaponinfo`;
CREATE TABLE `merc_weaponinfo`
(
    `id`               int(11) NOT NULL AUTO_INCREMENT,
    `merc_npc_type_id` int(11) NOT NULL,
    `minlevel`         tinyint(2) UNSIGNED NOT NULL DEFAULT 0,
    `maxlevel`         tinyint(2) UNSIGNED NOT NULL DEFAULT 0,
    `d_melee_texture1` int(11) NOT NULL DEFAULT 0,
    `d_melee_texture2` int(11) NOT NULL DEFAULT 0,
    `prim_melee_type`  tinyint(4) UNSIGNED NOT NULL DEFAULT 28,
    `sec_melee_type`   tinyint(4) UNSIGNED NOT NULL DEFAULT 28,
    PRIMARY KEY (`id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 61 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `mercs`;
CREATE TABLE `mercs`
(
    `MercID`           int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
    `OwnerCharacterID` int(10) UNSIGNED NOT NULL,
    `Slot`             tinyint(1) UNSIGNED NOT NULL DEFAULT 0,
    `Name`             varchar(64) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
    `TemplateID`       int(10) UNSIGNED NOT NULL DEFAULT 0,
    `SuspendedTime`    int(11) UNSIGNED NOT NULL DEFAULT 0,
    `IsSuspended`      tinyint(1) UNSIGNED NOT NULL DEFAULT 0,
    `TimerRemaining`   int(11) UNSIGNED NOT NULL DEFAULT 0,
    `Gender`           tinyint(3) UNSIGNED NOT NULL DEFAULT 0,
    `MercSize`         float NOT NULL DEFAULT 5,
    `StanceID`         tinyint(3) UNSIGNED NOT NULL DEFAULT 0,
    `HP`               int(11) UNSIGNED NOT NULL DEFAULT 0,
    `Mana`             int(11) UNSIGNED NOT NULL DEFAULT 0,
    `Endurance`        int(11) UNSIGNED NOT NULL DEFAULT 0,
    `Face`             int(10) UNSIGNED NOT NULL DEFAULT 1,
    `LuclinHairStyle`  int(10) UNSIGNED NOT NULL DEFAULT 1,
    `LuclinHairColor`  int(10) UNSIGNED NOT NULL DEFAULT 1,
    `LuclinEyeColor`   int(10) UNSIGNED NOT NULL DEFAULT 1,
    `LuclinEyeColor2`  int(10) UNSIGNED NOT NULL DEFAULT 1,
    `LuclinBeardColor` int(10) UNSIGNED NOT NULL DEFAULT 1,
    `LuclinBeard`      int(10) UNSIGNED NOT NULL DEFAULT 0,
    `DrakkinHeritage`  int(10) UNSIGNED NOT NULL DEFAULT 0,
    `DrakkinTattoo`    int(10) UNSIGNED NOT NULL DEFAULT 0,
    `DrakkinDetails`   int(10) UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`MercID`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 1 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

SET
FOREIGN_KEY_CHECKS = 1;

)",
	},
	ManifestEntry{
		.version = 9217,
		.description = "2023_01_15_chatchannel_reserved_names.sql",
		.check = "SHOW TABLES LIKE 'chatchannel_reserved_names'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `chatchannel_reserved_names`
(
    `id`   int(11) NOT NULL AUTO_INCREMENT,
    `name` varchar(64) NOT NULL,
    PRIMARY KEY (`id`) USING BTREE,
    UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

ALTER TABLE `chatchannels`
    ADD COLUMN `id` int(11) NOT NULL AUTO_INCREMENT FIRST,
DROP PRIMARY KEY,
ADD PRIMARY KEY (`id`) USING BTREE,
ADD UNIQUE INDEX(`name`)

)",
	},
	ManifestEntry{
		.version = 9218,
		.description = "2023_01_24_item_recast.sql",
		.check = "show columns from character_item_recast like '%recast_type%'",
		.condition = "contains",
		.match = "smallint",
		.sql = R"(
ALTER TABLE `character_item_recast`
	CHANGE COLUMN `recast_type` `recast_type` INT(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `id`;
)",
	},
	ManifestEntry{
		.version = 9219,
		.description = "2023_01_29_merchant_status_requirements.sql",
		.check = "SHOW COLUMNS FROM merchantlist LIKE 'min_status'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `merchantlist`
ADD COLUMN `min_status` tinyint(3) UNSIGNED NOT NULL DEFAULT 0 AFTER `level_required`,
ADD COLUMN `max_status` tinyint(3) UNSIGNED NOT NULL DEFAULT 255 AFTER `min_status`;
)"
	},
	ManifestEntry{
		.version = 9220,
		.description = "2022_12_19_player_events_tables.sql",
		.check = "SHOW TABLES LIKE 'player_event_logs'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `player_event_log_settings`
(
    `id`                 bigint(20) NOT NULL,
    `event_name`         varchar(100) DEFAULT NULL,
    `event_enabled`      tinyint(1) DEFAULT NULL,
    `retention_days`     int(11) DEFAULT 0,
    `discord_webhook_id` int(11) DEFAULT 0,
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE `player_event_logs`
(
    `id`              bigint(20) NOT NULL AUTO_INCREMENT,
    `account_id`      bigint(20) DEFAULT NULL,
    `character_id`    bigint(20) DEFAULT NULL,
    `zone_id`         int(11) DEFAULT NULL,
    `instance_id`     int(11) DEFAULT NULL,
    `x`               float        DEFAULT NULL,
    `y`               float        DEFAULT NULL,
    `z`               float        DEFAULT NULL,
    `heading`         float        DEFAULT NULL,
    `event_type_id`   int(11) DEFAULT NULL,
    `event_type_name` varchar(255) DEFAULT NULL,
    `event_data`      longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL CHECK (json_valid(`event_data`)),
    `created_at`      datetime     DEFAULT NULL,
    PRIMARY KEY (`id`),
    KEY               `event_created_at` (`event_type_id`,`created_at`),
    KEY               `zone_id` (`zone_id`),
    KEY               `character_id` (`character_id`,`zone_id`) USING BTREE,
    KEY               `created_at` (`created_at`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4;

DROP TABLE `hackers`;
DROP TABLE `eventlog`;

)"
	},
	ManifestEntry{
		.version = 9221,
		.description = "2023_02_24_npc_scaling_zone_id_instance_version.sql",
		.check = "SHOW COLUMNS FROM `npc_scale_global_base` LIKE 'zone_id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_scale_global_base`
ADD COLUMN `zone_id` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `level`,
ADD COLUMN `instance_version` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `zone_id`,
DROP PRIMARY KEY,
ADD PRIMARY KEY (`type`, `level`, `zone_id`, `instance_version`) USING BTREE;
)"
	},
	ManifestEntry{
		.version = 9222,
		.description = "2023_02_28_npc_scaling_zone_list_version_list.sql",
		.check = "SHOW COLUMNS FROM `npc_scale_global_base` LIKE 'zone_id_list'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_scale_global_base`
    CHANGE COLUMN `zone_id` `zone_id_list` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL AFTER `level`,
    CHANGE COLUMN `instance_version` `instance_version_list` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL AFTER `zone_id_list`,
    DROP PRIMARY KEY,
    ADD PRIMARY KEY (`type`, `level`, `zone_id_list`(255), `instance_version_list`(255)) USING BTREE;
)"
	}, ManifestEntry{
		.version = 9223,
		.description = "2023_03_04_npc_scale_global_base_heroic_strikethrough.sql",
		.check = "SHOW COLUMNS FROM `npc_scale_global_base` LIKE 'heroic_strikethrough'",
		.condition = "empty",
		.match = "",
		.sql = R"(
UPDATE `npc_scale_global_base` SET ac = 0 WHERE ac IS NULL;
UPDATE `npc_scale_global_base` SET hp = 0 WHERE hp IS NULL;
UPDATE `npc_scale_global_base` SET accuracy = 0 WHERE accuracy IS NULL;
UPDATE `npc_scale_global_base` SET slow_mitigation = 0 WHERE slow_mitigation IS NULL;
UPDATE `npc_scale_global_base` SET attack = 0 WHERE attack IS NULL;
UPDATE `npc_scale_global_base` SET strength = 0 WHERE strength IS NULL;
UPDATE `npc_scale_global_base` SET stamina = 0 WHERE stamina IS NULL;
UPDATE `npc_scale_global_base` SET dexterity = 0 WHERE dexterity IS NULL;
UPDATE `npc_scale_global_base` SET agility = 0 WHERE agility IS NULL;
UPDATE `npc_scale_global_base` SET intelligence = 0 WHERE intelligence IS NULL;
UPDATE `npc_scale_global_base` SET wisdom = 0 WHERE wisdom IS NULL;
UPDATE `npc_scale_global_base` SET charisma = 0 WHERE charisma IS NULL;
UPDATE `npc_scale_global_base` SET magic_resist = 0 WHERE magic_resist IS NULL;
UPDATE `npc_scale_global_base` SET cold_resist = 0 WHERE cold_resist IS NULL;
UPDATE `npc_scale_global_base` SET fire_resist = 0 WHERE fire_resist IS NULL;
UPDATE `npc_scale_global_base` SET poison_resist = 0 WHERE poison_resist IS NULL;
UPDATE `npc_scale_global_base` SET disease_resist = 0 WHERE disease_resist IS NULL;
UPDATE `npc_scale_global_base` SET corruption_resist = 0 WHERE corruption_resist IS NULL;
UPDATE `npc_scale_global_base` SET physical_resist = 0 WHERE physical_resist IS NULL;
UPDATE `npc_scale_global_base` SET min_dmg = 0 WHERE min_dmg IS NULL;
UPDATE `npc_scale_global_base` SET max_dmg = 0 WHERE max_dmg IS NULL;
UPDATE `npc_scale_global_base` SET hp_regen_rate = 0 WHERE hp_regen_rate IS NULL;
UPDATE `npc_scale_global_base` SET attack_delay = 0 WHERE attack_delay IS NULL;
UPDATE `npc_scale_global_base` SET physical_resist = 0 WHERE physical_resist IS NULL;
UPDATE `npc_scale_global_base` SET spell_scale = 100 WHERE spell_scale IS NULL;
UPDATE `npc_scale_global_base` SET heal_scale = 100 WHERE heal_scale IS NULL;
UPDATE `npc_scale_global_base` SET special_abilities = '' WHERE special_abilities IS NULL;
ALTER TABLE `npc_scale_global_base`
    MODIFY COLUMN `ac` int(11) NOT NULL DEFAULT 0 AFTER `instance_version_list`,
    MODIFY COLUMN `hp` int(11) NOT NULL DEFAULT 0 AFTER `ac`,
    MODIFY COLUMN `accuracy` int(11) NOT NULL DEFAULT 0 AFTER `hp`,
    MODIFY COLUMN `slow_mitigation` int(11) NOT NULL DEFAULT 0 AFTER `accuracy`,
    MODIFY COLUMN `attack` int(11) NOT NULL DEFAULT 0 AFTER `slow_mitigation`,
    MODIFY COLUMN `strength` int(11) NOT NULL DEFAULT 0 AFTER `attack`,
    MODIFY COLUMN `stamina` int(11) NOT NULL DEFAULT 0 AFTER `strength`,
    MODIFY COLUMN `dexterity` int(11) NOT NULL DEFAULT 0 AFTER `stamina`,
    MODIFY COLUMN `agility` int(11) NOT NULL DEFAULT 0 AFTER `dexterity`,
    MODIFY COLUMN `intelligence` int(11) NOT NULL DEFAULT 0 AFTER `agility`,
    MODIFY COLUMN `wisdom` int(11) NOT NULL DEFAULT 0 AFTER `intelligence`,
    MODIFY COLUMN `charisma` int(11) NOT NULL DEFAULT 0 AFTER `wisdom`,
    MODIFY COLUMN `magic_resist` int(11) NOT NULL DEFAULT 0 AFTER `charisma`,
    MODIFY COLUMN `cold_resist` int(11) NOT NULL DEFAULT 0 AFTER `magic_resist`,
    MODIFY COLUMN `fire_resist` int(11) NOT NULL DEFAULT 0 AFTER `cold_resist`,
    MODIFY COLUMN `poison_resist` int(11) NOT NULL DEFAULT 0 AFTER `fire_resist`,
    MODIFY COLUMN `disease_resist` int(11) NOT NULL DEFAULT 0 AFTER `poison_resist`,
    MODIFY COLUMN `corruption_resist` int(11) NOT NULL DEFAULT 0 AFTER `disease_resist`,
    MODIFY COLUMN `physical_resist` int(11) NOT NULL DEFAULT 0 AFTER `corruption_resist`,
    MODIFY COLUMN `min_dmg` int(11) NOT NULL DEFAULT 0 AFTER `physical_resist`,
    MODIFY COLUMN `max_dmg` int(11) NOT NULL DEFAULT 0 AFTER `min_dmg`,
    MODIFY COLUMN `hp_regen_rate` int(11) NOT NULL DEFAULT 0 AFTER `max_dmg`,
    MODIFY COLUMN `attack_delay` int(11) NOT NULL DEFAULT 0 AFTER `hp_regen_rate`,
    MODIFY COLUMN `spell_scale` int(11) NOT NULL DEFAULT 100 AFTER `attack_delay`,
    MODIFY COLUMN `heal_scale` int(11) NOT NULL DEFAULT 100 AFTER `spell_scale`,
    MODIFY COLUMN special_abilities text CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL AFTER heal_scale,
    ADD COLUMN `heroic_strikethrough` int(11) NOT NULL DEFAULT 0 AFTER `heal_scale`;

)"
	},
	ManifestEntry{
		.version = 9224,
		.description = "2023_03_08_npc_scale_global_base_avoidance.sql",
		.check = "SHOW COLUMNS FROM `npc_scale_global_base` LIKE 'hp_regen_per_second'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_scale_global_base`
MODIFY COLUMN `hp` bigint(20) NOT NULL DEFAULT 0 AFTER `ac`,
MODIFY COLUMN `hp_regen_rate` bigint(20) NOT NULL DEFAULT 0 AFTER `max_dmg`,
ADD COLUMN `hp_regen_per_second` bigint(20) NOT NULL DEFAULT 0 AFTER `hp_regen_rate`,
ADD COLUMN `avoidance` int(11) unsigned NOT NULL DEFAULT 0 AFTER `heal_scale`;

)"
	},
	ManifestEntry{
		.version = 9225,
		.description = "2023_01_21_bots_raid_members.sql",
		.check = "SHOW COLUMNS FROM `raid_members` LIKE 'bot_id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
DROP INDEX `PRIMARY` ON `raid_members`;
CREATE UNIQUE INDEX `UNIQUE` ON `raid_members`(`name`);
ALTER TABLE `raid_members` ADD COLUMN `bot_id` int(4) NOT NULL DEFAULT 0 AFTER `charid`;
ALTER TABLE `raid_members` ADD COLUMN `id` BIGINT UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT FIRST;

)"
	},
	ManifestEntry{
		.version = 9226,
		.description = "2023_03_17_corpse_fields.sql",
		.check = "SHOW COLUMNS FROM `character_corpse_items` LIKE 'custom_data'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `character_corpse_items`
	ADD COLUMN `custom_data` TEXT NULL AFTER `attuned`,
	ADD COLUMN `ornamenticon` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `custom_data`,
	ADD COLUMN `ornamentidfile` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `ornamenticon`,
	ADD COLUMN `ornament_hero_model` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `ornamentidfile`;

)"
	},
	ManifestEntry{
		.version = 9227,
		.description = "2023_03_24_npc_scale_global_base_verify.sql",
		.check = "SHOW COLUMNS FROM `npc_scale_global_base` LIKE 'heroic_strikethrough'",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
UPDATE `npc_scale_global_base` SET ac = 0 WHERE ac IS NULL;
UPDATE `npc_scale_global_base` SET hp = 0 WHERE hp IS NULL;
UPDATE `npc_scale_global_base` SET accuracy = 0 WHERE accuracy IS NULL;
UPDATE `npc_scale_global_base` SET slow_mitigation = 0 WHERE slow_mitigation IS NULL;
UPDATE `npc_scale_global_base` SET attack = 0 WHERE attack IS NULL;
UPDATE `npc_scale_global_base` SET strength = 0 WHERE strength IS NULL;
UPDATE `npc_scale_global_base` SET stamina = 0 WHERE stamina IS NULL;
UPDATE `npc_scale_global_base` SET dexterity = 0 WHERE dexterity IS NULL;
UPDATE `npc_scale_global_base` SET agility = 0 WHERE agility IS NULL;
UPDATE `npc_scale_global_base` SET intelligence = 0 WHERE intelligence IS NULL;
UPDATE `npc_scale_global_base` SET wisdom = 0 WHERE wisdom IS NULL;
UPDATE `npc_scale_global_base` SET charisma = 0 WHERE charisma IS NULL;
UPDATE `npc_scale_global_base` SET magic_resist = 0 WHERE magic_resist IS NULL;
UPDATE `npc_scale_global_base` SET cold_resist = 0 WHERE cold_resist IS NULL;
UPDATE `npc_scale_global_base` SET fire_resist = 0 WHERE fire_resist IS NULL;
UPDATE `npc_scale_global_base` SET poison_resist = 0 WHERE poison_resist IS NULL;
UPDATE `npc_scale_global_base` SET disease_resist = 0 WHERE disease_resist IS NULL;
UPDATE `npc_scale_global_base` SET corruption_resist = 0 WHERE corruption_resist IS NULL;
UPDATE `npc_scale_global_base` SET physical_resist = 0 WHERE physical_resist IS NULL;
UPDATE `npc_scale_global_base` SET min_dmg = 0 WHERE min_dmg IS NULL;
UPDATE `npc_scale_global_base` SET max_dmg = 0 WHERE max_dmg IS NULL;
UPDATE `npc_scale_global_base` SET hp_regen_rate = 0 WHERE hp_regen_rate IS NULL;
UPDATE `npc_scale_global_base` SET attack_delay = 0 WHERE attack_delay IS NULL;
UPDATE `npc_scale_global_base` SET physical_resist = 0 WHERE physical_resist IS NULL;
UPDATE `npc_scale_global_base` SET spell_scale = 100 WHERE spell_scale IS NULL;
UPDATE `npc_scale_global_base` SET heal_scale = 100 WHERE heal_scale IS NULL;
UPDATE `npc_scale_global_base` SET special_abilities = '' WHERE special_abilities IS NULL;
ALTER TABLE `npc_scale_global_base`
    MODIFY COLUMN `ac` int(11) NOT NULL DEFAULT 0 AFTER `instance_version_list`,
    MODIFY COLUMN `hp` bigint(20) NOT NULL DEFAULT 0 AFTER `ac`,
    MODIFY COLUMN `accuracy` int(11) NOT NULL DEFAULT 0 AFTER `hp`,
    MODIFY COLUMN `slow_mitigation` int(11) NOT NULL DEFAULT 0 AFTER `accuracy`,
    MODIFY COLUMN `attack` int(11) NOT NULL DEFAULT 0 AFTER `slow_mitigation`,
    MODIFY COLUMN `strength` int(11) NOT NULL DEFAULT 0 AFTER `attack`,
    MODIFY COLUMN `stamina` int(11) NOT NULL DEFAULT 0 AFTER `strength`,
    MODIFY COLUMN `dexterity` int(11) NOT NULL DEFAULT 0 AFTER `stamina`,
    MODIFY COLUMN `agility` int(11) NOT NULL DEFAULT 0 AFTER `dexterity`,
    MODIFY COLUMN `intelligence` int(11) NOT NULL DEFAULT 0 AFTER `agility`,
    MODIFY COLUMN `wisdom` int(11) NOT NULL DEFAULT 0 AFTER `intelligence`,
    MODIFY COLUMN `charisma` int(11) NOT NULL DEFAULT 0 AFTER `wisdom`,
    MODIFY COLUMN `magic_resist` int(11) NOT NULL DEFAULT 0 AFTER `charisma`,
    MODIFY COLUMN `cold_resist` int(11) NOT NULL DEFAULT 0 AFTER `magic_resist`,
    MODIFY COLUMN `fire_resist` int(11) NOT NULL DEFAULT 0 AFTER `cold_resist`,
    MODIFY COLUMN `poison_resist` int(11) NOT NULL DEFAULT 0 AFTER `fire_resist`,
    MODIFY COLUMN `disease_resist` int(11) NOT NULL DEFAULT 0 AFTER `poison_resist`,
    MODIFY COLUMN `corruption_resist` int(11) NOT NULL DEFAULT 0 AFTER `disease_resist`,
    MODIFY COLUMN `physical_resist` int(11) NOT NULL DEFAULT 0 AFTER `corruption_resist`,
    MODIFY COLUMN `min_dmg` int(11) NOT NULL DEFAULT 0 AFTER `physical_resist`,
    MODIFY COLUMN `max_dmg` int(11) NOT NULL DEFAULT 0 AFTER `min_dmg`,
    MODIFY COLUMN `hp_regen_rate` bigint(20) NOT NULL DEFAULT 0 AFTER `max_dmg`,
    MODIFY COLUMN `attack_delay` int(11) NOT NULL DEFAULT 0 AFTER `hp_regen_rate`,
    MODIFY COLUMN `hp_regen_per_second` bigint(20) NOT NULL DEFAULT 0 AFTER `hp_regen_rate`,
    MODIFY COLUMN `spell_scale` int(11) NOT NULL DEFAULT 100 AFTER `attack_delay`,
    MODIFY COLUMN `heal_scale` int(11) NOT NULL DEFAULT 100 AFTER `spell_scale`,
    MODIFY COLUMN `heroic_strikethrough` int(11) NOT NULL DEFAULT 0 AFTER `avoidance`,
    MODIFY COLUMN `avoidance` int(11) unsigned NOT NULL DEFAULT 0 AFTER `heal_scale`,
    MODIFY COLUMN special_abilities text CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL AFTER heroic_strikethrough;

)"
	},
	ManifestEntry{
		.version = 9228,
		.description = "2023_05_08_character_tribute_primary_key.sql",
		.check = "SHOW COLUMNS FROM `character_tribute` LIKE 'character_id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `character_tribute`
CHANGE COLUMN `id` `character_id` int(11) UNSIGNED NOT NULL DEFAULT 0,
ADD COLUMN `id` int(11) NOT NULL AUTO_INCREMENT FIRST,
ADD PRIMARY KEY (`id`);
)",
	},
	ManifestEntry{
		.version = 9229,
		.description = "2023_07_04_chatchannel_reserved_names_fix.sql",
		.check = "SHOW TABLES LIKE 'chatchannel_reserved_names'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `chatchannel_reserved_names`
(
`id`   int(11) NOT NULL AUTO_INCREMENT,
`name` varchar(64) NOT NULL,
PRIMARY KEY (`id`) USING BTREE,
UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
)"
	},
	ManifestEntry{
		.version = 9230,
		.description = "2023_06_23_raid_feature_updates",
		.check = "SHOW COLUMNS FROM `raid_members` LIKE 'is_assister'",
		.condition = "empty",
		.match = "",
		.sql = R"(
	ALTER TABLE `raid_members`
	ADD COLUMN `is_marker` TINYINT UNSIGNED DEFAULT 0 NOT NULL AFTER `islooter`,
	ADD COLUMN `is_assister` TINYINT UNSIGNED DEFAULT 0 NOT NULL AFTER `is_marker`,
	ADD COLUMN `note` VARCHAR(64) DEFAULT '' NOT NULL AFTER `is_assister`;

	ALTER TABLE `raid_details`
	ADD COLUMN `marked_npc_1` SMALLINT UNSIGNED DEFAULT 0 NOT NULL AFTER `motd`,
	ADD COLUMN `marked_npc_2` SMALLINT UNSIGNED DEFAULT 0 NOT NULL AFTER `marked_npc_1`,
	ADD COLUMN `marked_npc_3` SMALLINT UNSIGNED DEFAULT 0 NOT NULL AFTER `marked_npc_2`;
	)",
	},
	ManifestEntry{
		.version = 9231,
		.description = "2023_07_14_npc_unsigned_melee_texture.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'd_melee_texture1'",
		.condition = "contains",
		.match = "int(11) signed",
		.sql = R"(ALTER TABLE `npc_types`
	MODIFY COLUMN `d_melee_texture1` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `armortint_blue`,
	MODIFY COLUMN `d_melee_texture2` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `d_melee_texture1`;
	)",
	},
	ManifestEntry{
		.version = 9232,
		.description = "2023_07_11_command_subsettings.sql",
		.check = "SHOW TABLES LIKE 'command_subsettings'",
		.condition = "empty",
		.match = "",
		.sql = R"(CREATE TABLE `command_subsettings` (
`id` int UNSIGNED NOT NULL AUTO_INCREMENT,
`parent_command` varchar(32) NOT NULL,
`sub_command` varchar(32) NOT NULL,
`access_level` int(11) UNSIGNED NOT NULL DEFAULT 0,
`top_level_aliases` varchar(255) NOT NULL,
PRIMARY KEY (`id`),
UNIQUE INDEX `command`(`parent_command`, `sub_command`)
)
)"
	},
	ManifestEntry{
		.version = 9233,
		.description = "2023_07_16_scoped_data_buckets.sql",
		.check = "SHOW COLUMNS FROM `data_buckets` LIKE 'character_id'",
		.condition = "empty",
		.match = "",
		.sql = R"(

ALTER TABLE `data_buckets`
ADD COLUMN `character_id` bigint(11) NOT NULL DEFAULT 0 AFTER `expires`,
ADD COLUMN `npc_id` bigint(11) NOT NULL DEFAULT 0 AFTER `character_id`,
ADD COLUMN `bot_id` bigint(11) NOT NULL DEFAULT 0 AFTER `npc_id`,
DROP INDEX `key_index`,
ADD UNIQUE INDEX `keys`(`key`,`character_id`,`npc_id`,`bot_id`);

UPDATE data_buckets SET character_id = SUBSTRING_INDEX(SUBSTRING_INDEX( `key`, '-', 2 ), '-', -1), `key` = SUBSTR(SUBSTRING_INDEX(`key`, SUBSTRING_INDEX( `key`, '-', 2 ), -1), 2) WHERE `key` LIKE 'character-%';
UPDATE data_buckets SET npc_id = SUBSTRING_INDEX(SUBSTRING_INDEX( `key`, '-', 2 ), '-', -1), `key` = SUBSTR(SUBSTRING_INDEX(`key`, SUBSTRING_INDEX( `key`, '-', 2 ), -1), 2) WHERE `key` LIKE 'npc-%';
UPDATE data_buckets SET bot_id = SUBSTRING_INDEX(SUBSTRING_INDEX( `key`, '-', 2 ), '-', -1), `key` = SUBSTR(SUBSTRING_INDEX(`key`, SUBSTRING_INDEX( `key`, '-', 2 ), -1), 2) WHERE `key` LIKE 'bot-%';

)"
	},
	ManifestEntry{
		.version = 9234,
		.description = "2023_07_27_update_raid_details.sql",
		.check = "SHOW COLUMNS FROM `raid_details` LIKE 'marked_npc_1_entity_id';",
		.condition = "empty",
		.match = "",
		.sql = R"(ALTER TABLE `raid_details`
			CHANGE COLUMN `marked_npc_1` `marked_npc_1_entity_id` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `motd`,
			ADD COLUMN `marked_npc_1_zone_id` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `marked_npc_1_entity_id`,
			ADD COLUMN `marked_npc_1_instance_id` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `marked_npc_1_zone_id`,
			CHANGE COLUMN `marked_npc_2` `marked_npc_2_entity_id` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `marked_npc_1_instance_id`,
			ADD COLUMN `marked_npc_2_zone_id` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `marked_npc_2_entity_id`,
			ADD COLUMN `marked_npc_2_instance_id` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `marked_npc_2_zone_id`,
			CHANGE COLUMN `marked_npc_3` `marked_npc_3_entity_id` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `marked_npc_2_instance_id`,
			ADD COLUMN `marked_npc_3_zone_id` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `marked_npc_3_entity_id`,
			ADD COLUMN `marked_npc_3_instance_id` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `marked_npc_3_zone_id`;
		)"
	},
	ManifestEntry{
		.version = 9235,
		.description = "2023_07_31_character_stats_record.sql",
		.check = "SHOW TABLES LIKE 'character_stats_record'",
		.condition = "empty",
		.match = "",
		.sql = R"(

CREATE TABLE `character_stats_record`  (
  `character_id` int NOT NULL,
  `name` varchar(100) NULL,
  `status` int NULL DEFAULT 0,
  `level` int NULL DEFAULT 0,
  `class` int NULL DEFAULT 0,
  `race` int NULL DEFAULT 0,
  `aa_points` int NULL DEFAULT 0,
  `hp` bigint NULL DEFAULT 0,
  `mana` bigint NULL DEFAULT 0,
  `endurance` bigint NULL DEFAULT 0,
  `ac` int NULL DEFAULT 0,
  `strength` int NULL DEFAULT 0,
  `stamina` int NULL DEFAULT 0,
  `dexterity` int NULL DEFAULT 0,
  `agility` int NULL DEFAULT 0,
  `intelligence` int NULL DEFAULT 0,
  `wisdom` int NULL DEFAULT 0,
  `charisma` int NULL DEFAULT 0,
  `magic_resist` int NULL DEFAULT 0,
  `fire_resist` int NULL DEFAULT 0,
  `cold_resist` int NULL DEFAULT 0,
  `poison_resist` int NULL DEFAULT 0,
  `disease_resist` int NULL DEFAULT 0,
  `corruption_resist` int NULL DEFAULT 0,
  `heroic_strength` int NULL DEFAULT 0,
  `heroic_stamina` int NULL DEFAULT 0,
  `heroic_dexterity` int NULL DEFAULT 0,
  `heroic_agility` int NULL DEFAULT 0,
  `heroic_intelligence` int NULL DEFAULT 0,
  `heroic_wisdom` int NULL DEFAULT 0,
  `heroic_charisma` int NULL DEFAULT 0,
  `heroic_magic_resist` int NULL DEFAULT 0,
  `heroic_fire_resist` int NULL DEFAULT 0,
  `heroic_cold_resist` int NULL DEFAULT 0,
  `heroic_poison_resist` int NULL DEFAULT 0,
  `heroic_disease_resist` int NULL DEFAULT 0,
  `heroic_corruption_resist` int NULL DEFAULT 0,
  `haste` int NULL DEFAULT 0,
  `accuracy` int NULL DEFAULT 0,
  `attack` int NULL DEFAULT 0,
  `avoidance` int NULL DEFAULT 0,
  `clairvoyance` int NULL DEFAULT 0,
  `combat_effects` int NULL DEFAULT 0,
  `damage_shield_mitigation` int NULL DEFAULT 0,
  `damage_shield` int NULL DEFAULT 0,
  `dot_shielding` int NULL DEFAULT 0,
  `hp_regen` int NULL DEFAULT 0,
  `mana_regen` int NULL DEFAULT 0,
  `endurance_regen` int NULL DEFAULT 0,
  `shielding` int NULL DEFAULT 0,
  `spell_damage` int NULL DEFAULT 0,
  `spell_shielding` int NULL DEFAULT 0,
  `strikethrough` int NULL DEFAULT 0,
  `stun_resist` int NULL DEFAULT 0,
  `backstab` int NULL DEFAULT 0,
  `wind` int NULL DEFAULT 0,
  `brass` int NULL DEFAULT 0,
  `string` int NULL DEFAULT 0,
  `percussion` int NULL DEFAULT 0,
  `singing` int NULL DEFAULT 0,
  `baking` int NULL DEFAULT 0,
  `alchemy` int NULL DEFAULT 0,
  `tailoring` int NULL DEFAULT 0,
  `blacksmithing` int NULL DEFAULT 0,
  `fletching` int NULL DEFAULT 0,
  `brewing` int NULL DEFAULT 0,
  `jewelry` int NULL DEFAULT 0,
  `pottery` int NULL DEFAULT 0,
  `research` int NULL DEFAULT 0,
  `alcohol` int NULL DEFAULT 0,
  `fishing` int NULL DEFAULT 0,
  `tinkering` int NULL DEFAULT 0,
  `created_at` datetime DEFAULT NULL,
  `updated_at` datetime DEFAULT NULL,
  PRIMARY KEY (`character_id`)
);
)"
	},
	ManifestEntry{
		.version = 9236,
		.description = "2023_08_24_aa_ability_auto_grant.sql",
		.check = "SHOW COLUMNS FROM `aa_ability` LIKE 'auto_grant_enabled';",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `aa_ability` ADD COLUMN `auto_grant_enabled` TINYINT(4) NOT NULL DEFAULT '0' AFTER `reset_on_death`;
UPDATE `aa_ability` SET `auto_grant_enabled` = 1 WHERE `grant_only` = 0 AND `charges` = 0 AND `category` = -1;
)"
	},
	ManifestEntry{
		.version = 9237,
		.description = "2023_10_15_import_13th_floor.sql",
		.check = "SHOW COLUMNS FROM `items` LIKE 'bardeffect';",
		.condition = "missing",
		.match = "mediumint",
		.sql = R"(
ALTER TABLE `items`
 MODIFY COLUMN `scriptfileid`        MEDIUMINT(6)     NOT NULL DEFAULT 0,
 MODIFY COLUMN `powersourcecapacity` MEDIUMINT(7)     NOT NULL DEFAULT 0,
 MODIFY COLUMN `augdistiller`        INT(11) UNSIGNED NOT NULL DEFAULT 0,
 MODIFY COLUMN `scrollunk1`          INT(11) UNSIGNED NOT NULL DEFAULT 0,
 MODIFY COLUMN `bardeffect`          MEDIUMINT(6)     NOT NULL DEFAULT 0;
)"
	},
	ManifestEntry{
		.version = 9238,
		.description = "2023_10_18_tradeskill_add_learned_by_item_id.sql",
		.check = "SHOW COLUMNS FROM `tradeskill_recipe` LIKE 'learned_by_item_id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `tradeskill_recipe`
	ADD COLUMN `learned_by_item_id` int(11) NOT NULL DEFAULT 0 AFTER `must_learn`;
)"
	},
	ManifestEntry{
		.version = 9239,
		.description = "2023_10_18_blocked_spells_expansions_content_flags.sql",
		.check = "SHOW COLUMNS FROM `blocked_spells` LIKE 'min_expansion'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `blocked_spells`
ADD COLUMN `min_expansion` tinyint(4) NOT NULL DEFAULT -1 AFTER `description`,
ADD COLUMN `max_expansion` tinyint(4) NOT NULL DEFAULT -1 AFTER `min_expansion`,
ADD COLUMN `content_flags` varchar(100) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL AFTER `max_expansion`,
ADD COLUMN `content_flags_disabled` varchar(100) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL AFTER `content_flags`;
)"
	},
	ManifestEntry{
		.version = 9240,
		.description = "2023_10_29_variables_id.sql",
		.check = "SHOW COLUMNS FROM `variables` LIKE 'id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `variables`
ADD COLUMN `id` int(11) NOT NULL AUTO_INCREMENT FIRST,
DROP PRIMARY KEY,
ADD PRIMARY KEY (`id`) USING BTREE,
ADD UNIQUE INDEX(`varname`);
)"
	},
	ManifestEntry{
		.version = 9241,
		.description = "2023_10_29_split_spawn2_enabled.sql",
		.check = "SHOW TABLES LIKE 'spawn2_disabled'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `spawn2_backup_2023_10_29` LIKE `spawn2`;
INSERT INTO `spawn2_backup_2023_10_29` SELECT * FROM `spawn2`;
CREATE TABLE `spawn2_disabled` (
  `id` bigint(11) NOT NULL AUTO_INCREMENT,
  `spawn2_id` int(11) DEFAULT NULL,
  `instance_id` int(11) DEFAULT 0,
  `disabled` smallint(11) DEFAULT 0,
  PRIMARY KEY (`id`),
  UNIQUE KEY `spawn2_id` (`spawn2_id`,`instance_id`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4;
INSERT INTO spawn2_disabled (spawn2_id, disabled) SELECT id, 1 FROM spawn2 WHERE enabled = 0;
ALTER TABLE `spawn2` DROP COLUMN `enabled`;
)",
	},
	ManifestEntry{
		.version = 9242,
		.description = "2023_11_7_mintime_maxtime_spawnentry.sql",
		.check = "SHOW COLUMNS FROM `spawnentry` LIKE 'min_time'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `spawnentry`
ADD COLUMN `min_time` smallint(4) NOT NULL DEFAULT 0 AFTER `condition_value_filter`,
ADD COLUMN `max_time` smallint(4) NOT NULL DEFAULT 0 AFTER `min_time`;
)",
		.content_schema_update = true
	},
	ManifestEntry{
		.version = 9243,
		.description = "2023_11_27_starting_items_revamp.sql",
		.check = "SHOW COLUMNS FROM `starting_items` LIKE 'race_list'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `starting_items_backup_9243` LIKE `starting_items`;
INSERT INTO `starting_items_backup_9243` SELECT * FROM `starting_items`;

CREATE TABLE `starting_items_new`  (
  `id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `race_list` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `class_list` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `deity_list` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `zone_id_list` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `item_id` int(11) UNSIGNED NOT NULL DEFAULT 0,
  `item_charges` tinyint(3) UNSIGNED NOT NULL DEFAULT 1,
  `gm` mediumint(3) UNSIGNED NOT NULL DEFAULT 0,
  `slot` mediumint(9) NOT NULL DEFAULT -1,
  `min_expansion` tinyint(4) NOT NULL DEFAULT -1,
  `max_expansion` tinyint(4) NOT NULL DEFAULT -1,
  `content_flags` varchar(100) NULL,
  `content_flags_disabled` varchar(100) NULL,
  PRIMARY KEY (`id`)
);

INSERT INTO
`starting_items_new`
(
	SELECT
		0 AS `id`,
		GROUP_CONCAT(DISTINCT `class` ORDER BY class ASC SEPARATOR '|') AS `class_list`,
		GROUP_CONCAT(DISTINCT `race` ORDER BY race ASC SEPARATOR '|') AS `race_list`,
		GROUP_CONCAT(DISTINCT `deityid` ORDER BY deityid ASC SEPARATOR '|') AS `deity_list`,
		GROUP_CONCAT(DISTINCT `zoneid` ORDER BY zoneid ASC SEPARATOR '|') AS `zone_list`,
		`itemid`,
		`item_charges`,
		`gm`,
		`slot`,
		`min_expansion`,
		`max_expansion`,
		`content_flags`,
		`content_flags_disabled`
	FROM
		`starting_items`
	GROUP BY
		`itemid`
);

DROP TABLE `starting_items`;
RENAME TABLE `starting_items_new` TO `starting_items`;
)",
		.content_schema_update = true
	},
	ManifestEntry{
		.version = 9244,
		.description = "2023_11_30_items_table_schema.sql",
		.check = "SHOW COLUMNS FROM `items` LIKE 'updated'",
		.condition = "contains",
		.match = "0000-00-00 00:00:00",
		.sql = R"(
ALTER TABLE `items` MODIFY COLUMN `updated` datetime NULL DEFAULT NULL;
		)",
		.content_schema_update = true
	},
	ManifestEntry{
		.version = 9245,
		.description = "2023_12_03_object_incline.sql",
		.check = "SHOW COLUMNS FROM `object` LIKE 'incline'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `object` CHANGE COLUMN `unknown08` `size_percentage` float NOT NULL DEFAULT 0 AFTER `icon`;
ALTER TABLE `object` CHANGE COLUMN `unknown10` `solid_type` mediumint(5) NOT NULL DEFAULT 0 AFTER `size`;
ALTER TABLE `object` CHANGE COLUMN `unknown20` `incline` int(11) NOT NULL DEFAULT 0 AFTER `solid_type`;
)",
		.content_schema_update = true
	},
	ManifestEntry{
		.version = 9246,
		.description = "2023_12_07_keyring_id.sql",
		.check = "SHOW COLUMNS FROM `keyring` LIKE 'id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `keyring`
ADD COLUMN `id` int UNSIGNED NOT NULL AUTO_INCREMENT FIRST,
ADD PRIMARY KEY (`id`);
)"
	},
	ManifestEntry{
		.version = 9247,
		.description = "2023_12_14_starting_items_fix.sql",
		.check = "SHOW COLUMNS FROM `starting_items` LIKE 'inventory_slot'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `starting_items`
CHANGE COLUMN `race_list` `temporary` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL AFTER `id`,
CHANGE COLUMN `class_list` `race_list` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL AFTER `temporary`,
CHANGE COLUMN `gm` `status` mediumint(3) NOT NULL DEFAULT 0 AFTER `item_charges`,
CHANGE COLUMN `slot` `inventory_slot` mediumint(9) NOT NULL DEFAULT -1 AFTER `status`;

ALTER TABLE `starting_items`
CHANGE COLUMN `temporary` `class_list` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL AFTER `id`;
)",
		.content_schema_update = true
	},
	ManifestEntry{
		.version = 9248,
		.description = "2023_12_22_drop_npc_emotes_index.sql",
		.check = "show index from npc_emotes where key_name = 'emoteid'",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_emotes` DROP INDEX `emoteid`;
)",
		.content_schema_update = true
	},
	ManifestEntry{
		.version = 9249,
		.description = "2023_12_26_add_tasks_enabled_column.sql",
		.check = "SHOW COLUMNS FROM `tasks` LIKE 'enabled'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `tasks`
ADD COLUMN `enabled` smallint NULL DEFAULT 1 AFTER `faction_amount`
)",
		.content_schema_update = true
	},
	ManifestEntry{
		.version = 9250,
		.description = "2023_01_06_task_activities_list_group.sql",
		.check = "SHOW COLUMNS FROM `task_activities` LIKE 'list_group'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `task_activities`
	ADD COLUMN `list_group` TINYINT UNSIGNED NOT NULL DEFAULT '0' AFTER `optional`;
)",
		.content_schema_update = true
	},
	ManifestEntry{
		.version = 9251,
		.description = "2023_01_12_instance_list_notes.sql",
		.check = "SHOW COLUMNS FROM `instance_list` LIKE 'notes'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `instance_list`
	ADD COLUMN `notes` varchar(50) NOT NULL DEFAULT '' AFTER `never_expires`;
)",
	},
	ManifestEntry{
		.version = 9252,
		.description = "2024_01_07_zone_idle_when_empty.sql",
		.check = "SHOW COLUMNS FROM `zone` LIKE 'idle_when_empty'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `zone`
ADD COLUMN `idle_when_empty` tinyint(1) UNSIGNED NOT NULL DEFAULT 1 AFTER `min_lava_damage`,
ADD COLUMN `seconds_before_idle` int(11) UNSIGNED NOT NULL DEFAULT 60 AFTER `idle_when_empty`;
)",
		.content_schema_update = true
	},
	ManifestEntry{
		.version = 9253,
		.description = "2024_01_13_merchantlist_slot.sql",
		.check = "SHOW COLUMNS FROM `merchantlist` LIKE 'slot'",
		.condition = "missing",
		.match = "unsigned",
		.sql = R"(
ALTER TABLE `merchantlist`
	MODIFY COLUMN `slot` int(11) UNSIGNED NOT NULL DEFAULT 0
)",
		.content_schema_update = true
	},
	ManifestEntry{
		.version = 9254,
		.description = "2024_01_13_merchantlist_temp_slot.sql",
		.check = "SHOW COLUMNS FROM `merchantlist_temp` LIKE 'slot'",
		.condition = "contains",
		.match = "tinyint",
		.sql = R"(
ALTER TABLE `merchantlist_temp`
	MODIFY COLUMN `slot` int(11) UNSIGNED NOT NULL DEFAULT 0
)"
	},
	ManifestEntry{
		.version = 9255,
		.description = "2024_01_13_drop_item_tick_deprecated.sql",
		.check = "show tables like 'item_tick'",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
DROP TABLE IF EXISTS item_tick
)"
	},
	ManifestEntry{
		.version = 9256,
		.description = "2024_01_16_increase_spawngroup_size.sql",
		.check = "SHOW COLUMNS FROM `spawngroup` LIKE 'name'",
		.condition = "contains",
		.match = "varchar(50)",
		.sql = R"(
ALTER TABLE `spawngroup`
MODIFY COLUMN `name` varchar(200) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '' AFTER `id`;
)",
		.content_schema_update = true
	},
	ManifestEntry{
		.version = 9257,
		.description = "2024_01_16_ground_spawns_fix_z.sql",
		.check = "SHOW COLUMNS FROM `ground_spawns` LIKE 'fix_z'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `ground_spawns`
ADD COLUMN `fix_z` tinyint(1) UNSIGNED NOT NULL DEFAULT 1 AFTER `respawn_timer`;
)",
		.content_schema_update = true
	},
	ManifestEntry{
		.version = 9258,
		.description = "2024_02_04_base_data.sql",
		.check = "SHOW COLUMNS FROM `base_data` LIKE 'hp_regen'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `base_data`
CHANGE COLUMN `unk1` `hp_regen` double NOT NULL AFTER `end`,
CHANGE COLUMN `unk2` `end_regen` double NOT NULL AFTER `hp_regen`,
MODIFY COLUMN `level` tinyint(3) UNSIGNED NOT NULL FIRST,
MODIFY COLUMN `class` tinyint(2) UNSIGNED NOT NULL AFTER `level`;
)",
		.content_schema_update = true
	},
	ManifestEntry{
		.version = 9259,
		.description = "2024_01_13_corpse_rez_overhaul.sql",
		.check = "SHOW COLUMNS FROM `character_corpses` LIKE 'rez_time'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `character_corpses`
ADD COLUMN `rez_time` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `wc_9`,
ADD COLUMN `gm_exp` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `rez_time`,
ADD COLUMN `killed_by` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `gm_exp`,
ADD COLUMN `rezzable` tinyint(1) UNSIGNED NOT NULL DEFAULT 0 AFTER `killed_by`;
)"
	},
	ManifestEntry{
		.version = 9260,
		.description = "2023_11_11_guild_features.sql",
		.check = "SHOW TABLES LIKE 'guild_permissions'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `guild_permissions` (
	`id` INT(11) NOT NULL AUTO_INCREMENT,
	`perm_id` INT(11) NOT NULL DEFAULT '0',
	`guild_id` INT(11) NOT NULL DEFAULT '0',
	`permission` INT(11) NOT NULL DEFAULT '0',
	PRIMARY KEY (`id`) USING BTREE,
	UNIQUE INDEX `perm_id_guild_id` (`perm_id`, `guild_id`) USING BTREE
)
ENGINE=InnoDB
AUTO_INCREMENT=1;

UPDATE guild_ranks SET title = 'Leader' WHERE `rank` = '1';
UPDATE guild_ranks SET title = 'Senior Officer' WHERE `rank` = '2';
UPDATE guild_ranks SET title = 'Officer' WHERE `rank` = '3';
UPDATE guild_ranks SET title = 'Senior Member' WHERE `rank` = '4';
UPDATE guild_ranks SET title = 'Member' WHERE `rank` = '5';
UPDATE guild_ranks SET title = 'Junior Member' WHERE `rank` = '6';
UPDATE guild_ranks SET title = 'Initiate' WHERE `rank` = '7';
UPDATE guild_ranks SET title = 'Recruit' WHERE `rank` = '8';

DELETE FROM guild_ranks WHERE `rank` = 0;

ALTER TABLE `guild_ranks`
	DROP COLUMN `can_hear`,
	DROP COLUMN `can_speak`,
	DROP COLUMN `can_invite`,
	DROP COLUMN `can_remove`,
	DROP COLUMN `can_promote`,
	DROP COLUMN `can_demote`,
	DROP COLUMN `can_motd`,
	DROP COLUMN `can_warpeace`;

UPDATE guild_members SET `rank` = '5' WHERE `rank` = '0';
UPDATE guild_members SET `rank` = '3' WHERE `rank` = '1';
UPDATE guild_members SET `rank` = '1' WHERE `rank` = '2';

ALTER TABLE `guild_members`
	ADD COLUMN `online` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0' AFTER `alt`;

ALTER TABLE `guilds`
	ADD COLUMN `favor` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `url`;

CREATE TABLE guild_tributes (
  guild_id int(11) unsigned NOT NULL DEFAULT 0,
  tribute_id_1 int(11) unsigned NOT NULL DEFAULT 0,
  tribute_id_1_tier int(11) unsigned NOT NULL DEFAULT 0,
  tribute_id_2 int(11) unsigned NOT NULL DEFAULT 0,
  tribute_id_2_tier int(11) unsigned NOT NULL DEFAULT 0,
  time_remaining int(11) unsigned NOT NULL DEFAULT 0,
  enabled int(11) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (guild_id) USING BTREE
) ENGINE=InnoDB;
)"
	},
	ManifestEntry{
		.version = 9261,
		.description = "2024_02_11_character_corpses.sql",
		.check = "SHOW COLUMNS FROM `character_corpses` LIKE 'time_of_death'",
		.condition = "contains",
		.match = "0000-00-00 00:00:00",
		.sql = R"(
ALTER TABLE `character_corpses` MODIFY COLUMN `time_of_death` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP;
		)"
	},
	ManifestEntry{
		.version = 9262,
		.description = "2024_02_11_object_contents.sql",
		.check = "SHOW COLUMNS FROM `object_contents` LIKE 'droptime'",
		.condition = "contains",
		.match = "0000-00-00 00:00:00",
		.sql = R"(
ALTER TABLE `object_contents` MODIFY COLUMN `droptime` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP;
		)"
	},
	ManifestEntry{
		.version = 9263,
		.description = "2024_02_16_rearrange_zone_columns.sql",
		.check = "show columns from zone like 'note'",
		.condition = "missing",
		.match = "varchar(200)",
		.sql = R"(
ALTER TABLE `zone`
MODIFY COLUMN `id` int(10) NOT NULL AUTO_INCREMENT FIRST,
MODIFY COLUMN `zoneidnumber` int(4) NOT NULL DEFAULT 0 AFTER `id`,
MODIFY COLUMN `version` tinyint(3) UNSIGNED NOT NULL DEFAULT 0 AFTER `zoneidnumber`,
MODIFY COLUMN `short_name` varchar(32) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL AFTER `version`,
MODIFY COLUMN `long_name` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL AFTER `short_name`,
MODIFY COLUMN `min_status` tinyint(3) UNSIGNED NOT NULL DEFAULT 0 AFTER `long_name`,
MODIFY COLUMN `note` varchar(200) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL AFTER `map_file_name`,
MODIFY COLUMN `min_expansion` tinyint(4) NOT NULL DEFAULT -1 AFTER `note`,
MODIFY COLUMN `max_expansion` tinyint(4) NOT NULL DEFAULT -1 AFTER `min_expansion`,
MODIFY COLUMN `content_flags` varchar(100) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL AFTER `max_expansion`,
MODIFY COLUMN `content_flags_disabled` varchar(100) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL AFTER `content_flags`,
MODIFY COLUMN `expansion` tinyint(3) NOT NULL DEFAULT 0 AFTER `content_flags_disabled`,
MODIFY COLUMN `file_name` varchar(16) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL AFTER `expansion`,
MODIFY COLUMN `safe_x` float NOT NULL DEFAULT 0 AFTER `file_name`,
MODIFY COLUMN `safe_y` float NOT NULL DEFAULT 0 AFTER `safe_x`,
MODIFY COLUMN `safe_z` float NOT NULL DEFAULT 0 AFTER `safe_y`,
MODIFY COLUMN `safe_heading` float NOT NULL DEFAULT 0 AFTER `safe_z`;
		)",
		.content_schema_update = true
	},
	ManifestEntry{
		.version = 9264,
		.description = "2024_02_18_starting_items_augments.sql",
		.check = "SHOW COLUMNS FROM `starting_items` LIKE 'augment_one'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `starting_items`
ADD COLUMN `augment_one` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `item_charges`,
ADD COLUMN `augment_two` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `augment_one`,
ADD COLUMN `augment_three` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `augment_two`,
ADD COLUMN `augment_four` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `augment_three`,
ADD COLUMN `augment_five` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `augment_four`,
ADD COLUMN `augment_six` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `augment_five`;
		)",
		.content_schema_update = true
	},
	ManifestEntry{
		.version = 9265,
		.description = "2024_03_03_add_id_to_guild_bank.sql",
		.check = "SHOW COLUMNS FROM `guild_bank` LIKE 'id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `guild_bank`
ADD COLUMN `id` INT UNSIGNED NOT NULL AUTO_INCREMENT FIRST,
ADD PRIMARY KEY (`id`);
		)",
	},
	ManifestEntry{
		.version = 9266,
		.description = "2024_03_02_rule_values_rule_value_length.sql",
		.check = "SHOW COLUMNS FROM `rule_values` LIKE 'rule_value'",
		.condition = "contains",
		.match = "varchar(30)",
		.sql = R"(
ALTER TABLE `rule_values`
MODIFY COLUMN `rule_value` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL AFTER `rule_name`;
		)"
	},
	ManifestEntry{
		.version = 9267,
		.description = "2024_02_18_group_id_bot_id.sql",
		.check = "SHOW COLUMNS FROM `group_id` LIKE 'bot_id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `group_id`
CHANGE COLUMN `groupid` `group_id` int(11) UNSIGNED NOT NULL DEFAULT 0 FIRST,
CHANGE COLUMN `charid` `character_id` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `group_id`,
CHANGE COLUMN `ismerc` `merc_id` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `name`,
ADD COLUMN `bot_id` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `character_id`,
MODIFY COLUMN `name` varchar(64) NOT NULL DEFAULT '' AFTER `character_id`,
DROP PRIMARY KEY,
ADD PRIMARY KEY (`group_id`, `character_id`, `bot_id`, `merc_id`) USING BTREE;
ALTER TABLE `group_id`
MODIFY COLUMN `character_id` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `name`;
)"
	},
	ManifestEntry{
		.version = 9268,
		.description = "2024_03_23_skill_caps.sql",
		.check = "SHOW COLUMNS FROM `skill_caps` LIKE 'skill_id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `skill_caps`
CHANGE COLUMN `skillID` `skill_id` tinyint(3) UNSIGNED NOT NULL DEFAULT 0 FIRST,
CHANGE COLUMN `class` `class_id` tinyint(3) UNSIGNED NOT NULL DEFAULT 0 AFTER `skill_id`,
ADD COLUMN `id` int(3) UNSIGNED NOT NULL AUTO_INCREMENT FIRST,
DROP PRIMARY KEY,
ADD PRIMARY KEY (`id`) USING BTREE,
ADD INDEX `level_skill_cap`(`skill_id`, `class_id`, `level`, `cap`);
)",
		.content_schema_update = true,
	},
	ManifestEntry{
		.version = 9269,
		.description = "2024_03_27_account_auto_login_charname.sql",
		.check = "SHOW COLUMNS FROM `account` LIKE 'auto_login_charname'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `account`
ADD COLUMN `auto_login_charname` varchar(64) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '' AFTER `charname`;
)"
	},
	ManifestEntry{
		.version = 9270,
		.description = "2024_04_31_content_flagging_lootdrop_entries.sql",
		.check = "SHOW COLUMNS FROM `lootdrop_entries` LIKE 'content_flags'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `lootdrop_entries` ADD `min_expansion` tinyint(4) NOT NULL DEFAULT -1;
ALTER TABLE `lootdrop_entries` ADD `max_expansion` tinyint(4) NOT NULL DEFAULT -1;
ALTER TABLE `lootdrop_entries` ADD `content_flags` varchar(100) NULL;
ALTER TABLE `lootdrop_entries` ADD `content_flags_disabled` varchar(100) NULL;
)",
		.content_schema_update = true
	},
	ManifestEntry{
		.version = 9271,
		.description = "2024_03_10_parcel_implementation.sql",
		.check = "SHOW TABLES LIKE 'character_parcels'",
		.condition = "empty",
		.match = "",
		.sql = R"(CREATE TABLE `character_parcels` (
				`id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
				`char_id` INT(10) UNSIGNED NOT NULL DEFAULT '0',
				`item_id` INT(10) UNSIGNED NOT NULL DEFAULT '0',
				`slot_id` INT(10) UNSIGNED NOT NULL DEFAULT '0',
				`quantity` INT(10) UNSIGNED NOT NULL DEFAULT '0',
				`from_name` VARCHAR(64) NULL DEFAULT NULL COLLATE 'latin1_swedish_ci',
				`note` VARCHAR(1024) NULL DEFAULT NULL COLLATE 'latin1_swedish_ci',
				`sent_date` DATETIME NULL DEFAULT NULL,
				PRIMARY KEY (`id`) USING BTREE,
				UNIQUE INDEX `data_constraint` (`slot_id`, `char_id`) USING BTREE
				)
				COLLATE='latin1_swedish_ci'
				ENGINE=InnoDB
				AUTO_INCREMENT=1;
		)"
	},
	ManifestEntry{
		.version     = 9272,
		.description = "2024_04_23_add_parcel_support_for_augmented_items.sql",
		.check       = "SHOW COLUMNS FROM `character_parcels` LIKE 'aug_slot_1'",
		.condition   = "empty",
		.match       = "",
		.sql         = R"(
ALTER TABLE `character_parcels`
	ADD COLUMN `aug_slot_1` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `item_id`,
	ADD COLUMN `aug_slot_2` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `aug_slot_1`,
	ADD COLUMN `aug_slot_3` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `aug_slot_2`,
	ADD COLUMN `aug_slot_4` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `aug_slot_3`,
	ADD COLUMN `aug_slot_5` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `aug_slot_4`,
	ADD COLUMN `aug_slot_6` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `aug_slot_5`;
)"
	},
	ManifestEntry{
		.version = 9273,
		.description = "2024_04_24_door_close_timer.sql",
		.check = "SHOW COLUMNS FROM `doors` LIKE 'close_timer_ms'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `doors`
ADD COLUMN `close_timer_ms` smallint(8) UNSIGNED NOT NULL DEFAULT 5000 AFTER `is_ldon_door`;
)",
		.content_schema_update = true
	},
	ManifestEntry{
		.version = 9274,
		.description = "2024_05_02_parcel_npc_content.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'is_parcel_merchant'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types`
ADD COLUMN `is_parcel_merchant` TINYINT(1) UNSIGNED NOT NULL DEFAULT '0' AFTER `keeps_sold_items`;
)",
		.content_schema_update = true
	},
	ManifestEntry{
		.version = 9275,
		.description = "2024_04_28_character_extra_haste.sql",
		.check = "SHOW COLUMNS FROM `character_data` LIKE 'extra_haste'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `character_data`
ADD COLUMN `extra_haste` int(11) NOT NULL DEFAULT 0 AFTER `wis`;
)"
	},
	ManifestEntry{
		.version = 9276,
		.description = "2024_05_12_fix_guild_bank_dup_issue.sql",
		.check = "SHOW COLUMNS FROM `guild_bank` WHERE FIELD = 'qty' AND Type LIKE '%unsigned';",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
ALTER TABLE `guild_bank`
	CHANGE COLUMN `qty` `qty` INT(10) NOT NULL DEFAULT '0' AFTER `itemid`;
)"
	},
	ManifestEntry{
		.version = 9277,
		.description = "2024_05_09_parcel_enable_containers.sql",
		.check = "SHOW TABLES LIKE 'character_parcels_containers'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `character_parcels_containers` (
	`id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`parcels_id` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`slot_id` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`item_id` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`aug_slot_1` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`aug_slot_2` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`aug_slot_3` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`aug_slot_4` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`aug_slot_5` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`aug_slot_6` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`quantity` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	PRIMARY KEY (`id`) USING BTREE,
	INDEX `fk_character_parcels_id` (`parcels_id`) USING BTREE,
	CONSTRAINT `fk_character_parcels_id` FOREIGN KEY (`parcels_id`) REFERENCES `character_parcels` (`id`) ON UPDATE NO ACTION ON DELETE CASCADE
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
AUTO_INCREMENT=1
;
)"
	},
	ManifestEntry{
		.version = 9278,
		.description = "2024_05_06_npc_greed.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'greed'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types`
ADD COLUMN `greed` tinyint(8) UNSIGNED NOT NULL DEFAULT 0 AFTER `merchant_id`;
 )",
		.content_schema_update = true
	},
	ManifestEntry{
		.version = 9279,
		.description = "2024_05_13_content_flagging_npc_spells_entries.sql",
		.check = "SHOW COLUMNS FROM `npc_spells_entries` LIKE 'content_flags'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_spells_entries` ADD `min_expansion` tinyint(4) NOT NULL DEFAULT -1;
ALTER TABLE `npc_spells_entries` ADD `max_expansion` tinyint(4) NOT NULL DEFAULT -1;
ALTER TABLE `npc_spells_entries` ADD `content_flags` varchar(100) NULL;
ALTER TABLE `npc_spells_entries` ADD `content_flags_disabled` varchar(100) NULL;
)",
		.content_schema_update = true
	},
	ManifestEntry{
		.version     = 9280,
		.description = "2024_05_11_update_trader_support.sql",
		.check       = "SHOW COLUMNS FROM `trader` LIKE 'aug_slot_1'",
		.condition   = "empty",
		.match       = "",
		.sql         = R"(
ALTER TABLE `trader`
	ADD COLUMN `id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT FIRST,
	CHANGE COLUMN `char_id` `char_id` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `id`,
	CHANGE COLUMN `item_id` `item_id` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `char_id`,
	ADD COLUMN `aug_slot_1` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `item_id`,
	ADD COLUMN `aug_slot_2` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `aug_slot_1`,
	ADD COLUMN `aug_slot_3` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `aug_slot_2`,
	ADD COLUMN `aug_slot_4` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `aug_slot_3`,
	ADD COLUMN `aug_slot_5` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `aug_slot_4`,
	ADD COLUMN `aug_slot_6` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `aug_slot_5`,
	CHANGE COLUMN `serialnumber` `item_sn` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `aug_slot_6`,
	CHANGE COLUMN `charges` `item_charges` INT(11) NOT NULL DEFAULT '0' AFTER `item_sn`,
	ADD COLUMN `char_entity_id` INT(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `slot_id`,
	ADD COLUMN `char_zone_id` INT(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `char_entity_id`,
	ADD COLUMN `active_transaction` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0 AFTER `char_zone_id`,
	DROP PRIMARY KEY,
	ADD PRIMARY KEY (`id`),
	ADD INDEX `charid_slotid` (`char_id`, `slot_id`);
)"
	},
	ManifestEntry{
		.version     = 9281,
		.description = "2024_06_24_update_buyer_support.sql",
		.check       = "SHOW COLUMNS FROM `buyer` LIKE 'id'",
		.condition   = "empty",
		.match       = "",
		.sql         = R"(
ALTER TABLE `buyer`
	ADD COLUMN `id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT FIRST,
	CHANGE COLUMN `charid` `char_id` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `id`,
	ADD COLUMN `char_entity_id` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `char_id`,
	ADD COLUMN `char_name` VARCHAR(64) NULL DEFAULT NULL AFTER `char_entity_id`,
	ADD COLUMN `char_zone_id` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `char_name`,
	ADD COLUMN `char_zone_instance_id` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `char_zone_id`,
	ADD COLUMN `transaction_date` DATETIME NULL DEFAULT NULL AFTER `char_zone_instance_id`,
	ADD COLUMN `welcome_message` VARCHAR(256) NULL DEFAULT NULL AFTER `transaction_date`,
	DROP COLUMN `buyslot`,
	DROP COLUMN `itemid`,
	DROP COLUMN `itemname`,
	DROP COLUMN `quantity`,
	DROP COLUMN `price`,
	DROP PRIMARY KEY,
	ADD PRIMARY KEY (`id`) USING BTREE,
	ADD INDEX `charid` (`char_id`);

CREATE TABLE `buyer_buy_lines` (
	`id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
	`buyer_id` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
	`char_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`buy_slot_id` INT(11) NOT NULL DEFAULT '0',
	`item_id` INT(11) NOT NULL DEFAULT '0',
	`item_qty` INT(11) NOT NULL DEFAULT '0',
	`item_price` INT(11) NOT NULL DEFAULT '0',
	`item_icon` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`item_name` VARCHAR(64) NOT NULL DEFAULT '' COLLATE 'latin1_swedish_ci',
	PRIMARY KEY (`id`) USING BTREE,
	INDEX `buyerid_charid_buyslotid` (`buyer_id`, `char_id`, `buy_slot_id`) USING BTREE
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
AUTO_INCREMENT=1;

CREATE TABLE `buyer_trade_items` (
	`id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
	`buyer_buy_lines_id` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
	`item_id` INT(11) NOT NULL DEFAULT '0',
	`item_qty` INT(11) NOT NULL DEFAULT '0',
	`item_icon` INT(11) NOT NULL DEFAULT '0',
	`item_name` VARCHAR(64) NOT NULL DEFAULT '0' COLLATE 'latin1_swedish_ci',
	PRIMARY KEY (`id`) USING BTREE,
	INDEX `buyerbuylinesid` (`buyer_buy_lines_id`) USING BTREE
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
AUTO_INCREMENT=1;
)"
	},
	ManifestEntry{
		.version = 9282,
		.description = "2024_08_02_spell_buckets_comparison.sql",
		.check = "SHOW COLUMNS FROM `spell_buckets` LIKE 'bucket_comparison'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `spell_buckets`
CHANGE COLUMN `spellid` `spell_id` int UNSIGNED NOT NULL FIRST,
CHANGE COLUMN `key` `bucket_name` varchar(100) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '' AFTER `spell_id`,
CHANGE COLUMN `value` `bucket_value` varchar(100) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '' AFTER `bucket_name`,
ADD COLUMN `bucket_comparison` tinyint UNSIGNED NOT NULL DEFAULT 0 AFTER `bucket_value`,
DROP PRIMARY KEY,
ADD PRIMARY KEY (`spell_id`) USING BTREE;
)"
	},
	ManifestEntry{
		.version     = 9283,
		.description = "2024_08_05_fix_client_hotbar",
		.check       = "SHOW COLUMNS FROM `inventory` LIKE 'guid'",
		.condition   = "empty",
		.match       = "",
		.sql         = R"(
ALTER TABLE `inventory`
	ADD COLUMN `guid` BIGINT UNSIGNED NULL DEFAULT '0' AFTER `ornament_hero_model`;
ALTER TABLE `inventory_snapshots`
	ADD COLUMN `guid` BIGINT UNSIGNED NULL DEFAULT '0' AFTER `ornament_hero_model`;
)"
	},
	ManifestEntry{
		.version = 9284,
		.description = "2024_10_08_character_exp_modifiers_default.sql",
		.check = "SHOW CREATE TABLE `character_exp_modifiers`",
		.condition = "contains",
		.match = "`exp_modifier` float NOT NULL,",
		.sql = R"(
ALTER TABLE `character_exp_modifiers`
MODIFY COLUMN `aa_modifier` float NOT NULL DEFAULT 1.0 AFTER `instance_version`,
MODIFY COLUMN `exp_modifier` float NOT NULL DEFAULT 1.0 AFTER `aa_modifier`;
)"
	},
	ManifestEntry{
		.version = 9285,
		.description = "2024_11_08_data_buckets_indexes.sql",
		.check = "SHOW CREATE TABLE `data_buckets`",
		.condition = "missing",
		.match = "idx_character_expires",
		.sql = R"(
CREATE INDEX idx_character_expires ON data_buckets (character_id, expires);
CREATE INDEX idx_npc_expires ON data_buckets (npc_id, expires);
CREATE INDEX idx_bot_expires ON data_buckets (bot_id, expires);
)"
	},
	ManifestEntry{
		.version = 9286,
		.description = "2024_11_26_bazaar_find_trader.sql",
		.check       = "SHOW COLUMNS FROM `trader` LIKE 'char_zone_instance_id'",
		.condition   = "empty",
		.match       = "",
		.sql         = R"(
ALTER TABLE `trader`
	ADD COLUMN `char_zone_instance_id` INT NULL DEFAULT '0' AFTER `char_zone_id`;
)"
	},
	ManifestEntry{
		.version = 9287,
		.description = "2024_11_26_bazaar_find_trader.sql",
		.check       = "SHOW COLUMNS FROM `npc_types` LIKE 'walkspeed'",
		.condition   = "missing",
		.match       = "float",
		.sql         = R"(
ALTER TABLE `npc_types` MODIFY COLUMN `walkspeed` float NOT NULL DEFAULT 0;
)",
		.content_schema_update = true
	},
	ManifestEntry{
		.version = 9288,
		.description = "2024_11_10_zone_player_partitioning.sql",
		.check = "SHOW CREATE TABLE `zone`",
		.condition = "missing",
		.match = "shard_at_player_count",
		.sql = R"(
ALTER TABLE `zone`
ADD COLUMN `shard_at_player_count` int(11) NULL DEFAULT 0 AFTER `seconds_before_idle`;
)",
		.content_schema_update = true
	},
	ManifestEntry{
		.version     = 9289,
		.description = "2025_01_19_evolving_items__character_evolving_items",
		.check       = "SHOW TABLES LIKE 'character_evolving_items'",
		.condition   = "empty",
		.match       = "",
		.sql         = R"(
CREATE TABLE `character_evolving_items` (
	`id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
	`character_id` INT(10) UNSIGNED NULL DEFAULT '0',
	`item_id` INT(10) UNSIGNED NULL DEFAULT '0',
	`activated` TINYINT(1) UNSIGNED NULL DEFAULT '0',
	`equipped` TINYINT(3) UNSIGNED NULL DEFAULT '0',
	`current_amount` BIGINT(20) NULL DEFAULT '0',
	`progression` DOUBLE(22,0) NULL DEFAULT '0',
	`final_item_id` INT(10) UNSIGNED NULL DEFAULT '0',
	`deleted_at` DATETIME NULL DEFAULT NULL,
	PRIMARY KEY (`id`) USING BTREE
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
AUTO_INCREMENT=1
;
)"
	},
	ManifestEntry{
		.version     = 9290,
		.description = "2025_01_19_evolving_items__items_evolving_details",
		.check       = "SHOW TABLES LIKE 'items_evolving_details'",
		.condition   = "empty",
		.match       = "",
		.sql         = R"(
CREATE TABLE `items_evolving_details` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `item_evo_id` int(10) unsigned DEFAULT 0 COMMENT 'items.evoid',
  `item_evolve_level` int(10) unsigned DEFAULT 0 COMMENT 'items.evolvinglevel',
  `item_id` int(10) unsigned DEFAULT 0 COMMENT 'items.id',
  `type` int(10) unsigned DEFAULT 0,
  `sub_type` int(10) unsigned DEFAULT 0,
  `required_amount` bigint(20) DEFAULT NULL,
  PRIMARY KEY (`id`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;
INSERT INTO `items_evolving_details` VALUES
(1,1000,1,86900,99,1,0),
(2,1000,2,86901,99,1,0),
(3,1000,3,86902,99,1,0),
(4,1000,4,86903,99,1,0),
(5,1000,5,86904,99,1,0),
(6,1001,1,86910,99,1,0),
(7,1001,2,86911,99,1,0),
(8,1001,3,86912,99,1,0),
(9,1001,4,86913,99,1,0),
(10,1001,5,86914,99,1,0),
(11,1001,6,86915,99,1,0),
(12,1001,7,86916,99,1,0),
(13,1002,1,86920,99,1,0),
(14,1002,2,86921,99,1,0),
(15,1002,3,86922,99,1,0),
(16,1002,4,86923,99,1,0),
(17,1002,5,86924,99,1,0),
(18,1002,6,86925,99,1,0),
(19,1002,7,86926,99,1,0),
(20,1003,1,86930,99,1,0),
(21,1003,2,86931,99,1,0),
(22,1003,3,86932,99,1,0),
(23,1003,4,86933,99,1,0),
(24,1003,5,86934,99,1,0),
(25,1003,6,86935,99,1,0),
(26,1004,1,86940,99,1,0),
(27,1004,2,86941,99,1,0),
(28,1004,3,86942,99,1,0),
(29,1004,4,86943,99,1,0),
(30,1004,5,86944,99,1,0),
(31,1004,6,86945,99,1,0),
(32,1005,1,86950,99,1,0),
(33,1005,2,86951,99,1,0),
(34,1005,3,86952,99,1,0),
(35,1005,4,86953,99,1,0),
(36,1005,5,86954,99,1,0),
(37,1005,6,86955,99,1,0),
(38,1005,7,86956,99,1,0),
(39,1006,1,86960,99,1,0),
(40,1006,2,86961,99,1,0),
(41,1006,3,86962,99,1,0),
(42,1006,4,86963,99,1,0),
(43,1006,5,86964,99,1,0),
(44,1006,6,86965,99,1,0),
(45,1006,7,86966,99,1,0),
(46,1007,1,86970,99,1,0),
(47,1007,2,86971,99,1,0),
(48,1007,3,86972,99,1,0),
(49,1007,4,86973,99,1,0),
(50,1007,5,86974,99,1,0),
(51,1007,6,86975,99,1,0),
(52,1007,7,86976,99,1,0),
(53,1008,1,86980,99,1,0),
(54,1008,2,86981,99,1,0),
(55,1008,3,86982,99,1,0),
(56,1008,4,86983,99,1,0),
(57,1008,5,86984,99,1,0),
(58,1009,1,86990,99,1,0),
(59,1009,2,86991,99,1,0),
(60,1009,3,86992,99,1,0),
(61,1009,4,86993,99,1,0),
(62,1009,5,86994,99,1,0),
(63,1009,6,86995,99,1,0),
(64,1009,7,86996,99,1,0),
(65,1009,8,86997,99,1,0),
(66,1009,9,86998,99,1,0),
(67,1009,10,86999,99,1,0),
(68,1010,1,90001,99,1,0),
(69,1010,2,90002,99,1,0),
(70,1010,3,90003,99,1,0),
(71,1010,6,90006,99,1,0),
(72,1010,7,90007,99,1,0),
(73,1010,8,90008,99,1,0),
(74,1010,10,90010,99,1,0),
(75,1050,1,89270,99,1,0),
(76,1050,2,89271,99,1,0),
(77,1050,3,89272,99,1,0),
(78,1050,4,89273,99,1,0),
(79,1050,5,89274,99,1,0),
(80,1050,6,89275,99,1,0),
(81,1051,1,89280,99,1,0),
(82,1051,2,89281,99,1,0),
(83,1051,3,89282,99,1,0),
(84,1051,4,89283,99,1,0),
(85,1051,5,89284,99,1,0),
(86,1052,1,89290,99,1,0),
(87,1052,2,89291,99,1,0),
(88,1052,3,89292,99,1,0),
(89,1052,4,89293,99,1,0),
(90,1052,5,89294,99,1,0),
(91,1052,6,89295,99,1,0),
(92,1052,7,89296,99,1,0),
(93,1053,1,89300,99,1,0),
(94,1053,2,89301,99,1,0),
(95,1053,3,89302,99,1,0),
(96,1053,4,89303,99,1,0),
(97,1053,5,89304,99,1,0),
(98,1053,6,89305,99,1,0),
(99,1053,7,89306,99,1,0),
(100,1053,8,89307,99,1,0),
(101,1053,9,89308,99,1,0),
(102,1054,1,89310,99,1,0),
(103,1054,2,89311,99,1,0),
(104,1054,3,89312,99,1,0),
(105,1054,4,89313,99,1,0),
(106,1054,5,89314,99,1,0),
(107,1054,6,89315,99,1,0),
(108,1055,1,89320,99,1,0),
(109,1055,2,89321,99,1,0),
(110,1055,3,89322,99,1,0),
(111,1055,4,89323,99,1,0),
(112,1055,5,89324,99,1,0),
(113,1055,6,89325,99,1,0),
(114,1055,7,89326,99,1,0),
(115,1056,1,89330,99,1,0),
(116,1056,2,89331,99,1,0),
(117,1056,3,89332,99,1,0),
(118,1056,4,89333,99,1,0),
(119,1056,5,89334,99,1,0),
(120,1057,1,89340,99,1,0),
(121,1057,2,89341,99,1,0),
(122,1057,3,89342,99,1,0),
(123,1057,4,89343,99,1,0),
(124,1057,5,89344,99,1,0),
(125,1057,6,89345,99,1,0),
(126,1057,7,89346,99,1,0),
(127,1057,8,89347,99,1,0),
(128,1058,1,89350,99,1,0),
(129,1058,2,89351,99,1,0),
(130,1058,3,89352,99,1,0),
(131,1058,4,89353,99,1,0),
(132,1058,5,89354,99,1,0),
(133,1058,6,89355,99,1,0),
(134,1058,7,89356,99,1,0),
(135,1059,1,89360,99,1,0),
(136,1059,2,89361,99,1,0),
(137,1059,3,89362,99,1,0),
(138,1060,1,89490,99,1,0),
(139,1060,2,89491,99,1,0),
(140,1060,3,89492,99,1,0),
(141,1061,1,89500,99,1,0),
(142,1061,2,89501,99,1,0),
(143,1061,3,89502,99,1,0),
(144,1062,1,89510,99,1,0),
(145,1062,2,89511,99,1,0),
(146,1062,3,89512,99,1,0),
(147,1063,1,89520,99,1,0),
(148,1063,2,89521,99,1,0),
(149,1063,3,89522,99,1,0),
(150,1064,1,89530,99,1,0),
(151,1064,2,89531,99,1,0),
(152,1064,3,89532,99,1,0),
(153,1065,1,89540,99,1,0),
(154,1065,2,89541,99,1,0),
(155,1065,3,89542,99,1,0),
(156,1066,1,89550,3,274,500),
(157,1066,2,89551,3,274,1000),
(158,1066,3,89552,3,274,2000),
(159,1067,1,89560,99,1,0),
(160,1067,2,89561,99,1,0),
(161,1067,3,89562,99,1,0),
(162,1069,1,85571,99,1,0),
(163,1069,2,85572,99,1,0),
(164,1069,3,85573,99,1,0),
(165,1200,1,95001,99,1,0),
(166,1200,2,95002,99,1,0),
(167,1200,3,95003,99,1,0),
(168,1200,4,95004,99,1,0),
(169,1200,5,95005,99,1,0),
(170,1200,6,95006,99,1,0),
(171,1200,7,95007,99,1,0),
(172,1201,1,95008,99,1,0),
(173,1201,2,95009,99,1,0),
(174,1201,3,95010,99,1,0),
(175,1201,4,95011,99,1,0),
(176,1201,5,95012,99,1,0),
(177,1201,6,95013,99,1,0),
(178,1201,7,95014,99,1,0),
(179,1202,1,95015,99,1,0),
(180,1202,2,95016,99,1,0),
(181,1202,3,95017,99,1,0),
(182,1202,4,95018,99,1,0),
(183,1202,5,95019,99,1,0),
(184,1202,6,95020,99,1,0),
(185,1202,7,95021,99,1,0),
(186,1203,1,95022,99,1,0),
(187,1203,2,95023,99,1,0),
(188,1203,3,95024,99,1,0),
(189,1203,4,95025,99,1,0),
(190,1203,5,95026,99,1,0),
(191,1203,6,95027,99,1,0),
(192,1203,7,95028,99,1,0),
(193,1204,1,95029,99,1,0),
(194,1204,2,95030,99,1,0),
(195,1204,3,95031,99,1,0),
(196,1204,4,95032,99,1,0),
(197,1204,5,95033,99,1,0),
(198,1204,6,95034,99,1,0),
(199,1204,7,95035,99,1,0),
(200,1205,1,95036,99,1,0),
(201,1205,2,95037,99,1,0),
(202,1205,3,95038,99,1,0),
(203,1205,4,95039,99,1,0),
(204,1205,5,95040,99,1,0),
(205,1205,6,95041,99,1,0),
(206,1205,7,95042,99,1,0),
(207,1206,1,95043,99,1,0),
(208,1206,2,95044,99,1,0),
(209,1206,3,95045,99,1,0),
(210,1206,4,95046,99,1,0),
(211,1206,5,95047,99,1,0),
(212,1206,6,95048,99,1,0),
(213,1206,7,95049,99,1,0),
(214,1207,1,95050,99,1,0),
(215,1207,2,95051,99,1,0),
(216,1207,3,95052,99,1,0),
(217,1207,4,95053,99,1,0),
(218,1207,5,95054,99,1,0),
(219,1207,6,95055,99,1,0),
(220,1207,7,95056,99,1,0),
(221,1208,1,95057,99,1,0),
(222,1208,2,95058,99,1,0),
(223,1208,3,95059,99,1,0),
(224,1208,4,95060,99,1,0),
(225,1208,5,95061,99,1,0),
(226,1208,6,95062,99,1,0),
(227,1208,7,95063,99,1,0),
(228,1209,1,95064,99,1,0),
(229,1209,2,95065,99,1,0),
(230,1209,3,95066,99,1,0),
(231,1209,4,95067,99,1,0),
(232,1209,5,95068,99,1,0),
(233,1209,6,95069,99,1,0),
(234,1209,7,95070,99,1,0),
(235,1210,1,95071,99,1,0),
(236,1210,2,95072,99,1,0),
(237,1210,3,95073,99,1,0),
(238,1210,4,95074,99,1,0),
(239,1210,5,95075,99,1,0),
(240,1210,6,95076,99,1,0),
(241,1210,7,95077,99,1,0),
(242,1211,1,85612,1,1,100000),
(243,1211,2,85613,1,1,200000),
(244,1211,3,85614,1,1,300000),
(245,1214,1,80035,99,1,0),
(246,1301,1,102700,99,1,0),
(247,1301,4,102703,99,1,0),
(248,1302,1,102704,99,1,0),
(249,1302,2,102705,99,1,0),
(250,1303,1,102706,99,1,0),
(251,1303,2,102707,99,1,0),
(252,1303,3,102708,99,1,0),
(253,1304,1,102709,99,1,0),
(254,1304,5,102713,99,1,0),
(255,1305,1,102714,99,1,0),
(256,1306,1,102716,99,1,0),
(257,1306,5,102720,99,1,0),
(258,1307,1,102721,99,1,0),
(259,1307,3,102723,99,1,0),
(260,1308,1,102724,99,1,0),
(261,1309,1,102727,99,1,0),
(262,1309,2,102728,99,1,0),
(263,1310,1,102729,99,1,0),
(264,1310,3,102731,99,1,0),
(265,1311,1,102732,99,1,0),
(266,1311,4,102735,99,1,0),
(267,1312,1,102736,99,1,0),
(268,1312,3,102738,99,1,0),
(269,1313,1,102739,99,1,0),
(270,1314,1,102743,99,1,0),
(271,1314,2,102744,99,1,0),
(272,1314,3,102745,99,1,0),
(273,1315,1,102746,99,1,0),
(274,1315,2,102747,99,1,0),
(275,1316,1,102748,99,1,0),
(276,1316,5,102752,99,1,0),
(277,1317,1,102753,99,1,0),
(278,1318,1,102756,99,1,0),
(279,1319,1,102759,99,1,0),
(280,1319,3,102761,99,1,0),
(281,1320,1,102762,99,1,0),
(282,1321,1,102765,99,1,0),
(283,1321,2,102766,99,1,0),
(284,1321,3,102767,99,1,0),
(285,1322,1,102768,99,1,0),
(286,1322,2,102769,99,1,0),
(287,1322,3,102770,99,1,0),
(288,1323,1,102771,99,1,0),
(289,1324,1,102774,99,1,0),
(290,1400,1,102800,99,1,0),
(291,1401,1,102807,99,1,0),
(292,1401,7,102813,99,1,0),
(293,1402,1,102814,99,1,0),
(294,1402,7,102820,99,1,0),
(295,1403,1,102821,99,1,0),
(296,1404,1,102828,99,1,0),
(297,1405,1,102835,99,1,0),
(298,1406,1,102842,99,1,0),
(299,1408,1,109310,99,1,0),
(300,1408,5,109314,99,1,0),
(301,1409,1,109315,99,1,0),
(302,1409,5,109319,99,1,0),
(303,1410,1,109320,99,1,0),
(304,1410,5,109324,99,1,0),
(305,1411,1,109325,99,1,0),
(306,1411,5,109329,99,1,0),
(307,1412,1,109330,99,1,0),
(308,1412,5,109334,99,1,0),
(309,1413,1,109335,99,1,0),
(310,1413,5,109339,99,1,0),
(311,1414,1,109340,99,1,0),
(312,1414,5,109344,99,1,0),
(313,1415,1,109345,99,1,0),
(314,1415,5,109349,99,1,0),
(315,1416,1,109350,99,1,0),
(316,1416,2,109351,99,1,0),
(317,1416,5,109354,99,1,0),
(318,1417,1,109355,99,1,0),
(319,1417,5,109359,99,1,0),
(320,1418,1,109360,99,1,0),
(321,1418,5,109364,99,1,0),
(322,1419,1,109365,99,1,0),
(323,1419,3,109367,99,1,0),
(324,1419,5,109369,99,1,0),
(325,1420,1,109370,99,1,0),
(326,1420,5,109374,99,1,0),
(327,1421,1,109375,99,1,0),
(328,1421,5,109379,99,1,0),
(329,1422,1,109380,99,1,0),
(330,1422,5,109384,99,1,0),
(331,1423,1,109385,99,1,0),
(332,1423,2,109386,99,1,0),
(333,1423,5,109389,99,1,0),
(334,1436,1,120378,99,1,0),
(335,1436,2,120379,99,1,0),
(336,1436,3,120380,99,1,0),
(337,1436,4,120381,99,1,0),
(338,1436,5,120382,99,1,0),
(339,1436,6,120383,99,1,0),
(340,1436,7,120384,99,1,0),
(341,1436,8,120385,99,1,0),
(342,1436,9,120386,99,1,0),
(343,1436,10,120387,99,1,0),
(344,1436,11,120388,99,1,0),
(345,1436,12,120389,99,1,0),
(346,1436,13,120390,99,1,0),
(347,1436,14,120391,99,1,0),
(348,1436,15,120392,99,1,0),
(349,1436,16,120393,99,1,0),
(350,1436,17,120394,99,1,0),
(351,1436,18,120395,99,1,0),
(352,1436,19,120396,99,1,0),
(353,1436,20,120397,99,1,0),
(354,1440,1,56992,99,1,0),
(355,1440,2,56993,99,1,0),
(356,1440,3,56994,99,1,0),
(357,1440,4,56995,99,1,0),
(358,1440,5,56996,99,1,0),
(359,1441,1,132787,99,1,0),
(360,1441,2,132788,99,1,0),
(361,1441,3,132789,99,1,0),
(362,1441,4,132790,99,1,0),
(363,1441,5,132791,99,1,0),
(364,1441,6,132792,99,1,0),
(365,1441,7,132793,99,1,0),
(366,1441,8,132794,99,1,0),
(367,1441,9,132795,99,1,0),
(368,1441,10,132796,99,1,0),
(369,1441,11,132797,99,1,0),
(370,1441,13,132799,99,1,0),
(371,1441,14,132800,99,1,0),
(372,1441,15,132801,99,1,0),
(373,1441,16,132802,99,1,0),
(374,1441,17,132803,99,1,0),
(375,1441,18,132804,99,1,0),
(376,1441,19,132805,99,1,0),
(377,1441,20,132806,99,1,0),
(378,1442,1,133137,99,1,0),
(379,1442,2,133138,99,1,0),
(380,1442,3,133139,99,1,0),
(381,1442,4,133140,99,1,0),
(382,1442,10,133146,99,1,0),
(383,1442,11,133147,99,1,0),
(384,1442,12,133148,99,1,0),
(385,1442,13,133149,99,1,0),
(386,1442,14,133150,99,1,0),
(387,1442,15,133151,99,1,0),
(388,1442,16,133152,99,1,0),
(389,1442,17,133153,99,1,0),
(390,1442,18,133154,99,1,0),
(391,1442,19,133155,99,1,0),
(392,1442,20,133156,99,1,0),
(393,1443,1,133406,99,1,0),
(394,1443,2,133407,99,1,0),
(395,1443,3,133408,99,1,0),
(396,1443,4,133409,99,1,0),
(397,1443,5,133410,99,1,0),
(398,1443,6,133411,99,1,0),
(399,1443,7,133412,99,1,0),
(400,1443,8,133413,99,1,0),
(401,1443,9,133414,99,1,0),
(402,1443,10,133415,99,1,0),
(403,1443,11,133416,99,1,0),
(404,1443,12,133417,99,1,0),
(405,1443,13,133418,99,1,0),
(406,1443,14,133419,99,1,0),
(407,1443,15,133420,99,1,0),
(408,1443,16,133421,99,1,0),
(409,1443,17,133422,99,1,0),
(410,1443,18,133423,99,1,0),
(411,1443,19,133424,99,1,0),
(412,1443,20,133425,99,1,0),
(413,1444,1,94938,99,1,0),
(414,1444,2,94939,99,1,0),
(415,1444,3,94940,99,1,0),
(416,1444,4,94941,99,1,0),
(417,1444,5,94942,99,1,0),
(418,1444,6,94943,99,1,0),
(419,1444,7,94944,99,1,0),
(420,1444,8,94945,99,1,0),
(421,1444,9,94946,99,1,0),
(422,1444,10,94947,99,1,0),
(423,1444,11,94948,99,1,0),
(424,1444,12,94949,99,1,0),
(425,1444,13,94950,99,1,0),
(426,1444,14,94951,99,1,0),
(427,1444,15,94952,99,1,0),
(428,1444,16,94953,99,1,0),
(429,1444,17,94954,99,1,0),
(430,1444,18,94955,99,1,0),
(431,1444,19,94956,99,1,0),
(432,1444,20,94957,99,1,0),
(433,1445,1,98858,99,1,0),
(434,1445,2,98859,99,1,0),
(435,1445,3,98860,99,1,0),
(436,1445,4,98861,99,1,0),
(437,1445,5,98862,99,1,0);

)",
		.content_schema_update = true
	},
	ManifestEntry{
		.version = 9291,
		.description = "2025_01_21_add_remove_zone_fields",
		.check = "SHOW COLUMNS FROM `zone` LIKE 'client_update_range'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE zone DROP COLUMN IF EXISTS npc_update_range;
ALTER TABLE zone DROP COLUMN IF EXISTS max_movement_update_range;
ALTER TABLE `zone` ADD COLUMN `client_update_range` int(11) NOT NULL DEFAULT 600 AFTER `npc_max_aggro_dist`;
)",
		.content_schema_update = true,
	},
	ManifestEntry{
		.version = 9292,
		.description = "2025_01_21_data_buckets_account_id",
		.check       = "SHOW COLUMNS FROM `data_buckets` LIKE 'account_id'",
		.condition   = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `data_buckets`
ADD COLUMN `account_id` bigint(11) NULL DEFAULT 0 AFTER `expires`,
DROP INDEX `keys`,
ADD UNIQUE INDEX `keys` (`key`, `character_id`, `npc_id`, `bot_id`, `account_id`) USING BTREE;

-- Add the INDEX for character_id and key
ALTER TABLE `data_buckets` ADD KEY `idx_account_id_key` (`account_id`, `key`);
)",
		.content_schema_update = false
	},
	ManifestEntry{
		.version = 9293,
		.description = "2025_01_10_create_pet_names_table.sql",
		.check = "SHOW TABLES LIKE 'character_pet_name'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `character_pet_name` (
    `character_id` INT(11) NOT NULL PRIMARY KEY,
    `name` VARCHAR(64) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
)",
	},
	ManifestEntry{
		.version = 9294,
		.description = "2025_01_26_items_table_bazaar_search_indexes.sql",
		.check = "SHOW CREATE TABLE `items`",
		.condition = "missing",
		.match = "idx_slots_reclevel",
		.sql = R"(
-- indexes for the `items` table
CREATE INDEX idx_slots_reclevel ON items (slots, reclevel);
CREATE INDEX idx_itemclass_itemtype ON items (itemclass, itemtype);
CREATE INDEX idx_augment_slots ON items (
    augslot1type,
    augslot2type,
    augslot3type,
    augslot4type,
    augslot5type,
    augslot6type
);
CREATE INDEX idx_races_classes ON items (races, classes);

-- common stat fields
CREATE INDEX idx_item_ac ON items (ac);
CREATE INDEX idx_item_hp ON items (hp);
CREATE INDEX idx_item_mana ON items (mana);
CREATE INDEX idx_item_reclevel ON items (reclevel);
CREATE INDEX idx_item_type_skill ON items (itemtype, skillmodtype);
)",
		.content_schema_update = true
	},
	ManifestEntry{
		.version = 9295,
		.description = "2025_01_26_trader_table_bazaar_search_indexes.sql",
		.check = "SHOW CREATE TABLE `trader`",
		.condition = "missing",
		.match = "idx_trader_item",
		.sql = R"(
-- indexes for the `trader` table
CREATE INDEX idx_trader_item ON trader (item_id, item_cost);
CREATE INDEX idx_trader_char ON trader (char_id, char_zone_id, char_zone_instance_id);
CREATE INDEX idx_trader_item_sn ON trader (item_sn);
CREATE INDEX idx_trader_item_cost ON trader (item_cost);
CREATE INDEX idx_trader_active_transaction ON trader (active_transaction);
)",
		.content_schema_update = false
	},
	ManifestEntry{
		.version = 9296,
		.description = "2025_02_01_trader_table_listing_date.sql",
		.check = "SHOW CREATE TABLE `trader`",
		.condition = "missing",
		.match = "listing_date",
		.sql = R"(
ALTER TABLE `trader`
	ADD COLUMN `listing_date` DATETIME NULL DEFAULT NULL AFTER `active_transaction`,
	ADD INDEX `idx_trader_listing_date` (`listing_date`);
)",
		.content_schema_update = false
	},
	ManifestEntry{
		.version = 9297,
		.description = "2024_01_22_sharedbank_guid_primary_key.sql",
		.check = "SHOW COLUMNS FROM `sharedbank` LIKE 'guid'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `sharedbank`
CHANGE COLUMN `acctid` `account_id` int(11) UNSIGNED NOT NULL DEFAULT 0 FIRST,
CHANGE COLUMN `slotid` `slot_id` mediumint(7) UNSIGNED NOT NULL DEFAULT 0 AFTER `account_id`,
CHANGE COLUMN `itemid` `item_id` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `slot_id`,
CHANGE COLUMN `augslot1` `augment_one` mediumint(7) UNSIGNED NOT NULL DEFAULT 0 AFTER `charges`,
CHANGE COLUMN `augslot2` `augment_two` mediumint(7) UNSIGNED NOT NULL DEFAULT 0 AFTER `augment_one`,
CHANGE COLUMN `augslot3` `augment_three` mediumint(7) UNSIGNED NOT NULL DEFAULT 0 AFTER `augment_two`,
CHANGE COLUMN `augslot4` `augment_four` mediumint(7) UNSIGNED NOT NULL DEFAULT 0 AFTER `augment_three`,
CHANGE COLUMN `augslot5` `augment_five` mediumint(7) UNSIGNED NOT NULL DEFAULT 0 AFTER `augment_four`,
CHANGE COLUMN `augslot6` `augment_six` mediumint(7) UNSIGNED NOT NULL DEFAULT 0 AFTER `augment_five`,
MODIFY COLUMN `charges` smallint(3) UNSIGNED NOT NULL DEFAULT 0 AFTER `item_id`,
ADD COLUMN `color` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `charges`,
ADD COLUMN `ornament_icon` int(11) UNSIGNED NOT NULL AFTER `custom_data`,
ADD COLUMN `ornament_idfile` int(11) UNSIGNED NOT NULL AFTER `ornament_icon`,
ADD COLUMN `ornament_hero_model` int(11) NOT NULL AFTER `ornament_idfile`,
ADD COLUMN `guid` bigint(20) UNSIGNED NOT NULL DEFAULT 0 AFTER `ornament_hero_model`,
ADD PRIMARY KEY (`account_id`, `slot_id`);
)",
		.content_schema_update = false,
		.force_interactive = false
	},
	ManifestEntry{
		.version = 9298,
		.description = "2024_10_24_inventory_changes.sql",
		.check = "SHOW COLUMNS FROM `inventory` LIKE 'character_id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `inventory`
CHANGE COLUMN `charid` `character_id` int(11) UNSIGNED NOT NULL DEFAULT 0 FIRST,
CHANGE COLUMN `slotid` `slot_id` mediumint(7) UNSIGNED NOT NULL DEFAULT 0 AFTER `character_id`,
CHANGE COLUMN `itemid` `item_id` int(11) UNSIGNED NULL DEFAULT 0 AFTER `slot_id`,
CHANGE COLUMN `augslot1` `augment_one` mediumint(7) UNSIGNED NOT NULL DEFAULT 0 AFTER `color`,
CHANGE COLUMN `augslot2` `augment_two` mediumint(7) UNSIGNED NOT NULL DEFAULT 0 AFTER `augment_one`,
CHANGE COLUMN `augslot3` `augment_three` mediumint(7) UNSIGNED NOT NULL DEFAULT 0 AFTER `augment_two`,
CHANGE COLUMN `augslot4` `augment_four` mediumint(7) UNSIGNED NOT NULL DEFAULT 0 AFTER `augment_three`,
CHANGE COLUMN `augslot5` `augment_five` mediumint(7) UNSIGNED NOT NULL DEFAULT 0 AFTER `augment_four`,
CHANGE COLUMN `augslot6` `augment_six` mediumint(7) UNSIGNED NOT NULL DEFAULT 0 AFTER `augment_five`,
CHANGE COLUMN `ornamenticon` `ornament_icon` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `custom_data`,
CHANGE COLUMN `ornamentidfile` `ornament_idfile` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `ornament_icon`,
DROP PRIMARY KEY,
ADD PRIMARY KEY (`character_id`, `slot_id`) USING BTREE;
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 251) + 4010) WHERE `slot_id` BETWEEN 251 AND 260; -- Bag 1
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 261) + 4210) WHERE `slot_id` BETWEEN 261 AND 270; -- Bag 2
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 271) + 4410) WHERE `slot_id` BETWEEN 271 AND 280; -- Bag 3
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 281) + 4610) WHERE `slot_id` BETWEEN 281 AND 290; -- Bag 4
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 291) + 4810) WHERE `slot_id` BETWEEN 291 AND 300; -- Bag 5
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 301) + 5010) WHERE `slot_id` BETWEEN 301 AND 310; -- Bag 6
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 311) + 5210) WHERE `slot_id` BETWEEN 311 AND 320; -- Bag 7
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 321) + 5410) WHERE `slot_id` BETWEEN 321 AND 330; -- Bag 8
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 331) + 5610) WHERE `slot_id` BETWEEN 331 AND 340; -- Bag 9
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 341) + 5810) WHERE `slot_id` BETWEEN 341 AND 350; -- Bag 10
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 8000) + 6010) WHERE `slot_id` BETWEEN 8000 AND 8199; -- Cursor Overflow
DELETE FROM `inventory` WHERE `slot_id` BETWEEN 8200 AND 8999; -- Extreme Cursor Overflow
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 351) + 6010) WHERE `slot_id` BETWEEN 351 AND 360; -- Cursor Bag
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 2031) + 6210) WHERE `slot_id` BETWEEN 2031 AND 2040; -- Bank Bag 1
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 2041) + 6410) WHERE `slot_id` BETWEEN 2041 AND 2050; -- Bank Bag 2
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 2051) + 6610) WHERE `slot_id` BETWEEN 2051 AND 2060; -- Bank Bag 3
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 2061) + 6810) WHERE `slot_id` BETWEEN 2061 AND 2070; -- Bank Bag 4
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 2071) + 7010) WHERE `slot_id` BETWEEN 2071 AND 2080; -- Bank Bag 5
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 2081) + 7210) WHERE `slot_id` BETWEEN 2081 AND 2090; -- Bank Bag 6
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 2091) + 7410) WHERE `slot_id` BETWEEN 2091 AND 2100; -- Bank Bag 7
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 2101) + 7610) WHERE `slot_id` BETWEEN 2101 AND 2110; -- Bank Bag 8
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 2111) + 7810) WHERE `slot_id` BETWEEN 2111 AND 2120; -- Bank Bag 9
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 2121) + 8010) WHERE `slot_id` BETWEEN 2121 AND 2130; -- Bank Bag 10
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 2131) + 8210) WHERE `slot_id` BETWEEN 2131 AND 2140; -- Bank Bag 11
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 2141) + 8410) WHERE `slot_id` BETWEEN 2141 AND 2150; -- Bank Bag 12
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 2151) + 8610) WHERE `slot_id` BETWEEN 2151 AND 2160; -- Bank Bag 13
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 2161) + 8810) WHERE `slot_id` BETWEEN 2161 AND 2170; -- Bank Bag 14
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 2171) + 9010) WHERE `slot_id` BETWEEN 2171 AND 2180; -- Bank Bag 15
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 2181) + 9210) WHERE `slot_id` BETWEEN 2181 AND 2190; -- Bank Bag 16
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 2191) + 9410) WHERE `slot_id` BETWEEN 2191 AND 2200; -- Bank Bag 17
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 2201) + 9610) WHERE `slot_id` BETWEEN 2201 AND 2210; -- Bank Bag 18
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 2211) + 9810) WHERE `slot_id` BETWEEN 2211 AND 2220; -- Bank Bag 19
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 2221) + 10010) WHERE `slot_id` BETWEEN 2221 AND 2230; -- Bank Bag 20
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 2231) + 10210) WHERE `slot_id` BETWEEN 2231 AND 2240; -- Bank Bag 21
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 2241) + 10410) WHERE `slot_id` BETWEEN 2241 AND 2250; -- Bank Bag 22
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 2251) + 10610) WHERE `slot_id` BETWEEN 2251 AND 2260; -- Bank Bag 23
UPDATE `inventory` SET `slot_id` = ((`slot_id` - 2261) + 10810) WHERE `slot_id` BETWEEN 2261 AND 2270; -- Bank Bag 24
UPDATE `sharedbank` SET `slot_id` = ((`slot_id` - 2531) + 11010) WHERE `slot_id` BETWEEN 2531 AND 2540; -- Shared Bank Bag 1
UPDATE `sharedbank` SET `slot_id` = ((`slot_id` - 2541) + 11210) WHERE `slot_id` BETWEEN 2541 AND 2550; -- Shared Bank Bag 2
)",
		.content_schema_update = false,
		.force_interactive = false
	},
	ManifestEntry{
		.version = 9299,
		.description = "2024_10_24_merchantlist_temp_uncap.sql",
		.check = "SHOW CREATE TABLE `merchantlist_temp`",
		.condition = "contains",
		.match = "`slot` tinyint(3)",
		.sql = R"(
ALTER TABLE `merchantlist_temp`
MODIFY COLUMN `slot` int UNSIGNED NOT NULL DEFAULT 0 AFTER `npcid`;
)",
		.content_schema_update = false
	},
	ManifestEntry{
		.version = 9300,
		.description = "2024_10_15_npc_types_multiquest_enabled.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'multiquest_enabled'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types`
ADD COLUMN `multiquest_enabled` tinyint(1) UNSIGNED NOT NULL DEFAULT 0 AFTER `is_parcel_merchant`;
)",
		.content_schema_update = true
	},
	ManifestEntry{
		.version = 9301,
		.description = "2024_10_08_add_detail_player_event_logging.sql",
		.check       = "SHOW COLUMNS FROM `player_event_log_settings` LIKE 'etl_enabled'",
		.condition   = "empty",
		.match       = "",
		.sql = R"(
ALTER TABLE `player_event_log_settings`
	ADD COLUMN `etl_enabled` TINYINT(1) UNSIGNED NOT NULL DEFAULT '0' AFTER `discord_webhook_id`;
ALTER TABLE `player_event_logs`
	ADD COLUMN `etl_table_id` BIGINT(20) NOT NULL DEFAULT '0' AFTER `event_data`;
UPDATE `player_event_log_settings` SET `etl_enabled` = 1 WHERE `id` = 14;
CREATE TABLE `player_event_loot_items` (
	`id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
	`item_id` INT(10) UNSIGNED NULL DEFAULT NULL,
	`item_name` VARCHAR(64) NULL DEFAULT NULL COLLATE 'latin1_swedish_ci',
	`charges` INT(11) NULL DEFAULT NULL,
	`augment_1_id` INT UNSIGNED NULL DEFAULT '0',
	`augment_2_id` INT UNSIGNED NULL DEFAULT '0',
	`augment_3_id` INT UNSIGNED NULL DEFAULT '0',
	`augment_4_id` INT UNSIGNED NULL DEFAULT '0',
	`augment_5_id` INT UNSIGNED NULL DEFAULT '0',
	`augment_6_id` INT UNSIGNED NULL DEFAULT '0',
	`npc_id` INT(10) UNSIGNED NULL DEFAULT NULL,
	`corpse_name` VARCHAR(64) NULL DEFAULT NULL COLLATE 'latin1_swedish_ci',
	`created_at` DATETIME NULL DEFAULT NULL,
	PRIMARY KEY (`id`) USING BTREE,
	INDEX `item_id_npc_id` (`item_id`, `npc_id`) USING BTREE,
	INDEX `created_at` (`created_at`) USING BTREE
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
AUTO_INCREMENT=1;
UPDATE `player_event_log_settings` SET `etl_enabled` = 1 WHERE `id` = 16;
CREATE TABLE `player_event_merchant_sell` (
	`id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
	`npc_id` INT(10) UNSIGNED NULL DEFAULT '0',
	`merchant_name` VARCHAR(64) NULL DEFAULT NULL COLLATE 'latin1_swedish_ci',
	`merchant_type` INT(10) UNSIGNED NULL DEFAULT '0',
	`item_id` INT(10) UNSIGNED NULL DEFAULT '0',
	`item_name` VARCHAR(64) NULL DEFAULT NULL COLLATE 'latin1_swedish_ci',
	`charges` INT(11) NULL DEFAULT '0',
	`cost` INT(10) UNSIGNED NULL DEFAULT '0',
	`alternate_currency_id` INT(10) UNSIGNED NULL DEFAULT '0',
	`player_money_balance` BIGINT(20) UNSIGNED NULL DEFAULT '0',
	`player_currency_balance` BIGINT(20) UNSIGNED NULL DEFAULT '0',
	`created_at` DATETIME NULL DEFAULT NULL,
	PRIMARY KEY (`id`) USING BTREE,
	INDEX `item_id_npc_id` (`item_id`, `npc_id`) USING BTREE,
	INDEX `created_at` (`created_at`) USING BTREE
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
AUTO_INCREMENT=1;
UPDATE `player_event_log_settings` SET `etl_enabled` = 1 WHERE `id` = 15;
CREATE TABLE `player_event_merchant_purchase` (
	`id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
	`npc_id` INT(10) UNSIGNED NULL DEFAULT '0',
	`merchant_name` VARCHAR(64) NULL DEFAULT NULL COLLATE 'latin1_swedish_ci',
	`merchant_type` INT(10) UNSIGNED NULL DEFAULT '0',
	`item_id` INT(10) UNSIGNED NULL DEFAULT '0',
	`item_name` VARCHAR(64) NULL DEFAULT NULL COLLATE 'latin1_swedish_ci',
	`charges` INT(11) NULL DEFAULT '0',
	`cost` INT(10) UNSIGNED NULL DEFAULT '0',
	`alternate_currency_id` INT(10) UNSIGNED NULL DEFAULT '0',
	`player_money_balance` BIGINT(20) UNSIGNED NULL DEFAULT '0',
	`player_currency_balance` BIGINT(20) UNSIGNED NULL DEFAULT '0',
	`created_at` DATETIME NULL DEFAULT NULL,
	PRIMARY KEY (`id`) USING BTREE,
	INDEX `item_id_npc_id` (`item_id`, `npc_id`) USING BTREE,
	INDEX `created_at` (`created_at`) USING BTREE
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
AUTO_INCREMENT=1;
UPDATE `player_event_log_settings` SET `etl_enabled` = 1 WHERE `id` = 22;
CREATE TABLE `player_event_npc_handin` (
	`id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
	`npc_id` INT(10) UNSIGNED NULL DEFAULT '0',
	`npc_name` VARCHAR(64) NULL DEFAULT NULL COLLATE 'latin1_swedish_ci',
	`handin_copper` BIGINT(20) UNSIGNED NULL DEFAULT '0',
	`handin_silver` BIGINT(20) UNSIGNED NULL DEFAULT '0',
	`handin_gold` BIGINT(20) UNSIGNED NULL DEFAULT '0',
	`handin_platinum` BIGINT(20) UNSIGNED NULL DEFAULT '0',
	`return_copper` BIGINT(20) UNSIGNED NULL DEFAULT '0',
	`return_silver` BIGINT(20) UNSIGNED NULL DEFAULT '0',
	`return_gold` BIGINT(20) UNSIGNED NULL DEFAULT '0',
	`return_platinum` BIGINT(20) UNSIGNED NULL DEFAULT '0',
	`is_quest_handin` TINYINT(3) UNSIGNED NULL DEFAULT '0',
	`created_at` DATETIME NULL DEFAULT NULL,
	PRIMARY KEY (`id`) USING BTREE,
	INDEX `npc_id_is_quest_handin` (`npc_id`, `is_quest_handin`) USING BTREE,
	INDEX `created_at` (`created_at`) USING BTREE
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
AUTO_INCREMENT=1;
CREATE TABLE `player_event_npc_handin_entries` (
	`id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
	`player_event_npc_handin_id` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
	`type` INT(10) UNSIGNED NULL DEFAULT NULL,
	`item_id` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`charges` INT(11) NOT NULL DEFAULT '0',
	`evolve_level` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`evolve_amount` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
	`augment_1_id` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`augment_2_id` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`augment_3_id` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`augment_4_id` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`augment_5_id` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`augment_6_id` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`created_at` DATETIME NULL DEFAULT NULL,
	PRIMARY KEY (`id`) USING BTREE,
	INDEX `type_item_id` (`type`, `item_id`) USING BTREE,
	INDEX `player_event_npc_handin_id` (`player_event_npc_handin_id`) USING BTREE,
	INDEX `created_at` (`created_at`) USING BTREE
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
AUTO_INCREMENT=1;
UPDATE `player_event_log_settings` SET `etl_enabled` = 1 WHERE `id` = 27;
CREATE TABLE `player_event_trade` (
	`id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`char1_id` INT(10) UNSIGNED NULL DEFAULT '0',
	`char2_id` INT(10) UNSIGNED NULL DEFAULT '0',
	`char1_copper` BIGINT(20) UNSIGNED NULL DEFAULT '0',
	`char1_silver` BIGINT(20) UNSIGNED NULL DEFAULT '0',
	`char1_gold` BIGINT(20) UNSIGNED NULL DEFAULT '0',
	`char1_platinum` BIGINT(20) UNSIGNED NULL DEFAULT '0',
	`char2_copper` BIGINT(20) UNSIGNED NULL DEFAULT '0',
	`char2_silver` BIGINT(20) UNSIGNED NULL DEFAULT '0',
	`char2_gold` BIGINT(20) UNSIGNED NULL DEFAULT '0',
	`char2_platinum` BIGINT(20) UNSIGNED NULL DEFAULT '0',
	`created_at` DATETIME NULL DEFAULT NULL,
	PRIMARY KEY (`id`) USING BTREE,
	INDEX `char1_id_char2_id` (`char1_id`, `char2_id`) USING BTREE,
	INDEX `created_at` (`created_at`) USING BTREE
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
AUTO_INCREMENT=1;
CREATE TABLE `player_event_trade_entries` (
	`id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
	`player_event_trade_id` BIGINT(20) UNSIGNED NULL DEFAULT '0',
	`char_id` INT(10) UNSIGNED NULL DEFAULT '0',
	`slot` SMALLINT(6) NULL DEFAULT '0',
	`item_id` INT(10) UNSIGNED NULL DEFAULT '0',
	`charges` SMALLINT(6) NULL DEFAULT '0',
	`augment_1_id` INT(10) UNSIGNED NULL DEFAULT '0',
	`augment_2_id` INT(10) UNSIGNED NULL DEFAULT '0',
	`augment_3_id` INT(10) UNSIGNED NULL DEFAULT '0',
	`augment_4_id` INT(10) UNSIGNED NULL DEFAULT '0',
	`augment_5_id` INT(10) UNSIGNED NULL DEFAULT '0',
	`augment_6_id` INT(10) UNSIGNED NULL DEFAULT '0',
	`in_bag` TINYINT(4) NULL DEFAULT '0',
	`created_at` DATETIME NULL DEFAULT NULL,
	PRIMARY KEY (`id`) USING BTREE,
	INDEX `player_event_trade_id` (`player_event_trade_id`) USING BTREE,
	INDEX `created_at` (`created_at`) USING BTREE
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
AUTO_INCREMENT=1;
UPDATE `player_event_log_settings` SET `etl_enabled` = 0 WHERE `id` = 54;
CREATE TABLE `player_event_speech` (
	`id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
	`to_char_id` VARCHAR(64) NULL DEFAULT NULL COLLATE 'latin1_swedish_ci',
	`from_char_id` VARCHAR(64) NULL DEFAULT NULL COLLATE 'latin1_swedish_ci',
	`guild_id` INT(10) UNSIGNED NULL DEFAULT '0',
	`type` INT(10) UNSIGNED NULL DEFAULT '0',
	`min_status` INT(10) UNSIGNED NULL DEFAULT '0',
	`message` LONGTEXT NULL DEFAULT NULL COLLATE 'latin1_swedish_ci',
	`created_at` DATETIME NULL DEFAULT NULL,
	PRIMARY KEY (`id`) USING BTREE,
	INDEX `to_char_id_from_char_id` (`to_char_id`, `from_char_id`) USING BTREE,
	INDEX `created_at` (`created_at`) USING BTREE
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
AUTO_INCREMENT=1;
UPDATE `player_event_log_settings` SET `etl_enabled` = 1 WHERE `id` = 44;
CREATE TABLE `player_event_killed_npc` (
	`id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
	`npc_id` INT(10) UNSIGNED NULL DEFAULT '0',
	`npc_name` VARCHAR(64) NULL DEFAULT NULL COLLATE 'latin1_swedish_ci',
	`combat_time_seconds` INT(10) UNSIGNED NULL DEFAULT '0',
	`total_damage_per_second_taken` BIGINT(20) UNSIGNED NULL DEFAULT '0',
	`total_heal_per_second_taken` BIGINT(20) UNSIGNED NULL DEFAULT '0',
	`created_at` DATETIME NULL DEFAULT NULL,
	PRIMARY KEY (`id`) USING BTREE,
	INDEX `npc_id` (`npc_id`) USING BTREE,
	INDEX `created_at` (`created_at`) USING BTREE
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB;
UPDATE `player_event_log_settings` SET `etl_enabled` = 1 WHERE `id` = 45;
CREATE TABLE `player_event_killed_named_npc` (
	`id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
	`npc_id` INT(10) UNSIGNED NULL DEFAULT '0',
	`npc_name` VARCHAR(64) NULL DEFAULT NULL COLLATE 'latin1_swedish_ci',
	`combat_time_seconds` INT(10) UNSIGNED NULL DEFAULT '0',
	`total_damage_per_second_taken` BIGINT(20) UNSIGNED NULL DEFAULT '0',
	`total_heal_per_second_taken` BIGINT(20) UNSIGNED NULL DEFAULT '0',
	`created_at` DATETIME NULL DEFAULT NULL,
	PRIMARY KEY (`id`) USING BTREE,
	INDEX `npc_id` (`npc_id`) USING BTREE,
	INDEX `created_at` (`created_at`) USING BTREE
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB;
UPDATE `player_event_log_settings` SET `etl_enabled` = 1 WHERE `id` = 46;
CREATE TABLE `player_event_killed_raid_npc` (
	`id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
	`npc_id` INT(10) UNSIGNED NULL DEFAULT '0',
	`npc_name` VARCHAR(64) NULL DEFAULT NULL COLLATE 'latin1_swedish_ci',
	`combat_time_seconds` INT(10) UNSIGNED NULL DEFAULT '0',
	`total_damage_per_second_taken` BIGINT(20) UNSIGNED NULL DEFAULT '0',
	`total_heal_per_second_taken` BIGINT(20) UNSIGNED NULL DEFAULT '0',
	`created_at` DATETIME NULL DEFAULT NULL,
	PRIMARY KEY (`id`) USING BTREE,
	INDEX `npc_id` (`npc_id`) USING BTREE,
	INDEX `created_at` (`created_at`) USING BTREE
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB;
UPDATE `player_event_log_settings` SET `etl_enabled` = 1 WHERE `id` = 4;
CREATE TABLE `player_event_aa_purchase` (
	`id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
	`aa_ability_id` INT(11) NULL DEFAULT '0',
	`cost` INT(11) NULL DEFAULT '0',
	`previous_id` INT(11) NULL DEFAULT '0',
	`next_id` INT(11) NULL DEFAULT '0',
	`created_at` DATETIME NULL DEFAULT NULL,
	PRIMARY KEY (`id`) USING BTREE,
	INDEX `created_at` (`created_at`) USING BTREE
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
;
)"
	},
	ManifestEntry{
		.version = 9302,
		.description = "2025_02_09_illusion_block.sql",
		.check = "SHOW COLUMNS FROM `character_data` LIKE 'illusion_block'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `character_data`
	ADD COLUMN `illusion_block` TINYINT(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `deleted_at`;

UPDATE `command_settings`
SET `aliases` =
    CASE
        WHEN LENGTH(`aliases`) > 0 AND `aliases` NOT LIKE '%|ib%'
            THEN CONCAT(`aliases`, '|ib')
        WHEN LENGTH(`aliases`) = 0
            THEN 'ib'
        ELSE `aliases`
    END
WHERE `command` = 'illusionblock'
AND `aliases` NOT LIKE '%ib%';
)",
	},
	ManifestEntry{
		.version = 9303,
		.description = "2025_02_13_corpse_slot_fix.sql",
		.check = "SELECT * FROM `character_corpse_items` WHERE `equip_slot` BETWEEN 251 AND 350",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
UPDATE `character_corpse_items` SET `equip_slot` = ((`equip_slot` - 251) + 4010) WHERE `equip_slot` BETWEEN 251 AND 260; -- Bag 1
UPDATE `character_corpse_items` SET `equip_slot` = ((`equip_slot` - 261) + 4210) WHERE `equip_slot` BETWEEN 261 AND 270; -- Bag 2
UPDATE `character_corpse_items` SET `equip_slot` = ((`equip_slot` - 271) + 4410) WHERE `equip_slot` BETWEEN 271 AND 280; -- Bag 3
UPDATE `character_corpse_items` SET `equip_slot` = ((`equip_slot` - 281) + 4610) WHERE `equip_slot` BETWEEN 281 AND 290; -- Bag 4
UPDATE `character_corpse_items` SET `equip_slot` = ((`equip_slot` - 291) + 4810) WHERE `equip_slot` BETWEEN 291 AND 300; -- Bag 5
UPDATE `character_corpse_items` SET `equip_slot` = ((`equip_slot` - 301) + 5010) WHERE `equip_slot` BETWEEN 301 AND 310; -- Bag 6
UPDATE `character_corpse_items` SET `equip_slot` = ((`equip_slot` - 311) + 5210) WHERE `equip_slot` BETWEEN 311 AND 320; -- Bag 7
UPDATE `character_corpse_items` SET `equip_slot` = ((`equip_slot` - 321) + 5410) WHERE `equip_slot` BETWEEN 321 AND 330; -- Bag 8
UPDATE `character_corpse_items` SET `equip_slot` = ((`equip_slot` - 331) + 5610) WHERE `equip_slot` BETWEEN 331 AND 340; -- Bag 9
UPDATE `character_corpse_items` SET `equip_slot` = ((`equip_slot` - 341) + 5810) WHERE `equip_slot` BETWEEN 341 AND 350; -- Bag 10
)",
	},
	ManifestEntry{
		.version     = 9304,
		.description = "2024_12_01_update_guild_bank",
		.check       = "SHOW COLUMNS FROM `guild_bank` LIKE 'augment_one_id'",
		.condition   = "empty",
		.match       = "",
		.sql         = R"(
ALTER TABLE `guild_bank`
	DROP INDEX `guildid`,
	CHANGE COLUMN `guildid` `guild_id` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `id`,
	CHANGE COLUMN `itemid` `item_id` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `slot`,
	CHANGE COLUMN `whofor` `who_for` VARCHAR(64) NULL DEFAULT NULL COLLATE 'utf8_general_ci' AFTER `permissions`,
	ADD COLUMN `augment_one_id` INT UNSIGNED NULL DEFAULT '0' AFTER `item_id`,
	ADD COLUMN `augment_two_id` INT UNSIGNED NULL DEFAULT '0' AFTER `augment_one_id`,
	ADD COLUMN `augment_three_id` INT UNSIGNED NULL DEFAULT '0' AFTER `augment_two_id`,
	ADD COLUMN `augment_four_id` INT UNSIGNED NULL DEFAULT '0' AFTER `augment_three_id`,
	ADD COLUMN `augment_five_id` INT UNSIGNED NULL DEFAULT '0' AFTER `augment_four_id`,
	ADD COLUMN `augment_six_id` INT UNSIGNED NULL DEFAULT '0' AFTER `augment_five_id`,
	CHANGE COLUMN `qty` `quantity` INT(10) NOT NULL DEFAULT '0' AFTER `augment_six_id`;
ALTER TABLE `guild_bank`
	ADD INDEX `guild_id` (`guild_id`);
)"
	},
	ManifestEntry{
		.version = 9305,
		.description = "2024_12_01_expedition_dz_merge.sql",
		.check = "SHOW COLUMNS FROM `dynamic_zones` LIKE 'is_locked'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `dynamic_zones`
	ADD COLUMN `is_locked` TINYINT NOT NULL DEFAULT '0' AFTER `has_zone_in`,
	ADD COLUMN `add_replay` TINYINT NOT NULL DEFAULT '1' AFTER `is_locked`;

ALTER TABLE `expedition_lockouts`
	CHANGE COLUMN `expedition_id` `dynamic_zone_id` INT(10) UNSIGNED NOT NULL AFTER `id`,
	DROP INDEX `expedition_id_event_name`,
	ADD UNIQUE INDEX `dz_id_event_name` (`dynamic_zone_id`, `event_name`) USING BTREE;

UPDATE expedition_lockouts lockouts
	INNER JOIN expeditions ON lockouts.dynamic_zone_id = expeditions.id
	SET lockouts.dynamic_zone_id = expeditions.dynamic_zone_id;

DROP TABLE `expeditions`;

RENAME TABLE `expedition_lockouts` TO `dynamic_zone_lockouts`;
)"
	},
	ManifestEntry{
		.version = 9306,
		.description = "2025_02_16_data_buckets_zone_id_instance_id.sql",
		.check       = "SHOW COLUMNS FROM `data_buckets` LIKE 'zone_id'",
		.condition   = "empty",
		.match = "",
		.sql = R"(
-- Drop old indexes if exists
DROP INDEX IF EXISTS `keys` ON `data_buckets`;
DROP INDEX IF EXISTS `idx_npc_expires` ON `data_buckets`;
DROP INDEX IF EXISTS `idx_bot_expires` ON `data_buckets`;

-- Add zone_id, instance_id
ALTER TABLE `data_buckets`
	MODIFY COLUMN `npc_id` int(11) NOT NULL DEFAULT 0 AFTER `character_id`,
	MODIFY COLUMN `bot_id` int(11) NOT NULL DEFAULT 0 AFTER `npc_id`,
	ADD COLUMN `zone_id` smallint(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `bot_id`,
	ADD COLUMN `instance_id` smallint(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `zone_id`;

ALTER TABLE `data_buckets`
	MODIFY COLUMN `account_id` bigint(11) UNSIGNED NULL DEFAULT 0 AFTER `expires`,
	MODIFY COLUMN `character_id` bigint(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `account_id`,
	MODIFY COLUMN `npc_id` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `character_id`,
	MODIFY COLUMN `bot_id` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `npc_id`;

-- Create optimized unique index with `key` first
CREATE UNIQUE INDEX `keys` ON data_buckets (`key`, character_id, npc_id, bot_id, account_id, zone_id, instance_id);

-- Create indexes for just instance_id (instance deletion)
CREATE INDEX idx_instance_id ON data_buckets (instance_id);
)",
		.content_schema_update = false
	},
	ManifestEntry{
		.version     = 9307,
		.description = "2025_02_17_zone_state_spawns.sql",
		.check       = "SHOW TABLES LIKE 'zone_state_spawns'",
		.condition   = "empty",
		.match       = "",
		.sql         = R"(
CREATE TABLE `zone_state_spawns` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `zone_id` int(11) unsigned DEFAULT NULL,
  `instance_id` int(11) unsigned DEFAULT NULL,
  `is_corpse` tinyint(11) DEFAULT 0,
  `decay_in_seconds` int(11) DEFAULT 0,
  `npc_id` int(10) unsigned DEFAULT NULL,
  `spawn2_id` int(10) unsigned NOT NULL,
  `spawngroup_id` int(10) unsigned NOT NULL,
  `x` float NOT NULL,
  `y` float NOT NULL,
  `z` float NOT NULL,
  `heading` float NOT NULL,
  `respawn_time` int(10) unsigned NOT NULL,
  `variance` int(10) unsigned NOT NULL,
  `grid` int(10) unsigned DEFAULT 0,
  `current_waypoint` int(11) DEFAULT 0,
  `path_when_zone_idle` smallint(6) DEFAULT 0,
  `condition_id` smallint(5) unsigned DEFAULT 0,
  `condition_min_value` smallint(6) DEFAULT 0,
  `enabled` smallint(6) DEFAULT 1,
  `anim` smallint(5) unsigned DEFAULT 0,
  `loot_data` text DEFAULT NULL,
  `entity_variables` text DEFAULT NULL,
  `buffs` text DEFAULT NULL,
  `hp` bigint(20) DEFAULT 0,
  `mana` bigint(20) DEFAULT 0,
  `endurance` bigint(20) DEFAULT 0,
  `created_at` datetime DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4
)",
		.content_schema_update = false
	},
	ManifestEntry{
		.version = 9308,
		.description = "2025_03_29_add_multivalue_support_to_evolving_subtype.sql",
		.check = "SHOW COLUMNS FROM `items_evolving_details` LIKE 'sub_type'",
		.condition = "missing",
		.match = "varchar(200)",
		.sql = R"(
ALTER TABLE `items_evolving_details`
	CHANGE COLUMN `sub_type` `sub_type` VARCHAR(200) NULL DEFAULT '0' AFTER `type`;
)",
		.content_schema_update = true
	},
	// this one got missed being added to PEQ dumps so adding it again so it gets added when folks take a new release
	ManifestEntry{
		.version = 9309,
		.description = "2025_03_1_create_pet_names_table_if_not_exist.sql",
		.check = "SHOW TABLES LIKE 'character_pet_name'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `character_pet_name` (
    `character_id` INT(11) NOT NULL PRIMARY KEY,
    `name` VARCHAR(64) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
)",
	},
	ManifestEntry{
		.version = 9310,
		.description = "2025_03_7_expand_horse_def.sql",
		.check = "SHOW COLUMNS FROM `horses` LIKE 'helmtexture'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `horses`
	ADD COLUMN `helmtexture` TINYINT(2) NOT NULL DEFAULT -1 AFTER `texture`;
)",
		.content_schema_update = true
	},
	ManifestEntry{
		.version = 9311,
		.description = "2025_03_09_add_zone_state_is_zone_field.sql",
		.check = "SHOW COLUMNS FROM `zone_state_spawns` LIKE 'is_zone'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `zone_state_spawns`
	ADD COLUMN `is_zone` tinyint(11) NULL DEFAULT 0 AFTER `is_corpse`;
)",
		.content_schema_update = false
	},
	ManifestEntry{
		.version = 9312,
		.description = "2025_03_11_data_bucket_indexes.sql",
		.check = "SHOW INDEX FROM data_buckets",
		.condition = "missing",
		.match = "idx_zone_instance_expires",
		.sql = R"(
DROP INDEX IF EXISTS `idx_zone_instance_expires` ON `data_buckets`;
DROP INDEX IF EXISTS `idx_character_expires` ON `data_buckets`;
DROP INDEX IF EXISTS `idx_bot_expires` ON `data_buckets`;
ALTER TABLE data_buckets ADD INDEX idx_zone_instance_expires (zone_id, instance_id, expires);
ALTER TABLE data_buckets ADD INDEX idx_character_expires (character_id, expires);
ALTER TABLE data_buckets ADD INDEX idx_bot_expires (bot_id, expires);
)",
	.content_schema_update = false
	},
	ManifestEntry{
		.version = 9313,
		.description = "2025_03_11_zone_state_spawns.sql",
		.check = "SHOW INDEX FROM zone_state_spawns",
		.condition = "missing",
		.match = "idx_zone_instance",
		.sql = R"(
ALTER TABLE zone_state_spawns ADD INDEX idx_zone_instance (zone_id, instance_id);
)",
	.content_schema_update = false
	},
	ManifestEntry{
		.version = 9314,
		.description = "2025_03_12_zone_state_spawns_one_time_truncate.sql",
		.check = "SELECT * FROM db_version WHERE version >= 9314",
		.condition = "empty",
		.match = "",
		.sql = R"(
TRUNCATE TABLE zone_state_spawns;
)",
		.content_schema_update = false
	},
	ManifestEntry{
		.version = 9315,
		.description = "2025_03_29_character_tribute_index.sql",
		.check = "SHOW INDEX FROM character_tribute",
		.condition = "missing",
		.match = "idx_character_id",
		.sql = R"(
ALTER TABLE character_tribute ADD INDEX idx_character_id (character_id);
)",
		.content_schema_update = false
	},
	ManifestEntry{
		.version = 9316,
		.description = "2025_03_29_player_titlesets_index.sql",
		.check = "SHOW INDEX FROM player_titlesets",
		.condition = "missing",
		.match = "idx_char_id",
		.sql = R"(
ALTER TABLE player_titlesets ADD INDEX idx_char_id (char_id);
)",
		.content_schema_update = false
	},
	ManifestEntry{
		.version = 9317,
		.description = "2025_03_29_respawn_times_instance_index.sql",
		.check = "SHOW INDEX FROM respawn_times",
		.condition = "missing",
		.match = "idx_instance_id",
		.sql = R"(
ALTER TABLE respawn_times ADD INDEX idx_instance_id (instance_id);
)",
		.content_schema_update = false
	},
	ManifestEntry{
		.version = 9318,
		.description = "2025_03_29_zone_state_spawns_instance_index.sql",
		.check = "SHOW INDEX FROM zone_state_spawns",
		.condition = "missing",
		.match = "idx_instance_id",
		.sql = R"(
ALTER TABLE zone_state_spawns ADD INDEX idx_instance_id (instance_id);
)",
		.content_schema_update = false
	},
	ManifestEntry{
		.version = 9319,
		.description = "2025_03_29_data_buckets_expires_index.sql",
		.check = "SHOW INDEX FROM data_buckets",
		.condition = "missing",
		.match = "idx_expires",
		.sql = R"(
CREATE INDEX idx_expires ON data_buckets (expires);
)",
		.content_schema_update = false
	},
	ManifestEntry{
		.version = 9320,
		.description = "2025_03_23_add_respawn_times_expire_at.sql",
		.check = "SHOW COLUMNS FROM `respawn_times` LIKE 'expire_at'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `respawn_times`
ADD COLUMN `expire_at` int(11) UNSIGNED NULL DEFAULT 0 AFTER `duration`;

UPDATE respawn_times set expire_at = `start` + `duration`; -- backfill existing data

CREATE INDEX `idx_expire_at` ON `respawn_times` (`expire_at`);
)",
		.content_schema_update = false
	},
	ManifestEntry{
		.version = 9321,
		.description = "2025_03_30_instance_list_add_expire_at.sql",
		.check = "SHOW COLUMNS FROM `instance_list` LIKE 'expire_at'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `instance_list`
ADD COLUMN `expire_at` bigint(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `duration`;

UPDATE instance_list set expire_at = `start_time` + `duration`; -- backfill existing data

CREATE INDEX `idx_expire_at` ON `instance_list` (`expire_at`);
)",
		.content_schema_update = false
	},
	ManifestEntry{
		.version = 9322,
		.description = "2025_04_24_add_npc_tint_id.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'npc_tint_id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types`
	ADD COLUMN `npc_tint_id` SMALLINT UNSIGNED NULL DEFAULT '0' AFTER `multiquest_enabled`;
)",
		.content_schema_update = true
	},
	ManifestEntry{
		.version = 9323,
		.description = "2025_04_16_character_data_first_login.sql",
		.check = "SHOW COLUMNS FROM `character_data` LIKE 'first_login'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `character_data`
CHANGE COLUMN `firstlogon` `ingame` tinyint(1) UNSIGNED NOT NULL DEFAULT 0 AFTER `xtargets`,
ADD COLUMN `first_login` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `xtargets`;
)",
		.content_schema_update = false
	},
	ManifestEntry{
		.version = 9324,
		.description = "2025_06_11_player_event_logs_table.sql",
		.check = "SHOW CREATE TABLE `player_event_logs`",
		.condition = "missing",
		.match = "COMPRESS",
		.sql = R"(
ALTER TABLE player_event_logs ROW_FORMAT=COMPRESSED;
CREATE INDEX idx_event_type_char_id ON player_event_logs (event_type_id, character_id);
)",
		.content_schema_update = false
	},
	ManifestEntry{
		.version = 9325,
		.description = "2025_06_10_character_corpses_entity_variables.sql",
		.check = "SHOW COLUMNS FROM `character_corpses` LIKE 'entity_variables'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `character_corpses`
ADD COLUMN `entity_variables` TEXT DEFAULT NULL AFTER `rezzable`;
)",
		.content_schema_update = false
	},
	ManifestEntry{
		.version = 9326,
		.description = "2025_07_27_add_indexes_npc_spawns_loot.sql",
		.check = "SHOW INDEX FROM npc_types",
		.condition = "missing",
		.match = "idx_npc_types_loottable_id",
		.sql = R"(
ALTER TABLE npc_types
    ADD INDEX idx_npc_types_loottable_id (loottable_id);

ALTER TABLE spawnentry
    ADD INDEX idx_spawnentry_spawngroup_id (spawngroupID),
    ADD INDEX idx_spawnentry_npc_id (npcID);

ALTER TABLE lootdrop_entries
    ADD INDEX idx_lootdrop_entries_lootdrop_id (lootdrop_id),
    ADD INDEX idx_lootdrop_entries_item_id (item_id);

ALTER TABLE loottable_entries
    ADD INDEX idx_loottable_entries_lootdrop_id (lootdrop_id),
    ADD INDEX idx_loottable_entries_loottable_id (loottable_id);
)",
		.content_schema_update = true
	},
	ManifestEntry{
		.version = 9327,
		.description = "2025_08_13_character_stats_record_heal_amount.sql",
		.check = "SHOW COLUMNS FROM `character_stats_record` LIKE 'heal_amount'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `character_stats_record`
ADD COLUMN `heal_amount` int(11) NULL DEFAULT 0 AFTER `spell_damage`;
)"
	},
	ManifestEntry{
		.version = 9328,
		.description = "2025_08_22_character_parcel_updates.sql",
		.check = "SHOW COLUMNS FROM `character_parcels` LIKE 'evolve_amount'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `character_parcels`
	ADD COLUMN `evolve_amount` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `quantity`;

ALTER TABLE `character_parcels_containers`
	ADD COLUMN `evolve_amount` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `quantity`;
)",
		.content_schema_update = false
	},
// -- template; copy/paste this when you need to create a new entry
//	ManifestEntry{
//		.version = 9228,
//		.description = "some_new_migration.sql",
//		.check = "SHOW COLUMNS FROM `table_name` LIKE 'column_name'",
//		.condition = "empty",
//		.match = "",
//		.sql = R"(
//
//)"

	// Used for testing
//	ManifestEntry{
//		.version = 9229,
//		.description = "new_database_check_test",
//		.check = "SHOW TABLES LIKE 'new_table'",
//		.condition = "empty",
//		.match = "",
//		.sql = R"(
//CREATE TABLE `new_table`  (
//  `id` int NOT NULL AUTO_INCREMENT,
//  PRIMARY KEY (`id`)
//);
//CREATE TABLE `new_table1`  (
//  `id` int NOT NULL AUTO_INCREMENT,
//  PRIMARY KEY (`id`)
//);
//CREATE TABLE `new_table2`  (
//  `id` int NOT NULL AUTO_INCREMENT,
//  PRIMARY KEY (`id`)
//);
//CREATE TABLE `new_table3`  (
//  `id` int NOT NULL AUTO_INCREMENT,
//  PRIMARY KEY (`id`)
//);
//)",
//	}

};

// see struct definitions for what each field does
// struct ManifestEntry {
// 	int         version{};     // database version of the migration
// 	std::string description{}; // description of the migration ex: "add_new_table" or "add_index_to_table"
// 	std::string check{};       // query that checks against the condition
// 	std::string condition{};   // condition or "match_type" - Possible values [contains|match|missing|empty|not_empty]
// 	std::string match{};       // match field that is not always used, but works in conjunction with "condition" values [missing|match|contains]
// 	std::string sql{};         // the SQL DDL that gets ran when the condition is true
// };
