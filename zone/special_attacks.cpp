/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemulator.net)

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

#include "common/rulesys.h"
#include "common/strings.h"
#include "zone/bot.h"
#include "zone/client.h"
#include "zone/entity.h"
#include "zone/lua_parser.h"
#include "zone/npc.h"
#include "zone/string_ids.h"

#include <cstring>

extern double frame_time;

int Mob::GetBaseSkillDamage(EQ::skills::SkillType skill, Mob *target)
{
	int base = EQ::skills::GetBaseDamage(skill);
	auto skill_level = GetSkill(skill);
	switch (skill) {
		case EQ::skills::SkillDragonPunch:
		case EQ::skills::SkillEagleStrike:
		case EQ::skills::SkillTigerClaw:
			if (skill_level >= 25) {
				base++;
			}

			if (skill_level >= 75) {
				base++;
			}

			if (skill_level >= 125) {
				base++;
			}

			if (skill_level >= 175) {
				base++;
			}

			if (RuleB(Character, ItemExtraSkillDamageCalcAsPercent) && GetSkillDmgAmt(skill) > 0) {
				base *= std::abs(GetSkillDmgAmt(skill) / 100);
			}

			return base;
		case EQ::skills::SkillFrenzy:
			if (IsClient() && CastToClient()->GetInv().GetItem(EQ::invslot::slotPrimary)) {
				if (GetLevel() > 15) {
					base += GetLevel() - 15;
				}

				if (base > 23) {
					base = 23;
				}

				if (GetLevel() > 50) {
					base += 2;
				}

				if (GetLevel() > 54) {
					base++;
				}

				if (GetLevel() > 59) {
					base++;
				}
			}

			if (RuleB(Character, ItemExtraSkillDamageCalcAsPercent) && GetSkillDmgAmt(skill) > 0) {
				base *= std::abs(GetSkillDmgAmt(skill) / 100);
			}

			return base;
		case EQ::skills::SkillFlyingKick: {
			float skill_bonus = skill_level / 9.0f;
			float ac_bonus    = 0.0f;
			if (IsClient()) {
				auto inst = CastToClient()->GetInv().GetItem(EQ::invslot::slotFeet);
				if (inst) {
					ac_bonus = inst->GetItemArmorClass(true) / 25.0f;
				}
			}

			if (ac_bonus > skill_bonus) {
				ac_bonus = skill_bonus;
			}

			if (RuleB(Character, ItemExtraSkillDamageCalcAsPercent) && GetSkillDmgAmt(skill) > 0) {
				return static_cast<int>(ac_bonus + skill_bonus) * std::abs(GetSkillDmgAmt(skill) / 100);
			}

			return static_cast<int>(ac_bonus + skill_bonus);
		}
		case EQ::skills::SkillKick:
		case EQ::skills::SkillRoundKick: {
			// there is some base *= 4 case in here?
			float skill_bonus = skill_level / 10.0f;
			float ac_bonus    = 0.0f;
			if (IsClient()) {
				auto inst = CastToClient()->GetInv().GetItem(EQ::invslot::slotFeet);
				if (inst) {
					ac_bonus = inst->GetItemArmorClass(true) / 25.0f;
				}
			}

			if (skill_level >= 75) {
				base++;
			}

			if (skill_level >= 175) {
				base++;
			}

			if (RuleB(Character, ItemExtraSkillDamageCalcAsPercent) && GetSkillDmgAmt(skill) > 0) {
				return static_cast<int>(ac_bonus + skill_bonus) * std::abs(GetSkillDmgAmt(skill) / 100);
			}

			return static_cast<int>(ac_bonus + skill_bonus);
		}
		case EQ::skills::SkillBash: {
			float                  skill_bonus = skill_level / 10.0f;
			float                  ac_bonus    = 0.0f;
			const EQ::ItemInstance *inst       = nullptr;
			if (IsClient()) {
				if (HasShieldEquipped()) {
					inst = CastToClient()->GetInv().GetItem(EQ::invslot::slotSecondary);
				} else if (HasTwoHanderEquipped()) {
					if (RuleB(Combat, BashTwoHanderUseShoulderAC)) {
						inst = CastToClient()->GetInv().GetItem(EQ::invslot::slotShoulders);
					} else {
						inst = CastToClient()->GetInv().GetItem(EQ::invslot::slotPrimary);
					}
				}
			}

			if (inst) {
				ac_bonus = inst->GetItemArmorClass(true) / RuleR(Combat, BashACBonusDivisor);
			} else {
				return 0;
			} // return 0 in cases where we don't have an item

			if (ac_bonus > skill_bonus) {
				ac_bonus = skill_bonus;
			}

			if (RuleB(Character, ItemExtraSkillDamageCalcAsPercent) && GetSkillDmgAmt(skill) > 0) {
				return static_cast<int>(ac_bonus + skill_bonus) * std::abs(GetSkillDmgAmt(skill) / 100);
			}

			return static_cast<int>(ac_bonus + skill_bonus);
		}
		case EQ::skills::SkillBackstab: {
			float skill_bonus = static_cast<float>(skill_level) * 0.02f;
			base              = 3; // There seems to be a base 3 for NPCs or some how BS w/o weapon?
			// until we get a better inv system for NPCs they get nerfed!
			if (IsClient()) {
				auto *inst = CastToClient()->GetInv().GetItem(EQ::invslot::slotPrimary);
				if (inst && inst->GetItem() && inst->GetItem()->ItemType == EQ::item::ItemType1HPiercing) {
					base = inst->GetItemBackstabDamage(true);
					if (!inst->GetItemBackstabDamage()) {
						base += inst->GetItemWeaponDamage(true);
					}

					if (target) {
						if (inst->GetItemElementalFlag(true) && inst->GetItemElementalDamage(true) &&
							!RuleB(Combat, BackstabIgnoresElemental)) {
							base += target->ResistElementalWeaponDmg(inst);
						}

						if ((inst->GetItemBaneDamageBody(true) || inst->GetItemBaneDamageRace(true)) &&
							!RuleB(Combat, BackstabIgnoresBane)) {
							base += target->CheckBaneDamage(inst);
						}
					}
				}
			} else if (IsNPC()) {
				auto *npc = CastToNPC();
				base = round((npc->GetMaxDMG() - npc->GetMinDMG()) / RuleR(NPC, NPCBackstabMod));
				// parses show relatively low BS mods from lots of NPCs, so either their BS skill is super low
				// or their mod is divided again, this is probably not the right mod, but it's better
				skill_bonus /= 3.0f;
			}

			if (RuleB(Character, ItemExtraSkillDamageCalcAsPercent) && GetSkillDmgAmt(skill) > 0) {
				return static_cast<int>(static_cast<float>(base) * (skill_bonus + 2.0f)) * std::abs(GetSkillDmgAmt(skill) / 100);
			}

			return static_cast<int>(static_cast<float>(base) * (skill_bonus + 2.0f));
		}
		default: {
			return 0;
		}
	}
}

void Mob::DoSpecialAttackDamage(Mob *who, EQ::skills::SkillType skill, int32 base_damage, int32 min_damage,
				int32 hate_override, int ReuseTime)
{
	// this really should go through the same code as normal melee damage to
	// pick up all the special behavior there

	if ((who == nullptr ||
	     ((IsClient() && CastToClient()->dead) || (who->IsClient() && who->CastToClient()->dead)) || HasDied() ||
	     (!IsAttackAllowed(who))))
		return;

	DamageHitInfo my_hit;
	my_hit.damage_done = 1; // min 1 dmg
	my_hit.base_damage = base_damage;
	my_hit.min_damage = min_damage;
	my_hit.skill = skill;

	if (my_hit.base_damage == 0)
		my_hit.base_damage = GetBaseSkillDamage(my_hit.skill);

	if (base_damage == DMG_INVULNERABLE)
		my_hit.damage_done = DMG_INVULNERABLE;

	if (who->GetInvul() || who->GetSpecialAbility(SpecialAbility::MeleeImmunity))
		my_hit.damage_done = DMG_INVULNERABLE;

	if (who->GetSpecialAbility(SpecialAbility::MeleeImmunityExceptBane) && skill != EQ::skills::SkillBackstab)
		my_hit.damage_done = DMG_INVULNERABLE;

	int64 hate = my_hit.base_damage;
	if (hate_override > -1)
		hate = hate_override;

	if (skill == EQ::skills::SkillBash) {
		if (IsClient()) {
			EQ::ItemInstance *item = CastToClient()->GetInv().GetItem(EQ::invslot::slotSecondary);
			if (item) {
				if (item->GetItem()->ItemType == EQ::item::ItemTypeShield) {
					hate += item->GetItem()->AC;
				}
				const EQ::ItemData *itm = item->GetItem();
				auto fbash = GetSpellFuriousBash(itm->Focus.Effect);
				hate = hate * (100 + fbash) / 100;
				if (fbash)
					MessageString(Chat::FocusEffect, GLOWS_RED, itm->Name);
			}
		}
	}

	my_hit.offense = offense(my_hit.skill);
	my_hit.tohit = GetTotalToHit(my_hit.skill, 0);

	// Rogue Backstab Haste Correction
	// Haste should only provide a max of a 2 s reduction to Backstab cooldown, but it seems that while BackstabReuseTimer can be reduced, there is another timer (repop on the button)
	// that is controlling the actual cooldown.  I'm not sure how this is implemented, but it is impacted by spell haste (including bard v2 and v3), but not worn haste.
	// This code applies an adjustment to backstab accuracy to compensate for this so that Rogue DPS doesn't significantly outclass other classes.

	if (
		RuleB(Combat, RogueBackstabHasteCorrection) &&
		skill == EQ::skills::SkillBackstab &&
		GetHaste() > 100
	) {
		int haste_spell = spellbonuses.haste - spellbonuses.inhibitmelee + spellbonuses.hastetype2 + spellbonuses.hastetype3;
		int haste_worn = itembonuses.haste;

		// Compute Intended Cooldown.  100% Spell = 1 s reduction (max), 40% Worn = 1 s reduction (max).
		int reduction_intended_spell = haste_spell > 100 ? 100 : haste_spell;
		int reduction_intended_worn = 2.5 * (haste_worn > 40 ? 40 : haste_worn);
		int16 intended_cooldown = 1000 - reduction_intended_spell - reduction_intended_worn;

		// Compute Actual Cooldown.  Actual only impacted by spell haste ( + v2 + v3), and is 10 s / (100 + haste)
		int actual_cooldown = 100000 / (100 + haste_spell);

		// Compute Accuracy Adjustment
		int backstab_accuracy_adjust = actual_cooldown * 1000 / intended_cooldown;

		// orig_accuracy = my_hit.tohit;
		int adjusted_accuracy = my_hit.tohit * backstab_accuracy_adjust / 1000;
		my_hit.tohit = adjusted_accuracy;
	}

	my_hit.hand = EQ::invslot::slotPrimary; // Avoid checks hand for throwing/archery exclusion, primary should
						  // work for most
	if (skill == EQ::skills::SkillThrowing || skill == EQ::skills::SkillArchery) {
		my_hit.hand = EQ::invslot::slotRange;
	}

	DoAttack(who, my_hit);

	who->AddToHateList(this, hate, 0);
	who->Damage(this, my_hit.damage_done, SPELL_UNKNOWN, skill, false);

	// Make sure 'this' has not killed the target and 'this' is not dead (Damage shield ect).
	if (!GetTarget()) {
		return;
	}

	if (HasDied()) {
		return;
	}

	TryCastOnSkillUse(who, skill);

	if (HasSkillProcs()) {
		TrySkillProc(who, skill, ReuseTime * 1000);
	}

	if (my_hit.damage_done > 0 && HasSkillProcSuccess()) {
		TrySkillProc(who, skill, ReuseTime * 1000, true);
	}
}

