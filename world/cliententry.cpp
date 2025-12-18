#include "cliententry.h"

#include "world/clientlist.h"
#include "world/login_server_list.h"
#include "world/login_server.h"
#include "world/shared_task_manager.h"
#include "world/world_config.h"
#include "world/worlddb.h"
#include "world/zoneserver.h"

extern uint32            numplayers;
extern volatile bool     RunLoops;

ClientListEntry::ClientListEntry(
	uint32 id,
	uint32 login_server_id,
	const char *login_server_name,
	const char *account_name,
	const char *login_key,
	int16 is_world_admin,
	uint32 ip_address,
	uint8 local
)
	: m_id(id)
{
	ClearVars(true);

	LogDebug(
		"id [{}] loginserver_id [{}] loginserver_name [{}] login_name [{}] login_key [{}] is_world_admin [{}] ip [{}] local [{}]",
		id,
		login_server_id,
		login_server_name,
		account_name,
		login_key,
		is_world_admin,
		ip_address,
		local
	);

	m_ip_address      = ip_address;
	m_login_server_id = login_server_id;
	if (login_server_id > 0) {
		m_account_id = database.GetAccountIDFromLSID(
			login_server_name,
			login_server_id,
			m_account_name,
			&m_admin
		);
	}

	strn0cpy(m_login_account_name, account_name, sizeof(m_login_account_name));
	strn0cpy(m_key, login_key, sizeof(m_key));
	strn0cpy(m_source_loginserver, login_server_name, sizeof(m_source_loginserver));

	m_world_admin = is_world_admin;
	m_is_local    = (local == 1);

	memset(m_lfg_comments, 0, 64);
}

ClientListEntry::ClientListEntry(uint32 in_id, ZoneServer *z, ServerClientList_Struct *scl, CLE_Status online)
	: m_id(in_id)
{
	ClearVars(true);

	m_ip_address      = 0;
	m_login_server_id = scl->LSAccountID;
	strn0cpy(m_login_account_name, scl->name, sizeof(m_login_account_name));
	strn0cpy(m_key, scl->lskey, sizeof(m_key));
	m_world_admin = 0;

	m_account_id = scl->AccountID;
	strn0cpy(m_account_name, scl->AccountName, sizeof(m_account_name));
	m_admin = scl->Admin;

	m_instance         = 0;
	m_lfg_from_level   = 0;
	m_lfg_to_level     = 0;
	m_lfg_match_filter = false;
	memset(m_lfg_comments, 0, 64);

	if (online >= CLE_Status::Zoning) {
		Update(z, scl, online);
	}
	else {
		SetOnline(online);
	}
}

ClientListEntry::~ClientListEntry()
{
	if (RunLoops) {
		Camp(); // updates zoneserver's numplayers
		ClientList::Instance()->RemoveCLEReferances(this);
	}
	for (auto &elem: m_tell_queue) {
		safe_delete_array(elem);
	}
	m_tell_queue.clear();
}

void ClientListEntry::SetChar(uint32 iCharID, const char *iCharName)
{
	m_char_id = iCharID;
	strn0cpy(m_char_name, iCharName, sizeof(m_char_name));
}

void ClientListEntry::SetOnline(CLE_Status iOnline)
{
	LogClientLogin(
		"Online status [{}] ({}) status [{}] ({})",
		AccountName(),
		AccountID(),
		CLEStatusString[iOnline],
		static_cast<int>(iOnline)
	);

	if (iOnline >= CLE_Status::Online && m_online < CLE_Status::Online) {
		numplayers++;
	}
	else if (iOnline < CLE_Status::Online && m_online >= CLE_Status::Online) {
		numplayers--;
	}
	if (iOnline != CLE_Status::Online || m_online < CLE_Status::Online) {
		m_online = iOnline;
	}
	if (iOnline < CLE_Status::Zoning) {
		Camp();
	}
	if (m_online >= CLE_Status::Online) {
		m_stale = 0;
	}
}

