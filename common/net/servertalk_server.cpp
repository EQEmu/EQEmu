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
#include "servertalk_server.h"

EQ::Net::ServertalkServer::ServertalkServer()
{
}

EQ::Net::ServertalkServer::~ServertalkServer()
{
}

void EQ::Net::ServertalkServer::Listen(const ServertalkServerOptions& opts)
{
	m_credentials = opts.credentials;
	m_server = std::make_unique<EQ::Net::TCPServer>();
	m_server->Listen(opts.port, opts.ipv6, [this](std::shared_ptr<EQ::Net::TCPConnection> connection) {
		m_unident_connections.push_back(std::make_shared<ServertalkServerConnection>(connection, this));
	});
}

void EQ::Net::ServertalkServer::OnConnectionIdentified(const std::string &type, std::function<void(std::shared_ptr<ServertalkServerConnection>)> cb)
{
	m_on_ident.emplace(std::make_pair(type, cb));
}

void EQ::Net::ServertalkServer::OnConnectionRemoved(const std::string &type, std::function<void(std::shared_ptr<ServertalkServerConnection>)> cb)
{
	m_on_disc.emplace(std::make_pair(type, cb));
}

void EQ::Net::ServertalkServer::ConnectionDisconnected(ServertalkServerConnection *conn)
{
	if (conn->GetIdentifier().empty()) {
		auto iter = m_unident_connections.begin();
		while (iter != m_unident_connections.end()) {
			if (conn == iter->get()) {
				m_unident_connections.erase(iter);
				return;
			}
			++iter;
		}
	}
	else {
		auto type = m_ident_connections.find(conn->GetIdentifier());
		if (type != m_ident_connections.end()) {
			auto iter = type->second.begin();
			while (iter != type->second.end()) {
				if (conn == iter->get()) {
					auto on_disc = m_on_disc.find(conn->GetIdentifier());
					if (on_disc != m_on_disc.end()) {
						on_disc->second(*iter);
					}
					type->second.erase(iter);
					return;
				}
				++iter;
			}
		}
	}
}

void EQ::Net::ServertalkServer::ConnectionIdentified(ServertalkServerConnection *conn)
{
	auto iter = m_unident_connections.begin();
	while (iter != m_unident_connections.end()) {
		if (conn == iter->get()) {
			auto on_ident = m_on_ident.find(conn->GetIdentifier());
			if (on_ident != m_on_ident.end()) {
				on_ident->second(*iter);
			}

			if (m_ident_connections.count(conn->GetIdentifier()) > 0) {
				auto &vec = m_ident_connections[conn->GetIdentifier()];
				vec.push_back(*iter);
			}
			else {
				std::vector<std::shared_ptr<EQ::Net::ServertalkServerConnection>> vec;
				vec.push_back(*iter);
				m_ident_connections.emplace(std::make_pair(conn->GetIdentifier(), vec));
			}

			m_unident_connections.erase(iter);
			return;
		}
		++iter;
	}
}

bool EQ::Net::ServertalkServer::CheckCredentials(const std::string &credentials)
{
	if (credentials.compare(m_credentials) == 0) {
		return true;
	}

	return false;
}
