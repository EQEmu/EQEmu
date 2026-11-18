#pragma once

#include "common/multi_world_selector.h"

#include <string>

namespace EQ::Net::MultiWorldSelector {

inline std::string ResolveConfigPath(int argc, char *const argv[])
{
	return argc > 1 ? argv[1] : ConfigFileName;
}

} // namespace EQ::Net::MultiWorldSelector
