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

#include "common/eq_packet_structs.h"
#include "common/net/servertalk_client_connection.h"

#include <mutex>

class WorldServer {
public:
	WorldServer();
	~WorldServer();

	void Connect();
	bool SendPacket(ServerPacket *pack);
	std::string GetIP() const;
	uint16 GetPort() const;
	bool Connected() const;

	void HandleMessage(uint16 opcode, const EQ::Net::Packet &p);
private:
	std::unique_ptr<EQ::Net::ServertalkClient> m_connection;

};
