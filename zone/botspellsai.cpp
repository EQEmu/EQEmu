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

#include "bot.h"

#include "common/data_verification.h"
#include "common/repositories/bot_spells_entries_repository.h"
#include "common/repositories/npc_spells_repository.h"

bool Bot::AICastSpell(Mob* tar, uint8 chance, uint16 spell_type, uint16 sub_target_type, uint16 sub_type) {
	if (!tar) {
		return false;
	}

	LogBotSpellChecksDetail("{} says, 'Attempting {} AICastSpell on {}.'", GetCleanName(), GetSpellTypeNameByID(spell_type), tar->GetCleanName());

	if (
		!AI_HasSpells() ||
		(spell_type == BotSpellTypes::Pet && tar != this) ||
		(IsPetBotSpellType(spell_type) && !tar->IsPet()) ||
		(!IsPetBotSpellType(spell_type) && tar->IsPet()) ||
		(!RuleB(Bots, AllowBuffingHealingFamiliars) && tar->IsFamiliar()) ||
		(tar->IsPet() && tar->IsCharmed() && spell_type == BotSpellTypes::PetBuffs && !RuleB(Bots, AllowCharmedPetBuffs)) ||
		(tar->IsPet() && tar->IsCharmed() && spell_type == BotSpellTypes::PetCures && !RuleB(Bots, AllowCharmedPetCures)) ||
		(tar->IsPet() && tar->IsCharmed() && IsHealBotSpellType(spell_type) && !RuleB(Bots, AllowCharmedPetHeals))
	) {
		return false;
	}

	if (
		!IsCommandedSpell() &&
		zone->random.Int(0, 100) > chance
	) {
		return false;
	}

	if ((spell_type != BotSpellTypes::Resurrect && spell_type != BotSpellTypes::SummonCorpse) && tar->GetAppearance() == eaDead) {
		if (!((tar->IsClient() && tar->CastToClient()->GetFeigned()) || tar->IsBot())) {
			return false;
		}
	}

	uint8 bot_class = GetClass();

	SetCastedSpellType(UINT16_MAX); // this is for recast timers
	SetTempSpellType(spell_type); // this is for spell checks

	BotSpell bot_spell;
	bot_spell.SpellId = 0;
	bot_spell.SpellIndex = 0;
	bot_spell.ManaCost = 0;

	if (!BotSpellTypeRequiresLoS(spell_type) || tar == this) {
		SetHasLoS(true);
	}

	switch (spell_type) {
		case BotSpellTypes::Slow:
			if (tar->GetSpecialAbility(SpecialAbility::SlowImmunity)) {
				return false;
			}

			break;
		case BotSpellTypes::Snare:
			if (tar->GetSpecialAbility(SpecialAbility::SnareImmunity)) {
				return false;
			}

			break;
		case BotSpellTypes::AELull:
		case BotSpellTypes::Lull:
			if (tar->GetSpecialAbility(SpecialAbility::PacifyImmunity)) {
				return false;
			}

			break;
		case BotSpellTypes::Fear:
			if (tar->GetSpecialAbility(SpecialAbility::FearImmunity)) {
				return false;
			}

			if (!IsCommandedSpell() && (tar->IsRooted() || tar->GetSnaredAmount() == -1)) {
				return false;
			}

			break;
		case BotSpellTypes::Dispel:
			if (tar->GetSpecialAbility(SpecialAbility::DispellImmunity)) {
				return false;
			}

			if (!IsCommandedSpell() && tar->CountDispellableBuffs() <= 0) {
				return false;
			}

			break;
		case BotSpellTypes::HateRedux:
			if (!IsCommandedSpell() && !GetNeedsHateRedux(tar)) {
				return false;
			}

			break;
		case BotSpellTypes::InCombatBuff:
			if (!IsCommandedSpell() && GetClass() != Class::Shaman && spell_type == BotSpellTypes::InCombatBuff && IsCasterClass(GetClass()) && GetLevel() >= GetStopMeleeLevel()) {
				return false;
			}

			break;
		case BotSpellTypes::HateLine:
			if (!tar->IsNPC()) {
				return false;
			}

			break;
		case BotSpellTypes::InCombatBuffSong:
		case BotSpellTypes::Buff:
		case BotSpellTypes::PetBuffs:
		case BotSpellTypes::PreCombatBuff:
		case BotSpellTypes::DamageShields:
		case BotSpellTypes::PetDamageShields:
		case BotSpellTypes::ResistBuffs:
		case BotSpellTypes::PetResistBuffs:
		case BotSpellTypes::Teleport:
		case BotSpellTypes::Succor:
		case BotSpellTypes::BindAffinity:
		case BotSpellTypes::Identify:
		case BotSpellTypes::Levitate:
		case BotSpellTypes::Rune:
		case BotSpellTypes::WaterBreathing:
		case BotSpellTypes::Size:
		case BotSpellTypes::Invisibility:
		case BotSpellTypes::MovementSpeed:
		case BotSpellTypes::SendHome:
			if (!tar->IsOfClientBot() && !(tar->IsPet() && tar->GetOwner() && tar->GetOwner()->IsOfClientBot())) {
				return false;
			}

			break;
		case BotSpellTypes::PreCombatBuffSong:
		case BotSpellTypes::OutOfCombatBuffSong:
			if (!IsCommandedSpell() && (IsEngaged() || tar->IsEngaged())) { // Out-of-Combat songs can not be cast in combat
				return false;
			}

			break;
		case BotSpellTypes::AEMez:
		case BotSpellTypes::Mez:
			return BotCastMez(tar, bot_class, bot_spell, spell_type);
		case BotSpellTypes::AENukes:
		case BotSpellTypes::AERains:
		case BotSpellTypes::PBAENuke:
		case BotSpellTypes::Nuke:
		case BotSpellTypes::AEStun:
		case BotSpellTypes::Stun:
			return BotCastNuke(tar, bot_class, bot_spell, spell_type);
		case BotSpellTypes::RegularHeal:
		case BotSpellTypes::GroupCompleteHeals:
		case BotSpellTypes::CompleteHeal:
		case BotSpellTypes::FastHeals:
		case BotSpellTypes::VeryFastHeals:
		case BotSpellTypes::GroupHeals:
		case BotSpellTypes::GroupHoTHeals:
		case BotSpellTypes::HoTHeals:
		case BotSpellTypes::PetRegularHeals:
		case BotSpellTypes::PetCompleteHeals:
		case BotSpellTypes::PetFastHeals:
		case BotSpellTypes::PetVeryFastHeals:
		case BotSpellTypes::PetHoTHeals:
			if (!tar->IsOfClientBot() && !(tar->IsPet() && tar->GetOwner() && tar->GetOwner()->IsOfClientBot())) {
				return false;
			}

			return BotCastHeal(tar, bot_class, bot_spell, spell_type);
		case BotSpellTypes::GroupCures:
		case BotSpellTypes::Cure:
		case BotSpellTypes::PetCures:
			if (!tar->IsOfClientBot() && !(tar->IsPet() && tar->GetOwner() && tar->GetOwner()->IsOfClientBot())) {
				return false;
			}

			return BotCastCure(tar, bot_class, bot_spell, spell_type);
		case BotSpellTypes::Pet:
			if (HasPet() || IsBotCharmer()) {
				return false;
			}

			return BotCastPet(tar, bot_class, bot_spell, spell_type);
		case BotSpellTypes::Resurrect:
			if (!tar->IsCorpse() || !tar->CastToCorpse()->IsPlayerCorpse()) {
				return false;
			}

			break;
		case BotSpellTypes::Charm:
			if (HasPet() || tar->IsCharmed() || !tar->IsNPC() || tar->GetSpecialAbility(SpecialAbility::CharmImmunity)) {
				return false;
			}

			break;
		default:
			break;
	}

	std::vector<BotSpell_wPriority> bot_spell_list = GetPrioritizedBotSpellsBySpellType(this, spell_type, tar, (IsAEBotSpellType(spell_type) || sub_target_type == CommandedSubTypes::AETarget), sub_target_type, sub_type);

	for (const auto& s : bot_spell_list) {
		if (!IsValidSpell(s.SpellId)) {
			continue;
		}

		if (BotRequiresLoSToCast(spell_type, s.SpellId) && !HasLoS()) {
			continue;
		}

		if (IsInvulnerabilitySpell(s.SpellId)) {
			tar = this; //target self for invul type spells
		}

		if (IsCommandedSpell() && IsCasting()) {
			RaidGroupSay(
				fmt::format(
					"Interrupting {}. I have been commanded to try to cast a [{}] spell, {} on {}.",
					CastingSpellID() ? spells[CastingSpellID()].name : "my spell",
					GetSpellTypeNameByID(spell_type),
					spells[s.SpellId].name,
					tar->GetCleanName()
				).c_str()
			);

			InterruptSpell();
		}

		if (AIDoSpellCast(s.SpellIndex, tar, s.ManaCost)) {
			if (BotSpellTypeUsesTargetSettings(spell_type)) {
				SetCastedSpellType(UINT16_MAX);

				if (!IsCommandedSpell()) {
					SetBotSpellRecastTimer(spell_type, tar, true);
				}
			}
			else {
				SetCastedSpellType(spell_type);
			}

			RaidGroupSay(
				fmt::format(
					"Casting {} [{}] on {}.",
					GetSpellName(s.SpellId),
					GetSpellTypeNameByID(spell_type),
					(tar == this ? "myself" : tar->GetCleanName())
				).c_str()
			);

			return true;
		}
	}

	return false;
}

bool Bot::BotCastMez(Mob* tar, uint8 bot_class, BotSpell& bot_spell, uint16 spell_type) {
	std::vector<BotSpell_wPriority> bot_spell_list = GetPrioritizedBotSpellsBySpellType(this, spell_type, tar, IsAEBotSpellType(spell_type));

	for (const auto& s : bot_spell_list) {
		if (!IsValidSpell(s.SpellId)) {
			continue;
		}

		if (BotRequiresLoSToCast(spell_type, s.SpellId) && !HasLoS()) {
			continue;
		}

		if (!IsCommandedSpell()) {
			Mob* add_mob = GetFirstIncomingMobToMez(this, s.SpellId, spell_type, IsAEBotSpellType(spell_type));

			if (!add_mob) {
				continue;
			}

			tar = add_mob;
		}

		if (AIDoSpellCast(s.SpellIndex, tar, s.ManaCost)) {
			if (BotSpellTypeUsesTargetSettings(spell_type)) {
				SetCastedSpellType(UINT16_MAX);

				if (!IsCommandedSpell()) {
					SetBotSpellRecastTimer(spell_type, tar, true);
				}
			}
			else {
				SetCastedSpellType(spell_type);
			}

			RaidGroupSay(
				fmt::format(
					"Casting {} [{}] on {}.",
					GetSpellName(s.SpellId),
					GetSpellTypeNameByID(spell_type),
					(tar == this ? "myself" : tar->GetCleanName())
				).c_str()
			);

			return true;
		}
	}

	return false;
}

bool Bot::BotCastCure(Mob* tar, uint8 bot_class, BotSpell& bot_spell, uint16 spell_type) {
	uint32 current_time = Timer::GetCurrentTime();
	uint32 next_cure_time = tar->DontCureMeBefore();

	if (!IsCommandedSpell()) {
		if ((next_cure_time > current_time) || !GetNeedsCured(tar)) {
			return false;
		}
	}

	bot_spell = GetBestBotSpellForCure(this, tar, spell_type);

	if (!IsValidSpell(bot_spell.SpellId)) {
		return false;
	}

	if (BotRequiresLoSToCast(spell_type, bot_spell.SpellId) && !HasLoS()) {
		return false;
	}

	if (AIDoSpellCast(bot_spell.SpellIndex, tar, bot_spell.ManaCost)) {
		if (IsGroupSpell(bot_spell.SpellId)) {
			if (!IsCommandedSpell()) {
				for (Mob* m : GatherSpellTargets(false, tar)) {
					SetBotSpellRecastTimer(spell_type, m, true);
				}
			}

			RaidGroupSay(
				fmt::format(
					"Curing the group with {}.",
					GetSpellName(bot_spell.SpellId)
				).c_str()
			);
		}
		else {
			if (!IsCommandedSpell()) {
				SetBotSpellRecastTimer(spell_type, tar, true);
			}

			RaidGroupSay(
				fmt::format(
					"Curing {} with {}.",
					(tar == this ? "myself" : tar->GetCleanName()),
					GetSpellName(bot_spell.SpellId)
				).c_str()
			);
		}

		return true;
	}

	return false;
}

