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

#include "npc.h"

#include "common/bodytypes.h"
#include "common/classes.h"
#include "common/data_verification.h"
#include "common/emu_versions.h"
#include "common/events/player_event_logs.h"
#include "common/features.h"
#include "common/item_instance.h"
#include "common/linked_list.h"
#include "common/misc_functions.h"
#include "common/repositories/npc_types_repository.h"
#include "common/repositories/spawn2_repository.h"
#include "common/repositories/spawnentry_repository.h"
#include "common/repositories/spawngroup_repository.h"
#include "common/rulesys.h"
#include "common/say_link.h"
#include "common/seperator.h"
#include "common/servertalk.h"
#include "common/skill_caps.h"
#include "common/spdat.h"
#include "common/strings.h"
#include "zone/bot.h"
#include "zone/client.h"
#include "zone/entity.h"
#include "zone/npc_scale_manager.h"
#include "zone/quest_parser_collection.h"
#include "zone/spawn2.h"
#include "zone/string_ids.h"
#include "zone/water_map.h"
#include "zone/zone.h"

#include <cstdio>
#include <string>
#include <utility>

extern Zone* zone;
extern QueryServ* QServ;
extern volatile bool is_zone_loaded;
extern EntityList entity_list;

NPC::NPC(const NPCType *npc_type_data, Spawn2 *in_respawn, const glm::vec4 &position, GravityBehavior iflymode, bool IsCorpse)
	: Mob(
		  npc_type_data->name,
		  npc_type_data->lastname,
		  npc_type_data->max_hp,
		  npc_type_data->max_hp,
		  npc_type_data->gender,
		  npc_type_data->race,
		  npc_type_data->class_,
		  npc_type_data->bodytype,
		  npc_type_data->deity,
		  npc_type_data->level,
		  npc_type_data->npc_id,
		  npc_type_data->size,
		  npc_type_data->runspeed,
		  position,
		  npc_type_data->light, // innate_light
		  npc_type_data->texture,
		  npc_type_data->helmtexture,
		  npc_type_data->AC,
		  npc_type_data->ATK,
		  npc_type_data->STR,
		  npc_type_data->STA,
		  npc_type_data->DEX,
		  npc_type_data->AGI,
		  npc_type_data->INT,
		  npc_type_data->WIS,
		  npc_type_data->CHA,
		  npc_type_data->haircolor,
		  npc_type_data->beardcolor,
		  npc_type_data->eyecolor1,
		  npc_type_data->eyecolor2,
		  npc_type_data->hairstyle,
		  npc_type_data->luclinface,
		  npc_type_data->beard,
		  npc_type_data->drakkin_heritage,
		  npc_type_data->drakkin_tattoo,
		  npc_type_data->drakkin_details,
		  npc_type_data->armor_tint,
		  0,
		  npc_type_data->see_invis,            // pass see_invis/see_ivu flags to mob constructor
		  npc_type_data->see_invis_undead,
		  npc_type_data->see_hide,
		  npc_type_data->see_improved_hide,
		  npc_type_data->hp_regen,
		  npc_type_data->mana_regen,
		  npc_type_data->qglobal,
		  npc_type_data->maxlevel,
		  npc_type_data->scalerate,
		  npc_type_data->armtexture,
		  npc_type_data->bracertexture,
		  npc_type_data->handtexture,
		  npc_type_data->legtexture,
		  npc_type_data->feettexture,
		  npc_type_data->use_model,
		  npc_type_data->always_aggro,
		  npc_type_data->heroic_strikethrough,
		  npc_type_data->keeps_sold_items,
		  npc_type_data->hp_regen_per_second,
		  npc_type_data->m_npc_tint_id
	  ),
	  attacked_timer(CombatEventTimer_expire),
	  swarm_timer(100),
	  m_resumed_from_zone_suspend_shutoff_timer(10000),
	  classattack_timer(1000),
	  monkattack_timer(1000),
	  knightattack_timer(1000),
	  assist_timer(AIassistcheck_delay),
	  qglobal_purge_timer(30000),
	  send_hp_update_timer(2000),
	  enraged_timer(1000),
	  taunt_timer(TauntReuseTime * 1000),
	  m_SpawnPoint(position),
	  m_GuardPoint(-1, -1, -1, 0),
	  m_GuardPointSaved(0, 0, 0, 0)
{
	//What is the point of this, since the names get mangled..
	Mob *mob = entity_list.GetMob(name);
	if (mob != nullptr) {
		entity_list.RemoveEntity(mob->GetID());
	}

	int moblevel = GetLevel();

	NPCTypedata      = npc_type_data;
	NPCTypedata_ours = nullptr;
	respawn2         = in_respawn;

	swarm_timer.Disable();

	if (size <= 0.0f) {
		size = GetRaceGenderDefaultHeight(race, gender);
	}

	// lava dragon is a fixed size model and should always use its default
	// otherwise pathing issues
	if (race == Race::LavaDragon) {
		size = 5;
	}
	if (race == Race::Wurm) {
		size = 15;
	}

	SetTaunting(false);
	proximity            = nullptr;
	m_loot_copper        = 0;
	m_loot_silver        = 0;
	m_loot_gold          = 0;
	m_loot_platinum      = 0;
	max_dmg              = npc_type_data->max_dmg;
	min_dmg              = npc_type_data->min_dmg;
	attack_count         = npc_type_data->attack_count;
	grid                 = 0;
	wp_m                 = 0;
	max_wp               = 0;
	save_wp              = 0;
	spawn_group_id       = 0;
	swarmInfoPtr         = nullptr;
	spellscale           = npc_type_data->spellscale;
	healscale            = npc_type_data->healscale;
	pAggroRange          = npc_type_data->aggroradius;
	pAssistRange         = npc_type_data->assistradius;
	findable             = npc_type_data->findable;
	trackable            = npc_type_data->trackable;
	MR                   = npc_type_data->MR;
	CR                   = npc_type_data->CR;
	DR                   = npc_type_data->DR;
	FR                   = npc_type_data->FR;
	PR                   = npc_type_data->PR;
	Corrup               = npc_type_data->Corrup;
	PhR                  = npc_type_data->PhR;
	STR                  = npc_type_data->STR;
	STA                  = npc_type_data->STA;
	AGI                  = npc_type_data->AGI;
	DEX                  = npc_type_data->DEX;
	INT                  = npc_type_data->INT;
	WIS                  = npc_type_data->WIS;
	CHA                  = npc_type_data->CHA;
	npc_mana             = npc_type_data->Mana;
	m_is_underwater_only = npc_type_data->underwater;
	m_is_quest_npc       = npc_type_data->is_quest_npc;

	//quick fix of ordering if they screwed it up in the DB
	if (max_dmg < min_dmg) {
		int tmp = min_dmg;
		min_dmg = max_dmg;
		max_dmg = tmp;
	}

	// Max Level and Stat Scaling if maxlevel is set
	if (maxlevel > level) {
		LevelScale();
	}

	base_damage          = round((max_dmg - min_dmg) / 1.9);
	min_damage           = min_dmg - round(base_damage / 10.0);
	accuracy_rating      = npc_type_data->accuracy_rating;
	avoidance_rating     = npc_type_data->avoidance_rating;
	ATK                  = npc_type_data->ATK;
	heroic_strikethrough = npc_type_data->heroic_strikethrough;
	keeps_sold_items     = npc_type_data->keeps_sold_items;
	m_multiquest_enabled = npc_type_data->multiquest_enabled;

	// used for when switch back to charm
	default_ac               = npc_type_data->AC;
	default_min_dmg          = min_dmg;
	default_max_dmg          = max_dmg;
	default_attack_delay     = npc_type_data->attack_delay;
	default_accuracy_rating  = npc_type_data->accuracy_rating;
	default_avoidance_rating = npc_type_data->avoidance_rating;
	default_atk              = npc_type_data->ATK;
	strn0cpy(default_special_abilities, npc_type_data->special_abilities, 512);

	// used for when getting charmed, if 0, doesn't swap
	charm_ac               = npc_type_data->charm_ac;
	charm_min_dmg          = npc_type_data->charm_min_dmg;
	charm_max_dmg          = npc_type_data->charm_max_dmg;
	charm_attack_delay     = npc_type_data->charm_attack_delay;
	charm_accuracy_rating  = npc_type_data->charm_accuracy_rating;
	charm_avoidance_rating = npc_type_data->charm_avoidance_rating;
	charm_atk              = npc_type_data->charm_atk;

	CalcMaxMana();
	RestoreMana();

	MerchantType          = npc_type_data->merchanttype;
	merchant_open         = (
		GetClass() == Class::Merchant ||
		GetClass() == Class::DiscordMerchant ||
		GetClass() == Class::AdventureMerchant ||
		GetClass() == Class::NorrathsKeepersMerchant ||
		GetClass() == Class::DarkReignMerchant ||
		GetClass() == Class::AlternateCurrencyMerchant
	);
	adventure_template_id = npc_type_data->adventure_template;
	flymode               = iflymode;

	// If server has set a flymode in db honor it over all else.
	// If server has not set a flymde in db, and this is a boat - force floating.
	if (npc_type_data->flymode >= 0) {
		flymode = static_cast<GravityBehavior>(npc_type_data->flymode);
	}
	else if (GetIsBoat()) {
		flymode = GravityBehavior::Floating;
	}

	guard_anim            = eaStanding;

	m_roambox.max_x     = -2;
	m_roambox.max_y     = -2;
	m_roambox.min_x     = -2;
	m_roambox.min_y     = -2;
	m_roambox.distance  = 0;
	m_roambox.dest_x    = -2;
	m_roambox.dest_y    = -2;
	m_roambox.dest_z    = 0;
	m_roambox.delay     = 1000;
	m_roambox.min_delay = 1000;

	p_depop            = false;
	m_loottable_id     = npc_type_data->loottable_id;
	m_skip_global_loot = npc_type_data->skip_global_loot;
	m_skip_auto_scale  = npc_type_data->skip_auto_scale;
	rare_spawn         = npc_type_data->rare_spawn;
	no_target_hotkey   = npc_type_data->no_target_hotkey;
	primary_faction    = 0;
	faction_amount     = npc_type_data->faction_amount;

	SetNPCFactionID(npc_type_data->npc_faction_id);

	npc_spells_id        = 0;
	HasAISpell           = false;
	HasAISpellEffects    = false;
	innate_proc_spell_id = 0;
	m_record_loot_stats  = false;

	if (GetClass() == Class::MercenaryLiaison && RuleB(Mercs, AllowMercs)) {
		LoadMercenaryTypes();
		LoadMercenaries();
	}

	SpellFocusDMG  = 0;
	SpellFocusHeal = 0;

	pet_spell_id = 0;

	delaytimer      = false;
	combat_event    = false;
	attack_speed    = npc_type_data->attack_speed;
	attack_delay    = npc_type_data->attack_delay;
	slow_mitigation = npc_type_data->slow_mitigation;

	EntityList::RemoveNumbers(name);
	entity_list.MakeNameUnique(name);

	npc_aggro = npc_type_data->npc_aggro;

	AISpellVar.fail_recast                     = static_cast<uint32>(RuleI(Spells, AI_SpellCastFinishedFailRecast));
	AISpellVar.engaged_no_sp_recast_min        = static_cast<uint32>(RuleI(Spells, AI_EngagedNoSpellMinRecast));
	AISpellVar.engaged_no_sp_recast_max        = static_cast<uint32>(RuleI(Spells, AI_EngagedNoSpellMaxRecast));
	AISpellVar.engaged_beneficial_self_chance  = static_cast<uint8> (RuleI(Spells, AI_EngagedBeneficialSelfChance));
	AISpellVar.engaged_beneficial_other_chance = static_cast<uint8> (RuleI(Spells, AI_EngagedBeneficialOtherChance));
	AISpellVar.engaged_detrimental_chance      = static_cast<uint8> (RuleI(Spells, AI_EngagedDetrimentalChance));
	AISpellVar.pursue_no_sp_recast_min         = static_cast<uint32>(RuleI(Spells, AI_PursueNoSpellMinRecast));
	AISpellVar.pursue_no_sp_recast_max         = static_cast<uint32>(RuleI(Spells, AI_PursueNoSpellMaxRecast));
	AISpellVar.pursue_detrimental_chance       = static_cast<uint8> (RuleI(Spells, AI_PursueDetrimentalChance));
	AISpellVar.idle_no_sp_recast_min           = static_cast<uint32>(RuleI(Spells, AI_IdleNoSpellMinRecast));
	AISpellVar.idle_no_sp_recast_max           = static_cast<uint32>(RuleI(Spells, AI_IdleNoSpellMaxRecast));
	AISpellVar.idle_beneficial_chance          = static_cast<uint8> (RuleI(Spells, AI_IdleBeneficialChance));

	// It's possible for IsBot() to not be set yet during Bot loading, so have to use an alternative to catch Bots
	if (!EQ::ValueWithin(npc_type_data->npc_spells_id, EQ::constants::BotSpellIDs::Warrior, EQ::constants::BotSpellIDs::Berserker)) {
		AI_Init();
		AI_Start();
	} else {
		CastToBot()->AI_Bot_Init();
		CastToBot()->AI_Bot_Start();
	}

	d_melee_texture1 = npc_type_data->d_melee_texture1;
	d_melee_texture2 = npc_type_data->d_melee_texture2;
	herosforgemodel  = npc_type_data->herosforgemodel;

	ammo_idfile = npc_type_data->ammo_idfile;
	memset(equipment, 0, sizeof(equipment));
	prim_melee_type = npc_type_data->prim_melee_type;
	sec_melee_type  = npc_type_data->sec_melee_type;
	ranged_type     = npc_type_data->ranged_type;

	// If Melee Textures are not set, set attack type to Hand to Hand as default
	if (!d_melee_texture1) {
		prim_melee_type = 28;
	}
	if (!d_melee_texture2) {
		sec_melee_type = 28;
	}

	//give NPCs skill values...
	int r;
	for (r = 0; r <= EQ::skills::HIGHEST_SKILL; r++) {
		skills[r] = SkillCaps::Instance()->GetSkillCap(GetClass(), (EQ::skills::SkillType)r, moblevel).cap;
	}
	// some overrides -- really we need to be able to set skills for mobs in the DB
	// There are some known low level SHM/BST pets that do not follow this, which supports
	// the theory of needing to be able to set skills for each mob separately
	if (IsBot()) {
		if (GetClass() != Class::Paladin && GetClass() != Class::ShadowKnight) {
			knightattack_timer.Disable();
		}
		else if (GetClass() != Class::Monk || GetLevel() < 10) {
			monkattack_timer.Disable();
		}
	}
	else {
		if (moblevel > 50) {
			skills[EQ::skills::SkillDoubleAttack] = 250;
			skills[EQ::skills::SkillDualWield] = 250;
		}
		else if (moblevel > 3) {
			skills[EQ::skills::SkillDoubleAttack] = moblevel * 5;
			skills[EQ::skills::SkillDualWield] = skills[EQ::skills::SkillDoubleAttack];
		}
		else {
			skills[EQ::skills::SkillDoubleAttack] = moblevel * 5;
		}
	}

	SetLDoNTrapped(false);
	SetLDoNTrapType(0);
	SetLDoNTrapSpellID(0);
	SetLDoNLocked(false);
	SetLDoNLockedSkill(0);
	SetLDoNTrapDetected(false);

	if (npc_type_data->trap_template > 0) {
		std::map<uint32, std::list<LDoNTrapTemplate *> >::iterator trap_ent_iter;
		std::list<LDoNTrapTemplate *> trap_list;

		trap_ent_iter = zone->ldon_trap_entry_list.find(npc_type_data->trap_template);
		if (trap_ent_iter != zone->ldon_trap_entry_list.end()) {
			trap_list = trap_ent_iter->second;
			if (trap_list.size() > 0) {
				auto trap_list_iter = trap_list.begin();
				std::advance(trap_list_iter, zone->random.Int(0, trap_list.size() - 1));
				LDoNTrapTemplate *trap_template = (*trap_list_iter);
				if (trap_template) {
					if (trap_template->spell_id > 0) {
						SetLDoNTrapped(true);
						SetLDoNTrapSpellID(trap_template->spell_id);
					} else {
						SetLDoNTrapped(false);
						SetLDoNTrapSpellID(0);
					}

					SetLDoNTrapType(static_cast<uint8>(trap_template->type));

					if (trap_template->locked > 0) {
						SetLDoNLocked(true);
						SetLDoNLockedSkill(trap_template->skill);
					} else {
						SetLDoNLocked(false);
						SetLDoNLockedSkill(0);
					}

					SetLDoNTrapDetected(false);
				}
			}
		}
	}

	reface_timer = new Timer(15000);
	reface_timer->Disable();

	qGlobals = nullptr;

	SetEmoteID(npc_type_data->emoteid);
	InitializeBuffSlots();
	CalcBonuses();

	raid_target    = npc_type_data->raid_target;
	ignore_despawn = npc_type_data->ignore_despawn;
	m_targetable   = !npc_type_data->untargetable;
	m_npc_tint_id  = npc_type_data->m_npc_tint_id;

	npc_scale_manager->ScaleNPC(this);

	RestoreMana();

	if (GetBodyType() == BodyType::Animal && !RuleB(NPC, AnimalsOpenDoors)) {
		m_can_open_doors = false;
	}

	GetInv().SetInventoryVersion(EQ::versions::MobVersion::RoF2);
	GetInv().SetGMInventory(false);
}

float NPC::GetRoamboxMaxX() const
{
	return m_roambox.max_x;
}

float NPC::GetRoamboxMaxY() const
{
	return m_roambox.max_y;
}

float NPC::GetRoamboxMinX() const
{
	return m_roambox.min_x;
}

float NPC::GetRoamboxMinY() const
{
	return m_roambox.min_y;
}

float NPC::GetRoamboxDistance() const
{
	return m_roambox.distance;
}

float NPC::GetRoamboxDestinationX() const
{
	return m_roambox.dest_x;
}

float NPC::GetRoamboxDestinationY() const
{
	return m_roambox.dest_y;
}

float NPC::GetRoamboxDestinationZ() const
{
	return m_roambox.dest_z;
}

uint32 NPC::GetRoamboxDelay() const
{
	return m_roambox.delay;
}

uint32 NPC::GetRoamboxMinDelay() const
{
	return m_roambox.min_delay;
}

NPC::~NPC()
{
	AI_Stop();

	if (proximity) {
		entity_list.RemoveProximity(GetID());
		safe_delete(proximity);
	}

	safe_delete(NPCTypedata_ours);

	LootItems::iterator cur, end;
	cur = m_loot_items.begin();
	end = m_loot_items.end();
	for (; cur != end; ++cur) {
		LootItem *item = *cur;
		safe_delete(item);
	}

	m_loot_items.clear();
	faction_list.clear();

	safe_delete(reface_timer);
	safe_delete(swarmInfoPtr);
	safe_delete(qGlobals);
	UninitializeBuffSlots();
}

void NPC::SetTarget(Mob* mob) {
	if(mob == GetTarget())		//dont bother if they are allready our target
		return;

	if (GetPetTargetLockID()) {
		TryDepopTargetLockedPets(mob);
	}

	if (mob) {
		SetAttackTimer();
	} else {
		ranged_timer.Disable();
		//attack_timer.Disable();
		attack_dw_timer.Disable();
	}

	// either normal pet and owner is client or charmed pet and owner is client
	Mob *owner = nullptr;
	if (IsPet() && IsPetOwnerClient()) {
		owner = GetOwner();
	} else if (IsCharmed()) {
		owner = GetOwner();
		if (owner && !owner->IsClient())
			owner = nullptr;
	}

	if (owner) {
		auto client = owner->CastToClient();
		if (client->ClientVersionBit() & EQ::versions::maskUFAndLater) {
			auto app = new EQApplicationPacket(OP_PetHoTT, sizeof(ClientTarget_Struct));
			auto ct = (ClientTarget_Struct *)app->pBuffer;
			ct->new_target = mob ? mob->GetID() : 0;
			client->FastQueuePacket(&app);
		}
	}
	Mob::SetTarget(mob);
}

