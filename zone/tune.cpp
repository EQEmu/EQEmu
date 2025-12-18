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

#include "common/data_verification.h"
#include "common/eq_constants.h"
#include "common/eq_packet_structs.h"
#include "common/rulesys.h"
#include "common/spdat.h"
#include "common/strings.h"
#include "zone/bot.h"
#include "zone/fastmath.h"
#include "zone/lua_parser.h"
#include "zone/queryserv.h"
#include "zone/string_ids.h"
#include "zone/water_map.h"
#include "zone/worldserver.h"
#include "zone/zone.h"

extern QueryServ* QServ;
extern WorldServer worldserver;
extern FastMath g_Math;
extern EntityList entity_list;
extern Zone* zone;


void Mob::TuneGetStats(Mob* defender, Mob *attacker)
{
	if (!defender || !attacker) {
		Message(0, "#Tune - Processing... Abort! Can not find attacker or defender");
		return;
	}
	int max_damage = 0;
	int min_damage = 0;
	int mean_dmg = 0;
	float tmp_pct_mitigated = 0.0f;
	float hit_chance = 0.0f;

	max_damage = attacker->TuneClientGetMaxDamage(defender);
	min_damage = attacker->TuneClientGetMinDamage(defender, max_damage);

	if (!max_damage)
	{
		Message(0, "#Tune - Processing... Abort! Damage not found! [MaxDMG %i MinDMG %i]", max_damage, min_damage);
		return;
	}
	mean_dmg = attacker->TuneClientGetMeanDamage(defender);
	tmp_pct_mitigated = 100.0f - (static_cast<float>(mean_dmg) * 100.0f / static_cast<float>(max_damage));

	hit_chance = TuneGetHitChance(defender, attacker);

	Message(0, "#STATS#############START######################");
	Message(0, "[#Tune] Defender Statistics vs Attacker");
	Message(0, "[#Tune] Defender Name: %s", defender->GetCleanName());
	Message(0, "[#Tune] AC Mitigation pct:   %.0f pct ", round(tmp_pct_mitigated));
	Message(0, "[#Tune] Total AC: %i ", defender->TuneACSum());
	Message(0, "[#Tune] Mean Damage Taken: %i per hit", mean_dmg);
	Message(0, "[#Tune] Chance to be missed:  %.0f pct", (100.0f - round(hit_chance)));
	Message(0, "[#Tune] Avoidance: %i ", TuneGetAvoidance(defender, attacker));
	Message(0, "[#Tune] Riposte Chance: %.0f pct ", round(TuneGetAvoidMeleeChance(defender, attacker, DMG_RIPOSTED)));
	Message(0, "[#Tune] Block Chance: %.0f pct ", round(TuneGetAvoidMeleeChance(defender, attacker, DMG_BLOCKED)));
	Message(0, "[#Tune] Parry Chance: %.0f pct ", round(TuneGetAvoidMeleeChance(defender, attacker, DMG_PARRIED)));
	Message(0, "[#Tune] Dodge Chance: %.0f pct ", round(TuneGetAvoidMeleeChance(defender, attacker, DMG_DODGED)));

	if (defender->IsNPC())
	{
		Message(0, "[#Tune] NPC STAT AC: %i ", static_cast<int>(defender->CastToNPC()->GetNPCStat("ac")));
		Message(0, "[#Tune] NPC STAT Avoidance: %i ", static_cast<int>(defender->CastToNPC()->GetNPCStat("avoidance")));
	}

	Message(0, "################################################");
	Message(0, "[#Tune] Attacker Statistics vs Defender");
	Message(0, "[#Tune] Attacker Name: %s", attacker->GetCleanName());

	if (max_damage > 0) {
		Message(0, "[#Tune] Max Damage %i Min Damage %i", max_damage, min_damage);
		Message(0, "[#Tune] Total Offense: %i ", TuneGetOffense(defender, attacker));
		Message(0, "[#Tune] Chance to hit:  %.0f pct", round(hit_chance));
		Message(0, "[#Tune] Total Accuracy: %i ", TuneGetAccuracy( defender,attacker));

		if (attacker->IsNPC())
		{
			Message(0, "[#Tune] NPC STAT ATK: %i ", static_cast<int>(attacker->CastToNPC()->GetNPCStat("atk")));
			Message(0, "[#Tune] NPC STAT Accuracy: %i ", static_cast<int>(attacker->CastToNPC()->GetNPCStat("accuracy")));
		}
	}
	else{
		Message(0, "[#Tune] Can not melee this target");
	}

	Message(0, "#STATS#############COMPLETE###################");
	return;
}