bool Bot::BotCastPet(Mob* tar, uint8 bot_class, BotSpell& bot_spell, uint16 spell_type) {
	if (bot_class == Class::Wizard) {
		auto buffs_max = GetMaxBuffSlots();
		auto my_buffs = GetBuffs();
		int familiar_buff_slot = -1;
		if (buffs_max && my_buffs) {
			for (int index = 0; index < buffs_max; ++index) {
				if (IsEffectInSpell(my_buffs[index].spellid, SpellEffect::Familiar)) {
					MakePet(my_buffs[index].spellid, spells[my_buffs[index].spellid].teleport_zone);
					familiar_buff_slot = index;
					break;
				}
			}
		}
		if (GetPetID()) {
			return false;
		}
		if (familiar_buff_slot >= 0) {
			BuffFadeBySlot(familiar_buff_slot);
			return false;
		}

		bot_spell = GetFirstBotSpellBySpellType(this, spell_type);
	}
	else if (bot_class == Class::Magician) {
		bot_spell = GetBestBotMagicianPetSpell(this, spell_type);
	}
	else {
		bot_spell = GetFirstBotSpellBySpellType(this, spell_type);
	}

	if (!IsValidSpell(bot_spell.SpellId)) {
		return false;
	}

	if (BotRequiresLoSToCast(spell_type, bot_spell.SpellId) && !HasLoS()) {
		return false;
	}

	if (AIDoSpellCast(bot_spell.SpellIndex, tar, bot_spell.ManaCost)) {
		SetCastedSpellType(spell_type);

		RaidGroupSay(
			fmt::format(
				"Summoning a pet [{}].",
				GetSpellName(bot_spell.SpellId)
			).c_str()
		);

		return true;
	}

	return false;
}

bool Bot::BotCastNuke(Mob* tar, uint8 bot_class, BotSpell& bot_spell, uint16 spell_type) {
	if (BotRequiresLoSToCast(spell_type, bot_spell.SpellId) && !HasLoS()) {
		return false;
	}

	if (spell_type == BotSpellTypes::Stun || spell_type == BotSpellTypes::AEStun) {
		uint8 stun_chance = (tar->IsCasting() ? RuleI(Bots, StunCastChanceIfCasting) : RuleI(Bots, StunCastChanceNormal));

		if (bot_class == Class::Paladin) {
			stun_chance = RuleI(Bots, StunCastChancePaladins);
		}

		if (
			!tar->GetSpecialAbility(SpecialAbility::StunImmunity) &&
			(
				IsCommandedSpell() ||
				(!tar->IsStunned() && (zone->random.Int(1, 100) <= stun_chance))
			)
		) {
			bot_spell = GetBestBotSpellForStunByTargetType(this, ST_TargetOptional, spell_type, IsAEBotSpellType(spell_type), tar);
		}

		if (!IsValidSpell(bot_spell.SpellId)) {
			return false;
		}
	}

	if (!IsValidSpell(bot_spell.SpellId)) {
		bot_spell = GetBestBotSpellForNukeByBodyType(this, tar->GetBodyType(), spell_type, IsAEBotSpellType(spell_type), tar);
	}

	if (spell_type == BotSpellTypes::Nuke && bot_class == Class::Wizard && !IsValidSpell(bot_spell.SpellId)) {
		bot_spell = GetBestBotWizardNukeSpellByTargetResists(this, tar, spell_type);
	}

	if (!IsValidSpell(bot_spell.SpellId)) {
		std::vector<BotSpell_wPriority> bot_spell_list = GetPrioritizedBotSpellsBySpellType(this, spell_type, tar, IsAEBotSpellType(spell_type));

		for (const auto& s : bot_spell_list) {
			if (!IsValidSpell(s.SpellId)) {
				continue;
			}

			if (AIDoSpellCast(s.SpellIndex, tar, s.ManaCost)) {
				SetCastedSpellType(spell_type);

				RaidGroupSay(
					fmt::format(
						"Casting {} [{}] on {}.",
						GetSpellName(s.SpellId),
						GetSpellTypeNameByID(spell_type),
						tar->GetCleanName()
					).c_str()
				);

				return true;
			}
		}
	}
	else {
		if (AIDoSpellCast(bot_spell.SpellIndex, tar, bot_spell.ManaCost)) {
			SetCastedSpellType(spell_type);

			RaidGroupSay(
				fmt::format(
					"Casting {} [{}] on {}.",
					GetSpellName(bot_spell.SpellId),
					GetSpellTypeNameByID(spell_type),
					tar->GetCleanName()
				).c_str()
			);

			return true;
		}
	}

	return false;
}

bool Bot::BotCastHeal(Mob* tar, uint8 bot_class, BotSpell& bot_spell, uint16 spell_type) {
	if (!TargetValidation(tar)) {
		return false;
	}

	bot_spell = GetSpellByHealType(spell_type, tar);

	if (!IsValidSpell(bot_spell.SpellId)) {
		return false;
	}

	if (AIDoSpellCast(bot_spell.SpellIndex, tar, bot_spell.ManaCost)) {
		if (IsGroupSpell(bot_spell.SpellId)) {
			if (bot_class != Class::Bard) {
				if (!IsCommandedSpell()) {
					for (Mob* m : GatherSpellTargets(false, tar)) {
						SetBotSpellRecastTimer(spell_type, m, true);
					}
				}
			}

			RaidGroupSay(
				fmt::format(
					"Healing the group with {} [{}].",
					GetSpellName(bot_spell.SpellId),
					GetSpellTypeNameByID(spell_type)
				).c_str()
			);

		}
		else {
			if (bot_class != Class::Bard) {
				if (!IsCommandedSpell()) {
					SetBotSpellRecastTimer(spell_type, tar, true);
				}
			}

			RaidGroupSay(
				fmt::format(
					"Healing {} with {} [{}].",
					(tar == this ? "myself" : tar->GetCleanName()),
					GetSpellName(bot_spell.SpellId),
					GetSpellTypeNameByID(spell_type)
				).c_str()
			);
		}

		return true;
	}

	return false;
}

bool Bot::AIDoSpellCast(int32 i, Mob* tar, int32 mana_cost, uint32* oDontDoAgainBefore) {
	bool result = false;

	// manacost has special values, -1 is no mana cost, -2 is instant cast (no mana)
	int32 manaCost = mana_cost;

	if (manaCost == -1) {
		manaCost = spells[AIBot_spells[i].spellid].mana;
	} else if (manaCost == -2) {
		manaCost = 0;
	}

	int64 hasMana = GetMana();

	// Allow bots to cast buff spells even if they are out of mana
	if (
		RuleB(Bots, FinishBuffing) &&
		manaCost > hasMana &&
		!IsEngaged() &&
		IsBotBuffSpellType(AIBot_spells[i].type)
	) {
		SetMana(manaCost);
	}

	float dist2 = 0;

	if (AIBot_spells[i].type == BotSpellTypes::Escape) {
		dist2 = 0;
	} else
		dist2 = DistanceSquared(m_Position, tar->GetPosition());

	if (IsValidSpellRange(AIBot_spells[i].spellid, tar) && (mana_cost <= GetMana() || IsBotNonSpellFighter())) {
		casting_spell_AIindex = i;
		LogAI("spellid [{}] tar [{}] mana [{}] Name [{}]", AIBot_spells[i].spellid, tar->GetName(), mana_cost, spells[AIBot_spells[i].spellid].name);
		result = Mob::CastSpell(AIBot_spells[i].spellid, tar->GetID(), EQ::spells::CastingSlot::Gem2, spells[AIBot_spells[i].spellid].cast_time, AIBot_spells[i].manacost == -2 ? 0 : mana_cost, oDontDoAgainBefore, -1, -1, 0, &(AIBot_spells[i].resist_adjust));

		if (IsCasting() && IsSitting())
			Stand();
	}

	// if the spell wasn't casted, then take back any extra mana that was given to the bot to cast that spell
	if (!result) {
		SetMana(hasMana);
	}

	return result;
}

bool Bot::AI_PursueCastCheck() {
	if (GetAppearance() == eaDead || delaytimer || spellend_timer.Enabled() || IsFeared() || IsSilenced() || IsAmnesiad() || GetHP() < 0) {
		return false;
	}

	bool result = false;

	if (GetTarget() && AIautocastspell_timer->Check(false)) {

		LogAIDetail("Bot Pursue autocast check triggered: [{}]", GetCleanName());
		LogBotSpellChecksDetail("{} says, 'AI_PursueCastCheck started.'", GetCleanName());

		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.

		if (!IsAttackAllowed(GetTarget())) {
			return false;
		}

		auto cast_order = GetSpellTypesPrioritized(BotPriorityCategories::Pursue);
		Mob* tar = nullptr;

		for (auto& current_cast : cast_order) {
			if (current_cast.priority == 0) {
				SetSpellTypeAITimer(current_cast.spellType, RuleI(Bots, AICastSpellTypeHeldDelay));
				LogBotSpellChecksDetail("{} says, '[{}] is priority 0, skipping.'", GetCleanName(), GetSpellTypeNameByID(current_cast.spellType));
				continue;
			}

			if (!RuleB(Bots, AllowAIMez) && (current_cast.spellType == BotSpellTypes::AEMez || current_cast.spellType == BotSpellTypes::Mez)) {
				continue;
			}

			if (IsCommandedBotSpellType(current_cast.spellType)) { // Unsupported by AI currently.
				continue;
			}

			if (AIBot_spells_by_type[current_cast.spellType].empty() && AIBot_spells_by_type[GetParentSpellType(current_cast.spellType)].empty()) {
				continue;
			}

			if (!SpellTypeAIDelayCheck(current_cast.spellType)) {
				continue;
			}

			result = AttemptAICastSpell(current_cast.spellType, nullptr);

			if (!result && IsBotSpellTypeBeneficial(current_cast.spellType)) {
				result = AttemptCloseBeneficialSpells(current_cast.spellType);
			}

			SetSpellTypeAITimer(current_cast.spellType, RuleI(Bots, AICastSpellTypeDelay));

			if (result) {
				break;
			}
		}

		if (!AIautocastspell_timer->Enabled()) {
			AIautocastspell_timer->Start(RandomTimer(RuleI(Bots, MinDelayBetweenInCombatCastAttempts), RuleI(Bots, MaxDelayBetweenInCombatCastAttempts)), false);
		}
	}

	return result;
}

