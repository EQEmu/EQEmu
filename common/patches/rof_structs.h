/*	EQEMu: Everquest Server Emulator

	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#pragma once

#include "common/patches/rof_limits.h"
#include "common/textures.h"
#include "common/types.h"

namespace RoF { namespace structs {

/*
** Compiler override to ensure
** byte aligned structures
*/
#pragma pack(push)
#pragma pack(1)

struct LoginInfo_Struct {
/*000*/	char	login_info[64];
/*064*/	uint8	unknown064[124];
/*188*/	uint8	zoning;			// 01 if zoning, 00 if not
/*189*/	uint8	unknown189[275];
/*488*/
};

struct EnterWorld_Struct {
/*000*/	char	name[64];
/*064*/	uint32	tutorial;		// 01 on "Enter Tutorial", 00 if not
/*068*/	uint32	return_home;		// 01 on "Return Home", 00 if not
};

//New For SoF
struct WorldObjectsSent_Struct {
};

// New for RoF - Size: 12
struct InventorySlot_Struct
{
/*000*/	int16	Type;		// Worn and Normal inventory = 0, Bank = 1, Shared Bank = 2, Delete Item = -1
/*002*/	int16	Unknown02;
/*004*/	int16	Slot;
/*006*/	int16	SubIndex;
/*008*/	int16	AugIndex;	// Guessing - Seen 0xffff
/*010*/	int16	Unknown01;	// Normally 0 - Seen 13262 when deleting an item, but didn't match item ID
/*012*/
};

// New for RoF - Used for Merchant_Purchase_Struct
// Can't sellfrom other than main inventory so Slot Type is not needed.
struct TypelessInventorySlot_Struct
{
/*000*/	int16	Slot;
/*002*/	int16	SubIndex;
/*004*/	int16	AugIndex;
/*006*/	int16	Unknown01;
/*008*/
};

/* Name Approval Struct */
/* Len: */
/* Opcode: 0x8B20*/
struct NameApproval
{
	char name[64];
	uint32 race;
	uint32 class_;
	uint32 deity;
};

/*
** Entity identification struct
** Size: 4 bytes
** OPCodes: OP_DeleteSpawn, OP_Assist
*/
struct EntityId_Struct
{
/*00*/	uint32	entity_id;
/*04*/
};

struct Duel_Struct
{
	uint32 duel_initiator;
	uint32 duel_target;
};

struct DuelResponse_Struct
{
	uint32 target_id;
	uint32 entity_id;
	uint32 unknown;
};

//Adventure stuff,not a net one,just one for our use
static const uint32 ADVENTURE_COLLECT		= 0;
static const uint32 ADVENTURE_MASSKILL		= 1;
static const uint32 ADVENTURE_NAMED			= 2;
static const uint32 ADVENTURE_RESCUE		= 3;

static const uint32 BUFF_COUNT = 42;			// was 25
static const uint32 BLOCKED_BUFF_COUNT = 30;		// was 20

static const uint32 MAX_PLAYER_TRIBUTES = 5;
static const uint32 MAX_TRIBUTE_TIERS = 10;
static const uint32 TRIBUTE_NONE = 0xFFFFFFFF;

static const uint32 MAX_GROUP_LEADERSHIP_AA_ARRAY = 16;
static const uint32 MAX_RAID_LEADERSHIP_AA_ARRAY = 16;
static const uint32 MAX_LEADERSHIP_AA_ARRAY = (MAX_GROUP_LEADERSHIP_AA_ARRAY+MAX_RAID_LEADERSHIP_AA_ARRAY);

static const uint32 MAX_NUMBER_GUILDS = 1500;

// Used primarily in the Player Profile:
static const uint32 MAX_PP_LANGUAGE		= 32;	// was 25

static const uint32 MAX_PP_SKILL		= PACKET_SKILL_ARRAY_SIZE;	// 100 - actual skills buffer size
static const uint32 MAX_PP_INNATE_SKILL	= 25;
static const uint32 MAX_PP_AA_ARRAY		= 300;
static const uint32 MAX_PP_DISCIPLINES	= 200;	// was 100
static const uint32 MAX_GROUP_MEMBERS	= 6;
static const uint32 MAX_RECAST_TYPES	= 20;

struct AdventureInfo {
	uint32 QuestID;
	uint32 NPCID;
	bool in_use;
	uint32 status;
	bool ShowCompass;
	uint32 Objetive;// can be item to collect,mobs to kill,boss to kill and someone to rescue.
	uint32 ObjetiveValue;// number of items,or number of needed mob kills.
	char text[512];
	uint8 type;
	uint32 minutes;
	uint32 points;
	float x;
	float y;
	uint32 zoneid;
	uint32 zonedungeonid;
};
///////////////////////////////////////////////////////////////////////////////


/*
** Color_Struct
** Size: 4 bytes
** Used for convenience
** Merth: Gave struct a name so gcc 2.96 would compile
**
*/
struct Tint_Struct
{
	union {
		struct {
			uint8 Blue;
			uint8 Green;
			uint8 Red;
			uint8 UseTint;	// if there's a tint this is FF
		};
		uint32 Color;
	};
};

struct TintProfile
{
	union {
		struct {
			Tint_Struct Head;
			Tint_Struct Chest;
			Tint_Struct Arms;
			Tint_Struct Wrist;
			Tint_Struct Hands;
			Tint_Struct Legs;
			Tint_Struct Feet;
			Tint_Struct Primary;
			Tint_Struct Secondary;
		};
		Tint_Struct Slot[EQ::textures::materialCount];
	};
};

/*
* Visible equiptment.
* Size: 20 Octets
*/
struct Texture_Struct
{
	uint32 Material;
	uint32 Unknown1;
	uint32 EliteMaterial;
	uint32 HeroForgeModel;
	uint32 Material2;	// Same as material?
};

// Needs more research regarding new slots
//struct TextureProfile
//{
//	union {
//		struct {
//			Texture_Struct Head;
//			Texture_Struct Chest;
//			Texture_Struct Arms;
//			Texture_Struct Wrist;
//			Texture_Struct Hands;
//			Texture_Struct Legs;
//			Texture_Struct Feet;
//			Texture_Struct Primary;
//			Texture_Struct Secondary;
//		};
//		Texture_Struct Slot[EQ::textures::TextureCount];
//	};
//
//	TextureProfile();
//};

struct CharSelectEquip : Texture_Struct, Tint_Struct {};

struct CharacterSelectEntry_Struct
{
/*0000*/	char Name[1];				// Name null terminated
/*0000*/	uint8 Class;
/*0000*/	uint32 Race;
/*0000*/	uint8 Level;
/*0000*/	uint8 ShroudClass;
/*0000*/	uint32 ShroudRace;
/*0000*/	uint16 Zone;
/*0000*/	uint16 Instance;
/*0000*/	uint8 Gender;
/*0000*/	uint8 Face;
/*0000*/	CharSelectEquip	Equip[9];
/*0000*/	uint8 Unknown15;			// Seen FF
/*0000*/	uint8 Unknown19;			// Seen FF
/*0000*/	uint32 DrakkinTattoo;
/*0000*/	uint32 DrakkinDetails;
/*0000*/	uint32 Deity;
/*0000*/	uint32 PrimaryIDFile;
/*0000*/	uint32 SecondaryIDFile;
/*0000*/	uint8 HairColor;
/*0000*/	uint8 BeardColor;
/*0000*/	uint8 EyeColor1;
/*0000*/	uint8 EyeColor2;
/*0000*/	uint8 HairStyle;
/*0000*/	uint8 Beard;
/*0000*/	uint8 GoHome;				// Seen 0 for new char and 1 for existing
/*0000*/	uint8 Tutorial;				// Seen 1 for new char or 0 for existing
/*0000*/	uint32 DrakkinHeritage;
/*0000*/	uint8 Unknown1;				// Seen 0
/*0000*/	uint8 Enabled;				// Swapped position with 'GoHome' 02/23/2015
/*0000*/	uint32 LastLogin;
/*0000*/	uint8 Unknown2;				// Seen 0
};

/*
** Character Selection Struct
**
*/
struct CharacterSelect_Struct
{
/*000*/	uint32 CharCount;	//number of chars in this packet
/*004*/	CharacterSelectEntry_Struct Entries[0];
};

struct Membership_Entry_Struct
{
/*000*/ uint32 purchase_id;		// Seen 1, then increments 90287 to 90300
/*004*/ uint32 bitwise_entry;	// Seen 16 to 65536 - Skips 4096
/*008*/
};

struct Membership_Setting_Struct
{
/*000*/ uint32 setting_index;	// 0, 1, or 2
/*004*/ uint32 setting_id;		// 0 to 21
/*008*/ int32 setting_value;	// All can be 0, 1, or -1
/*012*/
};

struct Membership_Details_Struct
{
/*0000*/ uint32 membership_setting_count;	// Seen 66
/*0016*/ Membership_Setting_Struct settings[66];
/*0012*/ uint32 race_entry_count;	// Seen 15
/*1044*/ Membership_Entry_Struct membership_races[15];
/*0012*/ uint32 class_entry_count;	// Seen 15
/*1044*/ Membership_Entry_Struct membership_classes[15];
/*1044*/ uint32 exit_url_length;	// Length of the exit_url string (0 for none)
/*1048*/ //char exit_url[42];		// Upgrade to Silver or Gold Membership URL
/*1048*/ uint32 exit_url_length2;	// Length of the exit_url2 string (0 for none)
/*0000*/ //char exit_url2[49];		// Upgrade to Gold Membership URL
};

struct Membership_Struct
{
/*000*/ uint32 membership;	// Seen 2 on Gold Account
/*004*/ uint32 races;	// Seen ff ff 01 00
/*008*/ uint32 classes;	// Seen ff ff 01 01
/*012*/ uint32 entrysize; // Seen 22
/*016*/ int32 entries[22]; // Most -1, 1, and 0 for Gold Status
/*104*/
};


/*
** Generic Spawn Struct
** Length: 897 Octets
** Used in:
**   spawnZoneStruct
**   dbSpawnStruct
**   petStruct
**   newSpawnStruct
*/
/*
showeq -> eqemu
sed -e 's/_t//g' -e 's/seto_0xFF/set_to_0xFF/g'
*/

struct Spawn_Struct_Bitfields
{
/*00*/	unsigned   gender:2;		// Gender (0=male, 1=female, 2=monster)
/*02*/	unsigned   ispet:1;			// Guessed based on observing live spawns
/*03*/	unsigned   afk:1;			// 0=no, 1=afk
/*04*/	unsigned   anon:2;			// 0=normal, 1=anon, 2=roleplay
/*06*/	unsigned   gm:1;
/*06*/	unsigned   sneak:1;
/*08*/	unsigned   lfg:1;
/*09*/	unsigned   unknown09:1;
/*10*/	unsigned   invis:12;		// there are 3000 different (non-GM) invis levels
/*22*/	unsigned   linkdead:1;		// 1 Toggles LD on or off after name. Correct for RoF
/*23*/	unsigned   showhelm:1;
/*24*/	unsigned   unknown24:1;		// Prefixes name with !
/*25*/	unsigned   trader:1;
/*26*/	unsigned   unknown26:1;
/*27*/	unsigned   targetable:1;
/*28*/	unsigned   targetable_with_hotkey:1;
/*29*/	unsigned   showname:1;
/*30*/	unsigned   unknown30:1;
/*30*/	unsigned   untargetable:1;	// Untargetable with mouse
	/*
	// Unknown in RoF
	unsigned   betabuffed:1;
	unsigned   buyer:1;
	unsigned   buyer:1;
	*/
};

struct Spawn_Struct_Position
{
/*000*/	signed	padding0000:12;
	signed	y:19;
	signed	padding0001:1;

/*004*/	signed  deltaX:13;      // change in x
	signed  deltaHeading:10;// change in heading
	signed  padding0008:9;

/*008*/	signed	deltaY:13;
	signed	z:19;

/*012*/	signed	x:19;
	signed  animation:10;   // animation
	signed  padding0016:3;

/*004*/	signed	heading:12;
	signed	deltaZ:13;      // change in z
	signed	padding0020:7;
};

/*
struct Spawn_Struct_Position
{
	signed   padding0000:12; // ***Placeholder
         signed   deltaX:13;      // change in x
         signed   padding0005:7;  // ***Placeholder
 signed   deltaHeading:10;// change in heading
         signed   deltaY:13;      // change in y
         signed   padding0006:9;  // ***Placeholder
 signed   y:19;           // y coord
         signed   animation:13;   // animation
 unsigned heading:12;     // heading
         signed   x:19;           // x coord
         signed   padding0014:1;  // ***Placeholder
 signed   z:19;           // z coord
         signed   deltaZ:13;      // change in z
}; */

struct Spawn_Struct
{
// Note this struct is not used as such, it is here for reference. As the struct is variable sized, the packets
// are constructed in Live.cpp
//
/*0000*/ char     name[1];	//name[64];
/*0000*/ //uint8     nullterm1; // hack to null terminate name
/*0064*/ uint32 spawnId;
/*0068*/ uint8  level;
/*0069*/ float  bounding_radius; // used in melee, overrides calc
/*0073*/ uint8  NPC;           // 0=player,1=npc,2=pc corpse,3=npc corpse
	 Spawn_Struct_Bitfields	Bitfields;
/*0000*/ uint8  otherData; // & 4 - has title, & 8 - has suffix, & 1 - it's a chest or untargetable
/*0000*/ float unknown3;	// seen -1
/*0000*/ float unknown4;
/*0000*/ float size;
/*0000*/ uint8  face;
/*0000*/ float    walkspeed;
/*0000*/ float    runspeed;
/*0000*/ uint32 race;
/*0000*/ uint8  showname; // for body types - was charProperties
/*0000*/ uint32 bodytype;
/*0000*/ //uint32 bodytype2;      // this is only present if charProperties==2
                                  // are there more than two possible properties?
/*0000*/ uint8  curHp;
/*0000*/ uint8  haircolor;
/*0000*/ uint8  beardcolor;
/*0000*/ uint8  eyecolor1;
/*0000*/ uint8  eyecolor2;
/*0000*/ uint8  hairstyle;
/*0000*/ uint8  beard;
/*0000*/ uint32 drakkin_heritage;
/*0000*/ uint32 drakkin_tattoo;
/*0000*/ uint32 drakkin_details;
/*0000*/ uint8  statue;				// was holding
/*0000*/ uint32 deity;
/*0000*/ uint32 guildID;
/*0000*/ uint32 guildrank;			// 0=member, 1=officer, 2=leader, -1=not guilded
/*0000*/ uint8  class_;
/*0000*/ uint8  pvp;					// 0 = normal name color, 2 = PVP name color
/*0000*/ uint8  StandState;			// stand state - 0x64 for normal animation
/*0000*/ uint8  light;
/*0000*/ uint8  flymode;
/*0000*/ uint8  equip_chest2;
/*0000*/ uint8  unknown9;
/*0000*/ uint8  unknown10;
/*0000*/ uint8  helm;
/*0000*/ char     lastName[1];
/*0000*/ //uint8     lastNameNull; //hack!
/*0000*/ uint32 aatitle;		// 0=none, 1=general, 2=archtype, 3=class was AARank
/*0000*/ uint8  unknown12;
/*0000*/ uint32 petOwnerId;
/*0000*/ uint8  unknown13;
/*0000*/ uint32 PlayerState;		// Stance 64 = normal 4 = aggressive 40 = stun/mezzed
/*0000*/ uint32 unknown15;
/*0000*/ uint32 unknown16;
/*0000*/ uint32 unknown17;
/*0000*/ //uint8  unknownRoF3;
/*0000*/ uint32 unknown18;
/*0000*/ uint32 unknown19;
	 Spawn_Struct_Position Position;
/*0000*/ TintProfile equipment_tint;

// skip these bytes if not a valid player race
/*0000*/ union
         {
           struct
           {
			   /*0000*/ Texture_Struct equip_helmet;     // Equiptment: Helmet visual
			   /*0000*/ Texture_Struct equip_chest;      // Equiptment: Chest visual
			   /*0000*/ Texture_Struct equip_arms;       // Equiptment: Arms visual
			   /*0000*/ Texture_Struct equip_bracers;    // Equiptment: Wrist visual
			   /*0000*/ Texture_Struct equip_hands;      // Equiptment: Hands visual
			   /*0000*/ Texture_Struct equip_legs;       // Equiptment: Legs visual
			   /*0000*/ Texture_Struct equip_feet;       // Equiptment: Boots visual
			   /*0000*/ Texture_Struct equip_primary;    // Equiptment: Main visual
			   /*0000*/ Texture_Struct equip_secondary;  // Equiptment: Off visual
           } equip;
		   /*0000*/ Texture_Struct equipment[9];
         };

/*0000*/ //char title[0];  // only read if(hasTitleOrSuffix & 4)
/*0000*/ //char suffix[0]; // only read if(hasTitleOrSuffix & 8)
	 char unknown20[8];
	 uint8 IsMercenary;	// If NPC == 1 and this == 1, then the NPC name is Orange.
/*0000*/ char unknown21[55];
};


/*
** Generic Spawn Struct
** Fields from old struct not yet found:
**	uint8	traptype;	// 65 is disarmable trap, 66 and 67 are invis triggers/traps
**	uint8	is_pet;		// 0=no, 1=yes
**	uint8	afk;		// 0=no, 1=afk
**	uint8	is_npc;		// 0=no, 1=yes
**	uint8	max_hp;		// (name prolly wrong)takes on the value 100 for players, 100 or 110 for NPCs and 120 for PC corpses...
**	uint8	guildrank;	// 0=normal, 1=officer, 2=leader
**	uint8	eyecolor2;	//not sure, may be face
**	uint8	aaitle;		// 0=none, 1=general, 2=archtype, 3=class
*/

/*
** New Spawn
** Length: 176 Bytes
** OpCode: 4921
*/
struct NewSpawn_Struct
{
	struct Spawn_Struct spawn;	// Spawn Information
};


/*
** Client Zone Entry struct
** Length: 68 Octets
** OpCode: ZoneEntryCode (when direction == client)
*/
struct ClientZoneEntry_Struct {
/*00*/ uint32	unknown00;	// ***Placeholder
/*04*/ char	char_name[64];	// Player firstname [32]
/*68*/ uint32	unknown68;
/*72*/ uint32	unknown72;
};


/*
** Server Zone Entry Struct
** Length: 452 Bytes
** OPCodes: OP_ServerZoneEntry
**
*/
struct ServerZoneEntry_Struct //Adjusted from SEQ Everquest.h Struct
{
	struct NewSpawn_Struct player;
};


//New Zone Struct - Size: 948
struct NewZone_Struct {
/*0000*/	char	char_name[64];			// Character Name
/*0064*/	char	zone_short_name[32];	// Zone Short Name
/*0096*/	char    unknown0096[96];
/*0192*/	char	zone_long_name[278];	// Zone Long Name
/*0470*/	uint8	ztype;					// Zone type (usually FF)
/*0471*/	uint8	fog_red[4];				// Zone fog (red)
/*0475*/	uint8	fog_green[4];				// Zone fog (green)
/*0479*/	uint8	fog_blue[4];				// Zone fog (blue)
/*0483*/	uint8	unknown323;
/*0484*/	float	fog_minclip[4];
/*0500*/	float	fog_maxclip[4];
/*0516*/	float	gravity;
/*0520*/	uint8	time_type;
/*0521*/    uint8   rain_chance[4];
/*0525*/    uint8   rain_duration[4];
/*0529*/    uint8   snow_chance[4];
/*0533*/    uint8   snow_duration[4];
/*0537*/    uint8   unknown537[33];
/*0570*/	uint8	sky;					// Sky Type
/*0571*/	uint8	unknown571[13];			// ***Placeholder
/*0584*/	float	zone_exp_multiplier;	// Experience Multiplier
/*0588*/	float	safe_y;					// Zone Safe Y
/*0592*/	float	safe_x;					// Zone Safe X
/*0596*/	float	safe_z;					// Zone Safe Z
/*0600*/	float	min_z;					// Guessed - NEW - Seen 0
/*0604*/	float	max_z;					// Guessed
/*0608*/	float	underworld;				// Underworld, min z (Not Sure?)
/*0612*/	float	minclip;				// Minimum View Distance
/*0616*/	float	maxclip;				// Maximum View DIstance
/*0620*/	uint8	unknown620[84];		// ***Placeholder
/*0704*/	char	zone_short_name2[96];	//zone file name? excludes instance number which can be in previous version.
/*0800*/	int32	unknown800;	//seen -1
/*0804*/	char	unknown804[40]; //
/*0844*/	int32  unknown844;	//seen 600
/*0848*/	int32  unknown848;
/*0852*/	uint16 zone_id;
/*0854*/	uint16 zone_instance;
/*0856*/	uint32 scriptNPCReceivedanItem;
/*0860*/	uint32 bCheck;					// padded bool
/*0864*/	uint32 scriptIDSomething;
/*0868*/	uint32 underworld_teleport_index; // > 0 teleports w/ zone point index, invalid succors, -1 affects some collisions
/*0872*/	uint32 scriptIDSomething3;
/*0876*/	uint32 suspend_buffs;
/*0880*/	uint32 lava_damage;		// Seen 50
/*0884*/	uint32 min_lava_damage;		// Seen 10
/*0888*/	uint8  unknown888;		// Seen 1
/*0889*/	uint8  unknown889;		// Seen 0 (POK) or 1 (rujj)
/*0890*/	uint8  unknown890;		// Seen 1
/*0891*/	uint8  unknown891;		// Seen 0
/*0892*/	uint8  unknown892;		// Seen 0
/*0893*/	uint8  unknown893;		// Seen 0 - 00
/*0894*/	uint8  fall_damage;	// 0 = Fall Damage on, 1 = Fall Damage off
/*0895*/	uint8  unknown895;		// Seen 0 - 00
/*0896*/	uint32 fast_regen_hp;		// Seen 180
/*0900*/	uint32 fast_regen_mana;		// Seen 180
/*0904*/	uint32 fast_regen_endurance;		// Seen 180
/*0908*/	uint32 unknown908;		// Seen 2
/*0912*/	uint32 unknown912;		// Seen 2
/*0916*/	float  FogDensity;		// Most zones have this set to 0.33 Blightfire had 0.16
/*0920*/	uint32 unknown920;		// Seen 0
/*0924*/	uint32 unknown924;		// Seen 0
/*0928*/	uint32 unknown928;		// Seen 0
/*0932*/	int32  unknown932;		// Seen -1
/*0936*/	int32  unknown936;		// Seen -1
/*0940*/	uint32 unknown940;		// Seen 0
/*0944*/	float  unknown944;		// Seen 1.0
/*0948*/
};


