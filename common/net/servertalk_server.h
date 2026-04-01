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

#include "common/net/servertalk_server_connection.h"
#include "common/net/tcp_server.h"

#include <map>
#include <vector>

namespace EQ
{
	namespace Net
	{
		struct ServertalkServerOptions
		{
			int port;
			bool ipv6;
			std::string credentials;

			ServertalkServerOptions() {
				ipv6 = false;
			}
		};

		class ServertalkServer
		{
		public:
			ServertalkServer();
			~ServertalkServer();

			void Listen(const ServertalkServerOptions& opts);
			void OnConnectionIdentified(const std::string &type, std::function<void(std::shared_ptr<ServertalkServerConnection>)> cb);
			void OnConnectionRemoved(const std::string &type, std::function<void(std::shared_ptr<ServertalkServerConnection>)> cb);

		private:
			void ConnectionDisconnected(ServertalkServerConnection *conn);
			void ConnectionIdentified(ServertalkServerConnection *conn);
			bool CheckCredentials(const std::string &credentials);

			std::unique_ptr<EQ::Net::TCPServer> m_server;
			std::vector<std::shared_ptr<ServertalkServerConnection>> m_unident_connections;
			std::map<std::string, std::vector<std::shared_ptr<ServertalkServerConnection>>> m_ident_connections;

			std::map<std::string, std::function<void(std::shared_ptr<ServertalkServerConnection>)>> m_on_ident;
			std::map<std::string, std::function<void(std::shared_ptr<ServertalkServerConnection>)>> m_on_disc;
			bool m_encrypted;
			bool m_allow_downgrade;
			std::string m_credentials;

			friend class ServertalkServerConnection;
		};
	}
}
