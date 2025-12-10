#include "../world_server_cli.h"
#include "../../common/json/json.h"
#include "../../common/version.h"

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

	std::stringstream payload;
	payload << j;

	std::cout << payload.str() << std::endl;
}
