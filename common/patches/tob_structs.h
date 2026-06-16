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
			Unknown18,
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
			Unknown40,
			Unknown41,
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
			/*032*/	uint8	unknown032[144]; // this is mostly a string passed to the teleport function (follow starting at 0x1401F71BA), it appears to be an override for a message
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
			/*144*/ uint8 type;
			/*145*/ uint8 level;
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
		//@0x1402024C0 MissileHitInfo::Deserialize(CUnSerializeBuffer *buffer);
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

	};	//end namespace structs
};	//end namespace tob
