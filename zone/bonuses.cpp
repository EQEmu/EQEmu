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
#include "../common/classes.h"
#include "../common/data_verification.h"
#include "../common/global_define.h"
#include "../common/item_instance.h"
#include "../common/rulesys.h"
#include "../common/spdat.h"

#include "client.h"
#include "entity.h"
#include "mob.h"

#include "bot.h"

#include "quest_parser_collection.h"


#ifndef WIN32
#include <stdlib.h>
#include "../common/unix.h"
#endif


void Mob::CalcBonuses()
{
	CalcSpellBonuses(&spellbonuses);
	CalcAABonuses(&aabonuses);
	CalcMaxHP();
	CalcMaxMana();
	SetAttackTimer();
	CalcAC();
	CalcSeeInvisibleLevel();
	CalcInvisibleLevel();

	/* Fast walking NPC's are prone to disappear into walls/hills
		We set this here because NPC's can cast spells to change walkspeed/runspeed
	*/
	float get_walk_speed = static_cast<float>(0.025f * GetWalkspeed());
	rooted = FindType(SpellEffect::Root);
}

void NPC::CalcBonuses()
{
	memset(&itembonuses, 0, sizeof(StatBonuses));

	if (GetOwner() || RuleB(NPC, UseItemBonusesForNonPets)) {
		CalcItemBonuses(&itembonuses);
	}

	// This has to happen last, so we actually take the item bonuses into account.
	Mob::CalcBonuses();
}

void Client::CalcBonuses()
{
	memset(&itembonuses, 0, sizeof(StatBonuses));
	CalcItemBonuses(&itembonuses);
	CalcHeroicBonuses(&itembonuses);
	CalcEdibleBonuses(&itembonuses);
	CalcSpellBonuses(&spellbonuses);
	CalcAABonuses(&aabonuses);

	CalcSeeInvisibleLevel();
	CalcInvisibleLevel();

	ProcessItemCaps(); // caps that depend on spell/aa bonuses

	RecalcWeight();

	CalcAC();
	CalcATK();
	CalcHaste();

	CalcSTR();
	CalcSTA();
	CalcDEX();
	CalcAGI();
	CalcINT();
	CalcWIS();
	CalcCHA();

	CalcMR();
	CalcFR();
	CalcDR();
	CalcPR();
	CalcCR();
	CalcCorrup();

	CalcMaxHP();
	CalcMaxMana();
	CalcMaxEndurance();

	SetAttackTimer();

	rooted = FindType(SpellEffect::Root);

	XPRate = 100 + spellbonuses.XPRateMod;

	if (GetMaxXTargets() != 5 + aabonuses.extra_xtargets)
		SetMaxXTargets(5 + aabonuses.extra_xtargets);

	// hmm maybe a better way to do this
	int metabolism = spellbonuses.Metabolism + itembonuses.Metabolism + aabonuses.Metabolism;
	int timer = GetClass() == Class::Monk ? CONSUMPTION_MNK_TIMER : CONSUMPTION_TIMER;
	timer = timer * (100 + metabolism) / 100;
	if (timer != consume_food_timer.GetTimerTime())
		consume_food_timer.SetTimer(timer);
}

int Mob::CalcRecommendedLevelBonus(uint8 current_level, uint8 recommended_level, int base_stat)
{
	if (recommended_level && current_level < recommended_level) {
		int32 stat_modifier = (current_level * 10000 / recommended_level) * base_stat;

		stat_modifier += stat_modifier < 0 ? -5000 : 5000;

		return (stat_modifier / 10000);
	}

	return 0;
}

void Mob::CalcItemBonuses(StatBonuses* b) {
	ClearItemFactionBonuses();
	SetShieldEquipped(false);
	SetTwoHandBluntEquipped(false);
	SetTwoHanderEquipped(false);
	SetDualWeaponsEquipped(false);

	int16 i;

	for (i = EQ::invslot::BONUS_BEGIN; i <= EQ::invslot::BONUS_SKILL_END; i++) {
		const auto* inst = GetInv().GetItem(i);

		if (!inst) {
			continue;
		}

		AddItemBonuses(inst, b, false, false, 0, (i == EQ::invslot::slotAmmo));

		//These are given special flags due to how often they are checked for various spell effects.
		const auto* item = inst->GetItem();
		if (
			item &&
			item->ItemType == EQ::item::ItemTypeShield &&
			i == EQ::invslot::slotSecondary
		) {
			SetShieldEquipped(true);
		} else if (
			item &&
			item->ItemType == EQ::item::ItemType2HBlunt &&
			i == EQ::invslot::slotPrimary
		) {
			SetTwoHandBluntEquipped(true);
			SetTwoHanderEquipped(true);
		} else if (
			item &&
			(item->ItemType == EQ::item::ItemType2HSlash || item->ItemType == EQ::item::ItemType2HPiercing) &&
			i == EQ::invslot::slotPrimary
		) {
			SetTwoHanderEquipped(true);
		}
	}

	if (CanThisClassDualWield()) {
		SetDualWeaponsEquipped(true);
	}

	if (IsClient()) {
		if (CastToClient()->GetPP().tribute_active) {
			for (auto const &t: CastToClient()->GetPP().tributes) {
				auto item_id = CastToClient()->LookupTributeItemID(t.tribute, t.tier);
				if (item_id) {
					const EQ::ItemInstance *inst = database.CreateItem(item_id);
					if (!inst) {
						continue;
					}

					AddItemBonuses(inst, b, false, true);
					safe_delete(inst);
				}
			}
		}
	}

	if (IsOfClientBot()) {
		for (i = EQ::invslot::GUILD_TRIBUTE_BEGIN; i <= EQ::invslot::GUILD_TRIBUTE_END; i++) {
			const auto* inst = GetInv().GetItem(i);
			if (!inst) {
				continue;
			}

			AddItemBonuses(inst, b, false, true);
		}
	}

	if (
		RuleI(Spells, AdditiveBonusWornType) &&
		RuleI(Spells, AdditiveBonusWornType) != EQ::item::ItemEffectWorn
	) {
		for (i = EQ::invslot::BONUS_BEGIN; i <= EQ::invslot::BONUS_STAT_END; i++) {
			const EQ::ItemInstance* inst = m_inv[i];
			if (!inst) {
				continue;
			}

			AdditiveWornBonuses(inst, b);
		}
	}

	if (IsMerc()) {
		SetAttackTimer();
	}
}

// These item stat caps depend on spells/AAs so we process them after those are processed
void Mob::ProcessItemCaps()
{
	itembonuses.HPRegen = std::min(itembonuses.HPRegen, CalcHPRegenCap());
	itembonuses.ManaRegen = std::min(itembonuses.ManaRegen, CalcManaRegenCap());
	itembonuses.EnduranceRegen = std::min(itembonuses.EnduranceRegen, CalcEnduranceRegenCap());

	// The Sleeper Tomb Avatar proc counts towards item ATK
	// The client uses a 100 here, so using a 100 here the client and server will agree
	// For example, if you set the effect to be 200 it will get 100 item ATK and 100 spell ATK
	if (IsValidSpell(SPELL_AVATAR_ST_PROC) && FindBuff(SPELL_AVATAR_ST_PROC)) {
		itembonuses.ATK += 100;
		spellbonuses.ATK -= 100;
	}

	itembonuses.ATK = std::min(itembonuses.ATK, CalcItemATKCap());

	if (IsOfClientBotMerc() && itembonuses.SpellDmg > RuleI(Character, ItemSpellDmgCap)) {
		itembonuses.SpellDmg = RuleI(Character, ItemSpellDmgCap);
	}

	if (IsOfClientBotMerc() && itembonuses.HealAmt > RuleI(Character, ItemHealAmtCap)) {
		itembonuses.HealAmt = RuleI(Character, ItemHealAmtCap);
	}
}

void Mob::AddItemBonuses(const EQ::ItemInstance* inst, StatBonuses* b, bool is_augment, bool is_tribute, int recommended_level_override, bool is_ammo_item) {
	if (!inst || !inst->IsClassCommon()) {
		return;
	}

	if (inst->GetAugmentType() == 0 && is_augment) {
		return;
	}

	const auto* item = inst->GetItem();
	if (!item) {
		return;
	}

	if (IsClient() && !is_tribute && !inst->IsEquipable(GetBaseRace(), GetClass())) {
		if (item->ItemType != EQ::item::ItemTypeFood && item->ItemType != EQ::item::ItemTypeDrink) {
			return;
		}
	}

	const auto current_level = GetLevel();

	if (IsClient() && current_level < inst->GetItemRequiredLevel(true)) {
		return;
	}

	if (is_ammo_item) {
		return;
	}

	const auto recommended_level = is_augment ? recommended_level_override : inst->GetItemRecommendedLevel(true);
	const bool meets_recommended = (IsNPC() && !RuleB(Items, NPCUseRecommendedLevels)) || current_level >= recommended_level;

	auto CalcItemBonus = [&](int statValue) -> int {
		return meets_recommended ? statValue : CalcRecommendedLevelBonus(current_level, recommended_level, statValue);
	};

	auto CalcCappedItemBonus = [&](int currentStat, int bonus, int cap) -> int {
		int calc_stat = currentStat + CalcItemBonus(bonus);
		return IsOfClientBotMerc() ? std::min(cap, calc_stat) : calc_stat;
	};

	b->HP += CalcItemBonus(item->HP);
	b->Mana += CalcItemBonus(item->Mana);
	b->Endurance += CalcItemBonus(item->Endur);
	b->AC += CalcItemBonus(item->AC);

	b->STR += CalcItemBonus(item->AStr + item->HeroicStr);
	b->STA += CalcItemBonus(item->ASta + item->HeroicSta);
	b->DEX += CalcItemBonus(item->ADex + item->HeroicDex);
	b->AGI += CalcItemBonus(item->AAgi + item->HeroicAgi);
	b->INT += CalcItemBonus(item->AInt + item->HeroicInt);
	b->WIS += CalcItemBonus(item->AWis + item->HeroicWis);
	b->CHA += CalcItemBonus(item->ACha + item->HeroicCha);

	b->HeroicSTR += CalcItemBonus(item->HeroicStr);
	b->HeroicSTA += CalcItemBonus(item->HeroicSta);
	b->HeroicDEX += CalcItemBonus(item->HeroicDex);
	b->HeroicAGI += CalcItemBonus(item->HeroicAgi);
	b->HeroicINT += CalcItemBonus(item->HeroicInt);
	b->HeroicWIS += CalcItemBonus(item->HeroicWis);
	b->HeroicCHA += CalcItemBonus(item->HeroicCha);

	b->STRCapMod += item->HeroicStr;
	b->STACapMod += item->HeroicSta;
	b->DEXCapMod += item->HeroicDex;
	b->AGICapMod += item->HeroicAgi;
	b->INTCapMod += item->HeroicInt;
	b->WISCapMod += item->HeroicWis;
	b->CHACapMod += item->HeroicCha;

	b->MR += CalcItemBonus(item->MR + item->HeroicMR);
	b->FR += CalcItemBonus(item->FR + item->HeroicFR);
	b->CR += CalcItemBonus(item->CR + item->HeroicCR);
	b->PR += CalcItemBonus(item->PR + item->HeroicPR);
	b->DR += CalcItemBonus(item->DR + item->HeroicDR);
	b->Corrup += CalcItemBonus(item->SVCorruption + item->HeroicSVCorrup);

	b->HeroicMR += CalcItemBonus(item->HeroicMR);
	b->HeroicFR += CalcItemBonus(item->HeroicFR);
	b->HeroicCR += CalcItemBonus(item->HeroicCR);
	b->HeroicPR += CalcItemBonus(item->HeroicPR);
	b->HeroicDR += CalcItemBonus(item->HeroicDR);
	b->HeroicCorrup += CalcItemBonus(item->HeroicSVCorrup);

	b->MRCapMod += item->HeroicMR;
	b->FRCapMod += item->HeroicFR;
	b->CRCapMod += item->HeroicCR;
	b->PRCapMod += item->HeroicPR;
	b->DRCapMod += item->HeroicDR;
	b->CorrupCapMod += item->HeroicSVCorrup;

	b->HPRegen += CalcItemBonus(item->Regen);
	b->ManaRegen += CalcItemBonus(item->ManaRegen);
	b->EnduranceRegen += CalcItemBonus(item->EnduranceRegen);

	// These have rule-configured caps.
	b->ATK              = CalcCappedItemBonus(b->ATK, item->Attack, RuleI(Character, ItemATKCap) + itembonuses.ItemATKCap + spellbonuses.ItemATKCap + aabonuses.ItemATKCap);
	b->DamageShield     = CalcCappedItemBonus(b->DamageShield, item->DamageShield, RuleI(Character, ItemDamageShieldCap));
	b->SpellShield      = CalcCappedItemBonus(b->SpellShield, item->SpellShield, RuleI(Character, ItemSpellShieldingCap));
	b->MeleeMitigation  = CalcCappedItemBonus(b->MeleeMitigation, item->Shielding, RuleI(Character, ItemShieldingCap));
	b->StunResist       = CalcCappedItemBonus(b->StunResist, item->StunResist, RuleI(Character, ItemStunResistCap));
	b->StrikeThrough    = CalcCappedItemBonus(b->StrikeThrough, item->StrikeThrough, RuleI(Character, ItemStrikethroughCap));
	b->AvoidMeleeChance = CalcCappedItemBonus(b->AvoidMeleeChance, item->Avoidance, RuleI(Character, ItemAvoidanceCap));
	b->HitChance        = CalcCappedItemBonus(b->HitChance, item->Accuracy, RuleI(Character, ItemAccuracyCap));
	b->ProcChance       = CalcCappedItemBonus(b->ProcChance, item->CombatEffects, RuleI(Character, ItemCombatEffectsCap));
	b->DoTShielding     = CalcCappedItemBonus(b->DoTShielding, item->DotShielding, RuleI(Character, ItemDoTShieldingCap));
	b->HealAmt          = CalcCappedItemBonus(b->HealAmt, item->HealAmt, RuleI(Character, ItemHealAmtCap));
	b->SpellDmg         = CalcCappedItemBonus(b->SpellDmg, item->SpellDmg, RuleI(Character, ItemSpellDmgCap));
	b->Clairvoyance     = CalcCappedItemBonus(b->Clairvoyance, item->Clairvoyance, RuleI(Character, ItemClairvoyanceCap));
	b->DSMitigation     = CalcCappedItemBonus(b->DSMitigation, item->DSMitigation, RuleI(Character, ItemDSMitigationCap));

	if (b->haste < item->Haste) {
		b->haste = item->Haste;
	}

	if (item->ExtraDmgAmt != 0 && item->ExtraDmgSkill <= EQ::skills::HIGHEST_SKILL) {
		if (item->ExtraDmgSkill == ALL_SKILLS) {
			for (const auto &skill_id: EQ::skills::GetExtraDamageSkills()) {
				b->SkillDamageAmount[skill_id] = CalcCappedItemBonus(b->SkillDamageAmount[skill_id], item->ExtraDmgAmt, RuleI(Character, ItemExtraDmgCap));
			}
		} else {
			b->SkillDamageAmount[item->ExtraDmgSkill] = CalcCappedItemBonus(b->SkillDamageAmount[item->ExtraDmgSkill], item->ExtraDmgAmt, RuleI(Character, ItemExtraDmgCap));
		}
	}

	if (item->Worn.Effect > 0 && item->Worn.Type == EQ::item::ItemEffectWorn) {
		ApplySpellsBonuses(item->Worn.Effect, item->Worn.Level, b, 0, item->Worn.Type);
	}

	if (item->Focus.Effect > 0 && item->Focus.Type == EQ::item::ItemEffectFocus) {
		if (
			IsOfClientBotMerc() ||
			(IsNPC() && RuleB(Spells, NPC_UseFocusFromItems))
		) {
			ApplySpellsBonuses(item->Focus.Effect, item->Focus.Level, b, 0);
		}
	}

	switch (item->BardType) {
		case EQ::item::ItemTypeAllInstrumentTypes: { // (e.g. Singing Short Sword)
			if (item->BardValue > b->singingMod) {
				b->singingMod = item->BardValue;
			}

			if (item->BardValue > b->brassMod) {
				b->brassMod = item->BardValue;
			}

			if (item->BardValue > b->stringedMod) {
				b->stringedMod = item->BardValue;
			}

			if (item->BardValue > b->percussionMod) {
				b->percussionMod = item->BardValue;
			}

			if (item->BardValue > b->windMod) {
				b->windMod = item->BardValue;
			}

			break;
		}
		case EQ::item::ItemTypeSinging: {
			if (item->BardValue > b->singingMod) {
				b->singingMod = item->BardValue;
			}

			break;
		}
		case EQ::item::ItemTypeWindInstrument: {
			if (item->BardValue > b->windMod) {
				b->windMod = item->BardValue;
			}

			break;
		}
		case EQ::item::ItemTypeStringedInstrument: {
			if (item->BardValue > b->stringedMod) {
				b->stringedMod = item->BardValue;
			}

			break;
		}
		case EQ::item::ItemTypeBrassInstrument: {
			if (item->BardValue > b->brassMod) {
				b->brassMod = item->BardValue;
			}

			break;
		}
		case EQ::item::ItemTypePercussionInstrument: {
			if (item->BardValue > b->percussionMod) {
				b->percussionMod = item->BardValue;
			}

			break;
		}
	}

	if (item->SkillModValue != 0 && item->SkillModType <= EQ::skills::HIGHEST_SKILL) {
		if (
			(item->SkillModValue > 0 && b->skillmod[item->SkillModType] < item->SkillModValue) ||
			(item->SkillModValue < 0 && b->skillmod[item->SkillModType] > item->SkillModValue)
			) {
			b->skillmod[item->SkillModType] = item->SkillModValue;
		}
	}

	if (item->FactionMod1) {
		if (item->FactionAmt1 > 0 && item->FactionAmt1 > GetItemFactionBonus(item->FactionMod1)) {
			AddItemFactionBonus(item->FactionMod1, item->FactionAmt1);
		} else if (item->FactionAmt1 < 0 && item->FactionAmt1 < GetItemFactionBonus(item->FactionMod1)) {
			AddItemFactionBonus(item->FactionMod1, item->FactionAmt1);
		}
	}

	if (item->FactionMod2) {
		if (item->FactionAmt2 > 0 && item->FactionAmt2 > GetItemFactionBonus(item->FactionMod2)) {
			AddItemFactionBonus(item->FactionMod2, item->FactionAmt2);
		} else if (item->FactionAmt2 < 0 && item->FactionAmt2 < GetItemFactionBonus(item->FactionMod2)) {
			AddItemFactionBonus(item->FactionMod2, item->FactionAmt2);
		}
	}

	if (item->FactionMod3) {
		if (item->FactionAmt3 > 0 && item->FactionAmt3 > GetItemFactionBonus(item->FactionMod3)) {
			AddItemFactionBonus(item->FactionMod3, item->FactionAmt3);
		} else if (item->FactionAmt3 < 0 && item->FactionAmt3 < GetItemFactionBonus(item->FactionMod3)) {
			AddItemFactionBonus(item->FactionMod3, item->FactionAmt3);
		}
	}

	if (item->FactionMod4) {
		if (item->FactionAmt4 > 0 && item->FactionAmt4 > GetItemFactionBonus(item->FactionMod4)) {
			AddItemFactionBonus(item->FactionMod4, item->FactionAmt4);
		} else if (item->FactionAmt4 < 0 && item->FactionAmt4 < GetItemFactionBonus(item->FactionMod4)) {
			AddItemFactionBonus(item->FactionMod4, item->FactionAmt4);
		}
	}

	if (!is_augment) {
		for (int i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++) {
			const auto* augment = inst->GetAugment(i);
			if (!augment) {
				continue;
			}

			AddItemBonuses(augment, b, true, false, recommended_level);
		}
	}
}

void Mob::AdditiveWornBonuses(const EQ::ItemInstance* inst, StatBonuses* b, bool is_augment) {
	/*
		Powerful Non-live like option allows developers to add worn effects on items that
		can stack with other worn effects of the same spell effect type, instead of only taking the highest value.
		Ie Cleave I = 40 pct cleave - So if you equip 3 cleave I items you will have a 120% cleave bonus.
		To enable use RuleI(Spells, AdditiveBonusWornType)
		Setting value =  2  Will force all live items to automatically be calculated additively
		Setting value to anything else will indicate the item 'worntype' that if set to the same, will cause the bonuses to use this calculation
		which will also stack with regular (worntype 2) effects. [Ie set rule = 3 and item worntype = 3]
	*/

	if (!inst || !inst->IsClassCommon()) {
		return;
	}

	if (inst->GetAugmentType() == 0 && is_augment) {
		return;
	}

	const auto* item = inst->GetItem();

	if (!inst->IsEquipable(GetBaseRace(), GetClass())) {
		return;
	}

	if (GetLevel() < item->ReqLevel) {
		return;
	}

	if (item->Worn.Effect > 0 && item->Worn.Type == RuleI(Spells, AdditiveBonusWornType)) {
		ApplySpellsBonuses(
			item->Worn.Effect,
			item->Worn.Level,
			b,
			0,
			item->Worn.Type
		);
	}

	if (!is_augment) {
		int i;
		for (i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++) {
			AdditiveWornBonuses(inst->GetAugment(i), b, true);
		}
	}
}

void Client::CalcEdibleBonuses(StatBonuses* newbon) {
	uint32 i;

	bool food = false;
	bool drink = false;
	for (i = EQ::invslot::GENERAL_BEGIN; i <= EQ::invslot::GENERAL_END; i++)
	{
		if (food && drink)
			break;
		const EQ::ItemInstance* inst = GetInv().GetItem(i);
		if (inst && inst->GetItem() && inst->IsClassCommon()) {
			const EQ::ItemData *item = inst->GetItem();
			if (!food && item->ItemType == EQ::item::ItemTypeFood)
				food = true;
			else if (!drink && item->ItemType == EQ::item::ItemTypeDrink)
				drink = true;
			else
				continue;
			AddItemBonuses(inst, newbon);
		}
	}
	for (i = EQ::invbag::GENERAL_BAGS_BEGIN; i <= EQ::invbag::GENERAL_BAGS_END; i++)
	{
		if (food && drink)
			break;
		const EQ::ItemInstance* inst = GetInv().GetItem(i);
		if (inst && inst->GetItem() && inst->IsClassCommon()) {
			const EQ::ItemData *item = inst->GetItem();
			if (!food && item->ItemType == EQ::item::ItemTypeFood)
				food = true;
			else if (!drink && item->ItemType == EQ::item::ItemTypeDrink)
				drink = true;
			else
				continue;
			AddItemBonuses(inst, newbon);
		}
	}
}

void Mob::CalcAABonuses(StatBonuses *newbon)
{
	memset(newbon, 0, sizeof(StatBonuses)); // start fresh

	for (const auto &aa : aa_ranks) {
		auto ability_rank = zone->GetAlternateAdvancementAbilityAndRank(aa.first, aa.second.first);
		auto ability = ability_rank.first;
		auto rank = ability_rank.second;

		if(!ability) {
			continue;
		}

		// bad data or no effects
		if (rank->effects.empty())
			continue;

		ApplyAABonuses(*rank, newbon);
	}
}

