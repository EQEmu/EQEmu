/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2004 EQEMu Development Team (http://eqemu.org)

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

#pragma once

/*

	This file defines many optional features for the emu
	as well as various parameters used by the emu.

	If ambitious, most of these could prolly be turned into
	database variables, but the really frequently run pieces
	of code, should not be done that way for speed reasons IMO

*/

/*

Core Zone features

*/

#ifdef EMBPERL
//Enable the new XS based perl parser
#define EMBPERL_XS

//enable classes in the new XS based parser
#define EMBPERL_XS_CLASSES

//enable IO capture and transmission to in game clients
//this seems to make perl very unhappy on reload, and crashes
#define EMBPERL_IO_CAPTURE

#endif

/*

Map Configuration
In general, these computations are expensive, so if you have performance
problems, consider turning them off.

*/

//uncomment this to make the LOS code say all mobs can see all others
//when no map file is loaded, opposed to the default nobody-sees-anybody
#define LOS_DEFAULT_CAN_SEE

/*

Zone extensions and features

*/

//Uncomment this to scale XP gained from MOBs based on their CON
//#define CON_XP_SCALING

//Uncomment to make group buffs affect group pets
#define GROUP_BUFF_PETS

//Uncomment this line to enable named quest files:
#define QUEST_SCRIPTS_BYNAME

#ifdef QUEST_SCRIPTS_BYNAME
//extends byname system to look in a templates directory
//independant of zone name
#define QUEST_GLOBAL_BYNAME
#define QUEST_GLOBAL_DIRECTORY "global"
#endif

//the min ratio at which a mob's speed is reduced
#define FLEE_HP_MINSPEED 22
//number of tics to try to run straight away before looking again
#define FLEE_RUN_DURATION 1000
//number of miliseconds between when a mob will check its flee state
//this is only checked when the mob is damaged.
#define FLEE_CHECK_TIMER 2000

//enable functionality to send log message to the client
//it still needs to be enabled with the #logs command
#define CLIENT_LOGS

//uncomment to print warnings about commands with 0 status...
//#define COMMANDS_WARNINGS

//uncomment to allow perl commands to override compiled commands
#define COMMANDS_PERL_OVERRIDE

//only log commands which require this minimum status or more
#define COMMANDS_LOGGING_MIN_STATUS 1

//path to where sql logs should be placed
#define SQL_LOG_PATH "sql_logs/"

//The highest you can #setskill / #setallskill
#define HIGHEST_CAN_SET_SKILL 400

#define SKILL_MAX_LEVEL 75

/*

Zone Numerical configuration

*/

//Reuse times for various skills, here for convenience, in sec
//set to 0 to disable server side checking of timers.
enum {	//reuse times
	FeignDeathReuseTime = 9,
	SneakReuseTime = 7,
	HideReuseTime = 8,
	TauntReuseTime = 5,
	InstillDoubtReuseTime = 9,
	FishingReuseTime = 11,
	ForagingReuseTime = 50,
	MendReuseTime = 360,
	BashReuseTime = 5,
	BackstabReuseTime = 9,
	KickReuseTime = 5,
	TailRakeReuseTime = 6,
	EagleStrikeReuseTime = 5,
	RoundKickReuseTime = 9,
	TigerClawReuseTime = 6,
	FlyingKickReuseTime = 7,
	SenseTrapsReuseTime = 9,
	DisarmTrapsReuseTime = 9,
	HarmTouchReuseTime = 4300,
	LayOnHandsReuseTime = 4300,
	FrenzyReuseTime = 10
};

enum {	//timer settings, all in milliseconds
	AImovement_duration = 100,
	AIthink_duration = 50,
	AIscanarea_delay = 6000,
	AIfeignremember_delay = 500,
	AItarget_check_duration = 500,
	AI_scan_door_open_interval = 1000,
	// AIClientScanarea_delay = 750,	//used in REVERSE_AGGRO
	AIassistcheck_delay = 3000,		//now often a fighting NPC will yell for help
	AI_check_signal_timer_delay = 500, // How often EVENT_SIGNAL checks are processed
	ClientProximity_interval = 150,
	CombatEventTimer_expire = 12000,
	Tribute_duration = 600000,
	FeignMemoryDuration = 120000, // Duration player must feign death to clear zonewide agro.
	EnragedTimer = 360000,
	EnragedDurationTimer = 10000
};

enum {	//some random constants
	//each of these attack modifiers are added to the NPC's level to determine their
	//probability of executing such an attack (which may or may not hit)
	NPCDualAttackModifier = 20,
	NPCTripleAttackModifier = 0,
	NPCQuadAttackModifier = -20
};

//Max number of groups you can link with. Not tied to the client.
//if group linking is enabled above
#define MAX_GROUP_LINKS 8

//this is the number of levels above the thief's level that
//an npc can be and still let the theif PP them
#define THIEF_PICKPOCKET_OVER 5

//this is the % chance that an NPC will dual wield a 2nd weapon
//in its loot table, if it is able to.
//Aug 2007: was 5% chance.. changed to 100% by default since that seems more normal
//Kept it intact codewise incase someone wants to or is already using it.
#define NPC_DW_CHANCE 100

//This is the entry in npc_types to spawn for trap damagaes
#define TRAP_NPC_TYPE 1586

//This is the multiplier of eqemu speed to get client speed
//tweak this if pathing mobs seem to jump forward or backwards
//this should prolly be dynamic based on ping time or something.. who knows
//Values found in the emu somewhere at one point in time: 36, 43
#define NPC_RUNANIM_RATIO 37

//this is used to multiply an NPCs movement rate, yeilding map units..
#define NPC_SPEED_MULTIPLIER 46		//used to be 2.8... no idea why it changed

//minimum level to do alchemy
#define MIN_LEVEL_ALCHEMY 25

//chance ratio that a
#define THREATENINGLY_AGGRO_CHANCE 32 // 32/128 (25%) chance that a mob will arrgo on con Threatenly

//max factions per npc faction list
#define MAX_NPC_FACTIONS 20

//individual faction pool
#define MAX_PERSONAL_FACTION 2000
#define MIN_PERSONAL_FACTION -2000

//The Level Cap:
//#define LEVEL_CAP RuleI(Character, MaxLevel)	//hard cap is 127
#define HARD_LEVEL_CAP 127

//the square of the maximum range at whihc you could possibly use NPC services (shop, tribute, etc)
#define USE_NPC_RANGE2 40000		//arbitrary right now

// Squared range for rampage 75.0 * 75.0 for now
#define NPC_RAMPAGE_RANGE2 5625.0f

//the formula for experience for killing a mob.
//level is the only valid variable to use
#define EXP_FORMULA (level * level * 75 * 35 / 10)

#define HIGHEST_AA_VALUE 35

//Leadership AA experience points
#define GROUP_EXP_PER_POINT 1000
#define RAID_EXP_PER_POINT 2000

#define ZONE_CONTROLLER_NPC_ID 10

// Timer to update aggrometer
#define AGGRO_METER_UPDATE_MS 1000

// This is the item ID we use for say links, we use the max that fits in 5 ASCII chars
#define SAYLINK_ITEM_ID 0xFFFFF


// consumption timers for food/drink here instead of rules because the client
// uses these. Times in ms.
#define CONSUMPTION_TIMER 46000
#define CONSUMPTION_MNK_TIMER 92000

/*

Developer configuration

*/

//#define EQPROFILE
#ifdef EQPROFILE
//Enable the zone profiler
#define ZONE_PROFILE

#define COMMON_PROFILE

#define PROFILE_DUMP_TIME 180
#endif	//EQPROFILE
