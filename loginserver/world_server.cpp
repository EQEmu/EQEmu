#include "world_server.h"

#include "common/ip_util.h"
#include "common/repositories/login_server_admins_repository.h"
#include "common/repositories/login_world_servers_repository.h"
#include "common/strings.h"
#include "loginserver/login_server.h"
#include "loginserver/login_types.h"

extern LoginServer server;
extern Database    database;

WorldServer::WorldServer(std::shared_ptr<EQ::Net::ServertalkServerConnection> worldserver_connection)
{
	m_connection                   = worldserver_connection;
	m_zones_booted                 = 0;
	m_players_online               = 0;
	m_server_status                = 0;
	m_server_id                    = 0;
	m_server_list_type_id          = 0;
	m_server_process_type          = 0;
	m_is_server_authorized_to_list = false;
	m_is_server_trusted            = false;
	m_is_server_logged_in          = false;

	worldserver_connection->OnMessage(
		ServerOP_NewLSInfo,
		std::bind(&WorldServer::ProcessNewLSInfo, this, std::placeholders::_1, std::placeholders::_2)
	);

	worldserver_connection->OnMessage(
		ServerOP_LSStatus,
		std::bind(&WorldServer::ProcessLSStatus, this, std::placeholders::_1, std::placeholders::_2)
	);

	worldserver_connection->OnMessage(
		ServerOP_UsertoWorldRespLeg,
		std::bind(
			&WorldServer::ProcessUserToWorldResponseLegacy,
			this,
			std::placeholders::_1,
			std::placeholders::_2
		)
	);

	worldserver_connection->OnMessage(
		ServerOP_UsertoWorldResp,
		std::bind(&WorldServer::ProcessUserToWorldResponse, this, std::placeholders::_1, std::placeholders::_2)
	);

	worldserver_connection->OnMessage(
		ServerOP_LSAccountUpdate,
		std::bind(&WorldServer::ProcessLSAccountUpdate, this, std::placeholders::_1, std::placeholders::_2)
	);
}

WorldServer::~WorldServer() = default;

void WorldServer::Reset()
{
	m_server_id                    = 0;
	m_zones_booted                 = 0;
	m_players_online               = 0;
	m_server_status                = 0;
	m_server_list_type_id          = 0;
	m_server_process_type          = 0;
	m_is_server_authorized_to_list = false;
	m_is_server_logged_in          = false;
}

void WorldServer::ProcessNewLSInfo(uint16_t opcode, const EQ::Net::Packet &packet)
{
	LogNetcode(
		"Application packet received from server [{:#04x}] [Size: {}]\n{}",
		opcode, packet.Length(), packet.ToString()
	);

	if (packet.Length() < sizeof(LoginserverNewWorldRequest)) {
		LogError(
			"Received application packet with opcode ServerOP_NewLSInfo, but it was too small. Discarded to avoid buffer overrun."
		);
		return;
	}

	auto *r = (LoginserverNewWorldRequest *) packet.Data();

	// If remote IP is missing, use local IP unless it's 127.0.0.1
	if (r->remote_ip_address[0] == '\0' && r->local_ip_address[0] != '\0' &&
		strcmp(r->local_ip_address, "127.0.0.1") != 0) {
		strncpy(r->remote_ip_address, r->local_ip_address, sizeof(r->remote_ip_address) - 1);
		r->remote_ip_address[sizeof(r->remote_ip_address) - 1] = '\0'; // Ensure null termination
	}

	LogInfo(
		"New World Server Info | name [{}] shortname [{}] remote_address [{}] local_address [{}] account [{}] password [{}] server_type [{}]",
		r->server_long_name,
		r->server_short_name,
		r->remote_ip_address,
		r->local_ip_address,
		r->account_name,
		r->account_password,
		r->server_process_type
	);

	HandleNewWorldserver(r);
}

