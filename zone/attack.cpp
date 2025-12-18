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

#include "common/data_verification.h"
#include "common/eq_constants.h"
#include "common/eq_packet_structs.h"
#include "common/events/player_event_logs.h"
#include "common/misc_functions.h"
#include "common/rulesys.h"
#include "common/spdat.h"
#include "common/strings.h"
#include "zone/bot.h"
#include "zone/fastmath.h"
#include "zone/lua_parser.h"
#include "zone/mob.h"
#include "zone/npc.h"
#include "zone/queryserv.h"
#include "zone/quest_parser_collection.h"
#include "zone/string_ids.h"
#include "zone/water_map.h"
#include "zone/worldserver.h"
#include "zone/zone.h"

extern QueryServ* QServ;
extern WorldServer worldserver;
extern FastMath g_Math;
extern EntityList entity_list;
extern Zone* zone;

//SYNC WITH: tune.cpp, mob.h TuneAttackAnimation
EQ::skills::SkillType Mob::AttackAnimation(int Hand, const EQ::ItemInstance* weapon, EQ::skills::SkillType skillinuse)
{
	// Determine animation
	int type = 0;
	if (weapon && weapon->IsClassCommon()) {
		const EQ::ItemData* item = weapon->GetItem();

		Log(Logs::Detail, Logs::Attack, "Weapon skill : %i", item->ItemType);

		switch (item->ItemType) {
		case EQ::item::ItemType1HSlash: // 1H Slashing
			skillinuse = EQ::skills::Skill1HSlashing;
			type = anim1HWeapon;
			break;
		case EQ::item::ItemType2HSlash: // 2H Slashing
			skillinuse = EQ::skills::Skill2HSlashing;
			type = anim2HSlashing;
			break;
		case EQ::item::ItemType1HPiercing: // Piercing
			skillinuse = EQ::skills::Skill1HPiercing;
			type = anim1HPiercing;
			break;
		case EQ::item::ItemType1HBlunt: // 1H Blunt
			skillinuse = EQ::skills::Skill1HBlunt;
			type = anim1HWeapon;
			break;
		case EQ::item::ItemType2HBlunt: // 2H Blunt
			skillinuse = EQ::skills::Skill2HBlunt;
			type = RuleB(Combat, Classic2HBAnimation) ? anim2HWeapon : anim2HSlashing;
			break;
		case EQ::item::ItemType2HPiercing: // 2H Piercing
			if (IsClient() && CastToClient()->ClientVersion() < EQ::versions::ClientVersion::RoF2)
				skillinuse = EQ::skills::Skill1HPiercing;
			else
				skillinuse = EQ::skills::Skill2HPiercing;
			type = anim2HWeapon;
			break;
		case EQ::item::ItemTypeMartial:
			skillinuse = EQ::skills::SkillHandtoHand;
			type = animHand2Hand;
			break;
		default:
			skillinuse = EQ::skills::SkillHandtoHand;
			type = animHand2Hand;
			break;
		}// switch
	}
	else if (IsNPC()) {
		switch (skillinuse) {
			case EQ::skills::Skill1HBlunt: // 1H Blunt
			case EQ::skills::Skill1HSlashing: // 1H Slashing
				type = anim1HWeapon;
				break;
			case EQ::skills::Skill2HBlunt: // 2H Blunt
			case EQ::skills::Skill2HSlashing: // 2H Slashing
				type = anim2HSlashing;
				break;
			case EQ::skills::Skill1HPiercing: // Piercing
				type = anim1HPiercing;
				break;
			case EQ::skills::Skill2HPiercing: // 2H Piercing
				type = anim2HWeapon;
				break;
			case EQ::skills::SkillHandtoHand:
				type = animHand2Hand;
				break;
			default:
				type = animHand2Hand;
				break;
		}// switch
	}
	else {
		skillinuse = EQ::skills::SkillHandtoHand;
		type = animHand2Hand;
	}

	// If we're attacking with the secondary hand, play the dual wield anim
	if (Hand == EQ::invslot::slotSecondary) {// DW anim
		type = animDualWield;

		//allow animation chance to fire to be similar to your dw chance
		if (GetDualWieldingSameDelayWeapons() == 2) {
			SetDualWieldingSameDelayWeapons(3);
		}
	}

	//If both weapons have same delay this allows a chance for DW animation
	if (GetDualWieldingSameDelayWeapons() && Hand == EQ::invslot::slotPrimary) {

		if (GetDualWieldingSameDelayWeapons() == 3 && zone->random.Roll(50)) {
			type = animDualWield;
			SetDualWieldingSameDelayWeapons(2);//Don't roll again till you do another dw attack.
		}
		SetDualWieldingSameDelayWeapons(2);//Ensures first attack is always primary.
	}

	DoAnim(type, 0, false);

	return skillinuse;
}
//SYNC WITH: tune.cpp, mob.h Tunecompute_tohit
int Mob::compute_tohit(EQ::skills::SkillType skillinuse)
{
	int tohit = GetSkill(EQ::skills::SkillOffense) + 7;
	tohit += GetSkill(skillinuse);

	if (IsNPC()) {
		if (RuleB(Combat, UseMobStaticOffenseSkill)) {
			tohit = GetMobFixedWeaponSkill() + GetMobFixedOffenseSkill() + 7;
		}
		tohit += CastToNPC()->GetAccuracyRating();
	} else if (IsClient()) {
		double reduction = CastToClient()->GetIntoxication() / 2.0;
		if (reduction > 20.0) {
			reduction = std::min((110 - reduction) / 100.0, 1.0);
			tohit = reduction * static_cast<double>(tohit);
		} else if (IsBerserk()) {
			tohit += (GetLevel() * 2) / 5;
		}
	}
	return std::max(tohit, 1);
}

// return -1 in cases that always hit
//SYNC WITH: tune.cpp, mob.h TuneGetTotalToHit
int Mob::GetTotalToHit(EQ::skills::SkillType skill, int chance_mod)
{
	if (chance_mod >= 10000) // override for stuff like SpellEffect::SkillAttack
		return -1;

	// calculate attacker's accuracy
	auto accuracy = compute_tohit(skill) + 10; // add 10 in case the NPC's stats are fucked
	if (chance_mod > 0) // multiplier
		accuracy *= chance_mod;

	// Torven parsed an apparent constant of 1.2 somewhere in here * 6 / 5 looks eqmathy to me!
	// new test clients have 121 / 100
	accuracy = (accuracy * 121) / 100;

	// unsure on the stacking order of these effects, rather hard to parse
	// item mod2 accuracy isn't applied to range? Theory crafting and parses back it up I guess
	// mod2 accuracy -- flat bonus
	if (skill != EQ::skills::SkillArchery && skill != EQ::skills::SkillThrowing) {
		accuracy += itembonuses.HitChance;
	} else {
		// Applying a scale factor as sources suggest Accuracy should reduce number of missing by 0.1% per point, so 150 = 15% reduction in misses.
		// Based on my calculator 150 Accuracy was reducing misses by too much (closer to 20%)
		// NOTE: This doesn't mean if you have a 30% miss chance you now miss 15%.  It means if you have a 30% miss chance you now have a 30% * (100% - 15%) = 30% * 85% = 25.5% miss chance
		// Using same scale factor for Avoidance and Accuracy since they impact the formula about the same.
		accuracy += itembonuses.HitChance * RuleI(Combat, PCAccuracyAvoidanceMod2Scale) / 100;
	}

	//518 Increase ATK accuracy by percentage, stackable
	auto atkhit_bonus = itembonuses.Attack_Accuracy_Max_Percent + aabonuses.Attack_Accuracy_Max_Percent + spellbonuses.Attack_Accuracy_Max_Percent;
	if (atkhit_bonus)
		accuracy += round(static_cast<double>(accuracy) * static_cast<double>(atkhit_bonus) * 0.0001);

	// 216 Melee Accuracy Amt aka SpellEffect::Accuracy -- flat bonus
	accuracy += itembonuses.Accuracy[EQ::skills::HIGHEST_SKILL + 1] +
		aabonuses.Accuracy[EQ::skills::HIGHEST_SKILL + 1] +
		spellbonuses.Accuracy[EQ::skills::HIGHEST_SKILL + 1] +
		itembonuses.Accuracy[skill] +
		aabonuses.Accuracy[skill] +
		spellbonuses.Accuracy[skill];

	// auto hit discs (and looks like there are some autohit AAs)
	if (spellbonuses.HitChanceEffect[skill] >= 10000 || aabonuses.HitChanceEffect[skill] >= 10000)
		return -1;

	if (spellbonuses.HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1] >= 10000)
		return -1;

	// 184 Accuracy % aka SpellEffect::HitChance -- percentage increase
	auto hit_bonus = itembonuses.HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1] +
		aabonuses.HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1] +
		spellbonuses.HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1] +
		itembonuses.HitChanceEffect[skill] +
		aabonuses.HitChanceEffect[skill] +
		spellbonuses.HitChanceEffect[skill];

	if (skill == EQ::skills::SkillArchery) {
		hit_bonus += spellbonuses.increase_archery + aabonuses.increase_archery + itembonuses.increase_archery;
		hit_bonus -= hit_bonus * RuleR(Combat, ArcheryHitPenalty);
	}

	accuracy = (accuracy * (100 + hit_bonus)) / 100;

	// TODO: April 2003 added an archery/throwing PVP accuracy penalty while moving, should be in here some where,
	// but PVP is less important so I haven't tried parsing it at all

	// There is also 110 Ranger Archery Accuracy % which should probably be in here some where, but it's not in any spells/aas
	// Name implies it's a percentage increase, if one wishes to implement, do it like the hit_bonus above but limited to ranger archery

	// There is also 183 UNUSED - Skill Increase Chance which devs say isn't used at all in code, but some spells reference it
	// I do not recommend implementing this once since there are spells that use it which would make this not live-like with default spell files
	return accuracy;
}

// based on dev quotes
// the AGI bonus has actually drastically changed from classic
//SYNC WITH: tune.cpp, mob.h Tunecompute_defense
int Mob::compute_defense()
{
	int defense = GetSkill(EQ::skills::SkillDefense) * 400 / 225;

	// In new code, AGI becomes a large contributor to avoidance at low levels, since AGI isn't capped by Level but Defense is
	// A scale factor is implemented for PCs to reduce the effect of AGI at low levels.  This isn't applied to NPCs since they can be
	// easily controlled via the Database.
	if (RuleB(Combat, LegacyComputeDefense)) {
		int agi_scale_factor = 1000;

		if (IsOfClientBot()) {
			agi_scale_factor = std::min(1000, static_cast<int>(GetLevel()) * 1000 / 70); // Scales Agi Contribution for PC's Level, max Contribution at Level 70
		}

		defense += agi_scale_factor * (800 * (GetAGI() - 40)) / 3600 / 1000;

		if (IsOfClientBot()) {
			defense += GetHeroicAGI() / 10;
		}

		defense += itembonuses.AvoidMeleeChance * RuleI(Combat, PCAccuracyAvoidanceMod2Scale) / 100; // item mod2
	} else {
		defense += (8000 * (GetAGI() - 40)) / 36000;

		if (IsOfClientBot()) {
			defense += itembonuses.heroic_agi_avoidance;
		}

		defense += itembonuses.AvoidMeleeChance; // item mod2
	}


	//516 SpellEffect::AC_Mitigation_Max_Percent
	auto ac_bonus = itembonuses.AC_Mitigation_Max_Percent + aabonuses.AC_Mitigation_Max_Percent + spellbonuses.AC_Mitigation_Max_Percent;
	if (ac_bonus) {
		defense += round(static_cast<double>(defense) * static_cast<double>(ac_bonus) * 0.0001);
	}

	if (IsNPC()) {
		defense += CastToNPC()->GetAvoidanceRating();
	}

	if (IsClient()) {
		double reduction = CastToClient()->GetIntoxication() / 2.0;
		if (reduction > 20.0) {
			reduction = std::min((110 - reduction) / 100.0, 1.0);
			defense = reduction * static_cast<double>(defense);
		}
	}

	return std::max(1, defense);
}

// return -1 in cases that always miss
// SYNC WITH : tune.cpp, mob.h TuneGetTotalDefense()
int Mob::GetTotalDefense()
{
	auto avoidance = compute_defense() + 10; // add 10 in case the NPC's stats are fucked
	auto evasion_bonus = spellbonuses.AvoidMeleeChanceEffect; // we check this first since it has a special case
	if (evasion_bonus >= 10000)
		return -1;

	// 515 SpellEffect::AC_Avoidance_Max_Percent
	auto ac_aviodance_bonus = itembonuses.AC_Avoidance_Max_Percent + aabonuses.AC_Avoidance_Max_Percent + spellbonuses.AC_Avoidance_Max_Percent;
	if (ac_aviodance_bonus)
		avoidance += round(static_cast<double>(avoidance) * static_cast<double>(ac_aviodance_bonus) * 0.0001);

	// 172 Evasion aka SpellEffect::AvoidMeleeChance
	evasion_bonus += itembonuses.AvoidMeleeChanceEffect + aabonuses.AvoidMeleeChanceEffect; // item bonus here isn't mod2 avoidance

	// 215 Pet Avoidance % aka SpellEffect::PetAvoidance
	evasion_bonus += GetPetAvoidanceBonusFromOwner();

	// Evasion is a percentage bonus according to AA descriptions
	if (evasion_bonus)
		avoidance = (avoidance * (100 + evasion_bonus)) / 100;

	return avoidance;
}

// called when a mob is attacked, does the checks to see if it's a hit
// and does other mitigation checks. 'this' is the mob being attacked.
// SYNC WITH : tune.cpp, mob.h TuneCheckHitChance()
bool Mob::CheckHitChance(Mob* other, DamageHitInfo &hit)
{
#ifdef LUA_EQEMU
	bool lua_ret = false;
	bool ignoreDefault = false;
	lua_ret = LuaParser::Instance()->CheckHitChance(this, other, hit, ignoreDefault);

	if(ignoreDefault) {
		return lua_ret;
	}
#endif

	Mob *attacker = other;
	Mob *defender = this;
	Log(Logs::Detail, Logs::Attack, "CheckHitChance(%s) attacked by %s", defender->GetName(), attacker->GetName());

	if (defender->IsOfClientBotMerc() && defender->IsSitting()) {
		return true;
	}

	auto avoidance = defender->GetTotalDefense();
	if (avoidance == -1) // some sort of auto avoid disc
		return false;

	auto accuracy = hit.tohit;
	if (accuracy == -1)
		return true;

	// so now we roll!
	// relevant dev quote:
	// Then your chance to simply avoid the attack is checked (defender's avoidance roll beat the attacker's accuracy roll.)
	int tohit_roll = zone->random.Roll0(accuracy);
	int avoid_roll = zone->random.Roll0(avoidance);
	Log(Logs::Detail, Logs::Attack, "CheckHitChance accuracy(%d => %d) avoidance(%d => %d)", accuracy, tohit_roll, avoidance, avoid_roll);

	// tie breaker? Don't want to be biased any one way
	if (tohit_roll == avoid_roll)
		return zone->random.Roll(50);
	return tohit_roll > avoid_roll;
}

bool Mob::AvoidDamage(Mob *other, DamageHitInfo &hit)
{
#ifdef LUA_EQEMU
	bool lua_ret = false;
	bool ignoreDefault = false;
	lua_ret = LuaParser::Instance()->AvoidDamage(this, other, hit, ignoreDefault);

	if (ignoreDefault) {
		return lua_ret;
	}
#endif

	/* called when a mob is attacked, does the checks to see if it's a hit
	* and does other mitigation checks. 'this' is the mob being attacked.
	*
	* special return values:
	* -1 - block
	* -2 - parry
	* -3 - riposte
	* -4 - dodge
	*
	*/

	/* Order according to current (SoF+?) dev quotes:
	* https://web.archive.org/web/20250816014133/https://forums.everquest.com/index.php?threads/test-update-06-10-15.223510/page-2#post-3261772
	* https://web.archive.org/web/20250816014133/https://forums.everquest.com/index.php?threads/test-update-06-10-15.223510/page-2#post-3268227
	* Riposte 50, hDEX, must have weapon/fists, doesn't work on archery/throwing
	* Block 25, hDEX, works on archery/throwing, behind block done here if back to attacker base1 is chance
	* Parry 45, hDEX, doesn't work on throwing/archery, must be facing target
	* Dodge 45, hAGI, works on archery/throwing, monks can dodge attacks from behind
	* Shield Block, rand base1
	* Staff Block, rand base1
	*    regular strike through
	*    avoiding the attack (CheckHitChance)
	* As soon as one succeeds, none of the rest are checked
	*
	* Formula (all int math)
	* (posted for parry, assume rest at the same)
	* Chance = (((SKILL + 100) + [((SKILL+100) * SPA(175).Base1) / 100]) / 45) + [(hDex / 25) - min([hStrikethrough, hDex / 25])].
	* hStrikethrough is a mob stat that was added to counter the bonuses of heroic stats
	* Number rolled against 100, if the chance is greater than 100 it happens 100% of time
	*
	* Things with 10k accuracy mods can be avoided with these skills qq
	*/
	Mob *attacker = other;
	Mob *defender = this;

	bool InFront = !attacker->BehindMob(this, attacker->GetX(), attacker->GetY());

	/*
	This special ability adds a negative modifer to the defenders riposte/block/parry/chance
	therefore reducing the defenders chance to successfully avoid the melee attack. Works in tandem with Heroic Strikethrough. This may
	ultimately end up being more useful as fields in npc_types.
	*/

	int counter_all     = 0;
	int counter_riposte = 0;
	int counter_block   = 0;
	int counter_parry   = 0;
	int counter_dodge   = 0;

	if (attacker->GetSpecialAbility(SpecialAbility::CounterAvoidDamage)) {
		counter_all     = attacker->GetSpecialAbilityParam(SpecialAbility::CounterAvoidDamage, 0);
		counter_riposte = attacker->GetSpecialAbilityParam(SpecialAbility::CounterAvoidDamage, 1);
		counter_block   = attacker->GetSpecialAbilityParam(SpecialAbility::CounterAvoidDamage, 2);
		counter_parry   = attacker->GetSpecialAbilityParam(SpecialAbility::CounterAvoidDamage, 3);
		counter_dodge   = attacker->GetSpecialAbilityParam(SpecialAbility::CounterAvoidDamage, 4);
	}

	int modify_all     = 0;
	int modify_riposte = 0;
	int modify_block   = 0;
	int modify_parry   = 0;
	int modify_dodge   = 0;

	if (GetSpecialAbility(SpecialAbility::ModifyAvoidDamage)) {
		modify_all     = GetSpecialAbilityParam(SpecialAbility::ModifyAvoidDamage, 0);
		modify_riposte = GetSpecialAbilityParam(SpecialAbility::ModifyAvoidDamage, 1);
		modify_block   = GetSpecialAbilityParam(SpecialAbility::ModifyAvoidDamage, 2);
		modify_parry   = GetSpecialAbilityParam(SpecialAbility::ModifyAvoidDamage, 3);
		modify_dodge   = GetSpecialAbilityParam(SpecialAbility::ModifyAvoidDamage, 4);
	}

	/* Heroic Strikethrough Implementation per Dev Quotes (2018):
	* https://web.archive.org/web/20250816014810/https://forums.everquest.com/index.php?threads/illusions-benefit-neza-10-dodge.246757/#post-3622670
	* Step1 = HeroicStrikethrough(NPC)
	* Step2 = HeroicAgility / 25
	* Step3 = MIN( Step1, Step2 )
	* Step4 = DodgeSkill + 100
	* Step5 = Step4 + ( DodgeSkill * DodgeSPA ) / 100
	* Step6 = Step5 / 45
	* DodgeChance = Step6 + ( Step2 - Step3 )

	* Formula (all int math)
	* (posted for parry, dodge appears to be the same as confirmed per Dev, assuming Riposte/Block are the same)
	* Chance = (((SKILL + 100) + [((SKILL+100) * SPA(175).Base1) / 100]) / 45) + [(hDex / 25) - min([hStrikethrough, hDex / 25])].
	If an NPC's Heroic Strikethrough is higher than your character's Heroic Agility bonus, you are disqualified from the "bonus" you would have gotten at the end.
	*/

	int hstrikethrough = attacker->GetHeroicStrikethrough();

	// riposte -- it may seem crazy, but if the attacker has SPA 173 on them, they are immune to Ripo
	bool ImmuneRipo = false;
	if (!RuleB(Combat, UseLiveRiposteMechanics)) {
		ImmuneRipo = attacker->aabonuses.RiposteChance || attacker->spellbonuses.RiposteChance || attacker->itembonuses.RiposteChance || attacker->IsEnraged();
	}
	/*
		Live Riposte Mechanics (~Kayen updated 1/22)
		-Ripostes can not trigger another riposte. (Ie. Riposte from defender can't then trigger the attacker to riposte)
		-Ripostes can not be 'avoided', only hit or miss.
		-Attacker with SPA 173 is not immune to riposte. The defender can riposte against the attackers melee hits.

		Legacy Riposte Mechanics
		-Ripostes can trigger another riposte
		-Attacker with SPA 173 is immune to riposte
		-Attacker that is enraged is immune to riposte
	*/

	// Need to check if we have something in MainHand to actually attack with (or fists)
	if (hit.hand != EQ::invslot::slotRange && (CanThisClassRiposte() || IsEnraged()) && InFront && !ImmuneRipo) {
		if (IsEnraged()) {
			hit.damage_done = DMG_RIPOSTED;
			LogCombat("I am enraged, riposting frontal attack");
			return true;
		}
		if (IsClient())
			CastToClient()->CheckIncreaseSkill(EQ::skills::SkillRiposte, other, -10);
		// check auto discs ... I guess aa/items too :P
		if (spellbonuses.RiposteChance == 10000 || aabonuses.RiposteChance == 10000 || itembonuses.RiposteChance == 10000) {
			hit.damage_done = DMG_RIPOSTED;
			return true;
		}
		int chance = GetSkill(EQ::skills::SkillRiposte) + 100;
		chance += (chance * (aabonuses.RiposteChance + spellbonuses.RiposteChance + itembonuses.RiposteChance)) / 100;
		chance /= 50;
		chance += (itembonuses.HeroicDEX / 25) - std::min(hstrikethrough,(itembonuses.HeroicDEX / 25)); // "Heroic Strikethrough" subtracted here to counter HeroicDEX
		if (counter_riposte || counter_all) {
			float counter = (counter_riposte + counter_all) / 100.0f;
			chance -= chance * counter;
		}
		if (modify_riposte || modify_all) {
			float npc_modifier = (modify_riposte + modify_all) / 100.0f;
			chance += chance * npc_modifier;
		}
		// AA Slippery Attacks
		if (hit.hand == EQ::invslot::slotSecondary) {
			int slip = aabonuses.OffhandRiposteFail + itembonuses.OffhandRiposteFail + spellbonuses.OffhandRiposteFail;
			chance += chance * slip / 100;
		}
		if (chance > 0 && zone->random.Roll(chance)) { // could be <0 from offhand stuff
			hit.damage_done = DMG_RIPOSTED;
			return true;
		}
	}

	// block
	bool bBlockFromRear = false;

	// a successful roll on this does not mean a successful block is forthcoming. only that a chance to block
	// from a direction other than the rear is granted.

	int BlockBehindChance = aabonuses.BlockBehind + spellbonuses.BlockBehind + itembonuses.BlockBehind;

	if (BlockBehindChance && zone->random.Roll(BlockBehindChance))
		bBlockFromRear = true;

	if (CanThisClassBlock() && (InFront || bBlockFromRear)) {
		if (IsClient())
			CastToClient()->CheckIncreaseSkill(EQ::skills::SkillBlock, other, -10);
		// check auto discs ... I guess aa/items too :P
		if (spellbonuses.IncreaseBlockChance == 10000 || aabonuses.IncreaseBlockChance == 10000 ||
			itembonuses.IncreaseBlockChance == 10000) {
			hit.damage_done = DMG_BLOCKED;
			return true;
		}
		int chance = GetSkill(EQ::skills::SkillBlock) + 100;
		chance += (chance * (aabonuses.IncreaseBlockChance + spellbonuses.IncreaseBlockChance + itembonuses.IncreaseBlockChance)) / 100;
		chance /= 25;
		chance += (itembonuses.HeroicDEX / 25) - std::min(hstrikethrough,(itembonuses.HeroicDEX / 25)); // "Heroic Strikethrough" subtracted here to counter HeroicDEX
		if (counter_block || counter_all) {
			float counter = (counter_block + counter_all) / 100.0f;
			chance -= chance * counter;
		}
		if (modify_block || modify_all) {
			float npc_modifier = (modify_block + modify_all) / 100.0f;
			chance += chance * npc_modifier;
		}
		if (zone->random.Roll(chance)) {
			hit.damage_done = DMG_BLOCKED;
			return true;
		}
	}

	// parry
	if (CanThisClassParry() && InFront && hit.hand != EQ::invslot::slotRange) {
		if (IsClient())
			CastToClient()->CheckIncreaseSkill(EQ::skills::SkillParry, other, -10);
		// check auto discs ... I guess aa/items too :P
		if (spellbonuses.ParryChance == 10000 || aabonuses.ParryChance == 10000 || itembonuses.ParryChance == 10000) {
			hit.damage_done = DMG_PARRIED;
			return true;
		}
		int chance = GetSkill(EQ::skills::SkillParry) + 100;
		chance += (chance * (aabonuses.ParryChance + spellbonuses.ParryChance + itembonuses.ParryChance)) / 100;
		chance /= 45;
		chance += (itembonuses.HeroicDEX / 25) - std::min(hstrikethrough,(itembonuses.HeroicDEX / 25)); // "Heroic Strikethrough" subtracted here to counter HeroicDEX
		if (counter_parry || counter_all) {
			float counter = (counter_parry + counter_all) / 100.0f;
			chance -= chance * counter;
		}
		if (modify_parry || modify_all) {
			float npc_modifier = (modify_parry + modify_all) / 100.0f;
			chance += chance * npc_modifier;
		}
		if (zone->random.Roll(chance)) {
			hit.damage_done = DMG_PARRIED;
			return true;
		}
	}

	// dodge
	if (CanThisClassDodge() && (InFront || GetClass() == Class::Monk)) {
		if (IsClient())
			CastToClient()->CheckIncreaseSkill(EQ::skills::SkillDodge, other, -10);
		// check auto discs ... I guess aa/items too :P
		if (spellbonuses.DodgeChance == 10000 || aabonuses.DodgeChance == 10000 || itembonuses.DodgeChance == 10000) {
			hit.damage_done = DMG_DODGED;
			return true;
		}
		int chance = GetSkill(EQ::skills::SkillDodge) + 100;
		chance += (chance * (aabonuses.DodgeChance + spellbonuses.DodgeChance + itembonuses.DodgeChance)) / 100;
		chance /= 45;
		chance += (itembonuses.HeroicAGI / 25) - std::min(hstrikethrough,(itembonuses.HeroicAGI / 25)); // "Heroic Strikethrough" subtracted here to counter HeroicAGI
		if (counter_dodge || counter_all) {
			float counter = (counter_dodge + counter_all) / 100.0f;
			chance -= chance * counter;
		}
		if (modify_dodge || modify_all) {
			float npc_modifier = (modify_dodge + modify_all) / 100.0f;
			chance += chance * npc_modifier;
		}
		if (zone->random.Roll(chance)) {
			hit.damage_done = DMG_DODGED;
			return true;
		}
	}

	// Try Shield Block OR TwoHandBluntBlockCheck
	if (HasShieldEquipped() && (aabonuses.ShieldBlock || spellbonuses.ShieldBlock || itembonuses.ShieldBlock) && (InFront || bBlockFromRear)) {
		int chance = aabonuses.ShieldBlock + spellbonuses.ShieldBlock + itembonuses.ShieldBlock;
		if (counter_block || counter_all) {
			float counter = (counter_block + counter_all) / 100.0f;
			chance -= chance * counter;
		}
		if (zone->random.Roll(chance)) {
			hit.damage_done = DMG_BLOCKED;
			return true;
		}
	}

	if (HasTwoHandBluntEquipped() && (aabonuses.TwoHandBluntBlock || spellbonuses.TwoHandBluntBlock || itembonuses.TwoHandBluntBlock) && (InFront || bBlockFromRear)) {
		int chance = aabonuses.TwoHandBluntBlock + itembonuses.TwoHandBluntBlock + spellbonuses.TwoHandBluntBlock;
		if (counter_block || counter_all) {
			float counter = (counter_block + counter_all) / 100.0f;
			chance -= chance * counter;
		}
		if (zone->random.Roll(chance)) {
			hit.damage_done = DMG_BLOCKED;
			return true;
		}
	}

	return false;
}

int Mob::GetACSoftcap()
{
	// from test server Resources/ACMitigation.txt
	static int war_softcaps[] = {
		312, 314, 316, 318, 320, 322, 324, 326, 328, 330, 332, 334, 336, 338, 340, 342, 344, 346, 348, 350, 352,
		354, 356, 358, 360, 362, 364, 366, 368, 370, 372, 374, 376, 378, 380, 382, 384, 386, 388, 390, 392, 394,
		396, 398, 400, 402, 404, 406, 408, 410, 412, 414, 416, 418, 420, 422, 424, 426, 428, 430, 432, 434, 436,
		438, 440, 442, 444, 446, 448, 450, 452, 454, 456, 458, 460, 462, 464, 466, 468, 470, 472, 474, 476, 478,
		480, 482, 484, 486, 488, 490, 492, 494, 496, 498, 500, 502, 504, 506, 508, 510, 512, 514, 516, 518, 520
	};

	static int clrbrdmnk_softcaps[] = {
		274, 276, 278, 278, 280, 282, 284, 286, 288, 290, 292, 292, 294, 296, 298, 300, 302, 304, 306, 308, 308,
		310, 312, 314, 316, 318, 320, 322, 322, 324, 326, 328, 330, 332, 334, 336, 336, 338, 340, 342, 344, 346,
		348, 350, 352, 352, 354, 356, 358, 360, 362, 364, 366, 366, 368, 370, 372, 374, 376, 378, 380, 380, 382,
		384, 386, 388, 390, 392, 394, 396, 396, 398, 400, 402, 404, 406, 408, 410, 410, 412, 414, 416, 418, 420,
		422, 424, 424, 426, 428, 430, 432, 434, 436, 438, 440, 440, 442, 444, 446, 448, 450, 452, 454, 454, 456
	};

	static int palshd_softcaps[] = {
		298, 300, 302, 304, 306, 308, 310, 312, 314, 316, 318, 320, 322, 324, 326, 328, 330, 332, 334, 336, 336,
		338, 340, 342, 344, 346, 348, 350, 352, 354, 356, 358, 360, 362, 364, 366, 368, 370, 372, 374, 376, 378,
		380, 382, 384, 384, 386, 388, 390, 392, 394, 396, 398, 400, 402, 404, 406, 408, 410, 412, 414, 416, 418,
		420, 422, 424, 426, 428, 430, 432, 432, 434, 436, 438, 440, 442, 444, 446, 448, 450, 452, 454, 456, 458,
		460, 462, 464, 466, 468, 470, 472, 474, 476, 478, 480, 480, 482, 484, 486, 488, 490, 492, 494, 496, 498
	};

	static int rng_softcaps[] = {
		286, 288, 290, 292, 294, 296, 298, 298, 300, 302, 304, 306, 308, 310, 312, 314, 316, 318, 320, 322, 322,
		324, 326, 328, 330, 332, 334, 336, 338, 340, 342, 344, 344, 346, 348, 350, 352, 354, 356, 358, 360, 362,
		364, 366, 368, 368, 370, 372, 374, 376, 378, 380, 382, 384, 386, 388, 390, 390, 392, 394, 396, 398, 400,
		402, 404, 406, 408, 410, 412, 414, 414, 416, 418, 420, 422, 424, 426, 428, 430, 432, 434, 436, 436, 438,
		440, 442, 444, 446, 448, 450, 452, 454, 456, 458, 460, 460, 462, 464, 466, 468, 470, 472, 474, 476, 478
	};

	static int dru_softcaps[] = {
		254, 256, 258, 260, 262, 264, 264, 266, 268, 270, 272, 272, 274, 276, 278, 280, 282, 282, 284, 286, 288,
		290, 290, 292, 294, 296, 298, 300, 300, 302, 304, 306, 308, 308, 310, 312, 314, 316, 318, 318, 320, 322,
		324, 326, 328, 328, 330, 332, 334, 336, 336, 338, 340, 342, 344, 346, 346, 348, 350, 352, 354, 354, 356,
		358, 360, 362, 364, 364, 366, 368, 370, 372, 372, 374, 376, 378, 380, 382, 382, 384, 386, 388, 390, 390,
		392, 394, 396, 398, 400, 400, 402, 404, 406, 408, 410, 410, 412, 414, 416, 418, 418, 420, 422, 424, 426
	};

	static int rogshmbstber_softcaps[] = {
		264, 266, 268, 270, 272, 272, 274, 276, 278, 280, 282, 282, 284, 286, 288, 290, 292, 294, 294, 296, 298,
		300, 302, 304, 306, 306, 308, 310, 312, 314, 316, 316, 318, 320, 322, 324, 326, 328, 328, 330, 332, 334,
		336, 338, 340, 340, 342, 344, 346, 348, 350, 350, 352, 354, 356, 358, 360, 362, 362, 364, 366, 368, 370,
		372, 374, 374, 376, 378, 380, 382, 384, 384, 386, 388, 390, 392, 394, 396, 396, 398, 400, 402, 404, 406,
		408, 408, 410, 412, 414, 416, 418, 418, 420, 422, 424, 426, 428, 430, 430, 432, 434, 436, 438, 440, 442
	};

	static int necwizmagenc_softcaps[] = {
		248, 250, 252, 254, 256, 256, 258, 260, 262, 264, 264, 266, 268, 270, 272, 272, 274, 276, 278, 280, 280,
		282, 284, 286, 288, 288, 290, 292, 294, 296, 296, 298, 300, 302, 304, 304, 306, 308, 310, 312, 312, 314,
		316, 318, 320, 320, 322, 324, 326, 328, 328, 330, 332, 334, 336, 336, 338, 340, 342, 344, 344, 346, 348,
		350, 352, 352, 354, 356, 358, 360, 360, 362, 364, 366, 368, 368, 370, 372, 374, 376, 376, 378, 380, 382,
		384, 384, 386, 388, 390, 392, 392, 394, 396, 398, 400, 400, 402, 404, 406, 408, 408, 410, 412, 414, 416
	};

	int level = std::min(105, static_cast<int>(GetLevel())) - 1;

	switch (GetClass()) {
	case Class::Warrior:
		return war_softcaps[level];
	case Class::Cleric:
	case Class::Bard:
	case Class::Monk:
		return clrbrdmnk_softcaps[level];
	case Class::Paladin:
	case Class::ShadowKnight:
		return palshd_softcaps[level];
	case Class::Ranger:
		return rng_softcaps[level];
	case Class::Druid:
		return dru_softcaps[level];
	case Class::Rogue:
	case Class::Shaman:
	case Class::Beastlord:
	case Class::Berserker:
		return rogshmbstber_softcaps[level];
	case Class::Necromancer:
	case Class::Wizard:
	case Class::Magician:
	case Class::Enchanter:
		return necwizmagenc_softcaps[level];
	default:
		return 350;
	}
}

