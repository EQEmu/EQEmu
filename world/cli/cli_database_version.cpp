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

#include "common/json/json.h"
#include "common/rulesys.h"
#include "common/version.h"

#include <iostream>

void WorldserverCLI::DatabaseVersion(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Shows database version";

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	Json::Value v;

	v["database_version"]        = CURRENT_BINARY_DATABASE_VERSION;
	v["bots_database_version"]   = RuleB(Bots, Enabled) ? CURRENT_BINARY_BOTS_DATABASE_VERSION : 0;
	v["custom_database_version"] = CUSTOM_BINARY_DATABASE_VERSION;

	std::cout << v << std::endl;
}
