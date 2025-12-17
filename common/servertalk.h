#pragma once

#include "common/eq_packet_structs.h"
#include "common/guilds.h"
#include "common/net/packet.h"
#include "common/packet_functions.h"
#include "common/types.h"

#include "cereal/cereal.hpp"
#include "cereal/archives/binary.hpp"
#include "cereal/types/chrono.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"
#include "glm/vec4.hpp"

#define SERVER_TIMEOUT	45000	// how often keepalive gets sent
#define INTERSERVER_TIMER					10000
#define LoginServer_StatusUpdateInterval	15000
#define LoginServer_AuthStale				60000
#define AUTHCHANGE_TIMEOUT					900	// in seconds

#define ServerOP_KeepAlive			0x0001	// packet to test if port is still open
#define ServerOP_ChannelMessage		0x0002	// broadcast/guildsay
#define ServerOP_SetZone			0x0003	// client -> server zoneinfo
#define ServerOP_ShutdownAll		0x0004	// exit(0);
#define ServerOP_ZoneShutdown		0x0005	// unload all data, goto sleep mode
#define ServerOP_ZoneBootup			0x0006	// come out of sleep mode and load zone specified
#define ServerOP_ZoneStatus			0x0007	// Shows status of all zones
#define ServerOP_SetConnectInfo		0x0008	// Tells server address and port #
#define ServerOP_EmoteMessage		0x0009	// Worldfarts
#define ServerOP_ClientList			0x000A	// Update worldserver's client list, for #whos
#define ServerOP_Who				0x000B	// #who
#define ServerOP_ZonePlayer			0x000C	// #zone, or #summon
#define ServerOP_KickPlayer			0x000D	// #kick
#define ServerOP_RefreshGuild		0x000E	// Notice to all zoneservers to refresh their guild cache for ID# in packet (ServerGuildRefresh_Struct)
#define ServerOP_VoiceMacro		0x000F
//#define ServerOP_GuildInvite		0x0010
#define ServerOP_DeleteGuild		0x0011	// ServerGuildID_Struct
#define ServerOP_GuildRankUpdate	0x0012
#define ServerOP_GuildCharRefresh	0x0013
#define ServerOP_GuildMemberUpdate	0x0014
#define ServerOP_RequestOnlineGuildMembers	0x0015
#define ServerOP_OnlineGuildMembersResponse	0x0016
#define ServerOP_LFGuildUpdate		0x0017

#define ServerOP_FlagUpdate			0x0018	// GM flag updated for character, refresh the memory cache
#define ServerOP_GMGoto				0x0019
#define ServerOP_MultiLineMsg		0x001A
#define ServerOP_Lock				0x001B	// For #lock/#unlock inside server
#define ServerOP_Motd				0x001C	// For changing MoTD inside server.
#define ServerOP_Uptime				0x001D
#define ServerOP_Petition			0x001E
#define	ServerOP_KillPlayer			0x001F
#define ServerOP_UpdateGM			0x0020
#define ServerOP_RezzPlayer			0x0021
#define ServerOP_ZoneReboot			0x0022
#define ServerOP_ZoneToZoneRequest	0x0023
#define ServerOP_AcceptWorldEntrance 0x0024
#define ServerOP_ZAAuth				0x0025
#define ServerOP_ZAAuthFailed		0x0026
#define ServerOP_ZoneIncClient		0x0027	// Incoming client
#define ServerOP_ClientListKA		0x0028
#define ServerOP_ChangeWID			0x0029
#define ServerOP_IPLookup			0x002A
#define ServerOP_LockZone			0x002B
#define ServerOP_ItemStatus			0x002C
#define ServerOP_OOCMute			0x002D
#define ServerOP_Revoke				0x002E
#define	ServerOP_WebInterfaceCall   0x002F
#define ServerOP_GroupIDReq			0x0030
#define ServerOP_GroupIDReply		0x0031
#define ServerOP_GroupLeave			0x0032	// for disbanding out of zone folks
#define ServerOP_RezzPlayerAccept	0x0033
#define ServerOP_SpawnCondition		0x0034
#define ServerOP_SpawnEvent			0x0035
#define ServerOP_SetLaunchName		0x0036
#define ServerOP_RezzPlayerReject	0x0037
#define ServerOP_SpawnPlayerCorpse	0x0038
#define ServerOP_Consent			0x0039
#define ServerOP_Consent_Response	0x003a
#define ServerOP_ForceGroupUpdate	0x003b
#define ServerOP_OOZGroupMessage	0x003c
#define ServerOP_DisbandGroup		0x003d //for disbanding a whole group cross zone
#define ServerOP_GroupJoin			0x003e //for joining ooz folks
#define ServerOP_UpdateSpawn		0x003f
#define ServerOP_SpawnStatusChange	0x0040
#define ServerOP_DropClient         0x0041	// DropClient
#define ServerOP_IsOwnerOnline		0x0042
#define ServerOP_DepopAllPlayersCorpses	0x0060
#define ServerOP_QGlobalUpdate		0x0061
#define ServerOP_QGlobalDelete		0x0062
#define ServerOP_DepopPlayerCorpse	0x0063
#define ServerOP_RequestTellQueue	0x0064 // client asks for it's tell queues
#define ServerOP_ChangeSharedMem	0x0065
#define ServerOP_WebInterfaceEvent  0x0066
#define ServerOP_WebInterfaceSubscribe 0x0067
#define ServerOP_WebInterfaceUnsubscribe 0x0068
#define ServerOP_GuildPermissionUpdate			  0x0069
#define ServerOP_GuildTributeUpdate				  0x0070
#define ServerOP_GuildRankNameChange			  0x0071
#define ServerOP_GuildTributeActivate			  0x0072
#define ServerOP_GuildTributeOptInToggle		  0x0073
#define ServerOP_GuildTributeFavAndTimer		  0x0074
#define ServerOP_RequestGuildActiveTributes		  0x0075
#define ServerOP_RequestGuildFavorAndTimer		  0x0076
#define ServerOP_GuildTributeUpdateDonations      0x0077
#define ServerOP_GuildMemberLevelUpdate           0x0078
#define ServerOP_GuildMemberPublicNote            0x0079
#define ServerOP_GuildMemberRemove                0x007A
#define ServerOP_GuildMemberAdd                   0x007B
#define ServerOP_GuildChannel                     0x007C
#define ServerOP_GuildURL                         0x007D
#define ServerOP_GuildSendGuildList				  0x007E
#define ServerOP_GuildMembersList                 0x007F

#define ServerOP_ParcelDelivery                   0x0090
#define ServerOP_ParcelPrune                      0x0091

#define ServerOP_RaidAdd			0x0100 //in use
#define ServerOP_RaidRemove			0x0101 //in use
#define	ServerOP_RaidDisband		0x0102 //in use
#define ServerOP_RaidLockFlag		0x0103 //in use
#define ServerOP_RaidGroupLeader	0x0104 //in use
#define ServerOP_RaidLeader			0x0105 //in use
#define	ServerOP_RaidGroupSay		0x0106 //in use
#define	ServerOP_RaidSay			0x0107 //in use
#define	ServerOP_DetailsChange		0x0108 //in use