//A lot of the normal spell functions (IsBlankSpellEffect, etc) are set for just spells (in common/spdat.h).
//For now, we'll just put them directly into the code and comment with the corresponding normal function
//Maybe we'll fix it later? :-D
void Mob::ApplyAABonuses(const AA::Rank &rank, StatBonuses *newbon)
{
	if (rank.effects.empty()) // sanity check. why bother if no slots to fill?
		return;

	uint32 effect = 0;
	int32 base_value = 0;
	int32 limit_value = 0; // only really used for SpellEffect::RaiseStatCap & SpellEffect::ReduceSkillTimer in aa_effects table
	uint32 slot = 0;

	for (const auto &e : rank.effects) {
		effect = e.effect_id;
		base_value = e.base_value;
		limit_value = e.limit_value;
		slot = e.slot;

		// we default to 0 (SpellEffect::CurrentHP) for the effect, so if there aren't any base1/2 values, we'll just skip it
		if (effect == 0 && base_value == 0 && limit_value == 0)
			continue;

		// IsBlankSpellEffect()
		if (effect == SpellEffect::Blank || (effect == SpellEffect::CHA && base_value == 0) || effect == SpellEffect::StackingCommand_Block ||
		    effect == SpellEffect::StackingCommand_Overwrite)
			continue;

		LogAA("Applying Effect [{}] from AA [{}] in slot [{}] (base1: [{}], base2: [{}]) on [{}]",
			effect, rank.id, slot, base_value, limit_value, GetCleanName());

		uint8 focus = IsFocusEffect(0, 0, true, effect);
		if (focus) {
			newbon->FocusEffects[focus] = effect;
			continue;
		}

		switch (effect) {
		case SpellEffect::ACv2:
		case SpellEffect::ArmorClass:
			newbon->AC += base_value;
			break;
		// Note: AA effects that use accuracy are skill limited, while spell effect is not.
		case SpellEffect::Accuracy:
			// Bad data or unsupported new skill
			if (limit_value > EQ::skills::HIGHEST_SKILL)
				break;
			if ((limit_value == ALL_SKILLS) && (newbon->Accuracy[EQ::skills::HIGHEST_SKILL + 1] < base_value))
				newbon->Accuracy[EQ::skills::HIGHEST_SKILL + 1] = base_value;
			else if (newbon->Accuracy[limit_value] < base_value)
				newbon->Accuracy[limit_value] += base_value;
			break;
		case SpellEffect::CurrentHP: // regens
			newbon->HPRegen += base_value;
			break;
		case SpellEffect::CurrentEndurance:
			newbon->EnduranceRegen += base_value;
			break;
		case SpellEffect::MovementSpeed:
			newbon->movementspeed += base_value; // should we let these stack?
			/*if (base1 > newbon->movementspeed)	//or should we use a total value?
				newbon->movementspeed = base1;*/
			break;
		case SpellEffect::STR:
			newbon->STR += base_value;
			break;
		case SpellEffect::DEX:
			newbon->DEX += base_value;
			break;
		case SpellEffect::AGI:
			newbon->AGI += base_value;
			break;
		case SpellEffect::STA:
			newbon->STA += base_value;
			break;
		case SpellEffect::INT:
			newbon->INT += base_value;
			break;
		case SpellEffect::WIS:
			newbon->WIS += base_value;
			break;
		case SpellEffect::CHA:
			newbon->CHA += base_value;
			break;
		case SpellEffect::WaterBreathing:
			// handled by client
			break;
		case SpellEffect::CurrentMana:
			newbon->ManaRegen += base_value;
			break;
		case SpellEffect::ManaPool:
			newbon->Mana += base_value;
			break;
		case SpellEffect::ItemManaRegenCapIncrease:
			newbon->ItemManaRegenCap += base_value;
			break;
		case SpellEffect::ResistFire:
			newbon->FR += base_value;
			break;
		case SpellEffect::ResistCold:
			newbon->CR += base_value;
			break;
		case SpellEffect::ResistPoison:
			newbon->PR += base_value;
			break;
		case SpellEffect::ResistDisease:
			newbon->DR += base_value;
			break;
		case SpellEffect::ResistMagic:
			newbon->MR += base_value;
			break;
		case SpellEffect::ResistCorruption:
			newbon->Corrup += base_value;
			break;
		case SpellEffect::IncreaseSpellHaste:
			break;
		case SpellEffect::IncreaseRange:
			break;
		case SpellEffect::MaxHPChange:
			newbon->PercentMaxHPChange += base_value;
			break;
		case SpellEffect::Packrat:
			newbon->Packrat += base_value;
			break;
		case SpellEffect::TwoHandBash:
			break;
		case SpellEffect::SetBreathLevel:
			break;
		case SpellEffect::RaiseStatCap:
			switch (limit_value) {
			// are these #define'd somewhere?
			case 0: // str
				newbon->STRCapMod += base_value;
				break;
			case 1: // sta
				newbon->STACapMod += base_value;
				break;
			case 2: // agi
				newbon->AGICapMod += base_value;
				break;
			case 3: // dex
				newbon->DEXCapMod += base_value;
				break;
			case 4: // wis
				newbon->WISCapMod += base_value;
				break;
			case 5: // int
				newbon->INTCapMod += base_value;
				break;
			case 6: // cha
				newbon->CHACapMod += base_value;
				break;
			case 7: // mr
				newbon->MRCapMod += base_value;
				break;
			case 8: // cr
				newbon->CRCapMod += base_value;
				break;
			case 9: // fr
				newbon->FRCapMod += base_value;
				break;
			case 10: // pr
				newbon->PRCapMod += base_value;
				break;
			case 11: // dr
				newbon->DRCapMod += base_value;
				break;
			case 12: // corruption
				newbon->CorrupCapMod += base_value;
				break;
			}
			break;
		case SpellEffect::SpellSlotIncrease:
			break;
		case SpellEffect::MysticalAttune:
			newbon->BuffSlotIncrease += base_value;
			break;
		case SpellEffect::TotalHP:
			newbon->FlatMaxHPChange += base_value;
			break;
		case SpellEffect::StunResist:
			newbon->StunResist += base_value;
			break;
		case SpellEffect::SpellCritChance:
			newbon->CriticalSpellChance += base_value;
			break;
		case SpellEffect::SpellCritDmgIncrease:
			newbon->SpellCritDmgIncrease += base_value;
			break;
		case SpellEffect::DotCritDmgIncrease:
			newbon->DotCritDmgIncrease += base_value;
			break;
		case SpellEffect::ResistSpellChance:
			newbon->ResistSpellChance += base_value;
			break;
		case SpellEffect::CriticalHealChance:
			newbon->CriticalHealChance += base_value;
			break;
		case SpellEffect::CriticalHealOverTime:
			newbon->CriticalHealOverTime += base_value;
			break;
		case SpellEffect::CriticalDoTChance:
			newbon->CriticalDoTChance += base_value;
			break;
		case SpellEffect::ReduceSkillTimer:
			newbon->SkillReuseTime[limit_value] += base_value;
			break;
		case SpellEffect::Fearless:
			newbon->Fearless = true;
			break;
		case SpellEffect::PersistantCasting:
			newbon->PersistantCasting += base_value;
			break;
		case SpellEffect::DelayDeath:
			newbon->DelayDeath += base_value;
			break;
		case SpellEffect::FrontalStunResist:
			newbon->FrontalStunResist += base_value;
			break;
		case SpellEffect::ImprovedBindWound:
			newbon->BindWound += base_value;
			break;
		case SpellEffect::MaxBindWound:
			newbon->MaxBindWound += base_value;
			break;
		case SpellEffect::SeeInvis:
			base_value = std::min({ base_value, MAX_INVISIBILTY_LEVEL });
			if (newbon->SeeInvis < base_value) {
				newbon->SeeInvis = base_value;
			}
			break;
		case SpellEffect::BaseMovementSpeed:
			newbon->BaseMovementSpeed += base_value;
			break;
		case SpellEffect::IncreaseRunSpeedCap:
			newbon->IncreaseRunSpeedCap += base_value;
			break;
		case SpellEffect::ConsumeProjectile:
			newbon->ConsumeProjectile += base_value;
			break;
		case SpellEffect::ForageAdditionalItems:
			newbon->ForageAdditionalItems += base_value;
			break;
		case SpellEffect::Salvage:
			newbon->SalvageChance += base_value;
			break;
		case SpellEffect::ArcheryDamageModifier:
			newbon->ArcheryDamageModifier += base_value;
			break;
		case SpellEffect::DoubleRangedAttack:
			newbon->DoubleRangedAttack += base_value;
			break;
		case SpellEffect::DamageShield:
			newbon->DamageShield += base_value;
			break;
		case SpellEffect::CharmBreakChance:
			newbon->CharmBreakChance += base_value;
			break;
		case SpellEffect::OffhandRiposteFail:
			newbon->OffhandRiposteFail += base_value;
			break;
		case SpellEffect::ItemAttackCapIncrease:
			newbon->ItemATKCap += base_value;
			break;
		case SpellEffect::GivePetGroupTarget:
			newbon->GivePetGroupTarget = true;
			break;
		case SpellEffect::ItemHPRegenCapIncrease:
			newbon->ItemHPRegenCap += base_value;
			break;
		case SpellEffect::Ambidexterity:
			newbon->Ambidexterity += base_value;
			break;
		case SpellEffect::PetMaxHP:
			newbon->PetMaxHP += base_value;
			break;
		case SpellEffect::AvoidMeleeChance:
			newbon->AvoidMeleeChanceEffect += base_value;
			break;
		case SpellEffect::CombatStability:
			newbon->CombatStability += base_value;
			break;
		case SpellEffect::AddSingingMod:
			switch (limit_value) {
			case EQ::item::ItemTypeWindInstrument:
				newbon->windMod += base_value;
				break;
			case EQ::item::ItemTypeStringedInstrument:
				newbon->stringedMod += base_value;
				break;
			case EQ::item::ItemTypeBrassInstrument:
				newbon->brassMod += base_value;
				break;
			case EQ::item::ItemTypePercussionInstrument:
				newbon->percussionMod += base_value;
				break;
			case EQ::item::ItemTypeSinging:
				newbon->singingMod += base_value;
				break;
			}
			break;
		case SpellEffect::SongModCap:
			newbon->songModCap += base_value;
			break;
		case SpellEffect::PetCriticalHit:
			newbon->PetCriticalHit += base_value;
			break;
		case SpellEffect::PetAvoidance:
			newbon->PetAvoidance += base_value;
			break;
		case SpellEffect::ShieldBlock:
			newbon->ShieldBlock += base_value;
			break;
		case SpellEffect::ShieldEquipDmgMod:
			newbon->ShieldEquipDmgMod += base_value;
			break;
		case SpellEffect::SecondaryDmgInc:
			newbon->SecondaryDmgInc = true;
			break;
		case SpellEffect::ChangeAggro:
			newbon->hatemod += base_value;
			break;
		case SpellEffect::EndurancePool:
			newbon->Endurance += base_value;
			break;
		case SpellEffect::ChannelChanceItems:
			newbon->ChannelChanceItems += base_value;
			break;
		case SpellEffect::ChannelChanceSpells:
			newbon->ChannelChanceSpells += base_value;
			break;
		case SpellEffect::DoubleSpecialAttack:
			newbon->DoubleSpecialAttack += base_value;
			break;
		case SpellEffect::TripleBackstab:
			newbon->TripleBackstab += base_value;
			break;
		case SpellEffect::FrontalBackstabMinDmg:
			newbon->FrontalBackstabMinDmg = true;
			break;
		case SpellEffect::FrontalBackstabChance:
			newbon->FrontalBackstabChance += base_value;
			break;
		case SpellEffect::Double_Backstab_Front:
			newbon->Double_Backstab_Front += base_value;
			break;
		case SpellEffect::BlockBehind:
			newbon->BlockBehind += base_value;
			break;
		case SpellEffect::StrikeThrough:
		case SpellEffect::StrikeThrough2:
			newbon->StrikeThrough += base_value;
			break;
		case SpellEffect::DoubleAttackChance:
			newbon->DoubleAttackChance += base_value;
			break;
		case SpellEffect::GiveDoubleAttack:
			newbon->GiveDoubleAttack += base_value;
			break;
		case SpellEffect::ProcChance:
			newbon->ProcChanceSPA += base_value;
			break;
		case SpellEffect::RiposteChance:
			newbon->RiposteChance += base_value;
			break;
		case SpellEffect::DodgeChance:
			newbon->DodgeChance += base_value;
			break;
		case SpellEffect::ParryChance:
			newbon->ParryChance += base_value;
			break;
		case SpellEffect::IncreaseBlockChance:
			newbon->IncreaseBlockChance += base_value;
			break;
		case SpellEffect::Flurry:
			newbon->FlurryChance += base_value;
			break;
		case SpellEffect::PetFlurry:
			newbon->PetFlurry += base_value;
			break;
		case SpellEffect::BardSongRange:
			newbon->SongRange += base_value;
			break;
		case SpellEffect::RootBreakChance:
			newbon->RootBreakChance += base_value;
			break;
		case SpellEffect::UnfailingDivinity:
			newbon->UnfailingDivinity += base_value;
			break;
		case SpellEffect::CrippBlowChance:
			newbon->CrippBlowChance += base_value;
			break;

		case SpellEffect::HitChance: {
			// Bad data or unsupported new skill
			if (limit_value > EQ::skills::HIGHEST_SKILL)
				break;
			if (limit_value == ALL_SKILLS)
				newbon->HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1] += base_value;
			else
				newbon->HitChanceEffect[limit_value] += base_value;
			break;
		}

		case SpellEffect::ProcOnKillShot:
			for (int i = 0; i < MAX_SPELL_TRIGGER * 3; i += 3) {
				if (!newbon->SpellOnKill[i] ||
				    ((newbon->SpellOnKill[i] == limit_value) && (newbon->SpellOnKill[i + 1] < base_value))) {
					// base1 = chance, base2 = SpellID to be triggered, base3 = min npc level
					newbon->SpellOnKill[i] = limit_value;
					newbon->SpellOnKill[i + 1] = base_value;

					if (GetLevel() > 15)
						newbon->SpellOnKill[i + 2] =
						    GetLevel() - 15; // AA specifiy "non-trivial"
					else
						newbon->SpellOnKill[i + 2] = 0;

					break;
				}
			}
			break;

		case SpellEffect::SpellOnDeath:
			for (int i = 0; i < MAX_SPELL_TRIGGER * 2; i += 2) {
				if (!newbon->SpellOnDeath[i]) {
					// base1 = SpellID to be triggered, base2 = chance to fire
					newbon->SpellOnDeath[i] = base_value;
					newbon->SpellOnDeath[i + 1] = limit_value;
					break;
				}
			}
			break;

		case SpellEffect::WeaponProc:
		case SpellEffect::AddMeleeProc:
			for (int i = 0; i < MAX_AA_PROCS; i += 4) {
				if (!newbon->SpellProc[i + SBIndex::COMBAT_PROC_ORIGIN_ID]) {
					newbon->SpellProc[i + SBIndex::COMBAT_PROC_ORIGIN_ID] = rank.id;   //aa rank id
					newbon->SpellProc[i + SBIndex::COMBAT_PROC_SPELL_ID] = base_value; //proc spell id
					newbon->SpellProc[i + SBIndex::COMBAT_PROC_RATE_MOD] = limit_value; //proc rate modifer
					newbon->SpellProc[i + SBIndex::COMBAT_PROC_REUSE_TIMER] = 0;	  //Lock out Timer
					break;
				}
			}
			break;

		case SpellEffect::RangedProc:
			for (int i = 0; i < MAX_AA_PROCS; i += 4) {
				if (!newbon->RangedProc[i + SBIndex::COMBAT_PROC_ORIGIN_ID]) {
					newbon->RangedProc[i + SBIndex::COMBAT_PROC_ORIGIN_ID] = rank.id;   //aa rank id
					newbon->RangedProc[i + SBIndex::COMBAT_PROC_SPELL_ID] = base_value; //proc spell id
					newbon->RangedProc[i + SBIndex::COMBAT_PROC_RATE_MOD] = limit_value; //proc rate modifer
					newbon->RangedProc[i + SBIndex::COMBAT_PROC_REUSE_TIMER] = 0;	   //Lock out Timer
					break;
				}
			}
			break;

		case SpellEffect::DefensiveProc:
			for (int i = 0; i < MAX_AA_PROCS; i += 4) {
				if (!newbon->DefensiveProc[i + SBIndex::COMBAT_PROC_ORIGIN_ID]) {
					newbon->DefensiveProc[i + SBIndex::COMBAT_PROC_ORIGIN_ID] = rank.id;   //aa rank id
					newbon->DefensiveProc[i + SBIndex::COMBAT_PROC_SPELL_ID] = base_value; //proc spell id
					newbon->DefensiveProc[i + SBIndex::COMBAT_PROC_RATE_MOD] = limit_value; //proc rate modifer
					newbon->DefensiveProc[i + SBIndex::COMBAT_PROC_REUSE_TIMER] = 0;	  //Lock out Timer
					break;
				}
			}
			break;

		case SpellEffect::Proc_Timer_Modifier: {
			/*
				AA can multiples of this in a single effect, proc should use the timer
				that comes after the respective proc spell effect, thus rank.id will be already set
				when this is checked.
			*/

			newbon->Proc_Timer_Modifier = true;

			for (int i = 0; i < MAX_AA_PROCS; i += 4) {
				if (newbon->SpellProc[i + SBIndex::COMBAT_PROC_ORIGIN_ID] == rank.id) {
					if (!newbon->SpellProc[i + SBIndex::COMBAT_PROC_REUSE_TIMER]) {
						newbon->SpellProc[i + SBIndex::COMBAT_PROC_REUSE_TIMER] = limit_value;//Lock out Timer
						break;
					}
				}

				if (newbon->RangedProc[i + SBIndex::COMBAT_PROC_ORIGIN_ID] == rank.id) {
					if (!newbon->RangedProc[i + SBIndex::COMBAT_PROC_REUSE_TIMER]) {
						newbon->RangedProc[i + SBIndex::COMBAT_PROC_REUSE_TIMER] = limit_value;//Lock out Timer
						break;
					}
				}

				if (newbon->DefensiveProc[i + SBIndex::COMBAT_PROC_ORIGIN_ID] == rank.id) {
					if (!newbon->DefensiveProc[i + SBIndex::COMBAT_PROC_REUSE_TIMER]) {
						newbon->DefensiveProc[i + SBIndex::COMBAT_PROC_REUSE_TIMER] = limit_value;//Lock out Timer
						break;
					}
				}
			}
			break;
		}

		case SpellEffect::CriticalHitChance: {
			// Bad data or unsupported new skill
			if (limit_value > EQ::skills::HIGHEST_SKILL)
				break;
			if (limit_value == ALL_SKILLS)
				newbon->CriticalHitChance[EQ::skills::HIGHEST_SKILL + 1] += base_value;
			else
				newbon->CriticalHitChance[limit_value] += base_value;
		} break;

		case SpellEffect::CriticalDamageMob: {
			// Bad data or unsupported new skill
			if (limit_value > EQ::skills::HIGHEST_SKILL)
				break;
			// base1 = effect value, base2 = skill restrictions(-1 for all)
			if (limit_value == ALL_SKILLS)
				newbon->CritDmgMod[EQ::skills::HIGHEST_SKILL + 1] += base_value;
			else
				newbon->CritDmgMod[limit_value] += base_value;
			break;
		}

		case SpellEffect::Critical_Melee_Damage_Mod_Max:
		{
			// Bad data or unsupported new skill
			if (limit_value > EQ::skills::HIGHEST_SKILL)
				break;
			int skill = limit_value == ALL_SKILLS ? EQ::skills::HIGHEST_SKILL + 1 : limit_value;
			if (base_value < 0 && newbon->CritDmgModNoStack[skill] > base_value)
				newbon->CritDmgModNoStack[skill] = base_value;
			else if (base_value > 0 && newbon->CritDmgModNoStack[skill] < base_value)
				newbon->CritDmgModNoStack[skill] = base_value;
			break;
		}

		case SpellEffect::CriticalSpellChance: {
			newbon->CriticalSpellChance += base_value;

			if (limit_value > newbon->SpellCritDmgIncNoStack)
				newbon->SpellCritDmgIncNoStack = limit_value;

			break;
		}

		case SpellEffect::ResistFearChance: {
			newbon->ResistFearChance += base_value; // these should stack
			break;
		}

		case SpellEffect::SkillDamageAmount: {
			// Bad data or unsupported new skill
			if (limit_value > EQ::skills::HIGHEST_SKILL)
				break;
			if (limit_value == ALL_SKILLS)
				newbon->SkillDamageAmount[EQ::skills::HIGHEST_SKILL + 1] += base_value;
			else
				newbon->SkillDamageAmount[limit_value] += base_value;
			break;
		}

		case SpellEffect::SkillAttackProc: {
			for (int i = 0; i < MAX_CAST_ON_SKILL_USE; i += 3) {
				if (!newbon->SkillAttackProc[i + SBIndex::SKILLATK_PROC_SPELL_ID]) { // spell id
					newbon->SkillAttackProc[i + SBIndex::SKILLATK_PROC_SPELL_ID] = rank.spell; // spell to proc
					newbon->SkillAttackProc[i + SBIndex::SKILLATK_PROC_CHANCE] = base_value; // Chance base 1000 = 100% proc rate
					newbon->SkillAttackProc[i + SBIndex::SKILLATK_PROC_SKILL] = limit_value; // Skill to Proc Offr

					if (limit_value <= EQ::skills::HIGHEST_SKILL) {
						newbon->HasSkillAttackProc[limit_value] = true; //check first before looking for any effects.
					}
					break;
				}
			}
			break;
		}

		case SpellEffect::DamageModifier: {
			// Bad data or unsupported new skill
			if (limit_value > EQ::skills::HIGHEST_SKILL)
				break;
			if (limit_value == ALL_SKILLS)
				newbon->DamageModifier[EQ::skills::HIGHEST_SKILL + 1] += base_value;
			else
				newbon->DamageModifier[limit_value] += base_value;
			break;
		}

		case SpellEffect::DamageModifier2: {
			// Bad data or unsupported new skill
			if (limit_value > EQ::skills::HIGHEST_SKILL)
				break;
			if (limit_value == ALL_SKILLS)
				newbon->DamageModifier2[EQ::skills::HIGHEST_SKILL + 1] += base_value;
			else
				newbon->DamageModifier2[limit_value] += base_value;
			break;
		}

		case SpellEffect::Skill_Base_Damage_Mod: {
			// Bad data or unsupported new skill
			if (limit_value > EQ::skills::HIGHEST_SKILL)
				break;
			if (limit_value == ALL_SKILLS)
				newbon->DamageModifier3[EQ::skills::HIGHEST_SKILL + 1] += base_value;
			else
				newbon->DamageModifier3[limit_value] += base_value;
			break;
		}

		case SpellEffect::SlayUndead: {
			if (newbon->SlayUndead[SBIndex::SLAYUNDEAD_DMG_MOD] < base_value) {
				newbon->SlayUndead[SBIndex::SLAYUNDEAD_DMG_MOD] = base_value; // Rate
				newbon->SlayUndead[SBIndex::SLAYUNDEAD_RATE_MOD] = limit_value; // Damage Modifier
			}
			break;
		}

		case SpellEffect::DoubleRiposte: {
			newbon->DoubleRiposte += base_value;
			break;
		}

		case SpellEffect::GiveDoubleRiposte: {
			// 0=Regular Riposte 1=Skill Attack Riposte 2=Skill
			if (limit_value == 0) {
				if (newbon->GiveDoubleRiposte[SBIndex::DOUBLE_RIPOSTE_CHANCE] < base_value)
					newbon->GiveDoubleRiposte[SBIndex::DOUBLE_RIPOSTE_CHANCE] = base_value;
			}
			// Only for special attacks.
			else if (limit_value > 0 && (newbon->GiveDoubleRiposte[SBIndex::DOUBLE_RIPOSTE_SKILL_ATK_CHANCE] < base_value)) {
				newbon->GiveDoubleRiposte[SBIndex::DOUBLE_RIPOSTE_SKILL_ATK_CHANCE] = base_value;
				newbon->GiveDoubleRiposte[SBIndex::DOUBLE_RIPOSTE_SKILL]            = limit_value;
			}

			break;
		}

		// Physically raises skill cap ie if 55/55 it will raise to 55/60
		case SpellEffect::RaiseSkillCap: {

			if (limit_value > EQ::skills::HIGHEST_SKILL)
				break;

			if (newbon->RaiseSkillCap[limit_value] < base_value)
				newbon->RaiseSkillCap[limit_value] = base_value;
			break;
		}

		case SpellEffect::MasteryofPast: {
			if (newbon->MasteryofPast < base_value)
				newbon->MasteryofPast = base_value;
			break;
		}

		case SpellEffect::CastingLevel: {
			newbon->adjusted_casting_skill += base_value;
			break;
		}

		case SpellEffect::CastingLevel2: {
			newbon->effective_casting_level += base_value;
			break;
		}

		case SpellEffect::DivineSave: {
			if (newbon->DivineSaveChance[SBIndex::DIVINE_SAVE_CHANCE] < base_value) {
				newbon->DivineSaveChance[SBIndex::DIVINE_SAVE_CHANCE]           = base_value;
				newbon->DivineSaveChance[SBIndex::DIVINE_SAVE_SPELL_TRIGGER_ID] = limit_value;
			}
			break;
		}

		case SpellEffect::SpellEffectResistChance: {
			for (int e = 0; e < MAX_RESISTABLE_EFFECTS * 2; e += 2) {
				if (
					!newbon->SEResist[e + 1] ||
					(
						newbon->SEResist[e + 1] &&
						newbon->SEResist[e] == limit_value &&
						newbon->SEResist[e + 1] < base_value
					)
				) {
					newbon->SEResist[e] = limit_value; // Spell Effect ID
					newbon->SEResist[e + 1] = base_value; // Resist Chance
					break;
				}
			}
			break;
		}

		case SpellEffect::MitigateDamageShield: {

			//AA that increase mitigation are set to negative.
			if (base_value < 0) {
				base_value = base_value * (-1);
			}

			newbon->DSMitigationOffHand += base_value;
			break;
		}

		case SpellEffect::FinishingBlow: {
			// base1 = chance, base2 = damage
			if (newbon->FinishingBlow[SBIndex::FINISHING_EFFECT_DMG] < limit_value) {
				newbon->FinishingBlow[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = base_value;
				newbon->FinishingBlow[SBIndex::FINISHING_EFFECT_DMG]         = limit_value;
			}
			break;
		}

		case SpellEffect::FinishingBlowLvl: {
			// base1 = level, base2 = ??? (Set to 200 in AA data, possible proc rate mod?)
			if (newbon->FinishingBlowLvl[SBIndex::FINISHING_EFFECT_LEVEL_MAX] < base_value) {
				newbon->FinishingBlowLvl[SBIndex::FINISHING_EFFECT_LEVEL_MAX]          = base_value;
				newbon->FinishingBlowLvl[SBIndex::FINISHING_BLOW_LEVEL_HP_RATIO] = limit_value;
			}
			break;
		}

		case SpellEffect::StunBashChance:
			newbon->StunBashChance += base_value;
			break;

		case SpellEffect::IncreaseChanceMemwipe:
			newbon->IncreaseChanceMemwipe += base_value;
			break;

		case SpellEffect::CriticalMend:
			newbon->CriticalMend += base_value;
			break;

		case SpellEffect::HealRate:
			newbon->HealRate += base_value;
			break;

		case SpellEffect::MeleeLifetap: {

			if ((base_value < 0) && (newbon->MeleeLifetap > base_value))
				newbon->MeleeLifetap = base_value;

			else if (newbon->MeleeLifetap < base_value)
				newbon->MeleeLifetap = base_value;
			break;
		}

		case SpellEffect::Vampirism:
			newbon->Vampirism += base_value;
			break;

		case SpellEffect::FrenziedDevastation:
			newbon->FrenziedDevastation += limit_value;
			break;

		case SpellEffect::SpellProcChance:
			newbon->SpellProcChance += base_value;
			break;

		case SpellEffect::Berserk:
			newbon->BerserkSPA = true;
			break;

		case SpellEffect::Metabolism:
			newbon->Metabolism += base_value;
			break;

		case SpellEffect::ImprovedReclaimEnergy: {
			if ((base_value < 0) && (newbon->ImprovedReclaimEnergy > base_value))
				newbon->ImprovedReclaimEnergy = base_value;

			else if (newbon->ImprovedReclaimEnergy < base_value)
				newbon->ImprovedReclaimEnergy = base_value;
			break;
		}

		case SpellEffect::HeadShot: {
			if (newbon->HeadShot[SBIndex::FINISHING_EFFECT_DMG] < limit_value) {
				newbon->HeadShot[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = base_value;
				newbon->HeadShot[SBIndex::FINISHING_EFFECT_DMG]         = limit_value;
			}
			break;
		}

		case SpellEffect::HeadShotLevel: {
			if (newbon->HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX] < base_value) {
				newbon->HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX] = base_value;
				newbon->HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_CHANCE_BONUS] = limit_value;
			}
			break;
		}

		case SpellEffect::Assassinate: {
			if (newbon->Assassinate[SBIndex::FINISHING_EFFECT_DMG] < limit_value) {
				newbon->Assassinate[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = base_value;
				newbon->Assassinate[SBIndex::FINISHING_EFFECT_DMG]         = limit_value;
			}
			break;
		}

		case SpellEffect::AssassinateLevel: {
			if (newbon->AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX] < base_value) {
				newbon->AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX]          = base_value;
				newbon->AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_CHANCE_BONUS] = limit_value;
			}
			break;
		}

		case SpellEffect::PetMeleeMitigation:
			newbon->PetMeleeMitigation += base_value;
			break;

		case SpellEffect::FactionModPct: {
			if ((base_value < 0) && (newbon->FactionModPct > base_value))
				newbon->FactionModPct = base_value;

			else if (newbon->FactionModPct < base_value)
				newbon->FactionModPct = base_value;
			break;
		}

		case SpellEffect::Illusion:
			newbon->Illusion = rank.spell;
			break;

		case SpellEffect::IllusionPersistence:
			newbon->IllusionPersistence = base_value;
			break;

		case SpellEffect::LimitToSkill: {

			// Bad data or unsupported new skill
			if (base_value > EQ::skills::HIGHEST_SKILL) {
				break;
			}
			if (base_value <= EQ::skills::HIGHEST_SKILL) {
				newbon->LimitToSkill[base_value] = true;
				newbon->LimitToSkill[EQ::skills::HIGHEST_SKILL + 2] = true; //Used as a general exists check
			}
			break;
		}

		case SpellEffect::SkillProcAttempt: {
			for (int e = 0; e < MAX_SKILL_PROCS; e++) {
				if (newbon->SkillProc[e] && newbon->SkillProc[e] == rank.id)
					break; // Do not use the same aa id more than once.

				else if (!newbon->SkillProc[e]) {
					newbon->SkillProc[e] = rank.id;
					break;
				}
			}
			break;
		}

		case SpellEffect::SkillProcSuccess: {

			for (int e = 0; e < MAX_SKILL_PROCS; e++) {
				if (newbon->SkillProcSuccess[e] && newbon->SkillProcSuccess[e] == rank.id)
					break; // Do not use the same spell id more than once.

				else if (!newbon->SkillProcSuccess[e]) {
					newbon->SkillProcSuccess[e] = rank.id;
					break;
				}
			}
			break;
		}

		case SpellEffect::MeleeMitigation:
			newbon->MeleeMitigationEffect += base_value;
			break;

		case SpellEffect::ATK:
			newbon->ATK += base_value;
			break;
		case SpellEffect::IncreaseExtTargetWindow:
			newbon->extra_xtargets += base_value;
			break;

		case SpellEffect::PC_Pet_Rampage: {
			newbon->PC_Pet_Rampage[SBIndex::PET_RAMPAGE_CHANCE] += base_value; //Chance to rampage
			if (newbon->PC_Pet_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] < limit_value)
				newbon->PC_Pet_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] = limit_value; //Damage modifer - take highest
			break;
		}

		case SpellEffect::PC_Pet_AE_Rampage: {
			newbon->PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_CHANCE] += base_value; //Chance to rampage
			if (newbon->PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] < limit_value)
				newbon->PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] = limit_value; //Damage modifer - take highest
			break;
		}

		case SpellEffect::PC_Pet_Flurry_Chance:
			newbon->PC_Pet_Flurry += base_value; //Chance to Flurry
			break;

		case SpellEffect::ShroudofStealth:
			newbon->ShroudofStealth = true;
			break;

		case SpellEffect::ReduceFallDamage:
			newbon->ReduceFallDamage += base_value;
			break;

		case SpellEffect::ReduceTradeskillFail:{

			if (limit_value > EQ::skills::HIGHEST_SKILL)
				break;

			newbon->ReduceTradeskillFail[limit_value] += base_value;
			break;
		}

		case SpellEffect::TradeSkillMastery:
			if (newbon->TradeSkillMastery < base_value)
				newbon->TradeSkillMastery = base_value;
			break;

		case SpellEffect::NoBreakAESneak:
			if (newbon->NoBreakAESneak < base_value)
				newbon->NoBreakAESneak = base_value;
			break;

		case SpellEffect::FeignedCastOnChance:
			if (newbon->FeignedCastOnChance < base_value)
				newbon->FeignedCastOnChance = base_value;
			break;

		case SpellEffect::AddPetCommand:
			if (base_value && limit_value < PetCommand::Max)
				newbon->PetCommands[limit_value] = true;
			break;

		case SpellEffect::FeignedMinion:
			if (newbon->FeignedMinionChance < base_value) {
				newbon->FeignedMinionChance = base_value;
			}
			newbon->PetCommands[PetCommand::Feign] = true;
			break;

		case SpellEffect::AdditionalAura:
			newbon->aura_slots += base_value;
			break;

		case SpellEffect::IncreaseTrapCount:
			newbon->trap_slots += base_value;
			break;

		case SpellEffect::ForageSkill:
			newbon->GrantForage += base_value;
			// we need to grant a skill point here
			// I'd rather not do this here, but whatever, probably fine
			if (IsClient()) {
				auto client = CastToClient();
				if (client->GetRawSkill(EQ::skills::SkillType::SkillForage) == 0)
					client->SetSkill(EQ::skills::SkillType::SkillForage, 1);
			}
			break;

		case SpellEffect::Attack_Accuracy_Max_Percent:
			newbon->Attack_Accuracy_Max_Percent += base_value;
			break;

		case SpellEffect::AC_Mitigation_Max_Percent:
			newbon->AC_Mitigation_Max_Percent += base_value;
			break;

		case SpellEffect::AC_Avoidance_Max_Percent:
			newbon->AC_Avoidance_Max_Percent += base_value;
			break;

		case SpellEffect::Damage_Taken_Position_Mod:
		{
			//Mitigate if damage taken from behind base2 = 0, from front base2 = 1
			if (limit_value < 0 || limit_value >= 2) {
				break;
			}
			else if (base_value < 0 && newbon->Damage_Taken_Position_Mod[limit_value] > base_value)
				newbon->Damage_Taken_Position_Mod[limit_value] = base_value;
			else if (base_value > 0 && newbon->Damage_Taken_Position_Mod[limit_value] < base_value)
				newbon->Damage_Taken_Position_Mod[limit_value] = base_value;
			break;
		}

		case SpellEffect::Melee_Damage_Position_Mod:
		{
			if (limit_value < 0 || limit_value >= 2) {
				break;
			}
			else if (base_value < 0 && newbon->Melee_Damage_Position_Mod[limit_value] > base_value)
				newbon->Melee_Damage_Position_Mod[limit_value] = base_value;
			else if (base_value > 0 && newbon->Melee_Damage_Position_Mod[limit_value] < base_value)
				newbon->Melee_Damage_Position_Mod[limit_value] = base_value;
			break;
		}

		case SpellEffect::Damage_Taken_Position_Amt:
		{
			//Mitigate if damage taken from behind base2 = 0, from front base2 = 1
			if (limit_value < 0 || limit_value >= 2) {
				break;
			}
			newbon->Damage_Taken_Position_Amt[limit_value] += base_value;
			break;
		}

		case SpellEffect::Melee_Damage_Position_Amt:
		{
			//Mitigate if damage taken from behind base2 = 0, from front base2 = 1
			if (limit_value < 0 || limit_value >= 2) {
				break;
			}

			newbon->Melee_Damage_Position_Amt[limit_value] += base_value;
			break;
		}

		case SpellEffect::DS_Mitigation_Amount:
			newbon->DS_Mitigation_Amount += base_value;
			break;

		case SpellEffect::DS_Mitigation_Percentage:
			newbon->DS_Mitigation_Percentage += base_value;
			break;

		case SpellEffect::Pet_Crit_Melee_Damage_Pct_Owner:
			newbon->Pet_Crit_Melee_Damage_Pct_Owner += base_value;
			break;

		case SpellEffect::Pet_Add_Atk:
			newbon->Pet_Add_Atk += base_value;
			break;

		case SpellEffect::Weapon_Stance:
		{
			if (IsValidSpell(base_value)) { //base1 is the spell_id of buff
				if (limit_value <= WEAPON_STANCE_TYPE_MAX) { //0=2H, 1=Shield, 2=DW
					if (IsValidSpell(newbon->WeaponStance[limit_value])) { //Check if we already a spell_id saved for this effect
						if (spells[newbon->WeaponStance[limit_value]].rank < spells[base_value].rank) { //If so, check if any new spellids with higher rank exist (live spells for this are ranked).
							newbon->WeaponStance[limit_value] = base_value; //Overwrite with new effect
							SetWeaponStanceEnabled(true);
						}
					}
					else {
						newbon->WeaponStance[limit_value] = base_value; //If no prior effect exists, then apply
						SetWeaponStanceEnabled(true);
					}
				}
			}
			break;
		}

		case SpellEffect::ExtraAttackChance:
		{
			if (newbon->ExtraAttackChance[SBIndex::EXTRA_ATTACK_CHANCE] < base_value) {
				newbon->ExtraAttackChance[SBIndex::EXTRA_ATTACK_CHANCE]   = base_value;
				newbon->ExtraAttackChance[SBIndex::EXTRA_ATTACK_NUM_ATKS] = limit_value ? limit_value : 1;
			}
			break;
		}

		case SpellEffect::AddExtraAttackPct_1h_Primary:
		{
			if (newbon->ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_CHANCE] < base_value) {
				newbon->ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_CHANCE]   = base_value;
				newbon->ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_NUM_ATKS] = limit_value ? limit_value : 1;
			}
			break;
		}

		case SpellEffect::AddExtraAttackPct_1h_Secondary:
		{

			if (newbon->ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_CHANCE] < base_value) {
				newbon->ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_CHANCE]   = base_value;
				newbon->ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_NUM_ATKS] = limit_value ? limit_value : 1;
			}
			break;
		}

		case SpellEffect::Double_Melee_Round:
		{
			if (newbon->DoubleMeleeRound[SBIndex::DOUBLE_MELEE_ROUND_CHANCE] < base_value) {
				newbon->DoubleMeleeRound[SBIndex::DOUBLE_MELEE_ROUND_CHANCE] = base_value;
				newbon->DoubleMeleeRound[SBIndex::DOUBLE_MELEE_ROUND_DMG_BONUS] = limit_value;

			}
			break;
		}

		case SpellEffect::ExtendedShielding:
		{
			if (newbon->ExtendedShielding < base_value) {
				newbon->ExtendedShielding = base_value;
			}
			break;
		}

		case SpellEffect::ShieldDuration:
		{
			if (newbon->ShieldDuration < base_value) {
				newbon->ShieldDuration = base_value;
			}
			break;
		}

		case SpellEffect::Worn_Endurance_Regen_Cap:
			newbon->ItemEnduranceRegenCap += base_value;
			break;


		case SpellEffect::SecondaryForte:
			if (newbon->SecondaryForte < base_value) {
				newbon->SecondaryForte = base_value;
			}
			break;

		case SpellEffect::ZoneSuspendMinion:
			newbon->ZoneSuspendMinion = base_value;
			break;


		case SpellEffect::Reflect:

			if (newbon->reflect[SBIndex::REFLECT_CHANCE] < base_value) {
				newbon->reflect[SBIndex::REFLECT_CHANCE] = base_value;
			}
			if (newbon->reflect[SBIndex::REFLECT_RESISTANCE_MOD] < limit_value) {
				newbon->reflect[SBIndex::REFLECT_RESISTANCE_MOD] = limit_value;
			}
			break;

		case SpellEffect::SpellDamageShield:
			newbon->SpellDamageShield += base_value;
			break;

		case SpellEffect::Amplification:
			newbon->Amplification += base_value;
			break;

		case SpellEffect::MitigateSpellDamage:
		{
			newbon->MitigateSpellRune[SBIndex::MITIGATION_RUNE_PERCENT] += base_value;
			break;
		}

		case SpellEffect::MitigateDotDamage:
		{
			newbon->MitigateDotRune[SBIndex::MITIGATION_RUNE_PERCENT] += base_value;
			break;
		}

		case SpellEffect::TrapCircumvention:
			newbon->TrapCircumvention += base_value;
			break;

		// to do
		case SpellEffect::PetDiscipline:
			break;
		case SpellEffect::PotionBeltSlots:
			break;
		case SpellEffect::BandolierSlots:
			break;
		case SpellEffect::ReduceApplyPoisonTime:
			break;
		case SpellEffect::NimbleEvasion:
			break;

		// not handled here
		case SpellEffect::HastenedAASkill:
		// not handled here but don't want to clutter debug log -- these may need to be verified to ignore
		case SpellEffect::LimitMaxLevel:
		case SpellEffect::LimitResist:
		case SpellEffect::LimitTarget:
		case SpellEffect::LimitEffect:
		case SpellEffect::LimitSpellType:
		case SpellEffect::LimitMinDur:
		case SpellEffect::LimitInstant:
		case SpellEffect::LimitMinLevel:
		case SpellEffect::LimitCastTimeMin:
		case SpellEffect::LimitCastTimeMax:
		case SpellEffect::LimitSpell:
		case SpellEffect::LimitCombatSkills:
		case SpellEffect::LimitManaMin:
		case SpellEffect::LimitSpellGroup:
		case SpellEffect::LimitSpellClass:
		case SpellEffect::LimitSpellSubclass:
		case SpellEffect::LimitHPPercent:
		case SpellEffect::LimitManaPercent:
		case SpellEffect::LimitEndPercent:
		case SpellEffect::LimitClass:
		case SpellEffect::LimitRace:
		case SpellEffect::LimitCastingSkill:
		case SpellEffect::LimitUseMin:
		case SpellEffect::LimitUseType:
			break;

		default:
			LogAA("SPA [{}] not accounted for in AA [{}] ([{}])", effect, rank.base_ability->name.c_str(), rank.id);
			break;
		}

	}
}

