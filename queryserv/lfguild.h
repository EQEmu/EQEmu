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

#include "common/servertalk.h"
#include "common/types.h"

#include <list>
#include <string>

class PlayerLookingForGuild
{
public:
	PlayerLookingForGuild(char *Name, char *Comments, uint32 Level, uint32 Class, uint32 AACount, uint32 Timezone, uint32 TimePosted);

	std::string Name;
	std::string Comments;
	uint32 Level;
	uint32 Class;
	uint32 AACount;
	uint32 TimeZone;
	uint32 TimePosted;
};

class GuildLookingForPlayers
{

public:
	GuildLookingForPlayers(char *Name, char *Comments, uint32 FromLevel, uint32 ToLevel, uint32 Classes, uint32 AACount, uint32 Timezone, uint32 TimePosted);

	std::string Name;
	std::string Comments;
	uint32 FromLevel;
	uint32 ToLevel;
	uint32 Classes;
	uint32 AACount;
	uint32 TimeZone;
	uint32 TimePosted;
};

class LFGuildManager
{

public:
	bool LoadDatabase();
	void HandlePacket(ServerPacket *pack);
	void ExpireEntries();

	static LFGuildManager* Instance()
	{
		static LFGuildManager instance;
		return &instance;
	}
private:
	void SendPlayerMatches(uint32 FromZoneID, uint32 FromInstanceID, char *From, uint32 FromLevel, uint32 ToLevel, uint32 MinAA, uint32 TimeZone, uint32 Classes);
	void SendGuildMatches(uint32 FromZoneID, uint32 FromInstanceID, char *From, uint32 Level, uint32 AAPoints, uint32 TimeZone, uint32 Class);
	void TogglePlayer(uint32 FromZoneID, uint32 FromInstanceID, char *From, uint32 Class, uint32 Level, uint32 AAPoints, char *Comments, uint32 Toggle, uint32 TimeZone);
	void ToggleGuild(uint32 FromZoneID, uint32 FromInstanceID, char *From, char* GuildName, char *Comments, uint32 FromLevel, uint32 ToLevel, uint32 Classes, uint32 AACount, uint32 Toggle, uint32 TimeZone);
	void SendPlayerStatus(uint32 FromZoneID, uint32 FromInstanceID, char *From);
	void SendGuildStatus(uint32 FromZoneID, uint32 FromInstanceID, char *From, char *GuildName);
	std::list<PlayerLookingForGuild> Players;
	std::list<GuildLookingForPlayers> Guilds;
};