#define ServerOP_UpdateGroup		0x010A //in use
#define ServerOP_RaidGroupDisband	0x010B //in use
#define ServerOP_RaidChangeGroup	0x010C //in use
#define ServerOP_RaidGroupAdd		0x010D
#define ServerOP_RaidGroupRemove	0x010E
#define ServerOP_GroupInvite		0x010F
#define ServerOP_GroupFollow		0x0110
#define ServerOP_GroupFollowAck		0x0111
#define ServerOP_GroupCancelInvite	0x0112
#define ServerOP_RaidMOTD			0x0113
#define ServerOP_RaidNote           0x0114

#define ServerOP_TraderMessaging    0x0120
#define ServerOP_BazaarPurchase     0x0121
#define ServerOP_BuyerMessaging     0x0122

#define ServerOP_InstanceUpdateTime			0x014F
#define ServerOP_AdventureRequest			0x0150
#define ServerOP_AdventureRequestAccept		0x0151
#define ServerOP_AdventureRequestDeny		0x0152
#define ServerOP_AdventureRequestCreate		0x0153
#define ServerOP_AdventureData				0x0154
#define ServerOP_AdventureDataClear			0x0155
#define ServerOP_AdventureCreateDeny		0x0156
#define ServerOP_AdventureDataRequest		0x0157
#define ServerOP_AdventureClickDoor			0x0158
#define ServerOP_AdventureClickDoorReply	0x0159
#define ServerOP_AdventureClickDoorError	0x015a
#define ServerOP_AdventureLeave				0x015b
#define ServerOP_AdventureLeaveReply		0x015c
#define ServerOP_AdventureLeaveDeny			0x015d
#define ServerOP_AdventureCountUpdate		0x015e
#define ServerOP_AdventureZoneData			0x015f
#define ServerOP_AdventureAssaCountUpdate	0x0160
#define ServerOP_AdventureFinish			0x0161
#define ServerOP_AdventureLeaderboard		0x0162

#define ServerOP_WhoAll				0x0210
#define ServerOP_FriendsWho			0x0211
#define ServerOP_LFGMatches			0x0212
#define ServerOP_LFPUpdate			0x0213
#define ServerOP_LFPMatches			0x0214
#define ServerOP_ClientVersionSummary 0x0215

// dz
#define ServerOP_DzAddRemoveMember            0x0450
#define ServerOP_DzRemoveAllMembers           0x0451
#define ServerOP_DzSetSecondsRemaining        0x0452
#define ServerOP_DzDurationUpdate             0x0453
#define ServerOP_DzSetCompass                 0x0454
#define ServerOP_DzSetSafeReturn              0x0455
#define ServerOP_DzSetZoneIn                  0x0456
#define ServerOP_DzSwapMembers                0x0457
#define ServerOP_DzGetMemberStatuses          0x0458
#define ServerOP_DzUpdateMemberStatus         0x0459
#define ServerOP_DzLeaderChanged              0x045a
#define ServerOP_DzExpireWarning              0x045b
#define ServerOP_DzCreated                    0x045c
#define ServerOP_DzDeleted                    0x045d
#define ServerOP_DzSetSwitchID                0x045e
#define ServerOP_DzMovePC                     0x045f
#define ServerOP_DzLock                       0x0460
#define ServerOP_DzReplayOnJoin               0x0461
#define ServerOP_DzLockout                    0x0462
#define ServerOP_DzLockoutDuration            0x0463
#define ServerOP_DzCharacterLockout           0x0464
#define ServerOP_DzAddPlayer                  0x0465
#define ServerOP_DzSaveInvite                 0x0466
#define ServerOP_DzRequestInvite              0x0467
#define ServerOP_DzMakeLeader                 0x0468
#define ServerOP_DzGetBulkMemberStatuses      0x0469

#define ServerOP_LSInfo				0x1000
#define ServerOP_LSStatus			0x1001
#define ServerOP_LSClientAuthLeg	0x1002
#define ServerOP_LSFatalError		0x1003
#define ServerOP_SystemwideMessage	0x1005
#define ServerOP_ListWorlds			0x1006
#define ServerOP_PeerConnect		0x1007
#define ServerOP_NewLSInfo			0x1008
#define ServerOP_LSRemoteAddr		0x1009
#define ServerOP_LSAccountUpdate	0x100A
#define ServerOP_LSClientAuth		0x100B

#define ServerOP_EncapPacket		0x2007	// Packet within a packet
#define ServerOP_WorldListUpdate	0x2008
#define ServerOP_WorldListRemove	0x2009
#define ServerOP_TriggerWorldListRefresh	0x200A
#define ServerOP_WhoAllReply		0x2010
#define ServerOP_SetWorldTime		0x200B
#define ServerOP_GetWorldTime		0x200C
#define ServerOP_SyncWorldTime		0x200E
#define ServerOP_RefreshCensorship	0x200F

#define ServerOP_LSZoneInfo			0x3001
#define ServerOP_LSZoneStart		0x3002
#define ServerOP_LSZoneBoot			0x3003
#define ServerOP_LSZoneShutdown		0x3004
#define ServerOP_LSZoneSleep		0x3005
#define ServerOP_LSPlayerLeftWorld	0x3006
#define ServerOP_LSPlayerJoinWorld	0x3007
#define ServerOP_LSPlayerZoneChange	0x3008

#define	ServerOP_UsertoWorldReqLeg	0xAB00
#define	ServerOP_UsertoWorldRespLeg	0xAB01
#define	ServerOP_UsertoWorldReq		0xAB02
#define	ServerOP_UsertoWorldResp	0xAB03

#define ServerOP_LauncherConnectInfo	0x3000
#define ServerOP_LauncherZoneRequest	0x3001
#define ServerOP_LauncherZoneStatus		0x3002
#define ServerOP_DoZoneCommand		0x3003

#define ServerOP_CameraShake 0x4000
#define ServerOP_HotReloadQuests 0x4001
#define ServerOP_QueryServGeneric 0x4002
#define ServerOP_UCSMailMessage 0x4003
#define ServerOP_UCSMessage 0x4004
#define ServerOP_UCSServerStatusReply 0x4005
#define ServerOP_UCSServerStatusRequest 0x4006
#define ServerOP_UpdateSchedulerEvents 0x4007
#define ServerOP_DiscordWebhookMessage 0x4008

#define ServerOP_ServerReloadRequest 0x4100

#define ServerOP_CZDialogueWindow 0x4500
#define ServerOP_CZLDoNUpdate 0x4501
#define ServerOP_CZMarquee 0x4502
#define ServerOP_CZMessage 0x4503
#define ServerOP_CZMove 0x4504
#define ServerOP_CZSetEntityVariable 0x4505
#define ServerOP_CZSignal 0x4506
#define ServerOP_CZSpell 0x4507
#define ServerOP_CZTaskUpdate 0x4508
#define ServerOP_CZClientMessageString 0x4509