void WorldServer::ProcessLSStatus(uint16_t opcode, const EQ::Net::Packet &packet)
{
	LogNetcode(
		"Application packet received from server [{:#04x}] [Size: {}]\n{}",
		opcode,
		packet.Length(),
		packet.ToString()
	);

	if (packet.Length() < sizeof(LoginserverWorldStatusUpdate)) {
		LogError(
			"Received application packet from server that had opcode ServerOP_LSStatus, but was too small. Discarded to avoid buffer overrun"
		);

		return;
	}

	auto *ls_status = (LoginserverWorldStatusUpdate *) packet.Data();

	LogDebug(
		"World Server Status Update Received | Server [{}] Status [{}] Players [{}] Zones [{}]",
		m_server_long_name,
		ls_status->status,
		ls_status->num_players,
		ls_status->num_zones
	);

	HandleWorldserverStatusUpdate(ls_status);
}

void WorldServer::ProcessUserToWorldResponseLegacy(uint16_t opcode, const EQ::Net::Packet &packet)
{
	LogNetcode(
		"Application packet received from server [{:#04x}] [Size: {}]\n{}",
		opcode,
		packet.Length(),
		packet.ToString()
	);

	if (packet.Length() < sizeof(UsertoWorldResponseLegacy)) {
		LogError(
			"Received application packet from server that had opcode ServerOP_UsertoWorldResp, "
			"but was too small. Discarded to avoid buffer overrun"
		);

		return;
	}

	auto *res = (UsertoWorldResponseLegacy *) packet.Data();

	LogDebug("Trying to find client with user id of [{}]", res->lsaccountid);
	std::string db_loginserver = "local";
	if (std::getenv("LSPX")) {
		db_loginserver = "eqemu";
	}
	Client *c = server.client_manager->GetClient(res->lsaccountid, db_loginserver);
	if (c) {
		LogDebug(
			"Found client with user id of [{}] and account name of [{}]",
			res->lsaccountid,
			c->GetAccountName()
		);

		auto *outapp = new EQApplicationPacket(
			OP_PlayEverquestResponse,
			sizeof(PlayEverquestResponse)
		);

		auto *play = (PlayEverquestResponse *) outapp->pBuffer;
		play->base_header.sequence = c->GetCurrentPlaySequence();
		play->server_number        = c->GetSelectedPlayServerID();

		if (res->response > 0) {
			play->base_reply.success = true;
			SendClientAuthToWorld(c);
		}

		switch (res->response) {
			case UserToWorldStatusSuccess:
				play->base_reply.error_str_id = LS::ErrStr::ERROR_NONE;
				break;
			case UserToWorldStatusWorldUnavail:
				play->base_reply.error_str_id = LS::ErrStr::ERROR_SERVER_UNAVAILABLE;
				break;
			case UserToWorldStatusSuspended:
				play->base_reply.error_str_id = LS::ErrStr::ERROR_ACCOUNT_SUSPENDED;
				break;
			case UserToWorldStatusBanned:
				play->base_reply.error_str_id = LS::ErrStr::ERROR_ACCOUNT_BANNED;
				break;
			case UserToWorldStatusWorldAtCapacity:
				play->base_reply.error_str_id = LS::ErrStr::ERROR_WORLD_MAX_CAPACITY;
				break;
			case UserToWorldStatusAlreadyOnline:
				play->base_reply.error_str_id = LS::ErrStr::ERROR_ACTIVE_CHARACTER;
				break;
			default:
				play->base_reply.error_str_id = LS::ErrStr::ERROR_UNKNOWN;
				break;
		}

		LogDebug(
			"Sending play response: allowed [{}] sequence [{}] server number [{}] message [{}]",
			play->base_reply.success,
			play->base_header.sequence,
			play->server_number,
			play->base_reply.error_str_id
		);

		c->SendPlayResponse(outapp);
		delete outapp;
	}
	else {
		LogError(
			"Received User-To-World Response for [{}] but could not find the client referenced!",
			res->lsaccountid
		);
	}
}

