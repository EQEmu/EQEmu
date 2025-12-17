#include "tcp_connection.h"

#include "common/event/event_loop.h"

#include <iostream>

WriteReqPool tcp_write_pool;

void on_close_handle(uv_handle_t* handle) {
	delete (uv_tcp_t *)handle;
}

EQ::Net::TCPConnection::TCPConnection(uv_tcp_t *socket)
{
	m_socket = socket;
	m_socket->data = this;
}

EQ::Net::TCPConnection::~TCPConnection() {
	Disconnect();
}

void EQ::Net::TCPConnection::Connect(const std::string &addr, int port, bool ipv6, std::function<void(std::shared_ptr<TCPConnection>)> cb)
{
	struct EQTCPConnectBaton
	{
		uv_tcp_t *socket;
		std::function<void(std::shared_ptr<EQ::Net::TCPConnection>)> cb;
	};

	auto loop = EQ::EventLoop::Get().Handle();
	uv_tcp_t *socket = new uv_tcp_t;
	memset(socket, 0, sizeof(uv_tcp_t));
	uv_tcp_init(loop, socket);

	sockaddr_storage iaddr;
	if (ipv6) {
		uv_ip6_addr(addr.c_str(), port, (sockaddr_in6*)&iaddr);
	}
	else {
		uv_ip4_addr(addr.c_str(), port, (sockaddr_in*)&iaddr);
	}

	uv_connect_t *connect = new uv_connect_t;
	memset(connect, 0, sizeof(uv_connect_t));

	EQTCPConnectBaton *baton = new EQTCPConnectBaton;
	baton->cb = cb;
	baton->socket = socket;
	connect->data = baton;
	uv_tcp_connect(connect, socket, (sockaddr*)&iaddr,
		[](uv_connect_t* req, int status) {
		EQTCPConnectBaton *baton = (EQTCPConnectBaton*)req->data;
		auto socket = baton->socket;
		auto cb = baton->cb;

		delete baton;

		if (status < 0) {
			uv_close((uv_handle_t*)socket, on_close_handle);
			delete req;
			cb(nullptr);
		}
		else {
			delete req;
			std::shared_ptr<EQ::Net::TCPConnection> connection(new EQ::Net::TCPConnection(socket));
			cb(connection);
		}
	});
}

void EQ::Net::TCPConnection::Start()
{
	uv_read_start(
		(uv_stream_t *) m_socket, [](uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
			if (suggested_size > 65536) {
				buf->base = new char[suggested_size];
				buf->len  = suggested_size;
				return;
			}

			static thread_local char temp_buf[65536];
			buf->base = temp_buf;
			buf->len  = 65536;
		}, [](uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
			auto *connection = (TCPConnection *) stream->data;

			if (nread > 0) {
				connection->Read(buf->base, nread);
			}
			else if (nread == UV_EOF) {
				connection->Disconnect();
			}
			else if (nread < 0) {
				connection->Disconnect();
			}

			if (buf->len > 65536) {
				delete [] buf->base;
			}
		}
	);
}

void EQ::Net::TCPConnection::OnRead(std::function<void(TCPConnection*, const unsigned char*, size_t)> cb)
{
	m_on_read_cb = cb;
}

void EQ::Net::TCPConnection::OnDisconnect(std::function<void(TCPConnection*)> cb)
{
	m_on_disconnect_cb = cb;
}

void EQ::Net::TCPConnection::Disconnect()
{
	if (m_socket) {
		m_socket->data = this;
		uv_close((uv_handle_t*)m_socket, [](uv_handle_t* handle) {
			TCPConnection *connection = (TCPConnection*)handle->data;

			if (connection->m_on_disconnect_cb) {
				connection->m_on_disconnect_cb(connection);
			}

			delete (uv_tcp_t *)handle;
		});
		m_socket = nullptr;
	}
}

void EQ::Net::TCPConnection::Read(const char *data, size_t count)
{
	if (m_on_read_cb) {
		m_on_read_cb(this, (unsigned char*)data, count);
	}
}

