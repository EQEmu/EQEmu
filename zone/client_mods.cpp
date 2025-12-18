/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

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

#include "client.h"

#include "common/eqemu_logsys.h"
#include "common/rulesys.h"
#include "common/spdat.h"
#include "zone/bot.h"
#include "zone/mob.h"

#include <algorithm>


int32 Client::GetMaxStat() const
{
	if ((RuleI(Character, StatCap)) > 0) {
		return (RuleI(Character, StatCap));
	}
	int level = GetLevel();
	int32 base = 0;
	if (level < 61) {
		base = 255;
	}
	else if (ClientVersion() >= EQ::versions::ClientVersion::SoF) {
		base = 255 + 5 * (level - 60);
	}
	else if (level < 71) {
		base = 255 + 5 * (level - 60);
	}
	else {
		base = 330;
	}
	return base;
}

int32 Client::GetMaxResist() const
{
	int level = GetLevel();
	int32 base = 500;
	if (level > 65) {
		base += ((level - 65) * 5);
	}
	return base;
}

int32 Client::GetMaxSTR() const
{
	return GetMaxStat()
	       + itembonuses.STRCapMod
	       + spellbonuses.STRCapMod
	       + aabonuses.STRCapMod;
}
int32 Client::GetMaxSTA() const
{
	return GetMaxStat()
	       + itembonuses.STACapMod
	       + spellbonuses.STACapMod
	       + aabonuses.STACapMod;
}
int32 Client::GetMaxDEX() const
{
	return GetMaxStat()
	       + itembonuses.DEXCapMod
	       + spellbonuses.DEXCapMod
	       + aabonuses.DEXCapMod;
}
int32 Client::GetMaxAGI() const
{
	return GetMaxStat()
	       + itembonuses.AGICapMod
	       + spellbonuses.AGICapMod
	       + aabonuses.AGICapMod;
}
int32 Client::GetMaxINT() const
{
	return GetMaxStat()
	       + itembonuses.INTCapMod
	       + spellbonuses.INTCapMod
	       + aabonuses.INTCapMod;
}
int32 Client::GetMaxWIS() const
{
	return GetMaxStat()
	       + itembonuses.WISCapMod
	       + spellbonuses.WISCapMod
	       + aabonuses.WISCapMod;
}
int32 Client::GetMaxCHA() const
{
	return GetMaxStat()
	       + itembonuses.CHACapMod
	       + spellbonuses.CHACapMod
	       + aabonuses.CHACapMod;
}
int32 Client::GetMaxMR() const
{
	return GetMaxResist()
	       + itembonuses.MRCapMod
	       + spellbonuses.MRCapMod
	       + aabonuses.MRCapMod;
}
int32 Client::GetMaxPR() const
{
	return GetMaxResist()
	       + itembonuses.PRCapMod
	       + spellbonuses.PRCapMod
	       + aabonuses.PRCapMod;
}
int32 Client::GetMaxDR() const
{
	return GetMaxResist()
	       + itembonuses.DRCapMod
	       + spellbonuses.DRCapMod
	       + aabonuses.DRCapMod;
}
int32 Client::GetMaxCR() const
{
	return GetMaxResist()
	       + itembonuses.CRCapMod
	       + spellbonuses.CRCapMod
	       + aabonuses.CRCapMod;
}
int32 Client::GetMaxFR() const
{
	return GetMaxResist()
	       + itembonuses.FRCapMod
	       + spellbonuses.FRCapMod
	       + aabonuses.FRCapMod;
}
int32 Client::GetMaxCorrup() const
{
	return GetMaxResist()
	       + itembonuses.CorrupCapMod
	       + spellbonuses.CorrupCapMod
	       + aabonuses.CorrupCapMod;
}
int32 Client::LevelRegen()
{
	bool sitting = IsSitting();
	bool feigned = GetFeigned();
	int level = GetLevel();
	bool bonus = GetPlayerRaceBit(GetBaseRace()) & RuleI(Character, BaseHPRegenBonusRaces);
	uint8 multiplier1 = bonus ? 2 : 1;
	int64 hp = 0;
	//these calculations should match up with the info from Monkly Business, which was last updated ~05/2008: http://www.monkly-business.net/index.php?pageid=abilities
	if (level < 51) {
		if (sitting) {
			if (level < 20) {
				hp += 2 * multiplier1;
			}
			else if (level < 50) {
				hp += 3 * multiplier1;
			}
			else {	//level == 50
				hp += 4 * multiplier1;
			}
		}
		else {	//feigned or standing
			hp += 1 * multiplier1;
		}
	}
	//there may be an easier way to calculate this next part, but I don't know what it is
	else {	//level >= 51
		int32 tmp = 0;
		float multiplier2 = 1;
		if (level < 56) {
			tmp = 2;
			if (bonus) {
				multiplier2 = 3;
			}
		}
		else if (level < 60) {
			tmp = 3;
			if (bonus) {
				multiplier2 = 3.34;
			}
		}
		else if (level < 61) {
			tmp = 4;
			if (bonus) {
				multiplier2 = 3;
			}
		}
		else if (level < 63) {
			tmp = 5;
			if (bonus) {
				multiplier2 = 2.8;
			}
		}
		else if (level < 65) {
			tmp = 6;
			if (bonus) {
				multiplier2 = 2.67;
			}
		}
		else {	//level >= 65
			tmp = 7;
			if (bonus) {
				multiplier2 = 2.58;
			}
		}
		hp += int32(float(tmp) * multiplier2);
		if (sitting) {
			hp += 3 * multiplier1;
		}
		else if (feigned) {
			hp += 1 * multiplier1;
		}
	}
	return hp;
}

int64 Client::CalcHPRegen(bool bCombat)
{
	int64 item_regen = itembonuses.HPRegen; // worn spells and +regen, already capped
	item_regen += itembonuses.heroic_hp_regen;

	item_regen += aabonuses.HPRegen;

	int64 base = 0;
	auto base_data = zone->GetBaseData(GetLevel(), GetClass());
	if (base_data.level == GetLevel()) {
		base = static_cast<int>(base_data.hp_regen);
	}

	auto level = GetLevel();
	bool skip_innate = false;

	if (IsSitting()) {
		if (level >= 50) {
			base++;
			if (level >= 65)
				base++;
		}

		if ((Timer::GetCurrentTime() - tmSitting) > 60000) {
			if (!IsAffectedByBuffByGlobalGroup(GlobalGroup::Lich)) {
				auto tic_diff = std::min((Timer::GetCurrentTime() - tmSitting) / 60000, static_cast<uint32>(9));
				if (tic_diff != 1) { // starts at 2 mins
					int tic_bonus = tic_diff * 1.5 * base;
					if (m_pp.InnateSkills[InnateRegen] != InnateDisabled)
						tic_bonus = tic_bonus * 1.2;
					base = tic_bonus;
					skip_innate = true;
				} else if (m_pp.InnateSkills[InnateRegen] == InnateDisabled) { // no innate regen gets first tick
					int tic_bonus = base * 1.5;
					base = tic_bonus;
				}
			}
		}
	}

	if (!skip_innate && m_pp.InnateSkills[InnateRegen] != InnateDisabled) {
		if (level >= 50) {
			++base;
			if (level >= 55) {
				++base;
			}
		}
		base *= 2;
	}

	if (IsStarved())
		base = 0;

	base += GroupLeadershipAAHealthRegeneration();
	// some IsKnockedOut that sets to -1
	base = base * 100.0f * AreaHPRegen * 0.01f + 0.5f;
	// another check for IsClient && !(base + item_regen) && Cur_HP <= 0 do --base; do later

	if (!bCombat && CanFastRegen() && (IsSitting() || CanMedOnHorse())) {
		auto max_hp = GetMaxHP();
		int64 fast_regen = 6 * (max_hp / (zone && zone->newzone_data.fast_regen_hp > 0 ? zone->newzone_data.fast_regen_hp : 180));
		if (base < fast_regen) // weird, but what the client is doing
			base = fast_regen;
	}

	int64 regen = base + item_regen + spellbonuses.HPRegen; // TODO: client does this in buff tick
	return (regen * RuleI(Character, HPRegenMultiplier) / 100);
}

