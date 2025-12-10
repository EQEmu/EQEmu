#include "../world_server_cli.h"
#include "../../common/version.h"
#include "../../common/json/json.h"
#include "../../common/rulesys.h"

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

	std::stringstream payload;
	payload << v;

	std::cout << payload.str() << std::endl;
}
