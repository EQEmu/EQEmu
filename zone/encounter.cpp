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

#include "encounter.h"

#include "common/races.h"
#include "zone/entity.h"
#include "zone/mob.h"

class Zone;

Encounter::Encounter(const char *enc_name) : Mob(
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
	0, // in_npcype_id
	0, // in_size
	0, // in_runspeed
	glm::vec4(0, 0, 0, 0), // position
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
	0, // in_heroic_strikethrough
	false // in_keeps_sold_items
) {
	encounter_name[0] = 0;
	strn0cpy(encounter_name, enc_name, 64);
	remove_me = false;
}

Encounter::~Encounter()
{

}

bool Encounter::Process() {
	if (remove_me) return false;
	return true;
}