int64 Client::CalcHPRegenCap()
{
	int64 cap = RuleI(Character, ItemHealthRegenCap);
	if (GetLevel() > 60) {
		cap = std::max(cap, static_cast<int64>(GetLevel() - 30)); // if the rule is set greater than normal I guess
	}

	if (GetLevel() > 65) {
		cap += GetLevel() - 65;
	}

	cap += aabonuses.ItemHPRegenCap + spellbonuses.ItemHPRegenCap + itembonuses.ItemHPRegenCap;

	return (cap * RuleI(Character, HPRegenMultiplier) / 100);
}

int64 Client::CalcMaxHP()
{
	float nd = 10000;
	max_hp = (CalcBaseHP() + itembonuses.HP);
	//The AA desc clearly says it only applies to base hp..
	//but the actual effect sent on live causes the client
	//to apply it to (basehp + itemhp).. I will oblige to the client's whims over
	//the aa description

	nd += aabonuses.PercentMaxHPChange + spellbonuses.PercentMaxHPChange + itembonuses.PercentMaxHPChange;	//Natural Durability, Physical Enhancement, Planar Durability
	max_hp = (float)max_hp * (float)nd / (float)10000; //this is to fix the HP-above-495k issue
	max_hp += spellbonuses.FlatMaxHPChange + aabonuses.FlatMaxHPChange + itembonuses.FlatMaxHPChange;

	max_hp += GroupLeadershipAAHealthEnhancement();
	if (current_hp > max_hp) {
		current_hp = max_hp;
	}
	int64 hp_perc_cap = spellbonuses.HPPercCap[SBIndex::RESOURCE_PERCENT_CAP];
	if (hp_perc_cap) {
		int64 curHP_cap = (max_hp * hp_perc_cap) / 100;
		if (current_hp > curHP_cap || (spellbonuses.HPPercCap[SBIndex::RESOURCE_AMOUNT_CAP] && current_hp > spellbonuses.HPPercCap[SBIndex::RESOURCE_AMOUNT_CAP])) {
			current_hp = curHP_cap;
		}
	}

	return max_hp;
}

uint32 Mob::GetClassLevelFactor()
{
	uint32 multiplier = 0;
	uint8  mlevel     = GetLevel();
	switch (GetClass()) {
		case Class::Warrior: {
			if (mlevel < 20) {
				multiplier = 220;
			}
			else if (mlevel < 30) {
				multiplier = 230;
			}
			else if (mlevel < 40) {
				multiplier = 250;
			}
			else if (mlevel < 53) {
				multiplier = 270;
			}
			else if (mlevel < 57) {
				multiplier = 280;
			}
			else if (mlevel < 60) {
				multiplier = 290;
			}
			else if (mlevel < 70) {
				multiplier = 300;
			}
			else {
				multiplier = 311;
			}
			break;
		}
		case Class::Druid:
		case Class::Cleric:
		case Class::Shaman: {
			if (mlevel < 70) {
				multiplier = 150;
			}
			else {
				multiplier = 157;
			}
			break;
		}
		case Class::Berserker:
		case Class::Paladin:
		case Class::ShadowKnight: {
			if (mlevel < 35) {
				multiplier = 210;
			}
			else if (mlevel < 45) {
				multiplier = 220;
			}
			else if (mlevel < 51) {
				multiplier = 230;
			}
			else if (mlevel < 56) {
				multiplier = 240;
			}
			else if (mlevel < 60) {
				multiplier = 250;
			}
			else if (mlevel < 68) {
				multiplier = 260;
			}
			else {
				multiplier = 270;
			}
			break;
		}
		case Class::Monk:
		case Class::Bard:
		case Class::Rogue:
		case Class::Beastlord: {
			if (mlevel < 51) {
				multiplier = 180;
			}
			else if (mlevel < 58) {
				multiplier = 190;
			}
			else if (mlevel < 70) {
				multiplier = 200;
			}
			else {
				multiplier = 210;
			}
			break;
		}
		case Class::Ranger: {
			if (mlevel < 58) {
				multiplier = 200;
			}
			else if (mlevel < 70) {
				multiplier = 210;
			}
			else {
				multiplier = 220;
			}
			break;
		}
		case Class::Magician:
		case Class::Wizard:
		case Class::Necromancer:
		case Class::Enchanter: {
			if (mlevel < 70) {
				multiplier = 120;
			}
			else {
				multiplier = 127;
			}
			break;
		}
		default: {
			if (mlevel < 35) {
				multiplier = 210;
			}
			else if (mlevel < 45) {
				multiplier = 220;
			}
			else if (mlevel < 51) {
				multiplier = 230;
			}
			else if (mlevel < 56) {
				multiplier = 240;
			}
			else if (mlevel < 60) {
				multiplier = 250;
			}
			else {
				multiplier = 260;
			}
			break;
		}
	}

	return multiplier;
}

int64 Client::CalcBaseHP()
{
	if (ClientVersion() >= EQ::versions::ClientVersion::SoF && RuleB(Character, SoDClientUseSoDHPManaEnd)) {
		int stats = GetSTA();
		if (stats > 255) {
			stats = (stats - 255) / 2;
			stats += 255;
		}
		base_hp = 5;
		auto base_data = zone->GetBaseData(GetLevel(), GetClass());
		if (base_data.level == GetLevel()) {
			base_hp += base_data.hp + (base_data.hp_fac * stats);
			base_hp += itembonuses.heroic_max_hp;
		}
	}
	else {
		uint32 Post255;
		uint32 lm = GetClassLevelFactor();
		if ((GetSTA() - 255) / 2 > 0) {
			Post255 = (GetSTA() - 255) / 2;
		}
		else {
			Post255 = 0;
		}
		base_hp = (5) + (GetLevel() * lm / 10) + (((GetSTA() - Post255) * GetLevel() * lm / 3000)) + ((Post255 * GetLevel()) * lm / 6000);
	}
	return base_hp;
}