void Mob::CalcSpellBonuses(StatBonuses* newbon)
{
	int i;

	memset(newbon, 0, sizeof(StatBonuses));
	newbon->AggroRange = -1;
	newbon->AssistRange = -1;

	int buff_count = GetMaxTotalSlots();
	for (i = 0; i < buff_count; i++) {
		if (IsValidSpell(buffs[i].spellid)) {
			ApplySpellsBonuses(buffs[i].spellid, buffs[i].casterlevel, newbon, buffs[i].casterid, 0, buffs[i].ticsremaining, i, buffs[i].instrument_mod);

			if (buffs[i].hit_number > 0) {
				Numhits(true);
			}
		}
	}

	//Applies any perma NPC spell bonuses from npc_spells_effects table.
	if (IsNPC())
		CastToNPC()->ApplyAISpellEffects(newbon);

	//Disables a specific spell effect bonus completely, can also be limited to negate only item, AA or spell bonuses.
	if (spellbonuses.NegateEffects){
		for(i = 0; i < buff_count; i++) {
			if(IsValidSpell(buffs[i].spellid) && (IsEffectInSpell(buffs[i].spellid, SpellEffect::NegateSpellEffect)) )
				NegateSpellEffectBonuses(buffs[i].spellid);
		}
	}

	if (GetClass() == Class::Bard)
		newbon->ManaRegen = 0; // Bards do not get mana regen from spells.
}