bool NPC::Process()
{
	if (p_depop)
	{
		Mob* owner = entity_list.GetMob(ownerid);
		if (owner != 0)
		{
			//if(GetBodyType() != BodyType::SwarmPet)
			// owner->SetPetID(0);
			ownerid = 0;
			petid = 0;
		}
		return false;
	}

	if (IsStunned() && stunned_timer.Check()) {
		Mob::UnStun();
		spun_timer.Disable();
	}

	SpellProcess();

	if (swarm_timer.Check()) {
		DepopSwarmPets();
	}

	if (m_scan_close_mobs_timer.Check()) {
		entity_list.ScanCloseMobs(this);
	}

	if (m_mob_check_moving_timer.Check()) {
		CheckScanCloseMobsMovingTimer();
	}

	if (hp_regen_per_second > 0 && hp_regen_per_second_timer.Check()) {
		if (GetHP() < GetMaxHP()) {
			SetHP(GetHP() + hp_regen_per_second);
		}
	}

	// zone state corpse creation timer
	if (RuleB(Zone, StateSavingOnShutdown)) {
		// shuts off the temporary spawn protected state of the NPC
		if (m_resumed_from_zone_suspend_shutoff_timer.Check()) {
			m_resumed_from_zone_suspend_shutoff_timer.Disable();
			SetResumedFromZoneSuspend(false);
		}
	}

	if (tic_timer.Check()) {
		if (m_clear_wearchange_cache_timer.Check()) {
			m_last_seen_wearchange.clear();
		}

		if (parse->HasQuestSub(GetNPCTypeID(), EVENT_TICK)) {
			parse->EventNPC(EVENT_TICK, this, nullptr, "", 0);
		}

		BuffProcess();

		if (currently_fleeing) {
			ProcessFlee();
		}

		int64 npc_sitting_regen_bonus = 0;
		int64 pet_regen_bonus = 0;
		int64 npc_regen = 0;
		int64 npc_hp_regen = GetNPCHPRegen();

		if (GetAppearance() == eaSitting) {
			npc_sitting_regen_bonus += 3;
		}

		int64 ooc_regen_calc = 0;
		if (ooc_regen > 0) { //should pull from Mob class
			ooc_regen_calc += GetMaxHP() * ooc_regen / 100;
		}

		/**
		 * Use max value between two values
		 */
		npc_regen = std::max(npc_hp_regen, ooc_regen_calc);

		if ((GetHP() < GetMaxHP()) && !IsPet()) {
			if (!IsEngaged()) {
				SetHP(GetHP() + npc_regen + npc_sitting_regen_bonus);
			}
			else {
				SetHP(GetHP() + npc_hp_regen);
			}
		}
		else if (GetHP() < GetMaxHP() && GetOwnerID() != 0) {
			if (!IsEngaged()) {
				if (ooc_regen > 0) {
					pet_regen_bonus = std::max(ooc_regen_calc, npc_hp_regen);
				}
				else {
					pet_regen_bonus = npc_hp_regen + (GetLevel() / 5);
				}

				SetHP(GetHP() + npc_sitting_regen_bonus + pet_regen_bonus);
			}
			else {
				SetHP(GetHP() + npc_hp_regen);
			}

		}
		else {
			SetHP(GetHP() + npc_hp_regen + npc_sitting_regen_bonus);
		}

		if (GetMana() < GetMaxMana()) {
			if (RuleB(NPC, UseMeditateBasedManaRegen)) {
				int64 npc_idle_mana_regen_bonus = 2;
				uint16 meditate_skill = GetSkill(EQ::skills::SkillMeditate);
				if (!IsEngaged() && meditate_skill > 0) {
					uint8 clevel = GetLevel();
					npc_idle_mana_regen_bonus =
						(((meditate_skill / 10) +
						(clevel - (clevel / 4))) / 4) + 4;
				}
				SetMana(GetMana() + mana_regen + npc_idle_mana_regen_bonus);
			}
			else {
				SetMana(GetMana() + mana_regen + npc_sitting_regen_bonus);
			}
		}

		SendHPUpdate();

		if (zone->adv_data && !p_depop) {
			ServerZoneAdventureDataReply_Struct *ds = (ServerZoneAdventureDataReply_Struct *) zone->adv_data;
			if (ds->type == Adventure_Rescue && ds->data_id == GetNPCTypeID()) {
				Mob *o = GetOwner();
				if (o && o->IsClient()) {
					float x_diff = ds->dest_x - GetX();
					float y_diff = ds->dest_y - GetY();
					float z_diff = ds->dest_z - GetZ();
					float dist   = ((x_diff * x_diff) + (y_diff * y_diff) + (z_diff * z_diff));
					if (dist < RuleR(Adventure, DistanceForRescueComplete)) {
						zone->DoAdventureCountIncrease();
						Say(
							"You don't know what this means to me. Thank you so much for finding and saving me from"
							" this wretched place. I'll find my way from here."
						);
						Depop();
					}
				}
			}
		}
	}

	/**
	 * Send HP updates when engaged
	 */
	if (send_hp_update_timer.Check(false) && IsEngaged()) {
		SendHPUpdate();
	}

	if (viral_timer.Check()) {
		VirusEffectProcess();
	}

	if(spellbonuses.GravityEffect == 1) {
		if(gravity_timer.Check())
			DoGravityEffect();
	}

	if(reface_timer->Check() && !IsEngaged() && IsPositionEqualWithinCertainZ(m_Position, m_GuardPoint, 5.0f)) {
		RotateTo(m_GuardPoint.w);
		reface_timer->Disable();
	}

	// needs to be done before mez and stun
	if (ForcedMovement)
		ProcessForcedMovement();

	if (IsMezzed())
		return true;

	if(IsStunned()) {
		if(spun_timer.Check())
			Spin();
		return true;
	}

	if (enraged_timer.Check()){
		ProcessEnrage();

		/* Don't keep running the check every second if we don't have enrage */
		if (!GetSpecialAbility(SpecialAbility::Enrage)) {
			enraged_timer.Disable();
		}
	}

	//Handle assists...
	if (assist_cap_timer.Check()) {
		if (NPCAssistCap() > 0)
			DelAssistCap();
		else
			assist_cap_timer.Disable();
	}

	if (assist_timer.Check() && IsEngaged() && !Charmed() && !HasAssistAggro() &&
	    NPCAssistCap() < RuleI(Combat, NPCAssistCap)) {
		// Some cases like flash of light used for aggro haven't set target
		if (!GetTarget()) {
			SetTarget(hate_list.GetMobWithMostHateOnList(this));
		}
		AIYellForHelp(this, GetTarget());
		if (NPCAssistCap() > 0 && !assist_cap_timer.Enabled())
			assist_cap_timer.Start(RuleI(Combat, NPCAssistCapTimer));
	}

	if(qGlobals)
	{
		if(qglobal_purge_timer.Check())
		{
			qGlobals->PurgeExpiredGlobals();
		}
	}

	if (bot_attack_flag_timer.Check()) {
		bot_attack_flag_timer.Disable();
		ClearBotAttackFlags();
	}

	AI_Process();

	return true;
}

uint32 NPC::CountLoot() {
	return(m_loot_items.size());
}

void NPC::UpdateEquipmentLight()
{
	m_Light.Type[EQ::lightsource::LightEquipment] = 0;
	m_Light.Level[EQ::lightsource::LightEquipment] = 0;

	for (int index = EQ::invslot::EQUIPMENT_BEGIN; index <= EQ::invslot::EQUIPMENT_END; ++index) {
		if (index == EQ::invslot::slotAmmo) { continue; }

		auto item = database.GetItem(equipment[index]);
		if (item == nullptr) { continue; }

		if (EQ::lightsource::IsLevelGreater(item->Light, m_Light.Type[EQ::lightsource::LightEquipment])) {
			m_Light.Type[EQ::lightsource::LightEquipment] = item->Light;
			m_Light.Level[EQ::lightsource::LightEquipment] = EQ::lightsource::TypeToLevel(m_Light.Type[EQ::lightsource::LightEquipment]);
		}
	}

	uint8 general_light_type = 0;
	for (auto iter = m_loot_items.begin(); iter != m_loot_items.end(); ++iter) {
		auto item = database.GetItem((*iter)->item_id);
		if (item == nullptr) { continue; }

		if (!item->IsClassCommon()) { continue; }
		if (item->Light < 9 || item->Light > 13) { continue; }

		if (EQ::lightsource::TypeToLevel(item->Light))
			general_light_type = item->Light;
	}

	if (EQ::lightsource::IsLevelGreater(general_light_type, m_Light.Type[EQ::lightsource::LightEquipment]))
		m_Light.Type[EQ::lightsource::LightEquipment] = general_light_type;

	m_Light.Level[EQ::lightsource::LightEquipment] = EQ::lightsource::TypeToLevel(m_Light.Type[EQ::lightsource::LightEquipment]);
}

void NPC::Depop(bool start_spawn_timer) {
	if (emoteid) {
		DoNPCEmote(EQ::constants::EmoteEventTypes::OnDespawn, emoteid);
	}

	parse->EventBotMercNPC(EVENT_DESPAWN, this, nullptr);

	if (parse->HasQuestSub(ZONE_CONTROLLER_NPC_ID, EVENT_DESPAWN_ZONE)) {
		DispatchZoneControllerEvent(EVENT_DESPAWN_ZONE, this, "", 0, nullptr);
	}

	if (parse->ZoneHasQuestSub(EVENT_DESPAWN_ZONE)) {
		std::vector<std::any> args = { this };
		parse->EventZone(EVENT_DESPAWN_ZONE, zone, "", 0, &args);
	}

	p_depop = true;
	if (respawn2) {
		if (start_spawn_timer) {
			respawn2->DeathReset();
		} else {
			respawn2->Depop();
		}
	}
}

bool NPC::SpawnZoneController()
{
	if (!RuleB(Zone, UseZoneController)) {
		return false;
	}

	auto npc_type = new NPCType;
	memset(npc_type, 0, sizeof(NPCType));

	strncpy(npc_type->name, "zone_controller", 60);
	npc_type->current_hp       = 2000000000;
	npc_type->max_hp           = 2000000000;
	npc_type->hp_regen         = 100000000;
	npc_type->race             = 240;
	npc_type->size             = .1;
	npc_type->gender           = 2;
	npc_type->class_           = 1;
	npc_type->deity            = 1;
	npc_type->level            = 200;
	npc_type->npc_id           = ZONE_CONTROLLER_NPC_ID;
	npc_type->loottable_id     = 0;
	npc_type->texture          = 3;
	npc_type->runspeed         = 0;
	npc_type->d_melee_texture1 = 0;
	npc_type->d_melee_texture2 = 0;
	npc_type->merchanttype     = 0;
	npc_type->bodytype         = 11;
	npc_type->skip_global_loot = true;

	if (RuleB(Zone, EnableZoneControllerGlobals)) {
		npc_type->qglobal = true;
	}

	npc_type->prim_melee_type = 28;
	npc_type->sec_melee_type  = 28;

	npc_type->findable  = 0;
	npc_type->trackable = 0;
	npc_type->untargetable = 1;

	strcpy(npc_type->special_abilities, "1,1,3000,50^12,1^14,1^16,1^18,1^19,1^20,1^21,1^22,1^23,1^24,1^25,1^26,1^32,1^33,1^35,1^46,1^47,1^48,1^49,1^50,1^52,1^53,1^54,1^55,1^56,1^57,1");

	glm::vec4 point;
	point.x = 30000;
	point.y = 10000;
	point.z = -10000;

	auto npc = new NPC(npc_type, nullptr, point, GravityBehavior::Flying);
	npc->GiveNPCTypeData(npc_type);

	entity_list.AddNPC(npc);

	return true;
}

void NPC::SpawnGridNodeNPC(const glm::vec4 &position, int32 grid_id, int32 grid_number, int32 zoffset) {
	auto npc_type = new NPCType;
	memset(npc_type, 0, sizeof(NPCType));

	std::string str_zoffset = Strings::NumberToWords(zoffset);
	std::string str_number = Strings::NumberToWords(grid_number);

	strcpy(npc_type->name, str_number.c_str());
	if (zoffset != 0) {
		strcat(npc_type->name, "(Stacked)");
	}

	npc_type->current_hp = 4000000;
	npc_type->max_hp = 4000000;
	npc_type->race = 2254;
	npc_type->gender = Gender::Neuter;
	npc_type->class_ = 9;
	npc_type->deity = 1;
	npc_type->level = 200;
	npc_type->npc_id = 0;
	npc_type->loottable_id = 0;
	npc_type->texture = 1;
	npc_type->light = 1;
	npc_type->size = 1;
	npc_type->runspeed = 0;
	npc_type->merchanttype = 1;
	npc_type->bodytype = 1;
	npc_type->show_name = true;
	npc_type->findable = true;
	strn0cpy(npc_type->special_abilities, "24,1^35,1", 512);

	auto node_position = glm::vec4(position.x, position.y, position.z, position.w);
	auto npc = new NPC(npc_type, nullptr, node_position, GravityBehavior::Flying);
	npc->name[strlen(npc->name) - 3] = (char) NULL;
	npc->GiveNPCTypeData(npc_type);
	npc->SetEntityVariable("grid_id", itoa(grid_id));
	entity_list.AddNPC(npc);
}

NPC * NPC::SpawnZonePointNodeNPC(std::string name, const glm::vec4 &position)
{
	auto npc_type = new NPCType;
	memset(npc_type, 0, sizeof(NPCType));

	char node_name[64];
	strn0cpy(node_name, name.c_str(), 64);

	strcpy(npc_type->name, entity_list.MakeNameUnique(node_name));

	npc_type->current_hp       = 4000000;
	npc_type->max_hp           = 4000000;
	npc_type->race             = 2254;
	npc_type->gender           = 2;
	npc_type->class_           = 9;
	npc_type->deity            = 1;
	npc_type->level            = 200;
	npc_type->npc_id           = 0;
	npc_type->loottable_id     = 0;
	npc_type->texture          = 1;
	npc_type->light            = 1;
	npc_type->size             = 5;
	npc_type->runspeed         = 0;
	npc_type->merchanttype     = 1;
	npc_type->bodytype         = 1;
	npc_type->show_name        = true;
	npc_type->findable         = true;

	strcpy(npc_type->special_abilities, "12,1^13,1^14,1^15,1^16,1^17,1^19,1^22,1^24,1^25,1^28,1^31,1^35,1^39,1^42,1");

	auto node_position = glm::vec4(position.x, position.y, position.z, position.w);
	auto npc           = new NPC(npc_type, nullptr, node_position, GravityBehavior::Flying);

	npc->name[strlen(npc->name)-3] = (char) NULL;

	npc->GiveNPCTypeData(npc_type);

	entity_list.AddNPC(npc);

	return npc;
}

NPC * NPC::SpawnNodeNPC(std::string name, std::string last_name, const glm::vec4 &position) {
	auto npc_type = new NPCType;
	memset(npc_type, 0, sizeof(NPCType));

	strncpy(npc_type->name, name.c_str(), 60);

	npc_type->current_hp       = 4000000;
	npc_type->max_hp           = 4000000;
	npc_type->race             = 127;
	npc_type->gender           = 0;
	npc_type->class_           = 9;
	npc_type->deity            = 1;
	npc_type->level            = 200;
	npc_type->npc_id           = 0;
	npc_type->loottable_id     = 0;
	npc_type->texture          = 1;
	npc_type->light            = 1;
	npc_type->size             = 3;
	npc_type->d_melee_texture1 = 1;
	npc_type->d_melee_texture2 = 1;
	npc_type->merchanttype     = 1;
	npc_type->bodytype         = 1;
	npc_type->show_name        = true;
	npc_type->findable         = true;
	npc_type->runspeed         = 1.25;

	strcpy(npc_type->special_abilities, "12,1^13,1^14,1^15,1^16,1^17,1^19,1^22,1^24,1^25,1^28,1^31,1^35,1^39,1^42,1");

	auto node_position = glm::vec4(position.x, position.y, position.z, position.w);
	auto npc           = new NPC(npc_type, nullptr, node_position, GravityBehavior::Flying);

	npc->name[strlen(npc->name) - 3] = (char) NULL;

	npc->GiveNPCTypeData(npc_type);

	entity_list.AddNPC(npc, true, true);

	return npc;
}

NPC* NPC::SpawnNPC(const char* spawncommand, const glm::vec4& position, Client* client) {
	if (spawncommand == 0 || spawncommand[0] == 0) {
		return 0;
	}
	else {
		Seperator sep(spawncommand);
		//Lets see if someone didn't fill out the whole #spawn function properly
		if (!sep.IsNumber(1)) {
			sprintf(sep.arg[1], "1");
		}

		if (!sep.IsNumber(2)) {
			sprintf(sep.arg[2], "1");
		}

		if (!sep.IsNumber(3)) {
			sprintf(sep.arg[3], "0");
		}

		if (Strings::ToInt(sep.arg[4]) > 2100000000 || Strings::ToInt(sep.arg[4]) <= 0) {
			sprintf(sep.arg[4], " ");
		}

		if (!strcmp(sep.arg[5], "-")) {
			sprintf(sep.arg[5], " ");
		}

		if (!sep.IsNumber(5)) {
			sprintf(sep.arg[5], " ");
		}

		if (!sep.IsNumber(6)) {
			sprintf(sep.arg[6], "1");
		}

		if (!sep.IsNumber(8)) {
			sprintf(sep.arg[8], "0");
		}

		if (!sep.IsNumber(9)) {
			sprintf(sep.arg[9], "0");
		}

		if (!sep.IsNumber(7)) {
			sprintf(sep.arg[7], "0");
		}

		if (!strcmp(sep.arg[4], "-")) {
			sprintf(sep.arg[4], " ");
		}

		if (!sep.IsNumber(10)) {    // bodytype
			sprintf(sep.arg[10], "0");
		}

		//Calc MaxHP if client neglected to enter it...
		if (sep.arg[4] && !sep.IsNumber(4)) {
			sprintf(sep.arg[4], "1");
		}

		// Autoselect NPC Gender
		if (sep.arg[5][0] == 0) {
			sprintf(sep.arg[5], "%i", (int) Mob::GetDefaultGender(Strings::ToInt(sep.arg[1])));
		}

		//Time to create the NPC!!
		auto npc_type = new NPCType;
		memset(npc_type, 0, sizeof(NPCType));

		strncpy(npc_type->name, sep.arg[0], 60);

		npc_type->current_hp       = Strings::ToInt(sep.arg[4]);
		npc_type->max_hp           = Strings::ToInt(sep.arg[4]);
		npc_type->race             = Strings::ToInt(sep.arg[1]);
		npc_type->gender           = Strings::ToInt(sep.arg[5]);
		npc_type->class_           = Strings::ToInt(sep.arg[6]);
		npc_type->deity            = 1;
		npc_type->level            = Strings::ToInt(sep.arg[2]);
		npc_type->npc_id           = 0;
		npc_type->loottable_id     = 0;
		npc_type->texture          = Strings::ToInt(sep.arg[3]);
		npc_type->light            = 0;
		npc_type->runspeed         = 1.25f;
		npc_type->d_melee_texture1 = Strings::ToUnsignedInt(sep.arg[7]);
		npc_type->d_melee_texture2 = Strings::ToUnsignedInt(sep.arg[8]);
		npc_type->merchanttype     = Strings::ToInt(sep.arg[9]);
		npc_type->bodytype         = Strings::ToInt(sep.arg[10]);

		npc_type->STR = 0;
		npc_type->STA = 0;
		npc_type->DEX = 0;
		npc_type->AGI = 0;
		npc_type->INT = 0;
		npc_type->WIS = 0;
		npc_type->CHA = 0;

		npc_type->attack_delay = 3000;

		npc_type->prim_melee_type = static_cast<uint8>(EQ::skills::SkillHandtoHand);
		npc_type->sec_melee_type  = static_cast<uint8>(EQ::skills::SkillHandtoHand);

		auto npc = new NPC(npc_type, nullptr, position, GravityBehavior::Water);
		npc->GiveNPCTypeData(npc_type);

		entity_list.AddNPC(npc);

		if (client) {
			// Notify client of spawn data
			client->Message(Chat::White, fmt::format("Name | {}", npc->name).c_str());
			client->Message(Chat::White, fmt::format("Level | {}", npc->level).c_str());
			client->Message(Chat::White, fmt::format("Health | {}", npc->max_hp).c_str());
			client->Message(Chat::White, fmt::format("Race | {} ({})", GetRaceIDName(npc->race), npc->race).c_str());
			client->Message(Chat::White, fmt::format("Class | {} ({})", GetClassIDName(npc->class_), npc->class_).c_str());
			client->Message(Chat::White, fmt::format("Gender | {} ({})", GetGenderName(npc->gender), npc->gender).c_str());
			client->Message(Chat::White, fmt::format("Texture | {}", npc->texture).c_str());

			if (npc->d_melee_texture1 || npc->d_melee_texture2) {
				client->Message(
					Chat::White,
					fmt::format(
						"Weapon Item Number | Primary: {} Secondary: {}",
						npc->d_melee_texture1,
						npc->d_melee_texture2
					).c_str()
				);
			}

			if (npc->MerchantType) {
				client->Message(Chat::White, fmt::format("Merchant ID | {}", npc->MerchantType).c_str());
			}

			if (npc->bodytype) {
				client->Message(
					Chat::White,
					fmt::format(
						"Body Type | {} ({})",
						BodyType::GetName(npc->bodytype),
						npc->bodytype
					).c_str()
				);
			}

			client->Message(Chat::White, "New NPC spawned!");
		}

		return npc;
	}
}

