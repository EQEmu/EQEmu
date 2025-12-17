#include "tcp_server.h"

#include "common/event/event_loop.h"

void on_close_tcp_server_handle(uv_handle_t* handle) {
	delete (uv_tcp_t *)handle;
}

EQ::Net::TCPServer::TCPServer()
{
	m_socket = nullptr;
}

EQ::Net::TCPServer::~TCPServer() {
	Close();
}

void EQ::Net::TCPServer::Listen(int port, bool ipv6, std::function<void(std::shared_ptr<TCPConnection>)> cb)
{
	if (ipv6) {
		Listen("::", port, ipv6, cb);
	}
	else {
		Listen("0.0.0.0", port, ipv6, cb);
	}
}

void EQ::Net::TCPServer::Listen(const std::string &addr, int port, bool ipv6, std::function<void(std::shared_ptr<TCPConnection>)> cb)
{
	if (m_socket) {
		return;
	}

	m_on_new_connection = cb;

	auto loop = EQ::EventLoop::Get().Handle();
	m_socket = new uv_tcp_t;
	memset(m_socket, 0, sizeof(uv_tcp_t));
	uv_tcp_init(loop, m_socket);

	sockaddr_storage iaddr;
	if (ipv6) {
		uv_ip6_addr(addr.c_str(), port, (sockaddr_in6*)&iaddr);
	}
	else {
		uv_ip4_addr(addr.c_str(), port, (sockaddr_in*)&iaddr);
	}

	uv_tcp_bind(m_socket, (sockaddr*)&iaddr, 0);
	m_socket->data = this;

	uv_listen((uv_stream_t*)m_socket, 128, [](uv_stream_t* server, int status) {
		if (status < 0) {
			return;
		}

		auto loop = EQ::EventLoop::Get().Handle();
		uv_tcp_t *client = new uv_tcp_t;
		memset(client, 0, sizeof(uv_tcp_t));
		uv_tcp_init(loop, client);

		if (uv_accept(server, (uv_stream_t*)client) < 0) {
			delete client;
			return;
		}

		EQ::Net::TCPServer *s = (EQ::Net::TCPServer*)server->data;
		s->AddClient(client);
	});
}

void EQ::Net::TCPServer::Close()
{
	if (m_socket) {
		uv_close((uv_handle_t*)m_socket, on_close_tcp_server_handle);
		m_socket = nullptr;
	}
}

void EQ::Net::TCPServer::AddClient(uv_tcp_t *c)
{
	std::shared_ptr<TCPConnection> client(new TCPConnection(c));
	if (m_on_new_connection) {
		m_on_new_connection(client);
	}
}
