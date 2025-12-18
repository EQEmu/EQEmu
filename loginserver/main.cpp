#include "common/crash.h"
#include "common/database.h"
#include "common/eqemu_logsys.h"
#include "common/event/event_loop.h"
#include "common/event/timer.h"
#include "common/events/player_event_logs.h"
#include "common/http/httplib.h"
#include "common/path_manager.h"
#include "common/platform.h"
#include "common/timer.h"
#include "common/types.h"
#include "loginserver/login_server.h"
#include "loginserver/loginserver_command_handler.h"
#include "loginserver/loginserver_webserver.h"

#include <string>
#include <thread>

LoginServer     server;
bool            run_server = true;
Database        database;

void LoadDatabaseConnection()
{
	LogInfo("MySQL Database Init");

	if (!database.Connect(
		server.config.GetVariableString("database", "host", "localhost"),
		server.config.GetVariableString("database", "user", "root"),
		server.config.GetVariableString("database", "password", ""),
		server.config.GetVariableString("database", "db", "peq"),
		server.config.GetVariableInt("database", "port", 3306)
	)) {
		LogError("Cannot continue without a database connection");
		std::exit(1);
	}
}

void LoadServerConfig()
{
	server.config = EQ::JsonConfigFile::Load(
		fmt::format("{}/login.json", PathManager::Instance()->GetServerPath())
	);
	LogInfo("Config System Init");

	server.options.RejectDuplicateServers(
		server.config.GetVariableBool(
			"worldservers",
			"reject_duplicate_servers",
			false
		)
	);
	server.options.SetShowPlayerCount(server.config.GetVariableBool("worldservers", "show_player_count", false));
	server.options.AllowUnregistered(
		server.config.GetVariableBool(
			"worldservers",
			"unregistered_allowed",
			true
		)
	);
	server.options.SetWorldDevTestServersListBottom(
		server.config.GetVariableBool(
			"worldservers",
			"dev_test_servers_list_bottom",
			false
		)
	);
	server.options.SetWorldSpecialCharacterStartListBottom(
		server.config.GetVariableBool(
			"worldservers",
			"special_character_start_list_bottom",
			false
		)
	);

	server.options.DisplayExpansions(
		server.config.GetVariableBool(
			"client_configuration",
			"display_expansions",
			false
		)
	);
	server.options.MaxExpansions(
		server.config.GetVariableInt(
			"client_configuration",
			"max_expansions_mask",
			67108863
		)
	);

	server.options.AutoCreateAccounts(server.config.GetVariableBool("account", "auto_create_accounts", true));

	if (std::getenv("LSPX")) {
		server.options.EQEmuLoginServerAddress(
			server.config.GetVariableString(
				"general",
				"eqemu_loginserver_address",
				"login.eqemulator.net:5999"
			)
		);
	}

	server.options.DefaultLoginServerName(
		server.config.GetVariableString(
			"general",
			"default_loginserver_name",
			"local"
		)
	);

#ifdef ENABLE_SECURITY
	server.options.EncryptionMode(server.config.GetVariableInt("security", "mode", 13));
#else
	server.options.EncryptionMode(server.config.GetVariableInt("security", "mode", 6));
#endif

	server.options.AllowTokenLogin(server.config.GetVariableBool("security", "allow_token_login", false));
}

void start_web_server()
{
	Sleep(1);

	int web_api_port = server.config.GetVariableInt("web_api", "port", 6000);
	LogInfo("Webserver API now listening on port [{}]", web_api_port);

	httplib::Server api;

	api.set_logger(
		[](const auto &req, const auto &res) {
			if (!req.path.empty()) {
				LogInfo("[API] Request [{}] via [{}:{}]", req.path, req.remote_addr, req.remote_port);
			}
		}
	);

	LoginserverWebserver::RegisterRoutes(api);
	api.listen("0.0.0.0", web_api_port);
}

