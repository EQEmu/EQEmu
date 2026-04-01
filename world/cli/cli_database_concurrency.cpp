/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "world/world_server_cli.h"

#include "common/eqemu_config.h"
#include "common/repositories/zone_repository.h"

#include <csignal>
#include <thread>

Database db;
Database db2;

volatile sig_atomic_t stop;
void inthand(int signum) {
	stop = 1;
}

[[noreturn]] void DatabaseTest()
{
	while (true) {
		LogInfo("DatabaseTest Query");
		db.QueryDatabase("SELECT 1");
	}
}

[[noreturn]] void DatabaseTestSecondConnection()
{
	while (true) {
		LogInfo("DatabaseTest Query");
		db2.QueryDatabase("SELECT 1");
	}
}


void WorldserverCLI::TestDatabaseConcurrency(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Test command to test database concurrency";

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	signal(SIGINT, inthand);

	LogInfo("Database test");

	auto mutex = new Mutex;

	auto c = EQEmuConfig::get();
	LogInfo("Connecting to MySQL");
	if (!db.Connect(
		c->DatabaseHost.c_str(),
		c->DatabaseUsername.c_str(),
		c->DatabasePassword.c_str(),
		c->DatabaseDB.c_str(),
		c->DatabasePort
	)) {
		LogError("Cannot continue without a database connection");
		return;
	}

	db.SetMutex(mutex);

	db2.SetMySQL(db);

	db2.SetMutex(mutex);

	std::thread(DatabaseTest).detach();
	std::thread(DatabaseTest).detach();
	std::thread(DatabaseTestSecondConnection).detach();

	while (!stop) {

	}

	safe_delete(mutex);
}
