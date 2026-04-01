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

#include "common/net/tcp_connection.h"

namespace EQ
{
	namespace Net
	{
		class TCPServer
		{
		public:
			TCPServer();
			~TCPServer();

			void Listen(int port, bool ipv6, std::function<void(std::shared_ptr<TCPConnection>)> cb);
			void Listen(const std::string &addr, int port, bool ipv6, std::function<void(std::shared_ptr<TCPConnection>)> cb);
			void Close();
			void AddClient(uv_tcp_t *c);

		private:
			std::function<void(std::shared_ptr<TCPConnection>)> m_on_new_connection;
			uv_tcp_t *m_socket;
		};
	}
}