void Mob::TuneGetACByPctMitigation(Mob* defender, Mob *attacker, float pct_mitigation, int interval, int max_loop, int atk_override, int Msg)
{
	Message(0, " ");
	/*
		Find the amount of AC stat that has to be added/subtracted from DEFENDER to reach a specific average mitigation value based on ATTACKER's offense statistics.
		Can use atk_override to find the value verse a hypothetical amount of worn ATK
	*/

	if (pct_mitigation > 100 || pct_mitigation < 0) {
		Message(0, "[#Tune] - Processing... Abort! Mitigation value out of range ( %.0f ) pct. Must be between 0-100.", pct_mitigation);
		return;
	}
	if (!defender) {
		Message(0, "[#Tune] - Processing... Abort! No Defender found.");
		return;
	}
	if (!attacker) {
		Message(0, "[#Tune] - Processing... Abort! No Attacker found.");
		return;
	}
	if (defender->GetID() == attacker->GetID()) {
		Message(0, "[#Tune] - Processing... Abort! Error Attacker can not be the Defender.");
		return;
	}

	int max_damage = 0;
	int min_damage = 0;
	int mean_dmg = 0;
	float tmp_pct_mitigated = 0.0f;
	float base_pct_mitigation = pct_mitigation;
	int loop_add_ac = 0;
	int end = 0;
	int value = 0;


	max_damage = attacker->TuneClientGetMaxDamage(defender);
	min_damage = attacker->TuneClientGetMinDamage(defender, max_damage);

	if (!max_damage)
	{
		Message(0, "#Tune - Processing... Abort! Damage not found! [MaxDMG %i MinDMG %i]", max_damage, min_damage);
		return;
	}

	//Obtain baseline mitigation for current stats
	mean_dmg = attacker->TuneClientGetMeanDamage(defender,0,atk_override);
	tmp_pct_mitigated = 100.0f - (static_cast<float>(mean_dmg) * 100.0f / static_cast<float>(max_damage));

	Message(0, "###################START###################");
	Message(0, "[#Tune] DFENDER Name: %s", defender->GetCleanName());
	Message(0, "[#Tune] DEFENDER AC Mitigation pct:   %.0f pct ", round(tmp_pct_mitigated));
	Message(0, "[#Tune] DEFENDER Total AC: %i ", defender->TuneACSum());
	Message(0, "[#Tune] ATTACKER Name: %s", attacker->GetCleanName());
	Message(0, "[#Tune] ATTACKER Max Damage %i Min Damage %i", max_damage, min_damage);
	Message(0, "[#Tune] ATTACKER Total Offense: %i ", TuneGetOffense(defender, attacker, atk_override));
	Message(0, "##########################################");
	Message(0, "[#Tune] Begin Parse [Interval %i Max Loop Iterations %i]", interval, max_loop);

	if (tmp_pct_mitigated > pct_mitigation)
	{
		interval = interval * -1;
		Message(0, "[#Tune] NOTE: Defenders 'AC' must be LOWERED due to defenders AC Mitigation ( %.0f pct ) being greater than the desired ( %.0f pct )", tmp_pct_mitigated, pct_mitigation);
	}

	Message(0, "[#Tune] Processing... Find AC for defender to have Mitigation of ( %.0f pct ) agianst this attacker.", pct_mitigation);


	for (int j = 0; j < max_loop; j++)
	{
		mean_dmg = attacker->TuneClientGetMeanDamage(defender, 0, atk_override, loop_add_ac,0);
		tmp_pct_mitigated = 100.0f - (static_cast<float>(mean_dmg) * 100.0f / static_cast<float>(max_damage));

		if (Msg >= 1)
		{
			Message(0, "[#Tune] - Processing... [%i] [AC %i] Average Melee Hit  %i | Pct Mitigated %.2f ", j, loop_add_ac, mean_dmg, tmp_pct_mitigated);
		}

		if (interval > 0 && tmp_pct_mitigated >= pct_mitigation)
		{
			end = 1;
		}

		else if (interval < 0 && tmp_pct_mitigated <= pct_mitigation)
		{
			end = 1;
		}

		else if (interval < 0 && mean_dmg == min_damage)
		{
			Message(0, "[#Tune][WARNING] Mitigation can not be further decreased due to minium hit value (%i). Minium mitigation ( %.0f ) pct", min_damage, tmp_pct_mitigated);
			base_pct_mitigation = tmp_pct_mitigated;
			end = 1;
		}

		if (end >= 1) {

			Message(0, "###################RESULTS###################");

			if (atk_override) {
				Message(0, "[#Tune] ATK STAT OVERRRIDE. This is the amount of AC adjustment needed if this attacker had ( %i ) raw ATK stat", atk_override);
			}

			if (defender->IsNPC())
			{

				Message(0, "[#Tune] Recommended NPC RAW AC ADJUSTMENT ( %i ) on ' %s ' to acheive an average mitigation of ( %.0f pct ) verse ' %s '", loop_add_ac, defender->GetCleanName(), base_pct_mitigation, attacker->GetCleanName());
				Message(0, "[#Tune] SET NPC 'AC' stat value = [ %i ]", loop_add_ac + defender->CastToNPC()->GetRawAC());
				Message(0, "###################COMPLETE###################");
			}
			if (defender->IsClient())
			{
				Message(0, "[#Tune] Recommended CLIENT AC ADJUSTMENT ( %i ) on ' %s '  to acheive an average mitigation of ( %.0f pct ) verse ' %s '", loop_add_ac, defender->GetCleanName(), base_pct_mitigation, attacker->GetCleanName());

				if (loop_add_ac >= 0) {
					Message(0, "[#Tune] MODIFY Client Item AC or Spell Effect AC by [+ %i ]", loop_add_ac);
				}
				else {
					Message(0, "[#Tune] MODIFY Client Item AC or Spell Effect AC by [ %i ]", loop_add_ac);
				}

				Message(0, "###################COMPLETE###################");
			}

			return;
		}

		loop_add_ac = loop_add_ac + interval;
	}

	Message(0, "###################ABORT#######################");
	Message(0, "[#Tune] - Error: Unable to find desired result for ( %.0f pct ) - Increase interval ( %i ) AND/OR max loop value ( %i ) and run again.", pct_mitigation, interval, max_loop);
	Message(0, "[#Tune] - Parse ended at an AC ADJUSTMENT of ( %i ) on ' %s ' to acheive an average mitigation of ( %.0f pct ) verse ' %s '", loop_add_ac, attacker->GetCleanName(), tmp_pct_mitigated, defender->GetCleanName());
	Message(0, "###################COMPLETE###################");
	return;
}

