/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2004 EQEMu Development Team (http://eqemu.org)

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

#include "mob.h"

#include "common/data_verification.h"
#include "common/features.h"
#include "common/repositories/criteria/content_filter_criteria.h"
#include "common/repositories/npc_spells_entries_repository.h"
#include "common/repositories/npc_spells_repository.h"
#include "common/rulesys.h"
#include "common/strings.h"
#include "zone/bot.h"
#include "zone/client.h"
#include "zone/entity.h"
#include "zone/fastmath.h"
#include "zone/map.h"
#include "zone/npc.h"
#include "zone/quest_parser_collection.h"
#include "zone/string_ids.h"
#include "zone/water_map.h"

#include "glm/gtx/projection.hpp"
#include <algorithm>
#include <iostream>
#include <limits>

extern EntityList entity_list;
extern FastMath g_Math;

extern Zone *zone;

#if EQDEBUG >= 12
	#define MobAI_DEBUG_Spells	25
#elif EQDEBUG >= 9
	#define MobAI_DEBUG_Spells	10
#else
	#define MobAI_DEBUG_Spells	-1
#endif

//NOTE: do NOT pass in beneficial and detrimental spell types into the same call here!
bool NPC::AICastSpell(Mob* tar, uint8 iChance, uint32 iSpellTypes, bool bInnates) {
	if (!tar)
		return false;

	if (IsNoCast())
		return false;

	if(AI_HasSpells() == false)
		return false;

	// Rooted mobs were just standing around when tar out of range.
	// Any sane mob would cast if they can.
	bool cast_only_option = (IsRooted() && !CombatRange(tar));

	// innates are always attempted
	if (!cast_only_option && iChance < 100 && !bInnates) {
		if (zone->random.Int(0, 100) >= iChance)
			return false;
	}

	float dist2;

	if (iSpellTypes & SpellType_Escape) {
		dist2 = 0; //DistNoRoot(*this);	//WTF was up with this...
	}
	else
		dist2 = DistanceSquared(m_Position, tar->GetPosition());

	bool checked_los = false;	//we do not check LOS until we are absolutely sure we need to, and we only do it once.

	float manaR = GetManaRatio();
	for (int i = static_cast<int>(AIspells.size()) - 1; i >= 0; i--) {
		if (!IsValidSpell(AIspells[i].spellid)) {
			// this is both to quit early to save cpu and to avoid casting bad spells
			// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
			//return false;
			continue;
		}

		if ((AIspells[i].priority == 0 && !bInnates) || (AIspells[i].priority != 0 && bInnates)) {
			// so "innate" spells are special and spammed a bit
			// we define an innate spell as a spell with priority 0
			continue;
		}

		// we reuse these fields for heal overrides
		if (AIspells[i].type != SpellType_Heal && AIspells[i].min_hp != 0 && GetIntHPRatio() < AIspells[i].min_hp)
			continue;

		if (AIspells[i].type != SpellType_Heal && AIspells[i].max_hp != 0 && GetIntHPRatio() > AIspells[i].max_hp)
			continue;

		if (iSpellTypes & AIspells[i].type) {
			// manacost has special values, -1 is no mana cost, -2 is instant cast (no mana)
			int32 mana_cost = AIspells[i].manacost;
			if (mana_cost == -1)
				mana_cost = spells[AIspells[i].spellid].mana;
			else if (mana_cost == -2)
				mana_cost = 0;
			// this is ugly -- ignore distance for hatelist spells, looks like the client is only checking distance for some targettypes in CastSpell,
			// should probably match that eventually. This should be good enough for now I guess ....
			if (
				(
				 (spells[AIspells[i].spellid].target_type == ST_HateList || spells[AIspells[i].spellid].target_type == ST_AETargetHateList) ||
				 (
				  // note: I think this check is actually wrong and we should be checking range instead in all cases, BUT if range is 0, range check is skipped? Works for now
				  (spells[AIspells[i].spellid].target_type==ST_AECaster || spells[AIspells[i].spellid].target_type==ST_AEBard || spells[AIspells[i].spellid].target_type==ST_AEClientV1)
				  && dist2 <= spells[AIspells[i].spellid].aoe_range*spells[AIspells[i].spellid].aoe_range
				 ) ||
				 dist2 <= spells[AIspells[i].spellid].range*spells[AIspells[i].spellid].range
				 )
				&& (mana_cost <= GetMana() || GetMana() == GetMaxMana())
				&& (AIspells[i].time_cancast + (zone->random.Int(0, 4) * 500)) <= Timer::GetCurrentTime() //break up the spelling casting over a period of time.
				) {

				LogAI("Casting: spellid [{}] tar [{}] dist2[[{}]]<=[{}] mana_cost[[{}]]<=[{}] cancast[[{}]]<=[{}] type [{}]",
					AIspells[i].spellid, tar->GetName(), dist2, (spells[AIspells[i].spellid].range * spells[AIspells[i].spellid].range), mana_cost, GetMana(), AIspells[i].time_cancast, Timer::GetCurrentTime(), AIspells[i].type);

				switch (AIspells[i].type) {
					case SpellType_Heal: {
						if (
							(spells[AIspells[i].spellid].target_type == ST_Target || tar == this)
							&& tar->DontHealMeBefore() < Timer::GetCurrentTime()
							&& !(tar->IsPet() && tar->GetOwner()->IsOfClientBot())	//no buffing PC's pets
							) {

							auto hp_ratio = tar->GetIntHPRatio();

							int min_hp = AIspells[i].min_hp; // well 0 is default, so no special case here
							int max_hp = AIspells[i].max_hp ? AIspells[i].max_hp : RuleI(Spells, AI_HealHPPct);

							if (EQ::ValueWithin(hp_ratio, min_hp, max_hp) || (tar->IsClient() && hp_ratio <= 99)) { // not sure about client bit, leaving it
								uint32 tempTime = 0;
								AIDoSpellCast(i, tar, mana_cost, &tempTime);
								tar->SetDontHealMeBefore(tempTime);
								return true;
							}
						}
						break;
					}
					case SpellType_Root: {
						Mob *rootee = GetHateRandom();
						if (rootee && !rootee->IsRooted() && !rootee->IsFeared() && (bInnates || zone->random.Roll(50))
							&& rootee->DontRootMeBefore() < Timer::GetCurrentTime()
							&& rootee->CanBuffStack(AIspells[i].spellid, GetLevel(), true) >= 0
							) {
							if(!checked_los) {
								if(!CheckLosFN(rootee))
									return(false);	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
								checked_los = true;
							}
							uint32 tempTime = 0;
							AIDoSpellCast(i, rootee, mana_cost, &tempTime);
							rootee->SetDontRootMeBefore(tempTime);
							return true;
						}
						break;
					}
					case SpellType_Buff: {
						if (
							(spells[AIspells[i].spellid].target_type == ST_Target || tar == this)
							&& tar->DontBuffMeBefore() < Timer::GetCurrentTime()
							&& !tar->IsImmuneToSpell(AIspells[i].spellid, this)
							&& tar->CanBuffStack(AIspells[i].spellid, GetLevel(), true) >= 0
							&& !(tar->IsPet() && tar->GetOwner()->IsOfClientBot() && this != tar)	//no buffing PC's pets, but they can buff themself
							)
						{
							if(!checked_los) {
								if(!CheckLosFN(tar))
									return(false);
								checked_los = true;
							}
							uint32 tempTime = 0;
							AIDoSpellCast(i, tar, mana_cost, &tempTime);
							tar->SetDontBuffMeBefore(tempTime);
							return true;
						}
						break;
					}

					case SpellType_InCombatBuff: {
						if(bInnates || zone->random.Roll(50)) {
							if (tar->CanBuffStack(AIspells[i].spellid, GetLevel(), true) >= 0) {
								AIDoSpellCast(i, tar, mana_cost);
								return true;
							}
						}
						break;
					}

					case SpellType_Escape: {
						// If min_hp !=0 then the spell list has specified
						// custom range and we're inside that range if we
						// made it here.
						if (AIspells[i].min_hp != 0 || GetHPRatio() <= (RuleI(NPC, NPCGatePercent))) {
							auto npcSpawnPoint = CastToNPC()->GetSpawnPoint();
							if (!RuleB(NPC, NPCGateNearBind) && DistanceNoZ(m_Position, npcSpawnPoint) < RuleI(NPC, NPCGateDistanceBind)) {
								break;
							} else {
								AIDoSpellCast(i, tar, mana_cost);
								return true;
							}
						}
						break;
					}
					case SpellType_Slow:
					case SpellType_Debuff: {
						Mob * debuffee = GetHateRandom();
						if (debuffee && manaR >= 10 && (bInnates || zone->random.Roll(70)) &&
								debuffee->CanBuffStack(AIspells[i].spellid, GetLevel(), true) >= 0) {
							if (!checked_los) {
								if (!CheckLosFN(debuffee))
									return false;
								checked_los = true;
							}
							AIDoSpellCast(i, debuffee, mana_cost);
							return true;
						}
						break;
					}
					case SpellType_Nuke: {
						if (
							manaR >= 10 && (bInnates || (zone->random.Roll(70)
							&& tar->CanBuffStack(AIspells[i].spellid, GetLevel(), false) >= 0)) // saying it's a nuke here, AI shouldn't care too much if overwriting
							) {
							if(!checked_los) {
								if(!CheckLosFN(tar))
									return(false);	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
								checked_los = true;
							}
							AIDoSpellCast(i, tar, mana_cost);
							return true;
						}
						break;
					}
					case SpellType_Dispel: {
						if(bInnates || zone->random.Roll(15))
						{
							if(!checked_los) {
								if(!CheckLosFN(tar))
									return(false);	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
								checked_los = true;
							}
							if(tar->CountDispellableBuffs() > 0)
							{
								AIDoSpellCast(i, tar, mana_cost);
								return true;
							}
						}
						break;
					}
					case SpellType_Mez: {
						if(bInnates || zone->random.Roll(20))
						{
							Mob * mezTar = nullptr;
							mezTar = entity_list.GetTargetForMez(this);

							if(mezTar && mezTar->CanBuffStack(AIspells[i].spellid, GetLevel(), true) >= 0)
							{
								AIDoSpellCast(i, mezTar, mana_cost);
								return true;
							}
						}
						break;
					}

					case SpellType_Charm:
					{
						if(!IsPet() && (bInnates || zone->random.Roll(20)))
						{
							Mob * chrmTar = GetHateRandom();
							if(chrmTar && chrmTar->CanBuffStack(AIspells[i].spellid, GetLevel(), true) >= 0)
							{
								AIDoSpellCast(i, chrmTar, mana_cost);
								return true;
							}
						}
						break;
					}

					case SpellType_Pet: {
						//keep mobs from recasting pets when they have them.
						if (!IsPet() && !GetPetID() && (bInnates || zone->random.Roll(25))) {
							AIDoSpellCast(i, tar, mana_cost);
							return true;
						}
						break;
					}
					case SpellType_Lifetap: {
						if (GetHPRatio() <= 95
							&& (bInnates || zone->random.Roll(50))
							&& tar->CanBuffStack(AIspells[i].spellid, GetLevel(), true) >= 0
							) {
							if(!checked_los) {
								if(!CheckLosFN(tar))
									return(false);	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
								checked_los = true;
							}
							AIDoSpellCast(i, tar, mana_cost);
							return true;
						}
						break;
					}
					case SpellType_Snare: {
						if (
							!tar->IsRooted()
							&& (bInnates || zone->random.Roll(50))
							&& tar->DontSnareMeBefore() < Timer::GetCurrentTime()
							&& tar->CanBuffStack(AIspells[i].spellid, GetLevel(), true) >= 0
							) {
							if(!checked_los) {
								if(!CheckLosFN(tar))
									return(false);	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
								checked_los = true;
							}
							uint32 tempTime = 0;
							AIDoSpellCast(i, tar, mana_cost, &tempTime);
							tar->SetDontSnareMeBefore(tempTime);
							return true;
						}
						break;
					}
					case SpellType_DOT: {
						if (
							(bInnates || zone->random.Roll(60))
							&& tar->DontDotMeBefore() < Timer::GetCurrentTime()
							&& tar->CanBuffStack(AIspells[i].spellid, GetLevel(), true) >= 0
							) {
							if(!checked_los) {
								if(!CheckLosFN(tar))
									return(false);	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
								checked_los = true;
							}
							uint32 tempTime = 0;
							AIDoSpellCast(i, tar, mana_cost, &tempTime);
							tar->SetDontDotMeBefore(tempTime);
							return true;
						}
						break;
					}
					default: {
						std::cout << "Error: Unknown spell type in AICastSpell. caster:" << GetName() << " type:" << AIspells[i].type << " slot:" << i << std::endl;
						break;
					}
				}
			}
			else {
				LogAI("NotCasting: spellid [{}] tar [{}] dist2[[{}]]<=[{}] mana_cost[[{}]]<=[{}] cancast[[{}]]<=[{}] type [{}]",
					AIspells[i].spellid, tar->GetName(), dist2, (spells[AIspells[i].spellid].range * spells[AIspells[i].spellid].range), mana_cost, GetMana(), AIspells[i].time_cancast, Timer::GetCurrentTime(), AIspells[i].type);
			}
		}
	}
	return false;
}

bool NPC::AIDoSpellCast(int32 i, Mob* tar, int32 mana_cost, uint32* oDontDoAgainBefore) {
	LogAI("spellid [{}] tar [{}] mana [{}] Name [{}]", AIspells[i].spellid, tar->GetName(), mana_cost, spells[AIspells[i].spellid].name);
	casting_spell_AIindex = i;

	return CastSpell(AIspells[i].spellid, tar->GetID(), EQ::spells::CastingSlot::Gem2, AIspells[i].manacost == -2 ? 0 : -1, mana_cost, oDontDoAgainBefore, -1, -1, 0, &(AIspells[i].resist_adjust));
}

void Mob::AI_Init()
{
	pAIControlled = false;
	AI_think_timer.reset(nullptr);
	AI_walking_timer.reset(nullptr);
	AI_movement_timer.reset(nullptr);
	AI_target_check_timer.reset(nullptr);
	AI_feign_remember_timer.reset(nullptr);
	AI_scan_area_timer.reset(nullptr);
	AI_check_signal_timer.reset(nullptr);
	AI_scan_door_open_timer.reset(nullptr);

	minLastFightingDelayMoving = RuleI(NPC, LastFightingDelayMovingMin);
	maxLastFightingDelayMoving = RuleI(NPC, LastFightingDelayMovingMax);

	m_dont_heal_me_before  = 0;
	m_dont_buff_me_before  = Timer::GetCurrentTime() + 400;
	m_dont_dot_me_before   = 0;
	m_dont_root_me_before  = 0;
	m_dont_snare_me_before = 0;
	m_dont_cure_me_before  = 0;
}

void NPC::AI_Init()
{
	AIautocastspell_timer.reset(nullptr);
	casting_spell_AIindex = static_cast<uint8>(AIspells.size());

	m_roambox.max_x     = 0;
	m_roambox.max_y     = 0;
	m_roambox.min_x     = 0;
	m_roambox.min_y     = 0;
	m_roambox.distance  = 0;
	m_roambox.dest_x    = 0;
	m_roambox.dest_y    = 0;
	m_roambox.dest_z    = 0;
	m_roambox.delay     = 2500;
	m_roambox.min_delay = 2500;
}

void Client::AI_Init()
{
	minLastFightingDelayMoving = CLIENT_LD_TIMEOUT;
	maxLastFightingDelayMoving = CLIENT_LD_TIMEOUT;
}

void Mob::AI_Start(uint32 iMoveDelay) {
	if (pAIControlled)
		return;

	if (iMoveDelay)
		time_until_can_move = Timer::GetCurrentTime() + iMoveDelay;
	else
		time_until_can_move = 0;

	pAIControlled  = true;
	AI_think_timer = std::make_unique<Timer>(AIthink_duration);
	AI_think_timer->Trigger();

	AI_walking_timer        = std::make_unique<Timer>(0);
	AI_movement_timer       = std::make_unique<Timer>(AImovement_duration);
	AI_target_check_timer   = std::make_unique<Timer>(AItarget_check_duration);
	AI_feign_remember_timer = std::make_unique<Timer>(AIfeignremember_delay);
	AI_scan_door_open_timer = std::make_unique<Timer>(AI_scan_door_open_interval);

	if (GetBodyType() == BodyType::Animal && !RuleB(NPC, AnimalsOpenDoors)) {
		SetCanOpenDoors(false);
	}

	if(!RuleB(Aggro, NPCAggroMaxDistanceEnabled)) {
		hate_list_cleanup_timer.Disable();
	}

	if (CastToNPC()->GetNPCAggro())
		AI_scan_area_timer = std::make_unique<Timer>(RandomTimer(RuleI(NPC, NPCToNPCAggroTimerMin), RuleI(NPC, NPCToNPCAggroTimerMax)));

	AI_check_signal_timer = std::make_unique<Timer>(AI_check_signal_timer_delay);


	if (GetAggroRange() == 0)
		pAggroRange = 70;
	if (GetAssistRange() == 0)
		pAssistRange = 70;

	hate_list.WipeHateList();

	m_Delta = glm::vec4();
	pRunAnimSpeed = 0;
}

void Client::AI_Start(uint32 iMoveDelay) {
	Mob::AI_Start(iMoveDelay);

	if (!pAIControlled)
		return;

	pClientSideTarget = GetTarget() ? GetTarget()->GetID() : 0;
	SendAppearancePacket(AppearanceType::Animation, Animation::Freeze);	// this freezes the client
	SendAppearancePacket(AppearanceType::Linkdead, 1); // Sending LD packet so *LD* appears by the player name when charmed/feared -Kasai
	SetAttackTimer();
	SetFeigned(false);
}

void NPC::AI_Start(uint32 iMoveDelay) {
	Mob::AI_Start(iMoveDelay);
	if (!pAIControlled)
		return;

	if (AIspells.empty()) {
		AIautocastspell_timer = std::make_unique<Timer>(1000);
		AIautocastspell_timer->Disable();
	} else {
		AIautocastspell_timer = std::make_unique<Timer>(500);
		AIautocastspell_timer->Start(RandomTimer(0, 300), false);
	}

	if (NPCTypedata) {
		AI_AddNPCSpells(NPCTypedata->npc_spells_id);
		ProcessSpecialAbilities(NPCTypedata->special_abilities);
		AI_AddNPCSpellsEffects(NPCTypedata->npc_spells_effects_id);
	}

	SendTo(GetX(), GetY(), GetZ());
	SaveGuardSpot(GetPosition());
}

void Mob::AI_Stop() {
	if (!IsAIControlled())
		return;

	pAIControlled = false;

	AI_think_timer.reset(nullptr);
	AI_walking_timer.reset(nullptr);
	AI_movement_timer.reset(nullptr);
	AI_target_check_timer.reset(nullptr);
	AI_scan_area_timer.reset(nullptr);
	AI_feign_remember_timer.reset(nullptr);
	AI_check_signal_timer.reset(nullptr);
	AI_scan_door_open_timer.reset(nullptr);

	hate_list.WipeHateList();
}

void NPC::AI_Stop() {
	Waypoints.clear();
	AIautocastspell_timer.reset(nullptr);
}

void Client::AI_Stop() {
	Mob::AI_Stop();
	MessageString(Chat::Red,PLAYER_REGAIN);

	auto app = new EQApplicationPacket(OP_Charm, sizeof(Charm_Struct));
	Charm_Struct *ps = (Charm_Struct*)app->pBuffer;
	ps->owner_id = 0;
	ps->pet_id = GetID();
	ps->command = 0;
	entity_list.QueueClients(this, app);
	safe_delete(app);

	SetTarget(entity_list.GetMob(pClientSideTarget));
	SendAppearancePacket(AppearanceType::Animation, GetAppearanceValue(GetAppearance()));
	SendAppearancePacket(AppearanceType::Linkdead, 0); // Removing LD packet so *LD* no longer appears by the player name when charmed/feared -Kasai
	if (!auto_attack) {
		attack_timer.Disable();
		attack_dw_timer.Disable();
	}
	if (IsLD())
	{
		Save();
		Disconnect();
	}
}

// only call this on a zone shutdown event
void Mob::AI_ShutDown() {
	attack_timer.Disable();
	attack_dw_timer.Disable();
	ranged_timer.Disable();
	tic_timer.Disable();
	mana_timer.Disable();
	spellend_timer.Disable();
	rewind_timer.Disable();
	bindwound_timer.Disable();
	stunned_timer.Disable();
	spun_timer.Disable();
	bardsong_timer.Disable();
	gravity_timer.Disable();
	viral_timer.Disable();
	flee_timer.Disable();

	for (int sat = 0; sat < SpecialAbility::Max; ++sat) {
		if (SpecialAbilities[sat].timer)
			SpecialAbilities[sat].timer->Disable();
	}
}

//todo: expand the logic here to cover:
//redundant debuffs
//buffing owner
//certain types of det spells that need special behavior.
void Client::AI_SpellCast()
{
	if(!charm_cast_timer.Check())
		return;

	Mob *targ = GetTarget();
	if(!targ)
		return;

	float dist = DistanceSquaredNoZ(m_Position, targ->GetPosition());

	std::vector<uint32> valid_spells;
	std::vector<uint32> slots;

	for(uint32 x = 0; x < 9; ++x)
	{
		uint32 current_spell = m_pp.mem_spells[x];
		if(!IsValidSpell(current_spell))
			continue;

		if(IsBeneficialSpell(current_spell))
		{
			continue;
		}

		if(dist > spells[current_spell].range*spells[current_spell].range)
		{
			continue;
		}

		if(GetMana() < spells[current_spell].mana)
		{
			continue;
		}

		if(IsEffectInSpell(current_spell, SpellEffect::Charm))
		{
			continue;
		}

		if(!GetPTimers().Expired(&database, pTimerSpellStart + current_spell, false))
		{
			continue;
		}

		if(targ->CanBuffStack(current_spell, GetLevel(), true) < 0)
		{
			continue;
		}

		//bard songs cause trouble atm
		if(IsBardSong(current_spell))
			continue;

		valid_spells.push_back(current_spell);
		slots.push_back(x);
	}

	uint32 spell_to_cast = 0xFFFFFFFF;
	EQ::spells::CastingSlot slot_to_use = EQ::spells::CastingSlot::Item;
	if(valid_spells.size() == 1)
	{
		spell_to_cast = valid_spells[0];
		slot_to_use = static_cast<EQ::spells::CastingSlot>(slots[0]);
	}
	else if(valid_spells.empty())
	{
		return;
	}
	else
	{
		uint32 idx = zone->random.Int(0, (valid_spells.size()-1));
		spell_to_cast = valid_spells[idx];
		slot_to_use = static_cast<EQ::spells::CastingSlot>(slots[idx]);
	}

	if(IsMesmerizeSpell(spell_to_cast) || IsFearSpell(spell_to_cast))
	{
		Mob *tar = entity_list.GetTargetForMez(this);
		if(!tar)
		{
			tar = GetTarget();
			if(tar && IsFearSpell(spell_to_cast))
			{
				if(!IsBardSong(spell_to_cast))
				{
					StopNavigation();
				}
				CastSpell(spell_to_cast, tar->GetID(), slot_to_use);
				return;
			}
		}
	}
	else
	{
		Mob *tar = GetTarget();
		if(tar)
		{
			if(!IsBardSong(spell_to_cast))
			{
				StopNavigation();
			}
			CastSpell(spell_to_cast, tar->GetID(), slot_to_use);
			return;
		}
	}


}

void Client::AI_Process()
{

	if (!IsAIControlled())
		return;

	if (!(AI_think_timer->Check() || attack_timer.Check(false)))
		return;

	if (IsCasting())
		return;

	bool engaged = IsEngaged();

	Mob *ow = GetOwner();
	if(!engaged)
	{
		if(ow)
		{
			if(ow->IsEngaged())
			{
				Mob *tar = ow->GetTarget();
				if(tar)
				{
					AddToHateList(tar, 1, 0, false);
				}
			}
		}
	}

	if(!ow)
	{
		if(!IsFeared() && !IsLD())
		{
			BuffFadeByEffect(SpellEffect::Charm);
			return;
		}
	}

	if (RuleB(Combat, EnableFearPathing)) {
		if (currently_fleeing) {

			if (IsRooted()) {
				//make sure everybody knows were not moving, for appearance sake
				if (IsMoving()) {
					FaceTarget();
					StopNavigation();
				}
				//continue on to attack code, ensuring that we execute the engaged code
				engaged = true;
			}
			else {
				if (AI_movement_timer->Check()) {
					// Check if we have reached the last fear point
					if(IsPositionEqualWithinCertainZ(glm::vec3(GetX(), GetY(), GetZ()), m_FearWalkTarget, 5.0f)) {
						CalculateNewFearpoint();
					}
					else {
						RunTo(m_FearWalkTarget.x, m_FearWalkTarget.y, m_FearWalkTarget.z);
					}
				}
				if (RuleB(Character, ProcessFearedProximity) && proximity_timer.Check()) {
					entity_list.ProcessMove(this, glm::vec3(GetX(), GetY(), GetZ()));
					if (RuleB(TaskSystem, EnableTaskSystem) && RuleB(TaskSystem, EnableTaskProximity))
						ProcessTaskProximities(GetX(), GetY(), GetZ());

					m_Proximity = glm::vec3(GetX(), GetY(), GetZ());
				}
				return;
			}

		}
	}

	if (engaged)
	{
		if (IsRooted())
			SetTarget(hate_list.GetClosestEntOnHateList(this));
		else
		{
			if(AI_target_check_timer->Check())
			{
				SetTarget(hate_list.GetMobWithMostHateOnList(this));
			}
		}

		if (!GetTarget())
			return;

		if (GetTarget()->IsCorpse()) {
			RemoveFromHateList(this);
			RemoveFromRampageList(this);
			return;
		}

		if(DivineAura())
			return;

		bool is_combat_range = CombatRange(GetTarget());

		if (is_combat_range) {
			if (IsMoving()) {
				StopNavigation();
			}

			if (charm_class_attacks_timer.Check()) {
				DoClassAttacks(GetTarget());
			}

			if (AI_movement_timer->Check()) {
				if (CalculateHeadingToTarget(GetTarget()->GetX(), GetTarget()->GetY()) !=
				    m_Position.w) {
					FaceTarget();
				}
			}
			if (GetTarget() && !IsStunned() && !IsMezzed() && !GetFeigned() && IsAttackAllowed(GetTarget())) {
				if (attack_timer.Check()) {
					// Should charmed clients not be procing?
					DoAttackRounds(GetTarget(), EQ::invslot::slotPrimary);
				}
			}

			if (CanThisClassDualWield() && GetTarget() && !IsStunned() && !IsMezzed() && !GetFeigned() && IsAttackAllowed(GetTarget())) {
				if (attack_dw_timer.Check()) {
					if (CheckDualWield()) {
						// Should charmed clients not be procing?
						DoAttackRounds(GetTarget(), EQ::invslot::slotSecondary);
					}
				}
			}
		} else {
			if(!IsRooted())
			{
				if(AI_movement_timer->Check())
				{
					RunTo(GetTarget()->GetX(), GetTarget()->GetY(), GetTarget()->GetZ());
				}
			}
			else if(IsMoving())
			{
				FaceTarget();
			}
		}
		AI_SpellCast();
	}
	else
	{
		if(AI_feign_remember_timer->Check()) {
			std::set<uint32>::iterator remembered_feigned_mobid;
			remembered_feigned_mobid = feign_memory_list.begin();
			while (remembered_feigned_mobid != feign_memory_list.end()) {

				Mob* remembered_mob = entity_list.GetMob(*remembered_feigned_mobid);
				if (remembered_mob == nullptr || remembered_mob->IsCorpse()) {
					//they are gone now...
					remembered_feigned_mobid = feign_memory_list.erase(remembered_feigned_mobid);
				} else if (!remembered_mob->GetFeigned()) {
					AddToHateList(remembered_mob,1);
					remembered_feigned_mobid = feign_memory_list.erase(remembered_feigned_mobid);
					break;
				} else {
					//they are still feigned, carry on...
					++remembered_feigned_mobid;
				}
			}
		}

		if (IsPet()) {
			Mob *owner = GetOwner();
			if (owner == nullptr)
				return;

			float dist = DistanceSquared(m_Position, owner->GetPosition());
			if (dist >= 202500) { // >= 450 distance
				Teleport(owner->GetPosition());
			} else if (dist >= 400) { // >=20
				if (AI_movement_timer->Check()) {
					if (dist >= 1225) {
						RunTo(owner->GetX(), owner->GetY(), owner->GetZ());
					}
					else {
						WalkTo(owner->GetX(), owner->GetY(), owner->GetZ());
					}
				}
			} else {
				StopNavigation();
			}
		}
	}
}

void Mob::ProcessForcedMovement()
{
	// we are being pushed, we will hijack this movement timer
	// this also needs to be done before casting to have a chance to interrupt
	// this flag won't be set if the mob can't be pushed (rooted etc)
	if (AI_movement_timer->Check()) {
		bool bPassed = true;
		glm::vec3 normal;

		// no zone map = fucked
		if (zone->HasMap()) {
			// in front
			m_CollisionBox[0].x = m_Position.x + 3.0f * g_Math.FastSin(0.0f);
			m_CollisionBox[0].y = m_Position.y + 3.0f * g_Math.FastCos(0.0f);
			m_CollisionBox[0].z = m_Position.z;

			// 45 right front
			m_CollisionBox[1].x = m_Position.x + 3.0f * g_Math.FastSin(64.0f);
			m_CollisionBox[1].y = m_Position.y + 3.0f * g_Math.FastCos(64.0f);
			m_CollisionBox[1].z = m_Position.z;

			// to right
			m_CollisionBox[2].x = m_Position.x + 3.0f * g_Math.FastSin(128.0f);
			m_CollisionBox[2].y = m_Position.y + 3.0f * g_Math.FastCos(128.0f);
			m_CollisionBox[2].z = m_Position.z;

			// 45 right back
			m_CollisionBox[3].x = m_Position.x + 3.0f * g_Math.FastSin(192.0f);
			m_CollisionBox[3].y = m_Position.y + 3.0f * g_Math.FastCos(192.0f);
			m_CollisionBox[3].z = m_Position.z;

			// behind
			m_CollisionBox[4].x = m_Position.x + 3.0f * g_Math.FastSin(256.0f);
			m_CollisionBox[4].y = m_Position.y + 3.0f * g_Math.FastCos(256.0f);
			m_CollisionBox[4].z = m_Position.z;

			// 45 left back
			m_CollisionBox[5].x = m_Position.x + 3.0f * g_Math.FastSin(320.0f);
			m_CollisionBox[5].y = m_Position.y + 3.0f * g_Math.FastCos(320.0f);
			m_CollisionBox[5].z = m_Position.z;

			// to left
			m_CollisionBox[6].x = m_Position.x + 3.0f * g_Math.FastSin(384.0f);
			m_CollisionBox[6].y = m_Position.y + 3.0f * g_Math.FastCos(384.0f);
			m_CollisionBox[6].z = m_Position.z;

			// 45 left front
			m_CollisionBox[7].x = m_Position.x + 3.0f * g_Math.FastSin(448.0f);
			m_CollisionBox[7].y = m_Position.y + 3.0f * g_Math.FastCos(448.0f);
			m_CollisionBox[7].z = m_Position.z;

			// collision happened, need to move along the wall
			float distance = 0.0f, shortest = std::numeric_limits<float>::infinity();
			glm::vec3 tmp_nrm;
			for (auto &vec : m_CollisionBox) {
				if (zone->zonemap->DoCollisionCheck(vec, vec + m_Delta, tmp_nrm, distance)) {
					bPassed = false; // lets try with new projection next pass
					if (distance < shortest) {
						normal = tmp_nrm;
						shortest = distance;
					}
				}
			}
		}

		if (bPassed) {
			ForcedMovement = 0;
			Teleport(m_Position + m_Delta);
			m_Delta = glm::vec4();
			SentPositionPacket(0.0f, 0.0f, 0.0f, 0.0f, 0, true);
			FixZ(); // so we teleport to the ground locally, we want the client to interpolate falling etc
		} else if (--ForcedMovement) {
			if (normal.z < -0.15f) // prevent too much wall climbing. ex. OMM's room in anguish
				normal.z = 0.0f;
			auto proj = glm::proj(static_cast<glm::vec3>(m_Delta), normal);
			m_Delta.x -= proj.x;
			m_Delta.y -= proj.y;
			m_Delta.z -= proj.z;
		} else {
			m_Delta = glm::vec4(); // well, we failed to find a spot to be forced to, lets give up
		}
	}
}

void Mob::AI_Process() {
	if (!IsAIControlled())
		return;

	if (!(AI_think_timer->Check() || attack_timer.Check(false)))
		return;


	if (IsCasting())
		return;

	bool engaged  = IsEngaged();
	bool doranged = false;

	if (!zone->CanDoCombat() || IsPetStop() || IsPetRegroup()) {
		engaged = false;
	}

	if (moving && CanOpenDoors()) {
		if (AI_scan_door_open_timer->Check()) {
			HandleDoorOpen();
		}
	}

	// Begin: Additions for Wiz Fear Code
	//
	if (RuleB(Combat, EnableFearPathing)) {
		if (currently_fleeing) {
			if ((IsRooted() || (IsBlind() && CombatRange(hate_list.GetClosestEntOnHateList(this)))) && !IsPetStop() &&
				!IsPetRegroup()) {
				//make sure everybody knows were not moving, for appearance sake
				if (IsMoving()) {
					FaceTarget();
					StopNavigation();
					moved = false;
				}
				//continue on to attack code, ensuring that we execute the engaged code
				engaged = true;
			}
			else {
				if (AI_movement_timer->Check()) {
					// Check if we have reached the last fear point
					if (DistanceNoZ(glm::vec3(GetX(), GetY(), GetZ()), m_FearWalkTarget) <= 5.0f) {
						// Calculate a new point to run to
						StopNavigation();
						CalculateNewFearpoint();
					}
					RunTo(
						m_FearWalkTarget.x,
						m_FearWalkTarget.y,
						m_FearWalkTarget.z
					);
				}
				return;
			}
		}
	}

	// trigger EVENT_SIGNAL if required
	if (AI_check_signal_timer->Check() && IsNPC()) {
		CastToNPC()->CheckSignal();
	}

	if (engaged) {
		if (IsNPC() && m_z_clip_check_timer.Check()) {
			bool is_moving = IsMoving() && !(IsRooted() || IsStunned() || IsMezzed());
			auto t         = GetTarget();
			if (is_moving && t) {
				float self_z            = GetZ() - GetZOffset();
				float target_z          = t->GetPosition().z - t->GetZOffset();
				bool  can_path_to       = CastToNPC()->CanPathTo(t->GetX(), t->GetY(), t->GetZ());
				bool  within_distance   = DistanceNoZ(GetPosition(), t->GetPosition()) < 75;
				bool  within_z_distance = std::abs(self_z - target_z) >= 25;
				if (within_distance && within_z_distance && !can_path_to) {
					float new_z = FindDestGroundZ(t->GetPosition());
					GMMove(t->GetPosition().x, t->GetPosition().y, new_z + GetZOffset(), t->GetPosition().w, false);
					FaceTarget(t);
				}
			}
		}

		if (!(GetPlayerState() & static_cast<uint32>(PlayerState::Aggressive)))
			SendAddPlayerState(PlayerState::Aggressive);

		// NPCs will forget people after 10 mins of not interacting with them or out of range
		// both of these maybe zone specific, hardcoded for now
		if (hate_list_cleanup_timer.Check()) {
			hate_list.RemoveStaleEntries(600000, static_cast<float>(zone->newzone_data.npc_aggro_max_dist));
			if (hate_list.IsHateListEmpty()) {
				AI_Event_NoLongerEngaged();
				zone->DelAggroMob();
				if (IsNPC() && !RuleB(Aggro, AllowTickPulling))
					ResetAssistCap();
			}
		}
		// we are prevented from getting here if we are blind and don't have a target in range
		// from above, so no extra blind checks needed
		if ((IsRooted() && !GetSpecialAbility(SpecialAbility::IgnoreRootAggroRules)) || IsBlind())
			SetTarget(hate_list.GetClosestEntOnHateList(this));
		else {
			if (AI_target_check_timer->Check()) {
				if (
					IsNPC() &&
					!CastToNPC()->GetSwarmInfo() &&
					(!IsPet() || (HasOwner() && GetOwner()->IsNPC())) &&
					!CastToNPC()->GetNPCAggro()
				) {
					WipeHateList(true); // wipe NPCs from hate list to prevent faction war
				}

				if (IsFocused()) {
					if (!target) {
						SetTarget(hate_list.GetMobWithMostHateOnList(this));
					}
				}
				else {
					if (!ImprovedTaunt())
						SetTarget(hate_list.GetMobWithMostHateOnList(this));
				}

			}
		}

		if (!target)
			return;

		if (target->IsCorpse()) {
			RemoveFromHateList(this);
			RemoveFromRampageList(this);
			return;
		}

		if (target->IsMezzed() && IsPet()) {

			auto pet_owner = GetOwner();
			if (pet_owner && pet_owner->IsClient()) {
				pet_owner->MessageString(Chat::NPCQuestSay, CANNOT_WAKE, GetCleanName(), target->GetCleanName());
			}

			RemoveFromHateList(target);
			return;
		}

		if (IsPet() && GetOwner() && GetOwner()->IsBot() && target == GetOwner())
		{
			// this blocks all pet attacks against owner..bot pet test (copied above check)
			RemoveFromHateList(this);
			return;
		}

		if (DivineAura())
			return;

		ProjectileAttack();

		if (shield_timer.Check()) {
			ShieldAbilityFinish();
		}

		auto npcSpawnPoint = CastToNPC()->GetSpawnPoint();
		if (GetSpecialAbility(SpecialAbility::Tether)) {
			float tether_range = static_cast<float>(GetSpecialAbilityParam(SpecialAbility::Tether, 0));
			tether_range = tether_range > 0.0f ? tether_range * tether_range : pAggroRange * pAggroRange;

			if (DistanceSquaredNoZ(m_Position, npcSpawnPoint) > tether_range) {
				GMMove(npcSpawnPoint.x, npcSpawnPoint.y, npcSpawnPoint.z, npcSpawnPoint.w);
			}
		}
		else if (GetSpecialAbility(SpecialAbility::Leash)) {
			float leash_range = static_cast<float>(GetSpecialAbilityParam(SpecialAbility::Leash, 0));
			leash_range = leash_range > 0.0f ? leash_range * leash_range : pAggroRange * pAggroRange;

			if (DistanceSquaredNoZ(m_Position, npcSpawnPoint) > leash_range) {
				GMMove(npcSpawnPoint.x, npcSpawnPoint.y, npcSpawnPoint.z, npcSpawnPoint.w);
				RestoreHealth();
				BuffFadeAll();
				WipeHateList();
				return;
			}
		}

		StartEnrage();

		bool is_combat_range = CombatRange(target);

		if (is_combat_range) {
			if (IsMoving()) {
				StopNavigation();
			}

			FaceTarget();

			//casting checked above...
			if (target && !IsStunned() && !IsMezzed() && GetAppearance() != eaDead && !IsMeleeDisabled()) {

				//we should check to see if they die mid-attacks, previous
				//crap of checking target for null was not gunna cut it

				//try main hand first
				if (attack_timer.Check()) {
					DoMainHandAttackRounds(target);
					TriggerDefensiveProcs(target, EQ::invslot::slotPrimary, false);

					bool specialed = false; // NPCs can only do one of these a round
					if (GetSpecialAbility(SpecialAbility::Flurry)) {
						int flurry_chance = GetSpecialAbilityParam(SpecialAbility::Flurry, 0);
						flurry_chance = flurry_chance > 0 ? flurry_chance : RuleI(Combat, NPCFlurryChance);

						if (zone->random.Roll(flurry_chance)) {
							ExtraAttackOptions opts;
							int                cur = GetSpecialAbilityParam(SpecialAbility::Flurry, 2);
							if (cur > 0)
								opts.damage_percent = cur / 100.0f;

							cur = GetSpecialAbilityParam(SpecialAbility::Flurry, 3);
							if (cur > 0)
								opts.damage_flat = cur;

							cur = GetSpecialAbilityParam(SpecialAbility::Flurry, 4);
							if (cur > 0)
								opts.armor_pen_percent = cur / 100.0f;

							cur = GetSpecialAbilityParam(SpecialAbility::Flurry, 5);
							if (cur > 0)
								opts.armor_pen_flat = cur;

							cur = GetSpecialAbilityParam(SpecialAbility::Flurry, 6);
							if (cur > 0)
								opts.crit_percent = cur / 100.0f;

							cur = GetSpecialAbilityParam(SpecialAbility::Flurry, 7);
							if (cur > 0)
								opts.crit_flat = cur;

							Flurry(&opts);
							specialed = true;
						}
					}

					if (IsPet() || IsTempPet()) {
						Mob *owner = nullptr;
						owner = GetOwner();

						if (owner) {
							int16 flurry_chance = owner->aabonuses.PetFlurry +
												  owner->spellbonuses.PetFlurry + owner->itembonuses.PetFlurry;

							if (flurry_chance && zone->random.Roll(flurry_chance))
								Flurry(nullptr);
						}
					}


					//SE_PC_Pet_Rampage SPA 464 on pet, chance modifier
					if ((IsPet() || IsTempPet()) && IsPetOwnerOfClientBot()) {
						int chance = spellbonuses.PC_Pet_Rampage[SBIndex::PET_RAMPAGE_CHANCE] + itembonuses.PC_Pet_Rampage[SBIndex::PET_RAMPAGE_CHANCE] + aabonuses.PC_Pet_Rampage[SBIndex::PET_RAMPAGE_CHANCE];
						if (chance && zone->random.Roll(chance)) {
							Rampage(nullptr);
						}
					}


					if (GetSpecialAbility(SpecialAbility::Rampage) && !specialed) {
						int rampage_chance = GetSpecialAbilityParam(SpecialAbility::Rampage, 0);
						rampage_chance = rampage_chance > 0 ? rampage_chance : 20;
						if (zone->random.Roll(rampage_chance)) {
							ExtraAttackOptions opts;
							int                cur = GetSpecialAbilityParam(SpecialAbility::Rampage, 3);
							if (cur > 0) {
								opts.damage_flat = cur;
							}

							cur = GetSpecialAbilityParam(SpecialAbility::Rampage, 4);
							if (cur > 0) {
								opts.armor_pen_percent = cur / 100.0f;
							}

							cur = GetSpecialAbilityParam(SpecialAbility::Rampage, 5);
							if (cur > 0) {
								opts.armor_pen_flat = cur;
							}

							cur = GetSpecialAbilityParam(SpecialAbility::Rampage, 6);
							if (cur > 0) {
								opts.crit_percent = cur / 100.0f;
							}

							cur = GetSpecialAbilityParam(SpecialAbility::Rampage, 7);
							if (cur > 0) {
								opts.crit_flat = cur;
							}
							Rampage(&opts);
							specialed = true;
						}
					}

					//SE_PC_Pet_Rampage SPA 465 on pet, chance modifier
					if ((IsPet() || IsTempPet()) && IsPetOwnerOfClientBot()) {
						int chance = spellbonuses.PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_CHANCE] + itembonuses.PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_CHANCE] + aabonuses.PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_CHANCE];
						if (chance && zone->random.Roll(chance)) {
							Rampage(nullptr);
						}
					}

					if (GetSpecialAbility(SpecialAbility::AreaRampage) && !specialed) {
						int rampage_chance = GetSpecialAbilityParam(SpecialAbility::AreaRampage, 0);
						rampage_chance = rampage_chance > 0 ? rampage_chance : 20;
						if (zone->random.Roll(rampage_chance)) {
							ExtraAttackOptions opts;
							int                cur = GetSpecialAbilityParam(SpecialAbility::AreaRampage, 3);
							if (cur > 0) {
								opts.damage_flat = cur;
							}

							cur = GetSpecialAbilityParam(SpecialAbility::AreaRampage, 4);
							if (cur > 0) {
								opts.armor_pen_percent = cur / 100.0f;
							}

							cur = GetSpecialAbilityParam(SpecialAbility::AreaRampage, 5);
							if (cur > 0) {
								opts.armor_pen_flat = cur;
							}

							cur = GetSpecialAbilityParam(SpecialAbility::AreaRampage, 6);
							if (cur > 0) {
								opts.crit_percent = cur / 100.0f;
							}

							cur = GetSpecialAbilityParam(SpecialAbility::AreaRampage, 7);
							if (cur > 0) {
								opts.crit_flat = cur;
							}

							cur = GetSpecialAbilityParam(SpecialAbility::AreaRampage, 8);
							if (cur > 0) {
								opts.range_percent = cur;
							}

							AreaRampage(&opts);
							specialed = true;
						}
					}
				}

				//now off hand
				if (attack_dw_timer.Check() && CanThisClassDualWield())
					DoOffHandAttackRounds(target);

				//now special attacks (kick, etc)
				if (IsNPC())
					CastToNPC()->DoClassAttacks(target);

			}
			AI_EngagedCastCheck();

		}    //end is within combat rangepet
		else {

			// See if we can summon the mob to us
			if (!HateSummon()) {

				//could not summon them, check ranged...
				if (GetSpecialAbility(SpecialAbility::RangedAttack) || HasBowAndArrowEquipped()) {
					doranged = true;
				}

				// Now pursue
				// TODO: Check here for another person on hate list with close hate value
				if (AI_PursueCastCheck()) {
					if (IsCasting() && GetClass() != Class::Bard) {
						StopNavigation();
						FaceTarget();
					}
				}
				// mob/npc waits until call for help complete, others can move
				else if (AI_movement_timer->Check() && target &&
						(GetOwnerID() || IsBot() || IsTempPet() ||
						CastToNPC()->GetCombatEvent())) {
					if (!IsRooted()) {
						LogAIDetail("Pursuing [{}] while engaged", target->GetName());
						RunTo(target->GetX(), target->GetY(), target->GetZ());

					}
					else {
						FaceTarget();
					}
				}
			}
		}
	}
	else {
		if (GetPlayerState() & static_cast<uint32>(PlayerState::Aggressive))
			SendRemovePlayerState(PlayerState::Aggressive);

		if (IsPetStop()) // pet stop won't be engaged, so we will always get here and we want the above branch to execute
			return;

		if (zone->CanDoCombat() && AI_feign_remember_timer->Check()) {
			// 6/14/06
			// Improved Feign Death Memory
			// check to see if any of our previous feigned targets have gotten up.
			std::set<uint32>::iterator remembered_feigned_mobid;
			remembered_feigned_mobid = feign_memory_list.begin();
			while (remembered_feigned_mobid != feign_memory_list.end()) {
				Mob *remembered_mob = entity_list.GetMob(*remembered_feigned_mobid);
				if (remembered_mob == nullptr || remembered_mob->IsCorpse()) {
					//they are gone now...
					remembered_feigned_mobid = feign_memory_list.erase(remembered_feigned_mobid);
				}
				else if (!remembered_mob->GetFeigned()) {
					AddToHateList(remembered_mob, 1);
					remembered_feigned_mobid = feign_memory_list.erase(remembered_feigned_mobid);
					break;
				}
				else {
					//they are still feigned, carry on...
					++remembered_feigned_mobid;
				}
			}
		}
		if (AI_IdleCastCheck()) {
			if (IsCasting() && GetClass() != Class::Bard) {
				StopNavigation();
			}
		}
		else if (zone->CanDoCombat() && IsNPC() && CastToNPC()->GetNPCAggro() && AI_scan_area_timer->Check()) {
			CastToNPC()->DoNpcToNpcAggroScan();
		}
		else if (AI_movement_timer->Check() && !IsRooted()) {
			if (IsPet()) {
				// we're a pet, do as we're told
				switch (m_pet_order) {
					case PetOrder::Follow: {

						Mob *owner = GetOwner();
						if (owner == nullptr) {
							break;
						}

						glm::vec4 pet_owner_position = owner->GetPosition();
						float     distance_to_owner  = DistanceSquared(m_Position, pet_owner_position);
						float     z_distance         = pet_owner_position.z - m_Position.z;

						if (distance_to_owner >= 400 || z_distance > 100) {

							bool running = false;

							/**
							 * Distance: >= 35 (Run if far away)
							 */
							if (distance_to_owner >= 1225) {
								running = true;
							}

							/**
							 * Distance: >= 450 (Snap to owner)
							 */
							if (distance_to_owner >= 202500 || z_distance > 100) {
								if (running) {
									RunTo(pet_owner_position.x, pet_owner_position.y, pet_owner_position.z);
								}
								else {
									WalkTo(pet_owner_position.x, pet_owner_position.y, pet_owner_position.z);
								}
							}
							else {

								if (running) {
									RunTo(pet_owner_position.x, pet_owner_position.y, pet_owner_position.z);
								}
								else {
									WalkTo(pet_owner_position.x, pet_owner_position.y, pet_owner_position.z);
								}
							}
						}
						else {
							StopNavigation();
						}

						break;
					}
					case PetOrder::Sit: {
						SetAppearance(eaSitting, false);
						break;
					}
					case PetOrder::Guard: {
						//only NPCs can guard stuff. (forced by where the guard movement code is in the AI)
						if (IsNPC()) {
							CastToNPC()->NextGuardPosition();
						}
						break;
					}
					case PetOrder::Feign: {
						SetAppearance(eaDead, false);
						break;
					}
				}
				if (IsPetRegroup()) {
					return;
				}
			}
				/* Entity has been assigned another entity to follow */
			else if (GetFollowID()) {
				Mob *follow = entity_list.GetMob(static_cast<uint16>(GetFollowID()));
				if (!follow) {
					SetFollowID(0);
					SetFollowDistance(100);
					SetFollowCanRun(true);
				}
				else {

					float distance        = DistanceSquared(m_Position, follow->GetPosition());
					int   follow_distance = GetFollowDistance();

					/**
					 * Default follow distance is 100
					 */
					if (distance >= follow_distance) {
						bool running = false;
						// maybe we want the NPC to only walk doing follow logic
						if (GetFollowCanRun() && distance >= follow_distance + 150) {
							running = true;
						}

						auto &Goal = follow->GetPosition();

						if (running) {
							RunTo(Goal.x, Goal.y, Goal.z);
						}
						else {
							WalkTo(Goal.x, Goal.y, Goal.z);
						}
					}
					else {
						moved = false;
						StopNavigation();
					}
				}
			}
			else //not a pet, and not following somebody...
			{
				// dont move till a bit after you last fought
				if (time_until_can_move < Timer::GetCurrentTime()) {
					if (IsClient()) {

						/**
						 * LD timer expired, drop out of world
						 */
						if (CastToClient()->IsLD()) {
							CastToClient()->Disconnect();
						}

						return;
					}

					if (IsNPC()) {
						if (RuleB(NPC, SmartLastFightingDelayMoving) && !feign_memory_list.empty()) {
							minLastFightingDelayMoving = 0;
							maxLastFightingDelayMoving = 0;
						}
						/* All normal NPC pathing */
						CastToNPC()->AI_DoMovement();
					}
				}
			}
		}
	}

	if (forget_timer.Check()) {
		forget_timer.Disable();
		entity_list.ClearZoneFeignAggro(this);
	}

	//Do Ranged attack here
	if (doranged) {
		RangedAttack(target);
	}
}

