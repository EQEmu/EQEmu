#include "../world_server_cli.h"
#include "../../common/eqemu_logsys.h"
#include "../../common/eqemu_logsys_log_aliases.h"
#include "../worlddb.h"

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