double Mob::GetSoftcapReturns()
{
	// These are based on the dev post, they seem to be correct for every level
	// AKA no more hard caps
	switch (GetClass()) {
	case Class::Warrior:
		return 0.35;
	case Class::Cleric:
	case Class::Bard:
	case Class::Monk:
		return 0.3;
	case Class::Paladin:
	case Class::ShadowKnight:
		return 0.33;
	case Class::Ranger:
		return 0.315;
	case Class::Druid:
		return 0.265;
	case Class::Rogue:
	case Class::Shaman:
	case Class::Beastlord:
	case Class::Berserker:
		return 0.28;
	case Class::Necromancer:
	case Class::Wizard:
	case Class::Magician:
	case Class::Enchanter:
		return 0.25;
	default:
		return 0.3;
	}
}

int Mob::GetClassRaceACBonus()
{
	int ac_bonus = 0;
	auto level = GetLevel();
	if (GetClass() == Class::Monk) {
		int hardcap = 30;
		int softcap = 14;
		if (level > 99) {
			hardcap = 58;
			softcap = 35;
		}
		else if (level > 94) {
			hardcap = 57;
			softcap = 34;
		}
		else if (level > 89) {
			hardcap = 56;
			softcap = 33;
		}
		else if (level > 84) {
			hardcap = 55;
			softcap = 32;
		}
		else if (level > 79) {
			hardcap = 54;
			softcap = 31;
		}
		else if (level > 74) {
			hardcap = 53;
			softcap = 30;
		}
		else if (level > 69) {
			hardcap = 53;
			softcap = 28;
		}
		else if (level > 64) {
			hardcap = 53;
			softcap = 26;
		}
		else if (level > 63) {
			hardcap = 50;
			softcap = 24;
		}
		else if (level > 61) {
			hardcap = 47;
			softcap = 24;
		}
		else if (level > 59) {
			hardcap = 45;
			softcap = 24;
		}
		else if (level > 54) {
			hardcap = 40;
			softcap = 20;
		}
		else if (level > 50) {
			hardcap = 38;
			softcap = 18;
		}
		else if (level > 44) {
			hardcap = 36;
			softcap = 17;
		}
		else if (level > 29) {
			hardcap = 34;
			softcap = 16;
		}
		else if (level > 14) {
			hardcap = 32;
			softcap = 15;
		}
		int weight = IsClient() ? CastToClient()->CalcCurrentWeight()/10 : 0;
		if (weight < hardcap - 1) {
			double temp = level + 5;
			if (weight > softcap) {
				double redux = static_cast<double>(weight - softcap) * 6.66667;
				redux = (100.0 - std::min(100.0, redux)) * 0.01;
				temp = std::max(0.0, temp * redux);
			}
			ac_bonus = static_cast<int>((4.0 * temp) / 3.0);
		}
		else if (weight > hardcap + 1) {
			double temp = level + 5;
			double multiplier = std::min(1.0, (weight - (static_cast<double>(hardcap) - 10.0)) / 100.0);
			temp = (4.0 * temp) / 3.0;
			ac_bonus -= static_cast<int>(temp * multiplier);
		}
	}

	if (GetClass() == Class::Rogue) {
		int level_scaler = level - 26;
		if (GetAGI() < 80)
			ac_bonus = level_scaler / 4;
		else if (GetAGI() < 85)
			ac_bonus = (level_scaler * 2) / 4;
		else if (GetAGI() < 90)
			ac_bonus = (level_scaler * 3) / 4;
		else if (GetAGI() < 100)
			ac_bonus = (level_scaler * 4) / 4;
		else if (GetAGI() >= 100)
			ac_bonus = (level_scaler * 5) / 4;
		if (ac_bonus > 12)
			ac_bonus = 12;
	}

	if (GetClass() == Class::Beastlord) {
		int level_scaler = level - 6;
		if (GetAGI() < 80)
			ac_bonus = level_scaler / 5;
		else if (GetAGI() < 85)
			ac_bonus = (level_scaler * 2) / 5;
		else if (GetAGI() < 90)
			ac_bonus = (level_scaler * 3) / 5;
		else if (GetAGI() < 100)
			ac_bonus = (level_scaler * 4) / 5;
		else if (GetAGI() >= 100)
			ac_bonus = (level_scaler * 5) / 5;
		if (ac_bonus > 16)
			ac_bonus = 16;
	}

	if (GetRace() == Race::Iksar)
		ac_bonus += EQ::Clamp(static_cast<int>(level), 10, 35);

	return ac_bonus;
}
//SYNC WITH: tune.cpp, mob.h TuneACSum
int Mob::ACSum(bool skip_caps)
{
	int ac = 0; // this should be base AC whenever shrouds come around
	ac += itembonuses.AC; // items + food + tribute
	int shield_ac = 0;
	if (HasShieldEquipped() && IsOfClientBot()) {
		auto inst = (IsClient()) ? GetInv().GetItem(EQ::invslot::slotSecondary) : CastToBot()->GetBotItem(EQ::invslot::slotSecondary);
		if (inst) {
			if (inst->GetItemRecommendedLevel(true) <= GetLevel()) {
				shield_ac = inst->GetItemArmorClass(true);
			} else {
				shield_ac = CalcRecommendedLevelBonus(GetLevel(), inst->GetItemRecommendedLevel(true), inst->GetItemArmorClass(true));
			}
		}
		shield_ac += itembonuses.heroic_str_shield_ac;
	}
	// EQ math
	ac = (ac * 4) / 3;
	// anti-twink
	if (!skip_caps && IsOfClientBot() && GetLevel() < RuleI(Combat, LevelToStopACTwinkControl)) {
		ac = std::min(ac, 25 + 6 * GetLevel());
	}
	ac = std::max(0, ac + GetClassRaceACBonus());
	if (IsNPC()) {
		// This is the developer tweaked number
		// for the VAST amount of NPCs in EQ this number didn't exceed 600 until recently (PoWar)
		// According to the guild hall Combat Dummies, a level 50 classic EQ mob it should be ~115
		// For a 60 PoP mob ~120, 70 OoW ~120
		ac += GetAC();
		ac += GetPetACBonusFromOwner();
		auto spell_aa_ac = aabonuses.AC + spellbonuses.AC;
		ac += GetSkill(EQ::skills::SkillDefense) / 5;
		if (EQ::ValueWithin(static_cast<int>(GetClass()), Class::Necromancer, Class::Enchanter))
			ac += spell_aa_ac / 3;
		else
			ac += spell_aa_ac / 4;
	}
	else { // TODO: so we can't set NPC skills ... so the skill bonus ends up being HUGE so lets nerf them a bit
		auto spell_aa_ac = aabonuses.AC + spellbonuses.AC;
		if (EQ::ValueWithin(static_cast<int>(GetClass()), Class::Necromancer, Class::Enchanter))
			ac += GetSkill(EQ::skills::SkillDefense) / 2 + spell_aa_ac / 3;
		else
			ac += GetSkill(EQ::skills::SkillDefense) / 3 + spell_aa_ac / 4;
	}

	if (GetAGI() > 70)
		ac += GetAGI() / 20;
	if (ac < 0)
		ac = 0;

	if (!skip_caps && IsOfClientBot()) {
		auto softcap = GetACSoftcap();
		auto returns = GetSoftcapReturns();
		int total_aclimitmod = aabonuses.CombatStability + itembonuses.CombatStability + spellbonuses.CombatStability;
		if (total_aclimitmod)
			softcap = (softcap * (100 + total_aclimitmod)) / 100;
		softcap += shield_ac;
		if (ac > softcap) {
			auto over_cap = ac - softcap;
			ac = softcap + (over_cap * returns);
		}
		LogCombatDetail("ACSum ac [{}] softcap [{}] returns [{}]", ac, softcap, returns);
	}
	else {
		LogCombatDetail("ACSum ac [{}]", ac);
	}

	return ac;
}

int Mob::GetBestMeleeSkill()
{
	int bestSkill=0;

	EQ::skills::SkillType meleeSkills[]=
	{	EQ::skills::Skill1HBlunt,
	  	EQ::skills::Skill1HSlashing,
		EQ::skills::Skill2HBlunt,
		EQ::skills::Skill2HSlashing,
		EQ::skills::SkillHandtoHand,
		EQ::skills::Skill1HPiercing,
		EQ::skills::Skill2HPiercing,
		EQ::skills::SkillCount
	};
	int i;

	for (i=0; meleeSkills[i] != EQ::skills::SkillCount; ++i) {
		int value;
		value = GetSkill(meleeSkills[i]);
		bestSkill = std::max(value, bestSkill);
	}

	return bestSkill;
}
//SYNC WITH: tune.cpp, mob.h Tuneoffense
int Mob::offense(EQ::skills::SkillType skill)
{
	int offense = GetSkill(skill);
	if (RuleB(Combat, UseMobStaticOffenseSkill) && IsNPC() && !IsPet() && !IsTempPet()) {
		offense = GetMobFixedWeaponSkill();
	}

	int stat_bonus = GetSTR();

	switch (skill) {
		case EQ::skills::SkillArchery:
		case EQ::skills::SkillThrowing:
			stat_bonus = GetDEX();
			break;

		// Mobs with no weapons default to H2H.
		// Since H2H is capped at 100 for many many classes,
		// lets not handicap mobs based on not spawning with a
		// weapon.
		//
		// Maybe we tweak this if Disarm is actually implemented.

		case EQ::skills::SkillHandtoHand:
			offense = GetBestMeleeSkill();
			break;
	}

	if (stat_bonus >= 75) {
		offense += (2 * stat_bonus - 150) / 3;
	}

	// GetATK() = ATK + itembonuses.ATK + spellbonuses.ATK.  However, ATK appears to already be itembonuses.ATK + spellbonuses.ATK for PCs, so as is, it is double counting attack
	// This causes attack to be significantly more important than it should be based on era rule of thumbs.  I do not want to change the GetATK() function in case doing so breaks something,
	// so instead I am just adding a /2 to remedy the double counting.  NPCs do not have this issue, so they are broken up.
	// PCAttackPowerScaling is used to help bring attack power further in line with era estimates.
	if (IsOfClientBotMerc()) {
		offense += (GetATK() / 2 + GetPetATKBonusFromOwner()) * RuleI(Combat, PCAttackPowerScaling) / 100;
	} else {
		offense += GetATK();
	}

	return offense;
}

// this assumes "this" is the defender
// this returns between 0.1 to 2.0
double Mob::RollD20(int offense, int mitigation)
{
	static double mods[] = {
		0.1, 0.2, 0.3, 0.4, 0.5,
		0.6, 0.7, 0.8, 0.9, 1.0,
		1.1, 1.2, 1.3, 1.4, 1.5,
		1.6, 1.7, 1.8, 1.9, 2.0
	};

	if (IsOfClientBotMerc() && IsSitting()) {
		return mods[19];
	}

	auto atk_roll = zone->random.Roll0(offense + 5);
	auto def_roll = zone->random.Roll0(mitigation + 5);

	int avg = std::max(1, (offense + mitigation + 10) / 2);
	int index = std::max(0, (atk_roll - def_roll) + (avg / 2));

	index = EQ::Clamp((index * 20) / avg, 0, 19);

	return mods[index];
}
//SYNC WITH: tune.cpp, mob.h TuneMeleeMitigation
void Mob::MeleeMitigation(Mob *attacker, DamageHitInfo &hit, ExtraAttackOptions *opts)
{
#ifdef LUA_EQEMU
	bool ignoreDefault = false;
	LuaParser::Instance()->MeleeMitigation(this, attacker, hit, opts, ignoreDefault);

	if (ignoreDefault) {
		return;
	}
#endif

	if (hit.damage_done < 0 || hit.base_damage == 0)
		return;

	Mob* defender = this;
	auto mitigation = defender->GetMitigationAC();
	if (IsClient() && attacker->IsClient())
		mitigation = mitigation * 80 / 100; // 2004 PvP changes

	if (opts) {
		mitigation *= (1.0f - opts->armor_pen_percent);
		mitigation -= opts->armor_pen_flat;
	}

	auto roll = RollD20(hit.offense, mitigation);

	// Add bonus to roll if level difference is sufficient
	const int level_diff            = attacker->GetLevel() - GetLevel();
	const int level_diff_roll_check = RuleI(Combat, LevelDifferenceRollCheck);

	if (level_diff_roll_check >= 0) {
		if (level_diff > level_diff_roll_check) {
			roll += RuleR(Combat, LevelDifferenceRollBonus);

			if (roll > 2.0f) {
				roll = 2.0f;
			}
		} else if (level_diff < (-level_diff_roll_check)) {
			roll -= RuleR(Combat, LevelDifferenceRollBonus);

			if (roll < 0.1f) {
				roll = 0.1f;
			}
		}
	}

	// +0.5 for rounding, min to 1 dmg
	hit.damage_done = std::max(static_cast<int>(roll * static_cast<double>(hit.base_damage) + 0.5), 1);

	Log(Logs::Detail, Logs::Attack, "mitigation %d vs offense %d. base %d rolled %f damage %d", mitigation, hit.offense, hit.base_damage, roll, hit.damage_done);
}

//Returns the weapon damage against the input mob
//if we cannot hit the mob with the current weapon we will get a value less than or equal to zero
//Else we know we can hit.
//GetWeaponDamage(mob*, const EQ::ItemData*) is intended to be used for mobs or any other situation where we do not have a client inventory item
//GetWeaponDamage(mob*, const EQ::ItemInstance*) is intended to be used for situations where we have a client inventory item
int64 Mob::GetWeaponDamage(Mob *against, const EQ::ItemData *weapon_item) {
	int64 dmg = 0;
	int64 banedmg = 0;

	//can't hit invulnerable stuff with weapons.
	if (against->GetInvul() || against->GetSpecialAbility(SpecialAbility::MeleeImmunity)) {
		return 0;
	}

	//check to see if our weapons or fists are magical.
	if (against->GetSpecialAbility(SpecialAbility::MeleeImmunityExceptMagical)) {
		if (GetSpecialAbility(SpecialAbility::MagicalAttack)) {
			dmg = 1;
		}
		//On live this occurs for ALL NPC's >= 10
		else if (IsNPC() && GetLevel() >= RuleI(Combat, NPCAttackMagicLevel)) {
			dmg = 1;
		}
		else if (weapon_item) {
			if (weapon_item->Magic) {
				if (weapon_item->Damage && (weapon_item->IsType1HWeapon() || weapon_item->IsType2HWeapon())) {
					dmg = weapon_item->Damage;
				}
				//Non weapon items, ie. boots for kick.
				else if (weapon_item->ItemType == EQ::item::ItemTypeArmor) {
					dmg = 1;
				}
				else {
					return 0;
				}
			}
			else {
				return 0;
			}
		}
		else if ((GetClass() == Class::Monk || GetClass() == Class::Beastlord) && GetLevel() >= 30) {
			dmg = GetHandToHandDamage();
		}
		else {
			return 0;
		}
	}
	else {
		if (weapon_item) {
			dmg = weapon_item->Damage;

			dmg = dmg <= 0 ? 1 : dmg;
		}
		else {
			dmg = GetHandToHandDamage();
		}
	}

	int eledmg = 0;
	if (!against->GetSpecialAbility(SpecialAbility::MagicImmunity)) {
		if (weapon_item && weapon_item->ElemDmgAmt) {
			//we don't check resist for npcs here
			eledmg = weapon_item->ElemDmgAmt;
			dmg += eledmg;
		}
	}

	if (against->GetSpecialAbility(SpecialAbility::MeleeImmunityExceptBane)) {
		if (weapon_item) {
			if (weapon_item->BaneDmgBody == against->GetBodyType()) {
				banedmg += weapon_item->BaneDmgAmt;
			}

			if (weapon_item->BaneDmgRace == against->GetRace()) {
				banedmg += weapon_item->BaneDmgRaceAmt;
			}
		}

		if (!banedmg) {
			if (!GetSpecialAbility(SpecialAbility::BaneAttack))
				return 0;
			else
				return 1;
		}
		else
			dmg += banedmg;
	}
	else {
		if (weapon_item) {
			if (weapon_item->BaneDmgBody == against->GetBodyType()) {
				banedmg += weapon_item->BaneDmgAmt;
			}

			if (weapon_item->BaneDmgRace == against->GetRace()) {
				banedmg += weapon_item->BaneDmgRaceAmt;
			}
		}

		dmg += (banedmg + eledmg);
	}

	if (dmg <= 0) {
		return 0;
	}
	else
		return dmg;
}

int64 Mob::GetWeaponDamage(Mob *against, const EQ::ItemInstance *weapon_item, int64 *hate)
{
	int64 dmg = 0;
	int64 banedmg = 0;
	int x = 0;

	if (!against || against->GetInvul() || against->GetSpecialAbility(SpecialAbility::MeleeImmunity))
		return 0;

	// check for items being illegally attained
	if (weapon_item) {
		if (!weapon_item->GetItem()) {
			return 0;
		}

		if (weapon_item->GetItemRequiredLevel(true) > GetLevel()) {
			return 0;
		}

		if (!weapon_item->IsClassEquipable(GetClass()) &&
			(
				!IsBot() ||
				(IsBot() && !RuleB(Bots, AllowBotEquipAnyClassGear))
			)
		) {
			return 0;
		}

		if (
			!weapon_item->IsRaceEquipable(GetBaseRace()) &&
			(
				!IsBot() ||
				(IsBot() && !RuleB(Bots, AllowBotEquipAnyRaceGear))
			)
		) {
			return 0;
		}
	}

	if (against->GetSpecialAbility(SpecialAbility::MeleeImmunityExceptMagical)) {
		if (weapon_item) {
			// check to see if the weapon is magic
			bool MagicWeapon = weapon_item->GetItemMagical(true) || spellbonuses.MagicWeapon || itembonuses.MagicWeapon;
			if (MagicWeapon) {
				auto rec_level = weapon_item->GetItemRecommendedLevel(true);
				if (IsClient() && GetLevel() < rec_level)
					dmg = CastToClient()->CalcRecommendedLevelBonus(
						GetLevel(), rec_level, weapon_item->GetItemWeaponDamage(true));
				else
					dmg = weapon_item->GetItemWeaponDamage(true);
				dmg = dmg <= 0 ? 1 : dmg;
			}
			else {
				return 0;
			}
		}
		else {
			bool MagicGloves = false;
			if (IsClient()) {
				const EQ::ItemInstance *gloves = CastToClient()->GetInv().GetItem(EQ::invslot::slotHands);
				if (gloves)
					MagicGloves = gloves->GetItemMagical(true);
			}

			if (GetClass() == Class::Monk || GetClass() == Class::Beastlord) {
				if (MagicGloves || GetLevel() >= 30) {
					dmg = GetHandToHandDamage();
					if (hate)
						*hate += dmg;
				}
			}
			else if (GetOwner() &&
				GetLevel() >=
				RuleI(Combat, PetAttackMagicLevel)) { // pets wouldn't actually use this but...
				dmg = 1; // it gives us an idea if we can hit
			}
			else if (MagicGloves || GetSpecialAbility(SpecialAbility::MagicalAttack)) {
				dmg = 1;
			}
			else
				return 0;
		}
	}
	else {
		if (weapon_item) {
			if (weapon_item->GetItem()) {
				auto rec_level = weapon_item->GetItemRecommendedLevel(true);
				if (IsClient() && GetLevel() < rec_level) {
					dmg = CastToClient()->CalcRecommendedLevelBonus(
						GetLevel(), rec_level, weapon_item->GetItemWeaponDamage(true));
				}
				else {
					dmg = weapon_item->GetItemWeaponDamage(true);
				}

				dmg = dmg <= 0 ? 1 : dmg;
			}
		}
		else {
			dmg = GetHandToHandDamage();
			if (hate)
				*hate += dmg;
		}
	}

	int eledmg = 0;
	if (!against->GetSpecialAbility(SpecialAbility::MagicImmunity)) {
		if (weapon_item && weapon_item->GetItem() && weapon_item->GetItemElementalFlag(true))
			// the client actually has the way this is done, it does not appear to check req!
			eledmg = against->ResistElementalWeaponDmg(weapon_item);
	}

	if (weapon_item && weapon_item->GetItem() &&
		(weapon_item->GetItemBaneDamageBody(true) || weapon_item->GetItemBaneDamageRace(true)))
		banedmg = against->CheckBaneDamage(weapon_item);

	if (against->GetSpecialAbility(SpecialAbility::MeleeImmunityExceptBane)) {
		if (!banedmg) {
			if (!GetSpecialAbility(SpecialAbility::BaneAttack))
				return 0;
			else
				return 1;
		}
		else {
			dmg += (banedmg + eledmg);
			if (hate)
				*hate += banedmg;
		}
	}
	else {
		dmg += (banedmg + eledmg);
		if (hate)
			*hate += banedmg;
	}

	return std::max((int64)0, dmg);
}

int64 Mob::DoDamageCaps(int64 base_damage)
{
	// this is based on a client function that caps melee base_damage
	auto level = GetLevel();
	auto stop_level = RuleI(Combat, LevelToStopDamageCaps);
	if (stop_level && stop_level <= level)
		return base_damage;
	int cap = 0;
	if (level >= 125) {
		cap = 7 * level;
	}
	else if (level >= 110) {
		cap = 6 * level;
	}
	else if (level >= 90) {
		cap = 5 * level;
	}
	else if (level >= 70) {
		cap = 4 * level;
	}
	else if (level >= 40) {
		switch (GetClass()) {
		case Class::Cleric:
		case Class::Druid:
		case Class::Shaman:
			cap = 80;
			break;
		case Class::Necromancer:
		case Class::Wizard:
		case Class::Magician:
		case Class::Enchanter:
			cap = 40;
			break;
		default:
			cap = 200;
			break;
		}
	}
	else if (level >= 30) {
		switch (GetClass()) {
		case Class::Cleric:
		case Class::Druid:
		case Class::Shaman:
			cap = 26;
			break;
		case Class::Necromancer:
		case Class::Wizard:
		case Class::Magician:
		case Class::Enchanter:
			cap = 18;
			break;
		default:
			cap = 60;
			break;
		}
	}
	else if (level >= 20) {
		switch (GetClass()) {
		case Class::Cleric:
		case Class::Druid:
		case Class::Shaman:
			cap = 20;
			break;
		case Class::Necromancer:
		case Class::Wizard:
		case Class::Magician:
		case Class::Enchanter:
			cap = 12;
			break;
		default:
			cap = 30;
			break;
		}
	}
	else if (level >= 10) {
		switch (GetClass()) {
		case Class::Cleric:
		case Class::Druid:
		case Class::Shaman:
			cap = 12;
			break;
		case Class::Necromancer:
		case Class::Wizard:
		case Class::Magician:
		case Class::Enchanter:
			cap = 10;
			break;
		default:
			cap = 14;
			break;
		}
	}
	else {
		switch (GetClass()) {
		case Class::Cleric:
		case Class::Druid:
		case Class::Shaman:
			cap = 9;
			break;
		case Class::Necromancer:
		case Class::Wizard:
		case Class::Magician:
		case Class::Enchanter:
			cap = 6;
			break;
		default:
			cap = 10; // this is where the 20 damage cap comes from
			break;
		}
	}

	return std::min((int64)cap, base_damage);
}

// other is the defender, this is the attacker
//SYNC WITH: tune.cpp, mob.h TuneDoAttack
void Mob::DoAttack(Mob *other, DamageHitInfo &hit, ExtraAttackOptions *opts, bool FromRiposte)
{
	if (!other) {
		return;
	}

	LogCombat("[{}]::DoAttack vs [{}] base [{}] min [{}] offense [{}] tohit [{}] skill [{}]", GetName(),
		other->GetName(), hit.base_damage, hit.min_damage, hit.offense, hit.tohit, hit.skill);

	if (!RuleB(Combat, UseLiveRiposteMechanics)) {
		FromRiposte = false;
	}

	// check to see if we hit..
	if (!FromRiposte && other->AvoidDamage(this, hit)) {
		if (int strike_through = itembonuses.StrikeThrough + spellbonuses.StrikeThrough + aabonuses.StrikeThrough;
				strike_through && zone->random.Roll(strike_through)) {

			FilteredMessageString(
				this, /* Sender */
				Chat::StrikeThrough, /* Type: 339 */
				FilterStrikethrough, /* FilterType: 12 */
				STRIKETHROUGH_STRING /* You strike through your opponent's defenses! */
			);

			hit.damage_done = 1;			// set to one, we will check this to continue
		}

		if (hit.damage_done == DMG_RIPOSTED) {
			DoRiposte(other);
			return;
		}

		LogCombat("Avoided/strikethrough damage with code [{}]", hit.damage_done);
	}

	if (hit.damage_done >= 0) {
		if (other->CheckHitChance(this, hit)) {
			if (IsNPC() && other->IsClient() && other->animation > 0 && GetLevel() >= 5 && BehindMob(other, GetX(), GetY())) {
				// ~ 12% chance
				if (zone->random.Roll(RuleI(Combat, StunChance))) {
					int stun_resist2 = other->spellbonuses.FrontalStunResist + other->itembonuses.FrontalStunResist + other->aabonuses.FrontalStunResist;
					int stun_resist = other->spellbonuses.StunResist + other->itembonuses.StunResist + other->aabonuses.StunResist;
					if (zone->random.Roll(stun_resist2)) {
						other->FilteredMessageString(
							this,
							Chat::Stun,
							FilterStuns,
							AVOID_STUNNING_BLOW
						);
					} else if (zone->random.Roll(stun_resist)) {
						other->FilteredMessageString(
							this,
							Chat::Stun,
							FilterStuns,
							SHAKE_OFF_STUN
						);
					} else {
						other->Stun(3000); // yuck -- 3 seconds
					}
				}
			}
			other->MeleeMitigation(this, hit, opts);
			if (hit.damage_done > 0) {
				ApplyDamageTable(hit);
				CommonOutgoingHitSuccess(other, hit, opts);
			}
			LogCombat("Final damage after all reductions: [{}]", hit.damage_done);
		}
		else {
			LogCombat("Attack missed. Damage set to 0");
			hit.damage_done = 0;
		}

		parse->EventBotMerc(EVENT_USE_SKILL, this, nullptr,
			[&]() {
				return fmt::format(
					"{} {}",
					hit.skill,
					GetSkill(hit.skill)
				);
			}
		);
	}
}

//note: throughout this method, setting `damage` to a negative is a way to
//stop the attack calculations
// IsFromSpell added to allow spell effects to use Attack. (Mainly for the Rampage AA right now.)
//SYNC WITH: tune.cpp, mob.h TuneClientAttack
bool Mob::Attack(Mob* other, int Hand, bool bRiposte, bool IsStrikethrough, bool IsFromSpell, ExtraAttackOptions *opts)
{
	if (!other || (other && other->IsCorpse())) {
		SetTarget(nullptr);
		LogError("A null or invalid Mob object was passed for evaluation!");
		return false;
	}

	if (!GetTarget()) {
		SetTarget(other);
	}

	LogCombatDetail("Attacking [{}] with hand [{}] [{}]", other->GetName(), Hand, bRiposte ? "this is a riposte" : "");

	if (
		(IsCasting() && GetClass() != Class::Bard && !IsFromSpell)
		|| ((IsClient() && CastToClient()->dead) || (other->IsClient() && other->CastToClient()->dead))
		|| (GetHP() < 0)
		|| (!IsAttackAllowed(other))
		|| (IsBot() && GetAppearance() == eaDead)
		) {
		LogCombat("Attack cancelled, invalid circumstances");
		return false; // Only bards can attack while casting
	}

	if (DivineAura() && !CastToClient()->GetGM()) { //cant attack while invulnerable unless your a gm
		LogCombat("Attack cancelled, Divine Aura is in effect");
		MessageString(Chat::DefaultText, DIVINE_AURA_NO_ATK);
		return false;
	}

	if (GetFeigned()) {
		return false; // Rogean: How can you attack while feigned? Moved up from Aggro Code.
	}

	const EQ::ItemInstance* weapon = nullptr;


	if (IsBot()) {
		FaceTarget(GetTarget());
	}

	if (Hand == EQ::invslot::slotSecondary) {
		weapon = (IsClient()) ? GetInv().GetItem(EQ::invslot::slotSecondary) : CastToBot()->GetBotItem(EQ::invslot::slotSecondary);
		OffHandAtk(true);
	}
	else {
		weapon = (IsClient()) ? GetInv().GetItem(EQ::invslot::slotPrimary) : CastToBot()->GetBotItem(EQ::invslot::slotPrimary);
		OffHandAtk(false);
	}
	if (weapon != nullptr) {
		if (!weapon->IsWeapon()) {
			LogCombat("Attack cancelled, Item [{}] ([{}]) is not a weapon", weapon->GetItem()->Name, weapon->GetID());
			return(false);
		}
		LogCombatDetail("Attacking with weapon: [{}] ([{}])", weapon->GetItem()->Name, weapon->GetID());
	}
	else {
		LogCombatDetail("Attacking without a weapon");
	}

	DamageHitInfo my_hit;
	// calculate attack_skill and skillinuse depending on hand and weapon
	// also send Packet to near clients
	my_hit.skill = AttackAnimation(Hand, weapon);
	LogCombatDetail("Attacking with [{}] in slot [{}] using skill [{}]", weapon ? weapon->GetItem()->Name : "Fist", Hand, my_hit.skill);

	// Now figure out damage
	my_hit.damage_done = 1;
	my_hit.min_damage = 0;
	int64 hate = 0;
	if (weapon)
		hate = (weapon->GetItem()->Damage + weapon->GetItem()->ElemDmgAmt);

	my_hit.base_damage = GetWeaponDamage(other, weapon, &hate);
	if (hate == 0 && my_hit.base_damage > 1)
		hate = my_hit.base_damage;

	//if weapon damage > 0 then we know we can hit the target with this weapon
	//otherwise we cannot and we set the damage to -5 later on
	if (my_hit.base_damage > 0) {
		// if we revamp this function be more general, we will have to make sure this isn't
		// executed for anything BUT normal melee damage weapons from auto attack
		if (Hand == EQ::invslot::slotPrimary || Hand == EQ::invslot::slotSecondary)
			my_hit.base_damage = DoDamageCaps(my_hit.base_damage);
		auto shield_inc = spellbonuses.ShieldEquipDmgMod + itembonuses.ShieldEquipDmgMod + aabonuses.ShieldEquipDmgMod;
		if (shield_inc > 0 && HasShieldEquipped() && Hand == EQ::invslot::slotPrimary) {
			my_hit.base_damage = my_hit.base_damage * (100 + shield_inc) / 100;
			hate = hate * (100 + shield_inc) / 100;
		}

		if (IsClient()) {
			CastToClient()->CheckIncreaseSkill(my_hit.skill, other, -15);
			CastToClient()->CheckIncreaseSkill(EQ::skills::SkillOffense, other, -15);
		}

		// ***************************************************************
		// *** Calculate the damage bonus, if applicable, for this hit ***
		// ***************************************************************

#ifndef EQEMU_NO_WEAPON_DAMAGE_BONUS

		// If you include the preprocessor directive "#define EQEMU_NO_WEAPON_DAMAGE_BONUS", that indicates that you do not
		// want damage bonuses added to weapon damage at all. This feature was requested by ChaosSlayer on the EQEmu Forums.
		//
		// This is not recommended for normal usage, as the damage bonus represents a non-trivial component of the DPS output
		// of weapons wielded by higher-level melee characters (especially for two-handed weapons).

		int ucDamageBonus = 0;

		if (Hand == EQ::invslot::slotPrimary && GetLevel() >= 28 && IsWarriorClass())
		{
			// Damage bonuses apply only to hits from the main hand (Hand == MainPrimary) by characters level 28 and above
			// who belong to a melee class. If we're here, then all of these conditions apply.

			ucDamageBonus = GetWeaponDamageBonus(weapon ? weapon->GetItem() : (const EQ::ItemData*) nullptr);

			my_hit.min_damage = ucDamageBonus;
			hate += ucDamageBonus;
		}
#endif
		//Live AA - Sinister Strikes *Adds weapon damage bonus to offhand weapon.
		if (Hand == EQ::invslot::slotSecondary &&
			(
				aabonuses.SecondaryDmgInc ||
				itembonuses.SecondaryDmgInc ||
				spellbonuses.SecondaryDmgInc
			)
		) {
			ucDamageBonus = GetWeaponDamageBonus(weapon ? weapon->GetItem() : (const EQ::ItemData*) nullptr, true);

			my_hit.min_damage = ucDamageBonus;
			hate += ucDamageBonus;
		}

		LogCombatDetail("Damage calculated base [{}] min damage [{}] skill [{}]", my_hit.base_damage, my_hit.min_damage, my_hit.skill);

		int hit_chance_bonus = 0;
		my_hit.offense = offense(my_hit.skill); // we need this a few times
		my_hit.hand = Hand;

		if (opts) {
			my_hit.base_damage *= opts->damage_percent;
			my_hit.base_damage += opts->damage_flat;
			hate *= opts->hate_percent;
			hate += opts->hate_flat;
			hit_chance_bonus += opts->hit_chance;
		}

		my_hit.tohit = GetTotalToHit(my_hit.skill, hit_chance_bonus);

		DoAttack(other, my_hit, opts, bRiposte);

		LogCombatDetail("Final damage after all reductions [{}]", my_hit.damage_done);
	}
	else {
		my_hit.damage_done = DMG_INVULNERABLE;
	}

	// Hate Generation is on a per swing basis, regardless of a hit, miss, or block, its always the same.
	// If we are this far, this means we are atleast making a swing.

	other->AddToHateList(this, hate);

	//Guard Assist Code
	if (RuleB(Character, PVPEnableGuardFactionAssist) &&
		(
			(IsClient() && other->IsClient()) ||
			(HasOwner() && GetOwner()->IsClient() && other->IsClient())
		)
	) {
		for (auto const& [id, mob] : other->GetCloseMobList()) {
			if (!mob) {
				continue;
			}

			if (mob->IsNPC() && mob->CastToNPC()->IsGuard()) {
				float distance = Distance(other->CastToClient()->m_Position, mob->GetPosition());
				if ((mob->CheckLosFN(other) || mob->CheckLosFN(this)) && distance <= 70) {
					auto petorowner = GetOwnerOrSelf();
					if (other->GetReverseFactionCon(mob) <= petorowner->GetReverseFactionCon(mob)) {
						mob->AddToHateList(this);
					}
				}
			}
		}
	}

	///////////////////////////////////////////////////////////
	////// Send Attack Damage
	///////////////////////////////////////////////////////////
	other->Damage(this, my_hit.damage_done, SPELL_UNKNOWN, my_hit.skill, true, -1, false, m_specialattacks);

	if (CastToClient()->IsDead() || (IsBot() && GetAppearance() == eaDead)) {
		return false;
	}

	MeleeLifeTap(my_hit.damage_done);

	CommonBreakInvisibleFromCombat();

	if (GetTarget()) {
		TriggerDefensiveProcs(other, Hand, true, my_hit.damage_done);
	}

	if (my_hit.damage_done > 0) {
		return true;
	}
	else {
		return false;
	}
}