// We should probably refactor this to take the struct not the packet
void Client::OPCombatAbility(const CombatAbility_Struct *ca_atk)
{
	if (!GetTarget()) {
		return;
	}

	// make sure were actually able to use such an attack. (Bards can throw while casting. ~Kayen confirmed on live 1/22)
	if (
		(spellend_timer.Enabled() && GetClass() != Class::Bard) ||
		IsFeared() ||
		IsStunned() ||
		IsMezzed() ||
		DivineAura() ||
		dead
	) {
		return;
	}

	pTimerType timer = pTimerCombatAbility;
	// RoF2+ Tiger Claw is unlinked from other monk skills, if they ever do that for other classes there will need
	// to be more checks here
	if (ClientVersion() >= EQ::versions::ClientVersion::RoF2 && ca_atk->m_skill == EQ::skills::SkillTigerClaw) {
		timer = pTimerCombatAbility2;
	}

	bool bypass_skill_check = false;

	if (ca_atk->m_skill == EQ::skills::SkillBash) { // SLAM - Bash without a shield equipped
		switch (GetRace()) {
			case Race::Ogre:
			case Race::Troll:
			case Race::Barbarian:
				bypass_skill_check = true;
			default:
				break;
		}
	}

	// Check to see if actually have skill
	if (!MaxSkill(static_cast<EQ::skills::SkillType>(ca_atk->m_skill)) && !bypass_skill_check) {
		return;
	}

	if (GetTarget()->GetID() != ca_atk->m_target) { // invalid packet.
		return;
	}

	if (!IsAttackAllowed(GetTarget())) {
		return;
	}

	// These two are not subject to the combat ability timer, as they
	// allready do their checking in conjunction with the attack timer
	// throwing weapons
	if (ca_atk->m_atk == EQ::invslot::slotRange) {
		if (ca_atk->m_skill == EQ::skills::SkillThrowing) {
			SetAttackTimer();
			ThrowingAttack(GetTarget());

			if (CheckDoubleRangedAttack()) {
				ThrowingAttack(GetTarget(), true);
			}

			return;
		}

		// ranged attack (archery)
		if (ca_atk->m_skill == EQ::skills::SkillArchery) {
			SetAttackTimer();
			if (RangedAttack(GetTarget()) && CheckDoubleRangedAttack()) {
				RangedAttack(GetTarget(), true);
			}

			return;
		}
	}

	// check range for all these abilities, they are all close combat stuff
	if (!CombatRange(GetTarget())) {
		return;
	}

	if (!p_timers.Expired(&database, timer, false)) {
		Message(Chat::Red, "Ability recovery time not yet met.");
		return;
	}

	int reuse_time     = 0;
	int haste          = GetHaste();
	int haste_modifier = 0;

	if (haste >= 0) {
		haste_modifier = (10000 / (100 + haste)); //+100% haste = 2x as many attacks
	} else {
		haste_modifier = (100 - haste); //-100% haste = 1/2 as many attacks
	}

	int64 damage          = 0;
	int16 skill_reduction = GetSkillReuseTime(ca_atk->m_skill);

	// not sure what the '100' indicates, if ->m_atk is not used as 'slot' reference, then change SlotRange above back to '11'
	if (
		ca_atk->m_atk == 100 &&
		ca_atk->m_skill == EQ::skills::SkillBash
	) { // SLAM - Bash without a shield equipped
		if (GetTarget() != this) {
			CheckIncreaseSkill(EQ::skills::SkillBash, GetTarget(), 10);
			DoAnim(animTailRake, 0, false);

			int hate_override = 0;

			if (
				GetWeaponDamage(GetTarget(), GetInv().GetItem(EQ::invslot::slotSecondary)) <= 0 &&
				GetWeaponDamage(GetTarget(), GetInv().GetItem(EQ::invslot::slotShoulders)) <= 0
			) {
				damage = -5;
			} else {
				hate_override = damage = GetBaseSkillDamage(EQ::skills::SkillBash, GetTarget());
			}

			reuse_time = BashReuseTime - 1 - skill_reduction;
			reuse_time = (reuse_time * haste_modifier) / 100;
			DoSpecialAttackDamage(GetTarget(), EQ::skills::SkillBash, damage, 0, hate_override, reuse_time);

			if (reuse_time) {
				p_timers.Start(timer, reuse_time);
			}
		}

		return;
	}

	if (ca_atk->m_atk == 100 && ca_atk->m_skill == EQ::skills::SkillFrenzy) {
		int attack_rounds = 1;
		int max_dmg       = GetBaseSkillDamage(EQ::skills::SkillFrenzy, GetTarget());

		CheckIncreaseSkill(EQ::skills::SkillFrenzy, GetTarget(), 10);
		DoAnim(anim1HWeapon, 0, false);

		if (GetClass() == Class::Berserker) {
			int chance = GetLevel() * 2 + GetSkill(EQ::skills::SkillFrenzy);

			if (zone->random.Roll0(450) < chance) {
				attack_rounds++;
			}

			if (zone->random.Roll0(450) < chance) {
				attack_rounds++;
			}
		}

		reuse_time = FrenzyReuseTime - 1 - skill_reduction;
		reuse_time = (reuse_time * haste_modifier) / 100;

		const EQ::ItemInstance* primary_in_use = GetInv().GetItem(EQ::invslot::slotPrimary);
		if (primary_in_use && GetWeaponDamage(GetTarget(), primary_in_use) <= 0) {
			max_dmg = DMG_INVULNERABLE;
		}

		while (attack_rounds > 0) {
			if (GetTarget()) {
				DoSpecialAttackDamage(GetTarget(), EQ::skills::SkillFrenzy, max_dmg, 0, max_dmg, reuse_time);
			}

			attack_rounds--;
		}

		if (reuse_time) {
			p_timers.Start(timer, reuse_time);
		}

		return;
	}

	const uint8 class_id = GetClass();

	// Warrior, Ranger, Monk, Beastlord, and Berserker can kick always
	const uint32 allowed_kick_classes = RuleI(Combat, ExtraAllowedKickClassesBitmask);

	const bool can_use_kick = (
		class_id == Class::Warrior ||
		class_id == Class::Ranger ||
		class_id == Class::Monk ||
		class_id == Class::Beastlord ||
		class_id == Class::Berserker ||
		allowed_kick_classes & GetPlayerClassBit(class_id)
	);

	bool found_skill = false;

	if (
		ca_atk->m_atk == 100 &&
		ca_atk->m_skill == EQ::skills::SkillKick &&
		can_use_kick
	) {
		if (GetTarget() != this) {
			CheckIncreaseSkill(EQ::skills::SkillKick, GetTarget(), 10);
			DoAnim(animKick, 0, false);

			int hate_override = 0;
			if (GetWeaponDamage(GetTarget(), GetInv().GetItem(EQ::invslot::slotFeet)) <= 0) {
				damage = -5;
			} else {
				hate_override = damage = GetBaseSkillDamage(EQ::skills::SkillKick, GetTarget());
			}

			reuse_time = KickReuseTime - 1 - skill_reduction;
			DoSpecialAttackDamage(GetTarget(), EQ::skills::SkillKick, damage, 0, hate_override, reuse_time);

			found_skill = true;
		}
	}

	if (class_id == Class::Monk) {
		reuse_time = MonkSpecialAttack(GetTarget(), ca_atk->m_skill) - 1 - skill_reduction;

		// Live AA - Technique of Master Wu
		int wu_chance = (
			itembonuses.DoubleSpecialAttack +
			spellbonuses.DoubleSpecialAttack +
			aabonuses.DoubleSpecialAttack
		);

		if (wu_chance) {
			const int monk_special_attacks[5] = {
				EQ::skills::SkillFlyingKick,
				EQ::skills::SkillDragonPunch,
				EQ::skills::SkillEagleStrike,
				EQ::skills::SkillTigerClaw,
				EQ::skills::SkillRoundKick
			};

			int extra = 0;
			// always 1/4 of the double attack chance, 25% at rank 5 (100/4)
			while (wu_chance > 0) {
				if (zone->random.Roll(wu_chance)) {
					++extra;
				} else {
					break;
				}

				wu_chance /= 4;
			}

			if (extra) {
				SendColoredText(
					400,
					fmt::format(
						"The spirit of Master Wu fills you! You gain {} additional attack{}.",
						extra,
						extra != 1 ? "s" : ""
					)
				);
			}

			const bool is_classic_master_wu = RuleB(Combat, ClassicMasterWu);
			while (extra) {
				MonkSpecialAttack(
					GetTarget(),
					(is_classic_master_wu ? monk_special_attacks[zone->random.Int(0, 4)] : ca_atk->m_skill)
				);
				--extra;
			}
		}

		if (reuse_time < 100) {
			// hackish... but we return a huge reuse time if this is an
			// invalid skill, otherwise, we can safely assume it is a
			// valid monk skill and just cast it to a SkillType
			CheckIncreaseSkill((EQ::skills::SkillType) ca_atk->m_skill, GetTarget(), 10);
		}

		found_skill = true;
	}

	if (
		ca_atk->m_atk == 100 &&
		ca_atk->m_skill == EQ::skills::SkillBackstab &&
		class_id == Class::Rogue
	) {
		reuse_time = BackstabReuseTime - 1 - skill_reduction;
		TryBackstab(GetTarget(), reuse_time);
		found_skill = true;
	}

	if (!found_skill) {
		reuse_time = 9 - skill_reduction;
	}

	reuse_time = (reuse_time * haste_modifier) / 100;

	reuse_time = EQ::Clamp(reuse_time, 0, reuse_time);

	if (reuse_time) {
		p_timers.Start(timer, reuse_time);
	}
}

//returns the reuse time in sec for the special attack used.
int Mob::MonkSpecialAttack(Mob *other, uint8 unchecked_type)
{
	if (!other)
		return 0;

	int64 ndamage = 0;
	int32 max_dmg = 0;
	int32 min_dmg = 0;
	int reuse = 0;
	EQ::skills::SkillType skill_type; // to avoid casting... even though it "would work"
	uint8 itemslot = EQ::invslot::slotFeet;
	if (IsNPC()) {
		auto *npc = CastToNPC();
		min_dmg = npc->GetMinDamage();
	}

	switch (unchecked_type) {
	case EQ::skills::SkillFlyingKick:
		skill_type = EQ::skills::SkillFlyingKick;
		max_dmg = GetBaseSkillDamage(skill_type);
		min_dmg = 0; // revamped FK formula is missing the min mod?
		DoAnim(animFlyingKick, 0, false);
		reuse = FlyingKickReuseTime;
		break;
	case EQ::skills::SkillDragonPunch:
		skill_type = EQ::skills::SkillDragonPunch;
		max_dmg = GetBaseSkillDamage(skill_type);
		itemslot = EQ::invslot::slotHands;
		DoAnim(animTailRake, 0, false);
		reuse = TailRakeReuseTime;
		break;
	case EQ::skills::SkillEagleStrike:
		skill_type = EQ::skills::SkillEagleStrike;
		max_dmg = GetBaseSkillDamage(skill_type);
		itemslot = EQ::invslot::slotHands;
		DoAnim(animEagleStrike, 0, false);
		reuse = EagleStrikeReuseTime;
		break;
	case EQ::skills::SkillTigerClaw:
		skill_type = EQ::skills::SkillTigerClaw;
		max_dmg = GetBaseSkillDamage(skill_type);
		itemslot = EQ::invslot::slotHands;
		DoAnim(animTigerClaw, 0, false);
		reuse = TigerClawReuseTime;
		break;
	case EQ::skills::SkillRoundKick:
		skill_type = EQ::skills::SkillRoundKick;
		max_dmg = GetBaseSkillDamage(skill_type);
		DoAnim(animRoundKick, 0, false);
		reuse = RoundKickReuseTime;
		break;
	case EQ::skills::SkillKick:
		skill_type = EQ::skills::SkillKick;
		max_dmg = GetBaseSkillDamage(skill_type);
		DoAnim(animKick, 0, false);
		reuse = KickReuseTime;
		break;
	default:
		Log(Logs::Detail, Logs::Attack, "Invalid special attack type %d attempted", unchecked_type);
		return (1000); /* nice long delay for them, the caller depends on this! */
	}

	if (IsClient()) {
		if (GetWeaponDamage(other, CastToClient()->GetInv().GetItem(itemslot)) <= 0) {
			max_dmg = DMG_INVULNERABLE;
		}
	} else {
		if (GetWeaponDamage(other, (const EQ::ItemData *)nullptr) <= 0) {
			max_dmg = DMG_INVULNERABLE;
		}
	}

	int32 ht = 0;
	if (max_dmg > 0) {
		ht = max_dmg;
	}

	// aggro should never be negative else it does massive aggro
	if (ht < 0)	{
		ht = 0;
	}

	DoSpecialAttackDamage(other, skill_type, max_dmg, min_dmg, ht, reuse);

	return reuse;
}