void Mob::TuneGetATKByPctMitigation(Mob* defender, Mob *attacker, float pct_mitigation, int interval, int max_loop, int ac_override, int Msg)
{
	Message(0, " ");
	/*
		Find the amount of ATK stat that has to be added/subtracted from ATTACKER to reach a specific average mitigation value based on DEFENDERS's mitigation statistics.
		Can use ac_override to find the value verse a hypothetical amount of worn AC
	*/
	if (pct_mitigation > 100 || pct_mitigation < 0) {
		Message(0, "[#Tune] - Processing... Abort! Mitigation value out of range ( %.0f ) pct. Must be between 0-100.", pct_mitigation);
		return;
	}
	if (!defender) {
		Message(0, "[#Tune] - Processing... Abort! No Defender found.");
		return;
	}
	if (!attacker) {
		Message(0, "[#Tune] - Processing... Abort! No Attacker found.");
		return;
	}
	if (defender->GetID() == attacker->GetID()) {
		Message(0, "[#Tune] - Processing... Abort! Error Attacker can not be the Defender.");
		return;
	}

	int max_damage = 0;
	int min_damage = 0;
	int mean_dmg = 0;
	float tmp_pct_mitigated = 0.0f;
	float base_pct_mitigation = pct_mitigation;
	int loop_add_atk = 0;
	int end = 0;
	int value = 0;


	max_damage = attacker->TuneClientGetMaxDamage(defender);
	min_damage = attacker->TuneClientGetMinDamage(defender, max_damage);

	if (!max_damage)
	{
		Message(0, "#Tune - Processing... Abort! Damage not found! [MaxDMG %i MinDMG %i]", max_damage, min_damage);
		return;
	}

	//Obtain baseline mitigation for current stats
	mean_dmg = attacker->TuneClientGetMeanDamage(defender, ac_override);
	tmp_pct_mitigated = 100.0f - (static_cast<float>(mean_dmg) * 100.0f / static_cast<float>(max_damage));

	Message(0, "###################START###################");
	Message(0, "[#Tune] DFENDER Name: %s", defender->GetCleanName());
	Message(0, "[#Tune] DEFENDER AC Mitigation pct:   %.0f pct ", round(tmp_pct_mitigated));
	Message(0, "[#Tune] DEFENDER Total AC: %i ", defender->TuneACSum(false, ac_override));
	Message(0, "[#Tune] ATTACKER Name: %s", attacker->GetCleanName());
	Message(0, "[#Tune] ATTACKER Max Damage %i Min Damage %i", max_damage, min_damage);
	Message(0, "[#Tune] ATTACKER Total Offense: %i ", TuneGetOffense(defender, attacker));
	Message(0, "##########################################");
	Message(0, "[#Tune] Begin Parse [Interval %i Max Loop Iterations %i]", interval, max_loop);

	if (tmp_pct_mitigated < pct_mitigation) {
		interval = interval * -1;
		Message(0, "[#Tune] NOTE: Attackers 'ATK' must be LOWERED due to defenders AC Mitigation ( %.0f pct ) being less than the desired ( %.0f pct )", tmp_pct_mitigated, pct_mitigation);
	}

	Message(0, "[#Tune] Processing... Find ATK on attacker for defender to have Mitigation of ( %.0f pct ) agianst this attacker.", pct_mitigation);

	for (int j = 0; j < max_loop; j++)
	{
		mean_dmg = attacker->TuneClientGetMeanDamage(defender, ac_override, 0, 0, loop_add_atk);
		tmp_pct_mitigated = 100.0f - (static_cast<float>(mean_dmg) * 100.0f / static_cast<float>(max_damage));

		if (Msg >= 3)
		{
			Message(0, "[#Tune] - Processing... [%i] [ATK %i] Average Melee Hit  %i | Pct Mitigated %.2f ", j, loop_add_atk, mean_dmg, tmp_pct_mitigated);
		}

		if (interval > 0 && tmp_pct_mitigated <= pct_mitigation) {
			end = 1;
		}
		else if (interval < 0 && tmp_pct_mitigated >= pct_mitigation) {
			end = 1;
		}

		else if (interval < 0 && mean_dmg == min_damage)
		{
			Message(0, "[#Tune] [WARNING] Mitigation can not be further decreased due to minium hit value ( %i ). Minium mitigation ( %.0f pct )", min_damage, tmp_pct_mitigated);
			base_pct_mitigation = tmp_pct_mitigated;
			end = 1;
		}

		if (end >= 1) {

			Message(0, "###################RESULTS###################");

			if (ac_override) {
				Message(0, "[#Tune] AC STAT OVERRRIDE. This is the amount of ATK adjustment needed if this defender had ( %i ) raw AC stat", ac_override);
			}

			if (attacker->IsNPC()) {
				Message(0, "[#Tune] Recommended NPC ATK ADJUSTMENT ( %i ) on ' %s ' so that their hits on average are mitgiated by ( %.0f pct ) verse ' %s '. ", loop_add_atk, attacker->GetCleanName(), base_pct_mitigation, defender->GetCleanName());
				Message(0, "[#Tune] SET NPC 'ATK' stat value = [ %i ]", loop_add_atk + defender->CastToNPC()->ATK);
				Message(0, "###################COMPLETE###################");
			}
			if (attacker->IsClient()) {
				Message(0, "[#Tune] Recommended CLIENT ATK ADJUSTMENT ( %i ) on ' %s ' so that their hits on average are mitigated by ( %.0f pct ) verse ' %s '. ", loop_add_atk, attacker->GetCleanName(), base_pct_mitigation, defender->GetCleanName());

				if (loop_add_atk >= 0) {
					Message(0, "[#Tune] MODIFY Client Item ATK or Spell Effect ATK by [+ %i ]", loop_add_atk);
				}
				else {
					Message(0, "[#Tune] MODIFY Client Item ATK or Spell Effect ATK by [ %i ]", loop_add_atk);
				}

				Message(0, "###################COMPLETE###################");
			}

			return;
		}

		loop_add_atk = loop_add_atk + interval;
	}

	Message(0, "###################ABORT#######################");
	Message(0, "[#Tune] - Error: Unable to find desired result for ( %.0f pct ) - Increase interval ( %i ) AND/OR max loop value ( %i ) and run again.", pct_mitigation, interval, max_loop);
	Message(0, "[#Tune] - Parse ended at an ATK ADJUSTMENT of ( %i ) on ' %s ' so that their hits on average are mitigated by ( %.0f pct ) verse ' %s '.", loop_add_atk, attacker->GetCleanName(), tmp_pct_mitigated, defender->GetCleanName());
	Message(0, "###################COMPLETE###################");
	return;
}

