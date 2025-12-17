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

#include "common/eq_packet_structs.h"
#include "common/patches/titanium_limits.h"
#include "common/textures.h"
#include "common/types.h"


namespace Titanium { namespace structs {


static const uint32 BUFF_COUNT = 25;

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

// yep, even tit had a version of the new inventory system, used by OP_MoveMultipleItems
struct InventorySlot_Struct
{
/*000*/	int32	Type;		// Worn and Normal inventory = 0, Bank = 1, Shared Bank = 2, Trade = 3, World = 4, Limbo = 5
/*004*/	int32	Slot;
/*008*/	int32	SubIndex;	// no aug index in Tit
/*012*/	int32	Unknown01;
};

// unsure if they have a version of this, completeness though
struct TypelessInventorySlot_Struct
{
/*000*/	int32	Slot;
/*004*/	int32	SubIndex;	// no aug index in Tit
/*008*/	int32	Unknown01;
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
/*
	Cofruben:
	Adventure stuff,not a net one,just one for our use
*/
static const uint32 ADVENTURE_COLLECT		= 0;
static const uint32 ADVENTURE_MASSKILL		= 1;
static const uint32 ADVENTURE_NAMED			= 2;
static const uint32 ADVENTURE_RESCUE		= 3;

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

struct TintProfile {
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

struct Texture_Struct
{
	uint32 Material;
};

struct TextureProfile
{
	union {
		struct {
			Texture_Struct Head;
			Texture_Struct Chest;
			Texture_Struct Arms;
			Texture_Struct Wrist;
			Texture_Struct Hands;
			Texture_Struct Legs;
			Texture_Struct Feet;
			Texture_Struct Primary;
			Texture_Struct Secondary;
		};
		Texture_Struct Slot[EQ::textures::materialCount];
	};

