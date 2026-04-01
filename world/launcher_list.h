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

#include "common/types.h"
#include "common/net/servertalk_server_connection.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

class LauncherLink;
class EQLConfig;

class LauncherList {
public:
	LauncherList();
	~LauncherList();

	void Process();

	void LoadList();
	EQLConfig *GetConfig(const char *name);
	void Remove(const char *name);

	void Add(std::shared_ptr<EQ::Net::ServertalkServerConnection> conn);
	void Remove(std::shared_ptr<EQ::Net::ServertalkServerConnection> conn);
	LauncherLink *Get(const char *name);

	int GetLauncherCount();
protected:
	std::map<std::string, EQLConfig *> m_configs;	//we own these objects
	std::map<std::string, LauncherLink *> m_launchers;	//we own these objects
	std::vector<LauncherLink *> m_pendingLaunchers;	//we own these objects, have not yet identified themself
	int nextID;
};