/*
** Memorize Spell Struct
** Length: 16 Bytes
**
*/
struct MemorizeSpell_Struct {
uint32 slot;     // Spot in the spell book/memorized slot
uint32 spell_id; // Spell id (200 or c8 is minor healing, etc)
uint32 scribing; // 1 if memorizing a spell, set to 0 if scribing to book, 2 if un-memming
uint32 reduction; // lowers reuse
};

/*
** Make Charmed Pet
** Length: 12 Bytes
**
*/
struct Charm_Struct {
/*00*/	uint32	owner_id;
/*04*/	uint32	pet_id;
/*08*/	uint32	command;    // 1: make pet, 0: release pet
/*12*/
};

struct InterruptCast_Struct
{
	uint32 spawnid;
	uint32 messageid;
	//char	message[0];
};

struct DeleteSpell_Struct
{
/*000*/int16	spell_slot;
/*002*/uint8	unknowndss002[2];
/*004*/uint8	success;
/*005*/uint8	unknowndss006[3];
/*008*/
};

struct ManaChange_Struct
{
/*00*/	uint32	new_mana;		// New Mana AMount
/*04*/	uint32	stamina;
/*08*/	uint32	spell_id;
/*12*/	uint8	keepcasting;	// won't stop the cast. Change mana while casting?
/*13*/	uint8	padding[3];		// client doesn't read it, garbage data seems like
/*16*/	int32	slot;			// -1 for normal usage slot for when we want silent interrupt? I think it does timer stuff or something. Linked Spell Reuse interrupt uses it
};

struct SwapSpell_Struct
{
	uint32 from_slot;
	uint32 to_slot;
};

struct BeginCast_Struct
{
/*000*/	uint32	spell_id;
/*004*/	uint16	caster_id;
/*006*/	uint32	cast_time;		// in miliseconds
/*010*/
};

struct CastSpell_Struct
{
/*00*/	uint32	slot;
/*04*/	uint32	spell_id;
/*08*/	InventorySlot_Struct inventory_slot;  // slot for clicky item, Seen unknown of 131 = normal cast
/*20*/	uint32	target_id;
/*24*/	uint32	cs_unknown[2];
/*32*/	float	y_pos;
/*36*/	float	x_pos;
/*40*/	float	z_pos;
/*44*/
};

/*
** SpawnAppearance_Struct
** Changes client appearance for all other clients in zone
** Size: 8 bytes
** Used in: OP_SpawnAppearance
**
*/
struct SpawnAppearance_Struct
{
/*0000*/ uint16 spawn_id;          // ID of the spawn
/*0002*/ uint16 type;              // Values associated with the type
/*0004*/ uint32 parameter;         // Type of data sent
/*0008*/
};

struct SpellBuff_Struct
{
/*000*/	uint8 effect_type;		// 0 = no buff, 2 = buff, 4 = inverse affects of buff
/*001*/	uint8 level;			// Seen 1 for no buff
/*002*/	uint8 unknown002;		//pretty sure padding now
/*003*/	uint8 unknown003;   	// MQ2 used to call this "damage shield" -- don't see client referencing it, so maybe server side DS type tracking?
/*004*/	float bard_modifier;
/*008*/	uint32 spellid;
/*012*/	uint32 duration;
/*016*/	uint32 player_id;		// caster ID, pretty sure just zone ID
/*020*/	uint32 num_hits;
/*024*/	float y;				// referenced by SPA 441
/*028*/	float x;				// unsure if all buffs get them
/*032*/	float z;				// as valid data
/*036*/	uint32 unknown036;
/*040*/	int32 slot_data[12];	// book keeping stuff per slot (counters, rune/vie)
/*088*/
};

struct SpellBuffPacket_Struct {
/*000*/	uint32 entityid;		// Player id who cast the buff
/*004*/	SpellBuff_Struct buff;
/*092*/	uint32 slotid;
/*096*/	uint32 bufffade;
/*100*/
};

struct BuffRemoveRequest_Struct
{
/*00*/ uint32 SlotID;
/*04*/ uint32 EntityID;
/*08*/
};

#if 0
// not in use
struct BuffIconEntry_Struct {
/*000*/ uint32 buff_slot;
/*004*/ uint32 spell_id;
/*008*/ uint32 tics_remaining;
/*012*/ uint32 num_hits;
// char name[0]; caster name is also here sometimes
// uint8  unknownend; 1 when single, 0 when all opposite of all_buffs?
};

// not in use
struct BuffIcon_Struct {
/*000*/ uint32 entity_id;
/*004*/ uint32 unknown004;
/*008*/ uint8  all_buffs; // 1 when updating all buffs, 0 when doing one
/*009*/ uint16 count;
/*011*/ BuffIconEntry_Struct entires[0];
};
#endif

struct GMTrainee_Struct
{
	/*000*/ uint32 npcid;
	/*004*/ uint32 playerid;
	/*008*/ uint32 skills[PACKET_SKILL_ARRAY_SIZE];
	/*408*/ uint8 unknown408[40];
	/*448*/
};

struct GMTrainEnd_Struct
{
	/*000*/ uint32 npcid;
	/*004*/ uint32 playerid;
	/*008*/
};

struct GMSkillChange_Struct {
/*000*/	uint16		npcid;
/*002*/ uint8		unknown1[2];    // something like PC_ID, but not really. stays the same thru the session though
/*002*/ uint16       skillbank;      // 0 if normal skills, 1 if languages
/*002*/ uint8		unknown2[2];
/*008*/ uint16		skill_id;
/*010*/ uint8		unknown3[2];
/*012*/
};

struct GMTrainSkillConfirm_Struct {	// SoF+ only
/*000*/	uint32	SkillID;
/*004*/	uint32	Cost;
/*008*/	uint8	NewSkill;	// Set to 1 for 'You have learned the basics' message.
/*009*/	char	TrainerName[64];
/*073*/ uint8	Unknown073[3];
/*076*/
};

struct ConsentResponse_Struct {
	char grantname[64];
	char ownername[64];
	uint8 permission;
	char zonename[64];
};

/*
** Name Generator Struct
** Length: 72 bytes
** OpCode: 0x0290
*/
struct NameGeneration_Struct
{
/*0000*/	uint32	race;
/*0004*/	uint32	gender;
/*0008*/	char	name[64];
/*0072*/
};

/*
** Character Creation struct
** Length: 96 Bytes
*/
struct CharCreate_Struct
{
/*0000*/	uint32	gender;
/*0004*/	uint32	race;
/*0008*/	uint32	class_;
/*0012*/	uint32	deity;
/*0016*/	uint32	start_zone;
/*0020*/	uint32	haircolor;
/*0024*/	uint32	beard;
/*0028*/	uint32	beardcolor;
/*0032*/	uint32	hairstyle;
/*0036*/	uint32	face;
/*0040*/	uint32	eyecolor1;
/*0044*/	uint32	eyecolor2;
/*0048*/	uint32	drakkin_heritage;
/*0052*/	uint32	drakkin_tattoo;
/*0056*/	uint32	drakkin_details;
/*0060*/	uint32	STR;
/*0064*/	uint32	STA;
/*0068*/	uint32	AGI;
/*0073*/	uint32	DEX;
/*0076*/	uint32	WIS;
/*0080*/	uint32	INT;
/*0084*/	uint32	CHA;
/*0088*/	uint32	tutorial;
/*0092*/	uint32	unknown0092;
/*0096*/
};

/*
** Character Creation struct
** Length: 0 Bytes
** OpCode: 0x
*/
struct CharCreate_Struct_Temp //Size is now 0
{
};

/*
 *Used in PlayerProfile
 */
struct AA_Array
{
	uint32 AA;
	uint32 value;
	uint32 charges;	// expendable charges
};

struct Disciplines_Struct {
	uint32 values[MAX_PP_DISCIPLINES];
};



struct Tribute_Struct {
	uint32 tribute;
	uint32 tier;
};

// Bandolier item positions
enum
{
	bandolierPrimary = 0,
	bandolierSecondary,
	bandolierRange,
	bandolierAmmo
};

struct BandolierItem_Struct
{
	char Name[1];	// Variable Length
	uint32 ID;
	uint32 Icon;
};

//len = 72
struct BandolierItem_Struct_Old
{
	uint32 ID;
	uint32 Icon;
	char Name[64];
};

//len = 320
struct Bandolier_Struct
{
	char Name[1];	// Variable Length
	BandolierItem_Struct Items[profile::BANDOLIER_ITEM_COUNT];
};

struct Bandolier_Struct_Old
{
	char Name[32];
	BandolierItem_Struct Items[profile::BANDOLIER_ITEM_COUNT];
};

struct PotionBeltItem_Struct
{
	char Name[1];	// Variable Length
	uint32 ID;
	uint32 Icon;
};

//len = 72
struct PotionBeltItem_Struct_Old
{
	uint32 ID;
	uint32 Icon;
	char Name[64];
};

struct PotionBelt_Struct
{
	PotionBeltItem_Struct Items[profile::POTION_BELT_SIZE];
};

struct PotionBelt_Struct_Old
{
	PotionBeltItem_Struct_Old Items[profile::POTION_BELT_SIZE];
};

struct GroupLeadershipAA_Struct {
	union {
		struct {
			uint32 groupAAMarkNPC;
			uint32 groupAANPCHealth;
			uint32 groupAADelegateMainAssist;
			uint32 groupAADelegateMarkNPC;
			uint32 groupAA4;
			uint32 groupAA5;
			uint32 groupAAInspectBuffs;
			uint32 groupAA7;
			uint32 groupAASpellAwareness;
			uint32 groupAAOffenseEnhancement;
			uint32 groupAAManaEnhancement;
			uint32 groupAAHealthEnhancement;
			uint32 groupAAHealthRegeneration;
			uint32 groupAAFindPathToPC;
			uint32 groupAAHealthOfTargetsTarget;
			uint32 groupAA15;
		};
		uint32 ranks[MAX_GROUP_LEADERSHIP_AA_ARRAY];
	};
};

struct RaidLeadershipAA_Struct {
	union {
		struct {
			uint32 raidAAMarkNPC;
			uint32 raidAANPCHealth;
			uint32 raidAADelegateMainAssist;
			uint32 raidAADelegateMarkNPC;
			uint32 raidAA4;
			uint32 raidAA5;
			uint32 raidAA6;
			uint32 raidAASpellAwareness;
			uint32 raidAAOffenseEnhancement;
			uint32 raidAAManaEnhancement;
			uint32 raidAAHealthEnhancement;
			uint32 raidAAHealthRegeneration;
			uint32 raidAAFindPathToPC;
			uint32 raidAAHealthOfTargetsTarget;
			uint32 raidAA14;
			uint32 raidAA15;
		};
		uint32 ranks[MAX_RAID_LEADERSHIP_AA_ARRAY];
	};
};

struct LeadershipAA_Struct {
	union {
		struct {
			GroupLeadershipAA_Struct group;
			RaidLeadershipAA_Struct raid;
		};
		uint32 ranks[MAX_LEADERSHIP_AA_ARRAY];
	};
};

 /**
* A bind point.
* Size: 20 Octets
*/
struct BindStruct {
   /*000*/ uint32 zone_id;
   /*004*/ float x;
   /*008*/ float y;
   /*012*/ float z;
   /*016*/ float heading;
   /*020*/
};


// Player Profile - Variable Length as of Oct 12 2012 patch
struct PlayerProfile_Struct
{
/*00000*/ uint32 checksum;				//
/*00004*/ uint32 checksum_size;			// Value = ( Packet Size - 9 )
/*00008*/ uint8 checksum_byte;			//
/*00009*/ uint8 unknown_rof1[3];		//
/*00012*/ uint32 unknown_rof2;			//
/*00016*/ uint8  gender;				// Player Gender - 0 Male, 1 Female
/*00017*/ uint32  race;					// Player race
/*00021*/ uint8  class_;				// Player class
/*00022*/ uint8   level;				// Level of player
/*00023*/ uint8   level1;				// Level of player (again?)
/*00024*/ uint32 bind_count;	// Seen 5
/*00028*/ BindStruct binds[5];			// Bind points (primary is first) 5 fields = 100 bytes
/*00128*/ uint32  deity;				// deity
/*00132*/ uint32 unknown_rof3;					// Maybe Drakkin Heritage?
/*00136*/ uint32 unknown4_count;		// Seen 10
/*00140*/ uint32 unknown_rof4[10];	// Seen all 0s
/*00180*/ uint32 equip_count;	// Seen 22
union
{
	struct
	{
		/*00184*/ Texture_Struct equip_helmet; // Equiptment: Helmet visual
		/*00204*/ Texture_Struct equip_chest; // Equiptment: Chest visual
		/*00224*/ Texture_Struct equip_arms; // Equiptment: Arms visual
		/*00244*/ Texture_Struct equip_bracers; // Equiptment: Wrist visual
		/*00264*/ Texture_Struct equip_hands; // Equiptment: Hands visual
		/*00284*/ Texture_Struct equip_legs; // Equiptment: Legs visual
		/*00304*/ Texture_Struct equip_feet; // Equiptment: Boots visual
		/*00324*/ Texture_Struct equip_primary; // Equiptment: Main visual
		/*00344*/ Texture_Struct equip_secondary; // Equiptment: Off visual
		// Below slots are just guesses, but all 0s anyway...
		/*00364*/ Texture_Struct equip_charm; // Equiptment: Non-visual
		/*00384*/ Texture_Struct equip_ear1; // Equiptment: Non-visual
		/*00404*/ Texture_Struct equip_ear2; // Equiptment: Non-visual
		/*00424*/ Texture_Struct equip_face; // Equiptment: Non-visual
		/*00444*/ Texture_Struct equip_neck; // Equiptment: Non-visual
		/*00464*/ Texture_Struct equip_shoulder; // Equiptment: Non-visual
		/*00484*/ Texture_Struct equip_bracer2; // Equiptment: Non-visual
		/*00504*/ Texture_Struct equip_range; // Equiptment: Non-visual
		/*00524*/ Texture_Struct equip_ring1; // Equiptment: Non-visual
		/*00544*/ Texture_Struct equip_ring2; // Equiptment: Non-visual
		/*00564*/ Texture_Struct equip_waist; // Equiptment: Non-visual
		/*00584*/ Texture_Struct equip_powersource; // Equiptment: Non-visual
		/*00604*/ Texture_Struct equip_ammo; // Equiptment: Non-visual
	} equip;
	/*00184*/ Texture_Struct equipment[22];
};
/*00624*/ uint32 equip2_count;			// Seen 9
/*00628*/ Texture_Struct equipment2[9];	// Appears to be Visible slots, but all 0s
/*00808*/ uint32 tint_count;			// Seen 9
/*00812*/ TintProfile item_tint;		// RR GG BB 00
/*00848*/ uint32 tint_count2;			// Seen 9
/*00852*/ TintProfile item_tint2;		// RR GG BB 00
/*00888*/ uint8   haircolor;			// Player hair color
/*00889*/ uint8   beardcolor;			// Player beard color
/*00890*/ uint32 unknown_rof5;			//
/*00894*/ uint8   eyecolor1;			// Player left eye color
/*00895*/ uint8   eyecolor2;			// Player right eye color
/*00896*/ uint8   hairstyle;			// Player hair style
/*00897*/ uint8   beard;				// Player beard type
/*00898*/ uint8   face;					// Player face
/*00899*/ uint32 drakkin_heritage;		//
/*00903*/ uint32 drakkin_tattoo;		//
/*00907*/ uint32 drakkin_details;		//
/*00911*/ uint8 unknown_rof6;			//
/*00912*/ int8 unknown_rof7;			// seen -1
/*00913*/ uint8 unknown_rof8;			//
/*00914*/ uint8 unknown_rof9;			//
/*00915*/ uint8 unknown_rof10;			// Seen 1 or 0 (on a female?)
/*00916*/ float height;					// Seen 7.0 (barb), 5.0 (woodelf), 5.5 (halfelf)
/*00920*/ float unknown_rof11;			// Seen 3.0
/*00924*/ float unknown_rof12;			// Seen 2.5
/*00928*/ float unknown_rof13;			// Seen 5.5
/*00932*/ uint32 primary;				// Seen 10528
/*00936*/ uint32 secondary;				// Seen 10006
/*00940*/ uint32  points;				// Unspent Practice points - RELOCATED???
/*00944*/ uint32  mana;					// Current mana
/*00948*/ uint32  cur_hp;				// Current HP without +HP equipment
/*00952*/ uint32  STR;					// Strength - 6e 00 00 00 - 110
/*00956*/ uint32  STA;					// Stamina - 73 00 00 00 - 115
/*00960*/ uint32  CHA;					// Charisma - 37 00 00 00 - 55
/*00964*/ uint32  DEX;					// Dexterity - 50 00 00 00 - 80
/*00968*/ uint32  INT;					// Intelligence - 3c 00 00 00 - 60
/*00972*/ uint32  AGI;					// Agility - 5f 00 00 00 - 95
/*00976*/ uint32  WIS;					// Wisdom - 46 00 00 00 - 70
/*00980*/ uint32 unknown_rof14[7];				// Probably base resists?
/*01008*/ uint32 aa_count;						// Seen 300
/*01012*/ AA_Array  aa_array[MAX_PP_AA_ARRAY];	// [300] 3600 bytes - AAs 12 bytes each
/*04612*/ uint32 skill_count;					// Seen 100
/*04616*/ uint32 skills[MAX_PP_SKILL];			// [100] 400 bytes - List of skills
/*05016*/ uint32 InnateSkills_count;				// Seen 25
/*05020*/ uint32 InnateSkills[MAX_PP_INNATE_SKILL];	// Most are 255 or 0
/*05120*/ uint32 discipline_count;				// Seen 200
/*05124*/ Disciplines_Struct  disciplines;		// [200] 800 bytes Known disciplines
/*05924*/ uint32 timestamp_count;				// Seen 20
/*05928*/ uint32 timestamps[20];				// Unknown Unix Timestamps - maybe recast related?
/*06008*/ uint32 recast_count;					// Seen 20
/*06012*/ uint32 recastTimers[MAX_RECAST_TYPES];// [20] 80 bytes - Timers (UNIX Time of last use)
/*06092*/ uint32 timestamp2_count;				// Seen 100
/*06096*/ uint32 timestamps2[100];				// Unknown Unix Timestamps - maybe Skill related?
/*06496*/ uint32 spell_book_count;				// Seen 720
/*06500*/ uint32 spell_book[spells::SPELLBOOK_SIZE];	// List of the Spells in spellbook 720 = 90 pages [2880 bytes]
/*09380*/ uint32 mem_spell_count;				// Seen 16
/*09384*/ int32 mem_spells[spells::SPELL_GEM_COUNT];	// [16] List of spells memorized - First 12 are set or -1 and last 4 are 0s
/*09448*/ uint32 unknown16_count;				// Seen 13
/*09452*/ uint32 unknown_rof16[13];				// Possibly spell or buff related
/*09504*/ uint8 unknown_rof17;					// Seen 0 or 8
/*09505*/ uint32 buff_count;					// Seen 42
/*09509*/ SpellBuff_Struct buffs[BUFF_COUNT];	// [42] 3360 bytes - Buffs currently on the player (42 Max) - (Each Size 80)
/*12869*/ uint32 platinum;				// Platinum Pieces on player
/*12873*/ uint32 gold;					// Gold Pieces on player
/*12877*/ uint32 silver;				// Silver Pieces on player
/*12881*/ uint32 copper;				// Copper Pieces on player
/*12885*/ uint32 platinum_cursor;		// Platinum Pieces on cursor
/*12889*/ uint32 gold_cursor;			// Gold Pieces on cursor
/*12893*/ uint32 silver_cursor;			// Silver Pieces on cursor
/*12897*/ uint32 copper_cursor;			// Copper Pieces on cursor
/*12901*/ uint32 intoxication;			// Alcohol level (in ticks till sober?) - Position Guessed
/*12905*/ uint32 toxicity;				// Potion Toxicity (15=too toxic, each potion adds 3) - Position Guessed
/*12909*/ uint32 unknown_rof19;			//
/*12913*/ uint32 thirst_level;			// Drink (ticks till next drink) - Position Guessed
/*12917*/ uint32 hunger_level;			// Food (ticks till next eat) - Position Guessed
/*12921*/ uint32 aapoints_spent;		// Number of spent AA points
/*12925*/ uint32 aapoint_count;			// Seen 5 - Unsure what this is exactly
/*12929*/ uint32 aapoints_assigned;		// Number of Assigned AA points - Seen 206 (total of the 4 fields below)
/*12933*/ uint32 aa_spent_general;		// Seen 63
/*12937*/ uint32 aa_spent_archetype;	// Seen 40
/*12941*/ uint32 aa_spent_class;		// Seen 103
/*12945*/ uint32 aa_spent_special;		// Seen 0
/*12949*/ uint32 aapoints;				// Unspent AA points - Seen 1
/*12953*/ uint16 unknown_rof20;			//
/*12955*/ uint32 bandolier_count;		// Seen 20
/*12959*/ Bandolier_Struct bandoliers[profile::BANDOLIERS_SIZE]; // [20] 740 bytes (Variable Name Sizes) - bandolier contents
/*13699*/ uint32 potionbelt_count;		// Seen 5
/*13703*/ PotionBelt_Struct potionbelt;	// [5] 45 bytes potion belt - (Variable Name Sizes)
/*13748*/ int32 unknown_rof21;			// Seen -1
/*13752*/ int32 hp_total;				// Guessed - Seen 20 on level 1 new mage
/*13756*/ int32 endurance_total;		// Guessed - Seen 20 on level 1 new mage
/*13760*/ int32 mana_total;			// Guessed - Only seen on casters so far - Matches above field if caster
/*13764*/ uint32 unknown_rof22[12];		// Same for all seen PPs - 48 bytes:
/*
19 00 00 00 19 00 00 00 19 00 00 00 0f 00 00 00
0f 00 00 00 0f 00 00 00 0f 00 00 00 1f 85 eb 3e
33 33 33 3f 08 00 00 00 02 00 00 00 01 00 00 00
*/
/*13812*/ uint32 unknown_rof23;			// Seen 5, 19, and 20 in examples
/*13816*/ uint32 unknown_rof24[4];		//
/*13832*/ uint32 unknown_rof25[2];		// Seen random numbers from 0 to 2165037
/*13840*/ uint32 unknown_rof26;			// Seen 106
//END SUB-STRUCT used for shrouding.
/*13844*/ uint32 name_str_len;			// Seen 64
/*13848*/ char   name[64];				// Name of player - 19960 for Live 1180 difference
/*13912*/ uint32 last_name_str_len;		// Seen 32
/*13916*/ char   last_name[32];			// Last name of player
/*13948*/ uint32 birthday;				// character birthday
/*13952*/ uint32 account_startdate;		// Date the Account was started
/*13956*/ uint32 lastlogin;				// character last save time
/*13960*/ uint32 timePlayedMin;			// time character played
/*13964*/ uint32 timeentitledonaccount;
/*13968*/ uint32 expansions;			// Bitmask for expansions ff 7f 00 00 - SoD
/*13972*/ uint32 language_count;		// Seen 32
/*13976*/ uint8 languages[MAX_PP_LANGUAGE];	// [32] 32 bytes - List of languages
/*14008*/ uint16 zone_id;				// see zones.h
/*14010*/ uint16 zoneInstance;			// Instance id
/*14012*/ float  y;                 	// Players y position (NOT positive about this switch)
/*14016*/ float  x;                 	// Players x position
/*14020*/ float  z;                 	// Players z position
/*14024*/ float  heading;           	// Players heading
/*14028*/ uint32 air_remaining;			// Air supply (seconds)
/*14032*/ int32 unknown_rof28;			// Seen -1
/*14036*/ uint8 unknown_rof29[10];		//
/*14046*/ uint32 unknown_rof30;			// Random large number or 0
/*14050*/ uint32 unknown_rof31;			//
/*14054*/ uint32 unknown32_count;		// Seen 5
/*14058*/ uint8 unknown_rof32[29];		//
/*14087*/ uint32 unknown33_count;		// Seen 32 for melee/hybrid and 21 for druid, 34 for mage
/*14091*/ uint32 unknown_rof33[64];		// Struct contains 2 ints, so double 32 count (Variable Sized)
// Position Varies after this point - Really starts varying at Bandolier names, but if no names set it starts here
/*00000*/ int32 unknown_rof34;			// Seen -1
/*00000*/ int32 unknown_rof35;			// Seen -1
/*00000*/ uint8 unknown_rof36[18];		//
/*00000*/ uint32 unknown37_count;		// Seen 5
/*00000*/ int32 unknown_rof37[10];		// Alternates -1 and 0 - Struct contains 2 ints
/*00000*/ uint32 unknown38_count;		// Seen 10
/*00000*/ int32 unknown_rof38[20];		// Alternates -1 and 0 - Struct contains 2 ints
/*00000*/ uint8 unknown_rof39[137];		//
/*00000*/ float unknown_rof40;			// Seen 1.0
/*00000*/ uint32 unknown_rof41[9];		//
/*00000*/ uint32 unknown_rof42;			// Seen 0 or 1
/*00000*/ uint32 unknown_string1_count;	// Seen 64
/*00000*/ char unknown_string1[64];		//
/*00000*/ uint8 unknown_rof43[30];		//
/*00000*/ uint32 unknown_string2_count;	// Seen 64
/*00000*/ char unknown_string2[64];		//
/*00000*/ uint32 unknown_string3_count;	// Seen 64
/*00000*/ char unknown_string3[64];		//
/*00000*/ uint32 unknown_rof44;			// Seen 0 or 50
/*00000*/ uint8 unknown_rof45[663];		//
/*00000*/ uint32 char_id;				// Guessed based on char creation date and values
/*00000*/ uint8 unknown_rof46;			// Seen 0 or 1
/*00000*/ uint32 unknown_rof47;			// Seen 6
/*00000*/ uint32 unknown_rof48[13];		// Seen all 0s or some mix of ints and float?
/*00000*/ uint32 unknown_rof49;			// Seen 64
/*00000*/ uint8 unknown_rof50[256];		// Seen mostly 0s, but one example had a 2 in the middle
/*00000*/ uint32 unknown_rof51;			// Seen 100 or 0
/*00000*/ uint8 unknown_rof52[82];		//
/*00000*/ uint32 unknown_rof53;			// Seen 50

uint8 unknown_rof54[1325];	// Unknown Section

// Bottom of Struct:
/*00000*/ uint8  groupAutoconsent;		// 0=off, 1=on
/*00000*/ uint8  raidAutoconsent;		// 0=off, 1=on
/*00000*/ uint8  guildAutoconsent;		// 0=off, 1=on
/*00000*/ uint8 unknown_rof55;			// Seen 1
/*00000*/ uint32 level3;				// SoF looks at the level here to determine how many leadership AA you can bank.
/*00000*/ uint32 showhelm;				// 0=no, 1=yes
/*00000*/ uint32 RestTimer;
/*00000*/ uint8 unknown_rof56;
/*00000*/ uint32 unknown_rof57;			// Seen 49
/*00000*/ uint8  unknown_rof58[1045];	// Seen all 0s

/*
///////////////////// - Haven't identified the below fields in the PP yet
uint8   pvp;					// 1=pvp, 0=not pvp
uint8   anon;					// 2=roleplay, 1=anon, 0=not anon
uint8   gm;					// 0=no, 1=yes (guessing!)
uint32   guild_id;				// guildid
uint8    guildrank;			// 0=member, 1=officer, 2=guildleader -1=no guild
uint32  guildbanker;
uint32 available_slots;
uint32  endurance;			// Current endurance
uint32  spellSlotRefresh[MAX_PP_MEMSPELL]; // Refresh time (millis) - 4 bytes Each * 16 = 64 bytes
uint32  abilitySlotRefresh;
///////////////////////

uint32  platinum_bank;		// Platinum Pieces in Bank
uint32  gold_bank;			// Gold Pieces in Bank
uint32  silver_bank;			// Silver Pieces in Bank
uint32  copper_bank;			// Copper Pieces in Bank
uint32  platinum_shared;		// Shared platinum pieces

uint32  autosplit;			// 0 = off, 1 = on

char      groupMembers[MAX_GROUP_MEMBERS][64];// 384 all the members in group, including self
char      groupLeader[64];	// Leader of the group ?
uint32  entityid;

uint32  leadAAActive;			// 0 = leader AA off, 1 = leader AA on
int32  ldon_points_guk;		// Earned GUK points
int32  ldon_points_mir;		// Earned MIR points
int32  ldon_points_mmc;		// Earned MMC points
int32  ldon_points_ruj;		// Earned RUJ points
int32  ldon_points_tak;		// Earned TAK points
int32  ldon_points_available;// Available LDON points
float  tribute_time_remaining;// Time remaining on tribute (millisecs)
uint32  career_tribute_points;// Total favor points for this char
uint32  tribute_points;		// Current tribute points
uint32  tribute_active;		// 0 = off, 1=on
Tribute_Struct tributes[MAX_PLAYER_TRIBUTES]; // [40] Current tribute loadout
double group_leadership_exp;	// Current group lead exp points
double raid_leadership_exp;	// Current raid lead AA exp points
uint32  group_leadership_points; // Unspent group lead AA points
uint32  raid_leadership_points;  // Unspent raid lead AA points
LeadershipAA_Struct leader_abilities; // [128]Leader AA ranks 19332

uint32  PVPKills;
uint32  PVPDeaths;
uint32  PVPCurrentPoints;
uint32  PVPCareerPoints;
uint32  PVPBestKillStreak;
uint32  PVPWorstDeathStreak;
uint32  PVPCurrentKillStreak;
PVPStatsEntry_Struct PVPLastKill;		// size 88
PVPStatsEntry_Struct PVPLastDeath;	// size 88
uint32  PVPNumberOfKillsInLast24Hours;
PVPStatsEntry_Struct PVPRecentKills[50];	// size 4400 - 88 each
uint32 expAA;					// Exp earned in current AA point
uint32 currentRadCrystals;	// Current count of radiant crystals
uint32 careerRadCrystals;		// Total count of radiant crystals ever
uint32 currentEbonCrystals;	// Current count of ebon crystals
uint32 careerEbonCrystals;	// Total count of ebon crystals ever
*/

};