uint32 ZoneDatabase::CreateNewNPCCommand(
	const std::string& zone,
	uint32 instance_version,
	Client* c,
	NPC* n,
	uint32 extra
)
{
	auto e = NpcTypesRepository::NewEntity();

	e.id              = NpcTypesRepository::GetOpenIDInZoneRange(*this, c->GetZoneID());
	e.name            = Strings::RemoveNumbers(n->GetName());
	e.level           = n->GetLevel();
	e.race            = n->GetRace();
	e.class_          = n->GetClass();
	e.hp              = n->GetMaxHP();
	e.mana            = n->GetMaxMana();
	e.gender          = n->GetGender();
	e.texture         = n->GetTexture();
	e.helmtexture     = n->GetHelmTexture();
	e.size            = n->GetSize();
	e.loottable_id    = n->GetLoottableID();
	e.merchant_id     = n->MerchantType;
	e.runspeed        = n->GetRunspeed();
	e.walkspeed       = n->GetWalkspeed();
	e.prim_melee_type = n->GetPrimSkill();
	e.sec_melee_type  = n->GetSecSkill();

	e.bodytype        = n->GetBodyType();
	e.npc_faction_id  = n->GetNPCFactionID();
	e.aggroradius     = n->GetAggroRange();
	e.assistradius    = n->GetAssistRange();

	e.AC              = n->GetAC();
	e.ATK             = n->GetATK();
	e.STR             = n->GetSTR();
	e.STA             = n->GetSTA();
	e.AGI             = n->GetAGI();
	e.DEX             = n->GetDEX();
	e.WIS             = n->GetWIS();
	e._INT            = n->GetINT();
	e.CHA             = n->GetCHA();

	e.PR              = n->GetPR();
	e.MR              = n->GetMR();
	e.DR              = n->GetDR();
	e.FR              = n->GetFR();
	e.CR              = n->GetCR();
	e.Corrup          = n->GetCorrup();
	e.PhR             = n->GetPhR();

	e.Accuracy        = n->GetAccuracyRating();
	e.slow_mitigation = n->GetSlowMitigation();
	e.mindmg          = n->GetMinDMG();
	e.maxdmg          = n->GetMaxDMG();
	e.hp_regen_rate   = n->GetHPRegen();
	e.hp_regen_per_second = n->GetHPRegenPerSecond();
	//e.attack_delay    = n->GetAttackDelay(); // Attack delay isn't copying correctly, 3000 becomes 18,400 in the copied NPC?
	e.spellscale      = n->GetSpellScale();
	e.healscale       = n->GetHealScale();
	e.Avoidance       = n->GetAvoidanceRating();
	e.heroic_strikethrough = n->GetHeroicStrikethrough();

	e.see_hide             = n->SeeHide();
	e.see_improved_hide    = n->SeeImprovedHide();
	e.see_invis            = n->SeeInvisible();
	e.see_invis_undead     = n->SeeInvisibleUndead();
	e.npc_tint_id          = n->GetNpcTintId();


	e = NpcTypesRepository::InsertOne(*this, e);

	if (!e.id) {
		return false;
	}

	auto sg = SpawngroupRepository::NewEntity();

	sg.name = fmt::format(
		"{}_{}_{}",
		zone,
		Strings::Escape(n->GetName()),
		Timer::GetCurrentTime()
	);

	sg = SpawngroupRepository::InsertOne(*this, sg);

	if (!sg.id) {
		return false;
	}

	n->SetSpawnGroupId(sg.id);
	n->SetNPCTypeID(e.id);

	auto s2 = Spawn2Repository::NewEntity();

	s2.zone         = zone;
	s2.version      = instance_version;
	s2.x            = n->GetX();
	s2.y            = n->GetY();
	s2.z            = n->GetZ();
	s2.respawntime  = extra > 0 ? extra : 1200;
	s2.heading      = n->GetHeading();
	s2.spawngroupID = sg.id;

	s2 = Spawn2Repository::InsertOne(*this, s2);

	if (!s2.id) {
		return false;
	}

	auto se = SpawnentryRepository::NewEntity();

	se.spawngroupID = sg.id;
	se.npcID        = e.id;
	se.chance       = 100;

	se = SpawnentryRepository::InsertOne(*this, se);

	if (!se.spawngroupID) {
		return false;
	}

	return true;
}

uint32 ZoneDatabase::AddNewNPCSpawnGroupCommand(
	const std::string& zone,
	uint32 instance_version,
	Client* c,
	NPC* n,
	uint32 in_respawn_time
)
{
	auto sg = SpawngroupRepository::InsertOne(
		*this,
		SpawngroupRepository::Spawngroup{
			.name = fmt::format(
				"{}_{}_{}",
				zone,
				Strings::Escape(n->GetName()),
				Timer::GetCurrentTime()
			)
		}
	);

	if (!sg.id) {
		return 0;
	}

	uint32 respawn_time = 1200;
	uint32 spawn_id     = 0;

	if (in_respawn_time) {
		respawn_time = in_respawn_time;
	} else if (n->respawn2 && n->respawn2->RespawnTimer()) {
		respawn_time = n->respawn2->RespawnTimer();
	}

	auto s2 = Spawn2Repository::NewEntity();

	s2.zone         = zone;
	s2.version      = instance_version;
	s2.x            = n->GetX();
	s2.y            = n->GetY();
	s2.z            = n->GetZ();
	s2.heading      = n->GetHeading();
	s2.respawntime  = respawn_time;
	s2.spawngroupID = sg.id;

	s2 = Spawn2Repository::InsertOne(*this, s2);

	if (!s2.id) {
		return 0;
	}

	auto se = SpawnentryRepository::NewEntity();

	se.spawngroupID = sg.id;
	se.npcID        = n->GetNPCTypeID();
	se.chance       = 100;

	se = SpawnentryRepository::InsertOne(*this, se);

	if (!se.spawngroupID) {
		return 0;
	}

	return s2.id;
}

uint32 ZoneDatabase::UpdateNPCTypeAppearance(Client* c, NPC* n)
{
	auto e = NpcTypesRepository::FindOne(*this, n->GetNPCTypeID());

	e.name         = Strings::RemoveNumbers(n->GetName());
	e.level        = n->GetLevel();
	e.race         = n->GetRace();
	e.class_       = n->GetClass();
	e.hp           = n->GetMaxHP();
	e.gender       = n->GetGender();
	e.texture      = n->GetTexture();
	e.helmtexture  = n->GetHelmTexture();
	e.size         = n->GetSize();
	e.loottable_id = n->GetLoottableID();
	e.merchant_id  = n->MerchantType;
	e.face         = n->GetLuclinFace();
	e.npc_tint_id  = n->GetNpcTintId();

	const int updated = NpcTypesRepository::UpdateOne(*this, e);

	return updated;
}

uint32 ZoneDatabase::DeleteSpawnLeaveInNPCTypeTable(const std::string& zone, Client* c, NPC* n, uint32 remove_spawngroup_id)
{
	if (!n->respawn2) {
		return 0;
	}

	const auto& l = Spawn2Repository::GetWhere(
		*this,
		fmt::format(
			"`id` = {} AND `zone` = '{}' AND `spawngroupID` = {}",
			n->respawn2->GetID(),
			zone,
			n->GetSpawnGroupId()
		)
	);

	if (l.empty()) {
		return 0;
	}

	auto e = l.front();

	if (!Spawn2Repository::DeleteOne(*this, e.id)) {
		return 0;
	}

	if (remove_spawngroup_id > 0) {
		if (!SpawngroupRepository::DeleteOne(*this, e.spawngroupID)) {
			return 0;
		}

		if (!SpawnentryRepository::DeleteOne(*this, e.spawngroupID)) {
			return 0;
		}
	}

	return 1;
}

uint32 ZoneDatabase::DeleteSpawnRemoveFromNPCTypeTable(
	const std::string& zone,
	uint32 instance_version,
	Client* c,
	NPC* n
)
{
	const auto& l = Spawn2Repository::GetWhere(
		*this,
		fmt::format(
			"`zone` = '{}' AND (`version` = {} OR `version` = -1) AND `spawngroupID` = {}",
			zone,
			instance_version,
			n->GetSpawnGroupId()
		)
	);

	if (l.empty()) {
		return 0;
	}

	auto e = l.front();

	if (!Spawn2Repository::DeleteOne(*this, e.id)) {
		return 0;
	}

	if (!SpawngroupRepository::DeleteOne(*this, e.spawngroupID)) {
		return 0;
	}

	if (!SpawnentryRepository::DeleteOne(*this, e.spawngroupID)) {
		return 0;
	}

	if (!NpcTypesRepository::DeleteOne(*this, n->GetNPCTypeID())) {
		return 0;
	}

	return 1;
}

uint32 ZoneDatabase::AddSpawnFromSpawnGroup(
	const std::string& zone,
	uint32 instance_version,
	Client* c,
	NPC* n,
	uint32 extra
)
{
	auto e = Spawn2Repository::NewEntity();

	e.zone         = zone;
	e.version      = instance_version;
	e.x            = c->GetX();
	e.y            = c->GetY();
	e.z            = c->GetZ();
	e.heading      = c->GetHeading();
	e.respawntime  = extra > 0 ? extra : 1200;
	e.spawngroupID = n->GetSpawnGroupId();

	e = Spawn2Repository::InsertOne(*this, e);

	if (!e.id) {
		return 0;
	}

	return 1;
}

uint32 ZoneDatabase::AddNPCTypes(
	const std::string& zone,
	uint32 instance_version,
	Client* c,
	NPC* n,
	uint32 spawngroup_id
)
{
	auto e = NpcTypesRepository::NewEntity();

	e.name            = Strings::RemoveNumbers(n->GetName());
	e.level           = n->GetLevel();
	e.race            = n->GetRace();
	e.class_          = n->GetClass();
	e.hp              = n->GetMaxHP();
	e.gender          = n->GetGender();
	e.texture         = n->GetTexture();
	e.helmtexture     = n->GetHelmTexture();
	e.size            = n->GetSize();
	e.loottable_id    = n->GetLoottableID();
	e.merchant_id     = n->MerchantType;
	e.face            = n->GetLuclinFace();
	e.runspeed        = n->GetRunspeed();
	e.prim_melee_type = static_cast<uint8_t>(EQ::skills::SkillHandtoHand);
	e.sec_melee_type  = static_cast<uint8_t>(EQ::skills::SkillHandtoHand);
	e.npc_tint_id     = n->GetNpcTintId();

	e = NpcTypesRepository::InsertOne(*this, e);

	if (!e.id) {
		return 0;
	}

	if (c) {
		c->Message(
			Chat::White,
			fmt::format(
				"NPC Created | ID: {} Name: {}",
				e.id,
				e.name
			).c_str()
		);
	}

	return 1;
}

uint32 ZoneDatabase::NPCSpawnDB(
	uint8 command,
	const std::string& zone,
	uint32 instance_version,
	Client *c,
	NPC* n,
	uint32 extra
)
{
	switch (command) {
		case NPCSpawnTypes::CreateNewSpawn: {
			return CreateNewNPCCommand(zone, instance_version, c, n, extra);
		}
		case NPCSpawnTypes::AddNewSpawngroup: {
			return AddNewNPCSpawnGroupCommand(zone, instance_version, c, n, extra);
		}
		case NPCSpawnTypes::UpdateAppearance: {
			return UpdateNPCTypeAppearance(c, n);
		}
		case NPCSpawnTypes::RemoveSpawn: {
			return DeleteSpawnLeaveInNPCTypeTable(zone, c, n, extra);
		}
		case NPCSpawnTypes::DeleteSpawn: {
			return DeleteSpawnRemoveFromNPCTypeTable(zone, instance_version, c, n);
		}
		case NPCSpawnTypes::AddSpawnFromSpawngroup: {
			return AddSpawnFromSpawnGroup(zone, instance_version, c, n, extra);
		}
		case NPCSpawnTypes::CreateNewNPC: {
			return AddNPCTypes(zone, instance_version, c, n, extra);
		}
	}

	return false;
}

uint32 NPC::GetEquipmentMaterial(uint8 material_slot) const
{
	const uint32 texture_profile_material = GetTextureProfileMaterial(material_slot);

	Log(Logs::Detail, Logs::MobAppearance, "[%s] material_slot: %u",
		clean_name,
		material_slot
	);

	if (texture_profile_material) {
		return texture_profile_material;
	}

	if (material_slot >= EQ::textures::materialCount) {
		return 0;
	}

	int16 invslot = EQ::InventoryProfile::CalcSlotFromMaterial(material_slot);
	if (invslot == INVALID_INDEX) {
		return 0;
	}

	if (equipment[invslot] == 0) {
		switch (material_slot) {
			case EQ::textures::armorHead:
				return helmtexture;
			case EQ::textures::armorChest:
				return texture;
			case EQ::textures::armorArms:
				return armtexture;
			case EQ::textures::armorWrist:
				return bracertexture;
			case EQ::textures::armorHands:
				return handtexture;
			case EQ::textures::armorLegs:
				return legtexture;
			case EQ::textures::armorFeet:
				return feettexture;
			case EQ::textures::weaponPrimary:
				return d_melee_texture1;
			case EQ::textures::weaponSecondary:
				return d_melee_texture2;
			default:
				//they have nothing in the slot, and its not a special slot... they get nothing.
				return 0;
		}
	}

	//they have some loot item in this slot, pass it up to the default handler
	return Mob::GetEquipmentMaterial(material_slot);
}

uint32 NPC::GetMaxDamage(uint8 tlevel)
{
	uint64 dmg = 0;
	if (tlevel < 40)
		dmg = tlevel*2+2;
	else if (tlevel < 50)
		dmg = level*25/10+2;
	else if (tlevel < 60)
		dmg = (tlevel*3+2)+((tlevel-50)*30);
	else
		dmg = (tlevel*3+2)+((tlevel-50)*35);
	return dmg;
}

void NPC::PickPocket(Client* thief)
{
	thief->CheckIncreaseSkill(EQ::skills::SkillPickPockets, nullptr, 5);

	//make sure were allowed to target them:
	int over_level = GetLevel();
	if(over_level > (thief->GetLevel() + THIEF_PICKPOCKET_OVER)) {
		thief->Message(Chat::Red, "You are too inexperienced to pick pocket this target");
		thief->SendPickPocketResponse(this, 0, PickPocketFailed);
		//should we check aggro
		return;
	}

	if(zone->random.Roll(5)) {
		if (zone->CanDoCombat())
			AddToHateList(thief, 50);
		Say("Stop thief!");
		thief->Message(Chat::Red, "You are noticed trying to steal!");
		thief->SendPickPocketResponse(this, 0, PickPocketFailed);
		return;
	}

	int steal_skill = thief->GetSkill(EQ::skills::SkillPickPockets);
	int steal_chance = steal_skill * 100 / (5 * over_level + 5);

	// Determine whether to steal money or an item.
	uint32 money[6] = {0, ((steal_skill >= 125) ? (GetPlatinum()) : (0)), ((steal_skill >= 60) ? (GetGold()) : (0)), GetSilver(),
					   GetCopper(), 0 };
	bool has_coin = ((money[PickPocketPlatinum] | money[PickPocketGold] | money[PickPocketSilver] | money[PickPocketCopper]) != 0);
	bool steal_item = (steal_skill >= steal_chance && (zone->random.Roll(50) || !has_coin));

	// still needs to have FindFreeSlot vs PutItemInInventory issue worked out
	while (steal_item) {
		std::vector<std::pair<const EQ::ItemData*, uint16>> loot_selection; // <const ItemData*, charges>
		for (auto item_iter : m_loot_items) {
			if (!item_iter || !item_iter->item_id)
				continue;

			auto item_test = database.GetItem(item_iter->item_id);
			if (item_test->Magic || !item_test->NoDrop || item_test->IsClassBag() || thief->CheckLoreConflict(item_test) || item_iter->equip_slot != EQ::invslot::SLOT_INVALID)
				continue;

			loot_selection.emplace_back(std::make_pair(item_test, ((item_test->Stackable) ? (1) : (item_iter->charges))));
		}
		if (loot_selection.empty()) {
			steal_item = false;
			break;
		}

		int random = zone->random.Int(0, (loot_selection.size() - 1));
		uint16 slot_id = thief->GetInv().FindFreeSlot(false, true, (loot_selection[random].first->Size), (loot_selection[random].first->ItemType == EQ::item::ItemTypeArrow));
		if (slot_id == INVALID_INDEX) {
			steal_item = false;
			break;
		}

		auto item_inst = database.CreateItem(loot_selection[random].first, loot_selection[random].second);
		if (item_inst == nullptr) {
			steal_item = false;
			break;
		}

		// Successful item pickpocket
		if (item_inst->IsStackable() && RuleB(Character, UseStackablePickPocketing)) {
			if (!thief->TryStacking(item_inst, ItemPacketTrade, false, false)) {
				thief->PutItemInInventory(slot_id, *item_inst);
				thief->SendItemPacket(slot_id, item_inst, ItemPacketTrade);
			}
		}
		else {
			thief->PutItemInInventory(slot_id, *item_inst);
			thief->SendItemPacket(slot_id, item_inst, ItemPacketTrade);
		}
		RemoveItem(item_inst->GetID());
		thief->SendPickPocketResponse(this, 0, PickPocketItem, item_inst->GetItem());
		safe_delete(item_inst);

		return;
	}

	while (!steal_item && has_coin) {
		uint32 coin_amount = zone->random.Int(1, (steal_skill / 25) + 1);

		int coin_type = PickPocketPlatinum;
		while (coin_type <= PickPocketCopper) {
			if (money[coin_type]) {
				if (coin_amount > money[coin_type])
					coin_amount = money[coin_type];
				break;
			}
			++coin_type;
		}
		if (coin_type > PickPocketCopper)
			break;

		memset(money, 0, (sizeof(int) * 6));
		money[coin_type] = coin_amount;

		if (zone->random.Roll(steal_chance)) { // Successful coin pickpocket
			switch (coin_type) {
			case PickPocketPlatinum:
				SetPlatinum(GetPlatinum() - coin_amount);
				break;
			case PickPocketGold:
				SetGold(GetGold() - coin_amount);
				break;
			case PickPocketSilver:
				SetSilver(GetSilver() - coin_amount);
				break;
			case PickPocketCopper:
				SetCopper(GetCopper() - coin_amount);
				break;
			default: // has_coin..but, doesn't have coin?
				thief->SendPickPocketResponse(this, 0, PickPocketFailed);
				return;
			}

			thief->AddMoneyToPP(money[PickPocketCopper], money[PickPocketSilver], money[PickPocketGold], money[PickPocketPlatinum], false);
			thief->SendPickPocketResponse(this, coin_amount, coin_type);
			return;
		}

		thief->SendPickPocketResponse(this, 0, PickPocketFailed);
		return;
	}

	thief->Message(Chat::White, "This target's pockets are empty");
	thief->SendPickPocketResponse(this, 0, PickPocketFailed);
}

