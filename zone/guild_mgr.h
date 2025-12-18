#pragma once

#include "common/guild_base.h"
#include "common/repositories/guild_bank_repository.h"
#include "common/types.h"
#include "zone/petitions.h"

#include <list>
#include <map>

//extern GuildRanks_Struct guilds[512];
//extern ZoneDatabase database;

#define PBUFFER 50
#define MBUFFER 50

class Client;
class ServerPacket;

struct GuildBankItems
{
	std::map<int32, GuildBankRepository::GuildBank> main_area{};
	std::map<int32, GuildBankRepository::GuildBank> deposit_area{};
};

struct GuildBank
{
	uint32         guild_id;
	GuildBankItems items{};

	GuildBank()
	{
		guild_id = 0;
	}
};

enum {
	GuildBankBulkItems   = 0,
	GuildBankItemUpdate  = 1,
	GuildBankPromote     = 3,
	GuildBankViewItem    = 4,
	GuildBankDeposit     = 5,
	GuildBankPermissions = 6,
	GuildBankWithdraw    = 7,
	GuildBankSplitStacks = 8,
	GuildBankMergeStacks = 9,
	GuildBankAcknowledge = 10
};

enum {
	GuildBankDepositArea = 0,
	GuildBankMainArea    = 1
};

enum {
	GuildBankBankerOnly     = 0,
	GuildBankSingleMember   = 1,
	GuildBankPublicIfUsable = 2,
	GuildBankPublic         = 3
};

class ZoneGuildManager : public BaseGuildManager {
public:
	~ZoneGuildManager(void);

	//called by worldserver when it receives a message from world.
	void ProcessWorldPacket(ServerPacket *pack);

	void ListGuilds(Client *c, std::string search_criteria = std::string()) const;
	void ListGuilds(Client *c, uint32 guild_id = 0) const;
	void DescribeGuild(Client *c, uint32 guild_id) const;
	bool IsActionABankAction(GuildAction action);
	bool SetGuild(Client *client, uint32 guild_id, uint8 rank);

	uint8 *MakeGuildMembers(uint32 guild_id, const char* prefix_name, uint32& length);
	void  SendToWorldMemberLevelUpdate(uint32 guild_id, uint32 level, std::string player_name);
	void  SendToWorldMemberPublicNote(uint32 guild_id, std::string player_name, std::string public_note);
	void  SendToWorldMemberRemove(uint32 guild_id, std::string player_name);
	void  SendToWorldMemberAdd(uint32 guild_id, uint32 char_id, uint32 level, uint32 _class, uint32 rank, uint32 zone_id, std::string player_name);
	void  SendToWorldGuildChannel(uint32 guild_id, std::string channel);
	void  SendToWorldGuildURL(uint32 guild_id, std::string url);
	void  SendToWorldSendGuildList();
	void  SendToWorldMemberRankUpdate(uint32 guild_id, uint32 rank, uint32 banker, uint32 alt, bool no_update, const char *player_name);
	void  SendToWorldSendGuildMembersList(uint32 guild_id);
	bool  RemoveMember(uint32 guild_id, uint32 char_id, std::string player_name);
	void  MemberAdd(uint32 guild_id, uint32 char_id, uint32 level, uint32 _class, uint32 rank, uint32 zone_id, std::string player_name);
	bool  MemberRankUpdate(uint32 guild_id, uint32 rank, uint32 banker, uint32 alt, bool no_update, const char *player_name);

	void RecordInvite(uint32 char_id, uint32 guild_id, uint8 rank);
	bool VerifyAndClearInvite(uint32 char_id, uint32 guild_id, uint8 rank);
	void SendGuildMemberUpdateToWorld(const char *MemberName, uint32 GuildID, uint16 ZoneID, uint32 LastSeen);
	void RequestOnlineGuildMembers(uint32 FromID, uint32 GuildID);
	void UpdateRankPermission(uint32 gid, uint32 charid, uint32 fid, uint32 rank, uint32 value);
	void SendPermissionUpdate(uint32 guild_id, uint32 rank, uint32 function_id, uint32 value);
	void UpdateRankName(uint32 gid, uint32 rank, std::string rank_name);
	void SendRankName(uint32 guild_id, uint32 rank, std::string rank_name);
	void SendAllRankNames(uint32 guild_id, uint32 char_id);
	GuildInfo* GetGuildByGuildID(uint32 guild_id);
	virtual void SendGuildRefresh(uint32 guild_id, bool name, bool motd, bool rank, bool relation);

protected:
	virtual void SendCharRefresh(uint32 old_guild_id, uint32 guild_id, uint32 charid);
	virtual void SendRankUpdate(uint32 CharID);
	virtual void SendGuildDelete(uint32 guild_id);

	std::map<uint32, std::pair<uint32, uint8> > m_inviteQueue;	//map from char ID to guild,rank

private:
	uint32 id;
};


class GuildBankManager
{

public:
	~GuildBankManager();
	void SendGuildBank(Client *c);
	bool AddItem(GuildBankRepository::GuildBank &guild_bank_item, Client* client);
	int Promote(uint32 GuildID, int SlotID, Client* c);
	void SetPermissions(uint32 GuildID, uint16 SlotID, uint32 Permissions, const char *MemberName, Client* c);
	std::unique_ptr<EQ::ItemInstance> GetItem(uint32 guild_id, uint16 area, uint16 slot_id, uint32 quantity);
	bool DeleteItem(uint32 GuildID, uint16 Area, uint16 SlotID, uint32 Quantity, Client* c);
	bool HasItem(uint32 guild_id, uint32 item_id);
	bool IsAreaFull(uint32 guild_id, uint16 area);
	int32 NextFreeBankSlot(uint32 guild_id, uint32 area);
	bool MergeStacks(uint32 GuildID, uint16 SlotID, Client* c);
	bool SplitStack(uint32 GuildID, uint16 SlotID, uint32 Quantity, Client* c);
	//bool AllowedToWithdraw(uint32 GuildID, uint16 Area, uint16 SlotID, const char *Name);
	void SendGuildBankItemUpdate(uint32 guild_id, int32 slot_id, uint32 area, bool display, Client* c);
	std::shared_ptr<GuildBank> GetGuildBank(uint32 guild_id);

private:
	bool IsLoaded(uint32 GuildID);
	void Load(uint32 GuildID);
	void UpdateItemQuantity(uint32 GuildID, uint16 Area, uint16 SlotID, uint32 Quantity);

	std::list<std::shared_ptr<GuildBank>> banks{};
};

extern ZoneGuildManager guild_mgr;
extern GuildBankManager *GuildBanks;
