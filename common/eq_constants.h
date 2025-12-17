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
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#pragma once

#include "common/skills.h"
#include "common/types.h"

namespace AppearanceType {
	constexpr uint32 Die                   = 0; // Causes the client to keel over and zone to bind point (default action)
	constexpr uint32 WhoLevel              = 1; // Level that shows up on /who
	constexpr uint32 MaxHealth             = 2;
	constexpr uint32 Invisibility          = 3; // 0 = Visible, 1 = Invisible
	constexpr uint32 PVP                   = 4; // 0 = Non-PVP, 1 = PVP
	constexpr uint32 Light                 = 5; // Light type emitted by player (lightstone, shiny shield)
	constexpr uint32 Animation             = 14; // 100 = Standing, 102 = Freeze, 105 = Looting, 110 = Sitting, 111 = Crouching, 115 = Lying
	constexpr uint32 Sneak                 = 15; // 0 = Normal, 1 = Sneaking
	constexpr uint32 SpawnID               = 16; // Server -> Client, sets player spawn ID
	constexpr uint32 Health                = 17; // Client->Server, my HP has changed (like regen tic)
	constexpr uint32 Linkdead              = 18; // 0 = Normal, 1 = Linkdead
	constexpr uint32 FlyMode               = 19; // 0 = Off, 1 = Flying, 2 = Levitating, 3 = Water, 4 = Floating, 5 = Levitating while Running
	constexpr uint32 GM                    = 20; // 0 = Non-GM, 1 = GM
	constexpr uint32 Anonymous             = 21; // 0 = Non-Anonymous, 1 = Anonymous, 2 = Roleplaying
	constexpr uint32 GuildID               = 22;
	constexpr uint32 GuildRank             = 23;
	constexpr uint32 AFK                   = 24; // 0 = Non-AFK, 1 = AFK
	constexpr uint32 Pet                   = 25; // Parameter is Entity ID of owner, or 0 for when charm breaks
	constexpr uint32 Summoned              = 27;
	constexpr uint32 Split                 = 28; // 0 = No Split, 1 = Auto Split
	constexpr uint32 Size                  = 29; // Spawn's Size
	constexpr uint32 SetType               = 30; // 0 = PC, 1 = NPC, 2 = Corpse
	constexpr uint32 NPCName               = 31; // Change PC name color to NPC name color
	constexpr uint32 AARank                = 32; // AA Rank Title ID, title in /who?
	constexpr uint32 CancelSneakHide       = 33; // Turns off Hide and Sneak
	constexpr uint32 AreaHealthRegen       = 35; // Guild Hall Regeneration Pool sets to value * 0.001
	constexpr uint32 AreaManaRegen         = 36; // Guild Hall Regeneration Pool sets to value * 0.001
	constexpr uint32 AreaEnduranceRegen    = 37; // Guild Hall Regeneration Pool sets to value * 0.001
	constexpr uint32 FreezeBeneficialBuffs = 38; // Freezes beneficial buff timers for PCs
	constexpr uint32 NPCTintIndex          = 39;
	constexpr uint32 GroupAutoConsent      = 40; // Auto Consent Group
	constexpr uint32 RaidAutoConsent       = 41; // Auto Consent Raid
	constexpr uint32 GuildAutoConsent      = 42; // Auto Consent Guild
	constexpr uint32 ShowHelm              = 43; // 0 = Hide, 1 = Show
	constexpr uint32 DamageState           = 44; // The damage state of a destructible object (0 through 10) plays sound IDs, most only have 2 or 4 states though
	constexpr uint32 EQPlayers             = 45; // EQ Players Update
	constexpr uint32 FindBits              = 46; // Set Find Bits?
	constexpr uint32 TextureType           = 48; // Texture Type?
	constexpr uint32 FacePick              = 49; // Turns off face pick window?
	constexpr uint32 AntiCheat             = 51; // Sent by the client randomly telling the server how long since last action has occurred
	constexpr uint32 GuildShow             = 52;
	constexpr uint32 OfflineMode           = 53; // Offline Mode
}

namespace Animation {
	constexpr uint32 Standing  = 100;
	constexpr uint32 Freeze    = 102;
	constexpr uint32 Looting   = 105;
	constexpr uint32 Sitting   = 110;
	constexpr uint32 Crouching = 111;
	constexpr uint32 Lying     = 115;
}

constexpr int16 RECAST_TYPE_UNLINKED_ITEM = -1;

typedef enum {
	eaStanding = 0,
	eaSitting,		//1
	eaCrouching,	//2
	eaDead,			//3
	eaLooting,		//4
	_eaMaxAppearance
} EmuAppearance;

namespace Chat {
	const uint16 White       = 0;
	const uint16 DimGray     = 1;
	const uint16 Default     = 1;
	const uint16 Green       = 2;
	const uint16 BrightBlue  = 3;
	const uint16 LightBlue   = 4;
	const uint16 Magenta     = 5;
	const uint16 Gray        = 6;
	const uint16 LightGray   = 7;
	const uint16 NPCQuestSay = 10;
	const uint16 DarkGray    = 12;
	const uint16 Red         = 13;
	const uint16 Lime        = 14;
	const uint16 Yellow      = 15;
	const uint16 Blue        = 16;
	const uint16 LightNavy   = 17;
	const uint16 Cyan        = 18;
	const uint16 Black       = 20;

	/**
	 * User colors
	 */
	const uint16 Say              = 256;
	const uint16 Tell             = 257;
	const uint16 Group            = 258;
	const uint16 Guild            = 259;
	const uint16 OOC              = 260;
	const uint16 Auction          = 261;
	const uint16 Shout            = 262;
	const uint16 Emote            = 263;
	const uint16 Spells           = 264;
	const uint16 YouHitOther      = 265;
	const uint16 OtherHitYou      = 266;
	const uint16 YouMissOther     = 267;
	const uint16 OtherMissYou     = 268;
	const uint16 Broadcasts       = 269;
	const uint16 Skills           = 270;
	const uint16 Disciplines      = 271;
	const uint16 Unused1          = 272;
	const uint16 DefaultText      = 273;
	const uint16 Unused2          = 274;
	const uint16 MerchantOffer    = 275;
	const uint16 MerchantExchange = 276;
	const uint16 YourDeath        = 277;
	const uint16 OtherDeath       = 278;
	const uint16 OtherHitOther    = 279;
	const uint16 OtherMissOther   = 280;
	const uint16 Who              = 281;
	const uint16 YellForHelp      = 282;
	const uint16 NonMelee         = 283;
	const uint16 SpellWornOff     = 284;
	const uint16 MoneySplit       = 285;
	const uint16 Loot             = 286;
	const uint16 DiceRoll         = 287;
	const uint16 OtherSpells      = 288;
	const uint16 SpellFailure     = 289;
	const uint16 ChatChannel      = 290;
	const uint16 Chat1            = 291;
	const uint16 Chat2            = 292;
	const uint16 Chat3            = 293;
	const uint16 Chat4            = 294;
	const uint16 Chat5            = 295;
	const uint16 Chat6            = 296;
	const uint16 Chat7            = 297;
	const uint16 Chat8            = 298;
	const uint16 Chat9            = 299;
	const uint16 Chat10           = 300;
	const uint16 MeleeCrit        = 301;
	const uint16 SpellCrit        = 302;
	const uint16 TooFarAway       = 303;
	const uint16 NPCRampage       = 304;
	const uint16 NPCFlurry        = 305;
	const uint16 NPCEnrage        = 306;
	const uint16 EchoSay          = 307;
	const uint16 EchoTell         = 308;
	const uint16 EchoGroup        = 309;
	const uint16 EchoGuild        = 310;
	const uint16 EchoOOC          = 311;
	const uint16 EchoAuction      = 312;
	const uint16 EchoShout        = 313;
	const uint16 EchoEmote        = 314;
	const uint16 EchoChat1        = 315;
	const uint16 EchoChat2        = 316;
	const uint16 EchoChat3        = 317;
	const uint16 EchoChat4        = 318;
	const uint16 EchoChat5        = 319;
	const uint16 EchoChat6        = 320;
	const uint16 EchoChat7        = 321;
	const uint16 EchoChat8        = 322;
	const uint16 EchoChat9        = 323;
	const uint16 EchoChat10       = 324;
	const uint16 DotDamage        = 325;
	const uint16 ItemLink         = 326;
	const uint16 RaidSay          = 327;
	const uint16 MyPet            = 328;
	const uint16 DamageShield     = 329;
	const uint16 LeaderShip       = 330;
	const uint16 PetFlurry        = 331;
	const uint16 PetCritical      = 332;
	const uint16 FocusEffect      = 333;
	const uint16 Experience       = 334;
	const uint16 System           = 335;
	const uint16 PetSpell         = 336;
	const uint16 PetResponse      = 337;
	const uint16 ItemSpeech       = 338;
	const uint16 StrikeThrough    = 339;
	const uint16 Stun             = 340;
};

