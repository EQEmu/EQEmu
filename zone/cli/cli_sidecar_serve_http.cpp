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

#include "zone/zone_cli.h"

#include "zone/sidecar_api/sidecar_api.h"
#include "common/cli/eqemu_command_handler.h"
#include "common/eqemu_logsys.h"
#include "common/http/httplib.h"
#include "common/platform.h"

void ZoneCLI::SidecarServeHttp(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	if (cmd[{"-h", "--help"}]) {
		return;
	}

	RegisterExecutablePlatform(EQEmuExePlatform::ExePlatformZoneSidecar);

	int port = 0;
	std::string key;
	if (!cmd("--port").str().empty()) {
		port = strtoll(cmd("--port").str().c_str(), nullptr, 10);
	}
	if (!cmd("--key").str().empty()) {
		key = cmd("--key").str();
	}

	SidecarApi::BootWebserver(port, key);
}
