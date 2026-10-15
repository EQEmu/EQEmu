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

#include "common/eq_packet_structs.h"
#include "common/skills.h"
#include "common/strings.h"

#include "fmt/format.h"

namespace TOB {
	namespace structs {
		// constants
		static const uint32 MAX_PP_AA_ARRAY = 300;
		static const uint32 MAX_PP_SKILL = PACKET_SKILL_ARRAY_SIZE;
		static const uint32 MAX_PP_INNATE_SKILL = 25;
		static const uint32 MAX_PP_DISCIPLINES = 300;
		static const uint32 MAX_PP_COMBAT_ABILITY_TIMERS = 25;
		static const uint32 MAX_PP_UNKNOWN_ABILITIES = 25;
		static const uint32 MAX_RECAST_TYPES = 25;
		static const uint32 MAX_ITEM_RECAST_TYPES = 100;
		static const uint32 BLOCKED_BUFF_COUNT = 60; // this might not be needed?
		static const uint32 BUFF_COUNT = 62;
		static const uint32 MAX_PP_LANGUAGE = 32;
		// Group id sent in the group roster when the sender has no real one (raid group updates). The client reads
		// 0 as "no group", and the Advanced Loot window then ignores the roster.
		static const uint32 GROUP_ID_PLACEHOLDER = 1;
#pragma pack(1)
		
		struct LoginInfo_Struct {
			/*000*/	char	login_info[64];
			/*064*/	uint8	unknown064[124];
			/*188*/	uint8	zoning;			// 01 if zoning, 00 if not
			/*189*/	uint8	unknown189[275];
			/*488*/
		};

		struct ClientZoneEntry_Struct {
			/*00*/ uint32	unknown00;	// ***Placeholder
			/*04*/ char	char_name[64];	// Player firstname [32]
			/*68*/ uint32	unknown68;
			/*72*/ uint32	unknown72;
			/*76*/ uint32	unknown76;
			/*80*/ uint32	unknown80;
			/*84*/ uint32	unknown84;
			/*88*/ uint32	unknown88;
			/*92*/
		};

		struct Membership_Struct
		{
			/*000*/ uint8 membership; //0 not gold, 2 gold
			/*001*/ uint32 races;	// Seen ff ff 01 00
			/*005*/ uint32 classes;	// Seen ff ff 01 00
			/*009*/ uint32 entrysize; // Seen 33
			/*013*/ int32 entries[33]; // Most -1, 1, and 0 for Gold Status
			/*145*/
		};

		struct Membership_Entry_Struct
		{
			/*000*/ uint32 purchase_id;		// Seen 1, then increments 90287 to 90300
			/*004*/ uint32 bitwise_entry;	// Seen 16 to 65536 - Skips 4096
			/*008*/
		};

		struct Membership_Setting_Struct
		{
			/*000*/ int8 setting_index;	// 0, 1, 2 or 3: f2p, silver, gold, platinum?
			/*001*/ int32 setting_id;		// 0 to 23 actually seen but the OP_Membership packet has up to 32
			/*005*/ int32 setting_value;	
			/*009*/
		};

		struct Membership_Details_Struct
		{
			/*000*/ uint32 membership_setting_count;	// Seen 96
			/*004*/ Membership_Setting_Struct settings[96]; // 864 Bytes
			/*364*/ uint32 race_entry_count;	// Seen 17
			/*368*/ Membership_Entry_Struct membership_races[17]; // 136 Bytes
			/*3f0*/ uint32 class_entry_count;	// Seen 15
			/*3f4*/ Membership_Entry_Struct membership_classes[17]; // 136 Bytes
			/*47c*/ uint32 exit_url_length;	// Length of the exit_url string (0 for none)
			/*480*/ //char exit_url[42];		// Upgrade to Silver or Gold Membership URL
		};

		struct MaxCharacters_Struct {
			/*000*/ uint32 total_character_slots; // total character slots, different than max characters
			/*004*/ uint32 marketplace_character_slots;
			/*008*/ uint32 unknown008; // definitely 4 bytes, read in client, value for CEverQuest::Unknown0x0608
			/*00c*/ uint8 head_start_button;
			/*00d*/ uint8 unused00d;
			/*00e*/ uint16 heroic_related;
			/*010*/ int64 heroic_50_count; // read as 64 bits in the client
			/*018*/ int32 heroic_100_count;
			/*01c*/ uint8 disable_character_creation;
			/*01d*/ uint8 unused01d[3];
			/*020*/ int32 monthly_claim; // (-1 for don't set)
			/*024*/ uint8 marketplace_related; // marketplace related boolean (int32 for convenience here, it's 4 bytes)
			/*025*/ uint8 unused025[3];
			/*028*/ int32 unused028;
			/*02c*/ uint8 add_marketplace_chars; // boolean on whether to add or set marketplace characters
			/*02d*/ uint8 add_unknown; // boolean on whether to add unknown008 or set marketplace characters to some unknown global
			/*02e*/ uint8 legacy_characters_ruleset;
			/*02f*/ uint8 unused02f;
			/*030*/ int32 num_max_characters; // used for legacy exp calculation
			/*034*/ int32 num_personas_available;
			/*038*/ int32 has_de_ranger;
			/*03c*/
		};

		struct ExpansionInfo_Struct {
			/*000*/	char	Unknown000[64];
			/*064*/	uint64	Expansions;
		};

		/*
		* Visible equipment.
		* Size: 20 Octets
		*/
		struct Texture_Struct
		{
			uint32 Material; // type
			uint32 Unknown1; // material
			uint32 EliteMaterial; // variation
			uint32 HeroForgeModel; // new armor ID
			uint32 Material2; // new armor type
		};

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

		struct CharSelectEquip : Texture_Struct, Tint_Struct {};

		struct CharacterSelectEntry_Struct
		{
			char Name[1];
			uint32 Class;
			uint32 Race;
			uint8 Level;
			uint32 ShroudClass;
			uint32 ShroudRace;
			uint16 Zone;
			uint16 Instance;
			uint8 Gender;
			uint8 Face;
			CharSelectEquip Equip[9];
			uint8 TextureType; //Seen 256
			uint8 HeadType; //Seen 0
			uint32 DrakkinTattoo; // tattoo index
			uint32 DrakkinDetails; // face attachment index
			uint32 Deity;
			uint32 PrimaryIDFile;
			uint32 SecondaryIDFile;
			uint8 HairColor;
			uint8 BeardColor;
			uint8 EyeColor1;
			uint8 EyeColor2;
			uint8 HairStyle;
			uint8 Beard;
			uint8 GoHome;
			uint8 Tutorial;
			uint32 DrakkinHeritage; // parent ID
			uint8 TooHighLevel;
			uint8 PreFTP;
			uint32 LastLogin;
			uint8 Usable;
			uint16 Shrouded;
			uint8 Unknown;
			uint64 CharacterId; // A Guess, Character I made a little bit after has a number a few hundred after the first
		};

		/*
		** Character Selection Struct
		**
		*/
		struct CharacterSelect_Struct
		{
			/*000*/	uint32 CharCount;	//number of chars in this packet
		};

		/*
		** Character Creation struct
		** Length: 168 Bytes
		** OpCode: 0x1859
		*/
		struct CharCreate_Struct
		{
			/*00*/ uint8 padding[72];
			/*48*/ uint32 gender;
			/*4c*/ uint32 race;
			/*50*/ uint32 class_;
			/*54*/ uint32 deity;
			/*58*/ uint32 start_zone; // this is the zone ID of the start zone
			/*5c*/ uint32 haircolor;
			/*60*/ uint32 beard;
			/*64*/ uint32 beardcolor;
			/*68*/ uint32 hairstyle;
			/*6c*/ uint32 face;
			/*70*/ uint32 eyecolor1;
			/*74*/ uint32 eyecolor2;
			/*78*/ uint32 drakkin_heritage;
			/*7c*/ uint32 drakkin_tattoo;
			/*80*/ uint32 drakkin_details;
			/*84*/ uint32 STR;
			/*88*/ uint32 STA;
			/*8c*/ uint32 AGI;
			/*90*/ uint32 DEX;
			/*94*/ uint32 WIS;
			/*98*/ uint32 INT;
			/*9c*/ uint32 CHA;
			/*a0*/ uint32 tutorial;
			/*a4*/ uint32 heroic_type;
			/*a8*/
		};

		struct NameApproval_Struct {
			char name[64];
			uint32 race_id;
			uint32 class_id;
			uint32 deity_id;
			uint32 heroic_type; // seen 0, client can also send 1-4
			uint32 unknown; // always 0?
		};

		enum TOBAppearance : uint32
		{
			None,
			WhoLevel,
			MaxHealth,
			Invisibility,
			PVP,
			Light,
			Animation,
			Sneak,
			SpawnID,
			Health,
			Linkdead,
			FlyMode,
			GM,
			Anonymous,
			GuildID,
			AFK,
			Pet,
			Summoned,
			Split, // Loot Settings "Auto Split Coin"
			Unknown19,
			SetType,
			NPCName,
			CancelSneakHide,
			AreaHealthRegen,
			AreaManaRegen,
			AreaEnduranceRegen,
			FreezeBeneficialBuffs,
			NPCTintIndex,
			Unknown28,
			Unknown29,
			Unknown30,
			ShowHelm,
			DamageState,
			Unknown33, //Some virtual function call; based on location might be EQPlayers (my guess personally) or FindBits
			TextureType, //Texture ID
			Unknown35,
			Unknown36,
			GuildShow,
			OfflineMode,
			Unknown39,
			AdvancedLooting, // Loot Settings "Use Advanced Looting"
			MasterLooterCandidate, // Loot Settings "Master Looter Candidate"
			Birthdate,
			EncounterLock
		};

		struct SpawnAppearance_Struct
		{
			/*0000*/ uint32 spawn_id;		// ID of the spawn
			/*0004*/ uint32 type;			// Values associated with the type
			/*0008*/ uint64 parameter;		// Type of data sent
			/*0016*/ uint64 lock_id; //the only place client uses this as far as I can tell is when you send 0x2c as type in which case it sets LockID = this
			/*0024*/
		};

		struct ChangeSize_Struct
		{
			/*00*/ uint32 EntityID;
			/*04*/ float Size;
			/*08*/ float CameraOffset;
			/*12*/ float AnimationSpeedRelated;
			/*16*/
		};

		struct Spawn_Struct_Bitfields
		{
			union {
				struct {
					// byte 1
					/*00*/	unsigned   gender : 2;		// Gender (0=male, 1=female, 2=monster)
					/*02*/	unsigned   ispet : 1;			// Guessed based on observing live spawns
					/*03*/	unsigned   afk : 1;			// 0=no, 1=afk
					/*04*/	unsigned   anon : 2;			// 0=normal, 1=anon, 2=roleplay
					/*06*/	unsigned   gm : 1;
					/*07*/	unsigned   sneak : 1;
					// byte 2
					/*08*/	unsigned   lfg : 1;
					/*09*/	unsigned   unk9 : 1;
					/*10*/	unsigned   invis : 12;		// there are 3000 different (non-GM) invis levels
					/*22*/	unsigned   linkdead : 1;		// 1 Toggles LD on or off after name. Correct for RoF2
					/*23*/	unsigned   showhelm : 1;
					// byte 4
					/*24*/	unsigned   betabuffed : 1;		// Prefixes name with !
					/*25*/	unsigned   trader : 1;
					/*26*/	unsigned   animationonpop : 1;
					/*27*/	unsigned   targetable : 1;
					/*28*/	unsigned   targetable_with_hotkey : 1;
					/*29*/	unsigned   showname : 1;
					/*30*/	unsigned   idleanimationsoff : 1; // what we called statue?
					/*31*/	unsigned   untargetable : 1;	// bClickThrough
					// byte 5
					/*32*/	unsigned   buyer : 1;
					/*33*/	unsigned   offline : 1;
					/*34*/	unsigned   interactiveobject : 1;
					/*35*/	unsigned   missile : 1;
					/*36*/	unsigned   title : 1;
					/*37*/	unsigned   suffix : 1;
					/*38*/	unsigned   unk38 : 1;
					/*39*/	unsigned   unk39 : 1;
				};
				uint8 raw[5];
			};
		};