// generation SQL:
// SELECT CONCAT('    constexpr uint16 ', UPPER(short_name), ' = ' , zoneidnumber, '; // ', long_name) from zone group by zoneidnumber ORDER BY zoneidnumber;
namespace Zones {
	constexpr uint16 QEYNOS = 1; // South Qeynos
	constexpr uint16 QEYNOS2 = 2; // North Qeynos
	constexpr uint16 QRG = 3; // The Surefall Glade
	constexpr uint16 QEYTOQRG = 4; // The Qeynos Hills
	constexpr uint16 HIGHPASS = 5; // Highpass Hold
	constexpr uint16 HIGHKEEP = 6; // High Keep
	constexpr uint16 FREPORTN = 8; // North Freeport
	constexpr uint16 FREPORTW = 9; // West Freeport
	constexpr uint16 FREPORTE = 10; // East Freeport
	constexpr uint16 RUNNYEYE = 11; // The Liberated Citadel of Runnyeye
	constexpr uint16 QEY2HH1 = 12; // The Western Plains of Karana
	constexpr uint16 NORTHKARANA = 13; // The Northern Plains of Karana
	constexpr uint16 SOUTHKARANA = 14; // The Southern Plains of Karana
	constexpr uint16 EASTKARANA = 15; // Eastern Plains of Karana
	constexpr uint16 BEHOLDER = 16; // Gorge of King Xorbb
	constexpr uint16 BLACKBURROW = 17; // Blackburrow
	constexpr uint16 PAW = 18; // The Lair of the Splitpaw
	constexpr uint16 RIVERVALE = 19; // Rivervale
	constexpr uint16 KITHICOR = 20; // Kithicor Forest
	constexpr uint16 COMMONS = 21; // West Commonlands
	constexpr uint16 ECOMMONS = 22; // East Commonlands
	constexpr uint16 ERUDNINT = 23; // The Erudin Palace
	constexpr uint16 ERUDNEXT = 24; // Erudin
	constexpr uint16 NEKTULOS = 25; // The Nektulos Forest
	constexpr uint16 CSHOME = 26; // Sunset Home
	constexpr uint16 LAVASTORM = 27; // The Lavastorm Mountains
	constexpr uint16 NEKTROPOS = 28; // Nektropos
	constexpr uint16 HALAS = 29; // Halas
	constexpr uint16 EVERFROST = 30; // Everfrost Peaks
	constexpr uint16 SOLDUNGA = 31; // Solusek's Eye
	constexpr uint16 SOLDUNGB = 32; // Nagafen's Lair
	constexpr uint16 MISTY = 33; // Misty Thicket
	constexpr uint16 NRO = 34; // Northern Desert of Ro
	constexpr uint16 SRO = 35; // Southern Desert of Ro
	constexpr uint16 BEFALLEN = 36; // Befallen
	constexpr uint16 OASIS = 37; // Oasis of Marr
	constexpr uint16 TOX = 38; // Toxxulia Forest
	constexpr uint16 HOLE = 39; // The Hole
	constexpr uint16 NERIAKA = 40; // Neriak - Foreign Quarter
	constexpr uint16 NERIAKB = 41; // Neriak - Commons
	constexpr uint16 NERIAKC = 42; // Neriak - 3rd Gate
	constexpr uint16 NERIAKD = 43; // Neriak Palace
	constexpr uint16 NAJENA = 44; // Najena
	constexpr uint16 QCAT = 45; // The Qeynos Aqueduct System
	constexpr uint16 INNOTHULE = 46; // Innothule Swamp
	constexpr uint16 FEERROTT = 47; // The Feerrott
	constexpr uint16 CAZICTHULE = 48; // Accursed Temple of CazicThule
	constexpr uint16 OGGOK = 49; // Oggok
	constexpr uint16 RATHEMTN = 50; // The Rathe Mountains
	constexpr uint16 LAKERATHE = 51; // Lake Rathetear
	constexpr uint16 GROBB = 52; // Grobb
	constexpr uint16 AVIAK = 53; // Aviak Village
	constexpr uint16 GFAYDARK = 54; // The Greater Faydark
	constexpr uint16 AKANON = 55; // Ak'Anon
	constexpr uint16 STEAMFONT = 56; // Steamfont Mountains
	constexpr uint16 LFAYDARK = 57; // The Lesser Faydark
	constexpr uint16 CRUSHBONE = 58; // Crushbone
	constexpr uint16 MISTMOORE = 59; // The Castle of Mistmoore
	constexpr uint16 KALADIMA = 60; // South Kaladim
	constexpr uint16 FELWITHEA = 61; // Northern Felwithe
	constexpr uint16 FELWITHEB = 62; // Southern Felwithe
	constexpr uint16 UNREST = 63; // The Estate of Unrest
	constexpr uint16 KEDGE = 64; // Kedge Keep
	constexpr uint16 GUKTOP = 65; // The City of Guk
	constexpr uint16 GUKBOTTOM = 66; // The Ruins of Old Guk
	constexpr uint16 KALADIMB = 67; // North Kaladim
	constexpr uint16 BUTCHER = 68; // Butcherblock Mountains
	constexpr uint16 OOT = 69; // Ocean of Tears
	constexpr uint16 CAULDRON = 70; // Dagnor's Cauldron
	constexpr uint16 AIRPLANE = 71; // The Plane of Sky
	constexpr uint16 FEARPLANE = 72; // The Plane of Fear
	constexpr uint16 PERMAFROST = 73; // The Permafrost Caverns
	constexpr uint16 KERRARIDGE = 74; // Kerra Isle
	constexpr uint16 PAINEEL = 75; // Paineel
	constexpr uint16 HATEPLANE = 76; // Plane of Hate
	constexpr uint16 ARENA = 77; // The Arena
	constexpr uint16 FIELDOFBONE = 78; // The Field of Bone
	constexpr uint16 WARSLIKSWOOD = 79; // The Warsliks Woods
	constexpr uint16 SOLTEMPLE = 80; // The Temple of Solusek Ro
	constexpr uint16 DROGA = 81; // The Temple of Droga
	constexpr uint16 CABWEST = 82; // Cabilis West
	constexpr uint16 SWAMPOFNOHOPE = 83; // The Swamp of No Hope
	constexpr uint16 FIRIONA = 84; // Firiona Vie
	constexpr uint16 LAKEOFILLOMEN = 85; // Lake of Ill Omen
	constexpr uint16 DREADLANDS = 86; // The Dreadlands
	constexpr uint16 BURNINGWOOD = 87; // The Burning Wood
	constexpr uint16 KAESORA = 88; // Kaesora
	constexpr uint16 SEBILIS = 89; // The Ruins of Sebilis
	constexpr uint16 CITYMIST = 90; // The City of Mist
	constexpr uint16 SKYFIRE = 91; // The Skyfire Mountains
	constexpr uint16 FRONTIERMTNS = 92; // Frontier Mountains
	constexpr uint16 OVERTHERE = 93; // The Overthere
	constexpr uint16 EMERALDJUNGLE = 94; // The Emerald Jungle
	constexpr uint16 TRAKANON = 95; // Trakanon's Teeth
	constexpr uint16 TIMOROUS = 96; // Timorous Deep
	constexpr uint16 KURN = 97; // Kurn's Tower
	constexpr uint16 ERUDSXING = 98; // Erud's Crossing
	constexpr uint16 STONEBRUNT = 100; // The Stonebrunt Mountains
	constexpr uint16 WARRENS = 101; // The Warrens
	constexpr uint16 KARNOR = 102; // Karnor's Castle
	constexpr uint16 CHARDOK = 103; // Chardok
	constexpr uint16 DALNIR = 104; // The Crypt of Dalnir
	constexpr uint16 CHARASIS = 105; // The Howling Stones
	constexpr uint16 CABEAST = 106; // Cabilis East
	constexpr uint16 NURGA = 107; // The Mines of Nurga
	constexpr uint16 VEESHAN = 108; // Veeshan's Peak
	constexpr uint16 VEKSAR = 109; // Veksar
	constexpr uint16 ICECLAD = 110; // The Iceclad Ocean
	constexpr uint16 FROZENSHADOW = 111; // The Tower of Frozen Shadow
	constexpr uint16 VELKETOR = 112; // Velketor's Labyrinth
	constexpr uint16 KAEL = 113; // Kael Drakkel
	constexpr uint16 SKYSHRINE = 114; // Skyshrine
	constexpr uint16 THURGADINA = 115; // The City of Thurgadin
	constexpr uint16 EASTWASTES = 116; // Eastern Wastes
	constexpr uint16 COBALTSCAR = 117; // Cobaltscar
	constexpr uint16 GREATDIVIDE = 118; // The Great Divide
	constexpr uint16 WAKENING = 119; // The Wakening Land
	constexpr uint16 WESTWASTES = 120; // The Western Wastes
	constexpr uint16 CRYSTAL = 121; // The Crystal Caverns
	constexpr uint16 NECROPOLIS = 123; // Dragon Necropolis
	constexpr uint16 TEMPLEVEESHAN = 124; // The Temple of Veeshan
	constexpr uint16 SIRENS = 125; // Siren's Grotto
	constexpr uint16 MISCHIEFPLANE = 126; // The Plane of Mischief
	constexpr uint16 GROWTHPLANE = 127; // The Plane of Growth
	constexpr uint16 SLEEPER = 128; // The Sleeper's Tomb
	constexpr uint16 THURGADINB = 129; // Icewell Keep
	constexpr uint16 ERUDSXING2 = 130; // Marauders Mire
	constexpr uint16 SHADOWHAVEN = 150; // Shadow Haven
	constexpr uint16 BAZAAR = 151; // The Bazaar
	constexpr uint16 NEXUS = 152; // Nexus
	constexpr uint16 ECHO_ = 153; // The Echo Caverns
	constexpr uint16 ACRYLIA = 154; // The Acrylia Caverns
	constexpr uint16 SHARVAHL = 155; // The City of Shar Vahl
	constexpr uint16 PALUDAL = 156; // The Paludal Caverns
	constexpr uint16 FUNGUSGROVE = 157; // The Fungus Grove
	constexpr uint16 VEXTHAL = 158; // Vex Thal
	constexpr uint16 SSERU = 159; // Sanctus Seru
	constexpr uint16 KATTA = 160; // Katta Castellum
	constexpr uint16 NETHERBIAN = 161; // Netherbian Lair
	constexpr uint16 SSRATEMPLE = 162; // Ssraeshza Temple
	constexpr uint16 GRIEGSEND = 163; // Grieg's End
	constexpr uint16 THEDEEP = 164; // The Deep
	constexpr uint16 SHADEWEAVER = 165; // Shadeweaver's Thicket
	constexpr uint16 HOLLOWSHADE = 166; // Hollowshade Moor
	constexpr uint16 GRIMLING = 167; // Grimling Forest
	constexpr uint16 MSERU = 168; // Marus Seru
	constexpr uint16 LETALIS = 169; // Mons Letalis
	constexpr uint16 TWILIGHT = 170; // The Twilight Sea
	constexpr uint16 THEGREY = 171; // The Grey
	constexpr uint16 TENEBROUS = 172; // The Tenebrous Mountains
	constexpr uint16 MAIDEN = 173; // The Maiden's Eye
	constexpr uint16 DAWNSHROUD = 174; // The Dawnshroud Peaks
	constexpr uint16 SCARLET = 175; // The Scarlet Desert
	constexpr uint16 UMBRAL = 176; // The Umbral Plains
	constexpr uint16 AKHEVA = 179; // The Akheva Ruins
	constexpr uint16 ARENA2 = 180; // The Arena Two
	constexpr uint16 JAGGEDPINE = 181; // The Jaggedpine Forest
	constexpr uint16 NEDARIA = 182; // Nedaria's Landing
	constexpr uint16 TUTORIAL = 183; // EverQuest Tutorial
	constexpr uint16 LOAD = 184; // Loading Zone
	constexpr uint16 LOAD2 = 185; // New Loading Zone
	constexpr uint16 HATEPLANEB = 186; // The Plane of Hate
	constexpr uint16 SHADOWREST = 187; // Shadowrest
	constexpr uint16 TUTORIALA = 188; // The Mines of Gloomingdeep
	constexpr uint16 TUTORIALB = 189; // The Mines of Gloomingdeep
	constexpr uint16 CLZ = 190; // Loading
	constexpr uint16 CODECAY = 200; // The Crypt of Decay
	constexpr uint16 POJUSTICE = 201; // The Plane of Justice
	constexpr uint16 POKNOWLEDGE = 202; // The Plane of Knowledge
	constexpr uint16 POTRANQUILITY = 203; // The Plane of Tranquility
	constexpr uint16 PONIGHTMARE = 204; // The Plane of Nightmares
	constexpr uint16 PODISEASE = 205; // The Plane of Disease
	constexpr uint16 POINNOVATION = 206; // The Plane of Innovation
	constexpr uint16 POTORMENT = 207; // Torment, the Plane of Pain
	constexpr uint16 POVALOR = 208; // The Plane of Valor
	constexpr uint16 BOTHUNDER = 209; // Bastion of Thunder
	constexpr uint16 POSTORMS = 210; // The Plane of Storms
	constexpr uint16 HOHONORA = 211; // The Halls of Honor
	constexpr uint16 SOLROTOWER = 212; // The Tower of Solusek Ro
	constexpr uint16 POWAR = 213; // Plane of War
	constexpr uint16 POTACTICS = 214; // Drunder, the Fortress of Zek
	constexpr uint16 POAIR = 215; // The Plane of Air
	constexpr uint16 POWATER = 216; // The Plane of Water
	constexpr uint16 POFIRE = 217; // The Plane of Fire
	constexpr uint16 POEARTHA = 218; // The Plane of Earth
	constexpr uint16 POTIMEA = 219; // The Plane of Time
	constexpr uint16 HOHONORB = 220; // The Temple of Marr
	constexpr uint16 NIGHTMAREB = 221; // The Lair of Terris Thule
	constexpr uint16 POEARTHB = 222; // The Plane of Earth
	constexpr uint16 POTIMEB = 223; // The Plane of Time
	constexpr uint16 GUNTHAK = 224; // The Gulf of Gunthak
	constexpr uint16 DULAK = 225; // Dulak's Harbor
	constexpr uint16 TORGIRAN = 226; // The Torgiran Mines
	constexpr uint16 NADOX = 227; // The Crypt of Nadox
	constexpr uint16 HATESFURY = 228; // Hate's Fury
	constexpr uint16 GUKA = 229; // Deepest Guk: Cauldron of Lost Souls
	constexpr uint16 RUJA = 230; // The Rujarkian Hills: Bloodied Quarries
	constexpr uint16 TAKA = 231; // Takish-Hiz: Sunken Library
	constexpr uint16 MIRA = 232; // Miragul's Menagerie: Silent Gallery
	constexpr uint16 MMCA = 233; // Mistmoore's Catacombs: Forlorn Caverns
	constexpr uint16 GUKB = 234; // The Drowning Crypt
	constexpr uint16 RUJB = 235; // The Rujarkian Hills: Halls of War
	constexpr uint16 TAKB = 236; // Takish-Hiz: Shifting Tower
	constexpr uint16 MIRB = 237; // Miragul's Menagerie: Frozen Nightmare
	constexpr uint16 MMCB = 238; // Mistmoore's Catacombs: Dreary Grotto
	constexpr uint16 GUKC = 239; // Deepest Guk: Ancient Aqueducts
	constexpr uint16 RUJC = 240; // The Rujarkian Hills: Wind Bridges
	constexpr uint16 TAKC = 241; // Takish-Hiz: Within the Compact
	constexpr uint16 MIRC = 242; // The Spider Den
	constexpr uint16 MMCC = 243; // Mistmoore's Catacombs: Struggles within the Progeny
	constexpr uint16 GUKD = 244; // The Mushroom Grove
	constexpr uint16 RUJD = 245; // The Rujarkian Hills: Prison Break
	constexpr uint16 TAKD = 246; // Takish-Hiz: Royal Observatory
	constexpr uint16 MIRD = 247; // Miragul's Menagerie: Hushed Banquet
	constexpr uint16 MMCD = 248; // Mistmoore's Catacombs: Chambers of Eternal Affliction
	constexpr uint16 GUKE = 249; // Deepest Guk: The Curse Reborn
	constexpr uint16 RUJE = 250; // The Rujarkian Hills: Drudge Hollows
	constexpr uint16 TAKE = 251; // Takish-Hiz: River of Recollection
	constexpr uint16 MIRE = 252; // The Frosted Halls
	constexpr uint16 MMCE = 253; // Mistmoore's Catacombs: Sepulcher of the Damned
	constexpr uint16 GUKF = 254; // Deepest Guk: Chapel of the Witnesses
	constexpr uint16 RUJF = 255; // The Rujarkian Hills: Fortified Lair of the Taskmasters
	constexpr uint16 TAKF = 256; // Takish-Hiz: Sandfall Corridors
	constexpr uint16 MIRF = 257; // The Forgotten Wastes
	constexpr uint16 MMCF = 258; // Mistmoore's Catacombs: Scion Lair of Fury
	constexpr uint16 GUKG = 259; // The Root Garden
	constexpr uint16 RUJG = 260; // The Rujarkian Hills: Hidden Vale of Deceit
	constexpr uint16 TAKG = 261; // Takish-Hiz: Balancing Chamber
	constexpr uint16 MIRG = 262; // Miragul's Menagerie: Heart of the Menagerie
	constexpr uint16 MMCG = 263; // Mistmoore's Catacombs: Cesspits of Putrescence
	constexpr uint16 GUKH = 264; // Deepest Guk: Accursed Sanctuary
	constexpr uint16 RUJH = 265; // The Rujarkian Hills: Blazing Forge
	constexpr uint16 TAKH = 266; // Takish-Hiz: Sweeping Tides
	constexpr uint16 MIRH = 267; // The Morbid Laboratory
	constexpr uint16 MMCH = 268; // Mistmoore's Catacombs: Aisles of Blood
	constexpr uint16 RUJI = 269; // The Rujarkian Hills: Arena of Chance
	constexpr uint16 TAKI = 270; // Takish-Hiz: Antiquated Palace
	constexpr uint16 MIRI = 271; // The Theater of Imprisoned Horror
	constexpr uint16 MMCI = 272; // Mistmoore's Catacombs: Halls of Sanguinary Rites
	constexpr uint16 RUJJ = 273; // The Rujarkian Hills: Barracks of War
	constexpr uint16 TAKJ = 274; // Takish-Hiz: Prismatic Corridors
	constexpr uint16 MIRJ = 275; // Miragul's Menagerie: Grand Library
	constexpr uint16 MMCJ = 276; // Mistmoore's Catacombs: Infernal Sanctuary
	constexpr uint16 CHARDOKB = 277; // Chardok: The Halls of Betrayal
	constexpr uint16 SOLDUNGC = 278; // The Caverns of Exile
	constexpr uint16 ABYSMAL = 279; // The Abysmal Sea
	constexpr uint16 NATIMBI = 280; // Natimbi, the Broken Shores
	constexpr uint16 QINIMI = 281; // Qinimi, Court of Nihilia
	constexpr uint16 RIWWI = 282; // Riwwi, Coliseum of Games
	constexpr uint16 BARINDU = 283; // Barindu, Hanging Gardens
	constexpr uint16 FERUBI = 284; // Ferubi, Forgotten Temple of Taelosia
	constexpr uint16 SNPOOL = 285; // Sewers of Nihilia, Pool of Sludg
	constexpr uint16 SNLAIR = 286; // Sewers of Nihilia, Lair of Trapp
	constexpr uint16 SNPLANT = 287; // Sewers of Nihilia, Purifying Pla
	constexpr uint16 SNCREMATORY = 288; // Sewers of Nihilia, Emanating Cre
	constexpr uint16 TIPT = 289; // Tipt, Treacherous Crags
	constexpr uint16 VXED = 290; // Vxed, the Crumbling Caverns
	constexpr uint16 YXTTA = 291; // Yxtta, Pulpit of Exiles
	constexpr uint16 UQUA = 292; // Uqua, the Ocean God Chantry
	constexpr uint16 KODTAZ = 293; // Kod'Taz, Broken Trial Grounds
	constexpr uint16 IKKINZ = 294; // Ikkinz, Chambers of Transcendence
	constexpr uint16 QVIC = 295; // Qvic, Prayer Grounds of Calling
	constexpr uint16 INKTUTA = 296; // Inktu'Ta, the Unmasked Chapel
	constexpr uint16 TXEVU = 297; // Txevu, Lair of the Elite
	constexpr uint16 TACVI = 298; // Tacvi, The Broken Temple
	constexpr uint16 QVICB = 299; // Qvic, the Hidden Vault
	constexpr uint16 WALLOFSLAUGHTER = 300; // Wall of Slaughter
	constexpr uint16 BLOODFIELDS = 301; // The Bloodfields
	constexpr uint16 DRANIKSSCAR = 302; // Dranik's Scar
	constexpr uint16 CAUSEWAY = 303; // Nobles' Causeway
	constexpr uint16 CHAMBERSA = 304; // Muramite Proving Grounds
	constexpr uint16 CHAMBERSB = 305; // Muramite Proving Grounds
	constexpr uint16 CHAMBERSC = 306; // Muramite Proving Grounds
	constexpr uint16 CHAMBERSD = 307; // Muramite Proving Grounds
	constexpr uint16 CHAMBERSE = 308; // Muramite Proving Grounds
	constexpr uint16 CHAMBERSF = 309; // Muramite Proving Grounds
	constexpr uint16 PROVINGGROUNDS = 316; // Muramite Proving Grounds
	constexpr uint16 ANGUISH = 317; // Anguish, the Fallen Palace
	constexpr uint16 DRANIKHOLLOWSA = 318; // Dranik's Hollows
	constexpr uint16 DRANIKHOLLOWSB = 319; // Dranik's Hollows
	constexpr uint16 DRANIKHOLLOWSC = 320; // Dranik's Hollows
	constexpr uint16 DRANIKCATACOMBSA = 328; // Catacombs of Dranik
	constexpr uint16 DRANIKCATACOMBSB = 329; // Catacombs of Dranik
	constexpr uint16 DRANIKCATACOMBSC = 330; // Catacombs of Dranik
	constexpr uint16 DRANIKSEWERSA = 331; // Sewers of Dranik
	constexpr uint16 DRANIKSEWERSB = 332; // Sewers of Dranik
	constexpr uint16 DRANIKSEWERSC = 333; // Sewers of Dranik
	constexpr uint16 RIFTSEEKERS = 334; // Riftseekers' Sanctum
	constexpr uint16 HARBINGERS = 335; // Harbinger's Spire
	constexpr uint16 DRANIK = 336; // The Ruined City of Dranik
	constexpr uint16 BROODLANDS = 337; // The Broodlands
	constexpr uint16 STILLMOONA = 338; // Stillmoon Temple
	constexpr uint16 STILLMOONB = 339; // The Ascent
	constexpr uint16 THUNDERCREST = 340; // Thundercrest Isles
	constexpr uint16 DELVEA = 341; // Lavaspinner's Lair
	constexpr uint16 DELVEB = 342; // Tirranun's Delve
	constexpr uint16 THENEST = 343; // The Nest
	constexpr uint16 GUILDLOBBY = 344; // Guild Lobby
	constexpr uint16 GUILDHALL = 345; // Guild Hall
	constexpr uint16 BARTER = 346; // The Barter Hall
	constexpr uint16 ILLSALIN = 347; // Ruins of Illsalin
	constexpr uint16 ILLSALINA = 348; // Illsalin Marketplace
	constexpr uint16 ILLSALINB = 349; // Temple of Korlach
	constexpr uint16 ILLSALINC = 350; // The Nargil Pits
	constexpr uint16 DREADSPIRE = 351; // Dreadspire Keep
	constexpr uint16 DRACHNIDHIVE = 354; // The Hive
	constexpr uint16 DRACHNIDHIVEA = 355; // The Hatchery
	constexpr uint16 DRACHNIDHIVEB = 356; // The Cocoons
	constexpr uint16 DRACHNIDHIVEC = 357; // Queen Sendaii`s Lair
	constexpr uint16 WESTKORLACH = 358; // Stoneroot Falls
	constexpr uint16 WESTKORLACHA = 359; // Prince's Manor
	constexpr uint16 WESTKORLACHB = 360; // Caverns of the Lost
	constexpr uint16 WESTKORLACHC = 361; // Lair of the Korlach
	constexpr uint16 EASTKORLACH = 362; // The Undershore
	constexpr uint16 EASTKORLACHA = 363; // Snarlstone Dens
	constexpr uint16 SHADOWSPINE = 364; // Shadow Spine
	constexpr uint16 CORATHUS = 365; // Corathus Creep
	constexpr uint16 CORATHUSA = 366; // Sporali Caverns
	constexpr uint16 CORATHUSB = 367; // The Corathus Mines
	constexpr uint16 NEKTULOSA = 368; // Shadowed Grove
	constexpr uint16 ARCSTONE = 369; // Arcstone, Isle of Spirits
	constexpr uint16 RELIC = 370; // Relic, the Artifact City
	constexpr uint16 SKYLANCE = 371; // Skylance
	constexpr uint16 DEVASTATION = 372; // The Devastation
	constexpr uint16 DEVASTATIONA = 373; // The Seething Wall
	constexpr uint16 RAGE = 374; // Sverag, Stronghold of Rage
	constexpr uint16 RAGEA = 375; // Razorthorn, Tower of Sullon Zek
	constexpr uint16 TAKISHRUINS = 376; // Ruins of Takish-Hiz
	constexpr uint16 TAKISHRUINSA = 377; // The Root of Ro
	constexpr uint16 ELDDAR = 378; // The Elddar Forest
	constexpr uint16 ELDDARA = 379; // Tunare's Shrine
	constexpr uint16 THEATER = 380; // Theater of Blood
	constexpr uint16 THEATERA = 381; // Deathknell, Tower of Dissonance
	constexpr uint16 FREEPORTEAST = 382; // East Freeport
	constexpr uint16 FREEPORTWEST = 383; // West Freeport
	constexpr uint16 FREEPORTSEWERS = 384; // Freeport Sewers
	constexpr uint16 FREEPORTACADEMY = 385; // Academy of Arcane Sciences
	constexpr uint16 FREEPORTTEMPLE = 386; // Temple of Marr
	constexpr uint16 FREEPORTMILITIA = 387; // Freeport Militia House: My Precious
	constexpr uint16 FREEPORTARENA = 388; // Arena
	constexpr uint16 FREEPORTCITYHALL = 389; // City Hall
	constexpr uint16 FREEPORTTHEATER = 390; // Theater of the Tranquil
	constexpr uint16 FREEPORTHALL = 391; // Hall of Truth: Bounty
	constexpr uint16 NORTHRO = 392; // North Desert of Ro
	constexpr uint16 SOUTHRO = 393; // South Desert of Ro
	constexpr uint16 CRESCENT = 394; // Crescent Reach
	constexpr uint16 MOORS = 395; // Blightfire Moors
	constexpr uint16 STONEHIVE = 396; // Stone Hive
	constexpr uint16 MESA = 397; // Goru`kar Mesa
	constexpr uint16 ROOST = 398; // Blackfeather Roost
	constexpr uint16 STEPPES = 399; // The Steppes
	constexpr uint16 ICEFALL = 400; // Icefall Glacier
	constexpr uint16 VALDEHOLM = 401; // Valdeholm
	constexpr uint16 FROSTCRYPT = 402; // Frostcrypt, Throne of the Shade King
	constexpr uint16 SUNDEROCK = 403; // Sunderock Springs
	constexpr uint16 VERGALID = 404; // Vergalid Mines
	constexpr uint16 DIREWIND = 405; // Direwind Cliffs
	constexpr uint16 ASHENGATE = 406; // Ashengate, Reliquary of the Scale
	constexpr uint16 HIGHPASSHOLD = 407; // Highpass Hold
	constexpr uint16 COMMONLANDS = 408; // The Commonlands
	constexpr uint16 OCEANOFTEARS = 409; // The Ocean of Tears
	constexpr uint16 KITHFOREST = 410; // Kithicor Forest
	constexpr uint16 BEFALLENB = 411; // Befallen
	constexpr uint16 HIGHPASSKEEP = 412; // HighKeep
	constexpr uint16 INNOTHULEB = 413; // The Innothule Swamp
	constexpr uint16 TOXXULIA = 414; // Toxxulia Forest
	constexpr uint16 MISTYTHICKET = 415; // The Misty Thicket
	constexpr uint16 KATTACASTRUM = 416; // Katta Castrum
	constexpr uint16 THALASSIUS = 417; // Thalassius, the Coral Keep
	constexpr uint16 ATIIKI = 418; // Jewel of Atiiki
	constexpr uint16 ZHISZA = 419; // Zhisza, the Shissar Sanctuary
	constexpr uint16 SILYSSAR = 420; // Silyssar, New Chelsith
	constexpr uint16 SOLTERIS = 421; // Solteris, the Throne of Ro
	constexpr uint16 BARREN = 422; // Barren Coast
	constexpr uint16 BURIEDSEA = 423; // The Buried Sea
	constexpr uint16 JARDELSHOOK = 424; // Jardel's Hook
	constexpr uint16 MONKEYROCK = 425; // Monkey Rock
	constexpr uint16 SUNCREST = 426; // Suncrest Isle
	constexpr uint16 DEADBONE = 427; // Deadbone Reef
	constexpr uint16 BLACKSAIL = 428; // Blacksail Folly
	constexpr uint16 MAIDENSGRAVE = 429; // Maiden's Grave
	constexpr uint16 REDFEATHER = 430; // Redfeather Isle
	constexpr uint16 SHIPMVP = 431; // The Open Sea
	constexpr uint16 SHIPMVU = 432; // The Open Sea
	constexpr uint16 SHIPPVU = 433; // The Open Sea
	constexpr uint16 SHIPUVU = 434; // The Open Sea
	constexpr uint16 SHIPMVM = 435; // The Open Sea
	constexpr uint16 MECHANOTUS = 436; // Fortress Mechanotus
	constexpr uint16 MANSION = 437; // Meldrath's Majestic Mansion
	constexpr uint16 STEAMFACTORY = 438; // The Steam Factory
	constexpr uint16 SHIPWORKSHOP = 439; // S.H.I.P. Workshop
	constexpr uint16 GYROSPIREB = 440; // Gyrospire Beza
	constexpr uint16 GYROSPIREZ = 441; // Gyrospire Zeka
	constexpr uint16 DRAGONSCALE = 442; // Dragonscale Hills
	constexpr uint16 LOPINGPLAINS = 443; // Loping Plains
	constexpr uint16 HILLSOFSHADE = 444; // Hills of Shade
	constexpr uint16 BLOODMOON = 445; // Bloodmoon Keep
	constexpr uint16 CRYSTALLOS = 446; // Crystallos, Lair of the Awakened
	constexpr uint16 GUARDIAN = 447; // The Mechamatic Guardian
	constexpr uint16 STEAMFONTMTS = 448; // The Steamfont Mountains
	constexpr uint16 CRYPTOFSHADE = 449; // Crypt of Shade
	constexpr uint16 DRAGONSCALEB = 451; // Deepscar's Den
	constexpr uint16 OLDFIELDOFBONE = 452; // Field of Scale
	constexpr uint16 OLDKAESORAA = 453; // Kaesora Library
	constexpr uint16 OLDKAESORAB = 454; // Kaesora Hatchery
	constexpr uint16 OLDKURN = 455; // Kurn's Tower
	constexpr uint16 OLDKITHICOR = 456; // Bloody Kithicor
	constexpr uint16 OLDCOMMONS = 457; // Old Commonlands
	constexpr uint16 OLDHIGHPASS = 458; // Highpass Hold
	constexpr uint16 THEVOIDA = 459; // The Void
	constexpr uint16 THEVOIDB = 460; // The Void
	constexpr uint16 THEVOIDC = 461; // The Void
	constexpr uint16 THEVOIDD = 462; // The Void
	constexpr uint16 THEVOIDE = 463; // The Void
	constexpr uint16 THEVOIDF = 464; // The Void
	constexpr uint16 THEVOIDG = 465; // The Void
	constexpr uint16 OCEANGREENHILLS = 466; // Oceangreen Hills
	constexpr uint16 OCEANGREENVILLAGE = 467; // Oceangreen Village
	constexpr uint16 OLDBLACKBURROW = 468; // BlackBurrow
	constexpr uint16 BERTOXTEMPLE = 469; // Temple of Bertoxxulous
	constexpr uint16 DISCORD = 470; // Korafax, Home of the Riders
	constexpr uint16 DISCORDTOWER = 471; // Citadel of the Worldslayer
	constexpr uint16 OLDBLOODFIELD = 472; // Old Bloodfields
	constexpr uint16 PRECIPICEOFWAR = 473; // The Precipice of War
	constexpr uint16 OLDDRANIK = 474; // City of Dranik
	constexpr uint16 TOSKIRAKK = 475; // Toskirakk
	constexpr uint16 KORASCIAN = 476; // Korascian Warrens
	constexpr uint16 RATHECHAMBER = 477; // Rathe Council Chamber
	constexpr uint16 BRELLSREST = 480; // Brell's Rest
	constexpr uint16 FUNGALFOREST = 481; // Fungal Forest
	constexpr uint16 UNDERQUARRY = 482; // The Underquarry
	constexpr uint16 COOLINGCHAMBER = 483; // The Cooling Chamber
	constexpr uint16 SHININGCITY = 484; // Kernagir, the Shining City
	constexpr uint16 ARTHICREX = 485; // Arthicrex
	constexpr uint16 FOUNDATION = 486; // The Foundation
	constexpr uint16 LICHENCREEP = 487; // Lichen Creep
	constexpr uint16 PELLUCID = 488; // Pellucid Grotto
	constexpr uint16 STONESNAKE = 489; // Volska's Husk
	constexpr uint16 BRELLSTEMPLE = 490; // Brell's Temple
	constexpr uint16 CONVORTEUM = 491; // The Convorteum
	constexpr uint16 BRELLSARENA = 492; // Brell's Arena
	constexpr uint16 WEDDINGCHAPEL = 493; // Wedding Chapel
	constexpr uint16 WEDDINGCHAPELDARK = 494; // Wedding Chapel
	constexpr uint16 DRAGONCRYPT = 495; // Lair of the Risen
	constexpr uint16 FEERROTT2 = 700; // The Feerrott
	constexpr uint16 THULEHOUSE1 = 701; // House of Thule
	constexpr uint16 THULEHOUSE2 = 702; // House of Thule, Upper Floors
	constexpr uint16 HOUSEGARDEN = 703; // The Grounds
	constexpr uint16 THULELIBRARY = 704; // The Library
	constexpr uint16 WELL = 705; // The Well
	constexpr uint16 FALLEN = 706; // Erudin Burning
	constexpr uint16 MORELLCASTLE = 707; // Morell's Castle
	constexpr uint16 SOMNIUM = 708; // Sanctum Somnium
	constexpr uint16 ALKABORMARE = 709; // Al'Kabor's Nightmare
	constexpr uint16 MIRAGULMARE = 710; // Miragul's Nightmare
	constexpr uint16 THULEDREAM = 711; // Fear Itself
	constexpr uint16 NEIGHBORHOOD = 712; // Sunrise Hills
	constexpr uint16 ARGATH = 724; // Argath, Bastion of Illdaera
	constexpr uint16 ARELIS = 725; // Valley of Lunanyn
	constexpr uint16 SARITHCITY = 726; // Sarith, City of Tides
	constexpr uint16 RUBAK = 727; // Rubak Oseka, Temple of the Sea
	constexpr uint16 BEASTDOMAIN = 728; // Beasts' Domain
	constexpr uint16 RESPLENDENT = 729; // The Resplendent Temple
	constexpr uint16 PILLARSALRA = 730; // Pillars of Alra
	constexpr uint16 WINDSONG = 731; // Windsong Sanctuary
	constexpr uint16 CITYOFBRONZE = 732; // Erillion, City of Bronze
	constexpr uint16 SEPULCHER = 733; // Sepulcher of Order
	constexpr uint16 EASTSEPULCHER = 734; // Sepulcher East
	constexpr uint16 WESTSEPULCHER = 735; // Sepulcher West
	constexpr uint16 SHARDSLANDING = 752; // Shard's Landing
	constexpr uint16 XORBB = 753; // Valley of King Xorbb
	constexpr uint16 KAELSHARD = 754; // Kael Drakkel: The King's Madness
	constexpr uint16 EASTWASTESSHARD = 755; // East Wastes: Zeixshi-Kar's Awakening
	constexpr uint16 CRYSTALSHARD = 756; // The Crystal Caverns: Fragment of Fear
	constexpr uint16 BREEDINGGROUNDS = 757; // The Breeding Grounds
	constexpr uint16 EVILTREE = 758; // Evantil, the Vile Oak
	constexpr uint16 GRELLETH = 759; // Grelleth's Palace, the Chateau of Filth
	constexpr uint16 CHAPTERHOUSE = 760; // Chapterhouse of the Fallen
	constexpr uint16 ARTTEST = 996; // Art Testing Domain
	constexpr uint16 FHALLS = 998; // The Forgotten Halls
	constexpr uint16 APPRENTICE = 999; // Designer Apprentice
}

