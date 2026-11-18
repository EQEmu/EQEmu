/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.
*/
#include "common/multi_world_selector.h"
#include "world_selector/selector_config_path.h"
#include "world_selector/world_selector_server.h"

#include <iostream>
#include <string>

int main(int argc, char **argv)
{
	const auto config_path = EQ::Net::MultiWorldSelector::ResolveConfigPath(argc, argv);
	EQ::Net::MultiWorldSelector::Config config;
	std::string error;
	if (EQ::Net::MultiWorldSelector::LoadConfigFile(config_path, config, error) !=
		EQ::Net::MultiWorldSelector::ConfigFileLoadStatus::Loaded) {
		std::cerr << error << '\n';
		return 1;
	}

	if (!config.enabled) {
		std::cout << "Multi-World Selector is disabled; nothing to do\n";
		return 0;
	}

	EQ::Net::MultiWorldSelector::WorldSelectorServer server(std::move(config));
	if (!server.Start(error)) {
		std::cerr << "Unable to start Multi-World Selector: " << error << '\n';
		return 1;
	}

	return server.Run();
}
