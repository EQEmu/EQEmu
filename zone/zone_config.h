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
#pragma once

#include "common/eqemu_config.h"

class ZoneConfig : public EQEmuConfig {
	public:
		uint16 ZonePort;
		std::string ZoneAddress;

	private:

	static ZoneConfig *_zone_config;

	ZoneConfig() : EQEmuConfig() {
		ZonePort=0;
	}

	public:

	// Produce a const singleton
	static const ZoneConfig *get() {
		if (_zone_config == nullptr)
			LoadConfig();
		return(_zone_config);
	}

	// Load the config
	static bool LoadConfig(const std::string &path = "")
	{
		safe_delete(_zone_config);
		safe_delete(_config);

		_zone_config = new ZoneConfig;
		_config      = _zone_config;

		return _config->parseFile(path);
	}

	// Accessors for the static private object
	static void SetZonePort(uint16 port) { if (_zone_config) _zone_config->ZonePort=port; }

	void Dump() const;
};