namespace Language {
	constexpr uint8 CommonTongue  = 0;
	constexpr uint8 Barbarian     = 1;
	constexpr uint8 Erudian       = 2;
	constexpr uint8 Elvish        = 3;
	constexpr uint8 DarkElvish    = 4;
	constexpr uint8 Dwarvish      = 5;
	constexpr uint8 Troll         = 6;
	constexpr uint8 Ogre          = 7;
	constexpr uint8 Gnomish       = 8;
	constexpr uint8 Halfling      = 9;
	constexpr uint8 ThievesCant   = 10;
	constexpr uint8 OldErudian    = 11;
	constexpr uint8 ElderElvish   = 12;
	constexpr uint8 Froglok       = 13;
	constexpr uint8 Goblin        = 14;
	constexpr uint8 Gnoll         = 15;
	constexpr uint8 CombineTongue = 16;
	constexpr uint8 ElderTeirDal  = 17;
	constexpr uint8 Lizardman     = 18;
	constexpr uint8 Orcish        = 19;
	constexpr uint8 Faerie        = 20;
	constexpr uint8 Dragon        = 21;
	constexpr uint8 ElderDragon   = 22;
	constexpr uint8 DarkSpeech    = 23;
	constexpr uint8 VahShir       = 24;
	constexpr uint8 Alaran        = 25;
	constexpr uint8 Hadal         = 26;
	constexpr uint8 Unknown27     = 27;