void Mob::TryBackstab(Mob *other, int ReuseTime) {
	if (!other) {
		return;
	}

	bool bIsBehind = false;
	bool bCanFrontalBS = false;

	//make sure we have a proper weapon if we are a client.
	if (IsClient()) {
		const EQ::ItemInstance *wpn = CastToClient()->GetInv().GetItem(EQ::invslot::slotPrimary);

		if (!wpn || (wpn->GetItem()->ItemType != EQ::item::ItemType1HPiercing)){
			MessageString(Chat::Red, BACKSTAB_WEAPON);
			return;
		}
	}
	else if (IsBot()) {
		auto bot = CastToBot();
		auto inst = bot->GetBotItem(EQ::invslot::slotPrimary);
		auto bot_piercer = inst ? inst->GetItem() : nullptr;

		if (!bot_piercer || bot_piercer->ItemType != EQ::item::ItemType1HPiercing) {
			if (!bot->GetCombatRoundForAlerts()) {
				bot->SetCombatRoundForAlerts();
				bot->RaidGroupSay("I can't backstab with this weapon!");
			}
			return;
		}
	}

	//Live AA - Triple Backstab
	int tripleChance = itembonuses.TripleBackstab + spellbonuses.TripleBackstab + aabonuses.TripleBackstab;

	if (BehindMob(other, GetX(), GetY())) {
		bIsBehind = true;
	}
	else {
		//Live AA - Seized Opportunity
		int FrontalBSChance = itembonuses.FrontalBackstabChance + spellbonuses.FrontalBackstabChance + aabonuses.FrontalBackstabChance;

		if (FrontalBSChance && zone->random.Roll(FrontalBSChance)) {
			bCanFrontalBS = true;
		}
	}

	if (bIsBehind || bCanFrontalBS || (IsNPC() && CanFacestab())) { // Player is behind other OR can do Frontal Backstab
		if (bCanFrontalBS && IsClient()) { // I don't think there is any message ...
			CastToClient()->Message(Chat::White, "Your fierce attack is executed with such grace, your target did not see it coming!");
		}

		RogueBackstab(other,false,ReuseTime);

		if (level >= RuleI(Combat, DoubleBackstabLevelRequirement)) {
			// TODO: 55-59 doesn't appear to match just checking double attack, 60+ does though
			if(IsOfClientBot() && CastToClient()->CheckDoubleAttack()) {
				if (other->GetHP() > 0) {
					RogueBackstab(other, false, ReuseTime);
				}

				if (tripleChance && other->GetHP() > 0 && zone->random.Roll(tripleChance)) {
					RogueBackstab(other, false, ReuseTime);
				}
			}
		}

		if (IsClient()) {
			CastToClient()->CheckIncreaseSkill(EQ::skills::SkillBackstab, other, 10);
		}

	}
	//Live AA - Chaotic Backstab
	else if(aabonuses.FrontalBackstabMinDmg || itembonuses.FrontalBackstabMinDmg || spellbonuses.FrontalBackstabMinDmg) {
		m_specialattacks = eSpecialAttacks::ChaoticStab;

		//we can stab from any angle, we do min damage though.
		// chaotic backstab can't double etc Seized can, but that's because it's a chance to do normal BS
		// Live actually added SPA 473 which grants chance to double here when they revamped chaotic/seized

		RogueBackstab(other, true, ReuseTime);

		if (IsClient()) {
			CastToClient()->CheckIncreaseSkill(EQ::skills::SkillBackstab, other, 10);
		}

		m_specialattacks = eSpecialAttacks::None;

		int double_bs_front = aabonuses.Double_Backstab_Front + itembonuses.Double_Backstab_Front + spellbonuses.Double_Backstab_Front;

		if (double_bs_front && other->GetHP() > 0 && zone->random.Roll(double_bs_front)) {
			RogueBackstab(other, false, ReuseTime);
		}
	}
	else { //We do a single regular attack if we attack from the front without chaotic stab
		Attack(other, EQ::invslot::slotPrimary);
	}
}

//heko: backstab
void Mob::RogueBackstab(Mob* other, bool min_damage, int ReuseTime)
{
	if (!other)
		return;

	int64 hate = 0;

	// make sure we can hit (bane, magical, etc)
	if (IsClient()) {
		const EQ::ItemInstance* wpn = CastToClient()->GetInv().GetItem(EQ::invslot::slotPrimary);

		if (!GetWeaponDamage(other, wpn)) {
			return;
		}
	}
	else if (IsBot()) {
		EQ::ItemInstance* botweaponInst = CastToBot()->GetBotItem(EQ::invslot::slotPrimary);

		if (botweaponInst) {
			if (!GetWeaponDamage(other, botweaponInst)) {
				return;
			}
		}
		else if (!GetWeaponDamage(other, (const EQ::ItemData*)nullptr)) {
			return;
		}
	}
	else if (!GetWeaponDamage(other, (const EQ::ItemData*)nullptr)) {
		return;
	}

	int base_damage = GetBaseSkillDamage(EQ::skills::SkillBackstab, other);
	hate = base_damage;

	DoSpecialAttackDamage(other, EQ::skills::SkillBackstab, base_damage, 0, hate, ReuseTime);
	DoAnim(anim1HPiercing, 0, false);
}

// assassinate [No longer used for regular assassinate 6-29-14]
void Mob::RogueAssassinate(Mob* other)
{
	//can you dodge, parry, etc.. an assassinate??
	//if so, use DoSpecialAttackDamage(other, BACKSTAB, 32000); instead
	if (GetWeaponDamage(other, IsClient() ? CastToClient()->GetInv().GetItem(EQ::invslot::slotPrimary) : (const EQ::ItemInstance*)nullptr) > 0){
		other->Damage(this, 32000, SPELL_UNKNOWN, EQ::skills::SkillBackstab);
	}else{
		other->Damage(this, -5, SPELL_UNKNOWN, EQ::skills::SkillBackstab);
	}
	DoAnim(anim1HPiercing, 0, false);	//piercing animation
}

bool Client::RangedAttack(Mob* other, bool CanDoubleAttack) {
	//conditions to use an attack checked before we are called
	if (!other) {
		return false;
	} else if (other == this) {
		return false;
	}

	//make sure the attack and ranged timers are up
	//if the ranged timer is disabled, then they have no ranged weapon and shouldent be attacking anyhow
	if (!CanDoubleAttack && ((attack_timer.Enabled() && !attack_timer.Check(false)) || (ranged_timer.Enabled() && !ranged_timer.Check()))) {
		LogCombat("Throwing attack canceled. Timer not up. Attack [{}], ranged [{}]", attack_timer.GetRemainingTime(), ranged_timer.GetRemainingTime());
		// The server and client timers are not exact matches currently, so this would spam too often if enabled
		//Message(Chat::White, "Error: Timer not up. Attack %d, ranged %d", attack_timer.GetRemainingTime(), ranged_timer.GetRemainingTime());
		return false;
	}
	const EQ::ItemInstance* RangeWeapon = m_inv[EQ::invslot::slotRange];

	//locate ammo
	int ammo_slot = EQ::invslot::slotAmmo;
	const EQ::ItemInstance* Ammo = m_inv[EQ::invslot::slotAmmo];

	if (!RangeWeapon || !RangeWeapon->IsClassCommon()) {
		LogCombat("Ranged attack canceled. Missing or invalid ranged weapon ([{}]) in slot [{}]", GetItemIDAt(EQ::invslot::slotRange), EQ::invslot::slotRange);
		Message(Chat::White, "Error: Rangeweapon: GetItem(%i)==0, you have no bow!", GetItemIDAt(EQ::invslot::slotRange));
		return false;
	}

	if (!Ammo || !Ammo->IsClassCommon()) {
		LogCombat("Ranged attack canceled. Missing or invalid ammo item ([{}]) in slot [{}]", GetItemIDAt(EQ::invslot::slotAmmo), EQ::invslot::slotAmmo);
		Message(Chat::White, "Error: Ammo: GetItem(%i)==0, you have no ammo!", GetItemIDAt(EQ::invslot::slotAmmo));
		return false;
	}

	const EQ::ItemData* RangeItem = RangeWeapon->GetItem();
	const EQ::ItemData* AmmoItem = Ammo->GetItem();

	if (RangeItem->ItemType != EQ::item::ItemTypeBow) {
		LogCombat("Ranged attack canceled. Ranged item is not a bow. type [{}]", RangeItem->ItemType);
		Message(Chat::White, "Error: Rangeweapon: Item %d is not a bow.", RangeWeapon->GetID());
		return false;
	}

	if (AmmoItem->ItemType != EQ::item::ItemTypeArrow) {
		LogCombat("Ranged attack canceled. Ammo item is not an arrow. type [{}]", AmmoItem->ItemType);
		Message(Chat::White, "Error: Ammo: type %d != %d, you have the wrong type of ammo!", AmmoItem->ItemType, EQ::item::ItemTypeArrow);
		return false;
	}

	LogCombat("Shooting [{}] with bow [{}] ([{}]) and arrow [{}] ([{}])", other->GetName(), RangeItem->Name, RangeItem->ID, AmmoItem->Name, AmmoItem->ID);

	//look for ammo in inventory if we only have 1 left...
	if (Ammo->GetCharges() == 1) {
		//first look for quivers
		bool found = false;
		for (int r = EQ::invslot::GENERAL_BEGIN; r <= EQ::invslot::GENERAL_END; r++) {
			const EQ::ItemInstance *pi = m_inv[r];
			if (pi == nullptr || !pi->IsClassBag()) {
				continue;
			}
			const EQ::ItemData* bagitem = pi->GetItem();
			if (!bagitem || bagitem->BagType != EQ::item::BagTypeQuiver) {
				continue;
			}

			//we found a quiver, look for the ammo in it
			for (int i = 0; i < bagitem->BagSlots; i++) {
				const EQ::ItemInstance* baginst = pi->GetItem(i);
				if (!baginst) {
					continue;
				}

				if (baginst->GetID() == Ammo->GetID()) {
					//we found it... use this stack
					//the item wont change, but the instance does
					Ammo = baginst;
					ammo_slot = EQ::InventoryProfile::CalcSlotId(r, i);
					found = true;
					LogCombat("Using ammo from quiver stack at slot [{}]. [{}] in stack", ammo_slot, Ammo->GetCharges());
					break;
				}
			}

			if (found) {
				break;
			}
		}

		if (!found) {
			//if we dont find a quiver, look through our inventory again
			//not caring if the thing is a quiver.
			int32 aslot = m_inv.HasItem(AmmoItem->ID, 1, invWherePersonal);
			if (aslot != INVALID_INDEX) {
				ammo_slot = aslot;
				Ammo = m_inv[aslot];
				LogCombat("Using ammo from inventory stack at slot [{}]. [{}] in stack", ammo_slot, Ammo->GetCharges());
			}
		}
	}

	float range = RangeItem->Range + AmmoItem->Range + GetRangeDistTargetSizeMod(GetTarget());
	LogCombat("Calculated bow range to be [{}]", range);
	range *= range;

	if (float dist = DistanceSquared(m_Position, other->GetPosition()); dist > range) {
		LogCombat("Ranged attack out of range client should catch this. ([{}] > [{}]).\n", dist, range);
		MessageString(Chat::Red,TARGET_OUT_OF_RANGE);//Client enforces range and sends the message, this is a backup just incase.
		return false;
	} else if (dist < (RuleI(Combat, MinRangedAttackDist)*RuleI(Combat, MinRangedAttackDist))){
		MessageString(Chat::Yellow,RANGED_TOO_CLOSE);//Client enforces range and sends the message, this is a backup just incase.
		return false;
	}

	if (!IsAttackAllowed(other) ||
		IsCasting() ||
		IsSitting() ||
		(DivineAura() && !GetGM()) ||
		IsStunned() ||
		IsFeared() ||
		IsMezzed() ||
		(GetAppearance() == eaDead)) {
		return false;
	}

	//Shoots projectile and/or applies the archery damage
	DoArcheryAttackDmg(other, RangeWeapon, Ammo,0,0,0,0,0,0, AmmoItem, ammo_slot);

	//EndlessQuiver AA base1 = 100% Chance to avoid consumption arrow.
	int ChanceAvoidConsume = aabonuses.ConsumeProjectile + itembonuses.ConsumeProjectile + spellbonuses.ConsumeProjectile;

	// Consume Ammo, unless Ammo Consumption is disabled or player has Endless Quiver
	bool consumes_ammo = RuleB(Combat, ArcheryConsumesAmmo);
	if (
		consumes_ammo &&
		(
			RangeItem->ExpendableArrow ||
			!ChanceAvoidConsume ||
			(ChanceAvoidConsume < 100 && zone->random.Int(0,99) > ChanceAvoidConsume)
		)
	) {
		DeleteItemInInventory(ammo_slot, 1, true);
		LogCombat("Consumed Archery Ammo from slot {}.", ammo_slot);
	} else if (!consumes_ammo) {
		LogCombat("Archery Ammo Consumption is disabled.");
	} else {
		LogCombat("Endless Quiver prevented Ammo Consumption.");
	}

	CheckIncreaseSkill(EQ::skills::SkillArchery, GetTarget(), -15);
	CommonBreakInvisibleFromCombat();

	return true;
}