bool Bot::AI_IdleCastCheck() {
	if (GetAppearance() == eaDead || delaytimer || spellend_timer.Enabled() || IsFeared() || IsSilenced() || IsAmnesiad() || GetHP() < 0) {
		return false;
	}

	bool result = false;

	if (AIautocastspell_timer->Check(false)) {

		LogAIDetail("Bot Non-Engaged autocast check triggered: [{}]", GetCleanName());
		LogBotSpellChecksDetail("{} says, 'AI_IdleCastCheck started.'", GetCleanName());

		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.

		bool pre_combat = false;
		Client* test_against = nullptr;

		if (HasGroup() && GetGroup()->GetLeader() && GetGroup()->GetLeader()->IsClient()) {
			test_against = GetGroup()->GetLeader()->CastToClient();
		}
		else if (GetOwner() && GetOwner()->IsClient()) {
			test_against = GetOwner()->CastToClient();
		}

		if (test_against) {
			pre_combat = test_against->GetBotPrecombat();
		}

		auto cast_order = GetSpellTypesPrioritized(BotPriorityCategories::Idle);
		Mob* tar = nullptr;

		for (auto& current_cast : cast_order) {
			if (current_cast.priority == 0) {
				SetSpellTypeAITimer(current_cast.spellType, RuleI(Bots, AICastSpellTypeHeldDelay));
				LogBotSpellChecksDetail("{} says, '[{}] is priority 0, skipping.'", GetCleanName(), GetSpellTypeNameByID(current_cast.spellType));
				continue;
			}

			if (!pre_combat && (current_cast.spellType == BotSpellTypes::PreCombatBuff || current_cast.spellType == BotSpellTypes::PreCombatBuffSong)) {
				continue;
			}

			if (!RuleB(Bots, AllowAIMez) && (current_cast.spellType == BotSpellTypes::AEMez || current_cast.spellType == BotSpellTypes::Mez)) {
				continue;
			}

			if (IsCommandedBotSpellType(current_cast.spellType)) { // Unsupported by AI currently.
				continue;
			}

			if (!IsBotSpellTypeBeneficial(current_cast.spellType)) {
				continue;
			}

			if (AIBot_spells_by_type[current_cast.spellType].empty() && AIBot_spells_by_type[GetParentSpellType(current_cast.spellType)].empty()) {
				continue;
			}

			if (!SpellTypeAIDelayCheck(current_cast.spellType)) {
				continue;
			}

			result = AttemptAICastSpell(current_cast.spellType, nullptr);

			if (result) {
				break;
			}

			result = AttemptCloseBeneficialSpells(current_cast.spellType);

			SetSpellTypeAITimer(current_cast.spellType, RuleI(Bots, AICastSpellTypeDelay));

			if (result) {
				break;
			}
		}

		if (!AIautocastspell_timer->Enabled()) {
			AIautocastspell_timer->Start(RandomTimer(RuleI(Bots, MinDelayBetweenOutCombatCastAttempts), RuleI(Bots, MaxDelayBetweenOutCombatCastAttempts)), false);
		}
	}

	return result;
}

bool Bot::AI_EngagedCastCheck() {
	if (GetAppearance() == eaDead || delaytimer || spellend_timer.Enabled() || IsFeared() || IsSilenced() || IsAmnesiad() || GetHP() < 0) {
		return false;
	}

	bool result = false;

	if (GetTarget() && AIautocastspell_timer->Check(false)) {

		LogAIDetail("Bot Engaged autocast check triggered: [{}]", GetCleanName());
		LogBotSpellChecksDetail("{} says, 'AI_EngagedCastCheck started.'", GetCleanName());

		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.

		if (!IsAttackAllowed(GetTarget())) {
			return false;
		}

		auto cast_order = GetSpellTypesPrioritized(BotPriorityCategories::Engaged);
		Mob* tar = nullptr;

		for (auto& current_cast : cast_order) {
			if (current_cast.priority == 0) {
				SetSpellTypeAITimer(current_cast.spellType, RuleI(Bots, AICastSpellTypeHeldDelay));
				LogBotSpellChecksDetail("{} says, '[{}] is priority 0, skipping.'", GetCleanName(), GetSpellTypeNameByID(current_cast.spellType));
				continue;
			}

			if (!RuleB(Bots, AllowAIMez) && (current_cast.spellType == BotSpellTypes::AEMez || current_cast.spellType == BotSpellTypes::Mez)) {
				continue;
			}

			if (IsCommandedBotSpellType(current_cast.spellType)) { // Unsupported by AI currently.
				continue;
			}

			if (AIBot_spells_by_type[current_cast.spellType].empty() && AIBot_spells_by_type[GetParentSpellType(current_cast.spellType)].empty()) {
				continue;
			}

			if (!SpellTypeAIDelayCheck(current_cast.spellType)) {
				continue;
			}

			result = AttemptAICastSpell(current_cast.spellType, nullptr);

			SetSpellTypeAITimer(current_cast.spellType, RuleI(Bots, AICastSpellTypeDelay));

			if (!result && IsBotSpellTypeBeneficial(current_cast.spellType)) {
				result = AttemptCloseBeneficialSpells(current_cast.spellType);
			}

			if (result) {
				break;
			}
		}

		if (!AIautocastspell_timer->Enabled()) {
			AIautocastspell_timer->Start(RandomTimer(RuleI(Bots, MinDelayBetweenInCombatCastAttempts), RuleI(Bots, MaxDelayBetweenInCombatCastAttempts)), false);
		}
	}

	return result;
}

bool Bot::AIHealRotation(Mob* tar, bool useFastHeals) {

	if (!tar) {
		return false;
	}

	if (!AI_HasSpells())
		return false;

	if (tar->GetAppearance() == eaDead) {
		if ((tar->IsClient() && tar->CastToClient()->GetFeigned()) || tar->IsBot()) {
			// do nothing
		}
		else {
			return false;
		}
	}

	uint8 botLevel = GetLevel();

	bool castedSpell = false;

	BotSpell botSpell;
	botSpell.SpellId = 0;
	botSpell.SpellIndex = 0;
	botSpell.ManaCost = 0;

	if (useFastHeals) {
		botSpell = GetBestBotSpellForRegularSingleTargetHeal(this, tar);

		if (!IsValidSpell(botSpell.SpellId))
			botSpell = GetBestBotSpellForFastHeal(this, tar);
	}
	else {
		botSpell = GetBestBotSpellForPercentageHeal(this, tar);

		if (!IsValidSpell(botSpell.SpellId)) {
			botSpell = GetBestBotSpellForRegularSingleTargetHeal(this, tar);
		}
		if (!IsValidSpell(botSpell.SpellId)) {
			botSpell = GetFirstBotSpellForSingleTargetHeal(this, tar);
		}
		if (!IsValidSpell(botSpell.SpellId)) {
			botSpell = GetFirstBotSpellBySpellType(this, BotSpellTypes::RegularHeal);
		}
	}

	LogAIDetail("heal spellid [{}] fastheals [{}] casterlevel [{}]",
		botSpell.SpellId, ((useFastHeals) ? ('T') : ('F')), GetLevel());

	LogAIDetail("target [{}] current_time [{}] donthealmebefore [{}]", tar->GetCleanName(), Timer::GetCurrentTime(), tar->DontHealMeBefore());

	// If there is still no spell id, then there isn't going to be one so we are done
	if (!IsValidSpell(botSpell.SpellId)) {
		return false;
	}

	// Can we cast this spell on this target?
	if (!
		(
			spells[botSpell.SpellId].target_type == ST_GroupTeleport ||
			spells[botSpell.SpellId].target_type == ST_Target ||
			tar == this
		) &&
		tar->CanBuffStack(botSpell.SpellId, botLevel, true) < 0
	) {
		return false;
	}

	uint32 TempDontHealMeBeforeTime = tar->DontHealMeBefore();
	if (IsValidSpellRange(botSpell.SpellId, tar)) {
		castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost, &TempDontHealMeBeforeTime);
	}

	if (castedSpell) {
		RaidGroupSay(
			fmt::format(
				"Casting {} on {}, please stay in range!",
				spells[botSpell.SpellId].name,
				tar->GetCleanName()
			).c_str()
		);
	}

	return castedSpell;
}

std::list<BotSpell> Bot::GetBotSpellsForSpellEffect(Bot* caster, uint16 spell_type, int spell_effect) {
	std::list<BotSpell> result;

	if (!caster) {
		return result;
	}

	if (auto bot_owner = caster->GetBotOwner(); !bot_owner) {
		return result;
	}

	if (caster->AI_HasSpells()) {
		const std::vector<BotSpells_wIndex>& bot_spell_list = caster->BotGetSpellsByType(spell_type);

		for (int i = bot_spell_list.size() - 1; i >= 0; i--) {
			if (!IsValidSpell(bot_spell_list[i].spellid)) {
				continue;
			}

			if (BotRequiresLoSToCast(spell_type, bot_spell_list[i].spellid) && !caster->HasLoS()) {
				continue;
			}

			if (
				caster->CheckSpellRecastTimer(bot_spell_list[i].spellid) &&
				(bot_spell_list[i].type == spell_type || bot_spell_list[i].type == GetParentSpellType(spell_type)) &&
				caster->IsValidSpellTypeBySpellID(spell_type, bot_spell_list[i].spellid) &&
				(IsEffectInSpell(bot_spell_list[i].spellid, spell_effect) || GetSpellTriggerSpellID(bot_spell_list[i].spellid, spell_effect))
			) {
				BotSpell bot_spell;
				bot_spell.SpellId = bot_spell_list[i].spellid;
				bot_spell.SpellIndex = bot_spell_list[i].index;
				bot_spell.ManaCost = bot_spell_list[i].manacost;

				result.push_back(bot_spell);
			}
		}
	}

	return result;
}

std::list<BotSpell> Bot::GetBotSpellsForSpellEffectAndTargetType(Bot* caster, uint16 spell_type, int spell_effect, SpellTargetType target_type) {
	std::list<BotSpell> result;

	if (!caster) {
		return result;
	}

	if (auto bot_owner = caster->GetBotOwner(); !bot_owner) {
		return result;
	}

	if (caster->AI_HasSpells()) {
		const std::vector<BotSpells_wIndex>& bot_spell_list = caster->BotGetSpellsByType(spell_type);

		for (int i = bot_spell_list.size() - 1; i >= 0; i--) {
			if (!IsValidSpell(bot_spell_list[i].spellid)) {
				continue;
			}

			if (BotRequiresLoSToCast(spell_type, bot_spell_list[i].spellid) && !caster->HasLoS()) {
				continue;
			}

			if (
				caster->CheckSpellRecastTimer(bot_spell_list[i].spellid) &&
				(bot_spell_list[i].type == spell_type || bot_spell_list[i].type == GetParentSpellType(spell_type)) &&
				caster->IsValidSpellTypeBySpellID(spell_type, bot_spell_list[i].spellid) &&
				(
					IsEffectInSpell(bot_spell_list[i].spellid, spell_effect) ||
					GetSpellTriggerSpellID(bot_spell_list[i].spellid, spell_effect)
				) &&
				(target_type == ST_TargetOptional || spells[bot_spell_list[i].spellid].target_type == target_type)
			) {
				BotSpell bot_spell;
				bot_spell.SpellId = bot_spell_list[i].spellid;
				bot_spell.SpellIndex = bot_spell_list[i].index;
				bot_spell.ManaCost = bot_spell_list[i].manacost;
				result.push_back(bot_spell);
			}
		}
	}

	return result;
}

std::list<BotSpell> Bot::GetBotSpellsBySpellType(Bot* caster, uint16 spell_type) {
	std::list<BotSpell> result;

	if (!caster) {
		return result;
	}

	if (auto bot_owner = caster->GetBotOwner(); !bot_owner) {
		return result;
	}

	if (caster->AI_HasSpells()) {
		const std::vector<BotSpells_wIndex>& bot_spell_list = caster->BotGetSpellsByType(spell_type);

		for (int i = bot_spell_list.size() - 1; i >= 0; i--) {
			if (!IsValidSpell(bot_spell_list[i].spellid)) {
				continue;
			}

			if (BotRequiresLoSToCast(spell_type, bot_spell_list[i].spellid) && !caster->HasLoS()) {
				continue;
			}

			if (
				caster->CheckSpellRecastTimer(bot_spell_list[i].spellid) &&
				(bot_spell_list[i].type == spell_type || bot_spell_list[i].type == GetParentSpellType(spell_type)) &&
				caster->IsValidSpellTypeBySpellID(spell_type, bot_spell_list[i].spellid)
			) {
				BotSpell bot_spell;
				bot_spell.SpellId = bot_spell_list[i].spellid;
				bot_spell.SpellIndex = bot_spell_list[i].index;
				bot_spell.ManaCost = bot_spell_list[i].manacost;

				result.push_back(bot_spell);
			}
		}
	}

	return result;
}

