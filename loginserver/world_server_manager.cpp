#include "world_server_manager.h"

#include "common/eqemu_logsys.h"
#include "common/ip_util.h"
#include "loginserver/login_server.h"
#include "loginserver/login_types.h"

#include <cstdlib>

extern LoginServer server;
extern bool        run_server;

WorldServerManager::WorldServerManager()
{
	int listen_port = server.config.GetVariableInt("general", "listen_port", 5998);

	m_server_connection = std::make_unique<EQ::Net::ServertalkServer>();
	EQ::Net::ServertalkServerOptions opts;
	opts.port = listen_port;
	opts.ipv6 = false;
	m_server_connection->Listen(opts);

	LogInfo("Loginserver now listening on port [{}]", listen_port);

	m_server_connection->OnConnectionIdentified(
		"World", [this](std::shared_ptr<EQ::Net::ServertalkServerConnection> c) {
			LogInfo(
				"New World Server connection from remote_ip [{}] port [{}]",
				c->Handle()->RemoteIP(),
				c->Handle()->RemotePort()
			);

			auto iter = std::find_if(
				m_world_servers.begin(), m_world_servers.end(),
				[&](const std::unique_ptr<WorldServer> &s) {
					return s->GetConnection()->Handle()->RemoteIP() == c->Handle()->RemoteIP() &&
						   s->GetConnection()->Handle()->RemotePort() == c->Handle()->RemotePort();
				}
			);

			if (iter != m_world_servers.end()) {
				LogInfo(
					"World server already existed for remote_ip [{}] port [{}] removing existing connection.",
					c->Handle()->RemoteIP(),
					c->Handle()->RemotePort()
				);

				m_world_servers.erase(iter);
			}

			m_world_servers.push_back(std::make_unique<WorldServer>(c));
		}
	);

	m_server_connection->OnConnectionRemoved(
		"World", [this](std::shared_ptr<EQ::Net::ServertalkServerConnection> c) {
			auto iter = std::find_if(
				m_world_servers.begin(), m_world_servers.end(),
				[&](const std::unique_ptr<WorldServer> &server) {
					return server->GetConnection()->GetUUID() == c->GetUUID();
				}
			);

			if (iter != m_world_servers.end()) {
				LogInfo(
					"World server ID [{}] long_name [{}] short_name [{}] has been disconnected, removing.",
					(*iter)->GetServerId(),
					(*iter)->GetServerLongName(),
					(*iter)->GetServerShortName()
				);
				m_world_servers.erase(iter);
			}
		}
	);

}

WorldServerManager::~WorldServerManager() = default;

std::unique_ptr<EQApplicationPacket> WorldServerManager::CreateServerListPacket(Client *client, uint32 sequence)
{
	unsigned int server_count = 0;
	in_addr      in{};
	in.s_addr = client->GetConnection()->GetRemoteIP();
	std::string client_ip = inet_ntoa(in);

	LogDebug("ServerManager::CreateServerListPacket via client address [{}]", client_ip);

	for (const auto &world_server: m_world_servers) {
		if (world_server->IsAuthorizedToList()) {
			++server_count;
		}
	}

	SerializeBuffer buf;

	// LoginBaseMessage_Struct header
	buf.WriteInt32(sequence);
	buf.WriteInt8(0);
	buf.WriteInt8(0);
	buf.WriteInt32(0);

	// LoginBaseReplyMessage_Struct
	buf.WriteInt8(true);  // success (no error)
	buf.WriteInt32(0x65); // 101 "No Error" eqlsstr
	buf.WriteString("");

	// ServerListReply_Struct
	buf.WriteInt32(server_count);

	for (const auto &s: m_world_servers) {
		if (!s->IsAuthorizedToList()) {
			LogDebug(
				"ServerManager::CreateServerListPacket | Server [{}] via IP [{}] is not authorized to be listed",
				s->GetServerLongName(),
				s->GetConnection()->Handle()->RemoteIP()
			);
			continue;
		}

		bool use_local_ip = false;

		std::string world_ip = s->GetConnection()->Handle()->RemoteIP();
		if (world_ip == client_ip || IpUtil::IsIpInPrivateRfc1918(client_ip)) {
			use_local_ip = true;
		}

		LogDebug(
			"CreateServerListPacket | Building list entry | Client [{}] IP [{}] Server Long Name [{}] Server IP [{}] ({})",
			client->GetAccountName(),
			client_ip,
			s->GetServerLongName(),
			use_local_ip ? s->GetLocalIP() : s->GetRemoteIP(),
			use_local_ip ? "Local" : "Remote"
		);

		s->SerializeForClientServerList(buf, use_local_ip, client->GetClientVersion());
	}

	return std::make_unique<EQApplicationPacket>(OP_ServerListResponse, buf);
}

void WorldServerManager::SendUserLoginToWorldRequest(
	unsigned int server_id,
	unsigned int client_account_id,
	const std::string &client_loginserver
)
{
	auto iter = std::find_if(
		m_world_servers.begin(), m_world_servers.end(),
		[&](const std::unique_ptr<WorldServer> &server) {
			return server->GetServerId() == server_id;
		}
	);

	if (iter != m_world_servers.end()) {
		EQ::Net::DynamicPacket outapp;
		outapp.Resize(sizeof(UsertoWorldRequest));

		auto *r = reinterpret_cast<UsertoWorldRequest *>(outapp.Data());
		r->worldid     = server_id;
		r->lsaccountid = client_account_id;
		strncpy(r->login, client_loginserver.c_str(), 64);

		(*iter)->GetConnection()->Send(ServerOP_UsertoWorldReq, outapp);

		LogNetcode("[UsertoWorldRequest] [Size: {}]\n{}", outapp.Length(), outapp.ToString());
	}
	else {
		LogError("Client requested a user to world but supplied an invalid id of {}", server_id);
	}
}

bool WorldServerManager::DoesServerExist(
	const std::string &server_long_name,
	const std::string &server_short_name,
	WorldServer *ignore
)
{
	return std::any_of(
		m_world_servers.begin(), m_world_servers.end(), [&](const std::unique_ptr<WorldServer> &s) {
			return s.get() != ignore &&
				   s->GetServerLongName() == server_long_name &&
				   s->GetServerShortName() == server_short_name;
		}
	);
}

void WorldServerManager::DestroyServerByName(
	std::string server_long_name,
	std::string server_short_name,
	WorldServer *ignore
)
{
	std::erase_if(
		m_world_servers, [&](const std::unique_ptr<WorldServer> &s) {
			if (s.get() == ignore) {
				return false;
			}
			if (s->GetServerLongName() == server_long_name &&
				s->GetServerShortName() == server_short_name) {
				s->GetConnection()->Handle()->Disconnect();
				LogInfo(
					"Removing world server ID [{}] long name [{}] short name [{}]",
					s->GetServerId(),
					server_long_name,
					server_short_name
				);
				return true;
			}
			return false;
		}
	);
}

const std::list<std::unique_ptr<WorldServer>> &WorldServerManager::GetWorldServers() const
{
	return m_world_servers;
}