void Mob::DoArcheryAttackDmg(Mob *other, const EQ::ItemInstance *RangeWeapon, const EQ::ItemInstance *Ammo,
							int32 weapon_damage, int16 chance_mod, int16 focus, int ReuseTime, uint32 range_id,
							uint32 ammo_id, const EQ::ItemData *AmmoItem, int AmmoSlot, float speed, bool DisableProcs)
{
	if ((other == nullptr ||
			((IsClient() && CastToClient()->dead) || (other->IsClient() && other->CastToClient()->dead)) ||
			HasDied() || (!IsAttackAllowed(other)) || (other->GetInvul() || other->GetSpecialAbility(SpecialAbility::MeleeImmunity)))) {
		return;
	}

	const EQ::ItemInstance *_RangeWeapon = nullptr;
	const EQ::ItemInstance *_Ammo = nullptr;
	const EQ::ItemData *last_ammo_used = nullptr;

	/*
	If LaunchProjectile is false this function will do archery damage on target,
	otherwise it will shoot the projectile at the target, once the projectile hits target
	this function is then run again to do the damage portion
	*/
	bool LaunchProjectile = false;

	if (RuleB(Combat, ProjectileDmgOnImpact)) {
		if (AmmoItem) { // won't be null when we are firing the arrow
			LaunchProjectile = true;
		} else {
			/*
			Item sync check on projectile landing.
			Weapon damage is already calculated so this only affects procs!
			Ammo proc check will use database to find proc if you used up your last ammo.
			If you change range item mid projectile flight, you loose your chance to proc from bow (Deal
			with it!).
			*/

			if (!RangeWeapon && !Ammo && range_id && ammo_id) {
				if (IsClient()) {
					_RangeWeapon = CastToClient()->m_inv[EQ::invslot::slotRange];
					if (_RangeWeapon && _RangeWeapon->GetItem() &&
					    _RangeWeapon->GetItem()->ID == range_id)
						RangeWeapon = _RangeWeapon;

					_Ammo = CastToClient()->m_inv[AmmoSlot];
					if (_Ammo && _Ammo->GetItem() && _Ammo->GetItem()->ID == ammo_id)
						Ammo = _Ammo;
					else
						last_ammo_used = database.GetItem(ammo_id);
				}
			}
		}
	} else if (AmmoItem) {
		SendItemAnimation(other, AmmoItem, EQ::skills::SkillArchery);
	}

	LogCombat("Ranged attack hit [{}]", other->GetName());

	int64 hate = 0;
	int64 TotalDmg = 0;
	int WDmg = 0;
	int ADmg = 0;
	if (!weapon_damage) {
		WDmg = GetWeaponDamage(other, RangeWeapon);
		ADmg = GetWeaponDamage(other, Ammo);
	} else {
		WDmg = weapon_damage;
	}

	if (LaunchProjectile) { // 1: Shoot the Projectile once we calculate weapon damage.
		TryProjectileAttack(other, AmmoItem, EQ::skills::SkillArchery, (WDmg + ADmg), RangeWeapon,
							Ammo, AmmoSlot, speed, DisableProcs);
		return;
	}

	if (focus) {
		WDmg += WDmg * focus / 100;
	}

	if (WDmg > 0 || ADmg > 0) {
		if (WDmg < 0) {
			WDmg = 0;
		}

		if (ADmg < 0) {
			ADmg = 0;
		}

		int MaxDmg = WDmg + ADmg;
		hate = ((WDmg + ADmg));

		if (RuleB(Combat, ProjectileDmgOnImpact)) {
			LogCombat("Bow and Arrow DMG [{}], Max Damage [{}]", WDmg, MaxDmg);
		}
		else {
			LogCombat("Bow DMG [{}], Arrow DMG [{}], Max Damage [{}]", WDmg, ADmg, MaxDmg);
		}

		if (MaxDmg == 0) {
			MaxDmg = 1;
		}

		DamageHitInfo my_hit {};
		my_hit.base_damage = MaxDmg;
		my_hit.min_damage = 0;
		my_hit.damage_done = 1;

		my_hit.skill = EQ::skills::SkillArchery;
		my_hit.offense = offense(my_hit.skill);
		my_hit.tohit = GetTotalToHit(my_hit.skill, chance_mod);
		my_hit.hand = EQ::invslot::slotRange;

		DoAttack(other, my_hit);
		TotalDmg = my_hit.damage_done;
	} else {
		TotalDmg = DMG_INVULNERABLE;
	}

	if (IsClient() && !CastToClient()->GetFeigned()) {
		other->AddToHateList(this, hate, 0);
	}

	other->Damage(this, TotalDmg, SPELL_UNKNOWN, EQ::skills::SkillArchery);


	if (!DisableProcs) {
		// Weapon Proc
		if (RangeWeapon && other && !other->HasDied()) {
			TryCombatProcs(RangeWeapon, other, EQ::invslot::slotRange);
		}

		// Ammo Proc, do not try spell procs if from ammo.
		if (last_ammo_used) {
			TryWeaponProc(nullptr, last_ammo_used, other, EQ::invslot::slotRange);
		}
		else if (Ammo && other && !other->HasDied()) {
			TryWeaponProc(Ammo, Ammo->GetItem(), other, EQ::invslot::slotRange);
		}
	}

	TryCastOnSkillUse(other, EQ::skills::SkillArchery);

	if (!DisableProcs) {
		// Skill Proc Attempt
		if (HasSkillProcs() && other && !other->HasDied()) {
			if (ReuseTime) {
				TrySkillProc(other, EQ::skills::SkillArchery, ReuseTime);
			}
			else {
				TrySkillProc(other, EQ::skills::SkillArchery, 0, false, EQ::invslot::slotRange);
			}
		}

		// Skill Proc Success ... can proc off hits OR misses
		if (HasSkillProcSuccess() && other && !other->HasDied()) {
			if (ReuseTime) {
				TrySkillProc(other, EQ::skills::SkillArchery, ReuseTime, true);
			}
			else {
				TrySkillProc(other, EQ::skills::SkillArchery, 0, true, EQ::invslot::slotRange);
			}
		}
	}
}

bool Mob::TryProjectileAttack(Mob *other, const EQ::ItemData *item, EQ::skills::SkillType skillInUse,
			      uint64 weapon_dmg, const EQ::ItemInstance *RangeWeapon,
			      const EQ::ItemInstance *Ammo, int AmmoSlot, float speed, bool DisableProcs)
{
	if (!other)
		return false;

	int slot = -1;

	// Make sure there is an avialable slot.
	for (int i = 0; i < MAX_SPELL_PROJECTILE; i++) {
		if (ProjectileAtk[i].target_id == 0) {
			slot = i;
			break;
		}
	}

	if (slot < 0)
		return false;

	float distance_mod = 0.0f;
	float distance = other->CalculateDistance(GetX(), GetY(), GetZ());

	/*
	New Distance Mod constant (7/25/21 update), modifier is needed to adjust slower speeds to have correct impact times at short distances.
	We use archery 4.0 speed as a baseline for the forumla.  At speed 1.5 at 50 pct distance mod is needed, where as speed 4.0 there is no modifer.
	Therefore, we derive out our modifer as follows. distance_mod = (speed - 4) * ((50 - 0)/(1.5-4)). The ratio there is -20.0f. distance_mod = (speed - 4) * -20.0f
	For distances >125 we use different modifier, this was all meticulously tested by eye to get the best possible outcome for projectile impact times. Not perfect though.
	*/

	if (distance <= 125.0f) {
		if (speed != 4.0f) { //Standard functions will always be 4.0f for archery.
			distance_mod = (speed - 4.0f) * -20.0f;
			distance += distance * distance_mod / 100.0f;
		}
	}
	else if (distance > 125.0f && distance <= 200.0f)
		distance = 3.14f * (distance / 2.0f); //Get distance of arc to better reflect projectile path length

	else if (distance > 200.0f) {
		distance = distance * 1.30f; //Add 30% to base distance if over 200 range to tighten up hit timing.
		distance = 3.14f * (distance / 2.0f); //Get distance of arc to better reflect projectile path length
	}

	float hit = 1200.0f + (10 * distance / speed);

	ProjectileAtk[slot].increment = 1;
	ProjectileAtk[slot].hit_increment = static_cast<uint16>(hit); // This projected hit time if target does NOT MOVE
	ProjectileAtk[slot].target_id = other->GetID();
	ProjectileAtk[slot].wpn_dmg = weapon_dmg;
	ProjectileAtk[slot].origin_x = GetX();
	ProjectileAtk[slot].origin_y = GetY();
	ProjectileAtk[slot].origin_z = GetZ();

	if (RangeWeapon && RangeWeapon->GetItem())
		ProjectileAtk[slot].ranged_id = RangeWeapon->GetItem()->ID;

	if (Ammo && Ammo->GetItem())
		ProjectileAtk[slot].ammo_id = Ammo->GetItem()->ID;

	ProjectileAtk[slot].ammo_slot = AmmoSlot;
	ProjectileAtk[slot].skill = skillInUse;
	ProjectileAtk[slot].speed_mod = speed;
	ProjectileAtk[slot].disable_procs = DisableProcs;

	SetProjectileAttack(true);

	if (item)
		SendItemAnimation(other, item, skillInUse, speed);

	return true;
}

void Mob::ProjectileAttack()
{
	if (!HasProjectileAttack())
		return;

	Mob *target = nullptr;
	bool disable = true;

	for (int i = 0; i < MAX_SPELL_PROJECTILE; i++) {
		if (ProjectileAtk[i].increment == 0)
			continue;

		disable = false;
		Mob *target = entity_list.GetMobID(ProjectileAtk[i].target_id);

		if (target && target->IsMoving()) {
			/*
			Only recalculate hit increment if target is moving.
			Due to frequency that we need to check increment the targets position variables may not be
			updated even if moving. Do a simple check before calculating distance.
			*/
			if (ProjectileAtk[i].tlast_x != target->GetX() || ProjectileAtk[i].tlast_y != target->GetY()) {

				ProjectileAtk[i].tlast_x = target->GetX();
				ProjectileAtk[i].tlast_y = target->GetY();

				//Recalculate from the original location the projectile was fired in relation to the current targets location.
				float distance = target->CalculateDistance(ProjectileAtk[i].origin_x, ProjectileAtk[i].origin_y, ProjectileAtk[i].origin_z);
				float distance_mod = 0.0f;

				if (distance <= 125.0f) {
					distance_mod = (ProjectileAtk[i].speed_mod - 4.0f) * -20.0f;
					distance += distance * distance_mod / 100.0f;
				}
				else if (distance > 125.0f && distance <= 200.0f)
					distance = 3.14f * (distance / 2.0f); //Get distance of arc to better reflect projectile path length

				else if (distance > 200.0f) {
					distance = distance * 1.30f; //Add 30% to base distance if over 200 range to tighten up hit timing.
					distance = 3.14f * (distance / 2.0f); //Get distance of arc to better reflect projectile path length
				}

				float hit = 1200.0f + (10 * distance / ProjectileAtk[i].speed_mod);

				ProjectileAtk[i].hit_increment = static_cast<uint16>(hit);
			}
		}

		// Check if we hit.
		if (ProjectileAtk[i].hit_increment <= ProjectileAtk[i].increment) {
			if (target) {
				if (IsNPC()) {
					if (ProjectileAtk[i].skill == EQ::skills::SkillConjuration) {
						if (IsValidSpell(ProjectileAtk[i].wpn_dmg))
							SpellOnTarget(ProjectileAtk[i].wpn_dmg, target, false, true,
								      spells[ProjectileAtk[i].wpn_dmg].resist_difficulty,
								      true);
					} else {
						CastToNPC()->DoRangedAttackDmg(
						    target, false, ProjectileAtk[i].wpn_dmg, 0,
						    static_cast<EQ::skills::SkillType>(ProjectileAtk[i].skill));
					}
				} else {
					if (ProjectileAtk[i].skill == EQ::skills::SkillArchery)
						DoArcheryAttackDmg(target, nullptr, nullptr, ProjectileAtk[i].wpn_dmg,
								   0, 0, 0, ProjectileAtk[i].ranged_id,
								   ProjectileAtk[i].ammo_id, nullptr,
								   ProjectileAtk[i].ammo_slot, 4.0f, ProjectileAtk[i].disable_procs);
					else if (ProjectileAtk[i].skill == EQ::skills::SkillThrowing)
						DoThrowingAttackDmg(target, nullptr, nullptr, ProjectileAtk[i].wpn_dmg,
								    0, 0, 0, ProjectileAtk[i].ranged_id,
								    ProjectileAtk[i].ammo_slot, 4.0f, ProjectileAtk[i].disable_procs);
					else if (ProjectileAtk[i].skill == EQ::skills::SkillConjuration &&
						 IsValidSpell(ProjectileAtk[i].wpn_dmg))
						SpellOnTarget(ProjectileAtk[i].wpn_dmg, target, false, true,
							      spells[ProjectileAtk[i].wpn_dmg].resist_difficulty, true);
				}
			}

			ProjectileAtk[i].increment = 0;
			ProjectileAtk[i].target_id = 0;
			ProjectileAtk[i].wpn_dmg = 0;
			ProjectileAtk[i].origin_x = 0.0f;
			ProjectileAtk[i].origin_y = 0.0f;
			ProjectileAtk[i].origin_z = 0.0f;
			ProjectileAtk[i].tlast_x = 0.0f;
			ProjectileAtk[i].tlast_y = 0.0f;
			ProjectileAtk[i].ranged_id = 0;
			ProjectileAtk[i].ammo_id = 0;
			ProjectileAtk[i].ammo_slot = 0;
			ProjectileAtk[i].skill = 0;
			ProjectileAtk[i].speed_mod = 0.0f;
		} else {
			ProjectileAtk[i].increment += 1000 * frame_time;
		}
	}

	if (disable)
		SetProjectileAttack(false);
}

