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

#include "common/event/timer.h"
#include "common/net/servertalk_server_connection.h"
#include "common/servertalk.h"
#include "common/types.h"

#include <memory>

class UCSConnection
{
public:
	UCSConnection();
	void SetConnection(std::shared_ptr<EQ::Net::ServertalkServerConnection> connection);
	void ProcessPacket(uint16 opcode, EQ::Net::Packet &p);
	void SendPacket(ServerPacket* pack);
	void Disconnect() { if(connection && connection->Handle()) connection->Handle()->Disconnect(); }
	void SendMessage(const char *From, const char *Message);
	const std::shared_ptr<EQ::Net::ServertalkServerConnection> &GetConnection() const;
	inline bool IsConnected() const { return connection->Handle() ? connection->Handle()->IsConnected() : false; }

	static UCSConnection* Instance()
	{
		static UCSConnection instance;
		return &instance;
	}

private:
	inline std::string GetIP() const { return (connection && connection->Handle()) ? connection->Handle()->RemoteIP() : 0; }
	std::shared_ptr<EQ::Net::ServertalkServerConnection> connection;

};