int main(int argc, char **argv)
{
	RegisterExecutablePlatform(ExePlatformLogin);
	set_exception_handler();

	LogInfo("Logging System Init");

	if (argc == 1) {
		EQEmuLogSys::Instance()->LoadLogSettingsDefaults();
	}

	PathManager::Instance()->Init();

	// command handler
	if (argc > 1) {
		EQEmuLogSys::Instance()->SilenceConsoleLogging();

		LoadServerConfig();
		LoadDatabaseConnection();

		EQEmuLogSys::Instance()->LoadLogSettingsDefaults();
		EQEmuLogSys::Instance()->log_settings[Logs::Debug].log_to_console = static_cast<uint8>(Logs::General);
		EQEmuLogSys::Instance()->log_settings[Logs::Debug].is_category_enabled = 1;

		LoginserverCommandHandler::CommandHandler(argc, argv);
	}

	LoadServerConfig();
	LoadDatabaseConnection();

	if (argc == 1) {
		EQEmuLogSys::Instance()->SetDatabase(&database)
			->SetLogPath("logs")
			->LoadLogDatabaseSettings()
			->StartFileLogs();
	}

	LogInfo("Server Manager Init");
	server.server_manager = new WorldServerManager();
	if (!server.server_manager) {
		LogError("Server Manager Failed to Start");
		LogInfo("Database System Shutdown");
		return 1;
	}

	LogInfo("Client Manager Init");
	server.client_manager = new ClientManager();
	if (!server.client_manager) {
		LogError("Client Manager Failed to Start");
		LogInfo("Server Manager Shutdown");
		delete server.server_manager;

		LogInfo("Database System Shutdown");
		return 1;
	}

#ifdef WIN32
#ifdef UNICODE
		SetConsoleTitle(L"EQEmu Login Server");
#else
		SetConsoleTitle("EQEmu Login Server");
#endif
#endif

	LogInfo("Server Started");

	bool web_api_enabled = server.config.GetVariableBool("web_api", "enabled", true);
	if (web_api_enabled) {
		std::thread web_api_thread(start_web_server);
		web_api_thread.detach();
	}

	LogInfo("[Config] [Account] CanAutoCreateAccounts [{}]", server.options.CanAutoCreateAccounts());
	LogInfo("[Config] [ClientConfiguration] DisplayExpansions [{}]", server.options.IsDisplayExpansions());
	LogInfo("[Config] [ClientConfiguration] MaxExpansions [{}]", server.options.GetMaxExpansions());

	if (std::getenv("LSPX")) {
		LogInfo("[Config] [Account] LSPX [on]");
	}

	LogInfo("[Config] [WorldServer] IsRejectingDuplicateServers [{}]", server.options.IsRejectingDuplicateServers());
	LogInfo("[Config] [WorldServer] IsUnregisteredAllowed [{}]", server.options.IsUnregisteredAllowed());
	LogInfo("[Config] [WorldServer] ShowPlayerCount [{}]", server.options.IsShowPlayerCountEnabled());
	LogInfo(
		"[Config] [WorldServer] DevAndTestServersListBottom [{}]",
		server.options.IsWorldDevTestServersListBottom()
	);
	LogInfo(
		"[Config] [WorldServer] SpecialCharactersStartListBottom [{}]",
		server.options.IsWorldSpecialCharacterStartListBottom()
	);
	LogInfo("[Config] [Security] GetEncryptionMode [{}]", server.options.GetEncryptionMode());
	LogInfo("[Config] [Security] IsTokenLoginAllowed [{}]", server.options.IsTokenLoginAllowed());

	Timer keepalive(INTERSERVER_TIMER); // does auto-reconnect

	auto loop_fn = [&](EQ::Timer* t) {
		Timer::SetCurrentTime();

		if (keepalive.Check()) {
			keepalive.Start();
			database.ping();
		}

		if (!run_server) {
			EQ::EventLoop::Get().Shutdown();
			return;
		}

		server.client_manager->Process();
	};

	EQ::Timer process_timer(loop_fn);
	process_timer.Start(32, true);

	EQ::EventLoop::Get().Run();

	LogInfo("Server Shutdown");

	LogInfo("Client Manager Shutdown");
	delete server.client_manager;

	LogInfo("Server Manager Shutdown");
	delete server.server_manager;

	return 0;
}