	constexpr uint8 MaxValue = 100;
}
namespace PetInfoType {
	constexpr int Current   = 0;
	constexpr int Suspended = 1;
}

namespace BuffEffectType {
	constexpr uint8 None        = 0;
	constexpr uint8 Buff        = 2;
	constexpr uint8 InverseBuff = 4;
}

namespace AlternateCurrencyMode {
	constexpr uint32 Update   = 7;
	constexpr uint32 Populate = 8;
}

typedef enum {
	FilterNone = 0,
	FilterGuildChat = 1,		//0=hide, 1=show
	FilterSocials = 2,			//0=hide, 1=show
	FilterGroupChat = 3,		//0=hide, 1=show
	FilterShouts = 4,			//0=hide, 1=show
	FilterAuctions = 5,			//0=hide, 1=show
	FilterOOC = 6,				//0=hide, 1=show
	FilterBadWords = 7,			//0=hide, 1=show
	FilterPCSpells = 8,			//0=show, 1=hide, 2=group only
	FilterNPCSpells = 9,		//0=show, 1=hide
	FilterBardSongs = 10,		//0=show, 1=mine only, 2=group only, 3=hide
	FilterSpellCrits = 11,		//0=show, 1=mine only, 2=hide
	FilterMeleeCrits = 12,		//0=show, 1=hide
	FilterSpellDamage = 13,		//0=show, 1=mine only, 2=hide
	FilterMyMisses = 14,		//0=hide, 1=show
	FilterOthersMiss = 15,		//0=hide, 1=show
	FilterOthersHit = 16,		//0=hide, 1=show
	FilterMissedMe = 17,		//0=hide, 1=show
	FilterDamageShields = 18,	//0=show, 1=hide
	FilterDOT = 19,				//0=show, 1=hide
	FilterPetHits = 20,			//0=show, 1=hide
	FilterPetMisses = 21,		//0=show, 1=hide
	FilterFocusEffects = 22,	//0=show, 1=hide
	FilterPetSpells = 23,		//0=show, 1=hide
	FilterHealOverTime = 24,	//0=show, 1=mine only, 2=hide
	FilterItemSpeech = 25,		//0=show, 1=hide // RoF2 Confirmed
	FilterStrikethrough = 26,	//0=show, 1=hide // RoF2 Confirmed
	FilterStuns = 27,			//0=show, 1=hide // RoF2 Confirmed
	FilterBardSongsOnPets = 28,	//0=show, 1=hide // RoF2 Confirmed
	_FilterCount
} eqFilterType;