void NPC::AI_DoMovement() {

	float move_speed = GetMovespeed();

	if (move_speed <= 0.0f) {
		return;
	}

	// Roambox logic sets precedence
	if (m_roambox.distance > 0) {
		HandleRoambox();
		return;
	}
	else if (roamer) {
		if (AI_walking_timer->Check()) {
			pause_timer_complete = true;
			AI_walking_timer->Disable();
		}

		int32 gridno = CastToNPC()->GetGrid();

		if (gridno > 0 || cur_wp == EQ::WaypointStatus::QuestControlNoGrid) {
			if (pause_timer_complete == true) { // time to pause at wp is over
				AI_SetupNextWaypoint();
			}    // endif (pause_timer_complete==true)
			else if (!(AI_walking_timer->Enabled())) {    // currently moving
				bool doMove = true;
				if(IsPositionEqual(glm::vec2(m_CurrentWayPoint.x, m_CurrentWayPoint.y), glm::vec2(GetX(), GetY()))) {
					LogAIDetail("We have reached waypoint [{}] ({},{},{}) on grid [{}]",
						cur_wp,
						GetX(),
						GetY(),
						GetZ(),
						GetGrid());

					if (wandertype == GridRandomPath)
					{
						if (cur_wp == patrol)
						{
							// reached our randomly selected destination; force a pause
							if (cur_wp_pause == 0)
							{
								if (Waypoints.size() >= cur_wp && Waypoints[cur_wp].pause)
									cur_wp_pause = Waypoints[cur_wp].pause;
								else if (Waypoints.size() > 0 && Waypoints[0].pause)
									cur_wp_pause = Waypoints[0].pause;
								else
									cur_wp_pause = 38;
							}
							Log(Logs::Detail, Logs::AI, "NPC using wander type GridRandomPath on grid %d at waypoint %d has reached its random destination; pause time is %d", GetGrid(), cur_wp, cur_wp_pause);
						}
						else
							cur_wp_pause = 0; // skipping pauses until destination
					}

					SetWaypointPause();
					if (GetAppearance() != eaStanding) {
						SetAppearance(eaStanding, false);
					}
					if (cur_wp_pause > 0 && m_CurrentWayPoint.w >= 0.0) {
						RotateTo(m_CurrentWayPoint.w);
					}

					if (parse->HasQuestSub(GetNPCTypeID(), EVENT_WAYPOINT_ARRIVE)) {
						parse->EventNPC(EVENT_WAYPOINT_ARRIVE, CastToNPC(), nullptr, std::to_string(cur_wp), 0);
					}

					// No need to move as we are there.  Next loop will
					// take care of normal grids, even at pause 0.
					// We do need to call and setup a wp if we're cur_wp=-2
					// as that is where roamer is unset and we don't want
					// the next trip through to move again based on grid stuff.
					doMove = false;
					if (cur_wp == EQ::WaypointStatus::QuestControlNoGrid) {
						AI_SetupNextWaypoint();
					}

					// wipe feign memory since we reached our first waypoint
					if (cur_wp == 1)
						ClearFeignMemory();

					if (cur_wp_pause == 0) {
						pause_timer_complete = true;
						AI_SetupNextWaypoint();
						doMove = true;
					}
				}

				if (doMove) {    // not at waypoint yet or at 0 pause WP, so keep moving
					NavigateTo(
						m_CurrentWayPoint.x,
						m_CurrentWayPoint.y,
						m_CurrentWayPoint.z
					);

				}
			}
		}        // endif (gridno > 0)
			// handle new quest grid command processing
		else if (gridno < 0) {    // this mob is under quest control
			if (pause_timer_complete == true) { // time to pause has ended
				SetGrid(0 - GetGrid()); // revert to AI control
				LogPathing("Quest pathing is finished. Resuming on grid [{}]", GetGrid());

				SetAppearance(eaStanding, false);

				CalculateNewWaypoint();
			}
		}

	}
	else if (IsGuarding()) {
		bool at_gp = IsPositionEqualWithinCertainZ(m_Position, m_GuardPoint, 15.0f);

		if (at_gp) {

			if (moved) {
				LogAIDetail("Reached guard point ({},{},{})", m_GuardPoint.x, m_GuardPoint.y, m_GuardPoint.z);

				ClearFeignMemory();
				moved = false;
				if (GetTarget() == nullptr || DistanceSquared(m_Position, GetTarget()->GetPosition()) >= 5 * 5) {
					RotateTo(m_GuardPoint.w);
				}
				else {
					FaceTarget(GetTarget());
				}
				SetAppearance(GetGuardPointAnim());
			}
		}
		else {
			NavigateTo(m_GuardPoint.x, m_GuardPoint.y, m_GuardPoint.z);
		}
	}
}

