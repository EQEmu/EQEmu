#include "loginserver_command_handler.h"

#include "common/repositories/login_api_tokens_repository.h"
#include "common/util/uuid.h"
#include "loginserver/account_management.h"
#include "loginserver/login_server.h"
#include "loginserver/loginserver_webserver.h"

extern LoginServer server;

namespace LoginserverCommandHandler {
	void CommandHandler(int argc, char **argv)
	{
		if (argc == 1) { return; }

		argh::parser cmd;
		cmd.parse(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);
		EQEmuCommand::DisplayDebug(cmd);

		auto function_map = EQEmuCommand::function_map;
		function_map["login-user:check-credentials"]          = &LoginserverCommandHandler::CheckLoginserverUserCredentials;
		function_map["login-user:check-external-credentials"] = &LoginserverCommandHandler::CheckExternalLoginserverUserCredentials;
		function_map["login-user:create"]                     = &LoginserverCommandHandler::CreateLocalLoginserverAccount;
		function_map["login-user:update-credentials"]         = &LoginserverCommandHandler::UpdateLoginserverUserCredentials;
		function_map["web-api-token:create"]                  = &LoginserverCommandHandler::CreateLoginserverApiToken;
		function_map["web-api-token:list"]                    = &LoginserverCommandHandler::ListLoginserverApiTokens;
		function_map["world-admin:create"]                    = &LoginserverCommandHandler::CreateLoginserverWorldAdminAccount;
		function_map["world-admin:update"]                    = &LoginserverCommandHandler::UpdateLoginserverWorldAdminAccountPassword;
		function_map["health:check-login"]                    = &LoginserverCommandHandler::HealthCheckLogin;

		EQEmuCommand::HandleMenu(function_map, cmd, argc, argv);
	}

	void CreateLoginserverApiToken(int argc, char **argv, argh::parser &cmd, std::string &description)
	{
		description = "Creates Loginserver API Token";

		if (cmd[{"-h", "--help"}]) {
			return;
		}

		std::vector<std::string> arguments = {};
		std::vector<std::string> options   = {
			"--read",
			"--write"
		};

		EQEmuCommand::ValidateCmdInput(arguments, options, cmd, argc, argv);

		bool can_read  = cmd[{"-r", "--read"}];
		bool can_write = cmd[{"-w", "--write"}];

		if (!can_read || !can_write) {
			LogInfo("--read or --write must be set or both!");
			exit(1);
		}

		auto t = LoginApiTokensRepository::NewEntity();
		t.can_read   = can_read;
		t.can_write  = can_write;
		t.token      = EQ::Util::UUID::Generate().ToString();
		t.created_at = std::time(nullptr);

		auto created = LoginApiTokensRepository::InsertOne(database, t);
		if (created.id) {
			LogInfo("Created Loginserver API token [{}] [{}]", created.id, created.token);
		}
	}

	void ListLoginserverApiTokens(int argc, char **argv, argh::parser &cmd, std::string &description)
	{
		description = "Lists Loginserver API Tokens";

		if (cmd[{"-h", "--help"}]) {
			return;
		}

		server.token_manager = new LoginserverWebserver::TokenManager;
		server.token_manager->LoadApiTokens();

		for (auto &it: server.token_manager->loaded_api_tokens) {
			LogInfo(
				"token id [{}] [{}] can_write [{}] can_read [{}]",
				it.second.id,
				it.second.token,
				it.second.can_write,
				it.second.can_read
			);
		}
	}

	void CreateLocalLoginserverAccount(int argc, char **argv, argh::parser &cmd, std::string &description)
	{
		description = "Creates Local Loginserver Account";

		std::vector<std::string> arguments = {
			"{username}",
			"{password}"
		};
		std::vector<std::string> options   = {
			"--email=*"
		};

		if (cmd[{"-h", "--help"}]) {
			return;
		}

		EQEmuCommand::ValidateCmdInput(arguments, options, cmd, argc, argv);

		LoginAccountContext c;
		c.username = cmd(2).str();
		c.password = cmd(3).str();
		c.email    = cmd("--email").str();

		AccountManagement::CreateLoginServerAccount(c);
	}

