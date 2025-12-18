#pragma once

#include "common/md5.h"
#include "common/rulesys.h"
#include "common/servertalk.h"
#include "common/types.h"

#include <vector>

typedef enum {
	Never,
	Offline,
	Online,
	CharSelect,
	Zoning,
	InZone
} CLE_Status;

static const char *CLEStatusString[] = {
	"Never",
	"Offline",
	"Online",
	"CharSelect",
	"Zoning",
	"InZone"
};

class ZoneServer;
struct ServerClientList_Struct;

class ClientListEntry {
public:

	ClientListEntry(
		uint32 id,
		uint32 login_server_id,
		const char *login_server_name,
		const char *account_name,
		const char *login_key,
		int16 is_world_admin = 0,
		uint32 ip_address = 0,
		uint8 local = 0
	);

	ClientListEntry(uint32 id, ZoneServer *z, ServerClientList_Struct *scl, CLE_Status online);
	~ClientListEntry();
	bool CheckStale();
	void Update(ZoneServer *zoneserver, ServerClientList_Struct *scl, CLE_Status iOnline = CLE_Status::InZone);
	void LSUpdate(ZoneServer *zoneserver);
	void LSZoneChange(ZoneToZone_Struct *ztz);
	bool CheckAuth(uint32 loginserver_account_id, const char *key_password);
	void SetOnline(CLE_Status iOnline = CLE_Status::Online);
	void SetChar(uint32 iCharID, const char *iCharName);
	inline CLE_Status Online() { return m_online; }
	inline const uint32 GetID() const { return m_id; }
	inline const uint32 GetIP() const { return m_ip_address; }
	inline void SetIP(const uint32 &iIP) { m_ip_address = iIP; }
	inline void KeepAlive() { m_stale = 0; }
	inline uint8 GetStaleCounter() const { return m_stale; }
	void LeavingZone(ZoneServer *iZS = 0, CLE_Status iOnline = CLE_Status::Offline);
	void Camp(ZoneServer *iZS = 0);

	// Login Server stuff
	inline const char *LoginServer() const { return m_source_loginserver; }
	inline uint32 LSID() const { return m_login_server_id; }
	inline uint32 LSAccountID() const { return m_login_server_id; }
	inline const char *LSName() const { return m_login_account_name; }
	inline int16 WorldAdmin() const { return m_world_admin; }
	inline const char *GetLSKey() const { return m_key; }
	inline const CLE_Status GetOnline() const { return m_online; }

	// Account stuff
	inline uint32 AccountID() const { return m_account_id; }
	inline const char *AccountName() const { return m_account_name; }
	inline int16 Admin() const { return m_admin; }
	inline void SetAdmin(uint16 iAdmin) { m_admin = iAdmin; }

	// Character info
	inline ZoneServer *Server() const { return m_zone_server; }
	inline void ClearServer() { m_zone_server = 0; }
	inline uint32 CharID() const { return m_char_id; }
	inline const char *name() const { return m_char_name; }
	inline uint32 zone() const { return m_zone; }
	inline uint16 instance() const { return m_instance; }
	inline uint8 level() const { return m_level; }
	inline uint8 class_() const { return m_class_; }
	inline uint16 race() const { return m_race; }
	inline uint8 Anon() { return m_anon; }
	inline uint8 TellsOff() const { return m_tells_off; }
	inline uint32 GuildID() const { return m_guild_id; }
	inline uint32 GuildRank() const { return m_guild_rank; }
	inline bool GuildTributeOptIn() const { return m_guild_tribute_opt_in; }
	inline void SetGuild(uint32 guild_id) { m_guild_id = guild_id; }
	inline void SetGuildTributeOptIn(bool opt) { m_guild_tribute_opt_in = opt; }
	inline bool LFG() const { return m_lfg; }
	inline uint8 GetGM() const { return m_gm; }
	inline void SetGM(uint8 igm) { m_gm = igm; }
	inline void SetZone(uint32 zone) { m_zone = zone; }
	inline bool IsLocalClient() const { return m_is_local; }
	inline uint8 GetLFGFromLevel() const { return m_lfg_from_level; }
	inline uint8 GetLFGToLevel() const { return m_lfg_to_level; }
	inline bool GetLFGMatchFilter() const { return m_lfg_match_filter; }
	inline const char *GetLFGComments() const { return m_lfg_comments; }
	inline uint8 GetClientVersion() { return m_client_version; }

	inline bool TellQueueFull() const { return m_tell_queue.size() >= RuleI(World, TellQueueSize); }
	inline bool TellQueueEmpty() const { return m_tell_queue.empty(); }
	inline void PushToTellQueue(ServerChannelMessage_Struct *scm) { m_tell_queue.push_back(scm); }
	void ProcessTellQueue();

	void SetPendingDzInvite(ServerPacket *pack) { m_dz_invite.reset(pack->Copy()); };
	std::unique_ptr<ServerPacket> GetPendingDzInvite() { return std::move(m_dz_invite); }

private:
	void ClearVars(bool iAll = false);

	const uint32 m_id;
	uint32       m_ip_address;
	CLE_Status   m_online;
	uint8        m_stale;

	// Login Server stuff
	char   m_source_loginserver[64]{}; //Loginserver we came from.
	uint32 m_login_server_id;
	char   m_login_account_name[32]{};
	char   m_key[16]{};
	int16  m_world_admin;        // Login server's suggested admin status setting
	bool   m_is_local;

	// Account stuff
	uint32 m_account_id;
	char   m_account_name[32]{};
	int16  m_admin{};

	// Character info
	ZoneServer *m_zone_server{};
	uint32 m_zone{};
	uint16 m_instance{};
	uint32 m_char_id{};
	char   m_char_name[64]{};
	uint8  m_level{};
	uint8  m_class_{};
	uint16 m_race{};
	uint8  m_anon{};
	uint8  m_tells_off{};
	uint32 m_guild_id{};
	uint32 m_guild_rank;
	bool   m_guild_tribute_opt_in{};
	bool   m_lfg{};
	uint8  m_gm{};
	uint8  m_client_version{};
	uint8  m_lfg_from_level{};
	uint8  m_lfg_to_level{};
	bool   m_lfg_match_filter{};
	char   m_lfg_comments[64]{};

	// Tell Queue -- really a vector :D
	std::vector<ServerChannelMessage_Struct *> m_tell_queue;

	std::unique_ptr<ServerPacket> m_dz_invite;
};
