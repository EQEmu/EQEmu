#pragma once

#include "common/event/timer.h"
#include "common/net/packet.h"
#include "common/net/servertalk_common.h"
#include "common/net/tcp_connection.h"

namespace EQ
{
	namespace Net
	{
		class ServertalkClient
		{
		public:
			ServertalkClient(const std::string &addr, int port, bool ipv6, const std::string &identifier, const std::string &credentials);
			~ServertalkClient();

			void Send(uint16_t opcode, EQ::Net::Packet &p);
			void SendPacket(ServerPacket *p);
			void OnConnect(std::function<void(ServertalkClient*)> cb) { m_on_connect_cb = cb; }
			void OnMessage(uint16_t opcode, std::function<void(uint16_t, EQ::Net::Packet&)> cb);
			void OnMessage(std::function<void(uint16_t, EQ::Net::Packet&)> cb);
			bool Connected() const { return m_connecting != true; }

			std::shared_ptr<EQ::Net::TCPConnection> Handle() { return m_connection; }

			const std::unique_ptr<EQ::Timer> &GetTimer() const { return m_timer; }

		private:
			void Connect();
			void ProcessData(EQ::Net::TCPConnection *c, const unsigned char *data, size_t length);
			void SendHello();
			void InternalSend(ServertalkPacketType type, EQ::Net::Packet &p);
			void ProcessReadBuffer();
			void ProcessHello(EQ::Net::Packet &p);
			void ProcessMessage(EQ::Net::Packet &p);
			void SendHandshake();

			std::unique_ptr<EQ::Timer> m_timer;

			std::string m_addr;
			std::string m_identifier;
			std::string m_credentials;
			bool m_connecting;
			int m_port;
			bool m_ipv6;
			std::shared_ptr<EQ::Net::TCPConnection> m_connection;
			std::vector<char> m_buffer;
			std::unordered_map<uint16_t, std::function<void(uint16_t, EQ::Net::Packet&)>> m_message_callbacks;
			std::function<void(uint16_t, EQ::Net::Packet&)> m_message_callback;
			std::function<void(ServertalkClient*)> m_on_connect_cb;
		};
	}
}
