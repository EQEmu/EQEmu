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
	General outline of spell casting process

	1.
		a)	Client clicks a spell bar gem, ability, or item. client_process.cpp
		gets the op and calls CastSpell() with all the relevant info including
		cast time.

		b) NPC does CastSpell() from AI

	2.
		a)	CastSpell() determines there is a cast time and sets some state keeping
		flags to be used to check the progress of casting and finish it later.

		b)	CastSpell() sees there's no cast time, and calls CastedSpellFinished()
		Go to step 4.

	3.
		SpellProcess() notices that the spell casting timer which was set by
		CastSpell() is expired, and calls CastedSpellFinished()

	4.
		CastedSpellFinished() checks some timed spell specific things, like
		wether to interrupt or not, due to movement or melee. If successful
		SpellFinished() is called.

	5.
		SpellFinished() checks some things like LoS, reagents, target and
		figures out what's going to get hit by this spell based on its type.

	6.
		a)	Single target spell, SpellOnTarget() is called.

		b)	AE spell, Entity::AESpell() is called.

		c)	Group spell, Group::CastGroupSpell()/SpellOnTarget() is called as
		needed.

	7.
		SpellOnTarget() may or may not call SpellEffect() to cause effects to
		the target

	8.
		If this was timed, CastedSpellFinished() will restore the client's
		spell bar gems.

	Most user code should call CastSpell(), with a 0 casting time if needed,
	and not SpellFinished().
*/

#include "mob.h"

#include "common/bodytypes.h"
#include "common/classes.h"
#include "common/data_bucket.h"
#include "common/data_verification.h"
#include "common/eqemu_logsys.h"
#include "common/events/player_event_logs.h"
#include "common/item_instance.h"
#include "common/misc_functions.h"
#include "common/repositories/character_corpses_repository.h"
#include "common/repositories/spell_buckets_repository.h"
#include "common/rulesys.h"
#include "common/spdat.h"
#include "common/strings.h"
#include "zone/bot.h"
#include "zone/client.h"
#include "zone/fastmath.h"
#include "zone/lua_parser.h"
#include "zone/mob_movement_manager.h"
#include "zone/queryserv.h"
#include "zone/quest_parser_collection.h"
#include "zone/string_ids.h"
#include "zone/water_map.h"
#include "zone/worldserver.h"

#include <algorithm>
#include <cassert>

extern Zone         *zone;
extern volatile bool is_zone_loaded;
extern WorldServer   worldserver;
extern FastMath      g_Math;
extern QueryServ    *QServ;

using EQ::spells::CastingSlot;

// this is run constantly for every mob
void Mob::SpellProcess()
{
	// check the rapid recast prevention timer
	if(delaytimer == true && spellend_timer.Check())
	{
		spellend_timer.Disable();
		delaytimer = false;
		return;
	}

	// a timed spell is finished casting
	if (casting_spell_id != 0 && casting_spell_checks && spellend_timer.Check())
	{
		spellend_timer.Disable();
		delaytimer = false;
		CastedSpellFinished(casting_spell_id, casting_spell_targetid, casting_spell_slot,
			casting_spell_mana, casting_spell_inventory_slot, casting_spell_resist_adjust);
	}

}

void NPC::SpellProcess()
{
	Mob::SpellProcess();
}

///////////////////////////////////////////////////////////////////////////////
// functions related to begin/finish casting, fizzling etc

//
// only CastSpell and DoCastSpell should be setting casting_spell_id.
// basically casting_spell_id is only set when casting a triggered spell from
// the spell bar gems, an ability, or an item. note that it's actually set
// even if it's a 0 cast time, but then the spell is finished right after and
// it's unset. this is ok, since the 0 cast time spell is still a triggered
// one.
// the rule is you can cast one triggered (usually timed) spell at a time
// but things like SpellFinished() can run concurrent with a triggered cast
// to allow procs to work
bool Mob::CastSpell(uint16 spell_id, uint16 target_id, CastingSlot slot,
	int32 cast_time, int32 mana_cost, uint32* oSpellWillFinish, uint32 item_slot,
	uint32 timer, uint32 timer_duration, int16 *resist_adjust,
	uint32 aa_id)
{
	LogSpells("CastSpell called for spell [{}] ([{}]) on entity [{}], slot [{}], time [{}], mana [{}], from item slot [{}]",
		(IsValidSpell(spell_id)) ? spells[spell_id].name : "UNKNOWN SPELL", spell_id, target_id, static_cast<int>(slot), cast_time, mana_cost, (item_slot == 0xFFFFFFFF) ? 999 : item_slot);

	if (casting_spell_id == spell_id) {
		ZeroCastingVars();
	}

	//If spell fails checks here determine if we need to send packet to client to reset spell bar.
	bool send_spellbar_enable = true;
	if ((item_slot != -1 && cast_time == 0) || aa_id) {
		send_spellbar_enable = false;
	}

	if (!IsValidSpell(spell_id) ||
		casting_spell_id ||
		delaytimer ||
		spellend_timer.Enabled()) {
		LogSpells("Spell casting canceled: not able to cast now. Valid? [{}], casting [{}], waiting? [{}], spellend? [{}]",
			IsValidSpell(spell_id), casting_spell_id, delaytimer, spellend_timer.Enabled());
		StopCastSpell(spell_id, send_spellbar_enable);
		return false;
	}

	//Goal of Spells:UseSpellImpliedTargeting is to replicate the EQ2 feature where spells will 'pass through' invalid targets to target's target to try to find a valid target.
	if (RuleB(Spells,UseSpellImpliedTargeting) && IsOfClientBot()) {
		Mob* spell_target = entity_list.GetMobID(target_id);
		if (spell_target) {
			Mob* targets_target = spell_target->GetTarget();
			if (targets_target) {
				// If either this is beneficial and the target is not a player or player's pet or vis versa
				if ((IsBeneficialSpell(spell_id) && (!(spell_target->IsOfClientBot() || (spell_target->HasOwner() && spell_target->GetOwner()->IsOfClientBot()))))
					|| (IsDetrimentalSpell(spell_id) && (spell_target->IsOfClientBot() || (spell_target->HasOwner() && spell_target->GetOwner()->IsOfClientBot())))) {
					//Check if the target's target is a valid target; we can use DoCastingChecksOnTarget() here because we can let it handle the failure as vanilla would
					if (DoCastingChecksOnTarget(true, spell_id, targets_target)) {
						target_id = targets_target->GetID();
					}
					else {
						//Just return false here because we are going to fail the next check block anyway if we reach this point.
						StopCastSpell(spell_id, send_spellbar_enable);
						return false;
					}
				}
			}
		}
	}

	if (!DoCastingChecksOnCaster(spell_id, slot) ||
		!DoCastingChecksZoneRestrictions(true, spell_id) ||
		!DoCastingChecksOnTarget(true, spell_id, entity_list.GetMobID(target_id))) {
		StopCastSpell(spell_id, send_spellbar_enable);
		return false;
	}
	else {
		casting_spell_checks = true;
	}

	//It appears that the Sanctuary effect is removed by a check on the client side (keep this however for redundancy)
	if (spellbonuses.Sanctuary && (spells[spell_id].target_type != ST_Self && GetTarget() != this) || IsDetrimentalSpell(spell_id)) {
		BuffFadeByEffect(SpellEffect::Sanctuary);
	}

	if (spellbonuses.NegateIfCombat) {
		BuffFadeByEffect(SpellEffect::NegateIfCombat);
	}

	if(GetTarget() && IsManaTapSpell(spell_id)) {
		// If melee, block if ManaTapsOnAnyClass rule is false
		// if caster, block if ManaTapsRequireNPCMana and no mana
		bool melee_block  = !RuleB(Spells, ManaTapsOnAnyClass);
		bool caster_block = (
			!GetTarget()->IsPureMeleeClass() &&
			RuleB(Spells, ManaTapsRequireNPCMana) &&
			GetTarget()->GetMana() == 0
		);
		if (melee_block || caster_block) {
			InterruptSpell(TARGET_NO_MANA, 0x121, spell_id);
			return false;
		}
	}

	//Casting a spell from an item click will also stop bard pulse.
	if (HasActiveSong() && (IsBardSong(spell_id) || slot == CastingSlot::Item)) {
		LogSpells("Casting a new song while singing a song. Killing old song [{}]", bardsong);
		//Note: this does NOT tell the client
		ZeroBardPulseVars();
	}

	//Added to prevent MQ2 exploitation of equipping normally-unequippable/clickable items with effects and clicking them for benefits.
	if (item_slot != 0xFFFFFFFF && IsClient() && (slot == CastingSlot::Item || slot == CastingSlot::PotionBelt))
	{
		if (!CheckItemRaceClassDietyRestrictionsOnCast(item_slot)) {
			StopCastSpell(spell_id, send_spellbar_enable);
			return false;
		}
	}

	Mob* spell_target = entity_list.GetMobID(target_id);
	std::vector<std::any> args = { spell_target };
	int return_value = parse->EventMob(EVENT_CAST_BEGIN, this, nullptr, [&]() {
		return fmt::format(
			"{} {} {} {}",
			spell_id,
			GetID(),
			GetCasterLevel(spell_id),
			target_id
		);
	}, 0, &args);

	if (IsClient() && return_value != 0) {
		if (IsDiscipline(spell_id)) {
			CastToClient()->SendDisciplineTimer(spells[spell_id].timer_id, 0);
		} else {
			CastToClient()->SendSpellBarEnable(spell_id);
		}

		return false;
	}

	//To prevent NPC ghosting when spells are cast from scripts
	if (IsNPC() && IsMoving() && cast_time > 0) {
		StopNavigation();
	}

	if(resist_adjust)
	{
		return(DoCastSpell(spell_id, target_id, slot, cast_time, mana_cost, oSpellWillFinish, item_slot, timer, timer_duration, *resist_adjust, aa_id));
	}
	else
	{
		return(DoCastSpell(spell_id, target_id, slot, cast_time, mana_cost, oSpellWillFinish, item_slot, timer, timer_duration, spells[spell_id].resist_difficulty, aa_id));
	}
}

//
// the order of things here is intentional and important. make sure you
// understand the whole spell casting process and the flags that are passed
// around if you're gonna modify this
//
// this is the 2nd phase of CastSpell, broken up like this to make it easier
// to repeat a spell for bard songs
//
bool Mob::DoCastSpell(uint16 spell_id, uint16 target_id, CastingSlot slot,
					int32 cast_time, int32 mana_cost, uint32* oSpellWillFinish,
					uint32 item_slot, uint32 timer, uint32 timer_duration,
					int16 resist_adjust, uint32 aa_id)
{
	Mob* pMob = nullptr;
	int32 orgcasttime;

	if (!IsValidSpell(spell_id)) {
		InterruptSpell();
		return(false);
	}

	const SPDat_Spell_Struct &spell = spells[spell_id];

	LogSpells("DoCastSpell called for spell [{}] ([{}]) on entity [{}], slot [{}], time [{}], mana [{}], from item [{}]",
		spell.name, spell_id, target_id, static_cast<int>(slot), cast_time, mana_cost, item_slot==0xFFFFFFFF?999:item_slot);

	casting_spell_id = spell_id;
	casting_spell_slot = slot;
	casting_spell_inventory_slot = item_slot;
	if (casting_spell_timer != 0xFFFFFFFF) {
		casting_spell_timer = timer;
		casting_spell_timer_duration = timer_duration;
	}
	casting_spell_aa_id = aa_id;

	// check for fizzle
	// note that CheckFizzle itself doesn't let NPCs fizzle,
	// but this code allows for it.
	if (slot < CastingSlot::MaxGems && !CheckFizzle(spell_id)) {
		int fizzle_msg = IsBardSong(spell_id) ? MISS_NOTE : SPELL_FIZZLE;

		uint32 use_mana = ((spells[spell_id].mana) / 4);
		LogSpells("Spell casting canceled: fizzled. [{}] mana has been consumed", use_mana);

		// fizzle 1/4 the mana away
		Mob::SetMana(GetMana() - use_mana); // We send StopCasting which will update mana
		StopCasting();

		MessageString(Chat::SpellFailure, fizzle_msg);

		/**
		 * Song Failure message
		 */
		entity_list.FilteredMessageCloseString(
			this,
			true,
			RuleI(Range, SpellMessages),
			Chat::SpellFailure,
			(IsClient() ? FilterPCSpells : FilterNPCSpells),
			(fizzle_msg == MISS_NOTE ? MISSED_NOTE_OTHER : SPELL_FIZZLE_OTHER),
			0,
			/*
				MessageFormat: You miss a note, bringing your song to a close! (if missed note)
				MessageFormat: A missed note brings %1's song to a close!
				MessageFormat: %1's spell fizzles!
			*/
			GetName()
		);

		TryTriggerOnCastRequirement();
		return(false);
	}

	SaveSpellLoc();
	LogSpells("Casting [{}] Started at ({},{},{})", spell_id, m_SpellLocation.x, m_SpellLocation.y, m_SpellLocation.z);

	// if this spell doesn't require a target, or if it's an optional target
	// and a target wasn't provided, then it's us; unless TGB is on and this
	// is a TGB compatible spell.
	if (
		(
			IsGroupSpell(spell_id) ||
			spell.target_type == ST_AEClientV1 ||
			spell.target_type == ST_Self ||
			spell.target_type == ST_AECaster ||
			spell.target_type == ST_Ring ||
			spell.target_type == ST_Beam
		) && target_id == 0
	) {
		LogSpells("Spell [{}] auto-targeted the caster. Group? [{}], target type [{}]", spell_id, IsGroupSpell(spell_id), static_cast<int>(spell.target_type));
		target_id = GetID();
	}

	if (cast_time <= -1) {
		// save the non-reduced cast time to use in the packet
		cast_time = orgcasttime = spell.cast_time;
		// if there's a cast time, check if they have a modifier for it
		if (cast_time) {
			cast_time = GetActSpellCasttime(spell_id, cast_time);
		}
	}
	//must use SPA 415 with focus (SPA 127/500/501) to reduce item recast
	else if (cast_time && IsOfClientBot() && slot == CastingSlot::Item && item_slot != 0xFFFFFFFF) {
		orgcasttime = cast_time;
		if (cast_time) {
			cast_time = GetActSpellCasttime(spell_id, cast_time);
		}
	} else {
		orgcasttime = cast_time;
	}

	// we checked for spells not requiring targets above
	if(target_id == 0) {
		LogSpells("Spell Error: no target. spell=[{}]", spell_id);
		if(IsClient()) {
			//clients produce messages... npcs should not for this case
			MessageString(Chat::Red, SPELL_NEED_TAR);
			InterruptSpell();
		} else {
			InterruptSpell(0, 0, 0);	//the 0 args should cause no messages
		}
		ZeroCastingVars();
		return(false);
	}

	// ok now we know the target
	casting_spell_targetid = target_id;

	// We don't get actual mana cost here, that's done when we consume the mana
	if (mana_cost == -1) {
		mana_cost = spell.mana;
	}

	// mana is checked for clients on the frontend. we need to recheck it for NPCs though
	// If you're at full mana, let it cast even if you dont have enough mana

	// we calculated this above, now enforce it
	if (mana_cost > 0 && slot != CastingSlot::Item || (IsBot() && !CastToBot()->IsBotNonSpellFighter())) {
		int my_curmana = GetMana();
		int my_maxmana = GetMaxMana();
		if (my_curmana < mana_cost) {// not enough mana
			//this is a special case for NPCs with no mana...
			if (IsNPC() && my_curmana == my_maxmana) {
				mana_cost = 0;
			} else {
				DoSpellInterrupt(spell_id, mana_cost, my_curmana);
				return false;
			}
		}
	}

	if (mana_cost > GetMana()) {
		mana_cost = GetMana();
	}

	// we know our mana cost now
	casting_spell_mana = mana_cost;

	casting_spell_resist_adjust = resist_adjust;

	LogSpells("Spell [{}]: Casting time [{}] (orig [{}]), mana cost [{}]",
			spell_id, cast_time, orgcasttime, mana_cost);

	// now tell the people in the area -- we ALWAYS want to send this, even instant cast spells.
	// The only time this is skipped is for NPC innate procs and weapon procs. Procs from buffs
	// oddly still send this. Since those cases don't reach here, we don't need to check them
	if (slot != CastingSlot::Discipline) {
		SendBeginCast(spell_id, orgcasttime);
	}

	// cast time is 0, just finish it right now and be done with it
	if(cast_time == 0) {
		CastedSpellFinished(spell_id, target_id, slot, mana_cost, item_slot, resist_adjust); //
		return true;
	}
	// ok we know it has a cast time so we can start the timer now
	spellend_timer.Start(cast_time);

	if (IsAIControlled()) {
		SetRunAnimSpeed(0);
		pMob = entity_list.GetMob(target_id);
		if (pMob && this != pMob) {
			FaceTarget(pMob);
		}
	}

	// if we got here we didn't fizzle, and are starting our cast
	if (oSpellWillFinish) {
		*oSpellWillFinish = Timer::GetCurrentTime() + cast_time + 100;
	}

	if (RuleB(Spells, UseItemCastMessage) && IsClient() && slot == CastingSlot::Item && item_slot != 0xFFFFFFFF) {
		auto item = CastToClient()->GetInv().GetItem(item_slot);
		if (item && item->GetItem()) {
			MessageString(Chat::FocusEffect, BEGINS_TO_GLOW, item->GetItem()->Name);
		}
	}

	return true;
}

void Mob::DoSpellInterrupt(uint16 spell_id, int32 mana_cost, int my_curmana) {
	//The client will prevent spell casting if insufficient mana, this is only for serverside enforcement.
	LogSpells("Not enough mana spell [{}] curmana [{}] cost [{}]\n", spell_id, my_curmana, mana_cost);
	if (IsClient()) {
		//clients produce messages... npcs should not for this case
		MessageString(Chat::Red, INSUFFICIENT_MANA);
		InterruptSpell();
	} else {
		InterruptSpell(0, 0, 0);	//the 0 args should cause no messages
	}
	ZeroCastingVars();
}

void Mob::SendBeginCast(uint16 spell_id, uint32 casttime)
{
	auto outapp = new EQApplicationPacket(OP_BeginCast, sizeof(BeginCast_Struct));
	auto begincast = (BeginCast_Struct *)outapp->pBuffer;

	begincast->caster_id = GetID();
	begincast->spell_id = spell_id;
	begincast->cast_time = casttime; // client calculates reduced time by itself

	outapp->priority = 3;

	entity_list.QueueCloseClients(
		this, /* Sender */
		outapp, /* Packet */
		false, /* Ignore Sender */
		RuleI(Range, BeginCast),
		0, /* Skip this Mob */
		true /* Packet ACK */
	); //IsClient() ? FILTER_PCSPELLS : FILTER_NPCSPELLS);

	safe_delete(outapp);
}

bool Mob::DoCastingChecksOnCaster(int32 spell_id, CastingSlot slot) {

	/*
		These are casting requirements on the CASTER that will cancel a spell before spell finishes casting or prevent spell from casting.
		- caster_requirmement_id : checks specific requirements on caster (cast initiates)
		- linked timer spells. (cast initiates) [cancel before begin cast message]
		- must be out of combat spell field. (client blocks)
		- must be in combat spell field. (client blocks)

		Always checked at the start of CastSpell.
		Checked before special cases for bards casting from SpellFinished.
	*/

	/*
		Cannot cast if stunned or mezzed, unless spell has 'cast_not_standing' flag.
	*/
	if ((IsStunned() || IsMezzed()) && !IsCastNotStandingSpell(spell_id)) {
		LogSpells("Spell casting canceled [{}] : can not cast spell when stunned.", spell_id);
		return false;
	}
	/*
		Can not cast if feared.
	*/
	if (IsFeared()) {
		LogSpells("Spell casting canceled [{}] : can not cast spell when feared.", spell_id);
		return false;
	}
	/*
		Can not cast if spell
	*/
	if ((IsSilenced() && !IsDiscipline(spell_id))) {
		MessageString(Chat::Red, SILENCED_STRING);
		LogSpells("Spell casting canceled [{}] : can not cast spell when silenced.", spell_id);
		return false;
	}
	/*
		Can not cast if discipline.
	*/
	if (IsAmnesiad() && IsDiscipline(spell_id)) {
		MessageString(Chat::Red, MELEE_SILENCE);
		LogSpells("Spell casting canceled [{}] : can not use discipline with amnesia.", spell_id);
		return false;
	}
	/*
		Cannot cast under divine aura, unless spell has 'cast_not_standing' flag.
	*/
	if (DivineAura() && !IsCastNotStandingSpell(spell_id)) {
		LogSpells("Spell casting canceled [{}] : cannot cast while Divine Aura is in effect.", spell_id);
		InterruptSpell(173, 0x121, false); //not sure we need this.
		return false;
	}
	/*
		Linked Reused Timers that are not ready
	*/
	if (IsClient() && spells[spell_id].timer_id > 0 && slot < CastingSlot::MaxGems) {
		if (!CastToClient()->IsLinkedSpellReuseTimerReady(spells[spell_id].timer_id)) {
			LogSpells("Spell casting canceled [{}] : linked reuse timer not ready.", spell_id);
			return false;
		}
	}
	/*
		Spells that use caster_requirement_id field which requires specific conditions on caster to be met before casting.
	*/
	if (spells[spell_id].caster_requirement_id && !PassCastRestriction(spells[spell_id].caster_requirement_id)) {
		SendCastRestrictionMessage(spells[spell_id].caster_requirement_id, false, IsDiscipline(spell_id));
		LogSpells("Spell casting canceled [{}] : caster requirement id [{}] not met.", spell_id, spells[spell_id].caster_requirement_id);
		return false;
	}
	/*
		Spells that use field can_cast_in_comabt or can_cast_out of combat restricting
		caster to meet one of those conditions. If beneficial spell check casters state.
		If detrimental check the targets state (done elsewhere in this function).
	*/
	if (!spells[spell_id].can_cast_in_combat && spells[spell_id].can_cast_out_of_combat) {
		if (IsBeneficialSpell(spell_id)) {
			if ((IsNPC() && IsEngaged()) || (IsClient() && CastToClient()->GetAggroCount())) {
				if (IsDiscipline(spell_id)) {
					MessageString(Chat::Red, NO_ABILITY_IN_COMBAT);
				}
				else {
					MessageString(Chat::Red, NO_CAST_IN_COMBAT);
				}
				LogSpells("Spell casting canceled [{}] : can not use spell while in combat.", spell_id);
				return false;
			}
		}
	}
	else if (spells[spell_id].can_cast_in_combat && !spells[spell_id].can_cast_out_of_combat) {
		if (IsBeneficialSpell(spell_id)) {
			if ((IsNPC() && !IsEngaged()) || (IsClient() && !CastToClient()->GetAggroCount())) {
				if (IsDiscipline(spell_id)) {
					MessageString(Chat::Red, NO_ABILITY_OUT_OF_COMBAT);
				}
				else {
					MessageString(Chat::Red, NO_CAST_OUT_OF_COMBAT);
				}
				LogSpells("Spell casting canceled [{}] : can not use spell while out of combat.", spell_id);
				return false;
			}
		}
	}
	/*
		Focus version of Silence will prevent spell casting
	*/
	if (IsClient() && !IsDiscipline(spell_id)) {
		int chance = CastToClient()->GetFocusEffect(focusFcMute, spell_id);//client only
		if (chance && zone->random.Roll(chance)) {
			MessageString(Chat::Red, SILENCED_STRING);
			LogSpells("Spell casting canceled: can not cast spell when silenced from SPA 357 FcMute.");
			return(false);
		}
	}

	return true;
}

struct SpellCheck {
	std::function<bool()> condition; // The condition to check
	std::function<bool()> action;    // The action if the condition fails
};

bool Mob::DoCastingChecksZoneRestrictions(bool check_on_casting, int32 spell_id) {

	/*
		These are casting requirements determined by ZONE limiters that will cancel a spell before spell finishes casting or prevent spell from casting.
		- levitate zone restriction (client blocks)  [cancel before begin cast message]
		- can not cast outdoor [cancels after spell finishes channeling]

		If the spell is a cast spell, check on CastSpell and ignore on SpellFinished.
		If the spell is initiated from SpellFinished, then check at start of SpellFinished.
	*/

	bool bypass_casting_restrictions = !IsClient();
	glm::vec3 position = glm::vec3(GetPosition());

	auto gm_bypass_message = [&](const std::string& restriction) {
		if (CastToClient()->GetGM()) {
			Message(
				Chat::White,
				fmt::format(
					"Your GM flag allows you to bypass {} and cast {}.",
					restriction,
					Saylink::Silent(
						fmt::format("#castspell {}", spell_id),
						GetSpellName(spell_id)
					)
				).c_str()
			);
			return true;
		}
		return false;
	};

	std::vector<SpellCheck> spell_checks = {
		// Blocked spells
		{
			[&]() { return !bypass_casting_restrictions && zone->IsSpellBlocked(spell_id, position); },
			[&]() {
				if (gm_bypass_message("zone blocked spells")) { return true; }
				const char* msg = zone->GetSpellBlockedMessage(spell_id, position);
				Message(Chat::Red, msg ? msg : "You can't cast this spell here.");
				return false;
			}
		},
		// Levitation restriction
		{
			[&]() { return !bypass_casting_restrictions && !zone->CanLevitate() && IsEffectInSpell(spell_id, SpellEffect::Levitate); },
			[&]() {
				if (gm_bypass_message("zone levitation restrictions")) { return true; }
				Message(Chat::Red, "You have entered an area where levitation effects do not function.");
				return false;
			}
		},
		// Detrimental spells restriction
		{
			[&]() { return !bypass_casting_restrictions && IsDetrimentalSpell(spell_id) && !zone->CanDoCombat(); },
			[&]() {
				if (gm_bypass_message("no combat zone restrictions")) { return true; }
				Message(Chat::Red, "You cannot cast detrimental spells here.");
				return false;
			}
		},
		// Daytime-only spells
		{
			[&]() { return !bypass_casting_restrictions && spells[spell_id].time_of_day == SpellTimeRestrictions::Day && !zone->zone_time.IsDayTime(); },
			[&]() {
				if (gm_bypass_message("spell daytime restrictions")) { return true; }
				MessageString(Chat::Red, CAST_DAYTIME);
				return false;
			}
		},
		// Nighttime-only spells
		{
			[&]() { return !bypass_casting_restrictions && spells[spell_id].time_of_day == SpellTimeRestrictions::Night && !zone->zone_time.IsNightTime(); },
			[&]() {
				if (gm_bypass_message("spell nighttime restrictions")) return true;
				MessageString(Chat::Red, CAST_NIGHTTIME);
				return false;
			}
		},
		// Outdoor-only spells
		{
			[&]() { return check_on_casting && !bypass_casting_restrictions && spells[spell_id].zone_type == 1 && !zone->CanCastOutdoor(); },
			[&]() {
				if (gm_bypass_message("zone outdoor restrictions")) return true;
				MessageString(Chat::Red, CAST_OUTDOORS);
				return false;
			}
		}
	};

	for (const auto& check : spell_checks) {
		if (check.condition() && !check.action()) {
			return false;
		}
	}

	return true;
}


bool Mob::DoCastingChecksOnTarget(bool check_on_casting, int32 spell_id, Mob *spell_target) {

	/*
		These are casting requirements or TARGETS that will cancel a spell before spell finishes casting or prevent spell from casting.
		- cast_restriction : checks specific requirements on target (cast initiates)
		- target level restriction on buffs (cast initiates)
		- can not cast life tap on self (client blocks) [cancel before begin cast message]
		- can not cast sacrifice on self (cast initiates) [cancel before begin cast message]
		- charm restrictions (cast initiates) [cancel before begin cast message]
		- pcnpc_only_flag - (client blocks] [cancel before being cast message]

		If the spell is a casted spell, check on CastSpell and ignore on SpellFinished.
		If the spell is a initiated from SpellFinished, then check at start of SpellFinished.
		Always check again on SpellOnTarget to account for AE checks.
	*/

	bool ignore_on_casting = false;

	if (check_on_casting) {
		if (spells[spell_id].target_type == ST_AEClientV1 ||
			spells[spell_id].target_type == ST_AECaster ||
			spells[spell_id].target_type == ST_Ring ||
			spells[spell_id].target_type == ST_Beam) {
			return true;
		}

		if (!spell_target) {
			if (IsGroupSpell(spell_id)) {
				return true;
			} else if (spells[spell_id].target_type == ST_Self) {
				spell_target = this;
			}
		} else {
			if (IsGroupSpell(spell_id) && spell_target != this) {
				ignore_on_casting = true;
			}
		}
	}

	//If we still do not have a target end.
	if (!spell_target){
		return false;
	}
	/*
		Spells that use caster_restriction field which requires specific conditions on target to be met before casting.
		[Insufficient mana first]
	*/
	if (spells[spell_id].cast_restriction && !spell_target->PassCastRestriction(spells[spell_id].cast_restriction)) {
		SendCastRestrictionMessage(spells[spell_id].cast_restriction, true, IsDiscipline(spell_id));
		LogSpells("Spell casting canceled [{}] : target requirement id [{}] not met.", spell_id, spells[spell_id].caster_requirement_id);
		return false;
	}
	/*
		Spells that use field can_cast_in_comabt or can_cast_out of combat restricting
		caster to meet one of those conditions. If beneficial spell check casters state (done else where in this function)
		if detrimental check the targets state.
	*/
	if (!spells[spell_id].can_cast_in_combat && spells[spell_id].can_cast_out_of_combat) {
		if (IsDetrimentalSpell(spell_id)) {
			if (((spell_target->IsNPC() && spell_target->IsEngaged()) ||
				(spell_target->IsClient() && spell_target->CastToClient()->GetAggroCount()))) {
				MessageString(Chat::Red, SPELL_NO_EFFECT); // Unsure correct string
				LogSpells("Spell casting canceled [{}] : can not use spell while your target is in combat.", spell_id);
				return false;
			}
		}
	}
	else if (spells[spell_id].can_cast_in_combat && !spells[spell_id].can_cast_out_of_combat) {
		if (IsDetrimentalSpell(spell_id)) {
			if (((spell_target->IsNPC() && !spell_target->IsEngaged()) ||
				(spell_target->IsClient() && !spell_target->CastToClient()->GetAggroCount()))) {
				MessageString(Chat::Red, SPELL_NO_EFFECT); // Unsure correct string
				LogSpells("Spell casting canceled [{}] : can not use spell while your target is out of combat.", spell_id);
				return false;
			}
		}
	}
	/*
		Prevent buffs from being cast on targets who don't meet level restriction
	*/

	if (!spell_target->CheckSpellLevelRestriction(this, spell_id)) {
		return false;
	}
	/*
		Prevents buff from being cast based on tareget ing PC OR NPC (1 = PCs, 2 = NPCs)
		These target types skip pcnpc only check (according to dev quotes)
	*/
	if (!ignore_on_casting) {
		if (spells[spell_id].pcnpc_only_flag && spells[spell_id].target_type != ST_AETargetHateList && spells[spell_id].target_type != ST_HateList) {
			if (spells[spell_id].pcnpc_only_flag == PCNPCOnlyFlagType::PC && !spell_target->IsClient() && !spell_target->IsMerc() && !spell_target->IsBot()) {
				if (check_on_casting) {
					Message(Chat::SpellFailure, "This spell only works on other PCs");
				}
				return false;
			}
			else if (spells[spell_id].pcnpc_only_flag == PCNPCOnlyFlagType::NPC && (spell_target->IsClient() || spell_target->IsMerc() || spell_target->IsBot())) {
				if (check_on_casting) {
					Message(Chat::SpellFailure, "This spell only works on NPCs.");
				}
				return false;
			}
		}
	}
	/*
		Cannot cast shield target on self
	*/
	if (this == spell_target && IsEffectInSpell(spell_id, SpellEffect::Shield_Target)) {
		LogSpells("You cannot shield yourself");
		Message(Chat::SpellFailure, "You cannot shield yourself.");
		return false;
	}
	/*
		Cannot cast life tap on self
	*/
	if (this == spell_target && IsLifetapSpell(spell_id)) {
		LogSpells("You cannot lifetap yourself");
		MessageString(Chat::SpellFailure, CANT_DRAIN_SELF);
		return false;
	}
	/*
		Cannot cast sacrifice on self
	*/
	if (this == spell_target && IsSacrificeSpell(spell_id)) {
		LogSpells("You cannot sacrifice yourself");
		MessageString(Chat::SpellFailure, CANNOT_SAC_SELF);
		return false;
	}
	/*
		Max level of target for harmony to take hold
	*/
	if (IsClient() && IsHarmonySpell(spell_id) && !HarmonySpellLevelCheck(spell_id, spell_target)) {
		MessageString(Chat::SpellFailure, SPELL_NO_EFFECT);
		LogSpells("Spell casting canceled [{}] : can not use harmony on this target.", spell_id);
		return false;
	}
	/*
		Various charm related target restrictions
	*/
	if (IsEffectInSpell(spell_id, SpellEffect::Charm) && !PassCharmTargetRestriction(spell_target)) {
		LogSpells("Spell casting canceled [{}] : can not use charm on this target.", spell_id);
		return false;
	}
	/*
		Requires target to be in same group or same raid in order to apply invisible.
	*/
	if (
		check_on_casting &&
		spells[spell_id].target_type != ST_Self &&
		GetTarget() != this &&
		RuleB(Spells, InvisRequiresGroup) &&
		IsInvisibleSpell(spell_id)
	) {
		if (IsClient() && spell_target && spell_target->IsClient()) {
			if (spell_target && spell_target->GetID() != GetID()) {
				bool cast_failed = true;
				if (spell_target->IsGrouped()) {
					Group *target_group = spell_target->GetGroup();
					Group *my_group = GetGroup();
					if (target_group &&
						my_group &&
						(target_group->GetID() == my_group->GetID())) {
						cast_failed = false;
					}
				} else if (spell_target->IsRaidGrouped()) {
					Raid *target_raid = spell_target->GetRaid();
					Raid *my_raid = GetRaid();
					if (target_raid &&
						my_raid &&
						(target_raid->GetGroup(spell_target->CastToClient()) == my_raid->GetGroup(CastToClient()))) {
						cast_failed = false;
					}
				}

				if (cast_failed) {
					MessageString(Chat::Red, TARGET_GROUP_MEMBER);
					return false;
				}
			}
		}
	}

	return true;
}

