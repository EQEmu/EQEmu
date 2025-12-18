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

#include "zone/npc.h"

#include "glm/vec4.hpp"
#include <map>

class Client;
class Mob;
struct NPCType;
struct NewSpawn_Struct;

class Horse : public NPC {
public:
	Horse(Client* c, uint16 spell_id, const glm::vec4& position);

	virtual void FillSpawnStruct(NewSpawn_Struct* ns, Mob* m);

	static bool IsHorseSpell(uint16 spell_id);
protected:
	Client* owner;

	static std::map<uint16, const NPCType*> horse_types;
	static const NPCType* GetHorseType(uint16 spell_id);
	static const NPCType* BuildHorseType(uint16 spell_id);
};
