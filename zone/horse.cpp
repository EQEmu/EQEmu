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

#include "horse.h"

#include "common/eqemu_logsys.h"
#include "common/linked_list.h"
#include "common/repositories/horses_repository.h"
#include "common/strings.h"
#include "zone/client.h"
#include "zone/entity.h"
#include "zone/mob.h"
#include "zone/string_ids.h"

std::map<uint16, const NPCType *> Horse::horse_types;

Horse::Horse(
	Client *c,
	uint16 spell_id,
	const glm::vec4& position
) : NPC(
	GetHorseType(spell_id),
	nullptr,
	position,
	GravityBehavior::Water
)
{
	strn0cpy(name, fmt::format("{}`s_Mount00", c->GetCleanName()).c_str(), sizeof(name));

	is_horse = true;
	owner    = c;
}

void Horse::FillSpawnStruct(NewSpawn_Struct* ns, Mob* m) {
	NPC::FillSpawnStruct(ns, m);

	ns->spawn.petOwnerId = 0;
	ns->spawn.runspeed   = NPCTypedata->runspeed;
}

bool Horse::IsHorseSpell(uint16 spell_id)
{
	return GetHorseType(spell_id);
}

const NPCType *Horse::GetHorseType(uint16 spell_id)
{
	if (horse_types.count(spell_id)) {
		return horse_types[spell_id];
	}

	const NPCType* n;

	horse_types[spell_id] = n = BuildHorseType(spell_id);

	return n;
}

const NPCType *Horse::BuildHorseType(uint16 spell_id)
{
	const auto& l = HorsesRepository::GetWhere(
		content_db,
		fmt::format(
			"`filename` = '{}'",
			Strings::Escape(spells[spell_id].teleport_zone)
		)
	);

	if (l.empty()) {
		LogError("No Database entry for mount: [{}], check the horses table.", spells[spell_id].teleport_zone);
		return nullptr;
	}

	auto n = new NPCType;

	memset(n, 0, sizeof(NPCType));

	strn0cpy(n->name, "Unclaimed_Mount", sizeof(n->name));

	strn0cpy(n->special_abilities, "19,1^20,1^24,1", sizeof(n->special_abilities));

	auto e = l.front();

	n->current_hp   = 1;
	n->max_hp       = 1;
	n->race         = e.race;
	n->gender       = e.gender;
	n->class_       = Class::Warrior;
	n->deity        = 1;
	n->level        = 1;
	n->npc_id       = 0;
	n->loottable_id = 0;
	n->texture      = e.texture;
	n->helmtexture  = e.helmtexture == -1 ? e.texture : e.helmtexture;
	n->runspeed     = e.mountspeed;
	n->light        = 0;
	n->STR          = 75;
	n->STA          = 75;
	n->DEX          = 75;
	n->AGI          = 75;
	n->INT          = 75;
	n->WIS          = 75;
	n->CHA          = 75;

	return n;
}

void Client::SummonHorse(uint16 spell_id)
{
	if (GetHorseId()) {
		MessageString(Chat::Red, ALREADY_ON_A_MOUNT);
		return;
	}

	if (!Horse::IsHorseSpell(spell_id)) {
		LogError("[{}] tried to summon an unknown horse, spell_id [{}].", GetName(), spell_id);
		return;
	}

	Horse* h = new Horse(this, spell_id, GetPosition());

	entity_list.AddNPC(h, false);

	EQApplicationPacket outapp;

	h->CreateHorseSpawnPacket(&outapp);

	entity_list.QueueClients(h, &outapp);

	SetHorseId(h->GetID());
	BuffFadeBySitModifier();
}

void Client::SetHorseId(uint16 in_horse_id)
{
	if (horseId == in_horse_id) {
		return;
	}

	if (horseId) {
		Mob* h = entity_list.GetMob(horseId);
		if (h) {
			h->Depop();
		}
	}

	horseId = in_horse_id;
}

void Mob::CreateHorseSpawnPacket(EQApplicationPacket* app, Mob* m) {
	app->SetOpcode(OP_NewSpawn);
	safe_delete_array(app->pBuffer);
	app->pBuffer = new uchar[sizeof(NewSpawn_Struct)];
	app->size = sizeof(NewSpawn_Struct);
	memset(app->pBuffer, 0, sizeof(NewSpawn_Struct));
	auto* ns = (NewSpawn_Struct*) app->pBuffer;
	FillSpawnStruct(ns, m);
}
