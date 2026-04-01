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

class WorldConfig : public EQEmuConfig {
public:
	virtual std::string GetByName(const std::string &var_name) const;

	bool UpdateStats;
	bool LoginDisabled;

private:

	static WorldConfig *_world_config;

	WorldConfig() : EQEmuConfig() {
		LoginDisabled=false;
		UpdateStats=true;
	}

public:

	// Produce a const singleton
	static const WorldConfig *get() {
		if (_world_config == nullptr)
			LoadConfig();
		return(_world_config);
	}

	// Load the config
	static bool LoadConfig(const std::string& path = "") {
		if (_world_config != nullptr)
			delete _world_config;
		_world_config=new WorldConfig;
		_config=_world_config;

		return _config->parseFile(path);
	}

	// Accessors for the static private object
	static void LockWorld() { if (_world_config) _world_config->Locked=true; }
	static void UnlockWorld() { if (_world_config) _world_config->Locked=false; }

	static void SetWorldAddress(const std::string& addr) { if (_world_config) _world_config->WorldAddress=addr; }
	static void SetLocalAddress(const std::string& addr) { if (_world_config) _world_config->LocalAddress=addr; }

	void Dump() const;
};
