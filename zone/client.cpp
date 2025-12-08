/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.org)

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
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// for windows compile
#ifndef _WINDOWS
	#include <stdarg.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include "../common/unix.h"
#endif

extern volatile bool RunLoops;

#include "../common/eqemu_logsys.h"
#include "../common/features.h"
#include "../common/spdat.h"
#include "../common/guilds.h"
#include "../common/rulesys.h"
#include "../common/strings.h"
#include "../common/data_verification.h"
#include "../common/profanity_manager.h"
#include "../common/data_bucket.h"
#include "dynamic_zone.h"
#include "expedition_request.h"
#include "position.h"
#include "worldserver.h"
#include "zonedb.h"
#include "petitions.h"
#include "command.h"
#include "water_map.h"
#include "bot_command.h"
#include "string_ids.h"
#include "dialogue_window.h"

#include "guild_mgr.h"
#include "quest_parser_collection.h"
#include "queryserv.h"
#include "mob_movement_manager.h"
#include "cheat_manager.h"
#include "lua_parser.h"

#include "../common/repositories/character_alternate_abilities_repository.h"
#include "../common/repositories/character_expedition_lockouts_repository.h"
#include "../common/repositories/account_flags_repository.h"
#include "../common/repositories/bug_reports_repository.h"
#include "../common/repositories/char_recipe_list_repository.h"
#include "../common/repositories/character_spells_repository.h"
#include "../common/repositories/character_disciplines_repository.h"
#include "../common/repositories/character_data_repository.h"
#include "../common/repositories/character_pet_name_repository.h"
#include "../common/repositories/completed_tasks_repository.h"
#include "../common/repositories/discovered_items_repository.h"
#include "../common/repositories/inventory_repository.h"
#include "../common/repositories/keyring_repository.h"
#include "../common/repositories/tradeskill_recipe_repository.h"
#include "../common/events/player_events.h"
#include "../common/events/player_event_logs.h"
#include "dialogue_window.h"
#include "../common/zone_store.h"
#include "../common/skill_caps.h"


extern QueryServ* QServ;
extern EntityList entity_list;
extern Zone* zone;
extern volatile bool is_zone_loaded;
extern WorldServer worldserver;
extern uint32 numclients;

void UpdateWindowTitle(char* iNewTitle);

// client constructor purely for testing / mocking
Client::Client() : Mob(
	"No name", // in_name
	"", // in_lastname
	0, // in_cur_hp
	0, // in_max_hp
	Gender::Male, // in_gender
	Race::Doug, // in_race
	Class::None, // in_class
	BodyType::Humanoid, // in_bodytype
	Deity::Unknown, // in_deity
	0, // in_level
	0, // in_npctype_id
	0.0f, // in_size
	0.7f, // in_runspeed
	glm::vec4(), // position
	0, // in_light
	0xFF, // in_texture
	0xFF, // in_helmtexture
	0, // in_ac
	0, // in_atk
	0, // in_str
	0, // in_sta
	0, // in_dex
	0, // in_agi
	0, // in_int
	0, // in_wis
	0, // in_cha
	0, // in_haircolor
	0, // in_beardcolor
	0, // in_eyecolor1
	0, // in_eyecolor2
	0, // in_hairstyle
	0, // in_luclinface
	0, // in_beard
	0, // in_drakkin_heritage
	0, // in_drakkin_tattoo
	0, // in_drakkin_details
	EQ::TintProfile(), // in_armor_tint
	0xff, // in_aa_title
	0, // in_see_invis
	0, // in_see_invis_undead
	0, // in_see_hide
	0, // in_see_improved_hide
	0, // in_hp_regen
	0, // in_mana_regen
	0, // in_qglobal
	0, // in_maxlevel
	0, // in_scalerate
	0, // in_armtexture
	0, // in_bracertexture
	0, // in_handtexture
	0, // in_legtexture
	0, // in_feettexture
	0, // in_usemodel
	false, // in_always_aggros_foes
	0, // in_heroic_strikethrough
	false // in_keeps_sold_items
),
				   hpupdate_timer(2000),
				   camp_timer(29000),
				   process_timer(100),
				   consume_food_timer(CONSUMPTION_TIMER),
				   zoneinpacket_timer(1000),
				   linkdead_timer(RuleI(Zone, ClientLinkdeadMS)),
				   dead_timer(2000),
				   global_channel_timer(1000),
				   fishing_timer(8000),
				   endupkeep_timer(1000),
				   autosave_timer(RuleI(Character, AutosaveIntervalS) * 1000),
				   m_client_npc_aggro_scan_timer(RuleI(Aggro, ClientAggroCheckIdleInterval)),
				   m_client_bulk_npc_pos_update_timer(60 * 1000),
				   tribute_timer(Tribute_duration),
				   proximity_timer(ClientProximity_interval),
				   TaskPeriodic_Timer(RuleI(TaskSystem, PeriodicCheckTimer) * 1000),
				   charm_update_timer(6000),
				   rest_timer(1),
				   pick_lock_timer(1000),
				   charm_class_attacks_timer(3000),
				   charm_cast_timer(3500),
				   qglobal_purge_timer(30000),
				   TrackingTimer(2000),
				   RespawnFromHoverTimer(0),
				   merc_timer(RuleI(Mercs, UpkeepIntervalMS)),
				   ItemQuestTimer(500),
				   anon_toggle_timer(250),
				   afk_toggle_timer(250),
				   helm_toggle_timer(250),
				   aggro_meter_timer(AGGRO_METER_UPDATE_MS),
				   m_Proximity(FLT_MAX, FLT_MAX, FLT_MAX), //arbitrary large number
				   m_ZoneSummonLocation(-2.0f, -2.0f, -2.0f, -2.0f),
				   m_AutoAttackPosition(0.0f, 0.0f, 0.0f, 0.0f),
				   m_AutoAttackTargetLocation(0.0f, 0.0f, 0.0f),
				   last_region_type(RegionTypeUnsupported),
				   m_dirtyautohaters(false),
				   m_position_update_timer(10000),
				   consent_throttle_timer(2000),
				   tmSitting(0),
				   parcel_timer(RuleI(Parcel, ParcelDeliveryDelay)),
				   lazy_load_bank_check_timer(1000),
				   bandolier_throttle_timer(0)
{
	eqs = nullptr;
	for (auto client_filter = FilterNone; client_filter < _FilterCount; client_filter = eqFilterType(client_filter + 1)) {
		SetFilter(client_filter, FilterShow);
	}

	cheat_manager.SetClient(this);
	mMovementManager->AddClient(this);
	character_id = 0;
	conn_state = NoPacketsReceived;
	client_data_loaded = false;
	berserk = false;
	dead = false;
	client_state = CLIENT_CONNECTING;
	SetTrader(false);
	Haste = 0;
	SetCustomerID(0);
	SetTraderID(0);
	TrackingID = 0;
	WID = 0;
	account_id = 0;
	admin = AccountStatus::Player;
	lsaccountid = 0;
	guild_id = GUILD_NONE;
	guildrank = 0;
	guild_tribute_opt_in = 0;
	SetGuildListDirty(false);
	GuildBanker = false;
	memset(lskey, 0, sizeof(lskey));
	strcpy(account_name, "");
	tellsoff = false;
	last_reported_mana = 0;
	last_reported_endurance = 0;
	last_reported_endurance_percent = 0;
	last_reported_mana_percent = 0;
	gm_hide_me = false;
	m_is_afk   = false;
	LFG        = false;
	LFGFromLevel = 0;
	LFGToLevel = 0;
	LFGMatchFilter = false;
	LFGComments[0] = '\0';
	LFP = false;
	gmspeed = 0;
	gminvul = false;
	playeraction = 0;
	SetTarget(0);
	auto_attack = false;
	auto_fire = false;
	runmode = false;
	linkdead_timer.Disable();
	zonesummon_id = 0;
	zonesummon_ignorerestrictions = 0;
	bZoning              = false;
	m_lock_save_position = false;
	zone_mode            = ZoneUnsolicited;
	casting_spell_id = 0;
	npcflag = false;
	npclevel = 0;
	fishing_timer.Disable();
	dead_timer.Disable();
	camp_timer.Disable();
	autosave_timer.Disable();
	GetMercTimer()->Disable();
	instalog = false;
	m_pp.autosplit = false;
	// initialise haste variable
	m_tradeskill_object = nullptr;
	delaytimer = false;
	PendingRezzXP = -1;
	PendingRezzDBID = 0;
	PendingRezzSpellID = 0;
	numclients++;
	// emuerror;
	UpdateWindowTitle(nullptr);
	horseId = 0;
	tgb = false;
	tribute_master_id = 0xFFFFFFFF;
	tribute_timer.Disable();
	task_state         = nullptr;
	TotalSecondsPlayed = 0;
	keyring.clear();
	bind_sight_target = nullptr;
	p_raid_instance = nullptr;
	mercid = 0;
	mercSlot = 0;
	InitializeMercInfo();
	SetMerc(0);
	if (RuleI(World, PVPMinLevel) > 0 && level >= RuleI(World, PVPMinLevel) && m_pp.pvp == 0) SetPVP(true, false);
	dynamiczone_removal_timer.Disable();

	//for good measure:
	memset(&m_pp, 0, sizeof(m_pp));
	memset(&m_epp, 0, sizeof(m_epp));
	PendingTranslocate = false;
	PendingSacrifice = false;
	sacrifice_caster_id = 0;
	controlling_boat_id = 0;
	controlled_mob_id = 0;
	qGlobals = nullptr;

	if (!RuleB(Character, PerCharacterQglobalMaxLevel) && !RuleB(Character, PerCharacterBucketMaxLevel)) {
		SetClientMaxLevel(0);
	} else if (RuleB(Character, PerCharacterQglobalMaxLevel)) {
		SetClientMaxLevel(GetCharMaxLevelFromQGlobal());
	} else if (RuleB(Character, PerCharacterBucketMaxLevel)) {
		SetClientMaxLevel(GetCharMaxLevelFromBucket());
	}

	KarmaUpdateTimer = new Timer(RuleI(Chat, KarmaUpdateIntervalMS));
	GlobalChatLimiterTimer = new Timer(RuleI(Chat, IntervalDurationMS));
	AttemptedMessages = 0;
	TotalKarma = 0;
	m_ClientVersion = EQ::versions::ClientVersion::Unknown;
	m_ClientVersionBit = 0;
	AggroCount = 0;
	ooc_regen = false;
	AreaHPRegen = 1.0f;
	AreaManaRegen = 1.0f;
	AreaEndRegen = 1.0f;
	XPRate = 100;
	current_endurance = 0;

	CanUseReport = true;
	aa_los_them_mob = nullptr;
	los_status = false;
	los_status_facing = false;
	HideCorpseMode = HideCorpseNone;
	PendingGuildInvitation = false;

	InitializeBuffSlots();

	adventure_request_timer = nullptr;
	adventure_create_timer = nullptr;
	adventure_leave_timer = nullptr;
	adventure_door_timer = nullptr;
	adv_requested_data = nullptr;
	adventure_stats_timer = nullptr;
	adventure_leaderboard_timer = nullptr;
	adv_data = nullptr;
	adv_requested_theme = LDoNTheme::Unused;
	adv_requested_id = 0;
	adv_requested_member_count = 0;

	for(int i = 0; i < XTARGET_HARDCAP; ++i)
	{
		XTargets[i].Type = Auto;
		XTargets[i].ID = 0;
		XTargets[i].Name[0] = 0;
		XTargets[i].dirty = false;
	}
	MaxXTargets = 5;
	XTargetAutoAddHaters = true;
	m_autohatermgr.SetOwner(this, nullptr, nullptr);
	m_activeautohatermgr = &m_autohatermgr;

	initial_respawn_selection = 0;
	alternate_currency_loaded = false;

	interrogateinv_flag = false;

	trapid = 0;

	for (int i = 0; i < InnateSkillMax; ++i)
		m_pp.InnateSkills[i] = InnateDisabled;

	temp_pvp = false;

	moving = false;

	environment_damage_modifier = 0;
	invulnerable_environment_damage = false;

	// rate limiter
	m_list_task_timers_rate_limit.Start(1000);

	// gm
	SetDisplayMobInfoWindow(true);
	SetDevToolsEnabled(true);

	bot_owner_options[booDeathMarquee] = false;
	bot_owner_options[booStatsUpdate] = false;
	bot_owner_options[booSpawnMessageSay] = false;
	bot_owner_options[booSpawnMessageTell] = true;
	bot_owner_options[booSpawnMessageClassSpecific] = true;
	bot_owner_options[booAutoDefend] = RuleB(Bots, AllowOwnerOptionAutoDefend);
	bot_owner_options[booBuffCounter] = false;
	bot_owner_options[booMonkWuMessage] = false;

	m_parcel_platinum         = 0;
	m_parcel_gold             = 0;
	m_parcel_silver           = 0;
	m_parcel_copper           = 0;
	m_parcel_count            = 0;
	m_parcel_enabled          = true;
	m_parcel_merchant_engaged = false;
	m_parcels.clear();

	m_buyer_id = 0;

	SetBotPulling(false);
	SetBotPrecombat(false);

	AI_Init();

}

Client::Client(EQStreamInterface *ieqs) : Mob(
	"No name", // in_name
	"", // in_lastname
	0, // in_cur_hp
	0, // in_max_hp
	Gender::Male, // in_gender
	Race::Doug, // in_race
	Class::None, // in_class
	BodyType::Humanoid, // in_bodytype
	Deity::Unknown, // in_deity
	0, // in_level
	0, // in_npctype_id
	0.0f, // in_size
	0.7f, // in_runspeed
	glm::vec4(), // position
	0, // in_light
	0xFF, // in_texture
	0xFF, // in_helmtexture
	0, // in_ac
	0, // in_atk
	0, // in_str
	0, // in_sta
	0, // in_dex
	0, // in_agi
	0, // in_int
	0, // in_wis
	0, // in_cha
	0, // in_haircolor
	0, // in_beardcolor
	0, // in_eyecolor1
	0, // in_eyecolor2
	0, // in_hairstyle
	0, // in_luclinface
	0, // in_beard
	0, // in_drakkin_heritage
	0, // in_drakkin_tattoo
	0, // in_drakkin_details
	EQ::TintProfile(), // in_armor_tint
	0xff, // in_aa_title
	0, // in_see_invis
	0, // in_see_invis_undead
	0, // in_see_hide
	0, // in_see_improved_hide
	0, // in_hp_regen
	0, // in_mana_regen
	0, // in_qglobal
	0, // in_maxlevel
	0, // in_scalerate
	0, // in_armtexture
	0, // in_bracertexture
	0, // in_handtexture
	0, // in_legtexture
	0, // in_feettexture
	0, // in_usemodel
	false, // in_always_aggros_foes
	0, // in_heroic_strikethrough
	false // in_keeps_sold_items
),
	hpupdate_timer(2000),
	camp_timer(29000),
	bot_camp_timer((RuleI(Bots, CampTimer) * 1000)),
	process_timer(100),
	consume_food_timer(CONSUMPTION_TIMER),
	zoneinpacket_timer(1000),
	linkdead_timer(RuleI(Zone, ClientLinkdeadMS)),
	dead_timer(2000),
	global_channel_timer(1000),
	fishing_timer(8000),
	endupkeep_timer(1000),
	autosave_timer(RuleI(Character, AutosaveIntervalS) * 1000),
	m_client_npc_aggro_scan_timer(RuleI(Aggro, ClientAggroCheckIdleInterval)),
	m_client_bulk_npc_pos_update_timer(60 * 1000),
	tribute_timer(Tribute_duration),
	proximity_timer(ClientProximity_interval),
	TaskPeriodic_Timer(RuleI(TaskSystem, PeriodicCheckTimer) * 1000),
	charm_update_timer(6000),
	rest_timer(1),
	pick_lock_timer(1000),
	charm_class_attacks_timer(3000),
	charm_cast_timer(3500),
	qglobal_purge_timer(30000),
	TrackingTimer(2000),
	RespawnFromHoverTimer(0),
	merc_timer(RuleI(Mercs, UpkeepIntervalMS)),
	ItemQuestTimer(500),
	anon_toggle_timer(250),
	afk_toggle_timer(250),
	helm_toggle_timer(250),
	aggro_meter_timer(AGGRO_METER_UPDATE_MS),
	m_Proximity(FLT_MAX, FLT_MAX, FLT_MAX), //arbitrary large number
	m_ZoneSummonLocation(-2.0f, -2.0f, -2.0f, -2.0f),
	m_AutoAttackPosition(0.0f, 0.0f, 0.0f, 0.0f),
	m_AutoAttackTargetLocation(0.0f, 0.0f, 0.0f),
	last_region_type(RegionTypeUnsupported),
	m_dirtyautohaters(false),
	m_position_update_timer(10000),
	consent_throttle_timer(2000),
	tmSitting(0),
	parcel_timer(RuleI(Parcel, ParcelDeliveryDelay)),
	lazy_load_bank_check_timer(1000),
	bandolier_throttle_timer(0)
{
	for (auto client_filter = FilterNone; client_filter < _FilterCount; client_filter = eqFilterType(client_filter + 1)) {
		SetFilter(client_filter, FilterShow);
	}

	cheat_manager.SetClient(this);
	mMovementManager->AddClient(this);
	character_id = 0;
	conn_state = NoPacketsReceived;
	client_data_loaded = false;
	berserk = false;
	dead = false;
	eqs = ieqs;
	ip = eqs->GetRemoteIP();
	port = ntohs(eqs->GetRemotePort());
	client_state = CLIENT_CONNECTING;
	SetTrader(false);
	Haste = 0;
	SetCustomerID(0);
	SetTraderID(0);
	TrackingID = 0;
	WID = 0;
	account_id = 0;
	admin = AccountStatus::Player;
	lsaccountid = 0;
	guild_id = GUILD_NONE;
	guildrank = 0;
	guild_tribute_opt_in = 0;
	SetGuildListDirty(false);
	GuildBanker = false;
	memset(lskey, 0, sizeof(lskey));
	strcpy(account_name, "");
	tellsoff = false;
	last_reported_mana = 0;
	last_reported_endurance = 0;
	last_reported_endurance_percent = 0;
	last_reported_mana_percent = 0;
	gm_hide_me = false;
	m_is_afk   = false;
	LFG        = false;
	LFGFromLevel = 0;
	LFGToLevel = 0;
	LFGMatchFilter = false;
	LFGComments[0] = '\0';
	LFP = false;
	gmspeed = 0;
	gminvul = false;
	playeraction = 0;
	SetTarget(0);
	auto_attack = false;
	auto_fire = false;
	runmode = false;
	linkdead_timer.Disable();
	zonesummon_id = 0;
	zonesummon_ignorerestrictions = 0;
	bZoning              = false;
	m_lock_save_position = false;
	zone_mode            = ZoneUnsolicited;
	casting_spell_id = 0;
	npcflag = false;
	npclevel = 0;
	fishing_timer.Disable();
	dead_timer.Disable();
	camp_timer.Disable();
	bot_camp_timer.Disable();
	autosave_timer.Disable();
	GetMercTimer()->Disable();
	instalog = false;
	m_pp.autosplit = false;
	// initialise haste variable
	m_tradeskill_object = nullptr;
	delaytimer = false;
	PendingRezzXP = -1;
	PendingRezzDBID = 0;
	PendingRezzSpellID = 0;
	numclients++;
	// emuerror;
	UpdateWindowTitle(nullptr);
	horseId = 0;
	tgb = false;
	tribute_master_id = 0xFFFFFFFF;
	tribute_timer.Disable();
	task_state         = nullptr;
	TotalSecondsPlayed = 0;
	keyring.clear();
	bind_sight_target = nullptr;
	p_raid_instance = nullptr;
	mercid = 0;
	mercSlot = 0;
	InitializeMercInfo();
	SetMerc(0);
	if (RuleI(World, PVPMinLevel) > 0 && level >= RuleI(World, PVPMinLevel) && m_pp.pvp == 0) SetPVP(true, false);
	dynamiczone_removal_timer.Disable();

	//for good measure:
	memset(&m_pp, 0, sizeof(m_pp));
	memset(&m_epp, 0, sizeof(m_epp));
	PendingTranslocate = false;
	PendingSacrifice = false;
	sacrifice_caster_id = 0;
	controlling_boat_id = 0;
	controlled_mob_id = 0;
	qGlobals = nullptr;

	if (!RuleB(Character, PerCharacterQglobalMaxLevel) && !RuleB(Character, PerCharacterBucketMaxLevel)) {
		SetClientMaxLevel(0);
	} else if (RuleB(Character, PerCharacterQglobalMaxLevel)) {
		SetClientMaxLevel(GetCharMaxLevelFromQGlobal());
	} else if (RuleB(Character, PerCharacterBucketMaxLevel)) {
		SetClientMaxLevel(GetCharMaxLevelFromBucket());
	}

	KarmaUpdateTimer = new Timer(RuleI(Chat, KarmaUpdateIntervalMS));
	GlobalChatLimiterTimer = new Timer(RuleI(Chat, IntervalDurationMS));
	AttemptedMessages = 0;
	TotalKarma = 0;
	m_ClientVersion = EQ::versions::ClientVersion::Unknown;
	m_ClientVersionBit = 0;
	AggroCount = 0;
	ooc_regen = false;
	AreaHPRegen = 1.0f;
	AreaManaRegen = 1.0f;
	AreaEndRegen = 1.0f;
	XPRate = 100;
	current_endurance = 0;

	CanUseReport = true;
	aa_los_them_mob = nullptr;
	los_status = false;
	los_status_facing = false;
	HideCorpseMode = HideCorpseNone;
	PendingGuildInvitation = false;

	InitializeBuffSlots();

	adventure_request_timer = nullptr;
	adventure_create_timer = nullptr;
	adventure_leave_timer = nullptr;
	adventure_door_timer = nullptr;
	adv_requested_data = nullptr;
	adventure_stats_timer = nullptr;
	adventure_leaderboard_timer = nullptr;
	adv_data = nullptr;
	adv_requested_theme = LDoNTheme::Unused;
	adv_requested_id = 0;
	adv_requested_member_count = 0;

	for(int i = 0; i < XTARGET_HARDCAP; ++i)
	{
		XTargets[i].Type = Auto;
		XTargets[i].ID = 0;
		XTargets[i].Name[0] = 0;
		XTargets[i].dirty = false;
	}
	MaxXTargets = 5;
	XTargetAutoAddHaters = true;
	m_autohatermgr.SetOwner(this, nullptr, nullptr);
	m_activeautohatermgr = &m_autohatermgr;

	initial_respawn_selection = 0;
	alternate_currency_loaded = false;

	interrogateinv_flag = false;

	trapid = 0;

	for (int i = 0; i < InnateSkillMax; ++i)
		m_pp.InnateSkills[i] = InnateDisabled;

	temp_pvp = false;

	moving = false;

	environment_damage_modifier = 0;
	invulnerable_environment_damage = false;

	// rate limiter
	m_list_task_timers_rate_limit.Start(1000);

	// gm
	SetDisplayMobInfoWindow(true);
	SetDevToolsEnabled(true);

	bot_owner_options[booDeathMarquee] = false;
	bot_owner_options[booStatsUpdate] = false;
	bot_owner_options[booSpawnMessageSay] = false;
	bot_owner_options[booSpawnMessageTell] = true;
	bot_owner_options[booSpawnMessageClassSpecific] = true;
	bot_owner_options[booAutoDefend] = RuleB(Bots, AllowOwnerOptionAutoDefend);
	bot_owner_options[booBuffCounter] = false;
	bot_owner_options[booMonkWuMessage] = false;

	m_parcel_platinum         = 0;
	m_parcel_gold             = 0;
	m_parcel_silver           = 0;
	m_parcel_copper           = 0;
	m_parcel_count            = 0;
	m_parcel_enabled          = true;
	m_parcel_merchant_engaged = false;
	m_parcels.clear();

	m_buyer_id = 0;

	SetBotPulling(false);
	SetBotPrecombat(false);

	AI_Init();
}

Client::~Client() {
	entity_list.RemoveMobFromCloseLists(this);
	m_close_mobs.clear();

	if (ClientVersion() == EQ::versions::ClientVersion::RoF2 && RuleB (Parcel, EnableParcelMerchants)) {
		DoParcelCancel();
	}

	mMovementManager->RemoveClient(this);

	DataBucket::DeleteCachedBuckets(DataBucketLoadType::Account, AccountID());
	DataBucket::DeleteCachedBuckets(DataBucketLoadType::Client, CharacterID());

	if (RuleB(Bots, Enabled)) {
		Bot::ProcessBotOwnerRefDelete(this);
	}

	if (zone) {
		zone->ClearEXPModifier(this);
	}

	if (!IsZoning()) {
		if(IsInAGuild()) {
			guild_mgr.UpdateDbMemberOnline(CharacterID(), false);
			guild_mgr.SendGuildMemberUpdateToWorld(GetName(), GuildID(), 0, time(nullptr));
		}
	}

	Mob* horse = entity_list.GetMob(CastToClient()->GetHorseId());
	if (horse)
		horse->Depop();

	Mob* merc = entity_list.GetMob(GetMercenaryID());
	if (merc)
		merc->Depop();

	if(IsTrader()) {
		TraderEndTrader();
	}

	if(IsBuyer()) {
		ToggleBuyerMode(false);
	}

	if(conn_state != ClientConnectFinished) {
		LogDebug("Client [{}] was destroyed before reaching the connected state:", GetName());
		ReportConnectingState();
	}

	if(m_tradeskill_object != nullptr) {
		m_tradeskill_object->Close();
		m_tradeskill_object = nullptr;
	}

	m_close_mobs.clear();

	if(IsDueling() && GetDuelTarget() != 0) {
		Entity* entity = entity_list.GetID(GetDuelTarget());
		if(entity != nullptr && entity->IsClient()) {
			entity->CastToClient()->SetDueling(false);
			entity->CastToClient()->SetDuelTarget(0);
			entity_list.DuelMessage(entity->CastToClient(),this,true);
		}
	}

	if(GetTarget())
		GetTarget()->IsTargeted(-1);

	//if we are in a group and we are not zoning, force leave the group
	if(isgrouped && !bZoning && is_zone_loaded)
		LeaveGroup();

	UpdateWho(2);

	if(IsHoveringForRespawn())
	{
		m_pp.zone_id = m_pp.binds[0].zone_id;
		m_pp.zoneInstance = m_pp.binds[0].instance_id;
		m_Position.x = m_pp.binds[0].x;
		m_Position.y = m_pp.binds[0].y;
		m_Position.z = m_pp.binds[0].z;
	}

	// we save right now, because the client might be zoning and the world
	// will need this data right away
	Save(2); // This fails when database destructor is called first on shutdown

	safe_delete(task_state);
	safe_delete(KarmaUpdateTimer);
	safe_delete(GlobalChatLimiterTimer);
	safe_delete(qGlobals);
	safe_delete(adventure_request_timer);
	safe_delete(adventure_create_timer);
	safe_delete(adventure_leave_timer);
	safe_delete(adventure_door_timer);
	safe_delete(adventure_stats_timer);
	safe_delete(adventure_leaderboard_timer);
	safe_delete_array(adv_requested_data);
	safe_delete_array(adv_data);

	ClearRespawnOptions();

	numclients--;
	UpdateWindowTitle(nullptr);
	if(zone)
		zone->RemoveAuth(GetName(), lskey);

	//let the stream factory know were done with this stream
	if (eqs) {
		eqs->Close();
		eqs->ReleaseFromUse();
		safe_delete(eqs);
	}

	UninitializeBuffSlots();
}

void Client::SendZoneInPackets()
{

	//////////////////////////////////////////////////////
	// Spawn Appearance Packet
	auto outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
	SpawnAppearance_Struct* sa = (SpawnAppearance_Struct*)outapp->pBuffer;
	sa->type = AppearanceType::SpawnID;			// Is 0x10 used to set the player id?
	sa->parameter = GetID();	// Four bytes for this parameter...
	outapp->priority = 6;
	QueuePacket(outapp);
	safe_delete(outapp);

	// Inform the world about the client
	outapp = new EQApplicationPacket();

	CreateSpawnPacket(outapp);
	outapp->priority = 6;
	if (!GetHideMe()) entity_list.QueueClients(this, outapp, true);
	safe_delete(outapp);
	SetSpawned();
	if (GetPVP(false))	//force a PVP update until we fix the spawn struct
		SendAppearancePacket(AppearanceType::PVP, GetPVP(false), true, false);

	//Send AA Exp packet:
	if (GetLevel() >= 51)
		SendAlternateAdvancementStats();

	// Send exp packets
	outapp = new EQApplicationPacket(OP_ExpUpdate, sizeof(ExpUpdate_Struct));
	ExpUpdate_Struct* eu = (ExpUpdate_Struct*)outapp->pBuffer;
	uint32 tmpxp1 = GetEXPForLevel(GetLevel() + 1);
	uint32 tmpxp2 = GetEXPForLevel(GetLevel());

	// Crash bug fix... Divide by zero when tmpxp1 and 2 equalled each other, most likely the error case from GetEXPForLevel() (invalid class, etc)
	if (tmpxp1 != tmpxp2 && tmpxp1 != 0xFFFFFFFF && tmpxp2 != 0xFFFFFFFF) {
		float tmpxp = (float)((float)m_pp.exp - tmpxp2) / ((float)tmpxp1 - tmpxp2);
		eu->exp = (uint32)(330.0f * tmpxp);
		outapp->priority = 6;
		QueuePacket(outapp);
	}
	safe_delete(outapp);

	SendAlternateAdvancementTimers();

	outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(ZoneInSendName_Struct));
	ZoneInSendName_Struct* zonesendname = (ZoneInSendName_Struct*)outapp->pBuffer;
	strcpy(zonesendname->name, m_pp.name);
	strcpy(zonesendname->name2, m_pp.name);
	zonesendname->unknown0 = 0x0A;
	QueuePacket(outapp);
	safe_delete(outapp);

	if (IsInAGuild()) {
		guild_mgr.UpdateDbMemberOnline(CharacterID(), true);
		//SendGuildMembers();
		SendGuildURL();
		SendGuildChannel();
		if (RuleB(Guild, EnableLFGuild)) {
			SendGuildLFGuildStatus();
		}
	}
	if (RuleB(Guild, EnableLFGuild)) {
		SendLFGuildStatus();
	}

	//No idea why live sends this if even were not in a guild
	SendGuildMOTD();
}

void Client::SendLogoutPackets() {

	auto outapp = new EQApplicationPacket(OP_CancelTrade, sizeof(CancelTrade_Struct));
	CancelTrade_Struct* ct = (CancelTrade_Struct*) outapp->pBuffer;
	ct->fromid = GetID();
	ct->action = groupActUpdate;
	FastQueuePacket(&outapp);

	outapp = new EQApplicationPacket(OP_PreLogoutReply);
	FastQueuePacket(&outapp);

}

void Client::ReportConnectingState() {
	switch(conn_state) {
	case NoPacketsReceived:		//havent gotten anything
		LogDebug("Client has not sent us an initial zone entry packet");
		break;
	case ReceivedZoneEntry:		//got the first packet, loading up PP
		LogDebug("Client sent initial zone packet, but we never got their player info from the database");
		break;
	case PlayerProfileLoaded:	//our DB work is done, sending it
		LogDebug("We were sending the player profile, tributes, tasks, spawns, time and weather, but never finished");
		break;
	case ZoneInfoSent:		//includes PP, tributes, tasks, spawns, time and weather
		LogDebug("We successfully sent player info and spawns, waiting for client to request new zone");
		break;
	case NewZoneRequested:	//received and sent new zone request
		LogDebug("We received client's new zone request, waiting for client spawn request");
		break;
	case ClientSpawnRequested:	//client sent ReqClientSpawn
		LogDebug("We received the client spawn request, and were sending objects, doors, zone points and some other stuff, but never finished");
		break;
	case ZoneContentsSent:		//objects, doors, zone points
		LogDebug("The rest of the zone contents were successfully sent, waiting for client ready notification");
		break;
	case ClientReadyReceived:	//client told us its ready, send them a bunch of crap like guild MOTD, etc
		LogDebug("We received client ready notification, but never finished Client::CompleteConnect");
		break;
	case ClientConnectFinished:	//client finally moved to finished state, were done here
		LogDebug("Client is successfully connected");
		break;
	};
}

bool Client::SaveAA()
{
	std::vector<CharacterAlternateAbilitiesRepository::CharacterAlternateAbilities> v;

	uint32 aa_points_spent = 0;

	auto e = CharacterAlternateAbilitiesRepository::NewEntity();

	for (auto &rank : aa_ranks) {
		auto a = zone->GetAlternateAdvancementAbility(rank.first);
		if (!a) {
			continue;
		}

		if (rank.second.first > 0) {
			auto r = a->GetRankByPointsSpent(rank.second.first);
			if (!r) {
				continue;
			}

			aa_points_spent += r->total_cost;

			e.id       = character_id;
			e.aa_id    = a->first_rank_id;
			e.aa_value = rank.second.first;
			e.charges  = rank.second.second;

			v.emplace_back(e);
		}
	}

	m_pp.aapoints_spent = aa_points_spent + m_epp.expended_aa;

	if (v.empty()) {
		return true;
	}

	return CharacterAlternateAbilitiesRepository::ReplaceMany(database, v);
}

void Client::RemoveExpendedAA(int aa_id)
{
	CharacterAlternateAbilitiesRepository::DeleteWhere(
		database,
		fmt::format(
			"`id` = {} AND `aa_id` = {}",
			CharacterID(),
			aa_id
		)
	);
}

bool Client::Save(uint8 iCommitNow) {
	if(!ClientDataLoaded())
		return false;

	BenchTimer timer;

	/* Wrote current basics to PP for saves */
	if (!m_lock_save_position) {
		m_pp.x       = m_Position.x;
		m_pp.y       = m_Position.y;
		m_pp.z       = m_Position.z;
		m_pp.heading = m_Position.w;
	}

	m_pp.guildrank = guildrank;

	if (dead && GetHP() <= 0) {
		m_pp.cur_hp = GetMaxHP();
		m_pp.mana   = current_mana;
		if (RuleB(Character, FullManaOnDeath)) {
			m_pp.mana = GetMaxMana();
		}

		m_pp.endurance = current_endurance;
		if (RuleB(Character, FullEndurOnDeath)) {
			m_pp.endurance = GetMaxEndurance();
		}
	} else { 	// Otherwise, no changes.
		m_pp.cur_hp    = GetHP();
		m_pp.mana      = current_mana;
		m_pp.endurance = current_endurance;
	}

	/* Save Character Currency */
	database.SaveCharacterCurrency(CharacterID(), &m_pp);

	// save character binds
	// this may not need to be called in Save() but it's here for now
	// to maintain the current behavior
	database.SaveCharacterBinds(this);

	/* Save Character Buffs */
	database.SaveBuffs(this);

	/* Total Time Played */
	TotalSecondsPlayed += (time(nullptr) - m_pp.lastlogin);
	m_pp.timePlayedMin = (TotalSecondsPlayed / 60);
	m_pp.RestTimer = GetRestTimer();

	/* Save Mercs */
	if (GetMercInfo().MercTimerRemaining > RuleI(Mercs, UpkeepIntervalMS)) {
		GetMercInfo().MercTimerRemaining = RuleI(Mercs, UpkeepIntervalMS);
	}

	if (GetMercTimer()->Enabled()) {
		GetMercInfo().MercTimerRemaining = GetMercTimer()->GetRemainingTime();
	}

	if (dead || (!GetMerc() && !GetMercInfo().IsSuspended)) {
		memset(&m_mercinfo, 0, sizeof(struct MercInfo));
	}

	m_pp.lastlogin = time(nullptr);

	if (GetPet() && GetPet()->CastToNPC()->GetPetSpellID() && !dead) {
		NPC *pet = GetPet()->CastToNPC();
		m_petinfo.SpellID = pet->CastToNPC()->GetPetSpellID();
		m_petinfo.HP = pet->GetHP();
		m_petinfo.Mana = pet->GetMana();
		pet->GetPetState(m_petinfo.Buffs, m_petinfo.Items, m_petinfo.Name);
		m_petinfo.petpower = pet->GetPetPower();
		m_petinfo.size = pet->GetSize();
		m_petinfo.taunting = pet->CastToNPC()->IsTaunting();
	} else {
		memset(&m_petinfo, 0, sizeof(struct PetInfo));
	}
	database.SavePetInfo(this);

	if(tribute_timer.Enabled()) {
		m_pp.tribute_time_remaining = tribute_timer.GetRemainingTime();
	}
	else {
		m_pp.tribute_time_remaining = 0xFFFFFFFF; m_pp.tribute_active = 0;
	}

	if (m_pp.hunger_level < 0)
		m_pp.hunger_level = 0;

	if (m_pp.thirst_level < 0)
		m_pp.thirst_level = 0;

	p_timers.Store(&database);

	database.SaveCharacterTribute(this);
	SaveTaskState(); /* Save Character Task */

	LogFood("Client::Save - hunger_level: [{}] thirst_level: [{}]", m_pp.hunger_level, m_pp.thirst_level);

	// perform snapshot before SaveCharacterData() so that m_epp will contain the updated time
	if (RuleB(Character, ActiveInvSnapshots) && time(nullptr) >= GetNextInvSnapshotTime()) {
		if (database.SaveCharacterInvSnapshot(CharacterID())) {
			SetNextInvSnapshot(RuleI(Character, InvSnapshotMinIntervalM));
		}
		else {
			SetNextInvSnapshot(RuleI(Character, InvSnapshotMinRetryM));
		}
	}

	database.SaveCharacterData(this, &m_pp, &m_epp); /* Save Character Data */

	database.SaveCharacterEXPModifier(this);

	if (RuleB(Bots, Enabled)) {
		database.botdb.SaveBotSettings(this);
	}

	LogInfo("Save for [{}] took [{}]", GetCleanName(), timer.elapsed());

	return true;
}

CLIENTPACKET::CLIENTPACKET()
{
	app = nullptr;
	ack_req = false;
}

CLIENTPACKET::~CLIENTPACKET()
{
	safe_delete(app);
}

//this assumes we do not own pApp, and clones it.
bool Client::AddPacket(const EQApplicationPacket *pApp, bool bAckreq) {
	if (!pApp)
		return false;
	if(!zoneinpacket_timer.Enabled()) {
		//drop the packet because it will never get sent.
		return(false);
	}

	auto c = std::make_unique<CLIENTPACKET>();

	c->ack_req = bAckreq;
	c->app = pApp->Copy();

	clientpackets.push_back(std::move(c));
	return true;
}

//this assumes that it owns the object pointed to by *pApp
bool Client::AddPacket(EQApplicationPacket** pApp, bool bAckreq) {
	if (!pApp || !(*pApp))
		return false;
	if(!zoneinpacket_timer.Enabled()) {
		//drop the packet because it will never get sent.
		return(false);
	}
	auto c = std::make_unique<CLIENTPACKET>();

	c->ack_req = bAckreq;
	c->app = *pApp;
	*pApp = nullptr;

	clientpackets.push_back(std::move(c));
	return true;
}

bool Client::SendAllPackets() {
	CLIENTPACKET* cp = nullptr;
	while (!clientpackets.empty()) {
		cp = clientpackets.front().get();
		if(eqs)
			eqs->FastQueuePacket((EQApplicationPacket **)&cp->app, cp->ack_req);
		clientpackets.pop_front();
	}
	return true;
}

void Client::QueuePacket(const EQApplicationPacket* app, bool ack_req, CLIENT_CONN_STATUS required_state, eqFilterType filter) {
	if (filter != FilterNone && GetFilter(filter) == FilterHide) {
		return;
	}

	if (RuleB(Character, AutoIdleFilterPackets) && m_is_idle && IsFilteredAFKPacket(app)) {
		return;
	}

	if (client_state != CLIENT_CONNECTED && required_state == CLIENT_CONNECTED) {
		AddPacket(app, ack_req);
		return;
	}

	// if the program doesnt care about the status or if the status isnt what we requested
	if (required_state != CLIENT_CONNECTINGALL && client_state != required_state) {
		// todo: save packets for later use
		AddPacket(app, ack_req);
	}
	else if (eqs) {
		eqs->QueuePacket(app, ack_req);
	}
}

void Client::FastQueuePacket(EQApplicationPacket** app, bool ack_req, CLIENT_CONN_STATUS required_state) {
	// if the program doesnt care about the status or if the status isnt what we requested
	if (required_state != CLIENT_CONNECTINGALL && client_state != required_state) {
		// todo: save packets for later use
		AddPacket(app, ack_req);
		return;
	}
	else {
		if(eqs)
			eqs->FastQueuePacket((EQApplicationPacket **)app, ack_req);
		else if (app && (*app))
			delete *app;
		*app = nullptr;
	}
	return;
}

void Client::ChannelMessageReceived(uint8 chan_num, uint8 language, uint8 lang_skill, const char* orig_message, const char* targetname, bool is_silent) {
	char message[4096];
	strn0cpy(message, orig_message, sizeof(message));

	LogDebug("Client::ChannelMessageReceived() Channel:[{}] message:[{}]", chan_num, message);

	if (RuleB(Chat, AlwaysCaptureCommandText)) {
		if (message[0] == COMMAND_CHAR) {
			if (command_dispatch(this, message, false) == -2) {
				if (parse->PlayerHasQuestSub(EVENT_COMMAND)) {
					int i = parse->EventPlayer(EVENT_COMMAND, this, message, 0);
					if (i == 0 && !RuleB(Chat, SuppressCommandErrors)) {
						Message(Chat::Red, "Command '%s' not recognized.", message);
					}
				}
				else if (parse->PlayerHasQuestSub(EVENT_SAY)) {
					int i = parse->EventPlayer(EVENT_SAY, this, message, 0);
					if (i == 0 && !RuleB(Chat, SuppressCommandErrors)) {
						Message(Chat::Red, "Command '%s' not recognized.", message);
					}
				}
				else {
					if (!RuleB(Chat, SuppressCommandErrors)) {
						Message(Chat::Red, "Command '%s' not recognized.", message);
					}
				}
			}
			return;
		}

		if (message[0] == BOT_COMMAND_CHAR) {
			if (RuleB(Bots, Enabled)) {
				if (bot_command_dispatch(this, message) == -2) {
					if (parse->PlayerHasQuestSub(EVENT_BOT_COMMAND)) {
						int i = parse->EventPlayer(EVENT_BOT_COMMAND, this, message, 0);
						if (i == 0 && !RuleB(Chat, SuppressCommandErrors)) {
							Message(Chat::Red, "Bot command '%s' not recognized.", message);
						}
					}
					else if (parse->PlayerHasQuestSub(EVENT_SAY)) {
						int i = parse->EventPlayer(EVENT_SAY, this, message, 0);
						if (i == 0 && !RuleB(Chat, SuppressCommandErrors)) {
							Message(Chat::Red, "Bot command '%s' not recognized.", message);
						}
					}
					else {
						if (!RuleB(Chat, SuppressCommandErrors)) {
							Message(Chat::Red, "Bot command '%s' not recognized.", message);
						}
					}
				}
			} else {
				Message(Chat::Red, "Bots are disabled on this server.");
			}
			return;
		}
	}

	if (targetname == nullptr) {
		targetname = (!GetTarget()) ? "" : GetTarget()->GetName();
	}

	if(RuleB(Chat, EnableAntiSpam))
	{
		if(strcmp(targetname, "discard") != 0)
		{
			if(chan_num == ChatChannel_Shout || chan_num == ChatChannel_Auction || chan_num == ChatChannel_OOC || chan_num == ChatChannel_Tell)
			{
				if(GlobalChatLimiterTimer)
				{
					if(GlobalChatLimiterTimer->Check(false))
					{
						GlobalChatLimiterTimer->Start(RuleI(Chat, IntervalDurationMS));
						AttemptedMessages = 0;
					}
				}

				uint32 AllowedMessages = RuleI(Chat, MinimumMessagesPerInterval) + TotalKarma;
				AllowedMessages = AllowedMessages > RuleI(Chat, MaximumMessagesPerInterval) ? RuleI(Chat, MaximumMessagesPerInterval) : AllowedMessages;

				if(RuleI(Chat, MinStatusToBypassAntiSpam) <= Admin())
					AllowedMessages = 10000;

				AttemptedMessages++;
				if(AttemptedMessages > AllowedMessages)
				{
					if(AttemptedMessages > RuleI(Chat, MaxMessagesBeforeKick))
					{
						Kick("Sent too many chat messages at once.");
						return;
					}
					if(GlobalChatLimiterTimer)
					{
						Message(0, "You have been rate limited, you can send more messages in %i seconds.",
							GlobalChatLimiterTimer->GetRemainingTime() / 1000);
						return;
					}
					else
					{
						Message(0, "You have been rate limited, you can send more messages in 60 seconds.");
						return;
					}
				}
			}
		}
	}

	if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::EventType::SPEECH)) {
		PlayerEvent::PlayerSpeech e{};
		std::string msg = message;
		if (!msg.empty() && msg.at(0) != '#' && msg.at(0) != '^') {
			e.message    = message;
			e.min_status = Admin();
			e.type       = chan_num;
			e.to         = targetname;
			e.from       = GetCleanName();
			if (chan_num == ChatChannel_Guild) {
				e.guild_id = GuildID();
			}

			RecordPlayerEventLog(PlayerEvent::SPEECH, e);
		}
	}

	// Garble the message based on drunkness
	if (GetIntoxication() > 0 && !(RuleB(Chat, ServerWideOOC) && chan_num == ChatChannel_OOC) && !GetGM()) {
		GarbleMessage(message, (int)(GetIntoxication() / 3));
		language   = Language::CommonTongue; // No need for language when drunk
		lang_skill = Language::MaxValue;
	}

	// some channels don't use languages
	if (
		chan_num == ChatChannel_OOC ||
		chan_num == ChatChannel_GMSAY ||
		chan_num == ChatChannel_Broadcast ||
		chan_num == ChatChannel_Petition
	) {
		language   = Language::CommonTongue;
		lang_skill = Language::MaxValue;
	}

	// Censor the message
	if (EQ::ProfanityManager::IsCensorshipActive() && (chan_num != ChatChannel_Say))
		EQ::ProfanityManager::RedactMessage(message);

	switch(chan_num)
	{
	case ChatChannel_Guild: { /* Guild Chat */
		if (!IsInAGuild()) {
			MessageString(Chat::DefaultText, GUILD_NOT_MEMBER2);	//You are not a member of any guild.
		} else if (!guild_mgr.CheckPermission(GuildID(), GuildRank(), GUILD_ACTION_GUILD_CHAT_SPEAK_IN)) {
			MessageString(Chat::EchoGuild, NO_PROPER_ACCESS);
		} else if (!worldserver.SendChannelMessage(this, targetname, chan_num, GuildID(), language, lang_skill, message)) {
			Message(Chat::White, "Error: World server disconnected");
		}
		break;
	}
	case ChatChannel_Group: { /* Group Chat */
		Raid* raid = entity_list.GetRaidByClient(this);
		if(raid) {
			raid->RaidGroupSay((const char*) message, this, language, lang_skill);
			break;
		}

		Group* group = GetGroup();
		if(group != nullptr) {
			group->GroupMessage(this,language,lang_skill,(const char*) message);
		}
		break;
	}
	case ChatChannel_Raid: { /* Raid Say */
		Raid* raid = entity_list.GetRaidByClient(this);
		if(raid){
			raid->RaidSay((const char*) message, this, language, lang_skill);
		}
		break;
	}
	case ChatChannel_Shout: { /* Shout */
		Mob *sender = this;
		if (GetPet() && GetTarget() == GetPet() && GetPet()->FindType(SpellEffect::VoiceGraft))
			sender = GetPet();

		entity_list.ChannelMessage(sender, chan_num, language, lang_skill, message);
		break;
	}
	case ChatChannel_Auction: { /* Auction */
		if(RuleB(Chat, ServerWideAuction))
		{
			if(!global_channel_timer.Check())
			{
				if(strlen(targetname) == 0)
					ChannelMessageReceived(chan_num, language, lang_skill, message, "discard"); //Fast typer or spammer??
				else
					return;
			}

			if(GetRevoked())
			{
				Message(0, "You have been revoked. You may not talk on Auction.");
				return;
			}

			if(TotalKarma < RuleI(Chat, KarmaGlobalChatLimit))
			{
				if(GetLevel() < RuleI(Chat, GlobalChatLevelLimit))
				{
					Message(0, "You do not have permission to talk in Auction at this time.");
					return;
				}
			}

			if (!worldserver.SendChannelMessage(this, 0, chan_num, 0, language, lang_skill, message))
			Message(0, "Error: World server disconnected");
		}
		else if(!RuleB(Chat, ServerWideAuction)) {
			Mob *sender = this;

			if (GetPet() && GetTarget() == GetPet() && GetPet()->FindType(SpellEffect::VoiceGraft))
			sender = GetPet();

			entity_list.ChannelMessage(sender, chan_num, language, lang_skill, message);
		}
		break;
	}
	case ChatChannel_OOC: { /* OOC */
		if(RuleB(Chat, ServerWideOOC))
		{
			if(!global_channel_timer.Check())
			{
				if(strlen(targetname) == 0)
					ChannelMessageReceived(chan_num, language, lang_skill, message, "discard"); //Fast typer or spammer??
				else
					return;
			}
			if(worldserver.IsOOCMuted() && admin < AccountStatus::GMAdmin)
			{
				Message(0,"OOC has been muted. Try again later.");
				return;
			}

			if(GetRevoked())
			{
				Message(0, "You have been revoked. You may not talk on OOC.");
				return;
			}

			if(TotalKarma < RuleI(Chat, KarmaGlobalChatLimit))
			{
				if(GetLevel() < RuleI(Chat, GlobalChatLevelLimit))
				{
					Message(0, "You do not have permission to talk in OOC at this time.");
					return;
				}
			}

			if (!worldserver.SendChannelMessage(this, 0, chan_num, 0, language, lang_skill, message))
			{
				Message(0, "Error: World server disconnected");
			}
		}
		else if(!RuleB(Chat, ServerWideOOC))
		{
			Mob *sender = this;

			if (GetPet() && GetTarget() == GetPet() && GetPet()->FindType(SpellEffect::VoiceGraft))
				sender = GetPet();

			entity_list.ChannelMessage(sender, chan_num, language, lang_skill, message);
		}
		break;
	}
	case ChatChannel_Broadcast: /* Broadcast */
	case ChatChannel_GMSAY: { /* GM Say */
		if (!(admin >= AccountStatus::QuestTroupe))
			Message(0, "Error: Only GMs can use this channel");
		else if (!worldserver.SendChannelMessage(this, targetname, chan_num, 0, language, lang_skill, message))
			Message(0, "Error: World server disconnected");
		break;
	}
	case ChatChannel_Tell: { /* Tell */
			if(!global_channel_timer.Check())
			{
				if(strlen(targetname) == 0)
					ChannelMessageReceived(ChatChannel_Tell, language, lang_skill, message, "discard"); //Fast typer or spammer??
				else
					return;
			}

			if(GetRevoked())
			{
				Message(0, "You have been revoked. You may not send tells.");
				return;
			}

			if(TotalKarma < RuleI(Chat, KarmaGlobalChatLimit))
			{
				if(GetLevel() < RuleI(Chat, GlobalChatLevelLimit))
				{
					Message(0, "You do not have permission to send tells at this time.");
					return;
				}
			}

			char target_name[64];

			if(targetname)
			{
				size_t i = strlen(targetname);
				int x;
				for(x = 0; x < i; ++x)
				{
					if(targetname[x] == '%')
					{
						target_name[x] = '/';
					}
					else
					{
						target_name[x] = targetname[x];
					}
				}
				target_name[x] = '\0';
			}

			if(!worldserver.SendChannelMessage(this, target_name, chan_num, 0, language, lang_skill, message))
				Message(0, "Error: World server disconnected");
		break;
	}
	case ChatChannel_Say: { /* Say */
		if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::SAY)) {
			std::string msg = message;
			if (!msg.empty() && msg.at(0) != '#' && msg.at(0) != '^') {
				auto e = PlayerEvent::SayEvent{
					.message = message,
					.target = GetTarget() ? GetTarget()->GetCleanName() : ""
				};
				RecordPlayerEventLog(PlayerEvent::SAY, e);
			}
		}

		if (message[0] == COMMAND_CHAR) {
			if (command_dispatch(this, message, false) == -2) {
				if (parse->PlayerHasQuestSub(EVENT_COMMAND)) {
					int i = parse->EventPlayer(EVENT_COMMAND, this, message, 0);
					if (i == 0 && !RuleB(Chat, SuppressCommandErrors)) {
						Message(Chat::Red, "Command '%s' not recognized.", message);
					}
				}
				else if (parse->PlayerHasQuestSub(EVENT_SAY)) {
					int i = parse->EventPlayer(EVENT_SAY, this, message, 0);
					if (i == 0 && !RuleB(Chat, SuppressCommandErrors)) {
						Message(Chat::Red, "Command '%s' not recognized.", message);
					}
				}
				else {
					if (!RuleB(Chat, SuppressCommandErrors)) {
						Message(Chat::Red, "Command '%s' not recognized.", message);
					}
				}
			}
			break;
		}

		if (message[0] == BOT_COMMAND_CHAR) {
			if (RuleB(Bots, Enabled)) {
				if (bot_command_dispatch(this, message) == -2) {
					if (parse->PlayerHasQuestSub(EVENT_BOT_COMMAND)) {
						int i = parse->EventPlayer(EVENT_BOT_COMMAND, this, message, 0);
						if (i == 0 && !RuleB(Chat, SuppressCommandErrors)) {
							Message(Chat::Red, "Bot command '%s' not recognized.", message);
						}
					}
					else if (parse->PlayerHasQuestSub(EVENT_SAY)) {
						int i = parse->EventPlayer(EVENT_SAY, this, message, 0);
						if (i == 0 && !RuleB(Chat, SuppressCommandErrors)) {
							Message(Chat::Red, "Bot command '%s' not recognized.", message);
						}
					}
					else {
						if (!RuleB(Chat, SuppressCommandErrors)) {
							Message(Chat::Red, "Bot command '%s' not recognized.", message);
						}
					}
				}
			} else {
				Message(Chat::Red, "Bots are disabled on this server.");
			}
			break;
		}

		if (EQ::ProfanityManager::IsCensorshipActive()) {
			EQ::ProfanityManager::RedactMessage(message);
		}

		Mob* sender = this;
		if (GetPet() && GetTarget() == GetPet() && GetPet()->FindType(SpellEffect::VoiceGraft)) {
			sender = GetPet();
		}

		if (!is_silent) {
			entity_list.ChannelMessage(sender, chan_num, language, lang_skill, message);
		}

		if (parse->PlayerHasQuestSub(EVENT_SAY)) {
			parse->EventPlayer(EVENT_SAY, this, message, language);
		}

		if (sender != this) {
			break;
		}

		if (quest_manager.ProximitySayInUse()) {
			entity_list.ProcessProximitySay(message, this, language);
		}

		Mob* t = GetTarget();

		if (
			t &&
			!IsInvisible(t) &&
			DistanceNoZ(m_Position, t->GetPosition()) <= RuleI(Range, Say)
		) {
			const bool is_engaged = t->IsEngaged();

			if (is_engaged) {
				parse->EventBotMercNPC(EVENT_AGGRO_SAY, t, this, [&]() { return message; }, language);
			} else {
				parse->EventBotMercNPC(EVENT_SAY, t, this, [&]() { return message; }, language);
			}

			if (t->IsNPC() && !is_engaged) {
				CheckLDoNHail(t->CastToNPC());
				CheckEmoteHail(t->CastToNPC(), message);

				if (RuleB(TaskSystem, EnableTaskSystem)) {
					if (UpdateTasksOnSpeakWith(t->CastToNPC())) {
						t->CastToNPC()->DoQuestPause(this);
					}
				}
			}
		}
		break;
	}
	case ChatChannel_UCSRelay:
	{
		// UCS Relay for Underfoot and later.
		if(!worldserver.SendChannelMessage(this, 0, chan_num, 0, language, lang_skill, message))
			Message(0, "Error: World server disconnected");
		break;
	}
	case ChatChannel_Emotes:
	{
		// Emotes for Underfoot and later.
		// crash protection -- cheater
		message[1023] = '\0';
		size_t msg_len = strlen(message);
		if (msg_len > 512)
			message[512] = '\0';

		auto outapp = new EQApplicationPacket(OP_Emote, 4 + msg_len + strlen(GetName()) + 2);
		Emote_Struct* es = (Emote_Struct*)outapp->pBuffer;
		char *Buffer = (char *)es;
		Buffer += 4;
		snprintf(Buffer, sizeof(Emote_Struct) - 4, "%s %s", GetName(), message);
		entity_list.QueueCloseClients(this, outapp, true, RuleI(Range, Emote), 0, true, FilterSocials);
		safe_delete(outapp);
		break;
	}
	default: {
		Message(0, "Channel (%i) not implemented", (uint16)chan_num);
	}
	}
}

void Client::ChannelMessageSend(
	const char *from,
	const char *to,
	uint8 channel_id,
	uint8 language_id,
	uint8 language_skill,
	const char *message,
	...
)
{
	if (
		(channel_id == ChatChannel_Petition && Admin() < AccountStatus::QuestTroupe) ||
		(channel_id == ChatChannel_GMSAY && !GetGM())
	) {
		return;
	}

	va_list argptr;
	char    buffer[4096];
	char    message_sender[64];

	va_start(argptr, message);
	vsnprintf(buffer, 4096, message, argptr);
	va_end(argptr);

	EQApplicationPacket app(OP_ChannelMessage, sizeof(ChannelMessage_Struct) + strlen(buffer) + 1);

	auto* cm = (ChannelMessage_Struct *) app.pBuffer;

	if (from == 0) {
		strcpy(cm->sender, "ZServer");
	} else if (from[0] == 0) {
		strcpy(cm->sender, "ZServer");
	} else {
		CleanMobName(from, message_sender);
		strcpy(cm->sender, message_sender);
	}

	if (to != 0) {
		strcpy((char *) cm->targetname, to);
	} else if (channel_id == ChatChannel_Tell) {
		strcpy(cm->targetname, m_pp.name);
	} else {
		cm->targetname[0] = 0;
	}

	uint8 listener_skill;

	const bool is_valid_language = EQ::ValueWithin(language_id, Language::CommonTongue, Language::Unknown27);

	if (is_valid_language) {
		listener_skill = m_pp.languages[language_id];
		cm->language   = listener_skill < 24 ? Language::Unknown27 : language_id;
	} else {
		listener_skill = m_pp.languages[Language::CommonTongue];
		cm->language   = Language::CommonTongue;
	}

	// set effective language skill = lower of sender and receiver skills
	uint8 effective_skill = (language_skill < listener_skill ? language_skill : listener_skill);
	if (effective_skill > Language::MaxValue) {
		effective_skill = Language::MaxValue;
	}

	cm->skill_in_language = effective_skill;

	cm->chan_num = channel_id;
	strcpy(&cm->message[0], buffer);

	QueuePacket(&app);

	const bool can_train_self = RuleB(Client, SelfLanguageLearning);
	const bool is_not_sender  = strcmp(GetCleanName(), cm->sender);

	if (can_train_self || is_not_sender) {
		if (
			channel_id == ChatChannel_Group &&
			listener_skill < Language::MaxValue
		) { // group message in non-mastered language, check for skill up
			if (is_valid_language && m_pp.languages[language_id] <= language_skill) {
				CheckLanguageSkillIncrease(language_id, language_skill);
			}
		}
	}
}

void Client::Message(uint32 type, const char* message, ...) {
	if (GetFilter(FilterSpellDamage) == FilterHide && type == Chat::NonMelee)
		return;
	if (GetFilter(FilterMeleeCrits) == FilterHide && type == Chat::MeleeCrit) //98 is self...
		return;
	if (GetFilter(FilterSpellCrits) == FilterHide && type == Chat::SpellCrit)
		return;

	va_list argptr;
	auto buffer = new char[4096];
	va_start(argptr, message);
	vsnprintf(buffer, 4096, message, argptr);
	va_end(argptr);

	SerializeBuffer buf(sizeof(SpecialMesgHeader_Struct) + 12 + 64 + 64);
	buf.WriteInt8(static_cast<int8>(Journal::SpeakMode::Raw));
	buf.WriteInt8(static_cast<int8>(Journal::Mode::None));
	buf.WriteInt8(0); // language
	buf.WriteUInt32(type);
	buf.WriteUInt32(0); // target spawn ID used for journal filtering, ignored here
	buf.WriteString(""); // send name, not applicable here
	buf.WriteInt32(0); // location, client seems to ignore
	buf.WriteInt32(0);
	buf.WriteInt32(0);
	buf.WriteString(buffer);

	auto app = new EQApplicationPacket(OP_SpecialMesg, buf);

	FastQueuePacket(&app);

	safe_delete_array(buffer);
}

void Client::FilteredMessage(Mob *sender, uint32 type, eqFilterType filter, const char* message, ...) {
	if (!FilteredMessageCheck(sender, filter))
		return;

	va_list argptr;
	auto buffer = new char[4096];
	va_start(argptr, message);
	vsnprintf(buffer, 4096, message, argptr);
	va_end(argptr);

	SerializeBuffer buf(sizeof(SpecialMesgHeader_Struct) + 12 + 64 + 64);
	buf.WriteInt8(static_cast<int8>(Journal::SpeakMode::Raw));
	buf.WriteInt8(static_cast<int8>(Journal::Mode::None));
	buf.WriteInt8(0); // language
	buf.WriteUInt32(type);
	buf.WriteUInt32(0); // target spawn ID used for journal filtering, ignored here
	buf.WriteString(""); // send name, not applicable here
	buf.WriteInt32(0); // location, client seems to ignore
	buf.WriteInt32(0);
	buf.WriteInt32(0);
	buf.WriteString(buffer);

	auto app = new EQApplicationPacket(OP_SpecialMesg, buf);

	FastQueuePacket(&app);

	safe_delete_array(buffer);
}

void Client::SetMaxHP() {
	if(dead)
		return;
	SetHP(CalcMaxHP());
	SendHPUpdate();
	Save();
}

bool Client::UpdateLDoNPoints(uint32 theme_id, int points)
{
	if (points < 0) {
		if (m_pp.ldon_points_available < (0 - points)) {
			return false;
		}
	}

	bool is_loss = false;

	switch (theme_id) {
		case LDoNTheme::Unused: { // No theme, so distribute evenly across all
			int split_points = (points / 5);

			int guk_points = (split_points + (points % 5));
			int mir_points = split_points;
			int mmc_points = split_points;
			int ruj_points = split_points;
			int tak_points = split_points;

			split_points = 0;

			if (points < 0) {
				if (m_pp.ldon_points_available < (0 - points)) {
					return false;
				}

				is_loss = true;

				if (m_pp.ldon_points_guk < (0 - guk_points)) {
					mir_points += (guk_points + m_pp.ldon_points_guk);
					guk_points = (0 - m_pp.ldon_points_guk);
				}

				if (m_pp.ldon_points_mir < (0 - mir_points)) {
					mmc_points += (mir_points + m_pp.ldon_points_mir);
					mir_points = (0 - m_pp.ldon_points_mir);
				}

				if (m_pp.ldon_points_mmc < (0 - mmc_points)) {
					ruj_points += (mmc_points + m_pp.ldon_points_mmc);
					mmc_points = (0 - m_pp.ldon_points_mmc);
				}

				if (m_pp.ldon_points_ruj < (0 - ruj_points)) {
					tak_points += (ruj_points + m_pp.ldon_points_ruj);
					ruj_points = (0 - m_pp.ldon_points_ruj);
				}

				if (m_pp.ldon_points_tak < (0 - tak_points)) {
					split_points = (tak_points + m_pp.ldon_points_tak);
					tak_points   = (0 - m_pp.ldon_points_tak);
				}
			}

			m_pp.ldon_points_guk += guk_points;
			m_pp.ldon_points_mir += mir_points;
			m_pp.ldon_points_mmc += mmc_points;
			m_pp.ldon_points_ruj += ruj_points;
			m_pp.ldon_points_tak += tak_points;

			points -= split_points;

			if (split_points != 0) { // if anything left, recursively loop thru again
				UpdateLDoNPoints(LDoNTheme::Unused, split_points);
			}

			break;
		}
		case LDoNTheme::GUK: {
			if (points < 0) {
				if (m_pp.ldon_points_guk < (0 - points)) {
					return false;
				}

				is_loss = true;
			}

			m_pp.ldon_points_guk += points;
			break;
		}
		case LDoNTheme::MIR: {
			if (points < 0) {
				if (m_pp.ldon_points_mir < (0 - points)) {
					return false;
				}

				is_loss = true;
			}

			m_pp.ldon_points_mir += points;
			break;
		}
		case LDoNTheme::MMC: {
			if (points < 0) {
				if (m_pp.ldon_points_mmc < (0 - points)) {
					return false;
				}

				is_loss = true;
			}

			m_pp.ldon_points_mmc += points;
			break;
		}
		case LDoNTheme::RUJ: {
			if (points < 0) {
				if (m_pp.ldon_points_ruj < (0 - points)) {
					return false;
				}

				is_loss = true;
			}

			m_pp.ldon_points_ruj += points;
			break;
		}
		case LDoNTheme::TAK: {
			if (points < 0) {
				if (m_pp.ldon_points_tak < (0 - points)) {
					return false;
				}

				is_loss = true;
			}

			m_pp.ldon_points_tak += points;
			break;
		}
	}

	m_pp.ldon_points_available += points;

	QuestEventID event_id = is_loss ? EVENT_LDON_POINTS_LOSS : EVENT_LDON_POINTS_GAIN;

	if (parse->PlayerHasQuestSub(event_id)) {
		const std::string &export_string = fmt::format(
			"{} {}",
			theme_id,
			std::abs(points)
		);

		parse->EventPlayer(event_id, this, export_string, 0);
	}

	auto outapp = new EQApplicationPacket(OP_AdventurePointsUpdate, sizeof(AdventurePoints_Update_Struct));
	auto *apus = (AdventurePoints_Update_Struct *) outapp->pBuffer;

	apus->ldon_available_points = m_pp.ldon_points_available;
	apus->ldon_guk_points       = m_pp.ldon_points_guk;
	apus->ldon_mirugal_points   = m_pp.ldon_points_mir;
	apus->ldon_mistmoore_points = m_pp.ldon_points_mmc;
	apus->ldon_rujarkian_points = m_pp.ldon_points_ruj;
	apus->ldon_takish_points    = m_pp.ldon_points_tak;

	outapp->priority = 6;

	QueuePacket(outapp);
	safe_delete(outapp);
	return true;
}

void Client::SetLDoNPoints(uint32 theme_id, uint32 points)
{
	switch (theme_id) {
		case LDoNTheme::GUK: {
			m_pp.ldon_points_guk = points;
			break;
		}
		case LDoNTheme::MIR: {
			m_pp.ldon_points_mir = points;
			break;
		}
		case LDoNTheme::MMC: {
			m_pp.ldon_points_mmc = points;
			break;
		}
		case LDoNTheme::RUJ: {
			m_pp.ldon_points_ruj = points;
			break;
		}
		case LDoNTheme::TAK: {
			m_pp.ldon_points_tak = points;
			break;
		}
	}

	m_pp.ldon_points_available = (
		m_pp.ldon_points_guk +
		m_pp.ldon_points_mir +
		m_pp.ldon_points_mmc +
		m_pp.ldon_points_ruj +
		m_pp.ldon_points_tak
	);

	auto outapp = new EQApplicationPacket(OP_AdventurePointsUpdate, sizeof(AdventurePoints_Update_Struct));

	auto a = (AdventurePoints_Update_Struct*) outapp->pBuffer;

	a->ldon_available_points = m_pp.ldon_points_available;
	a->ldon_guk_points       = m_pp.ldon_points_guk;
	a->ldon_mirugal_points   = m_pp.ldon_points_mir;
	a->ldon_mistmoore_points = m_pp.ldon_points_mmc;
	a->ldon_rujarkian_points = m_pp.ldon_points_ruj;
	a->ldon_takish_points    = m_pp.ldon_points_tak;

	outapp->priority = 6;

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SetSkill(EQ::skills::SkillType skillid, uint16 value) {
	if (skillid > EQ::skills::HIGHEST_SKILL)
		return;
	m_pp.skills[skillid] = value; // We need to be able to #setskill 254 and 255 to reset skills

	database.SaveCharacterSkill(CharacterID(), skillid, value);
	auto outapp = new EQApplicationPacket(OP_SkillUpdate, sizeof(SkillUpdate_Struct));
	SkillUpdate_Struct* skill = (SkillUpdate_Struct*)outapp->pBuffer;
	skill->skillId=skillid;
	skill->value=value;
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::IncreaseLanguageSkill(uint8 language_id, uint8 increase)
{
	if (!EQ::ValueWithin(language_id, Language::CommonTongue, Language::Unknown27)) {
		return;
	}

	m_pp.languages[language_id] += increase;

	if (m_pp.languages[language_id] > Language::MaxValue) {
		m_pp.languages[language_id] = Language::MaxValue;
	}

	database.SaveCharacterLanguage(CharacterID(), language_id, m_pp.languages[language_id]);

	auto outapp = new EQApplicationPacket(OP_SkillUpdate, sizeof(SkillUpdate_Struct));
	auto* s = (SkillUpdate_Struct*) outapp->pBuffer;

	s->skillId = 100 + language_id;
	s->value   = m_pp.languages[language_id];

	QueuePacket(outapp);
	safe_delete(outapp);

	MessageString(Chat::Skills, LANG_SKILL_IMPROVED);
}

void Client::AddSkill(EQ::skills::SkillType skillid, uint16 value) {
	if (skillid > EQ::skills::HIGHEST_SKILL)
		return;
	value = GetRawSkill(skillid) + value;
	uint16 max = GetMaxSkillAfterSpecializationRules(skillid, MaxSkill(skillid));
	if (value > max)
		value = max;
	SetSkill(skillid, value);
}

void Client::SendSound(){//Makes a sound.
	auto outapp = new EQApplicationPacket(OP_Sound, 68);
	unsigned char x[68];
	memset(x, 0, 68);
	x[0]=0x22;
	memset(&x[4],0x8002,sizeof(uint16));
	memset(&x[8],0x8624,sizeof(uint16));
	memset(&x[12],0x4A01,sizeof(uint16));
	x[16]=0x05;
	x[28]=0x00;//change this value to give gold to the client
	memset(&x[40],0xFFFFFFFF,sizeof(uint32));
	memset(&x[44],0xFFFFFFFF,sizeof(uint32));
	memset(&x[48],0xFFFFFFFF,sizeof(uint32));
	memset(&x[52],0xFFFFFFFF,sizeof(uint32));
	memset(&x[56],0xFFFFFFFF,sizeof(uint32));
	memset(&x[60],0xFFFFFFFF,sizeof(uint32));
	memset(&x[64],0xffffffff,sizeof(uint32));
	memcpy(outapp->pBuffer,x,outapp->size);
	QueuePacket(outapp);
	safe_delete(outapp);

}
void Client::UpdateWho(uint8 remove)
{
	if (account_id == 0) {
		return;
	}
	if (!worldserver.Connected()) {
		return;
	}

	auto pack = new ServerPacket(ServerOP_ClientList, sizeof(ServerClientList_Struct));
	auto *s   = (ServerClientList_Struct *) pack->pBuffer;
	s->remove = remove;
	s->wid    = GetWID();
	s->IP     = GetIP();
	s->charid = CharacterID();
	strcpy(s->name, GetName());

	s->gm        = GetGM();
	s->Admin     = Admin();
	s->AccountID = AccountID();
	strcpy(s->AccountName, AccountName());

	s->LSAccountID = LSAccountID();
	strn0cpy(s->lskey, lskey, sizeof(s->lskey));

	s->zone        = zone->GetZoneID();
	s->instance_id = zone->GetInstanceID();
	s->race        = GetRace();
	s->class_      = GetClass();
	s->level       = GetLevel();

	if (m_pp.anon == 0) {
		s->anon = 0;
	}
	else if (m_pp.anon == 1) {
		s->anon = 1;
	}
	else if (m_pp.anon >= 2) {
		s->anon = 2;
	}

	s->ClientVersion = static_cast<unsigned int>(ClientVersion());
	s->tellsoff      = tellsoff;
	s->guild_id      = guild_id;
	s->guild_rank    = guildrank;
	s->guild_tribute_opt_in = guild_tribute_opt_in;
	s->LFG           = LFG;
	if (LFG) {
		s->LFGFromLevel   = LFGFromLevel;
		s->LFGToLevel     = LFGToLevel;
		s->LFGMatchFilter = LFGMatchFilter;
		memcpy(s->LFGComments, LFGComments, sizeof(s->LFGComments));
	}

	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Client::WhoAll(Who_All_Struct* whom) {

	if (!worldserver.Connected())
		Message(0, "Error: World server disconnected");
	else {
		auto pack = new ServerPacket(ServerOP_Who, sizeof(ServerWhoAll_Struct));
		ServerWhoAll_Struct* whoall = (ServerWhoAll_Struct*) pack->pBuffer;
		whoall->admin = Admin();
		whoall->fromid=GetID();
		strcpy(whoall->from, GetName());
		strn0cpy(whoall->whom, whom->whom, 64);
		whoall->lvllow = whom->lvllow;
		whoall->lvlhigh = whom->lvlhigh;
		whoall->gmlookup = whom->gmlookup;
		whoall->wclass = whom->wclass;
		whoall->wrace = whom->wrace;
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void Client::FriendsWho(char *FriendsString) {

	if (!worldserver.Connected())
		Message(0, "Error: World server disconnected");
	else {
		auto pack =
			new ServerPacket(ServerOP_FriendsWho, sizeof(ServerFriendsWho_Struct) + strlen(FriendsString));
		ServerFriendsWho_Struct* FriendsWho = (ServerFriendsWho_Struct*) pack->pBuffer;
		FriendsWho->FromID = GetID();
		strcpy(FriendsWho->FromName, GetName());
		strcpy(FriendsWho->FriendsString, FriendsString);
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void Client::UpdateAdmin(bool from_database) {
	int16 tmp = admin;
	if (from_database) {
		admin = database.GetAccountStatus(account_id);
	}

	if (tmp == admin && from_database) {
		return;
	}

	if (m_pp.gm) {
		LogInfo("[{}] - [{}] is a GM", __FUNCTION__ , GetName());
		PetitionList::Instance()->UpdateGMQueue();
	}

	UpdateWho();
}

void Client::SetStats(uint8 type,int16 set_val){
	if(type>STAT_DISEASE){
		printf("Error in Client::IncStats, received invalid type of: %i\n",type);
		return;
	}
	auto outapp = new EQApplicationPacket(OP_IncreaseStats, sizeof(IncreaseStat_Struct));
	IncreaseStat_Struct* iss=(IncreaseStat_Struct*)outapp->pBuffer;
	switch(type){
		case STAT_STR:
			if(set_val>0)
				iss->str=set_val;
			if(set_val<0)
				m_pp.STR=0;
			else if(set_val>255)
				m_pp.STR=255;
			else
				m_pp.STR=set_val;
			break;
		case STAT_STA:
			if(set_val>0)
				iss->sta=set_val;
			if(set_val<0)
				m_pp.STA=0;
			else if(set_val>255)
				m_pp.STA=255;
			else
				m_pp.STA=set_val;
			break;
		case STAT_AGI:
			if(set_val>0)
				iss->agi=set_val;
			if(set_val<0)
				m_pp.AGI=0;
			else if(set_val>255)
				m_pp.AGI=255;
			else
				m_pp.AGI=set_val;
			break;
		case STAT_DEX:
			if(set_val>0)
				iss->dex=set_val;
			if(set_val<0)
				m_pp.DEX=0;
			else if(set_val>255)
				m_pp.DEX=255;
			else
				m_pp.DEX=set_val;
			break;
		case STAT_INT:
			if(set_val>0)
				iss->int_=set_val;
			if(set_val<0)
				m_pp.INT=0;
			else if(set_val>255)
				m_pp.INT=255;
			else
				m_pp.INT=set_val;
			break;
		case STAT_WIS:
			if(set_val>0)
				iss->wis=set_val;
			if(set_val<0)
				m_pp.WIS=0;
			else if(set_val>255)
				m_pp.WIS=255;
			else
				m_pp.WIS=set_val;
			break;
		case STAT_CHA:
			if(set_val>0)
				iss->cha=set_val;
			if(set_val<0)
				m_pp.CHA=0;
			else if(set_val>255)
				m_pp.CHA=255;
			else
				m_pp.CHA=set_val;
		break;
	}
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::IncStats(uint8 type, int16 increase_val)
{
	if (type > STAT_DISEASE) {
		printf("Error in Client::IncStats, received invalid type of: %i\n", type);
		return;
	}
	auto                outapp = new EQApplicationPacket(OP_IncreaseStats, sizeof(IncreaseStat_Struct));
	IncreaseStat_Struct *iss   = (IncreaseStat_Struct *) outapp->pBuffer;
	switch (type) {
		case STAT_STR:
			if (increase_val > 0) {
				iss->str = increase_val;
			}

			if ((m_pp.STR + increase_val * 2) > 255) {
				m_pp.STR = 255;
			} else {
				m_pp.STR += increase_val * 2;
			}
			break;
		case STAT_STA:
			if (increase_val > 0) {
				iss->sta = increase_val;
			}

			if ((m_pp.STA + increase_val * 2) > 255) {
				m_pp.STA = 255;
			} else {
				m_pp.STA += increase_val * 2;
			}
			break;
		case STAT_AGI:
			if (increase_val > 0) {
				iss->agi = increase_val;
			}
			if ((m_pp.AGI + increase_val * 2) > 255) {
				m_pp.AGI = 255;
			} else {
				m_pp.AGI += increase_val * 2;
			}
			break;
		case STAT_DEX:
			if (increase_val > 0) {
				iss->dex = increase_val;
			}

			if ((m_pp.DEX + increase_val * 2) > 255) {
				m_pp.DEX = 255;
			} else {
				m_pp.DEX += increase_val * 2;
			}
			break;
		case STAT_INT:
			if (increase_val > 0) {
				iss->int_ = increase_val;
			}

			if ((m_pp.INT + increase_val * 2) > 255) {
				m_pp.INT = 255;
			} else {
				m_pp.INT += increase_val * 2;
			}
			break;
		case STAT_WIS:
			if (increase_val > 0) {
				iss->wis = increase_val;
			}

			if ((m_pp.WIS + increase_val * 2) > 255) {
				m_pp.WIS = 255;
			} else {
				m_pp.WIS += increase_val * 2;
			}
			break;
		case STAT_CHA:
			if (increase_val > 0) {
				iss->cha = increase_val;
			}

			if ((m_pp.CHA + increase_val * 2) > 255) {
				m_pp.CHA = 255;
			} else {
				m_pp.CHA += increase_val * 2;
			}
			break;
	}
	QueuePacket(outapp);
	safe_delete(outapp);
}

const int64& Client::SetMana(int64 amount) {
	bool update = false;
	if (amount < 0)
		amount = 0;
	if (amount > GetMaxMana())
		amount = GetMaxMana();
	if (amount != current_mana)
		update = true;
	current_mana = amount;
	if (update)
		Mob::SetMana(amount);
	CheckManaEndUpdate();
	return current_mana;
}

void Client::CheckManaEndUpdate() {
	if (!Connected())
		return;

	if (last_reported_mana != current_mana || last_reported_endurance != current_endurance) {

		if (ClientVersion() >= EQ::versions::ClientVersion::SoD) {
			SendManaUpdate();
			SendEnduranceUpdate();
		}

		auto outapp = new EQApplicationPacket(OP_ManaChange, sizeof(ManaChange_Struct));
		ManaChange_Struct* mana_change = (ManaChange_Struct*)outapp->pBuffer;
		mana_change->new_mana = current_mana;
		mana_change->stamina = current_endurance;
		mana_change->spell_id = casting_spell_id;
		mana_change->keepcasting = 1;
		mana_change->slot = -1;
		outapp->priority = 6;
		QueuePacket(outapp);
		safe_delete(outapp);

		/* Let others know when our mana percent has changed */
		if (GetManaPercent() != last_reported_mana_percent) {
			Group *group = GetGroup();
			Raid *raid = GetRaid();

			if (raid) {
				raid->SendManaPacketFrom(this);
			}
			else if (group) {
				group->SendManaPacketFrom(this);
			}

			auto mana_packet = new EQApplicationPacket(OP_ManaUpdate, sizeof(ManaUpdate_Struct));
			ManaUpdate_Struct* mana_update = (ManaUpdate_Struct*)mana_packet->pBuffer;
			mana_update->cur_mana = GetMana();
			mana_update->max_mana = GetMaxMana();
			mana_update->spawn_id = GetID();
			if ((ClientVersionBit() & EQ::versions::ClientVersionBitmask::maskSoDAndLater) != 0)
				QueuePacket(mana_packet); // do we need this with the OP_ManaChange packet above?
			entity_list.QueueClientsByXTarget(this, mana_packet, false, EQ::versions::ClientVersionBitmask::maskSoDAndLater);
			safe_delete(mana_packet);

			last_reported_mana_percent = GetManaPercent();
		}

		/* Let others know when our endurance percent has changed */
		if (GetEndurancePercent() != last_reported_endurance_percent) {
			Group *group = GetGroup();
			Raid *raid = GetRaid();

			if (raid) {
				raid->SendEndurancePacketFrom(this);
			}
			else if (group) {
				group->SendEndurancePacketFrom(this);
			}

			auto endurance_packet = new EQApplicationPacket(OP_EnduranceUpdate, sizeof(EnduranceUpdate_Struct));
			EnduranceUpdate_Struct* endurance_update = (EnduranceUpdate_Struct*)endurance_packet->pBuffer;
			endurance_update->cur_end = GetEndurance();
			endurance_update->max_end = GetMaxEndurance();
			endurance_update->spawn_id = GetID();
			if ((ClientVersionBit() & EQ::versions::ClientVersionBitmask::maskSoDAndLater) != 0)
				QueuePacket(endurance_packet); // do we need this with the OP_ManaChange packet above?
			entity_list.QueueClientsByXTarget(this, endurance_packet, false, EQ::versions::ClientVersionBitmask::maskSoDAndLater);
			safe_delete(endurance_packet);

			last_reported_endurance_percent = GetEndurancePercent();
		}

		last_reported_mana = current_mana;
		last_reported_endurance = current_endurance;
	}
}

// sends mana update to self
void Client::SendManaUpdate()
{
	auto mana_app = new EQApplicationPacket(OP_ManaUpdate, sizeof(ManaUpdate_Struct));
	ManaUpdate_Struct* mana_update = (ManaUpdate_Struct*)mana_app->pBuffer;
	mana_update->cur_mana = GetMana();
	mana_update->max_mana = GetMaxMana();
	mana_update->spawn_id = GetID();
	QueuePacket(mana_app);
	safe_delete(mana_app);
}

// sends endurance update to self
void Client::SendEnduranceUpdate()
{
	auto end_app = new EQApplicationPacket(OP_EnduranceUpdate, sizeof(EnduranceUpdate_Struct));
	EnduranceUpdate_Struct* endurance_update = (EnduranceUpdate_Struct*)end_app->pBuffer;
	endurance_update->cur_end = GetEndurance();
	endurance_update->max_end = GetMaxEndurance();
	endurance_update->spawn_id = GetID();
	QueuePacket(end_app);
	safe_delete(end_app);
}

void Client::FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho)
{
	Mob::FillSpawnStruct(ns, ForWho);

	// Populate client-specific spawn information
	ns->spawn.afk       = m_is_afk;
	ns->spawn.lfg       = LFG; // afk and lfg are cleared on zoning on live
	ns->spawn.anon      = m_pp.anon;
	ns->spawn.gm        = GetGM() ? 1 : 0;
	ns->spawn.guildID   = GuildID();
	ns->spawn.trader    = IsTrader();
	ns->spawn.buyer     = IsBuyer();
//	ns->spawn.linkdead	= IsLD() ? 1 : 0;
//	ns->spawn.pvp		= GetPVP(false) ? 1 : 0;
	ns->spawn.show_name = true;

	strcpy(ns->spawn.title, m_pp.title);
	strcpy(ns->spawn.suffix, m_pp.suffix);

	if (IsBecomeNPC() == true)
		ns->spawn.NPC = 1;
	else if (ForWho == this)
		ns->spawn.NPC = 10;
	else
		ns->spawn.NPC = 0;
	ns->spawn.is_pet = 0;

	if (!IsInAGuild()) {
		ns->spawn.guildrank = 0xFF;
	} else {
		ns->spawn.guildrank = guild_mgr.GetDisplayedRank(GuildID(), GuildRank(), CharacterID());
		ns->spawn.guild_show = guild_mgr.CheckPermission(GuildID(), GuildRank(), GUILD_ACTION_DISPLAY_GUILD_NAME);
	}
	ns->spawn.size			= 0; // Changing size works, but then movement stops! (wth?)
	ns->spawn.runspeed		= (gmspeed == 0) ? runspeed : 3.125f;
	ns->spawn.showhelm = m_pp.showhelm ? 1 : 0;

	UpdateEquipmentLight();
	UpdateActiveLight();
	ns->spawn.light = m_Light.Type[EQ::lightsource::LightActive];
}

bool Client::GMHideMe(Client* client) {
	if (gm_hide_me) {
		if (client == 0)
			return true;
		else if (admin > client->Admin())
			return true;
		else
			return false;
	}
	else
		return false;
}

void Client::Duck() {
	SetAppearance(eaCrouching, false);
}

void Client::Stand() {
	SetAppearance(eaStanding, false);
}

void Client::Sit() {
	SetAppearance(eaSitting, false);
}

void Client::ChangeLastName(std::string last_name) {
	memset(m_pp.last_name, 0, sizeof(m_pp.last_name));
	strn0cpy(m_pp.last_name, last_name.c_str(), sizeof(m_pp.last_name));
	auto outapp = new EQApplicationPacket(OP_GMLastName, sizeof(GMLastName_Struct));
	auto gmn = (GMLastName_Struct*) outapp->pBuffer;
	strn0cpy(gmn->name, name, sizeof(gmn->name));
	strn0cpy(gmn->gmname, name, sizeof(gmn->gmname));
	strn0cpy(gmn->lastname, last_name.c_str(), sizeof(gmn->lastname));

	gmn->unknown[0] = 1;
	gmn->unknown[1] = 1;
	gmn->unknown[2] = 1;
	gmn->unknown[3] = 1;

	entity_list.QueueClients(this, outapp, false);

	safe_delete(outapp);
}

// Deprecated, this packet does not actually work in ROF2
bool Client::ChangeFirstName(const std::string in_firstname, const std::string gmname)
{
	if (!ChangeFirstName(in_firstname)) {
		return false;
	}

	// send name update packet
	auto outapp = new EQApplicationPacket(OP_GMNameChange, sizeof(GMName_Struct));
	GMName_Struct* gmn=(GMName_Struct*)outapp->pBuffer;
	strn0cpy(gmn->gmname,gmname.c_str(),64);
	strn0cpy(gmn->oldname,GetName(),64);
	strn0cpy(gmn->newname,in_firstname.c_str(),64);
	gmn->unknown[0] = 1;
	gmn->unknown[1] = 1;
	gmn->unknown[2] = 1;
	entity_list.QueueClients(this, outapp, false);
	safe_delete(outapp);

	// success
	return true;
}

bool Client::ChangeFirstName(const std::string in_firstname)
{
	// check duplicate name
	bool used_name = database.IsNameUsed(in_firstname) || database.IsPetNameUsed(in_firstname);
	if (used_name || !database.CheckNameFilter(in_firstname, false)) {
		return false;
	}

	// update character_
	if(!database.UpdateNameByID(CharacterID(), in_firstname))
		return false;

	// Send Name Update to Clients
	SendRename(this, GetName(), in_firstname.c_str());
	SetName(in_firstname.c_str());

	// update pp
	memset(m_pp.name, 0, sizeof(m_pp.name));
	snprintf(m_pp.name, sizeof(m_pp.name), "%s", in_firstname.c_str());
	strcpy(name, m_pp.name);
	Save();

	// Update the active char in account table
	database.UpdateLiveChar(in_firstname, AccountID());

	// finally, update the /who list
	UpdateWho();

	// success
	ClearNameChange();
	return true;
}

void Client::SetGM(bool toggle) {
	m_pp.gm = toggle ? 1 : 0;
	m_inv.SetGMInventory((bool)m_pp.gm);
	Message(
		Chat::White,
		fmt::format(
			"You are {} flagged as a GM.",
			m_pp.gm ? "now" : "no longer"
		).c_str()
	);
	SendAppearancePacket(AppearanceType::GM, m_pp.gm);
	Save();
	UpdateWho();
}

void Client::ReadBook(BookRequest_Struct* book)
{
	const std::string& text_file = book->txtfile;

	if (text_file.empty()) {
		return;
	}

	auto b = content_db.GetBook(text_file);

	if (!b.text.empty()) {
		auto outapp = new EQApplicationPacket(OP_ReadBook, b.text.size() + sizeof(BookText_Struct));
		auto inst = const_cast<EQ::ItemInstance*>(m_inv[book->invslot]);

		auto t = (BookText_Struct*) outapp->pBuffer;

		t->window     = book->window;
		t->type       = book->type;
		t->invslot    = book->invslot;
		t->target_id  = book->target_id;
		t->can_cast   = 0; // todo: implement
		t->can_scribe = false;

		if (ClientVersion() >= EQ::versions::ClientVersion::SoF && book->invslot <= EQ::invbag::GENERAL_BAGS_END) {
			if (inst && inst->GetItem()) {
				auto recipe = TradeskillRecipeRepository::GetWhere(
					content_db,
					fmt::format(
						"learned_by_item_id = {} LIMIT 1",
						inst->GetItem()->ID
					)
				);

				t->type       = inst->GetItem()->Book;
				t->can_scribe = !recipe.empty();
			}
		}

		memcpy(t->booktext, b.text.c_str(), b.text.size());

		if (EQ::ValueWithin(b.language, Language::CommonTongue, Language::Unknown27)) {
			if (m_pp.languages[b.language] < Language::MaxValue) {
				GarbleMessage(t->booktext, (Language::MaxValue - m_pp.languages[b.language]));
			}
		}

		// Send only books and scrolls to this event
		if (parse->PlayerHasQuestSub(EVENT_READ_ITEM) && t->type != BookType::ItemInfo) {
			std::vector<std::any> args = {
				b.text,
				t->can_cast,
				t->can_scribe,
				t->invslot,
				t->target_id,
				t->type,
				inst
			};

			parse->EventPlayer(EVENT_READ_ITEM, this, book->txtfile, inst ? inst->GetID() : 0, &args);
		}

		QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void Client::QuestReadBook(const char* text, uint8 type) {
	std::string booktxt2 = text;
	int length = booktxt2.length();
	if (booktxt2[0] != '\0') {
		auto outapp = new EQApplicationPacket(OP_ReadBook, length + sizeof(BookText_Struct));
		BookText_Struct *out = (BookText_Struct *) outapp->pBuffer;
		out->window = 0xFF;
		out->type = type;
		out->invslot = 0;
		memcpy(out->booktext, booktxt2.c_str(), length);
		QueuePacket(outapp);
		safe_delete(outapp);
	}
}

uint32 Client::GetCarriedPlatinum() {
	return (
		GetMoney(MoneyTypes::Platinum, MoneySubtypes::Personal) +
		(GetMoney(MoneyTypes::Gold, MoneySubtypes::Personal) / 10) +
		(GetMoney(MoneyTypes::Silver, MoneySubtypes::Personal) / 100) +
		(GetMoney(MoneyTypes::Copper, MoneySubtypes::Personal) / 1000)
	);
}

bool Client::TakePlatinum(uint32 platinum, bool update_client) {
	if (GetCarriedPlatinum() >= platinum) {
		const auto copper = static_cast<uint64>(platinum) * 1000;
		return TakeMoneyFromPP(copper, update_client);
	}

	return false;
}

bool Client::TakeMoneyFromPP(uint64 copper, bool update_client) {
	int64 player_copper, silver, gold, platinum;
	player_copper = m_pp.copper;
	silver = static_cast<int64>(m_pp.silver) * 10;
	gold = static_cast<int64>(m_pp.gold) * 100;
	platinum = static_cast<int64>(m_pp.platinum) * 1000;

	int64 client_total = player_copper + silver + gold + platinum;

	client_total -= copper;
	if (client_total < 0) {
		return false; // Not enough money!
	} else {
		player_copper -= copper;
		if(player_copper <= 0) {
			copper = std::abs(player_copper);
			m_pp.copper = 0;
		} else {
			m_pp.copper = player_copper;

			if (update_client) {
				SendMoneyUpdate();
			}

			SaveCurrency();
			return true;
		}

		silver -= copper;
		if (silver <= 0) {
			copper = std::abs(silver);
			m_pp.silver = 0;
		} else {
			m_pp.silver = silver / 10;
			m_pp.copper += (silver - (m_pp.silver * 10));

			if (update_client) {
				SendMoneyUpdate();
			}

			SaveCurrency();
			return true;
		}

		gold -=copper;

		if (gold <= 0) {
			copper = std::abs(gold);
			m_pp.gold = 0;
		} else {
			m_pp.gold = gold / 100;
			uint64 silver_test = (gold - (static_cast<uint64>(m_pp.gold) * 100)) / 10;
			m_pp.silver += silver_test;
			uint64 copper_test = (gold - (static_cast<uint64>(m_pp.gold) * 100 + silver_test * 10));
			m_pp.copper += copper_test;

			if (update_client) {
				SendMoneyUpdate();
			}

			SaveCurrency();
			return true;
		}

		platinum -= copper;

		//Impossible for plat to be negative, already checked above

		m_pp.platinum = platinum / 1000;
		uint64 gold_test = (platinum - (static_cast<uint64>(m_pp.platinum) * 1000)) / 100;
		m_pp.gold += gold_test;
		uint64 silver_test = (platinum - (static_cast<uint64>(m_pp.platinum) * 1000 + gold_test * 100)) / 10;
		m_pp.silver += silver_test;
		uint64 copper_test = (platinum - (static_cast<uint64>(m_pp.platinum) * 1000 + gold_test * 100 + silver_test * 10));
		m_pp.copper = copper_test;

		if (update_client) {
			SendMoneyUpdate();
		}

		RecalcWeight();
		SaveCurrency();
		return true;
	}
}

void Client::AddPlatinum(uint32 platinum, bool update_client) {
	const auto copper = static_cast<uint64>(platinum) * 1000;
	AddMoneyToPP(copper, update_client);
}

void Client::AddMoneyToPP(uint64 copper, bool update_client){
	uint64 temporary_copper;
	uint64 temporary_copper_two;
	temporary_copper = copper;

	/* Add Amount of Platinum */
	temporary_copper_two = temporary_copper / 1000;
	m_external_handin_money_returned.platinum = temporary_copper_two;
	int32 new_value = m_pp.platinum + temporary_copper_two;

	if (new_value < 0) {
		m_pp.platinum = 0;
	} else {
		m_pp.platinum = m_pp.platinum + temporary_copper_two;
	}

	temporary_copper -= temporary_copper_two * 1000;

	/* Add Amount of Gold */
	temporary_copper_two = temporary_copper / 100;
	new_value = m_pp.gold + temporary_copper_two;
	m_external_handin_money_returned.gold = temporary_copper_two;

	if (new_value < 0) {
		m_pp.gold = 0;
	} else {
		m_pp.gold = m_pp.gold + temporary_copper_two;
	}

	temporary_copper -= temporary_copper_two * 100;

	/* Add Amount of Silver */
	temporary_copper_two = temporary_copper / 10;
	new_value = m_pp.silver + temporary_copper_two;
	m_external_handin_money_returned.silver = temporary_copper_two;

	if (new_value < 0) {
		m_pp.silver = 0;
	} else {
		m_pp.silver = m_pp.silver + temporary_copper_two;
	}

	temporary_copper -= temporary_copper_two * 10;

	/* Add Amount of Copper */
	temporary_copper_two = temporary_copper;
	new_value = m_pp.copper + temporary_copper_two;
	m_external_handin_money_returned.copper = temporary_copper_two;

	if (new_value < 0) {
		m_pp.copper = 0;
	} else {
		m_pp.copper = m_pp.copper + temporary_copper_two;
	}

	//send them all at once, since the above code stopped working.
	if (update_client) {
		SendMoneyUpdate();
	}

	RecalcWeight();

	SaveCurrency();

	m_external_handin_money_returned.return_source = "AddMoneyToPP";

	LogDebug("Client::AddMoneyToPP() [{}] should have: plat:[{}] gold:[{}] silver:[{}] copper:[{}]", GetName(), m_pp.platinum, m_pp.gold, m_pp.silver, m_pp.copper);
}

void Client::AddMoneyToPP(uint32 copper, uint32 silver, uint32 gold, uint32 platinum, bool update_client){
	int32 new_value = m_pp.platinum + platinum;
	if (new_value >= 0 && new_value > m_pp.platinum) {
		m_pp.platinum += platinum;
	}

	new_value = m_pp.gold + gold;
	if (new_value >= 0 && new_value > m_pp.gold) {
		m_pp.gold += gold;
	}

	new_value = m_pp.silver + silver;
	if (new_value >= 0 && new_value > m_pp.silver) {
		m_pp.silver += silver;
	}

	new_value = m_pp.copper + copper;
	if (new_value >= 0 && new_value > m_pp.copper) {
		m_pp.copper += copper;
	}

	if (update_client) {
		SendMoneyUpdate();
	}

	RecalcWeight();
	SaveCurrency();

	m_external_handin_money_returned = ExternalHandinMoneyReturned{
		.copper = copper,
		.silver = silver,
		.gold = gold,
		.platinum = platinum,
		.return_source = "AddMoneyToPP"
	};

#if (EQDEBUG>=5)
		LogDebug("Client::AddMoneyToPP() [{}] should have: plat:[{}] gold:[{}] silver:[{}] copper:[{}]",
			GetName(), m_pp.platinum, m_pp.gold, m_pp.silver, m_pp.copper);
#endif
}

void Client::SendMoneyUpdate() {
	auto outapp = new EQApplicationPacket(OP_MoneyUpdate, sizeof(MoneyUpdate_Struct));
	MoneyUpdate_Struct* mus= (MoneyUpdate_Struct*)outapp->pBuffer;

	mus->platinum = m_pp.platinum;
	mus->gold = m_pp.gold;
	mus->silver = m_pp.silver;
	mus->copper = m_pp.copper;

	FastQueuePacket(&outapp);
}

bool Client::HasMoney(uint64 copper) {

	if (
		(static_cast<uint64>(m_pp.copper) +
		(static_cast<uint64>(m_pp.silver) * 10) +
		(static_cast<uint64>(m_pp.gold) * 100) +
		(static_cast<uint64>(m_pp.platinum) * 1000)) >= copper
	) {
		return true;
	}

	return false;
}

uint64 Client::GetCarriedMoney() {

	return (
		(
			static_cast<uint64>(m_pp.copper) +
			(static_cast<uint64>(m_pp.silver) * 10) +
			(static_cast<uint64>(m_pp.gold) * 100) +
			(static_cast<uint64>(m_pp.platinum) * 1000)
		)
	);
}

uint64 Client::GetAllMoney() {

	return (
		(
			static_cast<uint64>(m_pp.copper) +
			(static_cast<uint64>(m_pp.silver) * 10) +
			(static_cast<uint64>(m_pp.gold) * 100) +
			(static_cast<uint64>(m_pp.platinum) * 1000) +
			(
				static_cast<uint64>(m_pp.copper_bank) +
				(static_cast<uint64>(m_pp.silver_bank) * 10) +
				(static_cast<uint64>(m_pp.gold_bank) * 100) +
				(static_cast<uint64>(m_pp.platinum_bank) * 1000) +
				(
					static_cast<uint64>(m_pp.copper_cursor) +
					(static_cast<uint64>(m_pp.silver_cursor) * 10) +
					(static_cast<uint64>(m_pp.gold_cursor) * 100) +
					(static_cast<uint64>(m_pp.platinum_cursor) * 1000) +
					(static_cast<uint64>(m_pp.platinum_shared) * 1000)
				)
			)
		)
	);
}

bool Client::CheckIncreaseSkill(EQ::skills::SkillType skillid, Mob *against_who, int chancemodi) {
	if (IsDead() || IsUnconscious()) {
		return false;
	}

	if (IsAIControlled()) { // no skillups while chamred =p
		return false;
	}

	if (against_who && against_who->IsCorpse()) { // no skillups on corpses
		return false;
	}

	if (skillid > EQ::skills::HIGHEST_SKILL) {
		return false;
	}

	auto skillval = GetRawSkill(skillid);
	auto maxskill = GetMaxSkillAfterSpecializationRules(skillid, MaxSkill(skillid));

	if (parse->PlayerHasQuestSub(EVENT_USE_SKILL)) {
		const auto& export_string = fmt::format(
			"{} {}",
			skillid,
			skillval
		);

		parse->EventPlayer(EVENT_USE_SKILL, this, export_string, 0);
	}

	if (against_who) {
		if (
			against_who->GetSpecialAbility(SpecialAbility::AggroImmunity) ||
			against_who->GetSpecialAbility(SpecialAbility::ClientAggroImmunity) ||
			against_who->IsClient() ||
			GetLevelCon(against_who->GetLevel()) == ConsiderColor::Gray
		) {
			return false;
		}
	}

	// Make sure we're not already at skill cap
	if (skillval < maxskill)
	{
		double Chance = 0;
		if (RuleI(Character, SkillUpMaximumChancePercentage) + chancemodi - RuleI(Character, SkillUpMinimumChancePercentage) <= RuleI(Character, SkillUpMinimumChancePercentage)) {
			Chance = RuleI(Character, SkillUpMinimumChancePercentage);
		}
		else {
			// f(x) = (max - min + modification) * .99^skillval + min
			// This results in a exponential decay where as you skill up, you lose a slight chance to skill up, ranging from your modified maximum to approaching your minimum
			// This result is increased by the existing SkillUpModifier rule
			double working_chance = (((RuleI(Character, SkillUpMaximumChancePercentage) - RuleI(Character, SkillUpMinimumChancePercentage) + chancemodi) * (pow(0.99, skillval))) + RuleI(Character, SkillUpMinimumChancePercentage));
			Chance = (working_chance * RuleI(Character, SkillUpModifier) / 100);
		}

		if(zone->random.Real(0, 99) < Chance)
		{
			SetSkill(skillid, GetRawSkill(skillid) + 1);

			if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::SKILL_UP)) {
				auto e = PlayerEvent::SkillUpEvent{
					.skill_id = static_cast<uint32>(skillid),
					.value = static_cast<int>((skillval + 1)),
					.max_skill = static_cast<int16>(maxskill),
					.against_who = (against_who) ? against_who->GetCleanName() : GetCleanName(),
				};
				RecordPlayerEventLog(PlayerEvent::SKILL_UP, e);
			}

			if (parse->PlayerHasQuestSub(EVENT_SKILL_UP)) {
				const auto& export_string = fmt::format(
					"{} {} {} {}",
					skillid,
					skillval + 1,
					maxskill,
					0
				);

				parse->EventPlayer(EVENT_SKILL_UP, this, export_string, 0);
			}

			LogSkills("Skill [{}] at value [{}] successfully gain with [{}] chance (mod [{}])", skillid, skillval, Chance, chancemodi);
			return true;
		} else {
			LogSkills("Skill [{}] at value [{}] failed to gain with [{}] chance (mod [{}])", skillid, skillval, Chance, chancemodi);
		}
	} else {
		LogSkills("Skill [{}] at value [{}] cannot increase due to maxmum [{}]", skillid, skillval, maxskill);
	}
	return false;
}

void Client::CheckLanguageSkillIncrease(uint8 language_id, uint8 teacher_skill) {
	if (IsDead() || IsUnconscious()) {
		return;
	}

	if (IsAIControlled()) {
		return;
	}

	if (!EQ::ValueWithin(language_id, Language::CommonTongue, Language::Unknown27)) {
		return;
	}

	const uint8 language_skill = m_pp.languages[language_id]; // get current language skill

	if (language_skill < Language::MaxValue) { // if the language isn't already maxed
		int chance = 5 + ((teacher_skill - language_skill) / 10); // greater chance to learn if teacher's skill is much higher than yours
		chance = (chance * RuleI(Character, SkillUpModifier) / 100);

		if (zone->random.Real(0, 100) < chance) { // if they make the roll
			IncreaseLanguageSkill(language_id);

			if (parse->PlayerHasQuestSub(EVENT_LANGUAGE_SKILL_UP)) {
				const auto &export_string = fmt::format(
					"{} {} {}",
					language_id,
					language_skill + 1,
					Language::MaxValue
				);

				parse->EventPlayer(EVENT_LANGUAGE_SKILL_UP, this, export_string, 0);
			}

			LogSkills("Language [{}] at value [{}] successfully gain with [{}] % chance", language_id, language_skill, chance);
		} else {
			LogSkills("Language [{}] at value [{}] failed to gain with [{}] % chance", language_id, language_skill, chance);
		}
	}
}

bool Client::HasSkill(EQ::skills::SkillType skill_id) const
{
	return GetSkill(skill_id) > 0 && CanHaveSkill(skill_id);
}

bool Client::CanHaveSkill(EQ::skills::SkillType skill_id) const
{
	if (
		ClientVersion() < EQ::versions::ClientVersion::RoF2 &&
		class_ == Class::Berserker &&
		skill_id == EQ::skills::Skill1HPiercing
	) {
		skill_id = EQ::skills::Skill2HPiercing;
	}

	return SkillCaps::Instance()->GetSkillCap(GetClass(), skill_id, RuleI(Character, MaxLevel)).cap > 0;
}

uint16 Client::MaxSkill(EQ::skills::SkillType skill_id, uint8 class_id, uint8 level) const
{
	if (
		ClientVersion() < EQ::versions::ClientVersion::RoF2 &&
		class_id == Class::Berserker &&
		skill_id == EQ::skills::Skill1HPiercing
	) {
		skill_id = EQ::skills::Skill2HPiercing;
	}

	return SkillCaps::Instance()->GetSkillCap(class_id, skill_id, level).cap;
}

uint8 Client::GetSkillTrainLevel(EQ::skills::SkillType skill_id, uint8 class_id)
{
	if (
		ClientVersion() < EQ::versions::ClientVersion::RoF2 &&
		class_id == Class::Berserker &&
		skill_id == EQ::skills::Skill1HPiercing
	) {
		skill_id = EQ::skills::Skill2HPiercing;
	}

	return SkillCaps::Instance()->GetSkillTrainLevel(class_id, skill_id, RuleI(Character, MaxLevel));
}

uint16 Client::GetMaxSkillAfterSpecializationRules(EQ::skills::SkillType skillid, uint16 maxSkill)
{
	uint16 Result = maxSkill;

	uint16 PrimarySpecialization = 0, SecondaryForte = 0;

	uint16 PrimarySkillValue = 0, SecondarySkillValue = 0;

	uint16 MaxSpecializations = aabonuses.SecondaryForte ? 2 : 1;

	if (skillid >= EQ::skills::SkillSpecializeAbjure && skillid <= EQ::skills::SkillSpecializeEvocation)
	{
		bool HasPrimarySpecSkill = false;

		int NumberOfPrimarySpecSkills = 0;

		for (int i = EQ::skills::SkillSpecializeAbjure; i <= EQ::skills::SkillSpecializeEvocation; ++i)
		{
			if(m_pp.skills[i] > 50)
			{
				HasPrimarySpecSkill = true;
				NumberOfPrimarySpecSkills++;
			}
			if(m_pp.skills[i] > PrimarySkillValue)
			{
				if(PrimarySkillValue > SecondarySkillValue)
				{
					SecondarySkillValue = PrimarySkillValue;
					SecondaryForte = PrimarySpecialization;
				}

				PrimarySpecialization = i;
				PrimarySkillValue = m_pp.skills[i];
			}
			else if(m_pp.skills[i] > SecondarySkillValue)
			{
				SecondaryForte = i;
				SecondarySkillValue = m_pp.skills[i];
			}
		}

		if(SecondarySkillValue <=50)
			SecondaryForte = 0;

		if(HasPrimarySpecSkill)
		{
			if(NumberOfPrimarySpecSkills <= MaxSpecializations)
			{
				if(MaxSpecializations == 1)
				{
					if(skillid != PrimarySpecialization)
					{
						Result = 50;
					}
				}
				else
				{
					if((skillid != PrimarySpecialization) && ((skillid == SecondaryForte) || (SecondaryForte == 0)))
					{
						if((PrimarySkillValue > 100) || (!SecondaryForte))
							Result = 100;
					}
					else if(skillid != PrimarySpecialization)
					{
						Result = 50;
					}
				}
			}
			else
			{
				Message(Chat::Red, "Your spell casting specializations skills have been reset. "
						"Only %i primary specialization skill is allowed.", MaxSpecializations);

				for (int i = EQ::skills::SkillSpecializeAbjure; i <= EQ::skills::SkillSpecializeEvocation; ++i)
					SetSkill((EQ::skills::SkillType)i, 1);

				Save();

				LogInfo("Reset [{}]'s caster specialization skills to 1"
								"Too many specializations skills were above 50.", GetCleanName());
			}

		}
	}

	Result += spellbonuses.RaiseSkillCap[skillid] + itembonuses.RaiseSkillCap[skillid] + aabonuses.RaiseSkillCap[skillid];

	if (skillid == EQ::skills::SkillType::SkillForage)
		Result += aabonuses.GrantForage;

	return Result;
}

void Client::SetPVP(bool toggle, bool message) {
	m_pp.pvp = toggle ? 1 : 0;

	if (message) {
		if(GetPVP()) {
			MessageString(Chat::Shout, PVP_ON);
		} else {
			Message(Chat::Shout, "You now follow the ways of Order.");
		}
	}

	SendAppearancePacket(AppearanceType::PVP, GetPVP());
	Save();
}

void Client::Kick(const std::string &reason) {
	client_state = CLIENT_KICKED;

	LogClientLogin("Client [{}] kicked, reason [{}]", GetCleanName(), reason.c_str());
}

void Client::WorldKick() {
	auto outapp = new EQApplicationPacket(OP_GMKick, sizeof(GMKick_Struct));
	GMKick_Struct* gmk = (GMKick_Struct *)outapp->pBuffer;
	strcpy(gmk->name,GetName());
	QueuePacket(outapp);
	safe_delete(outapp);
	Kick("World kick issued");
}

void Client::GMKill() {
	auto outapp = new EQApplicationPacket(OP_GMKill, sizeof(GMKill_Struct));
	GMKill_Struct* gmk = (GMKill_Struct *)outapp->pBuffer;
	strcpy(gmk->name,GetName());
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::MemorizeSpell(uint32 slot, uint32 spell_id, uint32 scribing, uint32 reduction){
	if (
		!EQ::ValueWithin(
			slot,
			0,
			(EQ::spells::DynamicLookup(ClientVersion(), GetGM())->SpellbookSize - 1)
		)
	) {
		return;
	}

	if (
		!EQ::ValueWithin(
			spell_id,
			3,
			EQ::spells::DynamicLookup(ClientVersion(), GetGM())->SpellIdMax
		) &&
		spell_id != UINT32_MAX
	) {
		return;
	}

	auto outapp = new EQApplicationPacket(OP_MemorizeSpell, sizeof(MemorizeSpell_Struct));

	auto* mss = (MemorizeSpell_Struct*) outapp->pBuffer;

	mss->scribing  = scribing;
	mss->slot      = slot;
	mss->spell_id  = spell_id;
	mss->reduction = reduction;

	outapp->priority = 5;

	if (
		parse->PlayerHasQuestSub(EVENT_SCRIBE_SPELL) ||
		parse->PlayerHasQuestSub(EVENT_MEMORIZE_SPELL) ||
		parse->PlayerHasQuestSub(EVENT_UNMEMORIZE_SPELL)
	) {
		const auto export_string = fmt::format("{} {}", slot, spell_id);

		if (
			scribing == ScribeSpellActions::Memorize &&
			parse->PlayerHasQuestSub(EVENT_MEMORIZE_SPELL)
		) {
			parse->EventPlayer(EVENT_MEMORIZE_SPELL, this, export_string, 0);
		} else if (
			scribing == ScribeSpellActions::Unmemorize &&
			parse->PlayerHasQuestSub(EVENT_UNMEMORIZE_SPELL)
		) {
			parse->EventPlayer(EVENT_UNMEMORIZE_SPELL, this, export_string, 0);
		} else if (
			scribing == ScribeSpellActions::Scribe &&
			parse->PlayerHasQuestSub(EVENT_SCRIBE_SPELL)
		) {
			parse->EventPlayer(EVENT_SCRIBE_SPELL, this, export_string, 0);
		}
	}

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::Disarm(Client* disarmer, int chance) {
	int16 slot = EQ::invslot::SLOT_INVALID;
	const EQ::ItemInstance *inst = GetInv().GetItem(EQ::invslot::slotPrimary);
	if (inst && inst->IsWeapon()) {
		slot = EQ::invslot::slotPrimary;
	}
	else {
		inst = GetInv().GetItem(EQ::invslot::slotSecondary);
		if (inst && inst->IsWeapon())
			slot = EQ::invslot::slotSecondary;
	}
	if (slot != EQ::invslot::SLOT_INVALID && inst->IsClassCommon()) {
		// We have an item that can be disarmed.
		if (zone->random.Int(0, 1000) <= chance) {
			// Find a free inventory slot
			int16 slot_id = EQ::invslot::SLOT_INVALID;
			slot_id = m_inv.FindFreeSlot(false, true, inst->GetItem()->Size, (inst->GetItem()->ItemType == EQ::item::ItemTypeArrow));
			if (slot_id != EQ::invslot::SLOT_INVALID)
			{
				EQ::ItemInstance *InvItem = m_inv.PopItem(slot);
				if (InvItem) { // there should be no way it is not there, but check anyway
					EQApplicationPacket* outapp = new EQApplicationPacket(OP_MoveItem, sizeof(MoveItem_Struct));
					MoveItem_Struct* mi = (MoveItem_Struct*)outapp->pBuffer;
					mi->from_slot = slot;
					mi->to_slot = 0xFFFFFFFF;
					if (inst->IsStackable()) // it should not be stackable
						mi->number_in_stack = inst->GetCharges();
					else
						mi->number_in_stack = 0;
					FastQueuePacket(&outapp); // this deletes item from the weapon slot on the client
					if (PutItemInInventory(slot_id, *InvItem, true))
						database.SaveInventory(CharacterID(), NULL, slot);
					auto matslot = (slot == EQ::invslot::slotPrimary ? EQ::textures::weaponPrimary : EQ::textures::weaponSecondary);
					if (matslot != EQ::textures::materialInvalid)
						SendWearChange(matslot);
				}
				MessageString(Chat::Skills, DISARMED);
				if (disarmer != this)
					disarmer->MessageString(Chat::Skills, DISARM_SUCCESS, GetCleanName());
				if (chance != 1000)
					disarmer->CheckIncreaseSkill(EQ::skills::SkillDisarm, nullptr, 4);
				CalcBonuses();
				// CalcEnduranceWeightFactor();
				return;
			}
			disarmer->MessageString(Chat::Skills, DISARM_FAILED);
			if (chance != 1000)
				disarmer->CheckIncreaseSkill(EQ::skills::SkillDisarm, nullptr, 2);
			return;
		}
	}
	disarmer->MessageString(Chat::Skills, DISARM_FAILED);
}

bool Client::BindWound(Mob *bindmob, bool start, bool fail)
{
	EQApplicationPacket *outapp = nullptr;
	if (!fail) {
		outapp = new EQApplicationPacket(OP_Bind_Wound, sizeof(BindWound_Struct));
		BindWound_Struct *bind_out = (BindWound_Struct *)outapp->pBuffer;
		// Start bind
		if (!bindwound_timer.Enabled()) {
			// make sure we actually have a bandage... and consume it.
			int16 bslot = m_inv.HasItemByUse(EQ::item::ItemTypeBandage, 1, invWhereWorn | invWherePersonal);
			if (bslot == INVALID_INDEX) {
				bind_out->type = 3;
				QueuePacket(outapp);
				bind_out->type = 7; // this is the wrong message, dont know the right one.
				QueuePacket(outapp);
				safe_delete(outapp);
				return (true);
			}
			DeleteItemInInventory(bslot, 1, true); // do we need client update?

			// start complete timer
			bindwound_timer.Start(10000);
			bindwound_target = bindmob;

			// Send client unlock
			bind_out->type = 3;
			QueuePacket(outapp);
			bind_out->type = 0;
			// Client Unlocked
			if (!bindmob) {
				// send "bindmob dead" to client
				bind_out->type = 4;
				QueuePacket(outapp);
				bind_out->type = 0;
				bindwound_timer.Disable();
				bindwound_target = 0;
			} else {
				// send bindmob "stand still"
				if (!bindmob->IsAIControlled() && bindmob != this) {
					bindmob->CastToClient()->MessageString(Chat::Yellow,
										  YOU_ARE_BEING_BANDAGED);
				} else if (bindmob->IsAIControlled() && bindmob != this) {
					; // Tell IPC to stand still?
				} else {
					; // Binding self
				}
			}
		} else if (bindwound_timer.Check()) // Did the timer finish?
		{
			// finish bind
			// disable complete timer
			bindwound_timer.Disable();
			bindwound_target = 0;
			if (!bindmob) {
				// send "bindmob gone" to client
				bind_out->type = 5; // not in zone
				QueuePacket(outapp);
				bind_out->type = 0;
			}

			else {
				if (!GetFeigned() && (DistanceSquared(bindmob->GetPosition(), m_Position) <= 400)) {
					// send bindmob bind done
					if (!bindmob->IsAIControlled() && bindmob != this) {

					} else if (bindmob->IsAIControlled() && bindmob != this) {
						// Tell IPC to resume??
					} else {
						// Binding self
					}
					// Send client bind done

					bind_out->type = 1; // Done
					QueuePacket(outapp);
					bind_out->type = 0;
					CheckIncreaseSkill(EQ::skills::SkillBindWound, nullptr, 5);

					if (RuleB(Character, UseOldBindWound)) {
						int maxHPBonus = spellbonuses.MaxBindWound + itembonuses.MaxBindWound +
							aabonuses.MaxBindWound;

						int max_percent = 50 + maxHPBonus;

						if (GetClass() == Class::Monk && GetSkill(EQ::skills::SkillBindWound) > 200) {
							max_percent = 70 + maxHPBonus;
						}

						int64 max_hp = bindmob->GetMaxHP() * max_percent / 100;

						// send bindmob new hp's
						if (bindmob->GetHP() < bindmob->GetMaxHP() && bindmob->GetHP() <= (max_hp)-1) {
							// 0.120 per skill point, 0.60 per skill level, minimum 3 max 30
							int bindhps = 3;

							if (GetSkill(EQ::skills::SkillBindWound) > 200) {
								bindhps += GetSkill(EQ::skills::SkillBindWound) * 4 / 10;
							}
							else if (GetSkill(EQ::skills::SkillBindWound) >= 10) {
								bindhps += GetSkill(EQ::skills::SkillBindWound) / 4;
							}

							// Implementation of aaMithanielsBinding is a guess (the multiplier)
							int bindBonus = spellbonuses.BindWound + itembonuses.BindWound +
								aabonuses.BindWound;

							bindhps += bindhps * bindBonus / 100;

							// if the bind takes them above the max bindable
							// cap it at that value. Dont know if live does it this way
							// but it makes sense to me.
							int chp = bindmob->GetHP() + bindhps;
							if (chp > max_hp)
								chp = max_hp;

							bindmob->SetHP(chp);
							bindmob->SendHPUpdate();
						}
						else {
							// I dont have the real, live
							Message(Chat::Yellow, "You cannot bind wounds above %d%% hitpoints.",
								max_percent);
							if (bindmob != this && bindmob->IsClient())
								bindmob->CastToClient()->Message(
								15,
								"You cannot have your wounds bound above %d%% hitpoints.",
								max_percent);
							// Too many hp message goes here.
						}
					}
					else {
						int percent_base = 50;
						if (GetRawSkill(EQ::skills::SkillBindWound) > 200) {
							if ((GetClass() == Class::Monk) || (GetClass() == Class::Beastlord))
								percent_base = 70;
							else if ((GetLevel() > 50) && ((GetClass() == Class::Warrior) || (GetClass() == Class::Rogue) || (GetClass() == Class::Cleric)))
								percent_base = 70;
						}

						int percent_bonus = spellbonuses.MaxBindWound + itembonuses.MaxBindWound + aabonuses.MaxBindWound;

						int max_percent = percent_base + percent_bonus;
						if (max_percent < 0)
							max_percent = 0;
						if (max_percent > 100)
							max_percent = 100;

						int max_hp = (bindmob->GetMaxHP() * max_percent) / 100;
						if (max_hp > bindmob->GetMaxHP())
							max_hp = bindmob->GetMaxHP();

						if (bindmob->GetHP() < bindmob->GetMaxHP() && bindmob->GetHP() < max_hp) {
							int bindhps = 3; // base bind hp
							if (percent_base >= 70)
								bindhps = (GetSkill(EQ::skills::SkillBindWound) * 4) / 10; // 8:5 skill-to-hp ratio
							else if (GetSkill(EQ::skills::SkillBindWound) >= 12)
								bindhps = GetSkill(EQ::skills::SkillBindWound) / 4; // 4:1 skill-to-hp ratio

							int bonus_hp_percent = spellbonuses.BindWound + itembonuses.BindWound + aabonuses.BindWound;

							bindhps += (bindhps * bonus_hp_percent) / 100;

							if (bindhps < 3)
								bindhps = 3;

							bindhps += bindmob->GetHP();
							if (bindhps > max_hp)
								bindhps = max_hp;

							bindmob->SetHP(bindhps);
							bindmob->SendHPUpdate();
						}
						else {
							Message(Chat::Yellow, "You cannot bind wounds above %d%% hitpoints.", max_percent);
							if (bindmob != this && bindmob->IsClient())
								bindmob->CastToClient()->Message(Chat::Yellow, "You cannot have your wounds bound above %d%% hitpoints.", max_percent);
						}
					}
				}
				else {
					// Send client bind failed
					if (bindmob != this)
						bind_out->type = 6; // They moved
					else
						bind_out->type = 7; // Bandager moved

					QueuePacket(outapp);
					bind_out->type = 0;
				}
			}
		}
	} else if (bindwound_timer.Enabled()) {
		// You moved
		outapp = new EQApplicationPacket(OP_Bind_Wound, sizeof(BindWound_Struct));
		BindWound_Struct *bind_out = (BindWound_Struct *)outapp->pBuffer;
		bindwound_timer.Disable();
		bindwound_target = 0;
		bind_out->type = 7;
		QueuePacket(outapp);
		bind_out->type = 3;
		QueuePacket(outapp);
	}
	safe_delete(outapp);
	return true;
}

void Client::SetMaterial(int16 in_slot, uint32 item_id)
{
	const EQ::ItemData *item = database.GetItem(item_id);
	if (item && item->IsClassCommon()) {
		uint8 matslot = EQ::InventoryProfile::CalcMaterialFromSlot(in_slot);
		if (matslot != EQ::textures::materialInvalid) {
			m_pp.item_material.Slot[matslot].Material = GetEquipmentMaterial(matslot);
		}
	}
}

void Client::ServerFilter(SetServerFilter_Struct* filter){

/*	this code helps figure out the filter IDs in the packet if needed
	static SetServerFilter_Struct ssss;
	int r;
	uint32 *o = (uint32 *) &ssss;
	uint32 *n = (uint32 *) filter;
	for(r = 0; r < (sizeof(SetServerFilter_Struct)/4); r++) {
		if(*o != *n)
			LogFile->write(EQEMuLog::Debug, "Filter %d changed from %d to %d", r, *o, *n);
		o++; n++;
	}
	memcpy(&ssss, filter, sizeof(SetServerFilter_Struct));
*/
#define Filter0(type) \
	if(filter->filters[type] == 1) \
		SetFilter(type, FilterShow); \
	else \
		SetFilter(type, FilterHide);
#define Filter1(type) \
	if(filter->filters[type] == 0) \
		SetFilter(type, FilterShow); \
	else \
		SetFilter(type, FilterHide);

	Filter0(FilterGuildChat);
	Filter0(FilterSocials);
	Filter0(FilterGroupChat);
	Filter0(FilterShouts);
	Filter0(FilterAuctions);
	Filter0(FilterOOC);
	Filter0(FilterBadWords);

	if (filter->filters[FilterPCSpells] == 0) {
		SetFilter(FilterPCSpells, FilterShow);
	} else if (filter->filters[FilterPCSpells] == 1) {
		SetFilter(FilterPCSpells, FilterHide);
	} else {
		SetFilter(FilterPCSpells, FilterShowGroupOnly);
	}

	Filter1(FilterNPCSpells);

	if (filter->filters[FilterBardSongs] == 0) {
		SetFilter(FilterBardSongs, FilterShow);
	} else if (filter->filters[FilterBardSongs] == 1) {
		SetFilter(FilterBardSongs, FilterShowSelfOnly);
	} else if (filter->filters[FilterBardSongs] == 2) {
		SetFilter(FilterBardSongs, FilterShowGroupOnly);
	} else {
		SetFilter(FilterBardSongs, FilterHide);
	}

	if (filter->filters[FilterSpellCrits] == 0) {
		SetFilter(FilterSpellCrits, FilterShow);
	} else if (filter->filters[FilterSpellCrits] == 1) {
		SetFilter(FilterSpellCrits, FilterShowSelfOnly);
	} else {
		SetFilter(FilterSpellCrits, FilterHide);
	}

	if (filter->filters[FilterMeleeCrits] == 0) {
		SetFilter(FilterMeleeCrits, FilterShow);
	} else if (filter->filters[FilterMeleeCrits] == 1) {
		SetFilter(FilterMeleeCrits, FilterShowSelfOnly);
	} else {
		SetFilter(FilterMeleeCrits, FilterHide);
	}

	if (filter->filters[FilterSpellDamage] == 0) {
		SetFilter(FilterSpellDamage, FilterShow);
	} else if (filter->filters[FilterSpellDamage] == 1) {
		SetFilter(FilterSpellDamage, FilterShowSelfOnly);
	} else {
		SetFilter(FilterSpellDamage, FilterHide);
	}

	Filter0(FilterMyMisses);
	Filter0(FilterOthersMiss);
	Filter0(FilterOthersHit);
	Filter0(FilterMissedMe);
	Filter1(FilterDamageShields);

	if (ClientVersionBit() & EQ::versions::maskSoDAndLater) {
		if (filter->filters[FilterDOT] == 0) {
			SetFilter(FilterDOT, FilterShow);
		} else if (filter->filters[FilterDOT] == 1) {
			SetFilter(FilterDOT, FilterShowSelfOnly);
		} else if (filter->filters[FilterDOT] == 2) {
			SetFilter(FilterDOT, FilterShowGroupOnly);
		} else {
			SetFilter(FilterDOT, FilterHide);
		}
	} else {
		if (filter->filters[FilterDOT] == 0) { // show functions as self only
			SetFilter(FilterDOT, FilterShowSelfOnly);
		} else {
			SetFilter(FilterDOT, FilterHide);
		}
	}

	Filter1(FilterPetHits);
	Filter1(FilterPetMisses);
	Filter1(FilterFocusEffects);
	Filter1(FilterPetSpells);

	if (ClientVersionBit() & EQ::versions::maskSoDAndLater) {
		if (filter->filters[FilterHealOverTime] == 0) {
			SetFilter(FilterHealOverTime, FilterShow);
		} else if (filter->filters[FilterHealOverTime] == 1) {
			SetFilter(FilterHealOverTime, FilterShowSelfOnly);
		} else {
			SetFilter(FilterHealOverTime, FilterHide);
		}
	} else { // these clients don't have a 'self only' filter
		Filter1(FilterHealOverTime);
	}

	Filter1(FilterItemSpeech);
	Filter1(FilterStrikethrough);
	Filter1(FilterStuns);
	Filter1(FilterBardSongsOnPets);
}

// this version is for messages with no parameters
void Client::MessageString(uint32 type, uint32 string_id, uint32 distance)
{
	if (GetFilter(FilterSpellDamage) == FilterHide && type == Chat::NonMelee)
		return;
	if (GetFilter(FilterMeleeCrits) == FilterHide && type == Chat::MeleeCrit) //98 is self...
		return;
	if (GetFilter(FilterSpellCrits) == FilterHide && type == Chat::SpellCrit)
		return;
	auto outapp = new EQApplicationPacket(OP_SimpleMessage, 12);
	SimpleMessage_Struct* sms = (SimpleMessage_Struct*)outapp->pBuffer;
	sms->color=type;
	sms->string_id=string_id;

	sms->unknown8=0;

	if(distance>0)
		entity_list.QueueCloseClients(this,outapp,false,distance);
	else
		QueuePacket(outapp);
	safe_delete(outapp);
}

//
// this list of 9 args isn't how I want to do it, but to use va_arg
// you have to know how many args you're expecting, and to do that we have
// to load the eqstr file and count them in the string.
// This hack sucks but it's gonna work for now.
//
void Client::MessageString(uint32 type, uint32 string_id, const char* message1,
	const char* message2,const char* message3,const char* message4,
	const char* message5,const char* message6,const char* message7,
	const char* message8,const char* message9, uint32 distance)
{
	if (GetFilter(FilterSpellDamage) == FilterHide && type == Chat::NonMelee)
		return;
	if (GetFilter(FilterMeleeCrits) == FilterHide && type == Chat::MeleeCrit) //98 is self...
		return;
	if (GetFilter(FilterSpellCrits) == FilterHide && type == Chat::SpellCrit)
		return;
	if (GetFilter(FilterDamageShields) == FilterHide && type == Chat::DamageShield)
		return;
	if (GetFilter(FilterFocusEffects) == FilterHide && type == Chat::FocusEffect)
		return;

	if (type == Chat::Emote)
		type = 4;

	if (!message1) {
		MessageString(type, string_id);	// use the simple message instead
		return;
	}

	const char *message_arg[] = {
		message1, message2, message3, message4, message5,
		message6, message7, message8, message9
	};

	SerializeBuffer buf(20);
	buf.WriteInt32(0); // unknown
	buf.WriteInt32(string_id);
	buf.WriteInt32(type);
	for (auto &m : message_arg) {
		if (m == nullptr)
			break;
		buf.WriteString(m);
	}

	buf.WriteInt8(0); // prevent oob in packet translation, maybe clean that up sometime

	auto outapp = std::make_unique<EQApplicationPacket>(OP_FormattedMessage, buf);

	if (distance > 0)
		entity_list.QueueCloseClients(this, outapp.get(), false, distance);
	else
		QueuePacket(outapp.get());
}

void Client::MessageString(const CZClientMessageString_Struct* msg)
{
	if (msg)
	{
		if (msg->args_size == 0)
		{
			MessageString(msg->chat_type, msg->string_id);
		}
		else
		{
			uint32_t outsize = sizeof(FormattedMessage_Struct) + msg->args_size;
			auto outapp = std::make_unique<EQApplicationPacket>(OP_FormattedMessage, outsize);
			auto outbuf = reinterpret_cast<FormattedMessage_Struct*>(outapp->pBuffer);
			outbuf->string_id = msg->string_id;
			outbuf->type = msg->chat_type;
			memcpy(outbuf->message, msg->args, msg->args_size);
			QueuePacket(outapp.get());
		}
	}
}

// helper function, returns true if we should see the message
bool Client::FilteredMessageCheck(Mob *sender, eqFilterType filter)
{
	eqFilterMode mode = GetFilter(filter);
	// easy ones first
	if (mode == FilterShow) {
		return true;
	} else if (mode == FilterHide) {
		return false;
	}

	if (sender != this && mode == FilterShowSelfOnly) {
		return false;
	} else if (sender) {
		if (mode == FilterShowGroupOnly) {
			auto g = GetGroup();
			auto r = GetRaid();
			if (g) {
				if (g->IsGroupMember(sender)) {
					return true;
				}
			} else if (r && sender->IsClient()) {
				auto rgid1 = r->GetGroup(this);
				auto rgid2 = r->GetGroup(sender->CastToClient());
				if (rgid1 != RAID_GROUPLESS && rgid1 == rgid2) {
					return true;
				}
			} else {
				return false;
			}
		}
	}

	// we passed our checks
	return true;
}

void Client::FilteredMessageString(Mob *sender, uint32 type,
		eqFilterType filter, uint32 string_id)
{
	if (!FilteredMessageCheck(sender, filter))
		return;

	auto outapp = new EQApplicationPacket(OP_SimpleMessage, 12);
	SimpleMessage_Struct *sms = (SimpleMessage_Struct *)outapp->pBuffer;
	sms->color = type;
	sms->string_id = string_id;

	sms->unknown8 = 0;

	QueuePacket(outapp);
	safe_delete(outapp);

	return;
}

void Client::FilteredMessageString(Mob *sender, uint32 type, eqFilterType filter, uint32 string_id,
		const char *message1, const char *message2, const char *message3,
		const char *message4, const char *message5, const char *message6,
		const char *message7, const char *message8, const char *message9)
{
	if (!FilteredMessageCheck(sender, filter))
		return;

	if (type == Chat::Emote)
		type = 4;

	if (!message1) {
		FilteredMessageString(sender, type, filter, string_id);	// use the simple message instead
		return;
	}

	const char *message_arg[] = {
		message1, message2, message3, message4, message5,
		message6, message7, message8, message9
	};

	SerializeBuffer buf(20);
	buf.WriteInt32(0); // unknown
	buf.WriteInt32(string_id);
	buf.WriteInt32(type);
	for (auto &m : message_arg) {
		if (m == nullptr)
			break;
		buf.WriteString(m);
	}

	buf.WriteInt8(0); // prevent oob in packet translation, maybe clean that up sometime

	auto outapp = std::make_unique<EQApplicationPacket>(OP_FormattedMessage, buf);

	QueuePacket(outapp.get());
}

void Client::Tell_StringID(uint32 string_id, const char *who, const char *message)
{
	char string_id_str[10];
	snprintf(string_id_str, 10, "%d", string_id);

	MessageString(Chat::EchoTell, TELL_QUEUED_MESSAGE, who, string_id_str, message);
}

void Client::SetTint(int16 in_slot, uint32 color) {
	EQ::textures::Tint_Struct new_color;
	new_color.Color = color;
	SetTint(in_slot, new_color);
	database.SaveCharacterMaterialColor(CharacterID(), in_slot, color);
}

// Still need to reconcile bracer01 versus bracer02
void Client::SetTint(int16 in_slot, EQ::textures::Tint_Struct& color) {

	uint8 matslot = EQ::InventoryProfile::CalcMaterialFromSlot(in_slot);
	if (matslot != EQ::textures::materialInvalid)
	{
		m_pp.item_tint.Slot[matslot].Color = color.Color;
		database.SaveCharacterMaterialColor(CharacterID(), in_slot, color.Color);
	}

}

void Client::SetHideMe(bool flag)
{
	EQApplicationPacket app;

	gm_hide_me = flag;

	if (gm_hide_me) {
		database.SetHideMe(AccountID(), true);
		CreateDespawnPacket(&app, false);
		entity_list.RemoveFromTargets(this);
		trackable = false;
		if (RuleB(Command, HideMeCommandDisablesTells)) {
			tellsoff  = true;
		}
	} else {
		database.SetHideMe(AccountID(), false);
		CreateSpawnPacket(&app);
		trackable = true;
		tellsoff  = false;
	}

	entity_list.QueueClientsStatus(this, &app, true, 0, Admin() - 1);
	UpdateWho();
}

void Client::SetLanguageSkill(uint8 language_id, uint8 language_skill)
{
	if (!EQ::ValueWithin(language_id, Language::CommonTongue, Language::Unknown27)) {
		return;
	}

	if (language_skill > Language::MaxValue) {
		language_skill = Language::MaxValue;
	}

	m_pp.languages[language_id] = language_skill;

	database.SaveCharacterLanguage(CharacterID(), language_id, language_skill);

	auto outapp = new EQApplicationPacket(OP_SkillUpdate, sizeof(SkillUpdate_Struct));
	auto* s = (SkillUpdate_Struct*) outapp->pBuffer;

	s->skillId = 100 + language_id;
	s->value   = m_pp.languages[language_id];

	QueuePacket(outapp);
	safe_delete(outapp);

	MessageString(Chat::Skills, LANG_SKILL_IMPROVED);
}

void Client::LinkDead()
{
	if (GetGroup())
	{
		entity_list.MessageGroup(this,true,15,"%s has gone linkdead.",GetName());
		GetGroup()->DelMember(this);
		if (GetMerc())
		{
			GetMerc()->RemoveMercFromGroup(GetMerc(), GetMerc()->GetGroup());
		}
	}
	Raid *raid = entity_list.GetRaidByClient(this);
	if(raid){
		raid->MemberZoned(this);
	}

	SetDynamicZoneMemberStatus(DynamicZoneMemberStatus::LinkDead);

//	save_timer.Start(2500);
	linkdead_timer.Start(RuleI(Zone,ClientLinkdeadMS));
	SendAppearancePacket(AppearanceType::Linkdead, 1);
	client_state = CLIENT_LINKDEAD;
	AI_Start(CLIENT_LD_TIMEOUT);
}

uint8 Client::SlotConvert(uint8 slot,bool bracer){
	uint8 slot2 = 0; // why are we returning MainCharm instead of INVALID_INDEX? (must be a pre-charm segment...)
	if(bracer)
		return EQ::invslot::slotWrist2;
	switch(slot) {
	case EQ::textures::armorHead:
		slot2 = EQ::invslot::slotHead;
		break;
	case EQ::textures::armorChest:
		slot2 = EQ::invslot::slotChest;
		break;
	case EQ::textures::armorArms:
		slot2 = EQ::invslot::slotArms;
		break;
	case EQ::textures::armorWrist:
		slot2 = EQ::invslot::slotWrist1;
		break;
	case EQ::textures::armorHands:
		slot2 = EQ::invslot::slotHands;
		break;
	case EQ::textures::armorLegs:
		slot2 = EQ::invslot::slotLegs;
		break;
	case EQ::textures::armorFeet:
		slot2 = EQ::invslot::slotFeet;
		break;
	}
	return slot2;
}

uint8 Client::SlotConvert2(uint8 slot){
	uint8 slot2 = 0; // same as above...
	switch(slot){
	case EQ::invslot::slotHead:
		slot2 = EQ::textures::armorHead;
		break;
	case EQ::invslot::slotChest:
		slot2 = EQ::textures::armorChest;
		break;
	case EQ::invslot::slotArms:
		slot2 = EQ::textures::armorArms;
		break;
	case EQ::invslot::slotWrist1:
		slot2 = EQ::textures::armorWrist;
		break;
	case EQ::invslot::slotHands:
		slot2 = EQ::textures::armorHands;
		break;
	case EQ::invslot::slotLegs:
		slot2 = EQ::textures::armorLegs;
		break;
	case EQ::invslot::slotFeet:
		slot2 = EQ::textures::armorFeet;
		break;
	}
	return slot2;
}

void Client::Escape()
{
	entity_list.RemoveFromTargets(this, true);
	SetInvisible(Invisibility::Invisible);
	MessageString(Chat::Skills, ESCAPE);
}

float Client::CalcClassicPriceMod(Mob* other, bool reverse) {
	float price_multiplier = 0.8f;

	if (other && other->IsNPC()) {
		FACTION_VALUE faction_level = GetFactionLevel(CharacterID(), other->CastToNPC()->GetNPCTypeID(), GetRace(), GetClass(), GetDeity(), other->CastToNPC()->GetPrimaryFaction(), other);
		int32 cha = GetCHA();

		if (faction_level <= FACTION_AMIABLY) {
			cha += 11;		// amiable faction grants a defacto 11 charisma bonus
		}

		uint8 greed = other->CastToNPC()->GetGreedPercent();

		// Sony's precise algorithm is unknown, but this produces output that is virtually identical
		if (faction_level <= FACTION_INDIFFERENTLY) {
			if (cha > 75) {
				if (greed) {
					// this is derived from curve fitting to a lot of price data
					price_multiplier = -0.2487768 + (1.599635 - -0.2487768) / (1 + pow((cha / 135.1495), 1.001983));
					price_multiplier += (greed + 25u) / 100.0f;  // default vendor markup is 25%; anything above that is 'greedy'
					price_multiplier = 1.0f / price_multiplier;
				}
				else {
					// non-greedy merchants use a linear scale
					price_multiplier = 1.0f - ((115.0f - cha) * 0.004f);
				}
			}
			else if (cha > 60) {
				price_multiplier = 1.0f / (1.25f + (greed / 100.0f));
			}
			else {
				price_multiplier = 1.0f / ((1.0f - (cha - 120.0f) / 220.0f) + (greed / 100.0f));
			}
		}
		else { // apprehensive
			if (cha > 75) {
				if (greed) {
					// this is derived from curve fitting to a lot of price data
					price_multiplier = -0.25f + (1.823662 - -0.25f) / (1 + (cha / 135.0f));
					price_multiplier += (greed + 25u) / 100.0f;  // default vendor markup is 25%; anything above that is 'greedy'
					price_multiplier = 1.0f / price_multiplier;
				}
				else {
					price_multiplier = (100.0f - (145.0f - cha) / 2.8f) / 100.0f;
				}
			}
			else if (cha > 60) {
				price_multiplier = 1.0f / (1.4f + greed / 100.0f);
			}
			else {
				price_multiplier = 1.0f / ((1.0f + (143.574 - cha) / 196.434) + (greed / 100.0f));
			}
		}

		float maxResult = 1.0f / 1.05;		// price reduction caps at this amount
		if (price_multiplier > maxResult) {
			price_multiplier = maxResult;
		}

		if (!reverse) {
			price_multiplier = 1.0f / price_multiplier;
		}
	}

	LogMerchants(
		"[{}] [{}] items at [{}] price multiplier [{}] [{}]",
		other->GetName(),
		reverse ? "buys" : "sells",
		price_multiplier,
		reverse ? "from" : "to",
		GetName()
	);

	return price_multiplier;
}

float Client::CalcNewPriceMod(Mob* other, bool reverse)
{
	float chaformula = 0;
	if (other)
	{
		int factionlvl = GetFactionLevel(CharacterID(), other->CastToNPC()->GetNPCTypeID(), GetFactionRace(), GetClass(), GetDeity(), other->CastToNPC()->GetPrimaryFaction(), other);
		if (factionlvl >= FACTION_APPREHENSIVELY) // Apprehensive or worse.
		{
			if (GetCHA() > 103)
			{
				chaformula = (GetCHA() - 103)*((-(RuleR(Merchant, ChaBonusMod))/100)*(RuleI(Merchant, PriceBonusPct))); // This will max out price bonus.
				if (chaformula < -1*(RuleI(Merchant, PriceBonusPct)))
					chaformula = -1*(RuleI(Merchant, PriceBonusPct));
			}
			else if (GetCHA() < 103)
			{
				chaformula = (103 - GetCHA())*(((RuleR(Merchant, ChaPenaltyMod))/100)*(RuleI(Merchant, PricePenaltyPct))); // This will bottom out price penalty.
				if (chaformula > 1*(RuleI(Merchant, PricePenaltyPct)))
					chaformula = 1*(RuleI(Merchant, PricePenaltyPct));
			}
		}
		if (factionlvl <= FACTION_INDIFFERENTLY) // Indifferent or better.
		{
			if (GetCHA() > 75)
			{
				chaformula = (GetCHA() - 75)*((-(RuleR(Merchant, ChaBonusMod))/100)*(RuleI(Merchant, PriceBonusPct))); // This will max out price bonus.
				if (chaformula < -1*(RuleI(Merchant, PriceBonusPct)))
					chaformula = -1*(RuleI(Merchant, PriceBonusPct));
			}
			else if (GetCHA() < 75)
			{
				chaformula = (75 - GetCHA())*(((RuleR(Merchant, ChaPenaltyMod))/100)*(RuleI(Merchant, PricePenaltyPct))); // Faction modifier keeps up from reaching bottom price penalty.
				if (chaformula > 1*(RuleI(Merchant, PricePenaltyPct)))
					chaformula = 1*(RuleI(Merchant, PricePenaltyPct));
			}
		}
	}

	if (reverse)
		chaformula *= -1; //For selling
	//Now we have, for example, 10
	chaformula /= 100; //Convert to 0.10
	chaformula += 1; //Convert to 1.10;
	return chaformula; //Returns 1.10, expensive stuff!
}

float Client::CalcPriceMod(Mob* other, bool reverse)
{
	float price_mod = CalcNewPriceMod(other, reverse);

	if (RuleB(Merchant, UseClassicPriceMod)) {
		price_mod = CalcClassicPriceMod(other, reverse);
	}

	return price_mod;
}

void Client::GetGroupAAs(GroupLeadershipAA_Struct *into) const {
	memcpy(into, &m_pp.leader_abilities.group, sizeof(GroupLeadershipAA_Struct));
}

void Client::GetRaidAAs(RaidLeadershipAA_Struct *into) const {
	memcpy(into, &m_pp.leader_abilities.raid, sizeof(RaidLeadershipAA_Struct));
}

void Client::EnteringMessages(Client* client)
{
	std::string rules = RuleS(World, Rules);

	if (!rules.empty() || database.GetVariable("Rules", rules)) {
		const uint8 flag = database.GetAgreementFlag(client->AccountID());
		if (!flag) {
			const std::string& rules_link = Saylink::Silent("#serverrules", "rules");

			client->Message(
				Chat::White,
				fmt::format(
					"You must agree to the {} before you can move.",
					rules_link
				).c_str()
			);

			client->SendAppearancePacket(AppearanceType::Animation, Animation::Freeze);
		}
	}
}

void Client::SendRules()
{
	std::string rules = RuleS(World, Rules);

	if (rules.empty() && !database.GetVariable("Rules", rules)) {
		return;
	}

	auto lines = Strings::Split(rules, "|");
	auto line_number = 1;
	for (auto&& line : lines) {
		Message(
			Chat::White,
			fmt::format(
				"{}. {}",
				line_number,
				line
			).c_str()
		);
		line_number++;
	}
}

void Client::SetEndurance(int32 newEnd)
{
	/*Endurance can't be less than 0 or greater than max*/
	if(newEnd < 0)
		newEnd = 0;
	else if(newEnd > GetMaxEndurance()){
		newEnd = GetMaxEndurance();
	}

	current_endurance = newEnd;
	CheckManaEndUpdate();
}

void Client::SacrificeConfirm(Mob *caster)
{
	auto outapp = new EQApplicationPacket(OP_Sacrifice, sizeof(Sacrifice_Struct));
	Sacrifice_Struct *ss = (Sacrifice_Struct *)outapp->pBuffer;

	if (!caster || PendingSacrifice) {
		safe_delete(outapp);
		return;
	}

	if (GetLevel() < RuleI(Spells, SacrificeMinLevel)) {
		caster->MessageString(Chat::Red, SAC_TOO_LOW); // This being is not a worthy sacrifice.
		safe_delete(outapp);
		return;
	}

	if (GetLevel() > RuleI(Spells, SacrificeMaxLevel)) {
		caster->MessageString(Chat::Red, SAC_TOO_HIGH);
		safe_delete(outapp);
		return;
	}

	ss->CasterID = caster->GetID();
	ss->TargetID = GetID();
	ss->Confirm = 0;
	QueuePacket(outapp);
	safe_delete(outapp);
	// We store the Caster's id, because when the packet comes back, it only has the victim's entityID in it,
	// not the caster.
	sacrifice_caster_id = caster->GetID();
	PendingSacrifice = true;
}

//Essentially a special case death function
void Client::Sacrifice(Mob *caster)
{
	if (GetLevel() >= RuleI(Spells, SacrificeMinLevel) && GetLevel() <= RuleI(Spells, SacrificeMaxLevel)) {
		int exploss = (int)(GetLevel() * (GetLevel() / 18.0) * 12000);
		if (exploss < GetEXP()) {
			SetEXP(ExpSource::Sacrifice, GetEXP() - exploss, GetAAXP(), false);
			SendLogoutPackets();

			// make our become corpse packet, and queue to ourself before OP_Death.
			EQApplicationPacket app2(OP_BecomeCorpse, sizeof(BecomeCorpse_Struct));
			BecomeCorpse_Struct *bc = (BecomeCorpse_Struct *)app2.pBuffer;
			bc->spawn_id = GetID();
			bc->x = GetX();
			bc->y = GetY();
			bc->z = GetZ();
			QueuePacket(&app2);

			// make death packet
			EQApplicationPacket app(OP_Death, sizeof(Death_Struct));
			Death_Struct *d = (Death_Struct *)app.pBuffer;
			d->spawn_id = GetID();
			d->killer_id = caster ? caster->GetID() : 0;
			d->bindzoneid = GetPP().binds[0].zone_id;
			d->spell_id = SPELL_UNKNOWN;
			d->attack_skill = 0xe7;
			d->damage = 0;
			app.priority = 6;
			entity_list.QueueClients(this, &app);

			BuffFadeAll();
			UnmemSpellAll();
			Group *g = GetGroup();
			if (g) {
				g->MemberZoned(this);
			}
			Raid *r = entity_list.GetRaidByClient(this);
			if (r) {
				r->MemberZoned(this);
			}
			ClearAllProximities();
			if (RuleB(Character, LeaveCorpses)) {
				auto new_corpse = new Corpse(this, 0);
				entity_list.AddCorpse(new_corpse, GetID());
				SetID(0);
				entity_list.QueueClients(this, &app2, true);
			}
			Save();
			GoToDeath();
			if (caster && caster->IsClient()) {
				caster->CastToClient()->SummonItem(RuleI(Spells, SacrificeItemID));
			} else if (caster && caster->IsNPC()) {
				caster->CastToNPC()->AddItem(RuleI(Spells, SacrificeItemID), 1, false);
			}
		}
	} else {
		caster->MessageString(Chat::Red, SAC_TOO_LOW); // This being is not a worthy sacrifice.
	}
}

void Client::SendOPTranslocateConfirm(Mob *Caster, uint16 SpellID) {

	if(!Caster || PendingTranslocate)
		return;

	const SPDat_Spell_Struct &Spell = spells[SpellID];

	auto outapp = new EQApplicationPacket(OP_Translocate, sizeof(Translocate_Struct));
	Translocate_Struct *ts = (Translocate_Struct*)outapp->pBuffer;

	strcpy(ts->Caster, Caster->GetName());
	PendingTranslocateData.spell_id = ts->SpellID = SpellID;

	if((SpellID == 1422) || (SpellID == 1334) || (SpellID == 3243)) {
		PendingTranslocateData.zone_id = ts->ZoneID = m_pp.binds[0].zone_id;
		PendingTranslocateData.instance_id = m_pp.binds[0].instance_id;
		PendingTranslocateData.x = ts->x = m_pp.binds[0].x;
		PendingTranslocateData.y = ts->y = m_pp.binds[0].y;
		PendingTranslocateData.z = ts->z = m_pp.binds[0].z;
		PendingTranslocateData.heading = m_pp.binds[0].heading;
	}
	else {
		PendingTranslocateData.zone_id = ts->ZoneID = ZoneID(Spell.teleport_zone);
		PendingTranslocateData.instance_id = 0;
		PendingTranslocateData.y = ts->y = Spell.base_value[0];
		PendingTranslocateData.x = ts->x = Spell.base_value[1];
		PendingTranslocateData.z = ts->z = Spell.base_value[2];
		PendingTranslocateData.heading = 0.0;
	}

	ts->unknown008 = 0;
	ts->Complete = 0;

	PendingTranslocate = true;
	TranslocateTime = time(nullptr);

	QueuePacket(outapp);
	safe_delete(outapp);

	return;
}
void Client::SendPickPocketResponse(Mob *from, uint32 amt, int type, const EQ::ItemData* item){
	auto outapp = new EQApplicationPacket(OP_PickPocket, sizeof(sPickPocket_Struct));
	sPickPocket_Struct *pick_out = (sPickPocket_Struct *)outapp->pBuffer;
	pick_out->coin = amt;
	pick_out->from = GetID();
	pick_out->to = from->GetID();
		pick_out->myskill = GetSkill(EQ::skills::SkillPickPockets);

	if ((type >= PickPocketPlatinum) && (type <= PickPocketCopper) && (amt == 0))
		type = PickPocketFailed;

	pick_out->type = type;
	if (item)
		strcpy(pick_out->itemname, item->Name);
	else
		pick_out->itemname[0] = '\0';
	// if we do not send this packet the client will lock up and require the player to relog.
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SetHoTT(uint32 mobid) {
	auto outapp = new EQApplicationPacket(OP_TargetHoTT, sizeof(ClientTarget_Struct));
	ClientTarget_Struct *ct = (ClientTarget_Struct *) outapp->pBuffer;
	ct->new_target = mobid;
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendPopupToClient(const char *Title, const char *Text, uint32 PopupID, uint32 Buttons, uint32 Duration)
{
	auto outapp = new EQApplicationPacket(OP_OnLevelMessage, sizeof(OnLevelMessage_Struct));

	OnLevelMessage_Struct *olms = (OnLevelMessage_Struct *) outapp->pBuffer;

	if ((strlen(Title) > (sizeof(olms->Title) - 1)) || (strlen(Text) > (sizeof(olms->Text) - 1))) {
		safe_delete(outapp);
		return;
	}

	strcpy(olms->Title, Title);
	strcpy(olms->Text, Text);

	olms->Buttons = Buttons;

	if (Duration > 0) {
		olms->Duration = Duration * 1000;
	}
	else {
		olms->Duration = 0xffffffff;
	}

	olms->PopupID    = PopupID;
	olms->NegativeID = 0;

	sprintf(olms->ButtonName0, "%s", "Yes");
	sprintf(olms->ButtonName1, "%s", "No");
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendFullPopup(
	const char *Title,
	const char *Text,
	uint32 PopupID,
	uint32 NegativeID,
	uint32 Buttons,
	uint32 Duration,
	const char *ButtonName0,
	const char *ButtonName1,
	uint32 SoundControls
)
{
	auto outapp = new EQApplicationPacket(OP_OnLevelMessage, sizeof(OnLevelMessage_Struct));

	OnLevelMessage_Struct *olms = (OnLevelMessage_Struct *) outapp->pBuffer;

	if ((strlen(Text) > (sizeof(olms->Text) - 1)) || (strlen(Title) > (sizeof(olms->Title) - 1))) {
		safe_delete(outapp);
		return;
	}

	if (ButtonName0 && ButtonName1 && ((strlen(ButtonName0) > (sizeof(olms->ButtonName0) - 1)) ||
									   (strlen(ButtonName1) > (sizeof(olms->ButtonName1) - 1)))) {
		safe_delete(outapp);
		return;
	}

	strcpy(olms->Title, Title);
	strcpy(olms->Text, Text);

	olms->Buttons = Buttons;

	if (ButtonName0 == nullptr || ButtonName1 == nullptr) {
		sprintf(olms->ButtonName0, "%s", "Yes");
		sprintf(olms->ButtonName1, "%s", "No");
	}
	else {
		strcpy(olms->ButtonName0, ButtonName0);
		strcpy(olms->ButtonName1, ButtonName1);
	}

	if (Duration > 0) {
		olms->Duration = Duration * 1000;
	}
	else {
		olms->Duration = 0xffffffff;
	}

	olms->PopupID       = PopupID;
	olms->NegativeID    = NegativeID;
	olms->SoundControls = SoundControls;

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendWindow(
	uint32 button_one_id,
	uint32 button_two_id,
	uint32 button_type,
	const char* button_one_text,
	const char* button_two_text,
	uint32 duration,
	int title_type,
	Mob* target,
	const char* title,
	const char* text,
	...
)
{
	va_list argptr;
	char    buffer[4096];

	va_start(argptr, text);
	vsnprintf(buffer, sizeof(buffer), text, argptr);
	va_end(argptr);

	size_t len = strlen(buffer);

	auto  app  = new EQApplicationPacket(OP_OnLevelMessage, sizeof(OnLevelMessage_Struct));
	auto* olms = (OnLevelMessage_Struct *) app->pBuffer;

	if (strlen(text) > (sizeof(olms->Text) - 1)) {
		safe_delete(app);
		return;
	}

	if (!target) {
		title_type = 0;
	}

	switch (title_type) {
		case 1: {
			char name[64] = "";
			strcpy(name, target->GetName());

			if (strlen(target->GetLastName()) > 0) {
				char last_name[64] = "";
				strcpy(last_name, target->GetLastName());
				strcat(name, " ");
				strcat(name, last_name);
			}

			strcpy(olms->Title, name);
			break;
		}
		case 2: {
			if (target->IsClient() && target->CastToClient()->GuildID()) {
				auto guild_name = guild_mgr.GetGuildName(target->CastToClient()->GuildID());
				strn0cpy(olms->Title, guild_name, sizeof(olms->Title));
			} else {
				strcpy(olms->Title, "No Guild");
			}
			break;
		}
		default: {
			strcpy(olms->Title, title);
			break;
		}
	}

	memcpy(olms->Text, buffer, len + 1);

	olms->Buttons = button_type;

	strn0cpy(olms->ButtonName0, button_one_text, sizeof(olms->ButtonName0));
	strn0cpy(olms->ButtonName1, button_two_text, sizeof(olms->ButtonName1));

	if (duration > 0) {
		olms->Duration = duration * 1000;
	} else {
		olms->Duration = UINT32_MAX;
	}

	olms->PopupID    = button_one_id;
	olms->NegativeID = button_two_id;

	FastQueuePacket(&app);
}

void Client::KeyRingLoad()
{
	const auto &l = KeyringRepository::GetWhere(
		database,
		fmt::format(
			"`char_id` = {} ORDER BY `item_id` ASC",
			character_id
		)
	);

	if (l.empty()) {
		return;
	}

	for (const auto& e : l) {
		keyring.emplace_back(e.item_id);
	}
}

bool Client::KeyRingAdd(uint32 item_id)
{
	if (!item_id || KeyRingCheck(item_id)) {
		return false;
	}

	auto e = KeyringRepository::NewEntity();

	e.char_id = CharacterID();
	e.item_id = item_id;

	e = KeyringRepository::InsertOne(database, e);

	if (!e.id) {
		return false;
	}

	keyring.emplace_back(item_id);

	if (!RuleB(World, UseItemLinksForKeyRing)) {
		Message(Chat::LightBlue, "Added to keyring.");
		return true;
	}

	const std::string &item_link = database.CreateItemLink(item_id);

	Message(
		Chat::LightBlue,
		fmt::format(
			"Added {} to keyring.",
			item_link
		).c_str()
	);
	return true;
}

bool Client::KeyRingCheck(uint32 item_id)
{
	return std::find(keyring.begin(), keyring.end(), item_id) != keyring.end();
}

bool Client::KeyRingClear()
{
	keyring.clear();

	return KeyringRepository::DeleteWhere(
		database,
		fmt::format(
			"`char_id` = {}",
			CharacterID()
		)
	);
}

void Client::KeyRingList(Client* c)
{
	if (!c) {
		return;
	}

	std::string message = "Keys on Keyring:";
	if (c != this) {
		message = fmt::format(
			"Keys on Keyring for {}:",
			GetCleanName()
		);
	}

	c->Message(Chat::LightBlue, message.c_str());

	const EQ::ItemData *item = nullptr;

	for (const uint32& e : keyring) {
		item = database.GetItem(e);
		if (item) {
			const std::string& item_string = (
				RuleB(World, UseItemLinksForKeyRing) ?
				database.CreateItemLink(e) :
				item->Name
			);

			c->Message(Chat::LightBlue, item_string.c_str());
		}
	}
}

bool Client::KeyRingRemove(uint32 item_id)
{
	keyring.erase(
		std::remove(
			keyring.begin(),
			keyring.end(),
			item_id
		)
	);

	return KeyringRepository::DeleteWhere(
		database,
		fmt::format(
			"`char_id` = {} AND `item_id` = {}",
			CharacterID(),
			item_id
		)
	);
}

bool Client::IsNameChangeAllowed() {
	if (RuleB(Character, AlwaysAllowNameChange)) {
		return true;
	}

	auto k = GetScopedBucketKeys();
	k.key = "name_change_allowed";

	auto b = DataBucket::GetData(&database, k);
	if (!b.value.empty()) {
		return true;
	}

	return false;
}

bool Client::ClearNameChange() {
	if (!IsNameChangeAllowed()) {
		return false;
	}

	auto k = GetScopedBucketKeys();
	k.key = "name_change_allowed";

	DataBucket::DeleteData(&database, k);

	return true;
}

void Client::InvokeChangeNameWindow(bool immediate) {
	if (!IsNameChangeAllowed()) {
		return;
	}

	auto packet_op = immediate ? OP_InvokeNameChangeImmediate : OP_InvokeNameChangeLazy;

	auto outapp = new EQApplicationPacket(packet_op, 0);
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::GrantNameChange() {

	auto k = GetScopedBucketKeys();
	k.key = "name_change_allowed";
	k.value = "allowed"; // potentially put a timestamp here
	DataBucket::SetData(&database, k);

	InvokeChangeNameWindow(true);
}

bool Client::IsPetNameChangeAllowed() {
	if (RuleB(Pets, AlwaysAllowPetRename)) {
		return true;
	}

	DataBucketKey k = GetScopedBucketKeys();
	k.key = "PetNameChangesAllowed";

	auto b = DataBucket::GetData(&database, k);
	if (!b.value.empty()) {
		return true;
	}

	return false;
}

void Client::InvokeChangePetName(bool immediate) {
	if (!IsPetNameChangeAllowed()) {
		return;
	}

	auto packet_op = immediate ? OP_InvokeChangePetNameImmediate : OP_InvokeChangePetName;

	auto outapp = new EQApplicationPacket(packet_op, 0);
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::GrantPetNameChange() {
	DataBucketKey k = GetScopedBucketKeys();
	k.key = "PetNameChangesAllowed";
	k.value = "true";
	DataBucket::SetData(&database, k);

	InvokeChangePetName(true);
}

void Client::ClearPetNameChange() {
	DataBucketKey k = GetScopedBucketKeys();
	k.key = "PetNameChangesAllowed";

	DataBucket::DeleteData(&database, k);
}

bool Client::ChangePetName(std::string new_name)
{
	if (new_name.empty()) {
		return false;
	}

	if (!IsPetNameChangeAllowed()) {
		return false;
	}

	auto pet = GetPet();
	if (!pet) {
		return false;
	}

	if (pet->GetName() == new_name) {
		return false;
	}

	if (!database.CheckNameFilter(new_name) || database.IsNameUsed(new_name)) {
		return false;
	}

	CharacterPetNameRepository::ReplaceOne(
		database,
		CharacterPetNameRepository::CharacterPetName{
			.character_id = static_cast<int32_t>(CharacterID()),
			.name = new_name
		}
	);

	pet->TempName(new_name.c_str());

	ClearPetNameChange();
	return true;
}

bool Client::IsDiscovered(uint32 item_id)
{
	if (
		std::find(
			zone->discovered_items.begin(),
			zone->discovered_items.end(),
			item_id
		) != zone->discovered_items.end()
	) {
		return true;
	}

	if (
		DiscoveredItemsRepository::GetWhere(
			database,
			fmt::format(
				"`item_id` = {} LIMIT 1",
				item_id
			)
		).empty()
	) {
		return false;
	}

	zone->discovered_items.emplace_back(item_id);

	return true;
}

void Client::DiscoverItem(uint32 item_id) {
	auto e = DiscoveredItemsRepository::NewEntity();

	e.account_status = Admin();
	e.char_name = GetCleanName();
	e.discovered_date = std::time(nullptr);
	e.item_id = item_id;

	auto d = DiscoveredItemsRepository::InsertOne(database, e);

	if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::DISCOVER_ITEM)) {
		const auto* item = database.GetItem(item_id);

		auto e = PlayerEvent::DiscoverItemEvent{
			.item_id = item_id,
			.item_name = item->Name,
		};
		RecordPlayerEventLog(PlayerEvent::DISCOVER_ITEM, e);

	}

	if (parse->PlayerHasQuestSub(EVENT_DISCOVER_ITEM)) {
		auto* item = database.CreateItem(item_id);
		std::vector<std::any> args = { item };

		parse->EventPlayer(EVENT_DISCOVER_ITEM, this, "", item_id, &args);
	}
}

void Client::UpdateLFP() {

	Group *g = GetGroup();

	if(g && !g->IsLeader(this)) {
		database.SetLFP(CharacterID(), false);
		worldserver.StopLFP(CharacterID());
		LFP = false;
		return;
	}

	GroupLFPMemberEntry LFPMembers[MAX_GROUP_MEMBERS];

	for(unsigned int i=0; i<MAX_GROUP_MEMBERS; i++) {
		LFPMembers[i].Name[0] = '\0';
		LFPMembers[i].Class = Class::None;
		LFPMembers[i].Level = 0;
		LFPMembers[i].Zone = 0;
	}

	// Slot 0 is always for the group leader, or the player if not in a group
	strcpy(LFPMembers[0].Name, GetName());
	LFPMembers[0].Class = GetClass();
	LFPMembers[0].Level = GetLevel();
	LFPMembers[0].Zone = zone->GetZoneID();

	if(g) {
		// Fill the LFPMembers array with the rest of the group members, excluding ourself
		// We don't fill in the class, level or zone, because we may not be able to determine
		// them if the other group members are not in this zone. World will fill in this information
		// for us, if it can.
		int NextFreeSlot = 1;
		for(unsigned int i = 0; i < MAX_GROUP_MEMBERS; i++) {
			if((g->membername[i][0] != '\0') && strcasecmp(g->membername[i], LFPMembers[0].Name))
				strcpy(LFPMembers[NextFreeSlot++].Name, g->membername[i]);
		}
	}
	worldserver.UpdateLFP(CharacterID(), LFPMembers);
}

bool Client::GroupFollow(Client* inviter) {

	if (inviter) {
		isgrouped = true;
		Raid* raid = entity_list.GetRaidByClient(inviter);
		Raid* iraid = entity_list.GetRaidByClient(this);

		//inviter has a raid don't do group stuff instead do raid stuff!
		if (raid) {
			// Suspend the merc while in a raid (maybe a rule could be added for this)
			if (GetMerc()) {
				GetMerc()->Suspend();
			}

			uint32 groupToUse = 0xFFFFFFFF;
			for (const auto& m : raid->members) {
				if (m.member && m.member == inviter) {
					groupToUse = m.group_number;
					break;
				}
			}
			if (iraid == raid) {
				//both in same raid
				uint32 ngid = raid->GetGroup(inviter->GetName());
				if (raid->GroupCount(ngid) < MAX_GROUP_MEMBERS) {
					raid->MoveMember(GetName(), ngid);
					raid->SendGroupDisband(this);
					raid->GroupUpdate(ngid);
				}
				return false;
			}
			if (raid->RaidCount() < MAX_RAID_MEMBERS)
			{
				// okay, so we now have a single client (this) joining a group in a raid
				// And they're not already in the raid (which is above and doesn't need xtarget shit)
				if (!GetXTargetAutoMgr()->empty()) {
					raid->GetXTargetAutoMgr()->merge(*GetXTargetAutoMgr());
					GetXTargetAutoMgr()->clear();
					RemoveAutoXTargets();
				}

				SetXTargetAutoMgr(raid->GetXTargetAutoMgr());
				if (!GetXTargetAutoMgr()->empty())
					SetDirtyAutoHaters();

				if (raid->GroupCount(groupToUse) < MAX_GROUP_MEMBERS)
				{
					raid->SendRaidCreate(this);
					raid->SendMakeLeaderPacketTo(raid->leadername, this);
					raid->AddMember(this, groupToUse);
					raid->SendBulkRaid(this);
					//raid->SendRaidGroupAdd(GetName(), groupToUse);
					//raid->SendGroupUpdate(this);
					raid->GroupUpdate(groupToUse); //break
					if (raid->IsLocked())
					{
						raid->SendRaidLockTo(this);
					}
					return false;
				}
				else
				{
					raid->SendRaidCreate(this);
					raid->SendMakeLeaderPacketTo(raid->leadername, this);
					raid->AddMember(this);
					raid->SendBulkRaid(this);
					if (raid->IsLocked())
					{
						raid->SendRaidLockTo(this);
					}
					return false;
				}
			}
		}

		Group* group = entity_list.GetGroupByClient(inviter);

		if (!group)
		{
			//Make new group
			group = new Group(inviter);

			if (!group)
			{
				return false;
			}

			entity_list.AddGroup(group);

			if (group->GetID() == 0)
			{
				Message(Chat::Red, "Unable to get new group id. Cannot create group.");
				inviter->Message(Chat::Red, "Unable to get new group id. Cannot create group.");
				return false;
			}

			//now we have a group id, can set inviter's id
			group->AddToGroup(inviter);
			database.SetGroupLeaderName(group->GetID(), inviter->GetName());
			group->UpdateGroupAAs();

			//Invite the inviter into the group first.....dont ask
			if (inviter->ClientVersion() < EQ::versions::ClientVersion::SoD)
			{
				auto outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
				GroupJoin_Struct* outgj = (GroupJoin_Struct*)outapp->pBuffer;
				strcpy(outgj->membername, inviter->GetName());
				strcpy(outgj->yourname, inviter->GetName());
				outgj->action = groupActInviteInitial; // 'You have formed the group'.
				group->GetGroupAAs(&outgj->leader_aas);
				inviter->QueuePacket(outapp);
				safe_delete(outapp);
			}
			else
			{
				// SoD and later
				inviter->SendGroupCreatePacket();
				inviter->SendGroupLeaderChangePacket(inviter->GetName());
				inviter->SendGroupJoinAcknowledge();
			}
			group->GetXTargetAutoMgr()->merge(*inviter->GetXTargetAutoMgr());
			inviter->GetXTargetAutoMgr()->clear();
			inviter->SetXTargetAutoMgr(group->GetXTargetAutoMgr());
		}

		if (!group)
		{
			return false;
		}

		// Remove merc from old group before adding client to the new one
		if (GetMerc() && GetMerc()->HasGroup())
		{
			GetMerc()->RemoveMercFromGroup(GetMerc(), GetMerc()->GetGroup());
		}

		if (!group->AddMember(this))
		{
			// If failed to add client to new group, regroup with merc
			if (GetMerc())
			{
				GetMerc()->MercJoinClientGroup();
			}
			else
			{
				isgrouped = false;
			}
			return false;
		}

		if (ClientVersion() >= EQ::versions::ClientVersion::SoD)
		{
			SendGroupJoinAcknowledge();
		}

		// Temporary hack for SoD, as things seem to work quite differently
		if (inviter->IsClient() && inviter->ClientVersion() >= EQ::versions::ClientVersion::SoD)
		{
			database.RefreshGroupFromDB(inviter);
		}

		// Add the merc back into the new group if possible
		if (GetMerc())
		{
			GetMerc()->MercJoinClientGroup();
		}

		if (inviter->IsLFP())
		{
			// If the player who invited us to a group is LFP, have them update world now that we have joined their group.
			inviter->UpdateLFP();
		}

		database.RefreshGroupFromDB(this);
		group->SendHPManaEndPacketsTo(this);
		//send updates to clients out of zone...
		group->SendGroupJoinOOZ(this);
		return true;
	}
	return false;
}

uint16 Client::GetPrimarySkillValue()
{
	EQ::skills::SkillType skill = EQ::skills::HIGHEST_SKILL; //because nullptr == 0, which is 1H Slashing, & we want it to return 0 from GetSkill
	bool equipped = m_inv.GetItem(EQ::invslot::slotPrimary);

	if (!equipped)
		skill = EQ::skills::SkillHandtoHand;

	else {

		uint8 type = m_inv.GetItem(EQ::invslot::slotPrimary)->GetItem()->ItemType; //is this the best way to do this?

		switch (type) {
		case EQ::item::ItemType1HSlash: // 1H Slashing
			skill = EQ::skills::Skill1HSlashing;
			break;
		case EQ::item::ItemType2HSlash: // 2H Slashing
			skill = EQ::skills::Skill2HSlashing;
			break;
		case EQ::item::ItemType1HPiercing: // Piercing
			skill = EQ::skills::Skill1HPiercing;
			break;
		case EQ::item::ItemType1HBlunt: // 1H Blunt
			skill = EQ::skills::Skill1HBlunt;
			break;
		case EQ::item::ItemType2HBlunt: // 2H Blunt
			skill = EQ::skills::Skill2HBlunt;
			break;
		case EQ::item::ItemType2HPiercing: // 2H Piercing
			if (IsClient() && CastToClient()->ClientVersion() < EQ::versions::ClientVersion::RoF2)
				skill = EQ::skills::Skill1HPiercing;
			else
				skill = EQ::skills::Skill2HPiercing;
			break;
		case EQ::item::ItemTypeMartial: // Hand to Hand
			skill = EQ::skills::SkillHandtoHand;
			break;
		default: // All other types default to Hand to Hand
			skill = EQ::skills::SkillHandtoHand;
			break;
		}
	}

	return GetSkill(skill);
}

uint32 Client::GetTotalATK()
{
	uint32 AttackRating = 0;
	uint32 WornCap = itembonuses.ATK;

	if(IsClient()) {
		AttackRating = ((WornCap * 1.342) + (GetSkill(EQ::skills::SkillOffense) * 1.345) + ((GetSTR() - 66) * 0.9) + (GetPrimarySkillValue() * 2.69));
		AttackRating += aabonuses.ATK + GroupLeadershipAAOffenseEnhancement();

		if (AttackRating < 10)
			AttackRating = 10;
	}
	else
		AttackRating = GetATK();

	AttackRating += spellbonuses.ATK;

	return AttackRating;
}

uint32 Client::GetATKRating()
{
	uint32 AttackRating = 0;
	if(IsClient()) {
		AttackRating = (GetSkill(EQ::skills::SkillOffense) * 1.345) + ((GetSTR() - 66) * 0.9) + (GetPrimarySkillValue() * 2.69);

		if (AttackRating < 10)
			AttackRating = 10;
	}
	return AttackRating;
}

void Client::VoiceMacroReceived(uint32 Type, char *Target, uint32 MacroNumber) {

	uint32 GroupOrRaidID = 0;

	switch(Type) {

		case VoiceMacroGroup: {

			Group* g = GetGroup();

			if(g)
				GroupOrRaidID = g->GetID();
			else
				return;

			break;
		}

		case VoiceMacroRaid: {

			Raid* r = GetRaid();

			if(r)
				GroupOrRaidID = r->GetID();
			else
				return;

			break;
		}
	}

	if(!worldserver.SendVoiceMacro(this, Type, Target, MacroNumber, GroupOrRaidID))
		Message(0, "Error: World server disconnected");
}

void Client::ClearGroupAAs() {
	for(unsigned int i = 0; i < MAX_GROUP_LEADERSHIP_AA_ARRAY; i++)
		m_pp.leader_abilities.ranks[i] = 0;

	m_pp.group_leadership_points = 0;
	m_pp.raid_leadership_points = 0;
	m_pp.group_leadership_exp = 0;
	m_pp.raid_leadership_exp = 0;

	Save();
	database.SaveCharacterLeadershipAbilities(CharacterID(), &m_pp);
}

void Client::UpdateGroupAAs(int32 points, uint32 type) {
	switch(type) {
		case 0: { m_pp.group_leadership_points += points; break; }
		case 1: { m_pp.raid_leadership_points += points; break; }
	}
	SendLeadershipEXPUpdate();
}

bool Client::IsLeadershipEXPOn() {

	if(!m_pp.leadAAActive)
		return false;

	Group *g = GetGroup();

	if (g && g->IsLeader(this) && g->GroupCount() > 2)
		return true;

	Raid *r = GetRaid();

	if (!r)
		return false;

	// raid leaders can only gain raid AA XP
	if (r->IsLeader(this)) {
		if (r->RaidCount() > 17)
			return true;
		else
			return false;
	}

	uint32 gid = r->GetGroup(this);

	if (gid > 11) // not in a group
		return false;

	if (r->IsGroupLeader(GetName()) && r->GroupCount(gid) > 2)
		return true;

	return false;

}

uint32 Client::GetAggroCount() {
	return AggroCount;
}

// we pass in for book keeping if RestRegen is enabled
void Client::IncrementAggroCount(bool raid_target)
{
	// This method is called when a client is added to a mob's hate list. It turns the clients aggro flag on so
	// rest state regen is stopped, and for SoF, it sends the opcode to show the crossed swords in-combat indicator.
	AggroCount++;

	if(!RuleB(Character, RestRegenEnabled))
		return;

	uint32 newtimer = raid_target ? RuleI(Character, RestRegenRaidTimeToActivate) : RuleI(Character, RestRegenTimeToActivate);

	// When our aggro count is 1 here, we are exiting rest state. We need to pause our current timer, if we have time remaining
	// We should not actually have to do anything to the Timer object since the AggroCount counter blocks it from being checked
	// and will have it's timer changed when we exit combat so let's not do any extra work
	if (AggroCount == 1 && rest_timer.GetRemainingTime()) // the Client::rest_timer is never disabled, so don't need to check
		m_pp.RestTimer = std::max(1u, rest_timer.GetRemainingTime() / 1000); // I guess round up?

	// save the new timer if it's higher
	m_pp.RestTimer = std::max(m_pp.RestTimer, newtimer);

	// If we already had aggro before this method was called, the combat indicator should already be up for SoF clients,
	// so we don't need to send it again.
	//
	if(AggroCount > 1)
		return;

	if (ClientVersion() >= EQ::versions::ClientVersion::SoF) {
		auto outapp = new EQApplicationPacket(OP_RestState, 1);
		char *Buffer = (char *)outapp->pBuffer;
		VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0x01);
		QueuePacket(outapp);
		safe_delete(outapp);
	}

}

void Client::DecrementAggroCount()
{
	// This should be called when a client is removed from a mob's hate list (it dies or is memblurred).
	// It checks whether any other mob is aggro on the player, and if not, starts the rest timer.
	// For SoF, the opcode to start the rest state countdown timer in the UI is sent.

	// If we didn't have aggro before, this method should not have been called.
	if(!AggroCount)
		return;

	AggroCount--;

	if(!RuleB(Character, RestRegenEnabled))
		return;

	// Something else is still aggro on us, can't rest yet.
	if (AggroCount)
		return;

	rest_timer.Start(m_pp.RestTimer * 1000);

	if (ClientVersion() >= EQ::versions::ClientVersion::SoF) {
		auto outapp = new EQApplicationPacket(OP_RestState, 5);
		char *Buffer = (char *)outapp->pBuffer;
		VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0x00);
		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, m_pp.RestTimer);
		QueuePacket(outapp);
		safe_delete(outapp);
	}
}

// when we cast a beneficial spell we need to steal our targets current timer
// That's what we use this for
void Client::UpdateRestTimer(uint32 new_timer)
{
	// their timer was 0, so we don't do anything
	if (new_timer == 0)
		return;

	if (!RuleB(Character, RestRegenEnabled))
		return;

	// so if we're currently on aggro, we check our saved timer
	if (AggroCount) {
		if (m_pp.RestTimer < new_timer) // our timer needs to be updated, don't need to update client here
			m_pp.RestTimer = new_timer;
	} else { // if we're not aggro, we need to check if current timer needs updating
		if (rest_timer.GetRemainingTime() / 1000 < new_timer) {
			rest_timer.Start(new_timer * 1000);
			if (ClientVersion() >= EQ::versions::ClientVersion::SoF) {
				auto outapp = new EQApplicationPacket(OP_RestState, 5);
				char *Buffer = (char *)outapp->pBuffer;
				VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0x00);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, new_timer);
				QueuePacket(outapp);
				safe_delete(outapp);
			}
		}
	}
}

void Client::SendPVPStats()
{
	// This sends the data to the client to populate the PVP Stats Window.
	//
	// When the PVP Stats window is opened, no opcode is sent. Therefore this method should be called
	// from Client::CompleteConnect, and also when the player makes a PVP kill.
	//
	auto outapp = new EQApplicationPacket(OP_PVPStats, sizeof(PVPStats_Struct));
	PVPStats_Struct *pvps = (PVPStats_Struct *)outapp->pBuffer;

	pvps->Kills = m_pp.PVPKills;
	pvps->Deaths = m_pp.PVPDeaths;
	pvps->PVPPointsAvailable = m_pp.PVPCurrentPoints;
	pvps->TotalPVPPoints = m_pp.PVPCareerPoints;
	pvps->BestKillStreak = m_pp.PVPBestKillStreak;
	pvps->WorstDeathStreak = m_pp.PVPWorstDeathStreak;
	pvps->CurrentKillStreak = m_pp.PVPCurrentKillStreak;

	// TODO: Record and send other PVP Stats

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendCrystalCounts()
{
	auto outapp = new EQApplicationPacket(OP_CrystalCountUpdate, sizeof(CrystalCountUpdate_Struct));
	CrystalCountUpdate_Struct *ccus = (CrystalCountUpdate_Struct *)outapp->pBuffer;

	ccus->CurrentRadiantCrystals = GetRadiantCrystals();
	ccus->CurrentEbonCrystals = GetEbonCrystals();
	ccus->CareerRadiantCrystals = m_pp.careerRadCrystals;
	ccus->CareerEbonCrystals = m_pp.careerEbonCrystals;


	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendDisciplineTimers()
{

	auto outapp = new EQApplicationPacket(OP_DisciplineTimer, sizeof(DisciplineTimer_Struct));
	DisciplineTimer_Struct *dts = (DisciplineTimer_Struct *)outapp->pBuffer;

	for(unsigned int i = 0; i < MAX_DISCIPLINE_TIMERS; ++i)
	{
		uint32 RemainingTime = p_timers.GetRemainingTime(pTimerDisciplineReuseStart + i);

		if(RemainingTime > 0)
		{
			dts->TimerID = i;
			dts->Duration = RemainingTime;
			QueuePacket(outapp);
		}
	}

	safe_delete(outapp);
}

void Client::SendRespawnBinds()
{
	// This sends the data to the client to populate the Respawn from Death Window.
	//
	// This should be sent after OP_Death for SoF clients
	// Client will respond with a 4 byte packet that includes the number of the selection made
	//

		//If no options have been given, default to Bind + Rez
	if (respawn_options.empty())
	{
		BindStruct* b = &m_pp.binds[0];
		RespawnOption opt;
		opt.name = "Bind Location";
		opt.zone_id = b->zone_id;
		opt.instance_id = b->instance_id;
		opt.x = b->x;
		opt.y = b->y;
		opt.z = b->z;
		opt.heading = b->heading;
		respawn_options.push_front(opt);
	}
	//Rez is always added at the end
	RespawnOption rez;
	rez.name = "Resurrect";
	rez.zone_id = zone->GetZoneID();
	rez.instance_id = zone->GetInstanceID();
	rez.x = GetX();
	rez.y = GetY();
	rez.z = GetZ();
	rez.heading = GetHeading();
	respawn_options.push_back(rez);

	int num_options = respawn_options.size();
	uint32 PacketLength = 17 + (26 * num_options); //Header size + per-option invariant size

	std::list<RespawnOption>::iterator itr;
	RespawnOption* opt = nullptr;

	//Find string size for each option
	for (itr = respawn_options.begin(); itr != respawn_options.end(); ++itr)
	{
		opt = &(*itr);
		PacketLength += opt->name.size() + 1; //+1 for cstring
	}

	auto outapp = new EQApplicationPacket(OP_RespawnWindow, PacketLength);
	char* buffer = (char*)outapp->pBuffer;

	//Packet header
	VARSTRUCT_ENCODE_TYPE(uint32, buffer, initial_respawn_selection); //initial selection (from 0)
	VARSTRUCT_ENCODE_TYPE(uint32, buffer, RuleI(Character, RespawnFromHoverTimer) * 1000);
	VARSTRUCT_ENCODE_TYPE(uint32, buffer, 0); //unknown
	VARSTRUCT_ENCODE_TYPE(uint32, buffer, num_options); //number of options to display

	//Individual options
	int count = 0;
	for (itr = respawn_options.begin(); itr != respawn_options.end(); ++itr)
	{
		opt = &(*itr);
		VARSTRUCT_ENCODE_TYPE(uint32, buffer, count++); //option num (from 0)
		VARSTRUCT_ENCODE_TYPE(uint32, buffer, opt->zone_id);
		VARSTRUCT_ENCODE_TYPE(float, buffer, opt->x);
		VARSTRUCT_ENCODE_TYPE(float, buffer, opt->y);
		VARSTRUCT_ENCODE_TYPE(float, buffer, opt->z);
		VARSTRUCT_ENCODE_TYPE(float, buffer, opt->heading);
		VARSTRUCT_ENCODE_STRING(buffer, opt->name.c_str());
		VARSTRUCT_ENCODE_TYPE(uint8, buffer, (count == num_options)); //is this one Rez (the last option)?
	}

	QueuePacket(outapp);
	safe_delete(outapp);
	return;
}

void Client::HandleLDoNOpen(NPC *target)
{
	if(target)
	{
		if(target->GetClass() != Class::LDoNTreasure)
		{
			LogDebug("[{}] tried to open [{}] but [{}] was not a treasure chest",
				GetName(), target->GetName(), target->GetName());
			return;
		}

		if (target->GetSpecialAbility(SpecialAbility::OpenImmunity))
		{
			LogDebug("[{}] tried to open [{}] but it was immune", GetName(), target->GetName());
			return;
		}

		if(DistanceSquaredNoZ(m_Position, target->GetPosition()) > RuleI(Adventure, LDoNTrapDistanceUse))
		{
			LogDebug("[{}] tried to open [{}] but [{}] was out of range",
				GetName(), target->GetName(), target->GetName());
			Message(Chat::Red, "Treasure chest out of range.");
			return;
		}

		if(target->IsLDoNTrapped())
		{
			if(target->GetLDoNTrapSpellID() != 0)
			{
				MessageString(Chat::Red, LDON_ACCIDENT_SETOFF2);
				target->SpellFinished(target->GetLDoNTrapSpellID(), this, EQ::spells::CastingSlot::Item, 0, -1, spells[target->GetLDoNTrapSpellID()].resist_difficulty);
				target->SetLDoNTrapSpellID(0);
				target->SetLDoNTrapped(false);
				target->SetLDoNTrapDetected(false);
			}
			else
			{
				target->SetLDoNTrapSpellID(0);
				target->SetLDoNTrapped(false);
				target->SetLDoNTrapDetected(false);
			}
		}

		if(target->IsLDoNLocked())
		{
			MessageString(Chat::Skills, LDON_STILL_LOCKED, target->GetCleanName());
			return;
		}
		else
		{
			target->AddToHateList(this, 0, 500000, false, false, false);
			if(target->GetLDoNTrapType() != 0)
			{
				if(GetRaid())
				{
					GetRaid()->SplitExp(ExpSource::LDoNChest, target->GetLevel()*target->GetLevel()*2625/10, target);
				}
				else if(GetGroup())
				{
					GetGroup()->SplitExp(ExpSource::LDoNChest, target->GetLevel()*target->GetLevel()*2625/10, target);
				}
				else
				{
					AddEXP(ExpSource::LDoNChest, target->GetLevel()*target->GetLevel()*2625/10, GetLevelCon(target->GetLevel()));
				}
			}
			target->Death(this, 0, SPELL_UNKNOWN, EQ::skills::SkillHandtoHand);
		}
	}
}

void Client::HandleLDoNSenseTraps(NPC *target, uint16 skill, uint8 type)
{
	if(target && target->GetClass() == Class::LDoNTreasure)
	{
		if(target->IsLDoNTrapped())
		{
			if((target->GetLDoNTrapType() == LDoNTypeCursed || target->GetLDoNTrapType() == LDoNTypeMagical) && type != target->GetLDoNTrapType())
			{
				MessageString(Chat::Skills, LDON_CANT_DETERMINE_TRAP, target->GetCleanName());
				return;
			}

			if(target->IsLDoNTrapDetected())
			{
				MessageString(Chat::Skills, LDON_CERTAIN_TRAP, target->GetCleanName());
			}
			else
			{
				int check = LDoNChest_SkillCheck(target, skill);
				switch(check)
				{
				case -1:
				case 0:
					MessageString(Chat::Skills, LDON_DONT_KNOW_TRAPPED, target->GetCleanName());
					break;
				case 1:
					MessageString(Chat::Skills, LDON_CERTAIN_TRAP, target->GetCleanName());
					target->SetLDoNTrapDetected(true);
					break;
				default:
					break;
				}
			}
		}
		else
		{
			MessageString(Chat::Skills, LDON_CERTAIN_NOT_TRAP, target->GetCleanName());
		}
	}
}

void Client::HandleLDoNDisarm(NPC *target, uint16 skill, uint8 type)
{
	if(target)
	{
		if(target->GetClass() == Class::LDoNTreasure)
		{
			if(!target->IsLDoNTrapped())
			{
				MessageString(Chat::Skills, LDON_WAS_NOT_TRAPPED, target->GetCleanName());
				return;
			}

			if((target->GetLDoNTrapType() == LDoNTypeCursed || target->GetLDoNTrapType() == LDoNTypeMagical) && type != target->GetLDoNTrapType())
			{
				MessageString(Chat::Skills, LDON_HAVE_NOT_DISARMED, target->GetCleanName());
				return;
			}

			int check = 0;
			if(target->IsLDoNTrapDetected())
			{
				check = LDoNChest_SkillCheck(target, skill);
			}
			else
			{
				check = LDoNChest_SkillCheck(target, skill*33/100);
			}
			switch(check)
			{
			case 1:
				target->SetLDoNTrapDetected(false);
				target->SetLDoNTrapped(false);
				target->SetLDoNTrapSpellID(0);
				MessageString(Chat::Skills, LDON_HAVE_DISARMED, target->GetCleanName());
				break;
			case 0:
				MessageString(Chat::Skills, LDON_HAVE_NOT_DISARMED, target->GetCleanName());
				break;
			case -1:
				MessageString(Chat::Red, LDON_ACCIDENT_SETOFF2);
				target->SpellFinished(target->GetLDoNTrapSpellID(), this, EQ::spells::CastingSlot::Item, 0, -1, spells[target->GetLDoNTrapSpellID()].resist_difficulty);
				target->SetLDoNTrapSpellID(0);
				target->SetLDoNTrapped(false);
				target->SetLDoNTrapDetected(false);
				break;
			}
		}
	}
}

void Client::HandleLDoNPickLock(NPC *target, uint16 skill, uint8 type)
{
	if(target)
	{
		if(target->GetClass() == Class::LDoNTreasure)
		{
			if(target->IsLDoNTrapped())
			{
				MessageString(Chat::Red, LDON_ACCIDENT_SETOFF2);
				target->SpellFinished(target->GetLDoNTrapSpellID(), this, EQ::spells::CastingSlot::Item, 0, -1, spells[target->GetLDoNTrapSpellID()].resist_difficulty);
				target->SetLDoNTrapSpellID(0);
				target->SetLDoNTrapped(false);
				target->SetLDoNTrapDetected(false);
			}

			if(!target->IsLDoNLocked())
			{
				MessageString(Chat::Skills, LDON_WAS_NOT_LOCKED, target->GetCleanName());
				return;
			}

			if((target->GetLDoNTrapType() == LDoNTypeCursed || target->GetLDoNTrapType() == LDoNTypeMagical) && type != target->GetLDoNTrapType())
			{
				Message(Chat::Skills, "You cannot unlock %s with this skill.", target->GetCleanName());
				return;
			}

			int check = LDoNChest_SkillCheck(target, skill);

			switch(check)
			{
			case 0:
			case -1:
				MessageString(Chat::Skills, LDON_PICKLOCK_FAILURE, target->GetCleanName());
				break;
			case 1:
				target->SetLDoNLocked(false);
				MessageString(Chat::Skills, LDON_PICKLOCK_SUCCESS, target->GetCleanName());
				break;
			}
		}
	}
}

int	Client::LDoNChest_SkillCheck(NPC *target, int skill)
{
	if(!target)
		return -1;

	int	chest_difficulty = target->GetLDoNLockedSkill() == 0 ? (target->GetLevel() * 5) : target->GetLDoNLockedSkill();
	float base_difficulty = RuleR(Adventure, LDoNBaseTrapDifficulty);

	if(chest_difficulty == 0)
		chest_difficulty = 5;

	float chance = ((100.0f - base_difficulty) * ((float)skill / (float)chest_difficulty));

	if(chance > (100.0f - base_difficulty))
	{
		chance = 100.0f - base_difficulty;
	}

	float d100 = (float)zone->random.Real(0, 100);

	if(d100 <= chance)
		return 1;
	else
	{
		if(d100 > (chance + RuleR(Adventure, LDoNCriticalFailTrapThreshold)))
			return -1;
	}

	return 0;
}

void Client::SummonAndRezzAllCorpses()
{
	PendingRezzXP = -1;

	auto Pack = new ServerPacket(ServerOP_DepopAllPlayersCorpses, sizeof(ServerDepopAllPlayersCorpses_Struct));

	ServerDepopAllPlayersCorpses_Struct *sdapcs = (ServerDepopAllPlayersCorpses_Struct*)Pack->pBuffer;

	sdapcs->CharacterID = CharacterID();
	sdapcs->ZoneID = zone->GetZoneID();
	sdapcs->InstanceID = zone->GetInstanceID();

	worldserver.SendPacket(Pack);

	safe_delete(Pack);

	entity_list.RemoveAllCorpsesByCharID(CharacterID());

	int CorpseCount = database.SummonAllCharacterCorpses(CharacterID(), zone->GetZoneID(), zone->GetInstanceID(), GetPosition());
	if(CorpseCount <= 0)
	{
		Message(Chat::Yellow, "You have no corpses to summnon.");
		return;
	}

	int RezzExp = entity_list.RezzAllCorpsesByCharID(CharacterID());

	if(RezzExp > 0)
		SetEXP(ExpSource::Resurrection, GetEXP() + RezzExp, GetAAXP(), true);

	Message(Chat::Yellow, "All your corpses have been summoned to your feet and have received a 100% resurrection.");
}

void Client::SummonAllCorpses(const glm::vec4& position)
{
	auto summonLocation = position;
	if(IsOrigin(position) && position.w == 0.0f)
		summonLocation = GetPosition();

	auto Pack = new ServerPacket(ServerOP_DepopAllPlayersCorpses, sizeof(ServerDepopAllPlayersCorpses_Struct));

	ServerDepopAllPlayersCorpses_Struct *sdapcs = (ServerDepopAllPlayersCorpses_Struct*)Pack->pBuffer;

	sdapcs->CharacterID = CharacterID();
	sdapcs->ZoneID = zone->GetZoneID();
	sdapcs->InstanceID = zone->GetInstanceID();

	worldserver.SendPacket(Pack);

	safe_delete(Pack);

	entity_list.RemoveAllCorpsesByCharID(CharacterID());

	database.SummonAllCharacterCorpses(CharacterID(), zone->GetZoneID(), zone->GetInstanceID(), summonLocation);
}

void Client::DepopAllCorpses()
{
	auto Pack = new ServerPacket(ServerOP_DepopAllPlayersCorpses, sizeof(ServerDepopAllPlayersCorpses_Struct));

	ServerDepopAllPlayersCorpses_Struct *sdapcs = (ServerDepopAllPlayersCorpses_Struct*)Pack->pBuffer;

	sdapcs->CharacterID = CharacterID();
	sdapcs->ZoneID = zone->GetZoneID();
	sdapcs->InstanceID = zone->GetInstanceID();

	worldserver.SendPacket(Pack);

	safe_delete(Pack);

	entity_list.RemoveAllCorpsesByCharID(CharacterID());
}

void Client::DepopPlayerCorpse(uint32 dbid)
{
	auto Pack = new ServerPacket(ServerOP_DepopPlayerCorpse, sizeof(ServerDepopPlayerCorpse_Struct));

	ServerDepopPlayerCorpse_Struct *sdpcs = (ServerDepopPlayerCorpse_Struct*)Pack->pBuffer;

	sdpcs->DBID = dbid;
	sdpcs->ZoneID = zone->GetZoneID();
	sdpcs->InstanceID = zone->GetInstanceID();

	worldserver.SendPacket(Pack);

	safe_delete(Pack);

	entity_list.RemoveCorpseByDBID(dbid);
}

void Client::BuryPlayerCorpses()
{
	database.BuryAllCharacterCorpses(CharacterID());
}

void Client::NotifyNewTitlesAvailable()
{
	auto outapp = new EQApplicationPacket(OP_NewTitlesAvailable, 0);

	QueuePacket(outapp);

	safe_delete(outapp);

}

void Client::SetStartZone(uint32 zoneid, float x, float y, float z, float heading)
{
	// setting city to zero allows the player to use /setstartcity to set the city themselves
	if(zoneid == 0) {
		m_pp.binds[4].zone_id = 0;
		Message(Chat::Yellow,"Your starting city has been reset. Use /setstartcity to choose a new one");
		return;
	}

	// check to make sure the zone is valid
	const char *target_zone_name = ZoneName(zoneid);
	if(target_zone_name == nullptr)
		return;

	m_pp.binds[4].zone_id = zoneid;
	if(zone->GetInstanceID() != 0 && zone->IsInstancePersistent()) {
		m_pp.binds[4].instance_id = zone->GetInstanceID();
	}

	if (x == 0 && y == 0 && z == 0) {
		auto zd = GetZone(m_pp.binds[4].zone_id);
		if (zd) {
			m_pp.binds[4].x = zd->safe_x;
			m_pp.binds[4].y = zd->safe_y;
			m_pp.binds[4].z = zd->safe_z;
			m_pp.binds[4].heading = zd->safe_heading;
		}
	}
	else {
		m_pp.binds[4].x = x;
		m_pp.binds[4].y = y;
		m_pp.binds[4].z = z;
		m_pp.binds[4].heading = heading;
	}
}

uint32 Client::GetStartZone()
{
	return m_pp.binds[4].zone_id;
}

void Client::ShowSkillsWindow()
{
	std::string popup_text;
	std::map<EQ::skills::SkillType, std::string> skills_map = EQ::skills::GetSkillTypeMap();

	if (ClientVersion() < EQ::versions::ClientVersion::RoF2) {
		skills_map[EQ::skills::Skill1HPiercing] = "Piercing";
	}

	// Table Start
	popup_text += "<table>";

	for (const auto& skill : skills_map) {
		auto skill_id = skill.first;
		auto skill_name = skill.second;
		auto can_have_skill = CanHaveSkill(skill_id);
		auto current_skill = GetSkill(skill_id);
		auto max_skill = MaxSkill(skill_id);
		auto skill_maxed = current_skill >= max_skill;
		if (
			skill_id == EQ::skills::Skill2HPiercing &&
			ClientVersion() < EQ::versions::ClientVersion::RoF2
		) {
			continue;
		}

		if (
			!can_have_skill ||
			!current_skill ||
			!max_skill
		) {
			continue;
		}

		// Row Start
		popup_text += "<tr>";

		// Skill Name
		popup_text += fmt::format(
			"<td>{}</td>",
			skill_name
		);

		// Current Skill Level out of Max Skill Level or a Check Mark for Maxed
		popup_text += fmt::format(
			"<td>{}</td>",
			(
				skill_maxed ?
				"<c \"#00FF00\">Max</c>" :
				fmt::format(
					"{}/{}",
					current_skill,
					max_skill
				)
			)
		);

		// Row End
		popup_text += "</tr>";
	}

	// Table End
	popup_text += "</table>";

	SendPopupToClient(
		"Skills",
		popup_text.c_str()
	);
}

void Client::Signal(int signal_id)
{
	if (parse->PlayerHasQuestSub(EVENT_SIGNAL)) {
		parse->EventPlayer(EVENT_SIGNAL, this, std::to_string(signal_id), 0);
	}
}

void Client::SendPayload(int payload_id, std::string payload_value)
{
	if (parse->PlayerHasQuestSub(EVENT_PAYLOAD)) {
		const auto& export_string = fmt::format("{} {}", payload_id, payload_value);

		parse->EventPlayer(EVENT_PAYLOAD, this, export_string, 0);
	}
}

void Client::SendRewards()
{
	std::vector<ClientReward> rewards;
	std::string query = StringFormat("SELECT reward_id, amount "
									"FROM account_rewards "
									"WHERE account_id = %i "
									"ORDER BY reward_id", AccountID());
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		ClientReward cr;
		cr.id = Strings::ToInt(row[0]);
		cr.amount = Strings::ToInt(row[1]);
		rewards.push_back(cr);
	}

	if(rewards.empty())
		return;

	auto vetapp = new EQApplicationPacket(OP_VetRewardsAvaliable, (sizeof(InternalVeteranReward) * rewards.size()));
	uchar *data = vetapp->pBuffer;
	for(int i = 0; i < rewards.size(); ++i) {
		InternalVeteranReward *ivr = (InternalVeteranReward*)data;
		ivr->claim_id = rewards[i].id;
		ivr->number_available = rewards[i].amount;
		auto iter = zone->VeteranRewards.begin();
		for (;iter != zone->VeteranRewards.end(); ++iter)
			if((*iter).claim_id == rewards[i].id)
				break;

		if(iter != zone->VeteranRewards.end()) {
			InternalVeteranReward ivro = (*iter);
			ivr->claim_count = ivro.claim_count;
			for(int x = 0; x < ivro.claim_count; ++x) {
				ivr->items[x].item_id = ivro.items[x].item_id;
				ivr->items[x].charges = ivro.items[x].charges;
				strcpy(ivr->items[x].item_name, ivro.items[x].item_name);
			}
		}

		data += sizeof(InternalVeteranReward);
	}

	FastQueuePacket(&vetapp);
}

bool Client::TryReward(uint32 claim_id)
{
	// Make sure we have an open spot
	// Make sure we have it in our acct and count > 0
	// Make sure the entry was found
	// If we meet all the criteria:
	// Decrement our count by 1 if it > 1 delete if it == 1
	// Create our item in bag if necessary at the free inv slot
	// save
	uint32 free_slot = 0xFFFFFFFF;

	for (int i = EQ::invslot::GENERAL_BEGIN; i <= EQ::invslot::GENERAL_END; ++i) {
		EQ::ItemInstance *item = GetInv().GetItem(i);
		if (!item) {
			free_slot = i;
			break;
		}
	}

	if (free_slot == 0xFFFFFFFF)
		return false;

	std::string query = StringFormat("SELECT amount FROM account_rewards "
					 "WHERE account_id = %i AND reward_id = %i",
					 AccountID(), claim_id);
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return false;

	if (results.RowCount() == 0)
		return false;

	auto row = results.begin();

	uint32 amt = Strings::ToInt(row[0]);
	if (amt == 0)
		return false;

	auto iter = std::find_if(zone->VeteranRewards.begin(), zone->VeteranRewards.end(),
			[claim_id](const InternalVeteranReward &a) { return a.claim_id == claim_id; });

	if (iter == zone->VeteranRewards.end())
		return false;

	if (amt == 1) {
		query = StringFormat("DELETE FROM account_rewards "
					 "WHERE account_id = %i AND reward_id = %i",
					 AccountID(), claim_id);
		auto results = database.QueryDatabase(query);
	} else {
		query = StringFormat("UPDATE account_rewards SET amount = (amount-1) "
					 "WHERE account_id = %i AND reward_id = %i",
					 AccountID(), claim_id);
		auto results = database.QueryDatabase(query);
	}

	auto &ivr = (*iter);
	EQ::ItemInstance *claim = database.CreateItem(ivr.items[0].item_id, ivr.items[0].charges);
	if (!claim) {
		Save();
		return true;
	}

	bool lore_conflict = CheckLoreConflict(claim->GetItem());

	for (int y = 1; y < 8; y++)
		if (ivr.items[y].item_id && claim->GetItem()->ItemClass == 1) {
			EQ::ItemInstance *item_temp = database.CreateItem(ivr.items[y].item_id, ivr.items[y].charges);
			if (item_temp) {
				if (CheckLoreConflict(item_temp->GetItem())) {
					lore_conflict = true;
					DuplicateLoreMessage(ivr.items[y].item_id);
				}
				claim->PutItem(y - 1, *item_temp);
				safe_delete(item_temp);
			}
		}

	if (lore_conflict) {
		safe_delete(claim);
		return true;
	}

	PutItemInInventory(free_slot, *claim);
	SendItemPacket(free_slot, claim, ItemPacketTrade);
	safe_delete(claim);

	Save();
	return true;
}

uint32 Client::GetLDoNPointsTheme(uint32 t)
{
	switch(t)
	{
	case LDoNTheme::GUK:
		return m_pp.ldon_points_guk;
	case LDoNTheme::MIR:
		return m_pp.ldon_points_mir;
	case LDoNTheme::MMC:
		return m_pp.ldon_points_mmc;
	case LDoNTheme::RUJ:
		return m_pp.ldon_points_ruj;
	case LDoNTheme::TAK:
		return m_pp.ldon_points_tak;
	default:
		return 0;
	}
}

uint32 Client::GetLDoNWinsTheme(uint32 t)
{
	switch(t)
	{
	case LDoNTheme::GUK:
		return m_pp.ldon_wins_guk;
	case LDoNTheme::MIR:
		return m_pp.ldon_wins_mir;
	case LDoNTheme::MMC:
		return m_pp.ldon_wins_mmc;
	case LDoNTheme::RUJ:
		return m_pp.ldon_wins_ruj;
	case LDoNTheme::TAK:
		return m_pp.ldon_wins_tak;
	default:
		return 0;
	}
}

uint32 Client::GetLDoNLossesTheme(uint32 t)
{
	switch(t)
	{
	case LDoNTheme::GUK:
		return m_pp.ldon_losses_guk;
	case LDoNTheme::MIR:
		return m_pp.ldon_losses_mir;
	case LDoNTheme::MMC:
		return m_pp.ldon_losses_mmc;
	case LDoNTheme::RUJ:
		return m_pp.ldon_losses_ruj;
	case LDoNTheme::TAK:
		return m_pp.ldon_losses_tak;
	default:
		return 0;
	}
}

void Client::UpdateLDoNWinLoss(uint32 theme_id, bool win, bool remove) {
	switch (theme_id) {
		case LDoNTheme::GUK:
			if (win) {
				m_pp.ldon_wins_guk += (remove ? -1 : 1);
			} else {
				m_pp.ldon_losses_guk += (remove ? -1 : 1);
			}
			break;
		case LDoNTheme::MIR:
			if (win) {
				m_pp.ldon_wins_mir += (remove ? -1 : 1);
			} else {
				m_pp.ldon_losses_mir += (remove ? -1 : 1);
			}
			break;
		case LDoNTheme::MMC:
			if (win) {
				m_pp.ldon_wins_mmc += (remove ? -1 : 1);
			} else {
				m_pp.ldon_losses_mmc += (remove ? -1 : 1);
			}
			break;
		case LDoNTheme::RUJ:
			if (win) {
				m_pp.ldon_wins_ruj += (remove ? -1 : 1);
			} else {
				m_pp.ldon_losses_ruj += (remove ? -1 : 1);
			}
			break;
		case LDoNTheme::TAK:
			if (win) {
				m_pp.ldon_wins_tak += (remove ? -1 : 1);
			} else {
				m_pp.ldon_losses_tak += (remove ? -1 : 1);
			}
			break;
		default:
			return;
	}
	database.UpdateAdventureStatsEntry(CharacterID(), theme_id, win, remove);
}


void Client::SuspendMinion(int value)
{
	/*
		SPA 151 Allows an extra pet to be saved and resummoned later.
		Casting with a pet but without a suspended pet will suspend the pet
		Casting without a pet and with a suspended pet will unsuspend the pet
		effect value 0 = save pet with no buffs or equipment
		effect value 1 = save pet with buffs and equipment
		effect value 2 = unknown
		Note: SPA 308 allows for suspended pets to be resummoned after zoning.
	*/

	NPC *CurrentPet = GetPet()->CastToNPC();

	if(!CurrentPet)
	{
		if(m_suspendedminion.SpellID > 0)
		{
			if (m_suspendedminion.SpellID >= SPDAT_RECORDS) {
				Message(Chat::Red, "Invalid suspended minion spell id (%u).", m_suspendedminion.SpellID);
				memset(&m_suspendedminion, 0, sizeof(PetInfo));
				return;
			}

			MakePoweredPet(m_suspendedminion.SpellID, spells[m_suspendedminion.SpellID].teleport_zone,
				m_suspendedminion.petpower, m_suspendedminion.Name, m_suspendedminion.size);

			CurrentPet = GetPet()->CastToNPC();

			if(!CurrentPet)
			{
				Message(Chat::Red, "Failed to recall suspended minion.");
				return;
			}

			if(value >= 1)
			{
				CurrentPet->SetPetState(m_suspendedminion.Buffs, m_suspendedminion.Items);

				CurrentPet->SendPetBuffsToClient();
			}
			CurrentPet->CalcBonuses();

			CurrentPet->SetHP(m_suspendedminion.HP);

			CurrentPet->SetMana(m_suspendedminion.Mana);

			CurrentPet->SetTaunting(m_suspendedminion.taunting);

			MessageString(Chat::Magenta, SUSPEND_MINION_UNSUSPEND, CurrentPet->GetCleanName());

			memset(&m_suspendedminion, 0, sizeof(struct PetInfo));
			// TODO: These pet command states need to be synced ...
			// Will just fix them for now
			if (m_ClientVersionBit & EQ::versions::maskUFAndLater) {
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
		}
		else
			return;

	}
	else
	{
		uint16 SpellID = CurrentPet->GetPetSpellID();

		if(SpellID)
		{
			if(m_suspendedminion.SpellID > 0)
			{
				MessageString(Chat::Red,ONLY_ONE_PET);

				return;
			}
			else if(CurrentPet->IsEngaged())
			{
				MessageString(Chat::Red,SUSPEND_MINION_FIGHTING);

				return;
			}
			else if(entity_list.Fighting(CurrentPet))
			{
				MessageString(Chat::Blue,SUSPEND_MINION_HAS_AGGRO);
			}
			else
			{
				m_suspendedminion.SpellID = SpellID;

				m_suspendedminion.HP = CurrentPet->GetHP();

				m_suspendedminion.Mana = CurrentPet->GetMana();
				m_suspendedminion.petpower = CurrentPet->GetPetPower();
				m_suspendedminion.size = CurrentPet->GetSize();

				if(value >= 1)
					CurrentPet->GetPetState(m_suspendedminion.Buffs, m_suspendedminion.Items, m_suspendedminion.Name);
				else
					strn0cpy(m_suspendedminion.Name, CurrentPet->GetName(), 64); // Name stays even at rank 1

				MessageString(Chat::Magenta, SUSPEND_MINION_SUSPEND, CurrentPet->GetCleanName());

				CurrentPet->Depop(false);

				SetPetID(0);
			}
		}
		else
		{
			MessageString(Chat::Red, ONLY_SUMMONED_PETS);

			return;
		}
	}
}

void Client::AddPVPPoints(uint32 Points)
{
	m_pp.PVPCurrentPoints += Points;
	m_pp.PVPCareerPoints += Points;

	Save();

	SendPVPStats();
}

void Client::AddEbonCrystals(uint32 amount, bool is_reclaim) {
	m_pp.currentEbonCrystals += amount;
	m_pp.careerEbonCrystals += amount;

	SaveCurrency();
	SendCrystalCounts();

	MessageString(
		Chat::Yellow,
		YOU_RECEIVE,
		fmt::format(
			"{} {}",
			amount,
			database.CreateItemLink(RuleI(Zone, EbonCrystalItemID))
		).c_str()
	);

	if (parse->PlayerHasQuestSub(EVENT_CRYSTAL_GAIN)) {
		const std::string &export_string = fmt::format(
			"{} 0 {}",
			amount,
			is_reclaim ? 1 : 0
		);
		parse->EventPlayer(EVENT_CRYSTAL_GAIN, this, export_string, 0);
	}
}

void Client::AddRadiantCrystals(uint32 amount, bool is_reclaim) {
	m_pp.currentRadCrystals += amount;
	m_pp.careerRadCrystals += amount;

	SaveCurrency();
	SendCrystalCounts();

	MessageString(
		Chat::Yellow,
		YOU_RECEIVE,
		fmt::format(
			"{} {}",
			amount,
			database.CreateItemLink(RuleI(Zone, RadiantCrystalItemID))
		).c_str()
	);

	if (parse->PlayerHasQuestSub(EVENT_CRYSTAL_GAIN)) {
		const std::string &export_string = fmt::format(
			"0 {} {}",
			amount,
			is_reclaim ? 1 : 0
		);
		parse->EventPlayer(EVENT_CRYSTAL_GAIN, this, export_string, 0);
	}
}

void Client::RemoveEbonCrystals(uint32 amount, bool is_reclaim) {
	m_pp.currentEbonCrystals -= amount;

	SaveCurrency();
	SendCrystalCounts();

	if (parse->PlayerHasQuestSub(EVENT_CRYSTAL_LOSS)) {
		const std::string &export_string = fmt::format(
			"{} 0 {}",
			amount,
			is_reclaim ? 1 : 0
		);
		parse->EventPlayer(EVENT_CRYSTAL_LOSS, this, export_string, 0);
	}
}

void Client::RemoveRadiantCrystals(uint32 amount, bool is_reclaim) {
	m_pp.currentRadCrystals -= amount;

	SaveCurrency();
	SendCrystalCounts();

	if (parse->PlayerHasQuestSub(EVENT_CRYSTAL_LOSS)) {
		const std::string &export_string = fmt::format(
			"0 {} {}",
			amount,
			is_reclaim ? 1 : 0
		);
		parse->EventPlayer(EVENT_CRYSTAL_LOSS, this, export_string, 0);
	}
}

void Client::SetEbonCrystals(uint32 value) {
	m_pp.currentEbonCrystals = value;
	SaveCurrency();
	SendCrystalCounts();
}

void Client::SetRadiantCrystals(uint32 value) {
	m_pp.currentRadCrystals = value;
	SaveCurrency();
	SendCrystalCounts();
}

// Processes a client request to inspect a SoF+ client's equipment.
void Client::ProcessInspectRequest(Client *requestee, Client *requester)
{
	if (requestee && requester) {
		auto outapp = new EQApplicationPacket(OP_InspectAnswer, sizeof(InspectResponse_Struct));
		auto insr   = (InspectResponse_Struct *) outapp->pBuffer;

		insr->TargetID = requester->GetID();
		insr->playerid = requestee->GetID();

		const EQ::ItemData     *item                      = nullptr;
		const EQ::ItemInstance *inst                      = nullptr;

		for (int16 L = EQ::invslot::EQUIPMENT_BEGIN; L <= EQ::invslot::EQUIPMENT_END; L++) {
			inst = requestee->GetInv().GetItem(L);

			if (inst) {
				item = inst->GetItem();
				if (item) {
					strcpy(insr->itemnames[L], item->Name);

					const EQ::ItemData *augment_item = nullptr;
					const auto         augment       = inst->GetOrnamentationAugment();

					if (augment) {
						augment_item = augment->GetItem();
					}

					if (augment_item) {
						insr->itemicons[L] = augment_item->Icon;
					} else if (inst->GetOrnamentationIcon()) {
						insr->itemicons[L] = inst->GetOrnamentationIcon();
					} else {
						insr->itemicons[L] = item->Icon;
					}
				} else {
					insr->itemnames[L][0] = '\0';
					insr->itemicons[L]    = 0xFFFFFFFF;
				}
			} else {
				insr->itemnames[L][0] = '\0';
				insr->itemicons[L]    = 0xFFFFFFFF;
			}
		}

		strcpy(insr->text, requestee->GetInspectMessage().text);

		// There could be an OP for this..or not... (Ti clients are not processed here..this message is generated client-side)
		if (requestee->IsClient() && requestee != requester) {
			requestee->Message(
				Chat::White,
				fmt::format(
					"{} is looking at your equipment...",
					requester->GetName()
				).c_str()
			);
		}

		requester->QueuePacket(outapp); // Send answer to requester
		safe_delete(outapp);
	}
}

void Client::GuildBankAck()
{
	auto outapp = new EQApplicationPacket(OP_GuildBank, sizeof(GuildBankAck_Struct));

	GuildBankAck_Struct *gbas = (GuildBankAck_Struct*) outapp->pBuffer;

	gbas->Action = GuildBankAcknowledge;

	FastQueuePacket(&outapp);
}

void Client::GuildBankDepositAck(bool Fail, int8 action)
{

	auto outapp = new EQApplicationPacket(OP_GuildBank, sizeof(GuildBankDepositAck_Struct));

	GuildBankDepositAck_Struct *gbdas = (GuildBankDepositAck_Struct*) outapp->pBuffer;

	gbdas->Action = action;

	gbdas->Fail = Fail ? 1 : 0;

	FastQueuePacket(&outapp);
}

void Client::ClearGuildBank()
{
	auto outapp = new EQApplicationPacket(OP_GuildBank, sizeof(GuildBankClear_Struct));

	GuildBankClear_Struct *gbcs = (GuildBankClear_Struct*) outapp->pBuffer;

	gbcs->Action = GuildBankBulkItems;
	gbcs->DepositAreaCount = 0;
	gbcs->MainAreaCount = 0;

	FastQueuePacket(&outapp);
}

void Client::SendGroupCreatePacket()
{
	// For SoD and later clients, this is sent the Group Leader upon initial creation of the group
	//
	auto outapp = new EQApplicationPacket(OP_GroupUpdateB, 32 + strlen(GetName()));

	char *Buffer = (char *)outapp->pBuffer;
	// Header
	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);	// group ID probably
	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 1);	// count of members in packet
	VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// Null Leader name, shouldn't be null besides this case

	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);	// Member 0, index
	VARSTRUCT_ENCODE_STRING(Buffer, GetName());	// group member name
	VARSTRUCT_ENCODE_TYPE(uint16, Buffer, 0);	// merc flag
	VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// owner name (if merc)
	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, GetLevel());	// level
	VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// group tank flag
	VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// group assist flag
	VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// group puller flag
	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);	// offline flag
	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);	// timestamp

	FastQueuePacket(&outapp);
}

void Client::SendGroupLeaderChangePacket(const char *LeaderName)
{
	// For SoD and later, send name of Group Leader to this client

	auto outapp = new EQApplicationPacket(OP_GroupLeaderChange, sizeof(GroupLeaderChange_Struct));

	GroupLeaderChange_Struct *glcs = (GroupLeaderChange_Struct*)outapp->pBuffer;

	strn0cpy(glcs->LeaderName, LeaderName, sizeof(glcs->LeaderName));

	FastQueuePacket(&outapp);
}

void Client::SendGroupJoinAcknowledge()
{
	// For SoD and later, This produces the 'You have joined the group' message.
	auto outapp = new EQApplicationPacket(OP_GroupAcknowledge, 4);
	FastQueuePacket(&outapp);
}

void Client::SendAdventureError(const char *error)
{
	size_t error_size = strlen(error);
	auto outapp = new EQApplicationPacket(OP_AdventureInfo, (error_size + 2));
	strn0cpy((char*)outapp->pBuffer, error, error_size);
	FastQueuePacket(&outapp);
}

void Client::SendAdventureDetails()
{
	if(adv_data)
	{
		ServerSendAdventureData_Struct *ad = (ServerSendAdventureData_Struct*)adv_data;
		auto outapp = new EQApplicationPacket(OP_AdventureData, sizeof(AdventureRequestResponse_Struct));
		AdventureRequestResponse_Struct *arr = (AdventureRequestResponse_Struct*)outapp->pBuffer;
		arr->unknown000 = 0xBFC40100;
		arr->unknown2080 = 0x0A;
		arr->risk = ad->risk;
		strcpy(arr->text, ad->text);

		if(ad->time_to_enter != 0)
		{
			arr->timetoenter = ad->time_to_enter;
		}
		else
		{
			arr->timeleft = ad->time_left;
		}

		if(ad->zone_in_id == zone->GetZoneID())
		{
			arr->y = ad->x;
			arr->x = ad->y;
			arr->showcompass = 1;
		}
		FastQueuePacket(&outapp);

		SendAdventureCount(ad->count, ad->total);
	}
	else
	{
		ServerSendAdventureData_Struct *ad = (ServerSendAdventureData_Struct*)adv_data;
		auto outapp = new EQApplicationPacket(OP_AdventureData, sizeof(AdventureRequestResponse_Struct));
		FastQueuePacket(&outapp);
	}
}

void Client::SendAdventureCount(uint32 count, uint32 total)
{
	auto outapp = new EQApplicationPacket(OP_AdventureUpdate, sizeof(AdventureCountUpdate_Struct));
	AdventureCountUpdate_Struct *acu = (AdventureCountUpdate_Struct*)outapp->pBuffer;
	acu->current = count;
	acu->total = total;
	FastQueuePacket(&outapp);
}

void Client::NewAdventure(int id, int theme, const char *text, int member_count, const char *members)
{
	size_t text_size = strlen(text);
	auto outapp = new EQApplicationPacket(OP_AdventureDetails, text_size + 2);
	strn0cpy((char*)outapp->pBuffer, text, text_size);
	FastQueuePacket(&outapp);

	adv_requested_id = id;
	adv_requested_theme = theme;
	safe_delete_array(adv_requested_data);
	adv_requested_member_count = member_count;
	adv_requested_data = new char[64 * member_count];
	memcpy(adv_requested_data, members, (64 * member_count));
}

void Client::ClearPendingAdventureData()
{
	adv_requested_id = 0;
	adv_requested_theme = LDoNTheme::Unused;
	safe_delete_array(adv_requested_data);
	adv_requested_member_count = 0;
}

bool Client::IsOnAdventure()
{
	if(adv_data)
	{
		ServerSendAdventureData_Struct *ad = (ServerSendAdventureData_Struct*)adv_data;
		if(ad->zone_in_id == 0)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	return false;
}

void Client::LeaveAdventure()
{
	if(!GetPendingAdventureLeave())
	{
		PendingAdventureLeave();
		auto pack = new ServerPacket(ServerOP_AdventureLeave, 64);
		strcpy((char*)pack->pBuffer, GetName());
		worldserver.SendPacket(pack);
		delete pack;
	}
}

void Client::ClearCurrentAdventure()
{
	if(adv_data)
	{
		ServerSendAdventureData_Struct* ds = (ServerSendAdventureData_Struct*)adv_data;
		if(ds->finished_adventures > 0)
		{
			ds->instance_id = 0;
			ds->risk = 0;
			memset(ds->text, 0, 512);
			ds->time_left = 0;
			ds->time_to_enter = 0;
			ds->x = 0;
			ds->y = 0;
			ds->zone_in_id = 0;
			ds->zone_in_object = 0;
		}
		else
		{
			safe_delete(adv_data);
		}

		SendAdventureError("You are not currently assigned to an adventure.");
	}
}

void Client::AdventureFinish(bool win, int theme, int points)
{
	UpdateLDoNPoints(theme, points);
	auto outapp = new EQApplicationPacket(OP_AdventureFinish, sizeof(AdventureFinish_Struct));
	AdventureFinish_Struct *af = (AdventureFinish_Struct*)outapp->pBuffer;
	af->win_lose = win ? 1 : 0;
	af->points = points;
	FastQueuePacket(&outapp);
}

void Client::CheckLDoNHail(NPC* n)
{
	if (!zone->adv_data || !n || n->GetOwnerID()) {
		return;
	}

	auto* ds = (ServerZoneAdventureDataReply_Struct*) zone->adv_data;
	if (ds->type != Adventure_Rescue || ds->data_id != n->GetNPCTypeID()) {
		return;
	}

	if (entity_list.CheckNPCsClose(n)) {
		n->Say(
			"You're here to save me? I couldn't possibly risk leaving yet. There are "
			"far too many of those horrid things out there waiting to recapture me! Please get "
			"rid of some more of those vermin and then we can try to leave."
		);
		return;
	}

	auto pet = GetPet();
	if (pet) {
		if (pet->GetPetType() == PetType::Charmed) {
			pet->BuffFadeByEffect(SpellEffect::Charm);
		} else if (pet->GetPetType() == PetType::Follow) {
			pet->SetOwnerID(0);
		} else {
			pet->Depop();
		}
	}

	SetPet(n);
	n->SetOwnerID(GetID());
	n->Say(
		"Wonderful! Someone to set me free! I feared for my life for so long, "
		"never knowing when they might choose to end my life. Now that you're here though "
		"I can rest easy. Please help me find my way out of here as soon as you can "
		"I'll stay close behind you!"
	);
}

void Client::CheckEmoteHail(NPC* n, const char* message)
{
	if (!Strings::BeginsWith(Strings::ToLower(message), "hail")) {
		return;
	}

	if (!n || n->GetOwnerID()) {
		return;
	}

	const uint32 emote_id = n->GetEmoteID();
	if (emote_id) {
		n->DoNPCEmote(EQ::constants::EmoteEventTypes::Hailed, emote_id, this);
	}
}

void Client::MarkSingleCompassLoc(float in_x, float in_y, float in_z, uint8 count)
{
	m_has_quest_compass = (count != 0);
	m_quest_compass.x = in_x;
	m_quest_compass.y = in_y;
	m_quest_compass.z = in_z;

	SendDzCompassUpdate();
}

void Client::SendZonePoints()
{
	int                             count = 0;
	LinkedListIterator<ZonePoint *> iterator(zone->zone_point_list);
	iterator.Reset();
	while (iterator.MoreElements()) {
		ZonePoint *data = iterator.GetData();

		if (ClientVersionBit() & data->client_version_mask) {
			count++;
		}

		iterator.Advance();
	}

	uint32 zpsize = sizeof(ZonePoints) + ((count + 1) * sizeof(ZonePoint_Entry));
	auto outapp = new EQApplicationPacket(OP_SendZonepoints, zpsize);
	ZonePoints* zp = (ZonePoints*)outapp->pBuffer;
	zp->count = count;

	int i = 0;
	iterator.Reset();
	while(iterator.MoreElements())
	{
		ZonePoint* data = iterator.GetData();

		LogZonePoints(
			"Sending zone point to client [{}] mask [{}] x [{}] y [{}] z [{}] number [{}]",
			GetCleanName(),
			ClientVersionBit() & data->client_version_mask ? "true" : "false",
			data->x,
			data->y,
			data->z,
			data->number
		);

		if(ClientVersionBit() & data->client_version_mask)
		{
			zp->zpe[i].iterator = data->number;
			zp->zpe[i].x = data->target_x;
			zp->zpe[i].y = data->target_y;
			zp->zpe[i].z = data->target_z;
			zp->zpe[i].heading = data->target_heading;
			zp->zpe[i].zoneid = data->target_zone_id;

			// if the target zone is the same as the current zone, use the instance of the current zone
			// if we don't use the same instance_id that the client was sent, the client will forcefully
			// issue a zone change request when they should be simply moving to a different point in the same zone
			// because the client will think the zone point target is different from the current instance
			auto target_instance = data->target_zone_instance;
			if (data->target_zone_id == zone->GetZoneID() && data->target_zone_instance == 0) {
				target_instance = zone->GetInstanceID();
			}

			zp->zpe[i].zoneinstance = target_instance;
			i++;
		}
		iterator.Advance();
	}

	FastQueuePacket(&outapp);
}

void Client::SendTargetCommand(uint32 EntityID)
{
	auto outapp = new EQApplicationPacket(OP_TargetCommand, sizeof(ClientTarget_Struct));
	ClientTarget_Struct *cts = (ClientTarget_Struct*)outapp->pBuffer;
	cts->new_target = EntityID;
	FastQueuePacket(&outapp);
}

void Client::LocateCorpse()
{
	Corpse *ClosestCorpse = nullptr;
	if(!GetTarget())
		ClosestCorpse = entity_list.GetClosestCorpse(this, nullptr);
	else if(GetTarget()->IsCorpse())
		ClosestCorpse = entity_list.GetClosestCorpse(this, GetTarget()->CastToCorpse()->GetOwnerName());
	else
		ClosestCorpse = entity_list.GetClosestCorpse(this, GetTarget()->GetCleanName());

	if(ClosestCorpse)
	{
		MessageString(Chat::Spells, SENSE_CORPSE_DIRECTION);
		SetHeading(CalculateHeadingToTarget(ClosestCorpse->GetX(), ClosestCorpse->GetY()));
		SetTarget(ClosestCorpse);
		SendTargetCommand(ClosestCorpse->GetID());
		SentPositionPacket(0.0f, 0.0f, 0.0f, 0.0f, 0, true);
	}
	else if(!GetTarget())
		MessageString(Chat::Red, SENSE_CORPSE_NONE);
	else
		MessageString(Chat::Red, SENSE_CORPSE_NOT_NAME);
}

void Client::NPCSpawn(NPC *target_npc, const char *identifier, uint32 extra)
{
	if (!target_npc || !identifier) {
		return;
	}

	std::string spawn_type = Strings::ToLower(identifier);
	bool is_add = spawn_type.find("add") != std::string::npos;
	bool is_create = spawn_type.find("create") != std::string::npos;
	bool is_delete = spawn_type.find("delete") != std::string::npos;
	bool is_remove = spawn_type.find("remove") != std::string::npos;
	bool is_update = spawn_type.find("update") != std::string::npos;
	bool is_clone = spawn_type.find("clone") != std::string::npos;
	if (is_add || is_create) {
		// extra sets the Respawn Timer for add/create
		content_db.NPCSpawnDB(
			is_add ? NPCSpawnTypes::AddNewSpawngroup : NPCSpawnTypes::CreateNewSpawn,
			zone->GetShortName(),
			zone->GetInstanceVersion(),
			this,
			target_npc->CastToNPC(),
			extra
		);
	} else if (is_delete || is_remove || is_update) {
		uint8 spawn_update_type = (
			is_delete ?
			NPCSpawnTypes::DeleteSpawn :
			(
				is_remove ?
				NPCSpawnTypes::RemoveSpawn :
				NPCSpawnTypes::UpdateAppearance
			)
		);
		content_db.NPCSpawnDB(
			spawn_update_type,
			zone->GetShortName(),
			zone->GetInstanceVersion(),
			this,
			target_npc->CastToNPC(),
			extra
		);
	} else if (is_clone) {
		content_db.NPCSpawnDB(
			NPCSpawnTypes::AddSpawnFromSpawngroup,
			zone->GetShortName(),
			zone->GetInstanceVersion(),
			this,
			target_npc->CastToNPC(),
			extra
		);
	}
}

bool Client::IsDraggingCorpse(uint16 CorpseID)
{
	for (auto It = DraggedCorpses.begin(); It != DraggedCorpses.end(); ++It) {
		if (It->second == CorpseID)
			return true;
	}

	return false;
}

void Client::DragCorpses()
{
	for (auto It = DraggedCorpses.begin(); It != DraggedCorpses.end(); ++It) {
		Mob *corpse = entity_list.GetMob(It->second);

		if (corpse && corpse->IsPlayerCorpse() &&
				(DistanceSquaredNoZ(m_Position, corpse->GetPosition()) <= RuleR(Character, DragCorpseDistance)))
			continue;

		if (!corpse || !corpse->IsPlayerCorpse() ||
				corpse->CastToCorpse()->IsBeingLooted() ||
				!corpse->CastToCorpse()->Summon(this, false, false)) {
			MessageString(Chat::DefaultText, CORPSEDRAG_STOP);
			It = DraggedCorpses.erase(It);
			if (It == DraggedCorpses.end())
				break;
		}
	}
}

void Client::ConsentCorpses(std::string consent_name, bool deny)
{
	if (strcasecmp(consent_name.c_str(), GetName()) == 0) {
		MessageString(Chat::Red, CONSENT_YOURSELF);
	}
	else if (!consent_throttle_timer.Check()) {
		MessageString(Chat::Red, CONSENT_WAIT);
	}
	else {
		auto pack = new ServerPacket(ServerOP_Consent, sizeof(ServerOP_Consent_Struct));
		ServerOP_Consent_Struct* scs = (ServerOP_Consent_Struct*)pack->pBuffer;
		strn0cpy(scs->grantname, consent_name.c_str(), sizeof(scs->grantname));
		strn0cpy(scs->ownername, GetName(), sizeof(scs->ownername));
		strn0cpy(scs->zonename, "Unknown", sizeof(scs->zonename));
		scs->permission = deny ? 0 : 1;
		scs->zone_id = zone->GetZoneID();
		scs->instance_id = zone->GetInstanceID();
		scs->consent_type = EQ::consent::Normal;
		scs->consent_id = 0;
		if (strcasecmp(scs->grantname, "group") == 0) {
			if (!deny) {
				Group* grp = GetGroup();
				scs->consent_id = grp ? grp->GetID() : 0;
			}
			scs->consent_type = EQ::consent::Group;
		}
		else if (strcasecmp(scs->grantname, "raid") == 0) {
			if (!deny) {
				Raid* raid = GetRaid();
				scs->consent_id = raid ? raid->GetID() : 0;
			}
			scs->consent_type = EQ::consent::Raid;
		}
		else if (strcasecmp(scs->grantname, "guild") == 0) {
			if (!deny) {
				scs->consent_id = GuildID();
			}
			scs->consent_type = EQ::consent::Guild;
			// update all corpses in db so buried/unloaded corpses see new consent id
			database.UpdateCharacterCorpseConsent(CharacterID(), scs->consent_id);
		}
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void Client::Doppelganger(uint16 spell_id, Mob *target, const char *name_override, int pet_count, int pet_duration)
{
	if(!target || !IsValidSpell(spell_id) || GetID() == target->GetID())
		return;

	PetRecord record;
	if(!database.GetPetEntry(spells[spell_id].teleport_zone, &record))
	{
		LogError("Unknown doppelganger spell id: [{}], check pets table", spell_id);
		Message(Chat::Red, "Unable to find data for pet %s", spells[spell_id].teleport_zone);
		return;
	}

	SwarmPet_Struct pet;
	pet.count = pet_count;
	pet.duration = pet_duration;
	pet.npc_id = record.npc_type;

	NPCType *made_npc = nullptr;

	const NPCType *npc_type = content_db.LoadNPCTypesData(pet.npc_id);
	if(npc_type == nullptr) {
		LogError("Unknown npc type for doppelganger spell id: [{}]", spell_id);
		Message(0,"Unable to find pet!");
		return;
	}
	// make a custom NPC type for this
	made_npc = new NPCType;
	memcpy(made_npc, npc_type, sizeof(NPCType));

	strcpy(made_npc->name, name_override);
	made_npc->level = GetLevel();
	made_npc->race = GetRace();
	made_npc->gender = GetGender();
	made_npc->size = GetSize();
	made_npc->AC = GetAC();
	made_npc->STR = GetSTR();
	made_npc->STA = GetSTA();
	made_npc->DEX = GetDEX();
	made_npc->AGI = GetAGI();
	made_npc->MR = GetMR();
	made_npc->FR = GetFR();
	made_npc->CR = GetCR();
	made_npc->DR = GetDR();
	made_npc->PR = GetPR();
	made_npc->Corrup = GetCorrup();
	made_npc->PhR = GetPhR();
	// looks
	made_npc->texture = GetEquipmentMaterial(EQ::textures::armorChest);
	made_npc->helmtexture = GetEquipmentMaterial(EQ::textures::armorHead);
	made_npc->haircolor = GetHairColor();
	made_npc->beardcolor = GetBeardColor();
	made_npc->eyecolor1 = GetEyeColor1();
	made_npc->eyecolor2 = GetEyeColor2();
	made_npc->hairstyle = GetHairStyle();
	made_npc->luclinface = GetLuclinFace();
	made_npc->beard = GetBeard();
	made_npc->drakkin_heritage = GetDrakkinHeritage();
	made_npc->drakkin_tattoo = GetDrakkinTattoo();
	made_npc->drakkin_details = GetDrakkinDetails();
	made_npc->d_melee_texture1 = GetEquipmentMaterial(EQ::textures::weaponPrimary);
	made_npc->d_melee_texture2 = GetEquipmentMaterial(EQ::textures::weaponSecondary);
	for (int i = EQ::textures::textureBegin; i <= EQ::textures::LastTexture; i++)	{
		made_npc->armor_tint.Slot[i].Color = GetEquipmentColor(i);
	}
	made_npc->loottable_id = 0;

	int summon_count = pet.count;

	if(summon_count > MAX_SWARM_PETS)
		summon_count = MAX_SWARM_PETS;

	static const glm::vec2 swarmPetLocations[MAX_SWARM_PETS] = {
		glm::vec2(5, 5), glm::vec2(-5, 5), glm::vec2(5, -5), glm::vec2(-5, -5),
		glm::vec2(10, 10), glm::vec2(-10, 10), glm::vec2(10, -10), glm::vec2(-10, -10),
		glm::vec2(8, 8), glm::vec2(-8, 8), glm::vec2(8, -8), glm::vec2(-8, -8)
	};

	while(summon_count > 0) {
		auto npc_type_copy = new NPCType;
		memcpy(npc_type_copy, made_npc, sizeof(NPCType));

		NPC* swarm_pet_npc = new NPC(
				npc_type_copy,
				0,
				GetPosition() + glm::vec4(swarmPetLocations[summon_count - 1], 0.0f, 0.0f),
				GravityBehavior::Water);

		if(!swarm_pet_npc->GetSwarmInfo()){
			auto nSI = new SwarmPet;
			swarm_pet_npc->SetSwarmInfo(nSI);
			swarm_pet_npc->GetSwarmInfo()->duration = new Timer(pet_duration*1000);
		}
		else{
			swarm_pet_npc->GetSwarmInfo()->duration->Start(pet_duration*1000);
		}

		swarm_pet_npc->StartSwarmTimer(pet_duration * 1000);

		swarm_pet_npc->GetSwarmInfo()->owner_id = GetID();
		swarm_pet_npc->SetFollowID(GetID());

		// Give the pets alittle more agro than the caster and then agro them on the target
		target->AddToHateList(swarm_pet_npc, (target->GetHateAmount(this) + 100), (target->GetDamageAmount(this) + 100));
		swarm_pet_npc->AddToHateList(target, 1000, 1000);
		swarm_pet_npc->GetSwarmInfo()->target = 0;

		//we allocated a new NPC type object, give the NPC ownership of that memory
		swarm_pet_npc->GiveNPCTypeData(npc_type_copy);

		entity_list.AddNPC(swarm_pet_npc);
		summon_count--;
	}

	safe_delete(made_npc);
}

void Client::AssignToInstance(uint16 instance_id)
{
	database.AddClientToInstance(instance_id, CharacterID());
}

void Client::RemoveFromInstance(uint16 instance_id)
{
	database.RemoveClientFromInstance(instance_id, CharacterID());
}

void Client::SendAltCurrencies() {
	if (ClientVersion() >= EQ::versions::ClientVersion::SoF) {
		const uint32 currency_count = zone->AlternateCurrencies.size();
		if (!currency_count) {
			return;
		}

		auto outapp = new EQApplicationPacket(
			OP_AltCurrency,
			sizeof(AltCurrencyPopulate_Struct) +
			sizeof(AltCurrencyPopulateEntry_Struct) * currency_count
		);

		auto a = (AltCurrencyPopulate_Struct*) outapp->pBuffer;

		a->opcode = AlternateCurrencyMode::Populate;
		a->count  = currency_count;

		uint32 currency_id = 0;
		for (const auto& c : zone->AlternateCurrencies) {
			const auto* item = database.GetItem(c.item_id);

			a->entries[currency_id].currency_number  = c.id;
			a->entries[currency_id].unknown00        = 1;
			a->entries[currency_id].currency_number2 = c.id;
			a->entries[currency_id].item_id          = c.item_id;
			a->entries[currency_id].item_icon        = item ? item->Icon : 1000;
			a->entries[currency_id].stack_size       = item ? item->StackSize : 1000;

			currency_id++;
		}

		FastQueuePacket(&outapp);
	}
}

void Client::SetAlternateCurrencyValue(uint32 currency_id, uint32 new_amount)
{
	if (!zone->DoesAlternateCurrencyExist(currency_id)) {
		return;
	}

	const uint32 current_amount = alternate_currency[currency_id];

	const bool is_gain = new_amount > current_amount;

	const uint32 change_amount = is_gain ? (new_amount - current_amount) : (current_amount - new_amount);

	if (!change_amount) {
		return;
	}

	alternate_currency[currency_id] = new_amount;
	database.UpdateAltCurrencyValue(CharacterID(), currency_id, new_amount);
	SendAlternateCurrencyValue(currency_id);

	QuestEventID event_id = is_gain ? EVENT_ALT_CURRENCY_GAIN : EVENT_ALT_CURRENCY_LOSS;
	if (parse->PlayerHasQuestSub(event_id)) {
		const std::string &export_string = fmt::format(
			"{} {} {}",
			currency_id,
			change_amount,
			new_amount
		);

		parse->EventPlayer(event_id, this, export_string, 0);
	}
}

bool Client::RemoveAlternateCurrencyValue(uint32 currency_id, uint32 amount)
{
	if (!amount || !zone->DoesAlternateCurrencyExist(currency_id)) {
		return false;
	}

	const uint32 current_amount = alternate_currency[currency_id];
	if (current_amount < amount) {
		return false;
	}

	const uint32 new_amount = (current_amount - amount);

	alternate_currency[currency_id] = new_amount;
	database.UpdateAltCurrencyValue(CharacterID(), currency_id, new_amount);
	SendAlternateCurrencyValue(currency_id);

	if (parse->PlayerHasQuestSub(EVENT_ALT_CURRENCY_LOSS)) {
		const std::string &export_string = fmt::format(
			"{} {} {}",
			currency_id,
			amount,
			new_amount
		);

		parse->EventPlayer(EVENT_ALT_CURRENCY_LOSS, this, export_string, 0);
	}

	return true;
}

int Client::AddAlternateCurrencyValue(uint32 currency_id, int amount, bool is_scripted)
{
	if (!zone->DoesAlternateCurrencyExist(currency_id)) {
		return 0;
	}

	if (!amount) {
		return 0;
	}

	if (!alternate_currency_loaded) {
		alternate_currency_queued_operations.push(std::make_pair(currency_id, amount));
		return 0;
	}

	int new_value = 0;
	auto iter = alternate_currency.find(currency_id);
	if (iter == alternate_currency.end()) {
		new_value = amount;
	} else {
		new_value = (*iter).second + amount;
	}

	if (new_value < 0) {
		new_value = 0;
		alternate_currency[currency_id] = 0;
		database.UpdateAltCurrencyValue(CharacterID(), currency_id, 0);
	} else {
		alternate_currency[currency_id] = new_value;
		database.UpdateAltCurrencyValue(CharacterID(), currency_id, new_value);
	}

	SendAlternateCurrencyValue(currency_id);

	QuestEventID event_id = amount > 0 ? EVENT_ALT_CURRENCY_GAIN : EVENT_ALT_CURRENCY_LOSS;
	if (parse->PlayerHasQuestSub(event_id)) {
		const std::string &export_string = fmt::format(
			"{} {} {}",
			currency_id,
			std::abs(amount),
			new_value
		);

		parse->EventPlayer(event_id, this, export_string, 0);
	}

	return new_value;
}

void Client::SendAlternateCurrencyValues()
{
	for (const auto& alternate_currency : zone->AlternateCurrencies) {
		SendAlternateCurrencyValue(alternate_currency.id, false);
	}
}

void Client::SendAlternateCurrencyValue(uint32 currency_id, bool send_if_null)
{
	const auto value = GetAlternateCurrencyValue(currency_id);
	if (value > 0 || send_if_null) {
		auto outapp = new EQApplicationPacket(OP_AltCurrency, sizeof(AltCurrencyUpdate_Struct));
		auto update = (AltCurrencyUpdate_Struct *) outapp->pBuffer;
		update->opcode          = 7;
		update->currency_number = currency_id;
		update->amount          = value;
		update->unknown072      = 1;

		strn0cpy(update->name, GetName(), sizeof(update->name));

		FastQueuePacket(&outapp);
	}
}

uint32 Client::GetAlternateCurrencyValue(uint32 currency_id) const
{
	if (!zone->DoesAlternateCurrencyExist(currency_id)) {
		return 0;
	}

	auto iter = alternate_currency.find(currency_id);

	return iter == alternate_currency.end() ? 0 : (*iter).second;
}

void Client::ProcessAlternateCurrencyQueue() {
	while(!alternate_currency_queued_operations.empty()) {
		std::pair<uint32, int32> op = alternate_currency_queued_operations.front();

		AddAlternateCurrencyValue(op.first, op.second);

		alternate_currency_queued_operations.pop();
	}
}

void Client::OpenLFGuildWindow()
{
	auto outapp = new EQApplicationPacket(OP_LFGuild, 8);

	outapp->WriteUInt32(6);

	FastQueuePacket(&outapp);
}

bool Client::IsXTarget(const Mob *m) const
{
	if(!XTargettingAvailable() || !m || (m->GetID() == 0))
		return false;

	for(int i = 0; i < GetMaxXTargets(); ++i)
	{
		if(XTargets[i].ID == m->GetID())
			return true;
	}
	return false;
}

bool Client::IsClientXTarget(const Client *c) const
{
	if(!XTargettingAvailable() || !c)
		return false;

	for(int i = 0; i < GetMaxXTargets(); ++i)
	{
		if(!strcasecmp(XTargets[i].Name, c->GetName()))
			return true;
	}
	return false;
}


void Client::UpdateClientXTarget(Client *c)
{
	if(!XTargettingAvailable() || !c)
		return;

	for(int i = 0; i < GetMaxXTargets(); ++i)
	{
		if(!strcasecmp(XTargets[i].Name, c->GetName()))
		{
			XTargets[i].ID = c->GetID();
			SendXTargetPacket(i, c);
		}
	}
}

// IT IS NOT SAFE TO CALL THIS IF IT'S NOT INITIAL AGGRO
void Client::AddAutoXTarget(Mob *m, bool send)
{
	if (m->IsBot() || ((m->IsPet() || m->IsTempPet()) && m->IsPetOwnerBot())) {
		return;
	}

	m_activeautohatermgr->increment_count(m);

	if (!XTargettingAvailable() || !XTargetAutoAddHaters || IsXTarget(m)) {
		return;
	}

	for (int i = 0; i < GetMaxXTargets(); ++i) {
		if (XTargets[i].Type == Auto && XTargets[i].ID == 0) {
			XTargets[i].ID = m->GetID();

			if (send) { // if we don't send we're bulk sending updates later on
				SendXTargetPacket(i, m);
			} else {
				XTargets[i].dirty = true;
			}

			break;
		}
	}

	LogXTargets(
		"Adding [{}] to [{}] ({}) XTargets",
		m->GetCleanName(),
		GetCleanName(),
		GetID()
	);
}

void Client::RemoveXTarget(Mob *m, bool OnlyAutoSlots)
{
	if (!XTargettingAvailable() || !m || !m_activeautohatermgr) {
		return;
	}

	m_activeautohatermgr->decrement_count(m);
	// now we may need to clean up our CurrentTargetNPC entries
	for (int i = 0; i < GetMaxXTargets(); ++i) {
		if (XTargets[i].Type == CurrentTargetNPC && XTargets[i].ID == m->GetID()) {
			XTargets[i].Type  = Auto;
			XTargets[i].ID    = 0;
			XTargets[i].dirty = true;
		}
	}

	auto r = GetRaid();
	if (r) {
		r->UpdateRaidXTargets();
	}

	LogXTargets(
		"Removing [{}] from [{}] ({}) XTargets",
		m->GetCleanName(),
		GetCleanName(),
		GetID()
	);
}

void Client::UpdateXTargetType(XTargetType Type, Mob *m, const char *Name)
{
	if (!XTargettingAvailable()) {
		return;
	}

	for (int i = 0; i < GetMaxXTargets(); ++i) {
		if (XTargets[i].Type == Type) {
			if (m) {
				XTargets[i].ID = m->GetID();
			}
			else {
				XTargets[i].ID = 0;
			}

			if (Name) {
				strncpy(XTargets[i].Name, Name, 64);
			}

			SendXTargetPacket(i, m);
		}
	}
}

void Client::SendXTargetPacket(uint32 Slot, Mob *m)
{
	if(!XTargettingAvailable())
		return;

	uint32 PacketSize = 18;

	if(m)
		PacketSize += strlen(m->GetCleanName());
	else
	{
		PacketSize += strlen(XTargets[Slot].Name);
	}

	auto outapp = new EQApplicationPacket(OP_XTargetResponse, PacketSize);
	outapp->WriteUInt32(GetMaxXTargets());
	outapp->WriteUInt32(1);
	outapp->WriteUInt32(Slot);
	if(m)
	{
		outapp->WriteUInt8(1);
	}
	else
	{
		if (strlen(XTargets[Slot].Name) && ((XTargets[Slot].Type == CurrentTargetPC) ||
			(XTargets[Slot].Type == GroupTank) ||
			(XTargets[Slot].Type == GroupAssist) ||
			(XTargets[Slot].Type == Puller)))
		{
			outapp->WriteUInt8(2);
		}
		else
		{
			outapp->WriteUInt8(0);
		}
	}
	outapp->WriteUInt32(XTargets[Slot].ID);
	outapp->WriteString(m ? m->GetCleanName() : XTargets[Slot].Name);
	FastQueuePacket(&outapp);
}

// This is a bulk packet, we use it when we remove something since we need to reorder the xtargets and maybe
// add new mobs! Currently doesn't check if there is a dirty flag set, so it should only be called when there is
void Client::SendXTargetUpdates()
{
	if (!XTargettingAvailable())
		return;

	int count = 0;
	// header is 4 bytes max xtargets, 4 bytes count
	// entry is 4 bytes slot, 1 byte unknown, 4 bytes ID, 65 char name
	auto outapp = new EQApplicationPacket(OP_XTargetResponse, 8 + 74 * GetMaxXTargets()); // fuck it max size
	outapp->WriteUInt32(GetMaxXTargets());
	outapp->WriteUInt32(1); // we will correct this later
	for (int i = 0; i < GetMaxXTargets(); ++i) {
		if (XTargets[i].dirty) {
			outapp->WriteUInt32(i);
			// MQ2 checks this for valid mobs, so 0 is bad here at least ...
			outapp->WriteUInt8(XTargets[i].ID ? 1 : 0);
			outapp->WriteUInt32(XTargets[i].ID);
			outapp->WriteString(XTargets[i].Name);
			count++;
			XTargets[i].dirty = false;
		}
	}

	// RemoveXTarget probably got called with a mob not on our xtargets
	if (count == 0) {
		safe_delete(outapp);
		return;
	}

	auto newbuff = new uchar[outapp->GetWritePosition()];
	memcpy(newbuff, outapp->pBuffer, outapp->GetWritePosition());
	safe_delete_array(outapp->pBuffer);
	outapp->pBuffer = newbuff;
	outapp->size = outapp->GetWritePosition();
	outapp->SetWritePosition(4);
	outapp->WriteUInt32(count);
	FastQueuePacket(&outapp);
}

void Client::RemoveGroupXTargets()
{
	if(!XTargettingAvailable())
		return;

	for(int i = 0; i < GetMaxXTargets(); ++i)
	{
		if ((XTargets[i].Type == GroupTank) ||
			(XTargets[i].Type == GroupAssist) ||
			(XTargets[i].Type == Puller))
		{
			XTargets[i].ID = 0;
			XTargets[i].Name[0] = 0;
			SendXTargetPacket(i, nullptr);
		}
	}
}

void Client::RemoveAutoXTargets()
{
	if(!XTargettingAvailable())
		return;

	for(int i = 0; i < GetMaxXTargets(); ++i)
	{
		if(XTargets[i].Type == Auto)
		{
			XTargets[i].ID = 0;
			XTargets[i].Name[0] = 0;
			SendXTargetPacket(i, nullptr);
		}
	}
}

void Client::ShowXTargets(Client *c)
{
	if (!c) {
		return;
	}

	auto xtarget_count = 0;

	for (int i = 0; i < GetMaxXTargets(); ++i) {
		c->Message(
			Chat::White,
			fmt::format(
				"xtarget slot [{}] type [{}] ID [{}] name [{}]",
				i,
				static_cast<int>(XTargets[i].Type),
				XTargets[i].ID,
				strlen(XTargets[i].Name) ? XTargets[i].Name : "No Name"
			).c_str()
		);

		xtarget_count++;
	}

	auto &list = GetXTargetAutoMgr()->get_list();
	 // yeah, I kept having to do something for debugging to tell if managers were the same object or not :P
	 // so lets use the address as an "ID"
	c->Message(
		Chat::White,
		fmt::format(
			"XTargetAutoMgr ID [{}] size [{}]",
			fmt::ptr(GetXTargetAutoMgr()),
			list.size()
		).c_str()
	);

	int count = 0;
	for (auto &e : list) {
		c->Message(
			Chat::White,
			fmt::format(
				"Spawn ID: {} Count: {}",
				e.spawn_id,
				e.count
			).c_str()
		);

		count++;

		if (count == 20) {
			break;
		}
	}
}

void Client::ProcessXTargetAutoHaters()
{
	if (!XTargettingAvailable())
		return;

	// move shit up! If the removed NPC was in a CurrentTargetNPC slot it becomes Auto
	// and we need to potentially fill it
	std::queue<int> empty_slots;
	for (int i = 0; i < GetMaxXTargets(); ++i) {
		if (XTargets[i].Type != Auto)
			continue;

		if (XTargets[i].ID != 0 && !GetXTargetAutoMgr()->contains_mob(XTargets[i].ID)) {
			XTargets[i].ID = 0;
			XTargets[i].dirty = true;
		}

		if (XTargets[i].ID == 0) {
			empty_slots.push(i);
			continue;
		}

		if (XTargets[i].ID != 0 && !empty_slots.empty()) {
			int temp = empty_slots.front();
			std::swap(XTargets[i], XTargets[temp]);
			XTargets[i].dirty = XTargets[temp].dirty = true;
			empty_slots.pop();
			empty_slots.push(i);
		}
	}
	// okay, now we need to check if we have any empty slots and if we have aggro
	// We make the assumption that if we shuffled the NPCs up that they're still on the aggro
	// list in the same order. We could probably do this better and try to calc if
	// there are new NPCs for our empty slots on the manager, but ahhh fuck it.
	if (!empty_slots.empty() && !GetXTargetAutoMgr()->empty() && XTargetAutoAddHaters) {
		auto &haters = GetXTargetAutoMgr()->get_list();
		for (auto &e : haters) {
			auto *mob = entity_list.GetMob(e.spawn_id);
			if (mob && !IsXTarget(mob)) {
				auto slot = empty_slots.front();
				empty_slots.pop();
				XTargets[slot].dirty = true;
				XTargets[slot].ID = mob->GetID();
				strn0cpy(XTargets[slot].Name, mob->GetCleanName(), 64);
			}
			if (empty_slots.empty())
				break;
		}
	}

	m_dirtyautohaters = false;
	SendXTargetUpdates();
}

// This function is called when a client is added to a group
// Group leader joining isn't handled by this function
void Client::JoinGroupXTargets(Group *g)
{
	if (!g)
		return;

	if (!GetXTargetAutoMgr()->empty()) {
		g->GetXTargetAutoMgr()->merge(*GetXTargetAutoMgr());
		GetXTargetAutoMgr()->clear();
		RemoveAutoXTargets();
	}

	SetXTargetAutoMgr(g->GetXTargetAutoMgr());

	if (!GetXTargetAutoMgr()->empty())
		SetDirtyAutoHaters();
}

// This function is called when a client leaves a group
void Client::LeaveGroupXTargets(Group *g)
{
	if (!g)
		return;

	SetXTargetAutoMgr(nullptr); // this will set it back to our manager
	RemoveAutoXTargets();
	entity_list.RefreshAutoXTargets(this); // this will probably break the temporal ordering, but whatever
	// We now have a rebuilt, valid auto hater manager, so we need to demerge from the groups
	if (!GetXTargetAutoMgr()->empty()) {
		GetXTargetAutoMgr()->demerge(*g->GetXTargetAutoMgr()); // this will remove entries where we only had aggro
		SetDirtyAutoHaters();
	}
}

// This function is called when a client leaves a group
void Client::LeaveRaidXTargets(Raid *r)
{
	if (!r)
		return;

	SetXTargetAutoMgr(nullptr); // this will set it back to our manager
	RemoveAutoXTargets();
	entity_list.RefreshAutoXTargets(this); // this will probably break the temporal ordering, but whatever
	// We now have a rebuilt, valid auto hater manager, so we need to demerge from the groups
	if (!GetXTargetAutoMgr()->empty()) {
		GetXTargetAutoMgr()->demerge(*r->GetXTargetAutoMgr()); // this will remove entries where we only had aggro
		SetDirtyAutoHaters();
	}
}

void Client::SetMaxXTargets(uint8 NewMax)
{
	if(!XTargettingAvailable())
		return;

	if(NewMax > XTARGET_HARDCAP)
		return;

	MaxXTargets = NewMax;

	Save(0);

	for(int i = MaxXTargets; i < XTARGET_HARDCAP; ++i)
	{
		XTargets[i].Type = Auto;
		XTargets[i].ID = 0;
		XTargets[i].Name[0] = 0;
	}

	auto outapp = new EQApplicationPacket(OP_XTargetResponse, 8);
	outapp->WriteUInt32(GetMaxXTargets());
	outapp->WriteUInt32(0);
	FastQueuePacket(&outapp);
}

void Client::SendWebLink(const char *website)
{
	if (website) {
		size_t len = strlen(website) + 1;
		if (len > 1)
		{
			auto outapp = new EQApplicationPacket(OP_Weblink, sizeof(Weblink_Struct) + len);
			Weblink_Struct* wl = (Weblink_Struct*)outapp->pBuffer;
			memcpy(wl->weblink, website, len);
			wl->weblink[len] = '\0';

			FastQueuePacket(&outapp);
		}
	}
}

void Client::SendMercPersonalInfo()
{
	uint32 mercTypeCount = 1;
	uint32 mercCount = 1; //TODO: Un-hardcode this and support multiple mercs like in later clients than SoD.
	uint32 i = 0;
	uint32 altCurrentType = 19; //TODO: Implement alternate currency purchases involving mercs!

	MercTemplate *mercData = &zone->merc_templates[GetMercInfo().MercTemplateID];

	int stancecount = 0;
	stancecount += zone->merc_stance_list[GetMercInfo().MercTemplateID].size();
	if(stancecount > MAX_MERC_STANCES || mercCount > MAX_MERC || mercTypeCount > MAX_MERC_GRADES)
	{
		Log(Logs::General, Logs::Mercenaries, "SendMercPersonalInfo canceled: (%i) (%i) (%i) for %s", stancecount, mercCount, mercTypeCount, GetName());
		SendMercMerchantResponsePacket(0);
		return;
	}

	if (ClientVersion() >= EQ::versions::ClientVersion::RoF) {
		auto outapp = new EQApplicationPacket(OP_MercenaryDataUpdate, sizeof(MercenaryDataUpdate_Struct));
		auto mdus   = (MercenaryDataUpdate_Struct *) outapp->pBuffer;

		mdus->MercStatus                    = 0;
		mdus->MercCount                     = mercCount;
		mdus->MercData[i].MercID            = mercData->MercTemplateID;
		mdus->MercData[i].MercType          = mercData->MercType;
		mdus->MercData[i].MercSubType       = mercData->MercSubType;
		mdus->MercData[i].PurchaseCost      = Merc::CalcPurchaseCost(mercData->MercTemplateID, GetLevel(), 0);
		mdus->MercData[i].UpkeepCost        = Merc::CalcUpkeepCost(mercData->MercTemplateID, GetLevel(), 0);
		mdus->MercData[i].Status            = 0;
		mdus->MercData[i].AltCurrencyCost   = Merc::CalcPurchaseCost(
			mercData->MercTemplateID,
			GetLevel(),
			altCurrentType
		);
		mdus->MercData[i].AltCurrencyUpkeep = Merc::CalcPurchaseCost(
			mercData->MercTemplateID,
			GetLevel(),
			altCurrentType
		);
		mdus->MercData[i].AltCurrencyType   = altCurrentType;
		mdus->MercData[i].MercUnk01         = 0;
		mdus->MercData[i].TimeLeft          = GetMercInfo().MercTimerRemaining;    //GetMercTimer().GetRemainingTime();
		mdus->MercData[i].MerchantSlot      = i + 1;
		mdus->MercData[i].MercUnk02         = 1;
		mdus->MercData[i].StanceCount       = zone->merc_stance_list[mercData->MercTemplateID].size();
		mdus->MercData[i].MercUnk03         = 0;
		mdus->MercData[i].MercUnk04         = 1;

		strn0cpy(mdus->MercData[i].MercName, GetMercInfo().merc_name, sizeof(mdus->MercData[i].MercName));

		uint32 stanceindex = 0;
		if (mdus->MercData[i].StanceCount != 0) {
			auto iter = zone->merc_stance_list[mercData->MercTemplateID].begin();
			while (iter != zone->merc_stance_list[mercData->MercTemplateID].end()) {
				mdus->MercData[i].Stances[stanceindex].StanceIndex = stanceindex;
				mdus->MercData[i].Stances[stanceindex].Stance      = (iter->StanceID);
				stanceindex++;
				++iter;
			}
		}

		mdus->MercData[i].MercUnk05 = 1;
		FastQueuePacket(&outapp);
		safe_delete(outapp);
		return;
	} else {
		auto outapp = new EQApplicationPacket(OP_MercenaryDataResponse, sizeof(MercenaryMerchantList_Struct));
		auto mml    = (MercenaryMerchantList_Struct *) outapp->pBuffer;

		mml->MercTypeCount = mercTypeCount; //We should only have one merc entry.
		mml->MercGrades[i] = 1;

		mml->MercCount                  = mercCount;
		mml->Mercs[i].MercID            = mercData->MercTemplateID;
		mml->Mercs[i].MercType          = mercData->MercType;
		mml->Mercs[i].MercSubType       = mercData->MercSubType;
		mml->Mercs[i].PurchaseCost      = RuleB(Mercs, ChargeMercPurchaseCost) ? Merc::CalcPurchaseCost(mercData->MercTemplateID, GetLevel(), 0) : 0;
		mml->Mercs[i].UpkeepCost        = RuleB(Mercs, ChargeMercUpkeepCost) ? Merc::CalcUpkeepCost(mercData->MercTemplateID, GetLevel(), 0) : 0;
		mml->Mercs[i].Status            = 0;
		mml->Mercs[i].AltCurrencyCost   = RuleB(Mercs, ChargeMercPurchaseCost) ? Merc::CalcPurchaseCost(mercData->MercTemplateID, GetLevel(), altCurrentType) : 0;
		mml->Mercs[i].AltCurrencyUpkeep = RuleB(Mercs, ChargeMercUpkeepCost) ? Merc::CalcUpkeepCost(mercData->MercTemplateID, GetLevel(), altCurrentType) : 0;
		mml->Mercs[i].AltCurrencyType   = altCurrentType;
		mml->Mercs[i].MercUnk01         = 0;
		mml->Mercs[i].TimeLeft          = GetMercInfo().MercTimerRemaining;
		mml->Mercs[i].MerchantSlot      = i + 1;
		mml->Mercs[i].MercUnk02         = 1;
		mml->Mercs[i].StanceCount       = zone->merc_stance_list[mercData->MercTemplateID].size();
		mml->Mercs[i].MercUnk03         = 0;
		mml->Mercs[i].MercUnk04         = 1;

		strn0cpy(mml->Mercs[i].MercName, GetMercInfo().merc_name, sizeof(mml->Mercs[i].MercName));

		int stanceindex = 0;
		if (mml->Mercs[i].StanceCount != 0) {
			auto iter = zone->merc_stance_list[mercData->MercTemplateID].begin();
			while (iter != zone->merc_stance_list[mercData->MercTemplateID].end()) {
				mml->Mercs[i].Stances[stanceindex].StanceIndex = stanceindex;
				mml->Mercs[i].Stances[stanceindex].Stance      = (iter->StanceID);
				stanceindex++;
				++iter;
			}
		}

		FastQueuePacket(&outapp);
		safe_delete(outapp);
		return;
	}
}

void Client::SendClearMercInfo()
{
	auto outapp = new EQApplicationPacket(OP_MercenaryDataUpdate, sizeof(NoMercenaryHired_Struct));
	NoMercenaryHired_Struct *nmhs = (NoMercenaryHired_Struct*)outapp->pBuffer;
	nmhs->MercStatus = -1;
	nmhs->MercCount = 0;
	nmhs->MercID = 1;
	FastQueuePacket(&outapp);
}


void Client::DuplicateLoreMessage(uint32 ItemID)
{
	if (!(m_ClientVersionBit & EQ::versions::maskRoFAndLater))
	{
		MessageString(Chat::White, PICK_LORE);
		return;
	}

	const EQ::ItemData *item = database.GetItem(ItemID);

	if(!item)
		return;

	MessageString(Chat::White, PICK_LORE, item->Name);
}

void Client::GarbleMessage(char *message, uint8 variance)
{
	// Garble message by variance%
	const char alpha_list[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"; // only change alpha characters for now
	const char delimiter = 0x12;
	int delimiter_count = 0;

	// Don't garble # commands
	if (message[0] == COMMAND_CHAR || message[0] == BOT_COMMAND_CHAR) {
		return;
	}

	for (size_t i = 0; i < strlen(message); i++) {
		// Client expects hex values inside of a text link body
		if (message[i] == delimiter) {
			if (!(delimiter_count & 1)) { i += EQ::constants::SAY_LINK_BODY_SIZE; }
			++delimiter_count;
			continue;
		}

		uint8 chance = (uint8)zone->random.Int(0, 115); // variation just over worst possible scrambling
		if (isalpha((unsigned char)message[i]) && (chance <= variance)) {
			uint8 rand_char = (uint8)zone->random.Int(0,51); // choose a random character from the alpha list
			message[i] = alpha_list[rand_char];
		}
	}
}

// returns what Other thinks of this
FACTION_VALUE Client::GetReverseFactionCon(Mob* iOther) {
	if (GetOwnerID()) {
		return GetOwnerOrSelf()->GetReverseFactionCon(iOther);
	}

	iOther = iOther->GetOwnerOrSelf();

	if (iOther->GetPrimaryFaction() < 0)
		return GetSpecialFactionCon(iOther);

	if (iOther->GetPrimaryFaction() == 0)
		return FACTION_INDIFFERENTLY;

	return GetFactionLevel(CharacterID(), 0, GetFactionRace(), GetClass(), GetDeity(), iOther->GetPrimaryFaction(), iOther);
}

bool Client::ReloadCharacterFaction(Client *c, uint32 facid, uint32 charid)
{
	if (database.SetCharacterFactionLevel(charid, facid, 0, 0, factionvalues))
		return true;
	else
		return false;
}

//o--------------------------------------------------------------
//| Name: GetFactionLevel; Dec. 16, 2001
//o--------------------------------------------------------------
//| Notes: Gets the characters faction standing with the specified NPC.
//|			Will return Indifferent on failure.
//o--------------------------------------------------------------
FACTION_VALUE Client::GetFactionLevel(uint32 char_id, uint32 npc_id, uint32 p_race, uint32 p_class, uint32 p_deity, int32 pFaction, Mob* tnpc)
{
	if (pFaction < 0)
		return GetSpecialFactionCon(tnpc);
	FACTION_VALUE fac = FACTION_INDIFFERENTLY;
	int32 tmpFactionValue;
	FactionMods fmods;

	// few optimizations
	if (GetFeigned())
		return FACTION_INDIFFERENTLY;
	if(!zone->CanDoCombat())
		return FACTION_INDIFFERENTLY;
	if (invisible_undead && tnpc && !tnpc->SeeInvisibleUndead())
		return FACTION_INDIFFERENTLY;
	if (IsInvisible(tnpc))
		return FACTION_INDIFFERENTLY;
	if (tnpc && tnpc->GetOwnerID() != 0) // pets con amiably to owner and indiff to rest
	{
		if (char_id == tnpc->GetOwner()->CastToClient()->CharacterID())
			return FACTION_AMIABLY;
		else
			return FACTION_INDIFFERENTLY;
	}

	//First get the NPC's Primary faction
	if(pFaction > 0)
	{
		//Get the faction data from the database
		if(content_db.GetFactionData(&fmods, p_class, p_race, p_deity, pFaction))
		{
			//Get the players current faction with pFaction
			tmpFactionValue = GetCharacterFactionLevel(pFaction);
			//Tack on any bonuses from Alliance type spell effects
			tmpFactionValue += GetFactionBonus(pFaction);
			tmpFactionValue += GetItemFactionBonus(pFaction);
			//Return the faction to the client
			fac = CalculateFaction(&fmods, tmpFactionValue);
		}
	}
	else
	{
		return(FACTION_INDIFFERENTLY);
	}

	// merchant fix
	if (tnpc && tnpc->IsNPC() && tnpc->CastToNPC()->MerchantType && (fac == FACTION_THREATENINGLY || fac == FACTION_SCOWLS))
		fac = FACTION_DUBIOUSLY;

	if (tnpc != 0 && fac != FACTION_SCOWLS && tnpc->CastToNPC()->CheckAggro(this))
		fac = FACTION_THREATENINGLY;

	return fac;
}

//Sets the characters faction standing with the specified NPC.
void Client::SetFactionLevel(
	uint32 character_id,
	uint32 npc_faction_id,
	uint8 class_id,
	uint8 race_id,
	uint8 deity_id,
	bool is_quest
)
{
	auto l = zone->GetNPCFactionEntries(npc_faction_id);

	if (l.empty()) {
		return;
	}

	int current_value;

	for (auto& e : l) {
		if (e.faction_id <= 0 || e.value == 0) {
			continue;
		}

		int faction_before;
		int faction_minimum;
		int faction_maximum;

		FactionMods faction_modifiers;

		content_db.GetFactionData(&faction_modifiers, class_id, race_id, deity_id, e.faction_id);

		if (is_quest) {
			if (e.value > 0) {
				e.value = -std::abs(e.value);
			} else if (e.value < 0) {
				e.value = std::abs(e.value);
			}
		}

		// Adjust the amount you can go up or down so the resulting range
		// is PERSONAL_MAX - PERSONAL_MIN
		//
		// Adjust these values for cases where starting faction is below
		// min or above max by not allowing any earn in those directions.
		faction_minimum = faction_modifiers.min - faction_modifiers.base;
		faction_minimum = std::min(0, faction_minimum);

		faction_maximum = faction_modifiers.max - faction_modifiers.base;
		faction_maximum = std::max(0, faction_maximum);

		// Get the characters current value with that faction
		current_value = GetCharacterFactionLevel(e.faction_id);
		faction_before = current_value;

#ifdef LUA_EQEMU
		int32 lua_ret = 0;
		bool ignore_default = false;
		lua_ret = LuaParser::Instance()->UpdatePersonalFaction(this, e.value, e.faction_id, current_value, e.temp, faction_minimum, faction_maximum, ignore_default);

		if (ignore_default) {
			e.value = lua_ret;
		}
#endif

		UpdatePersonalFaction(
			character_id,
			e.value,
			e.faction_id,
			&current_value,
			e.temp,
			faction_minimum,
			faction_maximum
		);

		SendFactionMessage(
			e.value,
			e.faction_id,
			faction_before,
			current_value,
			e.temp,
			faction_minimum,
			faction_maximum
		);
	}
}

void Client::SetFactionLevel2(uint32 char_id, int32 faction_id, uint8 char_class, uint8 char_race, uint8 char_deity, int32 value, uint8 temp)
{
	int32 current_value;

	//Get the npc faction list
	if(faction_id > 0 && value != 0) {
		int32 faction_before_hit;
		FactionMods fm;
		int32 this_faction_max;
		int32 this_faction_min;

		// Find out starting faction for this faction
		// It needs to be used to adj max and min personal
		// The range is still the same, 1200-3000(4200), but adjusted for base
		content_db.GetFactionData(&fm, GetClass(), GetFactionRace(), GetDeity(),
			faction_id);

		// Adjust the amount you can go up or down so the resulting range
		// is PERSONAL_MAX - PERSONAL_MIN
		//
		// Adjust these values for cases where starting faction is below
		// min or above max by not allowing any earn/loss in those directions.
		// At least one faction starts out way below min, so we don't want
		// to allow loses in those cases, just massive gains.
		this_faction_min = fm.min - fm.base;
		this_faction_min = std::min(0, this_faction_min);
		this_faction_max = fm.max - fm.base;
		this_faction_max = std::max(0, this_faction_max);

		//Get the faction modifiers
		current_value = GetCharacterFactionLevel(faction_id);
		faction_before_hit = current_value;

#ifdef LUA_EQEMU
		int32 lua_ret = 0;
		bool ignore_default = false;
		lua_ret = LuaParser::Instance()->UpdatePersonalFaction(this, value, faction_id, current_value, temp, this_faction_min, this_faction_max, ignore_default);

		if (ignore_default) {
			value = lua_ret;
		}
#endif

		UpdatePersonalFaction(char_id, value, faction_id, &current_value, temp, this_faction_min, this_faction_max);

		//Message(Chat::Lime, "Min(%d) Max(%d) Before(%d), After(%d)\n", this_faction_min, this_faction_max, faction_before_hit, current_value);

		SendFactionMessage(value, faction_id, faction_before_hit, current_value, temp, this_faction_min, this_faction_max);
	}

	return;
}

int32 Client::GetCharacterFactionLevel(int32 faction_id)
{
	if (faction_id <= 0)
		return 0;
	faction_map::iterator res;
	res = factionvalues.find(faction_id);
	if (res == factionvalues.end())
		return 0;
	return res->second;
}

// Common code to set faction level.
// Applies HeroicCHA is it applies
// Checks for bottom out and max faction and old faction db entries
// Updates the faction if we are not minned, maxed or we need to repair

void Client::UpdatePersonalFaction(int32 char_id, int32 npc_value, int32 faction_id, int32 *current_value, int32 temp, int32 this_faction_min, int32 this_faction_max)
{
	bool repair = false;
	bool change = false;

	if (itembonuses.HeroicCHA)
	{
		int faction_mod = itembonuses.HeroicCHA / 5;
		// If our result isn't truncated, then just do that
		if (npc_value * faction_mod / 100 != 0)
			npc_value += npc_value * faction_mod / 100;
		// If our result is truncated, then double a mob's value every once and a while to equal what they would have got
		else
		{
			if (zone->random.Int(0, 100) < faction_mod)
				npc_value *= 2;
		}
	}

	// Set flag when to update db
	// Repair needed, as db changes could modify a base value for a faction
	// and we need to auto correct when that happens.
	if (*current_value > this_faction_max)
	{
		*current_value = this_faction_max;
		repair = true;
	}
	else if (*current_value < this_faction_min)
	{
		*current_value = this_faction_min;
		repair = true;
	}
	else if ((m_pp.gm != 1) && (npc_value != 0) &&
		((npc_value > 0 && *current_value != this_faction_max) ||
		((npc_value < 0 && *current_value != this_faction_min))))
		change = true;

	if (change || repair)
	{
		*current_value += npc_value;

		if (*current_value > this_faction_max)
			*current_value = this_faction_max;
		else if (*current_value < this_faction_min)
			*current_value = this_faction_min;

		database.SetCharacterFactionLevel(char_id, faction_id, *current_value, temp, factionvalues);
	}

return;
}

// returns the character's faction level, adjusted for racial, class, and deity modifiers
int32 Client::GetModCharacterFactionLevel(int32 faction_id) {
	int32 Modded = GetCharacterFactionLevel(faction_id);
	FactionMods fm;
	if (content_db.GetFactionData(&fm, GetClass(), GetFactionRace(), GetDeity(), faction_id))
	{
		Modded += fm.base + fm.class_mod + fm.race_mod + fm.deity_mod;

		//Tack on any bonuses from Alliance type spell effects
		Modded += GetFactionBonus(faction_id);
		Modded += GetItemFactionBonus(faction_id);
	}

	return Modded;
}

void Client::MerchantRejectMessage(Mob *merchant, int primaryfaction)
{
	int messageid = 0;
	int32 tmpFactionValue = 0;
	int32 lowestvalue = 0;
	FactionMods fmod;

	// If a faction is involved, get the data.
	if (primaryfaction > 0) {
		if (content_db.GetFactionData(&fmod, GetClass(), GetFactionRace(), GetDeity(), primaryfaction)) {
			tmpFactionValue = GetCharacterFactionLevel(primaryfaction);
			lowestvalue = std::min(std::min(tmpFactionValue, fmod.deity_mod),
						  std::min(fmod.class_mod, fmod.race_mod));
		}
	}
	// If no primary faction or biggest influence is your faction hit
	if (primaryfaction <= 0 || lowestvalue == tmpFactionValue) {
		merchant->SayString(zone->random.Int(WONT_SELL_DEEDS1, WONT_SELL_DEEDS6));
	} else if (lowestvalue == fmod.race_mod) { // race biggest
		// Non-standard race (ex. illusioned to wolf)
		if (!IsPlayerRace(GetRace())) {
			messageid = zone->random.Int(1, 3); // these aren't sequential StringIDs :(
			switch (messageid) {
			case 1:
				messageid = WONT_SELL_NONSTDRACE1;
				break;
			case 2:
				messageid = WONT_SELL_NONSTDRACE2;
				break;
			case 3:
				messageid = WONT_SELL_NONSTDRACE3;
				break;
			default: // w/e should never happen
				messageid = WONT_SELL_NONSTDRACE1;
				break;
			}
			merchant->SayString(messageid);
		} else { // normal player races
			messageid = zone->random.Int(1, 4);
			switch (messageid) {
			case 1:
				messageid = WONT_SELL_RACE1;
				break;
			case 2:
				messageid = WONT_SELL_RACE2;
				break;
			case 3:
				messageid = WONT_SELL_RACE3;
				break;
			case 4:
				messageid = WONT_SELL_RACE4;
				break;
			default: // w/e should never happen
				messageid = WONT_SELL_RACE1;
				break;
			}
			merchant->SayString(messageid, itoa(GetRace()));
		}
	} else if (lowestvalue == fmod.class_mod) {
		merchant->SayString(zone->random.Int(WONT_SELL_CLASS1, WONT_SELL_CLASS5), itoa(GetClass()));
	} else {
		// Must be deity - these two sound the best for that.
		// Can't use a message with a field, GUI wants class/race names.
		// for those message IDs.  These are straight text.
		merchant->SayString(zone->random.Int(WONT_SELL_DEEDS1, WONT_SELL_DEEDS2));
	}
	return;
}

//o--------------------------------------------------------------
//| Name: SendFactionMessage
//o--------------------------------------------------------------
//| Purpose: Send faction change message to client
//o--------------------------------------------------------------
void Client::SendFactionMessage(int32 tmpvalue, int32 faction_id, int32 faction_before_hit, int32 totalvalue, uint8 temp, int32 this_faction_min, int32 this_faction_max)
{
	char  name[50];
	int32 faction_value;

	// If we're dropping from MAX or raising from MIN or repairing,
	// we should base the message on the new updated value so we don't show
	// a min MAX message
	//
	// If we're changing any other place, we use the value before the
	// hit.  For example, if we go from 1199 to 1200 which is the MAX
	// we still want to say faction got better this time around.

	if (!EQ::ValueWithin(faction_before_hit, this_faction_min, this_faction_max)) {
		faction_value = totalvalue;
	} else {
		faction_value = faction_before_hit;
	}

	// default to Faction# if we couldn't get the name from the ID
	if (!content_db.GetFactionName(faction_id, name, sizeof(name))) {
		snprintf(name, sizeof(name), "Faction%i", faction_id);
	}

	if (tmpvalue == 0 || temp == 1 || temp == 2) {
		return;
	} else if (faction_value >= this_faction_max) {
		MessageString(Chat::Yellow, FACTION_BEST, name);
	} else if (faction_value <= this_faction_min) {
		MessageString(Chat::Yellow, FACTION_WORST, name);
	} else if (tmpvalue > 0 && !RuleB(Client, UseLiveFactionMessage)) {
		MessageString(Chat::Yellow, FACTION_BETTER, name);
	} else if (tmpvalue < 0 && !RuleB(Client, UseLiveFactionMessage)) {
		MessageString(Chat::Yellow, FACTION_WORSE, name);
	} else if (RuleB(Client, UseLiveFactionMessage)) {
		Message(
			Chat::Yellow,
			fmt::format(
				"Your faction standing with {} has been adjusted by {}.",
				name,
				tmpvalue
			).c_str()
		);
	} //New Live faction message (14261)
}

void Client::LoadAccountFlags()
{
	accountflags.clear();

	const auto& l = AccountFlagsRepository::GetWhere(database, fmt::format("p_accid = {}", account_id));
	if (l.empty()) {
		return;
	}

	for (const auto& e : l) {
		accountflags[e.p_flag] = e.p_value;
	}
}

void Client::ClearAccountFlag(const std::string& flag)
{
	auto e = AccountFlagsRepository::NewEntity();

	e.p_accid = account_id;
	e.p_flag  = flag;

	AccountFlagsRepository::ClearFlag(database, e);
}

void Client::SetAccountFlag(const std::string& flag, const std::string& value)
{
	auto e = AccountFlagsRepository::NewEntity();

	e.p_accid = account_id;
	e.p_flag  = flag;
	e.p_value = value;

	AccountFlagsRepository::ReplaceFlag(database, e);

	accountflags[flag] = value;
}

std::string Client::GetAccountFlag(const std::string& flag)
{
	return accountflags[flag];
}

std::vector<std::string> Client::GetAccountFlags()
{
	std::vector<std::string> l;

	l.reserve(accountflags.size());

	for (const auto& e : accountflags) {
		l.emplace_back(e.first);
	}

	return l;
}

void Client::ItemTimerCheck()
{
	int i;
	for (i = EQ::invslot::POSSESSIONS_BEGIN; i <= EQ::invslot::POSSESSIONS_END; i++)
	{
		TryItemTimer(i);
	}
	for (i = EQ::invbag::GENERAL_BAGS_BEGIN; i <= EQ::invbag::CURSOR_BAG_END; i++)
	{
		TryItemTimer(i);
	}
}

void Client::TryItemTimer(int slot)
{
	EQ::ItemInstance* inst = m_inv.GetItem(slot);
	if(!inst) {
		return;
	}

	auto item_timers = inst->GetTimers();
	auto it_iter = item_timers.begin();
	while(it_iter != item_timers.end()) {
		if(it_iter->second.Check()) {
			if (parse->ItemHasQuestSub(inst, EVENT_TIMER)) {
				parse->EventItem(EVENT_TIMER, this, inst, nullptr, it_iter->first, 0);
			}
		}
		++it_iter;
	}

	if (slot > EQ::invslot::EQUIPMENT_END) {
		return;
	}

	for (int x = EQ::invaug::SOCKET_BEGIN; x <= EQ::invaug::SOCKET_END; ++x)
	{
		EQ::ItemInstance * a_inst = inst->GetAugment(x);
		if(!a_inst) {
			continue;
		}

		auto& item_timers = a_inst->GetTimers();
		auto it_iter = item_timers.begin();
		while(it_iter != item_timers.end()) {
			if(it_iter->second.Check()) {
				if (parse->ItemHasQuestSub(a_inst, EVENT_TIMER)) {
					parse->EventItem(EVENT_TIMER, this, a_inst, nullptr, it_iter->first, 0);
				}
			}
			++it_iter;
		}
	}
}

void Client::SendItemScale(EQ::ItemInstance *inst) {
	int slot = m_inv.GetSlotByItemInst(inst);
	if(slot != -1) {
		inst->ScaleItem();
		SendItemPacket(slot, inst, ItemPacketCharmUpdate);
		CalcBonuses();
	}
}

void Client::AddRespawnOption(std::string option_name, uint32 zoneid, uint16 instance_id, float x, float y, float z, float heading, bool initial_selection, int8 position)
{
	//If respawn window is already open, any changes would create an inconsistency with the client
	if (IsHoveringForRespawn()) { return; }

	if (zoneid == 0)
		zoneid = zone->GetZoneID();

	//Create respawn option
	RespawnOption res_opt;
	res_opt.name = option_name;
	res_opt.zone_id = zoneid;
	res_opt.instance_id = instance_id;
	res_opt.x = x;
	res_opt.y = y;
	res_opt.z = z;
	res_opt.heading = heading;

	if (position == -1 || position >= respawn_options.size())
	{
		//No position specified, or specified beyond the end, simply append
		respawn_options.push_back(res_opt);
		//Make this option the initial selection for the window if desired
		if (initial_selection)
			initial_respawn_selection = static_cast<uint8>(respawn_options.size()) - 1;
	}
	else if (position == 0)
	{
		respawn_options.push_front(res_opt);
		if (initial_selection)
			initial_respawn_selection = 0;
	}
	else
	{
		//Insert new option between existing options
		std::list<RespawnOption>::iterator itr;
		uint8 pos = 0;
		for (itr = respawn_options.begin(); itr != respawn_options.end(); ++itr)
		{
			if (pos++ == position)
			{
				respawn_options.insert(itr,res_opt);
				//Make this option the initial selection for the window if desired
				if (initial_selection)
					initial_respawn_selection = pos;
				return;
			}
		}
	}
}

bool Client::RemoveRespawnOption(std::string option_name)
{
	//If respawn window is already open, any changes would create an inconsistency with the client
	if (IsHoveringForRespawn() || respawn_options.empty()) { return false; }

	bool had = false;
	RespawnOption* opt = nullptr;
	std::list<RespawnOption>::iterator itr;
	for (itr = respawn_options.begin(); itr != respawn_options.end(); ++itr)
	{
		opt = &(*itr);
		if (opt->name.compare(option_name) == 0)
		{
			itr = respawn_options.erase(itr);
			had = true;
			//could be more with the same name, so keep going...
		}
	}
	return had;
}

bool Client::RemoveRespawnOption(uint8 position)
{
	//If respawn window is already open, any changes would create an inconsistency with the client
	if (IsHoveringForRespawn() || respawn_options.empty()) { return false; }

	//Easy cases first...
	if (position == 0)
	{
		respawn_options.pop_front();
		return true;
	}
	else if (position == (respawn_options.size() - 1))
	{
		respawn_options.pop_back();
		return true;
	}

	std::list<RespawnOption>::iterator itr;
	uint8 pos = 0;
	for (itr = respawn_options.begin(); itr != respawn_options.end(); ++itr)
	{
		if (pos++ == position)
		{
			respawn_options.erase(itr);
			return true;
		}
	}
	return false;
}

void Client::SetHunger(int32 in_hunger)
{
	EQApplicationPacket *outapp = nullptr;
	outapp = new EQApplicationPacket(OP_Stamina, sizeof(Stamina_Struct));
	Stamina_Struct* sta = (Stamina_Struct*)outapp->pBuffer;
	sta->food = in_hunger;
	sta->water = m_pp.thirst_level > 6000 ? 6000 : m_pp.thirst_level;

	m_pp.hunger_level = in_hunger;

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SetThirst(int32 in_thirst)
{
	EQApplicationPacket *outapp = nullptr;
	outapp = new EQApplicationPacket(OP_Stamina, sizeof(Stamina_Struct));
	Stamina_Struct* sta = (Stamina_Struct*)outapp->pBuffer;
	sta->food = m_pp.hunger_level > 6000 ? 6000 : m_pp.hunger_level;
	sta->water = in_thirst;

	m_pp.thirst_level = in_thirst;

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SetIntoxication(int32 in_intoxication)
{
	m_pp.intoxication = EQ::Clamp(in_intoxication, 0, 200);
}

void Client::SetConsumption(int32 in_hunger, int32 in_thirst)
{
	EQApplicationPacket *outapp = nullptr;
	outapp = new EQApplicationPacket(OP_Stamina, sizeof(Stamina_Struct));
	Stamina_Struct* sta = (Stamina_Struct*)outapp->pBuffer;
	sta->food = in_hunger;
	sta->water = in_thirst;

	m_pp.hunger_level = in_hunger;
	m_pp.thirst_level = in_thirst;

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::Consume(const EQ::ItemData *item, uint8 type, int16 slot, bool auto_consume)
{
	if (!item)
		return;

	int increase = item->CastTime_ * 100;
	if (!auto_consume) // force feeding is half as effective
		increase /= 2;

	if (increase < 0) // wasn't food? oh well
		return;

	if (type == EQ::item::ItemTypeFood) {
		m_pp.hunger_level += increase;

		LogFood("Consuming food, points added to hunger_level: [{}] - current_hunger: [{}]", increase, m_pp.hunger_level);

		DeleteItemInInventory(slot, 1);

		if (!auto_consume) // no message if the client consumed for us
			entity_list.MessageCloseString(this, true, 50, 0, EATING_MESSAGE, GetName(), item->Name);

		LogFood("Eating from slot: [{}]", (int)slot);

	} else {
		m_pp.thirst_level += increase;

		DeleteItemInInventory(slot, 1);

		LogFood("Consuming drink, points added to thirst_level: [{}] current_thirst: [{}]", increase, m_pp.thirst_level);

		if (!auto_consume) // no message if the client consumed for us
			entity_list.MessageCloseString(this, true, 50, 0, DRINKING_MESSAGE, GetName(), item->Name);

		LogFood("Drinking from slot: [{}]", (int)slot);
	}
}

void Client::SendMarqueeMessage(uint32 type, std::string message, uint32 duration)
{
	if (!duration || !message.length()) {
		return;
	}

	EQApplicationPacket outapp(OP_Marquee, sizeof(ClientMarqueeMessage_Struct) + message.length());
	ClientMarqueeMessage_Struct* cms = (ClientMarqueeMessage_Struct*) outapp.pBuffer;

	cms->type = type;
	cms->unk04 = 10;
	cms->priority = 510;
	cms->fade_in_time = 0;
	cms->fade_out_time = 3000;
	cms->duration = duration;

	strcpy(cms->msg, message.c_str());

	QueuePacket(&outapp);
}

void Client::SendMarqueeMessage(uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, std::string message)
{
	if (!duration || !message.length()) {
		return;
	}

	EQApplicationPacket outapp(OP_Marquee, sizeof(ClientMarqueeMessage_Struct) + message.length());
	ClientMarqueeMessage_Struct* cms = (ClientMarqueeMessage_Struct*) outapp.pBuffer;

	cms->type = type;
	cms->unk04 = 10;
	cms->priority = priority;
	cms->fade_in_time = fade_in;
	cms->fade_out_time = fade_out;
	cms->duration = duration;

	strcpy(cms->msg, message.c_str());

	QueuePacket(&outapp);
}

void Client::PlayMP3(const char* fname)
{
	std::string filename = fname;
	auto outapp = new EQApplicationPacket(OP_PlayMP3, filename.length() + 1);
	PlayMP3_Struct* buf = (PlayMP3_Struct*)outapp->pBuffer;
	strncpy(buf->filename, fname, filename.length());
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::ExpeditionSay(const char *str, int ExpID) {

	std::string query = StringFormat("SELECT `player_name` FROM `cust_inst_players` "
									"WHERE `inst_id` = %i", ExpID);
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return;

	if(results.RowCount() == 0) {
		Message(Chat::Lime, "You say to the expedition, '%s'", str);
		return;
	}

	for(auto row = results.begin(); row != results.end(); ++row) {
		const char* charName = row[0];
		if(strcmp(charName, GetCleanName()) != 0) {
			worldserver.SendEmoteMessage(
				charName,
				0,
				AccountStatus::Player,
				Chat::Lime,
				fmt::format(
					"{} says to the expedition, '{}'",
					GetCleanName(),
					str
				).c_str()
			);
		}
		// ChannelList->CreateChannel(ChannelName, ChannelOwner, ChannelPassword, true, Strings::ToInt(row[3]));
	}


}

int Client::GetQuiverHaste(int delay)
{
	const EQ::ItemInstance *pi = nullptr;
	for (int r = EQ::invslot::GENERAL_BEGIN; r <= EQ::invslot::GENERAL_END; r++) {
		pi = GetInv().GetItem(r);
		if (pi && pi->IsClassBag() && pi->GetItem()->BagType == EQ::item::BagTypeQuiver &&
			pi->GetItem()->BagWR > 0)
			break;
		if (r == EQ::invslot::GENERAL_END)
			// we will get here if we don't find a valid quiver
			return 0;
	}
	return (pi->GetItem()->BagWR * 0.0025f * delay) + 1;
}

void Client::SendColoredText(uint32 color, std::string message)
{
	// arbitrary size limit
	if (message.size() > 512) // live does send this with empty strings sometimes ...
		return;
	auto outapp = new EQApplicationPacket(OP_ColoredText, sizeof(ColoredText_Struct) + message.size());
	ColoredText_Struct *cts = (ColoredText_Struct *)outapp->pBuffer;
	cts->color = color;
	strcpy(cts->msg, message.c_str());
	QueuePacket(outapp);
	safe_delete(outapp);
}


void Client::QuestReward(Mob* target, uint32 copper, uint32 silver, uint32 gold, uint32 platinum, uint32 itemid, uint32 exp, bool faction)
{

	auto outapp = new EQApplicationPacket(OP_Sound, sizeof(QuestReward_Struct));
	memset(outapp->pBuffer, 0, sizeof(QuestReward_Struct));
	QuestReward_Struct* qr = (QuestReward_Struct*)outapp->pBuffer;

	qr->mob_id = target ? target->GetID() : 0;		// Entity ID for the from mob name
	qr->target_id = GetID();			// The Client ID (this)
	qr->copper = copper;
	qr->silver = silver;
	qr->gold = gold;
	qr->platinum = platinum;
	qr->item_id[0] = itemid;
	qr->exp_reward = exp;

	if (copper > 0 || silver > 0 || gold > 0 || platinum > 0) {
		AddMoneyToPP(copper, silver, gold, platinum);
	}

	if (itemid > 0) {
		SummonItemIntoInventory(itemid, -1, 0, 0, 0, 0, 0, 0, false);
	}

	if (faction) {
		if (target && target->IsNPC() && !target->IsCharmed()) {
			int32 nfl_id = target->CastToNPC()->GetNPCFactionID();
			SetFactionLevel(CharacterID(), nfl_id, GetBaseClass(), GetBaseRace(), GetDeity(), true);
			qr->faction = target->CastToNPC()->GetPrimaryFaction();
			qr->faction_mod = 1; // Too lazy to get real value, not even used by client anyhow.
		}
	}

	if (exp > 0) {
		AddEXP(ExpSource::Quest, exp);
	}

	QueuePacket(outapp, true, Client::CLIENT_CONNECTED);
	safe_delete(outapp);
}

void Client::QuestReward(Mob* target, const QuestReward_Struct &reward, bool faction)
{
	auto outapp = new EQApplicationPacket(OP_Sound, sizeof(QuestReward_Struct));
	memset(outapp->pBuffer, 0, sizeof(QuestReward_Struct));
	QuestReward_Struct* qr = (QuestReward_Struct*)outapp->pBuffer;

	memcpy(qr, &reward, sizeof(QuestReward_Struct));

	// not set in caller because reasons
	qr->mob_id = target ? target->GetID() : 0; // Entity ID for the from mob name, tasks won't set this

	if (reward.copper > 0 || reward.silver > 0 || reward.gold > 0 || reward.platinum > 0) {
		AddMoneyToPP(reward.copper, reward.silver, reward.gold, reward.platinum);
	}

	for (int i = 0; i < QUESTREWARD_COUNT; ++i) {
		if (reward.item_id[i] > 0) {
			SummonItemIntoInventory(reward.item_id[i], -1, 0, 0, 0, 0, 0, 0, false);
		}
	}

	// only process if both are valid
	// if we don't have a target here, we want to just reward, but if there is a target, need to check charm
	if (reward.faction && reward.faction_mod && (target == nullptr || !target->IsCharmed())) {
		RewardFaction(reward.faction, reward.faction_mod);
	}

	// legacy support
	if (faction) {
		if (target && target->IsNPC() && !target->IsCharmed()) {
			int32 nfl_id = target->CastToNPC()->GetNPCFactionID();
			SetFactionLevel(CharacterID(), nfl_id, GetBaseClass(), GetBaseRace(), GetDeity(), true);
			qr->faction = target->CastToNPC()->GetPrimaryFaction();
			qr->faction_mod = 1; // Too lazy to get real value, not even used by client anyhow.
		}
	}

	if (reward.exp_reward > 0) {
		AddEXP(ExpSource::Quest, reward.exp_reward);
	}

	QueuePacket(outapp, true, Client::CLIENT_CONNECTED);
	safe_delete(outapp);
}

void Client::CashReward(uint32 copper, uint32 silver, uint32 gold, uint32 platinum)
{
	auto outapp = std::make_unique<EQApplicationPacket>(OP_CashReward, sizeof(CashReward_Struct));
	auto outbuf = reinterpret_cast<CashReward_Struct *>(outapp->pBuffer);
	outbuf->copper = copper;
	outbuf->silver = silver;
	outbuf->gold = gold;
	outbuf->platinum = platinum;

	AddMoneyToPP(copper, silver, gold, platinum);

	QueuePacket(outapp.get());
}

void Client::RewardFaction(int faction_id, int amount)
{
	SetFactionLevel2(CharacterID(), faction_id, GetClass(), GetBaseRace(), GetDeity(), amount, false);

	auto f = zone->GetFactionAssociation(faction_id);
	if (!f) {
		return;
	}

	std::vector<int> faction_ids = {
		f->id_1,
		f->id_2,
		f->id_3,
		f->id_4,
		f->id_5,
		f->id_6,
		f->id_7,
		f->id_8,
		f->id_9,
		f->id_10
	};

	std::vector<float> faction_modifiers = {
		f->mod_1,
		f->mod_2,
		f->mod_3,
		f->mod_4,
		f->mod_5,
		f->mod_6,
		f->mod_7,
		f->mod_8,
		f->mod_9,
		f->mod_10
	};

	std::vector<float> temporary_values = {
		static_cast<float>(faction_modifiers[0] * amount),
		static_cast<float>(faction_modifiers[1] * amount),
		static_cast<float>(faction_modifiers[2] * amount),
		static_cast<float>(faction_modifiers[3] * amount),
		static_cast<float>(faction_modifiers[4] * amount),
		static_cast<float>(faction_modifiers[5] * amount),
		static_cast<float>(faction_modifiers[6] * amount),
		static_cast<float>(faction_modifiers[7] * amount),
		static_cast<float>(faction_modifiers[8] * amount),
		static_cast<float>(faction_modifiers[9] * amount)
	};

	std::vector<int> signs = {
		temporary_values[0] < 0.0f ? -1 : 1,
		temporary_values[1] < 0.0f ? -1 : 1,
		temporary_values[2] < 0.0f ? -1 : 1,
		temporary_values[3] < 0.0f ? -1 : 1,
		temporary_values[4] < 0.0f ? -1 : 1,
		temporary_values[5] < 0.0f ? -1 : 1,
		temporary_values[6] < 0.0f ? -1 : 1,
		temporary_values[7] < 0.0f ? -1 : 1,
		temporary_values[8] < 0.0f ? -1 : 1,
		temporary_values[9] < 0.0f ? -1 : 1
	};

	std::vector<int> new_values = {
		std::max(1, static_cast<int>(std::abs(temporary_values[0]))) * signs[0],
		std::max(1, static_cast<int>(std::abs(temporary_values[1]))) * signs[1],
		std::max(1, static_cast<int>(std::abs(temporary_values[2]))) * signs[2],
		std::max(1, static_cast<int>(std::abs(temporary_values[3]))) * signs[3],
		std::max(1, static_cast<int>(std::abs(temporary_values[4]))) * signs[4],
		std::max(1, static_cast<int>(std::abs(temporary_values[5]))) * signs[5],
		std::max(1, static_cast<int>(std::abs(temporary_values[6]))) * signs[6],
		std::max(1, static_cast<int>(std::abs(temporary_values[7]))) * signs[7],
		std::max(1, static_cast<int>(std::abs(temporary_values[8]))) * signs[8],
		std::max(1, static_cast<int>(std::abs(temporary_values[9]))) * signs[9]
	};

	for (uint16 slot_id = 0; slot_id < faction_ids.size(); slot_id++) {
		if (faction_ids[slot_id] > 0) {
			SetFactionLevel2(
				CharacterID(),
				faction_ids[slot_id],
				GetClass(),
				GetBaseRace(),
				GetDeity(),
				new_values[slot_id],
				false
			);
		}
	}
}

void Client::SendHPUpdateMarquee(){
	if (!IsClient() || !current_hp || !max_hp) {
		return;
	}

	/* Health Update Marquee Display: Custom*/
	const auto health_percentage = static_cast<uint8>(current_hp * 100 / max_hp);
	if (health_percentage >= 100) {
		return;
	}

	const auto health_update_notification = fmt::format("Health: {}%%", health_percentage);
	SendMarqueeMessage(Chat::Yellow, 510, 0, 3000, 3000, health_update_notification);
}

uint32 Client::GetMoney(uint8 type, uint8 subtype) {
	uint32 value = 0;

	switch (type) {
		case MoneyTypes::Copper: {
			switch (subtype) {
				case MoneySubtypes::Personal:
					value = static_cast<uint32>(m_pp.copper);
					break;
				case MoneySubtypes::Bank:
					value = static_cast<uint32>(m_pp.copper_bank);
					break;
				case MoneySubtypes::Cursor:
					value = static_cast<uint32>(m_pp.copper_cursor);
					break;
				default:
					break;
			}
			break;
		}
		case MoneyTypes::Silver: {
			switch (subtype) {
				case MoneySubtypes::Personal:
					value = static_cast<uint32>(m_pp.silver);
					break;
				case MoneySubtypes::Bank:
					value = static_cast<uint32>(m_pp.silver_bank);
					break;
				case MoneySubtypes::Cursor:
					value = static_cast<uint32>(m_pp.silver_cursor);
					break;
				default:
					break;
			}
			break;
		}
		case MoneyTypes::Gold: {
			switch (subtype) {
				case MoneySubtypes::Personal:
					value = static_cast<uint32>(m_pp.gold);
					break;
				case MoneySubtypes::Bank:
					value = static_cast<uint32>(m_pp.gold_bank);
					break;
				case MoneySubtypes::Cursor:
					value = static_cast<uint32>(m_pp.gold_cursor);
					break;
				default:
					break;
			}
			break;
		}
		case MoneyTypes::Platinum: {
			switch (subtype) {
				case MoneySubtypes::Personal:
					value = static_cast<uint32>(m_pp.platinum);
					break;
				case MoneySubtypes::Bank:
					value = static_cast<uint32>(m_pp.platinum_bank);
					break;
				case MoneySubtypes::Cursor:
					value = static_cast<uint32>(m_pp.platinum_cursor);
					break;
				case MoneySubtypes::SharedBank:
					value = static_cast<uint32>(m_pp.platinum_shared);
					break;
				default:
					break;
			}
			break;
		}
		default:
			break;
	}

	return value;
}

int Client::GetAccountAge() {
	return (time(nullptr) - GetAccountCreation());
}

void Client::CheckRegionTypeChanges()
{
	if (!zone->HasWaterMap()) {
		return;
	}

	auto new_region = zone->watermap->ReturnRegionType(glm::vec3(m_Position));

	// still same region, do nothing
	if (last_region_type == new_region) {
		return;
	}

	// If we got out of water clear any water aggro for water only npcs
	if (last_region_type == RegionTypeWater) {
		entity_list.ClearWaterAggro(this);
	}

	// region type changed
	last_region_type = new_region;

	// PVP is the only state we need to keep track of, so we can just return now for PVP servers
	if (RuleI(World, PVPSettings) > 0) {
		return;
	}

	if (last_region_type == RegionTypePVP && RuleB(World, EnablePVPRegions)) {
		temp_pvp = true;
	} else if (temp_pvp) {
		temp_pvp = false;
	}
}

void Client::ProcessAggroMeter()
{
	if (!AggroMeterAvailable()) {
		aggro_meter_timer.Disable();
		return;
	}

	// we need to decide if we need to send OP_AggroMeterTargetInfo now
	// This packet sends the current lock target ID and the current target ID
	// target ID will be either our target or our target of target when we're targeting a PC
	bool send_targetinfo = false;
	auto cur_tar = GetTarget();

	// probably should have PVP rules ...
	if (cur_tar && cur_tar != this) {
		if (cur_tar->IsNPC() && !cur_tar->IsPetOwnerOfClientBot() && cur_tar->GetID() != m_aggrometer.get_target_id()) {
			m_aggrometer.set_target_id(cur_tar->GetID());
			send_targetinfo = true;
		} else if ((cur_tar->IsPetOwnerOfClientBot() || cur_tar->IsClient()) && cur_tar->GetTarget() && cur_tar->GetTarget()->GetID() != m_aggrometer.get_target_id()) {
			m_aggrometer.set_target_id(cur_tar->GetTarget()->GetID());
			send_targetinfo = true;
		}
	} else if (m_aggrometer.get_target_id()) {
		m_aggrometer.set_target_id(0);
		send_targetinfo = true;
	}

	if (m_aggrometer.update_lock())
		send_targetinfo = true;

	if (send_targetinfo) {
		auto app = new EQApplicationPacket(OP_AggroMeterTargetInfo, sizeof(uint32) * 2);
		app->WriteUInt32(m_aggrometer.get_lock_id());
		app->WriteUInt32(m_aggrometer.get_target_id());
		FastQueuePacket(&app);
	}

	// we could just calculate how big the packet would need to be ... but it's easier this way :P should be 87 bytes
	auto app = new EQApplicationPacket(OP_AggroMeterUpdate, m_aggrometer.max_packet_size());

	cur_tar = entity_list.GetMob(m_aggrometer.get_target_id());

	// first we must check the secondary
	// TODO: lock target should affect secondary as well
	bool send = false;
	Mob *secondary = nullptr;
	bool has_aggro = false;
	if (cur_tar) {
		if (cur_tar->GetTarget() == this) {// we got aggro
			secondary = cur_tar->GetSecondaryHate(this);
			has_aggro = true;
		} else {
			secondary = cur_tar->CheckAggro(cur_tar->GetTarget()) ? cur_tar->GetTarget() : nullptr; // make sure they are targeting for aggro reasons
		}
	}

	if (secondary && secondary->GetID() != m_aggrometer.get_secondary_id()) {
		m_aggrometer.set_secondary_id(secondary->GetID());
		app->WriteUInt8(1);
		app->WriteUInt32(m_aggrometer.get_secondary_id());
		send = true;
	} else if (!secondary && m_aggrometer.get_secondary_id()) {
		m_aggrometer.set_secondary_id(0);
		app->WriteUInt8(1);
		app->WriteUInt32(0);
		send = true;
	} else { // might not need to send in this case
		app->WriteUInt8(0);
	}

	auto count_offset = app->GetWritePosition();
	app->WriteUInt8(0);

	int count = 0;
	auto add_entry = [&app, &count, this](AggroMeter::AggroTypes i) {
		count++;
		app->WriteUInt8(i);
		app->WriteUInt16(m_aggrometer.get_pct(i));
	};
	// TODO: Player entry should either be lock or yourself, ignoring lock for now
	// player, secondary, and group depend on your target/lock
	if (cur_tar) {
		if (m_aggrometer.set_pct(AggroMeter::AT_Player, cur_tar->GetHateRatio(cur_tar->GetTarget(), this)))
			add_entry(AggroMeter::AT_Player);

		if (m_aggrometer.set_pct(AggroMeter::AT_Secondary, has_aggro ? cur_tar->GetHateRatio(this, secondary) : secondary ? 100 : 0))
			add_entry(AggroMeter::AT_Secondary);

		if (IsRaidGrouped()) {
			auto raid = GetRaid();
			if (raid) {
				auto gid = raid->GetGroup(this);
				if (gid < MAX_RAID_GROUPS) {
					int at_id = AggroMeter::AT_Group1;
					for (const auto& m : raid->members) {
						if (m.member && m.member != this && m.group_number == gid) {
							if (m_aggrometer.set_pct(static_cast<AggroMeter::AggroTypes>(at_id), cur_tar->GetHateRatio(cur_tar->GetTarget(), m.member)))
								add_entry(static_cast<AggroMeter::AggroTypes>(at_id));
							at_id++;
							if (at_id > AggroMeter::AT_Group5)
								break;
						}
					}
				}
			}
		} else if (IsGrouped()) {
			auto group = GetGroup();
			if (group) {
				int at_id = AggroMeter::AT_Group1;
				for (int i = 0; i < MAX_GROUP_MEMBERS; ++i) {
					if (group->members[i] && group->members[i] != this) {
						if (m_aggrometer.set_pct(static_cast<AggroMeter::AggroTypes>(at_id), cur_tar->GetHateRatio(cur_tar->GetTarget(), group->members[i])))
							add_entry(static_cast<AggroMeter::AggroTypes>(at_id));
						at_id++;
					}
				}
			}
		}
	} else { // we might need to clear out some data now
		if (m_aggrometer.set_pct(AggroMeter::AT_Player, 0))
			add_entry(AggroMeter::AT_Player);
		if (m_aggrometer.set_pct(AggroMeter::AT_Secondary, 0))
			add_entry(AggroMeter::AT_Secondary);
		if (m_aggrometer.set_pct(AggroMeter::AT_Group1, 0))
			add_entry(AggroMeter::AT_Group1);
		if (m_aggrometer.set_pct(AggroMeter::AT_Group2, 0))
			add_entry(AggroMeter::AT_Group2);
		if (m_aggrometer.set_pct(AggroMeter::AT_Group3, 0))
			add_entry(AggroMeter::AT_Group3);
		if (m_aggrometer.set_pct(AggroMeter::AT_Group4, 0))
			add_entry(AggroMeter::AT_Group4);
		if (m_aggrometer.set_pct(AggroMeter::AT_Group5, 0))
			add_entry(AggroMeter::AT_Group5);
	}

	// now to go over our xtargets
	// if the entry is an NPC it's our hate relative to the NPCs current tank
	// if it's a PC, it's their hate relative to our current target
	for (int i = 0; i < GetMaxXTargets(); ++i) {
		if (XTargets[i].ID) {
			auto mob = entity_list.GetMob(XTargets[i].ID);
			if (mob) {
				int ratio = 0;
				if (mob->IsNPC())
					ratio = mob->GetHateRatio(mob->GetTarget(), this);
				else if (cur_tar)
					ratio = cur_tar->GetHateRatio(cur_tar->GetTarget(), mob);
				if (m_aggrometer.set_pct(static_cast<AggroMeter::AggroTypes>(AggroMeter::AT_XTarget1 + i), ratio))
					add_entry(static_cast<AggroMeter::AggroTypes>(AggroMeter::AT_XTarget1 + i));
			}
		}
	}

	if (send || count) {
		app->size = app->GetWritePosition(); // this should be safe, although not recommended
		// but this way we can have a smaller buffer created for the packet dispatched to the client w/o resizing this one
		app->SetWritePosition(count_offset);
		app->WriteUInt8(count);
		FastQueuePacket(&app);
	} else {
		safe_delete(app);
	}
}

void Client::SetPetCommandState(uint8 button, uint8 state)
{
	auto app = new EQApplicationPacket(OP_PetCommandState, sizeof(PetCommandState_Struct));

	auto s = (PetCommandState_Struct*) app->pBuffer;

	s->button_id = button;
	s->state     = state;

	FastQueuePacket(&app);
}

bool Client::CanMedOnHorse()
{
	// no horse is false
	if (GetHorseId() == 0)
		return false;

	// can't med while attacking
	if (auto_attack)
		return false;

	return animation == 0 && m_Delta.x == 0.0f && m_Delta.y == 0.0f; // TODO: animation is SpeedRun
}

void Client::EnableAreaHPRegen(int value)
{
	AreaHPRegen = value * 0.001f;
	SendAppearancePacket(AppearanceType::AreaHealthRegen, value, false);
}

void Client::DisableAreaHPRegen()
{
	AreaHPRegen = 1.0f;
	SendAppearancePacket(AppearanceType::AreaHealthRegen, 1000, false);
}

void Client::EnableAreaManaRegen(int value)
{
	AreaManaRegen = value * 0.001f;
	SendAppearancePacket(AppearanceType::AreaManaRegen, value, false);
}

void Client::DisableAreaManaRegen()
{
	AreaManaRegen = 1.0f;
	SendAppearancePacket(AppearanceType::AreaManaRegen, 1000, false);
}

void Client::EnableAreaEndRegen(int value)
{
	AreaEndRegen = value * 0.001f;
	SendAppearancePacket(AppearanceType::AreaEnduranceRegen, value, false);
}

void Client::DisableAreaEndRegen()
{
	AreaEndRegen = 1.0f;
	SendAppearancePacket(AppearanceType::AreaEnduranceRegen, 1000, false);
}

void Client::EnableAreaRegens(int value)
{
	EnableAreaHPRegen(value);
	EnableAreaManaRegen(value);
	EnableAreaEndRegen(value);
}

void Client::DisableAreaRegens()
{
	DisableAreaHPRegen();
	DisableAreaManaRegen();
	DisableAreaEndRegen();
}

void Client::InitInnates()
{
	// this function on the client also inits the level one innate skills (like swimming, hide, etc)
	// we won't do that here, lets just do the InnateSkills for now. Basically translation of what the client is doing
	// A lot of these we could probably have ignored because they have no known use or are 100% client side
	// but I figured just in case we'll do them all out
	//
	// The client calls this in a few places. When you remove a vision buff and in SetHeights, which is called in
	// illusions, mounts, and a bunch of other cases. All of the calls to InitInnates are wrapped in restoring regen
	// besides the call initializing the first time
	auto race   = GetRace();
	auto class_ = GetClass();

	for (int i = 0; i < InnateSkillMax; ++i) {
		m_pp.InnateSkills[i] = InnateDisabled;
	}

	m_pp.InnateSkills[InnateInspect] = InnateEnabled;
	m_pp.InnateSkills[InnateOpen] = InnateEnabled;

	if (race >= Race::Froglok2) {
		if (race == Race::Skeleton2 || race == Race::Froglok2) {
			m_pp.InnateSkills[InnateUltraVision] = InnateEnabled;
		} else {
			m_pp.InnateSkills[InnateInfravision] = InnateEnabled;
		}
	}

	switch (race) {
		case Race::Barbarian:
		case Race::HalasCitizen:
			m_pp.InnateSkills[InnateSlam] = InnateEnabled;
			break;
		case Race::Erudite:
		case Race::EruditeCitizen:
			m_pp.InnateSkills[InnateLore] = InnateEnabled;
			break;
		case Race::WoodElf:
		case Race::Fayguard:
			m_pp.InnateSkills[InnateInfravision] = InnateEnabled;
			break;
		case Race::Gnome:
		case Race::HighElf:
		case Race::Felguard:
			m_pp.InnateSkills[InnateInfravision] = InnateEnabled;
			m_pp.InnateSkills[InnateLore]        = InnateEnabled;
			break;
		case Race::Troll:
		case Race::GrobbCitizen:
			m_pp.InnateSkills[InnateRegen]       = InnateEnabled;
			m_pp.InnateSkills[InnateSlam]        = InnateEnabled;
			m_pp.InnateSkills[InnateInfravision] = InnateEnabled;
			break;
		case Race::Dwarf:
		case Race::KaladimCitizen:
			m_pp.InnateSkills[InnateInfravision] = InnateEnabled;
			break;
		case Race::Ogre:
		case Race::OggokCitizen:
			m_pp.InnateSkills[InnateInfravision] = InnateEnabled;
			m_pp.InnateSkills[InnateSlam]        = InnateEnabled;
			m_pp.InnateSkills[InnateNoBash]      = InnateEnabled;
			m_pp.InnateSkills[InnateBashDoor]    = InnateEnabled;
			break;
		case Race::Halfling:
		case Race::RivervaleCitizen:
			m_pp.InnateSkills[InnateInfravision] = InnateEnabled;
			break;
		case Race::Iksar:
			m_pp.InnateSkills[InnateRegen]       = InnateEnabled;
			m_pp.InnateSkills[InnateInfravision] = InnateEnabled;
			break;
		case Race::VahShir:
			m_pp.InnateSkills[InnateInfravision] = InnateEnabled;
			break;
		case Race::DarkElf:
		case Race::NeriakCitizen:
		case Race::ElfVampire:
		case Race::FroglokGhoul:
		case Race::Ghost:
		case Race::Ghoul:
		case Race::Skeleton:
		case Race::Vampire:
		case Race::Wisp:
		case Race::Zombie:
		case Race::Spectre:
		case Race::DwarfGhost:
		case Race::EruditeGhost:
		case Race::DragonSkeleton:
		case Race::Innoruuk:
			m_pp.InnateSkills[InnateUltraVision] = InnateEnabled;
			break;
		case Race::Human:
		case Race::FreeportGuard:
		case Race::HumanBeggar:
		case Race::HighpassCitizen:
		case Race::QeynosCitizen:
		case Race::Froglok2: // client does froglok weird, but this should work out fine
			break;
		default:
			m_pp.InnateSkills[InnateInfravision] = InnateEnabled;
			break;
	}

	switch (class_) {
		case Class::Druid:
			m_pp.InnateSkills[InnateHarmony] = InnateEnabled;
			break;
		case Class::Bard:
			m_pp.InnateSkills[InnateReveal] = InnateEnabled;
			break;
		case Class::Rogue:
			m_pp.InnateSkills[InnateSurprise] = InnateEnabled;
			m_pp.InnateSkills[InnateReveal]   = InnateEnabled;
			break;
		case Class::Ranger:
			m_pp.InnateSkills[InnateAwareness] = InnateEnabled;
			break;
		case Class::Monk:
			m_pp.InnateSkills[InnateSurprise]  = InnateEnabled;
			m_pp.InnateSkills[InnateAwareness] = InnateEnabled;
		default:
			break;
	}
}

bool Client::GetDisplayMobInfoWindow() const
{
	return display_mob_info_window;
}

void Client::SetDisplayMobInfoWindow(bool display_mob_info_window)
{
	Client::display_mob_info_window = display_mob_info_window;
}

bool Client::IsDevToolsEnabled() const
{
	return dev_tools_enabled && GetGM() && RuleB(World, EnableDevTools);
}

void Client::SetDevToolsEnabled(bool in_dev_tools_enabled)
{
	const auto dev_tools_key = fmt::format("{}-dev-tools-disabled", AccountID());

	if (in_dev_tools_enabled) {
		DataBucket::DeleteData(&database, dev_tools_key);
	} else {
		DataBucket::SetData(&database, dev_tools_key, "true");
	}

	Client::dev_tools_enabled = in_dev_tools_enabled;
}

bool Client::IsEXPEnabled() const {
	return m_exp_enabled;
}

void Client::SetEXPEnabled(bool is_exp_enabled)
{
	auto c = CharacterDataRepository::FindOne(database, CharacterID());

	c.exp_enabled = is_exp_enabled;

	auto updated = CharacterDataRepository::UpdateOne(database, c);

	if (!updated) {
		return;
	}

	m_exp_enabled = is_exp_enabled;
}

void Client::SetPrimaryWeaponOrnamentation(uint32 model_id)
{
	auto primary_item = m_inv.GetItem(EQ::invslot::slotPrimary);
	if (primary_item) {
		auto l = InventoryRepository::GetWhere(
			database,
			fmt::format(
				"`character_id` = {} AND `slot_id` = {}",
				character_id,
				EQ::invslot::slotPrimary
			)
		);

		if (l.empty()) {
			return;
		}

		auto e = l.front();

		e.ornament_idfile = model_id;

		const int updated = InventoryRepository::UpdateOne(database, e);

		if (updated) {
			primary_item->SetOrnamentationIDFile(model_id);
			SendItemPacket(EQ::invslot::slotPrimary, primary_item, ItemPacketTrade);
			WearChange(EQ::textures::weaponPrimary, model_id, 0);

			Message(Chat::Yellow, "Your primary weapon appearance has been modified.");
		}
	}
}

void Client::SetSecondaryWeaponOrnamentation(uint32 model_id)
{
	auto secondary_item = m_inv.GetItem(EQ::invslot::slotSecondary);
	if (secondary_item) {
		auto l = InventoryRepository::GetWhere(
			database,
			fmt::format(
				"`character_id` = {} AND `slot_id` = {}",
				character_id,
				EQ::invslot::slotSecondary
			)
		);

		if (l.empty()) {
			return;
		}

		auto e = l.front();

		e.ornament_idfile = model_id;

		const int updated = InventoryRepository::UpdateOne(database, e);

		if (updated) {
			secondary_item->SetOrnamentationIDFile(model_id);
			SendItemPacket(EQ::invslot::slotSecondary, secondary_item, ItemPacketTrade);
			WearChange(EQ::textures::weaponSecondary, model_id, 0);

			Message(Chat::Yellow, "Your secondary weapon appearance has been modified.");
		}
	}
}

/**
 * Used in #goto <player_name>
 *
 * @param player_name
 */
bool Client::GotoPlayer(const std::string& player_name)
{
	const auto& l = CharacterDataRepository::GetWhere(
		database,
		fmt::format(
			"name = '{}' AND last_login > (UNIX_TIMESTAMP() - 600) LIMIT 1",
			Strings::Escape(player_name)
		)
	);

	if (l.empty()) {
		return false;
	}

	const auto& e = l.front();

	if (e.zone_instance > 0 && !database.CheckInstanceExists(e.zone_instance)) {
		Message(Chat::Yellow, "Instance no longer exists...");
		return false;
	}

	if (e.zone_instance > 0) {
		database.AddClientToInstance(e.zone_instance, CharacterID());
	}

	MovePC(e.zone_id, e.zone_instance, e.x, e.y, e.z, e.heading);

	return true;
}

bool Client::GotoPlayerGroup(const std::string& player_name)
{
	if (!GetGroup()) {
		return GotoPlayer(player_name);
	}

	for (auto &m: GetGroup()->members) {
		if (m && m->IsClient()) {
			auto c = m->CastToClient();
			if (!c->GotoPlayer(player_name)) {
				return false;
			}
		}
	}

	return true;
}

bool Client::GotoPlayerRaid(const std::string& player_name)
{
	if (!GetRaid()) {
		return GotoPlayer(player_name);
	}

	for (auto &m: GetRaid()->members) {
		if (m.member && m.member->IsClient()) {
			auto c = m.member->CastToClient();
			if (!c->GotoPlayer(player_name)) {
				return false;
			}
		}
	}

	return true;
}

void Client::SendToGuildHall()
{
	std::string zone_short_name = "guildhall";
	uint32      zone_id         = ZoneID(zone_short_name.c_str());
	if (zone_id == 0) {
		return;
	}

	uint32      expiration_time         = (RuleI(Instances, GuildHallExpirationDays) * 86400);
	uint16      instance_id             = 0;
	std::string guild_hall_instance_key = fmt::format("guild-hall-instance-{}", GuildID());
	std::string instance_data           = DataBucket::GetData(&database, guild_hall_instance_key);
	if (!instance_data.empty() && Strings::ToInt(instance_data) > 0) {
		instance_id = Strings::ToInt(instance_data);
	}

	if (instance_id <= 0) {
		if (!database.GetUnusedInstanceID(instance_id)) {
			Message(Chat::Red, "Server was unable to find a free instance id.");
			return;
		}

		if (!database.CreateInstance(instance_id, zone_id, 1, expiration_time)) {
			Message(Chat::Red, "Server was unable to create a new instance.");
			return;
		}

		DataBucket::SetData(
			&database,
			guild_hall_instance_key,
			std::to_string(instance_id),
			std::to_string(expiration_time)
		);
	}

	AssignToInstance(instance_id);
	MovePC(345, instance_id, -1.00, -1.00, 3.34, 0, 1);
}

void Client::CheckVirtualZoneLines()
{
	for (auto &virtual_zone_point : zone->virtual_zone_point_list) {
		float half_width = ((float) virtual_zone_point.width / 2);

		if (
			GetX() > (virtual_zone_point.x - half_width) &&
			GetX() < (virtual_zone_point.x + half_width) &&
			GetY() > (virtual_zone_point.y - half_width) &&
			GetY() < (virtual_zone_point.y + half_width) &&
			GetZ() >= (virtual_zone_point.z - 10) &&
			GetZ() < (virtual_zone_point.z + (float) virtual_zone_point.height)
			) {

			MovePC(
				virtual_zone_point.target_zone_id,
				virtual_zone_point.target_instance,
				virtual_zone_point.target_x,
				virtual_zone_point.target_y,
				virtual_zone_point.target_z,
				virtual_zone_point.target_heading
			);

			LogZonePoints(
				"Virtual Zone Box Sending player [{}] to [{}]",
				GetCleanName(),
				ZoneLongName(virtual_zone_point.target_zone_id)
			);
		}
	}
}

void Client::ShowDevToolsMenu()
{
	std::string menu_search;
	std::string menu_show;
	std::string menu_reload_one;
	std::string menu_toggle;
	std::string window_toggle;

	/**
	 * Search entity commands
	 */
	menu_search += Saylink::Silent("#list corpses", "Corpses");
	menu_search += " | " + Saylink::Silent("#list doors", "Doors");
	menu_search += " | " + Saylink::Silent("#finditem", "Items");
	menu_search += " | " + Saylink::Silent("#list npcs", "NPC");
	menu_search += " | " + Saylink::Silent("#list objects", "Objects");
	menu_search += " | " + Saylink::Silent("#list players", "Players");
	menu_search += " | " + Saylink::Silent("#findzone", "Zones");

	/**
	 * Show
	 */
	menu_show += Saylink::Silent("#showzonepoints", "Zone Points");
	menu_show += " | " + Saylink::Silent("#showzonegloballoot", "Zone Global Loot");
	menu_show += " | " + Saylink::Silent("#show content_flags", "Content Flags");

	/**
	 * Reload
	 */
	menu_reload_one += Saylink::Silent("#reload", "Reload Menu (#reload)");

	/**
	 * Show window status
	 */
	menu_toggle = Saylink::Silent("#devtools menu enable", "Enable");
	if (IsDevToolsEnabled()) {
		menu_toggle = Saylink::Silent("#devtools menu disable", "Disable");
	}

	window_toggle = Saylink::Silent("#devtools window enable", "Enable");
	if (GetDisplayMobInfoWindow()) {
		window_toggle = Saylink::Silent("#devtools window disable", "Disable");
	}

	/**
	 * Print menu
	 */
	SendChatLineBreak();

	Message(Chat::White, "Developer Tools Menu");

	Message(
		Chat::White,
		fmt::format(
			"Show Menu | {}",
			Saylink::Silent("#dev")
		).c_str()
	);

	Message(
		Chat::White,
		fmt::format(
			"Toggle Menu | {}",
			menu_toggle
		).c_str()
	);

	Message(
		Chat::White,
		fmt::format(
			"Toggle Window | {}",
			window_toggle
		).c_str()
	);

	Message(
		Chat::White,
		fmt::format(
			"Search | {}",
			menu_search
		).c_str()
	);

	Message(
		Chat::White,
		fmt::format(
			"Show | {}",
			menu_show
		).c_str()
	);

	Message(
		Chat::White,
		fmt::format(
			"Reload | {}",
			menu_reload_one
		).c_str()
	);

	auto help_link = Saylink::Silent("#help");

	Message(
		Chat::White,
		fmt::format(
			"Note: You can search for commands with {} [Search String]",
			help_link
		).c_str()
	);

	SendChatLineBreak();

	Message(
		Chat::White,
		fmt::format(
			"Current Expansion | {} ({})",
			WorldContentService::Instance()->GetCurrentExpansionName(),
			WorldContentService::Instance()->GetCurrentExpansion()
		).c_str()
	);


	auto z = GetZoneVersionWithFallback(zone->GetZoneID(), zone->GetInstanceVersion());

	if (z) {
		Message(
			Chat::White,
			fmt::format(
				"Current Zone | [{}] ({}) version [{}] instance_id [{}] min/max expansion ({}/{}) content_flags [{}]",
				z->short_name,
				z->long_name,
				z->version,
				zone->GetInstanceID(),
				z->min_expansion,
				z->max_expansion,
				z->content_flags
			).c_str()
		);
	}

	SendChatLineBreak();
}

void Client::SendChatLineBreak(uint16 color) {
	Message(color, "------------------------------------------------");
}

void Client::SendCrossZoneMessage(
	Client* client, const std::string& character_name, uint16_t chat_type, const std::string& message)
{
	// if client is null, falls back to sending a cross zone message by name
	if (!client && !character_name.empty())
	{
		client = entity_list.GetClientByName(character_name.c_str());
	}

	if (client)
	{
		client->Message(chat_type, message.c_str());
	}
	else if (!character_name.empty() && !message.empty())
	{
		uint32_t pack_size = sizeof(CZMessage_Struct);
		auto pack = std::make_unique<ServerPacket>(ServerOP_CZMessage, pack_size);
		auto buf = reinterpret_cast<CZMessage_Struct*>(pack->pBuffer);
		uint8 update_type = CZUpdateType_Character;
		int update_identifier = 0;
		buf->update_type = update_type;
		buf->update_identifier = update_identifier;
		buf->type = chat_type;
		strn0cpy(buf->message, message.c_str(), sizeof(buf->message));
		strn0cpy(buf->client_name, character_name.c_str(), sizeof(buf->client_name));

		worldserver.SendPacket(pack.get());
	}
}

void Client::SendCrossZoneMessageString(
	Client* client, const std::string& character_name, uint16_t chat_type,
	uint32_t string_id, const std::initializer_list<std::string>& arguments)
{
	// if client is null, falls back to sending a cross zone message by name
	if (!client && !character_name.empty()) // double check client isn't in this zone
	{
		client = entity_list.GetClientByName(character_name.c_str());
	}

	if (!client && character_name.empty())
	{
		return;
	}

	SerializeBuffer argbuf;
	for (const auto& argument : arguments)
	{
		argbuf.WriteString(argument);
	}

	uint32_t args_size = static_cast<uint32_t>(argbuf.size());
	uint32_t pack_size = sizeof(CZClientMessageString_Struct) + args_size;
	auto pack = std::make_unique<ServerPacket>(ServerOP_CZClientMessageString, pack_size);
	auto buf = reinterpret_cast<CZClientMessageString_Struct*>(pack->pBuffer);
	buf->string_id = string_id;
	buf->chat_type = chat_type;
	strn0cpy(buf->client_name, character_name.c_str(), sizeof(buf->client_name));
	buf->args_size = args_size;
	if (argbuf.size() > 0)
	{
		memcpy(buf->args, argbuf.buffer(), argbuf.size());
	}

	if (client)
	{
		client->MessageString(buf);
	}
	else
	{
		worldserver.SendPacket(pack.get());
	}
}

void Client::SendDynamicZoneUpdates()
{
	// bit inefficient since each do lookups but it avoids duplicating code here
	SendDzCompassUpdate();
	SetDynamicZoneMemberStatus(DynamicZoneMemberStatus::Online);

	m_dz_lockouts = CharacterExpeditionLockoutsRepository::GetLockouts(database, CharacterID());

	// expeditions are the only dz type that keep the window updated
	if (DynamicZone* dz = GetExpedition())
	{
		dz->SendClientWindowUpdate(this);

		// live synchronizes lockouts obtained during the active expedition to
		// members once they zone into the expedition's dynamic zone instance
		if (dz->IsCurrentZoneDz())
		{
			dz->SyncCharacterLockouts(CharacterID(), m_dz_lockouts);
		}
	}

	SendDzLockoutTimers();

	// ask world for any pending invite we saved from a previous zone
	RequestPendingDzInvite();
}

DynamicZone* Client::CreateExpedition(DynamicZone& dz, bool silent)
{
	return DynamicZone::TryCreate(*this, dz, silent);
}

DynamicZone* Client::CreateExpedition(uint32 zone, uint32 version, uint32 duration, const std::string& name, uint32 min_players, uint32 max_players, bool silent)
{
	DynamicZone dz{ zone, version, duration, DynamicZoneType::Expedition };
	dz.SetName(name);
	dz.SetMinPlayers(min_players);
	dz.SetMaxPlayers(max_players);

	return DynamicZone::TryCreate(*this, dz, silent);
}

DynamicZone* Client::CreateExpeditionFromTemplate(uint32_t dz_template_id)
{
	auto it = zone->dz_template_cache.find(dz_template_id);
	if (it != zone->dz_template_cache.end())
	{
		DynamicZone dz(DynamicZoneType::Expedition);
		dz.LoadTemplate(it->second);
		return DynamicZone::TryCreate(*this, dz, false);
	}
	return nullptr;
}

void Client::CreateTaskDynamicZone(int task_id, DynamicZone& dz_request)
{
	if (task_state)
	{
		task_state->CreateTaskDynamicZone(this, task_id, dz_request);
	}
}

DynamicZone* Client::GetExpedition() const
{
	if (zone)
	{
		for (uint32_t dz_id : m_dynamic_zone_ids)
		{
			auto it = zone->dynamic_zone_cache.find(dz_id);
			if (it != zone->dynamic_zone_cache.end() && it->second->IsExpedition())
			{
				return it->second.get();
			}
		}
	}
	return nullptr;
}

uint32_t Client::GetExpeditionID() const
{
	if (const DynamicZone* dz = GetExpedition())
	{
		return dz->GetID();
	}
	return 0;
}

void Client::AddDzLockout(const DzLockout& lockout, bool update_db)
{
	// todo: support for account based lockouts like live AoC expeditions

	// if client already has this lockout, we're replacing it with the new one
	std::erase_if(m_dz_lockouts, [&](const DzLockout& l) { return l.IsSame(lockout); });

	m_dz_lockouts.push_back(lockout);

	if (update_db) // for quest api
	{
		CharacterExpeditionLockoutsRepository::InsertLockouts(database, CharacterID(), { lockout });
	}

	SendDzLockoutTimers();
}

void Client::AddDzLockout(const std::string& expedition, const std::string& event, uint32_t seconds, std::string uuid)
{
	auto lockout = DzLockout::Create(expedition, event, seconds, uuid);
	AddDzLockout(lockout, true);
}

void Client::AddDzLockoutDuration(const DzLockout& lockout, int seconds, const std::string& uuid, bool update_db)
{
	auto it = std::ranges::find_if(m_dz_lockouts, [&](const DzLockout& l) { return l.IsSame(lockout); });
	if (it != m_dz_lockouts.end())
	{
		it->AddLockoutTime(seconds);

		if (!uuid.empty())
		{
			it->SetUUID(uuid);
		}

		if (update_db)
		{
			CharacterExpeditionLockoutsRepository::InsertLockouts(database, CharacterID(), { *it });
		}

		SendDzLockoutTimers();
	}
	else if (seconds > 0) // missing lockouts inserted for reductions would be instantly expired
	{
		AddDzLockout(lockout, update_db);
	}
}

void Client::RemoveDzLockout(const std::string& expedition, const std::string& event, bool update_db)
{
	std::erase_if(m_dz_lockouts, [&](const DzLockout& l) { return l.IsSame(expedition, event); });

	if (update_db) // for quest api
	{
		CharacterExpeditionLockoutsRepository::DeleteWhere(database, fmt::format(
			"character_id = {} AND expedition_name = '{}' AND event_name = '{}'",
			CharacterID(), Strings::Escape(expedition), Strings::Escape(event)));
	}

	SendDzLockoutTimers();
}

void Client::RemoveDzLockouts(const std::string& expedition, bool update_db)
{
	if (expedition.empty())
	{
		if (update_db)
		{
			CharacterExpeditionLockoutsRepository::DeleteWhere(database, fmt::format(
				"character_id = {}", CharacterID()));
		}
		m_dz_lockouts.clear();
	}
	else
	{
		if (update_db)
		{
			CharacterExpeditionLockoutsRepository::DeleteWhere(database, fmt::format(
				"character_id = {} AND expedition_name = '{}'", CharacterID(), Strings::Escape(expedition)));
		}
		std::erase_if(m_dz_lockouts, [&](const DzLockout& l) { return l.DzName() == expedition; });
	}

	SendDzLockoutTimers();
}

const DzLockout* Client::GetDzLockout(const std::string& expedition, const std::string& event) const
{
	for (const auto& lockout : m_dz_lockouts)
	{
		if (!lockout.IsExpired() && lockout.IsSame(expedition, event))
		{
			return &lockout;
		}
	}
	return nullptr;
}

std::vector<DzLockout> Client::GetDzLockouts(const std::string& expedition)
{
	std::vector<DzLockout> lockouts;
	for (const auto& lockout : m_dz_lockouts)
	{
		if (!lockout.IsExpired() && lockout.DzName() == expedition)
		{
			lockouts.push_back(lockout);
		}
	}
	return lockouts;
}

bool Client::HasDzLockout(const std::string& expedition, const std::string& event) const
{
	return GetDzLockout(expedition, event) != nullptr;
}

void Client::SendDzLockoutTimers()
{
	std::vector<ExpeditionLockoutTimerEntry_Struct> lockout_entries;

	// client displays lockouts rounded down to nearest minute, send lockouts
	// with 60s offset added to compensate (live does this too)
	constexpr uint32_t rounding_seconds = 60;

	// erases expired lockouts while building lockout timer list
	for (auto it = m_dz_lockouts.begin(); it != m_dz_lockouts.end();)
	{
		uint32_t seconds_remaining = it->GetSecondsRemaining();
		if (seconds_remaining == 0)
		{
			it = m_dz_lockouts.erase(it);
		}
		else
		{
			ExpeditionLockoutTimerEntry_Struct lockout{};
			strn0cpy(lockout.expedition_name, it->DzName().c_str(), sizeof(lockout.expedition_name));
			lockout.seconds_remaining = seconds_remaining + rounding_seconds;
			lockout.event_type = it->IsReplay() ? DynamicZone::ReplayTimerID : DynamicZone::EventTimerID;
			strn0cpy(lockout.event_name, it->Event().c_str(), sizeof(lockout.event_name));

			lockout_entries.emplace_back(lockout);
			++it;
		}
	}

	uint32_t lockout_count = static_cast<uint32_t>(lockout_entries.size());
	uint32_t lockout_entries_size = sizeof(ExpeditionLockoutTimerEntry_Struct) * lockout_count;
	uint32_t outsize = sizeof(ExpeditionLockoutTimers_Struct) + lockout_entries_size;
	auto outapp = std::make_unique<EQApplicationPacket>(OP_DzExpeditionLockoutTimers, outsize);
	auto outbuf = reinterpret_cast<ExpeditionLockoutTimers_Struct*>(outapp->pBuffer);
	outbuf->count = lockout_count;
	if (!lockout_entries.empty())
	{
		memcpy(outbuf->timers, lockout_entries.data(), lockout_entries_size);
	}
	QueuePacket(outapp.get());
}

void Client::RequestPendingDzInvite() const
{
	ServerPacket pack(ServerOP_DzRequestInvite, static_cast<uint32_t>(sizeof(ServerCharacterID_Struct)));
	auto packbuf = reinterpret_cast<ServerCharacterID_Struct*>(pack.pBuffer);
	packbuf->char_id = CharacterID();
	worldserver.SendPacket(&pack);
}

void Client::DzListTimers()
{
	// only lists player's current replay timer lockouts, not all event lockouts
	bool found = false;
	for (const auto& lockout : m_dz_lockouts)
	{
		if (lockout.IsReplay())
		{
			found = true;
			auto time = lockout.GetTimeRemainingStrs();
			MessageString(Chat::Yellow, DZ_TIMER, time.days.c_str(), time.hours.c_str(), time.mins.c_str(), lockout.DzName().c_str());
		}
	}

	if (!found)
	{
		MessageString(Chat::Yellow, DZ_NO_TIMERS);
	}
}

void Client::SetDzRemovalTimer(bool enable_timer)
{
	uint32_t timer_ms = RuleI(DynamicZone, ClientRemovalDelayMS);

	LogDynamicZones(
		"Character [{}] instance [{}] removal timer enabled: [{}] delay (ms): [{}]",
		CharacterID(), zone ? zone->GetInstanceID() : 0, enable_timer, timer_ms
	);

	if (enable_timer)
	{
		dynamiczone_removal_timer.Start(timer_ms);
	}
	else
	{
		dynamiczone_removal_timer.Disable();
	}
}

void Client::SendDzCompassUpdate()
{
	// client may be associated with multiple dynamic zone compasses in this zone
	std::vector<DynamicZoneCompassEntry_Struct> compass_entries;

	// need to sort by local doorid in case multiple have same dz switch id (live only sends first)
	// todo: just store zone's door list ordered and ditch this
	std::vector<Doors*> switches;
	switches.reserve(entity_list.GetDoorsList().size());
	for (const auto& door_pair : entity_list.GetDoorsList())
	{
		switches.push_back(door_pair.second);
	}
	std::sort(switches.begin(), switches.end(),
		[](Doors* lhs, Doors* rhs) { return lhs->GetDoorID() < rhs->GetDoorID(); });

	for (const auto& client_dz : GetDynamicZones())
	{
		auto& compass = client_dz->GetCompassLocation();
		if (zone && zone->IsZone(compass.zone_id, 0))
		{
			DynamicZoneCompassEntry_Struct entry{};
			entry.dz_zone_id = client_dz->GetZoneID();
			entry.dz_instance_id = client_dz->GetInstanceID();
			entry.dz_type = static_cast<uint32_t>(client_dz->GetType());
			entry.x = compass.x;
			entry.y = compass.y;
			entry.z = compass.z;

			compass_entries.emplace_back(entry);
		}

		// if client has a dz with a switch id add compass to any switch locs that share it
		if (client_dz->GetSwitchID() != 0)
		{
			// live only sends one if multiple in zone have the same switch id
			auto it = std::find_if(switches.begin(), switches.end(),
				[&](const auto& eqswitch) {
					return eqswitch->GetDzSwitchID() == client_dz->GetSwitchID();
				});

			if (it != switches.end())
			{
				DynamicZoneCompassEntry_Struct entry{};
				entry.dz_zone_id = client_dz->GetZoneID();
				entry.dz_instance_id = client_dz->GetInstanceID();
				entry.dz_type = static_cast<uint32_t>(client_dz->GetType());
				entry.dz_switch_id = client_dz->GetSwitchID();
				entry.x = (*it)->GetX();
				entry.y = (*it)->GetY();
				entry.z = (*it)->GetZ();

				compass_entries.emplace_back(entry);
			}
		}
	}

	// compass set via MarkSingleCompassLocation()
	if (m_has_quest_compass)
	{
		DynamicZoneCompassEntry_Struct entry{};
		entry.dz_zone_id = 0;
		entry.dz_instance_id = 0;
		entry.dz_type = 0;
		entry.x = m_quest_compass.x;
		entry.y = m_quest_compass.y;
		entry.z = m_quest_compass.z;

		compass_entries.emplace_back(entry);
	}

	QueuePacket(CreateCompassPacket(compass_entries).get());
}

std::unique_ptr<EQApplicationPacket> Client::CreateCompassPacket(
	const std::vector<DynamicZoneCompassEntry_Struct>& compass_entries)
{
	uint32 count = static_cast<uint32_t>(compass_entries.size());
	uint32 entries_size = sizeof(DynamicZoneCompassEntry_Struct) * count;
	uint32 outsize = sizeof(DynamicZoneCompass_Struct) + entries_size;
	auto outapp = std::make_unique<EQApplicationPacket>(OP_DzCompass, outsize);
	auto outbuf = reinterpret_cast<DynamicZoneCompass_Struct*>(outapp->pBuffer);
	outbuf->count = count;
	if (!compass_entries.empty())
	{
		memcpy(outbuf->entries, compass_entries.data(), entries_size);
	}

	return outapp;
}

void Client::GoToDzSafeReturnOrBind(const DynamicZone* dynamic_zone)
{
	if (dynamic_zone)
	{
		const auto& safereturn = dynamic_zone->GetSafeReturnLocation();
		if (safereturn.zone_id != 0)
		{
			LogDynamicZonesDetail("Sending [{}] to safereturn zone [{}]", CharacterID(), safereturn.zone_id);
			MovePC(safereturn.zone_id, 0, safereturn.x, safereturn.y, safereturn.z, safereturn.heading);
			return;
		}
	}

	GoToBind();
}

void Client::AddDynamicZoneID(uint32_t dz_id)
{
	auto it = std::find_if(m_dynamic_zone_ids.begin(), m_dynamic_zone_ids.end(),
		[&](uint32_t current_dz_id) { return current_dz_id == dz_id; });

	if (it == m_dynamic_zone_ids.end())
	{
		LogDynamicZonesDetail("Adding dz [{}] to client [{}]", dz_id, GetName());
		m_dynamic_zone_ids.push_back(dz_id);
	}
}

void Client::RemoveDynamicZoneID(uint32_t dz_id)
{
	LogDynamicZonesDetail("Removing dz [{}] from client [{}]", dz_id, GetName());
	m_dynamic_zone_ids.erase(std::remove_if(m_dynamic_zone_ids.begin(), m_dynamic_zone_ids.end(),
		[&](uint32_t current_dz_id) { return current_dz_id == dz_id; }
	), m_dynamic_zone_ids.end());
}

std::vector<DynamicZone*> Client::GetDynamicZones(uint32_t zone_id, int zone_version)
{
	std::vector<DynamicZone*> client_dzs;

	for (uint32_t dz_id : m_dynamic_zone_ids)
	{
		auto dz = DynamicZone::FindDynamicZoneByID(dz_id);
		if (dz &&
		   (zone_id == 0 || dz->GetZoneID() == zone_id) &&
		   (zone_version < 0 || dz->GetZoneVersion() == zone_version))
		{
			client_dzs.emplace_back(dz);
		}
	}

	return client_dzs;
}

void Client::SetDynamicZoneMemberStatus(DynamicZoneMemberStatus status)
{
	// sets status on all associated dzs client may have. if client is online
	// inside a dz, only that dz has the "In Dynamic Zone" status set
	for (auto& dz : GetDynamicZones())
	{
		// the rule to disable this status is handled internally by the dz
		if (status == DynamicZoneMemberStatus::Online && dz->IsCurrentZoneDz())
		{
			status = DynamicZoneMemberStatus::InDynamicZone;
		}
		dz->SetMemberStatus(CharacterID(), status);
	}
}

void Client::MovePCDynamicZone(uint32 zone_id, int zone_version, bool msg_if_invalid)
{
	if (zone_id == 0)
	{
		return;
	}

	auto client_dzs = GetDynamicZones(zone_id, zone_version);

	if (client_dzs.empty())
	{
		if (msg_if_invalid)
		{
			MessageString(Chat::Red, DZ_WAY_IS_BLOCKED); // unconfirmed message
		}
	}
	else if (client_dzs.size() == 1)
	{
		auto dz = client_dzs.front();
		DynamicZoneLocation zonein = dz->GetZoneInLocation();
		ZoneMode zone_mode = dz->HasZoneInLocation() ? ZoneMode::ZoneSolicited : ZoneMode::ZoneToSafeCoords;
		MovePC(zone_id, dz->GetInstanceID(), zonein.x, zonein.y, zonein.z, zonein.heading, 0, zone_mode);
	}
	else
	{
		LogDynamicZonesDetail("Sending DzSwitchListWnd to [{}] for zone [{}] with [{}] dynamic zone(s)",
			CharacterID(), zone_id, client_dzs.size());

		// client has more than one dz for this zone, send out the switchlist window
		QueuePacket(CreateDzSwitchListPacket(client_dzs).get());
	}
}

bool Client::TryMovePCDynamicZoneSwitch(int dz_switch_id)
{
	auto client_dzs = GetDynamicZones();

	std::vector<DynamicZone*> switch_dzs;
	auto it = std::copy_if(client_dzs.begin(), client_dzs.end(), std::back_inserter(switch_dzs),
		[&](const DynamicZone* dz) { return dz->GetSwitchID() == dz_switch_id; });

	if (switch_dzs.size() == 1)
	{
		LogDynamicZonesDetail("Moving client [{}] to dz with switch id [{}]", GetName(), dz_switch_id);
		switch_dzs.front()->MovePCInto(this, true);
	}
	else if (switch_dzs.size() > 1)
	{
		QueuePacket(CreateDzSwitchListPacket(switch_dzs).get());
	}

	return !switch_dzs.empty();
}

std::unique_ptr<EQApplicationPacket> Client::CreateDzSwitchListPacket(
	const std::vector<DynamicZone*>& client_dzs)
{
	uint32 count = static_cast<uint32_t>(client_dzs.size());
	uint32 entries_size = sizeof(DynamicZoneChooseZoneEntry_Struct) * count;
	uint32 outsize = sizeof(DynamicZoneChooseZone_Struct) + entries_size;
	auto outapp = std::make_unique<EQApplicationPacket>(OP_DzChooseZone, outsize);
	auto outbuf = reinterpret_cast<DynamicZoneChooseZone_Struct*>(outapp->pBuffer);
	outbuf->count = count;
	for (int i = 0; i < client_dzs.size(); ++i)
	{
		outbuf->choices[i].dz_zone_id = client_dzs[i]->GetZoneID();
		outbuf->choices[i].dz_instance_id = client_dzs[i]->GetInstanceID();
		outbuf->choices[i].dz_type = static_cast<uint32_t>(client_dzs[i]->GetType());
		strn0cpy(outbuf->choices[i].description, client_dzs[i]->GetName().c_str(), sizeof(outbuf->choices[i].description));
		strn0cpy(outbuf->choices[i].leader_name, client_dzs[i]->GetLeaderName().c_str(), sizeof(outbuf->choices[i].leader_name));
	}
	return outapp;
}

void Client::MovePCDynamicZone(const std::string& zone_name, int zone_version, bool msg_if_invalid)
{
	auto zone_id = ZoneID(zone_name.c_str());
	MovePCDynamicZone(zone_id, zone_version, msg_if_invalid);
}

void Client::Fling(float value, float target_x, float target_y, float target_z, bool ignore_los, bool clip_through_walls, bool calculate_speed) {
	BuffFadeByEffect(SpellEffect::Levitate);
	if (CheckLosFN(target_x, target_y, target_z, 6.0f) || ignore_los) {
		auto p = new EQApplicationPacket(OP_Fling, sizeof(fling_struct));
		auto* f = (fling_struct*) p->pBuffer;

		if (!calculate_speed) {
			f->speed_z = value;
		} else {
			auto speed = 1.0f;
			const auto distance = CalculateDistance(target_x, target_y, target_z);

			auto z_diff = target_z - GetZ();
			if (z_diff != 0.0f) {
				speed += std::abs(z_diff) / 12.0f;
			}

			speed += distance / 200.0f;

			speed++;

			speed = std::abs(speed);

			f->speed_z = speed;
		}

		f->collision = clip_through_walls ? 0 : -1;
		f->travel_time = -1;
		f->unk3 = 1;
		f->disable_fall_damage = 1;
		f->new_y = target_y;
		f->new_x = target_x;
		f->new_z = target_z;
		p->priority = 6;
		FastQueuePacket(&p);
	}
}

std::vector<int> Client::GetLearnableDisciplines(uint8 min_level, uint8 max_level) {
	std::vector<int> learnable_disciplines;
	for (uint16 spell_id = 0; spell_id < SPDAT_RECORDS; ++spell_id) {
		bool learnable = true;
		if (!IsValidSpell(spell_id)) {
			continue;
		}

		if (!IsDiscipline(spell_id)) {
			continue;
		}

		if (spells[spell_id].classes[Class::Warrior] == 0) {
			continue;
		}

		if (max_level && spells[spell_id].classes[m_pp.class_ - 1] > max_level) {
			continue;
		}

		if (min_level > 1 && spells[spell_id].classes[m_pp.class_ - 1] < min_level) {
			continue;
		}

		if (spells[spell_id].skill == EQ::skills::SkillTigerClaw) {
			continue;
		}

		if (RuleB(Spells, UseCHAScribeHack) && spells[spell_id].effect_id[EFFECT_COUNT - 1] == SpellEffect::CHA) {
			continue;
		}

		if (HasDisciplineLearned(spell_id)) {
			continue;
		}

		if (RuleB(Spells, EnableSpellGlobals) && !SpellGlobalCheck(spell_id, CharacterID())) {
			learnable = false;
		} else if (RuleB(Spells, EnableSpellBuckets) && !SpellBucketCheck(spell_id, CharacterID())) {
			learnable = false;
		}

		if (learnable) {
			learnable_disciplines.push_back(spell_id);
		}
	}
	return learnable_disciplines;
}

std::vector<int> Client::GetLearnedDisciplines() {
	std::vector<int> learned_disciplines;
	for (int index = 0; index < MAX_PP_DISCIPLINES; index++) {
		if (IsValidSpell(m_pp.disciplines.values[index])) {
			learned_disciplines.push_back(m_pp.disciplines.values[index]);
		}
	}
	return learned_disciplines;
}

std::vector<int> Client::GetMemmedSpells() {
	std::vector<int> memmed_spells;
	for (int index = 0; index < EQ::spells::SPELL_GEM_COUNT; index++) {
		if (IsValidSpell(m_pp.mem_spells[index])) {
			memmed_spells.push_back(m_pp.mem_spells[index]);
		}
	}
	return memmed_spells;
}

std::vector<int> Client::GetScribeableSpells(uint8 min_level, uint8 max_level) {
	std::vector<int> scribeable_spells;
	std::unordered_map<uint32, std::vector<uint16>> spell_group_cache = LoadSpellGroupCache(min_level, max_level);

	for (uint16 spell_id = 0; spell_id < SPDAT_RECORDS; ++spell_id) {
		bool scribeable = true;
		if (!IsValidSpell(spell_id)) {
			continue;
		}

		if (IsDiscipline(spell_id)) {
			continue;
		}

		if (spells[spell_id].classes[Class::Warrior] == 0) {
			continue;
		}

		if (max_level && spells[spell_id].classes[m_pp.class_ - 1] > max_level) {
			continue;
		}

		if (min_level > 1 && spells[spell_id].classes[m_pp.class_ - 1] < min_level) {
			continue;
		}

		if (spells[spell_id].skill == EQ::skills::SkillTigerClaw) {
			continue;
		}

		if (RuleB(Spells, UseCHAScribeHack) && spells[spell_id].effect_id[EFFECT_COUNT - 1] == SpellEffect::CHA) {
			continue;
		}

		if (HasSpellScribed(spell_id)) {
			continue;
		}

		if (
			RuleB(Spells, EnableSpellGlobals) &&
			!SpellGlobalCheck(spell_id, CharacterID())
		) {
			scribeable = false;
		} else if (
			RuleB(Spells, EnableSpellBuckets) &&
			!SpellBucketCheck(spell_id, CharacterID())
		) {
			scribeable = false;
		}

		if (spells[spell_id].spell_group) {
			const auto& g = spell_group_cache.find(spells[spell_id].spell_group);
			if (g != spell_group_cache.end()) {
				for (const auto& s : g->second) {
					if (
						EQ::ValueWithin(spells[s].classes[m_pp.class_ - 1], min_level, max_level) &&
						s == spell_id &&
						scribeable
					) {
						scribeable_spells.push_back(spell_id);
					}
					continue;
				}
			}
		} else if (scribeable) {
			scribeable_spells.push_back(spell_id);
		}
	}
	return scribeable_spells;
}

std::vector<int> Client::GetScribedSpells() {
	std::vector<int> scribed_spells;
	for (int index = 0; index < EQ::spells::SPELLBOOK_SIZE; index++) {
		if (IsValidSpell(m_pp.spell_book[index])) {
			scribed_spells.push_back(m_pp.spell_book[index]);
		}
	}
	return scribed_spells;
}

void Client::SetAnon(uint8 anon_flag) {
	m_pp.anon = anon_flag;
	auto outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
	SpawnAppearance_Struct* spawn_appearance = (SpawnAppearance_Struct*)outapp->pBuffer;
	spawn_appearance->spawn_id = GetID();
	spawn_appearance->type = AppearanceType::Anonymous;
	spawn_appearance->parameter = anon_flag;
	entity_list.QueueClients(this, outapp);
	Save();
	UpdateWho();
	safe_delete(outapp);
}

void Client::SetAFK(uint8 afk_flag)
{
	if (!afk_flag) {
		ResetAFKTimer();
	}

	bool changed_afk_state = (m_is_afk && !afk_flag) || (!m_is_afk && afk_flag);

	if (!changed_afk_state) {
		return;
	}

	// set messaging based on the state
	std::string you_are = "You are no longer AFK.";
	if (!m_is_afk && afk_flag) {
		you_are = "You are now AFK.";
	}

	// set the state
	m_is_afk = afk_flag;

	// inform of state change
	Message(Chat::Yellow, you_are.c_str());

	// send the spawn appearance packet to all clients
	static EQApplicationPacket p(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
	auto *s = (SpawnAppearance_Struct *) p.pBuffer;
	s->spawn_id  = GetID();
	s->type      = AppearanceType::AFK;
	s->parameter = afk_flag;
	entity_list.QueueClients(this, &p);
}

void Client::SendToInstance(std::string instance_type, std::string zone_short_name, uint32 instance_version, float x, float y, float z, float heading, std::string instance_identifier, uint32 duration) {
	uint32 zone_id = ZoneID(zone_short_name);
	std::string current_instance_type = Strings::ToLower(instance_type);
	std::string instance_type_name = "public";
	if (current_instance_type.find("solo") != std::string::npos) {
		instance_type_name = GetCleanName();
	} else if (current_instance_type.find("group") != std::string::npos) {
		uint32 group_id = (GetGroup() ? GetGroup()->GetID() : 0);
		instance_type_name = itoa(group_id);
	} else if (current_instance_type.find("raid") != std::string::npos) {
		uint32 raid_id = (GetRaid() ? GetRaid()->GetID() : 0);
		instance_type_name = itoa(raid_id);
	} else if (current_instance_type.find("guild") != std::string::npos) {
		uint32 guild_id = (GuildID() > 0 ? GuildID() : 0);
		instance_type_name = itoa(guild_id);
	}

	std::string full_bucket_name = fmt::format(
		"{}_{}_{}_{}",
		current_instance_type,
		instance_type_name,
		instance_identifier,
		zone_short_name
	);
	std::string current_bucket_value = DataBucket::GetData(&database, full_bucket_name);
	uint16 instance_id = 0;

	if (current_bucket_value.length() > 0) {
		instance_id = Strings::ToInt(current_bucket_value);
	} else {
		if(!database.GetUnusedInstanceID(instance_id)) {
			Message(Chat::White, "Server was unable to find a free instance id.");
			return;
		}

		if(!database.CreateInstance(instance_id, zone_id, instance_version, duration)) {
			Message(Chat::White, "Server was unable to create a new instance.");
			return;
		}

		DataBucket::SetData(&database, full_bucket_name, itoa(instance_id), itoa(duration));
	}

	AssignToInstance(instance_id);
	MovePC(zone_id, instance_id, x, y, z, heading);
}

uint32 Client::CountItem(uint32 item_id)
{
	uint32 quantity = 0;
	EQ::ItemInstance *item = nullptr;

	for (const int16& slot_id : GetInventorySlots()) {
		item = GetInv().GetItem(slot_id);
		if (item && item->GetID() == item_id) {
			quantity += (item->IsStackable() ? item->GetCharges() : 1);
		}
	}

	return quantity;
}

void Client::ResetItemCooldown(uint32 item_id)
{
	EQ::ItemInstance *item = nullptr;
	const EQ::ItemData* item_d = database.GetItem(item_id);
	if (!item_d) {
		return;
	}
	int recast_type = item_d->RecastType;
	bool found_item = false;

	for (const int16& slot_id : GetInventorySlots()) {
		item = GetInv().GetItem(slot_id);
		if (item) {
			item_d = item->GetItem();
			if (
				item_d &&
				item->GetID() == item_id ||
				(
					item_d->RecastType != RECAST_TYPE_UNLINKED_ITEM &&
					item_d->RecastType == recast_type
				)
			) {
				item->SetRecastTimestamp(0);
				DeleteItemRecastTimer(item_d->ID);
				SendItemPacket(slot_id, item, ItemPacketCharmUpdate);
				found_item = true;
			}
		}
	}

	if (!found_item) {
		DeleteItemRecastTimer(item_id); //We didn't find the item but we still want to remove the timer
	}
}

void Client::SetItemCooldown(uint32 item_id, bool use_saved_timer, uint32 in_seconds)
{
	EQ::ItemInstance *item = nullptr;
	const EQ::ItemData* item_d = database.GetItem(item_id);
	if (!item_d) {
		return;
	}
	int recast_type = item_d->RecastType;
	auto timestamps = database.GetItemRecastTimestamps(CharacterID());
	uint32 total_time = 0;
	uint32 current_time = static_cast<uint32>(std::time(nullptr));
	uint32 final_time = 0;
	const auto timer_type = item_d->RecastType != RECAST_TYPE_UNLINKED_ITEM ? item_d->RecastType : item_id;
	const int timer_id = recast_type != RECAST_TYPE_UNLINKED_ITEM ? (pTimerItemStart + recast_type) : (pTimerNegativeItemReuse * item_id);

	if (use_saved_timer) {
		if (item_d->RecastType != RECAST_TYPE_UNLINKED_ITEM) {
			total_time = timestamps.count(item_d->RecastType) ? timestamps.at(item_d->RecastType) : 0;
		} else {
			total_time = timestamps.count(item_id) ? timestamps.at(item_id) : 0;
		}
	} else {
		total_time = current_time + in_seconds;
	}

	if (total_time > current_time) {
		final_time = total_time - current_time;
	}

	for (const int16& slot_id : GetInventorySlots()) {
		item = GetInv().GetItem(slot_id);
		if (item) {
			item_d = item->GetItem();
			if (
				item_d &&
				item->GetID() == item_id ||
				(
					item_d->RecastType != RECAST_TYPE_UNLINKED_ITEM &&
					item_d->RecastType == recast_type
				)
			) {
				item->SetRecastTimestamp(total_time);
				SendItemPacket(slot_id, item, ItemPacketCharmUpdate);
			}
		}
	}

	//Start timers and update in database only when timer is changed
	if (!use_saved_timer) {
		GetPTimers().Clear(&database, timer_id);
		GetPTimers().Start((timer_id), in_seconds);
		database.UpdateItemRecast(
			CharacterID(),
			timer_type,
			GetPTimers().Get(timer_id)->GetReadyTimestamp()
		);
	}
	SendItemRecastTimer(recast_type, final_time, true);
}

uint32 Client::GetItemCooldown(uint32 item_id)
{
	const EQ::ItemData* item_d = database.GetItem(item_id);
	if (!item_d) {
		return 0;
	}

	int recast_type = item_d->RecastType;
	auto timestamps = database.GetItemRecastTimestamps(CharacterID());
	const auto timer_type = recast_type != RECAST_TYPE_UNLINKED_ITEM ? recast_type : item_id;
	uint32 total_time = 0;
	uint32 current_time = static_cast<uint32>(std::time(nullptr));
	uint32 final_time = 0;

	total_time = timestamps.count(timer_type) ? timestamps.at(timer_type) : 0;

	if (total_time > current_time) {
		final_time = total_time - current_time;
	}

	return final_time;
}

void Client::RemoveItem(uint32 item_id, uint32 quantity)
{
	uint32 removed_count = 0;
	EQ::ItemInstance *item = nullptr;

	for (const int16& slot_id : GetInventorySlots()) {
		if (removed_count == quantity) {
			break;
		}

		item = GetInv().GetItem(slot_id);
		if (item && item->GetID() == item_id) {
			uint32 charges    = item->IsStackable() ? item->GetCharges() : 0;
			uint32 stack_size = std::max(charges, static_cast<uint32>(1));
			if ((removed_count + stack_size) <= quantity) {
				removed_count += stack_size;
				DeleteItemInInventory(slot_id, charges, true);
			} else {
				uint32 amount_left = (quantity - removed_count);
				if (amount_left > 0 && stack_size >= amount_left) {
					removed_count += amount_left;
					DeleteItemInInventory(slot_id, amount_left, true);
				}
			}
		}
	}
}

void Client::SetGMStatus(int new_status) {
	if (Admin() != new_status) {
		database.UpdateGMStatus(AccountID(), new_status);
		UpdateAdmin();
	}
}

void Client::ApplyWeaponsStance()
{
	/*

		If you have a weapons stance bonus from at least one bonus type, each time you change weapons this function will ensure the correct
		associated buffs are applied, and previous buff is removed. If your weapon stance bonus is completely removed it will, ensure buff is
		also removed (ie, removing an item that has worn effect with weapon stance, or clicking off a buff). If client no longer has/never had
		any spells/item/aa bonuses with weapon stance effect this function will only do a simple bool check.

		Note: Live like behavior is once you have the triggered buff you can manually click it off to remove it. Swaping any items in inventory will
		reapply it automatically.

	 	Only buff spells should be used as triggered spell effect. IsBuffSpell function also checks spell id validity.
		WeaponStance bonus arrary: 0=2H Weapon 1=Shield 2=Dualweild

		Toggling ON or OFF
			- From spells, just remove the Primary buff that contains the WeaponStance effect in it.
			- For items with worn effect, unequip the item.
			- For AA abilities, a hotkey is used to Enable and Disable the effect. See. Client::TogglePassiveAlternativeAdvancement in aa.cpp for extensive details.

		Rank
			- Most important for AA, but if you have more than one of WeaponStance effect for a given type, the spell trigger buff will apply whatever has the highest
		'rank' value from the spells table. AA's on live for this effect naturally do this. Be awere of this if making custom spells/worn effects/AA.

		When creating weapon stance effects, you do not need to use all three types. For example, can make an effect where you only get a buff from equiping shield.

	*/

	if (!IsWeaponStanceEnabled()) {
		return;
	}

	bool enabled           = false;
	bool item_bonus_exists = false;
	bool aa_bonus_exists   = false;

	if (weaponstance.spellbonus_enabled) {

		if (spellbonuses.WeaponStance[WEAPON_STANCE_TYPE_2H] || spellbonuses.WeaponStance[WEAPON_STANCE_TYPE_SHIELD] ||
			spellbonuses.WeaponStance[WEAPON_STANCE_TYPE_DUAL_WIELD]) {

			enabled = true;

			// Check if no longer has correct combination of weapon type and buff, if so remove buff.
			if (!HasTwoHanderEquipped() && IsBuffSpell(spellbonuses.WeaponStance[WEAPON_STANCE_TYPE_2H]) &&
				FindBuff(spellbonuses.WeaponStance[WEAPON_STANCE_TYPE_2H])) {
				BuffFadeBySpellID(spellbonuses.WeaponStance[WEAPON_STANCE_TYPE_2H]);
			}
			else if (!HasShieldEquipped() && IsBuffSpell(spellbonuses.WeaponStance[WEAPON_STANCE_TYPE_SHIELD]) &&
					 FindBuff(spellbonuses.WeaponStance[WEAPON_STANCE_TYPE_SHIELD])) {
				BuffFadeBySpellID(spellbonuses.WeaponStance[WEAPON_STANCE_TYPE_SHIELD]);
			}
			else if (!HasDualWeaponsEquipped() &&
					 IsBuffSpell(spellbonuses.WeaponStance[WEAPON_STANCE_TYPE_DUAL_WIELD]) &&
					 FindBuff(spellbonuses.WeaponStance[WEAPON_STANCE_TYPE_DUAL_WIELD])) {
				BuffFadeBySpellID(spellbonuses.WeaponStance[WEAPON_STANCE_TYPE_DUAL_WIELD]);
			}
			// If you have correct combination of weapon type and bonus, and do not already have buff, then apply buff.
			if (HasTwoHanderEquipped() && IsBuffSpell(spellbonuses.WeaponStance[WEAPON_STANCE_TYPE_2H])) {
				if (!FindBuff(spellbonuses.WeaponStance[WEAPON_STANCE_TYPE_2H])) {
					SpellOnTarget(spellbonuses.WeaponStance[WEAPON_STANCE_TYPE_2H], this);
				}
				weaponstance.spellbonus_buff_spell_id = spellbonuses.WeaponStance[WEAPON_STANCE_TYPE_2H];
			}
			else if (HasShieldEquipped() && IsBuffSpell(spellbonuses.WeaponStance[WEAPON_STANCE_TYPE_SHIELD])) {

				if (!FindBuff(spellbonuses.WeaponStance[WEAPON_STANCE_TYPE_SHIELD])) {
					SpellOnTarget(spellbonuses.WeaponStance[WEAPON_STANCE_TYPE_SHIELD], this);
				}
				weaponstance.spellbonus_buff_spell_id = spellbonuses.WeaponStance[WEAPON_STANCE_TYPE_SHIELD];
			}
			else if (HasDualWeaponsEquipped() && IsBuffSpell(spellbonuses.WeaponStance[WEAPON_STANCE_TYPE_DUAL_WIELD])) {

				if (!FindBuff(spellbonuses.WeaponStance[WEAPON_STANCE_TYPE_DUAL_WIELD])) {
					SpellOnTarget(spellbonuses.WeaponStance[WEAPON_STANCE_TYPE_DUAL_WIELD], this);
				}
				weaponstance.spellbonus_buff_spell_id = spellbonuses.WeaponStance[WEAPON_STANCE_TYPE_DUAL_WIELD];
			}
		}
	}

	// Spellbonus effect removal is checked in BuffFadeBySlot(int slot, bool iRecalcBonuses) in spell_effects.cpp when the buff is clicked off or fades.

	if (weaponstance.itembonus_enabled) {

		if (itembonuses.WeaponStance[WEAPON_STANCE_TYPE_2H] || itembonuses.WeaponStance[WEAPON_STANCE_TYPE_SHIELD] ||
			itembonuses.WeaponStance[WEAPON_STANCE_TYPE_DUAL_WIELD]) {

			enabled           = true;
			item_bonus_exists = true;


			// Edge case check if have multiple items with WeaponStance worn effect. Make sure correct buffs are applied if items are removed but others left on.
			if (weaponstance.itembonus_buff_spell_id) {

				bool buff_desync = true;
				if (weaponstance.itembonus_buff_spell_id == itembonuses.WeaponStance[WEAPON_STANCE_TYPE_2H] ||
					weaponstance.itembonus_buff_spell_id == itembonuses.WeaponStance[WEAPON_STANCE_TYPE_SHIELD] ||
					(weaponstance.itembonus_buff_spell_id == itembonuses.WeaponStance[WEAPON_STANCE_TYPE_DUAL_WIELD])) {
					buff_desync = false;
				}

				if (buff_desync) {
					int fade_spell = weaponstance.itembonus_buff_spell_id;
					weaponstance.itembonus_buff_spell_id = 0; //Need to zero this before we fade to prevent any recursive loops.
					BuffFadeBySpellID(fade_spell);
				}
			}

			// Check if no longer has correct combination of weapon type and buff, if so remove buff.
			if (!HasTwoHanderEquipped() && IsBuffSpell(itembonuses.WeaponStance[WEAPON_STANCE_TYPE_2H]) &&
				FindBuff(itembonuses.WeaponStance[WEAPON_STANCE_TYPE_2H])) {
				BuffFadeBySpellID(itembonuses.WeaponStance[WEAPON_STANCE_TYPE_2H]);
			}
			else if (!HasShieldEquipped() && IsBuffSpell(itembonuses.WeaponStance[WEAPON_STANCE_TYPE_SHIELD]) &&
					 FindBuff(itembonuses.WeaponStance[WEAPON_STANCE_TYPE_SHIELD])) {
				BuffFadeBySpellID(itembonuses.WeaponStance[WEAPON_STANCE_TYPE_SHIELD]);
			}
			else if (!HasDualWeaponsEquipped() && IsBuffSpell(itembonuses.WeaponStance[WEAPON_STANCE_TYPE_DUAL_WIELD]) &&
					 FindBuff(itembonuses.WeaponStance[WEAPON_STANCE_TYPE_DUAL_WIELD])) {
				BuffFadeBySpellID(itembonuses.WeaponStance[WEAPON_STANCE_TYPE_DUAL_WIELD]);
			}

			// If you have correct combination of weapon type and bonus, and do not already have buff, then apply buff.
			if (HasTwoHanderEquipped() && IsBuffSpell(itembonuses.WeaponStance[WEAPON_STANCE_TYPE_2H])) {

				if (!FindBuff(itembonuses.WeaponStance[WEAPON_STANCE_TYPE_2H])) {
					SpellOnTarget(itembonuses.WeaponStance[WEAPON_STANCE_TYPE_2H], this);
				}
				weaponstance.itembonus_buff_spell_id = itembonuses.WeaponStance[WEAPON_STANCE_TYPE_2H];
			}
			else if (HasShieldEquipped() && IsBuffSpell(itembonuses.WeaponStance[WEAPON_STANCE_TYPE_SHIELD])) {

				if (!FindBuff(itembonuses.WeaponStance[WEAPON_STANCE_TYPE_SHIELD])) {
					SpellOnTarget(itembonuses.WeaponStance[WEAPON_STANCE_TYPE_SHIELD], this);
				}
				weaponstance.itembonus_buff_spell_id = itembonuses.WeaponStance[WEAPON_STANCE_TYPE_SHIELD];
			}
			else if (HasDualWeaponsEquipped() && IsBuffSpell(itembonuses.WeaponStance[WEAPON_STANCE_TYPE_DUAL_WIELD])) {
				if (!FindBuff(itembonuses.WeaponStance[WEAPON_STANCE_TYPE_DUAL_WIELD])) {
					SpellOnTarget(itembonuses.WeaponStance[WEAPON_STANCE_TYPE_DUAL_WIELD], this);
				}
				weaponstance.itembonus_buff_spell_id = itembonuses.WeaponStance[WEAPON_STANCE_TYPE_DUAL_WIELD];
			}
		}
	}

	// Itembonus effect removal when item is removed
	if (!item_bonus_exists && weaponstance.itembonus_enabled) {
		weaponstance.itembonus_enabled = false;

		if (weaponstance.itembonus_buff_spell_id) {
			BuffFadeBySpellID(weaponstance.itembonus_buff_spell_id);
			weaponstance.itembonus_buff_spell_id = 0;
		}
	}

	if (weaponstance.aabonus_enabled) {

		if (aabonuses.WeaponStance[WEAPON_STANCE_TYPE_2H] || aabonuses.WeaponStance[WEAPON_STANCE_TYPE_SHIELD] ||
			aabonuses.WeaponStance[WEAPON_STANCE_TYPE_DUAL_WIELD]) {

			enabled         = true;
			aa_bonus_exists = true;

			//Check if no longer has correct combination of weapon type and buff, if so remove buff.
			if (!HasTwoHanderEquipped() && IsBuffSpell(aabonuses.WeaponStance[WEAPON_STANCE_TYPE_2H]) &&
				FindBuff(aabonuses.WeaponStance[WEAPON_STANCE_TYPE_2H])) {
				BuffFadeBySpellID(aabonuses.WeaponStance[WEAPON_STANCE_TYPE_2H]);
			}

			else if (!HasShieldEquipped() && IsBuffSpell(aabonuses.WeaponStance[WEAPON_STANCE_TYPE_SHIELD]) &&
					 FindBuff(aabonuses.WeaponStance[WEAPON_STANCE_TYPE_SHIELD])) {
				BuffFadeBySpellID(aabonuses.WeaponStance[WEAPON_STANCE_TYPE_SHIELD]);
			}

			else if (!HasDualWeaponsEquipped() && IsBuffSpell(aabonuses.WeaponStance[WEAPON_STANCE_TYPE_DUAL_WIELD]) &&
					 FindBuff(aabonuses.WeaponStance[WEAPON_STANCE_TYPE_DUAL_WIELD])) {
				BuffFadeBySpellID(aabonuses.WeaponStance[WEAPON_STANCE_TYPE_DUAL_WIELD]);
			}

			//If you have correct combination of weapon type and bonus, and do not already have buff, then apply buff.
			if (HasTwoHanderEquipped() && IsBuffSpell(aabonuses.WeaponStance[WEAPON_STANCE_TYPE_2H])) {
				if (!FindBuff(aabonuses.WeaponStance[WEAPON_STANCE_TYPE_2H])) {
					SpellOnTarget(aabonuses.WeaponStance[WEAPON_STANCE_TYPE_2H], this);
				}
				weaponstance.aabonus_buff_spell_id = aabonuses.WeaponStance[WEAPON_STANCE_TYPE_2H];
			}

			else if (HasShieldEquipped() && IsBuffSpell(aabonuses.WeaponStance[WEAPON_STANCE_TYPE_SHIELD])) {
				if (!FindBuff(aabonuses.WeaponStance[WEAPON_STANCE_TYPE_SHIELD])) {
					SpellOnTarget(aabonuses.WeaponStance[WEAPON_STANCE_TYPE_SHIELD], this);
				}
				weaponstance.aabonus_buff_spell_id = aabonuses.WeaponStance[WEAPON_STANCE_TYPE_SHIELD];
			}

			else if (HasDualWeaponsEquipped() && IsBuffSpell(aabonuses.WeaponStance[WEAPON_STANCE_TYPE_DUAL_WIELD])) {

				if (!FindBuff(aabonuses.WeaponStance[WEAPON_STANCE_TYPE_DUAL_WIELD])) {
					SpellOnTarget(aabonuses.WeaponStance[WEAPON_STANCE_TYPE_DUAL_WIELD], this);
				}
				weaponstance.aabonus_buff_spell_id = aabonuses.WeaponStance[WEAPON_STANCE_TYPE_DUAL_WIELD];
			}
		}
	}

	// AA bonus removal is checked in TogglePassiveAA in aa.cpp. when the hot key is toggled.

	// If no bonuses remain present, prevent additional future checks until new bonus is applied.
	if (!enabled) {
		SetWeaponStanceEnabled(false);
		weaponstance.aabonus_enabled = false;
		weaponstance.itembonus_enabled = false;
		weaponstance.spellbonus_enabled = false;
	}
}

uint16 Client::GetDoorToolEntityId() const
{
	return m_door_tool_entity_id;
}

void Client::SetDoorToolEntityId(uint16 door_tool_entity_id)
{
	Client::m_door_tool_entity_id = door_tool_entity_id;
}

uint16 Client::GetObjectToolEntityId() const
{
	return m_object_tool_entity_id;
}

void Client::SetObjectToolEntityId(uint16 object_tool_entity_id)
{
	Client::m_object_tool_entity_id = object_tool_entity_id;
}

int Client::GetIPExemption()
{
	return database.GetIPExemption(GetIPString());
}

std::string Client::GetIPString()
{
	in_addr client_ip{};
	client_ip.s_addr = GetIP();
	return inet_ntoa(client_ip);
}

void Client::SetIPExemption(int exemption_amount)
{
	database.SetIPExemption(GetIPString(), exemption_amount);
}

void Client::ReadBookByName(std::string book_name, uint8 book_type)
{
	auto b = content_db.GetBook(book_name);

	if (!b.text.empty()) {
		LogDebug("Book Name: [{}] Text: [{}]", book_name, b.text);

		auto outapp = new EQApplicationPacket(OP_ReadBook, b.text.size() + sizeof(BookText_Struct));

		auto o = (BookText_Struct *) outapp->pBuffer;

		o->window  = std::numeric_limits<uint8>::max();
		o->type    = book_type;
		o->invslot = 0;

		memcpy(o->booktext, b.text.c_str(), b.text.size());

		if (EQ::ValueWithin(b.language, Language::CommonTongue, Language::Unknown27)) {
			if (m_pp.languages[b.language] < Language::MaxValue) {
				GarbleMessage(o->booktext, (Language::MaxValue - m_pp.languages[b.language]));
			}
		}

		QueuePacket(outapp);
		safe_delete(outapp);
	}
}

// this will fetch raid clients if exists
// fallback to group if raid doesn't exist
// fallback to self if group doesn't exist
std::vector<Client *> Client::GetPartyMembers()
{
	// get clients to update
	std::vector<Client *> clients_to_update = {};

	// raid
	if (const auto raid = entity_list.GetRaidByClient(this)) {
		for (auto &m : raid->members) {
			if (m.is_bot) {
				continue;
			}

			if (m.member && m.member->IsClient()) {
				clients_to_update.push_back(m.member->CastToClient());
			}
		}
	}

	// group
	if (clients_to_update.empty()) {
		Group *group = entity_list.GetGroupByClient(this);
		if (group) {
			for (auto &m : group->members) {
				if (m && m->IsClient()) {
					clients_to_update.push_back(m->CastToClient());
				}
			}
		}
	}

	// solo
	if (clients_to_update.empty()) {
		clients_to_update.push_back(this);
	}

	return clients_to_update;
}

void Client::SummonBaggedItems(uint32 bag_item_id, const std::vector<LootItem>& bag_items)
{
	if (bag_items.empty())
	{
		return;
	}

	// todo: maybe some common functions for SpellEffect::SummonItem and SpellEffect::SummonItemIntoBag

	const EQ::ItemData* bag_item = database.GetItem(bag_item_id);
	if (!bag_item)
	{
		Message(Chat::Red, fmt::format("Unable to summon item [{}]. Item not found.", bag_item_id).c_str());
		return;
	}

	if (CheckLoreConflict(bag_item))
	{
		DuplicateLoreMessage(bag_item_id);
		return;
	}

	int bag_item_charges = 1; // just summoning a single bag
	EQ::ItemInstance* summoned_bag = database.CreateItem(bag_item_id, bag_item_charges);
	if (!summoned_bag || !summoned_bag->IsClassBag())
	{
		Message(Chat::Red, fmt::format("Failed to summon bag item [{}]", bag_item_id).c_str());
		safe_delete(summoned_bag);
		return;
	}

	for (const auto& item : bag_items)
	{
		uint8 open_slot = summoned_bag->FirstOpenSlot();
		if (open_slot == 0xff)
		{
			Message(Chat::Red, "Attempting to summon item in to bag, but there is no room in the summoned bag!");
			break;
		}

		const EQ::ItemData* current_item = database.GetItem(item.item_id);

		if (CheckLoreConflict(current_item))
		{
			DuplicateLoreMessage(item.item_id);
		}
		else
		{
			EQ::ItemInstance* summoned_bag_item = database.CreateItem(
				item.item_id,
				item.charges,
				item.aug_1,
				item.aug_2,
				item.aug_3,
				item.aug_4,
				item.aug_5,
				item.aug_6,
				item.attuned,
				item.custom_data,
				item.ornamenticon,
				item.ornamentidfile,
				item.ornament_hero_model
			);
			if (summoned_bag_item)
			{
				summoned_bag->PutItem(open_slot, *summoned_bag_item);
				safe_delete(summoned_bag_item);
			}
		}
	}

	PushItemOnCursor(*summoned_bag);
	SendItemPacket(EQ::invslot::slotCursor, summoned_bag, ItemPacketLimbo);
	safe_delete(summoned_bag);
}

void Client::SaveSpells()
{
	std::vector<CharacterSpellsRepository::CharacterSpells> character_spells = {};

	for (int index = 0; index < EQ::spells::SPELLBOOK_SIZE; index++) {
		if (IsValidSpell(m_pp.spell_book[index])) {
			auto spell = CharacterSpellsRepository::NewEntity();
			spell.id       = CharacterID();
			spell.slot_id  = index;
			spell.spell_id = m_pp.spell_book[index];
			character_spells.emplace_back(spell);
		}
	}

	CharacterSpellsRepository::DeleteWhere(database, fmt::format("id = {}", CharacterID()));

	if (!character_spells.empty()) {
		CharacterSpellsRepository::InsertMany(database, character_spells);
	}
}

void Client::SaveDisciplines()
{
	std::vector<CharacterDisciplinesRepository::CharacterDisciplines> v;

	std::vector<std::string> delete_slots;

	for (uint16 slot_id = 0; slot_id < MAX_PP_DISCIPLINES; slot_id++) {
		if (IsValidSpell(m_pp.disciplines.values[slot_id])) {
			auto e = CharacterDisciplinesRepository::NewEntity();

			e.id      = CharacterID();
			e.slot_id = slot_id;
			e.disc_id = m_pp.disciplines.values[slot_id];

			v.emplace_back(e);
		} else {
			delete_slots.emplace_back(std::to_string(slot_id));
		}
	}

	if (!delete_slots.empty()) {
		CharacterDisciplinesRepository::DeleteWhere(
			database,
			fmt::format(
				"`id` = {} AND `slot_id` IN ({})",
				CharacterID(),
				Strings::Join(delete_slots, ", ")
			)
		);
	}

	if (!v.empty()) {
		CharacterDisciplinesRepository::ReplaceMany(database, v);
	}
}

uint16 Client::ScribeSpells(uint8 min_level, uint8 max_level)
{
	auto             available_book_slot = GetNextAvailableSpellBookSlot();
	std::vector<int> spell_ids           = GetScribeableSpells(min_level, max_level);
	uint16           scribed_spells      = 0;

	if (!spell_ids.empty()) {
		for (const auto& spell_id : spell_ids) {
			if (available_book_slot == -1) {
				Message(
					Chat::Red,
					fmt::format(
						"Unable to scribe {} ({}) to Spell Book because your Spell Book is full.",
						spells[spell_id].name,
						spell_id
					).c_str()
				);
				break;
			}

			if (HasSpellScribed(spell_id)) {
				continue;
			}

			// defer saving per spell and bulk save at the end
			ScribeSpell(spell_id, available_book_slot, true, true);
			available_book_slot = GetNextAvailableSpellBookSlot(available_book_slot);
			scribed_spells++;
		}
	}

	if (scribed_spells > 0) {
		std::string spell_message = (
			scribed_spells == 1 ?
			"a new spell" :
			fmt::format("{} new spells", scribed_spells)
		);
		Message(Chat::White, fmt::format("You have learned {}!", spell_message).c_str());

		// bulk insert spells
		SaveSpells();
	}
	return scribed_spells;
}

uint16 Client::LearnDisciplines(uint8 min_level, uint8 max_level)
{
	auto             available_discipline_slot = GetNextAvailableDisciplineSlot();
	auto             character_id              = CharacterID();
	std::vector<int> spell_ids                 = GetLearnableDisciplines(min_level, max_level);
	uint16           learned_disciplines       = 0;

	if (!spell_ids.empty()) {
		for (const auto& spell_id : spell_ids) {
			if (available_discipline_slot == -1) {
				Message(
					Chat::Red,
					fmt::format(
						"Unable to learn {} ({}) because your Discipline slots are full.",
						spells[spell_id].name,
						spell_id
					).c_str()
				);
				break;
			}

			if (HasDisciplineLearned(spell_id)) {
				continue;
			}

			GetPP().disciplines.values[available_discipline_slot] = spell_id;
			available_discipline_slot = GetNextAvailableDisciplineSlot(available_discipline_slot);
			learned_disciplines++;
		}
	}

	if (learned_disciplines > 0) {
		std::string discipline_message = (
			learned_disciplines == 1 ?
			"a new discipline" :
			fmt::format("{} new disciplines", learned_disciplines)
		);
		Message(Chat::White, fmt::format("You have learned {}!", discipline_message).c_str());
		SendDisciplineUpdate();
		SaveDisciplines();
	}

	return learned_disciplines;
}

uint16 Client::GetClassTrackingDistanceMultiplier(uint16 class_) {
	switch (class_) {
	case Class::Warrior:
		return RuleI(Character, WarriorTrackingDistanceMultiplier);
	case Class::Cleric:
		return RuleI(Character, ClericTrackingDistanceMultiplier);
	case Class::Paladin:
		return RuleI(Character, PaladinTrackingDistanceMultiplier);
	case Class::Ranger:
		return RuleI(Character, RangerTrackingDistanceMultiplier);
	case Class::ShadowKnight:
		return RuleI(Character, ShadowKnightTrackingDistanceMultiplier);
	case Class::Druid:
		return RuleI(Character, DruidTrackingDistanceMultiplier);
	case Class::Monk:
		return RuleI(Character, MonkTrackingDistanceMultiplier);
	case Class::Bard:
		return RuleI(Character, BardTrackingDistanceMultiplier);
	case Class::Rogue:
		return RuleI(Character, RogueTrackingDistanceMultiplier);
	case Class::Shaman:
		return RuleI(Character, ShamanTrackingDistanceMultiplier);
	case Class::Necromancer:
		return RuleI(Character, NecromancerTrackingDistanceMultiplier);
	case Class::Wizard:
		return RuleI(Character, WizardTrackingDistanceMultiplier);
	case Class::Magician:
		return RuleI(Character, MagicianTrackingDistanceMultiplier);
	case Class::Enchanter:
		return RuleI(Character, EnchanterTrackingDistanceMultiplier);
	case Class::Beastlord:
		return RuleI(Character, BeastlordTrackingDistanceMultiplier);
	case Class::Berserker:
		return RuleI(Character, BerserkerTrackingDistanceMultiplier);
	default:
		return 0;
	}
}

bool Client::CanThisClassTrack() {
	return (GetClassTrackingDistanceMultiplier(GetClass()) > 0) ? true : false;
}

void Client::ReconnectUCS()
{
	EQApplicationPacket      *outapp         = nullptr;
	std::string              buffer;
	std::string              mail_key        = m_mail_key;
	EQ::versions::UCSVersion connection_type = EQ::versions::ucsUnknown;

	// chat server packet
	switch (ClientVersion()) {
		case EQ::versions::ClientVersion::Titanium:
			connection_type = EQ::versions::ucsTitaniumChat;
			break;
		case EQ::versions::ClientVersion::SoF:
			connection_type = EQ::versions::ucsSoFCombined;
			break;
		case EQ::versions::ClientVersion::SoD:
			connection_type = EQ::versions::ucsSoDCombined;
			break;
		case EQ::versions::ClientVersion::UF:
			connection_type = EQ::versions::ucsUFCombined;
			break;
		case EQ::versions::ClientVersion::RoF:
			connection_type = EQ::versions::ucsRoFCombined;
			break;
		case EQ::versions::ClientVersion::RoF2:
			connection_type = EQ::versions::ucsRoF2Combined;
			break;
		default:
			connection_type = EQ::versions::ucsUnknown;
			break;
	}

	buffer = StringFormat(
		"%s,%i,%s.%s,%c%s",
		Config->GetUCSHost().c_str(),
		Config->GetUCSPort(),
		Config->ShortName.c_str(),
		GetName(),
		connection_type,
		mail_key.c_str()
	);

	outapp = new EQApplicationPacket(OP_SetChatServer, (buffer.length() + 1));
	memcpy(outapp->pBuffer, buffer.c_str(), buffer.length());
	outapp->pBuffer[buffer.length()] = '\0';

	QueuePacket(outapp);
	safe_delete(outapp);

	// mail server packet
	switch (ClientVersion()) {
		case EQ::versions::ClientVersion::Titanium:
			connection_type = EQ::versions::ucsTitaniumMail;
			break;
		default:
			// retain value from previous switch
			break;
	}

	buffer = StringFormat(
		"%s,%i,%s.%s,%c%s",
		Config->GetUCSHost().c_str(),
		Config->GetUCSPort(),
		Config->ShortName.c_str(),
		GetName(),
		connection_type,
		mail_key.c_str()
	);

	outapp = new EQApplicationPacket(OP_SetChatServer2, (buffer.length() + 1));
	memcpy(outapp->pBuffer, buffer.c_str(), buffer.length());
	outapp->pBuffer[buffer.length()] = '\0';

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendReloadCommandMessages() {
	SendChatLineBreak();

	for (auto &t: ServerReload::GetTypes()) {
		std::string reload_slug = Strings::Slugify(ServerReload::GetNameClean(t), "_");;
		auto reload_link        = Saylink::Silent(fmt::format("#reload {}", reload_slug), "Local");
		auto reload_link_global = Saylink::Silent(fmt::format("#reload {} global", reload_slug), "Global");

		Message(
			Chat::White,
			fmt::format(
				"Usage: [{}] [{}] #reload {} - Reloads {}",
				reload_link,
				reload_link_global,
				reload_slug,
				ServerReload::GetName(t)
			).c_str()
		);
	}

	SendChatLineBreak();
}

void Client::Undye()
{
	for (uint8 slot = EQ::textures::textureBegin; slot <= EQ::textures::LastTexture; slot++) {
		auto inventory_slot = SlotConvert(slot);
		auto inst = m_inv.GetItem(inventory_slot);

		if (inst) {
			inst->SetColor(inst->GetItem()->Color);
			database.SaveInventory(CharacterID(), inst, inventory_slot);
		}

		m_pp.item_tint.Slot[slot].Color = 0;
		SendWearChange(slot);
	}

	database.DeleteCharacterMaterialColor(CharacterID());
}

void Client::SetTrackingID(uint32 entity_id)
{
	if (!entity_id) {
		TrackingID = 0;
		return;
	}

	auto *m = entity_list.GetMob(entity_id);
	if (!m) {
		TrackingID = 0;
		return;
	}

	TrackingID = entity_id;
}

int Client::GetRecipeMadeCount(uint32 recipe_id)
{
	auto r = CharRecipeListRepository::GetWhere(
		database,
		fmt::format("char_id = {} AND recipe_id = {}", CharacterID(), recipe_id)
	);

	if (!r.empty() && r[0].recipe_id) {
		return r[0].madecount;
	}

	return 0;
}

bool Client::HasRecipeLearned(uint32 recipe_id)
{
	auto r = CharRecipeListRepository::GetWhere(
		database,
		fmt::format("char_id = {} AND recipe_id = {}", CharacterID(), recipe_id)
	);

	if (!r.empty() && r[0].recipe_id) {
		return true;
	}

	return false;
}

bool Client::IsLockSavePosition() const
{
	return m_lock_save_position;
}

void Client::SetLockSavePosition(bool lock_save_position)
{
	Client::m_lock_save_position = lock_save_position;
}

void Client::SetAAPoints(uint32 points)
{
	const uint32 current_points = m_pp.aapoints;

	m_pp.aapoints = points;

	QuestEventID event_id = points > current_points ? EVENT_AA_GAIN : EVENT_AA_LOSS;
	const uint32 change   = event_id == EVENT_AA_GAIN ? points - current_points : current_points - points;

	if (parse->PlayerHasQuestSub(event_id)) {
		parse->EventPlayer(event_id, this, std::to_string(change), 0);
	}

	SendAlternateAdvancementStats();
}

bool Client::RemoveAAPoints(uint32 points)
{
	if (m_pp.aapoints < points) {
		return false;
	}

	m_pp.aapoints -= points;

	if (parse->PlayerHasQuestSub(EVENT_AA_LOSS)) {
		parse->EventPlayer(EVENT_AA_LOSS, this, std::to_string(points), 0);
	}

	SendAlternateAdvancementStats();

	return true;
}

void Client::AddAAPoints(uint32 points)
{
	m_pp.aapoints += points;

	if (parse->PlayerHasQuestSub(EVENT_AA_GAIN)) {
		parse->EventPlayer(EVENT_AA_GAIN, this, std::to_string(points), 0);
	}

	if (points == 1 && m_pp.aapoints == 1) {
		MessageString(Chat::Yellow, GAIN_SINGLE_AA_SINGLE_AA, fmt::format_int(m_pp.aapoints).c_str());
	} else if (points == 1 && m_pp.aapoints > 1) {
		MessageString(Chat::Yellow, GAIN_SINGLE_AA_MULTI_AA, fmt::format_int(m_pp.aapoints).c_str());
	} else {
		MessageString(Chat::Yellow, GAIN_MULTI_AA_MULTI_AA, fmt::format_int(points).c_str(), fmt::format_int(m_pp.aapoints).c_str());
	}

	SendAlternateAdvancementStats();
}

bool Client::SendGMCommand(std::string message, bool ignore_status) {
	return command_dispatch(this, message, ignore_status) >= 0 ? true : false;
}

void Client::RegisterBug(BugReport_Struct* r) {
	if (!r) {
		return;
	}

	auto b = BugReportsRepository::NewEntity();

	b.zone                = zone->GetShortName();
	b.client_version_id   = static_cast<uint32_t>(ClientVersion());
	b.client_version_name = EQ::versions::ClientVersionName(ClientVersion());
	b.account_id          = AccountID();
	b.character_id        = CharacterID();
	b.character_name      = GetName();
	b.reporter_spoof      = (strcmp(GetCleanName(), r->reporter_name) != 0 ? 1 : 0);
	b.category_id         = r->category_id;
	b.category_name       = r->category_name;
	b.reporter_name       = r->reporter_name;
	b.ui_path             = r->ui_path;
	b.pos_x               = r->pos_x;
	b.pos_y               = r->pos_y;
	b.pos_z               = r->pos_z;
	b.heading             = r->heading;
	b.time_played         = r->time_played;
	b.target_id           = r->target_id;
	b.target_name         = r->target_name;
	b.optional_info_mask  = r->optional_info_mask;
	b._can_duplicate      = ((r->optional_info_mask & Bug::InformationFlag::Repeatable) != 0 ? 1 : 0);
	b._crash_bug          = ((r->optional_info_mask & Bug::InformationFlag::Crash) != 0 ? 1 : 0);
	b._target_info        = ((r->optional_info_mask & Bug::InformationFlag::TargetInfo) != 0 ? 1 : 0);
	b._character_flags    = ((r->optional_info_mask & Bug::InformationFlag::CharacterFlags) != 0 ? 1 : 0);
	b._unknown_value      = ((r->optional_info_mask & Bug::InformationFlag::Unknown) != 0 ? 1 : 0);
	b.bug_report          = r->bug_report;
	b.system_info         = r->system_info;

#ifdef LUA_EQEMU
	bool ignore_default = false;
	LuaParser::Instance()->RegisterBug(this, b, ignore_default);
	if (ignore_default) {
		return;
	}
#endif

	auto n = BugReportsRepository::InsertOne(database, b);
	if (!n.id) {
		Message(Chat::White, "Failed to created your bug report."); // Client sends success message
		return;
	}

	LogBugs("id [{}] report [{}] account [{}] name [{}] charid [{}] zone [{}]", n.id, r->bug_report, AccountID(), GetCleanName(), CharacterID(), zone->GetShortName());

	worldserver.SendEmoteMessage(
		0,
		0,
		AccountStatus::QuestTroupe,
		Chat::Yellow,
		fmt::format(
			"{} has created a new bug report, would you like to {} it?",
			GetCleanName(),
			Saylink::Silent(
				fmt::format(
					"#bugs view {}",
					n.id
				),
				"view"
			)
		).c_str()
	);
}

std::vector<Mob*> Client::GetApplySpellList(
	ApplySpellType apply_type,
	bool allow_pets,
	bool is_raid_group_only,
	bool allow_bots
) {
	std::vector<Mob*> l;

	if (apply_type == ApplySpellType::Raid && IsRaidGrouped()) {
		auto* r = GetRaid();
		if (r) {
			auto group_id = r->GetGroup(this);
			if (EQ::ValueWithin(group_id, 0, (MAX_RAID_GROUPS - 1))) {
				for (const auto& m : r->members) {
					if (m.member && m.member->IsClient() && (!is_raid_group_only || r->GetGroup(m.member) == group_id)) {
						l.push_back(m.member);

						if (allow_pets && m.member->HasPet()) {
							l.push_back(m.member->GetPet());
						}

						if (allow_bots) {
							const auto& sbl = entity_list.GetBotListByCharacterID(m.member->CharacterID());
							for (const auto& b : sbl) {
								l.push_back(b);
							}
						}
					}
				}
			}
		}
	} else if (apply_type == ApplySpellType::Group && IsGrouped()) {
		auto* g = GetGroup();
		if (g) {
			for (auto i = 0; i < MAX_GROUP_MEMBERS; i++) {
				auto* m = g->members[i];
				if (m && m->IsClient()) {
					l.push_back(m->CastToClient());

					if (allow_pets && m->HasPet()) {
						l.push_back(m->GetPet());
					}

					if (allow_bots) {
						const auto& sbl = entity_list.GetBotListByCharacterID(m->CastToClient()->CharacterID());
						for (const auto& b : sbl) {
							l.push_back(b);
						}
					}
				}
			}
		}
	} else {
		l.push_back(this);

		if (allow_pets && HasPet()) {
			l.push_back(GetPet());
		}

		if (allow_bots) {
			const auto& sbl = entity_list.GetBotListByCharacterID(CharacterID());
			for (const auto& b : sbl) {
				l.push_back(b);
			}
		}
	}

	return l;
}

void Client::ApplySpell(
	int spell_id,
	int duration,
	int level,
	ApplySpellType apply_type,
	bool allow_pets,
	bool is_raid_group_only,
	bool allow_bots
) {
	const auto& l = GetApplySpellList(apply_type, allow_pets, is_raid_group_only, allow_bots);

	for (const auto& m : l) {
		m->ApplySpellBuff(spell_id, duration, level);
	}
}

void Client::SetSpellDuration(
	int spell_id,
	int duration,
	int level,
	ApplySpellType apply_type,
	bool allow_pets,
	bool is_raid_group_only,
	bool allow_bots
) {
	const auto& l = GetApplySpellList(apply_type, allow_pets, is_raid_group_only, allow_bots);

	for (const auto& m : l) {
		m->SetBuffDuration(spell_id, duration, level);
	}
}

std::string Client::GetGuildPublicNote()
{
	if (!IsInAGuild()) {
		return std::string();
	}

	CharGuildInfo gci;
	if (!guild_mgr.GetCharInfo(character_id, gci)) {
		return std::string();
	}

	return gci.public_note;
}

void Client::MaxSkills()
{
	for (const auto &s : EQ::skills::GetSkillTypeMap()) {
		auto current_skill_value = (
			EQ::skills::IsSpecializedSkill(s.first) ?
			MAX_SPECIALIZED_SKILL :
			SkillCaps::Instance()->GetSkillCap(GetClass(), s.first, GetLevel()).cap
		);

		if (GetSkill(s.first) < current_skill_value) {
			SetSkill(s.first, current_skill_value);
		}
	}
}

void Client::SendPath(Mob* target)
{
	if (!target) {
		EQApplicationPacket outapp(OP_FindPersonReply, 0);
		QueuePacket(&outapp);
		return;
	}


	if (
		!RuleB(Pathing, Find) &&
		RuleB(Bazaar, EnableWarpToTrader) &&
		target->IsClient() &&
		(
			target->CastToClient()->IsTrader() ||
			target->CastToClient()->IsBuyer()
		)
		) {
		Message(
			Chat::Yellow,
			fmt::format(
				"Moving you to Trader {}.",
				target->GetName()
			).c_str()
		);
		MovePC(
			zone->GetZoneID(),
			zone->GetInstanceID(),
			target->GetX(),
			target->GetY(),
			target->GetZ(),
			0.0f
		);
		return;
	}

	std::vector<FindPerson_Point> points;

	if (!RuleB(Pathing, Find) || !zone->pathing) {
		points.clear();
		FindPerson_Point a;
		FindPerson_Point b;

		a.x = GetX();
		a.y = GetY();
		a.z = GetZ();
		b.x = target->GetX();
		b.y = target->GetY();
		b.z = target->GetZ();

		points.push_back(a);
		points.push_back(b);
	}
	else {
		glm::vec3 path_start(
			GetX(),
			GetY(),
			GetZ() + (GetSize() < 6.0 ? 6 : GetSize()) * HEAD_POSITION
		);

		glm::vec3 path_end(
			target->GetX(),
			target->GetY(),
			target->GetZ() + (target->GetSize() < 6.0 ? 6 : target->GetSize()) * HEAD_POSITION
		);

		bool partial   = false;
		bool stuck     = false;
		auto path_list = zone->pathing->FindRoute(path_start, path_end, partial, stuck);

		if (path_list.empty() || partial) {
			EQApplicationPacket outapp(OP_FindPersonReply, 0);
			QueuePacket(&outapp);
			return;
		}

		// Live appears to send the points in this order:
		// Final destination.
		// Current Position.
		// rest of the points.
		FindPerson_Point p;

		int point_number = 0;

		bool leads_to_teleporter = false;

		auto v = path_list.back();

		p.x = v.pos.x;
		p.y = v.pos.y;
		p.z = v.pos.z;
		points.push_back(p);

		p.x = GetX();
		p.y = GetY();
		p.z = GetZ();
		points.push_back(p);

		for (const auto &n: path_list) {
			if (n.teleport) {
				leads_to_teleporter = true;
				break;
			}

			glm::vec3 v = n.pos;
			p.x = v.x;
			p.y = v.y;
			p.z = v.z;

			points.push_back(p);

			++point_number;
		}

		if (!leads_to_teleporter) {
			p.x = target->GetX();
			p.y = target->GetY();
			p.z = target->GetZ();

			points.push_back(p);
		}
	}

	SendPathPacket(points);
}

void Client::UseAugmentContainer(int container_slot)
{
	auto in_augment = new AugmentItem_Struct[sizeof(AugmentItem_Struct)];
	in_augment->container_slot = container_slot;
	in_augment->augment_slot   = -1;
	Object::HandleAugmentation(this, in_augment, nullptr);
	safe_delete_array(in_augment);
}

PlayerEvent::PlayerEvent Client::GetPlayerEvent()
{
	auto e = PlayerEvent::PlayerEvent{};
	e.account_id      = AccountID();
	e.character_id    = CharacterID();
	e.character_name  = GetCleanName();
	e.x               = GetX();
	e.y               = GetY();
	e.z               = GetZ();
	e.heading         = GetHeading();
	e.zone_id         = GetZoneID();
	e.zone_short_name = zone ? zone->GetShortName() : "";
	e.zone_long_name  = zone ? zone->GetLongName() : "";
	e.instance_id     = GetInstanceID();
	e.guild_id        = GuildID();
	e.guild_name      = guild_mgr.GetGuildName(GuildID());
	e.account_name    = AccountName();

	return e;
}

void Client::PlayerTradeEventLog(Trade *t, Trade *t2)
{
	Client *trader       = t->GetOwner()->CastToClient();
	Client *trader2      = t2->GetOwner()->CastToClient();
	uint8  t_item_count  = 0;
	uint8  t2_item_count = 0;

	auto money_t  = PlayerEvent::Money{
		.platinum = t->pp,
		.gold = t->gp,
		.silver = t->sp,
		.copper = t->cp,
	};
	auto money_t2 = PlayerEvent::Money{
		.platinum = t2->pp,
		.gold = t2->gp,
		.silver = t2->sp,
		.copper = t2->cp,
	};

	// trader 1 item count
	for (uint16 i = EQ::invslot::TRADE_BEGIN; i <= EQ::invslot::TRADE_END; i++) {
		if (trader->GetInv().GetItem(i)) {
			t_item_count++;
		}
	}

	// trader 2 item count
	for (uint16 i = EQ::invslot::TRADE_BEGIN; i <= EQ::invslot::TRADE_END; i++) {
		if (trader2->GetInv().GetItem(i)) {
			t2_item_count++;
		}
	}

	std::vector<PlayerEvent::TradeItemEntry> t_entries = {};
	t_entries.reserve(t_item_count);
	if (t_item_count > 0) {
		for (uint16 i = EQ::invslot::TRADE_BEGIN; i <= EQ::invslot::TRADE_END; i++) {
			const EQ::ItemInstance *inst = trader->GetInv().GetItem(i);
			if (inst) {
				t_entries.emplace_back(
					PlayerEvent::TradeItemEntry{
					.slot           = i,
					.item_id        = inst->GetItem()->ID,
					.augment_1_id   = inst->GetAugmentItemID(0),
					.augment_1_name = inst->GetAugment(0) ? inst->GetAugment(0)->GetItem()->Name : "",
					.augment_2_id   = inst->GetAugmentItemID(1),
					.augment_2_name = inst->GetAugment(1) ? inst->GetAugment(1)->GetItem()->Name : "",
					.augment_3_id   = inst->GetAugmentItemID(2),
					.augment_3_name = inst->GetAugment(2) ? inst->GetAugment(2)->GetItem()->Name : "",
					.augment_4_id   = inst->GetAugmentItemID(3),
					.augment_4_name = inst->GetAugment(3) ? inst->GetAugment(3)->GetItem()->Name : "",
					.augment_5_id   = inst->GetAugmentItemID(4),
					.augment_5_name = inst->GetAugment(4) ? inst->GetAugment(4)->GetItem()->Name : "",
					.augment_6_id   = inst->GetAugmentItemID(5),
					.augment_6_name = inst->GetAugment(5) ? inst->GetAugment(5)->GetItem()->Name : "",.item_name      = inst->GetItem()->Name,
					.charges        = static_cast<uint16>(inst->GetCharges()),
					.in_bag         = false,
					}
				);

				if (inst->IsClassBag()) {
					for (uint8 j = EQ::invbag::SLOT_BEGIN; j <= EQ::invbag::SLOT_END; j++) {
						inst = trader->GetInv().GetItem(i, j);
						if (inst) {
							t_entries.emplace_back(PlayerEvent::TradeItemEntry{
								.slot           = j,
								.item_id        = inst->GetItem()->ID,
								.augment_1_id   = inst->GetAugmentItemID(0),
								.augment_1_name = inst->GetAugment(0) ? inst->GetAugment(0)->GetItem()->Name : "",
								.augment_2_id   = inst->GetAugmentItemID(1),
								.augment_2_name = inst->GetAugment(1) ? inst->GetAugment(1)->GetItem()->Name : "",
								.augment_3_id   = inst->GetAugmentItemID(2),
								.augment_3_name = inst->GetAugment(2) ? inst->GetAugment(2)->GetItem()->Name : "",
								.augment_4_id   = inst->GetAugmentItemID(3),
								.augment_4_name = inst->GetAugment(3) ? inst->GetAugment(3)->GetItem()->Name : "",
								.augment_5_id   = inst->GetAugmentItemID(4),
								.augment_5_name = inst->GetAugment(4) ? inst->GetAugment(4)->GetItem()->Name : "",
								.augment_6_id   = inst->GetAugmentItemID(5),
								.augment_6_name = inst->GetAugment(5) ? inst->GetAugment(5)->GetItem()->Name : "",
								.item_name      = inst->GetItem()->Name,
								.charges        = static_cast<uint16>(inst->GetCharges()),
								.in_bag         = true,
							}
						);
						}
					}
				}
			}
		}
	}

	std::vector<PlayerEvent::TradeItemEntry> t2_entries = {};
	t_entries.reserve(t2_item_count);
	if (t2_item_count > 0) {
		for (uint16 i = EQ::invslot::TRADE_BEGIN; i <= EQ::invslot::TRADE_END; i++) {
			const EQ::ItemInstance *inst = trader2->GetInv().GetItem(i);
			if (inst) {
				t2_entries.emplace_back(PlayerEvent::TradeItemEntry{
					.slot           = i,
					.item_id        = inst->GetItem()->ID,
					.augment_1_id   = inst->GetAugmentItemID(0),
					.augment_1_name = inst->GetAugment(0) ? inst->GetAugment(0)->GetItem()->Name : "",
					.augment_2_id   = inst->GetAugmentItemID(1),
					.augment_2_name = inst->GetAugment(1) ? inst->GetAugment(1)->GetItem()->Name : "",
					.augment_3_id   = inst->GetAugmentItemID(2),
					.augment_3_name = inst->GetAugment(2) ? inst->GetAugment(2)->GetItem()->Name : "",
					.augment_4_id   = inst->GetAugmentItemID(3),
					.augment_4_name = inst->GetAugment(3) ? inst->GetAugment(3)->GetItem()->Name : "",
					.augment_5_id   = inst->GetAugmentItemID(4),
					.augment_5_name = inst->GetAugment(4) ? inst->GetAugment(4)->GetItem()->Name : "",
					.augment_6_id   = inst->GetAugmentItemID(5),
					.augment_6_name = inst->GetAugment(5) ? inst->GetAugment(5)->GetItem()->Name : "",
					.item_name      = inst->GetItem()->Name,
					.charges        = static_cast<uint16>(inst->GetCharges()),
					.in_bag         = false,
				}
				);

				if (inst->IsClassBag()) {
					for (uint8 j = EQ::invbag::SLOT_BEGIN; j <= EQ::invbag::SLOT_END; j++) {
						inst = trader2->GetInv().GetItem(i, j);
						if (inst) {
							t2_entries.emplace_back(PlayerEvent::TradeItemEntry{
								.slot           = j,
								.item_id        = inst->GetItem()->ID,
								.augment_1_id   = inst->GetAugmentItemID(0),
								.augment_1_name = inst->GetAugment(0) ? inst->GetAugment(0)->GetItem()->Name : "",
								.augment_2_id   = inst->GetAugmentItemID(1),
								.augment_2_name = inst->GetAugment(1) ? inst->GetAugment(1)->GetItem()->Name : "",
								.augment_3_id   = inst->GetAugmentItemID(2),
								.augment_3_name = inst->GetAugment(2) ? inst->GetAugment(2)->GetItem()->Name : "",
								.augment_4_id   = inst->GetAugmentItemID(3),
								.augment_4_name = inst->GetAugment(3) ? inst->GetAugment(3)->GetItem()->Name : "",
								.augment_5_id   = inst->GetAugmentItemID(4),
								.augment_5_name = inst->GetAugment(4) ? inst->GetAugment(4)->GetItem()->Name : "",
								.augment_6_id   = inst->GetAugmentItemID(5),
								.augment_6_name = inst->GetAugment(5) ? inst->GetAugment(5)->GetItem()->Name : "",
								.item_name      = inst->GetItem()->Name,
								.charges        = static_cast<uint16>(inst->GetCharges()),
								.in_bag         = true,
							}
							);
						}
					}
				}
			}
		}
	}

	auto e = PlayerEvent::TradeEvent{
		.character_1_id = trader->CharacterID(),
		.character_1_name = trader->GetCleanName(),
		.character_2_id = trader2->CharacterID(),
		.character_2_name = trader2->GetCleanName(),
		.character_1_give_money = money_t,
		.character_2_give_money = money_t2,
		.character_1_give_items = t_entries,
		.character_2_give_items = t2_entries
	};

	RecordPlayerEventLogWithClient(trader, PlayerEvent::TRADE, e);
	//Not sure the usefulness of sending the same data twice??
	RecordPlayerEventLogWithClient(trader2, PlayerEvent::TRADE, e);
}

void Client::ShowSpells(Client* c, ShowSpellType show_spell_type)
{
	std::string spell_string;

	switch (show_spell_type) {
		case ShowSpellType::Disciplines:
			spell_string = "Discipline";
			break;
		case ShowSpellType::Spells:
			spell_string = "Spell";
			break;
		default:
			return;
	}

	std::string spell_table;

	// Headers
	spell_table += DialogueWindow::TableRow(
		fmt::format(
			"{}{}{}",
			DialogueWindow::TableCell("Slot"),
			DialogueWindow::TableCell(spell_string),
			DialogueWindow::TableCell("Spell ID")
		)
	);

	std::map<int, uint16> m;
	auto                  spell_count = 0;

	if (show_spell_type == ShowSpellType::Disciplines) {
		for (auto index = 0; index < MAX_PP_DISCIPLINES; index++) {
			if (IsValidSpell(m_pp.disciplines.values[index])) {
				m[index] = static_cast<uint16>(m_pp.disciplines.values[index]);
			}
		}
	} else if (show_spell_type == ShowSpellType::Spells) {
		for (auto index = 0; index < EQ::spells::SPELL_GEM_COUNT; index++) {
			if (IsValidSpell(m_pp.mem_spells[index])) {
				m[index] = static_cast<uint16>(m_pp.mem_spells[index]);
			}
		}
	}

	for (const auto& s : m) {
		spell_table += DialogueWindow::TableRow(
			fmt::format(
				"{}{}{}",
				DialogueWindow::TableCell(std::to_string(s.first)),
				DialogueWindow::TableCell(GetSpellName(s.second)),
				DialogueWindow::TableCell(Strings::Commify(s.second))
			)
		);

		spell_count++;
	}

	if (!spell_count) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} not have any {}s {}.",
				c->GetTargetDescription(this, TargetDescriptionType::UCYou),
				c == this ? "do" : "does",
				Strings::ToLower(spell_string),
				show_spell_type == ShowSpellType::Disciplines ? "learned" : "memorized"
			).c_str()
		);
		return;
	}

	if (spell_table.size() >= 4096) {
		for (const auto& [index, spell_id] : m) {
			c->Message(
				Chat::White,
				fmt::format(
					"{}. {} ({})",
					index,
					GetSpellName(spell_id),
					spell_id
				).c_str()
			);
		}
		return;
	}

	spell_table = DialogueWindow::Table(spell_table);

	c->SendPopupToClient(
		fmt::format(
			"{}s for {}",
			spell_string,
			c->GetTargetDescription(this, TargetDescriptionType::UCSelf)
		).c_str(),
		spell_table.c_str()
	);
}

std::string GetZoneModeString(ZoneMode mode)
{
	switch (mode) {
		case ZoneToSafeCoords:
			return "ZoneToSafeCoords";
		case GMSummon:
			return "GMSummon";
		case GMHiddenSummon:
			return "GMHiddenSummon";
		case ZoneToBindPoint:
			return "ZoneToBindPoint";
		case ZoneSolicited:
			return "ZoneSolicited";
		case ZoneUnsolicited:
			return "ZoneUnsolicited";
		case GateToBindPoint:
			return "GateToBindPoint";
		case SummonPC:
			return "SummonPC";
		case Rewind:
			return "Rewind";
		case EvacToSafeCoords:
			return "EvacToSafeCoords";
		default:
			return "Unknown";
	}
}

void Client::ClearXTargets()
{
	if (!XTargettingAvailable()) {
		return;
	}

	for (int i = 0; i < GetMaxXTargets(); ++i) {
		if (XTargets[i].ID) {
			Mob* m = entity_list.GetMob(XTargets[i].ID);

			if (m) {
				RemoveXTarget(m, false);
			}

			XTargets[i].ID      = 0;
			XTargets[i].Name[0] = 0;
			XTargets[i].dirty   = false;

			SendXTargetPacket(i, nullptr);
		}
	}
}

float Client::GetAAEXPModifier(uint32 zone_id, int16 instance_version)
{
	return database.GetAAEXPModifierByCharID(
		CharacterID(),
		zone_id,
		instance_version
	);
}

float Client::GetEXPModifier(uint32 zone_id, int16 instance_version)
{
	return database.GetEXPModifierByCharID(
		CharacterID(),
		zone_id,
		instance_version
	);
}

void Client::SetAAEXPModifier(uint32 zone_id, float aa_modifier, int16 instance_version)
{
	database.SetAAEXPModifierByCharID(
		CharacterID(),
		zone_id,
		aa_modifier,
		instance_version
	);

	database.LoadCharacterEXPModifier(this);
}

void Client::SetEXPModifier(uint32 zone_id, float exp_modifier, int16 instance_version)
{
	database.SetEXPModifierByCharID(
		CharacterID(),
		zone_id,
		exp_modifier,
		instance_version
	);

	database.LoadCharacterEXPModifier(this);
}

int Client::GetAAEXPPercentage()
{
	int scaled = static_cast<int>(330.0f * static_cast<float>(GetAAXP()) / GetRequiredAAExperience());

	return static_cast<int>(std::round(scaled * 100.0 / 330.0));
}

int Client::GetEXPPercentage()
{
	float    norm = 0.0f;
	uint32_t min  = GetEXPForLevel(GetLevel());
	uint32_t max  = GetEXPForLevel(GetLevel() + 1);

	if (min != max) {
		norm = static_cast<float>(GetEXP() - min) / (max - min);
	}

	int scaled = static_cast<int>(330.0f * norm); // scale and truncate

	return static_cast<int>(std::round(scaled * 100.0 / 330.0)); // unscaled pct
}

std::vector<Mob*> Client::GetRaidOrGroupOrSelf(bool clients_only)
{
	std::vector<Mob*> v;

	if (IsRaidGrouped()) {
		Raid* r = GetRaid();

		if (r) {
			for (const auto& m : r->members) {
				if (m.member && (!m.is_bot || !clients_only)) {
					v.emplace_back(m.member);
				}
			}
		}
	} else if (IsGrouped()) {
		Group* g = GetGroup();

		if (g) {
			for (const auto& m : g->members) {
				if (m && (m->IsClient() || !clients_only)) {
					v.emplace_back(m);
				}
			}
		}
	} else {
		v.emplace_back(this);
	}

	return v;
}

uint16 Client::GetSkill(EQ::skills::SkillType skill_id) const
{
	if (skill_id <= EQ::skills::HIGHEST_SKILL) {
		return (itembonuses.skillmod[skill_id] > 0 ? (itembonuses.skillmodmax[skill_id] > 0 ? std::min(
			m_pp.skills[skill_id] + itembonuses.skillmodmax[skill_id],
			m_pp.skills[skill_id] * (100 + itembonuses.skillmod[skill_id]) / 100
		) : m_pp.skills[skill_id] * (100 + itembonuses.skillmod[skill_id]) / 100) : m_pp.skills[skill_id]);
	}
	return 0;
}

void Client::RemoveItemBySerialNumber(uint32 serial_number, uint32 quantity)
{
	EQ::ItemInstance *item = nullptr;

	uint32 removed_count = 0;

	const auto& slot_ids = GetInventorySlots();

	for (const int16& slot_id : slot_ids) {
		if (removed_count == quantity) {
			break;
		}

		item = GetInv().GetItem(slot_id);
		if (item && item->GetSerialNumber() == serial_number) {
			uint32 charges    = item->IsStackable() ? item->GetCharges() : 0;
			uint32 stack_size = std::max(charges, static_cast<uint32>(1));
			if ((removed_count + stack_size) <= quantity) {
				removed_count += stack_size;
				DeleteItemInInventory(slot_id, charges, true);
			} else {
				uint32 amount_left = (quantity - removed_count);
				if (amount_left > 0 && stack_size >= amount_left) {
					removed_count += amount_left;
					DeleteItemInInventory(slot_id, amount_left, true);
				}
			}
		}
	}
}

void Client::SendTopLevelInventory()
{
	EQ::ItemInstance* inst = nullptr;

	static const int16 slots[][2] = {
		{ EQ::invslot::POSSESSIONS_BEGIN,     EQ::invslot::POSSESSIONS_END },
		{ EQ::invbag::GENERAL_BAGS_BEGIN,     EQ::invbag::GENERAL_BAGS_END },
		{ EQ::invbag::CURSOR_BAG_BEGIN,       EQ::invbag::CURSOR_BAG_END }
	};

	const auto& inv = GetInv();

	const size_t slot_index_count = sizeof(slots) / sizeof(slots[0]);
	for (int slot_index = 0; slot_index < slot_index_count; ++slot_index) {
		for (int slot_id = slots[slot_index][0]; slot_id <= slots[slot_index][1]; ++slot_id) {
			inst = inv.GetItem(slot_id);
			if (inst) {
				SendItemPacket(slot_id, inst, ItemPacketType::ItemPacketTrade);
			}
		}
	}
}

void Client::CheckSendBulkNpcPositions(bool force)
{
	float distance_moved                      = DistanceNoZ(m_last_position_before_bulk_update, GetPosition());
	float update_range                        = RuleI(Range, MobCloseScanDistance);
	bool  moved_far_enough_before_bulk_update = distance_moved >= update_range;
	bool  is_ready_to_update                  = (
		m_client_bulk_npc_pos_update_timer.Check() || moved_far_enough_before_bulk_update
	);

	int updated_count = 0;
	int skipped_count = 0;
	if (is_ready_to_update || force) {
		auto &mob_movement_manager = MobMovementManager::Get();

		for (auto &e: entity_list.GetMobList()) {
			Mob *mob = e.second;
			if (!mob->IsNPC()) {
				continue;
			}

			int animation_speed = 0;
			if (mob->IsMoving()) {
				if (mob->IsRunning()) {
					animation_speed = (mob->IsFeared() ? mob->GetFearSpeed() : mob->GetRunspeed());
				}
				else {
					animation_speed = mob->GetWalkspeed();
				}
			}

			// if we have seen this mob before, and it hasn't moved, skip it
			if (m_last_seen_mob_position.contains(mob->GetID())) {
				if (m_last_seen_mob_position[mob->GetID()] == mob->GetPosition()) {
					LogPositionUpdateDetail(
						"Mob [{}] has already been sent to client [{}] at this position, skipping",
						mob->GetCleanName(),
						GetCleanName()
					);
					skipped_count++;
					continue;
				}
			}

			mob_movement_manager.SendCommandToClients(
				mob,
				0.0,
				0.0,
				0.0,
				0.0,
				animation_speed,
				ClientRangeAny,
				this
			);

			updated_count++;
		}

		if (force) {
			static EQApplicationPacket p(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
			auto      *s = (PlayerPositionUpdateServer_Struct *) p.pBuffer;
			for (auto &e: entity_list.GetCorpseList()) {
				Corpse *c = e.second;
				MakeSpawnUpdate(s);
				QueuePacket(&p, false);
			}
		}

		LogPositionUpdate(
			"[{}] Sent [{}] bulk updated NPC positions, skipped [{}] distance_moved [{}] update_range [{}]",
			GetCleanName(),
			updated_count,
			skipped_count,
			distance_moved,
			update_range
		);

		m_last_position_before_bulk_update = GetPosition();
	}
}


const uint16 scan_npc_aggro_timer_idle   = RuleI(Aggro, ClientAggroCheckIdleInterval);
const uint16 scan_npc_aggro_timer_moving = RuleI(Aggro, ClientAggroCheckMovingInterval);

void Client::CheckClientToNpcAggroTimer()
{
	LogAggroDetail(
		"ClientUpdate [{}] {}moving, scan timer [{}]",
		GetCleanName(),
		IsMoving() ? "" : "NOT ",
		m_client_npc_aggro_scan_timer.GetRemainingTime()
	);

	if (IsMoving()) {
		if (m_client_npc_aggro_scan_timer.GetRemainingTime() > scan_npc_aggro_timer_moving) {
			LogAggroDetail("Client [{}] Restarting with moving timer", GetCleanName());
			m_client_npc_aggro_scan_timer.Disable();
			m_client_npc_aggro_scan_timer.Start(scan_npc_aggro_timer_moving);
			m_client_npc_aggro_scan_timer.Trigger();
		}
	}
	else if (m_client_npc_aggro_scan_timer.GetDuration() == scan_npc_aggro_timer_moving) {
		LogAggroDetail("Client [{}] Restarting with idle timer", GetCleanName());
		m_client_npc_aggro_scan_timer.Disable();
		m_client_npc_aggro_scan_timer.Start(scan_npc_aggro_timer_idle);
	}
}

void Client::ClientToNpcAggroProcess()
{
	if (zone->CanDoCombat() && !GetFeigned() && m_client_npc_aggro_scan_timer.Check()) {
		int npc_scan_count = 0;
		for (auto& close_mob : GetCloseMobList()) {
			Mob* mob = close_mob.second;
			if (!mob) {
				continue;
			}

			if (mob->IsClient()) {
				continue;
			}

			if (mob->CheckWillAggro(this) && !mob->CheckAggro(this)) {
				mob->AddToHateList(this, 25);
			}

			npc_scan_count++;
		}
		LogAggro("Checking Reverse Aggro (client->npc) scanned_npcs ([{}])", npc_scan_count);
	}
}

const std::vector<int16>& Client::GetInventorySlots()
{
	static const std::vector<std::pair<int16, int16>> slots = {
		{EQ::invslot::POSSESSIONS_BEGIN,     EQ::invslot::POSSESSIONS_END},
		{EQ::invbag::GENERAL_BAGS_BEGIN,     EQ::invbag::GENERAL_BAGS_END},
		{EQ::invbag::CURSOR_BAG_BEGIN,       EQ::invbag::CURSOR_BAG_END},
		{EQ::invslot::BANK_BEGIN,            EQ::invslot::BANK_END},
		{EQ::invbag::BANK_BAGS_BEGIN,        EQ::invbag::BANK_BAGS_END},
		{EQ::invslot::SHARED_BANK_BEGIN,     EQ::invslot::SHARED_BANK_END},
		{EQ::invbag::SHARED_BANK_BAGS_BEGIN, EQ::invbag::SHARED_BANK_BAGS_END},
	};

	static std::vector<int16> slot_ids;

	if (slot_ids.empty()) {
		for (const auto &[begin, end]: slots) {
			for (int16 slot_id = begin; slot_id <= end; ++slot_id) {
				slot_ids.emplace_back(slot_id);
			}
		}
	}

	return slot_ids;
}

void Client::ShowZoneShardMenu()
{
	auto z = GetZone(GetZoneID());
	if (z && !z->shard_at_player_count) {
		return;
	}

	auto results = CharacterDataRepository::GetInstanceZonePlayerCounts(database, GetZoneID());
	LogZoning("Zone sharding results count [{}]", results.size());

	if (results.empty()) {
		Message(Chat::White, "No zone shards found.");
		return;
	}

	if (!results.empty()) {
		Message(Chat::White, "Available Zone Shards:");
	}

	int number = 1;
	for (auto &e: results) {
		std::string teleport_link = Saylink::Silent(
			fmt::format("#zoneshard {} {}", e.zone_id, (e.instance_id == 0 ? -1 : e.instance_id)),
			"Teleport"
		);

		std::string yours;
		if (e.zone_id == GetZoneID() && e.instance_id == GetInstanceID()) {
			teleport_link = "Teleport";
			yours         = " (Yours)";
		}

		Message(
			Chat::White, fmt::format(
				" --> [{}] #{} {} ({}) [{}/{}] players {}",
				teleport_link,
				number,
				z->long_name,
				e.instance_id,
				e.player_count,
				z->shard_at_player_count,
				yours
			).c_str()
		);
		number++;
	}
}

std::string Client::SendBotCommandHelpWindow(const BotCommandHelpParams& params) {
	unsigned string_length = 0;
	unsigned current_place = 0;
	uint16 max_length = RuleI(Command, MaxHelpLineLength); // Line length before splitting
	const std::string& header_color = "indian_red";
	const std::string& description_color = "light_grey";
	const std::string& description_color_secondary = "dark_orange";
	const std::string& example_color = "goldenrod";
	const std::string& example_color_secondary = "slate_blue";
	const std::string& option_color = "light_grey";
	const std::string& option_color_secondary = "slate_blue";
	const std::string& filler_line_color = "dark_grey";

	std::string filler_line = "--------------------------------------------------------------------";
	std::string filler_dia = DialogueWindow::TableRow(DialogueWindow::TableCell(fmt::format("{}", DialogueWindow::ColorMessage(filler_line_color, filler_line))));
	std::string break_line = DialogueWindow::Break();
	std::string popup_text;

	if (!params.description.empty()) {
		popup_text += GetCommandHelpHeader("[Description]", header_color);
		popup_text += SplitCommandHelpText(params.description, description_color, max_length);
	}

	if (!params.notes.empty()) {
		popup_text += break_line + break_line;
		popup_text += GetCommandHelpHeader("[Notes]", header_color);
		popup_text += SplitCommandHelpText(params.notes, description_color_secondary, max_length);
	}

	if (!params.example_format.empty()) {
		popup_text += filler_dia;
		popup_text += GetCommandHelpHeader("[Examples]", header_color);
		popup_text += SplitCommandHelpText(params.example_format, example_color, max_length);
	}

	if (!params.examples_one.empty()) {
		popup_text += break_line + break_line;
		popup_text += SplitCommandHelpText(params.examples_one, example_color, max_length, example_color_secondary);
	}

	if (!params.examples_two.empty()) {
		popup_text += SplitCommandHelpText(params.examples_two, example_color, max_length, example_color_secondary);
	}

	if (!params.examples_three.empty()) {
		popup_text += SplitCommandHelpText(params.examples_three, example_color, max_length, example_color_secondary);
	}

	if (!params.options.empty()) {
		popup_text += filler_dia;
		popup_text += GetCommandHelpHeader("[Options]", header_color);
		popup_text += SplitCommandHelpText(params.options, option_color, max_length);
	}

	if (!params.options_one.empty()) {
		popup_text += break_line + break_line;
		popup_text += SplitCommandHelpText(params.options_one, option_color_secondary, max_length);
	}

	if (!params.options_two.empty()) {
		popup_text += SplitCommandHelpText(params.options_two, option_color_secondary, max_length);
	}

	if (!params.options_three.empty()) {
		popup_text += SplitCommandHelpText(params.options_three, option_color_secondary, max_length);
	}

	if (!params.actionables.empty()) {
		popup_text += filler_dia;
		popup_text += GetCommandHelpHeader("[Actionables]", header_color);
		popup_text += SplitCommandHelpText(params.actionables, description_color, max_length);
	}

	popup_text = DialogueWindow::Table(popup_text);

	return popup_text;
}

std::string Client::GetCommandHelpHeader(std::string msg, std::string color) {
	std::string return_text = DialogueWindow::TableRow(
		DialogueWindow::TableCell(
			fmt::format(
				"{}",
				DialogueWindow::ColorMessage(color, msg)
			)
		)
	);

	return return_text;
}

std::string Client::SplitCommandHelpText(std::vector<std::string> msg, std::string color, uint16 max_length, std::string secondary_color) {
	std::string return_text;

	for (int i = 0; i < msg.size(); i++) {
		std::vector<std::string> msg_split;
		int string_length = msg[i].length() + 1;
		int end_count = 0;
		int new_count = 0;
		int split_count = 0;

		for (int x = 0; x < string_length; x = end_count) {
			end_count = std::min(int(string_length), (int(x) + std::min(int(string_length), int(max_length))));

			if ((string_length - (x + 1)) > max_length) {
				for (int y = end_count; y >= x; --y) {
					if (msg[i][y] == ' ') {
							split_count = y - x;
							msg_split.emplace_back(msg[i].substr(x, split_count));
							end_count = y + 1;

							break;
					}

					if (y == x) {
						msg_split.emplace_back(msg[i].substr(x, max_length));

						break;
					}
				}
			}
			else {
				msg_split.emplace_back(msg[i].substr(x, (string_length - 1) - x));

				break;
			}
		}

		for (const auto& s : msg_split) {
			return_text += DialogueWindow::TableRow(
				DialogueWindow::TableCell(DialogueWindow::ColorMessage(((!secondary_color.empty() && i== 0) ? secondary_color : color), s))
			);
		}
	}

	return return_text;
}

void Client::SetAAEXPPercentage(uint8 percentage)
{
	const uint32 before_percentage = m_epp.perAA;

	if (before_percentage > 0 && percentage == 0) {
		MessageString(Chat::White, AA_OFF);
	}
	else if (before_percentage == 0 && percentage > 0) {
		MessageString(Chat::White, AA_ON);
	}

	m_epp.perAA = EQ::Clamp(static_cast<int>(percentage), 0, 100);

	SendAlternateAdvancementStats();
	SendAlternateAdvancementTable();
}

void Client::BroadcastPositionUpdate()
{
	EQApplicationPacket               outapp(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
	PlayerPositionUpdateServer_Struct *spu = (PlayerPositionUpdateServer_Struct *) outapp.pBuffer;

	memset(spu, 0x00, sizeof(PlayerPositionUpdateServer_Struct));
	spu->spawn_id      = GetID();
	spu->x_pos         = FloatToEQ19(GetX());
	spu->y_pos         = FloatToEQ19(GetY());
	spu->z_pos         = FloatToEQ19(GetZ());
	spu->heading       = FloatToEQ12(GetHeading());
	spu->delta_x       = FloatToEQ13(0);
	spu->delta_y       = FloatToEQ13(0);
	spu->delta_z       = FloatToEQ13(0);
	spu->delta_heading = FloatToEQ10(0);
	spu->animation     = 0;

	entity_list.QueueCloseClients(this, &outapp, true, zone->GetClientUpdateRange());

	Group *g = GetGroup();
	if (g) {
		for (auto &m: g->members) {
			if (m && m->IsClient() && m != this) {
				m->CastToClient()->QueuePacket(&outapp);
			}
		}
	}
}

std::string Client::GetAccountBucket(std::string bucket_name)
{
	DataBucketKey k = {};
	k.account_id   = AccountID();
	k.key          = bucket_name;

	return DataBucket::GetData(&database, k).value;
}

void Client::SetAccountBucket(std::string bucket_name, std::string bucket_value, std::string expiration)
{
	DataBucketKey k = {};
	k.account_id   = AccountID();
	k.key          = bucket_name;
	k.expires      = expiration;
	k.value        = bucket_value;

	DataBucket::SetData(&database, k);
}

void Client::DeleteAccountBucket(std::string bucket_name)
{
	DataBucketKey k = {};
	k.account_id   = AccountID();
	k.key          = bucket_name;

	DataBucket::DeleteData(&database, k);
}

std::string Client::GetAccountBucketExpires(std::string bucket_name)
{
	DataBucketKey k = {};
	k.account_id   = AccountID();
	k.key          = bucket_name;

	return DataBucket::GetDataExpires(&database, k);
}

std::string Client::GetAccountBucketRemaining(std::string bucket_name)
{
	DataBucketKey k = {};
	k.account_id   = AccountID();
	k.key          = bucket_name;

	return DataBucket::GetDataRemaining(&database, k);
}

std::string Client::GetBandolierName(uint8 bandolier_slot)
{
	if (!EQ::ValueWithin(bandolier_slot, 0, 3)) {
		return std::string();
	}

	return GetPP().bandoliers[bandolier_slot].Name;
}

uint32 Client::GetBandolierItemIcon(uint8 bandolier_slot, uint8 slot_id)
{
	if (
		!EQ::ValueWithin(bandolier_slot, 0, 3) ||
		!EQ::ValueWithin(slot_id, 0, 3)
	) {
		return 0;
	}

	return GetPP().bandoliers[bandolier_slot].Items[slot_id].Icon;
}

uint32 Client::GetBandolierItemID(uint8 bandolier_slot, uint8 slot_id)
{
	if (
		!EQ::ValueWithin(bandolier_slot, 0, 3) ||
		!EQ::ValueWithin(slot_id, 0, 3)
	) {
		return 0;
	}

	return GetPP().bandoliers[bandolier_slot].Items[slot_id].ID;
}

std::string Client::GetBandolierItemName(uint8 bandolier_slot, uint8 slot_id)
{
	if (
		!EQ::ValueWithin(bandolier_slot, 0, 3) ||
		!EQ::ValueWithin(slot_id, 0, 3)
		) {
		return std::string();
	}

	return GetPP().bandoliers[bandolier_slot].Items[slot_id].Name;
}

void Client::SendMerchantEnd()
{
	SetMerchantSessionEntityID(0);

	if (ClientVersion() == EQ::versions::ClientVersion::RoF2 && RuleB(Parcel, EnableParcelMerchants)) {
		DoParcelCancel();
		SetEngagedWithParcelMerchant(false);
	}

	EQApplicationPacket empty(OP_ShopEndConfirm);
	QueuePacket(&empty);
}

void Client::CheckItemDiscoverability(uint32 item_id)
{
	if (!RuleB(Character, EnableDiscoveredItems) || IsDiscovered(item_id)) {
		return;
	}

	if (GetGM()) {
		const std::string& item_link = database.CreateItemLink(item_id);
		Message(
			Chat::White,
			fmt::format(
				"Your GM flag prevents {} from being added to discovered items.",
				item_link
			).c_str()
		);
		return;
	}

	DiscoverItem(item_id);
}

bool Client::UncompleteTask(int task_id)
{
	CompletedTasksRepository::DeleteWhere(
		database,
		fmt::format(
			"charid = {} AND taskid = {}",
			CharacterID(),
			task_id
		)
	);

	return task_state->UncompleteTask(task_id);
}
