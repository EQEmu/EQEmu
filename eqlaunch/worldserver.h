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