void NPC::AI_SetupNextWaypoint() {
	int32 spawn_id = GetSpawnPointID();
	LinkedListIterator<Spawn2*> iterator(zone->spawn2_list);
	iterator.Reset();
	Spawn2 *found_spawn = nullptr;

	while (iterator.MoreElements())
	{
		Spawn2* cur = iterator.GetData();
		iterator.Advance();
		if (cur->GetID() == spawn_id)
		{
			found_spawn = cur;
			break;
		}
	}

	if (wandertype == GridOneWayRepop && cur_wp == CastToNPC()->GetMaxWp()) {
		CastToNPC()->Depop(true); //depop and restart spawn timer
		if (found_spawn)
			found_spawn->SetNPCPointerNull();
	}
	else if (wandertype == GridOneWayDepop && cur_wp == CastToNPC()->GetMaxWp()) {
		CastToNPC()->Depop(false);//depop without spawn timer
		if (found_spawn)
			found_spawn->SetNPCPointerNull();
	}
	else {
		pause_timer_complete = false;
		LogPathingDetail(
			"[{}] departing waypoint [{}]",
			GetCleanName(),
			cur_wp
		);
		//if we were under quest control (with no grid), we are done now..
		if (cur_wp == EQ::WaypointStatus::QuestControlNoGrid) {
			LogPathing("Non-grid quest mob has reached its quest ordered waypoint. Leaving pathing mode");
			roamer = false;
			cur_wp = 0;
		}

		SetAppearance(eaStanding, false);

		entity_list.OpenDoorsNear(this);

		if (!DistractedFromGrid) {
			if (parse->HasQuestSub(GetNPCTypeID(), EVENT_WAYPOINT_DEPART)) {
				parse->EventNPC(EVENT_WAYPOINT_DEPART, CastToNPC(), nullptr, std::to_string(cur_wp), 0);
			}

			//setup our next waypoint, if we are still on our normal grid
			//remember that the quest event above could have done anything it wanted with our grid
			if (GetGrid() > 0) {
				CastToNPC()->CalculateNewWaypoint();
			}
		}
		else {
			DistractedFromGrid = false;
		}
	}
}