void WorldServer::ProcessUserToWorldResponse(uint16_t opcode, const EQ::Net::Packet &packet)
{
	LogNetcode(
		"Application packet received from server [{:#04x}] [Size: {}]\n{}",
		opcode,
		packet.Length(),
		packet.ToString()
	);

	if (packet.Length() < sizeof(UsertoWorldResponse)) {
		LogError(
			"Received application packet from server that had opcode ServerOP_UsertoWorldResp, "
			"but was too small. Discarded to avoid buffer overrun"
		);

		return;
	}

	auto res = (UsertoWorldResponse *) packet.Data();
	LogDebug("Trying to find client with user id of [{}]", res->lsaccountid);

	Client *c = server.client_manager->GetClient(
		res->lsaccountid,
		res->login
	);

	if (c) {
		LogDebug(
			"Found client with user id of [{}] and account name of {}",
			res->lsaccountid,
			c->GetAccountName().c_str()
		);

		auto *outapp = new EQApplicationPacket(
			OP_PlayEverquestResponse,
			sizeof(PlayEverquestResponse)
		);

		auto *r = (PlayEverquestResponse *) outapp->pBuffer;
		r->base_header.sequence = c->GetCurrentPlaySequence();
		r->server_number        = c->GetSelectedPlayServerID();

		LogDebug(
			"Found sequence and play of [{}] [{}]",
			c->GetCurrentPlaySequence(),
			c->GetSelectedPlayServerID()
		);

		LogDebug("[Size: [{}]] {}", outapp->size, DumpPacketToString(outapp));

		if (res->response > 0) {
			r->base_reply.success = true;
			SendClientAuthToWorld(c);
		}

		switch (res->response) {
			case UserToWorldStatusSuccess:
				r->base_reply.error_str_id = LS::ErrStr::ERROR_NONE;
				break;
			case UserToWorldStatusWorldUnavail:
				r->base_reply.error_str_id = LS::ErrStr::ERROR_SERVER_UNAVAILABLE;
				break;
			case UserToWorldStatusSuspended:
				r->base_reply.error_str_id = LS::ErrStr::ERROR_ACCOUNT_SUSPENDED;
				break;
			case UserToWorldStatusBanned:
				r->base_reply.error_str_id = LS::ErrStr::ERROR_ACCOUNT_BANNED;
				break;
			case UserToWorldStatusWorldAtCapacity:
				r->base_reply.error_str_id = LS::ErrStr::ERROR_WORLD_MAX_CAPACITY;
				break;
			case UserToWorldStatusAlreadyOnline:
				r->base_reply.error_str_id = LS::ErrStr::ERROR_ACTIVE_CHARACTER;
				break;
			default:
				r->base_reply.error_str_id = LS::ErrStr::ERROR_UNKNOWN;
				break;
		}

		LogDebug(
			"Sending play response with following data, allowed [{}], sequence {}, server number {}, message {}",
			r->base_reply.success,
			r->base_header.sequence,
			r->server_number,
			r->base_reply.error_str_id
		);

		c->SendPlayResponse(outapp);
		delete outapp;
	}
	else {
		LogError(
			"Received User-To-World Response for [{}] but could not find the client referenced!.",
			res->lsaccountid
		);
	}
}

void WorldServer::ProcessLSAccountUpdate(uint16_t opcode, const EQ::Net::Packet &packet)
{
	LogNetcode(
		"Application packet received from server [{:#04x}] [Size: {}]\n{}",
		opcode,
		packet.Length(),
		packet.ToString()
	);

	if (packet.Length() < sizeof(LoginserverAccountUpdate)) {
		LogError(
			"Received application packet from server that had opcode ServerLSAccountUpdate_Struct, "
			"but was too small. Discarded to avoid buffer overrun"
		);

		return;
	}

	LogDebug("ServerOP_LSAccountUpdate packet received from [{}]", m_server_short_name);

	auto *r = (LoginserverAccountUpdate *) packet.Data();
	if (m_is_server_trusted) {
		LogDebug("ServerOP_LSAccountUpdate update processed for: [{}]", r->user_account_name);

		LoginAccountContext c{};
		c.username           = r->user_account_name;
		c.source_loginserver = "local";
		auto a = LoginAccountsRepository::GetAccountFromContext(database, c);
		if (a.id > 0) {
			a.account_email    = r->user_email;
			a.account_password = r->user_account_password;
			a.last_ip_address  = "0.0.0.0";
			LoginAccountsRepository::UpdateOne(database, a);
		}
	}
}