void Mob::ApplySpellsBonuses(uint16 spell_id, uint8 casterlevel, StatBonuses *new_bonus, uint16 casterId,
			     uint8 WornType, int32 ticsremaining, int buffslot, int instrument_mod,
			     bool IsAISpellEffect, uint16 effect_id, int32 se_base, int32 se_limit, int32 se_max)
{
	int i, effect_value, limit_value, max_value, spell_effect_id;
	bool AdditiveWornBonus = false;

	if(!IsAISpellEffect && !IsValidSpell(spell_id))
		return;

	for (i = 0; i < EFFECT_COUNT; i++)
	{
		//Buffs/Item effects
		if (!IsAISpellEffect) {

			if(IsBlankSpellEffect(spell_id, i))
				continue;

			uint8 focus = IsFocusEffect(spell_id, i);
			if (focus)
			{
				if (WornType){
					if (RuleB(Spells, UseAdditiveFocusFromWornSlotWithLimits)) {
						new_bonus->FocusEffectsWornWithLimits[focus] = spells[spell_id].effect_id[i];
					}
					else if (RuleB(Spells, UseAdditiveFocusFromWornSlot)) {
						new_bonus->FocusEffectsWorn[focus] += spells[spell_id].base_value[i];
					}
				}
				else {
					new_bonus->FocusEffects[focus] = spells[spell_id].effect_id[i];
				}
				continue;
			}

			if (WornType && (RuleI(Spells, AdditiveBonusWornType) == WornType)) {
				AdditiveWornBonus = true;
			}

			spell_effect_id = spells[spell_id].effect_id[i];
			effect_value = CalcSpellEffectValue(spell_id, i, casterlevel, instrument_mod, nullptr, ticsremaining, casterId);
			limit_value = spells[spell_id].limit_value[i];
			max_value = spells[spell_id].max_value[i];
		}
		//Use AISpellEffects
		else {
			spell_effect_id = effect_id;
			effect_value    = se_base;
			limit_value     = se_limit;
			max_value       = se_max;

			//Special custom cases for loading effects on to NPC from 'npc_spels_effects' table
			//Non-Focused Effect to modify incoming spell damage by resist type.
			if (spell_effect_id == SpellEffect::FcSpellVulnerability) {
				ModVulnerability(limit_value, effect_value);
			}

			break;
		}

		switch (spell_effect_id)
		{
			case SpellEffect::CurrentHP: //regens
				if(effect_value > 0) {
					new_bonus->HPRegen += effect_value;
				}
				break;

			case SpellEffect::CurrentEndurance:
				new_bonus->EnduranceRegen += effect_value;
				break;

			case SpellEffect::ChangeFrenzyRad:
			{
				if (max_value != 0 && GetLevel() > max_value)
					break;

				if(new_bonus->AggroRange == -1 || effect_value < new_bonus->AggroRange)
				{
					new_bonus->AggroRange = static_cast<float>(effect_value);
				}
				break;
			}

			case SpellEffect::Harmony:
			{
				if (max_value != 0 && GetLevel() > max_value)
					break;
				// Harmony effect as buff - kinda tricky
				// harmony could stack with a lull spell, which has better aggro range
				// take the one with less range in any case
				if(new_bonus->AssistRange == -1 || effect_value < new_bonus->AssistRange)
				{
					new_bonus->AssistRange = static_cast<float>(effect_value);
				}
				break;
			}

			case SpellEffect::AttackSpeed:
			{
				if ((effect_value - 100) > 0) { // Haste
					if (new_bonus->haste < 0) break; // Slowed - Don't apply haste
					if ((effect_value - 100) > new_bonus->haste) {
						new_bonus->haste = effect_value - 100;
					}
				}
				else if ((effect_value - 100) < 0) { // Slow
					int real_slow_value = (100 - effect_value) * -1;
					real_slow_value -= (real_slow_value * GetSlowMitigation()/100);
					if (real_slow_value < new_bonus->haste)
						new_bonus->haste = real_slow_value;
				}
				break;
			}

			case SpellEffect::AttackSpeed2:
			{
				if ((effect_value - 100) > 0) { // Haste V2 - Stacks with V1 but does not Overcap
					if (new_bonus->hastetype2 < 0) break; //Slowed - Don't apply haste2
					if ((effect_value - 100) > new_bonus->hastetype2) {
						new_bonus->hastetype2 = effect_value - 100;
					}
				}
				else if ((effect_value - 100) < 0) { // Slow
					int real_slow_value = (100 - effect_value) * -1;
					real_slow_value -= (real_slow_value * GetSlowMitigation()/100);
					if (real_slow_value < new_bonus->hastetype2)
						new_bonus->hastetype2 = real_slow_value;
				}
				break;
			}

			case SpellEffect::AttackSpeed3:
			{
				if (effect_value < 0){ //Slow
					effect_value -= (effect_value * GetSlowMitigation()/100);
					if (effect_value < new_bonus->hastetype3)
						new_bonus->hastetype3 = effect_value;
				}

				else if (effect_value > 0) { // Haste V3 - Stacks and Overcaps
					if (effect_value > new_bonus->hastetype3) {
						new_bonus->hastetype3 = effect_value;
					}
				}
				break;
			}

			case SpellEffect::AttackSpeed4:
			{
				// These don't generate the IMMUNE_ATKSPEED message and the icon shows up
				// but have no effect on the mobs attack speed
				if (GetSpecialAbility(SpecialAbility::SlowImmunity))
					break;

				if (effect_value < 0) //A few spells use negative values(Descriptions all indicate it should be a slow)
					effect_value = effect_value * -1;

				if (effect_value > 0 && effect_value > new_bonus->inhibitmelee) {
					effect_value -= (effect_value * GetSlowMitigation()/100);
					if (effect_value > new_bonus->inhibitmelee)
						new_bonus->inhibitmelee = effect_value;
				}

				break;
			}

			case SpellEffect::IncreaseArchery:
			{
				new_bonus->increase_archery += effect_value;
				break;
			}

			case SpellEffect::TotalHP:
			{
				new_bonus->FlatMaxHPChange += effect_value;
				break;
			}

			case SpellEffect::ManaRegen_v2:
			case SpellEffect::CurrentMana:
			{
				new_bonus->ManaRegen += effect_value;
				break;
			}

			case SpellEffect::ManaPool:
			{
				new_bonus->Mana += effect_value;
				break;
			}

			case SpellEffect::Stamina:
			{
				new_bonus->EnduranceReduction += effect_value;
				break;
			}

			case SpellEffect::ACv2:
			case SpellEffect::ArmorClass:
			{
				new_bonus->AC += effect_value;
				break;
			}

			case SpellEffect::ATK:
			{
				new_bonus->ATK += effect_value;
				break;
			}

			case SpellEffect::STR:
			{
				new_bonus->STR += effect_value;
				break;
			}

			case SpellEffect::DEX:
			{
				new_bonus->DEX += effect_value;
				break;
			}

			case SpellEffect::AGI:
			{
				new_bonus->AGI += effect_value;
				break;
			}

			case SpellEffect::STA:
			{
				new_bonus->STA += effect_value;
				break;
			}

			case SpellEffect::INT:
			{
				new_bonus->INT += effect_value;
				break;
			}

			case SpellEffect::WIS:
			{
				new_bonus->WIS += effect_value;
				break;
			}

			case SpellEffect::CHA:
			{
				if (spells[spell_id].base_value[i] != 0) {
					new_bonus->CHA += effect_value;
				}
				break;
			}

			case SpellEffect::AllStats:
			{
				new_bonus->STR += effect_value;
				new_bonus->DEX += effect_value;
				new_bonus->AGI += effect_value;
				new_bonus->STA += effect_value;
				new_bonus->INT += effect_value;
				new_bonus->WIS += effect_value;
				new_bonus->CHA += effect_value;
				break;
			}

			case SpellEffect::ResistFire:
			{
				new_bonus->FR += effect_value;
				break;
			}

			case SpellEffect::ResistCold:
			{
				new_bonus->CR += effect_value;
				break;
			}

			case SpellEffect::ResistPoison:
			{
				new_bonus->PR += effect_value;
				break;
			}

			case SpellEffect::ResistDisease:
			{
				new_bonus->DR += effect_value;
				break;
			}

			case SpellEffect::ResistMagic:
			{
				new_bonus->MR += effect_value;
				break;
			}

			case SpellEffect::ResistAll:
			{
				new_bonus->MR += effect_value;
				new_bonus->DR += effect_value;
				new_bonus->PR += effect_value;
				new_bonus->CR += effect_value;
				new_bonus->FR += effect_value;
				break;
			}

			case SpellEffect::ResistCorruption:
			{
				new_bonus->Corrup += effect_value;
				break;
			}

			case SpellEffect::RaiseStatCap:
			{
				switch(spells[spell_id].limit_value[i])
				{
					//are these #define'd somewhere?
					case 0: //str
						new_bonus->STRCapMod += effect_value;
						break;
					case 1: //sta
						new_bonus->STACapMod += effect_value;
						break;
					case 2: //agi
						new_bonus->AGICapMod += effect_value;
						break;
					case 3: //dex
						new_bonus->DEXCapMod += effect_value;
						break;
					case 4: //wis
						new_bonus->WISCapMod += effect_value;
						break;
					case 5: //int
						new_bonus->INTCapMod += effect_value;
						break;
					case 6: //cha
						new_bonus->CHACapMod += effect_value;
						break;
					case 7: //mr
						new_bonus->MRCapMod += effect_value;
						break;
					case 8: //cr
						new_bonus->CRCapMod += effect_value;
						break;
					case 9: //fr
						new_bonus->FRCapMod += effect_value;
						break;
					case 10: //pr
						new_bonus->PRCapMod += effect_value;
						break;
					case 11: //dr
						new_bonus->DRCapMod += effect_value;
						break;
					case 12: // corruption
						new_bonus->CorrupCapMod += effect_value;
						break;
				}
				break;
			}

			case SpellEffect::CastingLevel:	// Brilliance of Ro
			{
				new_bonus->adjusted_casting_skill += effect_value;
				break;
			}

			case SpellEffect::CastingLevel2:
			{
				new_bonus->effective_casting_level += effect_value;

				if (RuleB(Spells, SnareOverridesSpeedBonuses) && effect_value < 0) {
					new_bonus->movementspeed = effect_value;
				}
				break;
			}

			case SpellEffect::MovementSpeed:
				new_bonus->movementspeed += effect_value;
				break;

			case SpellEffect::SpellDamageShield:
				new_bonus->SpellDamageShield += effect_value;
				break;

			case SpellEffect::DamageShield:
			{
				new_bonus->DamageShield += effect_value;
				new_bonus->DamageShieldSpellID = spell_id;
				//When using npc_spells_effects MAX value can be set to determine DS Type
				if (IsAISpellEffect && max_value)
					new_bonus->DamageShieldType = GetDamageShieldType(spell_id, max_value);
				else
					new_bonus->DamageShieldType = GetDamageShieldType(spell_id);

				break;
			}

			case SpellEffect::ReverseDS:
			{
				new_bonus->ReverseDamageShield += effect_value;
				new_bonus->ReverseDamageShieldSpellID = spell_id;

				if (IsAISpellEffect && max_value)
					new_bonus->ReverseDamageShieldType = GetDamageShieldType(spell_id, max_value);
				else
					new_bonus->ReverseDamageShieldType = GetDamageShieldType(spell_id);
				break;
			}

			case SpellEffect::Reflect:

				if (AdditiveWornBonus) {
					new_bonus->reflect[SBIndex::REFLECT_CHANCE] += effect_value;
				}

				else if (new_bonus->reflect[SBIndex::REFLECT_CHANCE] < effect_value) {
					new_bonus->reflect[SBIndex::REFLECT_CHANCE] = effect_value;
					new_bonus->reflect[SBIndex::REFLECT_RESISTANCE_MOD] = limit_value;
					new_bonus->reflect[SBIndex::REFLECT_DMG_EFFECTIVENESS] = max_value;
				}
				break;

			case SpellEffect::Amplification:
				new_bonus->Amplification += effect_value;
				break;

			case SpellEffect::ChangeAggro:
				new_bonus->hatemod += effect_value;
				break;

			case SpellEffect::MeleeMitigation:
				// This value is negative because it counteracts another SPA :P
				new_bonus->MeleeMitigationEffect += effect_value;
				break;

			case SpellEffect::CriticalHitChance:
			{
				// Bad data or unsupported new skill
				if (limit_value > EQ::skills::HIGHEST_SKILL)
					break;
				if (AdditiveWornBonus) {
					if(limit_value == ALL_SKILLS)
						new_bonus->CriticalHitChance[EQ::skills::HIGHEST_SKILL + 1] += effect_value;
					else
						new_bonus->CriticalHitChance[limit_value] += effect_value;
				}

				else if(effect_value < 0) {

					if (limit_value == ALL_SKILLS && new_bonus->CriticalHitChance[EQ::skills::HIGHEST_SKILL + 1] > effect_value)
						new_bonus->CriticalHitChance[EQ::skills::HIGHEST_SKILL + 1] = effect_value;
					else if(limit_value != ALL_SKILLS && new_bonus->CriticalHitChance[limit_value] > effect_value)
						new_bonus->CriticalHitChance[limit_value] = effect_value;
				}


				else if (limit_value == ALL_SKILLS && new_bonus->CriticalHitChance[EQ::skills::HIGHEST_SKILL + 1] < effect_value)
					new_bonus->CriticalHitChance[EQ::skills::HIGHEST_SKILL + 1] = effect_value;
				else if(limit_value != ALL_SKILLS && new_bonus->CriticalHitChance[limit_value] < effect_value)
						new_bonus->CriticalHitChance[limit_value] = effect_value;

				break;
			}

			case SpellEffect::CrippBlowChance:
			{
				if (AdditiveWornBonus)
					new_bonus->CrippBlowChance += effect_value;

				else if((effect_value < 0) && (new_bonus->CrippBlowChance > effect_value))
						new_bonus->CrippBlowChance = effect_value;

				else if(new_bonus->CrippBlowChance < effect_value)
					new_bonus->CrippBlowChance = effect_value;

				break;
			}

			case SpellEffect::AvoidMeleeChance:
			{
				if (AdditiveWornBonus)
					new_bonus->AvoidMeleeChanceEffect += effect_value;

				else if((effect_value < 0) && (new_bonus->AvoidMeleeChanceEffect > effect_value))
					new_bonus->AvoidMeleeChanceEffect = effect_value;

				else if(new_bonus->AvoidMeleeChanceEffect < effect_value)
					new_bonus->AvoidMeleeChanceEffect = effect_value;
				break;
			}

			case SpellEffect::RiposteChance:
			{
				if (AdditiveWornBonus)
					new_bonus->RiposteChance += effect_value;

				else if((effect_value < 0) && (new_bonus->RiposteChance > effect_value))
					new_bonus->RiposteChance = effect_value;

				else if(new_bonus->RiposteChance < effect_value)
					new_bonus->RiposteChance = effect_value;
				break;
			}

			case SpellEffect::DodgeChance:
			{
				if (AdditiveWornBonus)
					new_bonus->DodgeChance += effect_value;

				else if((effect_value < 0) && (new_bonus->DodgeChance > effect_value))
					new_bonus->DodgeChance = effect_value;

				if(new_bonus->DodgeChance < effect_value)
					new_bonus->DodgeChance = effect_value;
				break;
			}

			case SpellEffect::ParryChance:
			{
				if (AdditiveWornBonus)
					new_bonus->ParryChance += effect_value;

				else if((effect_value < 0) && (new_bonus->ParryChance > effect_value))
					new_bonus->ParryChance = effect_value;

				if(new_bonus->ParryChance < effect_value)
					new_bonus->ParryChance = effect_value;
				break;
			}

			case SpellEffect::DualWieldChance:
			{
				if (AdditiveWornBonus)
					new_bonus->DualWieldChance += effect_value;

				else if((effect_value < 0) && (new_bonus->DualWieldChance > effect_value))
					new_bonus->DualWieldChance = effect_value;

				if(new_bonus->DualWieldChance < effect_value)
					new_bonus->DualWieldChance = effect_value;
				break;
			}

			case SpellEffect::DoubleAttackChance:
			{

				if (AdditiveWornBonus)
					new_bonus->DoubleAttackChance += effect_value;

				else if((effect_value < 0) && (new_bonus->DoubleAttackChance > effect_value))
					new_bonus->DoubleAttackChance = effect_value;

				if(new_bonus->DoubleAttackChance < effect_value)
					new_bonus->DoubleAttackChance = effect_value;
				break;
			}

			case SpellEffect::TripleAttackChance:
			{

				if (AdditiveWornBonus)
					new_bonus->TripleAttackChance += effect_value;

				else if((effect_value < 0) && (new_bonus->TripleAttackChance > effect_value))
					new_bonus->TripleAttackChance = effect_value;

				if(new_bonus->TripleAttackChance < effect_value)
					new_bonus->TripleAttackChance = effect_value;
				break;
			}

			case SpellEffect::MeleeLifetap:
			{
				if (AdditiveWornBonus)
					new_bonus->MeleeLifetap += spells[spell_id].base_value[i];

				else if((effect_value < 0) && (new_bonus->MeleeLifetap > effect_value))
					new_bonus->MeleeLifetap = effect_value;

				else if(new_bonus->MeleeLifetap < effect_value)
					new_bonus->MeleeLifetap = effect_value;
				break;
			}

			case SpellEffect::Vampirism:
				new_bonus->Vampirism += effect_value;
				break;

			case SpellEffect::AllInstrumentMod:
			{
				if(effect_value > new_bonus->singingMod)
					new_bonus->singingMod = effect_value;
				if(effect_value > new_bonus->brassMod)
					new_bonus->brassMod = effect_value;
				if(effect_value > new_bonus->percussionMod)
					new_bonus->percussionMod = effect_value;
				if(effect_value > new_bonus->windMod)
					new_bonus->windMod = effect_value;
				if(effect_value > new_bonus->stringedMod)
					new_bonus->stringedMod = effect_value;
				break;
			}

			case SpellEffect::ResistSpellChance:
				new_bonus->ResistSpellChance += effect_value;
				break;

			case SpellEffect::ResistFearChance:
			{
				new_bonus->ResistFearChance += effect_value; // these should stack
				break;
			}

			case SpellEffect::Fearless:
				new_bonus->Fearless = true;
				break;

			case SpellEffect::HundredHands:
			{
				if (AdditiveWornBonus)
					new_bonus->HundredHands += effect_value;

				if (effect_value > 0 && effect_value > new_bonus->HundredHands)
					new_bonus->HundredHands = effect_value; //Increase Weapon Delay
				else if (effect_value < 0 && effect_value < new_bonus->HundredHands)
					new_bonus->HundredHands = effect_value; //Decrease Weapon Delay
				break;
			}

			case SpellEffect::MeleeSkillCheck:
			{
				if(new_bonus->MeleeSkillCheck < effect_value) {
					new_bonus->MeleeSkillCheck = effect_value;
					new_bonus->MeleeSkillCheckSkill = limit_value==ALL_SKILLS?255:limit_value;
				}
				break;
			}

			case SpellEffect::HitChance:
			{
				// Bad data or unsupported new skill
				if (limit_value > EQ::skills::HIGHEST_SKILL)
					break;

				if (AdditiveWornBonus){
					if(limit_value == ALL_SKILLS)
						new_bonus->HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1] += effect_value;
					else
						new_bonus->HitChanceEffect[limit_value] += effect_value;
				}

				else if(limit_value == ALL_SKILLS){

					if ((effect_value < 0) && (new_bonus->HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1] > effect_value))
						new_bonus->HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1] = effect_value;

					else if (!new_bonus->HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1] ||
						((new_bonus->HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1] > 0) && (new_bonus->HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1] < effect_value)))
						new_bonus->HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1] = effect_value;
				}

				else {

					if ((effect_value < 0) && (new_bonus->HitChanceEffect[limit_value] > effect_value))
						new_bonus->HitChanceEffect[limit_value] = effect_value;

					else if (!new_bonus->HitChanceEffect[limit_value] ||
							((new_bonus->HitChanceEffect[limit_value] > 0) && (new_bonus->HitChanceEffect[limit_value] < effect_value)))
							new_bonus->HitChanceEffect[limit_value] = effect_value;
				}

				break;

			}

			case SpellEffect::DamageModifier:
			{
				// Bad data or unsupported new skill
				if (limit_value > EQ::skills::HIGHEST_SKILL)
					break;
				int skill = limit_value == ALL_SKILLS ? EQ::skills::HIGHEST_SKILL + 1 : limit_value;
				if (effect_value < 0 && new_bonus->DamageModifier[skill] > effect_value)
					new_bonus->DamageModifier[skill] = effect_value;
				else if (effect_value > 0 && new_bonus->DamageModifier[skill] < effect_value)
					new_bonus->DamageModifier[skill] = effect_value;
				break;
			}

			case SpellEffect::DamageModifier2:
			{
				// Bad data or unsupported new skill
				if (limit_value > EQ::skills::HIGHEST_SKILL)
					break;
				int skill = limit_value == ALL_SKILLS ? EQ::skills::HIGHEST_SKILL + 1 : limit_value;
				if (effect_value < 0 && new_bonus->DamageModifier2[skill] > effect_value)
					new_bonus->DamageModifier2[skill] = effect_value;
				else if (effect_value > 0 && new_bonus->DamageModifier2[skill] < effect_value)
					new_bonus->DamageModifier2[skill] = effect_value;
				break;
			}

			case SpellEffect::Skill_Base_Damage_Mod:
			{
				// Bad data or unsupported new skill
				if (limit_value > EQ::skills::HIGHEST_SKILL)
					break;
				int skill = limit_value == ALL_SKILLS ? EQ::skills::HIGHEST_SKILL + 1 : limit_value;
				if (effect_value < 0 && new_bonus->DamageModifier3[skill] > effect_value)
					new_bonus->DamageModifier3[skill] = effect_value;
				else if (effect_value > 0 && new_bonus->DamageModifier3[skill] < effect_value)
					new_bonus->DamageModifier3[skill] = effect_value;
				break;
			}

			case SpellEffect::MinDamageModifier:
			{
				// Bad data or unsupported new skill
				if (limit_value > EQ::skills::HIGHEST_SKILL)
					break;
				int skill = limit_value == ALL_SKILLS ? EQ::skills::HIGHEST_SKILL + 1 : limit_value;
				if (effect_value < 0 && new_bonus->MinDamageModifier[skill] > effect_value)
					new_bonus->MinDamageModifier[skill] = effect_value;
				else if (effect_value > 0 && new_bonus->MinDamageModifier[skill] < effect_value)
					new_bonus->MinDamageModifier[skill] = effect_value;
				break;
			}

			case SpellEffect::ReduceSkill: {
				// Bad data or unsupported new skill
				if (spells[spell_id].base_value[i] > EQ::skills::HIGHEST_SKILL) {
					break;
				}
				//cap skill reducation at 100%
				uint32 skill_reducation_percent = spells[spell_id].formula[i];
				if (spells[spell_id].formula[i] > 100) {
					skill_reducation_percent = 100;
				}

				if (spells[spell_id].base_value[i] <= EQ::skills::HIGHEST_SKILL) {
					if (new_bonus->ReduceSkill[spells[spell_id].base_value[i]] < skill_reducation_percent) {
						new_bonus->ReduceSkill[spells[spell_id].base_value[i]] = skill_reducation_percent;
					}
				}
				break;
			}

			case SpellEffect::StunResist:
			{
				if(new_bonus->StunResist < effect_value)
					new_bonus->StunResist = effect_value;
				break;
			}

			case SpellEffect::ProcChance:
			{
				if (AdditiveWornBonus)
					new_bonus->ProcChanceSPA += effect_value;

				else if((effect_value < 0) && (new_bonus->ProcChanceSPA > effect_value))
					new_bonus->ProcChanceSPA = effect_value;

				if(new_bonus->ProcChanceSPA < effect_value)
					new_bonus->ProcChanceSPA = effect_value;

				break;
			}

			case SpellEffect::ExtraAttackChance:
			{
				if (AdditiveWornBonus) {
					new_bonus->ExtraAttackChance[SBIndex::EXTRA_ATTACK_CHANCE] += effect_value;
					new_bonus->ExtraAttackChance[SBIndex::EXTRA_ATTACK_NUM_ATKS] = limit_value ? limit_value : 1;
				}
				if (new_bonus->ExtraAttackChance[SBIndex::EXTRA_ATTACK_CHANCE] < effect_value) {
					new_bonus->ExtraAttackChance[SBIndex::EXTRA_ATTACK_CHANCE]   = effect_value;
					new_bonus->ExtraAttackChance[SBIndex::EXTRA_ATTACK_NUM_ATKS] = limit_value ? limit_value : 1;
				}
				break;
			}

			case SpellEffect::AddExtraAttackPct_1h_Primary:
			{
				if (AdditiveWornBonus) {
					new_bonus->ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_CHANCE] += effect_value;
					new_bonus->ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_NUM_ATKS] = limit_value ? limit_value : 1;
				}

				if (new_bonus->ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_CHANCE] < effect_value) {
					new_bonus->ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_CHANCE]   = effect_value;
					new_bonus->ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_NUM_ATKS] = limit_value ? limit_value : 1;
				}
				break;
			}

			case SpellEffect::AddExtraAttackPct_1h_Secondary:
			{
				if (AdditiveWornBonus) {
					new_bonus->ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_CHANCE] += effect_value;
					new_bonus->ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_NUM_ATKS] = limit_value ? limit_value : 1;
				}

				if (new_bonus->ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_CHANCE] < effect_value) {
					new_bonus->ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_CHANCE]   = effect_value;
					new_bonus->ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_NUM_ATKS] = limit_value ? limit_value : 1;
				}
				break;
			}

			case SpellEffect::Double_Melee_Round:
			{
				if (AdditiveWornBonus) {
					new_bonus->DoubleMeleeRound[SBIndex::DOUBLE_MELEE_ROUND_CHANCE] += effect_value;
					new_bonus->DoubleMeleeRound[SBIndex::DOUBLE_MELEE_ROUND_DMG_BONUS] += limit_value;
				}

				if (new_bonus->DoubleMeleeRound[SBIndex::DOUBLE_MELEE_ROUND_CHANCE] < effect_value) {
					new_bonus->DoubleMeleeRound[SBIndex::DOUBLE_MELEE_ROUND_CHANCE] = effect_value;
					new_bonus->DoubleMeleeRound[SBIndex::DOUBLE_MELEE_ROUND_DMG_BONUS] = limit_value;
				}
				break;
			}

			case SpellEffect::PercentXPIncrease:
			{
				if(new_bonus->XPRateMod < effect_value)
					new_bonus->XPRateMod = effect_value;
				break;
			}

			case SpellEffect::DeathSave:
			{
				if(new_bonus->DeathSave[SBIndex::DEATH_SAVE_TYPE] < effect_value)
				{
					new_bonus->DeathSave[SBIndex::DEATH_SAVE_TYPE]               = effect_value; //1='Partial' 2='Full'
					new_bonus->DeathSave[SBIndex::DEATH_SAVE_BUFFSLOT]           = buffslot;
					//These are used in later expansion spell effects.
					new_bonus->DeathSave[SBIndex::DEATH_SAVE_MIN_LEVEL_FOR_HEAL] = limit_value;//Min level for HealAmt
					new_bonus->DeathSave[SBIndex::DEATH_SAVE_HEAL_AMT]           = max_value;//HealAmt
				}
				break;
			}

			case SpellEffect::DivineSave:
			{
				if (AdditiveWornBonus) {
					new_bonus->DivineSaveChance[SBIndex::DIVINE_SAVE_CHANCE] += effect_value;
					new_bonus->DivineSaveChance[SBIndex::DIVINE_SAVE_SPELL_TRIGGER_ID] = 0;
				}

				else if(new_bonus->DivineSaveChance[SBIndex::DIVINE_SAVE_CHANCE] < effect_value)
				{
					new_bonus->DivineSaveChance[SBIndex::DIVINE_SAVE_CHANCE]           = effect_value;
					new_bonus->DivineSaveChance[SBIndex::DIVINE_SAVE_SPELL_TRIGGER_ID] = limit_value;
				}
				break;
			}

			case SpellEffect::Flurry:
				new_bonus->FlurryChance += effect_value;
				break;

			case SpellEffect::Accuracy:
			{
				// Bad data or unsupported new skill
				if (limit_value > EQ::skills::HIGHEST_SKILL)
					break;
				if ((effect_value < 0) && (new_bonus->Accuracy[EQ::skills::HIGHEST_SKILL + 1] > effect_value))
					new_bonus->Accuracy[EQ::skills::HIGHEST_SKILL + 1] = effect_value;

				else if (!new_bonus->Accuracy[EQ::skills::HIGHEST_SKILL + 1] ||
					((new_bonus->Accuracy[EQ::skills::HIGHEST_SKILL + 1] > 0) && (new_bonus->Accuracy[EQ::skills::HIGHEST_SKILL + 1] < effect_value)))
					new_bonus->Accuracy[EQ::skills::HIGHEST_SKILL + 1] = effect_value;
				break;
			}

			case SpellEffect::MaxHPChange:
				new_bonus->PercentMaxHPChange += effect_value;
				break;

			case SpellEffect::EndurancePool:
				new_bonus->Endurance += effect_value;
				break;

			case SpellEffect::HealRate:
				new_bonus->HealRate += effect_value;
				break;

			case SpellEffect::SkillDamageTaken:
			{
				// Bad data or unsupported new skill
				if (limit_value > EQ::skills::HIGHEST_SKILL)
					break;
				//When using npc_spells_effects if MAX value set, use stackable quest based modifier.
				if (IsAISpellEffect && max_value){
					if(limit_value == ALL_SKILLS)
						SkillDmgTaken_Mod[EQ::skills::HIGHEST_SKILL + 1] = effect_value;
					else
						SkillDmgTaken_Mod[limit_value] = effect_value;
				}
				else {

					if(limit_value == ALL_SKILLS)
						new_bonus->SkillDmgTaken[EQ::skills::HIGHEST_SKILL + 1] += effect_value;
					else
						new_bonus->SkillDmgTaken[limit_value] += effect_value;

				}
				break;
			}

			case SpellEffect::SpellCritChance:
				new_bonus->CriticalSpellChance += effect_value;
				break;

			case SpellEffect::CriticalSpellChance:
			{
				new_bonus->CriticalSpellChance += effect_value;

				if (limit_value > new_bonus->SpellCritDmgIncNoStack)
					new_bonus->SpellCritDmgIncNoStack = limit_value;
				break;
			}

			case SpellEffect::SpellCritDmgIncrease:
				new_bonus->SpellCritDmgIncrease += effect_value;
				break;

			case SpellEffect::DotCritDmgIncrease:
				new_bonus->DotCritDmgIncrease += effect_value;
				break;

			case SpellEffect::CriticalHealChance:
				new_bonus->CriticalHealChance += effect_value;
				break;

			case SpellEffect::CriticalHealOverTime:
				new_bonus->CriticalHealOverTime += effect_value;
				break;

			case SpellEffect::CriticalHealDecay:
				new_bonus->CriticalHealDecay = true;
				break;

			case SpellEffect::CriticalRegenDecay:
				new_bonus->CriticalRegenDecay = true;
				break;

			case SpellEffect::CriticalDotDecay:
				new_bonus->CriticalDotDecay = true;
				break;

			case SpellEffect::MitigateDamageShield:
			{
				/*
				Bard songs have identical negative base value and positive max
				The effect for the songs should increase mitigation. There are
				spells that do decrease the mitigation with just negative base values.
				To be consistent all values that increase mitigation will be set to positives
				*/
				if (max_value > 0 && effect_value < 0) {
					effect_value = max_value;
				}

				new_bonus->DSMitigationOffHand += effect_value;
				break;
			}

			case SpellEffect::CriticalDoTChance:
				new_bonus->CriticalDoTChance += effect_value;
				break;

			case SpellEffect::ProcOnKillShot:
			{
				for(int e = 0; e < MAX_SPELL_TRIGGER*3; e+=3)
				{
					if(!new_bonus->SpellOnKill[e])
					{
						// Base2 = Spell to fire | Base1 = % chance | Base3 = min level
						new_bonus->SpellOnKill[e] = limit_value;
						new_bonus->SpellOnKill[e+1] = effect_value;
						new_bonus->SpellOnKill[e+2] = max_value;
						break;
					}
				}
				break;
			}

			case SpellEffect::SpellOnDeath:
			{
				for(int e = 0; e < MAX_SPELL_TRIGGER; e+=2)
				{
					if(!new_bonus->SpellOnDeath[e])
					{
						// Base2 = Spell to fire | Base1 = % chance
						new_bonus->SpellOnDeath[e] = limit_value;
						new_bonus->SpellOnDeath[e+1] = effect_value;
						break;
					}
				}
				break;
			}

			case SpellEffect::CriticalDamageMob:
			{
				// Bad data or unsupported new skill
				if (limit_value > EQ::skills::HIGHEST_SKILL)
					break;
				if(limit_value == ALL_SKILLS)
					new_bonus->CritDmgMod[EQ::skills::HIGHEST_SKILL + 1] += effect_value;
				else
					new_bonus->CritDmgMod[limit_value] += effect_value;
				break;
			}

			case SpellEffect::Critical_Melee_Damage_Mod_Max:
			{
				// Bad data or unsupported new skill
				if (limit_value > EQ::skills::HIGHEST_SKILL)
					break;
				int skill = limit_value == ALL_SKILLS ? EQ::skills::HIGHEST_SKILL + 1 : limit_value;
				if (effect_value < 0 && new_bonus->CritDmgModNoStack[skill] > effect_value)
					new_bonus->CritDmgModNoStack[skill] = effect_value;
				else if (effect_value > 0 && new_bonus->CritDmgModNoStack[skill] < effect_value) {
					new_bonus->CritDmgModNoStack[skill] = effect_value;
				}
				break;
			}

			case SpellEffect::ReduceSkillTimer:
			{
				if(new_bonus->SkillReuseTime[limit_value] < effect_value)
					new_bonus->SkillReuseTime[limit_value] = effect_value;
				break;
			}

			case SpellEffect::SkillDamageAmount:
			{
				// Bad data or unsupported new skill
				if (limit_value > EQ::skills::HIGHEST_SKILL)
					break;
				if(limit_value == ALL_SKILLS)
					new_bonus->SkillDamageAmount[EQ::skills::HIGHEST_SKILL + 1] += effect_value;
				else
					new_bonus->SkillDamageAmount[limit_value] += effect_value;
				break;
			}

			case SpellEffect::GravityEffect:
				new_bonus->GravityEffect = 1;
				break;

			case SpellEffect::AntiGate:
				new_bonus->AntiGate = true;
				break;

			case SpellEffect::MagicWeapon:
				new_bonus->MagicWeapon = true;
				break;

			case SpellEffect::Hunger:
				new_bonus->hunger = true;
				break;

			case SpellEffect::IncreaseBlockChance:
				if (AdditiveWornBonus)
					new_bonus->IncreaseBlockChance += effect_value;
				else if (effect_value < 0 && new_bonus->IncreaseBlockChance > effect_value)
					new_bonus->IncreaseBlockChance = effect_value;
				else if (new_bonus->IncreaseBlockChance < effect_value)
					new_bonus->IncreaseBlockChance = effect_value;
				break;

			case SpellEffect::PersistantCasting:
				new_bonus->PersistantCasting += effect_value;
				break;

			case SpellEffect::LimitHPPercent:
			{
				if(new_bonus->HPPercCap[SBIndex::RESOURCE_PERCENT_CAP] != 0 && new_bonus->HPPercCap[SBIndex::RESOURCE_PERCENT_CAP] > effect_value){
					new_bonus->HPPercCap[SBIndex::RESOURCE_PERCENT_CAP] = effect_value;
					new_bonus->HPPercCap[SBIndex::RESOURCE_AMOUNT_CAP]  = limit_value;
				}
				else if(new_bonus->HPPercCap[SBIndex::RESOURCE_PERCENT_CAP] == 0){
					new_bonus->HPPercCap[SBIndex::RESOURCE_PERCENT_CAP] = effect_value;
					new_bonus->HPPercCap[SBIndex::RESOURCE_AMOUNT_CAP]  = limit_value;
				}
				break;
			}
			case SpellEffect::LimitManaPercent:
			{
				if(new_bonus->ManaPercCap[SBIndex::RESOURCE_PERCENT_CAP] != 0 && new_bonus->ManaPercCap[SBIndex::RESOURCE_PERCENT_CAP] > effect_value){
					new_bonus->ManaPercCap[SBIndex::RESOURCE_PERCENT_CAP] = effect_value;
					new_bonus->ManaPercCap[SBIndex::RESOURCE_AMOUNT_CAP]  = limit_value;
				}
				else if(new_bonus->ManaPercCap[SBIndex::RESOURCE_PERCENT_CAP] == 0) {
					new_bonus->ManaPercCap[SBIndex::RESOURCE_PERCENT_CAP] = effect_value;
					new_bonus->ManaPercCap[SBIndex::RESOURCE_AMOUNT_CAP]  = limit_value;
				}

				break;
			}
			case SpellEffect::LimitEndPercent:
			{
				if(new_bonus->EndPercCap[SBIndex::RESOURCE_PERCENT_CAP] != 0 && new_bonus->EndPercCap[SBIndex::RESOURCE_PERCENT_CAP] > effect_value) {
					new_bonus->EndPercCap[SBIndex::RESOURCE_PERCENT_CAP] = effect_value;
					new_bonus->EndPercCap[SBIndex::RESOURCE_AMOUNT_CAP]  = limit_value;
				}

				else if(new_bonus->EndPercCap[SBIndex::RESOURCE_PERCENT_CAP] == 0){
					new_bonus->EndPercCap[SBIndex::RESOURCE_PERCENT_CAP] = effect_value;
					new_bonus->EndPercCap[SBIndex::RESOURCE_AMOUNT_CAP]  = limit_value;
				}

				break;
			}

			case SpellEffect::NegateSpellEffect:
				new_bonus->NegateEffects = true;
				break;

			case SpellEffect::ImmuneFleeing:
				new_bonus->ImmuneToFlee = true;
				if (currently_fleeing) // lets update shit now instead of next tick
					ProcessFlee();
				break;

			case SpellEffect::DelayDeath:
				new_bonus->DelayDeath += effect_value;
				break;

			case SpellEffect::SpellProcChance:
				new_bonus->SpellProcChance += effect_value;
				break;

			case SpellEffect::CharmBreakChance:
				new_bonus->CharmBreakChance += effect_value;
				break;

			case SpellEffect::BardSongRange:
				new_bonus->SongRange += effect_value;
				break;

			case SpellEffect::HPToMana:
			{
				//Lower the ratio the more favorable
				if ((!new_bonus->HPToManaConvert) || (new_bonus->HPToManaConvert >= effect_value)) {
					new_bonus->HPToManaConvert = spells[spell_id].base_value[i];
				}
				break;
			}

			case SpellEffect::SkillDamageAmount2:
			{
				// Bad data or unsupported new skill
				if (limit_value > EQ::skills::HIGHEST_SKILL)
					break;
				if(limit_value == ALL_SKILLS)
					new_bonus->SkillDamageAmount2[EQ::skills::HIGHEST_SKILL + 1] += effect_value;
				else
					new_bonus->SkillDamageAmount2[limit_value] += effect_value;
				break;
			}

			case SpellEffect::NegateAttacks:
			{
				if (
					!new_bonus->NegateAttacks[SBIndex::NEGATE_ATK_EXISTS] ||
					(
						new_bonus->NegateAttacks[SBIndex::NEGATE_ATK_MAX_DMG_ABSORB_PER_HIT] &&
						new_bonus->NegateAttacks[SBIndex::NEGATE_ATK_MAX_DMG_ABSORB_PER_HIT] < max_value
					)
				) {
					new_bonus->NegateAttacks[SBIndex::NEGATE_ATK_EXISTS]                 = 1;
					new_bonus->NegateAttacks[SBIndex::NEGATE_ATK_BUFFSLOT]               = buffslot;
					new_bonus->NegateAttacks[SBIndex::NEGATE_ATK_MAX_DMG_ABSORB_PER_HIT] = max_value;
				}
				break;
			}

			case SpellEffect::MitigateMeleeDamage:
			{
				if (new_bonus->MitigateMeleeRune[SBIndex::MITIGATION_RUNE_PERCENT] < effect_value){
					new_bonus->MitigateMeleeRune[SBIndex::MITIGATION_RUNE_PERCENT]                = effect_value;
					new_bonus->MitigateMeleeRune[SBIndex::MITIGATION_RUNE_BUFFSLOT]               = buffslot;
					new_bonus->MitigateMeleeRune[SBIndex::MITIGATION_RUNE_MAX_DMG_ABSORB_PER_HIT] = limit_value;
					new_bonus->MitigateMeleeRune[SBIndex::MITIGATION_RUNE_MAX_HP_AMT]             = max_value;
				}
				break;
			}


			case SpellEffect::MeleeThresholdGuard:
			{
				if (new_bonus->MeleeThresholdGuard[SBIndex::THRESHOLDGUARD_MITIGATION_PERCENT] < effect_value){
					new_bonus->MeleeThresholdGuard[SBIndex::THRESHOLDGUARD_MITIGATION_PERCENT]  = effect_value;
					new_bonus->MeleeThresholdGuard[SBIndex::THRESHOLDGUARD_BUFFSLOT]           = buffslot;
					new_bonus->MeleeThresholdGuard[SBIndex::THRESHOLDGUARD_MIN_DMG_TO_TRIGGER] = limit_value;
				}
				break;
			}

			case SpellEffect::SpellThresholdGuard:
			{
				if (new_bonus->SpellThresholdGuard[SBIndex::THRESHOLDGUARD_MITIGATION_PERCENT] < effect_value){
					new_bonus->SpellThresholdGuard[SBIndex::THRESHOLDGUARD_MITIGATION_PERCENT]  = effect_value;
					new_bonus->SpellThresholdGuard[SBIndex::THRESHOLDGUARD_BUFFSLOT]           = buffslot;
					new_bonus->SpellThresholdGuard[SBIndex::THRESHOLDGUARD_MIN_DMG_TO_TRIGGER] = limit_value;
				}
				break;
			}

			case SpellEffect::MitigateSpellDamage:
			{
				if (WornType) {
					new_bonus->MitigateSpellRune[SBIndex::MITIGATION_RUNE_PERCENT] += effect_value;
				}

				if (new_bonus->MitigateSpellRune[SBIndex::MITIGATION_RUNE_PERCENT] < effect_value){
					new_bonus->MitigateSpellRune[SBIndex::MITIGATION_RUNE_PERCENT]                = effect_value;
					new_bonus->MitigateSpellRune[SBIndex::MITIGATION_RUNE_BUFFSLOT]               = buffslot;
					new_bonus->MitigateSpellRune[SBIndex::MITIGATION_RUNE_MAX_DMG_ABSORB_PER_HIT] = limit_value;
					new_bonus->MitigateSpellRune[SBIndex::MITIGATION_RUNE_MAX_HP_AMT]             = max_value;
				}
				break;
			}

			case SpellEffect::MitigateDotDamage:
			{
				if (WornType) {
					new_bonus->MitigateDotRune[SBIndex::MITIGATION_RUNE_PERCENT] += effect_value;
				}

				if (new_bonus->MitigateDotRune[SBIndex::MITIGATION_RUNE_PERCENT] < effect_value){
					new_bonus->MitigateDotRune[SBIndex::MITIGATION_RUNE_PERCENT]                = effect_value;
					new_bonus->MitigateDotRune[SBIndex::MITIGATION_RUNE_BUFFSLOT]               = buffslot;
					new_bonus->MitigateDotRune[SBIndex::MITIGATION_RUNE_MAX_DMG_ABSORB_PER_HIT] = limit_value;
					new_bonus->MitigateDotRune[SBIndex::MITIGATION_RUNE_MAX_HP_AMT]             = max_value;
				}
				break;
			}

			case SpellEffect::ManaAbsorbPercentDamage:
			{
				if (new_bonus->ManaAbsorbPercentDamage < effect_value){
					new_bonus->ManaAbsorbPercentDamage = effect_value;
				}
				break;
			}

			case SpellEffect::Endurance_Absorb_Pct_Damage:
			{
				if (new_bonus->EnduranceAbsorbPercentDamage[SBIndex::ENDURANCE_ABSORD_MITIGIATION] < effect_value) {
					new_bonus->EnduranceAbsorbPercentDamage[SBIndex::ENDURANCE_ABSORD_MITIGIATION]  = effect_value;
					new_bonus->EnduranceAbsorbPercentDamage[SBIndex::ENDURANCE_ABSORD_DRAIN_PER_HP] = limit_value;
				}
				break;
			}

			case SpellEffect::Shield_Target:
			{
				if (new_bonus->ShieldTargetSpa[SBIndex::SHIELD_TARGET_MITIGATION_PERCENT] < effect_value) {
					new_bonus->ShieldTargetSpa[SBIndex::SHIELD_TARGET_MITIGATION_PERCENT] = effect_value;
					new_bonus->ShieldTargetSpa[SBIndex::SHIELD_TARGET_BUFFSLOT] = buffslot;
				}
				break;
			}

			case SpellEffect::TriggerMeleeThreshold:
				new_bonus->TriggerMeleeThreshold = true;
				break;

			case SpellEffect::TriggerSpellThreshold:
				new_bonus->TriggerSpellThreshold = true;
				break;

			case SpellEffect::ShieldBlock:
				new_bonus->ShieldBlock += effect_value;
				break;

			case SpellEffect::ShieldEquipDmgMod:
				new_bonus->ShieldEquipDmgMod += effect_value;
				break;

			case SpellEffect::BlockBehind:
				new_bonus->BlockBehind += effect_value;
				break;

			case SpellEffect::Blind:
				if (!RuleB(Combat, AllowRaidTargetBlind) && IsRaidTarget()) { // do not blind raid targets
					break;
				}

				new_bonus->IsBlind = true;
				break;

			case SpellEffect::Fear:
				new_bonus->IsFeared = true;
				break;

			//AA bonuses - implemented broadly into spell/item effects
			case SpellEffect::FrontalStunResist:
				new_bonus->FrontalStunResist += effect_value;
				break;

			case SpellEffect::ImprovedBindWound:
				new_bonus->BindWound += effect_value;
				break;

			case SpellEffect::MaxBindWound:
				new_bonus->MaxBindWound += effect_value;
				break;

			case SpellEffect::BaseMovementSpeed:
				new_bonus->BaseMovementSpeed += effect_value;
				break;

			case SpellEffect::IncreaseRunSpeedCap:
				new_bonus->IncreaseRunSpeedCap += effect_value;
				break;

			case SpellEffect::DoubleSpecialAttack:
				new_bonus->DoubleSpecialAttack += effect_value;
				break;

			case SpellEffect::TripleBackstab:
				new_bonus->TripleBackstab += effect_value;
				break;

			case SpellEffect::FrontalBackstabMinDmg:
				new_bonus->FrontalBackstabMinDmg = true;
				break;

			case SpellEffect::FrontalBackstabChance:
				new_bonus->FrontalBackstabChance += effect_value;
				break;

			case SpellEffect::Double_Backstab_Front:
				new_bonus->Double_Backstab_Front += effect_value;
				break;

			case SpellEffect::ConsumeProjectile:
				new_bonus->ConsumeProjectile += effect_value;
				break;

			case SpellEffect::ForageAdditionalItems:
				new_bonus->ForageAdditionalItems += effect_value;
				break;

			case SpellEffect::Salvage:
				new_bonus->SalvageChance += effect_value;
				break;

			case SpellEffect::ArcheryDamageModifier:
				new_bonus->ArcheryDamageModifier += effect_value;
				break;

			case SpellEffect::DoubleRangedAttack:
				new_bonus->DoubleRangedAttack += effect_value;
				break;

			case SpellEffect::SecondaryDmgInc:
				new_bonus->SecondaryDmgInc = true;
				break;

			case SpellEffect::StrikeThrough:
			case SpellEffect::StrikeThrough2:
				new_bonus->StrikeThrough += effect_value;
				break;

			case SpellEffect::GiveDoubleAttack:
				new_bonus->GiveDoubleAttack += effect_value;
				break;

			case SpellEffect::PetCriticalHit:
				new_bonus->PetCriticalHit += effect_value;
				break;

			case SpellEffect::CombatStability:
				new_bonus->CombatStability += effect_value;
				break;

			case SpellEffect::AddSingingMod:
				switch (limit_value) {
				case EQ::item::ItemTypeWindInstrument:
					new_bonus->windMod += effect_value;
					break;
				case EQ::item::ItemTypeStringedInstrument:
					new_bonus->stringedMod += effect_value;
					break;
				case EQ::item::ItemTypeBrassInstrument:
					new_bonus->brassMod += effect_value;
					break;
				case EQ::item::ItemTypePercussionInstrument:
					new_bonus->percussionMod += effect_value;
					break;
				case EQ::item::ItemTypeSinging:
					new_bonus->singingMod += effect_value;
					break;
				default:
					break;
				}
				break;

			case SpellEffect::SongModCap:
				new_bonus->songModCap += effect_value;
				break;

			case SpellEffect::PetAvoidance:
				new_bonus->PetAvoidance += effect_value;
				break;

			case SpellEffect::Ambidexterity:
				new_bonus->Ambidexterity += effect_value;
				break;

			case SpellEffect::PetMaxHP:
				new_bonus->PetMaxHP += effect_value;
				break;

			case SpellEffect::PetFlurry:
				new_bonus->PetFlurry += effect_value;
				break;

			case SpellEffect::GivePetGroupTarget:
				new_bonus->GivePetGroupTarget = true;
				break;

			case SpellEffect::RootBreakChance:
				new_bonus->RootBreakChance += effect_value;
				break;

			case SpellEffect::ChannelChanceItems:
				new_bonus->ChannelChanceItems += effect_value;
				break;

			case SpellEffect::ChannelChanceSpells:
				new_bonus->ChannelChanceSpells += effect_value;
				break;

			case SpellEffect::UnfailingDivinity:
				new_bonus->UnfailingDivinity += effect_value;
				break;


			case SpellEffect::ItemHPRegenCapIncrease:
				new_bonus->ItemHPRegenCap += effect_value;
				break;

			case SpellEffect::OffhandRiposteFail:
				new_bonus->OffhandRiposteFail += effect_value;
				break;

			case SpellEffect::ItemAttackCapIncrease:
				new_bonus->ItemATKCap += effect_value;
				break;

			case SpellEffect::TwoHandBluntBlock:
				new_bonus->TwoHandBluntBlock += effect_value;
				break;

			case SpellEffect::StunBashChance:
				new_bonus->StunBashChance += effect_value;
				break;

			case SpellEffect::IncreaseChanceMemwipe:
				new_bonus->IncreaseChanceMemwipe += effect_value;
				break;

			case SpellEffect::CriticalMend:
				new_bonus->CriticalMend += effect_value;
				break;

			case SpellEffect::SpellEffectResistChance:
			{
				for(int e = 0; e < MAX_RESISTABLE_EFFECTS*2; e+=2)
				{
					if (
						!new_bonus->SEResist[e + 1] ||
						(
							new_bonus->SEResist[e + 1] &&
							new_bonus->SEResist[e] == limit_value &&
							new_bonus->SEResist[e + 1] < effect_value
						)
					) {
						new_bonus->SEResist[e]     = limit_value; //Spell Effect ID
						new_bonus->SEResist[e + 1] = effect_value; //Resist Chance
						break;
					}
				}
				break;
			}

			case SpellEffect::MasteryofPast:
			{
				if(new_bonus->MasteryofPast < effect_value)
					new_bonus->MasteryofPast = effect_value;
				break;
			}

			case SpellEffect::DoubleRiposte:
			{
				new_bonus->DoubleRiposte += effect_value;
				break;
			}

			case SpellEffect::GiveDoubleRiposte:
			{
				//Only allow for regular double riposte chance.
				if(new_bonus->GiveDoubleRiposte[limit_value] == 0){
					if(new_bonus->GiveDoubleRiposte[SBIndex::DOUBLE_RIPOSTE_CHANCE] < effect_value)
						new_bonus->GiveDoubleRiposte[SBIndex::DOUBLE_RIPOSTE_CHANCE] = effect_value;
				}
				break;
			}

			case SpellEffect::SlayUndead: {
				if (new_bonus->SlayUndead[SBIndex::SLAYUNDEAD_DMG_MOD] < effect_value) {
					new_bonus->SlayUndead[SBIndex::SLAYUNDEAD_RATE_MOD] = limit_value; // Rate
					new_bonus->SlayUndead[SBIndex::SLAYUNDEAD_DMG_MOD] = effect_value; // Damage Modifier
				}
				break;
			}

			case SpellEffect::TriggerOnReqTarget:
			case SpellEffect::TriggerOnReqCaster:
				new_bonus->TriggerOnCastRequirement = true;
				break;

			case SpellEffect::DivineAura:
				new_bonus->DivineAura = true;
				break;

			case SpellEffect::ImprovedTaunt:
				if (new_bonus->ImprovedTaunt[SBIndex::IMPROVED_TAUNT_MAX_LVL] < effect_value) {
					new_bonus->ImprovedTaunt[SBIndex::IMPROVED_TAUNT_MAX_LVL]   = effect_value;
					new_bonus->ImprovedTaunt[SBIndex::IMPROVED_TAUNT_AGGRO_MOD] = limit_value;
					new_bonus->ImprovedTaunt[SBIndex::IMPROVED_TAUNT_BUFFSLOT]  = buffslot;
				}
				break;


			case SpellEffect::DistanceRemoval:
				new_bonus->DistanceRemoval = true;
				break;

			case SpellEffect::FrenziedDevastation:
				new_bonus->FrenziedDevastation += limit_value;
				break;

			case SpellEffect::Root:
				if (new_bonus->Root[SBIndex::ROOT_EXISTS] && (new_bonus->Root[SBIndex::ROOT_BUFFSLOT] > buffslot)){
					new_bonus->Root[SBIndex::ROOT_EXISTS]   = 1;
					new_bonus->Root[SBIndex::ROOT_BUFFSLOT] = buffslot;
				}
				else if (!new_bonus->Root[SBIndex::ROOT_EXISTS]){
					new_bonus->Root[SBIndex::ROOT_EXISTS]   = 1;
					new_bonus->Root[SBIndex::ROOT_BUFFSLOT] = buffslot;
				}
				break;

			case SpellEffect::Rune:

				if (new_bonus->MeleeRune[SBIndex::RUNE_AMOUNT] && (new_bonus->MeleeRune[SBIndex::RUNE_BUFFSLOT] > buffslot)){

					new_bonus->MeleeRune[SBIndex::RUNE_AMOUNT]   = effect_value;
					new_bonus->MeleeRune[SBIndex::RUNE_BUFFSLOT] = buffslot;
				}
				else if (!new_bonus->MeleeRune[SBIndex::RUNE_AMOUNT]){
					new_bonus->MeleeRune[SBIndex::RUNE_AMOUNT]   = effect_value;
					new_bonus->MeleeRune[SBIndex::RUNE_BUFFSLOT] = buffslot;
				}

				break;

			case SpellEffect::AbsorbMagicAtt:
				if (new_bonus->AbsorbMagicAtt[SBIndex::RUNE_AMOUNT] && (new_bonus->AbsorbMagicAtt[SBIndex::RUNE_BUFFSLOT] > buffslot)){
					new_bonus->AbsorbMagicAtt[SBIndex::RUNE_AMOUNT]   = effect_value;
					new_bonus->AbsorbMagicAtt[SBIndex::RUNE_BUFFSLOT] = buffslot;
				}
				else if (!new_bonus->AbsorbMagicAtt[SBIndex::RUNE_AMOUNT]){
					new_bonus->AbsorbMagicAtt[SBIndex::RUNE_AMOUNT]   = effect_value;
					new_bonus->AbsorbMagicAtt[SBIndex::RUNE_BUFFSLOT] = buffslot;
				}
				break;

			case SpellEffect::NegateIfCombat:
				new_bonus->NegateIfCombat = true;
				break;

			case SpellEffect::Screech:
				new_bonus->Screech = effect_value;
				break;

			case SpellEffect::AlterNPCLevel:

				if (IsNPC()){
					if (!new_bonus->AlterNPCLevel
					|| ((effect_value < 0) && (new_bonus->AlterNPCLevel > effect_value))
					|| ((effect_value > 0) && (new_bonus->AlterNPCLevel < effect_value))) {

						int tmp_lv =  GetOrigLevel() + effect_value;
						if (tmp_lv < 1)
							tmp_lv = 1;
						else if (tmp_lv > 255)
							tmp_lv = 255;
						if ((GetLevel() != tmp_lv)){
							new_bonus->AlterNPCLevel = effect_value;
							SetLevel(tmp_lv);
						}
					}
				}
				break;

			case SpellEffect::AStacker:
				new_bonus->AStacker[SBIndex::BUFFSTACKER_EXISTS] = 1;
				new_bonus->AStacker[SBIndex::BUFFSTACKER_VALUE]  = effect_value;
				break;

			case SpellEffect::BStacker:
				new_bonus->BStacker[SBIndex::BUFFSTACKER_EXISTS] = 1;
				new_bonus->BStacker[SBIndex::BUFFSTACKER_VALUE]  = effect_value;
				break;

			case SpellEffect::CStacker:
				new_bonus->CStacker[SBIndex::BUFFSTACKER_EXISTS] = 1;
				new_bonus->CStacker[SBIndex::BUFFSTACKER_VALUE]  = effect_value;
				break;

			case SpellEffect::DStacker:
				new_bonus->DStacker[SBIndex::BUFFSTACKER_EXISTS] = 1;
				new_bonus->DStacker[SBIndex::BUFFSTACKER_VALUE]  = effect_value;
				break;

			case SpellEffect::Berserk:
				new_bonus->BerserkSPA = true;
				break;


			case SpellEffect::Metabolism:
				new_bonus->Metabolism += effect_value;
				break;

			case SpellEffect::ImprovedReclaimEnergy:
			{
				if((effect_value < 0) && (new_bonus->ImprovedReclaimEnergy > effect_value))
					new_bonus->ImprovedReclaimEnergy = effect_value;

				else if(new_bonus->ImprovedReclaimEnergy < effect_value)
					new_bonus->ImprovedReclaimEnergy = effect_value;
				break;
			}

			case SpellEffect::HeadShot:
			{
				if(new_bonus->HeadShot[SBIndex::FINISHING_EFFECT_DMG] < limit_value){
					new_bonus->HeadShot[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = effect_value;
					new_bonus->HeadShot[SBIndex::FINISHING_EFFECT_DMG]         = limit_value;
				}
				break;
			}

			case SpellEffect::HeadShotLevel: {
				if (new_bonus->HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX] < effect_value) {
					new_bonus->HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX]          = effect_value;
					new_bonus->HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_CHANCE_BONUS] = limit_value;
				}
				break;
			}

			case SpellEffect::Assassinate:
			{
				if(new_bonus->Assassinate[SBIndex::FINISHING_EFFECT_DMG] < limit_value){
					new_bonus->Assassinate[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = effect_value;
					new_bonus->Assassinate[SBIndex::FINISHING_EFFECT_DMG]         = limit_value;
				}
				break;
			}

			case SpellEffect::AssassinateLevel:
			{
				if(new_bonus->AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX] < effect_value) {
					new_bonus->AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX]          = effect_value;
					new_bonus->AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_CHANCE_BONUS] = limit_value;
				}
				break;
			}

			case SpellEffect::FinishingBlow:
			{
				//base1 = chance, base2 = damage
				if (new_bonus->FinishingBlow[SBIndex::FINISHING_EFFECT_DMG] < limit_value){
					new_bonus->FinishingBlow[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = effect_value;
					new_bonus->FinishingBlow[SBIndex::FINISHING_EFFECT_DMG]         = limit_value;
				}
				break;
			}

			case SpellEffect::FinishingBlowLvl:
			{
				if (new_bonus->FinishingBlowLvl[SBIndex::FINISHING_EFFECT_LEVEL_MAX] < effect_value){
					new_bonus->FinishingBlowLvl[SBIndex::FINISHING_EFFECT_LEVEL_MAX]    = effect_value;
					new_bonus->FinishingBlowLvl[SBIndex::FINISHING_BLOW_LEVEL_HP_RATIO] = limit_value;
				}
				break;
			}

			case SpellEffect::PetMeleeMitigation:
				new_bonus->PetMeleeMitigation += effect_value;
				break;

			case SpellEffect::Sanctuary:
				new_bonus->Sanctuary = true;
				break;

			case SpellEffect::FactionModPct:
			{
				if((effect_value < 0) && (new_bonus->FactionModPct > effect_value))
					new_bonus->FactionModPct = effect_value;

				else if(new_bonus->FactionModPct < effect_value)
					new_bonus->FactionModPct = effect_value;
				break;
			}

			case SpellEffect::Illusion:
				new_bonus->Illusion = spell_id;
				break;

			case SpellEffect::IllusionPersistence:
				new_bonus->IllusionPersistence = effect_value;
				break;

			case SpellEffect::LimitToSkill: {
				// Bad data or unsupported new skill
				if (effect_value > EQ::skills::HIGHEST_SKILL) {
					break;
				}
				if (effect_value <= EQ::skills::HIGHEST_SKILL){
					new_bonus->LimitToSkill[effect_value] = true;
					new_bonus->LimitToSkill[EQ::skills::HIGHEST_SKILL + 2] = true; //Used as a general exists check
					}
				break;
			}

			case SpellEffect::SkillProcAttempt:{

				for(int e = 0; e < MAX_SKILL_PROCS; e++)
				{
					if (new_bonus->SkillProc[e] && new_bonus->SkillProc[e] == spell_id) {
						break; //Do not use the same spell id more than once.
					}
					else if(!new_bonus->SkillProc[e]){
						new_bonus->SkillProc[e] = spell_id;
						HasSkillProcs();//This returns it correctly as debug
						break;
					}
				}
				break;
			}

			case SpellEffect::SkillProcSuccess:{

				for(int e = 0; e < MAX_SKILL_PROCS; e++)
				{
					if(new_bonus->SkillProcSuccess[e] && new_bonus->SkillProcSuccess[e] == spell_id)
						break; //Do not use the same spell id more than once.

					else if(!new_bonus->SkillProcSuccess[e]){
						new_bonus->SkillProcSuccess[e] = spell_id;
						break;
					}
				}
				break;
			}

			case SpellEffect::SkillAttackProc: {
				for (int i = 0; i < MAX_CAST_ON_SKILL_USE; i += 3) {
					if (!new_bonus->SkillAttackProc[i + SBIndex::SKILLATK_PROC_SPELL_ID]) { // spell id
						new_bonus->SkillAttackProc[i + SBIndex::SKILLATK_PROC_SPELL_ID] = max_value; // spell to proc
						new_bonus->SkillAttackProc[i + SBIndex::SKILLATK_PROC_CHANCE] = effect_value; // Chance base 1000 = 100% proc rate
						new_bonus->SkillAttackProc[i + SBIndex::SKILLATK_PROC_SKILL] = limit_value; // Skill to Proc Offr

						if (limit_value <= EQ::skills::HIGHEST_SKILL) {
							new_bonus->HasSkillAttackProc[limit_value] = true; //check first before looking for any effects.
						}
						break;
					}
				}
				break;
			}

			case SpellEffect::PC_Pet_Rampage: {
				new_bonus->PC_Pet_Rampage[SBIndex::PET_RAMPAGE_CHANCE] += effect_value; //Chance to rampage
				if (new_bonus->PC_Pet_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] < limit_value)
					new_bonus->PC_Pet_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] = limit_value; //Damage modifer - take highest
				break;
			}

			case SpellEffect::PC_Pet_AE_Rampage: {
				new_bonus->PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_CHANCE] += effect_value; //Chance to rampage
				if (new_bonus->PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] < limit_value)
					new_bonus->PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] = limit_value; //Damage modifer - take highest
				break;
			}

			case SpellEffect::PC_Pet_Flurry_Chance:
				new_bonus->PC_Pet_Flurry += effect_value; //Chance to Flurry
				break;

			case SpellEffect::ShroudofStealth:
				new_bonus->ShroudofStealth = true;
				break;

			case SpellEffect::ReduceFallDamage:
				new_bonus->ReduceFallDamage += effect_value;
				break;

			case SpellEffect::ReduceTradeskillFail:{

				if (limit_value > EQ::skills::HIGHEST_SKILL)
					break;

				new_bonus->ReduceTradeskillFail[limit_value] += effect_value;
				break;
			}

			case SpellEffect::TradeSkillMastery:
				if (new_bonus->TradeSkillMastery < effect_value)
					new_bonus->TradeSkillMastery = effect_value;
				break;

			case SpellEffect::RaiseSkillCap: {
				if (limit_value > EQ::skills::HIGHEST_SKILL)
					break;

				if (new_bonus->RaiseSkillCap[limit_value] < effect_value)
					new_bonus->RaiseSkillCap[limit_value] = effect_value;
				break;
			}

			case SpellEffect::NoBreakAESneak:
				if (new_bonus->NoBreakAESneak < effect_value)
					new_bonus->NoBreakAESneak = effect_value;
				break;

			case SpellEffect::FeignedCastOnChance:
				if (new_bonus->FeignedCastOnChance < effect_value)
					new_bonus->FeignedCastOnChance = effect_value;
				break;

			case SpellEffect::AdditionalAura:
				if (new_bonus->aura_slots < effect_value)
					new_bonus->aura_slots = effect_value;
				break;

			case SpellEffect::IncreaseTrapCount:
				if (new_bonus->trap_slots < effect_value)
					new_bonus->trap_slots = effect_value;
				break;

			case SpellEffect::Attack_Accuracy_Max_Percent:
				new_bonus->Attack_Accuracy_Max_Percent += effect_value;
				break;


			case SpellEffect::AC_Mitigation_Max_Percent:
				new_bonus->AC_Mitigation_Max_Percent += effect_value;
				break;

			case SpellEffect::AC_Avoidance_Max_Percent:
				new_bonus->AC_Avoidance_Max_Percent += effect_value;
				break;

			case SpellEffect::Damage_Taken_Position_Mod:
			{
				//Mitigate if damage taken from behind base2 = 0, from front base2 = 1
				if (limit_value < 0 || limit_value >= 2) {
					break;
				}
				if (AdditiveWornBonus)
					new_bonus->Damage_Taken_Position_Mod[limit_value] += effect_value;
				else if (effect_value < 0 && new_bonus->Damage_Taken_Position_Mod[limit_value] > effect_value)
					new_bonus->Damage_Taken_Position_Mod[limit_value] = effect_value;
				else if (effect_value > 0 && new_bonus->Damage_Taken_Position_Mod[limit_value] < effect_value)
					new_bonus->Damage_Taken_Position_Mod[limit_value] = effect_value;
				break;
			}

			case SpellEffect::Melee_Damage_Position_Mod:
			{
				//Increase damage by percent from behind base2 = 0, from front base2 = 1
				if (limit_value < 0 || limit_value >= 2) {
					break;
				}
				if (AdditiveWornBonus)
					new_bonus->Melee_Damage_Position_Mod[limit_value] += effect_value;
				else if (effect_value < 0 && new_bonus->Melee_Damage_Position_Mod[limit_value] > effect_value)
					new_bonus->Melee_Damage_Position_Mod[limit_value] = effect_value;
				else if (effect_value > 0 && new_bonus->Melee_Damage_Position_Mod[limit_value] < effect_value)
					new_bonus->Melee_Damage_Position_Mod[limit_value] = effect_value;
				break;
			}

			case SpellEffect::Damage_Taken_Position_Amt:
			{
				//Mitigate if damage taken from behind base2 = 0, from front base2 = 1
				if (limit_value < 0 || limit_value >= 2) {
					break;
				}

				new_bonus->Damage_Taken_Position_Amt[limit_value] += effect_value;
				break;
			}

			case SpellEffect::Melee_Damage_Position_Amt:
			{
				//Mitigate if damage taken from behind base2 = 0, from front base2 = 1
				if (limit_value < 0 || limit_value >= 2) {
					break;
				}

				new_bonus->Melee_Damage_Position_Amt[limit_value] += effect_value;
				break;
			}

			case SpellEffect::DS_Mitigation_Amount:
				new_bonus->DS_Mitigation_Amount += effect_value;
				break;

			case SpellEffect::DS_Mitigation_Percentage:
				new_bonus->DS_Mitigation_Percentage += effect_value;
				break;

			case SpellEffect::Pet_Crit_Melee_Damage_Pct_Owner:
				new_bonus->Pet_Crit_Melee_Damage_Pct_Owner += effect_value;
				break;

			case SpellEffect::Pet_Add_Atk:
				new_bonus->Pet_Add_Atk += effect_value;
				break;

			case SpellEffect::ExtendedShielding:
			{
				if (AdditiveWornBonus) {
					new_bonus->ExtendedShielding += effect_value;
				}
				else if (effect_value < 0 && new_bonus->ExtendedShielding > effect_value){
					new_bonus->ExtendedShielding = effect_value;
				}
				else if (effect_value > 0 && new_bonus->ExtendedShielding < effect_value){
					new_bonus->ExtendedShielding = effect_value;
				}
				break;
			}

			case SpellEffect::ShieldDuration:
			{
				if (AdditiveWornBonus) {
					new_bonus->ShieldDuration += effect_value;
				}
				else if (effect_value < 0 && new_bonus->ShieldDuration > effect_value){
					new_bonus->ShieldDuration = effect_value;
				}
				else if (effect_value > 0 && new_bonus->ShieldDuration < effect_value){
					new_bonus->ShieldDuration = effect_value;
				}
				break;
			}

			case SpellEffect::Worn_Endurance_Regen_Cap:
				new_bonus->ItemEnduranceRegenCap += effect_value;
				break;

			case SpellEffect::ItemManaRegenCapIncrease:
				new_bonus->ItemManaRegenCap += effect_value;
				break;

			case SpellEffect::Weapon_Stance: {
				if (IsValidSpell(effect_value)) { //base1 is the spell_id of buff
					if (limit_value <= WEAPON_STANCE_TYPE_MAX) { //0=2H, 1=Shield, 2=DW
						if (IsValidSpell(new_bonus->WeaponStance[limit_value])) { //Check if we already a spell_id saved for this effect
							if (spells[new_bonus->WeaponStance[limit_value]].rank < spells[effect_value].rank) { //If so, check if any new spellids with higher rank exist (live spells for this are ranked).
								new_bonus->WeaponStance[limit_value] = effect_value; //Overwrite with new effect
								SetWeaponStanceEnabled(true);

								if (WornType) {
									weaponstance.itembonus_enabled = true;
								}
								else {
									weaponstance.spellbonus_enabled = true;
								}
							}
						}
						else {
							new_bonus->WeaponStance[limit_value] = effect_value; //If no prior effect exists, then apply
							SetWeaponStanceEnabled(true);

							if (WornType) {
								weaponstance.itembonus_enabled = true;
							}
							else {
								weaponstance.spellbonus_enabled = true;
							}
						}
					}
				}
				break;
			}

			case SpellEffect::Invisibility:
			case SpellEffect::Invisibility2:
				effect_value = std::min({ effect_value, MAX_INVISIBILTY_LEVEL });
				if (new_bonus->invisibility < effect_value)
					new_bonus->invisibility = effect_value;
				break;

			case SpellEffect::InvisVsUndead:
			case SpellEffect::InvisVsUndead2:
				if (new_bonus->invisibility_verse_undead < effect_value)
					new_bonus->invisibility_verse_undead = effect_value;
				break;

			case SpellEffect::InvisVsAnimals:
			case SpellEffect::ImprovedInvisAnimals:
				effect_value = std::min({ effect_value, MAX_INVISIBILTY_LEVEL });
				if (new_bonus->invisibility_verse_animal < effect_value)
					new_bonus->invisibility_verse_animal = effect_value;
				break;

			case SpellEffect::SeeInvis:
				effect_value = std::min({ effect_value, MAX_INVISIBILTY_LEVEL });
				if (new_bonus->SeeInvis < effect_value) {
					new_bonus->SeeInvis = effect_value;
				}
				break;

			case SpellEffect::ZoneSuspendMinion:
				new_bonus->ZoneSuspendMinion = effect_value;
				break;

			case SpellEffect::CompleteHeal:
				new_bonus->CompleteHealBuffBlocker = true;
				break;

			case SpellEffect::TrapCircumvention:
				new_bonus->TrapCircumvention += effect_value;
				break;
		}
	}
}

