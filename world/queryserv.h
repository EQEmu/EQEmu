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
#include "common/net/servertalk_server.h"
#include "common/servertalk.h"
#include "common/event/timer.h"

class QueryServConnection
{
public:
	QueryServConnection();
	void AddConnection(std::shared_ptr<EQ::Net::ServertalkServerConnection> connection);
	void RemoveConnection(std::shared_ptr<EQ::Net::ServertalkServerConnection> connection);
	void HandleGenericMessage(uint16_t opcode, EQ::Net::Packet &p);
	void HandleLFGuildUpdateMessage(uint16_t opcode, EQ::Net::Packet &p);
	bool SendPacket(ServerPacket* pack);

	static QueryServConnection* Instance()
	{
		static QueryServConnection instance;
		return &instance;
	}

private:
	std::map<std::string, std::shared_ptr<EQ::Net::ServertalkServerConnection>> m_streams;
	std::unique_ptr<EQ::Timer> m_keepalive;
};