#define ServerOP_WWDialogueWindow 0x4750
#define ServerOP_WWLDoNUpdate 0x4751
#define ServerOP_WWMarquee 0x4752
#define ServerOP_WWMessage 0x4753
#define ServerOP_WWMove 0x4754
#define ServerOP_WWSetEntityVariable 0x4755
#define ServerOP_WWSignal 0x4756
#define ServerOP_WWSpell 0x4757
#define ServerOP_WWTaskUpdate 0x4758

// player events
#define ServerOP_QSSendQuery 0x5000
#define ServerOP_PlayerEvent 0x5100
#define ServerOP_SendPlayerEventSettings 0x5101

enum {
	CZUpdateType_Character,
	CZUpdateType_Group,
	CZUpdateType_Raid,
	CZUpdateType_Guild,
	CZUpdateType_Expedition,
	CZUpdateType_ClientName,
	CZUpdateType_NPC
};

enum {
	CZLDoNUpdateSubtype_AddLoss,
	CZLDoNUpdateSubtype_AddPoints,
	CZLDoNUpdateSubtype_AddWin,
	CZLDoNUpdateSubtype_RemoveLoss,
	CZLDoNUpdateSubtype_RemoveWin,
};

enum {
	CZMoveUpdateSubtype_MoveZone,
	CZMoveUpdateSubtype_MoveZoneInstance
};

enum {
	CZSpellUpdateSubtype_Cast,
	CZSpellUpdateSubtype_Remove
};

enum {
	CZTaskUpdateSubtype_ActivityReset,
	CZTaskUpdateSubtype_ActivityUpdate,
	CZTaskUpdateSubtype_AssignTask,
	CZTaskUpdateSubtype_DisableTask,
	CZTaskUpdateSubtype_EnableTask,
	CZTaskUpdateSubtype_FailTask,
	CZTaskUpdateSubtype_RemoveTask
};

enum {
	WWLDoNUpdateType_AddLoss,
	WWLDoNUpdateType_AddPoints,
	WWLDoNUpdateType_AddWin,
	WWLDoNUpdateType_RemoveLoss,
	WWLDoNUpdateType_RemoveWin
};

enum {
	WWMoveUpdateType_MoveZone,
	WWMoveUpdateType_MoveZoneInstance
};

enum {
	WWSetEntityVariableUpdateType_Character,
	WWSetEntityVariableUpdateType_NPC
};

enum {
	WWSignalUpdateType_Character,
	WWSignalUpdateType_NPC
};

enum {
	WWSpellUpdateType_Cast,
	WWSpellUpdateType_Remove
};

enum {
	WWTaskUpdateType_ActivityReset,
	WWTaskUpdateType_ActivityUpdate,
	WWTaskUpdateType_AssignTask,
	WWTaskUpdateType_DisableTask,
	WWTaskUpdateType_EnableTask,
	WWTaskUpdateType_FailTask,
	WWTaskUpdateType_RemoveTask
};

/* Query Serv Generic Packet Flag/Type Enumeration */
enum { QSG_LFGuild = 0 };
enum {	QSG_LFGuild_PlayerMatches = 0, QSG_LFGuild_UpdatePlayerInfo, QSG_LFGuild_RequestPlayerInfo, QSG_LFGuild_UpdateGuildInfo, QSG_LFGuild_GuildMatches,
	QSG_LFGuild_RequestGuildInfo };


enum {
	UserToWorldStatusWorldUnavail = 0,
	UserToWorldStatusSuccess = 1,
	UserToWorldStatusSuspended = -1,
	UserToWorldStatusBanned = -2,
	UserToWorldStatusWorldAtCapacity = -3,
	UserToWorldStatusAlreadyOnline = -4
};

/************ PACKET RELATED STRUCT ************/
class ServerPacket
{
public:
	~ServerPacket() { safe_delete_array(pBuffer); }
	ServerPacket(uint16 in_opcode = 0, uint32 in_size = 0) {
		this->compressed = false;
		size = in_size;
		opcode = in_opcode;
		if (size == 0) {
			pBuffer = 0;
		}
		else {
			pBuffer = new uchar[size];
			memset(pBuffer, 0, size);
		}
		_wpos = 0;
		_rpos = 0;
	}

	ServerPacket(uint16 in_opcode, const EQ::Net::Packet &p) {
		this->compressed = false;
		size = (uint32)p.Length();
		opcode = in_opcode;
		if (size == 0) {
			pBuffer = 0;
		}
		else {
			pBuffer = new uchar[size];
			memcpy(pBuffer, p.Data(), size);
		}
		_wpos = 0;
		_rpos = 0;
	}

	ServerPacket* Copy() {
		ServerPacket* ret = new ServerPacket(this->opcode, this->size);
		if (this->size)
			memcpy(ret->pBuffer, this->pBuffer, this->size);
		ret->compressed = this->compressed;
		ret->InflatedSize = this->InflatedSize;
		return ret;
	}

	void WriteUInt8(uint8 value) { *(uint8 *)(pBuffer + _wpos) = value; _wpos += sizeof(uint8); }
	void WriteInt8(uint8_t value) { *(uint8_t *)(pBuffer + _wpos) = value; _wpos += sizeof(uint8_t); }
	void WriteUInt32(uint32 value) { *(uint32 *)(pBuffer + _wpos) = value; _wpos += sizeof(uint32); }
	void WriteInt32(int32_t value) { *(int32_t *)(pBuffer + _wpos) = value; _wpos += sizeof(int32_t); }

	void WriteString(const char * str) { uint32 len = static_cast<uint32>(strlen(str)) + 1; memcpy(pBuffer + _wpos, str, len); _wpos += len; }

	uint8 ReadUInt8() { uint8 value = *(uint8 *)(pBuffer + _rpos); _rpos += sizeof(uint8); return value; }
	uint32 ReadUInt32() { uint32 value = *(uint32 *)(pBuffer + _rpos); _rpos += sizeof(uint32); return value; }
	void ReadString(char *str) { uint32 len = static_cast<uint32>(strlen((char *)(pBuffer + _rpos))) + 1; memcpy(str, pBuffer + _rpos, len); _rpos += len; }

	uint32 GetWritePosition() { return _wpos; }
	uint32 GetReadPosition() { return _rpos; }
	void SetWritePosition(uint32 Newwpos) { _wpos = Newwpos; }
	void WriteSkipBytes(uint32 count) { _wpos += count; }
	void ReadSkipBytes(uint32 count) { _rpos += count; }
	void SetReadPosition(uint32 Newrpos) { _rpos = Newrpos; }

	uint32	size;
	uint16	opcode;
	uchar*	pBuffer;
	uint32	_wpos;
	uint32	_rpos;
	bool	compressed;
	uint32	InflatedSize;
	uint32	destination;
};

#pragma pack(1)

struct SPackSendQueue {
	uint16 size;
	uchar buffer[0];
};

struct ServerZoneStateChange_Struct {
	uint32 zone_server_id;
	uint32 zone_id;
	uint16 instance_id;
	bool   is_static;
	char   admin_name[64];
};