typedef enum {
	FilterHide,
	FilterShow,
	FilterShowGroupOnly,
	FilterShowSelfOnly
} eqFilterMode;

#define STAT_STR                 0
#define STAT_STA                 1
#define STAT_AGI                 2
#define STAT_DEX                 3
#define STAT_INT                 4
#define STAT_WIS                 5
#define STAT_CHA                 6
#define STAT_MAGIC               7
#define STAT_COLD                8
#define STAT_FIRE                9
#define STAT_POISON              10
#define STAT_DISEASE             11
#define STAT_MANA                12
#define STAT_HP                  13
#define STAT_AC                  14
#define STAT_ENDURANCE           15
#define STAT_ATTACK              16
#define STAT_HP_REGEN            17
#define STAT_MANA_REGEN          18
#define STAT_HASTE               19
#define STAT_DAMAGE_SHIELD       20
#define STAT_DS_MITIGATION       22
#define	STAT_HEAL_AMOUNT         23
#define	STAT_SPELL_DAMAGE        24
#define	STAT_CLAIRVOYANCE        25
#define	STAT_HEROIC_AGILITY      26
#define	STAT_HEROIC_CHARISMA     27
#define	STAT_HEROIC_DEXTERITY    28
#define	STAT_HEROIC_INTELLIGENCE 29
#define	STAT_HEROIC_STAMINA      30
#define	STAT_HEROIC_STRENGTH     31
#define	STAT_HEROIC_WISDOM       32
#define	STAT_BASH                33
#define	STAT_BACKSTAB            34
#define	STAT_DRAGON_PUNCH        35
#define	STAT_EAGLE_STRIKE        36
#define	STAT_FLYING_KICK         37
#define STAT_KICK                38
#define	STAT_ROUND_KICK          39
#define STAT_TIGER_CLAW          40
#define STAT_FRENZY              41