void Client::Damage(Mob* other, int64 damage, uint16 spell_id, EQ::skills::SkillType attack_skill, bool avoidable, int8 buffslot, bool iBuffTic, eSpecialAttacks special)
{
	if (dead || IsCorpse())
		return;

	if (!IsValidSpell(spell_id)) {
		spell_id = SPELL_UNKNOWN;
	}

	// cut all PVP spell damage to 2/3
	// Blasting ourselfs is considered PvP
	//Don't do PvP mitigation if the caster is damaging himself
	//should this be applied to all damage? comments sound like some is for spell DMG
	//patch notes on PVP reductions only mention archery/throwing ... not normal dmg
	if (other && other->IsClient() && (other != this) && damage > 0) {
		int PvPMitigation = 100;
		if (attack_skill == EQ::skills::SkillArchery || attack_skill == EQ::skills::SkillThrowing)
			PvPMitigation = 80;
		else
			PvPMitigation = 67;
		damage = std::max<int64_t>((damage * PvPMitigation) / 100, 1);
	}

	if (!ClientFinishedLoading())
		damage = -5;

	//do a majority of the work...
	CommonDamage(other, damage, spell_id, attack_skill, avoidable, buffslot, iBuffTic, special);

	if (damage > 0) {

		if (!IsValidSpell(spell_id)) {
			CheckIncreaseSkill(EQ::skills::SkillDefense, other, -15);
		}
	}
}

bool Client::Death(Mob* killer_mob, int64 damage, uint16 spell, EQ::skills::SkillType attack_skill, KilledByTypes killed_by, bool is_buff_tic)
{
	if (!ClientFinishedLoading() || dead) {
		return false;
	}

	if (!spell) {
		spell = SPELL_UNKNOWN;
	}

	if (parse->PlayerHasQuestSub(EVENT_DEATH)) {
		const auto& export_string = fmt::format(
			"{} {} {} {}",
			killer_mob ? killer_mob->GetID() : 0,
			damage,
			spell,
			static_cast<int>(attack_skill)
		);

		if (parse->EventPlayer(EVENT_DEATH, this, export_string, 0) != 0) {
			if (GetHP() < 0) {
				SetHP(0);
			}
			return false;
		}
	}

	if (killer_mob && killer_mob->IsOfClientBot() && IsValidSpell(spell) && damage > 0) {
		char val1[20] = { 0 };

		entity_list.MessageCloseString(
			this, /* Sender */
			false, /* Skip Sender */
			RuleI(Range, DamageMessages),
			Chat::NonMelee, /* 283 */
			HIT_NON_MELEE, /* %1 hit %2 for %3 points of non-melee damage. */
			killer_mob->GetCleanName(), /* Message1 */
			GetCleanName(), /* Message2 */
			ConvertArray(damage, val1)/* Message3 */
		);
	}

	int exploss = 0;
	LogCombat("Fatal blow dealt by [{}] with [{}] damage, spell [{}], skill [{}]", killer_mob ? killer_mob->GetName() : "Unknown", damage, spell, attack_skill);

	// #1: Send death packet to everyone
	uint8 killed_level = GetLevel();

	SendLogoutPackets();

	/* Make self become corpse packet */
	EQApplicationPacket app2(OP_BecomeCorpse, sizeof(BecomeCorpse_Struct));
	BecomeCorpse_Struct* bc = (BecomeCorpse_Struct*)app2.pBuffer;
	bc->spawn_id = GetID();
	bc->x = GetX();
	bc->y = GetY();
	bc->z = GetZ();
	QueuePacket(&app2);

	/* Make Death Packet */
	EQApplicationPacket app(OP_Death, sizeof(Death_Struct));
	Death_Struct* d = (Death_Struct*)app.pBuffer;

	// Convert last message to color to avoid duplicate damage messages
	// that occur in these rare cases when this is the death blow.
	if (IsValidSpell(spell) &&
		(attack_skill == EQ::skills::SkillTigerClaw ||
        (IsDamageSpell(spell) && IsDiscipline(spell)) ||
		!is_buff_tic)) {
			d->attack_skill = DamageTypeSpell;
			d->spell_id = (is_buff_tic) ? UINT32_MAX : spell;
	}
	else {
		d->attack_skill = SkillDamageTypes[attack_skill];
		d->spell_id = UINT32_MAX;
	}

	d->spawn_id = GetID();
	d->killer_id = killer_mob ? killer_mob->GetID() : 0;
	d->corpseid = GetID();
	d->bindzoneid = m_pp.binds[0].zone_id;
	d->damage = damage;
	app.priority = 6;
	entity_list.QueueClients(this, &app);

	// #2: figure out things that affect the player dying and mark them dead

	InterruptSpell();

	Mob* m_pet = GetPet();
	SetPet(0);
	SetHorseId(0);
	ShieldAbilityClearVariables();
	dead = true;

	if (m_pet && m_pet->IsCharmed()) {
		m_pet->BuffFadeByEffect(SpellEffect::Charm);
	}

	if (GetMerc()) {
		GetMerc()->Suspend();
	}

	if (killer_mob) {
		parse->EventBotMercNPC(EVENT_SLAY, killer_mob, this);

		if (killer_mob->IsNPC()) {
			killed_by = KilledByTypes::Killed_NPC;

			auto emote_id = killer_mob->GetEmoteID();
			if (emote_id) {
				killer_mob->CastToNPC()->DoNPCEmote(EQ::constants::EmoteEventTypes::KilledPC, emoteid, this);
			}

			killer_mob->TrySpellOnKill(killed_level, spell);
		}

		if (
			killer_mob->IsClient() &&
			(IsDueling() || killer_mob->CastToClient()->IsDueling())
		) {
			SetDueling(false);
			SetDuelTarget(0);
			if (
				killer_mob->IsClient() &&
				killer_mob->CastToClient()->IsDueling() &&
				killer_mob->CastToClient()->GetDuelTarget() == GetID()
			) {
				//if duel opponent killed us...
				killer_mob->CastToClient()->SetDueling(false);
				killer_mob->CastToClient()->SetDuelTarget(0);
				entity_list.DuelMessage(killer_mob, this, false);
				killed_by = KilledByTypes::Killed_DUEL;
			} else {
				//otherwise, we just died, end the duel.
				Mob* who = entity_list.GetMob(GetDuelTarget());
				if (who && who->IsClient()) {
					who->CastToClient()->SetDueling(false);
					who->CastToClient()->SetDuelTarget(0);
				}
			}
		} else if (killer_mob->IsClient()) {
			killed_by = KilledByTypes::Killed_PVP;
		}
	}

	entity_list.RemoveFromTargets(this, true);
	hate_list.RemoveEntFromHateList(this);
	RemoveAutoXTargets();

	//remove ourself from all proximities
	ClearAllProximities();

	/*
	#3: exp loss and corpse generation
	*/

	// figure out if they should lose exp
	if (RuleB(Character, UseDeathExpLossMult)) {
		float exp_losses[] = { 0.005f, 0.015f, 0.025f, 0.035f, 0.045f, 0.055f, 0.065f, 0.075f, 0.085f, 0.095f, 0.110f };
		int exp_loss = RuleI(Character, DeathExpLossMultiplier);
		if (!EQ::ValueWithin(exp_loss, 0, 10)) {
			exp_loss = 3;
		}

		auto current_exp_loss = exp_losses[exp_loss];

		exploss = static_cast<int>(static_cast<float>(GetEXP()) * current_exp_loss); //loose % of total XP pending rule (choose 0-10)
	}

	if (!RuleB(Character, UseDeathExpLossMult)) {
		exploss = static_cast<int>(GetLevel() * (GetLevel() / 18.0) * 12000);
	}

	if (RuleB(Zone, LevelBasedEXPMods)) {
		// Death in levels with xp_mod (such as hell levels) was resulting
		// in losing more that appropriate since the loss was the same but
		// getting it back would take way longer.  This makes the death the
		// same amount of time to recover.  Will also lose more if level is
		// granting a bonus.
		exploss *= zone->level_exp_mod[GetLevel()].ExpMod;
	}

	if (exploss > 0 && RuleB(Character, DeathKeepLevel)) {
		int32 total_exp = GetEXP();
		uint32 level_min_exp = GetEXPForLevel(killed_level);
		int32 level_exp = total_exp - level_min_exp;
		if (exploss > level_exp) {
			exploss = level_exp;
		}
	}

	if ((GetLevel() < RuleI(Character, DeathExpLossLevel)) || (GetLevel() > RuleI(Character, DeathExpLossMaxLevel)) || IsBecomeNPC()) {
		exploss = 0;
	} else if (killer_mob) {
		if (
			killer_mob->IsOfClientBot() ||
			(killer_mob->GetOwner() && killer_mob->GetOwner()->IsOfClientBot())
		) {
			exploss = 0;
		}
	}

	if (IsValidSpell(spell)) {
		uint32 buff_count = GetMaxTotalSlots();
		for (uint16 buffIt = 0; buffIt < buff_count; buffIt++) {
			if (buffs[buffIt].spellid == spell && buffs[buffIt].client) {
				exploss = 0;	// no exp loss for pvp dot
				break;
			}
		}
	}

	bool leave_corpse = false;
	Corpse* new_corpse = nullptr;

	// now we apply the exp loss, unmem their spells, and make a corpse
	// unless they're a GM (or less than lvl 10
	if (!GetGM()) {
		if (exploss > 0) {
			int32 newexp = GetEXP();
			if (exploss > newexp) {
				//lost more than we have... wtf..
				newexp = 1;
			} else {
				newexp -= exploss;
			}
			SetEXP(ExpSource::Death, newexp, GetAAXP());
			//m_epp.perAA = 0;	//reset to no AA exp on death.
		}

		auto illusion_spell_id = spellbonuses.Illusion;

		//this generates a lot of 'updates' to the client that the client does not need
		if (RuleB(Spells, BuffsFadeOnDeath)) {
			BuffFadeNonPersistDeath();
		}

		if (RuleB(Character, UnmemSpellsOnDeath)) {
			if ((ClientVersionBit() & EQ::versions::maskSoFAndLater) && RuleB(Character, RespawnFromHover)) {
				UnmemSpellAll(true);
			} else {
				UnmemSpellAll(false);
			}
		}

		if (
			(RuleB(Character, LeaveCorpses) && GetLevel() >= RuleI(Character, DeathItemLossLevel)) ||
			RuleB(Character, LeaveNakedCorpses)
		) {
			// creating the corpse takes the cash/items off the player too
			new_corpse = new Corpse(this, exploss, killed_by);

			std::string tmp;
			database.GetVariable("ServerType", tmp);
			if (tmp[0] == '1' && tmp[1] == '\0' && killer_mob && killer_mob->IsClient()) {
				database.GetVariable("PvPreward", tmp);
				auto reward = Strings::ToInt(tmp);
				if (reward == 3) {
					database.GetVariable("PvPitem", tmp);
					auto pvp_item_id = Strings::ToInt(tmp);
					const auto* item = database.GetItem(pvp_item_id);
					if (item) {
						new_corpse->SetPlayerKillItemID(pvp_item_id);
					}
				} else if (reward == 2) {
					new_corpse->SetPlayerKillItemID(-1);
				} else if (reward == 1) {
					new_corpse->SetPlayerKillItemID(1);
				} else {
					new_corpse->SetPlayerKillItemID(0);
				}

				if (killer_mob->CastToClient()->isgrouped) {
					auto* group = entity_list.GetGroupByClient(killer_mob->CastToClient());
					if (group) {
						for (int i = 0; i < 6; i++) {
							if (group->members[i]) {
								new_corpse->AllowPlayerLoot(group->members[i], i);
							}
						}
					}
				}
			}

			entity_list.AddCorpse(new_corpse, GetID());
			SetID(0);

			//send the become corpse packet to everybody else in the zone.
			entity_list.QueueClients(this, &app2, true);
			ApplyIllusionToCorpse(illusion_spell_id, new_corpse);
			leave_corpse = true;
		}
	} else {
		BuffFadeDetrimental();
	}

	/*
	Reset AA reuse timers that need to be, live-like this is only Lay on Hands
	*/
	ResetOnDeathAlternateAdvancement();

	/*
	Reset reuse timer for classic skill based Lay on Hands (For tit I guess)
	*/
	if (GetClass() == Class::Paladin) { // we could check if it's not expired I guess, but should be fine not to
		p_timers.Clear(&database, pTimerLayHands);
	}

	/*
	Finally, send em home

	We change the mob variables, not pp directly, because Save() will copy
	from these and overwrite what we set in pp anyway
	*/

	if (leave_corpse && (ClientVersionBit() & EQ::versions::maskSoFAndLater) && RuleB(Character, RespawnFromHover)) {
		ClearDraggedCorpses();
		RespawnFromHoverTimer.Start(RuleI(Character, RespawnFromHoverTimer) * 1000);
		SendRespawnBinds();
	} else {
		if (isgrouped) {
			auto* g = GetGroup();
			if (g) {
				g->MemberZoned(this);
			}
		}

		auto* r = entity_list.GetRaidByClient(this);

		if (r) {
			r->MemberZoned(this);
		}

		dead_timer.Start(5000, true);
		m_pp.zone_id = m_pp.binds[0].zone_id;
		m_pp.zoneInstance = m_pp.binds[0].instance_id;
		database.MoveCharacterToZone(CharacterID(), m_pp.zone_id);
		Save();
		GoToDeath();
	}

	if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::DEATH)) {
		auto e = PlayerEvent::DeathEvent{
			.killer_id = killer_mob ? static_cast<uint32>(killer_mob->GetID()) : static_cast<uint32>(0),
			.killer_name = killer_mob ? killer_mob->GetCleanName() : "No Killer",
			.damage = damage,
			.spell_id = spell,
			.spell_name = IsValidSpell(spell) ? spells[spell].name : "No Spell",
			.skill_id = static_cast<int>(attack_skill),
			.skill_name = !EQ::skills::GetSkillName(attack_skill).empty() ? EQ::skills::GetSkillName(attack_skill) : "No Skill",
		};

		RecordPlayerEventLog(PlayerEvent::DEATH, e);
	}

	if (parse->PlayerHasQuestSub(EVENT_DEATH_COMPLETE)) {
		const auto& export_string = fmt::format(
			"{} {} {} {}",
			killer_mob ? killer_mob->GetID() : 0,
			damage,
			spell,
			static_cast<int>(attack_skill)
		);

		std::vector<std::any> args = { new_corpse };

		parse->EventPlayer(EVENT_DEATH_COMPLETE, this, export_string, 0, &args);
	}
	return true;
}

bool Client::CheckIfAlreadyDead()
{
	if (!ClientFinishedLoading()) {
		return false;
	}

	if (dead) {
		return false;	//cant die more than once...
	}

	return true;
}

//SYNC WITH: tune.cpp, mob.h TuneNPCAttack
bool NPC::Attack(Mob* other, int Hand, bool bRiposte, bool IsStrikethrough, bool IsFromSpell, ExtraAttackOptions *opts)
{
	if (!other) {
		SetTarget(nullptr);
		LogError("A null Mob object was passed to NPC::Attack() for evaluation!");
		return false;
	}

	if (DivineAura())
		return(false);

	if (!GetTarget())
		SetTarget(other);

	//Check that we can attack before we calc heading and face our target
	if (!IsAttackAllowed(other)) {
		if (GetOwnerID()) {
			SayString(NOT_LEGAL_TARGET);
		}

		if (other->IsClient()) {
			other->CastToClient()->RemoveXTarget(this, false);
		}

		RemoveFromHateList(other);
		RemoveFromRampageList(other);
		LogCombat("I am not allowed to attack [{}]", other->GetName());
		return false;
	}

	FaceTarget(GetTarget());

	DamageHitInfo my_hit;
	my_hit.skill = EQ::skills::SkillHandtoHand;
	my_hit.hand = Hand;
	my_hit.damage_done = 1;
	if (Hand == EQ::invslot::slotPrimary) {
		my_hit.skill = static_cast<EQ::skills::SkillType>(GetPrimSkill());
		OffHandAtk(false);
	}

	if (Hand == EQ::invslot::slotSecondary) {
		my_hit.skill = static_cast<EQ::skills::SkillType>(GetSecSkill());
		OffHandAtk(true);
	}

	//figure out what weapon they are using, if any
	const EQ::ItemData *weapon = nullptr;
	if (Hand == EQ::invslot::slotPrimary && equipment[EQ::invslot::slotPrimary] > 0) {
		weapon = database.GetItem(equipment[EQ::invslot::slotPrimary]);
	} else if (equipment[EQ::invslot::slotSecondary]) {
		weapon = database.GetItem(equipment[EQ::invslot::slotSecondary]);
	}

	//We dont factor much from the weapon into the attack.
	//Just the skill type so it doesn't look silly using punching animations and stuff while wielding weapons
	if (weapon) {
		LogCombat("Attacking with weapon: [{}] ([{}]) (too bad im not using it for much)", weapon->Name, weapon->ID);

		if (Hand == EQ::invslot::slotSecondary && !weapon->IsType1HWeapon()) {
			LogCombat("Attack with non-weapon cancelled");
			return false;
		}

		switch (weapon->ItemType) {
			case EQ::item::ItemType1HSlash:
				my_hit.skill = EQ::skills::Skill1HSlashing;
				break;
			case EQ::item::ItemType2HSlash:
				my_hit.skill = EQ::skills::Skill2HSlashing;
				break;
			case EQ::item::ItemType1HPiercing:
				my_hit.skill = EQ::skills::Skill1HPiercing;
				break;
			case EQ::item::ItemType2HPiercing:
				my_hit.skill = EQ::skills::Skill2HPiercing;
				break;
			case EQ::item::ItemType1HBlunt:
				my_hit.skill = EQ::skills::Skill1HBlunt;
				break;
			case EQ::item::ItemType2HBlunt:
				my_hit.skill = EQ::skills::Skill2HBlunt;
				break;
			case EQ::item::ItemTypeBow:
				my_hit.skill = EQ::skills::SkillArchery;
				break;
			case EQ::item::ItemTypeLargeThrowing:
			case EQ::item::ItemTypeSmallThrowing:
				my_hit.skill = EQ::skills::SkillThrowing;
				break;
			default:
				my_hit.skill = EQ::skills::SkillHandtoHand;
				break;
		}
	}

	//Guard Assist Code
	if (RuleB(Character, PVPEnableGuardFactionAssist)) {
		if (IsClient() && other->IsClient() || (HasOwner() && GetOwner()->IsClient() && other->IsClient())) {
			for (auto& e : other->GetCloseMobList()) {
				auto mob = e.second;
				if (!mob) {
					continue;
				}

				if (mob->IsNPC() && mob->CastToNPC()->IsGuard()) {
					float distance = Distance(other->GetPosition(), mob->GetPosition());
					if ((mob->CheckLosFN(other) || mob->CheckLosFN(this)) && distance <= 70) {
						if (other->GetReverseFactionCon(mob) <= GetOwner()->GetReverseFactionCon(mob)) {
							mob->AddToHateList(this);
						}
					}
				}
			}
		}
	}

	int64 weapon_damage = GetWeaponDamage(other, weapon);

	//do attack animation regardless of whether or not we can hit below
	int16 charges = 0;
	EQ::ItemInstance weapon_inst(weapon, charges);
	my_hit.skill = AttackAnimation(Hand, &weapon_inst, my_hit.skill);

	//basically "if not immune" then do the attack
	if (weapon_damage > 0) {

		//ele and bane dmg too
		//NPCs add this differently than PCs
		//if NPCs can't inheriently hit the target we don't add bane/magic dmg which isn't exactly the same as PCs
		int eleBane = 0;
		if (weapon) {
			if (RuleB(NPC, UseBaneDamage)) {
				if (weapon->BaneDmgBody == other->GetBodyType()) {
					eleBane += weapon->BaneDmgAmt;
				}

				if (weapon->BaneDmgRace == other->GetRace()) {
					eleBane += weapon->BaneDmgRaceAmt;
				}
			}

			// I don't think NPCs use this either ....
			if (weapon->ElemDmgAmt) {
				eleBane += (weapon->ElemDmgAmt * other->ResistSpell(weapon->ElemDmgType, 0, this) / 100);
			}
		}

		if (!RuleB(NPC, UseItemBonusesForNonPets)) {
			if (!GetOwner()) {
				eleBane = 0;
			}
		}

		uint8 otherlevel = other->GetLevel();
		uint8 mylevel = GetLevel();

		otherlevel = otherlevel ? otherlevel : 1;
		mylevel = mylevel ? mylevel : 1;

		my_hit.base_damage = GetBaseDamage() + eleBane;
		my_hit.min_damage = GetMinDamage();
		int32 hate = my_hit.base_damage + my_hit.min_damage;

		int hit_chance_bonus = 0;

		if (opts) {
			my_hit.base_damage *= opts->damage_percent;
			my_hit.base_damage += opts->damage_flat;
			hate *= opts->hate_percent;
			hate += opts->hate_flat;
			hit_chance_bonus += opts->hit_chance;
		}

		my_hit.offense = offense(my_hit.skill);
		my_hit.tohit = GetTotalToHit(my_hit.skill, hit_chance_bonus);

		DoAttack(other, my_hit, opts, bRiposte);

		other->AddToHateList(this, hate);

		LogCombat("Final damage against [{}]: [{}]", other->GetName(), my_hit.damage_done);

		if (other->IsClient() && IsPet() && GetOwner()->IsOfClientBot()) {
			//pets do half damage to clients in pvp
			my_hit.damage_done /= 2;
			if (my_hit.damage_done < 1) {
				my_hit.damage_done = 1;
			}
		}
	} else {
		my_hit.damage_done = DMG_INVULNERABLE;
	}

	if (GetHP() > 0 && !other->HasDied()) {
		other->Damage(this, my_hit.damage_done, SPELL_UNKNOWN, my_hit.skill, true, -1, false, m_specialattacks); // Not avoidable client already had thier chance to Avoid
	} else {
		return false;
	}

	if (HasDied()) { //killed by damage shield ect
		return false;
	}

	MeleeLifeTap(my_hit.damage_done);

	CommonBreakInvisibleFromCombat();

	//I doubt this works...
	if (!GetTarget()) {
		return true; //We killed them
	}

	bool has_hit = my_hit.damage_done > 0;
	if (has_hit && !bRiposte && !other->HasDied()) {
		TryWeaponProc(nullptr, weapon, other, Hand);

		if (!other->HasDied()) {
			TrySpellProc(nullptr, weapon, other, Hand);
		}

		if (HasSkillProcSuccess() && !other->HasDied()) {
			TrySkillProc(other, my_hit.skill, 0, true, Hand);
		}
	}

	if (GetHP() > 0 && !other->HasDied()) {
		TriggerDefensiveProcs(other, Hand, true, my_hit.damage_done);
	}

	return has_hit;
}

void NPC::Damage(Mob* other, int64 damage, uint16 spell_id, EQ::skills::SkillType attack_skill, bool avoidable, int8 buffslot, bool iBuffTic, eSpecialAttacks special) {
	if (spell_id == 0)
		spell_id = SPELL_UNKNOWN;

	//handle EVENT_ATTACK. Resets after we have not been attacked for 12 seconds
	if (attacked_timer.Check()) {
		parse->EventMercNPC(EVENT_ATTACK, this, other);
	}

	attacked_timer.Start(CombatEventTimer_expire);

	if (!IsEngaged())
		zone->AddAggroMob();

	if (GetClass() == Class::LDoNTreasure)
	{
		if (IsLDoNLocked() && GetLDoNLockedSkill() != LDoNTypeMechanical)
		{
			damage = -5;
		}
		else
		{
			if (IsLDoNTrapped())
			{
				MessageString(Chat::Red, LDON_ACCIDENT_SETOFF2);
				SpellFinished(GetLDoNTrapSpellID(), other, EQ::spells::CastingSlot::Item, 0, -1, spells[GetLDoNTrapSpellID()].resist_difficulty, false);
				SetLDoNTrapSpellID(0);
				SetLDoNTrapped(false);
				SetLDoNTrapDetected(false);
			}
		}
	}

	//do a majority of the work...
	CommonDamage(other, damage, spell_id, attack_skill, avoidable, buffslot, iBuffTic, special);
}