		struct Spawn_Struct_Position
		{
			union {
				struct {
					signed deltaHeading : 10;
					signed animation : 10;
					unsigned pad1 : 12;

					signed deltaX : 13;
					signed z : 19;

					signed y : 19;
					unsigned heading : 12;
					unsigned pad2 : 1;

					signed x : 19;
					signed deltaZ : 13;

					unsigned pitch : 12;
					signed deltaY : 13;
					unsigned pad3 : 7;
				};
				uint32_t raw[5];
			};
		};

		struct Client_Position
		{
			/*0x00*/ float y;
			/*0x04*/ float delta_y;
			/*0x08*/ float x;
			/*0x0c*/ int animation : 10;
			signed padding1 : 22;
			/*0x10*/ float delta_x;
			/*0x14*/ float z;
			/*0x18*/ float delta_z;
			/*0x1c*/ int heading : 12;
			int pitch : 12;
			signed padding2 : 8;
			/*0x20*/ int delta_heading : 10;
			signed padding3 : 22;
			/*0x24*/
		};

		struct PlayerPositionUpdateServer_Struct
		{
			/*00*/ uint16 spawn_id;
			/*02*/ uint16 vehicle_id;
			/*04*/ Spawn_Struct_Position position;
			/*24*/
		};

		struct PlayerPositionUpdateClient_Struct {
			/*00*/ uint16 sequence;
			/*02*/ uint16 spawn_id;
			/*04*/ uint16 vehicle_id;
			/*06*/ Client_Position position;
			/*38*/
		};

		struct Door_Struct
		{
			/*000*/ char name[32];
			/*032*/ float DefaultY;
			/*036*/ float DefaultX;
			/*040*/ float DefaultZ;
			/*044*/ float DefaultHeading;
			/*048*/ uint32 DefaultDoorAngle; //rof2's incline
			/*052*/ float Y; //most (all I've seen?) doors match the defaults here
			/*056*/ float X;
			/*060*/ float Z;
			/*064*/ float Heading;
			/*068*/ float DoorAngle;
			/*072*/ uint32 ScaleFactor; //rof2's size
			/*076*/ uint32 Unknown76; //client doesn't seem to read this
			/*080*/ uint8 Id; //doorid
			/*081*/ uint8 Type; //opentype
			/*082*/ uint8 State; //state_at_spawn
			/*083*/ uint8 DefaultState; //invert_state
			/*084*/ int32 Param; //door_param (spell id?)
			/*088*/ uint32 AdventureDoorId;
			/*092*/ uint32 DynDoorID;
			/*096*/ uint32 RealEstateDoorID;
			/*100*/ uint8 bHasScript;
			/*101*/ uint8 bUsable; //1 if clickable
			/*102*/ uint8 bRemainOpen;
			/*103*/ uint8 bVisible; //1 is visible
			/*104*/ uint8 bHeadingChanged;
			/*105*/ uint8 padding1[3];
			/*108*/ float TopSpeed1;
			/*112*/ float TopSpeed2;
			/*116*/ uint8 bNeedsTimeStampSet;
			/*117*/ uint8 padding2[3];
			/*120*/ float unknownFloat1;
			/*124*/ float unknownFloat2;
			/*128*/ uint8 unknownByte1;
			/*129*/ uint8 padding3[3];
			/*132*/
		};

		struct ZonePoint_Entry {
			/*00*/ uint32 iterator;
			/*04*/ float y;
			/*08*/ float x;
			/*12*/ float z;
			/*16*/ float heading;
			/*20*/ uint16 zoneid;
			/*22*/ uint16 zoneinstance;
			/*24*/ uint32 unknown024;
			/*28*/ uint32 unknown028;
			/*32*/
		};

		struct ZonePoints {
			/*00*/ uint32 count;
			/*04*/ struct ZonePoint_Entry zpe[0]; // Always add one extra to the end after all zonepoints
		};

		struct EnterWorld_Struct {
			/*000*/	char	name[64];
			/*064*/	int32	unknown1; // this appears to always be 0
			/*068*/	int32	zoneID;  // this is -1 for "last zone"
		};

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
			/*032*/	uint8	unknown032[144]; // this is mostly a string passed to the teleport function, it appears to be an override for a message
			/*172*/	uint32	unknown172;
			/*176*/
		};

		struct Weather_Struct {
			/*000*/ uint32 val1;     // 0xFF = clear weather
			/*004*/ uint32 type;     // 0x31=rain, 0x02=snow, 0=normal
			/*008*/ uint32 unknown;  // TOB wire padding; client skips this offset
			/*012*/ uint32 mode;     // server struct's mode field, shifted to +0x0C on TOB
			/*016*/
		};

		struct WearChange_Struct {
			/*000*/ uint32 spawn_id;
			/*004*/ uint32 wear_slot_id;
			/*008*/ uint32 armor_id;
			/*012*/ uint32 variation;
			/*016*/ uint32 material;
			/*020*/ uint32 new_armor_id;
			/*024*/ uint32 new_armor_type;
			/*028*/ uint32 color;
			/*032*/
		};

		struct Who_All_Struct {
			/*000*/ char    whom[64];
			/*064*/ uint8   unknown064[64]; // always zero
			/*128*/ uint32  wrace;          // 0xFFFFFFFF = any race
			/*132*/ uint32  wclass;         // 0xFFFFFFFF = any class
			/*136*/ uint32  lvllow;         // 0xFFFFFFFF = any level
			/*140*/ uint32  lvlhigh;        // 0xFFFFFFFF = any level
			/*144*/ uint32  gmlookup;       // 0xFFFFFFFF = not filtering by GM
			/*148*/ uint32  guildid_flag;   // 0xFFFFFFFF = no special filter; 0x7FC00000 = guild/trader/buyer active
			/*152*/ uint32  guildid;        // actual guild ID for /who guild, else 0
			/*156*/ uint32  unknown09C;     // always 0
			/*160*/ uint32  type;           // 0 = /who, 3 = /who all
			/*164*/ uint8   unknown0A4[12]; // padding
			/*176*/
		};

		struct ExpUpdate_Struct
		{
			/*000*/ uint64 exp; // This is exp % / 1000 now; eg 69250 = 69.25%
			/*008*/ uint64 unknown; // if this is the value "2", it opens up the tip window
		};

		struct DeleteSpawn_Struct
		{
			/*00*/ uint32 spawn_id;		// Spawn ID to delete
			/*04*/ uint8 Decay;			// Seen 1
			/*05*/
		};

		//OP_SetServerFilter
		struct SetServerFilter_Struct {
			uint32 filters[69];
		};

		// Was new to RoF2, doesn't look changed
		// The padding is because these structs are padded to the default 4 bytes
		struct InventorySlot_Struct
		{
			/*000*/	int32 Type;
			/*004*/	int16 Slot;
			/*006*/	int16 SubIndex;
			/*008*/	int16 AugIndex;
			/*010*/	int16 Padding2;
			/*012*/
		};

		// Was new for RoF2 - Used for Merchant_Purchase_Struct, doesn't look changed
		// Can't sellfrom other than main inventory so Slot Type is not needed.
		// There is in general no padding for this, but sometimes a pad will be added
		struct TypelessInventorySlot_Struct
		{
			/*000*/	int16 Slot;
			/*002*/	int16 SubIndex;
			/*004*/	int16 AugIndex;
			/*006*/
		};

		struct Consider_Struct {
			/*000*/ uint32	playerid;               // PlayerID
			/*004*/ uint32	targetid;               // TargetID
			/*008*/ uint32	faction;                // Faction
			/*012*/ uint32	level;					// Level
			/*016*/ uint32	report_mode;			// 0 normally, 4 will do a more detailed report that only works if you have GM flag set
			/*020*/ uint8	rare_creature;			// Will do the rare creature string
			/*021*/ uint8	loot_locked;			// Will list the target as (loot locked)
			/*022*/ uint8	unknown022;				// Padding probably
			/*023*/ uint8	unknown023;				// Padding probably
			/*024*/
		};

		struct Consume_Struct
		{
			/*000*/ InventorySlot_Struct slot;    // ItemGlobalIndex: Type(4)+Slot(2)+SubIndex(2)+AugIndex(2)+Pad(2)
			/*012*/ uint32               unknown; // always 0xFFFFFFFF on wire
			/*016*/ uint8                type;    // 0=Food, 1=Water (server expects 1=Food, 2=Water)
			/*017*/ uint8                mode;    // 0=auto-consume, 1=right-click
			/*018*/ uint8                pad[2];
			/*020*/
		};

		struct SpawnHPUpdate_Struct
		{
			/*00*/ int16	spawn_id;
			/*02*/ int64	cur_hp;
			/*10*/ int64	max_hp;
			/*18*/
		};

		struct ClickDoor_Struct {
			/*00*/ uint16 player_id;
			/*02*/ uint8 padding1[2];
			/*04*/ uint32 item_id;
			/*08*/ uint32 picklockskill;
			/*12*/ uint8 doorid;
			/*13*/ uint8 padding2[3];
		};

		/*
		Flags for special:
		WildRampage: 0x1
		Rampage: 0x2
		NoCastOnText: 0x4
		DoubleBowShot: 0x8
		UnknownSpellFlag: 0x10 // display flag of some sort, setting to 1 calls DisplayChatText
		Flurry: 0x20
		Riposte: 0x40
		Critical: 0x80
		Lucky: 0x100
		FinishingBlow: 0x200
		CripplingBlow: 0x400
		Assassinate: 0x800
		DeadlyStrike: 0x1000
		SlayUndead: 0x2000
		Headshot: 0x4000
		Strikethrough: 0x8000
		LuckyRiposte: 0x10000
		Twincast: 0x20000
		ShieldBlock: 0x40000
		StaffBlock: 0x80000
		Locked: 0x100000
		Might be more flags beyond this but I'm not sure
		*/

		struct CombatDamage_Struct
		{
			/*000*/ uint16 target;
			/*002*/ uint16 source;
			/*004*/ uint32 unknown1; // not read by the client
			/*008*/ int64 damage;
			/*016*/ uint32 special; // flags; will document above
			/*020*/ int32 spellid;
			/*024*/ uint32 spell_level; // spell caster level (unconfirmed; it is used for the spell link)
			/*028*/ float force;
			/*032*/ float hit_heading;
			/*036*/ float hit_pitch;
			/*040*/ uint8 type; // skill
			/*041*/ uint8 isoffhand; // used for determining skill used for message
			/*042*/ uint8 padding[2];
			/*044*/ uint32 unknown2; // not read by the client
			/*048*/ 
		};

		struct EnvDamage2_Struct {
			/*0000*/    uint16 entity_id;   // spawn ID of entity taking damage (self)
			/*0002*/    uint16 unknown2;    // source/attacker spawn_id; 0 for environmental
			/*0004*/    int32  spell_id;    // spell causing damage; -1 or 0 = none
			/*0008*/    uint64 unknown8;
			/*0010*/    int64  damage;      // damage amount
			/*0018*/    uint64 unknown18;
			/*0020*/    uint64 unknown20;
			/*0028*/    float  unknown28;
			/*002C*/    uint32 unknown2C;
			/*0030*/    uint32 unknown30;
			/*0034*/    uint32 unknown34;
			/*0038*/    uint8  dmgtype;     // 0xFA=Lava, 0xFB=Drowning, 0xFC=Falling, 0xFD=Trap
			/*0039*/    uint8  unknown39;
		};