// This should return the combined AC of all the items the player is wearing.
int32 Client::GetRawItemAC()
{
	int32 Total = 0;
	// this skips MainAmmo..add an '=' conditional if that slot is required (original behavior)
	for (int16 slot_id = EQ::invslot::BONUS_BEGIN; slot_id <= EQ::invslot::BONUS_STAT_END; slot_id++) {
		const EQ::ItemInstance* inst = m_inv[slot_id];
		if (inst && inst->IsClassCommon()) {
			Total += inst->GetItem()->AC;
		}
	}
	return Total;
}

int64 Client::CalcMaxMana()
{
	if (IsIntelligenceCasterClass() || IsWisdomCasterClass()) {
		max_mana = (
			CalcBaseMana() +
			itembonuses.Mana +
			spellbonuses.Mana +
			aabonuses.Mana +
			GroupLeadershipAAManaEnhancement()
		);
	} else {
		max_mana = 0;
	}

	if (max_mana < 0) {
		max_mana = 0;
	}

	if (current_mana > max_mana) {
		current_mana = max_mana;
	}

	int mana_perc_cap = spellbonuses.ManaPercCap[SBIndex::RESOURCE_PERCENT_CAP];
	if (mana_perc_cap) {
		int curMana_cap = (max_mana * mana_perc_cap) / 100;
		if (current_mana > curMana_cap || (spellbonuses.ManaPercCap[SBIndex::RESOURCE_AMOUNT_CAP] && current_mana > spellbonuses.ManaPercCap[SBIndex::RESOURCE_AMOUNT_CAP])) {
			current_mana = curMana_cap;
		}
	}

	LogSpells("for [{}] returning [{}]", GetName(), max_mana);
	return max_mana;
}

int64 Client::CalcBaseMana()
{
	int   ConvertedWisInt = 0;
	int   MindLesserFactor, MindFactor;
	int   WisInt          = 0;
	int64 base_mana       = 0;
	int   wisint_mana     = 0;
	int64 max_m           = 0;

	if (IsIntelligenceCasterClass()) {
		WisInt = GetINT();

		if (ClientVersion() >= EQ::versions::ClientVersion::SoF && RuleB(Character, SoDClientUseSoDHPManaEnd)) {
			ConvertedWisInt = WisInt;

			int over200 = WisInt;
			if (WisInt > 100) {
				if (WisInt > 200) {
					over200 = (WisInt - 200) / -2 + WisInt;
				}
				ConvertedWisInt = (3 * over200 - 300) / 2 + over200;
			}

			auto base_data = zone->GetBaseData(GetLevel(), GetClass());
			if (base_data.level == GetLevel()) {
				max_m = base_data.mana + (ConvertedWisInt * base_data.mana_fac) + itembonuses.heroic_max_mana;
			}
		} else {
			if (((WisInt - 199) / 2) > 0) {
				MindLesserFactor = (WisInt - 199) / 2;
			} else {
				MindLesserFactor = 0;
			}

			MindFactor = WisInt - MindLesserFactor;

			if (WisInt > 100) {
				max_m = (((5 * (MindFactor + 20)) / 2) * 3 * GetLevel() / 40);
			} else {
				max_m = (((5 * (MindFactor + 200)) / 2) * 3 * GetLevel() / 100);
			}
		}
	} else if (IsWisdomCasterClass()) {
		WisInt = GetWIS();

		if (ClientVersion() >= EQ::versions::ClientVersion::SoF && RuleB(Character, SoDClientUseSoDHPManaEnd)) {
			ConvertedWisInt = WisInt;

			int over200 = WisInt;
			if (WisInt > 100) {
				if (WisInt > 200) {
					over200 = (WisInt - 200) / -2 + WisInt;
				}
				ConvertedWisInt = (3 * over200 - 300) / 2 + over200;
			}

			auto base_data = zone->GetBaseData(GetLevel(), GetClass());
			if (base_data.level == GetLevel()) {
				max_m = base_data.mana + (ConvertedWisInt * base_data.mana_fac) + itembonuses.heroic_max_mana;
			}
		} else {
			if (((WisInt - 199) / 2) > 0) {
				MindLesserFactor = (WisInt - 199) / 2;
			} else {
				MindLesserFactor = 0;
			}

			MindFactor = WisInt - MindLesserFactor;

			if (WisInt > 100) {
				max_m = (((5 * (MindFactor + 20)) / 2) * 3 * GetLevel() / 40);
			} else {
				max_m = (((5 * (MindFactor + 200)) / 2) * 3 * GetLevel() / 100);
			}
		}
	} else {
		max_m = 0;
	}

	return max_m;
}

int64 Client::CalcBaseManaRegen()
{
	uint8 clevel = GetLevel();
	int64 regen = 0;
	if (IsSitting() || (GetHorseId() != 0)) {
		if (HasSkill(EQ::skills::SkillMeditate)) {
			regen = (((GetSkill(EQ::skills::SkillMeditate) / 10) + (clevel - (clevel / 4))) / 4) + 4;
		}
		else {
			regen = 2;
		}
	}
	else {
		regen = 2;
	}
	return regen;
}

int64 Client::CalcManaRegen(bool bCombat)
{
	int64 regen = 0;
	auto level = GetLevel();
	// so the new formulas break down with older skill caps where you don't have the skill until 4 or 8
	// so for servers that want to use the old skill progression they can set this rule so they
	// will get at least 1 for standing and 2 for sitting.
	bool old = RuleB(Character, OldMinMana);
	if (!IsStarved()) {
		// client does some base regen for shrouds here
		if (IsSitting() || CanMedOnHorse()) {
			// kind of weird to do it here w/e
			// client does some base medding regen for shrouds here
			if (GetClass() != Class::Bard) {
				auto skill = GetSkill(EQ::skills::SkillMeditate);
				if (skill > 0) {
					regen++;
					if (skill > 1)
						regen++;
					if (skill >= 15)
						regen += skill / 15;
				}
			}
			if (old)
				regen = std::max(regen, static_cast<int64>(2));
		} else if (old) {
			regen = std::max(regen, static_cast<int64>(1));
		}
	}

	if (level > 61) {
		regen++;
		if (level > 63)
			regen++;
	}

	regen += aabonuses.ManaRegen;
	// add in + 1 bonus for SpellEffect::CompleteHeal, but we don't do anything for it yet?

	int item_bonus = itembonuses.ManaRegen; // this is capped already
	item_bonus += itembonuses.heroic_mana_regen;
	regen += item_bonus;

	if (level <= 70 && regen > 65)
		regen = 65;

	regen = regen * 100.0f * AreaManaRegen * 0.01f + 0.5f;

	if (!bCombat && CanFastRegen() && (IsSitting() || CanMedOnHorse())) {
		auto max_mana = GetMaxMana();
		int fast_regen = 6 * (max_mana / zone->newzone_data.fast_regen_mana);
		if (regen < fast_regen) // weird, but what the client is doing
			regen = fast_regen;
	}

	regen += spellbonuses.ManaRegen; // TODO: live does this in buff tick
	return (regen * RuleI(Character, ManaRegenMultiplier) / 100);
}

