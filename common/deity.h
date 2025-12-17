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
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#pragma once

#include "common/types.h"

#include <map>
#include <string>
#include <limits>

namespace Deity {
	constexpr uint32 Unknown       = 0;
	constexpr uint32 Agnostic1     = 140;
	constexpr uint32 Bertoxxulous  = 201;
	constexpr uint32 BrellSirilis  = 202;
	constexpr uint32 CazicThule    = 203;
	constexpr uint32 ErollisiMarr  = 204;
	constexpr uint32 Bristlebane   = 205;
	constexpr uint32 Innoruuk      = 206;
	constexpr uint32 Karana        = 207;
	constexpr uint32 MithanielMarr = 208;
	constexpr uint32 Prexus        = 209;
	constexpr uint32 Quellious     = 210;
	constexpr uint32 RallosZek     = 211;
	constexpr uint32 RodcetNife    = 212;
	constexpr uint32 SolusekRo     = 213;
	constexpr uint32 TheTribunal   = 214;
	constexpr uint32 Tunare        = 215;
	constexpr uint32 Veeshan       = 216;
	constexpr uint32 Agnostic2     = 396;

	namespace Bitmask {
		constexpr uint32 Agnostic      = 1;
		constexpr uint32 Bertoxxulous  = 2;
		constexpr uint32 BrellSirilis  = 4;
		constexpr uint32 CazicThule    = 8;
		constexpr uint32 ErollisiMarr  = 16;
		constexpr uint32 Bristlebane   = 32;
		constexpr uint32 Innoruuk      = 64;
		constexpr uint32 Karana        = 128;
		constexpr uint32 MithanielMarr = 256;
		constexpr uint32 Prexus        = 512;
		constexpr uint32 Quellious     = 1024;
		constexpr uint32 RallosZek     = 2048;
		constexpr uint32 RodcetNife    = 4096;
		constexpr uint32 SolusekRo     = 8192;
		constexpr uint32 TheTribunal   = 16384;
		constexpr uint32 Tunare        = 32768;
		constexpr uint32 Veeshan       = 65536;
		constexpr uint32 All           = std::numeric_limits<uint32>::max();
	}

	uint32 GetBitmask(uint32 deity_id);
	std::string GetName(uint32 deity_id);
	bool IsValid(uint32 deity_id);
}

static std::map<uint32, std::string> deity_names = {
	{ Deity::Agnostic1,     "Agnostic" },
	{ Deity::Agnostic2,     "Agnostic" },
	{ Deity::Bertoxxulous,  "Bertoxxulous" },
	{ Deity::BrellSirilis,  "Brell Serilis" },
	{ Deity::Bristlebane,   "Bristlebane" },
	{ Deity::CazicThule,    "Cazic-Thule" },
	{ Deity::ErollisiMarr,  "Erollisi Marr" },
	{ Deity::Innoruuk,      "Innoruuk" },
	{ Deity::Karana,        "Karana" },
	{ Deity::MithanielMarr, "Mithaniel Marr" },
	{ Deity::Prexus,        "Prexus" },
	{ Deity::Quellious,     "Quellious" },
	{ Deity::RallosZek,     "Rallos Zek" },
	{ Deity::RodcetNife,    "Rodcet Nife" },
	{ Deity::SolusekRo,     "Solusek Ro" },
	{ Deity::TheTribunal,   "The Tribunal" },
	{ Deity::Tunare,        "Tunare" },
	{ Deity::Veeshan,       "Veeshan" }
};

static std::map<uint32, uint32> deity_bitmasks = {
	{ Deity::Agnostic1,     Deity::Bitmask::Agnostic },
	{ Deity::Agnostic2,     Deity::Bitmask::Agnostic },
	{ Deity::Bertoxxulous,  Deity::Bitmask::Bertoxxulous },
	{ Deity::BrellSirilis,  Deity::Bitmask::BrellSirilis },
	{ Deity::CazicThule,    Deity::Bitmask::CazicThule },
	{ Deity::ErollisiMarr,  Deity::Bitmask::ErollisiMarr },
	{ Deity::Bristlebane,   Deity::Bitmask::Bristlebane },
	{ Deity::Innoruuk,      Deity::Bitmask::Innoruuk },
	{ Deity::Karana,        Deity::Bitmask::Karana },
	{ Deity::MithanielMarr, Deity::Bitmask::MithanielMarr },
	{ Deity::Prexus,        Deity::Bitmask::Prexus },
	{ Deity::Quellious,     Deity::Bitmask::Quellious },
	{ Deity::RallosZek,     Deity::Bitmask::RallosZek },
	{ Deity::RodcetNife,    Deity::Bitmask::RodcetNife },
	{ Deity::SolusekRo,     Deity::Bitmask::SolusekRo },
	{ Deity::TheTribunal,   Deity::Bitmask::TheTribunal },
	{ Deity::Tunare,        Deity::Bitmask::Tunare },
	{ Deity::Veeshan,       Deity::Bitmask::Veeshan }
};
