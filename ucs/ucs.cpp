/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2008 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

*/

#include "common/crash.h"
#include "common/discord/discord_manager.h"
#include "common/eqemu_logsys.h"
#include "common/event/event_loop.h"
#include "common/events/player_event_logs.h"
#include "common/net/servertalk_client_connection.h"
#include "common/net/tcp_server.h"
#include "common/opcodemgr.h"
#include "common/path_manager.h"
#include "common/platform.h"
#include "common/rulesys.h"
#include "common/servertalk.h"
#include "common/zone_store.h"
#include "ucs/chatchannel.h"
#include "ucs/clientlist.h"
#include "ucs/database.h"
#include "ucs/ucsconfig.h"
#include "ucs/worldserver.h"

#include <csignal>
#include <list>
#include <thread>

ChatChannelList *ChannelList;
Clientlist *g_Clientlist;
UCSDatabase database;
WorldServer *worldserver = nullptr;

const ucsconfig *Config;

std::string WorldShortName;

uint32 ChatMessagesSent = 0;
uint32 MailMessagesSent = 0;

std::string GetMailPrefix() {

	return "SOE.EQ." + WorldShortName + ".";

}

void crash_func() {
	std::this_thread::sleep_for(std::chrono::milliseconds(10000));
	int* p=0;
	*p=0;
}

void Shutdown() {
	LogInfo("Shutting down...");
	ChannelList->RemoveAllChannels();
	g_Clientlist->CloseAllConnections();
	EQEmuLogSys::Instance()->CloseFileLogs();
}

int caught_loop = 0;
void CatchSignal(int sig_num) {
	LogInfo("Caught signal [{}]", sig_num);

	EQ::EventLoop::Get().Shutdown();

	caught_loop++;
	// when signal handler is incapable of exiting properly
	if (caught_loop > 1) {
		LogInfo("In a signal handler loop and process is incapable of exiting properly, forcefully cleaning up");
		ChannelList->RemoveAllChannels();
		g_Clientlist->CloseAllConnections();
		EQEmuLogSys::Instance()->CloseFileLogs();
		std::exit(0);
	}
}

void PlayerEventQueueListener() {
	while (caught_loop == 0) {
		DiscordManager::Instance()->ProcessMessageQueue();
		Sleep(100);
	}
}

int main() {
	RegisterExecutablePlatform(ExePlatformUCS);
	EQEmuLogSys::Instance()->LoadLogSettingsDefaults();
	set_exception_handler();

	PathManager::Instance()->Init();

	// Check every minute for unused channels we can delete
	//
	Timer ChannelListProcessTimer(60000);
	Timer ClientConnectionPruneTimer(60000);

	Timer keepalive(INTERSERVER_TIMER); // does auto-reconnect

	LogInfo("Starting EQEmu Universal Chat Server");

	if (!ucsconfig::LoadConfig()) {
		LogInfo("Loading server configuration failed");
		return 1;
	}

	Config = ucsconfig::get();

	WorldShortName = Config->ShortName;

	LogInfo("Connecting to MySQL");

	if (!database.Connect(
		Config->DatabaseHost.c_str(),
		Config->DatabaseUsername.c_str(),
		Config->DatabasePassword.c_str(),
		Config->DatabaseDB.c_str(),
		Config->DatabasePort)) {
		LogInfo("Cannot continue without a database connection");
		return 1;
	}

	EQEmuLogSys::Instance()
		->SetDatabase(&database)
		->SetLogPath(PathManager::Instance()->GetLogPath())
		->LoadLogDatabaseSettings()
		->StartFileLogs();

	PlayerEventLogs::Instance()->SetDatabase(&database)->Init();

	char tmp[64];

	// ucs has no 'reload rules' handler
	if (database.GetVariable("RuleSet", tmp, sizeof(tmp)-1)) {
		LogInfo("Loading rule set [{}]", tmp);
		if(!RuleManager::Instance()->LoadRules(&database, tmp, false)) {
			LogInfo("Failed to load ruleset [{}], falling back to defaults", tmp);
		}
	} else {
		if(!RuleManager::Instance()->LoadRules(&database, "default", false)) {
			LogInfo("No rule set configured, using default rules");
		}
	}

	EQ::InitializeDynamicLookups();

	database.ExpireMail();

	g_Clientlist = new Clientlist(Config->GetUCSPort());

	ChannelList = new ChatChannelList();

	database.LoadChatChannels();

	std::signal(SIGINT, CatchSignal);
	std::signal(SIGTERM, CatchSignal);
	std::signal(SIGKILL, CatchSignal);
	std::signal(SIGSEGV, CatchSignal);

	std::thread(PlayerEventQueueListener).detach();

	worldserver = new WorldServer;

	// uncomment to simulate timed crash for catching SIGSEV
//	std::thread crash_test(crash_func);
//	crash_test.detach();

	auto loop_fn = [&](EQ::Timer* t) {
		if (keepalive.Check()) {
			keepalive.Start();
			database.ping();
		}

		Timer::SetCurrentTime();

		g_Clientlist->Process();

		if (ChannelListProcessTimer.Check()) {
			ChannelList->Process();
		}

		if (ClientConnectionPruneTimer.Check()) {
			g_Clientlist->CheckForStaleConnectionsAll();
		}

	};

	EQ::Timer process_timer(loop_fn);
	process_timer.Start(32, true);

	EQ::EventLoop::Get().Run();

	Shutdown();
}

void UpdateWindowTitle(char* iNewTitle) {
#ifdef _WINDOWS
		char tmp[500];
		if (iNewTitle) {
				snprintf(tmp, sizeof(tmp), "UCS: %s", iNewTitle);
		}
		else {
				snprintf(tmp, sizeof(tmp), "UCS");
		}
		SetConsoleTitle(tmp);
#endif
}