int64 Client::CalcManaRegenCap()
{
	int64 cap = RuleI(Character, ItemManaRegenCap) + aabonuses.ItemManaRegenCap + itembonuses.ItemManaRegenCap + spellbonuses.ItemManaRegenCap;
	return (cap * RuleI(Character, ManaRegenMultiplier) / 100);
}

uint32 Client::CalcCurrentWeight()
{
	const EQ::ItemData* TempItem = nullptr;
	EQ::ItemInstance* ins = nullptr;
	uint32 Total = 0;
	int x;
	for (x = EQ::invslot::POSSESSIONS_BEGIN; x <= EQ::invslot::POSSESSIONS_END; x++) {
		TempItem = 0;
		ins = GetInv().GetItem(x);
		if (ins) {
			TempItem = ins->GetItem();
		}
		if (TempItem) {
			Total += TempItem->Weight;
		}
	}
	for (x = EQ::invbag::GENERAL_BAGS_BEGIN; x <= EQ::invbag::CURSOR_BAG_END; x++) {
		int TmpWeight = 0;
		TempItem = 0;
		ins = GetInv().GetItem(x);
		if (ins) {
			TempItem = ins->GetItem();
		}
		if (TempItem) {
			TmpWeight = TempItem->Weight;
		}
		if (TmpWeight > 0) {
			// this code indicates that weight redux bags can only be in the first general inventory slot to be effective...
			// is this correct? or can we scan for the highest weight redux and use that? (need client verifications)
			int bagslot = EQ::invslot::slotGeneral1;
			int reduction = 0;
			for (int m = EQ::invbag::GENERAL_BAGS_BEGIN + EQ::invbag::SLOT_COUNT; m <= EQ::invbag::CURSOR_BAG_END; m += EQ::invbag::SLOT_COUNT) {
				if (x >= m) {
					bagslot += 1;
				}
			}
			EQ::ItemInstance* baginst = GetInv().GetItem(bagslot);
			if (baginst && baginst->GetItem() && baginst->IsClassBag()) {
				reduction = baginst->GetItem()->BagWR;
			}
			if (reduction > 0) {
				TmpWeight -= TmpWeight * reduction / 100;
			}
			Total += TmpWeight;
		}
	}
	//TODO: coin weight reduction (from purses, etc), since client already calculates it
	/*  From the Wiki http://www.eqemulator.net/wiki/wikka.php?wakka=EQEmuDBSchemaitems under bagwr (thanks Trevius):
	    Interestingly, you can also have bags that reduce coin weight. However, in order to set bags to reduce coin weight, you MUST set the Item ID somewhere between 17201 and 17230. This is hard coded into the client.
	    The client is set to have certain coin weight reduction on a per Item ID basis within this range. The best way to create an new item to reduce coin weight is to examine existing bags in this range.
	    Search for the words "coin purse" with the #finditem command in game and the Bag WR setting on those bags is the amount they will reduce coin weight. It is easiest to overwrite one of those bags if you wish to create one with the
	    same weight reduction amount for coins. You can use other Item IDs in this range for setting coin weight reduction, but by using an existing item, at least you will know the amount the client will reduce it by before you create it.
	    This is the ONLY instance I have seen where the client is hard coded to particular Item IDs to set a certain property for an item. It is very odd.
	*/
	// SoD+ client has no weight for coin
	if (EQ::behavior::StaticLookup(EQ::versions::ConvertClientVersionToMobVersion(ClientVersion()))->CoinHasWeight) {
		Total += (m_pp.platinum + m_pp.gold + m_pp.silver + m_pp.copper) / 4;
	}
	float Packrat = (float)spellbonuses.Packrat + (float)aabonuses.Packrat + (float)itembonuses.Packrat;
	if (Packrat > 0) {
		Total = (uint32)((float)Total * (1.0f - ((Packrat * 1.0f) / 100.0f)));    //AndMetal: 1% per level, up to 5% (calculated from Titanium client). verified thru client that it reduces coin weight by the same %
	}
	//without casting to float & back to uint32, this didn't work right
	return Total;
}

int32 Client::CalcAlcoholPhysicalEffect()
{
	if (GetIntoxication() <= 55) {
		return 0;
	}
	return (GetIntoxication() - 40) / 16;
}

int32 Client::CalcSTR()
{
	int32 val = m_pp.STR + itembonuses.STR + spellbonuses.STR + CalcAlcoholPhysicalEffect();
	int32 mod = aabonuses.STR;
	STR = val + mod;
	if (STR < 1) {
		STR = 1;
	}
	int m = GetMaxSTR();
	if (STR > m) {
		STR = m;
	}
	return (STR);
}

int32 Client::CalcSTA()
{
	int32 val = m_pp.STA + itembonuses.STA + spellbonuses.STA + CalcAlcoholPhysicalEffect();
	int32 mod = aabonuses.STA;
	STA = val + mod;
	if (STA < 1) {
		STA = 1;
	}
	int m = GetMaxSTA();
	if (STA > m) {
		STA = m;
	}
	return (STA);
}

int32 Client::CalcAGI()
{
	int32 val = m_pp.AGI + itembonuses.AGI + spellbonuses.AGI - CalcAlcoholPhysicalEffect();
	int32 mod = aabonuses.AGI;
	int32 str = GetSTR();
	//Encumbered penalty
	if (weight > (str * 10)) {
		//AGI is halved when we double our weight, zeroed (defaults to 1) when we triple it. this includes AGI from AAs
		float total_agi = float(val + mod);
		float str_float = float(str);
		AGI = (int32)(((-total_agi) / (str_float * 2)) * (((float)weight / 10) - str_float) + total_agi);	//casting to an int assumes this will be floor'd. without using floats & casting to int16, the calculation doesn't work right
	}
	else {
		AGI = val + mod;
	}
	if (AGI < 1) {
		AGI = 1;
	}
	int m = GetMaxAGI();
	if (AGI > m) {
		AGI = m;
	}
	return (AGI);
}

int32 Client::CalcDEX()
{
	int32 val = m_pp.DEX + itembonuses.DEX + spellbonuses.DEX - CalcAlcoholPhysicalEffect();
	int32 mod = aabonuses.DEX;
	DEX = val + mod;
	if (DEX < 1) {
		DEX = 1;
	}
	int m = GetMaxDEX();
	if (DEX > m) {
		DEX = m;
	}
	return (DEX);
}

int32 Client::CalcINT()
{
	int32 val = m_pp.INT + itembonuses.INT + spellbonuses.INT;
	int32 mod = aabonuses.INT;
	INT = val + mod;
	if (GetIntoxication()) {
		int32 AlcINT = INT - (int32)((float)GetIntoxication() / 200.0f * (float)INT) - 1;
		if ((AlcINT < (int)(0.2 * INT))) {
			INT = (int)(0.2f * (float)INT);
		}
		else {
			INT = AlcINT;
		}
	}
	if (INT < 1) {
		INT = 1;
	}
	int m = GetMaxINT();
	if (INT > m) {
		INT = m;
	}
	return (INT);
}