/*
** Client Target Struct
** Length: 2 Bytes
** OpCode: 6221
*/
struct ClientTarget_Struct {
/*000*/	uint32	new_target;			// Target ID
};

/*
** Target Rejection Struct
** Length: 12 Bytes
** OpCode: OP_TargetReject
*/
struct TargetReject_Struct {
/*00*/	uint8	unknown00[12];
};

struct PetCommand_Struct {
/*00*/ uint32	command;
/*04*/ uint32	target;
/*08*/ uint32	unknown08;
};

/*
** Delete Spawn
** Length: 5 Bytes
** OpCode: OP_DeleteSpawn
*/
struct DeleteSpawn_Struct
{
/*00*/ uint32 spawn_id;		// Spawn ID to delete
/*04*/ uint8 unknown04;		// Seen 1
/*05*/
};

/*
** Channel Message received or sent
** Length: 144 Bytes + Variable Length + 1
** OpCode: OP_ChannelMessage
**
*/
struct ChannelMessage_Struct
{
/*000*/	char	targetname[64];		// Tell recipient
/*064*/	char	sender[64];			// The senders name (len might be wrong)
/*128*/	uint32	language;			// Language
/*132*/	uint32	chan_num;			// Channel
/*136*/	uint32	cm_unknown4[2];		// ***Placeholder
/*144*/	uint32	skill_in_language;	// The players skill in this language? might be wrong
/*148*/	char	message[0];			// Variable length message
};

/*
** Special Message
** Length: 4 Bytes + Variable Text Length + 1
** OpCode: OP_SpecialMesg
**
*/
/*
	Theres something wrong with this... example live packet:
Server->Client: [ Opcode: OP_SpecialMesg (0x0fab) Size: 244 ]
   0: 01 02 00 0A 00 00 00 09 - 05 00 00 42 61 72 73 74  | ...........Barst
  16: 72 65 20 53 6F 6E 67 77 - 65 61 76 65 72 00 7C F9  | re Songweaver.|.
  32: FF FF 84 FF FF FF 03 00 - 00 00 47 72 65 65 74 69  | ..........Greeti

*/
struct SpecialMesg_Struct
{
/*00*/	char	header[3];				// 04 04 00 <-- for #emote style msg
/*03*/	uint32	msg_type;				// Color of text (see MT_*** below)
/*07*/	uint32	target_spawn_id;		// Who is it being said to?
/*11*/	char	sayer[1];				// Who is the source of the info - Was 1
/*12*/	uint8	unknown12[12];
/*24*/	char	message[128];				// What is being said? - was 128
};

/*
** When somebody changes what they're wearing
**      or give a pet a weapon (model changes)
** Length: 19 Bytes
*/
struct WearChange_Struct{
/*000*/ uint16 spawn_id;
/*002*/ uint32 material;
/*006*/ uint32 unknown06;
/*010*/ uint32 elite_material;	// 1 for Drakkin Elite Material
/*014*/ uint32 hero_forge_model; // New to VoA
/*018*/ uint32 unknown18; // New to RoF
/*022*/ Tint_Struct color;
/*026*/ uint8 wear_slot_id;
/*027*/
};

/*
** Type:   Bind Wound Structure
** Length: 8 Bytes
*/
//Fixed for 7-14-04 patch
struct BindWound_Struct
{
/*000*/	uint16	to;			// TargetID
/*002*/	uint16	unknown2;		// ***Placeholder
/*004*/	uint16	type;
/*006*/	uint16	unknown6;
};


/*
** Type:   Zone Change Request (before hand)
** Length: 88 bytes
** OpCode: a320
*/

struct ZoneChange_Struct {
/*000*/	char	char_name[64];     // Character Name
/*064*/	uint16	zoneID;
/*066*/	uint16	instanceID;
/*068*/	uint32  Unknown068;
/*072*/	uint32  Unknown072;
/*076*/	float	y;
/*080*/	float	x;
/*084*/	float	z;
/*088*/	uint32	zone_reason;	//0x0A == death, I think
/*092*/	int32	success;		// =0 client->server, =1 server->client, -X=specific error
/*096*/ uint32	Unknown096;	// Not sure the extra 4 bytes goes here or earlier in the struct.
/*100*/
};

struct RequestClientZoneChange_Struct {
/*000*/	uint16	zone_id;
/*002*/	uint16	instance_id;
/*004*/	uint32	unknown004;
/*008*/	float	y;
/*012*/	float	x;
/*016*/	float	z;
/*020*/	float	heading;
/*024*/	uint32	type;	//unknown... values
/*032*/	uint8	unknown032[144];
/*172*/	uint32	unknown172;
/*176*/
};

struct Animation_Struct {
/*00*/	uint16 spawnid;
/*02*/	uint8 action;
/*03*/	uint8 speed;
/*04*/
};

// this is what causes the caster to animate and the target to
// get the particle effects around them when a spell is cast
// also causes a buff icon
struct Action_Struct
{
/*00*/	uint16 target;			// id of target
/*02*/	uint16 source;			// id of caster
/*04*/	uint16 level;			// level of caster for spells, OSX dump says attack rating, guess spells use it for level
/*06*/  uint32 unknown06;		// OSX dump says base_damage, was used for bard mod too, this is 0'd :(
/*10*/	float instrument_mod;
/*14*/  float force;
/*18*/  float hit_heading;
/*22*/  float hit_pitch;
/*26*/  uint8 type;				// 231 (0xE7) for spells, skill
/*27*/  uint32 damage;			// OSX says min_damage
/*31*/  uint16 unknown31;		// OSX says tohit
/*33*/	uint32 spell;			// spell id being cast
/*37*/	uint8 spell_level;			// level of caster again? Or maybe the castee
/*38*/	uint8 effect_flag;		// if this is 4, the effect is valid: or if two are sent at the same time?
/*39*/
};

// Starting with 2/21/2006, OP_Actions seem to come in pairs, duplicating
// themselves, with the second one with slightly more information. Maybe this
// has to do with buff blocking??
struct ActionAlt_Struct
{
/*00*/	uint16 target;			// id of target
/*02*/	uint16 source;			// id of caster
/*04*/	uint16 level;			// level of caster for spells, OSX dump says attack rating, guess spells use it for level
/*06*/  uint32 unknown06;		// OSX dump says base_damage, was used for bard mod too, this is 0'd :(
/*10*/	float instrument_mod;
/*14*/  float force;
/*18*/  float hit_heading;
/*22*/  float hit_pitch;
/*26*/  uint8 type;				// 231 (0xE7) for spells, skill
/*27*/  uint32 damage;			// OSX says min_damage
/*31*/  uint16 unknown31;		// OSX says tohit
/*33*/	uint32 spell;			// spell id being cast
/*37*/	uint8 spell_level;		// level of caster again? Or maybe the castee
/*38*/	uint8 effect_flag;		// if this is 4, the effect is valid: or if two are sent at the same time?
/*39*/	uint8 spell_gem;
/*40*/	InventorySlot_Struct slot;
/*52*/	uint32 item_cast_type;	// ItemSpellTypes enum from MQ2
/*56*/
};

// this is what prints the You have been struck. and the regular
// melee messages like You try to pierce, etc.  It's basically the melee
// and spell damage message
struct CombatDamage_Struct
{
/* 00 */	uint16	target;
/* 02 */	uint16	source;
/* 04 */	uint8	type;			//slashing, etc.  231 (0xE7) for spells
/* 05 */	uint32	spellid;
/* 09 */	int32	damage;
/* 13 */	float	force;		// cd cc cc 3d
/* 17 */	float	hit_heading;		// see above notes in Action_Struct
/* 21 */	float	hit_pitch;
/* 25 */	uint8	secondary;	// 0 for primary hand, 1 for secondary
/* 26 */	uint32	special; // 2 = Rampage, 1 = Wild Rampage
/* 30 */
};


/*
** Consider Struct
** Length: 20 Bytes
*/
struct Consider_Struct{
/*000*/ uint32	playerid;               // PlayerID
/*004*/ uint32	targetid;               // TargetID
/*008*/ uint32	faction;                // Faction
/*012*/ uint32	level;					// Level
/*016*/ uint8	pvpcon;					// Pvp con flag 0/1
/*017*/ uint8	unknown017[3];			//
/*020*/
};

/*
** Spawn Death Blow
** Length: 32 Bytes
** OpCode: 0114
*/
struct Death_Struct
{
/*000*/	uint32	spawn_id;
/*004*/	uint32	killer_id;
/*008*/	uint32	corpseid;	// was corpseid
/*012*/	uint32	attack_skill;	// was type
/*016*/	uint32	spell_id;
/*020*/ uint32	bindzoneid;	//bindzoneid?
/*024*/	uint32	damage;
/*028*/	uint32	unknown028;
};

struct BecomeCorpse_Struct {
	uint32	spawn_id;
	float y;
	float x;
	float z;
};

struct ZonePlayerToBind_Struct {
/*000*/	uint16	bind_zone_id;
/*002*/	uint16	bind_instance_id;
/*004*/	float	x;
/*008*/	float	y;
/*012*/	float	z;
/*016*/	float	heading;
/*020*/	char	zone_name[1];  // Or "Bind Location"
/*000*/	uint8	unknown021;	// Seen 1 - Maybe 0 would be to force a rezone and 1 is just respawn
/*000*/	uint32	unknown022;	// Seen 32 or 59
/*000*/	uint32	unknown023;	// Seen 0
/*000*/	uint32	unknown024;	// Seen 21 or 43
};

struct ZonePlayerToBindHeader_Struct
{
	/*000*/	uint16	bind_zone_id;
	/*002*/	uint16	bind_instance_id;
	/*004*/	float	x;
	/*008*/	float	y;
	/*012*/	float	z;
	/*016*/	float	heading;
	/*020*/	char	zone_name[1];  // Or "Bind Location"
};

struct ZonePlayerToBindFooter_Struct
{
	/*000*/	uint8	unknown021;	// Seen 1 - Maybe 0 would be to force a rezone and 1 is just respawn
	/*000*/	uint32	unknown022;	// Seen 32 or 59
	/*000*/	uint32	unknown023;	// Seen 0
	/*000*/	uint32	unknown024;	// Seen 21 or 43
};

typedef struct {
/*000*/	uint32	bind_number;		// Number of this bind in the iteration
/*004*/	uint32	bind_zone_id;		// ID of the zone for this bind point or resurect point
/*008*/	float	x;					// X loc for this bind point
/*012*/	float	y;					// Y loc for this bind point
/*016*/	float	z;					// Z loc for this bind point
/*020*/	float	heading;			// Heading for this bind point
/*024*/	char	bind_zone_name[1];	// Or "Bind Location" or "Resurrect"
/*000*/	uint8	validity;		// 0 = valid choice, 1 = not a valid choice at this time (resurrection)
} RespawnOptions_Struct;

struct RespawnWindow_Struct {
/*000*/	uint32	unknown000;		// Seen 0
/*004*/	uint32	time_remaining;	// Total time before respawn in milliseconds
/*008*/	uint32	unknown008;		// Seen 0
/*012*/	uint32	total_binds;	// Total Bind Point Options? - Seen 2
/*016*/ RespawnOptions_Struct bind_points;
// First bind point is "Bind Location" and the last one is "Ressurect"
};


/*
** Spawn position update - Size: 22
**	Struct sent from server->client to update position of
**	another spawn's position update in zone (whether NPC or PC)
**
*/
struct PlayerPositionUpdateServer_Struct
{
/*0000*/	uint16		spawn_id;
/*0002*/	uint16		vehicle_id;
/*0004*/	signed		padding0004 : 12;
			signed		y_pos : 19;			// y coord
			unsigned	padding : 1;
/*0008*/	signed		delta_x : 13;		// change in x
			signed		delta_heading : 10;	// change in heading
			signed		padding0008 : 9;
/*0012*/	signed		delta_y : 13;		// change in y
			signed		z_pos : 19;			// z coord
/*0016*/	signed		x_pos : 19;			// x coord
			signed		animation : 10;		// animation
			signed		padding0016 : 3;
/*0020*/	unsigned	heading : 12;		// heading
			signed		delta_z : 13;		// change in z
			signed		padding0020 : 7;
/*0024*/
};


/*
** Player position update - Size: 40
**	Struct sent from client->server to update
**	player position on server
**
*/
struct PlayerPositionUpdateClient_Struct
{
/*0000*/	uint16		sequence;			// increments one each packet - Verified
/*0002*/	uint16		spawn_id;			// Player's spawn id
/*0004*/	uint16		vehicle_id;			// Player's vehicle spawn id
/*0006*/	uint8		unknown0004[4];		// ***Placeholder
/*0010*/	float		delta_y;			// Change in y
/*0014*/	float		x_pos;				// x coord (2nd loc value)
/*0018*/	float		y_pos;				// y coord (1st loc value)
/*0022*/	signed		delta_heading : 10;	// change in heading
			unsigned	animation : 10;		// ***Placeholder
			unsigned	padding0024 : 12;	// animation
/*0026*/	float		delta_z;			// Change in z
/*0030*/	float		delta_x;			// Change in x
/*0034*/	float		z_pos;				// z coord (3rd loc value)
/*0038*/	unsigned	heading : 12;		// Directional heading
			unsigned	padding0040 : 10;	// ***Placeholder
			unsigned	padding0041 : 10;	// ***Placeholder
/*0042*/
};

/*
** Spawn HP Update
** Length: 10 Bytes
** OpCode: OP_HPUpdate
*/
struct SpawnHPUpdate_Struct
{
/*00*/ int16	spawn_id;
/*02*/ uint32	cur_hp;
/*06*/ int32	max_hp;
/*10*/
};

/*
** SendExpZonein
** Length: 152 Bytes
** OpCode: OP_SendExpZonein
*/
struct SendExpZonein_Struct
{
/*0000*/ uint16 spawn_id;          // ID of the spawn
/*0002*/ uint16 type;              // Values associated with the type
/*0004*/ uint32 parameter;         // Type of data sent
/*0008*/ uint32 exp;                    // Current experience ratio from 0 to 330
/*0012*/ uint32	expAA;
/*0016*/ uint8	unknown0016[4];
/*0020*/ char	name[64];
/*0084*/ char	last_name[64];
/*00148*/ uint32	unknown132;
/*00152*/
};

/*
** SendExpZonein
** Length: 0 Bytes
** OpCode: OP_SendExpZonein
*/
//struct SendExpZonein_Struct {};

struct SpawnHPUpdate_Struct2
{
/*01*/ int16	spawn_id;
/*00*/ uint8	hp;
};
/*
** Stamina
** Length: 8 Bytes
** OpCode: 5721
*/
struct Stamina_Struct {
/*00*/ uint32 food;                     // (low more hungry 127-0)
/*02*/ uint32 water;                    // (low more thirsty 127-0)
};

/*
** Level Update
** Length: 12 Bytes
*/
struct LevelUpdate_Struct
{
/*00*/ uint32 level;                  // New level
/*04*/ uint32 level_old;              // Old level
/*08*/ uint32 exp;                    // Current Experience
};