struct ServerZoneIncomingClient_Struct {
	uint32	zoneid;		// in case the zone shut down, boot it back up
	uint16	instanceid; // instance id if it exists for booting up
	uint32	ip;			// client's IP address
	uint32	wid;		// client's WorldID#
	uint32	accid;
	int16	admin;
	uint32	charid;
	uint32 lsid;
	bool	tellsoff;
	char	charname[64];
	char	lskey[30];
};

struct ServerZoneDropClient_Struct
{
	uint32 lsid;
};

struct ServerChangeWID_Struct {
	uint32	charid;
	uint32	newwid;
};
struct SendGroup_Struct{
	uint8	grouptotal;
	uint32	zoneid;
	char	leader[64];
	char	thismember[64];
	char	members[5][64];
};

struct ServerGroupFollow_Struct {
	uint32 CharacterID;
	GroupGeneric_Struct gf;
};

struct ServerGroupFollowAck_Struct {
	char Name[64];
};


struct ServerChannelMessage_Struct {
	char deliverto[64];
	char to[64];
	char from[64];
	uint8 fromadmin;
	bool noreply;
	uint16 chan_num;
	uint32 guilddbid;
	uint8 language;
	uint8 lang_skill;
	uint8 queued; // 0 = not queued, 1 = queued, 2 = queue full, 3 = offline
	char message[0];
};

struct ServerEmoteMessage_Struct {
	char	to[64];
	uint32	guilddbid;
	int16	minstatus;
	uint32	type;
	char	message[0];
};

struct ServerVoiceMacro_Struct {
	char	From[64];
	union {
		char	To[64];
		uint32	GroupID;
		uint32	RaidID;
	};
	uint32	Type;
	uint32	Voice;
	uint32	MacroNumber;
};

struct ServerClientList_Struct {
	uint8	remove;
	uint32	wid;
	uint32	IP;
	uint32	zone;
	uint16	instance_id;
	int16	Admin;
	uint32	charid;
	char	name[64];
	uint32	AccountID;
	char	AccountName[30];
	uint32	LSAccountID;
	char	lskey[30];
	uint16	race;
	uint8	class_;
	uint8	level;
	uint8	anon;
	bool	tellsoff;
	uint32	guild_id;
	uint32	guild_rank;
	bool    guild_tribute_opt_in;
	bool	LFG;
	uint8	gm;
	uint8	ClientVersion;
	uint8	LFGFromLevel;
	uint8	LFGToLevel;
	bool	LFGMatchFilter;
	char	LFGComments[64];
};

struct ServerClientListKeepAlive_Struct {
	uint32	numupdates;
	uint32	wid[0];
};

struct ServerZonePlayer_Struct {
	char	adminname[64];
	int16	adminrank;
	uint8	ignorerestrictions;
	char	name[64];
	char	zone[25];
	uint32	instance_id;
	float	x_pos;
	float	y_pos;
	float	z_pos;
};

struct RezzPlayer_Struct {
	uint32	dbid;
	uint32	exp;
	uint16	rezzopcode;
	//char	packet[160];
	Resurrect_Struct rez;
};

struct ServerZoneReboot_Struct {
//	char	ip1[250];
	char	ip2[250];
	uint16	port;
	uint32	zoneid;
};

struct SetZone_Struct {
	uint16	instanceid;
	uint32	zoneid;
	bool	staticzone;
};

struct ServerKickPlayer_Struct {
	char adminname[64];
	int16 adminrank;
	char name[64];
	uint32 account_id;
};

struct ServerLSInfo_Struct {
	char	name[201]; // name the worldserver wants
	char	address[250];			// DNS address of the server
	char	account[31];			// account name for the worldserver
	char	password[31];			// password for the name
	char	protocolversion[25];	// Major protocol version number
	char	serverversion[64];		// minor server software version number
	uint8	servertype; // 0=world, 1=chat, 2=login, 3=MeshLogin
};

struct LoginserverNewWorldRequest {
	char	server_long_name[201]; // name the worldserver wants
	char	server_short_name[50]; // shortname the worldserver wants
	char	remote_ip_address[125];			// DNS address of the server
	char	local_ip_address[125];			// DNS address of the server
	char	account_name[31];			// account name for the worldserver
	char	account_password[31];			// password for the name
	char	protocol_version[25];	// Major protocol version number
	char	server_version[64];		// minor server software version number
	uint8	server_process_type; // 0=world, 1=chat, 2=login, 3=MeshLogin
};

struct LoginserverAccountUpdate {            // for updating info on login server
	char   world_account[31];            // account name for the worldserver
	char   world_password[31];            // password for the name
	uint32 user_account_id; // player account ID
	char   user_account_name[31];            // player account name
	char   user_account_password[51];            // player account password
	char   user_email[101]; // player account email address
};

struct LoginserverWorldStatusUpdate {
	int32 status;
	int32 num_players;
	int32 num_zones;
};
struct LoginserverZoneInfoUpdate {
	uint32 zone;
	uint16 count;
	uint32 zone_wid;
};
struct ZoneBoot_Struct {
	uint32 zone;
	uint32 instance;
	char compile_time[25];
	uint32 zone_wid;
};
struct ZoneShutdown_Struct {
	uint32 zone;
	uint32 instance;
	uint32 zone_wid;
};
struct ServerLSZoneSleep_Struct {
	uint32 zone;
	uint32 instance;
	uint32 zone_wid;
};

struct ServerLSPlayerJoinWorld_Struct {
	uint32 lsaccount_id;
	char key[30];
};

struct ServerLSPlayerLeftWorld_Struct {
	uint32 lsaccount_id;
	char key[30];
};

struct ServerLSPlayerZoneChange_Struct {
	uint32 lsaccount_id;
	uint32 from; // 0 = world
	uint32 to; // 0 = world
};

struct ClientAuth {
	uint32 loginserver_account_id; // ID# in login server's db
	char   loginserver_name[64];
	char   account_name[30]; // username in login server's db
	char   key[30]; // the key the client will present
	uint8  lsadmin; // login server admin level
	int16  is_world_admin; // login's suggested worldadmin level setting for this user, up to the world if they want to obey it
	uint32 ip_address;
	uint8  is_client_from_local_network; // 1 if the client is from the local network

	template<class Archive>
	void serialize(Archive &ar)
	{
		ar(
			loginserver_account_id,
			loginserver_name,
			account_name,
			key,
			lsadmin,
			is_world_admin,
			ip_address,
			is_client_from_local_network
		);
	}
};

struct ClientAuthLegacy {
	uint32 loginserver_account_id; // ID# in login server's db
	char   loginserver_account_name[30]; // username in login server's db
	char   key[30]; // the key the client will present
	uint8  loginserver_admin_level; // login server admin level
	int16  is_world_admin; // login's suggested worldadmin level setting for this user, up to the world if they want to obey it
	uint32 ip_address;
	uint8  is_client_from_local_network; // 1 if the client is from the local network

