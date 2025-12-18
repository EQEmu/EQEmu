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

#include "mob.h"

#include "common/eqemu_logsys.h"
#include "common/misc_functions.h"
#include "common/spdat.h"
#include "zone/client.h"
#include "zone/entity.h"
#include "zone/position.h"
#include "zone/string_ids.h"
#include "zone/worldserver.h"
#include "zone/zonedb.h"

float Mob::GetActSpellRange(uint16 spell_id, float range)
{
	float extrange = 100;

	extrange += GetFocusEffect(focusRange, spell_id);

	return (range * extrange) / 100;
}

int64 Mob::GetActSpellDamage(uint16 spell_id, int64 value, Mob* target) {
	if (spells[spell_id].target_type == ST_Self) {
		return value;
	}

	if (IsNPC()) {
		value += value * CastToNPC()->GetSpellFocusDMG() / 100;

		if (CastToNPC()->GetSpellScale()) {
			value = int64(static_cast<float>(value) * CastToNPC()->GetSpellScale() / 100.0f);
		}
	}

	if (RuleB(Spells, AllowExtraDmgSkill) && RuleB(Character, ItemExtraSkillDamageCalcAsPercent) && GetSkillDmgAmt(spells[spell_id].skill) > 0) {
		value *= std::abs(GetSkillDmgAmt(spells[spell_id].skill) / 100);
	}

	bool Critical = false;
	int64 base_value = value;
	int chance = 0;
	int legacy_manaburn_cap = RuleI(Spells, LegacyManaburnCap);

	chance = RuleI(Spells, BaseCritChance); //Wizard base critical chance is 2% (Does not scale with level)
	chance += itembonuses.CriticalSpellChance + spellbonuses.CriticalSpellChance + aabonuses.CriticalSpellChance;
	chance += itembonuses.FrenziedDevastation + spellbonuses.FrenziedDevastation + aabonuses.FrenziedDevastation;

	//Crtical Hit Calculation pathway
	if (chance > 0 || (IsOfClientBot() && GetClass() == Class::Wizard && GetLevel() >= RuleI(Spells, WizCritLevel))) {

		 int32 ratio = RuleI(Spells, BaseCritRatio); //Critical modifier is applied from spell effects only. Keep at 100 for live like criticals.

		//Improved Harm Touch is a guaranteed crit if you have at least one level of SCF.
		if (spell_id == SPELL_IMP_HARM_TOUCH && IsOfClientBot() && (GetAA(aaSpellCastingFury) > 0) && (GetAA(aaUnholyTouch) > 0)) {
			 chance = 100;
		}

		if (spells[spell_id].override_crit_chance > 0 && chance > spells[spell_id].override_crit_chance) {
			chance = spells[spell_id].override_crit_chance;
		}

		if (zone->random.Roll(chance)) {
			Critical = true;
			ratio += itembonuses.SpellCritDmgIncrease + spellbonuses.SpellCritDmgIncrease + aabonuses.SpellCritDmgIncrease;
			ratio += itembonuses.SpellCritDmgIncNoStack + spellbonuses.SpellCritDmgIncNoStack + aabonuses.SpellCritDmgIncNoStack;
		} else if ((IsOfClientBot() && GetClass() == Class::Wizard) || (IsMerc() && GetClass() == CASTERDPS)) {
			if ((GetLevel() >= RuleI(Spells, WizCritLevel)) && zone->random.Roll(RuleI(Spells, WizCritChance))) {
				//Wizard innate critical chance is calculated seperately from spell effect and is not a set ratio. (20-70 is parse confirmed)
				ratio += zone->random.Int(RuleI(Spells, WizardCritMinimumRandomRatio), RuleI(Spells, WizardCritMaximumRandomRatio));
				Critical = true;
			}
		}

		if (IsOfClientBot() && GetClass() == Class::Wizard) {
			ratio += RuleI(Spells, WizCritRatio); //Default is zero
		}

		if (IsClient() && IsHarmTouchSpell(spell_id)) {
			ratio += RuleI(Spells, HarmTouchCritRatio); //Default is zero
		}

		if (Critical) {
			value = base_value*ratio/100;

			if (RuleB(Spells, UseClassicHarmTouchDamage)) {
				// Need to scale HT damage differently after level 40! It no longer scales by the constant value in the spell file. It scales differently, instead of 10 more damage per level, it does 30 more damage per level. So we multiply the level minus 40 times 20 if they are over level 40.
				if (IsHarmTouchSpell(spell_id) && GetLevel() > 40) {
					value -= (GetLevel() - 40) * 20;
				}

				//This adds the extra damage from the AA Unholy Touch, 450 per level to the AA Improved Harm TOuch.
				if (spell_id == SPELL_IMP_HARM_TOUCH && IsOfClientBotMerc()) { //Improved Harm Touch
					value -= GetAA(aaUnholyTouch) * 450; //Unholy Touch
				}
			}

			value += base_value*GetFocusEffect(focusImprovedDamage, spell_id) / 100;
			value += base_value*GetFocusEffect(focusImprovedDamage2, spell_id) / 100;

			value += int(base_value*GetFocusEffect(focusFcDamagePctCrit, spell_id) / 100) * ratio / 100;
			value += int(base_value*GetFocusEffect(focusFcAmplifyMod, spell_id) / 100) * ratio / 100;

			if (target) {
				value += int(base_value*target->GetVulnerability(this, spell_id, 0) / 100) * ratio / 100;
				value -= target->GetFcDamageAmtIncoming(this, spell_id);
			}

			value -= GetFocusEffect(focusFcDamageAmtCrit, spell_id) * ratio / 100;

			value -= GetFocusEffect(focusFcDamageAmt, spell_id);
			value -= GetFocusEffect(focusFcDamageAmt2, spell_id);
			value -= GetFocusEffect(focusFcAmplifyAmt, spell_id);

			if (RuleB(Spells, AllowExtraDmgSkill) && !RuleB(Character, ItemExtraSkillDamageCalcAsPercent)) {
				value -= GetSkillDmgAmt(spells[spell_id].skill) * ratio / 100;
			}

			if (RuleB(Spells, IgnoreSpellDmgLvlRestriction) && !spells[spell_id].no_heal_damage_item_mod && itembonuses.SpellDmg) {
				value -= GetExtraSpellAmt(spell_id, itembonuses.SpellDmg, base_value) * ratio / 100;

			} else if (!spells[spell_id].no_heal_damage_item_mod && itembonuses.SpellDmg && spells[spell_id].classes[(GetClass() % 17) - 1] >= GetLevel() - 5) {
				value -= GetExtraSpellAmt(spell_id, itembonuses.SpellDmg, base_value) * ratio / 100;
			}

			// legacy manaburn can crit, but is still held to the same cap
			if (RuleB(Spells, LegacyManaburn) && spell_id == SPELL_MANA_BURN) {
				if (value < -legacy_manaburn_cap) {
					value = -legacy_manaburn_cap;
				}
			}

			entity_list.FilteredMessageCloseString(
				this, true, 100, Chat::SpellCrit, FilterSpellCrits,
				OTHER_CRIT_BLAST, nullptr, GetName(), itoa(-value));

			if (IsClient()) {
				MessageString(Chat::SpellCrit, YOU_CRIT_BLAST, itoa(-value));
			}

			return value;
		}
	}
	//Non Crtical Hit Calculation pathway
	value = base_value;

	value += base_value*GetFocusEffect(focusImprovedDamage, spell_id) / 100;
	value += base_value*GetFocusEffect(focusImprovedDamage2, spell_id) / 100;

	value += base_value*GetFocusEffect(focusFcDamagePctCrit, spell_id) / 100;
	value += base_value*GetFocusEffect(focusFcAmplifyMod, spell_id) / 100;

	if (target) {
		value += base_value*target->GetVulnerability(this, spell_id, 0) / 100;
		value -= target->GetFcDamageAmtIncoming(this, spell_id);
	}

	value -= GetFocusEffect(focusFcDamageAmtCrit, spell_id);

	value -= GetFocusEffect(focusFcDamageAmt, spell_id);
	value -= GetFocusEffect(focusFcDamageAmt2, spell_id);
	value -= GetFocusEffect(focusFcAmplifyAmt, spell_id);

	if (RuleB(Spells, AllowExtraDmgSkill) && !RuleB(Character, ItemExtraSkillDamageCalcAsPercent)) {
		value -= GetSkillDmgAmt(spells[spell_id].skill);
	}

	if (RuleB(Spells, IgnoreSpellDmgLvlRestriction) && !spells[spell_id].no_heal_damage_item_mod && itembonuses.SpellDmg)
		value -= GetExtraSpellAmt(spell_id, itembonuses.SpellDmg, base_value);

	else if (
		!spells[spell_id].no_heal_damage_item_mod &&
		GetSpellDmg() &&
		spells[spell_id].classes[(GetClass() % 17) - 1] >= GetLevel() - 5
	) {
		value -= GetExtraSpellAmt(spell_id, GetSpellDmg(), base_value);
	}

	// Apply Manaburn Damage Cap
	if (RuleB(Spells, LegacyManaburn) && spell_id == SPELL_MANA_BURN) {
		if (value < -legacy_manaburn_cap) {
			value = -legacy_manaburn_cap;
		}
	}

	return value;
}

