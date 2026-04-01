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
#include "world/world_server_cli.h"

#include "common/database_schema.h"
#include "world/worlddb.h"

void WorldserverCLI::MercsEnable(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Enables mercenaries";

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	// bootstrap merc tables if they don't exist
	if (!database.DoesTableExist("merc_types")) {
		LogInfo("Bootstrapping merc tables");
		database.SourceSqlFromUrl(
			"https://raw.githubusercontent.com/EQEmu/Server/master/utils/sql/merc_tables_bootstrap.sql"
		);
	}
	else {
		LogInfo("Merc tables already exist, skipping bootstrap");
	}

	uint32 expansions_setting        = RuleManager::Instance()->GetIntRule(RuleManager::Int__ExpansionSettings);
	uint32 expansions_setting_before = expansions_setting;
	bool   update_expansions         = false;

	if (expansions_setting != -1 && !(expansions_setting & EQ::expansions::ExpansionBitmask::bitSoD)) {
		expansions_setting += EQ::expansions::ExpansionBitmask::bitSoD;
		update_expansions = true;
	}

	LogInfo("Enabling Mercenaries");
	LogInfo("Setting rule Mercs:AllowMercs to true");
	RuleManager::Instance()->SetRule("Mercs:AllowMercs", "true", &database, true, true);

	if (update_expansions) {
		LogInfo(
			"Updating World:ExpansionSettings from [{}] to [{}] to enable Mercenary Liaison spawns",
			expansions_setting_before,
			expansions_setting
		);

		RuleManager::Instance()->SetRule(
			"World:ExpansionSettings",
			std::to_string(expansions_setting),
			&database,
			true,
			true
		);
	}

	LogInfo("Mercenaries enabled");
}
