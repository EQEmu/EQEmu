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

#include "common/eq_packet_structs.h"
#include "common/inventory_profile.h"

#pragma pack(push)
#pragma pack(1)

/*
	This is a place to store data that dosent have a home in the
	official player profile.

	Try not to expand or re-arrange thing in this struct
	to make conversion between sizes easier.
	if something gets removed from here, just turn it into an unused
	item, so conversion is not needed. Then reuse old unused values if
	possible later instead of adding more, so long as old values left
	over would not be extrememly unpleasent side effects.
*/
struct ExtendedProfile_Struct {
	// Pet stuff
	uint16				pet_id; /* Not Used */
	uint16				old_pet_hp; /* Not Used */
	uint16				old_pet_mana; /* Not Used */
	SpellBuff_Struct	pet_buffs[BUFF_COUNT]; /* Not Used */
	EQ::TextureMaterialProfile	pet_items; /* Not Used */
	char				merc_name[64]; /* Used */

	uint32				aa_effects; /* Used */
	uint32				perAA; /* Used: % of exp going to AAs */
	uint32				expended_aa; /* Used: Total of expended AA */
	uint32				pet_hp;  /* Not Used */
	uint32				pet_mana; /* Not Used */
	uint32				mercTemplateID; /* Not Used */
	uint32				mercSuspendedTime; /* Not Used */
	bool				mercIsSuspended; /* Not Used */
	uint32				mercTimerRemaining; /* Not Used */
	uint8				mercGender; /* Not Used */
	int32				mercState; /* Not Used */
	uint32				last_invsnapshot_time; /* Used */
	uint32				next_invsnapshot_time; /* Used */
};

#pragma pack(pop)

void InitExtendedProfile(ExtendedProfile_Struct *p);
bool SetExtendedProfile(ExtendedProfile_Struct *to, char *old, unsigned int len);
