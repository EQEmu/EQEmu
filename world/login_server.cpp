#include "login_server.h"

#include "common/eq_packet_structs.h"
#include "common/eqemu_logsys.h"
#include "common/misc_functions.h"
#include "common/packet_dump.h"
#include "common/servertalk.h"
#include "common/strings.h"
#include "common/version.h"
#include "world/cliententry.h"
#include "world/clientlist.h"
#include "world/login_server_list.h"
#include "world/world_config.h"
#include "world/worlddb.h"
#include "world/zonelist.h"
#include "world/zoneserver.h"

#include <cstring>
#include <cstdio>
#include <cstdlib>

extern uint32        numzones;
extern uint32        numplayers;
extern volatile bool RunLoops;

LoginServer::LoginServer(const char *iAddress, uint16 iPort, const char *Account, const char *Password, bool legacy)
{
	strn0cpy(m_loginserver_address, iAddress, 256);
	m_loginserver_port   = iPort;
	m_login_account      = Account;
	m_login_password     = Password;
	m_can_account_update = false;
	m_is_legacy          = legacy;
	Connect();
}

LoginServer::~LoginServer()
{
}

void LoginServer::ProcessUsertoWorldReqLeg(uint16_t opcode, EQ::Net::Packet &p)
{
	const WorldConfig *Config = WorldConfig::get();
	LogNetcode("Received ServerPacket from LS OpCode {:#04x}", opcode);

	UsertoWorldRequestLegacy *utwr = (UsertoWorldRequestLegacy *) p.Data();
	uint32                   id    = database.GetAccountIDFromLSID("eqemu", utwr->lsaccountid);
	int16                           status = database.GetAccountStatus(id);

	LogDebug(
		"id [{}] status [{}] account_id [{}] world_id [{}] from_id [{}] to_id [{}] ip [{}]",
		id,
		status,
		utwr->lsaccountid,
		utwr->worldid,
		utwr->FromID,
		utwr->ToID,
		utwr->IPAddr
	);

	ServerPacket outpack;
	outpack.opcode  = ServerOP_UsertoWorldRespLeg;
	outpack.size    = sizeof(UsertoWorldResponseLegacy);
	outpack.pBuffer = new uchar[outpack.size];
	memset(outpack.pBuffer, 0, outpack.size);

	UsertoWorldResponseLegacy *utwrs = (UsertoWorldResponseLegacy *) outpack.pBuffer;
	utwrs->lsaccountid = utwr->lsaccountid;
	utwrs->ToID        = utwr->FromID;
	utwrs->worldid     = utwr->worldid;
	utwrs->response    = UserToWorldStatusSuccess;

	if (Config->Locked) {
		if (status < (RuleI(GM, MinStatusToBypassLockedServer))) {
			LogDebug(
				"Server locked and status is not high enough for account_id [{0}]",
				utwr->lsaccountid
			);
			utwrs->response = UserToWorldStatusWorldUnavail;
			SendPacket(&outpack);
			return;
		}
	}

	int32 x = Config->MaxClients;
	if ((int32) numplayers >= x && x != -1 && x != 255 && status < (RuleI(GM, MinStatusToBypassLockedServer))) {
		LogDebug("World at capacity account_id [{0}]", utwr->lsaccountid);
		utwrs->response = UserToWorldStatusWorldAtCapacity;
		SendPacket(&outpack);
		return;
	}

	if (status == -1) {
		LogDebug("User suspended account_id [{0}]", utwr->lsaccountid);
		utwrs->response = UserToWorldStatusSuspended;
		SendPacket(&outpack);
		return;
	}

	if (status == -2) {
		LogDebug("User banned account_id [{0}]", utwr->lsaccountid);
		utwrs->response = UserToWorldStatusBanned;
		SendPacket(&outpack);
		return;
	}

	if (RuleB(World, EnforceCharacterLimitAtLogin)) {
		if (ClientList::Instance()->IsAccountInGame(utwr->lsaccountid)) {
			LogDebug("User already online account_id [{0}]", utwr->lsaccountid);
			utwrs->response = UserToWorldStatusAlreadyOnline;
			SendPacket(&outpack);
			return;
		}
	}

	LogDebug("Sent response to account_id [{0}]", utwr->lsaccountid);

	SendPacket(&outpack);
}

