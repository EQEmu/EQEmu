/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
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
