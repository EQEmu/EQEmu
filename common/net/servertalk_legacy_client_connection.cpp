#include "servertalk_legacy_client_connection.h"

#include "common/net/dns.h"
#include "common/eqemu_logsys.h"

EQ::Net::ServertalkLegacyClient::ServertalkLegacyClient(const std::string &addr, int port, bool ipv6)
	: m_timer(std::make_unique<EQ::Timer>(100, true, std::bind(&EQ::Net::ServertalkLegacyClient::Connect, this)))
{
	m_port = port;
	m_ipv6 = ipv6;
	m_connecting = false;
	DNSLookup(addr, port, false, [this](const std::string &address) {
		m_addr = address;
	});
}

EQ::Net::ServertalkLegacyClient::~ServertalkLegacyClient()
{
}

void EQ::Net::ServertalkLegacyClient::Send(uint16_t opcode, EQ::Net::Packet &p)
{
	if (!m_connection)
		return;

	EQ::Net::DynamicPacket out;
	out.PutUInt16(0, opcode);
	out.PutUInt16(2, p.Length() + 4);
	out.PutPacket(4, p);

	m_connection->Write((const char*)out.Data(), out.Length());
}

void EQ::Net::ServertalkLegacyClient::SendPacket(ServerPacket *p)
{
	EQ::Net::DynamicPacket pout;
	if (p->pBuffer) {
		pout.PutData(0, p->pBuffer, p->size);
	}
	Send(p->opcode, pout);
}

void EQ::Net::ServertalkLegacyClient::OnMessage(uint16_t opcode, std::function<void(uint16_t, EQ::Net::Packet&)> cb)
{
	m_message_callbacks.emplace(std::make_pair(opcode, cb));
}

void EQ::Net::ServertalkLegacyClient::OnMessage(std::function<void(uint16_t, EQ::Net::Packet&)> cb)
{
	m_message_callback = cb;
}

void EQ::Net::ServertalkLegacyClient::Connect()
{
	if (m_addr.length() == 0 || m_port == 0 || m_connection || m_connecting) {
		return;
	}

	m_connecting = true;
	EQ::Net::TCPConnection::Connect(m_addr, m_port, false, [this](std::shared_ptr<EQ::Net::TCPConnection> connection) {
		if (connection == nullptr) {
			LogNetTCP("Error connecting to {0}:{1}, attempting to reconnect...", m_addr, m_port);
			m_connecting = false;
			return;
		}

		LogNetTCP("Connected to {0}:{1}", m_addr, m_port);
		m_connection = connection;
		m_connection->OnDisconnect([this](EQ::Net::TCPConnection *c) {
			LogNetTCP("Connection lost to {0}:{1}, attempting to reconnect...", m_addr, m_port);
			m_connection.reset();
		});

		m_connection->OnRead(std::bind(&EQ::Net::ServertalkLegacyClient::ProcessData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		m_connection->Start();
		m_connecting = false;

		if (m_on_connect_cb) {
			m_on_connect_cb(this);
		}
	});
}

void EQ::Net::ServertalkLegacyClient::ProcessData(EQ::Net::TCPConnection *c, const unsigned char *data, size_t length)
{
	m_buffer.insert(m_buffer.end(), (const char*)data, (const char*)data + length);
	ProcessReadBuffer();
}

void EQ::Net::ServertalkLegacyClient::ProcessReadBuffer()
{
	size_t current = 0;
	size_t total = m_buffer.size();

	while (current < total) {
		auto left = total - current;

		/*
		//header:
		//uint16 opcode;
		//uint16 length;
		*/
		uint16_t length = 0;
		uint16_t opcode = 0;
		if (left < 4) {
			break;
		}

		opcode = *(uint16_t*)&m_buffer[current];
		length = *(uint16_t*)&m_buffer[current + 2];
		if (length < 4) {
			break;
		}

		length -= 4;

		if (current + 4 + length > total) {
			break;
		}

		if (length == 0) {
			EQ::Net::DynamicPacket p;

			auto cb = m_message_callbacks.find(opcode);
			if (cb != m_message_callbacks.end()) {
				cb->second(opcode, p);
			}

			if (m_message_callback) {
				m_message_callback(opcode, p);
			}
		}
		else {
			EQ::Net::StaticPacket p(&m_buffer[current + 4], length);

			auto cb = m_message_callbacks.find(opcode);
			if (cb != m_message_callbacks.end()) {
				cb->second(opcode, p);
			}

			if (m_message_callback) {
				m_message_callback(opcode, p);
			}
		}

		current += length + 4;
	}

	if (current == total) {
		m_buffer.clear();
	}
	else {
		m_buffer.erase(m_buffer.begin(), m_buffer.begin() + current);
	}
}