bool NPC::Death(Mob* killer_mob, int64 damage, uint16 spell, EQ::skills::SkillType attack_skill, KilledByTypes killed_by, bool is_buff_tic)
{
	LogCombat(
		"Fatal blow dealt by [{}] with [{}] damage, spell [{}], skill [{}]",
		(killer_mob ? killer_mob->GetName() : "[nullptr]"),
		damage,
		spell,
		attack_skill
	);

	Mob* owner_or_self = killer_mob ? killer_mob->GetOwnerOrSelf() : nullptr;

	auto exports = [&]() {
		return fmt::format(
			"{} {} {} {}",
			killer_mob ? killer_mob->GetID() : 0,
			damage,
			spell,
			static_cast<int>(attack_skill)
		);
	};

	if (parse->EventBotMercNPC(EVENT_DEATH, this, owner_or_self, exports) != 0) {
		if (GetHP() < 0) {
			SetHP(0);
		}

		return false;
	}

	if (m_resumed_from_zone_suspend && !IsQueuedForCorpse()) {
		LogInfo("NPC [{}] is resumed from zone suspend, cannot kill until zone resume is complete.", GetCleanName());
		SetHP(0);
		return false;
	}

	if (IsMultiQuestEnabled()) {
		for (auto &i: m_hand_in.items) {
			if (i.is_multiquest_item && i.item->GetItem()->NoDrop != 0) {
				auto lde = LootdropEntriesRepository::NewNpcEntity();
				lde.equip_item   = 0;
				lde.item_charges = i.item->GetCharges();
				AddLootDrop(i.item->GetItem(), lde, true);
			}
		}
	}

	if (killer_mob && killer_mob->IsOfClientBot() && IsValidSpell(spell) && damage > 0) {
		char val1[20] = { 0 };

		entity_list.MessageCloseString(
			this, /* Sender */
			false, /* Skip Sender */
			RuleI(Range, DamageMessages),
			Chat::NonMelee, /* 283 */
			HIT_NON_MELEE, /* %1 hit %2 for %3 points of non-melee damage. */
			killer_mob->GetCleanName(), /* Message1 */
			GetCleanName(), /* Message2 */
			ConvertArray(damage, val1) /* Message3 */
		);
	}

	if (IsEngaged()) {
		zone->DelAggroMob();
		LogAttackDetail("{} Mob{} currently aggroed.", zone->MobsAggroCount(), zone->MobsAggroCount() != 1 ? "s" : "");
	}

	ShieldAbilityClearVariables();

	SetHP(0);
	SetPet(0);

	if (GetSwarmOwner()) {
		Mob* owner = entity_list.GetMobID(GetSwarmOwner());
		if (owner) {
			owner->SetTempPetCount(owner->GetTempPetCount() - 1);
		}
	}

	auto* killer = GetHateDamageTop(this);

	entity_list.RemoveFromTargets(this, p_depop);

	if (p_depop) {
		return false;
	}

	const int illusion_spell_id = spellbonuses.Illusion;

	HasAISpellEffects = false;

	BuffFadeAll();

	const uint8 killed_level = GetLevel();

	if (GetClass() == Class::LDoNTreasure) { // open chest
		static EQApplicationPacket p(OP_Animation, sizeof(Animation_Struct));
		auto a = (Animation_Struct*) p.pBuffer;
		a->spawnid = GetID();
		a->action  = 0x0F;
		a->speed   = 10;
		entity_list.QueueCloseClients(this, &p);
	}

	auto app = new EQApplicationPacket(OP_Death, sizeof(Death_Struct));

	auto d = (Death_Struct*) app->pBuffer;

	// Convert last message to color to avoid duplicate damage messages
	// that occur in these rare cases when this is the death blow.
	if (IsValidSpell(spell) &&
		(attack_skill == EQ::skills::SkillTigerClaw ||
        (IsDamageSpell(spell) && IsDiscipline(spell)) ||
		!is_buff_tic)) {
			d->attack_skill = DamageTypeSpell;
			d->spell_id = (is_buff_tic) ? UINT32_MAX : spell;
	}
	else {
		d->attack_skill = SkillDamageTypes[attack_skill];
		d->spell_id = UINT32_MAX;
	}

	d->spawn_id     = GetID();
	d->killer_id    = killer_mob ? killer_mob->GetID() : 0;
	d->bindzoneid   = 0;
	d->damage       = damage;
	d->corpseid		= GetID();

	app->priority = 1;

	entity_list.QueueClients(killer_mob, app, false);

	safe_delete(app);

	if (respawn2) {
		respawn2->DeathReset(1);
	}

	if (killer_mob && GetClass() != Class::LDoNTreasure) {
		hate_list.AddEntToHateList(killer_mob, damage);
	}

	Mob* give_exp = hate_list.GetDamageTopOnHateList(this);

	if (give_exp) {
		give_exp = killer;
	}

	if (give_exp && give_exp->HasOwner()) {
		auto owner = give_exp->GetOwner();

		if (owner) {
			Mob* ulimate_owner = give_exp->GetUltimateOwner();
			bool pet_owner_is_client = give_exp->IsPet() && owner->IsClient();
			bool pet_owner_is_bot = give_exp->IsPet() && owner->IsBot();
			bool owner_is_client = owner->IsClient();

			bool is_in_same_group_or_raid = (
				pet_owner_is_client ||
				(pet_owner_is_bot && owner->IsInGroupOrRaid(ulimate_owner)) ||
				(owner_is_client && give_exp->IsInGroupOrRaid(ulimate_owner))
			);

			give_exp = (is_in_same_group_or_raid ? give_exp->GetUltimateOwner() : nullptr);
		}
		else {
			give_exp = nullptr;
		}
	}

	if (give_exp && give_exp->IsTempPet() && give_exp->IsPetOwnerOfClientBot()) {
		if (give_exp->IsNPC() && give_exp->CastToNPC()->GetSwarmOwner()) {
			Mob* temp_owner = entity_list.GetMobID(give_exp->CastToNPC()->GetSwarmOwner());
			if (temp_owner) {
				give_exp = temp_owner;
			}
		}
	}

	int player_count = 0; // QueryServ Player Counting

	Client* give_exp_client = nullptr;
	if (give_exp && give_exp->IsClient()) {
		give_exp_client = give_exp->CastToClient();
	}

	//do faction hits even if we are a merchant, so long as a player killed us
	if (!IsCharmed() && give_exp_client && !RuleB(NPC, EnableMeritBasedFaction)) {
		hate_list.DoFactionHits(GetNPCFactionID(), GetPrimaryFaction(), GetFactionAmount());
	}

	const bool is_ldon_treasure = GetClass() == Class::LDoNTreasure;

	if (give_exp_client && !IsCorpse()) {
		Group* killer_group = entity_list.GetGroupByClient(give_exp_client);
		Raid*  killer_raid  = entity_list.GetRaidByClient(give_exp_client);

		int64 final_exp = give_exp_client->GetExperienceForKill(this);

		// handle task credit on behalf of the killer
		if (RuleB(TaskSystem, EnableTaskSystem)) {
			LogTasksDetail(
				"Triggering HandleUpdateTasksOnKill for [{}] npc [{}]",
				give_exp_client->GetCleanName(),
				GetNPCTypeID()
			);
			TaskManager::Instance()->HandleUpdateTasksOnKill(give_exp_client, this);
		}

		if (killer_raid) {
			if (!is_ldon_treasure && MerchantType == 0) {
				killer_raid->SplitExp(ExpSource::Kill, final_exp, this);

				if (
					killer_mob &&
					(
						killer_raid->IsRaidMember(killer_mob->GetName()) ||
						killer_raid->IsRaidMember(killer_mob->GetUltimateOwner()->GetName())
					)
				) {
					killer_mob->TrySpellOnKill(killed_level, spell);
				}
			}

			/* Send the EVENT_KILLED_MERIT event for all raid members */
			for (const auto& m : killer_raid->members) {
				if (m.is_bot) {
					continue;
				}

				if (m.member) {
					if (RuleB(NPC, EnableMeritBasedFaction)) {
						m.member->SetFactionLevel(
							m.member->CharacterID(),
							GetNPCFactionID(),
							m.member->GetBaseClass(),
							m.member->GetBaseRace(),
							m.member->GetDeity()
						);
					}

					player_count++;
				}
			}
		} else if (give_exp_client->IsGrouped() && killer_group) {
			if (!is_ldon_treasure && MerchantType == 0) {
					killer_group->SplitExp(ExpSource::Kill, final_exp, this);

				if (
					killer_mob &&
					(
						killer_group->IsGroupMember(killer_mob->GetName()) ||
						killer_group->IsGroupMember(killer_mob->GetUltimateOwner()->GetName())
					)
				) {
					killer_mob->TrySpellOnKill(killed_level, spell);
				}
			}

			/* Update kill tasks for all group members */
			for (const auto& m : killer_group->members) {
				if (m && m->IsClient()) {
					Client* c = m->CastToClient();

					if (RuleB(NPC, EnableMeritBasedFaction)) {
						c->SetFactionLevel(
							c->CharacterID(),
							GetNPCFactionID(),
							c->GetBaseClass(),
							c->GetBaseRace(),
							c->GetDeity()
						);
					}

					player_count++;
				}
			}
		} else {
			if (!is_ldon_treasure && !MerchantType) {
				const uint32 con_level = give_exp->GetLevelCon(GetLevel());

				if (con_level != ConsiderColor::Gray) {
					if (!GetOwner() || (GetOwner() && !GetOwner()->IsOfClientBot())) {
						give_exp_client->AddEXP(ExpSource::Kill, final_exp, con_level, false, this);

						if (
							killer_mob &&
							(
								killer_mob->GetID() == give_exp_client->GetID() ||
								killer_mob->GetUltimateOwner()->GetID() == give_exp_client->GetID()
							)
						) {
							killer_mob->TrySpellOnKill(killed_level, spell);
						}
					}
				}
			}

			if (RuleB(NPC, EnableMeritBasedFaction)) {
				give_exp_client->SetFactionLevel(
					give_exp_client->CharacterID(),
					GetNPCFactionID(),
					give_exp_client->GetBaseClass(),
					give_exp_client->GetBaseRace(),
					give_exp_client->GetDeity()
				);
			}
		}
	}

	const bool allow_merchant_corpse = RuleB(Merchant, AllowCorpse);
	const bool is_merchant           = (class_ == Class::Merchant || class_ == Class::AdventureMerchant || MerchantType != 0);

	Corpse* corpse = nullptr;

	const uint16 entity_id = GetID();

	if (
		(
			!HasOwner() &&
			!IsMerc() &&
			!GetSwarmInfo() &&
			(!is_merchant || allow_merchant_corpse) &&
			(
				(
					killer &&
					(
						killer->IsClient() ||
						(
							killer->HasOwner() &&
							killer->GetUltimateOwner()->IsClient()
						) ||
						(
							killer->IsNPC() &&
							killer->CastToNPC()->GetSwarmInfo() &&
							killer->CastToNPC()->GetSwarmInfo()->GetOwner() &&
							killer->CastToNPC()->GetSwarmInfo()->GetOwner()->IsClient()
						)
					)
				) ||
				(
					killer_mob && is_ldon_treasure
				)
			)
		)
		|| IsQueuedForCorpse()
		) {
		if (killer) {
			if (killer->GetOwner() != 0 && killer->GetOwner()->IsClient()) {
				killer = killer->GetOwner();
			}

			if (killer->IsClient() && !killer->CastToClient()->GetGM()) {
				CheckTrivialMinMaxLevelDrop(killer);
			}
		}

		entity_list.RemoveFromAutoXTargets(this);

		corpse = new Corpse(
			this,
			&m_loot_items,
			GetNPCTypeID(),
			&NPCTypedata,
			(
				level > 54 ?
				RuleI(NPC, MajorNPCCorpseDecayTime) :
				RuleI(NPC, MinorNPCCorpseDecayTime)
			)
		);

		if (killer_mob && emoteid) {
			DoNPCEmote(EQ::constants::EmoteEventTypes::AfterDeath, emoteid, killer_mob);
		}

		entity_list.LimitRemoveNPC(this);

		entity_list.AddCorpse(corpse, GetID());

		// The client sees NPC corpses as name's_corpse.  The server uses
		// name`s_corpse so that %T works on corpses (client workaround)
		// Rename the new corpse on client side.
		std::string old_name = Strings::Replace(corpse->GetName(), "`s_corpse", "'s_corpse");
		SendRename(killer_mob, old_name.c_str(), corpse->GetName());

		entity_list.UnMarkNPC(GetID());
		entity_list.RemoveNPC(GetID());

		m_close_mobs.clear();
		SetID(0);
		ApplyIllusionToCorpse(illusion_spell_id, corpse);

		if (killer && killer->IsClient()) {
			corpse->AllowPlayerLoot(killer, 0);
			if (killer->IsGrouped()) {
				Group* g = entity_list.GetGroupByClient(killer->CastToClient());
				if (g) {
					uint8 slot_id = 0;

					for (const auto &m : g->members) {
						if (m) {
							corpse->AllowPlayerLoot(m, slot_id);
						}

						slot_id++;
					}
				}
			} else if (killer->IsRaidGrouped()) {
				Raid* r = entity_list.GetRaidByClient(killer->CastToClient());
				if (r) {
					uint8 slot_id = 0;

					for (const auto &m : r->members) {
						if (m.is_bot) {
							continue;
						}

						switch (r->GetLootType()) {
							case RaidLootType::LeaderOnly:
								if (m.member && m.is_raid_leader) {
									corpse->AllowPlayerLoot(m.member, slot_id);
									slot_id++;
								}
								break;
							case RaidLootType::LeaderAndGroupLeadersOnly:
								if (m.member && (m.is_raid_leader || m.is_group_leader)) {
									corpse->AllowPlayerLoot(m.member, slot_id);
									slot_id++;
								}
								break;
							case RaidLootType::LeaderSelected:
								if (m.member && m.is_looter) {
									corpse->AllowPlayerLoot(m.member, slot_id);
									slot_id++;
								}
								break;
							case RaidLootType::EntireRaid:
							default:
								if (m.member) {
									corpse->AllowPlayerLoot(m.member, slot_id);
									slot_id++;
								}
								break;
						}
					}
				}
			}
		} else if (killer_mob && is_ldon_treasure) {
			Mob* ultimate_owner = killer_mob->GetUltimateOwner();
			if (ultimate_owner->IsClient()) {
				corpse->AllowPlayerLoot(ultimate_owner, 0);
			}
		}

		if (zone && zone->adv_data) {
			auto sr = (ServerZoneAdventureDataReply_Struct *) zone->adv_data;
			if (sr->type == Adventure_Kill) {
				zone->DoAdventureCountIncrease();
			} else if (sr->type == Adventure_Assassinate) {
				if (sr->data_id == GetNPCTypeID()) {
					zone->DoAdventureCountIncrease();
				} else {
					zone->DoAdventureAssassinationCountIncrease();
				}
			}
		}
	} else {
		entity_list.RemoveFromXTargets(this);
	}

	if (IsNPC()) {
		if (emoteid) {
			DoNPCEmote(EQ::constants::EmoteEventTypes::OnDeath, emoteid, killer_mob);
		}
	}

	if (owner_or_self) {
		if (owner_or_self->IsNPC()) {
			if (parse->HasQuestSub(owner_or_self->GetNPCTypeID(), EVENT_NPC_SLAY)) {
				parse->EventNPC(EVENT_NPC_SLAY, owner_or_self->CastToNPC(), this, "", 0);
			}

			const uint32 emote_id = owner_or_self->GetEmoteID();
			if (emote_id) {
				owner_or_self->CastToNPC()->DoNPCEmote(EQ::constants::EmoteEventTypes::KilledNPC, emote_id, this);
			}

			if (killer_mob) {
				killer_mob->TrySpellOnKill(killed_level, spell);
			}
		}
	}

	if (killer_mob) {
		parse->EventBotMerc(EVENT_NPC_SLAY, killer_mob, this);

		killer_mob->TrySpellOnKill(killed_level, spell);
	}

	WipeHateList();

	p_depop = true;

	if (killer_mob && killer_mob->GetTarget() == this) { // We can kill things without having them targeted
		killer_mob->SetTarget(nullptr);
	}

	entity_list.UpdateFindableNPCState(this, true);

	m_combat_record.Stop();

	if (give_exp_client && !IsCorpse()) {
		const auto& v = give_exp_client->GetRaidOrGroupOrSelf(true);
		for (const auto& m : v) {
			m->CastToClient()->RecordKilledNPCEvent(this);

			if (parse->HasQuestSub(GetNPCTypeID(), EVENT_KILLED_MERIT)) {
				parse->EventNPC(EVENT_KILLED_MERIT, this, m, "killed", 0);
			}
		}
	}

	std::vector<std::any> args = { corpse };

	parse->EventMercNPC(EVENT_DEATH_COMPLETE, this, owner_or_self,
		[&]() {
			return fmt::format(
				"{} {} {} {} {} {} {} {} {}",
				killer_mob ? killer_mob->GetID() : 0,
				damage,
				spell,
				static_cast<int>(attack_skill),
				entity_id,
				m_combat_record.GetStartTime(),
				m_combat_record.GetEndTime(),
				m_combat_record.GetDamageReceived(),
				m_combat_record.GetHealingReceived()
			);
		},
		0, &args
	);

	// Zone controller process EVENT_DEATH_ZONE (Death events)
	if (parse->HasQuestSub(ZONE_CONTROLLER_NPC_ID, EVENT_DEATH_ZONE)) {
		const auto& export_string = fmt::format(
			"{} {} {} {} {} {} {} {} {}",
			killer_mob ? killer_mob->GetID() : 0,
			damage,
			spell,
			static_cast<int>(attack_skill),
			entity_id,
			m_combat_record.GetStartTime(),
			m_combat_record.GetEndTime(),
			m_combat_record.GetDamageReceived(),
			m_combat_record.GetHealingReceived()
		);

		std::vector<std::any> args = { corpse, this };

		DispatchZoneControllerEvent(EVENT_DEATH_ZONE, owner_or_self, export_string, 0, &args);
	}

	if (parse->ZoneHasQuestSub(EVENT_DEATH_ZONE)) {
		const auto& export_string = fmt::format(
			"{} {} {} {} {} {} {} {} {}",
			killer_mob ? killer_mob->GetID() : 0,
			damage,
			spell,
			static_cast<int>(attack_skill),
			entity_id,
			m_combat_record.GetStartTime(),
			m_combat_record.GetEndTime(),
			m_combat_record.GetDamageReceived(),
			m_combat_record.GetHealingReceived()
		);

		std::vector<std::any> args = { corpse, this, owner_or_self };

		parse->EventZone(EVENT_DEATH_ZONE, zone, export_string, 0, &args);
	}

	return true;
}

void Mob::AddToHateList(Mob* other, int64 hate /*= 0*/, int64 damage /*= 0*/, bool iYellForHelp /*= true*/, bool bFrenzy /*= false*/, bool iBuffTic /*= false*/, uint16 spell_id, bool pet_command)
{
	if (!other)
		return;

	if (other->IsDestroying())
		return;

	if (other == this)
		return;

	if (other->IsClient() && (other->CastToClient()->IsZoning() || other->CastToClient()->Connected() == false))
		return;

	if (other->IsTrap())
		return;

	if (damage < 0) {
		hate = 1;
	}

	if (iYellForHelp)
		SetPrimaryAggro(true);
	else
		SetAssistAggro(true);

	bool was_engaged = IsEngaged();
	Mob* owner = other->GetOwner();
	Mob* my_pet = GetPet();
	Mob* my_owner = GetOwner();
	Mob* target_mob = GetTarget();
	bool on_hatelist = CheckAggro(other);

	AddRampage(other);
	if (on_hatelist) { // odd reason, if you're not on the hate list, subtlety etc don't apply!
		// Spell Casting Subtlety etc
		int64 hatemod = 100 + other->spellbonuses.hatemod + other->itembonuses.hatemod + other->aabonuses.hatemod;

		if (hatemod < 1) {
			hatemod = 1;
		}
		hate = ((hate * (hatemod)) / 100);
	} else {
		if (IsCharmed()){
			hate += RuleI(Aggro, InitialPetAggroBonus);
		} else {
			hate += RuleI(Aggro, InitialAggroBonus);
		}
	}

	// Pet that is /pet hold on will not add to their hate list if they're not engaged
	// Pet that is /pet hold on and /pet focus on will not add others to their hate list
	// Pet that is /pet ghold on will never add to their hate list unless /pet attack or /pet qattack

	// we skip these checks if it's forced through a pet command
	if (!pet_command) {
		if (IsPet()) {
			if ((IsGHeld() || (IsHeld() && IsFocused())) && !on_hatelist) // we want them to be able to climb the hate list
				return;
			if ((IsHeld() || IsPetStop() || IsPetRegroup()) && !was_engaged) // not 100% sure on stop/regroup kind of hard to test, but regroup is like "classic hold"
				return;
		}
	}

	if (other->IsNPC() && (other->IsPet() || other->CastToNPC()->GetSwarmOwner() > 0)) {
		TryTriggerOnCastRequirement();
	}

	if (IsClient() && !IsAIControlled()) {
		return;
	}

	if (IsFamiliar() || GetSpecialAbility(SpecialAbility::AggroImmunity)) {
		return;
	}

	if (other->IsBot() && GetSpecialAbility(SpecialAbility::BotAggroImmunity)) {
		return;
	}

	if (other->IsClient() && GetSpecialAbility(SpecialAbility::ClientAggroImmunity)) {
		return;
	}

	if (other->IsNPC() && GetSpecialAbility(SpecialAbility::NPCAggroImmunity)) {
		return;
	}

	if (IsValidSpell(spell_id) && IsNoDetrimentalSpellAggroSpell(spell_id)) {
		return;
	}

	if (other == my_owner) {
		return;
	}

	if (other->GetSpecialAbility(SpecialAbility::BeingAggroImmunity)) {
		return;
	}

	if (GetSpecialAbility(SpecialAbility::TunnelVision)) {
		int tv_mod = GetSpecialAbilityParam(SpecialAbility::TunnelVision, 0);

		Mob *top = GetTarget();
		if (top && top != other) {
			if (tv_mod) {
				float tv = tv_mod / 100.0f;
				hate *= tv;
			}
			else {
				hate *= RuleR(Aggro, TunnelVisionAggroMod);
			}
		}
	}
	// first add self

	// The damage on the hate list is used to award XP to the killer. This check is to prevent Killstealing.
	// e.g. Mob has 5000 hit points, Player A melees it down to 500 hp, Player B executes a headshot (10000 damage).
	// If we add 10000 damage, Player B would get the kill credit, so we only award damage credit to player B of the
	// amount of HP the mob had left.
	//
	if (damage > GetHP())
		damage = GetHP();

	if (spellbonuses.ImprovedTaunt[SBIndex::IMPROVED_TAUNT_AGGRO_MOD] && (GetLevel() < spellbonuses.ImprovedTaunt[SBIndex::IMPROVED_TAUNT_MAX_LVL])
		&& other && (buffs[spellbonuses.ImprovedTaunt[SBIndex::IMPROVED_TAUNT_BUFFSLOT]].casterid != other->GetID()))
		hate = (hate*spellbonuses.ImprovedTaunt[SBIndex::IMPROVED_TAUNT_AGGRO_MOD]) / 100;

	hate_list.AddEntToHateList(other, hate, damage, bFrenzy, !iBuffTic);

	if (other->IsClient() && !on_hatelist && !IsOnFeignMemory(other))
		other->CastToClient()->AddAutoXTarget(this);

	// if other is a bot, add the bots client to the hate list
	if (RuleB(Bots, Enabled)) {
		 if (other->IsBot()) {
			auto other_ = other->CastToBot();

			if (!other_ || !other_->GetBotOwner()) {
				return;
			}

			auto owner_ = other_->GetBotOwner()->CastToClient();
			if (!owner_ || owner_->IsDead() ||
				!owner_->InZone()) { // added isdead and inzone checks to avoid issues in AddAutoXTarget(...) below
				return;
			}

			if (owner_->GetFeigned()) {
				AddFeignMemory(owner_);
			}
			else if (!hate_list.IsEntOnHateList(owner_)) {
				hate_list.AddEntToHateList(owner_, 0, 0, false, true);
				owner_->AddAutoXTarget(this); // this was being called on dead/out-of-zone clients
			}
		}
	}

	// if other is a merc, add the merc client to the hate list
	if (other->IsMerc()) {
		if (other->CastToMerc()->GetMercenaryOwner() && other->CastToMerc()->GetMercenaryOwner()->CastToClient()->GetFeigned()) {
			AddFeignMemory(other->CastToMerc()->GetMercenaryOwner()->CastToClient());
		}
		else {
			if (!hate_list.IsEntOnHateList(other->CastToMerc()->GetMercenaryOwner()))
				hate_list.AddEntToHateList(other->CastToMerc()->GetMercenaryOwner(), 0, 0, false, true);
			// if mercs are reworked to include adding 'this' to owner's xtarget list, this should reflect bots code above
		}
	} //MERC

	//if I am a pet, then add pet owner if there's one
	if (owner) { // Other is a pet, add him and it
				 // EverHood 6/12/06
				 // Can't add a feigned owner to hate list
		if (owner->IsClient() && owner->CastToClient()->GetFeigned()) {
			//they avoid hate due to feign death...
		}
		else {
			// cb:2007-08-17
			// owner must get on list, but he's not actually gained any hate yet
			if (
				!owner->GetSpecialAbility(SpecialAbility::AggroImmunity) &&
				!(owner->IsBot() && GetSpecialAbility(SpecialAbility::BotAggroImmunity)) &&
				!(owner->IsClient() && GetSpecialAbility(SpecialAbility::ClientAggroImmunity)) &&
				!(owner->IsNPC() && GetSpecialAbility(SpecialAbility::NPCAggroImmunity))
			) {
				if (owner->IsClient() && !CheckAggro(owner)) {
					owner->CastToClient()->AddAutoXTarget(this);
				}
				hate_list.AddEntToHateList(owner, 0, 0, false, !iBuffTic);
			}
		}
	}

	if (my_pet) {
		bool aggro_immunity 		= my_pet->GetSpecialAbility(SpecialAbility::AggroImmunity);
		bool bot_aggro_immunity 	= IsBot() && my_pet->GetSpecialAbility(SpecialAbility::BotAggroImmunity);
		bool client_aggro_immunity 	= IsClient() && my_pet->GetSpecialAbility(SpecialAbility::ClientAggroImmunity);
		bool npc_aggro_immunity 	= IsNPC() && my_pet->GetSpecialAbility(SpecialAbility::NPCAggroImmunity);
		bool can_add_to_hatelist 	= !my_pet->IsFamiliar() &&
			!aggro_immunity &&
			!bot_aggro_immunity &&
			!client_aggro_immunity &&
			!npc_aggro_immunity;

		if (can_add_to_hatelist) {
			bool bot_with_controllable_pet = IsBot() && CastToBot()->HasControllablePet(BotAnimEmpathy::Attack);

			if (!IsBot() || bot_with_controllable_pet) {
				my_pet->hate_list.AddEntToHateList(other, 0, 0, bFrenzy);
			}
		}
	}
	else if (my_owner) { // I am a pet, add other to owner if it's NPC/LD
		if (my_owner->IsAIControlled()) {
			bool aggro_immunity = my_owner->GetSpecialAbility(SpecialAbility::AggroImmunity);
			bool bot_aggro_immunity = my_owner->IsBot() && GetSpecialAbility(SpecialAbility::BotAggroImmunity);
			bool client_aggro_immunity = my_owner->IsClient() && GetSpecialAbility(SpecialAbility::ClientAggroImmunity);
			bool npc_aggro_immunity = my_owner->IsNPC() && GetSpecialAbility(SpecialAbility::NPCAggroImmunity);
			bool can_add_to_hatelist = !aggro_immunity &&
				!bot_aggro_immunity &&
				!client_aggro_immunity &&
				!npc_aggro_immunity;

			if (can_add_to_hatelist) {
				my_owner->hate_list.AddEntToHateList(other, 0, 0, bFrenzy);
			}
		}
	}

	//I have a swarm pet, add other to it.
	if (GetTempPetCount()) {
		entity_list.AddTempPetsToHateList(this, other, bFrenzy);
	}

	if (!was_engaged) {
		if (IsNPC() && other->IsClient() && other->CastToClient()) {
			if (parse->HasQuestSub(GetNPCTypeID(), EVENT_AGGRO)) {
				parse->EventNPC(EVENT_AGGRO, CastToNPC(), other, "", 0);
			}
		}

		AI_Event_Engaged(other, iYellForHelp);
	}
}

// this is called from Damage() when 'this' is attacked by 'other.
// 'this' is the one being attacked
// 'other' is the attacker
// a damage shield causes damage (or healing) to whoever attacks the wearer
// a reverse ds causes damage to the wearer whenever it attack someone
// given this, a reverse ds must be checked each time the wearer is attacking
// and not when they're attacked
//a damage shield on a spell is a negative value but on an item it's a positive value so add the spell value and subtract the item value to get the end ds value
void Mob::DamageShield(Mob* attacker, bool spell_ds) {

	if (!attacker || this == attacker)
		return;

	int DS = 0;
	int rev_ds = 0;
	uint16 spellid = 0;

	if (!spell_ds)
	{
		DS = spellbonuses.DamageShield;
		rev_ds = attacker->spellbonuses.ReverseDamageShield;

		if (IsValidSpell(spellbonuses.DamageShieldSpellID)) {
			spellid = spellbonuses.DamageShieldSpellID;
		}
	}
	else {
		DS = spellbonuses.SpellDamageShield + itembonuses.SpellDamageShield + aabonuses.SpellDamageShield;
		rev_ds = 0;
		// This ID returns "you are burned", seemed most appropriate for spell DS
		spellid = 2166;
		/*
			Live Message - not yet used on emu
			Feedback onto you "YOUR mind burns from TARGETS NAME's feedback for %i points of non-melee damage."
			Feedback onto other "TARGETS NAME's mind burns from YOUR feedback for %i points of non-melee damage."
		*/
	}

	if (DS == 0 && rev_ds == 0)
		return;

	LogCombat("Applying Damage Shield of value [{}] to [{}]", DS, attacker->GetName());

	//invert DS... spells yield negative values for a true damage shield
	if (DS < 0) {
		if (!spell_ds) {

			DS += aabonuses.DamageShield; //Live AA - coat of thistles. (negative value)
			DS -= itembonuses.DamageShield; //+Damage Shield should only work when you already have a DS spell
			DS -= attacker->aabonuses.DS_Mitigation_Amount + attacker->itembonuses.DS_Mitigation_Amount + attacker->spellbonuses.DS_Mitigation_Amount; //Negative value to reduce
			//Do not allow flat amount reductions to reduce past 0.
			if (DS >= 0)
				return;

											//Spell data for damage shield mitigation shows a negative value for spells for clients and positive
											//value for spells that effect pets. Unclear as to why. For now will convert all positive to be consistent.
			if (attacker->IsOffHandAtk()) {
				int32 mitigation = attacker->itembonuses.DSMitigationOffHand +
					attacker->spellbonuses.DSMitigationOffHand +
					attacker->aabonuses.DSMitigationOffHand;
				DS -= DS*mitigation / 100;
			}

			int ds_mitigation = attacker->itembonuses.DSMitigation;
			// Subtract mitigations because DS_Mitigation_Percentage is a negative value when reducing total, thus final value will be positive
			ds_mitigation -= attacker->aabonuses.DS_Mitigation_Percentage + attacker->itembonuses.DS_Mitigation_Percentage + attacker->spellbonuses.DS_Mitigation_Percentage; //Negative value to reduce

			DS -= DS * ds_mitigation / 100;
		}

		attacker->Damage(this, -DS, spellid, EQ::skills::SkillAbjuration/*hackish*/, false);
		//we can assume there is a spell now

		static EQApplicationPacket p(OP_Damage, sizeof(CombatDamage_Struct));
		auto                       b = (CombatDamage_Struct *) p.pBuffer;
		b->target  = attacker->GetID();
		b->source  = GetID();
		b->type    = spellbonuses.DamageShieldType;
		b->spellid = 0x0;
		b->damage  = DS;
		entity_list.QueueCloseClients(this, &p);
	}
	else if (DS > 0 && !spell_ds) {
		//we are healing the attacker...
		attacker->HealDamage(DS);
		//TODO: send a packet???
	}

	//Reverse DS
	//this is basically a DS, but the spell is on the attacker, not the attackee
	//if we've gotten to this point, we know we know "attacker" hit "this" (us) for damage & we aren't invulnerable
	uint16 rev_ds_spell_id = SPELL_UNKNOWN;

	if (IsValidSpell(spellbonuses.ReverseDamageShieldSpellID)) {
		rev_ds_spell_id = spellbonuses.ReverseDamageShieldSpellID;
	}

	if (rev_ds < 0) {
		LogCombat("Applying Reverse Damage Shield of value [{}] to [{}]", rev_ds, attacker->GetName());
		attacker->Damage(this, -rev_ds, rev_ds_spell_id, EQ::skills::SkillAbjuration/*hackish*/, false); //"this" (us) will get the hate, etc. not sure how this works on Live, but it'll works for now, and tanks will love us for this
																											//do we need to send a damage packet here also?
	}
}

uint8 Mob::GetWeaponDamageBonus(const EQ::ItemData *weapon, bool offhand)
{
	// dev quote with old and new formulas
	// https://web.archive.org/web/20250816013618/https://forums.everquest.com/index.php?threads/test-update-09-17-15.226618/page-5#post-3326194
	//
	// We assume that the level check is done before calling this function and sinister strikes is checked before
	// calling for offhand DB
	auto level = GetLevel();

	if (!weapon) {
		return 1 + ((level - 28) / 3); // how does weaponless scale?
	}

	auto delay = weapon->Delay;
	if (weapon->IsType1HWeapon() || weapon->ItemType == EQ::item::ItemTypeMartial) {
		// we assume sinister strikes is checked before calling here
		if (!offhand) {
			if (delay <= 39) {
				return 1 + ((level - 28) / 3);
			} else if (delay < 43) {
				return 2 + ((level - 28) / 3) + ((delay - 40) / 3);
			} else if (delay < 45) {
				return 3 + ((level - 28) / 3) + ((delay - 40) / 3);
			} else if (delay >= 45) {
				return 4 + ((level - 28) / 3) + ((delay - 40) / 3);
			}
		} else {
			if (RuleB(Skills, UseAltSinisterStrikeFormula)) {
				if (delay <= 19) {
					return 5 + ((level - 40) / 3) * (delay / 30);
				} else if (delay <= 23) {
					return 6 + ((level - 40) / 3) * (delay / 30);
				} else if (delay >= 24) {
					return 7 + ((level - 40) / 3) * (delay / 30);
				}
			} else {
				return 1 + ((level - 40) / 3) * (delay / 30); // YOOO shit's useless waste of AAs
			}
		}
	} else {
		// 2h damage bonus
		int64 damage_bonus = 1 + (level - 28) / 3;

		if (delay <= 27) {
			return damage_bonus + 1;
		}

		// Client isn't reflecting what the dev quoted, this matches better
		if (level > 29) {
			int level_bonus = (level - 30) / 5 + 1;
			if (level > 50) {
				level_bonus++;
				int level_bonus2 = level - 50;
				if (level > 67) {
					level_bonus2 += 5;
				} else if (level > 59) {
					level_bonus2 += 4;
				} else if (level > 58) {
					level_bonus2 += 3;
				} else if (level > 56) {
					level_bonus2 += 2;
				} else if (level > 54) {
					level_bonus2++;
				}
				level_bonus += level_bonus2 * delay / 40;
			}
			damage_bonus += level_bonus;
		}
		if (delay >= 40) {
			int delay_bonus = (delay - 40) / 3 + 1;
			if (delay >= 45) {
				delay_bonus += 2;
			} else if (delay >= 43) {
				delay_bonus++;
			}
			damage_bonus += delay_bonus;
		}
		return damage_bonus;
	}

	return 0;
}

int Mob::GetHandToHandDamage(void)
{
	if (RuleB(Combat, UseRevampHandToHand)) {
		// everyone uses this in the revamp!
		int skill = GetSkill(EQ::skills::SkillHandtoHand);
		int epic = 0;
		if (IsClient() && CastToClient()->GetItemIDAt(12) == 10652 && GetLevel() > 46)
			epic = 280;
		if (epic > skill)
			skill = epic;
		return skill / 15 + 3;
	}

	static uint8 mnk_dmg[] = { 99,
		4, 4, 4, 4, 5, 5, 5, 5, 5, 6,           // 1-10
		6, 6, 6, 6, 7, 7, 7, 7, 7, 8,           // 11-20
		8, 8, 8, 8, 9, 9, 9, 9, 9, 10,          // 21-30
		10, 10, 10, 10, 11, 11, 11, 11, 11, 12, // 31-40
		12, 12, 12, 12, 13, 13, 13, 13, 13, 14, // 41-50
		14, 14, 14, 14, 14, 14, 14, 14, 14, 14, // 51-60
		14, 14 };                                // 61-62
	static uint8 bst_dmg[] = { 99,
		4, 4, 4, 4, 4, 5, 5, 5, 5, 5,        // 1-10
		5, 6, 6, 6, 6, 6, 6, 7, 7, 7,        // 11-20
		7, 7, 7, 8, 8, 8, 8, 8, 8, 9,        // 21-30
		9, 9, 9, 9, 9, 10, 10, 10, 10, 10,   // 31-40
		10, 11, 11, 11, 11, 11, 11, 12, 12 }; // 41-49
	if (GetClass() == Class::Monk) {
		if (IsClient() && CastToClient()->GetItemIDAt(12) == 10652 && GetLevel() > 50)
			return 9;
		if (level > 62)
			return 15;
		return mnk_dmg[level];
	}
	else if (GetClass() == Class::Beastlord) {
		if (level > 49)
			return 13;
		return bst_dmg[level];
	}
	return 2;
}

int Mob::GetHandToHandDelay(void)
{
	if (RuleB(Combat, UseRevampHandToHand)) {
		// everyone uses this in the revamp!
		int skill = GetSkill(EQ::skills::SkillHandtoHand);
		int epic = 0;
		int iksar = 0;
		if (IsClient() && CastToClient()->GetItemIDAt(12) == 10652 && GetLevel() > 46)
			epic = 280;
		else if (GetRace() == Race::Iksar)
			iksar = 1;
		// the delay bonus from the monk epic scales up to a skill of 280
		if (epic >= skill)
			epic = skill;
		return iksar - epic / 21 + 38;
	}

	int delay = 35;
	static uint8 mnk_hum_delay[] = { 99,
		35, 35, 35, 35, 35, 35, 35, 35, 35, 35, // 1-10
		35, 35, 35, 35, 35, 35, 35, 35, 35, 35, // 11-20
		35, 35, 35, 35, 35, 35, 35, 34, 34, 34, // 21-30
		34, 33, 33, 33, 33, 32, 32, 32, 32, 31, // 31-40
		31, 31, 31, 30, 30, 30, 30, 29, 29, 29, // 41-50
		29, 28, 28, 28, 28, 27, 27, 27, 27, 26, // 51-60
		24, 22 };                                // 61-62
	static uint8 mnk_iks_delay[] = { 99,
		35, 35, 35, 35, 35, 35, 35, 35, 35, 35, // 1-10
		35, 35, 35, 35, 35, 35, 35, 35, 35, 35, // 11-20
		35, 35, 35, 35, 35, 35, 35, 35, 35, 34, // 21-30
		34, 34, 34, 34, 34, 33, 33, 33, 33, 33, // 31-40
		33, 32, 32, 32, 32, 32, 32, 31, 31, 31, // 41-50
		31, 31, 31, 30, 30, 30, 30, 30, 30, 29, // 51-60
		25, 23 };                                // 61-62
	static uint8 bst_delay[] = { 99,
		35, 35, 35, 35, 35, 35, 35, 35, 35, 35, // 1-10
		35, 35, 35, 35, 35, 35, 35, 35, 35, 35, // 11-20
		35, 35, 35, 35, 35, 35, 35, 35, 34, 34, // 21-30
		34, 34, 34, 33, 33, 33, 33, 33, 32, 32, // 31-40
		32, 32, 32, 31, 31, 31, 31, 31, 30, 30, // 41-50
		30, 30, 30, 29, 29, 29, 29, 29, 28, 28, // 51-60
		28, 28, 28, 27, 27, 27, 27, 27, 26, 26, // 61-70
		26, 26, 26 };                            // 71-73

	if (GetClass() == Class::Monk) {
		// Have a look to see if we have epic fists on
		if (IsClient() && CastToClient()->GetItemIDAt(12) == 10652 && GetLevel() > 50)
			return 16;
		int level = GetLevel();
		if (level > 62)
			return GetRace() == Race::Iksar ? 21 : 20;
		return GetRace() == Race::Iksar ? mnk_iks_delay[level] : mnk_hum_delay[level];
	}
	else if (GetClass() == Class::Beastlord) {
		int level = GetLevel();
		if (level > 73)
			return 25;
		return bst_delay[level];
	}
	return 35;
}

