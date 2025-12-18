#pragma once

#include "common/types.h"
#include "common/net/servertalk_server.h"
#include "common/servertalk.h"
#include "common/event/timer.h"

class QueryServConnection
{
public:
	QueryServConnection();
	void AddConnection(std::shared_ptr<EQ::Net::ServertalkServerConnection> connection);
	void RemoveConnection(std::shared_ptr<EQ::Net::ServertalkServerConnection> connection);
	void HandleGenericMessage(uint16_t opcode, EQ::Net::Packet &p);
	void HandleLFGuildUpdateMessage(uint16_t opcode, EQ::Net::Packet &p);
	bool SendPacket(ServerPacket* pack);

	static QueryServConnection* Instance()
	{
		static QueryServConnection instance;
		return &instance;
	}

private:
	std::map<std::string, std::shared_ptr<EQ::Net::ServertalkServerConnection>> m_streams;
	std::unique_ptr<EQ::Timer> m_keepalive;
};