/*
** Experience Update
** Length: 14 Bytes
** OpCode: 9921
*/
struct ExpUpdate_Struct
{
/*0000*/ uint32 exp;                    // Current experience ratio from 0 to 330
/*0004*/ uint32 aaxp; // @BP ??
};

/*
** Item Packet Struct - Works on a variety of opcodes
** Packet Types: See ItemPacketType enum
**
*/
enum ItemPacketType
{
	ItemPacketViewLink			= 0x00,
	ItemPacketTradeView			= 0x65,
	ItemPacketLoot				= 0x66,
	ItemPacketTrade				= 0x67,
	ItemPacketCharInventory		= 0x69,
	ItemPacketSummonItem		= 0x6A,
	ItemPacketTributeItem		= 0x6C,
	ItemPacketMerchant			= 0x64,
	ItemPacketWorldContainer	= 0x6B
};
struct ItemPacket_Struct
{
/*00*/	ItemPacketType	PacketType;
/*04*/	char			SerializedItem[1]; //was 1
/*xx*/
};

struct BulkItemPacket_Struct
{
/*00*/	char			SerializedItem[0];
/*xx*/
};

struct Consume_Struct
{
/*000*/ InventorySlot_Struct	inventory_slot;
/*012*/ uint32	auto_consumed;	// 0xffffffff when auto eating e7030000 when right click
/*016*/ uint32	type;			// 0x01=Food 0x02=Water
/*020*/ uint32	c_unknown1;		// Seen 2
/*024*/
};

struct ItemNamePacket_Struct {
/*000*/	uint32 item_id;
/*004*/	uint32 unkown004;
/*008*/ char name[64];
/*072*/
};

// Length: 16
struct ItemProperties_Struct_Old {

/*000*/ uint8	unknown01[2];
/*002*/ uint8	charges;
/*003*/ uint8	unknown02[13];
/*016*/
};

// Length: 8
struct ItemProperties_Struct {

/*000*/ uint8	unknown01[4];
/*004*/ uint8	charges;
/*005*/ uint8	unknown02[3];
/*008*/
};

struct DeleteItem_Struct
{
/*0000*/ InventorySlot_Struct	from_slot;
/*0012*/ InventorySlot_Struct	to_slot;
/*0024*/ uint32		number_in_stack;
/*0028*/
};

struct MoveItem_Struct
{
/*0000*/ InventorySlot_Struct	from_slot;
/*0012*/ InventorySlot_Struct	to_slot;
/*0024*/ uint32		number_in_stack;
/*0028*/
};

//
// from_slot/to_slot
// -1 - destroy
//  0 - cursor
//  1 - inventory
//  2 - bank
//  3 - trade
//  4 - shared bank
//
// cointype
// 0 - copeer
// 1 - silver
// 2 - gold
// 3 - platinum
//
static const uint32 COINTYPE_PP = 3;
static const uint32 COINTYPE_GP = 2;
static const uint32 COINTYPE_SP = 1;
static const uint32 COINTYPE_CP = 0;

struct MoveCoin_Struct
{
		 int32 from_slot;
		 int32 to_slot;
		 int32 cointype1;
		 int32 cointype2;
		 int32	amount;
};
struct TradeCoin_Struct{
	uint32	trader;
	uint8	slot;
	uint16	unknown5;
	uint8	unknown7;
	uint32	amount;
};
struct TradeMoneyUpdate_Struct{
	uint32	trader;
	uint32	type;
	uint32	amount;
};
/*
** Surname struct
** Size: 100 bytes
*/
struct Surname_Struct
{
/*0000*/	char name[64];
/*0064*/	uint32 unknown0064;
/*0068*/	char lastname[32];
/*0100*/
};

struct GuildsListEntry_Struct {
	char name[64];
};

struct GuildsList_Struct {
	uint8 head[64]; // First on guild list seems to be empty...
	GuildsListEntry_Struct Guilds[MAX_NUMBER_GUILDS];
};

struct GuildUpdate_Struct {
	uint32	guildID;
	GuildsListEntry_Struct entry;
};

struct GuildBankAck_Struct
{
/*00*/	uint32	Action;	//	10
/*04*/	uint32	Unknown04;
/*08*/	uint32	Unknown08;
};

struct GuildBankDepositAck_Struct
{
/*00*/	uint32	Action;	//	10
/*04*/	uint32	Unknown04;
/*08*/	uint32	Unknown08;
/*08*/	uint32	Fail;	//1 = Fail, 0 = Success
};

struct GuildBankPromote_Struct
{
/*00*/	uint32	Action;	// 3
/*04*/	uint32	Unknown04;
/*08*/	uint32	Unknown08;
/*12*/	uint32	Slot;
/*16*/	uint32	Slot2;	// Always appears to be the same as Slot for Action code 3
/*20*/  uint32  unknown20;
};

struct GuildBankPermissions_Struct
{
/*00*/	uint32	Action;	// 6
/*04*/	uint32	Unknown04;
/*08*/	uint32	Unknown08;
/*08*/	uint16	SlotID;
/*10*/	uint16	Unknown10; // Saw 1, probably indicating it is the main area rather than deposits
/*12*/	uint32	ItemID;
/*16*/	uint32	Permissions;
/*20*/	char	MemberName[64];
};

struct GuildBankViewItem_Struct
{
/*00*/	uint32	Action;
/*04*/	uint32	Unknown04;
/*08*/	uint32	Unknown08;
/*08*/	uint16	SlotID;	// 0 = Deposit area, 1 = Main area
/*10*/	uint16	Area;
/*12*/	uint32	Unknown12;
/*16*/	uint32	Unknown16;
};

struct GuildBankWithdrawItem_Struct
{
/*00*/	uint32	Action;
/*04*/	uint32	Unknown04;
/*08*/	uint32	Unknown08;
/*08*/	uint16	SlotID;
/*10*/	uint16	Area;
/*12*/	uint32	Unknown12;
/*16*/	uint32	Quantity;
/*20*/
};

struct GuildBankItemUpdate_Struct
{
	void Init(uint32 inAction, uint32 inUnknown004, uint16 inSlotID, uint16 inArea, uint16 inUnknown012, uint32 inItemID, uint32 inIcon, uint32 inQuantity,
			uint32 inPermissions, uint32 inAllowMerge, bool inUseable)
	{
		action       = inAction;
		unknown004   = inUnknown004;
		slot_id      = inSlotID;
		area         = inArea;
		display      = inUnknown012;
		item_id      = inItemID;
		icon_id      = inIcon;
		quantity     = inQuantity;
		permissions  = inPermissions;
		allow_merge  = inAllowMerge;
		is_useable   = inUseable;
		item_name[0] = '\0';
		donator[0]   = '\0';
		who_for[0]   = '\0';
	};

/*000*/	uint32	action;
/*004*/	uint32	unknown004;
/*008*/	uint32	unknown008;
/*012*/	uint16	slot_id;
/*014*/	uint16	area;
/*016*/	uint32	display;
/*020*/	uint32	item_id;
/*024*/	uint32	icon_id;
/*028*/	uint32	quantity;
/*032*/	uint32	permissions;
/*036*/	uint8	allow_merge;
/*037*/	uint8	is_useable;	// Used in conjunction with the Public-if-useable permission.
/*038*/	char	item_name[64];
/*102*/	char	donator[64];
/*166*/ char	who_for[64];
/*230*/	uint16	unknown226;
};

struct GuildBankClear_Struct
{
/*00*/	uint32	Action;
/*04*/	uint32	Unknown04;
/*08*/	uint32	Unknown08;
/*12*/	uint32	DepositAreaCount;
/*16*/	uint32	MainAreaCount;
};

/*
** Money Loot
** Length: 22 Bytes
** OpCode: 5020
*/
struct moneyOnCorpseStruct {
/*0000*/ uint8	response;		// 0 = someone else is, 1 = OK, 2 = not at this time
/*0001*/ uint8	unknown1;		// = 0x5a
/*0002*/ uint8	unknown2;		// = 0x40
/*0003*/ uint8	unknown3;		// = 0
/*0004*/ uint32	platinum;		// Platinum Pieces
/*0008*/ uint32	gold;			// Gold Pieces

/*0012*/ uint32	silver;			// Silver Pieces
/*0016*/ uint32	copper;			// Copper Pieces
};

struct LootingItem_Struct {
/*000*/	uint32	lootee;
/*004*/	uint32	looter;
/*008*/	uint16	slot_id;
/*010*/	uint16	unknown10;
/*012*/	int32	auto_loot;
/*016*/	uint32	unknown16;
/*020*/
};

struct GuildManageStatus_Struct{
	uint32	guildid;
	uint32	oldrank;
	uint32	newrank;
	char	name[64];
};
// Guild invite, remove
struct GuildJoin_Struct{
/*000*/	uint32	guild_id;
/*004*/	uint32	unknown04;
/*008*/	uint32	level;
/*012*/	uint32	class_;
/*016*/	uint32	rank;//0 member, 1 officer, 2 leader
/*020*/	uint32	zoneid;
/*024*/	uint32	unknown24;
/*028*/	char	name[64];
/*092*/
};
struct GuildInviteAccept_Struct {
	char inviter[64];
	char newmember[64];
	uint32 response;
	uint32 guildeqid;
};
struct GuildManageRemove_Struct {
	uint32 guildeqid;
	char member[64];
};
struct GuildCommand_Struct {
/*000*/	char othername[64];
/*064*/	char myname[64];
/*128*/	uint16 guildeqid;
/*130*/	uint8 unknown[2];		// for guildinvite all 0's, for remove 0=0x56, 2=0x02
/*132*/	uint32 officer;
/*136*/	uint32 unknown136;		// New in RoF
/*140*/
};

// Opcode OP_GMZoneRequest
// Size = 88 bytes
struct GMZoneRequest_Struct {
/*0000*/	char	charname[64];
/*0064*/	uint32	zone_id;
/*0068*/	float	x;
/*0072*/	float	y;
/*0076*/	float	z;
/*0080*/	float	heading;
/*0084*/	uint32	success;		// 0 if command failed, 1 if succeeded?
/*0088*/
//	/*072*/	int8	success;		// =0 client->server, =1 server->client, -X=specific error
//	/*073*/	uint8	unknown0073[3]; // =0 ok, =ffffff error
};

struct GMSummon_Struct {
/*  0*/	char    charname[64];
/* 30*/	char    gmname[64];
/* 60*/ uint32	success;
/* 61*/	uint32	zoneID;
/*92*/	int32  y;
/*96*/	int32  x;
/*100*/ int32  z;
/*104*/	uint32 unknown2; // E0 E0 56 00
};

struct GMGoto_Struct { // x,y is swapped as compared to summon and makes sense as own packet
/*  0*/	char    charname[64];

/* 64*/	char    gmname[64];
/* 128*/ uint32	success;
/* 132*/	uint32	zoneID;

/*136*/	int32  y;
/*140*/	int32  x;
/*144*/ int32  z;
/*148*/	uint32 unknown2; // E0 E0 56 00
};

struct GMLastName_Struct {
/*000*/	char name[64];
/*064*/	char gmname[64];
/*128*/	char lastname[64];
/*192*/	uint16 unknown[4];	// 0x00, 0x00, 0x01, 0x00 = Update the clients
/*200*/	uint32 unknown200[8];
/*232*/
};

struct OnLevelMessage_Struct {
/*0000*/	uint32  Title_Count;
/*0000*/	char    Title[128];
/*0000*/	uint32  Text_Count;
/*0000*/	char    Text[4096];
/*0000*/	uint32  ButtonName0_Count;
/*0000*/	char	ButtonName0[25];	// If Buttons = 1, these two are the text for the left and right buttons respectively
/*0000*/	uint32  ButtonName1_Count;
/*0000*/	char	ButtonName1[25];
/*0000*/	uint8	Buttons;
/*0000*/	uint8	SoundControls;	// Something to do with audio controls
/*0000*/	uint32  Duration;
/*0000*/	uint32  PopupID;	// If none zero, a response packet with 00 00 00 00 <PopupID> is returned on clicking the left button
/*0000*/	uint32  NegativeID;	// If none zero, a response packet with 01 00 00 00 <NegativeID> is returned on clicking the right button
/*0000*/	uint32  Unknown4288;
/*0000*/	uint8	Unknown4276;
/*0000*/	uint8	Unknown4277;
/*0000*/
};

//Combat Abilities
struct CombatAbility_Struct {
	uint32 m_target;		//the ID of the target mob
	uint32 m_atk;
	uint32 m_skill;
};

//Instill Doubt
struct Instill_Doubt_Struct {
	uint8 i_id;
	uint8 ia_unknown;
	uint8 ib_unknown;
	uint8 ic_unknown;
	uint8 i_atk;

	uint8 id_unknown;
	uint8 ie_unknown;
	uint8 if_unknown;
	uint8 i_type;
	uint8 ig_unknown;
	uint8 ih_unknown;
	uint8 ii_unknown;
};

struct GiveItem_Struct {
	uint16 to_entity;
	int16 to_equipSlot;
	uint16 from_entity;
	int16 from_equipSlot;
};

struct RandomReq_Struct {
	uint32 low;
	uint32 high;
};

/* 9/23/03 reply to /random command */
struct RandomReply_Struct {
/* 00 */	uint32 low;
/* 04 */	uint32 high;
/* 08 */	uint32 result;
/* 12 */	char name[64];
/* 76 */
};

/*
** LFG_Appearance_Struct
** Packet sent to clients to notify when someone in zone toggles LFG flag
** Size: 8 bytes
** Used in: OP_LFGAppearance
**
*/
struct LFG_Appearance_Struct
{
/*0000*/ uint32 spawn_id;		// ID of the client
/*0004*/ uint8 lfg;				// 1=LFG, 0=Not LFG
/*0005*/ char unknown0005[3];	//
/*0008*/
};


// EverQuest Time Information:
// 72 minutes per EQ Day
// 3 minutes per EQ Hour
// 6 seconds per EQ Tick (2 minutes EQ Time)
// 3 seconds per EQ Minute

struct TimeOfDay_Struct {
	uint8	hour;
	uint8	minute;
	uint8	day;
	uint8	month;
	uint16	year;
/*0006*/ uint16 unknown0016;            // Placeholder
/*0008*/
};

// Darvik: shopkeeper structs
struct MerchantClick_Struct
{
    /*000*/ uint32 npc_id;      // Merchant NPC's entity id
    /*004*/ uint32 player_id;
    /*008*/ uint32 command;     // 1=open, 0=cancel/close
    /*012*/ float  rate;        // cost multiplier, dosent work anymore
    /*016*/ int32  tab_display; // bitmask b000 none, b001 Purchase/Sell, b010 Recover, b100 Parcels
    /*020*/ int32  unknown020;  // Seen 2592000 from Server or -1 from Client
    /*024*/
};

/*
Unknowns:
0 is e7 from 01 to // MAYBE SLOT IN PURCHASE
1 is 03
2 is 00
3 is 00
4 is ??
5 is ??
6 is 00 from a0 to
7 is 00 from 3f to */
/*
0 is F6 to 01
1 is CE CE
4A 4A
00 00
00 E0
00 CB
00 90
00 3F
*/

struct Merchant_Sell_Struct {
/*000*/ uint32 npcid;		// Merchant NPC's entity id
/*004*/ uint32 playerid;	// Player's entity id
/*008*/ uint32 itemslot;	// Merchant Slot / Item Instance ID
/*012*/ uint32 unknown12;
/*016*/ uint32 quantity;	// Already sold
/*020*/ uint32 unknown20;
/*024*/ uint32 price;
/*028*/ uint32 unknown28;	// Normally 0, but seen 84 c5 63 00 as well
/*032*/
};

struct Merchant_Purchase_Struct {
/*000*/	uint32	npcid;			// Merchant NPC's entity id
/*004*/	TypelessInventorySlot_Struct	inventory_slot;
/*012*/	uint32	quantity;
/*016*/	uint32	price;
/*020*/
};

struct Merchant_DelItem_Struct{
/*000*/	uint32	npcid;			// Merchant NPC's entity id
/*004*/	uint32	playerid;		// Player's entity id
/*008*/	uint32	itemslot;
};

struct AltCurrencyDefinition_Struct {
    uint32 id;
    uint32 item_id;
};

//One packet i didn't include here is the alt currency merchant window.
//it works much like the adventure merchant window
//it is formated like: dbstringid|1|dbstringid|count
//ex for a blank crowns window you would send:
//999999|1|999999|0
//any items come after in much the same way adventure merchant items do except there is no theme included

//Server -> Client
//Populates the initial Alternate Currency Window
struct AltCurrencyPopulateEntry_Struct
{
/*000*/ uint32 currency_number;		// corresponds to a dbstr id as well, the string matches what shows up in the "alternate currency" tab.
/*004*/ uint32 unknown00;			// always 1
/*008*/ uint32 currency_number2;	// always same as currency number
/*012*/ uint32 item_id;				// appears to be the item id
/*016*/ uint32 item_icon;			// actual icon
/*020*/ uint32 stack_size;			// most are set to 1000, the stack size for the item; should match db i think or there will be problems.
/*024*/ uint8  display;				// If set to 0, will not display by default.
/*025*/
};

struct AltCurrencyPopulate_Struct {
/*000*/ uint32 opcode;	// 8 for populate
/*004*/ uint32 count;	// number of entries
/*008*/ AltCurrencyPopulateEntry_Struct entries[0];
};

//Server -> Client
//Updates the value of a specific Alternate Currency
struct AltCurrencyUpdate_Struct {
/*000*/ uint32 opcode; //7 for update
/*004*/ char name[64]; //name of client (who knows why just do it)
/*068*/ uint32 currency_number; //matches currency_number from populate entry
/*072*/ uint32 unknown072; //always 1
/*076*/ uint32 amount; //new amount
/*080*/ uint32 unknown080; //seen 0
/*084*/ uint32 unknown084; //seen 0
};

//Client -> Server
//When an item is selected while the alt currency merchant window is open
struct AltCurrencySelectItem_Struct
{
/*000*/ uint32 merchant_entity_id;
/*004*/ TypelessInventorySlot_Struct inventory_slot;
/*008*/ uint32 unknown008;
/*012*/ uint32 unknown012;
/*016*/ uint32 unknown016;
/*020*/ uint32 unknown020;
/*024*/ uint32 unknown024;
/*028*/ uint32 unknown028;
/*032*/ uint32 unknown032;
/*036*/ uint32 unknown036;
/*040*/ uint32 unknown040;
/*044*/ uint32 unknown044;
/*048*/ uint32 unknown048;
/*052*/ uint32 unknown052;
/*056*/ uint32 unknown056;
/*060*/ uint32 unknown060;
/*064*/ uint32 unknown064;
/*068*/ uint32 unknown068;
/*072*/ uint32 unknown072;
/*076*/ uint32 unknown076;
};

//Server -> Client
//As setup it makes it so that item can't be sold to the merchant.
//eg: "I will give you no doubloons for a cloth cap."
//Probably also sends amounts somewhere
struct AltCurrencySelectItemReply_Struct {
/*000*/ uint32 unknown000;
/*004*/ uint8  unknown004; //0xff
/*005*/ uint8  unknown005; //0xff
/*006*/ uint8  unknown006; //0xff
/*007*/ uint8  unknown007; //0xff
/*008*/ char   item_name[64];
/*072*/ uint32 unknown074;
/*076*/ uint32 cost;
/*080*/ uint32 unknown080;
/*084*/ uint32 unknown084;
};

//Client -> Server
//Requests purchase of a specific item from the vendor
struct AltCurrencyPurchaseItem_Struct {
/*000*/ uint32 merchant_entity_id;
/*004*/ uint32 item_id;
/*008*/ uint32 unknown008; //1
};

//Client -> Server
//Reclaims / Create currency button pushed.
struct AltCurrencyReclaim_Struct {
/*000*/ uint32 currency_id;
/*004*/ uint32 unknown004;
/*008*/ uint32 count;
/*012*/ uint32 reclaim_flag; //1 = this is reclaim
};

struct AltCurrencySellItem_Struct {
/*000*/ uint32 merchant_entity_id;
/*004*/ TypelessInventorySlot_Struct inventory_slot;
/*008*/ uint32 charges;
/*012*/ uint32 cost;
};

struct Adventure_Purchase_Struct {
/*000*/	uint32	some_flag;	//set to 1 generally...
/*004*/	uint32	npcid;
/*008*/	uint32	itemid;
/*012*/	uint32	variable;
};

struct Adventure_Sell_Struct {
/*000*/	uint32	unknown000;	//0x01 - Stack Size/Charges?
/*004*/	uint32	npcid;
/*008*/ TypelessInventorySlot_Struct inventory_slot;
/*016*/	uint32	charges;
/*020*/	uint32	sell_price;
/*024*/
};

struct AdventurePoints_Update_Struct {
/*000*/	uint32				ldon_available_points;		// Total available points
/*004*/ uint8				unkown_apu004[20];
/*024*/	uint32				ldon_guk_points;		// Earned Deepest Guk points
/*028*/	uint32				ldon_mirugal_points;		// Earned Mirugal' Mebagerie points
/*032*/	uint32				ldon_mistmoore_points;		// Earned Mismoore Catacombs Points
/*036*/	uint32				ldon_rujarkian_points;		// Earned Rujarkian Hills points
/*040*/	uint32				ldon_takish_points;		// Earned Takish points
/*044*/	uint8				unknown_apu042[216];
};


struct AdventureFinish_Struct{
	uint32 win_lose;//Cofruben: 00 is a lose,01 is win.
	uint32 points;
};
//OP_AdventureRequest
struct AdventureRequest_Struct{
	uint32 risk;//1 normal,2 hard.
	uint32 entity_id;
};
struct AdventureRequestResponse_Struct{
	uint32 unknown000;
	char text[2048];
	uint32 timetoenter;
	uint32 timeleft;
	uint32 risk;
	float x;
	float y;
	float z;
	uint32 showcompass;
	uint32 unknown2080;
};

//this is mostly right but something is off that causes the client to crash sometimes
//I don't really care enough about the feature to work on it anymore though.
struct AdventureLeaderboardEntry_Struct
{
/*004*/ char name[64];
/*008*/ uint32 success;
/*012*/ uint32 failure;
/*016*/
};

struct AdventureLeaderboard_Struct
{
/*000*/ uint32 unknown000;
/*004*/ uint32 unknown004;
/*008*/ uint32 success;
/*012*/ uint32 failure;
/*016*/ uint32 our_rank;
/*020*/
};

/*struct Item_Shop_Struct {
	uint16 merchantid;
	uint8 itemtype;
	ItemData item;
	uint8 iss_unknown001[6];
};*/