float Mob::GetRangeDistTargetSizeMod(Mob* other)
{
	/*
	Range is enforced client side, therefore these numbers do not need to be 100% accurate just close enough to
	prevent any exploitation. The range mod changes in some situations depending on if size is from spawn or from SendIllusionPacket changes.
	At present time only calculate from spawn (it is no consistent what happens to the calc when changing it after spawn).
	*/
	if (!other)
		return 0.0f;

	float tsize = other->GetSize();

	if (GetSize() > tsize)
		tsize = GetSize();

	float mod = 0.0f;
	/*These are correct numbers if mob size is changed via #size (Don't know why it matters but it does)
	if (tsize < 7)
		mod = 16.0f;
	else if (tsize >=7 && tsize <= 20)
		mod = 16.0f + (0.6f * (tsize - 6.0f));
	else if (tsize >=20 && tsize <= 60)
		mod = 25.0f + (1.25f * (tsize - 20.0f));
	else
		mod = 75.0f;
	*/

	if (tsize < 10)
		mod = 18.0f;
	else if (tsize >=10 && tsize < 15)
		mod = 20.0f + (4.0f * (tsize - 10.0f));
	else if (tsize >=15 && tsize <= 20)
		mod = 42.0f + (5.8f * (tsize - 15.0f));
	else
		mod = 75.0f;

	return (mod + 2.0f); //Add 2.0f as buffer to prevent any chance of failures, client enforce range check regardless.
}

void NPC::RangedAttack(Mob *other)
{
	if (!other)
		return;
	// make sure the attack and ranged timers are up
	// if the ranged timer is disabled, then they have no ranged weapon and shouldent be attacking anyhow
	if ((attack_timer.Enabled() && !attack_timer.Check(false)) ||
	    (ranged_timer.Enabled() && !ranged_timer.Check())) {
		LogCombat("Archery canceled. Timer not up. Attack [{}], ranged [{}]", attack_timer.GetRemainingTime(),
			  ranged_timer.GetRemainingTime());
		return;
	}

	if (!HasBowAndArrowEquipped() && !GetSpecialAbility(SpecialAbility::RangedAttack))
		return;

	if (!CheckLosFN(other))
		return;

	int attacks = 1;
	float min_range = static_cast<float>(RuleI(Combat, MinRangedAttackDist));
	float max_range = 250.0f; // needs to be longer than 200(most spells)

	if (GetSpecialAbility(SpecialAbility::RangedAttack)) {
		int temp_attacks = GetSpecialAbilityParam(SpecialAbility::RangedAttack, 0);
		attacks = temp_attacks > 0 ? temp_attacks : 1;

		int temp_min_range = GetSpecialAbilityParam(SpecialAbility::RangedAttack, 4); // Min Range of NPC attack
		int temp_max_range = GetSpecialAbilityParam(SpecialAbility::RangedAttack, 1); // Max Range of NPC attack
		if (temp_max_range)
			max_range = static_cast<float>(temp_max_range);
		if (temp_min_range)
			min_range = static_cast<float>(temp_min_range);
	}

	max_range *= max_range;
	min_range *= min_range;

	for (int i = 0; i < attacks; ++i) {
		if (DistanceSquared(m_Position, other->GetPosition()) > max_range)
			return;
		else if (DistanceSquared(m_Position, other->GetPosition()) < min_range)
			return;

		if (!other || !IsAttackAllowed(other) || IsCasting() || DivineAura() || IsStunned() || IsFeared() ||
		    IsMezzed() || (GetAppearance() == eaDead)) {
			return;
		}

		FaceTarget(other);

		DoRangedAttackDmg(other);

		CommonBreakInvisibleFromCombat();
	}
}

void NPC::DoRangedAttackDmg(Mob* other, bool Launch, int16 damage_mod, int16 chance_mod, EQ::skills::SkillType skill, float speed, const char *IDFile)
{
	if ((other == nullptr ||
		(other->HasDied())) ||
		HasDied() ||
		(!IsAttackAllowed(other)) ||
		(other->GetInvul() ||
		other->GetSpecialAbility(SpecialAbility::MeleeImmunity)))
	{
		return;
	}

	EQ::skills::SkillType skillInUse = static_cast<EQ::skills::SkillType>(GetRangedSkill());

	if (skill != skillInUse)
		skillInUse = skill;

	if (Launch)
	{
		const char *ammo = "IT10";

		if (IDFile != nullptr)
			ammo = IDFile;
		else if (GetAmmoIDfile())
			ammo = GetAmmoIDfile();

		ProjectileAnimation(other, 0,false,speed,0,0,0,ammo,skillInUse);

		if (RuleB(Combat, ProjectileDmgOnImpact))
		{
			TryProjectileAttack(other, nullptr, skillInUse, damage_mod, nullptr, nullptr, 0, speed);
			return;
		}
	}

	if (!chance_mod)
		chance_mod = GetSpecialAbilityParam(SpecialAbility::RangedAttack, 2);

	int TotalDmg = 0;
	int MaxDmg = GetBaseDamage() * RuleR(Combat, ArcheryNPCMultiplier); // should add a field to npc_types
	int MinDmg = GetMinDamage() * RuleR(Combat, ArcheryNPCMultiplier);

	if (!damage_mod)
		 damage_mod = GetSpecialAbilityParam(SpecialAbility::RangedAttack, 3);//Damage modifier

	DamageHitInfo my_hit;
	my_hit.base_damage = MaxDmg;
	my_hit.min_damage = MinDmg;
	my_hit.damage_done = 1;

	my_hit.skill = skill;
	my_hit.offense = offense(my_hit.skill);
	my_hit.tohit = GetTotalToHit(my_hit.skill, chance_mod);
	my_hit.hand = EQ::invslot::slotRange;

	DoAttack(other, my_hit);

	TotalDmg = my_hit.damage_done;

	if (TotalDmg > 0) {
		TotalDmg += TotalDmg * damage_mod / 100;
		other->AddToHateList(this, TotalDmg, 0);
	} else {
		other->AddToHateList(this, 0, 0);
	}

	other->Damage(this, TotalDmg, SPELL_UNKNOWN, skillInUse);

	//try proc on hits and misses
	if (other && !other->HasDied()) {
		TrySpellProc(nullptr, (const EQ::ItemData*)nullptr, other, EQ::invslot::slotRange);
	}

	TryCastOnSkillUse(other, skillInUse);

	if (HasSkillProcs() && other && !other->HasDied()) {
		TrySkillProc(other, skillInUse, 0, false, EQ::invslot::slotRange);
	}

	if (HasSkillProcSuccess() && other && !other->HasDied()) {
		TrySkillProc(other, skillInUse, 0, true, EQ::invslot::slotRange);
	}
}

void Client::ThrowingAttack(Mob* other, bool CanDoubleAttack) { //old was 51
	//conditions to use an attack checked before we are called
	if (!other)
		return;
	//make sure the attack and ranged timers are up
	//if the ranged timer is disabled, then they have no ranged weapon and shouldent be attacking anyhow
	if((!CanDoubleAttack && (attack_timer.Enabled() && !attack_timer.Check(false)) || (ranged_timer.Enabled() && !ranged_timer.Check()))) {
		LogCombat("Throwing attack canceled. Timer not up. Attack [{}], ranged [{}]", attack_timer.GetRemainingTime(), ranged_timer.GetRemainingTime());
		// The server and client timers are not exact matches currently, so this would spam too often if enabled
		//Message(0, "Error: Timer not up. Attack %d, ranged %d", attack_timer.GetRemainingTime(), ranged_timer.GetRemainingTime());
		return;
	}

	int ammo_slot = EQ::invslot::slotRange;
	const EQ::ItemInstance* RangeWeapon = m_inv[EQ::invslot::slotRange];

	if (!RangeWeapon || !RangeWeapon->IsClassCommon()) {
		LogCombat("Ranged attack canceled. Missing or invalid ranged weapon ([{}]) in slot [{}]", GetItemIDAt(EQ::invslot::slotRange), EQ::invslot::slotRange);
		Message(0, "Error: Rangeweapon: GetItem(%i)==0, you have nothing to throw!", GetItemIDAt(EQ::invslot::slotRange));
		return;
	}

	const EQ::ItemData* item = RangeWeapon->GetItem();
	if (item->ItemType != EQ::item::ItemTypeLargeThrowing && item->ItemType != EQ::item::ItemTypeSmallThrowing) {
		LogCombat("Ranged attack canceled. Ranged item [{}] is not a throwing weapon. type [{}]", item->ID, item->ItemType);
		Message(0, "Error: Rangeweapon: GetItem(%i)==0, you have nothing useful to throw!", GetItemIDAt(EQ::invslot::slotRange));
		return;
	}

	LogCombat("Throwing [{}] ([{}]) at [{}]", item->Name, item->ID, other->GetName());

	if(RangeWeapon->GetCharges() == 1) {
		//first check ammo
		const EQ::ItemInstance* AmmoItem = m_inv[EQ::invslot::slotAmmo];
		if(AmmoItem != nullptr && AmmoItem->GetID() == RangeWeapon->GetID()) {
			//more in the ammo slot, use it
			RangeWeapon = AmmoItem;
			ammo_slot = EQ::invslot::slotAmmo;
			LogCombat("Using ammo from ammo slot, stack at slot [{}]. [{}] in stack", ammo_slot, RangeWeapon->GetCharges());
		} else {
			//look through our inventory for more
			int32 aslot = m_inv.HasItem(item->ID, 1, invWherePersonal);
			if (aslot != INVALID_INDEX) {
				//the item wont change, but the instance does, not that it matters
				ammo_slot = aslot;
				RangeWeapon = m_inv[aslot];
				LogCombat("Using ammo from inventory slot, stack at slot [{}]. [{}] in stack", ammo_slot, RangeWeapon->GetCharges());
			}
		}
	}

	float range = item->Range + GetRangeDistTargetSizeMod(other);
	LogCombat("Calculated bow range to be [{}]", range);
	range *= range;
	float dist = DistanceSquared(m_Position, other->GetPosition());
	if(dist > range) {
		LogCombat("Throwing attack out of range client should catch this. ([{}] > [{}]).\n", dist, range);
		MessageString(Chat::Red,TARGET_OUT_OF_RANGE);//Client enforces range and sends the message, this is a backup just incase.
		return;
	}
	else if(dist < (RuleI(Combat, MinRangedAttackDist)*RuleI(Combat, MinRangedAttackDist))){
		MessageString(Chat::Yellow,RANGED_TOO_CLOSE);//Client enforces range and sends the message, this is a backup just incase.
		return;
	}

	if(!IsAttackAllowed(other) ||
		(IsCasting() && GetClass() != Class::Bard) ||
		IsSitting() ||
		(DivineAura() && !GetGM()) ||
		IsStunned() ||
		IsFeared() ||
		IsMezzed() ||
		(GetAppearance() == eaDead)){
		return;
	}

	DoThrowingAttackDmg(other, RangeWeapon, item, 0, 0, 0, 0, 0,ammo_slot);

	// Consume Ammo, unless Ammo Consumption is disabled
	if (RuleB(Combat, ThrowingConsumesAmmo)) {
		DeleteItemInInventory(ammo_slot, 1, true);
		LogCombat("Consumed Throwing Ammo from slot {}.", ammo_slot);
	} else {
		LogCombat("Throwing Ammo Consumption is disabled.");
	}

	CommonBreakInvisibleFromCombat();
}