void Mob::TuneGetAvoidanceByHitChance(Mob* defender, Mob *attacker, float hit_chance, int interval, int max_loop, int accuracy_override, int Msg)
{
	Message(0, " ");
	if (hit_chance > 100 || hit_chance < 0) {
		Message(0, "[#Tune] - Processing... Abort! Hit Chance value out of range ( %.0f ) pct. Must be between 0-100.", hit_chance);
		return;
	}
	if (!defender) {
		Message(0, "[#Tune] - Processing... Abort! No Defender found.");
		return;
	}
	if (!attacker) {
		Message(0, "[#Tune] - Processing... Abort! No Attacker found.");
		return;
	}
	if (defender->GetID() == attacker->GetID()) {
		Message(0, "[#Tune] - Processing... Abort! Error Attacker can not be the Defender.");
		return;
	}

	int loop_add_avoid = 0;
	float tmp_hit_chance = 0.0f;
	bool end = false;
	int base_avoidance = TuneGetAvoidance(defender, attacker);

	tmp_hit_chance = TuneGetHitChance(defender, attacker, 0, accuracy_override);

	Message(0, "###################START###################");
	Message(0, "[#Tune] DEFENDER Name: %s", defender->GetCleanName());
	Message(0, "[#Tune] DEFENDER Chance to be missed:  %.0f pct", (100.0f - round(tmp_hit_chance)));
	Message(0, "[#Tune] DEFENDER Avoidance: %i ", TuneGetAvoidance(defender, attacker));
	Message(0, "[#Tune] ATTACKER Name: %s", attacker->GetCleanName());
	Message(0, "[#Tune] ATTACKER Chance to hit:  %.0f pct", round(tmp_hit_chance));
	Message(0, "[#Tune] ATTACKER Accuracy: %i ", TuneGetAccuracy(defender, attacker, accuracy_override));
	Message(0, "##########################################");
	Message(0, "[#Tune] Begin Parse [Interval %i Max Loop Iterations %i]", interval, max_loop);

	if (tmp_hit_chance < hit_chance) {
		interval = interval * -1;
		Message(0, "[#Tune] NOTE: Defenders 'AVOIDANCE' must be LOWERED due to defenders ( %.0f pct ) chance to be hit being less than the desired ( %.0f pct )", tmp_hit_chance, hit_chance);
	}

	Message(0, "[#Tune] - Processing... Find Avoidance needed on defender for a ( %.0f pct ) hit chance from attacker. Base attacker hit chance ( %.0f pct ). ", hit_chance, tmp_hit_chance);

	for (int j = 0; j < max_loop; j++)
	{
		tmp_hit_chance = TuneGetHitChance(defender, attacker,0, accuracy_override, loop_add_avoid,0);

		if (Msg >= 3)
		{
			Message(0, "[#Tune] - Processing... [%i] AVOIDANCE %i | Hit Chance %.2f ", j, loop_add_avoid, tmp_hit_chance);
		}

		if (interval > 0 && tmp_hit_chance <= hit_chance)
		{
			end = true;
		}
		else if (interval < 0 && tmp_hit_chance >= hit_chance)
		{
			end = true;
		}

		if (end) {

			Message(0, "###################RESULTS###################");

			if (accuracy_override) {
				Message(0, "[#Tune] ACCURACY STAT OVERRRIDE. This is the amount of AVOIDANCE adjustment needed if this attacker had ( %i ) raw ACCURACY stat", accuracy_override);
			}

			if (defender->IsNPC()) {
				Message(0, "[#Tune] Recommended NPC AVOIDANCE ADJUSTMENT of ( %i ) on ' %s ' will result in ' %s ' having a ( %.0f pct) hit chance.", loop_add_avoid, defender->GetCleanName(), attacker->GetCleanName(), hit_chance);
				Message(0, "[#Tune] SET NPC 'AVOIDANCE' stat value = [ %i ]", loop_add_avoid + defender->CastToNPC()->GetAvoidanceRating());
				Message(0, "###################COMPLETE###################");
			}
			else if (defender->IsClient()) {
				Message(0, "[#Tune] Recommended CLIENT AVOIDANCE ADJUSTMENT of ( %i ) on ' %s ' will result in ' %s ' having a ( %.0f pct) hit chance.", loop_add_avoid, defender->GetCleanName(), attacker->GetCleanName(), hit_chance);

				int final_avoidance = TuneGetAvoidance(defender, attacker, 0, loop_add_avoid);
				int evasion_bonus = TuneCalcEvasionBonus(final_avoidance, base_avoidance);

				if (loop_add_avoid >= 0) {
					Message(0, "[#Tune] OPTION1: MODIFY Client Heroic AGI or Avoidance Mod2 stat by [+ %i ]", loop_add_avoid);
					Message(0, "[#Tune] OPTION2: Give CLIENT an evasion bonus using SPA 172 Evasion SpellEffect::AvoidMeleeChance from (spells/items/aa) of [+ %i pct ]", evasion_bonus);

				}
				else {
					Message(0, "[#Tune] OPTION1: MODIFY Client Heroic AGI or Avoidance Mod2 stat by [ %i ]", loop_add_avoid);
					Message(0, "[#Tune] OPTION2: Give CLIENT an evasion bonus using SPA 172 Evasion SpellEffect::AvoidMeleeChance from (spells/items/aa) of [ %i pct ]", evasion_bonus);
				}

				Message(0, "###################COMPLETE###################");
			}

			return;
		}

		loop_add_avoid = loop_add_avoid + interval;
	}

	Message(0, "###################ABORT#######################");
	Message(0, "[#Tune] Error: Unable to find desired result for ( %.0f pct) - Increase interval (%i) AND/OR max loop value (%i) and run again.", hit_chance, interval, max_loop);
	Message(0, "[#Tune] Parse ended at AVOIDANCE ADJUSTMENT ( %i ) on ' %s ' will result in ' %s ' having a ( %.0f pct) hit chance.", loop_add_avoid, defender->GetCleanName(), hit_chance, attacker->GetCleanName());
	Message(0, "###################COMPLETE###################");
}