struct Illusion_Struct { // Was size: 336
/*000*/ uint32 spawnid;
/*004*/ char charname[64];
/*068*/ uint16 race;
/*070*/ char unknown006[2];		// Weird green name
/*072*/ uint8 gender;
/*073*/ uint8 texture;
/*074*/ uint8 unknown074;
/*075*/ uint8 unknown075;
/*076*/ uint8 helmtexture;
/*077*/ uint8 unknown077;
/*078*/ uint8 unknown078;
/*079*/ uint8 unknown079;
/*080*/ uint32 face;
/*084*/ uint8 hairstyle;		// Some Races don't change Hair Style Properly in SoF
/*085*/ uint8 haircolor;
/*086*/ uint8 beard;
/*087*/ uint8 beardcolor;
/*088*/ float size;
/*092*/ uint8 unknown092[148];
/*240*/ uint32 unknown240;		// Removes armor?
/*244*/ uint32 drakkin_heritage;
/*248*/ uint32 drakkin_tattoo;
/*252*/ uint32 drakkin_details;
/*256*/ uint8 unknown256[60];	// This and below are new to RoF
/*316*/ int32 unknown316;		// Seen -1
/*320*/ uint8 unknown320[16];
/*336*/
};

struct ZonePoint_Entry { //32 octets
/*0000*/	uint32	iterator;
/*0004*/	float	y;
/*0008*/	float	x;
/*0012*/	float	z;
/*0016*/	float	heading;
/*0020*/	uint16	zoneid;
/*0022*/	uint16	zoneinstance; // LDoN instance
/*0024*/	uint32  unknown024;
/*0028*/	uint32	unknown028;
/*0032*/
};

struct ZonePoints {
/*0000*/	uint32	count;
/*0004*/	struct	ZonePoint_Entry zpe[0]; // Always add one extra to the end after all zonepoints
//*0xxx*/    uint8     unknown0xxx[24]; //New from SEQ
};

struct SkillUpdate_Struct {
/*00*/	uint32 skillId;
/*04*/	uint32 value;
/*08*/	uint8 unknown08;	// Seen 1
/*09*/	uint8 unknown09;	// Seen 80
/*10*/	uint8 unknown10;	// Seen 136
/*11*/	uint8 unknown11;	// Seen 54
/*12*/
};

struct ZoneUnavail_Struct {
	//This actually varies, but...
	char zonename[16];
	int16 unknown[4];
};

struct GroupInvite_Struct {
/*0000*/	char	invitee_name[64];
/*0064*/	char	inviter_name[64];
/*0128*/	uint32	unknown0128;
/*0132*/	uint32	unknown0132;
/*0136*/	uint32	unknown0136;
/*0140*/	uint32	unknown0140;
/*0144*/	uint32	unknown0144;
/*0148*/
};

struct GroupGeneric_Struct {
/*0000*/	char	name1[64];
/*0064*/	char	name2[64];
/*0128*/	uint32	unknown0128;
/*0132*/	uint32	unknown0132;
/*0136*/	uint32	unknown0136;
/*0140*/	uint32	unknown0140;
/*0144*/	uint32	unknown0144;
/*0148*/
};

struct GroupCancel_Struct {
/*000*/	char	name1[64];
/*064*/	char	name2[64];
/*128*/	uint8	unknown128[20];
/*148*/	uint32	toggle;
/*152*/
};

struct GroupUpdate_Struct {
/*0000*/	uint32	action;
/*0004*/	char	yourname[64];
/*0068*/	char	membername[5][64];
/*0388*/	char	leadersname[64];
/*0452*/
};

struct GroupUpdate2_Struct {
/*0000*/	uint32	action;
/*0004*/	char	yourname[64];
/*0068*/	char	membername[5][64];
/*0388*/	char	leadersname[64];
/*0452*/	GroupLeadershipAA_Struct leader_aas;
/*0516*/	uint8	unknown[252];	// Titanium uses [188] here
/*0768*/
};

struct GroupUpdate_Struct_Live {	// New for Live
/*0000*/	uint32	groupid;		// Guess - Matches unknown0136 from GroupFollow_Struct
/*0004*/	uint32	totalmembers;	// Guess
/*0000*/	//uint32	leadersname[0];	// Group Leader Name Null Terminated
/*0008*/	//GroupMembers_Struct groupmembers;
};

struct GroupMembers_Struct {	// New for Live
/*0000*/	uint32	membernumber;	// Guess - number of member in the group (0 to 5?)
/*0000*/	//char	member_name[0];	// Member Name Null Terminated
/*0000*/	uint8	unknown001[3];	// Seen 0
/*0000*/	uint32	memberlevel;	// Guess
/*0000*/	uint8	unknown002[11];	// Seen 0
};

struct GroupJoin_Struct_Live {	// New for Live
/*0000*/	uint32	unknown0000;	// Matches unknown0136 from GroupFollow_Struct
/*0004*/	uint32	action;
/*0008*/	uint8	unknown0008[5];	// Seen 0
/*0013*/	//char	member_name[0];	// Null Terminated?
/*0000*/	uint8	unknown0013[3];	// Seen 0
/*0000*/	uint32	unknown0016;	// Matches unknown0132 from GroupFollow_Struct
/*0000*/	uint8	unknown0020[11];	// Seen 0
};

struct GroupJoin_Struct {
/*000*/	char	unknown000[64];
/*064*/	char	membername[64];
/*128*/	uint8	unknown128[20];	// Leadership AA ?
/*148*/
};

struct GroupFollow_Struct { // Live Follow Struct
/*0000*/	char	name1[64];	// inviter
/*0064*/	char	name2[64];	// invitee
/*0128*/	uint32	unknown0128;	// Seen 0
/*0132*/	uint32	unknown0132;	// Group ID or member level?
/*0136*/	uint32	unknown0136;	// Maybe Voice Chat Channel or Group ID?
/*0140*/	uint32	unknown0140;	// Seen 0
/*0144*/	uint32	unknown0144;	// Seen 0
/*0148*/	uint32	unknown0148;
/*0152*/
};

struct InspectBuffs_Struct {
/*000*/ uint32 spell_id[BUFF_COUNT];
/*168*/ int32 tics_remaining[BUFF_COUNT];
};

struct LFG_Struct {
/*000*/	uint32 unknown000;
/*004*/	uint32 value; // 0x00 = off 0x01 = on
/*008*/	uint32 unknown008;
/*012*/	uint32 unknown012;
/*016*/	char	name[64];
};

struct FaceChange_Struct {
/*000*/	uint8	haircolor;
/*001*/	uint8	beardcolor;
/*002*/	uint8	eyecolor1;
/*003*/	uint8	eyecolor2;
/*004*/	uint8	hairstyle;
/*005*/	uint8	beard;
/*006*/	uint8	face;
/*007*/ uint8   unused_padding;
/*008*/ uint32	drakkin_heritage;
/*012*/ uint32	drakkin_tattoo;
/*016*/ uint32	drakkin_details;
/*020*/ uint32  entity_id;
/*024*/
};
//there are only 10 faces for barbs changing woad just
//increase the face value by ten so if there were 8 woad
//designs then there would be 80 barb faces

/*
** Trade request from one client to another
** Used to initiate a trade
** Size: 8 bytes
** Used in: OP_TradeRequest
*/
struct TradeRequest_Struct {
/*00*/	uint32 to_mob_id;
/*04*/	uint32 from_mob_id;
/*08*/
};

struct TradeAccept_Struct {
/*00*/	uint32 from_mob_id;
/*04*/	uint32 unknown4;		//seems to be garbage
/*08*/
};

/*
** Cancel Trade struct
** Sent when a player cancels a trade
** Size: 8 bytes
** Used In: OP_CancelTrade
**
*/
struct CancelTrade_Struct {
/*00*/	uint32 fromid;
/*04*/	uint32 action;
/*08*/
};

struct PetitionUpdate_Struct {
	uint32 petnumber;    // Petition Number
	uint32 color;		// 0x00 = green, 0x01 = yellow, 0x02 = red
	uint32 status;
	time_t senttime;    // 4 has to be 0x1F
	char accountid[32];
	char gmsenttoo[64];
	int32 quetotal;
	char charname[64];
};

struct Petition_Struct {
	uint32 petnumber;
	uint32 urgency;
	char accountid[32];
	char lastgm[32];
	uint32	zone;
	//char zone[32];
	char charname[64];
	uint32 charlevel;
	uint32 charclass;
	uint32 charrace;
	uint32 unknown;
	//time_t senttime; // Time?
	uint32 checkouts;
	uint32 unavail;
	//uint8 unknown5[4];
	time_t senttime;
	uint32 unknown2;
	char petitiontext[1024];
	char gmtext[1024];
};


struct Who_All_Struct { // 156 length total
/*000*/	char	whom[64];
/*088*/	uint8	unknown088[64];
/*064*/	uint32	wrace;		// FF FF = no race
/*068*/	uint32	wclass;		// FF FF = no class
/*072*/	uint32	lvllow;		// FF FF = no numbers
/*076*/	uint32	lvlhigh;	// FF FF = no numbers
/*080*/	uint32	gmlookup;	// FF FF = not doing /who all gm
/*084*/	uint32	guildid;	// Also used for Buyer/Trader/LFG
/*152*/	uint32	type;		// 0 = /who 3 = /who all
/*156*/
};

struct Stun_Struct { // 8 bytes total
/*000*/	uint32	duration; // Duration of stun
/*004*/	uint8	unknown004; // seen 0
/*005*/	uint8	unknown005; // seen 163
/*006*/	uint8	unknown006; // seen 67
/*007*/	uint8	unknown007; // seen 0
/*008*/
};

struct AugmentItem_Struct {
/*00*/	uint32	dest_inst_id;			// The unique serial number for the item instance that is being augmented
/*04*/	uint32	container_index;				// Seen 0
/*08*/	InventorySlot_Struct container_slot;	// Slot of the item being augmented
/*20*/	uint32	augment_index;				// Seen 0
/*24*/	InventorySlot_Struct augment_slot;	// Slot of the distiller to use (if one applies)
/*36*/	int32	augment_action;			// Guessed - 0 = augment, 1 = remove with distiller, 3 = delete aug
/*36*/	//int32	augment_slot;
/*40*/
};

// OP_Emote
struct Emote_Struct {
/*0000*/	uint32 unknown01;
/*0004*/	char message[1024]; // was 1024
/*1028*/
};

// Inspect
struct Inspect_Struct {
	uint32 TargetID;
	uint32 PlayerID;
};

//OP_InspectAnswer - Size: 1860
struct InspectResponse_Struct{
/*000*/	uint32 TargetID;
/*004*/	uint32 playerid;
/*008*/	char itemnames[23][64];
/*1480*/uint32 itemicons[23];
/*1572*/char text[288];	// Max number of chars in Inspect Window appears to be 254
/*1860*/
};

//OP_SetDataRate
struct SetDataRate_Struct {
	float newdatarate;
};

//OP_SetServerFilter
struct SetServerFilter_Struct {
	uint32 filters[35];		//see enum eqFilterType [31]
};

//Op_SetServerFilterAck
struct SetServerFilterAck_Struct {
	uint8 blank[8];
};
struct IncreaseStat_Struct{
	/*0000*/	uint8	unknown0;
	/*0001*/	uint8	str;
	/*0002*/	uint8	sta;
	/*0003*/	uint8	agi;
	/*0004*/	uint8	dex;
	/*0005*/	uint8	int_;
	/*0006*/	uint8	wis;
	/*0007*/	uint8	cha;
	/*0008*/	uint8	fire;
	/*0009*/	uint8	cold;
	/*0010*/	uint8	magic;
	/*0011*/	uint8	poison;
	/*0012*/	uint8	disease;
	/*0013*/	char	unknown13[116];
	/*0129*/	uint8	str2;
	/*0130*/	uint8	sta2;
	/*0131*/	uint8	agi2;
	/*0132*/	uint8	dex2;
	/*0133*/	uint8	int_2;
	/*0134*/	uint8	wis2;
	/*0135*/	uint8	cha2;
	/*0136*/	uint8	fire2;
	/*0137*/	uint8	cold2;
	/*0138*/	uint8	magic2;
	/*0139*/	uint8	poison2;
	/*0140*/	uint8	disease2;
};

struct GMName_Struct {
	char oldname[64];
	char gmname[64];
	char newname[64];
	uint8 badname;
	uint8 unknown[3];
};

struct GMDelCorpse_Struct {
	char corpsename[64];
	char gmname[64];
	uint8 unknown;
};

struct GMKick_Struct {
	char name[64];
	char gmname[64];
	uint8 unknown;
};


struct GMKill_Struct {
	char name[64];
	char gmname[64];
	uint8 unknown;
};


struct GMEmoteZone_Struct {
	char text[512];
};

// The BookText_Struct is not used in  SoF and later clients.
// The BookRequest_Struct is used instead for both request and reply.
//
struct BookText_Struct {
	uint8 window;	// where to display the text (0xFF means new window)
	uint8 type;             //type: 0=scroll, 1=book, 2=item info.. prolly
	char booktext[1]; // Variable Length - was 1
};
// This is the request to read a book.
// This is just a "text file" on the server
// or in our case, the 'name' column in our books table.
struct BookRequest_Struct {
/*0000*/	uint32 window;		// where to display the text (0xFFFFFFFF means new window).
/*0004*/	uint16 invslot;		// Is the slot, but the RoF conversion causes it to fail.  Turned to 0 since it isnt required anyway.
/*0008*/	uint32 unknown006;	// Seen FFFFFFFF
/*0010*/	uint16 unknown008;	// seen 0000
/*0012*/	uint32 type;		// 0 = Scroll, 1 = Book, 2 = Item Info. Possibly others
/*0016*/	uint32 unknown0012;
/*0020*/	uint16 unknown0016;
/*0022*/	char txtfile[8194];
};

/*
** Object/Ground Spawn struct
** Used for Forges, Ovens, ground spawns, items dropped to ground, etc
** Size: Variable
** OpCodes: OP_CreateObject
**
*/
struct Object_Struct {
/*00*/	uint32	object_count;		// Iteration count in the object list
/*00*/	char	object_name[1];		// Name of object, usually something like IT63_ACTORDEF
/*00*/	uint16	zone_id;			// Redudant, but: Zone the object appears in
/*00*/	uint16	zone_instance;		//
/*00*/	uint32	drop_id;			// Unique object id for zone
/*00*/	uint32	unknown024;			// 53 9e f9 7e - same for all objects in the zone?
/*00*/	float	heading;			// heading
/*00*/	float	x_tilt;				//Tilt entire object on X axis
/*00*/	float	y_tilt;				//Tilt entire object on Y axis
/*00*/	float	size;				// Size - default 1
/*00*/	float	z;					// z coord
/*00*/	float	x;					// x coord
/*00*/	float	y;					// y coord
/*00*/	int32	object_type;		// Type of object, not directly translated to OP_OpenObject
};
// object_type - 01 = generic drop, 02 = armor, 19 = weapon
// 0xff seems to be indicative of the tradeskill/openable items that end up returning the old style item type in the OP_OpenObject

/*
** Click Object Struct
** Client clicking on zone object (forge, groundspawn, etc)
** Size: 8 bytes
** Last Updated: Oct-17-2003
**
*/
struct ClickObject_Struct {
/*00*/	uint32 drop_id;		// Appears to use the Object Count field now
/*04*/	uint32 player_id;
/*08*/
};

struct Shielding_Struct {
	uint32 target_id;
};

/*
** Click Object Acknowledgement Struct
** Response to client clicking on a World Container (ie, forge)
**
*/
struct ClickObjectAction_Struct {
/*00*/  //uint32 player_id;	// Appears to have been removed
/*00*/	uint32	drop_id;	// Appears to use the object_count field now
/*04*/	int32	unknown04;	// Seen -1
/*08*/	int32	unknown08;	// Seen -1
/*08*/	//uint32 open;		// 1=opening, 0=closing - Removed?
/*12*/	uint32	type;		// See object.h, "Object Types"
/*16*/	uint32	unknown16;	//
/*20*/	uint32	icon;		// Icon to display for tradeskill containers
/*24*/	uint32	unknown24;	//
/*28*/	char	object_name[64]; // Object name to display
/*92*/
};

/*
** This is different now, mostly unknown
**
*/
struct CloseContainer_Struct {
/*00*/	uint32	player_id;	// Entity Id of player who clicked object
/*04*/	uint32	drop_id;	// Zone-specified unique object identifier
/*08*/	uint32	open;		// 1=opening, 0=closing
/*12*/	uint32	unknown12[12];
};

/*
** Generic Door Struct
** Length: 52 Octets
** Used in:
**    cDoorSpawnsStruct(f721)
**
*/
struct Door_Struct
{
/*0000*/ char    name[32];            // Filename of Door // Was 10char long before... added the 6 in the next unknown to it: Daeken M. BlackBlade
/*0032*/ float   yPos;               // y loc
/*0036*/ float   xPos;               // x loc
/*0040*/ float   zPos;               // z loc
/*0044*/ float	 heading;
/*0048*/ uint32   incline;	// rotates the whole door
/*0052*/ uint32   size;			// 100 is normal, smaller number = smaller model
/*0054*/ uint8    unknown0054[4]; // 00 00 00 00
/*0060*/ uint8   doorId;             // door's id #
/*0061*/ uint8   opentype;
/*0062*/ uint8  state_at_spawn;
/*0063*/ uint8  invert_state;	// if this is 1, the door is normally open
/*0064*/ uint32  door_param; // normally ff ff ff ff (-1)
/*0068*/ uint32	unknown0068; // 00 00 00 00
/*0072*/ uint32	unknown0072; // 00 00 00 00
/*0076*/ uint8	unknown0076[4]; // New for RoF
/*0080*/ uint8	unknown0080; // seen 1 or 0
/*0081*/ uint8	unknown0081; // seen 1 (always?)
/*0082*/ uint8	unknown0082; // seen 0 (always?)
/*0083*/ uint8	unknown0083; // seen 1 (always?)
/*0084*/ uint8	unknown0084; // seen 0 (always?)
/*0085*/ uint8	unknown0085; // seen 1 or 0 or rarely 2C or 90 or ED or 2D or A1
/*0086*/ uint8  unknown0086; // seen 0 or rarely FF or FE or 10 or 5A or 82
/*0087*/ uint8  unknown0087; // seen 0 or rarely 02 or 7C
/*0088*/ uint8  unknown0088[8]; // mostly 0s, the last 3 bytes are something tho
/*0096*/
};

struct DoorSpawns_Struct {
	struct Door_Struct doors[0];
};

/*
 OP Code: 	Op_ClickDoor
 Size:		16
*/
struct ClickDoor_Struct {
/*000*/	uint8	doorid;
/*001*/	uint8	unknown001;		// This may be some type of action setting
/*002*/	uint8	unknown002;		// This is sometimes set after a lever is closed
/*003*/	uint8	unknown003;		// Seen 0
/*004*/	uint8	picklockskill;
/*005*/	uint8	unknown005[3];
/*008*/ uint32	item_id;
/*012*/ uint16	player_id;
/*014*/ uint8	unknown014[2];
/*016*/
};

struct MoveDoor_Struct {
	uint8	doorid;
	uint8	action;
};


struct BecomeNPC_Struct {
	uint32 id;
	int32 maxlevel;
};

struct Underworld_Struct {
	float speed;
	float y;
	float x;
	float z;
};

struct Resurrect_Struct
{
/*000*/	uint32	unknown000;
/*004*/	uint16	zone_id;
/*006*/	uint16	instance_id;
/*008*/	float	y;
/*012*/	float	x;
/*016*/	float	z;
/*020*/	uint32	unknown020;
/*024*/	char	your_name[64];
/*088*/	uint32	unknown088;
/*092*/	char	rezzer_name[64];
/*156*/	uint32	spellid;
/*160*/	char	corpse_name[64];
/*224*/	uint32	action;
/*228*/	uint32	unknown228;
/*232*/
};

struct SetRunMode_Struct {
	uint8 mode;                    //01=run  00=walk
	uint8 unknown[3];
};

// EnvDamage is EnvDamage2 without a few bytes at the end.
// Size: 37 bytes
struct EnvDamage2_Struct {
/*0000*/	uint32 id;
/*0004*/	uint16 unknown4;
/*0006*/	uint32 damage;
/*0010*/	float unknown10;	// New to Underfoot - Seen 1
/*0014*/	uint8 unknown14[12];
/*0026*/	uint8 dmgtype; // FA = Lava, FB = Drowning, FC = Falling, FD = Trap
/*0027*/	uint8 unknown27[4];
/*0031*/	uint16 unknown31;	// New to Underfoot - Seen 66
/*0033*/	uint16 constant;		// Always FFFF
/*0035*/	uint16 unknown35;
/*0037*/
};

//Bazaar Stuff

enum {
	BazaarTrader_StartTraderMode = 1,
	BazaarTrader_EndTraderMode = 2,
	BazaarTrader_UpdatePrice = 3,
	BazaarTrader_EndTransaction = 4,
	BazaarSearchResults = 7,
	BazaarWelcome = 9,
	BazaarBuyItem = 10,
	BazaarTrader_ShowItems = 11,
	BazaarSearchDone = 12,
	BazaarTrader_CustomerBrowsing = 13,
	BazaarInspectItem = 18,
	BazaarSearchDone2 = 19,
	BazaarTrader_StartTraderMode2 = 22
};

enum {
	BazaarPriceChange_Fail = 0,
	BazaarPriceChange_UpdatePrice = 1,
	BazaarPriceChange_RemoveItem = 2,
	BazaarPriceChange_AddItem = 3
};

struct BazaarWindowStart_Struct {
	uint8   Action;
	uint8   Unknown001;
	uint16  Unknown002;
};


struct BazaarWelcome_Struct {
	BazaarWindowStart_Struct Beginning;
	uint32  Traders;
	uint32  Items;
	uint8   Unknown012[8];
};

struct BazaarSearch_Struct {
	BazaarWindowStart_Struct Beginning;
	uint32	TraderID;
	uint32	Class_;
	uint32	Race;
	uint32	ItemStat;
	uint32	Slot;
	uint32	Type;
	char	Name[64];
	uint32	MinPrice;
	uint32	MaxPrice;
	uint32	Minlevel;
	uint32	MaxLlevel;
};
struct BazaarInspect_Struct{
	uint32 ItemID;
	uint32 Unknown004;
	char Name[64];
};

struct NewBazaarInspect_Struct {
/*000*/	BazaarWindowStart_Struct Beginning;
/*004*/	char Name[64];
/*068*/	uint32 Unknown068;
/*072*/	int32 SerialNumber;
/*076*/	uint32 Unknown076;
/*080*/	uint32 SellerID;
/*084*/	uint32 Unknown084;
};

