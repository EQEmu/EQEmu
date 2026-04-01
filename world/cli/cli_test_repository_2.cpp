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

#include "common/repositories/zone_repository.h"
#include "world/worlddb.h"

void WorldserverCLI::TestRepository2(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Test command";

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	auto zones = ZoneRepository::GetWhere(content_db, "short_name = 'anguish'");

	for (auto &zone: zones) {
		LogInfo(
			"Zone [{}] long_name [{}] id [{}]",
			zone.short_name,
			zone.long_name,
			zone.id
		);
	}
}