/**
 * @param attacker
 * @param yell_for_help
 */
void Mob::AI_Event_Engaged(Mob *attacker, bool yell_for_help)
{
	if (!IsAIControlled()) {
		return;
	}

	SetAppearance(eaStanding);

	parse->EventBotMerc(EVENT_COMBAT, this, attacker, [&] { return "1"; });

	if (IsNPC()) {
		CastToNPC()->AIautocastspell_timer->Start(300, false);

		if (yell_for_help) {
			if (IsPet()) {
				GetOwner()->AI_Event_Engaged(attacker, yell_for_help);
			}
			else if (!HasAssistAggro() && NPCAssistCap() < RuleI(Combat, NPCAssistCap)) {
				CastToNPC()->AIYellForHelp(this, attacker);
				if (NPCAssistCap() > 0 && !assist_cap_timer.Enabled()) {
					assist_cap_timer.Start(RuleI(Combat, NPCAssistCapTimer));
				}
			}
		}

		if (CastToNPC()->GetGrid() > 0) {
			DistractedFromGrid = true;
		}
		if (attacker && !attacker->IsCorpse()) {
			//Because sometimes the AIYellForHelp triggers another engaged and then immediately a not engaged
			//if the target dies before it goes off
			if (attacker->GetHP() > 0) {
				if (!CastToNPC()->GetCombatEvent() && GetHP() > 0) {
					if (parse->HasQuestSub(GetNPCTypeID(), EVENT_COMBAT)) {
						parse->EventNPC(EVENT_COMBAT, CastToNPC(), attacker, "1", 0);
					}

					if (emoteid) {
						CastToNPC()->DoNPCEmote(EQ::constants::EmoteEventTypes::EnterCombat, emoteid, attacker);
					}

					std::string mob_name = GetCleanName();
					m_combat_record.Start(mob_name);
					CastToNPC()->SetCombatEvent(true);
				}
			}
		}
	}
}

