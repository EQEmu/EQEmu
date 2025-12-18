/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

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