		struct Animation_Struct {
			/*00*/	uint16 spawnid;
			/*02*/	uint8 action;
			/*03*/	uint8 speed;
			/*04*/
		};

		struct Death_Struct
		{
			/*000*/	uint32	spawn_id;
			/*004*/	uint32	killer_id;
			/*008*/	uint32	corpseid;
			/*012*/	uint32	unknown1; //not read by client
			/*016*/	int32   spell_id;
			/*020*/ uint32	attack_skill;
			/*024*/	uint64	damage;
			/*032*/	uint32	unknown2; //not read by client
			/*036*/	uint32	unknown3; //not read by client
			/*040*/
		};

		struct DeleteItem_Struct
		{
			/*0000*/ InventorySlot_Struct	from_slot;
			/*0012*/ InventorySlot_Struct	to_slot;
			/*0024*/ uint32			number_in_stack;
			/*0028*/
		};

		struct MoveItem_Struct
		{
			/*0000*/ InventorySlot_Struct	from_slot;
			/*0012*/ InventorySlot_Struct	to_slot;
			/*0024*/ uint32			number_in_stack;
			/*0028*/
		};

		struct ItemPreviewRequest_Struct
		{
			/*0x00*/ uint32 itemid;             // GetItemRecordNum()
			/*0x04*/ uint32 actor_tag1;         // ItemBase::ActorTag1 [+0x5C] if mold applied, else 0
			/*0x08*/ uint32 actor_tag2;         // ItemBase::ActorTag2 [+0xF8] if mold applied, else 0
			/*0x0C*/ uint32 mold_points;        // GetTotalPointsByThemeEarned if mold, else 0xFFFFFFFF
			/*0x10*/ uint32 mold_data1;         // sub_14064F970(item) if mold, else 0
			/*0x14*/ uint32 mold_data2;         // sub_1406501A0(item) if mold, else 0
			/*0x18*/ uint8  is_marketplace;     // 1 when sent from CMarketplaceWnd, else 0
			/*0x19*/ uint8  is_mold_applied;    // 1 when transformation mold applied (BYTE1 of word), else 0
			/*0x1A*/ uint16 unknown01a;         // uninitialized stack — dword read at var_10 spills 2 extra bytes
			/*0x1C*/
		};

		struct ItemRecastDelay_Struct
		{
			/*000*/ InventorySlot_Struct item_slot;    // zeroed until server struct gains item slot fields
			/*012*/ uint32               recast_delay; // seconds until item can be used again
			/*016*/ uint32               recast_type;  // recast group (1..99); SetCoreItemRecastTimer
			/*020*/
		};

		struct ItemVerifyRequest_Struct
		{
			/*000*/ InventorySlot_Struct inventory_slot; // ItemGlobalIndex: Type(+0 int32) Slot(+4) SubIndex(+6) AugIndex(+8) Pad(+10)
			/*012*/ uint32 target;                       // Target entity ID (g_pTargetPlayer+0x168), or 0
			/*016*/
		};

		struct ItemVerifyReply_Struct
		{
			/*000*/ int32  slot;        // Right-clicked slot (passed as ItemGlobalIndex* to GetItemByGlobalIndex)
			/*004*/ uint32 spell;       // Spell ID; client reads lower 16 bits; 0x407 triggers autobook-scribe
			/*008*/ uint32 target;      // Target Entity ID
			/*012*/ int32  unknown0;    // Exit gate: handler skips if < 0; send 0
			/*016*/ int32  recast_time; // fasttime() timestamp for autobook-scribe path (spell==0x407); send 0
			/*020*/
		};

		struct MerchantClickRequest_Struct
		{
			/*000*/ uint32 npc_id;      // Merchant NPC's entity id
			/*004*/
		};

		struct MerchantClickResponse_Struct
		{
			/*000*/ uint32 npc_id;      // Merchant NPC's entity id
			/*004*/ uint32 player_id;
			/*008*/ float rate;
			/*012*/ uint32 tab_display; // bitmask b000 none, b001 Purchase/Sell, b010 Recover, b100 Parcels
			/*016*/ uint32 ldon_category; // ldon cat for ldon merchants
			/*020*/ uint32 alt_currency1; //These two usually match but I imagine they could be different?
			/*024*/ uint32 alt_currency2;
			/*028*/ uint32 unknown028; // Observed 256
			/*032*/
		};

		struct BeginCast_Struct
		{
			/*000*/	int32 spell_id;
			/*004*/	uint16 caster_id;
			/*006*/	uint32 cast_time; // in miliseconds
			/*010*/	uint32 unknown0a; // I think this is caster effective level but im not sure. live always sends 0. The client uses this for the spell link
			/*014*/	uint8 unknown0e; // 0 will short circuit the cast, seen 1 from live usually, maybe related to interrupts or particles or something
			/*015*/
		};

		// Used for OP_ReadBook in both directions (S→C text display, C→S content request).
		// IDA-confirmed layout: type+target_id precede invslot (differs from RoF2 ordering).
		struct BookRequest_Struct
		{
			/*0000*/ uint32 window;   // 0xFFFFFFFF = new window; maps from emu->window (0xFF)
			/*0004*/ uint32 type;     // 0=note/scroll, 1=book, 2=item info
			/*0008*/ uint32 target_id;
			/*0012*/ TypelessInventorySlot_Struct invslot;
			/*0018*/ uint8  padding;
			/*0019*/ uint8  can_cast;
			/*0020*/ uint8  can_scribe; // book-path cast button; note-path scribe button
			/*0021*/ char   txtfile[8194]; // null-terminated text / book file name
			/*8215*/
		};

		struct BookButton_Struct
		{
			/*000*/ TypelessInventorySlot_Struct slot; // book ItemIndex
			/*006*/	int16 unknown2;   // zero padding
			/*008*/	int32 target_id;  // spawn_id of target player or 0
			/*012*/	int32 unknown3;   // zero padding
			/*016*/
		};

		struct MemorizeSpell_Struct {
			uint32 slot;		// Spot in the spell book/memorized slot
			int32  spell_id;	// Spell id (200 or c8 is minor healing, etc)
			uint32 scribing;	// -1 refreshes book, 0 scribe to book, 2 end mem, 1 start mem, 3 unmem, 4 set activated item keyring -- client will send back 2 if a 0 operation updated a memorized spell of the same group + subgroup
			uint32 reduction;	// lower reuse (only used if scribing is 4)
		};

		struct LinkedSpellReuseTimer_Struct {
			uint32 timer_id;    // +0x00 linked spell group index (0–24)
			uint32 unknown;     // +0x04 extra DWORD present in TOB/TDS+ clients
			uint32 end_time;    // +0x08 absolute time when spell group is ready
			uint32 start_time;  // +0x0C server send timestamp (for client latency correction)
		};

		//I've observed 5 s16 that are all -1.
		//Clicky items don't even trigger this as far as i can tell so not sure what this is for now.
		//One of these could have changed to a s32 but im not sure.
		struct CastSpellInventorySlot_Struct {
			/*00*/ int16 type;
			/*02*/ int16 slot;
			/*04*/ int16 sub_index;
			/*06*/ int16 aug_index;
			/*08*/ int16 unknown1;
			/*10*/
		};

		struct CastSpell_Struct
		{
			/*00*/	uint32	slot;
			/*04*/	int32   spell_id;
			/*08*/	CastSpellInventorySlot_Struct inventory_slot; 
			/*18*/	uint32	target_id;
			/*22*/	uint32	spell_crc; 
			/*26*/  float y_pos;
			/*30*/  float x_pos;
			/*34*/  float z_pos;
			/*38*/	uint8 unknown; //not sure, might also be before y_pos; only ever seen zero for both but should be easy to figure out later
			/*39*/
		};

		struct Charm_Struct
		{
			/*00*/ uint32 owner_id;
			/*04*/ uint32 pet_id;
			/*08*/ uint32 charmer_id; // TOB-only field not present in server Charm_Struct; purpose unknown (passed to sub_1402FA570 when non-null); set to 0
			/*0C*/ uint8  command;    // 1=make pet, 0=release pet; server sends this as uint32 at +0x08
			/*0D*/
		};

		struct InterruptCast_Struct
		{
			uint32 spawnid;
			uint32 messageid;
			char	message[0];
		};

		struct EQAffectSlot_Struct {
			/*00*/ int32 slot;
			/*04*/ int32 padding;
			/*08*/ int64 value;
			/*16*/
		};

		struct EQAffect_Struct
		{
			/*000*/ EQAffectSlot_Struct slots[6];
			/*096*/ EqGuid caster_id;
			/*104*/ uint32 flags;
			/*108*/ int32  spell_id;
			/*112*/ uint32 duration;
			/*116*/ uint32 initial_duration;
			/*120*/ uint32 hit_count;
			/*124*/ uint32 viral_timer;
			/*128*/ float modifier;
			/*132*/ float y;
			/*136*/ float x;
			/*140*/ float z;
			/*144*/ uint8 level; // caster level used by the spell formulas
			/*145*/ uint8 type;  // non-zero = effects are subtracted (reversed); normal buffs send 0
			/*146*/ uint8 charges; //no idea if these are right; eqlib doesn't seem to know either
			/*147*/ uint8 activatable;
			/*148*/ uint32 unknown1; //might be some timer, not sure though
			/*152*/
		};

		struct EQAffectPacket_Struct {
			/*000*/ uint32 entity_id;
			/*004*/ int32 unknown004;
			/*008*/ EQAffect_Struct affect;
			/*160*/ uint32 slot_id;
			/*164*/ uint32 buff_fade; // 1: remove, 2: modify, 3: add new
			/*168*/
		};

		struct ManaChange_Struct
		{
			uint32 new_mana;
			uint32 stamina; // endurance
			int32  spell_id;
			uint32 keepcasting;
			int32 slot; // gem slot
		};

		//This is what we call OP_Action
		//To the client though this is basically a missile hit though
		//OP_Action is basically "instant missile hit" to the client
		// MissileHitInfo::Deserialize(CUnSerializeBuffer *buffer);
		struct MissileHitInfo
		{
			uint16 target;
			uint16 source;
			int32  spell_id;
			//4 leaves a buff
			uint32 effect_type; 
			uint32 effective_casting_level;
			//Client does read this but only does something if it's negative
			int64 unknown1;
			//I don't see the client read this one outside basic serialization
			int64 unknown2;
			//I don't see the client read this one either but based on captures from live it seems to match spell damage
			int64 damage;
			float modifier;
			float force;
			float hit_heading;
			float hit_pitch;
			//same convention as damage
			//231 for spell, otherwise it's skill in use
			uint8 skill; 
			uint8 level; //the client doesn't actually deserialize anything past level
			//live however has a lot more info here depending on packet type
		};

		struct MobHealth_Struct
		{
			/*01*/ int16 spawn_id;
			/*00*/ uint32 hp;
		};

		struct GMTrainee_Struct
		{
			/*000*/ uint32 npcid;
			/*004*/ uint32 playerid;
			/*008*/ uint32 skills[PACKET_SKILL_ARRAY_SIZE];
			/*408*/ uint8 unknown408[36];
			/*444*/
		};

		struct GMTrainSkillConfirm_Struct {
			/*000*/	uint32	SkillID;
			/*004*/	uint32	Cost;
			/*008*/	uint8	NewSkill;	// Set to 1 for 'You have learned the basics' message.
			/*009*/	char	TrainerName[64];
			/*073*/ uint8	Unknown073[3];
			/*076*/
		};