void Mob::TuneGetAccuracyByHitChance(Mob* defender, Mob *attacker, float hit_chance, int interval, int max_loop, int avoidance_override, int Msg)
{
	Message(0, " ");
	if (hit_chance > 100 || hit_chance < 0) {
		Message(0, "[#Tune] - Processing... Abort! Hit Chance value out of range ( %.0f ) pct. Must be between 0-100.", hit_chance);
		return;
	}
	if (!defender) {
		Message(0, "[#Tune] - Processing... Abort! No Defender found.");
		return;
	}
	if (!attacker) {
		Message(0, "[#Tune] - Processing... Abort! No Attacker found.");
		return;
	}
	if (defender->GetID() == attacker->GetID()) {
		Message(0, "[#Tune] - Processing... Abort! Error Attacker can not be the Defender.");
		return;
	}

	int loop_add_accuracy = 0;
	float tmp_hit_chance = 0.0f;
	bool end = false;

	tmp_hit_chance = TuneGetHitChance(defender, attacker, avoidance_override);

	Message(0, "###################START###################");
	Message(0, "[#Tune] DEFENDER Name: %s", defender->GetCleanName());
	Message(0, "[#Tune] DEFENDER Chance to be missed:  %.0f pct", (100.0f - round(tmp_hit_chance)));
	Message(0, "[#Tune] DEFENDER Avoidance: %i ", TuneGetAvoidance(defender, attacker, avoidance_override));
	Message(0, "[#Tune] ATTACKER Name: %s", attacker->GetCleanName());
	Message(0, "[#Tune] ATTACKER Chance to hit:  %.0f pct", round(tmp_hit_chance));
	Message(0, "[#Tune] ATTACKER Accuracy: %i ", TuneGetAccuracy(defender, attacker));
	Message(0, "##########################################");
	Message(0, "[#Tune] Begin Parse [Interval %i Max Loop Iterations %i]", interval, max_loop);


	if (tmp_hit_chance > hit_chance) {
		interval = interval * -1;
		Message(0, "[#Tune] NOTE: Attackers 'ACCURACY' must be LOWERED due to attackers ( %.0f pct ) chance to hit being less than the desired ( %.0f pct )", tmp_hit_chance, hit_chance);
	}

	Message(0, "[#Tune] - Processing... Find Accuracy needed on attacker for a ( %.0f pct ) hit chance on defender. Base attacker hit chance ( %.0f pct ). ", hit_chance, tmp_hit_chance);

	for (int j = 0; j < max_loop; j++)
	{
		tmp_hit_chance = TuneGetHitChance(defender, attacker, avoidance_override, 0, 0, loop_add_accuracy);

		if (Msg >= 3)
		{
			Message(0, "[#Tune] - Processing... [%i] ACCURACY %i | Hit Chance %.2f ", j, loop_add_accuracy, tmp_hit_chance);
		}

		if (interval > 0 && tmp_hit_chance >= hit_chance)
		{
			end = true;
		}

		else if (interval < 0 && tmp_hit_chance <= hit_chance)
		{
			end = true;
		}

		if (end) {

			Message(0, "###################RESULTS###################");

			if (avoidance_override) {
				Message(0, "[#Tune] AVOIDANCE STAT OVERRRIDE. This is the amount of ACCURACY adjustment needed if this defender had ( %i ) raw AVOIDANCE stat", avoidance_override);
			}

			if (attacker->IsNPC()) {
				Message(0, "[#Tune] Recommended NPC ACCURACY ADJUSTMENT of ( %i ) on ' %s ' will result in ( %.0f pct ) chance to hit ' %s '.", loop_add_accuracy, attacker->GetCleanName(), hit_chance, defender->GetCleanName());
				Message(0, "[#Tune] SET NPC 'ACCURACY' stat value = [ %i ]", loop_add_accuracy + attacker->CastToNPC()->GetAccuracyRating());
				Message(0, "###################COMPLETE###################");
			}
			else if (attacker->IsClient()) {
				Message(0, "[#Tune] Recommended CLIENT ACCURACY ADJUSTMENT of ( %i ) on  %s ' will result in ( %.0f pct ) chance to hit ' %s '.", loop_add_accuracy, attacker->GetCleanName(), hit_chance, defender->GetCleanName());

				if (loop_add_accuracy >= 0) {
					Message(0, "[#Tune] MODIFY Client Accuracy Mod2 stat or SPA 216 Melee Accuracy (spells/items/aa) [+ %i ]", loop_add_accuracy);

				}
				else {
					Message(0, "[#Tune] Give Client Accuracy Mod2 stat or SPA 216 Melee Accuracy (spells/items/aa) of [ %i ]", loop_add_accuracy);
				}

				Message(0, "###################COMPLETE###################");
			}

			return;
		}

		loop_add_accuracy = loop_add_accuracy + interval;
	}

	Message(0, "###################ABORT#######################");
	Message(0, "[#Tune] Error: Unable to find desired result for ( %.0f pct) - Increase interval (%i) AND/OR max loop value (%i) and run again.", hit_chance, interval, max_loop);
	Message(0, "[#Tune] Parse ended at ACCURACY ADJUSTMENT of ( %i ) on ' %s ' will result in ( %.0f pct ) chance to hit ' %s '.", loop_add_accuracy, attacker->GetCleanName(), hit_chance, defender->GetCleanName());
	Message(0, "###################COMPLETE###################");
}

/*
	Tune support functions
*/

int64 Mob::TuneClientGetMeanDamage(Mob* other, int ac_override, int atk_override, int add_ac, int add_atk)
{
	uint32 total_damage = 0;
	int loop_max = 1000;

	for (int i = 0; i < loop_max; i++)
	{
		if (IsClient()) {
			total_damage += TuneClientAttack(other, true, true, 10000, ac_override, atk_override, add_ac, add_atk);
		}
		else {
			total_damage += TuneNPCAttack(other, true, true, 10000, ac_override, atk_override, add_ac, add_atk);
		}
	}

	return(total_damage / loop_max);
}

int64 Mob::TuneClientGetMaxDamage(Mob* other)
{
	uint32 max_hit = 0;
	uint32 current_hit = 0;
	int loop_max = 1000;

	for (int i = 0; i < loop_max; i++)
	{
		if (IsClient()) {
			current_hit = TuneClientAttack(other, true, true, 10000, 1, 10000);
		}
		else {
			current_hit = TuneNPCAttack(other, true, true, 10000, 1, 10000);
		}

		if (current_hit > max_hit) {
			max_hit = current_hit;
		}
	}
	return(max_hit);
}

int64 Mob::TuneClientGetMinDamage(Mob* other, int max_hit)
{
	uint32 min_hit = max_hit;
	uint32 current_hit = 0;
	int loop_max = 1000;

	for (int i = 0; i < loop_max; i++)
	{
		if (IsClient()) {
			current_hit = TuneClientAttack(other, true, true, 10000, 10000, 1);
		}
		else {
			current_hit = TuneNPCAttack(other, true, true, 10000, 10000, 1);
		}

		if (current_hit < min_hit) {
			min_hit = current_hit;
		}
	}
	return(min_hit);
}

float Mob::TuneGetACMitigationPct(Mob* defender, Mob *attacker) {

	int max_damage = 0;
	int min_damage = 0;

	max_damage = attacker->TuneClientGetMaxDamage(defender);
	min_damage = attacker->TuneClientGetMinDamage(defender, max_damage);

	if (!max_damage)
	{
		Message(0, "[#Tune] Calculation Failure. Error: [Mob::TuneGetACMitigationPct] No max damage found");
		return max_damage;
	}

	int mean_dmg = attacker->TuneClientGetMeanDamage(defender);
	float tmp_pct_mitigated = 100.0f - (static_cast<float>(mean_dmg) * 100.0f / static_cast<float>(max_damage));

	return tmp_pct_mitigated;
}

int64 Mob::TuneGetOffense(Mob* defender, Mob *attacker, int atk_override)
{
	int offense_rating = 0;
	if (attacker->IsClient()) {
		offense_rating = attacker->TuneClientAttack(defender, true, true, 0, 0, atk_override, 0, 0, true);
	}
	else {
		offense_rating = attacker->TuneNPCAttack(defender, true, true, 0, 0, atk_override, 0, 0, true);
	}
	return offense_rating;
}

