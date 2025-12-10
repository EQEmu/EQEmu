#include "../zone_cli.h"
#include "../../common/cli/eqemu_command_handler.h"
#include "../../common/http/httplib.h"
#include "../../common/eqemu_logsys.h"
#include "../sidecar_api/sidecar_api.h"
#include "../../common/platform.h"

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