void WorldServer::HandleNewWorldserver(LoginserverNewWorldRequest *req)
{
	if (m_is_server_logged_in) {
		LogInfo("Login server was already marked as logged in, returning");
		return;
	}

	if (!HandleNewWorldserverValidation(req)) {
		LogError("failed validation rules");
		return;
	}

	SanitizeWorldServerName(req->server_long_name);

	m_server_long_name    = req->server_long_name;
	m_server_short_name   = req->server_short_name;
	m_account_password    = req->account_password;
	m_account_name        = req->account_name;
	m_local_ip            = req->local_ip_address;
	m_remote_ip_address   = req->remote_ip_address;
	m_server_version      = req->server_version;
	m_protocol            = req->protocol_version;
	m_server_process_type = req->server_process_type;
	m_is_server_logged_in = true;

	// Handle Duplicate Servers
	if (server.server_manager->DoesServerExist(m_server_long_name, m_server_short_name, this)) {
		if (server.options.IsRejectingDuplicateServers()) {
			LogError("World tried to login but a server with that name already exists");
			return;
		}
		LogInfo("World tried to login but a server with that name already exists, destroying [{}]", m_server_long_name);
		server.server_manager->DestroyServerByName(m_server_long_name, m_server_short_name, this);
	}

	LoginWorldContext c;
	c.long_name  = m_server_long_name;
	c.short_name = m_server_short_name;

	LoginServerAdminsRepository::LoginServerAdmins admin;

	// Handle Admin Authentication
	if (!m_account_name.empty() && !m_account_password.empty()) {
		admin = LoginServerAdminsRepository::GetByName(database, m_account_name);

		LoginWorldAdminAccountContext ac;
		ac.id            = admin.id;
		ac.username      = m_account_name;
		ac.password      = m_account_password;
		ac.password_hash = admin.account_password;

		if (admin.id && WorldServer::ValidateWorldServerAdminLogin(ac, admin)) {
			LogDebug(
				"Authenticated world admin [{}] ({}) for world [{}]",
				m_account_name,
				admin.id,
				m_server_short_name
			);
			c.admin_id = admin.id;
			m_is_server_authorized_to_list = true;
		}
	}

	auto world = LoginWorldServersRepository::GetFromWorldContext(database, c);
	if (!world.id) {
		if (!server.options.IsUnregisteredAllowed()) {
			LogError("WorldServer [{}] is not registered, and unregistered servers are not allowed",
					 m_server_long_name);
			return;
		}

		LogInfo("Server [{}] is not registered, handling as unregistered", m_server_long_name);
		m_is_server_authorized_to_list = true;

		auto w = LoginWorldServersRepository::NewEntity();
		w.long_name                 = m_server_long_name;
		w.short_name                = m_server_short_name;
		w.last_ip_address           = m_remote_ip_address;
		w.login_server_list_type_id = LS::ServerType::Standard;
		w.last_login_date           = std::time(nullptr);
		auto created = LoginWorldServersRepository::InsertOne(database, w);
		if (!created.id) {
			LogError("Failed to auto-register world server [{}]", m_server_long_name);
			return;
		}

		LogInfo(
			"Auto-registered world server [{}] with ID [{}]",
			m_server_long_name,
			created.id
		);
	}
	else {
		m_server_description           = world.tag_description;
		m_server_id                    = world.id;
		m_is_server_trusted            = world.is_server_trusted;
		m_server_list_type_id          = world.login_server_list_type_id;
		m_is_server_authorized_to_list = true;

		LogInfo(
			"Server ID [{}] long_name [{}] short_name [{}] successfully authenticated",
			world.id,
			world.long_name,
			world.short_name
		);
	}

	LogInfo(
		"World registration id [{}] for server [{}] ip_address [{}]",
		m_server_id,
		m_server_long_name,
		m_remote_ip_address
	);

	// Update the last login date and IP address
	world.last_login_date = std::time(nullptr);
	world.last_ip_address = m_remote_ip_address;
	LoginWorldServersRepository::UpdateOne(database, world);

	WorldServer::FormatWorldServerName(
		req->server_long_name,
		m_server_list_type_id
	);

	m_server_long_name = req->server_long_name;
}

