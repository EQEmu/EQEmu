/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemu.org)

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

#include "common/rulesys.h"
#include "common/types.h"

#include <map>
#include <string>

namespace Class {
	constexpr uint8 None                      = 0;
	constexpr uint8 Warrior                   = 1;
	constexpr uint8 Cleric                    = 2;
	constexpr uint8 Paladin                   = 3;
	constexpr uint8 Ranger                    = 4;
	constexpr uint8 ShadowKnight              = 5;
	constexpr uint8 Druid                     = 6;
	constexpr uint8 Monk                      = 7;
	constexpr uint8 Bard                      = 8;
	constexpr uint8 Rogue                     = 9;
	constexpr uint8 Shaman                    = 10;
	constexpr uint8 Necromancer               = 11;
	constexpr uint8 Wizard                    = 12;
	constexpr uint8 Magician                  = 13;
	constexpr uint8 Enchanter                 = 14;
	constexpr uint8 Beastlord                 = 15;
	constexpr uint8 Berserker                 = 16;
	constexpr uint8 WarriorGM                 = 20;
	constexpr uint8 ClericGM                  = 21;
	constexpr uint8 PaladinGM                 = 22;
	constexpr uint8 RangerGM                  = 23;
	constexpr uint8 ShadowKnightGM            = 24;
	constexpr uint8 DruidGM                   = 25;
	constexpr uint8 MonkGM                    = 26;
	constexpr uint8 BardGM                    = 27;
	constexpr uint8 RogueGM                   = 28;
	constexpr uint8 ShamanGM                  = 29;
	constexpr uint8 NecromancerGM             = 30;
	constexpr uint8 WizardGM                  = 31;
	constexpr uint8 MagicianGM                = 32;
	constexpr uint8 EnchanterGM               = 33;
	constexpr uint8 BeastlordGM               = 34;
	constexpr uint8 BerserkerGM               = 35;
	constexpr uint8 Banker                    = 40;
	constexpr uint8 Merchant                  = 41;
	constexpr uint8 DiscordMerchant           = 59;
	constexpr uint8 AdventureRecruiter        = 60;
	constexpr uint8 AdventureMerchant         = 61;
	constexpr uint8 LDoNTreasure              = 62;
	constexpr uint8 TributeMaster             = 63;
	constexpr uint8 GuildTributeMaster        = 64;
	constexpr uint8 GuildBanker               = 66;
	constexpr uint8 NorrathsKeepersMerchant   = 67;
	constexpr uint8 DarkReignMerchant         = 68;
	constexpr uint8 FellowshipMaster          = 69;
	constexpr uint8 AlternateCurrencyMerchant = 70;
	constexpr uint8 MercenaryLiaison          = 71;

	constexpr uint8  PLAYER_CLASS_COUNT  = 16;
	constexpr uint16 ALL_CLASSES_BITMASK = 65535;
};

static std::map<uint8, uint16> player_class_bitmasks = {
	{Class::Warrior,      1},
	{Class::Cleric,       2},
	{Class::Paladin,      4},
	{Class::Ranger,       8},
	{Class::ShadowKnight, 16},
	{Class::Druid,        32},
	{Class::Monk,         64},
	{Class::Bard,         128},
	{Class::Rogue,        256},
	{Class::Shaman,       512},
	{Class::Necromancer,  1024},
	{Class::Wizard,       2048},
	{Class::Magician,     4096},
	{Class::Enchanter,    8192},
	{Class::Beastlord,    16384},
	{Class::Berserker,    32768},
};

static std::string shadow_knight_class_name = (
	RuleB(World, UseOldShadowKnightClassExport) ?
	"Shadowknight" :
	"Shadow Knight"
);

static std::map<uint8, std::string> class_names = {
	{Class::Warrior,      "Warrior"},
	{Class::Cleric,       "Cleric"},
	{Class::Paladin,      "Paladin"},
	{Class::Ranger,       "Ranger"},
	{Class::ShadowKnight, shadow_knight_class_name},
	{Class::Druid,        "Druid"},
	{Class::Monk,         "Monk"},
	{Class::Bard,         "Bard"},
	{Class::Rogue,        "Rogue"},
	{Class::Shaman,       "Shaman"},
	{Class::Necromancer,  "Necromancer"},
	{Class::Wizard,       "Wizard"},
	{Class::Magician,     "Magician"},
	{Class::Enchanter,    "Enchanter"},
	{Class::Beastlord,    "Beastlord"},
	{Class::Berserker,    "Berserker"},
};


#define ARMOR_TYPE_UNKNOWN 0
#define ARMOR_TYPE_CLOTH 1
#define ARMOR_TYPE_LEATHER 2
#define ARMOR_TYPE_CHAIN 3
#define ARMOR_TYPE_PLATE 4

#define ARMOR_TYPE_FIRST ARMOR_TYPE_UNKNOWN
#define ARMOR_TYPE_LAST ARMOR_TYPE_PLATE
#define ARMOR_TYPE_COUNT 5

#define BOT_CLASS_BASE_ID_PREFIX 3000


const char* GetClassIDName(uint8 class_id, uint8 level = 0);

bool IsPlayerClass(uint8 class_id);
const std::string GetPlayerClassAbbreviation(uint8 class_id);

uint8 GetPlayerClassValue(uint8 class_id);
uint16 GetPlayerClassBit(uint8 class_id);

bool IsFighterClass(uint8 class_id);
bool IsSpellFighterClass(uint8 class_id);
bool IsNonSpellFighterClass(uint8 class_id);
bool IsHybridClass(uint8 class_id);
bool IsCasterClass(uint8 class_id);
bool IsINTCasterClass(uint8 class_id);
bool IsWISCasterClass(uint8 class_id);
bool IsHeroicINTCasterClass(uint8 class_id);
bool IsHeroicWISCasterClass(uint8 class_id);
bool IsPlateClass(uint8 class_id);
bool IsChainClass(uint8 class_id);
bool IsLeatherClass(uint8 class_id);
bool IsClothClass(uint8 class_id);
uint8 ClassArmorType(uint8 class_id);