// Note: Hate list may not be actually clear until after this function call completes
void Mob::AI_Event_NoLongerEngaged() {
	if (!IsAIControlled()) {
		return;
	}

	AI_walking_timer->Start(RandomTimer(3000,20000));
	time_until_can_move = Timer::GetCurrentTime();

	if (minLastFightingDelayMoving == maxLastFightingDelayMoving) {
		time_until_can_move += minLastFightingDelayMoving;
	} else {
		time_until_can_move += zone->random.Int(minLastFightingDelayMoving, maxLastFightingDelayMoving);
	}

	StopNavigation();
	ClearRampage();

	if (IsNPC()) {
		SetPrimaryAggro(false);
		SetAssistAggro(false);
		if (
			CastToNPC()->GetCombatEvent() &&
			GetHP() > 0 &&
			entity_list.GetNPCByID(GetID())
		) {
			if (parse->HasQuestSub(GetNPCTypeID(), EVENT_COMBAT)) {
				parse->EventNPC(EVENT_COMBAT, CastToNPC(), nullptr, "0", 0);
			}

			const uint32 emote_id = CastToNPC()->GetEmoteID();
			if (emote_id) {
				CastToNPC()->DoNPCEmote(EQ::constants::EmoteEventTypes::LeaveCombat, emote_id);
			}

			m_combat_record.Stop();
			CastToNPC()->SetCombatEvent(false);
		}
	} else {
		parse->EventBotMerc(EVENT_COMBAT, this, nullptr, [&]() { return "0"; });
	}
}

//this gets called from InterruptSpell() for failure or SpellFinished() for success
void NPC::AI_Event_SpellCastFinished(bool iCastSucceeded, uint16 slot) {
	if (slot == 1) {
		uint32 recovery_time = 0;
		if (iCastSucceeded) {
			if (casting_spell_AIindex < AIspells.size()) {
					recovery_time += spells[AIspells[casting_spell_AIindex].spellid].recovery_time;
					if (AIspells[casting_spell_AIindex].recast_delay >= 0)
					{
						if (AIspells[casting_spell_AIindex].recast_delay < 10000)
							AIspells[casting_spell_AIindex].time_cancast = Timer::GetCurrentTime() + (AIspells[casting_spell_AIindex].recast_delay*1000);
					}
					else
						AIspells[casting_spell_AIindex].time_cancast = Timer::GetCurrentTime() + spells[AIspells[casting_spell_AIindex].spellid].recast_time;
			}
			if (recovery_time < AIautocastspell_timer->GetSetAtTrigger())
				recovery_time = AIautocastspell_timer->GetSetAtTrigger();
			AIautocastspell_timer->Start(recovery_time, false);
		}
		else
			AIautocastspell_timer->Start(AISpellVar.fail_recast, false);
		casting_spell_AIindex = AIspells.size();
	}
}


bool NPC::AI_EngagedCastCheck() {
	if (AIautocastspell_timer->Check(false)) {
		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.

		LogAIDetail("Engaged autocast check triggered. Trying to cast healing spells then maybe offensive spells");

		// first try innate (spam) spells
		if(!AICastSpell(GetTarget(), 0, SpellType_Nuke | SpellType_Lifetap | SpellType_DOT | SpellType_Dispel | SpellType_Mez | SpellType_Slow | SpellType_Debuff | SpellType_Charm | SpellType_Root, true)) {
			// try innate (spam) self targeted spells
			if (!AICastSpell(this, 0, SpellType_InCombatBuff, true)) {
				// try casting a heal or gate
				if (!AICastSpell(this, AISpellVar.engaged_beneficial_self_chance, SpellType_Heal | SpellType_Escape | SpellType_InCombatBuff)) {
					// try casting a heal on nearby
					if (!AICheckCloseBeneficialSpells(this, AISpellVar.engaged_beneficial_other_chance, MobAISpellRange, SpellType_Heal)) {
						//nobody to heal, try some detrimental spells.
						if(!AICastSpell(GetTarget(), AISpellVar.engaged_detrimental_chance, SpellType_Nuke | SpellType_Lifetap | SpellType_DOT | SpellType_Dispel | SpellType_Mez | SpellType_Slow | SpellType_Debuff | SpellType_Charm | SpellType_Root)) {
							//no spell to cast, try again soon.
							AIautocastspell_timer->Start(RandomTimer(AISpellVar.engaged_no_sp_recast_min, AISpellVar.engaged_no_sp_recast_max), false);
						}
					}
				}
			}
		}
		return(true);
	}

	return(false);
}

bool NPC::AI_PursueCastCheck() {
	if (AIautocastspell_timer->Check(false)) {
		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.

		LogAIDetail("Engaged (pursuing) autocast check triggered. Trying to cast offensive spells");
		// checking innate (spam) spells first
		if(!AICastSpell(GetTarget(), AISpellVar.pursue_detrimental_chance, SpellType_Root | SpellType_Nuke | SpellType_Lifetap | SpellType_Snare | SpellType_DOT | SpellType_Dispel | SpellType_Mez | SpellType_Slow | SpellType_Debuff, true)) {
			if(!AICastSpell(GetTarget(), AISpellVar.pursue_detrimental_chance, SpellType_Root | SpellType_Nuke | SpellType_Lifetap | SpellType_Snare | SpellType_DOT | SpellType_Dispel | SpellType_Mez | SpellType_Slow | SpellType_Debuff)) {
				//no spell cast, try again soon.
				AIautocastspell_timer->Start(RandomTimer(AISpellVar.pursue_no_sp_recast_min, AISpellVar.pursue_no_sp_recast_max), false);
			} //else, spell casting finishing will reset the timer.
		}
		return(true);
	}
	return(false);
}

bool NPC::AI_IdleCastCheck() {
	if (AIautocastspell_timer->Check(false)) {
		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.
		if (!AICastSpell(this, AISpellVar.idle_beneficial_chance, SpellType_Heal | SpellType_Buff | SpellType_Pet)) {
			if(!AICheckCloseBeneficialSpells(this, 33, MobAISpellRange, SpellType_Heal | SpellType_Buff)) {
				//if we didnt cast any spells, our autocast timer just resets to the
				//last duration it was set to... try to put up a more reasonable timer...
				AIautocastspell_timer->Start(RandomTimer(AISpellVar.idle_no_sp_recast_min, AISpellVar.idle_no_sp_recast_max), false);

				LogSpells("Mob [{}] Min [{}] Max [{}]", GetCleanName(), AISpellVar.idle_no_sp_recast_min, AISpellVar.idle_no_sp_recast_max);

			}	//else, spell casting finishing will reset the timer.
		}	//else, spell casting finishing will reset the timer.
		return(true);
	}
	return(false);
}