static const uint8 DamageTypeSomething	= 0x1C;	//0x1c is something...
static const uint8 DamageTypeFalling	= 0xFC;
static const uint8 DamageTypeSpell		= 0xE7;
static const uint8 DamageTypeUnknown	= 0xFF;

/*
**	Skill damage types
**
**	(indexed by 'Skill' of SkillUseTypes)
*/
static const uint8 SkillDamageTypes[EQ::skills::HIGHEST_SKILL + 1] = // change to _SkillServerArraySize once activated
{
/*1HBlunt*/					0,
/*1HSlashing*/				1,
/*2HBlunt*/					0,
/*2HSlashing*/				1,
/*Abjuration*/				DamageTypeSpell,
/*Alteration*/				DamageTypeSpell,
/*ApplyPoison*/				DamageTypeUnknown,
/*Archery*/					7,
/*Backstab*/				8,
/*BindWound*/				DamageTypeUnknown,
/*Bash*/					10,
/*Block*/					DamageTypeUnknown,
/*BrassInstruments*/		DamageTypeSpell,
/*Channeling*/				DamageTypeUnknown,
/*Conjuration*/				DamageTypeSpell,
/*Defense*/					DamageTypeUnknown,
/*Disarm*/					DamageTypeUnknown,
/*DisarmTraps*/				DamageTypeUnknown,
/*Divination*/				DamageTypeSpell,
/*Dodge*/					DamageTypeUnknown,
/*DoubleAttack*/			DamageTypeUnknown,
/*DragonPunch*/				21,
/*DualWield*/				DamageTypeUnknown,
/*EagleStrike*/				23,
/*Evocation*/				DamageTypeSpell,
/*FeignDeath*/				4,
/*FlyingKick*/				30,
/*Forage*/					DamageTypeUnknown,
/*HandtoHand*/				4,
/*Hide*/					DamageTypeUnknown,
/*Kick*/					30,
/*Meditate*/				DamageTypeUnknown,
/*Mend*/					DamageTypeUnknown,
/*Offense*/					DamageTypeUnknown,
/*Parry*/					DamageTypeUnknown,
/*PickLock*/				DamageTypeUnknown,
/*1HPiercing*/				36,
/*Riposte*/					DamageTypeUnknown,
/*RoundKick*/				30,
/*SafeFall*/				DamageTypeUnknown,
/*SsenseHeading*/			DamageTypeUnknown,
/*Singing*/					DamageTypeSpell,
/*Sneak*/					DamageTypeUnknown,
/*SpecializeAbjure*/		DamageTypeUnknown,
/*SpecializeAlteration*/	DamageTypeUnknown,
/*SpecializeConjuration*/	DamageTypeUnknown,
/*SpecializeDivination*/	DamageTypeUnknown,
/*SpecializeEvocation*/		DamageTypeUnknown,
/*PickPockets*/				DamageTypeUnknown,
/*StringedInstruments*/		DamageTypeSpell,
/*Swimming*/				DamageTypeUnknown,
/*Throwing*/				51,
/*TigerClaw*/				23,
/*Tracking*/				DamageTypeUnknown,
/*WindInstruments*/			DamageTypeSpell,
/*Fishing*/					DamageTypeUnknown,
/*MakePoison*/				DamageTypeUnknown,
/*Tinkering*/				DamageTypeUnknown,
/*Research*/				DamageTypeUnknown,
/*Alchemy*/					DamageTypeUnknown,
/*Baking*/					DamageTypeUnknown,
/*Tailoring*/				DamageTypeUnknown,
/*SenseTraps*/				DamageTypeUnknown,
/*Blacksmithing*/			DamageTypeUnknown,
/*Fletching*/				DamageTypeUnknown,
/*Brewing*/					DamageTypeUnknown,
/*AlcoholTolerance*/		DamageTypeUnknown,
/*Begging*/					DamageTypeUnknown,
/*JewelryMaking*/			DamageTypeUnknown,
/*Pottery*/					DamageTypeUnknown,
/*PercussionInstruments*/	DamageTypeSpell,
/*Intimidation*/			DamageTypeUnknown,
/*Berserking*/				DamageTypeUnknown,
/*Taunt*/					DamageTypeUnknown,
/*Frenzy*/					74,
/*RemoveTrap*/				DamageTypeUnknown,	// Needs research (set for SenseTrap value)
/*TripleAttack*/			DamageTypeUnknown,	// Needs research (set for DoubleAttack value)
/*2HPiercing*/				36					// Needs research (set for 1HPiercing value - similar to slash/blunt)
};