int64 Mob::GetActReflectedSpellDamage(uint16 spell_id, int64 value, int effectiveness) {
	/*
		Reflected spells use the spells base damage before any modifiers or formulas applied.
		That value can then be modifier by the reflect spells 'max' value, defined here as effectiveness
		Default effectiveness is set at 100.
		Extra Spell Damage stat from the with the reflect effect will be applied to reflected damage
		with no level limitation, this was confirmed with extensive parsing ~Kayen
	*/
	if (IsNPC()) {
		value += value * CastToNPC()->GetSpellFocusDMG() / 100;

		if (CastToNPC()->GetSpellScale()) {
			value = int64(static_cast<float>(value) * CastToNPC()->GetSpellScale() / 100.0f);
		}
	}

	if (RuleB(Spells, AllowExtraDmgSkill) && RuleB(Character, ItemExtraSkillDamageCalcAsPercent) && GetSkillDmgAmt(spells[spell_id].skill) > 0) {
		value *= std::abs(GetSkillDmgAmt(spells[spell_id].skill) / 100);
	}

	int64 base_spell_dmg = value;

	value = value * effectiveness / 100;

	if (RuleB(Spells, AllowExtraDmgSkill) && !RuleB(Character, ItemExtraSkillDamageCalcAsPercent)) {
		value -= GetSkillDmgAmt(spells[spell_id].skill);
	}

	if (!spells[spell_id].no_heal_damage_item_mod && itembonuses.SpellDmg) {
		value -= GetExtraSpellAmt(spell_id, itembonuses.SpellDmg, base_spell_dmg);
	}

	return value;
}