int64 Mob::TuneGetAccuracy(Mob* defender, Mob *attacker, int accuracy_override, int add_accuracy)
{
	int accuracy = 0;
	if (attacker->IsClient()) {
		accuracy = attacker->TuneClientAttack(defender, true, true, 0, 0, 0, 0, 0, false, true,0,accuracy_override,0,add_accuracy);
	}
	else {
		accuracy = attacker->TuneNPCAttack(defender, true, true, 0, 0, 0, 0, 0, false, true, 0, accuracy_override, 0, add_accuracy);
	}
	return accuracy;
}

int64 Mob::TuneGetAvoidance(Mob* defender, Mob *attacker, int avoidance_override, int add_avoidance)
{
	return defender->TuneGetTotalDefense(avoidance_override, add_avoidance);
}

float Mob::TuneGetHitChance(Mob* defender, Mob *attacker, int avoidance_override, int accuracy_override, int add_avoidance, int add_accuracy)
{
	uint32 hit_count = 0;
	uint32 current_hit = 0;

	int loop_max = 2000;

	for (int i = 0; i < loop_max; i++)
	{
		if (attacker->IsClient()) {
			current_hit = attacker->TuneClientAttack(defender, true, false, 0, 0, 0, 0, 0, false, false, avoidance_override, accuracy_override, add_avoidance, add_accuracy);
		}
		else {
			current_hit = attacker->TuneNPCAttack(defender, true, false, 0, 0, 0, 0, 0, false, false, avoidance_override, accuracy_override, add_avoidance, add_accuracy);
		}

		if (current_hit > 0) {
			hit_count++;
		}
	}
	float chance = (static_cast<float>(hit_count) / 2000.0f) * 100.0f;
	return chance;
}

float Mob::TuneGetAvoidMeleeChance(Mob* defender, Mob *attacker, int type)
{
	uint32 current_hit = 0;
	uint32 hit_count = 0;

	/*
		-1 - block
		-2 - parry
		-3 - riposte
		-4 - dodge
	*/
	int loop_max = 3000;

	for (int i = 0; i < loop_max; i++)
	{
		if (attacker->IsClient()) {
			current_hit = attacker->TuneClientAttack(defender, false, true, 0);
		}
		else {
			current_hit = attacker->TuneNPCAttack(defender, false, true, 0);
		}

		if (current_hit == type) {
			hit_count++;
		}
	}
	float chance = (static_cast<float>(hit_count) / 3000.0f) * 100.0f;
	return chance;
}

int64 Mob::TuneCalcEvasionBonus(int final_avoidance, int base_avoidance) {

	/*
	float eb = static_cast<float>(final_avoidance) / static_cast<float>(base_avoidance);
	Shout(" eb %.2f ", eb);
	eb = eb * 100.f;
	Shout(" eb %.2f ", eb);
	eb = eb - 100.0f;
	Shout(" eb %.2f ", eb);
	return eb;
	*/

	int loop_max = 5000;
	int evasion_bonus = 10;
	int current_avoidance = 0;

	int interval = 1;

	if (base_avoidance > final_avoidance)
	{
		interval = interval * -1;
	}

	for (int i = 0; i < loop_max; i++)
	{
		current_avoidance = (base_avoidance * (100 + evasion_bonus)) / 100;

		if (interval > 0 && current_avoidance >= final_avoidance)
		{
			return evasion_bonus;
		}
		else if (interval < 0 && current_avoidance <= final_avoidance)
		{
			return evasion_bonus;
		}
		evasion_bonus = evasion_bonus + interval;
	}
	return 0;
}

/*
	Calculate from modified attack.cpp functions.
*/

int64 Mob::TuneNPCAttack(Mob* other, bool no_avoid, bool no_hit_chance, int hit_chance_bonus, int ac_override, int atk_override, int add_ac, int add_atk, bool get_offense, bool get_accuracy,
	int avoidance_override, int accuracy_override, int add_avoidance, int add_accuracy)
{
	if (!IsNPC()) {
		Message(Chat::Red, "#Tune Failure:  A null NON NPC object was passed to TuneNPCAttack() for evaluation!");
		return false;
	}

	if (!other) {
		Message(Chat::Red, "#Tune Failure:  A null Mob object was passed to TuneNPCAttack() for evaluation!");
		return false;
	}

	//Check that we can attack before we calc heading and face our target
	if (!IsAttackAllowed(other)) {
		Message(Chat::Red, "#Tune Failure: This NPC can not attack this target!");
		return false;
	}

	DamageHitInfo my_hit;
	my_hit.skill = EQ::skills::SkillHandtoHand;
	my_hit.hand = EQ::invslot::slotPrimary;
	my_hit.damage_done = 1;

	my_hit.skill = static_cast<EQ::skills::SkillType>(CastToNPC()->GetPrimSkill());
	OffHandAtk(false);

	uint8 otherlevel = other->GetLevel();
	uint8 mylevel = GetLevel();

	otherlevel = otherlevel ? otherlevel : 1;
	mylevel = mylevel ? mylevel : 1;

	my_hit.base_damage = CastToNPC()->GetBaseDamage();
	my_hit.min_damage = CastToNPC()->GetMinDamage();
	//int32 hate = my_hit.base_damage + my_hit.min_damage;

	my_hit.offense = Tuneoffense(my_hit.skill, atk_override, add_atk);
	if (get_offense) {
		return my_hit.offense;
	}
	my_hit.tohit = TuneGetTotalToHit(my_hit.skill, hit_chance_bonus, accuracy_override, add_accuracy);
	if (get_accuracy) {
		return my_hit.tohit;
	}

	TuneDoAttack(other, my_hit, nullptr, no_avoid, no_hit_chance, ac_override, add_ac, avoidance_override, accuracy_override, add_avoidance, add_accuracy);

	LogCombat("Final damage against [{}]: [{}]", other->GetName(), my_hit.damage_done);
	if (other->IsClient() && IsPet() && GetOwner()->IsOfClientBot()) {
		//pets do half damage to clients in pvp
		my_hit.damage_done /= 2;
		if (my_hit.damage_done < 1)
			my_hit.damage_done = 1;
	}

	return my_hit.damage_done;
}

