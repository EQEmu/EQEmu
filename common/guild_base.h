#pragma once

#include "common/guilds.h"
#include "common/repositories/guild_members_repository.h"
#include "common/repositories/guilds_repository.h"
#include "common/timer.h"

#include <map>
#include <string>
#include <vector>

struct DefaultPermissionStruct {
	GuildAction id;
	uint32      value;
};

struct DefaultRankNamesStruct {
	uint32      id;
	std::string name;
};

struct GuildTributeStruct {
	Timer    timer;
	uint32   id_1;
	uint32   id_2;
	uint32   id_1_tier;
	uint32   id_2_tier;
	uint32   favor;
	uint32   time_remaining;
	uint32   enabled;
	bool     send_timer;
};

class TributeData {
public:
	//this level data stored in regular byte order and must be flipped before sending
	TributeLevel_Struct tiers[MAX_TRIBUTE_TIERS];
	uint8       tier_count;
	uint32      unknown;
	std::string name;
	std::string description;
	bool        is_guild;	//is a guild tribute item
};

class Database;

class CharGuildInfo
{
	public:
		//fields from `characer_`
		uint32	    char_id;
		std::string	char_name;
		uint8	    class_;
		uint16	    level;
		uint32	    time_last_on;
		uint32	    zone_id;

		//fields from `guild_members`
		uint32	    guild_id;
		uint8	    rank;
		bool	    tribute_enable;
		uint32	    total_tribute;
		uint32	    last_tribute;		//timestamp
		bool	    banker;
		bool	    alt;
		std::string	public_note;
		bool	    online;
};

//this object holds guild functionality shared between world and zone.
class BaseGuildManager
{
	public:
		BaseGuildManager();
		virtual ~BaseGuildManager();

		//this must be called before doing anything else with this object
		BaseGuildManager * SetDatabase(Database *db)
		{
			m_db = db;

			return this;
		}

		BaseGuildManager * SetContentDatabase(Database *db)
		{
			m_content_db = db;

			return this;
		}

		bool LoadGuilds();
		virtual bool RefreshGuild(uint32 guild_id);

		//guild edit actions.
		uint32	CreateGuild(std::string name, uint32 leader_char_id);
		bool    StoreGuildDB(uint32 guild_id);
		bool	DeleteGuild(uint32 guild_id);
		bool	RenameGuild(uint32 guild_id, std::string name);
		bool	SetGuildMOTD(uint32 guild_id, std::string motd, std::string setter);
		bool	SetGuildURL(uint32 guild_id, std::string URL);
		bool	SetGuildChannel(uint32 guild_id, std::string Channel);

		//character edit actions
		bool	SetGuildLeader(uint32 guild_id, uint32 leader_char_id);
		bool	SetGuild(uint32 charid, uint32 guild_id, uint8 rank);
		bool	SetGuildRank(uint32 charid, uint8 rank);
		bool	SetBankerFlag(uint32 charid, bool is_banker);
		bool	ForceRankUpdate(uint32 charid);
		bool	GetAltFlag(uint32 CharID);
		bool	SetAltFlag(uint32 charid, bool is_alt);
		bool	GetBankerFlag(uint32 CharID, bool compat_mode = false);
		bool    GetGuildBankerStatus(uint32 guild_id, uint32 guild_rank);
		bool	SetTributeFlag(uint32 charid, bool enabled);
		bool	SetPublicNote(uint32 charid, std::string public_note);
		uint32  UpdateDbGuildFavor(uint32 guild_id, uint32 enabled);
		bool    UpdateDbGuildTributeEnabled(uint32 guild_id, uint32 enabled);
		bool    UpdateDbMemberTributeEnabled(uint32 guild_id, uint32 char_id, uint32 enabled);
		bool    UpdateDbTributeTimeRemaining(uint32 guild_id, uint32 enabled);
		uint32	UpdateDbMemberFavor(uint32 guild_id, uint32 char_id, uint32 favor);
		bool    UpdateDbMemberOnline(uint32 char_id, bool status);

