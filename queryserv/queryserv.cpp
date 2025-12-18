
#include "common/crash.h"
#include "common/eqemu_logsys.h"
#include "common/event/event_loop.h"
#include "common/events/player_event_logs.h"
#include "common/net/console_server.h"
#include "common/net/servertalk_server.h"
#include "common/opcodemgr.h"
#include "common/platform.h"
#include "common/rulesys.h"
#include "common/strings.h"
#include "common/timer.h"
#include "common/zone_store.h"
#include "queryserv/database.h"
#include "queryserv/lfguild.h"
#include "queryserv/worldserver.h"
#include "queryserv/zonelist.h"
#include "queryserv/zoneserver.h"
#include "queryservconfig.h"

#include <list>
#include <signal.h>
#include <thread>

volatile bool RunLoops = true;

QSDatabase            qs_database;
Database              database;
std::string           WorldShortName;
const queryservconfig *Config;
WorldServer           *worldserver = 0;
uint32                numzones     = 0;

void CatchSignal(int sig_num)
{
	RunLoops = false;
}

int main()
{
	RegisterExecutablePlatform(ExePlatformQueryServ);
	EQEmuLogSys::Instance()->LoadLogSettingsDefaults();
	set_exception_handler();
	Timer LFGuildExpireTimer(60000);

	PathManager::Instance()->Init();

	LogInfo("Starting EQEmu QueryServ");
	if (!queryservconfig::LoadConfig()) {
		LogInfo("Loading server configuration failed");
		return 1;
	}

	Config         = queryservconfig::get();
	WorldShortName = Config->ShortName;

	LogInfo("Connecting to MySQL");

	/* MySQL Connection */
	if (!qs_database.Connect(
		Config->QSDatabaseHost.c_str(),
		Config->QSDatabaseUsername.c_str(),
		Config->QSDatabasePassword.c_str(),
		Config->QSDatabaseDB.c_str(),
		Config->QSDatabasePort
	)) {
		LogInfo("Cannot continue without a qs database connection");
		return 1;
	}
	if (!database.Connect(
		Config->DatabaseHost.c_str(),
		Config->DatabaseUsername.c_str(),
		Config->DatabasePassword.c_str(),
		Config->DatabaseDB.c_str(),
		Config->DatabasePort
	)) {
		LogInfo("Cannot continue without a database connection");
		return 1;
	}

	EQEmuLogSys::Instance()->SetDatabase(&database)
		->SetLogPath(PathManager::Instance()->GetLogPath())
		->LoadLogDatabaseSettings()
		->StartFileLogs();

	if (signal(SIGINT, CatchSignal) == SIG_ERR) {
		LogInfo("Could not set signal handler");
		return 1;
	}
	if (signal(SIGTERM, CatchSignal) == SIG_ERR) {
		LogInfo("Could not set signal handler");
		return 1;
	}

	//rules:
	{
		std::string tmp;
		if (database.GetVariable("RuleSet", tmp)) {
			LogInfo("Loading rule set [{}]", tmp.c_str());
			if (!RuleManager::Instance()->LoadRules(&database, tmp.c_str(), false)) {
				LogError("Failed to load ruleset [{}], falling back to defaults", tmp.c_str());
			}
		}
		else {
			if (!RuleManager::Instance()->LoadRules(&database, "default", false)) {
				LogInfo("No rule set configured, using default rules");
			}
		}

		EQ::InitializeDynamicLookups();
	}

	std::unique_ptr<EQ::Net::ConsoleServer> console;
	EQ::Net::ServertalkServerOptions        server_opts;
	auto                                    server_connection = std::make_unique<EQ::Net::ServertalkServer>();
	server_opts.port        = Config->QSPort;
	server_opts.ipv6        = false;
	server_opts.credentials = Config->SharedKey;
	server_connection->Listen(server_opts);
	LogInfo("Server (TCP) listener started on port [{}]", Config->QSPort);

	server_connection->OnConnectionIdentified(
		"Zone", [&console](std::shared_ptr<EQ::Net::ServertalkServerConnection> connection) {
			numzones++;
			ZSList::Instance()->Add(new ZoneServer(connection, console.get()));

			LogInfo(
				"New Zone Server connection from [{}] at [{}:{}] zone_count [{}]",
				connection->Handle()->RemoteIP(),
				connection->Handle()->RemotePort(),
				connection->GetUUID(),
				numzones
			);
		}
	);

	server_connection->OnConnectionRemoved(
		"Zone", [](std::shared_ptr<EQ::Net::ServertalkServerConnection> connection) {
			numzones--;
			ZSList::Instance()->Remove(connection->GetUUID());

			LogInfo(
				"Removed Zone Server connection from [{}] total zone_count [{}]",
				connection->GetUUID(),
				numzones
			);
		}
	);

	/* Initial Connection to Worldserver */
	worldserver = new WorldServer;
	worldserver->Connect();

	/* Load Looking For Guild Manager */
	LFGuildManager::Instance()->LoadDatabase();

	Timer player_event_process_timer(1000);
	PlayerEventLogs::Instance()->SetDatabase(&qs_database)->Init();

	auto loop_fn = [&](EQ::Timer *t) {
		Timer::SetCurrentTime();

		if (!RunLoops) {
			EQ::EventLoop::Get().Shutdown();
			return;
		}

		if (LFGuildExpireTimer.Check()) {
			LFGuildManager::Instance()->ExpireEntries();
		}

		if (player_event_process_timer.Check()) {
			PlayerEventLogs::Instance()->Process();
		}
	};

	EQ::Timer process_timer(loop_fn);
	process_timer.Start(32, true);

	EQ::EventLoop::Get().Run();

	safe_delete(worldserver);
	EQEmuLogSys::Instance()->CloseFileLogs();
}

void UpdateWindowTitle(char *iNewTitle)
{
#ifdef _WINDOWS
	char tmp[500];
	if (iNewTitle) {
		snprintf(tmp, sizeof(tmp), "QueryServ: %s", iNewTitle);
	}
	else {
		snprintf(tmp, sizeof(tmp), "QueryServ");
	}
	SetConsoleTitle(tmp);
#endif
}