int64 Mob::TuneClientAttack(Mob* other, bool no_avoid, bool no_hit_chance, int hit_chance_bonus, int ac_override, int atk_override, int add_ac, int add_atk, bool get_offense, bool get_accuracy,
	int avoidance_override, int accuracy_override, int add_avoidance, int add_accuracy)
{
	if (!IsClient()) {
		Message(Chat::Red, "#Tune Failure:  A null NON CLIENT object was passed to TuneClientAttack() for evaluation!");
		return false;
	}

	if (!other) {
		Message(Chat::Red, "#Tune Failure:  A null Mob object was passed to TuneClientAttack() for evaluation!");
		return false;
	}

	int Hand = EQ::invslot::slotPrimary;

	EQ::ItemInstance* weapon = nullptr;
	weapon = CastToClient()->GetInv().GetItem(EQ::invslot::slotPrimary);
	OffHandAtk(false);

	if (weapon != nullptr) {
		if (!weapon->IsWeapon()) {
			Message(Chat::Red, "#Tune Failure: Attack cancelled, Item %s is not a weapon!", weapon->GetItem()->Name);
			return(false);
		}
	}

	DamageHitInfo my_hit;
	my_hit.skill = TuneAttackAnimation(Hand, weapon);

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
			my_hit.base_damage = CastToClient()->DoDamageCaps(my_hit.base_damage);
		auto shield_inc = spellbonuses.ShieldEquipDmgMod + itembonuses.ShieldEquipDmgMod + aabonuses.ShieldEquipDmgMod;
		if (shield_inc > 0 && HasShieldEquipped() && Hand == EQ::invslot::slotPrimary) {
			my_hit.base_damage = my_hit.base_damage * (100 + shield_inc) / 100;
			hate = hate * (100 + shield_inc) / 100;
		}

		// ***************************************************************
		// *** Calculate the damage bonus, if applicable, for this hit ***
		// ***************************************************************

		int ucDamageBonus = 0;

		if (Hand == EQ::invslot::slotPrimary && GetLevel() >= 28 && IsWarriorClass())
		{
			// Damage bonuses apply only to hits from the main hand (Hand == MainPrimary) by characters level 28 and above
			// who belong to a melee class. If we're here, then all of these conditions apply.

			ucDamageBonus = GetWeaponDamageBonus(weapon ? weapon->GetItem() : (const EQ::ItemData*) nullptr);

			my_hit.min_damage = ucDamageBonus;
			hate += ucDamageBonus;
		}

		my_hit.offense = Tuneoffense(my_hit.skill, atk_override, add_atk); // we need this a few times
		if (get_offense) {
			return my_hit.offense;
		}
		my_hit.hand = Hand;

		my_hit.tohit = TuneGetTotalToHit(my_hit.skill, hit_chance_bonus, accuracy_override, add_accuracy);
		if (get_accuracy) {
			return my_hit.tohit;
		}
		TuneDoAttack(other, my_hit, nullptr, no_avoid, no_hit_chance, ac_override, add_ac, avoidance_override, accuracy_override, add_avoidance, add_accuracy);
	}
	else {
		my_hit.damage_done = DMG_INVULNERABLE;
	}

	///////////////////////////////////////////////////////////
	////// Send Attack Damage
	///////////////////////////////////////////////////////////

	//other->Damage(this, my_hit.damage_done, SPELL_UNKNOWN, my_hit.skill, true, -1, false, m_specialattacks);

	return my_hit.damage_done;
}

void Mob::TuneDoAttack(Mob *other, DamageHitInfo &hit, ExtraAttackOptions *opts, bool no_avoid, bool no_hit_chance, int ac_override, int add_ac,
	int avoidance_override, int accuracy_override, int add_avoidance, int add_accuracy)
{
	if (!other)
		return;
	LogCombat("[{}]::DoAttack vs [{}] base [{}] min [{}] offense [{}] tohit [{}] skill [{}]", GetName(),
		other->GetName(), hit.base_damage, hit.min_damage, hit.offense, hit.tohit, hit.skill);

	// check to see if we hit..
	if (no_avoid || (!no_avoid && other->AvoidDamage(this, hit))) {
		int strike_through = itembonuses.StrikeThrough + spellbonuses.StrikeThrough + aabonuses.StrikeThrough;
		if (strike_through && zone->random.Roll(strike_through)) {
			MessageString(Chat::StrikeThrough,
				STRIKETHROUGH_STRING); // You strike through your opponents defenses!
			hit.damage_done = 1;			// set to one, we will check this to continue
		}
		// I'm pretty sure you can riposte a riposte
		if (hit.damage_done == DMG_RIPOSTED) {
			//DoRiposte(other); //Disabled for TUNE
			//if (IsDead())
			return;
		}
		LogCombat("Avoided/strikethrough damage with code [{}]", hit.damage_done);
	}

	if (hit.damage_done >= 0) {
		if (no_hit_chance || (!no_hit_chance && other->TuneCheckHitChance(this, hit, avoidance_override, add_avoidance))) {
			other->TuneMeleeMitigation(this, hit, ac_override, add_ac);
			if (hit.damage_done > 0) {
				ApplyDamageTable(hit);
				TuneCommonOutgoingHitSuccess(other, hit, opts);
			}
			LogCombat("Final damage after all reductions: [{}]", hit.damage_done);
		}
		else {
			LogCombat("Attack missed. Damage set to 0");
			hit.damage_done = 0;
		}
	}
}

void Mob::TuneMeleeMitigation(Mob *attacker, DamageHitInfo &hit, int ac_override, int add_ac)
{
	if (hit.damage_done < 0 || hit.base_damage == 0)
		return;

	Mob* defender = this;
	//auto mitigation = defender->GetMitigationAC();
	auto mitigation = defender->TuneACSum(false, ac_override, add_ac);
	if (IsClient() && attacker->IsClient())
		mitigation = mitigation * 80 / 100; // 2004 PvP changes

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
}

