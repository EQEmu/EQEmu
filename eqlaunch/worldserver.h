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

#include "common/net/servertalk_client_connection.h"

#include <map>
#include <memory>
#include <queue>
#include <string>

class ZoneLaunch;
class EQEmuConfig;

class WorldServer {
public:
	WorldServer(std::map<std::string, ZoneLaunch *> &zones, const char *name, const EQEmuConfig *config);
	~WorldServer();

	void HandleMessage(uint16 opcode, EQ::Net::Packet &p);

	void SendStatus(const char *short_name, uint32 start_count, bool running);

private:
	virtual void OnConnected();

	std::unique_ptr<EQ::Net::ServertalkClient> m_connection;
	const char *const m_name;
	const EQEmuConfig *const m_config;
	std::map<std::string, ZoneLaunch *> &m_zones;
};
