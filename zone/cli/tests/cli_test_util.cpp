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
#include "zone/zone.h"

void RunTest(const std::string &test_name, const std::string &expected, const std::string &actual)
{
	if (expected == actual) {
		std::cout << "[✅] " << test_name << " PASSED\n";
	} else {
		std::cerr << "[❌] " << test_name << " FAILED\n";
		std::cerr << "   📌 Expected: " << expected << "\n";
		std::cerr << "   ❌ Got:      " << actual << "\n";
		std::exit(1);
	}
}

void RunTest(const std::string &test_name, bool expected, bool actual)
{
	if (expected == actual) {
		std::cout << "[✅] " << test_name << " PASSED\n";
	}
	else {
		std::cerr << "[❌] " << test_name << " FAILED\n";
		std::cerr << "   📌 Expected: " << (expected ? "true" : "false") << "\n";
		std::cerr << "   ❌ Got:      " << (actual ? "true" : "false") << "\n";
		std::exit(1);
	}
}

void RunTest(const std::string &test_name, int expected, int actual)
{
	if (expected == actual) {
		std::cout << "[✅] " << test_name << " PASSED\n";
	}
	else {
		std::cerr << "[❌] " << test_name << " FAILED\n";
		std::cerr << "   📌 Expected: " << expected << "\n";
		std::cerr << "   ❌ Got:      " << actual << "\n";
		std::exit(1);
	}
}

extern Zone *zone;

void SetupZone(std::string zone_short_name, uint32 instance_id = 0) {
	EQEmuLogSys::Instance()->SilenceConsoleLogging();

	EQEmuLogSys::Instance()->log_settings[Logs::ZoneState].log_to_console = std::getenv("DEBUG") ? 3 : 0;
	EQEmuLogSys::Instance()->log_settings[Logs::Info].log_to_console = std::getenv("DEBUG") ? 3 : 0;
	EQEmuLogSys::Instance()->log_settings[Logs::Spawns].log_to_console = std::getenv("DEBUG") ? 3 : 0;

	// boot shell zone for testing
	Zone::Bootup(ZoneID(zone_short_name), 0, false);
	zone->StopShutdownTimer();
	entity_list.Process();
	entity_list.MobProcess();

	EQEmuLogSys::Instance()->EnableConsoleLogging();
}
