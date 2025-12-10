#include "../world_server_cli.h"
#include "../worlddb.h"

void WorldserverCLI::DatabaseSetAccountStatus(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Sets account status by account name";

	std::vector<std::string> arguments = {
		"{name}",
		"{status}"
	};

	std::vector<std::string> options = {};

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	EQEmuCommand::ValidateCmdInput(arguments, options, cmd, argc, argv);

	database.SetAccountStatus(
		cmd(2).str(),
		Strings::ToInt(cmd(3).str())
	);
}
