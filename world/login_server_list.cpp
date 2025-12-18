/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

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

#include "login_server_list.h"

#include "common/eq_packet_structs.h"
#include "common/packet_dump.h"
#include "common/servertalk.h"
#include "common/version.h"
#include "world/clientlist.h"
#include "world/login_server.h"
#include "world/world_config.h"
#include "world/worlddb.h"
#include "world/zonelist.h"
#include "world/zoneserver.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#define IGNORE_LS_FATAL_ERROR

extern uint32 numzones;
extern uint32 numplayers;
extern volatile bool	RunLoops;

LoginServerList::LoginServerList() {
}

LoginServerList::~LoginServerList() {
}

void LoginServerList::Add(const char* iAddress, uint16 iPort, const char* Account, const char* Password, bool Legacy)
{
	auto loginserver = new LoginServer(iAddress, iPort, Account, Password, Legacy);
	m_list.emplace_back(std::unique_ptr<LoginServer>(loginserver));
}

bool LoginServerList::SendStatus() {
	for (auto &iter : m_list) {
		(*iter).SendStatus();
	}

	return true;
}

bool LoginServerList::SendPacket(ServerPacket* pack) {
	for (auto &iter : m_list) {
		(*iter).SendPacket(pack);
	}

	return true;
}

bool LoginServerList::SendAccountUpdate(ServerPacket* pack) {
	LogInfo("Requested to send ServerOP_LSAccountUpdate packet to all loginservers");
	for (auto &iter : m_list) {
		if ((*iter).CanUpdate()) {
			(*iter).SendAccountUpdate(pack);
		}
	}

	return true;
}

bool LoginServerList::Connected() {
	for (auto &iter : m_list) {
		if ((*iter).Connected()) {
			return true;
		}
	}

	return false;
}