void WorldServer::HandleWorldserverStatusUpdate(LoginserverWorldStatusUpdate *u)
{
	m_players_online = u->num_players;
	m_zones_booted   = u->num_zones;
	m_server_status  = u->status;
}

void WorldServer::SendClientAuthToWorld(Client *c)
{
	EQ::Net::DynamicPacket outapp;
	ClientAuth             a{};

	a.loginserver_account_id = c->GetAccountID();

	strncpy(a.account_name, c->GetAccountName().c_str(), 30);
	strncpy(a.key, c->GetLoginKey().c_str(), 30);

	a.lsadmin        = 0;
	a.is_world_admin = 0;
	a.ip_address     = inet_addr(c->GetConnection()->GetRemoteAddr().c_str());
	strncpy(a.loginserver_name, &c->GetLoginServerName()[0], 64);

	const std::string &client_address(c->GetConnection()->GetRemoteAddr());
	std::string       world_address(m_connection->Handle()->RemoteIP());

	if (client_address == world_address) {
		a.is_client_from_local_network = 1;
	}
	else if (IpUtil::IsIpInPrivateRfc1918(client_address)) {
		LogInfo("Client is authenticating from a local address [{}]", client_address);
		a.is_client_from_local_network = 1;
	}
	else {
		a.is_client_from_local_network = 0;
	}

	struct in_addr ip_addr{};
	ip_addr.s_addr = a.ip_address;

	LogInfo(
		"Client authentication response: world_address [{}] client_address [{}]",
		world_address,
		client_address
	);

	LogInfo(
		"Sending Client Authentication Response ls_account_id [{}] ls_name [{}] name [{}] key [{}] ls_admin [{}] "
		"world_admin [{}] ip [{}] local [{}]",
		a.loginserver_account_id,
		a.loginserver_name,
		a.account_name,
		a.key,
		a.lsadmin,
		a.is_world_admin,
		inet_ntoa(ip_addr),
		a.is_client_from_local_network
	);

	outapp.PutSerialize(0, a);
	m_connection->Send(ServerOP_LSClientAuth, outapp);

	LogNetcode(
		"Sending [{:#04x}] [Size: {}]\n{}",
		ServerOP_LSClientAuth,
		outapp.Length(),
		outapp.ToString()
	);
}

constexpr static int MAX_ACCOUNT_NAME_LENGTH          = 30;
constexpr static int MAX_ACCOUNT_PASSWORD_LENGTH      = 30;
constexpr static int MAX_SERVER_LONG_NAME_LENGTH      = 200;
constexpr static int MAX_SERVER_SHORT_NAME_LENGTH     = 50;
constexpr static int MAX_SERVER_LOCAL_ADDRESS_LENGTH  = 125;
constexpr static int MAX_SERVER_REMOTE_ADDRESS_LENGTH = 125;
constexpr static int MAX_SERVER_VERSION_LENGTH        = 64;
constexpr static int MAX_SERVER_PROTOCOL_VERSION      = 25;