void Client::CalcItemScale() {
	bool changed = false;

	// MainAmmo excluded in helper function below
	if (CalcItemScale(EQ::invslot::EQUIPMENT_BEGIN, EQ::invslot::EQUIPMENT_END)) // original coding excluded MainAmmo (< 21)
		changed = true;

	if (CalcItemScale(EQ::invslot::GENERAL_BEGIN, EQ::invslot::GENERAL_END)) // original coding excluded MainCursor (< 30)
		changed = true;

	// I excluded cursor bag slots here because cursor was excluded above..if this is incorrect, change 'slot_y' here to CURSOR_BAG_END
	// and 'slot_y' above to CURSOR from GENERAL_END above - or however it is supposed to be...
	if (CalcItemScale(EQ::invbag::GENERAL_BAGS_BEGIN, EQ::invbag::GENERAL_BAGS_END)) // (< 341)
		changed = true;

	if (CalcItemScale(EQ::invslot::TRIBUTE_BEGIN, EQ::invslot::TRIBUTE_END)) // (< 405)
		changed = true;

	if(changed)
	{
		CalcBonuses();
	}
}

bool Client::CalcItemScale(uint32 slot_x, uint32 slot_y) {
	// behavior change: 'slot_y' is now [RANGE]_END and not [RANGE]_END + 1
	bool changed = false;
	uint32 i;
	for (i = slot_x; i <= slot_y; i++) {
		if (i == EQ::invslot::slotAmmo) // moved here from calling procedure to facilitate future range changes where MainAmmo may not be the last slot
			continue;

		EQ::ItemInstance* inst = m_inv.GetItem(i);

		if(inst == nullptr)
			continue;

		// TEST CODE: test for bazaar trader crashing with charm items
		if (IsTrader())
			if (i >= EQ::invbag::GENERAL_BAGS_BEGIN && i <= EQ::invbag::GENERAL_BAGS_END) {
				EQ::ItemInstance* parent_item = m_inv.GetItem(EQ::InventoryProfile::CalcSlotId(i));
				if (parent_item && parent_item->GetItem()->BagType == EQ::item::BagTypeTradersSatchel)
					continue;
			}

		bool update_slot = false;
		if(inst->IsScaling())
		{
			uint16 oldexp = inst->GetExp();

			if (parse->ItemHasQuestSub(inst, EVENT_SCALE_CALC)) {
				parse->EventItem(EVENT_SCALE_CALC, this, inst, nullptr, "", 0);
			}

			if (inst->GetExp() != oldexp) {	// if the scaling factor changed, rescale the item and update the client
				inst->ScaleItem();
				changed = true;
				update_slot = true;
			}
		}

		//iterate all augments
		for (int x = EQ::invaug::SOCKET_BEGIN; x <= EQ::invaug::SOCKET_END; ++x)
		{
			EQ::ItemInstance * a_inst = inst->GetAugment(x);
			if(!a_inst)
				continue;

			if(a_inst->IsScaling())
			{
				uint16 oldexp = a_inst->GetExp();

				if (parse->ItemHasQuestSub(a_inst, EVENT_SCALE_CALC)) {
					parse->EventItem(EVENT_SCALE_CALC, this, a_inst, nullptr, "", 0);
				}

				if (a_inst->GetExp() != oldexp)
				{
					a_inst->ScaleItem();
					changed = true;
					update_slot = true;
				}
			}
		}

		if(update_slot)
		{
			SendItemPacket(i, inst, ItemPacketCharmUpdate);
		}
	}
	return changed;
}