uint16 Mob::GetSpecializeSkillValue(uint16 spell_id) const {
	switch(spells[spell_id].skill) {
	case EQ::skills::SkillAbjuration:
		return(GetSkill(EQ::skills::SkillSpecializeAbjure));
	case EQ::skills::SkillAlteration:
		return(GetSkill(EQ::skills::SkillSpecializeAlteration));
	case EQ::skills::SkillConjuration:
		return(GetSkill(EQ::skills::SkillSpecializeConjuration));
	case EQ::skills::SkillDivination:
		return(GetSkill(EQ::skills::SkillSpecializeDivination));
	case EQ::skills::SkillEvocation:
		return(GetSkill(EQ::skills::SkillSpecializeEvocation));
	default:
		//wtf...
		break;
	}
	return(0);
}

void Client::CheckSpecializeIncrease(uint16 spell_id) {
	// These are not active because CheckIncreaseSkill() already does so.
	// It's such a rare occurance that adding them here is wasted..(ref only)
	/*
	if (IsDead() || IsUnconscious())
		return;
	if (IsAIControlled())
		return;
	*/

	switch(spells[spell_id].skill) {
	case EQ::skills::SkillAbjuration:
		CheckIncreaseSkill(EQ::skills::SkillSpecializeAbjure, nullptr);
		break;
	case EQ::skills::SkillAlteration:
		CheckIncreaseSkill(EQ::skills::SkillSpecializeAlteration, nullptr);
		break;
	case EQ::skills::SkillConjuration:
		CheckIncreaseSkill(EQ::skills::SkillSpecializeConjuration, nullptr);
		break;
	case EQ::skills::SkillDivination:
		CheckIncreaseSkill(EQ::skills::SkillSpecializeDivination, nullptr);
		break;
	case EQ::skills::SkillEvocation:
		CheckIncreaseSkill(EQ::skills::SkillSpecializeEvocation, nullptr);
		break;
	default:
		//wtf...
		break;
	}
}

void Client::CheckSongSkillIncrease(uint16 spell_id){
	// These are not active because CheckIncreaseSkill() already does so.
	// It's such a rare occurance that adding them here is wasted..(ref only)
	/*
	if (IsDead() || IsUnconscious())
		return;
	if (IsAIControlled())
		return;
	*/

	switch(spells[spell_id].skill)
	{
	case EQ::skills::SkillSinging:
		CheckIncreaseSkill(EQ::skills::SkillSinging, nullptr, -15);
		break;
	case EQ::skills::SkillPercussionInstruments:
		if(itembonuses.percussionMod > 0) {
			if (GetRawSkill(EQ::skills::SkillPercussionInstruments) > 0)	// no skill increases if not trained in the instrument
				CheckIncreaseSkill(EQ::skills::SkillPercussionInstruments, nullptr, -15);
			else
				MessageString(Chat::Red,NO_INSTRUMENT_SKILL);	// tell the client that they need instrument training
		}
		else
			CheckIncreaseSkill(EQ::skills::SkillSinging, nullptr, -15);
		break;
	case EQ::skills::SkillStringedInstruments:
		if(itembonuses.stringedMod > 0) {
			if (GetRawSkill(EQ::skills::SkillStringedInstruments) > 0)
				CheckIncreaseSkill(EQ::skills::SkillStringedInstruments, nullptr, -15);
			else
				MessageString(Chat::Red,NO_INSTRUMENT_SKILL);
		}
		else
			CheckIncreaseSkill(EQ::skills::SkillSinging, nullptr, -15);
		break;
	case EQ::skills::SkillWindInstruments:
		if(itembonuses.windMod > 0) {
			if (GetRawSkill(EQ::skills::SkillWindInstruments) > 0)
				CheckIncreaseSkill(EQ::skills::SkillWindInstruments, nullptr, -15);
			else
				MessageString(Chat::Red,NO_INSTRUMENT_SKILL);
		}
		else
			CheckIncreaseSkill(EQ::skills::SkillSinging, nullptr, -15);
		break;
	case EQ::skills::SkillBrassInstruments:
		if(itembonuses.brassMod > 0) {
			if (GetRawSkill(EQ::skills::SkillBrassInstruments) > 0)
				CheckIncreaseSkill(EQ::skills::SkillBrassInstruments, nullptr, -15);
			else
				MessageString(Chat::Red,NO_INSTRUMENT_SKILL);
		}
		else
			CheckIncreaseSkill(EQ::skills::SkillSinging, nullptr, -15);
		break;
	default:
		break;
	}
}

/*
returns true if spell is successful, false if it fizzled.
only works for clients, npcs shouldn't be fizzling..
new algorithm thats closer to live eq (i hope)
TODO: Add aa skills, item mods, reduced the chance to fizzle
*/
bool Mob::CheckFizzle(uint16 spell_id)
{
	return(true);
}

bool Client::CheckFizzle(uint16 spell_id)
{
	// GMs don't fizzle
	if (GetGM()) {
		Message(Chat::White, "Your GM flag prevents you from fizzling.");
		return true;
	}

	uint8 no_fizzle_level = 0;

	//Live AA - Spell Casting Expertise, Mastery of the Past
	no_fizzle_level = aabonuses.MasteryofPast + itembonuses.MasteryofPast + spellbonuses.MasteryofPast;

	if (spells[spell_id].classes[GetClass()-1] < no_fizzle_level) {
		return true;
	}

	if (RuleB(Spells, UseLegacyFizzleCode)) {
		// CALCULATE SPELL DIFFICULTY - THIS IS CAPPED AT 255
		// calculates minimum level this spell is available - ensures similar casting difficulty for all classes

		int minimum_level = UINT8_MAX;
		for (int a = 0; a < Class::PLAYER_CLASS_COUNT; a++) {
			int this_lvl = spells[spell_id].classes[a];
			if (this_lvl < minimum_level) {
				minimum_level = this_lvl;
			}
		}

		int spell_difficulty = (minimum_level * 5 < UINT8_MAX) ? minimum_level * 5 : UINT8_MAX;

		// CALCULATE EFFECTIVE CASTING SKILL WITH BONUSES
		int bonus_casting_level = itembonuses.adjusted_casting_skill + spellbonuses.adjusted_casting_skill + aabonuses.adjusted_casting_skill;
		int caster_skill = GetSkill(spells[spell_id].skill) + bonus_casting_level * 5;
		caster_skill = (caster_skill < UINT8_MAX) ? caster_skill : UINT8_MAX;

		LogSpellsDetail("Caster Skill - itembonus.ACS(112) [{}] + spellbonus.ACS(112) [{}] + aabonus.ACS(112) [{}] = TotalBonusCastingLevel [{}] | caster_skill [{}] (Max 255)", itembonuses.adjusted_casting_skill, spellbonuses.adjusted_casting_skill, aabonuses.adjusted_casting_skill, bonus_casting_level, caster_skill);

		// CALCULATE EFFECTIVE SPECIALIZATION SKILL VALUE
		float specialize_skill_value = GetSpecializeSkillValue(spell_id);
		switch (GetAA(aaSpellCastingMastery)) {
			case 1:
				specialize_skill_value = specialize_skill_value * 1.05;
				break;
			case 2:
				specialize_skill_value = specialize_skill_value * 1.15;
				break;
			case 3:
				specialize_skill_value = specialize_skill_value * 1.3;
				break;
		}

		float specialize_reduction = (specialize_skill_value > 50) ? (specialize_skill_value - 50) / 10 : 0.0f;

		// CALCULATE EFFECTIVE CASTING STAT VALUE
		float prime_stat_reduction = 0.0f;

		if (IsIntelligenceCasterClass()) {
			prime_stat_reduction = (GetINT() - 75) / 10.0;
		} else if (IsWisdomCasterClass()) {
			prime_stat_reduction = (GetWIS() - 75) / 10.0;
		}

		// BARDS ARE SPECIAL - they add both CHA and DEX mods to get casting rates similar to full casters without spec skill
		if (GetClass() == Class::Bard) {
			prime_stat_reduction = (GetCHA() - 75 + GetDEX() - 75) / 10.0;
		}

		// GET SPELL-SPECIFIC FIZZLE CHANCE (note that specialization is only used to reduce the Fizzle_adjust!)
		float spell_fizzle_adjust = static_cast<float>(spells[spell_id].base_difficulty);
		spell_fizzle_adjust = (spell_fizzle_adjust - specialize_reduction > 0) ? spell_fizzle_adjust - specialize_reduction : 0.0f;

		// CALCULATE FINAL FIZZLE CHANCE
		float fizzle_chance = spell_difficulty + spell_fizzle_adjust - caster_skill - prime_stat_reduction;

		if (fizzle_chance > 95.0f) {
			fizzle_chance = 95.0f;
		} else if (fizzle_chance < 2.0f) {
			fizzle_chance = 2.0f;
		}

		float fizzle_roll = zone->random.Real(0, 100);

		LogSpells("Check Fizzle [{}]  spell: [{}]  fizzle_chance: [{}]  roll: [{}]", GetName(), spell_id, fizzle_chance, fizzle_roll);

		return fizzle_roll > fizzle_chance;
	}

	//is there any sort of focus that affects fizzling?

	int par_skill;
	int act_skill;

	par_skill = spells[spell_id].classes[GetClass()-1] * 5 - 10;//IIRC even if you are lagging behind the skill levels you don't fizzle much
	if (par_skill > 235) {
		par_skill = 235;
	}

	par_skill += spells[spell_id].classes[GetClass()-1]; // maximum of 270 for level 65 spell

	act_skill = GetSkill(spells[spell_id].skill);
	act_skill += GetLevel(); // maximum of whatever the client can cheat
	act_skill += itembonuses.adjusted_casting_skill + spellbonuses.adjusted_casting_skill + aabonuses.adjusted_casting_skill;
	LogSpellsDetail("Adjusted casting skill: [{}]+[{}]+[{}]+[{}]+[{}]=[{}]", GetSkill(spells[spell_id].skill), GetLevel(), itembonuses.adjusted_casting_skill, spellbonuses.adjusted_casting_skill, aabonuses.adjusted_casting_skill, act_skill);

	//spell specialization
	float specialize = GetSpecializeSkillValue(spell_id);
	if (specialize > 0) {
		switch (GetAA(aaSpellCastingMastery)) {
		case 1:
			specialize = specialize * 1.05;
			break;
		case 2:
			specialize = specialize * 1.15;
			break;
		case 3:
			specialize = specialize * 1.3;
			break;
		}
		if (((specialize / 6.0f) + 15.0f) < zone->random.Real(0, 100)) {
			specialize *= SPECIALIZE_FIZZLE / 200.0f;
		} else {
			specialize = 0.0f;
		}
	}

	// == 0 --> on par
	// > 0 --> skill is lower, higher chance of fizzle
	// < 0 --> skill is better, lower chance of fizzle
	// the max that diff can be is +- 235
	float diff = par_skill + static_cast<float>(spells[spell_id].base_difficulty) - act_skill;

	// if you have high int/wis you fizzle less, you fizzle more if you are stupid
	if (GetClass() == Class::Bard) {
		diff -= (GetCHA() - 110) / 20.0;
	} else if (IsIntelligenceCasterClass()) {
		diff -= (GetINT() - 125) / 20.0;
	} else if (IsWisdomCasterClass()) {
		diff -= (GetWIS() - 125) / 20.0;
	}

	// base fizzlechance is lets say 5%, we can make it lower for AA skills or whatever
	float base_fizzle = 10;
	float fizzle_chance = base_fizzle - specialize + diff / 5.0;

	// always at least 1% chance to fail or 5% to succeed
	fizzle_chance = fizzle_chance < 1 ? 1 : (fizzle_chance > 95 ? 95 : fizzle_chance);

	float fizzle_roll = zone->random.Real(0, 100);

	LogSpells("Check Fizzle [{}] spell [{}] fizzlechance: [{}]  diff: [{}] roll: [{}]", GetName(), spell_id, fizzle_chance, diff, fizzle_roll);

	return fizzle_roll > fizzle_chance;
}

void Mob::ZeroCastingVars()
{
	// zero out the state keeping vars
	attacked_count = 0;
	spellend_timer.Disable();
	casting_spell_id = 0;
	casting_spell_targetid = 0;
	casting_spell_slot = CastingSlot::Gem1;
	casting_spell_mana = 0;
	casting_spell_inventory_slot = 0;
	casting_spell_timer = 0;
	casting_spell_timer_duration = 0;
	casting_spell_resist_adjust = 0;
	casting_spell_checks = false;
	casting_spell_aa_id = 0;
	casting_spell_recast_adjust = 0;
	delaytimer = false;
}


//This will cause server to stop trying to pulse a bard song. Does not stop song clientside.
void Mob::ZeroBardPulseVars()
{
	bardsong = 0;
	bardsong_target_id = 0;
	bardsong_slot = CastingSlot::Gem1;
	bardsong_timer.Disable();
}

void Mob::InterruptSpell(uint16 spellid)
{
	if (!IsValidSpell(spellid)) {
		spellid = casting_spell_id;
	}

	InterruptSpell(0, 0x121, spellid);
}

// color not used right now
void Mob::InterruptSpell(uint16 message, uint16 color, uint16 spellid)
{
	EQApplicationPacket *outapp = nullptr;
	uint16 message_other;
	bool bard_song_mode = false; //has the bard song gone to auto repeat mode

	if (IsBot()) {
		auto bot = CastToBot();
		bot->SetCastedSpellType(UINT16_MAX);

		if (IsValidSpell(spellid) && bot->CheckSpellRecastTimer(spellid)) {
			bot->ClearSpellRecastTimer(spellid);
		}
	}

	if (!IsValidSpell(spellid)) {
		if (bardsong) {
			spellid = bardsong;
			bard_song_mode = true;
		}
		else {
			if (IsBot() && !message && !color && !spellid) { // this is to prevent bots from spamming interrupts messages when trying to cast while OOM
				ZeroCastingVars();	// resets all the state keeping stuff
				LogSpells("Spell [{}] has been interrupted - Bot [{}] doesn't have enough mana", spellid, GetCleanName());
				return;
			}
			spellid = casting_spell_id;
		}
	}

	LogSpells("Interrupt: casting_spell_id [{}] casting_spell_slot [{}]",
		casting_spell_id, (int) casting_spell_slot);

	if(casting_spell_id && IsNPC()) {
		CastToNPC()->AI_Event_SpellCastFinished(false, static_cast<uint16>(casting_spell_slot));
	}

	if(casting_spell_aa_id && IsClient()) { //Rest AA Timer on failed cast
		CastToClient()->MessageString(Chat::SpellFailure, ABILITY_FAILED);
		CastToClient()->ResetAlternateAdvancementTimer(casting_spell_aa_id);
	}

	ZeroCastingVars();	// resets all the state keeping stuff

	LogSpells("Spell [{}] has been interrupted", spellid);

	if(!spellid)
		return;

	if (bardsong || IsBardSong(casting_spell_id)) {
		ZeroBardPulseVars();
	}

	if(bard_song_mode) {
		return;
	}

	if(!message)
		message = IsBardSong(spellid) ? SONG_ENDS_ABRUPTLY : INTERRUPT_SPELL;

	// clients need some packets
	if (IsClient() && message != SONG_ENDS)
	{
		// the interrupt message
		outapp = new EQApplicationPacket(OP_InterruptCast, sizeof(InterruptCast_Struct));
		InterruptCast_Struct* ic = (InterruptCast_Struct*) outapp->pBuffer;
		ic->messageid = message;
		ic->spawnid = GetID();
		outapp->priority = 5;
		CastToClient()->QueuePacket(outapp);
		safe_delete(outapp);

		SendSpellBarEnable(spellid);
	}

	// notify people in the area

	// first figure out what message others should get
	switch(message)
	{
		case SONG_ENDS:
			message_other = SONG_ENDS_OTHER;
			break;
		case SONG_ENDS_ABRUPTLY:
			message_other = SONG_ENDS_ABRUPTLY_OTHER;
			break;
		case MISS_NOTE:
			message_other = MISSED_NOTE_OTHER;
			break;
		case SPELL_FIZZLE:
			message_other = SPELL_FIZZLE_OTHER;
			break;
		default:
			message_other = INTERRUPT_SPELL_OTHER;
	}

	// this is the actual message, it works the same as a formatted message
	outapp = new EQApplicationPacket(OP_InterruptCast, sizeof(InterruptCast_Struct) + strlen(GetCleanName()) + 1);
	InterruptCast_Struct* ic = (InterruptCast_Struct*) outapp->pBuffer;
	ic->messageid = message_other;
	ic->spawnid = GetID();
	strcpy(ic->message, GetCleanName());
	entity_list.QueueCloseClients(this, outapp, true, RuleI(Range, SongMessages), 0, true, IsClient() ? FilterPCSpells : FilterNPCSpells);
	safe_delete(outapp);

}

// this is like interrupt, just it doesn't spam interrupt packets to everyone
// There are a few cases where this is what live does :P
void Mob::StopCasting()
{
	if (casting_spell_id && IsNPC()) {
		CastToNPC()->AI_Event_SpellCastFinished(false, static_cast<uint16>(casting_spell_slot));
	}

	if (IsClient()) {
		auto c = CastToClient();
		if (casting_spell_aa_id) { //Rest AA Timer on failed cast
			c->MessageString(Chat::SpellFailure, ABILITY_FAILED);
			c->ResetAlternateAdvancementTimer(casting_spell_aa_id);
		}

		int casting_slot = -1;
		if (casting_spell_slot < CastingSlot::MaxGems) {
			casting_slot = static_cast<int>(casting_spell_slot);
		}

		auto outapp = new EQApplicationPacket(OP_ManaChange, sizeof(ManaChange_Struct));
		auto mc = (ManaChange_Struct *)outapp->pBuffer;
		mc->new_mana = GetMana();
		mc->stamina = GetEndurance();
		mc->spell_id = casting_spell_id;
		mc->keepcasting = 0;
		mc->slot = casting_slot;
		c->FastQueuePacket(&outapp);
	}
	ZeroCastingVars();
}

void Mob::StopCastSpell(int32 spell_id, bool send_spellbar_enable)
{
	/*
		This is used when spells fail at CastSpell or when CastSpell is bypassed and spell is launched initially from SpellFinished.
		send_spellbar_enabled is false when the following
		-AA that fail at CastSpell because they never get timer set.
		-Instant cast items that fail at CastSpell because they never get timer set.
	*/
	// Often called before spell_id and slot are set.  For NPCs always update AI
	if (IsNPC()) {
		CastToNPC()->AI_Event_SpellCastFinished(false, 1);
	}

	if (send_spellbar_enable) {
		SendSpellBarEnable(spell_id);
	}
}

// this is called after the timer is up and the spell is finished
// casting. everything goes through here, including items with zero cast time
// only to be used from SpellProcess
// NOTE: do not put range checking, etc into this function. this should
// just check timed spell specific things before passing off to SpellFinished
// which figures out proper targets etc
void Mob::CastedSpellFinished(uint16 spell_id, uint32 target_id, CastingSlot slot,
							int32  mana_used, uint32 inventory_slot, int16 resist_adjust)
{
	if (!IsValidSpell(spell_id))
	{
		LogSpells("Casting of [{}] canceled: invalid spell id", spell_id);
		InterruptSpell();
		return;
	}

	bool IsFromItem = false;
	EQ::ItemInstance *item = nullptr;

	if(IsClient() && slot != CastingSlot::Item && slot != CastingSlot::PotionBelt && spells[spell_id].recast_time > 1000) { // 10 is item
		if(!CastToClient()->GetPTimers().Expired(&database, pTimerSpellStart + spell_id, false)) {
			//should we issue a message or send them a spell gem packet?
			MessageString(Chat::Red, SPELL_RECAST);
			LogSpells("Casting of [{}] canceled: spell reuse timer not expired", spell_id);
			StopCasting();
			return;
		}
	}
	/*
		Reinforcement only. Checks Item and Augment click recasts.
		Titanium client will prevent item recast on its own. This is only used to enforce. Titanium items are cast from Handle_OP_CastSpell.
		SOF+ client does not prevent item recast on its own. We enforce this in Handle_OP_ItemVerifyRequest where items are cast from.
	*/
	if(IsClient() && (slot == CastingSlot::Item || slot == CastingSlot::PotionBelt))
	{
		IsFromItem = true;
		item  = CastToClient()->GetInv().GetItem(inventory_slot); //checked for in reagents and charges.
		if (CastToClient()->HasItemRecastTimer(spell_id, inventory_slot)) {
			MessageString(Chat::Red, SPELL_RECAST);
			LogSpells("Casting of [{}] canceled: item or augment spell reuse timer not expired", spell_id);
			StopCasting();
			return;
		}
	}

	// prevent rapid recast - this can happen if somehow the spell gems
	// become desynced and the player casts again.
	if(IsClient())
	{
		if(delaytimer)
		{
			LogSpells("Casting of [{}] canceled: recast too quickly", spell_id);
			Message(Chat::Red, "You are unable to focus.");
			InterruptSpell();
			return;
		}
	}

	// make sure they aren't somehow casting 2 timed spells at once
	if (casting_spell_id != spell_id)
	{
		LogSpells("Casting of [{}] canceled: already casting", spell_id);
		MessageString(Chat::Red,ALREADY_CASTING);
		InterruptSpell();
		return;
	}

	bool bard_song_mode = false;
	bool regain_conc = false;
	Mob *spell_target = entity_list.GetMob(target_id);
	// here we do different things if this is a bard casting a bard song from
	// a spell bar slot
	if(GetClass() == Class::Bard) // bard's can move when casting any spell...
	{
		if (IsBardSong(spell_id) && slot < CastingSlot::MaxGems) {
			if (spells[spell_id].buff_duration == 0xFFFF) {
				LogSpells("Bard song [{}] not applying bard logic because duration. dur=[{}], recast=[{}]", spell_id, spells[spell_id].buff_duration, spells[spell_id].recast_time);
			}
			else {
				if (IsPulsingBardSong(spell_id)) {
					bardsong = spell_id;
					bardsong_slot = slot;

					if (spell_target) {
						bardsong_target_id = spell_target->GetID();
					}
					else if (spells[spell_id].target_type != ST_Target && spells[spell_id].target_type != ST_AETarget) {
						bardsong_target_id = GetID(); //This is a failsafe, you should always have a spell_target unless that target died/zoned.
					}
					else {
						InterruptSpell();
					}
					bardsong_timer.Start(6000);
				}
				LogSpells("Bard song [{}] started: slot [{}], target id [{}]", bardsong, (int)bardsong_slot, bardsong_target_id);
				bard_song_mode = true;
			}
		}
	}
	else // not bard, check movement
	{
		// if has been attacked, or moved while casting
		// check for regain concentration
		if
		(
			attacked_count > 0 ||
			GetX() != GetSpellX() ||
			GetY() != GetSpellY()
		)
		{
			// modify the chance based on how many times they were hit
			// but cap it so it's not that large a factor
			if(attacked_count > 15) attacked_count = 15;

			float channelchance, distance_moved, d_x, d_y, distancemod;

			if (IsOfClientBot()) {
				float channelbonuses = 0.0f;
				//AA that effect Spell channel chance are no longer on live. http://everquest.allakhazam.com/history/patches-2006-2.html
				//No harm in maintaining the effects regardless, since we do check for channel chance.
				channelbonuses += IsFromItem ?
						spellbonuses.ChannelChanceItems + itembonuses.ChannelChanceItems + aabonuses.ChannelChanceItems :
						spellbonuses.ChannelChanceSpells + itembonuses.ChannelChanceSpells + aabonuses.ChannelChanceSpells;
				// max 93% chance at 252 skill
				channelchance = 30 + GetSkill(EQ::skills::SkillChanneling) / 400.0f * 100;
				channelchance -= attacked_count * 2;
				channelchance += channelchance * channelbonuses / 100.0f;
			} else {
				// NPCs are just hard to interrupt, otherwise they get pwned
				channelchance = 85;
				channelchance -= attacked_count;
			}

			// as you get farther from your casting location,
			// it gets squarely harder to regain concentration
			if(GetX() != GetSpellX() || GetY() != GetSpellY())
			{
				d_x = std::abs(std::abs(GetX()) - std::abs(GetSpellX()));
				d_y = std::abs(std::abs(GetY()) - std::abs(GetSpellY()));
				if(d_x < 5 && d_y < 5)
				{
					//avoid the square root...
					distance_moved = d_x * d_x + d_y * d_y;
					// if you moved 1 unit, that's 25% off your chance to regain.
					// if you moved 2, you lose 100% off your chance
					distancemod = distance_moved * 25;
					channelchance -= distancemod;
				}
				else
				{
					channelchance = 0;
				}
			}

			LogSpells("Checking Interruption: spell x: [{}] spell y: [{}] cur x: [{}] cur y: [{}] channelchance [{}] channeling skill [{}]\n", GetSpellX(), GetSpellY(), GetX(), GetY(), channelchance, GetSkill(EQ::skills::SkillChanneling));

			if(!spells[spell_id].uninterruptable && zone->random.Real(0, 100) > channelchance) {
				LogSpells("Casting of [{}] canceled: interrupted", spell_id);
				InterruptSpell();
				return;
			}
			// if we got here, we regained concentration
			regain_conc = true;
			MessageString(Chat::Spells, REGAIN_AND_CONTINUE);
			entity_list.MessageCloseString(
				this,
				true,
				RuleI(Range, SpellMessages),
				Chat::Spells,
				OTHER_REGAIN_CAST,
				GetCleanName());
		}
	}

	// Check for consumables and Reagent focus items
	// first check for component reduction
	if(IsClient()) {
		int reg_focus = CastToClient()->GetFocusEffect(focusReagentCost,spell_id);//Client only
		/* it seems something causes some items not to consume reagents, it's not click type or temp flag
		 * it maybe cast time being instant, which I had a hard time disproving, so lets do that
		 * Items that might prove this wrong: Mystic Cloak (1057), Moss Mask (1400), and a bunch others
		 */
		if (item && item->GetItem() && item->GetItem()->CastTime == 0) {
			LogSpells("Spell [{}]: Casted from instant clicky, prevent reagent consumption", spell_id);
		} else if(zone->random.Roll(reg_focus)) {
			LogSpells("Spell [{}]: Reagent focus item prevented reagent consumption ([{}] chance)", spell_id, reg_focus);
		} else {
			if(reg_focus > 0)
				LogSpells("Spell [{}]: Reagent focus item failed to prevent reagent consumption ([{}] chance)", spell_id, reg_focus);
			Client *c = CastToClient();
			int component, component_count, inv_slot_id;
			bool missingreags = false;
			for(int t_count = 0; t_count < 4; t_count++) {
				component = spells[spell_id].component[t_count];
				component_count = spells[spell_id].component_count[t_count];

				if (component == -1)
					continue;

				// bard components are requirements for a certain instrument type, not a specific item
				if(bard_song_mode) {
					bool HasInstrument = true;
					int InstComponent = spells[spell_id].no_expend_reagent[0];

					switch (InstComponent) {
						case -1:
							continue;		// no instrument required, go to next component

						// percussion songs (13000 = hand drum)
						case INSTRUMENT_HAND_DRUM:
							if(itembonuses.percussionMod == 0) {			// check for the appropriate instrument type
								HasInstrument = false;
								c->MessageString(Chat::Red, SONG_NEEDS_DRUM);	// send an error message if missing
							}
							break;

						// wind songs (13001 = wooden flute)
						case INSTRUMENT_WOODEN_FLUTE:
							if(itembonuses.windMod == 0) {
								HasInstrument = false;
								c->MessageString(Chat::Red, SONG_NEEDS_WIND);
							}
							break;

						// string songs (13011 = lute)
						case INSTRUMENT_LUTE:
							if(itembonuses.stringedMod == 0) {
								HasInstrument = false;
								c->MessageString(Chat::Red, SONG_NEEDS_STRINGS);
							}
							break;

						// brass songs (13012 = horn)
						case INSTRUMENT_HORN:
							if(itembonuses.brassMod == 0) {
								HasInstrument = false;
								c->MessageString(Chat::Red, SONG_NEEDS_BRASS);
							}
							break;

						default:	// some non-instrument component. Let it go, but record it in the log
							LogSpells("Something odd happened: Song [{}] required component [{}]", spell_id, component);
					}

					if(!HasInstrument) {	// if the instrument is missing, log it and interrupt the song
						LogSpells("Song [{}]: Canceled. Missing required instrument [{}]", spell_id, component);
						if (c->GetGM()) {
							c->Message(
								Chat::White,
								"Your GM flag allows you to finish casting even though you're missing a required instrument."
							);
						} else {
							InterruptSpell();
							return;
						}
					}
				}	// end bard component section

				// handle the components for traditional casters
				else {
					if (!RuleB(Character, PetsUseReagents) && (IsEffectInSpell(spell_id, SpellEffect::SummonPet) || IsEffectInSpell(spell_id, SpellEffect::NecPet)) ||
						(IsBardSong(spell_id) && (slot == CastingSlot::Item|| slot == CastingSlot::PotionBelt))) {
						//bypass reagent cost
					}
					else if(c->GetInv().HasItem(component, component_count, invWhereWorn|invWherePersonal) == -1) // item not found
					{
						if (!missingreags)
						{
							c->MessageString(Chat::Red, MISSING_SPELL_COMP);
							missingreags=true;
						}

						const EQ::ItemData *item = database.GetItem(component);
						if(item) {
							c->MessageString(Chat::Red, MISSING_SPELL_COMP_ITEM, item->Name);
							LogSpells("Spell [{}]: Canceled. Missing required reagent [{}] ([{}])", spell_id, item->Name, component);
						}
						else {
							char TempItemName[64];
							strcpy((char*)&TempItemName, "UNKNOWN");
							LogSpells("Spell [{}]: Canceled. Missing required reagent [{}] ([{}])", spell_id, TempItemName, component);
						}
					}
				} // end bard/not bard ifs
			} // end reagent loop

			if (missingreags) {
				if (c->GetGM()) {
					c->Message(
						Chat::White,
						"Your GM flag allows you to finish casting even though you're missing required components."
					);
				} else {
					InterruptSpell();
					return;
				}
			}
			else if (!RuleB(Character, PetsUseReagents) && (IsEffectInSpell(spell_id, SpellEffect::SummonPet) || IsEffectInSpell(spell_id, SpellEffect::NecPet))) {
				//bypass reagent cost
			}
			else if (!bard_song_mode)
			{
				int noexpend;
				for(int t_count = 0; t_count < 4; t_count++) {
					component = spells[spell_id].component[t_count];
					noexpend = spells[spell_id].no_expend_reagent[t_count];
					if (component == -1 || noexpend == component)
						continue;
					component_count = spells[spell_id].component_count[t_count];
					LogSpells("Spell [{}]: Consuming [{}] of spell component item id [{}]", spell_id, component_count, component);
					// Components found, Deleting
					// now we go looking for and deleting the items one by one
					for(int s = 0; s < component_count; s++)
					{
						inv_slot_id = c->GetInv().HasItem(component, 1, invWhereWorn|invWherePersonal);
						if(inv_slot_id != -1)
						{
							c->DeleteItemInInventory(inv_slot_id, 1, true);
						}
						else
						{	// some kind of error in the code if this happens
							c->Message(Chat::Red, "ERROR: reagent item disappeared while processing?");
						}
					}
				}
				} // end missingreags/consumption
			} // end `focus did not help us`
	} // end IsClient() for reagents

	// this is common to both bard and non bard

	// if this was cast from an inventory slot, check out the item that's there

	int16 DeleteChargeFromSlot = -1;

	if(IsClient() && (slot == CastingSlot::Item || slot == CastingSlot::PotionBelt)
		&& inventory_slot != 0xFFFFFFFF)	// 10 is an item
	{
		DeleteChargeFromSlot = GetItemSlotToConsumeCharge(spell_id, inventory_slot);
	}
	if (IsBot() && slot == CastingSlot::Item && inventory_slot != 0xFFFFFFFF)	// 10 is an item
	{
		DeleteChargeFromSlot = GetItemSlotToConsumeCharge(spell_id, inventory_slot);
	}
	// we're done casting, now try to apply the spell
	if(!SpellFinished(spell_id, spell_target, slot, mana_used, inventory_slot, resist_adjust, false,-1, 0xFFFFFFFF, 0, true))
	{
		LogSpells("Casting of [{}] canceled: SpellFinished returned false", spell_id);
		// most of the cases we return false have a message already or are logic errors that shouldn't happen
		// if there are issues I guess we can do something else, but this should work
		StopCasting();
		return;
	}

	if(IsOfClientBotMerc()) {
		TrySympatheticProc(target, spell_id);
	}

	TryTwincast(this, target, spell_id);

	TryTriggerOnCastFocusEffect(focusTriggerOnCast, spell_id);

	if (IsClient() && DeleteChargeFromSlot >= 0) {
		CastToClient()->DeleteItemInInventory(DeleteChargeFromSlot, 1, true);
	}
	else if (IsBot() && DeleteChargeFromSlot >= 0) {
		EQ::ItemInstance* inst = CastToBot()->GetBotItem(DeleteChargeFromSlot);
		if (inst) {
			inst->SetCharges((inst->GetCharges() - 1));
			if (!database.botdb.SaveItemBySlot(CastToBot(), DeleteChargeFromSlot, inst)) {
				GetOwner()->Message(Chat::Red, "%s says, 'Failed to save item [%i] slot [%i] for [%s].", inst->GetID(), DeleteChargeFromSlot, GetCleanName());
				return;
			}
		}
		else {
			GetOwner()->Message(Chat::Red, "%s says, 'Failed to update item charges.", GetCleanName());
			LogError("Failed to update item charges for {}.", GetCleanName());
		}
	}

	std::vector<std::any> args = { spell_target };

	parse->EventMob(EVENT_CAST, this, nullptr, [&]() {
		return fmt::format(
			"{} {} {} {}",
			spell_id,
			GetID(),
			GetCasterLevel(spell_id),
			target_id
		);
	}, 0, &args);

	if(bard_song_mode)
	{
		if(IsClient())
		{
			Client *c = CastToClient();
			if((IsFromItem  && RuleB(Character, SkillUpFromItems)) || !IsFromItem) {
				c->CheckSongSkillIncrease(spell_id);
			}
			if (spells[spell_id].timer_id > 0 && slot < CastingSlot::MaxGems) {
				c->SetLinkedSpellReuseTimer(spells[spell_id].timer_id, (spells[spell_id].recast_time / 1000) - (casting_spell_recast_adjust / 1000));
			}
			if (RuleB(Spells, EnableBardMelody)) {
				c->MemorizeSpell(static_cast<uint32>(slot), spell_id, memSpellSpellbar, casting_spell_recast_adjust);
			}

			if (!IsFromItem) {
				c->CheckSongSkillIncrease(spell_id);
			}
		}
		LogSpells("Bard song [{}] should be started", spell_id);
	}
	else
	{
		if(IsClient())
		{
			Client *c = CastToClient();
			SendSpellBarEnable(spell_id);

			// this causes the delayed refresh of the spell bar gems
			if (spells[spell_id].timer_id > 0 && slot < CastingSlot::MaxGems) {
				c->SetLinkedSpellReuseTimer(spells[spell_id].timer_id, (spells[spell_id].recast_time / 1000) - (casting_spell_recast_adjust / 1000));
			}

			c->MemorizeSpell(static_cast<uint32>(slot), spell_id, memSpellSpellbar, casting_spell_recast_adjust);

			// this tells the client that casting may happen again
			SetMana(GetMana());

			// skills
			if (EQ::skills::IsCastingSkill(spells[spell_id].skill) && ((IsFromItem  && RuleB(Character, SkillUpFromItems)) || !IsFromItem)) {
				c->CheckIncreaseSkill(spells[spell_id].skill, nullptr);

				// increased chance of gaining channel skill if you regained concentration
				c->CheckIncreaseSkill(EQ::skills::SkillChanneling, nullptr, regain_conc ? 5 : 0);

				c->CheckSpecializeIncrease(spell_id);
			}
		}
	}

	// there should be no casting going on now
	ZeroCastingVars();

	// set the rapid recast timer for next time around
	// Why do we have this? It mostly just causes issues when things are working correctly
	// It also needs to be <users's ping to not cause issues
	delaytimer = true;
	spellend_timer.Start(10, true);

	LogSpells("Spell casting of [{}] is finished", spell_id);

}

