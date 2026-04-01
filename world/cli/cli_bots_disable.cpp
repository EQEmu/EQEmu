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

void WorldserverCLI::BotsDisable(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Disables bots and drops tables";

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	std::string input;
	std::cout << "Warning! This will drop all bot tables, are you sure? [y/n]";
	std::getline(std::cin, input);

	if (Strings::ToLower(input) != "y") {
		LogInfo("Aborting");
		return;
	}

	// drop bot tables
	std::string sql = "SET FOREIGN_KEY_CHECKS = 0;";
	for (auto &t: DatabaseSchema::GetBotTables()) {
		sql += fmt::format("DROP TABLE IF EXISTS {};", t);
		LogInfo("Dropping table [{}]", t);
	}

	sql += "SET FOREIGN_KEY_CHECKS = 1;";

	database.QueryDatabaseMulti(sql);

	// disable bots
	LogInfo("Setting rule Bots:Enabled to false");
	RuleManager::Instance()->SetRule("Bots:Enabled", "false", &database, true, true);
	LogInfo("Bots disabled");
}
