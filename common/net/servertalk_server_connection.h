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

#include "common/net/packet.h"
#include "common/net/servertalk_common.h"
#include "common/net/tcp_connection.h"

#include <vector>

namespace EQ
{
	namespace Net
	{
		class ServertalkServer;
		class ServertalkServerConnection
		{
		public:
			ServertalkServerConnection(std::shared_ptr<EQ::Net::TCPConnection> c, ServertalkServer *parent);
			~ServertalkServerConnection();

			void Send(uint16_t opcode, EQ::Net::Packet &p);
			void SendPacket(ServerPacket *p);
			void OnMessage(uint16_t opcode, std::function<void(uint16_t, EQ::Net::Packet&)> cb);
			void OnMessage(std::function<void(uint16_t, EQ::Net::Packet&)> cb);

			std::string GetIdentifier() const { return m_identifier; }
			std::shared_ptr<EQ::Net::TCPConnection> Handle() { return m_connection; }
			std::string GetUUID() const { return m_uuid; }
		private:
			void OnRead(TCPConnection* c, const unsigned char* data, size_t sz);
			void ProcessReadBuffer();
			void ProcessOldReadBuffer();
			void OnDisconnect(TCPConnection* c);
			void SendHello();
			void InternalSend(ServertalkPacketType type, EQ::Net::Packet &p);
			void ProcessHandshake(EQ::Net::Packet &p);
			void ProcessMessage(EQ::Net::Packet &p);
			void ProcessMessageOld(uint16_t opcode, EQ::Net::Packet &p);

			std::shared_ptr<EQ::Net::TCPConnection> m_connection;
			ServertalkServer *m_parent;

			std::vector<char> m_buffer;
			std::unordered_map<uint16_t, std::function<void(uint16_t, EQ::Net::Packet&)>> m_message_callbacks;
			std::function<void(uint16_t, EQ::Net::Packet&)> m_message_callback;
			std::string m_identifier;
			std::string m_uuid;
			bool m_legacy_mode;
		};
	}
}
