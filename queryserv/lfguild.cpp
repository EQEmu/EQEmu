#include "lfguild.h"

#include "common/rulesys.h"
#include "common/strings.h"
#include "queryserv/database.h"
#include "queryserv/worldserver.h"

extern WorldServer *worldserver;
extern QSDatabase  qs_database;

PlayerLookingForGuild::PlayerLookingForGuild(char *name, char *comments, uint32 level, uint32 classes, uint32 aa_count, uint32 time_zone, uint32 time_posted)
{
	Name        = std::string(name);
	Comments    = std::string(comments);
	Level       = level;
	Class       = classes;
	AACount     = aa_count;
	TimeZone    = time_zone;
	TimePosted  = time_posted;
}

GuildLookingForPlayers::GuildLookingForPlayers(char * name, char * comments, uint32 from_level, uint32 to_level, uint32 classes, uint32 aa_count, uint32 time_zone, uint32 time_posted)
{
	Name        = std::string(name);
	Comments    = std::string(comments);
	FromLevel   = from_level;
	ToLevel     = to_level;
	Classes     = classes;
	AACount     = aa_count;
	TimeZone    = time_zone;
	TimePosted  = time_posted;
}

bool LFGuildManager::LoadDatabase()
{
	std::string query = "SELECT `type`,`name`,`comment`, "
                        "`fromlevel`, `tolevel`, `classes`, "
                        "`aacount`, `timezone`, `timeposted` FROM `lfguild`";
    auto results = qs_database.QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

    for (auto row = results.begin(); row != results.end(); ++row) {
		uint32 type = Strings::ToUnsignedInt(row[0]);
		if(type == 0)
		{
			PlayerLookingForGuild p(row[1], row[2], Strings::ToUnsignedInt(row[3]), Strings::ToUnsignedInt(row[5]), Strings::ToUnsignedInt(row[6]), Strings::ToUnsignedInt(row[7]), Strings::ToUnsignedInt(row[8]));
			Players.push_back(p);
			continue;
		}

        GuildLookingForPlayers g(row[1], row[2], Strings::ToUnsignedInt(row[3]), Strings::ToUnsignedInt(row[4]), Strings::ToUnsignedInt(row[5]), Strings::ToUnsignedInt(row[6]), Strings::ToUnsignedInt(row[7]), Strings::ToUnsignedInt(row[8]));
        Guilds.push_back(g);
	}

	return true;
}

void LFGuildManager::HandlePacket(ServerPacket *pack)
{
	char From[64];

	pack->SetReadPosition(0);
	uint32 FromZoneID = pack->ReadUInt32();
	uint32 FromInstanceID = pack->ReadUInt32();
	pack->ReadString(From);
	pack->ReadSkipBytes(4);
	uint32 SubType = pack->ReadUInt32();

	switch(SubType)
	{
		case QSG_LFGuild_PlayerMatches:
		{
			uint32 FromLevel = pack->ReadUInt32();
			uint32 ToLevel = pack->ReadUInt32();
			uint32 MinAA = pack->ReadUInt32();
			uint32 TimeZone = pack->ReadUInt32();
			uint32 Classes = pack->ReadUInt32();

			SendPlayerMatches(FromZoneID, FromInstanceID, From, FromLevel, ToLevel, MinAA, TimeZone, Classes);
			break;
		}

		case QSG_LFGuild_UpdatePlayerInfo:
		{
			char Comments[257];
			uint32 Class = pack->ReadUInt32();
			uint32 Level = pack->ReadUInt32();
			uint32 AAPoints = pack->ReadUInt32();
			pack->ReadString(Comments);
			uint32 Toggle = pack->ReadUInt32();
			uint32 TimeZone = pack->ReadUInt32();

			TogglePlayer(FromZoneID, FromInstanceID, From, Class, Level, AAPoints, Comments, Toggle, TimeZone);

			break;
		}
		case QSG_LFGuild_RequestPlayerInfo:
		{
			SendPlayerStatus(FromZoneID, FromInstanceID, From);
			break;
		}
		case QSG_LFGuild_UpdateGuildInfo:
		{
			char GuildName[33], Comments[257];

			pack->ReadString(GuildName);
			pack->ReadString(Comments);

			uint32 FromLevel = pack->ReadUInt32();
			uint32 ToLevel = pack->ReadUInt32();
			uint32 Classes = pack->ReadUInt32();
			uint32 AACount = pack->ReadUInt32();
			uint32 Toggle = pack->ReadUInt32();
			uint32 TimeZone = pack->ReadUInt32();

			ToggleGuild(FromZoneID, FromInstanceID, From, GuildName, Comments, FromLevel, ToLevel, Classes, AACount, Toggle, TimeZone);


			break;
		}
		case QSG_LFGuild_GuildMatches:
		{
			uint32 Level = pack->ReadUInt32();
			uint32 AAPoints = pack->ReadUInt32();
			uint32 TimeZone = pack->ReadUInt32();
			uint32 Class = pack->ReadUInt32();

			SendGuildMatches(FromZoneID, FromInstanceID, From, Level, AAPoints, TimeZone, Class);
			break;
		}
		case QSG_LFGuild_RequestGuildInfo:
		{
			char GuildName[33];
			pack->ReadString(GuildName);

			SendGuildStatus(FromZoneID, FromInstanceID, From, GuildName);
			break;
		}

		default:
			break;
	}


}

