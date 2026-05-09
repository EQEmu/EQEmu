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

#include "common/eqemu_logsys_log_aliases.h"
#include "common/eqemu_logsys.h"
#include "world/worlddb.h"

void WorldserverCLI::CopyCharacter(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Copies a character into a destination account";

	std::vector<std::string> arguments = {
		"source_character_name",
		"destination_character_name",
		"destination_account_name"
	};
	std::vector<std::string> options   = {};
	
	if (cmd[{"-h", "--help"}]) {
		return;
	}

	EQEmuCommand::ValidateCmdInput(arguments, options, cmd, argc, argv);

	std::string source_character_name      = cmd(2).str();
	std::string destination_character_name = cmd(3).str();
	std::string destination_account_name   = cmd(4).str();

	LogInfo(
		"Attempting to copy character [{}] to [{}] via account [{}]",
		source_character_name,
		destination_character_name,
		destination_account_name
	);

	database.CopyCharacter(
		source_character_name,
		destination_character_name,
		destination_account_name
	);
}
