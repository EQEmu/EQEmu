#pragma once

#include "common/net/servertalk_server.h"
#include "common/packet_dump.h"
#include "common/servertalk.h"
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