int64 Mob::GetActDoTDamage(uint16 spell_id, int64 value, Mob* target, bool from_buff_tic) {

	if (target == nullptr)
		return value;

	if (IsNPC()) {
		value += value * CastToNPC()->GetSpellFocusDMG() / 100;

		if (CastToNPC()->GetSpellScale()) {
			value = int64(static_cast<float>(value) * CastToNPC()->GetSpellScale() / 100.0f);
		}
	}

	if (RuleB(Spells, AllowExtraDmgSkill) && RuleB(Character, ItemExtraSkillDamageCalcAsPercent) && GetSkillDmgAmt(spells[spell_id].skill) > 0) {
		value *= std::abs(GetSkillDmgAmt(spells[spell_id].skill) / 100);
	}

	int64 base_value = value;
	int64 extra_dmg = 0;
	int16 chance = 0;
	chance += itembonuses.CriticalDoTChance + spellbonuses.CriticalDoTChance + aabonuses.CriticalDoTChance;

	if (spellbonuses.CriticalDotDecay)
		chance += GetDecayEffectValue(spell_id, SpellEffect::CriticalDotDecay);

	if (spells[spell_id].override_crit_chance > 0 && chance > spells[spell_id].override_crit_chance)
		chance = spells[spell_id].override_crit_chance;

	if (!spells[spell_id].good_effect && chance > 0 && (zone->random.Roll(chance))) {
		int64 ratio = 200;
		ratio += itembonuses.DotCritDmgIncrease + spellbonuses.DotCritDmgIncrease + aabonuses.DotCritDmgIncrease;
		value = base_value*ratio/100;
		value += int64(base_value*GetFocusEffect(focusImprovedDamage, spell_id, nullptr, from_buff_tic)/100)*ratio/100;
		value += int64(base_value*GetFocusEffect(focusImprovedDamage2, spell_id, nullptr, from_buff_tic)/100)*ratio/100;
		value += int64(base_value*GetFocusEffect(focusFcDamagePctCrit, spell_id, nullptr, from_buff_tic)/100)*ratio/100;
		value += int64(base_value*GetFocusEffect(focusFcAmplifyMod, spell_id, nullptr, from_buff_tic) / 100)*ratio/100;
		value += int64(base_value*target->GetVulnerability(this, spell_id, 0, from_buff_tic)/100)*ratio/100;
		extra_dmg = target->GetFcDamageAmtIncoming(this, spell_id, from_buff_tic) +
					int64(GetFocusEffect(focusFcDamageAmtCrit, spell_id, nullptr, from_buff_tic)*ratio/100) +
					GetFocusEffect(focusFcDamageAmt, spell_id, nullptr, from_buff_tic) +
					GetFocusEffect(focusFcDamageAmt2, spell_id, nullptr, from_buff_tic) +
					GetFocusEffect(focusFcAmplifyAmt, spell_id, nullptr, from_buff_tic);

		if (RuleB(Spells, DOTsScaleWithSpellDmg)) {
			if (
				RuleB(Spells, IgnoreSpellDmgLvlRestriction) &&
				!spells[spell_id].no_heal_damage_item_mod &&
				GetSpellDmg()
			) {
				extra_dmg += GetExtraSpellAmt(spell_id, GetSpellDmg(), base_value)*ratio/100;
			}
			else if (
				!spells[spell_id].no_heal_damage_item_mod &&
				GetSpellDmg() &&
				spells[spell_id].classes[(GetClass() % 17) - 1] >= GetLevel() - 5
			) {
				extra_dmg += GetExtraSpellAmt(spell_id, GetSpellDmg(), base_value)*ratio/100;
			}
		}

		if (RuleB(Spells, AllowExtraDmgSkill) && !RuleB(Character, ItemExtraSkillDamageCalcAsPercent)) {
			extra_dmg += GetSkillDmgAmt(spells[spell_id].skill) * ratio / 100;
		}

		if (RuleB(Spells, DOTBonusDamageSplitOverDuration)) {
			if (extra_dmg) {
				const int duration = CalcBuffDuration(this, target, spell_id);
				if (duration > 0) {
					extra_dmg /= duration;
				}
			}
		}

		value -= extra_dmg;
	}
	else {

		value = base_value;
		value += base_value*GetFocusEffect(focusImprovedDamage, spell_id, nullptr, from_buff_tic)/100;
		value += base_value*GetFocusEffect(focusImprovedDamage2, spell_id, nullptr, from_buff_tic)/100;
		value += base_value*GetFocusEffect(focusFcDamagePctCrit, spell_id, nullptr, from_buff_tic)/100;
		value += base_value*GetFocusEffect(focusFcAmplifyMod, spell_id, nullptr, from_buff_tic)/100;
		value += base_value*target->GetVulnerability(this, spell_id, 0, from_buff_tic)/100;
		extra_dmg = target->GetFcDamageAmtIncoming(this, spell_id, from_buff_tic) +
					GetFocusEffect(focusFcDamageAmtCrit, spell_id, nullptr, from_buff_tic) +
					GetFocusEffect(focusFcDamageAmt, spell_id, nullptr, from_buff_tic) +
					GetFocusEffect(focusFcDamageAmt2, spell_id, nullptr, from_buff_tic) +
					GetFocusEffect(focusFcAmplifyAmt, spell_id, nullptr, from_buff_tic);

		if (RuleB(Spells, DOTsScaleWithSpellDmg)) {
			if (
				RuleB(Spells, IgnoreSpellDmgLvlRestriction) &&
				!spells[spell_id].no_heal_damage_item_mod &&
				GetSpellDmg()
			) {
				extra_dmg += GetExtraSpellAmt(spell_id, GetSpellDmg(), base_value);
			}
			else if (
				!spells[spell_id].no_heal_damage_item_mod &&
				GetSpellDmg() &&
				spells[spell_id].classes[(GetClass() % 17) - 1] >= GetLevel() - 5
			) {
				extra_dmg += GetExtraSpellAmt(spell_id, GetSpellDmg(), base_value);
			}
		}

		if (RuleB(Spells, AllowExtraDmgSkill) && !RuleB(Character, ItemExtraSkillDamageCalcAsPercent)) {
			extra_dmg += GetSkillDmgAmt(spells[spell_id].skill);
		}

		if (RuleB(Spells, DOTBonusDamageSplitOverDuration)) {
			if (extra_dmg) {
				const int duration = CalcBuffDuration(this, target, spell_id);
				if (duration > 0) {
					extra_dmg /= duration;
				}
			}
		}

		value -= extra_dmg;
	}

	return value;
}

int64 Mob::GetExtraSpellAmt(uint16 spell_id, int64 extra_spell_amt, int64 base_spell_dmg)
{

	if (RuleB(Spells, FlatItemExtraSpellAmt)) {
		if (RuleB(Spells, ItemExtraSpellAmtCalcAsPercent)) {
			return std::abs(base_spell_dmg) * extra_spell_amt / 100;
		} else {
			return extra_spell_amt;
		}
	}

	int total_cast_time = 0;

	if (spells[spell_id].recast_time >= spells[spell_id].recovery_time) {
		total_cast_time = spells[spell_id].recast_time + spells[spell_id].cast_time;
	} else {
		total_cast_time = spells[spell_id].recovery_time + spells[spell_id].cast_time;
	}

	if (total_cast_time > 0 && total_cast_time <= 2500) {
		extra_spell_amt = extra_spell_amt * 25 / 100;
	} else if (total_cast_time > 2500 && total_cast_time < 7000) {
		extra_spell_amt = extra_spell_amt * (167 * ((total_cast_time - 1000) / 1000)) / 1000;
	} else {
		extra_spell_amt = extra_spell_amt * total_cast_time / 7000;
	}

	//Confirmed with parsing 10/9/21 ~Kayen
	if (extra_spell_amt * 2 > std::abs(base_spell_dmg)) {
		extra_spell_amt = std::abs(base_spell_dmg) / 2;
	}

	if (RuleB(Spells, ItemExtraSpellAmtCalcAsPercent)) {
		return std::abs(base_spell_dmg) * extra_spell_amt / 100;
	}

	return extra_spell_amt;
}