void EQ::Net::TCPConnection::Write(const char* data, size_t count) {
	if (!m_socket || !data || count == 0) {
		std::cerr << "TCPConnection::Write - Invalid socket or data\n";
		return;
	}

	if (count <= TCP_BUFFER_SIZE) {
		// Fast path: use pooled request with embedded buffer
		auto req_opt = tcp_write_pool.acquire();
		if (!req_opt) {
			std::cerr << "TCPConnection::Write - Out of write requests\n";
			return;
		}

		TCPWriteReq* write_req = *req_opt;

		// Fill buffer and set context
		memcpy(write_req->buffer.data(), data, count);
		write_req->connection = this;
		write_req->magic = 0xC0FFEE;

		uv_buf_t buf = uv_buf_init(write_req->buffer.data(), static_cast<unsigned int>(count));

		int result = uv_write(
			&write_req->req,
			reinterpret_cast<uv_stream_t*>(m_socket),
			&buf,
			1,
			[](uv_write_t* req, int status) {
				auto* full_req = reinterpret_cast<TCPWriteReq*>(req);
				if (full_req->magic != 0xC0FFEE) {
					std::cerr << "uv_write callback - invalid magic, skipping release\n";
					return;
				}

				tcp_write_pool.release(full_req);

				if (status < 0 && full_req->connection) {
					std::cerr << "uv_write failed: " << uv_strerror(status) << std::endl;
					full_req->connection->Disconnect();
				}
			}
		);

		if (result < 0) {
			std::cerr << "uv_write() failed immediately: " << uv_strerror(result) << std::endl;
			tcp_write_pool.release(write_req);
		}

	} else {
		// Slow path: allocate heap buffer for large write
		LogNetTCP("[TCPConnection] Large write of [{}] bytes, using heap buffer", count);

		char* heap_buffer = new char[count];
		memcpy(heap_buffer, data, count);

		uv_write_t* write_req = new uv_write_t;
		write_req->data = heap_buffer;

		uv_buf_t buf = uv_buf_init(heap_buffer, static_cast<unsigned int>(count));

		int result = uv_write(
			write_req,
			reinterpret_cast<uv_stream_t*>(m_socket),
			&buf,
			1,
			[](uv_write_t* req, int status) {
				char* data = static_cast<char*>(req->data);
				delete[] data;
				delete req;

				if (status < 0) {
					std::cerr << "uv_write (large) failed: " << uv_strerror(status) << std::endl;
				}
			}
		);

		if (result < 0) {
			std::cerr << "uv_write() (large) failed immediately: " << uv_strerror(result) << std::endl;
			delete[] heap_buffer;
			delete write_req;
		}
	}
}


std::string EQ::Net::TCPConnection::LocalIP() const
{
	sockaddr_storage addr;
	int addr_len = sizeof(addr);
	uv_tcp_getsockname(m_socket, (sockaddr*)&addr, &addr_len);

	char endpoint[64] = { 0 };
	if (addr.ss_family == AF_INET) {
		uv_ip4_name((const sockaddr_in*)&addr, endpoint, 64);
	}
	else if (addr.ss_family == AF_INET6) {
		uv_ip6_name((const sockaddr_in6*)&addr, endpoint, 64);
	}

	return endpoint;
}

int EQ::Net::TCPConnection::LocalPort() const
{
	sockaddr_storage addr;
	int addr_len = sizeof(addr);
	uv_tcp_getsockname(m_socket, (sockaddr*)&addr, &addr_len);

	char endpoint[64] = { 0 };
	if (addr.ss_family == AF_INET) {
		sockaddr_in *s = (sockaddr_in*)&addr;
		return ntohs(s->sin_port);
	}
	else if (addr.ss_family == AF_INET6) {
		sockaddr_in6 *s = (sockaddr_in6*)&addr;
		return ntohs(s->sin6_port);
	}

	return 0;
}

std::string EQ::Net::TCPConnection::RemoteIP() const
{
	if (!m_socket) {
		return "";
	}

	sockaddr_storage addr;
	int addr_len = sizeof(addr);
	uv_tcp_getpeername(m_socket, (sockaddr*)&addr, &addr_len);

	char endpoint[64] = { 0 };
	if (addr.ss_family == AF_INET) {
		uv_ip4_name((const sockaddr_in*)&addr, endpoint, 64);
	}
	else if(addr.ss_family == AF_INET6) {
		uv_ip6_name((const sockaddr_in6*)&addr, endpoint, 64);
	}

	return endpoint;
}

int EQ::Net::TCPConnection::RemotePort() const
{
	if (!m_socket) {
		return 0;
	}

	sockaddr_storage addr;
	int addr_len = sizeof(addr);
	uv_tcp_getpeername(m_socket, (sockaddr*)&addr, &addr_len);

	char endpoint[64] = { 0 };
	if (addr.ss_family == AF_INET) {
		sockaddr_in *s = (sockaddr_in*)&addr;
		return ntohs(s->sin_port);
	}
	else if (addr.ss_family == AF_INET6) {
		sockaddr_in6 *s = (sockaddr_in6*)&addr;
		return ntohs(s->sin6_port);
	}

	return 0;
}

/**
 * @return
 */
bool EQ::Net::TCPConnection::IsConnected() const
{
	return m_socket != nullptr;
}