void NPC::Disarm(Client* client, int chance) {
	// disarm primary if available, otherwise disarm secondary
	const EQ::ItemData* weapon = NULL;
	uint8 eslot = 0xFF;
	if (equipment[EQ::invslot::slotPrimary] != 0)
		eslot = EQ::invslot::slotPrimary;
	else if (equipment[EQ::invslot::slotSecondary] != 0)
		eslot = EQ::invslot::slotSecondary;
	if (eslot != 0xFF) {
		if (zone->random.Int(0, 1000) <= chance) {
			weapon = database.GetItem(equipment[eslot]);
			if (weapon) {
				if (!weapon->Magic && weapon->NoDrop != 0) {
					int16               charges = -1;
					LootItems::iterator cur, end;
					cur = m_loot_items.begin();
					end = m_loot_items.end();
					// Get charges for the item in the loot table
					for (; cur != end; cur++) {
						LootItem * citem = *cur;
						if (citem->item_id == weapon->ID) {
							charges = citem->charges;
							break;
						}
					}
					EQ::ItemInstance *inst = NULL;
					inst = database.CreateItem(weapon->ID, charges);
					// Remove item from loot table
					RemoveItem(weapon->ID);
					CalcBonuses();
					if (inst) {
						// create a ground item
						Object* object = new Object(inst, GetX(), GetY(), GetZ(), 0.0f, 300000);
						entity_list.AddObject(object, true);
						object->StartDecay();
						safe_delete(inst);
					}
				}
			}
			// Update Appearance
			equipment[eslot] = 0;
			int matslot = eslot == EQ::invslot::slotPrimary ? EQ::textures::weaponPrimary : EQ::textures::weaponSecondary;
			if (matslot != -1)
				SendWearChange(matslot);
			if ((CastToMob()->GetBodyType() == BodyType::Humanoid || CastToMob()->GetBodyType() == BodyType::Summoned) && eslot == EQ::invslot::slotPrimary)
				Say("Ahh! My weapon!");
			client->MessageString(Chat::Skills, DISARM_SUCCESS, GetCleanName());
			if (chance != 1000)
				client->CheckIncreaseSkill(EQ::skills::SkillDisarm, nullptr, 4);
			return;
		}
		client->MessageString(Chat::Skills, DISARM_FAILED);
		if (chance != 1000)
			client->CheckIncreaseSkill(EQ::skills::SkillDisarm, nullptr, 2);
		return;
	}
	client->MessageString(Chat::Skills, DISARM_FAILED);
}

void Mob::NPCSpecialAttacks(const char* parse, int permtag, bool reset, bool remove) {
	if(reset)
	{
		ClearSpecialAbilities();
	}

	const char* orig_parse = parse;
	while (*parse)
	{
		switch(*parse)
		{
			case 'E':
				SetSpecialAbility(SpecialAbility::Enrage, remove ? 0 : 1);
				break;
			case 'F':
				SetSpecialAbility(SpecialAbility::Flurry, remove ? 0 : 1);
				break;
			case 'R':
				SetSpecialAbility(SpecialAbility::Rampage, remove ? 0 : 1);
				break;
			case 'r':
				SetSpecialAbility(SpecialAbility::AreaRampage, remove ? 0 : 1);
				break;
			case 'S':
				if(remove) {
					SetSpecialAbility(SpecialAbility::Summon, 0);
					StopSpecialAbilityTimer(SpecialAbility::Summon);
				} else {
					SetSpecialAbility(SpecialAbility::Summon, 1);
				}
			break;
			case 'T':
				SetSpecialAbility(SpecialAbility::TripleAttack, remove ? 0 : 1);
				break;
			case 'Q':
				//quad requires triple to work properly
				if(remove) {
					SetSpecialAbility(SpecialAbility::QuadrupleAttack, 0);
				} else {
					SetSpecialAbility(SpecialAbility::TripleAttack, 1);
					SetSpecialAbility(SpecialAbility::QuadrupleAttack, 1);
					}
				break;
			case 'b':
				SetSpecialAbility(SpecialAbility::BaneAttack, remove ? 0 : 1);
				break;
			case 'm':
				SetSpecialAbility(SpecialAbility::MagicalAttack, remove ? 0 : 1);
				break;
			case 'U':
				SetSpecialAbility(SpecialAbility::SlowImmunity, remove ? 0 : 1);
				break;
			case 'M':
				SetSpecialAbility(SpecialAbility::MesmerizeImmunity, remove ? 0 : 1);
				break;
			case 'C':
				SetSpecialAbility(SpecialAbility::CharmImmunity, remove ? 0 : 1);
				break;
			case 'N':
				SetSpecialAbility(SpecialAbility::StunImmunity, remove ? 0 : 1);
				break;
			case 'I':
				SetSpecialAbility(SpecialAbility::SnareImmunity, remove ? 0 : 1);
				break;
			case 'D':
				SetSpecialAbility(SpecialAbility::FearImmunity, remove ? 0 : 1);
				break;
			case 'K':
				SetSpecialAbility(SpecialAbility::DispellImmunity, remove ? 0 : 1);
				break;
			case 'A':
				SetSpecialAbility(SpecialAbility::MeleeImmunity, remove ? 0 : 1);
				break;
			case 'B':
				SetSpecialAbility(SpecialAbility::MagicImmunity, remove ? 0 : 1);
				break;
			case 'f':
				SetSpecialAbility(SpecialAbility::FleeingImmunity, remove ? 0 : 1);
				break;
			case 'O':
				SetSpecialAbility(SpecialAbility::MeleeImmunityExceptBane, remove ? 0 : 1);
				break;
			case 'W':
				SetSpecialAbility(SpecialAbility::MeleeImmunityExceptMagical, remove ? 0 : 1);
				break;
			case 'H':
				SetSpecialAbility(SpecialAbility::AggroImmunity, remove ? 0 : 1);
				break;
			case 'G':
				SetSpecialAbility(SpecialAbility::BeingAggroImmunity, remove ? 0 : 1);
				break;
			case 'g':
				SetSpecialAbility(SpecialAbility::CastingFromRangeImmunity, remove ? 0 : 1);
				break;
			case 'd':
				SetSpecialAbility(SpecialAbility::FeignDeathImmunity, remove ? 0 : 1);
				break;
			case 'Y':
				SetSpecialAbility(SpecialAbility::RangedAttack, remove ? 0 : 1);
				break;
			case 'L':
				SetSpecialAbility(SpecialAbility::DualWield, remove ? 0 : 1);
				break;
			case 't':
				SetSpecialAbility(SpecialAbility::TunnelVision, remove ? 0 : 1);
				break;
			case 'n':
				SetSpecialAbility(SpecialAbility::NoBuffHealFriends, remove ? 0 : 1);
				break;
			case 'p':
				SetSpecialAbility(SpecialAbility::PacifyImmunity, remove ? 0 : 1);
				break;
			case 'J':
				SetSpecialAbility(SpecialAbility::Leash, remove ? 0 : 1);
				break;
			case 'j':
				SetSpecialAbility(SpecialAbility::Tether, remove ? 0 : 1);
				break;
			case 'o':
				SetSpecialAbility(SpecialAbility::DestructibleObject, remove ? 0 : 1);
				SetDestructibleObject(remove ? true : false);
				break;
			case 'Z':
				SetSpecialAbility(SpecialAbility::HarmFromClientImmunity, remove ? 0 : 1);
				break;
			case 'i':
				SetSpecialAbility(SpecialAbility::TauntImmunity, remove ? 0 : 1);
				break;
			case 'e':
				SetSpecialAbility(SpecialAbility::AlwaysFlee, remove ? 0 : 1);
				break;
			case 'h':
				SetSpecialAbility(SpecialAbility::FleePercent, remove ? 0 : 1);
				break;

			default:
				break;
		}
		parse++;
	}

	if(permtag == 1 && GetNPCTypeID() > 0)
	{
		if(content_db.SetSpecialAttkFlag(GetNPCTypeID(), orig_parse))
		{
			LogInfo("NPCTypeID: [{}] flagged to [{}] for Special Attacks.\n",GetNPCTypeID(),orig_parse);
		}
	}
}

bool Mob::HasNPCSpecialAtk(const char* parse) {

	bool HasAllAttacks = true;

	while (*parse && HasAllAttacks == true)
	{
		switch(*parse)
		{
			case 'E':
				if (!GetSpecialAbility(SpecialAbility::Enrage))
					HasAllAttacks = false;
				break;
			case 'F':
				if (!GetSpecialAbility(SpecialAbility::Flurry))
					HasAllAttacks = false;
				break;
			case 'R':
				if (!GetSpecialAbility(SpecialAbility::Rampage))
					HasAllAttacks = false;
				break;
			case 'r':
				if (!GetSpecialAbility(SpecialAbility::AreaRampage))
					HasAllAttacks = false;
				break;
			case 'S':
				if (!GetSpecialAbility(SpecialAbility::Summon))
					HasAllAttacks = false;
				break;
			case 'T':
				if (!GetSpecialAbility(SpecialAbility::TripleAttack))
					HasAllAttacks = false;
				break;
			case 'Q':
				if (!GetSpecialAbility(SpecialAbility::QuadrupleAttack))
					HasAllAttacks = false;
				break;
			case 'b':
				if (!GetSpecialAbility(SpecialAbility::BaneAttack))
					HasAllAttacks = false;
				break;
			case 'm':
				if (!GetSpecialAbility(SpecialAbility::MagicalAttack))
					HasAllAttacks = false;
				break;
			case 'U':
				if (!GetSpecialAbility(SpecialAbility::SlowImmunity))
					HasAllAttacks = false;
				break;
			case 'M':
				if (!GetSpecialAbility(SpecialAbility::MesmerizeImmunity))
					HasAllAttacks = false;
				break;
			case 'C':
				if (!GetSpecialAbility(SpecialAbility::CharmImmunity))
					HasAllAttacks = false;
				break;
			case 'N':
				if (!GetSpecialAbility(SpecialAbility::StunImmunity))
					HasAllAttacks = false;
				break;
			case 'I':
				if (!GetSpecialAbility(SpecialAbility::SnareImmunity))
					HasAllAttacks = false;
				break;
			case 'D':
				if (!GetSpecialAbility(SpecialAbility::FearImmunity))
					HasAllAttacks = false;
				break;
			case 'A':
				if (!GetSpecialAbility(SpecialAbility::MeleeImmunity))
					HasAllAttacks = false;
				break;
			case 'B':
				if (!GetSpecialAbility(SpecialAbility::MagicImmunity))
					HasAllAttacks = false;
				break;
			case 'f':
				if (!GetSpecialAbility(SpecialAbility::FleeingImmunity))
					HasAllAttacks = false;
				break;
			case 'O':
				if (!GetSpecialAbility(SpecialAbility::MeleeImmunityExceptBane))
					HasAllAttacks = false;
				break;
			case 'W':
				if (!GetSpecialAbility(SpecialAbility::MeleeImmunityExceptMagical))
					HasAllAttacks = false;
				break;
			case 'H':
				if (!GetSpecialAbility(SpecialAbility::AggroImmunity))
					HasAllAttacks = false;
				break;
			case 'G':
				if (!GetSpecialAbility(SpecialAbility::BeingAggroImmunity))
					HasAllAttacks = false;
				break;
			case 'g':
				if (!GetSpecialAbility(SpecialAbility::CastingFromRangeImmunity))
					HasAllAttacks = false;
				break;
			case 'd':
				if (!GetSpecialAbility(SpecialAbility::FeignDeathImmunity))
					HasAllAttacks = false;
				break;
			case 'Y':
				if (!GetSpecialAbility(SpecialAbility::RangedAttack))
					HasAllAttacks = false;
				break;
			case 'L':
				if (!GetSpecialAbility(SpecialAbility::DualWield))
					HasAllAttacks = false;
				break;
			case 't':
				if (!GetSpecialAbility(SpecialAbility::TunnelVision))
					HasAllAttacks = false;
				break;
			case 'n':
				if (!GetSpecialAbility(SpecialAbility::NoBuffHealFriends))
					HasAllAttacks = false;
				break;
			case 'p':
				if(!GetSpecialAbility(SpecialAbility::PacifyImmunity))
					HasAllAttacks = false;
				break;
			case 'J':
				if(!GetSpecialAbility(SpecialAbility::Leash))
					HasAllAttacks = false;
				break;
			case 'j':
				if(!GetSpecialAbility(SpecialAbility::Tether))
					HasAllAttacks = false;
				break;
			case 'o':
				if(!GetSpecialAbility(SpecialAbility::DestructibleObject))
				{
					HasAllAttacks = false;
					SetDestructibleObject(false);
				}
				break;
			case 'Z':
				if(!GetSpecialAbility(SpecialAbility::HarmFromClientImmunity)){
					HasAllAttacks = false;
				}
				break;
			case 'e':
				if(!GetSpecialAbility(SpecialAbility::AlwaysFlee))
					HasAllAttacks = false;
				break;
			case 'h':
				if(!GetSpecialAbility(SpecialAbility::FleePercent))
					HasAllAttacks = false;
				break;
			default:
				HasAllAttacks = false;
				break;
		}
		parse++;
	}

	return HasAllAttacks;
}

void NPC::FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho)
{
	Mob::FillSpawnStruct(ns, ForWho);
	PetOnSpawn(ns);
	ns->spawn.is_npc = 1;
	UpdateActiveLight();
	ns->spawn.light       = GetActiveLightType();
	ns->spawn.show_name   = NPCTypedata->show_name;
	ns->spawn.trader      = false;
	ns->spawn.npc_tint_id = GetNpcTintId();
}

void NPC::PetOnSpawn(NewSpawn_Struct* ns)
{
	//Basic settings to make sure swarm pets work properly.
	Mob *swarm_owner = nullptr;
	if (GetSwarmOwner()) {
		swarm_owner = entity_list.GetMobID(GetSwarmOwner());
	}

	if (swarm_owner) {
		if (swarm_owner->IsClient()) {
			SetPetOwnerClient(true); //Simple flag to determine if pet belongs to a client
			SetAllowBeneficial(true);//Allow temp pets to receive buffs and heals if owner is client.
			//This will allow CLIENT swarm pets NOT to be targeted with F8.
			ns->spawn.targetable_with_hotkey = false;
			no_target_hotkey = true;
		} else {
			//NPC cast swarm pets should still be targetable with F8.
			ns->spawn.targetable_with_hotkey = true;
			no_target_hotkey = false;
		}

		SetTempPet(true); //Simple mob flag for checking if temp pet
		swarm_owner->SetTempPetsActive(true); //Necessary fail safe flag set if mob ever had a swarm pet to ensure they are removed.
		swarm_owner->SetTempPetCount(swarm_owner->GetTempPetCount() + 1);

		//Not recommended if using above (However, this will work better on older clients).
		if (RuleB(Pets, UnTargetableSwarmPet)) {
			ns->spawn.bodytype = BodyType::NoTarget;
		}

		if (
			!IsCharmed() &&
			swarm_owner->IsClient() &&
			RuleB(Pets, ClientPetsUseOwnerNameInLastName)
		) {
			const auto& tmp_lastname = fmt::format("{}'s Pet", swarm_owner->GetName());
			if (tmp_lastname.size() < sizeof(ns->spawn.lastName)) {
				strn0cpy(ns->spawn.lastName, tmp_lastname.c_str(), sizeof(ns->spawn.lastName));
			}
		}

		if (swarm_owner->IsBot()) {
			SetPetOwnerBot(true);
		} else if (swarm_owner->IsNPC()) {
			SetPetOwnerNPC(true);
		}
	} else if (GetOwnerID()) {
		ns->spawn.is_pet = 1;

		if (!IsCharmed()) {
			const auto c = entity_list.GetClientByID(GetOwnerID());
			if (c) {
				SetPetOwnerClient(true);
				if (RuleB(Pets, ClientPetsUseOwnerNameInLastName)) {
					const auto& tmp_lastname = fmt::format("{}'s Pet", c->GetName());
					if (tmp_lastname.size() < sizeof(ns->spawn.lastName)) {
						strn0cpy(ns->spawn.lastName, tmp_lastname.c_str(), sizeof(ns->spawn.lastName));
					}
				}
			} else {
				Mob* owner = entity_list.GetMob(GetOwnerID());
				if (owner) {
					if (owner->IsBot()) {
						SetPetOwnerBot(true);
					} else if (owner->IsNPC()) {
						SetPetOwnerNPC(true);
					}
				}
			}
		}
	} else {
		ns->spawn.is_pet = 0;
	}
}

void NPC::SetLevel(uint8 in_level, bool command)
{
	if(in_level > level)
		SendLevelAppearance();
	level = in_level;
	SendAppearancePacket(AppearanceType::WhoLevel, in_level);
}

