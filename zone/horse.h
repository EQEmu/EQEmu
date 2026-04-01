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