int64 Mob::GetActSpellHealing(uint16 spell_id, int64 value, Mob* target, bool from_buff_tic) {
	if (target == nullptr && IsBot()) {
		target = this;
	}

	if (IsNPC()) {
		value += value * CastToNPC()->GetSpellFocusHeal() / 100;

		if (IsNPC() && CastToNPC()->GetHealScale()) {
			value = int(static_cast<float>(value) * CastToNPC()->GetHealScale() / 100.0f);
		}
	}

	if (RuleB(Spells, AllowExtraDmgSkill) && RuleB(Character, ItemExtraSkillDamageCalcAsPercent) && GetSkillDmgAmt(spells[spell_id].skill) > 0) {
		value *= std::abs(GetSkillDmgAmt(spells[spell_id].skill) / 100);
	}

	int64 base_value = value;
	int16 critical_chance = 0;
	int8  critical_modifier = 1;

	if (spells[spell_id].buff_duration < 1) {
		critical_chance += itembonuses.CriticalHealChance + spellbonuses.CriticalHealChance + aabonuses.CriticalHealChance;

		if (spellbonuses.CriticalHealDecay) {
			critical_chance += GetDecayEffectValue(spell_id, SpellEffect::CriticalHealDecay);
		}
	}
	else {
		critical_chance = itembonuses.CriticalHealOverTime + spellbonuses.CriticalHealOverTime + aabonuses.CriticalHealOverTime;

		if (spellbonuses.CriticalRegenDecay) {
			critical_chance += GetDecayEffectValue(spell_id, SpellEffect::CriticalRegenDecay);
		}
	}

	if (critical_chance) {

		if (spells[spell_id].override_crit_chance > 0 && critical_chance > spells[spell_id].override_crit_chance) {
			critical_chance = spells[spell_id].override_crit_chance;
		}

		if (zone->random.Roll(critical_chance)) {
			critical_modifier = 2; //At present time no critical heal amount modifier SPA exists.
		}
	}

	if (GetClass() == Class::Cleric) {
		value += int64(base_value*RuleI(Spells, ClericInnateHealFocus) / 100);  //confirmed on live parsing clerics get an innate 5 pct heal focus
	}
	value += int64(base_value*GetFocusEffect(focusImprovedHeal, spell_id, nullptr, from_buff_tic) / 100);
	value += int64(base_value*GetFocusEffect(focusFcAmplifyMod, spell_id, nullptr, from_buff_tic) / 100);

	// Instant Heals
	if (spells[spell_id].buff_duration < 1) {

		if (RuleB(Spells, AllowExtraDmgSkill) && !RuleB(Character, ItemExtraSkillDamageCalcAsPercent)) {
			value += GetSkillDmgAmt(spells[spell_id].skill);
		}

		if (target) {
			value += int64(base_value * target->GetFocusEffect(focusFcHealPctIncoming, spell_id, this)/100); //SPA 393 Add before critical
			value += int64(base_value * target->GetFocusEffect(focusFcHealPctCritIncoming, spell_id, this)/100); //SPA 395 Add before critical (?)
		}

		value += GetFocusEffect(focusFcHealAmtCrit, spell_id); //SPA 396 Add before critical

		//Using IgnoreSpellDmgLvlRestriction to also allow healing to scale
		if (
			RuleB(Spells, IgnoreSpellDmgLvlRestriction) &&
			!spells[spell_id].no_heal_damage_item_mod &&
			GetHealAmt()
		) {
			value += GetExtraSpellAmt(spell_id, GetHealAmt(), base_value); //Item Heal Amt Add before critical
		}
		else if (
			!spells[spell_id].no_heal_damage_item_mod &&
			GetHealAmt() &&
			spells[spell_id].classes[(GetClass() % 17) - 1] >= GetLevel() - 5
		) {
			value += GetExtraSpellAmt(spell_id, GetHealAmt(), base_value); //Item Heal Amt Add before critical
		}

		if (target) {
			value += value * target->GetHealRate() / 100; //SPA 120 modifies value after Focus Applied but before critical
		}

		/*
			Apply critical hit modifier
		*/

		value *= critical_modifier;
		value += GetFocusEffect(focusFcHealAmt, spell_id); //SPA 392 Add after critical
		value += GetFocusEffect(focusFcAmplifyAmt, spell_id); //SPA 508 ? Add after critical

		if (target) {
			value += target->GetFocusEffect(focusFcHealAmtIncoming, spell_id, this); //SPA 394 Add after critical
		}

		if (critical_modifier > 1) {
			entity_list.MessageCloseString(
				this, true, 100, Chat::SpellCrit,
				OTHER_CRIT_HEAL, GetName(), itoa(value));

			if (IsClient()) {
				MessageString(Chat::SpellCrit, YOU_CRIT_HEAL, itoa(value));
			}
		}

		return value;
	}

	//Heal over time spells. [Heal Rate and Additional Healing effects do not increase this value]
	else {
		//Using IgnoreSpellDmgLvlRestriction to also allow healing to scale
		int64 extra_heal = 0;

		if (RuleB(Spells, AllowExtraDmgSkill) && !RuleB(Character, ItemExtraSkillDamageCalcAsPercent)) {
			extra_heal += GetSkillDmgAmt(spells[spell_id].skill);
		}

		if (RuleB(Spells, HOTsScaleWithHealAmt)) {
			if (
				RuleB(Spells, IgnoreSpellDmgLvlRestriction) &&
				!spells[spell_id].no_heal_damage_item_mod &&
				GetHealAmt()
			) {
				extra_heal += GetExtraSpellAmt(spell_id, GetHealAmt(), base_value);
			}
			else if (
				!spells[spell_id].no_heal_damage_item_mod &&
				GetHealAmt() &&
				spells[spell_id].classes[(GetClass() % 17) - 1] >= GetLevel() - 5
			) {
				extra_heal += GetExtraSpellAmt(spell_id, GetHealAmt(), base_value);
			}
		}

		if (extra_heal) {
			if (RuleB(Spells, HOTBonusHealingSplitOverDuration)) {
				const int duration = CalcBuffDuration(this, target, spell_id);
				if (duration > 0) {
					extra_heal /= duration;
				}
			}

			value += extra_heal;
		}

		value *= critical_modifier;
	}

	return value;
}


