/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.
*/
#pragma once

#include "common/multi_world_selector.h"

#include <string>

namespace EQ::Net::MultiWorldSelector {

// Best-effort by design: selector failure must never block the normal
// loginserver-to-World authorization path.
bool Notify(const Config &config, const ControlSelection &selection, std::string &error);

} // namespace EQ::Net::MultiWorldSelector