/*
// Used for worn NPC inventory tracking. NPCs don't use
// augments, so only the basic slots need to be kept track of.
#define MAX_WORN_INVENTORY	22
*/

/*
**	Inventory Slot Equipment Enum
**	Mostly used for third-party tools to reference inventory slots
**
**	[pre-HoT]
**	NOTE: Numbering for personal inventory goes top to bottom, then left to right
**	It's the opposite for inside bags: left to right, then top to bottom
**	Example:
**	Inventory:	Containers:
**	1	5		1	2
**	2	6		3	4
**	3	7		5	6
**	4	8		7	8
**	-	-		9	10
**
**	[HoT and Higher]
**	Note: Numbering for inventory and bags goes left to right, then top to bottom
**	Example:
**	Inventory:	Containers:
**	1	2		1	2
**	3	4		3	4
**	5	6		5	6
**	7	8		7	8
**	9	10		9	10
**	-	-		11	12	[Note: Additional slots are only available in RoF and higher]
**
*/

#define INVALID_INDEX	-1

static const uint32 MAX_SPELL_DB_ID_VAL = 65535;

static const uint32 DB_FACTION_GEM_CHOPPERS = 255;
static const uint32 DB_FACTION_HERETICS = 265;
static const uint32 DB_FACTION_KING_AKANON = 333;

