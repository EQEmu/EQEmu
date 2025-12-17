/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

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

#include "common/types.h"

#include <map>
#include <string>

// body types above 64 make the mob invisible
namespace BodyType {
	constexpr uint8 Humanoid       = 1;
	constexpr uint8 Lycanthrope    = 2;
	constexpr uint8 Undead         = 3;
	constexpr uint8 Giant          = 4;
	constexpr uint8 Construct      = 5;
	constexpr uint8 Extraplanar    = 6;
	constexpr uint8 Magical        = 7;     // this name might be a bit off,
	constexpr uint8 SummonedUndead = 8;
	constexpr uint8 RaidGiant      = 9;     // Velious era Raid Giant
	constexpr uint8 RaidColdain    = 10;    // Velious era Raid Coldain
	constexpr uint8 NoTarget       = 11;    // no name, can't target this bodytype
	constexpr uint8 Vampire        = 12;
	constexpr uint8 AtenHaRa       = 13;
	constexpr uint8 GreaterAkheva  = 14;
	constexpr uint8 KhatiSha       = 15;
	constexpr uint8 Seru           = 16;
	constexpr uint8 GriegVeneficus = 17;
	constexpr uint8 DrazNurakk     = 18;
	constexpr uint8 Zek            = 19;    //"creatures from the Plane of War."
	constexpr uint8 Luggald        = 20;
	constexpr uint8 Animal         = 21;
	constexpr uint8 Insect         = 22;
	constexpr uint8 Monster        = 23;
	constexpr uint8 Summoned       = 24;    // Elemental?
	constexpr uint8 Plant          = 25;
	constexpr uint8 Dragon         = 26;
	constexpr uint8 Summoned2      = 27;
	constexpr uint8 Summoned3      = 28;
	constexpr uint8 Dragon2        = 29;    // database data indicates this is a dragon type (Kunark and DoN?)
	constexpr uint8 VeliousDragon  = 30;    // might not be a tight set
	constexpr uint8 Familiar       = 31;
	constexpr uint8 Dragon3        = 32;
	constexpr uint8 Boxes          = 33;
	constexpr uint8 Muramite       = 34;    // tribal dudes
	constexpr uint8 NoTarget2      = 60;
	constexpr uint8 SwarmPet       = 63;    // Looks like weapon proc related temp pets and few misc pets, should not be used for checking swarm pets in general.
	constexpr uint8 MonsterSummon  = 64;
	constexpr uint8 InvisibleMan   = 66;    // no name, seen on 'InvisMan', can be /targeted
	constexpr uint8 Special        = 67;

	std::string GetName(uint8 body_type_id);
	bool IsValid(uint8 body_type_id);
}

static std::map<uint8, std::string> body_type_names = {
	{ BodyType::Humanoid,       "Humanoid" },
	{ BodyType::Lycanthrope,    "Lycanthrope" },
	{ BodyType::Undead,         "Undead" },
	{ BodyType::Giant,          "Giant" },
	{ BodyType::Construct,      "Construct" },
	{ BodyType::Extraplanar,    "Extraplanar" },
	{ BodyType::Magical,        "Magical" },
	{ BodyType::SummonedUndead, "Summoned Undead" },
	{ BodyType::RaidGiant,      "Raid Giant" },
	{ BodyType::RaidColdain,    "Raid Coldain" },
	{ BodyType::NoTarget,       "Untargetable" },
	{ BodyType::Vampire,        "Vampire" },
	{ BodyType::AtenHaRa,       "Aten Ha Ra" },
	{ BodyType::GreaterAkheva,  "Greater Akheva" },
	{ BodyType::KhatiSha,       "Khati Sha" },
	{ BodyType::Seru,           "Seru" },
	{ BodyType::GriegVeneficus, "Grieg Veneficus" },
	{ BodyType::DrazNurakk,     "Draz Nurakk" },
	{ BodyType::Zek,            "Zek" },
	{ BodyType::Luggald,        "Luggald" },
	{ BodyType::Animal,         "Animal" },
	{ BodyType::Insect,         "Insect" },
	{ BodyType::Monster,        "Monster" },
	{ BodyType::Summoned,       "Summoned" },
	{ BodyType::Plant,          "Plant" },
	{ BodyType::Dragon,         "Dragon" },
	{ BodyType::Summoned2,      "Summoned 2" },
	{ BodyType::Summoned3,      "Summoned 3" },
	{ BodyType::Dragon2,        "Dragon 2" },
	{ BodyType::VeliousDragon,  "Velious Dragon" },
	{ BodyType::Familiar,       "Familiar" },
	{ BodyType::Dragon3,        "Dragon 3" },
	{ BodyType::Boxes,          "Boxes" },
	{ BodyType::Muramite,       "Muramite" },
	{ BodyType::NoTarget2,      "Untargetable 2" },
	{ BodyType::SwarmPet,       "Swarm Pet" },
	{ BodyType::MonsterSummon,  "Monster Summon" },
	{ BodyType::InvisibleMan,   "Invisible Man" },
	{ BodyType::Special,        "Special" },
};