bool WorldServer::HandleNewWorldserverValidation(LoginserverNewWorldRequest *r)
{
	if (strlen(r->account_name) >= MAX_ACCOUNT_NAME_LENGTH) {
		LogError("HandleNewWorldserver error [account_name] was too long | max [{}]", MAX_ACCOUNT_NAME_LENGTH);
		return false;
	}
	else if (strlen(r->account_password) >= MAX_ACCOUNT_PASSWORD_LENGTH) {
		LogError("HandleNewWorldserver error [account_password] was too long | max [{}]", MAX_ACCOUNT_PASSWORD_LENGTH);
		return false;
	}
	else if (strlen(r->server_long_name) >= MAX_SERVER_LONG_NAME_LENGTH) {
		LogError("HandleNewWorldserver error [server_long_name] was too long | max [{}]", MAX_SERVER_LONG_NAME_LENGTH);
		return false;
	}
	else if (strlen(r->server_short_name) >= MAX_SERVER_SHORT_NAME_LENGTH) {
		LogError("HandleNewWorldserver error [server_short_name] was too long | max [{}]",
				 MAX_SERVER_SHORT_NAME_LENGTH);
		return false;
	}
	else if (strlen(r->server_version) >= MAX_SERVER_VERSION_LENGTH) {
		LogError("HandleNewWorldserver error [server_version] was too long | max [{}]", MAX_SERVER_VERSION_LENGTH);
		return false;
	}
	else if (strlen(r->protocol_version) >= MAX_SERVER_PROTOCOL_VERSION) {
		LogError("HandleNewWorldserver error [protocol_version] was too long | max [{}]", MAX_SERVER_PROTOCOL_VERSION);
		return false;
	}

	if (strlen(r->local_ip_address) <= MAX_SERVER_LOCAL_ADDRESS_LENGTH) {
		if (strlen(r->local_ip_address) == 0) {
			LogError("HandleNewWorldserver error, local address was null, defaulting to localhost");
			m_local_ip = "127.0.0.1";
		}
		else {
			m_local_ip = r->local_ip_address;
		}
	}
	else {
		LogError("HandleNewWorldserver error, local address was too long | max [{}]", MAX_SERVER_LOCAL_ADDRESS_LENGTH);
		return false;
	}

	if (strlen(r->remote_ip_address) <= MAX_SERVER_REMOTE_ADDRESS_LENGTH) {
		if (strlen(r->remote_ip_address) == 0) {
			m_remote_ip_address = GetConnection()->Handle()->RemoteIP();

			LogWarning(
				"Remote address was null, defaulting to stream address [{}]",
				m_remote_ip_address
			);
		}
		else {
			m_remote_ip_address = r->remote_ip_address;
		}
	}
	else {
		m_remote_ip_address = GetConnection()->Handle()->RemoteIP();

		LogWarning(
			"HandleNewWorldserver remote address was too long, defaulting to stream address [{}]",
			m_remote_ip_address
		);
	}

	return true;
}

bool WorldServer::ValidateWorldServerAdminLogin(
	LoginWorldAdminAccountContext &c,
	LoginServerAdminsRepository::LoginServerAdmins &admin
)
{
	auto encryption_mode = server.options.GetEncryptionMode();
	if (eqcrypt_verify_hash(c.username, c.password, c.password_hash, encryption_mode)) {
		return true;
	}

	if (encryption_mode < EncryptionModeArgon2) {
		encryption_mode = EncryptionModeArgon2;
	}

	uint32 insecure_source_encryption_mode = 0;
	auto   verify_encryption               = [&](int start, int end) {
		for (int i = start; i <= end; ++i) {
			if (i != encryption_mode && eqcrypt_verify_hash(c.username, c.password, c.password_hash, i)) {
				LogDebug("Checking for [{}] world admin", GetEncryptionByModeId(i));
				insecure_source_encryption_mode = i;
			}
		}
	};

	switch (c.password_hash.length()) {
		case CryptoHash::md5_hash_length:
			verify_encryption(EncryptionModeMD5, EncryptionModeMD5Triple);
			break;
		case CryptoHash::sha1_hash_length:
			verify_encryption(EncryptionModeSHA, EncryptionModeSHATriple);
			break;
		case CryptoHash::sha512_hash_length:
			verify_encryption(EncryptionModeSHA512, EncryptionModeSHA512Triple);
			break;
	}

	if (insecure_source_encryption_mode > 0) {
		LogInfo(
			"Updated insecure world_admin_username [{}] from mode [{}] ({}) to mode [{}] ({})",
			c.username,
			GetEncryptionByModeId(insecure_source_encryption_mode),
			insecure_source_encryption_mode,
			GetEncryptionByModeId(encryption_mode),
			encryption_mode
		);

		admin.account_password = eqcrypt_hash(c.username, c.password, encryption_mode);
		LoginServerAdminsRepository::UpdateOne(database, admin);

		return true;
	}

	return false;
}