int64 Mob::ReduceDamage(int64 damage)
{
	if (damage <= 0)
		return damage;

	int32 slot = -1;
	bool DisableMeleeRune = false;

	if (spellbonuses.NegateAttacks[SBIndex::NEGATE_ATK_EXISTS]) {
		slot = spellbonuses.NegateAttacks[SBIndex::NEGATE_ATK_BUFFSLOT];
		if (slot >= 0) {
			if (--buffs[slot].hit_number == 0) {

				if (!TryFadeEffect(slot))
					BuffFadeBySlot(slot, true);
			}

			if (spellbonuses.NegateAttacks[SBIndex::NEGATE_ATK_MAX_DMG_ABSORB_PER_HIT] && (damage > spellbonuses.NegateAttacks[SBIndex::NEGATE_ATK_MAX_DMG_ABSORB_PER_HIT]))
				damage -= spellbonuses.NegateAttacks[SBIndex::NEGATE_ATK_MAX_DMG_ABSORB_PER_HIT];
			else
				return DMG_RUNE;
		}
	}

	//Only mitigate if damage is above the minimium specified.
	if (spellbonuses.MeleeThresholdGuard[SBIndex::THRESHOLDGUARD_MITIGATION_PERCENT]) {
		slot = spellbonuses.MeleeThresholdGuard[SBIndex::THRESHOLDGUARD_BUFFSLOT];

		if (slot >= 0 && (damage > spellbonuses.MeleeThresholdGuard[SBIndex::THRESHOLDGUARD_MIN_DMG_TO_TRIGGER]))
		{
			DisableMeleeRune = true;
			int64 damage_to_reduce = damage * spellbonuses.MeleeThresholdGuard[SBIndex::THRESHOLDGUARD_MITIGATION_PERCENT] / 100;
			if (damage_to_reduce >= buffs[slot].melee_rune)
			{
				LogSpellsDetail("SE_MeleeThresholdGuard [{}] damage negated, [{}] damage remaining, fading buff", damage_to_reduce, buffs[slot].melee_rune);
				damage -= buffs[slot].melee_rune;
				if (!TryFadeEffect(slot))
					BuffFadeBySlot(slot);
			}
			else
			{
				LogSpellsDetail("SE_MeleeThresholdGuard [{}] damage negated, [{}] damage remaining", damage_to_reduce, buffs[slot].melee_rune);
				buffs[slot].melee_rune = (buffs[slot].melee_rune - damage_to_reduce);
				damage -= damage_to_reduce;
			}
		}
	}

	if (spellbonuses.MitigateMeleeRune[SBIndex::MITIGATION_RUNE_PERCENT] && !DisableMeleeRune) {
		slot = spellbonuses.MitigateMeleeRune[SBIndex::MITIGATION_RUNE_BUFFSLOT];
		if (slot >= 0)
		{
			int64 damage_to_reduce = damage * spellbonuses.MitigateMeleeRune[SBIndex::MITIGATION_RUNE_PERCENT] / 100;

			if (spellbonuses.MitigateMeleeRune[SBIndex::MITIGATION_RUNE_MAX_DMG_ABSORB_PER_HIT] && (damage_to_reduce > spellbonuses.MitigateMeleeRune[SBIndex::MITIGATION_RUNE_MAX_DMG_ABSORB_PER_HIT]))
				damage_to_reduce = spellbonuses.MitigateMeleeRune[SBIndex::MITIGATION_RUNE_MAX_DMG_ABSORB_PER_HIT];

			if (spellbonuses.MitigateMeleeRune[SBIndex::MITIGATION_RUNE_MAX_HP_AMT] && (damage_to_reduce >= buffs[slot].melee_rune))
			{
				LogSpellsDetail("SE_MitigateMeleeDamage [{}] damage negated, [{}] damage remaining, fading buff", damage_to_reduce, buffs[slot].melee_rune);
				damage -= buffs[slot].melee_rune;
				if (!TryFadeEffect(slot))
					BuffFadeBySlot(slot);
			}
			else
			{
				LogSpellsDetail("SE_MitigateMeleeDamage [{}] damage negated, [{}] damage remaining", damage_to_reduce, buffs[slot].melee_rune);

				if (spellbonuses.MitigateMeleeRune[SBIndex::MITIGATION_RUNE_MAX_HP_AMT])
					buffs[slot].melee_rune = (buffs[slot].melee_rune - damage_to_reduce);

				damage -= damage_to_reduce;
			}
		}
	}

	if (damage < 1)
		return DMG_RUNE;

	if (spellbonuses.MeleeRune[SBIndex::RUNE_AMOUNT] && spellbonuses.MeleeRune[SBIndex::RUNE_BUFFSLOT] >= 0)
		damage = RuneAbsorb(damage, SpellEffect::Rune);

	if (damage < 1)
		return DMG_RUNE;

	return(damage);
}

int64 Mob::AffectMagicalDamage(int64 damage, uint16 spell_id, const bool iBuffTic, Mob* attacker)
{
	if (damage <= 0)
		return damage;

	bool DisableSpellRune = false;
	int32 slot = -1;

	// See if we block the spell outright first
	if (!iBuffTic && spellbonuses.NegateAttacks[SBIndex::NEGATE_ATK_EXISTS]) {
		slot = spellbonuses.NegateAttacks[SBIndex::NEGATE_ATK_BUFFSLOT];
		if (slot >= 0) {
			if (--buffs[slot].hit_number == 0) {

				if (!TryFadeEffect(slot))
					BuffFadeBySlot(slot, true);
			}

			if (spellbonuses.NegateAttacks[SBIndex::NEGATE_ATK_MAX_DMG_ABSORB_PER_HIT] && (damage > spellbonuses.NegateAttacks[SBIndex::NEGATE_ATK_MAX_DMG_ABSORB_PER_HIT]))
				damage -= spellbonuses.NegateAttacks[SBIndex::NEGATE_ATK_MAX_DMG_ABSORB_PER_HIT];
			else
				return 0;
		}
	}

	// If this is a DoT, use DoT Shielding...
	if (iBuffTic) {
		int total_dotshielding = itembonuses.DoTShielding + itembonuses.MitigateDotRune[SBIndex::MITIGATION_RUNE_PERCENT] + aabonuses.MitigateDotRune[SBIndex::MITIGATION_RUNE_PERCENT];
		damage -= (damage * total_dotshielding / 100);

		if (spellbonuses.MitigateDotRune[SBIndex::MITIGATION_RUNE_PERCENT]) {
			slot = spellbonuses.MitigateDotRune[SBIndex::MITIGATION_RUNE_BUFFSLOT];
			if (slot >= 0)
			{
				int64 damage_to_reduce = damage * spellbonuses.MitigateDotRune[SBIndex::MITIGATION_RUNE_PERCENT] / 100;

				if (spellbonuses.MitigateDotRune[SBIndex::MITIGATION_RUNE_MAX_DMG_ABSORB_PER_HIT] && (damage_to_reduce > spellbonuses.MitigateDotRune[SBIndex::MITIGATION_RUNE_MAX_DMG_ABSORB_PER_HIT]))
					damage_to_reduce = spellbonuses.MitigateDotRune[SBIndex::MITIGATION_RUNE_MAX_DMG_ABSORB_PER_HIT];

				if (spellbonuses.MitigateDotRune[SBIndex::MITIGATION_RUNE_MAX_HP_AMT] && (damage_to_reduce >= buffs[slot].dot_rune))
				{
					damage -= buffs[slot].dot_rune;
					if (!TryFadeEffect(slot))
						BuffFadeBySlot(slot);
				}
				else
				{
					if (spellbonuses.MitigateDotRune[SBIndex::MITIGATION_RUNE_MAX_HP_AMT])
						buffs[slot].dot_rune = (buffs[slot].dot_rune - damage_to_reduce);

					damage -= damage_to_reduce;
				}
			}
		}
	}

	// This must be a DD then so lets apply Spell Shielding and runes.
	else
	{
		// Reduce damage by the Spell Shielding first so that the runes don't take the raw damage.
		int total_spellshielding = itembonuses.SpellShield + itembonuses.MitigateSpellRune[SBIndex::MITIGATION_RUNE_PERCENT] + aabonuses.MitigateSpellRune[SBIndex::MITIGATION_RUNE_PERCENT];
		damage -= (damage * total_spellshielding / 100);

		//Only mitigate if damage is above the minimium specified.
		if (spellbonuses.SpellThresholdGuard[SBIndex::THRESHOLDGUARD_MITIGATION_PERCENT]) {
			slot = spellbonuses.SpellThresholdGuard[SBIndex::THRESHOLDGUARD_BUFFSLOT];

			if (slot >= 0 && (damage > spellbonuses.MeleeThresholdGuard[SBIndex::THRESHOLDGUARD_MIN_DMG_TO_TRIGGER]))
			{
				DisableSpellRune = true;
				int64 damage_to_reduce = damage * spellbonuses.SpellThresholdGuard[SBIndex::THRESHOLDGUARD_MITIGATION_PERCENT] / 100;
				if (damage_to_reduce >= buffs[slot].magic_rune)
				{
					damage -= buffs[slot].magic_rune;
					if (!TryFadeEffect(slot))
						BuffFadeBySlot(slot);
				}
				else
				{
					buffs[slot].melee_rune = (buffs[slot].magic_rune - damage_to_reduce);
					damage -= damage_to_reduce;
				}
			}
		}

		// Do runes now.
		if (spellbonuses.MitigateSpellRune[SBIndex::MITIGATION_RUNE_PERCENT] && !DisableSpellRune) {
			slot = spellbonuses.MitigateSpellRune[SBIndex::MITIGATION_RUNE_BUFFSLOT];
			if (slot >= 0)
			{
				int64 damage_to_reduce = damage * spellbonuses.MitigateSpellRune[SBIndex::MITIGATION_RUNE_PERCENT] / 100;

				if (spellbonuses.MitigateSpellRune[SBIndex::MITIGATION_RUNE_MAX_DMG_ABSORB_PER_HIT] && (damage_to_reduce > spellbonuses.MitigateSpellRune[SBIndex::MITIGATION_RUNE_MAX_DMG_ABSORB_PER_HIT]))
					damage_to_reduce = spellbonuses.MitigateSpellRune[SBIndex::MITIGATION_RUNE_MAX_DMG_ABSORB_PER_HIT];

				if (spellbonuses.MitigateSpellRune[SBIndex::MITIGATION_RUNE_MAX_HP_AMT] && (damage_to_reduce >= buffs[slot].magic_rune))
				{
					LogSpellsDetail("SE_MitigateSpellDamage [{}] damage negated, [{}] damage remaining, fading buff", damage_to_reduce, buffs[slot].magic_rune);
					damage -= buffs[slot].magic_rune;
					if (!TryFadeEffect(slot))
						BuffFadeBySlot(slot);
				}
				else
				{
					LogSpellsDetail("SE_MitigateMeleeDamage [{}] damage negated, [{}] damage remaining", damage_to_reduce, buffs[slot].magic_rune);

					if (spellbonuses.MitigateSpellRune[SBIndex::MITIGATION_RUNE_MAX_HP_AMT])
						buffs[slot].magic_rune = (buffs[slot].magic_rune - damage_to_reduce);

					damage -= damage_to_reduce;
				}
			}
		}

		if (damage < 1)
			return 0;

		//Regular runes absorb spell damage (except dots) - Confirmed on live.
		if (spellbonuses.MeleeRune[SBIndex::RUNE_AMOUNT] && spellbonuses.MeleeRune[SBIndex::RUNE_BUFFSLOT] >= 0)
			damage = RuneAbsorb(damage, SpellEffect::Rune);

		if (spellbonuses.AbsorbMagicAtt[SBIndex::RUNE_AMOUNT] && spellbonuses.AbsorbMagicAtt[SBIndex::RUNE_BUFFSLOT] >= 0)
			damage = RuneAbsorb(damage, SpellEffect::AbsorbMagicAtt);

		if (damage < 1)
			return 0;
	}
	return damage;
}

int64 Mob::ReduceAllDamage(int64 damage)
{
	if (damage <= 0)
		return damage;

	if (spellbonuses.ManaAbsorbPercentDamage) {
		int64 mana_reduced = damage * spellbonuses.ManaAbsorbPercentDamage / 100;
		if (GetMana() >= mana_reduced) {
			damage -= mana_reduced;
			SetMana(GetMana() - mana_reduced);
			TryTriggerOnCastRequirement();
		}
	}

	if (spellbonuses.EnduranceAbsorbPercentDamage[SBIndex::ENDURANCE_ABSORD_MITIGIATION]) {
		int64 damage_reduced = damage * spellbonuses.EnduranceAbsorbPercentDamage[SBIndex::ENDURANCE_ABSORD_MITIGIATION] / 10000; //If hit for 1000, at 10% then lower damage by 100;
		int32 endurance_drain = damage_reduced * spellbonuses.EnduranceAbsorbPercentDamage[SBIndex::ENDURANCE_ABSORD_DRAIN_PER_HP] / 10000; //Reduce endurance by 0.05% per HP loss
		if (endurance_drain < 1)
			endurance_drain = 1;

		if (IsClient() && CastToClient()->GetEndurance() >= endurance_drain) {
			damage -= damage_reduced;
			CastToClient()->SetEndurance(CastToClient()->GetEndurance() - endurance_drain);
			TryTriggerOnCastRequirement();
		}
	}

	CheckNumHitsRemaining(NumHit::IncomingDamage);

	return(damage);
}

bool Mob::HasProcs() const
{
	for (int i = 0; i < m_max_procs; i++) {
		if (IsValidSpell(PermaProcs[i].spellID) || IsValidSpell(SpellProcs[i].spellID)) {
			return true;
		}
	}

	if (IsOfClientBot()) {
		for (int i = 0; i < MAX_AA_PROCS; i += 4) {
			if (aabonuses.SpellProc[i]) {
				return true;
			}
		}
	}
	return false;
}

bool Mob::HasDefensiveProcs() const
{
	for (int i = 0; i < m_max_procs; i++) {
		if (IsValidSpell(DefensiveProcs[i].spellID)) {
			return true;
		}
	}

	if (IsOfClientBot()) {
		for (int i = 0; i < MAX_AA_PROCS; i += 4) {
			if (aabonuses.DefensiveProc[i]) {
				return true;
			}
		}
	}
	return false;
}

bool Mob::HasSkillProcs() const
{
	for (int i = 0; i < MAX_SKILL_PROCS; i++) {
		if (spellbonuses.SkillProc[i] || itembonuses.SkillProc[i] || aabonuses.SkillProc[i])
			return true;
	}
	return false;
}

bool Mob::HasSkillProcSuccess() const
{
	for (int i = 0; i < MAX_SKILL_PROCS; i++) {
		if (spellbonuses.SkillProcSuccess[i] || itembonuses.SkillProcSuccess[i] || aabonuses.SkillProcSuccess[i])
			return true;
	}
	return false;
}

bool Mob::HasRangedProcs() const
{
	for (int i = 0; i < m_max_procs; i++){
		if (IsValidSpell(RangedProcs[i].spellID)) {
			return true;
		}
	}

	if (IsOfClientBot()) {
		for (int i = 0; i < MAX_AA_PROCS; i += 4) {
			if (aabonuses.RangedProc[i]) {
				return true;
			}
		}
	}
	return false;
}

bool Client::CheckDoubleAttack()
{
	//Check for bonuses that give you a double attack chance regardless of skill (ie Bestial Frenzy/Harmonious Attack AA)
	uint32 bonus_give_double_attack = aabonuses.GiveDoubleAttack + spellbonuses.GiveDoubleAttack + itembonuses.GiveDoubleAttack;

	if (!HasSkill(EQ::skills::SkillDoubleAttack) && !bonus_give_double_attack) {
		return false;
	}

	float chance = 0.0f;
	uint16 skill = GetSkill(EQ::skills::SkillDoubleAttack);

	int32 bonus_double_attack = 0;
	if ((GetClass() == Class::Paladin || GetClass() == Class::ShadowKnight) && (!HasTwoHanderEquipped())) {
		LogCombatDetail("Knight class without a 2 hand weapon equipped = No DA Bonus!");
	} else {
		bonus_double_attack = aabonuses.DoubleAttackChance + spellbonuses.DoubleAttackChance + itembonuses.DoubleAttackChance;
	}

	//Use skill calculations otherwise, if you only have AA applied GiveDoubleAttack chance then use that value as the base.
	if (skill) {
		chance = (float(skill + GetLevel()) * (float(100.0f + bonus_double_attack + bonus_give_double_attack) / 100.0f)) / 500.0f;
	} else {
		chance = (float(bonus_give_double_attack + bonus_double_attack) / 100.0f);
	}

	LogCombatDetail(
		"skill [{}] bonus_give_double_attack [{}] bonus_double_attack [{}] chance [{}]",
		skill,
		bonus_give_double_attack,
		bonus_double_attack,
		chance
	);

	return zone->random.Roll(chance);
}

// Admittedly these parses were short, but this check worked for 3 toons across multiple levels
// with varying triple attack skill (1-3% error at least)
bool Client::CheckTripleAttack()
{
	int chance;

	if (RuleB(Combat, ClassicTripleAttack)) {
		if (
			IsClient() &&
			GetLevel() >= 60 &&
			(
				GetClass() == Class::Warrior ||
				GetClass() == Class::Ranger ||
				GetClass() == Class::Monk ||
				GetClass() == Class::Berserker
			)
		) {
			switch (GetClass()) {
				case Class::Warrior:
					chance = RuleI(Combat, ClassicTripleAttackChanceWarrior);
					break;
				case Class::Ranger:
					chance = RuleI(Combat, ClassicTripleAttackChanceRanger);
					break;
				case Class::Monk:
					chance = RuleI(Combat, ClassicTripleAttackChanceMonk);
					break;
				case Class::Berserker:
					chance = RuleI(Combat, ClassicTripleAttackChanceBerserker);
					break;
				default:
					break;
			}
		}
	} else {
		chance = GetSkill(EQ::skills::SkillTripleAttack);
	}

	if (chance < 1) {
		return false;
	}

	int inc = aabonuses.TripleAttackChance + spellbonuses.TripleAttackChance + itembonuses.TripleAttackChance;
	chance = static_cast<int>(chance * (1 + inc / 100.0f));
	chance = (chance * 100) / (chance + 800);

	return zone->random.Int(1, 100) <= chance;
}

bool Client::CheckDoubleRangedAttack() {
	int32 chance = spellbonuses.DoubleRangedAttack + itembonuses.DoubleRangedAttack + aabonuses.DoubleRangedAttack;

	if (chance && zone->random.Roll(chance))
		return true;

	return false;
}

bool Mob::CheckDoubleAttack()
{
	// Not 100% certain pets follow this or if it's just from pets not always
	// having the same skills as most mobs
	int chance = GetSkill(EQ::skills::SkillDoubleAttack);
	if (GetLevel() > 35)
		chance += GetLevel();

	int per_inc = aabonuses.DoubleAttackChance + spellbonuses.DoubleAttackChance + itembonuses.DoubleAttackChance;
	if (per_inc)
		chance += chance * per_inc / 100;

	return zone->random.Int(1, 500) <= chance;
}

void Mob::CommonDamage(Mob* attacker, int64 &damage, const uint16 spell_id, const EQ::skills::SkillType skill_used, bool &avoidable, const int8 buffslot, const bool iBuffTic, eSpecialAttacks special) {
#ifdef LUA_EQEMU
	int64 lua_ret = 0;
	bool ignore_default = false;
	lua_ret = LuaParser::Instance()->CommonDamage(this, attacker, damage, spell_id, static_cast<int>(skill_used), avoidable, buffslot, iBuffTic, static_cast<int>(special), ignore_default);
	if (ignore_default) {
		damage = lua_ret;
	}
#endif
	// This method is called with skill_used=ABJURE for Damage Shield damage.
	bool FromDamageShield = (skill_used == EQ::skills::SkillAbjuration);
	bool ignore_invul = false;
	if (IsValidSpell(spell_id))
		ignore_invul = spell_id == SPELL_CAZIC_TOUCH || spells[spell_id].cast_not_standing;

	if (!ignore_invul && (GetInvul() || DivineAura())) {
		LogCombat("Avoiding [{}] damage due to invulnerability", damage);
		damage = DMG_INVULNERABLE;
	}

	// this should actually happen MUCH sooner, need to investigate though -- good enough for now
	if ((skill_used == EQ::skills::SkillArchery || skill_used == EQ::skills::SkillThrowing) && GetSpecialAbility(SpecialAbility::RangedAttackImmunity)) {
		LogCombat("Avoiding [{}] damage due to SpecialAbility::RangedAttackImmunity", damage);
		damage = DMG_INVULNERABLE;
	}

	if (IsValidSpell(spell_id) || attacker == nullptr)
		avoidable = false;

	// only apply DS if physical damage (no spell damage)
	// damage shield calls this function with spell_id set, so its unavoidable
	if (attacker && damage > 0 && !IsValidSpell(spell_id) && skill_used != EQ::skills::SkillArchery && skill_used != EQ::skills::SkillThrowing) {
		DamageShield(attacker);
	}

	if (!IsValidSpell(spell_id) && skill_used >= EQ::skills::Skill1HBlunt) {
		CheckNumHitsRemaining(NumHit::IncomingHitAttempts);

		if (attacker)
			attacker->CheckNumHitsRemaining(NumHit::OutgoingHitAttempts);
	}

	if (attacker) {
		if (attacker->IsClient()) {
			if (!RuleB(Combat, EXPFromDmgShield)) {
				// Damage shield damage shouldn't count towards who gets EXP
				if (!attacker->CastToClient()->GetFeigned() && !FromDamageShield)
					AddToHateList(attacker, 0, damage, true, false, iBuffTic, spell_id);
			}
			else {
				if (!attacker->CastToClient()->GetFeigned())
					AddToHateList(attacker, 0, damage, true, false, iBuffTic, spell_id);
			}
		}
		else
			AddToHateList(attacker, 0, damage, true, false, iBuffTic, spell_id);
	}

	bool died = false;
	if (damage > 0) {
		//if there is some damage being done and theres an attacker involved
		int previous_hp_ratio = GetHPRatio();

		if (attacker) {
			// if spell is lifetap add hp to the caster
			if (IsValidSpell(spell_id) && IsLifetapSpell(spell_id)) {
				int64 healed = damage;

				healed = RuleB(Spells, CompoundLifetapHeals) ? attacker->GetActSpellHealing(spell_id, healed) : healed;
				LogCombat("Applying lifetap heal of [{}] to [{}]", healed, attacker->GetName());
				attacker->HealDamage(healed);

				//we used to do a message to the client, but its gone now.
				// emote goes with every one ... even npcs
				entity_list.FilteredMessageClose(this, false, RuleI(Range, SpellMessages), Chat::Emote, FilterSocials, "%s beams a smile at %s", attacker->GetCleanName(), GetCleanName());
			}

			// If a client pet is damaged while sitting, stand, fix sit button,
			// and remove sitting regen.  Removes bug where client clicks sit
			// during battle and gains pet hp-regen and bugs the sit button.
			if (IsPet()) {
				Mob *owner = GetOwner();
				if (owner && owner->IsClient()) {
					if (GetPetOrder() == PetOrder::Sit) {
						SetPetOrder(GetPreviousPetOrder());
					}
					// fix GUI sit button to be unpressed and stop sitting regen
					owner->CastToClient()->SetPetCommandState(PetButton::Sit, PetButtonState::Off);
					SetAppearance(eaStanding);
				}
			}

		}	//end `if there is some damage being done and theres anattacker person involved`

		// pets that have GHold will never automatically add NPCs
		// pets that have Hold and no Focus will add NPCs if they're engaged
		// pets that have Hold and Focus will not add NPCs
		if (
			Mob* pet = GetPet();
			pet &&
			!pet->IsFamiliar() &&
			!pet->GetSpecialAbility(SpecialAbility::AggroImmunity) &&
			!pet->IsEngaged() &&
			attacker &&
			!(attacker->IsBot() && pet->GetSpecialAbility(SpecialAbility::BotAggroImmunity)) &&
			!(attacker->IsClient() && pet->GetSpecialAbility(SpecialAbility::ClientAggroImmunity)) &&
			!(attacker->IsNPC() && pet->GetSpecialAbility(SpecialAbility::NPCAggroImmunity)) &&
			attacker != this &&
			!attacker->IsCorpse() &&
			!pet->IsGHeld() &&
			!attacker->IsTrap() &&
			!pet->IsHeld()
		) {
			LogAggro("Sending pet [{}] into battle due to attack", pet->GetName());
			if (IsClient() && !pet->IsPetStop()) {
				// if pet was sitting his new mode is previous setting of
				// follow or guard after the battle (live verified)
				if (pet->GetPetOrder() == PetOrder::Sit) {
					pet->SetPetOrder(pet->GetPreviousPetOrder());
				}

				// fix GUI sit button to be unpressed and stop sitting regen
				CastToClient()->SetPetCommandState(PetButton::Sit, PetButtonState::Off);
				pet->SetAppearance(eaStanding);
			}

			pet->AddToHateList(attacker, 1, 0, true, false, false, spell_id);
			pet->SetTarget(attacker);
			MessageString(Chat::NPCQuestSay, PET_ATTACKING, pet->GetCleanName(), attacker->GetCleanName());
		}

		if (GetTempPetCount()) {
			entity_list.AddTempPetsToHateListOnOwnerDamage(this, attacker, spell_id);
		}

		//see if any runes want to reduce this damage
		if (!IsValidSpell(spell_id)) {
			if (IsClient()) {
				CommonBreakInvisible();
			}

			damage = ReduceDamage(damage);
			LogCombat("Melee Damage reduced to [{}]", damage);
			damage = ReduceAllDamage(damage);
			TryTriggerThreshHold(damage, SpellEffect::TriggerMeleeThreshold, attacker);

			CheckNumHitsRemaining(NumHit::IncomingHitSuccess);
		}
		else {
			int64 origdmg = damage;
			damage = AffectMagicalDamage(damage, spell_id, iBuffTic, attacker);
			if (
				origdmg != damage &&
				attacker &&
				attacker->IsClient() &&
				attacker->CastToClient()->GetFilter(FilterDamageShields) != FilterHide
			) {
				attacker->Message(
					Chat::Yellow,
					"The Spellshield absorbed %d of %d points of damage",
					origdmg - damage,
					origdmg
				);
			}
			if (damage == 0 && attacker && origdmg != damage && IsClient()) {
				//Kayen: Probably need to add a filter for this - Not sure if this msg is correct but there should be a message for spell negate/runes.
				Message(263, "%s tries to cast on YOU, but YOUR magical skin absorbs the spell.", attacker->GetCleanName());
			}
			damage = ReduceAllDamage(damage);
			TryTriggerThreshHold(damage, SpellEffect::TriggerSpellThreshold, attacker);
		}

		if (IsClient() && CastToClient()->sneaking) {
			CastToClient()->sneaking = false;
			SendAppearancePacket(AppearanceType::Sneak, 0);
		}

		if (attacker && attacker->IsClient() && attacker->CastToClient()->sneaking) {
			attacker->CastToClient()->sneaking = false;
			attacker->SendAppearancePacket(AppearanceType::Sneak, 0);
		}

		//final damage has been determined.
		int old_hp_ratio = (int)GetHPRatio();


		std::vector<std::any> args;

		int64 damage_override = 0;

		if (attacker) {
			args = { this };

			parse->EventMob(EVENT_DAMAGE_GIVEN, attacker, this,
				[&]() {
					return fmt::format(
						"{} {} {} {} {} {} {} {} {}",
						GetID(),
						damage,
						spell_id,
						static_cast<int>(skill_used),
						FromDamageShield ? 1 : 0,
						avoidable ? 1 : 0,
						buffslot,
						iBuffTic ? 1 : 0,
						static_cast<int>(special)
					);
				},
				0, &args
			);
		}

		args = { attacker };

		damage_override = parse->EventMob(EVENT_DAMAGE_TAKEN, this, attacker,
			[&]() {
				return fmt::format(
					"{} {} {} {} {} {} {} {} {}",
					attacker ? attacker->GetID() : 0,
					damage,
					spell_id,
					static_cast<int>(skill_used),
					FromDamageShield ? 1 : 0,
					avoidable ? 1 : 0,
					buffslot,
					iBuffTic ? 1 : 0,
					static_cast<int>(special)
				);
			},
			0, &args
		);

		if (damage_override > 0) {
			damage = damage_override;
		} else if (damage_override < 0) {
			damage = 0;
		}

		SetHP(int64(GetHP() - damage));

		if (HasDied()) {
			bool IsSaved = false;

			if (TryDivineSave()) {
				IsSaved = true;
			}

			if (!IsSaved && !TrySpellOnDeath()) {
				if (IsNPC()) {
					died = !CastToNPC()->GetDepop();
				} else if (IsClient()) {
					died = CastToClient()->CheckIfAlreadyDead();
				}

				if (died) {
					SetHP(-500);
				}

				// killedByType is clarified in Client::Death if we are client.
				if (Death(attacker, damage, spell_id, skill_used, KilledByTypes::Killed_NPC, iBuffTic)) {
					return;
				}
			}
		}
		else {
			if (GetHPRatio() < 16 && previous_hp_ratio >= 16) {
				TryDeathSave();
			}
		}

		TryTriggerOnCastRequirement();

		//fade mez if we are mezzed
		if (IsMezzed() && attacker) {
			LogCombat("Breaking mez due to attack");
			entity_list.MessageCloseString(
				this, /* Sender */
				true,  /* Skip Sender */
				RuleI(Range, SpellMessages),
				Chat::SpellWornOff, /* 284 */
				HAS_BEEN_AWAKENED, // %1 has been awakened by %2.
				GetCleanName(), /* Message1 */
				attacker->GetCleanName() /* Message2 */
			);
			BuffFadeByEffect(SpellEffect::Mez);
		}

		// broken up for readability
		// This is based on what the client is doing
		// We had a bunch of stuff like BaseImmunityLevel checks, which I think is suppose to just be for spells
		// This is missing some merc checks, but those mostly just skipped the spell bonuses I think ...
		bool can_stun = false;
		int stunbash_chance = 0; // bonus
		if (attacker) {
			if (skill_used == EQ::skills::SkillBash) {
				can_stun = true;
				if (attacker->IsClient() || attacker->IsBot() || attacker->IsMerc()) {
					stunbash_chance = attacker->spellbonuses.StunBashChance +
					                  attacker->itembonuses.StunBashChance +
					                  attacker->aabonuses.StunBashChance;
				}
			}
			else if (skill_used == EQ::skills::SkillKick &&
					attacker->GetClass() == Class::Warrior) {
				int stun_level = RuleI(Combat, NPCKickStunLevel);
				if (attacker->IsClient()) {
					stun_level = RuleI(Combat, PCKickStunLevel);
				}
				can_stun = (attacker->GetLevel() >= stun_level);
			}

			bool is_immune_to_frontal_stun = false;

			if (IsOfClientBotMerc()) {
				if (
					IsPlayerClass(GetClass()) &&
					RuleI(Combat, FrontalStunImmunityClasses) & GetPlayerClassBit(GetClass())
				) {
					is_immune_to_frontal_stun = true;
				}


				if (
					(
						IsPlayerRace(GetBaseRace()) &&
						RuleI(Combat, FrontalStunImmunityRaces) & GetPlayerRaceBit(GetBaseRace())
					) ||
					GetBaseRace() == Race::OggokCitizen
				) {
					is_immune_to_frontal_stun = true;
				}
			} else if (IsNPC()) {
				if (
					RuleB(Combat, NPCsUseFrontalStunImmunityClasses) &&
					IsPlayerClass(GetClass()) &&
					RuleI(Combat, FrontalStunImmunityClasses) & GetPlayerClassBit(GetClass())
				) {
					is_immune_to_frontal_stun = true;
				}

				if (
					RuleB(Combat, NPCsUseFrontalStunImmunityRaces) &&
					(
						(
							IsPlayerRace(GetBaseRace()) &&
							RuleI(Combat, FrontalStunImmunityRaces) & GetPlayerRaceBit(GetBaseRace())
						) ||
						GetBaseRace() == Race::OggokCitizen
					)
				) {
					is_immune_to_frontal_stun = true;
				}
			}

			if (
				is_immune_to_frontal_stun &&
				!attacker->BehindMob(this, attacker->GetX(), attacker->GetY())
			) {
				can_stun = false;
			}

			if (GetSpecialAbility(SpecialAbility::StunImmunity)) {
				can_stun = false;
			}
		}
		if (can_stun) {
			int bashsave_roll = zone->random.Int(0, 100);
			if (bashsave_roll > 98 || bashsave_roll > (55 - stunbash_chance)) {
				// did stun -- roll other resists
				// SpellEffect::FrontalStunResist description says any angle now a days
				int stun_resist2 = spellbonuses.FrontalStunResist + itembonuses.FrontalStunResist +
					aabonuses.FrontalStunResist;
				if (zone->random.Int(1, 100) > stun_resist2) {
					// stun resist 2 failed
					// time to check SpellEffect::StunResist and mod2 stun resist
					int stun_resist =
						spellbonuses.StunResist + itembonuses.StunResist + aabonuses.StunResist;
					if (zone->random.Int(0, 100) >= stun_resist) {
						// did stun
						// nothing else to check!
						Stun(RuleI(Combat, StunDuration));
						if (RuleB(Combat, ClientStunMessage) && attacker->IsClient()) {
							if (attacker) {
								entity_list.FilteredMessageClose(
									this,
									true,
									RuleI(Range, StunMessages),
									Chat::Stun,
									FilterStuns,
									"%s is stunned after being bashed by %s.",
									GetCleanName(),
									attacker->GetCleanName()
								);
							} else {
								entity_list.FilteredMessageClose(
									this,
									true,
									RuleI(Range, StunMessages),
									Chat::Stun,
									FilterStuns,
									"%s is stunned by a bash to the head.",
									GetCleanName()
								);
							}
						}
					} else {
						// stun resist passed!
						if (IsClient()) {
							FilteredMessageString(
								this,
								Chat::Stun,
								FilterStuns,
								SHAKE_OFF_STUN
							);
						}
					}
				} else {
					// stun resist 2 passed!
					if (IsClient()) {
						FilteredMessageString(
							this,
							Chat::Stun,
							FilterStuns,
							AVOID_STUNNING_BLOW
						);
					}
				}
			} else {
				// main stun failed -- extra interrupt roll
				// these spells are excluded
				// 90% chance >< -- stun immune won't reach this branch though :(
				if (IsCasting() && !EQ::ValueWithin(casting_spell_id, 859, 1023)) {
					if (zone->random.Int(0, 9) > 1) {
						InterruptSpell();
					}
				}
			}
		}

		if (IsValidSpell(spell_id) && !iBuffTic) {
			//see if root will break
			if (IsRooted() && !FromDamageShield)  // neotoyko: only spells cancel root
				TryRootFadeByDamage(buffslot, attacker);
		}
		else if (!IsValidSpell(spell_id))
		{
			//increment chances of interrupting
			if (IsCasting()) { //shouldnt interrupt on regular spell damage
				attacked_count++;
				LogCombat("Melee attack while casting. Attack count [{}]", attacked_count);
			}
		}

		//send an HP update if we are hurt
		if(GetHP() < GetMaxHP())
		{
			// Don't send a HP update for melee damage unless we've damaged ourself.
			if (IsNPC()) {
				int cur_hp_ratio = (int)GetHPRatio();
				if (cur_hp_ratio != old_hp_ratio) {
					SendHPUpdate(true);
				}
			} else if (!iBuffTic || died)	{ // Let regen handle buff tics unless this tic killed us.
				SendHPUpdate(true);
			}

			if (!died && IsNPC()) {
				CheckFlee();
			}
		}
	}	//end `if damage was done`

		//send damage packet...
	if (!iBuffTic) { //buff ticks do not send damage, instead they just call SendHPUpdate(), which is done above
		static EQApplicationPacket p(OP_Damage, sizeof(CombatDamage_Struct));
		auto                       a = (CombatDamage_Struct *) p.pBuffer;
		a->target = GetID();

		if (!attacker) {
			a->source = 0;
		} else if (attacker->IsClient() && attacker->CastToClient()->GMHideMe()) {
			a->source = 0;
		} else {
			a->source = attacker->GetID();
		}

		a->type = (EQ::ValueWithin(skill_used, EQ::skills::Skill1HBlunt, EQ::skills::Skill2HPiercing)) ?
				SkillDamageTypes[skill_used] : SkillDamageTypes[EQ::skills::SkillHandtoHand]; // was 0x1c
		a->damage = damage;
		a->spellid = spell_id;

		if (special == eSpecialAttacks::AERampage) {
			a->special = 1;
		} else if (special == eSpecialAttacks::Rampage) {
			a->special = 2;
		} else {
			a->special = 0;
		}

		a->hit_heading = attacker ? attacker->GetHeading() : 0.0f;
		if (RuleB(Combat, MeleePush) && damage > 0 && !IsRooted() &&
			(IsClient() || zone->random.Roll(RuleI(Combat, MeleePushChance)))) {
			a->force = EQ::skills::GetSkillMeleePushForce(skill_used);

			if (RuleR(Combat, MeleePushForceClientPercent) && IsClient()) {
				a->force += a->force * RuleR(Combat, MeleePushForceClientPercent);
			}

			if (RuleR(Combat, MeleePushForcePetPercent) && IsPet()) {
				a->force += a->force * RuleR(Combat, MeleePushForcePetPercent);
			}

			if (IsNPC()) {
				if (!RuleB(Combat, NPCtoNPCPush) && attacker && attacker->IsNPC()) {
					a->force = 0.0f; // 2013 change that disabled NPC vs NPC push
				} else {
					a->force *= 0.10f; // force against NPCs is divided by 10 I guess? ex bash is 0.3, parsed 0.03 against an NPC
				}

				if (ForcedMovement == 0 && a->force != 0.0f && position_update_melee_push_timer.Check()) {
					m_Delta.x += a->force * g_Math.FastSin(a->hit_heading);
					m_Delta.y += a->force * g_Math.FastCos(a->hit_heading);
					ForcedMovement = 3;
				}
			}
		}

		//Note: if players can become pets, they will not receive damage messages of their own
		//this was done to simplify the code here (since we can only effectively skip one mob on queue)
		eqFilterType filter;
		Mob* skip = attacker;
		Mob* owner = attacker ? attacker->GetOwner() : nullptr;
		if (attacker && owner && !attacker->IsBot()) {
			//attacker is a pet, let pet owners see their pet's damage
			if (owner->IsClient()) {
				if (FromDamageShield && damage > 0) {
					//special crap for spell damage, looks hackish to me
					char val1[20] = { 0 };
					owner->MessageString(Chat::NonMelee, OTHER_HIT_NONMELEE, GetCleanName(), ConvertArray(damage, val1));
				}
				else {
					if (damage > 0) {
						if (IsValidSpell(spell_id)) {
							filter = iBuffTic ? FilterDOT : FilterSpellDamage;
						} else {
							filter = FilterPetHits;
						}
					} else if (damage == -5) {
						filter = FilterNone;    //cant filter invulnerable
					} else {
						filter = FilterPetMisses;
					}

					if (!FromDamageShield) {
						entity_list.QueueCloseClients(
							attacker, /* Sender */
							&p, /* packet */
							false, /* Skip Sender */
							((IsValidSpell(spell_id)) ? RuleI(Range, SpellMessages) : RuleI(Range, DamageMessages)),
							0, /* don't skip anyone on spell */
							true, /* Packet ACK */
							filter /* eqFilterType filter */
						);
					}
				}
			}

			skip = owner;
		}
		else {
			//attacker is not a pet, send to the attacker
			//if the attacker is a client, try them with the correct filter
			if (attacker && attacker->IsOfClientBot()) {
				if ((IsValidSpell(spell_id) || FromDamageShield) && damage > 0) {
					//special crap for spell damage, looks hackish to me
					char val1[20] = { 0 };
					if (FromDamageShield) {
						if (attacker->IsBot()) {
							Mob* owner = attacker->GetOwner();

							if (owner && owner->CastToClient()->GetFilter(FilterDamageShields) != FilterHide) {
								owner->MessageString(
									Chat::DamageShield,
									OTHER_HIT_NONMELEE,
									GetCleanName(),
									ConvertArray(damage, val1)
								);
							}
						} else {
							if (attacker->CastToClient()->GetFilter(FilterDamageShields) != FilterHide) {
								attacker->MessageString(
									Chat::DamageShield,
									OTHER_HIT_NONMELEE,
									GetCleanName(),
									ConvertArray(damage, val1)
								);
							}
						}
					}
					else {
						entity_list.FilteredMessageCloseString(
							attacker, /* Sender */
							false, /* Sender is attacker, so do not skip */
							RuleI(Range, SpellMessages),
							Chat::NonMelee, /* 283 */
							FilterSpellDamage, /* FilterType: 13 */
							HIT_NON_MELEE, /* %1 hit %2 for %3 points of non-melee damage. */
							0,
							attacker->GetCleanName(), /* Message1 */
							GetCleanName(), /* Message2 */
							ConvertArray(damage, val1) /* Message3 */
						);
					}
				}
				// Only try to queue these packets to a client
				else {
					if (damage > 0) {
						if (IsValidSpell(spell_id)) {
							filter = iBuffTic ? FilterDOT : FilterSpellDamage;
						}
						else {
							filter = FilterNone;    //cant filter our own hits
						}
					}
					else if (damage == -5)
						filter = FilterNone;	//cant filter invulnerable
					else
						filter = FilterMyMisses;

					if (attacker->IsClient()) {
						attacker->CastToClient()->QueuePacket(&p, true, CLIENT_CONNECTED, filter);
					} else {
						entity_list.QueueCloseClients(
							attacker, /* Sender */
							&p, /* packet */
							false, /* Skip Sender */
							(
								IsValidSpell(spell_id) ?
								RuleI(Range, SpellMessages) :
								RuleI(Range, DamageMessages)
							),
							0, /* don't skip anyone on spell */
							true, /* Packet ACK */
							filter /* eqFilterType filter */
						);
					}
				}
			}
		}

		//send damage to all clients around except the specified skip mob (attacker or the attacker's owner) and ourself
		if (damage > 0) {
			if (IsValidSpell(spell_id)) {
				filter = iBuffTic ? FilterDOT : FilterSpellDamage;
			}
			else {
				filter = FilterOthersHit;
			}
		}
		else if (damage == -5)
			filter = FilterNone;	//cant filter invulnerable
		else
			filter = FilterOthersMiss;
		//make attacker (the attacker) send the packet so we can skip them and the owner
		//this call will send the packet to `this` as well (using the wrong filter) (will not happen until PC charm works)
		// If this is Damage Shield damage, the correct OP_Damage packets will be sent from Mob::DamageShield, so
		// we don't send them here.
		if (!FromDamageShield) {

			// Determine message range based on spell/other-damage
			int range;
			if (IsValidSpell(spell_id)) {
				range = RuleI(Range, SpellMessages);
			}
			else {
				range = RuleI(Range, DamageMessages);
			}

			// If an "innate" spell, change to spell type to
			// produce a spell message.  Send to everyone.
			// This fixes issues with npc-procs like 1002 and 918 and
			// damage based disciplines which need to spit out extra spell color.
			if (IsValidSpell(spell_id) &&
				(skill_used == EQ::skills::SkillTigerClaw ||
				(IsDamageSpell(spell_id) && IsDiscipline(spell_id)))
				) {
				a->type = DamageTypeSpell;
				entity_list.QueueCloseClients(
					this, /* Sender */
					&p, /* packet */
					false, /* Skip Sender */
					range, /* distance packet travels at the speed of sound */
					0, /* don't skip anyone on spell */
					true, /* Packet ACK */
					filter /* eqFilterType filter */
					);
			}
			else {
				//I dont think any filters apply to damage affecting us
				if (IsClient()) {
					CastToClient()->QueuePacket(&p);
				}

				// Send normal message to observers
				// Exclude damage done by client pets as that's handled
				// elsewhere using proper "my pet damage filter"
				Mob *owner = attacker->GetOwner();
				if (!owner || (owner && !owner->IsClient())) {
					entity_list.QueueCloseClients(
						this, /* Sender */
						&p, /* packet */
						true, /* Skip Sender */
						range, /* distance packet travels at the speed of sound */
						(IsValidSpell(spell_id) && skill_used != EQ::skills::SkillTigerClaw) ? 0 : skip,
						true, /* Packet ACK */
						filter /* eqFilterType filter */
						);
				}
			}
		}
	}
	else {
		//else, it is a buff tic...
		// So we can see our dot dmg like live shows it.
		if (IsValidSpell(spell_id) && damage > 0 && attacker && attacker != this) {
			//might filter on (attack_skill>200 && attack_skill<250), but I dont think we need it
			if (!attacker->IsCorpse() && attacker->IsClient()) {
				attacker->FilteredMessageString(attacker, Chat::DotDamage,
					FilterDOT, YOUR_HIT_DOT, GetCleanName(), itoa(damage),
					spells[spell_id].name);
			}

			if (IsClient()) {
				FilteredMessageString(this, Chat::DotDamage, FilterDOT,
					YOU_TAKE_DOT, itoa(damage), attacker->GetCleanName(),
					spells[spell_id].name);
			}

			/* older clients don't have the below String ID, but it will be filtered */
			entity_list.FilteredMessageCloseString(
				this, /* Sender */
				true, /* Skip Sender */
				RuleI(Range, SpellMessages),
				Chat::DotDamage, /* Type: 325 */
				FilterDOT, /* FilterType: 19 */
				OTHER_HIT_DOT,  /* MessageFormat: %1 has taken %2 damage from %3 by %4. */
				attacker,		/* sent above */
				GetCleanName(), /* Message1 */
				itoa(damage), /* Message2 */
				attacker->GetCleanName(), /* Message3 */
				spells[spell_id].name /* Message4 */
			);
		}
	}
}