void Client::DoItemEnterZone() {
	bool changed = false;

	// MainAmmo excluded in helper function below
	if (DoItemEnterZone(EQ::invslot::EQUIPMENT_BEGIN, EQ::invslot::EQUIPMENT_END)) // original coding excluded MainAmmo (< 21)
		changed = true;

	if (DoItemEnterZone(EQ::invslot::GENERAL_BEGIN, EQ::invslot::GENERAL_END)) // original coding excluded MainCursor (< 30)
		changed = true;

	// I excluded cursor bag slots here because cursor was excluded above..if this is incorrect, change 'slot_y' here to CURSOR_BAG_END
	// and 'slot_y' above to CURSOR from GENERAL_END above - or however it is supposed to be...
	if (DoItemEnterZone(EQ::invbag::GENERAL_BAGS_BEGIN, EQ::invbag::GENERAL_BAGS_END)) // (< 341)
		changed = true;

	if (DoItemEnterZone(EQ::invslot::TRIBUTE_BEGIN, EQ::invslot::TRIBUTE_END)) // (< 405)
		changed = true;

	if(changed)
	{
		CalcBonuses();
	}
}

bool Client::DoItemEnterZone(uint32 slot_x, uint32 slot_y) {
	// behavior change: 'slot_y' is now [RANGE]_END and not [RANGE]_END + 1
	bool changed = false;
	for(uint32 i = slot_x; i <= slot_y; i++) {
		if (i == EQ::invslot::slotAmmo) // moved here from calling procedure to facilitate future range changes where MainAmmo may not be the last slot
			continue;

		EQ::ItemInstance* inst = m_inv.GetItem(i);

		if(!inst)
			continue;

		// TEST CODE: test for bazaar trader crashing with charm items
		if (IsTrader())
			if (i >= EQ::invbag::GENERAL_BAGS_BEGIN && i <= EQ::invbag::GENERAL_BAGS_END) {
				EQ::ItemInstance* parent_item = m_inv.GetItem(EQ::InventoryProfile::CalcSlotId(i));
				if (parent_item && parent_item->GetItem()->BagType == EQ::item::BagTypeTradersSatchel)
					continue;
			}

		bool update_slot = false;
		if(inst->IsScaling())
		{
			uint16 oldexp = inst->GetExp();

			if (parse->ItemHasQuestSub(inst, EVENT_ITEM_ENTER_ZONE)) {
				parse->EventItem(EVENT_ITEM_ENTER_ZONE, this, inst, nullptr, "", 0);
			}

			if (i <= EQ::invslot::EQUIPMENT_END) {
				if (parse->ItemHasQuestSub(inst, EVENT_EQUIP_ITEM)) {
					parse->EventItem(EVENT_EQUIP_ITEM, this, inst, nullptr, "", i);
				}
			}

			if (inst->GetExp() != oldexp) {	// if the scaling factor changed, rescale the item and update the client
				inst->ScaleItem();
				changed = true;
				update_slot = true;
			}
		} else {
			if (i <= EQ::invslot::EQUIPMENT_END) {
				if (parse->ItemHasQuestSub(inst, EVENT_EQUIP_ITEM)) {
					parse->EventItem(EVENT_EQUIP_ITEM, this, inst, nullptr, "", i);
				}
			}

			if (parse->ItemHasQuestSub(inst, EVENT_ITEM_ENTER_ZONE)) {
				parse->EventItem(EVENT_ITEM_ENTER_ZONE, this, inst, nullptr, "", 0);
			}
		}

		//iterate all augments
		for (int x = EQ::invaug::SOCKET_BEGIN; x <= EQ::invaug::SOCKET_END; ++x)
		{
			EQ::ItemInstance *a_inst = inst->GetAugment(x);
			if(!a_inst)
				continue;

			if(a_inst->IsScaling())
			{
				uint16 oldexp = a_inst->GetExp();

				if (parse->ItemHasQuestSub(a_inst, EVENT_ITEM_ENTER_ZONE)) {
					parse->EventItem(EVENT_ITEM_ENTER_ZONE, this, a_inst, nullptr, "", 0);
				}

				if (a_inst->GetExp() != oldexp)
				{
					a_inst->ScaleItem();
					changed = true;
					update_slot = true;
				}
			} else {
				if (parse->ItemHasQuestSub(a_inst, EVENT_ITEM_ENTER_ZONE)) {
					parse->EventItem(EVENT_ITEM_ENTER_ZONE, this, a_inst, nullptr, "", 0);
				}
			}
		}

		if(update_slot)
		{
			SendItemPacket(i, inst, ItemPacketCharmUpdate);
		}
	}
	return changed;
}

uint8 Mob::IsFocusEffect(uint16 spell_id,int effect_index, bool AA,uint32 aa_effect)
{
	uint16 effect = 0;

	if (!AA)
		effect = spells[spell_id].effect_id[effect_index];
	else
		effect = aa_effect;

	switch (effect)
	{
		case SpellEffect::ImprovedDamage:
			return focusImprovedDamage;
		case SpellEffect::ImprovedDamage2:
			return focusImprovedDamage2;
		case SpellEffect::ImprovedHeal:
			return focusImprovedHeal;
		case SpellEffect::ReduceManaCost:
			return focusManaCost;
		case SpellEffect::IncreaseSpellHaste:
			return focusSpellHaste;
		case SpellEffect::IncreaseSpellDuration:
			return focusSpellDuration;
		case SpellEffect::SpellDurationIncByTic:
			return focusSpellDurByTic;
		case SpellEffect::SwarmPetDuration:
			return focusSwarmPetDuration;
		case SpellEffect::IncreaseRange:
			return focusRange;
		case SpellEffect::ReduceReagentCost:
			return focusReagentCost;
		case SpellEffect::PetPowerIncrease:
			return focusPetPower;
		case SpellEffect::SpellResistReduction:
			return focusResistRate;
		case SpellEffect::Fc_ResistIncoming:
			return focusFcResistIncoming;
		case SpellEffect::Fc_Amplify_Mod:
			return focusFcAmplifyMod;
		case SpellEffect::Fc_Amplify_Amt:
			return focusFcAmplifyAmt;
		case SpellEffect::SpellHateMod:
			return focusSpellHateMod;
		case SpellEffect::ReduceReuseTimer:
			return focusReduceRecastTime;
		case SpellEffect::TriggerOnCast:
			return focusTriggerOnCast;
		case SpellEffect::FcSpellVulnerability:
			return focusSpellVulnerability;
		case SpellEffect::Fc_Spell_Damage_Pct_IncomingPC:
			return focusFcSpellDamagePctIncomingPC;
		case SpellEffect::BlockNextSpellFocus:
			return focusBlockNextSpell;
		case SpellEffect::FcTwincast:
			return focusTwincast;
		case SpellEffect::SympatheticProc:
			return focusSympatheticProc;
		case SpellEffect::FcDamageAmt:
			return focusFcDamageAmt;
		case SpellEffect::FcDamageAmt2:
			return focusFcDamageAmt2;
		case SpellEffect::FcDamageAmtCrit:
			return focusFcDamageAmtCrit;
		case SpellEffect::FcDamagePctCrit:
			return focusFcDamagePctCrit;
		case SpellEffect::FcDamageAmtIncoming:
			return focusFcDamageAmtIncoming;
		case SpellEffect::Fc_Spell_Damage_Amt_IncomingPC:
			return focusFcSpellDamageAmtIncomingPC;
		case SpellEffect::FcHealAmtIncoming:
			return focusFcHealAmtIncoming;
		case SpellEffect::FcHealPctIncoming:
			return focusFcHealPctIncoming;
		case SpellEffect::FcBaseEffects:
			return focusFcBaseEffects;
		case SpellEffect::FcIncreaseNumHits:
			return focusIncreaseNumHits;
		case SpellEffect::FcLimitUse:
			return focusFcLimitUse;
		case SpellEffect::FcMute:
			return focusFcMute;
		case SpellEffect::FcTimerRefresh:
			return focusFcTimerRefresh;
		case SpellEffect::FcTimerLockout:
			return focusFcTimerLockout;
		case SpellEffect::Fc_Cast_Spell_On_Land:
			return focusFcCastSpellOnLand;
		case SpellEffect::FcStunTimeMod:
			return focusFcStunTimeMod;
		case SpellEffect::Fc_CastTimeMod2:
			return focusFcCastTimeMod2;
		case SpellEffect::Fc_CastTimeAmt:
			return focusFcCastTimeAmt;
		case SpellEffect::FcHealPctCritIncoming:
			return focusFcHealPctCritIncoming;
		case SpellEffect::FcHealAmt:
			return focusFcHealAmt;
		case SpellEffect::FcHealAmtCrit:
			return focusFcHealAmtCrit;

	}
	return 0;
}