		struct SkillUpdate_Struct {
			/*00*/	uint32 skillId;
			/*04*/	uint32 value;
			/*08*/	uint8 active;
			/*09*/	uint8 padding[3];
			/*12*/
		};

		struct AA_Array
		{
			uint32 AA;
			uint32 value; // points spent
			uint32 charges;	// expendable charges
		};

		struct AATable_Struct {
			/*00*/ uint32 aa_spent;				// Total AAs Spent
			/*04*/ uint32 aapoints_assigned[6];	// none, general, arch, class, special, focus, merc
			/*24*/ AA_Array aa_list[MAX_PP_AA_ARRAY];
		};

		struct AltAdvStats_Struct {
			/*000*/	uint32 experience;
			/*004*/	uint32 unspent;
			/*008*/	uint8 percentage;
			/*009*/	uint8 padding[3];
		};

		struct ZonePlayerToBind_Struct {
			//Same structure as the binds in PlayerProfile_Struct
			//Assembly calls the same function
			/*000*/	uint16	bind_zone_id;
			/*002*/	uint16	bind_instance_id;
			/*004*/	float	x;
			/*008*/	float	y;
			/*012*/	float	z;
			/*016*/	float	heading;
			/*020*/	char	zone_name[1];  // Or "Bind Location"
			/*021*/	uint32	unknown1;
			/*025*/	uint32	unknown2;
			/*029*/	uint32	unknown3;
		};

		struct ArmorPropertyStruct
		{
			/*000*/ uint32 type;
			/*004*/ uint32 variation;
			/*008*/ uint32 material;
			/*012*/ uint32 newArmorID;
			/*016*/ uint32 newArmorType;
			/*020*/
		};

		struct Illusion_Struct {
			/*000*/ uint32 spawnid;
			/*004*/ char charname[64];
			/*068*/ int32 race;
			/*072*/ int32 class_;
			/*076*/ float size;
			/*080*/ uint32 face;
			/*084*/ uint32 npc_tint;
			/*088*/ uint32 keep_armor_properties;
			/*092*/ uint32 drakkin_heritage;
			/*096*/ uint32 drakkin_tattoo;
			/*100*/ uint32 drakkin_details;
			/*104*/ uint8 gender;
			/*105*/ uint8 texture;
			/*106*/ uint8 helmtexture;
			/*107*/ uint8 armorVariation;
			/*108*/ uint8 armorMaterial;
			/*109*/ uint8 hairstyle;
			/*110*/ uint8 haircolor;
			/*111*/ uint8 beard;
			/*112*/ uint8 beardcolor;
			/*113*/ uint8 padding[3];
			/*116*/ ArmorPropertyStruct armorProperties[9];
			/*296*/ uint32 armorTints[9];
			/*332*/
		};

		struct IncreaseStat_Struct {
			/*000*/ uint32 spawn_id;   // must equal g_pLocalPlayer->SpawnID; not in server struct (see encoder TODO)
			/*004*/ uint32 stat_type;  // 0=STR 1=STA 2=AGI 3=DEX 4=INT 5=WIS 6=CHA; client ignores > 6
			/*008*/ uint32 value;      // must be > 0 to be applied
			/*012*/
		};

		struct moneyOnCorpseStruct {
			/*000*/ uint8 type; // 0 = someone is already looting, 1 = OK, 2 = cannot access at this time, 3 = OK, 4 = cannot loot while hostile nearby, 5 = too far away to loot, 6 = loot all, 7 = cancel loot, 8 = add access, 9 = using advloot (when right clicking), 10 = show advloot
			/*001*/ uint8 padding1[3];
			/*004*/ uint32 flags; // 1 = extra lucky bonus, 2 = lucky bonus, 4 = bonus
			/*008*/ uint32 platinum;
			/*012*/ uint32 gold;
			/*016*/ uint32 silver;
			/*020*/ uint32 copper;
			/*024*/
		};

		struct LootingItem_Struct {
			/*000*/	uint32	lootee;
			/*004*/	uint32	looter;
			/*008*/	uint32	slot_id;
			/*012*/	int32	auto_loot;
			/*016*/	uint32	unknown16;
			/*020*/
		};

		// OP_RezzRequest / OP_RezzComplete: an 8-byte field at 88 (shifts everything after it by 4)
		// and 24 extra bytes at the end; the client copies 0x100 bytes.
		struct Resurrect_Struct {
			/*000*/ uint32 unknown000;
			/*004*/ uint16 zone_id;
			/*006*/ uint16 instance_id;
			/*008*/ float y;
			/*012*/ float x;
			/*016*/ float z;
			/*020*/ uint32 unknown020;
			/*024*/ char your_name[64];
			/*088*/ uint64 unknown088;
			/*096*/ char rezzer_name[64];
			/*160*/ int32 spellid;
			/*164*/ char corpse_name[64];
			/*228*/ uint32 action;
			/*232*/ uint8 unknown232[24];
			/*256*/
		};

		// OP_ManaUpdate / OP_EnduranceUpdate
		struct ManaUpdate_Struct
		{
			/*00*/ uint16 spawn_id;
			/*02*/ uint32 cur_mana;
			/*06*/ uint32 max_mana;
			/*10*/
		};

		struct EnduranceUpdate_Struct
		{
			/*00*/ uint16 spawn_id;
			/*02*/ uint32 cur_end;
			/*06*/ uint32 max_end;
			/*10*/
		};

		// OP_MobManaUpdate / OP_MobEnduranceUpdate: percent is int32 now
		struct MobManaUpdate_Struct
		{
			/*00*/ int16  spawn_id;
			/*02*/ int32  mana;      // percent 0..100
			/*06*/
		};

		struct MobEnduranceUpdate_Struct
		{
			/*00*/ int16  spawn_id;
			/*02*/ int32  endurance; // percent 0..100
			/*06*/
		};

		struct ClickObject_Struct {
			/*00*/ uint32 drop_id;
			/*04*/ uint32 player_id;
			/*08*/ uint32 unknown08;
		};

		struct PetCommand_Struct {
			/*00*/ uint32 command;
			/*04*/ uint32 target;
			/*08*/ uint32 unknown08;
		};

		struct GroupFollow_Struct {
			/*000*/ char name1[64]; // inviter
			/*064*/ char name2[64]; // invitee
			/*128*/ uint8 unknown128[48];
			/*176*/
		};

		// C2S 0x7b49 /summon: GMSummon_Struct written field by field, names as u32 length (64) + 64 bytes
		struct GMSummon_Struct {
			/*000*/ uint32 charname_len;
			/*004*/ char   charname[64];
			/*068*/ uint32 gmname_len;
			/*072*/ char   gmname[64];
			/*136*/ uint8  success;
			/*137*/ uint32 zoneID;
			/*141*/ float  y;
			/*145*/ float  x;
			/*149*/ float  z;
			/*153*/ uint32 unknown2;
			/*157*/
		};

		// 0x354a both directions. toggle: 1 considering another group, 2 already grouped, 3 declined, 4/5 raid
		struct GroupCancel_Struct {
			/*000*/ char   name1[64]; // inviter
			/*064*/ char   name2[64]; // invitee
			/*128*/ uint8  unknown128[32];
			/*160*/ uint16 unknown160; // client sends 0x0100
			/*162*/ uint8  unknown162[6];
			/*168*/ uint8  toggle;
			/*169*/ uint8  unknown169[7];
			/*176*/
		};

// -----------------------------------------------------------------------------------------------------
		// S2C 0x13ff
		struct GuildMemberAdd_Struct {
		/*000*/ uint64 guild_id;
		/*008*/ uint32 level;
		/*012*/ uint32 class_;
		/*016*/ uint32 rank;
		/*020*/ uint8  guild_show;        // read as a byte
		/*021*/ uint8  pad021[3];
		/*024*/ uint32 zone_id;           // non-zero = online
		/*028*/ uint32 last_on;
		/*032*/ char   player_name[64];
		/*096*/
		};

		// S2C 0x70cf
		struct GuildMemberLevel_Struct {
		/*000*/ uint64 guild_id;
		/*008*/ char   player_name[64];
		/*072*/ uint32 level;
		/*076*/ uint32 class_;            // client overwrites member class with this
		/*080*/
		};

		// S2C 0x3b2f (SetGuildRank / GuildMemberRankAltBanker)
		struct GuildMemberRank_Struct {
		/*000*/ uint64 guild_id;
		/*008*/ uint32 rank;
		/*012*/ char   player_name[64];
		/*076*/ uint32 alt_banker;        // bit1 = alt
		/*080*/ uint8  guild_show;        // seen 1
		/*081*/ uint8  self_perm_flag; // only used when player_name == me
		/*082*/ uint8  pad082[6];
		/*088*/
		};

		// S2C 0x4ea8
		struct GuildMemberPublicNote_Struct {
		/*000*/ uint64 guild_id;
		/*008*/ char   player_name[64];
		/*072*/ char   public_note[256];
		/*328*/
		};

		// S2C 0x5460
		struct GuildRenameMember_Struct {
		/*000*/ uint64 guild_id;
		/*008*/ char   player_name[64];
		/*072*/ char   new_player_name[64];
		/*136*/
		};

		// S2C 0x614e
		struct GuildMemberDelete_Struct {
		/*000*/ uint64 guild_id;
		/*008*/ char   player_name[64];
		/*072*/
		};

		// S2C 0x5b63 (GuildMemberDetails / GuildMemberUpdate)
		struct GuildMemberDetails_Struct {
		/*000*/ uint64 guild_id;
		/*008*/ char   player_name[64];
		/*072*/ uint32 zone_id;
		/*076*/ uint32 last_on;
		/*080*/ uint32 offline_mode;      // 1 = offline
		/*084*/ uint32 pad084;
		/*088*/
		};

		// S2C 0x408a
		struct GuildRenameGuild_Struct {
		/*000*/ uint64 guild_id;
		/*008*/ char   new_guild_name[64];
		/*072*/
		};

		// S2C 0x73dc
		struct GuildDelete_Struct {
		/*000*/ uint64 guild_id;
		/*008*/
		};

		// S2C 0x1dab: a spawn joined / left a guild (ToB does not take this from SpawnAppearance)
		struct GuildMemberGuildID_Struct {
		/*000*/ uint32 spawn_id;
		/*004*/ uint64 guild_id;
		/*012*/
		};
		static_assert(sizeof(GuildMemberGuildID_Struct) == 12, "TOB GuildMemberGuildID_Struct");

		// S2C 0x7c20 / 0x45b8, C2S 0x6e0d / 0x34e3
		struct GuildMOTD_Struct {
		/*000*/ uint64 guild_id;
		/*008*/ char   name[64];
		/*072*/ char   setby_name[64];
		/*136*/ uint32 unknown136;
		/*140*/ char   motd[512];
		/*652*/ uint32 pad652;
		/*656*/
		};

		// C2S/S2C 0x26db
		struct GuildInvite_Struct {
		/*000*/ char   othername[64];     // invitee (S2C: compared with my name)
		/*064*/ char   myname[64];        // inviter
		/*128*/ char   guild_name[64];    // only used if use_guild_name != 0
		/*192*/ uint64 unknown192;        // filled by the client
		/*200*/ uint64 guild_id;
		/*208*/ uint32 my_rank;
		/*212*/ uint32 rank;              // S2C: must be 1..8 or the client ignores the invite
		/*216*/ uint8  use_guild_name;
		/*217*/ uint8  pad217[7];
		/*224*/
		};