void NPC::ModifyNPCStat(const std::string& stat, const std::string& value)
{
	const std::string& stat_lower = Strings::ToLower(stat);

	const std::string& variable_key = fmt::format(
		"modify_stat_{}",
		stat_lower
	);

	SetEntityVariable(variable_key, value);

	LogNPCScaling("NPC::ModifyNPCStat: Key [{}] Value [{}] ", variable_key, value);

	if (stat_lower == "ac") {
		AC = Strings::ToInt(value);
		CalcAC();
	} else if (stat_lower == "str") {
		STR = Strings::ToInt(value);
	} else if (stat_lower == "sta") {
		STA = Strings::ToInt(value);
	} else if (stat_lower == "agi") {
		AGI = Strings::ToInt(value);
		CalcAC();
	} else if (stat_lower == "dex") {
		DEX = Strings::ToInt(value);
	} else if (stat_lower == "wis") {
		WIS = Strings::ToInt(value);
		CalcMaxMana();
	} else if (stat_lower == "int" || stat_lower == "_int") {
		INT = Strings::ToInt(value);
		CalcMaxMana();
	} else if (stat_lower == "cha") {
		CHA = Strings::ToInt(value);
	} else if (stat_lower == "max_hp") {
		base_hp = Strings::ToBigInt(value);

		CalcMaxHP();
		if (current_hp > max_hp) {
			current_hp = max_hp;
		}

	} else if (stat_lower == "max_mana") {
		npc_mana = Strings::ToUnsignedBigInt(value);
		CalcMaxMana();
		if (current_mana > max_mana) {
			current_mana = max_mana;
		}
	} else if (stat_lower == "mr") {
		MR = Strings::ToInt(value);
	} else if (stat_lower == "fr") {
		FR = Strings::ToInt(value);
	} else if (stat_lower == "cr") {
		CR = Strings::ToInt(value);
	} else if (stat_lower == "cor") {
		Corrup = Strings::ToInt(value);
	} else if (stat_lower == "pr") {
		PR = Strings::ToInt(value);
	} else if (stat_lower == "dr") {
		DR = Strings::ToInt(value);
	} else if (stat_lower == "phr") {
		PhR = Strings::ToInt(value);
	} else if (stat_lower == "runspeed") {
		runspeed       = Strings::ToFloat(value);
		base_runspeed  = (int) (runspeed * 40.0f);
		base_walkspeed = base_runspeed * 100 / 265;
		walkspeed      = ((float) base_walkspeed) * 0.025f;
		base_fearspeed = base_runspeed * 100 / 127;
		fearspeed      = ((float) base_fearspeed) * 0.025f;
		CalcBonuses();
	} else if (stat_lower == "special_attacks") {
		NPCSpecialAttacks(value.c_str(), 0, true);
	} else if (stat_lower == "special_abilities") {
		ProcessSpecialAbilities(value);
	} else if (stat_lower == "attack_speed") {
		attack_speed = Strings::ToFloat(value);
		CalcBonuses();
	} else if (stat_lower == "attack_delay") {
		/* TODO: fix DB */
		attack_delay = Strings::ToInt(value) * 100;
		CalcBonuses();
	} else if (stat_lower == "atk") {
		ATK = Strings::ToInt(value);
	} else if (stat_lower == "accuracy") {
		accuracy_rating = Strings::ToInt(value);
	} else if (stat_lower == "avoidance") {
		avoidance_rating = Strings::ToInt(value);
	} else if (stat_lower == "trackable") {
		trackable = Strings::ToInt(value);
	} else if (stat_lower == "min_hit") {
		min_dmg     = Strings::ToInt(value);
		// Clamp max_dmg to be >= min_dmg
		max_dmg     = std::max(min_dmg, max_dmg);
		base_damage = round((max_dmg - min_dmg) / 1.9);
		min_damage  = min_dmg - round(base_damage / 10.0);
	} else if (stat_lower == "max_hit") {
		max_dmg     = Strings::ToInt(value);
		// Clamp min_dmg to be <= max_dmg
		min_dmg     = std::min(min_dmg, max_dmg);
		base_damage = round((max_dmg - min_dmg) / 1.9);
		min_damage  = min_dmg - round(base_damage / 10.0);
	} else if (stat_lower == "attack_count") {
		attack_count = Strings::ToInt(value);
	} else if (stat_lower == "see_invis") {
		see_invis = Strings::ToInt(value);
	} else if (stat_lower == "see_invis_undead") {
		see_invis_undead = Strings::ToInt(value);
	} else if (stat_lower == "see_hide") {
		see_hide = Strings::ToInt(value);
	} else if (stat_lower == "see_improved_hide") {
		see_improved_hide = Strings::ToInt(value);
	} else if (stat_lower == "hp_regen") {
		hp_regen = Strings::ToBigInt(value);
	} else if (stat_lower == "hp_regen_per_second") {
		hp_regen_per_second = Strings::ToBigInt(value);
	} else if (stat_lower == "mana_regen") {
		mana_regen = Strings::ToBigInt(value);
	} else if (stat_lower == "level") {
		SetLevel(Strings::ToInt(value));
	} else if (stat_lower == "aggro") {
		pAggroRange = Strings::ToFloat(value);
	} else if (stat_lower == "assist") {
		pAssistRange = Strings::ToFloat(value);
	} else if (stat_lower == "slow_mitigation") {
		slow_mitigation = Strings::ToInt(value);
	} else if (stat_lower == "loottable_id") {
		m_loottable_id = Strings::ToFloat(value);
	} else if (stat_lower == "healscale") {
		healscale = Strings::ToFloat(value);
	} else if (stat_lower == "spellscale") {
		spellscale = Strings::ToFloat(value);
	} else if (stat_lower == "npc_spells_id") {
		AI_AddNPCSpells(Strings::ToInt(value));
	} else if (stat_lower == "npc_spells_effects_id") {
		AI_AddNPCSpellsEffects(Strings::ToInt(value));
		CalcBonuses();
	} else if (stat_lower == "heroic_strikethrough") {
		heroic_strikethrough = Strings::ToInt(value);
	} else if (stat_lower == "keeps_sold_items") {
		SetKeepsSoldItems(Strings::ToBool(value));
	} else if (stat_lower == "charm_ac") {
		charm_ac = Strings::ToInt(value);
	} else if (stat_lower == "charm_min_dmg") {
		charm_min_dmg = Strings::ToInt(value);
	} else if (stat_lower == "charm_max_dmg") {
		charm_max_dmg = Strings::ToInt(value);
	} else if (stat_lower == "charm_attack_delay") {
		charm_attack_delay = Strings::ToInt(value);
	} else if (stat_lower == "charm_accuracy_rating") {
		charm_accuracy_rating = Strings::ToInt(value);
	} else if (stat_lower == "charm_avoidance_rating") {
		charm_avoidance_rating = Strings::ToInt(value);
	} else if (stat_lower == "charm_atk") {
		charm_atk = Strings::ToInt(value);
	} else if (stat_lower == "default_ac") {
		default_ac = Strings::ToInt(value);
	} else if (stat_lower == "default_min_dmg") {
		default_min_dmg = Strings::ToInt(value);
	} else if (stat_lower == "default_max_dmg") {
		default_max_dmg = Strings::ToInt(value);
	} else if (stat_lower == "default_attack_delay") {
		default_attack_delay = Strings::ToInt(value);
	} else if (stat_lower == "default_accuracy_rating") {
		default_accuracy_rating = Strings::ToInt(value);
	} else if (stat_lower == "default_avoidance_rating") {
		default_avoidance_rating = Strings::ToInt(value);
	} else if (stat_lower == "default_atk") {
		default_atk = Strings::ToInt(value);
	}
}

float NPC::GetNPCStat(const std::string& stat)
{
	const std::string& stat_lower = Strings::ToLower(stat);

	if (stat_lower == "ac") {
		return AC;
	} else if (stat_lower == "str") {
		return STR;
	} else if (stat_lower == "sta") {
		return STA;
	} else if (stat_lower == "agi") {
		return AGI;
	} else if (stat_lower == "dex") {
		return DEX;
	} else if (stat_lower == "wis") {
		return WIS;
	} else if (stat_lower == "int" || stat_lower == "_int") {
		return INT;
	} else if (stat_lower == "cha") {
		return CHA;
	} else if (stat_lower == "max_hp") {
		return base_hp;
	} else if (stat_lower == "max_mana") {
		return npc_mana;
	} else if (stat_lower == "mr") {
		return MR;
	} else if (stat_lower == "fr") {
		return FR;
	} else if (stat_lower == "cr") {
		return CR;
	} else if (stat_lower == "cor") {
		return Corrup;
	} else if (stat_lower == "phr") {
		return PhR;
	} else if (stat_lower == "pr") {
		return PR;
	} else if (stat_lower == "dr") {
		return DR;
	} else if (stat_lower == "runspeed") {
		return runspeed;
	} else if (stat_lower == "attack_speed") {
		return attack_speed;
	} else if (stat_lower == "attack_delay") {
		return attack_delay;
	} else if (stat_lower == "atk") {
		return ATK;
	} else if (stat_lower == "accuracy") {
		return accuracy_rating;
	} else if (stat_lower == "avoidance") {
		return avoidance_rating;
	} else if (stat_lower == "trackable") {
		return trackable;
	} else if (stat_lower == "min_hit") {
		return min_dmg;
	} else if (stat_lower == "max_hit") {
		return max_dmg;
	} else if (stat_lower == "attack_count") {
		return attack_count;
	} else if (stat_lower == "see_invis") {
		return see_invis;
	} else if (stat_lower == "see_invis_undead") {
		return see_invis_undead;
	} else if (stat_lower == "see_hide") {
		return see_hide;
	} else if (stat_lower == "see_improved_hide") {
		return see_improved_hide;
	} else if (stat_lower == "hp_regen") {
		return hp_regen;
	} else if (stat_lower == "hp_regen_per_second") {
		return hp_regen_per_second;
	} else if (stat_lower == "mana_regen") {
		return mana_regen;
	} else if (stat_lower == "level") {
		return GetOrigLevel();
	} else if (stat_lower == "aggro") {
		return pAggroRange;
	} else if (stat_lower == "assist") {
		return pAssistRange;
	} else if (stat_lower == "slow_mitigation") {
		return slow_mitigation;
	} else if (stat_lower == "loottable_id") {
		return m_loottable_id;
	} else if (stat_lower == "healscale") {
		return healscale;
	} else if (stat_lower == "spellscale") {
		return spellscale;
	} else if (stat_lower == "npc_spells_id") {
		return npc_spells_id;
	} else if (stat_lower == "npc_spells_effects_id") {
		return npc_spells_effects_id;
	} else if (stat_lower == "heroic_strikethrough") {
		return heroic_strikethrough;
	} else if (stat_lower == "keeps_sold_items") {
		return keeps_sold_items;
	} else if (stat_lower == "default_ac") {
		return default_ac;
	} else if (stat_lower == "default_min_hit") {
		return default_min_dmg;
	} else if (stat_lower == "default_max_hit") {
		return default_max_dmg;
	} else if (stat_lower == "default_attack_delay") {
		return default_attack_delay;
	} else if (stat_lower == "default_accuracy") {
		return default_accuracy_rating;
	} else if (stat_lower == "default_avoidance") {
		return default_avoidance_rating;
	} else if (stat_lower == "default_atk") {
		return default_atk;
	} else if (stat_lower == "charm_ac") {
		return charm_ac;
	} else if (stat_lower == "charm_min_hit") {
		return charm_min_dmg;
	} else if (stat_lower == "charm_max_hit") {
		return charm_max_dmg;
	} else if (stat_lower == "charm_attack_delay") {
		return charm_attack_delay;
	} else if (stat_lower == "charm_accuracy") {
		return charm_accuracy_rating;
	} else if (stat_lower == "charm_avoidance") {
		return charm_avoidance_rating;
	} else if (stat_lower == "charm_atk") {
		return charm_atk;
	}

	return 0.0f;
}

void NPC::LevelScale() {

	uint8 random_level = (zone->random.Int(level, maxlevel));

	float scaling = (((random_level / (float)level) - 1) * (scalerate / 100.0f));

	if (RuleB(NPC, NewLevelScaling)) {
		if (scalerate == 0 || maxlevel <= 25) {
			// Don't add HP to dynamically scaled NPCs since this will be calculated later
			if (max_hp > 0 || m_skip_auto_scale)
			{
				// pre-pop seems to scale by 20 HP increments while newer by 100
				// We also don't want 100 increments on newer noobie zones, check level
				if (zone->GetZoneID() < 200 || level < 48) {
					max_hp += (random_level - level) * 20;
					base_hp += (random_level - level) * 20;
				} else {
					max_hp += (random_level - level) * 100;
					base_hp += (random_level - level) * 100;
				}

				current_hp = max_hp;
			}

			// Don't add max_dmg to dynamically scaled NPCs since this will be calculated later
			if (max_dmg > 0 || m_skip_auto_scale)
			{
				max_dmg += (random_level - level) * 2;
			}
		} else {
			uint8 scale_adjust = 1;

			base_hp += (int64)(base_hp * scaling);
			max_hp += (int64)(max_hp * scaling);
			current_hp = max_hp;

			if (max_dmg) {
				max_dmg += (int64)(max_dmg * scaling / scale_adjust);
				min_dmg += (int64)(min_dmg * scaling / scale_adjust);
			}

			STR += (int)(STR * scaling / scale_adjust);
			STA += (int)(STA * scaling / scale_adjust);
			AGI += (int)(AGI * scaling / scale_adjust);
			DEX += (int)(DEX * scaling / scale_adjust);
			INT += (int)(INT * scaling / scale_adjust);
			WIS += (int)(WIS * scaling / scale_adjust);
			CHA += (int)(CHA * scaling / scale_adjust);
			if (MR)
				MR += (int)(MR * scaling / scale_adjust);
			if (CR)
				CR += (int)(CR * scaling / scale_adjust);
			if (DR)
				DR += (int)(DR * scaling / scale_adjust);
			if (FR)
				FR += (int)(FR * scaling / scale_adjust);
			if (PR)
				PR += (int)(PR * scaling / scale_adjust);
		}
	} else {
		// Compensate for scale rates at low levels so they don't add too much
		uint8 scale_adjust = 1;
		if(level > 0 && level <= 5)
			scale_adjust = 10;
		if(level > 5 && level <= 10)
			scale_adjust = 5;
		if(level > 10 && level <= 15)
			scale_adjust = 3;
		if(level > 15 && level <= 25)
			scale_adjust = 2;

		AC += (int)(AC * scaling);
		ATK += (int)(ATK * scaling);
		base_hp += (int64)(base_hp * scaling);
		max_hp += (int64)(max_hp * scaling);
		current_hp = max_hp;
		STR += (int)(STR * scaling / scale_adjust);
		STA += (int)(STA * scaling / scale_adjust);
		AGI += (int)(AGI * scaling / scale_adjust);
		DEX += (int)(DEX * scaling / scale_adjust);
		INT += (int)(INT * scaling / scale_adjust);
		WIS += (int)(WIS * scaling / scale_adjust);
		CHA += (int)(CHA * scaling / scale_adjust);
		if (MR)
			MR += (int)(MR * scaling / scale_adjust);
		if (CR)
			CR += (int)(CR * scaling / scale_adjust);
		if (DR)
			DR += (int)(DR * scaling / scale_adjust);
		if (FR)
			FR += (int)(FR * scaling / scale_adjust);
		if (PR)
			PR += (int)(PR * scaling / scale_adjust);

		if (max_dmg)
		{
			max_dmg += (int64)(max_dmg * scaling / scale_adjust);
			min_dmg += (int64)(min_dmg * scaling / scale_adjust);
		}

	}
	level = random_level;

}

uint32 NPC::GetSpawnPointID() const
{
	return respawn2 ? respawn2->GetID() : 0;
}

void NPC::NPCSlotTexture(uint8 slot, uint32 texture)
{
	if (slot == EQ::textures::TextureSlot::weaponPrimary) {
		d_melee_texture1 = texture;
	} else if (slot == EQ::textures::TextureSlot::weaponSecondary) {
		d_melee_texture2 = texture;
	} else {
		// Reserved for texturing individual armor slots
	}
}

uint32 NPC::GetSwarmOwner()
{
	if(GetSwarmInfo() != nullptr)
	{
		return GetSwarmInfo()->owner_id;
	}
	return 0;
}

uint32 NPC::GetSwarmTarget()
{
	if(GetSwarmInfo() != nullptr)
	{
		return GetSwarmInfo()->target;
	}
	return 0;
}

void NPC::SetSwarmTarget(int target_id)
{
	if(GetSwarmInfo() != nullptr)
	{
		GetSwarmInfo()->target = target_id;
	}
}

int64 NPC::CalcMaxMana()
{
	if (npc_mana == 0) {
		if (IsIntelligenceCasterClass()) {
			max_mana = (((GetINT() / 2) + 1) * GetLevel()) + spellbonuses.Mana + itembonuses.Mana;
		} else if (IsWisdomCasterClass()) {
			max_mana = (((GetWIS() / 2) + 1) * GetLevel()) + spellbonuses.Mana + itembonuses.Mana;
		} else {
			max_mana = 0;
		}

		if (max_mana < 0) {
			max_mana = 0;
		}

		return max_mana;
	} else {
		if (IsIntelligenceCasterClass()) {
			max_mana = npc_mana + spellbonuses.Mana + itembonuses.Mana;
		} else if (IsWisdomCasterClass()) {
			max_mana = npc_mana + spellbonuses.Mana + itembonuses.Mana;
		} else {
			max_mana = 0;
		}

		if (max_mana < 0) {
			max_mana = 0;
		}

		return max_mana;
	}
}

void NPC::SignalNPC(int _signal_id)
{
	signal_q.push_back(_signal_id);
}

void NPC::SendPayload(int payload_id, std::string payload_value)
{
	if (parse->HasQuestSub(GetNPCTypeID(), EVENT_PAYLOAD)) {
		const auto& export_string = fmt::format("{} {}", payload_id, payload_value);

		parse->EventNPC(EVENT_PAYLOAD, this, nullptr, export_string, 0);
	}
}

NPC_Emote_Struct* NPC::GetNPCEmote(uint32 emote_id, uint8 event_) {
	std::vector<NPC_Emote_Struct*> emotes;

	for (auto& e : zone->npc_emote_list) {
		if (e->emoteid == emote_id && e->event_ == event_) {
			emotes.emplace_back(e);
		}
	}

	if (emotes.empty()) {
		return nullptr;
	} else if (emotes.size() == 1) {
		return emotes.front();
	}

	const int index = zone->random.Roll0(emotes.size());

	return emotes[index];
}

void NPC::DoNPCEmote(uint8 event_, uint32 emote_id, Mob* t)
{
	if (!emote_id) {
		return;
	}

	const auto& e = GetNPCEmote(emote_id, event_);
	if (!e) {
		return;
	}

	std::string processed = e->text;

	// Mob Variables
	Strings::FindReplace(processed, "$mname", GetCleanName());
	Strings::FindReplace(processed, "$mracep", GetRacePlural());
	Strings::FindReplace(processed, "$mrace", GetRaceIDName(GetRace()));
	Strings::FindReplace(processed, "$mclass", GetClassIDName(GetClass()));
	Strings::FindReplace(processed, "$mclassp", GetClassPlural());

	// Target Variables
	Strings::FindReplace(processed, "$name", t ? t->GetCleanName() : "foe");
	Strings::FindReplace(processed, "$class", t ? GetClassIDName(t->GetClass()) : "class");
	Strings::FindReplace(processed, "$classp", t ? t->GetClassPlural() : "classes");
	Strings::FindReplace(processed, "$race", t ? GetRaceIDName(t->GetRace()) : "race");
	Strings::FindReplace(processed, "$racep", t ? t->GetRacePlural() : "races");

	if (emoteid == e->emoteid) {
		if (event_ == EQ::constants::EmoteEventTypes::Hailed && t) {
			DoQuestPause(t);
		}

		if (e->type == EQ::constants::EmoteTypes::Say) {
			Say(processed.c_str());
		} else if (e->type == EQ::constants::EmoteTypes::Emote) {
			Emote(processed.c_str());
		} else if (e->type == EQ::constants::EmoteTypes::Shout) {
			Shout(processed.c_str());
		} else if (e->type == EQ::constants::EmoteTypes::Proximity) {
			entity_list.MessageCloseString(
				this,
				true,
				200,
				Chat::NPCQuestSay,
				GENERIC_STRING,
				processed.c_str()
			);
		}
	}
}