bool Mob::DetermineSpellTargets(uint16 spell_id, Mob *&spell_target, Mob *&ae_center, CastAction_type &CastAction, CastingSlot slot, bool isproc)
{
/*
	The basic types of spells:

	Single target - some might be undead only, self only, etc, but these
	all affect the target of the caster.

	AE around caster - these affect entities close to the caster, and have
	no target.

	AE around target - these have a target, and affect the target as well as
	entities close to the target.

	AE on location - this is a tricky one that is cast on a mob target but
	has a special AE duration that keeps it recasting every 2.5 sec on the
	same location. These work the same as AE around target spells, except
	the target is a special beacon that's created when the spell is cast

	Group - the caster is always affected, but there's more
		targetgroupbuffs on - these affect the target and the target's group.
		targetgroupbuffs off - no target, affects the caster's group.

	Group Teleport - the caster plus his group are affected. these cannot
	be targeted.

	I think the string ID SPELL_NEED_TAR is wrong, it dosent seem to show up.
*/

	// during this switch, this variable gets set to one of these things
	// and that causes the spell to be executed differently

	uint8 target_bt = BodyType::Humanoid;
	SpellTargetType targetType = spells[spell_id].target_type;
	uint8 mob_body = spell_target ? spell_target->GetBodyType() : BodyType::Humanoid;

	if(IsIllusionSpell(spell_id)
		&& spell_target != nullptr // null ptr crash safeguard
		&& !spell_target->IsNPC() // still self only if NPC targetted
		&& IsOfClientBot()
		&& (IsGrouped() // still self only if not grouped
		|| IsRaidGrouped())
		&& (HasProjectIllusion())){
			LogAA("Project Illusion overwrote target caster: [{}] spell id: [{}] was ON", GetName(), spell_id);
			targetType = ST_GroupClientAndPet;
	}

	// NPC innate procs override the target type to single target.
	// Yes. This code will cause issues if they have the proc as innate AND on a weapon. Oh well.
	if (isproc && IsNPC() && CastToNPC()->GetInnateProcSpellID() == spell_id)
		targetType = ST_Target;

	switch (targetType)
	{
// single target spells
		case ST_Self:
		{
			bool bot_can_summon_corpse = IsBot() &&
				IsEffectInSpell(spell_id, SpellEffect::SummonCorpse) &&
				RuleB(Bots, AllowCommandedSummonCorpse);

			if (!bot_can_summon_corpse) {
				spell_target = this; // Summon corpse spells are self-only; bots need a fallthrough
			}

			CastAction = SingleTarget;
			break;
		}

		case ST_TargetOptional:
		{
			if (!spell_target)
			{
				LogSpells("Spell [{}] canceled: invalid target (normal)", spell_id);
				MessageString(Chat::Red, SPELL_NEED_TAR);
				return false;	// can't cast these unless we have a target
			}
			CastAction = SingleTarget;
			break;
		}

		// target required for these
		case ST_Undead: {
			if(!spell_target || (
				mob_body != BodyType::SummonedUndead
				&& mob_body != BodyType::Undead
				&& mob_body != BodyType::Vampire
				)
			)
			{
				//invalid target
				LogSpells("Spell [{}] canceled: invalid target of body type [{}] (undead)", spell_id, mob_body);
				if(!spell_target)
					MessageString(Chat::Red,SPELL_NEED_TAR);
				else
					MessageString(Chat::Red,CANNOT_AFFECT_NPC);
				return false;
			}
			CastAction = SingleTarget;
			break;
		}

		case ST_Summoned: {
			if(!spell_target || (mob_body != BodyType::Summoned && mob_body != BodyType::Summoned2 && mob_body != BodyType::Summoned3))
			{
				//invalid target
				LogSpells("Spell [{}] canceled: invalid target of body type [{}] (summoned)", spell_id, mob_body);
				MessageString(Chat::Red,SPELL_NEED_TAR);
				return false;
			}
			CastAction = SingleTarget;
			break;
		}

		case ST_SummonedPet:
		{
			if(!spell_target || (spell_target != GetPet()) ||
				(mob_body != BodyType::Summoned && mob_body != BodyType::Summoned2 && mob_body != BodyType::Summoned3 && mob_body != BodyType::Animal))
			{
				LogSpells("Spell [{}] canceled: invalid target of body type [{}] (summoned pet)",
							spell_id, mob_body);

				MessageString(Chat::Red, SPELL_NEED_TAR);

				return false;
			}
			CastAction = SingleTarget;
			break;
		}
		//single body type target spells...
		//this is a little hackish, but better than duplicating code IMO
		case ST_Plant: if(target_bt == BodyType::Humanoid) target_bt = BodyType::Plant;
		case ST_Dragon: if(target_bt == BodyType::Humanoid) target_bt = BodyType::Dragon;
		case ST_Giant: if(target_bt == BodyType::Humanoid) target_bt = BodyType::Giant;
		case ST_Animal: if(target_bt == BodyType::Humanoid) target_bt = BodyType::Animal;

		// check for special case body types (Velious dragons/giants)
		if(mob_body == BodyType::RaidGiant) mob_body = BodyType::Giant;
		if(mob_body == BodyType::VeliousDragon) mob_body = BodyType::Dragon;

		{
			if(!spell_target || mob_body != target_bt)
			{
				//invalid target
				LogSpells("Spell [{}] canceled: invalid target of body type [{}] (want body Type [{}])", spell_id, mob_body, target_bt);
				if(!spell_target)
					MessageString(Chat::Red,SPELL_NEED_TAR);
				else
					MessageString(Chat::Red,CANNOT_AFFECT_NPC);
				return false;
			}
			CastAction = SingleTarget;
			break;
		}

		case ST_Tap:
		case ST_LDoNChest_Cursed:
		case ST_Target: {
			if(IsLDoNObjectSpell(spell_id))
			{
				if(!spell_target)
				{
					LogSpells("Spell [{}] canceled: invalid target (ldon object)", spell_id);
					MessageString(Chat::Red,SPELL_NEED_TAR);
					return false;
				}
				else
				{
					if(!spell_target->IsNPC())
					{
						LogSpells("Spell [{}] canceled: invalid target (normal)", spell_id);
						MessageString(Chat::Red,SPELL_NEED_TAR);
						return false;
					}

					if(spell_target->GetClass() != Class::LDoNTreasure)
					{
						LogSpells("Spell [{}] canceled: invalid target (normal)", spell_id);
						MessageString(Chat::Red,SPELL_NEED_TAR);
						return false;
					}
				}
			}

			if(!spell_target)
			{
				LogSpells("Spell [{}] canceled: invalid target (normal)", spell_id);
				MessageString(Chat::Red,SPELL_NEED_TAR);
				return false;	// can't cast these unless we have a target
			}
			CastAction = SingleTarget;
			break;
		}

		case ST_Corpse:
		{
			if(!spell_target || !spell_target->IsPlayerCorpse())
			{
				LogSpells("Spell [{}] canceled: invalid target (corpse)", spell_id);
				uint32 message = ONLY_ON_CORPSES;
				if(!spell_target) message = SPELL_NEED_TAR;
				else if(!spell_target->IsCorpse()) message = ONLY_ON_CORPSES;
				else if(!spell_target->IsPlayerCorpse()) message = CORPSE_NOT_VALID;
				MessageString(Chat::Red, message);
				return false;
			}
			CastAction = SingleTarget;
			break;
		}
		case ST_Pet:
		{
			bool bot_casting_on_other_pet = IsBot() &&
				spell_target &&
				spell_target->GetOwner() != this &&
				RuleB(Bots, CanCastPetOnlyOnOthersPets);

			if (!bot_casting_on_other_pet) {
				spell_target = GetPet();
			}

			if (!spell_target) {
				LogSpells("Spell [{}] canceled: invalid target (no pet)", spell_id);
				MessageString(Chat::Red, NO_PET);
				return false; // Can't cast these unless we have a target
			}

			CastAction = SingleTarget;
			break;
		}

		case ST_AEBard:
		case ST_AECaster:
		case ST_AEClientV1:
		{
			spell_target = nullptr;
			ae_center = this;
			CastAction = AECaster;
			break;
		}

		case ST_HateList:
		{
			spell_target = nullptr;
			ae_center = this;
			CastAction = CAHateList;
			break;
		}

		case ST_AETargetHateList:
		{
			if (spells[spell_id].range > 0)
			{
				if(!spell_target)
					return false;

				ae_center = spell_target;
				CastAction = AETarget;
			}
			else {
				spell_target = nullptr;
				ae_center = this;
				CastAction = CAHateList;
			}
			break;
		}

		case ST_AreaClientOnly:
		case ST_AreaNPCOnly:
		{
			if (spells[spell_id].range > 0)
			{
				if(!spell_target)
					return false;

				ae_center = spell_target;
				CastAction = AETarget;
			}
			else {
				spell_target = nullptr;
				ae_center = this;
				CastAction = AECaster;
			}
			break;
		}

		case ST_UndeadAE:	//should only affect undead...
		case ST_SummonedAE:
		case ST_TargetAETap:
		case ST_AETarget:
		case ST_TargetAENoPlayersPets:
		{
			if(!spell_target)
			{
				LogSpells("Spell [{}] canceled: invalid target (AOE)", spell_id);
				MessageString(Chat::Red,SPELL_NEED_TAR);
				return false;
			}
			ae_center = spell_target;
			CastAction = AETarget;
			break;
		}

		// Group spells
		case ST_GroupTeleport:
		case ST_Group:
		case ST_GroupNoPets:
		{
			bool tgb_enabled = (IsClient() && RuleB(Character, EnableTGB) && CastToClient()->TGB()) ||
							   (IsBot() && RuleB(Bots, EnableBotTGB));
			bool tgb_compatible = IsTGBCompatibleSpell(spell_id);
			bool item_tgb_allowed = (slot != CastingSlot::Item || RuleB(Spells, AllowItemTGB));

			if (tgb_enabled && tgb_compatible && item_tgb_allowed) {
				bool valid_target = spell_target &&
					!spell_target->IsCorpse() &&
					(!spell_target->IsNPC() ||
					(spell_target->GetOwner() && spell_target->IsPetOwnerOfClientBot()));

				if (!valid_target) {
					spell_target = this;
				}
			} else {
				spell_target = this;
			}

			if (spell_target && spell_target->IsPet() && spells[spell_id].target_type == ST_GroupNoPets) {
				MessageString(Chat::Red, NO_CAST_ON_PET);
				return false;
			}

			CastAction = GroupSpell;
			break;
		}
		case ST_GroupClientAndPet:
		{
			if(!spell_target)
			{
				LogSpells("Spell [{}] canceled: invalid target (Group Required: Single Target)", spell_id);
				MessageString(Chat::Red,SPELL_NEED_TAR);
				return false;
			}

			if(spell_target != this)
			{
				if(spell_target == GetPet())
				{
					CastAction = SingleTarget;
				}
				else if(spell_target == GetOwner())
				{
					CastAction = SingleTarget;
				}
				else
				{
					uint32 group_id_caster = 0;
					uint32 group_id_target = 0;
					if(IsClient())
					{
						if(IsGrouped())
						{
							if (Group* group = GetGroup()) {
								group_id_caster = group->GetID();
							}
						}
						else if(IsRaidGrouped())
						{
							if (Raid* raid = GetRaid()) {
								uint32 group_id = raid->GetGroup(CastToClient());
								group_id_caster = (group_id == 0xFFFFFFFF) ? 0 : (group_id + 1);
							}
						}
					}
					else if(IsPet())
					{
						Mob *owner = GetOwner();
						if(owner->IsGrouped())
						{
							if (Group* group = owner->GetGroup()) {
								group_id_caster = group->GetID();
							}
						}
						else if(owner->IsRaidGrouped())
						{
							if (Raid* raid = owner->GetRaid()) {
								uint32 group_id = raid->GetGroup(owner->CastToClient());
								group_id_caster = (group_id == 0xFFFFFFFF) ? 0 : (group_id + 1);
							}
						}
					}
					else if(IsBot())
					{
						if (IsGrouped()) {
							if (auto group = GetGroup()) {
								group_id_caster = group->GetID();
							}
						} else if (IsRaidGrouped()) {
							if (auto raid = GetRaid()) {
								uint32 group_id = raid->GetGroup(GetName());
								group_id_caster = (group_id == 0xFFFFFFFF) ? 0 : (group_id + 1);
							}
						}
					}

					if(spell_target->IsClient())
					{
						if(spell_target->IsGrouped())
						{
							if (Group* group = spell_target->GetGroup()) {
								group_id_target = group->GetID();
							}
						}
						else if(spell_target->IsRaidGrouped())
						{
							if (Raid* raid = spell_target->GetRaid()) {
								uint32 group_id = raid->GetGroup(spell_target->CastToClient());
								group_id_target = (group_id == 0xFFFFFFFF) ? 0 : (group_id + 1);
							}
						}
					}
					else if(spell_target->IsPet())
					{
						Mob *owner = spell_target->GetOwner();
						if(owner->IsGrouped())
						{
							if (Group* group = owner->GetGroup()) {
								group_id_target = group->GetID();
							}
						}
						else if(owner->IsRaidGrouped())
						{
							if (Raid* raid = owner->GetRaid()) {
								uint32 group_id = raid->GetGroup(owner->CastToClient());
								group_id_target = (group_id == 0xFFFFFFFF) ? 0 : (group_id + 1);
							}
						}
					}
					else if(spell_target->IsBot())
					{
						if(spell_target->IsGrouped())
						{
							group_id_target = spell_target->GetGroup()->GetID();
						}
						else if(spell_target->IsRaidGrouped())
						{
							if(spell_target->GetOwner())
								group_id_target = (spell_target->GetRaid()->GetGroup(spell_target->GetOwner()->CastToClient()) == 0xFFFF) ? 0 : (spell_target->GetRaid()->GetGroup(spell_target->GetOwner()->CastToClient()) + 1);
						}
					}

					if(group_id_caster == 0 || group_id_target == 0)
					{
						LogSpells("Spell [{}] canceled: Attempted to cast a Single Target Group spell on a ungrouped member", spell_id);
						MessageString(Chat::Red, TARGET_GROUP_MEMBER);
						return false;
					}

					if(group_id_caster != group_id_target)
					{
						LogSpells("Spell [{}] canceled: Attempted to cast a Single Target Group spell on a ungrouped member", spell_id);
						MessageString(Chat::Red, TARGET_GROUP_MEMBER);
						return false;
					}

					CastAction = SingleTarget;
				}
			}
			else
			{
				CastAction = SingleTarget;
			}
			break;
		}

		case ST_Directional:
			CastAction = DirectionalAE;
			spell_target = nullptr;
			ae_center = nullptr;
			break;

		case ST_TargetsTarget:
		{
			Mob *spell_target_tot = spell_target ? spell_target->GetTarget() : nullptr;
			if (!spell_target_tot) {
				return false;
			}

			//Verfied from live - Target's Target needs to be in combat range to recieve the effect
			if (RuleB(Spells, TargetsTargetRequiresCombatRange) && !CombatRange(spell_target)) {
				return false;
			}

			spell_target = spell_target_tot;
			CastAction = SingleTarget;
			break;
		}

		case ST_PetMaster:
		{

			Mob *owner = nullptr;

			if (IsPet())
				owner = GetOwner();
			else if ((IsNPC() && CastToNPC()->GetSwarmOwner()))
				owner = entity_list.GetMobID(CastToNPC()->GetSwarmOwner());

			if (!owner)
				return false;

			spell_target = owner;
			CastAction = SingleTarget;
			break;
		}

		case ST_Beam:
		{
			CastAction = Beam;
			spell_target = nullptr;
			ae_center = nullptr;
			break;
		}

		case ST_Ring:
		{
			CastAction = TargetRing;
			spell_target = nullptr;
			ae_center = nullptr;
			break;
		}

		default:
		{
			LogSpells("I dont know Target Type: [{}]  Spell: ([{}]) [{}]", static_cast<int>(spells[spell_id].target_type), spell_id, spells[spell_id].name);
			Message(0, "I dont know Target Type: %d   Spell: (%d) %s", spells[spell_id].target_type, spell_id, spells[spell_id].name);
			CastAction = CastActUnknown;
			break;
		}
	}
	return(true);
}

// only used from CastedSpellFinished, and procs
// we can't interrupt in this, or anything called from this!
// if you need to abort the casting, return false
bool Mob::SpellFinished(uint16 spell_id, Mob *spell_target, CastingSlot slot, int32 mana_used,
						uint32 inventory_slot, int16 resist_adjust, bool isproc, int level_override,
						uint32 timer, uint32 timer_duration, bool from_casted_spell, uint32 aa_id)
{
	Mob *ae_center = nullptr;

	if(!IsValidSpell(spell_id))
		return false;

	//Death Touch targets the pet owner instead of the pet when said pet is tanking.
	if ((RuleB(Spells, CazicTouchTargetsPetOwner) && spell_target && spell_target->HasOwner()) && !spell_target->IsBot() && (spell_id == SPELL_CAZIC_TOUCH || spell_id == SPELL_TOUCH_OF_VINITRAS)) {
		Mob* owner = spell_target->GetOwner();

		if (owner) {
			spell_target = owner;
		}
	}

	if ((RuleB(Bots, CazicTouchBotsOwner) && spell_target && spell_target->IsBot()) && (spell_id == SPELL_CAZIC_TOUCH || spell_id == SPELL_TOUCH_OF_VINITRAS)) {
		auto bot_owner = spell_target->GetOwner();

		if (bot_owner) {
			spell_target = bot_owner;
		}
	}

	//Guard Assist Code
	if (RuleB(Character, PVPEnableGuardFactionAssist) && spell_target && IsDetrimentalSpell(spell_id) && spell_target != this) {
		if (IsClient() && spell_target->IsClient()|| (HasOwner() && GetOwner()->IsClient() && spell_target->IsClient())) {
			for (auto& e : spell_target->GetCloseMobList()) {
				auto mob = e.second;
				if (!mob) {
					continue;
				}
				if (mob->IsNPC() && mob->CastToNPC()->IsGuard()) {
					float distance = Distance(spell_target->GetPosition(), mob->GetPosition());
					if ((mob->CheckLosFN(spell_target) || mob->CheckLosFN(this)) && distance <= 70) {
						auto petorowner = GetOwnerOrSelf();
						if (spell_target->GetReverseFactionCon(mob) <= petorowner->GetReverseFactionCon(mob)) {
							mob->AddToHateList(this);
						}
					}
				}
			}
		}
	}

	//determine the type of spell target we have
	CastAction_type CastAction;
	if (!DetermineSpellTargets(spell_id, spell_target, ae_center, CastAction, slot, isproc)) {
		LogSpells("Spell [{}]: Determine spell targets failure.", spell_id);
		return(false);
	}

	//If spell was casted then we already checked these so skip, otherwise check here if being called directly from spell finished.
	if (!from_casted_spell) {
		if (!DoCastingChecksZoneRestrictions(true, spell_id)) {
			LogSpells("Spell [{}]: Zone restriction failure.", spell_id);
			return false;
		}
		if (!DoCastingChecksOnTarget(true, spell_id, spell_target)) {
			LogSpells("Spell [{}]: Casting checks on Target failure.", spell_id);
			return false;
		}
	}

	LogSpells("Spell [{}]: target type [{}], target [{}], AE center [{}]", spell_id, static_cast<int>(CastAction), spell_target?spell_target->GetName():"NONE", ae_center?ae_center->GetName():"NONE");

	// if a spell has the AEDuration flag, it becomes an AE on target
	// spell that's recast every 2500 msec for AEDuration msec.
	if(IsAEDurationSpell(spell_id)) {
		// the spells are AE target, but we aim them on a beacon
		glm::vec4 beacon_loc;
		if (spells[spell_id].target_type == ST_Ring) {
			beacon_loc = glm::vec4{ GetTargetRingX(),GetTargetRingY(), GetTargetRingZ(), GetHeading()};
		}
		else {
			if (spell_target) {
				beacon_loc = spell_target->GetPosition();
			}
			else {
				beacon_loc = GetPosition();
			}
		}
		// live has a bug where the heading is always north
		if (!RuleB(Spells, FixBeaconHeading)) {
			beacon_loc.w = 0.0f;
		}
		auto beacon = new Beacon(beacon_loc, spells[spell_id].aoe_duration);
		entity_list.AddBeacon(beacon);
		LogSpells("Spell [{}]: AE duration beacon created, entity id [{}]", spell_id, beacon->GetName());
		spell_target = nullptr;
		ae_center = beacon;
		CastAction = AECaster;
	}

	// check line of sight to target if it's a detrimental spell
	if (!spells[spell_id].npc_no_los && spell_target && IsDetrimentalSpell(spell_id) && (!CheckLosFN(spell_target) || !CheckWaterLoS(spell_target)) && !IsHarmonySpell(spell_id) && spells[spell_id].target_type != ST_TargetOptional)
	{
		LogSpells("Spell [{}]: cannot see target [{}]", spell_id, spell_target->GetName());
		MessageString(Chat::Red,CANT_SEE_TARGET);
		return false;
	}

	//range check our target, if we have one and it is not us
	float range = spells[spell_id].range + GetRangeDistTargetSizeMod(spell_target);
	bool can_use_tgb = (IsClient() && CastToClient()->TGB()) || (IsBot() && RuleB(Bots, EnableBotTGB));
	bool is_tgb_compatible = IsTGBCompatibleSpell(spell_id);
	bool is_group_spell = IsGroupSpell(spell_id);

	if (can_use_tgb && is_tgb_compatible && is_group_spell) {
		range = spells[spell_id].aoe_range;
	}

	range = GetActSpellRange(spell_id, range);
	if(IsOfClientBot() && IsIllusionSpell(spell_id) && (HasProjectIllusion())){
		range = 100;
	}

	if(spell_target != nullptr && spell_target != this) {
		//casting a spell on somebody but ourself, make sure they are in range
		float dist2 = DistanceSquared(m_Position, spell_target->GetPosition());
		float range2 = range * range;
		float min_range2 = spells[spell_id].min_range * spells[spell_id].min_range;
		if(dist2 > range2) {
			//target is out of range.
			LogSpells("Spell [{}]: Spell target is out of range (squared: [{}] > [{}])", spell_id, dist2, range2);
			MessageString(Chat::Red, TARGET_OUT_OF_RANGE);
			return(false);
		}
		else if (dist2 < min_range2){
			//target is too close range.
			LogSpells("Spell [{}]: Spell target is too close (squared: [{}] < [{}])", spell_id, dist2, min_range2);
			MessageString(Chat::Red, TARGET_TOO_CLOSE);
			return(false);
		}

		spell_target->CalcSpellPowerDistanceMod(spell_id, dist2);
	}
	//AE Duration spells were ignoring distance check from item clickies
	if(ae_center != nullptr && ae_center != this) {
		//casting a spell on somebody but ourself, make sure they are in range
		float dist2 = DistanceSquared(m_Position, ae_center->GetPosition());
		float range2 = range * range;
		float min_range2 = spells[spell_id].min_range * spells[spell_id].min_range;
		if(dist2 > range2) {
			//target is out of range.
			LogSpells("Spell [{}]: Spell target is out of range (squared: [{}] > [{}])", spell_id, dist2, range2);
			MessageString(Chat::Red, TARGET_OUT_OF_RANGE);
			return(false);
		}
		else if (dist2 < min_range2){
			//target is too close range.
			LogSpells("Spell [{}]: Spell target is too close (squared: [{}] < [{}])", spell_id, dist2, min_range2);
			MessageString(Chat::Red, TARGET_TOO_CLOSE);
			return(false);
		}

		ae_center->CalcSpellPowerDistanceMod(spell_id, dist2);
	}

	//
	// Switch #2 - execute the spell
	//

	switch(CastAction)
	{
		default:
		case CastActUnknown:
		case SingleTarget:
		{

			if(IsBot()) {
				bool StopLogic = false;
				if(!CastToBot()->DoFinishedSpellSingleTarget(spell_id, spell_target, slot, StopLogic))
					return false;
				if(StopLogic)
					break;
			}

			if(spell_target == nullptr) {
				LogSpells("Spell [{}]: Targeted spell, but we have no target", spell_id);
				return(false);
			}
			if (isproc) {
				SpellOnTarget(spell_id, spell_target, 0, true, resist_adjust, true, level_override);
			} else {
				if (spells[spell_id].target_type == ST_TargetOptional){
					if (!TrySpellProjectile(spell_target, spell_id))
						return false;
				}

				else if(!SpellOnTarget(spell_id, spell_target, 0, true, resist_adjust, false, level_override)) {
					if(IsBuffSpell(spell_id) && IsBeneficialSpell(spell_id)) {
						return false;
					}
				}
			}

			if(IsIllusionSpell(spell_id)
			&& IsOfClientBot()
			&& (HasProjectIllusion())){
				LogAA("Effect Project Illusion for [{}] on spell id: [{}] was ON", GetName(), spell_id);
				SetProjectIllusion(false);
			}
			else{
				LogAA("Effect Project Illusion for [{}] on spell id: [{}] was OFF", GetName(), spell_id);
			}
			break;
		}

		case AECaster:
		case AETarget:
		{
			// we can't cast an AE spell without something to center it on
			assert(ae_center != nullptr);

			if(ae_center->IsBeacon()) {
				// special ae duration spell
				ae_center->CastToBeacon()->AELocationSpell(this, spell_id, resist_adjust);
			} else {
				// unsure if we actually need this? Need to find some spell examples
				if(ae_center && ae_center == this && IsBeneficialSpell(spell_id))
					SpellOnTarget(spell_id, this);

				// NPCs should never be affected by an AE they cast. PB AEs shouldn't affect caster either
				// I don't think any other cases that get here matter
				bool affect_caster = !IsNPC() && spells[spell_id].target_type != ST_AECaster;

				if (spells[spell_id].target_type == ST_AETargetHateList)
					hate_list.SpellCast(this, spell_id, spells[spell_id].aoe_range, ae_center);
				else
					entity_list.AESpell(this, ae_center, spell_id, affect_caster, resist_adjust);
			}
			break;
		}

		case GroupSpell:
		{
			if (IsBot()) {
				bool stop_logic = false;
				if (!CastToBot()->DoFinishedSpellGroupTarget(spell_id, spell_target, slot, stop_logic)) {
					return false;
				}

				if(stop_logic) {
					break;
				}
			}

			// We hold off turning MBG off so we can still use it to calc the mana cost
			if (spells[spell_id].can_mgb && HasMGB()) {
				SpellOnTarget(spell_id, this);
				entity_list.MassGroupBuff(this, this, spell_id, true);
			} else {
				// at this point spell_target is a member of the other group, or the
				// caster if they're not using TGB
				// NOTE: this will always hit the caster, plus the target's group so
				// it can affect up to 7 people if the targeted group is not our own

				// Allow pets who cast group spells to affect the group.
				if (spell_target->IsPetOwnerOfClientBot() && IsPetOwnerOfClientBot()) {
					Mob* owner =  spell_target->GetOwner();

					if (owner) {
						spell_target = owner;
					}
				}

				if (spell_target->IsGrouped()) {
					Group *target_group = entity_list.GetGroupByMob(spell_target);
					if (target_group) {
						target_group->CastGroupSpell(this, spell_id);
						if (target_group != GetGroup() && GetClass() != Class::Bard) {
							SpellOnTarget(spell_id, this);
						}
					}
				} else if (spell_target->IsRaidGrouped() && spell_target->IsOfClientBot()) {
					Raid *target_raid = entity_list.GetRaidByClient(spell_target->CastToClient());
					if (IsBot()) {
						target_raid = entity_list.GetRaidByBot(spell_target->CastToBot());
					}

					uint32 gid = 0xFFFFFFFF;
					if (target_raid) {
						gid = target_raid->GetGroup(spell_target->GetName());
						if (gid < 12) {
							target_raid->CastGroupSpell(this, spell_id, gid);
						} else {
							SpellOnTarget(spell_id, spell_target);
						}
					}
				} else {
					// if target is grouped, CastGroupSpell will cast it on the caster
					// too, but if not then we have to do that here.

					if (spell_target != this) {
						SpellOnTarget(spell_id, this);
	#ifdef GROUP_BUFF_PETS
						//pet too
						if (spells[spell_id].target_type != ST_GroupNoPets && GetPet() && HasPetAffinity() && !GetPet()->IsCharmed()) {
							SpellOnTarget(spell_id, GetPet());
						}
	#endif
					}

					SpellOnTarget(spell_id, spell_target);
	#ifdef GROUP_BUFF_PETS
					//pet too
					if (spells[spell_id].target_type != ST_GroupNoPets && spell_target->GetPet() && spell_target->HasPetAffinity() && !spell_target->GetPet()->IsCharmed()) {
						SpellOnTarget(spell_id, spell_target->GetPet());
					}
	#endif
				}
			}
			break;
		}

		case CAHateList:
		{
			if(!IsOfClientBotMerc())
			{
				hate_list.SpellCast(this, spell_id, spells[spell_id].range > spells[spell_id].aoe_range ? spells[spell_id].range : spells[spell_id].aoe_range);
			}
			break;
		}

		case DirectionalAE:
		{
			ConeDirectional(spell_id, resist_adjust);
			break;
		}

		case Beam:
		{
			BeamDirectional(spell_id, resist_adjust);
			break;
		}

		case TargetRing:
		{
			entity_list.AESpell(this, nullptr, spell_id, false, resist_adjust);
			break;
		}
	}

	// Set and send the nimbus effect if this spell has one
	int NimbusEffect = GetSpellNimbusEffect(spell_id);
	if(NimbusEffect) {
		if(!IsNimbusEffectActive(NimbusEffect)) {
			SendSpellEffect(NimbusEffect, 500, 0, 1, 3000, true);
		}
	}

	bool mgb = HasMGB() && spells[spell_id].can_mgb;
	// if this was a spell slot or an ability use up the mana for it
	if(slot != CastingSlot::Item && slot != CastingSlot::PotionBelt && mana_used > 0)
	{
		mana_used = GetActSpellCost(spell_id, mana_used);
		if (mgb) {
			mana_used *= 2;
		}
		// clamp if we some how got focused above our current mana
		if (GetMana() < mana_used)
			mana_used = GetMana();
		LogSpells("Spell [{}]: consuming [{}] mana", spell_id, mana_used);
		if (!DoHPToManaCovert(mana_used)) {
			SetMana(GetMana() - mana_used);
			TryTriggerOnCastRequirement();
		}
	}
	// one may want to check if this is a disc or not, but we actually don't, there are non disc stuff that have end cost
	// lets not consume end for custom items that have disc procs.
	// One might also want to filter out USE_ITEM_SPELL_SLOT, but DISCIPLINE_SPELL_SLOT are both #defined to the same thing ...
	if (spells[spell_id].endurance_cost && !isproc) {
		auto end_cost = (int64)spells[spell_id].endurance_cost;
		if (mgb)
			end_cost *= 2;
		SetEndurance(GetEndurance() - EQ::ClampUpper(end_cost, GetEndurance()));
		TryTriggerOnCastRequirement();
	}
	if (mgb) {
		SetMGB(false);
	}
	/*
		Set Recast Timer on spells.
	*/

	if(IsClient() && !isproc && !IsFromTriggeredSpell(slot, inventory_slot))
	{
		if (slot == CastingSlot::AltAbility) {
			if (!aa_id) {
				aa_id = casting_spell_aa_id;
			}
			if (aa_id) {
				AA::Rank *rank = zone->GetAlternateAdvancementRank(aa_id);
				//handle expendable AA's
				if (rank && rank->base_ability) {
					ExpendAlternateAdvancementCharge(rank->base_ability->id);
				}
				//set AA recast timer
				CastToClient()->SendAlternateAdvancementTimer(rank->spell_type, 0, 0);
			}
		}
		//handle bard AA and Discipline recast timers when singing
		if (GetClass() == Class::Bard && spell_id != casting_spell_id && timer != 0xFFFFFFFF) {
			CastToClient()->GetPTimers().Start(timer, timer_duration);
			LogSpells("Spell [{}]: Setting BARD custom reuse timer [{}] to [{}]", spell_id, casting_spell_timer, casting_spell_timer_duration);
		}
		//handles AA and Discipline recast timers
		else if (spell_id == casting_spell_id && casting_spell_timer != 0xFFFFFFFF)
		{
			CastToClient()->GetPTimers().Start(casting_spell_timer, casting_spell_timer_duration);
			LogSpells("Spell [{}]: Setting custom reuse timer [{}] to [{}]", spell_id, casting_spell_timer, casting_spell_timer_duration);
		} else if (spells[spell_id].recast_time > 1000 && !spells[spell_id].is_discipline) {
			int recast = spells[spell_id].recast_time/1000;
			if (spell_id == SPELL_LAY_ON_HANDS)	{ //lay on hands
				recast -= GetAA(aaFervrentBlessing) * 420;
			} else if (IsHarmTouchSpell(spell_id)) { //harm touch
				recast -= GetAA(aaTouchoftheWicked) * 420;
			}

			int64 reduction = CastToClient()->GetFocusEffect(focusReduceRecastTime, spell_id);

			if (reduction) {
				recast -= reduction;
				casting_spell_recast_adjust = reduction * 1000; //used later to adjust on client with memorizespell_struct
				if (recast < 0) {
					casting_spell_recast_adjust = spells[spell_id].recast_time;
				}
				recast = std::max(recast, 0);
			}

			LogSpells("Spell [{}]: Setting long reuse timer to [{}] s (orig [{}])", spell_id, recast, spells[spell_id].recast_time);

			if (recast > 0) {
				CastToClient()->GetPTimers().Start(pTimerSpellStart + spell_id, recast);
			}
		}
	}

	if (IsBot() && !isproc && !IsFromTriggeredSpell(slot, inventory_slot) && IsValidSpell(spell_id)) {
		if (spells[spell_id].recast_time > 1000 && !spells[spell_id].is_discipline) {
			CastToBot()->SetSpellRecastTimer(spell_id);
		}
	}

	/*
		Set Recast Timer on item clicks, including augmenets.
	*/
	if(IsClient() && (slot == CastingSlot::Item || slot == CastingSlot::PotionBelt)){
		CastToClient()->SetItemRecastTimer(spell_id, inventory_slot);
	}
	else if (IsBot() && CastToBot()->GetIsUsingItemClick() && slot == CastingSlot::Item) {
		EQ::ItemInstance* inst = CastToBot()->GetBotItem(inventory_slot);
		const EQ::ItemData* item = nullptr;
		if (inst && inst->GetItem()) {
			item = inst->GetItem();
			CastToBot()->SetItemReuseTimer(item->ID);
			CastToBot()->SetIsUsingItemClick(false);
		}
	}

	if (IsNPC()) {
		CastToNPC()->AI_Event_SpellCastFinished(true, static_cast<uint16>(slot));
	}

	ApplyHealthTransferDamage(this, target, spell_id);

	//This needs to be here for bind sight to update correctly on client.
	if (IsClient() && IsEffectInSpell(spell_id, SpellEffect::BindSight)) {
		for (int i = 0; i < GetMaxTotalSlots(); i++) {
			if (buffs[i].spellid == spell_id) {
				CastToClient()->SendBuffNumHitPacket(buffs[i], i);//its hack, it works.
			}
		}
	}
	//Check if buffs has numhits, then resend packet so it displays the hit count.
	if (IsClient() && spells[spell_id].hit_number) {
		for (int i = 0; i < GetMaxTotalSlots(); i++) {
			if (buffs[i].spellid == spell_id && buffs[i].hit_number > 0) {
				CastToClient()->SendBuffNumHitPacket(buffs[i], i);
				break;
			}
		}
	}

	return true;
}

