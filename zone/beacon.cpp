/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2003 EQEMu Development Team (http://eqemulator.net)

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

/*
Beacon class, extends Mob. Used for AE rain spells to have a mob
target to center around.
*/

#include "common/races.h"
#include "zone/beacon.h"
#include "zone/entity.h"
#include "zone/mob.h"

extern EntityList entity_list;
extern Zone* zone;

class Zone;

// if lifetime is 0 this is a permanent beacon.. not sure if that'll be
// useful for anything
Beacon::Beacon(const glm::vec4 &in_pos, int lifetime) : Mob(
	nullptr, // in_name
	nullptr, // in_lastname
	0, // in_cur_hp
	0, // in_max_hp
	Gender::Male, // in_gender
	Race::InvisibleMan, // in_race
	Class::None, // in_class
	BodyType::NoTarget, // in_bodytype
	Deity::Unknown, // in_deity
	0, // in_level
	0, // in_npctype_id
	0.0f, // in_size
	0.0f, // in_runspeed
	in_pos, // position
	0, // in_light
	0, // in_texture
	0, // in_helmtexture
	0, // in_ac
	0, // in_atk
	0, // in_str
	0, // in_sta
	0, // in_dex
	0, // in_agi
	0, // in_int
	0, // in_wis
	0, // in_cha
	0, // in_haircolor
	0, // in_beardcolor
	0, // in_eyecolor1
	0, // in_eyecolor2
	0, // in_hairstyle
	0, // in_luclinface
	0, // in_beard
	0, // in_drakkin_heritage
	0, // in_drakkin_tattoo
	0, // in_drakkin_details
	EQ::TintProfile(), // in_armor_tint
	0, // in_aa_title
	0, // in_see_invis
	0, // in_see_invis_undead
	0, // in_see_hide
	0, // in_see_improved_hide
	0, // in_hp_regen
	0, // in_mana_regen
	0, // in_qglobal
	0, // in_maxlevel
	0, // in_scalerate
	0, // in_armtexture
	0, // in_bracertexture
	0, // in_handtexture
	0, // in_legtexture
	0, // in_feettexture
	0, // in_usemodel
	false, // in_always_aggros_foes
	0, //in_heroic_strikethrough
	false // in_keeps_sold_items
),
	  remove_timer(lifetime),
	  spell_timer(0)
{
	remove_timer.Disable();
	spell_timer.Disable();

	remove_me        = false;
	spell_id         = UINT16_MAX;
	resist_adjust    = 0;
	spell_iterations = 0;
	caster_id        = 0;
	max_targets      = 4;

	if (lifetime) {
		remove_timer.Start();
	}
}

Beacon::~Beacon()
{

}

bool Beacon::Process()
{
	if (remove_me) {
		return false;
	}

	if (
		spell_timer.Enabled() &&
		spell_timer.Check() &&
		IsValidSpell(spell_id)
	) {
		Mob* caster = entity_list.GetMob(caster_id);
		if (caster && spell_iterations-- && max_targets) {
			// NPCs should never be affected by an AE they cast. PB AEs shouldn't affect caster either
			// I don't think any other cases that get here matter
			const bool affect_caster = (!caster->IsNPC() && !caster->IsAIControlled()) &&
				spells[spell_id].target_type != ST_AECaster;
			entity_list.AESpell(caster, this, spell_id, affect_caster, resist_adjust, &max_targets);
		} else {
			// spell is done casting, or caster disappeared
			caster_id        = 0;
			spell_id         = UINT16_MAX;
			spell_iterations = 0;

			spell_timer.Disable();
		}
	}

	if (remove_timer.Enabled() && remove_timer.Check()) {
		return false;
	}

	return true;
}

void Beacon::AELocationSpell(Mob *caster, uint16 cast_spell_id, int16 in_resist_adjust)
{
	if (!IsValidSpell(cast_spell_id) || !caster) {
		return;
	}

	caster_id        = caster->GetID();
	spell_id         = cast_spell_id;
	resist_adjust    = in_resist_adjust;
	spell_iterations = spells[spell_id].aoe_duration / 2500;
	spell_iterations = spell_iterations < 1 ? 1 : spell_iterations;	// at least 1

	if (spells[spell_id].aoe_max_targets) {
		max_targets = spells[spell_id].aoe_max_targets;
	}

	spell_timer.Start(2500);
	spell_timer.Trigger();
}
