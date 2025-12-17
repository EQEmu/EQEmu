/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2013 EQEMu Development Team (http://eqemu.org)

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

#include "common/features.h"
#include "common/types.h"

#include <map>
#include <string>

enum FACTION_VALUE {
	FACTION_ALLY           = 1,
	FACTION_WARMLY         = 2,
	FACTION_KINDLY         = 3,
	FACTION_AMIABLY        = 4,
	FACTION_INDIFFERENTLY  = 5,
	FACTION_APPREHENSIVELY = 6,
	FACTION_DUBIOUSLY      = 7,
	FACTION_THREATENINGLY  = 8,
	FACTION_SCOWLS         = 9
};

struct NPCFactionList {
	uint32	id;
	uint32	primaryfaction;
	bool	assistprimaryfaction;
	uint32	factionid[MAX_NPC_FACTIONS];
	int32	factionvalue[MAX_NPC_FACTIONS];
	int8	factionnpcvalue[MAX_NPC_FACTIONS];
	uint8	factiontemp[MAX_NPC_FACTIONS];
};

struct FactionMods
{
	int32 base;
	int16	min; 	// The lowest your personal earned faction can go - before race/class/deity adjustments.
	int16	max; 	// The highest your personal earned faction can go - before race/class/deity adjustments.
	int32 class_mod;
	int32 race_mod;
	int32 deity_mod;
};

struct Faction {
	int32	id;
	std::map<std::string, int16> mods;
	int16	base;
	int16	min; // The lowest your personal earned faction can go - before race/class/deity adjustments.
	int16	max; // The highest your personal earned faction can go - before race/class/deity adjustments.
	char	name[50];
};

typedef std::map<uint32, int16> faction_map;
struct NPCFaction
{
	uint32 factionID;
	int32 value_mod;
	int8 npc_value;
	uint8 temp;
};

const char *FactionValueToString(FACTION_VALUE faction_value);
FACTION_VALUE CalculateFaction(FactionMods* fm, int32 tmpCharacter_value);