	template<class Archive>
	void serialize(Archive &ar)
	{
		ar(
			loginserver_account_id,
			loginserver_account_name,
			key,
			loginserver_admin_level,
			is_world_admin,
			ip_address,
			is_client_from_local_network
		);
	}
};

struct ServerSystemwideMessage {
	uint32	lsaccount_id;
	char	key[30];		// sessionID key for verification
	uint32	type;
	char	message[0];
};

struct ServerLSPeerConnect {
	uint32	ip;
	uint16	port;
};

struct ServerConnectInfo {
	char	address[250];
	char	local_address[250];
	uint16	port;
	uint32 process_id;
};

struct ServerGMGoto_Struct {
	char	myname[64];
	char	gotoname[64];
	int16	admin;
};

struct ServerMultiLineMsg_Struct {
	char	to[64];
	char	message[0];
};

struct ServerLock_Struct {
	char character_name[64];
	bool is_locked;
};

struct ServerMotd_Struct {
	char	myname[64]; // User that set the motd
	char	motd[512]; // the new MoTD
};

struct ServerUptime_Struct {
	uint32	zoneserverid;	// 0 for world
	char	adminname[64];
};

struct ServerPetitionUpdate_Struct {
	uint32 petid; // Petition Number
	uint8 status; // 0x00 = ReRead DB -- 0x01 = Checkout -- More? Dunno... lol
};

struct ServerWhoAll_Struct {
	int16 admin;
	uint32 fromid;
	char from[64];
	char whom[64];
	uint16 wrace; // FF FF = no race
	uint16 wclass; // FF FF = no class
	uint16 lvllow; // FF FF = no numbers
	uint16 lvlhigh; // FF FF = no numbers
	uint16 gmlookup; // FF FF = not doing /who all gm
};

struct ServerFriendsWho_Struct {
	uint32 FromID;
	char FromName[64];
	char FriendsString[1];
};

struct ServerKillPlayer_Struct {
	char gmname[64];
	char target[64];
	int16 admin;
};

struct ServerUpdateGM_Struct {
	char gmname[64];
	bool gmstatus;
};

struct ServerEncapPacket_Struct {
	uint32	ToID;	// ID number of the LWorld on the other server
	uint16	opcode;
	uint16	size;
	uchar	data[0];
};

struct ZoneToZone_Struct {
	char	name[64];
	uint32	guild_id;
	uint32	requested_zone_id;
	uint32	requested_instance_id;
	uint32	current_zone_id;
	uint32	current_instance_id;
	int8	response;
	int16	admin;
	uint8	ignorerestrictions;
};

struct WorldToZone_Struct {
	uint32	account_id;
	int8	response;
};
struct WorldShutDown_Struct {
	uint32	time;
	uint32	interval;
};
struct ServerSyncWorldList_Struct {
	uint32	RemoteID;
	uint32	ip;
	int32	status;
	char	name[201];
	char	address[250];
	char	account[31];
	uint32	accountid;
	uint8	authlevel;
	uint8	servertype;		// 0=world, 1=chat, 2=login
	uint32	adminid;
	uint8	greenname;
	uint8	showdown;
	int32	num_players;
	int32	num_zones;
	bool	placeholder;
};

struct UsertoWorldRequestLegacy {
	uint32 lsaccountid;
	uint32 worldid;
	uint32 FromID;
	uint32 ToID;
	char   IPAddr[64];
};

struct UsertoWorldRequest {
	uint32 lsaccountid;
	uint32 worldid;
	uint32 FromID; // appears to be unused today
	uint32 ToID; // appears to be unused today
	char   IPAddr[64];
	char   login[64];
};

struct UsertoWorldResponseLegacy {
	uint32 lsaccountid;
	uint32 worldid;
	int8   response; // -3) World Full, -2) Banned, -1) Suspended, 0) Denied, 1) Allowed
	uint32 FromID;
	uint32 ToID;
};

struct UsertoWorldResponse {
	uint32 lsaccountid;
	uint32 worldid;
	int8   response; // -3) World Full, -2) Banned, -1) Suspended, 0) Denied, 1) Allowed
	uint32 FromID; // appears to be unused today
	uint32 ToID; // appears to be unused today
	char   login[64];
};

// generic struct to be used for alot of simple zone->world questions
struct ServerGenericWorldQuery_Struct {
	char	from[64];	// charname the query is from
	int16	admin;		// char's admin level
	char	query[0];	// text of the query
};

struct ServerLockZone_Struct {
	uint8	op;
	char	adminname[64];
	uint16	zoneID;
};

struct RevokeStruct {
	char adminname[64];
	char name[64];
	bool toggle; //0 off 1 on
};

struct ServerGroupIDReply_Struct {
	uint32 start;	//a range of group IDs to use.
	uint32 end;
};

struct ServerGroupLeave_Struct {
	uint32 zoneid;
	uint16 instance_id;
	uint32 gid;
	char member_name[64];	//kick this member from the group
};

struct ServerGroupJoin_Struct {
	uint32 zoneid;
	uint16 instance_id;
	uint32 gid;
	char member_name[64];	//this person is joining the group
};

struct ServerForceGroupUpdate_Struct {
	uint32 origZoneID;
	uint16 instance_id;
	uint32 gid;
};

struct ServerGroupChannelMessage_Struct {
	uint32 zoneid;
	uint16 instanceid;
	uint32 groupid;
	char from[64];
	char message[0];
};

struct ServerDisbandGroup_Struct {
	uint32 zoneid;
	uint16 instance_id;
	uint32 groupid;
};

struct SimpleName_Struct{
	char name[64];
};

struct ServerSpawnCondition_Struct {
	uint32 zoneID;
	uint32 instanceID;
	uint16 condition_id;
	int16 value;
};

struct ServerSpawnEvent_Struct {
	uint32	zoneID;
	uint32	event_id;
};

//zone->world
struct LaunchName_Struct {
	char launcher_name[32];
	char zone_name[16];
};

struct LauncherConnectInfo {
	char name[64];
};

typedef enum {
	ZR_Start,
	ZR_Restart,
	ZR_Stop
} ZoneRequestCommands;
struct LauncherZoneRequest {
	uint8 command;
	char short_name[33];
	uint16 port;
};

struct LauncherZoneStatus {
	char short_name[33];
	uint32 start_count;
	uint8 running;
};


struct ServerGuildID_Struct {
	uint32 guild_id;
};

struct ServerGuildRefresh_Struct {
	uint32 guild_id;
	uint8 name_change;
	uint8 motd_change;
	uint8 rank_change;
	uint8 relation_change;
};

struct ServerGuildCharRefresh_Struct {
	uint32 guild_id;
	uint32 old_guild_id;
	uint32 char_id;
};

struct ServerGuildRankUpdate_Struct {
	uint32 guild_id;
	char   member_name[64];
	uint32 rank;
	uint32 banker;
	uint32 alt;
	bool   no_update;
};

struct ServerGuildMemberUpdate_Struct {
	uint32 guild_id;
	char   member_name[64];
	uint32 zone_id;
	uint32 last_seen;
};

struct ServerGuildPermissionUpdate_Struct {
	uint32 guild_id;
	char   member_name[64];
	uint32 rank;
	uint32 function_id;
	uint32 function_value;
};