bool NPC::CanTalk()
{
	switch (GetRace()) {
		case Race::Human:
		case Race::Barbarian:
		case Race::Erudite:
		case Race::WoodElf:
		case Race::HighElf:
		case Race::DarkElf:
		case Race::HalfElf:
		case Race::Dwarf:
		case Race::Troll:
		case Race::Ogre:
		case Race::Halfling:
		case Race::Gnome:
		case Race::Werewolf:
		case Race::Brownie:
		case Race::Centaur:
		case Race::Giant:
		case Race::Trakanon:
		case Race::VenrilSathir:
		case Race::Kerran:
		case Race::Fairy:
		case Race::Ghost:
		case Race::Gnoll:
		case Race::Goblin:
		case Race::FreeportGuard:
		case Race::LavaDragon:
		case Race::LizardMan:
		case Race::Minotaur:
		case Race::Orc:
		case Race::HumanBeggar:
		case Race::Pixie:
		case Race::Drachnid:
		case Race::SolusekRo:
		case Race::Tunare:
		case Race::Treant:
		case Race::Vampire:
		case Race::StatueOfRallosZek:
		case Race::HighpassCitizen:
		case Race::Zombie:
		case Race::QeynosCitizen:
		case Race::NeriakCitizen:
		case Race::EruditeCitizen:
		case Race::Bixie:
		case Race::RivervaleCitizen:
		case Race::Scarecrow:
		case Race::Sphinx:
		case Race::HalasCitizen:
		case Race::GrobbCitizen:
		case Race::OggokCitizen:
		case Race::KaladimCitizen:
		case Race::CazicThule:
		case Race::ElfVampire:
		case Race::Denizen:
		case Race::Efreeti:
		case Race::PhinigelAutropos:
		case Race::Mermaid:
		case Race::Harpy:
		case Race::Fayguard:
		case Race::Innoruuk:
		case Race::Djinn:
		case Race::InvisibleMan:
		case Race::Iksar:
		case Race::VahShir:
		case Race::Sarnak:
		case Race::Xalgoz:
		case Race::Yeti:
		case Race::IksarCitizen:
		case Race::ForestGiant:
		case Race::Burynai:
		case Race::Erollisi:
		case Race::Tribunal:
		case Race::Bertoxxulous:
		case Race::Bristlebane:
		case Race::Ratman:
		case Race::Coldain:
		case Race::VeliousDragon:
		case Race::Siren:
		case Race::FrostGiant:
		case Race::StormGiant:
		case Race::BlackAndWhiteDragon:
		case Race::GhostDragon:
		case Race::PrismaticDragon:
		case Race::Grimling:
		case Race::KhatiSha:
		case Race::Vampire2:
		case Race::Shissar:
		case Race::VampireVolatalis:
		case Race::Shadel:
		case Race::Netherbian:
		case Race::Akhevan:
		case Race::Wretch:
		case Race::LordInquisitorSeru:
		case Race::VahShirKing:
		case Race::VahShirGuard:
		case Race::TeleportMan:
		case Race::Werewolf2:
		case Race::Nymph:
		case Race::Dryad:
		case Race::Treant2:
		case Race::TarewMarr:
		case Race::SolusekRo2:
		case Race::GuardOfJustice:
		case Race::SolusekRoGuard:
		case Race::BertoxxulousNew:
		case Race::TribunalNew:
		case Race::TerrisThule:
		case Race::KnightOfPestilence:
		case Race::Lepertoloth:
		case Race::Pusling:
		case Race::WaterMephit:
		case Race::NightmareGoblin:
		case Race::Karana:
		case Race::Saryrn:
		case Race::FenninRo:
		case Race::SoulDevourer:
		case Race::NewRallosZek:
		case Race::VallonZek:
		case Race::TallonZek:
		case Race::AirMephit:
		case Race::EarthMephit:
		case Race::FireMephit:
		case Race::NightmareMephit:
		case Race::Zebuxoruk:
		case Race::MithanielMarr:
		case Race::UndeadKnight:
		case Race::Rathe:
		case Race::Xegony:
		case Race::Fiend:
		case Race::Quarm:
		case Race::Efreeti2:
		case Race::Valorian2:
		case Race::AnimatedArmor:
		case Race::UndeadFootman:
		case Race::RallosOgre:
		case Race::Froglok2:
		case Race::TrollCrewMember:
		case Race::PirateDeckhand:
		case Race::BrokenSkullPirate:
		case Race::PirateGhost:
		case Race::OneArmedPirate:
		case Race::SpiritmasterNadox:
		case Race::BrokenSkullTaskmaster:
		case Race::GnomePirate:
		case Race::DarkElfPirate:
		case Race::OgrePirate:
		case Race::HumanPirate:
		case Race::EruditePirate:
		case Race::UndeadVampire:
		case Race::Vampire3:
		case Race::RujarkianOrc:
		case Race::BoneGolem:
		case Race::SandElf:
		case Race::MasterVampire:
		case Race::MasterOrc:
		case Race::Mummy:
		case Race::NewGoblin:
		case Race::Nihil:
		case Race::Trusik:
		case Race::Ukun:
		case Race::Ixt:
		case Race::Ikaav:
		case Race::Aneuk:
		case Race::Kyv:
		case Race::Noc:
		case Race::Ratuk:
		case Race::Huvul:
		case Race::Mastruq:
		case Race::MataMuram:
		case Race::Succubus:
		case Race::Pyrilen:
		case Race::Dragorn:
		case Race::Gelidran:
		case Race::Minotaur2:
		case Race::CrystalShard:
		case Race::Goblin2:
		case Race::Giant2:
		case Race::Orc2:
		case Race::Werewolf3:
		case Race::Shiliskin:
		case Race::Minotaur3:
		case Race::Fairy2:
		case Race::Bolvirk:
		case Race::Elddar:
		case Race::ForestGiant2:
		case Race::BoneGolem2:
		case Race::Scrykin:
		case Race::Treant3:
		case Race::Vampire4:
		case Race::AyonaeRo:
		case Race::SullonZek:
		case Race::Bixie2:
		case Race::Centaur2:
		case Race::Drakkin:
		case Race::Giant3:
		case Race::Gnoll2:
		case Race::GiantShade:
		case Race::Harpy2:
		case Race::Satyr:
		case Race::Dynleth:
		case Race::Kedge:
		case Race::Kerran2:
		case Race::Shissar2:
		case Race::Siren2:
		case Race::Sphinx2:
		case Race::Human2:
		case Race::Brownie2:
		case Race::Exoskeleton:
		case Race::Minotaur4:
		case Race::Scarecrow2:
		case Race::Wereorc:
		case Race::ElvenGhost:
		case Race::HumanGhost:
		case Race::Burynai2:
		case Race::Dracolich:
		case Race::IksarGhost:
		case Race::Mephit:
		case Race::Sarnak2:
		case Race::Gnoll3:
		case Race::GodOfDiscord:
		case Race::Ogre2:
		case Race::Giant4:
		case Race::Apexus:
		case Race::Bellikos:
		case Race::BrellsFirstCreation:
		case Race::Brell:
		case Race::Coldain2:
		case Race::Coldain3:
		case Race::Telmira:
		case Race::MorellThule:
		case Race::Amygdalan:
		case Race::Sandman:
		case Race::RoyalGuard:
		case Race::CazicThule2:
		case Race::Erudite2:
		case Race::Alaran:
		case Race::AlaranGhost:
		case Race::Ratman2:
		case Race::Akheva:
		case Race::Luclin:
		case Race::Luclin2:
		case Race::Luclin3:
		case Race::Luclin4:
			return true;
		default:
			return false;
	}
}

//this is called with 'this' as the mob being looked at, and
//iOther the mob who is doing the looking. It should figure out
//what iOther thinks about 'this'
FACTION_VALUE NPC::GetReverseFactionCon(Mob* iOther) {
	iOther = iOther->GetOwnerOrSelf();
	int primaryFaction= iOther->GetPrimaryFaction();

	//I am pretty sure that this special faction call is backwards
	//and should be iOther->GetSpecialFactionCon(this)
	if (primaryFaction < 0)
		return GetSpecialFactionCon(iOther);

	if (primaryFaction == 0)
		return FACTION_INDIFFERENTLY;

	//if we are a pet, use our owner's faction stuff
	Mob *own = GetOwner();
	if (own != nullptr)
		return own->GetReverseFactionCon(iOther);

	//make sure iOther is an npc
	//also, if we dont have a faction, then they arnt gunna think anything of us either
	if(!iOther->IsNPC() || GetPrimaryFaction() == 0)
		return(FACTION_INDIFFERENTLY);

	//if we get here, iOther is an NPC too

	//otherwise, employ the npc faction stuff
	//so we need to look at iOther's faction table to see
	//what iOther thinks about our primary faction
	return(iOther->CastToNPC()->CheckNPCFactionAlly(GetPrimaryFaction()));
}

//Look through our faction list and return a faction con based
//on the npc_value for the other person's primary faction in our list.
FACTION_VALUE NPC::CheckNPCFactionAlly(int32 other_faction)
{
	for (const auto& e : faction_list) {
		if (e.faction_id == other_faction) {
			if (e.npc_value > 0) {
				return FACTION_ALLY;
			} else if (e.npc_value < 0) {
				return FACTION_SCOWLS;
			} else {
				return FACTION_INDIFFERENTLY;
			}
		}
	}

	// I believe that the assumption is, barring no entry in npc_faction_entries
	// that two npcs on like faction con ally to each other.  This catches cases
	// where an npc is on a faction but has no hits (hence no entry in
	// npc_faction_entries).

	if (GetPrimaryFaction() == other_faction) {
		return FACTION_ALLY;
	} else {
		return FACTION_INDIFFERENTLY;
	}
}

bool NPC::IsFactionListAlly(uint32 other_faction)
{
	return CheckNPCFactionAlly(other_faction) == FACTION_ALLY;
}

int NPC::GetScore()
{
    int lv = std::min(70, (int)GetLevel());
    int basedmg = (lv*2)*(1+(lv / 100)) - (lv / 2);
    int minx = 0;
    int64 basehp = 0;
    int hpcontrib = 0;
    int dmgcontrib = 0;
    int spccontrib = 0;
    int64 hp = GetMaxHP();
    int mindmg = min_dmg;
    int maxdmg = max_dmg;
    int final;

    if(lv < 46)
    {
		minx = static_cast<int> (ceil( ((lv - (lv / 10.0)) - 1.0) ));
		basehp = (lv * 10) + (lv * lv);
	}
	else
	{
		minx = static_cast<int> (ceil( ((lv - (lv / 10.0)) - 1.0) - (( lv - 45.0 ) / 2.0) ));
        basehp = (lv * 10) + ((lv * lv) * 4);
    }

    if(hp > basehp)
    {
        hpcontrib = static_cast<int> (((hp / static_cast<float> (basehp)) * 1.5));
        if(hpcontrib > 5) { hpcontrib = 5; }

        if(maxdmg > basedmg)
        {
            dmgcontrib = static_cast<int> (ceil( ((maxdmg / basedmg) * 1.5) ));
        }

        if(HasNPCSpecialAtk("E")) { spccontrib++; }    //Enrage
        if(HasNPCSpecialAtk("F")) { spccontrib++; }    //Flurry
        if(HasNPCSpecialAtk("R")) { spccontrib++; }    //Rampage
        if(HasNPCSpecialAtk("r")) { spccontrib++; }    //Area Rampage
        if(HasNPCSpecialAtk("S")) { spccontrib++; }    //Summon
        if(HasNPCSpecialAtk("T")) { spccontrib += 2; } //Triple
        if(HasNPCSpecialAtk("Q")) { spccontrib += 3; } //Quad
        if(HasNPCSpecialAtk("U")) { spccontrib += 5; } //SpecialAbility::SlowImmunity
        if(HasNPCSpecialAtk("L")) { spccontrib++; }    //Innate Dual Wield
    }

    if(npc_spells_id > 12)
	{
        if(lv < 16)
            spccontrib++;
        else
            spccontrib += static_cast<int> (floor(lv/15.0));
    }

    final = minx + hpcontrib + dmgcontrib + spccontrib;
    final = std::max(1, final);
    final = std::min(100, final);
    return(final);
}

uint32 NPC::GetSpawnKillCount()
{
	uint32 sid = GetSpawnPointID();

	if(sid > 0)
	{
		return(zone->GetSpawnKillCount(sid));
	}

	return(0);
}

void NPC::DoQuestPause(Mob* m)
{
	if (!m) {
		return;
	}

	if (IsMoving() && !IsOnHatelist(m)) {
		PauseWandering(RuleI(NPC, SayPauseTimeInSec));

		if (FacesTarget() && !m->sneaking) {
			FaceTarget(m);
		}
	} else if (!IsMoving()) {
		if (
			FacesTarget() &&
			!m->sneaking &&
			GetAppearance() != eaSitting &&
			GetAppearance() != eaDead
		) {
			FaceTarget(m);
		}
	}
}

void NPC::ChangeLastName(std::string last_name)
{
	auto outapp = new EQApplicationPacket(OP_GMLastName, sizeof(GMLastName_Struct));
	auto gmn = (GMLastName_Struct*) outapp->pBuffer;

	strn0cpy(gmn->name, GetName(), sizeof(gmn->name));
	strn0cpy(gmn->gmname, GetName(), sizeof(gmn->gmname));
	strn0cpy(gmn->lastname, last_name.c_str(), sizeof(gmn->lastname));

	gmn->unknown[0] = 1;
	gmn->unknown[1] = 1;
	gmn->unknown[2] = 1;
	gmn->unknown[3] = 1;

	entity_list.QueueClients(this, outapp, false);

	safe_delete(outapp);
}

void NPC::ClearLastName()
{
	std::string empty;
	ChangeLastName(empty);
}

void NPC::DepopSwarmPets()
{
	if (GetSwarmInfo()) {
		if (GetSwarmInfo()->duration->Check(false)){
			Mob* owner = entity_list.GetMobID(GetSwarmInfo()->owner_id);
			if (owner) {
				owner->SetTempPetCount(owner->GetTempPetCount() - 1);
			}
			Depop();
			return;
		}
	}
}

void NPC::ModifyStatsOnCharm(bool remove_charm, Mob* charmer)
{
	if (!remove_charm && parse->HasQuestSub(GetNPCTypeID(), EVENT_CHARM_START)) {
		parse->EventNPC(EVENT_CHARM_START, this, charmer, "", 0);
	} else if (remove_charm && parse->HasQuestSub(GetNPCTypeID(), EVENT_CHARM_END)) {
		parse->EventNPC(EVENT_CHARM_END, this, charmer, "", 0);
	}

	const int new_ac               = remove_charm ? default_ac : charm_ac;
	const int new_attack_delay     = remove_charm ? default_attack_delay : charm_attack_delay;
	const int new_accuracy_rating  = remove_charm ? default_accuracy_rating : charm_accuracy_rating;
	const int new_avoidance_rating = remove_charm ? default_avoidance_rating : charm_avoidance_rating;
	const int new_atk              = remove_charm ? default_atk : charm_atk;
	const int new_min_dmg          = remove_charm ? default_min_dmg : charm_min_dmg;
	const int new_max_dmg          = remove_charm ? default_max_dmg : charm_max_dmg;

	if (new_ac) {
		AC = new_ac;
	}

	if (new_attack_delay) {
		attack_delay = new_attack_delay;
	}

	if (new_accuracy_rating) {
		accuracy_rating = new_accuracy_rating;
	}

	if (new_avoidance_rating) {
		avoidance_rating = new_avoidance_rating;
	}

	if (new_atk) {
		ATK = new_atk;
	}

	if (new_min_dmg || new_max_dmg) {
		base_damage = std::round((new_max_dmg - new_min_dmg) / 1.9);
		min_damage  = new_min_dmg - std::round(base_damage / 10.0);
	}

	if (RuleB(Spells, CharmDisablesSpecialAbilities)) {
		if (remove_charm) {
			ProcessSpecialAbilities(default_special_abilities);
		} else {
			ClearSpecialAbilities();
		}
	}

	SetAttackTimer();
	CalcAC();
}

uint16 NPC::GetMeleeTexture1() const
{
	return d_melee_texture1;
}

uint16 NPC::GetMeleeTexture2() const
{
	return d_melee_texture2;
}

float NPC::GetProximityMinX()
{
	return proximity->min_x;
}

float NPC::GetProximityMaxX()
{
	return proximity->max_x;
}

float NPC::GetProximityMinY()
{
	return proximity->min_y;
}

float NPC::GetProximityMaxY()
{
	return proximity->max_y;
}

float NPC::GetProximityMinZ()
{
	return proximity->min_z;
}

float NPC::GetProximityMaxZ()
{
	return proximity->max_z;
}

bool NPC::IsProximitySet()
{
	if (proximity && proximity->proximity_set) {
		return proximity->proximity_set;
	}

	return false;
}

/**
 * @param box_size
 * @param move_distance
 * @param move_delay
 */
void NPC::SetSimpleRoamBox(float box_size, float move_distance, int move_delay)
{
	AI_SetRoambox(
		(move_distance != 0 ? move_distance : box_size / 2),
		GetX() + box_size,
		GetX() - box_size,
		GetY() + box_size,
		GetY() - box_size,
		move_delay
	);
}

/**
 * @param caster
 * @param chance
 * @param cast_range
 * @param spell_types
 * @return
 */
bool NPC::AICheckCloseBeneficialSpells(
	NPC *caster,
	uint8 chance,
	float cast_range,
	uint32 spell_types
)
{
	if((spell_types & SPELL_TYPES_DETRIMENTAL) != 0) {
		LogError("Detrimental spells requested from AICheckCloseBeneficialSpells!");
		return false;
	}

	if (!caster) {
		return false;
	}

	if (!caster->AI_HasSpells()) {
		return false;
	}

	if (caster->GetSpecialAbility(SpecialAbility::NoBuffHealFriends)) {
		return false;
	}

	if (chance < 100) {
		uint8 tmp = zone->random.Int(0, 99);
		if (tmp >= chance) {
			return false;
		}
	}

	/**
	 * Indifferent
	 */
	if (caster->GetPrimaryFaction() == 0) {
		return false;
	}

	/**
	 * Check through close range mobs
	 */
	for (auto & close_mob : caster->GetCloseMobList(cast_range)) {
		Mob *mob = close_mob.second;
		if (!mob) {
			continue;
		}

		if (mob->IsClient()) {
			continue;
		}

		float distance = Distance(mob->GetPosition(), caster->GetPosition());
		if (distance > cast_range) {
			continue;
		}

		if (!mob->CheckLosFN(caster)) {
			continue;
		}

		if (mob->GetReverseFactionCon(caster) >= FACTION_KINDLY) {
			continue;
		}

		LogAICastBeneficialClose(
			"NPC [{}] Distance [{}] Cast Range [{}] Caster [{}]",
			mob->GetCleanName(),
			distance,
			cast_range,
			caster->GetCleanName()
		);

		if ((spell_types & SpellType_Buff) && !RuleB(NPC, BuffFriends)) {
			if (mob != caster) {
				spell_types = SpellType_Heal;
			}
		}

		if (caster->AICastSpell(mob, 100, spell_types)) {
			return true;
		}
	}

	return false;
}

/**
 * @param sender
 * @param attacker
 */
void NPC::AIYellForHelp(Mob *sender, Mob *attacker)
{
	LogAIYellForHelp("Mob[{}] Target[{}]",
		(sender == nullptr ? "NULL MOB" : GetCleanName()),
		(attacker == nullptr ? "NULL TARGET" : attacker->GetCleanName())
		);

	if (!sender || !attacker) {
		return;
	}

	/**
	 * If we dont have a faction set, we're gonna be indiff to everybody
	 */
	if (sender->GetPrimaryFaction() == 0) {
		LogAIYellForHelp("No Primary Faction");
		return;
	}

	if (sender->HasAssistAggro()) {
		LogAIYellForHelp("I have assist aggro");
		return;
	}

	LogAIYellForHelp(
		"NPC [{}] ID [{}] is starting to scan",
		GetCleanName(),
		GetID()
	);

	for (auto &close_mob: sender->GetCloseMobList()) {
		Mob *mob = close_mob.second;
		if (!mob) {
			continue;
		}

		float distance = DistanceSquared(m_Position, mob->GetPosition());

		if (mob->IsClient()) {
			continue;
		}

		float assist_range = (mob->GetAssistRange() * mob->GetAssistRange());

		// Implement optional sneak-pull
		if (RuleB(Combat, EnableSneakPull) && attacker->sneaking) {
			assist_range = RuleI(Combat, SneakPullAssistRange);
			if (attacker->IsClient()) {
				float clientx = attacker->GetX();
				float clienty = attacker->GetY();
				if (attacker->CastToClient()->BehindMob(mob, clientx, clienty)) {
					assist_range = 0;
				}
			}
		}

		if (distance > assist_range) {
			continue;
		}

		LogAIYellForHelpDetail(
			"NPC [{}] ID [{}] is scanning - checking against NPC [{}] range [{}] dist [{}] in_range [{}]",
			GetCleanName(),
			GetID(),
			mob->GetCleanName(),
			assist_range,
			distance,
			(distance < assist_range)
		);

		if (mob->CheckAggro(attacker)) {
			continue;
		}

		if (sender->NPCAssistCap() >= RuleI(Combat, NPCAssistCap)) {
			break;
		}

		if (
			mob != sender
			&& mob != attacker
			&& mob->GetPrimaryFaction() != 0
			&& !mob->IsEngaged()
			&& ((!mob->IsPet()) || (mob->IsPet() && mob->GetOwner() && !mob->GetOwner()->IsOfClientBot()))
			) {

			/**
			 * if they are in range, make sure we are not green...
			 * then jump in if they are our friend
			 */
			if (mob->GetLevel() >= 50 || mob->AlwaysAggro() || attacker->GetLevelCon(mob->GetLevel()) != ConsiderColor::Gray) {
				if (mob->GetPrimaryFaction() == sender->CastToNPC()->GetPrimaryFaction()) {
					const auto f = zone->GetNPCFaction(mob->CastToNPC()->GetNPCFactionID());
					if (f) {
						if (f->ignore_primary_assist) {
							continue; //Same faction and ignore primary assist
						}
					}
				}

				if (sender->GetReverseFactionCon(mob) <= FACTION_AMIABLY) {
					//attacking someone on same faction, or a friend
					//Father Nitwit: make sure we can see them.
					if (mob->CheckLosFN(sender)) {
						mob->AddToHateList(attacker, 25, 0, false);
						sender->AddAssistCap();

						LogAIYellForHelpDetail(
							"NPC [{}] is assisting [{}] against target [{}]",
							mob->GetCleanName(),
							GetCleanName(),
							attacker->GetCleanName()
						);
					}
				}
			}
		}
	}

}

