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
/*
Below are the blob structures for zone state dumping to the database
-Quagmire

create table zone_state_dump (zonename varchar(16) not null primary key, spawn2_count int unsigned not null default 0,
npc_count int unsigned not null default 0, npcloot_count int unsigned not null default 0, gmspawntype_count int unsigned not null default 0,
spawn2 mediumblob, npcs mediumblob, npc_loot mediumblob, gmspawntype mediumblob, time timestamp(14));
*/

#pragma once

#include "common/faction.h"
#include "common/eq_packet_structs.h"
#include "common/inventory_profile.h"

#pragma pack(push)
#pragma pack(1)

struct NPCType
{
	char            name[64];
	char            lastname[64];
	int64           current_hp;
	int64           max_hp;
	float           size;
	float           runspeed;
	uint8           gender;
	uint16          race;
	uint8           class_;
	uint8           bodytype;    // added for targettype support
	uint32          deity;        //not loaded from DB
	uint8           level;
	uint32          npc_id;
	uint8           texture;
	uint8           helmtexture;
	uint32          herosforgemodel;
	uint32          loottable_id;
	uint32          npc_spells_id;
	uint32          npc_spells_effects_id;
	int32           npc_faction_id;
	int32           faction_amount;    // faction association magnitude, will use primary faction
	uint32          merchanttype;
	uint32          alt_currency_type;
	uint32          adventure_template;
	uint32          trap_template;
	uint8           light;
	uint32          AC;
	uint64          Mana;    //not loaded from DB
	uint32          ATK;    //not loaded from DB
	uint32          STR;
	uint32          STA;
	uint32          DEX;
	uint32          AGI;
	uint32          INT;
	uint32          WIS;
	uint32          CHA;
	int32           MR;
	int32           FR;
	int32           CR;
	int32           PR;
	int32           DR;
	int32           Corrup;
	int32           PhR;
	uint8           haircolor;
	uint8           beardcolor;
	uint8           eyecolor1;            // the eyecolors always seem to be the same, maybe left and right eye?
	uint8           eyecolor2;
	uint8           hairstyle;
	uint8           luclinface;            //
	uint8           beard;                //
	uint32          drakkin_heritage;
	uint32          drakkin_tattoo;
	uint32          drakkin_details;
	EQ::TintProfile armor_tint;
	uint32          min_dmg;
	uint32          max_dmg;
	uint32          charm_ac;
	uint32          charm_min_dmg;
	uint32          charm_max_dmg;
	int             charm_attack_delay;
	int             charm_accuracy_rating;
	int             charm_avoidance_rating;
	int             charm_atk;
	int16           attack_count;
	char            special_abilities[512];
	uint32          d_melee_texture1;
	uint32          d_melee_texture2;
	char            ammo_idfile[30];
	uint8           prim_melee_type;
	uint8           sec_melee_type;
	uint8           ranged_type;
	int64           hp_regen;
	int64           hp_regen_per_second;
	int64           mana_regen;
	int32           aggroradius; // added for AI improvement - neotokyo
	int32           assistradius; // assist radius, defaults to aggroradis if not set
	uint16          see_invis;            // See Invis flag added
	uint16          see_invis_undead;    // See Invis vs. Undead flag added
	bool            see_hide;
	bool            see_improved_hide;
	bool            qglobal;
	bool            npc_aggro;
	uint8           spawn_limit;    //only this many may be in zone at a time (0=no limit)
	uint8           mount_color;    //only used by horse class
	float           attack_speed;    //%+- on attack delay of the mob.
	int             attack_delay;    //delay between attacks in ms
	int             accuracy_rating;    // flat bonus before mods
	int             avoidance_rating;    // flat bonus before mods
	bool            findable;        //can be found with find command
	bool            trackable;
	bool            is_quest_npc;
	int16           slow_mitigation;
	uint8           maxlevel;
	uint32          scalerate;
	bool            private_corpse;
	bool            unique_spawn_by_name;
	bool            underwater;
	uint32          emoteid;
	float           spellscale;
	float           healscale;
	bool            no_target_hotkey;
	bool            raid_target;
	uint8           armtexture;
	uint8           bracertexture;
	uint8           handtexture;
	uint8           legtexture;
	uint8           feettexture;
	bool            ignore_despawn;
	bool            show_name; // should default on
	bool            untargetable;
	bool            skip_global_loot;
	bool            rare_spawn;
	bool            skip_auto_scale; // just so it doesn't mess up bots or mercs, probably should add to DB too just in case
	int8            stuck_behavior;
	uint16          use_model;
	int8            flymode;
	bool            always_aggro;
	int             exp_mod;
	int             heroic_strikethrough;
	bool            keeps_sold_items;
	bool            is_parcel_merchant;
	uint8			greed;
	bool            multiquest_enabled;
	uint32          m_npc_tint_id;
};

#pragma pack(pop)