void Mob::NegateSpellEffectBonuses(uint16 spell_id)
{
	if (!IsValidSpell(spell_id))
		return;

	int effect_value = 0;

	for (int i = 0; i < EFFECT_COUNT; i++)
	{
		bool negate_spellbonus = false;
		bool negate_aabonus = false;
		bool negate_itembonus = false;

		if (spells[spell_id].effect_id[i] == SpellEffect::NegateSpellEffect) {

			//Set negate types
			switch (spells[spell_id].base_value[i])
			{
				case NEGATE_SPA_ALL_BONUSES:
					negate_spellbonus = true;
					negate_aabonus = true;
					negate_itembonus = true;
					break;

				case NEGATE_SPA_SPELLBONUS:
					negate_spellbonus = true;
					break;

				case NEGATE_SPA_ITEMBONUS:
					negate_itembonus = true;
					break;

				case NEGATE_SPA_AABONUS:
					negate_aabonus = true;
					break;

				case NEGATE_SPA_ITEMBONUS_AND_AABONUS:
					negate_aabonus = true;
					negate_itembonus = true;
					break;

				case NEGATE_SPA_SPELLBONUS_AND_AABONUS:
					negate_aabonus = true;
					negate_spellbonus = true;
					break;

				case NEGATE_SPA_SPELLBONUS_AND_ITEMBONUS:
					negate_spellbonus = true;
					negate_itembonus = true;
					break;
			}

			//Negate focus effects
			for (int e = 0; e < HIGHEST_FOCUS + 1; e++)
			{
				if (spellbonuses.FocusEffects[e] == spells[spell_id].limit_value[i])
				{
					if (negate_spellbonus) { spellbonuses.FocusEffects[e] = effect_value; }
					continue;
				}
			}

			//Negate bonuses
			switch (spells[spell_id].limit_value[i])
			{
				case SpellEffect::CurrentHP:
					if (negate_spellbonus) { spellbonuses.HPRegen = effect_value; }
					if (negate_aabonus) { aabonuses.HPRegen = effect_value; }
					if (negate_itembonus) { itembonuses.HPRegen = effect_value; }
					break;

				case SpellEffect::CurrentEndurance:
					if (negate_spellbonus) { spellbonuses.EnduranceRegen = effect_value; }
					if (negate_aabonus) { aabonuses.EnduranceRegen = effect_value; }
					if (negate_itembonus) { itembonuses.EnduranceRegen = effect_value; }
					break;

				case SpellEffect::ChangeFrenzyRad:
					if (negate_spellbonus) { spellbonuses.AggroRange = static_cast<float>(effect_value); }
					if (negate_aabonus) { aabonuses.AggroRange = static_cast<float>(effect_value); }
					if (negate_itembonus) { itembonuses.AggroRange = static_cast<float>(effect_value); }
					break;

				case SpellEffect::Harmony:
					if (negate_spellbonus) { spellbonuses.AssistRange = static_cast<float>(effect_value); }
					if (negate_aabonus) { aabonuses.AssistRange = static_cast<float>(effect_value); }
					if (negate_itembonus) { itembonuses.AssistRange = static_cast<float>(effect_value); }
					break;

				case SpellEffect::AttackSpeed:
					if (negate_spellbonus) { spellbonuses.haste = effect_value; }
					if (negate_aabonus) { aabonuses.haste = effect_value; }
					if (negate_itembonus) { itembonuses.haste = effect_value; }
					break;

				case SpellEffect::AttackSpeed2:
					if (negate_spellbonus) { spellbonuses.hastetype2 = effect_value; }
					if (negate_aabonus) { aabonuses.hastetype2 = effect_value; }
					if (negate_itembonus) { itembonuses.hastetype2 = effect_value; }
					break;

				case SpellEffect::AttackSpeed3:
				{
					if (negate_spellbonus) { spellbonuses.hastetype3 = effect_value; }
					if (negate_aabonus) { aabonuses.hastetype3 = effect_value; }
					if (negate_itembonus) { itembonuses.hastetype3 = effect_value; }
					break;
				}

				case SpellEffect::AttackSpeed4:
					if (negate_spellbonus) { spellbonuses.inhibitmelee = effect_value; }
					if (negate_aabonus) { aabonuses.inhibitmelee = effect_value; }
					if (negate_itembonus) { itembonuses.inhibitmelee = effect_value; }
					break;

				case SpellEffect::IncreaseArchery:
					if (negate_spellbonus) { spellbonuses.increase_archery = effect_value; }
					if (negate_aabonus) { aabonuses.increase_archery = effect_value; }
					if (negate_itembonus) { itembonuses.increase_archery = effect_value; }
					break;

				case SpellEffect::TotalHP:
					if (negate_spellbonus) { spellbonuses.FlatMaxHPChange = effect_value; }
					if (negate_aabonus) { aabonuses.FlatMaxHPChange = effect_value; }
					if (negate_itembonus) { itembonuses.FlatMaxHPChange = effect_value; }
					break;

				case SpellEffect::ManaRegen_v2:
				case SpellEffect::CurrentMana:
					if (negate_spellbonus) { spellbonuses.ManaRegen = effect_value; }
					if (negate_aabonus) { aabonuses.ManaRegen = effect_value; }
					if (negate_itembonus) { itembonuses.ManaRegen = effect_value; }
					break;

				case SpellEffect::ManaPool:
					if (negate_spellbonus) { spellbonuses.Mana = effect_value; }
					if (negate_itembonus) { itembonuses.Mana = effect_value; }
					if (negate_aabonus) { aabonuses.Mana = effect_value; }
					break;

				case SpellEffect::Stamina:
					if (negate_spellbonus) { spellbonuses.EnduranceReduction = effect_value; }
					if (negate_itembonus) { itembonuses.EnduranceReduction = effect_value; }
					if (negate_aabonus) { aabonuses.EnduranceReduction = effect_value; }
					break;

				case SpellEffect::ACv2:
				case SpellEffect::ArmorClass:
					if (negate_spellbonus) { spellbonuses.AC = effect_value; }
					if (negate_aabonus) { aabonuses.AC = effect_value; }
					if (negate_itembonus) { itembonuses.AC = effect_value; }
					break;

				case SpellEffect::ATK:
					if (negate_spellbonus) { spellbonuses.ATK = effect_value; }
					if (negate_aabonus) { aabonuses.ATK = effect_value; }
					if (negate_itembonus) { itembonuses.ATK = effect_value; }
					break;

				case SpellEffect::STR:
					if (negate_spellbonus) { spellbonuses.STR = effect_value; }
					if (negate_itembonus) { itembonuses.STR = effect_value; }
					if (negate_aabonus) { aabonuses.STR = effect_value; }
					break;

				case SpellEffect::DEX:
					if (negate_spellbonus) { spellbonuses.DEX = effect_value; }
					if (negate_aabonus) { aabonuses.DEX = effect_value; }
					if (negate_itembonus) { itembonuses.DEX = effect_value; }
					break;

				case SpellEffect::AGI:
					if (negate_itembonus) { itembonuses.AGI = effect_value; }
					if (negate_aabonus) { aabonuses.AGI = effect_value; }
					if (negate_spellbonus) { spellbonuses.AGI = effect_value; }
					break;

				case SpellEffect::STA:
					if (negate_spellbonus) { spellbonuses.STA = effect_value; }
					if (negate_itembonus) { itembonuses.STA = effect_value; }
					if (negate_aabonus) { aabonuses.STA = effect_value; }
					break;

				case SpellEffect::INT:
					if (negate_spellbonus) { spellbonuses.INT = effect_value; }
					if (negate_aabonus) { aabonuses.INT = effect_value; }
					if (negate_itembonus) { itembonuses.INT = effect_value; }
					break;

				case SpellEffect::WIS:
					if (negate_spellbonus) { spellbonuses.WIS = effect_value; }
					if (negate_aabonus) { aabonuses.WIS = effect_value; }
					if (negate_itembonus) { itembonuses.WIS = effect_value; }
					break;

				case SpellEffect::CHA:
					if (negate_itembonus) { itembonuses.CHA = effect_value; }
					if (negate_spellbonus) { spellbonuses.CHA = effect_value; }
					if (negate_aabonus) { aabonuses.CHA = effect_value; }
					break;

				case SpellEffect::AllStats:
				{
					if (negate_spellbonus) {
						spellbonuses.STR = effect_value;
						spellbonuses.DEX = effect_value;
						spellbonuses.AGI = effect_value;
						spellbonuses.STA = effect_value;
						spellbonuses.INT = effect_value;
						spellbonuses.WIS = effect_value;
						spellbonuses.CHA = effect_value;
					}

					if (negate_itembonus) {
						itembonuses.STR = effect_value;
						itembonuses.DEX = effect_value;
						itembonuses.AGI = effect_value;
						itembonuses.STA = effect_value;
						itembonuses.INT = effect_value;
						itembonuses.WIS = effect_value;
						itembonuses.CHA = effect_value;
					}

					if (negate_aabonus) {
						aabonuses.STR = effect_value;
						aabonuses.DEX = effect_value;
						aabonuses.AGI = effect_value;
						aabonuses.STA = effect_value;
						aabonuses.INT = effect_value;
						aabonuses.WIS = effect_value;
						aabonuses.CHA = effect_value;
					}

					break;
				}

				case SpellEffect::ResistFire:
					if (negate_spellbonus) { spellbonuses.FR = effect_value; }
					if (negate_itembonus) { itembonuses.FR = effect_value; }
					if (negate_aabonus) { aabonuses.FR = effect_value; }
					break;

				case SpellEffect::ResistCold:
					if (negate_spellbonus) { spellbonuses.CR = effect_value; }
					if (negate_aabonus) { aabonuses.CR = effect_value; }
					if (negate_itembonus) { itembonuses.CR = effect_value; }
					break;

				case SpellEffect::ResistPoison:
					if (negate_spellbonus) { spellbonuses.PR = effect_value; }
					if (negate_aabonus) { aabonuses.PR = effect_value; }
					if (negate_itembonus) { itembonuses.PR = effect_value; }
					break;

				case SpellEffect::ResistDisease:
					if (negate_spellbonus) { spellbonuses.DR = effect_value; }
					if (negate_itembonus) { itembonuses.DR = effect_value; }
					if (negate_aabonus) { aabonuses.DR = effect_value; }
					break;

				case SpellEffect::ResistMagic:
					if (negate_spellbonus) { spellbonuses.MR = effect_value; }
					if (negate_aabonus) { aabonuses.MR = effect_value; }
					if (negate_itembonus) { itembonuses.MR = effect_value; }
					break;

				case SpellEffect::ResistAll:
				{
					if (negate_spellbonus) {
						spellbonuses.MR = effect_value;
						spellbonuses.DR = effect_value;
						spellbonuses.PR = effect_value;
						spellbonuses.CR = effect_value;
						spellbonuses.FR = effect_value;
					}

					if (negate_aabonus) {
						aabonuses.MR = effect_value;
						aabonuses.DR = effect_value;
						aabonuses.PR = effect_value;
						aabonuses.CR = effect_value;
						aabonuses.FR = effect_value;
					}

					if (negate_itembonus) {
						itembonuses.MR = effect_value;
						itembonuses.DR = effect_value;
						itembonuses.PR = effect_value;
						itembonuses.CR = effect_value;
						itembonuses.FR = effect_value;
					}

					break;
				}

				case SpellEffect::ResistCorruption:
					if (negate_spellbonus) { spellbonuses.Corrup = effect_value; }
					if (negate_itembonus) { itembonuses.Corrup = effect_value; }
					if (negate_aabonus) { aabonuses.Corrup = effect_value; }
					break;

				case SpellEffect::CastingLevel:	// Brilliance of Ro
					if (negate_spellbonus) { spellbonuses.adjusted_casting_skill = effect_value; }
					if (negate_aabonus) { aabonuses.adjusted_casting_skill = effect_value; }
					if (negate_itembonus) { itembonuses.adjusted_casting_skill = effect_value; }
					break;

				case SpellEffect::CastingLevel2:
					if (negate_spellbonus) { spellbonuses.effective_casting_level = effect_value; }
					if (negate_aabonus) { aabonuses.effective_casting_level = effect_value; }
					if (negate_itembonus) { itembonuses.effective_casting_level = effect_value; }
					break;


				case SpellEffect::MovementSpeed:
					if (negate_spellbonus) { spellbonuses.movementspeed = effect_value; }
					if (negate_aabonus) { aabonuses.movementspeed = effect_value; }
					if (negate_itembonus) { itembonuses.movementspeed = effect_value; }
					break;

				case SpellEffect::SpellDamageShield:
					if (negate_spellbonus) { spellbonuses.SpellDamageShield = effect_value; }
					if (negate_aabonus) { aabonuses.SpellDamageShield = effect_value; }
					if (negate_itembonus) { itembonuses.SpellDamageShield = effect_value; }
					break;

				case SpellEffect::DamageShield:
					if (negate_spellbonus) { spellbonuses.DamageShield = effect_value; }
					if (negate_aabonus) { aabonuses.DamageShield = effect_value; }
					if (negate_itembonus) { itembonuses.DamageShield = effect_value; }
					break;

				case SpellEffect::ReverseDS:
					if (negate_spellbonus) { spellbonuses.ReverseDamageShield = effect_value; }
					if (negate_aabonus) { aabonuses.ReverseDamageShield = effect_value; }
					if (negate_itembonus) { itembonuses.ReverseDamageShield = effect_value; }
					break;

				case SpellEffect::Reflect:
					if (negate_spellbonus) { spellbonuses.reflect[SBIndex::REFLECT_CHANCE] = effect_value; }
					if (negate_aabonus) { aabonuses.reflect[SBIndex::REFLECT_CHANCE] = effect_value; }
					if (negate_itembonus) { itembonuses.reflect[SBIndex::REFLECT_CHANCE] = effect_value; }
					break;

				case SpellEffect::Amplification:
					if (negate_spellbonus) { spellbonuses.Amplification = effect_value; }
					if (negate_itembonus) { itembonuses.Amplification = effect_value; }
					if (negate_aabonus) { aabonuses.Amplification = effect_value; }
					break;

				case SpellEffect::ChangeAggro:
					if (negate_spellbonus) { spellbonuses.hatemod = effect_value; }
					if (negate_itembonus) { itembonuses.hatemod = effect_value; }
					if (negate_aabonus) { aabonuses.hatemod = effect_value; }
					break;

				case SpellEffect::MeleeMitigation:
					if (negate_spellbonus) { spellbonuses.MeleeMitigationEffect = effect_value; }
					if (negate_itembonus) { itembonuses.MeleeMitigationEffect = effect_value; }
					if (negate_aabonus) { aabonuses.MeleeMitigationEffect = effect_value; }
					break;

				case SpellEffect::CriticalHitChance:
				{
					for (int e = 0; e < EQ::skills::HIGHEST_SKILL + 1; e++)
					{
						if (negate_spellbonus) { spellbonuses.CriticalHitChance[e] = effect_value; }
						if (negate_aabonus) { aabonuses.CriticalHitChance[e] = effect_value; }
						if (negate_itembonus) { itembonuses.CriticalHitChance[e] = effect_value; }
					}
				}

				case SpellEffect::CrippBlowChance:
					if (negate_spellbonus) { spellbonuses.CrippBlowChance = effect_value; }
					if (negate_aabonus) { aabonuses.CrippBlowChance = effect_value; }
					if (negate_itembonus) { itembonuses.CrippBlowChance = effect_value; }
					break;

				case SpellEffect::AvoidMeleeChance:
					if (negate_spellbonus) { spellbonuses.AvoidMeleeChanceEffect = effect_value; }
					if (negate_aabonus) { aabonuses.AvoidMeleeChanceEffect = effect_value; }
					if (negate_itembonus) { itembonuses.AvoidMeleeChanceEffect = effect_value; }
					break;

				case SpellEffect::RiposteChance:
					if (negate_spellbonus) { spellbonuses.RiposteChance = effect_value; }
					if (negate_aabonus) { aabonuses.RiposteChance = effect_value; }
					if (negate_itembonus) { itembonuses.RiposteChance = effect_value; }
					break;

				case SpellEffect::DodgeChance:
					if (negate_spellbonus) { spellbonuses.DodgeChance = effect_value; }
					if (negate_aabonus) { aabonuses.DodgeChance = effect_value; }
					if (negate_itembonus) { itembonuses.DodgeChance = effect_value; }
					break;

				case SpellEffect::ParryChance:
					if (negate_spellbonus) { spellbonuses.ParryChance = effect_value; }
					if (negate_aabonus) { aabonuses.ParryChance = effect_value; }
					if (negate_itembonus) { itembonuses.ParryChance = effect_value; }
					break;

				case SpellEffect::DualWieldChance:
					if (negate_spellbonus) { spellbonuses.DualWieldChance = effect_value; }
					if (negate_aabonus) { aabonuses.DualWieldChance = effect_value; }
					if (negate_itembonus) { itembonuses.DualWieldChance = effect_value; }
					break;

				case SpellEffect::DoubleAttackChance:
					if (negate_spellbonus) { spellbonuses.DoubleAttackChance = effect_value; }
					if (negate_aabonus) { aabonuses.DoubleAttackChance = effect_value; }
					if (negate_itembonus) { itembonuses.DoubleAttackChance = effect_value; }
					break;

				case SpellEffect::TripleAttackChance:
					if (negate_spellbonus) { spellbonuses.TripleAttackChance = effect_value; }
					if (negate_aabonus) { aabonuses.TripleAttackChance = effect_value; }
					if (negate_itembonus) { itembonuses.TripleAttackChance = effect_value; }
					break;

				case SpellEffect::MeleeLifetap:
					if (negate_spellbonus) { spellbonuses.MeleeLifetap = effect_value; }
					if (negate_aabonus) { aabonuses.MeleeLifetap = effect_value; }
					if (negate_itembonus) { itembonuses.MeleeLifetap = effect_value; }
					break;

				case SpellEffect::AllInstrumentMod:
				{
					if (negate_spellbonus) {
						spellbonuses.singingMod    = effect_value;
						spellbonuses.brassMod      = effect_value;
						spellbonuses.percussionMod = effect_value;
						spellbonuses.windMod       = effect_value;
						spellbonuses.stringedMod   = effect_value;
					}

					if (negate_itembonus) {
						itembonuses.singingMod    = effect_value;
						itembonuses.brassMod      = effect_value;
						itembonuses.percussionMod = effect_value;
						itembonuses.windMod       = effect_value;
						itembonuses.stringedMod   = effect_value;
					}

					if (negate_aabonus) {
						aabonuses.singingMod    = effect_value;
						aabonuses.brassMod      = effect_value;
						aabonuses.percussionMod = effect_value;
						aabonuses.windMod       = effect_value;
						aabonuses.stringedMod   = effect_value;
					}
					break;
				}

				case SpellEffect::ResistSpellChance:
					if (negate_spellbonus) { spellbonuses.ResistSpellChance = effect_value; }
					if (negate_aabonus) { aabonuses.ResistSpellChance = effect_value; }
					if (negate_itembonus) { itembonuses.ResistSpellChance = effect_value; }
					break;

				case SpellEffect::ResistFearChance:
					if (negate_spellbonus) {spellbonuses.ResistFearChance = effect_value;	}
					if (negate_aabonus) { aabonuses.ResistFearChance = effect_value; }
					if (negate_itembonus) { itembonuses.ResistFearChance = effect_value; }
					break;

				case SpellEffect::Fearless:
					if (negate_spellbonus) { spellbonuses.Fearless = false; }
					if (negate_aabonus) { aabonuses.Fearless = false; }
					if (negate_itembonus) { itembonuses.Fearless = false; }
					break;

				case SpellEffect::HundredHands:
					if (negate_spellbonus) { spellbonuses.HundredHands = effect_value; }
					if (negate_aabonus) { aabonuses.HundredHands = effect_value; }
					if (negate_itembonus) { itembonuses.HundredHands = effect_value; }
					break;

				case SpellEffect::MeleeSkillCheck:
				{
					if (negate_spellbonus) { spellbonuses.MeleeSkillCheck = effect_value; }
					if (negate_spellbonus) { spellbonuses.MeleeSkillCheckSkill = effect_value; }
					break;
				}

				case SpellEffect::HitChance:
				{
					for (int e = 0; e < EQ::skills::HIGHEST_SKILL + 1; e++)
					{
						if (negate_spellbonus) { spellbonuses.HitChanceEffect[e] = effect_value; }
						if (negate_aabonus) { aabonuses.HitChanceEffect[e] = effect_value; }
						if (negate_itembonus) { itembonuses.HitChanceEffect[e] = effect_value; }
					}
					break;
				}

				case SpellEffect::DamageModifier:
				{
					for (int e = 0; e < EQ::skills::HIGHEST_SKILL + 1; e++)
					{
						if (negate_spellbonus) { spellbonuses.DamageModifier[e] = effect_value; }
						if (negate_aabonus) { aabonuses.DamageModifier[e] = effect_value; }
						if (negate_itembonus) { itembonuses.DamageModifier[e] = effect_value; }
					}
					break;
				}

				case SpellEffect::DamageModifier2:
				{
					for (int e = 0; e < EQ::skills::HIGHEST_SKILL + 1; e++)
					{
						if (negate_spellbonus) { spellbonuses.DamageModifier2[e] = effect_value; }
						if (negate_aabonus) { aabonuses.DamageModifier2[e] = effect_value; }
						if (negate_itembonus) { itembonuses.DamageModifier2[e] = effect_value; }
					}
					break;
				}

				case SpellEffect::Skill_Base_Damage_Mod:
				{
					for (int e = 0; e < EQ::skills::HIGHEST_SKILL + 1; e++)
					{
						if (negate_spellbonus) { spellbonuses.DamageModifier3[e] = effect_value; }
						if (negate_aabonus) { aabonuses.DamageModifier3[e] = effect_value; }
						if (negate_itembonus) { itembonuses.DamageModifier3[e] = effect_value; }
					}
					break;
				}


				case SpellEffect::MinDamageModifier:
				{
					for (int e = 0; e < EQ::skills::HIGHEST_SKILL + 1; e++)
					{
						if (negate_spellbonus) { spellbonuses.MinDamageModifier[e] = effect_value; }
						if (negate_aabonus) { aabonuses.MinDamageModifier[e] = effect_value; }
						if (negate_itembonus) { itembonuses.MinDamageModifier[e] = effect_value; }
					}
					break;
				}

				case SpellEffect::StunResist:
					if (negate_spellbonus) { spellbonuses.StunResist = effect_value; }
					if (negate_aabonus) { aabonuses.StunResist = effect_value; }
					if (negate_itembonus) { itembonuses.StunResist = effect_value; }
					break;

				case SpellEffect::ProcChance:
					if (negate_spellbonus) { spellbonuses.ProcChanceSPA = effect_value; }
					if (negate_aabonus) { aabonuses.ProcChanceSPA = effect_value; }
					if (negate_itembonus) { itembonuses.ProcChanceSPA = effect_value; }
					break;

				case SpellEffect::ExtraAttackChance:
					if (negate_spellbonus) { spellbonuses.ExtraAttackChance[SBIndex::EXTRA_ATTACK_CHANCE] = effect_value; }
					if (negate_aabonus) { aabonuses.ExtraAttackChance[SBIndex::EXTRA_ATTACK_CHANCE] = effect_value; }
					if (negate_itembonus) { itembonuses.ExtraAttackChance[SBIndex::EXTRA_ATTACK_CHANCE] = effect_value; }
					break;

				case SpellEffect::AddExtraAttackPct_1h_Primary:
					if (negate_spellbonus) { spellbonuses.ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_CHANCE] = effect_value; }
					if (negate_aabonus) { aabonuses.ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_CHANCE] = effect_value; }
					if (negate_itembonus) { itembonuses.ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_CHANCE] = effect_value; }
					break;

				case SpellEffect::AddExtraAttackPct_1h_Secondary:
					if (negate_spellbonus) { spellbonuses.ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_CHANCE] = effect_value; }
					if (negate_aabonus) { aabonuses.ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_CHANCE] = effect_value; }
					if (negate_itembonus) { itembonuses.ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_CHANCE] = effect_value; }
					break;

				case SpellEffect::Double_Melee_Round:
					if (negate_spellbonus) { spellbonuses.DoubleMeleeRound[SBIndex::DOUBLE_MELEE_ROUND_CHANCE] = effect_value; }
					if (negate_aabonus) { aabonuses.DoubleMeleeRound[SBIndex::DOUBLE_MELEE_ROUND_CHANCE] = effect_value; }
					if (negate_itembonus) { itembonuses.DoubleMeleeRound[SBIndex::DOUBLE_MELEE_ROUND_CHANCE] = effect_value; }
					break;

				case SpellEffect::PercentXPIncrease:
					if (negate_spellbonus) { spellbonuses.XPRateMod = effect_value; }
					if (negate_aabonus) { aabonuses.XPRateMod = effect_value; }
					if (negate_itembonus) { itembonuses.XPRateMod = effect_value; }
					break;

				case SpellEffect::Flurry:
					if (negate_spellbonus) { spellbonuses.FlurryChance = effect_value; }
					if (negate_aabonus) { aabonuses.FlurryChance = effect_value; }
					if (negate_itembonus) { itembonuses.FlurryChance = effect_value; }
					break;

				case SpellEffect::Accuracy:
				{
					if (negate_spellbonus) { spellbonuses.Accuracy[EQ::skills::HIGHEST_SKILL + 1] = effect_value; }
					if (negate_itembonus) { itembonuses.Accuracy[EQ::skills::HIGHEST_SKILL + 1] = effect_value; }

					for (int e = 0; e < EQ::skills::HIGHEST_SKILL + 1; e++)
					{
						if (negate_aabonus) { aabonuses.Accuracy[e] = effect_value; }
					}
					break;
				}

				case SpellEffect::MaxHPChange:
					if (negate_spellbonus) { spellbonuses.PercentMaxHPChange = effect_value; }
					if (negate_aabonus) { aabonuses.PercentMaxHPChange = effect_value; }
					if (negate_itembonus) { itembonuses.PercentMaxHPChange = effect_value; }
					break;

				case SpellEffect::EndurancePool:
					if (negate_spellbonus) { spellbonuses.Endurance = effect_value; }
					if (negate_aabonus) { aabonuses.Endurance = effect_value; }
					if (negate_itembonus) { itembonuses.Endurance = effect_value; }
					break;

				case SpellEffect::HealRate:
					if (negate_spellbonus) { spellbonuses.HealRate = effect_value; }
					if (negate_aabonus) { aabonuses.HealRate = effect_value; }
					if (negate_itembonus) { itembonuses.HealRate = effect_value; }
					break;

				case SpellEffect::SkillDamageTaken:
				{
					for (int e = 0; e < EQ::skills::HIGHEST_SKILL + 1; e++)
					{
						if (negate_spellbonus) { spellbonuses.SkillDmgTaken[e] = effect_value; }
						if (negate_aabonus) { aabonuses.SkillDmgTaken[e] = effect_value; }
						if (negate_itembonus) { itembonuses.SkillDmgTaken[e] = effect_value; }

					}
					break;
				}

				case SpellEffect::SpellCritChance:
					if (negate_spellbonus) { spellbonuses.CriticalSpellChance = effect_value; }
					if (negate_aabonus) { aabonuses.CriticalSpellChance = effect_value; }
					if (negate_itembonus) { itembonuses.CriticalSpellChance = effect_value; }
					break;

				case SpellEffect::CriticalSpellChance:
					if (negate_spellbonus) {
						spellbonuses.CriticalSpellChance  = effect_value;
						spellbonuses.SpellCritDmgIncrease = effect_value;
					}

					if (negate_aabonus) {
						aabonuses.CriticalSpellChance  = effect_value;
						aabonuses.SpellCritDmgIncrease = effect_value;
					}

					if (negate_itembonus) {
						itembonuses.CriticalSpellChance  = effect_value;
						itembonuses.SpellCritDmgIncrease = effect_value;
					}

					break;

				case SpellEffect::SpellCritDmgIncrease:
					if (negate_spellbonus) { spellbonuses.SpellCritDmgIncrease = effect_value; }
					if (negate_aabonus) { aabonuses.SpellCritDmgIncrease = effect_value; }
					if (negate_itembonus) { itembonuses.SpellCritDmgIncrease = effect_value; }
					break;

				case SpellEffect::DotCritDmgIncrease:
					if (negate_spellbonus) { spellbonuses.DotCritDmgIncrease = effect_value; }
					if (negate_aabonus) { aabonuses.DotCritDmgIncrease = effect_value; }
					if (negate_itembonus) { itembonuses.DotCritDmgIncrease = effect_value; }
					break;

				case SpellEffect::CriticalHealChance:
					if (negate_spellbonus) { spellbonuses.CriticalHealChance = effect_value; }
					if (negate_aabonus) { aabonuses.CriticalHealChance = effect_value; }
					if (negate_itembonus) { itembonuses.CriticalHealChance = effect_value; }
					break;

				case SpellEffect::CriticalHealOverTime:
					if (negate_spellbonus) { spellbonuses.CriticalHealOverTime = effect_value; }
					if (negate_aabonus) { aabonuses.CriticalHealOverTime = effect_value; }
					if (negate_itembonus) { itembonuses.CriticalHealOverTime = effect_value; }
					break;

				case SpellEffect::MitigateDamageShield:
					if (negate_spellbonus) { spellbonuses.DSMitigationOffHand = effect_value; }
					if (negate_itembonus) { itembonuses.DSMitigationOffHand = effect_value; }
					if (negate_aabonus) { aabonuses.DSMitigationOffHand = effect_value; }
					break;

				case SpellEffect::CriticalDoTChance:
					if (negate_spellbonus) { spellbonuses.CriticalDoTChance = effect_value; }
					if (negate_aabonus) { aabonuses.CriticalDoTChance = effect_value; }
					if (negate_itembonus) { itembonuses.CriticalDoTChance = effect_value; }
					break;

				case SpellEffect::ProcOnKillShot:
				{
					for (int e = 0; e < MAX_SPELL_TRIGGER * 3; e += 3)
					{
						if (negate_spellbonus) {
							spellbonuses.SpellOnKill[e] = effect_value;
							spellbonuses.SpellOnKill[e + 1] = effect_value;
							spellbonuses.SpellOnKill[e + 2] = effect_value;
						}

						if (negate_aabonus) {
							aabonuses.SpellOnKill[e] = effect_value;
							aabonuses.SpellOnKill[e + 1] = effect_value;
							aabonuses.SpellOnKill[e + 2] = effect_value;
						}

						if (negate_itembonus) {
							itembonuses.SpellOnKill[e] = effect_value;
							itembonuses.SpellOnKill[e + 1] = effect_value;
							itembonuses.SpellOnKill[e + 2] = effect_value;
						}
					}

					break;
				}

				/*
				case SpellEffect::SpellOnDeath:
				{
					for(int e = 0; e < MAX_SPELL_TRIGGER; e=2)
					{
						if  (negate_spellbonus) { spellbonuses.SpellOnDeath[e] = SPELL_UNKNOWN;
						if  (negate_spellbonus) { spellbonuses.SpellOnDeath[e+1] = effect_value; }
					}
					break;
				}
				*/

				case SpellEffect::CriticalDamageMob:
				{
					for (int e = 0; e < EQ::skills::HIGHEST_SKILL + 1; e++)
					{
						if (negate_spellbonus) { spellbonuses.CritDmgMod[e] = effect_value; }
						if (negate_aabonus) { aabonuses.CritDmgMod[e] = effect_value; }
						if (negate_itembonus) { itembonuses.CritDmgMod[e] = effect_value; }
					}
					break;
				}

				case SpellEffect::Critical_Melee_Damage_Mod_Max:
				{
					for (int e = 0; e < EQ::skills::HIGHEST_SKILL + 1; e++)
					{
						if (negate_spellbonus) { spellbonuses.CritDmgModNoStack[e] = effect_value; }
						if (negate_aabonus) { aabonuses.CritDmgModNoStack[e] = effect_value; }
						if (negate_itembonus) { itembonuses.CritDmgModNoStack[e] = effect_value; }
					}
					break;
				}

				case SpellEffect::SkillDamageAmount:
				{
					for (int e = 0; e < EQ::skills::HIGHEST_SKILL + 1; e++)
					{
						if (negate_spellbonus) { spellbonuses.SkillDamageAmount[e] = effect_value; }
						if (negate_aabonus) { aabonuses.SkillDamageAmount[e] = effect_value; }
						if (negate_itembonus) { itembonuses.SkillDamageAmount[e] = effect_value; }
					}
					break;
				}

				case SpellEffect::IncreaseBlockChance:
					if (negate_spellbonus) { spellbonuses.IncreaseBlockChance = effect_value; }
					if (negate_aabonus) { aabonuses.IncreaseBlockChance = effect_value; }
					if (negate_itembonus) { itembonuses.IncreaseBlockChance = effect_value; }
					break;

				case SpellEffect::PersistantCasting:
					if (negate_spellbonus) { spellbonuses.PersistantCasting = effect_value; }
					if (negate_itembonus) { itembonuses.PersistantCasting = effect_value; }
					if (negate_aabonus) { aabonuses.PersistantCasting = effect_value; }
					break;

				case SpellEffect::ImmuneFleeing:
					if (negate_spellbonus) { spellbonuses.ImmuneToFlee = false; }
					break;

				case SpellEffect::DelayDeath:
					if (negate_spellbonus) { spellbonuses.DelayDeath = effect_value; }
					if (negate_aabonus) { aabonuses.DelayDeath = effect_value; }
					if (negate_itembonus) { itembonuses.DelayDeath = effect_value; }
					break;

				case SpellEffect::SpellProcChance:
					if (negate_spellbonus) { spellbonuses.SpellProcChance = effect_value; }
					if (negate_itembonus) { itembonuses.SpellProcChance = effect_value; }
					if (negate_aabonus) { aabonuses.SpellProcChance = effect_value; }
					break;

				case SpellEffect::CharmBreakChance:
					if (negate_spellbonus) { spellbonuses.CharmBreakChance = effect_value; }
					if (negate_aabonus) { aabonuses.CharmBreakChance = effect_value; }
					if (negate_itembonus) { itembonuses.CharmBreakChance = effect_value; }
					break;

				case SpellEffect::BardSongRange:
					if (negate_spellbonus) { spellbonuses.SongRange = effect_value; }
					if (negate_aabonus) { aabonuses.SongRange = effect_value; }
					if (negate_itembonus) { itembonuses.SongRange = effect_value; }
					break;

				case SpellEffect::SkillDamageAmount2:
				{
					for (int e = 0; e < EQ::skills::HIGHEST_SKILL + 1; e++)
					{
						if (negate_spellbonus) { spellbonuses.SkillDamageAmount2[e] = effect_value; }
						if (negate_aabonus) { aabonuses.SkillDamageAmount2[e] = effect_value; }
						if (negate_itembonus) { itembonuses.SkillDamageAmount2[e] = effect_value; }
					}
					break;
				}

				case SpellEffect::NegateAttacks:
					if (negate_spellbonus) {
						spellbonuses.NegateAttacks[SBIndex::NEGATE_ATK_EXISTS]   = effect_value;
						spellbonuses.NegateAttacks[SBIndex::NEGATE_ATK_BUFFSLOT] = effect_value;
					}

					break;

				case SpellEffect::MitigateMeleeDamage:
					if (negate_spellbonus) {
						spellbonuses.MitigateMeleeRune[SBIndex::MITIGATION_RUNE_PERCENT]  = effect_value;
						spellbonuses.MitigateMeleeRune[SBIndex::MITIGATION_RUNE_BUFFSLOT] = -1;
					}

					break;

				case SpellEffect::MeleeThresholdGuard:
					if (negate_spellbonus) {
						spellbonuses.MeleeThresholdGuard[SBIndex::THRESHOLDGUARD_MITIGATION_PERCENT] = effect_value;
						spellbonuses.MeleeThresholdGuard[SBIndex::THRESHOLDGUARD_BUFFSLOT]           = -1;
						spellbonuses.MeleeThresholdGuard[SBIndex::THRESHOLDGUARD_BUFFSLOT]           = effect_value;
					}

					break;

				case SpellEffect::SpellThresholdGuard:
					if (negate_spellbonus) {
						spellbonuses.SpellThresholdGuard[SBIndex::THRESHOLDGUARD_MITIGATION_PERCENT] = effect_value;
						spellbonuses.SpellThresholdGuard[SBIndex::THRESHOLDGUARD_BUFFSLOT]           = -1;
						spellbonuses.SpellThresholdGuard[SBIndex::THRESHOLDGUARD_BUFFSLOT]           = effect_value;
					}

					break;

				case SpellEffect::MitigateSpellDamage:
					if (negate_spellbonus) {
						spellbonuses.MitigateSpellRune[SBIndex::MITIGATION_RUNE_PERCENT]  = effect_value;
						spellbonuses.MitigateSpellRune[SBIndex::MITIGATION_RUNE_BUFFSLOT] = -1;
					}

					break;

				case SpellEffect::MitigateDotDamage:
					if (negate_spellbonus) {
						spellbonuses.MitigateDotRune[SBIndex::MITIGATION_RUNE_PERCENT]  = effect_value;
						spellbonuses.MitigateDotRune[SBIndex::MITIGATION_RUNE_BUFFSLOT] = -1;
					}

					break;

				case SpellEffect::ManaAbsorbPercentDamage:
					if (negate_spellbonus) { spellbonuses.ManaAbsorbPercentDamage = effect_value; }
					break;

				case SpellEffect::Endurance_Absorb_Pct_Damage:
					if (negate_spellbonus) {
						spellbonuses.EnduranceAbsorbPercentDamage[SBIndex::ENDURANCE_ABSORD_MITIGIATION]  = effect_value;
						spellbonuses.EnduranceAbsorbPercentDamage[SBIndex::ENDURANCE_ABSORD_DRAIN_PER_HP] = effect_value;
					}

					break;

				case SpellEffect::ShieldBlock:
					if (negate_spellbonus) { spellbonuses.ShieldBlock = effect_value; }
					if (negate_aabonus) { aabonuses.ShieldBlock = effect_value; }
					if (negate_itembonus) { itembonuses.ShieldBlock = effect_value; }

				case SpellEffect::BlockBehind:
					if (negate_spellbonus) { spellbonuses.BlockBehind = effect_value; }
					if (negate_aabonus) { aabonuses.BlockBehind = effect_value; }
					if (negate_itembonus) { itembonuses.BlockBehind = effect_value; }
					break;

				case SpellEffect::Blind:
					if (negate_spellbonus) { spellbonuses.IsBlind = false; }
					break;

				case SpellEffect::Fear:
					if (negate_spellbonus) { spellbonuses.IsFeared = false; }
					break;

				case SpellEffect::FrontalStunResist:
					if (negate_spellbonus) { spellbonuses.FrontalStunResist = effect_value; }
					if (negate_aabonus) { aabonuses.FrontalStunResist = effect_value; }
					if (negate_itembonus) { itembonuses.FrontalStunResist = effect_value; }
					break;

				case SpellEffect::ImprovedBindWound:
					if (negate_aabonus) { aabonuses.BindWound = effect_value; }
					if (negate_itembonus) { itembonuses.BindWound = effect_value; }
					if (negate_spellbonus) { spellbonuses.BindWound = effect_value; }
					break;

				case SpellEffect::MaxBindWound:
					if (negate_spellbonus) { spellbonuses.MaxBindWound = effect_value; }
					if (negate_aabonus) { aabonuses.MaxBindWound = effect_value; }
					if (negate_itembonus) { itembonuses.MaxBindWound = effect_value; }
					break;

				case SpellEffect::BaseMovementSpeed:
					if (negate_spellbonus) { spellbonuses.BaseMovementSpeed = effect_value; }
					if (negate_aabonus) { aabonuses.BaseMovementSpeed = effect_value; }
					if (negate_itembonus) { itembonuses.BaseMovementSpeed = effect_value; }
					break;

				case SpellEffect::IncreaseRunSpeedCap:
					if (negate_itembonus) { itembonuses.IncreaseRunSpeedCap = effect_value; }
					if (negate_aabonus) { aabonuses.IncreaseRunSpeedCap = effect_value; }
					if (negate_spellbonus) { spellbonuses.IncreaseRunSpeedCap = effect_value; }
					break;

				case SpellEffect::DoubleSpecialAttack:
					if (negate_spellbonus) { spellbonuses.DoubleSpecialAttack = effect_value; }
					if (negate_aabonus) { aabonuses.DoubleSpecialAttack = effect_value; }
					if (negate_itembonus) { itembonuses.DoubleSpecialAttack = effect_value; }
					break;

				case SpellEffect::TripleBackstab:
					if (negate_spellbonus) { spellbonuses.TripleBackstab = effect_value; }
					if (negate_aabonus) { aabonuses.TripleBackstab = effect_value; }
					if (negate_itembonus) { itembonuses.TripleBackstab = effect_value; }
					break;

				case SpellEffect::FrontalBackstabMinDmg:
					if (negate_spellbonus) { spellbonuses.FrontalBackstabMinDmg = false; }
					break;

				case SpellEffect::FrontalBackstabChance:
					if (negate_spellbonus) { spellbonuses.FrontalBackstabChance = effect_value; }
					if (negate_aabonus) { aabonuses.FrontalBackstabChance = effect_value; }
					if (negate_itembonus) { itembonuses.FrontalBackstabChance = effect_value; }
					break;

				case SpellEffect::Double_Backstab_Front:
					if (negate_spellbonus) { spellbonuses.Double_Backstab_Front = effect_value; }
					if (negate_aabonus) { aabonuses.Double_Backstab_Front = effect_value; }
					if (negate_itembonus) { itembonuses.Double_Backstab_Front = effect_value; }
					break;

				case SpellEffect::ConsumeProjectile:
					if (negate_spellbonus) { spellbonuses.ConsumeProjectile = effect_value; }
					if (negate_aabonus) { aabonuses.ConsumeProjectile = effect_value; }
					if (negate_itembonus) { itembonuses.ConsumeProjectile = effect_value; }
					break;

				case SpellEffect::ForageAdditionalItems:
					if (negate_spellbonus) { spellbonuses.ForageAdditionalItems = effect_value; }
					if (negate_aabonus) { aabonuses.ForageAdditionalItems = effect_value; }
					if (negate_itembonus) { itembonuses.ForageAdditionalItems = effect_value; }
					break;

				case SpellEffect::Salvage:
					if (negate_spellbonus) { spellbonuses.SalvageChance = effect_value; }
					if (negate_aabonus) { aabonuses.SalvageChance = effect_value; }
					if (negate_itembonus) { itembonuses.SalvageChance = effect_value; }
					break;

				case SpellEffect::ArcheryDamageModifier:
					if (negate_spellbonus) { spellbonuses.ArcheryDamageModifier = effect_value; }
					if (negate_aabonus) { aabonuses.ArcheryDamageModifier = effect_value; }
					if (negate_itembonus) { itembonuses.ArcheryDamageModifier = effect_value; }
					break;

				case SpellEffect::SecondaryDmgInc:
					if (negate_spellbonus) { spellbonuses.SecondaryDmgInc = false; }
					if (negate_aabonus) { aabonuses.SecondaryDmgInc = false; }
					if (negate_itembonus) { itembonuses.SecondaryDmgInc = false; }
					break;

				case SpellEffect::StrikeThrough:
				case SpellEffect::StrikeThrough2:
					if (negate_spellbonus) { spellbonuses.StrikeThrough = effect_value; }
					if (negate_aabonus) { aabonuses.StrikeThrough = effect_value; }
					if (negate_itembonus) { itembonuses.StrikeThrough = effect_value; }
					break;

				case SpellEffect::GiveDoubleAttack:
					if (negate_spellbonus) { spellbonuses.GiveDoubleAttack = effect_value; }
					if (negate_aabonus) { aabonuses.GiveDoubleAttack = effect_value; }
					if (negate_itembonus) { itembonuses.GiveDoubleAttack = effect_value; }
					break;

				case SpellEffect::PetCriticalHit:
					if (negate_spellbonus) { spellbonuses.PetCriticalHit = effect_value; }
					if (negate_aabonus) { aabonuses.PetCriticalHit = effect_value; }
					if (negate_itembonus) { itembonuses.PetCriticalHit = effect_value; }
					break;

				case SpellEffect::CombatStability:
					if (negate_spellbonus) { spellbonuses.CombatStability = effect_value; }
					if (negate_aabonus) { aabonuses.CombatStability = effect_value; }
					if (negate_itembonus) { itembonuses.CombatStability = effect_value; }
					break;

				case SpellEffect::PetAvoidance:
					if (negate_spellbonus) { spellbonuses.PetAvoidance = effect_value; }
					if (negate_aabonus) { aabonuses.PetAvoidance = effect_value; }
					if (negate_itembonus) { itembonuses.PetAvoidance = effect_value; }
					break;

				case SpellEffect::Ambidexterity:
					if (negate_spellbonus) { spellbonuses.Ambidexterity = effect_value; }
					if (negate_aabonus) { aabonuses.Ambidexterity = effect_value; }
					if (negate_itembonus) { itembonuses.Ambidexterity = effect_value; }
					break;

				case SpellEffect::PetMaxHP:
					if (negate_spellbonus) { spellbonuses.PetMaxHP = effect_value; }
					if (negate_aabonus) { aabonuses.PetMaxHP = effect_value; }
					if (negate_itembonus) { itembonuses.PetMaxHP = effect_value; }
					break;

				case SpellEffect::PetFlurry:
					if (negate_spellbonus) { spellbonuses.PetFlurry = effect_value; }
					if (negate_aabonus) { aabonuses.PetFlurry = effect_value; }
					if (negate_itembonus) { itembonuses.PetFlurry = effect_value; }
					break;

				case SpellEffect::GivePetGroupTarget:
					if (negate_spellbonus) { spellbonuses.GivePetGroupTarget = false; }
					if (negate_aabonus) { aabonuses.GivePetGroupTarget = false; }
					if (negate_itembonus) { itembonuses.GivePetGroupTarget = false; }
					break;

				case SpellEffect::PetMeleeMitigation:
					if (negate_spellbonus) { spellbonuses.PetMeleeMitigation = effect_value; }
					if (negate_itembonus) { itembonuses.PetMeleeMitigation = effect_value; }
					if (negate_aabonus) { aabonuses.PetMeleeMitigation = effect_value; }
					break;

				case SpellEffect::RootBreakChance:
					if (negate_spellbonus) { spellbonuses.RootBreakChance = effect_value; }
					if (negate_aabonus) { aabonuses.RootBreakChance = effect_value; }
					if (negate_itembonus) { itembonuses.RootBreakChance = effect_value; }
					break;

				case SpellEffect::ChannelChanceItems:
					if (negate_spellbonus) { spellbonuses.ChannelChanceItems = effect_value; }
					if (negate_aabonus) { aabonuses.ChannelChanceItems = effect_value; }
					if (negate_itembonus) { itembonuses.ChannelChanceItems = effect_value; }
					break;

				case SpellEffect::ChannelChanceSpells:
					if (negate_spellbonus) { spellbonuses.ChannelChanceSpells = effect_value; }
					if (negate_aabonus) { aabonuses.ChannelChanceSpells = effect_value; }
					if (negate_itembonus) { itembonuses.ChannelChanceSpells = effect_value; }
					break;

				case SpellEffect::UnfailingDivinity:
					if (negate_spellbonus) { spellbonuses.UnfailingDivinity = effect_value; }
					if (negate_aabonus) { aabonuses.UnfailingDivinity = effect_value; }
					if (negate_itembonus) { itembonuses.UnfailingDivinity = effect_value; }
					break;

				case SpellEffect::ItemHPRegenCapIncrease:
					if (negate_spellbonus) { spellbonuses.ItemHPRegenCap = effect_value; }
					if (negate_aabonus) { aabonuses.ItemHPRegenCap = effect_value; }
					if (negate_itembonus) { itembonuses.ItemHPRegenCap = effect_value; }
					break;

				case SpellEffect::Worn_Endurance_Regen_Cap:
					if (negate_spellbonus) { spellbonuses.ItemEnduranceRegenCap = effect_value; }
					if (negate_aabonus) { aabonuses.ItemEnduranceRegenCap = effect_value; }
					if (negate_itembonus) { itembonuses.ItemEnduranceRegenCap = effect_value; }
					break;

				case SpellEffect::OffhandRiposteFail:
					if (negate_spellbonus) { spellbonuses.OffhandRiposteFail = effect_value; }
					if (negate_aabonus) { aabonuses.OffhandRiposteFail = effect_value; }
					if (negate_itembonus) { itembonuses.OffhandRiposteFail = effect_value; }
					break;

				case SpellEffect::ItemAttackCapIncrease:
					if (negate_aabonus) { aabonuses.ItemATKCap = effect_value; }
					if (negate_itembonus) { itembonuses.ItemATKCap = effect_value; }
					if (negate_spellbonus) { spellbonuses.ItemATKCap = effect_value; }
					break;

				case SpellEffect::SpellEffectResistChance:
				{
					for (int e = 0; e < MAX_RESISTABLE_EFFECTS * 2; e += 2)
					{
						if (negate_spellbonus) {
							spellbonuses.SEResist[e]     = effect_value;
							spellbonuses.SEResist[e + 1] = effect_value;
						}
						if (negate_itembonus) {
							itembonuses.SEResist[e] = effect_value;
							itembonuses.SEResist[e + 1] = effect_value;
						}
						if (negate_aabonus) {
							aabonuses.SEResist[e] = effect_value;
							aabonuses.SEResist[e + 1] = effect_value;
						}
					}
					break;
				}

				case SpellEffect::MasteryofPast:
					if (negate_spellbonus) { spellbonuses.MasteryofPast = effect_value; }
					if (negate_aabonus) { aabonuses.MasteryofPast = effect_value; }
					if (negate_itembonus) { itembonuses.MasteryofPast = effect_value; }
					break;

				case SpellEffect::DoubleRiposte:
					if (negate_spellbonus) { spellbonuses.DoubleRiposte = effect_value; }
					if (negate_itembonus) { itembonuses.DoubleRiposte = effect_value; }
					if (negate_aabonus) { aabonuses.DoubleRiposte = effect_value; }
					break;

				case SpellEffect::GiveDoubleRiposte:
					if (negate_spellbonus) { spellbonuses.GiveDoubleRiposte[SBIndex::DOUBLE_RIPOSTE_CHANCE] = effect_value; }
					if (negate_itembonus) { itembonuses.GiveDoubleRiposte[SBIndex::DOUBLE_RIPOSTE_CHANCE] = effect_value; }
					if (negate_aabonus) { aabonuses.GiveDoubleRiposte[SBIndex::DOUBLE_RIPOSTE_CHANCE] = effect_value; }
					break;

				case SpellEffect::SlayUndead:
					if (negate_spellbonus) {
						spellbonuses.SlayUndead[SBIndex::SLAYUNDEAD_RATE_MOD] = effect_value;
						spellbonuses.SlayUndead[SBIndex::SLAYUNDEAD_DMG_MOD]  = effect_value;
					}

					if (negate_itembonus) {
						itembonuses.SlayUndead[SBIndex::SLAYUNDEAD_RATE_MOD] = effect_value;
						itembonuses.SlayUndead[SBIndex::SLAYUNDEAD_DMG_MOD]  = effect_value;
					}

					if (negate_aabonus) {
						aabonuses.SlayUndead[SBIndex::SLAYUNDEAD_RATE_MOD] = effect_value;
						aabonuses.SlayUndead[SBIndex::SLAYUNDEAD_DMG_MOD]  = effect_value;
					}

					break;

				case SpellEffect::DoubleRangedAttack:
					if (negate_spellbonus) { spellbonuses.DoubleRangedAttack = effect_value; }
					if (negate_aabonus) { aabonuses.DoubleRangedAttack = effect_value; }
					if (negate_itembonus) { itembonuses.DoubleRangedAttack = effect_value; }
					break;

				case SpellEffect::ShieldEquipDmgMod:
					if (negate_spellbonus) { spellbonuses.ShieldEquipDmgMod = effect_value; }
					if (negate_aabonus) { aabonuses.ShieldEquipDmgMod = effect_value; }
					if (negate_itembonus) { itembonuses.ShieldEquipDmgMod = effect_value; }
					break;

				case SpellEffect::TriggerMeleeThreshold:
					if (negate_spellbonus) { spellbonuses.TriggerMeleeThreshold = false; }
					break;

				case SpellEffect::TriggerSpellThreshold:
					if (negate_spellbonus) { spellbonuses.TriggerSpellThreshold = false; }
					break;

				case SpellEffect::DivineAura:
					if (negate_spellbonus) { spellbonuses.DivineAura = false; }
					break;

				case SpellEffect::StunBashChance:
					if (negate_spellbonus) { spellbonuses.StunBashChance = effect_value; }
					if (negate_itembonus) { itembonuses.StunBashChance = effect_value; }
					if (negate_aabonus) { aabonuses.StunBashChance = effect_value; }
					break;

				case SpellEffect::IncreaseChanceMemwipe:
					if (negate_spellbonus) { spellbonuses.IncreaseChanceMemwipe = effect_value; }
					if (negate_itembonus) { itembonuses.IncreaseChanceMemwipe = effect_value; }
					if (negate_aabonus) { aabonuses.IncreaseChanceMemwipe = effect_value; }
					break;

				case SpellEffect::CriticalMend:
					if (negate_spellbonus) { spellbonuses.CriticalMend = effect_value; }
					if (negate_itembonus) { itembonuses.CriticalMend = effect_value; }
					if (negate_aabonus) { aabonuses.CriticalMend = effect_value; }
					break;

				case SpellEffect::DistanceRemoval:
					if (negate_spellbonus) { spellbonuses.DistanceRemoval = false; }
					break;

				case SpellEffect::ImprovedTaunt:
					if (negate_spellbonus) {
						spellbonuses.ImprovedTaunt[SBIndex::IMPROVED_TAUNT_MAX_LVL]   = effect_value;
						spellbonuses.ImprovedTaunt[SBIndex::IMPROVED_TAUNT_AGGRO_MOD] = effect_value;
						spellbonuses.ImprovedTaunt[SBIndex::IMPROVED_TAUNT_BUFFSLOT]  = -1;
					}

					break;

				case SpellEffect::FrenziedDevastation:
					if (negate_spellbonus) { spellbonuses.FrenziedDevastation = effect_value; }
					if (negate_aabonus) { aabonuses.FrenziedDevastation = effect_value; }
					if (negate_itembonus) { itembonuses.FrenziedDevastation = effect_value; }
					break;

				case SpellEffect::Root:
					if (negate_spellbonus) {
						spellbonuses.Root[SBIndex::ROOT_EXISTS]   = effect_value;
						spellbonuses.Root[SBIndex::ROOT_BUFFSLOT] = -1;
					}

					break;

				case SpellEffect::Rune:
					if (negate_spellbonus) {
						spellbonuses.MeleeRune[SBIndex::RUNE_AMOUNT]   = effect_value;
						spellbonuses.MeleeRune[SBIndex::RUNE_BUFFSLOT] = -1;
					}

					break;

				case SpellEffect::AbsorbMagicAtt:
					if (negate_spellbonus) {
						spellbonuses.AbsorbMagicAtt[SBIndex::RUNE_AMOUNT]   = effect_value;
						spellbonuses.AbsorbMagicAtt[SBIndex::RUNE_BUFFSLOT] = -1;
					}

					break;

				case SpellEffect::Berserk:
					if (negate_spellbonus) { spellbonuses.BerserkSPA = false; }
					if (negate_aabonus) { aabonuses.BerserkSPA = false; }
					if (negate_itembonus) { itembonuses.BerserkSPA = false; }
					break;

				case SpellEffect::Vampirism:
					if (negate_spellbonus) { spellbonuses.Vampirism = effect_value; }
					if (negate_aabonus) { aabonuses.Vampirism = effect_value; }
					if (negate_itembonus) { itembonuses.Vampirism = effect_value; }
					break;

				case SpellEffect::Metabolism:
					if (negate_spellbonus) { spellbonuses.Metabolism = effect_value; }
					if (negate_aabonus) { aabonuses.Metabolism = effect_value; }
					if (negate_itembonus) { itembonuses.Metabolism = effect_value; }
					break;

				case SpellEffect::ImprovedReclaimEnergy:
					if (negate_spellbonus) { spellbonuses.ImprovedReclaimEnergy = effect_value; }
					if (negate_aabonus) { aabonuses.ImprovedReclaimEnergy = effect_value; }
					if (negate_itembonus) { itembonuses.ImprovedReclaimEnergy = effect_value; }
					break;

				case SpellEffect::HeadShot:
					if (negate_spellbonus) {
						spellbonuses.HeadShot[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = effect_value;
						spellbonuses.HeadShot[SBIndex::FINISHING_EFFECT_DMG]         = effect_value;
					}

					if (negate_aabonus) {
						aabonuses.HeadShot[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = effect_value;
						aabonuses.HeadShot[SBIndex::FINISHING_EFFECT_DMG]         = effect_value;
					}

					if (negate_itembonus) {
						itembonuses.HeadShot[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = effect_value;
						itembonuses.HeadShot[SBIndex::FINISHING_EFFECT_DMG]         = effect_value;
					}

					break;

				case SpellEffect::HeadShotLevel:
					if (negate_spellbonus) {
						spellbonuses.HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX]          = effect_value;
						spellbonuses.HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_CHANCE_BONUS] = effect_value;
					}

					if (negate_aabonus) {
						aabonuses.HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX]          = effect_value;
						aabonuses.HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_CHANCE_BONUS] = effect_value;
					}

					if (negate_itembonus) {
						itembonuses.HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX]          = effect_value;
						itembonuses.HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_CHANCE_BONUS] = effect_value;
					}

					break;

				case SpellEffect::Assassinate:
					if (negate_spellbonus) {
						spellbonuses.Assassinate[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = effect_value;
						spellbonuses.Assassinate[SBIndex::FINISHING_EFFECT_DMG]         = effect_value;
					}

					if (negate_aabonus) {
						aabonuses.Assassinate[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = effect_value;
						aabonuses.Assassinate[SBIndex::FINISHING_EFFECT_DMG]         = effect_value;
					}

					if (negate_itembonus) {
						itembonuses.Assassinate[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = effect_value;
						itembonuses.Assassinate[SBIndex::FINISHING_EFFECT_DMG]         = effect_value;
					}

					break;

				case SpellEffect::AssassinateLevel:
					if (negate_spellbonus) {
						spellbonuses.AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX]          = effect_value;
						spellbonuses.AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_CHANCE_BONUS] = effect_value;
					}

					if (negate_aabonus) {
						aabonuses.AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX]          = effect_value;
						aabonuses.AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_CHANCE_BONUS] = effect_value;
					}

					if (negate_itembonus) {
						itembonuses.AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX]          = effect_value;
						itembonuses.AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_CHANCE_BONUS] = effect_value;
					}

					break;

				case SpellEffect::FinishingBlow:
					if (negate_spellbonus) {
						spellbonuses.FinishingBlow[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = effect_value;
						spellbonuses.FinishingBlow[SBIndex::FINISHING_EFFECT_DMG]         = effect_value;
					}

					if (negate_aabonus) {
						aabonuses.FinishingBlow[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = effect_value;
						aabonuses.FinishingBlow[SBIndex::FINISHING_EFFECT_DMG]         = effect_value;
					}

					if (negate_itembonus) {
						itembonuses.FinishingBlow[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = effect_value;
						itembonuses.FinishingBlow[SBIndex::FINISHING_EFFECT_DMG]         = effect_value;
					}

					break;

				case SpellEffect::FinishingBlowLvl:
					if (negate_spellbonus) {
						spellbonuses.FinishingBlowLvl[SBIndex::FINISHING_EFFECT_LEVEL_MAX]    = effect_value;
						spellbonuses.FinishingBlowLvl[SBIndex::FINISHING_BLOW_LEVEL_HP_RATIO] = effect_value;
					}

					if (negate_aabonus) {
						aabonuses.FinishingBlowLvl[SBIndex::FINISHING_EFFECT_LEVEL_MAX]    = effect_value;
						aabonuses.FinishingBlowLvl[SBIndex::FINISHING_BLOW_LEVEL_HP_RATIO] = effect_value;
					}

					if (negate_itembonus) {
						itembonuses.FinishingBlowLvl[SBIndex::FINISHING_EFFECT_LEVEL_MAX]    = effect_value;
						itembonuses.FinishingBlowLvl[SBIndex::FINISHING_BLOW_LEVEL_HP_RATIO] = effect_value;
					}

					break;

				case SpellEffect::Sanctuary:
					if (negate_spellbonus) { spellbonuses.Sanctuary = false; }
					break;

				case SpellEffect::FactionModPct:
					if (negate_spellbonus) { spellbonuses.FactionModPct = effect_value; }
					if (negate_itembonus) { itembonuses.FactionModPct = effect_value; }
					if (negate_aabonus) { aabonuses.FactionModPct = effect_value; }
					break;

				case SpellEffect::IllusionPersistence:
					if (negate_spellbonus) { spellbonuses.IllusionPersistence = effect_value; }
					if (negate_itembonus) { itembonuses.IllusionPersistence = effect_value; }
					if (negate_aabonus) { aabonuses.IllusionPersistence = effect_value; }
					break;

				case SpellEffect::Attack_Accuracy_Max_Percent:
					if (negate_spellbonus) { spellbonuses.Attack_Accuracy_Max_Percent = effect_value; }
					if (negate_itembonus) { itembonuses.Attack_Accuracy_Max_Percent = effect_value; }
					if (negate_aabonus) { aabonuses.Attack_Accuracy_Max_Percent = effect_value; }
					break;


				case SpellEffect::AC_Mitigation_Max_Percent:
					if (negate_spellbonus) { spellbonuses.AC_Mitigation_Max_Percent = effect_value; }
					if (negate_itembonus) { itembonuses.AC_Mitigation_Max_Percent = effect_value; }
					if (negate_aabonus) { aabonuses.AC_Mitigation_Max_Percent = effect_value; }
					break;

				case SpellEffect::AC_Avoidance_Max_Percent:
					if (negate_spellbonus) { spellbonuses.AC_Avoidance_Max_Percent = effect_value; }
					if (negate_itembonus) { itembonuses.AC_Avoidance_Max_Percent = effect_value; }
					if (negate_aabonus) { aabonuses.AC_Avoidance_Max_Percent = effect_value; }
					break;

				case SpellEffect::Melee_Damage_Position_Mod:
					if (negate_spellbonus) {
						spellbonuses.Melee_Damage_Position_Mod[SBIndex::POSITION_BACK]  = effect_value;
						spellbonuses.Melee_Damage_Position_Mod[SBIndex::POSITION_FRONT] = effect_value;
					}

					if (negate_aabonus) {
						aabonuses.Melee_Damage_Position_Mod[SBIndex::POSITION_BACK]  = effect_value;
						aabonuses.Melee_Damage_Position_Mod[SBIndex::POSITION_FRONT] = effect_value;
					}

					if (negate_itembonus) {
						itembonuses.Melee_Damage_Position_Mod[SBIndex::POSITION_BACK]  = effect_value;
						itembonuses.Melee_Damage_Position_Mod[SBIndex::POSITION_FRONT] = effect_value;
					}

					break;

				case SpellEffect::Damage_Taken_Position_Mod:
					if (negate_spellbonus) {
						spellbonuses.Damage_Taken_Position_Mod[SBIndex::POSITION_BACK]  = effect_value;
						spellbonuses.Damage_Taken_Position_Mod[SBIndex::POSITION_FRONT] = effect_value;
					}

					if (negate_aabonus) {
						aabonuses.Damage_Taken_Position_Mod[SBIndex::POSITION_BACK]  = effect_value;
						aabonuses.Damage_Taken_Position_Mod[SBIndex::POSITION_FRONT] = effect_value;
					}

					if (negate_itembonus) {
						itembonuses.Damage_Taken_Position_Mod[SBIndex::POSITION_BACK]  = effect_value;
						itembonuses.Damage_Taken_Position_Mod[SBIndex::POSITION_FRONT] = effect_value;
					}

					break;

				case SpellEffect::Melee_Damage_Position_Amt:
					if (negate_spellbonus) {
						spellbonuses.Melee_Damage_Position_Amt[SBIndex::POSITION_BACK]  = effect_value;
						spellbonuses.Melee_Damage_Position_Amt[SBIndex::POSITION_FRONT] = effect_value;
					}

					if (negate_aabonus) {
						aabonuses.Melee_Damage_Position_Amt[SBIndex::POSITION_BACK]  = effect_value;
						aabonuses.Melee_Damage_Position_Amt[SBIndex::POSITION_FRONT] = effect_value;
					}

					if (negate_itembonus) {
						itembonuses.Melee_Damage_Position_Amt[SBIndex::POSITION_BACK]  = effect_value;
						itembonuses.Melee_Damage_Position_Amt[SBIndex::POSITION_FRONT] = effect_value;
					}

					break;

				case SpellEffect::Damage_Taken_Position_Amt:
					if (negate_spellbonus) {
						spellbonuses.Damage_Taken_Position_Amt[SBIndex::POSITION_BACK]  = effect_value;
						spellbonuses.Damage_Taken_Position_Amt[SBIndex::POSITION_FRONT] = effect_value;
					}

					if (negate_aabonus) {
						aabonuses.Damage_Taken_Position_Amt[SBIndex::POSITION_BACK]  = effect_value;
						aabonuses.Damage_Taken_Position_Amt[SBIndex::POSITION_FRONT] = effect_value;
					}

					if (negate_itembonus) {
						itembonuses.Damage_Taken_Position_Amt[SBIndex::POSITION_BACK]  = effect_value;
						itembonuses.Damage_Taken_Position_Amt[SBIndex::POSITION_FRONT] = effect_value;
					}

					break;

				case SpellEffect::DS_Mitigation_Amount:
					if (negate_spellbonus) { spellbonuses.DS_Mitigation_Amount = effect_value; }
					if (negate_itembonus) { itembonuses.DS_Mitigation_Amount = effect_value; }
					if (negate_aabonus) { aabonuses.DS_Mitigation_Amount = effect_value; }
					break;

				case SpellEffect::DS_Mitigation_Percentage:
					if (negate_spellbonus) { spellbonuses.DS_Mitigation_Percentage = effect_value; }
					if (negate_itembonus) { itembonuses.DS_Mitigation_Percentage = effect_value; }
					if (negate_aabonus) { aabonuses.DS_Mitigation_Percentage = effect_value; }
					break;

				case SpellEffect::Pet_Crit_Melee_Damage_Pct_Owner:
					if (negate_spellbonus) { spellbonuses.Pet_Crit_Melee_Damage_Pct_Owner = effect_value; }
					if (negate_itembonus) { itembonuses.Pet_Crit_Melee_Damage_Pct_Owner = effect_value; }
					if (negate_aabonus) { aabonuses.Pet_Crit_Melee_Damage_Pct_Owner = effect_value; }
					break;

				case SpellEffect::Pet_Add_Atk:
					if (negate_spellbonus) { spellbonuses.Pet_Add_Atk = effect_value; }
					if (negate_itembonus) { itembonuses.Pet_Add_Atk = effect_value; }
					if (negate_aabonus) { aabonuses.Pet_Add_Atk = effect_value; }
					break;

				case SpellEffect::PC_Pet_Rampage:
					if (negate_spellbonus) {
						spellbonuses.PC_Pet_Rampage[SBIndex::PET_RAMPAGE_CHANCE]  = effect_value;
						spellbonuses.PC_Pet_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] = effect_value;
					}

					if (negate_itembonus) {
						itembonuses.PC_Pet_Rampage[SBIndex::PET_RAMPAGE_CHANCE]  = effect_value;
						itembonuses.PC_Pet_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] = effect_value;
					}

					if (negate_aabonus) {
						aabonuses.PC_Pet_Rampage[SBIndex::PET_RAMPAGE_CHANCE]  = effect_value;
						aabonuses.PC_Pet_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] = effect_value;
					}

					break;

				case SpellEffect::PC_Pet_AE_Rampage:
					if (negate_spellbonus) {
						spellbonuses.PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_CHANCE]  = effect_value;
						spellbonuses.PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] = effect_value;
					}

					if (negate_itembonus) {
						itembonuses.PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_CHANCE]  = effect_value;
						itembonuses.PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] = effect_value;
					}

					if (negate_aabonus) {
						aabonuses.PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_CHANCE]  = effect_value;
						aabonuses.PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] = effect_value;
					}

					break;


				case SpellEffect::SkillProcSuccess: {
					for (int e = 0; e < MAX_SKILL_PROCS; e++)
					{
						if (negate_spellbonus) { spellbonuses.SkillProcSuccess[e] = effect_value; }
						if (negate_itembonus) { itembonuses.SkillProcSuccess[e] = effect_value; }
						if (negate_aabonus) { aabonuses.SkillProcSuccess[e] = effect_value; }
					}
					break;
				}

				case SpellEffect::SkillProcAttempt: {
					for (int e = 0; e < MAX_SKILL_PROCS; e++)
					{
						if (negate_spellbonus) { spellbonuses.SkillProc[e] = effect_value; }
						if (negate_itembonus) { itembonuses.SkillProc[e] = effect_value; }
						if (negate_aabonus) { aabonuses.SkillProc[e] = effect_value; }
					}
					break;
				}

				case SpellEffect::Shield_Target:	{
					if (negate_spellbonus) {
						spellbonuses.ShieldTargetSpa[SBIndex::SHIELD_TARGET_MITIGATION_PERCENT] = effect_value;
						spellbonuses.ShieldTargetSpa[SBIndex::SHIELD_TARGET_BUFFSLOT] = effect_value;
					}
					break;
				}
			}
		}
	}
}