void Mob::HealDamage(uint64 amount, Mob* caster, uint16 spell_id)
{
#ifdef LUA_EQEMU
	uint64 lua_ret = 0;
	bool ignore_default = false;

	lua_ret = LuaParser::Instance()->HealDamage(this, caster, amount, spell_id, ignore_default);
	if (ignore_default) {
		amount = lua_ret;
	}
#endif
	int64 maxhp = GetMaxHP();
	int64 curhp = GetHP();
	uint64 acthealed = 0;

	if (amount > (maxhp - curhp))
		acthealed = (maxhp - curhp);
	else
		acthealed = amount;

	if (acthealed > RuleI(Spells, HealAmountMessageFilterThreshold)) {
		if (caster) {
			if (IsBuffSpell(spell_id)) { // hots
				// message to caster
				if ((caster->IsClient() && caster == this)) {
					if (caster->CastToClient()->ClientVersionBit() & EQ::versions::maskSoFAndLater) {
						FilteredMessageString(caster, Chat::NonMelee, FilterHealOverTime,
							HOT_HEAL_SELF, itoa(acthealed), spells[spell_id].name);
					}
					else
						FilteredMessageString(caster, Chat::NonMelee, FilterHealOverTime,
							YOU_HEALED, GetCleanName(), itoa(acthealed));
				}
				else if ((caster->IsClient() && caster != this)) {
					if (caster->CastToClient()->ClientVersionBit() & EQ::versions::maskSoFAndLater)
						caster->FilteredMessageString(caster, Chat::NonMelee, FilterHealOverTime,
							HOT_HEAL_OTHER, GetCleanName(), itoa(acthealed),
							spells[spell_id].name);
					else
						caster->FilteredMessageString(caster, Chat::NonMelee, FilterHealOverTime,
							YOU_HEAL, GetCleanName(), itoa(acthealed));
				}

				// message to target
				if (IsClient() && caster != this) {
					if (CastToClient()->ClientVersionBit() & EQ::versions::maskSoFAndLater)
						FilteredMessageString(caster, Chat::NonMelee, FilterHealOverTime,
							HOT_HEALED_OTHER, caster->GetCleanName(),
							itoa(acthealed), spells[spell_id].name);
					else
						FilteredMessageString(this, Chat::NonMelee, FilterHealOverTime,
							YOU_HEALED, caster->GetCleanName(), itoa(acthealed));
				}
			}
			else { // normal heals
				// Message to caster
				if (caster->IsClient()) {
					caster->FilteredMessageString(caster, Chat::NonMelee,
						FilterSpellDamage, YOU_HEAL, GetCleanName(),
						itoa(acthealed));
					}

				// Message to target
				if (IsClient() && caster != this) {
					FilteredMessageString(caster, Chat::NonMelee,
					FilterSpellDamage, YOU_HEALED, caster->GetCleanName(),
					itoa(acthealed));
				}
			}
		} else if (
			CastToClient()->GetFilter(FilterHealOverTime) != FilterShowSelfOnly ||
			CastToClient()->GetFilter(FilterHealOverTime) != FilterHide
		) {
			Message(Chat::NonMelee, "You have been healed for %d points of damage.", acthealed);
		}
	}

	if (curhp < maxhp) {
		if ((curhp + amount) > maxhp) {
			curhp = maxhp;
		}
		else {
			curhp += amount;
		}
		SetHP(curhp);

		SendHPUpdate();
	}
}

//proc chance includes proc bonus
float Mob::GetProcChances(float ProcBonus, uint16 hand)
{
	int mydex = GetDEX();
	float ProcChance = 0.0f;

	uint32 weapon_speed = GetWeaponSpeedbyHand(hand);

	if (RuleB(Combat, AdjustProcPerMinute)) {
		ProcChance = (static_cast<float>(weapon_speed) *
			RuleR(Combat, AvgProcsPerMinute) / 60000.0f); // compensate for weapon_speed being in ms
		ProcBonus += static_cast<float>(mydex) * RuleR(Combat, ProcPerMinDexContrib);
		ProcChance += ProcChance * ProcBonus / 100.0f;
	}
	else {
		ProcChance = RuleR(Combat, BaseProcChance) +
			static_cast<float>(mydex) / RuleR(Combat, ProcDexDivideBy);
		ProcChance += ProcChance * ProcBonus / 100.0f;
	}

	LogCombat("Proc chance [{}] ([{}] from bonuses)", ProcChance, ProcBonus);
	return ProcChance;
}

float Mob::GetDefensiveProcChances(float &ProcBonus, float &ProcChance, uint16 hand, Mob* on) {

	if (!on)
		return ProcChance;

	int myagi = on->GetAGI();
	ProcBonus = 0;
	ProcChance = 0;

	uint32 weapon_speed = GetWeaponSpeedbyHand(hand);

	ProcChance = (static_cast<float>(weapon_speed) * RuleR(Combat, AvgDefProcsPerMinute) / 60000.0f); // compensate for weapon_speed being in ms
	ProcBonus += static_cast<float>(myagi) * RuleR(Combat, DefProcPerMinAgiContrib) / 100.0f;
	ProcChance = ProcChance + (ProcChance * ProcBonus);

	LogCombat("Defensive Proc chance [{}] ([{}] from bonuses)", ProcChance, ProcBonus);
	return ProcChance;
}

// argument 'weapon' not used
void Mob::TryDefensiveProc(Mob *on, uint16 hand)
{
	if (!on) {
		SetTarget(nullptr);
		LogError("A null Mob object was passed to Mob::TryDefensiveProc for evaluation!");
		return;
	}

	if (!HasDefensiveProcs()) {
		return;
	}

	if (!on->HasDied() && on->GetHP() > 0) {
		float proc_chance, proc_bonus;
		on->GetDefensiveProcChances(proc_bonus, proc_chance, hand, this);

		if (hand == EQ::invslot::slotSecondary) {
			proc_chance /= 2;
		}

		int level_penalty     = 0;
		int level_diff        = GetLevel() - on->GetLevel();
		int penalty_level_gap = RuleI(Spells, DefensiveProcPenaltyLevelGap);
		if (
			penalty_level_gap >= 0 &&
			level_diff > penalty_level_gap
		) {//10% penalty per level if > penalty_level_gap levels over target.
			level_penalty = (level_diff - penalty_level_gap) * RuleR(Spells, DefensiveProcPenaltyLevelGapModifier);
		}

		proc_chance -= proc_chance * level_penalty / 100;

		if (proc_chance < 0) {
			return;
		}

		//Spell Procs and Quest added procs
		for (int i = 0; i < m_max_procs; i++) {
			if (IsValidSpell(DefensiveProcs[i].spellID)) {
				if (!IsProcLimitTimerActive(DefensiveProcs[i].base_spellID, DefensiveProcs[i].proc_reuse_time, ProcType::DEFENSIVE_PROC)) {
					float chance = proc_chance * (static_cast<float>(DefensiveProcs[i].chance) / 100.0f);
					if (zone->random.Roll(chance)) {
						ExecWeaponProc(nullptr, DefensiveProcs[i].spellID, on);
						CheckNumHitsRemaining(NumHit::DefensiveSpellProcs, 0, DefensiveProcs[i].base_spellID);
						SetProcLimitTimer(DefensiveProcs[i].base_spellID, DefensiveProcs[i].proc_reuse_time, ProcType::DEFENSIVE_PROC);
					}
				}
			}
		}

		//AA Procs
		if (IsOfClientBot()) {
			for (int i = 0; i < MAX_AA_PROCS; i += 4) {
				int32  aa_rank_id          = aabonuses.DefensiveProc[i + +SBIndex::COMBAT_PROC_ORIGIN_ID];
				int32  aa_spell_id         = aabonuses.DefensiveProc[i + SBIndex::COMBAT_PROC_SPELL_ID];
				int32  aa_proc_chance      = 100 + aabonuses.DefensiveProc[i + SBIndex::COMBAT_PROC_RATE_MOD];
				uint32 aa_proc_reuse_timer = aabonuses.DefensiveProc[i + SBIndex::COMBAT_PROC_REUSE_TIMER];

				if (aa_rank_id) {
					if (!IsProcLimitTimerActive(-aa_rank_id, aa_proc_reuse_timer, ProcType::DEFENSIVE_PROC)) {
						float chance = proc_chance * (static_cast<float>(aa_proc_chance) / 100.0f);
						if (zone->random.Roll(chance) && IsValidSpell(aa_spell_id)) {
							ExecWeaponProc(nullptr, aa_spell_id, on);
							SetProcLimitTimer(-aa_rank_id, aa_proc_reuse_timer, ProcType::DEFENSIVE_PROC);
						}
					}
				}
			}
		}
	}
}

void Mob::TryCombatProcs(const EQ::ItemInstance* weapon_g, Mob *on, uint16 hand, const EQ::ItemData* weapon_data) {

	if (!on) {
		SetTarget(nullptr);
		LogError("A null Mob object was passed to Mob::TryWeaponProc for evaluation!");
		return;
	}

	if (!IsAttackAllowed(on)) {
		LogCombat("Preventing procing off of unattackable things");
		return;
	}

	if (DivineAura()) {
		LogCombat("Procs cancelled, Divine Aura is in effect");
		return;
	}

	//used for special case when checking last ammo item on projectile hit.
	if (!weapon_g && weapon_data) {
		TryWeaponProc(nullptr, weapon_data, on, hand);
		TrySpellProc(nullptr, weapon_data, on, hand);
		return;
	}

	if (!weapon_g) {
		TrySpellProc(nullptr, (const EQ::ItemData*)nullptr, on);
		return;
	}

	if (!weapon_g->IsClassCommon()) {
		TrySpellProc(nullptr, (const EQ::ItemData*)nullptr, on);
		return;
	}

	// Innate + aug procs from weapons
	// TODO: powersource procs -- powersource procs are on invis augs, so shouldn't need anything extra
	TryWeaponProc(weapon_g, weapon_g->GetItem(), on, hand);
	// Procs from Buffs and AA both melee and range
	TrySpellProc(weapon_g, weapon_g->GetItem(), on, hand);

	return;
}

void Mob::TryWeaponProc(const EQ::ItemInstance *inst, const EQ::ItemData *weapon, Mob *on, uint16 hand)
{
	if (!on) {
		return;
	}
	if (!weapon)
		return;
	uint16 skillinuse = 28;
	int ourlevel = GetLevel();
	float ProcBonus = static_cast<float>(aabonuses.ProcChanceSPA +
		spellbonuses.ProcChanceSPA + itembonuses.ProcChanceSPA);
	ProcBonus += static_cast<float>(itembonuses.ProcChance) / 10.0f; // Combat Effects
	float ProcChance = GetProcChances(ProcBonus, hand);

	if (hand == EQ::invslot::slotSecondary)
		ProcChance /= 2;

	// Try innate proc on weapon
	// We can proc once here, either weapon or one aug
	bool proced = false; // silly bool to prevent augs from going if weapon does

	if (weapon->Proc.Type == EQ::item::ItemEffectCombatProc && IsValidSpell(weapon->Proc.Effect)) {
		float WPC = ProcChance * (100.0f + // Proc chance for this weapon
			static_cast<float>(weapon->ProcRate)) / 100.0f;
		if (zone->random.Roll(WPC)) {	// 255 dex = 0.084 chance of proc. No idea what this number should be really.
			if (weapon->Proc.Level2 > ourlevel) {
				LogCombat("Tried to proc ([{}]), but our level ([{}]) is lower than required ([{}])",
					weapon->Name, ourlevel, weapon->Proc.Level2);
				if (IsPet()) {
					Mob *own = GetOwner();
					if (own)
						own->MessageString(Chat::Red, PROC_PETTOOLOW);
				}
				else {
					MessageString(Chat::Red, PROC_TOOLOW);
				}
			}
			else {
				LogCombat("Attacking weapon ([{}]) successfully procing spell [{}] ([{}] percent chance)", weapon->Name, weapon->Proc.Effect, WPC * 100);
				ExecWeaponProc(inst, weapon->Proc.Effect, on);
				proced = true;
			}
		}
	}
	//If OneProcPerWeapon is not enabled, we reset the try for that weapon regardless of if we procced or not.
	//This is for some servers that may want to have as many procs triggering from weapons as possible in a single round.
	if (!RuleB(Combat, OneProcPerWeapon))
		proced = false;

	if (!proced && inst) {
		for (int r = EQ::invaug::SOCKET_BEGIN; r <= EQ::invaug::SOCKET_END; r++) {
			const EQ::ItemInstance *aug_i = inst->GetAugment(r);
			if (!aug_i) // no aug, try next slot!
				continue;
			const EQ::ItemData *aug = aug_i->GetItem();
			if (!aug)
				continue;

			if (aug->Proc.Type == EQ::item::ItemEffectCombatProc && IsValidSpell(aug->Proc.Effect)) {
				float APC = ProcChance * (100.0f + // Proc chance for this aug
					static_cast<float>(aug->ProcRate)) / 100.0f;
				if (zone->random.Roll(APC)) {
					if (aug->Proc.Level2 > ourlevel) {
						if (IsPet()) {
							Mob *own = GetOwner();
							if (own)
								own->MessageString(Chat::Red, PROC_PETTOOLOW);
						}
						else {
							MessageString(Chat::Red, PROC_TOOLOW);
						}
					}
					else {
						ExecWeaponProc(aug_i, aug->Proc.Effect, on);
						if (RuleB(Combat, OneProcPerWeapon))
							break;
					}
				}
			}
		}
	}
	// TODO: Powersource procs -- powersource procs are from augs so shouldn't need anything extra

	return;
}

void Mob::TrySpellProc(const EQ::ItemInstance *inst, const EQ::ItemData *weapon, Mob *on, uint16 hand)
{
	if (!on) {
		return;
	}

	float ProcBonus = static_cast<float>(spellbonuses.SpellProcChance +
		itembonuses.SpellProcChance + aabonuses.SpellProcChance);
	float ProcChance = 0.0f;
	ProcChance = GetProcChances(ProcBonus, hand);

	bool passed_skill_limit_check = true;
	EQ::skills::SkillType skillinuse = EQ::skills::SkillHandtoHand;

	if (weapon){
		skillinuse = GetSkillByItemType(weapon->ItemType);
	}

	if (hand == EQ::invslot::slotSecondary) {
		ProcChance /= 2;
	}

	bool rangedattk = false;
	if (weapon && hand == EQ::invslot::slotRange) {
		if (weapon->ItemType == EQ::item::ItemTypeArrow ||
			weapon->ItemType == EQ::item::ItemTypeLargeThrowing ||
			weapon->ItemType == EQ::item::ItemTypeSmallThrowing ||
			weapon->ItemType == EQ::item::ItemTypeBow) {
			rangedattk = true;
		}
	}

	if (!weapon && hand == EQ::invslot::slotRange && GetSpecialAbility(SpecialAbility::RangedAttack)) {
		rangedattk = true;
	}

	int16 poison_slot=-1;

	for (uint32 i = 0; i < m_max_procs; i++) {
		if (IsPet() && hand != EQ::invslot::slotPrimary) //Pets can only proc spell procs from their primay hand (ie; beastlord pets)
			continue; // If pets ever can proc from off hand, this will need to change

		if (SpellProcs[i].base_spellID == POISON_PROC &&
			(!weapon || weapon->ItemType != EQ::item::ItemType1HPiercing)) {
			continue; // Old school poison will only proc with 1HP equipped.
		}

		// Not ranged
		if (!rangedattk) {
			// Perma procs (Not used for AA, they are handled below)
			if (IsValidSpell(PermaProcs[i].spellID)) {
				if (zone->random.Roll(PermaProcs[i].chance)) { // TODO: Do these get spell bonus?
					LogCombat("Permanent proc [{}] procing spell [{}] ([{}] percent chance)", i, PermaProcs[i].spellID, PermaProcs[i].chance);
					ExecWeaponProc(nullptr, PermaProcs[i].spellID, on);
				}
				else {
					LogCombat("Permanent proc [{}] failed to proc [{}] ([{}] percent chance)", i, PermaProcs[i].spellID, PermaProcs[i].chance);
				}
			}

			// Spell procs (buffs)
			if (IsValidSpell(SpellProcs[i].spellID)) {
				if (SpellProcs[i].base_spellID == POISON_PROC) {
					poison_slot=i;
					continue; // Process the poison proc last per @mackal
				}

				passed_skill_limit_check = PassLimitToSkill(skillinuse, SpellProcs[i].base_spellID, ProcType::MELEE_PROC);

				if (passed_skill_limit_check && !IsProcLimitTimerActive(SpellProcs[i].base_spellID, SpellProcs[i].proc_reuse_time, ProcType::MELEE_PROC)) {
					float chance = ProcChance * (static_cast<float>(SpellProcs[i].chance) / 100.0f);
					if (zone->random.Roll(chance)) {
						LogCombat("Spell proc [{}] procing spell [{}] ([{}] percent chance)", i, SpellProcs[i].spellID, chance);
						SendBeginCast(SpellProcs[i].spellID, 0);
						ExecWeaponProc(nullptr, SpellProcs[i].spellID, on, SpellProcs[i].level_override);
						SetProcLimitTimer(SpellProcs[i].base_spellID, SpellProcs[i].proc_reuse_time, ProcType::MELEE_PROC);
						CheckNumHitsRemaining(NumHit::OffensiveSpellProcs, 0, SpellProcs[i].base_spellID);
					}
					else {
						LogCombat("Spell proc [{}] failed to proc [{}] ([{}] percent chance)", i, SpellProcs[i].spellID, chance);
					}
				}
			}
		}
		else if (rangedattk) { // ranged only
							   // ranged spell procs (buffs)
			if (IsValidSpell(RangedProcs[i].spellID)) {

				passed_skill_limit_check = PassLimitToSkill(skillinuse, RangedProcs[i].base_spellID, ProcType::RANGED_PROC);

				if (passed_skill_limit_check && !IsProcLimitTimerActive(RangedProcs[i].base_spellID, RangedProcs[i].proc_reuse_time, ProcType::RANGED_PROC)) {
					float chance = ProcChance * (static_cast<float>(RangedProcs[i].chance) / 100.0f);
					if (zone->random.Roll(chance)) {
						LogCombat("Ranged proc [{}] procing spell [{}] ([{}] percent chance)", i, RangedProcs[i].spellID, chance);
						ExecWeaponProc(nullptr, RangedProcs[i].spellID, on);
						CheckNumHitsRemaining(NumHit::OffensiveSpellProcs, 0, RangedProcs[i].base_spellID);
						SetProcLimitTimer(RangedProcs[i].base_spellID, RangedProcs[i].proc_reuse_time, ProcType::RANGED_PROC);
					}
					else {
						LogCombat("Ranged proc [{}] failed to proc [{}] ([{}] percent chance)", i, RangedProcs[i].spellID, chance);
					}
				}
			}
		}
	}

	//AA Melee and Ranged Procs
	if (IsOfClientBot()) {
		for (int i = 0; i < MAX_AA_PROCS; i += 4) {

			int32 aa_rank_id = 0;
			int32 aa_spell_id = SPELL_UNKNOWN;
			int32 aa_proc_chance = 100;
			uint32 aa_proc_reuse_timer = 0;
			int proc_type = 0; //used to deterimne which timer array is used.

			if (!rangedattk) {

				aa_rank_id = aabonuses.SpellProc[i + SBIndex::COMBAT_PROC_ORIGIN_ID];
				aa_spell_id = aabonuses.SpellProc[i + SBIndex::COMBAT_PROC_SPELL_ID];
				aa_proc_chance += aabonuses.SpellProc[i + SBIndex::COMBAT_PROC_RATE_MOD];
				aa_proc_reuse_timer = aabonuses.SpellProc[i + SBIndex::COMBAT_PROC_RATE_MOD];
				proc_type = ProcType::MELEE_PROC;
			}
			else {
				aa_rank_id = aabonuses.RangedProc[i + SBIndex::COMBAT_PROC_ORIGIN_ID];
				aa_spell_id = aabonuses.RangedProc[i + SBIndex::COMBAT_PROC_SPELL_ID];
				aa_proc_chance += aabonuses.RangedProc[i + SBIndex::COMBAT_PROC_RATE_MOD];
				aa_proc_reuse_timer = aabonuses.RangedProc[i + SBIndex::COMBAT_PROC_RATE_MOD];
				proc_type = ProcType::RANGED_PROC;
			}

			if (aa_rank_id) {

				passed_skill_limit_check = PassLimitToSkill(skillinuse, 0, proc_type, aa_rank_id);

				if (passed_skill_limit_check && !IsProcLimitTimerActive(-aa_rank_id, aa_proc_reuse_timer, proc_type)) {
					float chance = ProcChance * (static_cast<float>(aa_proc_chance) / 100.0f);
					if (zone->random.Roll(chance) && IsValidSpell(aa_spell_id)) {
						LogCombat("AA proc [{}] procing spell [{}] ([{}] percent chance)", aa_rank_id, aa_spell_id, chance);
						ExecWeaponProc(nullptr, aa_spell_id, on);
						SetProcLimitTimer(-aa_rank_id, aa_proc_reuse_timer, proc_type);
					}
					else {
						LogCombat("AA proc [{}] failed to proc [{}] ([{}] percent chance)", aa_rank_id, aa_spell_id, chance);
					}
				}
			}
		}
	}

	if (poison_slot > -1) {
		bool one_shot = !RuleB(Combat, UseExtendedPoisonProcs);
		float chance = (one_shot) ? 100.0f : ProcChance * (static_cast<float>(SpellProcs[poison_slot].chance) / 100.0f);
		uint16 spell_id = SpellProcs[poison_slot].spellID;

		if (zone->random.Roll(chance)) {
			LogCombat("Poison proc [{}] procing spell [{}] ([{}] percent chance)", poison_slot, spell_id, chance);
			SendBeginCast(spell_id, 0);
			ExecWeaponProc(nullptr, spell_id, on, SpellProcs[poison_slot].level_override);
			if (one_shot) {
				RemoveProcFromWeapon(spell_id);
			}
		}
	}

	TryCastOnSkillUse(on, skillinuse);

	if (HasSkillProcs() && hand != EQ::invslot::slotRange) { //We check ranged skill procs within the attack functions.
		TrySkillProc(on, skillinuse, 0, false, hand);
	}

	if (HasSkillProcSuccess() && hand != EQ::invslot::slotRange) { //We check ranged skill procs within the attack functions.
		TrySkillProc(on, skillinuse, 0, true, hand);
	}
	return;
}

void Mob::TryPetCriticalHit(Mob *defender, DamageHitInfo &hit)
{
	if (hit.damage_done < 1)
		return;

	// Allows pets to perform critical hits.
	// Each rank adds an additional 1% chance for any melee hit (primary, secondary, kick, bash, etc) to critical,
	// dealing up to 63% more damage. http://www.magecompendium.com/aa-short-library.html
	// appears to be 70% damage, unsure if changed or just bad info before

	Mob *owner = nullptr;
	int critChance = 0;
	critChance += RuleI(Combat, PetBaseCritChance); // 0 by default
	int critMod = 170;

	if (IsPet())
		owner = GetOwner();
	else if ((IsNPC() && CastToNPC()->GetSwarmOwner()))
		owner = entity_list.GetMobID(CastToNPC()->GetSwarmOwner());
	else
		return;

	if (!owner)
		return;

	int CritPetChance =
		owner->aabonuses.PetCriticalHit + owner->itembonuses.PetCriticalHit + owner->spellbonuses.PetCriticalHit;

	if (CritPetChance || critChance)
		// For pets use PetCriticalHit for base chance, pets do not innately critical with without it
		critChance += CritPetChance;

	if (critChance > 0) {
		if (zone->random.Roll(critChance)) {
			critMod += GetCritDmgMod(hit.skill, owner);
			hit.damage_done += 5;
			hit.damage_done = (hit.damage_done * critMod) / 100;

			entity_list.FilteredMessageCloseString(
				this, /* Sender */
				false,  /* Skip Sender */
				RuleI(Range, CriticalDamage),
				Chat::MeleeCrit, /* Type: 301 */
				FilterMeleeCrits, /* FilterType: 12 */
				CRITICAL_HIT, /* MessageFormat: %1 scores a critical hit! (%2) */
				0,
				GetCleanName(), /* Message1 */
				itoa(hit.damage_done + hit.min_damage) /* Message2 */
			);

		}
	}
}