void LFGuildManager::SendPlayerMatches(uint32 FromZoneID, uint32 FromInstanceID, char *From, uint32 FromLevel, uint32 ToLevel, uint32 MinAA, uint32 TimeZone, uint32 Classes)
{
	std::list<PlayerLookingForGuild>::iterator it;
	std::list<PlayerLookingForGuild> Matches;

	uint32 PacketSize = strlen(From) + 21, NumberOfMatches = 0;

	for(it = Players.begin(); it != Players.end(); ++it)
	{
		uint32 bitmask = 1 << (*it).Class ;

		if(((*it).Level >= FromLevel) && ((*it).Level <= ToLevel) && ((*it).AACount >= MinAA) && (bitmask & Classes) && ((TimeZone == 0xFFFFFFFF) || (TimeZone == (*it).TimeZone)))
		{
			++NumberOfMatches;
			Matches.push_back(*it);
			PacketSize += (*it).Name.length() + (*it).Comments.length() + 18;
		}

	}

	auto pack = new ServerPacket(ServerOP_QueryServGeneric, PacketSize);

	pack->WriteUInt32(FromZoneID);
	pack->WriteUInt32(FromInstanceID);
	pack->WriteString(From);
	pack->WriteUInt32(QSG_LFGuild);
	pack->WriteUInt32(QSG_LFGuild_PlayerMatches);
	pack->WriteUInt32(NumberOfMatches);

	for(it = Matches.begin(); it != Matches.end(); ++it)
	{
		pack->WriteString((*it).Name.c_str());
		pack->WriteString((*it).Comments.c_str());
		pack->WriteUInt32((*it).Level);
		pack->WriteUInt32((*it).Class);
		pack->WriteUInt32((*it).AACount);
		pack->WriteUInt32((*it).TimeZone);

	}
	worldserver->SendPacket(pack);
	safe_delete(pack);
}

