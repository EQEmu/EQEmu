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

#include "common/rulesys.h"
#include "world/worlddb.h"

void WorldserverCLI::BotsEnable(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Bootstraps bot tables and enables bots";

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	// bootstrap bot tables if they don't exist
	if (!database.DoesTableExist("bot_data")) {
		LogInfo("Bootstrapping bot tables");
		database.SourceSqlFromUrl(
			"https://raw.githubusercontent.com/EQEmu/Server/master/utils/sql/bot_tables_bootstrap.sql"
		);
	}
	else {
		LogInfo("Bot tables already exist, skipping bootstrap");
	}

	LogInfo("Enabling bots");
	LogInfo("Setting rule Bots:Enabled to true");
	RuleManager::Instance()->SetRule("Bots:Enabled", "true", &database, true, true);
}