void LoginServer::ProcessUsertoWorldReq(uint16_t opcode, EQ::Net::Packet &p)
{
	const WorldConfig *Config = WorldConfig::get();
	LogNetcode("Received ServerPacket from LS OpCode {:#04x}", opcode);

	UsertoWorldRequest *utwr = (UsertoWorldRequest *) p.Data();
	uint32             id    = database.GetAccountIDFromLSID(utwr->login, utwr->lsaccountid);
	int16                     status = database.GetAccountStatus(id);

	LogDebug(
		"id [{}] status [{}] account_id [{}] world_id [{}] from_id [{}] to_id [{}] ip [{}]",
		id,
		status,
		utwr->lsaccountid,
		utwr->worldid,
		utwr->FromID,
		utwr->ToID,
		utwr->IPAddr
	);

	ServerPacket outpack;
	outpack.opcode  = ServerOP_UsertoWorldResp;
	outpack.size    = sizeof(UsertoWorldResponse);
	outpack.pBuffer = new uchar[outpack.size];
	memset(outpack.pBuffer, 0, outpack.size);

	UsertoWorldResponse *utwrs = (UsertoWorldResponse *) outpack.pBuffer;
	utwrs->lsaccountid = utwr->lsaccountid;
	utwrs->ToID        = utwr->FromID;
	strn0cpy(utwrs->login, utwr->login, 64);
	utwrs->worldid  = utwr->worldid;
	utwrs->response = UserToWorldStatusSuccess;

	if (Config->Locked == true) {
		if (status < (RuleI(GM, MinStatusToBypassLockedServer))) {
			LogDebug(
				"Server locked and status is not high enough for account_id [{0}]",
				utwr->lsaccountid
			);
			utwrs->response = UserToWorldStatusWorldUnavail;
			SendPacket(&outpack);
			return;
		}
	}

	int32 x = Config->MaxClients;
	if ((int32) numplayers >= x && x != -1 && x != 255 && status < (RuleI(GM, MinStatusToBypassLockedServer))) {
		LogDebug("World at capacity account_id [{0}]", utwr->lsaccountid);
		utwrs->response = UserToWorldStatusWorldAtCapacity;
		SendPacket(&outpack);
		return;
	}

	if (status == -1) {
		LogDebug("User suspended account_id [{0}]", utwr->lsaccountid);
		utwrs->response = UserToWorldStatusSuspended;
		SendPacket(&outpack);
		return;
	}

	if (status == -2) {
		LogDebug("User banned account_id [{0}]", utwr->lsaccountid);
		utwrs->response = UserToWorldStatusBanned;
		SendPacket(&outpack);
		return;
	}

	if (RuleB(World, EnforceCharacterLimitAtLogin)) {
		if (ClientList::Instance()->IsAccountInGame(utwr->lsaccountid)) {
			LogDebug("User already online account_id [{0}]", utwr->lsaccountid);
			utwrs->response = UserToWorldStatusAlreadyOnline;
			SendPacket(&outpack);
			return;
		}
	}

	LogDebug("Sent response to account_id [{0}]", utwr->lsaccountid);

	SendPacket(&outpack);
}

void LoginServer::ProcessLSClientAuthLegacy(uint16_t opcode, EQ::Net::Packet &p)
{
	const WorldConfig *Config = WorldConfig::get();
	LogNetcode("Received ServerPacket from LS OpCode {:#04x}", opcode);

	try {
		auto r = p.GetSerialize<ClientAuthLegacy>(0);

		LogDebug(
			"Processing Loginserver Auth Legacy | account_id [{}] account_name [{}] key [{}] admin [{}] ip [{}] "
			"local_network [{}]",
			r.loginserver_account_id,
			r.loginserver_account_name,
			r.key,
			r.is_world_admin,
			r.ip_address,
			r.is_client_from_local_network
		);

		ClientList::Instance()->CLEAdd(
			r.loginserver_account_id,
			"eqemu",
			r.loginserver_account_name,
			r.key,
			r.is_world_admin,
			r.ip_address,
			r.is_client_from_local_network
		);
	}
	catch (std::exception &ex) {
		LogError("Error parsing ClientAuthLegacy packet from world\nReason [{0}]", ex.what());
	}
}