struct BazaarReturnDone_Struct{
	uint32 Type;
	uint32 TraderID;
	uint32 Unknown008;
	uint32 Unknown012;
	uint32 Unknown016;
};

struct BazaarSearchResults_Struct {
/*000*/	BazaarWindowStart_Struct Beginning;
/*004*/	uint32	SellerID;
/*008*/	char	SellerName[64];
/*072*/	uint32	NumItems;
/*076*/	uint32	ItemID;
/*080*/	uint32	SerialNumber;
/*084*/	uint32	Unknown084;
/*088*/	char	ItemName[64];
/*152*/	uint32	Cost;
/*156*/	uint32	ItemStat;
/*160*/
};

struct ServerSideFilters_Struct {
uint8	clientattackfilters; // 0) No, 1) All (players) but self, 2) All (players) but group
uint8	npcattackfilters;	 // 0) No, 1) Ignore NPC misses (all), 2) Ignore NPC Misses + Attacks (all but self), 3) Ignores NPC Misses + Attacks (all but group)
uint8	clientcastfilters;	 // 0) No, 1) Ignore PC Casts (all), 2) Ignore PC Casts (not directed towards self)
uint8	npccastfilters;		 // 0) No, 1) Ignore NPC Casts (all), 2) Ignore NPC Casts (not directed towards self)
};

/*
** Client requesting item statistics
** Size: 52 bytes
** Used In: OP_ItemLinkClick
** Last Updated: 01/03/2012
**
*/
struct	ItemViewRequest_Struct {
/*000*/	uint32	item_id;
/*004*/	uint32	augments[6];
/*028*/ uint32	link_hash;
/*032*/	uint32	unknown028;	//seems to always be 4 on SoF client
/*036*/	char	unknown032[12];	//probably includes loregroup & evolving info. see Client::MakeItemLink() in zone/inventory.cpp:469
/*048*/	uint16	icon;
/*050*/	char	unknown046[2];
/*052*/
};

/*
 *  Client to server packet
 */
struct PickPocket_Struct {
// Size 18
    uint32 to;
    uint32 from;
    uint16 myskill;
    uint8 type; // -1 you are being picked, 0 failed , 1 = plat, 2 = gold, 3 = silver, 4 = copper, 5 = item
    uint8 unknown1; // 0 for response, unknown for input
    uint32 coin;
    uint8 lastsix[2];
};
/*
 * Server to client packet
 */

struct sPickPocket_Struct {
	// Size 28 = coin/fail
	uint32 to;
	uint32 from;
	uint32 myskill;
	uint32 type;
	uint32 coin;
	char itemname[64];
};


struct LogServer_Struct {
// Op_Code OP_LOGSERVER
/*000*/	uint32	unknown000;
/*004*/	uint8	enable_pvp;
/*005*/	uint8	unknown005;
/*006*/	uint8	unknown006;
/*007*/	uint8	unknown007;
/*008*/	uint8	enable_FV;
/*009*/	uint8	unknown009;
/*010*/	uint8	unknown010;
/*011*/	uint8	unknown011;
/*012*/	uint32	unknown012;	// htonl(1) on live
/*016*/	uint32	unknown016;	// htonl(1) on live
/*020*/	uint8	unknown020[12];
/*032*/ uint32	unknown032;
/*036*/	char	worldshortname[32];
/*068*/	uint8	unknown068[181];
/*249*/ uint8	unknown249[27];
/*276*/	float	unknown276[7];
/*304*/	uint8	unknown304[256];
/*560*/

/* 	Currently lost
	uint8	enablevoicemacros;
	uint8	enablemail;
*/
};

struct ApproveWorld_Struct {
// Size 544
// Op_Code OP_ApproveWorld
    uint8 unknown544[544];
};

struct ClientError_Struct
{
/*00001*/	char	type;
/*00001*/	char	unknown0001[69];
/*00069*/	char	character_name[64];
/*00134*/	char	unknown134[192];
/*00133*/	char	message[31994];
/*32136*/
};

struct MobHealth
{
	/*0000*/	uint8	hp; //health percent
	/*0001*/	uint16	id;//mobs id
};

struct Track_Struct {
	uint32 entityid;
	float distance;
	// Fields for SoD and later
	uint8 level;
	uint8 is_npc;
	char name[64];
	uint8 is_merc;
};

struct Tracking_Struct {
	uint16 entry_count;
	Track_Struct Entrys[0];
};

struct TrackTarget_Struct
{
	uint32	EntityID;
};

/*
** ZoneServerInfo_Struct
** Zone server information
** Size: 130 bytes
** Used In: OP_ZoneServerInfo
**
*/
struct ZoneServerInfo_Struct
{
/*0000*/	char	ip[128];
/*0128*/	uint16	port;
};

struct WhoAllPlayer{
	uint32	formatstring;
	uint32	pidstring;
	int32	unknown64;		// Seen -1
	char*	name;
	uint32	rankstring;
	char*	guild;
	uint32	unknown80[2];
	uint32	zonestring;
	uint32	zone;
	uint32	class_;
	uint32	level;
	uint32	race;
	char*	account;
	uint32	unknown100;
};

struct WhoAllReturnStruct {
	uint32	id;
	uint32	playerineqstring;
	char	line[27];
	uint8	unknown35;			// 0A
	uint32	unknown36;			// Seen 208243456
	uint32	playersinzonestring;
	uint32	unknown52;			// Same as playercount?
	uint32	unknown44[2];		// 0s
	uint32	unknown56;			// Same as playercount?
	uint32	playercount;		// Player Count in the who list
	struct WhoAllPlayer player[0];
};

// The following four structs are the WhoAllPlayer struct above broken down
// for use in World ClientList::SendFriendsWho to accomodate the user of variable
// length strings within the struct above.

struct	WhoAllPlayerPart1 {
	uint32	FormatMSGID;
	uint32	Unknown04;
	uint32	Unknown08;
	char	Name[1];
};

struct	WhoAllPlayerPart2 {
	uint32	RankMSGID;
	char	Guild[1];
};

struct	WhoAllPlayerPart3 {
	uint32	Unknown80[2];
	uint32	ZoneMSGID;
	uint32	Zone;
	uint32	Class_;
	uint32	Level;
	uint32	Race;
	char	Account[1];
};

struct	WhoAllPlayerPart4 {
	uint32	Unknown100;
};

struct TraderItemSerial_Struct {
	char	SerialNumber[17];
	uint8	Unknown18;
};

struct Trader_Struct {
/*0000*/	uint32	Code;
/*0004*/	TraderItemSerial_Struct	items[200];
/*3604*/	uint32	ItemCost[200];
/*4404*/
};

struct ClickTrader_Struct {
/*0000*/	uint32	Code;
/*0004*/	TraderItemSerial_Struct	items[200];
/*3604*/	uint32	ItemCost[200];
/*4404*/
};

struct GetItems_Struct {
	uint32	items[200];
};

struct BecomeTrader_Struct {
	uint32 id;
	uint32 code;
};

struct Trader_ShowItems_Struct {
/*000*/	uint32 Code;
/*004*/	char   SerialNumber[17];
/*021*/	uint8  Unknown21;
/*022*/	uint16 TraderID;
/*026*/	uint32 Stacksize;
/*030*/	uint32 Price;
/*032*/
};

struct TraderStatus_Struct {
/*000*/	uint32 Code;
/*004*/	uint32 Uknown04;
/*008*/	uint32 Uknown08;
/*012*/
};

struct TraderBuy_Struct {
/*000*/ uint32  action;
/*004*/	uint32	unknown_004;
/*008*/ uint32  price;
/*012*/	uint32	unknown_008;	// Probably high order bits of a 64 bit price.
/*016*/ uint32  trader_id;
/*020*/ char    item_name[64];
/*084*/ uint32  unknown_076;
/*088*/ uint32  item_id;
/*092*/ uint32  already_sold;
/*096*/ uint32  quantity;
/*100*/ uint32  unknown_092;
/*104*/
};

struct TraderItemUpdate_Struct{
	uint32 unknown0;
	uint32 traderid;
	uint8  fromslot;
	uint8  toslot; //7?
	uint16 charges;
};

struct MoneyUpdate_Struct{
	int32 platinum;
	int32 gold;
	int32 silver;
	int32 copper;
};

//struct MoneyUpdate_Struct
//{
//*0000*/ uint32 spawn_id;            // ***Placeholder
//*0004*/ uint32 cointype;           // Coin Type
//*0008*/ uint32 amount;             // Amount
//*0012*/
//};


struct TraderDelItem_Struct{
	uint32 slotid;
	uint32 quantity;
	uint32 unknown;
};

struct TraderClick_Struct{
	uint32 traderid;
	uint32 unknown4[2];
	uint32 approval;
};

struct FormattedMessage_Struct{
	uint32	unknown0;
	uint32	string_id;
	uint32	type;
	char	message[0];
//*0???*/ uint8  unknown0[8];         // ***Placeholder
};
struct SimpleMessage_Struct{
	uint32	string_id;
	uint32	color;
	uint32	unknown8;
};

// Size: 52 + strings
// Other than the strings, all of this packet is network byte order (reverse from normal)
struct GuildMemberEntry_Struct {
	char	name[1];			// variable length
	uint32	level;
	uint32	banker;				// 1=yes, 0=no
	uint32	class_;
	uint32	rank;
	uint32	time_last_on;
	uint32	tribute_enable;
	uint32	unknown01;			// Seen 0
	uint32	total_tribute;		// total guild tribute donated, network byte order
	uint32	last_tribute;		// unix timestamp
	uint32	unknown_one;		// unknown, set to 1
	char	public_note[1];		// variable length.
	uint16	zoneinstance;		// Seen 0s or -1 in RoF
	uint16	zone_id;			// Seen 0s or -1 in RoF
	uint32	unknown_one2;		// unknown, set to 1
	uint32	unknown04;			// Seen 0
};

//just for display purposes, this is not actually used in the message encoding other than for size.
struct GuildMembers_Struct {
	char	player_name[1];		// variable length.
	uint32	guildid;			// Was unknown02 - network byte order
	uint32	count;				// network byte order
	GuildMemberEntry_Struct member[0];
};

struct GuildMOTD_Struct{
/*0000*/	uint32	unknown0;
/*0004*/	char	name[64];
/*0068*/	char	setby_name[64];
/*0132*/	uint32	unknown132;
/*0136*/	char	motd[0]; //was 512
};

struct GuildURL_Struct{
/*0000*/	uint32	unknown0;	//index? seen server send 0 w/ the Guild URL, followed by 1 with nothing.
/*0004*/	uint32	unknown4;
/*0008*/	uint32	unknown8;	//seen 7
/*0012*/	char	setby_name[64];
/*0076*/	uint32	unknown132;	//seen 0xc7de
/*0136*/	char	url[4040];
};

struct GuildStatus_Struct
{
/*000*/	char	Name[64];
/*064*/	uint8	Unknown064[76];
};

struct GuildMemberUpdate_Struct {
/*00*/	uint32	GuildID;
/*04*/	char	MemberName[64];
/*68*/	uint16	ZoneID;
/*70*/	uint16	InstanceID;	//speculated
/*72*/	uint32	LastSeen;	//unix timestamp
/*76*/	uint32	Unknown76;
/*80*/
};

struct GuildMemberLevelUpdate_Struct {
/*00*/	uint32 guild_id;
/*04*/	char	member_name[64];
/*68*/	uint32	level;	//not sure
};

struct GuildUpdate_PublicNote {
	uint32	unknown0;
	char	name[64];
	char	target[64];
	char	note[100]; //we are cutting this off at 100, actually around 252
};

struct GuildDemoteStruct {
/*000*/	char	name[64];
/*064*/	char	target[64];
/*128*/	uint32	rank;			// New in RoF
/*132*/
};

struct GuildRemoveStruct {
/*000*/	char	target[64];
/*064*/	char	name[64];
/*128*/	uint32	GuildID;		// Was unknown128
/*132*/	uint32	leaderstatus;
/*136*/	uint32	unknown136;		// New in RoF
/*140*/
};

struct GuildMakeLeader {
	char	name[64];
	char	target[64];
};

// Server -> Client
// Update a guild members rank and banker status
struct GuildSetRank_Struct
{
/*00*/	uint32	guild_id;	// Was Unknown00
/*04*/	uint32	rank;
/*08*/	char	member_name[64];
/*72*/	uint32	banker;
/*76*/	uint32	unknown76;	// Seen 1 - Maybe Banker?
/*80*/
};

struct Make_Pet_Struct { //Simple struct for getting pet info
	uint8 level;
	uint8 class_;
	uint16 race;
	uint8 texture;
	uint8 pettype;
	float size;
	uint8 type;
	uint32 min_dmg;
	uint32 max_dmg;
};
struct GroundSpawn{
	float max_x;
	float max_y;
	float min_x;
	float min_y;
	float max_z;
	float heading;
	char name[16];
	uint32 item;
	uint32 max_allowed;
	uint32 respawntimer;
};
struct GroundSpawns {
	struct GroundSpawn spawn[50]; //Assigned max number to allow
};

//struct PetitionBug_Struct{
//	uint32	petition_number;
//	uint32	unknown4;
//	char	accountname[64];
//	uint32	zoneid;
//	char	name[64];
//	uint32	level;
//	uint32	class_;
//	uint32	race;
//	uint32	unknown152[3];
//	uint32	time;
//	uint32	unknown168;
//	char	text[1028];
//};

struct ApproveZone_Struct {
	char	name[64];
	uint32	zoneid;
	uint32	approve;
};
struct ZoneInSendName_Struct {
	uint32	unknown0;
	char	name[64];
	char	name2[64];
	uint32	unknown132;
};
struct ZoneInSendName_Struct2 {
	uint32	unknown0;
	char	name[64];
	uint32	unknown68[145];
};

struct StartTribute_Struct {
   uint32	client_id;
   uint32	tribute_master_id;
   uint32	response;
};

struct TributeLevel_Struct {
   uint32	level;	//backwards byte order!
   uint32	tribute_item_id;	//backwards byte order!
   uint32	cost;	//backwards byte order!
};

struct TributeAbility_Struct {
/*000*/	uint32	tribute_id;	//backwards byte order!
/*004*/	uint32	tier_count;	//backwards byte order!
/*008*/	TributeLevel_Struct tiers[MAX_TRIBUTE_TIERS];
/*128*/	uint32	unknown128;	// New to RoF
/*132*/	char	name[0];
};

struct GuildTributeAbility_Struct {
	uint32	guild_id;
	TributeAbility_Struct ability;
};

struct SelectTributeReq_Struct {
   uint32	client_id;	//? maybe action ID?
   uint32	tribute_id;
   uint32	unknown8;	//seen E3 00 00 00
};

struct SelectTributeReply_Struct {
   uint32	client_id;	//echoed from request.
   uint32	tribute_id;
   char	desc[0];
};

struct TributeInfo_Struct {
	uint32	active;		//0 == inactive, 1 == active
	uint32	tributes[MAX_PLAYER_TRIBUTES];	//-1 == NONE
	uint32	tiers[MAX_PLAYER_TRIBUTES];		//all 00's
	uint32	tribute_master_id;
};

struct TributeItem_Struct
{
/*00*/	InventorySlot_Struct	inventory_slot;
/*12*/	uint32	quantity;
/*16*/	uint32	tribute_master_id;
/*20*/	int32	tribute_points;
/*24*/
};

struct TributePoint_Struct {
	int32   tribute_points;
	uint32   unknown04;
	int32   career_tribute_points;
	uint32   unknown12;
};

struct TributeMoney_Struct {
	uint32   platinum;
	uint32   tribute_master_id;
	int32   tribute_points;
};


struct Split_Struct
{
	uint32	platinum;
	uint32	gold;
	uint32	silver;
	uint32	copper;
};


/*
** New Combine Struct
** Client requesting to perform a tradeskill combine
** Size: 24 bytes
** Used In: OP_TradeSkillCombine
** Last Updated: 01-05-2013
*/
struct NewCombine_Struct
{
/*00*/	InventorySlot_Struct container_slot;
/*12*/	InventorySlot_Struct guildtribute_slot;	// Slot type is 8? (MapGuildTribute = 8)
/*24*/
};


//client requesting favorite recipies
struct TradeskillFavorites_Struct {
	uint32 object_type;
	uint32 some_id;
	uint32 favorite_recipes[500];
};

//search request
struct RecipesSearch_Struct {
	uint32 object_type;	//same as in favorites
	uint32 some_id;			//same as in favorites
	uint32 mintrivial;
	uint32 maxtrivial;
	char query[56];
	uint32 unknown4;	//is set to 00 03 00 00
	uint32 unknown5; //is set to 4C DD 12 00
/*80*/
};

//one sent for each item, from server in reply to favorites or search
struct RecipeReply_Struct {
	uint32 object_type;
	uint32 some_id;	 //same as in favorites
	uint32 component_count;
	uint32 recipe_id;
	uint32 trivial;
	char recipe_name[64];
/*84*/
};

//received and sent back as an ACK with different reply_code
struct RecipeAutoCombine_Struct
{
/*00*/	uint32 object_type;
/*04*/	uint32 some_id;
/*08*/	InventorySlot_Struct container_slot;		//echoed in reply - Was uint32 unknown1
/*20*/	InventorySlot_Struct unknown_slot;		//echoed in reply
/*32*/	uint32 recipe_id;
/*36*/	uint32 reply_code;
/*40*/
};

struct LevelAppearance_Struct { //Sends a little graphic on level up
	uint32	spawn_id;
	uint32	parm1;
	uint32	value1a;
	uint32	value1b;
	uint32	parm2;
	uint32	value2a;
	uint32	value2b;
	uint32	parm3;
	uint32	value3a;
	uint32	value3b;
	uint32	parm4;
	uint32	value4a;
	uint32	value4b;
	uint32	parm5;
	uint32	value5a;
	uint32	value5b;
/*64*/
};
struct MerchantList{
	uint32	id;
	uint32	slot;
	uint32	item;
};
struct TempMerchantList{
	uint32	npcid;
	uint32	slot;
	uint32	item;
	uint32	charges; //charges/quantity
	uint32	origslot;
};


struct FindPerson_Point {
	float y;
	float x;
	float z;
};

struct FindPersonRequest_Struct {
/*00*/	uint32	unknown00;
/*04*/	uint32	npc_id;
/*08*/	uint32	unknown08;
/*12*/	uint32	unknown12;
/*16*/	FindPerson_Point client_pos;
/*28*/	uint32	unknown28;
/*32*/	uint32	unknown32;
/*36*/	uint32	unknown36;
};

//variable length packet of points
struct FindPersonResult_Struct {
	FindPerson_Point dest;
	FindPerson_Point path[0];	//last element must be the same as dest
};

struct MobRename_Struct {
/*000*/	char	old_name[64];
/*064*/	char	old_name_again[64];	//not sure what the difference is
/*128*/	char	new_name[64];
/*192*/	uint32	unknown192;		//set to 0
/*196*/	uint32	unknown196;		//set to 1
/*200*/
};

struct PlayMP3_Struct {
	char filename[0];
};

//this is for custom title display in the skill window
struct TitleEntry_Struct {
	uint32	skill_id;
	uint32	skill_value;
	char	title[1];
};

struct Titles_Struct {
	uint32	title_count;
	TitleEntry_Struct titles[0];
};

//this is for title selection by the client
struct TitleListEntry_Struct {
	uint32	unknown0;	//title ID
	char prefix[1];		//variable length, null terminated
	char postfix[1];		//variable length, null terminated
};

struct TitleList_Struct {
	uint32 title_count;
	TitleListEntry_Struct titles[0];	//list of title structs
	//uint32 unknown_ending; seen 0x7265, 0
};

struct SetTitle_Struct {
	uint32	is_suffix;	//guessed: 0 = prefix, 1 = suffix
	uint32	title_id;
};

struct SetTitleReply_Struct {
	uint32	is_suffix;	//guessed: 0 = prefix, 1 = suffix
	char	title[32];
	uint32	entity_id;
};


#if 0
// Old struct not used by Task System implementation but left for reference
struct TaskDescription_Struct {
/*000*/	uint32	activity_count;		//not right.
/*004*/	uint32	taskid;
/*008*/	uint8	unk;
/*009*/	uint32	id3;
/*013*/	uint32	unknown13;
/*017*/	char	name[1];	//variable length, 0 terminated
/*018*/	uint32	unknown18;
/*022*/	uint32	unknown22;
/*026*/	uint32	unknown26;
/*030*/	char	desc[1];	//variable length, 0 terminated
/*031*/	uint32	reward_count;   //not sure
/*035*/	uint8	unknown31;
/*036*/	uint32	unknown31;
/*040*/	uint32	unknown35;
/*044*/	uint16	unknown39;
/*046*/	char	reward_link[1];	//variable length, 0 terminated
/*047*/	uint32	unknown43;  //maybe crystal count?
/*051*/
};
#endif

struct TaskMemberList_Struct {
/*00*/  uint32  gopher_id;
/*04*/  uint32  unknown04;
/*08*/  uint32  member_count;   //1 less than the number of members
/*12*/  char	list_pointer[0];
/*	list is of the form:
	char member_name[1]	//null terminated string
	uint8   task_leader	//boolean flag
*/
};

#if 0
// Struct not used by Task System implentation but left for reference (current for RoF)
struct TaskActivity_Struct {
/*000*/	uint32	TaskSequenceNumber;
/*004*/	uint32	unknown2;
/*008*/	uint32	TaskID;
/*012*/	uint32	ActivityID;
/*016*/	uint32	unknown3;
/*020*/	uint32	ActivityType;
/*024*/	uint32	Optional;
/*028*/	uint8	unknown5;
/*032*/	char	Text1[1];			// Variable length - Null terminated
/*000*/	uint32	Text2Len;			// Lenth of the following string
/*000*/	char	Text2[1];			// Variable length - not Null terminated
/*000*/	uint32	GoalCount;
/*000*/	uint32	String1Len;			// Lenth of the following string - Seen 2
/*000*/	char	String1[1];			// Numeric String - Seen "-1" - not Null terminated
/*000*/	uint32	String2Len;			// Lenth of the following string - Seen 2
/*000*/	char	String2[1];			// Numeric String - Seen "-1" - not Null terminated
/*000*/	char	ZoneIDString1[1];	// Numeric String - Seen "398" - Null terminated
/*000*/	uint32	unknown7;			// Seen 0
/*000*/	char 	Text3[1];			// Variable length - Null terminated
/*000*/	uint32	DoneCount;
/*000*/	uint8	unknown9;			// Seen 1
/*000*/	char	ZoneIDString2[1];	// Numeric String - Seen "398" - Null terminated
};