		// C2S/S2C 0x1c99
		struct GuildInviteAccept_Struct {
		/*000*/ char   inviter[64];
		/*064*/ char   new_member[64];
		/*128*/ uint32 response;          // accept: rank, decline: 9 (client tests >= 9)
		/*132*/ uint32 pad132;
		/*136*/ uint64 guild_id;
		/*144*/ uint32 action;            // 1 accept, 2 decline
		/*148*/ uint32 pad148;
		/*152*/
		};

		// C2S 0x3ab0
		struct GuildRemove_Struct {
		/*000*/ char   othername[64];
		/*064*/ char   myname[64];
		/*128*/ uint64 guild_id;
		/*136*/ uint32 officer; // sender's rank
		/*140*/ uint32 unknown140;
		/*144*/
		};

		// C2S 0x118c (ToB leaves 'name' empty)
		struct GuildDemote_Struct {
		/*000*/ char   name[64];
		/*064*/ char   target[64];
		/*128*/ uint32 rank;
		/*132*/
		};

		// C2S 0x1ea1
		struct GuildPromote_Struct {
		/*000*/ char   target[64];
		/*064*/ char   name[64];
		/*128*/ uint64 guild_id;
		/*136*/ uint32 rank;
		/*140*/ uint32 myrank;            // uninitialised stack in the client: ignore
		/*144*/
		};

		// C2S 0x6108
		struct GuildManageBanker_Struct {
		/*000*/ uint64 unknown0;
		/*008*/ char   myname[64];
		/*072*/ char   member[64];
		/*136*/ uint32 enabled;           // flags, bit1 = alt
		/*140*/ uint32 pad140;
		/*144*/
		};

		// C2S 0x2817
		struct GuildPublicNote_Struct {
		/*000*/ uint64 guild_id;
		/*008*/ char   name[64];
		/*072*/ char   target[64];
		/*136*/ char   note[256];
		/*392*/
		};

		// C2S 0x1cfd
		struct GuildStatus_Struct {
		/*000*/ char   Name[64];
		/*064*/ uint8  Unknown064[80];
		/*144*/
		};

		// C2S 0x6079
		struct GuildOpenGuildWindow_Struct {
		/*000*/ uint64 guild_id;
		/*008*/ uint32 unknown8;
		/*012*/
		};

		// 0x1ae6 header (payload follows at 0x58)
		struct GuildUpdateHeader_Struct {
		/*000*/ uint32 action;
		/*004*/ uint32 unknown4;
		/*008*/ uint32 unknown8;
		/*012*/ uint32 unknown12;         // new in ToB
		/*016*/ char   name[64];
		/*080*/ uint64 guild_id;
		/*088*/
		};


		// S2C OP_SetTitleReply 0x5051: is_suffix@0, title@4 (128 bytes in ToB), entity_id@0x84
		struct SetTitleReply_Struct {
			/*000*/ uint32 is_suffix;
			/*004*/ char   title[128];
			/*132*/ uint32 entity_id;
			/*136*/
		};

		// OP_InspectRequest 0x5564: writes {target spawn id, own spawn id} as u32s
		struct Inspect_Struct {
			/*00*/ uint32 TargetID;
			/*04*/ uint32 PlayerID;
			/*08*/
		};

		// OP_GMLastName, C2S, 195 bytes
		struct GMLastName_Struct {
			/*000*/ char  name[64];
			/*040*/ char  gmname[64];
			/*080*/ char  lastname[64];
			/*0c0*/ uint8 unknown192[3];
			/*0c3*/
		};

		// OP_RaidDelegateAbility 0x8ea0, both directions: emu DelegateAbility_Struct behind an 8-byte prefix.
		// S2C reader reads type@8 slot@0xc action@0x10 entity@0x1c name@0x24 from the raw buffer.
		// C2S writer: u64 (always 0), 7 x u32, then a NUL-terminated name (variable length).
		struct DelegateAbility_Struct {
		/*000*/	uint64 unknown000;       // 0
		/*008*/	uint32 DelegateAbility;  // ToB enum: 0 = new (eqstr 0x2c27/0x2c28), 1 = main assist, 2 = main marker
		/*00c*/	uint32 MemberNumber;
		/*010*/	uint32 Action;           // 1 = clear, else set (same as emu ClearDelegate/SetDelegate)
		/*014*/	uint32 Unknown012;
		/*018*/	uint32 Unknown016;
		/*01c*/	uint32 EntityID;
		/*020*/	uint32 Unknown024;
		/*024*/	char   Name[64];
		/*064*/
		};

		// OP_TributeItem (both ways)
		struct TributeItem_Struct {
		/*00*/	InventorySlot_Struct	inventory_slot;
		/*12*/	uint32	quantity;
		/*16*/	uint32	tribute_master_id;
		/*20*/	int32	tribute_points;
		/*24*/
		};

		// OP_MercenaryTimer (S2C 0x6d3f). SuspendedTime widened to 64 bits.
		struct MercenaryStatus_Struct {
		/*00*/	uint32	MercEntityID;
		/*04*/	uint32	UpdateInterval;
		/*08*/	uint32	MercUnk01;
		/*12*/	uint32	MercState;		// ToB enum: server 5 ("normal") == ToB 6
		/*16*/	uint64	SuspendedTime;
		/*24*/
		};

		// OP_MercenarySuspendResponse (S2C 0x1e9b). Client reads *(u64*)p.
		struct SuspendMercenaryResponse_Struct {
		/*00*/	uint64	SuspendTime;
		/*08*/
		};

		// OP_GuildTributeDonatePlat (C2S 0x56f6)
		struct GuildTributeDonatePlatRequest_Struct {
		/*00*/	uint32	quantity;
		/*04*/	uint32	tribute_master_id;
		/*08*/	uint32	unknown08;
		/*12*/	uint32	unknown12;
		/*16*/	uint64	guild_id;
		/*24*/	uint32	unknown24;
		/*28*/	uint32	unknown28;
		/*32*/
		};

		// OP_GuildTributeDonateItem (C2S 0x647c)
		struct GuildTributeDonateItemRequest_Struct {
		/*00*/	InventorySlot_Struct	slot;
		/*12*/	uint32	quantity;
		/*16*/	uint32	tribute_master_id;
		/*20*/	uint32	unknown20;
		/*24*/	uint64	guild_id;
		/*32*/	uint64	unknown32;
		/*40*/
		};

		// OP_GuildTributeToggleReq (C2S 0x5c3b)
		struct GuildTributeToggleReq_Struct {
		/*00*/	uint32	command;
		/*04*/	uint32	unknown04;
		/*08*/
		};

		// OP_ShopSendParcel C2S: ToB's 6-byte slot + 2 pad (same pattern as AltCurrencySell)
		struct Parcel_Struct
		{
		/*000*/ uint32 npc_id;
		/*004*/ TypelessInventorySlot_Struct inventory_slot;
		/*010*/ uint16 unknown010;
		/*012*/ uint32 quantity;
		/*016*/ uint32 money_flag;
		/*020*/ char   send_to[64];
		/*084*/ char   note[128];
		/*212*/ uint32 unknown_212;
		/*216*/ uint32 unknown_216;
		/*220*/ uint32 unknown_220;
		/*224*/
		};

		// OP_AltCurrencySell 0x58ca, both directions
		struct AltCurrencySellItem_Struct
		{
		/*000*/ uint32 merchant_entity_id;
		/*004*/ TypelessInventorySlot_Struct inventory_slot;
		/*010*/ uint16 unknown010;
		/*012*/ uint32 charges;
		/*016*/ uint32 cost;
		/*020*/
		};
		static_assert(sizeof(AltCurrencySellItem_Struct) == 20, "TOB AltCurrencySellItem_Struct must be 20 bytes");

		// OP_AltCurrencySellSelection 0x3181 C2S; the S2C reply is the emu 88-byte struct
		struct AltCurrencySelectItem_Struct
		{
		/*000*/ uint32 merchant_entity_id;
		/*004*/ TypelessInventorySlot_Struct inventory_slot;
		/*010*/ char   unknown010[64];
		/*074*/ uint8  unknown074[6];
		/*080*/
		};
		static_assert(sizeof(AltCurrencySelectItem_Struct) == 80, "TOB AltCurrencySelectItem_Struct must be 80 bytes");

		// OP_GroupRoles 0x5205, both directions, 168 bytes
		struct GroupRole_Struct
		{
		/*000*/ char   Name1[64];        // member whose role changes
		/*064*/ char   Name2[64];        // group leader
		/*128*/ uint8  unknown128[8];
		/*136*/ uint32 unknown136;
		/*140*/ uint32 RoleNumber;       // 1 main tank, 2 main assist, 3 puller
		/*144*/ uint8  unknown144[16];
		/*160*/ uint8  Toggle;
		/*161*/ uint8  unknown161;       // 0 on S2C
		/*162*/ uint8  unknown162[6];
		/*168*/
		};
		static_assert(sizeof(GroupRole_Struct) == 168, "TOB GroupRole_Struct must be 168 bytes");

		// OP_PetitionUpdate (S2C 0x71fe) (GM petition queue): senttime is 32-bit on the wire
		struct PetitionUpdate_Struct {
		/*000*/	uint32	petnumber;
		/*004*/	uint32	color;      // 0 green, 1 yellow, 2 red
		/*008*/	uint32	status;     // 0xFFFFFFFF / 0xFFFFFFFD removes the entry
		/*012*/	uint32	senttime;
		/*016*/	char	accountid[32];
		/*048*/	char	gmsenttoo[64];
		/*112*/	int32	quetotal;
		/*116*/	char	charname[64];
		/*180*/	uint32	unknown180; // 0 -> client uses senttime
		/*184*/
		};

		// OP_GuildTributeFavorAndTimer (S2C 0x009e) reads favor/timers at +8/+12/+16
		struct GuildTributeFavorTimer_Struct {
		/*00*/	uint64	guild_id;          // not read by the client
		/*08*/	uint32	guild_favor;
		/*12*/	uint32	tribute_timer;
		/*16*/	uint32	trophy_timer;
		/*20*/
		};

		// OP_GuildSendActiveTributes (S2C 0x37cc) and OP_GuildTributeToggleReply (S2C 0x285a)
		struct GuildTributeSendActive_Struct {
		/*00*/	uint32	not_used;
		/*04*/	uint32	guild_favor;
		/*08*/	uint32	tribute_timer;
		/*12*/	uint32	tribute_enabled;   // client reads the low byte
		/*16*/	uint8	unknown16[12];     // server: 8
		/*28*/	uint32	tribute_id_1;
		/*32*/	uint32	tribute_id_2;
		/*36*/	uint32	tribute_id_1_tier;
		/*40*/	uint32	tribute_id_2_tier;
		/*44*/
		};

		// OP_GuildOptInOut (S2C 0x4413): member row update
		struct GuildTributeOptInOutReply_Struct {
		/*00*/	uint64	guild_id;
		/*08*/	char	player_name[64];
		/*72*/	uint32	tribute_toggle;
		/*76*/	uint32	tribute_trophy_toggle;
		/*80*/	uint32	no_donations;
		/*84*/	uint32	time;
		/*88*/	uint32	command;           // client reads the low byte
		/*92*/
		};

		// OP_GuildOptInOut (C2S 0x4413) (opt-in checkbox)
		struct GuildTributeOptInOutReq_Struct {
		/*00*/	uint64	guild_id;
		/*08*/	uint32	tribute_toggle;
		/*12*/	char	player[64];
		/*76*/	uint32	command;           // always 0
		/*80*/	uint32	tribute_master_id;
		/*84*/	uint32	unknown84;         // uninitialised
		/*88*/
		};