int32 Client::CalcWIS()
{
	int32 val = m_pp.WIS + itembonuses.WIS + spellbonuses.WIS;
	int32 mod = aabonuses.WIS;
	WIS = val + mod;
	if (GetIntoxication()) {
		int32 AlcWIS = WIS - (int32)((float)GetIntoxication() / 200.0f * (float)WIS) - 1;
		if ((AlcWIS < (int)(0.2 * WIS))) {
			WIS = (int)(0.2f * (float)WIS);
		}
		else {
			WIS = AlcWIS;
		}
	}
	if (WIS < 1) {
		WIS = 1;
	}
	int m = GetMaxWIS();
	if (WIS > m) {
		WIS = m;
	}
	return (WIS);
}

int32 Client::CalcCHA()
{
	int32 val = m_pp.CHA + itembonuses.CHA + spellbonuses.CHA;
	int32 mod = aabonuses.CHA;
	CHA = val + mod;
	if (CHA < 1) {
		CHA = 1;
	}
	int m = GetMaxCHA();
	if (CHA > m) {
		CHA = m;
	}
	return (CHA);
}

int Client::CalcHaste()
{
	/*  Tests: (based on results in newer char window)
	    68 v1 + 46 item + 25 over + 35 inhib = 204%
	    46 item + 5 v2 + 25 over + 35 inhib = 65%
	    68 v1 + 46 item + 5 v2 + 25 over + 35 inhib = 209%
	    75% slow + 35 inhib = 25%
	    35 inhib = 65%
	    75% slow = 25%
	    Conclusions:
	    the bigger effect in slow v. inhib wins
	    slow negates all other hastes
	    inhib will only negate all other hastes if you don't have v1 (ex. VQ)
	*/
	// slow beats all! Besides a better inhibit
	if (spellbonuses.haste < 0) {
		if (-spellbonuses.haste <= spellbonuses.inhibitmelee) {
			Haste = 100 - spellbonuses.inhibitmelee;
		}
		else {
			Haste = 100 + spellbonuses.haste;
		}
		return Haste;
	}
	// No haste and inhibit, kills all other hastes
	if (spellbonuses.haste == 0 && spellbonuses.inhibitmelee) {
		Haste = 100 - spellbonuses.inhibitmelee;
		return Haste;
	}
	int h = 0;
	int cap = 0;
	int level = GetLevel();
	// we know we have a haste spell and not slowed, no extra inhibit melee checks needed
	if (spellbonuses.haste) {
		h += spellbonuses.haste - spellbonuses.inhibitmelee;
	}
	if (spellbonuses.hastetype2 && level > 49) { // type 2 is capped at 10% and only available to 50+
		h += spellbonuses.hastetype2 > 10 ? 10 : spellbonuses.hastetype2;
	}
	// 26+ no cap, 1-25 10
	if (level > 25 || RuleB(Character, IgnoreLevelBasedHasteCaps)) { // 26+
		h += itembonuses.haste;
	}
	else {   // 1-25
		h += itembonuses.haste > 10 ? 10 : itembonuses.haste;
	}
	// 60+ 100, 51-59 85, 1-50 level+25
	if (level > 59 || RuleB(Character, IgnoreLevelBasedHasteCaps)) { // 60+
		cap = RuleI(Character, HasteCap);
	}
	else if (level > 50) {  // 51-59
		cap = 85;
	}
	else {   // 1-50
		cap = level + 25;
	}
	if (h > cap) {
		h = cap;
	}
	// 51+ 25 (despite there being higher spells...), 1-50 10
	if (level > 50 || RuleB(Character, IgnoreLevelBasedHasteCaps)) { // 51+
		cap = RuleI(Character, Hastev3Cap);
		if (spellbonuses.hastetype3 > cap) {
			h += cap;
		} else {
			h += spellbonuses.hastetype3;
		}
	}
	else {   // 1-50
		h += spellbonuses.hastetype3 > 10 ? 10 : spellbonuses.hastetype3;
	}
	h += extra_haste;	//GM granted haste.
	Haste = 100 + h;
	return Haste;
}

//The AA multipliers are set to be 5, but were 2 on WR
//The resistant discipline which I think should be here is implemented
//in Mob::ResistSpell
int32	Client::CalcMR()
{
	//racial bases
	switch (GetBaseRace()) {
		case Race::Human:
			MR = 25;
			break;
		case Race::Barbarian:
			MR = 25;
			break;
		case Race::Erudite:
			MR = 30;
			break;
		case Race::WoodElf:
			MR = 25;
			break;
		case Race::HighElf:
			MR = 25;
			break;
		case Race::DarkElf:
			MR = 25;
			break;
		case Race::HalfElf:
			MR = 25;
			break;
		case Race::Dwarf:
			MR = 30;
			break;
		case Race::Troll:
			MR = 25;
			break;
		case Race::Ogre:
			MR = 25;
			break;
		case Race::Halfling:
			MR = 25;
			break;
		case Race::Gnome:
			MR = 25;
			break;
		case Race::Iksar:
			MR = 25;
			break;
		case Race::VahShir:
			MR = 25;
			break;
		case Race::Froglok2:
			MR = 30;
			break;
		case Race::Drakkin:
		{
			MR = 25;
			if (GetDrakkinHeritage() == 2)
				MR += 10;
			else if (GetDrakkinHeritage() == 5)
				MR += 2;
			break;
		}
		default:
			MR = 20;
	}
	MR += itembonuses.MR + spellbonuses.MR + aabonuses.MR;
	if (GetClass() == Class::Warrior || GetClass() == Class::Berserker) {
		MR += GetLevel() / 2;
	}
	if (MR < 1) {
		MR = 1;
	}
	if (MR > GetMaxMR()) {
		MR = GetMaxMR();
	}
	return (MR);
}

int32	Client::CalcFR()
{
	//racial bases
	switch (GetBaseRace()) {
		case Race::Human:
			FR = 25;
			break;
		case Race::Barbarian:
			FR = 25;
			break;
		case Race::Erudite:
			FR = 25;
			break;
		case Race::WoodElf:
			FR = 25;
			break;
		case Race::HighElf:
			FR = 25;
			break;
		case Race::DarkElf:
			FR = 25;
			break;
		case Race::HalfElf:
			FR = 25;
			break;
		case Race::Dwarf:
			FR = 25;
			break;
		case Race::Troll:
			FR = 5;
			break;
		case Race::Ogre:
			FR = 25;
			break;
		case Race::Halfling:
			FR = 25;
			break;
		case Race::Gnome:
			FR = 25;
			break;
		case Race::Iksar:
			FR = 30;
			break;
		case Race::VahShir:
			FR = 25;
			break;
		case Race::Froglok2:
			FR = 25;
			break;
		case Race::Drakkin:
		{
			FR = 25;
			if (GetDrakkinHeritage() == 0)
				FR += 10;
			else if (GetDrakkinHeritage() == 5)
				FR += 2;
			break;
		}
		default:
			FR = 20;
	}
	int c = GetClass();
	if (c == Class::Ranger) {
		FR += 4;
		int l = GetLevel();
		if (l > 49) {
			FR += l - 49;
		}
	}
	if (c == Class::Monk) {
		FR += 8;
		int l = GetLevel();
		if (l > 49) {
			FR += l - 49;
		}
	}
	FR += itembonuses.FR + spellbonuses.FR + aabonuses.FR;
	if (FR < 1) {
		FR = 1;
	}
	if (FR > GetMaxFR()) {
		FR = GetMaxFR();
	}
	return (FR);
}