std::vector<BotSpell_wPriority> Bot::GetPrioritizedBotSpellsBySpellType(Bot* caster, uint16 spell_type, Mob* tar, bool AE, uint16 sub_target_type, uint16 sub_type) {
	std::vector<BotSpell_wPriority> result;

	if (caster && caster->AI_HasSpells()) {
		const std::vector<BotSpells_wIndex>& bot_spell_list = caster->BotGetSpellsByType(spell_type);

		for (int i = bot_spell_list.size() - 1; i >= 0; i--) {
			if (!IsValidSpell(bot_spell_list[i].spellid)) {
				continue;
			}

			if (BotRequiresLoSToCast(spell_type, bot_spell_list[i].spellid) && !caster->HasLoS()) {
				continue;
			}

			if (spell_type == BotSpellTypes::HateRedux && caster->GetClass() == Class::Bard) {
				if (spells[bot_spell_list[i].spellid].target_type != ST_Target) {
					continue;
				}
			}

			if (
				caster->CheckSpellRecastTimer(bot_spell_list[i].spellid) &&
				(bot_spell_list[i].type == spell_type || bot_spell_list[i].type == GetParentSpellType(spell_type)) &&
				caster->IsValidSpellTypeBySpellID(spell_type, bot_spell_list[i].spellid)
			) {
				if (
					caster->IsCommandedSpell() &&
					(
						!caster->IsValidSpellTypeSubType(spell_type, sub_target_type, bot_spell_list[i].spellid) ||
						!caster->IsValidSpellTypeSubType(spell_type, sub_type, bot_spell_list[i].spellid)
					)
				) {
					continue;
				}

				if (!AE && IsAnyAESpell(bot_spell_list[i].spellid) && !IsGroupSpell(bot_spell_list[i].spellid)) {
					continue;
				}
				else if (AE && !IsAnyAESpell(bot_spell_list[i].spellid)) {
					continue;
				}

				if (
					!caster->IsInGroupOrRaid(tar, true) &&
					(
						!RuleB(Bots, EnableBotTGB) ||
						(
							IsGroupSpell(bot_spell_list[i].spellid) &&
							!IsTGBCompatibleSpell(bot_spell_list[i].spellid)
						)
					)
				) {
					continue;
				}

				if (!IsPBAESpell(bot_spell_list[i].spellid) && !caster->CastChecks(bot_spell_list[i].spellid, tar, spell_type, false, IsAEBotSpellType(spell_type))) {
					continue;
				}

				if (
					caster->IsCommandedSpell() ||
					!AE ||
					!BotSpellTypeRequiresAEChecks(spell_type) ||
					caster->HasValidAETarget(caster, bot_spell_list[i].spellid, spell_type, tar)
				) {
					BotSpell_wPriority bot_spell;
					bot_spell.SpellId = bot_spell_list[i].spellid;
					bot_spell.SpellIndex = bot_spell_list[i].index;
					bot_spell.ManaCost = bot_spell_list[i].manacost;
					bot_spell.Priority = bot_spell_list[i].priority;

					result.emplace_back(bot_spell);
				}
			}
		}

		if (result.size() > 1) {
			std::sort(result.begin(), result.end(), [](BotSpell_wPriority const& l, BotSpell_wPriority const& r) {
				return l.Priority < r.Priority;
			});
		}
	}

	return result;
}

