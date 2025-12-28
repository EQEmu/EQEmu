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

#include "common/events/player_events.h"
#include "common/memory/ksm.h"

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