void Mob::TryCriticalHit(Mob *defender, DamageHitInfo &hit, ExtraAttackOptions *opts)
{
#ifdef LUA_EQEMU
	bool ignoreDefault = false;
	LuaParser::Instance()->TryCriticalHit(this, defender, hit, opts, ignoreDefault);

	if (ignoreDefault) {
		return;
	}
#endif

	if (hit.damage_done < 1 || !defender) {
		return;
	}

	// decided to branch this into it's own function since it's going to be duplicating a lot of the
	// code in here, but could lead to some confusion otherwise
	if ((IsPet() && GetOwner()->IsClient()) || (IsNPC() && CastToNPC()->GetSwarmOwner())) {
		TryPetCriticalHit(defender, hit);
		return;
	}

	if (IsPet() && GetOwner() && GetOwner()->IsBot()) {
		TryPetCriticalHit(defender, hit);
		return;
	}

	if (IsNPC() && !RuleB(Combat, NPCCanCrit)) {
		return;
	}

	// 1: Try Slay Undead
	if (defender->GetBodyType() == BodyType::Undead || defender->GetBodyType() == BodyType::SummonedUndead ||
		defender->GetBodyType() == BodyType::Vampire) {
		int slay_rate_bonus = aabonuses.SlayUndead[SBIndex::SLAYUNDEAD_RATE_MOD] + itembonuses.SlayUndead[SBIndex::SLAYUNDEAD_RATE_MOD] + spellbonuses.SlayUndead[SBIndex::SLAYUNDEAD_RATE_MOD];

		LogCombatDetail("Slayundead hit rate [{}]", slay_rate_bonus);

		if (slay_rate_bonus) {
			float slay_chance = ((static_cast<float>(slay_rate_bonus) / 10000.0f) * RuleR(Combat, SlayRateMultiplier));

			if (zone->random.Roll(slay_chance)) {
				int slay_damage_bonus = aabonuses.SlayUndead[SBIndex::SLAYUNDEAD_DMG_MOD] + itembonuses.SlayUndead[SBIndex::SLAYUNDEAD_DMG_MOD] + spellbonuses.SlayUndead[SBIndex::SLAYUNDEAD_DMG_MOD];

				LogCombatDetail("Slayundead damage bonus [{}]", slay_damage_bonus);

				hit.damage_done = std::max(hit.damage_done, hit.base_damage) + 5;
				hit.damage_done = (hit.damage_done * slay_damage_bonus) / 100;
				hit.damage_done = static_cast<int>(hit.damage_done * RuleR(Combat, SlayDamageMultiplier));

				int min_slay = (hit.min_damage + 5) * slay_damage_bonus / 100;

				LogCombatDetail(" Calculated Slayundead damage [{}] - Min Slay Undead Damage [{}]", hit.damage_done, min_slay);

				if (hit.damage_done < min_slay) {
					hit.damage_done = min_slay;
				}

				LogCombatDetail("Final Slayundead damage [{}]", hit.damage_done);

				int slay_sex = GetGender() == Gender::Female ? FEMALE_SLAYUNDEAD : MALE_SLAYUNDEAD;

				entity_list.FilteredMessageCloseString(
					this, /* Sender */
					false, /* Skip Sender */
					RuleI(Range, CriticalDamage),
					Chat::MeleeCrit, /* Type: 301 */
					FilterMeleeCrits, /* FilterType: 12 */
					slay_sex,
					0,
					GetCleanName(), /* Message1 */
					itoa(hit.damage_done) /* Message2 */
					);
				return;
			}
		}
	}

	// 2: Try Melee Critical
	// a lot of good info: http://giline.versus.jp/shiden/damage_e.htm, http://giline.versus.jp/shiden/su.htm

	// We either require an innate crit chance or some SPA 169 to crit
	bool innate_crit = false;
	int crit_chance = GetCriticalChanceBonus(hit.skill);
	if ((GetClass() == Class::Warrior || GetClass() == Class::Berserker) && GetLevel() >= 12) {
		innate_crit = true;
	} else if (GetClass() == Class::Ranger && GetLevel() >= 12 && hit.skill == EQ::skills::SkillArchery) {
		innate_crit = true;
	} else if (GetClass() == Class::Rogue && GetLevel() >= 12 && hit.skill == EQ::skills::SkillThrowing) {
		innate_crit = true;
	}

	// we have a chance to crit!
	if (innate_crit || crit_chance) {
		int difficulty = 0;

		if (hit.skill == EQ::skills::SkillArchery) {
			difficulty = RuleI(Combat, ArcheryCritDifficulty);
		} else if (hit.skill == EQ::skills::SkillThrowing) {
			difficulty = RuleI(Combat, ThrowingCritDifficulty);
		} else {
			difficulty = RuleI(Combat, MeleeCritDifficulty);
		}

		int roll = zone->random.Int(1, difficulty);
		int dex_bonus = GetDEX();

		if (dex_bonus > 255) {
			dex_bonus = 255 + ((dex_bonus - 255) / 5);
		}

		dex_bonus += 45; // chances did not match live without a small boost

						 // so if we have an innate crit we have a better chance, except for ber throwing
		if (!innate_crit || (GetClass() == Class::Berserker && hit.skill == EQ::skills::SkillThrowing)) {
			dex_bonus = dex_bonus * 3 / 5;
		}

		if (crit_chance) {
			dex_bonus += dex_bonus * crit_chance / 100;
		}

		// check if we crited
		if (roll < dex_bonus) {
			// step 1: check for finishing blow
			if (TryFinishingBlow(defender, hit.damage_done)) {
				return;
			}

			// step 2: calculate damage
			hit.damage_done = std::max(hit.damage_done, hit.base_damage) + 5;
			int og_damage = hit.damage_done;
			int crit_mod = 170 + GetCritDmgMod(hit.skill);

			if (crit_mod < 100) {
				crit_mod = 100;
			}

			hit.damage_done = hit.damage_done * crit_mod / 100;
			LogCombatDetail("Crit success roll [{}] dex chance [{}] og dmg [{}] crit_mod [{}] new dmg [{}]", roll, dex_bonus, og_damage, crit_mod, hit.damage_done);

			// step 3: check deadly strike
			if (GetClass() == Class::Rogue && hit.skill == EQ::skills::SkillThrowing) {
				if (BehindMob(defender, GetX(), GetY())) {
					int chance = GetLevel() * 12;
					if (zone->random.Int(1, 1000) < chance) {
						// step 3a: check assassinate
						int assassinate_damage = TryAssassinate(defender, hit.skill); // I don't think this is right
						if (assassinate_damage) {
							hit.damage_done = assassinate_damage;
							return;
						}
						hit.damage_done = hit.damage_done * 200 / 100;

						entity_list.FilteredMessageCloseString(
							this, /* Sender */
							false, /* Skip Sender */
							RuleI(Range, CriticalDamage),
							Chat::MeleeCrit, /* Type: 301 */
							FilterMeleeCrits, /* FilterType: 12 */
							DEADLY_STRIKE, /* MessageFormat: %1 scores a Deadly Strike!(%2) */
							0,
							GetCleanName(), /* Message1 */
							itoa(hit.damage_done + hit.min_damage) /* Message2 */
						);
						return;
					}
				}
			}

			// step 4: check crips
			// this SPA was reused on live ...
			bool berserk = spellbonuses.BerserkSPA || itembonuses.BerserkSPA || aabonuses.BerserkSPA;
			if (!berserk) {
				if (zone->random.Roll(GetCrippBlowChance())) {
					berserk = true;
				}
			}

			if (IsBerserk() || berserk) {
				hit.damage_done += og_damage * 119 / 100;
				LogCombat("Crip damage [{}]", hit.damage_done);

				entity_list.FilteredMessageCloseString(
					this, /* Sender */
					false, /* Skip Sender */
					RuleI(Range, CriticalDamage),
					Chat::MeleeCrit, /* Type: 301 */
					FilterMeleeCrits, /* FilterType: 12 */
					CRIPPLING_BLOW, /* MessageFormat: %1 lands a Crippling Blow!(%2) */
					0,
					GetCleanName(), /* Message1 */
					itoa(hit.damage_done + hit.min_damage) /* Message2 */
				);

				// Crippling blows also have a chance to stun
				// Kayen: Crippling Blow would cause a chance to interrupt for npcs < 55, with a
				// staggers message.
				if (defender->GetLevel() <= 55 && !defender->GetSpecialAbility(SpecialAbility::StunImmunity)) {
					entity_list.MessageCloseString(
						defender,
						true,
						RuleI(Range, Emote),
						Chat::Emote,
						STAGGERS,
						GetName()
					);
					defender->Stun(RuleI(Combat, StunDuration));
				}
				return;
			}

			/* Normal Critical hit message */
			entity_list.FilteredMessageCloseString(
				this, /* Sender */
				false, /* Skip Sender */
				RuleI(Range, CriticalDamage),
				Chat::MeleeCrit, /* Type: 301 */
				FilterMeleeCrits, /* FilterType: 12 */
				CRITICAL_HIT, /* MessageFormat: %1 scores a critical hit! (%2) */
				0,
				GetCleanName(), /* Message1 */
				itoa(hit.damage_done + hit.min_damage) /* Message2 */
			);
		}
	}
}

bool Mob::TryFinishingBlow(Mob *defender, int64 &damage)
{
	float hp_limit = 10.0f;
	auto fb_hp_limit = std::max(
		{
			aabonuses.FinishingBlowLvl[SBIndex::FINISHING_BLOW_LEVEL_HP_RATIO],
			spellbonuses.FinishingBlowLvl[SBIndex::FINISHING_BLOW_LEVEL_HP_RATIO],
			itembonuses.FinishingBlowLvl[SBIndex::FINISHING_BLOW_LEVEL_HP_RATIO]
		}
	);

	if (fb_hp_limit) {
		hp_limit = fb_hp_limit/10.0f;
	}

	if (defender && !defender->IsClient() && defender->GetHPRatio() < hp_limit) {
		uint32 finishing_blow_damage =
				   aabonuses.FinishingBlow[SBIndex::FINISHING_EFFECT_DMG] + spellbonuses.FinishingBlow[SBIndex::FINISHING_EFFECT_DMG] + itembonuses.FinishingBlow[SBIndex::FINISHING_EFFECT_DMG];

		uint32 finishing_blow_level = 0;
		finishing_blow_level = aabonuses.FinishingBlowLvl[SBIndex::FINISHING_EFFECT_LEVEL_MAX];
		if (finishing_blow_level < spellbonuses.FinishingBlowLvl[SBIndex::FINISHING_EFFECT_LEVEL_MAX]) {
			finishing_blow_level = spellbonuses.FinishingBlowLvl[SBIndex::FINISHING_EFFECT_LEVEL_MAX];
		} else if (finishing_blow_level < itembonuses.FinishingBlowLvl[SBIndex::FINISHING_EFFECT_LEVEL_MAX]) {
			finishing_blow_level = itembonuses.FinishingBlowLvl[SBIndex::FINISHING_EFFECT_LEVEL_MAX];
		}

		// modern AA description says rank 1 (500) is 50% chance
		int proc_chance = (
			aabonuses.FinishingBlow[SBIndex::FINISHING_EFFECT_PROC_CHANCE] +
			itembonuses.FinishingBlow[SBIndex::FINISHING_EFFECT_PROC_CHANCE] +
			spellbonuses.FinishingBlow[SBIndex::FINISHING_EFFECT_PROC_CHANCE]
		);

		if (
			(
				(RuleB(Combat, FinishingBlowOnlyWhenFleeing) && !defender->currently_fleeing) ||
				!RuleB(Combat, FinishingBlowOnlyWhenFleeing)
			) &&
			finishing_blow_level &&
			finishing_blow_damage &&
			defender->GetLevel() <= finishing_blow_level &&
			proc_chance >= zone->random.Int(1, 1000)
		) {
			/* Finishing Blow Critical Message */
			entity_list.FilteredMessageCloseString(
				this, /* Sender */
				false, /* Skip Sender */
				RuleI(Range, CriticalDamage),
				Chat::MeleeCrit, /* Type: 301 */
				FilterMeleeCrits, /* FilterType: 12 */
				FINISHING_BLOW, /* MessageFormat: %1 scores a Finishing Blow!!) */
				0,
				GetCleanName() /* Message1 */
			);

			damage = finishing_blow_damage;
			return true;
		}
	}
	return false;
}

void Mob::DoRiposte(Mob *defender)
{
	LogCombat("Preforming a riposte");

	if (!defender)
		return;

	// so ahhh the angle you can riposte is larger than the angle you can hit :P
	if (!defender->IsFacingMob(this)) {
		defender->MessageString(Chat::TooFarAway, CANT_SEE_TARGET);
		return;
	}

	defender->Attack(this, EQ::invslot::slotPrimary, true);

	if (HasDied())
		return;

	// this effect isn't used on live? See no AAs or spells
	int32 DoubleRipChance = defender->aabonuses.DoubleRiposte + defender->spellbonuses.DoubleRiposte +
		defender->itembonuses.DoubleRiposte;

	if (DoubleRipChance && zone->random.Roll(DoubleRipChance)) {
		LogCombat("Preforming a double riposted from SpellEffect::DoubleRiposte ([{}] percent chance)", DoubleRipChance);
		defender->Attack(this, EQ::invslot::slotPrimary, true);
		if (HasDied())
			return;
	}

	DoubleRipChance = defender->aabonuses.GiveDoubleRiposte[SBIndex::DOUBLE_RIPOSTE_CHANCE] + defender->spellbonuses.GiveDoubleRiposte[SBIndex::DOUBLE_RIPOSTE_CHANCE] +
					  defender->itembonuses.GiveDoubleRiposte[SBIndex::DOUBLE_RIPOSTE_CHANCE];

	// Live AA - Double Riposte
	if (DoubleRipChance && zone->random.Roll(DoubleRipChance)) {
		LogCombat("Preforming a double riposted from SpellEffect::GiveDoubleRiposte base1 == 0 ([{}] percent chance)", DoubleRipChance);
		defender->Attack(this, EQ::invslot::slotPrimary, true);
		if (HasDied())
			return;
	}

	// Double Riposte effect, allows for a chance to do RIPOSTE with a skill specific special attack (ie Return Kick).
	// Coded narrowly: Limit to one per client. Limit AA only. [1 = Skill Attack Chance, 2 = Skill]

	DoubleRipChance = defender->aabonuses.GiveDoubleRiposte[SBIndex::DOUBLE_RIPOSTE_SKILL_ATK_CHANCE];

	if (DoubleRipChance && zone->random.Roll(DoubleRipChance)) {
		LogCombat("Preforming a return SPECIAL ATTACK ([{}] percent chance)", DoubleRipChance);

		if (defender->GetClass() == Class::Monk)
			defender->MonkSpecialAttack(this, defender->aabonuses.GiveDoubleRiposte[SBIndex::DOUBLE_RIPOSTE_SKILL]);
		else if (defender->IsClient()) // so yeah, even if you don't have the skill you can still do the attack :P (and we don't crash anymore)
			defender->CastToClient()->DoClassAttacks(this, defender->aabonuses.GiveDoubleRiposte[SBIndex::DOUBLE_RIPOSTE_SKILL], true);
	}
}

void Mob::ApplyMeleeDamageMods(uint16 skill, int64 &damage, Mob *defender, ExtraAttackOptions *opts)
{
	int64 damage_bonus_mod = 0;
	damage_bonus_mod += GetMeleeDamageMod_SE(skill);
	damage_bonus_mod += GetMeleeDmgPositionMod(defender);

	if (opts) {
		damage_bonus_mod += opts->melee_damage_bonus_flat;
	}

	if (defender) {
		if (defender->IsOfClientBotMerc() && defender->GetClass() == Class::Warrior) {
			damage_bonus_mod -= 5;
		}

		if (defender->IsOfClientBotMerc()) {
			damage_bonus_mod += (
				defender->spellbonuses.MeleeMitigationEffect +
				defender->itembonuses.MeleeMitigationEffect +
				defender->aabonuses.MeleeMitigationEffect
			);
		}
	}

	damage += damage * damage_bonus_mod / 100;
}

bool Mob::HasDied() {
	bool Result = false;
	int64 hp_below = 0;

	hp_below = (GetDelayDeath() * -1);

	if ((GetHP()) <= (hp_below))
		Result = true;

	return Result;
}

const DamageTable &Mob::GetDamageTable() const
{
	static const DamageTable dmg_table[] = {
		{ 210, 49, 105 }, // 1-50
		{ 245, 35,  80 }, // 51
		{ 245, 35,  80 }, // 52
		{ 245, 35,  80 }, // 53
		{ 245, 35,  80 }, // 54
		{ 245, 35,  80 }, // 55
		{ 265, 28,  70 }, // 56
		{ 265, 28,  70 }, // 57
		{ 265, 28,  70 }, // 58
		{ 265, 28,  70 }, // 59
		{ 285, 23,  65 }, // 60
		{ 285, 23,  65 }, // 61
		{ 285, 23,  65 }, // 62
		{ 290, 21,  60 }, // 63
		{ 290, 21,  60 }, // 64
		{ 295, 19,  55 }, // 65
		{ 295, 19,  55 }, // 66
		{ 300, 19,  55 }, // 67
		{ 300, 19,  55 }, // 68
		{ 300, 19,  55 }, // 69
		{ 305, 19,  55 }, // 70
		{ 305, 19,  55 }, // 71
		{ 310, 17,  50 }, // 72
		{ 310, 17,  50 }, // 73
		{ 310, 17,  50 }, // 74
		{ 315, 17,  50 }, // 75
		{ 315, 17,  50 }, // 76
		{ 325, 17,  45 }, // 77
		{ 325, 17,  45 }, // 78
		{ 325, 17,  45 }, // 79
		{ 335, 17,  45 }, // 80
		{ 335, 17,  45 }, // 81
		{ 345, 17,  45 }, // 82
		{ 345, 17,  45 }, // 83
		{ 345, 17,  45 }, // 84
		{ 355, 17,  45 }, // 85
		{ 355, 17,  45 }, // 86
		{ 365, 17,  45 }, // 87
		{ 365, 17,  45 }, // 88
		{ 365, 17,  45 }, // 89
		{ 375, 17,  45 }, // 90
		{ 375, 17,  45 }, // 91
		{ 380, 17,  45 }, // 92
		{ 380, 17,  45 }, // 93
		{ 380, 17,  45 }, // 94
		{ 385, 17,  45 }, // 95
		{ 385, 17,  45 }, // 96
		{ 390, 17,  45 }, // 97
		{ 390, 17,  45 }, // 98
		{ 390, 17,  45 }, // 99
		{ 395, 17,  45 }, // 100
		{ 395, 17,  45 }, // 101
		{ 400, 17,  45 }, // 102
		{ 400, 17,  45 }, // 103
		{ 400, 17,  45 }, // 104
		{ 405, 17,  45 }  // 105
	};

	static const DamageTable mnk_table[] = {
		{ 220, 45, 100 }, // 1-50
		{ 245, 35,  80 }, // 51
		{ 245, 35,  80 }, // 52
		{ 245, 35,  80 }, // 53
		{ 245, 35,  80 }, // 54
		{ 245, 35,  80 }, // 55
		{ 285, 23,  65 }, // 56
		{ 285, 23,  65 }, // 57
		{ 285, 23,  65 }, // 58
		{ 285, 23,  65 }, // 59
		{ 290, 21,  60 }, // 60
		{ 290, 21,  60 }, // 61
		{ 290, 21,  60 }, // 62
		{ 295, 19,  55 }, // 63
		{ 295, 19,  55 }, // 64
		{ 300, 17,  50 }, // 65
		{ 300, 17,  50 }, // 66
		{ 310, 17,  50 }, // 67
		{ 310, 17,  50 }, // 68
		{ 310, 17,  50 }, // 69
		{ 320, 17,  50 }, // 70
		{ 320, 17,  50 }, // 71
		{ 325, 15,  45 }, // 72
		{ 325, 15,  45 }, // 73
		{ 325, 15,  45 }, // 74
		{ 330, 15,  45 }, // 75
		{ 330, 15,  45 }, // 76
		{ 335, 15,  40 }, // 77
		{ 335, 15,  40 }, // 78
		{ 335, 15,  40 }, // 79
		{ 345, 15,  40 }, // 80
		{ 345, 15,  40 }, // 81
		{ 355, 15,  40 }, // 82
		{ 355, 15,  40 }, // 83
		{ 355, 15,  40 }, // 84
		{ 365, 15,  40 }, // 85
		{ 365, 15,  40 }, // 86
		{ 375, 15,  40 }, // 87
		{ 375, 15,  40 }, // 88
		{ 375, 15,  40 }, // 89
		{ 385, 15,  40 }, // 90
		{ 385, 15,  40 }, // 91
		{ 390, 15,  40 }, // 92
		{ 390, 15,  40 }, // 93
		{ 390, 15,  40 }, // 94
		{ 395, 15,  40 }, // 95
		{ 395, 15,  40 }, // 96
		{ 400, 15,  40 }, // 97
		{ 400, 15,  40 }, // 98
		{ 400, 15,  40 }, // 99
		{ 405, 15,  40 }, // 100
		{ 405, 15,  40 }, // 101
		{ 410, 15,  40 }, // 102
		{ 410, 15,  40 }, // 103
		{ 410, 15,  40 }, // 104
		{ 415, 15,  40 }, // 105
	};

	bool monk = GetClass() == Class::Monk;
	bool melee = IsWarriorClass();
	// tables caped at 105 for now -- future proofed for a while at least :P
	int level = std::min(static_cast<int>(GetLevel()), 105);

	if (!melee || (!monk && level < 51))
		return dmg_table[0];

	if (monk && level < 51)
		return mnk_table[0];

	auto &which = monk ? mnk_table : dmg_table;
	return which[level - 50];
}

int Mob::GetMobFixedOffenseSkill()
{
	// Due to new code using a combination of Offense and Weapon skill to determine hit, depending on the class
	// and weapon wielded by a mob, the hit rate of an equal level mob could vary between 15% and 60%, which made
	// many mobs far too easy.  This particular call replaces the class based Offense Skill with a fixed value
	// equal to that of a Warrior of appropriate Level if UseMobFixedOffenseSkill flag is TRUE.

	int level = EQ::ClampUpper(std::max(1, static_cast<int>(GetLevel())), 60);

	if (level <= 40) {
		return (level * 5) + 5;
	} else if (EQ::ValueWithin(level, 41, 50)) {
		return 210;
	} else if (EQ::ValueWithin(level, 51, 58)) {
		return 210 + ((level - 50) * 5);
	}

	return 252;
}

int Mob::GetMobFixedWeaponSkill()
{
	// Due to new code using a combination of Offense and Weapon skill to determine hit, depending on the class
	// and weapon wielded by a mob, the hit rate of an equal level mob could vary between 15% and 60%, which made
	// many mobs far too easy.  This particular call replaces the weapon/class based Weapon Skill with a fixed value.
	// Two tables exist, one equal to a Warrior of appropriate level, and one modified to make hit rate equal to the old code
	// assuming the UseMobFixedOffenseSkill flag is set TRUE or the mob class is a Warrior (all the the bonus is in Weapon Skill).

	int level = EQ::ClampUpper(std::max(1, static_cast<int>(GetLevel())), 70);

	if (!RuleB(Combat, UseEnhancedMobStaticWeaponSkill)) {
		if (level <= 39) {
			return (level * 5) + 5;
		} else if (EQ::ValueWithin(level, 40, 50)) {
			return 200;
		} else if (EQ::ValueWithin(level, 51, 60)) {
			return 200 + ((level - 50) * 5);
		} else if (EQ::ValueWithin(level, 61, 65)) {
			return 250;
		}

		return 250 + ((level - 65) * 5);
	}

	if (level <= 39) {
		return (level * 6) - 1;
	} else if (EQ::ValueWithin(level, 45, 49)) {
		return 260;
	} else if (EQ::ValueWithin(level, 50, 54)) {
		return (level * 6) + 1;
	} else if (EQ::ValueWithin(level, 55, 59)) {
		return (level * 7) + 5;
	} else if (EQ::ValueWithin(level, 60, 65)) {
		return (level * 5) + 59;
	}

	return 330 + (level - 66);
}

void Mob::ApplyDamageTable(DamageHitInfo &hit)
{
#ifdef LUA_EQEMU
	bool ignoreDefault = false;
	LuaParser::Instance()->ApplyDamageTable(this, hit, ignoreDefault);

	if (ignoreDefault) {
		return;
	}
#endif

	// someone may want to add this to custom servers, can remove this if that's the case
	if (!IsClient()&& !IsBot()) {
		return;
	}

	// this was parsed, but we do see the min of 10 and the normal minus factor is 105, so makes sense
	if (hit.offense < 115)
		return;

	// things that come out to 1 dmg seem to skip this (ex non-bash slam classes)
	if (hit.damage_done < 2)
		return;

	auto &damage_table = GetDamageTable();

	if (zone->random.Roll(damage_table.chance))
		return;

	int basebonus = hit.offense - damage_table.minusfactor;
	basebonus = std::max(10, basebonus / 2);
	int extrapercent = zone->random.Roll0(basebonus);
	int percent = std::min(100 + extrapercent, damage_table.max_extra);
	hit.damage_done = (hit.damage_done * percent) / 100;

	if (IsWarriorClass() && GetLevel() > 54)
		hit.damage_done++;
	Log(Logs::Detail, Logs::Attack, "Damage table applied %d (max %d)", percent, damage_table.max_extra);
}

void Mob::TrySkillProc(Mob *on, EQ::skills::SkillType skill, uint16 ReuseTime, bool Success, uint16 hand, bool IsDefensive)
{
	if (!on) {
		SetTarget(nullptr);
		LogError("A null Mob object was passed to Mob::TrySkillProc for evaluation!");
		return;
	}

	if (on->HasDied()) {
		return;
	}

	if (!spellbonuses.LimitToSkill[skill] && !itembonuses.LimitToSkill[skill] && !aabonuses.LimitToSkill[skill]) {
		return;
	}

	/*
		Allow one proc from each (Spell/Item/AA)
		Kayen: Due to limited avialability of effects on live it is too difficult
		to confirm how they stack at this time, will adjust formula when more data is avialablle to test.
	*/
	bool CanProc = true;

	uint16 base_spell_id = 0;
	uint16 proc_spell_id = 0;
	float ProcMod = 0;
	float chance = 0;

	if (IsDefensive) {
		chance = on->GetSkillProcChances(ReuseTime, hand);
	}
	else {
		chance = GetSkillProcChances(ReuseTime, hand);
	}

	if (spellbonuses.LimitToSkill[skill]) {

		for (int i = 0; i < MAX_SKILL_PROCS; i++) {
			if (CanProc &&
				((!Success && spellbonuses.SkillProc[i] && IsValidSpell(spellbonuses.SkillProc[i]))
					|| (Success && spellbonuses.SkillProcSuccess[i] && IsValidSpell(spellbonuses.SkillProcSuccess[i])))) {

				if (Success) {
					base_spell_id = spellbonuses.SkillProcSuccess[i];
				}
				else {
					base_spell_id = spellbonuses.SkillProc[i];
				}

				proc_spell_id = 0;
				ProcMod = 0;

				for (int i = 0; i < EFFECT_COUNT; i++) {

					if (spells[base_spell_id].effect_id[i] == SpellEffect::SkillProcAttempt || spells[base_spell_id].effect_id[i] == SpellEffect::SkillProcSuccess) {
						proc_spell_id = spells[base_spell_id].base_value[i];
						ProcMod = static_cast<float>(spells[base_spell_id].limit_value[i]);
					}

					else if (spells[base_spell_id].effect_id[i] == SpellEffect::LimitToSkill && spells[base_spell_id].base_value[i] <= EQ::skills::HIGHEST_SKILL) {
						if (CanProc && spells[base_spell_id].base_value[i] == skill && IsValidSpell(proc_spell_id)) {
							float final_chance = chance * (ProcMod / 100.0f);
							if (zone->random.Roll(final_chance)) {
								ExecWeaponProc(nullptr, proc_spell_id, on);
								CheckNumHitsRemaining(NumHit::OffensiveSpellProcs, 0, base_spell_id);
								CanProc = false;
								break;
							}
						}
					}
					else {
						//Reset and check for proc in sequence
						proc_spell_id = 0;
						ProcMod = 0;
					}
				}
			}
		}
	}

	if (itembonuses.LimitToSkill[skill]) {
		CanProc = true;
		for (int i = 0; i < MAX_SKILL_PROCS; i++) {
			if (CanProc &&
				((!Success && itembonuses.SkillProc[i] && IsValidSpell(itembonuses.SkillProc[i]))
					|| (Success && itembonuses.SkillProcSuccess[i] && IsValidSpell(itembonuses.SkillProcSuccess[i])))) {

				if (Success) {
					base_spell_id = itembonuses.SkillProcSuccess[i];
				}
				else {
					base_spell_id = itembonuses.SkillProc[i];
				}

				proc_spell_id = 0;
				ProcMod = 0;

				for (int i = 0; i < EFFECT_COUNT; i++) {
					if (spells[base_spell_id].effect_id[i] == SpellEffect::SkillProcAttempt || spells[base_spell_id].effect_id[i] == SpellEffect::SkillProcSuccess) {
						proc_spell_id = spells[base_spell_id].base_value[i];
						ProcMod = static_cast<float>(spells[base_spell_id].limit_value[i]);
					}

					else if (spells[base_spell_id].effect_id[i] == SpellEffect::LimitToSkill && spells[base_spell_id].base_value[i] <= EQ::skills::HIGHEST_SKILL) {

						if (CanProc && spells[base_spell_id].base_value[i] == skill && IsValidSpell(proc_spell_id)) {
							float final_chance = chance * (ProcMod / 100.0f);
							if (zone->random.Roll(final_chance)) {
								ExecWeaponProc(nullptr, proc_spell_id, on);
								CanProc = false;
								break;
							}
						}
					}
					else {
						proc_spell_id = 0;
						ProcMod = 0;
					}
				}
			}
		}
	}

	if (IsOfClientBot() && aabonuses.LimitToSkill[skill]) {

		CanProc = true;
		uint32 effect_id = 0;
		int32 base_value = 0;
		int32 limit_value = 0;
		uint32 slot = 0;

		for (int i = 0; i < MAX_SKILL_PROCS; i++) {
			if (CanProc &&
				((!Success && aabonuses.SkillProc[i])
					|| (Success && aabonuses.SkillProcSuccess[i]))) {
				int aaid = 0;

				if (Success)
					base_spell_id = aabonuses.SkillProcSuccess[i];
				else
					base_spell_id = aabonuses.SkillProc[i];

				proc_spell_id = 0;
				ProcMod = 0;

				for (auto &rank_info : aa_ranks) {
					auto ability_rank = zone->GetAlternateAdvancementAbilityAndRank(rank_info.first, rank_info.second.first);
					auto ability = ability_rank.first;
					auto rank = ability_rank.second;

					if (!ability) {
						continue;
					}

					for (auto &effect : rank->effects) {
						effect_id = effect.effect_id;
						base_value = effect.base_value;
						limit_value = effect.limit_value;
						slot = effect.slot;

						if (effect_id == SpellEffect::SkillProcAttempt || effect_id == SpellEffect::SkillProcSuccess) {
							proc_spell_id = base_value;
							ProcMod = static_cast<float>(limit_value);
						}
						else if (effect_id == SpellEffect::LimitToSkill && base_value <= EQ::skills::HIGHEST_SKILL) {

							if (CanProc && base_value == skill && IsValidSpell(proc_spell_id)) {
								float final_chance = chance * (ProcMod / 100.0f);

								if (zone->random.Roll(final_chance)) {
									ExecWeaponProc(nullptr, proc_spell_id, on);
									CanProc = false;
									break;
								}
							}
						}
						else {
							proc_spell_id = 0;
							ProcMod = 0;
						}
					}
				}
			}
		}
	}
}

float Mob::GetSkillProcChances(uint16 ReuseTime, uint16 hand) {

	uint32 weapon_speed;
	float ProcChance = 0;

	if (!ReuseTime && hand) {
		weapon_speed = GetWeaponSpeedbyHand(hand);
		ProcChance = static_cast<float>(weapon_speed) * (RuleR(Combat, AvgProcsPerMinute) / 60000.0f);
		if (hand == EQ::invslot::slotSecondary) {
			ProcChance /= 2;
		}
	}

	else {
		ProcChance = static_cast<float>(ReuseTime) * (RuleR(Combat, AvgProcsPerMinute) / 60000.0f);
	}

	return ProcChance;
}

void Mob::TryCastOnSkillUse(Mob *on, EQ::skills::SkillType skill) {

	if (!spellbonuses.HasSkillAttackProc[skill] && !itembonuses.HasSkillAttackProc[skill] && !aabonuses.HasSkillAttackProc[skill]) {
		return;
	}

	if (!on) {
		SetTarget(nullptr);
		LogError("A null Mob object was passed to Mob::TryCastOnSkillUse for evaluation!");
		return;
	}

	if (on->HasDied()) {
		return;
	}

	if (spellbonuses.HasSkillAttackProc[skill]) {
		for (int i = 0; i < MAX_CAST_ON_SKILL_USE; i += 3) {
			if (
				spellbonuses.SkillAttackProc[i + SBIndex::SKILLATK_PROC_SPELL_ID] &&
				skill == spellbonuses.SkillAttackProc[i + SBIndex::SKILLATK_PROC_SKILL]
			) {
				if (
					IsValidSpell(spellbonuses.SkillAttackProc[i + SBIndex::SKILLATK_PROC_SPELL_ID]) &&
					zone->random.Int(1, 1000) <= spellbonuses.SkillAttackProc[i + SBIndex::SKILLATK_PROC_CHANCE]
				) {
					SpellFinished(spellbonuses.SkillAttackProc[i + SBIndex::SKILLATK_PROC_SPELL_ID], on, EQ::spells::CastingSlot::Item, 0, -1, spells[spellbonuses.SkillAttackProc[i + SBIndex::SKILLATK_PROC_SPELL_ID]].resist_difficulty);
				}
			}
		}
	}

	if (itembonuses.HasSkillAttackProc[skill]) {
		for (int i = 0; i < MAX_CAST_ON_SKILL_USE; i += 3) {
			if (
				itembonuses.SkillAttackProc[i + SBIndex::SKILLATK_PROC_SPELL_ID] &&
				skill == itembonuses.SkillAttackProc[i + SBIndex::SKILLATK_PROC_SKILL]
			) {
				if (
					IsValidSpell(itembonuses.SkillAttackProc[i + SBIndex::SKILLATK_PROC_SPELL_ID]) &&
					zone->random.Int(1, 1000) <= itembonuses.SkillAttackProc[i + SBIndex::SKILLATK_PROC_CHANCE]
				) {
					SpellFinished(itembonuses.SkillAttackProc[i + SBIndex::SKILLATK_PROC_SPELL_ID], on, EQ::spells::CastingSlot::Item, 0, -1, spells[itembonuses.SkillAttackProc[i + SBIndex::SKILLATK_PROC_SPELL_ID]].resist_difficulty);
				}
			}
		}
	}

	if (aabonuses.HasSkillAttackProc[skill]) {
		for (int i = 0; i < MAX_CAST_ON_SKILL_USE; i += 3) {
			if (
				IsValidSpell(aabonuses.SkillAttackProc[i + SBIndex::SKILLATK_PROC_SPELL_ID]) &&
				aabonuses.SkillAttackProc[i + SBIndex::SKILLATK_PROC_SPELL_ID] &&
				skill == aabonuses.SkillAttackProc[i + SBIndex::SKILLATK_PROC_SKILL]
			) {
				if (zone->random.Int(1, 1000) <= aabonuses.SkillAttackProc[i + SBIndex::SKILLATK_PROC_CHANCE]) {
					SpellFinished(aabonuses.SkillAttackProc[i + SBIndex::SKILLATK_PROC_SPELL_ID], on, EQ::spells::CastingSlot::Item, 0, -1, spells[aabonuses.SkillAttackProc[i + SBIndex::SKILLATK_PROC_SPELL_ID]].resist_difficulty);
				}
			}
		}
	}
}

