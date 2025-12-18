#include "world/world_server_cli.h"

#include "common/events/player_events.h"
#include "common/memory/ksm.hpp"

#include "cereal/archives/json.hpp"
#include "cereal/types/vector.hpp"
#include <iomanip>

void WorldserverCLI::TestCommand(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Test command";

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	void* start_marker = KSM::MarkHeapStart();
	std::cout << "Start marker: " << start_marker << "\n";

	std::vector<std::string> vec = {};
	for (int i = 0; i < 100000; i++) {
		vec.push_back("Some random string");
	}

	// Measure allocated memory size
	size_t allocated_size = KSM::MeasureHeapUsage(start_marker);
	// Convert to MB as a float and output with precision
	double allocated_size_mb = static_cast<double>(allocated_size) / (1024 * 1024);
	std::cout << std::fixed << std::setprecision(3)
			  << "Allocated size: " << allocated_size_mb << " MB\n";

	// Mark memory for KSM
	KSM::MarkMemoryForKSM(start_marker, allocated_size);
}