void ClientListEntry::LSUpdate(ZoneServer *iZS)
{
	if (WorldConfig::get()->UpdateStats) {
		auto pack = new ServerPacket;
		pack->opcode  = ServerOP_LSZoneInfo;
		pack->size    = sizeof(LoginserverZoneInfoUpdate);
		pack->pBuffer = new uchar[pack->size];
		auto *zone = (LoginserverZoneInfoUpdate *) pack->pBuffer;
		zone->count    = iZS->NumPlayers();
		zone->zone     = iZS->GetZoneID();
		zone->zone_wid = iZS->GetID();
		LoginServerList::Instance()->SendPacket(pack);
		safe_delete(pack);
	}
}

void ClientListEntry::LSZoneChange(ZoneToZone_Struct *ztz)
{
	if (WorldConfig::get()->UpdateStats) {
		auto pack = new ServerPacket;
		pack->opcode  = ServerOP_LSPlayerZoneChange;
		pack->size    = sizeof(ServerLSPlayerZoneChange_Struct);
		pack->pBuffer = new uchar[pack->size];
		auto *zonechange = (ServerLSPlayerZoneChange_Struct *) pack->pBuffer;
		zonechange->lsaccount_id = LSID();
		zonechange->from         = ztz->current_zone_id;
		zonechange->to           = ztz->requested_zone_id;
		LoginServerList::Instance()->SendPacket(pack);
		safe_delete(pack);
	}
}

void ClientListEntry::Update(ZoneServer *iZS, ServerClientList_Struct *scl, CLE_Status iOnline)
{
	if (m_zone_server != iZS) {
		if (m_zone_server) {
			m_zone_server->RemovePlayer();
			LSUpdate(m_zone_server);
		}
		if (iZS) {
			iZS->AddPlayer();
			LSUpdate(iZS);
		}
	}
	m_zone_server          = iZS;
	m_zone                 = scl->zone;
	m_instance             = scl->instance_id;
	m_char_id              = scl->charid;

	strcpy(m_char_name, scl->name);
	if (m_account_id == 0) {
		m_account_id = scl->AccountID;
		strcpy(m_account_name, scl->AccountName);
		strcpy(m_login_account_name, scl->AccountName);
		m_ip_address      = scl->IP;
		m_login_server_id = scl->LSAccountID;
		strn0cpy(m_key, scl->lskey, sizeof(m_key));
	}
	m_admin                = scl->Admin;
	m_level                = scl->level;
	m_class_               = scl->class_;
	m_race                 = scl->race;
	m_anon                 = scl->anon;
	m_tells_off            = scl->tellsoff;
	m_guild_id             = scl->guild_id;
	m_guild_rank           = scl->guild_rank;
	m_guild_tribute_opt_in = scl->guild_tribute_opt_in;
	m_lfg                  = scl->LFG;
	m_gm                   = scl->gm;
	m_client_version       = scl->ClientVersion;

	// Fields from the LFG Window
	if ((scl->LFGFromLevel != 0) && (scl->LFGToLevel != 0)) {
		m_lfg_from_level   = scl->LFGFromLevel;
		m_lfg_to_level     = scl->LFGToLevel;
		m_lfg_match_filter = scl->LFGMatchFilter;
		memcpy(m_lfg_comments, scl->LFGComments, sizeof(m_lfg_comments));
	}

	SetOnline(iOnline);
}

void ClientListEntry::LeavingZone(ZoneServer *iZS, CLE_Status iOnline)
{
	if (iZS != 0 && iZS != m_zone_server) {
		return;
	}
	SetOnline(iOnline);

	SharedTaskManager::Instance()->RemoveActiveInvitationByCharacterID(CharID());

	if (m_zone_server) {
		m_zone_server->RemovePlayer();
		LSUpdate(m_zone_server);
	}
	m_zone_server = 0;
	m_zone        = 0;
}