struct ServerGuildRankNameChange {
	uint32 guild_id;
	uint32 rank;
	char   rank_name[51]; //RoF2 has 51 max.
};

struct SpawnPlayerCorpse_Struct {
	uint32 player_corpse_id;
	uint32 zone_id;
};

struct ServerOP_Consent_Struct {
	char grantname[64];
	char ownername[64];
	char zonename[32];
	uint8 permission;
	uint32 zone_id;
	uint16 instance_id;
	uint8 consent_type; // 0 = normal, 1 = group, 2 = raid, 3 = guild
	uint32 consent_id;
};

struct ReloadTasks_Struct {
	uint8 reload_type;
	uint32 task_id;
};

struct ServerDepopAllPlayersCorpses_Struct
{
	uint32 CharacterID;
	uint32 ZoneID;
	uint16 InstanceID;
};

struct ServerDepopPlayerCorpse_Struct
{
	uint32 DBID;
	uint32 ZoneID;
	uint16 InstanceID;
};

struct ServerRaidGeneralAction_Struct {
	uint32 zoneid;
	uint16 instance_id;
	uint32 rid;
	uint32 gid;
	char playername[64];
};

struct ServerRaidGroupAction_Struct { //add / remove depends on opcode.
	char membername[64]; //member who's adding / leaving
	uint32 gid; //group id to send to.
	uint32 rid; //raid id to send to.
};

struct ServerRaidMessage_Struct {
	uint32 rid;
	uint32 gid;
	uint8 language;
	uint8 lang_skill;
	char from[64];
	char message[0];
};

struct ServerRaidMOTD_Struct {
	uint32 rid;
	char motd[1024];
};

struct ServerRaidNote_Struct {
	uint32 rid;
};

struct ServerLFGMatchesRequest_Struct {
	uint32	FromID;
	uint8	QuerierLevel;
	char	FromName[64];
	uint8	FromLevel;
	uint8	ToLevel;
	uint32	Classes;
};

struct ServerLFGMatchesResponse_Struct {
	char Name[64];
	uint8 Level;
	uint8 Class_;
	uint16 Zone;
	uint16 GuildID;
	uint16 Anon;
	char Comments[64];
};

struct ServerLFPUpdate_Struct {
	uint32 LeaderID;
	uint8 Action;
	uint8 MatchFilter;
	uint32	FromLevel;
	uint32	ToLevel;
	uint32	Classes;
	char	Comments[64];
	GroupLFPMemberEntry Members[MAX_GROUP_MEMBERS];
};

struct ServerLFPMatchesResponse_Struct {
	uint32	FromLevel;
	uint32	ToLevel;
	uint32	Classes;
	GroupLFPMemberEntry Members[MAX_GROUP_MEMBERS];
	char Comments[64];
};

struct ServerLFPMatchesRequest_Struct {
	uint32	FromID;
	uint8	FromLevel;
	uint8	ToLevel;
	uint8	QuerierClass;
	uint8	QuerierLevel;
	char	FromName[64];
};

struct UpdateSpawnTimer_Struct {
	uint32 id;
	uint32 duration;
};

struct ServerInstanceUpdateTime_Struct
{
	uint16 instance_id;
	uint32 new_duration;
};

struct ServerSpawnStatusChange_Struct {
	uint32 id;
	bool   new_status;
	uint32 instance_id;
};

struct ServerQGlobalUpdate_Struct
{
	uint32 id;
	char name[64];
	char value[128];
	uint32 npc_id;
	uint32 char_id;
	uint32 zone_id;
	uint32 expdate;
	uint32 from_zone_id;
	uint32 from_instance_id;
};

struct ServerQGlobalDelete_Struct
{
	char name[64];
	uint32 npc_id;
	uint32 char_id;
	uint32 zone_id;
	uint32 from_zone_id;
	uint32 from_instance_id;
};

struct ServerRequestOnlineGuildMembers_Struct
{
	uint32	FromID;
	uint32	GuildID;
};

struct ServerRequestClientVersionSummary_Struct
{
	char Name[64];
};

struct ServerAdventureRequest_Struct
{
	char leader[64];
	uint32 template_id;
	uint8 type;
	uint8 risk;
	uint8 member_count;
};

struct ServerAdventureRequestDeny_Struct
{
	char leader[64];
	char reason[512];
};

struct ServerAdventureRequestAccept_Struct
{
	char leader[64];
	char text[512];
	uint32 theme;
	uint32 id;
	uint32 member_count;
};

struct ServerAdventureRequestCreate_Struct
{
	char leader[64];
	uint32 theme;
	uint32 id;
	uint32 member_count;
};

struct ServerSendAdventureData_Struct
{
	char player[64];
	char text[512];
	uint32 time_left;
	uint32 time_to_enter;
	uint32 risk;
	float x;
	float y;
	int count;
	int total;
	uint32 zone_in_id;
	uint32 zone_in_object;
	uint16 instance_id;
	uint32 finished_adventures;
};

struct ServerFinishedAdventures_Struct
{
	uint32 zone_in_id;
	uint32 zone_in_object;
};

struct ServerPlayerClickedAdventureDoor_Struct
{
	char player[64];
	int id;
	int zone_id;
};

struct ServerPlayerClickedAdventureDoorReply_Struct
{
	char player[64];
	int zone_id;
	int instance_id;
	float x;
	float y;
	float z;
	float h;
};

struct ServerAdventureCount_Struct
{
	uint16 instance_id;
};

struct ServerAdventureCountUpdate_Struct
{
	char player[64];
	int count;
	int total;
};

struct ServerZoneAdventureDataReply_Struct
{
	uint16 instance_id;
	int count;
	int total;
	int type;
	int data_id;
	int assa_count;
	int assa_x;
	int assa_y;
	int assa_z;
	int assa_h;
	int dest_x;
	int dest_y;
	int dest_z;
	int dest_h;
};

struct ServerAdventureFinish_Struct
{
	char player[64];
	bool win;
	int points;
	int theme;
};

struct ServerLeaderboardRequest_Struct
{
	char player[64];
	uint8 type;
};

struct ServerCameraShake_Struct
{
	uint32 duration; // milliseconds
	float intensity;
};

struct ServerMailMessageHeader_Struct {
	char from[64];
	char to[64];
	char subject[128];
	char message[0];
};

struct DiscordWebhookMessage_Struct {
	uint32 webhook_id;
	char message[2000];
};

struct CZClientMessageString_Struct {
	uint32 string_id;
	uint16 chat_type;
	char   client_name[64];
	uint32 args_size;
	char   args[1]; // null delimited
};

struct CZDialogueWindow_Struct {
	uint8 update_type; // 0 - Character, 1 - Group, 2 - Raid, 3 - Guild, 4 - Expedition, 5 - Character Name
	int   update_identifier; // Character ID, Group ID, Raid ID, Guild ID, or Expedition ID based on update type, 0 for Character Name
	char  message[4096];
	char  client_name[64]; // Only used by Character Name Type, else empty
};

