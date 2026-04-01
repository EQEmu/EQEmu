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

#include "common/net/servertalk_server.h"
#include "common/packet_dump.h"
#include "loginserver/client.h"
#include "loginserver/world_server.h"

#include <list>

class WorldServerManager {
public:
	WorldServerManager();
	~WorldServerManager();
	void SendUserLoginToWorldRequest(
		unsigned int server_id,
		unsigned int client_account_id,
		const std::string &client_loginserver
	);
	std::unique_ptr<EQApplicationPacket> CreateServerListPacket(Client *client, uint32 sequence);
	bool DoesServerExist(const std::string &s, const std::string &server_short_name, WorldServer *ignore = nullptr);
	void DestroyServerByName(std::string s, std::string server_short_name, WorldServer *ignore = nullptr);
	const std::list<std::unique_ptr<WorldServer>> &GetWorldServers() const;

private:
	std::unique_ptr<EQ::Net::ServertalkServer> m_server_connection;
	std::list<std::unique_ptr<WorldServer>>    m_world_servers;
};