void NPC::RecalculateSkills()
{
  	int r;
	for (r = 0; r <= EQ::skills::HIGHEST_SKILL; r++) {
		skills[r] = SkillCaps::Instance()->GetSkillCap(GetClass(), (EQ::skills::SkillType)r, level).cap;
	}

	// some overrides -- really we need to be able to set skills for mobs in the DB
	// There are some known low level SHM/BST pets that do not follow this, which supports
	// the theory of needing to be able to set skills for each mob separately
	if (!IsBot()) {
		if (level > 50) {
			skills[EQ::skills::SkillDoubleAttack] = 250;
			skills[EQ::skills::SkillDualWield] = 250;
		}
		else if (level > 3) {
			skills[EQ::skills::SkillDoubleAttack] = level * 5;
			skills[EQ::skills::SkillDualWield] = skills[EQ::skills::SkillDoubleAttack];
		}
		else {
			skills[EQ::skills::SkillDoubleAttack] = level * 5;
		}
	}
}

void NPC::ReloadSpells() {
	AI_AddNPCSpells(GetNPCSpellsID());
	AI_AddNPCSpellsEffects(GetNPCSpellsEffectsID());
}

void NPC::ScaleNPC(uint8 npc_level, bool always_scale, bool override_special_abilities) {
	if (GetLevel() != npc_level) {
		SetLevel(npc_level);
		RecalculateSkills();
		ReloadSpells();
	}

	npc_scale_manager->ResetNPCScaling(this);
	npc_scale_manager->ScaleNPC(this, always_scale, override_special_abilities);
}

bool NPC::IsGuard()
{
	switch (GetRace()) {
	case Race::FreeportGuard:
		if (GetTexture() == 1 || GetTexture() == 2)
			return true;
		break;
	case Race::IksarCitizen:
		if (GetTexture() == 1)
			return true;
		break;
	case Race::Felguard:
	case Race::Fayguard:
	case Race::VahShirGuard:
	case Race::QeynosCitizen:
	case Race::RivervaleCitizen:
	case Race::EruditeCitizen:
	case Race::HalasCitizen:
	case Race::NeriakCitizen:
	case Race::GrobbCitizen:
	case Race::OggokCitizen:
	case Race::KaladimCitizen:
		return true;
	default:
		break;
	}
	if (GetPrimaryFaction() == DB_FACTION_GEM_CHOPPERS || GetPrimaryFaction() == DB_FACTION_HERETICS || GetPrimaryFaction() == DB_FACTION_KING_AKANON) { //these 3 factions of guards use player races instead of their own races so we must define them by faction.
		return true;
	}
	return false;
}

std::vector<int> NPC::GetLootList() {
	std::vector<int> npc_items;
	for (auto current_item  = m_loot_items.begin(); current_item != m_loot_items.end(); ++current_item) {
		LootItem * loot_item = *current_item;
		if (!loot_item) {
			LogError("NPC::GetLootList() - ItemList error, null item");
			continue;
		}

		if (std::find(npc_items.begin(), npc_items.end(), loot_item->item_id) != npc_items.end()) {
			continue;
		}

		npc_items.push_back(loot_item->item_id);
	}
	return npc_items;
}

bool NPC::IsRecordLootStats() const
{
	return m_record_loot_stats;
}

void NPC::SetRecordLootStats(bool record_loot_stats)
{
	NPC::m_record_loot_stats = record_loot_stats;
}

const std::vector<uint32> &NPC::GetRolledItems() const
{
	return m_rolled_items;
}

int NPC::GetRolledItemCount(uint32 item_id)
{
	int rolled_count = 0;
	for (auto &e: m_rolled_items) {
		if (item_id == e) {
			rolled_count++;
		}
	}

	return rolled_count;
}

void NPC::SendPositionToClients()
{
	static EQApplicationPacket p(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
	auto      *s = (PlayerPositionUpdateServer_Struct *) p.pBuffer;
	for (auto &c: entity_list.GetClientList()) {
		MakeSpawnUpdate(s);
		c.second->QueuePacket(&p, false);
	}
}

void NPC::HandleRoambox()
{
	bool has_arrived = GetCWP() == EQ::WaypointStatus::RoamBoxPauseInProgress && !IsMoving();
	if (has_arrived) {
		int roambox_move_delay = EQ::ClampLower(GetRoamboxDelay(), GetRoamboxMinDelay());
		int move_delay_max     = (roambox_move_delay > 0 ? roambox_move_delay : (int) GetRoamboxMinDelay() * 4);
		int random_timer       = RandomTimer(
			GetRoamboxMinDelay(),
			move_delay_max
		);

		LogNPCRoamBoxDetail(
			"({}) random_timer [{}] roambox_move_delay [{}] move_min [{}] move_max [{}]",
			GetCleanName(),
			random_timer,
			roambox_move_delay,
			(int) GetRoamboxMinDelay(),
			move_delay_max
		);

		time_until_can_move = Timer::GetCurrentTime() + random_timer;
		SetCurrentWP(0);
		return;
	}

	bool ready_to_set_new_destination = !IsMoving() && time_until_can_move < Timer::GetCurrentTime();
	if (ready_to_set_new_destination) {
		// make several attempts to find a valid next move in the box
		bool can_path = false;
		for (int i = 0; i < 10; i++) {
			auto move_x      = static_cast<float>(zone->random.Real(-m_roambox.distance, m_roambox.distance));
			auto move_y      = static_cast<float>(zone->random.Real(-m_roambox.distance, m_roambox.distance));
			auto requested_x = EQ::Clamp((GetX() + move_x), m_roambox.min_x, m_roambox.max_x);
			auto requested_y = EQ::Clamp((GetY() + move_y), m_roambox.min_y, m_roambox.max_y);
			auto requested_z = GetGroundZ(requested_x, requested_y);

			if (std::abs(requested_z - GetZ()) > 100) {
				LogNPCRoamBox("[{}] | Failed to find reasonable ground [{}]", GetCleanName(), i);
				continue;
			}

			std::vector<float> heights = {0, 250, -250};
			for (auto &h: heights) {
				if (CanPathTo(requested_x, requested_y, requested_z + h)) {
					LogNPCRoamBox("[{}] Found line of sight to path attempt [{}] at height [{}]", GetCleanName(), i, h);
					can_path = true;
					break;
				}
			}

			if (!can_path) {
				LogNPCRoamBox("[{}] | Failed line of sight to path attempt [{}]", GetCleanName(), i);
				continue;
			}

			m_roambox.dest_x = requested_x;
			m_roambox.dest_y = requested_y;

			/**
			 * If our roambox was configured with large distances, chances of hitting the min or max end of
			 * the clamp is high, this causes NPC's to gather on the border of a box, to reduce clustering
			 * either lower the roambox distance or the code will do a simple random between min - max when it
			 * hits the min or max of the clamp
			 */
			if (m_roambox.dest_x == m_roambox.min_x || m_roambox.dest_x == m_roambox.max_x) {
				m_roambox.dest_x = static_cast<float>(zone->random.Real(m_roambox.min_x, m_roambox.max_x));
			}

			if (m_roambox.dest_y == m_roambox.min_y || m_roambox.dest_y == m_roambox.max_y) {
				m_roambox.dest_y = static_cast<float>(zone->random.Real(m_roambox.min_y, m_roambox.max_y));
			}

			// If mob was not spawned in water, let's not randomly roam them into water
			// if the roam box was sloppily configured
			if (!GetWasSpawnedInWater()) {
				m_roambox.dest_z = GetGroundZ(m_roambox.dest_x, m_roambox.dest_y);
				if (zone->HasMap() && zone->HasWaterMap()) {
					auto position = glm::vec3(
						m_roambox.dest_x,
						m_roambox.dest_y,
						m_roambox.dest_z
					);

					// If someone brought us into water when we naturally wouldn't path there, return to spawn
					if (zone->watermap->InLiquid(position) && zone->watermap->InLiquid(m_Position)) {
						m_roambox.dest_x = m_SpawnPoint.x;
						m_roambox.dest_y = m_SpawnPoint.y;
					}

					if (zone->watermap->InLiquid(position)) {
						LogNPCRoamBoxDetail("[{}] | My destination is in water and I don't belong there!", GetCleanName());

						return;
					}
				}
			}
			else { // Mob was in water, make sure new spot is in water also
				m_roambox.dest_z = m_Position.z;
				auto position = glm::vec3(
					m_roambox.dest_x,
					m_roambox.dest_y,
					m_Position.z + 15
				);
				if (zone->HasWaterMap() && !zone->watermap->InLiquid(position)) {
					m_roambox.dest_x = m_SpawnPoint.x;
					m_roambox.dest_y = m_SpawnPoint.y;
					m_roambox.dest_z = m_SpawnPoint.z;
				}
			}

			LogNPCRoamBox(
				"[{}] | Pathing to [{}] [{}] [{}]",
				GetCleanName(),
				m_roambox.dest_x,
				m_roambox.dest_y,
				m_roambox.dest_z
			);

			LogNPCRoamBox(
				"NPC ({}) distance [{}] X (min/max) [{} / {}] Y (min/max) [{} / {}] | Dest x/y/z [{} / {} / {}]",
				GetCleanName(),
				m_roambox.distance,
				m_roambox.min_x,
				m_roambox.max_x,
				m_roambox.min_y,
				m_roambox.max_y,
				m_roambox.dest_x,
				m_roambox.dest_y,
				m_roambox.dest_z
			);

			if (can_path) {
				SetCurrentWP(EQ::WaypointStatus::RoamBoxPauseInProgress);
				NavigateTo(m_roambox.dest_x, m_roambox.dest_y, m_roambox.dest_z);
				return;
			}
		}

		// failed to find path, reset timer
		int roambox_move_delay = EQ::ClampLower(GetRoamboxDelay(), GetRoamboxMinDelay());
		int move_delay_max     = (roambox_move_delay > 0 ? roambox_move_delay : (int) GetRoamboxMinDelay() * 4);
		int random_timer       = RandomTimer(
			GetRoamboxMinDelay(),
			move_delay_max
		);
		time_until_can_move = Timer::GetCurrentTime() + random_timer;
	}

	return;
}

void NPC::SetTaunting(bool is_taunting) {
	taunting = is_taunting;

	if (IsPet() && IsPetOwnerClient()) {
		GetOwner()->CastToClient()->SetPetCommandState(PetButton::Taunt, is_taunting);
	}
}

bool NPC::CanPathTo(float x, float y, float z)
{
	PathfinderOptions opts;
	opts.smooth_path = true;
	opts.step_size   = RuleR(Pathing, NavmeshStepSize);
	opts.offset      = GetZOffset();
	opts.flags       = PathingNotDisabled ^ PathingZoneLine;

	bool partial = false;
	bool stuck   = false;
	auto route   = zone->pathing->FindPath(
		glm::vec3(GetX(), GetY(), GetZ()),
		glm::vec3(x, y, z),
		partial,
		stuck,
		opts
	);

	return !route.empty();
}

void NPC::DescribeSpecialAbilities(Client* c)
{
	if (!c) {
		return;
	}

	// These abilities are simple on/off flags
	static const std::vector<uint32> toggleable_special_abilities = {
		SpecialAbility::TripleAttack,
		SpecialAbility::QuadrupleAttack,
		SpecialAbility::DualWield,
		SpecialAbility::BaneAttack,
		SpecialAbility::MagicalAttack,
		SpecialAbility::SlowImmunity,
		SpecialAbility::MesmerizeImmunity,
		SpecialAbility::CharmImmunity,
		SpecialAbility::StunImmunity,
		SpecialAbility::SnareImmunity,
		SpecialAbility::FearImmunity,
		SpecialAbility::DispellImmunity,
		SpecialAbility::MeleeImmunity,
		SpecialAbility::MagicImmunity,
		SpecialAbility::FleeingImmunity,
		SpecialAbility::MeleeImmunityExceptBane,
		SpecialAbility::MeleeImmunityExceptMagical,
		SpecialAbility::AggroImmunity,
		SpecialAbility::BeingAggroImmunity,
		SpecialAbility::CastingFromRangeImmunity,
		SpecialAbility::FeignDeathImmunity,
		SpecialAbility::TauntImmunity,
		SpecialAbility::NoBuffHealFriends,
		SpecialAbility::PacifyImmunity,
		SpecialAbility::DestructibleObject,
		SpecialAbility::HarmFromClientImmunity,
		SpecialAbility::AlwaysFlee,
		SpecialAbility::AllowBeneficial,
		SpecialAbility::DisableMelee,
		SpecialAbility::AllowedToTank,
		SpecialAbility::IgnoreRootAggroRules,
		SpecialAbility::ProximityAggro,
		SpecialAbility::RangedAttackImmunity,
		SpecialAbility::ClientDamageImmunity,
		SpecialAbility::NPCDamageImmunity,
		SpecialAbility::ClientAggroImmunity,
		SpecialAbility::NPCAggroImmunity,
		SpecialAbility::MemoryFadeImmunity,
		SpecialAbility::OpenImmunity,
		SpecialAbility::AssassinateImmunity,
		SpecialAbility::HeadshotImmunity,
		SpecialAbility::BotAggroImmunity,
		SpecialAbility::BotDamageImmunity
	};

	// These abilities have parameters that need to be parsed out individually
	static const std::map<uint32, std::vector<std::string>> parameter_special_abilities = {
		{ SpecialAbility::Summon, { "Cooldown in Milliseconds", "Health Percentage" } },
		{
			SpecialAbility::Enrage,
			{
				"Health Percentage",
				"Duration in Milliseconds",
				"Cooldown in Milliseconds"
			}
		},
		{
			SpecialAbility::Rampage,
			{
				"Chance",
				"Targets",
				"Flat Damage Bonus",
				"Ignore Armor Percentage",
				"Ignore Flat Armor Amount",
				"Critical Chance",
				"Flat Critical Chance Bonus"
			}
		},
		{
			SpecialAbility::AreaRampage,
			{
				"Targets",
				"Normal Attack Damage Percentage",
				"Flat Damage Bonus",
				"Ignore Armor Percentage",
				"Ignore Flat Armor Amount",
				"Critical Chance",
				"Flat Critical Chance Bonus"
			}
		},
		{
			SpecialAbility::Flurry,
			{
				"Attacks",
				"Normal Attack Damage Percentage",
				"Flat Damage Bonus",
				"Ignore Armor Percentage",
				"Ignore Flat Armor Amount",
				"Critical Chance",
				"Flat Critical Chance Bonus"
			}
		},
		{
			SpecialAbility::RangedAttack,
			{
				"Attacks",
				"Maximum Range",
				"Chance",
				"Damage Percentage",
				"Minimum Range"
			}
		},
		{ SpecialAbility::TunnelVision, { "Aggro Modifier on Non-Tanks" } },
		{ SpecialAbility::Leash, { "Range" } },
		{ SpecialAbility::Tether, { "Range" } },
		{ SpecialAbility::FleePercent, { "Health Percentage", "Chance" } },
		{
			SpecialAbility::NPCChaseDistance,
			{
				"Maximum Distance",
				"Minimum Distance",
				"Ignore Line of Sight"
			}
		},
		{ SpecialAbility::CastingResistDifficulty, { "Resist Difficulty Value" } },
		{
			SpecialAbility::CounterAvoidDamage,
			{
				"Reduction Percentage for Block, Dodge, Parry, and Riposte",
				"Reduction Percentage for Riposte",
				"Reduction Percentage for Block",
				"Reduction Percentage for Parry",
				"Reduction Percentage for Dodge",
			}
		},
		{
			SpecialAbility::ModifyAvoidDamage,
			{
				"Addition Percentage for Block, Dodge, Parry, and Riposte",
				"Addition Percentage for Riposte",
				"Addition Percentage for Block",
				"Addition Percentage for Parry",
				"Addition Percentage for Dodge",
			}
		},
	};

	std::vector<std::string> messages = { };

	for (const auto& e : toggleable_special_abilities) {
		if (GetSpecialAbility(e)) {
			messages.emplace_back(
				fmt::format(
					"{} ({})",
					SpecialAbility::GetName(e),
					e
				)
			);
		}
	}

	int slot_id;

	for (const auto& e : parameter_special_abilities) {
		if (GetSpecialAbility(e.first)) {
			slot_id = 0;

			for (const auto& a : e.second) {
				messages.emplace_back(
					fmt::format(
						"{} ({}) | {}: {}",
						SpecialAbility::GetName(e.first),
						e.first,
						a,
						GetSpecialAbilityParam(e.first, slot_id)
					)
				);

				slot_id++;
			}
		}
	}

	if (messages.empty()) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} has no special abilities.",
				c->GetTargetDescription(this)
			).c_str()
		);
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} has the following special abilit{}:",
			c->GetTargetDescription(this),
			messages.size() != 1 ? "ies" : "y"
		).c_str()
	);

	std::sort(
		messages.begin(),
		messages.end(),
		[](const std::string& a, const std::string& b) {
			return a < b;
		}
	);

	for (const auto& e : messages) {
		c->Message(Chat::White, e.c_str());
	}
}

void NPC::DoNpcToNpcAggroScan()
{
	for (auto &close_mob : GetCloseMobList(GetAggroRange())) {
		Mob *mob = close_mob.second;
		if (!mob) {
			continue;
		}

		if (!mob->IsNPC()) {
			continue;
		}

		if (CheckWillAggro(mob)) {
			AddToHateList(mob);
		}
	}

	AI_scan_area_timer->Disable();
	AI_scan_area_timer->Start(
		RandomTimer(RuleI(NPC, NPCToNPCAggroTimerMin), RuleI(NPC, NPCToNPCAggroTimerMax)),
		false
	);
}

bool NPC::FacesTarget()
{
	const std::string& excluded_races_rule = RuleS(NPC, ExcludedFaceTargetRaces);

	if (excluded_races_rule.empty()) {
		return true;
	}

	const auto& v = Strings::Split(excluded_races_rule, ",");

	return std::find(v.begin(), v.end(), std::to_string(GetBaseRace())) == v.end();
}

bool NPC::CanPetTakeItem(const EQ::ItemInstance *inst)
{
	if (!inst) {
		return false;
	}

	if (!IsPetOwnerOfClientBot() && !IsCharmedPet()) {
		return false;
	}

	const bool can_take_nodrop = (RuleB(Pets, CanTakeNoDrop) || inst->GetItem()->NoDrop != 0)
								 || inst->GetItem()->NoRent == 0;

	const bool is_charmed_with_attuned = IsCharmed() && inst->IsAttuned();

	auto o = GetOwner() && GetOwner()->IsClient() ? GetOwner()->CastToClient() : nullptr;

	struct Check {
		bool condition;
		std::string message;
	};

	const Check checks[] = {
		{inst->IsAttuned(), "I cannot equip attuned items, master."},
		{!can_take_nodrop || is_charmed_with_attuned, "I cannot equip no-drop items, master."},
		{inst->GetItem()->IsQuestItem(), "I cannot equip quest items, master."},
		{!inst->GetItem()->IsPetUsable(), "I cannot equip that item, master."}
	};

	// Iterate over checks and return false if any condition is true
	for (const auto &c : checks) {
		if (c.condition) {
			if (o) {
				o->Message(Chat::PetResponse, fmt::format("{} says '{}'", GetCleanName(), c.message).c_str());
			}
			return false;
		}
	}

	return true;
}

