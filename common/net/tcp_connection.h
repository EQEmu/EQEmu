#pragma once

#include "common/net/tcp_connection_pooling.h"

#include "uv.h"
#include <functional>
#include <memory>
#include <string>

namespace EQ
{
	namespace Net
	{
		class TCPConnection
		{
		public:
			TCPConnection(uv_tcp_t *socket);
			~TCPConnection();

			static void Connect(const std::string &addr, int port, bool ipv6, std::function<void(std::shared_ptr<TCPConnection>)> cb);

			void Start();
			void OnRead(std::function<void(TCPConnection*, const unsigned char *, size_t)> cb);
			void OnDisconnect(std::function<void(TCPConnection*)> cb);
			void Disconnect();
			void Read(const char *data, size_t count);
			void Write(const char *data, size_t count);

			bool IsConnected() const;
			std::string LocalIP() const;
			int LocalPort() const;
			std::string RemoteIP() const;
			int RemotePort() const;

		private:
			TCPConnection();

			uv_tcp_t *m_socket;
			std::function<void(TCPConnection*, const unsigned char *, size_t)> m_on_read_cb;
			std::function<void(TCPConnection*)> m_on_disconnect_cb;
		};
	}
}
