#pragma once

#include "common/event/timer.h"
#include "common/net/servertalk_server_connection.h"
#include "common/servertalk.h"
#include "common/types.h"

#include <memory>

class UCSConnection
{
public:
	UCSConnection();
	void SetConnection(std::shared_ptr<EQ::Net::ServertalkServerConnection> connection);
	void ProcessPacket(uint16 opcode, EQ::Net::Packet &p);
	void SendPacket(ServerPacket* pack);
	void Disconnect() { if(connection && connection->Handle()) connection->Handle()->Disconnect(); }
	void SendMessage(const char *From, const char *Message);
	const std::shared_ptr<EQ::Net::ServertalkServerConnection> &GetConnection() const;
	inline bool IsConnected() const { return connection->Handle() ? connection->Handle()->IsConnected() : false; }

	static UCSConnection* Instance()
	{
		static UCSConnection instance;
		return &instance;
	}

private:
	inline std::string GetIP() const { return (connection && connection->Handle()) ? connection->Handle()->RemoteIP() : 0; }
	std::shared_ptr<EQ::Net::ServertalkServerConnection> connection;

};