void Mob::StartEnrage()
{
	// dont continue if already enraged
	if (bEnraged)
		return;

	if(!GetSpecialAbility(SpecialAbility::Enrage))
		return;

	int hp_ratio = GetSpecialAbilityParam(SpecialAbility::Enrage, 0);
	hp_ratio = hp_ratio > 0 ? hp_ratio : RuleI(NPC, StartEnrageValue);
	if(GetHPRatio() > static_cast<float>(hp_ratio)) {
		return;
	}

	if(RuleB(NPC, LiveLikeEnrage) && !((IsPet() && !IsCharmed() && GetOwner() && GetOwner()->IsOfClientBot()) ||
		(CastToNPC()->GetSwarmOwner() && entity_list.GetMob(CastToNPC()->GetSwarmOwner())->IsOfClientBot()))) {
		return;
	}

	Timer *timer = GetSpecialAbilityTimer(SpecialAbility::Enrage);
	if (timer && !timer->Check())
		return;

	int enraged_duration = GetSpecialAbilityParam(SpecialAbility::Enrage, 1);
	enraged_duration = enraged_duration > 0 ? enraged_duration : EnragedDurationTimer;
	StartSpecialAbilityTimer(SpecialAbility::Enrage, enraged_duration);

	// start the timer. need to call IsEnraged frequently since we dont have callback timers :-/
	bEnraged = true;
	entity_list.MessageCloseString(this, true, 200, Chat::NPCEnrage, NPC_ENRAGE_START, GetCleanName());
}

void Mob::ProcessEnrage(){
	if(IsEnraged()){
		Timer *timer = GetSpecialAbilityTimer(SpecialAbility::Enrage);
		if(timer && timer->Check()){
			entity_list.MessageCloseString(this, true, 200, Chat::NPCEnrage, NPC_ENRAGE_END, GetCleanName());

			int enraged_cooldown = GetSpecialAbilityParam(SpecialAbility::Enrage, 2);
			enraged_cooldown = enraged_cooldown > 0 ? enraged_cooldown : EnragedTimer;
			StartSpecialAbilityTimer(SpecialAbility::Enrage, enraged_cooldown);
			bEnraged = false;
		}
	}
}

bool Mob::IsEnraged()
{
	return bEnraged;
}

bool Mob::Flurry(ExtraAttackOptions *opts)
{
	// this is wrong, flurry is extra attacks on the current target
	Mob *target = GetTarget();
	if (target) {
		if (IsPet() || IsTempPet() || IsCharmed() || IsAnimation()) {
			entity_list.MessageCloseString(
				this,
				true,
				200,
				Chat::PetFlurry,
				NPC_FLURRY,
				GetCleanName(),
				target->GetCleanName());
		} else {
			entity_list.MessageCloseString(
				this,
				true,
				200,
				Chat::NPCFlurry,
				NPC_FLURRY,
				GetCleanName(),
				target->GetCleanName());
		}

		int num_attacks = GetSpecialAbilityParam(SpecialAbility::Flurry, 1);
		num_attacks = num_attacks > 0 ? num_attacks : RuleI(Combat, MaxFlurryHits);
		for (int i = 0; i < num_attacks; i++)
			Attack(target, EQ::invslot::slotPrimary, false, false, false, opts);
	}
	return true;
}

bool Mob::AddRampage(Mob *mob)
{
	if (!mob) {
		return false;
	}

	if (!GetSpecialAbility(SpecialAbility::Rampage)) {
		return false;
	}

	for (int i = 0; i < RampageArray.size(); i++) {
		// if Entity ID is already on the list don't add it again
		if (mob->GetID() == RampageArray[i]) {
			return false;
		}
	}
	RampageArray.push_back(mob->GetID());
	return true;
}

void Mob::ClearRampage()
{
	RampageArray.clear();
}

void Mob::RemoveFromRampageList(Mob* mob, bool remove_feigned)
{
	if (!mob) {
		return;
	}

	if (
		IsNPC() &&
		GetSpecialAbility(SpecialAbility::Rampage) &&
		(
			remove_feigned  ||
			mob->IsNPC() ||
			(
				mob->IsClient() &&
				!mob->CastToClient()->GetFeigned()
			)
		)
	) {
		for (int i = 0; i < RampageArray.size(); i++) {
			if (mob->GetID() == RampageArray[i]) {
				RampageArray[i] = 0;
			}
		}
	}
}

bool Mob::Rampage(ExtraAttackOptions *opts)
{
	int index_hit = 0;
	if (IsPet() || IsTempPet() || IsCharmed() || IsAnimation()){
		entity_list.MessageCloseString(this, true, 200, Chat::PetFlurry, NPC_RAMPAGE, GetCleanName());
	} else {
		entity_list.MessageCloseString(this, true, 200, Chat::NPCRampage, NPC_RAMPAGE, GetCleanName());
	}

	int rampage_targets = GetSpecialAbilityParam(SpecialAbility::Rampage, 1);

	if (rampage_targets == 0) { // if set to 0 or not set in the DB
		rampage_targets = RuleI(Combat, DefaultRampageTargets);
	}

	if (rampage_targets > RuleI(Combat, MaxRampageTargets)) {
		rampage_targets = RuleI(Combat, MaxRampageTargets);
	}

	m_specialattacks = eSpecialAttacks::Rampage;
	for (int i = 0; i < RampageArray.size(); i++) {
		if (index_hit >= rampage_targets) {
			break;
		}
		// range is important
		Mob *m_target = entity_list.GetMob(RampageArray[i]);
		if (m_target) {
			if (m_target == GetTarget()) {
				continue;
			}

			if (m_target->IsCorpse()) {
				LogAggroDetail("[{}] is on [{}]'s rampage list", m_target->GetCleanName(), GetCleanName());
				RemoveFromRampageList(m_target, true);
				continue;
			}

			if (DistanceSquaredNoZ(GetPosition(), m_target->GetPosition()) <= NPC_RAMPAGE_RANGE2) {
				ProcessAttackRounds(m_target, opts, true);
				index_hit++;
			}
		}
	}

	if (RuleB(Combat, RampageHitsTarget)) {
		if (index_hit < rampage_targets)
			ProcessAttackRounds(GetTarget(), opts, true);
	} else { // let's do correct behavior here, if they set above rule we can assume they want non-live like behavior
		if (index_hit < rampage_targets) {
			// so we go over in reverse order and skip range check
			// lets do it this way to still support non-live-like >1 rampage targets
			// likely live is just a fall through of the last valid mob
			for (auto i = RampageArray.crbegin(); i != RampageArray.crend(); ++i) {
				if (index_hit >= rampage_targets) {
					break;
				}
				auto m_target = entity_list.GetMob(*i);
				if (m_target) {
					if (m_target == GetTarget()) {
						continue;
					}
					ProcessAttackRounds(m_target, opts, true);
					index_hit++;
				}
			}
		}
	}

	m_specialattacks = eSpecialAttacks::None;

	return true;
}

void Mob::AreaRampage(ExtraAttackOptions *opts)
{
	int index_hit = 0;
	if (IsPet() || IsTempPet() || IsCharmed() || IsAnimation()) { // do not know every pet AA so thought it safer to add this
		entity_list.MessageCloseString(this, true, 200, Chat::PetFlurry, AE_RAMPAGE, GetCleanName());
	} else {
		entity_list.MessageCloseString(this, true, 200, Chat::NPCRampage, AE_RAMPAGE, GetCleanName());
	}

	int rampage_targets = GetSpecialAbilityParam(SpecialAbility::AreaRampage, 1);
	rampage_targets = rampage_targets > 0 ? rampage_targets : -1;
	m_specialattacks = eSpecialAttacks::AERampage;
	index_hit = hate_list.AreaRampage(this, GetTarget(), rampage_targets, opts);

	m_specialattacks = eSpecialAttacks::None;
}

uint32 Mob::GetLevelCon(uint8 mylevel, uint8 iOtherLevel) {

	uint32 conlevel = 0;

	if (RuleB(Character, UseOldConSystem))
	{
		int16 diff = iOtherLevel - mylevel;

		if (diff == 0)
			return ConsiderColor::White;
		else if (diff >= 1 && diff <= 2)
			return ConsiderColor::Yellow;
		else if (diff >= 3)
			return ConsiderColor::Red;

		if (mylevel <= 8)
		{
			if (diff <= -4)
				conlevel = ConsiderColor::Gray;
			else
				conlevel = ConsiderColor::DarkBlue;
		}
		else if (mylevel <= 9)
		{
			if (diff <= -6)
				conlevel = ConsiderColor::Gray;
			else if (diff <= -4)
				conlevel = ConsiderColor::LightBlue;
			else
				conlevel = ConsiderColor::DarkBlue;
		}
		else if (mylevel <= 13)
		{
			if (diff <= -7)
				conlevel = ConsiderColor::Gray;
			else if (diff <= -5)
				conlevel = ConsiderColor::LightBlue;
			else
				conlevel = ConsiderColor::DarkBlue;
		}
		else if (mylevel <= 15)
		{
			if (diff <= -7)
				conlevel = ConsiderColor::Gray;
			else if (diff <= -5)
				conlevel = ConsiderColor::LightBlue;
			else
				conlevel = ConsiderColor::DarkBlue;
		}
		else if (mylevel <= 17)
		{
			if (diff <= -8)
				conlevel = ConsiderColor::Gray;
			else if (diff <= -6)
				conlevel = ConsiderColor::LightBlue;
			else
				conlevel = ConsiderColor::DarkBlue;
		}
		else if (mylevel <= 21)
		{
			if (diff <= -9)
				conlevel = ConsiderColor::Gray;
			else if (diff <= -7)
				conlevel = ConsiderColor::LightBlue;
			else
				conlevel = ConsiderColor::DarkBlue;
		}
		else if (mylevel <= 25)
		{
			if (diff <= -10)
				conlevel = ConsiderColor::Gray;
			else if (diff <= -8)
				conlevel = ConsiderColor::LightBlue;
			else
				conlevel = ConsiderColor::DarkBlue;
		}
		else if (mylevel <= 29)
		{
			if (diff <= -11)
				conlevel = ConsiderColor::Gray;
			else if (diff <= -9)
				conlevel = ConsiderColor::LightBlue;
			else
				conlevel = ConsiderColor::DarkBlue;
		}
		else if (mylevel <= 31)
		{
			if (diff <= -12)
				conlevel = ConsiderColor::Gray;
			else if (diff <= -9)
				conlevel = ConsiderColor::LightBlue;
			else
				conlevel = ConsiderColor::DarkBlue;
		}
		else if (mylevel <= 33)
		{
			if (diff <= -13)
				conlevel = ConsiderColor::Gray;
			else if (diff <= -10)
				conlevel = ConsiderColor::LightBlue;
			else
				conlevel = ConsiderColor::DarkBlue;
		}
		else if (mylevel <= 37)
		{
			if (diff <= -14)
				conlevel = ConsiderColor::Gray;
			else if (diff <= -11)
				conlevel = ConsiderColor::LightBlue;
			else
				conlevel = ConsiderColor::DarkBlue;
		}
		else if (mylevel <= 41)
		{
			if (diff <= -16)
				conlevel = ConsiderColor::Gray;
			else if (diff <= -12)
				conlevel = ConsiderColor::LightBlue;
			else
				conlevel = ConsiderColor::DarkBlue;
		}
		else if (mylevel <= 45)
		{
			if (diff <= -17)
				conlevel = ConsiderColor::Gray;
			else if (diff <= -13)
				conlevel = ConsiderColor::LightBlue;
			else
				conlevel = ConsiderColor::DarkBlue;
		}
		else if (mylevel <= 49)
		{
			if (diff <= -18)
				conlevel = ConsiderColor::Gray;
			else if (diff <= -14)
				conlevel = ConsiderColor::LightBlue;
			else
				conlevel = ConsiderColor::DarkBlue;
		}
		else if (mylevel <= 53)
		{
			if (diff <= -19)
				conlevel = ConsiderColor::Gray;
			else if (diff <= -15)
				conlevel = ConsiderColor::LightBlue;
			else
				conlevel = ConsiderColor::DarkBlue;
		}
		else if (mylevel <= 55)
		{
			if (diff <= -20)
				conlevel = ConsiderColor::Gray;
			else if (diff <= -15)
				conlevel = ConsiderColor::LightBlue;
			else
				conlevel = ConsiderColor::DarkBlue;
		}
		else
		{
			if (diff <= -21)
				conlevel = ConsiderColor::Gray;
			else if (diff <= -16)
				conlevel = ConsiderColor::LightBlue;
			else
				conlevel = ConsiderColor::DarkBlue;
		}
	}
	else
	{
		int16 diff = iOtherLevel - mylevel;
		uint32 conGrayLvl = mylevel - (int32)((mylevel + 5) / 3);
		uint32 conGreenLvl = mylevel - (int32)((mylevel + 7) / 4);

		if (diff == 0)
			return ConsiderColor::White;
		else if (diff >= 1 && diff <= 3)
			return ConsiderColor::Yellow;
		else if (diff >= 4)
			return ConsiderColor::Red;

		if (mylevel <= 15)
		{
			if (diff <= -6)
				conlevel = ConsiderColor::Gray;
			else
				conlevel = ConsiderColor::DarkBlue;
		}
		else
			if (mylevel <= 20)
			{
				if (iOtherLevel <= conGrayLvl)
					conlevel = ConsiderColor::Gray;
				else
					if (iOtherLevel <= conGreenLvl)
						conlevel = ConsiderColor::Green;
					else
						conlevel = ConsiderColor::DarkBlue;
			}
			else
			{
				if (iOtherLevel <= conGrayLvl)
					conlevel = ConsiderColor::Gray;
				else
					if (iOtherLevel <= conGreenLvl)
						conlevel = ConsiderColor::Green;
					else
						if (diff <= -6)
							conlevel = ConsiderColor::LightBlue;
						else
							conlevel = ConsiderColor::DarkBlue;
			}
	}
	return conlevel;
}