int64 Mob::TuneACSum(bool skip_caps, int ac_override, int add_ac)
{
	int ac = 0; // this should be base AC whenever shrouds come around
	ac += itembonuses.AC; // items + food + tribute

	if (IsClient()) {
		if (ac_override) {
			ac = ac_override;
		}
		if (add_ac) {
			ac += add_ac;
		}
	}

	int shield_ac = 0;
	if (HasShieldEquipped() && IsOfClientBot()) {
		auto inst = (IsClient()) ? GetInv().GetItem(EQ::invslot::slotSecondary) : CastToBot()->GetBotItem(EQ::invslot::slotSecondary);
		if (inst) {
			if (inst->GetItemRecommendedLevel(true) <= GetLevel()) {
				shield_ac = inst->GetItemArmorClass(true);
			} else {
				shield_ac = CalcRecommendedLevelBonus(GetLevel(), inst->GetItemRecommendedLevel(true),inst->GetItemArmorClass(true));
			}
		}
		shield_ac += itembonuses.heroic_str_shield_ac;
	}
	// EQ math
	ac = (ac * 4) / 3;

	// anti-twink
	if (!skip_caps && IsClient() && GetLevel() < RuleI(Combat, LevelToStopACTwinkControl))
		ac = std::min(ac, 25 + 6 * GetLevel());
	ac = std::max(0, ac + GetClassRaceACBonus());

	if (IsNPC()) {
		// This is the developer tweaked number
		// for the VAST amount of NPCs in EQ this number didn't exceed 600 until recently (PoWar)
		// According to the guild hall Combat Dummies, a level 50 classic EQ mob it should be ~115
		// For a 60 PoP mob ~120, 70 OoW ~120


		if (ac_override) {
			ac += ac_override;
		}
		else {
			ac += GetAC();
		}
		ac += add_ac;

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

	if (!skip_caps && (IsClient())) {
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
	}
	return ac;
}

int64 Mob::Tuneoffense(EQ::skills::SkillType skill, int atk_override, int add_atk)
{
	int offense = GetSkill(skill);
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

	int32 tune_atk = GetATK();

	// GetATK() = ATK + itembonuses.ATK + spellbonuses.ATK.  However, ATK appears to already be itembonuses.ATK + spellbonuses.ATK for PCs, so as is, it is double counting attack
	// This causes attack to be significantly more important than it should be based on era rule of thumbs.  I do not want to change the GetATK() function in case doing so breaks something,
	// so instead I am just adding a /2 to remedy the double counting.  NPCs do not have this issue, so they are broken up.
	// PCAttackPowerScaling is used to help bring attack power further in line with era estimates.
	if (IsOfClientBotMerc()) {
		offense += (GetATK() / 2 + GetPetATKBonusFromOwner()) * RuleI(Combat, PCAttackPowerScaling) / 100;
	} else {
		offense += GetATK();
	}

	if (atk_override) {
		tune_atk = atk_override;
	}

	tune_atk += add_atk;

	return offense;
}

EQ::skills::SkillType Mob::TuneAttackAnimation(int Hand, const EQ::ItemInstance* weapon, EQ::skills::SkillType skillinuse)
{
	// Determine animation
	int type = 0;
	if (weapon && weapon->IsClassCommon()) {
		const EQ::ItemData* item = weapon->GetItem();

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
		case EQ::skills::Skill1HSlashing: // 1H Slashing
			type = anim1HWeapon;
			break;
		case EQ::skills::Skill2HSlashing: // 2H Slashing
			type = anim2HSlashing;
			break;
		case EQ::skills::Skill1HPiercing: // Piercing
			type = anim1HPiercing;
			break;
		case EQ::skills::Skill1HBlunt: // 1H Blunt
			type = anim1HWeapon;
			break;
		case EQ::skills::Skill2HBlunt: // 2H Blunt
			type = anim2HSlashing; //anim2HWeapon
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
	if (Hand == EQ::invslot::slotSecondary)	// DW anim
		type = animDualWield;

	return skillinuse;
}

int64 Mob::Tunecompute_tohit(EQ::skills::SkillType skillinuse, int accuracy_override, int add_accuracy)
{
	int tohit = GetSkill(EQ::skills::SkillOffense) + 7;
	tohit += GetSkill(skillinuse);
	if (IsNPC()) {
		if (accuracy_override) {
			tohit += accuracy_override;
		}
		else {
			tohit += CastToNPC()->GetAccuracyRating();
		}
		tohit += add_accuracy;
	}
	if (IsClient()) {
		double reduction = CastToClient()->GetIntoxication() / 2.0;
		if (reduction > 20.0) {
			reduction = std::min((110 - reduction) / 100.0, 1.0);
			tohit = reduction * static_cast<double>(tohit);
		}
		else if (IsBerserk()) {
			tohit += (GetLevel() * 2) / 5;
		}
	}
	return std::max(tohit, 1);
}

// return -1 in cases that always hit
int64 Mob::TuneGetTotalToHit(EQ::skills::SkillType skill, int chance_mod, int accuracy_override, int add_accuracy)
{
	if (chance_mod >= 10000) // override for stuff like SpellEffect::SkillAttack
		return -1;

	// calculate attacker's accuracy
	auto accuracy = Tunecompute_tohit(skill, accuracy_override, add_accuracy) + 10; // add 10 in case the NPC's stats are fucked
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

	if (IsClient()) {
		if (accuracy_override) {
			accuracy = accuracy_override;
		}
		accuracy += add_accuracy;
	}

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
	return accuracy;
}

// return -1 in cases that always miss
int64 Mob::TuneGetTotalDefense(int avoidance_override, int add_avoidance)
{
	auto avoidance = Tunecompute_defense(avoidance_override, add_avoidance) + 10; // add 10 in case the NPC's stats are fucked
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

int64 Mob::Tunecompute_defense(int avoidance_override, int add_avoidance)
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

// called when a mob is attacked, does the checks to see if it's a hit
// and does other mitigation checks. 'this' is the mob being attacked.
bool Mob::TuneCheckHitChance(Mob* other, DamageHitInfo &hit, int avoidance_override, int add_avoidance)
{

	Mob *attacker = other;
	Mob *defender = this;

	if (defender->IsClient() && defender->CastToClient()->IsSitting())
		return true;

	auto avoidance = defender->TuneGetTotalDefense(avoidance_override, add_avoidance);
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

	// tie breaker? Don't want to be biased any one way
	if (tohit_roll == avoid_roll)
		return zone->random.Roll(50);
	return tohit_roll > avoid_roll;
}

void Mob::TuneCommonOutgoingHitSuccess(Mob* defender, DamageHitInfo &hit, ExtraAttackOptions *opts)
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
		else if (GetClass() == Class::Ranger && GetLevel() > 50) { // no double dmg on headshot
			if ((defender->IsNPC() && !defender->IsMoving() && !defender->IsRooted()) || !RuleB(Combat, ArcheryBonusRequiresStationary)) {
				hit.damage_done *= 2;
				MessageString(Chat::MeleeCrit, BOW_DOUBLE_DAMAGE);
			}
		}
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
			if (ass > 0)
				hit.damage_done = ass;
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

	if (defender->GetShielderID()) {
		DoShieldDamageOnShielder(defender, hit.damage_done, hit.skill);
		hit.damage_done -= hit.damage_done * defender->GetShieldTargetMitigation() / 100; //Default shielded takes 50 pct damage
	}
}