bool NPC::IsGuildmasterForClient(Client *c) {
	std::map<uint8, uint8> guildmaster_map = {
		{ Class::Warrior, Class::WarriorGM },
		{ Class::Cleric, Class::ClericGM },
		{ Class::Paladin, Class::PaladinGM },
		{ Class::Ranger, Class::RangerGM },
		{ Class::ShadowKnight, Class::ShadowKnightGM },
		{ Class::Druid, Class::DruidGM },
		{ Class::Monk, Class::MonkGM },
		{ Class::Bard, Class::BardGM },
		{ Class::Rogue, Class::RogueGM },
		{ Class::Shaman, Class::ShamanGM },
		{ Class::Necromancer, Class::NecromancerGM },
		{ Class::Wizard, Class::WizardGM },
		{ Class::Magician, Class::MagicianGM },
		{ Class::Enchanter, Class::EnchanterGM },
		{ Class::Beastlord, Class::BeastlordGM },
		{ Class::Berserker, Class::BerserkerGM },
	};

	if (guildmaster_map.find(c->GetClass()) != guildmaster_map.end()) {
		if (guildmaster_map[c->GetClass()] == GetClass()) {
			return true;
		}
	}

	return false;
}

bool NPC::CheckHandin(
	Client *c,
	std::map<std::string, uint32> handin,
	std::map<std::string, uint32> required,
	std::vector<EQ::ItemInstance *> items
)
{
	auto h = Handin{};
	auto r = Handin{};

	std::string log_handin_prefix = fmt::format("[{}] -> [{}]", c->GetCleanName(), GetCleanName());

	// if the npc is a multi-quest npc, we want to re-use our previously set hand-in bucket
	if (!m_handin_started && IsMultiQuestEnabled()) {
		h = m_hand_in;
	}

	if (IsMultiQuestEnabled()) {
		LogNpcHandin("{} Multi-Quest hand-in enabled", log_handin_prefix);
	}

	std::vector<std::pair<const std::map<std::string, uint32>&, Handin&>> datasets = {};

	// if we've already started the hand-in process, we don't want to re-process the hand-in data
	// we continue to use the originally set hand-in bucket and decrement from it with each successive hand-in
	if (m_handin_started) {
		h = m_hand_in;
	} else {
		datasets.emplace_back(handin, h);
	}
	datasets.emplace_back(required, r);

	const std::string set_hand_in  = "Hand-in";
	const std::string set_required = "Required";
	for (const auto &[data_map, current_handin]: datasets) {
		std::string current_dataset = &current_handin == &h ? set_hand_in : set_required;
		for (const auto &[key, value]: data_map) {
			LogNpcHandinDetail("Processing [{}] key [{}] value [{}]", current_dataset, key, value);

			// Handle items
			if (Strings::IsNumber(key)) {
				if (const auto *exists = database.GetItem(Strings::ToUnsignedInt(key));
					exists && current_dataset == set_required) {
					current_handin.items.emplace_back(HandinEntry{.item_id = key, .count = value});
				}
				continue;
			}

			// Handle money and any other key-value pairs
			if (key == "platinum") { current_handin.money.platinum = value; }
			else if (key == "gold") { current_handin.money.gold = value; }
			else if (key == "silver") { current_handin.money.silver = value; }
			else if (key == "copper") { current_handin.money.copper = value; }
		}
	}

	// pull hand-in items from the item instances
	if (!m_handin_started) {
		for (const auto &i: items) {
			if (!i) {
				continue;
			}

			h.items.emplace_back(
				HandinEntry{
					.item_id = std::to_string(i->GetItem()->ID),
					.count = std::max(static_cast<uint16>(i->IsStackable() ? i->GetCharges() : 1), static_cast<uint16>(1)),
					.item = i->Clone(),
					.is_multiquest_item = false
				}
			);
		}
	}

	// compare hand-in to required, the item_id can be in any slot
	bool requirement_met = true;

	// money
	bool money_met = h.money.platinum == r.money.platinum
					 && h.money.gold == r.money.gold
					 && h.money.silver == r.money.silver
					 && h.money.copper == r.money.copper;

	// if we started the hand-in process, we want to use the hand-in items from the member variable hand-in bucket
	auto &handin_items = !m_handin_started ? h.items : m_hand_in.items;

	for (auto &h_item: h.items) {
		LogNpcHandinDetail(
			"{} Hand-in item [{}] ({}) count [{}] is_multiquest_item [{}]",
			log_handin_prefix,
			h_item.item->GetItem()->Name,
			h_item.item_id,
			h_item.count,
			h_item.is_multiquest_item
		);
	}

	// remove items from the hand-in bucket that were used to fulfill the requirement
	std::vector<HandinEntry> items_to_remove;

	// multi-quest
	if (IsMultiQuestEnabled()) {
		for (auto &h_item: m_hand_in.items) {
			for (const auto &r_item: r.items) {
				if (h_item.item_id == r_item.item_id && h_item.count == r_item.count) {
					h_item.is_multiquest_item = true;
				}
			}
		}
	}

	// check if the hand-in items fulfill the requirement
	bool items_met = true;
	if (!handin_items.empty() && !r.items.empty()) {
		std::vector<HandinEntry> before_handin_state = handin_items;
		for (const auto &r_item : r.items) {
			uint32 remaining_requirement = r_item.count;
			bool fulfilled = false;

			// Process the hand-in items using a standard for loop
			for (size_t i = 0; i < handin_items.size() && remaining_requirement > 0; ++i) {
				auto &h_item = handin_items[i];

				// Check if the item IDs match (normalize if necessary)
				bool id_match = (h_item.item_id == r_item.item_id);

				if (id_match) {
					uint32 used_count = std::min(remaining_requirement, h_item.count);
					// If the item is a multi-quest item, we don't want to consume it for the hand-in bucket
					if (!IsMultiQuestEnabled()) {
						h_item.count -= used_count;
					}
					remaining_requirement -= used_count;

					LogNpcHandinDetail(
						"{} >>>> Using item [{}] ({}) count [{}] to fulfill [{}], remaining requirement [{}]",
						log_handin_prefix,
						h_item.item->GetItem()->Name,
						h_item.item_id,
						used_count,
						r_item.item_id,
						remaining_requirement
					);

					// If the item is fully consumed, mark it for removal
					if (h_item.count == 0) {
						items_to_remove.push_back(h_item);
					}
				}
			}

			// If we cannot fulfill the requirement, mark as not met
			if (remaining_requirement > 0) {
				LogNpcHandinDetail(
					"{} >>>> Failed to fulfill requirement for [{}], remaining [{}]",
					log_handin_prefix,
					r_item.item_id,
					remaining_requirement
				);
				items_met = false;
				break;
			} else {
				fulfilled = true;
			}
		}

		// reset the hand-in items to the state prior to processing the hand-in
		// if we failed to fulfill the requirement
		if (!items_met) {
			handin_items = before_handin_state;
			items_to_remove.clear();
		}
	}
	else if (h.items.empty() && r.items.empty()) { // no items required, money only
		items_met = true;
	}
	else {
		items_met = false;
	}

	requirement_met = money_met && items_met;

	// in-case we trigger CheckHand-in multiple times, only set these once
	if (!m_handin_started) {
		m_handin_started  = true;
		m_hand_in         = h;
		// save original items for logging
		m_hand_in.original_items = m_hand_in.items;
		m_hand_in.original_money = m_hand_in.money;
	}

	// check if npc is guildmaster
	if (IsGuildmaster()) {
		for (const auto &remove_item : items_to_remove) {
			if (!remove_item.item) {
				continue;
			}

			if (!IsDisciplineTome(remove_item.item->GetItem())) {
				continue;
			}

			if (IsGuildmasterForClient(c)) {
				c->TrainDiscipline(remove_item.item->GetID());
				m_hand_in.items.erase(
					std::remove_if(
						m_hand_in.items.begin(),
						m_hand_in.items.end(),
						[&](const HandinEntry &i) {
							bool removed = i.item == remove_item.item;
							if (removed) {
								LogNpcHandin(
									"{} Hand-in success, removing discipline tome [{}] from hand-in bucket",
									log_handin_prefix,
									i.item_id
								);
							}
							return removed;
						}
					),
					m_hand_in.items.end()
				);
			} else {
				Say("You are not a member of my guild. I will not train you!");
				requirement_met = false;
				break;
			}
		}
	}

	// print current hand-in bucket
	LogNpcHandin(
		"{} > Before processing hand-in | requirement_met [{}] item_count [{}] platinum [{}] gold [{}] silver [{}] copper [{}]",
		log_handin_prefix,
		requirement_met,
		h.items.size(),
		h.money.platinum,
		h.money.gold,
		h.money.silver,
		h.money.copper
	);

	LogNpcHandin(
		"{} >> Handed Items | Item(s) ({}) platinum [{}] gold [{}] silver [{}] copper [{}]",
		log_handin_prefix,
		h.items.size(),
		h.money.platinum,
		h.money.gold,
		h.money.silver,
		h.money.copper
	);

	int item_count = 1;
	for (const auto &i: h.items) {
		LogNpcHandin(
			"{} >>> item{} [{}] ({}) count [{}]",
			log_handin_prefix,
			item_count,
			i.item->GetItem()->Name,
			i.item_id,
			i.count
		);
		item_count++;
	}

	LogNpcHandin(
		"{} >> Required Items | Item(s) ({}) platinum [{}] gold [{}] silver [{}] copper [{}]",
		log_handin_prefix,
		r.items.size(),
		r.money.platinum,
		r.money.gold,
		r.money.silver,
		r.money.copper
	);

	item_count = 1;
	for (const auto &i: r.items) {
		auto item = database.GetItem(Strings::ToUnsignedInt(i.item_id));

		LogNpcHandin(
			"{} >>> item{} [{}] ({}) count [{}]",
			log_handin_prefix,
			item_count,
			item ? item->Name : "Unknown",
			i.item_id,
			i.count
		);

		item_count++;
	}

	if (requirement_met) {
		std::vector<std::string> log_entries = {};
		for (const auto &remove_item: items_to_remove) {
			m_hand_in.items.erase(
				std::remove_if(
					m_hand_in.items.begin(),
					m_hand_in.items.end(),
					[&](const HandinEntry &i) {
						bool removed = (remove_item.item == i.item);
						if (removed) {
							log_entries.emplace_back(
								fmt::format(
									"{} >>> Hand-in success | Removing from hand-in bucket | item [{}] ({}) count [{}]",
									log_handin_prefix,
									i.item->GetItem()->Name,
									i.item_id,
									i.count
								)
							);
						}
						return removed;
					}
				),
				m_hand_in.items.end()
			);
		}

		// log successful hand-in items
		if (!log_entries.empty()) {
			for (const auto& log : log_entries) {
				LogNpcHandin("{}", log);
			}
		}

		// decrement successful hand-in money from current hand-in bucket
		if (h.money.platinum > 0 || h.money.gold > 0 || h.money.silver > 0 || h.money.copper > 0) {
			LogNpcHandin(
				"{} Hand-in success, removing money p [{}] g [{}] s [{}] c [{}]",
				log_handin_prefix,
				h.money.platinum,
				h.money.gold,
				h.money.silver,
				h.money.copper
			);
			m_hand_in.money.platinum -= h.money.platinum;
			m_hand_in.money.gold -= h.money.gold;
			m_hand_in.money.silver -= h.money.silver;
			m_hand_in.money.copper -= h.money.copper;
		}

		LogNpcHandin(
			"{} > End of hand-in | requirement_met [{}] item_count [{}] platinum [{}] gold [{}] silver [{}] copper [{}]",
			log_handin_prefix,
			requirement_met,
			m_hand_in.items.size(),
			m_hand_in.money.platinum,
			m_hand_in.money.gold,
			m_hand_in.money.silver,
			m_hand_in.money.copper
		);
		for (const auto &i: m_hand_in.items) {
			LogNpcHandin(
				"{} Hand-in success, item [{}] ({}) count [{}]",
				log_handin_prefix,
				i.item->GetItem()->Name,
				i.item_id,
				i.count
			);
		}
	}

	// when we meet requirements under multi-quest, we want to reset the hand-in bucket
	if (requirement_met && IsMultiQuestEnabled()) {
		ResetMultiQuest();
	}

	return requirement_met;
}

NPC::Handin NPC::ReturnHandinItems(Client *c)
{
	// player event
	std::vector<PlayerEvent::HandinEntry> handin_items;
	PlayerEvent::HandinMoney              handin_money{};
	std::vector<PlayerEvent::HandinEntry> return_items;
	PlayerEvent::HandinMoney              return_money{};
	for (const auto& i : m_hand_in.original_items) {
		if (i.item && i.item->GetItem()) {
			handin_items.emplace_back(
				PlayerEvent::HandinEntry{
					.item_id = i.item->GetID(),
					.item_name = i.item->GetItem()->Name,
					.augment_ids = i.item->GetAugmentIDs(),
					.augment_names = i.item->GetAugmentNames(),
					.charges = std::max(static_cast<uint16>(i.item->GetCharges()), static_cast<uint16>(1))
				}
			);
		}
	}

	auto returned = m_hand_in;

	// check if any money was handed in
	if (m_hand_in.original_money.platinum > 0 ||
		m_hand_in.original_money.gold > 0 ||
		m_hand_in.original_money.silver > 0 ||
		m_hand_in.original_money.copper > 0
		) {
		handin_money.copper   = m_hand_in.original_money.copper;
		handin_money.silver   = m_hand_in.original_money.silver;
		handin_money.gold     = m_hand_in.original_money.gold;
		handin_money.platinum = m_hand_in.original_money.platinum;
	}

	// if scripts have their own implementation of returning items instead of
	// going through return_items, this guards against returning items twice (duplicate items)
	bool external_returned_items = c->GetExternalHandinItemsReturned().size() > 0;
	bool returned_items_already = false;
	for (auto &handin_item: m_hand_in.items) {
		for (auto &i: c->GetExternalHandinItemsReturned()) {
			auto item = database.GetItem(i);
			if (item && std::to_string(item->ID) == handin_item.item_id) {
				LogNpcHandin(" -- External quest methods already returned item [{}] ({})", item->Name, item->ID);
				returned_items_already = true;
			}
		}
	}

	if (returned_items_already) {
		LogNpcHandin("External quest methods returned items, not returning items to player via ReturnHandinItems");
	}

	bool returned_handin = false;
	m_hand_in.items.erase(
		std::remove_if(
			m_hand_in.items.begin(),
			m_hand_in.items.end(),
			[&](HandinEntry &i) {
				if (i.item && i.item->GetItem() && !i.is_multiquest_item && !returned_items_already) {
					return_items.emplace_back(
						PlayerEvent::HandinEntry{
							.item_id = i.item->GetID(),
							.item_name = i.item->GetItem()->Name,
							.augment_ids = i.item->GetAugmentIDs(),
							.augment_names = i.item->GetAugmentNames(),
							.charges = std::max(static_cast<uint16>(i.item->GetCharges()), static_cast<uint16>(1))
						}
					);

					// If the item is stackable and the new charges don't match the original count
					// set the charges to the original count
					if (i.item->IsStackable() && i.item->GetCharges() != i.count) {
						i.item->SetCharges(i.count);
					}

					c->PushItemOnCursor(*i.item, true);
					LogNpcHandin(
						"Hand-in failed, returning item [{}] i.is_multiquest_item [{}]",
						i.item->GetItem()->Name,
						i.is_multiquest_item
					);

					returned_handin = true;
					return true; // Mark this item for removal
				}
				return false;
			}
		),
		m_hand_in.items.end()
	);

	// check if any money was handed in via external quest methods
	auto em = c->GetExternalHandinMoneyReturned();

	bool money_returned_via_external_quest_methods =
			 em.copper > 0 ||
			 em.silver > 0 ||
			 em.gold > 0 ||
			 em.platinum > 0;

	// check if any money was handed in
	bool money_handed = m_hand_in.money.platinum > 0 ||
						m_hand_in.money.gold > 0 ||
						m_hand_in.money.silver > 0 ||
						m_hand_in.money.copper > 0;
	if (money_handed && !money_returned_via_external_quest_methods) {
		c->AddMoneyToPP(
			m_hand_in.money.copper,
			m_hand_in.money.silver,
			m_hand_in.money.gold,
			m_hand_in.money.platinum,
			true
		);
		returned_handin = true;
		LogNpcHandin(
			"Hand-in failed, returning money p [{}] g [{}] s [{}] c [{}]",
			m_hand_in.money.platinum,
			m_hand_in.money.gold,
			m_hand_in.money.silver,
			m_hand_in.money.copper
		);

		// player event
		return_money.copper   = m_hand_in.money.copper;
		return_money.silver   = m_hand_in.money.silver;
		return_money.gold     = m_hand_in.money.gold;
		return_money.platinum = m_hand_in.money.platinum;

		// if multi-quest and we returned money, reset the hand-in bucket
		if (IsMultiQuestEnabled()) {
			m_hand_in.money = {};
			m_hand_in.original_money = {};
		}
	}

	if (money_returned_via_external_quest_methods) {
		LogNpcHandin(
			"Money handed in was returned via external quest methods, not returning money to player via ReturnHandinItems | handed-in p [{}] g [{}] s [{}] c [{}] returned-external p [{}] g [{}] s [{}] c [{}] source [{}]",
			m_hand_in.money.platinum,
			m_hand_in.money.gold,
			m_hand_in.money.silver,
			m_hand_in.money.copper,
			em.platinum,
			em.gold,
			em.silver,
			em.copper,
			em.return_source
		);
	}

	m_has_processed_handin_return = returned_handin;

	if (returned_handin) {
		Say(
			fmt::format(
				"I have no need for this {}, you can have it back.",
				c->GetCleanName()
			).c_str()
		);
	}

	const bool handed_in_money = (
		handin_money.platinum > 0 ||
		handin_money.gold > 0 ||
		handin_money.silver > 0 ||
		handin_money.copper > 0
	);
	const bool event_has_data_to_record = !handin_items.empty() || handed_in_money;

	if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::NPC_HANDIN) && event_has_data_to_record) {
		auto e = PlayerEvent::HandinEvent{
			.npc_id = GetNPCTypeID(),
			.npc_name = GetCleanName(),
			.handin_items = handin_items,
			.handin_money = handin_money,
			.return_items = return_items,
			.return_money = return_money,
			.is_quest_handin = parse->HasQuestSub(GetNPCTypeID(), EVENT_TRADE)
		};

		RecordPlayerEventLogWithClient(c, PlayerEvent::NPC_HANDIN, e);
	}

	return returned;
}

void NPC::ResetHandin()
{
	LogNpcHandin("Resetting hand-in bucket for [{}]", GetCleanName());
	m_has_processed_handin_return = false;
	m_handin_started              = false;
	if (!IsMultiQuestEnabled()) {
		for (auto &i: m_hand_in.original_items) {
			safe_delete(i.item);
		}

		m_hand_in = {};
	}
}

void NPC::ResetMultiQuest() {
	LogNpcHandin("Resetting multi-quest hand-in bucket for [{}]", GetCleanName());
	for (auto &i: m_hand_in.original_items) {
		safe_delete(i.item);
	}

	m_hand_in = {};
}

void NPC::SetNPCTintIndex(uint32 index)
{
	auto outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
	auto* s = (SpawnAppearance_Struct*) outapp->pBuffer;

	s->spawn_id  = GetID();
	s->type      = AppearanceType::NPCTintIndex;
	s->parameter = index;

	entity_list.QueueClients(this, outapp);
	safe_delete(outapp);
}