	void CreateLoginserverWorldAdminAccount(int argc, char **argv, argh::parser &cmd, std::string &description)
	{
		description = "Creates Loginserver World Administrator Account";

		std::vector<std::string> arguments = {
			"{username}",
			"{password}",
			"{email}"
		};
		std::vector<std::string> options   = {};

		if (cmd[{"-h", "--help"}]) {
			return;
		}

		EQEmuCommand::ValidateCmdInput(arguments, options, cmd, argc, argv);

		AccountManagement::CreateLoginserverWorldAdminAccount(
			cmd(2).str(),
			cmd(3).str(),
			cmd(4).str()
		);
	}

	void CheckLoginserverUserCredentials(int argc, char **argv, argh::parser &cmd, std::string &description)
	{
		description = "Check user login credentials";

		std::vector<std::string> arguments = {
			"{username}",
			"{password}"
		};
		std::vector<std::string> options   = {};

		if (cmd[{"-h", "--help"}]) {
			return;
		}

		EQEmuCommand::ValidateCmdInput(arguments, options, cmd, argc, argv);

		LoginAccountContext c;
		c.username = cmd(2).str();
		c.password = cmd(3).str();

		auto res = AccountManagement::CheckLoginserverUserCredentials(c);

		LogInfo("Credentials were {}", res != 0 ? "accepted" : "not accepted");
	}

	void UpdateLoginserverUserCredentials(int argc, char **argv, argh::parser &cmd, std::string &description)
	{
		description = "Change user login credentials";

		std::vector<std::string> arguments = {
			"{username}",
			"{password}"
		};
		std::vector<std::string> options   = {};

		if (cmd[{"-h", "--help"}]) {
			return;
		}

		EQEmuCommand::ValidateCmdInput(arguments, options, cmd, argc, argv);

		LoginAccountContext c;
		c.username = cmd(2).str();
		c.password = cmd(3).str();
		AccountManagement::UpdateLoginserverUserCredentials(c);
	}

	void CheckExternalLoginserverUserCredentials(int argc, char **argv, argh::parser &cmd, std::string &description)
	{
		description = "Check user external login credentials";

		std::vector<std::string> arguments = {
			"{username}",
			"{password}"
		};
		std::vector<std::string> options   = {};

		if (cmd[{"-h", "--help"}]) {
			return;
		}

		EQEmuCommand::ValidateCmdInput(arguments, options, cmd, argc, argv);

		LoginAccountContext c;
		c.username = cmd(2).str();
		c.password = cmd(3).str();
		auto res = AccountManagement::CheckExternalLoginserverUserCredentials(c);

		LogInfo("Credentials were {}", res ? "accepted" : "not accepted");
	}

	void UpdateLoginserverWorldAdminAccountPassword(int argc, char **argv, argh::parser &cmd, std::string &description)
	{
		description = "Update world admin account password";

		std::vector<std::string> arguments = {
			"{username}",
			"{password}"
		};
		std::vector<std::string> options   = {};

		if (cmd[{"-h", "--help"}]) {
			return;
		}

		EQEmuCommand::ValidateCmdInput(arguments, options, cmd, argc, argv);

		LoginAccountContext c;
		c.username = cmd(2).str();
		c.password = cmd(3).str();
		AccountManagement::UpdateLoginserverWorldAdminAccountPasswordByName(c);
	}

	void HealthCheckLogin(int argc, char **argv, argh::parser &cmd, std::string &description)
	{
		description = "Checks login health using a test user";

		std::vector<std::string> arguments = {};
		std::vector<std::string> options   = {};

		if (cmd[{"-h", "--help"}]) {
			return;
		}

		LogInfo("[CLI] [HealthCheck] Response code [{}]", AccountManagement::HealthCheckUserLogin());
	}
}