		// OP_GuildModifyBenefits (C2S 0x51e6), Modify Benefits button / window close. Only byte 0 and the
		// master id are initialised; the benefit choices go in OP_GuildSaveActiveTributes just before it.
		struct GuildTributeModifyBenefits_Struct {
		/*00*/	uint8	command;           // 1 = enter modify mode, 0 = save
		/*01*/	uint8	unknown01[79];
		/*80*/	uint32	tribute_master_id;
		/*84*/	uint32	unknown84;
		/*88*/
		};

		// OP_GuildSaveActiveTributes (C2S 0x7161).
		struct GuildTributeSaveActive_Struct {
		/*00*/	uint32	command;
		/*04*/	uint8	unknown04[20];
		/*24*/	uint32	master_tribute_id;
		/*28*/	uint32	tribute_id_1;
		/*32*/	uint32	tribute_id_2;
		/*36*/	uint32	tribute_1_tier;
		/*40*/	uint32	tribute_2_tier;
		/*44*/	uint8	unknown44[12];
		/*56*/
		};

		struct DynamicZoneInfo_Struct
		{
		/*000*/ uint32 client_id;
		/*004*/ uint32 unknown004;
		/*008*/ uint8  assigned;          // client reads one byte; 0 clears the window
		/*009*/ uint8  padding009[3];
		/*012*/ uint32 max_players;
		/*016*/ uint32 unknown016;        // new in ToB, purpose unknown
		/*020*/ char   dz_name[128];
		/*148*/ char   leader_name[64];
		/*212*/ uint8  dz_type;           // 1 = expedition; window auto-opens on first assignment only when 1
		/*213*/ uint8  padding213[3];
		/*216*/
		};

		struct DynamicZoneMemberListName_Struct
		{
		/*000*/ uint32 client_id;
		/*004*/ uint32 unknown004;
		/*008*/ uint8  add_name;          // 1 add, 0 remove
		/*009*/ uint8  padding009[3];
		/*012*/ uint32 unknown012;        // new in ToB, not read
		/*016*/ char   name[64];
		/*080*/
		};

		struct ExpeditionExpireWarning
		{
		/*000*/ uint32 client_id;
		/*004*/ uint32 unknown004;
		/*008*/ uint32 minutes_remaining;
		/*012*/
		};

		struct LoadSpellSet_Struct
		{
		/*000*/ uint8  spell[16];         // client fills min(count,14); bytes past count are uninitialised
		/*016*/ uint32 count;
		/*020*/
		};



		// OP_AltCurrency action 8 entry: 29 bytes
		struct AltCurrencyPopulateEntry_Struct
		{
		/*000*/ uint32 currency_number;
		/*004*/ uint32 unknown00;          // always 1
		/*008*/ uint32 currency_number2;
		/*012*/ uint32 item_id;
		/*016*/ uint32 item_icon;
		/*020*/ uint32 stack_size;
		/*024*/ uint32 sort_order; // NEW in ToB; used as a sort key. Position inferred.
		/*028*/ uint8  display;
		/*029*/
		};

		struct AltCurrencyPopulate_Struct
		{
		/*000*/ uint32 opcode;             // 8
		/*004*/ uint32 count;
		/*008*/ AltCurrencyPopulateEntry_Struct entries[0];
		};

		// OP_AltCurrency action 7. The client reads the fields without a size check.
		struct AltCurrencyUpdate_Struct
		{
		/*000*/ uint32 opcode;             // 7
		/*004*/ char   name[64];
		/*068*/ uint32 currency_number;
		/*072*/ uint32 unknown072;         // 1
		/*076*/ uint32 delta;              // only printed when show_message != 0 (eqstr 0x3426)
		/*080*/ uint32 amount;             // new total
		/*084*/ uint32 unknown084;
		/*088*/ uint8  show_message;
		/*089*/ uint8  unknown089[3];
		/*092*/
		};

		// OP_GroupMakeLeader C2S, 136 bytes
		struct GroupMakeLeader_Struct
		{
		/*000*/ uint32 unknown000;         // client writes 8
		/*004*/ char   current_leader[64];
		/*068*/ char   new_leader[64];
		/*132*/ uint32 unknown132;
		/*136*/
		};

		// OP_AdventureMerchantSell, 24 bytes
		struct Adventure_Sell_Struct
		{
		/*000*/ uint32 merchant_type;      // 1 LDoN, 2 Discord, 4 NK, 5 DR (the server sends 1)
		/*004*/ uint32 npcid;
		/*008*/ TypelessInventorySlot_Struct inventory_slot;
		/*014*/ uint16 unknown014;            // pad: the 6-byte ToB slot left the struct at 22 bytes
		/*016*/ uint32 charges;
		/*020*/ uint32 sell_price;
		/*024*/
		};

		// OP_GuildBank.
		struct GuildBankItemUpdate_Struct
		{
		/*000*/ uint32 action;
		/*004*/ uint32 not_used004;
		/*008*/ uint32 not_used008;
		/*012*/ uint16 slot_id;
		/*014*/ uint16 area;
		/*016*/ uint32 display;
		/*020*/ uint32 item_id;
		/*024*/ uint32 icon_id;
		/*028*/ uint32 quantity;
		/*032*/ uint32 permissions;
		/*036*/ uint8  allow_merge;
		/*037*/ uint8  is_useable;
		/*038*/ char   item_name[64];
		/*102*/ char   donator[64];
		/*166*/ char   who_for[64];
		/*230*/
		};
		struct GuildBankPromote_Struct      { uint32 Action; uint32 Unknown04; uint32 Unknown08; uint32 Slot; uint32 Slot2; uint32 unknown20; };
		struct GuildBankPermissions_Struct  { uint32 Action; uint32 Unknown04; uint32 Unknown08; uint16 SlotID; uint16 Unknown10; uint32 ItemID; uint32 Permissions; char MemberName[64]; };
		struct GuildBankViewItem_Struct     { uint32 Action; uint32 Unknown04; uint32 Unknown08; uint16 SlotID; uint16 Area; uint32 Unknown12; uint32 Unknown16; };
		struct GuildBankWithdrawItem_Struct { uint32 Action; uint32 Unknown04; uint32 Unknown08; uint16 SlotID; uint16 Area; uint32 Unknown12; uint32 Quantity; };
		// NOTE: the ToB client sends action 3 with a 24-byte body and action 5 with an 88-byte body.



		// OP_LFGuild. Every ToB body starts at 0x18. C2S bytes 4..0x18 are uninitialised stack.
		struct LFGuild_PlayerToggle_Struct // command 0, both directions
		{
		/*000*/ uint32 Command;                     // 0
		/*004*/ uint8  Unknown004[20];
		/*018*/ uint8  Unknown018[64];              // not used by the window
		/*058*/ char   Comment[524];                // server: Comment[256] + Unknown328[268]
		/*264*/ int32  TimeZone;
		/*268*/ uint8  Toggle;
		/*269*/ uint8  Unknown269[7];
		/*270*/ uint64 TimePosted;                  // u64 in ToB
		/*278*/ uint8  Unknown278[8];
		/*280*/
		};
		static_assert(sizeof(LFGuild_PlayerToggle_Struct) == 0x280, "TOB LFGuild player toggle");

		struct LFGuild_GuildToggle_Struct // command 1, both directions
		{
		/*000*/ uint32 Command;                     // 1
		/*004*/ uint8  Unknown004[28];              // 20 header bytes + 8 unused
		/*020*/ char   Comment[512];
		/*220*/ uint32 FromLevel;
		/*224*/ uint32 ToLevel;
		/*228*/ uint32 Classes;                     // bits 1..16
		/*22c*/ uint32 AACount;
		/*230*/ int32  TimeZone;
		/*234*/ uint8  Toggle;
		/*235*/ uint8  Unknown235[3];
		/*238*/ uint64 TimePosted;                  // u64 in ToB
		/*240*/ char   Name[64];                    // not read by the client
		/*280*/
		};
		static_assert(sizeof(LFGuild_GuildToggle_Struct) == 0x280, "TOB LFGuild guild toggle");

		struct LFGuild_SearchPlayer_Struct          // command 3, C2S, 48 bytes
		{
		/*00*/ uint32 Command;                      // 3
		/*04*/ uint8  Unknown004[20];
		/*18*/ uint32 FromLevel;
		/*1c*/ uint32 ToLevel;
		/*20*/ uint32 MinAA;
		/*24*/ int32  TimeZone;
		/*28*/ uint32 Classes;
		/*2c*/ uint32 Unknown02c;                   // uninitialised
		/*30*/
		};
		static_assert(sizeof(LFGuild_SearchPlayer_Struct) == 0x30, "TOB LFGuild search player");

		struct LFGuild_SearchGuild_Struct           // command 4, C2S, 40 bytes
		{
		/*00*/ uint32 Command;                      // 4
		/*04*/ uint8  Unknown004[20];
		/*18*/ uint32 Level;
		/*1c*/ uint32 AAPoints;
		/*20*/ int32  TimeZone;
		/*24*/ uint32 Class;
		/*28*/
		};
		static_assert(sizeof(LFGuild_SearchGuild_Struct) == 0x28, "TOB LFGuild search guild");

		// S2C command 3/4 result lists: u32 Command; uint8 Unknown004[20]; u32 Count; then serialized entries.
		static constexpr uint32 LFGuild_HeaderSize = 0x18;

		// OP_LFGuild commands (the same numbers are used by the server). 2 and 5 are ignored by the client, 6 shows the window.
		enum LFGuildCommand : uint32 {
			LFGuildPlayerStatus = 0,
			LFGuildGuildStatus  = 1,
			LFGuildPlayerSearch = 3,
			LFGuildGuildSearch  = 4,
		};


		struct FindPerson_Point {
			float y;
			float x;
			float z;
		};

		// C2S OP_FindPersonRequest, 40 bytes
		struct FindPersonRequest_Struct {
			/*00*/ uint32           request_type; // 1 = findable spawn, 2 = find-list id, 3/4 = by name, 5/7 = location, 6 = zone connection
			/*04*/ uint16           id;           // client copies only 16 bits
			/*06*/ uint16           unknown06;    // uninitialised
			/*08*/ uint32           unknown08;
			/*12*/ uint32           unknown12;
			/*16*/ FindPerson_Point client_pos;
			/*28*/ FindPerson_Point dest;
			/*40*/
		};
		static_assert(sizeof(FindPersonRequest_Struct) == 40, "TOB FindPersonRequest_Struct must be 40 bytes");

		struct GroupJoin_Struct {
			/*000*/ char owner_name[64]; // merc
			/*064*/ char membername[64];
			/*128*/ uint8 merc;
			/*129*/ uint8 padding129[3];
			/*132*/ uint32 level;
			/*136*/ uint8 unknown136[12];
			/*148*/
		};

		struct GroupGeneric_Struct {
			/*0000*/ char name1[64];
			/*0064*/ char name2[64];
			/*0128*/ uint32	unknown0128;
			/*0132*/ uint32	unknown0132;
			/*0136*/ uint32	unknown0136;
			/*0140*/ uint32	unknown0140;
			/*0144*/ uint32	unknown0144;
			/*0148*/ uint32	unknown0148;
			/*0152*/ uint16	unknown0152;
			/*0154*/ uint8	unknown0154[14];
			/*0168*/
		};

		// TOB pick pocket wire format (S→C and C→S identical layout).
		// coin sits at unaligned offset 0x0D; valid under pack(1).
		struct PickPocket_Struct {
			/*0x00*/ uint32 to;
			/*0x04*/ uint32 from;
			/*0x08*/ uint32 myskill;
			/*0x0C*/ uint8  type;
			/*0x0D*/ uint32 coin;
			/*0x11*/ uint32 nameLen;
			// char name[nameLen] follows, then uint8 luckily
			/*0x15*/
		};