void Mob::DoThrowingAttackDmg(Mob *other, const EQ::ItemInstance *RangeWeapon, const EQ::ItemData *AmmoItem,
	int32 weapon_damage, int16 chance_mod, int16 focus, int ReuseTime, uint32 range_id,
	int AmmoSlot, float speed, bool DisableProcs)
{
	if ((other == nullptr ||
		((IsClient() && CastToClient()->dead) || (other->IsClient() && other->CastToClient()->dead)) ||
		HasDied() || (!IsAttackAllowed(other)) || (other->GetInvul() || other->GetSpecialAbility(SpecialAbility::MeleeImmunity)))) {
		return;
	}

	const EQ::ItemInstance *m_RangeWeapon = nullptr;//throwing weapon
	const EQ::ItemData *last_ammo_used = nullptr;

	/*
	If LaunchProjectile is false this function will do archery damage on target,
	otherwise it will shoot the projectile at the target, once the projectile hits target
	this function is then run again to do the damage portion
	*/
	bool LaunchProjectile = false;

	if (RuleB(Combat, ProjectileDmgOnImpact)) {
		if (AmmoItem) {
			LaunchProjectile = true;
		}
		else {
			if (!RangeWeapon && range_id) {
				if (IsClient()) {
					m_RangeWeapon = CastToClient()->m_inv[AmmoSlot];

					if (m_RangeWeapon && m_RangeWeapon->GetItem() && m_RangeWeapon->GetItem()->ID == range_id) {
						RangeWeapon = m_RangeWeapon;
					}
					else {
						last_ammo_used = database.GetItem(range_id);
					}
				}
			}
		}
	}
	else if (AmmoItem) {
		SendItemAnimation(other, AmmoItem, EQ::skills::SkillThrowing);
	}

	LogCombat("Throwing attack hit [{}]", other->GetName());

	int WDmg = 0;

	if (!weapon_damage) {
		if (IsOfClientBot() && RangeWeapon) {
			WDmg = GetWeaponDamage(other, RangeWeapon);
		}
		else if (AmmoItem) {
			WDmg = GetWeaponDamage(other, AmmoItem);
		}

		if (LaunchProjectile) {
			TryProjectileAttack(other, AmmoItem, EQ::skills::SkillThrowing, WDmg, RangeWeapon,
				nullptr, AmmoSlot, speed);
			return;
		}
	}
	else {
		WDmg = weapon_damage;
	}

	if (focus) { // no longer used, keep for quests
		WDmg += WDmg * focus / 100;
	}

	int TotalDmg = 0;

	if (WDmg > 0) {
		DamageHitInfo my_hit {};
		my_hit.base_damage = WDmg;
		my_hit.min_damage = 0;
		my_hit.damage_done = 1;

		my_hit.skill = EQ::skills::SkillThrowing;
		my_hit.offense = offense(my_hit.skill);
		my_hit.tohit = GetTotalToHit(my_hit.skill, chance_mod);
		my_hit.hand = EQ::invslot::slotRange;

		DoAttack(other, my_hit);
		TotalDmg = my_hit.damage_done;

		LogCombat("Item DMG [{}]. Hit for damage [{}]", WDmg, TotalDmg);
	}
	else {
		TotalDmg = DMG_INVULNERABLE;
	}

	if (IsClient() && !CastToClient()->GetFeigned()) {
		other->AddToHateList(this, WDmg, 0);
	}

	other->Damage(this, TotalDmg, SPELL_UNKNOWN, EQ::skills::SkillThrowing);

	if (!DisableProcs && other && !other->HasDied()) {
		TryCombatProcs(RangeWeapon, other, EQ::invslot::slotRange, last_ammo_used);
	}

	TryCastOnSkillUse(other, EQ::skills::SkillThrowing);

	if (!DisableProcs) {
		if (HasSkillProcs() && other && !other->HasDied()) {
			if (ReuseTime) {
				TrySkillProc(other, EQ::skills::SkillThrowing, ReuseTime);
			}
			else {
				TrySkillProc(other, EQ::skills::SkillThrowing, 0, false, EQ::invslot::slotRange);
			}
		}

		if (HasSkillProcSuccess() && other && !other->HasDied()) {
			if (ReuseTime) {
				TrySkillProc(other, EQ::skills::SkillThrowing, ReuseTime, true);
			}
			else {
				TrySkillProc(other, EQ::skills::SkillThrowing, 0, true, EQ::invslot::slotRange);
			}
		}
	}

	if (IsClient()) {
		CastToClient()->CheckIncreaseSkill(EQ::skills::SkillThrowing, GetTarget());
	}
}

void Mob::SendItemAnimation(Mob *to, const EQ::ItemData *item, EQ::skills::SkillType skillInUse, float velocity) {
	auto outapp = new EQApplicationPacket(OP_SomeItemPacketMaybe, sizeof(Arrow_Struct));
	Arrow_Struct *as = (Arrow_Struct *) outapp->pBuffer;
	as->type = 1;
	as->src_x = GetX();
	as->src_y = GetY();
	as->src_z = GetZ();
	as->source_id = GetID();
	as->target_id = to->GetID();
	as->item_id = item->ID;

	as->item_type = item->ItemType;
	as->skill = (uint8)skillInUse;

	strn0cpy(as->model_name, item->IDFile, 16);


	/*
		The angular field affects how the object flies towards the target.
		A low angular (10) makes it circle the target widely, where a high
		angular (20000) makes it go straight at them.

		The tilt field causes the object to be tilted flying through the air
		and also seems to have an effect on how it behaves when circling the
		target based on the angular field.

		Arc causes the object to form an arc in motion. A value too high will
	*/
	as->velocity = velocity;

	//these angle and tilt used together seem to make the arrow/knife throw as straight as I can make it

	as->launch_angle = CalculateHeadingToTarget(to->GetX(), to->GetY());
	as->tilt = 125;
	as->arc = 50;


	//fill in some unknowns, we dont know their meaning yet
	//neither of these seem to change the behavior any
	as->unknown088 = 125;
	as->unknown092 = 16;

	entity_list.QueueCloseClients(this, outapp);
	safe_delete(outapp);
}

void Mob::ProjectileAnimation(Mob* to, int item_id, bool IsArrow, float speed, float angle, float tilt, float arc, const char *IDFile, EQ::skills::SkillType skillInUse) {
	if (!to)
		return;

	const EQ::ItemData* item = nullptr;
	uint8 item_type = 0;

	if(!item_id) {
		item = database.GetItem(8005); // Arrow will be default
	}
	else {
		item = database.GetItem(item_id); // Use the item input into the command
	}

	if(!item) {
		return;
	}
	if(IsArrow) {
		item_type = 27;
	}
	if(!item_type && !skillInUse) {
		item_type = item->ItemType;
	}
	else if (skillInUse)
		item_type = GetItemTypeBySkill(skillInUse);

	if(!speed) {
		speed = 4.0;
	}
	if(!angle) {
		angle = CalculateHeadingToTarget(to->GetX(), to->GetY());
	}
	if(!tilt) {
		tilt = 125;
	}
	if(!arc) {
		arc = 50;
	}

	const char *item_IDFile = item->IDFile;

	if (IDFile && (strncmp(IDFile, "IT", 2) == 0))
		item_IDFile = IDFile;

	// See SendItemAnimation() for some notes on this struct
	auto outapp = new EQApplicationPacket(OP_SomeItemPacketMaybe, sizeof(Arrow_Struct));
	Arrow_Struct *as = (Arrow_Struct *) outapp->pBuffer;
	as->type = 1;
	as->src_x = GetX();
	as->src_y = GetY();
	as->src_z = GetZ();
	as->source_id = GetID();
	as->target_id = to->GetID();
	as->item_id = item->ID;
	as->item_type = item_type;
	as->skill = skillInUse;	// Doesn't seem to have any effect
	strn0cpy(as->model_name, item_IDFile, 16);
	as->velocity = speed;
	as->launch_angle = angle;
	as->tilt = tilt;
	as->arc = arc;
	as->unknown088 = 125;
	as->unknown092 = 16;

	entity_list.QueueCloseClients(this, outapp);
	safe_delete(outapp);

}

void NPC::DoClassAttacks(Mob *target) {
	if(target == nullptr)
		return;	//gotta have a target for all these

	bool taunt_time = taunt_timer.Check();
	bool ca_time = classattack_timer.Check(false);
	bool ka_time = knightattack_timer.Check(false);

	const EQ::ItemData* boots = database.GetItem(equipment[EQ::invslot::slotFeet]);

	//only check attack allowed if we are going to do something
	if((taunt_time || ca_time || ka_time) && !IsAttackAllowed(target))
		return;

	if(ka_time){
		int knightreuse = 1000; //lets give it a small cooldown actually.

		switch(GetClass()){
			case Class::ShadowKnight: case Class::ShadowKnightGM:{
				if (CastSpell(SPELL_NPC_HARM_TOUCH, target->GetID())) {
					knightreuse = HarmTouchReuseTime * 1000;
					}
				break;
			}
			case Class::Paladin: case Class::PaladinGM:{
				if(GetHPRatio() < 20) {
					if (CastSpell(SPELL_LAY_ON_HANDS, GetID())) {
						knightreuse = LayOnHandsReuseTime * 1000;
					}
				} else {
					knightreuse = 2000; //Check again in two seconds.
				}
				break;
			}
		}
		knightattack_timer.Start(knightreuse);
	}

	//general stuff, for all classes....
	//only gets used when their primary ability get used too
	if (
		IsTaunting() &&
		HasOwner() &&
		target->IsNPC() &&
		target->GetBodyType() != BodyType::Undead &&
		taunt_time &&
		type_of_pet &&
		type_of_pet != PetType::TargetLock &&
		DistanceSquared(GetPosition(), target->GetPosition()) <= (RuleI(Pets, PetTauntRange) * RuleI(Pets, PetTauntRange))
	) {
		GetOwner()->MessageString(Chat::PetResponse, PET_TAUNTING);
		Taunt(target->CastToNPC(), false);
	}

	if(!ca_time) {
		return;
	}

	float HasteModifier = GetHaste() * 0.01f;

	int level = GetLevel();
	int reuse = TauntReuseTime * 1000;	//make this very long since if they dont use it once, they prolly never will
	bool did_attack = false;
	//class specific stuff...
	switch(GetClass()) {
		case Class::Rogue: case Class::RogueGM:
			if(level >= 10) {
				reuse = BackstabReuseTime * 1000;
				TryBackstab(target, reuse);
				did_attack = true;
			}
			break;
		case Class::Monk: case Class::MonkGM: {
			uint8 satype = EQ::skills::SkillKick;
			if (level > 29) { satype = EQ::skills::SkillFlyingKick; }
			else if (level > 24) { satype = EQ::skills::SkillDragonPunch; }
			else if (level > 19) { satype = EQ::skills::SkillEagleStrike; }
			else if (level > 9) { satype = EQ::skills::SkillTigerClaw; }
			else if (level > 4) { satype = EQ::skills::SkillRoundKick; }
			reuse = MonkSpecialAttack(target, satype);

			reuse *= 1000;
			did_attack = true;
			break;
		}
		case Class::Warrior: case Class::WarriorGM:{
			if(level >= RuleI(Combat, NPCBashKickLevel)){
				if(zone->random.Roll(75)) { //tested on live, warrior mobs both kick and bash, kick about 75% of the time, casting doesn't seem to make a difference.
					DoAnim(animKick, 0, false);
					int64 dmg = GetBaseSkillDamage(EQ::skills::SkillKick);

					if (GetWeaponDamage(target, boots) <= 0) {
						dmg = DMG_INVULNERABLE;
					}

					reuse = (KickReuseTime + 3) * 1000;
					DoSpecialAttackDamage(target, EQ::skills::SkillKick, dmg, GetMinDamage(), -1, reuse);
					did_attack = true;
				}
				else {
					DoAnim(animTailRake, 0, false);
					int64 dmg = GetBaseSkillDamage(EQ::skills::SkillBash);

					if (GetWeaponDamage(target, (const EQ::ItemData*)nullptr) <= 0)
						dmg = DMG_INVULNERABLE;

					reuse = (BashReuseTime + 3) * 1000;
					DoSpecialAttackDamage(target, EQ::skills::SkillBash, dmg, GetMinDamage(), -1, reuse);
					did_attack = true;
				}
			}
			break;
		}
		case Class::Berserker: case Class::BerserkerGM:{
			int AtkRounds = 1;
			int32 max_dmg = GetBaseSkillDamage(EQ::skills::SkillFrenzy);
			DoAnim(anim2HSlashing, 0, false);

			if (GetClass() == Class::Berserker) {
				int chance = GetLevel() * 2 + GetSkill(EQ::skills::SkillFrenzy);
				if (zone->random.Roll0(450) < chance)
					AtkRounds++;
				if (zone->random.Roll0(450) < chance)
					AtkRounds++;
			}

			while (AtkRounds > 0) {
				if (GetTarget())
					DoSpecialAttackDamage(GetTarget(), EQ::skills::SkillFrenzy, max_dmg, GetMinDamage(), -1, reuse);
				AtkRounds--;
			}

			did_attack = true;
			break;
		}
		case Class::Ranger: case Class::RangerGM:
		case Class::Beastlord: case Class::BeastlordGM: {
			//kick
			if(level >= RuleI(Combat, NPCBashKickLevel)){
				DoAnim(animKick, 0, false);
				int64 dmg = GetBaseSkillDamage(EQ::skills::SkillKick);

				if (GetWeaponDamage(target, boots) <= 0)
					dmg = DMG_INVULNERABLE;

				reuse = (KickReuseTime + 3) * 1000;
				DoSpecialAttackDamage(target, EQ::skills::SkillKick, dmg, GetMinDamage(), -1, reuse);
				did_attack = true;
			}
			break;
		}
		case Class::Cleric: case Class::ClericGM: //clerics can bash too.
		case Class::ShadowKnight: case Class::ShadowKnightGM:
		case Class::Paladin: case Class::PaladinGM:{
			if(level >= RuleI(Combat, NPCBashKickLevel)){
				DoAnim(animTailRake, 0, false);
				int64 dmg = GetBaseSkillDamage(EQ::skills::SkillBash);

				if (GetWeaponDamage(target, (const EQ::ItemData*)nullptr) <= 0)
					dmg = DMG_INVULNERABLE;

				reuse = (BashReuseTime + 3) * 1000;
				DoSpecialAttackDamage(target, EQ::skills::SkillBash, dmg, GetMinDamage(), -1, reuse);
				did_attack = true;
			}
			break;
		}
	}

	classattack_timer.Start(reuse / HasteModifier);
}

