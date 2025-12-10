#include "../../zone.h"

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