int32	Client::CalcDR()
{
	//racial bases
	switch (GetBaseRace()) {
		case Race::Human:
			DR = 15;
			break;
		case Race::Barbarian:
			DR = 15;
			break;
		case Race::Erudite:
			DR = 10;
			break;
		case Race::WoodElf:
			DR = 15;
			break;
		case Race::HighElf:
			DR = 15;
			break;
		case Race::DarkElf:
			DR = 15;
			break;
		case Race::HalfElf:
			DR = 15;
			break;
		case Race::Dwarf:
			DR = 15;
			break;
		case Race::Troll:
			DR = 15;
			break;
		case Race::Ogre:
			DR = 15;
			break;
		case Race::Halfling:
			DR = 20;
			break;
		case Race::Gnome:
			DR = 15;
			break;
		case Race::Iksar:
			DR = 15;
			break;
		case Race::VahShir:
			DR = 15;
			break;
		case Race::Froglok2:
			DR = 15;
			break;
		case Race::Drakkin:
		{
			DR = 15;
			if (GetDrakkinHeritage() == 1)
				DR += 10;
			else if (GetDrakkinHeritage() == 5)
				DR += 2;
			break;
		}
		default:
			DR = 15;
	}
	int c = GetClass();
	// the monk one is part of base resist
	if (c == Class::Monk) {
		int l = GetLevel();
		if (l > 50)
			DR += l - 50;
	}
	if (c == Class::Paladin) {
		DR += 8;
		int l = GetLevel();
		if (l > 49) {
			DR += l - 49;
		}
	}
	else if (c == Class::ShadowKnight || c == Class::Beastlord) {
		DR += 4;
		int l = GetLevel();
		if (l > 49) {
			DR += l - 49;
		}
	}
	DR += itembonuses.DR + spellbonuses.DR + aabonuses.DR;
	if (DR < 1) {
		DR = 1;
	}
	if (DR > GetMaxDR()) {
		DR = GetMaxDR();
	}
	return (DR);
}

int32	Client::CalcPR()
{
	//racial bases
	switch (GetBaseRace()) {
		case Race::Human:
			PR = 15;
			break;
		case Race::Barbarian:
			PR = 15;
			break;
		case Race::Erudite:
			PR = 15;
			break;
		case Race::WoodElf:
			PR = 15;
			break;
		case Race::HighElf:
			PR = 15;
			break;
		case Race::DarkElf:
			PR = 15;
			break;
		case Race::HalfElf:
			PR = 15;
			break;
		case Race::Dwarf:
			PR = 20;
			break;
		case Race::Troll:
			PR = 15;
			break;
		case Race::Ogre:
			PR = 15;
			break;
		case Race::Halfling:
			PR = 20;
			break;
		case Race::Gnome:
			PR = 15;
			break;
		case Race::Iksar:
			PR = 15;
			break;
		case Race::VahShir:
			PR = 15;
			break;
		case Race::Froglok2:
			PR = 30;
			break;
		case Race::Drakkin:
		{
			PR = 15;
			if (GetDrakkinHeritage() == 3)
				PR += 10;
			else if (GetDrakkinHeritage() == 5)
				PR += 2;
			break;
		}
		default:
			PR = 15;
	}
	int c = GetClass();
	// this monk bonus is part of the base
	if (c == Class::Monk) {
		int l = GetLevel();
		if (l > 50)
			PR += l - 50;
	}
	if (c == Class::Rogue) {
		PR += 8;
		int l = GetLevel();
		if (l > 49) {
			PR += l - 49;
		}
	}
	else if (c == Class::ShadowKnight) {
		PR += 4;
		int l = GetLevel();
		if (l > 49) {
			PR += l - 49;
		}
	}
	PR += itembonuses.PR + spellbonuses.PR + aabonuses.PR;
	if (PR < 1) {
		PR = 1;
	}
	if (PR > GetMaxPR()) {
		PR = GetMaxPR();
	}
	return (PR);
}

int32	Client::CalcCR()
{
	//racial bases
	switch (GetBaseRace()) {
		case Race::Human:
			CR = 25;
			break;
		case Race::Barbarian:
			CR = 35;
			break;
		case Race::Erudite:
			CR = 25;
			break;
		case Race::WoodElf:
			CR = 25;
			break;
		case Race::HighElf:
			CR = 25;
			break;
		case Race::DarkElf:
			CR = 25;
			break;
		case Race::HalfElf:
			CR = 25;
			break;
		case Race::Dwarf:
			CR = 25;
			break;
		case Race::Troll:
			CR = 25;
			break;
		case Race::Ogre:
			CR = 25;
			break;
		case Race::Halfling:
			CR = 25;
			break;
		case Race::Gnome:
			CR = 25;
			break;
		case Race::Iksar:
			CR = 15;
			break;
		case Race::VahShir:
			CR = 25;
			break;
		case Race::Froglok2:
			CR = 25;
			break;
		case Race::Drakkin:
		{
			CR = 25;
			if (GetDrakkinHeritage() == 4)
				CR += 10;
			else if (GetDrakkinHeritage() == 5)
				CR += 2;
			break;
		}
		default:
			CR = 25;
	}
	int c = GetClass();
	if (c == Class::Ranger || c == Class::Beastlord) {
		CR += 4;
		int l = GetLevel();
		if (l > 49) {
			CR += l - 49;
		}
	}
	CR += itembonuses.CR + spellbonuses.CR + aabonuses.CR;
	if (CR < 1) {
		CR = 1;
	}
	if (CR > GetMaxCR()) {
		CR = GetMaxCR();
	}
	return (CR);
}

int32	Client::CalcCorrup()
{
	Corrup = GetBaseCorrup() + itembonuses.Corrup + spellbonuses.Corrup + aabonuses.Corrup;
	if (Corrup > GetMaxCorrup()) {
		Corrup = GetMaxCorrup();
	}
	return (Corrup);
}

int32 Client::CalcATK()
{
	ATK = itembonuses.ATK + spellbonuses.ATK + aabonuses.ATK + GroupLeadershipAAOffenseEnhancement();
	return (ATK);
}

