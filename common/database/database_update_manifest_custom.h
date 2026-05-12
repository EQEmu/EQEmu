/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include "common/database/database_update.h"

#include <vector>

std::vector<ManifestEntry> manifest_entries_custom = {
	ManifestEntry{
		.version = 1,
		.description = "2025_05_16_new_database_check_test",
		.check = "SHOW TABLES LIKE 'new_table'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `new_table`  (
  `id` int NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`id`)
);
)",
		.content_schema_update = false,
	},
	ManifestEntry{
		.version = 2,
		.description = "2026_05_10_character_data_add_class_mask",
		.check = "SELECT COLUMN_NAME FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'character_data' AND COLUMN_NAME = 'class_mask'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `character_data` ADD COLUMN `class_mask` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `class`;
UPDATE `character_data` SET `class_mask` = POWER(2, `class` - 1) WHERE `class_mask` = 0 AND `class` BETWEEN 1 AND 16;
)",
		.content_schema_update = false,
	},
	ManifestEntry{
		.version = 3,
		.description = "2026_05_12_dynamic_zones_add_suspended_at",
		.check = "SELECT COLUMN_NAME FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'dynamic_zones' AND COLUMN_NAME = 'suspended_at'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `dynamic_zones` ADD COLUMN `suspended_at` INT UNSIGNED NULL DEFAULT NULL AFTER `add_replay`;
)",
		.content_schema_update = false,
	},
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