struct TaskHistoryEntry_Struct {
	uint32	task_id;
	char	name[1];
	uint32	completed_time;
};
struct TaskHistory_Struct {
	uint32 completed_count;
	TaskHistoryEntry_Struct entries[0];
};
#endif

struct AcceptNewTask_Struct {
	uint32  unknown00;
	uint32	task_id;		//set to 0 for 'decline'
	uint32	task_master_id;	//entity ID
};

//was all 0's from client, server replied with same op, all 0's
struct CancelTask_Struct {
	uint32 SequenceNumber;
	uint32 unknown4; // Only seen 0x00000002
};

#if 0
// old struct, not used by Task System implementation but left for reference.
struct AvaliableTask_Struct {
	uint32	task_index;		//no idea, seen 0x1
	uint32	task_master_id;	//entity ID
	uint32	task_id;
	uint32	unknown012;
	uint32	activity_count;	//not sure, seen 2
	char desc[1];	//variable length, 0 terminated
	uint32	reward_platinum;//not sure on these
	uint32	reward_gold;
	uint32	reward_silver;
	uint32	reward_copper;
	char some_name[1];	//variable length, 0 terminated
	uint8	unknown1;
	uint32	unknown2;	//0xFFFFFFFF
	uint32	unknown3;	//0xFFFFFFFF
	uint32	unknown4;	//seen 0x16
	uint8	unknown5;
};
#endif


// Many of the Task System packets contain variable length strings, as well as variable numbers
// of records, hence splitting them into multiple structs (header, middle, trailer) etc.
//
struct AvailableTaskHeader_Struct {
	uint32	TaskCount;
	uint32	unknown1;
	uint32	TaskGiver;
};

struct AvailableTaskData1_Struct {
	uint32	TaskID;
	uint32	unknown1;
	uint32	TimeLimit;
	uint32	unknown2;
};

struct AvailableTaskData2_Struct {
	uint32	unknown1,unknown2,unknown3,unknown4;
};

struct AvailableTaskTrailer_Struct {
	uint32	ItemCount;
	uint32	unknown1, unknown2;
	uint32	StartZone;
};

struct TaskDescriptionHeader_Struct {
	uint32	SequenceNumber; // The order the tasks appear in the journal. 0 for first task, 1 for second, etc.
	uint32	TaskID;
	uint32	unknown2;
	uint32	unknown3;
	uint8	unknown4;
};

struct TaskDescriptionData1_Struct {
	uint32	Duration;
	uint32	unknown2;
	uint32	StartTime;
};

struct TaskDescriptionData2_Struct {
	uint32	RewardCount; // ??
	uint32	unknown1;
	uint32	unknown2;
	uint16	unknown3;
	//uint8	unknown4;
};

struct TaskDescriptionTrailer_Struct {
	//uint16  unknown1; // 0x0012
	uint32	Points;
};

struct TaskActivityHeader_Struct {
	uint32	TaskSequenceNumber;
	uint32	unknown2; // Seen 0x00000002
	uint32	TaskID;
	uint32	ActivityID;
	uint32	unknown3;
	uint32	ActivityType;
	uint32	Optional;
	uint32	unknown5;
};

struct TaskActivityData1_Struct {
	uint32	GoalCount;
	uint32	unknown1; // 0xffffffff
	uint32	unknown2; // 0xffffffff
	uint32	ZoneID; // seen 0x36
	uint32  unknown3;
};

struct TaskActivityTrailer_Struct {
	uint32	DoneCount;
	uint32	unknown1; // Seen 1
};

// The Short_Struct is sent for tasks that are hidden and act as a placeholder
struct TaskActivityShort_Struct {
	uint32	TaskSequenceNumber;
	uint32	unknown2; // Seen 0x00000002
	uint32	TaskID;
	uint32	ActivityID;
	uint32	unknown3;
	uint32	ActivityType; // 0xffffffff for the short packet
	uint32  unknown4;
};

struct TaskActivityComplete_Struct {
	uint32	TaskIndex;
	uint32	unknown2; // 0x00000002
	uint32	unknown3;
	uint32	ActivityID;
	uint32	unknown4; // 0x00000001
	uint32	unknown5; // 0x00000001
};

#if 0
// This is a dupe of the CancelTask struct
struct TaskComplete_Struct {
	uint32	unknown00; // 0x00000000
	uint32	unknown04; // 0x00000002
};
#endif

struct TaskHistoryRequest_Struct {
	uint32	TaskIndex; // This is the sequence the task was sent in the Completed Tasks packet.
};

struct TaskHistoryReplyHeader_Struct {
	uint32	TaskID;
	uint32	ActivityCount;
};

struct TaskHistoryReplyData1_Struct {
	uint32	ActivityType;
};

struct TaskHistoryReplyData2_Struct {
	uint32  GoalCount;
	uint32	unknown04; // 0xffffffff
	uint32	unknown08; // 0xffffffff
	uint32	ZoneID;
	uint32	unknown16;
};

struct BankerChange_Struct {
/*00*/	uint32	platinum;
/*04*/	uint32	gold;
/*08*/	uint32	silver;
/*12*/	uint32	copper;
/*16*/	uint32	platinum_bank;
/*20*/	uint32	gold_bank;
/*24*/	uint32	silver_bank;
/*28*/	uint32	copper_bank;
/*32*/
};

struct LeadershipExpUpdate_Struct {
/*00*/	double	group_leadership_exp;
/*08*/	uint32	group_leadership_points;
/*12*/	uint32	Unknown12;
/*16*/	double	raid_leadership_exp;
/*24*/	uint32	raid_leadership_points;
};

struct UpdateLeadershipAA_Struct {
/*00*/	uint32	ability_id;
/*04*/	uint32	new_rank;
/*08*/	uint32	unknown08;
/*12*/
};

/**
* Leadership AA update
* Length: 32 Octets
* OpCode: LeadExpUpdate
*/
struct leadExpUpdateStruct {
   /*0000*/ uint32 unknown0000;          // All zeroes?
   /*0004*/ uint32 group_leadership_exp;         // Group leadership exp value
   /*0008*/ uint32 group_leadership_points;   // Unspent group points
   /*0012*/ uint32 unknown0012;          // Type?
   /*0016*/ uint32 unknown0016;          // All zeroes?
   /*0020*/ uint32 raid_leadership_exp;          // Raid leadership exp value
   /*0024*/ uint32 raid_leadership_points;    // Unspent raid points
   /*0028*/ uint32 unknown0028;
};

struct RaidGeneral_Struct {
/*00*/	uint32		action;
/*04*/	char		player_name[64];
/*68*/	uint32		unknown68;
/*72*/	char		leader_name[64];
/*136*/	uint32		parameter;
};

struct RaidAddMember_Struct {
/*000*/ RaidGeneral_Struct raidGen; //param = (group num-1); 0xFFFFFFFF = no group
/*136*/ uint8 _class;
/*137*/	uint8 level;
/*138*/	uint8 isGroupLeader;
/*139*/	uint8 flags[5]; //no idea if these are needed...
};

struct RaidNote_Struct {
/*000*/ RaidGeneral_Struct general;
/*140*/ char note[64];
};

struct RaidMOTD_Struct {
/*000*/ RaidGeneral_Struct general; // leader_name and action only used
/*140*/ char motd[1024]; // max size is 1024, but reply is variable
};

struct RaidLeadershipUpdate_Struct {
/*000*/	uint32 action;
/*004*/	char player_name[64];
/*068*/	uint32 Unknown068;
/*072*/	char leader_name[64];
/*136*/	GroupLeadershipAA_Struct group; //unneeded
/*200*/	RaidLeadershipAA_Struct raid;
/*264*/	char Unknown264[128];
};

struct RaidAdd_Struct {
/*000*/	uint32		action;	//=0
/*004*/	char		player_name[64];	//should both be the player's name
/*068*/	char		leader_name[64];
/*132*/	uint8		_class;
/*133*/	uint8		level;
/*134*/	uint8		has_group;
/*135*/	uint8		unknown135;	//seems to be 0x42 or 0
};

struct RaidCreate_Struct {
/*00*/	uint32		action;	//=8
/*04*/	char		leader_name[64];
/*68*/	uint32		leader_id;
};

struct RaidMemberInfo_Struct {
/*00*/	uint8		group_number;
/*01*/	char		member_name[1];		//dyanmic length, null terminated '\0'
/*00*/	uint8		unknown00;
/*01*/	uint8		_class;
/*02*/	uint8		level;
/*03*/	uint8		is_raid_leader;
/*04*/	uint8		is_group_leader;
/*05*/	uint8		main_tank;		//not sure
/*06*/	uint8		unknown06[5];	//prolly more flags
};

struct RaidDetails_Struct {
/*000*/	uint32		action;	//=6,20
/*004*/	char		leader_name[64];
/*068*/	uint32		unknown68[4];
/*084*/	LeadershipAA_Struct abilities;	//ranks in backwards byte order
/*128*/	uint8		unknown128[142];
/*354*/	uint32		leader_id;
};

struct RaidMembers_Struct {
/*000*/	RaidDetails_Struct		details;
/*358*/	uint32					member_count;		//including leader
/*362*/	RaidMemberInfo_Struct	members[1];
/*...*/	RaidMemberInfo_Struct	empty;	//seem to have an extra member with a 0 length name on the end
};

struct DynamicWall_Struct {
/*00*/  char	name[32];
/*32*/  float   y;
/*36*/  float   x;
/*40*/  float   z;
/*44*/  uint32  something;
/*48*/  uint32  unknown48;  //0
/*52*/  uint32  one_hundred;	//0x64
/*56*/  uint32  unknown56;  //0
/*60*/  uint32  something2;
/*64*/  int32  unknown64;  //-1
/*68*/  uint32  unknown68;  //0
/*72*/  uint32  unknown72;  //0
/*76*/  uint32  unknown76;  //0x100
/*80*/
};

// Bandolier actions
enum
{
	bandolierCreate = 0,
	bandolierRemove,
	bandolierSet
};

struct BandolierCreate_Struct
{
	/*00*/	uint32 Action;		//0 for create
	/*04*/	uint8 Number;
	/*05*/	char Name[32];
	/*37*/	uint16 Unknown37;	//seen 0x93FD
	/*39*/	uint8 Unknown39;	//0
};

struct BandolierDelete_Struct
{
	/*00*/	uint32 Action;
	/*04*/	uint8 Number;
	/*05*/	uint8 Unknown05[35];
};

struct BandolierSet_Struct
{
	/*00*/	uint32 Action;
	/*04*/	uint8 Number;
	/*05*/	uint8 Unknown05[35];
};

struct Arrow_Struct {
/*000*/	float	src_y;
/*004*/	float	src_x;
/*008*/	float	src_z;
/*012*/	uint8	unknown012[12];
/*024*/	float	velocity;		//4 is normal, 20 is quite fast
/*028*/	float	launch_angle;	//0-450ish, not sure the units, 140ish is straight
/*032*/	float	tilt;		//on the order of 125
/*036*/	uint8	unknown036[8];
/*044*/	float	arc;
/*048*/	uint32	source_id;
/*052*/	uint32	target_id;	//entity ID
/*056*/	uint32	item_id;
/*060*/	uint32	unknown060;
/*064*/	uint32	unknown064;
/*068*/	uint8	unknown068;
/*069*/	uint8	unknown069;
/*070*/	uint8	unknown070;
/*071*/	uint8	item_type;
/*072*/	uint8	skill;
/*073*/	uint8	unknown073[16];
/*089*/	char	model_name[27];
/*116*/
};

//made a bunch of trivial structs for stuff for opcode finder to use
struct Consent_Struct {
	char name[1];	//always at least a null - was 1
};

struct AdventureMerchant_Struct {
	uint32	unknown_flag;		//seems to be 1
	uint32	entity_id;
};

// OP_Save - Size: 484
struct Save_Struct {
/*000*/	uint8	unknown00[192];
/*192*/	uint8	unknown0192[176];
/*368*/	uint8	unknown0368[116];
/*484*/
};

struct GMToggle_Struct {
	uint8 unknown0[64];
	uint32 toggle;
};

struct ColoredText_Struct {
	uint32 color;
	char msg[1]; //was 1
/*0???*/ uint8  paddingXXX[3];          // always 0's
};

struct UseAA_Struct {
	uint32 begin;
	uint32 ability;
	uint32 end;
};

struct AA_Ability {
/*00*/	uint32 skill_id;
/*04*/	uint32 base_value;
/*08*/	uint32 limit_value;
/*12*/	uint32 slot;
/*16*/
};

struct SendAA_Struct {
/*0000*/	uint32 id;
/*0004*/	uint8 unknown004;		// uint32 unknown004; set to 1.
/*0005*/	int32 hotkey_sid;
/*0009*/	int32 hotkey_sid2;
/*0013*/	uint32 title_sid;
/*0017*/	uint32 desc_sid;
/*0021*/	uint32 class_type;
/*0025*/	uint32 cost;
/*0029*/	uint32 seq;
/*0033*/	uint32 current_level; //1s, MQ2 calls this AARankRequired
/*0037*/	uint32 prereq_skill_count;	// mutliple prereqs at least 1, even no prereqs
/*0041*/	uint32 prereq_skill;		//is < 0, abs() is category #
/*0045*/	uint32 prereq_minpoints_count;	// mutliple prereqs at least 1, even no prereqs
/*0049*/	uint32 prereq_minpoints; //min points in the prereq
/*0053*/	uint32 type;
/*0057*/	uint32 spellid;
/*0061*/	uint32 unknown057;	// Introduced during HoT - Seen 1 - Maybe account status or enable/disable AA?
/*0065*/	uint32 spell_type;
/*0069*/	uint32 spell_refresh;
/*0073*/	uint16 classes;
/*0075*/	uint16 berserker; //seems to be 1 if its a berserker ability
/*0077*/	uint32 max_level;
/*0081*/	uint32 last_id;
/*0085*/	uint32 next_id;
/*0089*/	uint32 cost2;
/*0093*/	uint8 unknown93;
/*0094*/	uint8 grant_only; // VetAAs, progression, etc
/*0095*/	uint8 unknown95; // 1 for skill cap increase AAs, Mystical Attuning, and RNG attack inc, doesn't seem to matter though
/*0096*/	uint32 expendable_charges; // max charges of the AA
/*0100*/	uint32 aa_expansion;
/*0104*/	uint32 special_category;
/*0108*/	uint8 shroud;
/*0109*/	uint8 unknown109;
/*0110*/	uint8 reset_on_death; // timer is reset on death
/*0111*/	uint8 unknown111;
/*0112*/	uint32 total_abilities;
/*0116*/	AA_Ability abilities[0];
};

struct AA_List {
	SendAA_Struct* aa[0];
};

struct AA_Action {
/*00*/	uint32	action;
/*04*/	uint32	ability;
/*08*/	uint32	target_id;
/*12*/	uint32	exp_value;
/*16*/
};

struct AAExpUpdate_Struct {
/*00*/	uint32 unknown00;	//seems to be a value from AA_Action.ability
/*04*/	uint32 aapoints_unspent;
/*08*/	uint8 aaxp_percent;	//% of exp that goes to AAs
/*09*/	uint8 unknown09[3];	//live doesn't always zero these, so they arnt part of aaxp_percent
/*12*/
};

struct AltAdvStats_Struct {
/*000*/  uint32 experience;
/*004*/  uint16 unspent;
/*006*/  uint16	unknown006;
/*008*/  uint8	percentage;
/*009*/  uint8	unknown009[3];
/*012*/
};

struct PlayerAA_Struct {						// Is this still used?
	AA_Array aa_list[MAX_PP_AA_ARRAY];
};

struct AATable_Struct {
/*00*/ uint32 aa_spent;				// Total AAs Spent
/*04*/ uint32 aapoints_assigned;	// Number of Assigned AA points - Seen 206 (total of the 4 fields below)
/*08*/ uint32 aa_spent_general;		// Seen 63
/*12*/ uint32 aa_spent_archetype;	// Seen 40
/*16*/ uint32 aa_spent_class;		// Seen 103
/*20*/ uint32 aa_spent_special;		// Seen 0
/*24*/ AA_Array aa_list[MAX_PP_AA_ARRAY];
};

struct Weather_Struct {
	uint32	val1;	//generall 0x000000FF
	uint32	type;	//0x31=rain, 0x02=snow(i think), 0 = normal
	uint32	mode;
};

struct ZoneInUnknown_Struct {
	uint32	val1;
	uint32	val2;
	uint32	val3;
};

struct MobHealth_Struct {
	uint16 entity_id;
	uint8 hp;
};

struct AnnoyingZoneUnknown_Struct {
	uint32	entity_id;
	uint32	value;		//always 4
};

struct LoadSpellSet_Struct {
	uint8	spell[12];	// 0xFFFFFFFF if no action, slot number if to unmem starting at 0
	uint32	unknown;	//Seen 12 - Maybe a gem count?
};

struct BlockedBuffs_Struct
{
/*000*/ int32 SpellID[BLOCKED_BUFF_COUNT];
/*120*/ uint32 Count;
/*124*/ uint8 Pet;
/*125*/ uint8 Initialise;
/*126*/ uint16 Flags;
};

//Size 24 Bytes
struct WorldObfuscator_Struct {
/*000*/ uint32 var1;
/*004*/ uint32 Unknown1;
/*008*/ uint32 Unknown2;
/*012*/ uint32 Unknown3;
/*016*/ uint32 var2;
/*020*/ uint32 Unknown4;
/*024*/
};

struct ExpansionInfo_Struct {
/*000*/	char	Unknown000[64];
/*064*/	uint32	Expansions;
};

struct ApplyPoison_Struct
{
	TypelessInventorySlot_Struct inventory_slot;
	uint32 success;
};

struct ItemVerifyRequest_Struct
{
/*000*/	InventorySlot_Struct inventory_slot;
/*012*/	uint32	target;		// Target Entity ID
/*016*/
};

struct ItemVerifyReply_Struct
{
/*000*/	InventorySlot_Struct inventory_slot;
/*012*/	uint32	spell;		// Spell ID to cast if different than item effect
/*016*/	uint32	target;		// Target Entity ID
/*020*/
};


struct RoFSlotStruct
{
	uint8	Bank;
	uint16	MainSlot;
	uint16	SubSlot;
};

struct ItemSerializationHeader
{
/*000*/	char unknown000[17];	// New for HoT. Looks like a string.
/*017*/	uint32 stacksize;
/*021*/	uint32 unknown004;
/*025*/	uint8  slot_type;	// 0 = normal, 1 = bank, 2 = shared bank, 9 = merchant, 20 = ?
/*026*/	uint16 main_slot;
/*028*/ uint16 sub_slot;
/*030*/ uint16 aug_slot;	// 0xffff
/*032*/	uint32 price;
/*036*/	uint32 merchant_slot; //1 if not a merchant item
/*040*/	uint32 scaled_value; //0
/*044*/	uint32 instance_id; //unique instance id if not merchant item, else is merchant slot
/*048*/	uint32 unknown028; //0
/*052*/	uint32 last_cast_time;	// Unix Time from PP of last cast for this recast type if recast delay > 0
/*056*/	uint32 charges; //Total Charges an item has (-1 for unlimited)
/*060*/	uint32 inst_nodrop;	// 1 if the item is no drop (attuned items)
/*064*/	uint32 unknown044;	// 0
/*068*/	uint32 unknown048;	// 0
/*072*/	uint32 unknown052;	// 0
		uint8 isEvolving;
};

struct EvolvingItem {
	uint8 unknown001;
	uint8 unknown002;
	uint8 unknown003;
	uint8 unknown004;
	int32 evoLevel;
	double progress;
	uint8 Activated;
	int32 evomaxlevel;
	uint8 unknown005[4];
};

struct ItemSerializationHeaderFinish
{
/*079*/	uint32 ornamentIcon;
/*083*/	int32 unknowna1;	// 0xffffffff
/*087*/	uint32 ornamentHeroModel;	// 0
/*091*/	uint8 unknown063;	// 0
/*092*/	uint32 unknowna3;	// 0
/*096*/	int32 unknowna4;	// 0xffffffff
/*100*/	uint32 unknowna5;	// 0
/*104*/	uint8 ItemClass; //0, 1, or 2
/*105*/
};

struct ItemBodyStruct
{
	uint32 id;
	int32 weight;	// Seen an item on Live with -0.1 weight
	uint8 norent;
	uint8 nodrop;
	uint8 attune;
	uint8 size;
	uint32 slots;
	uint32 price;
	uint32 icon;
	uint8 unknown1;
	uint8 unknown2;
	uint32 BenefitFlag;
	uint8 tradeskills;
	int8 CR;
	int8 DR;
	int8 PR;
	int8 MR;
	int8 FR;
	int8 SVCorruption;
	int8 AStr;
	int8 ASta;
	int8 AAgi;
	int8 ADex;
	int8 ACha;
	int8 AInt;
	int8 AWis;
	int32 HP;
	int32 Mana;
	uint32 Endur;
	int32 AC;
	int32 regen;
	int32 mana_regen;
	int32 end_regen;
	uint32 Classes;
	uint32 Races;
	uint32 Deity;
	int32 SkillModValue;
	int32 SkillModMax;		// Max skill point modification
	int32 SkillModType;
	uint32 SkillModExtra;	// Adds a "+value" after the mod percentage
	uint32 BaneDmgRace;
	uint32 BaneDmgBody;
	uint32 BaneDmgRaceAmt;
	int32 BaneDmgAmt;
	uint8 Magic;
	int32 CastTime_;
	uint32 ReqLevel;
	uint32 RecLevel;
	uint32 RecSkill;
	uint32 BardType;
	int32 BardValue;
	uint8 Light;
	uint8 Delay;
	uint8 ElemDmgType;
	uint8 ElemDmgAmt;
	uint8 Range;
	uint32 Damage;
	uint32 Color;
	uint32 Prestige;	// New to March 21 2012 client
	uint8 ItemType;
	uint32 Material;
	uint32 MaterialUnknown1;
	uint32 EliteMaterial;
	uint32 HerosForgeModel;		// New to March 21 2012 client
	uint32 MaterialUnknown2;	// New to December 10th 2012 client - NEW
	float  SellRate;
	int32 CombatEffects;
	int32 Shielding;
	int32 StunResist;
	int32 StrikeThrough;
	int32 ExtraDmgSkill;
	int32 ExtraDmgAmt;
	int32 SpellShield;
	int32 Avoidance;
	int32 Accuracy;
	uint32 CharmFileID;
	uint32 FactionMod1;
	int32 FactionAmt1;
	uint32 FactionMod2;
	int32 FactionAmt2;
	uint32 FactionMod3;
	int32 FactionAmt3;
	uint32 FactionMod4;
	int32 FactionAmt4;
};