void ClientListEntry::ClearVars(bool iAll)
{
	if (iAll) {
		m_online = CLE_Status::Never;
		m_stale  = 0;

		m_login_server_id = 0;
		memset(m_login_account_name, 0, sizeof(m_login_account_name));
		memset(m_key, 0, sizeof(m_key));
		m_world_admin = 0;

		m_account_id = 0;
		memset(m_account_name, 0, sizeof(m_account_name));
		m_admin = AccountStatus::Player;
	}
	m_zone_server = 0;
	m_zone        = 0;
	m_char_id     = 0;
	memset(m_char_name, 0, sizeof(m_char_name));
	m_level          = 0;
	m_class_         = 0;
	m_race           = 0;
	m_anon           = 0;
	m_tells_off      = 0;
	m_guild_id       = GUILD_NONE;
	m_guild_rank     = 0;
	m_lfg            = 0;
	m_gm             = 0;
	m_client_version = 0;
	for (auto &elem: m_tell_queue) {
		safe_delete_array(elem);
	}
	m_tell_queue.clear();
}

void ClientListEntry::Camp(ZoneServer *iZS)
{
	if (iZS != 0 && iZS != m_zone_server) {
		return;
	}
	if (m_zone_server) {
		m_zone_server->RemovePlayer();
		LSUpdate(m_zone_server);
	}

	ClearVars();

	m_stale = 0;
}

bool ClientListEntry::CheckStale()
{
	m_stale++;
	if (m_stale > 20) {
		if (m_online > CLE_Status::Offline) {
			SetOnline(CLE_Status::Offline);
		}

		return true;
	}
	return false;
}

bool ClientListEntry::CheckAuth(uint32 loginserver_account_id, const char *key_password)
{
	LogDebug(
		"ls_account_id [{}] key_password [{}] key [{}]",
		loginserver_account_id,
		key_password,
		m_key
	);

	if (m_login_server_id == loginserver_account_id && strncmp(m_key, key_password, 10) == 0) {
		LogDebug(
			"ls_account_id [{}] key_password [{}] m_key [{}] lsid [{}] m_account_id [{}]",
			loginserver_account_id,
			key_password,
			m_key,
			LSID(),
			m_account_id
		);

		// create account if it doesn't exist
		if (m_account_id == 0 && LSID() > 0) {
			int16 default_account_status = WorldConfig::get()->DefaultStatus;

			m_account_id = database.CreateAccount(
				m_login_account_name,
				std::string(),
				default_account_status,
				m_source_loginserver,
				LSID()
			);

			if (!m_account_id) {
				LogError(
					"Error adding local account for LS login [{}] [{}], duplicate name",
					m_source_loginserver,
					m_login_account_name
				);
				return false;
			}
			strn0cpy(m_account_name, m_login_account_name, sizeof(m_account_name));
			m_admin = default_account_status;
		}
		std::string lsworldadmin;
		if (database.GetVariable("honorlsworldadmin", lsworldadmin)) {
			if (Strings::ToInt(lsworldadmin) == 1 && m_world_admin != 0 &&
				(m_admin < m_world_admin || m_admin == AccountStatus::Player)) {
				m_admin = m_world_admin;
			}
		}
		return true;
	}
	return false;
}

void ClientListEntry::ProcessTellQueue()
{
	if (!Server()) {
		return;
	}

	ServerPacket *pack;
	auto         it = m_tell_queue.begin();
	while (it != m_tell_queue.end()) {
		pack = new ServerPacket(
			ServerOP_ChannelMessage,
			sizeof(ServerChannelMessage_Struct) + strlen((*it)->message) + 1
		);
		memcpy(pack->pBuffer, *it, pack->size);
		Server()->SendPacket(pack);
		safe_delete(pack);
		safe_delete_array(*it);
		it = m_tell_queue.erase(it);
	}
}