int32 Mob::GetActSpellCost(uint16 spell_id, int32 cost)
{
	//FrenziedDevastation doubles mana cost of all DD spells
	int16 FrenziedDevastation = itembonuses.FrenziedDevastation + spellbonuses.FrenziedDevastation + aabonuses.FrenziedDevastation;

	if (FrenziedDevastation && IsPureNukeSpell(spell_id))
		cost *= 2;

	// Formula = Unknown exact, based off a random percent chance up to mana cost(after focuses) of the cast spell
	if(itembonuses.Clairvoyance && spells[spell_id].classes[(GetClass()%17) - 1] >= GetLevel() - 5)
	{
		int mana_back = itembonuses.Clairvoyance * zone->random.Int(1, 100) / 100;
		// Doesnt generate mana, so best case is a free spell
		if(mana_back > cost)
			mana_back = cost;

		cost -= mana_back;
	}

	int spec = GetSpecializeSkillValue(spell_id);
	int PercentManaReduction = 0;
	if (spec)
		PercentManaReduction = 1 + spec / 20; // there seems to be some non-obvious rounding here, let's truncate for now.

	int16 focus_redux = GetFocusEffect(focusManaCost, spell_id);
	PercentManaReduction += focus_redux;

	cost -= cost * PercentManaReduction / 100;

	// Gift of Mana - reduces spell cost to 1 mana
	if(focus_redux >= 100) {
		int buff_max = GetMaxTotalSlots();
		for (int buffSlot = 0; buffSlot < buff_max; buffSlot++) {
			if (buffs[buffSlot].spellid == 0 || buffs[buffSlot].spellid >= SPDAT_RECORDS)
				continue;

			if(IsEffectInSpell(buffs[buffSlot].spellid, SpellEffect::ReduceManaCost)) {
				if(CalcFocusEffect(focusManaCost, buffs[buffSlot].spellid, spell_id) == 100)
					cost = 1;
			}
		}
	}

	if(cost < 0)
		cost = 0;

	return cost;
}

int32 Mob::GetActSpellDuration(uint16 spell_id, int32 duration)
{
	// focuses don't affect discipline duration (Except War Cries)
	if (
		IsDiscipline(spell_id) &&
		(
			spell_id != SPELL_BATTLE_CRY &&
			spell_id != SPELL_WAR_CRY &&
			spell_id != SPELL_BATTLE_CRY_OF_DRAVEL &&
			spell_id != SPELL_WAR_CRY_OF_DRAVEL &&
			spell_id != SPELL_BATTLE_CRY_OF_THE_MASTRUQ &&
			spell_id != SPELL_ANCIENT_CRY_OF_CHAOS &&
			spell_id != SPELL_BLOODTHIRST
		)
	) {
		return duration;
	}

	int increase = 100;
	increase += GetFocusEffect(focusSpellDuration, spell_id);
	int tic_inc = 0;
	tic_inc = GetFocusEffect(focusSpellDurByTic, spell_id);

	float focused = ((duration * increase) / 100.0f) + tic_inc;
	int ifocused = static_cast<int>(focused);

	// 7.6 is rounded to 7, 8.6 is rounded to 9
	// 6 is 6, etc
	if (FCMP(focused, ifocused) || ifocused % 2) // equal or odd
		return ifocused;
	else // even and not equal round to odd
		return ifocused + 1;
}

bool Client::TrainDiscipline(uint32 itemid) {

	//get the item info
	const EQ::ItemData *item = database.GetItem(itemid);
	if (!item) {
		Message(Chat::Red, "Unable to find the tome you turned in!");
		LogError("Unable to find turned in tome id [{}]", itemid);
		return false;
	}

	if (!item->IsClassCommon() || item->ItemType != EQ::item::ItemTypeSpell) {
		Message(Chat::Red, "Invalid item type, you cannot learn from this item.");
		//summon them the item back...
		SummonItem(itemid);
		return false;
	}

	//Need a way to determine the difference between a spell and a tome
	//so they cant turn in a spell and get it as a discipline
	//this is kinda a hack:
	const std::string item_name = item->Name;

	if (
		!Strings::BeginsWith(item_name, "Tome of ") &&
		!Strings::BeginsWith(item_name, "Skill: ")
	) {
		Message(Chat::Red, "This item is not a tome.");
		//summon them the item back...
		SummonItem(itemid);
		return false;
	}

	const auto player_class = GetClass();
	if (player_class == Class::Wizard || player_class == Class::Enchanter || player_class == Class::Magician || player_class == Class::Necromancer) {
		Message(Chat::Red, "Your class cannot learn from this tome.");
		//summon them the item back...
		SummonItem(itemid);
		return false;
	}

	//make sure we can train this...
	//can we use the item?
	const auto class_bit = static_cast<uint32>(1 << (player_class - 1));
	if (!(item->Classes & class_bit)) {
		Message(Chat::Red, "Your class cannot learn from this tome.");
		//summon them the item back...
		SummonItem(itemid);
		return false;
	}

	const auto spell_id = static_cast<uint32>(item->Scroll.Effect);
	if (!IsValidSpell(spell_id)) {
		Message(Chat::Red, "This tome Contains invalid knowledge.");
		return false;
	}

	//can we use the spell?
	const auto& spell = spells[spell_id];
	const auto level_to_use = spell.classes[player_class - 1];
	if (level_to_use == 255) {
		Message(Chat::Red, "Your class cannot learn from this tome.");
		//summon them the item back...
		SummonItem(itemid);
		return false;
	}

	if (level_to_use > GetLevel()) {
		Message(Chat::Red, fmt::format("You must be at least level {} to learn this discipline.", level_to_use).c_str());
		//summon them the item back...
		SummonItem(itemid);
		return false;
	}

	//add it to PP.
	for (int r = 0; r < MAX_PP_DISCIPLINES; r++) {
		if (m_pp.disciplines.values[r] == spell_id) {
			Message(Chat::Red, "You already know this discipline.");
			//summon them the item back...
			SummonItem(itemid);
			return false;
		} else if (m_pp.disciplines.values[r] == 0) {
			m_pp.disciplines.values[r] = spell_id;
			database.SaveCharacterDiscipline(CharacterID(), r, spell_id);
			SendDisciplineUpdate();
			Message(Chat::White, "You have learned a new discipline!");
			return true;
		}
	}

	Message(Chat::Red, "You have learned too many disciplines and can learn no more.");
	return false;
}