void Mob::CalcHeroicBonuses(StatBonuses* newbon)
{
	if (GetHeroicSTR()) {
		SetHeroicStrBonuses(newbon);
	}

	if (GetHeroicSTA()) {
		SetHeroicStaBonuses(newbon);
	}

	if (GetHeroicAGI()) {
		SetHeroicAgiBonuses(newbon);
	}

	if (GetHeroicDEX()) {
		SetHeroicDexBonuses(newbon);
	}

	if (GetHeroicINT()) {
		SetHeroicIntBonuses(newbon);
	}

	if (GetHeroicWIS()) {
		SetHeroicWisBonuses(newbon);
	}
}

void Mob::SetHeroicWisBonuses(StatBonuses* n)
{
	n->heroic_max_mana += IsHeroicWISCasterClass(GetClass()) ? GetHeroicWIS() * RuleR(Character, HeroicWisdomMultiplier) * 10 : 0;
	n->heroic_mana_regen += IsHeroicWISCasterClass(GetClass()) ? GetHeroicWIS() * RuleR(Character, HeroicWisdomMultiplier) / 25 : 0;
	n->HealAmt += GetHeroicWIS() * RuleR(Character, HeroicWisdomIncreaseHealAmtMultiplier);

	if (RuleB(Character, HeroicStatsUseDataBucketsToScale)) {
		n->heroic_max_mana += IsHeroicWISCasterClass(GetClass()) ? GetHeroicWIS() * CheckHeroicBonusesDataBuckets(HeroicBonusBucket::WisMaxMana) * 10 : 0;
		n->heroic_mana_regen += IsHeroicWISCasterClass(GetClass()) ? GetHeroicWIS() * CheckHeroicBonusesDataBuckets(HeroicBonusBucket::WisManaRegen) / 25 : 0;
		n->HealAmt += GetHeroicWIS() * CheckHeroicBonusesDataBuckets(HeroicBonusBucket::WisHealAmt);
	}
}

void Mob::SetHeroicIntBonuses(StatBonuses* n)
{
	n->heroic_max_mana += IsHeroicINTCasterClass(GetClass()) ? GetHeroicINT() * RuleR(Character, HeroicIntelligenceMultiplier) * 10 : 0;
	n->heroic_mana_regen += IsHeroicINTCasterClass(GetClass()) ? GetHeroicINT() * RuleR(Character, HeroicIntelligenceMultiplier) / 25 : 0;
	n->SpellDmg += GetHeroicINT() * RuleR(Character, HeroicIntelligenceIncreaseSpellDmgMultiplier);

	if (RuleB(Character, HeroicStatsUseDataBucketsToScale)) {
		n->heroic_max_mana += IsHeroicINTCasterClass(GetClass()) ? GetHeroicINT() * CheckHeroicBonusesDataBuckets(HeroicBonusBucket::IntMaxMana) * 10 : 0;
		n->heroic_mana_regen += IsHeroicINTCasterClass(GetClass()) ? GetHeroicINT() * CheckHeroicBonusesDataBuckets(HeroicBonusBucket::IntManaRegen) / 25 : 0;
		n->SpellDmg += GetHeroicINT() * CheckHeroicBonusesDataBuckets(HeroicBonusBucket::IntSpellDmg);
	}
}

void Mob::SetHeroicDexBonuses(StatBonuses* n)
{
	n->heroic_dex_ranged_damage += GetHeroicDEX() * RuleR(Character, HeroicDexterityMultiplier) / 10;
	n->heroic_max_end += GetHeroicDEX() * RuleR(Character, HeroicDexterityMultiplier) / 4 * 10.0f;
	n->heroic_end_regen += GetHeroicDEX() * RuleR(Character, HeroicDexterityMultiplier) / 4 / 50;

	if (RuleB(Character, HeroicStatsUseDataBucketsToScale)) {
		n->heroic_dex_ranged_damage += GetHeroicDEX() * CheckHeroicBonusesDataBuckets(HeroicBonusBucket::DexRangedDamage) / 10;
		n->heroic_max_end += GetHeroicDEX() * CheckHeroicBonusesDataBuckets(HeroicBonusBucket::DexMaxEndurance) / 4 * 10.0f;
		n->heroic_end_regen += GetHeroicDEX() * CheckHeroicBonusesDataBuckets(HeroicBonusBucket::DexEnduranceRegen) / 4 / 50;
	}
}

void Mob::SetHeroicAgiBonuses(StatBonuses* n)
{
	n->heroic_agi_avoidance += GetHeroicAGI() * RuleR(Character, HeroicAgilityMultiplier) / 10;
	n->heroic_max_end += GetHeroicAGI() * RuleR(Character, HeroicAgilityMultiplier) / 4 * 10.0f;
	n->heroic_end_regen += GetHeroicAGI() * RuleR(Character, HeroicAgilityMultiplier) / 4 / 50;

	if (RuleB(Character, HeroicStatsUseDataBucketsToScale)) {
		n->heroic_agi_avoidance += GetHeroicAGI() * CheckHeroicBonusesDataBuckets(HeroicBonusBucket::AgiAvoidance) / 10;
		n->heroic_max_end += GetHeroicAGI() * CheckHeroicBonusesDataBuckets(HeroicBonusBucket::AgiMaxEndurance) / 4 * 10.0f;
		n->heroic_end_regen += GetHeroicAGI() * CheckHeroicBonusesDataBuckets(HeroicBonusBucket::AgiEnduranceRegen) / 4 / 50;
	}
}

void Mob::SetHeroicStaBonuses(StatBonuses* n)
{
	n->heroic_max_hp += GetHeroicSTA() * RuleR(Character, HeroicStaminaMultiplier) * 10;
	n->heroic_hp_regen += GetHeroicSTA() * RuleR(Character, HeroicStaminaMultiplier) / 20;
	n->heroic_max_end += GetHeroicSTA() * RuleR(Character, HeroicStaminaMultiplier) / 4 * 10.0f;
	n->heroic_end_regen += GetHeroicSTA() * RuleR(Character, HeroicStaminaMultiplier) / 4 / 50;

	if (RuleB(Character, HeroicStatsUseDataBucketsToScale)) {
		n->heroic_max_hp += GetHeroicSTA() * CheckHeroicBonusesDataBuckets(HeroicBonusBucket::StaMaxHP) * 10;
		n->heroic_hp_regen += GetHeroicSTA() * CheckHeroicBonusesDataBuckets(HeroicBonusBucket::StaHPRegen) / 20;
		n->heroic_max_end += GetHeroicSTA() * CheckHeroicBonusesDataBuckets(HeroicBonusBucket::StaMaxEndurance) / 4 * 10.0f;
		n->heroic_end_regen += GetHeroicSTA() * CheckHeroicBonusesDataBuckets(HeroicBonusBucket::StaEnduranceRegen) / 4 / 50;
	}
}

void Mob::SetHeroicStrBonuses(StatBonuses* n)
{
	n->heroic_str_shield_ac += GetHeroicSTR() * RuleR(Character, HeroicStrengthMultiplier) / 10;
	n->heroic_str_melee_damage += GetHeroicSTR() * RuleR(Character, HeroicStrengthMultiplier) / 10;
	n->heroic_max_end += GetHeroicSTR() * RuleR(Character, HeroicStrengthMultiplier) / 4 * 10.0f;
	n->heroic_end_regen += GetHeroicSTR() * RuleR(Character, HeroicStrengthMultiplier) / 4 / 50;

	if (RuleB(Character, HeroicStatsUseDataBucketsToScale)) {
		n->heroic_str_shield_ac += GetHeroicSTR() * CheckHeroicBonusesDataBuckets(HeroicBonusBucket::StrShieldAC) / 10;
		n->heroic_str_melee_damage += GetHeroicSTR() * CheckHeroicBonusesDataBuckets(HeroicBonusBucket::StrMeleeDamage) / 10;
		n->heroic_max_end += GetHeroicSTR() * CheckHeroicBonusesDataBuckets(HeroicBonusBucket::StrMaxEndurance) / 4 * 10.0f;
		n->heroic_end_regen += GetHeroicSTR() * CheckHeroicBonusesDataBuckets(HeroicBonusBucket::StrEnduranceRegen) / 4 / 50;
	}
}

float Mob::CheckHeroicBonusesDataBuckets(std::string bucket_name)
{
	std::string bucket_value;
	if (!bucket_name.empty()) {
		DataBucketKey k = GetScopedBucketKeys();
		k.key = bucket_name;
		if (IsOfClientBot()) {
			bucket_value = DataBucket::GetData(&database, k).value;
		}

		if (bucket_value.empty() || !Strings::IsNumber(bucket_value)) {
			return 0.00f;
		}
	}

	return Strings::ToFloat(bucket_value);
}