struct CZLDoNUpdate_Struct {
	uint8  update_type; // 0 - Character, 1 - Group, 2 - Raid, 3 - Guild, 4 - Expedition, 5 - Character Name
	uint8  update_subtype; // 0 - Loss, 1 - Points, 2 - Win
	int    update_identifier; // Character ID, Group ID, Raid ID, Guild ID, or Expedition ID based on update type, 0 for Character Name
	uint32 theme_id;
	int    points; // Only used in Points Subtype, else 1
	char   client_name[64]; // Only used by Character Name Type, else empty
};

struct CZMarquee_Struct {
	uint8  update_type; // 0 - Character, 1 - Group, 2 - Raid, 3 - Guild, 4 - Expedition, 5 - Character Name
	int    update_identifier; // Character ID, Group ID, Raid ID, Guild ID, or Expedition ID based on update type, 0 for Character Name
	uint32 type;
	uint32 priority;
	uint32 fade_in;
	uint32 fade_out;
	uint32 duration;
	char   message[512];
	char   client_name[64]; // Only used by Character Name Type, else empty
};

struct CZMessage_Struct {
	uint8  update_type; // 0 - Character, 1 - Group, 2 - Raid, 3 - Guild, 4 - Expedition, 5 - Character Name
	int    update_identifier; // Group ID, Raid ID, Guild ID, or Expedition ID based on update type, 0 for Character Name
	uint32 type;
	char   message[512];
	char   client_name[64]; // Only used by Character Name Type, else empty
};

struct CZMove_Struct {
	std::string client_name       = std::string(); // Only used by Character Name Type, else empty
	glm::vec4   coordinates       = glm::vec4(0.f); // XYZ or XYZH, heading is optional, defaults to 0.
	uint16      instance_id       = 0; // Only used by Move Zone Instance, else 0
	uint32      update_identifier = 0; // Character ID, Group ID, Raid ID, Guild ID, or Expedition ID based on update type, 0 for Character Name
	uint8       update_type; // 0 - Character, 1 - Group, 2 - Raid, 3 - Guild, 4 - Expedition, 5 - Character Name
	uint8       update_subtype; // 0 - Move Zone, 1 - Move Zone Instance
	std::string zone_short_name   = std::string(); // Only used by Move Zone, else empty
};

struct CZSetEntityVariable_Struct {
	uint8 update_type; // 0 - Character, 1 - Group, 2 - Raid, 3 - Guild, 4 - Expedition, 5 - Character Name, 6 - NPC
	int   update_identifier; // Group ID, Raid ID, Guild ID, Expedition ID, or NPC ID based on update type, 0 for Character Name
	char  variable_name[256];
	char  variable_value[256];
	char  client_name[64]; // Only used by Character Type, else empty
};

struct CZSignal_Struct {
	uint8 update_type; // 0 - Character, 1 - Group, 2 - Raid, 3 - Guild, 4 - Expedition, 5 - Character Name, 6 - NPC
	int   update_identifier; // Character ID, Group ID, Raid ID, Guild ID, Expedition ID, or NPC ID based on update type, 0 for Character Name
	int   signal_id;
	char  client_name[64]; // Only used by Character Name Type, else empty
};

struct CZSpell_Struct {
	uint8  update_type; // 0 - Character, 1 - Group, 2 - Raid, 3 - Guild, 4 - Expedition, 5 - Character Name
	uint8  update_subtype; // 0 - Cast Spell, 1 - Remove Spell
	int    update_identifier; // Character ID, Group ID, Raid ID, Guild ID, or Expedition ID based on update type, 0 for Character Name
	uint32 spell_id;
	char   client_name[64]; // Only used by Character Name Type, else empty
};

struct CZTaskUpdate_Struct {
	uint8  update_type; // 0 - Character, 1 - Group, 2 - Raid, 3 - Guild, 4 - Expedition, 5 - Character Name
	uint8  update_subtype; // 0 - Activity Reset, 1 - Activity Update, 2 - Assign Task, 3 - Disable Task, 4 - Enable Task, 5 - Fail Task, 6 - Remove Task
	int    update_identifier; // Character ID, Group ID, Raid ID, Guild ID, or Expedition ID based on update type, 0 for Character Name
	uint32 task_identifier;
	int    task_subidentifier; // Activity ID for Activity Reset and Activity Update, NPC Entity ID for Assign Task, else -1
	int    update_count; // Only used by Activity Update, else 1
	bool   enforce_level_requirement; // Only used by Assign Task
	char   client_name[64]; // Only used by Character Name Type, else empty
};

struct WWDialogueWindow_Struct {
	char  message[4096];
	uint8 min_status;
	uint8 max_status;
};

struct WWLDoNUpdate_Struct {
	uint8  update_type; // 0 - Loss, 1 - Points, 2 - Win
	uint32 theme_id;
	int    points; // Only used in Points Subtype, else 1
	uint8  min_status;
	uint8  max_status;
};

struct WWMarquee_Struct {
	uint32 type;
	uint32 priority;
	uint32 fade_in;
	uint32 fade_out;
	uint32 duration;
	char   message[512];
	uint8  min_status;
	uint8  max_status;
};

struct WWMessage_Struct {
	uint32 type;
	char   message[512];
	uint8  min_status;
	uint8  max_status;
};

struct WWMove_Struct {
	uint8  update_type; // 0 - Move Zone, 1 - Move Zone Instance
	char   zone_short_name[32]; // Used with Move Zone
	uint16 instance_id; // Used with Move Zone Instance
	uint8  min_status;
	uint8  max_status;
};


struct WWSetEntityVariable_Struct {
	uint8 update_type; // 0 - Character, 1 - NPC
	char  variable_name[256];
	char  variable_value[256];
	uint8 min_status;
	uint8 max_status;
};

struct WWSignal_Struct {
	uint8 update_type; // 0 - Character, 1 - NPC
	int   signal_id;
	uint8 min_status;
	uint8 max_status;
};

struct WWSpell_Struct {
	uint8  update_type; // 0 - Cast Spell, 1 - Remove Spell
	uint32 spell_id;
	uint8  min_status;
	uint8  max_status;
};

struct WWTaskUpdate_Struct {
	uint8  update_type; // 0 - Activity Reset, 1 - Activity Update, 2 - Assign Task, 3 - Disable Task, 4 - Enable Task, 5 - Fail Task, 6 - Remove Task
	uint32 task_identifier;
	int    task_subidentifier; // Activity ID for Activity Reset and Activity Update, NPC Entity ID for Assign Task, else -1
	int    update_count; // Update Count for Activity Update, else 1
	bool   enforce_level_requirement; // Only used by Assign Task, else false
	uint8  min_status;
	uint8  max_status;
};

struct ReloadWorld_Struct {
	uint8 global_repop;
};

struct HotReloadQuestsStruct {
	char zone_short_name[200];
};

struct ServerRequestTellQueue_Struct {
	char name[64];
};

struct ServerIsOwnerOnline_Struct {
	char   name[64];
	uint32 corpse_id;
	uint16 zone_id;
	uint8  online;
	uint32 account_id;
};