struct AugSlotStruct
{
	uint32 type;
	uint8 visible;
	uint8 unknown;
};

struct ItemSecondaryBodyStruct
{
	uint32 augtype;
	// swapped augrestrict and augdistiller positions
	// (this swap does show the proper augment restrictions in Item Information window now)
	// unsure what the purpose of augdistiller is at this time 3/17/2014
	uint32 augdistiller;	// New to December 10th 2012 client - NEW
	uint32 augrestrict;
	AugSlotStruct augslots[6];

	uint32 ldonpoint_type;
	uint32 ldontheme;
	uint32 ldonprice;
	uint32 ldonsellbackrate;
	uint32 ldonsold;

	uint8 bagtype;
	uint8 bagslots;
	uint8 bagsize;
	uint8 wreduction;

	uint8 book;
	uint8 booktype;
	//int32 filename; filename is either 0xffffffff/0x00000000 or the null term string ex: CREWizardNote\0
};

struct ItemTertiaryBodyStruct
{
	int32 loregroup;
	uint8 artifact;
	uint8 summonedflag;
	uint32 favor;
	uint8 fvnodrop;
	int32 dotshield;
	int32 atk;
	int32 haste;
	int32 damage_shield;
	uint32 guildfavor;
	uint32 augdistil;
	int32 unknown3;	// 0xffffffff
	uint32 unknown4;
	uint8 no_pet;
	uint8 unknown5;

	uint8 potion_belt_enabled;
	uint32 potion_belt_slots;

	uint32 stacksize;
	uint8 no_transfer;
	uint16 expendablearrow;

	uint32 unknown8;
	uint32 unknown9;
	uint32 unknown10;
	uint32 unknown11;
	uint8 unknown12;
	uint8 unknown13;
	uint8 unknown14;
};

struct ClickEffectStruct
{
	int32 effect;
	uint8 level2;
	uint32 type;
	uint8 level;
	int32 max_charges;
	int32 cast_time;
	uint32 recast;
	int32 recast_type;
	uint32 clickunk5;
	//uint8 effect_string;
	//int32 clickunk7;
};

struct ProcEffectStruct
{
	int32 effect;
	uint8 level2;
	uint32 type;
	uint8 level;
	uint32 unknown1; // poison?
	uint32 unknown2;
	uint32 unknown3;
	uint32 unknown4;
	uint32 procrate;
	//uint8 effect_string;
	//uint32 unknown5;
};

struct WornEffectStruct //worn, focus and scroll effect
{
	int32 effect;
	uint8 level2;
	uint32 type;
	uint8 level;
	uint32 unknown1;
	uint32 unknown2;
	uint32 unknown3;
	uint32 unknown4;
	uint32 unknown5;
	//uint8 effect_string;
	//uint32 unknown6;
};

struct ItemQuaternaryBodyStruct
{
	uint32 scriptfileid;
	uint8 quest_item;
	uint32 Power; // Enables "Power" percentage field used by Power Sources
	uint32 Purity;
	uint8  unknown16;	// RoF
	uint32 BackstabDmg;
	uint32 DSMitigation;
	int32 HeroicStr;
	int32 HeroicInt;
	int32 HeroicWis;
	int32 HeroicAgi;
	int32 HeroicDex;
	int32 HeroicSta;
	int32 HeroicCha;
	int32 HeroicMR;
	int32 HeroicFR;
	int32 HeroicCR;
	int32 HeroicDR;
	int32 HeroicPR;
	int32 HeroicSVCorrup;
	int32 HealAmt;
	int32 SpellDmg;
	int32 Clairvoyance;
	int32 SubType;
	uint8 evolve_string; // Some String, but being evolution related is just a guess
	uint8 unknown19;
	uint32 unknown20;	// Bard Stuff?
	//uint32 unknown21;
	uint8 unknown22;
	uint32 unknown23;
	uint32 unknown24;
	uint32 unknown25;
	float unknown26;
	float unknown27;
	uint32 unknown_RoF6;	// 0 New to March 21 2012 client
	uint32 unknown28;	// 0xffffffff
	uint16 unknown29;
	uint32 unknown30;	// 0xffffffff
	uint16 unknown31;
	uint32 unknown32;
	float  unknown33;
	uint32 unknown34;
	uint32 unknown35;
	uint32 unknown36;
	uint32 unknown37;
	uint32 unknown_RoF7;
	uint32 unknown_RoF8;
	uint8 unknown38;	// 0
	uint8 unknown39;	// 1
};

struct AugmentInfo_Struct
{
/*000*/ uint32	itemid;			// id of the solvent needed
/*004*/ uint32	window;			// window to display the information in
/*008*/ char	augment_info[64];	// total packet length 76, all the rest were always 00
/*072*/ uint32	unknown072;
};

struct VeteranRewardItem
{
/*000*/	uint32 name_length;
/*004*/	//char item_name[0]; // THIS IS NOT NULL TERMED
/*???*/	uint32 item_id;
/*???*/	uint32 charges;
};

struct VeteranRewardEntry
{
/*000*/	uint32 claim_id; // guessed
/*004*/	uint32 avaliable_count;
/*008*/	uint32 claim_count;
/*012*/	char enabled;
/*013*/	//VeteranRewardItem items[0];
};

struct VeteranReward
{
/*000*/	uint32 claim_count;
/*004*/	//VeteranRewardEntry entries[0];
};

struct VeteranClaim
{
/*000*/	char name[68]; //name + other data
/*068*/	uint32 claim_id;
/*072*/	uint32 unknown072;
/*076*/	uint32 action;
};

struct ExpeditionInvite_Struct
{
/*000*/ uint32 client_id;            // unique character id
/*004*/ uint32 unknown004;
/*008*/ char   inviter_name[64];
/*072*/ char   expedition_name[128];
/*200*/ uint8  swapping;             // 0: adding 1: swapping
/*201*/ char   swap_name[64];        // if swapping, swap name being removed
/*265*/ uint8  padding[3];
/*268*/ uint16 dz_zone_id;           // dz_id zone/instance pair, sent back in reply
/*270*/ uint16 dz_instance_id;
};

struct ExpeditionInviteResponse_Struct
{
/*000*/ uint32 unknown000;
/*004*/ uint32 unknown004;
/*008*/ uint16 dz_zone_id;     // dz_id pair sent in invite
/*010*/ uint16 dz_instance_id;
/*012*/ uint8  accepted;       // 0: declined 1: accepted
/*013*/ uint8  swapping;       // 0: adding 1: swapping (sent in invite)
/*014*/ char   swap_name[64];  // swap name sent in invite
/*078*/ uint8  unknown078;     // padding garbage?
/*079*/ uint8  unknown079;     // padding garbage?
};

struct DynamicZoneInfo_Struct
{
/*000*/ uint32 client_id;
/*004*/ uint32 unknown004;
/*008*/ uint32 assigned; // padded bool
/*012*/ uint32 max_players;
/*016*/ char   dz_name[128];
/*144*/ char   leader_name[64];
};

struct DynamicZoneMemberEntry_Struct
{
/*000*/ char name[1];        // variable length, null terminated, max 0x40 (64)
/*000*/ uint8 online_status; // 0: unknown 1: Online, 2: Offline, 3: In Dynamic Zone, 4: Link Dead
};

struct DynamicZoneMemberList_Struct
{
/*000*/ uint32 client_id;
/*004*/ uint32 member_count; // number of players in window
/*008*/ DynamicZoneMemberEntry_Struct members[0]; // variable length
};

struct DynamicZoneMemberListName_Struct
{
/*000*/ uint32 client_id;
/*004*/ uint32 unknown004;
/*008*/ uint32 add_name;   // padded bool, 0: remove name, 1: add name with unknown status
/*012*/ char   name[64];
};

struct ExpeditionLockoutTimerEntry_Struct
{
/*000*/ char   expedition_name[1]; // variable length, null terminated, max 0x80 (128)
/*000*/ uint32 seconds_remaining;
/*000*/ int32  event_type;         // seen -1 (0xffffffff) for replay timers and 1 for event timers
/*000*/ char   event_name[1];      // variable length, null terminated, max 0x100 (256)
};

struct ExpeditionLockoutTimers_Struct
{
/*000*/ uint32 client_id;
/*004*/ uint32 count;
/*008*/ ExpeditionLockoutTimerEntry_Struct timers[0];
};

struct DynamicZoneLeaderName_Struct
{
/*000*/ uint32 client_id;
/*004*/ uint32 unknown004;
/*008*/ char   leader_name[64];
};

struct ExpeditionCommand_Struct
{
/*000*/ uint32 unknown000;
/*004*/ uint32 unknown004;
/*008*/ char   name[64];
};

struct ExpeditionCommandSwap_Struct
{
/*000*/ uint32 unknown000;
/*004*/ uint32 unknown004;
/*008*/ char   add_player_name[64]; // swap to (player must confirm)
/*072*/ char   rem_player_name[64]; // swap from
};

struct ExpeditionExpireWarning
{
/*000*/ uint32 client_id;
/*004*/ uint32 unknown004;
/*008*/ uint32 minutes_remaining;
};

struct DynamicZoneCompassEntry_Struct
{
/*000*/ uint16 dz_zone_id;      // target dz id pair
/*002*/ uint16 dz_instance_id;
/*004*/ uint32 dz_type;         // 1: Expedition, 2: Tutorial (purple), 3: Task, 4: Mission, 5: Quest (green)
/*008*/ uint32 dz_switch_id;
/*012*/ float y;
/*016*/ float x;
/*020*/ float z;
};

struct DynamicZoneCompass_Struct
{
/*000*/ uint32 client_id;
/*004*/ uint32 count;
/*008*/ DynamicZoneCompassEntry_Struct entries[0];
};

struct DynamicZoneChooseZoneEntry_Struct
{
/*000*/ uint16 dz_zone_id;     // dz_id pair
/*002*/ uint16 dz_instance_id;
/*004*/ uint32 unknown_id1;    // sent back in reply
/*008*/ uint32 dz_type;        // 1: Expedition, 2: Tutorial, 3: Task, 4: Mission, 5: Quest -- sent back in reply
/*012*/ uint32 unknown_id2;    // possibly an id based on dz type, for expeditions this was same as dz_id (zone|instance) but task dz was different
/*016*/ char   description[1]; // variable length, null terminated, max 0x80 (128)
/*000*/ char   leader_name[1]; // variable length, null terminated, max 0x40 (64)
};

struct DynamicZoneChooseZone_Struct
{
/*000*/ uint32 client_id;
/*004*/ uint32 count;
/*008*/ DynamicZoneChooseZoneEntry_Struct choices[0];
};

struct DynamicZoneChooseZoneReply_Struct
{
/*000*/ uint32 unknown000;     // ff ff ff ff
/*004*/ uint32 unknown004;     // seen 69 00 00 00
/*008*/ uint32 unknown008;     // ff ff ff ff
/*012*/ uint32 unknown_id1;    // from choose zone entry message
/*016*/ uint16 dz_zone_id;     // dz_id pair
/*018*/ uint16 dz_instance_id;
/*020*/ uint32 dz_type;        // 1: Expedition, 2: Tutorial, 3: Task, 4: Mission, 5: Quest
/*024*/ uint32 unknown_id2;    // from choose zone entry message
/*028*/ uint32 unknown028;     // 00 00 00 00
/*032*/ uint32 unknown032;     // always same as unknown044
/*036*/ uint32 unknown036;
/*040*/ uint32 unknown040;
/*044*/ uint32 unknown044;     // always same as unknown032
/*048*/ uint32 unknown048;     // seen 01 00 00 00 and 02 00 00 00
};

struct MaxCharacters_Struct
{
/*000*/ uint32 max_chars;	// Seen 4 on Silver Account (4 chars max)
/*004*/ uint32 unknown004;	// Seen 0
/*008*/ uint32 unknown008;	// Seen 0
};

// Used by MercenaryListEntry_Struct
struct MercenaryStance_Struct {
/*0000*/ uint32 StanceIndex; // Index of this stance (sometimes reverse reverse order - 3, 2, 1, 0 for 4 stances etc)
/*0004*/ uint32 Stance; // From dbstr_us.txt - 1^24^Passive^0, 2^24^Balanced^0, etc
};
// Used by MercenaryMerchantList_Struct
struct MercenaryListEntry_Struct {
/*0000*/ uint32 MercID; // ID unique to each type of mercenary (probably a DB id)
/*0004*/ uint32 MercType; // From dbstr_us.txt - Apprentice (330000100), Journeyman (330000200), Master (330000300)
/*0008*/ uint32 MercSubType; // From dbstr_us.txt - 330020105^23^Race: Guktan<br>Type: Healer<br>Confidence: High<br>Proficiency: Apprentice, Tier V...
/*0012*/ uint32 PurchaseCost; // Purchase Cost (in gold)
/*0016*/ uint32 UpkeepCost; // Upkeep Cost (in gold)
/*0020*/ uint32 Status; // Required Account Status (Free = 0, Silver = 1, Gold = 2) at merchants - Seen 0 (suspended) or 1 (unsuspended) on hired mercs ?
/*0024*/ uint32 AltCurrencyCost; // Alternate Currency Purchase Cost? (all seen costs show N/A Bayle Mark) - Seen 0
/*0028*/ uint32 AltCurrencyUpkeep; // Alternate Currency Upkeep Cost? (all seen costs show 1 Bayle Mark) - Seen 1
/*0032*/ uint32 AltCurrencyType; // Alternate Currency Type? - 19^17^Bayle Mark^0 - Seen 19
/*0036*/ uint8 MercUnk01; // Unknown (always see 0)
/*0037*/ int32 TimeLeft; // Unknown (always see -1 at merchant) - Seen 900000 (15 minutes in ms for newly hired merc)
/*0041*/ uint32 MerchantSlot; // Merchant Slot? Increments, but not always by 1 - May be for Merc Window Options (Seen 5, 36, 1 for active mercs)?
/*0045*/ uint32 MercUnk02; // Unknown (normally see 1, but sometimes 2 or 0)
/*0049*/ uint32 StanceCount; // Iterations of MercenaryStance_Struct - Normally 2 to 4 seen
/*0053*/ int32 MercUnk03; // Unknown (always 0 at merchant) - Seen on active merc: 93 a4 03 77, b8 ed 2f 26, 88 d5 8b c3, and 93 a4 ad 77
/*0057*/ uint8 MercUnk04; // Seen 1
/*0058*/ char MercName[1]; // Null Terminated Mercenary Name (00 at merchants)
/*0000*/ MercenaryStance_Struct Stances[1]; // Count Varies - From dbstr_us.txt - 1^24^Passive^0, 2^24^Balanced^0, etc
};

// Sent by the server when browsing the Mercenary Merchant
struct MercenaryMerchantList_Struct {
/*0000*/ uint32 MercTypeCount; // Number of Merc Types to follow
/*0004*/ uint32 MercTypes[1]; // Count varies, but hard set to 3 max for now - From dbstr_us.txt - Apprentice (330000100), Journeyman (330000200), Master (330000300)
/*0016*/ uint32 MercCount; // Number of MercenaryInfo_Struct to follow
/*0020*/ MercenaryListEntry_Struct Mercs[0]; // Data for individual mercenaries in the Merchant List
};

// OP_MercenaryDataRequest
// Right clicking merchant - shop request
struct MercenaryMerchantShopRequest_Struct {
/*0000*/ uint32 MercMerchantID; // Entity ID of the Mercenary Merchant
/*0004*/
};

// Used by MercenaryDataUpdate_Struct
struct MercenaryData_Struct {
/*0000*/ uint32 MercID; // ID unique to each type of mercenary (probably a DB id) - (if 1, do not send MercenaryData_Struct - No merc hired)
/*0004*/ uint32 MercType; // From dbstr_us.txt - Apprentice (330000100), Journeyman (330000200), Master (330000300)
/*0008*/ uint32 MercSubType; // From dbstr_us.txt - 330020105^23^Race: Guktan<br>Type: Healer<br>Confidence: High<br>Proficiency: Apprentice, Tier V...
/*0012*/ uint32 PurchaseCost; // Purchase Cost (in gold)
/*0016*/ uint32 UpkeepCost; // Upkeep Cost (in gold)
/*0020*/ uint32 Status; // Required Account Status (Free = 0, Silver = 1, Gold = 2) at merchants - Seen 0 (suspended) or 1 (unsuspended) on hired mercs ?
/*0024*/ uint32 AltCurrencyCost; // Alternate Currency Purchase Cost? (all seen costs show N/A Bayle Mark) - Seen 0
/*0028*/ uint32 AltCurrencyUpkeep; // Alternate Currency Upkeep Cost? (all seen costs show 1 Bayle Mark) - Seen 1
/*0032*/ uint32 AltCurrencyType; // Alternate Currency Type? - 19^17^Bayle Mark^0 - Seen 19
/*0036*/ uint8 MercUnk01; // Unknown (always see 0)
/*0037*/ int32 TimeLeft; // Unknown (always see -1 at merchant) - Seen 900000 (15 minutes in ms for newly hired merc)
/*0041*/ uint32 MerchantSlot; // Merchant Slot? Increments, but not always by 1 - May be for Merc Window Options (Seen 5, 36, 1 for active mercs)?
/*0045*/ uint32 MercUnk02; // Unknown (normally see 1, but sometimes 2 or 0)
/*0049*/ uint32 StanceCount; // Iterations of MercenaryStance_Struct - Normally 2 to 4 seen
/*0053*/ int32 MercUnk03; // Unknown (always 0 at merchant) - Seen on active merc: 93 a4 03 77, b8 ed 2f 26, 88 d5 8b c3, and 93 a4 ad 77
/*0057*/ uint8 MercUnk04; // Seen 1
/*0058*/ char MercName[1]; // Null Terminated Mercenary Name (00 at merchants)
/*0000*/ MercenaryStance_Struct Stances[1]; // Count Varies, but hard set to 2 for now - From dbstr_us.txt - 1^24^Passive^0, 2^24^Balanced^0, etc (1 to 9 as of April 2012)
/*0000*/ uint32 MercUnk05; // Seen 1 - Extra Merc Data field that differs from MercenaryListEntry_Struct
// MercUnk05 may be a field that is at the end of the packet only, even if multiple mercs are listed (haven't seen examples of multiple mercs owned at once)
};

// Should be named OP_MercenaryDataResponse, but the current opcode using that name should be renamed first
// Size varies if mercenary is hired or if browsing Mercenary Merchant
// This may also be the response for Client->Server 0x0327 (size 0) packet On Live as of April 2 2012
struct MercenaryDataUpdate_Struct {
/*0000*/ int32 MercStatus; // Seen 0 with merc and -1 with no merc hired
/*0004*/ uint32 MercCount; // Seen 1 with 1 merc hired and 0 with no merc hired
/*0008*/ MercenaryData_Struct MercData[0]; // Data for individual mercenaries in the Merchant List
};

// Size 12 and sent on Zone-In if no mercenary is currently hired and when merc is dismissed
// (Same packet as MercAssign_Struct?)
struct NoMercenaryHired_Struct {
/*0000*/ int32 MercStatus; // Seen -1 with no merc hired
/*0004*/ uint32 MercCount; // Seen 0 with no merc hired
/*0008*/ uint32 MercID; // Seen 1 when no merc is hired - ID unique to each type of mercenary
/*0012*/
};

// OP_MercenaryAssign (Same packet as NoMercenaryHired_Struct?)
// Not actually Merc related - This is actually a weapon equp packet
struct MercenaryAssign_Struct {
/*0000*/ uint32 MercEntityID; // Seen 0 (no merc spawned) or 615843841 and 22779137
/*0004*/ uint32 MercUnk01; //
/*0008*/ uint32 MercUnk02; //
/*0012*/
};

// OP_MercenaryTimer
// Sent on Zone-In, or after Dismissing, Suspending, or Unsuspending Mercs
struct MercenaryStatus_Struct {
/*0000*/ uint32 MercEntityID; // Seen 0 (no merc spawned) or 615843841 and 22779137
/*0004*/ uint32 UpdateInterval; // Seen 900000 - Matches from 0x6537 packet (15 minutes in ms?)
/*0008*/ uint32 MercUnk01; // Seen 180000 - 3 minutes in milleseconds? Maybe next update interval?
/*0012*/ uint32 MercState; // Seen 5 (normal) or 1 (suspended)
/*0016*/ uint32 SuspendedTime; // Seen 0 (not suspended) or c9 c2 64 4f (suspended on Sat Mar 17 11:58:49 2012) - Unix Timestamp
/*0020*/
};

// Sent from the client when using the Mercenary Window
struct MercenaryCommand_Struct {
/*0000*/ uint32 MercCommand; // Seen 0 (zone in with no merc or suspended), 1 (dismiss merc), 5 (normal state), 36 (zone in with merc)
/*0004*/ int32 Option; // Seen -1 (zone in with no merc), 0 (setting to passive stance), 1 (normal or setting to balanced stance)
/*0008*/
};

// Requesting to suspend or unsuspend merc
struct SuspendMercenary_Struct {
/*0000*/ uint8 SuspendMerc; // Seen 30 (48) for suspending or unsuspending
/*0001*/
};

// Response to suspend merc with timestamp
struct SuspendMercenaryResponse_Struct {
/*0000*/ uint32 SuspendTime; // Unix Timestamp - Seen a9 11 78 4f
/*0004*/
};

// Sent by client when requesting to view Mercenary info or Hire a Mercenary
struct MercenaryMerchantRequest_Struct {
/*0000*/ uint32 MercID; // Seen 399 and 400 for merc ID
/*0004*/ uint32 MercUnk01; // Seen 1
/*0008*/ uint32 MercMerchantID; // Entity ID for Mercenary Merchant
/*0012*/ uint32 MercUnk02; // Seen 65302016 (00 6e e4 03) - (probably actually individual uint8 fields), but seen as DWORD in Seeds client.
/*0016*/
};

// Sent by Server in response to requesting to view Mercenary info or Hire a Mercenary
struct MercenaryMerchantResponse_Struct {
/*0000*/ uint32 ResponseType;
/*0004*/
};

struct SayLinkBodyFrame_Struct {
/*000*/	char ActionID[1];
/*001*/	char ItemID[5];
/*006*/	char Augment1[5];
/*011*/	char Augment2[5];
/*016*/	char Augment3[5];
/*021*/	char Augment4[5];
/*026*/	char Augment5[5];
/*031*/	char Augment6[5];
/*036*/	char IsEvolving[1];
/*037*/	char EvolveGroup[4];
/*041*/	char EvolveLevel[1];
/*042*/	char OrnamentIcon[5];
/*047*/	char Hash[8];
/*055*/
};

#pragma pack(pop)

} /*structs*/
} /*RoF*/
