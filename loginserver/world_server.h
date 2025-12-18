#pragma once

#include "common/net/servertalk_server_connection.h"
#include "common/packet_dump.h"
#include "common/repositories/login_server_admins_repository.h"
#include "common/servertalk.h"
#include "loginserver/client.h"
#include "loginserver/login_types.h"

#include <memory>
#include <string>

/**
 * World server class, controls the connected server processing.
 */
class WorldServer {
public:
	WorldServer(std::shared_ptr<EQ::Net::ServertalkServerConnection> worldserver_connection);
	~WorldServer();
	void Reset();

	std::shared_ptr<EQ::Net::ServertalkServerConnection> GetConnection() { return m_connection; }
	unsigned int GetServerId() const { return m_server_id; }
	WorldServer *SetServerId(unsigned int id)
	{
		m_server_id = id;
		return this;
	}

	std::string GetServerLongName() const { return m_server_long_name; }
	std::string GetServerShortName() const { return m_server_short_name; }
	bool IsAuthorizedToList() const { return m_is_server_authorized_to_list; }
	std::string GetLocalIP() const { return m_local_ip; }
	std::string GetRemoteIP() const { return m_remote_ip_address; }

	// Gets what kind of server this server is (legends, preferred, normal)
	unsigned int GetServerListID() const { return m_server_list_type_id; }
	WorldServer *SetServerListTypeId(unsigned int in_server_list_id);

	int GetStatus() const { return m_server_status; }
	unsigned int GetZonesBooted() const { return m_zones_booted; }
	unsigned int GetPlayersOnline() const { return m_players_online; }

	void HandleNewWorldserver(LoginserverNewWorldRequest *req);
	void HandleWorldserverStatusUpdate(LoginserverWorldStatusUpdate *u);
	bool HandleNewWorldserverValidation(LoginserverNewWorldRequest *r);
	void SendClientAuthToWorld(Client *c);
	static bool ValidateWorldServerAdminLogin(
		LoginWorldAdminAccountContext &c,
		LoginServerAdminsRepository::LoginServerAdmins &admin
	);
	void SerializeForClientServerList(class SerializeBuffer &out, bool use_local_ip, LSClientVersion version) const;

private:

	void ProcessNewLSInfo(uint16_t opcode, const EQ::Net::Packet &packet);
	void ProcessLSStatus(uint16_t opcode, const EQ::Net::Packet &packet);
	void ProcessUserToWorldResponseLegacy(uint16_t opcode, const EQ::Net::Packet &packet);
	void ProcessUserToWorldResponse(uint16_t opcode, const EQ::Net::Packet &packet);
	void ProcessLSAccountUpdate(uint16_t opcode, const EQ::Net::Packet &packet);

	std::shared_ptr<EQ::Net::ServertalkServerConnection> m_connection;

	unsigned int m_zones_booted;
	unsigned int m_players_online;
	int          m_server_status;
	unsigned int m_server_id;
	unsigned int m_server_list_type_id;
	unsigned int m_server_process_type;
	std::string  m_server_description;
	std::string  m_server_long_name;
	std::string  m_server_short_name;
	std::string  m_account_name;
	std::string  m_account_password;
	std::string  m_remote_ip_address;
	std::string  m_local_ip;
	std::string  m_protocol;
	std::string  m_server_version;
	bool         m_is_server_authorized_to_list;
	bool         m_is_server_logged_in;
	bool         m_is_server_trusted; // this is primarily for worldserver being able to push updates to the loginserver

	static void FormatWorldServerName(char *name, int8 server_list_type);
};