void NPC::CheckSignal() {
	if (!signal_q.empty()) {
		int signal_id = signal_q.front();
		signal_q.pop_front();

		if (parse->HasQuestSub(GetNPCTypeID(), EVENT_SIGNAL)) {
			parse->EventNPC(EVENT_SIGNAL, this, nullptr, std::to_string(signal_id), 0);
		}
	}
}

bool IsSpellInList(DBnpcspells_Struct* spell_list, uint16 iSpellID);
bool IsSpellEffectInList(DBnpcspellseffects_Struct* spelleffect_list, uint16 iSpellEffectID, int32 base_value, int32 limit, int32 max_value);

bool NPC::AI_AddNPCSpells(uint32 iDBSpellsID) {
	// ok, this function should load the list, and the parent list then shove them into the struct and sort
	npc_spells_id = iDBSpellsID;
	AIspells.clear();
	if (iDBSpellsID == 0) {
		AIautocastspell_timer->Disable();
		return false;
	}
	DBnpcspells_Struct* spell_list = content_db.GetNPCSpells(iDBSpellsID);
	if (!spell_list) {
		AIautocastspell_timer->Disable();
		return false;
	}
	DBnpcspells_Struct* parentlist = content_db.GetNPCSpells(spell_list->parent_list);
	std::string debug_msg = StringFormat("Loading NPCSpells onto %s: dbspellsid=%u, level=%u", GetName(), iDBSpellsID, GetLevel());
	if (spell_list) {
		debug_msg.append(StringFormat(" (found, %u), parentlist=%u", spell_list->entries.size(), spell_list->parent_list));
		if (spell_list->parent_list) {
			if (parentlist)
				debug_msg.append(StringFormat(" (found, %u)", parentlist->entries.size()));
			else
				debug_msg.append(" (not found)");
		}
	}
	else {
		debug_msg.append(" (not found)");
	}
	LogAI("[{}]", debug_msg.c_str());

	if (parentlist) {
		for (const auto &iter : parentlist->entries) {
			LogAIDetail("([{}]) [{}]", iter.spellid, spells[iter.spellid].name);
		}
	}
	LogAI("fin (parent list)");
	if (spell_list) {
		for (const auto &iter : spell_list->entries) {
			LogAIDetail("([{}]) [{}]", iter.spellid, spells[iter.spellid].name);
		}
	}
	LogAI("fin (spell list)");

	uint16 attack_proc_spell = -1;
	int8 proc_chance = 3;
	uint16 range_proc_spell = -1;
	int16 rproc_chance = 0;
	uint16 defensive_proc_spell = -1;
	int16 dproc_chance = 0;
	uint32 _fail_recast = 0;
	uint32 _engaged_no_sp_recast_min = 0;
	uint32 _engaged_no_sp_recast_max = 0;
	uint8 _engaged_beneficial_self_chance = 0;
	uint8 _engaged_beneficial_other_chance = 0;
	uint8 _engaged_detrimental_chance = 0;
	uint32 _pursue_no_sp_recast_min = 0;
	uint32 _pursue_no_sp_recast_max = 0;
	uint8 _pursue_detrimental_chance = 0;
	uint32 _idle_no_sp_recast_min = 0;
	uint32 _idle_no_sp_recast_max = 0;
	uint8 _idle_beneficial_chance = 0;

	if (parentlist) {
		attack_proc_spell = parentlist->attack_proc;
		proc_chance = parentlist->proc_chance;
		range_proc_spell = parentlist->range_proc;
		rproc_chance = parentlist->rproc_chance;
		defensive_proc_spell = parentlist->defensive_proc;
		dproc_chance = parentlist->dproc_chance;
		_fail_recast = parentlist->fail_recast;
		_engaged_no_sp_recast_min = parentlist->engaged_no_sp_recast_min;
		_engaged_no_sp_recast_max = parentlist->engaged_no_sp_recast_max;
		_engaged_beneficial_self_chance = parentlist->engaged_beneficial_self_chance;
		_engaged_beneficial_other_chance = parentlist->engaged_beneficial_other_chance;
		_engaged_detrimental_chance = parentlist->engaged_detrimental_chance;
		_pursue_no_sp_recast_min = parentlist->pursue_no_sp_recast_min;
		_pursue_no_sp_recast_max = parentlist->pursue_no_sp_recast_max;
		_pursue_detrimental_chance = parentlist->pursue_detrimental_chance;
		_idle_no_sp_recast_min = parentlist->idle_no_sp_recast_min;
		_idle_no_sp_recast_max = parentlist->idle_no_sp_recast_max;
		_idle_beneficial_chance = parentlist->idle_beneficial_chance;
		for (auto &e : parentlist->entries) {
			if (GetLevel() >= e.minlevel && GetLevel() <= e.maxlevel && e.spellid > 0) {
				if (!IsSpellInList(spell_list, e.spellid))
				{
					AddSpellToNPCList(e.priority, e.spellid, e.type, e.manacost, e.recast_delay, e.resist_adjust, e.min_hp, e.max_hp);
				}
			}
		}
	}
	if (spell_list->attack_proc >= 0) {
		attack_proc_spell = spell_list->attack_proc;
		proc_chance = spell_list->proc_chance;
	}

	if (spell_list->range_proc >= 0) {
		range_proc_spell = spell_list->range_proc;
		rproc_chance = spell_list->rproc_chance;
	}

	if (spell_list->defensive_proc >= 0) {
		defensive_proc_spell = spell_list->defensive_proc;
		dproc_chance = spell_list->dproc_chance;
	}

	//If any casting variables are defined in the current list, ignore those in the parent list.
	if (spell_list->fail_recast || spell_list->engaged_no_sp_recast_min || spell_list->engaged_no_sp_recast_max
		|| spell_list->engaged_beneficial_self_chance || spell_list->engaged_beneficial_other_chance || spell_list->engaged_detrimental_chance
		|| spell_list->pursue_no_sp_recast_min || spell_list->pursue_no_sp_recast_max || spell_list->pursue_detrimental_chance
		|| spell_list->idle_no_sp_recast_min || spell_list->idle_no_sp_recast_max || spell_list->idle_beneficial_chance) {
		_fail_recast = spell_list->fail_recast;
		_engaged_no_sp_recast_min = spell_list->engaged_no_sp_recast_min;
		_engaged_no_sp_recast_max = spell_list->engaged_no_sp_recast_max;
		_engaged_beneficial_self_chance = spell_list->engaged_beneficial_self_chance;
		_engaged_beneficial_other_chance = spell_list->engaged_beneficial_other_chance;
		_engaged_detrimental_chance = spell_list->engaged_detrimental_chance;
		_pursue_no_sp_recast_min = spell_list->pursue_no_sp_recast_min;
		_pursue_no_sp_recast_max = spell_list->pursue_no_sp_recast_max;
		_pursue_detrimental_chance = spell_list->pursue_detrimental_chance;
		_idle_no_sp_recast_min = spell_list->idle_no_sp_recast_min;
		_idle_no_sp_recast_max = spell_list->idle_no_sp_recast_max;
		_idle_beneficial_chance = spell_list->idle_beneficial_chance;
	}

	for (auto &e : spell_list->entries) {
		if (GetLevel() >= e.minlevel && GetLevel() <= e.maxlevel && e.spellid > 0) {
			AddSpellToNPCList(e.priority, e.spellid, e.type, e.manacost, e.recast_delay, e.resist_adjust, e.min_hp, e.max_hp);
		}
	}

	std::sort(AIspells.begin(), AIspells.end(), [](const AISpells_Struct& a, const AISpells_Struct& b) {
		return a.priority > b.priority;
	});

	if (IsValidSpell(attack_proc_spell)) {
		AddProcToWeapon(attack_proc_spell, true, proc_chance);

		if(RuleB(Spells, NPCInnateProcOverride))
			innate_proc_spell_id = attack_proc_spell;
	}

	if (IsValidSpell(range_proc_spell))
		AddRangedProc(range_proc_spell, (rproc_chance + 100));

	if (IsValidSpell(defensive_proc_spell))
		AddDefensiveProc(defensive_proc_spell, (dproc_chance + 100));

	//Set AI casting variables

	AISpellVar.fail_recast = (_fail_recast) ? _fail_recast : RuleI(Spells, AI_SpellCastFinishedFailRecast);
	AISpellVar.engaged_no_sp_recast_min = (_engaged_no_sp_recast_min) ? _engaged_no_sp_recast_min : RuleI(Spells, AI_EngagedNoSpellMinRecast);
	AISpellVar.engaged_no_sp_recast_max = (_engaged_no_sp_recast_max) ? _engaged_no_sp_recast_max : RuleI(Spells, AI_EngagedNoSpellMaxRecast);
	AISpellVar.engaged_beneficial_self_chance = (_engaged_beneficial_self_chance) ? _engaged_beneficial_self_chance : RuleI(Spells, AI_EngagedBeneficialSelfChance);
	AISpellVar.engaged_beneficial_other_chance = (_engaged_beneficial_other_chance) ? _engaged_beneficial_other_chance : RuleI(Spells, AI_EngagedBeneficialOtherChance);
	AISpellVar.engaged_detrimental_chance = (_engaged_detrimental_chance) ? _engaged_detrimental_chance : RuleI(Spells, AI_EngagedDetrimentalChance);
	AISpellVar.pursue_no_sp_recast_min = (_pursue_no_sp_recast_min) ? _pursue_no_sp_recast_min : RuleI(Spells, AI_PursueNoSpellMinRecast);
	AISpellVar.pursue_no_sp_recast_max = (_pursue_no_sp_recast_max) ? _pursue_no_sp_recast_max : RuleI(Spells, AI_PursueNoSpellMaxRecast);
	AISpellVar.pursue_detrimental_chance = (_pursue_detrimental_chance) ? _pursue_detrimental_chance : RuleI(Spells, AI_PursueDetrimentalChance);
	AISpellVar.idle_no_sp_recast_min = (_idle_no_sp_recast_min) ? _idle_no_sp_recast_min : RuleI(Spells, AI_IdleNoSpellMinRecast);
	AISpellVar.idle_no_sp_recast_max = (_idle_no_sp_recast_max) ? _idle_no_sp_recast_max : RuleI(Spells, AI_IdleNoSpellMaxRecast);
	AISpellVar.idle_beneficial_chance = (_idle_beneficial_chance) ? _idle_beneficial_chance : RuleI(Spells, AI_IdleBeneficialChance);

	if (AIspells.empty())
		AIautocastspell_timer->Disable();
	else
		AIautocastspell_timer->Trigger();
	return true;
}

bool NPC::AI_AddNPCSpellsEffects(uint32 iDBSpellsEffectsID) {

	npc_spells_effects_id = iDBSpellsEffectsID;
	AIspellsEffects.clear();

	if (iDBSpellsEffectsID == 0)
		return false;

	DBnpcspellseffects_Struct* spell_effects_list = content_db.GetNPCSpellsEffects(iDBSpellsEffectsID);

	if (!spell_effects_list) {
		return false;
	}

	DBnpcspellseffects_Struct* parentlist = content_db.GetNPCSpellsEffects(spell_effects_list->parent_list);

	uint32 i;
	std::string debug_msg = StringFormat("Loading NPCSpellsEffects onto %s: dbspellseffectid=%u", GetName(), iDBSpellsEffectsID);
	if (spell_effects_list) {
		debug_msg.append(StringFormat(" (found, %u), parentlist=%u", spell_effects_list->numentries, spell_effects_list->parent_list));
		if (spell_effects_list->parent_list) {
			if (parentlist)
				debug_msg.append(StringFormat(" (found, %u)", parentlist->numentries));
			else
				debug_msg.append(" (not found)");
		}
	}
	else {
		debug_msg.append(" (not found)");
	}
	LogAI("[{}]", debug_msg.c_str());

	if (parentlist) {
		for (i=0; i<parentlist->numentries; i++) {
			if (GetLevel() >= parentlist->entries[i].minlevel && GetLevel() <= parentlist->entries[i].maxlevel && parentlist->entries[i].spelleffectid > 0) {
				if (!IsSpellEffectInList(spell_effects_list, parentlist->entries[i].spelleffectid, parentlist->entries[i].base_value,
					parentlist->entries[i].limit, parentlist->entries[i].max_value))
				{
				AddSpellEffectToNPCList(parentlist->entries[i].spelleffectid,
						parentlist->entries[i].base_value, parentlist->entries[i].limit,
						parentlist->entries[i].max_value);
				}
			}
		}
	}

	for (i=0; i<spell_effects_list->numentries; i++) {
		if (GetLevel() >= spell_effects_list->entries[i].minlevel && GetLevel() <= spell_effects_list->entries[i].maxlevel && spell_effects_list->entries[i].spelleffectid > 0) {
			AddSpellEffectToNPCList(spell_effects_list->entries[i].spelleffectid,
				spell_effects_list->entries[i].base_value, spell_effects_list->entries[i].limit,
				spell_effects_list->entries[i].max_value);
		}
	}

	return true;
}

