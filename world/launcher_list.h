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