void LFGuildManager::SendGuildMatches(uint32 FromZoneID, uint32 FromInstanceID, char *From, uint32 Level, uint32 AAPoints, uint32 TimeZone, uint32 Class)
{
	std::list<GuildLookingForPlayers>::iterator it;
	std::list<GuildLookingForPlayers> Matches;

	uint32 bitmask = 1 << Class ;

	uint32 PacketSize = strlen(From) + 21, NumberOfMatches = 0;

	for(it = Guilds.begin(); it != Guilds.end(); ++it)
	{
		if((Level >= (*it).FromLevel) && (Level <= (*it).ToLevel) && (AAPoints >= (*it).AACount) && (bitmask & (*it).Classes) && (((*it).TimeZone == 0xFFFFFFFF) || (TimeZone == 0xFFFFFFFF) || (TimeZone == (*it).TimeZone)))
		{
			++NumberOfMatches;
			Matches.push_back(*it);
			PacketSize += (*it).Name.length() + (*it).Comments.length() + 6;
		}

	}

	auto pack = new ServerPacket(ServerOP_QueryServGeneric, PacketSize);

	pack->WriteUInt32(FromZoneID);
	pack->WriteUInt32(FromInstanceID);
	pack->WriteString(From);
	pack->WriteUInt32(QSG_LFGuild);
	pack->WriteUInt32(QSG_LFGuild_GuildMatches);
	pack->WriteUInt32(NumberOfMatches);

	for(it = Matches.begin(); it != Matches.end(); ++it)
	{
		pack->WriteString((*it).Name.c_str());
		pack->WriteUInt32((*it).TimeZone);
		pack->WriteString((*it).Comments.c_str());
	}
	worldserver->SendPacket(pack);
	safe_delete(pack);
}

void LFGuildManager::TogglePlayer(uint32 FromZoneID, uint32 FromInstanceID, char *From, uint32 Class, uint32 Level, uint32 AAPoints, char *Comments, uint32 Toggle, uint32 TimeZone)
{
	for(auto it = Players.begin(); it != Players.end(); ++it)
		if(!strcasecmp((*it).Name.c_str(), From)) {
			Players.erase(it);
			break;
		}

    std::string query = StringFormat("DELETE FROM `lfguild` WHERE `type` = 0 AND `name` = '%s'", From);
    auto results = qs_database.QueryDatabase(query);

	uint32 Now = time(nullptr);

	if(Toggle == 1) {
		PlayerLookingForGuild p(From, Comments, Level, Class, AAPoints, TimeZone, Now);
		Players.push_back(p);

		query = StringFormat("INSERT INTO `lfguild` "
                            "(`type`, `name`, `comment`, `fromlevel`, `tolevel`, "
                            "`classes`, `aacount`, `timezone`, `timeposted`) "
                            "VALUES (0, '%s', '%s', %u, 0, %u, %u, %u, %u)",
                            From, Comments, Level, Class, AAPoints, TimeZone, Now);
        auto results = qs_database.QueryDatabase(query);
	}

	auto pack = new ServerPacket(ServerOP_QueryServGeneric, strlen(From) + strlen(Comments) + 30);

	pack->WriteUInt32(FromZoneID);
	pack->WriteUInt32(FromInstanceID);
	pack->WriteString(From);
	pack->WriteUInt32(QSG_LFGuild);
	pack->WriteUInt32(QSG_LFGuild_RequestPlayerInfo);
	pack->WriteString(Comments);
	pack->WriteUInt32(TimeZone);
	pack->WriteUInt32(Now);
	pack->WriteUInt32(Toggle);

	worldserver->SendPacket(pack);
	safe_delete(pack);
}

void LFGuildManager::ToggleGuild(uint32 FromZoneID, uint32 FromInstanceID, char *From, char* GuildName, char *Comments, uint32 FromLevel, uint32 ToLevel, uint32 Classes, uint32 AACount, uint32 Toggle, uint32 TimeZone)
{
	for(auto it = Guilds.begin(); it != Guilds.end(); ++it)
		if(!strcasecmp((*it).Name.c_str(), GuildName))
		{
			Guilds.erase(it);
			break;
		}

    std::string query = StringFormat("DELETE FROM `lfguild` WHERE `type` = 1 AND `name` = '%s'", GuildName);
    auto results = qs_database.QueryDatabase(query);

	uint32 Now = time(nullptr);

	if(Toggle == 1)
	{
		GuildLookingForPlayers g(GuildName, Comments, FromLevel, ToLevel, Classes, AACount, TimeZone, Now);
		Guilds.push_back(g);

		query = StringFormat("INSERT INTO `lfguild` "
                            "(`type`, `name`, `comment`, `fromlevel`, `tolevel`, "
                            "`classes`, `aacount`, `timezone`, `timeposted`) "
                            "VALUES (1, '%s', '%s', %u, %u, %u, %u, %u, %u)",
                            GuildName, Comments, FromLevel, ToLevel,
                            Classes, AACount, TimeZone, Now);
		auto results = qs_database.QueryDatabase(query);

	}

	auto pack = new ServerPacket(ServerOP_LFGuildUpdate, strlen(GuildName) + strlen(Comments) + 30);

	pack->WriteString(GuildName);
	pack->WriteString(Comments);
	pack->WriteUInt32(FromLevel);
	pack->WriteUInt32(ToLevel);
	pack->WriteUInt32(Classes);
	pack->WriteUInt32(AACount);
	pack->WriteUInt32(TimeZone);
	pack->WriteUInt32(Now);
	pack->WriteUInt32(Toggle);

	worldserver->SendPacket(pack);
	safe_delete(pack);
}

