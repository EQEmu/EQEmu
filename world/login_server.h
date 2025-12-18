#pragma once

#include "common/eq_packet_structs.h"
#include "common/event/timer.h"
#include "common/linked_list.h"
#include "common/mutex.h"
#include "common/net/servertalk_client_connection.h"
#include "common/net/servertalk_legacy_client_connection.h"
#include "common/queue.h"
#include "common/servertalk.h"
#include "common/timer.h"

#include <memory>

class LoginServer{
public:
	LoginServer(const char*, uint16, const char*, const char*, bool legacy);
	~LoginServer();

	bool Connect();
	void SendInfo();
	void SendStatus();

	void SendPacket(ServerPacket* pack);
	void SendAccountUpdate(ServerPacket *pack);
	bool Connected()
	{
		if (m_is_legacy) {
			if (m_legacy_client) {
				return m_legacy_client->Connected();
			}
		}
		else {
			if (m_client) {
				return m_client->Connected();
			}
		}

		return false;
	}
	bool CanUpdate() { return m_can_account_update; }

private:
	void ProcessUsertoWorldReqLeg(uint16_t opcode, EQ::Net::Packet &p);
	void ProcessUsertoWorldReq(uint16_t opcode, EQ::Net::Packet &p);
	void ProcessLSClientAuth(uint16_t opcode, EQ::Net::Packet &p);
	void ProcessLSClientAuthLegacy(uint16_t opcode, EQ::Net::Packet &p);
	void ProcessLSFatalError(uint16_t opcode, EQ::Net::Packet &p);
	void ProcessSystemwideMessage(uint16_t opcode, EQ::Net::Packet &p);
	void ProcessLSRemoteAddr(uint16_t opcode, EQ::Net::Packet &p);
	void ProcessLSAccountUpdate(uint16_t opcode, EQ::Net::Packet &p);

	std::unique_ptr<EQ::Timer> m_keepalive;

	std::unique_ptr<EQ::Net::ServertalkClient>       m_client;
	std::unique_ptr<EQ::Net::ServertalkLegacyClient> m_legacy_client;
	std::unique_ptr<EQ::Timer>                       m_statusupdate_timer;
	char                                             m_loginserver_address[256];
	uint32                                           m_loginserver_ip;
	uint16                                           m_loginserver_port;
	std::string                                      m_login_account;
	std::string                                      m_login_password;
	bool                                             m_can_account_update;
	bool                                             m_is_legacy;
};