		struct AugmentInfo_Struct
		{
			/*000*/ uint32	itemid; // id of the solvent needed
			/*004*/ uint32	window;	// window to display the information in
			/*008*/ char	augment_info[64]; // total packet length 80, all the rest were always 00
			/*072*/ uint32	unknown072; // seen 0, 56
			/*076*/ uint32	unknown076; // seen 8, 3, 11, always matches what client sends
			/*080*/
		};

		//seems to be unchanged from rof2?
		//it's the same size at least
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

		struct ApplyPoison_Struct
		{
			InventorySlot_Struct inventorySlot;
			uint32 success;
		};

		/*
		** Click Object Acknowledgement Struct
		** Response to client clicking on a World Container (ie, forge)
		** Seems to have not changed from RoF2
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

		// Server→Client (38 bytes): container_slot serialized without Padding2 to match client deserializer
		struct RecipeAutoCombine_Struct {
			/*00*/	uint32 object_type;
			/*04*/	uint32 some_id;
			/*08*/	int32  container_type;       // InventorySlot.Type
			/*12*/	int16  container_slot_index; // InventorySlot.Slot
			/*14*/	int16  container_subindex;   // InventorySlot.SubIndex
			/*16*/	int16  container_augindex;   // InventorySlot.AugIndex
			/*18*/	InventorySlot_Struct unknown_slot;  // 12 bytes; echoed in reply
			/*30*/	uint32 recipe_id;
			/*34*/	uint32 reply_code;
			/*38*/
		};

		// Client→Server (56 bytes): layout from CTradeskillWnd::HandleCombine
		struct RecipeAutoCombine_CS_Struct {
			/*00*/	uint32 con_type;            // GetConType of container
			/*04*/	uint32 recipe_id;           // GetItemRecordNum of recipe item
			/*08*/	uint32 unknown1;            // constant 4
			/*12*/	uint8  unknown2[8];         // zeros (uninitialized)
			/*20*/	InventorySlot_Struct container_slot;  // 12 bytes
			/*32*/	uint32 object_type;
			/*36*/	uint32 some_id;
			/*40*/	uint8  flag1;
			/*41*/	uint8  flag2;
			/*42*/	uint8  unknown3;
			/*43*/	uint8  flag3;
			/*44*/	uint32 unknown4;            // zeros (uninitialized)
			/*48*/	int64  start_tick;
			/*56*/
		};

		/*
		** New Combine Struct
		** Client requesting to perform a tradeskill combine
		** Size: 28 bytes
		** Used In: OP_TradeSkillCombine
		** Last Updated: 01-05-2013
		*/
		struct NewCombine_Struct
		{
			/*00*/	InventorySlot_Struct container_slot;
			/*12*/	InventorySlot_Struct guildtribute_slot;	// Slot type is 8? (MapGuildTribute = 8)
			/*24*/	uint32               unknown0x18;		// TOB wire format; not used by server
			/*28*/
		};

		struct DisciplineTimer_Struct {
			/*00*/ uint32 TimerID;
			/*04*/ uint32 Duration;
			/*08*/ uint32 Unknown08;   // server-side absolute expiry time (fasttime units)
			/*0C*/ uint32 ServerTime;  // server's current time when packet sent (fasttime units)
		};

		struct Disciplines_Struct {
			uint32 values[MAX_PP_DISCIPLINES];
		};

		struct Merchant_Sell_Request_Struct {
			/*000*/ uint32 npcid;		// Merchant NPC's entity id
			/*004*/ uint32 playerid;	// unset by client (stack garbage at this offset)
			/*008*/ uint32 itemslot;	// lower 4 bytes of 8-byte item instance ID
			/*012*/ uint32 unknown12;	// upper 4 bytes of 8-byte item instance ID
			/*016*/ uint32 quantity;	// Already sold
			/*020*/ uint32 unknown20;	// unset by client — trailing pad; client sends 24 bytes total
			/*024*/
		};

		struct Merchant_Sell_Response_Struct {
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

		struct Merchant_Purchase_Request_Struct {
			/*000*/	uint32	npcid;			// Merchant NPC's entity id
			/*004*/	TypelessInventorySlot_Struct	inventory_slot;
			/*010*/ int16 padding;
			/*012*/	uint32	quantity;
			/*016*/	
		};

		struct Merchant_Purchase_Response_Struct {
			/*000*/	TypelessInventorySlot_Struct	inventory_slot;
			/*006*/ int16 padding;
			/*008*/	uint32	quantity;
			/*012*/	uint32	price;
			/*016*/ uint32  unknown016;
			/*020*/
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

		struct Stun_Struct { // 8 bytes total
			/*000*/	uint32	duration; // Duration of stun
			/*004*/	uint8	unknown004; // seen 0
			/*005*/	uint8	unknown005; // seen 163
			/*006*/	uint8	unknown006; // seen 67
			/*007*/	uint8	unknown007; // seen 0
			/*008*/
		};

		/*
		 * Structs used in OP_CharacterCreateRequest
		 */
		struct RaceClassAllocation {
			uint32 Index;
			uint32 BaseStats[7];
			uint32 DefaultPointAllocation[7];
		};

		struct RaceClassCombos {
			uint64 ExpansionRequired;
			uint32 Race;
			uint32 Class;
			uint32 Deity;
			uint32 AllocationIndex;
			uint32 Zone;
		};

		struct fling_struct {
		/* 00 */ float  speed_z;       // must be > 0 for handler to proceed
		/* 04 */ float  new_y;
		/* 08 */ float  new_x;
		/* 12 */ float  new_z;
		/* 16 */ float  radius;        // landing zone radius; 0.0f = land exactly at target
		/* 20 */ uint32 unknown;       // not accessed; padding
		/* 24 */ int32  travel_time;   // -1 = auto-calc; 0 = default 1000ms; >0 = explicit ms
		/* 28 */ uint8  collision;     // 0 = disable collision; non-zero = keep collision
		/* 29 */ uint8  fall_damage;   // 0 = no fall damage (player.408=1); non-zero = take damage
		/* 30 */ uint8  z_override;    // 1 = override z-target comparison
		/* 31 */
		};

		struct BeggingResponse_Struct
		{
			/*00*/	uint32	Unknown00;
			/*04*/	uint32	Unknown04;
			/*08*/	uint32	Unknown08;
			/*12*/	uint8 	Result;	// 0 = Fail, 1 = Plat, 2 = Gold, 3 = Silver, 4 = Copper
			/*13*/	uint32	Amount;
			/*17*/  uint32  StringSize; // set this to 0, but it's a string size
			/*21*/  uint8   Lucky; // set to 1 to message a lucky beg
		};


	//Bazaar / Trader / Barter actions
	enum TOBBazaarTraderBuyerActions {
		Zero             = 0,
		BeginTraderMode  = 1,
		EndTraderMode    = 2,
		PriceUpdate      = 3,
		EndTransaction   = 4,
		BazaarSearch     = 7,
		WelcomeMessage   = 9,
		BuyTraderItem    = 10,
		ListTraderItems  = 11,
		BazaarInspect    = 18,
		ClickTrader      = 28,
		ItemMove         = 19,
		ReconcileItems   = 20,
		FirstOpenSearch  = 26
	};
	
	enum TOBBuyerActions {
		BuyerSearchResults   = 0x00,
		BuyerBuyLine         = 0x06,
		BuyerModifyBuyLine   = 0x07,
		BuyerRemoveItem      = 0x08,
		BuyerSellItem        = 0x09,
		BuyerBuyItem         = 0x0a,
		BuyerInspectBegin    = 0x0b,
		BuyerInspectEnd      = 0x0c,
		BuyerAppearance      = 0x0d,
		BuyerSendBuyLine     = 0x0e,
		BuyerItemInspect     = 0x0f,
		BuyerBrowsingBuyLine = 0x10,
		BarterWelcomeMessage = 0x11,
		BuyerWelcomeMessage  = 0x13,
		BuyerGreeting        = 0x14,
		BuyerInventoryFull   = 0x16
	};
	
	struct BarterItemSearchLinkRequest_Struct {
		uint32 action;
		uint32 unknown_004;
		uint32 seller_id;
		uint32 buyer_id;
		uint32 unknown_016;
		uint32 slot_id; // 0xffffffff main buy line 0x0 trade_item_1, 0x1 trade_item_2
		uint32 item_id;
		uint32 unknown_028;
	};
	
	struct BuyerWelcomeMessageUpdate_Struct {
		uint32 action;
		char   unknown_004[64];
		uint32 unknown_068;
		char   welcome_message[256];
	};
	
	struct Buyer_SetAppearance_Struct {
		uint32	action;
		uint32	entity_id;
		char	unknown[64];
		uint32	enabled;
	};
	
	struct BuyerRemoveItem_Struct {
		uint32	action;
		uint32	unknown004;
		uint32	slot_id;
		uint32	toggle;
	};
	
	struct BuyerLineSellItem_Struct {
		uint32                     action;
		uint32                     purchase_method; // 0 direct merchant, 1 via /barter window
		uint32                     unknown008;
		uint32                     buyer_entity_id;
		uint32                     seller_entity_id;
		char                       unknown[15];
		uint32                     slot;
		uint8                      enabled;
		uint32                     item_id;
		char                       item_name[64];
		uint32                     item_icon;
		uint32                     item_quantity;
		uint8                      item_toggle;
		uint32                     item_cost;
		uint32                     no_trade_items;
		BuyerLineTradeItems_Struct trade_items[10];
		char                       unknown2[13];
		uint32                     seller_quantity;
	};
	
	struct BuyerLineItemsSearch_Struct {
		uint32                     slot;
		uint8                      enabled;
		uint32                     item_id;
		char                       item_name[64];
		uint32                     item_icon;
		uint32                     item_quantity;
		uint8                      item_toggle;
		uint32                     item_cost;
		uint32                     buyer_id;
		BuyerLineTradeItems_Struct trade_items[MAX_BUYER_COMPENSATION_ITEMS];
	};
	
	struct BuyerLineSearch_Struct {
		uint32                                   action;
		uint32                                   no_items;
		std::vector<BuyerLineItemsSearch_Struct> buy_line;
	};
	
	struct BuyerStart_Struct {
		uint32                     action;
		uint16                     no_buyer_lines;
		uint32                     slot;
		uint8                      enabled;
		uint32                     item_id;
		char                       item_name[1];    // vary length
		uint32                     item_icon;
		uint32                     item_quantity;
		uint8                      toggle;
		uint32                     item_cost;
		uint32                     no_trade_items;
		BuyerLineTradeItems_Struct trade_items[1]; // size is actually no_trade_items.  If 0, then this is not in packet
		char                       unknown[13];
	};
	
	struct BuyerItemSearchResultEntry_Struct {
		char   item_name[64];
		uint32 item_id;
		uint32 item_icon;
		uint32 unknown_072;
	};
	
	struct BuyerItemSearchResults_Struct {
		uint32                            action;
		uint32                            result_count;
		BuyerItemSearchResultEntry_Struct results[];
	};
	
	enum {
		BazaarTrader_StartTraderMode  = 1,
		BazaarTrader_EndTraderMode    = 2,
		BazaarTrader_UpdatePrice      = 3,
		BazaarTrader_EndTransaction   = 4,
		BazaarSearchResults           = 7,
		BazaarWelcome                 = 9,
		BazaarBuyItem                 = 10,
		BazaarTrader_ShowItems        = 11,
		BazaarSearchDone              = 12,
		BazaarTrader_CustomerBrowsing = 13,
		BazaarInspectItem             = 18,
		BazaarSearchDone2             = 19,
		BazaarTrader_StartTraderMode2 = 22
	};
	
	enum {
		BazaarPriceChange_Fail        = 0,
		BazaarPriceChange_UpdatePrice = 1,
		BazaarPriceChange_RemoveItem  = 2,
		BazaarPriceChange_AddItem     = 3
	};
	
	struct BazaarWindowStart_Struct {
		uint8   Action;
		uint8   Unknown001;
		uint16  Unknown002;
	};
	
	struct BazaarDeliveryCost_Struct {
		uint32 action;
		uint32 voucher_delivery_cost;
		float  parcel_deliver_cost; //percentage of item cost
		uint32 unknown_010;
	};
	
	struct BazaarWelcome_Struct {
		uint32 action;
		uint32 unknown_004;
		uint32 num_of_traders;
		uint32 num_of_items;
	};
	
	struct BazaarSearch_Struct {
	/*000*/ uint32 action;
	/*004*/ uint8  search_scope;//1 all traders 0 local traders
	/*005*/ uint8  unknown_005{0};
	/*006*/ uint16 unknown_006{0};
	/*008*/ uint32 unknown_008{0};
	/*012*/ uint32 unknown_012{0};
	/*016*/ uint32 trader_id;
	/*020*/ uint32 _class;
	/*024*/ uint32 race;
	/*028*/ uint32 item_stat;
	/*032*/ uint32 slot;
	/*036*/ uint32 type;
	/*040*/ char   item_name[64];
	/*104*/ uint32 min_cost;
	/*108*/ uint32 max_cost;
	/*112*/ uint32 min_level;
	/*116*/ uint32 max_level;
	/*120*/ uint32 max_results;
	/*124*/ uint32 prestige;
	/*128*/ uint32 augment;
	};
	
	struct BazaarInspect_Struct {
		uint32 action;
		uint32 unknown_004;
		uint32 trader_id;
		char   serial_number[17];
		char   unknown_029[3];
		uint32 item_id;
		uint32 unknown_036;
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
	
	struct BazaarSearchDetails_Struct { //24+name+17
	/*014*/	uint32	trader_id;
	/*018*/	char	serial_num[17];
	/*022*/	uint32	cost;
	/*026*/	uint32	quanity;
	/*030*/	uint32	id;
	/*034*/	uint32	icon;
	/*038*/	char	name[1];
	/*039*/	uint32	stat;
	};
	
	struct BazaarSearchResults_Struct { //14
	/*000*/    uint32                     unknown000;
	/*004*/    uint16                     zone_id;
	/*006*/    uint32                     entity_id;
	/*010*/    uint32                     num_items;
	/*014*/    BazaarSearchDetails_Struct items[];
	};
	
	struct TraderItemSerial_Struct {
		char	serial_number[17];
		uint8	unknown_018;
	
		void operator=(uint32 a) {
			auto _tmp = fmt::format("{:016}", a);
			strn0cpy(this->serial_number, _tmp.c_str(), sizeof(this->serial_number));
		}
	};
	
	struct BeginTrader_Struct {
	/*0000*/    uint32                  action;
	/*0004*/    TraderItemSerial_Struct items[200];
	/*3604*/    uint32                  item_cost[200];
	/*4404*/
	};
	
	struct ClickTrader_Struct {
	/*0000*/    uint32                  action;
	/*0004*/    TraderItemSerial_Struct items[200];
	/*3604*/    uint32                  item_cost[200];
	/*4404*/
	};
	
	struct GetItems_Struct {
		uint32	items[200];
	};
	
	struct BecomeTrader_Struct {
		uint32 entity_id;
		uint32 action;
	};
	
	struct BazaarWindowRemoveTrader_Struct {
		uint32 action;
		uint32 trader_id;
	};
	
	struct TraderPriceUpdate_Struct {
		uint32 action;
		char   serial_number[17];
		char   unknown_021[3];
		uint32 unknown_024;
		uint32 new_price;
	};
	
	struct Trader_ShowItems_Struct {
	/*000*/	uint32 action;
	/*004*/	uint32 entity_id;
	/*008*/	uint32 unknown_008;
	/*012*/
	};
	
	struct TraderStatus_Struct {
	/*000*/	uint32 action;
	/*004*/	uint32 sub_action;
	/*008*/	uint32 uknown_008;
	/*012*/
	};
	
	struct TraderBuy_Struct {
	/*000*/ uint32	action;
	/*004*/	uint32	method;
	/*008*/ uint32	sub_action;
	/*012*/	uint32	unknown_012;
	/*016*/ uint32	trader_id;
	/*020*/ char	buyer_name[64];
	/*084*/ char	seller_name[64];
	/*148*/ char	unknown_148[32];
	/*180*/ char	item_name[64];
	/*244*/ char	serial_number[17];
	/*261*/ char	unknown_261[3];
	/*264*/ uint32	item_id;
	/*268*/ uint32	price;
	/*272*/ uint32	already_sold;
	/*276*/ uint32	unknown_276;
	/*280*/ uint32	quantity;
	/*284*/
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
		/*000*/ uint32 Unknown000;
		/*004*/ uint32 TraderID;
		/*008*/ char   SerialNumber[17];
		/*024*/ uint32 Unknown012;
		/*028*/
	};
	
	struct TraderClick_Struct{
		/*000*/	uint32 action;
		/*004*/	uint32 trader_id;
		/*008*/	uint32 unknown_008;
		/*012*/
	};

	// ---- OP_AdvLoot / OP_AdvLootMisc, client to server ----
	// Every body starts with a u16 subtype; the rest depends on it. Sizes are the minimum the server accepts.
	enum class TOBAdvLootSubtype : uint16 {
		Sync   = 3,
		Leave  = 7,
		Loot   = 8,
		Ask    = 0xa,
		Choice = 0xb,
		Give   = 0xe,
		State  = 0x11,
		Roll   = 0x12,
	};

	enum class TOBAdvLootMiscSubtype : uint16 {
		FreeGrab = 0xf,
	};

	struct AdvLootSubtype_Struct {
		/*000*/ uint16 subtype;
		/*002*/
	};

	struct AdvLootLeave_Struct {
		/*000*/ uint16 subtype;
		/*002*/ uint64 item_id;
		/*010*/ uint32 corpse_id;
		/*014*/ uint32 quantity;
		/*018*/
	};

	struct AdvLootLoot_Struct {
		/*000*/ uint16 subtype;
		/*002*/ uint64 item_id;
		/*010*/ uint32 corpse_id;
		/*014*/ uint32 quantity;
		/*018*/ uint32 sequence;	// echoed in the result so the window can drop the request
		/*022*/ uint8  flag;
		/*023*/
	};

	struct AdvLootChoice_Struct {
		/*000*/ uint16 subtype;
		/*002*/ uint32 transaction_id;
		/*006*/ uint32 corpse_id;
		/*010*/ uint64 item_id;
		/*018*/ uint16 count;		// entries in the trailing list, not used
		/*020*/ uint32 choice;		// first list entry; a NUL-terminated string follows it
		/*024*/
	};

	// master looter asking for need/greed
	struct AdvLootAsk_Struct {
		/*000*/ uint16 subtype;
		/*002*/ uint32 transaction_id;
		/*006*/ uint64 item_id;
		/*014*/ uint32 state;
		/*018*/ uint8  unknown018;
		/*019*/ uint32 time;
		/*023*/ uint32 count;		// entries in the trailing list
		/*027*/ uint32 corpse_id;	// first list entry
		/*031*/ uint32 quantity;
		/*035*/
	};

	// master looter giving an item or leaving it; a NUL-terminated player name follows
	struct AdvLootGive_Struct {
		/*000*/ uint16 subtype;
		/*002*/ uint32 transaction_id;	// the row's ask id, 0 when it was never asked
		/*006*/ uint32 corpse_id;
		/*010*/ uint64 item_id;
		/*018*/ uint16 quantity;
		/*020*/ uint8  leave;			// 1 = Leave On Corpse / Leave All, 0 = give
		/*021*/
	};

	struct AdvLootState_Struct {
		/*000*/ uint16 subtype;
		/*002*/ uint32 transaction_id;
		/*006*/ uint32 time;
		/*010*/ uint32 state;
		/*014*/	// one more byte follows; the server does not read it
	};

	struct AdvLootRoll_Struct {
		/*000*/ uint16 subtype;
		/*002*/ uint32 transaction_id;
		/*006*/
	};

	struct AdvLootMiscFreeGrab_Struct {
		/*000*/ uint16 subtype;
		/*002*/ uint32 transaction_id;
		/*006*/ uint32 corpse_id;
		/*010*/ // a string follows
	};

	// ---- OP_AdvLootCorpseItems, server to client ----
	// Variable length, built with SerializeBuffer:
	//   header:  u8 present, u8 shared, u32 unlock_seconds, u32 corpse_id, u32 item_count, cstr corpse_name
	//   per item: u64 item_id, u32 icon, u32 shared_quantity, u32 stack_size, u8 unknown, u32 unknown,
	//             u32 personal_quantity, cstr item_name, AdvLootItemTrailer_Struct, lenstr item_link
	struct AdvLootItemTrailer_Struct {
		/*000*/ uint32 unknown000;
		/*004*/ uint32 unknown004;
		/*008*/ uint32 unknown008;
		/*012*/ uint32 unknown012;
		/*016*/ uint32 unknown016;
		/*020*/
	};

	// values the client expects in AdvLootItemTrailer_Struct
	constexpr uint32 ADVLOOT_TRAILER_UNKNOWN004 = 0xFFFF;
	constexpr uint32 ADVLOOT_TRAILER_UNKNOWN008 = 0xFFFF;

#pragma pack()


	// These task structs are not directly serialized because they have variable sized members. They are used
	// as intermediate structs to deserialize server values

	struct CompletedTask_Struct
	{
		uint32 task_id;
		std::string title;
		uint32 completed_time;
	};

	struct TaskActivity_Struct
	{
		uint32 task_slot_index;
		uint32 task_type;
		uint32 task_id;
		uint32 activity_id;
		uint32 list_group;
		int32  activity_type;
		uint8  optional_byte;
		std::string target_name;
		std::string item_list;
		uint32 goal_count;
		std::string skill_list;
		std::string spell_list;
		std::string zones;
		uint32 dz_switch_id;
		std::string description_override;
		uint32 done_count;
		uint8  flag;
		std::string zones2;
	};

	struct TaskDescription_Struct
	{
		uint32 sequence_number;
		uint32 task_id;
		uint8  open_window;
		uint32 task_type;
		uint32 reward_type;
		std::string title;
		uint32 duration;
		uint32 start_time;
		std::string description;
		uint8  has_rewards;
		uint32 coin_reward;
		uint32 xp_reward;
		uint32 faction_reward;
		std::string reward_text;
		std::string item_link;
		uint32 points;
		uint8  has_reward_selection;
	};

	struct TaskSelectWindowSelector_Struct
	{
		int32 selector_id;
		int32 activity_type;
		int32 request_type; // deprecated
		std::string target_name;
		std::string item_list;
		int32 goal_count;
		std::string skill_list;
		std::string spell_list;
		std::string zones;
		std::string description_override;
		std::string zones_internal;
	};

	struct TaskSelectWindow_Struct
	{
		uint32 task_id;
		float reward_multiplier;
		uint32 duration;
		std::string title;
		std::string description;
		bool preview_enable;
		std::vector<TaskSelectWindowSelector_Struct> selectors;
	};

		// OP_Surname, both directions. The surname follows the header without a terminator.
		struct Surname_Struct {
			/*000*/ EqGuid character; // the character the surname belongs to
			/*008*/ int32  status;    // 0 from the client; 1 from the server when the surname was accepted
			/*012*/ uint32 length;    // surname length in bytes
			/*016*/
		};

	};	//end namespace structs
};	//end namespace tob