BotSpell Bot::GetFirstBotSpellBySpellType(Bot* caster, uint16 spell_type) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (caster && caster->AI_HasSpells()) {
		const std::vector<BotSpells_wIndex>& bot_spell_list = caster->BotGetSpellsByType(spell_type);

		for (int i = bot_spell_list.size() - 1; i >= 0; i--) {
			if (!IsValidSpell(bot_spell_list[i].spellid)) {
				continue;
			}

			if (BotRequiresLoSToCast(spell_type, bot_spell_list[i].spellid) && !caster->HasLoS()) {
				continue;
			}

			if (
				caster->CheckSpellRecastTimer(bot_spell_list[i].spellid) &&
				(bot_spell_list[i].type == spell_type || bot_spell_list[i].type == GetParentSpellType(spell_type)) &&
				caster->IsValidSpellTypeBySpellID(spell_type, bot_spell_list[i].spellid)
			) {
				result.SpellId = bot_spell_list[i].spellid;
				result.SpellIndex = bot_spell_list[i].index;
				result.ManaCost = bot_spell_list[i].manacost;

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForVeryFastHeal(Bot* caster, Mob* tar, uint16 spell_type) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (caster) {
		std::list<BotSpell> bot_spell_list = GetBotSpellsForSpellEffect(caster, spell_type, SpellEffect::CurrentHP);

		for (auto bot_spell_list_itr : bot_spell_list) {
			if (
				IsVeryFastHealSpell(bot_spell_list_itr.SpellId) && caster->CastChecks(bot_spell_list_itr.SpellId, tar, spell_type)) {
				result.SpellId = bot_spell_list_itr.SpellId;
				result.SpellIndex = bot_spell_list_itr.SpellIndex;
				result.ManaCost = bot_spell_list_itr.ManaCost;

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForFastHeal(Bot* caster, Mob* tar, uint16 spell_type) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (caster) {
		std::list<BotSpell> bot_spell_list = GetBotSpellsForSpellEffect(caster, spell_type, SpellEffect::CurrentHP);

		for (auto bot_spell_list_itr : bot_spell_list) {
			if (IsFastHealSpell(bot_spell_list_itr.SpellId) && caster->CastChecks(bot_spell_list_itr.SpellId, tar, spell_type)) {
				result.SpellId = bot_spell_list_itr.SpellId;
				result.SpellIndex = bot_spell_list_itr.SpellIndex;
				result.ManaCost = bot_spell_list_itr.ManaCost;

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForHealOverTime(Bot* caster, Mob* tar, uint16 spell_type) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (caster) {
		std::list<BotSpell> bot_spell_list = GetBotSpellsForSpellEffect(caster, spell_type, SpellEffect::HealOverTime);

		for (auto bot_spell_list_itr : bot_spell_list) {
			if (IsHealOverTimeSpell(bot_spell_list_itr.SpellId) && caster->CastChecks(bot_spell_list_itr.SpellId, tar, spell_type)) {
				result.SpellId = bot_spell_list_itr.SpellId;
				result.SpellIndex = bot_spell_list_itr.SpellIndex;
				result.ManaCost = bot_spell_list_itr.ManaCost;

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForPercentageHeal(Bot* caster, Mob* tar, uint16 spell_type) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (caster && caster->AI_HasSpells()) {
		const std::vector<BotSpells_wIndex>& bot_spell_list = caster->BotGetSpellsByType(spell_type);
		for (int i = bot_spell_list.size() - 1; i >= 0; i--) {
			if (!IsValidSpell(bot_spell_list[i].spellid)) {
				continue;
			}

			if (
				(bot_spell_list[i].type == spell_type || bot_spell_list[i].type == GetParentSpellType(spell_type)) &&
				caster->IsValidSpellTypeBySpellID(spell_type, bot_spell_list[i].spellid) &&
				IsCompleteHealSpell(bot_spell_list[i].spellid) &&
				caster->CastChecks(bot_spell_list[i].spellid, tar, spell_type)
			) {
				result.SpellId = bot_spell_list[i].spellid;
				result.SpellIndex = bot_spell_list[i].index;
				result.ManaCost = bot_spell_list[i].manacost;

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForRegularSingleTargetHeal(Bot* caster, Mob* tar, uint16 spell_type) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (caster) {
		std::list<BotSpell> bot_spell_list = GetBotSpellsForSpellEffect(caster, spell_type, SpellEffect::CurrentHP);

		for (std::list<BotSpell>::iterator bot_spell_list_itr = bot_spell_list.begin(); bot_spell_list_itr != bot_spell_list.end(); ++bot_spell_list_itr) {
			if (IsRegularSingleTargetHealSpell(bot_spell_list_itr->SpellId) && caster->CastChecks(bot_spell_list_itr->SpellId, tar, spell_type)) {
				result.SpellId = bot_spell_list_itr->SpellId;
				result.SpellIndex = bot_spell_list_itr->SpellIndex;
				result.ManaCost = bot_spell_list_itr->ManaCost;

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetFirstBotSpellForSingleTargetHeal(Bot* caster, Mob* tar, uint16 spell_type) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (caster) {
		std::list<BotSpell> bot_spell_list = GetBotSpellsForSpellEffect(caster, spell_type, SpellEffect::CurrentHP);

		for (std::list<BotSpell>::iterator bot_spell_list_itr = bot_spell_list.begin(); bot_spell_list_itr != bot_spell_list.end(); ++bot_spell_list_itr) {
			if (IsRegularSingleTargetHealSpell(bot_spell_list_itr->SpellId) && caster->CastChecks(bot_spell_list_itr->SpellId, tar, spell_type)) {
				result.SpellId = bot_spell_list_itr->SpellId;
				result.SpellIndex = bot_spell_list_itr->SpellIndex;
				result.ManaCost = bot_spell_list_itr->ManaCost;

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForGroupHeal(Bot* caster, Mob* tar, uint16 spell_type) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (!caster->TargetValidation(tar)) {
		return result;
	}

	std::list<BotSpell> bot_spell_list = GetBotSpellsForSpellEffect(caster, spell_type, SpellEffect::CurrentHP);
	int target_count = 0;
	int required_count = caster->GetSpellTypeAEOrGroupTargetCount(spell_type);

	for (std::list<BotSpell>::iterator bot_spell_list_itr = bot_spell_list.begin(); bot_spell_list_itr != bot_spell_list.end(); ++bot_spell_list_itr) {
		if (IsRegularGroupHealSpell(bot_spell_list_itr->SpellId)) {
			uint16 spell_id = bot_spell_list_itr->SpellId;

			if (caster->TargetValidation(tar) && !caster->IsCommandedSpell() && caster->IsValidSpellRange(spell_id, tar)) {
				target_count = caster->GetNumberNeedingHealedInGroup(tar, spell_type, spell_id, caster->GetAOERange(spell_id));

				if (target_count < required_count) {
					continue;
				}
			}

			result.SpellId = bot_spell_list_itr->SpellId;
			result.SpellIndex = bot_spell_list_itr->SpellIndex;
			result.ManaCost = bot_spell_list_itr->ManaCost;

			break;
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForGroupHealOverTime(Bot* caster, Mob* tar, uint16 spell_type) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (!caster->TargetValidation(tar)) {
		return result;
	}

	std::list<BotSpell> bot_spell_list = GetBotSpellsForSpellEffect(caster, spell_type, SpellEffect::HealOverTime);
	int target_count = 0;
	int required_count = caster->GetSpellTypeAEOrGroupTargetCount(spell_type);

	for (std::list<BotSpell>::iterator bot_spell_list_itr = bot_spell_list.begin(); bot_spell_list_itr != bot_spell_list.end(); ++bot_spell_list_itr) {
		if (IsGroupHealOverTimeSpell(bot_spell_list_itr->SpellId)) {
			uint16 spell_id = bot_spell_list_itr->SpellId;

			if (caster->TargetValidation(tar) && !caster->IsCommandedSpell() && caster->IsValidSpellRange(spell_id, tar)) {
				target_count = caster->GetNumberNeedingHealedInGroup(tar, spell_type, spell_id, caster->GetAOERange(spell_id));

				if (target_count < required_count) {
					continue;
				}
			}

			result.SpellId = bot_spell_list_itr->SpellId;
			result.SpellIndex = bot_spell_list_itr->SpellIndex;
			result.ManaCost = bot_spell_list_itr->ManaCost;

			break;
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForGroupCompleteHeal(Bot* caster, Mob* tar, uint16 spell_type) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (!caster->TargetValidation(tar)) {
		return result;
	}

	std::list<BotSpell> bot_spell_list = GetBotSpellsForSpellEffect(caster, spell_type, SpellEffect::CompleteHeal);
	int target_count = 0;
	int required_count = caster->GetSpellTypeAEOrGroupTargetCount(spell_type);

	for (std::list<BotSpell>::iterator bot_spell_list_itr = bot_spell_list.begin(); bot_spell_list_itr != bot_spell_list.end(); ++bot_spell_list_itr) {
		if (IsGroupCompleteHealSpell(bot_spell_list_itr->SpellId)) {
			uint16 spell_id = bot_spell_list_itr->SpellId;

			if (caster->TargetValidation(tar) && !caster->IsCommandedSpell() && caster->IsValidSpellRange(spell_id, tar)) {
				target_count = caster->GetNumberNeedingHealedInGroup(tar, spell_type, spell_id, caster->GetAOERange(spell_id));

				if (target_count < required_count) {
					continue;
				}
			}

			result.SpellId = bot_spell_list_itr->SpellId;
			result.SpellIndex = bot_spell_list_itr->SpellIndex;
			result.ManaCost = bot_spell_list_itr->ManaCost;

			break;
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForMez(Bot* caster, uint16 spell_type) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (caster) {
		std::list<BotSpell> bot_spell_list = GetBotSpellsForSpellEffect(caster, spell_type, SpellEffect::Mez);

		for (std::list<BotSpell>::iterator bot_spell_list_itr = bot_spell_list.begin(); bot_spell_list_itr != bot_spell_list.end(); ++bot_spell_list_itr) {
			if (
				IsMesmerizeSpell(bot_spell_list_itr->SpellId) &&
				caster->CheckSpellRecastTimer(bot_spell_list_itr->SpellId)
			) {
				result.SpellId = bot_spell_list_itr->SpellId;
				result.SpellIndex = bot_spell_list_itr->SpellIndex;
				result.ManaCost = bot_spell_list_itr->ManaCost;

				break;
			}
		}
	}

	return result;
}

Mob* Bot::GetFirstIncomingMobToMez(Bot* caster, uint16 spell_id, uint16 spell_type, bool AE) {
	Mob* result = nullptr;

	if (caster && caster->GetOwner()) {
		int spell_range = caster->GetActSpellRange(spell_id, spells[spell_id].range);
		int spell_ae_range = caster->GetAOERange(spell_id);
		bool is_pbae_spell = IsPBAESpell(spell_id);
		NPC* npc = nullptr;

		for (auto& close_mob : caster->m_close_mobs) {
			npc = close_mob.second->CastToNPC();

			if (!npc) {
				continue;
			}

			if (!caster->IsValidMezTarget(caster->GetOwner(), npc, spell_id)) {
				continue;
			}

			if (is_pbae_spell) {
				if (spell_ae_range < Distance(caster->GetPosition(), npc->GetPosition())) {
					continue;
				}
			}
			else {
				if (spell_range < Distance(caster->GetPosition(), npc->GetPosition())) {
					continue;
				}
			}

			if (AE) {
				int target_count = 0;

				for (auto& close_mob : caster->m_close_mobs) {
					Mob* m = close_mob.second;

					if (!caster->IsValidMezTarget(caster->GetOwner(), m, spell_id)) {
						continue;
					}

					if (spell_ae_range < Distance(npc->GetPosition(), m->GetPosition())) {
						continue;
					}

					if (caster->CastChecks(spell_id, m, spell_type, true, true)) {
						++target_count;
					}

					if (target_count >= caster->GetSpellTypeAEOrGroupTargetCount(spell_type)) {
						break;
					}
				}

				if (target_count < caster->GetSpellTypeAEOrGroupTargetCount(spell_type)) {
					continue;
				}

				result = npc;
			}
			else {
				if (spell_range < Distance(caster->GetPosition(), npc->GetPosition())) {
					continue;
				}

				if (!caster->CastChecks(spell_id, npc, spell_type, true)) {
					continue;
				}

				result = npc;
			}

			if (result) {
				return result;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotMagicianPetSpell(Bot* caster, uint16 spell_type) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (caster) {
		std::list<BotSpell> bot_spell_list = GetBotSpellsForSpellEffect(caster, spell_type, SpellEffect::SummonPet);
		std::string pet_type = GetBotMagicianPetType(caster);

		for(std::list<BotSpell>::iterator bot_spell_list_itr = bot_spell_list.begin(); bot_spell_list_itr != bot_spell_list.end(); ++bot_spell_list_itr) {
			if (
				IsSummonPetSpell(bot_spell_list_itr->SpellId) &&
				caster->CheckSpellRecastTimer(bot_spell_list_itr->SpellId) &&
				!strncmp(spells[bot_spell_list_itr->SpellId].teleport_zone, pet_type.c_str(), pet_type.length())
			) {
					result.SpellId = bot_spell_list_itr->SpellId;
					result.SpellIndex = bot_spell_list_itr->SpellIndex;
					result.ManaCost = bot_spell_list_itr->ManaCost;

					break;
			}
		}
	}

	return result;
}

std::string Bot::GetBotMagicianPetType(Bot* caster) {
	std::string result;

	if (caster) {
		uint8 pet_type = caster->GetPetChooserID();
		uint8 bot_level = caster->GetLevel();
		bool epic_allowed = false;
		std::string epic_spell_name = RuleS(Bots, EpicPetSpellName);

		if (epic_spell_name.empty()) {
			epic_spell_name = "SumMageMultiElement";
		}

		if (RuleB(Bots, AllowMagicianEpicPet)) {
			if (bot_level >= RuleI(Bots, AllowMagicianEpicPetLevel)) {
				if (!RuleI(Bots, RequiredMagicianEpicPetItemID)) {
					epic_allowed = true;
				}
				else {
					bool has_item = caster->HasBotItem(RuleI(Bots, RequiredMagicianEpicPetItemID)) != INVALID_INDEX;

					if (has_item) {
						epic_allowed = true;
					}
				}
			}
		}

		if (pet_type > 0) {
			switch (pet_type) {
				case SumWater:
					result = std::string("SumWater");
					break;
				case SumFire:
					result = std::string("SumFire");
					break;
				case SumAir:
					result = std::string("SumAir");
					break;
				case SumEarth:
					result = std::string("SumEarth");
					break;
				case MonsterSum:
					result = std::string("MonsterSum");
					break;
				case SumMageMultiElement:
					if (epic_allowed) {
						result = epic_spell_name;
					}
					else {
						caster->SetPetChooserID(0);
					}
					break;
			}
		}
		else {
			uint8 air_min_level = 255;
			uint8 fire_min_level = 255;
			uint8 water_min_level = 255;
			uint8 earth_min_level = 255;
			uint8 monster_min_level = 255;
			uint8 epic_min_level = 255;
			std::list<BotSpell> bot_spell_list = caster->GetBotSpellsBySpellType(caster, BotSpellTypes::Pet);

			for (const auto& s : bot_spell_list) {
				if (!IsValidSpell(s.SpellId)) {
					continue;
				}

				if (!IsEffectInSpell(s.SpellId, SpellEffect::SummonPet)) {
					continue;
				}

				auto spell = spells[s.SpellId];

				if (!strncmp(spell.teleport_zone, "SumWater", 8) && spell.classes[Class::Magician - 1] < water_min_level) {
					water_min_level = spell.classes[Class::Magician - 1];
				}
				else if (!strncmp(spell.teleport_zone, "SumFire", 7) && spell.classes[Class::Magician - 1] < fire_min_level) {
					fire_min_level = spell.classes[Class::Magician - 1];
				}
				else if (!strncmp(spell.teleport_zone, "SumAir", 6) && spell.classes[Class::Magician - 1] < air_min_level) {
					air_min_level = spell.classes[Class::Magician - 1];
				}
				else if (!strncmp(spell.teleport_zone, "SumEarth", 8) && spell.classes[Class::Magician - 1] < earth_min_level) {
					earth_min_level = spell.classes[Class::Magician - 1];
				}
				else if (!strncmp(spell.teleport_zone, "MonsterSum", 10) && spell.classes[Class::Magician - 1] < monster_min_level) {
					monster_min_level = spell.classes[Class::Magician - 1];
				}
				else if (!strncmp(spell.teleport_zone, epic_spell_name.c_str(), epic_spell_name.length()) && spell.classes[Class::Magician - 1] < epic_min_level) {
					epic_min_level = spell.classes[Class::Magician - 1];
				}
			}

			if (epic_allowed) {
				epic_min_level = std::max(int(epic_min_level), RuleI(Bots, AllowMagicianEpicPetLevel));

				if (bot_level >= epic_min_level) {
					result = epic_spell_name;
				}
			}
			else {
				bool found = false;
				uint8 count = 0;

				while (count <= 4 && !found) {
					int counter = zone->random.Int(1, 4);

					switch (counter) {
						case SumWater:
							if (bot_level >= water_min_level) {
								result = std::string("SumWater");
							}

							found = true;
							break;
						case SumFire:
							if (bot_level >= fire_min_level) {
								result = std::string("SumFire");
							}

							found = true;
							break;
						case SumAir:
							if (bot_level >= air_min_level) {
								result = std::string("SumAir");
							}

							found = true;
							break;
						case SumEarth:
							if (bot_level >= earth_min_level) {
								result = std::string("SumEarth");
							}

							found = true;
							break;
					}

					++count;
				}
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForNukeByTargetType(Bot* caster, SpellTargetType target_type, uint16 spell_type, bool AE, Mob* tar) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (tar == nullptr) {
		tar = caster->GetTarget();
	}

	if (caster) {
		std::list<BotSpell> bot_spell_list = GetBotSpellsForSpellEffectAndTargetType(caster, spell_type, SpellEffect::CurrentHP, target_type);

		for(std::list<BotSpell>::iterator bot_spell_list_itr = bot_spell_list.begin(); bot_spell_list_itr != bot_spell_list.end(); ++bot_spell_list_itr) {
			if (IsPureNukeSpell(bot_spell_list_itr->SpellId) || IsDamageSpell(bot_spell_list_itr->SpellId)) {
				if (!AE && IsAnyAESpell(bot_spell_list_itr->SpellId) && !IsGroupSpell(bot_spell_list_itr->SpellId)) {
					continue;
				}
				else if (AE && !IsAnyAESpell(bot_spell_list_itr->SpellId)) {
					continue;
				}

				if (!IsPBAESpell(bot_spell_list_itr->SpellId) && !caster->CastChecks(bot_spell_list_itr->SpellId, tar, spell_type, false, IsAEBotSpellType(spell_type))) {
					continue;
				}

				if (
					caster->IsCommandedSpell() ||
					!AE ||
					(AE && caster->HasValidAETarget(caster, bot_spell_list_itr->SpellId, spell_type, tar))
				) {
					result.SpellId = bot_spell_list_itr->SpellId;
					result.SpellIndex = bot_spell_list_itr->SpellIndex;
					result.ManaCost = bot_spell_list_itr->ManaCost;

					break;
				}
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForStunByTargetType(Bot* caster, SpellTargetType target_type, uint16 spell_type, bool AE, Mob* tar)
{
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (tar == nullptr) {
		tar = caster->GetTarget();
	}

	if (caster)
	{
		std::list<BotSpell> bot_spell_list = GetBotSpellsForSpellEffectAndTargetType(caster, spell_type, SpellEffect::Stun, target_type);

		for(std::list<BotSpell>::iterator bot_spell_list_itr = bot_spell_list.begin(); bot_spell_list_itr != bot_spell_list.end(); ++bot_spell_list_itr)
		{
			if (IsStunSpell(bot_spell_list_itr->SpellId)) {
				if (!AE && IsAnyAESpell(bot_spell_list_itr->SpellId) && !IsGroupSpell(bot_spell_list_itr->SpellId)) {
					continue;
				}
				else if (AE && !IsAnyAESpell(bot_spell_list_itr->SpellId)) {
					continue;
				}

				if (!IsPBAESpell(bot_spell_list_itr->SpellId) && !caster->CastChecks(bot_spell_list_itr->SpellId, tar, spell_type, false, IsAEBotSpellType(spell_type))) {
					continue;
				}

				if (
					caster->IsCommandedSpell() ||
					!AE ||
					(AE && caster->HasValidAETarget(caster, bot_spell_list_itr->SpellId, spell_type, tar))
				) {
					result.SpellId = bot_spell_list_itr->SpellId;
					result.SpellIndex = bot_spell_list_itr->SpellIndex;
					result.ManaCost = bot_spell_list_itr->ManaCost;

					break;
				}
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotWizardNukeSpellByTargetResists(Bot* caster, Mob* target, uint16 spell_type) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (caster && target) {

		const int lure_resis_value = -100;

		int32 level_mod = (target->GetLevel() - caster->GetLevel()) * (target->GetLevel() - caster->GetLevel()) / 2;

		if (target->GetLevel() - caster->GetLevel() < 0) {
			level_mod = -level_mod;
		}
		const int max_target_resist_value = caster->GetSpellTypeResistLimit(spell_type);
		bool select_lure_nuke = false;

		if (((target->GetMR() + level_mod) > max_target_resist_value) && ((target->GetCR() + level_mod) > max_target_resist_value) && ((target->GetFR() + level_mod) > max_target_resist_value)) {
			select_lure_nuke = true;
		}


		std::list<BotSpell> bot_spell_list = GetBotSpellsForSpellEffectAndTargetType(caster, spell_type, SpellEffect::CurrentHP, ST_Target);

		BotSpell first_wizard_magic_nuke_spell_found;
		first_wizard_magic_nuke_spell_found.SpellId = 0;
		first_wizard_magic_nuke_spell_found.SpellIndex = 0;
		first_wizard_magic_nuke_spell_found.ManaCost = 0;
		bool spell_selected = false;

		for (std::list<BotSpell>::iterator bot_spell_list_itr = bot_spell_list.begin(); bot_spell_list_itr != bot_spell_list.end(); ++bot_spell_list_itr) {
			if (!caster->IsValidSpellRange(bot_spell_list_itr->SpellId, target)) {
				continue;
			}

			if (select_lure_nuke && (spells[bot_spell_list_itr->SpellId].resist_difficulty < lure_resis_value)) {
				if (caster->CastChecks(bot_spell_list_itr->SpellId, target, spell_type)) {
					spell_selected = true;
				}
			}
			else if (!select_lure_nuke && IsPureNukeSpell(bot_spell_list_itr->SpellId)) {
				if (
					((target->GetMR() < target->GetCR()) || (target->GetMR() < target->GetFR())) &&
					(GetSpellResistType(bot_spell_list_itr->SpellId) == RESIST_MAGIC) &&
					(spells[bot_spell_list_itr->SpellId].resist_difficulty > lure_resis_value) &&
					caster->CastChecks(bot_spell_list_itr->SpellId, target, spell_type)
				) {
					spell_selected = true;
				}
				else if (
					((target->GetCR() < target->GetMR()) || (target->GetCR() < target->GetFR())) &&
					(GetSpellResistType(bot_spell_list_itr->SpellId) == RESIST_COLD) &&
					(spells[bot_spell_list_itr->SpellId].resist_difficulty > lure_resis_value) &&
					caster->CastChecks(bot_spell_list_itr->SpellId, target, spell_type)
				) {
					spell_selected = true;
				}
				else if (
					((target->GetFR() < target->GetCR()) || (target->GetFR() < target->GetMR())) &&
					(GetSpellResistType(bot_spell_list_itr->SpellId) == RESIST_FIRE) &&
					(spells[bot_spell_list_itr->SpellId].resist_difficulty > lure_resis_value) &&
					caster->CastChecks(bot_spell_list_itr->SpellId, target, spell_type)
				) {
					spell_selected = true;
				}
				else if (
					(GetSpellResistType(bot_spell_list_itr->SpellId) == RESIST_MAGIC) &&
					(spells[bot_spell_list_itr->SpellId].resist_difficulty > lure_resis_value) &&
					caster->CastChecks(bot_spell_list_itr->SpellId, target, spell_type)
				) {
					first_wizard_magic_nuke_spell_found.SpellId = bot_spell_list_itr->SpellId;
					first_wizard_magic_nuke_spell_found.SpellIndex = bot_spell_list_itr->SpellIndex;
					first_wizard_magic_nuke_spell_found.ManaCost = bot_spell_list_itr->ManaCost;
				}
			}

			if (spell_selected) {
				result.SpellId = bot_spell_list_itr->SpellId;
				result.SpellIndex = bot_spell_list_itr->SpellIndex;
				result.ManaCost = bot_spell_list_itr->ManaCost;

				break;
			}
		}

		if (!spell_selected) {
			for (std::list<BotSpell>::iterator bot_spell_list_itr = bot_spell_list.begin(); bot_spell_list_itr != bot_spell_list.end(); ++bot_spell_list_itr) {
				if (caster->CheckSpellRecastTimer(bot_spell_list_itr->SpellId)) {
					if (caster->CastChecks(bot_spell_list_itr->SpellId, target, spell_type)) {
						spell_selected = true;
					}
				}
				if (spell_selected) {
					result.SpellId = bot_spell_list_itr->SpellId;
					result.SpellIndex = bot_spell_list_itr->SpellIndex;
					result.ManaCost = bot_spell_list_itr->ManaCost;

					break;
				}
			}
		}

		if (result.SpellId == 0) {
			result = first_wizard_magic_nuke_spell_found;
		}
	}

	return result;
}

BotSpell Bot::GetDebuffBotSpell(Bot* caster, Mob *tar, uint16 spell_type) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (!tar || !caster)
		return result;

	if (caster->AI_HasSpells()) {
		const std::vector<BotSpells_wIndex>& bot_spell_list = caster->BotGetSpellsByType(spell_type);

		for (int i = bot_spell_list.size() - 1; i >= 0; i--) {
			if (!IsValidSpell(bot_spell_list[i].spellid)) {
				continue;
			}

			if (BotRequiresLoSToCast(spell_type, bot_spell_list[i].spellid) && !caster->HasLoS()) {
				continue;
			}

			if (((bot_spell_list[i].type == BotSpellTypes::Debuff) || IsDebuffSpell(bot_spell_list[i].spellid))
				&& (!tar->IsImmuneToSpell(bot_spell_list[i].spellid, caster)
				&& tar->CanBuffStack(bot_spell_list[i].spellid, caster->GetLevel(), true) >= 0)
				&& caster->CheckSpellRecastTimer(bot_spell_list[i].spellid)) {
				result.SpellId = bot_spell_list[i].spellid;
				result.SpellIndex = bot_spell_list[i].index;
				result.ManaCost = bot_spell_list[i].manacost;

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForResistDebuff(Bot* caster, Mob *tar, uint16 spell_type) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (!tar || !caster) {
		return result;
	}

	int level_mod = (tar->GetLevel() - caster->GetLevel())* (tar->GetLevel() - caster->GetLevel()) / 2;
	if (tar->GetLevel() - caster->GetLevel() < 0) {
		level_mod = -level_mod;
	}

	bool needs_magic_resist_debuff = (tar->GetMR() + level_mod) > 100;
	bool needs_cold_resist_debuff = (tar->GetCR() + level_mod) > 100;
	bool needs_fire_resist_debuff = (tar->GetFR() + level_mod) > 100;
	bool needs_poison_resist_debuff = (tar->GetPR() + level_mod) > 100;
	bool needs_disease_resist_debuff = (tar->GetDR() + level_mod) > 100;

	if (caster->AI_HasSpells()) {
		const std::vector<BotSpells_wIndex>& bot_spell_list = caster->BotGetSpellsByType(spell_type);

		for (int i = bot_spell_list.size() - 1; i >= 0; i--) {
			if (!IsValidSpell(bot_spell_list[i].spellid)) {
				continue;
			}

			if (BotRequiresLoSToCast(spell_type, bot_spell_list[i].spellid) && !caster->HasLoS()) {
				continue;
			}

			if (
				(bot_spell_list[i].type == BotSpellTypes::Debuff || IsResistDebuffSpell(bot_spell_list[i].spellid)) &&
				(
					(needs_magic_resist_debuff && (IsEffectInSpell(bot_spell_list[i].spellid, SpellEffect::ResistMagic) || IsEffectInSpell(bot_spell_list[i].spellid, SpellEffect::ResistAll))) ||
					(needs_cold_resist_debuff && (IsEffectInSpell(bot_spell_list[i].spellid, SpellEffect::ResistCold) || IsEffectInSpell(bot_spell_list[i].spellid, SpellEffect::ResistAll))) ||
					(needs_fire_resist_debuff && (IsEffectInSpell(bot_spell_list[i].spellid, SpellEffect::ResistFire) || IsEffectInSpell(bot_spell_list[i].spellid, SpellEffect::ResistAll))) ||
					(needs_poison_resist_debuff && (IsEffectInSpell(bot_spell_list[i].spellid, SpellEffect::ResistPoison) || IsEffectInSpell(bot_spell_list[i].spellid, SpellEffect::ResistAll))) ||
					(needs_disease_resist_debuff && (IsEffectInSpell(bot_spell_list[i].spellid, SpellEffect::ResistDisease) || IsEffectInSpell(bot_spell_list[i].spellid, SpellEffect::ResistAll)))
				) &&
				!tar->IsImmuneToSpell(bot_spell_list[i].spellid, caster) &&
				tar->CanBuffStack(bot_spell_list[i].spellid, caster->GetLevel(), true) >= 0 &&
				caster->CheckSpellRecastTimer(bot_spell_list[i].spellid)
			) {
				result.SpellId = bot_spell_list[i].spellid;
				result.SpellIndex = bot_spell_list[i].index;
				result.ManaCost = bot_spell_list[i].manacost;

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForCure(Bot* caster, Mob* tar, uint16 spell_type) {
	BotSpell_wPriority result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (!tar) {
		return result;
	}

	if (caster) {
		std::vector<BotSpell_wPriority> bot_spell_list_itr = GetPrioritizedBotSpellsBySpellType(caster, spell_type, tar);

		if (IsGroupBotSpellType(spell_type)) {
			int count_needs_cured = 0;
			uint16 count_poisoned = 0;
			uint16 count_diseased = 0;
			uint16 count_cursed = 0;
			uint16 count_corrupted = 0;

			for (std::vector<BotSpell_wPriority>::iterator itr = bot_spell_list_itr.begin(); itr != bot_spell_list_itr.end(); ++itr) {
				if (!IsValidSpell(itr->SpellId) || !IsGroupSpell(itr->SpellId)) {
					continue;
				}

				for (Mob* m : (IsGroupBotSpellType(spell_type) ? caster->GetSpellTargetList() : caster->GetSpellTargetList(true))) {
					if (caster->GetNeedsCured(m)) {
						if (caster->CastChecks(itr->SpellId, m, spell_type, true, IsGroupBotSpellType(spell_type))) {
							if (m->FindType(SpellEffect::PoisonCounter)) {
								++count_poisoned;
							}
							if (m->FindType(SpellEffect::DiseaseCounter)) {
								++count_diseased;
							}
							if (m->FindType(SpellEffect::CurseCounter)) {
								++count_cursed;
							}
							if (m->FindType(SpellEffect::CorruptionCounter)) {
								++count_corrupted;
							}
						}
					}
				}

				if (
					(count_poisoned >= caster->GetSpellTypeAEOrGroupTargetCount(spell_type) && IsEffectInSpell(itr->SpellId, SpellEffect::PoisonCounter)) ||
					(count_diseased >= caster->GetSpellTypeAEOrGroupTargetCount(spell_type) && IsEffectInSpell(itr->SpellId, SpellEffect::DiseaseCounter)) ||
					(count_cursed >= caster->GetSpellTypeAEOrGroupTargetCount(spell_type) && IsEffectInSpell(itr->SpellId, SpellEffect::CurseCounter)) ||
					(count_corrupted >= caster->GetSpellTypeAEOrGroupTargetCount(spell_type) && IsEffectInSpell(itr->SpellId, SpellEffect::CorruptionCounter))
				) {
					result.SpellId = itr->SpellId;
					result.SpellIndex = itr->SpellIndex;
					result.ManaCost = itr->ManaCost;

					break;
				}
			}
		}
		else {
			for (std::vector<BotSpell_wPriority>::iterator itr = bot_spell_list_itr.begin(); itr != bot_spell_list_itr.end(); ++itr) {
				if (!IsValidSpell(itr->SpellId) || IsGroupSpell(itr->SpellId)) {
					continue;
				}

				if (
					tar->FindType(SpellEffect::PoisonCounter) && IsEffectInSpell(itr->SpellId, SpellEffect::PoisonCounter) ||
					tar->FindType(SpellEffect::DiseaseCounter) && IsEffectInSpell(itr->SpellId, SpellEffect::DiseaseCounter) ||
					tar->FindType(SpellEffect::CurseCounter) && IsEffectInSpell(itr->SpellId, SpellEffect::CurseCounter) ||
					tar->FindType(SpellEffect::CorruptionCounter) && IsEffectInSpell(itr->SpellId, SpellEffect::CorruptionCounter)
				) {
					result.SpellId = itr->SpellId;
					result.SpellIndex = itr->SpellIndex;
					result.ManaCost = itr->ManaCost;
					break;
				}
			}
		}
	}

	return result;
}

uint8 Bot::GetChanceToCastBySpellType(uint16 spell_type)
{
	switch (spell_type) {
		case BotSpellTypes::AENukes:
		case BotSpellTypes::AERains:
		case BotSpellTypes::AEStun:
		case BotSpellTypes::AESnare:
		case BotSpellTypes::AESlow:
		case BotSpellTypes::AEDebuff:
		case BotSpellTypes::AEFear:
		case BotSpellTypes::AEDispel:
		case BotSpellTypes::AEDoT:
		case BotSpellTypes::AELifetap:
		case BotSpellTypes::AERoot:
		case BotSpellTypes::PBAENuke:
		case BotSpellTypes::AEHateLine:
			return RuleI(Bots, PercentChanceToCastAEs);
		case BotSpellTypes::GroupHeals:
		case BotSpellTypes::GroupCompleteHeals:
		case BotSpellTypes::GroupHoTHeals:
			return RuleI(Bots, PercentChanceToCastGroupHeal);
		case BotSpellTypes::Nuke:
			return RuleI(Bots, PercentChanceToCastNuke);
		case BotSpellTypes::Root:
			return RuleI(Bots, PercentChanceToCastRoot);
		case BotSpellTypes::Buff:
		case BotSpellTypes::PetBuffs:
		case BotSpellTypes::ResistBuffs:
		case BotSpellTypes::PetResistBuffs:
		case BotSpellTypes::DamageShields:
		case BotSpellTypes::PetDamageShields:
			return RuleI(Bots, PercentChanceToCastBuff);
		case BotSpellTypes::Escape:
			return RuleI(Bots, PercentChanceToCastEscape);
		case BotSpellTypes::Lifetap:
			return RuleI(Bots, PercentChanceToCastLifetap);
		case BotSpellTypes::Snare:
			return RuleI(Bots, PercentChanceToCastSnare);
		case BotSpellTypes::DOT:
			return RuleI(Bots, PercentChanceToCastDOT);
		case BotSpellTypes::Dispel:
			return RuleI(Bots, PercentChanceToCastDispel);
		case BotSpellTypes::InCombatBuff:
			return RuleI(Bots, PercentChanceToCastInCombatBuff);
		case BotSpellTypes::HateLine:
			return RuleI(Bots, PercentChanceToCastHateLine);
		case BotSpellTypes::Mez:
			return RuleI(Bots, PercentChanceToCastMez);
		case BotSpellTypes::Slow:
			return RuleI(Bots, PercentChanceToCastSlow);
		case BotSpellTypes::Debuff:
			return RuleI(Bots, PercentChanceToCastDebuff);
		case BotSpellTypes::Cure:
		case BotSpellTypes::PetCures:
			return RuleI(Bots, PercentChanceToCastCure);
		case BotSpellTypes::GroupCures:
			return RuleI(Bots, PercentChanceToCastGroupCure);
		case BotSpellTypes::HateRedux:
			return RuleI(Bots, PercentChanceToCastHateRedux);
		case BotSpellTypes::Fear:
			return RuleI(Bots, PercentChanceToCastFear);
		case BotSpellTypes::RegularHeal:
		case BotSpellTypes::CompleteHeal:
		case BotSpellTypes::FastHeals:
		case BotSpellTypes::VeryFastHeals:
		case BotSpellTypes::HoTHeals:
		case BotSpellTypes::PetRegularHeals:
		case BotSpellTypes::PetCompleteHeals:
		case BotSpellTypes::PetFastHeals:
		case BotSpellTypes::PetVeryFastHeals:
		case BotSpellTypes::PetHoTHeals:
			return RuleI(Bots, PercentChanceToCastHeal);
		case BotSpellTypes::AEMez:
			return RuleI(Bots, PercentChanceToCastAEMez);
		default:
			return RuleI(Bots, PercentChanceToCastOtherType);
	}

	return RuleI(Bots, PercentChanceToCastOtherType);
}

bool Bot::AI_AddBotSpells(uint32 bot_spell_id) {
	// ok, this function should load the list, and the parent list then shove them into the struct and sort
	npc_spells_id = bot_spell_id;
	AIBot_spells.clear();
	AIBot_spells_enforced.clear();
	AIBot_spells_by_type.clear();

	if (!bot_spell_id) {
		AIautocastspell_timer->Disable();
		return false;
	}

	auto* spell_list = content_db.GetBotSpells(bot_spell_id);
	if (!spell_list) {
		AIautocastspell_timer->Disable();
		return false;
	}

	auto* parentlist = content_db.GetBotSpells(spell_list->parent_list);

	auto debug_msg = fmt::format(
		"Loading Bot spells onto {}: dbspellsid={}, level={}",
		GetName(),
		bot_spell_id,
		GetLevel()
	);

	debug_msg.append(
		fmt::format(
			" (found, {})",
			spell_list->entries.size()
		)
	);

	LogAI("[{}]", debug_msg);
	for (const auto &iter: spell_list->entries) {
		LogAIDetail("([{}]) [{}]", iter.spellid, spells[iter.spellid].name);
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
			if (
				EQ::ValueWithin(GetLevel(), e.minlevel, e.maxlevel) &&
				e.spellid &&
				!IsSpellInBotList(spell_list, e.spellid)
			) {
				if (!e.bucket_name.empty() && !e.bucket_value.empty()) {
					if (!CheckDataBucket(e.bucket_name, e.bucket_value, e.bucket_comparison)) {
						continue;
					}
				}

				const auto& bs = GetBotSpellSetting(e.spellid);
				if (bs) {
					if (!bs->is_enabled) {
						continue;
					}

					AddSpellToBotList(
						bs->priority,
						e.spellid,
						e.type,
						e.manacost,
						e.recast_delay,
						e.resist_adjust,
						e.minlevel,
						e.maxlevel,
						bs->min_hp,
						bs->max_hp,
						e.bucket_name,
						e.bucket_value,
						e.bucket_comparison
					);
					continue;
				}

				if (!GetBotEnforceSpellSetting()) {
					AddSpellToBotList(
						e.priority,
						e.spellid,
						e.type,
						e.manacost,
						e.recast_delay,
						e.resist_adjust,
						e.minlevel,
						e.maxlevel,
						e.min_hp,
						e.max_hp,
						e.bucket_name,
						e.bucket_value,
						e.bucket_comparison
					);
				} else {
					AddSpellToBotEnforceList(
						e.priority,
						e.spellid,
						e.type,
						e.manacost,
						e.recast_delay,
						e.resist_adjust,
						e.minlevel,
						e.maxlevel,
						e.min_hp,
						e.max_hp,
						e.bucket_name,
						e.bucket_value,
						e.bucket_comparison
					);
				}
			}
		}
	}

	attack_proc_spell = spell_list->attack_proc;
	proc_chance = spell_list->proc_chance;

	range_proc_spell = spell_list->range_proc;
	rproc_chance = spell_list->rproc_chance;

	defensive_proc_spell = spell_list->defensive_proc;
	dproc_chance = spell_list->dproc_chance;

	//If any casting variables are defined in the current list, ignore those in the parent list.
	if (
		spell_list->fail_recast ||
		spell_list->engaged_no_sp_recast_min ||
		spell_list->engaged_no_sp_recast_max ||
		spell_list->engaged_beneficial_self_chance ||
		spell_list->engaged_beneficial_other_chance ||
		spell_list->engaged_detrimental_chance ||
		spell_list->pursue_no_sp_recast_min ||
		spell_list->pursue_no_sp_recast_max ||
		spell_list->pursue_detrimental_chance ||
		spell_list->idle_no_sp_recast_min ||
		spell_list->idle_no_sp_recast_max ||
		spell_list->idle_beneficial_chance
	) {
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
		if (EQ::ValueWithin(GetLevel(), e.minlevel, e.maxlevel) && e.spellid) {
			if (!e.bucket_name.empty() && !e.bucket_value.empty()) {
				if (!CheckDataBucket(e.bucket_name, e.bucket_value, e.bucket_comparison)) {
					continue;
				}
			}

			const auto& bs = GetBotSpellSetting(e.spellid);
			if (bs) {
				if (!bs->is_enabled) {
					continue;
				}

				AddSpellToBotList(
					bs->priority,
					e.spellid,
					e.type,
					e.manacost,
					e.recast_delay,
					e.resist_adjust,
					e.minlevel,
					e.maxlevel,
					bs->min_hp,
					bs->max_hp,
					e.bucket_name,
					e.bucket_value,
					e.bucket_comparison
				);
				continue;
			}

			if (!GetBotEnforceSpellSetting()) {
				AddSpellToBotList(
					e.priority,
					e.spellid,
					e.type,
					e.manacost,
					e.recast_delay,
					e.resist_adjust,
					e.minlevel,
					e.maxlevel,
					e.min_hp,
					e.max_hp,
					e.bucket_name,
					e.bucket_value,
					e.bucket_comparison
				);
			} else {
				AddSpellToBotEnforceList(
					e.priority,
					e.spellid,
					e.type,
					e.manacost,
					e.recast_delay,
					e.resist_adjust,
					e.minlevel,
					e.maxlevel,
					e.min_hp,
					e.max_hp,
					e.bucket_name,
					e.bucket_value,
					e.bucket_comparison
				);
			}
		}
	}

	std::sort(AIBot_spells.begin(), AIBot_spells.end(), [](const BotSpells& a, const BotSpells& b) {
		return a.priority > b.priority;
	});

	if (IsValidSpell(attack_proc_spell)) {
		AddProcToWeapon(attack_proc_spell, true, proc_chance);

		if (RuleB(Spells, NPCInnateProcOverride)) {
			innate_proc_spell_id = attack_proc_spell;
		}
	}

	if (IsValidSpell(range_proc_spell)) {
		AddRangedProc(range_proc_spell, (rproc_chance + 100));
	}

	if (IsValidSpell(defensive_proc_spell)) {
		AddDefensiveProc(defensive_proc_spell, (dproc_chance + 100));
	}

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

	if (AIBot_spells.empty()) {
		AIautocastspell_timer->Disable();
	} else {
		AIautocastspell_timer->Trigger();
		AssignBotSpellsToTypes(AIBot_spells, AIBot_spells_by_type); // Assign AIBot_spells to AIBot_spells_by_type with an index
	}
	return true;
}

bool IsSpellInBotList(DBbotspells_Struct* spell_list, uint16 iSpellID) {
	auto it = std::find_if (
		spell_list->entries.begin(),
		spell_list->entries.end(),
		[iSpellID](const DBbotspells_entries_Struct &a) {
			return a.spellid == iSpellID;
		}
	);

	return it != spell_list->entries.end();
}

DBbotspells_Struct* ZoneDatabase::GetBotSpells(uint32 bot_spell_id)
{
	if (!bot_spell_id) {
		return nullptr;
	}

	auto c = bot_spells_cache.find(bot_spell_id);
	if (c != bot_spells_cache.end()) { // it's in the cache, easy =)
		return &c->second;
	}

	if (!bot_spells_loadtried.count(bot_spell_id)) { // no reason to ask the DB again if we have failed once already
		bot_spells_loadtried.insert(bot_spell_id);

		auto n = NpcSpellsRepository::FindOne(content_db, bot_spell_id);
		if (!n.id) {
			return nullptr;
		}

		DBbotspells_Struct spell_set;

		spell_set.parent_list = n.parent_list;
		spell_set.attack_proc = n.attack_proc;
		spell_set.proc_chance = n.proc_chance;
		spell_set.range_proc = n.range_proc;
		spell_set.rproc_chance = n.rproc_chance;
		spell_set.defensive_proc = n.defensive_proc;
		spell_set.dproc_chance = n.dproc_chance;
		spell_set.fail_recast = n.fail_recast;
		spell_set.engaged_no_sp_recast_min = n.engaged_no_sp_recast_min;
		spell_set.engaged_no_sp_recast_max = n.engaged_no_sp_recast_max;
		spell_set.engaged_beneficial_self_chance = n.engaged_b_self_chance;
		spell_set.engaged_beneficial_other_chance = n.engaged_b_other_chance;
		spell_set.engaged_detrimental_chance = n.engaged_d_chance;
		spell_set.pursue_no_sp_recast_min = n.pursue_no_sp_recast_min;
		spell_set.pursue_no_sp_recast_max = n.pursue_no_sp_recast_max;
		spell_set.pursue_detrimental_chance = n.pursue_d_chance;
		spell_set.idle_no_sp_recast_min = n.idle_no_sp_recast_min;
		spell_set.idle_no_sp_recast_max = n.idle_no_sp_recast_max;
		spell_set.idle_beneficial_chance = n.idle_b_chance;

		auto bse = BotSpellsEntriesRepository::GetWhere(
			content_db,
			fmt::format(
				"npc_spells_id = {}",
				bot_spell_id
			)
		);

		if (!bse.empty()) {
			for (const auto& e : bse) {
				DBbotspells_entries_Struct entry;

				entry.spellid           = e.spell_id;
				entry.type              = e.type;
				entry.minlevel          = e.minlevel;
				entry.maxlevel          = e.maxlevel;
				entry.manacost          = e.manacost;
				entry.recast_delay      = e.recast_delay;
				entry.priority          = e.priority;
				entry.min_hp            = e.min_hp;
				entry.max_hp            = e.max_hp;
				entry.resist_adjust     = e.resist_adjust;
				entry.bucket_name       = e.bucket_name;
				entry.bucket_value      = e.bucket_value;
				entry.bucket_comparison = e.bucket_comparison;

				// some spell types don't make much since to be priority 0, so fix that
				if (!IsBotSpellTypeInnate(entry.type) && entry.priority == 0) {
					entry.priority = 1;
				}

				if (e.resist_adjust) {
					entry.resist_adjust = e.resist_adjust;
				} else if (IsValidSpell(e.spell_id)) {
					entry.resist_adjust = spells[e.spell_id].resist_difficulty;
				}

				spell_set.entries.push_back(entry);
			}
		}

		bot_spells_cache.emplace(std::make_pair(bot_spell_id, spell_set));

		return &bot_spells_cache[bot_spell_id];
	}

	return nullptr;
}

// adds a spell to the list, taking into account priority and resorting list as needed.
void Bot::AddSpellToBotList(
	int16 in_priority,
	uint16 in_spell_id,
	uint32 in_type,
	int16 in_mana_cost,
	int32 in_recast_delay,
	int16 in_resist_adjust,
	uint8 in_min_level,
	uint8 in_max_level,
	int8 in_min_hp,
	int8 in_max_hp,
	std::string in_bucket_name,
	std::string in_bucket_value,
	uint8 in_bucket_comparison
) {
	if (!IsValidSpell(in_spell_id)) {
		return;
	}

	HasAISpell = true;
	BotSpells t;

	t.priority          = in_priority;
	t.spellid           = in_spell_id;
	t.type              = in_type;
	t.manacost          = in_mana_cost;
	t.recast_delay      = in_recast_delay;
	t.time_cancast      = 0;
	t.resist_adjust     = in_resist_adjust;
	t.minlevel          = in_min_level;
	t.maxlevel          = in_max_level;
	t.min_hp            = in_min_hp;
	t.max_hp            = in_max_hp;
	t.bucket_name       = in_bucket_name;
	t.bucket_value      = in_bucket_value;
	t.bucket_comparison = in_bucket_comparison;

	AIBot_spells.push_back(t);

	// If we're going from an empty list, we need to start the timer
	if (AIBot_spells.empty()) {
		AIautocastspell_timer->Start(RandomTimer(0, 300), false);
	}
}

// adds spells to the list ^spells that are returned if ^enforce is enabled
void Bot::AddSpellToBotEnforceList(
	int16 iPriority,
	uint16 iSpellID,
	uint32 iType,
	int16 iManaCost,
	int32 iRecastDelay,
	int16 iResistAdjust,
	uint8 min_level,
	uint8 max_level,
	int8 min_hp,
	int8 max_hp,
	std::string bucket_name,
	std::string bucket_value,
	uint8 bucket_comparison
) {
	if (!IsValidSpell(iSpellID)) {
		return;
	}

	HasAISpell = true;
	BotSpells t;

	t.priority			= iPriority;
	t.spellid			= iSpellID;
	t.type				= iType;
	t.manacost			= iManaCost;
	t.recast_delay		= iRecastDelay;
	t.time_cancast		= 0;
	t.resist_adjust		= iResistAdjust;
	t.minlevel			= min_level;
	t.maxlevel			= maxlevel;
	t.min_hp			= min_hp;
	t.max_hp			= max_hp;
	t.bucket_name		= bucket_name;
	t.bucket_value		= bucket_value;
	t.bucket_comparison	= bucket_comparison;

	AIBot_spells_enforced.push_back(t);
}

//this gets called from InterruptSpell() for failure or SpellFinished() for success
void Bot::AI_Bot_Event_SpellCastFinished(bool iCastSucceeded, uint16 slot) {
	if (slot == 1) {
		uint32 recovery_time = 0;
		if (iCastSucceeded) {
			if (casting_spell_AIindex < AIBot_spells.size()) {
				recovery_time += spells[AIBot_spells[casting_spell_AIindex].spellid].recovery_time;
				if (AIBot_spells[casting_spell_AIindex].recast_delay >= 0) {
					if (AIBot_spells[casting_spell_AIindex].recast_delay < 10000) {
						AIBot_spells[casting_spell_AIindex].time_cancast = Timer::GetCurrentTime() + (AIBot_spells[casting_spell_AIindex].recast_delay*1000);
					}
				}
				else {
					AIBot_spells[casting_spell_AIindex].time_cancast = Timer::GetCurrentTime() + spells[AIBot_spells[casting_spell_AIindex].spellid].recast_time;
				}
			}
			if (recovery_time < AIautocastspell_timer->GetSetAtTrigger()) {
				recovery_time = AIautocastspell_timer->GetSetAtTrigger();
			}
			AIautocastspell_timer->Start(recovery_time, false);
		}
		else {
			AIautocastspell_timer->Start(AISpellVar.fail_recast, false);
		}
		casting_spell_AIindex = AIBot_spells.size();
	}
}

bool Bot::HasBotSpellEntry(uint16 spell_id) {
	auto* spell_list = content_db.GetBotSpells(GetBotSpellID());

	if (!spell_list) {
		return false;
	}

	// Check if Spell ID is found in Bot Spell Entries
	for (auto& e : spell_list->entries) {
		if (spell_id == e.spellid) {
			return true;
		}
	}

	return false;
}

bool Bot::CanUseBotSpell(uint16 spell_id) {
	if (AIBot_spells.empty()) {
		return false;
	}

	for (const auto& s : AIBot_spells) {
		if (!IsValidSpell(s.spellid)) {
			return false;
		}

		if (s.spellid != spell_id) {
			continue;
		}

		if (s.minlevel > GetLevel()) {
			return false;
		}

		return true;
	}

	return false;
}

bool Bot::IsValidSpellRange(uint16 spell_id, Mob* tar) {
	if (!IsValidSpell(spell_id) || !tar) {
		return false;
	}

	float range = spells[spell_id].range + GetRangeDistTargetSizeMod(tar);

	if (
		spells[spell_id].target_type != ST_AETargetHateList &&
		!IsTargetableAESpell(spell_id) &&
		IsAnyAESpell(spell_id)
	) {
		range = GetAOERange(spell_id);
	}

	if (RuleB(Bots, EnableBotTGB) && IsTGBCompatibleSpell(spell_id) && IsGroupSpell(spell_id)) {
		range = spells[spell_id].aoe_range;
	}

	range = GetActSpellRange(spell_id, range);

	if (HasProjectIllusion() && IsIllusionSpell(spell_id)) {
		range = 100;
	}

	float dist2 = DistanceSquared(m_Position, tar->GetPosition());
	float range2 = range * range;
	float min_range2 = spells[spell_id].min_range * spells[spell_id].min_range;

	if (dist2 > range2) {
		//target is out of range.
		return false;
	}
	else if (dist2 < min_range2) {
		//target is too close range.
		return false;
	}

	return true;
}

BotSpell Bot::GetBestBotSpellForNukeByBodyType(Bot* caster, uint8 body_type, uint16 spell_type, bool AE, Mob* tar) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (!caster || !body_type) {
		return result;
	}

	if (tar == nullptr) {
		tar = caster->GetTarget();
	}

	switch (body_type) {
		case BodyType::Undead:
		case BodyType::SummonedUndead:
		case BodyType::Vampire:
			result = GetBestBotSpellForNukeByTargetType(caster, (!AE ? ST_Undead : ST_UndeadAE), spell_type, AE, tar);
			break;
		case BodyType::Summoned:
		case BodyType::Summoned2:
		case BodyType::Summoned3:
			result = GetBestBotSpellForNukeByTargetType(caster, (!AE ? ST_Summoned : ST_SummonedAE), spell_type, AE, tar);
			break;
		case BodyType::Animal:
			result = GetBestBotSpellForNukeByTargetType(caster, ST_Animal, spell_type, AE, tar);
			break;
		case BodyType::Plant:
			result = GetBestBotSpellForNukeByTargetType(caster, ST_Plant, spell_type, AE, tar);
			break;
		case BodyType::Giant:
			result = GetBestBotSpellForNukeByTargetType(caster, ST_Giant, spell_type, AE, tar);
			break;
		case BodyType::Dragon:
			result = GetBestBotSpellForNukeByTargetType(caster, ST_Dragon, spell_type, AE, tar);
			break;
		default:
			break;
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForRez(Bot* caster, Mob* target, uint16 spell_type) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (caster) {
		std::list<BotSpell> bot_spell_list = GetBotSpellsForSpellEffect(caster, spell_type, SpellEffect::Revive);

		for (std::list<BotSpell>::iterator bot_spell_list_itr = bot_spell_list.begin(); bot_spell_list_itr != bot_spell_list.end(); ++bot_spell_list_itr) {
			if (
				IsResurrectSpell(bot_spell_list_itr->SpellId) &&
				caster->CheckSpellRecastTimer(bot_spell_list_itr->SpellId)
			) {
				result.SpellId = bot_spell_list_itr->SpellId;
				result.SpellIndex = bot_spell_list_itr->SpellIndex;
				result.ManaCost = bot_spell_list_itr->ManaCost;

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForCharm(Bot* caster, Mob* target, uint16 spell_type) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (caster) {
		std::list<BotSpell> bot_spell_list = GetBotSpellsForSpellEffect(caster, spell_type, SpellEffect::Charm);

		for (std::list<BotSpell>::iterator bot_spell_list_itr = bot_spell_list.begin(); bot_spell_list_itr != bot_spell_list.end(); ++bot_spell_list_itr) {
			if (
				IsCharmSpell(bot_spell_list_itr->SpellId) &&
				caster->CastChecks(bot_spell_list_itr->SpellId, target, spell_type)
			) {
				result.SpellId = bot_spell_list_itr->SpellId;
				result.SpellIndex = bot_spell_list_itr->SpellIndex;
				result.ManaCost = bot_spell_list_itr->ManaCost;

				break;
			}
		}
	}

	return result;
}
