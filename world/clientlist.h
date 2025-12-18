#pragma once

#include "common/eq_packet_structs.h"
#include "common/event/timer.h"
#include "common/json/json.h"
#include "common/linked_list.h"
#include "common/net/console_server_connection.h"
#include "common/rulesys.h"
#include "common/servertalk.h"
#include "common/timer.h"

#include <string>
#include <vector>

class Client;
class ZoneServer;
class WorldTCPConnection;
class ClientListEntry;
class ServerPacket;
struct ServerClientList_Struct;

class ClientList {
public:
	ClientList();
	~ClientList();

	void Process();

	//from old ClientList
	void	Add(Client* client);
	Client*	Get(uint32 ip, uint16 port);
	Client* FindByAccountID(uint32 account_id);

	void	ZoneBootup(ZoneServer* zs);
	void	RemoveCLEReferances(ClientListEntry* cle);
	std::map<uint32, ClientListEntry*> GetGuildClientsWithTributeOptIn(uint32 guild_id);
	std::map<uint32, ClientListEntry*> GetGuildClientList(uint32 guild_id);
	//Client* GetClient(uint32 char_id);

	//from ZSList

	void	SendWhoAll(uint32 fromid,const char* to, int16 admin, Who_All_Struct* whom, WorldTCPConnection* connection);
	void	SendFriendsWho(ServerFriendsWho_Struct *FriendsWho, WorldTCPConnection* connection);
	void	SendOnlineGuildMembers(uint32 FromID, uint32 GuildID);
	void	SendClientVersionSummary(const char *Name);
	void	SendLFGMatches(ServerLFGMatchesRequest_Struct *LFGMatchesRequest);
	void	ConsoleSendWhoAll(const char* to, int16 admin, Who_All_Struct* whom, WorldTCPConnection* connection);
	void	SendCLEList(const int16& admin, const char* to, WorldTCPConnection* connection, const char* search_criteria = 0);

	bool	SendPacket(const char* to, ServerPacket* pack);

	void	ClientUpdate(ZoneServer* zoneserver, ServerClientList_Struct* scl);
	void	CLERemoveZSRef(ZoneServer* iZS);
	ClientListEntry* CheckAuth(uint32 loginserver_account_id, const char* key);
	ClientListEntry* FindCharacter(const char* name);
	ClientListEntry* FindCLEByAccountID(uint32 iAccID);
	ClientListEntry* FindCLEByCharacterID(uint32 iCharID);
	void	GetCLEIP(uint32 in_ip);
	void	DisconnectByIP(uint32 in_ip);
	void	CLCheckStale();
	void	CLEKeepAlive(uint32 numupdates, uint32* wid);
	void	CLEAdd(uint32 login_server_id, const char* login_server_name, const char* login_name, const char* login_key, int16 world_admin = AccountStatus::Player, uint32 ip_address = 0, uint8 is_local=0);
	void	UpdateClientGuild(uint32 char_id, uint32 guild_id);
	bool    IsAccountInGame(uint32 iLSID);

	int GetClientCount();
	void GetClients(const char *zone_name, std::vector<ClientListEntry *> &into);

	void GetClientList(Json::Value &response, bool full_list = false);
	void GetGuildClientList(Json::Value& response, uint32 guild_id);

	void SendCharacterMessage(uint32_t character_id, int chat_type, const std::string& message);
	void SendCharacterMessage(const std::string& character_name, int chat_type, const std::string& message);
	void SendCharacterMessage(ClientListEntry* character, int chat_type, const std::string& message);
	void SendCharacterMessageID(uint32_t character_id, int chat_type, int eqstr_id, std::initializer_list<std::string> args = {});
	void SendCharacterMessageID(const std::string& character_name, int chat_type, int eqstr_id, std::initializer_list<std::string> args = {});
	void SendCharacterMessageID(ClientListEntry* character, int chat_type, int eqstr_id, std::initializer_list<std::string> args = {});

	void AddToZoneServerCaches(ClientListEntry* cle);
	void RebuildZoneServerCaches();

	std::vector<uint32_t> GetGuildZoneServers(uint32 guild_id);
	inline std::vector<uint32_t> GetZoneServersWithGMs()
	{
		return {m_gm_zone_server_ids.begin(), m_gm_zone_server_ids.end()};
	}

	static ClientList* Instance()
	{
		static ClientList instance;
		return &instance;
	}

private:
	void OnTick(EQ::Timer *t);
	inline uint32 GetNextCLEID() { return NextCLEID++; }

	//this is the list of people actively connected to zone
	LinkedList<Client*> list;

	//this is the list of people in any zone, not nescesarily connected to world
	Timer	CLStale_timer;
	uint32 NextCLEID;
	LinkedList<ClientListEntry *> clientlist;


	std::unique_ptr<EQ::Timer> m_tick;

	// Zone server routing caches
	Timer                                                      m_poll_cache_timer;
	std::unordered_set<uint32_t>                               m_gm_zone_server_ids;
	std::unordered_map<uint32_t, std::unordered_set<uint32_t>> m_guild_zone_server_ids;
};