enum ChatChannelNames : uint16
{
	ChatChannel_Guild = 0,
	ChatChannel_Group = 2,
	ChatChannel_Shout = 3,
	ChatChannel_Auction = 4,
	ChatChannel_OOC = 5,
	ChatChannel_Broadcast = 6,
	ChatChannel_Tell = 7,
	ChatChannel_Say = 8,
	ChatChannel_Petition = 10,
	ChatChannel_GMSAY = 11,
	ChatChannel_TellEcho = 14,
	ChatChannel_Raid = 15,

	ChatChannel_UNKNOWN_Guild = 17,
	ChatChannel_UNKNOWN_GMSAY = 18,
	ChatChannel_UCSRelay = 20,
	ChatChannel_Emotes = 22
};

namespace ZoneBlockedSpellTypes {
	const uint8 ZoneWide = 1;
	const uint8 Region   = 2;
};

enum StartZoneIndex {
    Odus = 0,
    Qeynos,
    Halas,
    Rivervale,
    Freeport,
    Neriak,
    Grobb,
    Oggok,
    Kaladim,
    GreaterFaydark,
    Felwithe,
    Akanon,
    Cabilis,
    SharVahl,
    RatheMtn
};

enum FVNoDropFlagRule
{
	Disabled = 0,
	Enabled = 1,
	AdminOnly = 2
};

enum Anonymity : uint8
{
	NotAnonymous,
	Anonymous,
	Roleplaying
};

enum ZoningMessage : int8 {
	ZoneNoMessage    = 0,
	ZoneSuccess      = 1,
	ZoneNotReady     = -1,
	ZoneValidPC      = -2,
	ZoneStoryZone    = -3,
	ZoneNoExpansion  = -6,
	ZoneNoExperience = -7
};

enum class RecipeCountType : uint8
{
	Component,
	Container,
	Fail,
	Salvage,
	Success
};

#define ALT_CURRENCY_ID_RADIANT 4
#define ALT_CURRENCY_ID_EBON 5

enum ResurrectionActions
{
	Decline,
	Accept
};

enum ScribeSpellActions
{
	Scribe,
	Memorize,
	Unmemorize
};

enum SpellTimeRestrictions
{
	NoRestriction,
	Day,
	Night
};

enum MoneyTypes
{
	Copper,
	Silver,
	Gold,
	Platinum
};

enum MoneySubtypes
{
	Personal,
	Bank,
	Cursor,
	SharedBank // Platinum Only
};

namespace RaidLootType {
	constexpr uint32 LeaderOnly                = 1;
	constexpr uint32 LeaderAndGroupLeadersOnly = 2;
	constexpr uint32 LeaderSelected            = 3;
	constexpr uint32 EntireRaid                = 4;
}

namespace LeadershipAbilitySlot {
	constexpr uint16 HealthOfTargetsTarget = 14;
}

enum ExpSource
{
	Quest,
	GM,
	Kill,
	Death,
	Resurrection,
	LDoNChest,
	Task,
	Sacrifice
};

#define PARCEL_SEND_ITEMS    0
#define PARCEL_SEND_MONEY    1
#define PARCEL_MONEY_ITEM_ID 99990 // item id of money
#define PARCEL_LIMIT         5
#define PARCEL_BEGIN_SLOT    1

namespace DoorType {
	constexpr uint32 BuyerStall = 155;
}
