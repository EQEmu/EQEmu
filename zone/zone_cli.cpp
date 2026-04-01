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
#include "zone_cli.h"

#include "common/cli/eqemu_command_handler.h"

#include <cstring>

bool ZoneCLI::RanConsoleCommand(int argc, char **argv)
{
	return argc > 1 && (strstr(argv[1], ":") != nullptr || strstr(argv[1], "--") != nullptr);
}

bool ZoneCLI::RanSidecarCommand(int argc, char **argv)
{
	return argc > 1 && (strstr(argv[1], "sidecar:") != nullptr);
}

bool ZoneCLI::RanTestCommand(int argc, char **argv)
{
	return argc > 1 && (strstr(argv[1], "tests:") != nullptr);
}

void ZoneCLI::CommandHandler(int argc, char **argv)
{
	if (argc == 1) { return; }

	argh::parser cmd;
	cmd.parse(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);
	EQEmuCommand::DisplayDebug(cmd);

	// Declare command mapping
	auto function_map = EQEmuCommand::function_map;

	// Register commands
	function_map["benchmark:databuckets"]        = &ZoneCLI::BenchmarkDatabuckets;
	function_map["sidecar:serve-http"]           = &ZoneCLI::SidecarServeHttp;
	function_map["tests:databuckets"]            = &ZoneCLI::TestDataBuckets;
	function_map["tests:npc-handins"]            = &ZoneCLI::TestNpcHandins;
	function_map["tests:npc-handins-multiquest"] = &ZoneCLI::TestNpcHandinsMultiQuest;
	function_map["tests:zone-state"]             = &ZoneCLI::TestZoneState;

	EQEmuCommand::HandleMenu(function_map, cmd, argc, argv);
}