void NPC::ApplyAISpellEffects(StatBonuses* newbon)
{
	if (!AI_HasSpellsEffects())
		return;

	for (int i = 0; i < AIspellsEffects.size(); i++)
		ApplySpellsBonuses(0, 0, newbon, 0, 0, 0, -1, 10, true, AIspellsEffects[i].spelleffectid,
				   AIspellsEffects[i].base_value, AIspellsEffects[i].limit, AIspellsEffects[i].max_value);

	return;
}

// adds a spell to the list, taking into account priority and resorting list as needed.
void NPC::AddSpellEffectToNPCList(uint16 iSpellEffectID, int32 base_value, int32 limit, int32 max_value, bool apply_bonus)
{
	if(!iSpellEffectID)
		return;

	HasAISpellEffects = true;
	AISpellsEffects_Struct t;

	t.spelleffectid = iSpellEffectID;
	t.base_value = base_value;
	t.limit = limit;
	t.max_value = max_value;
	AIspellsEffects.push_back(t);

	//we recalculate if applied from quest script.
	if (apply_bonus) {
		CalcBonuses();
	}
}

void NPC::RemoveSpellEffectFromNPCList(uint16 iSpellEffectID, bool apply_bonus)
{
	auto iter = AIspellsEffects.begin();
	while (iter != AIspellsEffects.end())
	{
		if ((*iter).spelleffectid == iSpellEffectID)
		{
			iter = AIspellsEffects.erase(iter);
			continue;
		}
		++iter;
	}

	if (apply_bonus) {
		CalcBonuses();
	}
}

bool NPC::HasAISpellEffect(uint16 spell_effect_id)
{
	for (const auto& spell_effect : AIspellsEffects) {
		if (spell_effect.spelleffectid == spell_effect_id) {
			return true;
		}
	}

	return false;
}

bool IsSpellEffectInList(DBnpcspellseffects_Struct* spelleffect_list, uint16 iSpellEffectID, int32 base_value, int32 limit, int32 max_value) {
	for (uint32 i=0; i < spelleffect_list->numentries; i++) {
		if (spelleffect_list->entries[i].spelleffectid == iSpellEffectID &&  spelleffect_list->entries[i].base_value == base_value
			&& spelleffect_list->entries[i].limit == limit && spelleffect_list->entries[i].max_value == max_value)
			return true;
	}
	return false;
}

bool IsSpellInList(DBnpcspells_Struct* spell_list, uint16 iSpellID) {
	auto it = std::find_if(spell_list->entries.begin(), spell_list->entries.end(),
			       [iSpellID](const DBnpcspells_entries_Struct &a) { return a.spellid == iSpellID; });
	return it != spell_list->entries.end();
}

// adds a spell to the list, taking into account priority and resorting list as needed.
void NPC::AddSpellToNPCList(int16 iPriority, uint16 iSpellID, uint32 iType,
							int16 iManaCost, int32 iRecastDelay, int16 iResistAdjust, int8 min_hp, int8 max_hp)
{

	if(!IsValidSpell(iSpellID))
		return;

	HasAISpell = true;
	AISpells_Struct t;

	t.priority = iPriority;
	t.spellid = iSpellID;
	t.type = iType;
	t.manacost = iManaCost;
	t.recast_delay = iRecastDelay;
	t.time_cancast = 0;
	t.resist_adjust = iResistAdjust;
	t.min_hp = min_hp;
	t.max_hp = max_hp;

	AIspells.push_back(t);

	// If we're going from an empty list, we need to start the timer
	if (AIspells.size() == 1)
		AIautocastspell_timer->Start(RandomTimer(0, 300), false);
}

void NPC::RemoveSpellFromNPCList(uint16 spell_id)
{
	auto iter = AIspells.begin();
	while(iter != AIspells.end())
	{
		if((*iter).spellid == spell_id)
		{
			iter = AIspells.erase(iter);
			continue;
		}
		++iter;
	}
}

void NPC::AISpellsList(Client *c)
{
	if (!c) {
		return;
	}

	if (AIspells.size() > 0) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} has {} AI spells.",
				GetCleanName(),
				AIspells.size()
			).c_str()
		);

		int spell_slot = 1;
		for (const auto& ai_spell : AIspells) {
			c->Message(
				Chat::White,
				fmt::format(
					"Spell {} | Name: {} ({}) Type: {} Mana Cost: {}",
					spell_slot,
					GetSpellName(ai_spell.spellid),
					ai_spell.spellid,
					ai_spell.type,
					ai_spell.manacost
				).c_str()
			);

			c->Message(
				Chat::White,
				fmt::format(
					"Spell {} | Priority: {} Recast Delay: {} Resist Difficulty: {}",
					spell_slot,
					ai_spell.priority,
					ai_spell.recast_delay,
					ai_spell.resist_adjust
				).c_str()
			);

			if (ai_spell.time_cancast) {
				c->Message(
					Chat::White,
					fmt::format(
						"Spell {} | Time Can Cast : {}",
						spell_slot,
						ai_spell.time_cancast
					).c_str()
				);
			}

			if (ai_spell.resist_adjust) {
				c->Message(
					Chat::White,
					fmt::format(
						"Spell {} | Resist Adjust: {}",
						spell_slot,
						ai_spell.resist_adjust
					).c_str()
				);
			}

			if (ai_spell.min_hp || ai_spell.max_hp) {
				c->Message(
					Chat::White,
					fmt::format(
						"Spell {} | Min HP: {} Max HP: {}",
						spell_slot,
						ai_spell.min_hp,
						ai_spell.max_hp
					).c_str()
				);
			}

			spell_slot++;
		}
	}
	else {
		c->Message(
			Chat::White,
			fmt::format(
				"{} has no AI spells.",
				GetCleanName()
			).c_str()
		);
	}

	return;
}

DBnpcspells_Struct *ZoneDatabase::GetNPCSpells(uint32 npc_spells_id)
{
	if (npc_spells_id == 0) {
		return nullptr;
	}

	auto it = npc_spells_cache.find(npc_spells_id);
	if (it != npc_spells_cache.end()) { // it's in the cache, easy =)
		return &it->second;
	}

	if (!npc_spells_loadtried.count(npc_spells_id)) { // no reason to ask the DB again if we have failed once already
		npc_spells_loadtried.insert(npc_spells_id);

		auto ns = NpcSpellsRepository::FindOne(*this, npc_spells_id);
		if (!ns.id) {
			return nullptr;
		}

		DBnpcspells_Struct ss;

		ss.parent_list                     = ns.parent_list;
		ss.attack_proc                     = ns.attack_proc;
		ss.proc_chance                     = ns.proc_chance;
		ss.range_proc                      = ns.range_proc;
		ss.rproc_chance                    = ns.rproc_chance;
		ss.defensive_proc                  = ns.defensive_proc;
		ss.dproc_chance                    = ns.dproc_chance;
		ss.fail_recast                     = ns.fail_recast;
		ss.engaged_no_sp_recast_min        = ns.engaged_no_sp_recast_min;
		ss.engaged_no_sp_recast_max        = ns.engaged_no_sp_recast_max;
		ss.engaged_beneficial_self_chance  = ns.engaged_b_self_chance;
		ss.engaged_beneficial_other_chance = ns.engaged_b_other_chance;
		ss.engaged_detrimental_chance      = ns.engaged_d_chance;
		ss.pursue_no_sp_recast_min         = ns.pursue_no_sp_recast_min;
		ss.pursue_no_sp_recast_max         = ns.pursue_no_sp_recast_max;
		ss.pursue_detrimental_chance       = ns.pursue_d_chance;
		ss.idle_no_sp_recast_min           = ns.idle_no_sp_recast_min;
		ss.idle_no_sp_recast_max           = ns.idle_no_sp_recast_max;
		ss.idle_beneficial_chance          = ns.idle_b_chance;

		auto entries = NpcSpellsEntriesRepository::GetWhere(
			*this,
			fmt::format(
				"npc_spells_id = {} {} ORDER BY minlevel",
				npc_spells_id,
				ContentFilterCriteria::apply()
			)
		);

		for (auto &e: entries) {
			DBnpcspells_entries_Struct se{};

			se.spellid      = e.spellid;
			se.type         = e.type;
			se.minlevel     = e.minlevel;
			se.maxlevel     = e.maxlevel;
			se.manacost     = e.manacost;
			se.recast_delay = e.recast_delay;
			se.priority     = e.priority;
			se.min_hp       = e.min_hp;
			se.max_hp       = e.max_hp;

			// some spell types don't make much since to be priority 0, so fix that
			if (!(se.type & SPELL_TYPES_INNATE) && se.priority == 0) {
				se.priority = 1;
			}

			if (e.resist_adjust) {
				se.resist_adjust = e.resist_adjust;
			}
			else if (IsValidSpell(e.spellid)) {
				se.resist_adjust = spells[e.spellid].resist_difficulty;
			}

			ss.entries.push_back(se);
		}

		npc_spells_cache.emplace(std::make_pair(npc_spells_id, ss));

		return &npc_spells_cache[npc_spells_id];
	}

	return nullptr;
}

DBnpcspellseffects_Struct *ZoneDatabase::GetNPCSpellsEffects(uint32 iDBSpellsEffectsID)
{
	if (iDBSpellsEffectsID == 0)
		return nullptr;

	if (!npc_spellseffects_cache) {
		npc_spellseffects_maxid = GetMaxNPCSpellsEffectsID();
		npc_spellseffects_cache = new DBnpcspellseffects_Struct *[npc_spellseffects_maxid + 1];
		npc_spellseffects_loadtried = new bool[npc_spellseffects_maxid + 1];
		for (uint32 i = 0; i <= npc_spellseffects_maxid; i++) {
			npc_spellseffects_cache[i] = nullptr;
			npc_spellseffects_loadtried[i] = false;
		}
	}

	if (iDBSpellsEffectsID > npc_spellseffects_maxid)
		return nullptr;

	if (npc_spellseffects_cache[iDBSpellsEffectsID]) // it's in the cache, easy =)
		return npc_spellseffects_cache[iDBSpellsEffectsID];

	if (npc_spellseffects_loadtried[iDBSpellsEffectsID])
		return nullptr;

	npc_spellseffects_loadtried[iDBSpellsEffectsID] = true;

	std::string query =
	    StringFormat("SELECT id, parent_list FROM npc_spells_effects WHERE id=%d", iDBSpellsEffectsID);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return nullptr;
	}

	if (results.RowCount() != 1)
		return nullptr;

	auto row = results.begin();
	uint32 tmpparent_list = Strings::ToInt(row[1]);

	query = StringFormat("SELECT spell_effect_id, minlevel, "
			     "maxlevel,se_base, se_limit, se_max "
			     "FROM npc_spells_effects_entries "
			     "WHERE npc_spells_effects_id = %d ORDER BY minlevel",
			     iDBSpellsEffectsID);
	results = QueryDatabase(query);
	if (!results.Success())
		return nullptr;

	uint32 tmpSize =
	    sizeof(DBnpcspellseffects_Struct) + (sizeof(DBnpcspellseffects_entries_Struct) * results.RowCount());
	npc_spellseffects_cache[iDBSpellsEffectsID] = (DBnpcspellseffects_Struct *)new uchar[tmpSize];
	memset(npc_spellseffects_cache[iDBSpellsEffectsID], 0, tmpSize);
	npc_spellseffects_cache[iDBSpellsEffectsID]->parent_list = tmpparent_list;
	npc_spellseffects_cache[iDBSpellsEffectsID]->numentries = results.RowCount();

	int entryIndex = 0;
	for (row = results.begin(); row != results.end(); ++row, ++entryIndex) {
		int spell_effect_id = Strings::ToInt(row[0]);
		npc_spellseffects_cache[iDBSpellsEffectsID]->entries[entryIndex].spelleffectid = spell_effect_id;
		npc_spellseffects_cache[iDBSpellsEffectsID]->entries[entryIndex].minlevel = Strings::ToInt(row[1]);
		npc_spellseffects_cache[iDBSpellsEffectsID]->entries[entryIndex].maxlevel = Strings::ToInt(row[2]);
		npc_spellseffects_cache[iDBSpellsEffectsID]->entries[entryIndex].base_value = Strings::ToInt(row[3]);
		npc_spellseffects_cache[iDBSpellsEffectsID]->entries[entryIndex].limit = Strings::ToInt(row[4]);
		npc_spellseffects_cache[iDBSpellsEffectsID]->entries[entryIndex].max_value = Strings::ToInt(row[5]);
	}

	return npc_spellseffects_cache[iDBSpellsEffectsID];
}

uint32 ZoneDatabase::GetMaxNPCSpellsEffectsID() {

	std::string query = "SELECT max(id) FROM npc_spells_effects";
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return 0;
	}

    if (results.RowCount() != 1)
        return 0;

    auto row = results.begin();
    if (!row[0])
        return 0;

    return Strings::ToInt(row[0]);
}