uint32 Mob::GetInstrumentMod(uint16 spell_id)
{
	if (GetClass() != Class::Bard || spells[spell_id].is_discipline || spell_id == SPELL_AMPLIFICATION) {
		//Other classes can get a base effects mod using SPA 413
		if (HasBaseEffectFocus()) {
			return (10 + (GetFocusEffect(focusFcBaseEffects, spell_id) / 10));//TODO: change action->instrument mod to float to support < 10% focus values
		}
		return 10;
	}

	//AA's click effects that use instrument/singing skills don't apply modifiers (Confirmed on live 11/24/21 ~Kayen)
	if (casting_spell_aa_id) {
		return 10;
	}

	uint32 effectmod = 10;
	int effectmodcap = 0;
	if (RuleB(Character, UseSpellFileSongCap)) {
		effectmodcap = spells[spell_id].song_cap / 10;
		if (effectmodcap) {
			effectmodcap += 10; //Actual calculated cap is 100 greater than songcap value.
		}
	}
	else {
		effectmodcap = RuleI(Character, BaseInstrumentSoftCap);
	}
	// this should never use spell modifiers...
	// if a spell grants better modifers, they are copied into the item mods
	// because the spells are supposed to act just like having the intrument.
	// item mods are in 10ths of percent increases
	// clickies (Symphony of Battle) that have a song skill don't get AA bonus for some reason
	// but clickies that are songs (selo's on Composers Greaves) do get AA mod as well

	/*Mechanics: updated 10/19/21 ~Kayen
		Bard Spell Effects

		Mod uses the highest bonus from either of these for each instrument
		SPA 179 SpellEffect::AllInstrumentMod is used for instrument spellbonus.______Mod. This applies to ALL instrument mods (Puretones Discipline)
		SPA 260 SpellEffect::AddSingingMod is used for instrument spellbonus.______Mod. This applies to indiviual instrument mods. (Instrument mastery AA)
			-Example usage: From AA a value of 4 = 40%

		SPA 118 SpellEffect::Amplification is a stackable singing mod, on live it exists as both spell and AA bonus (stackable)
			- Live Behavior: Amplifcation can be modified by singing mods and amplification itself, thus on the second cast of Amplification you will recieve
			  the mod from the first cast, this continues until you reach the song mod cap.

		SPA 261 SpellEffect::SongModCap raises song focus cap (No longer used on live)
		SPA 270 SpellEffect::BardSongRange increase range of beneficial bard songs (Sionachie's Crescendo)

		SPA 413 SpellEffect::FcBaseEffects focus effect that replaced item instrument mods

		Issues 10-15-21:
		Bonuses are not applied, unless song is stopped and restarted due to pulse keeping it continues. -> Need to recode songs to recast when duration ends.

		Formula Live Bards:
		mod = (10 + (aabonus.____Mod [SPA 260 AA Instrument Mastery]) + (SpellEffect::FcBaseEffect[SPA 413])/10 + (spellbonus.______Mod [SPA 179 Puretone Disc]) + (Amplication [SPA 118])/10

		TODO: Spell Table Fields that need to be implemented
		Field 225	//float base_effects_focus_slope;  // -- BASE_EFFECTS_FOCUS_SLOPE
		Field 226	//float base_effects_focus_offset; // -- BASE_EFFECTS_FOCUS_OFFSET (35161	Ruaabri's Reckless Renewal -120)
		Based on description possibly works as a way to quickly balance instrument mods to a song.
		Using a standard slope formula: y = mx + b
		modified_base_value = (base_effects_focus_slope x effectmod)(base_value) + (base_effects_focus_offset)
		Will need to confirm on live before implementing.
	*/

	switch (spells[spell_id].skill) {
	case EQ::skills::SkillPercussionInstruments:
		if (itembonuses.percussionMod == 0 && spellbonuses.percussionMod == 0)
			effectmod = 10;
		else if (GetSkill(EQ::skills::SkillPercussionInstruments) == 0)
			effectmod = 10;
		else if (itembonuses.percussionMod > spellbonuses.percussionMod)
			effectmod = itembonuses.percussionMod;
		else
			effectmod = spellbonuses.percussionMod;
		if (IsBardSong(spell_id))
			effectmod += aabonuses.percussionMod;
		break;
	case EQ::skills::SkillStringedInstruments:
		if (itembonuses.stringedMod == 0 && spellbonuses.stringedMod == 0)
			effectmod = 10;
		else if (GetSkill(EQ::skills::SkillStringedInstruments) == 0)
			effectmod = 10;
		else if (itembonuses.stringedMod > spellbonuses.stringedMod)
			effectmod = itembonuses.stringedMod;
		else
			effectmod = spellbonuses.stringedMod;
		if (IsBardSong(spell_id))
			effectmod += aabonuses.stringedMod;
		break;
	case EQ::skills::SkillWindInstruments:
		if (itembonuses.windMod == 0 && spellbonuses.windMod == 0)
			effectmod = 10;
		else if (GetSkill(EQ::skills::SkillWindInstruments) == 0)
			effectmod = 10;
		else if (itembonuses.windMod > spellbonuses.windMod)
			effectmod = itembonuses.windMod;
		else
			effectmod = spellbonuses.windMod;
		if (IsBardSong(spell_id))
			effectmod += aabonuses.windMod;
		break;
	case EQ::skills::SkillBrassInstruments:
		if (itembonuses.brassMod == 0 && spellbonuses.brassMod == 0)
			effectmod = 10;
		else if (GetSkill(EQ::skills::SkillBrassInstruments) == 0)
			effectmod = 10;
		else if (itembonuses.brassMod > spellbonuses.brassMod)
			effectmod = itembonuses.brassMod;
		else
			effectmod = spellbonuses.brassMod;
		if (IsBardSong(spell_id))
			effectmod += aabonuses.brassMod;
		break;
	case EQ::skills::SkillSinging:
		if (itembonuses.singingMod == 0 && spellbonuses.singingMod == 0)
			effectmod = 10;
		else if (itembonuses.singingMod > spellbonuses.singingMod)
			effectmod = itembonuses.singingMod;
		else
			effectmod = spellbonuses.singingMod;
		if (IsBardSong(spell_id))
			effectmod += aabonuses.singingMod + (spellbonuses.Amplification + itembonuses.Amplification + aabonuses.Amplification); //SPA 118 SpellEffect::Amplification
		break;
	default:
		effectmod = 10;
		return effectmod;
	}

	if (HasBaseEffectFocus()) {
		effectmod += (GetFocusEffect(focusFcBaseEffects, spell_id) / 10);
	}

	if (effectmod < 10) {
		effectmod = 10;
	}

	if (effectmodcap) {

		effectmodcap += aabonuses.songModCap + spellbonuses.songModCap + itembonuses.songModCap; //SPA 261 SpellEffect::SongModCap (not used on live)

		//Incase a negative modifier is used.
		if (effectmodcap <= 0) {
			effectmodcap = 10;
		}

		if (effectmod > effectmodcap) { // if the cap is calculated to be 0 using new rules, no cap.
			effectmod = effectmodcap;
		}
	}

	LogSpells("Name [{}] spell [{}] mod [{}] modcap [{}]\n", GetName(), spell_id, effectmod, effectmodcap);

	return effectmod;
}