		//queries
		bool	GetCharInfo(const char *char_name, CharGuildInfo &into);
		bool	GetCharInfo(uint32 char_id, CharGuildInfo &into);
		bool	GetEntireGuild(uint32 guild_id, std::vector<CharGuildInfo *> &members);	//caller is responsible for deleting each pointer in the resulting vector.
		bool	GuildExists(uint32 guild_id) const;
		bool	GetGuildMOTD(uint32 guild_id, char *motd_buffer, char *setter_buffer) const;
		bool	GetGuildURL(uint32 GuildID, char *URLBuffer) const;
		bool	GetGuildChannel(uint32 GuildID, char *ChannelBuffer) const;
        bool    IsCharacterInGuild(uint32 character_id, uint32 guild_id = 0);
        bool	GetGuildNameByID(uint32 guild_id, std::string& into) const;
        bool	IsGuildLeader(uint32 guild_id, uint32 char_id) const;
        bool	CheckGMStatus(uint32 guild_id, uint8 status) const;
        bool	CheckPermission(uint32 guild_id, uint8 rank, GuildAction act) const;
        bool	UpdateDbBankerFlag(uint32 charid, bool is_banker);
        GuildsListMessaging_Struct MakeGuildList();
        uint8	GetDisplayedRank(uint32 guild_id, uint8 rank, uint32 char_id) const;
        uint32	GetGuildIDByName(const char *GuildName);
		uint32  GetGuildIDByCharacterID(uint32 character_id);
        uint32	FindGuildByLeader(uint32 leader) const;
        uint32	NumberInGuild(uint32 guild_id);
        uint32	DoesAccountContainAGuildLeader(uint32 AccountID);
        const char* GetRankName(uint32 guild_id, uint8 rank) const;
        const char* GetGuildName(uint32 guild_id) const;
        std::string GetGuildNameByID(uint32 guild_id) const;
        std::string GetGuildRankName(uint32 guild_id, uint8 rank) const;
        std::vector<GuildMembersRepository::GuildMembers> GetGuildMembers(uint32 guild_id);

	protected:
		//the methods which must be defined by base classes.
		virtual void SendGuildRefresh(uint32 guild_id, bool name, bool motd, bool rank, bool relation) = 0;
		virtual void SendCharRefresh(uint32 old_guild_id, uint32 guild_id, uint32 charid) = 0;
		virtual void SendRankUpdate(uint32 CharID) = 0;
		virtual void SendGuildDelete(uint32 guild_id) = 0;

		uint32	UpdateDbCreateGuild(std::string name, uint32 leader_char_id);
		bool    UpdateDbDeleteGuild(uint32 guild_id, bool local_delete = true, bool db_delete = true);
		bool	UpdateDbRenameGuild(uint32 guild_id, std::string name);
		bool	UpdateDbGuildLeader(uint32 guild_id, uint32 leader_char_id);
		bool	UpdateDbGuildMOTD(uint32 guild_id, std::string motd, std::string setter);
		bool	UpdateDbGuildURL(uint32 GuildID, std::string URL);
		bool	UpdateDbGuildChannel(uint32 GuildID, std::string Channel);
		bool	UpdateDbGuild(uint32 charid, uint32 guild_id, uint8 rank);
		bool	UpdateDbGuildRank(uint32 charid, uint8 rank);
		bool	UpdateDbAltFlag(uint32 charid, bool is_alt);
		bool	UpdateDbTributeFlag(uint32 charid, bool enabled);
		bool	UpdateDbPublicNote(uint32 charid, std::string public_note);
		bool	QueryWithLogging(std::string query, const char *errmsg);
		bool	LocalDeleteGuild(uint32 guild_id);

		struct RankInfo
		{
			RankInfo();
			std::string rank_name;
		};
		struct Functions
		{
			uint32 id;
			uint32 perm_id;
			uint32 guild_id;
			uint32 perm_value;
		};
	public:
		class GuildInfo {
		public:
			GuildInfo();
			std::string   name;
			std::string   motd;
			std::string   motd_setter;
			std::string   url;
			std::string   channel;
			uint32        leader;
			uint8         minstatus;
			std::string   rank_names[GUILD_MAX_RANK + 1];
			Functions     functions[GUILD_MAX_FUNCTIONS + 1];
			GuildTributeStruct tribute;
		};
	virtual BaseGuildManager::GuildInfo* GetGuildByGuildID(uint32 guild_id);
	uint32 GetGuildTributeTimeRemaining(uint32 guild_id);

	protected:
		std::map<uint32, GuildInfo *> m_guilds;	//we own the pointers in this map
		void ClearGuilds();	//clears internal structure

		Database *m_db;
		Database *m_content_db;

		bool _StoreGuildDB(uint32 guild_id);
		GuildInfo* _CreateGuild(uint32 guild_id, std::string guild_name, uint32 leader_char_id, uint8 minstatus, std::string guild_motd, std::string motd_setter, std::string Channel, std::string URL, uint32 favour);
        GuildsRepository::Guilds CreateGuildRepoFromGuildInfo(uint32 guild_id, BaseGuildManager::GuildInfo& in);
};