// this should be refactored to generate an OP_CombatAbility struct and call OPCombatAbility
void Client::DoClassAttacks(Mob *ca_target, uint16 skill, bool IsRiposte)
{
	if(!ca_target)
		return;

	if(spellend_timer.Enabled() || IsFeared() || IsStunned() || IsMezzed() || DivineAura() || dead)
		return;

	if(!IsAttackAllowed(ca_target))
		return;

	//check range for all these abilities, they are all close combat stuff
	if(!CombatRange(ca_target)){
		return;
	}

	if(!IsRiposte && (!p_timers.Expired(&database, pTimerCombatAbility, false))) {
		return;
	}

	int ReuseTime = 0;
	float HasteMod = GetHaste() * 0.01f;

	uint16 skill_to_use = -1;

	if (skill == -1){
		switch(GetClass()){
		case Class::Warrior:
		case Class::Ranger:
		case Class::Beastlord:
			skill_to_use = EQ::skills::SkillKick;
			break;
		case Class::Berserker:
			skill_to_use = EQ::skills::SkillFrenzy;
			break;
		case Class::ShadowKnight:
		case Class::Paladin:
			skill_to_use = EQ::skills::SkillBash;
			break;
		case Class::Monk:
			if(GetLevel() >= 30)
			{
				skill_to_use = EQ::skills::SkillFlyingKick;
			}
			else if(GetLevel() >= 25)
			{
				skill_to_use = EQ::skills::SkillDragonPunch;
			}
			else if(GetLevel() >= 20)
			{
				skill_to_use = EQ::skills::SkillEagleStrike;
			}
			else if(GetLevel() >= 10)
			{
				skill_to_use = EQ::skills::SkillTigerClaw;
			}
			else if(GetLevel() >= 5)
			{
				skill_to_use = EQ::skills::SkillRoundKick;
			}
			else
			{
				skill_to_use = EQ::skills::SkillKick;
			}
			break;
		case Class::Rogue:
			skill_to_use = EQ::skills::SkillBackstab;
			break;
		}
	}

	else
		skill_to_use = skill;

	if(skill_to_use == -1)
		return;

	int64 dmg = GetBaseSkillDamage(static_cast<EQ::skills::SkillType>(skill_to_use), GetTarget());

	if (skill_to_use == EQ::skills::SkillBash) {
		if (ca_target!=this) {
			DoAnim(animTailRake, 0, false);

			if (GetWeaponDamage(ca_target, GetInv().GetItem(EQ::invslot::slotSecondary)) <= 0 && GetWeaponDamage(ca_target, GetInv().GetItem(EQ::invslot::slotShoulders)) <= 0)
				dmg = DMG_INVULNERABLE;

			ReuseTime = (BashReuseTime - 1) / HasteMod;

			DoSpecialAttackDamage(ca_target, EQ::skills::SkillBash, dmg, 0, -1, ReuseTime);

			if(ReuseTime > 0 && !IsRiposte) {
				p_timers.Start(pTimerCombatAbility, ReuseTime);
			}
		}
		return;
	}

	if (skill_to_use == EQ::skills::SkillFrenzy) {
		CheckIncreaseSkill(EQ::skills::SkillFrenzy, GetTarget(), 10);
		int AtkRounds = 1;
		DoAnim(anim1HWeapon, 0, false);

		ReuseTime = (FrenzyReuseTime - 1) / HasteMod;

		// bards can do riposte frenzy for some reason
		if (!IsRiposte && GetClass() == Class::Berserker) {
			int chance = GetLevel() * 2 + GetSkill(EQ::skills::SkillFrenzy);
			if (zone->random.Roll0(450) < chance)
				AtkRounds++;
			if (zone->random.Roll0(450) < chance)
				AtkRounds++;
		}

		while(AtkRounds > 0) {
			if (ca_target!=this)
				DoSpecialAttackDamage(ca_target, EQ::skills::SkillFrenzy, dmg, 0, dmg, ReuseTime);
			AtkRounds--;
		}

		if(ReuseTime > 0 && !IsRiposte) {
			p_timers.Start(pTimerCombatAbility, ReuseTime);
		}
		return;
	}

	if (skill_to_use == EQ::skills::SkillKick){
		if(ca_target!=this){
			DoAnim(animKick, 0, false);

			if (GetWeaponDamage(ca_target, GetInv().GetItem(EQ::invslot::slotFeet)) <= 0)
				dmg = DMG_INVULNERABLE;

			ReuseTime = KickReuseTime-1;

			DoSpecialAttackDamage(ca_target, EQ::skills::SkillKick, dmg, 0, -1, ReuseTime);
		}
	}

	if (skill_to_use == EQ::skills::SkillFlyingKick || skill_to_use == EQ::skills::SkillDragonPunch || skill_to_use == EQ::skills::SkillEagleStrike || skill_to_use == EQ::skills::SkillTigerClaw || skill_to_use == EQ::skills::SkillRoundKick) {
		ReuseTime = MonkSpecialAttack(ca_target, skill_to_use) - 1;
		MonkSpecialAttack(ca_target, skill_to_use);

		if (IsRiposte)
			return;

		//Live AA - Technique of Master Wu
		int wuchance = itembonuses.DoubleSpecialAttack + spellbonuses.DoubleSpecialAttack + aabonuses.DoubleSpecialAttack;
		if (wuchance) {
			const int MonkSPA[5] = {EQ::skills::SkillFlyingKick, EQ::skills::SkillDragonPunch,
						EQ::skills::SkillEagleStrike, EQ::skills::SkillTigerClaw,
						EQ::skills::SkillRoundKick};
			int extra = 0;
			// always 1/4 of the double attack chance, 25% at rank 5 (100/4)
			while (wuchance > 0) {
				if (zone->random.Roll(wuchance))
					extra++;
				else
					break;
				wuchance /= 4;
			}
			// They didn't add a string ID for this.
			std::string msg = StringFormat(
			    "The spirit of Master Wu fills you!  You gain %d additional attack(s).", extra);
			// live uses 400 here -- not sure if it's the best for all clients though
			SendColoredText(400, msg);
			auto classic = RuleB(Combat, ClassicMasterWu);
			while (extra) {
				MonkSpecialAttack(GetTarget(),
						  classic ? MonkSPA[zone->random.Int(0, 4)] : skill_to_use);
				extra--;
			}
		}
	}

	if (skill_to_use == EQ::skills::SkillBackstab){
		ReuseTime = BackstabReuseTime-1;

		if (IsRiposte)
			ReuseTime=0;

		TryBackstab(ca_target,ReuseTime);
	}

	ReuseTime = ReuseTime / HasteMod;
	if(ReuseTime > 0 && !IsRiposte){
		p_timers.Start(pTimerCombatAbility, ReuseTime);
	}
}

	/* Classic Taunt Methodology
	* This is not how Sony did it.  This is a guess that fits the very limited data available.
	* Low level players with maxed taunt for their level taunted about 50% on white cons.
	* A 65 ranger with 150 taunt skill (max) taunted about 50% on level 60 and under NPCs.
	* A 65 warrior with maxed taunt (230) was taunting around 50% on SSeru NPCs.		*/

	/* Rashere in 2006: "your taunt skill was irrelevant if you were above level 60 and taunting
	* something that was also above level 60."
	* Also: "The chance to taunt an NPC higher level than yourself dropped off at double the rate
	* if you were above level 60 than if you were below level 60 making it very hard to taunt creature
	* higher level than yourself if you were above level 60."
	*
	* See http://www.elitegamerslounge.com/home/soearchive/viewtopic.php?t=81156 */