void WorldServer::SerializeForClientServerList(SerializeBuffer &out, bool use_local_ip, LSClientVersion version) const
{
	// see LoginClientServerData_Struct
	if (use_local_ip) {
		out.WriteString(GetLocalIP());
	}
	else {
		out.WriteString(m_remote_ip_address);
	}

	if (version == cv_larion) {
		out.WriteUInt32(9000);
	}

	switch (GetServerListID()) {
		case LS::ServerType::Legends:
			out.WriteInt32(LS::ServerTypeFlags::Legends);
			break;
		case LS::ServerType::Preferred:
			out.WriteInt32(LS::ServerTypeFlags::Preferred);
			break;
		default:
			out.WriteInt32(LS::ServerTypeFlags::Standard);
			break;
	}
	if (version == cv_larion) {
		auto server_id = m_server_id;
		//if this is 0, the client will not show the server in the list
		out.WriteUInt32(1);
		out.WriteUInt32(server_id);
	}
	else {
		out.WriteUInt32(m_server_id);
	}

	out.WriteString(m_server_long_name);
	out.WriteString("us"); // country code
	out.WriteString("en"); // language code

	// 0 = Up, 1 = Down, 2 = Up, 3 = down, 4 = locked, 5 = locked(down)
	if (GetStatus() < 0) {
		if (GetZonesBooted() == 0) {
			out.WriteInt32(LS::ServerStatusFlags::Down);
		}
		else {
			out.WriteInt32(LS::ServerStatusFlags::Locked);
		}
	}
	else {
		out.WriteInt32(LS::ServerStatusFlags::Up);
	}

	out.WriteUInt32(GetPlayersOnline());
}

void WorldServer::FormatWorldServerName(char *name, int8 server_list_type)
{
	std::string server_long_name = name;
	server_long_name = Strings::Trim(server_long_name);

	bool name_set_to_bottom = false;
	if (server_list_type == LS::ServerType::Standard) {
		if (server.options.IsWorldDevTestServersListBottom()) {
			std::string s = Strings::ToLower(server_long_name);
			if (s.find("dev") != std::string::npos) {
				server_long_name   = fmt::format("|D| {}", server_long_name);
				name_set_to_bottom = true;
			}
			else if (s.find("test") != std::string::npos) {
				server_long_name   = fmt::format("|T| {}", server_long_name);
				name_set_to_bottom = true;
			}
			else if (s.find("installer") != std::string::npos) {
				server_long_name   = fmt::format("|I| {}", server_long_name);
				name_set_to_bottom = true;
			}
		}
		if (server.options.IsWorldSpecialCharacterStartListBottom() && !name_set_to_bottom) {
			auto first_char = server_long_name.c_str()[0];
			if (IsAllowedWorldServerCharacterList(first_char) && first_char != '|') {
				server_long_name   = fmt::format("|*| {}", server_long_name);
				name_set_to_bottom = true;
			}
		}
	}

	strn0cpy(name, server_long_name.c_str(), 201);
}