void Client::CalcMaxEndurance()
{
	max_end = CalcBaseEndurance() + spellbonuses.Endurance + itembonuses.Endurance + aabonuses.Endurance;
	if (max_end < 0) {
		max_end = 0;
	}
	if (current_endurance > max_end) {
		current_endurance = max_end;
	}
	int end_perc_cap = spellbonuses.EndPercCap[SBIndex::RESOURCE_PERCENT_CAP];
	if (end_perc_cap) {
		int curEnd_cap = (max_end * end_perc_cap) / 100;
		if (current_endurance > curEnd_cap || (spellbonuses.EndPercCap[SBIndex::RESOURCE_AMOUNT_CAP] && current_endurance > spellbonuses.EndPercCap[SBIndex::RESOURCE_AMOUNT_CAP])) {
			current_endurance = curEnd_cap;
		}
	}
}

int64 Client::CalcBaseEndurance()
{
	int64 base_end = 0;
	if (ClientVersion() >= EQ::versions::ClientVersion::SoF && RuleB(Character, SoDClientUseSoDHPManaEnd)) {
		double stats = (GetSTR() + GetSTA() + GetDEX() + GetAGI()) / 4.0f;

		if (stats > 201.0f) {
			stats = 1.25f * (stats - 201.0f) + 352.5f;
		}
		else if (stats > 100.0f) {
			stats = 2.5f * (stats - 100.0f) + 100.0f;
		}
		auto base_data = zone->GetBaseData(GetLevel(), GetClass());
		if (base_data.level == GetLevel()) {
			base_end = base_data.end + itembonuses.heroic_max_end + (base_data.end_fac * static_cast<int>(stats));
		}
	}
	else {
		int Stats = GetSTR() + GetSTA() + GetDEX() + GetAGI();
		int LevelBase = GetLevel() * 15;
		int at_most_800 = Stats;
		if (at_most_800 > 800) {
			at_most_800 = 800;
		}
		int Bonus400to800 = 0;
		int HalfBonus400to800 = 0;
		int Bonus800plus = 0;
		int HalfBonus800plus = 0;
		int BonusUpto800 = int( at_most_800 / 4 ) ;
		if (Stats > 400) {
			Bonus400to800 = int( (at_most_800 - 400) / 4 );
			HalfBonus400to800 = int( std::max( ( at_most_800 - 400 ), 0 ) / 8 );
			if (Stats > 800) {
				Bonus800plus = int( (Stats - 800) / 8 ) * 2;
				HalfBonus800plus = int( (Stats - 800) / 16 );
			}
		}
		int64 bonus_sum = BonusUpto800 + Bonus400to800 + HalfBonus400to800 + Bonus800plus + HalfBonus800plus;
		base_end = LevelBase;
		//take all of the sums from above, then multiply by level*0.075
		base_end += ( bonus_sum * 3 * GetLevel() ) / 40;
	}
	return base_end;
}

int64 Client::CalcEnduranceRegen(bool bCombat)
{
	int64 base = 0;
	if (!IsStarved()) {
		auto base_data = zone->GetBaseData(GetLevel(), GetClass());
		if (base_data.level == GetLevel()) {
			base = static_cast<int>(base_data.end_regen);
			if (!auto_attack && base > 0)
				base += base / 2;
		}
	}

	// so when we are mounted, our local client SpeedRun is always 0, so this is always false, but the packets we process it to our own shit :P
	bool is_running = runmode && animation != 0 && GetHorseId() == 0; // TODO: animation is really what MQ2 calls SpeedRun

	int weight_limit = GetSTR();
	auto level = GetLevel();
	if (GetClass() == Class::Monk) {
		if (level > 99)
			weight_limit = 58;
		else if (level > 94)
			weight_limit = 57;
		else if (level > 89)
			weight_limit = 56;
		else if (level > 84)
			weight_limit = 55;
		else if (level > 79)
			weight_limit = 54;
		else if (level > 64)
			weight_limit = 53;
		else if (level > 63)
			weight_limit = 50;
		else if (level > 61)
			weight_limit = 47;
		else if (level > 59)
			weight_limit = 45;
		else if (level > 54)
			weight_limit = 40;
		else if (level > 50)
			weight_limit = 38;
		else if (level > 44)
			weight_limit = 36;
		else if (level > 29)
			weight_limit = 34;
		else if (level > 14)
			weight_limit = 32;
		else
			weight_limit = 30;
	}

	bool encumbered = (CalcCurrentWeight() / 10) >= weight_limit;

	if (is_running)
		base += level / -15;

	if (encumbered)
		base += level / -15;

	auto item_bonus = itembonuses.EnduranceRegen + itembonuses.heroic_end_regen; // this is capped already
	base += item_bonus;

	base = base * AreaEndRegen + 0.5f;

	auto aa_regen = aabonuses.EnduranceRegen;

	int64 regen = base;
	if (!bCombat && CanFastRegen() && (IsSitting() || CanMedOnHorse())) {
		auto max_end = GetMaxEndurance();
		int fast_regen = 6 * (max_end / zone->newzone_data.fast_regen_endurance);
		if (aa_regen < fast_regen) // weird, but what the client is doing
			aa_regen = fast_regen;
	}

	regen += aa_regen;
	regen += spellbonuses.EnduranceRegen; // TODO: client does this in buff tick

	return (regen * RuleI(Character, EnduranceRegenMultiplier) / 100);
}

int64 Client::CalcEnduranceRegenCap()
{
	int64 cap = RuleI(Character, ItemEnduranceRegenCap) + aabonuses.ItemEnduranceRegenCap + itembonuses.ItemEnduranceRegenCap + spellbonuses.ItemEnduranceRegenCap;
	return (cap * RuleI(Character, EnduranceRegenMultiplier) / 100);
}

int32 Client::CalcItemATKCap()
{
	int cap = RuleI(Character, ItemATKCap) + itembonuses.ItemATKCap + spellbonuses.ItemATKCap + aabonuses.ItemATKCap;
	return cap;
}

int Client::GetRawACNoShield(int &shield_ac) const
{
	int ac = itembonuses.AC + spellbonuses.AC + aabonuses.AC;
	shield_ac = 0;
	const EQ::ItemInstance *inst = m_inv.GetItem(EQ::invslot::slotSecondary);
	if (inst) {
		if (inst->GetItem()->ItemType == EQ::item::ItemTypeShield) {
			ac -= inst->GetItem()->AC;
			shield_ac = inst->GetItem()->AC;
			for (uint8 i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++) {
				if (inst->GetAugment(i)) {
					ac -= inst->GetAugment(i)->GetItem()->AC;
					shield_ac += inst->GetAugment(i)->GetItem()->AC;
				}
			}
		}
	}
	return ac;
}
