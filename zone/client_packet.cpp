/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY except by those people which sell it, which
are required to give you total support for your newly bought product;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "../common/global_define.h"
#include "../common/eqemu_logsys.h"
#include "../common/opcodemgr.h"
#include "../common/raid.h"

#include <iomanip>
#include <iostream>
#include <math.h>
#include <set>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include "bot.h"

#ifdef _WINDOWS
#define snprintf	_snprintf
#define strncasecmp	_strnicmp
#define strcasecmp	_stricmp
#else
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

#include "../common/data_verification.h"
#include "../common/rdtsc.h"
#include "../common/data_bucket.h"
#include "dynamic_zone.h"
#include "event_codes.h"
#include "guild_mgr.h"
#include "merc.h"
#include "petitions.h"
#include "queryserv.h"
#include "quest_parser_collection.h"
#include "string_ids.h"
#include "titles.h"
#include "water_map.h"
#include "worldserver.h"
#include "zone.h"
#include "mob_movement_manager.h"
#include "../common/repositories/character_instance_safereturns_repository.h"
#include "../common/repositories/criteria/content_filter_criteria.h"
#include "../common/shared_tasks.h"
#include "gm_commands/door_manipulation.h"
#include "gm_commands/object_manipulation.h"
#include "client.h"
#include "../common/repositories/account_repository.h"
#include "../common/repositories/character_corpses_repository.h"
#include "../common/repositories/guild_tributes_repository.h"
#include "../common/repositories/buyer_buy_lines_repository.h"
#include "../common/repositories/character_pet_name_repository.h"
#include "../common/repositories/tradeskill_recipe_entries_repository.h"

#include "../common/events/player_event_logs.h"
#include "../common/repositories/character_stats_record_repository.h"
#include "dialogue_window.h"
#include "../common/rulesys.h"
#include "../common/repositories/adventure_members_repository.h"

extern QueryServ* QServ;
extern Zone* zone;
extern volatile bool is_zone_loaded;
extern WorldServer worldserver;
extern EntityList entity_list;
typedef void (Client::*ClientPacketProc)(const EQApplicationPacket *app);

//Use a map for connecting opcodes since it dosent get used a lot and is sparse
std::map<uint32, ClientPacketProc> ConnectingOpcodes;
//Use a static array for connected, for speed
ClientPacketProc ConnectedOpcodes[_maxEmuOpcode];

void MapOpcodes()
{
	ConnectingOpcodes.clear();
	memset(ConnectedOpcodes, 0, sizeof(ConnectedOpcodes));

	// Now put all the opcodes into their home...
	// connecting opcode handler assignments:
	ConnectingOpcodes[OP_ApproveZone] = &Client::Handle_Connect_OP_ApproveZone;
	ConnectingOpcodes[OP_BlockedBuffs] = &Client::Handle_OP_BlockedBuffs;
	ConnectingOpcodes[OP_ClientError] = &Client::Handle_Connect_OP_ClientError;
	ConnectingOpcodes[OP_ClientReady] = &Client::Handle_Connect_OP_ClientReady;
	ConnectingOpcodes[OP_ClientUpdate] = &Client::Handle_Connect_OP_ClientUpdate;
	ConnectingOpcodes[OP_GetGuildsList] = &Client::Handle_OP_GetGuildsList; // temporary hack
	ConnectingOpcodes[OP_ReqClientSpawn] = &Client::Handle_Connect_OP_ReqClientSpawn;
	ConnectingOpcodes[OP_ReqNewZone] = &Client::Handle_Connect_OP_ReqNewZone;
	ConnectingOpcodes[OP_SendAAStats] = &Client::Handle_Connect_OP_SendAAStats;
	ConnectingOpcodes[OP_SendAATable] = &Client::Handle_Connect_OP_SendAATable;
	ConnectingOpcodes[OP_SendExpZonein] = &Client::Handle_Connect_OP_SendExpZonein;
	ConnectingOpcodes[OP_RequestGuildTributes] = &Client::Handle_Connect_OP_SendGuildTributes;
	ConnectingOpcodes[OP_SendTributes] = &Client::Handle_Connect_OP_SendTributes;
	ConnectingOpcodes[OP_SetServerFilter] = &Client::Handle_Connect_OP_SetServerFilter;
	ConnectingOpcodes[OP_SpawnAppearance] = &Client::Handle_Connect_OP_SpawnAppearance;
	ConnectingOpcodes[OP_TGB] = &Client::Handle_Connect_OP_TGB;
	ConnectingOpcodes[OP_UpdateAA] = &Client::Handle_Connect_OP_UpdateAA;
	ConnectingOpcodes[OP_WearChange] = &Client::Handle_Connect_OP_WearChange;
	ConnectingOpcodes[OP_WorldObjectsSent] = &Client::Handle_Connect_OP_WorldObjectsSent;
	ConnectingOpcodes[OP_XTargetAutoAddHaters] = &Client::Handle_OP_XTargetAutoAddHaters;
	ConnectingOpcodes[OP_XTargetRequest] = &Client::Handle_OP_XTargetRequest;
	ConnectingOpcodes[OP_ZoneComplete] = &Client::Handle_Connect_OP_ZoneComplete;
	ConnectingOpcodes[OP_ZoneEntry] = &Client::Handle_Connect_OP_ZoneEntry;

	// connected opcode handler assignments:
	ConnectedOpcodes[OP_0x0193] = &Client::Handle_0x0193;
	ConnectedOpcodes[OP_AAAction] = &Client::Handle_OP_AAAction;
	ConnectedOpcodes[OP_AcceptNewTask] = &Client::Handle_OP_AcceptNewTask;
	ConnectedOpcodes[OP_AdventureInfoRequest] = &Client::Handle_OP_AdventureInfoRequest;
	ConnectedOpcodes[OP_AdventureLeaderboardRequest] = &Client::Handle_OP_AdventureLeaderboardRequest;
	ConnectedOpcodes[OP_AdventureMerchantPurchase] = &Client::Handle_OP_AdventureMerchantPurchase;
	ConnectedOpcodes[OP_AdventureMerchantRequest] = &Client::Handle_OP_AdventureMerchantRequest;
	ConnectedOpcodes[OP_AdventureMerchantSell] = &Client::Handle_OP_AdventureMerchantSell;
	ConnectedOpcodes[OP_AdventureRequest] = &Client::Handle_OP_AdventureRequest;
	ConnectedOpcodes[OP_AdventureStatsRequest] = &Client::Handle_OP_AdventureStatsRequest;
	ConnectedOpcodes[OP_AggroMeterLockTarget] = &Client::Handle_OP_AggroMeterLockTarget;
	ConnectedOpcodes[OP_AltCurrencyMerchantRequest] = &Client::Handle_OP_AltCurrencyMerchantRequest;
	ConnectedOpcodes[OP_AltCurrencyPurchase] = &Client::Handle_OP_AltCurrencyPurchase;
	ConnectedOpcodes[OP_AltCurrencyReclaim] = &Client::Handle_OP_AltCurrencyReclaim;
	ConnectedOpcodes[OP_AltCurrencySell] = &Client::Handle_OP_AltCurrencySell;
	ConnectedOpcodes[OP_AltCurrencySellSelection] = &Client::Handle_OP_AltCurrencySellSelection;
	ConnectedOpcodes[OP_Animation] = &Client::Handle_OP_Animation;
	ConnectedOpcodes[OP_ApplyPoison] = &Client::Handle_OP_ApplyPoison;
	ConnectedOpcodes[OP_Assist] = &Client::Handle_OP_Assist;
	ConnectedOpcodes[OP_AssistGroup] = &Client::Handle_OP_AssistGroup;
	ConnectedOpcodes[OP_AugmentInfo] = &Client::Handle_OP_AugmentInfo;
	ConnectedOpcodes[OP_AugmentItem] = &Client::Handle_OP_AugmentItem;
	ConnectedOpcodes[OP_AutoAttack] = &Client::Handle_OP_AutoAttack;
	ConnectedOpcodes[OP_AutoAttack2] = &Client::Handle_OP_AutoAttack2;
	ConnectedOpcodes[OP_AutoFire] = &Client::Handle_OP_AutoFire;
	ConnectedOpcodes[OP_Bandolier] = &Client::Handle_OP_Bandolier;
	ConnectedOpcodes[OP_BankerChange] = &Client::Handle_OP_BankerChange;
	ConnectedOpcodes[OP_Barter] = &Client::Handle_OP_Barter;
	ConnectedOpcodes[OP_BazaarInspect] = &Client::Handle_OP_BazaarInspect;
	ConnectedOpcodes[OP_BazaarSearch] = &Client::Handle_OP_BazaarSearch;
	ConnectedOpcodes[OP_Begging] = &Client::Handle_OP_Begging;
	ConnectedOpcodes[OP_Bind_Wound] = &Client::Handle_OP_Bind_Wound;
	ConnectedOpcodes[OP_BlockedBuffs] = &Client::Handle_OP_BlockedBuffs;
	ConnectedOpcodes[OP_BoardBoat] = &Client::Handle_OP_BoardBoat;
	ConnectedOpcodes[OP_BookButton] = &Client::Handle_OP_BookButton;
	ConnectedOpcodes[OP_Buff] = &Client::Handle_OP_Buff;
	ConnectedOpcodes[OP_BuffRemoveRequest] = &Client::Handle_OP_BuffRemoveRequest;
	ConnectedOpcodes[OP_Bug] = &Client::Handle_OP_Bug;
	ConnectedOpcodes[OP_Camp] = &Client::Handle_OP_Camp;
	ConnectedOpcodes[OP_CancelTask] = &Client::Handle_OP_CancelTask;
	ConnectedOpcodes[OP_CancelTrade] = &Client::Handle_OP_CancelTrade;
	ConnectedOpcodes[OP_CastSpell] = &Client::Handle_OP_CastSpell;
	ConnectedOpcodes[OP_ChannelMessage] = &Client::Handle_OP_ChannelMessage;
	ConnectedOpcodes[OP_ChangePetName] = &Client::Handle_OP_ChangePetName;
	ConnectedOpcodes[OP_ClearBlockedBuffs] = &Client::Handle_OP_ClearBlockedBuffs;
	ConnectedOpcodes[OP_ClearNPCMarks] = &Client::Handle_OP_ClearNPCMarks;
	ConnectedOpcodes[OP_ClearSurname] = &Client::Handle_OP_ClearSurname;
	ConnectedOpcodes[OP_ClickDoor] = &Client::Handle_OP_ClickDoor;
	ConnectedOpcodes[OP_ClickObject] = &Client::Handle_OP_ClickObject;
	ConnectedOpcodes[OP_ClickObjectAction] = &Client::Handle_OP_ClickObjectAction;
	ConnectedOpcodes[OP_ClientError] = &Client::Handle_OP_ClientError;
	ConnectedOpcodes[OP_ClientTimeStamp] = &Client::Handle_OP_ClientTimeStamp;
	ConnectedOpcodes[OP_ClientUpdate] = &Client::Handle_OP_ClientUpdate;
	ConnectedOpcodes[OP_CombatAbility] = &Client::Handle_OP_CombatAbility;
	ConnectedOpcodes[OP_ConfirmDelete] = &Client::Handle_OP_ConfirmDelete;
	ConnectedOpcodes[OP_Consent] = &Client::Handle_OP_Consent;
	ConnectedOpcodes[OP_ConsentDeny] = &Client::Handle_OP_ConsentDeny;
	ConnectedOpcodes[OP_Consider] = &Client::Handle_OP_Consider;
	ConnectedOpcodes[OP_ConsiderCorpse] = &Client::Handle_OP_ConsiderCorpse;
	ConnectedOpcodes[OP_Consume] = &Client::Handle_OP_Consume;
	ConnectedOpcodes[OP_ControlBoat] = &Client::Handle_OP_ControlBoat;
	ConnectedOpcodes[OP_CorpseDrag] = &Client::Handle_OP_CorpseDrag;
	ConnectedOpcodes[OP_CorpseDrop] = &Client::Handle_OP_CorpseDrop;
	ConnectedOpcodes[OP_CrashDump] = &Client::Handle_OP_CrashDump;
	ConnectedOpcodes[OP_CrystalCreate] = &Client::Handle_OP_CrystalCreate;
	ConnectedOpcodes[OP_CrystalReclaim] = &Client::Handle_OP_CrystalReclaim;
	ConnectedOpcodes[OP_Damage] = &Client::Handle_OP_Damage;
	ConnectedOpcodes[OP_Death] = &Client::Handle_OP_Death;
	ConnectedOpcodes[OP_DelegateAbility] = &Client::Handle_OP_DelegateAbility;
	ConnectedOpcodes[OP_DeleteItem] = &Client::Handle_OP_DeleteItem;
	ConnectedOpcodes[OP_DeleteSpawn] = &Client::Handle_OP_DeleteSpawn;
	ConnectedOpcodes[OP_DeleteSpell] = &Client::Handle_OP_DeleteSpell;
	ConnectedOpcodes[OP_Disarm] = &Client::Handle_OP_Disarm;
	ConnectedOpcodes[OP_DisarmTraps] = &Client::Handle_OP_DisarmTraps;
	ConnectedOpcodes[OP_DoGroupLeadershipAbility] = &Client::Handle_OP_DoGroupLeadershipAbility;
	ConnectedOpcodes[OP_DuelDecline] = &Client::Handle_OP_DuelDecline;
	ConnectedOpcodes[OP_DuelAccept] = &Client::Handle_OP_DuelAccept;
	ConnectedOpcodes[OP_DumpName] = &Client::Handle_OP_DumpName;
	ConnectedOpcodes[OP_Dye] = &Client::Handle_OP_Dye;
	ConnectedOpcodes[OP_DzAddPlayer] = &Client::Handle_OP_DzAddPlayer;
	ConnectedOpcodes[OP_DzChooseZoneReply] = &Client::Handle_OP_DzChooseZoneReply;
	ConnectedOpcodes[OP_DzExpeditionInviteResponse] = &Client::Handle_OP_DzExpeditionInviteResponse;
	ConnectedOpcodes[OP_DzListTimers] = &Client::Handle_OP_DzListTimers;
	ConnectedOpcodes[OP_DzMakeLeader] = &Client::Handle_OP_DzMakeLeader;
	ConnectedOpcodes[OP_DzPlayerList] = &Client::Handle_OP_DzPlayerList;
	ConnectedOpcodes[OP_DzRemovePlayer] = &Client::Handle_OP_DzRemovePlayer;
	ConnectedOpcodes[OP_DzSwapPlayer] = &Client::Handle_OP_DzSwapPlayer;
	ConnectedOpcodes[OP_DzQuit] = &Client::Handle_OP_DzQuit;
	ConnectedOpcodes[OP_Emote] = &Client::Handle_OP_Emote;
	ConnectedOpcodes[OP_EndLootRequest] = &Client::Handle_OP_EndLootRequest;
	ConnectedOpcodes[OP_EnvDamage] = &Client::Handle_OP_EnvDamage;
	ConnectedOpcodes[OP_EvolveItem] = &Client::Handle_OP_EvolveItem;
	ConnectedOpcodes[OP_FaceChange] = &Client::Handle_OP_FaceChange;
	ConnectedOpcodes[OP_FeignDeath] = &Client::Handle_OP_FeignDeath;
	ConnectedOpcodes[OP_FindPersonRequest] = &Client::Handle_OP_FindPersonRequest;
	ConnectedOpcodes[OP_Fishing] = &Client::Handle_OP_Fishing;
	ConnectedOpcodes[OP_FloatListThing] = &Client::Handle_OP_MovementHistoryList;
	ConnectedOpcodes[OP_Forage] = &Client::Handle_OP_Forage;
	ConnectedOpcodes[OP_FriendsWho] = &Client::Handle_OP_FriendsWho;
	ConnectedOpcodes[OP_GetGuildMOTD] = &Client::Handle_OP_GetGuildMOTD;
	ConnectedOpcodes[OP_GetGuildsList] = &Client::Handle_OP_GetGuildsList;
	ConnectedOpcodes[OP_GMBecomeNPC] = &Client::Handle_OP_GMBecomeNPC;
	ConnectedOpcodes[OP_GMDelCorpse] = &Client::Handle_OP_GMDelCorpse;
	ConnectedOpcodes[OP_GMEmoteZone] = &Client::Handle_OP_GMEmoteZone;
	ConnectedOpcodes[OP_GMEndTraining] = &Client::Handle_OP_GMEndTraining;
	ConnectedOpcodes[OP_GMFind] = &Client::Handle_OP_GMFind;
	ConnectedOpcodes[OP_GMGoto] = &Client::Handle_OP_GMGoto;
	ConnectedOpcodes[OP_GMHideMe] = &Client::Handle_OP_GMHideMe;
	ConnectedOpcodes[OP_GMKick] = &Client::Handle_OP_GMKick;
	ConnectedOpcodes[OP_GMKill] = &Client::Handle_OP_GMKill;
	ConnectedOpcodes[OP_GMLastName] = &Client::Handle_OP_GMLastName;
	ConnectedOpcodes[OP_GMNameChange] = &Client::Handle_OP_GMNameChange;
	ConnectedOpcodes[OP_GMSearchCorpse] = &Client::Handle_OP_GMSearchCorpse;
	ConnectedOpcodes[OP_GMServers] = &Client::Handle_OP_GMServers;
	ConnectedOpcodes[OP_GMSummon] = &Client::Handle_OP_GMSummon;
	ConnectedOpcodes[OP_GMToggle] = &Client::Handle_OP_GMToggle;
	ConnectedOpcodes[OP_GMTraining] = &Client::Handle_OP_GMTraining;
	ConnectedOpcodes[OP_GMTrainSkill] = &Client::Handle_OP_GMTrainSkill;
	ConnectedOpcodes[OP_GMZoneRequest] = &Client::Handle_OP_GMZoneRequest;
	ConnectedOpcodes[OP_GMZoneRequest2] = &Client::Handle_OP_GMZoneRequest2;
	ConnectedOpcodes[OP_GroundSpawn] = &Client::Handle_OP_CreateObject;
	ConnectedOpcodes[OP_GroupAcknowledge] = &Client::Handle_OP_GroupAcknowledge;
	ConnectedOpcodes[OP_GroupCancelInvite] = &Client::Handle_OP_GroupCancelInvite;
	ConnectedOpcodes[OP_GroupDelete] = &Client::Handle_OP_GroupDelete;
	ConnectedOpcodes[OP_GroupDisband] = &Client::Handle_OP_GroupDisband;
	ConnectedOpcodes[OP_GroupFollow] = &Client::Handle_OP_GroupFollow;
	ConnectedOpcodes[OP_GroupFollow2] = &Client::Handle_OP_GroupFollow2;
	ConnectedOpcodes[OP_GroupInvite] = &Client::Handle_OP_GroupInvite;
	ConnectedOpcodes[OP_GroupInvite2] = &Client::Handle_OP_GroupInvite2;
	ConnectedOpcodes[OP_GroupMakeLeader] = &Client::Handle_OP_GroupMakeLeader;
	ConnectedOpcodes[OP_GroupMentor] = &Client::Handle_OP_GroupMentor;
	ConnectedOpcodes[OP_GroupRoles] = &Client::Handle_OP_GroupRoles;
	ConnectedOpcodes[OP_GroupUpdate] = &Client::Handle_OP_GroupUpdate;
	ConnectedOpcodes[OP_GuildBank] = &Client::Handle_OP_GuildBank;
	ConnectedOpcodes[OP_GuildCreate] = &Client::Handle_OP_GuildCreate;
	ConnectedOpcodes[OP_GuildDelete] = &Client::Handle_OP_GuildDelete;
	ConnectedOpcodes[OP_GuildDemote] = &Client::Handle_OP_GuildDemote;
	ConnectedOpcodes[OP_GuildInvite] = &Client::Handle_OP_GuildInvite;
	ConnectedOpcodes[OP_GuildInviteAccept] = &Client::Handle_OP_GuildInviteAccept;
	ConnectedOpcodes[OP_GuildLeader] = &Client::Handle_OP_GuildLeader;
	ConnectedOpcodes[OP_GuildManageBanker] = &Client::Handle_OP_GuildManageBanker;
	ConnectedOpcodes[OP_GuildPeace] = &Client::Handle_OP_GuildPeace;
	ConnectedOpcodes[OP_GuildPromote] = &Client::Handle_OP_GuildPromote;
	ConnectedOpcodes[OP_GuildPublicNote] = &Client::Handle_OP_GuildPublicNote;
	ConnectedOpcodes[OP_GuildRemove] = &Client::Handle_OP_GuildRemove;
	ConnectedOpcodes[OP_GuildStatus] = &Client::Handle_OP_GuildStatus;
	ConnectedOpcodes[OP_GuildUpdate] = &Client::Handle_OP_GuildUpdate;
	ConnectedOpcodes[OP_GuildWar] = &Client::Handle_OP_GuildWar;
	ConnectedOpcodes[OP_GuildSelectTribute] = &Client::Handle_OP_GuildTributeSelect;
	ConnectedOpcodes[OP_GuildModifyBenefits] = &Client::Handle_OP_GuildTributeModifyBenefits;
	ConnectedOpcodes[OP_GuildOptInOut] = &Client::Handle_OP_GuildTributeOptInOut;
	ConnectedOpcodes[OP_GuildSaveActiveTributes] = &Client::Handle_OP_GuildTributeSaveActiveTributes;
	ConnectedOpcodes[OP_GuildTributeToggleReq] = &Client::Handle_OP_GuildTributeToggle;
	ConnectedOpcodes[OP_GuildTributeDonateItem] = &Client::Handle_OP_GuildTributeDonateItem;
	ConnectedOpcodes[OP_GuildTributeDonatePlat] = &Client::Handle_OP_GuildTributeDonatePlat;
	ConnectedOpcodes[OP_Heartbeat] = &Client::Handle_OP_Heartbeat;
	ConnectedOpcodes[OP_Hide] = &Client::Handle_OP_Hide;
	ConnectedOpcodes[OP_HideCorpse] = &Client::Handle_OP_HideCorpse;
	ConnectedOpcodes[OP_Illusion] = &Client::Handle_OP_Illusion;
	ConnectedOpcodes[OP_InspectAnswer] = &Client::Handle_OP_InspectAnswer;
	ConnectedOpcodes[OP_InspectMessageUpdate] = &Client::Handle_OP_InspectMessageUpdate;
	ConnectedOpcodes[OP_InspectRequest] = &Client::Handle_OP_InspectRequest;
	ConnectedOpcodes[OP_InstillDoubt] = &Client::Handle_OP_InstillDoubt;
	ConnectedOpcodes[OP_ItemLinkClick] = &Client::Handle_OP_ItemLinkClick;
	ConnectedOpcodes[OP_ItemLinkResponse] = &Client::Handle_OP_ItemLinkResponse;
	ConnectedOpcodes[OP_ItemName] = &Client::Handle_OP_ItemName;
	ConnectedOpcodes[OP_ItemPreview] = &Client::Handle_OP_ItemPreview;
	ConnectedOpcodes[OP_ItemPreviewRequest] = &Client::Handle_OP_ItemPreviewRequest;
	ConnectedOpcodes[OP_ItemVerifyRequest] = &Client::Handle_OP_ItemVerifyRequest;
	ConnectedOpcodes[OP_ItemViewUnknown] = &Client::Handle_OP_Ignore;
	ConnectedOpcodes[OP_Jump] = &Client::Handle_OP_Jump;
	ConnectedOpcodes[OP_KeyRing] = &Client::Handle_OP_KeyRing;
	ConnectedOpcodes[OP_KickPlayers] = &Client::Handle_OP_KickPlayers;
	ConnectedOpcodes[OP_LDoNButton] = &Client::Handle_OP_LDoNButton;
	ConnectedOpcodes[OP_LDoNDisarmTraps] = &Client::Handle_OP_LDoNDisarmTraps;
	ConnectedOpcodes[OP_LDoNInspect] = &Client::Handle_OP_LDoNInspect;
	ConnectedOpcodes[OP_LDoNOpen] = &Client::Handle_OP_LDoNOpen;
	ConnectedOpcodes[OP_LDoNPickLock] = &Client::Handle_OP_LDoNPickLock;
	ConnectedOpcodes[OP_LDoNSenseTraps] = &Client::Handle_OP_LDoNSenseTraps;
	ConnectedOpcodes[OP_LeadershipExpToggle] = &Client::Handle_OP_LeadershipExpToggle;
	ConnectedOpcodes[OP_LeaveAdventure] = &Client::Handle_OP_LeaveAdventure;
	ConnectedOpcodes[OP_LeaveBoat] = &Client::Handle_OP_LeaveBoat;
	ConnectedOpcodes[OP_LFGCommand] = &Client::Handle_OP_LFGCommand;
	ConnectedOpcodes[OP_LFGGetMatchesRequest] = &Client::Handle_OP_LFGGetMatchesRequest;
	ConnectedOpcodes[OP_LFGuild] = &Client::Handle_OP_LFGuild;
	ConnectedOpcodes[OP_LFPCommand] = &Client::Handle_OP_LFPCommand;
	ConnectedOpcodes[OP_LFPGetMatchesRequest] = &Client::Handle_OP_LFPGetMatchesRequest;
	ConnectedOpcodes[OP_LoadSpellSet] = &Client::Handle_OP_LoadSpellSet;
	ConnectedOpcodes[OP_Logout] = &Client::Handle_OP_Logout;
	ConnectedOpcodes[OP_LootItem] = &Client::Handle_OP_LootItem;
	ConnectedOpcodes[OP_LootRequest] = &Client::Handle_OP_LootRequest;
	ConnectedOpcodes[OP_ManaChange] = &Client::Handle_OP_ManaChange;
	ConnectedOpcodes[OP_MemorizeSpell] = &Client::Handle_OP_MemorizeSpell;
	ConnectedOpcodes[OP_Mend] = &Client::Handle_OP_Mend;
	ConnectedOpcodes[OP_MercenaryCommand] = &Client::Handle_OP_MercenaryCommand;
	ConnectedOpcodes[OP_MercenaryDataRequest] = &Client::Handle_OP_MercenaryDataRequest;
	ConnectedOpcodes[OP_MercenaryDataUpdateRequest] = &Client::Handle_OP_MercenaryDataUpdateRequest;
	ConnectedOpcodes[OP_MercenaryDismiss] = &Client::Handle_OP_MercenaryDismiss;
	ConnectedOpcodes[OP_MercenaryHire] = &Client::Handle_OP_MercenaryHire;
	ConnectedOpcodes[OP_MercenarySuspendRequest] = &Client::Handle_OP_MercenarySuspendRequest;
	ConnectedOpcodes[OP_MercenaryTimerRequest] = &Client::Handle_OP_MercenaryTimerRequest;
	ConnectedOpcodes[OP_MoveCoin] = &Client::Handle_OP_MoveCoin;
	ConnectedOpcodes[OP_MoveItem] = &Client::Handle_OP_MoveItem;
	ConnectedOpcodes[OP_MoveMultipleItems] = &Client::Handle_OP_MoveMultipleItems;
	ConnectedOpcodes[OP_OpenContainer] = &Client::Handle_OP_OpenContainer;
	ConnectedOpcodes[OP_OpenGuildTributeMaster] = &Client::Handle_OP_OpenGuildTributeMaster;
	ConnectedOpcodes[OP_OpenInventory] = &Client::Handle_OP_OpenInventory;
	ConnectedOpcodes[OP_OpenTributeMaster] = &Client::Handle_OP_OpenTributeMaster;
	ConnectedOpcodes[OP_ShopSendParcel] = &Client::Handle_OP_ShopSendParcel;
	ConnectedOpcodes[OP_ShopRetrieveParcel] = &Client::Handle_OP_ShopRetrieveParcel;
	ConnectedOpcodes[OP_PDeletePetition] = &Client::Handle_OP_PDeletePetition;
	ConnectedOpcodes[OP_PetCommands] = &Client::Handle_OP_PetCommands;
	ConnectedOpcodes[OP_Petition] = &Client::Handle_OP_Petition;
	ConnectedOpcodes[OP_PetitionBug] = &Client::Handle_OP_PetitionBug;
	ConnectedOpcodes[OP_PetitionCheckIn] = &Client::Handle_OP_PetitionCheckIn;
	ConnectedOpcodes[OP_PetitionCheckout] = &Client::Handle_OP_PetitionCheckout;
	ConnectedOpcodes[OP_PetitionDelete] = &Client::Handle_OP_PetitionDelete;
	ConnectedOpcodes[OP_PetitionQue] = &Client::Handle_OP_PetitionQue;
	ConnectedOpcodes[OP_PetitionRefresh] = &Client::Handle_OP_PetitionRefresh;
	ConnectedOpcodes[OP_PetitionResolve] = &Client::Handle_OP_PetitionResolve;
	ConnectedOpcodes[OP_PetitionUnCheckout] = &Client::Handle_OP_PetitionUnCheckout;
	ConnectedOpcodes[OP_PlayerStateAdd] = &Client::Handle_OP_PlayerStateAdd;
	ConnectedOpcodes[OP_PlayerStateRemove] = &Client::Handle_OP_PlayerStateRemove;
	ConnectedOpcodes[OP_PickPocket] = &Client::Handle_OP_PickPocket;
	ConnectedOpcodes[OP_PickZone] = &Client::Handle_OP_PickZone;
	ConnectedOpcodes[OP_PopupResponse] = &Client::Handle_OP_PopupResponse;
	ConnectedOpcodes[OP_PotionBelt] = &Client::Handle_OP_PotionBelt;
	ConnectedOpcodes[OP_PurchaseLeadershipAA] = &Client::Handle_OP_PurchaseLeadershipAA;
	ConnectedOpcodes[OP_PVPLeaderBoardDetailsRequest] = &Client::Handle_OP_PVPLeaderBoardDetailsRequest;
	ConnectedOpcodes[OP_PVPLeaderBoardRequest] = &Client::Handle_OP_PVPLeaderBoardRequest;
	ConnectedOpcodes[OP_QueryUCSServerStatus] = &Client::Handle_OP_QueryUCSServerStatus;
	ConnectedOpcodes[OP_RaidInvite] = &Client::Handle_OP_RaidCommand;
	ConnectedOpcodes[OP_RaidDelegateAbility] = &Client::Handle_OP_RaidDelegateAbility;
	ConnectedOpcodes[OP_RaidClearNPCMarks] = &Client::Handle_OP_RaidClearNPCMarks;
	ConnectedOpcodes[OP_RandomReq] = &Client::Handle_OP_RandomReq;
	ConnectedOpcodes[OP_ReadBook] = &Client::Handle_OP_ReadBook;
	ConnectedOpcodes[OP_RecipeAutoCombine] = &Client::Handle_OP_RecipeAutoCombine;
	ConnectedOpcodes[OP_RecipeDetails] = &Client::Handle_OP_RecipeDetails;
	ConnectedOpcodes[OP_RecipesFavorite] = &Client::Handle_OP_RecipesFavorite;
	ConnectedOpcodes[OP_RecipesSearch] = &Client::Handle_OP_RecipesSearch;
	ConnectedOpcodes[OP_ReloadUI] = &Client::Handle_OP_ReloadUI;
	ConnectedOpcodes[OP_RemoveBlockedBuffs] = &Client::Handle_OP_RemoveBlockedBuffs;
	ConnectedOpcodes[OP_RemoveTrap] = &Client::Handle_OP_RemoveTrap;
	ConnectedOpcodes[OP_Report] = &Client::Handle_OP_Report;
	ConnectedOpcodes[OP_RequestDuel] = &Client::Handle_OP_RequestDuel;
	ConnectedOpcodes[OP_RequestTitles] = &Client::Handle_OP_RequestTitles;
	ConnectedOpcodes[OP_RespawnWindow] = &Client::Handle_OP_RespawnWindow;
	ConnectedOpcodes[OP_Rewind] = &Client::Handle_OP_Rewind;
	ConnectedOpcodes[OP_RezzAnswer] = &Client::Handle_OP_RezzAnswer;
	ConnectedOpcodes[OP_Sacrifice] = &Client::Handle_OP_Sacrifice;
	ConnectedOpcodes[OP_SafeFallSuccess] = &Client::Handle_OP_SafeFallSuccess;
	ConnectedOpcodes[OP_SafePoint] = &Client::Handle_OP_SafePoint;
	ConnectedOpcodes[OP_Save] = &Client::Handle_OP_Save;
	ConnectedOpcodes[OP_SaveOnZoneReq] = &Client::Handle_OP_SaveOnZoneReq;
	ConnectedOpcodes[OP_SelectTribute] = &Client::Handle_OP_SelectTribute;
	ConnectedOpcodes[OP_SenseHeading] = &Client::Handle_OP_SenseHeading;
	ConnectedOpcodes[OP_SenseTraps] = &Client::Handle_OP_SenseTraps;
	ConnectedOpcodes[OP_SetGuildMOTD] = &Client::Handle_OP_SetGuildMOTD;
	ConnectedOpcodes[OP_SetRunMode] = &Client::Handle_OP_SetRunMode;
	ConnectedOpcodes[OP_SetServerFilter] = &Client::Handle_OP_SetServerFilter;
	ConnectedOpcodes[OP_SetStartCity] = &Client::Handle_OP_SetStartCity;
	ConnectedOpcodes[OP_SetTitle] = &Client::Handle_OP_SetTitle;
	ConnectedOpcodes[OP_Shielding] = &Client::Handle_OP_Shielding;
	ConnectedOpcodes[OP_ShopEnd] = &Client::Handle_OP_ShopEnd;
	ConnectedOpcodes[OP_ShopPlayerBuy] = &Client::Handle_OP_ShopPlayerBuy;
	ConnectedOpcodes[OP_ShopPlayerSell] = &Client::Handle_OP_ShopPlayerSell;
	ConnectedOpcodes[OP_ShopRequest] = &Client::Handle_OP_ShopRequest;
	ConnectedOpcodes[OP_Sneak] = &Client::Handle_OP_Sneak;
	ConnectedOpcodes[OP_SpawnAppearance] = &Client::Handle_OP_SpawnAppearance;
	ConnectedOpcodes[OP_Split] = &Client::Handle_OP_Split;
	ConnectedOpcodes[OP_Surname] = &Client::Handle_OP_Surname;
	ConnectedOpcodes[OP_SwapSpell] = &Client::Handle_OP_SwapSpell;
	ConnectedOpcodes[OP_TargetCommand] = &Client::Handle_OP_TargetCommand;
	ConnectedOpcodes[OP_TargetMouse] = &Client::Handle_OP_TargetMouse;
	ConnectedOpcodes[OP_TaskHistoryRequest] = &Client::Handle_OP_TaskHistoryRequest;
	ConnectedOpcodes[OP_TaskTimers] = &Client::Handle_OP_TaskTimers;
	ConnectedOpcodes[OP_Taunt] = &Client::Handle_OP_Taunt;
	ConnectedOpcodes[OP_TestBuff] = &Client::Handle_OP_TestBuff;
	ConnectedOpcodes[OP_TGB] = &Client::Handle_OP_TGB;
	ConnectedOpcodes[OP_Track] = &Client::Handle_OP_Track;
	ConnectedOpcodes[OP_TrackTarget] = &Client::Handle_OP_TrackTarget;
	ConnectedOpcodes[OP_TrackUnknown] = &Client::Handle_OP_TrackUnknown;
	ConnectedOpcodes[OP_TradeAcceptClick] = &Client::Handle_OP_TradeAcceptClick;
	ConnectedOpcodes[OP_TradeBusy] = &Client::Handle_OP_TradeBusy;
	ConnectedOpcodes[OP_Trader] = &Client::Handle_OP_Trader;
	ConnectedOpcodes[OP_TraderBuy] = &Client::Handle_OP_TraderBuy;
	ConnectedOpcodes[OP_TradeRequest] = &Client::Handle_OP_TradeRequest;
	ConnectedOpcodes[OP_TradeRequestAck] = &Client::Handle_OP_TradeRequestAck;
	ConnectedOpcodes[OP_TraderShop] = &Client::Handle_OP_TraderShop;
	ConnectedOpcodes[OP_TradeSkillCombine] = &Client::Handle_OP_TradeSkillCombine;
	ConnectedOpcodes[OP_TradeSkillRecipeInspect] = &Client::Handle_OP_TradeSkillRecipeInspect;
	ConnectedOpcodes[OP_Translocate] = &Client::Handle_OP_Translocate;
	ConnectedOpcodes[OP_TributeItem] = &Client::Handle_OP_TributeItem;
	ConnectedOpcodes[OP_TributeMoney] = &Client::Handle_OP_TributeMoney;
	ConnectedOpcodes[OP_TributeNPC] = &Client::Handle_OP_TributeNPC;
	ConnectedOpcodes[OP_TributeToggle] = &Client::Handle_OP_TributeToggle;
	ConnectedOpcodes[OP_TributeUpdate] = &Client::Handle_OP_TributeUpdate;
	ConnectedOpcodes[OP_VetClaimRequest] = &Client::Handle_OP_VetClaimRequest;
	ConnectedOpcodes[OP_VoiceMacroIn] = &Client::Handle_OP_VoiceMacroIn;
	ConnectedOpcodes[OP_UpdateAura] = &Client::Handle_OP_UpdateAura;
	ConnectedOpcodes[OP_WearChange] = &Client::Handle_OP_WearChange;
	ConnectedOpcodes[OP_WhoAllRequest] = &Client::Handle_OP_WhoAllRequest;
	ConnectedOpcodes[OP_WorldUnknown001] = &Client::Handle_OP_Ignore;
	ConnectedOpcodes[OP_XTargetAutoAddHaters] = &Client::Handle_OP_XTargetAutoAddHaters;
	ConnectedOpcodes[OP_XTargetOpen] = &Client::Handle_OP_XTargetOpen;
	ConnectedOpcodes[OP_XTargetRequest] = &Client::Handle_OP_XTargetRequest;
	ConnectedOpcodes[OP_YellForHelp] = &Client::Handle_OP_YellForHelp;
	ConnectedOpcodes[OP_ZoneChange] = &Client::Handle_OP_ZoneChange;
	ConnectedOpcodes[OP_ResetAA] = &Client::Handle_OP_ResetAA;
	ConnectedOpcodes[OP_UnderWorld] = &Client::Handle_OP_UnderWorld;

	// shared tasks
	ConnectedOpcodes[OP_SharedTaskRemovePlayer]   = &Client::Handle_OP_SharedTaskRemovePlayer;
	ConnectedOpcodes[OP_SharedTaskAddPlayer]      = &Client::Handle_OP_SharedTaskAddPlayer;
	ConnectedOpcodes[OP_SharedTaskMakeLeader]     = &Client::Handle_OP_SharedTaskMakeLeader;
	ConnectedOpcodes[OP_SharedTaskInviteResponse] = &Client::Handle_OP_SharedTaskInviteResponse;
	ConnectedOpcodes[OP_SharedTaskAcceptNew]      = &Client::Handle_OP_SharedTaskAccept;
	ConnectedOpcodes[OP_SharedTaskQuit]           = &Client::Handle_OP_SharedTaskQuit;
	ConnectedOpcodes[OP_SharedTaskPlayerList]     = &Client::Handle_OP_SharedTaskPlayerList;

	LogInfo("Mapped [{}] client opcode handlers", _maxEmuOpcode);
}

void ClearMappedOpcode(EmuOpcode op)
{
	if (op >= _maxEmuOpcode)
		return;

	ConnectedOpcodes[op] = nullptr;
	auto iter = ConnectingOpcodes.find(op);
	if (iter != ConnectingOpcodes.end()) {
		ConnectingOpcodes.erase(iter);
	}
}

// client methods
int Client::HandlePacket(const EQApplicationPacket *app)
{
	auto o = eqs->GetOpcodeManager();
	LogPacketClientServer(
		"[{}] [{:#06x}] Size [{}] {}",
		OpcodeManager::EmuToName(app->GetOpcode()),
		o->EmuToEQ(app->GetOpcode()) == 0 ? app->GetProtocolOpcode() : o->EmuToEQ(app->GetOpcode()),
		app->Size(),
		(EQEmuLogSys::Instance()->IsLogEnabled(Logs::Detail, Logs::PacketClientServer) ? DumpPacketToString(app) : "")
	);

	EmuOpcode opcode = app->GetOpcode();
	if (opcode == OP_AckPacket) {
		return true;
	}

#if EQDEBUG >= 9
	std::cout << "Received 0x" << std::hex << std::setw(4) << std::setfill('0') << opcode << ", size=" << std::dec << app->size << std::endl;
#endif

	switch (client_state) {
	case CLIENT_CONNECTING: {
		if (ConnectingOpcodes.count(opcode) != 1) {
			if (parse->PlayerHasQuestSub(EVENT_UNHANDLED_OPCODE)) {
				std::vector<std::any> args = {const_cast<EQApplicationPacket *>(app)};
				parse->EventPlayer(EVENT_UNHANDLED_OPCODE, this, "", 1, &args);
			}

			break;
		}

		ClientPacketProc p;
		p = ConnectingOpcodes[opcode];

		//call the processing routine
		(this->*p)(app);

		//special case where connecting code needs to boot client...
		if (client_state == CLIENT_KICKED) {
			return(false);
		}

		break;
	}
	case CLIENT_CONNECTED: {
		ClientPacketProc p;
		p = ConnectedOpcodes[opcode];
		if (p == nullptr) {
			if (parse->PlayerHasQuestSub(EVENT_UNHANDLED_OPCODE)) {
				std::vector<std::any> args = {const_cast<EQApplicationPacket *>(app)};
				parse->EventPlayer(EVENT_UNHANDLED_OPCODE, this, "", 0, &args);
			}

			break;
		}

		//call the processing routine
		(this->*p)(app);
		break;
	}
	case CLIENT_KICKED:
	case DISCONNECTED:
	case CLIENT_LINKDEAD:
		break;
	default:
		LogDebug("Unknown client_state: [{}]\n", static_cast<int>(client_state));
		break;
	}

	return(true);
}

// Finish client connecting state
void Client::CompleteConnect()
{
	UpdateWho();
	client_state = CLIENT_CONNECTED;
	SendAllPackets();
	hpupdate_timer.Start();
	autosave_timer.Start();
	SetDuelTarget(0);
	SetDueling(false);

	EnteringMessages(this);
	LoadPEQZoneFlags();
	LoadZoneFlags();
	LoadAccountFlags();

	/* Sets GM flag if needed & Sends Petition Queue */
	UpdateAdmin(false);

	// Task Packets
	LoadClientTaskState();

	// moved to dbload and translators since we iterate there also .. keep m_pp values whatever they are when they get here
	/*const auto sbs = EQ::spells::DynamicLookup(ClientVersion(), GetGM())->SpellbookSize;
	for (uint32 spellInt = 0; spellInt < sbs; ++spellInt) {
		if (m_pp.spell_book[spellInt] < 3 || m_pp.spell_book[spellInt] > EQ::spells::SPELL_ID_MAX)
			m_pp.spell_book[spellInt] = 0xFFFFFFFF;
	}*/

	//SendAATable();

	if (GetGM() && (GetHideMe() || GetGMSpeed() || GetGMInvul() || flymode != 0 || tellsoff)) {
		std::vector<std::string> state;
		if (GetHideMe()) {
			state.emplace_back("hidden to all clients");
		}
		if (GetGMSpeed()) {
			state.emplace_back("running at GM speed");
		}
		if (GetGMInvul()) {
			state.emplace_back("invulnerable to all damage");
		}
		if (flymode == GravityBehavior::Flying) {
			state.emplace_back("flying");
		}
		else if (flymode == GravityBehavior::Levitating) {
			state.emplace_back("levitating");
		}
		if (tellsoff) {
			state.emplace_back("ignoring tells");
		}

		if (!state.empty()) {
			Message(Chat::Red, "[GM] You are currently %s.", Strings::Join(state, ", ").c_str());
		}
	}

	uint32 raidid = database.GetRaidID(GetName());
	Raid *raid = nullptr;
	if (raidid > 0) {
		raid = entity_list.GetRaidByID(raidid);
		if (!raid) {
			raid = new Raid(raidid);
			if (raid->GetID() != 0) {
				entity_list.AddRaid(raid, raidid);
				raid->LoadLeadership(); // Recreating raid in new zone, get leadership from DB
			}
			else {
				safe_delete(raid);
			}
		}
		if (raid) {
			SetRaidGrouped(true);
			raid->LearnMembers();
			if (RuleB(Bots, Enabled)) {
				SpawnRaidBotsOnConnect(raid);
			}
			raid->VerifyRaid();
			raid->GetRaidDetails();
			/*
			Only leader should get this; send to all for now till
			I figure out correct creation; can probably also send a no longer leader packet for non leaders
			but not important for now.
			*/
			raid->SendRaidCreate(this);
			raid->SendRaidAdd(GetName(), this);
			raid->SendBulkRaid(this);
			raid->SendGroupUpdate(this);
			raid->SendRaidMOTD(this);
			if (raid->IsLeader(this)) { // We're a raid leader, lets update just in case!
				raid->UpdateRaidAAs();
				raid->SendAllRaidLeadershipAA();
			}
			raid->SendMakeLeaderPacketTo(raid->leadername, this);
			uint32 grpID = raid->GetGroup(GetName());
			if (grpID < 12) {
				raid->SendRaidGroupRemove(GetName(), grpID);
				raid->SendRaidGroupAdd(GetName(), grpID);
				raid->CheckGroupMentor(grpID, this);
				if (raid->IsGroupLeader(GetName())) { // group leader same thing!
					raid->UpdateGroupAAs(raid->GetGroup(this));
					raid->GroupUpdate(grpID, false);
				}
			}
			raid->SendGroupLeadershipAA(this, grpID); // this may get sent an extra time ...

			SetXTargetAutoMgr(raid->GetXTargetAutoMgr());
			if (!GetXTargetAutoMgr()->empty())
				SetDirtyAutoHaters();

			if (raid->IsLocked())
				raid->SendRaidLockTo(this);

			raid->SendHPManaEndPacketsTo(this);
			raid->SendAssistTarget(this);
			raid->SendMarkTargets(this);
		}
	}
	else {
		Group *group = nullptr;
		group = GetGroup();
		if (group)
			group->SendHPManaEndPacketsTo(this);
	}


	//bulk raid send in here eventually

	//reapply some buffs
	uint32 buff_count = GetMaxTotalSlots();
	for (uint32 j1 = 0; j1 < buff_count; j1++) {
		if (!IsValidSpell(buffs[j1].spellid))
			continue;

		const SPDat_Spell_Struct &spell = spells[buffs[j1].spellid];

		int NimbusEffect = GetSpellNimbusEffect(buffs[j1].spellid);
		if (NimbusEffect) {
			if (!IsNimbusEffectActive(NimbusEffect))
				SendSpellEffect(NimbusEffect, 500, 0, 1, 3000, true);
		}

		for (int x1 = 0; x1 < EFFECT_COUNT; x1++) {
			switch (spell.effect_id[x1]) {
			case SpellEffect::Illusion: {
				if (GetIllusionBlock()) {
					break;
				}

				if (buffs[j1].persistant_buff) {
					Mob *caster = entity_list.GetMobID(buffs[j1].casterid);
					ApplySpellEffectIllusion(spell.id, caster, j1, spell.base_value[x1], spell.limit_value[x1], spell.max_value[x1]);
				}
				break;
			}
			case SpellEffect::SummonHorse: {
				if (RuleB(Character, PreventMountsFromZoning) || !zone->CanCastOutdoor()) {
					BuffFadeByEffect(SpellEffect::SummonHorse);
				} else {
					SummonHorse(buffs[j1].spellid);
				}
				break;
			}
			case SpellEffect::Silence:
			{
				Silence(true);
				break;
			}
			case SpellEffect::Amnesia:
			{
				Amnesia(true);
				break;
			}
			case SpellEffect::DivineAura:
			{
				invulnerable = true;
				break;
			}
			case SpellEffect::Invisibility2:
			case SpellEffect::Invisibility:
			{
				SendAppearancePacket(AppearanceType::Invisibility, Invisibility::Invisible);
				break;
			}
			case SpellEffect::Levitate:
			{
				if (!zone->CanLevitate()) {
					if (!GetGM()) {
						SendAppearancePacket(AppearanceType::FlyMode, 0);
						BuffFadeByEffect(SpellEffect::Levitate);
						Message(Chat::Red, "You can't levitate in this zone.");
						break;
					}

					Message(Chat::White, "Your GM flag allows you to levitate in this zone.");
				}

				SendAppearancePacket(
					AppearanceType::FlyMode,
					(
						spell.limit_value[x1] == 1 ?
							EQ::constants::GravityBehavior::LevitateWhileRunning :
							EQ::constants::GravityBehavior::Levitating
					),
					true,
					true
				);

				break;
			}
			case SpellEffect::AddMeleeProc:
			case SpellEffect::WeaponProc:
			{
				AddProcToWeapon(GetProcID(buffs[j1].spellid, x1), false, 100 + spells[buffs[j1].spellid].limit_value[x1], buffs[j1].spellid, buffs[j1].casterlevel, GetSpellProcLimitTimer(buffs[j1].spellid, ProcType::MELEE_PROC));
				break;
			}
			case SpellEffect::DefensiveProc:
			{
				AddDefensiveProc(GetProcID(buffs[j1].spellid, x1), 100 + spells[buffs[j1].spellid].limit_value[x1], buffs[j1].spellid, GetSpellProcLimitTimer(buffs[j1].spellid, ProcType::DEFENSIVE_PROC));
				break;
			}
			case SpellEffect::RangedProc:
			{
				AddRangedProc(GetProcID(buffs[j1].spellid, x1), 100 + spells[buffs[j1].spellid].limit_value[x1], buffs[j1].spellid, GetSpellProcLimitTimer(buffs[j1].spellid, ProcType::RANGED_PROC));
				break;
			}
			}
		}
	}

	/* Sends appearances for all mobs not doing Animation::Standing aka sitting, looting, playing dead */
	entity_list.SendZoneAppearance(this);
	/* Sends the Nimbus particle effects (up to 3) for any mob using them */
	entity_list.SendNimbusEffects(this);

	entity_list.SendUntargetable(this);

	entity_list.SendAppearanceEffects(this);

	entity_list.SendIllusionWearChange(this);

	SendWearChangeAndLighting(EQ::textures::LastTexture);
	Mob* pet = GetPet();
	if (pet) {
		pet->SendWearChangeAndLighting(EQ::textures::LastTexture);
		pet->SendPetBuffsToClient();
	}

	if (GetGroup())
		database.RefreshGroupFromDB(this);

	if (RuleB(TaskSystem, EnableTaskSystem))
		TaskPeriodic_Timer.Start();
	else
		TaskPeriodic_Timer.Disable();

	conn_state = ClientConnectFinished;

	if (zone)
		zone->weatherSend(this);

	TotalKarma = database.GetKarma(AccountID());
	SendDisciplineTimers();

	if (parse->PlayerHasQuestSub(EVENT_ENTER_ZONE)) {
		parse->EventPlayer(EVENT_ENTER_ZONE, this, "", 0);
	}

	if (parse->ZoneHasQuestSub(EVENT_ENTER_ZONE)) {
		std::vector<std::any> args = { this };
		parse->EventZone(EVENT_ENTER_ZONE, zone, "", 0, &args);
	}

	DeleteEntityVariable(SEE_BUFFS_FLAG);

	// the way that the client deals with positions during the initial spawn struct
	// is subtly different from how it deals with getting a position update
	// if a mob is slightly in the wall or slightly clipping a floor they will be
	// sent to a succor point
	SendMobPositions();

	m_last_position_before_bulk_update = GetPosition();

	/* This sub event is for if a player logs in for the first time since entering world. */
	if (ingame) {
		auto e = CharacterDataRepository::FindOne(
			database,
			CharacterID()
		);

		bool is_first_login = e.first_login == 0;

		RecordPlayerEventLog(PlayerEvent::WENT_ONLINE, PlayerEvent::EmptyEvent{});

		if (parse->PlayerHasQuestSub(EVENT_CONNECT)) {
			const std::string& export_string = fmt::format(
				"{} {} {}",
				e.last_login,
				time(nullptr) - e.last_login,
				is_first_login ? 1 : 0
			);
			parse->EventPlayer(EVENT_CONNECT, this, export_string, 0);
		}

		if (is_first_login) {
			e.first_login = time(nullptr);
			TraderRepository::DeleteWhere(database, fmt::format("`char_id` = '{}'", CharacterID()));
			BuyerRepository::DeleteBuyer(database, CharacterID());
			LogTradingDetail(
				"Removed trader abd buyer entries for Character ID {} on first logon to ensure table consistency.",
				CharacterID()
			);
		}

		e.last_login = time(nullptr);

		const int updated = CharacterDataRepository::UpdateOne(database, e);
		if (!updated) {
			LogError("Failed to update login time for character_id [{}]", CharacterID());
		}

		if (IsPetNameChangeAllowed() && !RuleB(Pets, AlwaysAllowPetRename)) {
			InvokeChangePetName(false);
		}

		if (IsNameChangeAllowed() && !RuleB(Character, AlwaysAllowNameChange)) {
			InvokeChangeNameWindow(false);
		}
	}

	if(ClientVersion() == EQ::versions::ClientVersion::RoF2 && RuleB(Parcel, EnableParcelMerchants)) {
		SendParcelStatus();
	}

	if (zone && zone->GetInstanceTimer()) {
		bool is_permanent = false;
		uint32 remaining_time = database.GetTimeRemainingInstance(zone->GetInstanceID(), is_permanent);
		auto time_string = Strings::SecondsToTime(remaining_time);
		Message(
			Chat::Yellow,
			fmt::format(
				"{} will expire in {}.",
				zone->GetZoneDescription(),
				time_string
			).c_str()
		);
	}

	SendRewards();
	SendAltCurrencies();
	database.LoadAltCurrencyValues(CharacterID(), alternate_currency);
	SendAlternateCurrencyValues();
	alternate_currency_loaded = true;
	ProcessAlternateCurrencyQueue();

	/* This needs to be set, this determines whether or not data was loaded properly before a save */
	client_data_loaded = true;

	CalcItemScale();
	DoItemEnterZone();

	if (zone->GetZoneID() == Zones::GUILDHALL && GuildBanks)
		GuildBanks->SendGuildBank(this);

	if (ClientVersion() >= EQ::versions::ClientVersion::SoD)
		entity_list.SendFindableNPCList(this);

	if (IsInAGuild()) {
		if (ingame) {
			guild_mgr.UpdateDbMemberOnline(CharacterID(), true);
			SendGuildMembersList();
		}

		guild_mgr.SendGuildMemberUpdateToWorld(GetName(), GuildID(), zone->GetZoneID(), time(nullptr));

		SendGuildList();
		if (GetGuildListDirty()) {
			SendGuildMembersList();
		}

		if (ClientVersion() >= EQ::versions::ClientVersion::RoF) {
			SendGuildRanks();
			SendGuildRankNames();
		}

		SendGuildActiveTributes(GuildID());
		SendGuildFavorAndTimer(GuildID());
		DoGuildTributeUpdate();

		auto guild = guild_mgr.GetGuildByGuildID(GuildID());
		if (guild) {
			ServerPacket* out = new ServerPacket(ServerOP_GuildTributeOptInToggle, sizeof(GuildTributeMemberToggle));
			GuildTributeMemberToggle* data = (GuildTributeMemberToggle*)out->pBuffer;
			CharGuildInfo gci;
			guild_mgr.GetCharInfo(CharacterID(), gci);

			data->char_id        = CharacterID();
			data->command        = 0x2fb;
			data->tribute_toggle = GuildTributeOptIn();
			data->guild_id       = GuildID();
			data->no_donations   = gci.total_tribute;
			data->time_remaining = guild_mgr.GetGuildTributeTimeRemaining(GuildID());
			strncpy(data->player_name, GetCleanName(), sizeof(data->player_name));

			worldserver.SendPacket(out);
			safe_delete(out)
		}
	}

	SendDynamicZoneUpdates();

	// Request adventure info
	auto members = AdventureMembersRepository::GetWhere(database, fmt::format("charid = {}", CharacterID()));
	if (!members.empty()) {
		auto pack = new ServerPacket(ServerOP_AdventureDataRequest, 64);
		strcpy((char*)pack->pBuffer, GetName());
		worldserver.SendPacket(pack);
		delete pack;
	}

	if (IsClient() && CastToClient()->ClientVersionBit() & EQ::versions::maskUFAndLater) {
		EQApplicationPacket *outapp = MakeBuffsPacket(false);
		CastToClient()->FastQueuePacket(&outapp);
	}

	// TODO: load these states
	// We at least will set them to the correct state for now
	if (m_ClientVersionBit & EQ::versions::maskUFAndLater && GetPet()) {
		SetPetCommandState(PetButton::Sit, PetButtonState::Off);
		SetPetCommandState(PetButton::Stop, PetButtonState::Off);
		SetPetCommandState(PetButton::Regroup, PetButtonState::Off);
		SetPetCommandState(PetButton::Follow, PetButtonState::On);
		SetPetCommandState(PetButton::Guard, PetButtonState::Off);
		// Taunt saved on client side for logging on with pet
		// In our db for when we zone.
		SetPetCommandState(PetButton::Hold, PetButtonState::Off);
		SetPetCommandState(PetButton::GreaterHold, PetButtonState::Off);
		SetPetCommandState(PetButton::Focus, PetButtonState::Off);
		SetPetCommandState(PetButton::SpellHold, PetButtonState::Off);
	}

	database.LoadAuras(this); // this ends up spawning them so probably safer to load this later (here)
	database.LoadCharacterDisciplines(this);

	entity_list.RefreshClientXTargets(this);

	worldserver.RequestTellQueue(GetName());

	entity_list.ScanCloseMobs(this);

	if (GetGM() && IsDevToolsEnabled()) {
		ShowDevToolsMenu();
	}

	auto z = GetZone(GetZoneID(), GetInstanceVersion());
	if (z && z->shard_at_player_count > 0 && !RuleB(Zone, ZoneShardQuestMenuOnly)) {
		ShowZoneShardMenu();
	}

	// shared tasks memberlist
	if (RuleB(TaskSystem, EnableTaskSystem) && GetTaskState()->HasActiveSharedTask()) {

		// struct
		auto p = new ServerPacket(
			ServerOP_SharedTaskRequestMemberlist,
			sizeof(ServerSharedTaskRequestMemberlist_Struct)
		);

		auto *r = (ServerSharedTaskRequestMemberlist_Struct *) p->pBuffer;

		// fill
		r->source_character_id = CharacterID();
		r->task_id             = GetTaskState()->GetActiveSharedTask().task_id;

		// send
		worldserver.SendPacket(p);
		safe_delete(p);
	}

	RecordStats();
	AutoGrantAAPoints();

	// set initial position for mob tracking
	m_last_seen_mob_position.reserve(entity_list.GetMobList().size());
	for (auto& mob : entity_list.GetMobList()) {
		if (!mob.second->IsNPC()) {
			continue;
		}

		m_last_seen_mob_position[mob.second->GetID()] = mob.second->GetPosition();
	}

	// enforce some rules..
	if (!CanEnterZone()) {
		LogInfo("Kicking character [{}] from zone, not allowed here (missing requirements)", GetCleanName());
		GoToBind();
		return;
	}
}

// connecting opcode handlers
/*
void Client::Handle_Connect_0x3e33(const EQApplicationPacket *app)
{
//OP_0x0380 = 0x642c
EQApplicationPacket* outapp = new EQApplicationPacket(OP_0x0380, sizeof(uint32)); // Dunno
QueuePacket(outapp);
safe_delete(outapp);
return;
}
*/

void Client::Handle_Connect_OP_ApproveZone(const EQApplicationPacket *app)
{
	if (app->size != sizeof(ApproveZone_Struct)) {
		LogError("Invalid size on OP_ApproveZone: Expected [{}], Got [{}]",
			sizeof(ApproveZone_Struct), app->size);
		return;
	}
	ApproveZone_Struct* azone = (ApproveZone_Struct*)app->pBuffer;
	azone->approve = 1;
	QueuePacket(app);
	return;
}

void Client::Handle_Connect_OP_ClientError(const EQApplicationPacket *app)
{
	if (app->size != sizeof(ClientError_Struct)) {
		LogError("Invalid size on OP_ClientError: Expected [{}], Got [{}]",
			sizeof(ClientError_Struct), app->size);
		return;
	}
	// Client reporting error to server
	ClientError_Struct* error = (ClientError_Struct*)app->pBuffer;
	LogError("Client error: [{}]", error->character_name);
	LogError("Error message: [{}]", error->message);
	Message(Chat::Red, error->message);
#if (EQDEBUG>=5)
	DumpPacket(app);
#endif
	return;
}

void Client::Handle_Connect_OP_ClientReady(const EQApplicationPacket *app)
{
	conn_state = ClientReadyReceived;
	if (!Spawned())
		SendZoneInPackets();
	CompleteConnect();
	SendHPUpdate();
}

void Client::Handle_Connect_OP_ClientUpdate(const EQApplicationPacket *app)
{
	//Once we get this, the client thinks it is connected
	//So give it the benefit of the doubt and move to connected

	Handle_Connect_OP_ClientReady(app);
}

void Client::Handle_Connect_OP_ReqClientSpawn(const EQApplicationPacket *app)
{
	conn_state = ClientSpawnRequested;

	auto outapp = new EQApplicationPacket;

	// Send Zone Doors
	if (entity_list.MakeDoorSpawnPacket(outapp, this))
	{
		QueuePacket(outapp);
	}
	safe_delete(outapp);

	// Send Zone Objects
	entity_list.SendZoneObjects(this);
	SendZonePoints();
	// Live does this
	outapp = new EQApplicationPacket(OP_SendAAStats, 0);
	FastQueuePacket(&outapp);

	// Tell client they can continue we're done
	outapp = new EQApplicationPacket(OP_ZoneServerReady, 0);
	FastQueuePacket(&outapp);
	outapp = new EQApplicationPacket(OP_SendExpZonein, 0);
	FastQueuePacket(&outapp);

	if (ClientVersion() >= EQ::versions::ClientVersion::RoF)
	{
		outapp = new EQApplicationPacket(OP_ClientReady, 0);
		FastQueuePacket(&outapp);
	}

	// New for Secrets of Faydwer - Used in Place of OP_SendExpZonein
	outapp = new EQApplicationPacket(OP_WorldObjectsSent, 0);
	QueuePacket(outapp);
	safe_delete(outapp);

	conn_state = ZoneContentsSent;

	return;
}

void Client::Handle_Connect_OP_ReqNewZone(const EQApplicationPacket *app)
{
	conn_state = NewZoneRequested;

	EQApplicationPacket* outapp = nullptr;

	/////////////////////////////////////
	// New Zone Packet
	outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
	NewZone_Struct* nz = (NewZone_Struct*)outapp->pBuffer;
	memcpy(outapp->pBuffer, &zone->newzone_data, sizeof(NewZone_Struct));
	strcpy(nz->char_name, m_pp.name);

	// This was using FastQueuePacket and the packet was never getting sent...
	// Not sure if this was timing....  but the NewZone was never logged until
	// I changed it.
	outapp->priority = 6;
	QueuePacket(outapp);
	safe_delete(outapp);

	return;
}

void Client::Handle_Connect_OP_SendAAStats(const EQApplicationPacket *app)
{
	SendAlternateAdvancementTimers();
	auto outapp = new EQApplicationPacket(OP_SendAAStats, 0);
	QueuePacket(outapp);
	safe_delete(outapp);
	return;
}

void Client::Handle_Connect_OP_SendAATable(const EQApplicationPacket *app)
{
	SendAlternateAdvancementTable();
	return;
}

void Client::Handle_Connect_OP_SendExpZonein(const EQApplicationPacket *app)
{
	auto outapp = new EQApplicationPacket(OP_SendExpZonein, 0);
	QueuePacket(outapp);
	safe_delete(outapp);

	// SoF+ Gets Zone-In packets after sending OP_WorldObjectsSent
	if (ClientVersion() < EQ::versions::ClientVersion::SoF)
	{
		SendZoneInPackets();
	}

	return;
}

void Client::Handle_Connect_OP_SendGuildTributes(const EQApplicationPacket *app)
{
	SendGuildTributes();
	return;
}

void Client::Handle_Connect_OP_SendTributes(const EQApplicationPacket *app)
{
	SendTributes();
	return;
}

void Client::Handle_Connect_OP_SetServerFilter(const EQApplicationPacket *app)
{
	if (app->size != sizeof(SetServerFilter_Struct)) {
		LogError("Received invalid sized OP_SetServerFilter");
		DumpPacket(app);
		return;
	}
	SetServerFilter_Struct* filter = (SetServerFilter_Struct*)app->pBuffer;
	ServerFilter(filter);
	return;
}

void Client::Handle_Connect_OP_SpawnAppearance(const EQApplicationPacket *app)
{
	return;
}

void Client::Handle_Connect_OP_TGB(const EQApplicationPacket *app)
{
	if (app->size != sizeof(uint32)) {
		LogError("Invalid size on OP_TGB: Expected [{}], Got [{}]",
			sizeof(uint32), app->size);
		return;
	}
	OPTGB(app);
	return;
}

void Client::Handle_Connect_OP_UpdateAA(const EQApplicationPacket *app)
{
	SendAlternateAdvancementPoints();
}

void Client::Handle_Connect_OP_WearChange(const EQApplicationPacket *app)
{
	//not sure what these are supposed to mean to us.
	return;
}

void Client::Handle_Connect_OP_WorldObjectsSent(const EQApplicationPacket *app)
{
	// New for SoF+
	auto outapp = new EQApplicationPacket(OP_WorldObjectsSent, 0);
	QueuePacket(outapp);
	safe_delete(outapp);

	// Packet order changed for SoF+, so below is sent here instead of OP_SendExpLogin
	SendZoneInPackets();

	if (RuleB(Mercs, AllowMercs))
	{
		SpawnMercOnZone();
	}

	return;
}

void Client::Handle_Connect_OP_ZoneComplete(const EQApplicationPacket *app)
{
	auto outapp = new EQApplicationPacket(OP_0x0347, 0);
	QueuePacket(outapp);
	safe_delete(outapp);
	return;
}

void Client::Handle_Connect_OP_ZoneEntry(const EQApplicationPacket *app)
{
	if (app->size != sizeof(ClientZoneEntry_Struct))
		return;
	ClientZoneEntry_Struct *cze = (ClientZoneEntry_Struct *)app->pBuffer;

	if (strlen(cze->char_name) > 63)
		return;

	conn_state = ReceivedZoneEntry;

	SetClientVersion(Connection()->ClientVersion());
	m_ClientVersionBit = EQ::versions::ConvertClientVersionToClientVersionBit(Connection()->ClientVersion());

	m_pp.SetPlayerProfileVersion(m_ClientVersion);
	m_inv.SetInventoryVersion(m_ClientVersion);

	/* Antighost code
	tmp var is so the search doesnt find this object
	*/
	Client* client = entity_list.GetClientByName(cze->char_name);
	if (!zone->GetAuth(ip, cze->char_name, &WID, &account_id, &character_id, &admin, lskey, &tellsoff)) {
		LogClientLogin("[{}] failed zone auth check", cze->char_name);
		if (nullptr != client) {
			client->Save();
			client->Kick("Failed auth check");
		}
		return;
	}

	strcpy(name, cze->char_name);
	/* Check for Client Spoofing */
	if (client != 0) {
		struct in_addr ghost_addr;
		ghost_addr.s_addr = eqs->GetRemoteIP();

		LogError("Ghosting client: Account ID:[{}] Name:[{}] Character:[{}] IP:[{}]",
			client->AccountID(), client->AccountName(), client->GetName(), inet_ntoa(ghost_addr));
		client->Save();
		client->Disconnect();
	}

	uint32 pplen = 0;
	EQApplicationPacket* outapp = nullptr;
	bool loaditems = 0;
	std::string query;

	uint32 cid = CharacterID();
	character_id = cid; /* Global character_id reference */

	/* Flush and reload factions */
	database.RemoveTempFactions(this);
	database.LoadCharacterFactionValues(cid, factionvalues);

	auto a = AccountRepository::FindOne(database, AccountID());
	if (a.id > 0) {
		strn0cpy(account_name, a.name.c_str(), sizeof(account_name));
		strn0cpy(loginserver, a.ls_id.c_str(), sizeof(loginserver));

		admin            = a.status;
		lsaccountid      = a.lsaccount_id;
		gmspeed          = a.gmspeed;
		revoked          = a.revoked;
		gm_hide_me       = a.hideme;
		account_creation = a.time_creation;
		gminvul          = a.invulnerable;
		flymode          = static_cast<GravityBehavior>(a.flymode);
		tellsoff         = gm_hide_me;
	}

	/* Load Character Data */
	query = fmt::format(
		"SELECT `lfp`, `lfg`, `xtargets`, `first_login`, `guild_id`, `rank`, `exp_enabled`, `tribute_enable`, `extra_haste`, `illusion_block`, `ingame` FROM `character_data` LEFT JOIN `guild_members` ON `id` = `char_id` WHERE `id` = {}",
		cid
	);
	auto results = database.QueryDatabase(query);
	for (auto row : results) {
		if (row[4] && Strings::ToInt(row[4]) > 0) {
			guild_id             = Strings::ToInt(row[4]);
			guildrank            = row[5] ? Strings::ToInt(row[5]) : GUILD_RANK_NONE;
			guild_tribute_opt_in = row[7] ? Strings::ToBool(row[7]) : 0;
		}

		SetEXPEnabled(Strings::ToBool(row[6]));
		SetExtraHaste(Strings::ToInt(row[8]), false);
		SetIllusionBlock(Strings::ToBool(row[9]));

		if (LFP) {
			LFP = Strings::ToInt(row[0]);
		}

		if (LFG) {
			LFG = Strings::ToInt(row[1]);
		}

		if (row[3]) {
			first_login = Strings::ToUnsignedInt(row[3]);
		}

		if (row[10]) {
			ingame = Strings::ToBool(row[10]);
		}
	}

	if (RuleB(Character, SharedBankPlat))
		m_pp.platinum_shared = database.GetSharedPlatinum(AccountID());

	database.ClearOldRecastTimestamps(cid); /* Clear out our old recast timestamps to keep the DB clean */
	// set to full support in case they're a gm with items in disabled expansion slots...but, have their gm flag off...
	// item loss will occur when they use the 'empty' slots, if this is not done
	m_inv.SetGMInventory(true);
	loaditems = database.GetInventory(this); /* Load Character Inventory */
	database.LoadCharacterBandolier(cid, &m_pp); /* Load Character Bandolier */
	database.LoadCharacterBindPoint(cid, &m_pp); /* Load Character Bind */
	database.LoadCharacterMaterialColor(cid, &m_pp); /* Load Character Material */
	database.LoadCharacterPotionBelt(cid, &m_pp); /* Load Character Potion Belt */
	database.LoadCharacterCurrency(cid, &m_pp); /* Load Character Currency into PP */
	database.LoadCharacterData(cid, &m_pp, &m_epp); /* Load Character Data from DB into PP as well as E_PP */
	database.LoadCharacterSkills(cid, &m_pp); /* Load Character Skills */
	database.LoadCharacterInspectMessage(cid, &m_inspect_message); /* Load Character Inspect Message */
	database.LoadCharacterSpellBook(cid, &m_pp); /* Load Character Spell Book */
	database.LoadCharacterMemmedSpells(cid, &m_pp);  /* Load Character Memorized Spells */
	database.LoadCharacterLanguages(cid, &m_pp); /* Load Character Languages */
	database.LoadCharacterLeadershipAbilities(cid, &m_pp); /* Load Character Leadership AA's */
	database.LoadCharacterTribute(this); /* Load CharacterTribute */
	database.LoadCharacterEXPModifier(this); /* Load Character EXP Modifier */
	database.LoadCharacterTitleSets(this); /* Load Character Title Sets */

	// this pattern is strange
	// this is remnants of the old way of doing things
	auto mail_keys = database.GetMailKey(CharacterID());
	m_mail_key_full = mail_keys.mail_key_full;
	m_mail_key      = mail_keys.mail_key;

	/* Load AdventureStats */
	AdventureStats_Struct as;
	if (database.GetAdventureStats(cid, &as))
	{
		m_pp.ldon_wins_guk = as.success.guk;
		m_pp.ldon_wins_mir = as.success.mir;
		m_pp.ldon_wins_mmc = as.success.mmc;
		m_pp.ldon_wins_ruj = as.success.ruj;
		m_pp.ldon_wins_tak = as.success.tak;
		m_pp.ldon_losses_guk = as.failure.guk;
		m_pp.ldon_losses_mir = as.failure.mir;
		m_pp.ldon_losses_mmc = as.failure.mmc;
		m_pp.ldon_losses_ruj = as.failure.ruj;
		m_pp.ldon_losses_tak = as.failure.tak;
	}

	/* Set item material tint */
	for (int i = EQ::textures::textureBegin; i <= EQ::textures::LastTexture; i++)
	{
		if (m_pp.item_tint.Slot[i].UseTint == 1 || m_pp.item_tint.Slot[i].UseTint == 255)
		{
			m_pp.item_tint.Slot[i].UseTint = 0xFF;
		}
	}

	if (level) { level = m_pp.level; }

	/* If GM, not trackable */
	if (gm_hide_me) { trackable = false; }
	if (gminvul) { invulnerable = true; }
	if (flymode > 0) { SendAppearancePacket(AppearanceType::FlyMode, flymode); }
	/* Set Con State for Reporting */
	conn_state = PlayerProfileLoaded;

	m_pp.zone_id = zone->GetZoneID();
	m_pp.zoneInstance = zone->GetInstanceID();

	/* Set Total Seconds Played */
	TotalSecondsPlayed = m_pp.timePlayedMin * 60;

	/* If we can maintain intoxication across zones, check for it */
	if (!RuleB(Character, MaintainIntoxicationAcrossZones))
		SetIntoxication(0);

	strcpy(name, m_pp.name);
	strcpy(lastname, m_pp.last_name);
	/* If PP is set to weird coordinates */
	if ((m_pp.x == -1 && m_pp.y == -1 && m_pp.z == -1) || (m_pp.x == -2 && m_pp.y == -2 && m_pp.z == -2)) {
		auto zone_safe_point = zone->GetSafePoint();
		m_pp.x = zone_safe_point.x;
		m_pp.y = zone_safe_point.y;
		m_pp.z = zone_safe_point.z;
		m_pp.heading = zone_safe_point.w;
	}
	/* If too far below ground, then fix */
	// float ground_z = GetGroundZ(m_pp.x, m_pp.y, m_pp.z);
	// if (m_pp.z < (ground_z - 500))
	// 	m_pp.z = ground_z;

	/* Set Mob variables for spawn */
	class_ = m_pp.class_;
	level = m_pp.level;
	m_Position.x = m_pp.x;
	m_Position.y = m_pp.y;
	m_Position.z = m_pp.z;
	m_Position.w = m_pp.heading;
	race = m_pp.race;
	base_race = m_pp.race;
	gender = m_pp.gender;
	base_gender = m_pp.gender;
	deity = m_pp.deity;
	haircolor = m_pp.haircolor;
	beardcolor = m_pp.beardcolor;
	eyecolor1 = m_pp.eyecolor1;
	eyecolor2 = m_pp.eyecolor2;
	hairstyle = m_pp.hairstyle;
	luclinface = m_pp.face;
	beard = m_pp.beard;
	drakkin_heritage = m_pp.drakkin_heritage;
	drakkin_tattoo = m_pp.drakkin_tattoo;
	drakkin_details = m_pp.drakkin_details;

	// Load Data Buckets
	ClearDataBucketCache();
	LoadDataBucketsCache();

	// Max Level for Character:PerCharacterQglobalMaxLevel and Character:PerCharacterBucketMaxLevel
	uint8 client_max_level = 0;
	if (RuleB(Character, PerCharacterQglobalMaxLevel)) {
		client_max_level = GetCharMaxLevelFromQGlobal();
	} else if (RuleB(Character, PerCharacterBucketMaxLevel)) {
		client_max_level = GetCharMaxLevelFromBucket();
	}
	SetClientMaxLevel(client_max_level);

	// we know our class now, so we might have to fix our consume timer!
	if (class_ == Class::Monk)
		consume_food_timer.SetTimer(CONSUMPTION_MNK_TIMER);

	InitInnates();

	/* Load Guild */
	if (!IsInAGuild()) {
		m_pp.guild_id = GUILD_NONE;
	}
	else {
		m_pp.guild_id = GuildID();
		CharGuildInfo cgi;
		if (guild_mgr.GetCharInfo(CharacterID(), cgi)) {
			m_pp.guildrank = cgi.rank;
		}
		if (zone->GetZoneID() == Zones::GUILDHALL) {
			GuildBanker = (guild_mgr.IsGuildLeader(GuildID(), CharacterID()) ||
				guild_mgr.GetBankerFlag(CharacterID()) ||
				ClientVersion() >= EQ::versions::ClientVersion::RoF ? true : false
				);
		}
	}
	m_pp.guildbanker = GuildBanker;

	switch (race)
	{
	case Race::Ogre:
		size = 9; break;
	case Race::Troll:
		size = 8; break;
	case Race::VahShir: case Race::Barbarian:
		size = 7; break;
	case Race::Human: case Race::HighElf: case Race::Erudite: case Race::Iksar: case Race::Drakkin:
		size = 6; break;
	case Race::HalfElf:
		size = 5.5; break;
	case Race::WoodElf: case Race::DarkElf: case Race::Froglok2:
		size = 5; break;
	case Race::Dwarf:
		size = 4; break;
	case Race::Halfling:
		size = 3.5; break;
	case Race::Gnome:
		size = 3; break;
	default:
		size = 0;
	}

	/* Check for Invalid points */
	if (m_pp.ldon_points_guk < 0 || m_pp.ldon_points_guk > 2000000000) { m_pp.ldon_points_guk = 0; }
	if (m_pp.ldon_points_mir < 0 || m_pp.ldon_points_mir > 2000000000) { m_pp.ldon_points_mir = 0; }
	if (m_pp.ldon_points_mmc < 0 || m_pp.ldon_points_mmc > 2000000000) { m_pp.ldon_points_mmc = 0; }
	if (m_pp.ldon_points_ruj < 0 || m_pp.ldon_points_ruj > 2000000000) { m_pp.ldon_points_ruj = 0; }
	if (m_pp.ldon_points_tak < 0 || m_pp.ldon_points_tak > 2000000000) { m_pp.ldon_points_tak = 0; }
	if (m_pp.ldon_points_available < 0 || m_pp.ldon_points_available > 2000000000) { m_pp.ldon_points_available = 0; }

	ReloadExpansionProfileSetting();

	if (!database.LoadAlternateAdvancement(this)) {
		LogError("Error loading AA points for [{}]", GetName());
	}

	if (RuleB(Bots, Enabled)) {
		LoadDefaultBotSettings();
		database.botdb.LoadBotSettings(this);
	}

	if (SPDAT_RECORDS > 0) {
		for (uint32 z = 0; z < EQ::spells::SPELL_GEM_COUNT; z++) {
			if (m_pp.mem_spells[z] >= (uint32)SPDAT_RECORDS)
				UnmemSpell(z, false);
		}

		database.LoadBuffs(this);
		uint32 max_slots = GetMaxBuffSlots();
		for (int i = 0; i < BUFF_COUNT; i++) {
			if (IsValidSpell(buffs[i].spellid)) {
				m_pp.buffs[i].spellid = buffs[i].spellid;
				m_pp.buffs[i].bard_modifier = buffs[i].instrument_mod;
				m_pp.buffs[i].effect_type = 2;
				m_pp.buffs[i].player_id = 0x2211;
				m_pp.buffs[i].level = buffs[i].casterlevel;
				m_pp.buffs[i].unknown003 = 0;
				m_pp.buffs[i].duration = buffs[i].ticsremaining;
				m_pp.buffs[i].counters = buffs[i].counters;
				m_pp.buffs[i].num_hits = buffs[i].hit_number;
			}
			else {
				m_pp.buffs[i].spellid = SPELLBOOK_UNKNOWN;
				m_pp.buffs[i].bard_modifier = 10;
				m_pp.buffs[i].effect_type = 0;
				m_pp.buffs[i].player_id = 0;
				m_pp.buffs[i].level = 0;
				m_pp.buffs[i].unknown003 = 0;
				m_pp.buffs[i].duration = 0;
				m_pp.buffs[i].counters = 0;
				m_pp.buffs[i].num_hits = 0;
			}
		}
	}

	/* Load Character Key Ring */
	KeyRingLoad();

	/* Send Group Members via PP */
	uint32 groupid = database.GetGroupID(GetName());
	Group* group = nullptr;
	if (groupid > 0) {
		group = entity_list.GetGroupByID(groupid);
		if (!group) {	//nobody from our is here... start a new group
			group = new Group(groupid);
			if (group->GetID() != 0)
				entity_list.AddGroup(group, groupid);
			else	//error loading group members...
			{
				delete group;
				group = nullptr;
			}
		}	//else, somebody from our group is already here...

		if (!group) { //cannot re-establish group, kill it
			Group::RemoveFromGroup(this);
		}

	}
	else {	//no group id
			//clear out the group junk in our PP
		uint32 xy = 0;
		for (xy = 0; xy < MAX_GROUP_MEMBERS; xy++)
			memset(m_pp.groupMembers[xy], 0, 64);
	}

	if (group) {
		char ln[64];
		char MainTankName[64];
		char AssistName[64];
		char PullerName[64];
		char NPCMarkerName[64];
		char mentoree_name[64];
		int mentor_percent;
		GroupLeadershipAA_Struct GLAA;
		memset(ln, 0, 64);
		database.GetGroupLeadershipInfo(group->GetID(), ln, MainTankName, AssistName, PullerName, NPCMarkerName, mentoree_name, &mentor_percent, &GLAA);
		group->LearnMembers();

		if (!group->GetLeader()) {
			Client *c = entity_list.GetClientByName(ln);
			if (c) {
				group->SetLeader(c);
			}
		}

		group->SetMainTank(MainTankName);
		group->SetMainAssist(AssistName);
		group->SetPuller(PullerName);
		group->SetNPCMarker(NPCMarkerName);
		group->SetGroupAAs(&GLAA);
		group->SetGroupMentor(mentor_percent, mentoree_name);
		JoinGroupXTargets(group);
		group->UpdatePlayer(this);
		LFG = false;
	}

	if (RuleB(Bots, Enabled)) {
		database.botdb.LoadOwnerOptions(this);
		// TODO: mod below function for loading spawned botgroups
		Bot::LoadAndSpawnAllZonedBots(this);
	}

	// If we are the leader, force an update of our group AAs to other members in the zone, in case
	// we purchased a new one while out-of-zone.
	// needs to be done after spawning bots
	if (group && group->IsLeader(this)) {
		group->SendLeadershipAAUpdate();
	}

	m_inv.SetGMInventory((bool)m_pp.gm); // set to current gm state for calc
	CalcBonuses();
	if (RuleB(Zone, EnableLoggedOffReplenishments) &&
		time(nullptr) - m_pp.lastlogin >= RuleI(Zone, MinOfflineTimeToReplenishments)) {
		m_pp.cur_hp = GetMaxHP();
		m_pp.mana = GetMaxMana();
		m_pp.endurance = GetMaxEndurance();
	}

	if (m_pp.cur_hp <= 0)
		m_pp.cur_hp = GetMaxHP();

	SetHP(m_pp.cur_hp);
	Mob::SetMana(m_pp.mana); // mob function doesn't send the packet
	SetEndurance(m_pp.endurance);

	/* Update LFP in case any (or all) of our group disbanded while we were zoning. */
	if (IsLFP()) { UpdateLFP(); }

	p_timers.SetCharID(CharacterID());
	if (!p_timers.Load(&database)) {
		LogError("Unable to load ability timers from the database for [{}] ([{}])!", GetCleanName(), CharacterID());
	}

	/* Load Spell Slot Refresh from Currently Memoried Spells */
	for (unsigned int i = 0; i < EQ::spells::SPELL_GEM_COUNT; ++i)
		if (IsValidSpell(m_pp.mem_spells[i]))
			m_pp.spellSlotRefresh[i] = p_timers.GetRemainingTime(pTimerSpellStart + m_pp.mem_spells[i]) * 1000;

	/* Ability slot refresh send SK/PAL */
	if (m_pp.class_ == Class::ShadowKnight || m_pp.class_ == Class::Paladin) {
		uint32 abilitynum = 0;
		if (m_pp.class_ == Class::ShadowKnight) { abilitynum = pTimerHarmTouch; }
		else { abilitynum = pTimerLayHands; }

		uint32 remaining = p_timers.GetRemainingTime(abilitynum);
		if (remaining > 0 && remaining < 15300)
			m_pp.abilitySlotRefresh = remaining * 1000;
		else
			m_pp.abilitySlotRefresh = 0;
	}

	/* Reset to max so they dont drown on zone in if its underwater */
	m_pp.air_remaining = 60;
	/* Check for PVP Zone status*/
	if (zone->IsPVPZone())
		m_pp.pvp = 1;
	/* Time entitled on Account: Move to account */
	m_pp.timeentitledonaccount = CharacterDataRepository::GetTotalTimePlayed(database, AccountID()) / 1440;
	/* Reset rest timer if the durations have been lowered in the database */
	if ((m_pp.RestTimer > RuleI(Character, RestRegenTimeToActivate)) && (m_pp.RestTimer > RuleI(Character, RestRegenRaidTimeToActivate)))
		m_pp.RestTimer = 0;

	/* This checksum should disappear once dynamic structs are in... each struct strategy will do it */ // looks to be in place now
	//CRC32::SetEQChecksum((unsigned char*)&m_pp, sizeof(PlayerProfile_Struct) - sizeof(m_pp.m_player_profile_version) - 4);
	// m_pp.checksum = 0; // All server out-bound player profile packets are now translated - no need to waste cycles calculating this...

	outapp = new EQApplicationPacket(OP_PlayerProfile, sizeof(PlayerProfile_Struct));

	/* The entityid field in the Player Profile is used by the Client in relation to Group Leadership AA */
	m_pp.entityid = GetID();
	memcpy(outapp->pBuffer, &m_pp, outapp->size);
	outapp->priority = 6;
	FastQueuePacket(&outapp);

	if (m_pp.RestTimer) {
		rest_timer.Start(m_pp.RestTimer * 1000);
	}

	if (RuleB(NPC, PetZoneWithOwner)) {
		/* Load Pet */
		database.LoadPetInfo(this);
		if (m_petinfo.SpellID > 1 && !GetPet() && m_petinfo.SpellID <= SPDAT_RECORDS) {
			MakePoweredPet(m_petinfo.SpellID, spells[m_petinfo.SpellID].teleport_zone, m_petinfo.petpower,
						   m_petinfo.Name, m_petinfo.size);
			if (GetPet() && GetPet()->IsNPC()) {
				NPC *pet = GetPet()->CastToNPC();
				pet->SetPetState(m_petinfo.Buffs, m_petinfo.Items);
				pet->CalcBonuses();
				pet->SetHP(m_petinfo.HP);
				pet->SetMana(m_petinfo.Mana);

				// Taunt persists when zoning on newer clients, overwrite default.
				if (m_ClientVersionBit & EQ::versions::maskUFAndLater) {
					if (!ingame) {
						pet->SetTaunting(m_petinfo.taunting);
					}
				}
			}
			m_petinfo.SpellID = 0;
		}
	}
	/* Moved here so it's after where we load the pet data. */
	if (!aabonuses.ZoneSuspendMinion && !spellbonuses.ZoneSuspendMinion && !itembonuses.ZoneSuspendMinion) {
		memset(&m_suspendedminion, 0, sizeof(PetInfo));
	}

	/* Server Zone Entry Packet */
	outapp = new EQApplicationPacket(OP_ZoneEntry, sizeof(ServerZoneEntry_Struct));
	ServerZoneEntry_Struct* sze = (ServerZoneEntry_Struct*)outapp->pBuffer;

	FillSpawnStruct(&sze->player, CastToMob());
	sze->player.spawn.curHp = 1;
	sze->player.spawn.NPC = 0;
	sze->player.spawn.z += 6;	//arbitrary lift, seems to help spawning under zone.
	outapp->priority = 6;
	FastQueuePacket(&outapp);

	/* Zone Spawns Packet */
	entity_list.SendZoneSpawnsBulk(this);
	entity_list.SendZoneCorpsesBulk(this);
	entity_list.SendZonePVPUpdates(this);	//hack until spawn struct is fixed.

											/* Time of Day packet */
	outapp = new EQApplicationPacket(OP_TimeOfDay, sizeof(TimeOfDay_Struct));
	TimeOfDay_Struct* tod = (TimeOfDay_Struct*)outapp->pBuffer;
	zone->zone_time.GetCurrentEQTimeOfDay(time(0), tod);
	outapp->priority = 6;
	FastQueuePacket(&outapp);

	/* Tribute Packets */
	DoTributeUpdate();
	if (m_pp.tribute_active) {
		//restart the tribute timer where we left off
		tribute_timer.Start(m_pp.tribute_time_remaining);
	}

	/*
	Character Inventory Packet
	this is not quite where live sends inventory, they do it after tribute
	*/
	if (loaditems) { /* Don't load if a length error occurs */
		if (m_pp.gm) {
			m_inv.SetGMInventory(true); // set to true to allow expansion-restricted packets through
		}

		BulkSendInventoryItems();
		/* Send stuff on the cursor which isn't sent in bulk */
		for (auto iter = m_inv.cursor_cbegin(); iter != m_inv.cursor_cend(); ++iter) {
			/* First item cursor is sent in bulk inventory packet */
			if (iter == m_inv.cursor_cbegin())
				continue;
			const EQ::ItemInstance *inst = *iter;
			SendItemPacket(EQ::invslot::slotCursor, inst, ItemPacketLimbo);
		}

		// this is kinda hackish atm..this process needs to be realigned to allow a contiguous flow
		m_inv.SetGMInventory((bool)m_pp.gm); // reset back to current gm state
	}

	ApplyWeaponsStance();

	auto dynamic_zone_member_entries = DynamicZoneMembersRepository::GetWhere(database,
		fmt::format("character_id = {}", CharacterID()));

	for (const auto& entry : dynamic_zone_member_entries)
	{
		m_dynamic_zone_ids.emplace_back(entry.dynamic_zone_id);
	}

	auto dz = zone->GetDynamicZone();
	if (dz && dz->GetSafeReturnLocation().zone_id != 0)
	{
		auto& safereturn = dz->GetSafeReturnLocation();

		auto safereturn_entry = CharacterInstanceSafereturnsRepository::NewEntity();
		safereturn_entry.character_id     = CharacterID();
		safereturn_entry.instance_zone_id = zone->GetZoneID();
		safereturn_entry.instance_id      = zone->GetInstanceID();
		safereturn_entry.safe_zone_id     = safereturn.zone_id;
		safereturn_entry.safe_x           = safereturn.x;
		safereturn_entry.safe_y           = safereturn.y;
		safereturn_entry.safe_z           = safereturn.z;
		safereturn_entry.safe_heading     = safereturn.heading;

		CharacterInstanceSafereturnsRepository::InsertOneOrUpdate(database, safereturn_entry);
	}
	else
	{
		CharacterInstanceSafereturnsRepository::DeleteWhere(database,
			fmt::format("character_id = {}", character_id));
	}

	/**
	 * DevTools Load Settings
	 */
	if (Admin() >= EQ::DevTools::GM_ACCOUNT_STATUS_LEVEL) {
		const auto dev_tools_key = fmt::format("{}-dev-tools-disabled", AccountID());
		if (DataBucket::GetData(&database, dev_tools_key) == "true") {
			dev_tools_enabled = false;
		}
	}

	if (m_ClientVersionBit & EQ::versions::maskUFAndLater) {
		outapp = new EQApplicationPacket(OP_XTargetResponse, 8);
		outapp->WriteUInt32(GetMaxXTargets());
		outapp->WriteUInt32(0);
		FastQueuePacket(&outapp);
	}

	/*
	Weather Packet
	This shouldn't be moved, this seems to be what the client
	uses to advance to the next state (sending ReqNewZone)
	*/
	outapp = new EQApplicationPacket(OP_Weather, 12);
	Weather_Struct *ws = (Weather_Struct *)outapp->pBuffer;
	ws->val1 = 0x000000FF;

	if (zone->zone_weather == EQ::constants::WeatherTypes::Raining) {
		ws->type = 0x31;
	} else if (zone->zone_weather == EQ::constants::WeatherTypes::Snowing) {
		outapp->pBuffer[8] = 0x01;
		ws->type = EQ::constants::WeatherTypes::Snowing;
	}

	outapp->priority = 6;
	QueuePacket(outapp);
	safe_delete(outapp);

	if (ClientVersion() >= EQ::versions::ClientVersion::RoF) {
		Handle_Connect_OP_ReqNewZone(nullptr);
	}

	SetAttackTimer();
	conn_state = ZoneInfoSent;
	zoneinpacket_timer.Start();
	return;
}

// connected opcode handlers
void Client::Handle_0x0193(const EQApplicationPacket *app)
{
	// Not sure what this opcode does. It started being sent when OP_ClientUpdate was
	// changed to pump OP_ClientUpdate back out instead of OP_MobUpdate
	// 2 bytes: 00 00

	return;
}

void Client::Handle_OP_AAAction(const EQApplicationPacket *app)
{
	LogAA("Received OP_AAAction");

	if (app->size != sizeof(AA_Action)) {
		LogAA("Error! OP_AAAction size didnt match!");
		return;
	}
	AA_Action* action = (AA_Action*)app->pBuffer;

	if (action->action == aaActionActivate) {//AA Hotkey
		LogAA("Activating AA [{}]", action->ability);
		ActivateAlternateAdvancementAbility(action->ability, action->target_id);
	}
	else if (action->action == aaActionBuy) {
		PurchaseAlternateAdvancementRank(action->ability);
	}
	else if (action->action == aaActionDisableEXP) { //Turn Off AA Exp
		if (m_epp.perAA > 0) {
			MessageString(Chat::White, AA_OFF);
		}

		m_epp.perAA = 0;
		SendAlternateAdvancementStats();
	}
	else if (action->action == aaActionSetEXP) {
		if (m_epp.perAA == 0)
			MessageString(Chat::White, AA_ON);
		m_epp.perAA = action->exp_value;
		if (m_epp.perAA < 0 || m_epp.perAA > 100)
			m_epp.perAA = 0;	// stop exploit with sanity check

								// send an update
		SendAlternateAdvancementStats();
		SendAlternateAdvancementTable();
	}
	else {
		LogAA("Unknown AA action : [{}] [{}] [{}] [{}]", action->action, action->ability, action->target_id, action->exp_value);
	}
}

void Client::Handle_OP_AcceptNewTask(const EQApplicationPacket *app)
{

	if (app->size != sizeof(AcceptNewTask_Struct)) {
		LogDebug("Size mismatch in OP_AcceptNewTask expected [{}] got [{}]", sizeof(AcceptNewTask_Struct), app->size);
		DumpPacket(app);
		return;
	}
	AcceptNewTask_Struct *ant = (AcceptNewTask_Struct*)app->pBuffer;

	if (ant->task_id > 0 && RuleB(TaskSystem, EnableTaskSystem) && task_state)
	{
		if (task_state->CanAcceptNewTask(this, ant->task_id, ant->task_master_id))
		{
			task_state->AcceptNewTask(this, ant->task_id, ant->task_master_id, std::time(nullptr));
		}
		task_state->ClearLastOffers();
	}
}

void Client::Handle_OP_AdventureInfoRequest(const EQApplicationPacket *app)
{
	if (app->size < sizeof(EntityId_Struct))
	{
		LogError("Handle_OP_AdventureInfoRequest had a packet that was too small");
		return;
	}
	EntityId_Struct* ent = (EntityId_Struct*)app->pBuffer;
	Mob * m = entity_list.GetMob(ent->entity_id);
	if (m && m->IsNPC())
	{
		std::map<uint32, std::string>::iterator it;
		it = zone->adventure_entry_list_flavor.find(m->CastToNPC()->GetAdventureTemplate());
		if (it != zone->adventure_entry_list_flavor.end())
		{
			auto outapp = new EQApplicationPacket(OP_AdventureInfo, (it->second.size() + 2));
			strn0cpy((char*)outapp->pBuffer, it->second.c_str(), it->second.size());
			FastQueuePacket(&outapp);
		}
		else
		{
			if (m->CastToNPC()->GetAdventureTemplate() != 0)
			{
				std::string text = "Choose your difficulty and preferred adventure type.";
				auto outapp = new EQApplicationPacket(OP_AdventureInfo, (text.size() + 2));
				strn0cpy((char*)outapp->pBuffer, text.c_str(), text.size());
				FastQueuePacket(&outapp);
			}
		}
	}
}

void Client::Handle_OP_AdventureLeaderboardRequest(const EQApplicationPacket *app)
{
	if (app->size < sizeof(AdventureLeaderboardRequest_Struct))
	{
		return;
	}

	if (adventure_leaderboard_timer)
	{
		return;
	}

	adventure_leaderboard_timer = new Timer(4000);
	auto pack = new ServerPacket(ServerOP_AdventureLeaderboard, sizeof(ServerLeaderboardRequest_Struct));
	ServerLeaderboardRequest_Struct *lr = (ServerLeaderboardRequest_Struct*)pack->pBuffer;
	strcpy(lr->player, GetName());

	AdventureLeaderboardRequest_Struct *lrs = (AdventureLeaderboardRequest_Struct*)app->pBuffer;
	lr->type = 1 + (lrs->theme * 2) + lrs->type;
	worldserver.SendPacket(pack);
	delete pack;
}

void Client::Handle_OP_AdventureMerchantPurchase(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Adventure_Purchase_Struct))
	{
		LogError("OP size error: OP_AdventureMerchantPurchase expected:[{}] got:[{}]", sizeof(Adventure_Purchase_Struct), app->size);
		return;
	}

	Adventure_Purchase_Struct* aps = (Adventure_Purchase_Struct*)app->pBuffer;
	uint32 merchantid = 0;
	Mob* tmp = entity_list.GetMob(aps->npcid);
	if (tmp == 0 || !tmp->IsNPC() || ((tmp->GetClass() != Class::AdventureMerchant) &&
		(tmp->GetClass() != Class::DiscordMerchant) && (tmp->GetClass() != Class::NorrathsKeepersMerchant) && (tmp->GetClass() != Class::DarkReignMerchant)))
		return;

	//you have to be somewhat close to them to be properly using them
	if (DistanceSquared(m_Position, tmp->GetPosition()) > USE_NPC_RANGE2)
		return;

	merchantid = tmp->CastToNPC()->MerchantType;

	const EQ::ItemData* item = nullptr;
	bool found = false;
	std::list<MerchantList> merchantlists = zone->merchanttable[merchantid];
	for (auto merchantlist : merchantlists) {
		if (GetLevel() < merchantlist.level_required) {
			continue;
		}

		int faction_id = tmp->GetPrimaryFaction();
		if (faction_id != 0 && GetModCharacterFactionLevel(faction_id) < merchantlist.faction_required) {
			continue;
		}

		item = database.GetItem(merchantlist.item);
		if (!item) {
			continue;
		}

		if (item->ID == aps->itemid) { //This check to make sure that the item is actually on the NPC, people attempt to inject packets to get items summoned...
			found = true;
			break;
		}
	}

	if (!item || !found) {
		Message(Chat::Red, "Error: The item you purchased does not exist!");
		return;
	}

	auto item_cost = item->LDoNPrice;
	bool cannot_afford = false;
	std::string merchant_type;
	if (item_cost < 0) {
		Message(Chat::Red, "This item cannot be bought.");
		return;
	}

	if (aps->Type == LDoNMerchant) {
		if (m_pp.ldon_points_available < item_cost) {
			Message(Chat::Red, "You cannot afford that item.");
			return;
		}

		if (item->LDoNTheme <= LDoNTheme::TAKBit) {
			uint32 ldon_theme;
			if (item->LDoNTheme & LDoNTheme::TAKBit) {
				if (m_pp.ldon_points_tak < item_cost) {
					cannot_afford = true;
					ldon_theme = LDoNTheme::TAK;
				}
			} else if (item->LDoNTheme & LDoNTheme::RUJBit) {
				if (m_pp.ldon_points_ruj < item_cost) {
					cannot_afford = true;
					ldon_theme = LDoNTheme::RUJ;
				}
			} else if (item->LDoNTheme & LDoNTheme::MMCBit) {
				if (m_pp.ldon_points_mmc < item_cost) {
					cannot_afford = true;
					ldon_theme = LDoNTheme::MMC;
				}
			} else if (item->LDoNTheme & LDoNTheme::MIRBit) {
				if (m_pp.ldon_points_mir < item_cost) {
					cannot_afford = true;
					ldon_theme = LDoNTheme::MIR;
				}
			} else if (item->LDoNTheme & LDoNTheme::GUKBit) {
				if (m_pp.ldon_points_guk < item_cost) {
					cannot_afford = true;
					ldon_theme = LDoNTheme::GUK;
				}
			}

			merchant_type = fmt::format(
				"{} Point{}",
				LDoNTheme::GetName(ldon_theme),
				item_cost != 1 ? "s" : ""
			);
		}
	} else if (aps->Type == DiscordMerchant) {
		if (GetPVPPoints() < item_cost) {
			cannot_afford = true;
			merchant_type = fmt::format(
				"PVP Point{}",
				item_cost != 1 ? "s" : ""
			);
		}
	} else if (aps->Type == NorrathsKeepersMerchant) {
		if (GetRadiantCrystals() < item_cost) {
			cannot_afford = true;
			merchant_type = database.CreateItemLink(RuleI(Zone, RadiantCrystalItemID));
		}
	} else if (aps->Type == DarkReignMerchant) {
		if (GetEbonCrystals() < item_cost) {
			cannot_afford = true;
			merchant_type = database.CreateItemLink(RuleI(Zone, EbonCrystalItemID));
		}
	} else {
		Message(Chat::Red, "Unknown Adventure Merchant Type.");
		return;
	}

	if (cannot_afford && !merchant_type.empty()) {
		Message(
			Chat::Red,
			fmt::format(
				"You need at least {} {} to purchase this item.",
				item_cost,
				merchant_type
			).c_str()
		);
	}


	if (CheckLoreConflict(item)) {
		Message(
			Chat::Yellow,
			fmt::format(
				"You already have a lore {} ({}) in your inventory.",
				database.CreateItemLink(item->ID),
				item->ID
			).c_str()
		);
		return;
	}

	if (aps->Type == LDoNMerchant) {
		int required_points = item_cost * -1;

		if (!UpdateLDoNPoints(6, required_points)) {
			return;
		}
	}
	else if (aps->Type == DiscordMerchant)
	{
		SetPVPPoints(GetPVPPoints() - item_cost);
		SendPVPStats();
	}
	else if (aps->Type == NorrathsKeepersMerchant)
	{
		SetRadiantCrystals(GetRadiantCrystals() - item_cost);
	}
	else if (aps->Type == DarkReignMerchant)
	{
		SetEbonCrystals(GetEbonCrystals() - (int32)item->LDoNPrice);
	}
	int16 charges = 1;
	if (item->MaxCharges != 0)
		charges = item->MaxCharges;

	CheckItemDiscoverability(item->ID);

	EQ::ItemInstance *inst = database.CreateItem(item, charges);
	if (!AutoPutLootInInventory(*inst, true, true))
	{
		PutLootInInventory(EQ::invslot::slotCursor, *inst);
	}
	safe_delete(inst);
	Save(1);
}

void Client::Handle_OP_AdventureMerchantRequest(const EQApplicationPacket *app)
{
	if (app->size != sizeof(AdventureMerchant_Struct))
	{
		LogError("OP size error: OP_AdventureMerchantRequest expected:[{}] got:[{}]", sizeof(AdventureMerchant_Struct), app->size);
		return;
	}
	std::stringstream ss(std::stringstream::in | std::stringstream::out);

	uint8 count = 0;
	AdventureMerchant_Struct* eid = (AdventureMerchant_Struct*)app->pBuffer;
	uint32 merchantid = 0;

	Mob* tmp = entity_list.GetMob(eid->entity_id);
	if (tmp == 0 || !tmp->IsNPC() || ((tmp->GetClass() != Class::AdventureMerchant) &&
		(tmp->GetClass() != Class::DiscordMerchant) && (tmp->GetClass() != Class::NorrathsKeepersMerchant) && (tmp->GetClass() != Class::DarkReignMerchant)))
		return;

	if (!tmp->CastToNPC()->IsMerchantOpen()) {
		tmp->SayString(zone->random.Int(MERCHANT_CLOSED_ONE, MERCHANT_CLOSED_THREE));
		return;
	}

	//you have to be somewhat close to them to be properly using them
	if (DistanceSquared(m_Position, tmp->GetPosition()) > USE_NPC_RANGE2)
		return;

	merchantid = tmp->CastToNPC()->MerchantType;

	const EQ::ItemData *item = nullptr;
	std::list<MerchantList> merlist = zone->merchanttable[merchantid];
	std::list<MerchantList>::const_iterator itr;
	for (itr = merlist.begin(); itr != merlist.end(); ++itr) {
		const MerchantList &ml = *itr;
		if (GetLevel() < ml.level_required) {
			continue;
		}

		int32 fac = tmp->GetPrimaryFaction();
		if (fac != 0 && GetModCharacterFactionLevel(fac) < ml.faction_required) {
			continue;
		}

		item = database.GetItem(ml.item);
		if (item) {
			uint32 theme = LDoNTheme::Unused;
			if (item->LDoNTheme > LDoNTheme::TAKBit) {
				theme = LDoNTheme::Unused;
			} else if (item->LDoNTheme & LDoNTheme::TAKBit) {
				theme = LDoNTheme::TAK;
			} else if (item->LDoNTheme & LDoNTheme::RUJBit) {
				theme = LDoNTheme::RUJ;
			} else if (item->LDoNTheme & LDoNTheme::MMCBit) {
				theme = LDoNTheme::MMC;
			} else if (item->LDoNTheme & LDoNTheme::MIRBit) {
				theme = LDoNTheme::MIR;
			} else if (item->LDoNTheme & LDoNTheme::GUKBit) {
				theme = LDoNTheme::GUK;
			}
			ss << "^" << item->Name << "|";
			ss << item->ID << "|";
			ss << item->LDoNPrice << "|";
			ss << theme << "|";
			ss << (item->Stackable ? 1 : 0) << "|";
			ss << (item->LoreFlag ? 1 : 0) << "|";
			ss << item->Races << "|";
			ss << item->Classes;
			count++;
		}
	}
	//Count
	//^Item Name,Item ID,Cost in Points,Theme (0=none),0,1,races bit map,classes bitmap
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_AdventureMerchantResponse, ss.str().size() + 2);
	outapp->pBuffer[0] = count;
	strn0cpy((char*)&outapp->pBuffer[1], ss.str().c_str(), ss.str().size());
	FastQueuePacket(&outapp);
}

void Client::Handle_OP_AdventureMerchantSell(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Adventure_Sell_Struct))
	{
		LogDebug("Size mismatch on OP_AdventureMerchantSell: got [{}] expected [{}]", app->size, sizeof(Adventure_Sell_Struct));
		DumpPacket(app);
		return;
	}

	Adventure_Sell_Struct *ams_in = (Adventure_Sell_Struct*)app->pBuffer;

	Mob* vendor = entity_list.GetMob(ams_in->npcid);
	if (vendor == 0 || !vendor->IsNPC() || ((vendor->GetClass() != Class::AdventureMerchant) &&
		(vendor->GetClass() != Class::NorrathsKeepersMerchant) && (vendor->GetClass() != Class::DarkReignMerchant)))
	{
		Message(Chat::Red, "Vendor was not found.");
		return;
	}

	if (DistanceSquared(m_Position, vendor->GetPosition())  > USE_NPC_RANGE2)
	{
		Message(Chat::Red, "Vendor is out of range.");
		return;
	}

	uint32 itemid = GetItemIDAt(ams_in->slot);

	if (itemid == 0)
	{
		Message(Chat::Red, "Found no item at that slot.");
		return;
	}

	const EQ::ItemData* item = database.GetItem(itemid);
	EQ::ItemInstance* inst = GetInv().GetItem(ams_in->slot);
	if (!item || !inst) {
		Message(Chat::Red, "You seemed to have misplaced that item...");
		return;
	}

	// Note that Lucy has ldonsold values of 4 and 5 for items sold by Norrath's Keepers and Dark Reign, whereas 13th Floor
	// has ldonsold = 0 for these items, so some manual editing of the items DB will be required to support sell back of the
	// items.
	//
	// The Merchant seems to have some other way of knowing whether he will accept the item, other than the ldonsold field,
	// e.g. if you summon items 76036 and 76053 (good and evil versions of Spell: Ward Of Vengeance), if you are interacting
	// with a Norrath's Keeper merchant and click on 76036 in your inventory, he says he will give you radiant crystals for
	// it, but he will refuse for item 76053.
	//
	// Similarly, just giving a cloth cap an ldonsold value of 4 will not make the Merchant buy it.
	//
	// Note that the the Client will not allow you to sell anything back to a Discord merchant, so there is no need to handle
	// that case here.
	if (item->LDoNSold == 0)
	{
		Message(Chat::Red, "The merchant does not want that item.");
		return;
	}

	if (item->LDoNPrice == 0)
	{
		Message(Chat::Red, "The merchant does not want that item.");
		return;
	}

	// 06/11/2016 This formula matches RoF2 client side calculation.
	uint32 price = EQ::Clamp(
		price,
		EQ::ClampUpper(
			(item->LDoNPrice + 1) * item->LDoNSellBackRate / 100,
			item->LDoNPrice
		),
		item->LDoNPrice
	);

	if (price == 0)
	{
		Message(Chat::Red, "The merchant does not want that item.");
		return;
	}

	if (!inst->IsStackable())
	{
		DeleteItemInInventory(ams_in->slot);
	}
	else
	{
		if (inst->GetCharges() < ams_in->charges)
		{
			ams_in->charges = inst->GetCharges();
		}

		if (ams_in->charges == 0)
		{
			Message(Chat::Red, "Charge mismatch error.");
			return;
		}

		DeleteItemInInventory(ams_in->slot, ams_in->charges);
		price *= ams_in->charges;
	}

	auto outapp = new EQApplicationPacket(OP_AdventureMerchantSell, sizeof(Adventure_Sell_Struct));
	Adventure_Sell_Struct *ams = (Adventure_Sell_Struct*)outapp->pBuffer;
	ams->slot = ams_in->slot;
	ams->unknown000 = 1;
	ams->npcid = ams->npcid;
	ams->charges = ams_in->charges;
	ams->sell_price = price;
	FastQueuePacket(&outapp);

	switch (vendor->GetClass())
	{
	case Class::AdventureMerchant:
	{
		UpdateLDoNPoints(6, price);
		break;
	}
	case Class::NorrathsKeepersMerchant:
	{
		SetRadiantCrystals(GetRadiantCrystals() + price);
		break;
	}
	case Class::DarkReignMerchant:
	{
		SetEbonCrystals(GetEbonCrystals() + price);
		break;
	}

	default:
		break;
	}

	Save(1);
}

void Client::Handle_OP_AdventureRequest(const EQApplicationPacket *app)
{
	if (app->size < sizeof(AdventureRequest_Struct))
	{
		LogError("Handle_OP_AdventureRequest had a packet that was too small");
		return;
	}

	if (IsOnAdventure())
	{
		return;
	}

	if (!p_timers.Expired(&database, pTimerStartAdventureTimer, false))
	{
		return;
	}

	if (GetPendingAdventureRequest())
	{
		return;
	}

	AdventureRequest_Struct* ars = (AdventureRequest_Struct*)app->pBuffer;
	uint8 group_members = 0;
	Raid *r = nullptr;
	Group *g = nullptr;

	if (IsRaidGrouped())
	{
		r = GetRaid();
		group_members = r->RaidCount();
	}
	else if (IsGrouped())
	{
		g = GetGroup();
		group_members = g->GroupCount();
	}
	else
	{
		return;
	}

	if (group_members < RuleI(Adventure, MinNumberForGroup) || group_members > RuleI(Adventure, MaxNumberForGroup))
	{
		return;
	}

	Mob* m = entity_list.GetMob(ars->entity_id);
	uint32 template_id = 0;
	if (m && m->IsNPC())
	{
		template_id = m->CastToNPC()->GetAdventureTemplate();
	}
	else
	{
		return;
	}

	auto packet =
		new ServerPacket(ServerOP_AdventureRequest, sizeof(ServerAdventureRequest_Struct) + (64 * group_members));
	ServerAdventureRequest_Struct *sar = (ServerAdventureRequest_Struct*)packet->pBuffer;
	sar->member_count = group_members;
	sar->risk = ars->risk;
	sar->type = ars->type;
	sar->template_id = template_id;
	strcpy(sar->leader, GetName());

	if (IsRaidGrouped())
	{
		int i = 0;
		for (int x = 0; x < MAX_RAID_MEMBERS; ++x)
		{
			if (i == group_members)
			{
				break;
			}

			const char *c_name = nullptr;
			c_name = r->GetClientNameByIndex(x);
			if (c_name)
			{
				memcpy((packet->pBuffer + sizeof(ServerAdventureRequest_Struct) + (64 * i)), c_name, strlen(c_name));
				++i;
			}
		}
	}
	else
	{
		int i = 0;
		for (int x = 0; x < MAX_GROUP_MEMBERS; ++x)
		{
			if (i == group_members)
			{
				break;
			}

			const char *c_name = nullptr;
			c_name = g->GetClientNameByIndex(x);
			if (c_name)
			{
				memcpy((packet->pBuffer + sizeof(ServerAdventureRequest_Struct) + (64 * i)), c_name, strlen(c_name));
				++i;
			}
		}
	}

	worldserver.SendPacket(packet);
	delete packet;
	p_timers.Start(pTimerStartAdventureTimer, 5);
}

void Client::Handle_OP_AdventureStatsRequest(const EQApplicationPacket *app)
{
	if (adventure_stats_timer)
	{
		return;
	}

	adventure_stats_timer = new Timer(8000);
	auto outapp = new EQApplicationPacket(OP_AdventureStatsReply, sizeof(AdventureStats_Struct));
	AdventureStats_Struct *as = (AdventureStats_Struct*)outapp->pBuffer;

	if (database.GetAdventureStats(CharacterID(), as))
	{
		m_pp.ldon_wins_guk = as->success.guk;
		m_pp.ldon_wins_mir = as->success.mir;
		m_pp.ldon_wins_mmc = as->success.mmc;
		m_pp.ldon_wins_ruj = as->success.ruj;
		m_pp.ldon_wins_tak = as->success.tak;
		m_pp.ldon_losses_guk = as->failure.guk;
		m_pp.ldon_losses_mir = as->failure.mir;
		m_pp.ldon_losses_mmc = as->failure.mmc;
		m_pp.ldon_losses_ruj = as->failure.ruj;
		m_pp.ldon_losses_tak = as->failure.tak;
	}

	FastQueuePacket(&outapp);
}

void Client::Handle_OP_AggroMeterLockTarget(const EQApplicationPacket *app)
{
	if (app->size < sizeof(uint32)) {
		LogError("Handle_OP_AggroMeterLockTarget had a packet that was too small");
		return;
	}

	SetAggroMeterLock(app->ReadUInt32(0));
	ProcessAggroMeter();
}

void Client::Handle_OP_AltCurrencyMerchantRequest(const EQApplicationPacket *app)
{
	VERIFY_PACKET_LENGTH(OP_AltCurrencyMerchantRequest, app, uint32);

	auto target = entity_list.GetNPCByID(*((uint32*)app->pBuffer));
	if (target) {
		if (DistanceSquared(m_Position, target->GetPosition())  > USE_NPC_RANGE2) {
			return;
		}

		if (target->GetClass() != Class::AlternateCurrencyMerchant) {
			return;
		}

		uint32 currency_id = target->GetAltCurrencyType();
		if (!currency_id) {
			return;
		}

		auto currency_item_id = zone->GetCurrencyItemID(currency_id);
		if (!currency_item_id) {
			return;
		}

		std::stringstream ss(std::stringstream::in | std::stringstream::out);
		std::stringstream item_ss(std::stringstream::in | std::stringstream::out);
		ss << currency_id << "|1|" << currency_id;
		uint32 count = 0;
		uint32 merchant_id = target->MerchantType;
		const EQ::ItemData *item = nullptr;

		std::list<MerchantList> merlist = zone->merchanttable[merchant_id];
		std::list<MerchantList>::const_iterator itr;
		for (itr = merlist.begin(); itr != merlist.end() && count < 255; ++itr) {
			const MerchantList &ml = *itr;
			if (GetLevel() < ml.level_required) {
				continue;
			}

			auto faction_id = target->GetPrimaryFaction();
			if (
				faction_id &&
				GetModCharacterFactionLevel(faction_id) < ml.faction_required
			) {
				continue;
			}

			item = database.GetItem(ml.item);
			if (item) {
				item_ss << "^" << item->Name << "|";
				item_ss << item->ID << "|";
				item_ss << ml.alt_currency_cost << "|";
				item_ss << "0|";
				item_ss << "1|";
				item_ss << item->Races << "|";
				item_ss << item->Classes;
				count++;
			}
		}

		if (count > 0) {
			ss << "|" << count << item_ss.str();
		} else {
			ss << "|0";
		}

		EQApplicationPacket* outapp = new EQApplicationPacket(OP_AltCurrencyMerchantReply, ss.str().length() + 1);
		memcpy(outapp->pBuffer, ss.str().c_str(), ss.str().length());
		FastQueuePacket(&outapp);
	}
}

void Client::Handle_OP_AltCurrencyPurchase(const EQApplicationPacket *app)
{
	VERIFY_PACKET_LENGTH(OP_AltCurrencyPurchase, app, AltCurrencyPurchaseItem_Struct);
	AltCurrencyPurchaseItem_Struct *purchase = (AltCurrencyPurchaseItem_Struct*)app->pBuffer;
	NPC* tar = entity_list.GetNPCByID(purchase->merchant_entity_id);
	if (tar) {
		if (DistanceSquared(m_Position, tar->GetPosition())> USE_NPC_RANGE2)
			return;

		if (tar->GetClass() != Class::AlternateCurrencyMerchant) {
			return;
		}

		uint32 alt_cur_id = tar->GetAltCurrencyType();
		if (alt_cur_id == 0) {
			return;
		}

		const EQ::ItemData* item = nullptr;
		uint32 cost = 0;
		uint32 current_currency = GetAlternateCurrencyValue(alt_cur_id);
		uint32 merchant_id = tar->MerchantType;
		bool found = false;
		std::list<MerchantList> merlist = zone->merchanttable[merchant_id];
		std::list<MerchantList>::const_iterator itr;
		for (itr = merlist.begin(); itr != merlist.end(); ++itr) {
			MerchantList ml = *itr;
			if (GetLevel() < ml.level_required) {
				continue;
			}

			int32 fac = tar->GetPrimaryFaction();
			if (fac != 0 && GetModCharacterFactionLevel(fac) < ml.faction_required) {
				continue;
			}

			item = database.GetItem(ml.item);
			if (!item)
				continue;

			if (item->ID == purchase->item_id) { //This check to make sure that the item is actually on the NPC, people attempt to inject packets to get items summoned...
				cost = ml.alt_currency_cost;
				found = true;
				break;
			}
		}

		if (!item || !found) {
			Message(Chat::Red, "Error: The item you purchased does not exist!");
			return;
		}

		if (cost > current_currency) {
			Message(Chat::Red, "You cannot afford that item right now.");
			return;
		}

		if (CheckLoreConflict(item))
		{
			Message(Chat::Yellow, "You can only have one of a lore item.");
			return;
		}

		if (parse->PlayerHasQuestSub(EVENT_ALT_CURRENCY_MERCHANT_BUY)) {
			const auto& export_string = fmt::format(
				"{} {} {} {} {}",
				alt_cur_id,
				tar->GetNPCTypeID(),
				tar->MerchantType,
				item->ID,
				cost
			);

			parse->EventPlayer(EVENT_ALT_CURRENCY_MERCHANT_BUY, this, export_string, 0);
		}

		uint64 current_balance = AddAlternateCurrencyValue(alt_cur_id, -((int) cost));
		int16  charges         = 1;
		if (item->MaxCharges != 0) {
			charges = item->MaxCharges;
		}

		if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::MERCHANT_PURCHASE)) {
			auto e = PlayerEvent::MerchantPurchaseEvent{
				.npc_id = tar->GetNPCTypeID(),
				.merchant_name = tar->GetCleanName(),
				.merchant_type = tar->MerchantType,
				.item_id = item->ID,
				.item_name = item->Name,
				.charges = charges,
				.cost = cost,
				.alternate_currency_id = alt_cur_id,
				.player_money_balance = GetCarriedMoney(),
				.player_currency_balance = current_balance,
			};

			RecordPlayerEventLog(PlayerEvent::MERCHANT_PURCHASE, e);
		}

		CheckItemDiscoverability(item->ID);

		EQ::ItemInstance *inst = database.CreateItem(item, charges);
		if (!AutoPutLootInInventory(*inst, true, true))
		{
			PutLootInInventory(EQ::invslot::slotCursor, *inst);
		}
		safe_delete(inst);

		Save(1);
	}
}

void Client::Handle_OP_AltCurrencyReclaim(const EQApplicationPacket *app)
{
	VERIFY_PACKET_LENGTH(OP_AltCurrencyReclaim, app, AltCurrencyReclaim_Struct);
	AltCurrencyReclaim_Struct *reclaim = (AltCurrencyReclaim_Struct*)app->pBuffer;
	uint32 item_id = zone->GetCurrencyItemID(reclaim->currency_id);

	if (!item_id) {
		return;
	}

	if (IsTrader()) {
		TraderEndTrader();
	}

	if (IsBuyer()) {
		ToggleBuyerMode(false);
	}

	/* Item to Currency Storage */
	if (reclaim->reclaim_flag == 1) {
		uint32 removed = NukeItem(item_id, invWhereWorn | invWherePersonal | invWhereCursor);
		if (removed > 0) {
			AddAlternateCurrencyValue(reclaim->currency_id, removed);
		}
	}
	/* Cursor to Item storage */
	else {
		uint32 max_currency = GetAlternateCurrencyValue(reclaim->currency_id);

		if (max_currency == 0 || reclaim->count == 0)
			return;

		/* If you input more than you have currency wise, just give the max of the currency you currently have */
		if (reclaim->count > max_currency) {
			SummonItem(item_id, max_currency);
			SetAlternateCurrencyValue(reclaim->currency_id, 0);
		}
		else {
			SummonItem(item_id, reclaim->count, 0, 0, 0, 0, 0, 0, false, EQ::invslot::slotCursor);
			AddAlternateCurrencyValue(reclaim->currency_id, -((int)reclaim->count));
		}
	}
}

void Client::Handle_OP_AltCurrencySell(const EQApplicationPacket *app)
{
	VERIFY_PACKET_LENGTH(OP_AltCurrencySell, app, AltCurrencySellItem_Struct);
	EQApplicationPacket *outapp = app->Copy();
	AltCurrencySellItem_Struct *sell = (AltCurrencySellItem_Struct*)outapp->pBuffer;

	NPC* tar = entity_list.GetNPCByID(sell->merchant_entity_id);
	if (tar) {
		if (DistanceSquared(m_Position, tar->GetPosition()) > USE_NPC_RANGE2)
			return;

		if (tar->GetClass() != Class::AlternateCurrencyMerchant) {
			return;
		}

		uint32 alt_cur_id = tar->GetAltCurrencyType();
		if (alt_cur_id == 0) {
			return;
		}

		EQ::ItemInstance* inst = GetInv().GetItem(sell->slot_id);
		if (!inst) {
			return;
		}

		if (!RuleB(Merchant, EnableAltCurrencySell)) {
			Message(Chat::Red, "Selling alternate currency items is disabled.");
			return;
		}

		const EQ::ItemData* item = nullptr;
		uint32 cost = 0;
		uint32 current_currency = GetAlternateCurrencyValue(alt_cur_id);
		uint32 merchant_id = tar->MerchantType;
		uint32 npc_id = tar->GetNPCTypeID();
		bool found = false;
		std::list<MerchantList> merlist = zone->merchanttable[merchant_id];
		std::list<MerchantList>::const_iterator itr;
		for (itr = merlist.begin(); itr != merlist.end(); ++itr) {
			MerchantList ml = *itr;
			if (GetLevel() < ml.level_required) {
				continue;
			}

			int32 fac = tar->GetPrimaryFaction();
			if (fac != 0 && GetModCharacterFactionLevel(fac) < ml.faction_required) {
				continue;
			}

			item = database.GetItem(ml.item);
			if (!item)
				continue;

			if (item->ID == inst->GetItem()->ID) {
				// 06/11/2016 This formula matches RoF2 client side calculation.
				cost = EQ::Clamp(
					cost,
					EQ::ClampUpper(
						static_cast<uint32>((ml.alt_currency_cost + 1) * item->LDoNSellBackRate / 100),
						static_cast<uint32>(ml.alt_currency_cost)
					),
					static_cast<uint32>(ml.alt_currency_cost)
				);
				found = true;
				break;
			}
		}

		if (!found) {
			return;
		}

		if (!inst->IsStackable())
		{
			DeleteItemInInventory(sell->slot_id);
		}
		else
		{
			if (inst->GetCharges() < sell->charges)
			{
				sell->charges = inst->GetCharges();
			}

			if (sell->charges == 0)
			{
				Message(Chat::Red, "Charge mismatch error.");
				return;
			}

			DeleteItemInInventory(sell->slot_id, sell->charges);
			cost *= sell->charges;
		}

		sell->cost = cost;

		if (parse->PlayerHasQuestSub(EVENT_ALT_CURRENCY_MERCHANT_SELL)) {
			const auto& export_string = fmt::format(
				"{} {} {} {} {}",
				alt_cur_id,
				tar->GetNPCTypeID(),
				tar->MerchantType,
				item->ID,
				cost
			);

			parse->EventPlayer(EVENT_ALT_CURRENCY_MERCHANT_SELL, this, export_string, 0);
		}

		FastQueuePacket(&outapp);
		uint64 new_balance = AddAlternateCurrencyValue(alt_cur_id, cost);

		if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::MERCHANT_SELL)) {
			auto e = PlayerEvent::MerchantSellEvent{
				.npc_id = tar->GetNPCTypeID(),
				.merchant_name = tar->GetCleanName(),
				.merchant_type = tar->CastToNPC()->MerchantType,
				.item_id = item->ID,
				.item_name = item->Name,
				.charges = static_cast<int16>(sell->charges),
				.cost = cost,
				.alternate_currency_id = 0,
				.player_money_balance = GetCarriedMoney(),
				.player_currency_balance = new_balance,
			};

			RecordPlayerEventLog(PlayerEvent::MERCHANT_SELL, e);
		}

		Save(1);
	}
}

void Client::Handle_OP_AltCurrencySellSelection(const EQApplicationPacket *app)
{
	VERIFY_PACKET_LENGTH(OP_AltCurrencySellSelection, app, AltCurrencySelectItem_Struct);

	AltCurrencySelectItem_Struct *select = (AltCurrencySelectItem_Struct*)app->pBuffer;
	NPC* tar = entity_list.GetNPCByID(select->merchant_entity_id);
	if (tar) {
		if (DistanceSquared(m_Position, tar->GetPosition()) > USE_NPC_RANGE2)
			return;

		if (tar->GetClass() != Class::AlternateCurrencyMerchant) {
			return;
		}

		uint32 alt_cur_id = tar->GetAltCurrencyType();
		if (alt_cur_id == 0) {
			return;
		}

		EQ::ItemInstance *inst = m_inv.GetItem(select->slot_id);
		if (!inst) {
			return;
		}

		const EQ::ItemData* item = nullptr;
		uint32 cost = 0;
		uint32 current_currency = GetAlternateCurrencyValue(alt_cur_id);
		uint32 merchant_id = tar->MerchantType;

		if (RuleB(Merchant, EnableAltCurrencySell)) {
			bool found = false;
			std::list<MerchantList> merlist = zone->merchanttable[merchant_id];
			std::list<MerchantList>::const_iterator itr;
			for (itr = merlist.begin(); itr != merlist.end(); ++itr) {
				MerchantList ml = *itr;
				if (GetLevel() < ml.level_required) {
					continue;
				}

				int32 fac = tar->GetPrimaryFaction();
				if (fac != 0 && GetModCharacterFactionLevel(fac) < ml.faction_required) {
					continue;
				}

				item = database.GetItem(ml.item);
				if (!item)
					continue;

				if (item->ID == inst->GetItem()->ID) {
					// 06/11/2016 This formula matches RoF2 client side calculation.
					cost = EQ::Clamp(
						cost,
						EQ::ClampUpper(
							static_cast<uint32>((ml.alt_currency_cost + 1) * item->LDoNSellBackRate / 100),
							static_cast<uint32>(ml.alt_currency_cost)
						),
						static_cast<uint32>(ml.alt_currency_cost)
					);
					found = true;
					break;
				}
			}

			if (!found) {
				cost = 0;
			}
		}
		else {
			cost = 0;
		}

		auto outapp =
			new EQApplicationPacket(OP_AltCurrencySellSelection, sizeof(AltCurrencySelectItemReply_Struct));
		AltCurrencySelectItemReply_Struct *reply = (AltCurrencySelectItemReply_Struct*)outapp->pBuffer;
		reply->unknown004 = 0xFF;
		reply->unknown005 = 0xFF;
		reply->unknown006 = 0xFFFF;
		reply->unknown008 = 0xFFFF;
		strcpy(reply->item_name, inst->GetItem()->Name);
		reply->cost = cost;
		FastQueuePacket(&outapp);
	}
}

void Client::Handle_OP_Animation(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Animation_Struct)) {
		LogError("Received invalid sized OP_Animation: got [{}], expected [{}]", app->size, sizeof(Animation_Struct));
		DumpPacket(app);
		return;
	}

	Animation_Struct *s = (Animation_Struct *)app->pBuffer;

	//might verify spawn ID, but it wouldent affect anything
	DoAnim(s->action, s->speed);

	return;
}

void Client::Handle_OP_ApplyPoison(const EQApplicationPacket *app)
{
	if (app->size != sizeof(ApplyPoison_Struct)) {
		LogError("Wrong size: OP_ApplyPoison, size=[{}], expected [{}]", app->size, sizeof(ApplyPoison_Struct));
		DumpPacket(app);
		return;
	}

	ApplyPoison_Struct* ApplyPoisonData = (ApplyPoison_Struct*)app->pBuffer;

	uint32 ApplyPoisonSuccessResult = 0;

	const EQ::ItemInstance* PoisonItemInstance = GetInv().GetItem(ApplyPoisonData->inventorySlot);

	const EQ::ItemData* poison = (PoisonItemInstance ? PoisonItemInstance->GetItem() : nullptr);

	bool IsPoison = (poison && poison->ItemType == EQ::item::ItemTypePoison);

	if (IsPoison && GetClass() == Class::Rogue) {

		// Live always checks for skillup, even when poison is too high
		CheckIncreaseSkill(EQ::skills::SkillApplyPoison, nullptr, 10);

		if (poison->Proc.Level2 > GetLevel()) {
			// Poison is too high to apply.
			MessageString(Chat::LightBlue, POISON_TOO_HIGH);
		}
		else {

			double ChanceRoll = zone->random.Real(0, 1);

			// Poisons that use this skill (old world poisons) almost
			// never fail to apply.  I did 25 applies of a trivial 120+
			// poison with an apply skill of 48 and they all worked.
			// Also did 25 straight poisons at apply skill 248 for very
			// high end and they never failed.
			// Apply poison ranging from 1-9, 28/30 worked for a level 18..
			// Poisons that don't proc until a level higher than the
			// rogue simply won't apply at all, no skill check done.

			uint16 poison_skill = GetSkill(EQ::skills::SkillApplyPoison);

			if (ChanceRoll < (.75 + poison_skill / 1000)) {
				ApplyPoisonSuccessResult = 1;
				AddProcToWeapon(poison->Proc.Effect, false, (GetDEX() / 100) + 103, POISON_PROC);
			}
		}

		// Live always deletes the item, success or failure. Even if too high.
		DeleteItemInInventory(ApplyPoisonData->inventorySlot, 1, true);
	}

	auto outapp = new EQApplicationPacket(OP_ApplyPoison, nullptr, sizeof(ApplyPoison_Struct));
	ApplyPoison_Struct* ApplyPoisonResult = (ApplyPoison_Struct*)outapp->pBuffer;
	ApplyPoisonResult->success = ApplyPoisonSuccessResult;
	ApplyPoisonResult->inventorySlot = ApplyPoisonData->inventorySlot;

	FastQueuePacket(&outapp);
}

void Client::Handle_OP_Assist(const EQApplicationPacket *app)
{
	if (app->size != sizeof(EntityId_Struct)) {
		LogDebug("Size mismatch in OP_Assist expected [{}] got [{}]", sizeof(EntityId_Struct), app->size);
		return;
	}

	EntityId_Struct* eid = (EntityId_Struct*)app->pBuffer;
	Entity* entity = entity_list.GetID(eid->entity_id);

	EQApplicationPacket* outapp = app->Copy();
	eid = (EntityId_Struct*)outapp->pBuffer;
	if (RuleB(Combat, AssistNoTargetSelf))
		eid->entity_id = GetID();
	if (entity && entity->IsMob()) {
		Mob *assistee = entity->CastToMob();
		if (assistee->GetTarget()) {
			Mob *new_target = assistee->GetTarget();
			if (new_target && (GetGM() ||
				Distance(m_Position, assistee->GetPosition()) <= TARGETING_RANGE)) {
				cheat_manager.SetExemptStatus(Assist, true);
				eid->entity_id = new_target->GetID();
			} else {
				eid->entity_id = 0;
			}
		} else {
			eid->entity_id = 0;
		}
	}

	FastQueuePacket(&outapp);
	return;
}

void Client::Handle_OP_AssistGroup(const EQApplicationPacket *app)
{
	if (app->size != sizeof(EntityId_Struct)) {
		LogDebug("Size mismatch in OP_AssistGroup expected [{}] got [{}]", sizeof(EntityId_Struct), app->size);
		return;
	}

	EntityId_Struct* eid = (EntityId_Struct*)app->pBuffer;
	Entity* entity = entity_list.GetID(eid->entity_id);

	if (entity && entity->IsMob()) {
		Mob* new_target = entity->CastToMob();
		if (new_target && (GetGM() ||
			Distance(m_Position, new_target->GetPosition()) <= TARGETING_RANGE)) {
			cheat_manager.SetExemptStatus(Assist, true);
			EQApplicationPacket* outapp = new EQApplicationPacket(OP_Assist, sizeof(EntityId_Struct));
			eid = (EntityId_Struct*)outapp->pBuffer;
			eid->entity_id = new_target->GetID();
			FastQueuePacket(&outapp);
			safe_delete(outapp);
		}
	}
}

void Client::Handle_OP_AugmentInfo(const EQApplicationPacket *app)
{
	// This packet is sent by the client when an Augment item information window is opened.
	// Some clients this seems to nuke the charm text (ex. Adventurer's Stone)

	if (app->size != sizeof(AugmentInfo_Struct)) {
		LogDebug("Size mismatch in OP_AugmentInfo expected [{}] got [{}]", sizeof(AugmentInfo_Struct), app->size);
		DumpPacket(app);
		return;
	}

	AugmentInfo_Struct* AugInfo = (AugmentInfo_Struct*)app->pBuffer;
	const EQ::ItemData * item = database.GetItem(AugInfo->itemid);

	if (item) {
		strn0cpy(AugInfo->augment_info, item->Name, 64);
		AugInfo->itemid = 0;
		QueuePacket(app);
	}
}

void Client::Handle_OP_AugmentItem(const EQApplicationPacket *app)
{
	if (app->size != sizeof(AugmentItem_Struct)) {
		LogError("Invalid size for AugmentItem_Struct: Expected: [{}], Got: [{}]", sizeof(AugmentItem_Struct), app->size);
		return;
	}

	AugmentItem_Struct* in_augment = (AugmentItem_Struct*)app->pBuffer;
	if (ClientVersion() >= EQ::versions::ClientVersion::RoF) {
		if (
			(in_augment->container_slot < EQ::invslot::EQUIPMENT_BEGIN || in_augment->container_slot > EQ::invslot::GENERAL_END) &&
			(in_augment->container_slot < EQ::invbag::GENERAL_BAGS_BEGIN || in_augment->container_slot > EQ::invbag::GENERAL_BAGS_END)
		) {
			Message(Chat::Red, "The server does not allow augmentation actions from this slot.");
			auto cursor_item = m_inv[EQ::invslot::slotCursor];
			auto augmented_item = m_inv[in_augment->container_slot];
			SendItemPacket(EQ::invslot::slotCursor, cursor_item, ItemPacketCharInventory);
			// this may crash clients on certain slots
			SendItemPacket(in_augment->container_slot, augmented_item, ItemPacketCharInventory);
			return;
		}

		EQ::ItemInstance *item_one_to_push = nullptr, *item_two_to_push = nullptr;

		EQ::ItemInstance *tobe_auged = nullptr, *old_aug = nullptr, *new_aug = nullptr, *aug = nullptr, *solvent = nullptr;
		EQ::InventoryProfile& user_inv = GetInv();

		uint16 item_slot = in_augment->container_slot;
		uint16 solvent_slot = in_augment->augment_slot;
		uint8 material = EQ::InventoryProfile::CalcMaterialFromSlot(item_slot); // for when player is augging a piece of equipment while they're wearing it

		if (item_slot == INVALID_INDEX || solvent_slot == INVALID_INDEX) {
			Message(Chat::Red, "Error: Invalid Aug Index.");
			return;
		}

		tobe_auged = user_inv.GetItem(item_slot);
		solvent = user_inv.GetItem(solvent_slot);

		if (!tobe_auged) {
			Message(Chat::Red, "Error: Invalid item passed for augmenting.");
			return;
		}

		if (in_augment->augment_action == AugmentActions::Remove || in_augment->augment_action == AugmentActions::Swap) {
			if (!solvent) { // Check for valid distiller if safely removing / swapping an augmentation
				old_aug = tobe_auged->GetAugment(in_augment->augment_index);
				if (!old_aug || old_aug->GetItem()->AugDistiller != 0) {
					LogError("Player tried to safely remove an augment without a distiller");
					Message(Chat::Red, "Error: Missing an augmentation distiller for safely removing this augment.");
					return;
				}
			} else if (solvent->GetItem()->ItemType == EQ::item::ItemTypeAugmentationDistiller) {
				old_aug = tobe_auged->GetAugment(in_augment->augment_index);

				if (!old_aug) {
					LogError("Player tried to safely remove a nonexistent augment");
					Message(Chat::Red, "Error: No augment found in slot %i for safely removing.", in_augment->augment_index);
					return;
				} else if (solvent->GetItem()->ID != old_aug->GetItem()->AugDistiller) {
					LogError("Player tried to safely remove an augment with the wrong distiller (item [{}] vs expected [{}])", solvent->GetItem()->ID, old_aug->GetItem()->AugDistiller);
					Message(Chat::Red, "Error: Wrong augmentation distiller for safely removing this augment.");
					return;
				}
			} else if (solvent->GetItem()->ItemType != EQ::item::ItemTypePerfectedAugmentationDistiller) {
				LogError("Player tried to safely remove an augment with a non-distiller item");
				Message(Chat::Red, "Error: Invalid augmentation distiller for safely removing this augment.");
				return;
			}
		}

		switch (in_augment->augment_action) {
			case AugmentActions::Insert:
			case AugmentActions::Swap:
				new_aug = user_inv.GetItem(EQ::invslot::slotCursor);

				if (!new_aug) { // Shouldn't get the OP code without the augment on the user's cursor, but maybe it's h4x.
					LogError("AugmentItem OpCode with 'Insert' or 'Swap' action received, but no augment on client's cursor");
					Message(Chat::Red, "Error: No augment found on cursor for inserting.");
					break;
				} else {
					if (!RuleB(Inventory, AllowMultipleOfSameAugment) && tobe_auged->ContainsAugmentByID(new_aug->GetID())) {
						Message(Chat::Red, "Error: Cannot put multiple of the same augment in an item.");
						break;
					}

					if (
						((tobe_auged->IsAugmentSlotAvailable(new_aug->GetAugmentType(), in_augment->augment_index)) != -1) &&
						tobe_auged->AvailableWearSlot(new_aug->GetItem()->Slots)
					) {
						old_aug = tobe_auged->RemoveAugment(in_augment->augment_index);
						if (old_aug) { // An old augment was removed in order to be replaced with the new one (augment_action 2)
							CalcBonuses();

							std::vector<std::any> args;
							args.push_back(old_aug);

							if (parse->ItemHasQuestSub(tobe_auged, EVENT_UNAUGMENT_ITEM)) {
								parse->EventItem(EVENT_UNAUGMENT_ITEM, this, tobe_auged, nullptr, "", in_augment->augment_index, &args);
							}

							args.assign(1, tobe_auged);
							args.push_back(false);

							if (parse->ItemHasQuestSub(old_aug, EVENT_AUGMENT_REMOVE)) {
								parse->EventItem(EVENT_AUGMENT_REMOVE, this, old_aug, nullptr, "", in_augment->augment_index, &args);
							}

							if (parse->PlayerHasQuestSub(EVENT_AUGMENT_REMOVE_CLIENT)) {
								const auto& export_string = fmt::format(
									"{} {} {} {} {}",
									tobe_auged->GetID(),
									item_slot,
									old_aug->GetID(),
									in_augment->augment_index,
									false
								);

								args.push_back(old_aug);

								parse->EventPlayer(EVENT_AUGMENT_REMOVE_CLIENT, this, export_string, 0, &args);
							}
						}

						if (new_aug->GetItem()->Attuneable) {
							new_aug->SetAttuned(true);
						}

						tobe_auged->PutAugment(in_augment->augment_index, *new_aug);
						tobe_auged->UpdateOrnamentationInfo();

						aug = tobe_auged->GetAugment(in_augment->augment_index);
						if (aug) {
							std::vector<std::any> args;
							args.push_back(aug);

							if (parse->ItemHasQuestSub(tobe_auged, EVENT_AUGMENT_ITEM)) {
								parse->EventItem(EVENT_AUGMENT_ITEM, this, tobe_auged, nullptr, "", in_augment->augment_index, &args);
							}

							args.assign(1, tobe_auged);

							if (parse->ItemHasQuestSub(aug, EVENT_AUGMENT_INSERT)) {
								parse->EventItem(EVENT_AUGMENT_INSERT, this, aug, nullptr, "", in_augment->augment_index, &args);
							}

							args.push_back(aug);

							if (parse->PlayerHasQuestSub(EVENT_AUGMENT_INSERT_CLIENT)) {
								const auto& export_string = fmt::format(
									"{} {} {} {}",
									tobe_auged->GetID(),
									item_slot,
									aug->GetID(),
									in_augment->augment_index
								);

								parse->EventPlayer(EVENT_AUGMENT_INSERT_CLIENT, this, export_string, 0, &args);
							}
						} else {
							Message(
								Chat::Red,
								fmt::format(
									"Error: Could not properly insert augmentation into augment slot {}. Aborting.",
									in_augment->augment_index
								).c_str()
							);
							break;
						}

						item_one_to_push = tobe_auged->Clone();
						if (old_aug) {
							item_two_to_push = old_aug->Clone();
						}

						if (item_one_to_push) { // Must push items after the items in inventory are deleted - necessary due to lore items...
							DeleteItemInInventory(item_slot, 0, true);
							DeleteItemInInventory(EQ::invslot::slotCursor, new_aug->IsStackable() ? 1 : 0, true);

							if (solvent) { // Consume the augment distiller
								DeleteItemInInventory(solvent_slot, solvent->IsStackable() ? 1 : 0, true);
							}

							if (item_two_to_push) { // This is a swap. Return the old aug to the player's cursor.
								if (!PutItemInInventory(EQ::invslot::slotCursor, *item_two_to_push, true)) {
									LogError("Problem returning old augment to player's cursor after augmentation swap");
									Message(Chat::Yellow, "Error: Failed to retrieve old augment after augmentation swap!");
								}
							}

							if (PutItemInInventory(item_slot, *item_one_to_push, true)) { // Successfully added an augment to the item
								CalcBonuses();
								if (material != EQ::textures::materialInvalid) { // Visible item augged while equipped. Send WC in case ornamentation changed.
									SendWearChange(material);
								}
							} else {
								Message(Chat::Red, "Error: No available slot for end result. Please free up the augment slot.");
							}
						} else {
							Message(Chat::Red, "Error in cloning item for augment. Aborted.");
						}
					} else {
						Message(Chat::Red, "Error: No available slot for augment in that item.");
					}
				}
				break;
			case AugmentActions::Remove:
				aug = tobe_auged->GetAugment(in_augment->augment_index);
				if (aug) {
					std::vector<std::any> args;
					args.push_back(aug);

					if (parse->ItemHasQuestSub(tobe_auged, EVENT_UNAUGMENT_ITEM)) {
						parse->EventItem(EVENT_UNAUGMENT_ITEM, this, tobe_auged, nullptr, "", in_augment->augment_index, &args);
					}

					args.assign(1, tobe_auged);
					args.push_back(false);

					if (parse->ItemHasQuestSub(aug, EVENT_AUGMENT_REMOVE)) {
						parse->EventItem(EVENT_AUGMENT_REMOVE, this, aug, nullptr, "", in_augment->augment_index, &args);
					}

					args.push_back(aug);

					if (parse->PlayerHasQuestSub(EVENT_AUGMENT_REMOVE_CLIENT)) {
						const auto& export_string = fmt::format(
							"{} {} {} {} {}",
							tobe_auged->GetID(),
							item_slot,
							aug->GetID(),
							in_augment->augment_index,
							false
						);

						parse->EventPlayer(EVENT_AUGMENT_REMOVE_CLIENT, this, export_string, 0, &args);
					}
				} else {
					Message(Chat::Red, "Error: Could not find augmentation to remove at index %i. Aborting.", in_augment->augment_index);
					break;
				}

				old_aug = tobe_auged->RemoveAugment(in_augment->augment_index);
				tobe_auged->UpdateOrnamentationInfo();

				item_one_to_push = tobe_auged->Clone();
				if (old_aug) {
					item_two_to_push = old_aug->Clone();
				}

				if (item_one_to_push && item_two_to_push) {
					if (solvent) {
						DeleteItemInInventory(solvent_slot, solvent->IsStackable() ? 1 : 0, true); // Consume the augment distiller
					}

					DeleteItemInInventory(item_slot, 0, true); // Remove the augmented item

					if (!PutItemInInventory(item_slot, *item_one_to_push, true)) { // Replace it with the unaugmented item
						LogError("Problem returning equipment item to player's inventory after safe augment removal");
						Message(Chat::Yellow, "Error: Failed to return item after de-augmentation!");
					}

					CalcBonuses();

					if (material != EQ::textures::materialInvalid) {
						SendWearChange(material); // Visible item augged while equipped. Send WC in case ornamentation changed.
					}

					// Drop the removed augment on the player's cursor
					if (!PutItemInInventory(EQ::invslot::slotCursor, *item_two_to_push, true)) {
						LogError("Problem returning augment to player's cursor after safe removal");
						Message(Chat::Yellow, "Error: Failed to return augment after removal from item!");
						break;
					}
				}
				break;
			case AugmentActions::Destroy:
				// RoF client does not require an augmentation solvent for destroying an augmentation in an item.
				// Augments can be destroyed with a right click -> Destroy at any time.
				aug = tobe_auged->GetAugment(in_augment->augment_index);
				if (aug) {
					std::vector<std::any> args;
					args.push_back(aug);

					if (parse->ItemHasQuestSub(tobe_auged, EVENT_UNAUGMENT_ITEM)) {
						parse->EventItem(EVENT_UNAUGMENT_ITEM, this, tobe_auged, nullptr, "", in_augment->augment_index, &args);
					}

					args.assign(1, tobe_auged);
					args.push_back(true);

					if (parse->ItemHasQuestSub(aug, EVENT_AUGMENT_REMOVE)) {
						parse->EventItem(EVENT_AUGMENT_REMOVE, this, aug, nullptr, "", in_augment->augment_index, &args);
					}

					args.push_back(aug);

					if (parse->PlayerHasQuestSub(EVENT_AUGMENT_REMOVE_CLIENT)) {
						const auto& export_string = fmt::format(
							"{} {} {} {} {}",
							tobe_auged->GetID(),
							item_slot,
							aug->GetID(),
							in_augment->augment_index,
							true
						);

						parse->EventPlayer(EVENT_AUGMENT_REMOVE_CLIENT, this, export_string, 0, &args);
					}
				} else {
					Message(
						Chat::Red,
						fmt::format(
							"Error: Could not find augmentation to remove at index {}. Aborting.",
							in_augment->augment_index
						).c_str()
					);
					break;
				}

				tobe_auged->DeleteAugment(in_augment->augment_index);
				tobe_auged->UpdateOrnamentationInfo();

				item_one_to_push = tobe_auged->Clone();
				if (item_one_to_push) {
					DeleteItemInInventory(item_slot, 0, true);

					if (!PutItemInInventory(item_slot, *item_one_to_push, true)) {
						LogError("Problem returning equipment item to player's inventory after augment deletion");
						Message(Chat::Yellow, "Error: Failed to return item after destroying augment!");
					}
				}

				CalcBonuses();

				if (material != EQ::textures::materialInvalid) {
					SendWearChange(material);
				}

				break;
			default: // Unknown
				LogInventory(
					"Unrecognized augmentation action - cslot: [{}] aslot: [{}] cidx: [{}] aidx: [{}] act: [{}] dest: [{}]",
					in_augment->container_slot,
					in_augment->augment_slot,
					in_augment->container_index,
					in_augment->augment_index,
					in_augment->augment_action,
					in_augment->dest_inst_id
				);
				break;
		}
		safe_delete(item_one_to_push);
		safe_delete(item_two_to_push);
	} else {
		Object::HandleAugmentation(this, in_augment, m_tradeskill_object); // Delegate to tradeskill object to perform combine
	}

	return;
}

void Client::Handle_OP_AutoAttack(const EQApplicationPacket *app)
{
	if (app->size != 4) {
		LogError("OP size error: OP_AutoAttack expected:4 got:[{}]", app->size);
		return;
	}

	if (app->pBuffer[0] == 0) {
		auto_attack = false;
		if (IsAIControlled()) {
			return;
		}
		attack_timer.Disable();
		ranged_timer.Disable();
		attack_dw_timer.Disable();

		m_AutoAttackPosition       = glm::vec4();
		m_AutoAttackTargetLocation = glm::vec3();
		aa_los_them_mob            = nullptr;
	}
	else if (app->pBuffer[0] == 1) {
		auto_attack = true;
		auto_fire   = false;
		if (IsAIControlled()) {
			return;
		}
		SetAttackTimer();

		if (GetTarget()) {
			aa_los_them_mob            = GetTarget();
			m_AutoAttackPosition       = GetPosition();
			m_AutoAttackTargetLocation = glm::vec3(aa_los_them_mob->GetPosition());
			los_status                 = CheckLosFN(aa_los_them_mob);
			los_status_facing          = IsFacingMob(aa_los_them_mob);
		}
		else {
			m_AutoAttackPosition       = GetPosition();
			m_AutoAttackTargetLocation = glm::vec3();
			aa_los_them_mob            = nullptr;
			los_status                 = false;
			los_status_facing          = false;
		}
	}
}

void Client::Handle_OP_AutoAttack2(const EQApplicationPacket *app)
{
	return;
}

void Client::Handle_OP_AutoFire(const EQApplicationPacket *app)
{
	if (app->size != sizeof(bool)) {
		LogDebug("Size mismatch in OP_AutoFire expected [{}] got [{}]", sizeof(bool), app->size);
		DumpPacket(app);
		return;
	}

	if (GetTarget() == this) {
		MessageString(Chat::TooFarAway, TRY_ATTACKING_SOMEONE);
		auto_fire = false;
		return;
	}

	bool *af = (bool*)app->pBuffer;
	auto_fire = *af;
	if(!RuleB(Character, EnableRangerAutoFire))
		auto_fire = false;

	auto_attack = false;
	SetAttackTimer();
}

void Client::Handle_OP_Bandolier(const EQApplicationPacket *app)
{
	// Although there are three different structs for OP_Bandolier, they are all the same size.
	if (app->size != sizeof(BandolierCreate_Struct)) {
		LogDebug("Size mismatch in OP_Bandolier expected [{}] got [{}]", sizeof(BandolierCreate_Struct), app->size);
		DumpPacket(app);
		return;
	}

	auto bs = (BandolierCreate_Struct*) app->pBuffer;

	switch (bs->Action) {
		case bandolierCreate:
			CreateBandolier(app);
			break;
		case bandolierRemove:
			RemoveBandolier(app);
			break;
		case bandolierSet:
			if (bandolier_throttle_timer.GetDuration() && !bandolier_throttle_timer.Check()) {
				Message(
					Chat::White,
					fmt::format(
						"You may only modify your bandolier once every {}.",
						Strings::ToLower(Strings::MillisecondsToTime(RuleI(Character, BandolierSwapDelay)))
					).c_str()
				);
				SendTopLevelInventory();
				break;
			}

			SetBandolier(app);
			break;
		default:
			LogDebug("Unknown Bandolier action [{}]", bs->Action);
			break;
	}
}

void Client::Handle_OP_BankerChange(const EQApplicationPacket *app)
{
	if (app->size != sizeof(BankerChange_Struct) && app->size != 4) //Titanium only sends 4 Bytes for this
	{
		LogDebug("Size mismatch in OP_BankerChange expected [{}] got [{}]", sizeof(BankerChange_Struct), app->size);
		DumpPacket(app);
		return;
	}

	uint32 distance = 0;
	NPC *banker = entity_list.GetClosestBanker(this, distance);

	if (!banker || distance > USE_NPC_RANGE2)
	{
		auto message = fmt::format(
		    "Player tried to make use of a banker(money) but {} is non-existant or too far away ({} units).",
		    banker ? banker->GetName() : "UNKNOWN NPC", distance);
		RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});
		return;
	}

	auto outapp = new EQApplicationPacket(OP_BankerChange, nullptr, sizeof(BankerChange_Struct));
	BankerChange_Struct *bc = (BankerChange_Struct *)outapp->pBuffer;

	if (m_pp.platinum < 0)
		m_pp.platinum = 0;
	if (m_pp.gold < 0)
		m_pp.gold = 0;
	if (m_pp.silver < 0)
		m_pp.silver = 0;
	if (m_pp.copper < 0)
		m_pp.copper = 0;

	if (m_pp.platinum_bank < 0)
		m_pp.platinum_bank = 0;
	if (m_pp.gold_bank < 0)
		m_pp.gold_bank = 0;
	if (m_pp.silver_bank < 0)
		m_pp.silver_bank = 0;
	if (m_pp.copper_bank < 0)
		m_pp.copper_bank = 0;

	uint64 cp = static_cast<uint64>(m_pp.copper) +
		(static_cast<uint64>(m_pp.silver) * 10) +
		(static_cast<uint64>(m_pp.gold) * 100) +
		(static_cast<uint64>(m_pp.platinum) * 1000);

	m_pp.copper = cp % 10;
	cp /= 10;
	m_pp.silver = cp % 10;
	cp /= 10;
	m_pp.gold = cp % 10;
	cp /= 10;
	m_pp.platinum = cp;

	cp = static_cast<uint64>(m_pp.copper_bank) +
		(static_cast<uint64>(m_pp.silver_bank) * 10) +
		(static_cast<uint64>(m_pp.gold_bank) * 100) +
		(static_cast<uint64>(m_pp.platinum_bank) * 1000);

	m_pp.copper_bank = cp % 10;
	cp /= 10;
	m_pp.silver_bank = cp % 10;
	cp /= 10;
	m_pp.gold_bank = cp % 10;
	cp /= 10;
	m_pp.platinum_bank = cp;

	bc->copper = m_pp.copper;
	bc->silver = m_pp.silver;
	bc->gold = m_pp.gold;
	bc->platinum = m_pp.platinum;

	bc->copper_bank = m_pp.copper_bank;
	bc->silver_bank = m_pp.silver_bank;
	bc->gold_bank = m_pp.gold_bank;
	bc->platinum_bank = m_pp.platinum_bank;

	FastQueuePacket(&outapp);

	return;
}

void Client::Handle_OP_Barter(const EQApplicationPacket *app)
{

	if (app->size < 4)
	{
		LogDebug("OP_Barter packet below minimum expected size. The packet was [{}] bytes", app->size);
		DumpPacket(app);
		return;
	}

	char* Buf = (char *)app->pBuffer;
	auto in = (BuyerGeneric_Struct *)app->pBuffer;

	// The first 4 bytes of the packet determine the action. A lot of Barter packets require the
	// packet the client sent, sent back to it as an acknowledgement.
	//
	//uint32 Action = VARSTRUCT_DECODE_TYPE(uint32, Buf);


	switch (in->action) {

		case Barter_BuyerSearch: {
			BuyerItemSearch(app);
			break;
		}

		case Barter_SellerSearch: {
			auto bsr = (BarterSearchRequest_Struct *) app->pBuffer;
			SendBuyerResults(*bsr);
			break;
		}

		case Barter_BuyerModeOn: {
			if (!IsTrader()) {
				ToggleBuyerMode(true);
			}
			else {
				ToggleBuyerMode(false);
				Message(Chat::Red, "You cannot be a Trader and Buyer at the same time.");
			}
			break;
		}

		case Barter_BuyerModeOff: {
			ToggleBuyerMode(false);
			break;
		}

		case Barter_BuyerItemUpdate: {
			ModifyBuyLine(app);
			break;
		}

		case Barter_BuyerItemStart: {
			CreateStartingBuyLines(app);
			break;
		}

		case Barter_BuyerItemRemove: {
			auto bris = (BuyerRemoveItem_Struct *) app->pBuffer;
			BuyerBuyLinesRepository::DeleteBuyLine(database, CharacterID(), bris->buy_slot_id);
			QueuePacket(app);
			break;
		}

		case Barter_SellItem: {
			SellToBuyer(app);
			break;
		}

		case Barter_BuyerInspectBegin: {
			ShowBuyLines(app);
			break;
		}

		case Barter_BuyerInspectEnd: {
			auto bir   = (BuyerInspectRequest_Struct *) app->pBuffer;
			auto buyer = entity_list.GetClientByID(bir->buyer_id);
			if (buyer) {
				buyer->WithCustomer(0);
			}

			break;
		}
		case Barter_BarterItemInspect: {
			auto               bislr = (BarterItemSearchLinkRequest_Struct *) app->pBuffer;
			const EQ::ItemData *item = database.GetItem(bislr->item_id);

			if (!item) {
				Message(Chat::Red, "Error: This item does not exist!");
				return;
			}

			EQ::ItemInstance *inst = database.CreateItem(item);
			if (inst) {
				SendItemPacket(0, inst, ItemPacketViewLink);
				safe_delete(inst);
			}
		break;
	}
	case Barter_Welcome:
	{
		SendBarterWelcome();
		break;
	}

	case Barter_WelcomeMessageUpdate: {
		auto bwmu = (BuyerWelcomeMessageUpdate_Struct *) app->pBuffer;
		SetBuyerWelcomeMessage(bwmu->welcome_message);
		break;
	}

	case Barter_BuyerItemInspect:
	{
		BuyerItemSearchLinkRequest_Struct* bislr = (BuyerItemSearchLinkRequest_Struct*)app->pBuffer;

		const EQ::ItemData* item = database.GetItem(bislr->ItemID);

		if (!item)
			Message(Chat::Red, "Error: This item does not exist!");
		else
		{
			EQ::ItemInstance* inst = database.CreateItem(item);
			if (inst)
			{
				SendItemPacket(0, inst, ItemPacketViewLink);
				safe_delete(inst);
			}
		}
		break;
	}

	case Barter_Greeting:
	{
		auto data = (BuyerGreeting_Struct *)app->pBuffer;
		SendBuyerGreeting(data->buyer_id);
	}
	case Barter_OpenBarterWindow:
	{
		SendBulkBazaarBuyers();
		break;
	}

	default:
		Message(Chat::Red, "Unrecognised Barter action.");
		LogTrading("Unrecognised Barter Action [{}]", in->action);

	}
}

void Client::Handle_OP_BazaarInspect(const EQApplicationPacket *app)
{
	if (app->size != sizeof(BazaarInspect_Struct)) {
		LogError("Invalid size for BazaarInspect_Struct: Expected [{}], Got [{}]",
			sizeof(BazaarInspect_Struct), app->size);
		return;
	}

	BazaarInspect_Struct* bis = (BazaarInspect_Struct*)app->pBuffer;

	const EQ::ItemData* item = database.GetItem(bis->item_id);

	if (!item) {
		Message(Chat::Red, "Error: This item does not exist!");
		return;
	}

	EQ::ItemInstance* inst = database.CreateItem(item);

	if (inst) {
		SendItemPacket(0, inst, ItemPacketViewLink);
		safe_delete(inst);
	}

	return;
}

void Client::Handle_OP_BazaarSearch(const EQApplicationPacket *app)
{
	uint32 action = *(uint32 *) app->pBuffer;

	switch (action) {
		case BazaarSearch: {
			BazaarSearchCriteria_Struct *bss = (BazaarSearchCriteria_Struct *) app->pBuffer;
			BazaarSearchCriteria_Struct search_details{};

			search_details.action           = BazaarSearchResults;
            search_details.augment          = bss->augment;
            search_details._class           = bss->_class;
            search_details.item_stat        = bss->item_stat;
            search_details.min_cost         = bss->min_cost;
            search_details.max_cost         = bss->max_cost;
            search_details.min_level        = bss->min_level;
            search_details.max_level        = bss->max_level;
            search_details.max_results      = bss->max_results;
            search_details.prestige         = bss->prestige;
            search_details.race             = bss->race;
            search_details.search_scope     = bss->search_scope;
            search_details.slot             = bss->slot;
            search_details.trader_entity_id = bss->trader_entity_id;
            search_details.trader_id        = bss->trader_id;
            search_details.type             = bss->type;
			strn0cpy(search_details.item_name, bss->item_name, sizeof(search_details.item_name));

			DoBazaarSearch(search_details);
			break;
		}
		case BazaarInspect: {
			auto in = (BazaarInspect_Struct *) app->pBuffer;
			DoBazaarInspect(*in);
			break;
		}
		case WelcomeMessage: {
			SendBazaarWelcome();
			break;
		}
		case FirstOpenSearch: {
			SendBulkBazaarTraders();
			break;
		}
		default: {
			LogError("Malformed BazaarSearch_Struct packet received, ignoring\n");
		}
	}
}

void Client::Handle_OP_Begging(const EQApplicationPacket *app)
{
	if (!p_timers.Expired(&database, pTimerBeggingPickPocket, false))
	{
		Message(Chat::Red, "Ability recovery time not yet met.");

		auto outapp = new EQApplicationPacket(OP_Begging, sizeof(BeggingResponse_Struct));
		BeggingResponse_Struct *brs = (BeggingResponse_Struct*)outapp->pBuffer;
		brs->Result = 0;
		FastQueuePacket(&outapp);
		return;
	}

	if (!HasSkill(EQ::skills::SkillBegging) || !GetTarget())
		return;

	if (GetTarget()->GetClass() == Class::LDoNTreasure)
		return;

	p_timers.Start(pTimerBeggingPickPocket, 8);

	auto outapp = new EQApplicationPacket(OP_Begging, sizeof(BeggingResponse_Struct));
	BeggingResponse_Struct *brs = (BeggingResponse_Struct*)outapp->pBuffer;

	brs->Result = 0; // Default, Fail.
	if (GetTarget() == this)
	{
		FastQueuePacket(&outapp);
		return;
	}

	int RandomChance = zone->random.Int(0, 100);

	int ChanceToAttack = 0;

	if (GetLevel() > GetTarget()->GetLevel())
		ChanceToAttack = zone->random.Int(0, 15);
	else
		ChanceToAttack = zone->random.Int(((GetTarget()->GetLevel() - GetLevel()) * 10) - 5, ((GetTarget()->GetLevel() - GetLevel()) * 10));

	if (ChanceToAttack < 0)
		ChanceToAttack = -ChanceToAttack;

	if (RandomChance < ChanceToAttack)
	{
		GetTarget()->Attack(this);
		QueuePacket(outapp);
		safe_delete(outapp);
		return;
	}

	uint16 CurrentSkill = GetSkill(EQ::skills::SkillBegging);

	float ChanceToBeg = ((float)(CurrentSkill / 700.0f) + 0.15f) * 100;

	if (RandomChance < ChanceToBeg)
	{
		brs->Amount = zone->random.Int(1, 10);
		// This needs some work to determine how much money they can beg, based on skill level etc.
		if (CurrentSkill < 50)
		{
			brs->Result = 4;	// Copper
			AddMoneyToPP(brs->Amount);
		}
		else
		{
			brs->Result = 3;	// Silver
			AddMoneyToPP(brs->Amount * 10);
		}

	}
	QueuePacket(outapp);
	safe_delete(outapp);
	CheckIncreaseSkill(EQ::skills::SkillBegging, nullptr, -10);
}

void Client::Handle_OP_Bind_Wound(const EQApplicationPacket *app)
{
	if (app->size != sizeof(BindWound_Struct)) {
		LogError("Size mismatch for Bind wound packet");
		DumpPacket(app);
	}

	// Not allowed to forage while mounted
	if (RuleB(Character, NoSkillsOnHorse) && GetHorseId()) {
		MessageString(Chat::Skills, NO_SKILL_WHILE_MOUNTED);
		return;
	}

	BindWound_Struct* bind_in = (BindWound_Struct*)app->pBuffer;
	Mob* bindmob = entity_list.GetMob(bind_in->to);
	if (!bindmob) {
		LogError("Bindwound on non-exsistant mob from [{}]", GetName());
	} else {
		LogDebug("BindWound in: to:\'[{}]\' from=\'[{}]\'", bindmob->GetName(), GetName());
		BindWound(bindmob, true);
	}
	return;
}

void Client::Handle_OP_BlockedBuffs(const EQApplicationPacket *app)
{
	if (!RuleB(Spells, EnableBlockedBuffs))
		return;

	if (app->size != sizeof(BlockedBuffs_Struct))
	{
		LogDebug("Size mismatch in OP_BlockedBuffs expected [{}] got [{}]", sizeof(BlockedBuffs_Struct), app->size);

		DumpPacket(app);

		return;
	}

	std::set<uint32>::iterator Iterator;

	BlockedBuffs_Struct *bbs = (BlockedBuffs_Struct*)app->pBuffer;

	std::set<uint32> *BlockedBuffs = bbs->Pet ? &PetBlockedBuffs : &PlayerBlockedBuffs;

	if (bbs->Initialise == 1)
	{
		BlockedBuffs->clear();

		for (unsigned int i = 0; i < BLOCKED_BUFF_COUNT; ++i)
		{
			if ((IsValidSpell(bbs->SpellID[i])) && IsBeneficialSpell(bbs->SpellID[i]) && !spells[bbs->SpellID[i]].no_block)
			{
				if (BlockedBuffs->find(bbs->SpellID[i]) == BlockedBuffs->end())
					BlockedBuffs->insert(bbs->SpellID[i]);
			}
		}

		auto outapp = new EQApplicationPacket(OP_BlockedBuffs, sizeof(BlockedBuffs_Struct));

		BlockedBuffs_Struct *obbs = (BlockedBuffs_Struct*)outapp->pBuffer;

		for (unsigned int i = 0; i < BLOCKED_BUFF_COUNT; ++i)
			obbs->SpellID[i] = -1;

		obbs->Pet = bbs->Pet;
		obbs->Initialise = 1;
		obbs->Flags = 0x54;
		obbs->Count = BlockedBuffs->size();

		unsigned int Element = 0;

		Iterator = BlockedBuffs->begin();

		while (Iterator != BlockedBuffs->end())
		{
			obbs->SpellID[Element++] = (*Iterator);
			++Iterator;
		}

		FastQueuePacket(&outapp);
		return;
	}

	if ((bbs->Initialise == 0) && (bbs->Count > 0))
	{
		auto outapp = new EQApplicationPacket(OP_BlockedBuffs, sizeof(BlockedBuffs_Struct));

		BlockedBuffs_Struct *obbs = (BlockedBuffs_Struct*)outapp->pBuffer;

		for (unsigned int i = 0; i < BLOCKED_BUFF_COUNT; ++i)
			obbs->SpellID[i] = -1;

		obbs->Pet = bbs->Pet;
		obbs->Initialise = 0;
		obbs->Flags = 0x54;

		for (unsigned int i = 0; i < BLOCKED_BUFF_COUNT; ++i)
		{
			if (!IsValidSpell(bbs->SpellID[i]) || !IsBeneficialSpell(bbs->SpellID[i]) || spells[bbs->SpellID[i]].no_block)
				continue;

			if ((BlockedBuffs->size() < BLOCKED_BUFF_COUNT) && (BlockedBuffs->find(bbs->SpellID[i]) == BlockedBuffs->end()))
				BlockedBuffs->insert(bbs->SpellID[i]);
		}
		obbs->Count = BlockedBuffs->size();

		Iterator = BlockedBuffs->begin();

		unsigned int Element = 0;

		while (Iterator != BlockedBuffs->end())
		{
			obbs->SpellID[Element++] = (*Iterator);
			++Iterator;
		}

		FastQueuePacket(&outapp);
	}
}

void Client::Handle_OP_BoardBoat(const EQApplicationPacket *app)
{
	// this sends unclean mob name, so capped at 64
	// a_boat006
	if (app->size <= 5 || app->size > 64) {
		LogError("Size mismatch in OP_BoardBoad. Expected greater than 5 less than 64, got [{}]", app->size);
		DumpPacket(app);
		return;
	}

	char boatname[64];
	memcpy(boatname, app->pBuffer, app->size);
	boatname[63] = '\0';

	Mob* boat = entity_list.GetMob(boatname);
	if (!boat || !boat->IsControllableBoat()) {
		return;
	}
	controlling_boat_id = boat->GetID();	// set the client's BoatID to show that it's on this boat

	Message(Chat::White, "Board boat: %s", boatname);

	return;
}

void Client::Handle_OP_BookButton(const EQApplicationPacket* app)
{
	if (app->size != sizeof(BookButton_Struct))
	{
		LogError("Size mismatch in OP_BookButton. expected [{}] got [{}]", sizeof(BookButton_Struct), app->size);
		DumpPacket(app);
		return;
	}

	BookButton_Struct* book = reinterpret_cast<BookButton_Struct*>(app->pBuffer);

	const EQ::ItemInstance* const inst = GetInv().GetItem(book->invslot);
	if (inst && inst->GetItem())
	{
		// todo: cast spell button (unknown if anything on live uses this)
		ScribeRecipes(inst->GetItem()->ID);
		DeleteItemInInventory(book->invslot, 1, true);
	}

	EQApplicationPacket outapp(OP_FinishWindow, 0);
	QueuePacket(&outapp);
}

void Client::Handle_OP_Buff(const EQApplicationPacket *app)
{
	/*
		Note: if invisibility is on client, this will force it to drop.
	*/
	if (app->size != sizeof(SpellBuffPacket_Struct))
	{
		LogError("Size mismatch in OP_Buff. expected [{}] got [{}]", sizeof(SpellBuffPacket_Struct), app->size);
		DumpPacket(app);
		return;
	}

	SpellBuffPacket_Struct* sbf = (SpellBuffPacket_Struct*)app->pBuffer;
	uint32 spid = sbf->buff.spellid;
	LogSpells("Client requested that buff with spell id [{}] be canceled", spid);

	//something about IsDetrimentalSpell() crashes this portion of code..
	//tbh we shouldn't use it anyway since this is a simple red vs blue buff check and
	//isdetrimentalspell() is much more complex
	if (spid == 0xFFFF || (IsValidSpell(spid) && (spells[spid].good_effect == 0))) {
		QueuePacket(app);
	}
	else {
		BuffFadeBySpellID(spid);
	}

	return;
}

void Client::Handle_OP_BuffRemoveRequest(const EQApplicationPacket *app)
{
	// In SoD, this is used for clicking off Pet Buffs only. In Underfoot, it is used both for Client and Pets
	// The payload contains buffslot and EntityID only, so we must check if the EntityID is ours or our pets.
	//
	VERIFY_PACKET_LENGTH(OP_BuffRemoveRequest, app, BuffRemoveRequest_Struct);

	BuffRemoveRequest_Struct *brrs = (BuffRemoveRequest_Struct*)app->pBuffer;

	Mob *m = nullptr;

	if (brrs->EntityID == GetID()) {
		m = this;
	}
	else if (brrs->EntityID == GetPetID()) {
		m = GetPet();
	}
	else if (GetGM())
	{
		m = entity_list.GetMobID(brrs->EntityID);
	}
	else {
		if (RuleB(Bots, Enabled)) {
			Mob *bot_test = entity_list.GetMob(brrs->EntityID);
			if (bot_test && bot_test->IsBot() && bot_test->GetOwner() == this) {
				m = bot_test;
			}
		}
	}

	if (!m)
		return;

	if (brrs->SlotID > (uint32)m->GetMaxTotalSlots())
		return;

	uint16 SpellID = m->GetSpellIDFromSlot(brrs->SlotID);

	if (SpellID && (GetGM() || ((IsBeneficialSpell(SpellID) || IsEffectInSpell(SpellID, SpellEffect::BindSight)) && !spells[SpellID].no_remove))) {
		m->BuffFadeBySlot(brrs->SlotID, true);
	}
}

void Client::Handle_OP_Bug(const EQApplicationPacket *app)
{
	if (!RuleB(Bugs, ReportingSystemActive)) {
		Message(Chat::Red, "Bug reporting is disabled on this server.");
		return;
	}

	if (app->size != sizeof(BugReport_Struct)) {
		LogError("Wrong size of BugReport_Struct got {} expected {}!", app->size, sizeof(BugReport_Struct));
		return;
	}

	auto *r = (BugReport_Struct *) app->pBuffer;
	RegisterBug(r);
}

void Client::Handle_OP_Camp(const EQApplicationPacket *app)
{
	if (ClientVersion() == EQ::versions::ClientVersion::RoF2 && RuleB(Parcel, EnableParcelMerchants) && GetEngagedWithParcelMerchant()) {
		Stand();
		MessageString(Chat::Yellow, TRADER_BUSY_TWO);
		return;
	}

	if (IsLFP())
		worldserver.StopLFP(CharacterID());

	if (GetGM())
	{
		if (RuleB(Character, EnableHackedFastCampForGM))
		{
			camp_timer.Start(100, true);
		}
		else {
			OnDisconnect(true);
		}

		return;
	}

	camp_timer.Start(29000, true);

	if (RuleB(Bots, Enabled)) {
		bot_camp_timer.Start((RuleI(Bots, CampTimer) * 1000), true);
	}

	return;
}

void Client::Handle_OP_CancelTask(const EQApplicationPacket *app)
{

	if (app->size != sizeof(CancelTask_Struct)) {
		LogDebug("Size mismatch in OP_CancelTask expected [{}] got [{}]", sizeof(CancelTask_Struct), app->size);
		DumpPacket(app);
		return;
	}
	CancelTask_Struct *cts = (CancelTask_Struct*)app->pBuffer;

	if (RuleB(TaskSystem, EnableTaskSystem) && task_state)
		task_state->CancelTask(this, cts->SequenceNumber, static_cast<TaskType>(cts->type));
}

void Client::Handle_OP_CancelTrade(const EQApplicationPacket *app)
{
	if (app->size != sizeof(CancelTrade_Struct)) {
		LogError("Wrong size: OP_CancelTrade, size=[{}], expected [{}]", app->size, sizeof(CancelTrade_Struct));
		return;
	}
	Mob* with = trade->With();
	if (with && with->IsClient()) {
		CancelTrade_Struct* msg = (CancelTrade_Struct*)app->pBuffer;

		// Forward cancel packet to other client
		msg->fromid = with->GetID();
		//msg->action = 1;

		with->CastToClient()->QueuePacket(app);

		// Put trade items/cash back into inventory
		FinishTrade(this);
		trade->Reset();
	}
	else if (with) {
		CancelTrade_Struct* msg = (CancelTrade_Struct*)app->pBuffer;
		msg->fromid = with->GetID();
		QueuePacket(app);
		FinishTrade(this);
		trade->Reset();
	}

	if (ClientVersion() == EQ::versions::ClientVersion::RoF2 && RuleB(Parcel, EnableParcelMerchants)) {
		DoParcelCancel();
		SetEngagedWithParcelMerchant(false);
	}

	EQApplicationPacket end_trade1(OP_FinishWindow, 0);
	QueuePacket(&end_trade1);

	EQApplicationPacket end_trade2(OP_FinishWindow2, 0);
	QueuePacket(&end_trade2);
	return;
}

void Client::Handle_OP_CastSpell(const EQApplicationPacket *app)
{
	using EQ::spells::CastingSlot;
	if (app->size != sizeof(CastSpell_Struct)) {
		std::cout << "Wrong size: OP_CastSpell, size=" << app->size << ", expected " << sizeof(CastSpell_Struct) << std::endl;
		return;
	}
	if (IsAIControlled()) {
		MessageString(Chat::Red, NOT_IN_CONTROL);
		//Message(Chat::Red, "You cant cast right now, You aren't in control of yourself!");
		return;
	}

	BreakInvisibleSpells();

	CastSpell_Struct* castspell = (CastSpell_Struct*)app->pBuffer;

	m_TargetRing = glm::vec3(castspell->x_pos, castspell->y_pos, castspell->z_pos);

	if (castspell->spell_id && IsValidSpell(castspell->spell_id)) {
		bool is_non_combat_zone = !zone->CanDoCombat() || zone->BuffTimersSuspended();
		bool is_excluded_reset  = is_non_combat_zone && IsBardSong(castspell->spell_id);
		if (!is_excluded_reset) {
			ResetAFKTimer();
		}
	}

	LogSpells("OP CastSpell: slot [{}] spell [{}] target [{}] inv [{}]", castspell->slot, castspell->spell_id, castspell->target_id, (unsigned long)castspell->inventoryslot);
	CastingSlot slot = static_cast<CastingSlot>(castspell->slot);

	if (RuleB(Spells, RequireMnemonicRetention)) {
		if (EQ::ValueWithin(castspell->slot, 8, 11) && GetAA(aaMnemonicRetention) < (castspell->slot - 7)) {
			InterruptSpell(castspell->spell_id);
			Message(Chat::Red, "You do not have the required AA to use this spell slot.");
			return;
		}
	}

	/* Memorized Spell */
	if (m_pp.mem_spells[castspell->slot] && m_pp.mem_spells[castspell->slot] == castspell->spell_id) {
		uint16 spell_to_cast = 0;
		if (castspell->slot < EQ::spells::SPELL_GEM_COUNT) {
			spell_to_cast = m_pp.mem_spells[castspell->slot];
			if (spell_to_cast != castspell->spell_id) {
				InterruptSpell(castspell->spell_id); //CHEATER!!!
				return;
			}
		}
		else if (castspell->slot >= EQ::spells::SPELL_GEM_COUNT) {
			InterruptSpell();
			return;
		}

		if (IsValidSpell(spell_to_cast)) {
			CastSpell(spell_to_cast, castspell->target_id, slot);
		}
		else {
			InterruptSpell();
		}
	}
	/* Spell Slot or Potion Belt Slot */
	else if (slot == CastingSlot::Item || slot == CastingSlot::PotionBelt)	// ITEM or POTION cast
	{
		if (m_inv.SupportsClickCasting(castspell->inventoryslot) || slot == CastingSlot::PotionBelt)	// sanity check
		{
			// packet field types will be reviewed as packet transistions occur
			const EQ::ItemInstance* inst = m_inv[castspell->inventoryslot]; //slot values are int16, need to check packet on this field
																			   //bool cancast = true;
			if (inst && inst->IsClassCommon())
			{
				const EQ::ItemData* item = inst->GetItem();
				if (item->Click.Effect != (uint32)castspell->spell_id)
				{
					std::string message = fmt::format("OP_CastSpell with item, tried to cast a different spell than what was on item - item spell id [{}] attempted [{}]", item->Click.Effect, (uint32)castspell->spell_id);
					RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});
					InterruptSpell(castspell->spell_id);	//CHEATER!!
					return;
				}

				if ((item->Click.Type == EQ::item::ItemEffectClick) || (item->Click.Type == EQ::item::ItemEffectExpendable) || (item->Click.Type == EQ::item::ItemEffectEquipClick) || (item->Click.Type == EQ::item::ItemEffectClick2))
				{
					if (item->Click.Level2 > 0)
					{
						if (GetLevel() >= item->Click.Level2)
						{
							auto* p_inst = (EQ::ItemInstance*) inst;
							int i = 0;

							if (parse->ItemHasQuestSub(p_inst, EVENT_ITEM_CLICK_CAST)) {
								i = parse->EventItem(EVENT_ITEM_CLICK_CAST, this, p_inst, nullptr, "", castspell->inventoryslot);
							}

							if (parse->PlayerHasQuestSub(EVENT_ITEM_CLICK_CAST_CLIENT)) {
								std::vector<std::any> args;
								args.emplace_back(p_inst);
								i = parse->EventPlayer(EVENT_ITEM_CLICK_CAST_CLIENT, this, std::to_string(castspell->inventoryslot), 0, &args);
							}

							if (i == 0) {
								CastSpell(item->Click.Effect, castspell->target_id, slot, item->CastTime, 0, 0, castspell->inventoryslot);
							}
							else {
								InterruptSpell(castspell->spell_id);
								SendSpellBarEnable(castspell->spell_id);
								return;
							}
						}
						else
						{
							RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = "OP_CastSpell with item, did not meet req level."});
							Message(Chat::Red, "Error: level not high enough.", castspell->inventoryslot);
							InterruptSpell(castspell->spell_id);
						}
					}
					else
					{
						auto* p_inst = (EQ::ItemInstance*) inst;

						int i = 0;

						if (parse->ItemHasQuestSub(p_inst, EVENT_ITEM_CLICK_CAST)) {
							i = parse->EventItem(EVENT_ITEM_CLICK_CAST, this, p_inst, nullptr, "", castspell->inventoryslot);
						}

						if (parse->PlayerHasQuestSub(EVENT_ITEM_CLICK_CAST_CLIENT)) {
							std::vector<std::any> args;
							args.emplace_back(p_inst);
							i = parse->EventPlayer(EVENT_ITEM_CLICK_CAST_CLIENT, this, std::to_string(castspell->inventoryslot), 0, &args);
						}

						if (i == 0) {
							CastSpell(item->Click.Effect, castspell->target_id, slot, item->CastTime, 0, 0, castspell->inventoryslot);
						}
						else {
							InterruptSpell(castspell->spell_id);
							SendSpellBarEnable(castspell->spell_id);
							return;
						}
					}
				}
				else
				{
					Message(Chat::Red, "Error: unknown item->Click.Type (0x%02x)", item->Click.Type);
				}
			}
			else
			{
				Message(Chat::Red, "Error: item not found in inventory slot #%i", castspell->inventoryslot);
				InterruptSpell(castspell->spell_id);
			}
		}
		else
		{
			Message(Chat::Red, "Error: castspell->inventoryslot >= %i (0x%04x)", EQ::invslot::slotCursor, castspell->inventoryslot);
			InterruptSpell(castspell->spell_id);
		}
	}
	/* Discipline -- older clients use the same slot as items, but we translate to it's own */
	else if (slot == CastingSlot::Discipline) {
		if (!UseDiscipline(castspell->spell_id, castspell->target_id)) {
			LogSpells("Unknown ability being used by [{}] spell being cast is: [{}]\n", GetName(), castspell->spell_id);
			InterruptSpell(castspell->spell_id);
			return;
		}
	}
	/* ABILITY cast (LoH and Harm Touch) */
	else if (slot == CastingSlot::Ability) {
		uint16 spell_to_cast = 0;

		if (castspell->spell_id == SPELL_LAY_ON_HANDS && GetClass() == Class::Paladin) {
			if (!p_timers.Expired(&database, pTimerLayHands)) {
				Message(Chat::Red, "Ability recovery time not yet met.");
				InterruptSpell(castspell->spell_id);
				return;
			}
			spell_to_cast = SPELL_LAY_ON_HANDS;
			p_timers.Start(pTimerLayHands, LayOnHandsReuseTime);
		}
		else if ((castspell->spell_id == SPELL_HARM_TOUCH
			|| castspell->spell_id == SPELL_HARM_TOUCH2) && GetClass() == Class::ShadowKnight) {
			if (!p_timers.Expired(&database, pTimerHarmTouch)) {
				Message(Chat::Red, "Ability recovery time not yet met.");
				InterruptSpell(castspell->spell_id);
				return;
			}

			// determine which version of HT we are casting based on level
			if (GetLevel() < 40)
				spell_to_cast = SPELL_HARM_TOUCH;
			else
				spell_to_cast = SPELL_HARM_TOUCH2;

			p_timers.Start(pTimerHarmTouch, HarmTouchReuseTime);
		}

		if (spell_to_cast > 0)	// if we've matched LoH or HT, cast now
			CastSpell(spell_to_cast, castspell->target_id, slot);
	}
	return;
}

void Client::Handle_OP_ChannelMessage(const EQApplicationPacket *app)
{
	auto* cm = (ChannelMessage_Struct*) app->pBuffer;

	if (app->size < sizeof(ChannelMessage_Struct)) {
		LogDebug("Size mismatch in OP_ChannelMessage expected [{}] got [{}]", sizeof(ChannelMessage_Struct), app->size);
		return;
	}

	// reject automatic AFK messages from resetting /afk
	std::string message = cm->message;
	if (!Strings::Contains(message, "Sorry, I am A.F.K.")) {
		ResetAFKTimer();
	}

	if (IsAIControlled() && !GetGM()) {
		Message(Chat::Red, "You try to speak but can't move your mouth!");
		return;
	}

	uint8 language_skill = Language::MaxValue;
	if (EQ::ValueWithin(cm->language, Language::CommonTongue, Language::Unknown27)) {
		language_skill = m_pp.languages[cm->language];
	}

	ChannelMessageReceived(cm->chan_num, cm->language, language_skill, cm->message, cm->targetname);
	return;
}

void Client::Handle_OP_ChangePetName(const EQApplicationPacket *app) {
	if (app->size != sizeof(ChangePetName_Struct)) {
		LogError("Got OP_ChangePetName of incorrect size. Expected [{}], got [{}].", sizeof(ChangePetName_Struct), app->size);
		return;
	}

	auto p = (ChangePetName_Struct *) app->pBuffer;
	if (!IsPetNameChangeAllowed()) {
		p->response_code = ChangeNameResponse::Ineligible;
		QueuePacket(app);
		return;
	}

	p->response_code = ChangeNameResponse::Denied;
	if (ChangePetName(p->new_pet_name)) {
		p->response_code = ChangeNameResponse::Accepted;
	}

	QueuePacket(app);
}

void Client::Handle_OP_ClearBlockedBuffs(const EQApplicationPacket *app)
{
	if (!RuleB(Spells, EnableBlockedBuffs))
		return;

	if (app->size != 1)
	{
		LogDebug("Size mismatch in OP_ClearBlockedBuffs expected 1 got [{}]", app->size);

		DumpPacket(app);

		return;
	}

	bool Pet = app->pBuffer[0];

	if (Pet)
		PetBlockedBuffs.clear();
	else
		PlayerBlockedBuffs.clear();

	QueuePacket(app);
}

void Client::Handle_OP_ClearNPCMarks(const EQApplicationPacket *app)
{

	if (app->size != 0)
	{
		LogDebug("Size mismatch in OP_ClearNPCMarks expected 0 got [{}]", app->size);

		DumpPacket(app);

		return;
	}

	Group *g = GetGroup();

	if (g)
		g->ClearAllNPCMarks();
}

void Client::Handle_OP_ClearSurname(const EQApplicationPacket *app)
{
	ChangeLastName("");
}

void Client::Handle_OP_ClickDoor(const EQApplicationPacket *app)
{
	if (app->size != sizeof(ClickDoor_Struct)) {
		LogError("Wrong size: OP_ClickDoor, size=[{}], expected [{}]", app->size, sizeof(ClickDoor_Struct));
		return;
	}
	ClickDoor_Struct* cd = (ClickDoor_Struct*)app->pBuffer;
	Doors* currentdoor = entity_list.FindDoor(cd->doorid);
	if (!currentdoor)
	{
		Message(Chat::Red, "Unable to find door, please notify a GM (DoorID: %i).", cd->doorid);
		return;
	}

	float distance = DistanceNoZ(GetPosition(), currentdoor->GetPosition());

	LogDoors(
		"Door [{}] client handle, client distance from door [{:.2f}]",
		currentdoor->GetDoorID(),
		distance
	);

	bool within_distance = distance < RuleI(Range, MaxDistanceToClickDoors);

	// distance gate this because some doors are client controlled and the client
	// will spam door click even across the zone to force a door back into desired state
	if (IsDevToolsEnabled() && within_distance) {
		SetDoorToolEntityId(currentdoor->GetEntityID());
		DoorManipulation::CommandHeader(this);
		Message(
			Chat::White,
			fmt::format(
				"Door ({}) [{}]",
				currentdoor->GetDoorID(),
				Saylink::Silent("#door edit")
			).c_str()
		);
	}

	// don't spam scripts with client controlled doors if not within distance
	if (within_distance) {
		int quest_return = 0;
		if (parse->PlayerHasQuestSub(EVENT_CLICK_DOOR)) {
			std::vector<std::any> args = {currentdoor};

			quest_return = parse->EventPlayer(EVENT_CLICK_DOOR, this, std::to_string(cd->doorid), 0, &args);
		}

		if (parse->ZoneHasQuestSub(EVENT_CLICK_DOOR)) {
			std::vector<std::any> args = { currentdoor, this };

			quest_return = parse->EventZone(EVENT_CLICK_DOOR, zone, std::to_string(cd->doorid), 0, &args);
		}

		if (quest_return == 0) {
			currentdoor->HandleClick(this, 0);
		}
	}
	else {
		// we let this pass because client controlled doors require this to force the linked doors
		// back into state
		currentdoor->HandleClick(this, 0);
	}

}

void Client::Handle_OP_ClickObject(const EQApplicationPacket *app)
{
	if (app->size != sizeof(ClickObject_Struct)) {
		LogError("Invalid size on ClickObject_Struct: Expected [{}], Got [{}]",
			sizeof(ClickObject_Struct), app->size);
		return;
	}

	auto* click_object = (ClickObject_Struct*) app->pBuffer;
	auto* entity = entity_list.GetID(click_object->drop_id);
	if (entity && entity->IsObject()) {
		auto* object = entity->CastToObject();

		object->HandleClick(this, click_object);

		if (parse->PlayerHasQuestSub(EVENT_CLICK_OBJECT)) {
			std::vector<std::any> args = { object };
			parse->EventPlayer(EVENT_CLICK_OBJECT, this, std::to_string(click_object->drop_id), GetID(), &args);
		}

		if (parse->ZoneHasQuestSub(EVENT_CLICK_OBJECT)) {
			std::vector<std::any> args = { object, this };
			parse->EventZone(EVENT_CLICK_OBJECT, zone, std::to_string(click_object->drop_id), GetID(), &args);
		}

		if (IsDevToolsEnabled()) {
			SetObjectToolEntityId(entity->GetID());
			ObjectManipulation::CommandHeader(this);
			Message(
				Chat::White,
				fmt::format(
					"Object ({}) [{}] [{}]",
					entity->CastToObject()->GetDBID(),
					Saylink::Silent("#object edit", "Edit"),
					Saylink::Silent("#object delete", "Delete")
				).c_str()
			);
		}
	}

	// Observed in RoF after OP_ClickObjectAction:
	//EQApplicationPacket end_trade2(OP_FinishWindow2, 0);
	//QueuePacket(&end_trade2);
	return;
}

void Client::Handle_OP_ClickObjectAction(const EQApplicationPacket *app)
{
	if (app->size == 0) {
		// RoF sends this packet 0 sized when switching from auto-combine to experiment windows.
		// Not completely sure if 0 sized is for this or for closing objects as commented out below
		EQApplicationPacket end_trade1(OP_FinishWindow, 0);
		QueuePacket(&end_trade1);

		EQApplicationPacket end_trade2(OP_FinishWindow2, 0);
		QueuePacket(&end_trade2);

		// RoF sends a 0 sized packet for closing objects
		if (GetTradeskillObject() && ClientVersion() >= EQ::versions::ClientVersion::RoF)
			GetTradeskillObject()->CastToObject()->Close();

		return;
	}
	else
	{
		if (app->size != sizeof(ClickObjectAction_Struct)) {
			LogError("Invalid size on OP_ClickObjectAction: Expected [{}], Got [{}]",
				sizeof(ClickObjectAction_Struct), app->size);
			return;
		}

		ClickObjectAction_Struct* oos = (ClickObjectAction_Struct*)app->pBuffer;
		Entity* entity = entity_list.GetEntityObject(oos->drop_id);
		if (entity && entity->IsObject()) {
			Object* object = entity->CastToObject();
			if (oos->open == 0) {
				object->Close();
			}
			else {
				LogError("Unsupported action [{}] in OP_ClickObjectAction", oos->open);
			}
		}
		else {
			LogError("Invalid object [{}] in OP_ClickObjectAction", oos->drop_id);
		}
	}

	SetTradeskillObject(nullptr);

	EQApplicationPacket end_trade1(OP_FinishWindow, 0);
	QueuePacket(&end_trade1);

	EQApplicationPacket end_trade2(OP_FinishWindow2, 0);
	QueuePacket(&end_trade2);
	return;
}

void Client::Handle_OP_ClientError(const EQApplicationPacket *app)
{
	ClientError_Struct* error = (ClientError_Struct*)app->pBuffer;
	LogError("Client error: [{}]", error->character_name);
	LogError("Error message:[{}]", error->message);
	return;
}

void Client::Handle_OP_ClientTimeStamp(const EQApplicationPacket *app)
{
	return;
}

void Client::Handle_OP_ClientUpdate(const EQApplicationPacket *app) {
	if (IsAIControlled())
		return;

	if (dead)
		return;

	/* Invalid size check */
	if (app->size != sizeof(PlayerPositionUpdateClient_Struct)
		&& app->size != (sizeof(PlayerPositionUpdateClient_Struct) + 1)
			) {
		LogError("OP size error: OP_ClientUpdate expected:[{}] got:[{}]",
			sizeof(PlayerPositionUpdateClient_Struct), app->size);
		return;
	}

	PlayerPositionUpdateClient_Struct *ppu = (PlayerPositionUpdateClient_Struct *) app->pBuffer;

	/* Non PC handling like boats and eye of zomm */
	if (ppu->spawn_id && ppu->spawn_id != GetID()) {
		Mob *cmob = entity_list.GetMob(ppu->spawn_id);

		if (!cmob) {
			return;
		}

		if (cmob->IsControllableBoat()) {
			// Controllable boats
			auto boat_delta = glm::vec4(ppu->delta_x, ppu->delta_y, ppu->delta_z, EQ10toFloat(ppu->delta_heading));
			cmob->SetDelta(boat_delta);

			static EQApplicationPacket outapp(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
			auto *ppus = (PlayerPositionUpdateServer_Struct *) outapp.pBuffer;
			cmob->MakeSpawnUpdate(ppus);
			entity_list.QueueCloseClients(cmob, &outapp, true, 300, this, false);

			/* Update the boat's position on the server, without sending an update */
			cmob->GMMove(ppu->x_pos, ppu->y_pos, ppu->z_pos, EQ12toFloat(ppu->heading));
			return;
		}
		else {
			// Eye of Zomm needs code here to track position of the eye on server
			// so that other clients see it.  I could add a check here for eye of zomm
			// race, to limit this code, but this should handle any client controlled
			// mob that gets updates from OP_ClientUpdate
			if (!cmob->IsControllableBoat() && ppu->spawn_id == controlled_mob_id) {
				cmob->SetPosition(ppu->x_pos, ppu->y_pos, ppu->z_pos);
				cmob->SetHeading(EQ12toFloat(ppu->heading));
				mMovementManager->SendCommandToClients(cmob, 0.0, 0.0, 0.0,
						0.0, 0, ClientRangeAny, nullptr, this);
				cmob->CastToNPC()->SaveGuardSpot(glm::vec4(ppu->x_pos,
						ppu->y_pos, ppu->z_pos, EQ12toFloat(ppu->heading)));
			}
		}
	return;
	}

	// At this point, all that's left is a client update.
	// Pure boat updates, and client contolled mob updates are complete.
	// This can still be tricky.  If ppu->vehicle_id is set, then the client
	// position is actually an offset from the boat he is inside.

	bool	on_boat = (ppu->vehicle_id != 0);

	// From this point forward, we need to use a new set of variables for client
	// position.  If the client is in a boat, we need to add the boat pos and
	// the client offset together.

	float	cx = ppu->x_pos;
	float	cy = ppu->y_pos;
	float	cz = ppu->z_pos;
	float 	new_heading = EQ12toFloat(ppu->heading);

	if (on_boat) {
		Mob *boat = entity_list.GetMob(ppu->vehicle_id);
		if (boat == 0) {
			LogError("Can't find boat for client position offset.");
		}
		else {
			if (boat->turning) return;

			// Calculate angle from boat heading to EQ heading
			double theta = std::fmod(((boat->GetHeading() * 360.0) / 512.0),360.0);
			double thetar = (theta * M_PI) / 180.0;

			// Boat cx is inverted (positive to left)
			// Boat cy is normal (positive toward heading)
			double cosine = std::cos(thetar);
			double sine = std::sin(thetar);

			double normalizedx, normalizedy;
			normalizedx = cx * cosine - -cy * sine;
			normalizedy = -cx * sine + cy * cosine;

			cx = boat->GetX() + normalizedx;
			cy = boat->GetY() + normalizedy;
			cz += boat->GetZ();

			new_heading += boat->GetHeading();
		}
	}

	cheat_manager.MovementCheck(glm::vec3(cx, cy, cz));

	if (IsDraggingCorpse())
		DragCorpses();

	/* Check to see if PPU should trigger an update to the rewind position. */
	float rewind_x_diff = 0;
	float rewind_y_diff = 0;

	rewind_x_diff = cx - m_RewindLocation.x;
	rewind_x_diff *= rewind_x_diff;
	rewind_y_diff = cy - m_RewindLocation.y;
	rewind_y_diff *= rewind_y_diff;

	/*
		We only need to store updated values if the player has moved.
		If the player has moved more than units for x or y, then we'll store
		his pre-PPU x and y for /rewind, in case he gets stuck.
	*/

	if ((rewind_x_diff > 750) || (rewind_y_diff > 750))
		m_RewindLocation = glm::vec3(m_Position);

	/*
		If the PPU was a large jump, such as a cross zone gate or Call of Hero,
			just update rewind coordinates to the new ppu coordinates. This will prevent exploitation.
	*/

	if ((rewind_x_diff > 5000) || (rewind_y_diff > 5000))
		m_RewindLocation = glm::vec3(cx, cy, cz);

	if (proximity_timer.Check()) {
		entity_list.ProcessMove(this, glm::vec3(cx, cy, cz));
		if (RuleB(TaskSystem, EnableTaskSystem) && RuleB(TaskSystem, EnableTaskProximity))
			ProcessTaskProximities(cx, cy, cz);

		m_Proximity = glm::vec3(cx, cy, cz);
	}

	if (RuleB(Skills, TrackingAutoRefreshSkillUps) && IsTracking() && ((m_Position.x != cx) || (m_Position.y != cy))) {
		if (zone->random.Real(0, 100) < 70)//should be good
			CheckIncreaseSkill(EQ::skills::SkillTracking, nullptr, -20);
	}

	if (cy != m_Position.y || cx != m_Position.x) {
		// End trader mode if we move
		if (IsTrader()) {
			TraderEndTrader();
		}

		if (IsBuyer()) {
			ToggleBuyerMode(false);
		}

		/* Break Hide if moving without sneaking and set rewind timer if moved */
		if ((hidden || improved_hidden) && !sneaking) {
			hidden = false;
			improved_hidden = false;
			if (!invisible) {
				auto outapp =
						new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
				SpawnAppearance_Struct *sa_out = (SpawnAppearance_Struct *) outapp->pBuffer;
				sa_out->spawn_id = GetID();
				sa_out->type = 0x03;
				sa_out->parameter = 0;
				entity_list.QueueClients(this, outapp, true);
				safe_delete(outapp);
			}
		}
		rewind_timer.Start(30000, true);
	}

	glm::vec4 prevDelta = m_Delta; // SetMoving clears m_Delta
	SetMoving(!(cy == m_Position.y && cx == m_Position.x));

	if (RuleB(Character, EnableAutoAFK)) {
		CheckAutoIdleAFK(ppu);
	}

	CheckClientToNpcAggroTimer();

	if (m_mob_check_moving_timer.Check()) {
		CheckScanCloseMobsMovingTimer();
	}

	CheckSendBulkNpcPositions();

	/* Update internal server position from what the client has sent */
	glm::vec4 prevPosition = m_Position;
	m_Position = glm::vec4(cx, cy, cz, new_heading);
	m_Delta = glm::vec4(ppu->delta_x, ppu->delta_y, ppu->delta_z, EQ10toFloat(ppu->delta_heading));
	int32 prevAnimation = ppu->animation;
	animation = ppu->animation;
	bool positionUpdated = m_Position != prevPosition || m_Delta != prevDelta || m_Delta != glm::vec4(0.0f) || prevAnimation != animation;

	/* Visual Debugging */
	if (RuleB(Character, OPClientUpdateVisualDebug)) {
		LogDebug("ClientUpdate: ppu x: [{}] y: [{}] z: [{}] h: [{}]", cx, cy, cz, new_heading);
		SendAppearanceEffect(78, 0, 0, 0, 0);
		SendAppearanceEffect(41, 0, 0, 0, 0);
	}

	/* Only feed real time updates when client is moving */
	if (positionUpdated) {
		/* Broadcast update to other clients */
		static EQApplicationPacket outapp(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
		PlayerPositionUpdateServer_Struct *position_update = (PlayerPositionUpdateServer_Struct *) outapp.pBuffer;

		MakeSpawnUpdate(position_update);

		if (gm_hide_me) {
			entity_list.QueueClientsStatus(this, &outapp, true, Admin(), AccountStatus::Max);
		} else {
			entity_list.QueueCloseClients(this, &outapp, true, RuleI(Range, ClientPositionUpdates), nullptr, true);
		}

		/* Always send position updates to group - send when beyond normal ClientPositionUpdate range */
		Group *group = GetGroup();
		Raid *raid = GetRaid();

		if (raid) {
			raid->QueueClients(this, &outapp, true, true, (RuleI(Range, ClientPositionUpdates) * -1));
		} else if (group) {
			group->QueueClients(this, &outapp, true, true, (RuleI(Range, ClientPositionUpdates) * -1));
		}
	}

	if (zone->watermap) {
		if (zone->watermap->InLiquid(glm::vec3(m_Position)) && IsMoving()) {
			CheckIncreaseSkill(EQ::skills::SkillSwimming, nullptr, -17);

			// Dismount horses when entering water
			if (GetHorseId() && RuleB(Character, DismountWater)) {
				SetHorseId(0);
				BuffFadeByEffect(SpellEffect::SummonHorse);
			}
		}
		CheckRegionTypeChanges();
	}

	CheckVirtualZoneLines();
}

void Client::Handle_OP_CombatAbility(const EQApplicationPacket *app)
{
	if (app->size != sizeof(CombatAbility_Struct)) {
		std::cout << "Wrong size on OP_CombatAbility. Got: " << app->size << ", Expected: " << sizeof(CombatAbility_Struct) << std::endl;
		return;
	}
	auto ca_atk = (CombatAbility_Struct *)app->pBuffer;
	OPCombatAbility(ca_atk);
	return;
}

void Client::Handle_OP_ConfirmDelete(const EQApplicationPacket* app)
{
	return;
}

void Client::Handle_OP_Consent(const EQApplicationPacket *app)
{
	if (app->size<64) {
		Consent_Struct* c = (Consent_Struct*)app->pBuffer;
		ConsentCorpses(c->name, false);
	}
}

void Client::Handle_OP_ConsentDeny(const EQApplicationPacket *app)
{
	if (app->size<64) {
		Consent_Struct* c = (Consent_Struct*)app->pBuffer;
		ConsentCorpses(c->name, true);
	}
}

void Client::Handle_OP_Consider(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Consider_Struct)) {
		LogDebug("Size mismatch in Consider expected [{}] got [{}]", sizeof(Consider_Struct), app->size);
		return;
	}

	auto *conin = (Consider_Struct*) app->pBuffer;
	auto *t     = entity_list.GetMob(conin->targetid);
	if (!t) {
		return;
	}

	if (parse->PlayerHasQuestSub(EVENT_CONSIDER)) {
		std::vector<std::any> args = { t };

		if (parse->EventPlayer(EVENT_CONSIDER, this, std::to_string(conin->targetid), 0, &args) == 1) {
			return;
		}
	}

	if (t->GetClass() == Class::LDoNTreasure) {
		Message(Chat::Yellow, fmt::format("{}", t->GetCleanName()).c_str());
		return;
	}

	auto outapp = new EQApplicationPacket(OP_Consider, sizeof(Consider_Struct));
	auto* con = (Consider_Struct*) outapp->pBuffer;
	con->playerid = GetID();
	con->targetid = conin->targetid;
	if (t->IsNPC()) {
		con->faction = GetFactionLevel(
			character_id,
			t->GetNPCTypeID(),
			GetFactionRace(),
			class_,
			deity,
			(t->IsNPC()) ? t->CastToNPC()->GetPrimaryFaction() : 0,
			t
		);
	} else {
		con->faction = 1;
	}

	con->level = GetLevelCon(t->GetLevel());

	if (ClientVersion() <= EQ::versions::ClientVersion::Titanium) {
		if (con->level == ConsiderColor::Gray) {
			con->level = ConsiderColor::Green;
		} else if (con->level == ConsiderColor::White) {
			con->level = ConsiderColor::WhiteTitanium;
		}
	}

	if (zone->IsPVPZone()) {
		if (!t->IsNPC()) {
			con->pvpcon = t->CastToClient()->GetPVP();
		}
	}

	// If we're feigned show NPC as indifferent
	if (t->IsNPC()) {
		if (GetFeigned()) {
			con->faction = FACTION_INDIFFERENTLY;
		}
	}

	if (!(con->faction == FACTION_SCOWLS)) {
		if (t->IsNPC()) {
			if (t->CastToNPC()->IsOnHatelist(this)) {
				con->faction = FACTION_THREATENINGLY;
			}
		}
	}

	if (con->faction == FACTION_APPREHENSIVELY) {
		con->faction = FACTION_SCOWLS;
	} else if (con->faction == FACTION_DUBIOUSLY) {
		con->faction = FACTION_THREATENINGLY;
	} else if (con->faction == FACTION_SCOWLS) {
		con->faction = FACTION_APPREHENSIVELY;
	} else if (con->faction == FACTION_THREATENINGLY) {
		con->faction = FACTION_DUBIOUSLY;
	}

	QueuePacket(outapp);
	// only wanted to check raid target once
	// and need con to still be around so, do it here!
	if (t->IsRaidTarget()) {
		uint32 color = 0;
		switch (con->level) {
			case ConsiderColor::Green:
				color = 2;
				break;
			case ConsiderColor::LightBlue:
				color = 10;
				break;
			case ConsiderColor::DarkBlue:
				color = 4;
				break;
			case ConsiderColor::WhiteTitanium:
			case ConsiderColor::White:
				color = 10;
				break;
			case ConsiderColor::Yellow:
				color = 15;
				break;
			case ConsiderColor::Red:
				color = 13;
				break;
			case ConsiderColor::Gray:
				color = 6;
				break;
		}

		if (ClientVersion() <= EQ::versions::ClientVersion::Titanium) {
			if (color == 6)	{
				color = 2;
			}
		}

		SendColoredText(color, std::string("This creature would take an army to defeat!"));
	}

	// this could be done better, but this is only called when you con so w/e
	// Shroud of Stealth has a special message
	if (improved_hidden && (!t->see_improved_hide && (t->SeeInvisible() || t->see_hide))) {
		MessageString(Chat::NPCQuestSay, SOS_KEEPS_HIDDEN); // we are trying to hide but they can see us
	} else if ((invisible || invisible_undead || hidden || invisible_animals) && !IsInvisible(t)) {
		MessageString(Chat::NPCQuestSay, SUSPECT_SEES_YOU);
	}

	safe_delete(outapp);

	return;
}

void Client::Handle_OP_ConsiderCorpse(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Consider_Struct)) {
		LogDebug("Size mismatch in Consider corpse expected [{}] got [{}]", sizeof(Consider_Struct), app->size);
		return;
	}

	auto* conin = (Consider_Struct*)app->pBuffer;
	auto* t = entity_list.GetCorpseByID(conin->targetid);
	if (!t) {
		return;
	}

	if (parse->PlayerHasQuestSub(EVENT_CONSIDER_CORPSE)) {
		std::vector<std::any> args = { t };

		if (parse->EventPlayer(EVENT_CONSIDER_CORPSE, this, std::to_string(conin->targetid), 0, &args)) {
			return;
		}
	}

	uint32 days, hours, minutes, seconds, remaining_time = 0;
	if (t && t->IsNPCCorpse()) {
		remaining_time = t->GetDecayTime();
		if (remaining_time != 0) {
			seconds = (remaining_time / 1000) % 60;
			minutes = (remaining_time / 60000) % 60;
			MessageString(
				Chat::NPCQuestSay,
				CORPSE_DECAY_TIME_MINUTE,
				std::to_string(minutes).c_str(),
				std::to_string(seconds).c_str()
			);
		} else {
			MessageString(Chat::NPCQuestSay, CORPSE_DECAY_NOW);
		}
	} else if (t && t->IsPlayerCorpse()) {
		remaining_time = t->GetRemainingRezTime();
		if (!t->IsRezzed()) {
			if (remaining_time > 0) {
				seconds = (remaining_time / 1000) % 60;
				minutes = (remaining_time / 60000) % 60;
				hours   = (remaining_time / 3600000) % 24;
				if (hours) {
					MessageString(
						Chat::White,
						CORPSE_REZ_TIME_HOUR,
						std::to_string(hours).c_str(),
						std::to_string(minutes).c_str(),
						std::to_string(seconds).c_str()
					);
				} else {
					MessageString(
						Chat::White,
						CORPSE_REZ_TIME_MINUTE,
						std::to_string(minutes).c_str(),
						std::to_string(seconds).c_str()
					);
				}
				hours = 0;
			} else {
				MessageString(Chat::White, CORPSE_TOO_OLD);
			}
		} else {
			Message(Chat::White, "This corpse has already accepted a resurrection.");
		}

		remaining_time = t->GetDecayTime();
		if (remaining_time != 0) {
			seconds = (remaining_time / 1000) % 60;
			minutes = (remaining_time / 60000) % 60;
			hours   = (remaining_time / 3600000) % 24;
			days    = remaining_time / 86400000;

			if (days) {
				MessageString(
					Chat::White,
					CORPSE_DECAY_TIME_DAY,
					std::to_string(days).c_str(),
					std::to_string(hours).c_str(),
					std::to_string(minutes).c_str(),
					std::to_string(seconds).c_str()
				);
			} else if (hours) {
				MessageString(
					Chat::White,
					CORPSE_DECAY_TIME_HOUR,
					std::to_string(hours).c_str(),
					std::to_string(minutes).c_str(),
					std::to_string(seconds).c_str()
				);
			} else {
				MessageString(
					Chat::White,
					CORPSE_DECAY_TIME_MINUTE,
					std::to_string(minutes).c_str(),
					std::to_string(seconds).c_str()
				);
			}
		} else {
			MessageString(Chat::White, CORPSE_DECAY_NOW);
		}
	}
}

void Client::Handle_OP_Consume(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Consume_Struct))
	{
		LogError("OP size error: OP_Consume expected:[{}] got:[{}]", sizeof(Consume_Struct), app->size);
		return;
	}
	Consume_Struct* pcs = (Consume_Struct*)app->pBuffer;
	if (pcs->type == 0x01)
	{
		if (m_pp.hunger_level > 6000)
		{
			EQApplicationPacket *outapp = nullptr;
			outapp = new EQApplicationPacket(OP_Stamina, sizeof(Stamina_Struct));
			Stamina_Struct* sta = (Stamina_Struct*)outapp->pBuffer;
			sta->food = m_pp.hunger_level > 6000 ? 6000 : m_pp.hunger_level;
			sta->water = m_pp.thirst_level > 6000 ? 6000 : m_pp.thirst_level;

			QueuePacket(outapp);
			safe_delete(outapp);
			return;
		}
	}
	else if (pcs->type == 0x02)
	{
		if (m_pp.thirst_level > 6000)
		{
			EQApplicationPacket *outapp = nullptr;
			outapp = new EQApplicationPacket(OP_Stamina, sizeof(Stamina_Struct));
			Stamina_Struct* sta = (Stamina_Struct*)outapp->pBuffer;
			sta->food = m_pp.hunger_level > 6000 ? 6000 : m_pp.hunger_level;
			sta->water = m_pp.thirst_level > 6000 ? 6000 : m_pp.thirst_level;

			QueuePacket(outapp);
			safe_delete(outapp);
			return;
		}
	}

	EQ::ItemInstance *myitem = GetInv().GetItem(pcs->slot);
	if (myitem == nullptr) {
		LogError("Consuming from empty slot [{}]", pcs->slot);
		return;
	}

	const EQ::ItemData* eat_item = myitem->GetItem();
	if (pcs->type == 0x01) {
		Consume(eat_item, EQ::item::ItemTypeFood, pcs->slot, (pcs->auto_consumed == 0xffffffff));
	}
	else if (pcs->type == 0x02) {
		Consume(eat_item, EQ::item::ItemTypeDrink, pcs->slot, (pcs->auto_consumed == 0xffffffff));
	}
	else {
		LogError("OP_Consume: unknown type, type:[{}]", (int)pcs->type);
		return;
	}
	if (m_pp.hunger_level > 50000)
		m_pp.hunger_level = 50000;
	if (m_pp.thirst_level > 50000)
		m_pp.thirst_level = 50000;
	EQApplicationPacket *outapp = nullptr;
	outapp = new EQApplicationPacket(OP_Stamina, sizeof(Stamina_Struct));
	Stamina_Struct* sta = (Stamina_Struct*)outapp->pBuffer;
	sta->food = m_pp.hunger_level > 6000 ? 6000 : m_pp.hunger_level;
	sta->water = m_pp.thirst_level > 6000 ? 6000 : m_pp.thirst_level;

	QueuePacket(outapp);
	safe_delete(outapp);
	return;
}

void Client::Handle_OP_ControlBoat(const EQApplicationPacket *app)
{
	if (app->size != sizeof(ControlBoat_Struct)) {
		LogError("Wrong size: OP_ControlBoat, size=[{}], expected [{}]", app->size, sizeof(ControlBoat_Struct));
		return;
	}

	ControlBoat_Struct* cbs = (ControlBoat_Struct*)app->pBuffer;
	Mob* boat = entity_list.GetMob(cbs->boatId);
	if (!boat) {

		LogError("Player tried to take control of non-existent boat (char_id: %u, boat_eid: %u)", CharacterID(), cbs->boatId);
		return;	// do nothing if the boat isn't valid
	}

	if (!boat->IsNPC() || !boat->IsControllableBoat())
	{
		auto message = fmt::format("OP_Control Boat was sent against {} which is of race {}", boat->GetName(), boat->GetRace());
		RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});
		return;
	}

	if (cbs->TakeControl) {
		// this uses the boat's target to indicate who has control of it. It has to check hate to make sure the boat isn't actually attacking anyone.
		if (!boat->GetTarget() || (boat->GetTarget() == this && boat->GetHateAmount(this) == 0)) {
			boat->SetTarget(this);
		}
		else {

			MessageString(Chat::Red, IN_USE);
			return;
		}
	}
	else {
		if (boat->GetTarget() == this) {
			boat->SetTarget(nullptr);
		}
	}

	// client responds better to a packet echo than an empty op
	QueuePacket(app);

	// have the boat signal itself, so quests can be triggered by boat use
	boat->CastToNPC()->SignalNPC(0);
}

void Client::Handle_OP_CorpseDrag(const EQApplicationPacket *app)
{
	if (DraggedCorpses.size() >= (unsigned int)RuleI(Character, MaxDraggedCorpses))
	{
		MessageString(Chat::Red, CORPSEDRAG_LIMIT);
		return;
	}

	VERIFY_PACKET_LENGTH(OP_CorpseDrag, app, CorpseDrag_Struct);

	CorpseDrag_Struct *cds = (CorpseDrag_Struct*)app->pBuffer;

	Mob* corpse = entity_list.GetMob(cds->CorpseName);

	if (!corpse || !corpse->IsPlayerCorpse() || corpse->CastToCorpse()->IsBeingLooted())
		return;

	Client *c = entity_list.FindCorpseDragger(corpse->GetID());

	if (c)
	{
		if (c == this)
			MessageString(Chat::DefaultText, CORPSEDRAG_ALREADY, corpse->GetCleanName());
		else
			MessageString(Chat::DefaultText, CORPSEDRAG_SOMEONE_ELSE, corpse->GetCleanName());

		return;
	}

	if (!corpse->CastToCorpse()->Summon(this, false, true))
		return;

	DraggedCorpses.emplace_back(std::pair<std::string, uint16>(cds->CorpseName, corpse->GetID()));

	MessageString(Chat::DefaultText, CORPSEDRAG_BEGIN, cds->CorpseName);
}

void Client::Handle_OP_CorpseDrop(const EQApplicationPacket *app)
{
	if (app->size == 1)
	{
		MessageString(Chat::DefaultText, CORPSEDRAG_STOPALL);
		ClearDraggedCorpses();
		return;
	}

	for (auto Iterator = DraggedCorpses.begin(); Iterator != DraggedCorpses.end(); ++Iterator)
	{
		if (!strcasecmp(Iterator->first.c_str(), (const char *)app->pBuffer))
		{
			MessageString(Chat::DefaultText, CORPSEDRAG_STOP);
			Iterator = DraggedCorpses.erase(Iterator);
			return;
		}
	}
}

void Client::Handle_OP_CrashDump(const EQApplicationPacket *app)
{
	return;
}

void Client::Handle_OP_CreateObject(const EQApplicationPacket *app)
{
	if (EQEmuLogSys::Instance()->log_settings[Logs::Inventory].is_category_enabled)
		LogInventory("Handle_OP_CreateObject() [psize: [{}]] [{}]", app->size, DumpPacketToString(app).c_str());

	DropItem(EQ::invslot::slotCursor);
	return;
}

void Client::Handle_OP_CrystalCreate(const EQApplicationPacket *app)
{
	VERIFY_PACKET_LENGTH(OP_CrystalCreate, app, CrystalReclaim_Struct);
	auto *cr = (CrystalReclaim_Struct *) app->pBuffer;

	const uint32 quantity   = cr->amount;
	const bool   is_radiant = cr->type == CrystalReclaimTypes::Radiant;
	const bool   is_ebon    = cr->type == CrystalReclaimTypes::Ebon;

	if (!is_radiant && !is_ebon) {
		return;
	}

	if (quantity < 1) {
		return;
	}

	// Check: Valid client state to make request.
	// In this situation the client is either desynced or attempting an exploit.
	const uint32 current_quantity = is_radiant ? GetRadiantCrystals() : GetEbonCrystals();
	if (!current_quantity) {
		return;
	}

	// Prevent the client from creating more than they have.
	uint32 amount  = EQ::ClampUpper(quantity, current_quantity);
	const uint32 item_id = is_radiant ? RuleI(Zone, RadiantCrystalItemID) : RuleI(Zone, EbonCrystalItemID);

	const auto item = database.GetItem(item_id);
	// Prevent pulling more than max stack size or 1,000 (if stackable), whichever is lesser
	const uint32 max_reclaim_amount = EQ::Clamp(
		item && item->Stackable ? item->StackSize : ItemStackSizeConstraint::Minimum,
		ItemStackSizeConstraint::Minimum,
		ItemStackSizeConstraint::Maximum
	);
	if (amount > max_reclaim_amount) {
		amount = max_reclaim_amount;
	}

	const bool success = SummonItem(item_id, amount);
	if (!success) {
		return;
	}

	if (is_ebon) {
		RemoveEbonCrystals(amount, true);
	} else if (is_radiant) {
		RemoveRadiantCrystals(amount, true);
	}
}

void Client::Handle_OP_CrystalReclaim(const EQApplicationPacket *app)
{
	const uint32 ebon    = NukeItem(RuleI(Zone, EbonCrystalItemID), invWhereWorn | invWherePersonal | invWhereCursor);
	const uint32 radiant = NukeItem(RuleI(Zone, RadiantCrystalItemID), invWhereWorn | invWherePersonal | invWhereCursor);
	if ((ebon + radiant) > 0) {
		AddEbonCrystals(ebon, true);
		AddRadiantCrystals(radiant, true);
	}
}

void Client::Handle_OP_Damage(const EQApplicationPacket *app)
{
	if (app->size != sizeof(CombatDamage_Struct)) {
		LogError("Received invalid sized OP_Damage: got [{}], expected [{}]", app->size, sizeof(CombatDamage_Struct));
		DumpPacket(app);
		return;
	}

	// Broadcast to other clients
	CombatDamage_Struct* damage = (CombatDamage_Struct*)app->pBuffer;
	//dont send to originator of falling damage packets
	entity_list.QueueClients(this, app, (damage->type == DamageTypeFalling));
	return;
}

void Client::Handle_OP_Death(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Death_Struct))
		return;

	Death_Struct* ds = (Death_Struct*)app->pBuffer;

	//I think this attack_skill value is really a value from SkillDamageTypes...
	if (ds->attack_skill > EQ::skills::HIGHEST_SKILL) {
		return;
	}

	if (GetHP() > 0)
		return;

	Mob* killer = entity_list.GetMob(ds->killer_id);
	Death(killer, ds->damage, ds->spell_id, (EQ::skills::SkillType)ds->attack_skill);
	return;
}

void Client::Handle_OP_DelegateAbility(const EQApplicationPacket *app)
{

	if (app->size != sizeof(DelegateAbility_Struct))
	{
		LogDebug("Size mismatch in OP_DelegateAbility expected [{}] got [{}]", sizeof(DelegateAbility_Struct), app->size);

		DumpPacket(app);

		return;
	}

	DelegateAbility_Struct* das = (DelegateAbility_Struct*)app->pBuffer;

	Group *g = GetGroup();

	if (!g) return;

	switch (das->DelegateAbility)
	{
	case 0:
	{
		g->DelegateMainAssist(das->Name);
		break;
	}
	case 1:
	{
		g->DelegateMarkNPC(das->Name);
		break;
	}
	case 2:
	{
		g->DelegateMainTank(das->Name);
		break;
	}
	case 3:
	{
		g->DelegatePuller(das->Name);
		break;
	}
	default:
		break;
	}
}

void Client::Handle_OP_DeleteItem(const EQApplicationPacket *app)
{
	if (app->size != sizeof(DeleteItem_Struct)) {
		std::cout << "Wrong size on OP_DeleteItem. Got: " << app->size << ", Expected: " << sizeof(DeleteItem_Struct) << std::endl;
		return;
	}

	DeleteItem_Struct* alc = (DeleteItem_Struct*)app->pBuffer;
	const EQ::ItemInstance *inst = GetInv().GetItem(alc->from_slot);
	if (inst && inst->GetItem()->ItemType == EQ::item::ItemTypeAlcohol) {
		entity_list.MessageCloseString(this, true, 50, 0, DRINKING_MESSAGE, GetName(), inst->GetItem()->Name);
		CheckIncreaseSkill(EQ::skills::SkillAlcoholTolerance, nullptr, 25);

		int16 AlcoholTolerance = GetSkill(EQ::skills::SkillAlcoholTolerance);
		int16 IntoxicationIncrease;

		if (ClientVersion() < EQ::versions::ClientVersion::SoD)
			IntoxicationIncrease = (200 - AlcoholTolerance) * 30 / 200 + 10;
		else
			IntoxicationIncrease = (270 - AlcoholTolerance) * 0.111111108 + 10;

		if (IntoxicationIncrease < 0)
			IntoxicationIncrease = 1;

		SetIntoxication(GetIntoxication()+IntoxicationIncrease);

		if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::ITEM_DESTROY) && inst->GetItem()) {
			auto e = PlayerEvent::DestroyItemEvent{
				.item_id      = inst->GetItem()->ID,
				.item_name    = inst->GetItem()->Name,
				.charges      = inst->GetCharges(),
				.augment_1_id = inst->GetAugmentItemID(0),
				.augment_2_id = inst->GetAugmentItemID(1),
				.augment_3_id = inst->GetAugmentItemID(2),
				.augment_4_id = inst->GetAugmentItemID(3),
				.augment_5_id = inst->GetAugmentItemID(4),
				.augment_6_id = inst->GetAugmentItemID(5),
				.attuned      = inst->IsAttuned(),
				.reason       = "Client deleted"
			};

			RecordPlayerEventLog(PlayerEvent::ITEM_DESTROY, e);
		}
	}

	DeleteItemInInventory(alc->from_slot, 1);
}

void Client::Handle_OP_DeleteSpawn(const EQApplicationPacket *app)
{
	// The client will send this with his id when he zones, maybe when he disconnects too?
	//eqs->RemoveData(); // Flushing the queue of packet data to allow for proper zoning

	//just make sure this gets out
	auto outapp = new EQApplicationPacket(OP_LogoutReply);
	FastQueuePacket(&outapp);

	outapp = new EQApplicationPacket(OP_DeleteSpawn, sizeof(EntityId_Struct));
	EntityId_Struct* eid = (EntityId_Struct*)outapp->pBuffer;
	eid->entity_id = GetID();

	entity_list.QueueClients(this, outapp, false);
	safe_delete(outapp);

	hate_list.RemoveEntFromHateList(CastToMob());

	Disconnect();
	return;
}

void Client::Handle_OP_Disarm(const EQApplicationPacket *app) {
	if (dead || bZoning) return;
	if (!HasSkill(EQ::skills::SkillDisarm))
		return;

	if (app->size != sizeof(Disarm_Struct)) {
		LogSkills("Size mismatch for Disarm_Struct packet");
		return;
	}

	Disarm_Struct *disarm = (Disarm_Struct *)app->pBuffer;

	if (!p_timers.Expired(&database, pTimerCombatAbility2, false)) {
		Message(Chat::Red, "Ability recovery time not yet met.");
		return;
	}

	p_timers.Start(pTimerCombatAbility2, 8);

	BreakInvis();
	Mob* pmob = entity_list.GetMob(disarm->source);
	Mob* tmob = entity_list.GetMob(disarm->target);
	if (!pmob || !tmob)
		return;
	if (pmob->GetID() != GetID()) {
		// Client sent a disarm request with an originator ID not matching their own ID.
		auto message = fmt::format("Player {} ({}) sent OP_Disarm with source ID of: {}", GetCleanName(), GetID(), pmob->GetID());
		RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});

		return;
	}
	// No disarm on corpses
	if (tmob->IsCorpse())
		return;
	// No target
	if (!GetTarget())
		return;
	// Targets don't match (possible hack, but not flagging)
	if (GetTarget() != tmob) {
		return;
	}
	// Too far away
	if (pmob->CalculateDistance(GetTarget()->GetX(), GetTarget()->GetY(), GetTarget()->GetZ()) > 400)
		return;

	// Can't see mob
	//if (tmob->BehindMob(pmob))
	//	return;
	// How can we disarm someone if we are feigned.
	if (GetFeigned())
		return;
	// We can't disarm someone who is feigned.
	if (tmob->IsClient() && tmob->CastToClient()->GetFeigned())
		return;
	if (GetTarget() == tmob && pmob == CastToMob() &&
		disarm->skill == GetSkill(EQ::skills::SkillDisarm) && IsAttackAllowed(tmob)) {
		int p_level = pmob->GetLevel() ? pmob->GetLevel() : 1;
		int t_level = tmob->GetLevel() ? tmob->GetLevel() : 1;
		// We have a disarmable target - sucess or fail, we always aggro the mob
		if (tmob->IsNPC()) {
			if (!tmob->CheckAggro(pmob)) {
				zone->AddAggroMob();
				tmob->AddToHateList(pmob, p_level);
			}
			else {
				tmob->AddToHateList(pmob, p_level / 3);
			}
		}
		int chance = GetSkill(EQ::skills::SkillDisarm); // (1% @ 0 skill) (11% @ 200 skill) - against even con
		chance /= 2;
		chance += 10;
		// Modify chance based on level difference
		float lvl_mod = p_level / t_level;
		chance *= lvl_mod;
		if (chance > 300)
			chance = 300; // max chance of 30%
		if (tmob->IsNPC()) {
			tmob->CastToNPC()->Disarm(this, chance);
		}
		else if (tmob->IsClient()) {
			tmob->CastToClient()->Disarm(this, chance);
		}
		return;
	}
	// Trying to disarm something we can't disarm
	MessageString(Chat::Skills, DISARM_NO_TARGET);

	return;
}

void Client::Handle_OP_DeleteSpell(const EQApplicationPacket *app)
{
	if (app->size != sizeof(DeleteSpell_Struct))
		return;

	EQApplicationPacket* outapp = app->Copy();
	DeleteSpell_Struct* dss = (DeleteSpell_Struct*)outapp->pBuffer;

	if (dss->spell_slot < 0 || dss->spell_slot >= EQ::spells::DynamicLookup(ClientVersion(), GetGM())->SpellbookSize)
		return;

	if (m_pp.spell_book[dss->spell_slot] != SPELLBOOK_UNKNOWN) {
		m_pp.spell_book[dss->spell_slot] = SPELLBOOK_UNKNOWN;
		database.DeleteCharacterSpell(CharacterID(), dss->spell_slot);
		dss->success = 1;
	}
	else
		dss->success = 0;

	FastQueuePacket(&outapp);
	return;
}

void Client::Handle_OP_DisarmTraps(const EQApplicationPacket *app)
{
	if (!HasSkill(EQ::skills::SkillDisarmTraps))
		return;

	if (!p_timers.Expired(&database, pTimerDisarmTraps, false)) {
		Message(Chat::Red, "Ability recovery time not yet met.");
		return;
	}

	int reuse = DisarmTrapsReuseTime - GetSkillReuseTime(EQ::skills::SkillDisarmTraps);

	if (reuse < 1)
		reuse = 1;

	p_timers.Start(pTimerDisarmTraps, reuse - 1);

	uint8 success = SKILLUP_FAILURE;
	float curdist = 0;
	Trap* trap = entity_list.FindNearbyTrap(this, 250, curdist, true);
	if (trap && trap->detected)
	{
		float max_radius = (trap->radius * 2) * (trap->radius * 2); // radius is used to trigger trap, so disarm radius should be a bit bigger.
		Log(Logs::General, Logs::Traps, "%s is attempting to disarm trap %d. Curdist is %0.2f maxdist is %0.2f", GetName(), trap->trap_id, curdist, max_radius);
		if (curdist <= max_radius)
		{
			int uskill = GetSkill(EQ::skills::SkillDisarmTraps);
			if ((zone->random.Int(0, 49) + uskill) >= (zone->random.Int(0, 49) + trap->skill))
			{
				success = SKILLUP_SUCCESS;
				MessageString(Chat::Skills, DISARMED_TRAP);
				trap->disarmed = true;
				Log(Logs::General, Logs::Traps, "Trap %d is disarmed.", trap->trap_id);
				trap->UpdateTrap();
			}
			else
			{
				int fail_rate = 25;
				int trap_circumvention = spellbonuses.TrapCircumvention + itembonuses.TrapCircumvention + aabonuses.TrapCircumvention;
				fail_rate -= fail_rate * trap_circumvention / 100;
				MessageString(Chat::Skills, FAIL_DISARM_DETECTED_TRAP);
				if (zone->random.Int(0, 99) < fail_rate) {
					trap->Trigger(this);
				}
			}
			CheckIncreaseSkill(EQ::skills::SkillDisarmTraps, nullptr);
			return;
		}
		else
		{
			MessageString(Chat::Skills, TRAP_TOO_FAR);
		}
	}
	else
	{
		MessageString(Chat::Skills, LDON_SENSE_TRAP2);
	}

	return;
}

void Client::Handle_OP_DoGroupLeadershipAbility(const EQApplicationPacket *app)
{

	if (app->size != sizeof(DoGroupLeadershipAbility_Struct)) {

		LogDebug("Size mismatch in OP_DoGroupLeadershipAbility expected [{}] got [{}]", sizeof(DoGroupLeadershipAbility_Struct), app->size);

		DumpPacket(app);

		return;
	}

	DoGroupLeadershipAbility_Struct* dglas = (DoGroupLeadershipAbility_Struct*)app->pBuffer;

	switch (dglas->Ability)
	{
	case GroupLeadershipAbility_MarkNPC:
	{
		if (GetTarget())
		{
			Group* g = GetGroup();
			if (g)
				g->MarkNPC(GetTarget(), dglas->Parameter);
		}
		break;
	}

	case groupAAInspectBuffs:
	{
		Mob *Target = GetTarget();

		if (!Target || !Target->IsClient())
			return;

		if (IsRaidGrouped()) {
			Raid *raid = GetRaid();
			if (!raid)
				return;
			uint32 group_id = raid->GetGroup(this);
			if (group_id > 11 || raid->GroupCount(group_id) < 3)
				return;
			Target->CastToClient()->InspectBuffs(this, raid->GetLeadershipAA(groupAAInspectBuffs, group_id));
			return;
		}

		Group *g = GetGroup();

		if (!g || (g->GroupCount() < 3))
			return;

		Target->CastToClient()->InspectBuffs(this, g->GetLeadershipAA(groupAAInspectBuffs));

		break;
	}

	case RaidLeadershipAbility_MainAssist:
	{
		//This is not needed as it executes from opcode 0x2b33 which is sent
		//with this opcode.
		//if (GetTarget())
		//{
		//	Raid* r = GetRaid();
		//	if (r)
		//	{
		//		r->DelegateAbility(GetTarget()->CastToClient()->GetName());
		//	}
		//}
		break;
	}
	case RaidLeadershipAbility_MarkNPC:
	{
		if (GetTarget() && GetTarget()->IsMob()) {
			Raid* r = GetRaid();
			if (r) {
				r->RaidMarkNPC(this, dglas->Parameter);
			}
		}
		break;

	}
	default:
		LogDebug("Got unhandled OP_DoGroupLeadershipAbility Ability: [{}] Parameter: [{}]", dglas->Ability, dglas->Parameter);
		break;
	}
}

void Client::Handle_OP_DuelDecline(const EQApplicationPacket *app)
{
	if (app->size != sizeof(DuelResponse_Struct)) {
		return;
	}

	DuelResponse_Struct* ds = (DuelResponse_Struct*)app->pBuffer;
	if (!ds->target_id || !ds->entity_id) {
		return;
	}

	Entity* entity = entity_list.GetID(ds->target_id);
	Entity* initiator = entity_list.GetID(ds->entity_id);
	if (!entity->IsClient() || !initiator->IsClient()) {
		return;
	}

	entity->CastToClient()->SetDuelTarget(0);
	entity->CastToClient()->SetDueling(false);
	initiator->CastToClient()->SetDuelTarget(0);
	initiator->CastToClient()->SetDueling(false);
	if (GetID() == initiator->GetID()) {
		entity->CastToClient()->MessageString(Chat::NPCQuestSay, DUEL_DECLINE, initiator->GetName());
	} else {
		initiator->CastToClient()->MessageString(Chat::NPCQuestSay, DUEL_DECLINE, entity->GetName());
	}
	return;
}

void Client::Handle_OP_DuelAccept(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Duel_Struct)) {
		return;
	}

	Duel_Struct* ds = (Duel_Struct*)app->pBuffer;
	if (!ds->duel_initiator || !ds->duel_target) {
		return;
	}

	Entity* entity = entity_list.GetID(ds->duel_target);
	Entity* initiator = entity_list.GetID(ds->duel_initiator);
	if (!entity || !initiator) {
		return;
	}

	if (GetDuelTarget() != ds->duel_initiator || IsDueling()) {
		return;
	}

	if (entity == this && initiator->IsClient()) {
		auto outapp = new EQApplicationPacket(OP_RequestDuel, sizeof(Duel_Struct));
		Duel_Struct* ds2 = (Duel_Struct*)outapp->pBuffer;

		ds2->duel_initiator = entity->GetID();
		ds2->duel_target = entity->GetID();
		initiator->CastToClient()->QueuePacket(outapp);

		outapp->SetOpcode(OP_DuelAccept);
		ds2->duel_initiator = initiator->GetID();

		initiator->CastToClient()->QueuePacket(outapp);

		QueuePacket(outapp);
		SetDueling(true);
		initiator->CastToClient()->SetDueling(true);
		SetDuelTarget(ds->duel_initiator);
		safe_delete(outapp);

		if (IsCasting()) {
			InterruptSpell();
		}

		if (initiator->CastToClient()->IsCasting()) {
			initiator->CastToClient()->InterruptSpell();
		}
	}
	return;
}

void Client::Handle_OP_DumpName(const EQApplicationPacket *app)
{
	return;
}

void Client::Handle_OP_Dye(const EQApplicationPacket *app)
{
	if (app->size != sizeof(EQ::TintProfile))
		printf("Wrong size of DyeStruct, Got: %i, Expected: %zu\n", app->size, sizeof(EQ::TintProfile));
	else {
		EQ::TintProfile* dye = (EQ::TintProfile*)app->pBuffer;
		DyeArmor(dye);
	}
	return;
}

void Client::Handle_OP_DzAddPlayer(const EQApplicationPacket *app)
{
	auto expedition = GetExpedition();
	if (expedition)
	{
		auto dzcmd = reinterpret_cast<ExpeditionCommand_Struct*>(app->pBuffer);
		expedition->DzAddPlayer(this, dzcmd->name);
	}
	else
	{
		// message string 8271 (not in emu clients) is the only /dz command that sends an error if no active expedition
		Message(Chat::System, "You could not use this command because you are not currently assigned to a dynamic zone.");
	}
}

void Client::Handle_OP_DzChooseZoneReply(const EQApplicationPacket *app)
{
	auto dzmsg = reinterpret_cast<DynamicZoneChooseZoneReply_Struct*>(app->pBuffer);

	LogDynamicZones("Character [{}] chose DynamicZone [{}]:[{}] type: [{}] with system id: [{}]",
		CharacterID(), dzmsg->dz_zone_id, dzmsg->dz_instance_id, dzmsg->dz_type, dzmsg->unknown_id2);

	if (!dzmsg->dz_instance_id || !database.VerifyInstanceAlive(dzmsg->dz_instance_id, CharacterID()))
	{
		// live just no-ops this without a message
		LogDynamicZones("Character [{}] chose invalid DynamicZone [{}]:[{}] or is no longer a member",
			CharacterID(), dzmsg->dz_zone_id, dzmsg->dz_instance_id);
		return;
	}

	auto client_dzs = GetDynamicZones();
	auto it = std::find_if(client_dzs.begin(), client_dzs.end(), [&](const DynamicZone* dz) {
		return dz->IsSameDz(dzmsg->dz_zone_id, dzmsg->dz_instance_id); });

	if (it != client_dzs.end())
	{
		DynamicZoneLocation loc = (*it)->GetZoneInLocation();
		ZoneMode zone_mode = (*it)->HasZoneInLocation() ? ZoneMode::ZoneSolicited : ZoneMode::ZoneToSafeCoords;
		MovePC(dzmsg->dz_zone_id, dzmsg->dz_instance_id, loc.x, loc.y, loc.z, loc.heading, 0, zone_mode);
	}
}

void Client::Handle_OP_DzExpeditionInviteResponse(const EQApplicationPacket *app)
{
	auto expedition = DynamicZone::FindDynamicZoneByID(m_dz_invite.dz_id);
	std::string swap_name = m_dz_invite.swap_name;
	m_dz_invite = {}; // clear before re-validating

	if (expedition)
	{
		auto dzmsg = reinterpret_cast<ExpeditionInviteResponse_Struct*>(app->pBuffer);
		expedition->DzInviteResponse(this, dzmsg->accepted, swap_name);
	}
}

void Client::Handle_OP_DzListTimers(const EQApplicationPacket *app)
{
	DzListTimers();
}

void Client::Handle_OP_DzMakeLeader(const EQApplicationPacket *app)
{
	auto expedition = GetExpedition();
	if (expedition)
	{
		auto dzcmd = reinterpret_cast<ExpeditionCommand_Struct*>(app->pBuffer);
		expedition->DzMakeLeader(this, dzcmd->name);
	}
}

void Client::Handle_OP_DzPlayerList(const EQApplicationPacket *app)
{
	auto expedition = GetExpedition();
	if (expedition) {
		expedition->DzPlayerList(this);
	}
}

void Client::Handle_OP_DzRemovePlayer(const EQApplicationPacket *app)
{
	auto expedition = GetExpedition();
	if (expedition)
	{
		auto dzcmd = reinterpret_cast<ExpeditionCommand_Struct*>(app->pBuffer);
		expedition->DzRemovePlayer(this, dzcmd->name);
	}
}

void Client::Handle_OP_DzSwapPlayer(const EQApplicationPacket *app)
{
	auto expedition = GetExpedition();
	if (expedition)
	{
		auto dzcmd = reinterpret_cast<ExpeditionCommandSwap_Struct*>(app->pBuffer);
		expedition->DzSwapPlayer(this, dzcmd->rem_player_name, dzcmd->add_player_name);
	}
}

void Client::Handle_OP_DzQuit(const EQApplicationPacket *app)
{
	auto expedition = GetExpedition();
	if (expedition) {
		expedition->DzQuit(this);
	}
}

void Client::Handle_OP_Emote(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Emote_Struct)) {
		LogError("Received invalid sized OP_Emote: got [{}], expected [{}]", app->size, sizeof(Emote_Struct));
		DumpPacket(app);
		return;
	}

	// Calculate new packet dimensions
	Emote_Struct* in = (Emote_Struct*)app->pBuffer;
	in->message[1023] = '\0';

	const char* name = GetName();
	uint32 len_name = strlen(name);
	uint32 len_msg = strlen(in->message);
	// crash protection -- cheater
	if (len_msg > 512) {
		in->message[512] = '\0';
		len_msg = 512;
	}
	uint32 len_packet = sizeof(in->type) + len_name
		+ len_msg + 1;

	// Construct outgoing packet
	auto outapp = new EQApplicationPacket(OP_Emote, len_packet);
	Emote_Struct* out = (Emote_Struct*)outapp->pBuffer;
	out->type = in->type;
	memcpy(out->message, name, len_name);
	memcpy(&out->message[len_name], in->message, len_msg);

	/*
	if (target && target->IsClient()) {
	entity_list.QueueCloseClients(this, outapp, false, 100, target);

	cptr = outapp->pBuffer + 2;

	// not sure if live does this or not. thought it was a nice feature, but would take a lot to
	// clean up grammatical and other errors. Maybe with a regex parser...
	replacestr((char *)cptr, target->GetName(), "you");
	replacestr((char *)cptr, " he", " you");
	replacestr((char *)cptr, " she", " you");
	replacestr((char *)cptr, " him", " you");
	replacestr((char *)cptr, " her", " you");
	target->CastToClient()->QueuePacket(outapp);

	}
	else
	*/
	entity_list.QueueCloseClients(this, outapp, true, RuleI(Range, Emote), 0, true, FilterSocials);

	safe_delete(outapp);
	return;
}

void Client::Handle_OP_EndLootRequest(const EQApplicationPacket *app)
{
	if (app->size != sizeof(uint32)) {
		std::cout << "Wrong size: OP_EndLootRequest, size=" << app->size << ", expected " << sizeof(uint32) << std::endl;
		return;
	}

	SetLooting(0);

	Entity* entity = entity_list.GetID(*((uint16*)app->pBuffer));
	if (entity == 0) {
		Message(Chat::Red, "Error: OP_EndLootRequest: Corpse not found (ent = 0)");
		if (ClientVersion() >= EQ::versions::ClientVersion::SoD)
			Corpse::SendEndLootErrorPacket(this);
		else
			Corpse::SendLootReqErrorPacket(this);
		return;
	}
	else if (!entity->IsCorpse()) {
		Message(Chat::Red, "Error: OP_EndLootRequest: Corpse not found (!entity->IsCorpse())");
		Corpse::SendLootReqErrorPacket(this);
		return;
	}
	else {
		entity->CastToCorpse()->EndLoot(this, app);
	}
	return;
}

void Client::Handle_OP_EnvDamage(const EQApplicationPacket *app)
{
	if (!ClientFinishedLoading()) {
		SetHP(GetHP() - 1);
		return;
	}

	if (app->size != sizeof(EnvDamage2_Struct)) {
		LogError("Received invalid sized OP_EnvDamage: got [{}], expected [{}]", app->size, sizeof(EnvDamage2_Struct));
		DumpPacket(app);
		return;
	}

	EnvDamage2_Struct* ed = (EnvDamage2_Struct*)app->pBuffer;
	auto damage = ed->damage;

	if (GetEnvironmentDamageModifier()) {
		damage = static_cast<int32>(damage) + (static_cast<int32>(damage) * GetEnvironmentDamageModifier() / 100);
	}

	if (ed->dmgtype == EQ::constants::EnvironmentalDamage::Falling) {
		if (zone->HasWaterMap()) {
			auto target_position = glm::vec3(GetX(), GetY(), GetZ());
			if (zone->watermap->InLiquid(target_position)) {
				return;
			}
		}

		uint32 mod = spellbonuses.ReduceFallDamage + itembonuses.ReduceFallDamage + aabonuses.ReduceFallDamage;
		damage -= damage * mod / 100;
	}

	if (damage < 0) {
		damage = 31337;
	}

	if (GetGM()) {
		Message(
			Chat::Red,
			fmt::format(
				"Your GM status protects you from {} points of {} (Type {}) damage.",
				damage,
				EQ::constants::GetEnvironmentalDamageName(ed->dmgtype),
				ed->dmgtype
			).c_str()
		);
		SetHP(GetHP() - 1);//needed or else the client wont acknowledge
		return;
	} else if (GetInvul()) {
		Message(
			Chat::Red,
			fmt::format(
				"Your invulnerability protects you from {} points of {} (Type {}) damage.",
				damage,
				EQ::constants::GetEnvironmentalDamageName(ed->dmgtype),
				ed->dmgtype
			).c_str()
		);
		SetHP(GetHP() - 1);//needed or else the client wont acknowledge
		return;
	}
	else if (GetInvulnerableEnvironmentDamage()) {
		SetHP(GetHP() - 1);
		return;
	} else if (zone->GetZoneID() == Zones::TUTORIAL || zone->GetZoneID() == Zones::LOAD) { // Hard coded tutorial and load zones for no fall damage
		return;
	} else {
		SetHP(GetHP() - (damage * RuleR(Character, EnvironmentDamageMulipliter)));

		if (parse->PlayerHasQuestSub(EVENT_ENVIRONMENTAL_DAMAGE)) {
			int         final_damage  = (damage * RuleR(Character, EnvironmentDamageMulipliter));
			const auto& export_string = fmt::format(
				"{} {} {}",
				ed->damage,
				ed->dmgtype,
				final_damage
			);
			parse->EventPlayer(EVENT_ENVIRONMENTAL_DAMAGE, this, export_string, 0);
		}

		if (ed->dmgtype == EQ::constants::EnvironmentalDamage::Trap) {
			BreakInvisibleSpells();
			CancelSneakHide();
		}
	}

	if (GetHP() <= 0) {
		Death(0, 32000, SPELL_UNKNOWN, EQ::skills::SkillHandtoHand);
	}
	SendHPUpdate();
	return;
}

void Client::Handle_OP_FaceChange(const EQApplicationPacket *app)
{
	if (app->size != sizeof(FaceChange_Struct)) {
		LogError("Invalid size for OP_FaceChange: Expected: [{}], Got: [{}]",
			sizeof(FaceChange_Struct), app->size);
		return;
	}

	auto fc = reinterpret_cast<FaceChange_Struct*>(app->pBuffer);

	// Notify other clients in zone
	SetFaceAppearance(*fc, true);

	m_pp.haircolor = fc->haircolor;
	m_pp.beardcolor = fc->beardcolor;
	m_pp.eyecolor1 = fc->eyecolor1;
	m_pp.eyecolor2 = fc->eyecolor2;
	m_pp.hairstyle = fc->hairstyle;
	m_pp.face = fc->face;
	m_pp.beard = fc->beard;
	m_pp.drakkin_heritage = fc->drakkin_heritage;
	m_pp.drakkin_tattoo = fc->drakkin_tattoo;
	m_pp.drakkin_details = fc->drakkin_details;
	Save();
	MessageString(Chat::Red, FACE_ACCEPTED);
	return;
}

void Client::Handle_OP_FeignDeath(const EQApplicationPacket *app)
{
	if (!HasSkill(EQ::skills::SkillFeignDeath)) {
		return;
	}

	if (!p_timers.Expired(&database, pTimerFeignDeath, false)) {
		Message(Chat::Red, "Ability recovery time not yet met.");
		return;
	}

	int reuse = FeignDeathReuseTime;
	reuse -= GetSkillReuseTime(EQ::skills::SkillFeignDeath);

	if (reuse < 1)
		reuse = 1;

	p_timers.Start(pTimerFeignDeath, reuse - 1);

	//BreakInvis();

	uint16 primfeign = GetSkill(EQ::skills::SkillFeignDeath);
	uint16 secfeign = GetSkill(EQ::skills::SkillFeignDeath);
	if (primfeign > 100) {
		primfeign = 100;
		secfeign = secfeign - 100;
		secfeign = secfeign / 2;
	}
	else
		secfeign = 0;

	uint16 totalfeign = primfeign + secfeign;
	if (zone->random.Real(0, 160) > totalfeign) {
		SetFeigned(false);
		entity_list.MessageCloseString(this, false, 200, 10, STRING_FEIGNFAILED, GetName());
	}
	else {
		SetFeigned(true);
	}

	CheckIncreaseSkill(EQ::skills::SkillFeignDeath, nullptr, 5);
	return;
}

void Client::Handle_OP_FindPersonRequest(const EQApplicationPacket *app)
{
	if (app->size != sizeof(FindPersonRequest_Struct))
		printf("Error in FindPersonRequest_Struct. Expected size of: %zu, but got: %i\n", sizeof(FindPersonRequest_Struct), app->size);
	else {
		auto* t = (FindPersonRequest_Struct*)app->pBuffer;

		auto* m = entity_list.GetMob(t->npc_id);

		SendPath(m);
	}
}

void Client::Handle_OP_Fishing(const EQApplicationPacket *app)
{
	if (!p_timers.Expired(&database, pTimerFishing, false)) {
		Message(Chat::Red, "Ability recovery time not yet met.");
		return;
	}

	if (CanFish()) {
		if (parse->PlayerHasQuestSub(EVENT_FISH_START)) {
			parse->EventPlayer(EVENT_FISH_START, this, "", 0);
		}

		//these will trigger GoFish() after a delay if we're able to actually fish, and if not, we won't stop the client from trying again immediately (although we may need to tell it to repop the button)
		p_timers.Start(pTimerFishing, FishingReuseTime - 1);
		fishing_timer.Start();
	}
	return;
	// Changes made based on Bobs work on foraging. Now can set items in the forage database table to
	// forage for.
}

void Client::Handle_OP_Forage(const EQApplicationPacket *app)
{

	if (!p_timers.Expired(&database, pTimerForaging, false)) {
		Message(Chat::Red, "Ability recovery time not yet met.");
		return;
	}

	// Not allowed to forage while mounted
	if (RuleB(Character, NoSkillsOnHorse) && GetHorseId()) {
		MessageString(Chat::Skills, NO_SKILL_WHILE_MOUNTED);
		return;
	}

	p_timers.Start(pTimerForaging, ForagingReuseTime - 1);

	ForageItem();

	return;
}

void Client::Handle_OP_FriendsWho(const EQApplicationPacket *app)
{
	char *FriendsString = (char*)app->pBuffer;
	FriendsWho(FriendsString);
	return;
}

void Client::Handle_OP_GetGuildMOTD(const EQApplicationPacket *app)
{
	LogGuilds("Received OP_GetGuildMOTD");

	SendGuildMOTD(true);

	if (IsInAGuild())
	{
		SendGuildURL();
		SendGuildChannel();
	}
}

void Client::Handle_OP_GetGuildsList(const EQApplicationPacket *app)
{
	LogGuilds("Received OP_GetGuildsList");

	SendGuildList();
}

void Client::Handle_OP_GMBecomeNPC(const EQApplicationPacket *app)
{
	if (!GetGM()) {
		Message(Chat::Red, "Your account has been reported for hacking.");
		RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = "Used /becomenpc when they shouldn't be able to"});
		return;
	}

	if (app->size != sizeof(BecomeNPC_Struct)) {
		LogError("Wrong size: OP_GMBecomeNPC, size=[{}], expected [{}]", app->size, sizeof(BecomeNPC_Struct));
		return;
	}

	auto *b = (BecomeNPC_Struct *) app->pBuffer;

	Mob *m = entity_list.GetMob(b->id);
	if (!m) {
		return;
	}

	if (m->IsClient()) {
		Client *t = m->CastToClient();
		t->QueuePacket(app);
		if (t->GetGM()) {
			t->SetInvul(false);
			t->SetHideMe(false);
			t->SetGM(false);
		}

		m->SendAppearancePacket(AppearanceType::NPCName, 1, true);
		t->SetBecomeNPC(true);
		t->SetBecomeNPCLevel(b->maxlevel);
		m->MessageString(Chat::White, TOGGLE_OFF);
		t->tellsoff = true;
		t->UpdateWho();
	}
}

void Client::Handle_OP_GMDelCorpse(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GMDelCorpse_Struct)) {
		return;
	}

	if (!GetGM()) {
		Message(Chat::Red, "Your account has been reported for hacking.");
		RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = "Used /delcorpse"});
		return;
	}

	auto *c     = (GMDelCorpse_Struct *) app->pBuffer;
	Mob  *corpse = entity_list.GetMob(c->corpsename);
	if (!c) {
		return;
	}

	if (!corpse->IsCorpse()) {
		return;
	}

	corpse->CastToCorpse()->Delete();
	Message(Chat::Red, fmt::format("Corpse {} deleted.", c->corpsename).c_str());
}

void Client::Handle_OP_GMEmoteZone(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GMEmoteZone_Struct)) {
		LogError("Wrong size: OP_GMEmoteZone, size=[{}], expected [{}]", app->size, sizeof(GMEmoteZone_Struct));
		return;
	}

	if (!GetGM()) {
		Message(Chat::Red, "Your account has been reported for hacking.");
		RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = "Used /emote"});
		return;
	}

	auto *gmez = (GMEmoteZone_Struct*)app->pBuffer;
	char *newmessage = nullptr;
	if (strstr(gmez->text, "^") == 0) {
		entity_list.Message(0, Chat::White, gmez->text);
	} else {
		for (newmessage = strtok((char *) gmez->text, "^"); newmessage != nullptr; newmessage = strtok(nullptr, "^")) {
			entity_list.Message(0, Chat::White, newmessage);
		}
	}
}

void Client::Handle_OP_GMEndTraining(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GMTrainEnd_Struct)) {
		LogDebug("Size mismatch in OP_GMEndTraining expected [{}] got [{}]", sizeof(GMTrainEnd_Struct), app->size);
		DumpPacket(app);
		return;
	}

	OPGMEndTraining(app);
}

void Client::Handle_OP_GMFind(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GMFind_Struct)) {
		LogError("Wrong size: OP_GMFind, size=[{}], expected [{}]", app->size, sizeof(GMFind_Struct));
		return;
	}

	if (!GetGM()) {
		Message(Chat::Red, "Your account has been reported for hacking.");
		RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = "Used /find"});
		return;
	}

	auto *r = (GMFind_Struct *) app->pBuffer;
	auto outapp = new EQApplicationPacket(OP_GMFind, sizeof(GMFind_Struct));
	auto *f = (GMFind_Struct *) outapp->pBuffer;

	strcpy(f->charname, r->charname);
	strcpy(f->gmname, r->gmname);

	auto* gt = entity_list.GetMob(r->charname);
	if (gt) {
		f->success = 1;
		f->x       = gt->GetX();
		f->y       = gt->GetY();
		f->z       = gt->GetZ();
		f->zoneID  = zone->GetZoneID();
	}

	FastQueuePacket(&outapp);
}

void Client::Handle_OP_GMGoto(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GMSummon_Struct)) {
		LogError("Wrong size: OP_GMGoto, size=[{}], expected [{}]", app->size, sizeof(GMSummon_Struct));
		return;
	}

	if (!GetGM()) {
		Message(Chat::Red, "Your account has been reported for hacking.");
		RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = "Used /goto"});
		return;
	}

	auto *gmg = (GMSummon_Struct *) app->pBuffer;
	Mob  *gt  = entity_list.GetMob(gmg->charname);
	if (gt) {
		MovePC(
			zone->GetZoneID(),
			zone->GetInstanceID(),
			gt->GetX(),
			gt->GetY(),
			gt->GetZ(),
			gt->GetHeading()
		);
	} else if (!worldserver.Connected()) {
		Message(Chat::Red, "Error: World server disconnected.");
	} else {
		auto pack = new ServerPacket(ServerOP_GMGoto, sizeof(ServerGMGoto_Struct));
		memset(pack->pBuffer, 0, pack->size);

		auto* g = (ServerGMGoto_Struct *) pack->pBuffer;
		strcpy(g->myname, GetName());
		strcpy(g->gotoname, gmg->charname);
		g->admin = admin;

		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void Client::Handle_OP_GMHideMe(const EQApplicationPacket *app)
{
	if (app->size != sizeof(SpawnAppearance_Struct)) {
		LogError("Wrong size: OP_GMHideMe, size=[{}], expected [{}]", app->size, sizeof(SpawnAppearance_Struct));
		return;
	}

	if (!GetGM()) {
		Message(Chat::Red, "Your account has been reported for hacking.");
		RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = "Used /hideme"});
		return;
	}

	auto *sa = (SpawnAppearance_Struct *) app->pBuffer;
	SetHideMe(!sa->parameter);
}

void Client::Handle_OP_GMKick(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GMKick_Struct)) {
		return;
	}

	if (!GetGM()) {
		Message(Chat::Red, "Your account has been reported for hacking.");
		RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = "Used /kick"});
		return;
	}

	auto *gmk = (GMKick_Struct *)app->pBuffer;
	Client *c = entity_list.GetClientByName(gmk->name);
	if (!c) {
		if (!worldserver.Connected()) {
			Message(Chat::Red, "Error: World server disconnected");
		} else {
			auto pack = new ServerPacket(ServerOP_KickPlayer, sizeof(ServerKickPlayer_Struct));
			auto *skp = (ServerKickPlayer_Struct *) pack->pBuffer;
			strcpy(skp->adminname, gmk->gmname);
			strcpy(skp->name, gmk->name);
			skp->adminrank = Admin();
			worldserver.SendPacket(pack);
			safe_delete(pack);
		}
	} else {
		entity_list.QueueClients(this, app);
	}
}

void Client::Handle_OP_GMKill(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GMKill_Struct)) {
		LogError("Wrong size: OP_GMKill, size=[{}], expected [{}]", app->size, sizeof(GMKill_Struct));
		return;
	}

	if (!GetGM()) {
		Message(Chat::Red, "Your account has been reported for hacking.");
		RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = "Used /kill"});
		return;
	}

	auto   *gmk    = (GMKill_Struct *) app->pBuffer;
	Mob    *obj    = entity_list.GetMob(gmk->name);
	Client *c = entity_list.GetClientByName(gmk->name);
	if (obj) {
		if (c) {
			entity_list.QueueClients(this, app);
		} else {
			obj->Kill();
		}
	} else {
		if (!worldserver.Connected()) {
			Message(Chat::Red, "Error: World server disconnected");
		} else {
			auto pack = new ServerPacket(ServerOP_KillPlayer, sizeof(ServerKillPlayer_Struct));
			auto *skp = (ServerKillPlayer_Struct *) pack->pBuffer;
			strcpy(skp->gmname, gmk->gmname);
			strcpy(skp->target, gmk->name);
			skp->admin = Admin();
			worldserver.SendPacket(pack);
			safe_delete(pack);
		}
	}
}

void Client::Handle_OP_GMLastName(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GMLastName_Struct)) {
		std::cout << "Wrong size on OP_GMLastName. Got: " << app->size << ", Expected: " << sizeof(GMLastName_Struct) << std::endl;
		return;
	}

	if (!GetGM()) {
		Message(Chat::Red, "Your account has been reported for hacking.");
		RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = "Used /lastname"});
		return;
	}

	auto *gmln = (GMLastName_Struct *) app->pBuffer;
	if (strlen(gmln->lastname) >= 64) {
		Message(Chat::Red, "/LastName: New last name too long. Max length is 63.");
	} else {
		Client *c = entity_list.GetClientByName(gmln->name);
		if (!c) {
			Message(Chat::Red, fmt::format("/LastName: {} not found", gmln->name).c_str());
		} else {
			c->ChangeLastName(gmln->lastname);
		}

		gmln->unknown[0] = 1;
		gmln->unknown[1] = 1;
		gmln->unknown[2] = 1;
		gmln->unknown[3] = 1;
		entity_list.QueueClients(this, app, false);
	}
}

void Client::Handle_OP_GMNameChange(const EQApplicationPacket *app)
{
	if (app->size == sizeof(AltChangeName_Struct)) {
		auto p = (AltChangeName_Struct *) app->pBuffer;

		if (!IsNameChangeAllowed()) {
			p->response_code = ChangeNameResponse::Ineligible;
			QueuePacket(app);
			return;
		}

		p->response_code = ChangeFirstName(p->new_name) ? ChangeNameResponse::Accepted : ChangeNameResponse::Denied;
		QueuePacket(app);

		return;
	}

	if (app->size != sizeof(GMName_Struct)) {
		LogError("Wrong size: OP_GMNameChange, size=[{}], expected [{}]", app->size, sizeof(GMName_Struct));
		return;
	}

	auto *gmn = (GMName_Struct *) app->pBuffer;
	if (!GetGM()) {
		Message(Chat::Red, "Your account has been reported for hacking.");
		RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = "Used /name"});
		return;
	}

	Client *c = entity_list.GetClientByName(gmn->oldname);
	LogInfo("GM([{}]) changeing players name. Old:[{}] New:[{}]", GetName(), gmn->oldname, gmn->newname);

	const bool used_name = database.IsNameUsed(gmn->newname);
	if (!c) {
		Message(Chat::Red, fmt::format("{} not found for name change. Operation failed!", gmn->oldname).c_str());
		return;
	}

	if (strlen(gmn->newname) > 63 || strlen(gmn->newname) == 0) {
		Message(Chat::Red, fmt::format("Invalid number of characters in new name '{}'.", gmn->newname).c_str());
		return;
	}

	if (used_name) {
		Message(Chat::Red, fmt::format("{} is already in use. Operation failed!", gmn->newname).c_str());
		return;
	}

	database.UpdateName(gmn->oldname, gmn->newname);
	strcpy(c->name, gmn->newname);
	c->Save();

	if (gmn->badname == 1) {
		database.AddToNameFilter(gmn->oldname);
	}

	auto *outapp = app->Copy();
	auto *gmn2 = (GMName_Struct *) outapp->pBuffer;
	gmn2->unknown[0] = 1;
	gmn2->unknown[1] = 1;
	gmn2->unknown[2] = 1;
	entity_list.QueueClients(this, outapp, false);
	safe_delete(outapp);
	UpdateWho();
}

void Client::Handle_OP_GMSearchCorpse(const EQApplicationPacket *app)
{
	// Could make this into a rule, although there is a hard limit since we are using a popup, of 4096 bytes that can
	// be displayed in the window, including all the HTML formatting tags.
	//
	const int max_results = 10;

	if (app->size < sizeof(GMSearchCorpse_Struct)) {
		LogDebug("OP_GMSearchCorpse size lower than expected: got [{}] expected at least [{}]", app->size, sizeof(GMSearchCorpse_Struct));
		DumpPacket(app);
		return;
	}

	auto s = (GMSearchCorpse_Struct *) app->pBuffer;
	s->Name[63] = '\0';

	const auto& l = CharacterCorpsesRepository::GetWhere(
		database,
		fmt::format(
			"`charname` LIKE '%{}%' ORDER BY `charname` LIMIT {}",
			Strings::Escape(s->Name),
			max_results
		)
	);

	if (l.empty()) {
		Message(
			Chat::White,
			fmt::format(
				"No corpses were found matching '{}'.",
				s->Name
			).c_str()
		);
		return;
	}

	if (l.size() == max_results) {
		Message(Chat::White, "Your search found too many results; some are not displayed.");
	} else {
		Message(
			Chat::White,
			fmt::format(
				"{} Corpse{} were found matching '{}'.",
				l.size(),
				l.size() != 1 ? "s" : "",
				s->Name
			).c_str()
		);
	}

	std::string popup_text = DialogueWindow::TableRow(
		DialogueWindow::TableCell("Name") +
		DialogueWindow::TableCell("Zone") +
		DialogueWindow::TableCell("Position") +
		DialogueWindow::TableCell("Date") +
		DialogueWindow::TableCell("Resurrected") +
		DialogueWindow::TableCell("Buried")
	);

	for (const auto& e : l) {
		popup_text += DialogueWindow::TableRow(
			DialogueWindow::TableCell(e.charname) +
			DialogueWindow::TableCell(
				fmt::format(
					"{} ({})",
					ZoneStore::Instance()->GetZoneLongName(e.zone_id, true),
					e.zone_id
				)
			) +
			DialogueWindow::TableCell(
				fmt::format(
					"{:.2f}, {:.2f}, {:.2f}, {:.2f}",
					e.x,
					e.y,
					e.z,
					e.heading
				)
			) +
			DialogueWindow::TableCell(std::to_string(e.time_of_death)) +
			DialogueWindow::TableCell(
				e.is_rezzed ?
				DialogueWindow::ColorMessage("forest_green", "Y") :
				DialogueWindow::ColorMessage("red_1", "N")
			) +
			DialogueWindow::TableCell(
				e.is_buried ?
				DialogueWindow::ColorMessage("forest_green", "Y") :
				DialogueWindow::ColorMessage("red_1", "N")
			)
		);
	}

	popup_text = DialogueWindow::Table(popup_text);

	SendPopupToClient("Corpses", popup_text.c_str());
}

void Client::Handle_OP_GMServers(const EQApplicationPacket *app)
{
	auto pack = new ServerPacket(ServerOP_ZoneStatus, sizeof(ServerZoneStatus_Struct));

	auto z = (ServerZoneStatus_Struct *) pack->pBuffer;
	z->admin = Admin();
	strn0cpy(z->name, GetName(), sizeof(z->name));

	worldserver.SendPacket(pack);
	delete pack;
}

void Client::Handle_OP_GMSummon(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GMSummon_Struct)) {
		std::cout << "Wrong size on OP_GMSummon. Got: " << app->size << ", Expected: " << sizeof(GMSummon_Struct) << std::endl;
		return;
	}

	OPGMSummon(app);
	return;
}

void Client::Handle_OP_GMToggle(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GMToggle_Struct)) {
		std::cout << "Wrong size on OP_GMToggle. Got: " << app->size << ", Expected: " << sizeof(GMToggle_Struct) << std::endl;
		return;
	}

	if (!GetGM()) {
		Message(Chat::Red, "Your account has been reported for hacking.");
		RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = "Used /toggle"});
		return;
	}

	auto *ts = (GMToggle_Struct *)app->pBuffer;
	if (ts->toggle == 0) {
		MessageString(Chat::White, TOGGLE_OFF);
		tellsoff = true;
	} else if (ts->toggle == 1) {
		MessageString(Chat::White, TOGGLE_ON);
		tellsoff = false;
	} else {
		Message(Chat::White, "Unknown value in /toggle packet.");
	}

	UpdateWho();
}

void Client::Handle_OP_GMTraining(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GMTrainee_Struct)) {
		LogDebug("Size mismatch in OP_GMTraining expected [{}] got [{}]", sizeof(GMTrainee_Struct), app->size);
		DumpPacket(app);
		return;
	}

	OPGMTraining(app);
	return;
}

void Client::Handle_OP_GMTrainSkill(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GMSkillChange_Struct)) {
		LogDebug("Size mismatch in OP_GMTrainSkill expected [{}] got [{}]", sizeof(GMSkillChange_Struct), app->size);
		DumpPacket(app);
		return;
	}

	OPGMTrainSkill(app);
	return;
}

void Client::Handle_OP_GMZoneRequest(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GMZoneRequest_Struct)) {
		std::cout << "Wrong size on OP_GMZoneRequest. Got: " << app->size << ", Expected: " << sizeof(GMZoneRequest_Struct) << std::endl;
		return;
	}

	if (!GetGM()) {
		Message(Chat::Red, "Your account has been reported for hacking.");
		RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = "Used /zone"});
		return;
	}

	auto  *gmzr    = (GMZoneRequest_Struct *) app->pBuffer;
	float target_x = -1, target_y = -1, target_z = -1, target_heading;

	int16  min_status = AccountStatus::Player;
	uint8  min_level  = 0;
	char   target_zone[32];
	uint16 zone_id    = gmzr->zone_id;
	if (gmzr->zone_id == 0) {
		zone_id = zonesummon_id;
	}

	const char *zone_short_name = ZoneName(zone_id);
	if (zone_short_name == nullptr) {
		target_zone[0] = 0;
	} else {
		strcpy(target_zone, zone_short_name);
	}

	// this both loads the safe points and does a sanity check on zone name
	auto z = GetZone(target_zone, 0);
	if (z) {
		target_x       = z->safe_x;
		target_y       = z->safe_y;
		target_z       = z->safe_z;
		target_heading = z->safe_heading;
	} else {
		target_zone[0] = 0;
	}

	auto outapp = new EQApplicationPacket(OP_GMZoneRequest, sizeof(GMZoneRequest_Struct));
	auto *gmzr2 = (GMZoneRequest_Struct *) outapp->pBuffer;
	strcpy(gmzr2->charname, GetName());
	gmzr2->zone_id = gmzr->zone_id;
	gmzr2->x       = target_x;
	gmzr2->y       = target_y;
	gmzr2->z       = target_z;
	gmzr2->heading = target_heading;

	if (target_zone[0] != 0 && admin >= min_status && GetLevel() >= min_level) {
		gmzr2->success = 1;
	} else {
		LogError("GetZoneSafeCoords failed. Zone ID [{}] Current Zone [{}]", gmzr->zone_id, zone->GetZoneID());
		gmzr2->success = 0;
	}

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::Handle_OP_GMZoneRequest2(const EQApplicationPacket *app)
{
	if (app->size < sizeof(uint32)) {
		LogError("OP size error: OP_GMZoneRequest2 expected:[{}] got:[{}]", sizeof(uint32), app->size);
		return;
	}

	if (!GetGM()) {
		Message(Chat::Red, "Your account has been reported for hacking.");
		RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = "Used /zone"});
		return;
	}

	uint32 zonereq = *((uint32 *)app->pBuffer);
	GoToSafeCoords(zonereq, 0);
}

void Client::Handle_OP_GroupAcknowledge(const EQApplicationPacket *app)
{
	return;
}

void Client::Handle_OP_GroupCancelInvite(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GroupCancel_Struct)) {
		LogError("Invalid size for OP_GroupCancelInvite: Expected: [{}], Got: [{}]",
			sizeof(GroupCancel_Struct), app->size);
		return;
	}

	GroupCancel_Struct* gf = (GroupCancel_Struct*)app->pBuffer;
	Mob* inviter = entity_list.GetClientByName(gf->name1);

	if (inviter != nullptr)
	{
		if (inviter->IsClient())
			inviter->CastToClient()->QueuePacket(app);
	}
	else
	{
		auto pack = new ServerPacket(ServerOP_GroupCancelInvite, sizeof(GroupCancel_Struct));
		memcpy(pack->pBuffer, gf, sizeof(GroupCancel_Struct));
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}

	if (!GetMerc())
	{
		Group::RemoveFromGroup(this);
	}
	return;
}

void Client::Handle_OP_GroupDelete(const EQApplicationPacket *app)
{
	//should check for leader, only they should be able to do this..
	Group* group = GetGroup();
	if (group)
		group->DisbandGroup();

	if (LFP)
		UpdateLFP();

	return;
}

void Client::Handle_OP_GroupDisband(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GroupGeneric_Struct)) {
		LogError("Invalid size for GroupGeneric_Struct: Expected: [{}], Got: [{}]",
			sizeof(GroupGeneric_Struct), app->size);
		return;
	}

	LogDebug("Member Disband Request from [{}]\n", GetName());

	GroupGeneric_Struct* gd = (GroupGeneric_Struct*)app->pBuffer;

	Raid *raid = entity_list.GetRaidByClient(this);
	if (raid) {
		Mob *memberToDisband = nullptr;

		if (!raid->IsGroupLeader(GetName())) {
			memberToDisband = this;
		} else {
			memberToDisband = GetTarget();
		}

		if (!memberToDisband) {
			memberToDisband = entity_list.GetMob(gd->name2);
		}

		if (!memberToDisband) {
			memberToDisband = this;
		}

		if (!memberToDisband->IsOfClientBot()) {
			return;
		}
		//we have a raid.. see if we're in a raid group
		uint32 grp = raid->GetGroup(memberToDisband->GetName());
		bool wasGrpLdr = raid->members[raid->GetPlayerIndex(memberToDisband->GetName())].is_group_leader;
		if (grp < MAX_RAID_GROUPS) {
			if (wasGrpLdr) {
				raid->SetGroupLeader(memberToDisband->GetName(), false);
				for (int x = 0; x < MAX_RAID_MEMBERS; x++) {
					if (raid->members[x].group_number == grp) {
						if (strlen(raid->members[x].member_name) > 0 &&
							strcmp(raid->members[x].member_name, memberToDisband->GetName()) != 0) {
							raid->SetGroupLeader(raid->members[x].member_name);
							break;
						}
					}
				}
			}
			raid->MoveMember(memberToDisband->GetName(), RAID_GROUPLESS);
			raid->GroupUpdate(grp);

			if (memberToDisband->IsClient()) {
				raid->SendGroupDisband(memberToDisband->CastToClient());
			}
		}
		//we're done
		return;
	}

	Group* group = GetGroup();

	if (!group) {
		return;
	}

	// this block is necessary to allow more control over controlling how bots are zoned or camped.
	if (RuleB(Bots, Enabled) && Bot::GroupHasBot(group)) {
		if (group->IsLeader(this)) {
			if ((GetTarget() == 0 || GetTarget() == this) || (group->GroupCount() < 3)) {
				Bot::ProcessBotGroupDisband(this, std::string());
			}
			else {
				Mob* tempMember = entity_list.GetMob(gd->name1); //Name1 is the target you are disbanding

				if (tempMember && tempMember->IsBot()) {
					auto b = tempMember->CastToBot();
					Bot::RemoveBotFromGroup(b, group);

					if (LFP)
					{
						// If we are looking for players, update to show we are on our own now.
						UpdateLFP();
					}

					return; //No need to continue from here we were removing a bot from party
				}
			}
		}
	}

	group = GetGroup();
	if (!group) {
		return;
	}

	Mob* memberToDisband = GetTarget();

	if (!memberToDisband)
		memberToDisband = entity_list.GetMob(gd->name2);

	if (memberToDisband) {
		auto group2 = memberToDisband->GetGroup();
		if (group2 != group) // they're not in our group!
			memberToDisband = this;
	}

	if (group->GroupCount() < 3)
	{
		group->DisbandGroup();
		if (GetMerc())
			GetMerc()->Suspend();
	}
	else if (group->IsLeader(this) && GetTarget() == nullptr)
	{
		if (group->GroupCount() > 2 && GetMerc() && !GetMerc()->IsSuspended())
		{
			group->DisbandGroup();
			GetMerc()->MercJoinClientGroup();
		}
		else
		{
			group->DisbandGroup();
			if (GetMerc())
				GetMerc()->Suspend();
		}
	}
	else if (group->IsLeader(this) && (GetTarget() == this || memberToDisband == this))
	{
		LeaveGroup();
		if (GetMerc() && !GetMerc()->IsSuspended())
		{
			GetMerc()->MercJoinClientGroup();
		}
	}
	else
	{
		if (memberToDisband)
		{
			if (group->IsLeader(this))
			{
				// the group leader can kick other members out of the group...
				if (memberToDisband->IsClient())
				{
					group->DelMember(memberToDisband, false);
					Client* memberClient = memberToDisband->CastToClient();
					Merc* memberMerc = memberToDisband->CastToClient()->GetMerc();
					if (memberClient && memberMerc)
					{
						memberMerc->MercJoinClientGroup();
					}
				}
				else if (memberToDisband->IsMerc())
				{
					memberToDisband->CastToMerc()->Suspend();
				}
			}
			else
			{
				// ...but other members can only remove themselves
				group->DelMember(this, false);

				if (GetMerc() && !GetMerc()->IsSuspended())
				{
					GetMerc()->MercJoinClientGroup();
				}
			}
		}
		else
		{
			LogError("Failed to remove player from group. Unable to find player named [{}] in player group", gd->name2);
		}
	}
	if (LFP)
	{
		// If we are looking for players, update to show we are on our own now.
		UpdateLFP();
	}

	return;
}

void Client::Handle_OP_GroupFollow(const EQApplicationPacket *app)
{
	Handle_OP_GroupFollow2(app);
}

void Client::Handle_OP_GroupFollow2(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GroupGeneric_Struct)) {
		LogError("Invalid size for OP_GroupFollow: Expected: [{}], Got: [{}]",
			sizeof(GroupGeneric_Struct), app->size);
		return;
	}

	if (LFP) {
		// If we were looking for players to start our own group, but we accept an invitation to another
		// group, turn LFP off.
		database.SetLFP(CharacterID(), false);
		worldserver.StopLFP(CharacterID());
	}

	GroupGeneric_Struct* gf = (GroupGeneric_Struct*)app->pBuffer;
	Mob* inviter = entity_list.GetClientByName(gf->name1);

	// Inviter and Invitee are in the same zone
	if (inviter != nullptr && inviter->IsClient())
	{
		if (GroupFollow(inviter->CastToClient()))
		{
			strn0cpy(gf->name1, inviter->GetName(), sizeof(gf->name1));
			strn0cpy(gf->name2, GetName(), sizeof(gf->name2));
			inviter->CastToClient()->QueuePacket(app);//notify inviter the client accepted
		}
	}
	else if (inviter == nullptr)
	{
		// Inviter is in another zone - Remove merc from group now if any
		LeaveGroup();

		auto pack = new ServerPacket(ServerOP_GroupFollow, sizeof(ServerGroupFollow_Struct));
		ServerGroupFollow_Struct *sgfs = (ServerGroupFollow_Struct *)pack->pBuffer;
		sgfs->CharacterID = CharacterID();
		strn0cpy(sgfs->gf.name1, gf->name1, sizeof(sgfs->gf.name1));
		strn0cpy(sgfs->gf.name2, gf->name2, sizeof(sgfs->gf.name2));
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void Client::Handle_OP_GroupInvite(const EQApplicationPacket *app)
{
	//this seems to be the initial invite to form a group
	Handle_OP_GroupInvite2(app);
}

void Client::Handle_OP_GroupInvite2(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GroupInvite_Struct)) {
		LogError("Invalid size for OP_GroupInvite: Expected: [{}], Got: [{}]",
			sizeof(GroupInvite_Struct), app->size);
		return;
	}

	GroupInvite_Struct* gis = (GroupInvite_Struct*)app->pBuffer;

	Mob* invitee = nullptr;

	if (RuleB(Character, GroupInvitesRequireTarget)) {
		// We can only invite the current target.
		invitee = GetTarget();
	} else {
		invitee = entity_list.GetMob(gis->invitee_name);
	}

	if (invitee == this) {
		MessageString(Chat::LightGray, GROUP_INVITEE_SELF);
		return;
	}

	if (invitee) {
		if (invitee->IsClient()) {
			if (invitee->CastToClient()->MercOnlyOrNoGroup() && !invitee->IsRaidGrouped()) {
				if (app->GetOpcode() == OP_GroupInvite2) {
					//Make a new packet using all the same information but make sure it's a fixed GroupInvite opcode so we
					//Don't have to deal with GroupFollow2 crap.
					auto outapp =
						new EQApplicationPacket(OP_GroupInvite, sizeof(GroupInvite_Struct));
					memcpy(outapp->pBuffer, app->pBuffer, outapp->size);
					invitee->CastToClient()->QueuePacket(outapp);
					safe_delete(outapp);
					return;
				} else {
					//The correct opcode, no reason to bother wasting time reconstructing the packet
					invitee->CastToClient()->QueuePacket(app);
				}
			} else if (invitee->IsRaidGrouped()) {
				Raid* inviter_raid = GetRaid();
				Raid* invitee_raid = invitee->CastToClient()->GetRaid();

				bool leader = false;

				if (invitee_raid) {
					leader = invitee_raid->IsGroupLeader(invitee->GetName());
				}

				if (inviter_raid != invitee_raid || leader) {
					MessageString(Chat::Default, ALREADY_IN_GRP_RAID, invitee->GetCleanName());
				} else {
					MessageString(Chat::Default, TARGET_ALREADY_IN_GROUP, invitee->GetCleanName());
				}

				return;

			} else {
				if (RuleB(Character, OnInviteReceiveAlreadyinGroupMessage)) {
					if (!invitee->CastToClient()->MercOnlyOrNoGroup()) {
						MessageString(Chat::Default, TARGET_ALREADY_IN_GROUP, invitee->GetCleanName());
					}
				}
			}
		} else if (invitee->IsBot()) {
			Client* inviter = entity_list.GetClientByName(gis->inviter_name);
			if (inviter && inviter->IsRaidGrouped() && !invitee->HasRaid()) {
				Bot::ProcessRaidInvite(invitee->CastToBot(), inviter, true);
			} else if (!invitee->HasRaid()) {
				Bot::ProcessBotGroupInvite(this, std::string(invitee->GetName()));
			} else {
				MessageString(Chat::LightGray, ALREADY_IN_RAID, invitee->GetCleanName());
			}
		}
	} else {
		if (RuleB(Character, GroupInvitesRequireTarget)) {
			Message(Chat::White, "You must target a player first to invite to join your group.");
		} else {
			auto pack = new ServerPacket(ServerOP_GroupInvite, sizeof(GroupInvite_Struct));
			memcpy(pack->pBuffer, gis, sizeof(GroupInvite_Struct));
			worldserver.SendPacket(pack);
			safe_delete(pack);
		}
	}
	return;
}

void Client::Handle_OP_GroupMakeLeader(const EQApplicationPacket *app)
{
	VERIFY_PACKET_LENGTH(OP_GroupMakeLeader, app, GroupMakeLeader_Struct);

	GroupMakeLeader_Struct *gmls = (GroupMakeLeader_Struct *)app->pBuffer;

	Mob* NewLeader = entity_list.GetClientByName(gmls->NewLeader);

	Group* g = GetGroup();

	if (NewLeader && g)
	{
		if (g->IsLeader(this))
			g->ChangeLeader(NewLeader);
		else {
			LogDebug("Group /makeleader request originated from non-leader member: [{}]", GetName());
			DumpPacket(app);
		}
	}
}

void Client::Handle_OP_GroupMentor(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GroupMentor_Struct)) {
		LogError("Wrong size: OP_GroupMentor, size=[{}], expected [{}]", app->size, sizeof(GroupMentor_Struct));
		DumpPacket(app);
		return;
	}
	GroupMentor_Struct *gms = (GroupMentor_Struct *)app->pBuffer;
	gms->name[63] = '\0';

	if (IsRaidGrouped()) {
		Raid *raid = GetRaid();
		if (!raid)
			return;
		uint32 group_id = raid->GetGroup(this);
		if (group_id > 11)
			return;
		if (strlen(gms->name))
			raid->SetGroupMentor(group_id, gms->percent, gms->name);
		else
			raid->ClearGroupMentor(group_id);
		return;
	}

	Group *group = GetGroup();
	if (!group)
		return;

	if (strlen(gms->name))
		group->SetGroupMentor(gms->percent, gms->name);
	else
		group->ClearGroupMentor();

	return;
}

void Client::Handle_OP_GroupRoles(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GroupRole_Struct)) {
		LogError("Wrong size: OP_GroupRoles, size=[{}], expected [{}]", app->size, sizeof(GroupRole_Struct));
		DumpPacket(app);
		return;
	}
	GroupRole_Struct *grs = (GroupRole_Struct*)app->pBuffer;

	Group *g = GetGroup();

	if (!g)
		return;

	switch (grs->RoleNumber)
	{
	case 1: //Main Tank
	{
		if (grs->Toggle)
			g->DelegateMainTank(grs->Name1, grs->Toggle);
		else
			g->UnDelegateMainTank(grs->Name1, grs->Toggle);
		break;
	}
	case 2: //Main Assist
	{
		if (grs->Toggle)
			g->DelegateMainAssist(grs->Name1, grs->Toggle);
		else
			g->UnDelegateMainAssist(grs->Name1, grs->Toggle);
		break;
	}
	case 3: //Puller
	{
		if (grs->Toggle)
			g->DelegatePuller(grs->Name1, grs->Toggle);
		else
			g->UnDelegatePuller(grs->Name1, grs->Toggle);
		break;
	}
	default:
		break;
	}
}

void Client::Handle_OP_GroupUpdate(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GroupUpdate_Struct))
	{
		LogDebug("Size mismatch on OP_GroupUpdate: got [{}] expected [{}]", app->size, sizeof(GroupUpdate_Struct));
		DumpPacket(app);
		return;
	}

	GroupUpdate_Struct* gu = (GroupUpdate_Struct*)app->pBuffer;

	switch (gu->action) {
	case groupActMakeLeader:
	{
		Mob* newleader = entity_list.GetClientByName(gu->membername[0]);
		Group* group = GetGroup();

		if (newleader && group) {
			// the client only sends this if it's the group leader, but check anyway
			if (group->IsLeader(this))
				group->ChangeLeader(newleader);
			else {
				LogDebug("Group /makeleader request originated from non-leader member: [{}]", GetName());
				DumpPacket(app);
			}
		}
		break;
	}

	default:
	{
		LogDebug("Received unhandled OP_GroupUpdate requesting action [{}]", gu->action);
		DumpPacket(app);
		return;
	}
	}
}

void Client::Handle_OP_GuildBank(const EQApplicationPacket *app)
{
	if (!GuildBanks) {
		GuildBankAck();
		return;
	}

	if (zone->GetZoneID() != Zones::GUILDHALL) {
		Message(Chat::Red, "The Guild Bank is not available in this zone.");
		GuildBankAck();
		return;
	}

	if (app->size < sizeof(uint32)) {
		LogError("Wrong size: OP_GuildBank, size=[{}], expected [{}]", app->size, sizeof(uint32));
		DumpPacket(app);
		GuildBankAck();
		return;
	}

	char  *Buffer     = (char *) app->pBuffer;
	uint32 Action     = VARSTRUCT_DECODE_TYPE(uint32, Buffer);
	uint32 sentAction = Action;

	if (!IsInAGuild()) {
		Message(Chat::Red, "You must be in a Guild to use the Guild Bank.");
		if (Action == GuildBankDeposit) {
			GuildBankDepositAck(true, sentAction);
		}
		else {
			GuildBankAck();
		}

		return;
	}

	if (!IsGuildBanker())
	{
		if (Action != GuildBankDeposit && Action != GuildBankViewItem && Action != GuildBankWithdraw)
		{
			LogError("Suspected hacking attempt on guild bank from [{}]", GetName());
			GuildBankAck();
			return;
		}
	}

	switch (Action) {
		case GuildBankPromote: {
			if (GuildBanks->IsAreaFull(GuildID(), GuildBankMainArea)) {
				MessageString(Chat::Red, GUILD_BANK_FULL);
				GuildBankAck();
				return;
			}

			auto gbps    = (GuildBankPromote_Struct *) app->pBuffer;
			int  slot_id = GuildBanks->Promote(GuildID(), gbps->Slot, this);

			if (slot_id >= 0) {
				auto inst = GuildBanks->GetItem(GuildID(), GuildBankMainArea, slot_id, 1);
				if (inst) {
					if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::GUILD_BANK_MOVE_TO_BANK_AREA)) {
						PlayerEvent::GuildBankTransaction log{};
						log.char_id  = CharacterID();
						log.guild_id = GuildID();
						log.item_id  = inst->GetID();
						log.quantity = 1;
						if (inst->GetCharges() > 0 || inst->IsStackable() || inst->GetItem()->MaxCharges > 0) {
							log.quantity = inst->GetCharges();
						}

						if (inst->IsAugmented()) {
							auto augs          = inst->GetAugmentIDs();
							log.aug_slot_one   = augs.at(0);
							log.aug_slot_two   = augs.at(1);
							log.aug_slot_three = augs.at(2);
							log.aug_slot_four  = augs.at(3);
							log.aug_slot_five  = augs.at(4);
							log.aug_slot_six   = augs.at(5);
						}

						RecordPlayerEventLog(PlayerEvent::GUILD_BANK_MOVE_TO_BANK_AREA, log);
					}

					MessageString(Chat::LightGray, GUILD_BANK_TRANSFERRED, inst->GetItem()->Name);
				}
			}
			else {
				Message(Chat::Red, "Unexpected error while moving item into Guild Bank.");
			}

			GuildBankAck();
			break;
		}

		case GuildBankViewItem: {
			auto gbvis = (GuildBankViewItem_Struct *) app->pBuffer;
			auto inst  = GuildBanks->GetItem(GuildID(), gbvis->Area, gbvis->SlotID, 1);

			if (!inst) {
				break;
			}

			SendItemPacket(0, inst.get(), ItemPacketViewLink);
			break;
		}

		case GuildBankDeposit: // Deposit Item
		{
			const auto cursor_item_inst = GetInv().GetItem(EQ::invslot::slotCursor);
			bool       allowed          = true;

			if (!cursor_item_inst) {
				Message(Chat::Red, "No Item on the cursor.");
				GuildBankDepositAck(true, sentAction);
				return;
			}

			const auto cursor_item = cursor_item_inst->GetItem();
			if (GuildBanks->IsAreaFull(GuildID(), GuildBankDepositArea)) {
				MessageString(Chat::Red, GUILD_BANK_FULL);
				GuildBankDepositAck(true, sentAction);
				if (ClientVersion() >= EQ::versions::ClientVersion::RoF) {
					GetInv().PopItem(EQ::invslot::slotCursor);
					PushItemOnCursor(*cursor_item_inst, true);
				}

				return;
			}

			if (!cursor_item->NoDrop ||
				cursor_item_inst->IsAttuned() ||
				cursor_item_inst->IsNoneEmptyContainer() ||
				cursor_item->NoRent == 0 ||
				(cursor_item->LoreFlag && GuildBanks->HasItem(GuildID(), cursor_item->ID))
				) {
				allowed = false;
			}

			if (!allowed) {
				MessageString(Chat::Red, GUILD_BANK_CANNOT_DEPOSIT);
				GuildBankDepositAck(true, sentAction);

				if (ClientVersion() >= EQ::versions::ClientVersion::RoF) {
					GetInv().PopItem(EQ::invslot::slotCursor);
					PushItemOnCursor(*cursor_item_inst, true);
				}
				return;
			}

			auto item        = GuildBankRepository::NewEntity();
			item.guild_id    = GuildID();
			item.area        = GuildBankDepositArea;
			item.item_id     = cursor_item->ID;
			item.quantity    = 1;
			if (cursor_item_inst->GetCharges() > 0 || cursor_item_inst->IsStackable() || cursor_item->MaxCharges > 0) {
				item.quantity = cursor_item_inst->GetCharges();
			}

			item.donator     = GetCleanName();
			item.permissions = GuildBankBankerOnly;
			if (cursor_item_inst->IsAugmented()) {
				auto const augs       = cursor_item_inst->GetAugmentIDs();
				item.augment_one_id   = augs.at(0);
				item.augment_two_id   = augs.at(1);
				item.augment_three_id = augs.at(2);
				item.augment_four_id  = augs.at(3);
				item.augment_five_id  = augs.at(4);
				item.augment_six_id   = augs.at(5);
			}

			if (GuildBanks->AddItem(item, this)) {
				GuildBankDepositAck(false, sentAction);
				DeleteItemInInventory(EQ::invslot::slotCursor, 0, false);

				if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::GUILD_BANK_DEPOSIT)) {
					PlayerEvent::GuildBankTransaction log{};
					log.char_id        = CharacterID();
					log.guild_id       = GuildID();
					log.item_id        = item.item_id;
					log.quantity       = item.quantity;
					log.aug_slot_one   = item.augment_one_id;
					log.aug_slot_two   = item.augment_two_id;
					log.aug_slot_three = item.augment_three_id;
					log.aug_slot_four  = item.augment_four_id;
					log.aug_slot_five  = item.augment_five_id;
					log.aug_slot_six   = item.augment_six_id;

					RecordPlayerEventLog(PlayerEvent::GUILD_BANK_DEPOSIT, log);
				}
			}

			break;
		}

		case GuildBankPermissions: {
			auto gbps = (GuildBankPermissions_Struct *) app->pBuffer;

			if (gbps->Permissions == 1) {
				GuildBanks->SetPermissions(GuildID(), gbps->SlotID, gbps->Permissions, gbps->MemberName, this);
			}
			else {
				GuildBanks->SetPermissions(GuildID(), gbps->SlotID, gbps->Permissions, "", this);
			}

			GuildBankAck();
			break;
		}

		case GuildBankWithdraw: {
			if (GetInv()[EQ::invslot::slotCursor]) {
				MessageString(Chat::Red, GUILD_BANK_EMPTY_HANDS);
				GuildBankAck();
				break;
			}

			auto gbwis = (GuildBankWithdrawItem_Struct *) app->pBuffer;
			auto inst  = GuildBanks->GetItem(GuildID(), gbwis->Area, gbwis->SlotID, gbwis->Quantity);

			if (!inst) {
				GuildBankAck();
				break;
			}

			if (!guild_mgr.CheckPermission(GuildID(), GuildRank(), GUILD_ACTION_BANK_WITHDRAW_ITEMS)) {
				Message(Chat::Red, "You do not have permission to withdraw.");
				GuildBankAck();
				break;
			}

			if (!IsGuildBanker()) {
				LogError("Suspected attempted hack on the guild bank from [{}]", GetName());
				GuildBankAck();
				break;
			}

			if (CheckLoreConflict(inst->GetItem())) {
				MessageString(Chat::Red, DUP_LORE);
				GuildBankAck();
				break;
			}

			gbwis->Quantity = 1;
			if (inst->GetCharges() > 0 || inst->IsStackable() || inst->GetItem()->MaxCharges > 0) {
				gbwis->Quantity = inst->GetCharges();
			}

			PushItemOnCursor(*inst.get());
			SendItemPacket(EQ::invslot::slotCursor, inst.get(), ItemPacketLimbo);
			GuildBanks->DeleteItem(GuildID(), gbwis->Area, gbwis->SlotID, gbwis->Quantity, this);

			if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::GUILD_BANK_WITHDRAWAL)) {
				PlayerEvent::GuildBankTransaction log{};
				log.char_id  = CharacterID();
				log.guild_id = GuildID();
				log.item_id  = inst->GetID();
				log.quantity = gbwis->Quantity;
				if (inst->IsAugmented()) {
					auto augs          = inst->GetAugmentIDs();
					log.aug_slot_one   = augs.at(0);
					log.aug_slot_two   = augs.at(1);
					log.aug_slot_three = augs.at(2);
					log.aug_slot_four  = augs.at(3);
					log.aug_slot_five  = augs.at(4);
					log.aug_slot_six   = augs.at(5);
				}

				RecordPlayerEventLog(PlayerEvent::GUILD_BANK_WITHDRAWAL, log);
			}

			else {
				Message(Chat::Red, "Unable to withdraw 0 quantity of %s", inst->GetItem()->Name);
			}

			GuildBankAck();
			break;
		}
		case GuildBankSplitStacks: {
			if (GuildBanks->IsAreaFull(GuildID(), GuildBankMainArea)) {
				MessageString(Chat::Red, GUILD_BANK_FULL);
			}
			else {
				auto gbwis = (GuildBankWithdrawItem_Struct *) app->pBuffer;
				GuildBanks->SplitStack(GuildID(), gbwis->SlotID, gbwis->Quantity, this);
			}

			GuildBankAck();
			break;
		}
		case GuildBankMergeStacks: {
			auto gbwis = (GuildBankWithdrawItem_Struct *) app->pBuffer;
			GuildBanks->MergeStacks(GuildID(), gbwis->SlotID, this);
			GuildBankAck();
			break;
		}

		default: {
			Message(Chat::Red, "Unexpected GuildBank action.");
			LogError("Received unexpected guild bank action code [{}] from [{}]", Action, GetName());
		}
	}
}

void Client::Handle_OP_GuildCreate(const EQApplicationPacket *app)
{
	if (IsInAGuild()) {
		Message(Chat::Red, "You are already in a guild!");
		return;
	}

	if (!RuleB(Guild, PlayerCreationAllowed)) {
		Message(
			Chat::Red,
			"This feature is disabled on this server. Contact a GM or post on your server message boards to create a guild."
		);
		return;
	}

	if ((Admin() < RuleI(Guild, PlayerCreationRequiredStatus)) ||
		(GetLevel() < RuleI(Guild, PlayerCreationRequiredLevel)) ||
		(CharacterDataRepository::GetTotalTimePlayed(database, AccountID()) < (unsigned int)RuleI(Guild, PlayerCreationRequiredTime)))
	{
		Message(Chat::Red, "Your status, level or time playing on this account are insufficient to use this feature.");
		return;
	}

	// The Underfoot client Guild Creation window will only allow a guild name of <= around 30 characters, but the packet is 64 bytes. Sanity check the
	// name anway.
	//

	char *GuildName = (char *)app->pBuffer;
#ifdef DARWIN
#if __DARWIN_C_LEVEL < 200809L
	if (strlen(GuildName) > 60)
#else
	if (strnlen(GuildName, 64) > 60)
#endif // __DARWIN_C_LEVEL
#else
	if (strnlen(GuildName, 64) > 60)
#endif // DARWIN
	{
		Message(Chat::Red, "Guild name too long.");
		return;
	}

	for (unsigned int i = 0; i < strlen(GuildName); ++i) {
		if (!isalpha(GuildName[i]) && (GuildName[i] != ' ')) {
			Message(Chat::Red, "Invalid character in Guild name.");
			return;
		}
	}

	int32 GuildCount = guild_mgr.DoesAccountContainAGuildLeader(AccountID());

	if (GuildCount >= RuleI(Guild, PlayerCreationLimit)) {
		Message(
			Chat::Red,
			"You cannot create this guild because this account may only be leader of %i guilds.",
			RuleI(Guild, PlayerCreationLimit));
		return;
	}

	if (guild_mgr.GetGuildIDByName(GuildName) != GUILD_NONE) {
		MessageString(Chat::Red, GUILD_NAME_IN_USE);
		return;
	}

	uint32 new_guild_id = guild_mgr.CreateGuild(GuildName, CharacterID());
	if (new_guild_id == GUILD_NONE) {
		Message(Chat::Red, "Guild creation failed.  An account can only have one Guild Leader.");
		return;
	}

	SetGuildID(new_guild_id);
	UpdateWho();
	guild_mgr.MemberAdd(new_guild_id, CharacterID(), GetLevel(), GetClass(), GUILD_LEADER, GetZoneID(), GetName());
	guild_mgr.SendGuildRefresh(new_guild_id, true, true, true, true);
	guild_mgr.SendToWorldSendGuildList();
	SendGuildSpawnAppearance();

	Message(Chat::Yellow, "You are now the leader of %s", GuildName);

	LogGuilds("[{}]: Creating guild [{}] with leader [{}] via UF+ GUI. It was given id [{}]",
			  GetName(),
			  GuildName,
			  CharacterID(),
			  new_guild_id
			  );
}

void Client::Handle_OP_GuildDelete(const EQApplicationPacket *app)
{
	LogGuilds("Received OP_GuildDelete");

	if (!IsInAGuild() || !guild_mgr.IsGuildLeader(GuildID(), CharacterID()))
		Message(Chat::Red, "You are not a guild leader or not in a guild.");
	else {
		LogGuilds("Deleting guild [{}] ([{}])", guild_mgr.GetGuildName(GuildID()), GuildID());
		if (!guild_mgr.DeleteGuild(GuildID()))
			Message(Chat::Red, "Guild delete failed. Do you have items in the Guild Bank?");
		else {
			Message(Chat::White, "Guild successfully deleted.");
		}
	}
}

void Client::Handle_OP_GuildDemote(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GuildDemoteStruct)) {
		LogGuilds("Error: app size of [{}] != size of GuildDemoteStruct of [{}]\n", app->size, sizeof(GuildDemoteStruct));
		return;
	}

	if (!IsInAGuild()) {
		Message(Chat::Red, "Error: You aren't in a guild!");
		return;
	}

	GuildDemoteStruct* demote = (GuildDemoteStruct*)app->pBuffer;
	auto rank = demote->rank;
	auto target = demote->target;
	auto target_client = entity_list.GetClientByName(target);

	CharGuildInfo gci;
	if (!guild_mgr.GetCharInfo(demote->target, gci)) {
		Message(Chat::Red, "Unable to find '%s'", demote->target);
		return;
	}

	if (gci.rank == GUILD_LEADER) {
		Message(Chat::Red, "You cannot demote the guild leader.");
		return;
	}

	if (gci.guild_id != GuildID()) {
		Message(Chat::Red, "You aren't in the same guild, what do you think you are doing?");
		return;
	}

	if (rank > GUILD_RECRUIT) {
		Message(Chat::Red, "%s cannot be demoted any further!", demote->target);
		return;
	}

	if (((strcasecmp(GetCleanName(), target) == 0 &&
		guild_mgr.CheckPermission(GuildID(), GuildRank(), GUILD_ACTION_MEMBERS_DEMOTE_SELF)) ||
		((guild_mgr.CheckPermission(GuildID(), GuildRank(), GUILD_ACTION_MEMBERS_DEMOTE)) ||
		(ClientVersion() < EQ::versions::ClientVersion::RoF && GuildRank() <= GUILD_OFFICER)) &&
		gci.rank > GuildRank()))
	{
		if (!guild_mgr.SetGuildRank(gci.char_id, rank))
		{
			Message(Chat::Red, "Error while setting rank %d on '%s'.", rank, demote->target);
			LogGuilds("Demoting [{}] ([{}]) from rank [{}] ([{}]) to [{}] ([{}]) in [{}] ([{}]) FAILED.",
				demote->target,
				gci.char_id,
				guild_mgr.GetRankName(GuildID(), gci.rank),
				gci.rank,
				guild_mgr.GetRankName(GuildID(), rank),
				rank,
				guild_mgr.GetGuildName(GuildID()),
				GuildID()
			);
			return;
		}

		auto c = entity_list.GetClientByName(demote->target);
		if (c) {
			c->SetGuildRank(rank);
			c->SendAppearancePacket(AppearanceType::GuildRank, rank, false);
		}

		bool banker_status = guild_mgr.GetGuildBankerStatus(gci.guild_id, rank);
		if (gci.banker != banker_status) {
			gci.banker = banker_status;
		}
		guild_mgr.UpdateDbBankerFlag(gci.char_id, gci.banker);
		guild_mgr.MemberRankUpdate(gci.guild_id, rank, gci.banker, gci.alt, false, demote->target);

		LogGuilds("Demoting [{}] ([{}]) from rank [{}] ([{}]) to [{}] ([{}]) in [{}] ([{}])",
				  demote->target,
				  gci.char_id,
				  guild_mgr.GetRankName(GuildID(), gci.rank),
				  gci.rank,
				  guild_mgr.GetRankName(GuildID(), rank),
				  rank,
				  guild_mgr.GetGuildName(GuildID()),
				  GuildID()
		);
	}
	else {
		Message(Chat::Red, "You do not have sufficient privileges to demote '%s'.", target);
	}
}

void Client::Handle_OP_GuildInvite(const EQApplicationPacket *app)
{
	LogGuilds("Received OP_GuildInvite");

	if (app->size != sizeof(GuildCommand_Struct)) {
		std::cout << "Wrong size: OP_GuildInvite, size=" << app->size << ", expected " << sizeof(GuildCommand_Struct) << std::endl;
		return;
	}

	GuildCommand_Struct* gc = (GuildCommand_Struct*)app->pBuffer;
	auto rank = gc->officer;
	if (ClientVersion() < EQ::versions::ClientVersion::RoF) {
		switch (gc->officer) {
			case GUILD_MEMBER_TI: {
				rank = GUILD_MEMBER;
				break;
			}
			case GUILD_OFFICER_TI: {
				rank = GUILD_OFFICER;
				break;
			}
			case GUILD_LEADER_TI: {
				rank = GUILD_LEADER;
				break;
			}
			default: {
				rank = GUILD_RANK_NONE;
				break;
			}
		}
	}

	if (!IsInAGuild()) {
		Message(Chat::Red, "Error: You are not in a guild!");
		return;
	}

	if (!guild_mgr.CheckPermission(GuildID(), GuildRank(), GUILD_ACTION_MEMBERS_INVITE) ||
			 (ClientVersion() < EQ::versions::ClientVersion::RoF && GuildRank() > GUILD_OFFICER)) {
		Message(Chat::Red, "Invalid rank.");
		return;
	}
	//ok, the invite is also used for changing rank as well.
	Mob *invitee = entity_list.GetMob(gc->othername);

	if (!invitee) {
		Message(
			Chat::Red,
			"Prospective guild member %s must be in zone to perform guild operations on them.",
			gc->othername
		);
		return;
	}

	if (invitee->IsClient()) {
		Client *client = invitee->CastToClient();

		//ok, figure out what they are trying to do.
		if (client && client->GuildID() == GuildID()) {
			//they are already in this guild, must be a promotion or demotion
			if (rank > client->GuildRank()) {
				//demotion
				if (!guild_mgr.CheckPermission(GuildID(), GuildRank(), GUILD_ACTION_MEMBERS_DEMOTE) ||
					(ClientVersion() < EQ::versions::ClientVersion::RoF && GuildRank() > GUILD_OFFICER)) {
					Message(Chat::Red, "You don't have permission to demote.");
					return;
				}

				//we could send this to the member and prompt them to see if they want to
				//be demoted (I guess), but I dont see a point in that.

				LogGuilds("[{}] ([{}]) is demoting [{}] ([{}]) to rank [{}] in guild [{}] ([{}])",
						  GetName(), CharacterID(),
						  client->GetName(), client->CharacterID(),
						  rank,
						  guild_mgr.GetGuildName(GuildID()), GuildID());

				if (!guild_mgr.SetGuildRank(client->CharacterID(), rank)) {
					Message(Chat::Red, "There was an error during the demotion, DB may now be inconsistent.");
					return;
				}

			}
			else if (rank < client->GuildRank()) {
				//promotion
				if (!guild_mgr.CheckPermission(GuildID(), GuildRank(), GUILD_ACTION_MEMBERS_PROMOTE) ||
					(ClientVersion() < EQ::versions::ClientVersion::RoF && GuildRank() > GUILD_OFFICER)
					) {
					Message(Chat::Red, "You don't have permission to demote.");
					return;
				}

				LogGuilds("[{}] ([{}]) is asking to promote [{}] ([{}]) to rank [{}] in guild [{}] ([{}])",
						  GetName(), CharacterID(),
						  client->GetName(), client->CharacterID(),
						  rank,
						  guild_mgr.GetGuildName(GuildID()), GuildID());

				//record the promotion with guild manager so we know its valid when we get the reply
				guild_mgr.RecordInvite(client->CharacterID(), GuildID(), rank);

				if (gc->guildeqid == 0) {
					gc->guildeqid = GuildID();
				}

				LogGuilds("Sending OP_GuildInvite for promotion to [{}], length [{}]", client->GetName(), app->size);

				// Convert Membership Level between RoF and previous clients.
				if (client->ClientVersion() >= EQ::versions::ClientVersion::RoF) {
					gc->officer = rank;
					if (client->ClientVersion() >= EQ::versions::ClientVersion::RoF &&
						ClientVersion() < EQ::versions::ClientVersion::RoF) {
						auto               outapp = new EQApplicationPacket(
							OP_GuildPromote,
							sizeof(GuildPromoteStruct));
						GuildPromoteStruct *gps   = (GuildPromoteStruct *) outapp->pBuffer;
						strn0cpy(gps->name, gc->myname, sizeof(gps->name));
						strn0cpy(gps->target, gc->othername, sizeof(gps->target));
						gps->myrank               = GuildRank();
						gps->rank                 = rank;
						Handle_OP_GuildPromote(outapp);
						safe_delete(outapp);
						return;
					}
				}
				client->QueuePacket(app);
			}
			else {
				Message(Chat::Red, "That member is already that rank.");
				return;
			}
		}
		else if (client && !client->IsInAGuild()) {
			//they are not in this or any other guild, this is an invite
			//
			if (client->GetPendingGuildInvitation()) {
				Message(Chat::Red, "That person is already considering a guild invitation.");
				return;
			}

			if (!guild_mgr.CheckPermission(GuildID(), GuildRank(), GUILD_ACTION_MEMBERS_INVITE) ||
				(ClientVersion() < EQ::versions::ClientVersion::RoF && GuildRank() > GUILD_OFFICER)) {
				Message(Chat::Red, "You don't have permission to invite.");
				return;
			}

			LogGuilds("Inviting [{}] ([{}]) into guild [{}] ([{}])",
					  client->GetName(), client->CharacterID(),
					  guild_mgr.GetGuildName(GuildID()), GuildID());

			//record the invite with guild manager so we know its valid when we get the reply
			guild_mgr.RecordInvite(client->CharacterID(), GuildID(), rank);

			if (gc->guildeqid == 0) {
				gc->guildeqid = GuildID();
			}

			// Convert Membership Level between RoF and previous clients.
			if (client->ClientVersion() < EQ::versions::ClientVersion::RoF &&
				ClientVersion() >= EQ::versions::ClientVersion::RoF) {
				gc->officer = GUILD_MEMBER_TI;
			}
			if (client->ClientVersion() >= EQ::versions::ClientVersion::RoF &&
				ClientVersion() < EQ::versions::ClientVersion::RoF) {
				gc->officer = GUILD_RECRUIT;
			}

			LogGuilds("Sending OP_GuildInvite for invite to [{}], length [{}]", client->GetName(), app->size);
			client->SetPendingGuildInvitation(true);
			client->QueuePacket(app);

		}
		else {
			//they are in some other guild
			Message(Chat::Red, "Player is in a guild.");
			return;
		}
	}
}

void Client::Handle_OP_GuildInviteAccept(const EQApplicationPacket *app)
{
	LogGuilds("Received OP_GuildInviteAccept");

	SetPendingGuildInvitation(false);

	if (app->size != sizeof(GuildInviteAccept_Struct)) {
		LogGuilds("Wrong size: OP_GuildInviteAccept, size={}, expected {}.", app->size, sizeof(GuildJoin_Struct));
		return;
	}

	GuildInviteAccept_Struct* gj = (GuildInviteAccept_Struct*)app->pBuffer;
	auto invitor  = gj->inviter;
	auto invitee  = gj->new_member;
	auto guild_id = gj->guild_id;
	auto response = gj->response;

	if (ClientVersion() < EQ::versions::ClientVersion::RoF) {
		switch (response) {
		case GUILD_MEMBER_TI:
		{
			response = GUILD_RECRUIT;
			break;
		}
		case GUILD_OFFICER_TI:
		{
			response = GUILD_OFFICER;
			break;
		}
		default:
		{
			response = GUILD_RANK_NONE;
			break;
		}
		}
	}

	Client* c_invitor = entity_list.GetClientByName(invitor);
	Client* c_invitee = entity_list.GetClientByName(invitee);
	if (!c_invitee) {
		return;
	}

	if (!c_invitee || !c_invitor) {
		guild_mgr.VerifyAndClearInvite(CharacterID(), guild_id, gj->response);
		Message(Chat::Yellow, "Both players must be in the same zone.");
		if (c_invitor) {
			c_invitor->Message(Chat::Yellow, "Both players must be in the same zone.");
		}
		return;
	}

	if (response >= GUILD_INVITE_DECLINE) {
		guild_mgr.VerifyAndClearInvite(CharacterID(), guild_id, gj->response);
		Message(Chat::Yellow, "You declined the guild invite.");
		if (c_invitor) {
			c_invitor->Message(Chat::Yellow, "The player declined the guild invite.");
		}
		return;
	}

	if (c_invitor && IsInAGuild() && GuildID() != c_invitor->GuildID()) {
		guild_mgr.VerifyAndClearInvite(CharacterID(), guild_id, gj->response);
		Message(Chat::Yellow, "You are already in a guild.  Please leave that guild first.");
		c_invitor->Message(Chat::Yellow, "Player is already in a guild.");
		return;
		}

	guild_mgr.VerifyAndClearInvite(CharacterID(), guild_id, gj->response);
	c_invitee->SetGuildID(guild_id);
	c_invitee->SetGuildListDirty(false);
	guild_mgr.MemberAdd(guild_id, c_invitee->CharacterID(), c_invitee->GetLevel(), c_invitee->GetClass(), response, c_invitee->GetZoneID(), std::string(c_invitee->GetCleanName()));
	SendGuildSpawnAppearance();

	LogGuilds("Adding [{}] ([{}]) to guild [{}] ([{}]) at rank [{}]",
		GetName(),
		CharacterID(),
		guild_mgr.GetGuildName(guild_id),
		guild_id,
		response
	);
}

void Client::Handle_OP_GuildLeader(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GuildMakeLeader_Struct)) {
		LogError("Received OP_Handle_GuildLeader packet of size {} which should be {}", app->size, sizeof(GuildMakeLeader_Struct));
		return;
	}

	GuildMakeLeader_Struct* gcl = (GuildMakeLeader_Struct*)app->pBuffer;
	auto new_leader = entity_list.GetClientByName(gcl->new_leader);
	if (!new_leader) {
		Message(Chat::Red, "Both players must be in the same zone to transfer guild leadership.");
		return;
	}

	if (GuildRank() != GUILD_LEADER) {
		Message(Chat::Red, "You must be the guild leader to reassign guild leadership.");
		return;
	}

	if (guild_mgr.SetGuildLeader(GuildID(), new_leader->CharacterID())) {
		Message(Chat::White, "Successfully Transfered Leadership to %s.", new_leader->GetCleanName());
		new_leader->Message(Chat::Yellow, "%s has transfered the guild leadership into your hands.", GetName());

		SetGuildRank(GUILD_OFFICER);
		guild_mgr.MemberRankUpdate(guild_id, GUILD_OFFICER, false, false, false, GetName());

		new_leader->SetGuildRank(GUILD_LEADER);
		guild_mgr.MemberRankUpdate(guild_id, GUILD_LEADER, true, false, false, new_leader->GetName());

		LogGuilds("Transfering leadership of [{}] ([{}]) to [{}] ([{}])",
			guild_mgr.GetGuildName(GuildID()),
			GuildID(),
			new_leader->GetCleanName(),
			new_leader->CharacterID()
		);
	}
			else
	{
		Message(Chat::Red, "Could not change leadership at this time.");
		LogGuilds("Tranfer of guild leadership for guild id {} failed.", GuildID());
	}
	return;
}

void Client::Handle_OP_GuildManageBanker(const EQApplicationPacket *app)
{

	LogGuilds("Got OP_GuildManageBanker of len [{}]", app->size);
	if (app->size != sizeof(GuildManageBanker_Struct)) {
		LogGuilds("Error: app size of [{}] != size of OP_GuildManageBanker of [{}]\n", app->size, sizeof(GuildManageBanker_Struct));
		return;
	}
	GuildManageBanker_Struct* gmb = (GuildManageBanker_Struct*)app->pBuffer;

	if (!IsInAGuild()) {
		Message(Chat::Red, "Your not in a guild!");
		return;
	}

	CharGuildInfo gci;

	if (!guild_mgr.GetCharInfo(gmb->member, gci))
	{
		Message(Chat::Red, "Unable to find '%s'", gmb->member);
		return;
	}
	bool IsCurrentlyABanker = guild_mgr.GetBankerFlag(gci.char_id, true);

	bool IsCurrentlyAnAlt = guild_mgr.GetAltFlag(gci.char_id);

	bool NewBankerStatus = gmb->enabled & 0x01;

	bool NewAltStatus = gmb->enabled & 0x02;

	if ((IsCurrentlyABanker != NewBankerStatus) && !guild_mgr.IsGuildLeader(GuildID(), CharacterID()))
	{
		Message(Chat::Red, "Only the guild leader can assign guild bankers!");
		return;
	}

	if (IsCurrentlyAnAlt != NewAltStatus)
	{
		bool IsAllowed = !strncasecmp(GetName(), gmb->member, strlen(GetName())) ||
			guild_mgr.CheckPermission(GuildID(), GuildRank(), GUILD_ACTION_MEMBERS_CHANGE_ALT_FLAG_FOR_OTHER);

		if (!IsAllowed)
		{
			Message(Chat::Red, "You are not allowed to change the alt status of %s", gmb->member);
			return;
		}
	}

	if (gci.guild_id != GuildID()) {
		Message(Chat::Red, "You aren't in the same guild, what do you think you are doing?");
		return;
	}

	if (IsCurrentlyABanker != NewBankerStatus)
	{
		gci.banker = NewBankerStatus;
		if (!guild_mgr.SetBankerFlag(gci.char_id, NewBankerStatus)) {
			Message(Chat::Red, "Error setting guild banker flag.");
			return;
		}

		if (NewBankerStatus)
			Message(Chat::White, "%s has been made a guild banker.", gmb->member);
		else
			Message(Chat::White, "%s is no longer a guild banker.", gmb->member);
	}
	if (IsCurrentlyAnAlt != NewAltStatus)
	{
		gci.alt = NewAltStatus;
		if (!guild_mgr.SetAltFlag(gci.char_id, NewAltStatus)) {
			Message(Chat::Red, "Error setting guild alt flag.");
			return;
		}

		if (NewAltStatus)
			Message(Chat::White, "%s has been marked as an alt.", gmb->member);
		else
			Message(Chat::White, "%s is no longer marked as an alt.", gmb->member);
	}
	guild_mgr.MemberRankUpdate(gci.guild_id, gci.rank, gci.banker, gci.alt, false, gmb->member);
}

void Client::Handle_OP_GuildPeace(const EQApplicationPacket *app)
{
	LogGuilds("Got OP_GuildPeace of len [{}]", app->size);
	return;
}

void Client::Handle_OP_GuildPromote(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GuildPromoteStruct)) {
		LogGuilds("Error: app size of [{}] != size of GuildDemoteStruct of [{}]\n", app->size, sizeof(GuildPromoteStruct));
		return;
	}

	if (!IsInAGuild()) {
		Message(Chat::Red, "Error: You aren't in a guild!");
	}
	else if (!guild_mgr.CheckPermission(GuildID(), GuildRank(), GUILD_ACTION_MEMBERS_PROMOTE) ||
			 (ClientVersion() < EQ::versions::ClientVersion::RoF && GuildRank() > GUILD_OFFICER)) {
		Message(Chat::Red, "You don't have permission to promote.");
	}
	else if (!worldserver.Connected()) {
		Message(Chat::Red, "Error: World server disconnected");
	}
	else {
		auto promote = (GuildPromoteStruct *) app->pBuffer;

		auto rank = promote->myrank;
		if (ClientVersion() < EQ::versions::ClientVersion::RoF) {
			switch (rank) {
				case GUILD_MEMBER_TI: {
					rank = GUILD_MEMBER;
					break;
				}
				case GUILD_OFFICER_TI: {
					rank = GUILD_OFFICER;
					break;
				}
				case GUILD_LEADER_TI: {
					rank = GUILD_LEADER;
					break;
				}
				default: {
					rank = GUILD_RANK_NONE;
					break;
				}
			}
		}

		CharGuildInfo gci;
		if (!guild_mgr.GetCharInfo(promote->target, gci)) {
			Message(Chat::Red, "Unable to find '%s'", promote->target);
			return;
		}
		if (gci.guild_id != GuildID()) {
			Message(Chat::Red, "You aren't in the same guild, what do you think you are doing?");
			return;
		}

		if (rank == GUILD_LEADER)
		{
			Message(Chat::Red, "You cannot promote someone to be guild leader. You must use /guildleader.");
			return;
		}

		LogGuilds("Promoting [{}] ([{}]) from rank [{}] ([{}]) to [{}] ([{}]) in [{}] ([{}])",
			promote->target, gci.char_id,
			guild_mgr.GetRankName(GuildID(), gci.rank), gci.rank,
			guild_mgr.GetRankName(GuildID(), rank), rank,
			guild_mgr.GetGuildName(GuildID()), GuildID());

		if (!guild_mgr.SetGuildRank(gci.char_id, rank)) {
			Message(Chat::Red, "Error while setting rank %d on '%s'.", rank, promote->target);
			return;
		}

		auto c = entity_list.GetClientByName(promote->target);
		if (c) {
			c->SetGuildRank(rank);
			c->SendAppearancePacket(AppearanceType::GuildRank, rank, false);
		}

		bool banker_status = guild_mgr.GetGuildBankerStatus(gci.guild_id, rank);

		guild_mgr.UpdateDbBankerFlag(gci.char_id, gci.banker);
		guild_mgr.MemberRankUpdate(gci.guild_id, rank, gci.banker, gci.alt, false, promote->target);
	}
	return;
}

void Client::Handle_OP_GuildPublicNote(const EQApplicationPacket *app)
{
	LogGuilds("Received OP_GuildPublicNote");

	if (app->size != sizeof(GuildUpdate_PublicNote)) {
		LogGuilds("Received packet to update a guild public note for {} of wrong size.", GetCleanName());
		return;
	}

	GuildUpdate_PublicNote* gpn = (GuildUpdate_PublicNote*)app->pBuffer;

	CharGuildInfo gci;
	if (!guild_mgr.GetCharInfo(gpn->target, gci)) {
		Message(Chat::Red, "Unable to find '%s'", gpn->target);
		return;
	}
	if (gci.guild_id != GuildID()) {
		Message(Chat::Red, "You aren't in the same guild, what do you think you are doing?");
		return;
	}

	if (!guild_mgr.CheckPermission(GuildID(), GuildRank(), GUILD_ACTION_EDIT_PUBLIC_NOTES) ||
		(ClientVersion() < EQ::versions::ClientVersion::RoF && GuildRank() > GUILD_OFFICER))
	{
		Message(Chat::Red, "You do not have access to update public guild notes.");
		return;
	}

	LogGuilds("Setting public note on [{}] ([{}]) in guild [{}] ([{}]) to: [{}]",
		gpn->target, gci.char_id,
		guild_mgr.GetGuildName(GuildID()), GuildID(),
		gpn->note);

	if (!guild_mgr.SetPublicNote(gci.char_id, std::string(gpn->note))) {
		Message(Chat::Red, "Failed to set public note on %s", gpn->target);
	}
	else {
		Message(Chat::White, "Successfully changed public note on %s", gpn->target);
		guild_mgr.SendToWorldMemberPublicNote(GuildID(), gpn->target, gpn->note);
	}
}

void Client::Handle_OP_GuildRemove(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GuildCommand_Struct)) {
		LogGuilds("Wrong size: OP_GuildRemove, size = {} expected {}.",
			app->size,
			sizeof(GuildCommand_Struct)
		);
		return;
	}

	GuildCommand_Struct* gc = (GuildCommand_Struct*)app->pBuffer;

	if (!IsInAGuild()) {
		Message(Chat::Red, "Error: You aren't in a guild!");
	}
	else if ((strcasecmp(gc->othername, GetName()) != 0 &&
		!guild_mgr.CheckPermission(GuildID(), GuildRank(), GUILD_ACTION_MEMBERS_REMOVE)) ||
		(ClientVersion() < EQ::versions::ClientVersion::RoF && GuildRank() > GUILD_OFFICER) &&
		strcasecmp(gc->othername, GetName()) != 0) {
		Message(Chat::Red, "You don't have permission to remove guild members.");
	}
	else {
		uint32 char_id;
		Client* client = entity_list.GetClientByName(gc->othername);

		if (client) {
			if (!client->IsInGuild(GuildID())) {
				Message(Chat::Red, "You aren't in the same guild, what do you think you are doing?");
				return;
			}
			guild_mgr.RemoveMember(client->GuildID(), client->CharacterID(), std::string(client->GetCleanName()));
			LogGuilds("Removing [{}] ([{}]) from guild [{}] ([{}])",
				client->GetName(), client->CharacterID(),
				guild_mgr.GetGuildName(GuildID()), GuildID());
		}
		else {
			CharGuildInfo gci;
			if (!guild_mgr.GetCharInfo(gc->othername, gci)) {
				Message(Chat::Red, "Unable to find '%s'", gc->othername);
				return;
			}
			if (gci.guild_id != GuildID()) {
				Message(Chat::Red, "You aren't in the same guild, what do you think you are doing?");
				return;
			}
			char_id = gci.char_id;

			LogGuilds("Removing remote/offline [{}] ([{}]) into guild [{}] ([{}])",
				gci.char_name.c_str(), gci.char_id,
				guild_mgr.GetGuildName(GuildID()), GuildID());
			guild_mgr.RemoveMember(gci.guild_id, gci.char_id, gci.char_name);
		}
		}
	return;
}

void Client::Handle_OP_GuildStatus(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GuildStatus_Struct))
	{
		LogDebug("Size mismatch in OP_GuildStatus expected [{}] got [{}]", sizeof(GuildStatus_Struct), app->size);

		DumpPacket(app);

		return;
	}
	GuildStatus_Struct *gss = (GuildStatus_Struct*)app->pBuffer;

	Client *c = entity_list.GetClientByName(gss->Name);

	if (!c) {
		MessageString(Chat::LightGray, TARGET_PLAYER_FOR_GUILD_STATUS);
		return;
	}

	uint32 TargetGuildID = c->GuildID();

	if (TargetGuildID == GUILD_NONE)
	{
		MessageString(Chat::LightGray, NOT_IN_A_GUILD, c->GetName());
		return;
	}

	const char *GuildName = guild_mgr.GetGuildName(TargetGuildID);

	if (!GuildName)
		return;

	auto rank = c->GuildRank();
	switch (rank) {
		case GUILD_LEADER: {
			MessageString(Chat::LightGray, LEADER_OF_X_GUILD, c->GetName(), GuildName);
			break;
		}
		case GUILD_SENIOR_OFFICER:
		case GUILD_OFFICER: {
			MessageString(Chat::LightGray, OFFICER_OF_X_GUILD, c->GetName(), GuildName);
			break;
		}
		case GUILD_SENIOR_MEMBER:
		case GUILD_MEMBER:
		case GUILD_JUNIOR_MEMBER:
		case GUILD_INITIATE:
		case GUILD_RECRUIT: {
			MessageString(Chat::LightGray, MEMBER_OF_X_GUILD, c->GetName(), GuildName);
			break;
		}
	}
}

void Client::Handle_OP_GuildUpdate(const EQApplicationPacket *app)
{
	if (!IsInAGuild()) {
		return;
	}

	GuildUpdateUCPStruct *gup = (GuildUpdateUCPStruct *) app->pBuffer;

	switch (gup->action) {
		case GuildUpdateURL: {
			if (app->size != sizeof(GuildUpdateURLAndChannel_Struct)) {
				LogDebug("Size mismatch in OP_GuildUpdateURLAndChannel expected [{}] got [{}]",
						 sizeof(GuildUpdateURLAndChannel_Struct),
						 app->size);
				DumpPacket(app);
				return;
			}

			guild_mgr.SetGuildURL(GuildID(), gup->payload.url_channel.text);
			guild_mgr.SendToWorldGuildURL(GuildID(), gup->payload.url_channel.text);

			break;
		}
		case GuildUpdateChannel: {
			if (app->size != sizeof(GuildUpdateURLAndChannel_Struct)) {
				LogDebug("Size mismatch in OP_GuildUpdateURLAndChannel expected [{}] got [{}]",
						 sizeof(GuildUpdateURLAndChannel_Struct),
						 app->size);
				DumpPacket(app);
				return;
			}

			guild_mgr.SetGuildChannel(GuildID(), gup->payload.url_channel.text);
			guild_mgr.SendToWorldGuildChannel(GuildID(), gup->payload.url_channel.text);
			break;
		}
		case GuildUpdateRanks: {
			if (!guild_mgr.CheckPermission(guild_id, guildrank, GUILD_ACTION_RANKS_CHANGE_RANK_NAMES)) {
				MessageString(Chat::Yellow, GUILD_PERMISSION_FAILED);
				return;
			}
			auto        rank = gup->payload.rank_name.rank;
			std::string rank_name(gup->payload.rank_name.rank_name);

			if (rank > GUILD_MAX_RANK || rank < 0 || rank_name.empty()) {
				LogGuilds("Received packet to update rank though rank {} or rank name {} was incorrect.",
						  rank,
						  rank_name.c_str());
				return;
			}
			guild_mgr.UpdateRankName(guild_id, rank, rank_name);
			guild_mgr.SendRankName(guild_id, rank, rank_name);

			break;
		}
		case GuildUpdatePermissions: {
			if (!guild_mgr.CheckPermission(guild_id, guildrank, GUILD_ACTION_RANKS_CHANGE_PERMISSIONS)) {
				MessageString(Chat::Yellow, GUILD_PERMISSION_FAILED);
				return;
			}
			guild_mgr.UpdateRankPermission(
				guild_id,
				character_id,
				gup->payload.permissions.function_id,
				gup->payload.permissions.rank,
				gup->payload.permissions.value
			);
			guild_mgr.SendPermissionUpdate(
				guild_id,
				gup->payload.permissions.rank,
				gup->payload.permissions.function_id,
				gup->payload.permissions.value
			);

			break;
		}
		default:
			break;
	}
}

void Client::Handle_OP_GuildWar(const EQApplicationPacket *app)
{
	LogGuilds("Got OP_GuildWar of len [{}]", app->size);
	return;
}

void Client::Handle_OP_Heartbeat(const EQApplicationPacket *app)
{
	return;
}

void Client::Handle_OP_Hide(const EQApplicationPacket *app)
{
	// newer client respond to OP_CancelSneakHide with OP_Hide with a size of 4 and 0 data
	if (app->size == 4) {
		auto data = app->ReadUInt32(0);
		if (data)
			LogDebug("Got OP_Hide with unexpected data [{}]", data);
		return;
	}

	if (!HasSkill(EQ::skills::SkillHide) && GetSkill(EQ::skills::SkillHide) == 0)
	{
		//Can not be able to train hide but still have it from racial though
		return; //You cannot hide if you do not have hide
	}

	if (!p_timers.Expired(&database, pTimerHide, false)) {
		Message(Chat::Red, "Ability recovery time not yet met.");
		return;
	}
	int reuse = HideReuseTime - GetSkillReuseTime(EQ::skills::SkillHide);

	if (reuse < 1)
		reuse = 1;

	p_timers.Start(pTimerHide, reuse - 1);

	float hidechance = ((GetSkill(EQ::skills::SkillHide) / 250.0f) + .25) * 100;
	float random = zone->random.Real(0, 100);
	CheckIncreaseSkill(EQ::skills::SkillHide, nullptr, 5);
	if (random < hidechance) {
		auto outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
		SpawnAppearance_Struct* sa_out = (SpawnAppearance_Struct*)outapp->pBuffer;
		sa_out->spawn_id = GetID();
		sa_out->type = 0x03;
		sa_out->parameter = 1;
		entity_list.QueueClients(this, outapp, true);
		safe_delete(outapp);
		if (spellbonuses.ShroudofStealth || aabonuses.ShroudofStealth || itembonuses.ShroudofStealth) {
			improved_hidden = true;
			hidden = true;
		}
		else
			hidden = true;
		tmHidden = Timer::GetCurrentTime();
	}
	if (GetClass() == Class::Rogue) {
		auto outapp = new EQApplicationPacket(OP_SimpleMessage, sizeof(SimpleMessage_Struct));
		SimpleMessage_Struct *msg = (SimpleMessage_Struct *)outapp->pBuffer;
		msg->color = 0x010E;
		Mob *evadetar = GetTarget();
		if (!auto_attack && (evadetar && evadetar->CheckAggro(this)
			&& evadetar->IsNPC())) {
			if (zone->random.Int(0, 260) < (int)GetSkill(EQ::skills::SkillHide)) {
				msg->string_id = EVADE_SUCCESS;
				RogueEvade(evadetar);
			}
			else {
				msg->string_id = EVADE_FAIL;
			}
		}
		else {
			if (hidden) {
				msg->string_id = HIDE_SUCCESS;
			}
			else {
				msg->string_id = HIDE_FAIL;
			}
		}
		FastQueuePacket(&outapp);
	}
	return;
}

void Client::Handle_OP_HideCorpse(const EQApplicationPacket *app)
{
	// New OPCode for SOD+ as /hidecorpse is handled serverside now.
	//
	if (app->size != sizeof(HideCorpse_Struct))
	{
		LogDebug("Size mismatch in OP_HideCorpse expected [{}] got [{}]", sizeof(HideCorpse_Struct), app->size);

		DumpPacket(app);

		return;
	}

	HideCorpse_Struct *hcs = (HideCorpse_Struct*)app->pBuffer;

	if (hcs->Action == HideCorpseLooted)
		return;

	if ((HideCorpseMode == HideCorpseNone) && (hcs->Action == HideCorpseNone))
		return;

	entity_list.HideCorpses(this, HideCorpseMode, hcs->Action);

	HideCorpseMode = hcs->Action;
}

void Client::Handle_OP_Ignore(const EQApplicationPacket *app)
{
	return;
}

void Client::Handle_OP_Illusion(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Illusion_Struct)) {
		LogError("Received invalid sized OP_Illusion: got [{}], expected [{}]", app->size, sizeof(Illusion_Struct));
		DumpPacket(app);
		return;
	}

	if (!GetGM()) {
		RecordPlayerEventLog(
			PlayerEvent::POSSIBLE_HACK,
			PlayerEvent::PossibleHackEvent{.message = "OP_Illusion sent by non Game Master"}
		);

		return;
	}

	Illusion_Struct* bnpc = (Illusion_Struct*)app->pBuffer;
	//these need to be implemented
	/*
	texture		= bnpc->texture;
	helmtexture	= bnpc->helmtexture;
	luclinface	= bnpc->luclinface;
	*/
	race = bnpc->race;
	size = 0;

	entity_list.QueueClients(this, app);
	return;
}

void Client::Handle_OP_InspectAnswer(const EQApplicationPacket *app)
{
	if (app->size != sizeof(InspectResponse_Struct)) {
		LogError("Wrong size: OP_InspectAnswer, size=[{}], expected [{}]", app->size, sizeof(InspectResponse_Struct));
		return;
	}

	//Fills the app sent from client.
	auto               outapp = app->Copy();
	auto               insr   = (InspectResponse_Struct *) outapp->pBuffer;
	auto               tmp    = entity_list.GetMob(insr->TargetID);
	const EQ::ItemData *item  = nullptr;

	for (int16 L                    = EQ::invslot::EQUIPMENT_BEGIN; L <= EQ::invslot::EQUIPMENT_END; L++) {
		const auto inst = GetInv().GetItem(L);
		item = inst ? inst->GetItem() : nullptr;

		if (item) {
			strcpy(insr->itemnames[L], item->Name);
			if (inst) {
				const auto augment = inst->GetOrnamentationAugment();

				if (augment) {
					insr->itemicons[L] = augment->GetItem()->Icon;
				} else if (inst->GetOrnamentationIcon()) {
					insr->itemicons[L] = inst->GetOrnamentationIcon();
				}
			} else {
				insr->itemicons[L] = item->Icon;
			}
		} else {
			insr->itemicons[L] = 0xFFFFFFFF;
		}
	}

	auto message         = (InspectMessage_Struct *) insr->text;
	auto inspect_message = GetInspectMessage();

	memcpy(&inspect_message, message, sizeof(InspectMessage_Struct));
	database.SaveCharacterInspectMessage(CharacterID(), &inspect_message);

	if (
		tmp &&
		tmp->IsClient()
	) {
		tmp->CastToClient()->QueuePacket(outapp);
	} // Send answer to requester
}

void Client::Handle_OP_InspectMessageUpdate(const EQApplicationPacket *app)
{

	if (app->size != sizeof(InspectMessage_Struct)) {
		LogError("Wrong size: OP_InspectMessageUpdate, size=[{}], expected [{}]", app->size, sizeof(InspectMessage_Struct));
		return;
	}

	InspectMessage_Struct* newmessage = (InspectMessage_Struct*)app->pBuffer;
	InspectMessage_Struct& playermessage = GetInspectMessage();
	memcpy(&playermessage, newmessage, sizeof(InspectMessage_Struct));
	database.SaveCharacterInspectMessage(CharacterID(), &playermessage);
}

void Client::Handle_OP_InspectRequest(const EQApplicationPacket *app)
{

	if (app->size != sizeof(Inspect_Struct)) {
		LogError("Wrong size: OP_InspectRequest, size=[{}], expected [{}]", app->size, sizeof(Inspect_Struct));
		return;
	}

	Inspect_Struct* ins = (Inspect_Struct*)app->pBuffer;
	Mob* tmp = entity_list.GetMob(ins->TargetID);

	if (tmp != 0 && tmp->IsClient()) {
		if (tmp->CastToClient()->ClientVersion() < EQ::versions::ClientVersion::SoF) { tmp->CastToClient()->QueuePacket(app); } // Send request to target
																																   // Inspecting an SoF or later client will make the server handle the request
		else { ProcessInspectRequest(tmp->CastToClient(), this); }
	}

	if (tmp != 0 && tmp->IsBot()) { Bot::ProcessBotInspectionRequest(tmp->CastToBot(), this); }

	return;
}

void Client::Handle_OP_InstillDoubt(const EQApplicationPacket *app)
{
	//packet is empty as of 12/14/04

	if (!p_timers.Expired(&database, pTimerInstillDoubt, false)) {
		Message(Chat::Red, "Ability recovery time not yet met.");
		return;
	}
	p_timers.Start(pTimerInstillDoubt, InstillDoubtReuseTime - 1);

	InstillDoubt(GetTarget());
	return;
}

void Client::Handle_OP_ItemLinkClick(const EQApplicationPacket *app)
{
	if (app->size != sizeof(ItemViewRequest_Struct)) {
		LogError("Wrong size on OP_ItemLinkClick. Got: [{}], Expected: [{}]", app->size,
			sizeof(ItemViewRequest_Struct));
		DumpPacket(app);
		return;
	}

	ItemViewRequest_Struct *ivrs = (ItemViewRequest_Struct *)app->pBuffer;

	// todo: verify ivrs->link_hash based on a rule, in case we don't care about people being able to sniff data
	// from the item DB

	const EQ::ItemData *item = database.GetItem(ivrs->item_id);
	if (!item) {
		if (ivrs->item_id != SAYLINK_ITEM_ID) {
			Message(Chat::Red, "Error: The item for the link you have clicked on does not exist!");
			return;
		}
		// This new scheme will shuttle the ID in the first augment for non-silent links
		// and the second augment for silent.
		std::string response = "";
		bool silentsaylink = ivrs->augments[1] > 0 ? true : false;
		int sayid = silentsaylink ? ivrs->augments[1] : ivrs->augments[0];

		if (sayid > 0) {
			std::string query = StringFormat("SELECT `phrase` FROM saylink WHERE `id` = '%i'", sayid);
			auto results = database.QueryDatabase(query);
			if (!results.Success()) {
				Message(Chat::Red, "Error: The saylink (%s) was not found in the database.", response.c_str());
				return;
			}

			if (results.RowCount() != 1) {
				Message(Chat::Red, "Error: The saylink (%s) was not found in the database.", response.c_str());
				return;
			}

			auto row = results.begin();
			response = row[0];
		}

		if (!response.empty()) {
			if (!silentsaylink) {
				Message(Chat::LightGray, "You say, '%s'", response.c_str());
			}

			ChannelMessageReceived(ChatChannel_Say, Language::CommonTongue, Language::MaxValue, response.c_str(), nullptr, true);

			return;
		}
		else {
			Message(Chat::Red, "Error: Say Link not found or is too long.");
			return;
		}
	}

	EQ::ItemInstance *inst =
		database.CreateItem(item, item->MaxCharges, ivrs->augments[0], ivrs->augments[1], ivrs->augments[2],
			ivrs->augments[3], ivrs->augments[4], ivrs->augments[5]);
	if (inst) {
		SendItemPacket(0, inst, ItemPacketViewLink);
		safe_delete(inst);
	}
}

void Client::Handle_OP_ItemLinkResponse(const EQApplicationPacket *app)
{
	if (app->size != sizeof(LDONItemViewRequest_Struct)) {
		LogError("OP size error: OP_ItemLinkResponse expected:[{}] got:[{}]", sizeof(LDONItemViewRequest_Struct), app->size);
		return;
	}
	LDONItemViewRequest_Struct* item = (LDONItemViewRequest_Struct*)app->pBuffer;
	EQ::ItemInstance* inst = database.CreateItem(item->item_id);
	if (inst) {
		SendItemPacket(0, inst, ItemPacketViewLink);
		safe_delete(inst);
	}
	return;
}

void Client::Handle_OP_ItemName(const EQApplicationPacket *app)
{
	if (app->size != sizeof(ItemNamePacket_Struct)) {
		LogError("Invalid size for ItemNamePacket_Struct: Expected: [{}], Got: [{}]",
			sizeof(ItemNamePacket_Struct), app->size);
		return;
	}
	ItemNamePacket_Struct *p = (ItemNamePacket_Struct*)app->pBuffer;
	const EQ::ItemData *item = nullptr;
	if ((item = database.GetItem(p->item_id)) != nullptr) {
		auto outapp = new EQApplicationPacket(OP_ItemName, sizeof(ItemNamePacket_Struct));
		p = (ItemNamePacket_Struct*)outapp->pBuffer;
		memset(p, 0, sizeof(ItemNamePacket_Struct));
		strcpy(p->name, item->Name);
		FastQueuePacket(&outapp);
	}
	return;
}

void Client::Handle_OP_ItemPreview(const EQApplicationPacket *app)
{
	VERIFY_PACKET_LENGTH(OP_ItemPreview, app, ItemPreview_Struct);
	ItemPreview_Struct *ips = (ItemPreview_Struct *)app->pBuffer;

	const EQ::ItemData* item = database.GetItem(ips->itemid);

	if (item) {
		auto outapp = new EQApplicationPacket(OP_ItemPreview, strlen(item->Name) + strlen(item->Lore) +
			strlen(item->IDFile) + 898);

		int spacer;
		for (spacer = 0; spacer < 16; spacer++) {
			outapp->WriteUInt8(48);
		}
		outapp->WriteUInt16(256);
		for (spacer = 0; spacer < 7; spacer++) {
			outapp->WriteUInt8(0);
		}
		for (spacer = 0; spacer < 7; spacer++) {
			outapp->WriteUInt8(255);
		}
		outapp->WriteUInt32(0);
		outapp->WriteUInt32(1);
		outapp->WriteUInt32(0);
		outapp->WriteUInt8(237); // Seems to be some kind of counter? increases by 1 for each preview that you do.
		outapp->WriteUInt16(2041); //F907
		for (spacer = 0; spacer < 36; spacer++) {
			outapp->WriteUInt8(0);
		}
		for (spacer = 0; spacer < 4; spacer++) {
			outapp->WriteUInt8(255);
		}
		for (spacer = 0; spacer < 9; spacer++) {
			outapp->WriteUInt8(0);
		}
		for (spacer = 0; spacer < 5; spacer++) {
			outapp->WriteUInt8(255);
		}
		for (spacer = 0; spacer < 5; spacer++) {
			outapp->WriteUInt8(0);
		}
		outapp->WriteString(item->Name);
		outapp->WriteString(item->Lore);
		outapp->WriteUInt8(0);
		outapp->WriteUInt32(ips->itemid);
		outapp->WriteUInt32(item->Weight);
		outapp->WriteUInt8(item->NoRent);
		outapp->WriteUInt8(item->NoDrop);
		outapp->WriteUInt8(item->Attuneable);
		outapp->WriteUInt8(item->Size);
		outapp->WriteUInt32(item->Slots);
		outapp->WriteUInt32(item->Price);
		outapp->WriteUInt32(item->Icon);
		outapp->WriteUInt8(0); //Unknown?
		outapp->WriteUInt8(0); //Placeable flag?
		outapp->WriteUInt32(item->BenefitFlag);
		outapp->WriteUInt8(item->Tradeskills);
		outapp->WriteUInt8(item->CR);
		outapp->WriteUInt8(item->DR);
		outapp->WriteUInt8(item->PR);
		outapp->WriteUInt8(item->MR);
		outapp->WriteUInt8(item->FR);
		outapp->WriteUInt8(item->AStr);
		outapp->WriteUInt8(item->ASta);
		outapp->WriteUInt8(item->AAgi);
		outapp->WriteUInt8(item->ADex);
		outapp->WriteUInt8(item->ACha);
		outapp->WriteUInt8(item->AInt);
		outapp->WriteUInt8(item->AWis);
		outapp->WriteSInt32(item->HP);
		outapp->WriteSInt32(item->Mana);
		outapp->WriteSInt32(item->Endur);
		outapp->WriteSInt32(item->AC);
		outapp->WriteUInt32(item->Regen);
		outapp->WriteUInt32(item->ManaRegen);
		outapp->WriteSInt32(item->EnduranceRegen);
		outapp->WriteUInt32(item->Classes);
		outapp->WriteUInt32(item->Races);
		outapp->WriteUInt32(item->Deity);
		outapp->WriteUInt32(item->SkillModValue);
		outapp->WriteUInt32(0); //SkillModValue
		outapp->WriteUInt32(item->SkillModType);
		outapp->WriteUInt32(0); //SkillModExtra
		outapp->WriteUInt32(item->BaneDmgRace);
		outapp->WriteUInt32(item->BaneDmgBody);
		outapp->WriteUInt32(item->BaneDmgRaceAmt);
		outapp->WriteUInt32(item->BaneDmgAmt);
		outapp->WriteUInt8(item->Magic);
		outapp->WriteUInt32(item->CastTime_);
		outapp->WriteUInt32(item->ReqLevel);
		outapp->WriteUInt32(item->RecLevel);
		outapp->WriteUInt32(item->RecSkill);
		outapp->WriteUInt32(item->BardType);
		outapp->WriteUInt32(item->BardValue);
		outapp->WriteUInt8(item->Light);
		outapp->WriteUInt8(item->Delay);
		outapp->WriteUInt8(item->ElemDmgType);
		outapp->WriteUInt8(item->ElemDmgAmt);
		outapp->WriteUInt8(item->Range);
		outapp->WriteUInt32(item->Damage);
		outapp->WriteUInt32(item->Color);
		outapp->WriteUInt32(0);	// Prestige
		outapp->WriteUInt8(item->ItemType);
		outapp->WriteUInt32(item->Material);
		outapp->WriteUInt32(0); //unknown
		outapp->WriteUInt32(item->EliteMaterial);
		outapp->WriteUInt32(item->HerosForgeModel);
		outapp->WriteUInt32(0);	// unknown
		outapp->WriteUInt32(0); //This is unknown057 from lucy
		for (spacer = 0; spacer < 77; spacer++) { //More Item stats, but some seem to be off based on packet check
			outapp->WriteUInt8(0);
		}
		outapp->WriteUInt32(0xFFFFFFFF); //Unknown but always seen as FF FF FF FF
		outapp->WriteUInt32(0); //Unknown
		for (spacer = 0; spacer < 6; spacer++) { //Augment stuff
			outapp->WriteUInt32(item->AugSlotType[spacer]);
			outapp->WriteUInt8(item->AugSlotVisible[spacer]);
			outapp->WriteUInt8(item->AugSlotUnk2[spacer]);
		}
		outapp->WriteUInt32(0); //New RoF 6th Aug Slot
		outapp->WriteUInt8(1); //^
		outapp->WriteUInt8(0); //^^
		outapp->WriteUInt32(item->LDoNSold);
		outapp->WriteUInt32(item->LDoNTheme);
		outapp->WriteUInt32(item->LDoNPrice);
		outapp->WriteUInt32(item->LDoNSellBackRate);
		for (spacer = 0; spacer < 11; spacer++) { //unknowns
			outapp->WriteUInt8(0);
		}
		outapp->WriteUInt32(0xFFFFFFFF); //Unknown but always seen as FF FF FF FF
		outapp->WriteUInt16(0); //Unknown
		outapp->WriteUInt32(item->Favor); // Tribute
		for (spacer = 0; spacer < 17; spacer++) { //unknowns
			outapp->WriteUInt8(0);
		}
		outapp->WriteUInt32(item->GuildFavor); // Tribute
		outapp->WriteUInt32(0); //Unknown
		outapp->WriteUInt32(0xFFFFFFFF); //Unknown but always seen as FF FF FF FF
		for (spacer = 0; spacer < 11; spacer++) { //unknowns
			outapp->WriteUInt8(0);
		}
		outapp->WriteUInt8(1);
		for (spacer = 0; spacer < 25; spacer++) { //unknowns
			outapp->WriteUInt8(0);
		}
		for (spacer = 0; spacer < 304; spacer++) { //Cast stuff and whole bunch of unknowns
			outapp->WriteUInt8(0);
		}
		outapp->WriteUInt8(142); // Always seen not in the item structure though 8E
		outapp->WriteUInt32(0); //unknown
		outapp->WriteUInt32(1); // Always seen as 1
		outapp->WriteUInt32(0); //unknown
		outapp->WriteUInt32(0xCDCCCC3D); // Unknown
		outapp->WriteUInt32(0);
		outapp->WriteUInt16(8256); //0x4020/8256
		outapp->WriteUInt16(0);
		outapp->WriteUInt32(0xFFFFFFFF); //Unknown but always seen as FF FF FF FF
		outapp->WriteUInt16(0);
		outapp->WriteUInt32(0xFFFFFFFF); //Unknown but always seen as FF FF FF FF
		outapp->WriteUInt32(0); //unknown
		outapp->WriteUInt32(0); //unknown
		outapp->WriteUInt16(0); //unknown
		outapp->WriteUInt32(32831); //0x3F80
		for (spacer = 0; spacer < 24; spacer++) { //whole bunch of unknowns always 0's
			outapp->WriteUInt8(0);
		}
		outapp->WriteUInt8(1);
		for (spacer = 0; spacer < 6; spacer++) { //whole bunch of unknowns always 0's
			outapp->WriteUInt8(0);
		}

		QueuePacket(outapp);
		safe_delete(outapp);
	}
	else
		return;
}

void Client::Handle_OP_ItemPreviewRequest(const EQApplicationPacket* app)
{
	VERIFY_PACKET_LENGTH(OP_ItemPreviewRequest, app, ItemPreview_Struct);
	auto ips  = (ItemPreview_Struct*) app->pBuffer;
	const EQ::ItemData* item = database.GetItem(ips->itemid);

	if (item) {
		EQ::ItemInstance* inst = database.CreateItem(item);
		if (inst) {
			std::string packet = inst->Serialize(-1);
			auto        outapp = new EQApplicationPacket(OP_ItemPreviewRequest, packet.length());
			memcpy(outapp->pBuffer, packet.c_str(), packet.length());

#if EQDEBUG >= 9
			DumpPacket(outapp);
#endif

			QueuePacket(outapp);
			safe_delete(outapp);
			safe_delete(inst);
		}
	}
}

void Client::Handle_OP_ItemVerifyRequest(const EQApplicationPacket *app)
{
	using EQ::spells::CastingSlot;
	if (app->size != sizeof(ItemVerifyRequest_Struct))
	{
		LogError("OP size error: OP_ItemVerifyRequest expected:[{}] got:[{}]", sizeof(ItemVerifyRequest_Struct), app->size);
		return;
	}

	ItemVerifyRequest_Struct* request = (ItemVerifyRequest_Struct*)app->pBuffer;
	int32 slot_id;
	int32 target_id;
	int32 spell_id = 0;
	slot_id = request->slot;
	target_id = request->target;

	cheat_manager.ProcessItemVerifyRequest(request->slot, request->target);

	EQApplicationPacket *outapp = nullptr;
	outapp = new EQApplicationPacket(OP_ItemVerifyReply, sizeof(ItemVerifyReply_Struct));
	ItemVerifyReply_Struct* reply = (ItemVerifyReply_Struct*)outapp->pBuffer;
	reply->slot = slot_id;
	reply->target = target_id;

	QueuePacket(outapp);
	safe_delete(outapp);


	if (IsAIControlled()) {
		MessageString(Chat::Red, NOT_IN_CONTROL);
		return;
	}

	if (slot_id < 0) {
		LogDebug("Unknown slot being used by [{}] slot being used is [{}]", GetName(), request->slot);
		return;
	}

	const EQ::ItemInstance* inst = m_inv[slot_id];
	if (!inst) {
		Message(Chat::Red, "Error: item not found in inventory slot #%i", slot_id);
		DeleteItemInInventory(slot_id, 0, true);
		return;
	}

	const EQ::ItemData* item = inst->GetItem();
	if (!item) {
		Message(Chat::Red, "Error: item not found in inventory slot #%i", slot_id);
		DeleteItemInInventory(slot_id, 0, true);
		return;
	}

	spell_id = item->Click.Effect;
	bool is_casting_bard_song = false;

	if (spell_id > 0) {

		if (!IsValidSpell(spell_id) ||
			IsStunned() ||
			IsFeared() ||
			IsMezzed() ||
			DivineAura() ||
			(spells[spell_id].target_type == ST_Ring) ||
			(IsSilenced() && !IsDiscipline(spell_id)) ||
			(IsAmnesiad() && IsDiscipline(spell_id)) ||
			(IsDetrimentalSpell(spell_id) && !zone->CanDoCombat()) ||
			(inst->IsScaling() && inst->GetExp() <= 0)) { // charms don't have spells when less than 0

			SendSpellBarEnable(spell_id);
			return;
		}

		if (casting_spell_id ||
			delaytimer ||
			spellend_timer.Enabled()) {

			/*
				Bards on live can click items while casting spell gems, it stops that song cast and replaces it with item click cast.
				Can not click while casting other items.
			*/
			if (GetClass() == Class::Bard && IsCasting() && casting_spell_slot < CastingSlot::MaxGems)
			{
				is_casting_bard_song = true;
			}
			else
			{
				SendSpellBarEnable(spell_id);
				return;
			}
		}
	}
	// Modern clients don't require pet targeted for item clicks that are ST_Pet
	if (spell_id > 0 && (spells[spell_id].target_type == ST_Pet || spells[spell_id].target_type == ST_SummonedPet))
		target_id = GetPetID();

	LogDebug("OP ItemVerifyRequest: spell=[{}] target=[{}] inv=[{}]", spell_id, target_id, slot_id);

	if (m_inv.SupportsClickCasting(slot_id) || ((item->ItemType == EQ::item::ItemTypePotion || item->PotionBelt) && m_inv.SupportsPotionBeltCasting(slot_id))) // sanity check
	{
		auto* p_inst = (EQ::ItemInstance*) inst;

		if (parse->ItemHasQuestSub(p_inst, EVENT_ITEM_CLICK)) {
			parse->EventItem(EVENT_ITEM_CLICK, this, p_inst, nullptr, "", slot_id);
		}

		if (parse->PlayerHasQuestSub(EVENT_ITEM_CLICK_CLIENT)) {
			std::vector<std::any> args;
			args.emplace_back(p_inst);
			parse->EventPlayer(EVENT_ITEM_CLICK_CLIENT, this, std::to_string(slot_id), 0, &args);
		}

		inst = m_inv[slot_id];
		if (!inst)
		{
			return;
		}

		int r;
		bool tryaug = false;
		EQ::ItemInstance* clickaug = nullptr;
		EQ::ItemData* augitem = nullptr;

		for (r = EQ::invaug::SOCKET_BEGIN; r <= EQ::invaug::SOCKET_END; r++) {
			const EQ::ItemInstance* aug_i = inst->GetAugment(r);
			if (!aug_i)
				continue;
			const EQ::ItemData* aug = aug_i->GetItem();
			if (!aug)
				continue;

			if ((aug->Click.Type == EQ::item::ItemEffectClick) || (aug->Click.Type == EQ::item::ItemEffectExpendable) || (aug->Click.Type == EQ::item::ItemEffectEquipClick) || (aug->Click.Type == EQ::item::ItemEffectClick2))
			{
				tryaug = true;
				clickaug = (EQ::ItemInstance*)aug_i;
				augitem = (EQ::ItemData*)aug;
				spell_id = aug->Click.Effect;
				break;
			}
		}

		if ((spell_id <= 0) && (item->ItemType != EQ::item::ItemTypeFood && item->ItemType != EQ::item::ItemTypeDrink && item->ItemType != EQ::item::ItemTypeAlcohol && item->ItemType != EQ::item::ItemTypeSpell))
		{
			LogDebug("Item with no effect right clicked by [{}]", GetName());
		}
		else if (inst->IsClassCommon())
		{
			if (
				!RuleB(Skills, RequireTomeHandin) &&
				item->ItemType == EQ::item::ItemTypeSpell &&
				(
					Strings::BeginsWith(item->Name, "Tome of ") ||
					Strings::BeginsWith(item->Name, "Skill: ")
				)
			) {
				DeleteItemInInventory(slot_id, 1, true);
				TrainDiscipline(item->ID);
			}
			else if (item->ItemType == EQ::item::ItemTypeSpell)
			{
				spell_id = item->Scroll.Effect;
				if (RuleB(Spells, AllowSpellMemorizeFromItem))
				{
					int highest_spell_id = GetHighestScribedSpellinSpellGroup(spells[spell_id].spell_group);
					if (spells[spell_id].spell_group > 0 && highest_spell_id > 0)
					{
						if (spells[spell_id].rank > spells[highest_spell_id].rank)
						{
							std::string message = fmt::format("{} will replace {} in your spellbook", GetSpellName(spell_id), GetSpellName(highest_spell_id));
							SetEntityVariable("slot_id",itoa(slot_id));
							SetEntityVariable("spell_id",itoa(item->ID));
							SendPopupToClient("", message.c_str(), 1000001, 1, 10);
							return;
						}
						else if (spells[spell_id].rank < spells[highest_spell_id].rank)
						{
							MessageString(Chat::Red, LESSER_SPELL_VERSION, spells[spell_id].name, spells[highest_spell_id].name);
							return;
						}
					}
					DeleteItemInInventory(slot_id, 1, true);
					MemorizeSpellFromItem(item->ID);
					return;
				} else {
					return;
				}
			}
			else if (
				item->Click.Effect > 0 &&
				(
					(item->Click.Type == EQ::item::ItemEffectClick) ||
					(item->Click.Type == EQ::item::ItemEffectExpendable) ||
					(item->Click.Type == EQ::item::ItemEffectEquipClick) ||
					(item->Click.Type == EQ::item::ItemEffectClick2)
				)
			) {
				if (inst->GetCharges() == 0)
				{
					//Message(Chat::White, "This item is out of charges.");
					MessageString(Chat::Red, ITEM_OUT_OF_CHARGES);
					return;
				}
				if (GetLevel() >= item->Click.Level2)
				{
					if (item->RecastDelay > 0)
					{
						if (item->RecastType != RECAST_TYPE_UNLINKED_ITEM && !GetPTimers().Expired(&database, (pTimerItemStart + item->RecastType), false)) {
							SetItemCooldown(item->ID, true);
							SendSpellBarEnable(item->Click.Effect);
							LogSpells("Casting of [{}] canceled: item spell reuse timer not expired", spell_id);
							return;
						} else if (item->RecastType == RECAST_TYPE_UNLINKED_ITEM  && !GetPTimers().Expired(&database, (pTimerNegativeItemReuse * item->ID), false)) {
							SetItemCooldown(item->ID, true);
							SendSpellBarEnable(item->Click.Effect);
							LogSpells("Casting of [{}] canceled: item spell reuse timer not expired", spell_id);
							return;
						}
					}

					int i = 0;

					if (parse->ItemHasQuestSub(p_inst, EVENT_ITEM_CLICK_CAST)) {
						i = parse->EventItem(EVENT_ITEM_CLICK_CAST, this, p_inst, nullptr, "", slot_id);
					}

					if (parse->PlayerHasQuestSub(EVENT_ITEM_CLICK_CAST_CLIENT)) {
						std::vector<std::any> args;
						args.emplace_back(p_inst);
						i = parse->EventPlayer(EVENT_ITEM_CLICK_CAST_CLIENT, this, std::to_string(slot_id), 0, &args);
					}

					inst = m_inv[slot_id];
					if (!inst)
					{
						return;
					}
					if (i == 0) {
						if (!IsCastWhileInvisibleSpell(item->Click.Effect)) {
							CommonBreakInvisible(); // client can't do this for us :(
						}
						if (GetClass() == Class::Bard){
							DoBardCastingFromItemClick(is_casting_bard_song, item->CastTime, item->Click.Effect, target_id, CastingSlot::Item, slot_id, item->RecastType, item->RecastDelay);
						}
						else {
							CastSpell(item->Click.Effect, target_id, CastingSlot::Item, item->CastTime, 0, 0, slot_id);
						}
					} else {
						InterruptSpell(item->Click.Effect);
						SendSpellBarEnable(item->Click.Effect);
						return;
					}
				}
				else
				{
					MessageString(Chat::Red, ITEMS_INSUFFICIENT_LEVEL);
					return;
				}
			}
			else if (tryaug)
			{
				if (clickaug->GetCharges() == 0)
				{
					//Message(Chat::White, "This item is out of charges.");
					MessageString(Chat::Red, ITEM_OUT_OF_CHARGES);
					return;
				}
				if (GetLevel() >= augitem->Click.Level2)
				{
					if (augitem->RecastDelay > 0)
					{
						if (augitem->RecastType != RECAST_TYPE_UNLINKED_ITEM && !GetPTimers().Expired(&database, (pTimerItemStart + augitem->RecastType), false)) {
							LogSpells("Casting of [{}] canceled: item spell reuse timer from augment not expired", spell_id);
							MessageString(Chat::Red, SPELL_RECAST);
							SendSpellBarEnable(augitem->Click.Effect);
							return;
						} else if (augitem->RecastType == RECAST_TYPE_UNLINKED_ITEM  && !GetPTimers().Expired(&database, (pTimerNegativeItemReuse * augitem->ID), false)) {
							MessageString(Chat::Red, SPELL_RECAST);
							SendSpellBarEnable(augitem->Click.Effect);
							LogSpells("Casting of [{}] canceled: item spell reuse timer not expired", spell_id);
							return;
						}
					}

					int i = 0;

					if (parse->ItemHasQuestSub(p_inst, EVENT_ITEM_CLICK_CAST)) {
						i = parse->EventItem(EVENT_ITEM_CLICK_CAST, this, clickaug, nullptr, "", slot_id);
					}

					if (parse->PlayerHasQuestSub(EVENT_ITEM_CLICK_CAST_CLIENT)) {
						std::vector<std::any> args;
						args.emplace_back(clickaug);
						i = parse->EventPlayer(EVENT_ITEM_CLICK_CAST_CLIENT, this, std::to_string(slot_id), 0, &args);
					}

					inst = m_inv[slot_id];
					if (!inst)
					{
						return;
					}

					if (i == 0) {
						if (!IsCastWhileInvisibleSpell(augitem->Click.Effect)) {
							CommonBreakInvisible(); // client can't do this for us :(
						}
						if (GetClass() == Class::Bard) {
							DoBardCastingFromItemClick(is_casting_bard_song, augitem->CastTime, augitem->Click.Effect, target_id, CastingSlot::Item, slot_id, augitem->RecastType, augitem->RecastDelay);
						}
						else {
							CastSpell(augitem->Click.Effect, target_id, CastingSlot::Item, augitem->CastTime, 0, 0, slot_id);
						}
					} else {
						InterruptSpell(item->Click.Effect);
						SendSpellBarEnable(item->Click.Effect);
						return;
					}
				}
				else
				{
					MessageString(Chat::Red, ITEMS_INSUFFICIENT_LEVEL);
					return;
				}
			}
			else
			{
				if (ClientVersion() >= EQ::versions::ClientVersion::SoD && !inst->IsEquipable(GetBaseRace(), GetClass()))
				{
					if (item->ItemType != EQ::item::ItemTypeFood && item->ItemType != EQ::item::ItemTypeDrink && item->ItemType != EQ::item::ItemTypeAlcohol)
					{
						LogDebug("Error: unknown item->Click.Type ([{}])", item->Click.Type);
					}
				}
				else
				{
					LogDebug("Error: unknown item->Click.Type ([{}])", item->Click.Type);
				}
			}
		}
		else
		{
			Message(Chat::Red, "Error: item not found in inventory slot #%i", slot_id);
		}
	}
	else
	{
		Message(Chat::Red, "Error: Invalid inventory slot for using effects (inventory slot #%i)", slot_id);
	}

	return;
}

void Client::Handle_OP_Jump(const EQApplicationPacket *app)
{
	SetEndurance(GetEndurance() - (GetLevel()<20 ? (225 * GetLevel() / 100) : 50));
	return;
}

void Client::Handle_OP_KeyRing(const EQApplicationPacket *app)
{
	KeyRingList(this);
}

void Client::Handle_OP_KickPlayers(const EQApplicationPacket *app)
{
	auto buf = reinterpret_cast<KickPlayers_Struct*>(app->pBuffer);
	if (buf->kick_expedition)
	{
		auto expedition = GetExpedition();
		if (expedition)
		{
			expedition->DzKickPlayers(this);
		}
	}
	else if (buf->kick_task && GetTaskState() && GetTaskState()->HasActiveSharedTask())
	{
		GetTaskState()->KickPlayersSharedTask(this);
	}
}

void Client::Handle_OP_LDoNButton(const EQApplicationPacket *app)
{
	if (app->size < sizeof(bool))
	{
		return;
	}

	if (GetPendingAdventureCreate())
	{
		return;
	}

	if (IsOnAdventure())
	{
		return;
	}

	bool* p = (bool*)app->pBuffer;
	if (*p == true)
	{
		auto pack =
			new ServerPacket(ServerOP_AdventureRequestCreate,
				sizeof(ServerAdventureRequestCreate_Struct) + (64 * adv_requested_member_count));
		ServerAdventureRequestCreate_Struct *sac = (ServerAdventureRequestCreate_Struct*)pack->pBuffer;
		strcpy(sac->leader, GetName());
		sac->id = adv_requested_id;
		sac->theme = adv_requested_theme;
		sac->member_count = adv_requested_member_count;
		memcpy((pack->pBuffer + sizeof(ServerAdventureRequestCreate_Struct)), adv_requested_data, (64 * adv_requested_member_count));
		worldserver.SendPacket(pack);
		delete pack;
		PendingAdventureCreate();
		ClearPendingAdventureData();
	}
	else
	{
		ClearPendingAdventureData();
	}
}

void Client::Handle_OP_LDoNDisarmTraps(const EQApplicationPacket *app)
{
	Mob * target = GetTarget();
	if (target && target->IsNPC() && !target->IsAura())
	{
		if (HasSkill(EQ::skills::SkillDisarmTraps))
		{
			if (DistanceSquaredNoZ(m_Position, target->GetPosition()) > RuleI(Adventure, LDoNTrapDistanceUse))
			{
				Message(Chat::Red, "%s is too far away.", target->GetCleanName());
				return;
			}
			HandleLDoNDisarm(target->CastToNPC(), GetSkill(EQ::skills::SkillDisarmTraps), LDoNTypeMechanical);
		}
		else
			Message(Chat::Red, "You do not have the disarm trap skill.");
	}
}

void Client::Handle_OP_LDoNInspect(const EQApplicationPacket *app)
{
	auto* t = GetTarget();
	if (t && t->GetClass() == Class::LDoNTreasure && !t->IsAura()) {
		if (parse->PlayerHasQuestSub(EVENT_INSPECT)) {
			std::vector<std::any> args = { t };
			if (parse->EventPlayer(EVENT_INSPECT, this, "", t->GetID(), &args) == 0) {
				Message(Chat::Yellow, fmt::format("{}", t->GetCleanName()).c_str());
			}
		}
	}
}

void Client::Handle_OP_LDoNOpen(const EQApplicationPacket *app)
{
	Mob * target = GetTarget();
	if (target && target->IsNPC() && !target->IsAura()) {
		HandleLDoNOpen(target->CastToNPC());
	}
}

void Client::Handle_OP_LDoNPickLock(const EQApplicationPacket *app)
{
	Mob * target = GetTarget();
	if (target && target->IsNPC() && !target->IsAura())
	{
		if (HasSkill(EQ::skills::SkillPickLock))
		{
			if (DistanceSquaredNoZ(m_Position, target->GetPosition()) > RuleI(Adventure, LDoNTrapDistanceUse))
			{
				Message(Chat::Red, "%s is too far away.", target->GetCleanName());
				return;
			}
			HandleLDoNPickLock(target->CastToNPC(), GetSkill(EQ::skills::SkillPickLock), LDoNTypeMechanical);
		}
		else
			Message(Chat::Red, "You do not have the pick locks skill.");
	}
}

void Client::Handle_OP_LDoNSenseTraps(const EQApplicationPacket *app)
{
	Mob * target = GetTarget();
	if (target && target->IsNPC() && !target->IsAura())
	{
		if (HasSkill(EQ::skills::SkillSenseTraps))
		{
			if (DistanceSquaredNoZ(m_Position, target->GetPosition()) > RuleI(Adventure, LDoNTrapDistanceUse))
			{
				Message(Chat::Red, "%s is too far away.", target->GetCleanName());
				return;
			}
			HandleLDoNSenseTraps(target->CastToNPC(), GetSkill(EQ::skills::SkillSenseTraps), LDoNTypeMechanical);
		}
		else
			Message(Chat::Red, "You do not have the sense traps skill.");
	}
}

void Client::Handle_OP_LeadershipExpToggle(const EQApplicationPacket *app)
{
	if (app->size != 1) {
		LogDebug("Size mismatch in OP_LeadershipExpToggle expected [{}] got [{}]", 1, app->size);
		DumpPacket(app);
		return;
	}
	uint8 *mode = (uint8 *)app->pBuffer;
	if (*mode) {
		m_pp.leadAAActive = 1;
		Save();
		MessageString(Chat::Yellow, LEADERSHIP_EXP_ON);
	}
	else {
		m_pp.leadAAActive = 0;
		Save();
		MessageString(Chat::Yellow, LEADERSHIP_EXP_OFF);
	}
}

void Client::Handle_OP_LeaveAdventure(const EQApplicationPacket *app)
{
	if (!IsOnAdventure())
	{
		return;
	}
	LeaveAdventure();
}

void Client::Handle_OP_LeaveBoat(const EQApplicationPacket *app)
{
	Mob* boat = entity_list.GetMob(controlling_boat_id);	// find the mob corresponding to the boat id
	if (boat) {
		if ((boat->GetTarget() == this) && boat->GetHateAmount(this) == 0) { // if the client somehow left while still controlling the boat (and the boat isn't attacking them)
			boat->SetTarget(nullptr); // fix it to stop later problems
		}
	}

	controlling_boat_id = 0;

	return;
}

void Client::Handle_OP_LFGCommand(const EQApplicationPacket *app)
{
	if (app->size != sizeof(LFG_Struct)) {
		std::cout << "Wrong size on OP_LFGCommand. Got: " << app->size << ", Expected: " << sizeof(LFG_Struct) << std::endl;
		DumpPacket(app);
		return;
	}

	// Process incoming packet
	LFG_Struct* lfg = (LFG_Struct*)app->pBuffer;

	switch (lfg->value & 0xFF) {
	case 0:
		if (LFG) {
			database.SetLFG(CharacterID(), false);
			LFG = false;
			LFGComments[0] = '\0';
		}
		break;
	case 1:
		if (!LFG) {
			LFG = true;
			database.SetLFG(CharacterID(), true);
		}
		LFGFromLevel = lfg->FromLevel;
		LFGToLevel = lfg->ToLevel;
		LFGMatchFilter = lfg->MatchFilter;
		strn0cpy(LFGComments, lfg->Comments, sizeof(LFGComments));
		break;
	default:
		Message(Chat::Red, "Error: unknown LFG value %i", lfg->value);
	}

	UpdateWho();

	// Issue outgoing packet to notify other clients
	auto outapp = new EQApplicationPacket(OP_LFGAppearance, sizeof(LFG_Appearance_Struct));
	LFG_Appearance_Struct* lfga = (LFG_Appearance_Struct*)outapp->pBuffer;
	lfga->spawn_id = GetID();
	lfga->lfg = (uint8)LFG;

	entity_list.QueueClients(this, outapp, true);
	safe_delete(outapp);
	return;
}

void Client::Handle_OP_LFGGetMatchesRequest(const EQApplicationPacket *app)
{

	if (app->size != sizeof(LFGGetMatchesRequest_Struct)) {
		LogError("Wrong size: OP_LFGGetMatchesRequest, size=[{}], expected [{}]", app->size, sizeof(LFGGetMatchesRequest_Struct));
		DumpPacket(app);
		return;
	}
	LFGGetMatchesRequest_Struct* gmrs = (LFGGetMatchesRequest_Struct*)app->pBuffer;

	if (!worldserver.Connected())
		Message(Chat::Red, "Error: World server disconnected");
	else {
		auto pack = new ServerPacket(ServerOP_LFGMatches, sizeof(ServerLFGMatchesRequest_Struct));
		ServerLFGMatchesRequest_Struct* smrs = (ServerLFGMatchesRequest_Struct*)pack->pBuffer;
		smrs->FromID = GetID();
		smrs->QuerierLevel = GetLevel();
		strcpy(smrs->FromName, GetName());
		smrs->FromLevel = gmrs->FromLevel;
		smrs->ToLevel = gmrs->ToLevel;
		smrs->Classes = gmrs->Classes;
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void Client::Handle_OP_LFGuild(const EQApplicationPacket *app)
{
	if (app->size < 4)
		return;

	uint32 Command = *((uint32 *)app->pBuffer);

	switch (Command)
	{
	case 0:
	{
		VERIFY_PACKET_LENGTH(OP_LFGuild, app, LFGuild_PlayerToggle_Struct);
		LFGuild_PlayerToggle_Struct *pts = (LFGuild_PlayerToggle_Struct *)app->pBuffer;

#ifdef DARWIN
#if __DARWIN_C_LEVEL < 200809L
		if (strlen(pts->Comment) > 256)
#else
		if (strnlen(pts->Comment, 256) > 256)
#endif // __DARWIN_C_LEVEL
#else
		if (strnlen(pts->Comment, 256) > 256)
#endif // DARWIN
			return;

		auto pack = new ServerPacket(ServerOP_QueryServGeneric, strlen(GetName()) + strlen(pts->Comment) + 38);

		pack->WriteUInt32(zone->GetZoneID());
		pack->WriteUInt32(zone->GetInstanceID());
		pack->WriteString(GetName());
		pack->WriteUInt32(QSG_LFGuild);
		pack->WriteUInt32(QSG_LFGuild_UpdatePlayerInfo);
		pack->WriteUInt32(GetBaseClass());
		pack->WriteUInt32(GetLevel());
		pack->WriteUInt32(GetSpentAA());
		pack->WriteString(pts->Comment);
		pack->WriteUInt32(pts->Toggle);
		pack->WriteUInt32(pts->TimeZone);

		worldserver.SendPacket(pack);
		safe_delete(pack);

		break;
	}
	case 1:
	{
		VERIFY_PACKET_LENGTH(OP_LFGuild, app, LFGuild_GuildToggle_Struct);
		LFGuild_GuildToggle_Struct *gts = (LFGuild_GuildToggle_Struct *)app->pBuffer;

#ifdef DARWIN
#if __DARWIN_C_LEVEL < 200809L
		if (strlen(gts->Comment) > 256)
#else
		if (strnlen(gts->Comment, 256) > 256)
#endif // __DARWIN_C_LEVEL
#else
		if (strnlen(gts->Comment, 256) > 256)
#endif // __DARWIN
			return;

		auto pack =
			new ServerPacket(ServerOP_QueryServGeneric, strlen(GetName()) + strlen(gts->Comment) +
				strlen(guild_mgr.GetGuildName(GuildID())) + 43);

		pack->WriteUInt32(zone->GetZoneID());
		pack->WriteUInt32(zone->GetInstanceID());
		pack->WriteString(GetName());
		pack->WriteUInt32(QSG_LFGuild);
		pack->WriteUInt32(QSG_LFGuild_UpdateGuildInfo);
		pack->WriteString(guild_mgr.GetGuildName(GuildID()));
		pack->WriteString(gts->Comment);
		pack->WriteUInt32(gts->FromLevel);
		pack->WriteUInt32(gts->ToLevel);
		pack->WriteUInt32(gts->Classes);
		pack->WriteUInt32(gts->AACount);
		pack->WriteUInt32(gts->Toggle);
		pack->WriteUInt32(gts->TimeZone);

		worldserver.SendPacket(pack);
		safe_delete(pack);

		break;
	}
	case 3:
	{
		VERIFY_PACKET_LENGTH(OP_LFGuild, app, LFGuild_SearchPlayer_Struct);

		auto pack = new ServerPacket(ServerOP_QueryServGeneric, strlen(GetName()) + 37);

		pack->WriteUInt32(zone->GetZoneID());
		pack->WriteUInt32(zone->GetInstanceID());
		pack->WriteString(GetName());
		pack->WriteUInt32(QSG_LFGuild);
		pack->WriteUInt32(QSG_LFGuild_PlayerMatches);

		LFGuild_SearchPlayer_Struct *sps = (LFGuild_SearchPlayer_Struct *)app->pBuffer;
		pack->WriteUInt32(sps->FromLevel);
		pack->WriteUInt32(sps->ToLevel);
		pack->WriteUInt32(sps->MinAA);
		pack->WriteUInt32(sps->TimeZone);
		pack->WriteUInt32(sps->Classes);

		worldserver.SendPacket(pack);
		safe_delete(pack);

		break;
	}
	case 4:
	{
		VERIFY_PACKET_LENGTH(OP_LFGuild, app, LFGuild_SearchGuild_Struct);

		auto pack = new ServerPacket(ServerOP_QueryServGeneric, strlen(GetName()) + 33);

		pack->WriteUInt32(zone->GetZoneID());
		pack->WriteUInt32(zone->GetInstanceID());
		pack->WriteString(GetName());
		pack->WriteUInt32(QSG_LFGuild);
		pack->WriteUInt32(QSG_LFGuild_GuildMatches);

		LFGuild_SearchGuild_Struct *sgs = (LFGuild_SearchGuild_Struct *)app->pBuffer;

		pack->WriteUInt32(sgs->Level);
		pack->WriteUInt32(sgs->AAPoints);
		pack->WriteUInt32(sgs->TimeZone);
		pack->WriteUInt32(sgs->Class);

		worldserver.SendPacket(pack);
		safe_delete(pack);

		break;
	}
	default:
		break;
	}
}

void Client::Handle_OP_LFPCommand(const EQApplicationPacket *app)
{

	if (app->size != sizeof(LFP_Struct)) {
		LogError("Wrong size: OP_LFPCommand, size=[{}], expected [{}]", app->size, sizeof(LFP_Struct));
		DumpPacket(app);
		return;
	}
	LFP_Struct *lfp = (LFP_Struct*)app->pBuffer;

	LFP = lfp->Action != LFPOff;
	database.SetLFP(CharacterID(), LFP);

	if (!LFP) {
		worldserver.StopLFP(CharacterID());
		return;
	}

	GroupLFPMemberEntry LFPMembers[MAX_GROUP_MEMBERS];

	for (unsigned int i = 0; i<MAX_GROUP_MEMBERS; i++) {
		LFPMembers[i].Name[0] = '\0';
		LFPMembers[i].Class = Class::None;
		LFPMembers[i].Level = 0;
		LFPMembers[i].Zone = 0;
		LFPMembers[i].GuildID = 0xFFFF;
	}

	Group *g = GetGroup();

	// Slot 0 is always for the group leader, or the player if not in a group
	strcpy(LFPMembers[0].Name, GetName());
	LFPMembers[0].Class = GetClass();
	LFPMembers[0].Level = GetLevel();
	LFPMembers[0].Zone = zone->GetZoneID();
	LFPMembers[0].GuildID = GuildID();

	if (g) {
		// This should not happen. The client checks if you are in a group and will not let you put LFP on if
		// you are not the leader.
		if (!g->IsLeader(this)) {
			LogError("Client sent LFP on for character [{}] who is grouped but not leader", GetName());
			return;
		}
		// Fill the LFPMembers array with the rest of the group members, excluding ourself
		// We don't fill in the class, level or zone, because we may not be able to determine
		// them if the other group members are not in this zone. World will fill in this information
		// for us, if it can.
		int NextFreeSlot = 1;
		for (unsigned int i = 0; i < MAX_GROUP_MEMBERS; i++) {
			if (strcasecmp(g->membername[i], LFPMembers[0].Name))
				strcpy(LFPMembers[NextFreeSlot++].Name, g->membername[i]);
		}
	}


	worldserver.UpdateLFP(CharacterID(), lfp->Action, lfp->MatchFilter, lfp->FromLevel, lfp->ToLevel, lfp->Classes,
		lfp->Comments, LFPMembers);


}

void Client::Handle_OP_LFPGetMatchesRequest(const EQApplicationPacket *app)
{

	if (app->size != sizeof(LFPGetMatchesRequest_Struct)) {
		LogError("Wrong size: OP_LFPGetMatchesRequest, size=[{}], expected [{}]", app->size, sizeof(LFPGetMatchesRequest_Struct));
		DumpPacket(app);
		return;
	}
	LFPGetMatchesRequest_Struct* gmrs = (LFPGetMatchesRequest_Struct*)app->pBuffer;

	if (!worldserver.Connected())
		Message(Chat::Red, "Error: World server disconnected");
	else {
		auto pack = new ServerPacket(ServerOP_LFPMatches, sizeof(ServerLFPMatchesRequest_Struct));
		ServerLFPMatchesRequest_Struct* smrs = (ServerLFPMatchesRequest_Struct*)pack->pBuffer;
		smrs->FromID = GetID();
		smrs->FromLevel = gmrs->FromLevel;
		smrs->ToLevel = gmrs->ToLevel;
		smrs->QuerierLevel = GetLevel();
		smrs->QuerierClass = GetClass();
		strcpy(smrs->FromName, GetName());
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}

	return;
}

void Client::Handle_OP_LoadSpellSet(const EQApplicationPacket *app)
{
	if (app->size != sizeof(LoadSpellSet_Struct)) {
		printf("Wrong size of LoadSpellSet_Struct! Expected: %zu, Got: %i\n", sizeof(LoadSpellSet_Struct), app->size);
		return;
	}
	int i;
	LoadSpellSet_Struct* ss = (LoadSpellSet_Struct*)app->pBuffer;
	for (i = 0; i < EQ::spells::SPELL_GEM_COUNT; i++) {
		if (ss->spell[i] != 0xFFFFFFFF)
			UnmemSpell(i, true);
	}
}

void Client::Handle_OP_Logout(const EQApplicationPacket *app)
{
	LogDebug("[{}] sent a logout packet", GetName());

	SendLogoutPackets();

	auto outapp = new EQApplicationPacket(OP_LogoutReply);
	FastQueuePacket(&outapp);

	Disconnect();
	return;
}

void Client::Handle_OP_LootItem(const EQApplicationPacket *app)
{
	if (app->size != sizeof(LootingItem_Struct)) {
		LogError("Wrong size: OP_LootItem, size=[{}], expected [{}]", app->size, sizeof(LootingItem_Struct));
		return;
	}

	auto* l = (LootingItem_Struct*) app->pBuffer;
	auto entity = entity_list.GetID(static_cast<uint16>(l->lootee));
	if (!entity) {
		auto outapp = new EQApplicationPacket(OP_LootComplete, 0);
		QueuePacket(outapp);
		safe_delete(outapp);
		return;
	}

	if (!entity->IsCorpse()) {
		Corpse::SendEndLootErrorPacket(this);
		return;
	}

	entity->CastToCorpse()->LootCorpseItem(this, app);
}

void Client::Handle_OP_LootRequest(const EQApplicationPacket *app)
{
	if (app->size != sizeof(uint32)) {
		std::cout << "Wrong size: OP_LootRequest, size=" << app->size << ", expected " << sizeof(uint32) << std::endl;
		return;
	}

	Entity* ent = entity_list.GetID(*((uint32*)app->pBuffer));
	if (ent == 0) {
		Message(Chat::Red, "Error: OP_LootRequest: Corpse not found (ent = 0)");
		Corpse::SendLootReqErrorPacket(this);
		return;
	}
	if (ent->IsCorpse())
	{
		SetLooting(ent->GetID()); //store the entity we are looting

		ent->CastToCorpse()->MakeLootRequestPackets(this, app);
		return;
	}
	else {
		std::cout << "npc == 0 LOOTING FOOKED3" << std::endl;
		Message(Chat::Red, "Error: OP_LootRequest: Corpse not a corpse?");
		Corpse::SendLootReqErrorPacket(this);
	}
	return;
}

void Client::Handle_OP_ManaChange(const EQApplicationPacket *app)
{
	if (app->size == 0) {
		// i think thats the sign to stop the songs
		if (IsBardSong(casting_spell_id) || HasActiveSong()) {
			InterruptSpell(SONG_ENDS, 0x121); //Live doesn't send song end message anymore (~Kayen 1/26/22)
		}
		else {
			InterruptSpell(INTERRUPT_SPELL, 0x121);
		}
		return;
	}
	/*
		I don't think the client sends proper manachanges
		with a length, just the 0 len ones for stopping songs
		ManaChange_Struct* p = (ManaChange_Struct*)app->pBuffer;
	*/
	else{
		printf("OP_ManaChange from client:\n");
		DumpPacket(app);
	}
	return;
}

/*
#if 0	// I dont think there's an op for this now, and we check this
// when the client is sitting
void Client::Handle_OP_Medding(const EQApplicationPacket *app)
{
if (app->pBuffer[0])
medding = true;
else
medding = false;
return;
}
#endif
*/

void Client::Handle_OP_MemorizeSpell(const EQApplicationPacket *app)
{
	cheat_manager.CheckMemTimer();
	OPMemorizeSpell(app);
	return;
}

void Client::Handle_OP_Mend(const EQApplicationPacket *app)
{
	if (!HasSkill(EQ::skills::SkillMend))
		return;

	if (!p_timers.Expired(&database, pTimerMend, false)) {
		Message(Chat::Red, "Ability recovery time not yet met.");
		return;
	}
	p_timers.Start(pTimerMend, (MendReuseTime - GetSkillReuseTime(EQ::skills::SkillMend)));

	int mendhp = GetMaxHP() / 4;
	int currenthp = GetHP();
	if (zone->random.Int(0, RuleI(Character, MendAlwaysSucceedValue)) < (int)GetSkill(EQ::skills::SkillMend)) {

		int criticalchance = spellbonuses.CriticalMend + itembonuses.CriticalMend + aabonuses.CriticalMend;

		if (zone->random.Int(0, 99) < criticalchance) {
			mendhp *= 2;
			MessageString(Chat::LightBlue, MEND_CRITICAL);
		}
		else {
			MessageString(Chat::LightBlue, MEND_SUCCESS);
		}
		SetHP(GetHP() + mendhp);
		SendHPUpdate();
	}
	else {
		/* the purpose of the following is to make the chance to worsen wounds much less common,
		which is more consistent with the way eq live works.
		according to my math, this should result in the following probability:
		0 skill - 25% chance to worsen
		20 skill - 23% chance to worsen
		50 skill - 16% chance to worsen */
		if ((GetSkill(EQ::skills::SkillMend) <= 75) && (zone->random.Int(GetSkill(EQ::skills::SkillMend), 100) < 75) && (zone->random.Int(1, 3) == 1))
		{
			SetHP(currenthp > mendhp ? (GetHP() - mendhp) : 1);
			SendHPUpdate();
			MessageString(Chat::LightBlue, MEND_WORSEN);
		}
		else
			MessageString(Chat::LightBlue, MEND_FAIL);
	}

	CheckIncreaseSkill(EQ::skills::SkillMend, nullptr, 10);
	return;
}

void Client::Handle_OP_MercenaryCommand(const EQApplicationPacket *app)
{
	if (app->size != sizeof(MercenaryCommand_Struct))
	{
		Message(Chat::Red, "Size mismatch in OP_MercenaryCommand expected %i got %i", sizeof(MercenaryCommand_Struct), app->size);
		LogDebug("Size mismatch in OP_MercenaryCommand expected [{}] got [{}]", sizeof(MercenaryCommand_Struct), app->size);
		DumpPacket(app);
		return;
	}

	MercenaryCommand_Struct* mc = (MercenaryCommand_Struct*)app->pBuffer;
	uint32 merc_command = mc->MercCommand;	// Seen 0 (zone in with no merc or suspended), 1 (dismiss merc), 5 (normal state), 20 (unknown), 36 (zone in with merc)
	int32 option = mc->Option;	// Seen -1 (zone in with no merc), 0 (setting to passive stance), 1 (normal or setting to balanced stance)

	Log(Logs::General, Logs::Mercenaries, "Command %i, Option %i received from %s.", merc_command, option, GetName());

	if (!RuleB(Mercs, AllowMercs))
		return;

	// Handle the Command here...
	// Will need a list of what every type of command is supposed to do
	// Unsure if there is a server response to this packet
	if (option >= 0)
	{
		Merc* merc = GetMerc();
		GetMercInfo().State = option;

		if (merc)
		{
			uint8 numStances = 0;

			//get number of available stances for the current merc
			std::list<MercStanceInfo> mercStanceList = zone->merc_stance_list[merc->GetMercenaryTemplateID()];
			auto iter = mercStanceList.begin();
			while (iter != mercStanceList.end()) {
				numStances++;
				++iter;
			}

			MercTemplate* mercTemplate = zone->GetMercTemplate(GetMerc()->GetMercenaryTemplateID());
			if (mercTemplate)
			{
				//check to see if selected option is a valid stance slot (option is the slot the stance is in, not the actual stance)
				if (option >= 0 && option < numStances)
				{
					merc->SetStance(mercTemplate->Stances[option]);
					GetMercInfo().Stance = mercTemplate->Stances[option];

					Log(Logs::General, Logs::Mercenaries, "Set Stance: %u for %s (%s)", merc->GetStance(), merc->GetName(), GetName());
				}
			}
		}
	}
}

void Client::Handle_OP_MercenaryDataRequest(const EQApplicationPacket *app)
{
	// The payload is 4 bytes. The EntityID of the Mercenary Liason which are of class 71.
	if (app->size != sizeof(MercenaryMerchantShopRequest_Struct))
	{
		LogDebug("Size mismatch in OP_MercenaryDataRequest expected 4 got [{}]", app->size);

		DumpPacket(app);

		return;
	}

	MercenaryMerchantShopRequest_Struct* mmsr = (MercenaryMerchantShopRequest_Struct*)app->pBuffer;
	uint32 merchant_id = mmsr->MercMerchantID;
	uint32 altCurrentType = 19;

	Log(Logs::General, Logs::Mercenaries, "Data Request for Merchant ID (%i) for %s.", merchant_id, GetName());

	//client is requesting data about currently owned mercenary
	if (merchant_id == 0) {

		//send info about your current merc(s)
		if (GetMercInfo().mercid)
		{
			Log(Logs::General, Logs::Mercenaries, "SendMercPersonalInfo Request for %s.", GetName());
			SendMercPersonalInfo();
		}
		else
		{
			Log(Logs::General, Logs::Mercenaries, "SendMercPersonalInfo Not Sent - MercID (%i) for %s.", GetMercInfo().mercid, GetName());
		}
	}

	if (!RuleB(Mercs, AllowMercs)) {
		return;
	}

	NPC* tar = entity_list.GetNPCByID(merchant_id);

	if (tar) {
		int mercTypeCount = 0;
		int mercCount = 0;

		if (DistanceSquared(m_Position, tar->GetPosition()) > USE_NPC_RANGE2)
			return;

		if (tar->GetClass() != Class::MercenaryLiaison) {
			return;
		}

		mercTypeCount = tar->GetNumMercenaryTypes(static_cast<unsigned int>(ClientVersion()));
		mercCount = tar->GetNumberOfMercenaries(static_cast<unsigned int>(ClientVersion()));

		if (mercCount > MAX_MERC)
			return;

		std::list<MercType> mercTypeList = tar->GetMercenaryTypesList(static_cast<unsigned int>(ClientVersion()));
		std::list<MercData> mercDataList = tar->GetMercenariesList(static_cast<unsigned int>(ClientVersion()));

		int i = 0;
		int StanceCount = 0;

		for (auto mercListItr = mercDataList.begin(); mercListItr != mercDataList.end(); ++mercListItr) {
			auto siter = zone->merc_stance_list[mercListItr->MercTemplateID].begin();
			for (siter = zone->merc_stance_list[mercListItr->MercTemplateID].begin(); siter != zone->merc_stance_list[mercListItr->MercTemplateID].end(); ++siter)
			{
				StanceCount++;
			}
		}

		auto outapp = new EQApplicationPacket(OP_MercenaryDataResponse, sizeof(MercenaryMerchantList_Struct));
		MercenaryMerchantList_Struct* mml = (MercenaryMerchantList_Struct*)outapp->pBuffer;

		mml->MercTypeCount = mercTypeCount;
		if (mercTypeCount > 0)
		{
			for (auto mercTypeListItr = mercTypeList.begin(); mercTypeListItr != mercTypeList.end();
			++mercTypeListItr) {
				mml->MercGrades[i] = mercTypeListItr->Type;	// DBStringID for Type
				i++;
			}
		}
		mml->MercCount = mercCount;

		if (mercCount > 0)
		{
			i = 0;
			for (auto mercListIter = mercDataList.begin(); mercListIter != mercDataList.end();
			++mercListIter) {
				mml->Mercs[i].MercID = mercListIter->MercTemplateID;
				mml->Mercs[i].MercType = mercListIter->MercType;
				mml->Mercs[i].MercSubType = mercListIter->MercSubType;
				mml->Mercs[i].PurchaseCost = RuleB(Mercs, ChargeMercPurchaseCost) ? Merc::CalcPurchaseCost(mercListIter->MercTemplateID, GetLevel(), 0) : 0;
				mml->Mercs[i].UpkeepCost = RuleB(Mercs, ChargeMercUpkeepCost) ? Merc::CalcUpkeepCost(mercListIter->MercTemplateID, GetLevel(), 0) : 0;
				mml->Mercs[i].Status = 0;
				mml->Mercs[i].AltCurrencyCost = RuleB(Mercs, ChargeMercPurchaseCost) ? Merc::CalcPurchaseCost(mercListIter->MercTemplateID, GetLevel(), altCurrentType) : 0;
				mml->Mercs[i].AltCurrencyUpkeep = RuleB(Mercs, ChargeMercUpkeepCost) ? Merc::CalcUpkeepCost(mercListIter->MercTemplateID, GetLevel(), altCurrentType) : 0;
				mml->Mercs[i].AltCurrencyType = altCurrentType;
				mml->Mercs[i].MercUnk01 = 0;
				mml->Mercs[i].TimeLeft = -1;
				mml->Mercs[i].MerchantSlot = i + 1;
				mml->Mercs[i].MercUnk02 = 1;
				int mercStanceCount = 0;
				auto iter = zone->merc_stance_list[mercListIter->MercTemplateID].begin();
				for (iter = zone->merc_stance_list[mercListIter->MercTemplateID].begin(); iter != zone->merc_stance_list[mercListIter->MercTemplateID].end(); ++iter)
				{
					mercStanceCount++;
				}
				mml->Mercs[i].StanceCount = mercStanceCount;
				mml->Mercs[i].MercUnk03 = 519044964;
				mml->Mercs[i].MercUnk04 = 1;
				//mml->Mercs[i].MercName;
				int stanceindex = 0;
				if (mercStanceCount > 0)
				{
					auto iter2 = zone->merc_stance_list[mercListIter->MercTemplateID].begin();
					while (iter2 != zone->merc_stance_list[mercListIter->MercTemplateID].end())
					{
						mml->Mercs[i].Stances[stanceindex].StanceIndex = stanceindex;
						mml->Mercs[i].Stances[stanceindex].Stance = (iter2->StanceID);
						stanceindex++;
						++iter2;
					}
				}
				i++;
			}
		}
		FastQueuePacket(&outapp);
	}
}

void Client::Handle_OP_MercenaryDataUpdateRequest(const EQApplicationPacket *app)
{
	// The payload is 0 bytes.
	if (app->size != 0)
	{
		Message(Chat::Red, "Size mismatch in OP_MercenaryDataUpdateRequest expected 0 got %i", app->size);
		LogDebug("Size mismatch in OP_MercenaryDataUpdateRequest expected 0 got [{}]", app->size);
		DumpPacket(app);
		return;
	}

	Log(Logs::General, Logs::Mercenaries, "Data Update Request Received for %s.", GetName());

	if (GetMercenaryID())
	{
		SendMercPersonalInfo();
	}
}

void Client::Handle_OP_MercenaryDismiss(const EQApplicationPacket *app)
{
	// The payload is 0 or 1 bytes.
	if (app->size > 1)
	{
		Message(Chat::Red, "Size mismatch in OP_MercenaryDismiss expected 0 got %i", app->size);
		LogDebug("Size mismatch in OP_MercenaryDismiss expected 0 got [{}]", app->size);
		DumpPacket(app);
		return;
	}

	uint8 Command = 0;
	if (app->size > 0)
	{
		char *InBuffer = (char *)app->pBuffer;
		Command = VARSTRUCT_DECODE_TYPE(uint8, InBuffer);
	}

	Log(Logs::General, Logs::Mercenaries, "Dismiss Request ( %i ) Received for %s.", Command, GetName());

	// Handle the dismiss here...
	DismissMerc(GetMercInfo().mercid);

}

void Client::Handle_OP_MercenaryHire(const EQApplicationPacket *app)
{
	// The payload is 16 bytes. First four bytes are the Merc ID (Template ID)
	if (app->size != sizeof(MercenaryMerchantRequest_Struct))
	{
		LogDebug("Size mismatch in OP_MercenaryHire expected [{}] got [{}]", sizeof(MercenaryMerchantRequest_Struct), app->size);

		DumpPacket(app);

		return;
	}

	MercenaryMerchantRequest_Struct* mmrq = (MercenaryMerchantRequest_Struct*)app->pBuffer;
	uint32 merc_template_id = mmrq->MercID;
	uint32 merchant_id = mmrq->MercMerchantID;
	uint32 merc_unk1 = mmrq->MercUnk01;
	uint32 merc_unk2 = mmrq->MercUnk02;

	Log(Logs::General, Logs::Mercenaries, "Template ID (%i), Merchant ID (%i), Unknown1 (%i), Unknown2 (%i), Client: %s", merc_template_id, merchant_id, merc_unk1, merc_unk2, GetName());

	//HirePending = true;
	SetHoTT(0);
	SendTargetCommand(0);

	if (!RuleB(Mercs, AllowMercs))
		return;

	MercTemplate* merc_template = zone->GetMercTemplate(merc_template_id);

	if (merc_template)
	{

		Mob* merchant = entity_list.GetNPCByID(merchant_id);
		if (!CheckCanHireMerc(merchant, merc_template_id))
		{
			return;
		}

		// Set time remaining to max on Hire
		GetMercInfo().MercTimerRemaining = RuleI(Mercs, UpkeepIntervalMS);

		// Get merc, assign it to client & spawn
		Merc* merc = Merc::LoadMercenary(this, merc_template, merchant_id, false);

		if (merc)
		{
			SpawnMerc(merc, true);
			merc->Save();

			if (RuleB(Mercs, ChargeMercPurchaseCost))
			{
				uint32 cost = Merc::CalcPurchaseCost(merc_template->MercTemplateID, GetLevel()) * 100; // Cost is in gold
				TakeMoneyFromPP(cost, true);
			}

			// approved hire request
			SendMercMerchantResponsePacket(0);
		}
		else
		{
			//merc failed to spawn
			SendMercMerchantResponsePacket(3);
		}
	}
	else
	{
		//merc doesn't exist in db
		SendMercMerchantResponsePacket(2);
	}
}

void Client::Handle_OP_MercenarySuspendRequest(const EQApplicationPacket *app)
{
	if (app->size != sizeof(SuspendMercenary_Struct))
	{
		Message(Chat::Red, "Size mismatch in OP_MercenarySuspendRequest expected %i got %i", sizeof(SuspendMercenary_Struct), app->size);
		LogDebug("Size mismatch in OP_MercenarySuspendRequest expected [{}] got [{}]", sizeof(SuspendMercenary_Struct), app->size);
		DumpPacket(app);
		return;
	}

	SuspendMercenary_Struct* sm = (SuspendMercenary_Struct*)app->pBuffer;
	uint32 merc_suspend = sm->SuspendMerc;	// Seen 30 for suspending or unsuspending

	Log(Logs::General, Logs::Mercenaries, "Suspend ( %i ) received for %s.", merc_suspend, GetName());

	if (!RuleB(Mercs, AllowMercs))
		return;

	// Check if the merc is suspended and if so, unsuspend, otherwise suspend it
	SuspendMercCommand();
}

void Client::Handle_OP_MercenaryTimerRequest(const EQApplicationPacket *app)
{
	// The payload is 0 bytes.
	if (app->size > 1)
	{
		Message(Chat::Red, "Size mismatch in OP_MercenaryTimerRequest expected 0 got %i", app->size);
		LogDebug("Size mismatch in OP_MercenaryTimerRequest expected 0 got [{}]", app->size);
		DumpPacket(app);
		return;
	}

	Log(Logs::General, Logs::Mercenaries, "Timer Request received for %s.", GetName());

	if (!RuleB(Mercs, AllowMercs)) {
		return;
	}

	// To Do: Load Mercenary Timer Data to properly populate this reply packet
	// All hard set values for now
	uint32 entityID = 0;
	uint32 mercState = 5;
	uint32 suspendedTime = 0;
	if (GetMercenaryID()) {
		Merc* merc = GetMerc();

		if (merc) {
			entityID = merc->GetID();

			if (GetMercInfo().IsSuspended) {
				mercState = 1;
				suspendedTime = GetMercInfo().SuspendedTime;
			}
		}
	}

	if (entityID > 0) {
		SendMercTimerPacket(entityID, mercState, suspendedTime, GetMercInfo().MercTimerRemaining, RuleI(Mercs, SuspendIntervalMS));
	}
}

void Client::Handle_OP_MoveCoin(const EQApplicationPacket *app)
{
	if (app->size != sizeof(MoveCoin_Struct)) {
		LogError("Wrong size on OP_MoveCoin. Got: [{}], Expected: [{}]", app->size, sizeof(MoveCoin_Struct));
		DumpPacket(app);
		return;
	}
	OPMoveCoin(app);
	return;
}

void Client::Handle_OP_MoveItem(const EQApplicationPacket *app)
{
	if (!CharacterID()) {
		return;
	}

	if (app->size != sizeof(MoveItem_Struct)) {
		LogError("Wrong size: OP_MoveItem, size=[{}], expected [{}]", app->size, sizeof(MoveItem_Struct));
		return;
	}

	ResetAFKTimer();

	BenchTimer bench;

	MoveItem_Struct* mi = (MoveItem_Struct*) app->pBuffer;
	if (spellend_timer.Enabled() && casting_spell_id && !IsBardSong(casting_spell_id)) {
		if (mi->from_slot != mi->to_slot && (mi->from_slot <= EQ::invslot::GENERAL_END || mi->from_slot > 39) &&
			IsValidSlot(mi->from_slot) && IsValidSlot(mi->to_slot)) {
			const EQ::ItemInstance* itm_from = GetInv().GetItem(mi->from_slot);
			const EQ::ItemInstance* itm_to   = GetInv().GetItem(mi->to_slot);
			auto message = fmt::format(
				"Player issued a move item from {}(item id {}) to {}(item id {}) while casting {}.",
				mi->from_slot,
				itm_from ? itm_from->GetID() : 0,
				mi->to_slot,
				itm_to ? itm_to->GetID() : 0,
				casting_spell_id
			);
			RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{ .message = message });
			Kick("Inventory desync"); // Kick client to prevent client and server from getting out-of-sync inventory slots
			return;
		}
	}

	database.TransactionBegin();

	if (!SwapItem(mi) && IsValidSlot(mi->from_slot) && IsValidSlot(mi->to_slot)) {
		SwapItemResync(mi);

		bool error = false;
		InterrogateInventory(this, false, true, false, error, false);
		if (error) {
			InterrogateInventory(this, true, false, true, error);
		}
	}

	for (int slot : {mi->to_slot, mi->from_slot}) {
		auto item = GetInv().GetItem(slot);
		if (item && item->IsEvolving()) {
			CharacterEvolvingItemsRepository::UpdateOne(database, item->GetEvolvingDetails());
		}
	}

	database.TransactionCommit();

	LogInventory("OP_MoveItem took [{}] ms", bench.elapsedMilliseconds());
}

void Client::Handle_OP_MoveMultipleItems(const EQApplicationPacket *app)
{
	// This packet is only sent from the client if we ctrl click items in inventory
	if (m_ClientVersionBit & EQ::versions::maskRoF2AndLater) {
		if (!CharacterID()) {
			LinkDead();
			return;
		}

		if (app->size < sizeof(MultiMoveItem_Struct)) {
			LinkDead();
			return; // Not enough data to be a valid packet
		}

		const MultiMoveItem_Struct* multi_move = reinterpret_cast<const MultiMoveItem_Struct*>(app->pBuffer);
		if (app->size != sizeof(MultiMoveItem_Struct) + sizeof(MultiMoveItemSub_Struct) * multi_move->count) {
			LinkDead();
			return; // Packet size does not match expected size
		}

		const int16 from_parent = multi_move->moves[0].from_slot.Slot;
		const int16 to_parent   = multi_move->moves[0].to_slot.Slot;

		// CTRL + left click drops an item into a bag without opening it.
		// This can be a bag, in which case it tries to fill the target bag with the contents of the bag on the cursor
		// CTRL + right click swaps the contents of two bags if a bag is on your cursor and you ctrl-right click on another bag.

		// We need to check if this is a swap or just an addition (left click or right click)
		// Check if any component of this transaction is coming from anywhere other than the cursor
		bool left_click = true;
		for (int i = 0; i < multi_move->count; i++) {
			if (multi_move->moves[i].from_slot.Slot != EQ::invslot::slotCursor) {
				left_click = false;
			}
		}

		// This is a left click which is purely additive. This should always be cursor object or cursor bag contents into general\bank\whatever bag
		if (left_click) {
			for (int i = 0; i < multi_move->count; i++) {
				MoveItem_Struct* mi = new MoveItem_Struct();
				mi->from_slot 	= multi_move->moves[i].from_slot.SubIndex == -1 ? multi_move->moves[i].from_slot.Slot : m_inv.CalcSlotId(multi_move->moves[i].from_slot.Slot, multi_move->moves[i].from_slot.SubIndex);
				mi->to_slot = m_inv.CalcSlotId(multi_move->moves[i].to_slot.Slot, multi_move->moves[i].to_slot.SubIndex);

				if (multi_move->moves[i].to_slot.Type == EQ::invtype::typeBank) { // Target is bank inventory
					mi->to_slot = m_inv.CalcSlotId(multi_move->moves[i].to_slot.Slot + EQ::invslot::BANK_BEGIN, multi_move->moves[i].to_slot.SubIndex);
				} else if (multi_move->moves[i].to_slot.Type == EQ::invtype::typeSharedBank) { // Target is shared bank inventory
					mi->to_slot = m_inv.CalcSlotId(multi_move->moves[i].to_slot.Slot + EQ::invslot::SHARED_BANK_BEGIN, multi_move->moves[i].to_slot.SubIndex);
				}

				// This sends '1' as the stack count for unstackable items, which our titanium-era SwapItem blows up
				if (m_inv.GetItem(mi->from_slot)->IsStackable()) {
					mi->number_in_stack = multi_move->moves[i].number_in_stack;
				} else {
					mi->number_in_stack = 0;
				}

				if (!SwapItem(mi) && IsValidSlot(mi->from_slot) && IsValidSlot(mi->to_slot)) {
					bool error = false;
					SwapItemResync(mi);
					InterrogateInventory(this, false, true, false, error, false);
					if (error) {
						InterrogateInventory(this, true, false, true, error);
					}
				}
				safe_delete(mi);
			}
		// This is the swap.
		// Client behavior is just to move stacks without combining them
		// Items get rearranged to fill the 'top' of the bag first
		} else {
			struct MoveInfo {
				EQ::ItemInstance* item;
				uint16 to_slot;
			};

			std::vector<MoveInfo> items;
    		items.reserve(multi_move->count);

			for (int i = 0; i < multi_move->count; i++) {
				// These are always bags, so we don't need to worry about raw items in slotCursor
				uint16 from_slot   = m_inv.CalcSlotId(multi_move->moves[i].from_slot.Slot, multi_move->moves[i].from_slot.SubIndex);
				if (multi_move->moves[i].from_slot.Type == EQ::invtype::typeBank) { // Target is bank inventory
					from_slot = m_inv.CalcSlotId(multi_move->moves[i].from_slot.Slot + EQ::invslot::BANK_BEGIN, multi_move->moves[i].from_slot.SubIndex);
				} else if (multi_move->moves[i].from_slot.Type == EQ::invtype::typeSharedBank) { // Target is shared bank inventory
					from_slot = m_inv.CalcSlotId(multi_move->moves[i].from_slot.Slot + EQ::invslot::SHARED_BANK_BEGIN, multi_move->moves[i].from_slot.SubIndex);
				}

				uint16 to_slot   = m_inv.CalcSlotId(multi_move->moves[i].to_slot.Slot, multi_move->moves[i].to_slot.SubIndex);
				if (multi_move->moves[i].to_slot.Type == EQ::invtype::typeBank) { // Target is bank inventory
					to_slot = m_inv.CalcSlotId(multi_move->moves[i].to_slot.Slot + EQ::invslot::BANK_BEGIN, multi_move->moves[i].to_slot.SubIndex);
				} else if (multi_move->moves[i].to_slot.Type == EQ::invtype::typeSharedBank) { // Target is shared bank inventory
					to_slot = m_inv.CalcSlotId(multi_move->moves[i].to_slot.Slot + EQ::invslot::SHARED_BANK_BEGIN, multi_move->moves[i].to_slot.SubIndex);
				}

				// I wasn't able to produce any error states on purpose.
				MoveInfo move{
					.item = m_inv.PopItem(from_slot), // Don't delete the instance here
					.to_slot = to_slot
				};

				if (move.item) {
					items.push_back(move);
					database.SaveInventory(CharacterID(), NULL, from_slot); // We have to manually save inventory here.
				} else {
					LinkDead();
					return; // Prevent inventory desync here. Forcing a resync would be better, but we don't have a MoveItem struct to work with.
				}
			}

			for (const MoveInfo& move : items) {
				PutItemInInventory(move.to_slot, *move.item); // This saves inventory too
			}
		}

	} else {
		LinkDead(); // This packet should not be sent by an older client
		return;
	}
}

void Client::Handle_OP_OpenContainer(const EQApplicationPacket *app)
{
	// Does not exist in Ti client
	// SoF, SoD and UF clients send a 4-byte packet indicating the 'parent' slot
	// SoF, SoD and UF slots are defined by a uint32 value and currently untranslated
	// RoF client sends a 12-byte packet based on the RoF::Structs::ItemSlotStruct

	// RoF structure types are defined as signed uint16 and currently untranslated
	// RoF::struct.SlotType = {0 - Equipment, 1 - Bank, 2 - Shared Bank} // not tested beyond listed types
	// RoF::struct.Unknown2 = 0
	// RoF::struct.MainSlot = { <parent slot range designated by slottype..zero-based> }
	// RoF::struct.SubSlot = -1 (non-child)
	// RoF::struct.AugSlot = -1 (non-child)
	// RoF::struct.Unknown1 = 141 (unsure why, but always appears to be this value..combine containers not tested)

	// SideNote: Watching the slot translations, Unknown1 is showing '141' as well on certain item swaps.
	// Manually looting a corpse results in a from '34' to '68' value for equipment items, '0' to '0' for inventory.
}

void Client::Handle_OP_OpenGuildTributeMaster(const EQApplicationPacket *app)
{
	LogTribute("Received OP_OpenGuildTributeMaster of length [{}]", app->size);

	if (app->size != sizeof(StartTribute_Struct))
		printf("Error in OP_OpenGuildTributeMaster. Expected size of: %zu, but got: %i\n", sizeof(StartTribute_Struct), app->size);
	else {
		//Opens the guild tribute master window
		StartTribute_Struct* st = (StartTribute_Struct*)app->pBuffer;
		Mob* tribmast = entity_list.GetMob(st->tribute_master_id);
		if (tribmast && tribmast->IsNPC() && tribmast->GetClass() == Class::GuildTributeMaster
			&& DistanceSquared(m_Position, tribmast->GetPosition()) <= USE_NPC_RANGE2) {
			st->response = 1;
			QueuePacket(app);
			tribute_master_id = st->tribute_master_id;
		}
		else {
			st->response = 0;
			QueuePacket(app);
		}
	}
	return;
}

void Client::Handle_OP_OpenInventory(const EQApplicationPacket *app)
{
	// Does not exist in Ti, UF or RoF clients
	// SoF and SoD both send a 4-byte packet with a uint32 value of '8'
}

void Client::Handle_OP_OpenTributeMaster(const EQApplicationPacket *app)
{
	LogTribute("Received OP_OpenTributeMaster of length [{}]", app->size);

	if (app->size != sizeof(StartTribute_Struct))
		printf("Error in OP_OpenTributeMaster. Expected size of: %zu, but got: %i\n", sizeof(StartTribute_Struct), app->size);
	else {
		//Opens the tribute master window
		StartTribute_Struct* st = (StartTribute_Struct*)app->pBuffer;
		Mob* tribmast = entity_list.GetMob(st->tribute_master_id);
		if (tribmast && tribmast->IsNPC() && tribmast->GetClass() == Class::TributeMaster
			&& DistanceSquared(m_Position, tribmast->GetPosition()) <= USE_NPC_RANGE2) {
			st->response = 1;
			QueuePacket(app);
			tribute_master_id = st->tribute_master_id;
			DoTributeUpdate();
		}
		else {
			st->response = 0;
			QueuePacket(app);
		}
	}
	return;
}

void Client::Handle_OP_PDeletePetition(const EQApplicationPacket *app)
{
	if (app->size < 2) {
		LogError("Wrong size: OP_PDeletePetition, size=[{}], expected [{}]", app->size, 2);
		return;
	}
	if (PetitionList::Instance()->DeletePetitionByCharName((char*)app->pBuffer))
		MessageString(Chat::White, PETITION_DELETED);
	else
		MessageString(Chat::White, PETITION_NO_DELETE);
	return;
}


void Client::Handle_OP_PetCommands(const EQApplicationPacket *app)
{
	if (app->size != sizeof(PetCommand_Struct)) {
		LogError("Wrong size: OP_PetCommands, size=[{}], expected [{}]", app->size, sizeof(PetCommand_Struct));
		return;
	}
	char val1[20] = { 0 };
	PetCommand_Struct* pet = (PetCommand_Struct*)app->pBuffer;
	Mob* mypet = GetPet();
	Mob *target = entity_list.GetMob(pet->target);

	if (!mypet || pet->command == PET_LEADER) {
		if (pet->command == PET_LEADER) {
			// we either send the ID of an NPC we're interested in or no ID for our own pet
			if (target) {
				auto owner = target->GetOwner();
				if (owner)
					target->SayString(PET_LEADERIS, owner->GetCleanName());
				else
					target->SayString(I_FOLLOW_NOONE);
			} else if (mypet) {
				mypet->SayString(PET_LEADERIS, GetName());
			}
		}

		return;
	}

	if (mypet->GetPetType() == petTargetLock && (pet->command != PET_HEALTHREPORT && pet->command != PET_GETLOST))
		return;

	// just let the command "/pet get lost" work for familiars
	if (mypet->GetPetType() == petFamiliar && pet->command != PET_GETLOST)
		return;

	uint32 PetCommand = pet->command;

	// Handle Sit/Stand toggle in UF and later.
	/*
	if (GetClientVersion() >= EQClientUnderfoot)
	{
	if (PetCommand == PET_SITDOWN)
	if (mypet->GetPetOrder() == SPO_Sit)
	PetCommand = PET_STANDUP;
	}
	*/

	switch (PetCommand)
	{
	case PET_ATTACK: {
		if (!target)
			break;

		if (RuleB(Pets, PetsRequireLoS) && !DoLosChecks(target)) {
			mypet->SayString(this, NOT_LEGAL_TARGET);
			break;
		}

		if (target->IsMezzed()) {
			MessageString(Chat::NPCQuestSay, CANNOT_WAKE, mypet->GetCleanName(), target->GetCleanName());
			break;
		}

		if (mypet->IsFeared())
			break; //prevent pet from attacking stuff while feared

		if (!mypet->IsAttackAllowed(target)) {
			mypet->SayString(this, NOT_LEGAL_TARGET);
			break;
		}

		// default range is 200, takes Z into account
		// really they do something weird where they're added to the aggro list then remove them
		// and will attack if they come in range -- too lazy, lets remove exploits for now
		if (DistanceSquared(mypet->GetPosition(), target->GetPosition()) >= RuleR(Aggro, PetAttackRange)) {
			// they say they're attacking then remove on live ... so they don't really say anything in this case ...
			break;
		}

		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			if (target != this && DistanceSquared(mypet->GetPosition(), target->GetPosition()) <= (RuleR(Pets, AttackCommandRange)*RuleR(Pets, AttackCommandRange))) {
				mypet->SetFeigned(false);
				if (mypet->IsPetStop()) {
					mypet->SetPetStop(false);
					SetPetCommandState(PET_BUTTON_STOP, 0);
				}
				if (mypet->IsPetRegroup()) {
					mypet->SetPetRegroup(false);
					SetPetCommandState(PET_BUTTON_REGROUP, 0);
				}

				// fix GUI sit button to be unpressed and stop sitting regen
				SetPetCommandState(PET_BUTTON_SIT, 0);
				if (mypet->GetPetOrder() == SPO_Sit || mypet->GetPetOrder() == SPO_FeignDeath) {
					mypet->SetPetOrder(mypet->GetPreviousPetOrder());
					mypet->SetAppearance(eaStanding);
				}

				zone->AddAggroMob();
				// classic acts like qattack
				int hate = 1;
				if (mypet->IsEngaged()) {
					auto top = mypet->GetHateMost();
					if (top && top != target)
						hate += mypet->GetHateAmount(top) - mypet->GetHateAmount(target) + 100; // should be enough to cause target change
				}
				mypet->AddToHateList(target, hate, 0, true, false, false, SPELL_UNKNOWN, true);
				MessageString(Chat::PetResponse, PET_ATTACKING, mypet->GetCleanName(), target->GetCleanName());
				SetTarget(target);
			}
		}
		break;
	}
	case PET_QATTACK: {
		if (mypet->IsFeared())
			break; //prevent pet from attacking stuff while feared

		if (!GetTarget()) {
			break;
		}

		if (RuleB(Pets, PetsRequireLoS) && !DoLosChecks(GetTarget())) {
			mypet->SayString(this, NOT_LEGAL_TARGET);
			break;
		}

		if (GetTarget()->IsMezzed()) {
			MessageString(Chat::NPCQuestSay, CANNOT_WAKE, mypet->GetCleanName(), GetTarget()->GetCleanName());
			break;
		}

		if (!mypet->IsAttackAllowed(GetTarget())) {
			mypet->SayString(this, NOT_LEGAL_TARGET);
			break;
		}

		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			if (GetTarget() != this && DistanceSquaredNoZ(mypet->GetPosition(), GetTarget()->GetPosition()) <= (RuleR(Pets, AttackCommandRange)*RuleR(Pets, AttackCommandRange))) {
				mypet->SetFeigned(false);
				if (mypet->IsPetStop()) {
					mypet->SetPetStop(false);
					SetPetCommandState(PET_BUTTON_STOP, 0);
				}
				if (mypet->IsPetRegroup()) {
					mypet->SetPetRegroup(false);
					SetPetCommandState(PET_BUTTON_REGROUP, 0);
				}

				// fix GUI sit button to be unpressed and stop sitting regen
				SetPetCommandState(PET_BUTTON_SIT, 0);
				if (mypet->GetPetOrder() == SPO_Sit || mypet->GetPetOrder() == SPO_FeignDeath) {
					mypet->SetPetOrder(mypet->GetPreviousPetOrder());
					mypet->SetAppearance(eaStanding);
				}

				zone->AddAggroMob();
				mypet->AddToHateList(GetTarget(), 1, 0, true, false, false, SPELL_UNKNOWN, true);
				MessageString(Chat::PetResponse, PET_ATTACKING, mypet->GetCleanName(), GetTarget()->GetCleanName());
			}
		}
		break;
	}
	case PET_BACKOFF: {
		if (mypet->IsFeared()) break; //keeps pet running while feared

		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			mypet->SayString(this, Chat::PetResponse, PET_CALMING);
			mypet->WipeHateList();
			mypet->SetTarget(nullptr);
			if (mypet->IsPetStop()) {
				mypet->SetPetStop(false);
				SetPetCommandState(PET_BUTTON_STOP, 0);
			}
		}
		break;
	}
	case PET_HEALTHREPORT: {
		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			MessageString(Chat::PetResponse, PET_REPORT_HP, ConvertArrayF(mypet->GetHPRatio(), val1));
			mypet->ShowBuffs(this);
		}
		break;
	}
	case PET_GETLOST: {
		if (mypet->Charmed())
			break;
		if (mypet->GetPetType() == petCharmed || !mypet->IsNPC()) {
			// eqlive ignores this command
			// we could just remove the charm
			// and continue
			mypet->BuffFadeByEffect(SpellEffect::Charm);
			break;
		}
		else {
			SetPet(nullptr);
		}

		mypet->SayString(this, Chat::PetResponse, PET_GETLOST_STRING);
		mypet->CastToNPC()->Depop();

		//Oddly, the client (Titanium) will still allow "/pet get lost" command despite me adding the code below. If someone can figure that out, you can uncomment this code and use it.
		/*
		if((mypet->GetPetType() == petAnimation && GetAA(aaAnimationEmpathy) >= 2) || mypet->GetPetType() != petAnimation) {
		mypet->SayString(PET_GETLOST_STRING);
		mypet->CastToNPC()->Depop();
		}
		*/

		break;
	}
	case PET_GUARDHERE: {
		if (mypet->IsFeared()) break; //could be exploited like PET_BACKOFF

		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			if (mypet->IsNPC()) {

				// Set Sit button to unpressed - send stand anim/end hpregen
				mypet->SetFeigned(false);
				SetPetCommandState(PET_BUTTON_SIT, 0);
				mypet->SetAppearance(eaStanding);

				mypet->SayString(this, Chat::PetResponse, PET_GUARDINGLIFE);
				mypet->SetPetOrder(SPO_Guard);
				mypet->CastToNPC()->SaveGuardSpot(mypet->GetPosition());
				if (!mypet->GetTarget()) // want them to not twitch if they're chasing something down
					mypet->StopNavigation();
				if (mypet->IsPetStop()) {
					mypet->SetPetStop(false);
					SetPetCommandState(PET_BUTTON_STOP, 0);
				}
			}
		}
		break;
	}
	case PET_FOLLOWME: {
		if (mypet->IsFeared()) break; //could be exploited like PET_BACKOFF

		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			mypet->SetFeigned(false);
			mypet->SayString(this, Chat::PetResponse, PET_FOLLOWING);
			mypet->SetPetOrder(SPO_Follow);

			// fix GUI sit button to be unpressed - send stand anim/end hpregen
			SetPetCommandState(PET_BUTTON_SIT, 0);
			mypet->SetAppearance(eaStanding);

			if (mypet->IsPetStop()) {
				mypet->SetPetStop(false);
				SetPetCommandState(PET_BUTTON_STOP, 0);
			}
		}
		break;
	}
	case PET_TAUNT: {
		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			if (mypet->CastToNPC()->IsTaunting())
			{
				MessageString(Chat::PetResponse, PET_NO_TAUNT);
				mypet->CastToNPC()->SetTaunting(false);
			}
			else
			{
				MessageString(Chat::PetResponse, PET_DO_TAUNT);
				mypet->CastToNPC()->SetTaunting(true);
			}
		}
		break;
	}
	case PET_TAUNT_ON: {
		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			MessageString(Chat::PetResponse, PET_DO_TAUNT);
			mypet->CastToNPC()->SetTaunting(true);
		}
		break;
	}
	case PET_TAUNT_OFF: {
		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			MessageString(Chat::PetResponse, PET_NO_TAUNT);
			mypet->CastToNPC()->SetTaunting(false);
		}
		break;
	}
	case PET_GUARDME: {
		if (mypet->IsFeared()) break; //could be exploited like PET_BACKOFF

		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			mypet->SetFeigned(false);
			mypet->SayString(this, Chat::PetResponse, PET_GUARDME_STRING);
			mypet->SetPetOrder(SPO_Follow);

			// Set Sit button to unpressed - send stand anim/end hpregen
			SetPetCommandState(PET_BUTTON_SIT, 0);
			mypet->SetAppearance(eaStanding);

			if (mypet->IsPetStop()) {
				mypet->SetPetStop(false);
				SetPetCommandState(PET_BUTTON_STOP, 0);
			}
		}
		break;
	}
	case PET_SIT: {
		if (mypet->IsFeared()) break; //could be exploited like PET_BACKOFF

		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			if (mypet->GetPetOrder() == SPO_Sit)
			{
				mypet->SetFeigned(false);
				mypet->SayString(this, Chat::PetResponse, PET_SIT_STRING);
				mypet->SetPetOrder(mypet->GetPreviousPetOrder());
				mypet->SetAppearance(eaStanding);
			}
			else
			{
				mypet->SetFeigned(false);
				mypet->SayString(this, Chat::PetResponse, PET_SIT_STRING);
				mypet->SetPetOrder(SPO_Sit);
				mypet->SetRunAnimSpeed(0);
				if (!mypet->UseBardSpellLogic())	//maybe we can have a bard pet
					mypet->InterruptSpell(); //No cast 4 u. //i guess the pet should start casting
				mypet->SetAppearance(eaSitting);
			}
		}
		break;
	}
	case PET_STANDUP: {
		if (mypet->IsFeared()) break; //could be exploited like PET_BACKOFF

		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			mypet->SetFeigned(false);
			mypet->SayString(this, Chat::PetResponse, PET_SIT_STRING);
			SetPetCommandState(PET_BUTTON_SIT, 0);
			mypet->SetPetOrder(mypet->GetPreviousPetOrder());
			mypet->SetAppearance(eaStanding);
		}
		break;
	}
	case PET_SITDOWN: {
		if (mypet->IsFeared()) break; //could be exploited like PET_BACKOFF

		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			mypet->SetFeigned(false);
			mypet->SayString(this, Chat::PetResponse, PET_SIT_STRING);
			SetPetCommandState(PET_BUTTON_SIT, 1);
			mypet->SetPetOrder(SPO_Sit);
			mypet->SetRunAnimSpeed(0);
			if (!mypet->UseBardSpellLogic())	//maybe we can have a bard pet
				mypet->InterruptSpell(); //No cast 4 u. //i guess the pet should start casting
			mypet->SetAppearance(eaSitting);
		}
		break;
	}
	case PET_HOLD: {
		if (aabonuses.PetCommands[PetCommand] && mypet->IsNPC()) {
			if (mypet->IsHeld())
			{
				if (m_ClientVersionBit & EQ::versions::maskSoDAndLater)
					MessageString(Chat::PetResponse, PET_HOLD_SET_OFF);
				mypet->SetHeld(false);
			}
			else
			{
				if (m_ClientVersionBit & EQ::versions::maskSoDAndLater)
					MessageString(Chat::PetResponse, PET_HOLD_SET_ON);

				if (m_ClientVersionBit & EQ::versions::maskUFAndLater)
					mypet->SayString(this, Chat::PetResponse, PET_NOW_HOLDING);
				else
					mypet->SayString(this, Chat::PetResponse, PET_ON_HOLD);

				mypet->SetHeld(true);
			}
			mypet->SetGHeld(false);
			SetPetCommandState(PET_BUTTON_GHOLD, 0);
		}
		break;
	}
	case PET_HOLD_ON: {
		if (aabonuses.PetCommands[PetCommand] && mypet->IsNPC() && !mypet->IsHeld()) {
			if (m_ClientVersionBit & EQ::versions::maskSoDAndLater)
				MessageString(Chat::PetResponse, PET_HOLD_SET_ON);

			if (m_ClientVersionBit & EQ::versions::maskUFAndLater)
				mypet->SayString(this, Chat::PetResponse, PET_NOW_HOLDING);
			else
				mypet->SayString(this, Chat::PetResponse, PET_ON_HOLD);
			mypet->SetHeld(true);
			mypet->SetGHeld(false);
			SetPetCommandState(PET_BUTTON_GHOLD, 0);
		}
		break;
	}
	case PET_HOLD_OFF: {
		if (aabonuses.PetCommands[PetCommand] && mypet->IsNPC() && mypet->IsHeld()) {
			if (m_ClientVersionBit & EQ::versions::maskSoDAndLater)
				MessageString(Chat::PetResponse, PET_HOLD_SET_OFF);
			mypet->SetHeld(false);
		}
		break;
	}
	case PET_GHOLD: {
		if (aabonuses.PetCommands[PetCommand] && mypet->IsNPC()) {
			if (mypet->IsGHeld())
			{
				if (m_ClientVersionBit & EQ::versions::maskUFAndLater)
					MessageString(Chat::PetResponse, PET_OFF_GHOLD);
				mypet->SetGHeld(false);
			}
			else
			{
				if (m_ClientVersionBit & EQ::versions::maskUFAndLater) {
					MessageString(Chat::PetResponse, PET_ON_GHOLD);
					mypet->SayString(this, Chat::PetResponse, PET_GHOLD_ON_MSG);
				} else {
					mypet->SayString(this, Chat::PetResponse, PET_ON_HOLD);
				}
				mypet->SetGHeld(true);
			}
			mypet->SetHeld(false);
			SetPetCommandState(PET_BUTTON_HOLD, 0);
		}
		break;
	}
	case PET_GHOLD_ON: {
		if (aabonuses.PetCommands[PetCommand] && mypet->IsNPC()) {
			if (m_ClientVersionBit & EQ::versions::maskUFAndLater) {
				MessageString(Chat::PetResponse, PET_ON_GHOLD);
				mypet->SayString(this, Chat::PetResponse, PET_GHOLD_ON_MSG);
			} else {
				mypet->SayString(this, Chat::PetResponse, PET_ON_HOLD);
			}
			mypet->SetGHeld(true);
			mypet->SetHeld(false);
			SetPetCommandState(PET_BUTTON_HOLD, 0);
		}
		break;
	}
	case PET_GHOLD_OFF: {
		if (aabonuses.PetCommands[PetCommand] && mypet->IsNPC() && mypet->IsGHeld()) {
			if (m_ClientVersionBit & EQ::versions::maskUFAndLater)
				MessageString(Chat::PetResponse, PET_OFF_GHOLD);
			mypet->SetGHeld(false);
		}
		break;
	}
	case PET_SPELLHOLD: {
		if (aabonuses.PetCommands[PetCommand] && mypet->IsNPC()) {
			if (mypet->IsFeared())
				break;
			if (mypet->IsNoCast()) {
				MessageString(Chat::PetResponse, PET_CASTING);
				if (m_ClientVersionBit & EQ::versions::maskSoDAndLater)
					MessageString(Chat::PetResponse, PET_SPELLHOLD_SET_OFF);
				mypet->SetNoCast(false);
			}
			else {
				MessageString(Chat::PetResponse, PET_NOT_CASTING);
				if (m_ClientVersionBit & EQ::versions::maskSoDAndLater)
					MessageString(Chat::PetResponse, PET_SPELLHOLD_SET_ON);
				mypet->SetNoCast(true);
			}
		}
		break;
	}
	case PET_SPELLHOLD_ON: {
		if (aabonuses.PetCommands[PetCommand] && mypet->IsNPC()) {
			if (mypet->IsFeared())
				break;
			if (!mypet->IsNoCast()) {
				MessageString(Chat::PetResponse, PET_NOT_CASTING);
				if (m_ClientVersionBit & EQ::versions::maskSoDAndLater)
					MessageString(Chat::PetResponse, PET_SPELLHOLD_SET_ON);
				mypet->SetNoCast(true);
			}
		}
		break;
	}
	case PET_SPELLHOLD_OFF: {
		if (aabonuses.PetCommands[PetCommand] && mypet->IsNPC()) {
			if (mypet->IsFeared())
				break;
			if (mypet->IsNoCast()) {
				MessageString(Chat::PetResponse, PET_CASTING);
				if (m_ClientVersionBit & EQ::versions::maskSoDAndLater)
					MessageString(Chat::PetResponse, PET_SPELLHOLD_SET_OFF);
				mypet->SetNoCast(false);
			}
		}
		break;
	}
	case PET_FOCUS: {
		if (aabonuses.PetCommands[PetCommand] && mypet->IsNPC()) {
			if (mypet->IsFeared())
				break;
			if (mypet->IsFocused()) {
				MessageString(Chat::PetResponse, PET_NOT_FOCUSING);
				if (m_ClientVersionBit & EQ::versions::maskSoDAndLater)
					MessageString(Chat::PetResponse, PET_FOCUS_SET_OFF);
				mypet->SetFocused(false);
			}
			else {
				MessageString(Chat::PetResponse, PET_NOW_FOCUSING);
				if (m_ClientVersionBit & EQ::versions::maskSoDAndLater)
					MessageString(Chat::PetResponse, PET_FOCUS_SET_ON);
				mypet->SetFocused(true);
			}
		}
		break;
	}
	case PET_FOCUS_ON: {
		if (aabonuses.PetCommands[PetCommand] && mypet->IsNPC()) {
			if (mypet->IsFeared())
				break;
			if (!mypet->IsFocused()) {
				MessageString(Chat::PetResponse, PET_NOW_FOCUSING);
				if (m_ClientVersionBit & EQ::versions::maskSoDAndLater)
					MessageString(Chat::PetResponse, PET_FOCUS_SET_ON);
				mypet->SetFocused(true);
			}
		}
		break;
	}
	case PET_FOCUS_OFF: {
		if (aabonuses.PetCommands[PetCommand] && mypet->IsNPC()) {
			if (mypet->IsFeared())
				break;
			if (mypet->IsFocused()) {
				MessageString(Chat::PetResponse, PET_NOT_FOCUSING);
				if (m_ClientVersionBit & EQ::versions::maskSoDAndLater)
					MessageString(Chat::PetResponse, PET_FOCUS_SET_OFF);
				mypet->SetFocused(false);
			}
		}
		break;
	}

	case PET_FEIGN: {
		if (aabonuses.PetCommands[PetCommand] && mypet->IsNPC()) {
			if (mypet->IsFeared())
				break;

			int pet_fd_chance = aabonuses.FeignedMinionChance;
			if (zone->random.Int(0, 99) > pet_fd_chance) {
				mypet->SetFeigned(false);
				entity_list.MessageCloseString(this, false, 200, 10, STRING_FEIGNFAILED, mypet->GetCleanName());
			}
			else {
				bool has_aggro_immunity = GetSpecialAbility(SpecialAbility::AggroImmunity);
				mypet->SetSpecialAbility(SpecialAbility::AggroImmunity, 1);
				mypet->WipeHateList();
				mypet->SetPetOrder(SPO_FeignDeath);
				mypet->SetRunAnimSpeed(0);
				mypet->StopNavigation();
				mypet->SetAppearance(eaDead);
				mypet->SetFeigned(true);
				mypet->SetTarget(nullptr);
				if (!mypet->UseBardSpellLogic()) {
					mypet->InterruptSpell();
				}

				if (!has_aggro_immunity) {
					mypet->SetSpecialAbility(SpecialAbility::AggroImmunity, 0);
				}
			}
		}
		break;
	}
	case PET_STOP: {
		if (mypet->IsFeared()) break; //could be exploited like PET_BACKOFF

		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			if (mypet->IsPetStop()) {
				mypet->SetPetStop(false);
			} else {
				mypet->SetPetStop(true);
				mypet->StopNavigation();
				mypet->SetTarget(nullptr);
				if (mypet->IsPetRegroup()) {
					mypet->SetPetRegroup(false);
					SetPetCommandState(PET_BUTTON_REGROUP, 0);
				}
			}
			mypet->SayString(this, Chat::PetResponse, PET_GETLOST_STRING);
		}
		break;
	}
	case PET_STOP_ON: {
		if (mypet->IsFeared()) break; //could be exploited like PET_BACKOFF

		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			mypet->SetPetStop(true);
			mypet->StopNavigation();
			mypet->SetTarget(nullptr);
			mypet->SayString(this, Chat::PetResponse, PET_GETLOST_STRING);
			if (mypet->IsPetRegroup()) {
				mypet->SetPetRegroup(false);
				SetPetCommandState(PET_BUTTON_REGROUP, 0);
			}
		}
		break;
	}
	case PET_STOP_OFF: {
		if (mypet->IsFeared()) break; //could be exploited like PET_BACKOFF

		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			mypet->SetPetStop(false);
			mypet->SayString(this, Chat::PetResponse, PET_GETLOST_STRING);
		}
		break;
	}
	case PET_REGROUP: {
		if (mypet->IsFeared()) break; //could be exploited like PET_BACKOFF

		if (aabonuses.PetCommands[PetCommand]) {
			if (mypet->IsPetRegroup()) {
				mypet->SetPetRegroup(false);
				mypet->SayString(this, Chat::PetResponse, PET_OFF_REGROUPING);
			} else {
				mypet->SetPetRegroup(true);
				mypet->SetTarget(nullptr);
				mypet->SayString(this, Chat::PetResponse, PET_ON_REGROUPING);
				if (mypet->IsPetStop()) {
					mypet->SetPetStop(false);
					SetPetCommandState(PET_BUTTON_STOP, 0);
				}
			}
		}
		break;
	}
	case PET_REGROUP_ON: {
		if (mypet->IsFeared()) break; //could be exploited like PET_BACKOFF

		if (aabonuses.PetCommands[PetCommand]) {
			mypet->SetPetRegroup(true);
			mypet->SetTarget(nullptr);
			mypet->SayString(this, Chat::PetResponse, PET_ON_REGROUPING);
			if (mypet->IsPetStop()) {
				mypet->SetPetStop(false);
				SetPetCommandState(PET_BUTTON_STOP, 0);
			}
		}
		break;
	}
	case PET_REGROUP_OFF: {
		if (mypet->IsFeared()) break; //could be exploited like PET_BACKOFF

		if (aabonuses.PetCommands[PetCommand]) {
			mypet->SetPetRegroup(false);
			mypet->SayString(this, Chat::PetResponse, PET_OFF_REGROUPING);
		}
		break;
	}
	default:
		printf("Client attempted to use a unknown pet command:\n");
		break;
	}
}
void Client::Handle_OP_Petition(const EQApplicationPacket *app)
{
	if (app->size <= 1)
		return;
	if (!worldserver.Connected())
		Message(Chat::Red, "Error: World server disconnected");
	/*else if(petition_list.FindPetitionByAccountName(AccountName()))
	{
	Message(Chat::White,"You already have a petition in queue, you cannot petition again until this one has been responded to or you have deleted the petition.");
	return;
	}*/
	else
	{
		if (PetitionList::Instance()->FindPetitionByAccountName(AccountName()))
		{
			Message(Chat::White, "You already have a petition in the queue, you must wait for it to be answered or use /deletepetition to delete it.");
			return;
		}
		auto pet = new Petition(CharacterID());
		pet->SetAName(AccountName());
		pet->SetClass(GetClass());
		pet->SetLevel(GetLevel());
		pet->SetCName(GetName());
		pet->SetRace(GetRace());
		pet->SetLastGM("");
		pet->SetCName(GetName());
		pet->SetPetitionText((char*)app->pBuffer);
		pet->SetZone(zone->GetZoneID());
		pet->SetUrgency(0);
		PetitionList::Instance()->AddPetition(pet);
		database.InsertPetitionToDB(pet);
		PetitionList::Instance()->UpdateGMQueue();
		PetitionList::Instance()->UpdateZoneListQueue();
		worldserver.SendEmoteMessage(
			0,
			0,
			AccountStatus::QuestTroupe,
			Chat::Yellow,
			fmt::format(
				"{} has made a petition. ID: {}",
				GetCleanName(),
				pet->GetID()
			).c_str()
		);
	}
	return;
}

void Client::Handle_OP_PetitionBug(const EQApplicationPacket *app)
{
	Message(Chat::White, "Petition Bugs are not supported, please use /bug.");

	return;
}

void Client::Handle_OP_PetitionCheckIn(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Petition_Struct)) {
		LogError("Wrong size: OP_PetitionCheckIn, size=[{}], expected [{}]", app->size, sizeof(Petition_Struct));
		return;
	}
	Petition_Struct* inpet = (Petition_Struct*)app->pBuffer;

	Petition* pet = PetitionList::Instance()->GetPetitionByID(inpet->petnumber);
	//if (inpet->urgency != pet->GetUrgency())
	pet->SetUrgency(inpet->urgency);
	pet->SetLastGM(GetName());
	pet->SetGMText(inpet->gmtext);

	pet->SetCheckedOut(false);
	PetitionList::Instance()->UpdatePetition(pet);
	PetitionList::Instance()->UpdateGMQueue();
	PetitionList::Instance()->UpdateZoneListQueue();
	return;
}

void Client::Handle_OP_PetitionCheckout(const EQApplicationPacket *app)
{
	if (app->size != sizeof(uint32)) {
		std::cout << "Wrong size: OP_PetitionCheckout, size=" << app->size << ", expected " << sizeof(uint32) << std::endl;
		return;
	}
	if (!worldserver.Connected())
		Message(Chat::Red, "Error: World server disconnected");
	else {
		uint32 getpetnum = *((uint32*)app->pBuffer);
		Petition* getpet = PetitionList::Instance()->GetPetitionByID(getpetnum);
		if (getpet != 0) {
			getpet->AddCheckout();
			getpet->SetCheckedOut(true);
			getpet->SendPetitionToPlayer(CastToClient());
			PetitionList::Instance()->UpdatePetition(getpet);
			PetitionList::Instance()->UpdateGMQueue();
			PetitionList::Instance()->UpdateZoneListQueue();
		}
	}
	return;
}

void Client::Handle_OP_PetitionDelete(const EQApplicationPacket *app)
{
	if (app->size != sizeof(PetitionUpdate_Struct)) {
		LogError("Wrong size: OP_PetitionDelete, size=[{}], expected [{}]", app->size, sizeof(PetitionUpdate_Struct));
		return;
	}
	auto outapp = new EQApplicationPacket(OP_PetitionUpdate, sizeof(PetitionUpdate_Struct));
	PetitionUpdate_Struct* pet = (PetitionUpdate_Struct*)outapp->pBuffer;
	pet->petnumber = *((int*)app->pBuffer);
	pet->color = 0x00;
	pet->status = 0xFFFFFFFF;
	pet->senttime = 0;
	strcpy(pet->accountid, "");
	strcpy(pet->gmsenttoo, "");
	pet->quetotal = PetitionList::Instance()->GetTotalPetitions();
	strcpy(pet->charname, "");
	FastQueuePacket(&outapp);

	if (PetitionList::Instance()->DeletePetition(pet->petnumber) == -1)
		std::cout << "Something is borked with: " << pet->petnumber << std::endl;
	PetitionList::Instance()->ClearPetitions();
	PetitionList::Instance()->UpdateGMQueue();
	PetitionList::Instance()->ReadDatabase();
	PetitionList::Instance()->UpdateZoneListQueue();
	return;
}

void Client::Handle_OP_PetitionQue(const EQApplicationPacket *app)
{
#ifdef _EQDEBUG
	printf("%s looking at petitions..\n", GetName());
#endif
	return;
}

void Client::Handle_OP_PetitionRefresh(const EQApplicationPacket *app)
{
	// This is When Client Asks for Petition Again and Again...
	// break is here because it floods the zones and causes lag if it
	// Were to actually do something:P We update on our own schedule now.
	return;
}

void Client::Handle_OP_PetitionResolve(const EQApplicationPacket *app)
{
	Handle_OP_PetitionDelete(app);
}

void Client::Handle_OP_PetitionUnCheckout(const EQApplicationPacket *app)
{
	if (app->size != sizeof(uint32)) {
		std::cout << "Wrong size: OP_PetitionUnCheckout, size=" << app->size << ", expected " << sizeof(uint32) << std::endl;
		return;
	}
	if (!worldserver.Connected())
		Message(Chat::Red, "Error: World server disconnected");
	else {
		uint32 getpetnum = *((uint32*)app->pBuffer);
		Petition* getpet = PetitionList::Instance()->GetPetitionByID(getpetnum);
		if (getpet != 0) {
			getpet->SetCheckedOut(false);
			PetitionList::Instance()->UpdatePetition(getpet);
			PetitionList::Instance()->UpdateGMQueue();
			PetitionList::Instance()->UpdateZoneListQueue();
		}
	}
	return;
}

void Client::Handle_OP_PlayerStateAdd(const EQApplicationPacket *app)
{
	if (app->size != sizeof(PlayerState_Struct)) {
		std::cout << "Wrong size: OP_PlayerStateAdd, size=" << app->size << ", expected " << sizeof(PlayerState_Struct) << std::endl;
		return;
	}

	PlayerState_Struct *ps = (PlayerState_Struct *)app->pBuffer;
	AddPlayerState(ps->state);

	entity_list.QueueClients(this, app, true);
}

void Client::Handle_OP_PlayerStateRemove(const EQApplicationPacket *app)
{
	if (app->size != sizeof(PlayerState_Struct)) {
		std::cout << "Wrong size: OP_PlayerStateRemove, size=" << app->size << ", expected " << sizeof(PlayerState_Struct) << std::endl;
		return;
	}
	PlayerState_Struct *ps = (PlayerState_Struct *)app->pBuffer;
	RemovePlayerState(ps->state);

	entity_list.QueueClients(this, app, true);
}

void Client::Handle_OP_PickPocket(const EQApplicationPacket *app)
{
	if (app->size != sizeof(PickPocket_Struct))
	{
		LogError("Size mismatch for Pick Pocket packet");
		DumpPacket(app);
	}

	if (!HasSkill(EQ::skills::SkillPickPockets))
	{
		return;
	}

	if (!p_timers.Expired(&database, pTimerBeggingPickPocket, false))
	{
		Message(Chat::Red, "Ability recovery time not yet met.");
		RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = "OP_PickPocket was sent again too quickly."});
		return;
	}
	PickPocket_Struct* pick_in = (PickPocket_Struct*)app->pBuffer;

	Mob* victim = entity_list.GetMob(pick_in->to);
	if (!victim)
		return;

	p_timers.Start(pTimerBeggingPickPocket, 8);

	if (victim == this) {
		Message(Chat::White, "You catch yourself red-handed.");
	}
	else if (victim->GetOwnerID()) {
		Message(Chat::White, "You cannot steal from pets!");
	}
	else if (victim->IsClient()) {
		Message(Chat::White, "Stealing from clients not yet supported.");
	}
	else if (Distance(GetPosition(), victim->GetPosition()) > 20) {
		Message(Chat::Red, "Attempt to pickpocket out of range detected.");
		RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = "OP_PickPocket was sent from outside combat range"});
	}
	else if (victim->IsNPC()) {
		auto body = victim->GetBodyType();
		if (body == BodyType::Humanoid || body == BodyType::Monster || body == BodyType::Giant ||
			body == BodyType::Lycanthrope) {
			victim->CastToNPC()->PickPocket(this);
			return;
		}
	}

	SendPickPocketResponse(victim, 0, PickPocketFailed);
}

void Client::Handle_OP_PickZone(const EQApplicationPacket *app)
{
	if (app->size != sizeof(PickZone_Struct)) {
		LogDebug("Size mismatch in OP_PickZone expected [{}] got [{}]", sizeof(PickZone_Struct), app->size);
		DumpPacket(app);
		return;
	}

	// handle
}

void Client::Handle_OP_PopupResponse(const EQApplicationPacket *app)
{

	if (app->size != sizeof(PopupResponse_Struct)) {
		LogDebug("Size mismatch in OP_PopupResponse expected [{}] got [{}]", sizeof(PopupResponse_Struct), app->size);

		DumpPacket(app);
		return;
	}

	PopupResponse_Struct *popup_response = (PopupResponse_Struct *) app->pBuffer;

	//Get Item Details if POPUPID_REPLACE_SPELLWINDOW was used

	/**
	 * Handle any EQEmu defined popup Ids first
	 */
	std::string response;
	switch (popup_response->popupid) {
		case POPUPID_REPLACE_SPELLWINDOW:
			DeleteItemInInventory(Strings::ToInt(GetEntityVariable("slot_id")), 1, true);
			MemorizeSpellFromItem(Strings::ToInt(GetEntityVariable("spell_id")));
			return;
			break;

		case POPUPID_UPDATE_SHOWSTATSWINDOW:
			if (GetTarget() && GetTarget()->IsOfClientBot()) {
				GetTarget()->SendStatsWindow(this, true);
			} else {
				SendStatsWindow(this, true);
			}
			return;
			break;

		case POPUPID_DIAWIND_ONE:
			if (EntityVariableExists(DIAWIND_RESPONSE_ONE_KEY)) {
				response = GetEntityVariable(DIAWIND_RESPONSE_ONE_KEY);
				if (!response.empty()) {
					ChannelMessageReceived(ChatChannel_Say, Language::CommonTongue, Language::MaxValue, response.c_str(), nullptr, true);
				}
			}
			break;

		case POPUPID_DIAWIND_TWO:
			if (EntityVariableExists(DIAWIND_RESPONSE_TWO_KEY)) {
				response = GetEntityVariable(DIAWIND_RESPONSE_TWO_KEY);
				if (!response.empty()) {
					ChannelMessageReceived(ChatChannel_Say, Language::CommonTongue, Language::MaxValue, response.c_str(), nullptr, true);
				}
			}
			break;

		case EQ::popupresponse::MOB_INFO_DISMISS:
			SetDisplayMobInfoWindow(false);
			Message(Chat::Yellow, "[DevTools] Window snoozed in this zone...");
			break;
		default:
			break;
	}

	if (parse->PlayerHasQuestSub(EVENT_POPUP_RESPONSE)) {
		parse->EventPlayer(EVENT_POPUP_RESPONSE, this, std::to_string(popup_response->popupid), 0);
	}

	if (parse->ZoneHasQuestSub(EVENT_POPUP_RESPONSE)) {
		std::vector<std::any> args = { this };
		parse->EventZone(EVENT_POPUP_RESPONSE, zone, std::to_string(popup_response->popupid), 0, &args);
	}

	auto t = GetTarget();
	if (t) {
		parse->EventBotMercNPC(EVENT_POPUP_RESPONSE, t, this, [&]() { return std::to_string(popup_response->popupid); });
	}
}

void Client::Handle_OP_PotionBelt(const EQApplicationPacket *app)
{
	if (app->size != sizeof(MovePotionToBelt_Struct)) {
		LogDebug("Size mismatch in OP_PotionBelt expected [{}] got [{}]", sizeof(MovePotionToBelt_Struct), app->size);
		DumpPacket(app);
		return;
	}

	MovePotionToBelt_Struct *mptbs = (MovePotionToBelt_Struct*)app->pBuffer;
	if (!EQ::ValueWithin(mptbs->SlotNumber, 0U, 3U)) {
		LogDebug("Client::Handle_OP_PotionBelt mptbs->SlotNumber out of range");
		return;
	}

	if (mptbs->Action == 0) {
		const EQ::ItemData *BaseItem = database.GetItem(mptbs->ItemID);
		if (BaseItem) {
			m_pp.potionbelt.Items[mptbs->SlotNumber].ID = BaseItem->ID;
			m_pp.potionbelt.Items[mptbs->SlotNumber].Icon = BaseItem->Icon;
			strn0cpy(m_pp.potionbelt.Items[mptbs->SlotNumber].Name, BaseItem->Name, sizeof(BaseItem->Name));
			database.SaveCharacterPotionBelt(CharacterID(), mptbs->SlotNumber, m_pp.potionbelt.Items[mptbs->SlotNumber].ID, m_pp.potionbelt.Items[mptbs->SlotNumber].Icon);
		}
	}
	else {
		m_pp.potionbelt.Items[mptbs->SlotNumber].ID = 0;
		m_pp.potionbelt.Items[mptbs->SlotNumber].Icon = 0;
		m_pp.potionbelt.Items[mptbs->SlotNumber].Name[0] = '\0';
	}
}

void Client::Handle_OP_PurchaseLeadershipAA(const EQApplicationPacket *app)
{
	if (app->size != sizeof(uint32)) {
		LogDebug("Size mismatch in OP_LeadershipExpToggle expected [{}] got [{}]", 1, app->size);
		DumpPacket(app);
		return;
	}
	uint32 aaid = *((uint32 *)app->pBuffer);

	if (aaid >= _maxLeaderAA)
		return;

	uint32 current_rank = m_pp.leader_abilities.ranks[aaid];
	if (current_rank >= MAX_LEADERSHIP_TIERS) {
		Message(Chat::Red, "This ability can be trained no further.");
		return;
	}

	uint8 cost = LeadershipAACosts[aaid][current_rank];
	if (cost == 0) {
		Message(Chat::Red, "This ability can be trained no further.");
		return;
	}

	//TODO: we need to enforce prerequisits

	if (aaid >= raidAAMarkNPC) {
		//it is a raid ability.
		if (cost > m_pp.raid_leadership_points) {
			Message(Chat::Red, "You do not have enough points to purchase this ability.");
			return;
		}

		//sell them the ability.
		m_pp.raid_leadership_points -= cost;
		m_pp.leader_abilities.ranks[aaid]++;

		database.SaveCharacterLeadershipAbilities(CharacterID(), &m_pp);
	}
	else {
		//it is a group ability.
		if (cost > m_pp.group_leadership_points) {
			Message(Chat::Red, "You do not have enough points to purchase this ability.");
			return;
		}

		//sell them the ability.
		m_pp.group_leadership_points -= cost;
		m_pp.leader_abilities.ranks[aaid]++;

		database.SaveCharacterLeadershipAbilities(CharacterID(), &m_pp);
	}

	//success, send them an update
	auto outapp = new EQApplicationPacket(OP_UpdateLeadershipAA, sizeof(UpdateLeadershipAA_Struct));
	UpdateLeadershipAA_Struct *u = (UpdateLeadershipAA_Struct *)outapp->pBuffer;
	u->ability_id = aaid;
	u->new_rank = m_pp.leader_abilities.ranks[aaid];
	if (aaid >= raidAAMarkNPC) // raid AA
		u->pointsleft = m_pp.raid_leadership_points;
	else // group AA
		u->pointsleft = m_pp.group_leadership_points;
	FastQueuePacket(&outapp);

	// Update all group members with the new AA the leader has purchased.
	if (IsRaidGrouped()) {
		Raid *r = GetRaid();
		if (!r)
			return;
		if (aaid >= raidAAMarkNPC) {
			r->UpdateRaidAAs();
			r->SendAllRaidLeadershipAA();
		}
		else {
			uint32 gid = r->GetGroup(this);
			r->UpdateGroupAAs(gid);
			r->GroupUpdate(gid, false);
		}
	}
	else if (IsGrouped()) {
		Group *g = GetGroup();
		if (!g)
			return;
		g->UpdateGroupAAs();
		g->SendLeadershipAAUpdate();
	}

}

void Client::Handle_OP_PVPLeaderBoardDetailsRequest(const EQApplicationPacket *app)
{
	// This opcode is sent by the client when the player right clicks a name on the PVP leaderboard and sends
	// further details about the selected player, e.g. Race/Class/AAs/Guild etc.
	//
	if (app->size != sizeof(PVPLeaderBoardDetailsRequest_Struct))
	{
		LogDebug("Size mismatch in OP_PVPLeaderBoardDetailsRequest expected [{}] got [{}]", sizeof(PVPLeaderBoardDetailsRequest_Struct), app->size);

		DumpPacket(app);

		return;
	}

	auto outapp = new EQApplicationPacket(OP_PVPLeaderBoardDetailsReply, sizeof(PVPLeaderBoardDetailsReply_Struct));
	PVPLeaderBoardDetailsReply_Struct *pvplbdrs = (PVPLeaderBoardDetailsReply_Struct *)outapp->pBuffer;

	// TODO: Record and send this data.

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::Handle_OP_PVPLeaderBoardRequest(const EQApplicationPacket *app)
{
	// This Opcode is sent by the client when the Leaderboard button on the PVP Stats window is pressed.
	//
	// It has a single uint32 payload which is the sort method:
	//
	// PVPSortByKills = 0, PVPSortByPoints = 1, PVPSortByInfamy = 2
	//
	if (app->size != sizeof(PVPLeaderBoardRequest_Struct))
	{
		LogDebug("Size mismatch in OP_PVPLeaderBoardRequest expected [{}] got [{}]", sizeof(PVPLeaderBoardRequest_Struct), app->size);

		DumpPacket(app);

		return;
	}
	/*PVPLeaderBoardRequest_Struct *pvplbrs = (PVPLeaderBoardRequest_Struct *)app->pBuffer;*/	//unused

	auto outapp = new EQApplicationPacket(OP_PVPLeaderBoardReply, sizeof(PVPLeaderBoard_Struct));
	/*PVPLeaderBoard_Struct *pvplb = (PVPLeaderBoard_Struct *)outapp->pBuffer;*/	//unused

																					// TODO: Record and send this data.

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::Handle_OP_QueryUCSServerStatus(const EQApplicationPacket *app)
{
	if (zone->IsUCSServerAvailable()) {
		EQApplicationPacket* outapp = nullptr;
		std::string buffer;

		std::string MailKey = GetMailKey();
		EQ::versions::UCSVersion ConnectionType = EQ::versions::ucsUnknown;

		// chat server packet
		switch (ClientVersion()) {
		case EQ::versions::ClientVersion::Titanium:
			ConnectionType = EQ::versions::ucsTitaniumChat;
			break;
		case EQ::versions::ClientVersion::SoF:
			ConnectionType = EQ::versions::ucsSoFCombined;
			break;
		case EQ::versions::ClientVersion::SoD:
			ConnectionType = EQ::versions::ucsSoDCombined;
			break;
		case EQ::versions::ClientVersion::UF:
			ConnectionType = EQ::versions::ucsUFCombined;
			break;
		case EQ::versions::ClientVersion::RoF:
			ConnectionType = EQ::versions::ucsRoFCombined;
			break;
		case EQ::versions::ClientVersion::RoF2:
			ConnectionType = EQ::versions::ucsRoF2Combined;
			break;
		default:
			ConnectionType = EQ::versions::ucsUnknown;
			break;
		}

		buffer = StringFormat("%s,%i,%s.%s,%c%s",
			Config->GetUCSHost().c_str(),
			Config->GetUCSPort(),
			Config->ShortName.c_str(),
			GetName(),
			ConnectionType,
			MailKey.c_str()
		);

		outapp = new EQApplicationPacket(OP_SetChatServer, (buffer.length() + 1));
		memcpy(outapp->pBuffer, buffer.c_str(), buffer.length());
		outapp->pBuffer[buffer.length()] = '\0';

		QueuePacket(outapp);
		safe_delete(outapp);

		// mail server packet
		switch (ClientVersion()) {
		case EQ::versions::ClientVersion::Titanium:
			ConnectionType = EQ::versions::ucsTitaniumMail;
			break;
		default:
			// retain value from previous switch
			break;
		}

		buffer = StringFormat("%s,%i,%s.%s,%c%s",
			Config->GetUCSHost().c_str(),
			Config->GetUCSPort(),
			Config->ShortName.c_str(),
			GetName(),
			ConnectionType,
			MailKey.c_str()
		);

		outapp = new EQApplicationPacket(OP_SetChatServer2, (buffer.length() + 1));
		memcpy(outapp->pBuffer, buffer.c_str(), buffer.length());
		outapp->pBuffer[buffer.length()] = '\0';

		QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void Client::Handle_OP_RaidCommand(const EQApplicationPacket* app)
{
	if (app->size < sizeof(RaidGeneral_Struct)) {
		LogError("Wrong size: OP_RaidCommand, size=[{}], expected at least [{}]", app->size, sizeof(RaidGeneral_Struct));
		DumpPacket(app);
		return;
	}

	auto raid_command_packet = (RaidGeneral_Struct*)app->pBuffer;
	switch (raid_command_packet->action)
	{
	case RaidCommandInviteIntoExisting:
	case RaidCommandInvite: {
		if (RuleB(Bots, Enabled) && entity_list.GetBotByBotName(raid_command_packet->player_name)) {
			auto player_to_invite = entity_list.GetBotByBotName(raid_command_packet->player_name);

			if (!player_to_invite) {
				break;
			}

			auto player_to_invite_group = player_to_invite->GetGroup();
			if (player_to_invite_group && player_to_invite_group->IsGroupMember(this)) {
				MessageString(Chat::Red, ALREADY_IN_PARTY);
				break;
			}

			if (player_to_invite->IsRaidGrouped()) {
				MessageString(Chat::White, ALREADY_IN_RAID, player_to_invite->GetCleanName()); //must invite members not in raid...
				return;
			}

			Bot::ProcessRaidInvite(player_to_invite, this);
			break;

		} else {
			Client* player_to_invite = entity_list.GetClientByName(raid_command_packet->player_name);
			if (!player_to_invite) {
				break;
			}

			Group* player_to_invite_group = player_to_invite->GetGroup();

			if (player_to_invite->HasRaid()) {
				Message(Chat::Red, "%s is already in a raid.", player_to_invite->GetName());
				break;
			}

			if (player_to_invite_group && player_to_invite_group->IsGroupMember(this)) {
				MessageString(Chat::Red, ALREADY_IN_PARTY);
				break;
			}
			// Not allowed: Invite a client that is in a group but not the groupleader
			if (player_to_invite_group && !player_to_invite_group->IsLeader(player_to_invite)) {
				Message(Chat::Red, "You can only invite an ungrouped player or group leader to join your raid.");
				break;
			}

			/* Send out invite to the client */
			auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
			RaidGeneral_Struct* raid_command = (RaidGeneral_Struct*)outapp->pBuffer;

			strn0cpy(raid_command->leader_name, raid_command_packet->leader_name, 64);
			strn0cpy(raid_command->player_name, raid_command_packet->player_name, 64);

			raid_command->parameter = 0;
			raid_command->action = 20;

			player_to_invite->QueuePacket(outapp);

			safe_delete(outapp);

			break;
		}
	}

	case RaidCommandAcceptInvite: {
		Client* player_sending_invite = entity_list.GetClientByName(raid_command_packet->player_name);

		// If the accepting client is in a group with a Bot or the invitor is in a group with a Bot, send the invite to Bot:ProcessRaidInvite
		// instead of remaining here.
		Bot* b = nullptr;
		Client* invitee = entity_list.GetClientByName(raid_command_packet->leader_name);
		Client* invitor = entity_list.GetClientByName(raid_command_packet->player_name);

		if (!invitee || !invitor) {
			return;
		}

		Group* g_invitee = invitee->GetGroup();
		Group* g_invitor = invitor->GetGroup();

		if (invitee && invitee->IsRaidGrouped()) {
			invitor->MessageString(Chat::White, ALREADY_IN_RAID, GetName()); //group failed, must invite members not in raid...
			return;
		}

		bool invitor_has_bot = false;
		bool invitee_has_bot = false;

		if (RuleB(Bots, Enabled)) {
			if (g_invitor && g_invitor->IsLeader(invitor)) {
				for (const auto& g_invitor_member : g_invitor->members) {
					if (g_invitor_member && g_invitor_member->IsBot()) {
						invitee_has_bot = true;
					}
				}
			}
			if (g_invitee && g_invitee->IsLeader(invitee)) {
				for (const auto& g_invitee_member : g_invitee->members) {
					if (g_invitee_member && g_invitee_member->IsBot()) {
						invitee_has_bot = true;
						break;
					}
				}
			}
			if (invitor_has_bot || invitee_has_bot) {
				Bot::ProcessRaidInvite(invitee, invitor);  //two clients with one or both having groups with bots
				break;
			}
		}

		if (player_sending_invite) {
			if (IsRaidGrouped()) {
				player_sending_invite->MessageString(Chat::White, ALREADY_IN_RAID, GetName()); //group failed, must invite members not in raid...
				return;
			}
			Raid* raid = entity_list.GetRaidByClient(player_sending_invite);
			if (raid) {
				raid->VerifyRaid();
				Group* group = GetGroup();
				if (group) {
					if (group->GroupCount() + raid->RaidCount() > MAX_RAID_MEMBERS) {
						player_sending_invite->Message(Chat::Red, "Invite failed, group invite would create a raid larger than the maximum number of members allowed.");
						return;
					}
				}
				else {
					if (1 + raid->RaidCount() > MAX_RAID_MEMBERS) {
						player_sending_invite->Message(Chat::Red, "Invite failed, member invite would create a raid larger than the maximum number of members allowed.");
						return;
					}
				}
				if (group) {//add us all
					uint32 free_group_id = raid->GetFreeGroup();
					Client* addClient = nullptr;
					for (int x = 0; x < MAX_GROUP_MEMBERS; x++) {
						if (group->members[x]) {
							Client* c = nullptr;
							if (group->members[x]->IsClient())
								c = group->members[x]->CastToClient();
							else
								continue;

							if (!addClient)
							{
								addClient = c;
								raid->SetGroupLeader(addClient->GetName());
							}

							raid->SendRaidCreate(c);
							if (group->IsLeader(group->members[x]))
								raid->AddMember(c, free_group_id, false, true);
							else
								raid->AddMember(c, free_group_id);
							raid->SendBulkRaid(c);
							raid->SendMakeLeaderPacketTo(raid->leadername, c);
							if (raid->IsLocked()) {
								raid->SendRaidLockTo(c);
							}
							raid->SendAssistTarget(c);
							raid->SendMarkTargets(c);
						}
					}
					group->JoinRaidXTarget(raid);
					group->DisbandGroup(true);
					raid->GroupUpdate(free_group_id);
				}
				else {
					raid->SendRaidCreate(this);
					raid->AddMember(this);
					raid->SendBulkRaid(this);
					raid->SendMakeLeaderPacketTo(raid->leadername, this);
					if (raid->IsLocked()) {
						raid->SendRaidLockTo(this);
					}
					raid->SendAssistTarget(this);
					raid->SendMarkTargets(this);
				}
			}
			else
			{
				auto player_sending_invite_group = player_sending_invite->GetGroup();
				Group* group = GetGroup();

				/* Prevent scenario where player had joined group with Raid invitor before accepting Raid invite */
				if (group && group == player_sending_invite_group) {
					player_sending_invite->MessageString(Chat::Red, INVITE_GROUP_LEADER);
					return;
				}

				if (group) //if our target has a group
				{
					raid = new Raid(player_sending_invite);
					entity_list.AddRaid(raid);
					raid->SetRaidDetails();

					uint32 raid_free_group_id = raid->GetFreeGroup();

					/* If we already have a group then cycle through adding us... */
					if (player_sending_invite_group) {
						Client* client_to_be_leader = nullptr;
						for (const auto& sending_invite_member : player_sending_invite_group->members) {
							if (sending_invite_member) {
								if (!client_to_be_leader) {
									if (sending_invite_member->IsClient()) {
										client_to_be_leader = sending_invite_member->CastToClient();
										raid->SetGroupLeader(client_to_be_leader->GetName());
									}
								}
								if (player_sending_invite_group->IsLeader(sending_invite_member)) {
									Client* c = nullptr;

									if (sending_invite_member->IsClient())
										c = sending_invite_member->CastToClient();
									else
										continue;

									raid->SendRaidCreate(c);
									raid->AddMember(c, raid_free_group_id, true, true, true);
									raid->SendBulkRaid(c);
									raid->SendMakeLeaderPacketTo(raid->leadername, c);
									if (raid->IsLocked()) {
										raid->SendRaidLockTo(c);
									}
									raid->SendAssistTarget(c);
									raid->SendMarkTargets(c);
								}
								else {
									Client* c = nullptr;

									if (sending_invite_member->IsClient())
										c = sending_invite_member->CastToClient();
									else
										continue;

									raid->SendRaidCreate(c);
									raid->AddMember(c, raid_free_group_id);
									raid->SendBulkRaid(c);
									raid->SendMakeLeaderPacketTo(raid->leadername, c);
									if (raid->IsLocked()) {
										raid->SendRaidLockTo(c);
									}
									raid->SendAssistTarget(c);
									raid->SendMarkTargets(c);
								}
							}
						}
						player_sending_invite_group->JoinRaidXTarget(raid, true);
						player_sending_invite_group->DisbandGroup(true);
						raid->GroupUpdate(raid_free_group_id);
						raid_free_group_id = raid->GetFreeGroup();
					}
					else {
						raid->SendRaidCreate(player_sending_invite);
						raid->AddMember(player_sending_invite, RAID_GROUPLESS, true, false, true);
					}

					Client* client_to_add = nullptr;
					/* Add client to an existing group */
					for (int x = 0; x < MAX_GROUP_MEMBERS; x++) {
						if (group->members[x]) {
							if (!client_to_add) {
								if (group->members[x]->IsClient()) {
									client_to_add = group->members[x]->CastToClient();
									raid->SetGroupLeader(client_to_add->GetName());
								}
							}
							if (group->IsLeader(group->members[x])) {
								Client* c = nullptr;

								if (group->members[x]->IsClient())
									c = group->members[x]->CastToClient();
								else
									continue;

								raid->SendRaidCreate(c);
								raid->AddMember(c, raid_free_group_id, false, true);
								raid->SendBulkRaid(c);
								raid->SendMakeLeaderPacketTo(raid->leadername, c);

								if (raid->IsLocked()) {
									raid->SendRaidLockTo(c);
								}
								raid->SendAssistTarget(c);
								raid->SendMarkTargets(c);
							}
							else
							{
								Client* c = nullptr;

								if (group->members[x]->IsClient())
									c = group->members[x]->CastToClient();
								else
									continue;

								raid->SendRaidCreate(c);
								raid->AddMember(c, raid_free_group_id);
								raid->SendBulkRaid(c);
								raid->SendMakeLeaderPacketTo(raid->leadername, c);
								if (raid->IsLocked()) {
									raid->SendRaidLockTo(c);
								}
								raid->SendAssistTarget(c);
								raid->SendMarkTargets(c);
							}
						}
					}
					group->JoinRaidXTarget(raid);
					group->DisbandGroup(true);

					raid->GroupUpdate(raid_free_group_id);
				}
				/* Target does not have a group */
				else {
					if (player_sending_invite_group) {

						raid = new Raid(player_sending_invite);

						entity_list.AddRaid(raid);
						raid->SetRaidDetails();
						Client* addClientig = nullptr;
						for (int x = 0; x < MAX_GROUP_MEMBERS; x++) {
							if (player_sending_invite_group->members[x]) {
								if (!addClientig) {
									if (player_sending_invite_group->members[x]->IsClient()) {
										addClientig = player_sending_invite_group->members[x]->CastToClient();
										raid->SetGroupLeader(addClientig->GetName());
									}
								}
								if (player_sending_invite_group->IsLeader(player_sending_invite_group->members[x])) {
									Client* c = nullptr;

									if (player_sending_invite_group->members[x]->IsClient())
										c = player_sending_invite_group->members[x]->CastToClient();
									else
										continue;

									raid->SendRaidCreate(c);
									raid->AddMember(c, 0, true, true, true);
									raid->SendBulkRaid(c);
									raid->SendMakeLeaderPacketTo(raid->leadername, c);
									if (raid->IsLocked()) {
										raid->SendRaidLockTo(c);
									}
									raid->SendAssistTarget(c);
									raid->SendMarkTargets(c);
								}
								else
								{
									Client* c = nullptr;
									if (player_sending_invite_group->members[x]->IsClient())
										c = player_sending_invite_group->members[x]->CastToClient();
									else
										continue;

									raid->SendRaidCreate(c);
									raid->AddMember(c, 0);
									raid->SendBulkRaid(c);
									raid->SendMakeLeaderPacketTo(raid->leadername, c);
									if (raid->IsLocked()) {
										raid->SendRaidLockTo(c);
									}
									raid->SendAssistTarget(c);
									raid->SendMarkTargets(c);
								}
							}
						}
						raid->SendRaidCreate(this);
						raid->SendBulkRaid(this);
						player_sending_invite_group->JoinRaidXTarget(raid, true);
						raid->AddMember(this);
						player_sending_invite_group->DisbandGroup(true);
						raid->GroupUpdate(0);
						raid->SendMakeLeaderPacketTo(raid->leadername, this);
						if (raid->IsLocked()) {
							raid->SendRaidLockTo(this);
						}
						raid->SendAssistTarget(this);
						raid->SendMarkTargets(this);
					}
					else { // neither has a group
						raid = new Raid(player_sending_invite);
						entity_list.AddRaid(raid);
						raid->SetRaidDetails();
						raid->SendRaidCreate(player_sending_invite);
						raid->SendRaidCreate(this);
						raid->AddMember(player_sending_invite, RAID_GROUPLESS, true, false, true);
						raid->SendBulkRaid(this);
						raid->AddMember(this);
						raid->SendMakeLeaderPacketTo(raid->leadername, this);
						if (raid->IsLocked()) {
							raid->SendRaidLockTo(this);
						}
						raid->SendAssistTarget(this);
						raid->SendMarkTargets(this);
					}
				}
			}
		}
		break;
	}
	case RaidCommandDisband: {
		Raid* raid = entity_list.GetRaidByClient(this);
		Client* c_to_disband = entity_list.GetClientByName(raid_command_packet->leader_name);
		Client* c_doing_disband = entity_list.GetClientByName(raid_command_packet->player_name);

		if (raid) {
			raid->VerifyRaid();
			uint32 group = raid->GetGroup(raid_command_packet->leader_name);

			if (RuleB(Bots, Enabled)) {
				Bot* const b_to_disband = entity_list.GetBotByBotName(raid_command_packet->leader_name);

				//Added to remove all bots if the Bot_Owner is removed from the Raid
				//Does not camp the Bots, just removes from the raid
				if (c_to_disband) {
					uint32 i = raid->GetPlayerIndex(raid_command_packet->leader_name);
					raid->RemoveRaidDelegates(raid_command_packet->leader_name);
					raid->SendRemoveAllRaidXTargets(raid_command_packet->leader_name);
					raid->SetNewRaidLeader(i);
					raid->HandleBotGroupDisband(c_to_disband->CharacterID());
					raid->HandleOfflineBots(c_to_disband->CharacterID());
					raid->RemoveMember(raid_command_packet->leader_name);
					raid->SendGroupDisband(c_to_disband);
					raid->GroupUpdate(group);
					if (!raid->RaidCount()) {
						raid->DisbandRaid();
					}
					break;
				} else if (b_to_disband) {
					uint32 gid = raid->GetGroup(b_to_disband->GetName());

					if (gid < 12 && (raid->IsGroupLeader(b_to_disband->GetName()) || raid->GroupCount(gid) < 2)) {
						uint32 owner_id = b_to_disband->CastToBot()->GetOwner()->CastToClient()->CharacterID();
						raid->RemoveRaidDelegates(raid_command_packet->leader_name);
						raid->UpdateRaidXTargets();
						raid->HandleBotGroupDisband(owner_id, gid);
					} else if (b_to_disband && raid->IsRaidMember(b_to_disband->GetName())) {
						raid->RemoveRaidDelegates(raid_command_packet->leader_name);
						raid->UpdateRaidXTargets();
						Bot::RemoveBotFromRaid(b_to_disband);
					} else if (gid < 12 && raid->GetGroupLeader(gid) && raid->GetGroupLeader(gid)->IsBot()) {
						c_doing_disband->Message(
							Chat::Yellow,
							fmt::format(
								"{} is in a Bot Group.  Please disband {} instead to remove the entire Bot group.",
								raid_command_packet->leader_name,
								raid->GetGroupLeader(gid)->CastToBot()->GetName()
							).c_str()
						);
					}

					break;
				}
			}
			uint32 i = raid->GetPlayerIndex(raid_command_packet->leader_name);
			if (group < 12) {
				if (raid->members[i].is_group_leader) { //assign group leader to someone else
					for (int x = 0; x < MAX_RAID_MEMBERS; x++) {
						if (strlen(raid->members[x].member_name) > 0 && i != x) {
							if (raid->members[x].group_number == group) {
								raid->SetGroupLeader(raid_command_packet->leader_name, false);
								raid->SetGroupLeader(raid->members[x].member_name);
								raid->UpdateGroupAAs(group);
								break;
							}
						}
					}
				}
			}
			raid->SetNewRaidLeader(i);
			raid->RemoveRaidDelegates(raid_command_packet->leader_name);
			raid->UpdateRaidXTargets();
			raid->RemoveMember(raid_command_packet->leader_name);
			Client* c = entity_list.GetClientByName(raid_command_packet->leader_name);
			if (c) {

				raid->SendGroupDisband(c);
			}
			else {
				auto pack =
					new ServerPacket(ServerOP_RaidGroupDisband, sizeof(ServerRaidGeneralAction_Struct));
				ServerRaidGeneralAction_Struct* rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
				rga->rid = GetID();
				rga->zoneid = zone->GetZoneID();
				rga->instance_id = zone->GetInstanceID();
				strn0cpy(rga->playername, raid_command_packet->leader_name, 64);
				worldserver.SendPacket(pack);
				safe_delete(pack);
			}
			raid->GroupUpdate(group);// break
			if (!raid->RaidCount())
				raid->DisbandRaid();
		}
		break;
	}
	case RaidCommandMoveGroup:
	{
		Raid* raid = entity_list.GetRaidByClient(this);
		if (raid) {
			/* Moving to group */
			if (raid_command_packet->parameter < 12) {
				uint8 group_count = raid->GroupCount(raid_command_packet->parameter);

				if (group_count < 6) {
					Client* c = entity_list.GetClientByName(raid_command_packet->leader_name);
					uint32 old_group = raid->GetGroup(raid_command_packet->leader_name);
					if (raid_command_packet->parameter == old_group) //don't rejoin grp if we order to join same group.
						break;

					if (raid->members[raid->GetPlayerIndex(raid_command_packet->leader_name)].is_group_leader) {
						raid->SetGroupLeader(raid_command_packet->leader_name, false);

						/* We were the leader of our old group */
						if (old_group < 12) {
							/* Assign new group leader if we can */
							for (int x = 0; x < MAX_RAID_MEMBERS; x++) {
								if (raid->members[x].group_number == old_group) {
									if (strcmp(raid_command_packet->leader_name, raid->members[x].member_name) != 0 && strlen(raid_command_packet->leader_name) > 0) {
										raid->SetGroupLeader(raid->members[x].member_name);
										raid->UpdateGroupAAs(old_group);

										Client* client_to_update = entity_list.GetClientByName(raid->members[x].member_name);
										if (client_to_update) {
											raid->SendRaidRemove(raid->members[x].member_name, client_to_update);
											raid->SendRaidCreate(client_to_update);
											raid->SendRaidAdd(raid->members[x].member_name, client_to_update);
											raid->SendBulkRaid(client_to_update);
											raid->SendMakeLeaderPacketTo(raid->leadername, client_to_update);
											if (raid->IsLocked()) {
												raid->SendRaidLockTo(client_to_update);
											}
										}
										else {
											auto pack = new ServerPacket(ServerOP_RaidChangeGroup, sizeof(ServerRaidGeneralAction_Struct));
											ServerRaidGeneralAction_Struct* raid_command_packet = (ServerRaidGeneralAction_Struct*)pack->pBuffer;

											raid_command_packet->rid = raid->GetID();
											raid_command_packet->zoneid = zone->GetZoneID();
											raid_command_packet->instance_id = zone->GetInstanceID();
											strn0cpy(raid_command_packet->playername, raid->members[x].member_name, 64);

											worldserver.SendPacket(pack);

											safe_delete(pack);
										}
										break;
									}
								}
							}
						}
					}
					if (group_count == 0) {
						raid->SetGroupLeader(raid_command_packet->leader_name);
						raid->UpdateGroupAAs(raid_command_packet->parameter);
					}

					raid->MoveMember(raid_command_packet->leader_name, raid_command_packet->parameter);
					if (c) {
						raid->SendGroupDisband(c);
					}
					else {
						auto pack = new ServerPacket(ServerOP_RaidGroupDisband, sizeof(ServerRaidGeneralAction_Struct));
						ServerRaidGeneralAction_Struct* raid_command = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
						raid_command->rid = raid->GetID();
						raid_command->zoneid = zone->GetZoneID();
						raid_command->instance_id = zone->GetInstanceID();
						strn0cpy(raid_command->playername, raid_command_packet->leader_name, 64);
						worldserver.SendPacket(pack);
						safe_delete(pack);
					}

					/* Send group update to our new group */
					raid->GroupUpdate(raid_command_packet->parameter);

					/* If our old was a group send update there too */
					if (old_group < 12) {
						raid->GroupUpdate(old_group);
					}
				}
			}
			/* Move player to ungrouped bank */
			else {
				Client* c = entity_list.GetClientByName(raid_command_packet->leader_name);
				uint32 oldgrp = raid->GetGroup(raid_command_packet->leader_name);
				if (raid->members[raid->GetPlayerIndex(raid_command_packet->leader_name)].is_group_leader) {
					raid->SetGroupLeader(raid_command_packet->leader_name, false);
					for (int x = 0; x < MAX_RAID_MEMBERS; x++) {
						if (raid->members[x].group_number == oldgrp && strlen(raid->members[x].member_name) > 0 && strcmp(raid->members[x].member_name, raid_command_packet->leader_name) != 0) {

							raid->SetGroupLeader(raid->members[x].member_name);
							raid->UpdateGroupAAs(oldgrp);

							Client* client_leaving_group = entity_list.GetClientByName(raid->members[x].member_name);
							if (client_leaving_group) {
								raid->SendRaidRemove(raid->members[x].member_name, client_leaving_group);
								raid->SendRaidCreate(client_leaving_group);
								raid->SendMakeLeaderPacketTo(raid->leadername, client_leaving_group);
								raid->SendRaidAdd(raid->members[x].member_name, client_leaving_group);
								raid->SendBulkRaid(client_leaving_group);
								if (raid->IsLocked()) {
									raid->SendRaidLockTo(client_leaving_group);
								}
							}
							else {
								auto pack = new ServerPacket(ServerOP_RaidChangeGroup, sizeof(ServerRaidGeneralAction_Struct));
								ServerRaidGeneralAction_Struct* raid_command = (ServerRaidGeneralAction_Struct*)pack->pBuffer;

								raid_command->rid = raid->GetID();
								strn0cpy(raid_command->playername, raid->members[x].member_name, 64);
								raid_command->zoneid = zone->GetZoneID();
								raid_command->instance_id = zone->GetInstanceID();

								worldserver.SendPacket(pack);
								safe_delete(pack);
							}
							break;
						}
					}
				}
				raid->MoveMember(raid_command_packet->leader_name, RAID_GROUPLESS);
				if (c) {
					raid->SendGroupDisband(c);
				}
				else {
					auto pack = new ServerPacket(ServerOP_RaidGroupDisband, sizeof(ServerRaidGeneralAction_Struct));
					ServerRaidGeneralAction_Struct* raid_command = (ServerRaidGeneralAction_Struct*)pack->pBuffer;

					raid_command->rid = raid->GetID();
					raid_command->zoneid = zone->GetZoneID();
					raid_command->instance_id = zone->GetInstanceID();
					strn0cpy(raid_command->playername, raid_command_packet->leader_name, 64);

					worldserver.SendPacket(pack);

					safe_delete(pack);
				}

				raid->GroupUpdate(oldgrp);
}
		}

		Client* client_moved = entity_list.GetClientByName(raid_command_packet->leader_name);

		if (client_moved && client_moved->GetRaid()) {
			client_moved->GetRaid()->SendHPManaEndPacketsTo(client_moved);
			client_moved->GetRaid()->SendHPManaEndPacketsFrom(client_moved);

			Log(Logs::General, Logs::HPUpdate,
				"Client::Handle_OP_RaidCommand :: %s sending and recieving HP/Mana/End updates",
				client_moved->GetCleanName()
			);
		}

		break;
	}
	case RaidCommandRaidLock:
	{
		Raid* raid = entity_list.GetRaidByClient(this);
		if (raid) {
			if (!raid->IsLocked())
				raid->LockRaid(true);
			else
				raid->SendRaidLockTo(this);
		}
		break;
	}
	case RaidCommandRaidUnlock:
	{
		Raid* raid = entity_list.GetRaidByClient(this);
		if (raid)
		{
			if (raid->IsLocked())
				raid->LockRaid(false);
			else
				raid->SendRaidUnlockTo(this);
		}
		break;
	}
	case RaidCommandLootType2:
	case RaidCommandLootType:
	{
		Raid* raid = entity_list.GetRaidByClient(this);
		if (raid) {
			Message(Chat::Yellow, "Loot type changed to: %d.", raid_command_packet->parameter);
			raid->ChangeLootType(raid_command_packet->parameter);
		}
		break;
	}

	case RaidCommandAddLooter2:
	case RaidCommandAddLooter:
	{
		Raid* raid = entity_list.GetRaidByClient(this);
		if (raid) {
			Message(Chat::Yellow, "Adding %s as a raid looter.", raid_command_packet->leader_name);
			raid->AddRaidLooter(raid_command_packet->leader_name);
		}
		break;
	}

	case RaidCommandRemoveLooter2:
	case RaidCommandRemoveLooter:
	{
		Raid* raid = entity_list.GetRaidByClient(this);
		if (raid) {
			Message(Chat::Yellow, "Removing %s as a raid looter.", raid_command_packet->leader_name);
			raid->RemoveRaidLooter(raid_command_packet->leader_name);
		}
		break;
	}

	case RaidCommandMakeLeader:
	{
		Raid* raid = entity_list.GetRaidByClient(this);
		if (raid) {
			if (strcmp(raid->leadername, GetName()) == 0) {
				raid->SetRaidLeader(GetName(), raid_command_packet->leader_name);
				raid->UpdateRaidAAs();
				raid->SendAllRaidLeadershipAA();
			}
		}
		break;
	}

	case RaidCommandSetMotd:
	{
		Raid* raid = entity_list.GetRaidByClient(this);
		if (!raid)
			break;
		// we don't use the RaidGeneral here!
		RaidMOTD_Struct* motd = (RaidMOTD_Struct*)app->pBuffer;
		raid->SetRaidMOTD(std::string(motd->motd));
		raid->SaveRaidMOTD();
		raid->SendRaidMOTDToWorld();
		break;
	}
	case RaidCommandSetNote:
	{

		Raid* raid = entity_list.GetRaidByClient(this);
		if (!raid) {
			break;
		}
		RaidNote_Struct* note = (RaidNote_Struct*)app->pBuffer;
		raid->SaveRaidNote(raid_command_packet->leader_name, note->note);
		raid->SendRaidNotesToWorld();
		break;
	}
	default: {
		Message(Chat::Red, "Raid command (%d) NYI", raid_command_packet->action);
		break;
	}
	}
}



void Client::Handle_OP_RandomReq(const EQApplicationPacket *app)
{
	if (app->size != sizeof(RandomReq_Struct)) {
		LogError("Wrong size: OP_RandomReq, size=[{}], expected [{}]", app->size, sizeof(RandomReq_Struct));
		return;
	}
	const RandomReq_Struct* rndq = (const RandomReq_Struct*)app->pBuffer;
	uint32 randLow = rndq->low > rndq->high ? rndq->high : rndq->low;
	uint32 randHigh = rndq->low > rndq->high ? rndq->low : rndq->high;
	uint32 randResult;

	if (randLow == 0 && randHigh == 0)
	{	// defaults
		randLow = 0;
		randHigh = 100;
	}
	randResult = zone->random.Int(randLow, randHigh);

	auto outapp = new EQApplicationPacket(OP_RandomReply, sizeof(RandomReply_Struct));
	RandomReply_Struct* rr = (RandomReply_Struct*)outapp->pBuffer;
	rr->low = randLow;
	rr->high = randHigh;
	rr->result = randResult;
	strcpy(rr->name, GetName());
	entity_list.QueueCloseClients(this, outapp, false, 400);
	safe_delete(outapp);
	return;
}

void Client::Handle_OP_ReadBook(const EQApplicationPacket *app)
{
	if (app->size != sizeof(BookRequest_Struct)) {
		LogError("Wrong size: OP_ReadBook, size=[{}], expected [{}]", app->size, sizeof(BookRequest_Struct));
		return;
	}

	auto b = (BookRequest_Struct*) app->pBuffer;
	ReadBook(b);

	if (ClientVersion() >= EQ::versions::ClientVersion::SoF) {
		EQApplicationPacket end_of_book(OP_FinishWindow, 0);
		QueuePacket(&end_of_book);
	}
}

void Client::Handle_OP_RecipeAutoCombine(const EQApplicationPacket *app)
{
	if (app->size != sizeof(RecipeAutoCombine_Struct)) {
		LogError("Invalid size for RecipeAutoCombine_Struct: Expected: [{}], Got: [{}]",
			sizeof(RecipeAutoCombine_Struct), app->size);
		return;
	}

	RecipeAutoCombine_Struct* rac = (RecipeAutoCombine_Struct*)app->pBuffer;

	Object::HandleAutoCombine(this, rac);
	return;
}

void Client::Handle_OP_RecipeDetails(const EQApplicationPacket *app)
{
	if (app->size < sizeof(uint32)) {
		LogError("Invalid size for RecipeDetails Request: Expected: [{}], Got: [{}]",
			sizeof(uint32), app->size);
		return;
	}
	uint32 *recipe_id = (uint32*)app->pBuffer;

	SendTradeskillDetails(*recipe_id);

	return;
}

void Client::Handle_OP_RecipesFavorite(const EQApplicationPacket *app)
{
	if (app->size != sizeof(TradeskillFavorites_Struct)) {
		LogError("Invalid size for TradeskillFavorites_Struct: Expected: [{}], Got: [{}]",
				 sizeof(TradeskillFavorites_Struct), app->size);
		return;
	}

	TradeskillFavorites_Struct* tsf = (TradeskillFavorites_Struct*)app->pBuffer;

	LogDebug("Requested Favorites for: [{}] - [{}]\n", tsf->object_type, tsf->some_id);

	// results show that object_type is combiner type
	// some_id = 0 if world combiner, item number otherwise

	// make where clause segment for container(s)
	std::string containers;
	uint32 combineObjectSlots;
	if (tsf->some_id == 0) {
		containers += StringFormat(" = %u ", tsf->object_type); // world combiner so no item number
		combineObjectSlots = 10;
	}
	else {
		containers += StringFormat(" in (%u, %u) ", tsf->object_type, tsf->some_id); // container in inventory
		auto item = database.GetItem(tsf->some_id);
		if (!item)
		{
			LogError("Invalid container ID: [{}]. GetItem returned null. Defaulting to BagSlots = 10.\n", tsf->some_id);
			combineObjectSlots = 10;
		}
		else
		{
			combineObjectSlots = item->BagSlots;
		}
	}

	std::string favoriteIDs; //gotta be big enough for 500 IDs
	bool first = true;
	//Assumes item IDs are <10 characters long
	for (uint16 favoriteIndex = 0; favoriteIndex < 500; ++favoriteIndex) {
		if (tsf->favorite_recipes[favoriteIndex] == 0)
			continue;

		if (first) {
			favoriteIDs += StringFormat("%u", tsf->favorite_recipes[favoriteIndex]);
			first = false;
		}
		else
			favoriteIDs += StringFormat(",%u", tsf->favorite_recipes[favoriteIndex]);
	}

	if (first)	//no favorites....
		return;

	// TODO: Clean this up
	const std::string query = StringFormat(
		SQL (
			SELECT
			tr.id,
			tr.name,
			tr.trivial,
			SUM(tre.componentcount),
			tr.tradeskill,
			tr.must_learn
				FROM
				tradeskill_recipe AS tr
				LEFT JOIN tradeskill_recipe_entries AS tre ON tr.id = tre.recipe_id
				WHERE
				tr.enabled <> 0
				AND tr.id IN (%s)
				AND tr.must_learn & 0x20 <> 0x20
				AND (
			(
				tr.must_learn & 0x3 <> 0
			)
				OR (tr.must_learn & 0x3 = 0)
			)
				%s
				GROUP BY
				tr.id
				HAVING
				sum(
				if(
				tre.item_id %s
				AND tre.iscontainer > 0,
				1,
				0
			)
			) > 0
				AND SUM(tre.componentcount) <= %u

				LIMIT
				100
		),
		favoriteIDs.c_str(),
		ContentFilterCriteria::apply().c_str(),
		containers.c_str(),
		combineObjectSlots
	);

	SendTradeskillSearchResults(query, tsf->object_type, tsf->some_id);
}

void Client::Handle_OP_RecipesSearch(const EQApplicationPacket *app)
{
	if (app->size != sizeof(RecipesSearch_Struct)) {
		LogError(
			"Invalid size for RecipesSearch_Struct: Expected: [{}], Got: [{}]",
			sizeof(RecipesSearch_Struct),
			app->size
		);

		return;
	}

	auto* p_recipes_search_struct = (RecipesSearch_Struct*)app->pBuffer;
	p_recipes_search_struct->query[55] = '\0';	//just to be sure.

	LogTradeskills(
		"Requested search recipes for object_type [{}] some_id [{}]",
		p_recipes_search_struct->object_type,
		p_recipes_search_struct->some_id
	);

	char   containers_where_clause[30];
	uint32 combine_object_slots;
	if (p_recipes_search_struct->some_id == 0) {
		// world combiner so no item number
		snprintf(containers_where_clause, 29, "= %u", p_recipes_search_struct->object_type);
		combine_object_slots = 10;
	}
	else {
		// container in inventory
		snprintf(containers_where_clause, 29, "in (%u,%u)", p_recipes_search_struct->object_type, p_recipes_search_struct->some_id);
		auto item = database.GetItem(p_recipes_search_struct->some_id);
		if (!item) {
			LogError(
				"Invalid container ID: [{}]. GetItem returned null. Defaulting to BagSlots = 10.",
				p_recipes_search_struct->some_id
			);
			combine_object_slots = 10;
		}
		else {
			combine_object_slots = item->BagSlots;
		}
	}

	std::string search_clause;
	if (p_recipes_search_struct->query[0] != 0) {
		char buf[120];	//larger than 2X rss->query
		database.DoEscapeString(buf, p_recipes_search_struct->query, strlen(p_recipes_search_struct->query));
		search_clause = StringFormat("name rlike '%s' AND", buf);
	}

	//arbitrary limit of 200 recipes, makes sense to me.
	// TODO: Clean this up
	std::string query = fmt::format(
		SQL(
			SELECT
			tr.id,
			tr.name,
			tr.trivial,
			SUM(tre.componentcount),
			tr.tradeskill,
			tr.must_learn
				FROM
				tradeskill_recipe AS tr
				LEFT JOIN tradeskill_recipe_entries AS tre ON tr.id = tre.recipe_id
				WHERE
				{} tr.trivial >= {}
				AND tr.trivial <= {}
				AND tr.enabled <> 0
				AND tr.must_learn & 0x20 <> 0x20
				AND (
					(
						tr.must_learn & 0x3 <> 0
					)
				OR (tr.must_learn & 0x3 = 0)
			)
			{}
				GROUP BY
				tr.id
				HAVING
				sum(
				if (
				tre.item_id {}
				AND tre.iscontainer > 0,
				1,
				0
			)
			) > 0
				AND SUM(tre.componentcount) <= {}

				LIMIT
				200
		),
		search_clause,
		p_recipes_search_struct->mintrivial,
		p_recipes_search_struct->maxtrivial,
		ContentFilterCriteria::apply(),
		containers_where_clause,
		combine_object_slots
	);

	SendTradeskillSearchResults(query, p_recipes_search_struct->object_type, p_recipes_search_struct->some_id);
}

void Client::Handle_OP_ReloadUI(const EQApplicationPacket *app)
{
	if (IsInAGuild())
	{
		SendGuildRanks();
		SendGuildMembers();
		if (ClientVersion() >= EQ::versions::ClientVersion::RoF) {
			SendGuildRankNames();
		}
	}
	return;
}

void Client::Handle_OP_RemoveBlockedBuffs(const EQApplicationPacket *app)
{
	if (!RuleB(Spells, EnableBlockedBuffs))
		return;

	if (app->size != sizeof(BlockedBuffs_Struct))
	{
		LogDebug("Size mismatch in OP_RemoveBlockedBuffs expected [{}] got [{}]", sizeof(BlockedBuffs_Struct), app->size);

		DumpPacket(app);

		return;
	}
	BlockedBuffs_Struct *bbs = (BlockedBuffs_Struct*)app->pBuffer;

	std::set<uint32> *BlockedBuffs = bbs->Pet ? &PetBlockedBuffs : &PlayerBlockedBuffs;

	std::set<uint32> RemovedBuffs;

	if (bbs->Count > 0)
	{
		std::set<uint32>::iterator Iterator;

		auto outapp = new EQApplicationPacket(OP_RemoveBlockedBuffs, sizeof(BlockedBuffs_Struct));

		BlockedBuffs_Struct *obbs = (BlockedBuffs_Struct*)outapp->pBuffer;

		for (unsigned int i = 0; i < BLOCKED_BUFF_COUNT; ++i)
			obbs->SpellID[i] = 0;

		obbs->Pet = bbs->Pet;
		obbs->Initialise = 0;
		obbs->Flags = 0x5a;

		for (unsigned int i = 0; i < bbs->Count; ++i)
		{
			Iterator = BlockedBuffs->find(bbs->SpellID[i]);

			if (Iterator != BlockedBuffs->end())
			{
				RemovedBuffs.insert(bbs->SpellID[i]);

				BlockedBuffs->erase(Iterator);
			}
		}
		obbs->Count = RemovedBuffs.size();

		Iterator = RemovedBuffs.begin();

		unsigned int Element = 0;

		while (Iterator != RemovedBuffs.end())
		{
			obbs->SpellID[Element++] = (*Iterator);
			++Iterator;
		}

		FastQueuePacket(&outapp);
	}
}

void Client::Handle_OP_RemoveTrap(const EQApplicationPacket *app)
{
	if (app->size != 4) {// just an int
		LogDebug("Size mismatch in OP_RemoveTrap expected 4 got [{}]", app->size);
		DumpPacket(app);
		return;
	}

	auto id = app->ReadUInt32(0);
	bool good = false;
	for (int i = 0; i < trap_mgr.count; ++i) {
		if (trap_mgr.auras[i].spawn_id == id) {
			good = true;
			break;
		}
	}
	if (good)
		RemoveAura(id);
	else
		MessageString(Chat::SpellFailure, NOT_YOUR_TRAP); // pretty sure this was red
}

void Client::Handle_OP_Report(const EQApplicationPacket *app)
{
	if (!CanUseReport)
	{
		MessageString(Chat::System, REPORT_ONCE);
		return;
	}

	uint32 size = app->size;
	uint32 current_point = 0;
	std::string reported, reporter;
	std::string current_string;
	int mode = 0;

	while (current_point < size)
	{
		if (mode < 2)
		{
			if (app->pBuffer[current_point] == '|')
			{
				mode++;
			}
			else
			{
				if (mode == 0)
				{
					reported += app->pBuffer[current_point];
				}
				else
				{
					reporter += app->pBuffer[current_point];
				}
			}
			current_point++;
		}
		else
		{
			if (app->pBuffer[current_point] == 0x0a)
			{
				current_string += '\n';
			}
			else if (app->pBuffer[current_point] == 0x00)
			{
				CanUseReport = false;
				database.AddReport(reporter, reported, current_string);
				return;
			}
			else
			{
				current_string += app->pBuffer[current_point];
			}
			current_point++;
		}
	}

	CanUseReport = false;
	database.AddReport(reporter, reported, current_string);
}

void Client::Handle_OP_RequestDuel(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Duel_Struct)) {
		return;
	}

	EQApplicationPacket* outapp = app->Copy();
	Duel_Struct* ds = (Duel_Struct*)outapp->pBuffer;
	if (!ds->duel_initiator || !ds->duel_target) {
		return;
	}

	uint32 duel = ds->duel_initiator;
	ds->duel_initiator = ds->duel_target;
	ds->duel_target = duel;
	Entity* entity = entity_list.GetID(ds->duel_target);

	if (
		GetID() != ds->duel_target &&
		entity->IsClient() &&
		entity->CastToClient()->IsDueling() &&
		entity->CastToClient()->GetDuelTarget()
	) {
		MessageString(Chat::NPCQuestSay, DUEL_CONSIDERING, entity->GetName());
		return;
	}

	if (IsDueling()) {
		MessageString(Chat::NPCQuestSay, DUEL_INPROGRESS);
		return;
	}

	if (
		GetID() != ds->duel_target &&
		entity->IsClient() &&
		!GetDuelTarget() &&
		!IsDueling() &&
		!entity->CastToClient()->IsDueling() &&
		!entity->CastToClient()->GetDuelTarget()
	) {
		SetDuelTarget(ds->duel_target);
		entity->CastToClient()->SetDuelTarget(GetID());
		ds->duel_target = ds->duel_initiator;
		entity->CastToClient()->FastQueuePacket(&outapp);
		entity->CastToClient()->SetDueling(false);
		SetDueling(false);
	} else {
		safe_delete(outapp);
	}

	return;
}

void Client::Handle_OP_RequestTitles(const EQApplicationPacket *app)
{

	auto outapp = title_manager.MakeTitlesPacket(this);

	if (outapp) {
		FastQueuePacket(&outapp);
	}
}

void Client::Handle_OP_RespawnWindow(const EQApplicationPacket *app)
{
	// This opcode is sent by the client when the player choses which bind to return to.
	// The client sends just a 4 byte packet with the selection number in it
	//
	if (app->size != 4)
	{
		LogDebug("Size mismatch in OP_RespawnWindow expected [{}] got [{}]", 4, app->size);
		DumpPacket(app);
		return;
	}
	char *Buffer = (char *)app->pBuffer;

	uint32 Option = VARSTRUCT_DECODE_TYPE(uint32, Buffer);
	HandleRespawnFromHover(Option);
}

void Client::Handle_OP_Rewind(const EQApplicationPacket *app)
{
	if ((rewind_timer.GetRemainingTime() > 1 && rewind_timer.Enabled())) {
		MessageString(Chat::System, REWIND_WAIT);
	}
	else {
		CastToClient()->MovePC(zone->GetZoneID(), zone->GetInstanceID(), m_RewindLocation.x, m_RewindLocation.y, m_RewindLocation.z, 0, 2, Rewind);
		rewind_timer.Start(30000, true);
	}
}

void Client::Handle_OP_RezzAnswer(const EQApplicationPacket *app)
{
	VERIFY_PACKET_LENGTH(OP_RezzAnswer, app, Resurrect_Struct);

	const auto* r = (const Resurrect_Struct*) app->pBuffer;

	LogSpells(
		"[Client::Handle_OP_RezzAnswer] Received OP_RezzAnswer from client. Pendingrezzexp is [{}] action is [{}]",
		PendingRezzXP,
		r->action ? "ACCEPT" : "DECLINE"
	);


	OPRezzAnswer(r->action, r->spellid, r->zone_id, r->instance_id, r->x, r->y, r->z);

	if (r->action == ResurrectionActions::Accept) {
		if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::REZ_ACCEPTED)) {
			auto e = PlayerEvent::ResurrectAcceptEvent{
				.resurrecter_name = r->rezzer_name,
				.spell_name = spells[r->spellid].name,
				.spell_id = r->spellid,
			};
			RecordPlayerEventLog(PlayerEvent::REZ_ACCEPTED, e);
		}

		EQApplicationPacket* outapp = app->Copy();
		// Send the OP_RezzComplete to the world server. This finds it's way to the zone that
		// the rezzed corpse is in to mark the corpse as rezzed.
		outapp->SetOpcode(OP_RezzComplete);
		worldserver.RezzPlayer(outapp, 0, 0, OP_RezzComplete);
		safe_delete(outapp);
	}
}

void Client::Handle_OP_Sacrifice(const EQApplicationPacket *app)
{

	if (app->size != sizeof(Sacrifice_Struct)) {
		LogDebug("Size mismatch in OP_Sacrifice expected [{}] got [{}]", sizeof(Sacrifice_Struct), app->size);
		DumpPacket(app);
		return;
	}
	Sacrifice_Struct *ss = (Sacrifice_Struct*)app->pBuffer;

	if (!PendingSacrifice) {
		LogError("Unexpected OP_Sacrifice reply");
		DumpPacket(app);
		return;
	}

	if (ss->Confirm) {
		Mob *Caster = entity_list.GetMob(sacrifice_caster_id);
		if (Caster) Sacrifice(Caster);
	}
	PendingSacrifice = false;
	sacrifice_caster_id = 0;
}

void Client::Handle_OP_SafeFallSuccess(const EQApplicationPacket *app)	// bit of a misnomer, sent whenever safe fall is used (success of fail)
{
	if (HasSkill(EQ::skills::SkillSafeFall)) //this should only get called if the client has safe fall, but just in case...
		CheckIncreaseSkill(EQ::skills::SkillSafeFall, nullptr); //check for skill up
}

void Client::Handle_OP_SafePoint(const EQApplicationPacket *app)
{
	return;
}

void Client::Handle_OP_Save(const EQApplicationPacket *app)
{
	// The payload is 192 bytes - Not sure what is contained in payload
	Save();
	return;
}

void Client::Handle_OP_SaveOnZoneReq(const EQApplicationPacket *app)
{
	Handle_OP_Save(app);
}

void Client::Handle_OP_SelectTribute(const EQApplicationPacket *app)
{
	LogTribute("Received OP_SelectTribute of length [{}]", app->size);

	//we should enforce being near a real tribute master to change this
	//but im not sure how I wanna do that right now.
	if (app->size != sizeof(SelectTributeReq_Struct))
		LogError("Invalid size on OP_SelectTribute packet");
	else {
		SelectTributeReq_Struct *t = (SelectTributeReq_Struct *)app->pBuffer;
		SendTributeDetails(t->client_id, t->tribute_id);
	}
	return;
}

void Client::Handle_OP_SenseHeading(const EQApplicationPacket *app)
{
	if (!HasSkill(EQ::skills::SkillSenseHeading)) {
		return;
	}

	if (RuleB(Skills, TrainSenseHeading)) {
		CheckIncreaseSkill(EQ::skills::SkillSenseHeading, nullptr, 0);
		return;
	}

	if (parse->PlayerHasQuestSub(EVENT_USE_SKILL)) {
		const auto& export_string = fmt::format(
			"{} {}",
			EQ::skills::SkillSenseHeading,
			GetRawSkill(EQ::skills::SkillSenseHeading)
		);

		parse->EventPlayer(EVENT_USE_SKILL, this, export_string, 0);
	}
}

void Client::Handle_OP_SenseTraps(const EQApplicationPacket *app)
{
	if (!HasSkill(EQ::skills::SkillSenseTraps))
		return;

	if (!p_timers.Expired(&database, pTimerSenseTraps, false)) {
		Message(Chat::Red, "Ability recovery time not yet met.");
		return;
	}

	int reuse = SenseTrapsReuseTime - GetSkillReuseTime(EQ::skills::SkillSenseTraps);

	if (reuse < 1)
		reuse = 1;

	p_timers.Start(pTimerSenseTraps, reuse - 1);

	float trap_curdist = 0;
	Trap* trap = entity_list.FindNearbyTrap(this, 800, trap_curdist);

	CheckIncreaseSkill(EQ::skills::SkillSenseTraps, nullptr);

	if (trap && trap->skill > 0) {
		int uskill = GetSkill(EQ::skills::SkillSenseTraps);
		if ((zone->random.Int(0, 99) + uskill) >= (zone->random.Int(0, 99) + trap->skill*0.75))
		{
			auto diff = trap->m_Position - glm::vec3(GetPosition());

			if (diff.x == 0 && diff.y == 0)
				Message(Chat::Skills, "You sense a trap right under your feet!");
			else if (diff.x > 10 && diff.y > 10)
				Message(Chat::Skills, "You sense a trap to the NorthWest.");
			else if (diff.x < -10 && diff.y > 10)
				Message(Chat::Skills, "You sense a trap to the NorthEast.");
			else if (diff.y > 10)
				Message(Chat::Skills, "You sense a trap to the North.");
			else if (diff.x > 10 && diff.y < -10)
				Message(Chat::Skills, "You sense a trap to the SouthWest.");
			else if (diff.x < -10 && diff.y < -10)
				Message(Chat::Skills, "You sense a trap to the SouthEast.");
			else if (diff.y < -10)
				Message(Chat::Skills, "You sense a trap to the South.");
			else if (diff.x > 10)
				Message(Chat::Skills, "You sense a trap to the West.");
			else
				Message(Chat::Skills, "You sense a trap to the East.");
			trap->detected = true;

			float angle = CalculateHeadingToTarget(trap->m_Position.x, trap->m_Position.y);

			if (angle < 0)
				angle = (256 + angle);

			angle *= 2;
			MovePC(zone->GetZoneID(), zone->GetInstanceID(), GetX(), GetY(), GetZ(), angle);
			return;
		}
	}
	Message(Chat::Skills, "You did not find any traps nearby.");
	return;
}

void Client::Handle_OP_SetGuildMOTD(const EQApplicationPacket *app)
{
	LogGuilds("Received OP_SetGuildMOTD");

	if (app->size != sizeof(GuildMOTD_Struct)) {
		// client calls for a motd on login even if they arent in a guild
		printf("Error: app size of %i != size of GuildMOTD_Struct of %zu\n", app->size, sizeof(GuildMOTD_Struct));
		return;
	}
	if (!IsInAGuild()) {
		Message(Chat::Red, "You are not in a guild!");
		return;
	}
	if (!guild_mgr.CheckPermission(GuildID(), GuildRank(), GUILD_ACTION_CHANGE_THE_MOTD) ||
		(ClientVersion() < EQ::versions::ClientVersion::RoF && GuildRank() > GUILD_OFFICER))
	{
		Message(Chat::Red, "You do not have permissions to edit your guild's MOTD.");
		return;
	}

	GuildMOTD_Struct* gmotd = (GuildMOTD_Struct*)app->pBuffer;

	LogGuilds("Setting MOTD for [{}] ([{}]) to: [{}] - [{}]",
		guild_mgr.GetGuildName(GuildID()), GuildID(), GetName(), gmotd->motd);

	if (!guild_mgr.SetGuildMOTD(GuildID(), gmotd->motd, GetName())) {
		Message(Chat::Red, "Motd update failed.");
	}

	return;
}

void Client::Handle_OP_SetRunMode(const EQApplicationPacket *app)
{
	if (app->size < sizeof(SetRunMode_Struct)) {
		LogError("Received invalid sized OP_SetRunMode: got [{}], expected [{}]", app->size, sizeof(SetRunMode_Struct));
		DumpPacket(app);
		return;
	}
	SetRunMode_Struct* rms = (SetRunMode_Struct*)app->pBuffer;
	if (rms->mode)
		runmode = true;
	else
		runmode = false;
	return;
}

void Client::Handle_OP_SetServerFilter(const EQApplicationPacket *app)
{
	if (app->size != sizeof(SetServerFilter_Struct)) {
		LogError("Received invalid sized OP_SetServerFilter: got [{}], expected [{}]", app->size, sizeof(SetServerFilter_Struct));
		DumpPacket(app);
		return;
	}
	SetServerFilter_Struct* filter = (SetServerFilter_Struct*)app->pBuffer;
	ServerFilter(filter);
	return;
}

void Client::Handle_OP_SetStartCity(const EQApplicationPacket *app)
{
	// if the character has a start city, don't let them use the command
	if (m_pp.binds[4].zone_id != 0 && m_pp.binds[4].zone_id != 189) {
		Message(Chat::Yellow, "Your home city has already been set.", m_pp.binds[4].zone_id, ZoneName(m_pp.binds[4].zone_id));
		return;
	}

	if (app->size < 1) {
		LogError("Wrong size: OP_SetStartCity, size=[{}], expected [{}]", app->size, 1);
		DumpPacket(app);
		return;
	}

	float x = 0.0f, y = 0.0f, z = 0.0f, heading = 0.0f;
	uint32 zone_id = 0;
	uint32 start_city = (uint32)strtol((const char*)app->pBuffer, nullptr, 10);
	std::string query = fmt::format(
		SQL(
			SELECT
			`zone_id`, `bind_id`, `x`, `y`, `z`, `heading`
			FROM
			`start_zones`
			WHERE
			player_class = {}
			AND
			player_deity = {}
			AND
			player_race = {} {}
		),
		m_pp.class_,
		m_pp.deity,
		m_pp.race,
		ContentFilterCriteria::apply().c_str()
	);
	auto results = content_db.QueryDatabase(query);
	if (!results.Success()) {
		LogError("No valid start zones found for /setstartcity");
		return;
	}

	bool valid_city = false;
	for (auto row = results.begin(); row != results.end(); ++row) {
		if (Strings::ToInt(row[1]) != 0)
			zone_id = Strings::ToInt(row[1]);
		else
			zone_id = Strings::ToInt(row[0]);

		if (zone_id != start_city)
			continue;

		valid_city = true;
		x = Strings::ToFloat(row[2]);
		y = Strings::ToFloat(row[3]);
		z = Strings::ToFloat(row[4]);
		heading = Strings::ToFloat(row[5]);
	}

	if (valid_city) {
		Message(Chat::Yellow, "Your home city has been set");
		SetStartZone(start_city, x, y, z, heading);
		return;
	}

	query = fmt::format(
		SQL(
			SELECT
			`zone_id`, `bind_id`
			FROM
			`start_zones`
			WHERE
			player_class = {}
			AND
			player_deity = {}
			AND
			player_race = {}
		),
		m_pp.class_,
		m_pp.deity,
		m_pp.race
	);
	results = content_db.QueryDatabase(query);
	if (!results.Success())
		return;

	Message(Chat::Yellow, "Use \"/setstartcity #\" to choose a home city from the following list:");

	for (auto row = results.begin(); row != results.end(); ++row) {
		if (Strings::ToInt(row[1]) != 0)
			zone_id = Strings::ToInt(row[1]);
		else
			zone_id = Strings::ToInt(row[0]);

		std::string zone_long_name = ZoneLongName(zone_id);
		Message(Chat::Yellow, "%d - %s", zone_id, zone_long_name.c_str());
	}

}

void Client::Handle_OP_SetTitle(const EQApplicationPacket *app)
{
	if (app->size != sizeof(SetTitle_Struct)) {
		LogDebug("Size mismatch in OP_SetTitle expected [{}] got [{}]", sizeof(SetTitle_Struct), app->size);
		DumpPacket(app);
		return;
	}

	auto sts = (SetTitle_Struct *) app->pBuffer;

	if (sts->title_id && !title_manager.HasTitle(this, sts->title_id)) {
		return;
	}

	std::string title = (
		sts->title_id ?
		(
			!sts->is_suffix ?
			title_manager.GetPrefix(sts->title_id) :
			title_manager.GetSuffix(sts->title_id)
		) :
		""
	);

	if (!sts->is_suffix) {
		SetAATitle(title);
	} else {
		SetTitleSuffix(title);
	}
}

void Client::Handle_OP_Shielding(const EQApplicationPacket *app)
{
	/*
		/shield command mechanics
		Warriors get this skill at level 30
		Used by typing /shield while targeting a player
		While active for the duration of 12 seconds baseline. The 'shield target' will take 50 pct less damage and
		the 'shielder' will be hit with the damage taken by the 'shield target' after all applicable mitigiont is calculated,
		the damage on the 'shielder' will be reduced by 25 percent, this reduction can be increased to 50 pct if equiping a shield.
		You receive a 1% increase in mitigation for every 2 AC on the shield.
		Shielder must stay with in a close distance (15 units) to your 'shield target'. If either move out of range, shield ends, no message given.
		Both duration and shield range can be modified by AA.
		Recast is 3 minutes.

		For custom use cases, Mob::ShieldAbility can be used in quests with all parameters being altered. This functional
		is also used for SPA 201 SpellEffect::PetShield, which functions in a simalar manner with pet shielding owner.

		Note: If either the shielder or the shield target die all variables are reset on both.

	*/

	if (app->size != sizeof(Shielding_Struct)) {
		LogError("OP size error: OP_Shielding expected:[{}] got:[{}]", sizeof(Shielding_Struct), app->size);
		return;
	}

	if (GetLevel() < 30) { //Client gives message
		return;
	}

	if (GetClass() != Class::Warrior){
		return;
	}

	if (!RuleB(Combat, EnableWarriorShielding)) {
		Message(Chat::White, "/shield is disabled.");
		return;
	}

	pTimerType timer = pTimerShieldAbility;
	if (!p_timers.Expired(&database, timer, false)) {
		auto remaining_time = p_timers.GetRemainingTime(timer);
		Message(
			Chat::White,
			fmt::format(
				"You can use the ability /shield in {}.",
				Strings::SecondsToTime(remaining_time)
			).c_str()
		);
		return;
	}

	auto shield = (Shielding_Struct*) app->pBuffer;
	if (ShieldAbility(shield->target_id, 15, 12000, 50, 25, true, false)) {
		p_timers.Start(timer, SHIELD_ABILITY_RECAST_TIME);
	}

	return;
}

void Client::Handle_OP_ShopEnd(const EQApplicationPacket *app)
{
	SendMerchantEnd();
}

void Client::Handle_OP_ShopPlayerBuy(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Merchant_Sell_Struct)) {
		LogError("Invalid size on OP_ShopPlayerBuy: Expected [{}], Got [{}]",
			sizeof(Merchant_Sell_Struct), app->size);
		return;
	}

	Merchant_Sell_Struct* mp = (Merchant_Sell_Struct*)app->pBuffer;
#if EQDEBUG >= 5
	LogDebug("[{}], purchase item", GetName());
	DumpPacket(app);
#endif

	int merchantid;
	bool tmpmer_used = false;
	Mob* tmp = entity_list.GetMob(mp->npcid);

	if (
		tmp == 0 ||
		!tmp->IsNPC() ||
		tmp->GetClass() != Class::Merchant ||
		mp->quantity < 1 ||
		DistanceSquared(m_Position, tmp->GetPosition()) > USE_NPC_RANGE2
	) {
		SendMerchantEnd();
		return;
	}

	merchantid = tmp->CastToNPC()->MerchantType;

	uint32 item_id = 0;
	std::list<MerchantList> merlist = zone->merchanttable[merchantid];
	std::list<MerchantList>::const_iterator itr;
	for (itr = merlist.begin(); itr != merlist.end(); ++itr) {
		MerchantList ml = *itr;
		if (GetLevel() < ml.level_required) {
			continue;
		}

		if (mp->itemslot == ml.slot) {
			item_id = ml.item;
			break;
		}
	}
	const EQ::ItemData* item = nullptr;
	uint32 prevcharges = 0;
	if (item_id == 0) { //check to see if its on the temporary table
		std::list<TempMerchantList> tmp_merlist = zone->tmpmerchanttable[tmp->GetNPCTypeID()];
		std::list<TempMerchantList>::const_iterator tmp_itr;
		TempMerchantList ml;
		for (tmp_itr = tmp_merlist.begin(); tmp_itr != tmp_merlist.end(); ++tmp_itr) {
			ml = *tmp_itr;
			if (mp->itemslot == ml.slot) {
				item_id = ml.item;
				tmpmer_used = true;
				prevcharges = ml.charges;
				break;
			}
		}
	}

	item = database.GetItem(item_id);

	if (!item) {
		//error finding item, client didnt get the update packet for whatever reason, roleplay a tad
		MessageString(Chat::White, ALREADY_SOLD);
		entity_list.SendMerchantInventory(tmp, mp->itemslot, true);
		SendMerchantEnd();
		return;
	}

	if (CheckLoreConflict(item)) {
		MessageString(Chat::White, DUPE_LORE_MERCHANT,tmp->GetCleanName(),item->Name);
		return;
	}

	if (tmpmer_used && (mp->quantity > prevcharges || item->MaxCharges > 1)) {
		if (prevcharges > item->MaxCharges && item->MaxCharges > 1) {
			mp->quantity = item->MaxCharges;
		} else {
			mp->quantity = prevcharges;
		}
	}

	// Item's stackable, but the quantity they want to buy exceeds the max stackable quantity.
	if (item->Stackable && mp->quantity > item->StackSize) {
		mp->quantity = item->StackSize;
	}

	auto outapp = new EQApplicationPacket(OP_ShopPlayerBuy, sizeof(Merchant_Sell_Struct));
	Merchant_Sell_Struct* mpo = (Merchant_Sell_Struct*)outapp->pBuffer;
	mpo->quantity = mp->quantity;
	mpo->playerid = mp->playerid;
	mpo->npcid = mp->npcid;
	mpo->itemslot = mp->itemslot;

	int16 freeslotid = INVALID_INDEX;
	int16 charges = 0;
	if (item->Stackable || tmpmer_used) {
		charges = mp->quantity;
	} else if ( item->MaxCharges >= 1) {
		charges = item->MaxCharges;
	}

	EQ::ItemInstance* inst = database.CreateItem(item, charges);

	int single_price = (item->Price * item->SellRate);

	// Don't use SellCostMod if using UseClassicPriceMod
	if (!RuleB(Merchant, UseClassicPriceMod)) {
		single_price *= RuleR(Merchant, SellCostMod);
	}

	if (RuleB(Merchant, UsePriceMod)) {
		single_price *= Client::CalcPriceMod(tmp, false);
	}

	if (item->MaxCharges > 1) {
		mpo->price = single_price;
	} else {
		mpo->price = single_price * mp->quantity;
	}

	if (mpo->price < 0)	{
		MessageString(Chat::White, ALREADY_SOLD);
		safe_delete(outapp);
		safe_delete(inst);
		SendMerchantEnd();
		return;
	}

	// this area needs some work..two inventory insertion check failure points
	// below do not return player's money..is this the intended behavior?

	if (!TakeMoneyFromPP(mpo->price)) {
		auto message = fmt::format(
			"Vendor Cheat attempted to buy qty [{}] of item_id [{}] item_name[{}] that cost [{}] copper but only has platinum [{}] gold [{}] silver [{}] copper [{}]",
			mpo->quantity,
			item->ID,
			item->Name,
			mpo->price,
			m_pp.platinum,
			m_pp.gold,
			m_pp.silver,
			m_pp.copper
		);

		RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});
		safe_delete(outapp);
		safe_delete(inst);
		return;
	}

	bool stacked = TryStacking(inst);

	if (!stacked) {
		freeslotid = m_inv.FindFreeSlot(false, true, item->Size);
	}

	// shouldn't we be reimbursing if these two fail?

	//make sure we are not completely full...
	if (freeslotid == EQ::invslot::slotCursor) {
		if (m_inv.GetItem(EQ::invslot::slotCursor) != nullptr) {
			Message(Chat::Red, "You do not have room for any more items.");
			safe_delete(outapp);
			safe_delete(inst);
			return;
		}
	}

	if (!stacked && freeslotid == INVALID_INDEX) {
		Message(Chat::Red, "You do not have room for any more items.");
		safe_delete(outapp);
		safe_delete(inst);
		return;
	}

	std::string packet;
	if (!stacked && inst) {
		PutItemInInventory(freeslotid, *inst);
		SendItemPacket(freeslotid, inst, ItemPacketTrade);
	} else if (!stacked) {
		LogError("OP_ShopPlayerBuy: item->ItemClass Unknown! Type: [{}]", item->ItemClass);
	}

	QueuePacket(outapp);

	if (inst && tmpmer_used) {
		int32 new_charges = prevcharges - mp->quantity;
		zone->SaveTempItem(merchantid, tmp->GetNPCTypeID(), item_id, new_charges);
		if (new_charges <= 0) {
			auto delitempacket = new EQApplicationPacket(OP_ShopDelItem, sizeof(Merchant_DelItem_Struct));
			Merchant_DelItem_Struct* delitem = (Merchant_DelItem_Struct*)delitempacket->pBuffer;
			delitem->itemslot = mp->itemslot;
			delitem->npcid = mp->npcid;
			delitem->playerid = mp->playerid;
			delitempacket->priority = 6;
			entity_list.QueueClients(tmp, delitempacket); //que for anyone that could be using the merchant so they see the update
			safe_delete(delitempacket);
		} else {
			// Update the charges/quantity in the merchant window
			inst->SetCharges(new_charges);
			inst->SetPrice(single_price);
			inst->SetMerchantSlot(mp->itemslot);
			inst->SetMerchantCount(new_charges);

			SendItemPacket(mp->itemslot, inst, ItemPacketMerchant);
		}
	}

	if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::MERCHANT_PURCHASE)) {
		auto e = PlayerEvent::MerchantPurchaseEvent{
			.npc_id = tmp->GetNPCTypeID(),
			.merchant_name = tmp->GetCleanName(),
			.merchant_type = tmp->CastToNPC()->MerchantType,
			.item_id = item->ID,
			.item_name = item->Name,
			.charges = static_cast<int16>(mpo->quantity),
			.cost = mpo->price,
			.alternate_currency_id = 0,
			.player_money_balance = GetCarriedMoney(),
			.player_currency_balance = 0,
		};

		RecordPlayerEventLog(PlayerEvent::MERCHANT_PURCHASE, e);
	}

	if (parse->PlayerHasQuestSub(EVENT_MERCHANT_BUY)) {
		const auto& export_string = fmt::format(
			"{} {} {} {} {}",
			tmp->GetNPCTypeID(),
			tmp->CastToNPC()->MerchantType,
			item_id,
			mpo->quantity,
			mpo->price
		);

		parse->EventPlayer(EVENT_MERCHANT_BUY, this, export_string, 0);
	}

	CheckItemDiscoverability(item_id);

	safe_delete(inst);
	safe_delete(outapp);
}

void Client::Handle_OP_ShopPlayerSell(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Merchant_Purchase_Struct)) {
		LogError("Invalid size on OP_ShopPlayerSell: Expected [{}], Got [{}]",
			sizeof(Merchant_Purchase_Struct), app->size);
		return;
	}
	RDTSC_Timer t1(true);
	Merchant_Purchase_Struct* mp = (Merchant_Purchase_Struct*)app->pBuffer;

	Mob* vendor = entity_list.GetMob(mp->npcid);

	if (vendor == 0 || !vendor->IsNPC() || vendor->GetClass() != Class::Merchant)
		return;

	//you have to be somewhat close to them to be properly using them
	if (DistanceSquared(m_Position, vendor->GetPosition()) > USE_NPC_RANGE2)
		return;

	uint32 price = 0;
	uint32 itemid = GetItemIDAt(mp->itemslot);
	if (itemid == 0)
		return;
	const EQ::ItemData* item = database.GetItem(itemid);
	EQ::ItemInstance* inst = GetInv().GetItem(mp->itemslot);
	if (!item || !inst) {
		Message(Chat::Red, "You seemed to have misplaced that item..");
		return;
	}
	if (mp->quantity > 1)
	{
		if ((inst->GetCharges() < 0) || (mp->quantity > (uint32)inst->GetCharges()))
			return;
	}

	if (!item->NoDrop) {
		//Message(Chat::Red,"%s tells you, 'LOL NOPE'", vendor->GetName());
		return;
	}

	uint32 cost_quantity = mp->quantity;
	if (inst->IsCharged())
		uint32 cost_quantity = 1;

	uint32 i;

	if (RuleB(Merchant, UsePriceMod)) {
		for (i = 1; i <= cost_quantity; i++) {
			price = (uint32)(item->Price * i) * Client::CalcPriceMod(vendor, true);

			// Don't use SellCostMod if using UseClassicPriceMod
			if (!RuleB(Merchant, UseClassicPriceMod)) {
				price *= RuleR(Merchant, BuyCostMod);
			}

			price += 0.5; // need to round up, because client does it automatically when displaying price

			if (price > 4000000000) {
				cost_quantity = i;
				mp->quantity = i;
				break;
			}
		}
	}
	else {
		for (i = 1; i <= cost_quantity; i++) {
			price = (uint32)((item->Price * i)*(RuleR(Merchant, BuyCostMod)) + 0.5); // need to round up, because client does it automatically when displaying price
			if (price > 4000000000) {
				cost_quantity = i;
				mp->quantity = i;
				break;
			}
		}
	}

	AddMoneyToPP(price);

	if (inst->IsStackable() || inst->IsCharged())
	{
		unsigned int i_quan = inst->GetCharges();
		if (mp->quantity > i_quan || inst->IsCharged())
			mp->quantity = i_quan;
	}
	else
		mp->quantity = 1;

	int charges = mp->quantity;

	if (vendor->GetKeepsSoldItems()) {
		int freeslot = 0;
		if (
			(freeslot = zone->SaveTempItem(
				vendor->CastToNPC()->MerchantType,
				vendor->GetNPCTypeID(),
				itemid,
				charges,
				true
			)
			) > 0) {
			EQ::ItemInstance *inst2 = inst->Clone();

			while (true) {
				if (!inst2) {
					break;
				}

				uint32 price = (item->Price * item->SellRate);

				// Don't use SellCostMod if using UseClassicPriceMod
				if (!RuleB(Merchant, UseClassicPriceMod)) {
					price *= RuleR(Merchant, SellCostMod);
				}

				if (RuleB(Merchant, UsePriceMod)) {
					price *= Client::CalcPriceMod(vendor, false);
				}

				inst2->SetPrice(price);
				inst2->SetMerchantSlot(freeslot);

				uint32 merchant_quantity = zone->GetTempMerchantQuantity(vendor->GetNPCTypeID(), freeslot);

				if (inst2->IsStackable()) {
					inst2->SetCharges(merchant_quantity);
				}

				inst2->SetMerchantCount(merchant_quantity);

				SendItemPacket(freeslot - 1, inst2, ItemPacketMerchant);
				safe_delete(inst2);

				break;
			}
		}
	}

	if (parse->PlayerHasQuestSub(EVENT_MERCHANT_SELL)) {
		const auto& export_string = fmt::format(
			"{} {} {} {} {}",
			vendor->GetNPCTypeID(),
			vendor->CastToNPC()->MerchantType,
			itemid,
			mp->quantity,
			price
		);

		parse->EventPlayer(EVENT_MERCHANT_SELL, this, export_string, 0);
	}

	if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::MERCHANT_SELL)) {
		auto e = PlayerEvent::MerchantSellEvent{
			.npc_id = vendor->GetNPCTypeID(),
			.merchant_name = vendor->GetCleanName(),
			.merchant_type = vendor->CastToNPC()->MerchantType,
			.item_id = item->ID,
			.item_name = item->Name,
			.charges = static_cast<int16>(mp->quantity),
			.cost = price,
			.alternate_currency_id = 0,
			.player_money_balance = GetCarriedMoney(),
			.player_currency_balance = 0,
		};

		RecordPlayerEventLog(PlayerEvent::MERCHANT_SELL, e);
	}

	// Now remove the item from the player, this happens regardless of outcome
	DeleteItemInInventory(
		mp->itemslot,
		(
			!inst->IsStackable() ?
			0 :
			mp->quantity
		)
	);


	//This forces the price to show up correctly for charged items.
	if (inst->IsCharged())
		mp->quantity = 1;

	auto outapp = new EQApplicationPacket(OP_ShopPlayerSell, sizeof(Merchant_Purchase_Struct));
	Merchant_Purchase_Struct* mco = (Merchant_Purchase_Struct*)outapp->pBuffer;
	mco->npcid = vendor->GetID();
	mco->itemslot = mp->itemslot;
	mco->quantity = mp->quantity;
	mco->price = price;
	QueuePacket(outapp);
	safe_delete(outapp);
	SendMoneyUpdate();
	t1.start();
	Save(1);
	t1.stop();
	std::cout << "Save took: " << t1.getDuration() << std::endl;
	return;
}

void Client::Handle_OP_ShopRequest(const EQApplicationPacket *app)
{
	if (app->size != sizeof(MerchantClick_Struct)) {
		LogError("Wrong size: OP_ShopRequest, size=[{}], expected [{}]", app->size, sizeof(MerchantClick_Struct));
		return;
	}

	MerchantClick_Struct *mc = (MerchantClick_Struct *) app->pBuffer;

	// Send back opcode OP_ShopRequest - tells client to open merchant window.
	// EQApplicationPacket* outapp = new EQApplicationPacket(OP_ShopRequest, sizeof(Merchant_Click_Struct));
	// Merchant_Click_Struct* mco=(Merchant_Click_Struct*)outapp->pBuffer;
	int merchant_id     = 0;
	int tabs_to_display = None;
	Mob *tmp            = entity_list.GetMob(mc->npc_id);

	if (tmp == 0 || !tmp->IsNPC() || tmp->GetClass() != Class::Merchant) {
		return;
	}

	// you have to be somewhat close to them to be properly using them
	if (DistanceSquared(m_Position, tmp->GetPosition()) > USE_NPC_RANGE2) {
		return;
	}

	merchant_id = tmp->CastToNPC()->MerchantType;

	if (ClientVersion() == EQ::versions::ClientVersion::RoF2 && tmp->CastToNPC()->GetParcelMerchant()) {
		tabs_to_display = SellBuyParcel;
	}
	else {
		tabs_to_display = SellBuy;
	}

	int action = MerchantActions::Open;
	if (merchant_id == 0) {
		auto outapp = new EQApplicationPacket(OP_ShopRequest, sizeof(MerchantClick_Struct));
		auto mco    = (MerchantClick_Struct *) outapp->pBuffer;
		mco->npc_id      = mc->npc_id;
		mco->player_id   = 0;
		mco->command     = MerchantActions::Open;
		mco->rate        = 1.0;
		mco->tab_display = tabs_to_display;

		QueuePacket(outapp);
		safe_delete(outapp);
		return;
	}

	if (tmp->IsEngaged()) {
		MessageString(Chat::White, MERCHANT_BUSY);
		action = MerchantActions::Close;
	}

	if (GetFeigned() || IsInvisible()) {
		Message(Chat::White, "You cannot use a merchant right now.");
		action = MerchantActions::Close;
	}

	int primaryfaction = tmp->CastToNPC()->GetPrimaryFaction();
	int factionlvl     = GetFactionLevel(
		CharacterID(), tmp->CastToNPC()->GetNPCTypeID(), GetRace(), GetClass(), GetDeity(),
		primaryfaction, tmp
	);
	if (factionlvl >= 7) {
		MerchantRejectMessage(tmp, primaryfaction);
		action = MerchantActions::Close;
	}

	if (tmp->Charmed()) {
		action = MerchantActions::Close;
	}

	if (!tmp->CastToNPC()->IsMerchantOpen()) {
		tmp->SayString(zone->random.Int(MERCHANT_CLOSED_ONE, MERCHANT_CLOSED_THREE));
		action = MerchantActions::Close;
	}

	auto outapp = new EQApplicationPacket(OP_ShopRequest, sizeof(MerchantClick_Struct));
	auto mco    = (MerchantClick_Struct *) outapp->pBuffer;

	mco->npc_id      = mc->npc_id;
	mco->player_id   = 0;
	mco->command     = action; // Merchant command 0x01 = open
	mco->tab_display = tabs_to_display;

	float buy_cost_mod = 1;

	// Only use the BuyCostMod if we're not using the classic function.
	if (!RuleB(Merchant, UseClassicPriceMod)) {
		buy_cost_mod = RuleR(Merchant, BuyCostMod);
	}

	if (RuleB(Merchant, UsePriceMod)) {
		mco->rate = 1 / (buy_cost_mod * Client::CalcPriceMod(tmp, true));
	}
	else {
		mco->rate = 1 / buy_cost_mod;
	}

	outapp->priority = 6;
	QueuePacket(outapp);
	safe_delete(outapp);

	if (action == MerchantActions::Open) {
		BulkSendMerchantInventory(merchant_id, tmp->GetNPCTypeID());
		SetMerchantSessionEntityID(tmp->GetID());

		if ((tabs_to_display & Parcel) == Parcel) {
			SendBulkParcels();
		}
	}

	return;
}

void Client::Handle_OP_Sneak(const EQApplicationPacket *app)
{
	if (!HasSkill(EQ::skills::SkillSneak) && GetSkill(EQ::skills::SkillSneak) == 0) {
		return; //You cannot sneak if you do not have sneak
	}

	if (!p_timers.Expired(&database, pTimerSneak, false)) {
		Message(Chat::Red, "Ability recovery time not yet met.");
		return;
	}
	p_timers.Start(pTimerSneak, SneakReuseTime - 1);

	bool was = sneaking;
	if (sneaking) {
		sneaking = false;
		hidden = false;
		improved_hidden = false;
		auto outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
		SpawnAppearance_Struct* sa_out = (SpawnAppearance_Struct*)outapp->pBuffer;
		sa_out->spawn_id = GetID();
		sa_out->type = 0x03;
		sa_out->parameter = 0;
		entity_list.QueueClients(this, outapp, true);
		safe_delete(outapp);
	} else {
		CheckIncreaseSkill(EQ::skills::SkillSneak, nullptr, 5);
	}

	float hidechance = ((GetSkill(EQ::skills::SkillSneak) / 300.0f) + .25) * 100;

	if (RuleB(Character, SneakAlwaysSucceedOver100)) {
		hidechance = std::max(10, (int)GetSkill(EQ::skills::SkillSneak));
	}

	float random = zone->random.Real(0, 99);

	if (!was && random < hidechance) {
		sneaking = true;
	}
	auto outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
	SpawnAppearance_Struct* sa_out = (SpawnAppearance_Struct*)outapp->pBuffer;
	sa_out->spawn_id = GetID();
	sa_out->type = 0x0F;
	sa_out->parameter = sneaking;
	QueuePacket(outapp);
	safe_delete(outapp);
	if (GetClass() == Class::Rogue) {
		outapp = new EQApplicationPacket(OP_SimpleMessage, 12);
		SimpleMessage_Struct *msg = (SimpleMessage_Struct *)outapp->pBuffer;
		msg->color = 0x010E;
		if (sneaking) {
			msg->string_id = 347;
		}
		else {
			msg->string_id = 348;
		}
		FastQueuePacket(&outapp);
	}
	return;
}

void Client::Handle_OP_SpawnAppearance(const EQApplicationPacket *app)
{
	if (app->size != sizeof(SpawnAppearance_Struct)) {
		std::cout << "Wrong size on OP_SpawnAppearance. Got: " << app->size << ", Expected: " << sizeof(SpawnAppearance_Struct) << std::endl;
		return;
	}
	SpawnAppearance_Struct* sa = (SpawnAppearance_Struct*)app->pBuffer;

	cheat_manager.ProcessSpawnApperance(sa->spawn_id, sa->type, sa->parameter);

	if (sa->spawn_id != GetID())
		return;

	if (sa->type == AppearanceType::Invisibility) {
		if (sa->parameter != 0)
		{
			if (!HasSkill(EQ::skills::SkillHide) && GetSkill(EQ::skills::SkillHide) == 0)
			{
				if (ClientVersion() < EQ::versions::ClientVersion::SoF)
				{
					auto message = fmt::format("Player sent OP_SpawnAppearance with AppearanceType::Invisibility [{}]", sa->parameter);
					RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});
				}
			}
			return;
		}
		ZeroInvisibleVars(InvisType::T_INVISIBLE);
		hidden = false;
		improved_hidden = false;
		entity_list.QueueClients(this, app, true);
		return;
	}
	else if (sa->type == AppearanceType::Animation) {
		if (IsAIControlled())
			return;

		if (sa->parameter == Animation::Standing) {
			SetAppearance(eaStanding);
			playeraction = 0;
			SetFeigned(false);
			BindWound(this, false, true);
			camp_timer.Disable();
			bot_camp_timer.Disable();
		}
		else if (sa->parameter == Animation::Sitting) {
			SetAppearance(eaSitting);
			playeraction = 1;
			if (!UseBardSpellLogic())
				InterruptSpell();
			SetFeigned(false);
			BindWound(this, false, true);
			tmSitting = Timer::GetCurrentTime();
			BuffFadeBySitModifier();
		}
		else if (sa->parameter == Animation::Crouching) {
			if (!UseBardSpellLogic())
				InterruptSpell();
			SetAppearance(eaCrouching);
			playeraction = 2;
			SetFeigned(false);
		}
		else if (sa->parameter == Animation::Lying) { // feign death too
			SetAppearance(eaDead);
			playeraction = 3;
			InterruptSpell();
		}
		else if (sa->parameter == Animation::Looting) {
			SetAppearance(eaLooting);
			playeraction = 4;
			SetFeigned(false);
		}

		else {
			LogError("Client [{}] :: unknown appearance [{}]", name, (int)sa->parameter);
			return;
		}

		entity_list.QueueClients(this, app, true);
	}
	else if (sa->type == AppearanceType::Anonymous) {
		if (!anon_toggle_timer.Check()) {
			return;
		}

		// For Anon/Roleplay
		if (sa->parameter == 1) { // Anon
			m_pp.anon = 1;
		}
		else if ((sa->parameter == 2) || (sa->parameter == 3)) { // This is Roleplay, or anon+rp
			m_pp.anon = 2;
		}
		else if (sa->parameter == 0) { // This is Non-Anon
			m_pp.anon = 0;
		}
		else {
			LogError("Client [{}] :: unknown Anon/Roleplay Switch [{}]", name, (int)sa->parameter);
			return;
		}
		entity_list.QueueClients(this, app, true);
		UpdateWho();
	}
	else if ((sa->type == AppearanceType::Health) && (dead == 0)) {
		return;
	}
	else if (sa->type == AppearanceType::AFK) {
		if (afk_toggle_timer.Check()) {
			m_is_afk = (sa->parameter == 1);
			m_is_manual_afk = (sa->parameter == 1);
			ResetAFKTimer();
			entity_list.QueueClients(this, app, true);
		}
	}
	else if (sa->type == AppearanceType::Split) {
		m_pp.autosplit = (sa->parameter == 1);
	}
	else if (sa->type == AppearanceType::Sneak) {
		if (sneaking == 0)
			return;

		if (sa->parameter != 0)
		{
			if (!HasSkill(EQ::skills::SkillSneak))
			{
				auto message = fmt::format("Player sent OP_SpawnAppearance with AppearanceType::Sneak [{}]", sa->parameter);
				RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});
			}
			return;
		}
		sneaking = 0;
		entity_list.QueueClients(this, app, true);
	}
	else if (sa->type == AppearanceType::Size)
	{
		auto message = fmt::format("Player sent OP_SpawnAppearance with AppearanceType::Size [{}]", sa->parameter);
		RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});
	}
	else if (sa->type == AppearanceType::Light)	// client emitting light (lightstone, shiny shield)
	{
		//don't do anything with this
	}
	else if (sa->type == AppearanceType::FlyMode)
	{
		// don't do anything with this, we tell the client when it's
		// levitating, not the other way around
	}
	else if (sa->type == AppearanceType::ShowHelm)
	{
		if (helm_toggle_timer.Check()) {
			m_pp.showhelm = (sa->parameter == 1);
			entity_list.QueueClients(this, app, true);
		}
	}
	else if (sa->type == AppearanceType::GroupAutoConsent)
	{
		m_pp.groupAutoconsent = (sa->parameter == 1);
		ConsentCorpses("Group", (sa->parameter != 1));
	}
	else if (sa->type == AppearanceType::RaidAutoConsent)
	{
		m_pp.raidAutoconsent = (sa->parameter == 1);
		ConsentCorpses("Raid", (sa->parameter != 1));
	}
	else if (sa->type == AppearanceType::GuildAutoConsent)
	{
		m_pp.guildAutoconsent = (sa->parameter == 1);
		ConsentCorpses("Guild", (sa->parameter != 1));
	}
	else {
		std::cout << "Unknown SpawnAppearance type: 0x" << std::hex << std::setw(4) << std::setfill('0') << sa->type << std::dec
			<< " value: 0x" << std::hex << std::setw(8) << std::setfill('0') << sa->parameter << std::dec << std::endl;
	}
	return;
}

void Client::Handle_OP_Split(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Split_Struct)) {
		LogError("Wrong size: OP_Split, size=[{}], expected [{}]", app->size, sizeof(Split_Struct));
		return;
	}
	// The client removes the money on its own, but we have to
	// update our state anyway, and make sure they had enough to begin
	// with.
	Split_Struct *split = (Split_Struct *)app->pBuffer;
	//Per the note above, Im not exactly sure what to do on error
	//to notify the client of the error...

	Group *group = nullptr;
	Raid *raid = nullptr;

	if (IsRaidGrouped()) {
		raid = GetRaid();
	} else if (IsGrouped()) {
		group = GetGroup();
	}

	// is there an actual error message for this?
	if (raid == nullptr && group == nullptr) {
		MessageString(Chat::Red, SPLIT_NO_GROUP);
		return;
	}

	if (!TakeMoneyFromPP(static_cast<uint64>(split->copper) +
		10 * static_cast<uint64>(split->silver) +
		100 * static_cast<uint64>(split->gold) +
		1000 * static_cast<uint64>(split->platinum))) {
		MessageString(Chat::Red, SPLIT_FAIL);
		return;
	}

	if (raid) {
		raid->SplitMoney(raid->GetGroup(this), split->copper, split->silver, split->gold, split->platinum);
	} else if (group) {
		group->SplitMoney(split->copper, split->silver, split->gold, split->platinum, this, true);
	}

	return;

}

void Client::Handle_OP_Surname(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Surname_Struct))
	{
		LogDebug("Size mismatch in Surname expected [{}] got [{}]", sizeof(Surname_Struct), app->size);
		return;
	}

	if (!p_timers.Expired(&database, pTimerSurnameChange, false) && !GetGM())
	{
		Message(Chat::Yellow, "You may only change surnames once every 7 days, your /surname is currently on cooldown.");
		return;
	}

	if (GetLevel() < 20)
	{
		MessageString(Chat::Yellow, SURNAME_LEVEL);
		return;
	}

	Surname_Struct* surname = (Surname_Struct*)app->pBuffer;

	char *c = nullptr;
	bool first = true;
	for (c = surname->lastname; *c; c++)
	{
		if (first)
		{
			*c = toupper(*c);
			first = false;
		}
		else
		{
			*c = tolower(*c);
		}
	}

	if (strlen(surname->lastname) >= 20) {
		MessageString(Chat::Yellow, SURNAME_TOO_LONG);
		return;
	}

	if (!database.CheckNameFilter(surname->lastname, true))
	{
		MessageString(Chat::Yellow, SURNAME_REJECTED);
		return;
	}

	ChangeLastName(surname->lastname);
	p_timers.Start(pTimerSurnameChange, 604800);

	EQApplicationPacket* outapp = app->Copy();
	outapp = app->Copy();
	surname = (Surname_Struct*)outapp->pBuffer;
	surname->unknown0064 = 1;
	FastQueuePacket(&outapp);
	return;
}

void Client::Handle_OP_SwapSpell(const EQApplicationPacket *app)
{

	if (app->size != sizeof(SwapSpell_Struct)) {
		std::cout << "Wrong size on OP_SwapSpell. Got: " << app->size << ", Expected: " << sizeof(SwapSpell_Struct) << std::endl;
		return;
	}
	const SwapSpell_Struct* swapspell = (const SwapSpell_Struct*)app->pBuffer;
	int swapspelltemp;

	const auto sbs = EQ::spells::DynamicLookup(ClientVersion(), GetGM())->SpellbookSize;
	if (swapspell->from_slot < 0 || swapspell->from_slot >= sbs)
		return;
	if (swapspell->to_slot < 0 || swapspell->to_slot >= sbs)
		return;

	swapspelltemp = m_pp.spell_book[swapspell->from_slot];
	if (swapspelltemp < 0) {
		return;
	}
	m_pp.spell_book[swapspell->from_slot] = m_pp.spell_book[swapspell->to_slot];
	m_pp.spell_book[swapspell->to_slot] = swapspelltemp;

	/* Save Spell Swaps */
	if (!database.SaveCharacterSpell(CharacterID(), m_pp.spell_book[swapspell->from_slot], swapspell->from_slot)) {
		database.DeleteCharacterSpell(CharacterID(), swapspell->from_slot);
	}
	if (!database.SaveCharacterSpell(CharacterID(), swapspelltemp, swapspell->to_slot)) {
		database.DeleteCharacterSpell(CharacterID(), swapspell->to_slot);
	}

	QueuePacket(app);
	return;
}

void Client::Handle_OP_TargetCommand(const EQApplicationPacket *app)
{
	if (app->size != sizeof(ClientTarget_Struct)) {
		LogError("OP size error: OP_TargetCommand expected:[{}] got:[{}]", sizeof(ClientTarget_Struct), app->size);
		return;
	}

	// Locate and cache new target
	ClientTarget_Struct* ct = (ClientTarget_Struct*)app->pBuffer;

	bool can_target=false;
	Mob *nt = entity_list.GetMob(ct->new_target);

	if (nt) {
		if (GetGM() || (!nt->IsInvisible(this) && (DistanceSquared(m_Position, nt->GetPosition()) <= TARGETING_RANGE*TARGETING_RANGE))) {
			if (nt->GetBodyType() == BodyType::NoTarget2 ||
				nt->GetBodyType() == BodyType::Special ||
				nt->GetBodyType() == BodyType::NoTarget) {
				can_target = false;
			}
			else {
				can_target = true;
			}
		}
	}

	if (can_target) {
		QueuePacket(app);
	}
	else {
		MessageString(Chat::Red, DONT_SEE_TARGET);
		auto outapp = new EQApplicationPacket(OP_TargetReject, sizeof(TargetReject_Struct));
		outapp->pBuffer[0] = 0x2f;
		outapp->pBuffer[1] = 0x01;
		outapp->pBuffer[4] = 0x0d;
		QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void Client::Handle_OP_TargetMouse(const EQApplicationPacket *app)
{
	if (app->size != sizeof(ClientTarget_Struct)) {
		LogError("OP size error: OP_TargetMouse expected:[{}] got:[{}]", sizeof(ClientTarget_Struct), app->size);
		return;
	}

	if (GetTarget())
	{
		GetTarget()->IsTargeted(-1);
	}

	// Locate and cache new target
	ClientTarget_Struct* ct = (ClientTarget_Struct*)app->pBuffer;
	pClientSideTarget = ct->new_target;
	if (!IsAIControlled())
	{
		Mob *nt = entity_list.GetMob(ct->new_target);
		if (nt)
		{
			SetTarget(nt);
			bool inspect_buffs = false;
			// rank 1 gives you ability to see NPC buffs in target window (SoD+)
			if (nt->IsNPC()) {
				if (IsRaidGrouped()) {
					Raid *raid = GetRaid();
					if (raid) {
						uint32 gid = raid->GetGroup(this);
						if (gid < 12 && raid->GroupCount(gid) > 2)
							inspect_buffs = raid->GetLeadershipAA(groupAAInspectBuffs, gid);
					}
				}
				else {
					Group *group = GetGroup();
					if (group && group->GroupCount() > 2)
						inspect_buffs = group->GetLeadershipAA(groupAAInspectBuffs);
				}
			}
			if (GetGM() || RuleB(Spells, AlwaysSendTargetsBuffs) || nt == this || inspect_buffs || (nt->IsClient() && !nt->CastToClient()->GetPVP()) ||
				(nt->IsPet() && nt->GetOwner() && nt->GetOwner()->IsClient() && !nt->GetOwner()->CastToClient()->GetPVP()) ||
				(nt->IsBot() && nt->GetOwner() && nt->GetOwner()->IsClient() && !nt->GetOwner()->CastToClient()->GetPVP()) || // TODO: bot pets
				(nt->IsMerc() && nt->GetOwner() && nt->GetOwner()->IsClient() && !nt->GetOwner()->CastToClient()->GetPVP()))
			{
				nt->SendBuffsToClient(this);
			}
		}
		else
		{
			SetTarget(nullptr);
			SetHoTT(0);
			UpdateXTargetType(TargetsTarget, nullptr);

			Group *g = GetGroup();

			if (g && g->HasRole(this, RoleAssist))
				g->SetGroupAssistTarget(0);

			if (g && g->HasRole(this, RoleTank))
				g->SetGroupTankTarget(0);

			if (g && g->HasRole(this, RolePuller))
				g->SetGroupPullerTarget(0);

			return;
		}
	}
	else
	{
		SetTarget(nullptr);
		SetHoTT(0);
		UpdateXTargetType(TargetsTarget, nullptr);
		return;
	}

	// HoTT
	if (GetTarget() && GetTarget()->GetTarget())
	{
		SetHoTT(GetTarget()->GetTarget()->GetID());
		UpdateXTargetType(TargetsTarget, GetTarget()->GetTarget());
	}
	else
	{
		SetHoTT(0);
		UpdateXTargetType(TargetsTarget, nullptr);
	}

	Group *g = GetGroup();

	if (g && g->HasRole(this, RoleAssist))
		g->SetGroupAssistTarget(GetTarget());

	if (g && g->HasRole(this, RoleTank))
		g->SetGroupTankTarget(GetTarget());

	if (g && g->HasRole(this, RolePuller))
		g->SetGroupPullerTarget(GetTarget());

	if (GetTarget())
	{
		if (GetGM())
		{
			GetTarget()->IsTargeted(1);
			return;
		}
		else if (RuleB(Character, AllowMQTarget))
		{
			GetTarget()->IsTargeted(1);
			return;
		}
		else if (cheat_manager.GetExemptStatus(Assist)) {
			GetTarget()->IsTargeted(1);
			cheat_manager.SetExemptStatus(Assist, false);
			return;
		}
		else if (GetTarget()->IsClient())
		{
			//make sure this client is in our raid/group
			GetTarget()->IsTargeted(1);
			return;
		}
		else if (GetTarget()->GetBodyType() == BodyType::NoTarget2 || GetTarget()->GetBodyType() == BodyType::Special
			|| GetTarget()->GetBodyType() == BodyType::NoTarget)
		{
			auto message = fmt::format(
				"[{}] attempting to target something untargetable [{}] bodytype [{}]",
				GetName(),
				GetTarget()->GetName(),
				(int) GetTarget()->GetBodyType()
			);
			RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});

			SetTarget((Mob*)nullptr);
			return;
		}
		else if (cheat_manager.GetExemptStatus(Port)) {
			GetTarget()->IsTargeted(1);
			return;
		}
		else if (cheat_manager.GetExemptStatus(Sense)) {
			GetTarget()->IsTargeted(1);
			cheat_manager.SetExemptStatus(Sense, false);
			return;
		}
		else if (IsXTarget(GetTarget()))
		{
			GetTarget()->IsTargeted(1);
			return;
		}
		else if (GetTarget()->IsPetOwnerOfClientBot())
		{
			GetTarget()->IsTargeted(1);
			return;
		}
		else if (GetBindSightTarget())
		{
			if (DistanceSquared(GetBindSightTarget()->GetPosition(), GetTarget()->GetPosition()) > (zone->newzone_data.maxclip*zone->newzone_data.maxclip))
			{
				if (DistanceSquared(m_Position, GetTarget()->GetPosition()) > (zone->newzone_data.maxclip*zone->newzone_data.maxclip))
				{
					auto message = fmt::format(
						"[{}] attempting to target something beyond the clip plane of {:.2f} "
						"units, from ({:.2f}, {:.2f}, {:.2f}) to {} ({:.2f}, {:.2f}, "
						"{:.2f})",
						GetName(),
						(zone->newzone_data.maxclip * zone->newzone_data.maxclip), GetX(),
						GetY(), GetZ(), GetTarget()->GetName(), GetTarget()->GetX(),
						GetTarget()->GetY(), GetTarget()->GetZ());

					RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});
					SetTarget(nullptr);
					return;
				}
			}
		}
		else if (DistanceSquared(m_Position, GetTarget()->GetPosition()) > (zone->newzone_data.maxclip*zone->newzone_data.maxclip))
		{
			auto message = fmt::format(
				"{} attempting to target something beyond the clip plane of {:.2f} "
				"units, from ({:.2f}, {:.2f}, {:.2f}) to {} ({:.2f}, {:.2f}, {:.2f})",
				GetName(),
				(zone->newzone_data.maxclip * zone->newzone_data.maxclip),
				GetX(),
				GetY(),
				GetZ(),
				GetTarget()->GetName(),
				GetTarget()->GetX(),
				GetTarget()->GetY(),
				GetTarget()->GetZ()
			);

			RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});
			SetTarget(nullptr);
			return;
		}

		GetTarget()->IsTargeted(1);
	}
	return;
}
void Client::Handle_OP_TaskHistoryRequest(const EQApplicationPacket *app)
{

	if (app->size != sizeof(TaskHistoryRequest_Struct)) {
		LogDebug("Size mismatch in OP_TaskHistoryRequest expected [{}] got [{}]", sizeof(TaskHistoryRequest_Struct), app->size);
		DumpPacket(app);
		return;
	}
	TaskHistoryRequest_Struct *ths = (TaskHistoryRequest_Struct*)app->pBuffer;

	if (RuleB(TaskSystem, EnableTaskSystem) && task_state)
		task_state->SendTaskHistory(this, ths->TaskIndex);
}

void Client::Handle_OP_Taunt(const EQApplicationPacket *app)
{
	if (app->size != sizeof(ClientTarget_Struct)) {
		std::cout << "Wrong size on OP_Taunt. Got: " << app->size << ", Expected: " << sizeof(ClientTarget_Struct) << std::endl;
		return;
	}

	if (!HasSkill(EQ::skills::SkillTaunt)) {
		return;
	}

	if (!p_timers.Expired(&database, pTimerTaunt, false)) {
		Message(Chat::Red, "Ability recovery time not yet met.");
		return;
	}
	p_timers.Start(pTimerTaunt, TauntReuseTime - 1);

	if (GetTarget() == nullptr || !GetTarget()->IsNPC())
		return;

	if (!zone->CanDoCombat()) {
		Message(Chat::Red, "You cannot taunt in a no combat zone.");
		return;
	}

	if (DistanceSquared(GetPosition(), GetTarget()->GetPosition()) > (RuleI(Skills, MaximumTauntDistance) * (RuleI(Skills, MaximumTauntDistance)))) {
		MessageString(Chat::TooFarAway, TAUNT_TOO_FAR);
		return;
	}

	Taunt(GetTarget()->CastToNPC(), false);
	return;
}

void Client::Handle_OP_TestBuff(const EQApplicationPacket *app)
{
	if (!RuleB(Character, EnableTestBuff)) {
		return;
	}

	if (parse->PlayerHasQuestSub(EVENT_TEST_BUFF)) {
		parse->EventPlayer(EVENT_TEST_BUFF, this, "", 0);
	}
}

void Client::Handle_OP_TGB(const EQApplicationPacket *app)
{
	OPTGB(app);
	return;
}

void Client::Handle_OP_Track(const EQApplicationPacket *app)
{
	if (!CanThisClassTrack()) {
		return;
	}

	if (GetSkill(EQ::skills::SkillTracking) == 0)
		SetSkill(EQ::skills::SkillTracking, 1);
	else
		CheckIncreaseSkill(EQ::skills::SkillTracking, nullptr, 15);

	if (!entity_list.MakeTrackPacket(this))
		LogError("Unable to generate OP_Track packet requested by client");

	return;
}

void Client::Handle_OP_TrackTarget(const EQApplicationPacket *app)
{
	if (!CanThisClassTrack()) {
		return;
	}

	if (app->size != sizeof(TrackTarget_Struct)) {
		LogError("Invalid size for OP_TrackTarget: Expected: [{}], Got: [{}]", sizeof(TrackTarget_Struct), app->size);
		return;
	}

	auto *t = (TrackTarget_Struct*) app->pBuffer;

	SetTrackingID(t->EntityID);
}

void Client::Handle_OP_TrackUnknown(const EQApplicationPacket *app)
{
	// size 0 send right after OP_Track
	return;
}

void Client::Handle_OP_TradeAcceptClick(const EQApplicationPacket *app)
{
	Mob* with = trade->With();
	trade->state = TradeAccepted;

	if (with && with->IsClient()) {
		//finish trade...
		// Have both accepted?
		Client* other = with->CastToClient();
		other->QueuePacket(app);

		if (other->trade->state == trade->state) {
			other->trade->state = TradeCompleting;
			trade->state = TradeCompleting;

			if (CheckTradeLoreConflict(other) || other->CheckTradeLoreConflict(this)) {
				MessageString(Chat::Red, TRADE_CANCEL_LORE);
				other->MessageString(Chat::Red, TRADE_CANCEL_LORE);
				FinishTrade(this);
				other->FinishTrade(other);
				other->trade->Reset();
				trade->Reset();
			}
			else if (CheckTradeNonDroppable()) {
				MessageString(Chat::Red, TRADE_HAS_BEEN_CANCELLED);
				other->MessageString(Chat::Red, TRADE_HAS_BEEN_CANCELLED);
				FinishTrade(this);
				other->FinishTrade(other);
				other->trade->Reset();
				trade->Reset();
				Message(Chat::Yellow, "Hacking activity detected in trade transaction.");
				// TODO: query (this) as a hacker
			}
			else if (other->CheckTradeNonDroppable()) {
				MessageString(Chat::Red, TRADE_HAS_BEEN_CANCELLED);
				other->MessageString(Chat::Red, TRADE_HAS_BEEN_CANCELLED);
				FinishTrade(this);
				other->FinishTrade(other);
				other->trade->Reset();
				trade->Reset();
				other->Message(Chat::Yellow, "Hacking activity detected in trade transaction.");
				// TODO: query (other) as a hacker
			}
			else {
				other->PlayerTradeEventLog(other->trade, trade);

				FinishTrade(other);
				other->FinishTrade(this);

				other->trade->Reset();
				trade->Reset();
			}
			// All done
			auto outapp = new EQApplicationPacket(OP_FinishTrade, 0);
			other->QueuePacket(outapp);
			FastQueuePacket(&outapp);
		}
	}
	// Trading with a Mob object that is not a Client.
	else if (with) {
		auto outapp = new EQApplicationPacket(OP_FinishTrade, 0);
		QueuePacket(outapp);
		safe_delete(outapp);
		if (with->IsNPC()) {
			FinishTrade(with->CastToNPC());
		}
		// TODO: Log Bot trades
		else if (with->IsBot())
			with->CastToBot()->FinishTrade(this, Bot::BotTradeClientNormal);
		trade->Reset();
	}


	return;
}

void Client::Handle_OP_TradeBusy(const EQApplicationPacket *app)
{
	if (app->size != sizeof(TradeBusy_Struct)) {
		LogError("Wrong size: OP_TradeBusy, size=[{}], expected [{}]", app->size, sizeof(TradeBusy_Struct));
		return;
	}
	// Trade request recipient is cancelling the trade due to being busy
	// Trade requester gets message "I'm busy right now"
	// Send busy message on to trade initiator if client
	TradeBusy_Struct* msg = (TradeBusy_Struct*)app->pBuffer;
	Mob* tradee = entity_list.GetMob(msg->to_mob_id);

	if (tradee && tradee->IsClient()) {
		tradee->CastToClient()->QueuePacket(app);
	}
	return;
}

void Client::Handle_OP_Trader(const EQApplicationPacket *app)
{
	// Bazaar Trader:
	//
	// SoF sends 1 or more unhandled OP_Trader packets of size 96 when a trade has completed.
	// I don't know what they are for (yet), but it doesn't seem to matter that we ignore them.
	auto action = *(uint32 *)app->pBuffer;

	switch (action) {
		case TraderOff: {
			TraderEndTrader();
			LogTrading("End Trader Session");
			break;
		}
		case TraderOn: {
			if (IsBuyer()) {
				TraderEndTrader();
				Message(Chat::Red, "You cannot be a Trader and Buyer at the same time.");
				return;
			}

			TraderStartTrader(app);
			break;
		}
		case PriceUpdate:
		case ItemMove: {
			LogTrading("Trader Price Update");
			TraderPriceUpdate(app);
			break;
		}
		case EndTransaction: {
			auto sis  = (Trader_ShowItems_Struct *) app->pBuffer;
			Client *c = entity_list.GetClientByID(sis->entity_id);
			if (c) {
				c->WithCustomer(0);
				LogTrading("End Transaction");
			}

			break;
		}
		case ListTraderItems: {
			TraderShowItems();
			LogTrading("Show Trader Items");
			break;
		}
		default: {
			LogError("Unknown size for OP_Trader: [{}]\n", app->size);
		}
	}
}

void Client::Handle_OP_TraderBuy(const EQApplicationPacket *app)
{
	// Bazaar Trader:
	//
	// Client has elected to buy an item from a Trader
	//
	auto in     = (TraderBuy_Struct *) app->pBuffer;

	if (RuleB(Bazaar, UseAlternateBazaarSearch) && in->trader_id >= TraderRepository::TRADER_CONVERT_ID) {
		auto trader = TraderRepository::GetTraderByInstanceAndSerialnumber(
			database,
			in->trader_id - TraderRepository::TRADER_CONVERT_ID,
			in->serial_number
		);

		if (!trader.trader_id) {
			LogTrading("Unable to convert trader id for {} and serial number {}.  Trader Buy aborted.",
				in->trader_id - TraderRepository::TRADER_CONVERT_ID,
				in->serial_number
			);
			return;
		}

		in->trader_id = trader.trader_id;
		strn0cpy(in->seller_name, trader.trader_name.c_str(), sizeof(in->seller_name));
	}

	auto trader = entity_list.GetClientByID(in->trader_id);

	switch (in->method) {
		case BazaarByVendor: {
			if (trader) {
				LogTrading("Buy item directly from vendor id <green>[{}] item_id <green>[{}] quantity <green>[{}] "
						   "serial_number <green>[{}]",
						   in->trader_id,
						   in->item_id,
						   in->quantity,
						   in->serial_number
				);
				BuyTraderItem(in, trader, app);
			}
			break;
		}
		case BazaarByParcel: {
			if (!RuleB(Parcel, EnableParcelMerchants) || !RuleB(Bazaar, EnableParcelDelivery)) {
				LogTrading(
					"Bazaar purchase attempt by parcel delivery though 'Parcel:EnableParcelMerchants' or "
					"'Bazaar::EnableParcelDelivery' not enabled."
				);
				Message(
					Chat::Yellow,
					"The bazaar parcel delivey system is not enabled on this server.  Please visit the vendor directly in the Bazaar."
				);
				in->method     = BazaarByParcel;
				in->sub_action = Failed;
				TradeRequestFailed(app);
				return;
			}
			LogTrading("Buy item by parcel delivery <green>[{}] item_id <green>[{}] quantity <green>[{}] "
					   "serial_number <green>[{}]",
					   in->trader_id,
					   in->item_id,
					   in->quantity,
					   in->serial_number
			);
			BuyTraderItemOutsideBazaar(in, app);
			break;
		}
		case BazaarByDirectToInventory: {
			if (!RuleB(Parcel, EnableDirectToInventoryDelivery)) {
				LogTrading("Bazaar purchase attempt by direct inventory delivery though "
						   "'Parcel:EnableDirectToInventoryDelivery' not enabled."
				);
				Message(
					Chat::Yellow,
					"Direct inventory delivey is not enabled on this server.  Please visit the vendor directly."
				);
				in->method     = BazaarByDirectToInventory;
				in->sub_action = Failed;
				TradeRequestFailed(app);
				return;
			}
			trader = entity_list.GetClientByCharID(in->trader_id);
			LogTrading("Buy item by direct inventory delivery <green>[{}] item_id <green>[{}] quantity <green>[{}] "
					   "serial_number <green>[{}]",
					   in->trader_id,
					   in->item_id,
					   in->quantity,
					   in->serial_number
			);
			Message(
				Chat::Yellow,
				"Direct inventory delivery is not yet implemented.  Please visit the vendor directly or purchase via parcel delivery."
			);
			in->method     = BazaarByDirectToInventory;
			in->sub_action = Failed;
			TradeRequestFailed(app);
			break;
		}
	}
}

void Client::Handle_OP_TradeRequest(const EQApplicationPacket *app)
{
	if (app->size != sizeof(TradeRequest_Struct)) {
		LogError("Wrong size: OP_TradeRequest, size=[{}], expected [{}]", app->size, sizeof(TradeRequest_Struct));
		return;
	}
	// Client requesting a trade session from an npc/client
	// Trade session not started until OP_TradeRequestAck is sent

	TradeRequest_Struct* msg = (TradeRequest_Struct*)app->pBuffer;
	Mob* tradee = entity_list.GetMob(msg->to_mob_id);

	// If the tradee is an untargettable mob - ignore
	// Helps in cases where servers use invisible_man, body type 11 for quests
	// and the client opens a trade by mistake.
	if (tradee && (tradee->GetBodyType() == 11)) {
		return;
	}

	CommonBreakInvisible();

	// Pass trade request on to recipient
	if (tradee && tradee->IsClient()) {
		// if we are idling we need to sync client positions otherwise clients will not be aware of each other
		if (m_is_idle) {
			SyncWorldPositionsToClient(true);
		}
		if (tradee->CastToClient()->IsIdle()) {
			tradee->CastToClient()->SyncWorldPositionsToClient(true);
		}

		tradee->CastToClient()->QueuePacket(app);
	}
	else if (tradee && (tradee->IsNPC() || tradee->IsBot())) {
        if (!tradee->IsEngaged()) {
            trade->Start(msg->to_mob_id);
            EQApplicationPacket *outapp = new EQApplicationPacket(OP_TradeRequestAck, sizeof(TradeRequest_Struct));
            TradeRequest_Struct *acc = (TradeRequest_Struct *) outapp->pBuffer;
            acc->from_mob_id = msg->to_mob_id;
            acc->to_mob_id = msg->from_mob_id;
            FastQueuePacket(&outapp);
            safe_delete(outapp);
        }
    }
	return;
	}

void Client::Handle_OP_TradeRequestAck(const EQApplicationPacket *app)
{
	if (app->size != sizeof(TradeRequest_Struct)) {
		LogError("Wrong size: OP_TradeRequestAck, size=[{}], expected [{}]", app->size, sizeof(TradeRequest_Struct));
		return;
	}
	// Trade request recipient is acknowledging they are able to trade
	// After this, the trade session has officially started
	// Send ack on to trade initiator if client
	TradeRequest_Struct* msg = (TradeRequest_Struct*)app->pBuffer;
	Mob* tradee = entity_list.GetMob(msg->to_mob_id);

	if (tradee && tradee->IsClient()) {
		// if we are idling we need to sync client positions otherwise clients will not be aware of each other
		if (m_is_idle) {
			SyncWorldPositionsToClient(true);
		}
		if (tradee->CastToClient()->IsIdle()) {
			tradee->CastToClient()->SyncWorldPositionsToClient(true);
		}

		trade->Start(msg->to_mob_id);
		tradee->CastToClient()->QueuePacket(app);
	}
	return;
}

void Client::Handle_OP_TraderShop(const EQApplicationPacket *app)
{
	auto in = (TraderClick_Struct *) app->pBuffer;
	LogTrading("Handle_OP_TraderShop: TraderClick_Struct TraderID [{}], Code [{}], Unknown008 [{}], Approval [{}]",
			   in->TraderID,
			   in->Code,
			   in->Unknown008,
			   in->Approval
	);

	switch (in->Code) {
		case ClickTrader: {
			LogTrading("Handle_OP_TraderShop case ClickTrader [{}]", in->Code);
			auto outapp =
				std::make_unique<EQApplicationPacket>(OP_TraderShop, static_cast<uint32>(sizeof(TraderClick_Struct))
			);
			auto data          = (TraderClick_Struct *) outapp->pBuffer;
			auto trader_client = entity_list.GetClientByID(in->TraderID);

			if (trader_client) {
				data->Approval = trader_client->WithCustomer(GetID());
				LogTrading("Client::Handle_OP_TraderShop: Shop Request ([{}]) to ([{}]) with Approval: [{}]",
						   GetCleanName(),
						   trader_client->GetCleanName(),
						   data->Approval
				);
			}
			else {
				LogTrading("Client::Handle_OP_TraderShop: entity_list.GetClientByID(tcs->traderid)"
						   " returned a nullptr pointer"
				);
				return;
			}

			data->Code       = ClickTrader;
			data->TraderID   = in->TraderID;
			data->Unknown008 = 0x3f800000;
			QueuePacket(outapp.get());

			if (data->Approval) {
				BulkSendTraderInventory(trader_client->CharacterID());
				trader_client->Trader_CustomerBrowsing(this);
				SetTraderID(in->TraderID);
				LogTrading("Client::Handle_OP_TraderShop: Trader Inventory Sent to [{}] from [{}]",
						   GetID(),
						   in->TraderID
				);
			}
			else {
				MessageString(Chat::Yellow, TRADER_BUSY);
				LogTrading("Client::Handle_OP_TraderShop: Trader Busy");
			}

			break;
		}
		case EndTransaction: {
			Client *c = entity_list.GetClientByID(GetTraderID());
			SetTraderID(0);
			if (c) {
				c->WithCustomer(0);
				LogTrading("End Transaction - Code [{}]", in->Code);
			}
			else {
				LogTrading("Null Client Pointer for Trader - Code [{}]", in->Code);
			}

			auto outapp = new EQApplicationPacket(OP_ShopEndConfirm);
			QueuePacket(outapp);
			safe_delete(outapp);
			break;
		}
		default: {
		}
	}
}

void Client::Handle_OP_TradeSkillCombine(const EQApplicationPacket *app)
{
	if (app->size != sizeof(NewCombine_Struct)) {
		LogError("Invalid size for NewCombine_Struct: Expected: [{}], Got: [{}]",
			sizeof(NewCombine_Struct), app->size);
		return;
	}
	/*if (m_tradeskill_object == nullptr) {
	Message(Chat::Red, "Error: Server is not aware of the tradeskill container you are attempting to use");
	return;
	}*/

	//fixed this to work for non-world objects

	// Delegate to tradeskill object to perform combine
	NewCombine_Struct* in_combine = (NewCombine_Struct*)app->pBuffer;
	Object::HandleCombine(this, in_combine, m_tradeskill_object);
	return;
}

void Client::Handle_OP_TradeSkillRecipeInspect(const EQApplicationPacket* app)
{
	if (app->size != sizeof(TradeSkillRecipeInspect_Struct)) {
		LogError(
			"Invalid size for TradeSkillRecipeInspect_Struct: Expected: [{}] Got: [{}]",
			sizeof(TradeSkillRecipeInspect_Struct),
			app->size
		);
		return;
	}

	auto s = (TradeSkillRecipeInspect_Struct*) app->pBuffer;

	const auto& v = TradeskillRecipeEntriesRepository::GetWhere(
		content_db,
		fmt::format(
			"`recipe_id` = {} AND `componentcount` = 0 AND `successcount` > 0 ORDER BY `id` ASC LIMIT 1",
			s->recipe_id
		)
	);

	if (v.empty()) {
		return;
	}

	const uint32 item_id = v.front().item_id;

	auto inst = database.CreateItem(item_id);
	if (inst) {
		SendItemPacket(0, inst, ItemPacketViewLink);
		safe_delete(inst);
	}
}

void Client::Handle_OP_Translocate(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Translocate_Struct)) {
		LogDebug("Size mismatch in OP_Translocate expected [{}] got [{}]", sizeof(Translocate_Struct), app->size);
		DumpPacket(app);
		return;
	}

	Translocate_Struct *its = (Translocate_Struct*)app->pBuffer;

	if (!PendingTranslocate) {
		return;
	}

	auto translocate_time_limit = RuleI(Spells, TranslocateTimeLimit);
	if (
		translocate_time_limit &&
		time(nullptr) > (TranslocateTime + translocate_time_limit)
	) {
		Message(Chat::Red, "You did not accept the Translocate within the required time limit.");
		PendingTranslocate = false;
		return;
	}

	if (its->Complete == 1) {
		uint32 spell_id = PendingTranslocateData.spell_id;
		bool in_translocate_zone = (
			zone->GetZoneID() == PendingTranslocateData.zone_id &&
			zone->GetInstanceID() == PendingTranslocateData.instance_id
		);

		int quest_return = 0;
		if (parse->SpellHasQuestSub(spell_id, EVENT_SPELL_EFFECT_TRANSLOCATE_COMPLETE)) {
			quest_return = parse->EventSpell(EVENT_SPELL_EFFECT_TRANSLOCATE_COMPLETE, nullptr, this, spell_id, "", 0);
		}

		if (quest_return == 0) {
			// If the spell has a translocate to bind effect, AND we are already in the zone the client
			// is bound in, use the GoToBind method. If we send OP_Translocate in this case, the client moves itself
			// to the bind coords it has from the PlayerProfile, but with the X and Y reversed. I suspect they are
			// reversed in the pp, and since spells like Gate are handled serverside, this has not mattered before.
			if (
				IsTranslocateSpell(spell_id) &&
				in_translocate_zone
				) {
				PendingTranslocate = false;
				GoToBind();
				return;
			}

			entity_list.ClearAggro(this);

			////Was sending the packet back to initiate client zone...
			////but that could be abusable, so lets go through proper channels
			MovePC(
				PendingTranslocateData.zone_id,
				PendingTranslocateData.instance_id,
				PendingTranslocateData.x,
				PendingTranslocateData.y,
				PendingTranslocateData.z,
				PendingTranslocateData.heading,
				0,
				ZoneSolicited
			);
		}
	}

	PendingTranslocate = false;
}

void Client::Handle_OP_TributeItem(const EQApplicationPacket *app)
{
	LogTribute("Received OP_TributeItem of length [{}]", app->size);

	//player donates an item...
	if (app->size != sizeof(TributeItem_Struct))
		printf("Error in OP_TributeItem. Expected size of: %zu, but got: %i\n", sizeof(StartTribute_Struct), app->size);
	else {
		TributeItem_Struct* t = (TributeItem_Struct*)app->pBuffer;

		tribute_master_id = t->tribute_master_id;
		//make sure they are dealing with a valid tribute master
		Mob* tribmast = entity_list.GetMob(t->tribute_master_id);
		if (!tribmast || !tribmast->IsNPC() || tribmast->GetClass() != Class::TributeMaster)
			return;
		if (DistanceSquared(m_Position, tribmast->GetPosition()) > USE_NPC_RANGE2)
			return;

		t->tribute_points = TributeItem(t->slot, t->quantity);

		LogTribute("Sending tribute item reply with [{}] points", t->tribute_points);

		QueuePacket(app);
	}
	return;
}

void Client::Handle_OP_TributeMoney(const EQApplicationPacket *app)
{
	LogTribute("Received OP_TributeMoney of length [{}]", app->size);

	//player donates money
	if (app->size != sizeof(TributeMoney_Struct))
		printf("Error in OP_TributeMoney. Expected size of: %zu, but got: %i\n", sizeof(StartTribute_Struct), app->size);
	else {
		TributeMoney_Struct* t = (TributeMoney_Struct*)app->pBuffer;

		tribute_master_id = t->tribute_master_id;
		//make sure they are dealing with a valid tribute master
		Mob* tribmast = entity_list.GetMob(t->tribute_master_id);
		if (!tribmast || !tribmast->IsNPC() || tribmast->GetClass() != Class::TributeMaster)
			return;
		if (DistanceSquared(m_Position, tribmast->GetPosition()) > USE_NPC_RANGE2)
			return;

		t->tribute_points = TributeMoney(t->platinum);

		LogTribute("Sending tribute money reply with [{}] points", t->tribute_points);

		QueuePacket(app);
	}
	return;
}

void Client::Handle_OP_TributeNPC(const EQApplicationPacket *app)
{
	LogTribute("Received OP_TributeNPC of length [{}]", app->size);

	return;
}

void Client::Handle_OP_TributeToggle(const EQApplicationPacket *app)
{
	LogTribute("Received OP_TributeToggle of length [{}]", app->size);

	if (app->size != sizeof(uint32))
		LogError("Invalid size on OP_TributeToggle packet");
	else {
		uint32 *val = (uint32 *)app->pBuffer;
		ToggleTribute(*val ? true : false);
	}
	return;
}

void Client::Handle_OP_TributeUpdate(const EQApplicationPacket *app)
{
	LogTribute("Received OP_TributeUpdate of length [{}]", app->size);

	//sent when the client changes their tribute settings...
	if (app->size != sizeof(TributeInfo_Struct))
		LogError("Invalid size on OP_TributeUpdate packet");
	else {
		TributeInfo_Struct *t = (TributeInfo_Struct *)app->pBuffer;
		ChangeTributeSettings(t);
	}
	return;
}

void Client::Handle_OP_VetClaimRequest(const EQApplicationPacket *app)
{
	if (app->size < sizeof(VeteranClaim)) {
		LogDebug("OP_VetClaimRequest size lower than expected: got [{}] expected at least [{}]", app->size, sizeof(VeteranClaim));
		DumpPacket(app);
		return;
	}

	VeteranClaim *vcr = (VeteranClaim *)app->pBuffer;

	if (vcr->claim_id == 0xFFFFFFFF) { // request update packet
		SendRewards();
		return;
	}
	// try to claim something!
	auto vetapp = new EQApplicationPacket(OP_VetClaimReply, sizeof(VeteranClaim));
	VeteranClaim *cr = (VeteranClaim *)vetapp->pBuffer;
	strcpy(cr->name, GetName());
	cr->claim_id = vcr->claim_id;

	if (!TryReward(vcr->claim_id))
		cr->action = 1;
	else
		cr->action = 0;

	FastQueuePacket(&vetapp);
}

void Client::Handle_OP_VoiceMacroIn(const EQApplicationPacket *app)
{

	if (app->size != sizeof(VoiceMacroIn_Struct)) {

		LogDebug("Size mismatch in OP_VoiceMacroIn expected [{}] got [{}]", sizeof(VoiceMacroIn_Struct), app->size);

		DumpPacket(app);

		return;
	}

	if (!RuleB(Chat, EnableVoiceMacros)) return;

	VoiceMacroIn_Struct* vmi = (VoiceMacroIn_Struct*)app->pBuffer;

	VoiceMacroReceived(vmi->Type, vmi->Target, vmi->MacroNumber);

}

void Client::Handle_OP_UpdateAura(const EQApplicationPacket *app)
{
	if (app->size != sizeof(AuraDestory_Struct)) {
		LogDebug("Size mismatch in OP_UpdateAura expected [{}] got [{}]", sizeof(AuraDestory_Struct), app->size);
		return;
	}

	// client only sends this for removing
	auto aura = (AuraDestory_Struct *)app->pBuffer;
	if (aura->action != 1)
		return; // could log I guess, but should only ever get this action

	RemoveAura(aura->entity_id);
	QueuePacket(app); // if we don't resend this, the client gets confused
	return;
}

void Client::Handle_OP_WearChange(const EQApplicationPacket *app)
{
	if (app->size != sizeof(WearChange_Struct)) {
		std::cout << "Wrong size: OP_WearChange, size=" << app->size << ", expected " << sizeof(WearChange_Struct) << std::endl;
		return;
	}

	WearChange_Struct* wc = (WearChange_Struct*)app->pBuffer;
	if (wc->spawn_id != GetID())
		return;

	// Hero Forge ID needs to be fixed here as RoF2 appears to send an incorrect value.
	if (wc->wear_slot_id >= 0 && wc->wear_slot_id < EQ::textures::weaponPrimary)
		wc->hero_forge_model = GetHerosForgeModel(wc->wear_slot_id);

	// we could maybe ignore this and just send our own from moveitem
	// We probably need to skip this entirely when it is send as an ack, but not sure how to ID that.
	entity_list.QueueClients(this, app, true);
}

void Client::Handle_OP_WhoAllRequest(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Who_All_Struct)) {
		std::cout << "Wrong size on OP_WhoAll. Got: " << app->size << ", Expected: " << sizeof(Who_All_Struct) << std::endl;
		return;
	}
	Who_All_Struct* whoall = (Who_All_Struct*)app->pBuffer;

	if (whoall->type == 0) // SoF only, for regular /who
		entity_list.ZoneWho(this, whoall);
	else
		WhoAll(whoall);
	return;
}

void Client::Handle_OP_XTargetAutoAddHaters(const EQApplicationPacket *app)
{
	if (app->size != 1)
	{
		LogDebug("Size mismatch in OP_XTargetAutoAddHaters, expected 1, got [{}]", app->size);
		DumpPacket(app);
		return;
	}

	XTargetAutoAddHaters = app->ReadUInt8(0);
	SetDirtyAutoHaters();
}

void Client::Handle_OP_XTargetOpen(const EQApplicationPacket *app)
{
	if (app->size != 4) {
		LogDebug("Size mismatch in OP_XTargetOpen, expected 1, got [{}]", app->size);
		DumpPacket(app);
		return;
	}

	auto outapp = new EQApplicationPacket(OP_XTargetOpenResponse, 0);
	FastQueuePacket(&outapp);
}

void Client::Handle_OP_XTargetRequest(const EQApplicationPacket *app)
{
	if (app->size < 12)
	{
		LogDebug("Size mismatch in OP_XTargetRequest, expected at least 12, got [{}]", app->size);
		DumpPacket(app);
		return;
	}

	uint32 Unknown000 = app->ReadUInt32(0);

	if (Unknown000 != 1)
		return;

	uint32 Slot = app->ReadUInt32(4);

	if (Slot >= XTARGET_HARDCAP)
		return;

	XTargetType Type = (XTargetType)app->ReadUInt32(8);

	XTargets[Slot].Type = Type;
	XTargets[Slot].ID = 0;
	XTargets[Slot].Name[0] = 0;

	switch (Type)
	{
	case Empty:
	case Auto:
	{
		break;
	}

	case CurrentTargetPC:
	{
		char Name[65];

		app->ReadString(Name, 12, 64);
		Client *c = entity_list.GetClientByName(Name);
		if (c)
		{
			XTargets[Slot].ID = c->GetID();
			strncpy(XTargets[Slot].Name, c->GetName(), 64);
		}
		else
		{
			strncpy(XTargets[Slot].Name, Name, 64);
		}
		SendXTargetPacket(Slot, c);

		break;
	}

	case CurrentTargetNPC:
	{
		char Name[65];
		app->ReadString(Name, 12, 64);
		Mob *m = entity_list.GetMob(Name);
		if (m)
		{
			XTargets[Slot].ID = m->GetID();
			SendXTargetPacket(Slot, m);
			break;
		}
	}

	case TargetsTarget:
	{
		if (GetTarget())
			UpdateXTargetType(TargetsTarget, GetTarget()->GetTarget());
		else
			UpdateXTargetType(TargetsTarget, nullptr);

		break;
	}

	case GroupTank:
	{
		Group *g = GetGroup();

		if (g)
		{
			Client *c = entity_list.GetClientByName(g->GetMainTankName());

			if (c)
			{
				XTargets[Slot].ID = c->GetID();
				strncpy(XTargets[Slot].Name, c->GetName(), 64);
			}
			else
			{
				strncpy(XTargets[Slot].Name, g->GetMainTankName(), 64);
			}
			SendXTargetPacket(Slot, c);
		}
		break;
	}
	case GroupTankTarget:
	{
		Group *g = GetGroup();

		if (g)
			g->NotifyTankTarget(this);

		break;
	}

	case GroupAssist:
	{
		Group *g = GetGroup();

		if (g)
		{
			Client *c = entity_list.GetClientByName(g->GetMainAssistName());

			if (c)
			{
				XTargets[Slot].ID = c->GetID();
				strncpy(XTargets[Slot].Name, c->GetName(), 64);
			}
			else
			{
				strncpy(XTargets[Slot].Name, g->GetMainAssistName(), 64);
			}
			SendXTargetPacket(Slot, c);
		}
		break;
	}

	case GroupAssistTarget:
	{

		Group *g = GetGroup();

		if (g)
			g->NotifyAssistTarget(this);

		break;
	}

	case Puller:
	{
		Group *g = GetGroup();

		if (g)
		{
			Client *c = entity_list.GetClientByName(g->GetPullerName());

			if (c)
			{
				XTargets[Slot].ID = c->GetID();
				strncpy(XTargets[Slot].Name, c->GetName(), 64);
			}
			else
			{
				strncpy(XTargets[Slot].Name, g->GetPullerName(), 64);
			}
			SendXTargetPacket(Slot, c);
		}
		break;
	}

	case PullerTarget:
	{

		Group *g = GetGroup();

		if (g)
			g->NotifyPullerTarget(this);

		break;
	}

	case GroupMarkTarget1:
	case GroupMarkTarget2:
	case GroupMarkTarget3:
	{
		Group *g = GetGroup();

		if (g)
			g->SendMarkedNPCsToMember(this);

		break;
	}

	case RaidAssist1:
	case RaidAssist2:
	case RaidAssist3:
	{
		struct AssistType {
			XTargetType type;
			int32       assist_slot;
		};

		std::vector<AssistType> assist_types = {
			{ RaidAssist1, MAIN_ASSIST_1_SLOT },
			{ RaidAssist2, MAIN_ASSIST_2_SLOT },
			{ RaidAssist3, MAIN_ASSIST_3_SLOT }
		};

		for (auto& t : assist_types) {
			if (t.type == Type) {
				Raid* r = GetRaid();
				if (r) {
					Client* ma = entity_list.GetClientByName(r->main_assister_pcs[t.assist_slot]);
					if (ma) {
						UpdateXTargetType(t.type, ma, ma->GetName());
					}
				}
			}
		}
		break;
	}

	case RaidAssist1Target:
	case RaidAssist2Target:
	case RaidAssist3Target:
	{
		struct AssistType {
			XTargetType type;
			int32       assist_slot;
		};

		std::vector<AssistType> assist_types = {
			{ RaidAssist1Target, MAIN_ASSIST_1_SLOT },
			{ RaidAssist2Target, MAIN_ASSIST_2_SLOT },
			{ RaidAssist3Target, MAIN_ASSIST_3_SLOT }
		};

		for (auto& t : assist_types) {
			if (t.type == Type) {
				Raid* r = GetRaid();
				if (r) {
					Client* ma = entity_list.GetClientByName(r->main_assister_pcs[t.assist_slot]);
					if (ma && ma->GetTarget()) {
						UpdateXTargetType(t.type, ma->GetTarget(), ma->GetTarget()->GetName());
					}
				}
			}
		}
		break;
	}

	case RaidMarkTarget1:
	case RaidMarkTarget2:
	case RaidMarkTarget3:
	{
		struct AssistType {
			XTargetType type;
			int32       assist_slot;
		};

		std::vector<AssistType> assist_types = {
			{ RaidMarkTarget1, MAIN_MARKER_1_SLOT },
			{ RaidMarkTarget2, MAIN_MARKER_2_SLOT },
			{ RaidMarkTarget3, MAIN_MARKER_3_SLOT }
		};

		for (auto& t : assist_types) {
			if (t.type == Type) {
				Raid* r = GetRaid();
				if (r) {
					auto mm = entity_list.GetNPCByID(r->marked_npcs[t.assist_slot].entity_id);
					if (mm) {
						UpdateXTargetType(t.type, mm->CastToMob(), mm->CastToMob()->GetName());
					}
				}
			}
		}
		break;
	}

	case MyPet:
	{
		Mob *m = GetPet();
		if (m)
		{
			XTargets[Slot].ID = m->GetID();
			SendXTargetPacket(Slot, m);

		}
		break;
	}
	case MyPetTarget:
	{
		Mob *m = GetPet();

		if (m)
			m = m->GetTarget();

		if (m)
		{
			XTargets[Slot].ID = m->GetID();
			SendXTargetPacket(Slot, m);

		}
		break;
	}

	default:
		LogDebug("Unhandled XTarget Type [{}]", static_cast<int>(Type));
		break;
	}

}

void Client::Handle_OP_YellForHelp(const EQApplicationPacket *app)
{
	auto outapp = new EQApplicationPacket(OP_YellForHelp, 4);
	*(uint32 *)outapp->pBuffer = GetID();
	entity_list.QueueCloseClients(this, outapp, true, 100.0);
	safe_delete(outapp);
	return;
}

void Client::Handle_OP_ResetAA(const EQApplicationPacket *app)
{
	if (Admin() >= AccountStatus::Guide) {
		Message(Chat::White, "Resetting AA points.");
		ResetAA();
	}
	return;
}

void Client::Handle_OP_MovementHistoryList(const EQApplicationPacket *app)
{
	cheat_manager.ProcessMovementHistory(app);
}

void Client::Handle_OP_UnderWorld(const EQApplicationPacket *app)
{
	UnderWorld *m_UnderWorld = (UnderWorld *) app->pBuffer;
	if (app->size != sizeof(UnderWorld)) {
		LogDebug("Size mismatch in OP_UnderWorld, expected {}, got [{}]", sizeof(UnderWorld), app->size);
		DumpPacket(app);
		return;
	}
	auto dist = Distance(
		glm::vec3(m_UnderWorld->x, m_UnderWorld->y, zone->newzone_data.underworld),
		glm::vec3(m_UnderWorld->x, m_UnderWorld->y, m_UnderWorld->z));
	cheat_manager.MovementCheck(glm::vec3(m_UnderWorld->x, m_UnderWorld->y, m_UnderWorld->z));
	if (m_UnderWorld->spawn_id == GetID() && dist <= 5.0f && zone->newzone_data.underworld_teleport_index != 0) {
		cheat_manager.SetExemptStatus(Port, true);
	}
}

void Client::Handle_OP_SharedTaskRemovePlayer(const EQApplicationPacket *app)
{
	if (app->size != sizeof(SharedTaskRemovePlayer_Struct)) {
		LogPacketClientServer(
			"Wrong size on Handle_OP_SharedTaskRemovePlayer | got [{}] expected [{}]",
			app->size,
			sizeof(SharedTaskRemovePlayer_Struct)
		);
		return;
	}
	auto *r = (SharedTaskRemovePlayer_Struct *) app->pBuffer;

	LogTasks(
		"field1 [{}] field2 [{}] player_name [{}]",
		r->field1,
		r->field2,
		r->player_name
	);

	// live no-ops this command if not in a shared task
	if (GetTaskState()->HasActiveSharedTask()) {
		// struct
		auto p = new ServerPacket(
			ServerOP_SharedTaskRemovePlayer,
			sizeof(ServerSharedTaskRemovePlayer_Struct)
		);

		auto *rp = (ServerSharedTaskRemovePlayer_Struct *) p->pBuffer;

		// fill
		rp->source_character_id = CharacterID();
		rp->task_id             = GetTaskState()->GetActiveSharedTask().task_id;
		strn0cpy(rp->player_name, r->player_name, sizeof(r->player_name));

		LogTasks(
			"source_character_id [{}] task_id [{}] player_name [{}]",
			rp->source_character_id,
			rp->task_id,
			rp->player_name
		);

		// send
		worldserver.SendPacket(p);
		safe_delete(p);
	}
}

void Client::Handle_OP_SharedTaskAddPlayer(const EQApplicationPacket *app)
{
	if (app->size != sizeof(SharedTaskAddPlayer_Struct)) {
		LogPacketClientServer(
			"Wrong size on Handle_OP_SharedTaskAddPlayer | got [{}] expected [{}]",
			app->size,
			sizeof(SharedTaskAddPlayer_Struct)
		);
		return;
	}
	auto *r = (SharedTaskAddPlayer_Struct *) app->pBuffer;

	LogTasks(
		"field1 [{}] field2 [{}] player_name [{}]",
		r->field1,
		r->field2,
		r->player_name
	);

	if (!GetTaskState()->HasActiveSharedTask()) {
		// this message is generated client-side in newer clients
		Message(Chat::System, TaskStr::Get(TaskStr::COULD_NOT_USE_COMMAND));
	}
	else {
		// struct
		auto p = new ServerPacket(
			ServerOP_SharedTaskAddPlayer,
			sizeof(ServerSharedTaskAddPlayer_Struct)
		);

		auto *rp = (ServerSharedTaskAddPlayer_Struct *) p->pBuffer;

		// fill
		rp->source_character_id = CharacterID();
		rp->task_id             = GetTaskState()->GetActiveSharedTask().task_id;
		strn0cpy(rp->player_name, r->player_name, sizeof(r->player_name));

		LogTasks(
			"source_character_id [{}] task_id [{}] player_name [{}]",
			rp->source_character_id,
			rp->task_id,
			rp->player_name
		);

		// send
		worldserver.SendPacket(p);
		safe_delete(p);
	}
}

void Client::Handle_OP_SharedTaskMakeLeader(const EQApplicationPacket *app)
{
	if (app->size != sizeof(SharedTaskMakeLeader_Struct)) {
		LogPacketClientServer(
			"Wrong size on Handle_OP_SharedTaskMakeLeader | got [{}] expected [{}]",
			app->size,
			sizeof(SharedTaskMakeLeader_Struct)
		);
		return;
	}

	auto *r = (SharedTaskMakeLeader_Struct *) app->pBuffer;
	LogTasks(
		"field1 [{}] field2 [{}] player_name [{}]",
		r->field1,
		r->field2,
		r->player_name
	);

	// live no-ops this command if not in a shared task
	if (GetTaskState()->HasActiveSharedTask()) {
		// struct
		auto p = new ServerPacket(
			ServerOP_SharedTaskMakeLeader,
			sizeof(ServerSharedTaskMakeLeader_Struct)
		);

		auto *rp = (ServerSharedTaskMakeLeader_Struct *) p->pBuffer;

		// fill
		rp->source_character_id = CharacterID();
		rp->task_id             = GetTaskState()->GetActiveSharedTask().task_id;
		strn0cpy(rp->player_name, r->player_name, sizeof(r->player_name));

		LogTasks(
			"source_character_id [{}] task_id [{}] player_name [{}]",
			rp->source_character_id,
			rp->task_id,
			rp->player_name
		);

		// send
		worldserver.SendPacket(p);
		safe_delete(p);
	}
}

void Client::Handle_OP_SharedTaskInviteResponse(const EQApplicationPacket *app)
{
	if (app->size != sizeof(SharedTaskInviteResponse_Struct)) {
		LogPacketClientServer(
			"Wrong size on SharedTaskInviteResponse | got [{}] expected [{}]",
			app->size,
			sizeof(SharedTaskInviteResponse_Struct)
		);
		return;
	}

	auto *r = (SharedTaskInviteResponse_Struct *) app->pBuffer;
	LogTasks(
		"unknown00 [{}] invite_id [{}] accepted [{}]",
		r->unknown00,
		r->invite_id,
		r->accepted
	);

	// struct
	auto p = new ServerPacket(
		ServerOP_SharedTaskInviteAcceptedPlayer,
		sizeof(ServerSharedTaskInviteAccepted_Struct)
	);

	auto *c = (ServerSharedTaskInviteAccepted_Struct *) p->pBuffer;

	// fill
	c->source_character_id = CharacterID();
	c->shared_task_id      = r->invite_id;
	c->accepted            = r->accepted;
	strn0cpy(c->player_name, GetName(), sizeof(c->player_name));

	LogTasks(
		"source_character_id [{}] shared_task_id [{}]",
		c->source_character_id,
		c->shared_task_id
	);

	// send
	worldserver.SendPacket(p);
	safe_delete(p);
}

void Client::Handle_OP_SharedTaskAccept(const EQApplicationPacket* app)
{
	auto buf = reinterpret_cast<SharedTaskAccept_Struct*>(app->pBuffer);

	LogTasksDetail(
		"unknown00 [{}] unknown04 [{}] npc_entity_id [{}] task_id [{}]",
		buf->unknown00,
		buf->unknown04,
		buf->npc_entity_id,
		buf->task_id
	);

	if (buf->task_id > 0 && RuleB(TaskSystem, EnableTaskSystem) && task_state)
	{
		if (task_state->CanAcceptNewTask(this, buf->task_id, buf->npc_entity_id))
		{
			task_state->AcceptNewTask(this, buf->task_id, buf->npc_entity_id, std::time(nullptr));
		}
		task_state->ClearLastOffers();
	}
}

void Client::Handle_OP_SharedTaskQuit(const EQApplicationPacket* app)
{
	if (GetTaskState()->HasActiveSharedTask())
	{
		CancelTask(TASKSLOTSHAREDTASK, TaskType::Shared);
	}
}

void Client::Handle_OP_TaskTimers(const EQApplicationPacket* app)
{
	GetTaskState()->ListTaskTimers(this);
}

void Client::Handle_OP_SharedTaskPlayerList(const EQApplicationPacket* app)
{
	if (GetTaskState()->HasActiveSharedTask())
	{
		uint32_t size = sizeof(ServerSharedTaskPlayerList_Struct);
		auto pack = std::make_unique<ServerPacket>(ServerOP_SharedTaskPlayerList, size);
		auto buf = reinterpret_cast<ServerSharedTaskPlayerList_Struct*>(pack->pBuffer);
		buf->source_character_id = CharacterID();
		buf->task_id = GetTaskState()->GetActiveSharedTask().task_id;

		worldserver.SendPacket(pack.get());
	}
}

int64 Client::GetSharedTaskId() const
{
	return m_shared_task_id;
}

void Client::SetSharedTaskId(int64 shared_task_id)
{
	Client::m_shared_task_id = shared_task_id;
}

bool Client::CanTradeFVNoDropItem()
{
	const int16 admin_status             = Admin();
	const int   no_drop_flag             = RuleI(World, FVNoDropFlag);
	const int   no_drop_min_admin_status = RuleI(Character, MinStatusForNoDropExemptions);
	switch (no_drop_flag) {
		case FVNoDropFlagRule::Disabled:
			return false;
		case FVNoDropFlagRule::Enabled:
			return true;
		case FVNoDropFlagRule::AdminOnly:
			return admin_status >= no_drop_min_admin_status;
		default:
			LogWarning("Invalid value {0} set for FVNoDropFlag", no_drop_flag);
			return false;
	}

	return false;
}

void Client::SendMobPositions()
{
	static EQApplicationPacket p(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
	auto      *s = (PlayerPositionUpdateServer_Struct *) p.pBuffer;
	for (auto &m: entity_list.GetMobList()) {
		m.second->MakeSpawnUpdate(s);
		QueuePacket(&p, false);
	}
}

struct RecordKillCheck {
	PlayerEvent::EventType event;
	bool                   check;
};

void Client::RecordKilledNPCEvent(NPC *n)
{
	if (!n) {
		LogError("NPC passed was invalid.");
		return;
	}

	bool is_named = Strings::Contains(n->GetName(), "#") && !n->IsRaidTarget();

	std::vector<RecordKillCheck> checks = {
		RecordKillCheck{.event = PlayerEvent::KILLED_NPC, .check = true},
		RecordKillCheck{.event = PlayerEvent::KILLED_NAMED_NPC, .check = is_named},
		RecordKillCheck{.event = PlayerEvent::KILLED_RAID_NPC, .check = n->IsRaidTarget()},
	};

	for (auto &c: checks) {
		if (c.check && PlayerEventLogs::Instance()->IsEventEnabled(c.event)) {
			auto e = PlayerEvent::KilledNPCEvent{
				.npc_id = n->GetNPCTypeID(),
				.npc_name = n->GetCleanName(),
				.combat_time_seconds = static_cast<uint32>(n->GetCombatRecord().TimeInCombat()),
				.total_damage_per_second_taken = static_cast<uint64>(n->GetCombatRecord().GetDamageReceivedPerSecond()),
				.total_heal_per_second_taken = static_cast<uint64>(n->GetCombatRecord().GetHealedReceivedPerSecond()),
			};
			RecordPlayerEventLog(c.event, e);
		}
	}
}

void Client::Handle_OP_RaidDelegateAbility(const EQApplicationPacket *app)
{
	if (app->size != sizeof(DelegateAbility_Struct)) {
		LogDebug(
			"Size mismatch in OP_RaidDelegateAbility expected [{}] got [{}]",
			sizeof(DelegateAbility_Struct),
			app->size
		);
		DumpPacket(app);
		return;
	}

	DelegateAbility_Struct *das = (DelegateAbility_Struct *) app->pBuffer;

	switch (das->DelegateAbility) {
		case RaidDelegateMainAssist: {
			auto r = GetRaid();
			if (r) {
				r->DelegateAbilityAssist(this, das->Name);
			}
			break;
		}
		case RaidDelegateMainMarker: {
			auto r = GetRaid();
			if (r) {
				r->DelegateAbilityMark(this, das->Name);
			}
			break;
		}
		default:
			LogDebug("RaidDelegateAbility default case");
			break;
	}
}

void Client::Handle_OP_RaidClearNPCMarks(const EQApplicationPacket* app)
{
	if (app->size != 0) {
		LogDebug("Size mismatch in OP_RaidClearNPCMark expected [{}] got [{}]", 0, app->size);
		DumpPacket(app);
		return;
	}

	auto r = GetRaid();
	if (r) {
		r->RaidClearNPCMarks(this);
	}
}

void Client::RecordStats()
{
	const uint32 character_id = CharacterID();
	if (!character_id) {
		return;
	}

	auto r = CharacterStatsRecordRepository::FindOne(
		database,
		character_id
	);

	r.status                   = Admin();
	r.name                     = GetCleanName();
	r.aa_points                = GetAAPoints() + GetSpentAA();
	r.level                    = GetLevel();
	r.class_                   = GetBaseClass();
	r.race                     = GetBaseRace();
	r.hp                       = GetMaxHP() - GetSpellBonuses().FlatMaxHPChange;
	r.mana                     = GetMaxMana() - GetSpellBonuses().Mana;
	r.endurance                = GetMaxEndurance() - GetSpellBonuses().Endurance;
	r.ac                       = GetDisplayAC() - GetSpellBonuses().AC;
	r.strength                 = GetSTR() - GetSpellBonuses().STR;
	r.stamina                  = GetSTA() - GetSpellBonuses().STA;
	r.dexterity                = GetDEX() - GetSpellBonuses().DEX;
	r.agility                  = GetAGI() - GetSpellBonuses().AGI;
	r.intelligence             = GetINT() - GetSpellBonuses().INT;
	r.wisdom                   = GetWIS() - GetSpellBonuses().WIS;
	r.charisma                 = GetCHA() - GetSpellBonuses().CHA;
	r.magic_resist             = GetMR() - GetSpellBonuses().MR;
	r.fire_resist              = GetFR() - GetSpellBonuses().FR;
	r.cold_resist              = GetCR() - GetSpellBonuses().CR;
	r.poison_resist            = GetPR() - GetSpellBonuses().PR;
	r.disease_resist           = GetDR() - GetSpellBonuses().DR;
	r.corruption_resist        = GetCorrup() - GetSpellBonuses().Corrup;
	r.heroic_strength          = GetHeroicSTR() - GetSpellBonuses().HeroicSTR;
	r.heroic_stamina           = GetHeroicSTA() - GetSpellBonuses().HeroicSTA;
	r.heroic_dexterity         = GetHeroicDEX() - GetSpellBonuses().HeroicDEX;
	r.heroic_agility           = GetHeroicAGI() - GetSpellBonuses().HeroicAGI;
	r.heroic_intelligence      = GetHeroicINT() - GetSpellBonuses().HeroicINT;
	r.heroic_wisdom            = GetHeroicWIS() - GetSpellBonuses().HeroicWIS;
	r.heroic_charisma          = GetHeroicCHA() - GetSpellBonuses().HeroicCHA;
	r.heroic_magic_resist      = GetHeroicMR() - GetSpellBonuses().HeroicMR;
	r.heroic_fire_resist       = GetHeroicFR() - GetSpellBonuses().HeroicFR;
	r.heroic_cold_resist       = GetHeroicCR() - GetSpellBonuses().HeroicCR;
	r.heroic_poison_resist     = GetHeroicPR() - GetSpellBonuses().HeroicPR;
	r.heroic_disease_resist    = GetHeroicDR() - GetSpellBonuses().HeroicDR;
	r.heroic_corruption_resist = GetHeroicCorrup() - GetSpellBonuses().HeroicCorrup;
	r.haste                    = GetHaste();
	r.accuracy                 = GetAccuracy() - GetSpellBonuses().Accuracy[EQ::skills::HIGHEST_SKILL + 1];
	r.attack                   = GetTotalATK() - GetSpellBonuses().ATK;
	r.avoidance                = GetAvoidance() - GetSpellBonuses().AvoidMeleeChance;
	r.clairvoyance             = GetClair() - GetSpellBonuses().Clairvoyance;
	r.combat_effects           = GetCombatEffects() - GetSpellBonuses().ProcChance;
	r.damage_shield_mitigation = GetDSMit() - GetSpellBonuses().DSMitigation;
	r.damage_shield            = GetDS() - GetSpellBonuses().DamageShield;
	r.dot_shielding            = GetDoTShield() - GetSpellBonuses().DoTShielding;
	r.hp_regen                 = GetHPRegen() - GetSpellBonuses().HPRegen;
	r.mana_regen               = GetManaRegen() - GetSpellBonuses().ManaRegen;
	r.endurance_regen          = GetEnduranceRegen() - GetSpellBonuses().EnduranceRegen;
	r.shielding                = GetShielding() - GetSpellBonuses().MeleeMitigation;
	r.spell_damage             = GetSpellDmg() - GetSpellBonuses().SpellDmg;
	r.heal_amount              = GetHealAmt() - GetSpellBonuses().HealAmt;
	r.spell_shielding          = GetSpellShield() - GetSpellBonuses().SpellShield;
	r.strikethrough            = GetStrikeThrough() - GetSpellBonuses().StrikeThrough;
	r.stun_resist              = GetStunResist() - GetSpellBonuses().StunResist;
	r.backstab                 = 0;
	r.wind                     = GetWindMod();
	r.brass                    = GetBrassMod();
	r.string                   = GetStringMod();
	r.percussion               = GetPercMod();
	r.singing                  = GetSingMod();
	r.baking                   = GetSkill(EQ::skills::SkillType::SkillBaking);
	r.alchemy                  = GetSkill(EQ::skills::SkillType::SkillAlchemy);
	r.jewelry                  = GetSkill(EQ::skills::SkillType::SkillJewelryMaking);
	r.tailoring                = GetSkill(EQ::skills::SkillType::SkillTailoring);
	r.blacksmithing            = GetSkill(EQ::skills::SkillType::SkillBlacksmithing);
	r.fletching                = GetSkill(EQ::skills::SkillType::SkillFletching);
	r.brewing                  = GetSkill(EQ::skills::SkillType::SkillBrewing);
	r.fishing                  = GetSkill(EQ::skills::SkillType::SkillFishing);
	r.pottery                  = GetSkill(EQ::skills::SkillType::SkillPottery);
	r.alcohol                  = GetSkill(EQ::skills::SkillType::SkillAlcoholTolerance);
	r.tinkering                = GetSkill(EQ::skills::SkillType::SkillTinkering);
	r.updated_at               = std::time(nullptr);

	if (r.character_id > 0) {
		CharacterStatsRecordRepository::UpdateOne(database, r);
	} else {
		r.character_id = character_id;
		r.created_at   = std::time(nullptr);
		CharacterStatsRecordRepository::InsertOne(database, r);
	}
}

void Client::ReloadExpansionProfileSetting()
{
	if (RuleB(World, UseClientBasedExpansionSettings)) {
		m_pp.expansions = EQ::expansions::ConvertClientVersionToExpansionsMask(ClientVersion());
	}
	else {
		m_pp.expansions = RuleI(World, ExpansionSettings);
	}
}

void Client::Handle_OP_GuildTributeSelect(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GuildTributeSelectReq_Struct)) {
		LogError("Invalid size on OP_GuildSelectTribute packet");
		return;
	}

	GuildTributeSelectReq_Struct *t = (GuildTributeSelectReq_Struct *) app->pBuffer;
	SendGuildTributeDetails(t->tribute_id, t->tier);
	return;
}

void Client::Handle_OP_GuildTributeModifyBenefits(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GuildTributeModifyBenefits_Struct)) {
		LogError("Invalid size on Handle_OP_GuildModifyBenefits packet");
		return;
	}

	GuildTributeModifyBenefits_Struct *t = (GuildTributeModifyBenefits_Struct *) app->pBuffer;

	EQApplicationPacket               *outapp = new EQApplicationPacket(
		OP_GuildModifyBenefits,
		sizeof(GuildTributeModifyBenefits_Struct));
	GuildTributeModifyBenefits_Struct *gmbs   = (GuildTributeModifyBenefits_Struct *) outapp->pBuffer;

	switch (t->command) {
		case GUILD_TRIBUTES_SAVE: {
			gmbs->command           = GUILD_TRIBUTES_SAVE;
			gmbs->data              = 1;
			gmbs->tribute_id_1      = t->tribute_id_1;
			gmbs->tribute_id_2      = t->tribute_id_2;
			gmbs->tribute_id_1_tier = t->tribute_id_1_tier;
			gmbs->tribute_id_2_tier = t->tribute_id_2_tier;
			gmbs->tribute_master_id = t->tribute_master_id;
			QueuePacket(outapp);

			auto sp  = new ServerPacket(ServerOP_RequestGuildActiveTributes, sizeof(GuildTributeUpdate));
			auto out = (GuildTributeUpdate *) sp->pBuffer;
			out->guild_id = GuildID();
			out->enabled  = false;
			worldserver.SendPacket(sp);
			safe_delete(sp);
			break;
		}
		case GUILD_TRIBUTES_MODIFY: {
			gmbs->command           = GUILD_TRIBUTES_MODIFY;
			gmbs->data              = 1;
			gmbs->tribute_id_1      = t->tribute_id_1;
			gmbs->tribute_id_2      = t->tribute_id_2;
			gmbs->tribute_id_1_tier = t->tribute_id_1_tier;
			gmbs->tribute_id_2_tier = t->tribute_id_2_tier;
			gmbs->tribute_master_id = t->tribute_master_id;
			QueuePacket(outapp);
			break;
		}
		default: {
			gmbs->command           = GUILD_TRIBUTES_SAVE;
			gmbs->data              = 1;
			gmbs->tribute_id_1      = 0;
			gmbs->tribute_id_2      = 0;
			gmbs->tribute_id_1_tier = 0;
			gmbs->tribute_id_2_tier = 0;
			gmbs->tribute_master_id = 0;
			QueuePacket(outapp);
			break;
		}
	}
	safe_delete(outapp)
	return;
}

void Client::Handle_OP_GuildTributeOptInOut(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GuildTributeOptInOutReq_Struct)) {
		LogError("Invalid size on Handle_OP_GuildTributeOptInOut packet");
		return;
	}

	GuildTributeOptInOutReq_Struct *in = (GuildTributeOptInOutReq_Struct *) app->pBuffer;

	CharGuildInfo gci;
	guild_mgr.GetCharInfo(in->player, gci);
	guild_mgr.UpdateDbMemberTributeEnabled(GuildID(), gci.char_id, in->tribute_toggle);

	auto *sout = new ServerPacket(
		ServerOP_GuildTributeOptInToggle,
		sizeof(GuildTributeMemberToggle));
	auto *out  = (GuildTributeMemberToggle *) sout->pBuffer;

	out->guild_id       = GuildID();
	out->char_id        = gci.char_id;
	out->tribute_toggle = in->tribute_toggle;
	out->command        = in->command;
	strn0cpy(out->player_name, in->player, sizeof(out->player_name));

	worldserver.SendPacket(sout);
	safe_delete(sout)
}

void Client::Handle_OP_GuildTributeSaveActiveTributes(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GuildTributeSaveActive_Struct)) {
		LogError("Invalid size on Handle_OP_GuildTributeOptInOut packet");
		return;
	}

	GuildTributeSaveActive_Struct *data = (GuildTributeSaveActive_Struct *) app->pBuffer;
	auto guild = guild_mgr.GetGuildByGuildID(GuildID());
	if (guild) {
		GuildTributesRepository::GuildTributes gt{};

		gt.guild_id          = GuildID();
		gt.tribute_id_1      = data->tribute_id_1;
		gt.tribute_id_2      = data->tribute_id_2;
		gt.tribute_id_1_tier = data->tribute_1_tier;
		gt.tribute_id_2_tier = data->tribute_2_tier;
		gt.enabled           = 0;
		gt.time_remaining    = RuleI(Guild, TributeTime);
		GuildTributesRepository::ReplaceOne(database, gt);

		guild->tribute.enabled        = 0;
		guild->tribute.id_1           = data->tribute_id_1;
		guild->tribute.id_2           = data->tribute_id_2;
		guild->tribute.id_1_tier      = data->tribute_1_tier;
		guild->tribute.id_2_tier      = data->tribute_2_tier;
		guild->tribute.time_remaining = RuleI(Guild, TributeTime);

		ServerPacket       *sp  = new ServerPacket(ServerOP_GuildTributeUpdate, sizeof(GuildTributeUpdate));
		GuildTributeUpdate *gtu = (GuildTributeUpdate *) sp->pBuffer;

		gtu->guild_id          = GuildID();
		gtu->tribute_id_1      = data->tribute_id_1;
		gtu->tribute_id_2      = data->tribute_id_2;
		gtu->tribute_id_1_tier = data->tribute_1_tier;
		gtu->tribute_id_2_tier = data->tribute_2_tier;
		gtu->enabled           = 0;
		gtu->favor             = guild->tribute.favor;
		gtu->time_remaining    = RuleI(Guild, TributeTime);

		worldserver.SendPacket(sp);
		safe_delete(sp)
	}
}

void Client::Handle_OP_GuildTributeToggle(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GuildTributeToggleReq_Struct)) {
		LogError("Invalid size on Handle_OP_GuildModifyBenefits packet");
		return;
	}

	GuildTributeToggleReq_Struct *gt = (GuildTributeToggleReq_Struct *) app->pBuffer;

	ServerPacket       *sp   = new ServerPacket(ServerOP_GuildTributeActivate, sizeof(GuildTributeUpdate));
	GuildTributeUpdate *gtu  = (GuildTributeUpdate *) sp->pBuffer;
	auto               guild = guild_mgr.GetGuildByGuildID(GuildID());
	if (guild) {
		switch (gt->command) {
			case GUILD_TRIBUTES_OFF: {
				gtu->guild_id          = GuildID();
				gtu->enabled           = GUILD_TRIBUTES_OFF;
				gtu->favor             = guild->tribute.favor;
				gtu->tribute_id_1      = guild->tribute.id_1;
				gtu->tribute_id_2      = guild->tribute.id_2;
				gtu->tribute_id_1_tier = guild->tribute.id_1_tier;
				gtu->tribute_id_2_tier = guild->tribute.id_2_tier;
				gtu->time_remaining    = guild->tribute.time_remaining;

				worldserver.SendPacket(sp);
				break;
			}
			case GUILD_TRIBUTES_ON: {
				gtu->guild_id          = GuildID();
				gtu->enabled           = GUILD_TRIBUTES_ON;
				gtu->favor             = guild->tribute.favor;
				gtu->tribute_id_1      = guild->tribute.id_1;
				gtu->tribute_id_2      = guild->tribute.id_2;
				gtu->tribute_id_1_tier = guild->tribute.id_1_tier;
				gtu->tribute_id_2_tier = guild->tribute.id_2_tier;
				gtu->time_remaining    = guild->tribute.time_remaining;

				worldserver.SendPacket(sp);
				break;
			}
			default: {
				LogError("Unknown GuildTributeUpdate to WorldServer command received.");
				break;
			}
		}
	}
	safe_delete(sp)

	RequestGuildFavorAndTimer(GuildID());

	return;
}

void Client::Handle_OP_GuildTributeDonateItem(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GuildTributeDonateItemRequest_Struct)) {
		LogError("Invalid size on Handle_OP_GuildTributeDonateItemRequest packet");
		return;
	}

	auto in = (GuildTributeDonateItemRequest_Struct *) app->pBuffer;

	const auto *inst = GetInv().GetItem(in->slot);
	auto favor = inst->GetItemGuildFavor() * in->quantity;

	auto guild = guild_mgr.GetGuildByGuildID(guild_id);
	if (guild) {
		guild->tribute.favor += favor;
		guild_mgr.UpdateDbGuildFavor(GuildID(), guild->tribute.favor);
		auto member_favor = guild_mgr.UpdateDbMemberFavor(GuildID(), CharacterID(), favor);

		if (inst->IsStackable()) {
			if (inst->GetCharges() < (int32) in->quantity) {
				favor = 0;
			}
			DeleteItemInInventory(in->slot, in->quantity, false, true);
		}
		else {
			DeleteItemInInventory(in->slot, 0, false, true);
		}

		SendGuildTributeDonateItemReply(in, favor);

		if(inst && PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::GUILD_TRIBUTE_DONATE_ITEM)) {
			auto e = PlayerEvent::GuildTributeDonateItem{ .item_id      = inst->GetID(),
														  .augment_1_id = inst->GetAugmentItemID(0),
														  .augment_2_id = inst->GetAugmentItemID(1),
														  .augment_3_id = inst->GetAugmentItemID(2),
														  .augment_4_id = inst->GetAugmentItemID(3),
														  .augment_5_id = inst->GetAugmentItemID(4),
														  .augment_6_id = inst->GetAugmentItemID(5),
														  .guild_favor  = favor
			};
			RecordPlayerEventLog(PlayerEvent::GUILD_TRIBUTE_DONATE_ITEM, e);
		}

		auto outapp = new ServerPacket(ServerOP_GuildTributeUpdateDonations, sizeof(GuildTributeUpdate));
		GuildTributeUpdate *out = (GuildTributeUpdate *) outapp->pBuffer;
		out->guild_id = GuildID();
		out->favor    = guild->tribute.favor;
		strn0cpy(out->player_name, GetCleanName(), sizeof(out->player_name));
		out->member_time    = time(nullptr);
		out->member_enabled = GuildTributeOptIn();
		out->member_favor   = member_favor;
		worldserver.SendPacket(outapp);
		safe_delete(outapp)

		RequestGuildFavorAndTimer(GuildID());
	}
}

void Client::Handle_OP_GuildTributeDonatePlat(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GuildTributeDonatePlatRequest_Struct)) {
		LogError("Invalid size on Handle_OP_GuildTributeDonatePlatRequest_Struct packet");
		return;
	}

	auto in = (GuildTributeDonatePlatRequest_Struct *) app->pBuffer;

	auto quantity = in->quantity;

	auto favor = quantity * RuleI(Guild, TributePlatConversionRate);
	auto guild = guild_mgr.GetGuildByGuildID(guild_id);
	if (guild) {
		guild->tribute.favor += favor;
		guild_mgr.UpdateDbGuildFavor(GuildID(), guild->tribute.favor);
		auto member_favor = guild_mgr.UpdateDbMemberFavor(GuildID(), CharacterID(), favor);

		TakePlatinum(quantity, false);
		SendGuildTributeDonatePlatReply(in, favor);

		if(PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::GUILD_TRIBUTE_DONATE_PLAT)) {
			auto e = PlayerEvent::GuildTributeDonatePlat {
				.plat = quantity,
				.guild_favor = favor
			};

			RecordPlayerEventLog(PlayerEvent::GUILD_TRIBUTE_DONATE_PLAT, e);
		}

		auto outapp = new ServerPacket(ServerOP_GuildTributeUpdateDonations, sizeof(GuildTributeUpdate));
		GuildTributeUpdate *out = (GuildTributeUpdate *) outapp->pBuffer;
		out->guild_id = GuildID();
		out->favor    = guild->tribute.favor;
		strncpy(out->player_name, GetCleanName(), sizeof(out->player_name));
		out->member_time    = time(nullptr);
		out->member_enabled = GuildTributeOptIn();
		out->member_favor   = member_favor;
		worldserver.SendPacket(outapp);
		safe_delete(outapp)

		RequestGuildFavorAndTimer(GuildID());
	}
}

void Client::Handle_OP_ShopSendParcel(const EQApplicationPacket *app)
{
    if (app->size != sizeof(Parcel_Struct)) {
        LogError("Received Handle_OP_ShopSendParcel packet. Expected size {}, received size {}.", sizeof(Parcel_Struct),
                 app->size);
        return;
    }

    auto parcel_in = (Parcel_Struct *)app->pBuffer;
    DoParcelSend(parcel_in);
}

void Client::Handle_OP_ShopRetrieveParcel(const EQApplicationPacket *app)
{
    if (app->size != sizeof(ParcelRetrieve_Struct)) {
        LogError("Received Handle_OP_ShopRetrieveParcel packet. Expected size {}, received size {}.",
                 sizeof(ParcelRetrieve_Struct), app->size);
        return;
    }

    auto parcel_in = (ParcelRetrieve_Struct *)app->pBuffer;
    DoParcelRetrieve(*parcel_in);
}

void Client::Handle_OP_EvolveItem(const EQApplicationPacket *app)
{
	if (app->size != sizeof(EvolveItemToggle)) {
		LogError(
			"Received Handle_OP_EvolveItem packet. Expected size {}, received size {}.",
			sizeof(EvolveItemToggle),
			app->size
		);
		return;
	}

	auto in = reinterpret_cast<EvolveItemToggle *>(app->pBuffer);

	switch (in->action) {
		case EvolvingItems::Actions::UPDATE_ITEMS: {
			DoEvolveItemToggle(app);
			break;
		}
		case EvolvingItems::Actions::FINAL_RESULT: {
			DoEvolveItemDisplayFinalResult(app);
			break;
		}
		case EvolvingItems::Actions::TRANSFER_XP: {
			DoEvolveTransferXP(app);
			break;
		}
		case EvolvingItems::Actions::TRANSFER_WINDOW_DETAILS: {
			SendEvolveXPWindowDetails(app);
		}
		default: {
		}
	}
}

bool Client::IsFilteredAFKPacket(const EQApplicationPacket *p)
{
	if (p->GetOpcode() == OP_ClientUpdate) {
		return true;
	}

	return false;
}

void Client::CheckAutoIdleAFK(PlayerPositionUpdateClient_Struct *p)
{
	if (!RuleB(Character, EnableAutoAFK)) {
		return;
	}

	bool is_non_combat_zone = !zone->CanDoCombat() || zone->BuffTimersSuspended();

	int seconds_before_afk =
			is_non_combat_zone ?
				RuleI(Character, SecondsBeforeAFKNonCombatZone) :
				RuleI(Character, SecondsBeforeAFKCombatZone);

	int seconds_before_idle =
			is_non_combat_zone ?
				RuleI(Character, SecondsBeforeIdleNonCombatZone) :
				RuleI(Character, SecondsBeforeIdleCombatZone);

	// seconds_before_idle can't be greater than seconds_before_afk
	if (seconds_before_idle > seconds_before_afk) {
		seconds_before_idle = seconds_before_afk;
	}

	bool has_moved =
			 m_Position.x != p->x_pos ||
			 m_Position.y != p->y_pos ||
			 m_Position.z != p->z_pos ||
			 m_Position.w != EQ12toFloat(p->heading);

	bool triggered_reset = m_afk_reset;
	bool was_idle        = m_is_idle;
	bool is_idle_or_afk  = m_is_idle || m_is_afk;

	if (!has_moved && (!m_is_idle || !m_is_afk)) {
		auto now              = std::chrono::steady_clock::now();
		auto since_last_moved = now - m_last_moved;

		if (!m_is_manual_afk && !m_is_afk && since_last_moved > std::chrono::seconds(seconds_before_afk)) {
			bool is_client_excluded_from_afk = (IsBuyer() || IsTrader() || GetGM());
			if (is_client_excluded_from_afk) {
				return;
			}

			LogInfo(
				"Client [{}] has been AFK for [{}] seconds",
				GetCleanName(),
				std::chrono::duration_cast<std::chrono::seconds>(since_last_moved).count()
			);
			SetAFK(true);
			return;
		}
		else if (!m_is_idle && since_last_moved > std::chrono::seconds(seconds_before_idle)) {
			bool is_client_excluded_from_idle = GetGM() && !is_non_combat_zone;
			if (is_client_excluded_from_idle) {
				return;
			}

			LogInfo(
				"Client [{}] has been idle for [{}] seconds",
				GetCleanName(),
				std::chrono::duration_cast<std::chrono::seconds>(since_last_moved).count()
			);
			m_is_idle = true;
			Message(Chat::Yellow, "You are now idle. Updates will be sent to you less frequently.");
			return;
		}
	}

	// if we triggered a reset, but didn't move, we are still idling but not AFK
	if (triggered_reset && was_idle) {
		m_is_idle = true;
	}

	// if we moved or triggered reset through other actions, we are no longer AFK.
	// we could trigger resetting AFK status through actions like message, cast, attack etc but still by idle until we move
	if (!m_is_manual_afk && (has_moved || triggered_reset) && m_is_afk) {
		LogInfo("AFK [{}] is no longer idle, syncing positions", GetCleanName());
		SetAFK(false);
		ResetAFKTimer();
	}

	// we could be not AFK and idle at the same time
	if (has_moved && m_is_idle) {
		LogInfo("Idle [{}] is no longer idle, syncing positions", GetCleanName());
		m_is_idle = false;
		Message(Chat::Yellow, "You are no longer idle.");
		SyncWorldPositionsToClient();
		ResetAFKTimer();
	}

	m_afk_reset = false;
}

void Client::SyncWorldPositionsToClient(bool ignore_idle)
{
	// if we are idle currently, we need to force updates (which bypasses idle status) and reset idle status
	bool reset_idle = false;
	if (ignore_idle && m_is_idle) {
		m_is_idle = false;
		reset_idle = true;
	}

	LogInfo("Syncing positions for client [{}]", GetCleanName());
	CheckSendBulkNpcPositions(true);

	static EQApplicationPacket cu(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));

	for (auto &e: entity_list.GetClientList()) {
		auto c = e.second;

		// skip if not in range
		if (Distance(c->GetPosition(), GetPosition()) > RuleI(Range, ClientPositionUpdates)) {
			continue;
		}

		// skip self
		if (c == this) {
			continue;
		}

		auto *spu = (PlayerPositionUpdateServer_Struct *) cu.pBuffer;

		memset(spu, 0x00, sizeof(PlayerPositionUpdateServer_Struct));
		spu->spawn_id      = c->GetID();
		spu->x_pos         = FloatToEQ19(c->GetX());
		spu->y_pos         = FloatToEQ19(c->GetY());
		spu->z_pos         = FloatToEQ19(c->GetZ());
		spu->heading       = FloatToEQ12(c->GetHeading());
		spu->delta_x       = FloatToEQ13(0);
		spu->delta_y       = FloatToEQ13(0);
		spu->delta_z       = FloatToEQ13(0);
		spu->delta_heading = FloatToEQ10(0);
		spu->animation     = 0;
		QueuePacket(&cu);
	}

	if (ignore_idle && reset_idle) {
		m_is_idle = false;
	}
}
