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

#include "common/types.h"
#include "world/worlddb.h"

#include <map>
#include <string>

class LauncherLink;

typedef struct {
	std::string name;
	uint16 port;
} LauncherZone;

//a class exported to perl representing a launcher's in-DB config
class EQLConfig {
public:
	EQLConfig(const char *launcher_name);

	void LoadSettings();
	static EQLConfig *CreateLauncher(const char *name, uint8 dynamic_count);

	void GetZones(std::vector<LauncherZone> &result);

//BEGIN PERL EXPORT
	Const_char * GetName() const { return(m_name.c_str()); }
	void StartZone(Const_char *zone_ref);
	int GetDynamicCount() const;
//END PERL EXPORT

protected:
	const std::string m_name;

	uint8 m_dynamics;

	std::map<std::string, LauncherZone> m_zones;	//static zones.
};