bool Mob::TryRootFadeByDamage(int buffslot, Mob* attacker) {

	/*Dev Quote 2010: http://forums.station.sony.com/eq/posts/list.m?topic_id=161443
	The Viscid Roots AA does the following: Reduces the chance for root to break by X percent.
	There is no distinction of any kind between the caster inflicted damage, or anyone
	else's damage. There is also no distinction between Direct and DOT damage in the root code.

	General Mechanics
	- Check buffslot to make sure damage from a root does not cancel the root
	- If multiple roots on target, always and only checks first root slot and if broken only removes that slots root.
	- Only roots on determental spells can be broken by damage.
	- Root break chance values obtained from live parses.
	*/

	if (!attacker || !spellbonuses.Root[SBIndex::ROOT_EXISTS] || spellbonuses.Root[SBIndex::ROOT_BUFFSLOT] < 0) {
		return false;
	}

	if (IsDetrimentalSpell(buffs[spellbonuses.Root[SBIndex::ROOT_BUFFSLOT]].spellid) && spellbonuses.Root[SBIndex::ROOT_BUFFSLOT] != buffslot) {

		int BreakChance = RuleI(Spells, RootBreakFromSpells);
		BreakChance -= BreakChance * buffs[spellbonuses.Root[SBIndex::ROOT_BUFFSLOT]].RootBreakChance / 100;
		int level_diff = attacker->GetLevel() - GetLevel();

		//Use baseline if level difference <= 1 (ie. If target is (1) level less than you, or equal or greater level)

		if (level_diff == 2) {
			BreakChance = (BreakChance * 80) / 100; //Decrease by 20%;
		}
		else if (level_diff >= 3 && level_diff <= 20) {
			BreakChance = (BreakChance * 60) / 100; //Decrease by 40%;
		}
		else if (level_diff > 21) {
			BreakChance = (BreakChance * 20) / 100; //Decrease by 80%;
		}

		if (BreakChance < 1) {
			BreakChance = 1;
		}

		if (zone->random.Roll(BreakChance)) {

			if (!TryFadeEffect(spellbonuses.Root[SBIndex::ROOT_BUFFSLOT])) {
				BuffFadeBySlot(spellbonuses.Root[SBIndex::ROOT_BUFFSLOT]);
				LogCombat("Spell broke root! BreakChance percent chance");
				return true;
			}
		}
	}

	LogCombat("Spell did not break root. BreakChance percent chance");
	return false;
}

int32 Mob::RuneAbsorb(int64 damage, uint16 type)
{
	uint32 buff_max = GetMaxTotalSlots();
	if (type == SpellEffect::Rune) {
		for (uint32 slot = 0; slot < buff_max; slot++) {
			if (slot == spellbonuses.MeleeRune[SBIndex::RUNE_BUFFSLOT] && spellbonuses.MeleeRune[SBIndex::RUNE_AMOUNT] && buffs[slot].melee_rune && IsValidSpell(buffs[slot].spellid)) {
				int melee_rune_left = buffs[slot].melee_rune;

				if (melee_rune_left > damage)
				{
					melee_rune_left -= damage;
					buffs[slot].melee_rune = melee_rune_left;
					return -6;
				}

				else
				{
					if (melee_rune_left > 0)
						damage -= melee_rune_left;

					if (!TryFadeEffect(slot))
						BuffFadeBySlot(slot);
				}
			}
		}
	}

	else {
		for (uint32 slot = 0; slot < buff_max; slot++) {
			if (slot == spellbonuses.AbsorbMagicAtt[SBIndex::RUNE_BUFFSLOT] && spellbonuses.AbsorbMagicAtt[SBIndex::RUNE_AMOUNT] && buffs[slot].magic_rune && IsValidSpell(buffs[slot].spellid)) {
				int magic_rune_left = buffs[slot].magic_rune;
				if (magic_rune_left > damage)
				{
					magic_rune_left -= damage;
					buffs[slot].magic_rune = magic_rune_left;
					return 0;
				}

				else
				{
					if (magic_rune_left > 0)
						damage -= magic_rune_left;

					if (!TryFadeEffect(slot))
						BuffFadeBySlot(slot);
				}
			}
		}
	}

	return damage;
}
//SYNC WITH: tune.cpp, mob.h TuneCommonOutgoingHitSucces
void Mob::CommonOutgoingHitSuccess(Mob* defender, DamageHitInfo &hit, ExtraAttackOptions *opts)
{
	if (!defender)
		return;

#ifdef LUA_EQEMU
	bool ignoreDefault = false;
	LuaParser::Instance()->CommonOutgoingHitSuccess(this, defender, hit, opts, ignoreDefault);

	if (ignoreDefault) {
		return;
	}
#endif

	// BER weren't parsing the halving
	if (hit.skill == EQ::skills::SkillArchery ||
		(hit.skill == EQ::skills::SkillThrowing && GetClass() != Class::Berserker))
		hit.damage_done /= 2;

	if (hit.damage_done < 1)
		hit.damage_done = 1;

	if (hit.skill == EQ::skills::SkillArchery) {
		int bonus = aabonuses.ArcheryDamageModifier + itembonuses.ArcheryDamageModifier + spellbonuses.ArcheryDamageModifier;
		hit.damage_done += hit.damage_done * bonus / 100;
		int headshot = TryHeadShot(defender, hit.skill);
		if (headshot > 0) {
			hit.damage_done = headshot;
		}
		else if (GetClass() == Class::Ranger && GetLevel() >= RuleI(Combat, ArcheryBonusLevelRequirement)) { // no double dmg on headshot
			if ((defender->IsNPC() && !defender->IsMoving() && !defender->IsRooted()) || !RuleB(Combat, ArcheryBonusRequiresStationary)) {
				hit.damage_done *= 2;
				MessageString(Chat::MeleeCrit, BOW_DOUBLE_DAMAGE);
			}
		}

		//Scale Factor for Archery Damage Tuning
		hit.damage_done *= RuleR(Combat, ArcheryBaseDamageBonus);
	}

	int extra_mincap = 0;
	int min_mod = hit.base_damage * GetMeleeMinDamageMod_SE(hit.skill) / 100;
	if (hit.skill == EQ::skills::SkillBackstab) {
		extra_mincap = GetLevel() < 7 ? 7 : GetLevel();
		if (GetLevel() >= 60)
			extra_mincap = GetLevel() * 2;
		else if (GetLevel() > 50)
			extra_mincap = GetLevel() * 3 / 2;
		if (IsSpecialAttack(eSpecialAttacks::ChaoticStab)) {
			hit.damage_done = extra_mincap;
		}
		else {
			int ass = TryAssassinate(defender, hit.skill);

			if (ass > 0) {
				hit.damage_done = ass;
			}
		}
	}
	else if (hit.skill == EQ::skills::SkillFrenzy && GetClass() == Class::Berserker && GetLevel() > 50) {
		extra_mincap = 4 * GetLevel() / 5;
	}

	// this has some weird ordering
	// Seems the crit message is generated before some of them :P

	// worn item +skill dmg, SPA 220, 418. Live has a normalized version that should be here too
	hit.min_damage += GetSkillDmgAmt(hit.skill) + GetPositionalDmgAmt(defender);

	// shielding mod2
	if (defender->itembonuses.MeleeMitigation)
		hit.min_damage -= hit.min_damage * defender->itembonuses.MeleeMitigation / 100;

	ApplyMeleeDamageMods(hit.skill, hit.damage_done, defender, opts);
	min_mod = std::max(min_mod, extra_mincap);
	if (min_mod && hit.damage_done < min_mod) // SPA 186
		hit.damage_done = min_mod;

	TryCriticalHit(defender, hit, opts);

	hit.damage_done += hit.min_damage;
	if (IsOfClientBot()) {
		int extra = 0;
		switch (hit.skill) {
			case EQ::skills::SkillThrowing:
			case EQ::skills::SkillArchery:
				extra = itembonuses.heroic_dex_ranged_damage;
				break;
			default:
				extra = itembonuses.heroic_str_melee_damage;
				break;
		}
		hit.damage_done += extra;
	}

	// this appears where they do special attack dmg mods
	int spec_mod = 0;
	if (IsSpecialAttack(eSpecialAttacks::Rampage)) {
		int mod = GetSpecialAbilityParam(SpecialAbility::Rampage, 2);
		if (mod > 0)
			spec_mod = mod;
		if ((IsPet() || IsTempPet()) && IsPetOwnerOfClientBot()) {
			//SE_PC_Pet_Rampage SPA 464 on pet, damage modifier
			int spell_mod = spellbonuses.PC_Pet_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] + itembonuses.PC_Pet_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] + aabonuses.PC_Pet_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD];
			if (spell_mod > spec_mod)
				spec_mod = spell_mod;
		}
	}
	else if (IsSpecialAttack(eSpecialAttacks::AERampage)) {
		int mod = GetSpecialAbilityParam(SpecialAbility::AreaRampage, 2);
		if (mod > 0)
			spec_mod = mod;
		if ((IsPet() || IsTempPet()) && IsPetOwnerOfClientBot()) {
			//SE_PC_Pet_AE_Rampage SPA 465 on pet, damage modifier
			int spell_mod = spellbonuses.PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] + itembonuses.PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] + aabonuses.PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD];
			if (spell_mod > spec_mod)
				spec_mod = spell_mod;
		}
	}
	if (spec_mod > 0)
		hit.damage_done = (hit.damage_done * spec_mod) / 100;

	int pct_damage_reduction = defender->GetSkillDmgTaken(hit.skill, opts) + defender->GetPositionalDmgTaken(this);

	hit.damage_done += (hit.damage_done * pct_damage_reduction / 100) + defender->GetPositionalDmgTakenAmt(this);

	if (defender->GetShielderID() || defender->spellbonuses.ShieldTargetSpa[SBIndex::SHIELD_TARGET_MITIGATION_PERCENT]) {
		bool use_shield_ability = true;
		//If defender is being shielded by an ability AND has a shield spell effect buff use highest mitigation value.
		if ((defender->GetShieldTargetMitigation() && defender->spellbonuses.ShieldTargetSpa[SBIndex::SHIELD_TARGET_MITIGATION_PERCENT]) &&
			 (defender->spellbonuses.ShieldTargetSpa[SBIndex::SHIELD_TARGET_MITIGATION_PERCENT] >= defender->GetShieldTargetMitigation())){
				bool use_shield_ability = false;
		}

		//use targeted /shield ability values
		if (defender->GetShielderID() && use_shield_ability) {
			DoShieldDamageOnShielder(defender, hit.damage_done, hit.skill);
			hit.damage_done -= hit.damage_done * defender->GetShieldTargetMitigation() / 100; //Default shielded takes 50 pct damage
		}
		//use spell effect SPA 463 values
		else if (defender->spellbonuses.ShieldTargetSpa[SBIndex::SHIELD_TARGET_MITIGATION_PERCENT]){
			DoShieldDamageOnShielderSpellEffect(defender, hit.damage_done, hit.skill);
			hit.damage_done -= hit.damage_done * defender->spellbonuses.ShieldTargetSpa[SBIndex::SHIELD_TARGET_MITIGATION_PERCENT] / 100;
		}
	}

	CheckNumHitsRemaining(NumHit::OutgoingHitSuccess);
}

void Mob::DoShieldDamageOnShielder(Mob *shield_target, int64 hit_damage_done, EQ::skills::SkillType skillInUse)
{
	if (!shield_target) {
		return;
	}

	Mob *shielder = entity_list.GetMob(shield_target->GetShielderID());
	if (!shielder) {
		shield_target->SetShielderID(0);
		shield_target->SetShieldTargetMitigation(0);
		return;
	}

	if (shield_target->CalculateDistance(shielder->GetX(), shielder->GetY(), shielder->GetZ()) > static_cast<float>(shielder->GetMaxShielderDistance())) {
		shielder->SetShieldTargetID(0);
		shielder->SetShielderMitigation(0);
		shielder->SetShielderMaxDistance(0);
		shielder->shield_timer.Disable();
		shield_target->SetShielderID(0);
		shield_target->SetShieldTargetMitigation(0);
		return; //Too far away, no message is given though.
	}

	int mitigation = shielder->GetShielderMitigation(); //Default shielder mitigates 25 pct of damage taken, this can be increased up to max 50 by equipping a shield item
	if (shielder->IsClient() && shielder->HasShieldEquipped()) {
		EQ::ItemInstance* inst = shielder->CastToClient()->GetInv().GetItem(EQ::invslot::slotSecondary);
		if (inst) {
			const EQ::ItemData* shield = inst->GetItem();
			if (shield && shield->ItemType == EQ::item::ItemTypeShield) {
				mitigation += shield->AC * 50 / 100; //1% increase per 2 AC
				mitigation = std::min(50, mitigation);//50 pct max mitigation bonus from /shield
			}
		}
	}

	hit_damage_done -= hit_damage_done * mitigation / 100;
	shielder->Damage(this, hit_damage_done, SPELL_UNKNOWN, skillInUse, true, -1, false, m_specialattacks);
}

void Mob::DoShieldDamageOnShielderSpellEffect(Mob* shield_target, int64 hit_damage_done, EQ::skills::SkillType skillInUse)
{
	if (!shield_target) {
		return;
	}
	/*
		SPA 463 SpellEffect::SHIELD_TARGET

		Live description: "Shields your target, taking a percentage of their damage".
		Only example spell on live is an NPC who uses it during a raid event "Laurion's Song" expansion. SPA 54492 'Guardian Stance' Described as 100% Melee Shielding

		Example of mechanic. Base value = 70. Caster puts buff on target. Each melee hit Buff Target takes 70% less damage, Buff Caster receives 30% of the melee damage.
		Added mechanic to cause buff to fade if target or caster are seperated by a distance greater than the casting range of the spell. This allows similiar mechanics
		to the /shield ability, without a range removal mechanic it would be too easy to abuse if put on a player spell. *can not confirm live does this currently

		Can not be cast on self.
	*/

	if (shield_target->spellbonuses.ShieldTargetSpa[SBIndex::SHIELD_TARGET_MITIGATION_PERCENT])
	{
		if (shield_target->spellbonuses.ShieldTargetSpa[SBIndex::SHIELD_TARGET_BUFFSLOT] >= 0)
		{
			Mob *shielder = entity_list.GetMob(shield_target->buffs[shield_target->spellbonuses.ShieldTargetSpa[SBIndex::SHIELD_TARGET_BUFFSLOT]].casterid);
			if (!shielder) {
				shield_target->BuffFadeBySlot(shield_target->spellbonuses.ShieldTargetSpa[SBIndex::SHIELD_TARGET_BUFFSLOT]);
				return;
			}

			int shield_spell_id = shield_target->buffs[shield_target->spellbonuses.ShieldTargetSpa[SBIndex::SHIELD_TARGET_BUFFSLOT]].spellid;
			if (!IsValidSpell(shield_spell_id)) {
				return;
			}

			float max_range = spells[shield_spell_id].range;
			if (spells[shield_spell_id].aoe_range > max_range) {
				max_range = spells[shield_spell_id].aoe_range;
			}
			max_range += 5.0f; //small buffer in case casted at exactly max range.

			if (shield_target->CalculateDistance(shielder->GetX(), shielder->GetY(), shielder->GetZ()) > max_range) {
				shield_target->BuffFadeBySlot(shield_target->spellbonuses.ShieldTargetSpa[SBIndex::SHIELD_TARGET_BUFFSLOT]);
				return;
			}

			int mitigation = 100 - shield_target->spellbonuses.ShieldTargetSpa[SBIndex::SHIELD_TARGET_MITIGATION_PERCENT];
			hit_damage_done -= hit_damage_done * mitigation / 100;
			shielder->Damage(this, hit_damage_done, SPELL_UNKNOWN, skillInUse, true, -1, false, m_specialattacks);
		}
	}
}

void Mob::CommonBreakInvisibleFromCombat()
{
	//break invis when you attack
	BreakInvisibleSpells();
	CancelSneakHide();

	if (spellbonuses.NegateIfCombat) {
		BuffFadeByEffect(SpellEffect::NegateIfCombat);
	}

	hidden = false;
	improved_hidden = false;
}

/* Dev quotes:
* Old formula
*	 Final delay = (Original Delay / (haste mod *.01f)) + ((Hundred Hands / 100) * Original Delay)
* New formula
*	 Final delay = (Original Delay / (haste mod *.01f)) + ((Hundred Hands / 1000) * (Original Delay / (haste mod *.01f))
* Base Delay	  20			  25			  30			  37
* Haste		   2.25			2.25			2.25			2.25
* HHE (old)	  -17			 -17			 -17			 -17
* Final Delay	 5.488888889	 6.861111111	 8.233333333	 10.15444444
*
* Base Delay	  20			  25			  30			  37
* Haste		   2.25			2.25			2.25			2.25
* HHE (new)	  -383			-383			-383			-383
* Final Delay	 5.484444444	 6.855555556	 8.226666667	 10.14622222
*
* Difference	 -0.004444444   -0.005555556   -0.006666667   -0.008222222
*
* These times are in 10th of a second
*/

void Mob::SetAttackTimer()
{
	attack_timer.SetAtTrigger(4000, true);
}

void Client::SetAttackTimer()
{
	float haste_mod = GetHaste() * 0.01f;
	int primary_speed = 0;
	int secondary_speed = 0;

	//default value for attack timer in case they have
	//an invalid weapon equipped:
	attack_timer.SetAtTrigger(4000, true);

	Timer *TimerToUse = nullptr;

	for (int i = EQ::invslot::slotRange; i <= EQ::invslot::slotSecondary; i++) {
		//pick a timer
		if (i == EQ::invslot::slotPrimary)
			TimerToUse = &attack_timer;
		else if (i == EQ::invslot::slotRange)
			TimerToUse = &ranged_timer;
		else if (i == EQ::invslot::slotSecondary)
			TimerToUse = &attack_dw_timer;
		else	//invalid slot (hands will always hit this)
			continue;

		const EQ::ItemData *ItemToUse = nullptr;

		//find our item
		EQ::ItemInstance *ci = GetInv().GetItem(i);
		if (ci)
			ItemToUse = ci->GetItem();

		//special offhand stuff
		if (i == EQ::invslot::slotSecondary) {
			//if we cant dual wield, skip it
			if (!CanThisClassDualWield() || HasTwoHanderEquipped()) {
				attack_dw_timer.Disable();
				continue;
			}
		}

		//see if we have a valid weapon
		if (ItemToUse != nullptr) {
			//check type and damage/delay
			if (!ItemToUse->IsClassCommon()
				|| ItemToUse->Damage == 0
				|| ItemToUse->Delay == 0) {
				//no weapon
				ItemToUse = nullptr;
			}
			// Check to see if skill is valid
			else if ((ItemToUse->ItemType > EQ::item::ItemTypeLargeThrowing) &&
				(ItemToUse->ItemType != EQ::item::ItemTypeMartial) &&
				(ItemToUse->ItemType != EQ::item::ItemType2HPiercing)) {
				//no weapon
				ItemToUse = nullptr;
			}
		}

		int hhe = itembonuses.HundredHands + spellbonuses.HundredHands;
		int speed = 0;
		int delay = 3500;

		//if we have no weapon..
		if (ItemToUse == nullptr)
			delay = 100 * GetHandToHandDelay();
		else
			//we have a weapon, use its delay
			delay = 100 * ItemToUse->Delay;

		speed = delay / haste_mod;

		if (ItemToUse && ItemToUse->ItemType == EQ::item::ItemTypeBow) {
			// Live actually had a bug here where they would return the non-modified attack speed
			// rather than the cap ...
			speed = std::max(speed - GetQuiverHaste(speed), RuleI(Combat, QuiverHasteCap));
		}
		else {
			if (RuleB(Spells, Jun182014HundredHandsRevamp))
				speed = static_cast<int>(speed + ((hhe / 1000.0f) * speed));
			else
				speed = static_cast<int>(speed + ((hhe / 100.0f) * delay));
		}

		bool reinit = !TimerToUse->Enabled();
		TimerToUse->SetAtTrigger(std::max(RuleI(Combat, MinHastedDelay), speed), reinit, reinit);

		if (i == EQ::invslot::slotPrimary) {
			primary_speed = speed;
		}
		else if (i == EQ::invslot::slotSecondary) {
			secondary_speed = speed;
		}
	}

	//To allow for duel wield animation to display correctly if both weapons have same delay
	if (primary_speed == secondary_speed) {
		SetDualWieldingSameDelayWeapons(1);
	}
	else {
		SetDualWieldingSameDelayWeapons(0);
	}
}

void NPC::SetAttackTimer()
{
	float haste_mod = GetHaste() * 0.01f;

	//default value for attack timer in case they have
	//an invalid weapon equipped:
	attack_timer.SetAtTrigger(4000, true);

	Timer *TimerToUse = nullptr;
	int hhe = itembonuses.HundredHands + spellbonuses.HundredHands;

	// Technically NPCs should do some logic for weapons, but the effect is minimal
	// What they do is take the lower of their set delay and the weapon's
	// ex. Mob's delay set to 20, weapon set to 19, delay 19
	// Mob's delay set to 20, weapon set to 21, delay 20
	int speed = 0;
	if (RuleB(Spells, Jun182014HundredHandsRevamp))
		speed = static_cast<int>((attack_delay / haste_mod) + ((hhe / 1000.0f) * (attack_delay / haste_mod)));
	else
		speed = static_cast<int>((attack_delay / haste_mod) + ((hhe / 100.0f) * attack_delay));

	for (int i = EQ::invslot::slotRange; i <= EQ::invslot::slotSecondary; i++) {
		//pick a timer
		if (i == EQ::invslot::slotPrimary)
			TimerToUse = &attack_timer;
		else if (i == EQ::invslot::slotRange)
			TimerToUse = &ranged_timer;
		else if (i == EQ::invslot::slotSecondary)
			TimerToUse = &attack_dw_timer;
		else	//invalid slot (hands will always hit this)
			continue;

		//special offhand stuff
		if (i == EQ::invslot::slotSecondary) {
			// SpecialAbility::QuadrupleAttack is uncheesable
			if (!CanThisClassDualWield() || (HasTwoHanderEquipped() && !GetSpecialAbility(SpecialAbility::QuadrupleAttack))) {
				attack_dw_timer.Disable();
				continue;
			}
		}

		TimerToUse->SetAtTrigger(std::max(RuleI(Combat, MinHastedDelay), speed), true, true);
	}
}

void Client::DoAttackRounds(Mob *target, int hand, bool IsFromSpell)
{
	if (!target || (target && target->IsCorpse())) {
		return;
	}

	Attack(target, hand, false, false, IsFromSpell);

	bool candouble = CanThisClassDoubleAttack();
	// extra off hand non-sense, can only double with skill of 150 or above
	// or you have any amount of GiveDoubleAttack
	if (candouble && hand == EQ::invslot::slotSecondary)
		candouble =
		    GetSkill(EQ::skills::SkillDoubleAttack) > 149 ||
		    (aabonuses.GiveDoubleAttack + spellbonuses.GiveDoubleAttack + itembonuses.GiveDoubleAttack) > 0;

	if (candouble) {
		CheckIncreaseSkill(EQ::skills::SkillDoubleAttack, target, -10);
		if (CheckDoubleAttack()) {
			Attack(target, hand, false, false, IsFromSpell);

			if (hand == EQ::invslot::slotPrimary) {

				if (HasTwoHanderEquipped()) {
					auto extraattackchance = aabonuses.ExtraAttackChance[SBIndex::EXTRA_ATTACK_CHANCE] + spellbonuses.ExtraAttackChance[SBIndex::EXTRA_ATTACK_CHANCE] +
											 itembonuses.ExtraAttackChance[SBIndex::EXTRA_ATTACK_CHANCE];
					if (extraattackchance && zone->random.Roll(extraattackchance)) {
						auto extraattackamt = std::max({aabonuses.ExtraAttackChance[SBIndex::EXTRA_ATTACK_NUM_ATKS], spellbonuses.ExtraAttackChance[SBIndex::EXTRA_ATTACK_NUM_ATKS], itembonuses.ExtraAttackChance[SBIndex::EXTRA_ATTACK_NUM_ATKS] });
						for (int i = 0; i < extraattackamt; i++) {
							Attack(target, hand, false, false, IsFromSpell);
						}
					}
				}
				else {
					auto extraattackchance_primary = aabonuses.ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_CHANCE] + spellbonuses.ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_CHANCE] +
													 itembonuses.ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_CHANCE];
					if (extraattackchance_primary && zone->random.Roll(extraattackchance_primary)) {
						auto extraattackamt_primary = std::max({aabonuses.ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_NUM_ATKS], spellbonuses.ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_NUM_ATKS], itembonuses.ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_NUM_ATKS] });
						for (int i = 0; i < extraattackamt_primary; i++) {
							Attack(target, hand, false, false, IsFromSpell);
						}
					}
				}
			}

			if (hand == EQ::invslot::slotSecondary) {
				auto extraattackchance_secondary = aabonuses.ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_CHANCE] + spellbonuses.ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_CHANCE] +
												   itembonuses.ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_CHANCE];
				if (extraattackchance_secondary && zone->random.Roll(extraattackchance_secondary)) {
					auto extraattackamt_secondary = std::max({aabonuses.ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_NUM_ATKS], spellbonuses.ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_NUM_ATKS], itembonuses.ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_NUM_ATKS] });
					for (int i = 0; i < extraattackamt_secondary; i++) {
						Attack(target, hand, false, false, IsFromSpell);
					}
				}
			}

			// you can only triple from the main hand
			if (hand == EQ::invslot::slotPrimary && CanThisClassTripleAttack()) {
				if (!RuleB(Combat, ClassicTripleAttack)) {
					CheckIncreaseSkill(EQ::skills::SkillTripleAttack, target, -10);
				}

				if (CheckTripleAttack()) {
					Attack(target, hand, false, false, IsFromSpell);
					int flurry_chance = aabonuses.FlurryChance + spellbonuses.FlurryChance +
							    itembonuses.FlurryChance;

					if (flurry_chance && zone->random.Roll(flurry_chance)) {
						Attack(target, hand, false, false, IsFromSpell);

						if (zone->random.Roll(flurry_chance)) {
							Attack(target, hand, false, false, IsFromSpell);
						}
						MessageString(Chat::NPCFlurry, YOU_FLURRY);
					}
				}
			}
		}
	}
}

bool Mob::CheckDualWield()
{
	// Pets /might/ follow a slightly different progression
	// although it could all be from pets having different skills than most mobs
	int chance = GetSkill(EQ::skills::SkillDualWield);
	if (GetLevel() > 35)
		chance += GetLevel();

	chance += aabonuses.Ambidexterity + spellbonuses.Ambidexterity + itembonuses.Ambidexterity;
	int per_inc = spellbonuses.DualWieldChance + aabonuses.DualWieldChance + itembonuses.DualWieldChance;
	if (per_inc)
		chance += chance * per_inc / 100;

	return zone->random.Int(1, 375) <= chance;
}

bool Client::CheckDualWield()
{
	int chance = GetSkill(EQ::skills::SkillDualWield) + GetLevel();

	chance += aabonuses.Ambidexterity + spellbonuses.Ambidexterity + itembonuses.Ambidexterity;
	int per_inc = spellbonuses.DualWieldChance + aabonuses.DualWieldChance + itembonuses.DualWieldChance;
	if (per_inc)
		chance += chance * per_inc / 100;

	return zone->random.Int(1, 375) <= chance;
}

void Mob::DoMainHandAttackRounds(Mob *target, ExtraAttackOptions *opts, bool rampage)
{
	if (!target) {
		return;
	}

	if (RuleB(Combat, UseLiveCombatRounds)) {
		// A "quad" on live really is just a successful dual wield where both double attack
		// The mobs that could triple lost the ability to when the triple attack skill was added in
		Attack(target, EQ::invslot::slotPrimary, false, false, false, opts);
		if (CanThisClassDoubleAttack() && CheckDoubleAttack()) {
			Attack(target, EQ::invslot::slotPrimary, false, false, false, opts);
			if ((IsPet() || IsTempPet()) && IsPetOwnerOfClientBot()) {
				int chance = spellbonuses.PC_Pet_Flurry + itembonuses.PC_Pet_Flurry + aabonuses.PC_Pet_Flurry;
				if (chance && zone->random.Roll(chance)) {
					Flurry(nullptr);
				}
			}
		}
		return;
	}

	if (IsNPC()) {
		int16 n_atk = CastToNPC()->GetNumberOfAttacks();
		if (n_atk <= 1 || rampage) {
			Attack(target, EQ::invslot::slotPrimary, false, false, false, opts);
		} else {
			for (int i = 0; i < n_atk; ++i) {
				Attack(target, EQ::invslot::slotPrimary, false, false, false, opts);
			}
		}
	} else {
		Attack(target, EQ::invslot::slotPrimary, false, false, false, opts);
	}

	// we use this random value in three comparisons with different
	// thresholds, and if its truely random, then this should work
	// out reasonably and will save us compute resources.
	int32 RandRoll = zone->random.Int(0, 99);
	if ((CanThisClassDoubleAttack() || GetSpecialAbility(SpecialAbility::TripleAttack) || GetSpecialAbility(SpecialAbility::QuadrupleAttack))
		// check double attack, this is NOT the same rules that clients use...
		&&
		RandRoll < (GetLevel() + NPCDualAttackModifier)) {
		Attack(target, EQ::invslot::slotPrimary, false, false, false, opts);
		// lets see if we can do a triple attack with the main hand
		// pets are excluded from triple and quads...
		if ((GetSpecialAbility(SpecialAbility::TripleAttack) || GetSpecialAbility(SpecialAbility::QuadrupleAttack)) && !IsPet() &&
			RandRoll < (GetLevel() + NPCTripleAttackModifier)) {
			Attack(target, EQ::invslot::slotPrimary, false, false, false, opts);
			// now lets check the quad attack
			if (GetSpecialAbility(SpecialAbility::QuadrupleAttack) && RandRoll < (GetLevel() + NPCQuadAttackModifier)) {
				Attack(target, EQ::invslot::slotPrimary, false, false, false, opts);
			}
		}
	}
}

void Mob::DoOffHandAttackRounds(Mob *target, ExtraAttackOptions *opts, bool rampage)
{
	if (!target) {
		return;
	}

	// Mobs will only dual wield w/ the flag or have a secondary weapon
	// For now, SpecialAbility::QuadrupleAttack means innate DW when Combat:UseLiveCombatRounds is true
	if ((GetSpecialAbility(SpecialAbility::DualWield) ||
		(RuleB(Combat, UseLiveCombatRounds) && GetSpecialAbility(SpecialAbility::QuadrupleAttack))) ||
		GetEquippedItemFromTextureSlot(EQ::textures::weaponSecondary) != 0) {
		if (CheckDualWield()) {
			Attack(target, EQ::invslot::slotSecondary, false, false, false, opts);
			if (CanThisClassDoubleAttack() && GetLevel() > 35 && CheckDoubleAttack() && !rampage) {
				Attack(target, EQ::invslot::slotSecondary, false, false, false, opts);

				if ((IsPet() || IsTempPet()) && IsPetOwnerOfClientBot()) {
					int chance = spellbonuses.PC_Pet_Flurry + itembonuses.PC_Pet_Flurry + aabonuses.PC_Pet_Flurry;
					if (chance && zone->random.Roll(chance)) {
						Flurry(nullptr);
					}
				}
			}
		}
	}
}


int Mob::GetPetAvoidanceBonusFromOwner()
{
	Mob *owner = nullptr;
	if (IsPet())
		owner = GetOwner();
	else if (IsNPC() && CastToNPC()->GetSwarmOwner())
		owner = entity_list.GetMobID(CastToNPC()->GetSwarmOwner());

	if (owner)
		return owner->aabonuses.PetAvoidance + owner->spellbonuses.PetAvoidance + owner->itembonuses.PetAvoidance;

	return 0;
}
int Mob::GetPetACBonusFromOwner()
{
	Mob *owner = nullptr;
	if (IsPet())
		owner = GetOwner();
	else if (IsNPC() && CastToNPC()->GetSwarmOwner())
		owner = entity_list.GetMobID(CastToNPC()->GetSwarmOwner());

	if (owner)
		return owner->aabonuses.PetMeleeMitigation + owner->spellbonuses.PetMeleeMitigation + owner->itembonuses.PetMeleeMitigation;

	return 0;
}
int Mob::GetPetATKBonusFromOwner()
{
	Mob *owner = nullptr;
	if (IsPet())
		owner = GetOwner();
	else if (IsNPC() && CastToNPC()->GetSwarmOwner())
		owner = entity_list.GetMobID(CastToNPC()->GetSwarmOwner());

	if (owner)
		return owner->aabonuses.Pet_Add_Atk + owner->spellbonuses.Pet_Add_Atk + owner->itembonuses.Pet_Add_Atk;

	return 0;
}


bool Mob::GetWasSpawnedInWater() const {
	return spawned_in_water;
}

void Mob::SetSpawnedInWater(bool spawned_in_water) {
	Mob::spawned_in_water = spawned_in_water;
}

int64 Mob::GetHPRegen() const
{
	return hp_regen;
}

int64 Mob::GetHPRegenPerSecond() const
{
	return hp_regen_per_second;
}

int64 Mob::GetManaRegen() const
{
	return mana_regen;
}

int64 Mob::GetEnduranceRegen() const
{
	return 0; // not implemented
}