void LoginServer::ProcessLSClientAuth(uint16_t opcode, EQ::Net::Packet &p)
{
	const WorldConfig *Config = WorldConfig::get();
	LogNetcode("Received ServerPacket from LS OpCode {:#04x}", opcode);

	try {
		auto r = p.GetSerialize<ClientAuth>(0);

		LogDebug(
			"Processing Loginserver Auth | account_id [{}] account_name [{}] loginserver_name [{}] key [{}] "
			"admin [{}] ip [{}] local_network [{}]",
			r.loginserver_account_id,
			r.account_name,
			r.loginserver_name,
			r.key,
			r.is_world_admin,
			r.ip_address,
			r.is_client_from_local_network
		);

		ClientList::Instance()->CLEAdd(
			r.loginserver_account_id,
			r.loginserver_name,
			r.account_name,
			r.key,
			r.is_world_admin,
			r.ip_address,
			r.is_client_from_local_network
		);
	}
	catch (std::exception &ex) {
		LogError("Error parsing ClientAuth packet from world\nReason [{0}]", ex.what());
	}
}

void LoginServer::ProcessLSFatalError(uint16_t opcode, EQ::Net::Packet &p)
{
	const WorldConfig *Config = WorldConfig::get();
	LogNetcode("Received ServerPacket from LS OpCode {:#04x}", opcode);

	std::string error;
	std::string reason;

	if (p.Length() > 1) {
		error = fmt::format("{}", (const char *) p.Data());
	}

	if (error.find("Worldserver Account / Password INVALID") != std::string::npos) {
		reason = "Usually this indicates you do not have a valid [account] and [password] (worldserver) account associated with your loginserver configuration. ";
		if (fmt::format("{}", m_loginserver_address).find("login.eqemulator.net") != std::string::npos) {
			reason += "For Legacy EQEmulator connections, you need to register your server @ https://www.eqemulator.org/index.php?pageid=ws_mgmt";
		}
	}

	LogInfo(
		"Login server [{}:{}] responded with fatal error [{}] {}\n",
		m_loginserver_address,
		m_loginserver_port,
		error,
		reason
	);

	if (m_legacy_client) {
		m_legacy_client.release();
	}
	else if (m_client) {
		m_client.release();
	}
}

void LoginServer::ProcessSystemwideMessage(uint16_t opcode, EQ::Net::Packet &p)
{
	const WorldConfig *Config = WorldConfig::get();
	LogNetcode("Received ServerPacket from LS OpCode {:#04x}", opcode);

	ServerSystemwideMessage *swm = (ServerSystemwideMessage *) p.Data();
	ZSList::Instance()->SendEmoteMessageRaw(
		0,
		0,
		AccountStatus::Player,
		swm->type,
		swm->message
	);
}

void LoginServer::ProcessLSRemoteAddr(uint16_t opcode, EQ::Net::Packet &p)
{
	const WorldConfig *Config = WorldConfig::get();
	LogNetcode("Received ServerPacket from LS OpCode {:#04x}", opcode);

	if (!Config->WorldAddress.length()) {
		WorldConfig::SetWorldAddress((char *) p.Data());
		LogInfo("Loginserver provided [{}] as world address", (const char *) p.Data());
	}
}

void LoginServer::ProcessLSAccountUpdate(uint16_t opcode, EQ::Net::Packet &p)
{
	const WorldConfig *Config = WorldConfig::get();
	LogNetcode("Received ServerPacket from LS OpCode {:#04x}", opcode);

	LogNetcode("Received ServerOP_LSAccountUpdate packet from loginserver");
	m_can_account_update = true;
}

