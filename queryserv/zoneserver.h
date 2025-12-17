#pragma once

#include "common/emu_constants.h"
#include "common/event/timer.h"
#include "common/net/servertalk_server.h"
#include "common/timer.h"
#include "world/console.h"
#include "world/world_tcp_connection.h"

#include <string>

class Client;
class ServerPacket;

class ZoneServer : public WorldTCPConnection {
public:
	ZoneServer(std::shared_ptr<EQ::Net::ServertalkServerConnection> in_connection, EQ::Net::ConsoleServer *in_console);
	~ZoneServer();
	void        SendPacket(ServerPacket *pack) { m_connection->SendPacket(pack); }
	void        SetIsZoneConnected(bool in) { m_is_zone_connected = in; }
	bool        GetIsZoneConnected() { return m_is_zone_connected; }
	void        HandleMessage(uint16 opcode, const EQ::Net::Packet &p);
	std::string GetUUID() const { return m_connection->GetUUID(); }
	void        SendPlayerEventLogSettings();

private:
	std::shared_ptr<EQ::Net::ServertalkServerConnection> m_connection{};
	bool                                                 m_is_zone_connected = false;
	EQ::Net::ConsoleServer                               *m_console;
};
