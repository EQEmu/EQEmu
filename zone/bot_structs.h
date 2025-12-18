/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.org)

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
#include "common/timer.h"
#include "zone/mob.h"

struct BotsAvailableList {
	uint32 bot_id;
	char   bot_name[64];
	uint16 class_;
	uint8  level;
	uint16 race;
	uint8  gender;
	char   owner_name[64];
	uint32 owner_id;
};

struct BotSpell {
	uint16 SpellId;
	int SpellIndex;
	int16 ManaCost;
};

struct BotSpell_wPriority : public BotSpell {
	uint8 Priority;
};

struct BotCastingRoles {
	bool GroupHealer;
	bool GroupSlower;
	bool GroupNuker;
	bool GroupDoter;
	//bool RaidHealer;
	//bool RaidSlower;
	//bool RaidNuker;
	//bool RaidDoter;
};

struct BotSpellSetting {
	int16  priority;
	uint8  min_level;
	uint8  max_level;
	int8   min_hp;
	int8   max_hp;
	bool   is_enabled = true;
};

struct BotSpells {
	uint32		type;			// 0 = never, must be one (and only one) of the defined values
	uint16		spellid;		// <= 0 = no spell
	int16		manacost;		// -1 = use spdat, -2 = no cast time
	uint32		time_cancast;	// when we can cast this spell next
	int32		recast_delay;
	int16		priority;
	int16		resist_adjust;
	uint8		minlevel;
	uint8		maxlevel;
	int16		min_hp;			// >0 won't cast if HP is below
	int16		max_hp;			// >0 won't cast if HP is above
	std::string	bucket_name;
	std::string	bucket_value;
	uint8		bucket_comparison;
};

struct BotSpells_wIndex {
	uint32		index;			//index of AIBot_spells
	uint32		type;			// 0 = never, must be one (and only one) of the defined values
	uint16		spellid;		// <= 0 = no spell
	int16		manacost;		// -1 = use spdat, -2 = no cast time
	uint32		time_cancast;	// when we can cast this spell next
	int32		recast_delay;
	int16		priority;
	int16		resist_adjust;
	uint8		minlevel;
	uint8		maxlevel;
	int16		min_hp;			// >0 won't cast if HP is below
	int16		max_hp;			// >0 won't cast if HP is above
	std::string	bucket_name;
	std::string	bucket_value;
	uint8		bucket_comparison;
};

struct BotTimer {
	uint32		timer_id;
	uint32		timer_value;
	uint32		recast_time;
	bool		is_spell;
	bool		is_disc;
	uint16		spell_id;
	bool		is_item;
	uint32		item_id;
};

struct BotSpellSettings {
	uint16      spell_type;                 // type ID of bot category
	std::string short_name;                 // type short name of bot category
	std::string name;                       // type name of bot category
	bool        hold;                       // 0 = allow spell type, 1 = hold spell type
	uint16      delay;                      // delay between casts of spell type, 1ms-60,000ms
	uint8       min_threshold;              // minimum target health threshold to allow casting of spell type
	uint8       max_threshold;              // maximum target health threshold to allow casting of spell type
	uint16      resist_limit;               // resist limit to skip spell type
	bool        aggro_check;                // whether or not to check for possible aggro before casting
	uint8       min_mana_pct;               // lower mana percentage limit to allow spell cast
	uint8       max_mana_pct;               // upper mana percentage limit to allow spell cast
	uint8       min_hp_pct;                 // lower HP percentage limit to allow spell cast
	uint8       max_hp_pct;                 // upper HP percentage limit to allow spell cast
	uint16      idle_priority;              // idle priority of the spell type
	uint16      engaged_priority;           // engaged priority of the spell type
	uint16      pursue_priority;            // pursue priority of the spell type
	uint16      ae_or_group_target_count;   // require target count to cast an AE or Group spell type
	uint16      announce_cast;              // announce when casting a certain spell type
	Timer       recast_timer;               // recast timer based off delay
	Timer       ai_delay;                   // spell timer based off delay	
};

struct BotSpellTypeOrder {
	uint16		spellType;
	uint16		priority;
};

struct BotBlockedBuffs {
	uint32_t	bot_id;
	uint32_t	spell_id;
	uint8_t		blocked;
	uint8_t		blocked_pet;
};

struct BotSpellTypesByClass {
	uint8_t min_level			= 255;
	std::string description;
};

struct CombatRangeInput {
	Mob*                    target;
	float                   target_distance;
	bool                    stop_melee_level;
	const EQ::ItemInstance* p_item;
	const EQ::ItemInstance* s_item;
};

struct CombatRangeOutput {
	bool  at_combat_range		= false;
	float melee_distance_min	= 0.0f;
	float melee_distance		= 0.0f;
	float melee_distance_max	= 0.0f;
};

struct CombatPositioningInput {
	Mob*	tar;
	bool	stop_melee_level;
	float	tar_distance;
	float	melee_distance_min;
	float	melee_distance;
	float	melee_distance_max;
	bool	behind_mob;
	bool	front_mob;
};

struct FindPositionInput {
	Mob*	tar;
	float	distance_min;
	float	distance_max;
	bool	behind_only;
	bool	front_only;
	bool	bypass_los;
};