bool LoginServer::Connect()
{
	char errbuf[1024];
	if ((m_loginserver_ip = ResolveIP(m_loginserver_address, errbuf)) == 0) {
		LogInfo("Unable to resolve [{}] to an IP", m_loginserver_address);
		return false;
	}

	if (m_loginserver_ip == 0 || m_loginserver_port == 0) {
		LogInfo(
			"Connect info incomplete, cannot connect: [{0}:{1}]",
			m_loginserver_address,
			m_loginserver_port
		);

		return false;
	}

	if (m_is_legacy) {
		m_legacy_client = std::make_unique<EQ::Net::ServertalkLegacyClient>(
			m_loginserver_address,
			m_loginserver_port,
			false
		);
		m_legacy_client->OnConnect(
			[this](EQ::Net::ServertalkLegacyClient *client) {
				if (client) {
					LogInfo(
						"Connected to Legacy Loginserver: [{0}:{1}]",
						m_loginserver_address,
						m_loginserver_port
					);

					SendInfo();
					SendStatus();
					ZSList::Instance()->SendLSZones();

					m_statusupdate_timer = std::make_unique<EQ::Timer>(
						LoginServer_StatusUpdateInterval, true, [this](EQ::Timer *t) {
							SendStatus();
						}
					);
				}
				else {
					LogInfo(
						"Could not connect to Legacy Loginserver: [{0}:{1}]",
						m_loginserver_address,
						m_loginserver_port
					);
				}
			}
		);

		m_legacy_client->OnMessage(
			ServerOP_UsertoWorldReqLeg,
			std::bind(
				&LoginServer::ProcessUsertoWorldReqLeg,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
		m_legacy_client->OnMessage(
			ServerOP_UsertoWorldReq,
			std::bind(
				&LoginServer::ProcessUsertoWorldReq,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
		m_legacy_client->OnMessage(
			ServerOP_LSClientAuthLeg,
			std::bind(
				&LoginServer::ProcessLSClientAuthLegacy,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
		m_legacy_client->OnMessage(
			ServerOP_LSClientAuth,
			std::bind(
				&LoginServer::ProcessLSClientAuth,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
		m_legacy_client->OnMessage(
			ServerOP_LSFatalError,
			std::bind(
				&LoginServer::ProcessLSFatalError,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
		m_legacy_client->OnMessage(
			ServerOP_SystemwideMessage,
			std::bind(
				&LoginServer::ProcessSystemwideMessage,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
		m_legacy_client->OnMessage(
			ServerOP_LSRemoteAddr,
			std::bind(
				&LoginServer::ProcessLSRemoteAddr,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
		m_legacy_client->OnMessage(
			ServerOP_LSAccountUpdate,
			std::bind(
				&LoginServer::ProcessLSAccountUpdate,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
	}
	else {
		m_client = std::make_unique<EQ::Net::ServertalkClient>(
			m_loginserver_address,
			m_loginserver_port,
			false,
			"World",
			""
		);
		m_client->OnConnect(
			[this](EQ::Net::ServertalkClient *client) {
				if (client) {
					LogInfo(
						"Connected to Loginserver [{0}:{1}]",
						m_loginserver_address,
						m_loginserver_port
					);
					SendInfo();
					SendStatus();
					ZSList::Instance()->SendLSZones();

					m_statusupdate_timer = std::make_unique<EQ::Timer>(
						LoginServer_StatusUpdateInterval, true, [this](EQ::Timer *t) {
							SendStatus();
						}
					);
				}
				else {
					LogInfo(
						"Could not connect to Loginserver: [{0}:{1}]",
						m_loginserver_address,
						m_loginserver_port
					);
				}
			}
		);

		m_client->OnMessage(
			ServerOP_UsertoWorldReqLeg,
			std::bind(
				&LoginServer::ProcessUsertoWorldReqLeg,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
		m_client->OnMessage(
			ServerOP_UsertoWorldReq,
			std::bind(
				&LoginServer::ProcessUsertoWorldReq,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
		m_client->OnMessage(
			ServerOP_LSClientAuthLeg,
			std::bind(
				&LoginServer::ProcessLSClientAuthLegacy,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
		m_client->OnMessage(
			ServerOP_LSClientAuth,
			std::bind(
				&LoginServer::ProcessLSClientAuth,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
		m_client->OnMessage(
			ServerOP_LSFatalError,
			std::bind(
				&LoginServer::ProcessLSFatalError,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
		m_client->OnMessage(
			ServerOP_SystemwideMessage,
			std::bind(
				&LoginServer::ProcessSystemwideMessage,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
		m_client->OnMessage(
			ServerOP_LSRemoteAddr,
			std::bind(
				&LoginServer::ProcessLSRemoteAddr,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
		m_client->OnMessage(
			ServerOP_LSAccountUpdate,
			std::bind(
				&LoginServer::ProcessLSAccountUpdate,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
	}

	return true;
}

void LoginServer::SendInfo()
{
	if (m_client == nullptr && m_legacy_client == nullptr) {
		LogDebug("No client to send info to loginserver");
		return;
	}

	const WorldConfig *Config = WorldConfig::get();

	auto pack = new ServerPacket;
	pack->opcode  = ServerOP_NewLSInfo;
	pack->size    = sizeof(LoginserverNewWorldRequest);
	pack->pBuffer = new uchar[pack->size];
	memset(pack->pBuffer, 0, pack->size);

	auto *l = (LoginserverNewWorldRequest *) pack->pBuffer;
	strcpy(l->protocol_version, EQEMU_PROTOCOL_VERSION);
	strcpy(l->server_version, LOGIN_VERSION);
	strcpy(l->server_long_name, Config->LongName.c_str());
	strcpy(l->server_short_name, Config->ShortName.c_str());
	strn0cpy(l->account_name, m_login_account.c_str(), 30);
	strn0cpy(l->account_password, m_login_password.c_str(), 30);
	if (Config->WorldAddress.length()) {
		strcpy(l->remote_ip_address, Config->WorldAddress.c_str());
	}
	if (Config->LocalAddress.length()) {
		strcpy(l->local_ip_address, Config->LocalAddress.c_str());
	}
	else {
		auto local_addr = m_is_legacy ? m_legacy_client->Handle()->LocalIP() : m_client->Handle()->LocalIP();
		strcpy(l->local_ip_address, local_addr.c_str());
		WorldConfig::SetLocalAddress(l->local_ip_address);
	}

	LogInfo(
		"protocol_version [{}] server_version [{}] long_name [{}] short_name [{}] account_name [{}] remote_ip_address [{}] local_ip [{}]",
		l->protocol_version,
		l->server_version,
		l->server_long_name,
		l->server_short_name,
		l->account_name,
		l->remote_ip_address,
		l->local_ip_address
	);

	SendPacket(pack);
	delete pack;
}


void LoginServer::SendStatus()
{
	if (m_client == nullptr && m_legacy_client == nullptr) {
		LogDebug("No client to send status to loginserver");
		return;
	}

	auto pack = new ServerPacket;
	pack->opcode  = ServerOP_LSStatus;
	pack->size    = sizeof(LoginserverWorldStatusUpdate);
	pack->pBuffer = new uchar[pack->size];
	memset(pack->pBuffer, 0, pack->size);
	auto loginserver_status = (LoginserverWorldStatusUpdate *) pack->pBuffer;

	if (WorldConfig::get()->Locked) {
		loginserver_status->status = -2;
	}
	else if (numzones <= 0) {
		loginserver_status->status = -2;
	}
	else {
		loginserver_status->status = numplayers;
	}

	loginserver_status->num_zones   = numzones;
	loginserver_status->num_players = numplayers;
	SendPacket(pack);
	delete pack;
}

void LoginServer::SendPacket(ServerPacket *pack)
{
	if (m_legacy_client) {
		m_legacy_client->SendPacket(pack);
	}
	else if (m_client) {
		m_client->SendPacket(pack);
	}
}

void LoginServer::SendAccountUpdate(ServerPacket *pack)
{
	if (m_client == nullptr && m_legacy_client == nullptr) {
		LogDebug("No client to send account update to loginserver");
		return;
	}

	auto *req = (LoginserverAccountUpdate *) pack->pBuffer;
	if (CanUpdate()) {
		LogInfo(
			"Sending ServerOP_LSAccountUpdate packet to loginserver: [{0}]:[{1}]",
			m_loginserver_address,
			m_loginserver_port
		);
		strn0cpy(req->world_account, m_login_account.c_str(), 30);
		strn0cpy(req->world_password, m_login_password.c_str(), 30);
		SendPacket(pack);
	}
}

