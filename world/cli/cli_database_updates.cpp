#include "../world_server_cli.h"
#include "../worlddb.h"
#include "../../common/database/database_update.h"

void WorldserverCLI::DatabaseUpdates(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Runs database updates manually";

	std::vector<std::string> options   = {
		"--skip-backup",
	};

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	DatabaseUpdate update;
	update.SetDatabase(&database)
		->SetSkipBackup(cmd[{"--skip-backup"}] )
		->SetContentDatabase(&content_db)
		->CheckDbUpdates();
}