bool Client::MemorizeSpellFromItem(uint32 item_id) {
	const auto& item = database.GetItem(item_id);
	if (!item) {
		Message(Chat::Red, "Unable to find the scroll!");
		LogError("Unable to find scroll id [{}]", item_id);
		return false;
	}

	if (!item->IsClassCommon() || item->ItemType != EQ::item::ItemTypeSpell) {
		Message(Chat::Red, "Invalid item type, you cannot learn from this item.");
		SummonItem(item_id);
		return false;
	}

	const std::string item_name = item->Name;

	if (
		!Strings::BeginsWith(item_name, "Spell: ") &&
		!Strings::BeginsWith(item_name, "Song: ")
	) {
		Message(Chat::Red, "This item is not a scroll.");
		SummonItem(item_id);
		return false;
	}

	const auto class_bit = static_cast<uint32>(1 << (GetClass() - 1));

	if (!(item->Classes & class_bit)) {
		Message(Chat::Red, "Your class cannot learn from this scroll.");
		SummonItem(item_id);
		return false;
	}

	const auto spell_id = static_cast<uint32>(item->Scroll.Effect);
	if (!IsValidSpell(spell_id)) {
		Message(Chat::Red, "This scroll Contains invalid knowledge.");
		return false;
	}

	const auto& spell = spells[spell_id];
	const auto level_to_use = spell.classes[GetClass() - 1];
	if (level_to_use == 255) {
		Message(Chat::Red, "Your class cannot learn from this scroll.");
		SummonItem(item_id);
		return false;
	}

	if (level_to_use > GetLevel()) {
		Message(Chat::Red, fmt::format("You must be at least level {} to learn this spell.", level_to_use).c_str());
		SummonItem(item_id);
		return false;
	}

	for (int index = 0; index < EQ::spells::SPELLBOOK_SIZE; index++) {
		if (!HasSpellScribed(spell_id)) {
			auto next_slot = GetNextAvailableSpellBookSlot();
			if (next_slot != -1) {
				ScribeSpell(spell_id, next_slot);
				return true;
			} else {
				Message(
					Chat::Red,
					fmt::format(
						"Unable to scribe {} ({}) to spellbook, no more spell book slots available.",
						((spell_id >= 0 && spell_id < SPDAT_RECORDS) ? spells[spell_id].name : "Out-of-range"),
						spell_id
					).c_str()
				);
				SummonItem(item_id);
				return false;
			}
		} else {
			Message(Chat::Red, "You already know this spell.");
			SummonItem(item_id);
			return false;
		}
	}

	Message(Chat::Red, "You have learned too many spells and can learn no more.");
	return false;
}

void Client::TrainDiscBySpellID(int32 spell_id)
{
	int i;
	for(i = 0; i < MAX_PP_DISCIPLINES; i++) {
		if(m_pp.disciplines.values[i] == 0) {
			m_pp.disciplines.values[i] = spell_id;
			database.SaveCharacterDiscipline(CharacterID(), i, spell_id);
			SendDisciplineUpdate();
			Message(Chat::Yellow, "You have learned a new combat ability!");
			return;
		}
	}
}

int Client::GetDiscSlotBySpellID(int32 spellid)
{
	int i;

	for(i = 0; i < MAX_PP_DISCIPLINES; i++)
	{
		if(m_pp.disciplines.values[i] == spellid)
			return i;
	}

	return -1;
}

void Client::SendDisciplineUpdate() {
	EQApplicationPacket app(OP_DisciplineUpdate, sizeof(Disciplines_Struct));
	Disciplines_Struct *d = (Disciplines_Struct*)app.pBuffer;
	memcpy(d, &m_pp.disciplines, sizeof(m_pp.disciplines));
	QueuePacket(&app);
}

bool Client::UseDiscipline(uint32 spell_id, uint32 target) {
	// Dont let client waste a reuse timer if they can't use the disc
	if ((IsStunned() && !IsCastNotStandingSpell(spell_id))||
		IsFeared() ||
		(IsMezzed() && !IsCastNotStandingSpell(spell_id)) ||
		IsAmnesiad() ||
		IsPet())
	{
		if (IsAmnesiad()) {
			MessageString(Chat::Red, MELEE_SILENCE);
		}
		return false;
	}

	//make sure we have the spell...
	int r;
	for(r = 0; r < MAX_PP_DISCIPLINES; r++) {
		if(m_pp.disciplines.values[r] == spell_id)
			break;
	}
	if(r == MAX_PP_DISCIPLINES)
		return false;	//not found.

	//make sure we can use it..
	if(!IsValidSpell(spell_id)) {
		Message(Chat::Red, "This tome Contains invalid knowledge.");
		return false;
	}

	if (DivineAura() && !IsCastNotStandingSpell(spell_id)) {
		return false;
	}

	//can we use the spell?
	const SPDat_Spell_Struct &spell = spells[spell_id];
	uint8 level_to_use = spell.classes[GetClass() - 1];
	if(level_to_use == 255) {
		Message(Chat::Red, "Your class cannot learn from this tome.");
		//should summon them a new one...
		return false;
	}

	if(level_to_use > GetLevel()) {
		MessageString(Chat::Red, DISC_LEVEL_USE_ERROR);
		//should summon them a new one...
		return false;
	}

	if(GetEndurance() < spell.endurance_cost) {
		Message(11, "You are too fatigued to use this skill right now.");
		return false;
	}

	// sneak attack discs require you to be hidden for 4 seconds before use
	if (spell.sneak && (!hidden || (hidden && (Timer::GetCurrentTime() - tmHidden) < 4000))) {
		MessageString(Chat::SpellFailure, SNEAK_RESTRICT);
		return false;
	}

	// the client does this check before calling CastSpell, should prevent discs being eaten
	if (spell.buff_duration_formula != 0 && spell.target_type == ST_Self && HasDiscBuff())
		return false;

	//Check the disc timer
	pTimerType DiscTimer = pTimerDisciplineReuseStart + spell.timer_id;
	if(!p_timers.Expired(&database, DiscTimer, false)) { // lets not set the reuse timer in case CastSpell fails (or we would have to turn off the timer, but CastSpell will set it as well)
		uint32 remaining_time = p_timers.GetRemainingTime(DiscTimer);
		Message(
			Chat::White,
			fmt::format(
				"You can use this discipline in {}.",
				Strings::SecondsToTime(remaining_time)
			).c_str()
		);
		return false;
	}

	bool instant_recast = true;

	if (spell.recast_time > 0) {
		uint32 reduced_recast = spell.recast_time / 1000;
		auto focus = GetFocusEffect(focusReduceRecastTime, spell_id);
		// do stupid stuff because custom servers.
		// we really should be able to just do the -= focus but since custom servers could have shorter reuse timers
		// we have to make sure we don't underflow the uint32 ...
		// and yes, the focus effect can be used to increase the durations (spell 38944)
		if (focus > reduced_recast) {
			reduced_recast = 0;
			if (GetPTimers().Enabled((uint32)DiscTimer))
				GetPTimers().Clear(&database, (uint32)DiscTimer);
		}
		else {
			reduced_recast -= focus;
		}

		if (reduced_recast > 0) {
			instant_recast = false;

			if (GetClass() == Class::Bard && IsCasting() && spells[spell_id].cast_time == 0) {
				if (DoCastingChecksOnCaster(spell_id, EQ::spells::CastingSlot::Discipline)) {
					SpellFinished(spell_id, entity_list.GetMob(target), EQ::spells::CastingSlot::Discipline, 0, -1, spells[spell_id].resist_difficulty, false, -1, (uint32)DiscTimer, reduced_recast, false);
				}
			}
			else {
				if (!CastSpell(spell_id, target, EQ::spells::CastingSlot::Discipline, -1, -1, 0, -1, (uint32)DiscTimer, reduced_recast)) {
					return false;
				}
			}
			SendDisciplineTimer(spells[spell_id].timer_id, reduced_recast);
		}
	}

	if (instant_recast) {
		if (GetClass() == Class::Bard && IsCasting() && spells[spell_id].cast_time == 0) {
			if (DoCastingChecksOnCaster(spell_id, EQ::spells::CastingSlot::Discipline)) {
				SpellFinished(spell_id, entity_list.GetMob(target), EQ::spells::CastingSlot::Discipline, 0, -1, spells[spell_id].resist_difficulty, false, -1, 0xFFFFFFFF, 0, false);
			}
		}
		else {
			CastSpell(spell_id, target, EQ::spells::CastingSlot::Discipline);
		}
	}
	return true;
}

