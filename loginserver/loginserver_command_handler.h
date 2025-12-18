#pragma once

#include "common/cli/eqemu_command_handler.h"

namespace LoginserverCommandHandler {
	void CommandHandler(int argc, char **argv);
	void CreateLoginserverApiToken(int argc, char **argv, argh::parser &cmd, std::string &description);
	void ListLoginserverApiTokens(int argc, char **argv, argh::parser &cmd, std::string &description);
	void CreateLocalLoginserverAccount(int argc, char **argv, argh::parser &cmd, std::string &description);
	void CreateLoginserverWorldAdminAccount(int argc, char **argv, argh::parser &cmd, std::string &description);
	void CheckLoginserverUserCredentials(int argc, char **argv, argh::parser &cmd, std::string &description);
	void UpdateLoginserverUserCredentials(int argc, char **argv, argh::parser &cmd, std::string &description);
	void CheckExternalLoginserverUserCredentials(int argc, char **argv, argh::parser &cmd, std::string &description);
	void UpdateLoginserverWorldAdminAccountPassword(int argc, char **argv, argh::parser &cmd, std::string &description);
	void HealthCheckLogin(int argc, char **argv, argh::parser &cmd, std::string &description);
};
