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