uint32 Client::GetDisciplineTimer(uint32 timer_id) {
	pTimerType disc_timer_id = pTimerDisciplineReuseStart + timer_id;
	uint32 disc_timer = 0;
	if (GetPTimers().Enabled(disc_timer_id)) {
		disc_timer = GetPTimers().GetRemainingTime(disc_timer_id);
	}
	return disc_timer;
}

void Client::ResetDisciplineTimer(uint32 timer_id) {
	pTimerType disc_timer_id = pTimerDisciplineReuseStart + timer_id;
	if (GetPTimers().Enabled(disc_timer_id)) {
		GetPTimers().Clear(&database, disc_timer_id);
	}
	SendDisciplineTimer(timer_id, 0);
}

void Client::ResetAllDisciplineTimers() {
	for (pTimerType disc_timer_id = pTimerDisciplineReuseStart; disc_timer_id <= pTimerDisciplineReuseEnd; disc_timer_id++) {
		uint32 current_timer_id = (disc_timer_id - pTimerDisciplineReuseStart);
		if (GetPTimers().Enabled(disc_timer_id)) {
			GetPTimers().Clear(&database, disc_timer_id);
		}
		SendDisciplineTimer(current_timer_id, 0);
	}
}

bool Client::HasDisciplineLearned(uint16 spell_id) {
	bool has_learned = false;
	for (auto index = 0; index < MAX_PP_DISCIPLINES; ++index) {
		if (GetPP().disciplines.values[index] == spell_id) {
			has_learned = true;
			break;
		}
	}
	return has_learned;
}

