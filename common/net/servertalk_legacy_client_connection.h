#pragma once

#include "common/event/timer.h"
#include "common/net/tcp_connection.h"
#include "common/net/packet.h"
#include "common/net/servertalk_common.h"

namespace EQ
{
	namespace Net
	{
		class ServertalkLegacyClient
		{
		public:
			ServertalkLegacyClient(const std::string &addr, int port, bool ipv6);
			~ServertalkLegacyClient();

			void Send(uint16_t opcode, EQ::Net::Packet &p);
			void SendPacket(ServerPacket *p);
			void OnConnect(std::function<void(ServertalkLegacyClient*)> cb) { m_on_connect_cb = cb; }
			void OnMessage(uint16_t opcode, std::function<void(uint16_t, EQ::Net::Packet&)> cb);
			void OnMessage(std::function<void(uint16_t, EQ::Net::Packet&)> cb);
			bool Connected() const { return m_connecting != true; }

			std::shared_ptr<EQ::Net::TCPConnection> Handle() { return m_connection; }
		private:
			void Connect();
			void ProcessData(EQ::Net::TCPConnection *c, const unsigned char *data, size_t length);
			void ProcessReadBuffer();

			std::unique_ptr<EQ::Timer> m_timer;

			std::string m_addr;
			bool m_connecting;
			int m_port;
			bool m_ipv6;
			std::shared_ptr<EQ::Net::TCPConnection> m_connection;
			std::vector<char> m_buffer;
			std::unordered_map<uint16_t, std::function<void(uint16_t, EQ::Net::Packet&)>> m_message_callbacks;
			std::function<void(uint16_t, EQ::Net::Packet&)> m_message_callback;
			std::function<void(ServertalkLegacyClient*)> m_on_connect_cb;
		};
	}
}
