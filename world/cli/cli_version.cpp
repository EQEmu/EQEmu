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
#include "common/version.h"

#include <iostream>

void WorldserverCLI::Version(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Shows server version";

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	Json::Value j;

	j["bots_database_version"]   = CURRENT_BINARY_BOTS_DATABASE_VERSION;
	j["compile_date"]            = COMPILE_DATE;
	j["compile_time"]            = COMPILE_TIME;
	j["custom_database_version"] = CUSTOM_BINARY_DATABASE_VERSION;
	j["database_version"]        = CURRENT_BINARY_DATABASE_VERSION;
	j["server_version"]          = CURRENT_VERSION;

	std::cout << j << std::endl;
}