void Client::SendDisciplineTimer(uint32 timer_id, uint32 duration)
{
	if (timer_id < MAX_DISCIPLINE_TIMERS)
	{
		auto outapp = new EQApplicationPacket(OP_DisciplineTimer, sizeof(DisciplineTimer_Struct));
		DisciplineTimer_Struct *dts = (DisciplineTimer_Struct *)outapp->pBuffer;
		dts->TimerID = timer_id;
		dts->Duration = duration;
		QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void EntityList::AETaunt(Client* taunter, float range, int bonus_hate)
{
	if (range == 0) {
		range = 40;
	}

	float range_squared = range * range;

	for (auto& it: taunter->GetCloseMobList(range)) {
		Mob *them = it.second;
		if (!them) {
			continue;
		}

		if (!them->IsNPC()) {
			continue;
		}

		float z_difference = taunter->GetZ() - them->GetZ();
		if (z_difference < 0) {
			z_difference *= -1;
		}

		if (
			z_difference < 10 &&
			taunter->IsAttackAllowed(them) &&
			DistanceSquaredNoZ(taunter->GetPosition(), them->GetPosition()) <= range_squared
		) {
			if (taunter->CheckLosFN(them)) {
				taunter->Taunt(them->CastToNPC(), true, 0, true, bonus_hate);
			}
		}
	}
}

void EntityList::AESpell(
	Mob* caster_mob,
	Mob* center_mob,
	uint16 spell_id,
	bool affect_caster,
	int16 resist_adjust,
	int* max_targets,
	bool is_scripted
)
{
	const auto& cast_target_position = (
		(!is_scripted && spells[spell_id].target_type == ST_Ring) ?
		caster_mob->GetTargetRingLocation() :
		static_cast<glm::vec3>(center_mob->GetPosition())
	);

	Mob* current_mob = nullptr;

	bool      is_detrimental_spell = IsDetrimentalSpell(spell_id);
	bool      is_npc               = caster_mob->IsNPC();
	float     distance             = caster_mob->GetAOERange(spell_id);
	float     distance_squared     = distance * distance;
	float     min_range_squared    = spells[spell_id].min_range * spells[spell_id].min_range;
	glm::vec2 min                  = { cast_target_position.x - distance, cast_target_position.y - distance };
	glm::vec2 max                  = { cast_target_position.x + distance, cast_target_position.y + distance };

	/**
	 * If using Old Rain Targets - there is no max target limitation
	 */
	if (RuleB(Spells, OldRainTargets)) {
		max_targets = nullptr;
	}

	int max_targets_allowed = RuleI(Spells, DefaultAOEMaxTargets);;
	if (max_targets) { // rains pass this in since they need to preserve the count through waves
		max_targets_allowed = *max_targets;
	} else if (spells[spell_id].aoe_max_targets) {
		max_targets_allowed = spells[spell_id].aoe_max_targets;
	} else if (
		IsTargetableAESpell(spell_id) &&
		is_detrimental_spell &&
		!is_npc &&
		!IsEffectInSpell(spell_id, SpellEffect::Lull) &&
		!IsEffectInSpell(spell_id, SpellEffect::Mez)
	) {
		max_targets_allowed = RuleI(Spells, TargetedAOEMaxTargets);
	} else if (
		IsPBAESpell(spell_id) &&
		IsDetrimentalSpell &&
		!is_npc
	) {
		max_targets_allowed = RuleI(Spells, PointBlankAOEMaxTargets);
	}

	int   target_hit_counter = 0;
	float distance_to_target = 0;

	LogAoeCast(
		"Close scan distance [{}] cast distance [{}]",
		RuleI(Range, MobCloseScanDistance),
		distance
	);
	auto list = caster_mob->GetCloseMobList(distance);
	for (auto& it: list) {
		current_mob = it.second;
		if (!current_mob) {
			continue;
		}

		LogAoeCast("Checking AOE against mob [{}]", current_mob->GetCleanName());

		if (current_mob->IsClient() && !current_mob->CastToClient()->ClientFinishedLoading()) {
			continue;
		}

		if (current_mob == caster_mob && !affect_caster) {
			continue;
		}

		if (spells[spell_id].target_type == ST_TargetAENoPlayersPets && current_mob->IsPetOwnerOfClientBot()) {
			continue;
		}

		if (spells[spell_id].target_type == ST_AreaClientOnly && !current_mob->IsOfClientBot()) {
			continue;
		}

		if (spells[spell_id].target_type == ST_AreaNPCOnly && !current_mob->IsNPC()) {
			continue;
		}

		if (spells[spell_id].pcnpc_only_flag == PCNPCOnlyFlagType::PC && !current_mob->IsOfClientBotMerc()) {
			continue;
		}

		if (spells[spell_id].pcnpc_only_flag == PCNPCOnlyFlagType::NPC && current_mob->IsOfClientBotMerc()) {
			continue;
		}

		if (!IsWithinAxisAlignedBox(static_cast<glm::vec2>(current_mob->GetPosition()), min, max)) {
			continue;
		}

		distance_to_target = DistanceSquared(current_mob->GetPosition(), cast_target_position);

		if (distance_to_target > distance_squared) {
			continue;
		}

		if (distance_to_target < min_range_squared) {
			continue;
		}

		if (
			is_npc &&
			current_mob->IsNPC() &&
			spells[spell_id].target_type != ST_AreaNPCOnly
		) {
			const auto faction_value = current_mob->GetReverseFactionCon(caster_mob);
			if (is_detrimental_spell) {
				if (
					!(caster_mob->CheckAggro(current_mob) ||
					faction_value == FACTION_THREATENINGLY ||
					faction_value == FACTION_SCOWLS)
				) {
					continue;
				}
			} else {
				if (!(faction_value <= FACTION_AMIABLY)) {
					continue;
				}
			}
		}

		if (is_detrimental_spell) {
			if (!caster_mob->IsAttackAllowed(current_mob, true)) {
				continue;
			}

			if (center_mob && !spells[spell_id].npc_no_los && !center_mob->CheckLosFN(current_mob)) {
				continue;
			}

			if (!center_mob && !spells[spell_id].npc_no_los && !caster_mob->CheckLosFN(
				caster_mob->GetTargetRingX(),
				caster_mob->GetTargetRingY(),
				caster_mob->GetTargetRingZ(),
				current_mob->GetSize())) {
				continue;
			}
		} else {
			/**
			 * Check to stop casting beneficial ae buffs (to wit: bard songs) on enemies...
			 * This does not check faction for beneficial AE buffs... only agro and attackable.
			 * I've tested for spells that I can find without problem, but a faction-based
			 * check may still be needed. Any changes here should also reflect in BardAEPulse()
			 */
			if (caster_mob->IsAttackAllowed(current_mob, true)) {
				continue;
			}

			if (caster_mob->CheckAggro(current_mob)) {
				continue;
			}
		}

		current_mob->CalcSpellPowerDistanceMod(spell_id, distance_to_target);
		caster_mob->SpellOnTarget(spell_id, current_mob, 0, true, resist_adjust);

		/**
		 * Increment hit count if max targets
		 */
		if (max_targets_allowed) {
			target_hit_counter++;
			if (target_hit_counter >= max_targets_allowed) {
				break;
			}
		}
	}

	LogAoeCast("Done iterating [{}]", caster_mob->GetCleanName());

	if (max_targets && max_targets_allowed) {
		*max_targets = *max_targets - target_hit_counter;
	}
}

void EntityList::MassGroupBuff(
	Mob* caster,
	Mob* center,
	uint16 spell_id,
	bool affect_caster
)
{
	Mob*  current_mob          = nullptr;
	float distance             = caster->GetAOERange(spell_id);
	float distance_squared     = distance * distance;
	bool  is_detrimental_spell = IsDetrimentalSpell(spell_id);

	for (auto& it: caster->GetCloseMobList(distance)) {
		current_mob = it.second;
		if (!current_mob) {
			continue;
		}

		if (current_mob == center) { // Skip Center
			continue;
		}

		if (current_mob == caster && !affect_caster) { // Skip Caster
			continue;
		}

		if (DistanceSquared(center->GetPosition(), current_mob->GetPosition()) > distance_squared) {    //make sure they are in range
			continue;
		}

		if (current_mob->IsNPC()) {
			Mob* owner = current_mob->GetOwner();
			if (owner) {
				if (!owner->IsOfClientBot()) {
					continue;
				}
			} else {
				continue;
			}
		}

		if (is_detrimental_spell) {
			continue;
		}

		caster->SpellOnTarget(spell_id, current_mob);
	}
}

void EntityList::AEAttack(
	Mob* attacker,
	float distance,
	int16 slot_id,
	int hit_count,
	bool is_from_spell,
	int attack_rounds
)
{
	Mob*  current_mob      = nullptr;
	float distance_squared = distance * distance;
	int   current_hits     = 0;

	for (auto& it: attacker->GetCloseMobList(distance)) {
		current_mob = it.second;
		if (!current_mob) {
			continue;
		}

		if (
			current_mob->IsNPC() &&
			current_mob != attacker &&
			attacker->IsAttackAllowed(current_mob) &&
			!current_mob->IsHorse() &&
			DistanceSquared(current_mob->GetPosition(), attacker->GetPosition()) <= distance_squared
		) {
			for (int i = 0; i < attack_rounds; i++) {
				if (
					!attacker->IsClient() ||
					attacker->GetClass() == Class::Monk ||
					attacker->GetClass() == Class::Ranger
				) {
					attacker->Attack(current_mob, slot_id, false, false, is_from_spell);
				} else {
					attacker->CastToClient()->DoAttackRounds(current_mob, slot_id, is_from_spell);
				}
			}

			current_hits++;
			if (hit_count != 0 && current_hits >= hit_count) {
				return;
			}
		}
	}
}