bool Mob::ApplyBardPulse(int32 spell_id, Mob *spell_target, CastingSlot slot) {

	/*
		Check any bard specific special behaviors we need before applying the next pulse.
		Note: Silence does not stop an active bard pulse.
	*/
	if (!spell_target) {
		return false;
	}
	/*
		Bard song charm that have no mana will continue to try and pulse on target, but will only reapply when charm fades.
		Live does not spam client with do not take hold messages. Checking here avoids that from happening. Only try to reapply if charm fades.
	*/
	if (spell_target->IsCharmed() && spells[spell_id].mana == 0 && spell_target->GetOwner() == this && IsEffectInSpell(spell_id, SpellEffect::Charm)) {
		if (IsClient()) {
			CastToClient()->CheckSongSkillIncrease(spell_id);
		}
		return true;
	}
	/*
		If divine aura applied while pulsing, it is not interrupted but does not reapply until DA fades.
	*/
	if (DivineAura() && !IsCastNotStandingSpell(spell_id)) {
		if (IsClient()) {
			CastToClient()->CheckSongSkillIncrease(spell_id);
		}
		return true;
	}
	/*
		Fear will stop pulsing.
	*/
	if (IsFeared()) {
		return false;
	}

	if (!SpellFinished(spell_id, spell_target, slot, spells[spell_id].mana, 0xFFFFFFFF, spells[spell_id].resist_difficulty)) {
		return false;
	}

	if (IsClient()) {
		CastToClient()->CheckSongSkillIncrease(spell_id);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// buff related functions

// returns how many _ticks_ the buff will last.
// a tick is 6 seconds
// this is the place to figure out random duration buffs like fear and charm.
// both the caster and target mobs are passed in, so different behavior can
// even be created depending on the types of mobs involved
//
// right now this is just an outline, working on this..
int Mob::CalcBuffDuration(Mob *caster, Mob *target, uint16 spell_id, int32 caster_level_override)
{
	int formula, duration;

	if(!IsValidSpell(spell_id) || (!caster && !target))
		return 0;

	if(!caster && !target)
		return 0;

	// if we have at least one, we can make do, we'll just pretend they're the same
	if(!caster)
		caster = target;
	if(!target)
		target = caster;

	// PVP duration
	if (IsDetrimentalSpell(spell_id) && target->IsClient() && caster->IsClient()) {
		formula = spells[spell_id].pvp_duration;
		duration = spells[spell_id].pvp_duration_cap;
	} else {
		formula = spells[spell_id].buff_duration_formula;
		duration = spells[spell_id].buff_duration;
	}

	int castlevel = caster->GetCasterLevel(spell_id);
	if(caster_level_override > 0)
		castlevel = caster_level_override;

	int res = CalcBuffDuration_formula(castlevel, formula, duration);
	if (
		(
			target->aabonuses.IllusionPersistence ||
			target->spellbonuses.IllusionPersistence ||
			target->itembonuses.IllusionPersistence ||
			RuleB(Spells, IllusionsAlwaysPersist)
		) &&
		spell_id != SPELL_MINOR_ILLUSION &&
		spell_id != SPELL_ILLUSION_TREE &&
		IsEffectInSpell(spell_id, SpellEffect::Illusion)
	) {
		res = 10000; // ~16h override
	}

	LogSpells("Spell [{}]: Casting level [{}], formula [{}], base_duration [{}]: result [{}]",
		spell_id, castlevel, formula, duration, res);

	return res;
}

// the generic formula calculations
int CalcBuffDuration_formula(int level, int formula, int duration)
{
	int temp;

	switch (formula) {
	case 1:
		temp = level > 3 ? level / 2 : 1;
		break;
	case 2:
		temp = level > 3 ? level / 2 + 5 : 6;
		break;
	case 3:
		temp = 30 * level;
		break;
	case 4: // only used by 'LowerElement'
		temp = 50;
		break;
	case 5:
		temp = 2;
		break;
	case 6:
		temp = level / 2 + 2;
		break;
	case 7:
		temp = level;
		break;
	case 8:
		temp = level + 10;
		break;
	case 9:
		temp = 2 * level + 10;
		break;
	case 10:
		temp = 3 * level + 10;
		break;
	case 11:
		temp = 30 * (level + 3);
		break;
	case 12:
		temp = level > 7 ? level / 4 : 1;
		break;
	case 13:
		temp = 4 * level + 10;
		break;
	case 14:
		temp = 5 * (level + 2);
		break;
	case 15:
		temp = 10 * (level + 10);
		break;
	case 50: // Permanent. Cancelled by casting/combat for perm invis, non-lev zones for lev, curing poison/curse
		// counters, etc.
		return -1;
	case 51: // Permanent. Cancelled when out of range of aura.
		return -4;
	default:
		// the client function has another bool parameter that if true returns -2 -- unsure
		if (formula < 200)
			return 0;
		temp = formula;
		break;
	}
	if (duration && duration < temp)
		temp = duration;
	return temp;
}

// helper function for AddBuff to determine stacking
// spellid1 is the spell already worn, spellid2 is the one trying to be cast
// returns:
// 0 if not the same type, no action needs to be taken
// 1 if spellid1 should be removed (overwrite)
// -1 if they can't stack and spellid2 should be stopped
//currently, a spell will not land if it would overwrite a better spell on any effect
//if all effects are better or the same, we overwrite, else we do nothing
int Mob::CheckStackConflict(uint16 spellid1, int caster_level1, uint16 spellid2, int caster_level2, Mob* caster1, Mob* caster2, int buffslot)
{
	const SPDat_Spell_Struct &sp1 = spells[spellid1];
	const SPDat_Spell_Struct &sp2 = spells[spellid2];

	int i, effect1, effect2, sp1_value, sp2_value;
	int blocked_effect, blocked_below_value, blocked_slot;
	int overwrite_effect, overwrite_below_value, overwrite_slot;

	LogSpells(
		"Check Stacking on old [{}] ([{}]) @ lvl [{}] (by [{}]) vs. new [{}] ([{}]) @ lvl [{}] (by [{}])",
		sp1.name,
		spellid1,
		caster_level1,
		!caster1 ? "Nobody" : caster1->GetName(),
		sp2.name,
		spellid2,
		caster_level2,
		!caster2 ? "Nobody" : caster2->GetName()
	);

	if (IsResurrectionEffects(spellid1)) {
		return 0;
	}

	if (spellbonuses.CompleteHealBuffBlocker && IsEffectInSpell(spellid2, SpellEffect::CompleteHeal)) {
		Message(0, "You must wait before you can be affected by this spell again.");
		return -1;
	}

	if (spellid1 == spellid2 ) {

		if (spellid1 == SPELL_EYE_OF_ZOMM && spellid2 == SPELL_EYE_OF_ZOMM) {//only the original Eye of Zomm spell will not take hold if affect is already on you, other versions client fades the buff as soon as cast.
			MessageString(Chat::Red, SPELL_NO_HOLD);
			return -1;
		}

		if (!IsStackableDOT(spellid1) && !IsEffectInSpell(spellid1, SpellEffect::ManaBurn)) { // mana burn spells we need to use the stacking command blocks live actually checks those first, we should probably rework to that too
			if (caster_level1 > caster_level2) { // cur buff higher level than new
				if (IsEffectInSpell(spellid1, SpellEffect::ImprovedTaunt)) {
					LogSpells("SE_ImprovedTaunt level exception, overwriting");
					return 1;
				} else {
					LogSpells("Spells the same but existing is higher level, stopping");
					return -1;
				}
			} else {
				LogSpells("Spells the same but newer is higher or equal level, overwriting");
				return 1;
			}
		} else if (spellid1 == SPELL_MANA_BURN) {
			LogSpells("Blocking spell because manaburn does not stack with itself");
			return -1;
		}
	}

	const std::string& always_stack_spells = RuleS(Spells, AlwaysStackSpells);
	if (spellid1 != spellid2 && !always_stack_spells.empty()) {
		const auto& v = Strings::Split(always_stack_spells, ",");
		if (Strings::Contains(v, std::to_string(spellid1))) {
			return 0;
		}
		if (Strings::Contains(v, std::to_string(spellid2))) {
			return 0;
		}
	}

	/*
	One of these is a bard song and one isn't and they're both beneficial so they should stack.
	*/
	if(IsBardSong(spellid1) != IsBardSong(spellid2))
	{
		if(!IsDetrimentalSpell(spellid1) && !IsDetrimentalSpell(spellid2))
		{
			LogSpells("[{}] and [{}] are beneficial, and one is a bard song, no action needs to be taken", sp1.name, sp2.name);
			return (0);
		}
	}

	bool effect_match = true; // Figure out if we're identical in effects on all slots.
	if (spellid1 != spellid2) {
		for (i = 0; i < EFFECT_COUNT; i++) {
			// we don't want this optimization for mana burns
			if (sp1.effect_id[i] != sp2.effect_id[i] || sp1.effect_id[i] == SpellEffect::ManaBurn) {
				effect_match = false;
				break;
			}
		}
	} else if (IsEffectInSpell(spellid1, SpellEffect::ManaBurn)) {
		LogSpells("We have a Mana Burn spell that is the same, they won't stack");
		return -1;
	}

	// check for special stacking overwrite in spell2 against effects in spell1
	// If all of the effects match they are the same line and shouldn't care for these checks
	if (!effect_match) {
		for(i = 0; i < EFFECT_COUNT; i++)
		{
			effect1 = sp1.effect_id[i];
			effect2 = sp2.effect_id[i];

			if (spellbonuses.Screech == 1) {
				if (effect2 == SpellEffect::Screech && sp2.base_value[i] == -1) {
					MessageString(Chat::SpellFailure, SCREECH_BUFF_BLOCK, sp2.name);
					return -1;
				}
			}

			/*
				Buff stacking prevention spell effects (446 - 449) works as follows... If B prevent A, if C prevent B, if D prevent C.
				If checking same type ie A vs A, which ever effect base value is higher will take hold.
				Special check is added to make sure the buffs stack properly when applied from fade on duration effect, since the buff
				is not fully removed at the time of the trigger
			*/
			if (spellbonuses.AStacker[SBIndex::BUFFSTACKER_EXISTS]) {
				if ((effect2 == SpellEffect::AStacker) && (sp2.effect_id[i] <= spellbonuses.AStacker[SBIndex::BUFFSTACKER_VALUE]))
					return -1;
			}

			if (spellbonuses.BStacker[SBIndex::BUFFSTACKER_EXISTS]) {
				if ((effect2 == SpellEffect::BStacker) && (sp2.effect_id[i] <= spellbonuses.BStacker[SBIndex::BUFFSTACKER_VALUE]))
					return -1;
				if ((effect2 == SpellEffect::AStacker) && (!IsCastOnFadeDurationSpell(spellid1) && buffs[buffslot].ticsremaining != 1 && IsEffectInSpell(spellid1, SpellEffect::BStacker)))
					return -1;
			}

			if (spellbonuses.CStacker[SBIndex::BUFFSTACKER_EXISTS]) {
				if ((effect2 == SpellEffect::CStacker) && (sp2.effect_id[i] <= spellbonuses.CStacker[SBIndex::BUFFSTACKER_VALUE]))
					return -1;
				if ((effect2 == SpellEffect::BStacker) && (!IsCastOnFadeDurationSpell(spellid1) && buffs[buffslot].ticsremaining != 1 && IsEffectInSpell(spellid1, SpellEffect::CStacker)))
					return -1;
			}

			if (spellbonuses.DStacker[SBIndex::BUFFSTACKER_EXISTS]) {
				if ((effect2 == SpellEffect::DStacker) && (sp2.effect_id[i] <= spellbonuses.DStacker[SBIndex::BUFFSTACKER_VALUE]))
					return -1;
				if ((effect2 == SpellEffect::CStacker) && (!IsCastOnFadeDurationSpell(spellid1) && buffs[buffslot].ticsremaining != 1 && IsEffectInSpell(spellid1, SpellEffect::DStacker)))
					return -1;
			}

			if(effect2 == SpellEffect::StackingCommand_Overwrite)
			{
				overwrite_effect = sp2.base_value[i];
				overwrite_slot = sp2.formula[i] - 201;	//they use base 1 for slots, we use base 0
				overwrite_below_value = sp2.max_value[i];
				if(sp1.effect_id[overwrite_slot] == overwrite_effect)
				{
					sp1_value = CalcSpellEffectValue(spellid1, overwrite_slot, caster_level1);

					LogSpells("[{}] ([{}]) overwrites existing spell if effect [{}] on slot [{}] is below [{}]. Old spell has value [{}] on that slot/effect. [{}]",
						sp2.name, spellid2, overwrite_effect, overwrite_slot, overwrite_below_value, sp1_value, (sp1_value < overwrite_below_value)?"Overwriting":"Not overwriting");

					if(sp1_value < overwrite_below_value)
					{
						if (IsResurrectionEffects(spellid1)) {
							int8 res_effect_check = GetSpellResurrectionSicknessCheck(spellid1, spellid2);
							if (res_effect_check != 0) {
								return res_effect_check;
							}
						}
						LogSpells("Overwrite spell because sp1_value < overwrite_below_value");
						return 1;			// overwrite spell if its value is less
					}
				} else {
					LogSpells("[{}] ([{}]) overwrites existing spell if effect [{}] on slot [{}] is below [{}], but we do not have that effect on that slot. Ignored",
						sp2.name, spellid2, overwrite_effect, overwrite_slot, overwrite_below_value);

				}
			} else if (effect1 == SpellEffect::StackingCommand_Block)
			{
				blocked_effect = sp1.base_value[i];
				blocked_slot = sp1.formula[i] - 201;
				blocked_below_value = sp1.max_value[i];

				if (sp2.effect_id[blocked_slot] == blocked_effect)
				{
					sp2_value = CalcSpellEffectValue(spellid2, blocked_slot, caster_level2);

					LogSpells("[{}] ([{}]) blocks effect [{}] on slot [{}] below [{}]. New spell has value [{}] on that slot/effect. [{}]",
						sp1.name, spellid1, blocked_effect, blocked_slot, blocked_below_value, sp2_value, (sp2_value < blocked_below_value)?"Blocked":"Not blocked");

					if (sp2_value < blocked_below_value)
					{
						if (IsDetrimentalSpell(spellid2))
						{
							//Live fixed this in 2018 to allow detrimental spells to bypass being blocked by SPA 148
							LogSpells("Detrimental spell [{}] ([{}]) avoids being blocked.", sp2.name, spellid2);
						}
						else
						{
							LogSpells("Blocking spell because sp2_Value < blocked_below_value");
							return -1;		//blocked
						}
					}
				} else {
					LogSpells("[{}] ([{}]) blocks effect [{}] on slot [{}] below [{}], but we do not have that effect on that slot. Ignored",
						sp1.name, spellid1, blocked_effect, blocked_slot, blocked_below_value);
				}
			}
		}
	} else {
		LogSpells("[{}] ([{}]) and [{}] ([{}]) appear to be in the same line, skipping Stacking Overwrite/Blocking checks",
				sp1.name, spellid1, sp2.name, spellid2);
	}

	bool sp1_detrimental = IsDetrimentalSpell(spellid1);
	bool sp2_detrimental = IsDetrimentalSpell(spellid2);
	bool sp_det_mismatch;

	if(sp1_detrimental == sp2_detrimental)
		sp_det_mismatch = false;
	else
		sp_det_mismatch = true;

	// now compare matching effects
	// arbitration takes place if 2 spells have the same effect at the same
	// effect slot, otherwise they're stackable, even if it's the same effect
	bool will_overwrite = false;
	bool values_equal = true;
	for(i = 0; i < EFFECT_COUNT; i++)
	{
		if(IsBlankSpellEffect(spellid1, i) || IsBlankSpellEffect(spellid2, i))
			continue;

		effect1 = sp1.effect_id[i];
		effect2 = sp2.effect_id[i];

		/*
		Quick check, are the effects the same, if so then
		keep going else ignore it for stacking purposes.
		*/
		if(effect1 != effect2)
			continue;

		if (IsBardOnlyStackEffect(effect1) && GetSpellLevel(spellid1, Class::Bard) != 255 &&
			GetSpellLevel(spellid2, Class::Bard) != 255)
			continue;

		// big ol' list according to the client, wasn't that nice!
		if (IsEffectIgnoredInStacking(effect1))
			continue;

		// negative AC affects are skipped. Ex. Sun's Corona and Glacier Breath should stack
		// There may be more SPAs we need to add here ....
		// The client does just check base rather than calculating the affect change value.
		if ((effect1 == SpellEffect::ArmorClass || effect1 == SpellEffect::ACv2) && sp2.base_value[i] < 0)
			continue;

		/*
		If target is a npc and caster1 and caster2 exist
		If Caster1 isn't the same as Caster2 and the effect is a DoT then ignore it.
		*/
		if(IsNPC() && caster1 && caster2 && caster1 != caster2) {
			if(effect1 == SpellEffect::CurrentHP && sp1_detrimental && sp2_detrimental) {
				LogSpells("Both casters exist and are not the same, the effect is a detrimental dot, moving on");
				continue;
			}
		}

		if(effect1 == SpellEffect::CompleteHeal){ //SE_CompleteHeal never stacks or overwrites ever, always block.
			LogSpells("Blocking spell because complete heal never stacks or overwries");
			return (-1);
		}

		/*
		If the spells aren't the same
		and the effect is a dot we can go ahead and stack it
		*/
		if(effect1 == SpellEffect::CurrentHP && spellid1 != spellid2 && sp1_detrimental && sp2_detrimental) {
			LogSpells("The spells are not the same and it is a detrimental dot, passing");
			continue;
		}

		sp1_value = CalcSpellEffectValue(spellid1, i, caster_level1);
		sp2_value = CalcSpellEffectValue(spellid2, i, caster_level2);

		// Spells like SoW won't stack if a snare effect is already in place.
		if (effect2 == SpellEffect::MovementSpeed && effect1 == SpellEffect::MovementSpeed) {
			if (sp1_value < 0 && sp2_value > 0) {
				return -1;
			} else if (sp2_value < 0 && sp1_value > 0) {
				continue;
			}
		}

		// DoTs won't overwrite regeneration but will block regeneration spells.
		if (spells[spellid1].buff_duration > 0 && spells[spellid2].buff_duration > 0 &&
			effect1 == SpellEffect::CurrentHP && effect2 == SpellEffect::CurrentHP) {
			if (!sp1_detrimental && sp2_detrimental) {
				continue;
			} else if (sp1_detrimental && !sp2_detrimental) {
				return -1;
			}
		}

		// some spells are hard to compare just on value. attack speed spells
		// have a value that's a percentage for instance
		if
		(
			effect1 == SpellEffect::AttackSpeed ||
			effect1 == SpellEffect::AttackSpeed2
		)
		{
			sp1_value -= 100;
			sp2_value -= 100;
		}

		if(sp1_value < 0)
			sp1_value = 0 - sp1_value;
		if(sp2_value < 0)
			sp2_value = 0 - sp2_value;

		if (IsResurrectionEffects(spellid1)) {
			int8 res_effect_check = GetSpellResurrectionSicknessCheck(spellid1, spellid2);
			if (res_effect_check != 0) {
				return res_effect_check;
			}
		}

		if(sp2_value < sp1_value) {
			LogSpells("Spell [{}] (value [{}]) is not as good as [{}] (value [{}]). Rejecting [{}]",
				sp2.name, sp2_value, sp1.name, sp1_value, sp2.name);
			return -1;	// can't stack
		}
		if (sp2_value != sp1_value)
			values_equal = false;

		//we dont return here... a better value on this one effect dosent mean they are
		//all better...

		LogSpells("Spell [{}] (value [{}]) is not as good as [{}] (value [{}]). We will overwrite [{}] if there are no other conflicts",
			sp1.name, sp1_value, sp2.name, sp2_value, sp1.name);
		will_overwrite = true;
	}

	//if we get here, then none of the values on the new spell are "worse"
	//so now we see if this new spell is any better, or if its not related at all
	if(will_overwrite) {
		if (values_equal && effect_match && !IsGroupSpell(spellid2) && IsGroupSpell(spellid1)) {
			LogSpells("[{}] ([{}]) appears to be the single target version of [{}] ([{}]), rejecting",
					sp2.name, spellid2, sp1.name, spellid1);
			return -1;
		}
		LogSpells("Stacking code decided that [{}] should overwrite [{}]", sp2.name, sp1.name);
		return(1);
	}

	LogSpells("Stacking code decided that [{}] is not affected by [{}]", sp2.name, sp1.name);
	return 0;
}

// Check Spell Level Restrictions
// returns true if they meet the restrictions, false otherwise
// derived from http://samanna.net/eq.general/buffs.shtml
// spells 1-50: no restrictons
// 51-65: SpellLevel/2+15
// 66+ Group Spells 62, Single Target 61
bool Mob::CheckSpellLevelRestriction(Mob *caster, uint16 spell_id)
{
	if (spells[spell_id].target_type == ST_Self) {
		LogSpells("[CheckSpellLevelRestriction] Self Only spell - no restrictions");
		return true;
	}

	if (!caster) {
		LogSpells("[CheckSpellLevelRestriction] No caster");
		return false;
	}

	if (caster->IsClient() && caster->CastToClient()->GetGM()) {
		LogSpells("[CheckSpellLevelRestriction] GM casting - No restrictions");
		return true;
	}

	bool check_for_restrictions = false;
	bool can_cast = true;

	// NON GM clients might be restricted by rule setting
	if (caster->IsOfClientBot()) {
		if (IsClient()) { // Only restrict client on client for this rule
			if (RuleB(Spells, BuffLevelRestrictions)) {
				check_for_restrictions = true;
			}
		}
		else if (IsBot()) {
			if (RuleB(Bots, BotBuffLevelRestrictions)) {
				check_for_restrictions = true;
			}
		}
	}
	// NPCS might be restricted by rule setting
	else if (RuleB(Spells, NPCBuffLevelRestrictions)) {
		check_for_restrictions = true;
	}

	if (check_for_restrictions) {
		int spell_level = GetSpellMinimumLevel(spell_id);

		// Only check for beneficial buffs
		if (IsBuffSpell(spell_id) && IsBeneficialSpell(spell_id)) {
			if (spell_level > 65) {
				if (IsGroupSpell(spell_id) && GetLevel() < 62) {
					can_cast = false;
				}
				else if (GetLevel() < 61) {
					can_cast = false;
				}
			} else if (spell_level > 50) { // 51-65
				if (GetLevel() < (spell_level / 2 + 15)) {
					can_cast = false;
				}
			}
		}
	}

	if (!can_cast) {
		LogSpells("Spell [{}] failed: recipient did not meet the level restrictions", spell_id);
		if (!IsBardSong(spell_id)) {
			caster->MessageString(Chat::SpellFailure, SPELL_TOO_POWERFUL);
		}
	}

	return can_cast;
}

uint32 Mob::GetFirstBuffSlot(bool disc, bool song)
{
	return 0;
}

uint32 Mob::GetLastBuffSlot(bool disc, bool song)
{
	return GetCurrentBuffSlots();
}

uint32 Client::GetFirstBuffSlot(bool disc, bool song)
{
	if (song)
		return GetMaxBuffSlots();
	if (disc)
		return GetMaxBuffSlots() + GetMaxSongSlots();
	return 0;
}

uint32 Client::GetLastBuffSlot(bool disc, bool song)
{
	if (song)
		return GetMaxBuffSlots() + GetCurrentSongSlots();
	if (disc)
		return GetMaxBuffSlots() + GetMaxSongSlots() + GetCurrentDiscSlots();
	return GetCurrentBuffSlots();
}

bool Mob::HasDiscBuff()
{
	int slot = GetFirstBuffSlot(true, false);
	return IsValidSpell(buffs[slot].spellid);
}

// returns the slot the buff was added to, -1 if it wasn't added due to
// stacking problems, and -2 if this is not a buff
// if caster is null, the buff will be added with the caster level being
// the level of the mob
int Mob::AddBuff(Mob *caster, uint16 spell_id, int duration, int32 level_override, bool disable_buff_overwrite)
{
	int buffslot, ret, caster_level, emptyslot = -1;
	bool will_overwrite = false;
	std::vector<int> overwrite_slots;

	if (level_override > 0)
		caster_level = level_override;
	else
		caster_level = caster ? caster->GetCasterLevel(spell_id) : GetCasterLevel(spell_id);

	if (duration == 0) {
		duration = CalcBuffDuration(caster, this, spell_id);

		if (caster && duration > 0) // negatives are perma buffs
			duration = caster->GetActSpellDuration(spell_id, duration);
	}

	if (duration == 0) {
		LogSpells("Buff [{}] failed to add because its duration came back as 0", spell_id);
		return -2;	// no duration? this isn't a buff
	}

	LogSpells("Trying to add buff [{}] cast by [{}] (cast level [{}]) with duration [{}]",
		spell_id, caster?caster->GetName():"UNKNOWN", caster_level, duration);

	// first we loop through everything checking that the spell
	// can stack with everything. this is to avoid stripping the spells
	// it would overwrite, and then hitting a buff we can't stack with.
	// we also check if overwriting will occur. this is so after this loop
	// we can determine if there will be room for this buff
	int buff_count = GetMaxTotalSlots();
	uint32 start_slot = GetFirstBuffSlot(IsDisciplineBuff(spell_id), spells[spell_id].short_buff_box);
	uint32 end_slot = GetLastBuffSlot(IsDisciplineBuff(spell_id), spells[spell_id].short_buff_box);

	for (buffslot = 0; buffslot < buff_count; buffslot++) {
		const Buffs_Struct &curbuf = buffs[buffslot];

		if (IsValidSpell(curbuf.spellid)) {
			// there's a buff in this slot
			ret = CheckStackConflict(
				curbuf.spellid,
				curbuf.casterlevel,
				spell_id,
				caster_level,
				entity_list.GetMobID(curbuf.casterid),
				caster,
				buffslot
			);

			if (ret == -1) { // stop the spell
				LogSpells(
					"Adding buff [{}] failed: stacking prevented by spell [{}] in slot [{}] with caster level [{}]",
					spell_id,
					curbuf.spellid,
					buffslot,
					curbuf.casterlevel
				);

				if (caster) {
					if (caster->IsClient() && RuleB(Client, UseLiveBlockedMessage) && caster->GetClass() != Class::Bard) {
						caster->Message(
							Chat::Red,
							fmt::format(
								"Your {} did not take hold on {}. (Blocked by {}.)",
								spells[spell_id].name,
								GetName(),
								spells[curbuf.spellid].name
							).c_str()
						);
					}

					if (caster->IsBot() && RuleB(Bots, BotsUseLiveBlockedMessage) && caster->GetClass() != Class::Bard) {
						caster->GetOwner()->Message(
							Chat::SpellFailure,
							fmt::format(
								"{}'s {} did not take hold on {}. (Blocked by {}.)",
								caster->GetCleanName(),
								spells[spell_id].name,
								GetName(),
								spells[curbuf.spellid].name
							).c_str()
						);
					}

					std::function<std::string()> f = [&]() {
						return fmt::format(
							"{} {}",
							curbuf.spellid,
							spell_id
						);
					};

					parse->EventMob(EVENT_SPELL_BLOCKED, caster, this, f);

					if (caster != this) {
						parse->EventMob(EVENT_SPELL_BLOCKED, this, caster, f);
					}
				}

				return -1;
			} else if (ret == 1 && !will_overwrite) {
				// set a flag to indicate that there will be overwriting
				LogSpells(
					"Adding buff [{}] will overwrite spell [{}] in slot [{}] with caster level [{}]",
					spell_id,
					curbuf.spellid,
					buffslot,
					curbuf.casterlevel
				);

				// If this is the first buff it would override, use its slot
				if (!will_overwrite && !IsDisciplineBuff(spell_id)) {
					emptyslot = buffslot;
				}

				will_overwrite = true;
				overwrite_slots.push_back(buffslot);
			} else if (ret == 2) {
				//ResurrectionEffectBlock handling to move potential overwrites to a new buff slock while keeping Res Sickness
				LogSpells(
					"Adding buff [{}] will overwrite spell [{}] in slot [{}] with caster level [{}], but ResurrectionEffectBlock is set to 2. Attempting to move [{}] to an empty buff slot.",
					spell_id,
					curbuf.spellid,
					buffslot,
					curbuf.casterlevel,
					spell_id
				);
			}
		} else {
			if (emptyslot == -1) {
				if (buffslot >= start_slot && buffslot < end_slot) {
					emptyslot = buffslot;
				}
			}
		}
	}

	// we didn't find an empty slot to put it in, and it's not overwriting
	// anything so there must not be any room left.
	if (emptyslot == -1 && !will_overwrite) {
		if (IsDetrimentalSpell(spell_id))  {//Sucks to be you, bye bye one of your buffs
			for (buffslot = 0; buffslot < buff_count; buffslot++) {
				const Buffs_Struct &curbuf = buffs[buffslot];
				if (IsBeneficialSpell(curbuf.spellid)) {
					LogSpells("No slot for detrimental buff [{}], so we are overwriting a beneficial buff [{}] in slot [{}]",
							spell_id, curbuf.spellid, buffslot);
					BuffFadeBySlot(buffslot, false);
					emptyslot = buffslot;
					break;
				}
			}
			if(emptyslot == -1) {
				LogSpells("Unable to find a buff slot for detrimental buff [{}]", spell_id);
				return -1;
			}
		} else {
			LogSpells("Unable to find a buff slot for beneficial buff [{}]", spell_id);
			return -1;
		}
	}
	//do not fade buff if from bard pulse, live does not give a fades message.
	bool from_bard_song_pulse = caster ? caster->IsActiveBardSong(spell_id) : false;

	// at this point we know that this buff will stick, but we have
	// to remove some other buffs already worn if will_overwrite is true
	if (will_overwrite && !disable_buff_overwrite) {
		std::vector<int>::iterator cur, end;
		cur = overwrite_slots.begin();
		end = overwrite_slots.end();
		for (; cur != end; ++cur) {
			// strip spell
			if (!from_bard_song_pulse) {
				BuffFadeBySlot(*cur, false);
			}

			// if we hadn't found a free slot before, or if this is earlier
			// we use it
			if (emptyslot == -1 || (*cur < emptyslot && !IsDisciplineBuff(spell_id)))
				emptyslot = *cur;
		}
	}

	//remove associated buffs for certain live spell lines
	if (IsAegolismSpell(spell_id)) {
		int buff_count = GetMaxBuffSlots();
		for (int slot = 0; slot < buff_count; slot++) {
			if (IsValidSpell(buffs[slot].spellid)) {
				if (AegolismStackingIsSymbolSpell(buffs[slot].spellid) || AegolismStackingIsArmorClassSpell(buffs[slot].spellid)) {
					BuffFadeBySlot(slot);
				}
			}
		}
	}

	buffs[emptyslot].spellid = spell_id;
	buffs[emptyslot].casterlevel = caster_level;
	if (caster && !caster->IsAura()) // maybe some other things we don't want to ...
		strcpy(buffs[emptyslot].caster_name, caster->GetCleanName());
	else
		memset(buffs[emptyslot].caster_name, 0, 64);
	buffs[emptyslot].casterid = caster ? caster->GetID() : 0;
	buffs[emptyslot].ticsremaining = duration;
	buffs[emptyslot].counters = CalculateCounters(spell_id);
	buffs[emptyslot].hit_number = spells[spell_id].hit_number;
	buffs[emptyslot].client = caster ? caster->IsClient() : 0;
	buffs[emptyslot].persistant_buff = 0;
	buffs[emptyslot].caston_x = 0;
	buffs[emptyslot].caston_y = 0;
	buffs[emptyslot].caston_z = 0;
	buffs[emptyslot].dot_rune = 0;
	buffs[emptyslot].ExtraDIChance = 0;
	buffs[emptyslot].RootBreakChance = 0;
	buffs[emptyslot].virus_spread_time = 0;
	buffs[emptyslot].instrument_mod = caster ? caster->GetInstrumentMod(spell_id) : 10;

	if (level_override > 0 || buffs[emptyslot].hit_number > 0) {
		buffs[emptyslot].UpdateClient = true;
	} else {
		if (buffs[emptyslot].ticsremaining > (1 + CalcBuffDuration_formula(caster_level, spells[spell_id].buff_duration_formula, spells[spell_id].buff_duration)))
			buffs[emptyslot].UpdateClient = true;
	}

	LogSpells("Buff [{}] added to slot [{}] with caster level [{}]", spell_id, emptyslot, caster_level);
	if (IsPet() && GetOwner() && GetOwner()->IsClient())
		SendPetBuffsToClient();

	if((IsClient() && !CastToClient()->GetPVP()) ||
		(IsPet() && GetOwner() && GetOwner()->IsClient() && !GetOwner()->CastToClient()->GetPVP()) ||
		(IsBot() && GetOwner() && GetOwner()->IsClient() && !GetOwner()->CastToClient()->GetPVP()) ||
		(IsMerc() && GetOwner() && GetOwner()->IsClient() && !GetOwner()->CastToClient()->GetPVP()))
	{
		EQApplicationPacket *outapp = MakeBuffsPacket();

		entity_list.QueueClientsByTarget(this, outapp, false, nullptr, true, false, EQ::versions::maskSoDAndLater);

		if(IsClient() && GetTarget() == this)
			CastToClient()->QueuePacket(outapp);

		safe_delete(outapp);
	}

	if (IsNPC()) {
		EQApplicationPacket *outapp = MakeBuffsPacket();
		entity_list.QueueClientsByTarget(this, outapp, false, nullptr, true, false, EQ::versions::maskSoDAndLater, true);
		safe_delete(outapp);
	}

	// recalculate bonuses since we stripped/added buffs
	CalcBonuses();

	return emptyslot;
}

// used by some MobAI stuff
// NOT USED BY SPELL CODE
// note that this should not be used for determining which slot to place a
// buff into
// returns -1 on stack failure, -2 if all slots full, the slot number if the buff should overwrite another buff, or a free buff slot
int Mob::CanBuffStack(uint16 spellid, uint8 caster_level, bool iFailIfOverwrite)
{
	int i, ret, firstfree = -2;

	LogAIDetail("Checking if buff [{}] cast at level [{}] can stack on me.[{}]", spellid, caster_level, iFailIfOverwrite?" failing if we would overwrite something":"");

	int buff_count = GetMaxTotalSlots();
	for (i=0; i < buff_count; i++)
	{
		const Buffs_Struct &curbuf = buffs[i];

		// no buff in this slot
		if (!IsValidSpell(curbuf.spellid))
		{
			// if we haven't found a free slot, this is the first one so save it
			if (firstfree == -2) {
				firstfree = i;
			}

			continue;
		}

		if (
			IsBot() &&
			GetClass() == Class::Bard &&
			curbuf.spellid == spellid &&
			curbuf.ticsremaining == 0 &&
			curbuf.casterid == GetID()
		) {
			LogAI("Bard check for song, spell [{}] has [{}] ticks remaining.", spellid, curbuf.ticsremaining);
			firstfree = i;
		}
		else {
			if (curbuf.spellid == spellid) {
				return(-1);	//do not recast a buff we already have on, we recast fast enough that we dont need to refresh our buffs
			}
		}

		// there's a buff in this slot
		ret = CheckStackConflict(curbuf.spellid, curbuf.casterlevel, spellid, caster_level, nullptr, nullptr, i);

		if (ret == 1) {
			// should overwrite current slot
			if (iFailIfOverwrite) {
				LogAIDetail("Buff [{}] would overwrite [{}] in slot [{}], reporting stack failure", spellid, curbuf.spellid, i);
				return(-1);
			}

			if (firstfree == -2) {
				firstfree = i;
			}
		}

		if(ret == -1) {
			LogAIDetail("Buff [{}] would conflict with [{}] in slot [{}], reporting stack failure", spellid, curbuf.spellid, i);
			return -1;	// stop the spell, can't stack it
		}

		if (ret == 2) { //ResurrectionEffectBlock handling to move potential overwrites to a new buff slock while keeping Res Sickness
			LogAIDetail("Adding buff [{}] will overwrite spell [{}] in slot [{}] with caster level [{}], but ResurrectionEffectBlock is set to 2. Attempting to move [{}] to an empty buff slot.",
				spellid, curbuf.spellid, i, curbuf.casterlevel, spellid);

			for (int x = 0; x < buff_count; x++) {
				const Buffs_Struct& curbuf = buffs[x];

				if (IsValidSpell(curbuf.spellid)) {
					continue;
				}
				else {
					firstfree = x;
				}
			}
		}
	}

	LogAIDetail("Reporting that buff [{}] could successfully be placed into slot [{}]", spellid, firstfree);

	return firstfree;
}

///////////////////////////////////////////////////////////////////////////////
// spell effect related functions
//
// this is actually applying a spell cast from 'this' on 'spelltar'
// it performs pvp checking and applies resists, etc then it
// passes it to SpellEffect which causes effects to the target
//
// this is called by these functions:
// Mob::SpellFinished
// Entity::AESpell (called by Mob::SpellFinished)
// Group::CastGroupSpell (called by Mob::SpellFinished)
//
// also note you can't interrupt the spell here. at this point it's going
// and if you don't want effects just return false. interrupting here will
// break stuff
//
bool Mob::SpellOnTarget(
	uint16 spell_id,
	Mob *spelltar,
	int reflect_effectiveness,
	bool use_resist_adjust,
	int16 resist_adjust,
	bool isproc,
	int level_override,
	int duration_override,
	bool disable_buff_overwrite
) {
	auto spellOwner = GetOwnerOrSelf();

	// well we can't cast a spell on target without a target
	if (!spelltar) {
		LogSpells("Unable to apply spell [{}] without a target", spell_id);
		Message(Chat::Red, "SOT: You must have a target for this spell.");
		return false;
	}

	if (spelltar->IsClient() && spelltar->CastToClient()->IsHoveringForRespawn()) {
		return false;
	}

	if (!IsValidSpell(spell_id)) {
		return false;
	}

	if (
		IsDetrimentalSpell(spell_id) &&
		!IsAttackAllowed(spelltar, true) &&
		!IsResurrectionEffects(spell_id) &&
		!IsEffectInSpell(spell_id, SpellEffect::BindSight)
	) {
		if (!IsClient() || !CastToClient()->GetGM()) {
			MessageString(Chat::SpellFailure, SPELL_NO_HOLD);
			return false;
		}
	}

	EQApplicationPacket *action_packet = nullptr, *message_packet = nullptr;
	float spell_effectiveness;

	// these target types skip pcnpc only check (according to dev quotes)
	// other AE spells this is redundant, oh well
	// 1 = PCs, 2 = NPCs
	if (
		spells[spell_id].pcnpc_only_flag &&
		spells[spell_id].target_type != ST_AETargetHateList &&
		spells[spell_id].target_type != ST_HateList
	) {
		if (
			spells[spell_id].pcnpc_only_flag == PCNPCOnlyFlagType::PC &&
			!spelltar->IsOfClientBotMerc()
		) {
			return false;
		} else if (
			spells[spell_id].pcnpc_only_flag == PCNPCOnlyFlagType::NPC &&
			(
				spelltar->IsOfClientBotMerc()
			)
		) {
			return false;
		}
	}

	uint16 caster_level = level_override > 0 ? level_override : GetCasterLevel(spell_id);

	LogSpells("Casting spell [{}] on [{}] with effective caster level [{}]", spell_id, spelltar->GetName(), caster_level);

	// Actual cast action - this causes the caster animation and the particles
	// around the target
	// we do this first, that way we get the particles even if the spell
	// doesn't land due to pvp protection
	// note: this packet is sent again if the spell is successful, with a flag
	// set
	action_packet = new EQApplicationPacket(OP_Action, sizeof(Action_Struct));
	Action_Struct* action = (Action_Struct*) action_packet->pBuffer;

	// select source
	if (IsClient() && CastToClient()->GMHideMe()) {
		action->source = spelltar->GetID();
	} else {
		action->source = GetID();
		// this is a hack that makes detrimental buffs work client to client
		// TODO figure out how to do this right
		if
		(
			IsDetrimentalSpell(spell_id) &&
			IsClient() &&
			spelltar->IsClient()
		)
		{
			action->source = spelltar->GetID();
		}
	}

	// select target
	uint16 target_id = 0;

	if (IsEffectInSpell(spell_id, SpellEffect::BindSight)) {
		action->target = GetID();
		target_id = GetID();
	} else {
		action->target = spelltar->GetID();
		target_id = spelltar->GetID();
	}

	action->spell_level = action->level = caster_level;	// caster level, for animation only
	action->type = 231;	// 231 means a spell
	action->spell = spell_id;
	action->force = spells[spell_id].push_back;
	action->hit_heading = GetHeading();
	action->hit_pitch = spells[spell_id].push_up;
	action->instrument_mod = GetInstrumentMod(spell_id);
	action->effect_flag = 0;

	if (spelltar != this && spelltar->IsClient()) {    // send to target
		spelltar->CastToClient()->QueuePacket(action_packet);
	}

	if (IsClient()) { // send to caster
		CastToClient()->QueuePacket(action_packet);
	}

	// send to people in the area, ignoring caster and target
	entity_list.QueueCloseClients(
		spelltar, /* Sender */
		action_packet, /* Packet */
		true, /* Ignore Sender */
		RuleI(Range, SpellMessages),
		this, /* Skip this Mob */
		true, /* Packet ACK */
		(spellOwner->IsClient() ? FilterPCSpells : FilterNPCSpells) /* EQ Filter Type: (8 or 9) */
	);

	std::vector<std::any> args = { spelltar };

	parse->EventMob(EVENT_CAST_ON, spelltar, this,
		[&]() {
			return fmt::format(
				"{} {} {} {}",
				spell_id,
				GetID(),
				caster_level,
				target_id
			);
		},
		0, &args
	);

	if (!DoCastingChecksOnTarget(false, spell_id, spelltar)) {
		safe_delete(action_packet);
		return false;
	}

	bool client_blocked_buffs =
		RuleB(Spells, EnableBlockedBuffs) &&
		(
			spelltar->IsClient() ||
			(spelltar->IsPet() && spelltar->IsPetOwnerClient())
		);

	bool bot_blocked_buffs =
		RuleB(Bots, AllowBotBlockedBuffs) &&
		(
			spelltar->IsBot() ||
			(spelltar->IsPet() && spelltar->IsPetOwnerBot())
		);

	// now check if the spell is allowed to land
	if (client_blocked_buffs || bot_blocked_buffs) {
		// We return true here since the caster's client should act like normal
		if (spelltar->IsBlockedBuff(spell_id)) {
			LogSpells(
				"Spell [{}] not applied to [{}] as it is a Blocked Buff",
				spell_id,
				spelltar->GetName()
			);
			safe_delete(action_packet);
			return true;
		}

		if (
			spelltar->IsPet() &&
			spelltar->GetOwner() &&
			spelltar->GetOwner()->IsBlockedPetBuff(spell_id)
		) {
			LogSpells(
				"Spell [{}] not applied to [{}] ([{}]'s pet) as it is a Pet Blocked Buff",
				spell_id,
				spelltar->GetName(),
				spelltar->GetOwner()->GetName()
			);
			safe_delete(action_packet);
			return true;
		}
	}

	// invuln mobs can't be affected by any spells, good or bad, except if caster is casting a spell with 'cast_not_standing' on self.
	if (
		(spelltar->GetInvul() && !spelltar->DivineAura()) ||
		(spelltar != this && spelltar->DivineAura()) ||
		(spelltar == this && spelltar->DivineAura() && !IsCastNotStandingSpell(spell_id))
	) {
		if (IsClient()) {
			Message(
				Chat::White,
				fmt::format(
					"Your spell {} has failed to land on {} because {} are invulnerable.",
					Saylink::Silent(
						fmt::format(
							"#castspell {}",
							spell_id
						),
						spells[spell_id].name
					),
					GetTargetDescription(spelltar),
					spelltar == this ? "you" : "they"
				).c_str()
			);
		}

		LogSpells("Casting spell [{}] on [{}] aborted: they are invulnerable", spell_id, spelltar->GetName());
		safe_delete(action_packet);
		return false;
	}

	//cannot hurt untargetable mobs
	uint8 bt = spelltar->GetBodyType();
	if (bt == BodyType::NoTarget || bt == BodyType::NoTarget2) {
		if (RuleB(Pets, UnTargetableSwarmPet)) {
			if (spelltar->IsNPC()) {
				if (!spelltar->CastToNPC()->GetSwarmOwner()) {
					LogSpells("Casting spell [{}] on [{}] aborted: they are untargetable", spell_id, spelltar->GetName());
					safe_delete(action_packet);
					return(false);
				}
			} else {
				LogSpells("Casting spell [{}] on [{}] aborted: they are untargetable", spell_id, spelltar->GetName());
				safe_delete(action_packet);
				return(false);
			}
		} else {
			LogSpells("Casting spell [{}] on [{}] aborted: they are untargetable", spell_id, spelltar->GetName());
			safe_delete(action_packet);
			return(false);
		}
	}

	// Prevent double invising, which made you uninvised
	// Not sure if all 3 should be stacking
	//This is not live like behavior (~Kayen confirmed 2/2/22)
	if (!RuleB(Spells, AllowDoubleInvis) && !IsActiveBardSong(spell_id)) {
		if (IsEffectInSpell(spell_id, SpellEffect::Invisibility)) {
			if (spelltar->invisible) {
				spelltar->MessageString(Chat::SpellFailure, ALREADY_INVIS, GetCleanName());
				safe_delete(action_packet);
				return false;
			}
		}

		if (IsEffectInSpell(spell_id, SpellEffect::InvisVsUndead)) {
			if (spelltar->invisible_undead) {
				spelltar->MessageString(Chat::SpellFailure, ALREADY_INVIS, GetCleanName());
				safe_delete(action_packet);
				return false;
			}
		}

		if (IsEffectInSpell(spell_id, SpellEffect::InvisVsAnimals)) {
			if (spelltar->invisible_animals) {
				spelltar->MessageString(Chat::SpellFailure, ALREADY_INVIS, GetCleanName());
				safe_delete(action_packet);
				return false;
			}
		}
	}

	if (!(IsClient() && CastToClient()->GetGM()) && !IsHarmonySpell(spell_id)) {// GMs can cast on anything
		// Beneficial spells check
		if (IsBeneficialSpell(spell_id)) {
			if (IsClient() && spelltar != this) {//let NPCs do beneficial spells on anybody if they want, should be the job of the AI, not the spell code to prevent this from going wrong
				Client* pClient = nullptr;
				Raid* pRaid = nullptr;
				Group* pBasicGroup = nullptr;
				uint32 nGroup = 0; //raid group

				Client* pClientTarget = nullptr;
				Raid* pRaidTarget = nullptr;
				Group* pBasicGroupTarget = nullptr;
				uint32 nGroupTarget = 0; //raid group

				Client* pClientTargetPet = nullptr;
				Raid* pRaidTargetPet = nullptr;
				Group* pBasicGroupTargetPet = nullptr;
				uint32 nGroupTargetPet = 0; //raid group

				const uint32 cnWTF = 0xFFFFFFFF + 1; //this should be zero unless on 64bit? forced uint64?

				//Caster client pointers
				pClient = CastToClient();
				pRaid = entity_list.GetRaidByClient(pClient);
				pBasicGroup = entity_list.GetGroupByMob(this);
				if (pRaid) {
					nGroup = pRaid->GetGroup(pClient) + 1;
				}

				//Target client pointers
				if (spelltar->IsClient()) {
					pClientTarget = spelltar->CastToClient();
					pRaidTarget = entity_list.GetRaidByClient(pClientTarget);
					pBasicGroupTarget = entity_list.GetGroupByMob(spelltar);
					if (pRaidTarget) {
						nGroupTarget = pRaidTarget->GetGroup(pClientTarget) + 1;
					}
				}

				if (spelltar->IsPet()) {
					Mob *owner = spelltar->GetOwner();
					if (owner->IsClient()) {
						pClientTargetPet = owner->CastToClient();
						pRaidTargetPet = entity_list.GetRaidByClient(pClientTargetPet);
						pBasicGroupTargetPet = entity_list.GetGroupByMob(owner);
						if (pRaidTargetPet) {
							nGroupTargetPet = pRaidTargetPet->GetGroup(pClientTargetPet) + 1;
						}
					}

				}

				if (
					(!IsAllianceSpell(spell_id) && !IsBeneficialAllowed(spelltar)) ||
					(IsGroupOnlySpell(spell_id) &&
						!(
							(
								pBasicGroup &&
								(
									pBasicGroup == pBasicGroupTarget ||
									pBasicGroup == pBasicGroupTargetPet
								)
							) || //Basic Group
							(
								nGroup != cnWTF &&
								(
									nGroup == nGroupTarget ||
									nGroup == nGroupTargetPet
								)
							) || //Raid group
							spelltar == GetPet() //should be able to cast grp spells on self and pet despite grped status.
						)
					)
				) {
					if (spells[spell_id].target_type == ST_AEBard) {
						//if it was a beneficial AE bard song don't spam the window that it would not hold
						LogSpells(
							"Beneficial ae bard song [{}] can't take hold [{}] -> [{}], IBA? [{}]",
							spell_id,
							GetName(),
							spelltar->GetName(),
							IsBeneficialAllowed(spelltar)
						);
					} else {
						LogSpells(
							"Beneficial spell [{}] can't take hold [{}] -> [{}], IBA? [{}]",
							spell_id,
							GetName(),
							spelltar->GetName(),
							IsBeneficialAllowed(spelltar)
						);
						MessageString(Chat::SpellFailure, SPELL_NO_HOLD);
					}
					safe_delete(action_packet);
					return false;
				}
			}
		} else if (
			!IsAttackAllowed(spelltar, true) &&
			!IsResurrectionEffects(spell_id) &&
			!IsEffectInSpell(spell_id, SpellEffect::BindSight)
		) { // Detrimental spells - PVP check
			LogSpells(
				"Detrimental spell [{}] can't take hold [{}] -> [{}]",
				spell_id,
				GetName(),
				spelltar->GetName()
			);
			spelltar->MessageString(Chat::SpellFailure, YOU_ARE_PROTECTED, GetCleanName());
			safe_delete(action_packet);
			return false;
		}
	}

	// ok at this point the spell is permitted to affect the target,
	// but we need to check special cases and resists

	// check immunities
	if (spelltar->IsImmuneToSpell(spell_id, this)) {
		// If we tried with Dire Charm, we need to reset the timer.
		if (IsClient() && (casting_spell_aa_id == aaDireCharm || casting_spell_aa_id == aaDireCharm2 || casting_spell_aa_id == aaDireCharm3)) {
			StopCasting();
		}

		//the above call does the message to the client if needed
		LogSpells("Spell [{}] can't take hold due to immunity [{}] -> [{}]", spell_id, GetName(), spelltar->GetName());
		safe_delete(action_packet);
		return false;
	}

	//check for AE_Undead
	if (spells[spell_id].target_type == ST_UndeadAE){
		if (
			spelltar->GetBodyType() != BodyType::SummonedUndead &&
			spelltar->GetBodyType() != BodyType::Undead &&
			spelltar->GetBodyType() != BodyType::Vampire
		) {
			safe_delete(action_packet);
			return false;
		}
	}

	//Need this to account for special AOE cases.
	if (
		IsClient() &&
		IsHarmonySpell(spell_id) &&
		!HarmonySpellLevelCheck(spell_id, spelltar)
	) {
		MessageString(Chat::SpellFailure, SPELL_NO_EFFECT);
		safe_delete(action_packet);
		return false;
	}

	// Block next spell effect should be used up first(since its blocking the next spell)
	if (CanBlockSpell()) {
		int buff_count = GetMaxTotalSlots();
		int focus = 0;
		for (int b = 0; b < buff_count; b++) {
			if (IsEffectInSpell(buffs[b].spellid, SpellEffect::BlockNextSpellFocus)) {
				focus = CalcFocusEffect(focusBlockNextSpell, buffs[b].spellid, spell_id);
				if (focus) {
					CheckNumHitsRemaining(NumHit::MatchingSpells, b);
					MessageString(Chat::SpellFailure, SPELL_WOULDNT_HOLD);
					safe_delete(action_packet);
					return false;
				}
			}
		}
	}
	/*
		Reflect
		base= % Chance to Reflect
		Limit= Resist Modifier (+Value for decrease chance to resist)
		Max= % of base spell damage (this is the base before any formula or focus is applied)
		On live any type of detrimental spell can be reflected as long as the Reflectable spell field is set, this includes AOE.
		The 'caster' of the reflected spell is owner of the reflect effect. Caster's focus effects are NOT applied to reflected spell.

		reflect_effectiveness is applied to damage spells, a value of 100 is no change to base damage. Other values change by percent. (50=50% of damage)
		we this variable to both check if a spell being applied is from a reflection and for the damage modifier.

		There are a few spells in database that are not detrimental that have Reflectable field set, however from testing, they do not actually reflect.
	*/
	if (
		spells[spell_id].reflectable &&
		!reflect_effectiveness &&
		spelltar &&
		this != spelltar &&
		IsDetrimentalSpell(spell_id) &&
		(
			spelltar->spellbonuses.reflect[SBIndex::REFLECT_CHANCE] ||
			spelltar->aabonuses.reflect[SBIndex::REFLECT_CHANCE] ||
			spelltar->itembonuses.reflect[SBIndex::REFLECT_CHANCE]
		)
	) {
		bool can_spell_reflect = false;
		switch (RuleI(Spells, ReflectType)) {
			case REFLECT_SINGLE_TARGET_SPELLS_ONLY: {
				if (spells[spell_id].target_type == ST_Target) {
					for (int y = 0; y < 16; y++) {
						if (spells[spell_id].classes[y] < 255) {
							can_spell_reflect = true;
						}
					}
				}

				break;
			}
			case REFLECT_ALL_PLAYER_SPELLS: {
				for (int y = 0; y < 16; y++) {
					if (spells[spell_id].classes[y] < 255) {
						can_spell_reflect = true;
					}
				}

				break;
			}
			case RELFECT_ALL_SINGLE_TARGET_SPELLS: {
				if (spells[spell_id].target_type == ST_Target) {
					can_spell_reflect = true;
				}

				break;
			}
			case REFLECT_ALL_SPELLS: {//This is live like behavior
				can_spell_reflect = true;
			}
			case REFLECT_DISABLED:
			default: {
				break;
			}
		}

		if (can_spell_reflect) {
			int reflect_resist_adjust = 0;
			int reflect_effectiveness_mod = 0; //Need value of 100 to do baseline unmodified damage.

			if (
				spelltar->spellbonuses.reflect[SBIndex::REFLECT_CHANCE] &&
				zone->random.Roll(spelltar->spellbonuses.reflect[SBIndex::REFLECT_CHANCE])
			) {
				reflect_resist_adjust     = spelltar->spellbonuses.reflect[SBIndex::REFLECT_RESISTANCE_MOD];
				reflect_effectiveness_mod = spelltar->spellbonuses.reflect[SBIndex::REFLECT_DMG_EFFECTIVENESS]
					? spelltar->spellbonuses.reflect[SBIndex::REFLECT_DMG_EFFECTIVENESS] : 100;
			} else if (
				spelltar->aabonuses.reflect[SBIndex::REFLECT_CHANCE] &&
				zone->random.Roll(spelltar->aabonuses.reflect[SBIndex::REFLECT_CHANCE])
			) {
				reflect_effectiveness_mod = 100;
				reflect_resist_adjust = spelltar->aabonuses.reflect[SBIndex::REFLECT_RESISTANCE_MOD];
			} else if (
				spelltar->itembonuses.reflect[SBIndex::REFLECT_CHANCE] &&
				zone->random.Roll(spelltar->itembonuses.reflect[SBIndex::REFLECT_CHANCE])
			) {
				reflect_resist_adjust     = spelltar->itembonuses.reflect[SBIndex::REFLECT_RESISTANCE_MOD];
				reflect_effectiveness_mod = spelltar->itembonuses.reflect[SBIndex::REFLECT_DMG_EFFECTIVENESS]
					? spelltar->itembonuses.reflect[SBIndex::REFLECT_DMG_EFFECTIVENESS] : 100;
			}

			if (reflect_effectiveness_mod) {
				if (RuleB(Spells, ReflectMessagesClose)) {
					entity_list.MessageCloseString(
						this, /* Sender */
						false, /* Skip Sender */
						RuleI(Range, SpellMessages), /* Range */
						Chat::Spells, /* Type */
						SPELL_REFLECT, /* String ID */
						GetCleanName(), /* Message 1 */
						spelltar->GetCleanName() /* Message 2 */
					);
				} else {
					MessageString(Chat::Spells, SPELL_REFLECT, GetCleanName(), spelltar->GetCleanName());
				}

				CheckNumHitsRemaining(NumHit::ReflectSpell);

				spelltar->SpellOnTarget(spell_id, this, reflect_effectiveness_mod, use_resist_adjust, (resist_adjust - reflect_resist_adjust));
				safe_delete(action_packet);
				return false;
			}
		}
	}

	// resist check - every spell can be resisted, beneficial or not
	// add: ok this isn't true, eqlive's spell data is fucked up, buffs are
	// not all unresistable, so changing this to only check certain spells
	if (IsResistableSpell(spell_id)) {
		spelltar->BreakInvisibleSpells(); //Any detrimental spell cast on you will drop invisible (can be AOE, non damage ect).

		if (
			IsCharmSpell(spell_id) ||
			IsMesmerizeSpell(spell_id) ||
			IsFearSpell(spell_id)
		) {
			spell_effectiveness = spelltar->ResistSpell(
				spells[spell_id].resist_type,
				spell_id,
				this,
				use_resist_adjust,
				resist_adjust,
				true,
				false,
				false,
				level_override
			);
		} else {
			spell_effectiveness = spelltar->ResistSpell(
				spells[spell_id].resist_type,
				spell_id,
				this,
				use_resist_adjust,
				resist_adjust,
				false,
				false,
				false,
				level_override
			);
		}

		if (spell_effectiveness < 100) {
			if (spell_effectiveness == 0 || !IsPartialResistableSpell(spell_id)) {
				LogSpells("Spell [{}] was completely resisted by [{}]", spell_id, spelltar->GetName());

				if (spells[spell_id].resist_type == RESIST_PHYSICAL){
					MessageString(Chat::SpellFailure, PHYSICAL_RESIST_FAIL,spells[spell_id].name);
					spelltar->MessageString(Chat::SpellFailure, YOU_RESIST, spells[spell_id].name);
				} else {
					MessageString(Chat::SpellFailure, TARGET_RESISTED, spells[spell_id].name);
					spelltar->MessageString(Chat::SpellFailure, YOU_RESIST, spells[spell_id].name);

					if (IsBot() && RuleB(Bots, ShowResistMessagesToOwner)) {
						CastToBot()->GetBotOwner()->Message
						(Chat::SpellFailure,
							fmt::format(
								"{} resisted {}'s spell: {}.",
								spelltar->GetCleanName(),
								GetCleanName(),
								spells[spell_id].name
							).c_str()
						);
					}
				}

				if (spelltar->IsAIControlled()) {
					auto aggro = CheckAggroAmount(spell_id, spelltar);
					if (aggro > 0) {
						if (!IsHarmonySpell(spell_id)) {
							spelltar->AddToHateList(this, aggro);
						} else if (!spelltar->PassCharismaCheck(this, spell_id)) {
							spelltar->AddToHateList(this, aggro);
						}
					} else {
						int64 newhate = spelltar->GetHateAmount(this) + aggro;
						spelltar->SetHateAmountOnEnt(this, std::max(static_cast<int64>(1), newhate));
					}
				}

				if (spelltar->IsClient()){
					spelltar->CastToClient()->BreakSneakWhenCastOn(this, true);
					spelltar->CastToClient()->BreakFeignDeathWhenCastOn(true);
				}

				spelltar->CheckNumHitsRemaining(NumHit::IncomingSpells);
				CheckNumHitsRemaining(NumHit::OutgoingSpells);

				safe_delete(action_packet);
				return false;
			}
		}
		if (spelltar->IsClient()){
			spelltar->CastToClient()->BreakSneakWhenCastOn(this, false);
			spelltar->CastToClient()->BreakFeignDeathWhenCastOn(false);
		}
	} else {
		spell_effectiveness = 100;
	}

	if (
		spells[spell_id].feedbackable &&
		(
			spelltar->spellbonuses.SpellDamageShield ||
			spelltar->itembonuses.SpellDamageShield ||
			spelltar->aabonuses.SpellDamageShield
		)
	) {
		spelltar->DamageShield(this, true);
	}

	if (!(spelltar->IsNPC() && IsNPC() && !(IsPet() && GetOwner()->IsClient()))) {
		if (spelltar->IsAIControlled() && IsDetrimentalSpell(spell_id) && !IsHarmonySpell(spell_id)) {
			auto aggro_amount = CheckAggroAmount(spell_id, spelltar, isproc);
			LogSpellsDetail("Spell {} cast on {} generated {} hate", spell_id,
				spelltar->GetName(), aggro_amount);
			if (aggro_amount > 0) {
				spelltar->AddToHateList(this, aggro_amount);
			} else {
				int64 newhate = spelltar->GetHateAmount(this) + aggro_amount;
				spelltar->SetHateAmountOnEnt(this, std::max(newhate, static_cast<int64>(1)));
			}
		} else if (IsBeneficialSpell(spell_id) && !IsSummonPCSpell(spell_id)) {
			if (this != spelltar && IsClient()){
				if (spelltar->IsClient()) {
					CastToClient()->UpdateRestTimer(spelltar->CastToClient()->GetRestTimer());
				} else if (spelltar->IsPet()) {
					auto* owner = spelltar->GetOwner();
					if (owner && owner != this && owner->IsClient()) {
						CastToClient()->UpdateRestTimer(owner->CastToClient()->GetRestTimer());
					}
				}
			}

			entity_list.AddHealAggro(
				spelltar,
				this,
				CheckHealAggroAmount(
					spell_id,
					spelltar,
					(spelltar->GetMaxHP() - spelltar->GetHP())
				)
			);
		}
	}

	// make sure spelltar is high enough level for the buff
	if (!spelltar->CheckSpellLevelRestriction(this, spell_id)) {
		safe_delete(action_packet);
		return false;
	}

	// cause the effects to the target
	if (
		!spelltar->SpellEffect(
			this,
			spell_id,
			spell_effectiveness,
			level_override,
			reflect_effectiveness,
			duration_override,
			disable_buff_overwrite
		)
	) {
		// if SpellEffect returned false there's a problem applying the
		// spell. It's most likely a buff that can't stack.
		LogSpells("Spell [{}] could not apply its effects [{}] -> [{}]\n", spell_id, GetName(), spelltar->GetName());
		if (casting_spell_aa_id) {
			MessageString(Chat::SpellFailure, SPELL_NO_HOLD);
			if (RuleB(Spells, LegacyManaburn) && IsClient() && casting_spell_aa_id == aaManaBurn) {
				StopCasting();
			}
		}
		safe_delete(action_packet);
		return false;
	}

	//Check SpellEffect::Fc_Cast_Spell_On_Land SPA 481 on target, if hit by this spell and Conditions are Met then target will cast the specified spell.
	if (spelltar) {
		spelltar->CastSpellOnLand(this, spell_id);
	}

	if (IsValidSpell(spells[spell_id].recourse_link) && spells[spell_id].recourse_link != spell_id) {
		SpellFinished(
			spells[spell_id].recourse_link,
			this,
			CastingSlot::Item,
			0,
			-1,
			spells[spells[spell_id].recourse_link].resist_difficulty
		);
	}

	if (IsDetrimentalSpell(spell_id)) {
		CheckNumHitsRemaining(NumHit::OutgoingSpells);

		if (spelltar) {
			spelltar->CheckNumHitsRemaining(NumHit::IncomingSpells);
		}
	}

	// send the action packet again now that the spell is successful
	// NOTE: this is what causes the buff icon to appear on the client, if
	// this is a buff - but it sortof relies on the first packet.
	// the complete sequence is 2 actions and 1 damage message
	action->effect_flag = 0x04;	// this is a success flag

	if (spells[spell_id].push_back != 0.0f || spells[spell_id].push_up != 0.0f) {
		if (spelltar->IsClient()) {
			if (!IsBuffSpell(spell_id)) {
				spelltar->CastToClient()->cheat_manager.SetExemptStatus(KnockBack, true);
			}
		} else if (
			RuleB(Spells, NPCSpellPush) &&
			!spelltar->IsPermaRooted() &&
			!spelltar->IsPseudoRooted() &&
			!spelltar->ForcedMovement
		) {
			spelltar->m_Delta.x += action->force * g_Math.FastSin(action->hit_heading);
			spelltar->m_Delta.y += action->force * g_Math.FastCos(action->hit_heading);
			spelltar->m_Delta.z += action->hit_pitch;
			spelltar->ForcedMovement = 6;
		}
	}

	if (spelltar->IsClient() && IsEffectInSpell(spell_id, SpellEffect::ShadowStep)) {
		spelltar->CastToClient()->cheat_manager.SetExemptStatus(ShadowStep, true);
	}

	if (!IsEffectInSpell(spell_id, SpellEffect::BindAffinity)) {
		if (spelltar != this && spelltar->IsClient()) {// send to target
			spelltar->CastToClient()->QueuePacket(action_packet);
		}

		if(IsClient()) {// send to caster
			CastToClient()->QueuePacket(action_packet);
		}
	}

	static EQApplicationPacket p(OP_Damage, sizeof(CombatDamage_Struct));
	auto                       cd = (CombatDamage_Struct *) p.pBuffer;
	cd->target = action->target;
	cd->source = action->source;
	cd->type = action->type;
	cd->spellid = action->spell;
	cd->force = action->force;
	cd->hit_heading = action->hit_heading;
	cd->hit_pitch = action->hit_pitch;
	cd->damage = 0;

	if (
		!IsLifetapSpell(spell_id) &&
		!IsEffectInSpell(spell_id, SpellEffect::BindAffinity) &&
		!IsAENukeSpell(spell_id) &&
		!IsDamageSpell(spell_id)
	) {
		entity_list.QueueCloseClients(
			spelltar, /* Sender */
			&p, /* Packet */
			false, /* Ignore Sender */
			RuleI(Range, SpellMessages),
			0, /* Skip this mob */
			true, /* Packet ACK */
			(spellOwner->IsClient() ? FilterPCSpells : FilterNPCSpells) /* Message Filter Type: (8 or 9) */
		);
	}

	safe_delete(action_packet);

	/*
		Bug: When an HP buff with a heal effect is applied for first time, the heal portion of the effect heals the client and
		updates HPs currently server side, but client side the HP bar does not register it as a heal thus you display as less than full HP.
		However due to server thinking your healed, you are unable to correct it by healing.
		Solution: You need to resend the HP update after buff completed and action packet resent.
	*/
	if ((IsEffectInSpell(spell_id, SpellEffect::TotalHP) || IsEffectInSpell(spell_id, SpellEffect::MaxHPChange)) && (IsEffectInSpell(spell_id, SpellEffect::CurrentHPOnce) || IsEffectInSpell(spell_id, SpellEffect::CurrentHP))) {
		SendHPUpdate(true);
	}

	LogSpells("Cast of [{}] by [{}] on [{}] complete successfully", spell_id, GetName(), spelltar->GetName());

	if (IsBot() && (CastToBot()->GetCastedSpellType() != UINT16_MAX)) {
		if (!CastToBot()->IsCommandedSpell()) {
			CastToBot()->SetBotSpellRecastTimer(CastToBot()->GetCastedSpellType(), spelltar);
		}

		CastToBot()->SetCastedSpellType(UINT16_MAX);
	}

	return true;
}

std::vector<uint16> Mob::GetBuffSpellIDs()
{
	std::vector<uint16> l;

	for (int i = 0; i < GetMaxTotalSlots(); i++) {
		const auto& e = buffs[i].spellid;
		if (IsValidSpell(e)) {
			l.emplace_back(e);
		}
	}

	return l;
}

bool Mob::FindBuff(uint16 spell_id, uint16 caster_id)
{
	const int buff_count = GetMaxTotalSlots();
	for (int buff_slot = 0; buff_slot < buff_count; buff_slot++) {
		const uint16 current_spell_id = buffs[buff_slot].spellid;
		if (
			IsValidSpell(current_spell_id) &&
			current_spell_id == spell_id &&
			(
				!caster_id ||
				buffs[buff_slot].casterid == caster_id
			)
		) {
			return true;
		}
	}

	return false;
}

uint16 Mob::FindBuffBySlot(int slot) {
	auto current_spell_id = buffs[slot].spellid;
	if (IsValidSpell(current_spell_id)) {
		return current_spell_id;
	}

	return 0;
}

uint32 Mob::BuffCount(bool is_beneficial, bool is_detrimental) {
	uint32 active_buff_count = 0;
	int buff_count = GetMaxTotalSlots();
	for (int buff_slot = 0; buff_slot < buff_count; buff_slot++) {
		const auto is_spell_beneficial = IsBeneficialSpell(buffs[buff_slot].spellid);
		if (
			IsValidSpell(buffs[buff_slot].spellid) &&
			(
				(is_beneficial && is_spell_beneficial) ||
				(is_detrimental && !is_spell_beneficial)
			)
		) {
			active_buff_count++;
		}
	}

	return active_buff_count;
}

bool Mob::HasBuffWithSpellGroup(int spell_group)
{
	for (int buff_slot = 0; buff_slot < GetMaxTotalSlots(); buff_slot++) {
		auto current_spell_id = buffs[buff_slot].spellid;
		if (
			IsValidSpell(current_spell_id) &&
			spells[current_spell_id].spell_group == spell_group
		) {
			return true;
		}
	}

	return false;
}

void Mob::BuffFadeAll()
{
	bool recalc_bonus = false;
	int buff_count = GetMaxTotalSlots();
	for (int buff_slot = 0; buff_slot < buff_count; buff_slot++) {
		if (IsValidSpell(buffs[buff_slot].spellid)) {
			BuffFadeBySlot(buff_slot, false);
			recalc_bonus = true;
		}
	}

	if (recalc_bonus) {
		CalcBonuses();
	}
}

void Mob::BuffFadeNonPersistDeath()
{
	bool recalc_bonus = false;
	int buff_count = GetMaxTotalSlots();
	for (int buff_slot = 0; buff_slot < buff_count; buff_slot++) {
		auto current_spell_id = buffs[buff_slot].spellid;
		if (
			IsValidSpell(current_spell_id) &&
			!IsPersistDeathSpell(current_spell_id) &&
			!HasPersistDeathIllusion(current_spell_id)
		) {
			BuffFadeBySlot(buff_slot, false);
			recalc_bonus = true;
		}
	}

	if (recalc_bonus) {
		CalcBonuses();
	}
}

void Mob::BuffFadeBeneficial() {
	bool recalc_bonus = false;
	int buff_count = GetMaxTotalSlots();
	for (int buff_slot = 0; buff_slot < buff_count; buff_slot++) {
		auto current_spell_id = buffs[buff_slot].spellid;
		if (
			IsValidSpell(current_spell_id) &&
			IsBeneficialSpell(current_spell_id)
		) {
			BuffFadeBySlot(buff_slot, false);
			recalc_bonus = true;
		}
	}

	if (recalc_bonus) {
		CalcBonuses();
	}
}

void Mob::BuffFadeDetrimental() {
	bool recalc_bonus = false;
	int buff_count = GetMaxTotalSlots();
	for (int buff_slot = 0; buff_slot < buff_count; buff_slot++) {
		auto current_spell_id = buffs[buff_slot].spellid;
		if (
			IsValidSpell(current_spell_id) &&
			IsDetrimentalSpell(current_spell_id)
		) {
			BuffFadeBySlot(buff_slot, false);
			recalc_bonus = true;
		}
	}

	if (recalc_bonus) {
		CalcBonuses();
	}
}

void Mob::BuffFadeDetrimentalByCaster(Mob *caster)
{
	if(!caster) {
		return;
	}

	bool recalc_bonus = false;
	int buff_count = GetMaxTotalSlots();
	for (int buff_slot = 0; buff_slot < buff_count; buff_slot++) {
		auto current_spell_id = buffs[buff_slot].spellid;
		if (
			IsValidSpell(current_spell_id) &&
			IsDetrimentalSpell(current_spell_id) &&
			caster->GetID() == buffs[buff_slot].casterid
		) {
			BuffFadeBySlot(buff_slot, false);
			recalc_bonus = true;
		}
	}

	if (recalc_bonus) {
		CalcBonuses();
	}
}

void Mob::BuffFadeBySitModifier()
{
	bool recalc_bonus = false;
	int buff_count = GetMaxTotalSlots();
	for (int buff_slot = 0; buff_slot < buff_count; buff_slot++) {
		auto current_spell_id = buffs[buff_slot].spellid;
		if (
			IsValidSpell(current_spell_id) &&
			spells[current_spell_id].disallow_sit
		) {
			BuffFadeBySlot(buff_slot, false);
			recalc_bonus = true;
		}
	}

	if (recalc_bonus) {
		CalcBonuses();
	}
}

void Mob::BuffFadeBySpellID(uint16 spell_id)
{
	bool recalc_bonus = false;
	int buff_count = GetMaxTotalSlots();
	for (int buff_slot = 0; buff_slot < buff_count; buff_slot++) {
		if (buffs[buff_slot].spellid == spell_id) {
			BuffFadeBySlot(buff_slot, false);
			recalc_bonus = true;
		}
	}

	if (recalc_bonus) {
		CalcBonuses();
	}
}

void Mob::BuffFadeBySpellIDAndCaster(uint16 spell_id, uint16 caster_id)
{
	bool recalc_bonus = false;
	auto buff_count = GetMaxTotalSlots();
	for (int buff_slot = 0; buff_slot < buff_count; buff_slot++) {
		if (
			buffs[buff_slot].spellid == spell_id &&
			buffs[buff_slot].casterid == caster_id
		) {
			BuffFadeBySlot(buff_slot, false);
			recalc_bonus = true;
		}
	}

	if (recalc_bonus) {
		CalcBonuses();
	}
}

void Mob::BuffFadeByEffect(int effect_id, int slot_to_skip)
{
	bool recalc_bonus = false;
	int buff_count = GetMaxTotalSlots();
	for(int buff_slot = 0; buff_slot < buff_count; buff_slot++) {
		auto current_spell_id = buffs[buff_slot].spellid;
		if (
			IsValidSpell(current_spell_id) &&
			IsEffectInSpell(current_spell_id, effect_id) &&
			buff_slot != slot_to_skip
		) {
			BuffFadeBySlot(buff_slot, false);
			recalc_bonus = true;
		}
	}

	if (recalc_bonus) {
		CalcBonuses();
	}
}

void Mob::BuffFadeSongs() {
	bool recalc_bonus = false;
	int  buff_count   = GetMaxTotalSlots();

	for (int buff_slot = 0; buff_slot < buff_count; buff_slot++) {
		const uint16 current_spell_id = buffs[buff_slot].spellid;
		if (IsBardSong(current_spell_id)) {
			BuffFadeBySlot(buff_slot, false);
			recalc_bonus = true;
		}
	}

	if (recalc_bonus) {
		CalcBonuses();
	}
}

bool Mob::IsAffectedByBuffByGlobalGroup(GlobalGroup group)
{
	int buff_count = GetMaxTotalSlots();
	for (int buff_slot = 0; buff_slot < buff_count; buff_slot++) {
		auto current_spell_id = buffs[buff_slot].spellid;
		if (
			IsValidSpell(current_spell_id) &&
			spells[current_spell_id].spell_category == static_cast<int>(group)
		) {
			return true;
		}
	}

	return false;
}

void Mob::BuffDetachCaster(Mob *caster) {
	if (!caster) {
		return;
	}

	int buff_count = GetMaxTotalSlots();
	for (int j = 0; j < buff_count; j++) {
		if (buffs[j].spellid != SPELL_UNKNOWN) {
			if (IsDetrimentalSpell(buffs[j].spellid)) {
				//this is a pretty terrible way to do this but
				//there really isn't another way till I rewrite the basics
				Mob* c = entity_list.GetMob(buffs[j].casterid);
				if (c && c == caster) {
					buffs[j].casterid = 0;
				}
			}
		}
	}
}

// checks if 'this' can be affected by spell_id from caster
// returns true if the spell should fail, false otherwise
bool Mob::IsImmuneToSpell(uint16 spell_id, Mob *caster)
{
	int effect_index;

	if(caster == nullptr)
		return(false);

	//TODO: this function loops through the effect list for
	//this spell like 10 times, this could easily be consolidated
	//into one loop through with a switch statement.

	LogSpells("Checking to see if we are immune to spell [{}] cast by [{}]", spell_id, caster->GetName());

#ifdef LUA_EQEMU
	bool is_immune = false;
	bool ignore_default = false;
	is_immune = LuaParser::Instance()->IsImmuneToSpell(this, caster, spell_id, ignore_default);

	if (ignore_default) {
		return is_immune;
	}
#endif

	if(!IsValidSpell(spell_id))
		return true;

	if(IsBeneficialSpell(spell_id) && (caster->GetNPCTypeID())) //then skip the rest, stop NPCs aggroing each other with buff spells. 2013-03-05
		return false;

	if(IsMesmerizeSpell(spell_id))
	{
		if(GetSpecialAbility(SpecialAbility::MesmerizeImmunity)) {
			LogSpells("We are immune to Mez spells");
			caster->MessageString(Chat::SpellFailure, CANNOT_MEZ);
			int32 aggro = caster->CheckAggroAmount(spell_id, this);
			if(aggro > 0) {
				AddToHateList(caster, aggro);
			} else {
				AddToHateList(caster, 1,0,true,false,false,spell_id);
			}
			return true;
		}

		// check max level for spell
		effect_index = GetSpellEffectIndex(spell_id, SpellEffect::Mez);
		assert(effect_index >= 0);
		// NPCs get to ignore the max level
		if((GetLevel() > spells[spell_id].max_value[effect_index]) &&
			(!caster->IsNPC() || (caster->IsNPC() && !RuleB(Spells, NPCIgnoreBaseImmunity))))
		{
			LogSpells("Our level ([{}]) is higher than the limit of this Mez spell ([{}])", GetLevel(), spells[spell_id].max_value[effect_index]);
			caster->MessageString(Chat::SpellFailure, CANNOT_MEZ_WITH_SPELL);
			AddToHateList(caster, 1,0,true,false,false,spell_id);
			return true;
		}
	}

	// slow and haste spells
	if(GetSpecialAbility(SpecialAbility::SlowImmunity) && IsEffectInSpell(spell_id, SpellEffect::AttackSpeed))
	{
		LogSpells("We are immune to Slow spells");
		caster->MessageString(Chat::Red, IMMUNE_ATKSPEED);
		int32 aggro = caster->CheckAggroAmount(spell_id, this);
		if(aggro > 0) {
			AddToHateList(caster, aggro);
		} else {
			AddToHateList(caster, 1,0,true,false,false,spell_id);
		}
		return true;
	}

	// client vs client fear
	if(IsEffectInSpell(spell_id, SpellEffect::Fear))
	{
		effect_index = GetSpellEffectIndex(spell_id, SpellEffect::Fear);
		if(GetSpecialAbility(SpecialAbility::FearImmunity)) {
			LogSpells("We are immune to Fear spells");
			caster->MessageString(Chat::Red, IMMUNE_FEAR);	// need to verify message type, not in MQ2Cast for easy look up
			int32 aggro = caster->CheckAggroAmount(spell_id, this);
			if(aggro > 0) {
				AddToHateList(caster, aggro);
			} else {
				AddToHateList(caster, 1,0,true,false,false,spell_id);
			}
			return true;
		} else if(IsClient() && caster->IsClient() && (caster->CastToClient()->GetGM() == false))
		{
			LogSpells("Clients cannot fear eachother!");
			caster->MessageString(Chat::Red, IMMUNE_FEAR);	// need to verify message type, not in MQ2Cast for easy look up
			return true;
		}
		else if(GetLevel() > spells[spell_id].max_value[effect_index] && spells[spell_id].max_value[effect_index] != 0)
		{
			LogSpells("Level is [{}], cannot be feared by this spell", GetLevel());
			caster->MessageString(Chat::Shout, FEAR_TOO_HIGH);
			int32 aggro = caster->CheckAggroAmount(spell_id, this);
			if (aggro > 0) {
				AddToHateList(caster, aggro);
			} else {
				AddToHateList(caster, 1,0,true,false,false,spell_id);
			}
			return true;
		}
		else if (CheckAATimer(aaTimerWarcry))
		{
			Message(Chat::Red, "Your are immune to fear.");
			LogSpells("Clients has WarCry effect, immune to fear!");
			caster->MessageString(Chat::Red, IMMUNE_FEAR);	// need to verify message type, not in MQ2Cast for easy look up
			return true;
		}
	}

	if(IsCharmSpell(spell_id))
	{
		if(GetSpecialAbility(SpecialAbility::CharmImmunity))
		{
			LogSpells("We are immune to Charm spells");
			caster->MessageString(Chat::Red, CANNOT_CHARM);	// need to verify message type, not in MQ2Cast for easy look up
			int32 aggro = caster->CheckAggroAmount(spell_id, this);
			if(aggro > 0) {
				AddToHateList(caster, aggro);
			} else {
				AddToHateList(caster, 1,0,true,false,false,spell_id);
			}
			return true;
		}

		if(this == caster)
		{
			LogSpells("You are immune to your own charms");
			caster->Message(Chat::Red, "You cannot charm yourself.");	// need to look up message?
			return true;
		}

		//let npcs cast whatever charm on anyone
		if(!caster->IsNPC())
		{
			// check level limit of charm spell
			effect_index = GetSpellEffectIndex(spell_id, SpellEffect::Charm);
			assert(effect_index >= 0);
			if(GetLevel() > spells[spell_id].max_value[effect_index] && spells[spell_id].max_value[effect_index] != 0)
			{
				LogSpells("Our level ([{}]) is higher than the limit of this Charm spell ([{}])", GetLevel(), spells[spell_id].max_value[effect_index]);
				if (RuleB(Spells, CharmAggroOverLevel)) {
					int64 aggro = caster->CheckAggroAmount(spell_id, this);
					aggro > 0 ? AddToHateList(caster, aggro) : AddToHateList(caster, 1, 0, true, false, false, spell_id);
					caster->MessageString(Chat::SpellFailure, TARGET_RESISTED, spells[spell_id].name);
					return true;
				}

				caster->MessageString(Chat::Red, CANNOT_CHARM_YET);	// need to verify message type, not in MQ2Cast for easy look up<Paste>
				AddToHateList(caster, 1,0,true,false,false,spell_id);
				return true;
			}
		}
	}

	if
	(
		IsEffectInSpell(spell_id, SpellEffect::Root) ||
		IsEffectInSpell(spell_id, SpellEffect::MovementSpeed)
	)
	{
		if(GetSpecialAbility(SpecialAbility::SnareImmunity)) {
			LogSpells("We are immune to Snare spells");
			caster->MessageString(Chat::Red, IMMUNE_MOVEMENT);
			int32 aggro = caster->CheckAggroAmount(spell_id, this);
			if(aggro > 0) {
				AddToHateList(caster, aggro);
			} else {
				AddToHateList(caster, 1,0,true,false,false,spell_id);
			}
			return true;
		}
	}

	if(IsLifetapSpell(spell_id))
	{
		if(this == caster)
		{
			LogSpells("You cannot lifetap yourself");
			caster->MessageString(Chat::SpellFailure, CANT_DRAIN_SELF);
			return true;
		}
	}

	if(IsSacrificeSpell(spell_id))
	{
		if(this == caster)
		{
			LogSpells("You cannot sacrifice yourself");
			caster->MessageString(Chat::SpellFailure, CANNOT_SAC_SELF);
			return true;
		}
	}

	LogSpells("No immunities to spell [{}] found", spell_id);

	return false;
}

int Mob::GetResist(uint8 resist_type)
{
	switch(resist_type)
	{
	case RESIST_FIRE:
		return GetFR();
	case RESIST_COLD:
		return GetCR();
	case RESIST_MAGIC:
		return GetMR();
	case RESIST_DISEASE:
		return GetDR();
	case RESIST_POISON:
		return GetPR();
	case RESIST_CORRUPTION:
		return GetCorrup();
	case RESIST_PRISMATIC:
		return (GetFR() + GetCR() + GetMR() + GetDR() + GetPR()) / 5;
	case RESIST_CHROMATIC:
		return std::min({GetFR(), GetCR(), GetMR(), GetDR(), GetPR()});
	case RESIST_PHYSICAL:
		if (IsNPC())
			return GetPhR();
		else
			return 0;
	default:
		return 0;
	}
}

//
// Spell resists:
// returns an effectiveness index from 0 to 100. for most spells, 100 means
// it landed, and anything else means it was resisted; however there are some
// spells that can be partially effective, and this value can be used there.
// TODO: we need to figure out how the following pvp values work and implement them
// pvp_resist_base
// pvp_resist_calc
// pvp_resist_cap
float Mob::ResistSpell(uint8 resist_type, uint16 spell_id, Mob *caster, bool use_resist_override, int resist_override, bool CharismaCheck, bool CharmTick, bool IsRoot, int level_override)
{

	if(!caster)
	{
		return 100;
	}

	if(spell_id != 0 && !IsValidSpell(spell_id))
	{
		return 0;
	}

	if(GetSpecialAbility(SpecialAbility::CastingFromRangeImmunity))
	{
		if(!caster->CombatRange(this))
		{
			return(0);
		}
	}

	if(GetSpecialAbility(SpecialAbility::MagicImmunity))
	{
		LogSpells("We are immune to magic, so we fully resist the spell [{}]", spell_id);
		return(0);
	}

	//Get resist modifier and adjust it based on focus 2 resist about eq to 1% resist chance
	int resist_modifier = 0;
	if (use_resist_override) {
		resist_modifier = resist_override;
	} else {
		// PVP, we don't have the normal per_level or cap stuff implemented ... so ahh do that
		// and make sure the PVP versions are also handled.
		if (IsClient() && caster->IsClient()) {
			resist_modifier = spells[spell_id].pvp_resist_base;
		} else {
			resist_modifier = spells[spell_id].resist_difficulty;
		}
	}

	if(caster->GetSpecialAbility(SpecialAbility::CastingResistDifficulty))
		resist_modifier += caster->GetSpecialAbilityParam(SpecialAbility::CastingResistDifficulty, 0);

	int64 focus_resist = caster->GetFocusEffect(focusResistRate, spell_id);

	resist_modifier -= 2 * focus_resist;

	int64 focus_incoming_resist = GetFocusEffect(focusFcResistIncoming, spell_id, caster);

	resist_modifier -= focus_incoming_resist;

	//Check for fear resist
	bool IsFear = false;
	if(IsFearSpell(spell_id))
	{
		IsFear = true;
		int fear_resist_bonuses = CalcFearResistChance();
		if(zone->random.Roll(fear_resist_bonuses))
		{
			LogSpells("Resisted spell in fear resistance, had [{}] chance to resist", fear_resist_bonuses);
			return 0;
		}
	}

	if (!CharmTick) {
		//Check for Spell Effect specific resistance chances (ie AA Mental Fortitude)
		if (resist_type != RESIST_NONE && TrySpellEffectResist(spell_id)) {
			return 0;
		}

		// Check for Chance to Resist Spell bonuses (ie Sanctification Discipline)
		if (!spells[spell_id].no_resist && resist_type != RESIST_NONE) {
			int resist_bonuses = CalcResistChanceBonus();
			if (resist_bonuses && zone->random.Roll(resist_bonuses) && !IsResurrectionSicknessSpell(spell_id)) {
				LogSpells("Resisted spell in sanctification, had [{}] chance to resist", resist_bonuses);
				return 0;
			}
		}
	}

	// Special case. If the caster has the Unholy Aura Discipline activated and the spell is HT,
	// or improved HT then the resist type is disease.
	if (IsHarmTouchSpell(spell_id) && caster->IsClient() && caster->CastToClient()->FindBuff(DISC_UNHOLY_AURA)) {
		resist_type = RESIST_DISEASE;
	}

	//Get the resist chance for the target
	if (resist_type == RESIST_NONE || spells[spell_id].no_resist) {
		LogSpells("Spell was unresistable");
		return 100;
	}

	int target_resist = GetResist(resist_type);

	// JULY 24, 2002 changes
	int level = GetLevel();
	if (RuleB(Spells,July242002PetResists) && IsPetOwnerOfClientBot() && caster->IsNPC() && !caster->IsPetOwnerOfClientBot()) {
		auto owner = GetOwner();
		if (owner != nullptr) {
			target_resist = std::max(target_resist, owner->GetResist(resist_type));
			level = owner->GetLevel();
		}
	}

	//Setup our base resist chance.
	int resist_chance = 0;
	int level_mod = 0;

	//Adjust our resist chance based on level modifiers
	uint8 caster_level = level_override > 0 ? level_override : caster->GetLevel();
	int temp_level_diff = level - caster_level;

	//Physical Resists are calclated using their own formula derived from extensive parsing.
	if (resist_type == RESIST_PHYSICAL) {
		level_mod = ResistPhysical(temp_level_diff, caster_level);
	}

	else {

		if(IsNPC() && level >= RuleI(Casting,ResistFalloff))
		{
			int a = (RuleI(Casting,ResistFalloff)-1) - caster_level;
			if(a > 0)
			{
				temp_level_diff = a;
			}
			else
			{
				temp_level_diff = 0;
			}
		}

		if(IsOfClientBot()&& level >= 21 && temp_level_diff > 15)
		{
			temp_level_diff = 15;
		}

		if(IsNPC() && temp_level_diff < -9)
		{
			temp_level_diff = -9;
		}

		level_mod = temp_level_diff * temp_level_diff / 2;
		if(temp_level_diff < 0)
		{
			level_mod = -level_mod;
		}

		if(IsNPC() && (caster_level - level) < -20)
		{
			level_mod = 1000;
		}

		//Even more level stuff this time dealing with damage spells
		if(IsNPC() && IsDamageSpell(spell_id) && level >= 17)
		{
			int level_diff;
			if(level >= RuleI(Casting,ResistFalloff))
			{
				level_diff = (RuleI(Casting,ResistFalloff)-1) - caster_level;
				if(level_diff < 0)
				{
					level_diff = 0;
				}
			}
			else
			{
				level_diff = level - caster_level;
			}
			level_mod += (2 * level_diff);
		}
	}

	if (CharismaCheck)
	{
		/*
		Charisma ONLY effects the initial resist check when charm is cast with 10 CHA = -1 Resist mod up to 255 CHA (min ~ 75 cha)
		Charisma less than ~ 75 gives a postive modifier to resist checks at approximate ratio of -10 CHA = +6 Resist.
		Mez spells do same initial resist check as a above.
		Lull spells only check charisma if inital cast is resisted to see if mob will aggro, same modifier/cap as above.
		Charisma DOES NOT extend charm durations.
		Fear resist chance is given a -20 resist modifier if CHA is < 100, from 100-255 it progressively reduces the negative mod to 0.
		Fears verse undead DO NOT apply a charisma modifer. (Note: unknown Base1 values defined in undead fears do not effect duration).
		*/
		int16 charisma = caster->GetCHA();

		if (IsFear && (spells[spell_id].target_type != ST_Undead)){

			if (charisma < 100)
				resist_modifier -= 20;

			else if (charisma <= 255)
				resist_modifier += (charisma - 100)/8;
		}

		else {

			if (charisma >= 75){

				if (charisma > RuleI(Spells, CharismaEffectivenessCap))
					charisma = RuleI(Spells, CharismaEffectivenessCap);

				resist_modifier -= (charisma - 75)/RuleI(Spells, CharismaEffectiveness);
			}
			else
				resist_modifier += ((75 - charisma)/10) * 6; //Increase Resist Chance
		}

	}


	//Lull spells DO NOT use regular resists on initial cast, instead they use a flat +15 modifier. Live parses confirm this.
	//Regular resists are used when checking if mob will aggro off of a lull resist.
	if(!CharismaCheck && IsHarmonySpell(spell_id))
		target_resist = 15;

	//Add our level, resist and -spell resist modifier to our roll chance
	resist_chance += level_mod;
	if (RuleB(Spells, EnableResistSoftCap)) {
		int soft_cap_level_modifier;

		if (GetLevel() > 60) {
			soft_cap_level_modifier = (GetLevel() - 60) * 10;
		}

		if ((target_resist + resist_modifier) > (RuleI(Spells, SpellResistSoftCap) + soft_cap_level_modifier)) {
			resist_chance += RuleI(Spells, SpellResistSoftCap) + soft_cap_level_modifier;
		} else {
			resist_chance += target_resist + resist_modifier;
		}
	} else {
		resist_chance += resist_modifier;
		resist_chance += target_resist;
	}

	//Do our min and max resist checks.
	if(resist_chance > spells[spell_id].max_resist && spells[spell_id].max_resist != 0)
	{
		resist_chance = spells[spell_id].max_resist;
	}

	if(resist_chance < spells[spell_id].min_resist && spells[spell_id].min_resist != 0)
	{
		resist_chance = spells[spell_id].min_resist;
	}

	//Average charm duration agianst mobs with 0% chance to resist on LIVE is ~ 68 ticks.
	//Minimum resist chance should be caclulated factoring in the RuleI(Spells, CharmBreakCheckChance)
	if (CharmTick) {

		float min_charmbreakchance = ((100.0f/static_cast<float>(RuleI(Spells, CharmBreakCheckChance)))/66.0f * 100.0f)*2.0f;
		if (resist_chance < static_cast<int>(min_charmbreakchance))
			resist_chance = min_charmbreakchance;
	}

	//Average root duration agianst mobs with 0% chance to resist on LIVE is ~ 22 ticks (6% resist chance).
	//Minimum resist chance should be caclulated factoring in the RuleI(Spells, RootBreakCheckChance)
	if (IsRoot) {

		float min_rootbreakchance = ((100.0f/static_cast<float>(RuleI(Spells, RootBreakCheckChance)))/22.0f * 100.0f)*2.0f;

		if (resist_chance < static_cast<int>(min_rootbreakchance))
			resist_chance = min_rootbreakchance;
	}

	//Finally our roll
	int roll = zone->random.Int(0, RuleB(Spells, EnableResistSoftCap) ? RuleI(Spells, SpellResistSoftCap) : 200);

	if(roll > resist_chance) {
		return 100;
	} else {
		//This is confusing but it's basically right
		//It skews partial resists up over 100 more often than not
		if(!IsPartialResistableSpell(spell_id))
		{
			return 0;
		}
		else
		{
			if(resist_chance < 1)
			{
				resist_chance = 1;
			}

			int partial_modifier = ((150 * (resist_chance - roll)) / resist_chance);

			if(IsNPC())
			{
				if(level > caster_level && level >= 17 && caster_level <= 50)
				{
					partial_modifier += 5;
				}

				if(level >= 30 && caster_level < 50)
				{
					partial_modifier += (caster_level - 25);
				}

				if(level < 15)
				{
					partial_modifier -= 5;
				}
			}

			if(caster->IsNPC())
			{
				if((level - caster_level) >= 20)
				{
					partial_modifier += (level - caster_level) * 1.5;
				}
			}

			if(partial_modifier <= 0)
			{
				return 100;
			}
			else if(partial_modifier >= 100)
			{
				return 0;
			}

			return (100.0f - partial_modifier);
		}
	}
}

int Mob::ResistPhysical(int level_diff, uint8 caster_level)
{
	/*	Physical resists use the standard level mod calculation in
	conjunction with a resist fall off formula that greatly prevents you
	from landing abilities on mobs that are higher level than you.
	After level 12, every 4 levels gained the max level you can hit
	your target without a sharp resist penalty is raised by 1.
	Extensive parsing confirms this, along with baseline phyiscal resist rates used.
	*/


	if (level_diff == 0)
		return level_diff;

	int level_mod = 0;

	if (level_diff > 0) {

		int ResistFallOff = 0;

		if (caster_level <= 12)
			ResistFallOff = 3;
		else
			ResistFallOff = caster_level/4;

		if (level_diff > ResistFallOff || level_diff >= 15)
			level_mod = ((level_diff * 10) + level_diff)*2;
		else
			level_mod = level_diff * level_diff / 2;
	}

	else
		level_mod = -(level_diff * level_diff / 2);

	return level_mod;
}

int16 Mob::CalcResistChanceBonus()
{
	int resistchance = spellbonuses.ResistSpellChance + itembonuses.ResistSpellChance;

	if (IsOfClientBot()) {
		resistchance += aabonuses.ResistSpellChance;
	}
	return resistchance;
}

int16 Mob::CalcFearResistChance()
{
	int resist_chance = spellbonuses.ResistFearChance + itembonuses.ResistFearChance + aabonuses.ResistFearChance;
	if (spellbonuses.Fearless || itembonuses.Fearless || aabonuses.Fearless) {
		resist_chance = 100;
	}

	return resist_chance;
}

float Mob::GetAOERange(uint16 spell_id)
{
	float range = spells[spell_id].aoe_range;

	/**
	 * For TGB
	 */
	if (range == 0) {
		range = spells[spell_id].range;
	}

	if (range == 0) {
		range = 10;
	}

	if (IsBardSong(spell_id) && IsBeneficialSpell(spell_id)) {
		//Live AA - Extended Notes, SionachiesCrescendo
		float song_bonus = static_cast<float>(aabonuses.SongRange + spellbonuses.SongRange + itembonuses.SongRange);
		range += range * song_bonus / 100.0f;
	}

	return GetActSpellRange(spell_id, range);
}

///////////////////////////////////////////////////////////////////////////////
// 'other' functions

void Mob::Spin() {
	if(IsClient()) {
		auto outapp = new EQApplicationPacket(OP_Action, sizeof(Action_Struct));
		outapp->pBuffer[0] = 0x0B;
		outapp->pBuffer[1] = 0x0A;
		outapp->pBuffer[2] = 0x0B;
		outapp->pBuffer[3] = 0x0A;
		outapp->pBuffer[4] = 0xE7;
		outapp->pBuffer[5] = 0x00;
		outapp->pBuffer[6] = 0x4D;
		outapp->pBuffer[7] = 0x04;
		outapp->pBuffer[8] = 0x00;
		outapp->pBuffer[9] = 0x00;
		outapp->pBuffer[10] = 0x00;
		outapp->pBuffer[11] = 0x00;
		outapp->pBuffer[12] = 0x00;
		outapp->pBuffer[13] = 0x00;
		outapp->pBuffer[14] = 0x00;
		outapp->pBuffer[15] = 0x00;
		outapp->pBuffer[16] = 0x00;
		outapp->pBuffer[17] = 0x00;
		outapp->pBuffer[18] = 0xD4;
		outapp->pBuffer[19] = 0x43;
		outapp->pBuffer[20] = 0x00;
		outapp->pBuffer[21] = 0x00;
		outapp->pBuffer[22] = 0x00;
		outapp->priority = 5;
		CastToClient()->QueuePacket(outapp);
		safe_delete(outapp);
	}
	else {
		float x,y,z,h;

		x=GetX();
		y=GetY();
		z=GetZ();
		h=GetHeading()+5;

		if (IsCorpse() || (IsClient() && !IsAIControlled())) {
			m_Position.x = x;
			m_Position.y = y;
			m_Position.z = z;
			mMovementManager->SendCommandToClients(this, 0.0, 0.0, 0.0, 0.0, 0, ClientRangeAny);
		}
		else {
			Teleport(glm::vec4(x, y, z, h));
		}
	}
}

void Mob::SendSpellBarDisable()
{
	if (!IsClient())
		return;

	CastToClient()->MemorizeSpell(0, SPELLBAR_UNLOCK, memSpellSpellbar);
}

// this puts the spell bar back into a usable state fast
void Mob::SendSpellBarEnable(uint16 spell_id)
{
	if(!IsClient())
		return;

	auto outapp = new EQApplicationPacket(OP_ManaChange, sizeof(ManaChange_Struct));
	ManaChange_Struct* manachange = (ManaChange_Struct*)outapp->pBuffer;
	manachange->new_mana = GetMana();
	manachange->spell_id = spell_id;
	manachange->stamina = CastToClient()->GetEndurance();
	manachange->keepcasting = 0;
	manachange->slot = CastToClient()->FindMemmedSpellBySpellID(spell_id);
	outapp->priority = 6;
	CastToClient()->QueuePacket(outapp);
	safe_delete(outapp);
}

void Mob::Stun(int duration)
{
	//make sure a shorter stun does not overwrite a longer one.
	if(stunned && stunned_timer.GetRemainingTime() > uint32(duration))
		return;

	auto spell_id = bardsong ? bardsong : casting_spell_id;

	if(IsValidSpell(spell_id) && !spells[spell_id].uninterruptable) {
		int persistent_casting = spellbonuses.PersistantCasting + itembonuses.PersistantCasting + aabonuses.PersistantCasting;

		if(zone->random.Int(0,99) > persistent_casting)
			InterruptSpell(spell_id);
	}

	if(duration > 0)
	{
		stunned = true;
		stunned_timer.Start(duration);
		SendAddPlayerState(PlayerState::Stunned);
	}
}

void Mob::UnStun() {
	if(stunned && stunned_timer.Enabled()) {
		stunned = false;
		stunned_timer.Disable();
		SendRemovePlayerState(PlayerState::Stunned);
	}
}

// Stuns "this"
void Client::Stun(int duration)
{
	Mob::Stun(duration);

	auto outapp = new EQApplicationPacket(OP_Stun, sizeof(Stun_Struct));
	Stun_Struct* stunon = (Stun_Struct*) outapp->pBuffer;
	stunon->duration = duration;
	outapp->priority = 5;
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::UnStun() {
	Mob::UnStun();

	auto outapp = new EQApplicationPacket(OP_Stun, sizeof(Stun_Struct));
	Stun_Struct* stunon = (Stun_Struct*) outapp->pBuffer;
	stunon->duration = 0;
	outapp->priority = 5;
	QueuePacket(outapp);
	safe_delete(outapp);
}

void NPC::Stun(int duration) {
	Mob::Stun(duration);
	StopNavigation();
}

void NPC::UnStun() {
	Mob::UnStun();
}

void Mob::Mesmerize()
{
	mezzed = true;

	auto spell_id = bardsong ? bardsong : casting_spell_id;

	if (spell_id)
		InterruptSpell(spell_id);

	StopNavigation();
}

void Client::MakeBuffFadePacket(uint16 spell_id, int slot_id, bool send_message)
{
	EQApplicationPacket* outapp = nullptr;

	outapp = new EQApplicationPacket(OP_Buff, sizeof(SpellBuffPacket_Struct));
	SpellBuffPacket_Struct* sbf = (SpellBuffPacket_Struct*) outapp->pBuffer;

	sbf->entityid = GetID();
	// i dont know why but this works.. for now
	sbf->buff.effect_type = 2;
//	sbf->slot=m_pp.buffs[slot_id].slotid;
//	sbf->level=m_pp.buffs[slot_id].level;
//	sbf->effect=m_pp.buffs[slot_id].effect;
	sbf->buff.spellid = spell_id;
	sbf->slotid = slot_id;
	sbf->bufffade = 1;
#if EQDEBUG >= 11
	printf("Sending SBF 1 from server:\n");
	DumpPacket(outapp);
#endif
	QueuePacket(outapp);

/*
	sbf->effect=0;
	sbf->level=0;
	sbf->slot=0;
*/
	sbf->buff.spellid = 0xffffffff;
#if EQDEBUG >= 11
	printf("Sending SBF 2 from server:\n");
	DumpPacket(outapp);
#endif
	QueuePacket(outapp);
	safe_delete(outapp);

	if(send_message)
	{
		const char *fadetext = spells[spell_id].spell_fades;
		outapp = new EQApplicationPacket(OP_ColoredText, sizeof(ColoredText_Struct) + strlen(fadetext));
		ColoredText_Struct *bfm = (ColoredText_Struct *) outapp->pBuffer;
		bfm->color = Chat::Spells;
		memcpy(bfm->msg, fadetext, strlen(fadetext));
		QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void Client::MemSpell(uint16 spell_id, int slot, bool update_client)
{
	if (slot >= EQ::spells::SPELL_GEM_COUNT || slot < 0) {
		return;
	}

	if(update_client) {
		if (IsValidSpell(m_pp.mem_spells[slot])) {
			UnmemSpell(slot, update_client);
		}
	}

	m_pp.mem_spells[slot] = spell_id;
	LogSpells("Spell [{}] memorized into slot [{}]", spell_id, slot);

	database.SaveCharacterMemorizedSpell(CharacterID(), m_pp.mem_spells[slot], slot);

	if(update_client) {
		MemorizeSpell(slot, spell_id, memSpellMemorize);
	}
}

void Client::UnmemSpell(int slot, bool update_client)
{
	if (slot >= EQ::spells::SPELL_GEM_COUNT || slot < 0) {
		return;
	}

	LogSpells("Spell [{}] forgotten from slot [{}]", m_pp.mem_spells[slot], slot);

	database.DeleteCharacterMemorizedSpell(CharacterID(), slot);

	if (update_client) {
		MemorizeSpell(slot, m_pp.mem_spells[slot], memSpellForget);
	}

	m_pp.mem_spells[slot] = UINT32_MAX;
}

void Client::UnmemSpellBySpellID(int32 spell_id)
{
	auto spell_gem = FindMemmedSpellBySpellID(spell_id);
	if (spell_gem >= EQ::spells::SPELL_GEM_COUNT || spell_gem < 0) {
		return;
	}

	UnmemSpell(spell_gem);
}

void Client::UnmemSpellAll(bool update_client)
{
	for (int spell_gem = 0; spell_gem < EQ::spells::SPELL_GEM_COUNT; spell_gem++) {
		if (IsValidSpell(m_pp.mem_spells[spell_gem])) {
			UnmemSpell(spell_gem, update_client);
		}
	}
}

uint32 Client::GetSpellIDByBookSlot(int book_slot) {
	if (book_slot <= EQ::spells::SPELLBOOK_SIZE) {
		return GetSpellByBookSlot(book_slot);
	}
	return -1;
}

int Client::FindEmptyMemSlot() {
	for (int spell_gem = 0; spell_gem < EQ::spells::SPELL_GEM_COUNT; spell_gem++) {
		if (!IsValidSpell(m_pp.mem_spells[spell_gem])) {
			return spell_gem;
		}
	}
	return -1;
}

uint16 Client::FindMemmedSpellBySlot(int slot) {
	if (IsValidSpell(m_pp.mem_spells[slot])) {
		return m_pp.mem_spells[slot];
	}
	return 0;
}

int Client::MemmedCount() {
	int memmed_count = 0;
	for (int spell_gem = 0; spell_gem < EQ::spells::SPELL_GEM_COUNT; spell_gem++) {
		if (IsValidSpell(m_pp.mem_spells[spell_gem])) {
			memmed_count++;
		}
	}
	return memmed_count;
}

int Client::FindMemmedSpellBySpellID(uint16 spell_id) {
	for (int spell_gem = 0; spell_gem < EQ::spells::SPELL_GEM_COUNT; spell_gem++) {
		if (IsValidSpell(m_pp.mem_spells[spell_gem]) && m_pp.mem_spells[spell_gem] == spell_id) {
			return spell_gem;
		}
	}
	return -1;
}


void Client::ScribeSpell(uint16 spell_id, int slot, bool update_client, bool defer_save)
{
	if (slot >= EQ::spells::SPELLBOOK_SIZE || slot < 0) {
		return;
	}

	if (update_client) {
		if (m_pp.spell_book[slot] != UINT32_MAX) {
			UnscribeSpell(slot, update_client, defer_save);
		}
	}

	m_pp.spell_book[slot] = spell_id;

	// defer save if we're bulk saving elsewhere
	if (!defer_save) {
		database.SaveCharacterSpell(CharacterID(), spell_id, slot);
	}
	LogSpells("Spell [{}] scribed into spell book slot [{}]", spell_id, slot);

	if (update_client) {
		MemorizeSpell(slot, spell_id, memSpellScribing);
	}
}

void Client::UnscribeSpell(int slot, bool update_client, bool defer_save)
{
	if (!EQ::ValueWithin(slot, 0, (EQ::spells::SPELLBOOK_SIZE - 1))) {
		return;
	}

	LogSpells("Spell [{}] erased from spell book slot [{}]", m_pp.spell_book[slot], slot);

	if (!defer_save) {
		database.DeleteCharacterSpell(CharacterID(), slot);
	}

	if (update_client && slot < EQ::spells::DynamicLookup(ClientVersion(), GetGM())->SpellbookSize) {
		auto  outapp = new EQApplicationPacket(OP_DeleteSpell, sizeof(DeleteSpell_Struct));
		auto* del = (DeleteSpell_Struct *) outapp->pBuffer;

		del->spell_slot = slot;
		del->success    = 1;

		QueuePacket(outapp);
		safe_delete(outapp);
	}

	if (parse->PlayerHasQuestSub(EVENT_UNSCRIBE_SPELL)) {
		const auto export_string = fmt::format("{} {}", slot, m_pp.spell_book[slot]);
		parse->EventPlayer(EVENT_UNSCRIBE_SPELL, this, export_string, 0);
	}

	m_pp.spell_book[slot] = UINT32_MAX;
}

void Client::UnscribeSpellAll(bool update_client)
{
	for (int i = 0; i < EQ::spells::SPELLBOOK_SIZE; i++) {
		if (m_pp.spell_book[i] != 0xFFFFFFFF) {
			UnscribeSpell(i, update_client, true);
		}
	}

	// bulk save at end (this will only delete)
	SaveSpells();
}

void Client::UnscribeSpellBySpellID(uint16 spell_id, bool update_client)
{
	for (int index = 0; index < EQ::spells::SPELLBOOK_SIZE; index++) {
		if (IsValidSpell(m_pp.spell_book[index]) && m_pp.spell_book[index] == spell_id) {
			UnscribeSpell(index, update_client, true);
			break;
		}
	}
}

void Client::UntrainDisc(int slot, bool update_client, bool defer_save)
{
	if (slot >= MAX_PP_DISCIPLINES || slot < 0) {
		return;
	}

	LogSpells("Discipline [{}] untrained from slot [{}]", m_pp.disciplines.values[slot], slot);
	m_pp.disciplines.values[slot] = 0;

	if (!defer_save) {
		database.DeleteCharacterDiscipline(CharacterID(), slot);
	}

	if (update_client) {
		SendDisciplineUpdate();
	}
}

void Client::UntrainDiscAll(bool update_client)
{
	for (int i = 0; i < MAX_PP_DISCIPLINES; i++) {
		if (m_pp.disciplines.values[i] != 0) {
			UntrainDisc(i, update_client, true);
		}
	}

	// bulk delete / save
	SaveDisciplines();
}

void Client::UntrainDiscBySpellID(uint16 spell_id, bool update_client)
{
	for (int slot = 0; slot < MAX_PP_DISCIPLINES; slot++) {
		if (m_pp.disciplines.values[slot] == spell_id) {
			UntrainDisc(slot, update_client);
			return;
		}
	}
}

int Client::GetNextAvailableSpellBookSlot(int starting_slot) {
	for (int i = starting_slot; i < EQ::spells::SPELLBOOK_SIZE; i++) {	//using starting_slot should help speed this up when we're iterating through a bunch of spells
		if (!IsValidSpell(GetSpellByBookSlot(i)))
			return i;
	}

	return -1;	//default
}

int Client::FindSpellBookSlotBySpellID(uint16 spellid) {
	for(int i = 0; i < EQ::spells::SPELLBOOK_SIZE; i++) {
		if(m_pp.spell_book[i] == spellid)
			return i;
	}

	return -1;	//default
}

uint32 Client::GetHighestScribedSpellinSpellGroup(uint32 spell_group)
{
	//Typical live spells follow 1/5/10 rank value for actual ranks 1/2/3, but this can technically be set as anything.

	int highest_rank = 0; //highest ranked found in spellgroup
	uint32 highest_spell_id = 0;  //spell_id of the highest ranked spell you have scribed in that spell rank.

	for (int i = 0; i < EQ::spells::SPELLBOOK_SIZE; i++) {

		if (IsValidSpell(m_pp.spell_book[i])) {
			if (spells[m_pp.spell_book[i]].spell_group == spell_group) {
				if (highest_rank < spells[m_pp.spell_book[i]].rank) {
					highest_rank = spells[m_pp.spell_book[i]].rank;
					highest_spell_id = m_pp.spell_book[i];
				}
			}
		}
	}
	return highest_spell_id;
}

std::unordered_map<uint32, std::vector<uint16>> Client::LoadSpellGroupCache(uint8 min_level, uint8 max_level) {
	std::unordered_map<uint32, std::vector<uint16>> spell_group_cache;

	const auto query = fmt::format(
		"SELECT a.spellgroup, a.id, a.rank "
		"FROM spells_new a "
		"INNER JOIN ("
		"SELECT spellgroup, MAX(`rank`) `rank` "
		"FROM spells_new "
		"GROUP BY spellgroup) "
		"b ON a.spellgroup = b.spellgroup AND a.rank = b.rank "
		"WHERE a.spellgroup IN (SELECT DISTINCT spellgroup FROM spells_new WHERE spellgroup != 0 and classes{} BETWEEN {} AND {}) ORDER BY `rank` DESC",
		m_pp.class_, min_level, max_level
	);

	auto results = content_db.QueryDatabase(query);
	if (!results.Success() || !results.RowCount()) {
		return spell_group_cache;
	}

	for (auto row : results) {
		spell_group_cache[Strings::ToUnsignedInt(row[0])].push_back(static_cast<uint16>(Strings::ToUnsignedInt(row[1])));
	}

	return spell_group_cache;
}

bool Client::SpellGlobalCheck(uint16 spell_id, uint32 character_id) {
	std::string query = fmt::format(
		"SELECT qglobal, value FROM spell_globals WHERE spellid = {}",
		spell_id
	);

	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return false; // Query failed, do not allow scribing.
	}

	if (!results.RowCount()) {
		return true; // Spell ID isn't listed in the spell_globals table, allow scribing,
	}

	auto row = results.begin();
	std::string spell_global_name = row[0];
	std::string spell_global_value = row[1];

	if (spell_global_name.empty()) {
		return true; // If the entry in the spell_globals table has nothing set for the qglobal name, allow scribing.
	}

	query = fmt::format(
		"SELECT value FROM quest_globals WHERE charid = {} AND name = '{}'",
		character_id,
		Strings::Escape(spell_global_name)
	);

	results = database.QueryDatabase(query);
	if (!results.Success()) {
		LogError(
			"Spell global [{}] for spell ID [{}] for character ID [{}] query failed.",
			spell_global_name,
			spell_id,
			character_id
		);

		return false; // Query failed, do not allow scribing.
	}

	if (!results.RowCount()) {
		LogError(
			"Spell global [{}] for spell ID [{}] for character ID [{}] does not exist.",
			spell_global_name,
			spell_id,
			character_id
		);

		return false; // No rows found, do not allow scribing.
	}

	row = results.begin();
	std::string global_value = row[0];
	if (Strings::IsNumber(global_value) && Strings::IsNumber(spell_global_value)) {
		if (Strings::ToInt(global_value) >= Strings::ToInt(spell_global_value)) {
			return true; // If value is greater than or equal to spell global value, allow scribing.
		}
	} else {
		if (global_value == spell_global_value) {
			return true; // If value is equal to spell bucket value, allow scribing.
		}
	}

	// If user's qglobal does not meet requirements, do not allow scribing.
	LogError(
		"Spell global [{}] for spell ID [{}] for character ID [{}] did not match value [{}] value found was [{}].",
		spell_global_name,
		spell_id,
		character_id,
		spell_global_value,
		global_value
	);

	return false;
}

bool Client::SpellBucketCheck(uint16 spell_id, uint32 character_id)
{
	const auto& e = SpellBucketsRepository::FindOne(database, spell_id);
	if (!e.spell_id || e.bucket_name.empty() || e.bucket_value.empty()) {
		return true;
	}

	auto k = GetScopedBucketKeys();

	k.key = e.bucket_name;

	const auto& b = DataBucket::GetData(&database, k);

	return zone->CompareDataBucket(e.bucket_comparison, e.bucket_value, b.value);
}

// TODO get rid of this
int16 Mob::GetBuffSlotFromType(uint16 type) {
	uint32 buff_count = GetMaxTotalSlots();
	for (int i = 0; i < buff_count; i++) {
		if (IsValidSpell(buffs[i].spellid)) {
			for (int j = 0; j < EFFECT_COUNT; j++) {
				if (spells[buffs[i].spellid].effect_id[j] == type )
					return i;
			}
		}
	}
	return -1;
}

uint16 Mob::GetSpellIDFromSlot(uint8 slot)
{
	if (IsValidSpell(buffs[slot].spellid)) {
		return buffs[slot].spellid;
	}
	return 0;
}

bool Mob::FindType(uint16 type, bool bOffensive, uint16 threshold) {
	int buff_count = GetMaxTotalSlots();
	for (int i = 0; i < buff_count; i++) {
		if (IsValidSpell(buffs[i].spellid)) {
			for (int j = 0; j < EFFECT_COUNT; j++) {
				// adjustments necessary for offensive npc casting behavior
				if (bOffensive) {
					if (spells[buffs[i].spellid].effect_id[j] == type) {
						int64 value =
								CalcSpellEffectValue_formula(spells[buffs[i].spellid].buff_duration_formula,
											spells[buffs[i].spellid].base_value[j],
											spells[buffs[i].spellid].max_value[j],
											buffs[i].casterlevel, buffs[i].spellid);
						LogSpells(
							"FindType type [{}] value [{}] threshold [{}]",
							type,
							value,
							threshold
						);
						if (value < threshold)
							return true;
					}
				} else {
					if (spells[buffs[i].spellid].effect_id[j] == type )
						return true;
				}
			}
		}
	}
	return false;
}

bool Mob::IsCombatProc(uint16 spell_id) {

	if (RuleB(Spells, FocusCombatProcs)) {
		return false;
	}

	if (!IsValidSpell(spell_id)) {
		return(false);
	}
	/*
		Procs that originate from casted spells are still limited by SPA 311 (~Kayen confirmed on live 2/4/22)
	*/
	for (int i = 0; i < m_max_procs; i++) {
		if (PermaProcs[i].spellID == spell_id ||
			SpellProcs[i].spellID == spell_id ||
			RangedProcs[i].spellID == spell_id ||
			DefensiveProcs[i].spellID == spell_id) {
			return true;
		}
	}

	if (IsOfClientBot()) {
		for (int i = 0; i < MAX_AA_PROCS; i += 4) {
			if (aabonuses.SpellProc[i + 1] == spell_id ||
				aabonuses.RangedProc[i + 1] == spell_id ||
				aabonuses.DefensiveProc[i + 1] == spell_id) {
				return true;
			}
		}
	}
	return false;
}

bool Mob::AddProcToWeapon(uint16 spell_id, bool bPerma, uint16 iChance, uint16 base_spell_id, int level_override, uint32 proc_reuse_time) {
	if(!IsValidSpell(spell_id)) {
		return false;
	}

	// Special case for Vampiric Embrace. If this is a Shadow Knight, the proc is different.
	if (spell_id == SPELL_VAMPIRIC_EMBRACE && GetClass() == Class::ShadowKnight) {
		spell_id = SPELL_VAMPIRIC_EMBRACE_OF_SHADOW;
	}

	int i;
	if (bPerma) {
		for (i = 0; i < m_max_procs; i++) {
			if (!IsValidSpell(PermaProcs[i].spellID)) {
				PermaProcs[i].spellID = spell_id;
				PermaProcs[i].chance = iChance;
				PermaProcs[i].base_spellID = base_spell_id;
				PermaProcs[i].level_override = level_override;
				PermaProcs[i].proc_reuse_time = proc_reuse_time;
				LogSpells("Added permanent proc spell [{}] with chance [{}] to slot [{}]", spell_id, iChance, i);
				return true;
			}
		}
		LogSpells("Too many perma procs for [{}]", GetName());
	} else {
		// If its a poison proc, replace any existing one if present.
		if (base_spell_id == POISON_PROC) {
			for (i = 0; i < m_max_procs; i++) {
				// If we already have a poison proc active replace it and return
				if (SpellProcs[i].base_spellID == POISON_PROC) {
					SpellProcs[i].spellID = spell_id;
					SpellProcs[i].chance = iChance;
					SpellProcs[i].level_override = level_override;
					SpellProcs[i].proc_reuse_time = proc_reuse_time;
					Log(Logs::Detail, Logs::Spells, "Replaced poison-granted proc spell %d with chance %d to slot %d", spell_id, iChance, i);
					return true;
				}
			}
		}

		// If we get here it either wasn't poison (which can only use 1 slot)
		// or it is poison and no poison procs are currently present.
		// Find a slot and use it as normal.

		for (i = 0; i < m_max_procs; i++) {
			if (!IsValidSpell(SpellProcs[i].spellID)) {
				SpellProcs[i].spellID = spell_id;
				SpellProcs[i].chance = iChance;
				SpellProcs[i].base_spellID = base_spell_id;;
				SpellProcs[i].level_override = level_override;
				SpellProcs[i].proc_reuse_time = proc_reuse_time;
				LogSpells("Added [{}]-granted proc spell [{}] with chance [{}] to slot [{}]", (base_spell_id == POISON_PROC) ? "poison" : "spell", spell_id, iChance, i);
				return true;
			}
		}
		LogSpells("Too many procs for [{}]", GetName());
	}
	return false;
}

bool Mob::RemoveProcFromWeapon(uint16 spell_id, bool bAll) {
	// Special case for Vampiric Embrace. If this is a Shadow Knight, the proc is different.
	if (spell_id == SPELL_VAMPIRIC_EMBRACE && GetClass() == Class::ShadowKnight) {
		spell_id = SPELL_VAMPIRIC_EMBRACE_OF_SHADOW;
	}

	for (int i = 0; i < m_max_procs; i++) {
		if (bAll || SpellProcs[i].spellID == spell_id) {
			SpellProcs[i].spellID = SPELL_UNKNOWN;
			SpellProcs[i].chance = 0;
			SpellProcs[i].base_spellID = SPELL_UNKNOWN;
			SpellProcs[i].level_override = -1;
			SpellProcs[i].proc_reuse_time = 0;
			LogSpells("Removed proc [{}] from slot [{}]", spell_id, i);
		}
	}
	return true;
}

bool Mob::AddDefensiveProc(uint16 spell_id, uint16 iChance, uint16 base_spell_id, uint32 proc_reuse_time)
{
	if(!IsValidSpell(spell_id))
		return(false);

	int i;
	for (i = 0; i < m_max_procs; i++) {
		if (!IsValidSpell(DefensiveProcs[i].spellID)) {
			DefensiveProcs[i].spellID = spell_id;
			DefensiveProcs[i].chance = iChance;
			DefensiveProcs[i].base_spellID = base_spell_id;
			DefensiveProcs[i].proc_reuse_time = proc_reuse_time;
			LogSpells("Added spell-granted defensive proc spell [{}] with chance [{}] to slot [{}]", spell_id, iChance, i);
			return true;
		}
	}

	return false;
}

bool Mob::RemoveDefensiveProc(uint16 spell_id, bool bAll)
{
	for (int i = 0; i < m_max_procs; i++) {
		if (bAll || DefensiveProcs[i].spellID == spell_id) {
			DefensiveProcs[i].spellID = SPELL_UNKNOWN;
			DefensiveProcs[i].chance = 0;
			DefensiveProcs[i].base_spellID = SPELL_UNKNOWN;
			DefensiveProcs[i].proc_reuse_time = 0;
			LogSpells("Removed defensive proc [{}] from slot [{}]", spell_id, i);
		}
	}
	return true;
}

bool Mob::AddRangedProc(uint16 spell_id, uint16 iChance, uint16 base_spell_id, uint32 proc_reuse_time)
{
	if(!IsValidSpell(spell_id))
		return(false);

	int i;
	for (i = 0; i < m_max_procs; i++) {
		if (!IsValidSpell(RangedProcs[i].spellID)) {
			RangedProcs[i].spellID = spell_id;
			RangedProcs[i].chance = iChance;
			RangedProcs[i].base_spellID = base_spell_id;
			RangedProcs[i].proc_reuse_time = proc_reuse_time;
			LogSpells("Added spell-granted ranged proc spell [{}] with chance [{}] to slot [{}]", spell_id, iChance, i);
			return true;
		}
	}

	return false;
}

bool Mob::RemoveRangedProc(uint16 spell_id, bool bAll)
{
	for (int i = 0; i < m_max_procs; i++) {
		if (bAll || RangedProcs[i].spellID == spell_id) {
			RangedProcs[i].spellID = SPELL_UNKNOWN;
			RangedProcs[i].chance = 0;
			RangedProcs[i].base_spellID = SPELL_UNKNOWN;
			RangedProcs[i].proc_reuse_time = 0;
			LogSpells("Removed ranged proc [{}] from slot [{}]", spell_id, i);
		}
	}
	return true;
}

// this is checked in a few places to decide wether special bard
// behavior should be used.
bool Mob::UseBardSpellLogic(uint16 spell_id, int slot)
{
	if (!IsValidSpell(spell_id)) {
		spell_id = casting_spell_id;
	}
	if (slot == -1) {
		slot = static_cast<int>(casting_spell_slot);
	}
	// should we treat this as a bard singing?
	return
	(
		IsValidSpell(spell_id) &&
		slot != -1 &&
		GetClass() == Class::Bard &&
		slot <= EQ::spells::SPELL_GEM_COUNT &&
		IsBardSong(spell_id)
	);
}

int Mob::GetCasterLevel(uint16 spell_id) {
	int level = GetLevel();
	if (GetClass() == Class::Bard) {
		// Bards receive effective casting level increases to resists/effect. They don't receive benefit from spells like intellectual superiority, however.
		level += itembonuses.effective_casting_level + aabonuses.effective_casting_level;
	} else {
		level += itembonuses.effective_casting_level + spellbonuses.effective_casting_level + aabonuses.effective_casting_level;
	}
	LogSpells("Determined effective casting level [{}]+[{}]+[{}]=[{}]", GetLevel(), spellbonuses.effective_casting_level, itembonuses.effective_casting_level, level);
	return std::max(1, level);
}

//This member function sets the buff duration on the client
//however it does not work if sent quickly after an action packets, which is what one might perfer to do
//Thus I use this in the buff process to update the correct duration once after casting
//this allows AAs and focus effects that increase buff duration to work correctly, but could probably
//be used for other things as well
void Client::SendBuffDurationPacket(Buffs_Struct &buff, int slot)
{
	EQApplicationPacket* outapp = nullptr;
	outapp = new EQApplicationPacket(OP_Buff, sizeof(SpellBuffPacket_Struct));
	SpellBuffPacket_Struct* sbf = (SpellBuffPacket_Struct*) outapp->pBuffer;

	sbf->entityid = GetID();

	sbf->buff.effect_type = 2;

	sbf->buff.level = buff.casterlevel > 0 ? buff.casterlevel : GetLevel();
	sbf->buff.bard_modifier = buff.instrument_mod;
	sbf->buff.spellid = buff.spellid;
	sbf->buff.duration = buff.ticsremaining;
	if (buff.dot_rune)
		sbf->buff.counters = buff.dot_rune;
	else if (buff.magic_rune)
		sbf->buff.counters = buff.magic_rune;
	else if (buff.melee_rune)
		sbf->buff.counters = buff.melee_rune;
	else if (buff.counters)
		sbf->buff.counters = buff.counters;
	sbf->buff.player_id = buff.casterid;
	sbf->buff.num_hits = buff.hit_number;
	sbf->buff.y = buff.caston_y;
	sbf->buff.x = buff.caston_x;
	sbf->buff.z = buff.caston_z;

	sbf->slotid = slot;
	sbf->bufffade = 0;
	FastQueuePacket(&outapp);
}

void Client::SendBuffNumHitPacket(Buffs_Struct &buff, int slot)
{
	// UF+ use this packet
	if (ClientVersion() < EQ::versions::ClientVersion::UF)
		return;
	EQApplicationPacket *outapp = nullptr;
	outapp = new EQApplicationPacket(OP_BuffCreate, sizeof(BuffIcon_Struct) + sizeof(BuffIconEntry_Struct));
	BuffIcon_Struct *bi = (BuffIcon_Struct *)outapp->pBuffer;
	bi->entity_id = GetID();
	bi->count = 1;
	bi->all_buffs = 0;
	bi->tic_timer = tic_timer.GetRemainingTime();

	bi->entries[0].buff_slot = slot;
	bi->entries[0].spell_id = buff.spellid;
	bi->entries[0].tics_remaining = buff.ticsremaining;
	bi->entries[0].num_hits = buff.hit_number;
	strn0cpy(bi->entries[0].caster, buff.caster_name, 64);
	bi->name_lengths = strlen(bi->entries[0].caster);
	FastQueuePacket(&outapp);
}

void Mob::SendPetBuffsToClient()
{
	// Don't really need this check, as it should be checked before this method is called, but it doesn't hurt
	// too much to check again.
	if(!(GetOwner() && GetOwner()->IsClient()))
		return;

	int PetBuffCount = 0;

	auto outapp = new EQApplicationPacket(OP_PetBuffWindow, sizeof(PetBuff_Struct));
	PetBuff_Struct* pbs=(PetBuff_Struct*)outapp->pBuffer;
	memset(outapp->pBuffer,0,outapp->size);
	pbs->petid=GetID();

	int MaxSlots = GetMaxTotalSlots();

	if(MaxSlots > PET_BUFF_COUNT)
		MaxSlots = PET_BUFF_COUNT;

	for(int buffslot = 0; buffslot < MaxSlots; buffslot++)
	{
		if (IsValidSpell(buffs[buffslot].spellid)) {
			pbs->spellid[buffslot] = buffs[buffslot].spellid;
			pbs->ticsremaining[buffslot] = buffs[buffslot].ticsremaining;
			PetBuffCount++;
		}
	}

	pbs->buffcount=PetBuffCount;
	GetOwner()->CastToClient()->QueuePacket(outapp);
	safe_delete(outapp);
}

void Mob::SendBuffsToClient(Client *c)
{
	if(!c)
		return;

	if (c->ClientVersionBit() & EQ::versions::maskSoDAndLater)
	{
		EQApplicationPacket *outapp = MakeBuffsPacket();
		c->FastQueuePacket(&outapp);
	}
}

EQApplicationPacket *Mob::MakeBuffsPacket(bool for_target, bool clear_buffs)
{
	uint32 count = 0;
	uint32 buff_count;

	// for self we want all buffs, for target, we want to skip song window buffs
	// since NPCs and pets don't have a song window, we still see it for them :P
	if (for_target) {
		buff_count = (clear_buffs) ? 0 : GetMaxBuffSlots();
	}
	else {
		buff_count = GetMaxTotalSlots();
	}

	for(int i = 0; i < buff_count; ++i) {
		if (IsValidSpell(buffs[i].spellid)) {
			++count;
		}
	}

	EQApplicationPacket* outapp = nullptr;

	//Create it for a targeting window, else create it for a create buff packet.
	if(for_target) {
		outapp = new EQApplicationPacket(OP_TargetBuffs, sizeof(BuffIcon_Struct) + sizeof(BuffIconEntry_Struct) * count);
	}
	else {
		outapp = new EQApplicationPacket(OP_BuffCreate, sizeof(BuffIcon_Struct) + sizeof(BuffIconEntry_Struct) * count);
	}
	BuffIcon_Struct *buff = (BuffIcon_Struct*)outapp->pBuffer;
	buff->entity_id = GetID();
	buff->count = count;
	buff->all_buffs = 1;
	buff->tic_timer = tic_timer.GetRemainingTime();
	// there are more types, the client doesn't seem to really care though. The others are also currently hard to fill in here ...
	// (see comment in common/eq_packet_structs.h)
	if (for_target)
		buff->type = IsClient() ? 5 : 7;
	else
		buff->type = 0;

	buff->name_lengths = 0; // hacky shit
	uint32 index = 0;
	for(int i = 0; i < buff_count; ++i) {
		if (IsValidSpell(buffs[i].spellid)) {
			buff->entries[index].buff_slot = i;
			buff->entries[index].spell_id = buffs[i].spellid;
			buff->entries[index].tics_remaining = buffs[i].ticsremaining;
			buff->entries[index].num_hits = buffs[i].hit_number;
			strn0cpy(buff->entries[index].caster, buffs[i].caster_name, 64);
			buff->name_lengths += strlen(buff->entries[index].caster);
			++index;
		}
	}

	return outapp;
}

void Mob::BuffModifyDurationBySpellID(uint16 spell_id, int32 newDuration)
{
	int buff_count = GetMaxTotalSlots();
	for(int i = 0; i < buff_count; ++i)
	{
		if (buffs[i].spellid == spell_id)
		{
			buffs[i].ticsremaining = newDuration;
			if(IsClient())
			{
				CastToClient()->SendBuffDurationPacket(buffs[i], i);
			}
		}
	}
}

int Client::GetCurrentBuffSlots() const
{
	int numbuffs = 15;
	// client does check spells and items
	numbuffs += aabonuses.BuffSlotIncrease + spellbonuses.BuffSlotIncrease + itembonuses.BuffSlotIncrease;
	if (GetLevel() > 70)
		numbuffs++;
	if (GetLevel() > 74)
		numbuffs++;
	return EQ::ClampUpper(numbuffs, EQ::spells::StaticLookup(m_ClientVersion)->LongBuffs);
}

int Client::GetCurrentSongSlots() const
{
	return EQ::spells::StaticLookup(m_ClientVersion)->ShortBuffs; // AAs dont affect this
}

void Client::InitializeBuffSlots()
{
	int max_slots = GetMaxTotalSlots();
	buffs = new Buffs_Struct[max_slots];
	for(int x = 0; x < max_slots; ++x)
	{
		buffs[x].spellid = SPELL_UNKNOWN;
		buffs[x].UpdateClient = false;
	}
}

void Client::UninitializeBuffSlots()
{
	safe_delete_array(buffs);
}

void NPC::InitializeBuffSlots()
{
	int max_slots = GetMaxTotalSlots();
	buffs = new Buffs_Struct[max_slots];
	for (int x = 0; x < max_slots; ++x) {
		buffs[x].spellid      = SPELL_UNKNOWN;
		buffs[x].UpdateClient = false;
	}
}

void NPC::UninitializeBuffSlots()
{
	safe_delete_array(buffs);
}

void Client::SendSpellAnim(uint16 target_id, uint16 spell_id)
{
	if (!target_id || !IsValidSpell(spell_id)) {
		return;
	}

	EQApplicationPacket app(OP_Action, sizeof(Action_Struct));
	auto* a = (Action_Struct*) app.pBuffer;

	a->target      = target_id;
	a->source      = GetID();
	a->type        = 231;
	a->spell       = spell_id;
	a->hit_heading = GetHeading();

	app.priority = 1;
	entity_list.QueueCloseClients(this, &app, false, RuleI(Range, SpellParticles));
}

void Client::SendItemRecastTimer(int32 recast_type, uint32 recast_delay, bool in_ignore_casting_requirement)
{
	if (recast_type == RECAST_TYPE_UNLINKED_ITEM) {
		return;
	}

	if (!recast_delay) {
		recast_delay = GetPTimers().GetRemainingTime(pTimerItemStart + recast_type);
	}

	if (recast_delay) {
		auto outapp = new EQApplicationPacket(OP_ItemRecastDelay, sizeof(ItemRecastDelay_Struct));

		auto ird = (ItemRecastDelay_Struct *) outapp->pBuffer;

		ird->recast_delay               = recast_delay;
		ird->recast_type                = static_cast<uint32>(recast_type);
		ird->ignore_casting_requirement = in_ignore_casting_requirement; //True allows reset of item cast timers

		QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void Client::SetItemRecastTimer(int32 spell_id, uint32 inventory_slot)
{
	auto item = CastToClient()->GetInv().GetItem(inventory_slot);

	if (!item) {
		return;
	}

	uint32 recast_delay = 0;
	int    recast_type  = 0;
	uint32 item_id      = item->GetItem()->ID;
	bool   from_augment = false;

	if (item->GetItem()->RecastDelay > 0) { // Check item
		recast_type = item->GetItem()->RecastType;
		recast_delay = item->GetItem()->RecastDelay;
	} else { // Check augments
		for (int r = EQ::invaug::SOCKET_BEGIN; r <= EQ::invaug::SOCKET_END; r++) {
			const auto aug = item->GetAugment(r);

			if (!aug) {
				continue;
			}

			const auto aug_data = aug->GetItem();
			if (!aug_data) {
				continue;
			}

			if (aug_data->Click.Effect == spell_id) {
				recast_delay = aug->GetItem()->RecastDelay;
				recast_type  = aug->GetItem()->RecastType;
				from_augment = true;
				item_id      = aug->GetItem()->ID;
				break;
			}
		}
	}

	//must use SPA 415 with focus (SPA 310) to reduce item recast
	int reduction = GetFocusEffect(focusReduceRecastTime, spell_id);
	if (reduction) {
		recast_delay -= reduction;
	}

	recast_delay = std::max(recast_delay, static_cast<uint32>(0));

	if (recast_delay > 0) {
		if (recast_type != RECAST_TYPE_UNLINKED_ITEM) {
			GetPTimers().Start((pTimerItemStart + recast_type), recast_delay);
			database.UpdateItemRecast(
				CharacterID(),
				recast_type,
				GetPTimers().Get(pTimerItemStart + recast_type)->GetReadyTimestamp()
			);
		} else if (recast_type == RECAST_TYPE_UNLINKED_ITEM) {
			GetPTimers().Start((pTimerNegativeItemReuse * item_id), recast_delay);
			database.UpdateItemRecast(
				CharacterID(),
				item_id,
				GetPTimers().Get(pTimerNegativeItemReuse * item_id)->GetReadyTimestamp()
			);
		}

		if (!from_augment) {
			SendItemRecastTimer(recast_type, recast_delay);
		}
	}
}

void Client::DeleteItemRecastTimer(uint32 item_id)
{
	const auto item = database.GetItem(item_id);

	if (!item) {
		return;
	}

	const auto recast_type = (
		item->RecastType != RECAST_TYPE_UNLINKED_ITEM ?
		item->RecastType :
		item_id
	);

	const int  timer_id    = (
		item->RecastType != RECAST_TYPE_UNLINKED_ITEM ?
		(pTimerItemStart + recast_type) :
		(pTimerNegativeItemReuse * item_id)
	);

	database.DeleteItemRecast(CharacterID(), recast_type);
	GetPTimers().Clear(&database, timer_id);

	if (recast_type != RECAST_TYPE_UNLINKED_ITEM) {
		SendItemRecastTimer(recast_type, 1, true);
	}
}

bool Client::HasItemRecastTimer(int32 spell_id, uint32 inventory_slot)
{
	auto item = CastToClient()->GetInv().GetItem(inventory_slot);

	if (!item || !item->GetItem()) {
		return false;
	}

	uint32 recast_delay = 0;
	int    recast_type  = 0;
	uint32 item_id      = 0;
	bool   from_augment = false;

	if (item->GetItem()->RecastDelay > 0) { // Check item
		recast_type  = item->GetItem()->RecastType;
		recast_delay = item->GetItem()->RecastDelay;
		item_id      = item->GetItem()->ID;
	} else { // Check augments
		for (int r = EQ::invaug::SOCKET_BEGIN; r <= EQ::invaug::SOCKET_END; r++) {
			const auto aug = item->GetAugment(r);

			if (!aug) {
				continue;
			}

			const auto aug_data = aug->GetItem();
			if (!aug_data) {
				continue;
			}

			if (aug_data->Click.Effect == spell_id) {
				if (aug->GetItem() && aug->GetItem()->RecastDelay > 0) {
					recast_delay = aug->GetItem()->RecastDelay;
					recast_type  = aug->GetItem()->RecastType;
					item_id      = aug->GetItem()->ID;
				}

				break;
			}
		}
	}

	if (!recast_delay) {
		return false;
	}

	if (
		recast_type != RECAST_TYPE_UNLINKED_ITEM &&
		!CastToClient()->GetPTimers().Expired(&database, (pTimerItemStart + recast_type), false)
	) {
		return true;
	} else if (
		recast_type == RECAST_TYPE_UNLINKED_ITEM &&
		!CastToClient()->GetPTimers().Expired(&database, (pTimerNegativeItemReuse * item_id), false)
	) {
		return true;
	}

	return false;
}

void Mob::CalcDestFromHeading(float heading, float distance, float MaxZDiff, float StartX, float StartY, float &dX, float &dY, float &dZ)
{
	if (!distance) { return; }
	if (!MaxZDiff) { MaxZDiff = 5; }

	float ReverseHeading = 512 - heading;
	float ConvertAngle = ReverseHeading * 360.0f / 512.0f;
	if (ConvertAngle <= 270)
		ConvertAngle = ConvertAngle + 90;
	else
		ConvertAngle = ConvertAngle - 270;

	float Radian = ConvertAngle * (3.1415927f / 180.0f);

	float CircleX = distance * std::cos(Radian);
	float CircleY = distance * std::sin(Radian);
	dX = CircleX + StartX;
	dY = CircleY + StartY;
	dZ = FindGroundZ(dX, dY, MaxZDiff);
}

void Mob::BeamDirectional(uint16 spell_id, int16 resist_adjust)
{
	int maxtarget_count = 0;
	bool beneficial_targets = false;

	if (IsBeneficialSpell(spell_id) && IsClient())
		beneficial_targets = true;

	std::list<Mob *> targets_in_range;

	entity_list.GetTargetsForConeArea(this, spells[spell_id].min_range, spells[spell_id].range,
					  spells[spell_id].range / 2, spells[spell_id].pcnpc_only_flag, targets_in_range);
	auto iter = targets_in_range.begin();

	float dX = 0;
	float dY = 0;
	float dZ = 0;

	CalcDestFromHeading(GetHeading(), spells[spell_id].range, 5, GetX(), GetY(), dX, dY, dZ);
	dZ = GetZ();

	// FIND SLOPE: Put it into the form y = mx + b
	float m = (dY - GetY()) / (dX - GetX());
	float b = (GetY() * dX - dY * GetX()) / (dX - GetX());

	while (iter != targets_in_range.end()) {
		if (!(*iter) || (beneficial_targets && ((*iter)->IsNPC() && !(*iter)->IsPetOwnerOfClientBot())) ||
			(*iter)->BehindMob(this, (*iter)->GetX(), (*iter)->GetY())) {
			++iter;
			continue;
		}

		if (IsNPC() && (*iter)->IsNPC()) {
			auto fac = (*iter)->GetReverseFactionCon(this);
			if (beneficial_targets) {
				// only affect mobs we would assist.
				if (!(fac <= FACTION_AMIABLY)) {
					++iter;
					continue;
				}
			} else {
				// affect mobs that are on our hate list, or which have bad faction with us
				if (!(CheckAggro(*iter) || fac == FACTION_THREATENINGLY || fac == FACTION_SCOWLS)) {
					++iter;
					continue;
				}
			}
		}

		if (!beneficial_targets) {
			if (!IsAttackAllowed((*iter), true)) {
				++iter;
				continue;
			}
		}
		else {
			if (IsAttackAllowed((*iter), true)) {
				++iter;
				continue;
			}
			if (CheckAggro((*iter))) {
				++iter;
				continue;
			}
		}

		//# shortest distance from line to target point
		float d = std::abs((*iter)->GetY() - m * (*iter)->GetX() - b) / sqrt(m * m + 1);

		if (d <= spells[spell_id].aoe_range) {
			if (CheckLosFN((*iter)) || spells[spell_id].npc_no_los) {
				(*iter)->CalcSpellPowerDistanceMod(spell_id, 0, this);
				SpellOnTarget(spell_id, (*iter), 0, true, resist_adjust);
				maxtarget_count++;
			}

			// not sure if we need this check, but probably do, need to check if it should be default limited or not
			if (spells[spell_id].aoe_max_targets && maxtarget_count >= spells[spell_id].aoe_max_targets)
				return;
		}
		++iter;
	}
}

void Mob::ConeDirectional(uint16 spell_id, int16 resist_adjust)
{
	int maxtarget_count = 0;
	bool beneficial_targets = false;

	if (IsBeneficialSpell(spell_id) && IsClient())
		beneficial_targets = true;

	float heading = GetHeading() * 360.0f / 512.0f; // convert to degrees

	float angle_start = spells[spell_id].directional_start + heading;
	float angle_end = spells[spell_id].directional_end + heading;

	while (angle_start > 360.0f)
		angle_start -= 360.0f;

	while (angle_end > 360.0f)
		angle_end -= 360.0f;

	std::list<Mob *> targets_in_range;

	entity_list.GetTargetsForConeArea(this, spells[spell_id].min_range, spells[spell_id].aoe_range,
					  spells[spell_id].aoe_range / 2, spells[spell_id].pcnpc_only_flag, targets_in_range);
	auto iter = targets_in_range.begin();

	while (iter != targets_in_range.end()) {
		if (!(*iter) || (beneficial_targets && ((*iter)->IsNPC() && !(*iter)->IsPetOwnerOfClientBot()))) {
			++iter;
			continue;
		}

		float heading_to_target =
			(CalculateHeadingToTarget((*iter)->GetX(), (*iter)->GetY()) * 360.0f / 512.0f);

		while (heading_to_target < 0.0f)
			heading_to_target += 360.0f;

		while (heading_to_target > 360.0f)
			heading_to_target -= 360.0f;

		if (IsNPC() && (*iter)->IsNPC()) {
			auto fac = (*iter)->GetReverseFactionCon(this);
			if (beneficial_targets) {
				// only affect mobs we would assist.
				if (!(fac <= FACTION_AMIABLY)) {
					++iter;
					continue;
				}
			} else {
				// affect mobs that are on our hate list, or which have bad faction with us
				if (!(CheckAggro(*iter) || fac == FACTION_THREATENINGLY || fac == FACTION_SCOWLS)) {
					++iter;
					continue;
				}
			}
		}

		if (!beneficial_targets) {
			if (!IsAttackAllowed((*iter), true)) {
				++iter;
				continue;
			}
		}
		else {
			if (IsAttackAllowed((*iter), true)) {
				++iter;
				continue;
			}
			if (CheckAggro((*iter))) {
				++iter;
				continue;
			}
		}

		if (angle_start > angle_end) {
			if ((heading_to_target >= angle_start && heading_to_target <= 360.0f) ||
				(heading_to_target >= 0.0f && heading_to_target <= angle_end)) {
				if (CheckLosFN((*iter)) || spells[spell_id].npc_no_los) {
					(*iter)->CalcSpellPowerDistanceMod(spell_id, 0, this);
					SpellOnTarget(spell_id, (*iter), 0, true, resist_adjust);
					maxtarget_count++;
				}
			}
		} else {
			if (heading_to_target >= angle_start && heading_to_target <= angle_end) {
				if (CheckLosFN((*iter)) || spells[spell_id].npc_no_los) {
					(*iter)->CalcSpellPowerDistanceMod(spell_id, 0, this);
					SpellOnTarget(spell_id, (*iter), 0, true, resist_adjust);
					maxtarget_count++;
				}
			}
		}

		// my SHM breath could hit all 5 dummies I could summon in arena
		if (spells[spell_id].aoe_max_targets && maxtarget_count >= spells[spell_id].aoe_max_targets)
			return;

		++iter;
	}
}

// duration in seconds
void Client::SetLinkedSpellReuseTimer(uint32 timer_id, uint32 duration)
{
	if (timer_id > 19)
		return;
	LogSpells("Setting Linked Spell Reuse [{}] for [{}]", timer_id, duration);
	GetPTimers().Start(pTimerLinkedSpellReuseStart + timer_id, duration);
	auto outapp = new EQApplicationPacket(OP_LinkedReuse, sizeof(LinkedSpellReuseTimer_Struct));
	auto lr = (LinkedSpellReuseTimer_Struct *)outapp->pBuffer;
	lr->timer_id = timer_id;
	lr->start_time = Timer::GetCurrentTime() / 1000;
	lr->end_time = lr->start_time + duration;
	FastQueuePacket(&outapp);
}

bool Client::IsLinkedSpellReuseTimerReady(uint32 timer_id)
{
	if (timer_id > 19)
		return true;
	return GetPTimers().Expired(&database, pTimerLinkedSpellReuseStart + timer_id, false);
}

int Client::GetNextAvailableDisciplineSlot(int starting_slot) {
	for (uint32 index = starting_slot; index < MAX_PP_DISCIPLINES; index++) {
		if (!IsValidSpell(GetPP().disciplines.values[index])) {
			return index;
		}
	}

	return -1; // Return -1 if No Slots open
}

void Client::ResetCastbarCooldownBySlot(int slot) {
	if (slot < 0) {
		for (unsigned int i = 0; i < EQ::spells::SPELL_GEM_COUNT; ++i) {
			if(IsValidSpell(m_pp.mem_spells[i])) {
				m_pp.spellSlotRefresh[i] = 1;
				GetPTimers().Clear(&database, (pTimerSpellStart + m_pp.mem_spells[i]));
				if (!IsLinkedSpellReuseTimerReady(spells[m_pp.mem_spells[i]].timer_id)) {
					GetPTimers().Clear(&database, (pTimerLinkedSpellReuseStart + spells[m_pp.mem_spells[i]].timer_id));
				}
				if (spells[m_pp.mem_spells[i]].timer_id > 0 && spells[m_pp.mem_spells[i]].timer_id < MAX_DISCIPLINE_TIMERS) {
					SetLinkedSpellReuseTimer(spells[m_pp.mem_spells[i]].timer_id, 0);
				}
				SendSpellBarEnable(m_pp.mem_spells[i]);
			}
		}
	} else if (slot < EQ::spells::SPELL_GEM_COUNT) {
		if(IsValidSpell(m_pp.mem_spells[slot])) {
			m_pp.spellSlotRefresh[slot] = 1;
			GetPTimers().Clear(&database, (pTimerSpellStart + m_pp.mem_spells[slot]));
			if (!IsLinkedSpellReuseTimerReady(spells[m_pp.mem_spells[slot]].timer_id)) {
				GetPTimers().Clear(&database, (pTimerLinkedSpellReuseStart + spells[m_pp.mem_spells[slot]].timer_id));

			}
			if (spells[m_pp.mem_spells[slot]].timer_id > 0 && spells[m_pp.mem_spells[slot]].timer_id < MAX_DISCIPLINE_TIMERS) {
				SetLinkedSpellReuseTimer(spells[m_pp.mem_spells[slot]].timer_id, 0);
			}
			SendSpellBarEnable(m_pp.mem_spells[slot]);
		}
	}
}

void Client::ResetAllCastbarCooldowns() {
	for (unsigned int i = 0; i < EQ::spells::SPELL_GEM_COUNT; ++i) {
		if(IsValidSpell(m_pp.mem_spells[i])) {
			m_pp.spellSlotRefresh[i] = 1;
			GetPTimers().Clear(&database, (pTimerSpellStart + m_pp.mem_spells[i]));
			if (!IsLinkedSpellReuseTimerReady(spells[m_pp.mem_spells[i]].timer_id)) {
				GetPTimers().Clear(&database, (pTimerLinkedSpellReuseStart + spells[m_pp.mem_spells[i]].timer_id));
			}
			if (spells[m_pp.mem_spells[i]].timer_id > 0 && spells[m_pp.mem_spells[i]].timer_id < MAX_DISCIPLINE_TIMERS) {
				SetLinkedSpellReuseTimer(spells[m_pp.mem_spells[i]].timer_id, 0);
			}
			SendSpellBarEnable(m_pp.mem_spells[i]);
		}
	}
}

void Client::ResetCastbarCooldownBySpellID(uint32 spell_id) {
	for (unsigned int i = 0; i < EQ::spells::SPELL_GEM_COUNT; ++i) {
		if(IsValidSpell(m_pp.mem_spells[i]) && m_pp.mem_spells[i] == spell_id) {
			m_pp.spellSlotRefresh[i] = 1;
			GetPTimers().Clear(&database, (pTimerSpellStart + m_pp.mem_spells[i]));
			if (!IsLinkedSpellReuseTimerReady(spells[m_pp.mem_spells[i]].timer_id)) {
				GetPTimers().Clear(&database, (pTimerLinkedSpellReuseStart + spells[m_pp.mem_spells[i]].timer_id));
			}
			if (spells[m_pp.mem_spells[i]].timer_id > 0 && spells[m_pp.mem_spells[i]].timer_id < MAX_DISCIPLINE_TIMERS) {
				SetLinkedSpellReuseTimer(spells[m_pp.mem_spells[i]].timer_id, 0);
			}
			SendSpellBarEnable(m_pp.mem_spells[i]);
			break;
		}
	}
}

bool Mob::IsActiveBardSong(int32 spell_id) {

	if (spell_id == bardsong) {
		return true;
	}
	return false;
}

void Mob::DoBardCastingFromItemClick(bool is_casting_bard_song, uint32 cast_time, int32 spell_id, uint16 target_id, EQ::spells::CastingSlot slot, uint32 item_slot, uint32 recast_type, uint32 recast_delay)
{
	/*
		Known bug: When a bard uses an augment with a clicky that has a cast time, the cast won't display. This issue only affects bards.
	*/
	if (is_casting_bard_song) {
		//For spells with cast times. Cancel song cast, stop pusling and start item cast.
		if (cast_time != 0) {
			EQApplicationPacket *outapp = nullptr;
			outapp = new EQApplicationPacket(OP_InterruptCast, sizeof(InterruptCast_Struct));
			InterruptCast_Struct* ic = (InterruptCast_Struct*)outapp->pBuffer;
			ic->messageid = SONG_ENDS;
			ic->spawnid = GetID();
			outapp->priority = 5;
			CastToClient()->QueuePacket(outapp);
			safe_delete(outapp);

			ZeroCastingVars();
			ZeroBardPulseVars();
		}
	}

	if (cast_time != 0) {
		if (!CastSpell(spell_id, target_id, CastingSlot::Item, cast_time, 0, 0, item_slot)) {
			if (IsBot()) {
				GetOwner()->Message(Chat::Red, "%s says, 'Casting failed for %s. This could be due to zone restrictions, target restrictions or other limiting factors.", GetCleanName(), CastToBot()->GetBotItem(item_slot)->GetItem()->Name);
			}
		}
	}
	//Instant cast items do not stop bard songs or interrupt casting.
	else if (CheckItemRaceClassDietyRestrictionsOnCast(item_slot) && DoCastingChecksOnCaster(spell_id, CastingSlot::Item)) {
		int16 DeleteChargeFromSlot = GetItemSlotToConsumeCharge(spell_id, item_slot);
		if (SpellFinished(spell_id, entity_list.GetMob(target_id), CastingSlot::Item, 0, item_slot)) {
			if (IsClient() && DeleteChargeFromSlot >= 0) {
				CastToClient()->DeleteItemInInventory(DeleteChargeFromSlot, 1, true);
			}
			else if (IsBot() && DeleteChargeFromSlot >= 0) {
				EQ::ItemInstance* inst = CastToBot()->GetBotItem(DeleteChargeFromSlot);
				if (inst) {
					inst->SetCharges((inst->GetCharges() - 1));
					if (!database.botdb.SaveItemBySlot(CastToBot(), DeleteChargeFromSlot, inst)) {
						GetOwner()->Message(Chat::Red, "%s says, 'Failed to save item [%i] slot [%i] for [%s].", inst->GetID(), DeleteChargeFromSlot, GetCleanName());
						return;
					}
				}
				else {
					GetOwner()->Message(Chat::Red, "%s says, 'Failed to update item charges.", GetCleanName());
					LogError("Failed to update item charges for {}.", GetCleanName());
				}
			}
		}
		else {
			if (IsBot()) {
				GetOwner()->Message(Chat::Red, "%s says, 'Casting failed for %s. This could be due to zone restrictions, target restrictions or other limiting factors.", GetCleanName(), CastToBot()->GetBotItem(item_slot)->GetItem()->Name);
			}
		}
	}
}

int16 Mob::GetItemSlotToConsumeCharge(int32 spell_id, uint32 inventory_slot)
{
	int16 DeleteChargeFromSlot = -1;

	if (!IsOfClientBot() || inventory_slot == 0xFFFFFFFF) {
		return DeleteChargeFromSlot;
	}

	EQ::ItemInstance *item = nullptr;
	if (IsClient()) {
		item = CastToClient()->GetInv().GetItem(inventory_slot);
	}
	else if (IsBot()) {
		item = CastToBot()->GetBotItem(inventory_slot);
	}

	bool fromaug = false;
	EQ::ItemData* augitem = nullptr;

	while (true) {
		if (item == nullptr)
			break;

		for (int r = EQ::invaug::SOCKET_BEGIN; r <= EQ::invaug::SOCKET_END; r++) {
			const EQ::ItemInstance* aug_i = item->GetAugment(r);

			if (!aug_i) {
				continue;
			}
			const EQ::ItemData* aug = aug_i->GetItem();
			if (!aug) {
				continue;
			}
			if (aug->Click.Effect == spell_id){
				fromaug = true;
				break;
			}
		}

		break;
	}

	if (item && item->IsClassCommon() && (item->GetItem()->Click.Effect == spell_id) && item->GetCharges() || fromaug){
		int16 charges = item->GetItem()->MaxCharges;

		if (fromaug) { charges = -1; } //Don't destroy the parent item

		if (charges > -1) {	// charged item, expend a charge
			LogSpells("Spell [{}]: Consuming a charge from item [{}] ([{}]) which had [{}]/[{}] charges", spell_id, item->GetItem()->Name, item->GetItem()->ID, item->GetCharges(), item->GetItem()->MaxCharges);
			DeleteChargeFromSlot = inventory_slot;
		}
		else {
			LogSpells("Spell [{}]: Cast from unlimited charge item [{}] ([{}]) ([{}] charges)", spell_id, item->GetItem()->Name, item->GetItem()->ID, item->GetItem()->MaxCharges);
		}
	}
	else{
		LogSpells("Item used to cast spell [{}] was missing from inventory slot [{}] after casting!", spell_id, inventory_slot);
		if (IsClient()) {
			Message(Chat::Red, "Casting Error: Active casting item not found in inventory slot %i", inventory_slot);
		}
		else if (IsBot()) {
			CastToBot()->GetOwner()->Message(Chat::Red, "Casting Error: Active casting item not found in inventory slot %i for %s", inventory_slot, GetCleanName());
		}
		InterruptSpell();
		return DeleteChargeFromSlot;
	}
	return DeleteChargeFromSlot;
}

bool Mob::CheckItemRaceClassDietyRestrictionsOnCast(uint32 inventory_slot) {

	if (inventory_slot == 0xFFFFFFFF) {
		return false;
	}

	//Added to prevent MQ2 exploitation of equipping normally-unequippable/clickable items with effects and clicking them for benefits.
	EQ::ItemInstance *itm = CastToClient()->GetInv().GetItem(inventory_slot);
	int bitmask = 1;
	bitmask = bitmask << (CastToClient()->GetClass() - 1);
	if (itm && itm->GetItem()->Classes != 65535) {
		if ((itm->GetItem()->Click.Type == EQ::item::ItemEffectEquipClick) && !(itm->GetItem()->Classes & bitmask)) {
			if (CastToClient()->ClientVersion() < EQ::versions::ClientVersion::SoF) {
				std::string message = fmt::format(
					"Attempted to click an equip-only effect on item_name [{}] item_id [{}] which they shouldn't be able to equip!",
					itm->GetItem()->Name,
					itm->GetItem()->ID
				);

				RecordPlayerEventLogWithClient(CastToClient(), PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});
			}
			else {
				MessageString(Chat::Red, MUST_EQUIP_ITEM);
			}
			return(false);
		}
		if ((itm->GetItem()->Click.Type == EQ::item::ItemEffectClick2) && !(itm->GetItem()->Classes & bitmask)) {
			if (CastToClient()->ClientVersion() < EQ::versions::ClientVersion::SoF) {
				std::string message = fmt::format(
					"Attempted to click a race/class restricted effect on item_name [{}] item_id [{}] which they shouldn't be able to click!",
					itm->GetItem()->Name,
					itm->GetItem()->ID
				);

				RecordPlayerEventLogWithClient(CastToClient(), PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});
			}
			else {
				if (CastToClient()->ClientVersion() >= EQ::versions::ClientVersion::RoF)
				{
					// Line 181 in eqstr_us.txt was changed in RoF+
					Message(Chat::Yellow, "Your race, class, or deity cannot use this item.");
				}
				else
				{
					MessageString(Chat::Red, CANNOT_USE_ITEM);
				}
			}
			return(false);
		}
	}
	if (itm && (itm->GetItem()->Click.Type == EQ::item::ItemEffectEquipClick) && inventory_slot > EQ::invslot::EQUIPMENT_END) {
		if (CastToClient()->ClientVersion() < EQ::versions::ClientVersion::SoF) {
			std::string message = fmt::format(
				"Attempted to click an equip-only effect on item_name [{}] item_id [{}] without equipping it!",
				itm->GetItem()->Name,
				itm->GetItem()->ID
			);

			RecordPlayerEventLogWithClient(CastToClient(), PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});
		}
		else {
			MessageString(Chat::Red, MUST_EQUIP_ITEM);
		}
		return(false);
	}

	return true;
}

bool Mob::IsFromTriggeredSpell(CastingSlot slot, uint32 item_slot) {
	//spells triggered using spells finished use item slot, but there is no item set.
	if ((slot == CastingSlot::Item) && (item_slot == 0xFFFFFFFF)) {
		return true;
	}
	return false;
}

void Mob::SetHP(int64 hp)
{
	if (hp >= max_hp) {
		current_hp = max_hp;
		return;
	}

	if (m_combat_record.InCombat()) {
		m_combat_record.ProcessHPEvent(hp, current_hp);
	}

	current_hp = hp;
}

void Mob::DrawDebugCoordinateNode(std::string node_name, const glm::vec4 vec)
{
	NPC *node = nullptr;

	for (const auto &n: entity_list.GetNPCList()) {
		if (n.second->GetEntityVariable("node_parent_id") == std::to_string(GetID())) {
			node = n.second;
			break;
		}
	}
	if (!node) {
		node = NPC::SpawnNodeNPC(node_name, "", GetPosition());
		node->SetEntityVariable("node_parent_id", std::to_string(GetID()));
	}
}

const CombatRecord &Mob::GetCombatRecord() const
{
	return m_combat_record;
}

bool Mob::CheckWaterLoS(Mob* m)
{
	if (
		!RuleB(Spells, WaterMatchRequiredForLoS) ||
		!zone->watermap
	) {
		return true;
	}

	return (
		zone->watermap->InLiquid(GetPosition()) ==
		zone->watermap->InLiquid(m->GetPosition())
	);
}