void LFGuildManager::ExpireEntries()
{
	for(auto it = Players.begin(); it != Players.end(); ++it)
	{
		if((*it).TimePosted + 604800 > (uint32)time(nullptr))
            continue;

        std::string query = StringFormat("DELETE from `lfguild` WHERE `type` = 0 AND `name` = '%s'", (*it).Name.c_str());
        auto results = qs_database.QueryDatabase(query);
        if(!results.Success())

        it = Players.erase(it);
    }

	for(auto it2 = Guilds.begin(); it2 != Guilds.end(); ++it2)
	{
		if((*it2).TimePosted + 2592000 > time(nullptr))
            continue;

        std::string query = StringFormat("DELETE from `lfguild` WHERE `type` = 1 AND `name` = '%s'", (*it2).Name.c_str());
        auto results = qs_database.QueryDatabase(query);
        if(!results.Success())

        it2 = Guilds.erase(it2);
	}
}

void LFGuildManager::SendPlayerStatus(uint32 FromZoneID, uint32 FromInstanceID, char *From)
{

	std::list<PlayerLookingForGuild>::iterator it;

	for(it = Players.begin(); it != Players.end(); ++it)
	{
		if(!strcasecmp((*it).Name.c_str(), From))
		{
			auto pack =
			    new ServerPacket(ServerOP_QueryServGeneric, strlen(From) + (*it).Comments.length() + 30);

			pack->WriteUInt32(FromZoneID);
			pack->WriteUInt32(FromInstanceID);
			pack->WriteString(From);
			pack->WriteUInt32(QSG_LFGuild);
			pack->WriteUInt32(QSG_LFGuild_RequestPlayerInfo);
			pack->WriteString((*it).Comments.c_str());
			pack->WriteUInt32((*it).TimeZone);
			pack->WriteUInt32((*it).TimePosted);
			pack->WriteUInt32(1);

			worldserver->SendPacket(pack);
			safe_delete(pack);
			break;
		}
	}
}

void LFGuildManager::SendGuildStatus(uint32 FromZoneID, uint32 FromInstanceID, char *From, char *GuildName)
{
	std::list<GuildLookingForPlayers>::iterator it;

	for(it = Guilds.begin(); it != Guilds.end(); ++it)
	{
		if(!strcasecmp((*it).Name.c_str(), GuildName))
		{
			auto pack =
			    new ServerPacket(ServerOP_QueryServGeneric, strlen(From) + (*it).Comments.length() + 42);

			pack->WriteUInt32(FromZoneID);
			pack->WriteUInt32(FromInstanceID);
			pack->WriteString(From);
			pack->WriteUInt32(QSG_LFGuild);
			pack->WriteUInt32(QSG_LFGuild_RequestGuildInfo);
			pack->WriteString((*it).Comments.c_str());
			pack->WriteUInt32((*it).FromLevel);
			pack->WriteUInt32((*it).ToLevel);
			pack->WriteUInt32((*it).Classes);
			pack->WriteUInt32((*it).AACount);
			pack->WriteUInt32((*it).TimeZone);
			pack->WriteUInt32((*it).TimePosted);

			worldserver->SendPacket(pack);
			safe_delete(pack);
			break;
		}
	}
}

