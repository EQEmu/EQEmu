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