	TextureProfile();
};

/*
** Character Selection Struct
** Length: 1704 Bytes
**
*/
struct CharacterSelect_Struct
{
/*0000*/	uint32 Race[10];				// Characters Race
/*0040*/	TintProfile CS_Colors[10];		// Characters Equipment Colors - packet requires length for 10 characters..but, client is limited to 8
/*0400*/	uint8 BeardColor[10];			// Characters beard Color
/*0410*/	uint8 HairStyle[10];			// Characters hair style
/*0420*/	TextureProfile Equip[10];		// Characters texture array
/*0780*/	uint32 SecondaryIDFile[10];		// Characters secondary IDFile number
/*0820*/	uint8 Unknown820[10];			// 10x ff
/*0830*/	uint8 Unknown830[2];			// 2x 00
/*0832*/	uint32 Deity[10];				// Characters Deity
/*0872*/	uint8 GoHome[10];				// 1=Go Home available, 0=not
/*0882*/	uint8 Tutorial[10];				// 1=Tutorial available, 0=not
/*0892*/	uint8 Beard[10];				// Characters Beard Type
/*0902*/	uint8 Unknown902[10];			// 10x ff
/*0912*/	uint32 PrimaryIDFile[10];		// Characters primary IDFile number
/*0952*/	uint8 HairColor[10];			// Characters Hair Color
/*0962*/	uint8 Unknown0962[2];			// 2x 00
/*0964*/	uint32 Zone[10];				// Characters Current Zone
/*1004*/	uint8 Class[10];				// Characters Classes
/*1014*/	uint8 Face[10];					// Characters Face Type
/*1024*/	char Name[10][64];				// Characters Names
/*1664*/	uint8 Gender[10];				// Characters Gender
/*1674*/	uint8 EyeColor1[10];			// Characters Eye Color
/*1684*/	uint8 EyeColor2[10];			// Characters Eye 2 Color
/*1694*/	uint8 Level[10];				// Characters Levels
/*1704*/
};

/*
** Generic Spawn Struct
** Length: 257 Bytes
** Fields from old struct not yet found:
**	float	size;
**	float	walkspeed;	// probably one of the ff 33 33 33 3f
**	float	runspeed;	// probably one of the ff 33 33 33 3f
**	uint8	traptype;	// 65 is disarmable trap, 66 and 67 are invis triggers/traps
**	uint8	npc_armor_graphic;	// 0xFF=Player, 0=none, 1=leather, 2=chain, 3=steelplate
**	uint8	npc_helm_graphic;	// 0xFF=Player, 0=none, 1=leather, 2=chain, 3=steelplate
**
*/

/*
** Generic Spawn Struct
** Length: 383 Octets
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
struct Spawn_Struct {
/*0000*/ uint8 unknown0000;
/*0001*/ uint8  gm;             // 0=no, 1=gm
/*0002*/ uint8 unknown0003;
/*0003*/ uint8   aaitle;       // 0=none, 1=general, 2=archtype, 3=class
/*0004*/ uint8 unknown0004;
/*0005*/ uint8  anon;           // 0=normal, 1=anon, 2=roleplay
/*0006*/ uint8  face;	          // Face id for players
/*0007*/ char     name[64];       // Player's Name
/*0071*/ uint16  deity;          // Player's Deity
/*0073*/ uint16 unknown0073;
/*0075*/ float    size;           // Model size
/*0079*/ uint32 unknown0079;
/*0083*/ uint8  NPC;            // 0=player,1=npc,2=pc corpse,3=npc corpse,a
/*0084*/ uint8  invis;          // Invis (0=not, 1=invis)
/*0085*/ uint8  haircolor;      // Hair color
/*0086*/ uint8  curHp;          // Current hp %%% wrong
/*0087*/ uint8  max_hp;         // (name prolly wrong)takes on the value 100 for players, 100 or 110 for NPCs and 120 for PC corpses...
/*0088*/ uint8  findable;       // 0=can't be found, 1=can be found
/*0089*/ uint8 unknown0089[5];
/*0094*/ signed   deltaHeading:10;// change in heading
         signed   x:19;           // x coord
         signed   padding0054:3;  // ***Placeholder
/*0098*/ signed   y:19;           // y coord
         signed   animation:10;   // animation
         signed   padding0058:3;  // ***Placeholder
/*0102*/ signed   z:19;           // z coord
         signed   deltaY:13;      // change in y
/*0106*/ signed   deltaX:13;      // change in x
         unsigned heading:12;     // heading
         signed   padding0066:7;  // ***Placeholder
/*0110*/ signed   deltaZ:13;      // change in z
         signed   padding0070:19; // ***Placeholder
/*0114*/ uint8  eyecolor1;      // Player's left eye color
/*0115*/ uint8 unknown0115[24];
/*0139*/ uint8  showhelm;       // 0=no, 1=yes
/*0140*/ uint8 unknown0140[4];
/*0144*/ uint8  is_npc;         // 0=no, 1=yes
/*0145*/ uint8  hairstyle;      // Hair style
/*0146*/ uint8  beardcolor;     // Beard color
/*0147*/ uint8 unknown0147[4];
/*0151*/ uint8  level;          // Spawn Level
/*0152*/ uint32 PlayerState;    // PlayerState controls some animation stuff
/*0156*/ uint8  beard;          // Beard style
/*0157*/ char     suffix[32];     // Player's suffix (of Veeshan, etc.)
/*0189*/ uint32 petOwnerId;     // If this is a pet, the spawn id of owner
/*0193*/ uint8   guildrank;      // 0=normal, 1=officer, 2=leader
/*0194*/ uint8 unknown0194[3];
/*0197*/ TextureProfile equipment;
/*0233*/ float    runspeed;       // Speed when running
/*0036*/ uint8  afk;            // 0=no, 1=afk
/*0238*/ uint32 guildID;        // Current guild
/*0242*/ char     title[32];      // Title
/*0274*/ uint8 unknown0274;
/*0275*/ uint8  helm;           // Helm texture
/*0276*/ uint8  set_to_0xFF[8]; // ***Placeholder (all ff)
/*0284*/ uint32 race;           // Spawn race
/*0288*/ uint32 unknown0288;
/*0292*/ char     lastName[32];   // Player's Lastname
/*0324*/ float    walkspeed;      // Speed when walking
/*0328*/ uint8 unknown0328;
/*0329*/ uint8  is_pet;         // 0=no, 1=yes
/*0330*/ uint8  light;          // Spawn's lightsource %%% wrong
/*0331*/ uint8  class_;         // Player's class
/*0332*/ uint8  eyecolor2;      // Left eye color
/*0333*/ uint8 flymode;
/*0334*/ uint8  gender;         // Gender (0=male, 1=female)
/*0335*/ uint8  bodytype;       // Bodytype
/*0336*/ uint8 unknown0336[3];
union
{
/*0339*/ uint8 equip_chest2;     // Second place in packet for chest texture (usually 0xFF in live packets)
                                  // Not sure why there are 2 of them, but it effects chest texture!
/*0339*/ uint8 mount_color;      // drogmor: 0=white, 1=black, 2=green, 3=red
                                  // horse: 0=brown, 1=white, 2=black, 3=tan
};
/*0340*/ uint32 spawnId;        // Spawn Id
/*0344*/ float bounding_radius; // used in melee, overrides calc
/*0348*/ TintProfile equipment_tint;
/*0384*/ uint8  lfg;            // 0=off, 1=lfg on
/*0385*/

};

/*
** New Spawn
** Length: 176 Bytes
** OpCode: 4921
*/
struct NewSpawn_Struct
{
	struct Spawn_Struct spawn;	// Spawn Information
};

struct ClientZoneEntry_Struct {
/*0000*/	uint32	unknown00;
/*0004*/	char	char_name[64];			// Character Name
};

/*
** Server Zone Entry Struct
** Length: 452 Bytes
** OPCodes: OP_ServerZoneEntry
**
*/
struct ServerZoneEntry_Struct
{
	struct NewSpawn_Struct player;
};

struct NewZone_Struct {
/*0000*/	char	char_name[64];			// Character Name
/*0064*/	char	zone_short_name[32];	// Zone Short Name
/*0096*/	char	zone_long_name[278];	// Zone Long Name
/*0374*/	uint8	ztype;					// Zone type (usually FF)
/*0375*/	uint8	fog_red[4];				// Zone fog (red)
/*0379*/	uint8	fog_green[4];			// Zone fog (green)
/*0383*/	uint8	fog_blue[4];			// Zone fog (blue)
/*0387*/	uint8	unknown323;
/*0388*/	float	fog_minclip[4];
/*0404*/	float	fog_maxclip[4];
/*0420*/	float	gravity;
/*0424*/	uint8	time_type;
/*0425*/    uint8   rain_chance[4];
/*0429*/    uint8   rain_duration[4];
/*0433*/    uint8   snow_chance[4];
/*0437*/    uint8   snow_duration[4];
/*0441*/	uint8	unknown360[33];
/*0474*/	uint8	sky;					// Sky Type
/*0475*/	uint8	unknown331[13];			// ***Placeholder
/*0488*/	float	zone_exp_multiplier;	// Experience Multiplier
/*0492*/	float	safe_y;					// Zone Safe Y
/*0496*/	float	safe_x;					// Zone Safe X
/*0500*/	float	safe_z;					// Zone Safe Z
/*0504*/	float	max_z;					// Guessed
/*0508*/	float	underworld;				// Underworld, min z (Not Sure?)
/*0512*/	float	minclip;				// Minimum View Distance
/*0516*/	float	maxclip;				// Maximum View DIstance
/*0520*/	uint8	unknown_end[84];		// ***Placeholder
/*0604*/	char	zone_short_name2[68];
/*0672*/	char	unknown672[12];
/*0684*/	uint16	zone_id;
/*0686*/	uint16	zone_instance;
/*0688*/	uint32	unknown688;
/*0692*/	uint8	unknown692[8];
/*0700*/
};

/*
** Memorize Spell Struct
** Length: 12 Bytes
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
	char	message[0];
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
};

struct SwapSpell_Struct
{
	uint32 from_slot;
	uint32 to_slot;


};

struct BeginCast_Struct
{
	// len = 8
/*000*/	uint16	caster_id;
/*002*/	uint16	spell_id;
/*004*/	uint32	cast_time;		// in miliseconds
};

struct CastSpell_Struct
{
	uint32	slot;
	uint32	spell_id;
	uint32	inventoryslot;  // slot for clicky item, 0xFFFF = normal cast
	uint32	target_id;
	uint8	cs_unknown[4];
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


// this is used inside profile
struct SpellBuff_Struct
{
/*000*/	uint8	effect_type;	// 0 = no buff, 2 = buff, 4 = inverse affects of buff
/*001*/ uint8	level;
/*002*/	uint8	bard_modifier;
/*003*/	uint8	unknown003;		// MQ2 used to call this "damage shield" -- don't see client referencing it, so maybe server side DS type tracking?
/*004*/	uint32	spellid;
/*008*/ int32	duration;
/*012*/	uint32	counters;		// single book keeping value (counters, rune/vie)
/*016*/	uint32	player_id;		// caster ID, pretty sure just zone ID
};

struct SpellBuffPacket_Struct {
/*000*/	uint32 entityid;
/*004*/	SpellBuff_Struct buff;
/*024*/	uint32 slotid;
/*028*/	uint32 bufffade;
/*032*/
};

struct ItemNamePacket_Struct {
/*000*/	uint32 item_id;
/*004*/	uint32 unkown004;
/*008*/ char name[64];
/*072*/
};

// Length: 10
struct ItemProperties_Struct {

uint8	unknown01[2];
uint8	charges;
uint8	unknown02[13];
};

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
};
struct ConsentResponse_Struct {
	char grantname[64];
	char ownername[64];
	uint8 permission;
	char zonename[32];
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
** Length: 140 Bytes
** OpCode: 0x0113
*/
struct CharCreate_Struct
{
/*0000*/	uint32	class_;
/*0004*/	uint32	haircolor;	// Might be hairstyle
/*0008*/	uint32	beardcolor;	// Might be beard
/*0012*/	uint32	beard;		// Might be beardcolor
/*0016*/	uint32	gender;
/*0020*/	uint32	race;
/*0024*/	uint32	start_zone;
	// 0 = odus
	// 1 = qeynos
	// 2 = halas
	// 3 = rivervale
	// 4 = freeport
	// 5 = neriak
	// 6 = gukta/grobb
	// 7 = ogguk
	// 8 = kaladim
	// 9 = gfay
	// 10 = felwithe
	// 11 = akanon
	// 12 = cabalis
	// 13 = shar vahl
/*0028*/	uint32	hairstyle;	// Might be haircolor
/*0032*/	uint32	deity;
/*0036*/	uint32	STR;
/*0040*/	uint32	STA;
/*0044*/	uint32	AGI;
/*0048*/	uint32	DEX;
/*0052*/	uint32	WIS;
/*0056*/	uint32	INT;
/*0060*/	uint32	CHA;
/*0064*/	uint32	face;		// Could be unknown0076
/*0068*/	uint32	eyecolor1;	//its possiable we could have these switched
/*0073*/	uint32	eyecolor2;	//since setting one sets the other we really can't check
/*0076*/	uint32	tutorial;
/*0080*/
};

/*
 *Used in PlayerProfile
 */
struct AA_Array
{
	uint32 AA;
	uint32 value;
};


static const uint32 MAX_PP_DISCIPLINES = 100;

struct Disciplines_Struct {
	uint32 values[MAX_PP_DISCIPLINES];
};

static const uint32 MAX_PLAYER_TRIBUTES = 5;
static const uint32 TRIBUTE_NONE = 0xFFFFFFFF;
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

//len = 72
struct BandolierItem_Struct
{
	uint32 ID;
	uint32 Icon;
	char Name[64];
};

//len = 320
struct Bandolier_Struct
{
	char Name[32];
	BandolierItem_Struct Items[profile::BANDOLIER_ITEM_COUNT];
};

//len = 72
struct PotionBeltItem_Struct
{
	uint32 ID;
	uint32 Icon;
	char Name[64];
};

//len = 288
struct PotionBelt_Struct
{
	PotionBeltItem_Struct Items[profile::POTION_BELT_SIZE];
};

static const uint32 MAX_GROUP_LEADERSHIP_AA_ARRAY = 16;
static const uint32 MAX_RAID_LEADERSHIP_AA_ARRAY = 16;
static const uint32 MAX_LEADERSHIP_AA_ARRAY = (MAX_GROUP_LEADERSHIP_AA_ARRAY+MAX_RAID_LEADERSHIP_AA_ARRAY);
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


/*
** Player Profile
**
** Length: 4308 bytes
** OpCode: 0x006a
 */
static const uint32 MAX_PP_LANGUAGE		= 28;

static const uint32 MAX_PP_SKILL		= PACKET_SKILL_ARRAY_SIZE;	// 100 - actual skills buffer size
static const uint32 MAX_PP_INNATE_SKILL	= 25;
static const uint32 MAX_PP_AA_ARRAY		= 240;
static const uint32 MAX_GROUP_MEMBERS	= 6;
static const uint32 MAX_RECAST_TYPES	= 20;
/*
showeq -> eqemu
sed -e 's/_t//g' -e 's/MAX_AA/MAX_PP_AA_ARRAY/g' \
    -e 's/MAX_SPELL_SLOTS/MAX_PP_MEMSPELL/g' \
    -e 's/MAX_KNOWN_SKILLS/MAX_PP_SKILL/g' \
    -e 's/MAXRIBUTES/MAX_PLAYER_TRIBUTES/g' \
    -e 's/MAX_BUFFS/BUFF_COUNT/g' \
    -e 's/MAX_KNOWN_LANGS/MAX_PP_LANGUAGE/g' \
    -e 's/MAX_RECASTYPES/MAX_RECAST_TYPES/g' \
    -e 's/spellBuff/SpellBuff_Struct/g' \
    -e 's/lastName/last_name/g' \
    -e 's/guildID/guild_id/g' \
    -e 's/itemint/item_tint/g' \
    -e 's/MANA/mana/g' \
    -e 's/curHp/cur_hp/g' \
    -e 's/sSpellBook/spell_book/g' \
    -e 's/sMemSpells/mem_spells/g' \
    -e 's/uint32[ \t]*disciplines\[MAX_DISCIPLINES\]/Disciplines_Struct  disciplines/g' \
    -e 's/aa_unspent/aapoints/g' \
    -e 's/aa_spent/aapoints_spent/g' \
    -e 's/InlineItem[ \t]*potionBelt\[MAX_POTIONS_IN_BELT\]/PotionBelt_Struct  potionbelt/g' \
    -e 's/ldon_guk_points/ldon_points_guk/g' \
    -e 's/ldon_mir_points/ldon_points_mir/g' \
    -e 's/ldon_mmc_points/ldon_points_mmc/g' \
    -e 's/ldon_ruj_points/ldon_points_ruj/g' \
    -e 's/ldonak_points/ldon_points_tak/g' \
    -e 's/ldon_avail_points/ldon_points_available/g' \
    -e 's/tributeTime/tribute_time_remaining/g' \
    -e 's/careerTribute/career_tribute_points/g' \
    -e 's/currentTribute/tribute_points/g' \
    -e 's/tributeActive/tribute_active/g' \
    -e 's/TributeStruct/Tribute_Struct/g' \
    -e 's/expGroupLeadAA/group_leadership_exp/g' \
    -e 's/expRaidLeadAA/raid_leadership_exp/g' \
    -e 's/groupLeadAAUnspent/group_leadership_points/g' \
    -e 's/raidLeadAAUnspent/raid_leadership_points/g' \
    -e 's/uint32[ \t]*leadershipAAs\[MAX_LEAD_AA\]/LeadershipAA_Struct leader_abilities/g' \
    -e 's/BandolierStruct/Bandolier_Struct/g' \
    -e 's/MAX_BANDOLIERS/MAX_PLAYER_BANDOLIER/g' \
    -e 's/birthdayTime/birthday/g' \
    -e 's/lastSaveTime/lastlogin/g' \
    -e 's/zoneId/zone_id/g' \
    -e 's/hunger/hunger_level/g' \
    -e 's/thirst/thirst_level/g' \
    -e 's/guildstatus/guildrank/g' \
    -e 's/airRemaining/air_remaining/g' \



 */
struct PlayerProfile_Struct
{
/*00000*/ uint32  checksum;           //
/*00004*/ uint32  gender;             // Player Gender - 0 Male, 1 Female
/*00008*/ uint32  race;               // Player race
/*00012*/ uint32  class_;             // Player class
/*00016*/ uint32  unknown00016;       // ***Placeholder
/*00020*/ uint8   level;              // Level of player
/*00021*/ uint8   level1;             // Level of player (again?)
/*00022*/ uint8   unknown00022[2];    // ***Placeholder
/*00024*/ BindStruct binds[5];          // Bind points (primary is first)
/*00124*/ uint32  deity;              // deity
/*00128*/ uint32  intoxication;       // Alcohol level (in ticks till sober?)
/*00132*/ uint32  spellSlotRefresh[spells::SPELL_GEM_COUNT]; // Refresh time (millis)
/*00168*/ uint32  abilitySlotRefresh;
/*00172*/ uint8   haircolor;          // Player hair color
/*00173*/ uint8   beardcolor;         // Player beard color
/*00174*/ uint8   eyecolor1;          // Player left eye color
/*00175*/ uint8   eyecolor2;          // Player right eye color
/*00176*/ uint8   hairstyle;          // Player hair style
/*00177*/ uint8   beard;              // Player beard type
/*00178*/ uint8 unknown00178[10];
/*00188*/ TextureProfile  item_material;   // Item texture/material of worn items
/*00224*/ uint8 unknown00224[44];
/*00268*/ TintProfile item_tint;    // RR GG BB 00
/*00304*/ AA_Array  aa_array[MAX_PP_AA_ARRAY];   // AAs
/*02224*/ uint32  points;             // Unspent Practice points
/*02228*/ uint32  mana;               // Current mana
/*02232*/ uint32  cur_hp;              // Current HP without +HP equipment
/*02236*/ uint32  STR;                // Strength
/*02240*/ uint32  STA;                // Stamina
/*02244*/ uint32  CHA;                // Charisma
/*02248*/ uint32  DEX;                // Dexterity
/*02252*/ uint32  INT;                // Intelligence
/*02256*/ uint32  AGI;                // Agility
/*02260*/ uint32  WIS;                // Wisdom
/*02264*/ uint8   face;               // Player face
/*02265*/ uint8 unknown02264[47];
/*02312*/ uint32   spell_book[spells::SPELLBOOK_SIZE];    // List of the Spells in spellbook
/*03912*/ uint8   unknown4184[448];   // all 0xff after last spell
/*04360*/ uint32   mem_spells[spells::SPELL_GEM_COUNT]; // List of spells memorized
/*04396*/ uint8 unknown04396[32];
/*04428*/ uint32  platinum;           // Platinum Pieces on player
/*04432*/ uint32  gold;               // Gold Pieces on player
/*04436*/ uint32  silver;             // Silver Pieces on player
/*04440*/ uint32  copper;             // Copper Pieces on player
/*04444*/ uint32  platinum_cursor;    // Platinum Pieces on cursor
/*04448*/ uint32  gold_cursor;        // Gold Pieces on cursor
/*04452*/ uint32  silver_cursor;      // Silver Pieces on cursor
/*04456*/ uint32  copper_cursor;      // Copper Pieces on cursor
/*04460*/ uint32  skills[MAX_PP_SKILL]; // [400] List of skills	// 100 dword buffer
/*04860*/ uint32  InnateSkills[MAX_PP_INNATE_SKILL];
/*04960*/ uint8   unknown04760[36];
/*04996*/ uint32  toxicity;           // Potion Toxicity (15=too toxic, each potion adds 3)
/*05000*/ uint32  thirst_level;             // Drink (ticks till next drink)
/*05004*/ uint32  hunger_level;             // Food (ticks till next eat)
/*05008*/ SpellBuff_Struct buffs[BUFF_COUNT];   // Buffs currently on the player
/*05508*/ Disciplines_Struct  disciplines; // Known disciplines
/*05908*/ uint32 recastTimers[MAX_RECAST_TYPES]; // Timers (GMT of last use)
/*05988*/ uint8 unknown05008[160];
/*06148*/ uint32  endurance;          // Current endurance
/*06152*/ uint32  aapoints_spent;           // Number of spent AA points
/*06156*/ uint32  aapoints;         // Unspent AA points
/*06160*/ uint8 unknown06160[4];
/*06164*/ Bandolier_Struct bandoliers[profile::BANDOLIERS_SIZE]; // bandolier contents
/*07444*/ uint8 unknown07444[5120];
/*12564*/ PotionBelt_Struct  potionbelt; // potion belt
/*12852*/ uint8 unknown12852[8];
/*12860*/ uint32 available_slots;
/*12864*/ uint8 unknown12864[76];
/*12940*/ char      name[64];           // Name of player
/*13004*/ char      last_name[32];       // Last name of player
/*13036*/ uint32   guild_id;            // guildid
/*13040*/ uint32  birthday;       // character birthday
/*13044*/ uint32  lastlogin;       // character last save time
/*13048*/ uint32  timePlayedMin;      // time character played
/*13052*/ uint8   pvp;                // 1=pvp, 0=not pvp
/*13053*/ uint8   anon;               // 2=roleplay, 1=anon, 0=not anon
/*13054*/ uint8   gm;                 // 0=no, 1=yes (guessing!)
/*13055*/ uint8    guildrank;        // 0=member, 1=officer, 2=guildleader
/*13056*/ uint32  guildbanker;
/*13060*/ uint8 unknown13054[8];
/*13068*/ uint32  exp;                // Current Experience
/*13072*/ uint8 unknown13072[8];
/*13080*/ uint32 timeentitledonaccount; // In days, displayed in /played
/*13084*/ uint8   languages[MAX_PP_LANGUAGE]; // List of languages
/*13109*/ uint8   unknown13109[4];    // All 0x00 (language buffer?)
/*13116*/ float     x;                  // Players x position
/*13120*/ float     y;                  // Players y position
/*13124*/ float     z;                  // Players z position
/*13128*/ float     heading;            // Players heading
/*13132*/ uint8   unknown13132[4];    // ***Placeholder
/*13136*/ uint32  platinum_bank;      // Platinum Pieces in Bank
/*13140*/ uint32  gold_bank;          // Gold Pieces in Bank
/*13144*/ uint32  silver_bank;        // Silver Pieces in Bank
/*13148*/ uint32  copper_bank;        // Copper Pieces in Bank
/*13152*/ uint32  platinum_shared;    // Shared platinum pieces
/*13156*/ uint8 unknown13156[84];
/*13240*/ uint32  expansions;         // Bitmask for expansions
/*13244*/ uint8 unknown13244[12];
/*13256*/ uint32  autosplit;          // 0 = off, 1 = on
/*13260*/ uint8 unknown13260[16];
/*13276*/ uint16  zone_id;             // see zones.h
/*13278*/ uint16  zoneInstance;       // Instance id
/*13280*/ char      groupMembers[MAX_GROUP_MEMBERS][64];// all the members in group, including self
/*13664*/ char      groupLeader[64];    // Leader of the group ?
/*13728*/ uint8 unknown13728[656];
/*14384*/ uint32  entityid;
/*14388*/ uint32  leadAAActive;       // 0 = leader AA off, 1 = leader AA on
/*14392*/ uint8 unknown14392[4];
/*14396*/ int32  ldon_points_guk;    // Earned GUK points
/*14400*/ int32  ldon_points_mir;    // Earned MIR points
/*14404*/ int32  ldon_points_mmc;    // Earned MMC points
/*14408*/ int32  ldon_points_ruj;    // Earned RUJ points
/*14412*/ int32  ldon_points_tak;    // Earned TAK points
/*14416*/ int32  ldon_points_available;  // Available LDON points
/*14420*/ uint8 unknown14420[132];
/*14552*/ uint32  tribute_time_remaining;        // Time remaining on tribute (millisecs)
/*14556*/ uint32  career_tribute_points;      // Total favor points for this char
/*14560*/ uint32  unknown7208;        // *** Placeholder
/*14564*/ uint32  tribute_points;     // Current tribute points
/*14568*/ uint32  unknown7216;        // *** Placeholder
/*14572*/ uint32  tribute_active;      // 0 = off, 1=on
/*14576*/ Tribute_Struct tributes[MAX_PLAYER_TRIBUTES]; // Current tribute loadout
/*14616*/ uint32  unknown14616;
/*14620*/ double  group_leadership_exp;
/*14628*/ double  raid_leadership_exp;
/*14640*/ uint32  group_leadership_points; // Unspent group lead AA points
/*14644*/ uint32  raid_leadership_points;  // Unspent raid lead AA points
/*14644*/ LeadershipAA_Struct leader_abilities; // Leader AA ranks
/*14772*/ uint8 unknown14772[128];
/*14900*/ uint32  air_remaining;       // Air supply (seconds)
/*14904*/ uint32  PVPKills;
/*14908*/ uint32  PVPDeaths;
/*14912*/ uint32  PVPCurrentPoints;
/*14916*/ uint32  PVPCareerPoints;
/*14920*/ uint32  PVPBestKillStreak;
/*14924*/ uint32  PVPWorstDeathStreak;
/*14928*/ uint32  PVPCurrentKillStreak;
/*14932*/ PVPStatsEntry_Struct PVPLastKill;
/*15020*/ PVPStatsEntry_Struct PVPLastDeath;
/*15108*/ uint32  PVPNumberOfKillsInLast24Hours;
/*15112*/ PVPStatsEntry_Struct PVPRecentKills[50];
/*19512*/ uint32 expAA;               // Exp earned in current AA point
/*19516*/ uint8 unknown19516[40];
/*19556*/ uint32 currentRadCrystals;  // Current count of radiant crystals
/*19560*/ uint32 careerRadCrystals;   // Total count of radiant crystals ever
/*19564*/ uint32 currentEbonCrystals; // Current count of ebon crystals
/*19568*/ uint32 careerEbonCrystals;  // Total count of ebon crystals ever
/*19572*/ uint8  groupAutoconsent;    // 0=off, 1=on
/*19573*/ uint8  raidAutoconsent;     // 0=off, 1=on
/*19574*/ uint8  guildAutoconsent;    // 0=off, 1=on
/*19575*/ uint8  unknown19575;     // ***Placeholder (6/29/2005)
/*19576*/ uint32 level3;              // Titanium looks here to determine the max leadership points you can bank.
/*19580*/ uint32 showhelm;            // 0=no, 1=yes
/*19584*/ uint8  unknown19584[4];     // ***Placeholder (10/27/2005)
/*19588*/ uint32 unknown19588;        // *** Placeholder
/*19584*/
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
/*000*/ uint32	command;
/*004*/ uint32	target;
};

/*
** Delete Spawn
** Length: 4 Bytes
** OpCode: OP_DeleteSpawn
*/
struct DeleteSpawn_Struct
{
/*00*/ uint32 spawn_id;             // Spawn ID to delete
/*04*/
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
/*11*/	char	sayer[1];				// Who is the source of the info
/*12*/	uint8	unknown12[12];
/*24*/	char	message[1];				// What is being said?
};

/*
** When somebody changes what they're wearing
**      or give a pet a weapon (model changes)
** Length: 16 Bytes
** Opcode: 9220
*/
struct WearChange_Struct{
/*000*/ uint16 spawn_id;
/*002*/ uint16 material;
/*004*/ Tint_Struct color;
/*009*/ uint8 wear_slot_id;
};

/*
** Type:   Bind Wound Structure
** Length: 8 Bytes
*/
//Fixed for 7-14-04 patch
struct BindWound_Struct
{
/*002*/	uint16	to;			// TargetID
/*004*/	uint16	unknown2;		// ***Placeholder
/*006*/	uint16	type;
/*008*/	uint16	unknown6;
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
/*068*/	float	y;
/*072*/	float	x;
/*076*/	float	z;
/*080*/	uint32	zone_reason;	//0x0A == death, I think
/*084*/	int32	success;		// =0 client->server, =1 server->client, -X=specific error
/*088*/
};

struct RequestClientZoneChange_Struct {
/*00*/	uint16	zone_id;
/*02*/	uint16	instance_id;
/*04*/	float	y;
/*08*/	float	x;
/*12*/	float	z;
/*16*/	float	heading;
/*20*/	uint32	type;	//unknown... values
};

struct Animation_Struct {
/*00*/	uint16 spawnid;
/*02*/	uint8 speed;
/*03*/	uint8 action;
/*04*/
};

// this is what causes the caster to animate and the target to
// get the particle effects around them when a spell is cast
// also causes a buff icon
struct Action_Struct
{
 /* 00 */	uint16 target;	// id of target
 /* 02 */	uint16 source;	// id of caster
 /* 04 */	uint16 level; // level of caster for spells, OSX dump says attack rating, guess spells use it for level
 /* 06 */	uint32 instrument_mod; // OSX dump says base damage, spells use it for bard song (different from newer clients)
 /* 10 */	float force;
 /* 14 */	float hit_heading;
 /* 18 */	float hit_pitch;
 /* 22 */	uint8 type;		// 231 (0xE7) for spells, skill
 /* 23 */	uint16 unknown23; // OSX says min_damage
 /* 25 */	uint16 unknown25; // OSX says tohit
 /* 27 */	uint16 spell;	// spell id being cast
 /* 29 */	uint8 spell_level;
// this field seems to be some sort of success flag, if it's 4
 /* 30 */	uint8 effect_flag;	// if this is 4, a buff icon is made
 /* 31 */
};

// this is what prints the You have been struck. and the regular
// melee messages like You try to pierce, etc.  It's basically the melee
// and spell damage message
struct CombatDamage_Struct
{
/* 00 */	uint16	target;
/* 02 */	uint16	source;
/* 04 */	uint8	type; //slashing, etc.  231 (0xE7) for spells, skill
/* 05 */	uint16	spellid;
/* 07 */	uint32	damage;
/* 11 */	float force;
/* 15 */	float hit_heading;	// see above notes in Action_Struct
/* 19 */	float hit_pitch;
/* 23 */
};

/*
** Consider Struct
** Length: 24 Bytes
** OpCode: 3721
*/
struct Consider_Struct{
/*000*/ uint32	playerid;               // PlayerID
/*004*/ uint32	targetid;               // TargetID
/*008*/ uint32	faction;                // Faction
/*0012*/ uint32	level;                  // Level
/*016*/ int32	cur_hp;                  // Current Hitpoints
/*020*/ int32	max_hp;                  // Maximum Hitpoints
/*024*/ uint8 pvpcon;     // Pvp con flag 0/1
/*025*/ uint8	unknown3[3];
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

/*
** Spawn position update
**	Struct sent from server->client to update position of
**	another spawn's position update in zone (whether NPC or PC)
**
*/
struct PlayerPositionUpdateServer_Struct
{
/*0000*/	uint16	spawn_id;
/*0002*/	int32	delta_heading : 10,	// change in heading
					x_pos : 19,			// x coord
					padding0002 : 3;	// ***Placeholder
/*0006*/	int32	y_pos : 19,			// y coord
					animation : 10,		// animation
					padding0006 : 3;	// ***Placeholder
/*0010*/	int32	z_pos : 19,			// z coord
					delta_y : 13;		// change in y
/*0014*/	int32	delta_x : 13,		// change in x
					heading : 12,		// heading
					padding0014 : 7;	// ***Placeholder
/*0018*/	int32	delta_z : 13,		// change in z
					padding0018 : 19;	// ***Placeholder
/*0022*/
};

/*
** Player position update
**	Struct sent from client->server to update
**	player position on server
**
*/
struct PlayerPositionUpdateClient_Struct
{
/*0000*/	uint16	spawn_id;
/*0022*/	uint16	sequence;			// increments one each packet
/*0004*/	float	y_pos;				// y coord
/*0008*/	float	delta_z;			// Change in z
/*0016*/	float	delta_x;			// Change in x
/*0012*/	float	delta_y;			// Change in y
/*0020*/	int32	animation : 10,		// animation
					delta_heading : 10,	// change in heading
					padding0020 : 12;	// ***Placeholder (mostly 1)
/*0024*/	float	x_pos;				// x coord
/*0028*/	float	z_pos;				// z coord
/*0034*/	uint16	heading : 12,		// Directional heading
					padding0004 : 4;	// ***Placeholder
/*0032*/	uint8	unknown0006[2];		// ***Placeholder
/*0036*/
};

/*
** Spawn HP Update
** Length: 10 Bytes
** OpCode: OP_HPUpdate
*/
struct SpawnHPUpdate_Struct
{
/*00*/ uint32	cur_hp;               // Id of spawn to update
/*04*/ int32	max_hp;                 // Maximum hp of spawn
/*08*/ int16	spawn_id;                 // Current hp of spawn
/*10*/
};
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
/*04*/	char			SerializedItem[1];
/*xx*/
};

struct BulkItemPacket_Struct
{
/*00*/	char			SerializedItem[0];
/*xx*/
};

struct Consume_Struct
{
/*0000*/ uint32	slot;
/*0004*/ uint32	auto_consumed; // 0xffffffff when auto eating e7030000 when right click
/*0008*/ uint8	c_unknown1[4];
/*0012*/ uint8	type; // 0x01=Food 0x02=Water
/*0013*/ uint8	unknown13[3];
};

struct DeleteItem_Struct
{
/*0000*/ uint32	from_slot;
/*0004*/ uint32	to_slot;
/*0008*/ uint32	number_in_stack;
/*0012*/
};

struct MoveItem_Struct
{
/*0000*/ uint32	from_slot;
/*0004*/ uint32	to_slot;
/*0008*/ uint32	number_in_stack;
/*0012*/
};

struct MultiMoveItemSub_Struct
{
/*0000*/ InventorySlot_Struct	from_slot;
/*0016*/ uint32 number_in_stack; // so the amount we are moving from the source
/*0020*/ InventorySlot_Struct	to_slot;
};

struct MultiMoveItem_Struct
{
/*0000*/ uint32	count;
/*0004*/ MultiMoveItemSub_Struct moves[0];
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

static const uint32 MAX_NUMBER_GUILDS = 1500;
struct GuildsList_Struct {
	uint8 head[64]; // First on guild list seems to be empty...
	GuildsListEntry_Struct Guilds[MAX_NUMBER_GUILDS];
};

struct GuildUpdate_Struct {
	uint32	guildID;
	GuildsListEntry_Struct entry;
};

struct GuildTributeDonateItemRequest_Struct {
	/*000*/ uint32 	slot;
	/*004*/ uint32 	quantity;
	/*008*/ uint32	tribute_master_id;
	/*012*/ uint32 	unknown12;
	/*016*/ uint32	guild_id;
	/*020*/ uint32 	unknown20;
	/*024*/ uint32	unknown24;
};

struct GuildTributeDonateItemReply_Struct {
	/*000*/ uint32	slot;
	/*004*/ uint32	quantity;
	/*008*/ uint32	unknown8;
	/*012*/	uint32	favor;
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

//opcode = 0x5220
// size 292


struct LootingItem_Struct {
/*000*/	uint32	lootee;
/*002*/	uint32	looter;
/*004*/	uint16	slot_id;
/*006*/	uint8	unknown3[2];
/*008*/	int32	auto_loot;
};

struct GuildManageStatus_Struct{
	uint32	guildid;
	uint32	oldrank;
	uint32	newrank;
	char	name[64];
};
// Guild invite, remove
struct GuildJoin_Struct{
/*000*/	uint32	guildid;
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
	char othername[64];
	char myname[64];
	uint16 guildeqid;
	uint8 unknown[2]; // for guildinvite all 0's, for remove 0=0x56, 2=0x02
	uint32 officer;
};

// 4244 bytes. Is not really an 'OnLevelMessage', it causes a popup box to display in the client
// Text looks like HTML.
struct OnLevelMessage_Struct {
/*0000*/       char    Title[128];
/*0128*/       char    Text[4096];
/*4224*/       uint32  Buttons;
/*4228*/       uint32  Duration;
/*4232*/       uint32  PopupID;
/*4236*/       uint32  unknown4236;
/*4240*/       uint32  unknown4240;
/*4244*/
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
	char name[64];
	char gmname[64];
	char lastname[64];
	uint16 unknown[4];	// 0x00, 0x00
					    // 0x01, 0x00 = Update the clients
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

struct LFG_Struct {
/*
Wrong size on OP_LFG. Got: 80, Expected: 68
   0: 00 00 00 00 01 00 00 00 - 00 00 00 00 64 00 00 00  | ............d...
  16: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
  32: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
  48: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
  64: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
Wrong size on OP_LFG. Got: 80, Expected: 68
   0: 00 00 00 00 01 00 00 00 - 3F 00 00 00 41 00 00 00  | ........?...A...
  16: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
  32: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
  48: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
  64: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
Wrong size on OP_LFG. Got: 80, Expected: 68
   0: 00 00 00 00 01 00 00 00 - 3F 00 00 00 41 00 00 00  | ........?...A...
  16: 46 72 75 62 20 66 72 75 - 62 20 66 72 75 62 00 00  | Frub frub frub..
  32: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
  48: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
  64: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
*/
/*000*/	uint32 unknown000;
/*004*/	uint32 value; // 0x00 = off 0x01 = on
/*008*/	uint32 unknown008;
/*012*/	uint32 unknown012;
/*016*/	char	name[64];
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
	uint32	year;
};

// Darvik: shopkeeper structs
struct MerchantClick_Struct {
/*000*/ uint32	npc_id;			// Merchant NPC's entity id
/*004*/ uint32	player_id;
/*008*/ uint32	command;		//1=open, 0=cancel/close
/*012*/ float	rate;			//cost multiplier, dosent work anymore
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
/*000*/	uint32	npcid;			// Merchant NPC's entity id
/*004*/	uint32	playerid;		// Player's entity id
/*008*/	uint32	itemslot;
		uint32	unknown12;
/*016*/	uint32	quantity;
/*020*/ uint32	price;
};
struct Merchant_Purchase_Struct {
/*000*/	uint32	npcid;			// Merchant NPC's entity id
/*004*/	uint32	itemslot;		// Player's entity id
/*008*/	uint32	quantity;
/*012*/	uint32	price;
};
struct Merchant_DelItem_Struct{
/*000*/	uint32	npcid;			// Merchant NPC's entity id
/*004*/	uint32	playerid;		// Player's entity id
/*008*/	uint32	itemslot;
};
struct Adventure_Purchase_Struct {
/*000*/	uint32	some_flag;	//set to 1 generally...
/*000*/	uint32	npcid;
/*004*/	uint32	itemid;
/*008*/	uint32	variable;
};

struct Adventure_Sell_Struct {
/*000*/	uint32	unknown000;	//0x01
/*004*/	uint32	npcid;
/*008*/	uint32	slot;
/*012*/	uint32	charges;
/*016*/	uint32	sell_price;
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



/*struct Item_Shop_Struct {
	uint16 merchantid;
	uint8 itemtype;
	Item_Struct item;
	uint8 iss_unknown001[6];
};*/

struct Illusion_Struct {
/*000*/	uint32	spawnid;
/*004*/	char	charname[64];
/*068*/	int race;
/*072*/	uint8	gender;
/*073*/	uint8	texture;
/*074*/	uint8	helmtexture;
/*075*/	uint8	unknown075;
/*076*/	uint32	face;
/*080*/	uint8	hairstyle;
/*081*/	uint8	haircolor;
/*082*/	uint8	beard;
/*083*/	uint8	beardcolor;
/*084*/	float	size;
/*088*/	char	unknown084[80];
/*168*/
};

struct ZonePoint_Entry {
/*0000*/	uint32	iterator;
/*0004*/	float	y;
/*0008*/	float	x;
/*0012*/	float	z;
/*0016*/	float	heading;
/*0020*/	uint16	zoneid;
/*0022*/	uint16	zoneinstance; // LDoN instance
};

struct ZonePoints {
/*0000*/	uint32	count;
/*0004*/	struct	ZonePoint_Entry zpe[0]; // Always add one extra to the end after all zonepoints
};

struct SkillUpdate_Struct {
/*00*/	uint32 skillId;
/*04*/	uint32 value;
/*08*/
};

struct ZoneUnavail_Struct {
	//This actually varies, but...
	char zonename[16];
	int16 unknown[4];
};

struct GroupGeneric_Struct {
	char name1[64];
	char name2[64];
};

struct GroupCancel_Struct {
	char	name1[64];
	char	name2[64];
	uint8	toggle;
};

struct GroupUpdate_Struct {
/*0000*/	uint32	action;
/*0004*/	char	yourname[64];
/*0068*/	char	membername[5][64];
/*0388*/	char	leadersname[64];
};

struct GroupUpdate2_Struct {
/*0000*/	uint32	action;
/*0004*/	char	yourname[64];
/*0068*/	char	membername[5][64];
/*0388*/	char	leadersname[64];
/*0452*/	GroupLeadershipAA_Struct leader_aas;
/*0580*/	uint8	unknown[188];
/*0768*/
};
struct GroupJoin_Struct {
/*0000*/	uint32	action;
/*0004*/	char	yourname[64];
/*0068*/	char	membername[64];
/*0132*/	uint8	unknown[84];
};

struct FaceChange_Struct {
/*000*/	uint8	haircolor;
/*001*/	uint8	beardcolor;
/*002*/	uint8	eyecolor1; // the eyecolors always seem to be the same, maybe left and right eye?
/*003*/	uint8	eyecolor2;
/*004*/	uint8	hairstyle;
/*005*/	uint8	beard;	// vesuvias
/*006*/	uint8	face;
//vesuvias:
//there are only 10 faces for barbs changing woad just
//increase the face value by ten so if there were 8 woad
//designs then there would be 80 barb faces
};

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


struct Who_All_Struct { // 76 length total
/*000*/	char	whom[64];
/*064*/	uint32	wrace;		// FF FF = no race

/*066*/	uint32	wclass;		// FF FF = no class
/*068*/	uint32	lvllow;		// FF FF = no numbers
/*070*/	uint32	lvlhigh;	// FF FF = no numbers
/*072*/	uint32	gmlookup;	// FF FF = not doing /who all gm
/*074*/	uint32	unknown074;
/*076*/	uint8	unknown076[64];
/*140*/
};

struct Stun_Struct { // 4 bytes total
	uint32 duration; // Duration of stun
};

struct AugmentItem_Struct {
/*00*/	int16	container_slot;
/*02*/	char	unknown02[2];
/*04*/	int32	augment_slot;
/*08*/
};

// OP_Emote
struct Emote_Struct {
/*0000*/	uint32 unknown01;
/*0004*/	char message[1024];
/*1028*/
};

// Inspect
struct Inspect_Struct {
	uint32 TargetID;
	uint32 PlayerID;
};
//OP_InspectAnswer
struct InspectResponse_Struct{//Cofruben:need to send two of this for the inspect response.
/*000*/	uint32 TargetID;
/*004*/	uint32 playerid;
/*008*/	char itemnames[22][64];
/*1416*/uint32 itemicons[22];
/*1504*/char text[288];
/*1792*/
};

//OP_SetDataRate
struct SetDataRate_Struct {
	float newdatarate;
};

//OP_SetServerFilter
struct SetServerFilter_Struct {
	uint32 filters[29];		//see enum eqFilterType
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

// This is where the Text is sent to the client.
// Use ` as a newline character in the text.
// Variable length.
struct BookText_Struct {
	uint8 window;	// where to display the text (0xFF means new window)
	uint8 type;             //type: 0=scroll, 1=book, 2=item info.. prolly others.
	char booktext[1]; // Variable Length
};
// This is the request to read a book.
// This is just a "text file" on the server
// or in our case, the 'name' column in our books table.
struct BookRequest_Struct {
	uint8 window;	// where to display the text (0xFF means new window)
	uint8 type;             //type: 0=scroll, 1=book, 2=item info.. prolly others.
	char txtfile[1];	// Variable length
};

/*
** Object/Ground Spawn struct
** Used for Forges, Ovens, ground spawns, items dropped to ground, etc
** Size: 92 bytes
** OpCodes: OP_CreateObject
** Last Updated: Oct-17-2003
**
*/
struct Object_Struct {
/*00*/	uint32	linked_list_addr[2];// They are, get this, prev and next, ala linked list
/*08*/	uint16	unknown008[2];		//
/*12*/	uint32	drop_id;			// Unique object id for zone
/*16*/	uint16	zone_id;			// Redudant, but: Zone the object appears in
/*18*/	uint16	zone_instance;		//
/*20*/	uint32	unknown020;			//
/*24*/	uint32	unknown024;			//
/*28*/	float	heading;			// heading
/*32*/	float	z;					// z coord
/*36*/	float	x;					// x coord
/*40*/	float	y;					// y coord
/*44*/	char	object_name[32];	// Name of object, usually something like IT63_ACTORDEF
/*76*/	uint32	unknown076;			//
// ShowEQ shows an extra field in here...
/*80*/	uint32	object_type;		// Type of object, not directly translated to OP_OpenObject
/*84*/	uint32	unknown084;			//set to 0xFF
/*88*/	uint32	spawn_id;			// Spawn Id of client interacting with object
/*92*/
};
//01 = generic drop, 02 = armor, 19 = weapon
//[13:40] and 0xff seems to be indicative of the tradeskill/openable items that end up returning the old style item type in the OP_OpenObject

/*
** Click Object Struct
** Client clicking on zone object (forge, groundspawn, etc)
** Size: 8 bytes
** Last Updated: Oct-17-2003
**
*/
struct ClickObject_Struct {
/*00*/	uint32 drop_id;
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
struct ClickObjectAck_Struct {
/*00*/	uint32	player_id;	// Entity Id of player who clicked object
/*04*/	uint32	drop_id;	// Zone-specified unique object identifier
/*08*/	uint32	open;		// 1=opening, 0=closing
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
/*0052*/ uint16   size;			// 100 is normal, smaller number = smaller model
/*0054*/ uint8    unknown0038[6];
/*0060*/ uint8   doorId;             // door's id #
/*0061*/ uint8   opentype;
/*
 *  Open types:
 * 66 = PORT1414 (Qeynos)
 * 55 = BBBOARD (Qeynos)
 * 100 = QEYLAMP (Qeynos)
 * 56 = CHEST1 (Qeynos)
 * 5 = DOOR1 (Qeynos)
 */
/*0062*/ uint8  state_at_spawn;
/*0063*/ uint8  invert_state;	// if this is 1, the door is normally open
/*0064*/ uint32  door_param;
/*0068*/ uint8  unknown0052[12]; // mostly 0s, the last 3 bytes are something tho
/*0080*/
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

struct Resurrect_Struct	{
	uint32	unknown00;
	uint16	zone_id;
	uint16	instance_id;
	float	y;
	float	x;
	float	z;
	char	your_name[64];
	uint32	unknown88;
	char	rezzer_name[64];
	uint32	spellid;
	char	corpse_name[64];
	uint32	action;
/* 228 */
};

struct SetRunMode_Struct {
	uint8 mode;
	uint8 unknown[3];
};

//EnvDamage is EnvDamage2 without a few bytes at the end.

struct EnvDamage2_Struct {
/*0000*/	uint32 id;
/*0004*/	uint16 unknown4;
/*0006*/	uint32 damage;
/*0010*/	uint8 unknown10[12];
/*0022*/	uint8 dmgtype; //FA = Lava; FC = Falling
/*0023*/	uint8 unknown2[4];
/*0027*/	uint16 constant; //Always FFFF
/*0029*/	uint16 unknown29;
};

//Bazaar Stuff =D

struct BazaarWindowStart_Struct {
	uint8   Action;
	uint8   Unknown001;
	uint16  Unknown002;
};


struct BazaarWelcome_Struct {
	BazaarWindowStart_Struct beginning;
	uint32  traders;
	uint32  items;
	uint8   unknown1[8];
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

struct BazaarInspect_Struct {
	uint32 action;
	char   player_name[64];
	uint32 unknown_068;
	uint32 serial_number;
	uint32 unknown_076;
	uint32 item_id;
};

struct BazaarReturnDone_Struct{
	uint32 type;
	uint32 traderid;
	uint32 unknown8;
	uint32 unknown12;
	uint32 unknown16;
};

struct BazaarSearchResults_Struct {
	BazaarWindowStart_Struct Beginning;
	uint32                   entity_id;
	uint32                   unknown_008;
	uint32                   item_id;
	uint32                   serial_number;
	uint32                   unknown_020;
	char                     item_name[64];
	uint32                   item_cost;
	uint32                   item_stat;
};

struct ServerSideFilters_Struct {
uint8	clientattackfilters; // 0) No, 1) All (players) but self, 2) All (players) but group
uint8	npcattackfilters;	 // 0) No, 1) Ignore NPC misses (all), 2) Ignore NPC Misses + Attacks (all but self), 3) Ignores NPC Misses + Attacks (all but group)
uint8	clientcastfilters;	 // 0) No, 1) Ignore PC Casts (all), 2) Ignore PC Casts (not directed towards self)
uint8	npccastfilters;		 // 0) No, 1) Ignore NPC Casts (all), 2) Ignore NPC Casts (not directed towards self)
};

/*
** Client requesting item statistics
** Size: 44 bytes
** Used In: OP_ItemLinkClick
** Last Updated: 2/15/2009
**
*/
struct	ItemViewRequest_Struct {
/*000*/	uint32	item_id;
/*004*/	uint32	augments[5];
/*024*/ uint32	link_hash;
/*028*/	char	unknown028[16];
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
/*004*/	uint32	unknown004;
/*008*/	uint32	unknown008;
/*012*/	uint32	unknown012;	// htonl(1) on live
/*016*/	uint32	unknown016;	// htonl(1) on live
/*020*/	uint8	unknown020[12];
/*032*/	char	worldshortname[32];
/*064*/	uint8	unknown064[32];
/*096*/	char	unknown096[16];	// 'pacman' on live
/*112*/	char	unknown112[16];	// '64.37,148,36' on live
/*126*/	uint8	unknown128[48];
/*176*/	uint32	unknown176;	// htonl(0x00002695)
/*180*/	char	unknown180[80];	// 'eqdataexceptions@mail.station.sony.com' on live
/*260*/	uint8	unknown260;	// 0x01 on live
/*261*/	uint8	unknown261;	// 0x01 on live
/*262*/	uint8	unknown262[2];
/*264*/
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
	//uint16 padding002;
	float distance;
};

struct Tracking_Struct {
	Track_Struct Entrys[0];
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
	uint8	unknown35; //0A
	uint32	unknown36;//0s
	uint32	playersinzonestring;
	uint32	unknown44[2]; //0s
	uint32	unknown52;//1
	uint32	unknown56;//1
	uint32	playercount;//1
	struct WhoAllPlayer player[0];
};

struct BeginTrader_Struct {
	uint32 action;
	uint32 unknown04;
	uint64 serial_number[EQ::invtype::BAZAAR_SIZE];
	uint32 cost[EQ::invtype::BAZAAR_SIZE];
};

struct Trader_Struct {
	uint32 action;
	uint32 unknown004;
	uint64 item_id[EQ::invtype::BAZAAR_SIZE];
	uint32 item_cost[EQ::invtype::BAZAAR_SIZE];
};

struct ClickTrader_Struct {
	uint32	code;
	uint32	unknown[161];//damn soe this is totally pointless :/ but at least your finally using memset! Good job :) -LE
	uint32	itemcost[EQ::invtype::BAZAAR_SIZE];
};

struct GetItems_Struct{
	uint32	items[EQ::invtype::BAZAAR_SIZE];
};

struct BecomeTrader_Struct {
	uint32 entity_id;
	uint32 action;
	char   trader_name[64];
};

struct Trader_ShowItems_Struct{
	uint32 action;
	uint32 entity_id;
	uint32 unknown08[3];
};

struct TraderBuy_Struct {
/*000*/ uint32  action;
/*004*/ uint32  price;
/*008*/ uint32  trader_id;
/*012*/ char    item_name[64];
/*076*/ uint32  unknown_076;
/*080*/ uint32  item_id;
/*084*/ uint32  already_sold;
/*088*/ uint32  quantity;
/*092*/ uint32  unknown_092;
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

struct TraderDelItem_Struct{
	uint32 slotid;
	uint32 quantity;
	uint32 unknown;
};

struct TraderClick_Struct{
	uint32 trader_id;
	uint32 action;
	uint32 unknown_004;
	uint32 approval;
};

struct FormattedMessage_Struct{
	uint32	unknown0;
	uint32	string_id;
	uint32	type;
	char	message[0];
};
struct SimpleMessage_Struct{
	uint32	string_id;
	uint32	color;
	uint32	unknown8;
};

struct GuildMemberEntry_Struct {
	char	name[1];					//variable length
	uint32	level;						//network byte order
	uint32	banker;						//1=yes, 0=no, network byte order
	uint32	class_;						//network byte order
	uint32	rank;						//network byte order
	uint32	time_last_on;				//network byte order
	uint32	tribute_enable;				//network byte order
	uint32	total_tribute;				//total guild tribute donated, network byte order
	uint32	last_tribute;				//unix timestamp
	uint32	unknown_one;		//unknown, set to one. (network byte order)
	char	public_note[1];				//variable length.
	uint16	zoneinstance;				//network byte order
	uint16	zone_id;					//network byte order
/* 42 + strings */
};

struct GuildMembers_Struct {	//just for display purposes, this is not actually used in the message encoding.
	char	player_name[1];		//variable length.
	uint32	count;				//network byte order
	GuildMemberEntry_Struct member[0];
};

struct GuildMOTD_Struct{
/*0000*/	uint32	unknown0;
/*0004*/	char	name[64];
/*0068*/	char	setby_name[64];
/*0132*/	uint32	unknown132;
/*0136*/	char	motd[512];
};
struct GuildUpdate_PublicNote{
	uint32	unknown0;
	char	name[64];
	char	target[64];
	char	note[100]; //we are cutting this off at 100, actually around 252
};
struct GuildDemoteStruct{
	char	name[64];
	char	target[64];
};
struct GuildRemoveStruct{
	char	target[64];
	char	name[64];
	uint32	unknown128;
	uint32	leaderstatus; //?
};
struct GuildMakeLeader{
	char	name[64];
	char	target[64];
};

struct BugReport_Struct {
/*0000*/	char	category_name[64];
/*0064*/	char	character_name[64];
/*0128*/	char	unused_0128[32];
/*0160*/	char	ui_path[128];
/*0288*/	float	pos_x;
/*0292*/	float	pos_y;
/*0296*/	float	pos_z;
/*0300*/	uint32	heading;
/*0304*/	uint32	unused_0304;
/*0308*/	uint32	time_played;
/*0312*/	char	padding_0312[8];
/*0320*/	uint32	target_id;
/*0324*/	char	padding_0324[140];
/*0464*/	uint32	unknown_0464;	// seems to always be '0'
/*0468*/	char	target_name[64];
/*0532*/	uint32	optional_info_mask;

// this looks like a butchered 8k buffer with 2 trailing dword fields
/*0536*/	char	unused_0536[2052];
/*2588*/	char	bug_report[2050];
/*4638*/	char	system_info[4098];
/*8736*/
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

static const uint32 MAX_TRIBUTE_TIERS = 10;

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
	uint32	tribute_id;	//backwards byte order!
	uint32	tier_count;	//backwards byte order!
	TributeLevel_Struct tiers[MAX_TRIBUTE_TIERS];
	char	name[0];
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

struct TributeItem_Struct {
	uint32	slot;
	uint32	quantity;
	uint32	tribute_master_id;
	int32	tribute_points;
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
** Size: 4 bytes
** Used In: OP_TradeSkillCombine
** Last Updated: Oct-15-2003
**
*/
struct NewCombine_Struct {
/*00*/	int16	container_slot;
/*02*/	int16	guildtribute_slot;
/*04*/
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
struct RecipeAutoCombine_Struct {
	uint32 object_type;
	uint32 some_id;
	uint32 unknown1;		//echoed in reply
	uint32 recipe_id;
	uint32 reply_code;		// 93 64 e1 00 (junk) in request
								// 00 00 00 00 in successful reply
								// f5 ff ff ff in 'you dont have all the stuff' reply
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
	uint32	npc_id;
	FindPerson_Point client_pos;
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

struct TaskDescription_Struct {
/*000*/	uint32	activity_count;		//not right.
/*004*/	uint32	taskid;
/*008*/	uint8	unk;
/*009*/	uint32	id3;
/*013*/	uint32	unknown13;
/*017*/	char	name[1];	//variable length, 0 terminated
/*018*/	uint32	unknown18;
/*022*/	uint32	unknown22;
/*026*/	char	desc[1];	//variable length, 0 terminated
/*027*/	uint32	reward_count;   //not sure
/*031*/	uint32	unknown31;
/*035*/	uint32	unknown35;
/*039*/	uint16	unknown39;
/*041*/	char	reward_link[1];	//variable length, 0 terminated
/*042*/	uint32	unknown43;  //maybe crystal count?
};

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


struct TaskActivity_Struct {
/*000*/	uint32	activity_count;		//not right
/*004*/	uint32	id3;
/*008*/	uint32	taskid;
/*012*/	uint32	activity_id;
/*016*/	uint32	unknown016;
/*020*/	uint32	activity_type;
/*024*/	uint32	unknown024;
/*028*/	uint32	unknown28;
/*032*/	char mob_name[1];	//variable length, 0 terminated
/*033*/	char item_name[1];	//variable length, 0 terminated
/*034*/	uint32	goal_count;
/*038*/	uint32	unknown38;	//0xFFFFFFFF
/*042*/	uint32	unknown42;	//0xFFFFFFFF
/*046*/	uint32	unknown46;  //saw 0x151,0x156
/*050*/	uint32	unknown50;  //saw 0x404,0
/*054*/	char activity_name[1];  //variable length, 0 terminated... commonly empty
/*055*/	uint32	done_count;
/*059*/	uint32	unknown59;	//=1 except on unknown and terminal activities?
/*063*/
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

struct AcceptNewTask_Struct {
	uint32	task_id;		//set to 0 for 'decline'
	uint32	task_master_id;	//entity ID
};

//was all 0's from client, server replied with same op, all 0's
struct CancelTask_Struct {
	uint32 unknown0;
	uint32 unknown4;
};

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


struct BankerChange_Struct {
	uint32	platinum;
	uint32	gold;
	uint32	silver;
	uint32	copper;
	uint32	platinum_bank;
	uint32	gold_bank;
	uint32	silver_bank;
	uint32	copper_bank;
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
/*08*/	uint32	pointsleft;
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
/*000*/	uint32		action;	//=10
/*004*/	char		player_name[64];	//should both be the player's name
/*068*/	char		leader_name[64];
/*132*/	uint32		parameter;
};

struct RaidAddMember_Struct {
/*000*/	RaidGeneral_Struct raidGen;
/*136*/	uint8		_class;
/*137*/	uint8		level;
/*138*/	uint8		isGroupLeader;
/*139*/	uint8		unknown139;	//seems to be 0x42 or 0
};

struct RaidNote_Struct {
/*000*/ RaidGeneral_Struct general;
/*136*/ char note[64];
};

struct RaidMOTD_Struct {
/*000*/ RaidGeneral_Struct general; // leader_name and action only used
/*136*/ char motd[1024]; // max size is 1024, but reply is variable
};

struct RaidLeadershipUpdate_Struct {
	/*000*/	uint32 action;
	/*004*/	char player_name[64];
//	/*068*/	uint32 Unknown068;
	/*072*/	char leader_name[64];
	/*136*/	GroupLeadershipAA_Struct group; //unneeded
	/*200*/	RaidLeadershipAA_Struct raid;
	/*264*/	char Unknown264[128];
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
/*000*/	uint32	type;		//unsure on name, seems to be 0x1, dosent matter
/*005*/	uint8	unknown004[12];
/*016*/	float	src_y;
/*020*/	float	src_x;
/*024*/	float	src_z;
/*028*/	uint8	unknown028[12];
/*040*/	float	velocity;		//4 is normal, 20 is quite fast
/*044*/	float	launch_angle;	//0-450ish, not sure the units, 140ish is straight
/*048*/	float	tilt;		//on the order of 125
/*052*/	uint8	unknown052[8];
/*060*/	float	arc;
/*064*/	uint8	unknown064[12];
/*076*/	uint32	source_id;
/*080*/ uint32	target_id;	//entity ID
/*084*/	uint32	item_id;	//1 to about 150ish
/*088*/	uint32	unknown088;	//seen 125, dosent seem to change anything..
/*092*/ uint32	unknown092;	//seen 16, dosent seem to change anything
/*096*/	uint8	unknown096[5];
/*101*/	char	model_name[16];
/*117*/	uint8	unknown117[19];
};

//made a bunch of trivial structs for stuff for opcode finder to use
struct Consent_Struct {
	char name[1];	//always at least a null
};

struct AdventureMerchant_Struct {
	uint32	unknown_flag;		//seems to be 1
	uint32	entity_id;
};

struct Save_Struct {
	uint8	unknown00[192];
};

struct GMToggle_Struct {
	uint8 unknown0[64];
	uint32 toggle;
};

struct GroupInvite_Struct {
	char invitee_name[64];
	char inviter_name[64];
//	uint8	unknown128[65];
};

struct ColoredText_Struct {
	uint32 color;
	char msg[1];
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
};

struct SendAA_Struct {
/*0000*/	uint32 id;
/*0004*/	uint32 unknown004;		//set to 1.
/*0008*/	uint32 hotkey_sid;
/*0012*/	uint32 hotkey_sid2;
/*0016*/	uint32 title_sid;
/*0020*/	uint32 desc_sid;
/*0024*/	uint32 class_type;
/*0028*/	uint32 cost;
/*0032*/	uint32 seq;
/*0036*/	uint32 current_level; //1s, MQ2 calls this AARankRequired
/*0040*/	uint32 prereq_skill;		//is < 0, abs() is category #
/*0044*/	uint32 prereq_minpoints; //min points in the prereq
/*0048*/	uint32 type;
/*0052*/	uint32 spellid;
/*0056*/	uint32 spell_type;
/*0060*/	uint32 spell_refresh;
/*0064*/	uint32 classes;
/*0068*/	uint32 max_level;
/*0072*/	uint32 last_id;
/*0076*/	uint32 next_id;
/*0080*/	uint32 cost2;
/*0084*/	uint32 unknown80[2]; //0s
/*0088*/	uint32 total_abilities;
/*0092*/	AA_Ability abilities[0];
};

struct AA_List {
	SendAA_Struct* aa[0];
};

struct AA_Action {
/*00*/	uint32	action;
/*04*/	uint32	ability;
/*08*/	uint32	target_id;
/*12*/	uint32	exp_value;
};

struct AAExpUpdate_Struct {
/*00*/	uint32 unknown00;	//seems to be a value from AA_Action.ability
/*04*/	uint32 aapoints_unspent;
/*08*/	uint8 aaxp_percent;	//% of exp that goes to AAs
/*09*/	uint8 unknown09[3];	//live dosent always zero these, so they arnt part of aaxp_percent
};


struct AltAdvStats_Struct {
/*000*/  uint32 experience;
/*004*/  uint16 unspent;
/*006*/  uint16	unknown006;
/*008*/  uint8	percentage;
/*009*/  uint8	unknown009[3];
};

struct PlayerAA_Struct {
	AA_Array aa_list[MAX_PP_AA_ARRAY];
};

struct AATable_Struct {
	AA_Array aa_list[MAX_PP_AA_ARRAY];
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
	uint32 spell[spells::SPELL_GEM_COUNT];
	uint32 unknown;
};

struct ApplyPoison_Struct {
	uint32 inventorySlot;
	uint32 success;
};

struct GuildMemberUpdate_Struct {
/*000*/	uint32 guild_id;	//not sure
/*004*/	char	member_name[64];
/*068*/	uint16	zone_id;
/*070*/	uint16	instance_id;
/*072*/	uint32	unknown072;
};

struct VeteranRewardItem
{
/*000*/	uint32 item_id;
/*004*/	char item_name[256];
};

struct VeteranReward
{
/*000*/	uint32 claim_id;
/*004*/	VeteranRewardItem item;
};

struct ExpeditionInvite_Struct
{
/*000*/ uint32 client_id;
/*004*/ char   inviter_name[64];
/*068*/ char   expedition_name[128];
/*196*/ uint8  swapping;             // 0: adding 1: swapping
/*197*/ char   swap_name[64];        // if swapping, swap name being removed
/*261*/ uint8  padding[3];
/*264*/ uint16 dz_zone_id;           // dz_id zone/instance pair, sent back in reply
/*268*/ uint16 dz_instance_id;
};

struct ExpeditionInviteResponse_Struct
{
/*000*/ uint32 unknown000;
/*004*/ uint16 dz_zone_id;     // dz_id pair sent in invite
/*006*/ uint16 dz_instance_id;
/*008*/ uint8  accepted;       // 0: declined 1: accepted
/*009*/ uint8  swapping;       // 0: adding 1: swapping (sent in invite)
/*010*/ char   swap_name[64];  // swap name sent in invite
/*074*/ uint8  unknown078;     // padding/garbage?
/*075*/ uint8  unknown079;     // padding/garbage?
};

struct DynamicZoneInfo_Struct
{
/*000*/ uint32 client_id;
/*004*/ uint32 assigned; // padded bool
/*008*/ uint32 max_players;
/*012*/ char   dz_name[128];
/*140*/ char   leader_name[64];
};

struct DynamicZoneMemberEntry_Struct
{
/*000*/ char name[1];        // variable length, null terminated, max 0x40 (64)
/*000*/ uint8 online_status; // 0: unknown 1: Online, 2: Offline, 3: In Dynamic Zone, 4: Link Dead
};

struct DynamicZoneMemberList_Struct
{
/*000*/ uint32 client_id;
/*004*/ uint32 member_count;
/*008*/ DynamicZoneMemberEntry_Struct members[0]; // variable length
};

struct DynamicZoneMemberListName_Struct
{
/*000*/ uint32 client_id;
/*004*/ uint32 add_name;   // padded bool, 0: remove name, 1: add name with unknown status
/*008*/ char   name[64];
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
/*004*/ char   leader_name[64];
};

struct ExpeditionCommand_Struct
{
/*000*/ uint32 unknown000;
/*004*/ char   name[64];
};

struct ExpeditionCommandSwap_Struct
{
/*000*/ uint32 unknown000;
/*004*/ char   add_player_name[64]; // swap to (player must confirm)
/*068*/ char   rem_player_name[64]; // swap from
};

struct ExpeditionExpireWarning
{
/*000*/ uint32 client_id;
/*004*/ uint32 minutes_remaining;
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
/*000*/ uint32 unknown000;
/*004*/ uint32 unknown004;
/*008*/ uint32 unknown_id1;
/*012*/ uint16 dz_zone_id;
/*014*/ uint16 dz_instance_id;
/*016*/ uint32 dz_type;        // 1: Expedition, 2: Tutorial, 3: Task, 4: Mission, 5: Quest
/*020*/ uint32 unknown_id2;
/*024*/ uint32 unknown024;
/*028*/ uint32 unknown028;     // always same as unknown040
/*032*/ uint32 unknown032;
/*036*/ uint32 unknown036;
/*040*/ uint32 unknown040;     // always same as unknown028
/*044*/ uint32 unknown044;
};

struct LFGuild_SearchPlayer_Struct
{
/*00*/	uint32	Command;
/*04*/	uint32	Unknown04;
/*08*/	uint32	FromLevel;
/*12*/	uint32	ToLevel;
/*16*/	uint32	MinAA;
/*20*/	uint32	TimeZone;
/*24*/	uint32	Classes;
};

struct LFGuild_SearchGuild_Struct
{
/*00*/	uint32	Command;
/*04*/	uint32	Unknown04;
/*08*/	uint32	Level;
/*12*/	uint32	AAPoints;
/*16*/	uint32	TimeZone;
/*20*/	uint32	Class;
/*24*/
};

struct LFGuild_PlayerToggle_Struct
{
/*000*/ uint32	Command;
/*004*/ uint8	Unknown004[68];
/*072*/ char	Comment[256];
/*328*/ uint8	Unknown328[268];
/*596*/ uint32	TimeZone;
/*600*/ uint8	Toggle;
/*601*/ uint8	Unknown601[7];
/*608*/ uint32	Expires;
/*612*/
};

struct LFGuild_GuildToggle_Struct
{
/*000*/ uint32	Command;
/*004*/ uint8	Unknown004[8];
/*012*/ char	Comment[256];
/*268*/ uint8	Unknown268[256];
/*524*/ uint32	FromLevel;
/*528*/ uint32	 ToLevel;
/*532*/ uint32	Classes;
/*536*/ uint32	AACount;
/*540*/ uint32	TimeZone;
/*544*/ uint8	Toggle;
/*545*/ uint8	Unknown545[3];
/*548*/ uint32	TimePosted;
/*552*/ char	Name[64];
/*616*/
};

//struct ItemEffect_Struct
//{
//	int16	Effect;
//	uint8	Type;
//	uint8	Level;
//	uint8	Level2;
//};
//
//struct Item_Struct
//{
//	// "titanium_itemfields_a.h"
//	char Name; // "\0"
//	char Lore; // "\0"
//	char IDFile; // "\0"
//	int ID;
//
//	int Weight;
//
//	// "titanium_itemfields_b.h"
//	int NoRent;
//	int NoDrop;
//	int Size;
//	int Slots;
//	int Price;
//	int Icon;
//	char Unk1; // '0'
//	char Unk2; // '0'
//	int BenefitFlag;
//	int Tradeskills;
//	int CR;
//	int DR;
//	int PR;
//	int MR;
//	int FR;
//	int AStr;
//	int ASta;
//	int AAgi;
//	int ADex;
//	int ACha;
//	int AInt;
//	int AWis;
//	int HP;
//	int Mana;
//	int AC;
//	int Deity;
//	int SkillModValue;
//	int SkillModMax;
//	int SkillModType;
//	int BaneDmgRace;
//	int BaneDmgAmt;
//	int BaneDmgBody;
//	int Magic;
//	int CastTime_;
//	int ReqLevel;
//	int BardType;
//	int BardValue;
//	int Light;
//	int Delay;
//	int RecLevel;
//	int RecSkill;
//	int ElemDmgType;
//	int ElemDmgAmt;
//	int Range;
//	int Damage;
//	int Color;
//	int Classes;
//	int Races;
//	char Unk3; // '0'
//	int MaxCharges;
//	int ItemType;
//	int Material;
//	int SellRate;
//	char Unk4; // '0'
//	int _CastTime_;
//	char Unk5; // '0'
//	int ProcRate;
//	int CombatEffects;
//	int Shielding;
//	int StunResist;
//	int StrikeThrough;
//	int ExtraDmgSkill;
//	int ExtraDmgAmt;
//	int SpellShield;
//	int Avoidance;
//	int Accuracy;
//	int CharmFileID;
//	int FactionMod1;
//	int FactionMod2;
//	int FactionMod3;
//	int FactionMod4;
//	int FactionAmt1;
//	int FactionAmt2;
//	int FactionAmt3;
//	int FactionAmt4;
//	char CharmFile; // "\0"
//	int AugType;
//
//	struct {
//		int AugSlotType;
//		int AugSlotVisible;
//	} AugSlot[5];
//
//	int LDoNTheme;
//	int LDoNPrice;
//	int LDoNSold;
//	int BagType;
//	int BagSlots;
//	int BagSize;
//	int BagWR;
//	int Book;
//	int BookType;
//	char Filename; // "\0"
//	int BaneDmgRaceAmt;
//	int AugRestrict;
//	int LoreGroup;
//	int PendingLoreFlag;
//	int ArtifactFlag;
//	int SummonedFlag;
//	int Favor;
//	int FVNoDrop;
//	int Endur;
//	int DotShielding;
//	int Attack;
//	int Regen;
//	int ManaRegen;
//	int EnduranceRegen;
//	int Haste;
//	int DamageShield;
//	int RecastDelay;
//	int RecastType;
//	int GuildFavor;
//	int AugDistiller;
//	char Unk6; // '0'
//	char Unk7; // '0'
//	int Attunable;
//	int NoPet;
//	char Unk8; // '0'
//	int PointType;
//	int PotionBelt;
//	int PotionBeltSlots;
//	int StackSize;
//	int NoTransfer;
//	int Stackable;
//
//	int ClickEffect;
//	int ClickType;
//	int ClickLevel2;
//	int ClickLevel;
//	char ClickName; // '0'
//	int ProcEffect;
//	int ProcType;
//	int ProcLevel2;
//	int ProcLevel;
//	char ProcName; // '0'
//	int WornEffect;
//	int WornType;
//	int WornLevel2;
//	int WornLevel;
//	char WornName; // '0'
//	int FocusEffect;
//	int FocusType;
//	int FocusLevel2;
//	int FocusLevel;
//	char FocusName; // '0'
//	int ScrollEffect;
//	int ScrollType;
//	int ScrollLevel2;
//	int ScrollLevel;
//	char ScrollName; // '0'
//};

struct SayLinkBodyFrame_Struct {
/*000*/	char ActionID[1];
/*001*/	char ItemID[5];
/*006*/	char Augment1[5];
/*011*/	char Augment2[5];
/*016*/	char Augment3[5];
/*021*/	char Augment4[5];
/*026*/	char Augment5[5];
/*031*/	char IsEvolving[1];
/*032*/	char EvolveGroup[4];
/*036*/	char EvolveLevel[1];
/*037*/	char Hash[8];
/*045*/
};

struct GuildSetRank_Struct
{
	/*00*/	uint32	unknown00;
	/*04*/	uint32	unknown04;
	/*08*/	uint32	rank;
	/*72*/	char	member_name[64];
	/*76*/	uint32	banker;
};

struct GuildMemberAdd_Struct {
	/*000*/ uint32 guild_id;
	/*004*/ uint32 unknown04;
	/*008*/ uint32 level;
	/*012*/ uint32 class_;
	/*016*/ uint32 rank_;
	/*020*/ uint32 zone_id;
	/*024*/ uint32 last_on;
	/*028*/ char   player_name[64];
};

struct GuildMemberRank_Struct {
	/*000*/ uint32 guild_id;
	/*004*/ uint32 unknown_004;
	/*008*/ uint32 rank_;
	/*012*/ char   player_name[64];
	/*076*/ uint32 alt_banker; //Banker/Alt bit 00 - none 10 - Alt 11 - Alt and Banker 01 - Banker.  Banker not functional for RoF2+
};

enum TiBazaarTraderBuyerActions {
	Zero            = 0,
	BeginTraderMode = 1,
	EndTraderMode   = 2,
	PriceUpdate     = 3,
	EndTransaction  = 4,
	BazaarSearch    = 7,
	WelcomeMessage  = 9,
	BuyTraderItem   = 10,
	ListTraderItems = 11,
	BazaarInspect   = 18,
	ItemMove        = 19,
	ReconcileItems  = 20
};

#pragma pack(pop)

} /*structs*/
} /*Titanium*/