void Mob::Taunt(NPC *who, bool always_succeed, int chance_bonus, bool from_spell, int32 bonus_hate)
{
	if (!who || DivineAura() || (!from_spell && !CombatRange(who)) || (IsNPC() && IsCharmed())) {
		return;
	}

	if (!always_succeed && IsClient()) {
		CastToClient()->CheckIncreaseSkill(EQ::skills::SkillTaunt, who, 10);
	}

	Mob *hate_top        = who->GetHateMost();
	int level_difference = GetLevel() - who->GetLevel();
	bool success         = false;
	int taunt_chance     = 0;

	// Support for how taunt worked pre 2000 on LIVE - Can not taunt NPC over your level.
	if (
		!RuleB(Combat, TauntOverLevel) &&
		level_difference < 0 ||
		who->GetSpecialAbility(SpecialAbility::TauntImmunity)
	) {
		MessageString(Chat::SpellFailure, FAILED_TAUNT);
		return;
	}

	if (always_succeed) {
		taunt_chance = 100;
	}

	// Modern Taunt
	if (!RuleB(Combat, ClassicTauntSystem)) {
		if (
			(hate_top && hate_top->GetHPRatio() >= 20) ||
			!hate_top ||
			chance_bonus
		) {
			if (chance_bonus) {
				taunt_chance = chance_bonus;
			} else {
				taunt_chance = 50;
			}
		} else {
			if (level_difference < 0) {
				taunt_chance += level_difference * 3;
				if (taunt_chance < 20) {
					taunt_chance = 20;
				}
			} else {
				taunt_chance += level_difference * 5;
				if (taunt_chance > 65) {
					taunt_chance = 65;
				}
			}

			if (IsClient() && !always_succeed) {
				taunt_chance -= (RuleR(Combat, TauntSkillFalloff) *
					(CastToClient()->MaxSkill(EQ::skills::SkillTaunt) -
					GetSkill(EQ::skills::SkillTaunt)));
			}

			if (taunt_chance < 1) {
				taunt_chance = 1;
			}
		}
	} else { // Classic Taunt
		if (GetLevel() >= 60 && level_difference < 0) {
			if (level_difference < -5) {
				taunt_chance = 0;
			} else if (level_difference == -5) {
				taunt_chance = 10;
			} else {
				taunt_chance = 50 + level_difference * 10;
			}
		} else {
			// this will make the skill difference between the tank classes actually affect success rates
			// but only for NPCs near the player's level.  Mid to low blues will start to taunt at 50%
			// even with lower skill
			taunt_chance = 50 * GetSkill(EQ::skills::SkillTaunt) / (who->GetLevel() * 5 + 5);
			taunt_chance += level_difference * 5;

			if (taunt_chance > 50) {
				taunt_chance = 50;
			} else if (taunt_chance < 10) {
				taunt_chance = 10;
			}
		}

		// Taunt Chance Rule Bonus
		taunt_chance += RuleI(Combat, TauntChanceBonus);
	}

	//success roll
	success = zone->random.Roll(taunt_chance);

	// Log result
	LogHate(
		"Taunter mob [{}] target npc [{}] taunt_chance [{}] success [{}] hate_top [{}]",
		GetMobDescription(),
		who->GetMobDescription(),
		taunt_chance,
		success ? "true" : "false",
		hate_top ? hate_top->GetMobDescription() : "not found"
	);

	// Actual Taunting
	if (success) {
		if (hate_top && hate_top != this) {
			int64 new_hate = (
				(who->GetNPCHate(hate_top) - who->GetNPCHate(this)) +
				bonus_hate +
				RuleI(Combat, TauntOverAggro) +
				1
			);

			LogHate(
				"Not Top Hate - Taunter [{}] Target [{}] Hated Top [{}] Hate Top Amt [{}] This Character Amt [{}] Bonus_Hate Amt [{}] TauntOverAggro Amt [{}] - Total [{}]",
				GetMobDescription(),
				who->GetMobDescription(),
				hate_top->GetMobDescription(),
				who->GetNPCHate(hate_top),
				who->GetNPCHate(this),
				bonus_hate,
				RuleI(Combat, TauntOverAggro),
				new_hate
			);

			who->CastToNPC()->AddToHateList(this, new_hate);
		} else {
			LogHate("Already Hate Top");
			who->CastToNPC()->AddToHateList(this, 12);
		}

		if (who->CanTalk()) {
			who->SayString(SUCCESSFUL_TAUNT, GetCleanName());
		}
	} else {
		MessageString(Chat::Skills, FAILED_TAUNT);
	}

	// Modern Abilities
	if (!RuleB(Combat, ClassicTauntSystem)) {
		TryCastOnSkillUse(who, EQ::skills::SkillTaunt);

		if (HasSkillProcs()) {
			TrySkillProc(who, EQ::skills::SkillTaunt, TauntReuseTime * 1000);
		}

		if (success && HasSkillProcSuccess()) {
			TrySkillProc(who, EQ::skills::SkillTaunt, TauntReuseTime * 1000, true);
		}
	}
}

void Mob::InstillDoubt(Mob *who) {
	//make sure we can use this skill
	/*int skill = GetSkill(INTIMIDATION);*/	//unused

	//make sure our target is an NPC
	if(!who || !who->IsNPC())
		return;

	if(DivineAura())
		return;

	//range check
	if(!CombatRange(who))
		return;

	if(IsClient()) {
		CastToClient()->CheckIncreaseSkill(EQ::skills::SkillIntimidation, who, 10);
	}

	//I think this formula needs work
	int value = 0;
	bool success = false;

	//user's bonus
	value += GetSkill(EQ::skills::SkillIntimidation) + GetCHA() / 4;

	//target's counters
	value -= target->GetLevel()*4 + who->GetWIS()/4;

	if (zone->random.Roll(value)) {
		//temporary hack...
		//cast fear on them... should prolly be a different spell
		//and should be un-resistable.
		SpellOnTarget(229, who, 0, true, -2000);
		success = true;
		//is there a success message?
	} else {
		MessageString(Chat::LightBlue,NOT_SCARING);
		//Idea from WR:
		/* if (target->IsNPC() && zone->random.Int(0,99) < 10 ) {
			entity_list.MessageClose(target, false, 50, Chat::NPCRampage, "%s lashes out in anger!",target->GetName());
			//should we actually do this? and the range is completely made up, unconfirmed
			entity_list.AEAttack(target, 50);
		}*/
	}

	TryCastOnSkillUse(who, EQ::skills::SkillIntimidation);

	if (HasSkillProcs()) {
		TrySkillProc(who, EQ::skills::SkillIntimidation, InstillDoubtReuseTime * 1000);
	}

	if (success && HasSkillProcSuccess()) {
		TrySkillProc(who, EQ::skills::SkillIntimidation, InstillDoubtReuseTime * 1000, true);
	}
}

int Mob::TryHeadShot(Mob *defender, EQ::skills::SkillType skillInUse)
{
	// Only works on YOUR target.
	if (
		defender &&
		!defender->IsOfClientBot() &&
		skillInUse == EQ::skills::SkillArchery &&
		GetTarget() == defender &&
		(defender->GetBodyType() == BodyType::Humanoid || !RuleB(Combat, HeadshotOnlyHumanoids)) &&
		!defender->GetSpecialAbility(SpecialAbility::HeadshotImmunity)
	) {
		uint32 HeadShot_Dmg = aabonuses.HeadShot[SBIndex::FINISHING_EFFECT_DMG] + spellbonuses.HeadShot[SBIndex::FINISHING_EFFECT_DMG] + itembonuses.HeadShot[SBIndex::FINISHING_EFFECT_DMG];
		uint8 HeadShot_Level = 0; // Get Highest Headshot Level
		HeadShot_Level = std::max({aabonuses.HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX], spellbonuses.HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX], itembonuses.HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX]});

		if (HeadShot_Dmg && HeadShot_Level && (defender->GetLevel() <= HeadShot_Level)) {
			int chance = GetDEX();
			chance = 100 * chance / (chance + 3500);
			if (IsOfClientBot()) {
				chance += GetHeroicDEX() / 25;
			}
			chance *= 10;
			int norm = aabonuses.HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_CHANCE_BONUS];
			if (norm > 0)
				chance = chance * norm / 100;
			chance += aabonuses.HeadShot[SBIndex::FINISHING_EFFECT_PROC_CHANCE] + spellbonuses.HeadShot[SBIndex::FINISHING_EFFECT_PROC_CHANCE] + itembonuses.HeadShot[SBIndex::FINISHING_EFFECT_PROC_CHANCE];
			if (zone->random.Int(1, 1000) <= chance) {
				entity_list.MessageCloseString(
					this, false, 200, Chat::MeleeCrit, FATAL_BOW_SHOT,
					GetName());
				return HeadShot_Dmg;
			}
		}
	}

	return 0;
}

int Mob::TryAssassinate(Mob *defender, EQ::skills::SkillType skillInUse)
{
	if (
		defender &&
		!defender->IsOfClientBot() &&
		GetLevel() >= RuleI(Combat, AssassinateLevelRequirement) &&
		(skillInUse == EQ::skills::SkillBackstab || skillInUse == EQ::skills::SkillThrowing) &&
		(defender->GetBodyType() == BodyType::Humanoid || !RuleB(Combat, AssassinateOnlyHumanoids)) &&
		!defender->GetSpecialAbility(SpecialAbility::AssassinateImmunity)
	) {
		int chance = GetDEX();
		if (skillInUse == EQ::skills::SkillBackstab) {
			chance = 100 * chance / (chance + 3500);
			if (IsOfClientBot()) {
				chance += GetHeroicDEX();
			}
			chance *= 10;
			int norm = aabonuses.AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_CHANCE_BONUS];
			if (norm > 0)
				chance = chance * norm / 100;
		} else if (skillInUse == EQ::skills::SkillThrowing) {
			if (chance > 255)
				chance = 260;
			else
				chance += 5;
		}

		chance += aabonuses.Assassinate[SBIndex::FINISHING_EFFECT_PROC_CHANCE] + spellbonuses.Assassinate[SBIndex::FINISHING_EFFECT_PROC_CHANCE] + itembonuses.Assassinate[SBIndex::FINISHING_EFFECT_PROC_CHANCE];

		uint32 Assassinate_Dmg =
				   aabonuses.Assassinate[SBIndex::FINISHING_EFFECT_DMG] + spellbonuses.Assassinate[SBIndex::FINISHING_EFFECT_DMG] + itembonuses.Assassinate[SBIndex::FINISHING_EFFECT_DMG];

		uint8 Assassinate_Level = 0; // Get Highest Headshot Level
		Assassinate_Level = std::max(
		    {aabonuses.AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX], spellbonuses.AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX], itembonuses.AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX]});

		// revamped AAs require AA line I believe?
		if (!Assassinate_Level)
			return 0;

		if (Assassinate_Dmg && Assassinate_Level && (defender->GetLevel() <= Assassinate_Level)) {
			if (zone->random.Int(1, 1000) <= chance) {
				entity_list.MessageCloseString(
					this, false, 200, Chat::MeleeCrit, ASSASSINATES,
					GetName());
				return Assassinate_Dmg;
			}
		}
	}

	return 0;
}

void Mob::DoMeleeSkillAttackDmg(Mob *other, int32 weapon_damage, EQ::skills::SkillType skillinuse, int16 chance_mod,
				int16 focus, bool can_riposte, int ReuseTime)
{
	if (!CanDoSpecialAttack(other)) {
		return;
	}

	/*
		For spells using skill value 98 (feral swipe ect) server sets this to 67 automatically.
		Kayen: This is unlikely to be completely accurate but use OFFENSE skill value for these effects.
		TODO: We need to stop moving skill 98, it's suppose to just be a dummy skill AFAIK
		Spells using offense should use the skill of your primary, if you can use it, otherwise h2h
	*/
	if (skillinuse == EQ::skills::SkillBegging) {
		skillinuse = EQ::skills::SkillOffense;
	}

	int64 damage = 0;
	int64 hate = 0;
	if (hate == 0 && weapon_damage > 1) {
		hate = weapon_damage;
	}

	if (weapon_damage > 0) {
		if (focus) {
			weapon_damage += weapon_damage * focus / 100;
		}

		if (skillinuse == EQ::skills::SkillBash && IsClient()) {
			EQ::ItemInstance *item =
				CastToClient()->GetInv().GetItem(EQ::invslot::slotSecondary);
			if (item) {
				if (item->GetItem()->ItemType == EQ::item::ItemTypeShield) {
					hate += item->GetItem()->AC;
				}
				const EQ::ItemData *itm = item->GetItem();
				hate = hate * (100 + GetSpellFuriousBash(itm->Focus.Effect)) / 100;
			}
		}

		DamageHitInfo my_hit {};

		my_hit.base_damage = weapon_damage;
		my_hit.min_damage  = 0;
		my_hit.damage_done = 1;
		my_hit.skill       = skillinuse;
		my_hit.offense     = offense(my_hit.skill);
		my_hit.tohit       = GetTotalToHit(my_hit.skill, chance_mod);
		my_hit.hand        = can_riposte ? EQ::invslot::slotPrimary : EQ::invslot::slotRange;

		if (IsNPC()) {
			my_hit.min_damage = CastToNPC()->GetMinDamage();
		}

		DoAttack(other, my_hit);
		damage = my_hit.damage_done;
	} else {
		damage = DMG_INVULNERABLE;
	}

	if (skillinuse == EQ::skills::SkillOffense) {    // Hack to allow damage to display.
		skillinuse = EQ::skills::SkillTigerClaw; //'strike' your opponent - Arbitrary choice for message.
	}

	other->AddToHateList(this, hate, 0);
	other->Damage(this, damage, SPELL_UNKNOWN, skillinuse);

	if (HasDied()) {
		return;
	}

	TryCastOnSkillUse(other, skillinuse);
}

bool Mob::CanDoSpecialAttack(Mob *other) {
	//Make sure everything is valid before doing any attacks.
	if (!other) {
		SetTarget(nullptr);
		return false;
	}

	if(!GetTarget())
		SetTarget(other);

	if ((other == nullptr || ((IsClient() && CastToClient()->dead) || (other->IsClient() && other->CastToClient()->dead)) || HasDied() || (!IsAttackAllowed(other)))) {
		return false;
	}

	if(other->GetInvul() || other->GetSpecialAbility(SpecialAbility::MeleeImmunity))
		return false;

	return true;
}
