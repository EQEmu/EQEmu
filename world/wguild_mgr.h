#pragma once

#include "common/types.h"
#include "common/guild_base.h"

class Client;
class ServerPacket;

class WorldGuildManager : public BaseGuildManager {
public:

	//called by zoneserver when it receives a guild message from zone.
	void ProcessZonePacket(ServerPacket *pack);
	void Process();
	bool LoadTributes();
	void SendGuildTributeFavorAndTimer(uint32 guild_id);
	void SendGuildTributeFavorAndTimer(uint32 guild_id, uint32 favor, uint32 time);
	uint32 GetGuildTributeCost(uint32 guild_id);
	virtual bool RefreshGuild(uint32 guild_id);

protected:
	virtual void SendGuildRefresh(uint32 guild_id, bool name, bool motd, bool rank, bool relation);
	virtual void SendCharRefresh(uint32 old_guild_id, uint32 guild_id, uint32 charid);
	virtual void SendRankUpdate(uint32 CharID) { return; }
	virtual void SendGuildDelete(uint32 guild_id);

	
	//map<uint32, uint32> m_tribute;	//map from guild ID to current tribute ammount
};

extern WorldGuildManager guild_mgr;