struct UCSServerStatus_Struct {
	uint8 available; // non-zero=true, 0=false
	union {
		struct {
			uint16 port;
			uint16 unused;
		};
		uint32 timestamp;
	};
};

struct ServerCharacterID_Struct {
	uint32_t char_id;
};

struct ServerDzLeaderID_Struct {
	uint32 dz_id;
	uint32 leader_id;
};

struct ServerDzMemberStatus_Struct {
	uint32 dz_id;
	uint32 sender_zone_id;
	uint16 sender_instance_id;
	uint8  status; // 0: unknown 1: Online 2: Offline 3: In Dynamic Zone 4: Link Dead
	uint32 character_id;
};

struct ServerDzMemberStatusEntry_Struct {
	uint32 character_id;
	uint8  online_status; // 0: unknown 1: Online 2: Offline 3: In Dynamic Zone 4: Link Dead
};

struct ServerDzMemberStatuses_Struct {
	uint32 dz_id;
	uint32 count;
	ServerDzMemberStatusEntry_Struct entries[0];
};

struct ServerDzCerealData_Struct {
	uint16_t zone_id;
	uint16_t inst_id;
	uint32_t cereal_size;
	char     cereal_data[1];
};

struct ServerDzMovePC_Struct {
	uint32 dz_id;
	uint16 sender_zone_id;
	uint16 sender_instance_id;
	uint32 character_id;
};

struct ServerDzLockout_Struct {
	uint32 dz_id;
	uint64 expire_time;
	uint32 duration;
	uint32 sender_zone_id;
	uint16 sender_instance_id;
	uint8  remove;
	uint8  members_only;
	int    seconds;
	char   event_name[256];
};

struct ServerDzLock_Struct {
	uint32 dz_id;
	uint32 sender_zone_id;
	uint16 sender_instance_id;
	bool   lock;
	uint8  lock_msg; // 0: none, 1: closing 2: trial begin
	uint32 color;
};

struct ServerDzBool_Struct {
	uint32 dz_id;
	uint32 sender_zone_id;
	uint16 sender_instance_id;
	bool   enabled;
};

struct ServerDzCharacterLockout_Struct {
	uint8  remove;
	uint32 char_id;
	uint64 expire_time;
	uint32 duration;
	char   uuid[37];
	char   expedition[128];
	char   event[256];
};

struct ServerDzExpireWarning_Struct {
	uint32_t dz_id;
	uint32_t minutes_remaining;
};

struct ServerDzCommand_Struct {
	uint32 dz_id;
	uint8  is_char_online;     // 0: target name is offline, 1: online
	char   requester_name[64];
	char   target_name[64];
	char   remove_name[64];    // used for swap command
};

struct ServerDzCommandMakeLeader_Struct {
	uint32 dz_id;
	uint32 requester_id;
	uint8  is_online;  // set by world, 0: new leader name offline, 1: online
	uint8  is_success; // set by world, 0: makeleader failed, 1: success (is online member)
	char   new_leader_name[64];
};

struct ServerDzID_Struct {
	uint32 dz_id;
	uint16 dz_zone_id;
	uint16 dz_instance_id; // for cache-independent redundancy (messages to dz's instance)
	uint32 sender_zone_id;
	uint16 sender_instance_id;
};

struct ServerDzLocation_Struct {
	uint32 dz_id;
	uint32 sender_zone_id;
	uint16 sender_instance_id;
	uint32 zone_id;
	float  y;
	float  x;
	float  z;
	float  heading;
};

struct ServerDzSwitchID_Struct {
	uint32 dz_id;
	int    dz_switch_id;
};

struct ServerDzMember_Struct {
	uint32 dz_id;
	uint16 dz_zone_id;
	uint16 dz_instance_id; // for cache redundancy
	uint16 sender_zone_id;
	uint16 sender_instance_id;
	uint8  removed; // 0: added, 1: removed
	uint32 character_id;
	uint8  character_status; // 0: unknown 1: Online 2: Offline 3: In Dynamic Zone 4: Link Dead
	char   character_name[64];
};

struct ServerDzMemberSwap_Struct {
	uint32 dz_id;
	uint16 dz_zone_id;
	uint16 dz_instance_id; // for cache redundancy
	uint16 sender_zone_id;
	uint16 sender_instance_id;
	uint32 add_character_id;
	uint32 remove_character_id;
	uint8  add_character_status;
	char   add_character_name[64];
	char   remove_character_name[64];
};

struct ServerDzSetDuration_Struct {
	uint32 dz_id;
	uint32 seconds;
};

struct ServerDzCreate_Struct {
	uint16_t origin_zone_id;
	uint16_t origin_instance_id;
	uint32_t dz_id;
	uint32_t cereal_size;
	char     cereal_data[1];
};

struct ServerSendPlayerEvent_Struct {
	uint32_t cereal_size;
	char cereal_data[0];
};

struct ServerDataBucketCacheUpdate_Struct {
	uint32_t cereal_size;
	char cereal_data[0];
};

struct ServerFlagUpdate_Struct {
	uint32 account_id;
	int16 admin;
};

struct ServerOOCMute_Struct {
	bool is_muted;
};

struct ServerZoneStatus_Struct {
	char  name[64];
	int16 admin;
};

struct GuildTributeUpdate {
	uint32 guild_id;
	uint32 tribute_id_1;
	uint32 tribute_id_2;
	uint32 tribute_id_1_tier;
	uint32 tribute_id_2_tier;
	uint32 enabled;
	uint32 favor;
	uint32 time_remaining;
	uint32 member_favor;
	uint32 member_time;
	uint32 member_enabled;
	char   player_name[64];
};

struct GuildTributeMemberToggle {
	uint32 guild_id;
	uint32 char_id;
	char   player_name[64];
	uint32 tribute_toggle;
	uint32 command;
	uint32 time_remaining;
	uint32 no_donations;
	uint32 member_last_donated;
};

struct ServerOP_GuildMessage_Struct {
	uint32 guild_id{GUILD_NONE};
	char   player_name[64]{0};
	uint32 player_level{0};
	uint32 player_class{0};
	uint32 player_rank{GUILD_RANK_NONE};
	uint32 player_zone_id{0};
	bool   player_offline{false};
	char   player_new_name[64]{0};
	char   new_guild_name[64]{0};
	char   note[256]{0};
    char   channel[2048]{0};
    char   url[2048]{0};
};

struct TraderMessaging_Struct {
	uint32 action;
	uint32 zone_id;
	uint32 instance_id;
	uint32 trader_id;
	uint32 entity_id;
	char   trader_name[64];
};

struct BazaarPurchaseMessaging_Struct {
	TraderBuy_Struct trader_buy_struct;
	uint32           item_aug_1;
	uint32           item_aug_2;
	uint32           item_aug_3;
	uint32           item_aug_4;
	uint32           item_aug_5;
	uint32           item_aug_6;
	uint32           buyer_id;
	uint32           item_quantity_available;
	uint32           id;
};

#pragma pack(pop)
