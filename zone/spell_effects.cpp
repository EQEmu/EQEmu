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

#include "common/bodytypes.h"
#include "common/classes.h"
#include "common/data_verification.h"
#include "common/eqemu_logsys.h"
#include "common/item_instance.h"
#include "common/misc_functions.h"
#include "common/rulesys.h"
#include "common/spdat.h"
#include "zone/bot.h"
#include "zone/lua_parser.h"
#include "zone/quest_parser_collection.h"
#include "zone/string_ids.h"
#include "zone/worldserver.h"

#include <cmath>


extern Zone* zone;
extern volatile bool is_zone_loaded;
extern WorldServer worldserver;


// the spell can still fail here, if the buff can't stack
// in this case false will be returned, true otherwise
bool Mob::SpellEffect(Mob* caster, uint16 spell_id, float partial, int level_override, int reflect_effectiveness, int32 duration_override, bool disable_buff_overwrite)
{
	int caster_level, buffslot, effect, effect_value, i;
	EQ::ItemInstance *SummonedItem=nullptr;
#ifdef SPELL_EFFECT_SPAM
#define _EDLEN	200
	char effect_desc[_EDLEN];
#endif

	if(!IsValidSpell(spell_id))
		return false;

	const SPDat_Spell_Struct &spell = spells[spell_id];

	if (spell.disallow_sit && IsBuffSpell(spell_id) && IsClient() && (CastToClient()->IsSitting() || CastToClient()->GetHorseId() != 0))
		return false;

	bool CanMemoryBlurFromMez = true;
	if (IsMezzed()) { //Check for special memory blur behavior when on mez, this needs to be before buff override.
		CanMemoryBlurFromMez = false;
	}

	bool c_override = false;
	if (caster && caster->IsClient() && GetCastedSpellInvSlot() > 0) {
		const EQ::ItemInstance *inst = caster->CastToClient()->GetInv().GetItem(GetCastedSpellInvSlot());
		if (inst) {
			if (inst->GetItem()->Click.Level > 0) {
				caster_level = inst->GetItem()->Click.Level;
				c_override = true;
			} else {
				caster_level = caster ? caster->GetCasterLevel(spell_id) : GetCasterLevel(spell_id);
			}
		} else if (level_override > 0) {
			caster_level = level_override;
			c_override = true;
		} else {
			caster_level = caster ? caster->GetCasterLevel(spell_id) : GetCasterLevel(spell_id);
		}
	} else if (level_override > 0) {
		caster_level = level_override;
		c_override = true;
	} else {
		caster_level = caster ? caster->GetCasterLevel(spell_id) : GetCasterLevel(spell_id);
	}

	if(c_override)
	{
		int durat = CalcBuffDuration(caster, this, spell_id, caster_level);
		if(durat) // negatives are perma buffs
		{
			buffslot = AddBuff(caster, spell_id, durat, caster_level);
			if(buffslot == -1)	// stacking failure
				return false;
		}
		else
		{
			buffslot = -2;
		}
	}
	else
	{
		if(IsBuffSpell(spell_id)){
			if(IsEffectInSpell(spell_id, SpellEffect::BindSight))
			{
				if(caster)
				{
					buffslot = caster->AddBuff(caster, spell_id);
				}
				else
					buffslot = -1;
			}
			else
			{
				buffslot = AddBuff(caster, spell_id, duration_override, -1, disable_buff_overwrite);
			}
			if(buffslot == -1)	// stacking failure
				return false;
		} else {
			buffslot = -2;	//represents not a buff I guess
		}
	}

#ifdef SPELL_EFFECT_SPAM
		Message(Chat::White, "You are affected by spell '%s' (id %d)", spell.name, spell_id);
		if(buffslot >= 0)
		{
			Message(Chat::White, "Buff slot:  %d  Duration:  %d tics", buffslot, buffs[buffslot].ticsremaining);
		}
#endif

	if(buffslot >= 0)
	{
		buffs[buffslot].melee_rune = 0;
		buffs[buffslot].magic_rune = 0;
		buffs[buffslot].hit_number = 0;

		if (spells[spell_id].hit_number > 0) {

			int numhit = spells[spell_id].hit_number;
			if (caster) {
				numhit += numhit * caster->GetFocusEffect(focusFcLimitUse, spell_id) / 100;
				numhit += caster->GetFocusEffect(focusIncreaseNumHits, spell_id);
			}
			buffs[buffslot].hit_number = numhit;
		}

		if (spells[spell_id].endurance_upkeep > 0)
			SetEndurUpkeep(true);

		if (IsClient() && CastToClient()->ClientVersionBit() & EQ::versions::maskUFAndLater)
		{
			EQApplicationPacket *outapp = MakeBuffsPacket(false);
			CastToClient()->FastQueuePacket(&outapp);
		}
	}

	if (IsClient()) {
		if (parse->SpellHasQuestSub(spell_id, EVENT_SPELL_EFFECT_CLIENT)) {
			const auto &export_string = fmt::format(
				"{} {} {} {}",
				caster ? caster->GetID() : 0,
				buffslot >= 0 ? buffs[buffslot].ticsremaining : 0,
				caster ? caster->GetLevel() : 0,
				buffslot
			);
			if (parse->EventSpell(EVENT_SPELL_EFFECT_CLIENT, nullptr, CastToClient(), spell_id, export_string, 0) != 0) {
				CalcBonuses();
				return true;
			}
		}
	} else if (IsNPC()) {
		if (parse->SpellHasQuestSub(spell_id, EVENT_SPELL_EFFECT_NPC)) {
			const auto &export_string = fmt::format(
				"{} {} {} {}",
				caster ? caster->GetID() : 0,
				buffslot >= 0 ? buffs[buffslot].ticsremaining : 0,
				caster ? caster->GetLevel() : 0,
				buffslot
			);
			if (parse->EventSpell(EVENT_SPELL_EFFECT_NPC, this, nullptr, spell_id, export_string, 0) != 0) {
				CalcBonuses();
				return true;
			}
		}
	} else if (IsBot()) {
		if (parse->SpellHasQuestSub(spell_id, EVENT_SPELL_EFFECT_BOT)) {
			const auto &export_string = fmt::format(
				"{} {} {} {}",
				caster ? caster->GetID() : 0,
				buffslot >= 0 ? buffs[buffslot].ticsremaining : 0,
				caster ? caster->GetLevel() : 0,
				buffslot
			);
			if (parse->EventSpell(EVENT_SPELL_EFFECT_BOT, this, nullptr, spell_id, export_string, 0) != 0) {
				CalcBonuses();
				return true;
			}
		}
	}

	if (IsVirusSpell(spell_id) && buffslot > -1) {
		if (!viral_timer.Enabled()) {
			viral_timer.Start(1000);
		}

		buffs[buffslot].virus_spread_time = zone->random.Int(GetSpellViralMinimumSpreadTime(spell_id), GetSpellViralMaximumSpreadTime(spell_id));
	}


	if (!IsDistanceModifierSpell(spell_id))
		SetSpellPowerDistanceMod(0);

	bool spell_trigger_cast_complete = false; //Used with SpellEffect::Spell_Trigger and SpellEffect::Chance_Best_in_Spell_Grp, true when spell has been triggered.

	// if buff slot, use instrument mod there, otherwise calc it
	uint32 instrument_mod = buffslot > -1 ? buffs[buffslot].instrument_mod : caster ? caster->GetInstrumentMod(spell_id) : 10;

	// iterate through the effects in the spell
	for (i = 0; i < EFFECT_COUNT; i++)
	{
		if(IsBlankSpellEffect(spell_id, i))
			continue;

		effect = spell.effect_id[i];
		effect_value = CalcSpellEffectValue(spell_id, i, caster_level, instrument_mod, caster ? caster : this);

		if(spell_id == SPELL_LAY_ON_HANDS && caster && caster->GetAA(aaImprovedLayOnHands))
			effect_value = GetMaxHP();

		if (GetSpellPowerDistanceMod())
			effect_value = effect_value*GetSpellPowerDistanceMod()/100;

		//Prevents effect from being applied
		if (spellbonuses.NegateEffects) {
			if (effect != SpellEffect::NegateSpellEffect && NegateSpellEffect(spell_id, effect)) {
				if (caster) {
					caster->Message(Chat::Red, "Part or all of this spell has lost its effectiveness."); //Placeholder msg, until live one is obtained.
				}
				continue;
			}
		}

#ifdef SPELL_EFFECT_SPAM
		effect_desc[0] = 0;
#endif
		switch(effect)
		{
			case SpellEffect::CurrentHP:	// nukes, heals; also regen/dot if a buff
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Current Hitpoints: %+i", effect_value);
#endif
				// SpellEffect::CurrentHP is calculated at first tick if its a dot/buff
				if (buffslot >= 0) {
					//This is here so dots with hit counters tic down on initial cast.
					if (caster && effect_value < 0) {
						caster->GetActDoTDamage(spell_id, effect_value, this, false);
					}
					break;
				}

				if (spells[spell_id].limit_value[i] && !PassCastRestriction(spells[spell_id].limit_value[i])) {
					break; //no messages are given on live if this fails.
				}

				// for offensive spells check if we have a spell rune on
				int64 dmg = effect_value;
				if(dmg < 0)
				{

					// take partial damage into account
					dmg = (int64) (dmg * partial / 100);

					//handles AAs and what not...
					if(caster) {
						if (reflect_effectiveness) {
							dmg = caster->GetActReflectedSpellDamage(spell_id, (int64)(spells[spell_id].base_value[i] * partial / 100), reflect_effectiveness);
						}
						else {
							dmg = caster->GetActSpellDamage(spell_id, dmg, this);
						}
						caster->ResourceTap(-dmg, spell_id);
					}

					if (dmg <= 0) {
						dmg = -dmg;
						Damage(caster, dmg, spell_id, spell.skill, false, buffslot, false);
					}
					//handles custom situation where quest function mitigation put high enough to allow damage to heal.
					else {
						HealDamage(dmg, caster);
					}
				}
				else if(dmg > 0) {
					//healing spell...

					if(caster)
						dmg = caster->GetActSpellHealing(spell_id, dmg, this);

					HealDamage(dmg, caster);
				}

#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Current Hitpoints: %+i  actual: %+i", effect_value, dmg);
#endif
				break;
			}

			case SpellEffect::CurrentHPOnce:	// used in buffs usually, see Courage
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Current Hitpoints Once: %+i", effect_value);
#endif
				if (spells[spell_id].limit_value[i] && !PassCastRestriction(spells[spell_id].limit_value[i])) {
					break; //no messages are given on live if this fails.
				}

				// hack fix for client health not reflecting server value
				last_hp = 0;
				int64 dmg = effect_value;

				//hardcoded for manaburn and life burn
				if (spell_id == SPELL_MANA_BURN || spell_id == SPELL_LIFE_BURN) {
					if (RuleB(Spells, LegacyManaburn)) {
						if (spell_id == SPELL_MANA_BURN && caster) { //Manaburn
							int manaburn_multiplier = zone->random.Int(150, 200); //Manaburn deals 150-200% of mana
							dmg = caster->GetMana() * manaburn_multiplier / 100;
							dmg *= -1;	//Damage should be negative
							dmg = caster->GetActSpellDamage(spell_id, dmg, this); // Spell can crit, so need this.  Damage cap handled in this function.
							LogSpellsDetail("manaburn_multiplier [{}], Mana [{}], Damage [{}]", manaburn_multiplier, caster->GetMana(), dmg);
							caster->SetMana(0);
						} else if (spell_id == SPELL_LIFE_BURN && caster) { //Lifeburn
							dmg = caster->GetHP() * -1;
							caster->SetHP(1);
							if (caster->IsClient()) {
								caster->CastToClient()->SetFeigned(true);
								caster->SendAppearancePacket(AppearanceType::Die, Animation::Lying);
							}
						}
					} else {
						if (spell_id == SPELL_MANA_BURN && caster) { //Manaburn
							dmg = caster->GetMana() * -3;
							caster->SetMana(0);
						} else if (spell_id == SPELL_LIFE_BURN && caster) { //Lifeburn
							dmg = caster->GetHP(); // just your current HP
							caster->SetHP(dmg / 4); // 2003 patch notes say ~ 1/4 HP. Should this be 1/4 your current HP or do 3/4 max HP dmg? Can it kill you?
							dmg = -dmg;
						}

						if (dmg < 0) {
							dmg = -dmg;
							Damage(caster, dmg, spell_id, spell.skill, false, buffslot, false);
						} else {
							HealDamage(dmg, caster);
						}
						break;
					}
				}
				//normal effects
				else {
					if (dmg < 0){
						dmg = (int64)(dmg * partial / 100);

						if (caster) {
							if (reflect_effectiveness) {
								dmg = caster->GetActReflectedSpellDamage(spell_id, (int64)(spells[spell_id].base_value[i] * partial / 100), reflect_effectiveness);
							}
							else {
								dmg = caster->GetActSpellDamage(spell_id, dmg, this);
							}
							caster->ResourceTap(-dmg, spell_id);
						}

						if (dmg <= 0) {
							dmg = -dmg;
							Damage(caster, dmg, spell_id, spell.skill, false, buffslot, false);
						}
						else {
							HealDamage(dmg, caster);
						}
					}
					else if (dmg > 0) {
						//do not apply focus/critical to buff spells
						if (caster && !IsEffectInSpell(spell_id, SpellEffect::TotalHP)) {
							dmg = caster->GetActSpellHealing(spell_id, dmg, this);
						}
						HealDamage(dmg, caster);
					}
				}
				break;
			}

			case SpellEffect::PercentalHeal:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Percental Heal: %+i (%d%% max)", spell.max_value[i], effect_value);
#endif
				int64 val = GetMaxHP() * spell.base_value[i] / 100;

				//This effect can also do damage by percent.
				if (val < 0) {

					if (spell.max_value[i] && -val > spell.max_value[i])
						val = -spell.max_value[i];

					if (caster)
						val = caster->GetActSpellDamage(spell_id, val, this);

				}

				else
				{
					if (spell.max_value[i] && val > spell.max_value[i])
						val = spell.max_value[i];

					if(caster)
						val = caster->GetActSpellHealing(spell_id, val, this);
				}

				if (val < 0)
					Damage(caster, -val, spell_id, spell.skill, false, buffslot, false);
				else
					HealDamage(val, caster);

				break;
			}

			case SpellEffect::CompleteHeal:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Complete Heal");
#endif
				int64 val = 7500 * effect_value;
				if (caster) {
					val = caster->GetActSpellHealing(spell_id, val, this);
				}
				if (val > 0) {
					HealDamage(val, caster);
				}

				break;
			}

			case SpellEffect::CurrentMana:
			{
				// Bards don't get mana from effects, good or bad.
				if(GetClass() == Class::Bard)
					break;
				if(IsManaTapSpell(spell_id)) {
					if (!IsPureMeleeClass()) {
#ifdef SPELL_EFFECT_SPAM
						snprintf(effect_desc, _EDLEN, "Current Mana: %+i", effect_value);
#endif
						SetMana(GetMana() + effect_value);
						if (caster)
							caster->SetMana(caster->GetMana() + std::abs(effect_value));

						if (effect_value < 0)
							TryTriggerOnCastRequirement();
#ifdef SPELL_EFFECT_SPAM
						if (caster)
							caster->Message(Chat::White, "You have gained %+i mana!", effect_value);
#endif
					}
				}
				else {
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Current Mana: %+i", effect_value);
#endif
				if (buffslot >= 0)
					break;

				SetMana(GetMana() + effect_value);
				if (effect_value < 0)
					TryTriggerOnCastRequirement();
				}

				break;
			}

			case SpellEffect::CurrentManaOnce:
			{
				// Bards don't get mana from effects, good or bad.
				if(GetClass() == Class::Bard)
					break;
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Current Mana Once: %+i", effect_value);
#endif
				SetMana(GetMana() + effect_value);
				break;
			}

			case SpellEffect::Translocate:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Translocate: %s %d %d %d heading %d",
					spell.teleport_zone, spell.base_value[1], spell.base_value[0],
					spell.base_value[2], spell.base_value[3]
				);
#endif
				if(IsClient())
				{

					if(caster)
						CastToClient()->SendOPTranslocateConfirm(caster, spell_id);

				}
				break;
			}

			case SpellEffect::Succor:
			{

				float x, y, z, heading;
				const char *target_zone = nullptr;

				x = static_cast<float>(spell.base_value[1]);
				y = static_cast<float>(spell.base_value[0]);
				z = static_cast<float>(spell.base_value[2]);
				heading = static_cast<float>(spell.base_value[3]);

				if(!strcmp(spell.teleport_zone, "same"))
				{
					target_zone = 0;
				}
				else
				{
					target_zone = spell.teleport_zone;
					if(IsNPC() && target_zone != zone->GetShortName()){
						if(!GetOwner()){
							CastToNPC()->Depop();
							break;
						}else{
							if(!GetOwner()->IsClient())
								CastToNPC()->Depop();
								break;
						}
					}
				}

				if(IsClient())
				{
					if(zone->random.Roll(RuleI(Spells, SuccorFailChance))) { //2% Fail chance by default

						if(IsClient()) {
							CastToClient()->MessageString(Chat::SpellFailure,SUCCOR_FAIL);
						}
						break;
					}

					// Below are the spellid's for known evac/succor spells that send player
					// to the current zone's safe points.

					// Succor = 1567
					// Lesser Succor = 2183
					// Evacuate = 1628
					// Lesser Evacuate = 2184
					// Decession = 2558
					// Greater Decession = 3244
					// Egress = 1566

					if (!target_zone) {
#ifdef SPELL_EFFECT_SPAM
						LogDebug("Succor/Evacuation Spell In Same Zone");
#endif
						if (IsClient()) {
							if (HasPet()) {
								if (RuleB(Spells, EvacClearCharmPet) && GetPet()->IsCharmed()) {
									GetPet()->BuffFadeByEffect(SpellEffect::Charm);
								}
							}

							CastToClient()->MovePC(zone->GetZoneID(), zone->GetInstanceID(), x, y, z, heading, 0, EvacToSafeCoords);
						} else {
							GMMove(x, y, z, heading);
						}

						if (RuleB(Spells, EvacClearAggroInSameZone)) {
							entity_list.ClearAggro(this);
						}
					} else {
#ifdef SPELL_EFFECT_SPAM
						LogDebug("Succor/Evacuation Spell To Another Zone");
#endif
						if(IsClient())
							CastToClient()->MovePC(target_zone, x, y, z, heading);
					}
				}

				break;
			}
			case SpellEffect::GateCastersBindpoint: // Used on Teleport Bind.
			case SpellEffect::Teleport:	// gates, rings, circles, etc
			case SpellEffect::Teleport2:
			{
				float x, y, z, heading;
				const char *target_zone = nullptr;

				x = static_cast<float>(spell.base_value[1]);
				y = static_cast<float>(spell.base_value[0]);
				z = static_cast<float>(spell.base_value[2]);
				heading = static_cast<float>(spell.base_value[3]);

				if(!strcmp(spell.teleport_zone, "same"))
				{
					target_zone = 0;
				}
				else
				{
					target_zone = spell.teleport_zone;

					if(IsNPC() && target_zone != zone->GetShortName()){
						if(!GetOwner()){
							CastToNPC()->Depop();
							break;
						}else{
							if(!GetOwner()->IsClient())
								CastToNPC()->Depop();
								break;
						}
					}
				}

				if (effect == SpellEffect::GateCastersBindpoint && caster && caster->IsClient())
				{ // Teleport Bind uses caster's bind point
					int index = spells[spell_id].base_value[i] - 1;
					if (index < 0 || index > 4)
						index = 0;
					x = caster->CastToClient()->GetBindX(index);
					y = caster->CastToClient()->GetBindY(index);
					z = caster->CastToClient()->GetBindZ(index);
					heading = caster->CastToClient()->GetBindHeading(index);
					//target_zone = caster->CastToClient()->GetBindZoneId(); target_zone doesn't work due to const char
					CastToClient()->MovePC(caster->CastToClient()->GetBindZoneID(index), 0, x, y, z, heading);
					break;
				}

#ifdef SPELL_EFFECT_SPAM
				const char *efstr = "Teleport";
				if(effect == SpellEffect::Teleport)
					efstr = "Teleport v1";
				else if(effect == SpellEffect::Teleport2)
					efstr = "Teleport v2";
				else if(effect == SpellEffect::Succor)
					efstr = "Succor";

				snprintf(effect_desc, _EDLEN,
					"%s: %0.2f, %0.2f, %0.2f heading %0.2f in %s",
					efstr, x, y, z, heading, target_zone ? target_zone : "same zone"
				);
#endif
				if(IsClient())
				{
					if(!target_zone)
						CastToClient()->MovePC(zone->GetZoneID(), zone->GetInstanceID(), x, y, z, heading);
					else
						CastToClient()->MovePC(target_zone, x, y, z, heading);
				}
				else{
					if(!target_zone)
						GMMove(x, y, z, heading);
				}
				break;
			}

			case SpellEffect::FleshToBone:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Flesh To Bone");
#endif
				if(IsClient()){
					EQ::ItemInstance* transI = CastToClient()->GetInv().GetItem(EQ::invslot::slotCursor);
					if (transI && transI->IsClassCommon() && transI->IsStackable()){
						int16 fcharges = transI->GetCharges();
						//Does it sound like meat... maybe should check if it looks like meat too...
						if(strstr(transI->GetItem()->Name, "meat") ||
							strstr(transI->GetItem()->Name, "Meat") ||
							strstr(transI->GetItem()->Name, "flesh") ||
							strstr(transI->GetItem()->Name, "Flesh") ||
							strstr(transI->GetItem()->Name, "parts") ||
							strstr(transI->GetItem()->Name, "Parts")){
							CastToClient()->DeleteItemInInventory(EQ::invslot::slotCursor, fcharges, true);
							CastToClient()->SummonItem(13073, fcharges);
						}
						else{
							Message(Chat::Red, "You can only transmute flesh to bone.");
						}
					} else{
						Message(Chat::Red, "You can only transmute flesh to bone.");
					}
				}
				break;
			}

			case SpellEffect::GroupFearImmunity:{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Group Fear Immunity");
#endif
				//Added client messages to give some indication this effect is active.
				// Is there a message generated? Too disgusted by raids.
				uint32 time = spell.base_value[i] * 10 * 1000;
				if (caster && caster->IsClient()) {
					if (caster->IsGrouped()) {
						auto group = caster->GetGroup();
						for (int i = 0; i < 6; ++i)
							if (group->members[i])
								group->members[i]->aa_timers[aaTimerWarcry].Start(time);
					} else if (caster->IsRaidGrouped()) {
						auto raid = caster->GetRaid();
						uint32 gid = raid->GetGroup(caster->CastToClient());
						if (gid < 12)
							for (int i = 0; i < MAX_RAID_MEMBERS; ++i)
								if (raid->members[i].member && raid->members[i].group_number == gid)
									raid->members[i].member->aa_timers[aaTimerWarcry].Start(time);
					}
				}
				break;
			}

			case SpellEffect::AddFaction:
			{
				if (caster && !IsPet() && GetPrimaryFaction() > 0) {
					caster->AddFactionBonus(GetPrimaryFaction(),effect_value);
				}
				break;
			}

			case SpellEffect::Stun:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Stun: %d msec", effect_value);
#endif
				//Typically we check for immunities else where but since stun immunities are different and only
				//Block the stun part and not the whole spell, we do it here, also do the message here so we wont get the message on a resist
				int max_level = spell.max_value[i];
				//max_level of 0 means we assume a default of 55.
				if (max_level == 0)
					max_level = RuleI(Spells, BaseImmunityLevel);
				// NPCs get to ignore max_level for their spells.
				// Ignore if spell is beneficial (ex. Harvest)
				if (IsDetrimentalSpell(spell.id) && (GetSpecialAbility(SpecialAbility::StunImmunity) ||
						((GetLevel() > max_level) && caster && (!caster->IsNPC() ||
						(caster->IsNPC() && !RuleB(Spells, NPCIgnoreBaseImmunity))))))
				{
					if (caster)
						caster->MessageString(Chat::SpellFailure, IMMUNE_STUN);
				} else {
					int stun_resist = itembonuses.StunResist+spellbonuses.StunResist;
					if (IsClient() || IsBot()) {
						stun_resist += aabonuses.StunResist;
					}

					if (stun_resist <= 0 || zone->random.Int(0,99) >= stun_resist) {
						LogCombat("Stunned. We had [{}] percent resist chance", stun_resist);

						if (caster && (caster->IsClient() || caster->IsBot())) {
							effect_value += effect_value*caster->GetFocusEffect(focusFcStunTimeMod, spell_id)/100;
						}

						Stun(effect_value);
					} else {
						if (IsClient())
							MessageString(Chat::Stun, SHAKE_OFF_STUN);

						LogCombat("Stun Resisted. We had [{}] percent resist chance", stun_resist);
					}
				}
				break;
			}

			case SpellEffect::Charm:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Charm: %+i (up to lvl %d)", effect_value, spell.max_value[i]);
#endif

				if (!caster) {    // can't be someone's pet unless we know who that someone is
					break;
				}

				if (IsNPC()) {
					CastToNPC()->SaveGuardSpotCharm();
				}
				InterruptSpell();
				entity_list.RemoveDebuffs(this);
				entity_list.RemoveFromHateLists(this);
				WipeHateList();

				Mob *my_pet = GetPet();
				if(my_pet)
				{
					my_pet->Kill();
				}

				caster->SetPet(this);
				SetOwnerID(caster->GetID());
				SetPetOrder(PetOrder::Follow);
				SetAppearance(eaStanding);
				// Client has saved previous pet sit/stand - make all new pets
				// stand and follow on charm.
				if (caster->IsClient()) {
					Client *cpet = caster->CastToClient();
					cpet->SetPetCommandState(PetButton::Sit, PetButtonState::Off);
					cpet->SetPetCommandState(PetButton::Follow, PetButtonState::On);
					cpet->SetPetCommandState(PetButton::Guard, PetButtonState::Off);
					cpet->SetPetCommandState(PetButton::Stop, PetButtonState::Off);
				}

				SetPetType(PetType::Charmed);

				// This was done in AddBuff, but we were not a pet yet, so
				// the target windows didn't get updated.
				EQApplicationPacket *outapp = MakeBuffsPacket();
				entity_list.QueueClientsByTarget(this, outapp, false, nullptr, true, false, EQ::versions::maskSoDAndLater);
				safe_delete(outapp);

				if(caster->IsClient()){
					auto app = new EQApplicationPacket(OP_Charm, sizeof(Charm_Struct));
					Charm_Struct *ps = (Charm_Struct*)app->pBuffer;
					ps->owner_id = caster->GetID();
					ps->pet_id = GetID();
					ps->command = 1;
					entity_list.QueueClients(this, app);
					safe_delete(app);
					SendPetBuffsToClient();
					SendAppearancePacket(AppearanceType::Pet, caster->GetID(), true, true);
				}

				if (IsClient()) {
					CastToClient()->AI_Start();
				} else if (IsNPC()) {
					CastToNPC()->SetPetSpellID(0);    //not a pet spell.
					CastToNPC()->ModifyStatsOnCharm(false, caster);
				}

				bool bBreak = false;

				// define spells with fixed duration
				// charm spells with -1 in field 209 are all of fixed duration, so lets use that instead of spell_ids
				if (spells[spell_id].no_resist) {
					bBreak = true;
				}

				if (buffslot > -1) {
					if (!bBreak) {
						int resistMod = static_cast<int>(partial) + (GetCHA() / 25);
						resistMod = resistMod > 100 ? 100 : resistMod;
						buffs[buffslot].ticsremaining = resistMod * buffs[buffslot].ticsremaining / 100;
					}

					if (IsOfClientBot() && buffs[buffslot].ticsremaining > RuleI(Character, MaxCharmDurationForPlayerCharacter)) {
						buffs[buffslot].ticsremaining = RuleI(Character, MaxCharmDurationForPlayerCharacter);
					}
				}

				break;
			}


			case SpellEffect::SenseDead:
			case SpellEffect::SenseSummoned:
			case SpellEffect::SenseAnimals:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Sense Target: %+i", effect_value);
#endif
				if(IsClient())
				{
					if (CastToClient()->ClientVersionBit() & EQ::versions::maskSoDAndLater)
					{
						uint8 bt = BodyType::Undead;

						int MessageID = SENSE_UNDEAD;

						if(effect == SpellEffect::SenseSummoned)
						{
							bt = BodyType::Summoned;
							MessageID = SENSE_SUMMONED;
						}
						else if(effect == SpellEffect::SenseAnimals)
						{
							bt = BodyType::Animal;
							MessageID = SENSE_ANIMAL;
						}

						Mob *ClosestMob = entity_list.GetClosestMobByBodyType(this, bt, true);

						if(ClosestMob)
						{
							MessageString(Chat::Spells, MessageID);
							SetHeading(CalculateHeadingToTarget(ClosestMob->GetX(), ClosestMob->GetY()));
							SetTarget(ClosestMob);
							CastToClient()->SendTargetCommand(ClosestMob->GetID());
							SentPositionPacket(0.0f, 0.0f, 0.0f, 0.0f, 0, true);
						}
						else
							MessageString(Chat::Red, SENSE_NOTHING);
					}
				}
				break;
			}

			case SpellEffect::Fear:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Fear: %+i", effect_value);
#endif
				if (IsOfClientBot() && buffslot > -1) {
					if (buffs[buffslot].ticsremaining > RuleI(Character, MaxFearDurationForPlayerCharacter)) {
						buffs[buffslot].ticsremaining = RuleI(Character, MaxFearDurationForPlayerCharacter);
					}
				}


				if (RuleB(Combat, EnableFearPathing)) {
					if (IsClient())
					{
						CastToClient()->AI_Start();
					}

					CalculateNewFearpoint();
					if (currently_fleeing) {
						break;
					}
				}
				else if (buffslot > -1) {
					Stun(buffs[buffslot].ticsremaining * 6000 - (6000 - tic_timer.GetRemainingTime()));
				}
				break;
			}

			case SpellEffect::BindAffinity: //TO DO: Add support for secondary and tertiary gate abilities
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Bind Affinity");
#endif
				if (IsClient()) {
					if (CastToClient()->GetGM() || RuleB(Character, BindAnywhere)) {
						if (CastToClient()->GetGM()) {
							Message(Chat::White, "Your GM flag allows you to bind anywhere.");
						}

						auto action_packet =
						    new EQApplicationPacket(OP_Action, sizeof(Action_Struct));
						Action_Struct* action = (Action_Struct*) action_packet->pBuffer;

						static EQApplicationPacket p(OP_Damage, sizeof(CombatDamage_Struct));
						auto                       cd = (CombatDamage_Struct *) p.pBuffer;

						action->target = GetID();
						action->source = caster ? caster->GetID() : GetID();
						action->level = 65;
						action->instrument_mod = 10;
						action->hit_heading = GetHeading();
						action->type = 231;
						action->spell = spell_id;
						action->effect_flag = 4;

						cd->target = action->target;
						cd->source = action->source;
						cd->type = action->type;
						cd->spellid = action->spell;
						cd->hit_heading = action->hit_heading;

						CastToClient()->QueuePacket(action_packet);

						if (caster && caster->IsClient() && caster != this) {
							caster->CastToClient()->QueuePacket(action_packet);
						}

						CastToClient()->QueuePacket(&p);

						if (caster && caster->IsClient() && caster != this) {
							caster->CastToClient()->QueuePacket(&p);
						}

						CastToClient()->SetBindPoint(spells[spell_id].base_value[i] - 1);
						Save();
						safe_delete(action_packet);
					} else {
						if (!zone->CanBind()) {
							MessageString(Chat::SpellFailure, CANNOT_BIND);
							break;
						}

						if (!zone->IsCity()) {
							if (caster != this && !zone->IsSpecialBindLocation(GetPosition())) {
								MessageString(Chat::SpellFailure, CANNOT_BIND);
								break;
							} else {
								auto action_packet = new EQApplicationPacket(
								    OP_Action, sizeof(Action_Struct));
								Action_Struct* action = (Action_Struct*) action_packet->pBuffer;

								static EQApplicationPacket p(OP_Damage, sizeof(CombatDamage_Struct));
								auto                       cd = (CombatDamage_Struct *) p.pBuffer;

								action->target = GetID();
								action->source = caster ? caster->GetID() : GetID();
								action->level = 65;
								action->instrument_mod = 10;
								action->hit_heading = GetHeading();
								action->type = 231;
								action->spell = spell_id;
								action->effect_flag = 4;

								cd->target = action->target;
								cd->source = action->source;
								cd->type = action->type;
								cd->spellid = action->spell;
								cd->hit_heading = action->hit_heading;

								CastToClient()->QueuePacket(action_packet);

								if (caster->IsClient() && caster != this) {
									caster->CastToClient()->QueuePacket(action_packet);
								}

								CastToClient()->QueuePacket(&p);

								if (caster->IsClient() && caster != this) {
									caster->CastToClient()->QueuePacket(&p);
								}

								CastToClient()->SetBindPoint(spells[spell_id].base_value[i] - 1);
								Save();
								safe_delete(action_packet);
							}
						} else {
							auto action_packet =
							    new EQApplicationPacket(OP_Action, sizeof(Action_Struct));
							Action_Struct* action = (Action_Struct*) action_packet->pBuffer;

							static EQApplicationPacket p(OP_Damage, sizeof(CombatDamage_Struct));
							auto                       cd = (CombatDamage_Struct *) p.pBuffer;

							action->target = GetID();
							action->source = caster ? caster->GetID() : GetID();
							action->level = 65;
							action->instrument_mod = 10;
							action->hit_heading = GetHeading();
							action->type = 231;
							action->spell = spell_id;
							action->effect_flag = 4;

							cd->target = action->target;
							cd->source = action->source;
							cd->type = action->type;
							cd->spellid = action->spell;
							cd->hit_heading = action->hit_heading;

							CastToClient()->QueuePacket(action_packet);

							if (caster->IsClient() && caster != this) {
								caster->CastToClient()->QueuePacket(action_packet);
							}

							CastToClient()->QueuePacket(&p);

							if (caster->IsClient() && caster != this) {
								caster->CastToClient()->QueuePacket(&p);
							}

							CastToClient()->SetBindPoint(spells[spell_id].base_value[i] - 1);
							Save();
							safe_delete(action_packet);
						}
					}
				}
				break;
			}

			case SpellEffect::Gate:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Gate");
#endif
				if(!spellbonuses.AntiGate){

					if(zone->random.Roll(effect_value))
						Gate(spells[spell_id].limit_value[i] - 1);
					else if (caster)
						caster->MessageString(Chat::SpellFailure,GATE_FAIL);
				}
				break;
			}

			case SpellEffect::CancelMagic:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Cancel Magic: %d", effect_value);
#endif
				if(GetSpecialAbility(SpecialAbility::DispellImmunity)){
					if (caster) {
						caster->MessageString(Chat::SpellFailure, SPELL_NO_EFFECT, spells[spell_id].name);
					}
					break;
				}

				DispelMagic(caster, spell_id, effect_value);
				break;
			}

			case SpellEffect::DispelDetrimental:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Dispel Detrimental: %d", effect_value);
#endif
				if(GetSpecialAbility(SpecialAbility::DispellImmunity)){
					if (caster)
						caster->MessageString(Chat::SpellFailure, SPELL_NO_EFFECT, spells[spell_id].name);
					break;
				}
				/*
					TODO: Parsing shows there is no level modifier. However, a consistent -2% modifer was
					found on spell with value 950 (95% spells would have 7% failure rates).
					Further investigation is needed. ~ Kayen
				*/
				int chance = spells[spell_id].base_value[i];
				int buff_count = GetMaxTotalSlots();
				for(int slot = 0; slot < buff_count; slot++) {
					if (IsValidSpell(buffs[slot].spellid) &&
						IsDetrimentalSpell(buffs[slot].spellid) &&
						spells[buffs[slot].spellid].dispel_flag == 0)
					{
						if (zone->random.Int(1, 1000) <= chance){
							BuffFadeBySlot(slot);
							slot = buff_count;
						}
					}
				}
				break;
			}

			case SpellEffect::DispelBeneficial:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Dispel Beneficial: %d", effect_value);
#endif
				if(GetSpecialAbility(SpecialAbility::DispellImmunity)){
					if (caster)
						caster->MessageString(Chat::SpellFailure, SPELL_NO_EFFECT, spells[spell_id].name);
					break;
				}

				int chance = spells[spell_id].base_value[i];
				int buff_count = GetMaxTotalSlots();
				for(int slot = 0; slot < buff_count; slot++) {
					if (IsValidSpell(buffs[slot].spellid) &&
						IsBeneficialSpell(buffs[slot].spellid) &&
						spells[buffs[slot].spellid].dispel_flag == 0)
					{
						if (zone->random.Int(1, 1000) <= chance) {
							BuffFadeBySlot(slot);
							slot = buff_count;
						}
					}
				}
				break;
			}

			case SpellEffect::Purify:
			{
				//Attempt to remove up to base amount of detrimental effects (excluding charm, fear, resurrection, and revival sickness).
				int purify_count = spells[spell_id].base_value[i];
				if (purify_count > GetMaxTotalSlots()) {
					purify_count = GetMaxTotalSlots();
				}

				for(int slot = 0; slot < purify_count; slot++) {
					if (IsValidSpell(buffs[slot].spellid) && IsDetrimentalSpell(buffs[slot].spellid)){

						if (!IsEffectInSpell(buffs[slot].spellid, SpellEffect::Charm) &&
							!IsEffectInSpell(buffs[slot].spellid, SpellEffect::Fear) &&
							buffs[slot].spellid != SPELL_RESURRECTION_SICKNESS &&
							buffs[slot].spellid != SPELL_RESURRECTION_SICKNESS2 &&
							buffs[slot].spellid != SPELL_RESURRECTION_SICKNESS3 &&
							buffs[slot].spellid != SPELL_RESURRECTION_SICKNESS4 &&
							buffs[slot].spellid != SPELL_REVIVAL_SICKNESS)
						{
							BuffFadeBySlot(slot);
						}
					}
				}
				break;
			}

			case SpellEffect::Mez:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Mesmerize");
#endif
				Mesmerize();
				break;
			}

			case SpellEffect::SummonItem:
			{
				const EQ::ItemData *item = database.GetItem(spell.base_value[i]);
#ifdef SPELL_EFFECT_SPAM
				const char *itemname = item ? item->Name : "*Unknown Item*";
				snprintf(effect_desc, _EDLEN, "Summon Item: %s (id %d)", itemname, spell.base_value[i]);
#endif
				if (!item) {
					Message(Chat::Red, "Unable to summon item %d. Item not found.", spell.base_value[i]);
				} else if (IsClient()) {
					Client *c = CastToClient();
					if (c->CheckLoreConflict(item)) {
						c->DuplicateLoreMessage(spell.base_value[i]);
					} else {
						int charges;
						if (item->Stackable)
							charges = (spell.formula[i] > item->StackSize) ? item->StackSize : spell.formula[i];
						else if (item->MaxCharges) // mod rods etc
							charges = item->MaxCharges;
						else
							charges = 1;

						if (charges < 1)
							charges = 1;

						if (SummonedItem) {
							c->PushItemOnCursor(*SummonedItem);
							c->SendItemPacket(EQ::invslot::slotCursor, SummonedItem, ItemPacketLimbo);
							safe_delete(SummonedItem);
						}
						SummonedItem = database.CreateItem(spell.base_value[i], charges);
					}
				}

				break;
			}
			case SpellEffect::SummonItemIntoBag:
			{
				const EQ::ItemData *item = database.GetItem(spell.base_value[i]);
				if (!item) {
					Message(Chat::Red, "Unable to summon item %d. Item not found.", spell.base_value[i]);
					break;
				}

#ifdef SPELL_EFFECT_SPAM
				const char *itemname = item ? item->Name : "*Unknown Item*";
				snprintf(effect_desc, _EDLEN, "Summon Item In Bag: %s (id %d)", itemname, spell.base_value[i]);
#endif
				uint8 slot;

				if (!SummonedItem || !SummonedItem->IsClassBag()) {
					if (caster)
						caster->Message(Chat::Red, "SE_SummonItemIntoBag but no bag has been summoned!");
				} else if ((slot = SummonedItem->FirstOpenSlot()) == 0xff) {
					if (caster)
						caster->Message(Chat::Red, "SE_SummonItemIntoBag but no room in summoned bag!");
				} else if (IsClient()) {
					if (CastToClient()->CheckLoreConflict(item)) {
						CastToClient()->DuplicateLoreMessage(spell.base_value[i]);
					} else {
						int charges;

						if (item->Stackable)
							charges = (spell.formula[i] > item->StackSize) ? item->StackSize : spell.formula[i];
						else if (item->MaxCharges) // mod rods, not sure if there are actual examples of this for IntoBag
							charges = item->MaxCharges;
						else
							charges = 1;

						if (charges < 1)
							charges = 1;

						EQ::ItemInstance *SubItem = database.CreateItem(spell.base_value[i], charges);
						if (SubItem != nullptr) {
							SummonedItem->PutItem(slot, *SubItem);
							safe_delete(SubItem);
						}
					}
				}

				break;
			}

			case SpellEffect::SummonBSTPet:
			case SpellEffect::NecPet:
			case SpellEffect::SummonPet:
			case SpellEffect::Familiar:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Summon %s: %s", (effect==SE_Familiar)?"Familiar":"Pet", spell.teleport_zone);
#endif
				if(GetPet())
				{
					MessageString(Chat::Shout, ONLY_ONE_PET);
				}
				else
				{
					MakePet(spell_id, spell.teleport_zone);
					// TODO: we need to sync the states for these clients ...
					// Will fix buttons for now
					Mob *pet=GetPet();
					if (IsClient() && pet) {
						auto c = CastToClient();
						if (c->ClientVersionBit() & EQ::versions::maskUFAndLater) {
							c->SetPetCommandState(PetButton::Sit, PetButtonState::Off);
							c->SetPetCommandState(PetButton::Stop, PetButtonState::Off);
							c->SetPetCommandState(PetButton::Regroup, PetButtonState::Off);
							c->SetPetCommandState(PetButton::Follow, PetButtonState::On);
							c->SetPetCommandState(PetButton::Guard, PetButtonState::Off);
							// Creating pet from spell - taunt always false
							// If suspended pet - that will be restore there
							// If logging in, client will send toggle
							c->SetPetCommandState(PetButton::Hold, PetButtonState::Off);
							c->SetPetCommandState(PetButton::GreaterHold, PetButtonState::Off);
							c->SetPetCommandState(PetButton::Focus, PetButtonState::Off);
							c->SetPetCommandState(PetButton::SpellHold, PetButtonState::Off);
						}
					}
				}
				break;
			}

			case SpellEffect::DivineAura:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Invulnerability");
#endif
				if (spell_id == 4789 && buffslot > -1) { // Touch of the Divine - Divine Save
					buffs[buffslot].ticsremaining = spells[spell_id].buff_duration;
				} // Prevent focus/aa buff extension

				SetInvul(true);
				break;
			}

			case SpellEffect::ShadowStep:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Shadow Step: %d", effect_value);
#endif
				if(IsNPC())	// see Song of Highsun - sends mob home
				{
					Gate();
				}
				// shadow step is handled by client already, nothing required
				break;
			}

			case SpellEffect::Blind:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Blind: %+i", effect_value);
#endif
				// 'cure blind'
				if (IsBeneficialSpell(spell_id) && spells[spell_id].buff_duration == 0) {
					int buff_count = GetMaxBuffSlots();
					for (int slot = 0; slot < buff_count; slot++) {
						if (IsValidSpell(buffs[slot].spellid) && IsEffectInSpell(buffs[slot].spellid, SpellEffect::Blind)) {
							if (caster && TryDispel(caster->GetCasterLevel(spell_id), buffs[slot].casterlevel, 1)) {
								BuffFadeBySlot(slot);
								slot = buff_count;
							}
						}
					}
				}
				else if (!IsClient()) {
					CalculateNewFearpoint();
				}
				break;
			}

			case SpellEffect::Rune:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Melee Absorb Rune: %+i", effect_value);
#endif
				if (buffslot > -1) {
					buffs[buffslot].melee_rune = effect_value;
				}
				break;
			}

			case SpellEffect::AbsorbMagicAtt:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Spell Absorb Rune: %+i", effect_value);
#endif
				if (effect_value > 0 && buffslot > -1) {
					buffs[buffslot].magic_rune = effect_value;
				}

				break;
			}

			case SpellEffect::MitigateMeleeDamage:
			{
				if (buffslot > -1) {
					buffs[buffslot].melee_rune = spells[spell_id].max_value[i];
				}
				break;
			}

			case SpellEffect::MeleeThresholdGuard:
			{
				if (buffslot > -1) {
					buffs[buffslot].melee_rune = spells[spell_id].max_value[i];
				}
				break;
			}

			case SpellEffect::SpellThresholdGuard:
			{
				if (buffslot > -1) {
					buffs[buffslot].magic_rune = spells[spell_id].max_value[i];
				}
				break;
			}

			case SpellEffect::MitigateSpellDamage:
			{
				if (buffslot > -1) {
					buffs[buffslot].magic_rune = spells[spell_id].max_value[i];
				}
				break;
			}

			case SpellEffect::MitigateDotDamage:
			{
				if (buffslot > -1) {
					buffs[buffslot].dot_rune = spells[spell_id].max_value[i];
				}
				break;
			}

			case SpellEffect::DistanceRemoval:
			{
				if (buffslot > -1) {
					buffs[buffslot].caston_x = int(GetX());
					buffs[buffslot].caston_y = int(GetY());
					buffs[buffslot].caston_z = int(GetZ());
				}
				break;
			}

			case SpellEffect::Levitate:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Levitate");
#endif
				//this sends the levitate packet to everybody else
				//who does not otherwise receive the buff packet.
				if (spells[spell_id].limit_value[i] == 1) {
					SendAppearancePacket(AppearanceType::FlyMode, EQ::constants::GravityBehavior::LevitateWhileRunning, true, true);
				}
				else {
					SendAppearancePacket(AppearanceType::FlyMode, EQ::constants::GravityBehavior::Levitating, true, true);
				}
				break;
			}

			case SpellEffect::DeathSave: {

				int16 mod = 0;

				if(caster) {
					mod =	caster->aabonuses.UnfailingDivinity +
							caster->itembonuses.UnfailingDivinity +
							caster->spellbonuses.UnfailingDivinity;
				}

				if (buffslot > -1) {
					buffs[buffslot].ExtraDIChance = mod;
				}
  				break;
 			}

			case SpellEffect::Illusion:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Illusion: race %d", effect_value);
#endif
				if (caster && caster->IsOfClientBot()) {
					Mob* target = IsClient() ? static_cast<Mob*>(CastToClient()) : static_cast<Mob*>(CastToBot());

					if (target && target->GetIllusionBlock()) {
						break;
					}
				}

				ApplySpellEffectIllusion(spell_id, caster, buffslot, spells[spell_id].base_value[i], spells[spell_id].limit_value[i], spells[spell_id].max_value[i]);
				break;
			}

			case SpellEffect::IllusionCopy:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Illusion Copy");
#endif
				if (caster && caster->IsOfClientBot()) {
					Mob* target = IsClient() ? static_cast<Mob*>(CastToClient()) : static_cast<Mob*>(CastToBot());

					if (target && target->GetIllusionBlock()) {
						break;
					}
				}

				if(caster && caster->GetTarget()){
						SendIllusionPacket
						(
							AppearanceStruct{
								.gender_id = caster->GetTarget()->GetGender(),
								.race_id = caster->GetTarget()->GetRace(),
								.texture = caster->GetTarget()->GetTexture(),
							}
						);
						caster->SendAppearancePacket(AppearanceType::Size, static_cast<uint32>(caster->GetTarget()->GetSize()));

						for (int x = EQ::textures::textureBegin; x <= EQ::textures::LastTintableTexture; x++)
							caster->SendWearChange(x);
				}
				break;
			}

			case SpellEffect::WipeHateList:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Memory Blur: %d", effect_value);
#endif
				//Memory blur component of Mez spells is not checked again if Mez is recast on a target that is already mezed
				if (!CanMemoryBlurFromMez && IsEffectInSpell(spell_id, SpellEffect::Mez)) {
					break;
				}

				int wipechance = 0;

				if (RuleB(Spells, EvacClearAggroInSameZone) && !wipechance && IsClient()) {
					entity_list.ClearAggro(this);
				}

				if (caster) {
					wipechance = caster->GetMemoryBlurChance(effect_value);
				}

				if(zone->random.Roll(wipechance))
				{
					if(IsAIControlled())
					{
						WipeHateList();
					}
					Message(Chat::Red, "Your mind fogs. Who are my friends? Who are my enemies?... it was all so clear a moment ago...");
				}
				break;
			}

			case SpellEffect::SpinTarget:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Spin: %d", effect_value);
#endif
				// the spinning is handled by the client
				int max_level = spells[spell_id].max_value[i];
				if(max_level == 0)
					max_level = RuleI(Spells, BaseImmunityLevel); // Default max is 55 level limit

				// NPCs ignore level limits in their spells
				if(GetSpecialAbility(SpecialAbility::StunImmunity) ||
					((GetLevel() > max_level)
					&& caster && (!caster->IsNPC() || (caster->IsNPC() && !RuleB(Spells, NPCIgnoreBaseImmunity)))))
				{
					if (caster)
						caster->MessageString(Chat::Shout, IMMUNE_STUN);
				}
				else
				{
					// the spinning is handled by the client
					// Stun duration is based on the effect_value, not the buff duration(alot don't have buffs)
					Stun(effect_value);
					if(!IsClient()) {
						Spin();
						spun_timer.Start(100); // spins alittle every 100 ms
					}
				}
				break;
			}

			case SpellEffect::EyeOfZomm:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Eye of Zomm");
#endif
				if(caster && caster->IsClient()) {
					char eye_name[64];
					snprintf(eye_name, sizeof(eye_name), "Eye_of_%s", caster->GetCleanName());
					int duration = CalcBuffDuration(caster, this, spell_id) * 6;
					uint16 eye_id=0;
					caster->TemporaryPets(spell_id, nullptr, eye_name, duration, false, false, &eye_id);
					if (eye_id != 0) {
						caster->CastToClient()->SetControlledMobId(eye_id);
					}
				}
				break;
			}

			case SpellEffect::ReclaimPet:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Reclaim Pet");
#endif
				if
				(
					IsNPC() &&
					GetOwnerID() &&		// I'm a pet
					caster &&					// there's a caster
					caster->GetID() == GetOwnerID()	&& // and it's my master
					GetPetType() != PetType::Charmed
				)
				{
				uint16 pet_spellid =  CastToNPC()->GetPetSpellID();
				uint16 pet_ActSpellCost = caster->GetActSpellCost(pet_spellid, spells[pet_spellid].mana);
				int16 ImprovedReclaimMod =	caster->spellbonuses.ImprovedReclaimEnergy +
											caster->itembonuses.ImprovedReclaimEnergy +
											caster->aabonuses.ImprovedReclaimEnergy;

				if (!ImprovedReclaimMod)
					ImprovedReclaimMod = 75; //Reclaim Energy default is 75% of actual mana cost

				pet_ActSpellCost = pet_ActSpellCost*ImprovedReclaimMod/100;

				caster->SetMana(caster->GetMana() + pet_ActSpellCost);

					if(caster->IsClient())
						caster->CastToClient()->SetPet(0);

					SetOwnerID(0);	// this will kill the pet
				}
				break;
			}

			case SpellEffect::BindSight:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Bind Sight");
#endif
				if(caster && caster->IsClient())
				{
					caster->CastToClient()->SetBindSightTarget(this);
				}
				break;
			}

			case SpellEffect::FeignDeath:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Feign Death");
#endif
				if(spell_id == SPELL_LIFEBURN) //Dook- Lifeburn fix
					break;

				if(IsClient()) {
					CastToClient()->SetHorseId(0); // dismount if have horse

					if (zone->random.Int(0, 99) > spells[spell_id].base_value[i]) {
						SetFeigned(false);
						entity_list.MessageCloseString(this, false, 200, 10, STRING_FEIGNFAILED, GetName());
					} else {
						SetFeigned(true);
					}
				}
				break;
			}

			case SpellEffect::Sentinel:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Sentinel");
#endif
				if(caster)
				{
					if(caster == this)
					{
						MessageString(Chat::Spells,
							SENTINEL_TRIG_YOU);
					}
					else
					{
						caster->MessageString(Chat::Spells,
							SENTINEL_TRIG_OTHER, GetCleanName());
					}
				}
				break;
			}

			case SpellEffect::LocateCorpse:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Locate Corpse");
#endif
				// This is handled by the client prior to SoD.
				//
				if (IsClient() && (CastToClient()->ClientVersionBit() & EQ::versions::maskSoDAndLater))
					CastToClient()->LocateCorpse();

				break;
			}

			case SpellEffect::SummonToCorpse:
			case SpellEffect::Revive:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Revive");	// heh the corpse won't see this
#endif
				if (IsCorpse() && CastToCorpse()->IsPlayerCorpse()) {

					if(caster)
						LogSpells("corpse being rezzed using spell [{}] by [{}]",
							spell_id, caster->GetName());

					CastToCorpse()->CastRezz(spell_id, caster);
				}
				break;
			}

			case SpellEffect::ModelSize:
			case SpellEffect::ChangeHeight:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Model Size: %d%%", effect_value);
#endif
				if (effect_value && effect_value != 100) {
					// Only allow 2 size changes from Base Size
					float modifyAmount = (static_cast<float>(effect_value) / 100.0f);
					float maxModAmount = GetBaseSize() * modifyAmount * modifyAmount;
					if ((GetSize() <= GetBaseSize() && GetSize() > maxModAmount) ||
						(GetSize() >= GetBaseSize() && GetSize() < maxModAmount) ||
						(GetSize() <= GetBaseSize() && maxModAmount > 1.0f) ||
						(GetSize() >= GetBaseSize() && maxModAmount < 1.0f))
					{
						ChangeSize(GetSize() * modifyAmount);
					}
				}
				//Only applies to SPA 89, max value also likely does something, but unknown.
				else if (effect == SpellEffect::ModelSize && spells[spell_id].limit_value[i]) {
					ChangeSize(spells[spell_id].limit_value[i]);
				}

				break;
			}

			case SpellEffect::Root:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Root: %+i", effect_value);
#endif
				rooted = true;

				if (caster && buffslot > -1) {
					buffs[buffslot].RootBreakChance = caster->aabonuses.RootBreakChance +
													caster->itembonuses.RootBreakChance +
													caster->spellbonuses.RootBreakChance;
				}

				break;
			}

			case SpellEffect::SummonHorse:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Summon Mount: %s", spell.teleport_zone);
#endif
				if(IsClient())	// NPCs can't ride
				{
					Client *client = CastToClient();

					// Prevent Feigned players from summoning horses and riding away to freedom.
					client->SetFeigned(false);
					client->Stand();
					client->SummonHorse(spell_id);
				}


				break;
			}

			case SpellEffect::SummonCorpse:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Summon Corpse: %d", effect_value);
#endif
				// can only summon corpses of clients
				if(!IsNPC()) {
					Client* TargetClient = nullptr;
					if(GetTarget())
						TargetClient = GetTarget()->CastToClient();
					else
						TargetClient = CastToClient();

					// We now have a valid target for this spell. Either the caster himself or a targetted player. Lets see if the target is in the group.
					Group* group = entity_list.GetGroupByClient(TargetClient);
					if(group) {
						if(!group->IsGroupMember(TargetClient)) {
							Message(Chat::Red, "Your target must be a group member for this spell.");
							break;
						}
					}
					else if (caster) {
						Raid *r = entity_list.GetRaidByClient(caster->CastToClient());
						if(r)
						{
							uint32 gid = 0xFFFFFFFF;
							gid = r->GetGroup(caster->GetName());
							if(gid < 11)
							{
								if (TargetClient && r->GetGroup(TargetClient->GetName()) != gid) {
									Message(Chat::Red, "Your target must be a group member for this spell.");
									break;
								}
							}
						} else {
							if(TargetClient != CastToClient()) {
								Message(Chat::Red, "Your target must be a group member for this spell.");
								break;
							}
						}
					}

					// Now we should either be casting this on self or its being cast on a valid group member
					if(TargetClient) {

						if (TargetClient->GetLevel() <= effect_value){

							Corpse *corpse = entity_list.GetCorpseByOwner(TargetClient);
							if(corpse) {
								if(TargetClient == CastToClient())
									MessageString(Chat::LightBlue, SUMMONING_CORPSE, TargetClient->CastToMob()->GetCleanName());
								else
									MessageString(Chat::LightBlue, SUMMONING_CORPSE_OTHER, TargetClient->CastToMob()->GetCleanName());

								corpse->Summon(CastToClient(), true, true);
							}
							else {
								// No corpse found in the zone
								MessageString(Chat::LightBlue, CORPSE_CANT_SENSE);
							}
						}
						else if (caster) {
							char level[4];
							ConvertArray(effect_value, level);
							caster->MessageString(Chat::SpellFailure,
								SPELL_LEVEL_REQ, level);
						}
					}
					else {
						MessageString(Chat::LightBlue, TARGET_NOT_FOUND);
						LogError("[{}] attempted to cast spell id [{}] with spell effect SpellEffect::SummonCorpse, but could not cast target into a Client object", GetCleanName(), spell_id);
					}
				}

				break;
			}
			case SpellEffect::SummonCorpseZone:
			{
				if (IsClient()) {
					Client* client_target = CastToClient();
					if (client_target && client_target->IsGrouped()) {
						Group* group = client_target->GetGroup();
						if (!group->IsGroupMember(caster)) {
							if (caster != this) {
								caster->MessageString(Chat::Red, SUMMON_ONLY_GROUP_CORPSE);
								break;
							}
						}
					} else if (caster) {
						if (caster->IsRaidGrouped()) {
							Raid *raid = caster->GetRaid();
							uint32 group_id = raid->GetGroup(caster->GetName());
							if (group_id > 0 && group_id < MAX_RAID_GROUPS) {
								if (client_target && raid->GetGroup(client_target->GetName()) != group_id) {
									caster->MessageString(Chat::Red, SUMMON_ONLY_GROUP_CORPSE);
									break;
								}
							}
						} else {
							if (caster != this) {
								caster->MessageString(Chat::Red, SUMMON_ONLY_GROUP_CORPSE);
								break;
							}
						}
					}

					if (client_target) {
						if (database.CountCharacterCorpses(client_target->CharacterID()) == 0) {
							if (caster == this) {
								Message(Chat::Yellow, "You have no corpses to summon.");
							} else {
								caster->Message(Chat::Yellow, "%s has no corpses to summon.", client_target->GetCleanName());
							}
						} else {
							if (caster == this) {
								Message(Chat::Spells, "Summoning your corpses.");
							} else {
								caster->MessageString(Chat::Spells, SUMMONING_CORPSE_ZONE, client_target->GetCleanName());
							}
							client_target->SummonAllCorpses(client_target->GetPosition());
						}
					} else {
						MessageString(Chat::Spells, TARGET_NOT_FOUND);
						LogError("[{}] attempted to cast spell id [{}] with spell effect SpellEffect::SummonCorpseZone, but could not cast target into a Client object", GetCleanName(), spell_id);
					}
				}
				break;
			}
			case SpellEffect::AddMeleeProc:
			case SpellEffect::WeaponProc:
			{
				uint16 proc_id = GetProcID(spell_id, i);
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Weapon Proc: %s (id %d)", spells[effect_value].name, procid);
#endif
				// Special case for Vampiric Embrace. If this is a Shadow Knight, the proc is different.
				if (proc_id == SPELL_VAMPIRIC_EMBRACE && GetClass() == Class::ShadowKnight) {
					proc_id = SPELL_VAMPIRIC_EMBRACE_OF_SHADOW;
				}

				AddProcToWeapon(proc_id, false, 100 + spells[spell_id].limit_value[i], spell_id, caster_level, GetSpellProcLimitTimer(spell_id, ProcType::MELEE_PROC));
				break;
			}

			case SpellEffect::RangedProc:
			{
				uint16 procid = GetProcID(spell_id, i);
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Ranged Proc: %+i", effect_value);
#endif
				AddRangedProc(procid, 100 + spells[spell_id].limit_value[i], spell_id, GetSpellProcLimitTimer(spell_id, ProcType::RANGED_PROC));
				break;
			}

			case SpellEffect::DefensiveProc:
			{
				uint16 procid = GetProcID(spell_id, i);
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Defensive Proc: %s (id %d)", spells[effect_value].name, procid);
#endif
				AddDefensiveProc(procid, 100 + spells[spell_id].limit_value[i], spell_id, GetSpellProcLimitTimer(spell_id, ProcType::DEFENSIVE_PROC));
				break;
			}

			case SpellEffect::NegateAttacks:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Melee Negate Attack Rune: %+i", effect_value);
#endif
				if(buffslot >= 0)
					buffs[buffslot].hit_number = effect_value;
				break;
			}
			case SpellEffect::AppraiseLDonChest:
			{
				if(IsNPC())
				{
					int check = spell.max_value[0];
					int target = spell.target_type;
					if(target == ST_LDoNChest_Cursed)
					{
						if(caster && caster->IsClient())
						{
							caster->CastToClient()->HandleLDoNSenseTraps(CastToNPC(), check, LDoNTypeCursed);
						}
					}
					else if(target == ST_Target)
					{
						if(caster && caster->IsClient())
						{
							caster->CastToClient()->HandleLDoNSenseTraps(CastToNPC(), check, LDoNTypeMagical);
						}
					}
				}
				break;
			}

			case SpellEffect::DisarmLDoNTrap:
			{
				if(IsNPC())
				{
					int check = spell.max_value[0];
					int target = spell.target_type;
					if(target == ST_LDoNChest_Cursed)
					{
						if(caster && caster->IsClient())
						{
							caster->CastToClient()->HandleLDoNDisarm(CastToNPC(), check, LDoNTypeCursed);
						}
					}
					else if(target == ST_Target)
					{
						if(caster && caster->IsClient())
						{
							caster->CastToClient()->HandleLDoNDisarm(CastToNPC(), check, LDoNTypeMagical);
						}
					}
				}
				break;
			}

			case SpellEffect::UnlockLDoNChest:
			{
				if(IsNPC())
				{
					int check = spell.max_value[0];
					int target = spell.target_type;
					if(target == ST_LDoNChest_Cursed)
					{
						if(caster && caster->IsClient())
						{
							caster->CastToClient()->HandleLDoNPickLock(CastToNPC(), check, LDoNTypeCursed);
						}
					}
					else if(target == ST_Target)
					{
						if(caster && caster->IsClient())
						{
							caster->CastToClient()->HandleLDoNPickLock(CastToNPC(), check, LDoNTypeMagical);
						}
					}
				}
				break;
			}

			case SpellEffect::Lull:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Lull");
#endif
				// TODO: check vs. CHA when harmony effect failed, if caster is to be added to hatelist
				break;
			}

			case SpellEffect::PoisonCounter:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Poison Counter: %+i", effect_value);
#endif
				if (effect_value < 0)
				{
					effect_value = 0 - effect_value;
					int buff_count = GetMaxTotalSlots();
					for (int j=0; j < buff_count; j++) {
						if (!IsValidSpell(buffs[j].spellid))
							continue;
						if (CalculatePoisonCounters(buffs[j].spellid) == 0)
							continue;
						if (effect_value >= static_cast<int>(buffs[j].counters)) {
							if (caster) {
								caster->Message(Chat::Spells,"You have cured your target of %s!",spells[buffs[j].spellid].name);
								caster->CastOnCurer(buffs[j].spellid);
								CastOnCure(buffs[j].spellid);
							}
							effect_value -= buffs[j].counters;
							buffs[j].counters = 0;
							BuffFadeBySlot(j);
						} else {
							buffs[j].counters -= effect_value;
							effect_value = 0;
							break;
						}
					}
				}
				break;
			}

			case SpellEffect::DiseaseCounter:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Disease Counter: %+i", effect_value);
#endif
				if (effect_value < 0)
				{
					effect_value = 0 - effect_value;
					int buff_count = GetMaxTotalSlots();
					for (int j=0; j < buff_count; j++) {
						if (!IsValidSpell(buffs[j].spellid))
							continue;
						if (CalculateDiseaseCounters(buffs[j].spellid) == 0)
							continue;
						if (effect_value >= static_cast<int>(buffs[j].counters))
						{
							if (caster) {
								caster->Message(Chat::Spells,"You have cured your target of %s!",spells[buffs[j].spellid].name);
								caster->CastOnCurer(buffs[j].spellid);
								CastOnCure(buffs[j].spellid);
							}
							effect_value -= buffs[j].counters;
							buffs[j].counters = 0;
							BuffFadeBySlot(j);
						}
						else
						{
							buffs[j].counters -= effect_value;
							effect_value = 0;
							break;
						}
					}
				}
				break;
			}

			case SpellEffect::CurseCounter:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Curse Counter: %+i", effect_value);
#endif
				if (effect_value < 0)
				{
					effect_value = 0 - effect_value;
					int buff_count = GetMaxTotalSlots();
					for (int j=0; j < buff_count; j++) {
						if (!IsValidSpell(buffs[j].spellid))
							continue;
						if (CalculateCurseCounters(buffs[j].spellid) == 0)
							continue;
						if (effect_value >= static_cast<int>(buffs[j].counters))
						{
							if (caster) {
								caster->Message(Chat::Spells,"You have cured your target of %s!",spells[buffs[j].spellid].name);
								caster->CastOnCurer(buffs[j].spellid);
								CastOnCure(buffs[j].spellid);
							}
							effect_value -= buffs[j].counters;
							buffs[j].counters = 0;
							BuffFadeBySlot(j);
						}
						else
						{
							buffs[j].counters -= effect_value;
							effect_value = 0;
							break;
						}
					}
				}
				break;
			}

			case SpellEffect::CorruptionCounter:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Corruption Counter: %+i", effect_value);
#endif
				if (effect_value < 0)
				{
					effect_value = -effect_value;
					int buff_count = GetMaxTotalSlots();
					for (int j=0; j < buff_count; j++) {
						if (!IsValidSpell(buffs[j].spellid))
							continue;
						if (CalculateCorruptionCounters(buffs[j].spellid) == 0)
							continue;
						if (effect_value >= static_cast<int>(buffs[j].counters)) {
							if (caster) {
								caster->Message(Chat::Spells,"You have cured your target of %s!",spells[buffs[j].spellid].name);
								caster->CastOnCurer(buffs[j].spellid);
								CastOnCure(buffs[j].spellid);
							}
							effect_value -= buffs[j].counters;
							buffs[j].counters = 0;
							BuffFadeBySlot(j);
						} else {
							buffs[j].counters -= effect_value;
							effect_value = 0;
							break;
						}
					}
				}
				break;
			}

			case SpellEffect::Destroy:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Destroy");
#endif
				if(IsNPC()) {
					if(GetLevel() <= 52)
						CastToNPC()->Depop(true);
					else
						Message(Chat::Red, "Your target is too high level to be affected by this spell.");
				}
				break;
			}

			case SpellEffect::TossUp:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Toss Up: %d", effect_value);
#endif
				if (IsNPC()) {
					Damage(caster, std::abs(effect_value), spell_id, spell.skill, false, buffslot, false);
				}
				break;
			}

			case SpellEffect::StopRain:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Stop Rain");
#endif
				zone->zone_weather = EQ::constants::WeatherTypes::None;
				zone->weather_intensity = 0;
				zone->weatherSend();
				break;
			}

			case SpellEffect::Sacrifice:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Sacrifice");
#endif
				if(!caster || !IsClient() ){
					break;
				}
				CastToClient()->SacrificeConfirm(caster);
				break;
			}

			case SpellEffect::SummonPC:
			{
				if (!caster) {
					break;
				}

				if (IsClient()) {
					if (caster->IsClient()) {
						if (!entity_list.IsInSameGroupOrRaidGroup(caster->CastToClient(), CastToClient())) {
							caster->MessageString(Chat::SpellFailure, TARGET_GROUP_MEMBER);
							break;
						}

						// clear aggro when summoned in zone and further than aggro clear distance rule.
						if (RuleR(Spells, CallOfTheHeroAggroClearDist) == 0 || caster->CalculateDistance(GetX(), GetY(), GetZ()) >= RuleR(Spells, CallOfTheHeroAggroClearDist)) {
							if (RuleB(Spells, EvacClearCharmPet)) {
								entity_list.ClearAggro(this, true);
							} else {
								entity_list.ClearAggro(this);
							}
						}
					} else if (!RuleB(Combat, SummonMeleeRange) && caster->GetZoneID() == GetZoneID() && caster->CombatRange(this)) {
						break;
					}

					CastToClient()->MovePC(zone->GetZoneID(), zone->GetInstanceID(), caster->GetX(),
							       caster->GetY(), caster->GetZ(), caster->GetHeading(), 2,
							       SummonPC);
					MessageString(Chat::Spells, PLAYER_SUMMONED);
				} else {
					caster->Message(Chat::Red, "This spell can only be cast on players.");
				}
				break;
			}

			case SpellEffect::Silence:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Silence");
#endif
				Silence(true);
				break;
			}

			case SpellEffect::Amnesia:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Amnesia");
#endif
				Amnesia(true);
				break;
			}

			case SpellEffect::CallPet:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Call Pet");
#endif
				// this is cast on self, not on the pet
				Mob *casters_pet = GetPet();
				if(casters_pet && casters_pet->IsNPC()){
					casters_pet->CastToNPC()->GMMove(GetX(), GetY(), GetZ(), GetHeading());
					if (!casters_pet->GetTarget()) {
						casters_pet->StopNavigation();
					}
				}
				break;
			}

			case SpellEffect::StackingCommand_Block:
			case SpellEffect::StackingCommand_Overwrite:
			{
				// these are special effects used by the buff stuff
				break;
			}


			case SpellEffect::TemporaryPets: //Dook- swarms and wards:
			{
				if (!caster)
					break;
				// this makes necro epic 1.5/2.0 proc work properly
				if((spell_id != 6882) && (spell_id != 6884)) // Chaotic Jester/Steadfast Servant
				{
					char pet_name[64];
					snprintf(pet_name, sizeof(pet_name), "%s`s_pet", caster->GetCleanName());
					caster->TemporaryPets(spell_id, this, pet_name);
				}
				else
					caster->TemporaryPets(spell_id, this, nullptr);
				break;
			}

			case SpellEffect::FadingMemories:		//Dook- escape etc
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Fading Memories");
#endif
				int max_level = 0;

				if (RuleB(Spells, UseFadingMemoriesMaxLevel)) {
					//handle ROF2 era where limit value determines max level
					if (spells[spell_id].limit_value[i]) {
						max_level = spells[spell_id].limit_value[i];
					}
					//handle modern client era where max value determines max level or range above client.
					else if (spells[spell_id].max_value[i]) {
						if (spells[spell_id].max_value[i] >= 1000) {
							max_level = 1000 - spells[spell_id].max_value[i];
						} else {
							max_level = GetLevel() + spells[spell_id].max_value[i];
						}
					}
				}

				if(zone->random.Roll(spells[spell_id].base_value[i])) {
					if (IsClient()) {
						int pre_aggro_count = CastToClient()->GetAggroCount();
						entity_list.RemoveFromTargetsFadingMemories(this, true, max_level);
						entity_list.ClearZoneFeignAggro(this);

						if (spellbonuses.ShroudofStealth || aabonuses.ShroudofStealth || itembonuses.ShroudofStealth) {
							improved_hidden = true;
							hidden = true;
						}

						SetInvisible(Invisibility::Invisible);
						int post_aggro_count = CastToClient()->GetAggroCount();
						if (RuleB(Spells, UseFadingMemoriesMaxLevel)) {
							if (pre_aggro_count == post_aggro_count) {
								Message(Chat::SpellFailure, "You failed to escape from all your opponents.");
								break;
							} else if (post_aggro_count) {
								Message(Chat::SpellFailure, "You failed to escape from combat but you evade some of your opponents.");
								break;
							}
						}
						MessageString(Chat::Skills, ESCAPE);
					} else{
						entity_list.RemoveFromTargets(caster);
						SetInvisible(Invisibility::Invisible);
					}
				}
				break;
			}

			case SpellEffect::Rampage:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Rampage");
#endif
				//defulat live range is 40, with 1 attack per round, no hit count limit
				float rampage_range = 40;
				if (spells[spell_id].aoe_range) {
					rampage_range = spells[spell_id].aoe_range; //added for expanded functionality
				}
				int attack_count = spells[spell_id].base_value[i]; //added for expanded functionality
				int hit_count = spells[spell_id].limit_value[i]; //added for expanded functionality
				if (caster) {
					entity_list.AEAttack(caster, rampage_range, EQ::invslot::slotPrimary, hit_count, true, attack_count); // on live wars dont get a duration ramp, its a one shot deal
				}
				break;
			}

			case SpellEffect::AEMelee:
			{
				//old aa

#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Duration Rampage");
#endif
				aa_timers[aaTimerRampage].Start(effect_value * 10 * 1000); // Live bug, was suppose to be 1 second per value
				break;
			}

			case SpellEffect::AETaunt:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "AE Taunt");
#endif
				if(caster && caster->IsClient()){
					//Live AE Taunt range is hardcoded at 40 (Spells for AE taunt all use zero range) Target type should be self only.
					float range = 40;
					if (spells[spell_id].max_value[i])//custom support if you want to alter range of AE Taunt.
						range = spells[spell_id].max_value[i];

					entity_list.AETaunt(caster->CastToClient(), range, spells[spell_id].base_value[i]);
				}
				break;
			}

			case SpellEffect::SkillAttack:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Skill Attack");
#endif
				/*
				Weapon Damage = spells[spell_id].base[i]
				Chance to Hit Bonus = spells[spell_id].base2[i]
				???? = spells[spell_id].max[i] - MOST of the effects have this value.
				*Max is lower value then Weapon base, possibly min hit vs Weapon Damage range ie. MakeRandInt(max,base)
				*/
				int ReuseTime = spells[spell_id].recast_time + spells[spell_id].recovery_time;
				if (!caster) {
					break;
				}

				int16 focus = RuleB(Spells, AllowFocusOnSkillDamageSpells) ? caster->GetMeleeDamageMod_SE(spells[spell_id].skill) : 0;

				switch(spells[spell_id].skill) {
					case EQ::skills::SkillThrowing:
						caster->DoThrowingAttackDmg(this, nullptr, nullptr, spells[spell_id].base_value[i],spells[spell_id].limit_value[i], focus, ReuseTime, 0, 0, 4.0f, true);
						break;
					case EQ::skills::SkillArchery:
						caster->DoArcheryAttackDmg(this, nullptr, nullptr, spells[spell_id].base_value[i],spells[spell_id].limit_value[i], focus, ReuseTime, 0, 0, nullptr, 0, 4.0f, true);
						break;
					default:
						caster->DoMeleeSkillAttackDmg(this, spells[spell_id].base_value[i], spells[spell_id].skill, spells[spell_id].limit_value[i], focus, false, ReuseTime);
						break;
				}
				break;
			}

			case SpellEffect::WakeTheDead:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Wake The Dead");
#endif
				//meh dupe issue with npc casting this
				if(caster && caster->IsClient()){
					int dur = spells[spell_id].max_value[i];
					if (!dur) {
						dur = 60;
					}

					Mob* m_target = caster->GetTarget();
					if (m_target) {
						entity_list.TryWakeTheDead(caster, m_target, spell_id, 250, dur, 1);
					}
				}
				break;
			}

			case SpellEffect::ArmyOfTheDead:
			{
				if (caster && caster->IsClient()) {
					int dur = spells[spell_id].max_value[i];
					if (!dur) {
						dur = 60;
					}

					int amount = spells[spell_id].base_value[i];
					if (!amount) {
						amount = 1;
					}

					Mob* m_target = caster->GetTarget();
					if (m_target) {
						entity_list.TryWakeTheDead(caster, m_target, spell_id, 250, dur, amount);
					}
				}
				break;
			}

			case SpellEffect::Doppelganger:
			{
				if(caster && caster->IsClient()) {
					char pet_name[64];
					snprintf(pet_name, sizeof(pet_name), "%s`s_doppelganger", caster->GetCleanName());
					int pet_count = spells[spell_id].base_value[i];
					int pet_duration = spells[spell_id].max_value[i];
					caster->CastToClient()->Doppelganger(spell_id, this, pet_name, pet_count, pet_duration);
				}
				break;
			}

			case SpellEffect::BardAEDot:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Bard AE Dot: %+i", effect_value);
#endif
				// SpellEffect::CurrentHP is calculated at first tick if its a dot/buff
				if (buffslot >= 0)
					break;
				//This effect does no damage if target is moving.
				if (!RuleB(Spells, PreNerfBardAEDoT) && IsMoving())
					break;

				// for offensive spells check if we have a spell rune on
				int64 dmg = effect_value;
				if(dmg < 0)
				{
					// take partial damage into account
					dmg = (int64) (dmg * partial / 100);

					//handles AAs and what not...
					//need a bard version of this prolly...
					//if(caster)
					//	dmg = caster->GetActSpellDamage(spell_id, dmg);

					dmg = -dmg;
					Damage(caster, dmg, spell_id, spell.skill, false, buffslot, false);
				} else if(dmg > 0) {
					//healing spell...
					if(caster)
						dmg = caster->GetActSpellHealing(spell_id, dmg, this);
					HealDamage(dmg, caster);
				}
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Current Hitpoints: %+i  actual: %+i", effect_value, dmg);
#endif
				break;
			}

			case SpellEffect::CurrentEndurance: {
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Current Endurance: %+i", effect_value);
#endif
				if(IsClient()) {
					CastToClient()->SetEndurance(CastToClient()->GetEndurance() + effect_value);
					if (effect_value < 0) {
						TryTriggerOnCastRequirement();
					}
				}
				break;
			}

			case SpellEffect::CurrentEnduranceOnce:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Current Endurance Once: %+i", effect_value);
#endif

				if (IsClient()) {
					CastToClient()->SetEndurance(CastToClient()->GetEndurance() + effect_value);
					if (effect_value < 0) {
						TryTriggerOnCastRequirement();
					}
				}
				break;
			}

			case SpellEffect::BalanceHP: {
				if(!caster)
					break;

				if(!caster->IsClient())
					break;

				Raid *r = entity_list.GetRaidByClient(caster->CastToClient());
				if(r)
				{
					uint32 gid = 0xFFFFFFFF;
					gid = r->GetGroup(caster->GetName());
					if(gid < 11)
					{
						r->BalanceHP(spell.base_value[i], gid, spell.range, caster, spell.limit_value[i]);
						break;
					}
				}

				Group *g = entity_list.GetGroupByClient(caster->CastToClient());

				if(!g)
					break;

				g->BalanceHP(spell.base_value[i], spell.range, caster, spell.limit_value[i]);
				break;
			}

			case SpellEffect::BalanceMana: {
				if(!caster)
					break;

				if(!caster->IsClient())
					break;

				Raid *r = entity_list.GetRaidByClient(caster->CastToClient());
				if(r)
				{
					uint32 gid = 0xFFFFFFFF;
					gid = r->GetGroup(caster->GetName());
					if(gid < 11)
					{
						r->BalanceMana(spell.base_value[i], gid, spell.range, caster, spell.limit_value[i]);
						break;
					}
				}

				Group *g = entity_list.GetGroupByClient(caster->CastToClient());

				if(!g)
					break;

				g->BalanceMana(spell.base_value[i], spell.range, caster, spell.limit_value[i]);
				break;
			}

			case SpellEffect::SummonAndResAllCorpses:
			{
				if(IsClient())
					CastToClient()->SummonAndRezzAllCorpses();

				break;
			}

			case SpellEffect::GateToHomeCity:
			{
				if(IsClient())
					CastToClient()->GoToBind(4);
				break;
			}

			case SpellEffect::SuspendPet:
			{
				if(IsClient())
					CastToClient()->SuspendMinion(spell.base_value[i]);

				break;
			}

			case SpellEffect::FcTimerRefresh:
			{
				if(IsClient()) {
					for (unsigned int i = 0; i < EQ::spells::SPELL_GEM_COUNT; ++i) {
						if(IsValidSpell(CastToClient()->m_pp.mem_spells[i])) {
							if (CalcFocusEffect(focusFcTimerRefresh, spell_id, CastToClient()->m_pp.mem_spells[i])){
								CastToClient()->m_pp.spellSlotRefresh[i] = 1;
								CastToClient()->GetPTimers().Clear(&database, (pTimerSpellStart + CastToClient()->m_pp.mem_spells[i]));
								if (!CastToClient()->IsLinkedSpellReuseTimerReady(spells[CastToClient()->m_pp.mem_spells[i]].timer_id)) {
									CastToClient()->GetPTimers().Clear(&database, (pTimerLinkedSpellReuseStart + spells[CastToClient()->m_pp.mem_spells[i]].timer_id));
								}
							}
						}
					}
					SetMana(GetMana());
				}
				break;
			}

			case SpellEffect::FcTimerLockout: {
				if (IsClient()) {
					for (unsigned int mem_spell : CastToClient()->m_pp.mem_spells) {
						if (IsValidSpell(mem_spell)) {
							int32 new_recast_timer = CalcFocusEffect(
								focusFcTimerLockout,
								spell_id,
								mem_spell
							);
							if (new_recast_timer) {
								bool apply_recast_timer = true;
								if (IsCasting() && casting_spell_id == mem_spell) {
									apply_recast_timer = false;
								}
								if (apply_recast_timer) {
									new_recast_timer = new_recast_timer / 1000;
									CastToClient()->GetPTimers().Start(
										pTimerSpellStart + mem_spell,
										static_cast<uint32>(new_recast_timer)
									);
								}
							}
						}
					}
					SetMana(GetMana());
				}
				break;
			}

			case SpellEffect::HealGroupFromMana: {
				if(!caster)
					break;

				if(!caster->IsClient())
					break;

				int32 max_mana = spell.base_value[i];
				int ratio = spell.limit_value[i];
				uint64 heal_amt = 0;

				if (caster->GetMana() <= max_mana){
					heal_amt = ratio*caster->GetMana()/10;
					caster->SetMana(0);
				}

				else {
					heal_amt = ratio*max_mana/10;
					caster->SetMana(caster->GetMana() - max_mana);
				}

				Raid *r = entity_list.GetRaidByClient(caster->CastToClient());
				if(r)
				{
					uint32 gid = 0xFFFFFFFF;
					gid = r->GetGroup(caster->GetName());
					if(gid < 11)
					{
						r->HealGroup(heal_amt,caster, gid, spell.range);
						break;
					}
				}

				Group *g = entity_list.GetGroupByClient(caster->CastToClient());

				if(!g){
					caster->HealDamage(heal_amt);
					break;
				}

				g->HealGroup(heal_amt, caster, spell.range);
				break;
			}

			case SpellEffect::ManaDrainWithDmg:
			{
				int mana_damage = 0;
				int32 mana_to_use = GetMana() - spell.base_value[i];
				if(mana_to_use > -1) {
					SetMana(GetMana() - spell.base_value[i]);
					TryTriggerOnCastRequirement();
					// we take full dmg(-10 to make the damage the right sign)
					mana_damage = spell.base_value[i] / -10 * spell.limit_value[i];
					Damage(caster, mana_damage, spell_id, spell.skill, false, i, true);
				}
				else {
					mana_damage = GetMana() / -10 * spell.limit_value[i];
					SetMana(0);
					Damage(caster, mana_damage, spell_id, spell.skill, false, i, true);
				}
				break;
			}

			case SpellEffect::EndDrainWithDmg:
			{
				if(IsClient()) {
					int end_damage = 0;
					int32 end_to_use = CastToClient()->GetEndurance() - spell.base_value[i];
					if(end_to_use > -1) {
						CastToClient()->SetEndurance(CastToClient()->GetEndurance() - spell.base_value[i]);
						TryTriggerOnCastRequirement();
						// we take full dmg(-10 to make the damage the right sign)
						end_damage = spell.base_value[i] / -10 * spell.limit_value[i];
						Damage(caster, end_damage, spell_id, spell.skill, false, i, true);
					}
					else {
						end_damage = CastToClient()->GetEndurance() / -10 * spell.limit_value[i];
						CastToClient()->SetEndurance(0);
						Damage(caster, end_damage, spell_id, spell.skill, false, i, true);
					}
				}
				break;
			}

			case SpellEffect::SetBodyType:
			{
				SetBodyType(spell.base_value[i], false);
				break;
			}

			case SpellEffect::Leap:
			{
				// These effects remove lev and only work a certain distance away.
				BuffFadeByEffect(SpellEffect::Levitate);
				if (caster && caster->GetTarget()) {
					float my_x = caster->GetX();
					float my_y = caster->GetY();
					float my_z = caster->GetZ();
					float target_x = GetX();
					float target_y = GetY();
					float target_z = GetZ();
					if ((CalculateDistance(my_x, my_y, my_z) > 10) &&
						(CalculateDistance(my_x, my_y, my_z) < 75) &&
						(caster->CheckLosFN(caster->GetTarget())))
					{
						float value, x_vector, y_vector, hypot;

						value = (float)spell.base_value[i]; // distance away from target

						x_vector = target_x - my_x;
						y_vector = target_y - my_y;
						hypot = sqrt(x_vector*x_vector + y_vector*y_vector);

						x_vector /= hypot;
						y_vector /= hypot;

						my_x = target_x - (x_vector * value);
						my_y = target_y - (y_vector * value);

						float new_ground = GetGroundZ(my_x, my_y);

						if(caster->IsClient())
							caster->CastToClient()->MovePC(zone->GetZoneID(), zone->GetInstanceID(), my_x, my_y, new_ground, GetHeading());
						else
							caster->GMMove(my_x, my_y, new_ground, GetHeading());
					}
				}
				break;
			}
			case SpellEffect::VoiceGraft:
			{
				if(caster && caster->GetPet())
					caster->spellbonuses.VoiceGraft = caster->GetPetID();

				break;
			}

			case SpellEffect::ManaBurn:
			{
				int32 max_mana = spell.base_value[i];
				int ratio = spell.limit_value[i];
				int64 dmg = 0;

				if (caster){
					if (caster->GetMana() <= max_mana){
							dmg = ratio*caster->GetMana()/10;
							caster->SetMana(0);
					}

					else {
						dmg = ratio*max_mana/10;
						caster->SetMana(caster->GetMana() - max_mana);
						TryTriggerOnCastRequirement();
					}

					if(IsDetrimentalSpell(spell_id)) {
						dmg = -dmg;
						Damage(caster, dmg, spell_id, spell.skill, false, buffslot, false);
					} else {
						HealDamage(dmg, caster);
					}
				}

				break;
			}

			case SpellEffect::Taunt:
			{
				if (caster && IsNPC()){
					caster->Taunt(CastToNPC(), false, spell.base_value[i], true, spell.limit_value[i]);
				}
				break;
			}

			case SpellEffect::AttackSpeed:
				if (spell.base_value[i] < 100)
					SlowMitigation(caster);
				break;

			case SpellEffect::AttackSpeed2:
				if (spell.base_value[i] < 100)
					SlowMitigation(caster);
				break;

			case SpellEffect::AttackSpeed3:
				if (spell.base_value[i] < 0)
					SlowMitigation(caster);
				break;

			case SpellEffect::AttackSpeed4:
				SlowMitigation(caster);
				break;

			case SpellEffect::AddHatePct:
			{
				if (IsNPC()){
					int64 new_hate = CastToNPC()->GetHateAmount(caster) * (100 + spell.base_value[i]) / 100;
					if (new_hate <= 0)
						new_hate = 1;

					CastToNPC()->SetHateAmountOnEnt(caster, new_hate);
				}
				break;
			}

			case SpellEffect::Hate:{

				if (buffslot >= 0)
					break;

				if(caster){
					if(effect_value > 0){
						if(caster){
							if(caster->IsClient() && !caster->CastToClient()->GetFeigned())
								AddToHateList(caster, effect_value);
							else if(!caster->IsClient())
								AddToHateList(caster, effect_value);
						}
					}else{
						int64 newhate = GetHateAmount(caster) + effect_value;
						if (newhate < 1)
							SetHateAmountOnEnt(caster,1);
						else
							SetHateAmountOnEnt(caster,newhate);
						}
				}
				break;
			}

			case SpellEffect::InterruptCasting:{
				if (buffslot >= 0)
					break;

				if(!spells[spell_id].uninterruptable && IsCasting() && zone->random.Roll(spells[spell_id].base_value[i]))
					InterruptSpell();

				break;
			}

			case SpellEffect::MassGroupBuff:{

				SetMGB(true);
				MessageString(Chat::Disciplines, MGB_STRING);
				break;
			}

			case SpellEffect::IllusionOther: {
				SetProjectIllusion(true);
				Message(Chat::NPCQuestSay, "The power of your next illusion spell will flow to your grouped target in your place.");
				break;
			}

			case SpellEffect::ApplyEffect: {

				if (caster && IsValidSpell(spells[spell_id].limit_value[i])){
					if(zone->random.Roll(spells[spell_id].base_value[i]))
						caster->SpellFinished(spells[spell_id].limit_value[i], this, EQ::spells::CastingSlot::Item, 0, -1, spells[spells[spell_id].limit_value[i]].resist_difficulty);
				}
				break;
			}

			case SpellEffect::SpellTrigger: {

				if (!spell_trigger_cast_complete) {
					if (caster && caster->TrySpellTrigger(this, spell_id, i))
						spell_trigger_cast_complete = true;
				}
				break;
			}


			case SpellEffect::Instant_Mana_Pct: {
				effect_value = spells[spell_id].base_value[i];
				int64 amt = std::abs(GetMaxMana() * effect_value / 10000);
				if (spells[spell_id].max_value[i] && amt > spells[spell_id].max_value[i])
					amt = spells[spell_id].max_value[i];

				if (effect_value < 0) {
					SetMana(GetMana() - amt);
				}
				else {
					SetMana(GetMana() + amt);
				}
				break;
			}

			case SpellEffect::Instant_Endurance_Pct: {
				effect_value = spells[spell_id].base_value[i];
				if (IsClient()) {
					int32 amt = std::abs(CastToClient()->GetMaxEndurance() * effect_value / 10000);
					if (spells[spell_id].max_value[i] && amt > spells[spell_id].max_value[i])
						amt = spells[spell_id].max_value[i];

					if (effect_value < 0) {
						CastToClient()->SetEndurance(CastToClient()->GetEndurance() - amt);
					}
					else {
						CastToClient()->SetEndurance(CastToClient()->GetEndurance() + amt);
					}
				}
				break;
			}
			/*
				Calc for base1 is found in ApplyHealthTransferDamage() due to needing to account for AOE functionality
				since effect can potentially kill caster.
			*/
			case SpellEffect::Health_Transfer: {
				if (caster) {
					effect_value = spells[spell_id].limit_value[i];
					int64 amt = std::abs(caster->GetMaxHP() * effect_value / 1000);

					if (effect_value < 0) {
						Damage(caster, amt, spell_id, spell.skill, false, buffslot, false);
					}
					else {
						HealDamage(amt, caster);
					}
				}
				break;
			}

			case SpellEffect::Chance_Best_in_Spell_Grp: {
				if (!spell_trigger_cast_complete) {
					if (caster && caster->TrySpellTrigger(this, spell_id, i))
						spell_trigger_cast_complete = true;
				}
				break;
			}

			case SpellEffect::Trigger_Best_in_Spell_Grp: {

				if (caster && !caster->IsClient())
					break;

				if (caster && zone->random.Roll(spells[spell_id].base_value[i])) {
					uint32 best_spell_id = caster->CastToClient()->GetHighestScribedSpellinSpellGroup(spells[spell_id].limit_value[i]);

					if (IsValidSpell(best_spell_id))
						caster->SpellFinished(best_spell_id, this, EQ::spells::CastingSlot::Item, 0, -1, spells[best_spell_id].resist_difficulty);
				}
				break;
			}

			case SpellEffect::Trigger_Spell_Non_Item: {
				//Only trigger if not from item
				if (caster && caster->IsClient() && GetCastedSpellInvSlot() > 0)
					break;

				if (zone->random.Roll(spells[spell_id].base_value[i]) && IsValidSpell(spells[spell_id].limit_value[i]))
						caster->SpellFinished(spells[spell_id].limit_value[i], this, EQ::spells::CastingSlot::Item, 0, -1, spells[spells[spell_id].limit_value[i]].resist_difficulty);

				break;
			}

			case SpellEffect::Hatelist_To_Tail_Index: {
				if (caster && zone->random.Roll(spells[spell_id].base_value[i]))
					caster->SetBottomRampageList();
				break;
			}

			case SpellEffect::Hatelist_To_Top_Index: {
				if (caster && zone->random.Roll(spells[spell_id].base_value[i]))
					caster->SetTopRampageList();
				break;
			}

			case SpellEffect::Fearstun: {
				//Normal 'stun' restrictions do not apply. base1=duration, base2=PC duration, max =lv restrict
				if (!caster)
					break;

				if (IsNPC() && GetSpecialAbility(SpecialAbility::StunImmunity)) {
					caster->MessageString(Chat::SpellFailure, IMMUNE_STUN);
					break;
				}

				if (IsNPC() && GetSpecialAbility(SpecialAbility::FearImmunity)) {
					caster->MessageString(Chat::SpellFailure, IMMUNE_FEAR);
					break;
				}
				int max_level = 0;
				if (spells[spell_id].max_value[i] >= 1000) {
					max_level = spells[spell_id].max_value[i] - 1000;
				}
				else {
					max_level = caster->GetLevel() + spells[spell_id].max_value[i];
				}

				if (spells[spell_id].max_value[i] == 0 || GetLevel() <= max_level) {
					if (IsClient() && spells[spell_id].limit_value[i])
						Stun(spells[spell_id].limit_value[i]);
					else
						Stun(spells[spell_id].base_value[i]);
				}
				else
					caster->MessageString(Chat::SpellFailure, FEAR_TOO_HIGH);
				break;
			}

			case SpellEffect::PetShield: {
				if (IsPet()) {
					Mob* petowner = GetOwner();
					if (petowner) {
						int shield_duration          = spells[spell_id].base_value[i] * 12 * 1000;
						int shield_target_mitigation = spells[spell_id].limit_value[i] ? spells[spell_id].limit_value[i] : 50;
						int shielder_mitigation      = spells[spell_id].max_value[i] ? spells[spell_id].limit_value[i] : 50;
						ShieldAbility(petowner->GetID(), 25, shield_duration, shield_target_mitigation, shielder_mitigation);
					}
				}
				break;
			}

			case SpellEffect::Weapon_Stance: {
				if (IsClient()) {
					CastToClient()->ApplyWeaponsStance();
				}
				break;
			}

			case SpellEffect::HealOverTime: {
				//This is here so buffs with hit counters tic down on initial cast.
				caster->GetActSpellHealing(spell_id, effect_value, nullptr, false);
				break;
			}

			case SpellEffect::PersistentEffect:
				MakeAura(spell_id);
				break;

			// Handled Elsewhere
			case SpellEffect::ImmuneFleeing:
			case SpellEffect::NegateSpellEffect:
			case SpellEffect::Knockdown: // handled by client
			case SpellEffect::ShadowStepDirectional: // handled by client
			case SpellEffect::SpellOnDeath:
			case SpellEffect::BlockNextSpellFocus:
			case SpellEffect::ReduceReuseTimer:
			case SpellEffect::SwarmPetDuration:
			case SpellEffect::LimitHPPercent:
			case SpellEffect::LimitManaPercent:
			case SpellEffect::LimitEndPercent:
			case SpellEffect::ExtraAttackChance:
			case SpellEffect::ProcChance:
			case SpellEffect::StunResist:
			case SpellEffect::MinDamageModifier:
			case SpellEffect::DamageModifier:
			case SpellEffect::DamageModifier2:
			case SpellEffect::HitChance:
			case SpellEffect::MeleeSkillCheck:
			case SpellEffect::HundredHands:
			case SpellEffect::ResistFearChance:
			case SpellEffect::ResistSpellChance:
			case SpellEffect::AllInstrumentMod:
			case SpellEffect::MeleeLifetap:
			case SpellEffect::DoubleAttackChance:
			case SpellEffect::TripleAttackChance:
			case SpellEffect::DualWieldChance:
			case SpellEffect::ParryChance:
			case SpellEffect::DodgeChance:
			case SpellEffect::RiposteChance:
			case SpellEffect::AvoidMeleeChance:
			case SpellEffect::CrippBlowChance:
			case SpellEffect::CriticalHitChance:
			case SpellEffect::MeleeMitigation:
			case SpellEffect::Reflect:
			case SpellEffect::Screech:
			case SpellEffect::Amplification:
			case SpellEffect::MagicWeapon:
			case SpellEffect::Hunger:
			case SpellEffect::MagnifyVision:
			case SpellEffect::Lycanthropy:
			case SpellEffect::NegateIfCombat:
			case SpellEffect::CastingLevel:
			case SpellEffect::CastingLevel2:
			case SpellEffect::RaiseStatCap:
			case SpellEffect::ResistAll:
			case SpellEffect::ResistMagic:
			case SpellEffect::ResistDisease:
			case SpellEffect::ResistPoison:
			case SpellEffect::ResistCold:
			case SpellEffect::ResistFire:
			case SpellEffect::AllStats:
			case SpellEffect::MakeDrunk:
			case SpellEffect::CHA:
			case SpellEffect::WIS:
			case SpellEffect::INT:
			case SpellEffect::STA:
			case SpellEffect::AGI:
			case SpellEffect::DEX:
			case SpellEffect::STR:
			case SpellEffect::ATK:
			case SpellEffect::ArmorClass:
			case SpellEffect::EndurancePool:
			case SpellEffect::Stamina:
			case SpellEffect::UltraVision:
			case SpellEffect::InfraVision:
			case SpellEffect::ManaPool:
			case SpellEffect::TotalHP:
			case SpellEffect::ChangeFrenzyRad:
			case SpellEffect::Harmony:
			case SpellEffect::ChangeAggro:
			case SpellEffect::Identify:
			case SpellEffect::InstantHate:
			case SpellEffect::ReduceHate:
			case SpellEffect::SpellDamageShield:
			case SpellEffect::ReverseDS:
			case SpellEffect::DamageShield:
			case SpellEffect::TrueNorth:
			case SpellEffect::WaterBreathing:
			case SpellEffect::MovementSpeed:
			case SpellEffect::PercentXPIncrease:
			case SpellEffect::DivineSave:
			case SpellEffect::Accuracy:
			case SpellEffect::Flurry:
			case SpellEffect::ImprovedDamage:
			case SpellEffect::ImprovedDamage2:
			case SpellEffect::ImprovedHeal:
			case SpellEffect::IncreaseSpellHaste:
			case SpellEffect::IncreaseSpellDuration:
			case SpellEffect::IncreaseRange:
			case SpellEffect::SpellHateMod:
			case SpellEffect::ReduceReagentCost:
			case SpellEffect::ReduceManaCost:
			case SpellEffect::LimitMaxLevel:
			case SpellEffect::LimitResist:
			case SpellEffect::LimitTarget:
			case SpellEffect::LimitEffect:
			case SpellEffect::LimitSpellType:
			case SpellEffect::LimitSpell:
			case SpellEffect::LimitMinDur:
			case SpellEffect::LimitInstant:
			case SpellEffect::LimitMinLevel:
			case SpellEffect::LimitCastTimeMin:
			case SpellEffect::LimitManaMin:
			case SpellEffect::LimitCombatSkills:
			case SpellEffect::SpellDurationIncByTic:
			case SpellEffect::TriggerOnCast:
			case SpellEffect::HealRate:
			case SpellEffect::SkillDamageTaken:
			case SpellEffect::FcSpellVulnerability:
			case SpellEffect::Fc_Spell_Damage_Pct_IncomingPC:
			case SpellEffect::Fc_Spell_Damage_Amt_IncomingPC:
			case SpellEffect::FcTwincast:
			case SpellEffect::DelayDeath:
			case SpellEffect::CastOnFadeEffect:
			case SpellEffect::CastOnFadeEffectNPC:
			case SpellEffect::CastOnFadeEffectAlways:
			case SpellEffect::CastOnRuneFadeEffect:
			case SpellEffect::MaxHPChange:
			case SpellEffect::SympatheticProc:
			case SpellEffect::FcDamageAmt:
			case SpellEffect::FcDamageAmt2:
			case SpellEffect::CriticalSpellChance:
			case SpellEffect::SpellCritChance:
			case SpellEffect::SpellCritDmgIncrease:
			case SpellEffect::DotCritDmgIncrease:
			case SpellEffect::CriticalHealChance:
			case SpellEffect::CriticalHealOverTime:
			case SpellEffect::CriticalDoTChance:
			case SpellEffect::ProcOnKillShot:
			case SpellEffect::ProcOnSpellKillShot:
			case SpellEffect::CriticalDamageMob:
			case SpellEffect::LimitSpellGroup:
			case SpellEffect::ResistCorruption:
			case SpellEffect::ReduceSkillTimer:
			case SpellEffect::HPToMana:
			case SpellEffect::ManaAbsorbPercentDamage:
			case SpellEffect::SkillDamageAmount:
			case SpellEffect::SkillDamageAmount2:
			case SpellEffect::GravityEffect:
			case SpellEffect::IncreaseBlockChance:
			case SpellEffect::AntiGate:
			case SpellEffect::Fearless:
			case SpellEffect::FcDamageAmtCrit:
			case SpellEffect::FcHealAmtCrit:
			case SpellEffect::CastOnCurer:
			case SpellEffect::CastOnCure:
			case SpellEffect::CastonNumHitFade:
			case SpellEffect::LimitToSkill:
			case SpellEffect::SpellProcChance:
			case SpellEffect::CharmBreakChance:
			case SpellEffect::BardSongRange:
			case SpellEffect::ACv2:
			case SpellEffect::ManaRegen_v2:
			case SpellEffect::FcDamagePctCrit:
			case SpellEffect::FcHealAmt:
			case SpellEffect::FcHealPctIncoming:
			case SpellEffect::CriticalHealDecay:
			case SpellEffect::CriticalRegenDecay:
			case SpellEffect::FcDamageAmtIncoming:
			case SpellEffect::LimitCastingSkill:
			case SpellEffect::MitigateDamageShield:
			case SpellEffect::FcBaseEffects:
			case SpellEffect::LimitClass:
			case SpellEffect::BlockBehind:
			case SpellEffect::ShieldBlock:
			case SpellEffect::PetCriticalHit:
			case SpellEffect::SlayUndead:
			case SpellEffect::GiveDoubleAttack:
			case SpellEffect::StrikeThrough:
			case SpellEffect::StrikeThrough2:
			case SpellEffect::SecondaryDmgInc:
			case SpellEffect::ArcheryDamageModifier:
			case SpellEffect::ConsumeProjectile:
			case SpellEffect::ForageAdditionalItems:
			case SpellEffect::Salvage:
			case SpellEffect::FrontalBackstabChance:
			case SpellEffect::FrontalBackstabMinDmg:
			case SpellEffect::TripleBackstab:
			case SpellEffect::DoubleSpecialAttack:
			case SpellEffect::IncreaseRunSpeedCap:
			case SpellEffect::BaseMovementSpeed:
			case SpellEffect::FrontalStunResist:
			case SpellEffect::ImprovedBindWound:
			case SpellEffect::MaxBindWound:
			case SpellEffect::CombatStability:
			case SpellEffect::AddSingingMod:
			case SpellEffect::SongModCap:
			case SpellEffect::HeadShot:
			case SpellEffect::HeadShotLevel:
			case SpellEffect::PetAvoidance:
			case SpellEffect::GiveDoubleRiposte:
			case SpellEffect::Ambidexterity:
			case SpellEffect::PetMaxHP:
			case SpellEffect::PetFlurry:
			case SpellEffect::MasteryofPast:
			case SpellEffect::GivePetGroupTarget:
			case SpellEffect::RootBreakChance:
			case SpellEffect::UnfailingDivinity:
			case SpellEffect::ChannelChanceSpells:
			case SpellEffect::ChannelChanceItems:
			case SpellEffect::FcHealPctCritIncoming:
			case SpellEffect::FcIncreaseNumHits:
			case SpellEffect::CastonFocusEffect:
			case SpellEffect::FcHealAmtIncoming:
			case SpellEffect::LimitManaMax:
			case SpellEffect::DoubleRangedAttack:
			case SpellEffect::ShieldEquipDmgMod:
			case SpellEffect::TriggerOnReqTarget:
			case SpellEffect::LimitRace:
			case SpellEffect::FcLimitUse:
			case SpellEffect::FcMute:
			case SpellEffect::LimitUseType:
			case SpellEffect::FcStunTimeMod:
			case SpellEffect::StunBashChance:
			case SpellEffect::IncreaseChanceMemwipe:
			case SpellEffect::CriticalMend:
			case SpellEffect::LimitCastTimeMax:
			case SpellEffect::TriggerOnReqCaster:
			case SpellEffect::FrenziedDevastation:
			case SpellEffect::AStacker:
			case SpellEffect::BStacker:
			case SpellEffect::CStacker:
			case SpellEffect::DStacker:
			case SpellEffect::DoubleRiposte:
			case SpellEffect::Berserk:
			case SpellEffect::Vampirism:
			case SpellEffect::Metabolism:
			case SpellEffect::FinishingBlow:
			case SpellEffect::FinishingBlowLvl:
			case SpellEffect::Assassinate:
			case SpellEffect::AssassinateLevel:
			case SpellEffect::FactionModPct:
			case SpellEffect::LimitSpellClass:
			case SpellEffect::Sanctuary:
			case SpellEffect::PetMeleeMitigation:
			case SpellEffect::SkillProcAttempt:
			case SpellEffect::SkillProcSuccess:
			case SpellEffect::SpellResistReduction:
			case SpellEffect::IncreaseArchery:
			case SpellEffect::Duration_HP_Pct:
			case SpellEffect::Duration_Mana_Pct:
			case SpellEffect::Duration_Endurance_Pct:
			case SpellEffect::Endurance_Absorb_Pct_Damage:
			case SpellEffect::AC_Mitigation_Max_Percent:
			case SpellEffect::AC_Avoidance_Max_Percent:
			case SpellEffect::Attack_Accuracy_Max_Percent:
			case SpellEffect::Critical_Melee_Damage_Mod_Max:
			case SpellEffect::Melee_Damage_Position_Mod:
			case SpellEffect::Damage_Taken_Position_Mod:
			case SpellEffect::Melee_Damage_Position_Amt:
			case SpellEffect::Damage_Taken_Position_Amt:
			case SpellEffect::DS_Mitigation_Amount:
			case SpellEffect::DS_Mitigation_Percentage:
			case SpellEffect::Double_Backstab_Front:
			case SpellEffect::Pet_Crit_Melee_Damage_Pct_Owner:
			case SpellEffect::Pet_Add_Atk:
			case SpellEffect::TwinCastBlocker:
			case SpellEffect::Fc_Cast_Spell_On_Land:
			case SpellEffect::Ff_CasterClass:
			case SpellEffect::Ff_Same_Caster:
			case SpellEffect::Fc_ResistIncoming:
			case SpellEffect::Fc_Amplify_Amt:
			case SpellEffect::Fc_Amplify_Mod:
			case SpellEffect::Fc_CastTimeAmt:
			case SpellEffect::Fc_CastTimeMod2:
			case SpellEffect::Ff_DurationMax:
			case SpellEffect::Ff_Endurance_Max:
			case SpellEffect::Ff_Endurance_Min:
			case SpellEffect::Ff_ReuseTimeMin:
			case SpellEffect::Ff_ReuseTimeMax:
			case SpellEffect::Ff_Value_Min:
			case SpellEffect::Ff_Value_Max:
			case SpellEffect::AddExtraAttackPct_1h_Primary:
			case SpellEffect::AddExtraAttackPct_1h_Secondary:
			case SpellEffect::Double_Melee_Round:
			case SpellEffect::Skill_Base_Damage_Mod:
			case SpellEffect::Worn_Endurance_Regen_Cap:
			case SpellEffect::Buy_AA_Rank:
			case SpellEffect::Ff_FocusTimerMin:
			case SpellEffect::Proc_Timer_Modifier:
			case SpellEffect::FFItemClass:
			case SpellEffect::SpellEffectResistChance:
			case SpellEffect::SeeInvis:
			case SpellEffect::Invisibility:
			case SpellEffect::Invisibility2:
			case SpellEffect::InvisVsAnimals:
			case SpellEffect::ImprovedInvisAnimals:
			case SpellEffect::InvisVsUndead:
			case SpellEffect::InvisVsUndead2:
			case SpellEffect::Shield_Target:
			case SpellEffect::ReduceSkill:
			{
				break;
			}

			default:
			{
#ifdef SPELL_EFFECT_SPAM
				snprintf(effect_desc, _EDLEN, "Unknown Effect ID %d", effect);
#else
				Message(Chat::White, "Unknown spell effect %d in spell %s (id %d)", effect, spell.name, spell_id);
#endif
			}
		}
#ifdef SPELL_EFFECT_SPAM
		Message(Chat::White, ". . . Effect #%i: %s", i + 1, (effect_desc && effect_desc[0]) ? effect_desc : "Unknown");
#endif
	}

	CalcBonuses();

	if (SummonedItem) {
		Client *c=CastToClient();
		c->PushItemOnCursor(*SummonedItem);
		c->SendItemPacket(EQ::invslot::slotCursor, SummonedItem, ItemPacketLimbo);
		safe_delete(SummonedItem);
	}

	return true;
}

int64 Mob::CalcSpellEffectValue(uint16 spell_id, int effect_id, int caster_level, uint32 instrument_mod, Mob *caster,
	int ticsremaining, uint16 caster_id)
{
	if (!IsValidSpell(spell_id) || effect_id < 0 || effect_id >= EFFECT_COUNT)
		return 0;

	int formula = spells[spell_id].formula[effect_id];
	int base_value = spells[spell_id].base_value[effect_id];
	int max_value = spells[spell_id].max_value[effect_id];
	int effect_value = 0;
	int oval = 0;

	if (IsBlankSpellEffect(spell_id, effect_id))
		return 0;

	effect_value = CalcSpellEffectValue_formula(formula, base_value, max_value, caster_level, spell_id, ticsremaining);

	// this doesn't actually need to be a song to get mods, just the right skill
	if (EQ::skills::IsBardInstrumentSkill(spells[spell_id].skill)
		&& IsInstrumentModifierAppliedToSpellEffect(spell_id, spells[spell_id].effect_id[effect_id])) {
			oval = effect_value;
			effect_value = effect_value * static_cast<int>(instrument_mod) / 10;
			LogSpells("Effect value [{}] altered with bard modifier of [{}] to yeild [{}]",
				oval, instrument_mod, effect_value);
	}
	/*
		SPA 413 SpellEffect::FcBaseEffects, modifies base value of a spell effect after formula calcultion, but before other focuses.
		This is applied to non-Bards in Mob::GetInstrumentMod
		Like bard modifiers, this is sent in the action_struct using action->instrument_mod (which is a base effect modifier)

		Issue: value sent with action->instrument_mod needs to be 10 or higher. Therefore lowest possible percent chance would be 11 (calculated to 10%)
		there are modern spells that use less than 10% but we send as a uint where lowest value has to be 10, where it should be a float for current clients.
		Though not ideal, at the moment for spells that are instant effects, the action packet doesn't matter and we will calculate the actual percent here correctly.
		Logic here is, caster_id is only sent from ApplySpellBonuses. Thus if it is a buff a long as the base effects is set to over 10% and at +10% intervals
		it will focus the base value correctly.

	*/

	/*
		Calculate base effects modifier for casters who are not bards.
	*/

	//This is checked from Mob::SpellEffects and applied to instant spells and runes.
	if (caster && caster->GetClass() != Class::Bard && caster->HasBaseEffectFocus()) {

		oval = effect_value;
		int mod = caster->GetFocusEffect(focusFcBaseEffects, spell_id);
		effect_value += effect_value * mod / 100;

		LogSpells("Instant Effect value [{}] altered with base effects modifier of [{}] to yeild [{}]",
			oval, mod, effect_value);
	}
	//This is checked from Mob::ApplySpellBonuses, applied to buffs that receive bonuses. See above, must be in 10% intervals to work.
	else if (caster_id && instrument_mod > 10) {

		Mob* buff_caster = entity_list.GetMob(caster_id);//If targeted bard song needed to confirm caster is not bard.
		if (buff_caster && buff_caster->GetClass() != Class::Bard) {
			oval = effect_value;
			effect_value = effect_value * static_cast<int>(instrument_mod) / 10;

			LogSpells("Bonus Effect value [{}] altered with base effects modifier of [{}] to yeild [{}]",
				oval, instrument_mod, effect_value);
		}
	}

	return effect_value;
}

// generic formula calculations
int64 Mob::CalcSpellEffectValue_formula(uint32 formula, int64 base_value, int64 max_value, int caster_level, uint16 spell_id, int ticsremaining)
{
#ifdef LUA_EQEMU
	int64 lua_ret = 0;
	bool ignoreDefault = false;
	lua_ret = LuaParser::Instance()->CalcSpellEffectValue_formula(this, formula, base_value, max_value, caster_level, spell_id, ticsremaining, ignoreDefault);

	if (ignoreDefault) {
		return lua_ret;
	}
#endif

/*
i need those formulas checked!!!!

0 = base
1 - 99 = base + level * formulaID
100 = base
101 = base + level / 2
102 = base + level
103 = base + level * 2
104 = base + level * 3
105 = base + level * 4
106 ? base + level * 5
107 ? min + level / 2
108 = min + level / 3
109 = min + level / 4
110 = min + level / 5
119 ? min + level / 8
121 ? min + level / 4
122 = splurt
123 ?
203 = stacking issues ? max
205 = stacking issues ? 105


0x77 = min + level / 8
*/

	int64 result = 0, updownsign = 1, ubase = base_value;
	if(ubase < 0)
		ubase = 0 - ubase;

	// this updown thing might look messed up but if you look at the
	// spells it actually looks like some have a positive base and max where
	// the max is actually less than the base, hence they grow downward
/*
This seems to mainly catch spells where both base and max are negative.
Strangely, damage spells have a negative base and positive max, but
snare has both of them negative, yet their range should work the same:
(meaning they both start at a negative value and the value gets lower)
*/
	if (max_value < base_value && max_value != 0)
	{
		// values are calculated down
		updownsign = -1;
	}
	else
	{
		// values are calculated up
		updownsign = 1;
	}

	LogSpells("spell [{}] formula [{}] base [{}] max [{}] lvl [{}] Up/Down [{}]",
		spell_id, formula, base_value, max_value, caster_level, updownsign);

	switch(formula)
	{
		case 60:	//used in stun spells..?
		case 70:
			result = ubase/100; break;
		case 0:
		case 100:	// confirmed 2/6/04
			result = ubase; break;
		case 101:	// confirmed 2/6/04
			result = updownsign * (ubase + (caster_level / 2)); break;
		case 102:	// confirmed 2/6/04
			result = updownsign * (ubase + caster_level); break;
		case 103:	// confirmed 2/6/04
			result = updownsign * (ubase + (caster_level * 2)); break;
		case 104:	// confirmed 2/6/04
			result = updownsign * (ubase + (caster_level * 3)); break;
		case 105:	// confirmed 2/6/04
			result = updownsign * (ubase + (caster_level * 4)); break;

		case 107:
		{
			int ticdif = CalcBuffDuration_formula(caster_level, spells[spell_id].buff_duration_formula, spells[spell_id].buff_duration) - std::max((ticsremaining - 1), 0);
			if (ticdif < 0)
				ticdif = 0;
			result = updownsign * (ubase - ticdif);
			degenerating_effects = true;
			break;
		}
		case 108:
		{
			int ticdif = CalcBuffDuration_formula(caster_level, spells[spell_id].buff_duration_formula, spells[spell_id].buff_duration) - std::max((ticsremaining - 1), 0);
			if (ticdif < 0)
				ticdif = 0;
			result = updownsign * (ubase - (2 * ticdif));
			degenerating_effects = true;
			break;
		}
		case 109:	// confirmed 2/6/04
			result = updownsign * (ubase + (caster_level / 4)); break;

		case 110:	// confirmed 2/6/04
			//is there a reason we dont use updownsign here???
			result = ubase + (caster_level / 6);
			break;

		case 111:
			result = updownsign * (ubase + 6 * (caster_level - 16));
			break;
		case 112:
			result = updownsign * (ubase + 8 * (caster_level - 24));
			break;
		case 113:
			result = updownsign * (ubase + 10 * (caster_level - 34));
			break;
		case 114:
			result = updownsign * (ubase + 15 * (caster_level - 44));
			break;

		case 115:	// this is only in symbol of transal
			result = ubase;
			if (caster_level > 15)
				result += 7 * (caster_level - 15);
			break;
		case 116:	// this is only in symbol of ryltan
			result = ubase;
			if (caster_level > 24)
				result += 10 * (caster_level - 24);
			break;
		case 117:	// this is only in symbol of pinzarn
			result = ubase;
			if (caster_level > 34)
				result += 13 * (caster_level - 34);
			break;
		case 118:	// used in naltron and a few others
			result = ubase;
			if (caster_level > 44)
				result += 20 * (caster_level - 44);
			break;

		case 119:	// confirmed 2/6/04
			result = ubase + (caster_level / 8);
			break;
		case 120:
		{
			int ticdif = CalcBuffDuration_formula(caster_level, spells[spell_id].buff_duration_formula, spells[spell_id].buff_duration) - std::max((ticsremaining - 1), 0);
			if (ticdif < 0)
				ticdif = 0;
			result = updownsign * (ubase - (5 * ticdif));
			degenerating_effects = true;
			break;
		}
		case 121:	// corrected 2/6/04
			result = ubase + (caster_level / 3); break;
		case 122:
		{
			int ticdif = CalcBuffDuration_formula(caster_level, spells[spell_id].buff_duration_formula, spells[spell_id].buff_duration) - std::max((ticsremaining - 1), 0);
			if(ticdif < 0)
				ticdif = 0;

			result = updownsign * (ubase - (12 * ticdif));
			degenerating_effects = true;
			break;
		}
		case 123:	// added 2/6/04
			result = zone->random.Int(ubase, std::abs(max_value));
			break;

		case 124:	// check sign
			result = ubase;
			if (caster_level > 50)
				result += updownsign * (caster_level - 50);
			break;

		case 125:	// check sign
			result = ubase;
			if (caster_level > 50)
				result += updownsign * 2 * (caster_level - 50);
			break;

		case 126:	// check sign
			result = ubase;
			if (caster_level > 50)
				result += updownsign * 3 * (caster_level - 50);
			break;

		case 127:	// check sign
			result = ubase;
			if (caster_level > 50)
				result += updownsign * 4 * (caster_level - 50);
			break;

		case 128:	// check sign
			result = ubase;
			if (caster_level > 50)
				result += updownsign * 5 * (caster_level - 50);
			break;

		case 129:	// check sign
			result = ubase;
			if (caster_level > 50)
				result += updownsign * 10 * (caster_level - 50);
			break;

		case 130:	// check sign
			result = ubase;
			if (caster_level > 50)
				result += updownsign * 15 * (caster_level - 50);
			break;

		case 131:	// check sign
			result = ubase;
			if (caster_level > 50)
				result += updownsign * 20 * (caster_level - 50);
			break;

		case 132:	// check sign
			result = ubase;
			if (caster_level > 50)
				result += updownsign * 25 * (caster_level - 50);
			break;

		case 137:	// used in berserker AA desperation
			result = ubase - static_cast<int>((ubase * (GetHPRatio() / 100.0f)));
			break;

		case 138: { // unused on live?
			int64 maxhps = GetMaxHP() / 2;
			if (GetHP() <= maxhps)
				result = -(ubase * GetHP() / maxhps);
			else
				result = -ubase;
			break;
		}

		case 139:	// check sign
			result = ubase + (caster_level > 30 ? (caster_level - 30) / 2 : 0);
			break;

		case 140:	// check sign
			result = ubase + (caster_level > 30 ? caster_level - 30 : 0);
			break;

		case 141:	// check sign
			result = ubase + (caster_level > 30 ? (3 * caster_level - 90) / 2 : 0);
			break;

		case 142:	// check sign
			result = ubase + (caster_level > 30 ? 2 * caster_level - 60 : 0);
			break;

		case 143:	// check sign
			result = ubase + (3 * caster_level / 4);
			break;

		case 144:	// Level 40+ Harm Touch
			result = ubase + (caster_level * 10) + (caster_level - 40) * 20;
			break;

		//these are used in stacking effects... formula unknown
		case 201:
		case 203:
			result = max_value;
			break;
		default:
		{
			if (formula < 100)
				result = ubase + (caster_level * formula);
			else if((formula > 1000) && (formula < 1999))
			{
				// These work like splurt, accept instead of being hard coded to 12, it is formula - 1000.
				// Formula 1999 seems to have a slightly different effect, so is not included here
				int64 ticdif = CalcBuffDuration_formula(caster_level, spells[spell_id].buff_duration_formula, spells[spell_id].buff_duration) - std::max((ticsremaining - 1), 0);
				if(ticdif < 0)
					ticdif = 0;

				result = updownsign * (ubase - ((formula - 1000) * ticdif));
				degenerating_effects = true;
			}
			else if((formula >= 2000) && (formula <= 2650))
			{
				// Source: http://crucible.samanna.net/viewtopic.php?f=38&t=6259
				result = ubase * (caster_level * (formula - 2000) + 1);
			}
			else
				LogDebug("Unknown spell effect value forumula [{}]", formula);
		}
	}

	int64 oresult = result;

	// now check result against the allowed maximum
	if (max_value != 0)
	{
		if (updownsign == 1)
		{
			if (result > max_value)
				result = max_value;
		}
		else
		{
			if (result < max_value)
				result = max_value;
		}
	}

	// if base is less than zero, then the result need to be negative too
	if (base_value < 0 && result > 0)
		result *= -1;

	LogSpells("Result: [{}] (orig [{}]) cap [{}] [{}]", result, oresult, max_value, (base_value < 0 && result > 0)?"Inverted due to negative base":"");

	return result;
}


void Mob::BuffProcess()
{
	int buff_count = GetMaxTotalSlots();

	for (int buffs_i = 0; buffs_i < buff_count; ++buffs_i)
	{
		if (IsValidSpell(buffs[buffs_i].spellid))
		{
			DoBuffTic(buffs[buffs_i], buffs_i, entity_list.GetMob(buffs[buffs_i].casterid));
			// If the Mob died during DoBuffTic, then the buff we are currently processing will have been removed
			if(!IsValidSpell(buffs[buffs_i].spellid)) {
				continue;
			}

			// DF_Permanent uses -1 DF_Aura uses -4 but we need to check negatives for some spells for some reason?
			if (spells[buffs[buffs_i].spellid].buff_duration_formula != DF_Permanent &&
			    spells[buffs[buffs_i].spellid].buff_duration_formula != DF_Aura &&
				buffs[buffs_i].ticsremaining != PERMANENT_BUFF_DURATION) {
				if(!zone->BuffTimersSuspended() || !IsSuspendableSpell(buffs[buffs_i].spellid))
				{
					--buffs[buffs_i].ticsremaining;

					if (buffs[buffs_i].ticsremaining < 0) {
						LogSpells("Buff [{}] in slot [{}] has expired. Fading", buffs[buffs_i].spellid, buffs_i);
						BuffFadeBySlot(buffs_i);
					}
					else
					{
						LogSpells("Buff [{}] in slot [{}] has [{}] tics remaining", buffs[buffs_i].spellid, buffs_i, buffs[buffs_i].ticsremaining);
					}
				}
				else if (IsClient() && !(CastToClient()->ClientVersionBit() & EQ::versions::maskSoFAndLater))
				{
					buffs[buffs_i].UpdateClient = true;
				}
			}

			if(IsClient())
			{
				if(buffs[buffs_i].UpdateClient == true)
				{
					CastToClient()->SendBuffDurationPacket(buffs[buffs_i], buffs_i);
					// Hack to get UF to play nicer, RoF seems fine without it
					if (CastToClient()->ClientVersion() == EQ::versions::ClientVersion::UF && buffs[buffs_i].hit_number > 0)
						CastToClient()->SendBuffNumHitPacket(buffs[buffs_i], buffs_i);
					buffs[buffs_i].UpdateClient = false;
				}
			}
		}
	}
}

void Mob::DoBuffTic(const Buffs_Struct &buff, int slot, Mob *caster)
{
	int effect, effect_value;

	if (!IsValidSpell(buff.spellid))
		return;

	const SPDat_Spell_Struct &spell = spells[buff.spellid];

	const auto& export_string = fmt::format(
		"{} {} {} {}",
		caster ? caster->GetID() : 0,
		buffs[slot].ticsremaining,
		caster ? caster->GetLevel() : 0,
		slot
	);

	if (IsClient()) {
		if (parse->SpellHasQuestSub(buff.spellid, EVENT_SPELL_EFFECT_BUFF_TIC_CLIENT)) {
			if (parse->EventSpell(EVENT_SPELL_EFFECT_BUFF_TIC_CLIENT, nullptr, CastToClient(), buff.spellid, export_string, 0) != 0) {
				return;
			}
		}
	} else if (IsNPC()) {
		if (parse->SpellHasQuestSub(buff.spellid, EVENT_SPELL_EFFECT_BUFF_TIC_NPC)) {
			if (parse->EventSpell(EVENT_SPELL_EFFECT_BUFF_TIC_NPC, this, nullptr, buff.spellid, export_string, 0) != 0) {
				return;
			}
		}
	} else if (IsBot()) {
		if (parse->SpellHasQuestSub(buff.spellid, EVENT_SPELL_EFFECT_BUFF_TIC_BOT)) {
			if (parse->EventSpell(EVENT_SPELL_EFFECT_BUFF_TIC_BOT, this, nullptr, buff.spellid, export_string, 0) != 0) {
				return;
			}
		}
	}

	for (int i = 0; i < EFFECT_COUNT; i++) {
		if (IsBlankSpellEffect(buff.spellid, i))
			continue;

		effect = spell.effect_id[i];
		// I copied the calculation into each case which needed it instead of
		// doing it every time up here, since most buff effects dont need it

		switch (effect) {
		case SpellEffect::CurrentHP: {
			if (spells[buff.spellid].limit_value[i] && !PassCastRestriction(spells[buff.spellid].limit_value[i])) {
				break;
			}

			effect_value = CalcSpellEffectValue(buff.spellid, i, buff.casterlevel, buff.instrument_mod,
							    caster, buff.ticsremaining);
			// Handle client cast DOTs here.
			if (caster && effect_value < 0) {

				if (IsDetrimentalSpell(buff.spellid)) {
					if (caster->IsClient()) {
						if (!caster->CastToClient()->GetFeigned()) {
							AddToHateList(caster, -effect_value);
						}
					} else if (!IsClient()) { // Allow NPC's to generate hate if casted on other NPC's
						AddToHateList(caster, -effect_value);
					}
				}

				effect_value = caster->GetActDoTDamage(buff.spellid, effect_value, this);

				caster->ResourceTap(-effect_value, buff.spellid);
				effect_value = -effect_value;
				Damage(caster, effect_value, buff.spellid, spell.skill, false, i, true);
			} else if (effect_value > 0) {
				// Regen spell...
				// handled with bonuses
			}
			break;
		}
		case SpellEffect::HealOverTime: {
			effect_value = CalcSpellEffectValue(buff.spellid, i, buff.casterlevel, buff.instrument_mod);
			if (caster) {
				effect_value = caster->GetActSpellHealing(buff.spellid, effect_value, nullptr, true);
			}

			HealDamage(effect_value, caster, buff.spellid);
			// healing aggro would go here; removed for now
			break;
		}

		case SpellEffect::CurrentEndurance: {
			// Handled with bonuses
			break;
		}

		case SpellEffect::BardAEDot: {
			effect_value =
			    CalcSpellEffectValue(buff.spellid, i, buff.casterlevel, buff.instrument_mod, caster);

			if ((!RuleB(Spells, PreNerfBardAEDoT) && IsMoving()) || invulnerable ||
			    /*effect_value > 0 ||*/ DivineAura())
				break;

			if (effect_value < 0) {
				effect_value = -effect_value;
				if (caster) {
					if (caster->IsClient() && !caster->CastToClient()->GetFeigned()) {
						AddToHateList(caster, effect_value);
					} else if (!caster->IsClient())
						AddToHateList(caster, effect_value);
				}
				Damage(caster, effect_value, buff.spellid, spell.skill, false, i, true);
			} else if (effect_value > 0) {
				// healing spell...
				HealDamage(effect_value, caster);
				// healing aggro would go here; removed for now
			}
			break;
		}

		case SpellEffect::Hate: {
			effect_value = CalcSpellEffectValue(buff.spellid, i, buff.casterlevel, buff.instrument_mod);
			if (caster) {
				if (effect_value > 0) {
					if (caster) {
						if (caster->IsClient() && !caster->CastToClient()->GetFeigned()) {
							AddToHateList(caster, effect_value);
						} else if (!caster->IsClient())
							AddToHateList(caster, effect_value);
					}
				} else {
					int64 newhate = GetHateAmount(caster) + effect_value;
					if (newhate < 1) {
						SetHateAmountOnEnt(caster, 1);
					} else {
						SetHateAmountOnEnt(caster, newhate);
					}
				}
			}
			break;
		}

		case SpellEffect::WipeHateList: {
			if (IsMesmerizeSpell(buff.spellid)) {
				break;
			}

			int wipechance = 0;

			if (caster) {
				wipechance = caster->GetMemoryBlurChance(effect_value);
			}

			if (zone->random.Roll(wipechance)) {
				if (IsAIControlled()) {
					WipeHateList();
				}
				Message(Chat::Red, "Your mind fogs. Who are my friends? Who are my enemies?... it was all so "
					    "clear a moment ago...");
			}
			break;
		}

		case SpellEffect::Charm: {
			if (!caster || !PassCharismaCheck(caster, buff.spellid)) {
				BuffFadeByEffect(SpellEffect::Charm);

				// Remove from hate list of any NPC's hate list and remove all NPCs this hate list
				if (IsNPC()) {
					entity_list.RemoveFromHateLists(this);
					WipeHateList(true);
				}
			}

			break;
		}

		case SpellEffect::Root: {
			/* Root formula derived from extensive personal live parses - Kayen
			ROOT has a 70% chance to do a resist check to break.
			*/

			if (zone->random.Roll(RuleI(Spells, RootBreakCheckChance))) {
				float resist_check =
				    ResistSpell(spells[buff.spellid].resist_type, buff.spellid, caster, 0, 0, 0, 0, true);

				if (resist_check == 100)
					break;
				else if (!TryFadeEffect(slot))
					BuffFadeBySlot(slot);
			}

			break;
		}

		case SpellEffect::Fear: {
			if (zone->random.Roll(RuleI(Spells, FearBreakCheckChance))) {
				float resist_check = ResistSpell(spells[buff.spellid].resist_type, buff.spellid, caster,0,0,true);

				if (resist_check == 100)
					break;
				else if (!TryFadeEffect(slot))
					BuffFadeBySlot(slot);
			}

			break;
		}

		case SpellEffect::Invisibility:
		case SpellEffect::InvisVsAnimals:
		case SpellEffect::InvisVsUndead: {
			if (buff.ticsremaining > 3) {
				if (!IsBardSong(buff.spellid)) {
					double break_chance = 2.0;
					if (caster) {
						break_chance -= (2 * (((double)caster->GetSkill(EQ::skills::SkillDivination) +
								       ((double)caster->GetLevel() * 3.0)) /
								      650.0));
					} else {
						break_chance -=
						    (2 *
							(((double)GetSkill(EQ::skills::SkillDivination) + ((double)GetLevel() * 3.0)) /
						      650.0));
					}

					if (zone->random.Real(0.0, 100.0) < break_chance) {
						BuffModifyDurationBySpellID(buff.spellid, 3);
					}
				}
			}
		}
		case SpellEffect::ImprovedInvisAnimals:
		case SpellEffect::Invisibility2:
		case SpellEffect::InvisVsUndead2: {
			if (!IsBardSong(buff.spellid)) {
				if (buff.ticsremaining <= 3 && buff.ticsremaining > 1) {
					MessageString(Chat::Spells, INVIS_BEGIN_BREAK);
				}
			}
			break;
		}
		case SpellEffect::InterruptCasting: {
			if (IsCasting()) {
				const auto &spell = spells[casting_spell_id];
				if (!IsCastNotStandingSpell(spell.id) && zone->random.Roll(spells[buff.spellid].base_value[i])) {
					InterruptSpell();
				}
			}
			break;
		}
		// These effects always trigger when they fade.
		// Should we have this triggered from else where?
		case SpellEffect::CastOnFadeEffect:
		case SpellEffect::CastOnFadeEffectNPC:
		case SpellEffect::CastOnFadeEffectAlways: {
			if (buff.ticsremaining == 0) {
				SpellFinished(spells[buff.spellid].base_value[i], this, EQ::spells::CastingSlot::Item, 0, -1, spells[spells[buff.spellid].base_value[i]].resist_difficulty);
			}
			break;
		}
		case SpellEffect::LocateCorpse: {
			// This is handled by the client prior to SoD.

			if (IsClient() && (CastToClient()->ClientVersionBit() & EQ::versions::maskSoDAndLater))
				CastToClient()->LocateCorpse();
		}

		case SpellEffect::DistanceRemoval: {
			if (spellbonuses.DistanceRemoval) {

				int distance =
				    ((int(GetX()) - buff.caston_x) * (int(GetX()) - buff.caston_x)) +
				    ((int(GetY()) - buff.caston_y) * (int(GetY()) - buff.caston_y)) +
				    ((int(GetZ()) - buff.caston_z) * (int(GetZ()) - buff.caston_z));

				if (distance > (spells[buff.spellid].base_value[i] * spells[buff.spellid].base_value[i])) {

					if (!TryFadeEffect(slot))
						BuffFadeBySlot(slot, true);
				}
				break;
			}
		}

		case SpellEffect::AddHateOverTimePct: {
			if (IsNPC()) {
				int64 new_hate = CastToNPC()->GetHateAmount(caster) * (100 + spell.base_value[i]) / 100;
				if (new_hate <= 0)
					new_hate = 1;

				CastToNPC()->SetHateAmountOnEnt(caster, new_hate);
			}
			break;
		}

		case SpellEffect::Duration_HP_Pct: {
			effect_value = spells[buff.spellid].base_value[i];
			int64 amt = std::abs(GetMaxHP() * effect_value / 100);
			if (spells[buff.spellid].max_value[i] && amt > spells[buff.spellid].max_value[i])
				amt = spells[buff.spellid].max_value[i];

			if (effect_value < 0) {
				Damage(this, amt, 0, EQ::skills::SkillEvocation, false);
			}
			else {
				HealDamage(amt);
			}
			break;
		}

		case SpellEffect::Duration_Mana_Pct: {
			effect_value = spells[buff.spellid].base_value[i];
			int32 amt = std::abs(GetMaxMana() * effect_value / 100);
			if (spells[buff.spellid].max_value[i] && amt > spells[buff.spellid].max_value[i])
				amt = spells[buff.spellid].max_value[i];

			if (effect_value < 0) {

				SetMana(GetMana() - amt);
			}
			else {
				SetMana(GetMana() + amt);
			}
			break;
		}

		case SpellEffect::Duration_Endurance_Pct: {
			effect_value = spells[buff.spellid].base_value[i];

			if (IsClient())	{
				int32 amt = std::abs(CastToClient()->GetMaxEndurance() * effect_value / 100);
				if (spells[buff.spellid].max_value[i] && amt > spells[buff.spellid].max_value[i])
					amt = spells[buff.spellid].max_value[i];

				if (effect_value < 0) {
					CastToClient()->SetEndurance(CastToClient()->GetEndurance() - amt);
				}
				else {
					CastToClient()->SetEndurance(CastToClient()->GetEndurance() + amt);
				}
			}
			break;
		}

		default: {
			// do we need to do anyting here?
		}
		}
		if (!IsValidSpell(buff.spellid)) // if we faded we're no longer valid!
			break;
	}

	/* Is this the best place for this?
	 * Ideally we would only recalc spell bonuses
	 * but we would also have to call all the Calc functions like Max HP
	 * so lets just call the main CalcBonuses
	 */
	if (degenerating_effects)
		CalcBonuses();
}

// removes the buff in the buff slot 'slot'
void Mob::BuffFadeBySlot(int slot, bool iRecalcBonuses)
{
	if(slot < 0 || slot > GetMaxTotalSlots())
		return;

	if(!IsValidSpell(buffs[slot].spellid))
		return;

	if (IsClient() && !CastToClient()->IsDead())
		CastToClient()->MakeBuffFadePacket(buffs[slot].spellid, slot);

	LogSpells("Fading buff [{}] from slot [{}]", buffs[slot].spellid, slot);

	const auto has_fade_event = parse->SpellHasQuestSub(buffs[slot].spellid, EVENT_SPELL_FADE);
	std::string export_string = "";
	if (has_fade_event) {
		export_string = fmt::format(
			"{} {} {} {}",
			buffs[slot].casterid,
			buffs[slot].ticsremaining,
			buffs[slot].casterlevel,
			slot
		);
	}

	if (IsClient()) {
		if (has_fade_event) {
			if (parse->EventSpell(EVENT_SPELL_FADE, nullptr, CastToClient(), buffs[slot].spellid, export_string, 0) != 0) {
				return;
			}
		}
	} else if (IsNPC()) {
		if (has_fade_event) {
			if (parse->EventSpell(EVENT_SPELL_FADE, this, nullptr, buffs[slot].spellid, export_string, 0) != 0) {
				return;
			}
		}
	} else if (IsBot()) {
		if (has_fade_event) {
			if (parse->EventSpell(EVENT_SPELL_FADE, this, nullptr, buffs[slot].spellid, export_string, 0) != 0) {
				return;
			}
		}
	}

	for (int i=0; i < EFFECT_COUNT; i++)
	{
		if(IsBlankSpellEffect(buffs[slot].spellid, i))
			continue;

		switch (spells[buffs[slot].spellid].effect_id[i])
		{
			case SpellEffect::AddMeleeProc:
			case SpellEffect::WeaponProc:
			{
				uint16 proc_id = GetProcID(buffs[slot].spellid, i);

				// Special case for Vampiric Embrace. If this is a Shadow Knight, the proc is different.
				if (proc_id == SPELL_VAMPIRIC_EMBRACE && GetClass() == Class::ShadowKnight) {
					proc_id = SPELL_VAMPIRIC_EMBRACE_OF_SHADOW;
				}

				RemoveProcFromWeapon(proc_id, false);
				break;
			}

			case SpellEffect::DefensiveProc:
			{
				uint16 procid = GetProcID(buffs[slot].spellid, i);
				RemoveDefensiveProc(procid);
				break;
			}

			case SpellEffect::RangedProc:
			{
				uint16 procid = GetProcID(buffs[slot].spellid, i);
				RemoveRangedProc(procid);
				break;
			}

			case SpellEffect::SummonHorse:
			{
				if(IsClient())
				{
					/*Mob* horse = entity_list.GetMob(CastToClient()->GetHorseId());
					if (horse) horse->Depop();
					CastToClient()->SetHasMount(false);*/
					CastToClient()->SetHorseId(0);
				}
				break;
			}

			case SpellEffect::IllusionCopy:
			case SpellEffect::Illusion:
			{
				SendIllusionPacket(
					AppearanceStruct{
						.gender_id = GetBaseGender(),
						.race_id = GetBaseRace(),
						.size = GetDefaultRaceSize(GetBaseRace(), GetBaseGender()),
						.texture = 0
					}
				);

				for (int x = EQ::textures::textureBegin; x <= EQ::textures::LastTintableTexture; x++) {
					SendWearChange(x);
				}

				break;
			}

			case SpellEffect::Levitate:
			{
				if (!AffectedBySpellExcludingSlot(slot, SpellEffect::Levitate))
					SendAppearancePacket(AppearanceType::FlyMode, 0);
				break;
			}

			case SpellEffect::Silence:
			{
				Silence(false);
				break;
			}

			case SpellEffect::Amnesia:
			{
				Amnesia(false);
				break;
			}

			case SpellEffect::DivineAura:
			{
				SetInvul(false);
				break;
			}

			case SpellEffect::Rune:
			{
				buffs[slot].melee_rune = 0;
				break;
			}

			case SpellEffect::AbsorbMagicAtt:
			{
				buffs[slot].magic_rune = 0;
				break;
			}

			case SpellEffect::Familiar:
			{
				Mob *mypet = GetPet();
				if (mypet){
					if(mypet->IsNPC())
						mypet->CastToNPC()->Depop();
					SetPetID(0);
				}
				break;
			}

			case SpellEffect::Mez:
			{
				SendAppearancePacket(AppearanceType::Animation, Animation::Standing);	// unfreeze
				mezzed = false;
				break;
			}

			case SpellEffect::Charm:
			{
				if (IsNPC()) {
					CastToNPC()->RestoreGuardSpotCharm();
					CastToNPC()->ModifyStatsOnCharm(true, GetOwner());
				}

				SendAppearancePacket(AppearanceType::Pet, 0, true, true);
				Mob* owner = GetOwner();
				SetOwnerID(0);
				SetPetType(PetType::None);
				SetHeld(false);
				SetGHeld(false);
				SetNoCast(false);
				SetFocused(false);
				SetPetStop(false);
				SetPetRegroup(false);
				if(owner)
				{
					owner->SetPet(0);
				}

				// Any client that has a previous charmed pet targetted shouldo
				// no longer see the buffs on the old pet.
				// QueueClientsByTarget preserves GM and leadership cases.

				EQApplicationPacket *outapp = MakeBuffsPacket(true, true);

				entity_list.QueueClientsByTarget(this, outapp, false, nullptr, true, false, EQ::versions::maskSoDAndLater, true, true);

				if (IsAIControlled())
				{
					//Remove damage over time effects on charmed pet and those applied by charmed pet.
					if (RuleB(Spells, PreventFactionWarOnCharmBreak)) {
						for (auto mob : hate_list.GetHateList()) {
							auto tar = mob->entity_on_hatelist;
							if (tar) {
								if (tar->IsCasting()) {
									tar->InterruptSpell(tar->CastingSpellID());
								}
								uint32 buff_count = tar->GetMaxTotalSlots();
								for (unsigned int j = 0; j < buff_count; j++) {
									if (IsValidSpell(tar->GetBuffs()[j].spellid)) {
										auto spell = spells[tar->GetBuffs()[j].spellid];
										if (spell.good_effect == DETRIMENTAL_EFFECT && IsEffectInSpell(spell.id, SpellEffect::CurrentHP) && tar->GetBuffs()[j].casterid == GetID()) {
											tar->BuffFadeBySpellID(spell.id);
										}
									}
								}
							}
						}
						if (IsCasting()) {
							InterruptSpell(CastingSpellID());
						}
						uint32 buff_count = GetMaxTotalSlots();
						for (unsigned int j = 0; j < buff_count; j++) {
							if (IsValidSpell(GetBuffs()[j].spellid )) {
								auto spell = spells[GetBuffs()[j].spellid];
								if (spell.good_effect == DETRIMENTAL_EFFECT && IsEffectInSpell(spell.id, SpellEffect::CurrentHP)) {
									BuffFadeBySpellID(spell.id);
								}
							}
						}
					}

					// clear the hate list of the mobs
					entity_list.ReplaceWithTarget(this, owner);
					WipeHateList();
					if (owner) {
						AddToHateList(owner, 1, 0);
					}
					//If owner dead, briefly setting Immmune Aggro while hatelists wipe for both pet and targets is needed to ensure no reaggroing.
					else if (IsNPC()){
						bool has_aggro_immunity = GetSpecialAbility(SpecialAbility::AggroImmunity); //check if already immune aggro
						SetSpecialAbility(SpecialAbility::AggroImmunity, 1);
						WipeHateList();
						if (IsCasting()) {
							InterruptSpell(CastingSpellID());
						}
						entity_list.RemoveFromHateLists(this);
						//If NPC targeting charmed pet are in process of casting on it after it is removed from hatelist, stop the cast to prevent reaggroing.
						Mob *current_npc = nullptr;
						for (auto &it : entity_list.GetNPCList()) {
							current_npc = it.second;

							if (current_npc && current_npc->IsCasting() && current_npc->GetTarget() == this) {
								current_npc->InterruptSpell(current_npc->CastingSpellID());
							}
						}

						if (!has_aggro_immunity) {
							SetSpecialAbility(SpecialAbility::AggroImmunity, 0);
						}
					}
					SendAppearancePacket(AppearanceType::Animation, Animation::Standing);
				}
				if(owner && owner->IsClient())
				{
					auto app = new EQApplicationPacket(OP_Charm, sizeof(Charm_Struct));
					Charm_Struct *ps = (Charm_Struct*)app->pBuffer;
					ps->owner_id = owner->GetID();
					ps->pet_id = GetID();
					ps->command = 0;
					entity_list.QueueClients(this, app);
					safe_delete(app);
				}
				if(IsClient())
				{
					InterruptSpell();
					if (CastToClient()->IsLD())
						CastToClient()->AI_Start(CLIENT_LD_TIMEOUT);
					else
					{
						bool feared = FindType(SpellEffect::Fear);
						if(!feared)
							CastToClient()->AI_Stop();
					}
				}
				break;
			}

			case SpellEffect::Root:
			{
				buffs[slot].RootBreakChance = 0;
				rooted = false;
				break;
			}

			case SpellEffect::Blind:
				if (currently_fleeing && !FindType(SpellEffect::Fear))
					currently_fleeing = false;
				break;

			case SpellEffect::Fear:
			{
				if(RuleB(Combat, EnableFearPathing)){
					if(IsClient())
					{
						bool charmed = FindType(SpellEffect::Charm);
						if(!charmed)
							CastToClient()->AI_Stop();
					}

					if(currently_fleeing) {
						currently_fleeing = false;
						break;
					}
				}
				else
				{
					UnStun();
				}
				break;
			}

			case SpellEffect::ImmuneFleeing: {
				if (RuleB(Combat, EnableFearPathing)) {
					if (flee_mode) {
						currently_fleeing = true;
						CheckFlee();
					}
				}
				break;
			}

			case SpellEffect::BindSight:
			{
				if(IsClient())
				{
					CastToClient()->SetBindSightTarget(nullptr);
				}
				break;
			}

			case SpellEffect::SetBodyType:
			{
				SetBodyType(GetOrigBodyType(), false);
				break;
			}

			case SpellEffect::AlterNPCLevel:
			{
				if (IsNPC())
					SetLevel(GetOrigLevel());
				break;
			}

			case SpellEffect::EyeOfZomm:
			{
				if (IsClient())
				{
					NPC* tmp_eye_of_zomm = entity_list.GetNPCByID(CastToClient()->GetControlledMobId());
					//On live there is about a 6 second delay before it despawns once new one spawns.
					if (tmp_eye_of_zomm) {
						tmp_eye_of_zomm->GetSwarmInfo()->duration->Disable();
						tmp_eye_of_zomm->GetSwarmInfo()->duration->Start(6000);
						tmp_eye_of_zomm->DisableSwarmTimer();
						tmp_eye_of_zomm->StartSwarmTimer(6000);
					}
					CastToClient()->SetControlledMobId(0);
				}
				break;
			}

			case SpellEffect::Weapon_Stance:
			{
				/*
					If we click off the spell buff (or fades naturally) giving us
					Weapon Stance effects it should remove all associated buff.
				*/
				if (weaponstance.spellbonus_buff_spell_id) {
					BuffFadeBySpellID(weaponstance.spellbonus_buff_spell_id);
				}
				weaponstance.spellbonus_enabled = false;
			}
		}
	}

	// notify caster (or their master) of buff that it's worn off
	Mob *p = entity_list.GetMob(buffs[slot].casterid);
	if (p && p != this && !IsBardSong(buffs[slot].spellid))
	{
		Mob *notify = p;
		if(p->IsPet())
			notify = p->GetOwner();
		if(p) {
			notify->MessageString(Chat::SpellWornOff, SPELL_WORN_OFF_OF,
				spells[buffs[slot].spellid].name, GetCleanName());
		}
	}

	if (HasNumhits()){

		uint32 buff_max = GetMaxTotalSlots();
		bool found_numhits = false;

		for(uint32 d = 0; d < buff_max; d++) {

			if(IsValidSpell(buffs[d].spellid) && (buffs[d].hit_number > 0)) {
				Numhits(true);
				found_numhits = true;
			}
		}

		if (!found_numhits)
			Numhits(false);
	}

	if (spells[buffs[slot].spellid].nimbus_effect > 0)
		RemoveNimbusEffect(spells[buffs[slot].spellid].nimbus_effect);

	buffs[slot].spellid = SPELL_UNKNOWN;
	if(IsPet() && GetOwner() && GetOwner()->IsClient()) {
		SendPetBuffsToClient();
	}
	if((IsClient() && !CastToClient()->GetPVP()) ||
		(IsPet() && GetOwner() && GetOwner()->IsClient() && !GetOwner()->CastToClient()->GetPVP()) ||
		(IsBot() && GetOwner() && GetOwner()->IsClient() && !GetOwner()->CastToClient()->GetPVP()) ||
		(IsMerc() && GetOwner() && GetOwner()->IsClient() && !GetOwner()->CastToClient()->GetPVP()))
	{
		EQApplicationPacket *outapp = MakeBuffsPacket();

		entity_list.QueueClientsByTarget(this, outapp, false, nullptr, true, false, EQ::versions::maskSoDAndLater);
		if(IsClient() && GetTarget() == this) {
			CastToClient()->QueuePacket(outapp);
		}

		safe_delete(outapp);
	}

	if (IsNPC()) {
		EQApplicationPacket *outapp = MakeBuffsPacket();
		entity_list.QueueClientsByTarget(this, outapp, false, nullptr, true, false, EQ::versions::maskSoDAndLater, true);
		safe_delete(outapp);
	}

	if (IsClient() && CastToClient()->ClientVersionBit() & EQ::versions::maskUFAndLater)
	{
		EQApplicationPacket *outapp = MakeBuffsPacket(false);
		CastToClient()->FastQueuePacket(&outapp);
	}

	// we will eventually call CalcBonuses() even if we skip it right here, so should correct itself if we still have them
	degenerating_effects = false;
	if (iRecalcBonuses)
		CalcBonuses();
}

int64 Mob::CalcAAFocus(focusType type, const AA::Rank &rank, uint16 spell_id)
{
	const SPDat_Spell_Struct &spell = spells[spell_id];

	bool not_focusable = spells[spell_id].not_focusable;

	int64  value       = 0;
	int    lvlModifier = 100;
	int    spell_level = 0;
	int    lvldiff     = 0;
	uint32 effect      = 0;
	int32  base_value  = 0;
	int32  limit_value = 0;
	uint32 slot        = 0;

	int index_id = -1;
	uint32 focus_reuse_time = 0;

	bool   is_from_item_click      = false;
	bool   try_apply_to_item_click = false;
	bool   has_item_limit_check    = false;

	if (casting_spell_inventory_slot && casting_spell_inventory_slot != -1) {
		is_from_item_click = true;
	}

	bool LimitFailure                  = false;
	bool LimitInclude[MaxLimitInclude] = {false};
	/* Certain limits require only one of several Include conditions to be true. Ie. Add damage to fire OR ice
	spells.
	0/1 SpellEffect::_LimitResist
	2/3 SpellEffect::_LimitSpell
	4/5 SpellEffect::_LimitEffect
	6/7 SpellEffect::_LimitTarget
	8/9 SpellEffect::_LimitSpellGroup:
	10/11 SpellEffect::LimitCastingSkill:
	12/13 SpellEffect::LimitSpellClass:
	14/15 SpellEffect::LimitSpellSubClass:
	16/17 SpellEffect::FFItemCLass:
	Remember: Update MaxLimitInclude in spdat.h if adding new limits that require Includes
	*/

	int FocusCount = 0;

	for (const auto &e : rank.effects) {
		effect = e.effect_id;
		base_value  = e.base_value;
		limit_value  = e.limit_value;
		slot   = e.slot;

		/*
		AA Foci's can contain multiple focus effects within the same AA.
		To handle this we will not automatically return zero if a limit is found.
		Instead if limit is found and multiple focus effects, we will reset the limit check
		when the next valid focus effect is found.
		*/

		if (IsFocusEffect(0, 0, true, effect)) {
			FocusCount++;
			// If limit found on prior check next, else end loop.
			if (FocusCount > 1) {

				for (int i = 0; i < MaxLimitInclude; i += 2) {
					if (LimitInclude[i] && !LimitInclude[i + 1]) {
						LimitFailure = true;
					}
				}

				if (LimitFailure) {
					value        = 0;
					LimitFailure = false;

					for (bool & l : LimitInclude) {
						l = false; // Reset array
					}
				}

				else {
					break;
				}
			}
		}

		switch (effect) {
			case SpellEffect::Blank:
				break;

			// Handle Focus Limits
			case SpellEffect::LimitResist:
				if (base_value < 0) {
					if (spell.resist_type == -base_value) { // Exclude
						LimitFailure = true;
					}
				}
				else {
					LimitInclude[IncludeExistsSELimitResist] = true;
					if (spell.resist_type == base_value) { // Include
						LimitInclude[IncludeFoundSELimitResist] = true;
					}
				}
				break;

			case SpellEffect::LimitInstant:
				if (base_value == 1 && spell.buff_duration) { // Fail if not instant
					LimitFailure = true;
				}
				if (base_value == 0 && (spell.buff_duration == 0)) { // Fail if instant
					LimitFailure = true;
				}

				break;

			case SpellEffect::LimitMaxLevel:
				spell_level = spell.classes[(GetClass() % 17) - 1];
				lvldiff     = spell_level - base_value;
				// every level over cap reduces the effect by base2 percent unless from a clicky when
				// ItemCastsUseFocus is true
				if (lvldiff > 0 && (spell_level <= RuleI(Character, MaxLevel) || RuleB(Character, ItemCastsUseFocus) == false)) {
					if (limit_value > 0) {
						lvlModifier -= limit_value * lvldiff;
						if (lvlModifier < 1) {
							LimitFailure = true;
						}
					}
					else {
						LimitFailure = true;
					}
				}
				break;

			case SpellEffect::LimitMinLevel:
				if ((spell.classes[(GetClass() % 17) - 1]) < base_value) {
					LimitFailure = true;
				}
				break;

			case SpellEffect::LimitCastTimeMin:
				if (static_cast<int32>(spell.cast_time) < base_value) {
					LimitFailure = true;
				}
				break;

			case SpellEffect::LimitCastTimeMax:
				if (static_cast<int32>(spell.cast_time) > base_value) {
					LimitFailure = true;
				}
				break;

			case SpellEffect::LimitSpell:
				if (base_value < 0) { // Exclude
					if (spell_id == -base_value) {
						LimitFailure = true;
					}
				}
				else {
					LimitInclude[IncludeExistsSELimitSpell] = true;
					if (spell_id == base_value) { // Include
						LimitInclude[IncludeFoundSELimitSpell] = true;
					}
				}
				break;

			case SpellEffect::LimitMinDur:
				if (base_value > CalcBuffDuration_formula(GetLevel(), spell.buff_duration_formula, spell.buff_duration)) {
					LimitFailure = true;
				}
				break;

			case SpellEffect::LimitEffect:
				if (base_value < 0) {
					if (IsEffectInSpell(spell_id, -base_value)) { // Exclude
						LimitFailure = true;
					}
				}
				else {
					LimitInclude[IncludeExistsSELimitEffect] = true;
					// they use 33 here for all classes ... unsure if the type check is really needed
					if (base_value == SpellEffect::SummonPet && type == focusReagentCost) {
						if (IsPetSpell(spell_id)) {
							LimitInclude[IncludeFoundSELimitEffect] = true;
						}
					}
					else {
						if (IsEffectInSpell(spell_id, base_value)) { // Include
							LimitInclude[IncludeFoundSELimitEffect] = true;
						}
					}
				}
				break;

			case SpellEffect::LimitSpellType:
				switch (base_value) {
					case 0:
						if (!IsDetrimentalSpell(spell_id)) {
							LimitFailure = true;
						}
						break;
					case 1:
						if (!IsBeneficialSpell(spell_id)) {
							LimitFailure = true;
						}
						break;
				}
				break;

			case SpellEffect::LimitManaMin:
				if (spell.mana < base_value) {
					LimitFailure = true;
				}
				break;

			case SpellEffect::LimitManaMax:
				if (spell.mana > base_value) {
					LimitFailure = true;
				}
				break;

			case SpellEffect::LimitTarget:
				if (base_value < 0) {
					if (-base_value == spell.target_type) { // Exclude
						LimitFailure = true;
					}
				}
				else {
					LimitInclude[IncludeExistsSELimitTarget] = true;
					if (base_value == spell.target_type) { // Include
						LimitInclude[IncludeFoundSELimitTarget] = true;
					}
				}
				break;

			case SpellEffect::LimitCombatSkills:
				if (base_value == 0 && (IsCombatSkill(spell_id) || IsCombatProc(spell_id))) { // Exclude Discs / Procs
					LimitFailure = true;
				}
				else if (base_value == 1 && (!IsCombatSkill(spell_id) || !IsCombatProc(spell_id))) { // Exclude Spells
					LimitFailure = true;
				}

				break;

			case SpellEffect::LimitSpellGroup:
				if (base_value < 0) {
					if (-base_value == spell.spell_group) { // Exclude
						LimitFailure = true;
					}
				}
				else {
					LimitInclude[IncludeExistsSELimitSpellGroup] = true;
					if (base_value == spell.spell_group) { // Include
						LimitInclude[IncludeFoundSELimitSpellGroup] = true;
					}
				}
				break;

			case SpellEffect::LimitCastingSkill:
				if (base_value < 0) {
					if (-base_value == spell.skill) {
						LimitFailure = true;
					}
				}
				else {
					LimitInclude[IncludeExistsSELimitCastingSkill] = true;
					if (base_value == spell.skill) {
						LimitInclude[IncludeFoundSELimitCastingSkill] = true;
					}
				}
				break;

			case SpellEffect::LimitSpellClass:
				if (base_value < 0) { // Exclude
					if (-base_value == spell.spell_class) {
						LimitFailure = true;
					}
				}
				else {
					LimitInclude[IncludeExistsSELimitSpellClass] = true;
					if (base_value == spell.spell_class) { // Include
						LimitInclude[IncludeFoundSELimitSpellClass] = true;
					}
				}
				break;

			case SpellEffect::LimitSpellSubclass:
				if (base_value < 0) { // Exclude
					if (-base_value == spell.spell_subclass) {
						LimitFailure = true;
					}
				}
				else {
					LimitInclude[IncludeExistsSELimitSpellSubclass] = true;
					if (base_value == spell.spell_subclass) { // Include
						LimitInclude[IncludeFoundSELimitSpellSubclass] = true;
					}
				}
				break;

			case SpellEffect::LimitClass:
				// Do not use this limit more then once per spell. If multiple class, treat value like items
				// would.
				if (!PassLimitClass(base_value, GetClass())) {
					LimitFailure = true;
				}
				break;

			case SpellEffect::LimitRace:
				if (base_value != GetRace()) {
					LimitFailure = true;
				}
				break;

			case SpellEffect::LimitUseMin:
				if (base_value > spell.hit_number) {
					LimitFailure = true;
				}
				break;

			case SpellEffect::LimitUseType:
				if (base_value != spell.hit_number_type) {
					LimitFailure = true;
				}
				break;

			case SpellEffect::Ff_DurationMax:
				if (base_value > spell.buff_duration) {
					LimitFailure = true;
				}
				break;

			case SpellEffect::Ff_Endurance_Min:
				if (spell.endurance_cost < base_value) {
					LimitFailure = true;
				}
				break;

			case SpellEffect::Ff_Endurance_Max:
				if (spell.endurance_cost > base_value) {
					LimitFailure = true;
				}
				break;

			case SpellEffect::Ff_ReuseTimeMin:
				if (spell.recast_time < base_value) {
					LimitFailure = true;
				}
				break;

			case SpellEffect::Ff_ReuseTimeMax:
				if (spell.recast_time > base_value) {
					LimitFailure = true;
				}
				break;

			case SpellEffect::Ff_Value_Min:
				index_id = GetSpellEffectIndex(spell_id, limit_value);
				if (index_id >= 0 && spell.base_value[index_id] < base_value) {
					LimitFailure = true;
				}
				break;

			case SpellEffect::Ff_Value_Max:
				index_id = GetSpellEffectIndex(spell_id, limit_value);
				if (index_id >= 0 && spell.base_value[index_id] > base_value) {
					LimitFailure = true;
				}
				break;

			case SpellEffect::Ff_Override_NotFocusable:
				if (base_value == 1) {
					not_focusable = false;
				}
				break;

			case SpellEffect::Ff_FocusTimerMin:
				if (IsFocusProcLimitTimerActive(-rank.id)) {
					LimitFailure = true;
				}
				else {
					focus_reuse_time = limit_value;
				}
				break;

			case SpellEffect::FFItemClass:
				has_item_limit_check = true;
				if (casting_spell_inventory_slot && casting_spell_inventory_slot != -1) {
					if (IsClient() && casting_spell_slot == EQ::spells::CastingSlot::Item && casting_spell_inventory_slot != 0xFFFFFFFF) {
						auto item = CastToClient()->GetInv().GetItem(casting_spell_inventory_slot);
						if (item && item->GetItem()) {
							//If ItemType set to < -1, then we will exclude either all Subtypes (-1000), or specific items by ItemType, SubType or Slot. See above for rules.
							if (base_value < -1) { //Excludes
								bool exclude_this_item = true;
								int tmp_itemtype = (item->GetItem()->ItemType + 100) * -1;
								//ItemType (if set to -1000, ignore and exclude any ItemType)
								if (base_value < -1 && base_value != -1000) {
									if (base_value != tmp_itemtype) {
										exclude_this_item = false;
									}
								}
								//SubType (if set to -1, ignore and exclude all SubTypes)
								if (limit_value >= 0) {
									if (limit_value != item->GetItem()->SubType) {
										exclude_this_item = false;
									}
								}
								if (exclude_this_item) {
									LimitFailure = true;
								}
							}
							else {//Includes
								LimitInclude[IncludeExistsSEFFItemClass] = true;
								bool include_this_item = true;
								//ItemType (if set to -1, ignore and include any ItemType)
								if (base_value >= 0) {
									if (base_value != item->GetItem()->ItemType) {
										include_this_item = false;
									}
								}
								//SubType (if set to -1, ignore and include any SubType)
								if (limit_value >= 0) {
									if (limit_value != item->GetItem()->SubType) {
										include_this_item = false;
									}
								}
								if (include_this_item) {
									LimitInclude[IncludeFoundSEFFItemClass] = true;
								}
							}
						}
					}
				}
				//If this is checking that focus can only be cast from an item, then if its not cast from item fail.
				else if (base_value >= -1) {
					LimitFailure = true;
				}
				//If we are checking to exclude items from a focus then do not fail unless the above check fails.
				break;

				/* These are not applicable to AA's because there is never a 'caster' of the 'buff' with the focus effect.
				case SpellEffect::Ff_Same_Caster:
				case SpellEffect::Ff_CasterClass:
				*/

				// Handle Focus Effects
			case SpellEffect::ImprovedDamage:
				if (type == focusImprovedDamage && base_value > value) {
					value = base_value;
				}
				break;

			case SpellEffect::ImprovedDamage2:
				if (type == focusImprovedDamage2 && base_value > value) {
					value = base_value;
				}
				break;

			case SpellEffect::Fc_Amplify_Mod:
				if (type == focusFcAmplifyMod && base_value > value) {
					value = base_value;
				}
				break;

			case SpellEffect::ImprovedHeal:
				if (type == focusImprovedHeal && base_value > value) {
					value = base_value;
				}
				break;

			case SpellEffect::ReduceManaCost:
				if (type == focusManaCost) {
					value = base_value;
				}
				break;

			case SpellEffect::IncreaseSpellHaste:
				if (type == focusSpellHaste && base_value > value) {
					value = base_value;
					try_apply_to_item_click = is_from_item_click ? true : false;
				}
				break;

			case SpellEffect::Fc_CastTimeMod2:
				if (type == focusFcCastTimeMod2 && base_value > value) {
					value = base_value;
					try_apply_to_item_click = is_from_item_click ? true : false;
				}
				break;

			case SpellEffect::Fc_CastTimeAmt:
				if (type == focusFcCastTimeAmt && base_value > value) {
					value = base_value;
					try_apply_to_item_click = is_from_item_click ? true : false;
				}
				break;

			case SpellEffect::IncreaseSpellDuration:
				if (type == focusSpellDuration && base_value > value) {
					value = base_value;
				}
				break;

			case SpellEffect::SpellDurationIncByTic:
				if (type == focusSpellDurByTic && base_value > value) {
					value = base_value;
				}
				break;

			case SpellEffect::SwarmPetDuration:
				if (type == focusSwarmPetDuration && base_value > value) {
					value = base_value;
				}
				break;

			case SpellEffect::IncreaseRange:
				if (type == focusRange && base_value > value) {
					value = base_value;
				}
				break;

			case SpellEffect::ReduceReagentCost:
				if (type == focusReagentCost && base_value > value) {
					value = base_value;
				}
				break;

			case SpellEffect::PetPowerIncrease:
				if (type == focusPetPower && base_value > value) {
					value = base_value;
				}
				break;

			case SpellEffect::SpellResistReduction:
				if (type == focusResistRate && base_value > value) {
					value = base_value;
				}
				break;

			case SpellEffect::Fc_ResistIncoming:
				if (type == focusFcResistIncoming && base_value > value) {
					value = base_value;
				}
				break;

			case SpellEffect::SpellHateMod:
				if (type == focusSpellHateMod) {
					if (value != 0) {
						if (value > 0) {
							if (base_value > value) {
								value = base_value;
							}
						}
						else {
							if (base_value < value) {
								value = base_value;
							}
						}
					}
					else {
						value = base_value;
					}
				}
				break;

			case SpellEffect::ReduceReuseTimer:
				if (type == focusReduceRecastTime) {
					value = base_value / 1000;
					try_apply_to_item_click = is_from_item_click ? true : false;
				}
				break;

			case SpellEffect::TriggerOnCast:
				if (type == focusTriggerOnCast) {
					if (zone->random.Roll(base_value)) {
						value = limit_value;
					}
					else {
						value        = 0;
						LimitFailure = true;
					}
				}
				break;

			case SpellEffect::FcSpellVulnerability:
				if (type == focusSpellVulnerability) {
					value = base_value;
				}
				break;

			case SpellEffect::Fc_Spell_Damage_Pct_IncomingPC:
				if (type == focusFcSpellDamagePctIncomingPC) {
					value = base_value;
				}
				break;

			case SpellEffect::BlockNextSpellFocus:
				if (type == focusBlockNextSpell) {
					if (zone->random.Roll(base_value)) {
						value = 1;
					}
				}
				break;

			case SpellEffect::FcTwincast:
				if (type == focusTwincast && !IsEffectInSpell(spell_id, SpellEffect::TwinCastBlocker)) {
					value = base_value;
				}
				break;

				// Note if using these as AA, make sure this is first focus used.
			case SpellEffect::SympatheticProc:
				if (type == focusSympatheticProc) {
					value = limit_value;
				}
				break;

			case SpellEffect::FcDamageAmt:
				if (type == focusFcDamageAmt) {
					value = base_value;
				}
				break;

			case SpellEffect::FcDamageAmt2:
				if (type == focusFcDamageAmt2) {
					value = base_value;
				}
				break;

			case SpellEffect::Fc_Amplify_Amt:
				if (type == focusFcAmplifyAmt) {
					value = base_value;
				}
				break;

			case SpellEffect::FcDamageAmtCrit:
				if (type == focusFcDamageAmtCrit) {
					value = base_value;
				}
				break;

			case SpellEffect::FcDamageAmtIncoming:
				if (type == focusFcDamageAmtIncoming) {
					value = base_value;
				}
				break;

			case SpellEffect::Fc_Spell_Damage_Amt_IncomingPC:
				if (type == focusFcSpellDamageAmtIncomingPC) {
					value = base_value;
				}
				break;

			case SpellEffect::FcHealAmtIncoming:
				if (type == focusFcHealAmtIncoming) {
					value = base_value;
				}
				break;

			case SpellEffect::FcHealPctCritIncoming:
				if (type == focusFcHealPctCritIncoming) {
					value = base_value;
				}
				break;

			case SpellEffect::FcHealAmtCrit:
				if (type == focusFcHealAmtCrit) {
					value = base_value;
				}
				break;

			case SpellEffect::FcHealAmt:
				if (type == focusFcHealAmt) {
					value = base_value;
				}
				break;

			case SpellEffect::FcHealPctIncoming:
				if (type == focusFcHealPctIncoming) {
					value = base_value;
				}
				break;

			case SpellEffect::FcBaseEffects:
				if (type == focusFcBaseEffects) {
					value = base_value;
				}
				break;

			case SpellEffect::FcDamagePctCrit:
				if (type == focusFcDamagePctCrit) {
					value = base_value;
				}
				break;

			case SpellEffect::FcIncreaseNumHits:
				if (type == focusIncreaseNumHits) {
					value = base_value;
				}
				break;

			case SpellEffect::FcLimitUse:
				if (type == focusFcLimitUse) {
					value = base_value;
				}
				break;

			case SpellEffect::FcMute:
				if (type == focusFcMute) {
					value = base_value;
				}
				break;

			case SpellEffect::FcStunTimeMod:
				if (type == focusFcStunTimeMod) {
					value = base_value;
				}
				break;

			case SpellEffect::Fc_Cast_Spell_On_Land:
				if (type == focusFcCastSpellOnLand) {
					if (zone->random.Roll(base_value)) {
						value = limit_value;
					}
					break;
				}
		}
	}

	for (int e = 0; e < MaxLimitInclude; e += 2) {
		if (LimitInclude[e] && !LimitInclude[e + 1]) {
			return 0;
		}
	}

	if (try_apply_to_item_click && !has_item_limit_check) {
		return 0;
	}

	if (LimitFailure) {
		return 0;
	}

	if (not_focusable) {
		return 0;
	}

	if (focus_reuse_time) {
		SetFocusProcLimitTimer(-rank.id, focus_reuse_time);
	}

	return (value * lvlModifier / 100);
}

//given an item/spell's focus ID and the spell being cast, determine the focus ammount, if any
//assumes that spell_id is not a bard spell and that both ids are valid spell ids
int64 Mob::CalcFocusEffect(focusType type, uint16 focus_id, uint16 spell_id, bool best_focus, uint16 casterid, Mob *caster)
{
	/*
	'this' is always the caster of the spell_id, most foci check for effects on the caster, however some check for effects on the target.
	'casterid' is the casterid of the caster of spell_id, used when spell_id is cast on a target with a focus effect that is checked by incoming spell.
	*/
	if (!IsValidSpell(focus_id) || !IsValidSpell(spell_id)) {
		return 0;
	}

	// No further checks if spell_id no_focusable, unless spell focus_id contains an override limiter.
	if (spells[spell_id].not_focusable && !IsEffectInSpell(focus_id, SpellEffect::Ff_Override_NotFocusable)) {
		return 0;
	}

	const SPDat_Spell_Struct &focus_spell = spells[focus_id];
	const SPDat_Spell_Struct &spell       = spells[spell_id];

	int64  value           = 0;
	int    lvlModifier     = 100;
	int    spell_level     = 0;
	int    lvldiff         = 0;
	uint32 Caston_spell_id = 0;
	int    index_id        = -1;
	uint32 focus_reuse_time = 0; //If this is set and all limits pass, start timer at end of script.

	bool   is_from_item_click      = false;
	bool   try_apply_to_item_click = false;
	bool   has_item_limit_check    = false;

	if (casting_spell_inventory_slot && casting_spell_inventory_slot != -1) {
		is_from_item_click = true;
	}


	bool LimitInclude[MaxLimitInclude] = {false};
	/* Certain limits require only one of several Include conditions to be true. Determined by limits being negative or positive
	Ie. Add damage to fire OR ice spells. If positive we 'Include', by checking each limit of same type to look for match until found. Opposed to
	just 'Excluding', where if set to negative, if we find that match then focus fails, ie Add damage to all spells BUT Fire.
	0/1 SpellEffect::_LimitResist
	2/3 SpellEffect::_LimitSpell
	4/5 SpellEffect::_LimitEffect
	6/7 SpellEffect::_LimitTarget
	8/9 SpellEffect::_LimitSpellGroup:
	10/11 SpellEffect::LimitCastingSkill:
	12/13 SpellEffect::LimitSpellClass:
	14/15 SpellEffect::LimitSpellSubClass:
	16/17 SpellEffect::FFItemCLass:
	Remember: Update MaxLimitInclude in spdat.h if adding new limits that require Includes
	*/

	for (int i = 0; i < EFFECT_COUNT; i++) {

		switch (focus_spell.effect_id[i]) {

			case SpellEffect::Blank:
				break;

			case SpellEffect::LimitResist:
				if (focus_spell.base_value[i] < 0) {
					if (spell.resist_type == -focus_spell.base_value[i]) { // Exclude
						return 0;
					}
				}
				else {
					LimitInclude[IncludeExistsSELimitResist] = true;
					if (spell.resist_type == focus_spell.base_value[i]) { // Include
						LimitInclude[IncludeFoundSELimitResist] = true;
					}
				}
				break;

			case SpellEffect::LimitInstant:
				if (focus_spell.base_value[i] == 1 && spell.buff_duration) { // Fail if not instant
					return 0;
				}
				if (focus_spell.base_value[i] == 0 && (spell.buff_duration == 0)) { // Fail if instant
					return 0;
				}

				break;

			case SpellEffect::LimitMaxLevel:
				if (IsNPC()) {
					break;
				}
				spell_level = spell.classes[(GetClass() % 17) - 1];
				lvldiff     = spell_level - focus_spell.base_value[i];
				// every level over cap reduces the effect by focus_spell.base2[i] percent unless from a clicky
				// when ItemCastsUseFocus is true
				if (lvldiff > 0 && (spell_level <= RuleI(Character, MaxLevel) ||
									RuleB(Character, ItemCastsUseFocus) == false)) {
					if (focus_spell.limit_value[i] > 0) {
						lvlModifier -= focus_spell.limit_value[i] * lvldiff;
						if (lvlModifier < 1) {
							return 0;
						}
					}
					else {
						return 0;
					}
				}
				break;

			case SpellEffect::LimitMinLevel:
				if (IsNPC()) {
					break;
				}
				if (spell.classes[(GetClass() % 17) - 1] < focus_spell.base_value[i]) {
					return (0);
				}
				break;

			case SpellEffect::LimitCastTimeMin:
				if (spells[spell_id].cast_time < (uint16) focus_spell.base_value[i]) {
					return (0);
				}
				break;

			case SpellEffect::LimitCastTimeMax:
				if (spells[spell_id].cast_time > (uint16) focus_spell.base_value[i]) {
					return (0);
				}
				break;

			case SpellEffect::LimitSpell:
				if (focus_spell.base_value[i] < 0) { // Exclude
					if (spell_id == -focus_spell.base_value[i]) {
						return (0);
					}
				}
				else {
					LimitInclude[IncludeExistsSELimitSpell] = true;
					if (spell_id == focus_spell.base_value[i]) { // Include
						LimitInclude[IncludeFoundSELimitSpell] = true;
					}
				}
				break;

			case SpellEffect::LimitMinDur:
				if (focus_spell.base_value[i] >
					CalcBuffDuration_formula(GetLevel(), spell.buff_duration_formula, spell.buff_duration)) {
					return (0);
				}
				break;

			case SpellEffect::LimitEffect:
				if (focus_spell.base_value[i] < 0) {
					if (IsEffectInSpell(spell_id, -focus_spell.base_value[i])) { // Exclude
						return 0;
					}
				}
				else {
					LimitInclude[IncludeExistsSELimitEffect] = true;
					if (IsEffectInSpell(spell_id, focus_spell.base_value[i])) { // Include
						LimitInclude[IncludeFoundSELimitEffect] = true;
					}
				}
				break;

			case SpellEffect::LimitSpellType:
				switch (focus_spell.base_value[i]) {
					case 0:
						if (!IsDetrimentalSpell(spell_id)) {
							return 0;
						}
						break;
					case 1:
						if (!IsBeneficialSpell(spell_id)) {
							return 0;
						}
						break;
					default:
						LogInfo("unknown limit spelltype [{}]", focus_spell.base_value[i]);
						break;
				}
				break;

			case SpellEffect::LimitManaMin:
				if (spell.mana < focus_spell.base_value[i]) {
					return 0;
				}
				break;

			case SpellEffect::LimitManaMax:
				if (spell.mana > focus_spell.base_value[i]) {
					return 0;
				}
				break;

			case SpellEffect::LimitTarget:
				if (focus_spell.base_value[i] < 0) {
					if (-focus_spell.base_value[i] == spell.target_type) { // Exclude
						return 0;
					}
				}
				else {
					LimitInclude[IncludeExistsSELimitTarget] = true;
					if (focus_spell.base_value[i] == spell.target_type) { // Include
						LimitInclude[IncludeFoundSELimitTarget] = true;
					}
				}
				break;

			case SpellEffect::LimitCombatSkills:
				if (focus_spell.base_value[i] == 0 &&
					(IsCombatSkill(spell_id) || IsCombatProc(spell_id))) { // Exclude Discs / Procs
					return 0;
				}
				else if (focus_spell.base_value[i] == 1 && (!IsCombatSkill(spell_id) || !IsCombatProc(spell_id))) { // Exclude Spells
					return 0;
				}

				break;

			case SpellEffect::LimitSpellGroup:
				if (focus_spell.base_value[i] < 0) {
					if (-focus_spell.base_value[i] == spell.spell_group) { // Exclude
						return 0;
					}
				}
				else {
					LimitInclude[IncludeExistsSELimitSpellGroup] = true;
					if (focus_spell.base_value[i] == spell.spell_group) { // Include
						LimitInclude[IncludeFoundSELimitSpellGroup] = true;
					}
				}
				break;

			case SpellEffect::LimitCastingSkill:
				if (focus_spell.base_value[i] < 0) {
					if (-focus_spell.base_value[i] == spell.skill) {
						return 0;
					}
				}
				else {
					LimitInclude[IncludeExistsSELimitCastingSkill] = true;
					if (focus_spell.base_value[i] == spell.skill) {
						LimitInclude[IncludeFoundSELimitCastingSkill] = true;
					}
				}
				break;

			case SpellEffect::LimitClass:
				// Do not use this limit more then once per spell. If multiple class, treat value like items
				// would.
				if (!PassLimitClass(focus_spell.base_value[i], GetClass())) {
					return 0;
				}
				break;

			case SpellEffect::LimitRace:
				if (focus_spell.base_value[i] != GetRace()) {
					return 0;
				}
				break;

			case SpellEffect::LimitUseMin:
				if (focus_spell.base_value[i] > spell.hit_number) {
					return 0;
				}
				break;

			case SpellEffect::LimitUseType:
				if (focus_spell.base_value[i] != spell.hit_number_type) {
					return 0;
				}
				break;

			case SpellEffect::CastonFocusEffect:
				if (focus_spell.base_value[i] > 0) {
					Caston_spell_id = focus_spell.base_value[i];
				}
				break;

			case SpellEffect::LimitSpellClass:
				if (focus_spell.base_value[i] < 0) { // Exclude
					if (-focus_spell.base_value[i] == spell.spell_class) {
						return 0;
					}
				}
				else {
					LimitInclude[IncludeExistsSELimitSpellClass] = true;
					if (focus_spell.base_value[i] == spell.spell_class) { // Include
						LimitInclude[IncludeFoundSELimitSpellClass] = true;
					}
				}
				break;

			case SpellEffect::LimitSpellSubclass:
				if (focus_spell.base_value[i] < 0) { // Exclude
					if (-focus_spell.base_value[i] == spell.spell_subclass) {
						return 0;
					}
				}
				else {
					LimitInclude[IncludeExistsSELimitSpellSubclass] = true;
					if (focus_spell.base_value[i] == spell.spell_subclass) { // Include
						LimitInclude[IncludeFoundSELimitSpellSubclass] = true;
					}
				}
				break;

			case SpellEffect::Ff_Same_Caster://hmm do i need to pass casterid from buff slot here
				if (focus_spell.base_value[i] == 0) {
					if (caster && casterid == caster->GetID()) {
						return 0;
					}//Mob casting is same as target, fail if you are casting on yourself.
				}
				else if (focus_spell.base_value[i] == 1) {
					if (caster && casterid != caster->GetID()) {
						return 0;
					}//Mob casting is not same as target, fail if you are not casting on yourself.
				}
				break;

			case SpellEffect::Ff_CasterClass: {

				// Do not use this limit more then once per spell. If multiple class, treat value like items would.
				if (caster && !PassLimitClass(focus_spell.base_value[i], caster->GetClass())) {
					return 0;
				}
				break;
			}

			case SpellEffect::Ff_DurationMax:
				if (focus_spell.base_value[i] > spell.buff_duration) {
					return 0;
				}
				break;

			case SpellEffect::Ff_Endurance_Min:
				if (spell.endurance_cost < focus_spell.base_value[i]) {
					return 0;
				}
				break;

			case SpellEffect::Ff_Endurance_Max:
				if (spell.endurance_cost > focus_spell.base_value[i]) {
					return 0;
				}
				break;

			case SpellEffect::Ff_ReuseTimeMin:
				if (spell.recast_time < focus_spell.base_value[i]) {
					return 0;
				}
				break;

			case SpellEffect::Ff_ReuseTimeMax:
				if (spell.recast_time > focus_spell.base_value[i]) {
					return 0;
				}
				break;

			case SpellEffect::Ff_Value_Min:
				index_id = GetSpellEffectIndex(spell_id, focus_spell.limit_value[i]);
				if (index_id >= 0 && spell.base_value[index_id] < focus_spell.base_value[i]) {
					return 0;
				}
				break;

			case SpellEffect::Ff_Value_Max:
				index_id = GetSpellEffectIndex(spell_id, focus_spell.limit_value[i]);
				if (index_id >= 0 && spell.base_value[index_id] > focus_spell.base_value[i]) {
					return 0;
				}
				break;

			case SpellEffect::Ff_FocusTimerMin:
				if (IsFocusProcLimitTimerActive(focus_spell.id)) {
					return 0;
				}
				else {
					focus_reuse_time = focus_spell.limit_value[i];
				}
				break;

			case SpellEffect::FFItemClass:

				/*
					Limits focuses to check if cast from item clicks. Can be used to INCLUDE or EXCLUDE items by ItemType and/or SubType and/or Slots
					Not used on live, going on information we have plus implemented as broadly as possible to allow all possible options.
					base = item table field 'ItemType' Limit = item table field 'SubType' Max = item table field 'Slots' (this is slot bitmask)

					When including: Setting base, limit, max respectively to -1 will cause it to ignore that check, letting any type or slot ect be used.

					Special rules for excluding. base value needs to be negative < -1, if excluding all ItemTypes set to -1000.
					For SubType and Slots set using same rules above as for includes. Ie. -1 for all, positive for specifics
					To exclude a specific ItemType we have to do some math. The exclude value will be the negative value of (ItemType + 100).
					If ItemType = 10, then SET ItemType= -110 to exclude. If its ItemType 0, then SET ItemType= -100 to exclude ect. Not ideal but it works.

					Usage example: [INCLUDE] Only focus spell if from click cast and is a 'defense armor' item type=10 [base= 10, limit= -1, max= -1]
					Usage example: [INCLUDE] Only focus spell if from click cast and is from helmet slot' slots= 4     [base= -1, limit= -1, max= 4]
					Usage example: [EXCLUDE] Do not focus spell if it is from an item click. [base= -1000, limit= -1, max= -1]
					Usage example: [EXCLUDE] Do not focus spell if it is from an item click from a helmet slot. [base= -1000, limit= -1, max= 4]
					Usage example: [EXCLUDE] Do not focus spell if it is from an item click and is a 'defense armor' item type=10. [base= -110, limit= -1, max= -1]

					Note: You can apply multiple includes or excludes to a single focus spell,  using multiple SPA 415 limits in the spell. Ie. Check for clicks from ItemType 10 or 11.

				*/
				has_item_limit_check = true;
				if (casting_spell_inventory_slot && casting_spell_inventory_slot != -1) {
					if (IsClient() && casting_spell_slot == EQ::spells::CastingSlot::Item && casting_spell_inventory_slot != 0xFFFFFFFF) {
						auto item = CastToClient()->GetInv().GetItem(casting_spell_inventory_slot);
						if (item && item->GetItem()) {
							//If ItemType set to < -1, then we will exclude either all Subtypes (-1000), or specific items by ItemType, SubType or Slot. See above for rules.
							if (focus_spell.base_value[i] < -1) { //Excludes
								bool exclude_this_item = true;
								int tmp_itemtype = (item->GetItem()->ItemType + 100) * -1;
								//ItemType (if set to -1000, ignore and exclude any ItemType)
								if (focus_spell.base_value[i] < -1 && focus_spell.base_value[i] != -1000) {
									if (focus_spell.base_value[i] != tmp_itemtype) {
										exclude_this_item = false;
									}
								}
								//SubType (if set to -1, ignore and exclude all SubTypes)
								if (focus_spell.limit_value[i] >= 0) {
									if (focus_spell.limit_value[i] != item->GetItem()->SubType) {
										exclude_this_item = false;
									}
								}
								//item slot bitmask (if set to -1, ignore and exclude all SubTypes)
								if (focus_spell.max_value[i] >= 0) {
									if (focus_spell.max_value[i] != item->GetItem()->Slots) {
										exclude_this_item = false;
									}
								}
								if (exclude_this_item) {
									return 0;
								}
							}
							else {//Includes
								LimitInclude[IncludeExistsSEFFItemClass] = true;
								bool include_this_item = true;
								//ItemType (if set to -1, ignore and include any ItemType)
								if (focus_spell.base_value[i] >= 0) {
									if (focus_spell.base_value[i] != item->GetItem()->ItemType) {
										include_this_item = false;
									}
								}
								//SubType (if set to -1, ignore and include any SubType)
								if (focus_spell.limit_value[i] >= 0) {
									if (focus_spell.limit_value[i] != item->GetItem()->SubType) {
										include_this_item = false;
									}
								}
								//item slot bitmask (if set to -1, ignore and include any slot)
								if (focus_spell.max_value[i] >= 0) {
									if (focus_spell.max_value[i] != item->GetItem()->Slots) {
										include_this_item = false;
									}
								}

								if (include_this_item) {
									LimitInclude[IncludeFoundSEFFItemClass] = true;
								}
							}
						}
					}
				}
				//If this is checking that focus can only be cast from an item, then if its not cast from item fail.
				else if (focus_spell.base_value[i] >= -1) {
					return 0;
				}
				//If we are checking to exclude items from a focus then do not fail unless the above check fails.
				break;

			// handle effects
			case SpellEffect::ImprovedDamage:
			case SpellEffect::ImprovedDamage2:
				if (!RuleB(Spells, UseClassicSpellFocus)) {
					if (type == focusImprovedDamage || type == focusImprovedDamage2) {
						value = GetFocusRandomEffectivenessValue(focus_spell.base_value[i], focus_spell.limit_value[i], best_focus);
					}
					break;
				} else {
					if (best_focus) {
						value = focus_spell.base_value[i];
					} else {
						value = zone->random.Int(1, focus_spell.base_value[i]);
					}
				}

			case SpellEffect::Fc_Amplify_Mod:
				if (type == focusFcAmplifyMod && focus_spell.base_value[i] > value) {
					value = focus_spell.base_value[i];
				}
				break;

			case SpellEffect::ImprovedHeal:
			case SpellEffect::ReduceManaCost:
				if (!RuleB(Spells, UseClassicSpellFocus)) {
					if (type == focusImprovedHeal || type == focusManaCost) {
						value = GetFocusRandomEffectivenessValue(focus_spell.base_value[i], focus_spell.limit_value[i], best_focus);
					}
					break;
				}

				if (best_focus) {
					value = focus_spell.base_value[i];
				} else {
					value = zone->random.Int(1, focus_spell.base_value[i]);
				}
				break;

			case SpellEffect::IncreaseSpellHaste:
				if (type == focusSpellHaste && focus_spell.base_value[i] > value) {
					value = focus_spell.base_value[i];
					try_apply_to_item_click = is_from_item_click ? true : false;
				}
				break;

			case SpellEffect::Fc_CastTimeMod2:
				if (type == focusFcCastTimeMod2 && focus_spell.base_value[i] > value) {
					value = focus_spell.base_value[i];
					try_apply_to_item_click = is_from_item_click ? true : false;
				}
				break;

			case SpellEffect::Fc_CastTimeAmt:
				if (type == focusFcCastTimeAmt && focus_spell.base_value[i] > value) {
					value = focus_spell.base_value[i];
					try_apply_to_item_click = is_from_item_click ? true : false;
				}
				break;

			case SpellEffect::IncreaseSpellDuration:
				if (type == focusSpellDuration && focus_spell.base_value[i] > value) {
					value = focus_spell.base_value[i];
				}
				break;

			case SpellEffect::SpellDurationIncByTic:
				if (type == focusSpellDurByTic && focus_spell.base_value[i] > value) {
					value = focus_spell.base_value[i];
				}
				break;

			case SpellEffect::SwarmPetDuration:
				if (type == focusSwarmPetDuration && focus_spell.base_value[i] > value) {
					value = focus_spell.base_value[i];
				}
				break;

			case SpellEffect::IncreaseRange:
				if (type == focusRange && focus_spell.base_value[i] > value) {
					value = focus_spell.base_value[i];
				}
				break;

			case SpellEffect::ReduceReagentCost:
				if (!RuleB(Spells, UseClassicSpellFocus)) {
					if (type == focusReagentCost) {
						value = GetFocusRandomEffectivenessValue(focus_spell.base_value[i], focus_spell.limit_value[i], best_focus);
					}
					break;
				}

				if (type == focusReagentCost && focus_spell.base_value[i] > value) {
					value = focus_spell.base_value[i];
				}
				break;

			case SpellEffect::PetPowerIncrease:
				if (type == focusPetPower && focus_spell.base_value[i] > value) {
					value = focus_spell.base_value[i];
				}
				break;

			case SpellEffect::SpellResistReduction:
				if (type == focusResistRate) {
					value = GetFocusRandomEffectivenessValue(focus_spell.base_value[i], focus_spell.limit_value[i], best_focus);
				}
				break;

			case SpellEffect::Fc_ResistIncoming:
				if (type == focusFcResistIncoming && focus_spell.base_value[i] > value) {
					value = focus_spell.base_value[i];
				}
				break;

			case SpellEffect::SpellHateMod:
				if (!RuleB(Spells, UseClassicSpellFocus)) {
					if (type == focusSpellHateMod) {
						value = GetFocusRandomEffectivenessValue(focus_spell.base_value[i], focus_spell.limit_value[i], best_focus);
					}
					break;
				}

				if (type == focusSpellHateMod) {
					if (best_focus) {
						value = focus_spell.base_value[i];
					} else {
						value = zone->random.Int(1, focus_spell.base_value[i]);
					}
				}
				break;

			case SpellEffect::ReduceReuseTimer:
				if (type == focusReduceRecastTime) {
					value = focus_spell.base_value[i] / 1000;
					try_apply_to_item_click = is_from_item_click ? true : false;
				}
				break;

			case SpellEffect::TriggerOnCast:
				if (type == focusTriggerOnCast) {
					if (zone->random.Roll(focus_spell.base_value[i])) {
						value = focus_spell.limit_value[i];
					}
					else {
						value = 0;
					}
				}
				break;

			case SpellEffect::BlockNextSpellFocus:
				if (type == focusBlockNextSpell) {
					if (zone->random.Roll(focus_spell.base_value[i])) {
						value = 1;
					}
				}
				break;

			case SpellEffect::SympatheticProc:
				if (type == focusSympatheticProc) {
					value = focus_id;
				}
				break;

			case SpellEffect::FcSpellVulnerability:
				if (type == focusSpellVulnerability) {
					value = GetFocusRandomEffectivenessValue(focus_spell.base_value[i], focus_spell.limit_value[i], best_focus);
				}
				break;

			case SpellEffect::Fc_Spell_Damage_Pct_IncomingPC:
				if (type == focusFcSpellDamagePctIncomingPC) {
					value = GetFocusRandomEffectivenessValue(focus_spell.base_value[i], focus_spell.limit_value[i], best_focus);
				}
				break;

			case SpellEffect::FcTwincast:
				if (type == focusTwincast && !IsEffectInSpell(spell_id, SpellEffect::TwinCastBlocker)) {
					value = focus_spell.base_value[i];
				}
				break;

			case SpellEffect::FcDamageAmt:
				if (type == focusFcDamageAmt) {
					value = focus_spell.base_value[i];
				}
				break;

			case SpellEffect::FcDamageAmt2:
				if (type == focusFcDamageAmt2) {
					value = focus_spell.base_value[i];
				}
				break;

			case SpellEffect::Fc_Amplify_Amt:
				if (type == focusFcAmplifyAmt) {
					value = focus_spell.base_value[i];
				}
				break;

			case SpellEffect::FcDamageAmtCrit:
				if (type == focusFcDamageAmtCrit) {
					value = focus_spell.base_value[i];
				}
				break;

			case SpellEffect::FcDamageAmtIncoming:
				if (type == focusFcDamageAmtIncoming) {
					value = focus_spell.base_value[i];
				}
				break;

			case SpellEffect::Fc_Spell_Damage_Amt_IncomingPC:
				if (type == focusFcSpellDamageAmtIncomingPC) {
					value = focus_spell.base_value[i];
				}
				break;

			case SpellEffect::FcHealAmtIncoming:
				if (type == focusFcHealAmtIncoming) {
					value = focus_spell.base_value[i];
				}
				break;

			case SpellEffect::FcDamagePctCrit:
				if (type == focusFcDamagePctCrit) {
					value = focus_spell.base_value[i];
				}
				break;

			case SpellEffect::FcHealPctCritIncoming:
				if (type == focusFcHealPctCritIncoming) {
					value = GetFocusRandomEffectivenessValue(focus_spell.base_value[i], focus_spell.limit_value[i], best_focus);
				}
				break;

			case SpellEffect::FcHealAmtCrit:
				if (type == focusFcHealAmtCrit) {
					value = focus_spell.base_value[i];
				}
				break;

			case SpellEffect::FcHealAmt:
				if (type == focusFcHealAmt) {
					value = focus_spell.base_value[i];
				}
				break;

			case SpellEffect::FcHealPctIncoming:
				if (type == focusFcHealPctIncoming) {
					value = GetFocusRandomEffectivenessValue(focus_spell.base_value[i], focus_spell.limit_value[i], best_focus);
				}
				break;

			case SpellEffect::FcBaseEffects:
				if (type == focusFcBaseEffects) {
					value = focus_spell.base_value[i];
				}
				break;

			case SpellEffect::FcIncreaseNumHits:
				if (type == focusIncreaseNumHits) {
					value = focus_spell.base_value[i];
				}
				break;

			case SpellEffect::FcLimitUse:
				if (type == focusFcLimitUse) {
					value = focus_spell.base_value[i];
				}
				break;

			case SpellEffect::FcMute:
				if (type == focusFcMute) {
					value = focus_spell.base_value[i];
				}
				break;

			case SpellEffect::FcStunTimeMod:
				if (type == focusFcStunTimeMod) {
					value = focus_spell.base_value[i];
				}
				break;

			case SpellEffect::FcTimerRefresh:
				if (type == focusFcTimerRefresh) {
					value = focus_spell.base_value[i];
				}
				break;

			case SpellEffect::FcTimerLockout:
				if (type == focusFcTimerLockout) {
					value = focus_spell.base_value[i];
				}
				break;

			case SpellEffect::Fc_Cast_Spell_On_Land:
				if (type == focusFcCastSpellOnLand) {
					if (zone->random.Roll(focus_spell.base_value[i])) {
						value = focus_spell.limit_value[i];
					}
					break;
				}

#if EQDEBUG >= 6
				// this spits up a lot of garbage when calculating spell focuses
				// since they have all kinds of extra effects on them.
				default:
					LogInfo("unknown effectid [{}]",
						focus_spell.effect_id[i]);
#endif
		}
	}

	for (int e = 0; e < MaxLimitInclude; e += 2) {
		if (LimitInclude[e] && !LimitInclude[e + 1]) {
			return 0;
		}
	}

	/*
		For item click cast/recast focus modifiers. Only use if SPA 415 exists.
		This is an item click but does not have SPA 415 limiter. Fail here.
	*/

	if (try_apply_to_item_click && !has_item_limit_check) {
		return 0;
	}

	if (Caston_spell_id) {
		if (IsValidSpell(Caston_spell_id) && (Caston_spell_id != spell_id)) {
			SpellFinished(
				Caston_spell_id,
				this,
				EQ::spells::CastingSlot::Item,
				0,
				-1,
				spells[Caston_spell_id].resist_difficulty
			);
		}
	}

	if (focus_reuse_time) {
		SetFocusProcLimitTimer(focus_spell.id, focus_reuse_time);
	}

	return (value * lvlModifier / 100);
}

void Mob::TryTriggerOnCastFocusEffect(focusType type, uint16 spell_id)
{
	if (IsBardSong(spell_id)) {
		return;
	}

	if (!IsValidSpell(spell_id)) {
		return;
	}

	int32 focus_spell_id = 0;
	int32 proc_spellid   = 0;

	// item focus
	if (IsOfClientBot() && itembonuses.FocusEffects[type]) {
		const EQ::ItemData *temp_item = nullptr;

		for (int x = EQ::invslot::EQUIPMENT_BEGIN; x <= EQ::invslot::EQUIPMENT_END; x++) {
			temp_item = nullptr;
			EQ::ItemInstance const *ins = (IsClient()) ? CastToClient()->GetInv().GetItem(x) : CastToBot()->GetBotItem(x);
			if (!ins) {
				continue;
			}
			temp_item = ins->GetItem();
			if (temp_item && temp_item->Focus.Effect > 0 && IsValidSpell(temp_item->Focus.Effect)) {
				focus_spell_id = temp_item->Focus.Effect;
				if (!IsEffectInSpell(focus_spell_id, SpellEffect::TriggerOnCast)) {
					continue;
				}

				proc_spellid = CalcFocusEffect(type, focus_spell_id, spell_id);
				if (proc_spellid) {
					TryTriggerOnCastProc(focus_spell_id, spell_id, proc_spellid);
				}
			}

			for (int y = EQ::invaug::SOCKET_BEGIN; y <= EQ::invaug::SOCKET_END; ++y) {
				EQ::ItemInstance *aug = ins->GetAugment(y);
				if (aug) {
					const EQ::ItemData *temp_item_aug = aug->GetItem();
					if (temp_item_aug && temp_item_aug->Focus.Effect > 0 && IsValidSpell(temp_item_aug->Focus.Effect)) {
						focus_spell_id = temp_item_aug->Focus.Effect;

						if (!IsEffectInSpell(focus_spell_id, SpellEffect::TriggerOnCast)) {
							continue;
						}

						proc_spellid = CalcFocusEffect(type, focus_spell_id, spell_id);
						if (proc_spellid) {
							TryTriggerOnCastProc(focus_spell_id, spell_id, proc_spellid);
						}
					}
				}
			}
		}
	}

	// Spell Focus
	if (spellbonuses.FocusEffects[type]) {
		int buff_slot = 0;
		for (buff_slot = 0; buff_slot < GetMaxTotalSlots(); buff_slot++) {
			focus_spell_id = buffs[buff_slot].spellid;
			if (!IsValidSpell(focus_spell_id)) {
				continue;
			}

			if (!IsEffectInSpell(focus_spell_id, SpellEffect::TriggerOnCast)) {
				continue;
			}

			proc_spellid = CalcFocusEffect(type, focus_spell_id, spell_id);
			if (proc_spellid) {
				TryTriggerOnCastProc(focus_spell_id, spell_id, proc_spellid);
				CheckNumHitsRemaining(NumHit::MatchingSpells, buff_slot);
			}
		}
	}

	// Only use one of this focus per AA effect.
	if (IsOfClientBot() && aabonuses.FocusEffects[type]) {
		for (const auto &aa : aa_ranks) {
			auto ability_rank = zone->GetAlternateAdvancementAbilityAndRank(aa.first, aa.second.first);
			auto ability      = ability_rank.first;
			auto rank         = ability_rank.second;

			if (!ability) {
				continue;
			}

			if (rank->effects.empty()) {
				continue;
			}

			proc_spellid = CalcAAFocus(type, *rank, spell_id);
			if (proc_spellid) {
				TryTriggerOnCastProc(0, spell_id, proc_spellid);
			}
		}
	}
}

bool Mob::TryTriggerOnCastProc(uint16 focusspellid, uint16 spell_id, uint16 proc_spellid)
{
	// We confirm spell_id and focuspellid are valid before passing into this.
	if (IsValidSpell(proc_spellid) && spell_id != focusspellid && spell_id != proc_spellid) {
		Mob* proc_target = GetTarget();
		if (proc_target) {
			SpellFinished(proc_spellid, proc_target, EQ::spells::CastingSlot::Item, 0, -1, spells[proc_spellid].resist_difficulty);
			return true;
		}
		// Edge cases where proc spell does not require a target such as PBAE, allows proc to still occur even if target potentially dead. Live spells exist with PBAE procs.
		else if (!IsTargetRequiredForSpell(proc_spellid)) {
			SpellFinished(proc_spellid, this, EQ::spells::CastingSlot::Item, 0, -1, spells[proc_spellid].resist_difficulty);
			return true;
		}
	}
	return false;
}

uint16 Mob::GetSympatheticFocusEffect(focusType type, uint16 spell_id) {

	if (IsBardSong(spell_id))
		return 0;

	uint16 proc_spellid = 0;
	float ProcChance = 0.0f;

	std::vector<int> SympatheticProcList;

	//item focus
	if (IsOfClientBot() && itembonuses.FocusEffects[type]) {


		for (int x = EQ::invslot::EQUIPMENT_BEGIN; x <= EQ::invslot::EQUIPMENT_END; x++)
		{
			if (SympatheticProcList.size() > MAX_SYMPATHETIC_PROCS)
				continue;

			EQ::ItemInstance const* ins = GetInv().GetItem(x);
			if (!ins)
				continue;
			if (
					auto TempItem = ins->GetItem();
					TempItem && TempItem->Focus.Effect > 0 && IsValidSpell(TempItem->Focus.Effect)
			) {

				proc_spellid = CalcFocusEffect(type, TempItem->Focus.Effect, spell_id);
				if (IsValidSpell(proc_spellid)){
					ProcChance = GetSympatheticProcChances(spell_id, spells[TempItem->Focus.Effect].base_value[0], TempItem->ProcRate);
					if(zone->random.Roll(ProcChance))
 						SympatheticProcList.push_back(proc_spellid);
				}
			}

			for (int y = EQ::invaug::SOCKET_BEGIN; y <= EQ::invaug::SOCKET_END; ++y)
			{
				if (SympatheticProcList.size() > MAX_SYMPATHETIC_PROCS)
					continue;

				auto aug = ins->GetAugment(y);
				if (aug)
				{
					auto TempItemAug = aug->GetItem();
					if (TempItemAug && TempItemAug->Focus.Effect > 0 && IsValidSpell(TempItemAug->Focus.Effect)) {
						proc_spellid = CalcFocusEffect(type, TempItemAug->Focus.Effect, spell_id);
						if (IsValidSpell(proc_spellid)){
							ProcChance = GetSympatheticProcChances(spell_id, spells[TempItemAug->Focus.Effect].base_value[0], TempItemAug->ProcRate);
							if(zone->random.Roll(ProcChance))
								SympatheticProcList.push_back(proc_spellid);
						}
					}
				}
			}
		}
	}

	//Spell Focus
	if (spellbonuses.FocusEffects[type]){
		uint16 focusspellid = 0;
		int buff_max = GetMaxTotalSlots();
		for (int buff_slot = 0; buff_slot < buff_max; buff_slot++) {

			if (SympatheticProcList.size() > MAX_SYMPATHETIC_PROCS) {
				continue;
			}

			focusspellid = buffs[buff_slot].spellid;
			if (!IsValidSpell(focusspellid)) {
				continue;
			}

				proc_spellid = CalcFocusEffect(type, focusspellid, spell_id);

			if (IsValidSpell(proc_spellid)) {

				ProcChance = GetSympatheticProcChances(spell_id, GetSympatheticSpellProcRate(proc_spellid));
				if (zone->random.Roll(ProcChance)) {
					SympatheticProcList.push_back(proc_spellid);
				}
			}
		}
	}

	/*Note: At present, ff designing custom AA to have a sympathetic proc effect, only use one focus
	effect within the aa_effects data for each AA*[No live AA's use this effect to my knowledge]*/
	if (IsOfClientBot() && aabonuses.FocusEffects[type]) {
		for (const auto &aa : aa_ranks) {
			if (SympatheticProcList.size() > MAX_SYMPATHETIC_PROCS) {
				break;
			}

			auto ability_rank = zone->GetAlternateAdvancementAbilityAndRank(aa.first, aa.second.first);
			auto ability = ability_rank.first;
			auto rank = ability_rank.second;

			if (!ability) {
				continue;
			}

			if (rank->effects.empty()) {
				continue;
			}
			proc_spellid = CalcAAFocus(type, *rank, spell_id);
			if (IsValidSpell(proc_spellid)) {
				ProcChance = GetSympatheticProcChances(spell_id, rank->effects[0].base_value);
				if (zone->random.Roll(ProcChance)) {
					SympatheticProcList.push_back(proc_spellid);
				}
			}
		}
	}

	if (!SympatheticProcList.empty())
	{
		uint8 random = zone->random.Int(0, SympatheticProcList.size()-1);
		int FinalSympatheticProc = SympatheticProcList[random];
		SympatheticProcList.clear();
		return FinalSympatheticProc;
	}
	return 0;
}

int64 Mob::GetFocusEffect(focusType type, uint16 spell_id, Mob *caster, bool from_buff_tic)
{
	if (IsBardSong(spell_id) && type != focusFcBaseEffects && type != focusSpellDuration && type != focusReduceRecastTime) {
		return 0;
	}

	int64 realTotal = 0;
	int64 realTotal2 = 0;
	int64 realTotal3 = 0;

	bool rand_effectiveness = false;

	//Improved Healing, Damage & Mana Reduction are handled differently in that some are random percentages
	//In these cases we need to find the most powerful effect, so that each piece of gear wont get its own chance
	if (RuleB(Spells, LiveLikeFocusEffects) && CanFocusUseRandomEffectivenessByType(type)) {
		rand_effectiveness = true;
	}

	//Check if item focus effect exists for the mob.
	if (itembonuses.FocusEffects[type]) {

		const EQ::ItemData* TempItem = nullptr;
		const EQ::ItemData* UsedItem = nullptr;
		uint16 UsedFocusID = 0;
		int32 Total = 0;
		int32 focus_max = 0;
		int32 focus_max_real = 0;

		//item focus
		for (int x = EQ::invslot::EQUIPMENT_BEGIN; x <= EQ::invslot::EQUIPMENT_END; x++) {
			TempItem = nullptr;
			EQ::ItemInstance* ins = GetInv().GetItem(x);
			if (!ins) {
				continue;
			}

			TempItem = ins->GetItem();
			if (TempItem && IsValidSpell(TempItem->Focus.Effect)) {
				if(rand_effectiveness) {
					focus_max = CalcFocusEffect(type, TempItem->Focus.Effect, spell_id, true);
					if (focus_max > 0 && focus_max_real >= 0 && focus_max > focus_max_real) {
						focus_max_real = focus_max;
						UsedItem = TempItem;
						UsedFocusID = TempItem->Focus.Effect;
					} else if (focus_max < 0 && focus_max < focus_max_real) {
						focus_max_real = focus_max;
						UsedItem = TempItem;
						UsedFocusID = TempItem->Focus.Effect;
					}
				}
				else {
					Total = CalcFocusEffect(type, TempItem->Focus.Effect, spell_id);
					if (Total > 0 && realTotal >= 0 && Total > realTotal) {
						realTotal = Total;
						UsedItem = TempItem;
						UsedFocusID = TempItem->Focus.Effect;
					} else if (Total < 0 && Total < realTotal) {
						realTotal = Total;
						UsedItem = TempItem;
						UsedFocusID = TempItem->Focus.Effect;
					}
				}
			}

			for (int y = EQ::invaug::SOCKET_BEGIN; y <= EQ::invaug::SOCKET_END; ++y) {
				EQ::ItemInstance *aug = nullptr;
				aug = ins->GetAugment(y);
				if (aug) {
					const EQ::ItemData* TempItemAug = aug->GetItem();
					if (TempItemAug && IsValidSpell(TempItemAug->Focus.Effect)) {
						if(rand_effectiveness) {
							focus_max = CalcFocusEffect(type, TempItemAug->Focus.Effect, spell_id, true);
							if (focus_max > 0 && focus_max_real >= 0 && focus_max > focus_max_real) {
								focus_max_real = focus_max;
								UsedItem = TempItem;
								UsedFocusID = TempItemAug->Focus.Effect;
							} else if (focus_max < 0 && focus_max < focus_max_real) {
								focus_max_real = focus_max;
								UsedItem = TempItem;
								UsedFocusID = TempItemAug->Focus.Effect;
							}
						}
						else {
							Total = CalcFocusEffect(type, TempItemAug->Focus.Effect, spell_id);
							if (Total > 0 && realTotal >= 0 && Total > realTotal) {
								realTotal = Total;
								UsedItem = TempItem;
								UsedFocusID = TempItemAug->Focus.Effect;
							} else if (Total < 0 && Total < realTotal) {
								realTotal = Total;
								UsedItem = TempItem;
								UsedFocusID = TempItemAug->Focus.Effect;
							}
						}
					}
				}
			}
		}

		if (IsClient()) {
			//Client Tribute Focus
			for (int x = EQ::invslot::TRIBUTE_BEGIN; x <= EQ::invslot::TRIBUTE_END; ++x) {
				TempItem = nullptr;
				EQ::ItemInstance* ins = GetInv().GetItem(x);
				if (!ins) {
					continue;
				}

				TempItem = ins->GetItem();
				if (TempItem && IsValidSpell(TempItem->Focus.Effect)) {
					if (rand_effectiveness) {
						focus_max = CalcFocusEffect(type, TempItem->Focus.Effect, spell_id, true);
						if (focus_max > 0 && focus_max_real >= 0 && focus_max > focus_max_real) {
							focus_max_real = focus_max;
							UsedItem = TempItem;
							UsedFocusID = TempItem->Focus.Effect;
						}
						else if (focus_max < 0 && focus_max < focus_max_real) {
							focus_max_real = focus_max;
							UsedItem = TempItem;
							UsedFocusID = TempItem->Focus.Effect;
						}
					}
					else {
						Total = CalcFocusEffect(type, TempItem->Focus.Effect, spell_id);
						if (Total > 0 && realTotal >= 0 && Total > realTotal) {
							realTotal = Total;
							UsedItem = TempItem;
							UsedFocusID = TempItem->Focus.Effect;
						}
						else if (Total < 0 && Total < realTotal) {
							realTotal = Total;
							UsedItem = TempItem;
							UsedFocusID = TempItem->Focus.Effect;
						}
					}
				}
			}
		}

		if (IsClient()) {
			//Client Guild Tribute Focus
			for (int x = EQ::invslot::GUILD_TRIBUTE_BEGIN; x <= EQ::invslot::GUILD_TRIBUTE_END; ++x) {
				TempItem = nullptr;
				EQ::ItemInstance* ins = GetInv().GetItem(x);
				if (!ins) {
					continue;
				}

				TempItem = ins->GetItem();
				if (TempItem && IsValidSpell(TempItem->Focus.Effect)) {
					if (rand_effectiveness) {
						focus_max = CalcFocusEffect(type, TempItem->Focus.Effect, spell_id, true);
						if (focus_max > 0 && focus_max_real >= 0 && focus_max > focus_max_real) {
							focus_max_real = focus_max;
							UsedItem = TempItem;
							UsedFocusID = TempItem->Focus.Effect;
						}
						else if (focus_max < 0 && focus_max < focus_max_real) {
							focus_max_real = focus_max;
							UsedItem = TempItem;
							UsedFocusID = TempItem->Focus.Effect;
						}
					}
					else {
						Total = CalcFocusEffect(type, TempItem->Focus.Effect, spell_id);
						if (Total > 0 && realTotal >= 0 && Total > realTotal) {
							realTotal = Total;
							UsedItem = TempItem;
							UsedFocusID = TempItem->Focus.Effect;
						}
						else if (Total < 0 && Total < realTotal) {
							realTotal = Total;
							UsedItem = TempItem;
							UsedFocusID = TempItem->Focus.Effect;
						}
					}
				}
			}
		}

		if (UsedItem && rand_effectiveness && focus_max_real != 0) {
			realTotal = CalcFocusEffect(type, UsedFocusID, spell_id);
		}

		if ((rand_effectiveness && UsedItem) || (realTotal != 0 && UsedItem)) {
			// there are a crap ton more of these, I was able to verify these ones though
			// the RNG effective ones appear to have a different message for failing to focus
			uint32 string_id = BEGINS_TO_GLOW; // this is really just clicky message ...
			switch (type) {
			case focusSpellHaste:
				string_id = SHIMMERS_BRIEFLY;
				break;
			case focusManaCost: // this might be GROWS_DIM for fail
				string_id = FLICKERS_PALE_LIGHT;
				break;
			case focusSpellDuration:
				string_id = SPARKLES;
				break;
			case focusImprovedDamage:
			case focusImprovedDamage2:
				if (realTotal)
					string_id = ALIVE_WITH_POWER;
				else
					string_id = SEEMS_DRAINED;
				break;
			case focusRange:
				string_id = PULSES_WITH_LIGHT;
				break;
			case focusSpellHateMod: // GLOWS_RED for increasing hate
				string_id = GLOWS_BLUE;
				break;
			case focusImprovedHeal:
				if (realTotal)
					string_id = FEEDS_WITH_POWER;
				else
					string_id = POWER_DRAIN_INTO;
				break;
			case focusReagentCost: // this might be GROWS_DIM for fail as well ...
				string_id = BEGINS_TO_SHINE;
				break;
			default:
				break;
			}
			MessageString(Chat::FocusEffect, string_id, UsedItem->Name);
		}
	}

	//Check if spell focus effect exists for the mob.
	if (spellbonuses.FocusEffects[type]) {

		//Spell Focus
		int32 Total2 = 0;
		int32 focus_max2 = 0;
		int32 focus_max_real2 = 0;

		int buff_tracker = -1;
		int buff_slot = 0;
		int32 focusspellid = 0;
		int32 focusspell_tracker = 0;
		int buff_max = GetMaxTotalSlots();
		for (buff_slot = 0; buff_slot < buff_max; buff_slot++) {
			focusspellid = buffs[buff_slot].spellid;
			if (focusspellid == 0 || focusspellid >= SPDAT_RECORDS) {
				continue;
			}

			if (rand_effectiveness) {
				focus_max2 = CalcFocusEffect(type, focusspellid, spell_id, true, buffs[buff_slot].casterid, caster);
				if (focus_max2 > 0 && focus_max_real2 >= 0 && focus_max2 > focus_max_real2) {
					focus_max_real2 = focus_max2;
					buff_tracker = buff_slot;
					focusspell_tracker = focusspellid;
				} else if (focus_max2 < 0 && focus_max2 < focus_max_real2) {
					focus_max_real2 = focus_max2;
					buff_tracker = buff_slot;
					focusspell_tracker = focusspellid;
				}
			}
			else {
				Total2 = CalcFocusEffect(type, focusspellid, spell_id, false, buffs[buff_slot].casterid, caster);
				if (Total2 > 0 && realTotal2 >= 0 && Total2 > realTotal2) {
					realTotal2 = Total2;
					buff_tracker = buff_slot;
					focusspell_tracker = focusspellid;
				} else if (Total2 < 0 && Total2 < realTotal2) {
					realTotal2 = Total2;
					buff_tracker = buff_slot;
					focusspell_tracker = focusspellid;
				}
			}
		}

		uint16 original_caster_id = 0;
		if (buff_tracker >= 0 && buffs[buff_tracker].casterid > 0) {
			original_caster_id = buffs[buff_tracker].casterid;
		}

		if (focusspell_tracker && rand_effectiveness && focus_max_real2 != 0) {
			realTotal2 = CalcFocusEffect(type, focusspell_tracker, spell_id, false, original_caster_id, caster);
		}

		if (!from_buff_tic && buff_tracker >= 0 && buffs[buff_tracker].hit_number > 0) {
			CheckNumHitsRemaining(NumHit::MatchingSpells, buff_tracker);
		}
	}

	// AA Focus
	if (aabonuses.FocusEffects[type]) {

		int32 Total3 = 0;

		for (const auto &aa : aa_ranks) {
			auto ability_rank = zone->GetAlternateAdvancementAbilityAndRank(aa.first, aa.second.first);
			auto ability = ability_rank.first;
			auto rank = ability_rank.second;

			if (!ability) {
				continue;
			}

			if (rank->effects.empty()) {
				continue;
			}

			Total3 = CalcAAFocus(type, *rank, spell_id);
			if (Total3 > 0 && realTotal3 >= 0 && Total3 > realTotal3) {
				realTotal3 = Total3;
			}
			else if (Total3 < 0 && Total3 < realTotal3) {
				realTotal3 = Total3;
			}
		}
	}

	if (type == focusReagentCost && (IsEffectInSpell(spell_id, SpellEffect::SummonItem) || IsSacrificeSpell(spell_id))) {
		return 0;
	}
	//Summon Spells that require reagents are typically imbue type spells, enchant metal, sacrifice and shouldn't be affected
	//by reagent conservation for obvious reasons.


	int32 worneffect_bonus = 0;
	//Non-Live like feature to allow for an additive focus bonus to be applied from foci that are placed in worn slot. (Limit Checks)
	if (RuleB(Spells, UseAdditiveFocusFromWornSlotWithLimits)) {
		//Check if item focus effect exists for the mob.
		if (itembonuses.FocusEffectsWornWithLimits[type]) {
			const EQ::ItemData* temporary_item = nullptr;
			const EQ::ItemData* used_item      = nullptr;

			//item focus
			for (int x = EQ::invslot::EQUIPMENT_BEGIN; x <= EQ::invslot::EQUIPMENT_END; x++) {
				temporary_item = nullptr;
				EQ::ItemInstance* ins = GetInv().GetItem(x);
				if (!ins) {
					continue;
				}

				temporary_item = ins->GetItem();
				if (temporary_item && IsValidSpell(temporary_item->Worn.Effect)) {
					if (rand_effectiveness) {
						worneffect_bonus += CalcFocusEffect(type, temporary_item->Worn.Effect, spell_id, true);
					}
					else {
						worneffect_bonus += CalcFocusEffect(type, temporary_item->Worn.Effect, spell_id);
					}
				}

				for (int y = EQ::invaug::SOCKET_BEGIN; y <= EQ::invaug::SOCKET_END; ++y) {
					EQ::ItemInstance* aug = nullptr;
					aug = ins->GetAugment(y);
					if (aug) {
						const EQ::ItemData* temporary_item_augment = aug->GetItem();
						if (temporary_item_augment && IsValidSpell(temporary_item_augment->Worn.Effect)) {
							if (rand_effectiveness) {
								worneffect_bonus += CalcFocusEffect(type, temporary_item_augment->Worn.Effect, spell_id, true);
							}
							else {
								worneffect_bonus += CalcFocusEffect(type, temporary_item_augment->Worn.Effect, spell_id);
						    }
						}
					}
				}
			}
		}
	}
	//Non-Live like feature to allow for an additive focus bonus to be applied from foci that are placed in worn slot. (No limit checks)
	else if (RuleB(Spells, UseAdditiveFocusFromWornSlot)) {
		worneffect_bonus = itembonuses.FocusEffectsWorn[type];
	}

	return realTotal + realTotal2 + realTotal3 + worneffect_bonus;
}

int64 NPC::GetFocusEffect(focusType type, uint16 spell_id, Mob* caster, bool from_buff_tic) {

	int64 realTotal = 0;
	int64 realTotal2 = 0;

	bool rand_effectiveness = false;

	//Improved Healing, Damage & Mana Reduction are handled differently in that some are random percentages
	//In these cases we need to find the most powerful effect, so that each piece of gear wont get its own chance
	if (RuleB(Spells, LiveLikeFocusEffects) && CanFocusUseRandomEffectivenessByType(type)) {
		rand_effectiveness = true;
	}

	if (RuleB(Spells, NPC_UseFocusFromItems) && itembonuses.FocusEffects[type]){

		const EQ::ItemData* TempItem = nullptr;
		const EQ::ItemData* UsedItem = nullptr;
		uint16 UsedFocusID = 0;
		int64 Total = 0;
		int64 focus_max = 0;
		int64 focus_max_real = 0;

		//item focus
		for (int i = EQ::invslot::EQUIPMENT_BEGIN; i <= EQ::invslot::EQUIPMENT_END; i++){
			const EQ::ItemData *cur = database.GetItem(equipment[i]);

			if(!cur)
				continue;

			TempItem = cur;

			if (TempItem && IsValidSpell(TempItem->Focus.Effect)) {
				if(rand_effectiveness) {
					focus_max = CalcFocusEffect(type, TempItem->Focus.Effect, spell_id, true);
					if (focus_max > 0 && focus_max_real >= 0 && focus_max > focus_max_real) {
						focus_max_real = focus_max;
						UsedItem = TempItem;
						UsedFocusID = TempItem->Focus.Effect;
					} else if (focus_max < 0 && focus_max < focus_max_real) {
						focus_max_real = focus_max;
						UsedItem = TempItem;
						UsedFocusID = TempItem->Focus.Effect;
					}
				}
				else {
					Total = CalcFocusEffect(type, TempItem->Focus.Effect, spell_id);
					if (Total > 0 && realTotal >= 0 && Total > realTotal) {
						realTotal = Total;
						UsedItem = TempItem;
						UsedFocusID = TempItem->Focus.Effect;
					} else if (Total < 0 && Total < realTotal) {
						realTotal = Total;
						UsedItem = TempItem;
						UsedFocusID = TempItem->Focus.Effect;
					}
				}
			}
		}

		if(UsedItem && rand_effectiveness && focus_max_real != 0)
			realTotal = CalcFocusEffect(type, UsedFocusID, spell_id);
	}

	if ((RuleB(Spells, NPC_UseFocusFromSpells) || IsTargetedFocusEffect(type)) && spellbonuses.FocusEffects[type]){

		//Spell Focus
		int64 Total2 = 0;
		int64 focus_max2 = 0;
		int64 focus_max_real2 = 0;

		int buff_tracker = -1;
		int buff_slot = 0;
		int64 focusspellid = 0;
		int64 focusspell_tracker = 0;
		int buff_max = GetMaxTotalSlots();
		for (buff_slot = 0; buff_slot < buff_max; buff_slot++) {
			focusspellid = buffs[buff_slot].spellid;
			if (focusspellid == 0 || focusspellid >= SPDAT_RECORDS)
				continue;

			if(rand_effectiveness) {
				focus_max2 = CalcFocusEffect(type, focusspellid, spell_id, true, buffs[buff_slot].casterid, caster);
				if (focus_max2 > 0 && focus_max_real2 >= 0 && focus_max2 > focus_max_real2) {
					focus_max_real2 = focus_max2;
					buff_tracker = buff_slot;
					focusspell_tracker = focusspellid;
				} else if (focus_max2 < 0 && focus_max2 < focus_max_real2) {
					focus_max_real2 = focus_max2;
					buff_tracker = buff_slot;
					focusspell_tracker = focusspellid;
				}
			}
			else {
				Total2 = CalcFocusEffect(type, focusspellid, spell_id, false, buffs[buff_slot].casterid, caster);
				if (Total2 > 0 && realTotal2 >= 0 && Total2 > realTotal2) {
					realTotal2 = Total2;
					buff_tracker = buff_slot;
					focusspell_tracker = focusspellid;
				} else if (Total2 < 0 && Total2 < realTotal2) {
					realTotal2 = Total2;
					buff_tracker = buff_slot;
					focusspell_tracker = focusspellid;
				}
			}
		}

		uint16 original_caster_id = 0;
		if (buff_tracker >= 0 && buffs[buff_tracker].casterid > 0) {
			original_caster_id = buffs[buff_tracker].casterid;
		}

		if (focusspell_tracker && rand_effectiveness && focus_max_real2 != 0) {
			realTotal2 = CalcFocusEffect(type, focusspell_tracker, spell_id, false, original_caster_id, caster);
		}

		if(!from_buff_tic && buff_tracker >= 0 && buffs[buff_tracker].hit_number > 0) {
			CheckNumHitsRemaining(NumHit::MatchingSpells, buff_tracker);
		}
	}

	return realTotal + realTotal2;
}

void Mob::CheckNumHitsRemaining(NumHit type, int32 buff_slot, uint16 spell_id)
{
	/*
	Field 175 = numhits type
	1:  [Incoming Hit Attempts] (323=SE_DefensiveProc, 172=SE_AvoidMeleeChance, 1=SE_ArmorClass, 40=SE_DivineAura)
	2:  [Outgoing Hit Attempts]  (185=SE_DamageModifer, 184=SE_HitChance)
	3:  [Incoming Spells]  (180=SE_ResistSpellChance, 296=SE_FcSpellVulnerability) //Note: Determinetal spells only unless proven otherwise
	4:  [Outgoing Spells]
	5:  [Outgoing Hit Successes] (220=SE_SkillDamageAmount, 178=SE_MeleeLifetap, 121=SE_ReverseDS, ?373=SE_CastOnFadeEffectAlways)
	6:  [Incoming Hit Successes] (59=SE_DamageShield, 197=SE_SkillDamageTaken, 162=define SpellEffect::MitigateMeleeDamage)
	7:  [Matching Spells] *When focus is triggered (focus effects)
	8:  [Incoming Hits or Spells] (329=SE_ManaAbsorbPercentDamage)
	9:  [Reflected Spells] If successful
	10: [Defensive Procs] Only count down buff of the proc that executed
	11: [Melee Procs] Only count down the buff of the proc that executed
	*/

	if (!HasNumhits())
		return;

	bool bDepleted = false;
	int buff_max = GetMaxTotalSlots();

	std::string buff_name;
	size_t buff_counter = 0;
	bool buff_update = false;

	//Spell specific procs [Type 7,10,11]
	if (IsValidSpell(spell_id)) {
		for (int d = 0; d < buff_max; d++) {
			if (buffs[d].spellid == spell_id && buffs[d].hit_number > 0 &&
			    spells[buffs[d].spellid].hit_number_type == static_cast<int>(type)) {

				buff_name = spells[buffs[d].spellid].name;
				buff_counter = (buffs[d].hit_number - 1);
				buff_update = true;

				if (--buffs[d].hit_number == 0) {
					CastOnNumHitFade(buffs[d].spellid);
					if (!TryFadeEffect(d))
						BuffFadeBySlot(d, true);
				} else if (IsClient()) { // still have numhits and client, update
					CastToClient()->SendBuffNumHitPacket(buffs[d], d);
				}
			}
		}
	} else if (type == NumHit::MatchingSpells) {
		if (buff_slot >= 0) {
			if (--buffs[buff_slot].hit_number == 0) {
				buff_name = spells[buffs[buff_slot].spellid].name;
				buff_counter = (buffs[buff_slot].hit_number - 1);
				buff_update = true;

				CastOnNumHitFade(buffs[buff_slot].spellid);
				if (!TryFadeEffect(buff_slot))
					BuffFadeBySlot(buff_slot , true);
			} else if (IsClient()) { // still have numhits and client, update
				CastToClient()->SendBuffNumHitPacket(buffs[buff_slot], buff_slot);
			}
		}
	}
	else {
		for (int d = 0; d < buff_max; d++) {
			if (IsValidSpell(buffs[d].spellid) && buffs[d].hit_number > 0 &&
			    spells[buffs[d].spellid].hit_number_type == static_cast<int>(type)) {

				buff_name = spells[buffs[d].spellid].name;
				buff_counter = (buffs[d].hit_number - 1);
				buff_update = true;

				if (--buffs[d].hit_number == 0) {
					CastOnNumHitFade(buffs[d].spellid);
					if (!TryFadeEffect(d))
						BuffFadeBySlot(d, true);
				} else if (IsClient()) { // still have numhits and client, update
					CastToClient()->SendBuffNumHitPacket(buffs[d], d);
				}
			}
		}
	}

	if (IsBot() && buff_update) {
		auto bot_owner = entity_list.GetBotOwnerByBotEntityID(GetID());
		if (bot_owner && bot_owner->GetBotOption(Client::booBuffCounter)) {
			bot_owner->CastToClient()->SendMarqueeMessage(
				Chat::Yellow,
				510,
				0,
				1000,
				3000,
				StringFormat(
					"%s has [%u] hit%s remaining on '%s'",
					GetCleanName(),
					buff_counter,
					(buff_counter == 1 ? "" : "s"),
					buff_name.c_str()
				)
			);
		}
	}
}

//for some stupid reason SK procs return theirs one base off...
uint16 Mob::GetProcID(uint16 spell_id, uint8 effect_index)
{
	if (!RuleB(Spells, SHDProcIDOffByOne)) // UF+ spell files
		return spells[spell_id].base_value[effect_index];

	// We should actually just be checking if the mob is SHD, but to not force
	// custom servers to create new spells, we will still do this
	bool sk = false;
	bool other = false;
	for (int x = 0; x < 16; x++) {
		if (x == 4) {
			if (spells[spell_id].classes[4] < 255)
				sk = true;
		} else {
			if (spells[spell_id].classes[x] < 255)
				other = true;
		}
	}

	if (sk && !other)
		return spells[spell_id].base_value[effect_index] + 1;
	else
		return spells[spell_id].base_value[effect_index];
}

bool Mob::TryDivineSave()
{
	/*
	How Touch of the Divine AA works:
	-Gives chance to avoid death when client is killed.
	-Chance is determined by the sum of AA/item/spell chances.
	-If the chance is met a divine aura like effect 'Touch of the Divine' is applied to the client removing detrimental spell effects.
	-If desired, additional spells can be triggered from the AA/item/spell effect, generally a heal.
	*/

	int32 success_chance = aabonuses.DivineSaveChance[SBIndex::DIVINE_SAVE_CHANCE] + itembonuses.DivineSaveChance[SBIndex::DIVINE_SAVE_CHANCE] + spellbonuses.DivineSaveChance[SBIndex::DIVINE_SAVE_CHANCE];

	if (success_chance && zone->random.Roll(success_chance))
	{
		SetHP(1);

		int32 EffectsToTry[] =
		{
			aabonuses.DivineSaveChance[SBIndex::DIVINE_SAVE_SPELL_TRIGGER_ID],
			itembonuses.DivineSaveChance[SBIndex::DIVINE_SAVE_SPELL_TRIGGER_ID],
			spellbonuses.DivineSaveChance[SBIndex::DIVINE_SAVE_SPELL_TRIGGER_ID]
		};
		//Fade the divine save effect here after saving the old effects off.
		//That way, if desired, the effect could apply SpellEffect::DivineSave again.
		BuffFadeByEffect(SpellEffect::DivineSave);
		for(size_t i = 0; i < ( sizeof(EffectsToTry) / sizeof(EffectsToTry[0]) ); ++i)
		{
			if( EffectsToTry[i] )
			{
				SpellOnTarget(EffectsToTry[i], this);
			}
		}

		SpellOnTarget(SPELL_TOUCH_OF_THE_DIVINE, this); //Touch of the Divine=4789, an Invulnerability/HoT/Purify effect
		SendHPUpdate();
		return true;
	}
	return false;
}

bool Mob::TryDeathSave() {

	/*
	How Death Save works:
	-Chance for Death Save to fire is the same for Death Pact/Divine Intervention
	-Base value of these determines amount healed (1=partial(300HP), 2='full (8000HP)) HARD CODED
	-Charisma of client who has the effect determines fire rate, parses show this clearly with ratio used.
	-Unfailing Divinity AA - Allows for a chance to give a heal for a percentage of the orginal value
	 when your innate chance fails and removes the buff. The spell effect for Unfailing Divinity gives
	 the a value of a heal modifier of the base effects heal.
	 Ie. Divine Intervention is 8000 HP Max UD1=20, therefore heal is 8000*20/100
	-No evidence of chance rate increasing between UD1-3, numbers indicate it uses same CHA rate as first DI.
	-In later expansions this SpellEffect::DeathSave was given a level limit and a heal value in its effect data.
	*/

	if (spellbonuses.DeathSave[SBIndex::DEATH_SAVE_TYPE]){

		int success_chance = 0;
		int buffSlot = spellbonuses.DeathSave[SBIndex::DEATH_SAVE_BUFFSLOT];
		int32 UD_HealMod = 0;
		int64 HealAmt = 300; //Death Pact max Heal

		if(buffSlot >= 0){
			UD_HealMod = buffs[buffSlot].ExtraDIChance;
			success_chance = ((GetCHA() * (RuleI(Spells, DeathSaveCharismaMod))) + 1) / 10; //(CHA Mod Default = 3)

			if (success_chance > 95) {
				success_chance = 95;
			}

			if(zone->random.Roll(success_chance)) {
				if (spellbonuses.DeathSave[SBIndex::DEATH_SAVE_TYPE] == DeathSave::HP8000) {
					HealAmt = RuleI(Spells, DivineInterventionHeal); //8000HP is how much LIVE Divine Intervention max heals
				}

				//Check if bonus Heal amount can be applied ([3] Bonus Heal [2] Level limit)
				if (spellbonuses.DeathSave[SBIndex::DEATH_SAVE_HEAL_AMT] && (GetLevel() >= spellbonuses.DeathSave[SBIndex::DEATH_SAVE_MIN_LEVEL_FOR_HEAL])) {
					HealAmt += spellbonuses.DeathSave[SBIndex::DEATH_SAVE_HEAL_AMT];
				}

				if ((GetMaxHP() - GetHP()) < HealAmt) {
					HealAmt = GetMaxHP() - GetHP();
				}

				SetHP((GetHP()+HealAmt));
				Message(Chat::Emote, "The gods have healed you for %i points of damage.", HealAmt);

				if (spellbonuses.DeathSave[SBIndex::DEATH_SAVE_TYPE] == DeathSave::HP8000) {
					entity_list.MessageCloseString(
						this,
						false,
						200,
						Chat::MeleeCrit,
						DIVINE_INTERVENTION,
						GetCleanName());
				} else {
					entity_list.MessageCloseString(this, false, 200, Chat::MeleeCrit, DEATH_PACT, GetCleanName());
				}

				SendHPUpdate();
				BuffFadeBySlot(buffSlot);
				return true;
			}
			else if (UD_HealMod) {
				success_chance = ((GetCHA() * (RuleI(Spells, DeathSaveCharismaMod))) + 1) / 10;

				if (success_chance > 95) {
					success_chance = 95;
				}

				if (zone->random.Roll(success_chance)) {
					if (spellbonuses.DeathSave[SBIndex::DEATH_SAVE_TYPE] == DeathSave::HP8000) {
						HealAmt = RuleI(Spells, DivineInterventionHeal);
					}

					//Check if bonus Heal amount can be applied ([3] Bonus Heal [2] Level limit)
					if (spellbonuses.DeathSave[SBIndex::DEATH_SAVE_HEAL_AMT] && (GetLevel() >= spellbonuses.DeathSave[SBIndex::DEATH_SAVE_MIN_LEVEL_FOR_HEAL])) {
						HealAmt += spellbonuses.DeathSave[SBIndex::DEATH_SAVE_HEAL_AMT];
					}

					HealAmt = HealAmt*UD_HealMod/100;

					if ((GetMaxHP() - GetHP()) < HealAmt) {
						HealAmt = GetMaxHP() - GetHP();
					}

					SetHP((GetHP()+HealAmt));
					Message(Chat::Emote, "The gods have healed you for %i points of damage.", HealAmt);

					if (spellbonuses.DeathSave[SBIndex::DEATH_SAVE_TYPE] == DeathSave::HP8000) {
						entity_list.MessageCloseString(
							this,
							false,
							200,
							Chat::MeleeCrit,
							DIVINE_INTERVENTION,
							GetCleanName());
					} else {
						entity_list.MessageCloseString(this, false, 200, Chat::MeleeCrit, DEATH_PACT, GetCleanName());
					}

					SendHPUpdate();
					BuffFadeBySlot(buffSlot);
					return true;
				}
			}
		}

		BuffFadeBySlot(buffSlot);
	}
	return false;
}

bool Mob::AffectedBySpellExcludingSlot(int slot, int effect)
{
	int buff_count = GetMaxTotalSlots();
	for (int i = 0; i < buff_count; i++)
	{
		if (i == slot)
			continue;

		if (IsEffectInSpell(buffs[i].spellid, effect))
			return true;
	}
	return false;
}

float Mob::GetSympatheticProcChances(uint16 spell_id, int16 ProcRateMod, int32 ItemProcRate) {

	float ProcChance = 0.0f;
	int32 total_cast_time = 0;
	float cast_time_mod = 0.0f;
	ProcRateMod -= 100;


	if (spells[spell_id].recast_time >= spells[spell_id].recovery_time)
		total_cast_time = spells[spell_id].recast_time + spells[spell_id].cast_time;
	else
		total_cast_time = spells[spell_id].recovery_time + spells[spell_id].cast_time;

	if (total_cast_time > 0 && total_cast_time <= 2500)
		cast_time_mod = 0.25f;
	 else if (total_cast_time > 2500 && total_cast_time < 7000)
		 cast_time_mod = 0.167f*((static_cast<float>(total_cast_time) - 1000.0f)/1000.0f);
	 else
		 cast_time_mod = static_cast<float>(total_cast_time) / 7000.0f;

	ProcChance = (RuleR(Casting, AvgSpellProcsPerMinute)/100.0f) * (static_cast<float>(100.0f + ProcRateMod) / 10.0f)
		* cast_time_mod * (static_cast<float>(100.0f + ItemProcRate)/100.0f);

 	return ProcChance;
 }

int16 Mob::GetSympatheticSpellProcRate(uint16 spell_id)
{
	for (int i = 0; i < EFFECT_COUNT; i++){
		if (spells[spell_id].effect_id[i] == SpellEffect::SympatheticProc)
			return spells[spell_id].base_value[i];
	}

	return 0;
}

uint16 Mob::GetSympatheticSpellProcID(uint16 spell_id)
{
	for (int i = 0; i < EFFECT_COUNT; i++){
		if (spells[spell_id].effect_id[i] == SpellEffect::SympatheticProc)
			return spells[spell_id].limit_value[i];
	}

	return 0;
}

bool Mob::DoHPToManaCovert(int32 mana_cost)
{
	if (spellbonuses.HPToManaConvert){
		int64 hp_cost = spellbonuses.HPToManaConvert * mana_cost / 100;
		if(hp_cost) {
			SetHP(GetHP()-hp_cost);
			return true;
		}
	return false;
	}

	return false;
}

int64 Mob::GetFcDamageAmtIncoming(Mob *caster, int32 spell_id, bool from_buff_tic)
{
	//THIS is target of spell cast
	int64 dmg = 0;
	dmg += GetFocusEffect(focusFcDamageAmtIncoming, spell_id, caster, from_buff_tic); //SPA 297 SpellEffect::FcDamageAmtIncoming
	dmg += GetFocusEffect(focusFcSpellDamageAmtIncomingPC, spell_id, caster, from_buff_tic); //SPA 484 SpellEffect::Fc_Spell_Damage_Amt_IncomingPC
	return dmg;
}

int64 Mob::GetFocusIncoming(focusType type, int effect, Mob *caster, uint32 spell_id) {

	//**** This can be removed when bot healing focus code is updated ****

	/*
	This is a general function for calculating best focus effect values for focus effects that exist on targets but modify incoming spells.
	Should be used when checking for foci that can exist on clients or npcs ect.
	Example: When your target has a focus limited buff that increases amount of healing on them.
	*/

	if (!caster) {
		return 0;
	}

	int64 value = 0;

	if (spellbonuses.FocusEffects[type]){

		int32 tmp_focus = 0;
		int tmp_buffslot = -1;

		int buff_count = GetMaxTotalSlots();
		for(int i = 0; i < buff_count; i++) {

			if((IsValidSpell(buffs[i].spellid) && IsEffectInSpell(buffs[i].spellid, effect))){

				int64 focus = caster->CalcFocusEffect(type, buffs[i].spellid, spell_id);

				if (!focus) {
					continue;
				}

				if (tmp_focus && focus > tmp_focus){
					tmp_focus = focus;
					tmp_buffslot = i;
				}

				else if (!tmp_focus){
					tmp_focus = focus;
					tmp_buffslot = i;
				}
			}
		}

		value = tmp_focus;

		if (tmp_buffslot >= 0)
			CheckNumHitsRemaining(NumHit::MatchingSpells, tmp_buffslot);
	}


	return value;
}

bool Mob::PassLimitClass(uint32 Classes_, uint16 Class_)
{
	//The class value for SpellEffect::LimitClass is +1 to its equivelent value in item dbase
	//Example Bard on items is '128' while Bard on SpellEffect::LimitClass is '256', keep this in mind if making custom spells.
	if (Class_ > 16)
		return false;

	Class_ += 1;
	for (int CurrentClass = 1; CurrentClass <= Class::PLAYER_CLASS_COUNT; ++CurrentClass){
		if (Classes_ % 2 == 1){
			if (CurrentClass == Class_)
				return true;
		}
		Classes_ >>= 1;
	}
	return false;
}

void Mob::DispelMagic(Mob* caster, uint16 spell_id, int effect_value)
{
	for (int slot = 0; slot < GetMaxTotalSlots(); slot++) {
		if (
			buffs[slot].spellid != SPELL_UNKNOWN &&
			spells[buffs[slot].spellid].dispel_flag == 0 &&
			!IsDiscipline(buffs[slot].spellid)
		) {
			if (caster && TryDispel(caster->GetCasterLevel(spell_id), buffs[slot].casterlevel, effect_value)) {
				BuffFadeBySlot(slot);
				break;
			}
		}
	}
}

bool Mob::TrySpellEffectResist(uint16 spell_id)
{
	/*
		SEResist variable
		0 = spell effect id to be check if can resist
		1 = percent chance of resistance
	*/


	if (!IsValidSpell(spell_id)) {
		return false;
	}

	if (!aabonuses.SEResist[1] && !spellbonuses.SEResist[1] && !itembonuses.SEResist[1]) {
		return false;
	}

	int resist_chance = 0;

	for(int i = 0; i < EFFECT_COUNT; ++i)
	{
		if (spells[spell_id].effect_id[i] == SpellEffect::Blank) {
			continue;
		}

		for(int d = 0; d < MAX_RESISTABLE_EFFECTS*2; d+=2)
		{
			resist_chance = 0;
			if (spells[spell_id].effect_id[i] == aabonuses.SEResist[d]){
				resist_chance += aabonuses.SEResist[d+1];
			}

			if (spells[spell_id].effect_id[i] == itembonuses.SEResist[d]){
				resist_chance += itembonuses.SEResist[d+1];
			}

			if (spells[spell_id].effect_id[i] == spellbonuses.SEResist[d]){
				resist_chance += spellbonuses.SEResist[d+1];
			}

			if (resist_chance) {
				if (zone->random.Roll(resist_chance)) {
					LogSpells("Resisted spell from Spell Effect Resistance, had [{}] chance to resist spell effect id [{}]", resist_chance, spells[spell_id].effect_id[i]);
					return true;
				}

				break;
			}
		}
	}
	return false;
}

bool Mob::TryDispel(uint8 caster_level, uint8 buff_level, int level_modifier){

	//Dispels - Check level of caster agianst buffs level (level of the caster who cast the buff)
	//Effect value of dispels are treated as a level modifier.
	//Values for scaling were obtain from live parses, best estimates.

	caster_level += level_modifier - 1;
	int dispel_chance = 32; //Baseline chance if no level difference and no modifier
	int level_diff = caster_level - buff_level;

	if (level_diff > 0)
		dispel_chance += level_diff * 7;

	else if (level_diff < 0)
		dispel_chance += level_diff * 2;

	if (dispel_chance >= 100)
		return true;

	else if (dispel_chance < 10)
		dispel_chance = 10;

	if (zone->random.Roll(dispel_chance))
		return true;
	else
		return false;
}

bool Mob::ImprovedTaunt(){

	if (spellbonuses.ImprovedTaunt[SBIndex::IMPROVED_TAUNT_MAX_LVL]){

		if (GetLevel() > spellbonuses.ImprovedTaunt[SBIndex::IMPROVED_TAUNT_MAX_LVL])
			return false;

		if (spellbonuses.ImprovedTaunt[SBIndex::IMPROVED_TAUNT_BUFFSLOT] >= 0){

			target = entity_list.GetMob(buffs[spellbonuses.ImprovedTaunt[SBIndex::IMPROVED_TAUNT_BUFFSLOT]].casterid);

			if (target){
				SetTarget(target);
				return true;
			}
			else {
				if(!TryFadeEffect(spellbonuses.ImprovedTaunt[SBIndex::IMPROVED_TAUNT_BUFFSLOT]))
					BuffFadeBySlot(spellbonuses.ImprovedTaunt[SBIndex::IMPROVED_TAUNT_BUFFSLOT], true); //If caster killed removed effect.
			}
		}
	}

	return false;
}


bool Mob::PassCastRestriction(int value)
{
	/*
		Restriction ID corresponds to the type 39 value in dstr_us on live clients (2021). See enum SpellRestriction for full list.
		Modern client will give a message corresponding the type 39 field in the dstr_us for many of these effects upon failure.

		Use with spell_news table field 'CastRestriction' which limits targets by restrictions below and 'caster_requirement' (field220)
		which limits caster by restrictions below.
		These restrictions also apply to direct damage,dot, heal spells using SPA 0 or SPA 79 by placing a restriction id in the LIMIT field.

		Note: (ID 221 - 249) For effect seen in mage spell 'Shock of Many' which increases damage based on number of pets on targets hatelist. The way it is implemented
		works for how our ROF2 spell file handles the effect where each slot fires individually, while on live it only takes the highest
		value. In the future the way check is done will need to be adjusted to check a defined range instead of just great than.
	*/

	if (value <= 0) {
		return true;
	}

	switch(value)
	{
		case 1:
			return true;
			break;

		case IS_NOT_ON_HORSE:
			if ((IsClient() && !CastToClient()->GetHorseId()) || IsBot() || IsMerc()) {
				return true;
			}
			break;

		case IS_ANIMAL_OR_HUMANOID:
			if ((GetBodyType() == BodyType::Animal) || (GetBodyType() == BodyType::Humanoid))
				return true;
			break;

		case IS_DRAGON:
			if (GetBodyType() == BodyType::Dragon || GetBodyType() == BodyType::VeliousDragon || GetBodyType() == BodyType::Dragon3)
				return true;
			break;

		case IS_ANIMAL_OR_INSECT:
			if ((GetBodyType() == BodyType::Animal) || (GetBodyType() == BodyType::Insect))
				return true;
			break;

		case IS_BODY_TYPE_MISC:
			if ((GetBodyType() == BodyType::Humanoid)  || (GetBodyType() == BodyType::Lycanthrope) || (GetBodyType() == BodyType::Giant) ||
				(GetBodyType() == BodyType::RaidGiant) || (GetBodyType() == BodyType::RaidColdain) || (GetBodyType() == BodyType::Animal)||
				(GetBodyType() == BodyType::Construct) || (GetBodyType() == BodyType::Dragon)		 || (GetBodyType() == BodyType::Insect)||
				(GetBodyType() == BodyType::VeliousDragon) || (GetBodyType() == BodyType::Muramite) || (GetBodyType() == BodyType::Magical))
				return true;
			break;

		case IS_BODY_TYPE_MISC2:
			if ((GetBodyType() == BodyType::Humanoid) || (GetBodyType() == BodyType::Lycanthrope) || (GetBodyType() == BodyType::Giant) ||
				(GetBodyType() == BodyType::RaidGiant) || (GetBodyType() == BodyType::RaidColdain) || (GetBodyType() == BodyType::Animal) ||
				(GetBodyType() == BodyType::Insect))
				return true;
			break;

		case IS_PLANT:
			if (GetBodyType() == BodyType::Plant)
				return true;
			break;

		case IS_GIANT:
			if (GetBodyType() == BodyType::Giant)
				return true;
			break;

		case IS_NOT_ANIMAL_OR_HUMANOID:
			if ((GetBodyType() != BodyType::Animal) || (GetBodyType() != BodyType::Humanoid))
				return true;
			break;

		case IS_BIXIE:
		case IS_BIXIE2:
			if ((GetRace() == Race::Bixie) ||(GetRace() == Race::Bixie2))
				return true;
			break;

		case IS_HARPY:
			if ((GetRace() == Race::Harpy) ||(GetRace() == Race::Harpy2))
				return true;
			break;

		case IS_GNOLL:
			if ((GetRace() == Race::Gnoll) || (GetRace() == Race::Gnoll2) || (GetRace() == Race::Gnoll3))
				return true;
			break;

		case IS_SPORALI:
			if ((GetRace() == Race::Sporali) ||(GetRace() == Race::Fungusman))
				return true;
			break;

		case IS_KOBOLD:
			if ((GetRace() == Race::Kobold) ||(GetRace() == Race::Kobold2))
				return true;
			break;

		case IS_FROSTCRYPT_SHADE:
			if (GetRace() == Race::GiantShade)
				return true;
			break;

		case IS_DRAKKIN:
			if (GetRace() == Race::Drakkin)
				return true;
			break;

		case IS_UNDEAD_OR_VALDEHOLM_GIANT:
			if (GetBodyType() == BodyType::Undead || GetRace() == Race::Giant2 || GetRace() == Race::Giant3)
				return true;
			break;

		case IS_ANIMAL_OR_PLANT:
			if ((GetBodyType() == BodyType::Animal) || (GetBodyType() == BodyType::Plant))
				return true;
			break;

		case IS_SUMMONED:
			if (GetBodyType() == BodyType::Summoned)
				return true;
			break;

		case IS_CLASS_WIZARD:
		case IS_WIZARD_USED_ON_MAGE_FIRE_PET:
			if (GetClass() == Class::Wizard)
				return true;
			break;

		case IS_UNDEAD:
			if (GetBodyType() == BodyType::Undead)
				return true;
			break;

		case IS_NOT_UNDEAD_OR_SUMMONED_OR_VAMPIRE:
			if ((GetBodyType() != BodyType::Undead) && (GetBodyType() != BodyType::Summoned) && (GetBodyType() != BodyType::Vampire))
				return true;
			break;

		case IS_FAE_OR_PIXIE:
			if ((GetRace() == Race::Pixie) || (GetRace() == Race::FayDrake))
				return  true;
			break;

		case IS_HUMANOID:
			if (GetBodyType() == BodyType::Humanoid)
				return true;
			break;

		case IS_UNDEAD_AND_HP_LESS_THAN_10_PCT:
			if ((GetBodyType() == BodyType::Undead) && (GetHPRatio() < 10))
				return true;
			break;

		case IS_CLOCKWORK_AND_HP_LESS_THAN_45_PCT:
			if ((GetRace() == Race::Gnomework || GetRace() == Race::ClockworkGnome) && (GetHPRatio() < 45))
				return true;
			break;

		case IS_WISP_AND_HP_LESS_THAN_10_PCT:
			if ((GetRace() == Race::Wisp) && (GetHPRatio() < 10))
				return true;
			break;

		case IS_CLASS_MELEE_THAT_CAN_BASH_OR_KICK_EXCEPT_BARD:
			if ((GetClass() != Class::Bard) && (GetClass() != Class::Rogue) && IsFighterClass(GetClass()))
				return true;
			break;

		case IS_CLASS_PURE_MELEE:
			if (GetClass() == Class::Rogue || GetClass() == Class::Warrior || GetClass() == Class::Berserker || GetClass() == Class::Monk)
				return true;
			break;

		case IS_CLASS_PURE_CASTER:
			if (IsINTCasterClass(GetClass()))
				return true;
			break;

		case IS_CLASS_HYBRID_CLASS:
			if (IsHybridClass(GetClass()))
				return true;
			break;

		case IS_CLASS_WARRIOR:
			if (GetClass() == Class::Warrior)
				return true;
			break;

		case IS_CLASS_CLERIC:
			if (GetClass() == Class::Cleric)
				return true;
			break;

		case IS_CLASS_PALADIN:
			if (GetClass() == Class::Paladin)
				return true;
			break;

		case IS_CLASS_RANGER:
			if (GetClass() == Class::Ranger)
				return true;
			break;

		case IS_CLASS_SHADOWKNIGHT:
			if (GetClass() == Class::ShadowKnight)
				return true;
			break;

		case IS_CLASS_DRUID:
			if (GetClass() == Class::Druid)
				return true;
			break;

		case IS_CLASS_MONK:
			if (GetClass() == Class::Monk)
				return true;
			break;

		case IS_CLASS_BARD2:
		case IS_CLASS_BARD:
			if (GetClass() == Class::Bard)
				return true;
			break;

		case IS_CLASS_ROGUE:
			if (GetClass() == Class::Rogue)
				return true;
			break;

		case IS_CLASS_SHAMAN:
			if (GetClass() == Class::Shaman)
				return true;
			break;

		case IS_CLASS_NECRO:
			if (GetClass() == Class::Necromancer)
				return true;
			break;

		case IS_CLASS_MAGE:
			if (GetClass() == Class::Magician)
				return true;
			break;

		case IS_CLASS_ENCHANTER:
			if (GetClass() == Class::Enchanter)
				return true;
			break;

		case IS_CLASS_BEASTLORD:
			if (GetClass() == Class::Beastlord)
				return true;
			break;

		case IS_CLASS_BERSERKER:
			if (GetClass() == Class::Berserker)
				return true;
			break;

		case IS_CLASS_CLR_SHM_DRU:
			if (IsWISCasterClass(GetClass()))
				return true;
			break;

		case IS_CLASS_NOT_WAR_PAL_SK:
			if ((GetClass() != Class::Warrior) && (GetClass() != Class::Paladin) && (GetClass() != Class::ShadowKnight))
				return true;
			break;

		case IS_LEVEL_UNDER_100:
			if (GetLevel() < 100)
				return true;
			break;

		case IS_NOT_RAID_BOSS:
			if (!IsRaidTarget())
				return true;
			break;

		case IS_RAID_BOSS:
			if (IsRaidTarget())
				return true;
			break;

		case FRENZIED_BURNOUT_ACTIVE:
			if (HasBuffWithSpellGroup(SPELLGROUP_FRENZIED_BURNOUT))
				return true;
			break;

		case FRENZIED_BURNOUT_NOT_ACTIVE:
			if (!HasBuffWithSpellGroup(SPELLGROUP_FRENZIED_BURNOUT))
				return true;
			break;

		case IS_HP_ABOVE_75_PCT:
			if (GetHPRatio() > 75)
				return true;
			break;

		case IS_HP_LESS_THAN_20_PCT:
			if (GetHPRatio() <= 20)
				return true;
			break;

		case IS_HP_LESS_THAN_50_PCT:
			if (GetHPRatio() <= 50)
				return true;
			break;

		case IS_HP_LESS_THAN_75_PCT:
			if (GetHPRatio() <= 75)
				return true;
			break;

		case IS_NOT_IN_COMBAT:
			if (!IsEngaged())
				return true;
			break;

		case IS_HP_LESS_THAN_35_PCT:
			if (GetHPRatio() <= 35)
				return true;
			break;

		case HAS_BETWEEN_1_TO_2_PETS_ON_HATELIST: {
			int count = hate_list.GetSummonedPetCountOnHateList();
			if (count >= 1 && count <= 2) {
				return true;
			}
			break;
		}

		case HAS_BETWEEN_3_TO_5_PETS_ON_HATELIST: {
			int count = hate_list.GetSummonedPetCountOnHateList();
			if (count >= 3 && count <= 5) {
				return true;
			}
			break;
		}

		case HAS_BETWEEN_6_TO_9_PETS_ON_HATELIST: {
			int count = hate_list.GetSummonedPetCountOnHateList();
			if (count >= 6 && count <= 9) {
				return true;
			}
			break;
		}

		case HAS_BETWEEN_10_TO_14_PETS_ON_HATELIST: {
			int count = hate_list.GetSummonedPetCountOnHateList();
			if (count >= 10 && count <= 14) {
				return true;
			}
			break;
		}

		case HAS_MORE_THAN_14_PETS_ON_HATELIST: {
			int count = hate_list.GetSummonedPetCountOnHateList();
			if (count > 14) {
				return true;
			}
			break;
		}

		case IS_CLASS_CHAIN_OR_PLATE:
			if ((GetClass() == Class::Warrior) || (GetClass() == Class::Bard) || (GetClass() == Class::ShadowKnight) || (GetClass() == Class::Paladin) || (GetClass() == Class::Cleric)
				|| (GetClass() == Class::Ranger) || (GetClass() == Class::Shaman) || (GetClass() == Class::Rogue) || (GetClass() == Class::Berserker)) {
				return true;
			}
			break;

		case IS_HP_BETWEEN_5_AND_9_PCT:
			if (GetHPRatio() >= 5 && GetHPRatio() <= 9)
				return true;
			break;

		case IS_HP_BETWEEN_10_AND_14_PCT:
			if (GetHPRatio() >= 10 && GetHPRatio() <= 14)
				return true;
			break;

		case IS_HP_BETWEEN_15_AND_19_PCT:
			if (GetHPRatio() >= 15 && GetHPRatio() <= 19)
				return true;
			break;

		case IS_HP_BETWEEN_20_AND_24_PCT:
			if (GetHPRatio() >= 20 && GetHPRatio() <= 24)
				return true;
			break;

		case IS_HP_BETWEEN_25_AND_29_PCT:
			if (GetHPRatio() >= 25 && GetHPRatio() <= 29)
				return true;
			break;

		case IS_HP_BETWEEN_30_AND_34_PCT:
			if (GetHPRatio() >= 30 && GetHPRatio() <= 34)
				return true;
			break;

		case IS_HP_BETWEEN_35_AND_39_PCT:
			if (GetHPRatio() >= 35 && GetHPRatio() <= 39)
				return true;
			break;

		case IS_HP_BETWEEN_40_AND_44_PCT:
			if (GetHPRatio() >= 40 && GetHPRatio() <= 44)
				return true;
			break;

		case IS_HP_BETWEEN_45_AND_49_PCT:
			if (GetHPRatio() >= 45 && GetHPRatio() <= 49)
				return true;
			break;

		case IS_HP_BETWEEN_50_AND_54_PCT:
			if (GetHPRatio() >= 50 && GetHPRatio() <= 54)
				return true;
			break;

		case IS_HP_BETWEEN_55_AND_59_PCT:
			if (GetHPRatio() >= 55 && GetHPRatio() <= 59)
				return true;
			break;

		case IS_HP_BETWEEN_5_AND_15_PCT:
			if (GetHPRatio() >= 5 && GetHPRatio() <= 15)
				return true;
			break;

		case IS_HP_BETWEEN_15_AND_25_PCT:
			if (GetHPRatio() >= 15 && GetHPRatio() <= 25)
				return true;
			break;

		case IS_HP_BETWEEN_1_AND_25_PCT:
			if (GetHPRatio() <= 25)
				return true;
			break;

		case IS_HP_BETWEEN_25_AND_35_PCT:
			if (GetHPRatio() > 25 && GetHPRatio() <= 35)
				return true;
			break;

		case IS_HP_BETWEEN_35_AND_45_PCT:
			if (GetHPRatio() > 35 && GetHPRatio() <= 45)
				return true;
			break;

		case IS_HP_BETWEEN_45_AND_55_PCT:
			if (GetHPRatio() > 45 && GetHPRatio() <= 55)
				return true;
			break;

		case IS_HP_BETWEEN_55_AND_65_PCT:
			if (GetHPRatio() > 55 && GetHPRatio() <= 65)
				return true;
			break;

		case IS_HP_BETWEEN_65_AND_75_PCT:
			if (GetHPRatio() > 65 && GetHPRatio() <= 75)
				return true;
			break;

		case IS_HP_BETWEEN_75_AND_85_PCT:
			if (GetHPRatio() > 75 && GetHPRatio() <= 85)
				return true;
			break;

		case IS_HP_BETWEEN_85_AND_95_PCT:
			if (GetHPRatio() > 85 && GetHPRatio() <= 95)
				return true;
			break;

		case IS_HP_ABOVE_45_PCT:
			if (GetHPRatio() > 45)
				return true;
			break;

		case IS_HP_ABOVE_55_PCT:
			if (GetHPRatio() > 55)
				return true;
			break;

		case IS_MANA_ABOVE_10_PCT:
			if (GetManaRatio() > 10)
				return true;
			break;

		case IS_ENDURANCE_BELOW_40_PCT:
			if (IsClient() && CastToClient()->GetEndurancePercent() <= 40)
				return true;
			break;

		case IS_MANA_BELOW_40_PCT:
			if (GetManaRatio() <= 40)
				return true;
			break;

		case IS_HP_ABOVE_20_PCT:
			if (GetHPRatio() > 20)
				return true;
			break;

		case IS_NOT_UNDEAD_OR_SUMMONED:
			if ((GetBodyType() != BodyType::Undead) && (GetBodyType() != BodyType::Summoned))
				return true;
			break;

		case IS_NOT_PLANT:
			if (GetBodyType() != BodyType::Plant)
				return true;
			break;

		case IS_NOT_CLIENT:
			if (!IsClient())
				return true;
			break;

		case IS_CLIENT:
			if (IsClient())
				return true;
			break;

		case IS_LEVEL_ABOVE_42_AND_IS_CLIENT:
			if (IsClient() && GetLevel() > 42)
				return true;
			break;

		case IS_TREANT:
			if (GetRace() == Race::Treant || GetRace() == Race::Treant2 || GetRace() == Race::Treant3)
				return true;
			break;

		case IS_SCARECROW:
			if (GetRace() == Race::Scarecrow || GetRace() == Race::Scarecrow2)
				return true;
			break;

		case IS_VAMPIRE_OR_UNDEAD_OR_UNDEADPET:
			if (GetBodyType() == BodyType::Vampire || GetBodyType() == BodyType::Undead || GetBodyType() == BodyType::SummonedUndead)
				return true;
			break;

		case IS_NOT_VAMPIRE_OR_UNDEAD:
			if (GetBodyType() != BodyType::Vampire && GetBodyType() != BodyType::Undead && GetBodyType() != BodyType::SummonedUndead)
				return true;
			break;

		case IS_CLASS_KNIGHT_HYBRID_MELEE:
			if (IsHybridClass(GetClass()) || IsNonSpellFighterClass(GetClass()))
				return true;
			break;

		case IS_CLASS_WARRIOR_CASTER_PRIEST:
			if (IsCasterClass(GetClass()) || GetClass() == Class::Warrior)
				return true;
			break;

		case IS_END_BELOW_21_PCT:
			if (IsClient() && CastToClient()->GetEndurancePercent() <= 21)
				return true;
			break;

		case IS_END_BELOW_25_PCT:
			if (IsClient() && CastToClient()->GetEndurancePercent() <= 25)
				return true;
			break;

		case IS_END_BELOW_29_PCT:
			if (IsClient() && CastToClient()->GetEndurancePercent() <= 29)
				return true;
			break;

		case IS_REGULAR_SERVER:
			return true; // todo implement progression flag assume not progression for now
			break;

		case IS_PROGRESSION_SERVER:
			return false; // todo implement progression flag assume not progression for now
			break;

		case IS_GOD_EXPANSION_UNLOCKED:
			return true; // todo implement progression flag assume not progression for now, this one is a check if GoD is live
			break;

		case IS_HUMANOID_LEVEL_84_MAX:
			if (GetBodyType() == BodyType::Humanoid && GetLevel() <= 84)
				return true;
			break;

		case IS_HUMANOID_LEVEL_86_MAX:
			if (GetBodyType() == BodyType::Humanoid && GetLevel() <= 86)
				return true;
			break;

		case IS_HUMANOID_LEVEL_88_MAX:
			if (GetBodyType() == BodyType::Humanoid && GetLevel() <= 88)
				return true;
			break;

		case IS_LEVEL_90_MAX:
			if (GetLevel() <= 90)
				return true;
			break;

		case IS_LEVEL_92_MAX:
			if (GetLevel() <= 92)
				return true;
			break;

		case IS_LEVEL_94_MAX:
			if (GetLevel() <= 94)
				return true;
			break;

		case IS_LEVEL_95_MAX:
			if (GetLevel() <= 95)
				return true;
			break;

		case IS_LEVEL_97_MAX:
			if (GetLevel() <= 97)
				return true;
			break;

		case IS_LEVEL_99_MAX:
			if (GetLevel() <= 99)
				return true;
			break;

		case IS_LEVEL_100_MAX:
			if (GetLevel() <= 100)
				return true;
			break;

		case IS_LEVEL_102_MAX:
			if (GetLevel() <= 102)
				return true;
			break;

		case IS_LEVEL_104_MAX:
			if (GetLevel() <= 104)
				return true;
			break;
		case IS_LEVEL_105_MAX:
			if (GetLevel() <= 105)
				return true;
			break;

		case IS_LEVEL_107_MAX:
			if (GetLevel() <= 107)
				return true;
			break;

		case IS_LEVEL_109_MAX:
			if (GetLevel() <= 109)
				return true;
			break;
		case IS_LEVEL_110_MAX:
			if (GetLevel() <= 110)
				return true;
			break;

		case IS_LEVEL_112_MAX:
			if (GetLevel() <= 112)
				return true;
			break;

		case IS_LEVEL_114_MAX:
			if (GetLevel() <= 114)
				return true;
			break;

		case IS_BETWEEN_LEVEL_1_AND_75:
			if (GetLevel() >= 1 && GetLevel() <= 75)
				return true;
			break;

		case IS_BETWEEN_LEVEL_76_AND_85:
			if (GetLevel() >= 76 && GetLevel() <= 85)
				return true;
			break;

		case IS_BETWEEN_LEVEL_86_AND_95:
			if (GetLevel() >= 86 && GetLevel() <= 95)
				return true;
			break;

		case IS_BETWEEN_LEVEL_96_AND_105:
			if (GetLevel() >= 96 && GetLevel() <= 105)
				return true;
			break;

		case IS_HP_LESS_THAN_80_PCT:
			if (GetHPRatio() < 80)
				return true;
			break;

		case IS_LEVEL_ABOVE_34:
			if (GetLevel() < 34)
				return true;
			break;

		case HAS_NO_MANA_BURN_BUFF: {
			bool has_effect = false;
			for (int i = 0; i < GetMaxTotalSlots(); i++) {
				if (IsValidSpell(buffs[i].spellid) && IsEffectInSpell(buffs[i].spellid, SpellEffect::ManaBurn)) {
					has_effect = true;
				}
			}
			if (!has_effect) {
				return true;
			}
			break;
		}

		case IS_CLIENT_AND_MALE_PLATE_USER:
			if (IsClient() && GetGender() == Gender::Male && IsPlateClass(GetClass()))
				return true;
			break;

		case IS_CLEINT_AND_MALE_DRUID_ENCHANTER_MAGICIAN_NECROANCER_SHAMAN_OR_WIZARD:
			if (IsClient() && GetGender() == Gender::Male && (IsCasterClass(GetClass()) && GetClass() != Class::Cleric))
				return true;
			break;

		case IS_CLIENT_AND_MALE_BEASTLORD_BERSERKER_MONK_RANGER_OR_ROGUE:
			if (IsClient() && GetGender() == Gender::Male &&
				(GetClass() == Class::Beastlord || GetClass() == Class::Berserker || GetClass() == Class::Monk || GetClass() == Class::Ranger || GetClass() == Class::Rogue))
				return true;
			break;

		case IS_CLIENT_AND_FEMALE_PLATE_USER:
			if (IsClient() && GetGender() == Gender::Female && IsPlateClass(GetClass()))
				return true;
			break;

		case IS_CLIENT_AND_FEMALE_DRUID_ENCHANTER_MAGICIAN_NECROANCER_SHAMAN_OR_WIZARD:
			if (IsClient() && GetGender() == Gender::Female && (IsCasterClass(GetClass()) && GetClass() != Class::Cleric))
				return true;
			break;

		case IS_CLIENT_AND_FEMALE_BEASTLORD_BERSERKER_MONK_RANGER_OR_ROGUE:
			if (IsClient() && GetGender() == Gender::Female &&
				(GetClass() == Class::Beastlord || GetClass() == Class::Berserker || GetClass() == Class::Monk || GetClass() == Class::Ranger || GetClass() == Class::Rogue))
				return true;
			break;

		case IS_HP_ABOVE_50_PCT:
			if (GetHPRatio() > 50)
				return true;
			break;

		case IS_HP_BELOW_50_PCT:
			if (GetHPRatio() <= 50)
				return true;
			break;

		case IS_OFF_HAND_EQUIPPED:
			if (HasShieldEquipped() || CanThisClassDualWield())
				return true;
			break;

		case IS_MANA_BELOW_20_PCT:
			if (GetManaRatio() <= 20)
				return true;
			break;

		case IS_MANA_ABOVE_50_PCT:
			if (GetManaRatio() >= 50)
				return true;
			break;

		case IS_SUMMONED_OR_UNDEAD:
			if (GetBodyType() == BodyType::Summoned || GetBodyType() == BodyType::Undead)
				return true;
			break;


		case IS_CLASS_CASTER_PRIEST:
			if (IsCasterClass(GetClass()))
				return true;
			break;

		case IS_END_OR_MANA_ABOVE_20_PCT: {
			if (IsNonSpellFighterClass(GetClass()) && CastToClient()->GetEndurancePercent() >= 20) {
				return true;
			}
			else if (!IsNonSpellFighterClass(GetClass()) && GetManaRatio() >= 20) {
				return true;
			}
			break;
		}
		case IS_END_OR_MANA_BELOW_10_PCT: {
			if (IsNonSpellFighterClass(GetClass()) && CastToClient()->GetEndurancePercent() <= 10) {
				return true;
			}
			else if (!IsNonSpellFighterClass(GetClass()) && GetManaRatio() <= 10) {
				return true;
			}
			else if (IsHybridClass(GetClass()) && CastToClient()->GetEndurancePercent() <= 10) {
				return true;
			}
			break;
		}
		case IS_END_OR_MANA_BELOW_30_PCT:
		case IS_END_OR_MANA_BELOW_30_PCT2: {
			if (IsNonSpellFighterClass(GetClass()) && CastToClient()->GetEndurancePercent() <= 30) {
				return true;
			}
			else if (!IsNonSpellFighterClass(GetClass()) && GetManaRatio() <= 30) {
				return true;
			}
			break;
		}

		case IS_NOT_CLASS_BARD:
			if (GetClass() != Class::Bard)
				return true;
			break;

		case HAS_NO_PACT_OF_FATE_RECOURSE_BUFF:
			if (!FindBuff(SPELL_PACT_OF_HATE_RECOURSE))
				return true;
			break;

		case HAS_NO_ROGUES_FURY_BUFF:
			if (!HasBuffWithSpellGroup(SPELLGROUP_ROGUES_FURY))
				return true;
			break;

		case HAS_NO_ILLUSIONS_OF_GRANDEUR_BUFF:
			if (!HasBuffWithSpellGroup(SPELLGROUP_ILLUSION_OF_GRANDEUR))
				return true;
			break;

		case HAS_NO_HARMONIOUS_PRECISION_BUFF:
			if (!HasBuffWithSpellGroup(SPELLGROUP_HARMONIOUS_PRECISION))
				return true;
			break;

		case HAS_NO_HARMONIOUS_EXPANSE_BUFF:
			if (!HasBuffWithSpellGroup(SPELLGROUP_HARMONIOUS_EXPANSE))
				return true;
			break;

		case HAS_NO_FURIOUS_RAMPAGE_BUFF:
			if (!HasBuffWithSpellGroup(SPELLGROUP_FURIOUS_RAMPAGE))
				return true;
			break;

		case HAS_NO_SHROUD_OF_PRAYER_BUFF:
			if (!HasBuffWithSpellGroup(SPELLGROUP_SHROUD_OF_PRAYER))
				return true;
			break;

		case HAS_INCENDIARY_OOZE_BUFF:
			if (FindBuff(SPELL_INCENDIARY_OOZE_BUFF))
				return true;
			break;

		//Not handled, just allow them to pass for now.
		case UNKNOWN_3:
		case HAS_CRYSTALLIZED_FLAME_BUFF:
		case UNKNOWN_199:
		case UNKNOWN_TOO_MUCH_HP_410:
		case UNKNOWN_TOO_MUCH_HP_411:
		case HAS_TBL_ESIANTI_ACCESS:
		case HAS_ITEM_CLOCKWORK_SCRAPS:
		case IN_TWO_HANDED_STANCE:
		case IN_DUAL_WIELD_HANDED_STANCE:
		case IN_SHIELD_STANCE:
		case NOT_IN_TWO_HANDED_STANCE:
		case NOT_IN_DUAL_WIELD_HANDED_STANCE:
		case NOT_IN_SHIELD_STANCE:
		case DISABLED_UNTIL_EXPANSION_ROK:
		case DISABLED_UNTIL_EXPANSION_SOV:
		case DISABLED_UNTIL_EXPANSION_SOL:
		case DISABLED_UNTIL_EXPANSION_POP:
		case DISABLED_UNTIL_EXPANSION_LOY:
		case DISABLED_UNTIL_EXPANSION_LDON:
		case DISABLED_UNTIL_EXPANSION_GOD:
		case DISABLED_UNTIL_EXPANSION_OOW:
		case DISABLED_UNTIL_EXPANSION_DON:
		case DISABLED_UNTIL_EXPANSION_DOD:
		case DISABLED_UNTIL_EXPANSION_POR:
		case DISABLED_UNTIL_EXPANSION_TSS:
		case DISABLED_UNTIL_EXPANSION_TBS:
		case DISABLED_UNTIL_EXPANSION_SOF:
		case DISABLED_UNTIL_EXPANSION_SOD:
		case DISABLED_UNTIL_EXPANSION_UF:
		case DISABLED_UNTIL_EXPANSION_HOT:
		case DISABLED_UNTIL_EXPANSION_VOA:
		case DISABLED_UNTIL_EXPANSION_ROF:
		case DISABLED_UNTIL_EXPANSION_COF:
		case DISABLED_UNTIL_EXPANSION_TDS:
		case DISABLED_UNTIL_EXPANSION_TBM:
		case DISABLED_UNTIL_EXPANSION_EOK:
		case DISABLED_UNTIL_EXPANSION_ROS:
		case DISABLED_UNTIL_EXPANSION_TBL:
		case DISABLED_UNTIL_EXPANSION_TOV:
		case DISABLED_UNTIL_EXPANSION_COV:
		case HAS_TRAVELED_TO_STRATOS:
		case HAS_TRAVELED_TO_AALISHAI:
		case HAS_TRAVELED_TO_MEARATS:
		case COMPLETED_ACHIEVEMENT_LEGENDARY_ANSWERER:
		case NOT_COMPLETED_ACHIEVEMENT_LEGENDARY_ANSWERER:
		case HAS_WEAPONSTANCE_DEFENSIVE_PROFICIENCY:
		case HAS_WEAPONSTANCE_TWO_HAND_PROFICIENCY:
		case HAS_WEAPONSTANCE_DUAL_WEILD_PROFICIENCY:
		case UNKNOWN_812:
		case UNKNOWN_814:
		case UNKNOWN_822:
		case UNKNOWN_840:
		case UNKNOWN_841:
		case UNKNOWN_99999:
			return true;
		break;
	}

	if (value >= HAS_AT_LEAST_1_PET_ON_HATELIST && value <= HAS_AT_LEAST_20_PETS_ON_HATELIST) {
		int count = hate_list.GetSummonedPetCountOnHateList();
		int minium_amount_of_pets_needed = (1 + value) - HAS_AT_LEAST_1_PET_ON_HATELIST;

		if (count >= minium_amount_of_pets_needed) {
			return true;
		}
	}

	if (value >= IS_HP_BELOW_5_PCT && value <= IS_HP_BELOW_95_PCT) {
		int hp_below_amt = 5 * ((1 + value) - IS_HP_BELOW_5_PCT);
		if (GetHPRatio() <= hp_below_amt) {
			return true;
		}
	}

	if (value >= IS_BODY_TYPE_UNDEFINED &&  value <= IS_BODY_TYPE_MURAMITE){
		if (GetBodyType() == (value - IS_BODY_TYPE_UNDEFINED))
			return true;
	}

	//Limit to Race. *Not implemented on live, too much potential not to give an option here.
	if (value >= IS_RACE_FIRST_CUSTOM && value <= IS_RACE_LAST_CUSTOM){
		if (GetRace() == (value - IS_RACE_FIRST_CUSTOM))
			return true;
	}

	return false;
}

void Mob::SendCastRestrictionMessage(int requirement_id, bool target_requirement, bool is_discipline) {

	if (!IsClient()) {
		return;
	}
	/*
		Most of these are the live messages that modern clients give. Current live dbstr_us type 39
		Having these messages display greatly enhances the useabllity of these fields. (CastRestriction=target, caster_requirement_id=caster)
		If target_requirement is false then use the caster requirement message.
		Added support for different messages for certain high yield restrictions based on if
		target or caster requirements.

	*/

	std::string msg = "";

	if (target_requirement) {
		msg = "Your target does not meet the spell requirements. ";
	}
	else {
		if (is_discipline) {
			msg = "Your combat ability is interrupted. ";
		}
		else {
			msg = "Your spell is interrupted. ";
		}
	}

	switch (requirement_id)
	{
	case IS_ANIMAL_OR_HUMANOID:
		Message(Chat::Red, fmt::format("{} This spell will only work on animals or humanoid creatures.", msg).c_str());
		break;
	case IS_DRAGON:
		Message(Chat::Red, fmt::format("{} This spell will only work on dragons.", msg).c_str());
		break;
	case IS_ANIMAL_OR_INSECT:
		Message(Chat::Red, fmt::format("{} This spell will only work on animals or insects.", msg).c_str());
		break;
	case IS_BODY_TYPE_MISC:
		Message(Chat::Red, fmt::format("{} This spell will only work on humanoids, lycanthropes, giants, Kael Drakkel giants, Coldain, animals, insects, constructs, dragons, Skyshrine dragons, Muramites, or creatures constructed from magic.", msg).c_str());
		break;
	case IS_BODY_TYPE_MISC2:
		Message(Chat::Red, fmt::format("{} This spell will only work on humanoids, lycanthropes, giants, Kael Drakkel giants, Coldain, animals, or insects.", msg).c_str());
		break;
	case IS_PLANT:
		Message(Chat::Red, fmt::format("{} This spell will only work on plants.", msg).c_str());
		break;
	case IS_GIANT:
		Message(Chat::Red, fmt::format("{} This spell will only work on animals.", msg).c_str());
		break;
	case IS_NOT_ANIMAL_OR_HUMANOID:
		Message(Chat::Red, fmt::format("{} This spell will only work on targets that are neither animals or humanoid.", msg).c_str());
		break;
	case IS_BIXIE:
		Message(Chat::Red, fmt::format("{} This spell will only work on bixies.", msg).c_str());
		break;
	case IS_HARPY:
		Message(Chat::Red, fmt::format("{} This spell will only work on harpies.", msg).c_str());
		break;
	case IS_GNOLL:
		Message(Chat::Red, fmt::format("{} This spell will only work on gnolls.", msg).c_str());
		break;
	case IS_SPORALI:
		Message(Chat::Red, fmt::format("{} This spell will only work on fungusoids.", msg).c_str());
		break;
	case IS_KOBOLD:
		Message(Chat::Red, fmt::format("{} This spell will only work on kobolds.", msg).c_str());
		break;
	case IS_FROSTCRYPT_SHADE:
		Message(Chat::Red, fmt::format("{} This spell will only work on undead creatures or the Shades of Frostcrypt.", msg).c_str());
		break;
	case IS_DRAKKIN:
		Message(Chat::Red, fmt::format("{} This spell will only work on Drakkin.", msg).c_str());
		break;
	case IS_UNDEAD_OR_VALDEHOLM_GIANT:
		Message(Chat::Red, fmt::format("{} This spell will only work on undead creatures or the inhabitants of Valdeholm.", msg).c_str());
		break;
	case IS_ANIMAL_OR_PLANT:
		Message(Chat::Red, fmt::format("{} This spell will only work on plants or animals.", msg).c_str());
		break;
	case IS_SUMMONED:
		Message(Chat::Red, fmt::format("{} This spell will only work on constructs, elementals, or summoned elemental minions.", msg).c_str());
		break;
	case IS_WIZARD_USED_ON_MAGE_FIRE_PET:
		Message(Chat::Red, fmt::format("{} This spell will only work on wizards.", msg).c_str());
		break;
	case IS_UNDEAD:
		Message(Chat::Red, fmt::format("{} This spell will only work on undead.", msg).c_str());
		break;
	case IS_NOT_UNDEAD_OR_SUMMONED_OR_VAMPIRE:
		Message(Chat::Red, fmt::format("{} This spell will only work on creatures that are not undead, constructs, elementals, or vampires.", msg).c_str());
		break;
	case IS_FAE_OR_PIXIE:
		Message(Chat::Red, fmt::format("{} This spell will only work on Fae or pixies.", msg).c_str());
		break;
	case IS_HUMANOID:
		Message(Chat::Red, fmt::format("{} This spell will only work on humanoids.", msg).c_str());
		break;
	case IS_UNDEAD_AND_HP_LESS_THAN_10_PCT:
		Message(Chat::Red, fmt::format("{} The Essence Extractor whirrs but does not light up.", msg).c_str());
		break;
	case IS_CLOCKWORK_AND_HP_LESS_THAN_45_PCT:
		Message(Chat::Red, fmt::format("{} This spell will only work on clockwork gnomes.", msg).c_str());
		break;
	case IS_WISP_AND_HP_LESS_THAN_10_PCT:
		Message(Chat::Red, fmt::format("{} This spell will only work on wisps at or below 10 pct of their maximum HP.", msg).c_str());
		break;
	case IS_CLASS_MELEE_THAT_CAN_BASH_OR_KICK_EXCEPT_BARD:
		Message(Chat::Red, fmt::format("{} This spell will only work on non-bard targets that can bash or kick.", msg).c_str());
		break;
	case IS_CLASS_PURE_MELEE:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} This spell will only affect melee classes (warriors, monks, rogues, and berserkers).", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability can only be used by melee classes (warriors, monks, rogues, and berserkers).", msg).c_str());
		}
		break;
	case IS_CLASS_PURE_CASTER:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} This spell will only affect pure caster classes (necromancers, wizards, magicians, and enchanters).", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability can only be used by pure caster classes (necromancers, wizards, magicians, and enchanters).", msg).c_str());
		}
		break;
	case IS_CLASS_HYBRID_CLASS:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} This spell will only affect hybrid classes (paladins, rangers, shadow knights, bards, and beastlords).", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability can only be used by hybrid classes (paladins, rangers, shadow knights, bards, and beastlords).", msg).c_str());
		}
		break;
	case IS_CLASS_WARRIOR:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} This spell will only affect Warriors.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability can only be used by Warriors.", msg).c_str());
		}
		break;
	case IS_CLASS_CLERIC:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} This spell will only affect Clerics.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability can only be used by Clerics.", msg).c_str());
		}
		break;
	case IS_CLASS_PALADIN:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} This spell will only affect Paladins.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability can only be used by Paladins.", msg).c_str());
		}
		break;
	case IS_CLASS_RANGER:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} This spell will only affect Warriors.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability can only be used by Warriors.", msg).c_str());
		}
		break;
	case IS_CLASS_SHADOWKNIGHT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} This spell will only affect Shadow Knights.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability can only be used by Shadow Knights.", msg).c_str());
		}
		break;
	case IS_CLASS_DRUID:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} This spell will only affect Druids.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability can only be used by Druids.", msg).c_str());
		}
		break;
	case IS_CLASS_MONK:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} This spell will only affect Monks.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability can only be used by Monks.", msg).c_str());
		}
		break;
	case IS_CLASS_BARD:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} This spell will only affect Bards.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability can only be used by Bards..", msg).c_str());
		}
		break;
	case IS_CLASS_ROGUE:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} This spell will only affect Rogues.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability can only be used by Rogues.", msg).c_str());
		}
		break;
	case IS_CLASS_SHAMAN:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} This spell will only affect Shamans.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability can only be used by Shamans.", msg).c_str());
		}
		break;
	case IS_CLASS_NECRO:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} This spell will only affect Necromancers.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability can only be used by Necromancers.", msg).c_str());
		}
		break;
	case IS_CLASS_WIZARD:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} This spell will only affect Wizards.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability can only be used by Wizards.", msg).c_str());
		}
		break;
	case IS_CLASS_MAGE:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} This spell will only affect Magicians.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability can only be used by Magicians.", msg).c_str());
		}
		break;
	case IS_CLASS_ENCHANTER:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} This spell will only affect Enchanters.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability can only be used by Enchanters.", msg).c_str());
		}
		break;
	case IS_CLASS_BEASTLORD:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} This spell will only affect Beastlords.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability can only be used by Beastlords.", msg).c_str());
		}
		break;
	case IS_CLASS_BERSERKER:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} This spell will only affect Berserkers.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability can only be used by Berserkers.", msg).c_str());
		}
		break;
	case IS_CLASS_CLR_SHM_DRU:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} This spell will only affect priest classes (clerics, druids, and shaman).", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability can only be used by priest classes (clerics, druids, and shaman).", msg).c_str());
		}
		break;
	case IS_CLASS_NOT_WAR_PAL_SK:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} This spell will not affect Warriors, Paladins, or Shadow Knights.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability can only be used by Warriors, Paladins, or Shadow Knights.", msg).c_str());
		}
		break;
	case IS_LEVEL_UNDER_100:
		Message(Chat::Red, fmt::format("{} This spell will not affect any target over level 100.", msg).c_str());
		break;
	case IS_NOT_RAID_BOSS:
		Message(Chat::Red, fmt::format("{} This spell will not affect raid bosses.", msg).c_str());
		break;
	case IS_RAID_BOSS:
		Message(Chat::Red, fmt::format("{} This spell will only affect raid bosses.", msg).c_str());
		break;
	case FRENZIED_BURNOUT_ACTIVE:
		Message(Chat::Red, fmt::format("{} This spell will only cast if you have Frenzied Burnout active.", msg).c_str());
		break;
	case FRENZIED_BURNOUT_NOT_ACTIVE:
		Message(Chat::Red, fmt::format("{} This spell will only cast if you do not have Frenzied Burnout active.", msg).c_str());
		break;
	case IS_HP_ABOVE_75_PCT:
		Message(Chat::Red, fmt::format("{} Your taret's HP must be at or above 75 pct of its maximum.", msg).c_str());
		break;
	case IS_HP_LESS_THAN_20_PCT:
		Message(Chat::Red, fmt::format("{} Your target's HP must be at 20 pct of its maximum or below.", msg).c_str());
		break;
	case IS_HP_LESS_THAN_50_PCT:
		Message(Chat::Red, fmt::format("{} Your target's HP must be at 50 pct of its maximum or below.", msg).c_str());
		break;
	case IS_HP_LESS_THAN_75_PCT:
		Message(Chat::Red, fmt::format("{} Your target's HP must be at 75 pct of its maximum or below.", msg).c_str());
		break;
	case IS_NOT_IN_COMBAT:
		Message(Chat::Red, fmt::format("{} This spell will only affect creatures that are not in combat.", msg).c_str());
		break;
	case HAS_AT_LEAST_1_PET_ON_HATELIST:
		Message(Chat::Red, fmt::format("{} Your target must have least 1 pet on their hate list.", msg).c_str());
		break;
	case HAS_AT_LEAST_2_PETS_ON_HATELIST:
		Message(Chat::Red, fmt::format("{} Your target must have least 2 pets on their hate list.", msg).c_str());
		break;
	case HAS_AT_LEAST_3_PETS_ON_HATELIST:
		Message(Chat::Red, fmt::format("{} Your target must have least 3 pets on their hate list.", msg).c_str());
		break;
	case HAS_AT_LEAST_4_PETS_ON_HATELIST:
		Message(Chat::Red, fmt::format("{} Your target must have least 4 pets on their hate list.", msg).c_str());
		break;
	case HAS_AT_LEAST_5_PETS_ON_HATELIST:
		Message(Chat::Red, fmt::format("{} Your target must have least 5 pets on their hate list.", msg).c_str());
		break;
	case HAS_AT_LEAST_6_PETS_ON_HATELIST:
		Message(Chat::Red, fmt::format("{} Your target must have least 6 pets on their hate list.", msg).c_str());
		break;
	case HAS_AT_LEAST_7_PETS_ON_HATELIST:
		Message(Chat::Red, fmt::format("{} Your target must have least 7 pets on their hate list.", msg).c_str());
		break;
	case HAS_AT_LEAST_8_PETS_ON_HATELIST:
		Message(Chat::Red, fmt::format("{} Your target must have least 8 pets on their hate list.", msg).c_str());
		break;
	case HAS_AT_LEAST_9_PETS_ON_HATELIST:
		Message(Chat::Red, fmt::format("{} Your target must have least 9 pets on their hate list.", msg).c_str());
		break;
	case HAS_AT_LEAST_10_PETS_ON_HATELIST:
		Message(Chat::Red, fmt::format("{} Your target must have least 10 pets on their hate list.", msg).c_str());
		break;
	case HAS_AT_LEAST_11_PETS_ON_HATELIST:
		Message(Chat::Red, fmt::format("{} Your target must have least 11 pets on their hate list.", msg).c_str());
		break;
	case HAS_AT_LEAST_12_PETS_ON_HATELIST:
		Message(Chat::Red, fmt::format("{} Your target must have least 12 pets on their hate list.", msg).c_str());
		break;
	case HAS_AT_LEAST_13_PETS_ON_HATELIST:
		Message(Chat::Red, fmt::format("{} Your target must have least 13 pets on their hate list.", msg).c_str());
		break;
	case HAS_AT_LEAST_14_PETS_ON_HATELIST:
		Message(Chat::Red, fmt::format("{} Your target must have least 14 pets on their hate list.", msg).c_str());
		break;
	case HAS_AT_LEAST_15_PETS_ON_HATELIST:
		Message(Chat::Red, fmt::format("{} Your target must have least 15 pets on their hate list.", msg).c_str());
		break;
	case HAS_AT_LEAST_16_PETS_ON_HATELIST:
		Message(Chat::Red, fmt::format("{} Your target must have least 16 pets on their hate list.", msg).c_str());
		break;
	case HAS_AT_LEAST_17_PETS_ON_HATELIST:
		Message(Chat::Red, fmt::format("{} Your target must have least 17 pets on their hate list.", msg).c_str());
		break;
	case HAS_AT_LEAST_18_PETS_ON_HATELIST:
		Message(Chat::Red, fmt::format("{} Your target must have least 18 pets on their hate list.", msg).c_str());
		break;
	case HAS_AT_LEAST_19_PETS_ON_HATELIST:
		Message(Chat::Red, fmt::format("{} Your target must have least 19 pets on their hate list.", msg).c_str());
		break;
	case HAS_AT_LEAST_20_PETS_ON_HATELIST:
		Message(Chat::Red, fmt::format("{} Your target must have least 20 pets on their hate list.", msg).c_str());
		break;
	case IS_HP_LESS_THAN_35_PCT:
		Message(Chat::Red, fmt::format("{} Your target's HP must be at 35 pct of its maximum or below.", msg).c_str());
		break;
	case HAS_BETWEEN_1_TO_2_PETS_ON_HATELIST:
		Message(Chat::Red, fmt::format("{} Your target must have between 1 and 2 pets on their hate list.", msg).c_str());
		break;
	case HAS_BETWEEN_3_TO_5_PETS_ON_HATELIST:
		Message(Chat::Red, fmt::format("{} Your target must have between 3 and 5 pets on their hate list.", msg).c_str());
		break;
	case HAS_BETWEEN_6_TO_9_PETS_ON_HATELIST:
		Message(Chat::Red, fmt::format("{} Your target must have between 6 and 9 pets on their hate list.", msg).c_str());
		break;
	case HAS_BETWEEN_10_TO_14_PETS_ON_HATELIST:
		Message(Chat::Red, fmt::format("{} Your target must have between 10 and 14 pets on their hate list.", msg).c_str());
		break;
	case HAS_MORE_THAN_14_PETS_ON_HATELIST:
		Message(Chat::Red, fmt::format("{} Your target must have between 15 or more pets on their hate list.", msg).c_str());
		break;
	case IS_CLASS_CHAIN_OR_PLATE:
		Message(Chat::Red, fmt::format("{} This spell will only affect plate or chain wearing classes.", msg).c_str());
		break;
	case IS_HP_BETWEEN_5_AND_9_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be between 5 pct and 9 pct of its maximum.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be between 5 pct and 9 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BETWEEN_10_AND_14_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be between 10 pct and 14 pct of its maximum.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be between 10 pct and 14 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BETWEEN_15_AND_19_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be between 15 pct and 19 pct of its maximum.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be between 15 pct and 19 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BETWEEN_20_AND_24_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be between 20 pct and 54 pct of its maximum.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be between 20 pct and 24 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BETWEEN_25_AND_29_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be between 25 pct and 29 pct of its maximum.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be between 25 pct and 29 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BETWEEN_30_AND_34_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be between 30 pct and 34 pct of its maximum.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be between 30 pct and 34 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BETWEEN_35_AND_39_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be between 35 pct and 39 pct of its maximum.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be between 35 pct and 39 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BETWEEN_40_AND_44_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be between 40 pct and 44 pct of its maximum.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be between 40 pct and 44 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BETWEEN_45_AND_49_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be between 45 pct and 49 pct of its maximum.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be between 45 pct and 49 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BETWEEN_50_AND_54_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be between 50 pct and 54 pct of its maximum.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be between 50 pct and 54 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BETWEEN_55_AND_59_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be between 55 pct and 59 pct of its maximum.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be between 55 pct and 59 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BETWEEN_5_AND_15_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be between 5 pct and 15 pct of its maximum.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be between 5 pct and 15 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BETWEEN_15_AND_25_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be between 15 pct and 25 pct of its maximum.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be between 15 pct and 25 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BETWEEN_1_AND_25_PCT:
		Message(Chat::Red, fmt::format("{} Your target's HP must be at 25 pct of its maximum or below.", msg).c_str());
		break;
	case IS_HP_BETWEEN_25_AND_35_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be between 25 pct and 35 pct of its maximum.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be between 25 pct and 35 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BETWEEN_35_AND_45_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be between 35 pct and 45 pct of its maximum.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be between 35 pct and 45 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BETWEEN_45_AND_55_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be between 45 pct and 55 pct of its maximum.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be between 45 pct and 55 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BETWEEN_55_AND_65_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be between 55 pct and 65 pct of its maximum.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be between 55 pct and 65 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BETWEEN_65_AND_75_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be between 65 pct and 75 pct of its maximum.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be between 65 pct and 75 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BETWEEN_75_AND_85_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be between 75 pct and 85 pct of its maximum.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be between 75 pct and 85 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BETWEEN_85_AND_95_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be between 85 pct and 95 pct of its maximum.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be between 85 pct and 95 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_ABOVE_45_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be at least 45 pct of its maximum.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be at least 45 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_ABOVE_55_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be at least 55 pct of its maximum.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be at least 55 pct of your maximum HP.", msg).c_str());
		}
		break;
	case UNKNOWN_TOO_MUCH_HP_410:
		Message(Chat::Red, fmt::format("{} Your target has too much HP to be affected by this spell.", msg).c_str());
		break;
	case UNKNOWN_TOO_MUCH_HP_411:
		Message(Chat::Red, fmt::format("{} Your target has too much HP to be affected by this spell.", msg).c_str());
		break;
	case IS_HP_ABOVE_99_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be at or above 99 pct of its maximum.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be at or above 99 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_MANA_ABOVE_10_PCT:
		Message(Chat::Red, fmt::format("{} You must have at least 10 pct of your maximum mana available to cast this spell.", msg).c_str());
		break;
	case IS_HP_BELOW_5_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be at 5 pct of its maximum or below.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be at or below 5 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BELOW_10_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be at 10 pct of its maximum or below.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be at or below 10 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BELOW_15_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be at 15 pct of its maximum or below.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be at or below 15 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BELOW_20_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be at 20 pct of its maximum or below.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be at or below 20 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BELOW_25_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be at 25 pct of its maximum or below.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be at or below 25 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BELOW_30_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be at 30 pct of its maximum or below.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be at or below 30 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BELOW_35_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be at 35 pct of its maximum or below.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be at or below 35 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BELOW_40_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be at 40 pct of its maximum or below.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be at or below 40 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BELOW_45_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be at 45 pct of its maximum or below.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be at or below 45 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BELOW_50_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be at 50 pct of its maximum or below.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be at or below 50 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BELOW_55_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be at 55 pct of its maximum or below.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be at or below 55 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BELOW_60_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be at 60 pct of its maximum or below.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be at or below 60 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BELOW_65_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be at 65 pct of its maximum or below.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be at or below 65 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BELOW_70_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be at 70 pct of its maximum or below.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be at or below 70 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BELOW_75_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be at 75 pct of its maximum or below.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be at or below 75 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BELOW_80_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be at 80 pct of its maximum or below.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be at or below 80 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BELOW_85_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be at 85 pct of its maximum or below.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be at or below 85 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BELOW_90_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be at 90 pct of its maximum or below.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be at or below 90 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_HP_BELOW_95_PCT:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} Your target's HP must be at 95 pct of its maximum or below.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability requires you to be at or below 95 pct of your maximum HP.", msg).c_str());
		}
		break;
	case IS_ENDURANCE_BELOW_40_PCT:
		Message(Chat::Red, fmt::format("{} This ability requires you to be at or below 40 pct of your maximum endurance.", msg).c_str());
		break;
	case IS_MANA_BELOW_40_PCT:
		Message(Chat::Red, fmt::format("{} This spells requires you to be at or below 40 pct of your maximum mana.", msg).c_str());
		break;
	case IS_HP_ABOVE_20_PCT:
		Message(Chat::Red, fmt::format("{} Your target's HP must be at least 21 pct of its maximum.", msg).c_str());
		break;
	case IS_BODY_TYPE_UNDEFINED:
		Message(Chat::Red, fmt::format("{} This spell will only work on creatures with an undefined body type.", msg).c_str());
		break;
	case IS_BODY_TYPE_HUMANOID:
		Message(Chat::Red, fmt::format("{} This spell will only work on humanoid creatures.", msg).c_str());
		break;
	case IS_BODY_TYPE_WEREWOLF:
		Message(Chat::Red, fmt::format("{} This spell will only work on lycanthrope creatures.", msg).c_str());
		break;
	case IS_BODY_TYPE_UNDEAD:
		Message(Chat::Red, fmt::format("{} This spell will only work on undead creatures.", msg).c_str());
		break;
	case IS_BODY_TYPE_GIANTS:
		Message(Chat::Red, fmt::format("{} This spell will only work on giants.", msg).c_str());
		break;
	case IS_BODY_TYPE_CONSTRUCTS:
		Message(Chat::Red, fmt::format("{} This spell will only work on constructs.", msg).c_str());
		break;
	case IS_BODY_TYPE_EXTRAPLANAR:
		Message(Chat::Red, fmt::format("{} This spell will only work on extraplanar creatures.", msg).c_str());
		break;
	case IS_BODY_TYPE_MAGICAL_CREATURE:
		Message(Chat::Red, fmt::format("{} This spell will only work on creatures constructed from magic.", msg).c_str());
		break;
	case IS_BODY_TYPE_UNDEADPET:
		Message(Chat::Red, fmt::format("{} This spell will only work on animated undead servants.", msg).c_str());
		break;
	case IS_BODY_TYPE_KAELGIANT:
		Message(Chat::Red, fmt::format("{} This spell will only work on the Giants of Kael Drakkal.", msg).c_str());
		break;
	case IS_BODY_TYPE_COLDAIN:
		Message(Chat::Red, fmt::format("{} This spell will only work on Coldain Dwarves.", msg).c_str());
		break;
	case IS_BODY_TYPE_VAMPIRE:
		Message(Chat::Red, fmt::format("{} This spell will only work on vampires.", msg).c_str());
		break;
	case IS_BODY_TYPE_ATEN_HA_RA:
		Message(Chat::Red, fmt::format("{} This spell will only work on Aten Ha Ra.", msg).c_str());
		break;
	case IS_BODY_TYPE_GREATER_AHKEVANS:
		Message(Chat::Red, fmt::format("{} This spell will only work on Greater Ahkevans.", msg).c_str());
		break;
	case IS_BODY_TYPE_KHATI_SHA:
		Message(Chat::Red, fmt::format("{} This spell will only work on Khati Sha.", msg).c_str());
		break;
	case IS_BODY_TYPE_LORD_INQUISITOR_SERU:
		Message(Chat::Red, fmt::format("{} This spell will only work on Lord Inquisitor Seru.", msg).c_str());
		break;
	case IS_BODY_TYPE_GRIEG_VENEFICUS:
		Message(Chat::Red, fmt::format("{} This spell will only work on Grieg Veneficus.", msg).c_str());
		break;
	case IS_BODY_TYPE_FROM_PLANE_OF_WAR:
		Message(Chat::Red, fmt::format("{} This spell will only work on creatures from the Plane of War.", msg).c_str());
		break;
	case IS_BODY_TYPE_LUGGALD:
		Message(Chat::Red, fmt::format("{} This spell will only work on Luggalds.", msg).c_str());
		break;
	case IS_BODY_TYPE_ANIMAL:
		Message(Chat::Red, fmt::format("{} This spell will only work on animals.", msg).c_str());
		break;
	case IS_BODY_TYPE_INSECT:
		Message(Chat::Red, fmt::format("{} This spell will only work on insects.", msg).c_str());
		break;
	case IS_BODY_TYPE_MONSTER:
		Message(Chat::Red, fmt::format("{} This spell will only work on monsters.", msg).c_str());
		break;
	case IS_BODY_TYPE_ELEMENTAL:
		Message(Chat::Red, fmt::format("{} This spell will only work on elemental creatures.", msg).c_str());
		break;
	case IS_BODY_TYPE_PLANT:
		Message(Chat::Red, fmt::format("{} This spell will only work on plants.", msg).c_str());
		break;
	case IS_BODY_TYPE_DRAGON2:
		Message(Chat::Red, fmt::format("{} This spell will only work on dragons.", msg).c_str());
		break;
	case IS_BODY_TYPE_SUMMONED_ELEMENTAL:
		Message(Chat::Red, fmt::format("{} This spell will only work on summoned elementals.", msg).c_str());
		break;
	case IS_BODY_TYPE_DRAGON_OF_TOV:
		Message(Chat::Red, fmt::format("{} This spell will only work on Dragons of Veeshan's Temple.", msg).c_str());
		break;
	case IS_BODY_TYPE_FAMILIAR:
		Message(Chat::Red, fmt::format("{} This spell will only work on familiars.", msg).c_str());
		break;
	case IS_BODY_TYPE_MURAMITE:
		Message(Chat::Red, fmt::format("{} This spell will only work on Muramites.", msg).c_str());
		break;
	case IS_NOT_UNDEAD_OR_SUMMONED:
		Message(Chat::Red, fmt::format("{} This spell will only targets that are not undead or summoned.", msg).c_str());
		break;
	case IS_NOT_PLANT:
		Message(Chat::Red, fmt::format("{} This spell will not affect plants.", msg).c_str());
		break;
	case IS_NOT_CLIENT:
		Message(Chat::Red, fmt::format("{} This spell will not work on adventurers.", msg).c_str());
		break;
	case IS_CLIENT:
		Message(Chat::Red, fmt::format("{} This spell will only work on adventurers.", msg).c_str());
		break;
	case IS_LEVEL_ABOVE_42_AND_IS_CLIENT:
		Message(Chat::Red, fmt::format("{} This spell will only work on level 43 or higher adventurers.", msg).c_str());
		break;
	case IS_TREANT:
		Message(Chat::Red, fmt::format("{} This spell will only work on treants.", msg).c_str());
		break;
	case IS_BIXIE2:
		Message(Chat::Red, fmt::format("{} This spell will only work on bixies.", msg).c_str());
		break;
	case IS_SCARECROW:
		Message(Chat::Red, fmt::format("{} This spell will only work on scarecrows.", msg).c_str());
		break;
	case IS_VAMPIRE_OR_UNDEAD_OR_UNDEADPET:
		Message(Chat::Red, fmt::format("{} This spell will only work on vampires, undead, or animated undead creatures.", msg).c_str());
		break;
	case IS_NOT_VAMPIRE_OR_UNDEAD:
		Message(Chat::Red, fmt::format("{} This spell will not work on vampires or undead creatures.", msg).c_str());
		break;
	case IS_CLASS_KNIGHT_HYBRID_MELEE:
		Message(Chat::Red, fmt::format("{} This spell will only work on knights, hybrids, or melee classes.", msg).c_str());
		break;
	case IS_CLASS_WARRIOR_CASTER_PRIEST:
		Message(Chat::Red, fmt::format("{} This spell will only work on warriors, casters, or priests.", msg).c_str());
		break;
	case IS_END_BELOW_21_PCT:
		Message(Chat::Red, fmt::format("{} This ability requires you to be at or below 21 pct of your maximum endurance.", msg).c_str());
		break;
	case IS_END_BELOW_25_PCT:
		Message(Chat::Red, fmt::format("{} This ability requires you to be at or below 25 pct of your maximum endurance.", msg).c_str());
		break;
	case IS_END_BELOW_29_PCT:
		Message(Chat::Red, fmt::format("{} This ability requires you to be at or below 29 pct of your maximum endurance.", msg).c_str());
		break;
	case IS_HUMANOID_LEVEL_84_MAX:
		Message(Chat::Red, fmt::format("{} This spell will only work on humanoids up to level 84.", msg).c_str());
		break;
	case IS_HUMANOID_LEVEL_86_MAX:
		Message(Chat::Red, fmt::format("{} This spell will only work on humanoids up to level 86.", msg).c_str());
		break;
	case IS_HUMANOID_LEVEL_88_MAX:
		Message(Chat::Red, fmt::format("{} This spell will only work on humanoids up to level 88.", msg).c_str());
		break;
	case HAS_CRYSTALLIZED_FLAME_BUFF:
		Message(Chat::Red, fmt::format("{} This spell will only work on targets afflicted by Crystallized Flame.", msg).c_str());
		break;
	case HAS_INCENDIARY_OOZE_BUFF:
		Message(Chat::Red, fmt::format("{} This spell will only work on targets afflicted by Incendiary Ooze.", msg).c_str());
		break;
	case IS_LEVEL_90_MAX:
		Message(Chat::Red, fmt::format("{} This spell will ony work targets level up to level 90.", msg).c_str());
		break;
	case IS_LEVEL_92_MAX:
		Message(Chat::Red, fmt::format("{} This spell will ony work targets level up to level 92.", msg).c_str());
		break;
	case IS_LEVEL_94_MAX:
		Message(Chat::Red, fmt::format("{} This spell will ony work targets level up to level 94.", msg).c_str());
		break;
	case IS_LEVEL_95_MAX:
		Message(Chat::Red, fmt::format("{} This spell will ony work targets level up to level 95.", msg).c_str());
		break;
	case IS_LEVEL_97_MAX:
		Message(Chat::Red, fmt::format("{} This spell will ony work targets level up to level 97.", msg).c_str());
		break;
	case IS_LEVEL_99_MAX:
		Message(Chat::Red, fmt::format("{} This spell will ony work targets level up to level 99.", msg).c_str());
		break;
	case IS_LEVEL_100_MAX:
		Message(Chat::Red, fmt::format("{} This spell will ony work targets level up to level 100.", msg).c_str());
		break;
	case IS_LEVEL_102_MAX:
		Message(Chat::Red, fmt::format("{} This spell will ony work targets level up to level 102.", msg).c_str());
		break;
	case IS_LEVEL_104_MAX:
		Message(Chat::Red, fmt::format("{} This spell will ony work targets level up to level 104.", msg).c_str());
		break;
	case IS_LEVEL_105_MAX:
		Message(Chat::Red, fmt::format("{} This spell will ony work targets level up to level 105.", msg).c_str());
		break;
	case IS_LEVEL_107_MAX:
		Message(Chat::Red, fmt::format("{} This spell will ony work targets level up to level 107.", msg).c_str());
		break;
	case IS_LEVEL_109_MAX:
		Message(Chat::Red, fmt::format("{} This spell will ony work targets level up to level 109.", msg).c_str());
		break;
	case IS_LEVEL_110_MAX:
		Message(Chat::Red, fmt::format("{} This spell will ony work targets level up to level 110.", msg).c_str());
		break;
	case IS_LEVEL_112_MAX:
		Message(Chat::Red, fmt::format("{} This spell will ony work targets level up to level 112.", msg).c_str());
		break;
	case IS_LEVEL_114_MAX:
		Message(Chat::Red, fmt::format("{} This spell will ony work targets level up to level 114.", msg).c_str());
		break;
	case HAS_TBL_ESIANTI_ACCESS:
		Message(Chat::Red, fmt::format("{} This spell will only transport adventurers who have gained access to Esianti: Palace of the Winds.", msg).c_str());
		break;
	case IS_BETWEEN_LEVEL_1_AND_75:
		Message(Chat::Red, fmt::format("{} This spell will only work on targets between level 1 and 75.", msg).c_str());
		break;
	case IS_BETWEEN_LEVEL_76_AND_85:
		Message(Chat::Red, fmt::format("{} This spell will only work on targets between level 76 and 85.", msg).c_str());
		break;
	case IS_BETWEEN_LEVEL_86_AND_95:
		Message(Chat::Red, fmt::format("{} This spell will only work on targets between level 86 and 95.", msg).c_str());
		break;
	case IS_BETWEEN_LEVEL_96_AND_105:
		Message(Chat::Red, fmt::format("{} This spell will only work on targets between level 96 and 105.", msg).c_str());
		break;
	case IS_HP_LESS_THAN_80_PCT:
		Message(Chat::Red, fmt::format("{} Your target's HP must be at 80 pct of its maximum or below.", msg).c_str());
		break;
	case IS_LEVEL_ABOVE_34:
		Message(Chat::Red, fmt::format("{} Your target must be level 35 or higher.", msg).c_str());
		break;
	case IN_TWO_HANDED_STANCE:
		Message(Chat::Red, fmt::format("{} You must be in your two-handed stance to use this ability.", msg).c_str());
		break;
	case IN_DUAL_WIELD_HANDED_STANCE:
		Message(Chat::Red, fmt::format("{} You must be in your dual-wielding stance to use this ability.", msg).c_str());
		break;
	case IN_SHIELD_STANCE:
		Message(Chat::Red, fmt::format("{} You must be in your shield stance to use this ability.", msg).c_str());
		break;
	case NOT_IN_TWO_HANDED_STANCE:
		Message(Chat::Red, fmt::format("{} You may not use this ability if you are in your two-handed stance.", msg).c_str());
		break;
	case NOT_IN_DUAL_WIELD_HANDED_STANCE:
		Message(Chat::Red, fmt::format("{} You may not use this ability if you are in your dual-wielding stance.", msg).c_str());
		break;
	case NOT_IN_SHIELD_STANCE:
		Message(Chat::Red, fmt::format("{} You may not use this ability if you are in your shield stance.", msg).c_str());
		break;
	case LEVEL_46_MAX:
		Message(Chat::Red, fmt::format("{} Your target must be level 46 or below.", msg).c_str());
		break;
	case HAS_NO_MANA_BURN_BUFF:
		Message(Chat::Red, fmt::format("{} This spell will not take hold until the effects of the previous Mana Burn have expired.", msg).c_str());
		break;
	case IS_CLIENT_AND_MALE_PLATE_USER:
		Message(Chat::Red, fmt::format("{} Your target wouldn't look right as that Jann.", msg).c_str());
		break;
	case IS_CLEINT_AND_MALE_DRUID_ENCHANTER_MAGICIAN_NECROANCER_SHAMAN_OR_WIZARD:
		Message(Chat::Red, fmt::format("{} Your target wouldn't look right as that Jann.", msg).c_str());
		break;
	case IS_CLIENT_AND_MALE_BEASTLORD_BERSERKER_MONK_RANGER_OR_ROGUE:
		Message(Chat::Red, fmt::format("{} Your target wouldn't look right as that Jann.", msg).c_str());
		break;
	case IS_CLIENT_AND_FEMALE_PLATE_USER:
		Message(Chat::Red, fmt::format("{} Your target wouldn't look right as that Jann.", msg).c_str());
		break;
	case IS_CLIENT_AND_FEMALE_DRUID_ENCHANTER_MAGICIAN_NECROANCER_SHAMAN_OR_WIZARD:
		Message(Chat::Red, fmt::format("{} Your target wouldn't look right as that Jann.", msg).c_str());
		break;
	case IS_CLIENT_AND_FEMALE_BEASTLORD_BERSERKER_MONK_RANGER_OR_ROGUE:
		Message(Chat::Red, fmt::format("{} Your target wouldn't look right as that Jann.", msg).c_str());
		break;
	case HAS_TRAVELED_TO_STRATOS:
		Message(Chat::Red, fmt::format("{} You must travel to Stratos at least once before wishing to go there.", msg).c_str());
		break;
	case HAS_TRAVELED_TO_AALISHAI:
		Message(Chat::Red, fmt::format("{} You must travel to Aalishai at least once before wishing to go there.", msg).c_str());
		break;
	case HAS_TRAVELED_TO_MEARATS:
		Message(Chat::Red, fmt::format("{} You must travel to Mearatas at least once before wishing to go there.", msg).c_str());
		break;
	case IS_HP_ABOVE_50_PCT:
		Message(Chat::Red, fmt::format("{} This target must be above 50 pct of its maximum hit points.", msg).c_str());
		break;
	case IS_HP_UNDER_50_PCT:
		Message(Chat::Red, fmt::format("{}  This target must be at oe below 50 pct of its maximum hit points.", msg).c_str());
		break;
	case IS_OFF_HAND_EQUIPPED:
		Message(Chat::Red, fmt::format("{} You must be wielding a weapon or shield in your offhand to use this ability.", msg).c_str());
		break;
	case HAS_NO_PACT_OF_FATE_RECOURSE_BUFF:
		Message(Chat::Red, fmt::format("{} This spell will not work while Pact of Fate Recourse is active.", msg).c_str());
		break;
	case HAS_NO_SHROUD_OF_PRAYER_BUFF:
		Message(Chat::Red, fmt::format("{} Your target cannot receive another Quiet Prayer this soon.", msg).c_str());
		break;
	case IS_MANA_BELOW_20_PCT:
		Message(Chat::Red, fmt::format("{} This ability requires you to be at or below 20 pct of your maximum mana.", msg).c_str());
		break;
	case IS_MANA_ABOVE_50_PCT:
		Message(Chat::Red, fmt::format("{} This ability requires you to be at or above 50 pct of your maximum mana.", msg).c_str());
		break;
	case COMPLETED_ACHIEVEMENT_LEGENDARY_ANSWERER:
		Message(Chat::Red, fmt::format("{} You have completed Legendary Answerer.", msg).c_str());
		break;
	case HAS_NO_ROGUES_FURY_BUFF:
		Message(Chat::Red, fmt::format("{} This spell will not affect anyone that currently has Rogue's Fury active.", msg).c_str());
		break;
	case NOT_COMPLETED_ACHIEVEMENT_LEGENDARY_ANSWERER:
		Message(Chat::Red, fmt::format("{} You must complete Legendary Answerer.", msg).c_str());
		break;
	case IS_SUMMONED_OR_UNDEAD:
		Message(Chat::Red, fmt::format("{} This spell can only be used on summoned or undead.", msg).c_str());
		break;
	case IS_CLASS_CASTER_PRIEST:
		Message(Chat::Red, fmt::format("{} This ability requires you to be a caster or priest.", msg).c_str());
		break;
	case IS_END_OR_MANA_ABOVE_20_PCT:
		Message(Chat::Red, fmt::format("{} You must have at least 20 pct of your maximum mana and endurance to use this ability.", msg).c_str());
		break;
	case IS_END_OR_MANA_BELOW_30_PCT:
		Message(Chat::Red, fmt::format("{} Your target already has 30 pct or more of their maximum mana or endurance.", msg).c_str());
		break;
	case IS_CLASS_BARD2:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} This spell will only affect Bards.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability can only be used by Bards.", msg).c_str());
		}
		break;
	case IS_NOT_CLASS_BARD:
		if (target_requirement) {
			Message(Chat::Red, fmt::format("{} This spell can not affect Bards.", msg).c_str());
		}
		else {
			Message(Chat::Red, fmt::format("{} This ability can not be used by Bards.", msg).c_str());
		}
		break;
	case HAS_NO_FURIOUS_RAMPAGE_BUFF:
		Message(Chat::Red, fmt::format("{} This ability cannot be activated while Furious Rampage is active.", msg).c_str());
		break;
	case IS_END_OR_MANA_BELOW_30_PCT2:
		Message(Chat::Red, fmt::format("{} You can only perform this solo if you have less than 30 pct mana or endurance.", msg).c_str());
		break;
	case HAS_NO_HARMONIOUS_PRECISION_BUFF:
		Message(Chat::Red, fmt::format("{} This spell will not work if you have the Harmonious Precision line active.", msg).c_str());
		break;
	case HAS_NO_HARMONIOUS_EXPANSE_BUFF:
		Message(Chat::Red, fmt::format("{} This spell will not work if you have the Harmonious Expanse line active.", msg).c_str());
		break;
	default:
		if (target_requirement) {
			Message(Chat::Red, "Your target does not meet the spell requirements.");
		}
		else {
			Message(Chat::Red, "Your spell would not take hold on your target.");
		}
		break;
	}
}

bool Mob::TrySpellProjectile(Mob* spell_target, uint16 spell_id, float speed) {

	/*For mage 'Bolt' line and other various spells.
	-This is mostly accurate for how the modern clients handle this effect.
	-It was changed at some point to use an actual projectile as done here (opposed to a particle effect in classic)
	-There is no LOS check to prevent a bolt from being cast. If you don't have LOS your bolt simply goes into whatever barrier
	and you lose your mana. If there is LOS the bolt will lock onto your target and the damage is applied when it hits the target.
	-If your target moves the bolt moves with it in any direction or angle (consistent with other projectiles).
	-The way this is written once a bolt is cast a the distance from the initial cast to the target repeatedly
	check and if target is moving recalculates at what predicted time the bolt should hit that target in client_process
	When bolt hits its predicted point the damage is then done to target.
	Note: Projectile speed of 1 takes 3 seconds to go 100 distance units. Calculations are based on this constant.
	Live Bolt speed: Projectile speed of X takes 5 seconds to go 300 distance units.
	Pending Implementation: What this code can not do is prevent damage if the bolt hits a barrier after passing the initial LOS check
	because the target has moved while the bolt is in motion. (it is rare to actual get this to occur on live in normal game play)
	*/

	if (!spell_target)
		return false;

	uint8 anim = spells[spell_id].casting_animation;
	int slot = -1;

	//Make sure there is an avialable bolt to be cast.
	for (int i = 0; i < MAX_SPELL_PROJECTILE; i++) {
		if (ProjectileAtk[i].target_id == 0) {
			slot = i;
			break;
		}
	}

	if (slot < 0)
		return false;

	float arc = 0.0f;
	float distance_mod = 0.0f;

	if (CheckLosFN(spell_target)) {

		float distance = spell_target->CalculateDistance(GetX(), GetY(), GetZ());

		/*
		New Distance Mod constant (7/25/21 update), modifier is needed to adjust slower speeds to have correct impact times at short distances.
		We use archery 4.0 speed as a baseline for the forumla.  At speed 1.5 at 50 pct distance mod is needed, where as speed 4.0 there is no modifer.
		Therefore, we derive out our modifer as follows. distance_mod = (speed - 4) * ((50 - 0)/(1.5-4)). The ratio there is -20.0f. distance_mod = (speed - 4) * -20.0f
		For distances >125 we use different modifier, this was all meticulously tested by eye to get the best possible outcome for projectile impact times. Not perfect though.
		*/

		if (distance <= 125.0f) {
			distance_mod = (speed - 4.0f) * -20.0f;
			distance += distance * distance_mod / 100.0f;
		}
		else if (distance > 125.0f && distance <= 200.0f)
			distance = 3.14f * (distance / 2.0f); //Get distance of arc to better reflect projectile path length

		else if (distance > 200.0f) {
			arc = 50.0f - ((distance - 200.0f) * 0.266f); //Arc angle gets drastically larger if >200 distance, lets lower it down gradually for better effect.
			arc = std::max(arc, 20.0f); //No lower than 20 arc
			distance = distance * 1.30f; //Add 30% to base distance if over 200 range to tighten up hit timing.
			distance = 3.14f * (distance / 2.0f); //Get distance of arc to better reflect projectile path length
		}

		float hit = 1200.0f + (10 * distance / speed);

		ProjectileAtk[slot].increment = 1;
		ProjectileAtk[slot].hit_increment = static_cast<uint16>(hit); //This projected hit time if target does NOT MOVE
		ProjectileAtk[slot].target_id = spell_target->GetID();
		ProjectileAtk[slot].wpn_dmg = spell_id; //Store spell_id in weapon damage field
		ProjectileAtk[slot].origin_x = GetX();
		ProjectileAtk[slot].origin_y = GetY();
		ProjectileAtk[slot].origin_z = GetZ();
		ProjectileAtk[slot].skill = EQ::skills::SkillConjuration;
		ProjectileAtk[slot].speed_mod = speed;

		SetProjectileAttack(true);
	}

	//This will use the correct graphic as defined in the player_1 field of spells_new table. Found in UF+ spell files.
	if (RuleB(Spells, UseLiveSpellProjectileGFX)) {
		ProjectileAnimation(spell_target, 0, false, speed, 0.0f, 0.0f, arc, spells[spell_id].player_1);
	}
	//This allows limited support for server using older spell files that do not contain data for bolt graphics.
	else {
		//Only use fire graphic for fire spells.
		if (spells[spell_id].resist_type == RESIST_FIRE) {

			if (IsClient()) {
				if (CastToClient()->ClientVersionBit() <= 4) //Titanium needs alternate graphic.
					ProjectileAnimation(spell_target, (RuleI(Spells, FRProjectileItem_Titanium)), false, speed, 0.0f, 0.0f, arc);
				else
					ProjectileAnimation(spell_target, (RuleI(Spells, FRProjectileItem_SOF)), false, speed, 0.0f, 0.0f, arc);
			}

			else
				ProjectileAnimation(spell_target, (RuleI(Spells, FRProjectileItem_NPC)), false, speed, 0.0f, 0.0f, arc);
		}
		//Default to an arrow if not using a mage bolt (Use up to date spell file and enable above rules for best results)
		else
			ProjectileAnimation(spell_target, 0, 1, speed, 0.0f, 0.0f, arc);
	}

	if (spells[spell_id].casting_animation == 64)
		anim = 44; //Corrects for animation error.

	DoAnim(anim, 0, true, IsClient() ? FilterPCSpells : FilterNPCSpells); //Override the default projectile animation.
	return true;
}

void Mob::ResourceTap(int64 damage, uint16 spellid)
{
	//'this' = caster
	if (!IsValidSpell(spellid))
		return;

	for (int i = 0; i < EFFECT_COUNT; i++) {
		if (spells[spellid].effect_id[i] == SpellEffect::ResourceTap) {
			damage = (damage * spells[spellid].base_value[i]) / 1000;

			if (damage) {
				if (spells[spellid].max_value[i] && (damage > spells[spellid].max_value[i]))
					damage = spells[spellid].max_value[i];

				if (spells[spellid].limit_value[i] == 0) { // HP Tap
					if (damage > 0)
						HealDamage(damage);
					else
						Damage(this, -damage, 0, EQ::skills::SkillEvocation, false);
				}

				if (spells[spellid].limit_value[i] == 1) // Mana Tap
					SetMana(GetMana() + damage);

				if (spells[spellid].limit_value[i] == 2 && IsClient()) // Endurance Tap
					CastToClient()->SetEndurance(CastToClient()->GetEndurance() + damage);

			}
		}
	}
}

void Mob::TryTriggerThreshHold(int64 damage, int effect_id,  Mob* attacker){

	if (damage <= 0)
		return;

	if ((SpellEffect::TriggerMeleeThreshold == effect_id) && !spellbonuses.TriggerMeleeThreshold )
		return;
	else if ((SpellEffect::TriggerSpellThreshold == effect_id) && !spellbonuses.TriggerSpellThreshold)
		return;

	int buff_count = GetMaxTotalSlots();

	for(int slot = 0; slot < buff_count; slot++) {

		if(IsValidSpell(buffs[slot].spellid)){

			for(int i = 0; i < EFFECT_COUNT; i++){

				if (spells[buffs[slot].spellid].effect_id[i] == effect_id){

					uint16 spell_id = spells[buffs[slot].spellid].base_value[i];

					if (damage > spells[buffs[slot].spellid].limit_value[i]){

						BuffFadeBySlot(slot);

						if (IsValidSpell(spell_id)) {

							if (IsBeneficialSpell(spell_id))
								SpellFinished(spell_id, this, EQ::spells::CastingSlot::Item, 0, -1, spells[spell_id].resist_difficulty);

							else if(attacker)
								SpellFinished(spell_id, attacker, EQ::spells::CastingSlot::Item, 0, -1, spells[spell_id].resist_difficulty);
						}
					}
				}
			}
		}
	}
}

void Mob::CastSpellOnLand(Mob* caster, int32 spell_id)
{
	/*
	This function checks for incoming spells on a mob, if they meet the criteria for focus SpellEffect::Fc_Cast_Spell_on_Land then
	a new spell will be cast by THIS mob as specified by the focus effect. Note: Chance to cast the spell is determined in
	the CalcFocusEffect function if not 100pct.
	ApplyFocusProcLimiter() function checks for SpellEffect::Proc_Timer_Modifier which allows for limiting how often a spell from effect can be triggered
	for example, if set to base=1 and base2= 1500, then for everyone 1 successful trigger, you will be unable to trigger again for 1.5 seconds.

	Live only has this focus in buffs/debuffs that can be placed on a target. TODO: Will consider adding support for it as AA and Item.
	*/
	if (!caster)
		return;

	int32 trigger_spell_id = 0;

	//Step 1: Check this focus effect exists on the mob.
	if (spellbonuses.FocusEffects[focusFcCastSpellOnLand]) {

		int buff_count = GetMaxTotalSlots();
		for (int i = 0; i < buff_count; i++) {

			if ((IsValidSpell(buffs[i].spellid) && (buffs[i].spellid != spell_id) && IsEffectInSpell(buffs[i].spellid, SpellEffect::Fc_Cast_Spell_On_Land))) {

				//Step 2: Check if we pass all focus limiters and focus chance roll
				trigger_spell_id = CalcFocusEffect(focusFcCastSpellOnLand, buffs[i].spellid, spell_id, false, buffs[i].casterid, caster);

				if (IsValidSpell(trigger_spell_id) && (trigger_spell_id != spell_id)) {

					//Step 3: Cast spells
					if (IsBeneficialSpell(trigger_spell_id)) {
						SpellFinished(trigger_spell_id, this, EQ::spells::CastingSlot::Item, 0, -1, spells[trigger_spell_id].resist_difficulty);
					}
					else {
						Mob* current_target = GetTarget();
						//For now don't let players cast detrimental effects on themselves if they are targeting themselves. Need to confirm behavior.
						if (current_target && current_target->GetID() != GetID())
							SpellFinished(trigger_spell_id, current_target, EQ::spells::CastingSlot::Item, 0, -1, spells[trigger_spell_id].resist_difficulty);
					}
				}
				if (i >= 0)
					CheckNumHitsRemaining(NumHit::MatchingSpells, i);
			}
		}
	}
}

void Mob::CalcSpellPowerDistanceMod(uint16 spell_id, float range, Mob* caster)
{
	if (IsDistanceModifierSpell(spell_id)){

		float distance = 0;

		if (caster && !range)
			distance = caster->CalculateDistance(GetX(), GetY(), GetZ());
		else
			distance = sqrt(range);

		distance = EQ::Clamp(distance, spells[spell_id].min_distance, spells[spell_id].max_distance);

		float dm_range = spells[spell_id].max_distance - spells[spell_id].min_distance;
		float dm_mod_interval = spells[spell_id].max_distance_mod - spells[spell_id].min_distance_mod;
		float dist_from_min = distance - spells[spell_id].min_distance;
		float mod = spells[spell_id].min_distance_mod + (dist_from_min * (dm_mod_interval/dm_range));
		mod *= 100.0f;

		SetSpellPowerDistanceMod(static_cast<int>(mod));
	}
}

void Mob::BreakInvisibleSpells()
{
	if(invisible) {
		nobuff_invisible = 0;
		BuffFadeByEffect(SpellEffect::Invisibility);
		BuffFadeByEffect(SpellEffect::Invisibility2);
	}
	if(invisible_undead) {
		ZeroInvisibleVars(InvisType::T_INVISIBLE_VERSE_UNDEAD);
		BuffFadeByEffect(SpellEffect::InvisVsUndead);
		BuffFadeByEffect(SpellEffect::InvisVsUndead2);
	}
	if(invisible_animals){
		ZeroInvisibleVars(InvisType::T_INVISIBLE_VERSE_ANIMAL);
		BuffFadeByEffect(SpellEffect::ImprovedInvisAnimals);
		BuffFadeByEffect(SpellEffect::InvisVsAnimals);
	}
}

void Client::BreakSneakWhenCastOn(Mob *caster, bool IsResisted)
{
	bool IsCastersTarget = false; // Chance to avoid only applies to AOE spells when not targeted.
	if (hidden || improved_hidden) {
		if (caster) {
			Mob *spell_target = caster->GetTarget();
			if (spell_target && spell_target == this) {
				IsCastersTarget = true;
			}
		}

		if (!IsCastersTarget) {
			int chance = spellbonuses.NoBreakAESneak + itembonuses.NoBreakAESneak + aabonuses.NoBreakAESneak;
			if (IsResisted) {
				chance *= RuleR(Spells, BreakSneakWhenCastOn);
			}

			if (chance && zone->random.Roll(chance)) {
				return; // Do not drop Sneak/Hide
			}
		}
		CancelSneakHide();
	}
}

void Client::BreakFeignDeathWhenCastOn(bool IsResisted)
{
	if (GetFeigned()) {
		int chance = spellbonuses.FeignedCastOnChance + itembonuses.FeignedCastOnChance + aabonuses.FeignedCastOnChance;

		if (IsResisted) {
			chance *= RuleR(Spells, BreakFeignDeathWhenCastOn);
		}

		if(chance && (zone->random.Roll(chance))){
			MessageString(Chat::SpellFailure,FD_CAST_ON_NO_BREAK);
			return;
		}

		SetFeigned(false);
		MessageString(Chat::SpellFailure,FD_CAST_ON);
	}
}

bool Mob::HarmonySpellLevelCheck(int32 spell_id, Mob *target)
{
	//'this' = caster of spell
	if (!target) {
		return false;
	}

	for (int i = 0; i < EFFECT_COUNT; i++) {
		// not important to check limit on SpellEffect::Lull as it doesnt have one and if the other components won't land, then SpellEffect::Lull wont either
		if (spells[spell_id].effect_id[i] == SpellEffect::ChangeFrenzyRad || spells[spell_id].effect_id[i] == SpellEffect::Harmony) {
			if ((spells[spell_id].max_value[i] != 0 && target->GetLevel() > spells[spell_id].max_value[i]) || target->GetSpecialAbility(SpecialAbility::PacifyImmunity)) {
				return false;
			}
		}
	}
	return true;
}

bool Mob::PassCharmTargetRestriction(Mob *target) {

	//Level restriction check should not go here.
	if (!target) {
		return false;
	}

	if (target->IsClient() && IsClient()) {
		MessageString(Chat::Red, CANNOT_AFFECT_PC);
		LogSpells("Spell casting canceled: Can not cast charm on a client.");
		return false;
	}
	else if (target->IsCorpse()) {
		LogSpells("Spell casting canceled: Can not cast charm on a corpse.");
		return false;
	}
	else if (GetPet() && IsClient()) {
		MessageString(Chat::Red, ONLY_ONE_PET);
		LogSpells("Spell casting canceled: Can not cast charm if you have a pet.");
		return false;
	}
	else if (target->GetOwner()) {
		MessageString(Chat::Red, CANNOT_CHARM);
		LogSpells("Spell casting canceled: Can not cast charm on a pet.");
		return false;
	}
	return true;
}

bool Mob::PassLimitToSkill(EQ::skills::SkillType skill, int32 spell_id, int proc_type, int aa_id)
{
	/*
		Check if SpellEffect::AddMeleProc or SpellEffect::RangedProc have a skill limiter. Passes automatically if no skill limiters present.
	*/
	int32 proc_type_spaid = 0;
	if (proc_type == ProcType::MELEE_PROC) {
		proc_type_spaid = SpellEffect::AddMeleeProc;
	}
	if (proc_type == ProcType::RANGED_PROC) {
		proc_type_spaid = SpellEffect::RangedProc;
	}

	bool match_proc_type = false;
	bool has_limit_check = false;

	if (!aa_id && spellbonuses.LimitToSkill[EQ::skills::HIGHEST_SKILL + 2]) {

		if (!IsValidSpell(spell_id)) {
			return false;
		}

		for (int i = 0; i < EFFECT_COUNT; i++) {
			if (spells[spell_id].effect_id[i] == proc_type_spaid) {
				match_proc_type = true;
			}
			if (match_proc_type && spells[spell_id].effect_id[i] == SpellEffect::LimitToSkill && spells[spell_id].base_value[i] <= EQ::skills::HIGHEST_SKILL) {

				has_limit_check = true;
				if (spells[spell_id].base_value[i] == skill) {
					return true;
				}
			}
		}
	}
	else if (aabonuses.LimitToSkill[EQ::skills::HIGHEST_SKILL + 2]) {

		int rank_id = 1;
		AA::Rank *rank = zone->GetAlternateAdvancementRank(aa_id);

		if (!rank) {
			return true;
		}

		AA::Ability *ability_in = rank->base_ability;
		if (!ability_in) {
			return true;
		}

		for (auto &aa : aa_ranks) {
			auto ability_rank = zone->GetAlternateAdvancementAbilityAndRank(aa.first, aa.second.first);
			auto ability = ability_rank.first;
			auto rank = ability_rank.second;

			if (!ability) {
				continue;
			}

			for (auto &effect : rank->effects) {
				if (effect.effect_id == proc_type_spaid) {
					match_proc_type = true;
				}

				if (match_proc_type && effect.effect_id == SpellEffect::LimitToSkill && effect.base_value <= EQ::skills::HIGHEST_SKILL) {
					has_limit_check = true;
					if (effect.base_value == skill) {
						return true;
					}
				}
			}
		}
	}

	if (has_limit_check) {
		return false; //Limit was found, but not matched, fail.
	}
	else {
		return true; //No limit is present, automatically pass.
	}
}

bool Mob::CanFocusUseRandomEffectivenessByType(focusType type)
{
	switch (type) {
	case focusImprovedDamage:
	case focusImprovedDamage2:
	case focusImprovedHeal:
	case focusManaCost:
	case focusResistRate:
	case focusFcDamagePctCrit:
	case focusReagentCost:
	case focusSpellHateMod:
	case focusSpellVulnerability:
	case focusFcSpellDamagePctIncomingPC:
	case focusFcHealPctIncoming:
	case focusFcHealPctCritIncoming:
		return true;
	}

	return false;
}

int Mob::GetFocusRandomEffectivenessValue(int focus_base, int focus_base2, bool best_focus)
{
	int64 value = 0;
	// This is used to determine which focus should be used for the random calculation
	if (best_focus) {
		// If the spell does not contain a base2 value, then its a straight non random
		value = focus_base;
		// If the spell contains a value in the base2 field then that is the max value
		if (focus_base2 != 0) {
			value = focus_base2;
		}
		return value;
	}
	else if (focus_base2 == 0 || focus_base == focus_base2) { // Actual focus calculation starts here
		return focus_base;
	}

	return zone->random.Int(focus_base, focus_base2);
}

bool Mob::NegateSpellEffect(uint16 spell_id, int effect_id)
{
	/*
		This works for most effects, anything handled purely by the client will bypass this (ie Gate, Shadowstep)
		Seen with resurrection effects, likely blocks the client from accepting a ressurection request. *Not implement at this time.
	*/

	for (int i = 0; i < GetMaxTotalSlots(); i++) {
		//Check for any buffs containing NegateEffect
		if (IsValidSpell(buffs[i].spellid) && IsEffectInSpell(buffs[i].spellid, SpellEffect::NegateSpellEffect) && spell_id != buffs[i].spellid) {
			//Match each of the negate effects with the current spell effect, if found, that effect will not be applied.
			for (int j = 0; j < EFFECT_COUNT; j++)
			{
				if (spells[buffs[i].spellid].effect_id[j] == SpellEffect::NegateSpellEffect &&
					spells[buffs[i].spellid].limit_value[j] == effect_id &&
					(spells[buffs[i].spellid].base_value[j] == NEGATE_SPA_ALL_BONUSES ||
					 spells[buffs[i].spellid].base_value[j] == NEGATE_SPA_SPELLBONUS ||
					 spells[buffs[i].spellid].base_value[j] == NEGATE_SPA_SPELLBONUS_AND_ITEMBONUS ||
					 spells[buffs[i].spellid].base_value[j] == NEGATE_SPA_SPELLBONUS_AND_AABONUS)) {
						return true;
				}
			}
		}
	}
	return false;
}

int Mob::GetMemoryBlurChance(int base_chance)
{
	/*
		Memory Blur mechanic for SPA 62
		Chance formula is effect chance + charisma modifer + caster level modifier
		Effect chance is base value of spell
		Charisma modifier is CHA/10 = %, with MAX of 15% (thus 150 cha gives you max bonus)
		Caster level modifier. +100% if caster < level 17 which scales down to 25% at > 53. **
		(Yes the above gets worse as you level. Behavior was confirmed on live.)
		Memory blur is applied to mez on initial cast using same formula. However, recasting on a target that
		is already mezed will not give a chance to memory blur. The blur is not checked on buff ticks.

		SPA 242 SpellEffect::IncreaseChanceMemwipe modifies the final chance after all bonuses are applied.
		This is also applied to memory blur from mez spells.

		this = caster
	*/
	int cha_mod = int(GetCHA() / 10);
	cha_mod = std::min(cha_mod, 15);

	int lvl_mod = 0;
	if (GetLevel() < 17) {
		lvl_mod = 100;
	}
	else if (GetLevel() > 53) {
		lvl_mod = 25;
	}
	else {
		lvl_mod = 100 + ((GetLevel() - 16)*-2);//Derived from above range of values.**
	}

	int chance = cha_mod + lvl_mod + base_chance;

	int chance_mod = spellbonuses.IncreaseChanceMemwipe + itembonuses.IncreaseChanceMemwipe + aabonuses.IncreaseChanceMemwipe;

	chance += chance * chance_mod / 100;
	return chance;
}

void Mob::VirusEffectProcess()
{
	/*
		Virus Mechanics
		To qualify as a virus effect buff, all of the following spell table need to be set. (At some point will correct names)
		viral_targets = MIN_SPREAD_TIME
		viral_timer   = MAX_SPREAD_TIME
		viral_range   = SPREAD_RADIUS
		Once a buff with a viral effect is applied, a 1000 ms timer will begin.
		The time at which the virus will attempt to spread is determined by a random value between MIN_SPREAD_TIME and MAX_SPREAD_TIME
		Each time the virus attempts to spread the next time interval will be chosen at random again.
		If a spreader finds a target for viral buff, when the viral buff spreads the duration on the new target will be the time remaining on the spreaders buff.
		Spreaders DOES NOT need LOS to spread. There is no max amount of targets the virus can spread to.
		When the spreader no longer has any viral buffs the timer stops.
		The current code supports spreading for both detrimental and beneficial spells.
	*/

	// Only spread in zones without perm buffs
	if (zone->BuffTimersSuspended()) {
		viral_timer.Disable();
		return;
	}

	bool stop_timer = true;
	for (int buffs_i = 0; buffs_i < GetMaxTotalSlots(); ++buffs_i)
	{
		if (IsValidSpell(buffs[buffs_i].spellid) && IsVirusSpell(buffs[buffs_i].spellid))
		{
			if (buffs[buffs_i].virus_spread_time > 0) {
				buffs[buffs_i].virus_spread_time -= 1;
				stop_timer = false;
			}

			if (buffs[buffs_i].virus_spread_time <= 0) {
				buffs[buffs_i].virus_spread_time = zone->random.Int(GetSpellViralMinimumSpreadTime(buffs[buffs_i].spellid), GetSpellViralMaximumSpreadTime(buffs[buffs_i].spellid));
				SpreadVirusEffect(buffs[buffs_i].spellid, buffs[buffs_i].casterid, buffs[buffs_i].ticsremaining);
				stop_timer = false;
			}
		}
	}

	if (stop_timer) {
		viral_timer.Disable();
	}
}

void Mob::SpreadVirusEffect(int32 spell_id, uint32 caster_id, int32 buff_tics_remaining)
{
	Mob *caster = entity_list.GetMob(caster_id);

	if (!caster) {
		return;
	}

	std::vector<Mob *> targets_in_range = entity_list.GetTargetsForVirusEffect(
		this,
		caster,
		GetSpellViralSpreadRange(spell_id),
		spells[spell_id].pcnpc_only_flag,
		spell_id
	);

	for (auto &mob: targets_in_range) {
		if (!mob) {
			continue;
		}

		if (!mob->FindBuff(spell_id)) {
			if (caster) {
				if (buff_tics_remaining) {
					//When virus is spread, the buff on new target is applied with the amount of time remaining on the spreaders buff.
					caster->SpellOnTarget(spell_id, mob, 0, false, 0, false, -1, buff_tics_remaining);
				}
			}
		}
	}
}

bool Mob::IsFocusProcLimitTimerActive(int32 focus_spell_id) {
	/*
		Used with SPA 511 SpellEffect::Ff_FocusTimerMin to limit how often a focus effect can be applied.
		Ie. Can only have a spell trigger once every 15 seconds, or to be more creative can only
		have the fire spells received a very high special focused once every 30 seconds.
		Note, this stores timers for both spell, item and AA related focuses For AA the focus_spell_id
		is saved as the the negative value of the rank.id (to avoid conflicting with spell_ids)
	*/
	for (int i = 0; i < MAX_FOCUS_PROC_LIMIT_TIMERS; i++) {
		if (focusproclimit_spellid[i] == focus_spell_id) {
			if (focusproclimit_timer[i].Enabled()) {
				if (focusproclimit_timer[i].GetRemainingTime() > 0) {
					return true;
				}
				else {
					focusproclimit_timer[i].Disable();
					focusproclimit_spellid[i] = 0;
				}
			}
		}
	}
	return false;
}

void Mob::SetFocusProcLimitTimer(int32 focus_spell_id, uint32 focus_reuse_time) {

	bool is_set = false;

	for (int i = 0; i < MAX_FOCUS_PROC_LIMIT_TIMERS; i++) {
		if (!focusproclimit_spellid[i] && !is_set) {
			focusproclimit_spellid[i] = focus_spell_id;
			focusproclimit_timer[i].SetTimer(focus_reuse_time);
			is_set = true;
		}
		//Remove old temporary focus if was from a buff you no longer have.
		else if (focusproclimit_spellid[i] > 0 && !FindBuff(focus_spell_id)) {
			focusproclimit_spellid[i] = 0;
			focusproclimit_timer[i].Disable();
		}
	}
}

bool Mob::IsProcLimitTimerActive(int32 base_spell_id, uint32 proc_reuse_time, int proc_type) {
	/*
		Used with SPA 512 SpellEffect::Proc_Timer_Modifier to limit how often a proc can be cast.
		If this effect exists it will prevent the next proc from firing until the timer
		defined in SPA 512 is finished. Ie. 1 proc every 55 seconds.
		Spell, Ranged, and Defensive procs all have their own timer array, therefore
		you can stack multiple different types of effects in the same spell. Make sure
		SPA 512 goes directly after each proc you want to have the timer.
	*/
	if (!proc_reuse_time) {
		return false;
	}

	for (int i = 0; i < MAX_PROC_LIMIT_TIMERS; i++) {

		if (proc_type == ProcType::MELEE_PROC) {
			if (spell_proclimit_spellid[i] == base_spell_id) {
				if (spell_proclimit_timer[i].Enabled()) {
					if (spell_proclimit_timer[i].GetRemainingTime() > 0) {
						return true;
					}
					else {
						spell_proclimit_timer[i].Disable();
						spell_proclimit_spellid[i] = 0;
					}
				}
			}
		}
		else if (proc_type == ProcType::RANGED_PROC) {
			if (ranged_proclimit_spellid[i] == base_spell_id) {
				if (ranged_proclimit_timer[i].Enabled()) {
					if (ranged_proclimit_timer[i].GetRemainingTime() > 0) {
						return true;
					}
					else {
						ranged_proclimit_timer[i].Disable();
						ranged_proclimit_spellid[i] = 0;
					}
				}
			}
		}
		else if (proc_type == ProcType::DEFENSIVE_PROC) {
			if (def_proclimit_spellid[i] == base_spell_id) {
				if (def_proclimit_timer[i].Enabled()) {
					if (def_proclimit_timer[i].GetRemainingTime() > 0) {
						return true;
					}
					else {
						def_proclimit_timer[i].Disable();
						def_proclimit_spellid[i] = 0;
					}
				}
			}
		}
	}
	return false;
}

void Mob::SetProcLimitTimer(int32 base_spell_id, uint32 proc_reuse_time, int proc_type) {

	if (!proc_reuse_time) {
		return;
	}

	bool is_set = false;

	for (int i = 0; i < MAX_PROC_LIMIT_TIMERS; i++) {

		if (proc_type == ProcType::MELEE_PROC) {
			if (!spell_proclimit_spellid[i] && !is_set) {
				spell_proclimit_spellid[i] = base_spell_id;
				spell_proclimit_timer[i].SetTimer(proc_reuse_time);
				is_set = true;
			}
			else if (spell_proclimit_spellid[i] > 0 && !FindBuff(base_spell_id)) {
				spell_proclimit_spellid[i] = 0;
				spell_proclimit_timer[i].Disable();
			}
		}

		if (proc_type == ProcType::RANGED_PROC) {
			if (!ranged_proclimit_spellid[i] && !is_set) {
				ranged_proclimit_spellid[i] = base_spell_id;
				ranged_proclimit_timer[i].SetTimer(proc_reuse_time);
				is_set = true;
			}
			else if (ranged_proclimit_spellid[i] > 0 && !FindBuff(base_spell_id)) {
				ranged_proclimit_spellid[i] = 0;
				ranged_proclimit_timer[i].Disable();
			}
		}

		if (proc_type == ProcType::DEFENSIVE_PROC) {
			if (!def_proclimit_spellid[i] && !is_set) {
				def_proclimit_spellid[i] = base_spell_id;
				def_proclimit_timer[i].SetTimer(proc_reuse_time);
				is_set = true;
			}
			else if (def_proclimit_spellid[i] > 0 && !FindBuff(base_spell_id)) {
				def_proclimit_spellid[i] = 0;
				def_proclimit_timer[i].Disable();
			}
		}
	}
}

void Mob::SendIllusionWearChange(Client* c) {

	/*
		We send this to client on Client::CompleteConnect() to properly update textures of
		other mobs in zone with illusions on them.
	*/
	if (!c) {
		return;
	}

	if (!spellbonuses.Illusion && !itembonuses.Illusion && !aabonuses.Illusion) {
		return;
	}

	for (int x = EQ::textures::textureBegin; x <= EQ::textures::LastTintableTexture; x++) {
		SendWearChange(x, c);
	}
}

void Mob::ApplyIllusionToCorpse(int32 spell_id, Corpse* new_corpse) {

	//Transfers most illusions over to the corpse upon death
	if (!IsValidSpell(spell_id)) {
		return;
	}

	if (!new_corpse) {
		return;
	}

	for (int i = 0; i < EFFECT_COUNT; i++){
		if (spells[spell_id].effect_id[i] == SpellEffect::Illusion) {
			new_corpse->ApplySpellEffectIllusion(spell_id, nullptr, -1, spells[spell_id].base_value[i], spells[spell_id].limit_value[i], spells[spell_id].max_value[i]);
			return;
		}
	}
}

void Mob::ApplySpellEffectIllusion(int32 spell_id, Mob *caster, int buffslot, int base, int limit, int max)
{
	// Gender Illusions
	if (base == -1) {
		// Specific Gender Illusions
		if (spell_id == SPELL_ILLUSION_MALE || spell_id == SPELL_ILLUSION_FEMALE) {
			uint8 specific_gender = spell_id == SPELL_ILLUSION_MALE ? Gender::Male : Gender::Female;

			if (caster && caster->GetTarget()) {
				SendIllusionPacket(
					AppearanceStruct{
						.gender_id = specific_gender,
						.race_id = caster->GetTarget()->GetBaseRace(),
						.texture = caster->GetTarget()->GetTexture(),
					}
				);
			}
		}
			// Change Gender Illusions
		else {
			if (caster && caster->GetTarget()) {
				uint8 opposite_gender = caster->GetTarget()->GetGender() == Gender::Male ? Gender::Female : Gender::Male;

				SendIllusionPacket(
					AppearanceStruct{
						.gender_id = opposite_gender,
						.race_id = caster->GetTarget()->GetRace(),
						.texture = caster->GetTarget()->GetTexture(),
					}
				);
			}
		}
	}
	// Racial Illusions
	else {
		auto gender_id = (
			max > 0 &&
			(
				max != 3 ||
				limit == 0
				) ?
				(max - 1) :
			Mob::GetDefaultGender(base, GetGender())
		);

		auto race_size = GetRaceGenderDefaultHeight(
			base,
			gender_id
		);

		if (base != Race::Elemental && base != Race::Drakkin) {
			if (max > 0) {
				if (limit == 0) {
					SendIllusionPacket(
						AppearanceStruct{
							.gender_id = static_cast<uint8>(gender_id),
							.race_id = static_cast<uint16>(base),
						}
					);
				} else {
					if (max != 3) {
						SendIllusionPacket(
							AppearanceStruct{
								.gender_id = static_cast<uint8>(gender_id),
								.helmet_texture = static_cast<uint8>(max),
								.race_id = static_cast<uint16>(base),
								.texture = static_cast<uint8>(limit),
							}
						);
					} else {
						SendIllusionPacket(
							AppearanceStruct{
								.gender_id = static_cast<uint8>(gender_id),
								.helmet_texture = static_cast<uint8>(limit),
								.race_id = static_cast<uint16>(base),
								.texture = static_cast<uint8>(limit),
							}
						);
					}
				}
			} else {
				SendIllusionPacket(
					AppearanceStruct{
						.gender_id = static_cast<uint8>(gender_id),
						.helmet_texture = static_cast<uint8>(max),
						.race_id = static_cast<uint16>(base),
						.texture = static_cast<uint8>(limit),
					}
				);
			}
		} else if (base == Race::Elemental){
			SendIllusionPacket(
				AppearanceStruct{
					.gender_id = static_cast<uint8>(gender_id),
					.race_id = static_cast<uint16>(base),
					.texture = static_cast<uint8>(limit),
				}
			);
		} else if (base == Race::Drakkin) {
			FaceChange_Struct f{
				.haircolor = GetHairColor(),
				.beardcolor = GetBeardColor(),
				.eyecolor1 = GetEyeColor1(),
				.eyecolor2 = GetEyeColor2(),
				.hairstyle = GetHairStyle(),
				.beard = GetBeard(),
				.face = GetLuclinFace(),
				.drakkin_heritage = static_cast<uint32>(limit),
				.drakkin_tattoo = GetDrakkinTattoo(),
				.drakkin_details = GetDrakkinDetails(),
			};

			SendIllusionPacket(
				AppearanceStruct{
					.gender_id = static_cast<uint8>(gender_id),
					.race_id = static_cast<uint16>(base),
				}
			);

			SetFaceAppearance(f);
		}

		SendAppearancePacket(AppearanceType::Size, race_size);
	}

	for (int x = EQ::textures::textureBegin; x <= EQ::textures::LastTintableTexture; x++) {
		SendWearChange(x);
	}

	if (buffslot != -1) {
		if (
			caster == this &&
			spell_id != SPELL_MINOR_ILLUSION &&
			spell_id != SPELL_ILLUSION_TREE &&
			(
				spellbonuses.IllusionPersistence ||
				aabonuses.IllusionPersistence ||
				itembonuses.IllusionPersistence ||
				RuleB(Spells, IllusionsAlwaysPersist)
			)
		) {
			buffs[buffslot].persistant_buff = 1;
		}
		else {
			buffs[buffslot].persistant_buff = 0;
		}
	}
}

bool Mob::HasPersistDeathIllusion(int32 spell_id) {
	if (
		spellbonuses.IllusionPersistence > 1 ||
		aabonuses.IllusionPersistence > 1  ||
		itembonuses.IllusionPersistence > 1 ||
		RuleB(Spells, IllusionsAlwaysPersist)
	) {
		if (
			spell_id != SPELL_MINOR_ILLUSION &&
			spell_id != SPELL_ILLUSION_TREE &&
			IsEffectInSpell(spell_id, SpellEffect::Illusion) &&
			IsBeneficialSpell(spell_id)
		) {
			return true;
		}
	}
	return false;
}

void Mob::SetBuffDuration(int spell_id, int duration, int level) {

	/*
		Will refresh the buff with specified spell_id to the specified duration
		If spell is -1, then all spells will be set to the specified duration
		If duration 0, then will set duration to buffs normal max duration.
	*/

	bool adjust_all_buffs = false;

	if (spell_id == -1) {
		adjust_all_buffs = true;
	}

	if (!adjust_all_buffs && !IsValidSpell(spell_id)){
		return;
	}

	if (duration < -1) {
		duration = PERMANENT_BUFF_DURATION;
	}

	int buff_count = GetMaxTotalSlots();
	for (int slot = 0; slot < buff_count; slot++) {
		if (!adjust_all_buffs) {
			if (IsValidSpell(buffs[slot].spellid) && buffs[slot].spellid == spell_id) {
				SpellOnTarget(buffs[slot].spellid, this, 0, false, 0, false, level, duration, true);
				return;
			}
		} else {
			if (IsValidSpell(buffs[slot].spellid)) {
				SpellOnTarget(buffs[slot].spellid, this, 0, false, 0, false, level, duration, true);
			}
		}
	}
}

void Mob::ApplySpellBuff(int spell_id, int duration, int level)
{
	/*
		Used for quest command to apply a new buff with custom duration.
		Duration set to 0 will apply with normal duration.
	*/
	if (!IsValidSpell(spell_id)) {
		return;
	}

	if (!spells[spell_id].buff_duration) {
		return;
	}

	if (duration <= -1) {
		duration = PERMANENT_BUFF_DURATION;
	}

	SpellOnTarget(spell_id, this, 0, false, 0, false, level, duration);
}

int Mob::GetBuffStatValueBySpell(int32 spell_id, const char* stat_identifier)
{
	if (!IsValidSpell(spell_id)) {
		return 0;
	}

	if (!stat_identifier) {
		return 0;
	}

	std::string id = Strings::ToLower(stat_identifier);

	int buff_count = GetMaxTotalSlots();
	for (int slot = 0; slot < buff_count; slot++) {
		if (IsValidSpell(buffs[slot].spellid) && buffs[slot].spellid == spell_id) {
			return GetBuffStatValueBySlot(slot, stat_identifier);
		}
	}
	return 0;
}

int Mob::GetBuffStatValueBySlot(uint8 slot, const char* stat_identifier)
{
	if (slot > GetMaxTotalSlots()) {
		return 0;
	}

	if (!stat_identifier) {
		return 0;
	}

	std::string id = Strings::ToLower(stat_identifier);

	if (id == "caster_level") { return buffs[slot].casterlevel; }
	else if (id == "spell_id") { return buffs[slot].spellid; }
	else if (id == "caster_id") { return buffs[slot].spellid; }
	else if (id == "ticsremaining") { return buffs[slot].ticsremaining; }
	else if (id == "counters") { return buffs[slot].counters; }
	else if (id == "hit_number") { return  buffs[slot].hit_number; }
	else if (id == "melee_rune") { return  buffs[slot].melee_rune; }
	else if (id == "magic_rune") { return  buffs[slot].magic_rune; }
	else if (id == "dot_rune") { return  buffs[slot].dot_rune; }
	else if (id == "caston_x") { return  buffs[slot].caston_x; }
	else if (id == "caston_y") { return buffs[slot].caston_y; }
	else if (id == "caston_z") { return  buffs[slot].caston_z; }
	else if (id == "instrument_mod") { return  buffs[slot].instrument_mod; }
	else if (id == "persistant_buff") { return  buffs[slot].persistant_buff; }
	else if (id == "client") { return  buffs[slot].client; }
	else if (id == "extra_di_chance") { return  buffs[slot].ExtraDIChance; }
	else if (id == "root_break_chance") { return  buffs[slot].RootBreakChance; }
	else if (id == "virus_spread_time") { return  buffs[slot].virus_spread_time; }
	return 0;
}
