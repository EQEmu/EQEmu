#include "zone/client.h"

void command_tune(Client *c, const Seperator *sep)
{
	//Work in progress - Kayen

	if (sep->arg[1][0] == '\0' || !strcasecmp(sep->arg[1], "help")) {
		c->Message(Chat::White, "Syntax: #tune [subcommand].");
		c->Message(Chat::White, "-- Tune System Commands --");
		c->Message(
			Chat::White,
			"-- Usage: Returns recommended combat statistical values based on a desired outcome through simulated combat."
		);
		c->Message(
			Chat::White,
			"-- This commmand can answer the following difficult questions whening tunings NPCs and Players."
		);
		c->Message(
			Chat::White,
			"-- Question: What is the average damage mitigation my AC provides against a specific targets attacks?"
		);
		c->Message(
			Chat::White,
			"-- Question: What is amount of AC would I need to add to acheive a specific average damage mitigation agianst specific targets attacks?"
		);
		c->Message(
			Chat::White,
			"-- Question: What is amount of AC would I need to add to my target to acheive a specific average damage mitigation from my attacks?"
		);
		c->Message(Chat::White, "-- Question: What is my targets average AC damage mitigation based on my ATK stat?");
		c->Message(
			Chat::White,
			"-- Question: What is amount of ATK would I need to add to myself to acheive a specific average damage mitigation on my target?"
		);
		c->Message(
			Chat::White,
			"-- Question: What is amount of ATK would I need to add to my target to acheive a specific average AC damage mitigation on myself?"
		);
		c->Message(Chat::White, "-- Question: What is my hit chance against a target?");
		c->Message(
			Chat::White,
			"-- Question: What is the amount of avoidance I need to add to my target to achieve a specific hit chance?"
		);
		c->Message(
			Chat::White,
			"-- Question: What is the amount of accuracy I need to add to my target to achieve a specific chance of hitting me?"
		);
		c->Message(Chat::White, "-- Question: ... and many more...");
		c->Message(Chat::White, " ");
		c->Message(Chat::White, "...#tune stats [A/D]");
		c->Message(
			Chat::White,
			"...#tune FindATK [A/D] [pct mitigation] [interval] [loop_max] [AC override] [Info Level]"
		);
		c->Message(
			Chat::White,
			"...#tune FindAC  [A/D] [pct mitigation] [interval] [loop_max] [ATK override] [Info Level] "
		);
		c->Message(
			Chat::White,
			"...#tune FindAccuracy  [A/D] [hit chance] [interval] [loop_max] [Avoidance override] [Info Level]"
		);
		c->Message(
			Chat::White,
			"...#tune FindAvoidance [A/D] [hit chance] [interval] [loop_max] [Accuracy override] [Info Level] "
		);
		c->Message(Chat::White, " ");
		c->Message(Chat::White, "-- DETAILS AND EXAMPLES ON USAGE");
		c->Message(Chat::White, " ");
		c->Message(
			Chat::White,
			"...Returns combat statistics, including AC mitigation pct, hit chance, and avoid melee chance for attacker and defender."
		);
		c->Message(Chat::White, "...#tune stats [A/D]");
		c->Message(Chat::White, "...");
		c->Message(
			Chat::White,
			"...Returns recommended ATK adjustment (+/-) on ATTACKER that will result in a specific average AC mitigation pct on DEFENDER. "
		);
		c->Message(
			Chat::White,
			"...#tune FindATK [A/D] [pct mitigation] [interval][loop_max][AC override][Info Level]"
		);
		c->Message(
			Chat::White,
			"...Example: Find the amount of ATK stat I need to add to the targeted NPC so that it hits me for 50 pct damage on average."
		);
		c->Message(Chat::White, "...Example: #tune FindATK D 50");
		c->Message(Chat::White, "...");
		c->Message(
			Chat::White,
			"...Returns recommended AC adjustment(+/-) on DEFENDER for a specific average AC mitigation pct from ATTACKER. "
		);
		c->Message(
			Chat::White,
			"...#tune FindAC  [A/D] [pct mitigation] [interval][loop_max][ATK override][Info Level] "
		);
		c->Message(
			Chat::White,
			"...Example: Find the amount of AC stat I need to add to the targeted NPC so that I hit it for 70 pct damage on average."
		);
		c->Message(Chat::White, "...Example: #tune FindAC D 70");
		c->Message(Chat::White, "...");
		c->Message(
			Chat::White,
			"...Returns recommended Accuracy adjustment (+/-) on ATTACKER that will result in a specific hit chance pct on DEFENDER. "
		);
		c->Message(
			Chat::White,
			"...#tune FindAccuracy  [A/D] [hit chance] [interval][loop_max][Avoidance override][Info Level]"
		);
		c->Message(
			Chat::White,
			"...Example: Find the amount of Accuracy stat I need to add to the targeted NPC so that it has a 60 pct hit chance against me."
		);
		c->Message(Chat::White, "...Example: #tune FindAccuracy D 60");
		c->Message(Chat::White, "...");
		c->Message(
			Chat::White,
			"...Returns recommended Avoidance adjustment (+/-) on DEFENDER for in a specific hit chance pct from ATTACKER. "
		);
		c->Message(
			Chat::White,
			"...#tune FindAvoidance [A/D] [hit chance] [interval][loop_max][Accuracy override][Info Level] "
		);
		c->Message(
			Chat::White,
			"...Example: Find the amount of Avoidance stat I need to add to the targeted NPC so that I have a 30 pct hit chance against it."
		);
		c->Message(Chat::White, "...Example: #tune FindAvoidance D 30");
		c->Message(Chat::White, "... ");
		c->Message(Chat::White, "...Usage: [A/D] You must input either A or D.");
		c->Message(Chat::White, "...Category [A] : YOU are the ATTACKER. YOUR TARGET is the DEFENDER.");
		c->Message(Chat::White, "...Category [D] : YOU are the DEFENDER. YOUR TARGET is the ATTACKER.");
		c->Message(Chat::White, "...If TARGET is in combat, DEFENDER is the TARGETs TARGET.");

		c->Message(Chat::White, " ");

		c->Message(
			Chat::White,
			"-- Warning: The calculations done in this process are intense and can potentially cause zone crashes depending on parameters set, use with caution!"
		);
		c->Message(Chat::White, "-- Below are OPTIONAL parameters.");
		c->Message(
			Chat::White,
			"-- Note: [interval] Determines how much the stat being checked increases/decreases till it finds the best result. Lower is more accurate. Default=10"
		);
		c->Message(
			Chat::White,
			"-- Note: [loop_max] Determines how many iterations are done to increases/decreases the stat till it finds the best result. Higher is more accurate. Default=1000"
		);
		c->Message(
			Chat::White,
			"-- Note: [Stat Override] Will override that stat on mob being checked with the specified value. Default=0"
		);
		c->Message(
			Chat::White,
			"-- Example: If as the attacker you want to find the ATK value you would need to have agianst a target with 1000 AC to achieve an average AC mitigation of 50 pct."
		);
		c->Message(Chat::White, "-- Example: #tune FindATK A 50 0 0 1000");
		c->Message(Chat::White, "-- Note: [Info Level] How much parsing detail is displayed[0 - 1]. Default: [0] ");
		c->Message(Chat::White, " ");

		return;
	}
	/*
		Category A: YOU are the attacker and your target is the defender
		Category D: YOU are the defender and your target is the attacker
	*/

	Mob *attacker = c;
	Mob *defender = c->GetTarget();

	if (!defender) {
		c->Message(Chat::White, "[#Tune] - Error no target selected. [#Tune help]");
		return;
	}

	//Use if checkings on engaged targets.
	Mob *ttarget = attacker->GetTarget();
	if (ttarget) {
		defender = ttarget;
	}

	if (!strcasecmp(sep->arg[1], "stats")) {

		if (!strcasecmp(sep->arg[2], "A")) {
			c->TuneGetStats(defender, attacker);
		}
		else if (!strcasecmp(sep->arg[2], "D")) {
			c->TuneGetStats(attacker, defender);
		}
		else {
			c->TuneGetStats(defender, attacker);
		}
		return;
	}

	if (!strcasecmp(sep->arg[1], "FindATK")) {
		float pct_mitigation = Strings::ToFloat(sep->arg[3]);
		int   interval       = Strings::ToInt(sep->arg[4]);
		int   max_loop       = Strings::ToInt(sep->arg[5]);
		int   ac_override    = Strings::ToInt(sep->arg[6]);
		int   info_level     = Strings::ToInt(sep->arg[7]);

		if (!pct_mitigation) {
			c->Message(Chat::White, "[#Tune] - Error must enter the desired percent mitigation on defender.");
			c->Message(
				Chat::White,
				"...Returns recommended ATK adjustment (+/-) on ATTACKER that will result in a specific average AC mitigation pct on DEFENDER. "
			);
			c->Message(
				Chat::White,
				"...#tune FindATK [A/D] [pct mitigation] [interval][loop_max][AC override][Info Level]"
			);
			c->Message(
				Chat::White,
				"...Example: Find the amount of ATK stat I need to add to the targeted NPC so that it hits me for 50 pct damage on average."
			);
			c->Message(Chat::White, "...Example: #tune FindATK D 50");
			return;
		}

		if (!interval) {
			interval = 10;
		}
		if (!max_loop) {
			max_loop = 1000;
		}
		if (!ac_override) {
			ac_override = 0;
		}
		if (!info_level) {
			info_level = 0;
		}

		if (!strcasecmp(sep->arg[2], "A")) {
			c->TuneGetATKByPctMitigation(
				defender,
				attacker,
				pct_mitigation,
				interval,
				max_loop,
				ac_override,
				info_level
			);
		}
		else if (!strcasecmp(sep->arg[2], "D")) {
			c->TuneGetATKByPctMitigation(
				attacker,
				defender,
				pct_mitigation,
				interval,
				max_loop,
				ac_override,
				info_level
			);
		}
		else {
			c->Message(Chat::White, "#Tune - Error no category selcted. [#Tune help]");
			c->Message(
				Chat::White,
				"Usage #tune FindATK [A/B] [pct mitigation] [interval][loop_max][AC Overwride][Info Level] "
			);
			c->Message(Chat::White, "...Usage: [A/D] You must input either A or D.");
			c->Message(Chat::White, "...Category [A] : YOU are the ATTACKER. YOUR TARGET is the DEFENDER.");
			c->Message(Chat::White, "...Category [D] : YOU are the DEFENDER. YOUR TARGET is the ATTACKER.");
			c->Message(Chat::White, "...If TARGET is in combat, DEFENDER is the TARGETs TARGET.");
			c->Message(Chat::White, "... ");
			c->Message(
				Chat::White,
				"...Example: Find the amount of ATK stat I need to add to the targeted NPC so that it hits me for 50 pct damage on average."
			);
			c->Message(Chat::White, "...Example: #tune FindATK D 50");
		}
		return;
	}

	if (!strcasecmp(sep->arg[1], "FindAC")) {
		float pct_mitigation = Strings::ToFloat(sep->arg[3]);
		int   interval       = Strings::ToInt(sep->arg[4]);
		int   max_loop       = Strings::ToInt(sep->arg[5]);
		int   atk_override   = Strings::ToInt(sep->arg[6]);
		int   info_level     = Strings::ToInt(sep->arg[7]);

		if (!pct_mitigation) {
			c->Message(Chat::White, "#Tune - Error must enter the desired percent mitigation on defender.");
			c->Message(
				Chat::White,
				"...Returns recommended AC adjustment(+/-) on DEFENDER for a specific average AC mitigation pct from ATTACKER. "
			);
			c->Message(
				Chat::White,
				"...#tune FindAC  [A/D] [pct mitigation] [interval][loop_max][ATK override][Info Level] "
			);
			c->Message(
				Chat::White,
				"...Example: Find the amount of AC stat I need to add to the targeted NPC so that I hit it for 70 pct damage on average."
			);
			c->Message(Chat::White, "...Example: #tune FindAC D 70");
			return;
		}

		if (!interval) {
			interval = 10;
		}
		if (!max_loop) {
			max_loop = 1000;
		}
		if (!atk_override) {
			atk_override = 0;
		}
		if (!info_level) {
			info_level = 0;
		}

		if (!strcasecmp(sep->arg[2], "A")) {
			c->TuneGetACByPctMitigation(
				defender,
				attacker,
				pct_mitigation,
				interval,
				max_loop,
				atk_override,
				info_level
			);
		}
		else if (!strcasecmp(sep->arg[2], "D")) {
			c->TuneGetACByPctMitigation(
				attacker,
				defender,
				pct_mitigation,
				interval,
				max_loop,
				atk_override,
				info_level
			);
		}
		else {
			c->Message(Chat::White, "#Tune - Error no category selcted. [#Tune help]");
			c->Message(
				Chat::White,
				"Usage #tune FindATK [A/B] [pct mitigation] [interval][loop_max][AC Overwride][Info Level] "
			);
			c->Message(Chat::White, "...Usage: [A/D] You must input either A or D.");
			c->Message(Chat::White, "...Category [A] : YOU are the ATTACKER. YOUR TARGET is the DEFENDER.");
			c->Message(Chat::White, "...Category [D] : YOU are the DEFENDER. YOUR TARGET is the ATTACKER.");
			c->Message(Chat::White, "...If TARGET is in combat, DEFENDER is the TARGETs TARGET.");
			c->Message(Chat::White, "... ");
			c->Message(
				Chat::White,
				"...Example: Find the amount of AC stat I need to add to the targeted NPC so that I hit it for 70 pct damage on average."
			);
			c->Message(Chat::White, "...Example: #tune FindAC D 70");
		}

		return;
	}

	if (!strcasecmp(sep->arg[1], "FindAccuracy")) {
		float hit_chance     = Strings::ToFloat(sep->arg[3]);
		int   interval       = Strings::ToInt(sep->arg[4]);
		int   max_loop       = Strings::ToInt(sep->arg[5]);
		int   avoid_override = Strings::ToInt(sep->arg[6]);
		int   info_level     = Strings::ToInt(sep->arg[7]);

		if (!hit_chance) {
			c->Message(Chat::White, "#Tune - Error must enter the desired hit chance on defender.");
			c->Message(
				Chat::White,
				"...Returns recommended Accuracy adjustment (+/-) on ATTACKER that will result in a specific hit chance pct on DEFENDER. "
			);
			c->Message(
				Chat::White,
				"...#tune FindAccuracy  [A/D] [hit chance] [interval][loop_max][Avoidance override][Info Level]"
			);
			c->Message(
				Chat::White,
				"...Example: Find the amount of Accuracy stat I need to add to the targeted NPC so that it has a 60 pct hit chance against me."
			);
			c->Message(Chat::White, "...Example: #tune FindAccuracy D 60");
			return;
		}

		if (!interval) {
			interval = 10;
		}
		if (!max_loop) {
			max_loop = 1000;
		}
		if (!avoid_override) {
			avoid_override = 0;
		}
		if (!info_level) {
			info_level = 0;
		}

		if (!strcasecmp(sep->arg[2], "A")) {
			c->TuneGetAccuracyByHitChance(
				defender,
				attacker,
				hit_chance,
				interval,
				max_loop,
				avoid_override,
				info_level
			);
		}
		else if (!strcasecmp(sep->arg[2], "D")) {
			c->TuneGetAccuracyByHitChance(
				attacker,
				defender,
				hit_chance,
				interval,
				max_loop,
				avoid_override,
				info_level
			);
		}
		else {
			c->Message(Chat::White, "#Tune - Error no category selcted. [#Tune help]");
			c->Message(
				Chat::White,
				"...#tune FindAccuracy  [A/D] [hit chance] [interval][loop_max][Avoidance override][Info Level]"
			);
			c->Message(Chat::White, "...Usage: [A/D] You must input either A or D.");
			c->Message(Chat::White, "...Category [A] : YOU are the ATTACKER. YOUR TARGET is the DEFENDER.");
			c->Message(Chat::White, "...Category [D] : YOU are the DEFENDER. YOUR TARGET is the ATTACKER.");
			c->Message(Chat::White, "...If TARGET is in combat, DEFENDER is the TARGETs TARGET.");
			c->Message(Chat::White, "... ");
			c->Message(
				Chat::White,
				"...Example: Find the amount of Accuracy stat I need to add to the targeted NPC so that it has a 60 pct hit chance against me."
			);
			c->Message(Chat::White, "...Example: #tune FindAccuracy D 60");
		}

		return;
	}

	if (!strcasecmp(sep->arg[1], "FindAvoidance")) {
		float hit_chance   = Strings::ToFloat(sep->arg[3]);
		int   interval     = Strings::ToInt(sep->arg[4]);
		int   max_loop     = Strings::ToInt(sep->arg[5]);
		int   acc_override = Strings::ToInt(sep->arg[6]);
		int   info_level   = Strings::ToInt(sep->arg[7]);

		if (!hit_chance) {
			c->Message(Chat::White, "#Tune - Error must enter the desired hit chance on defender.");
			c->Message(
				Chat::White,
				"...Returns recommended Avoidance adjustment (+/-) on DEFENDER for in a specific hit chance pct from ATTACKER. "
			);
			c->Message(
				Chat::White,
				"...#tune FindAvoidance [A/D] [hit chance] [interval][loop_max][Accuracy override][Info Level] "
			);
			c->Message(
				Chat::White,
				"...Example: Find the amount of Avoidance stat I need to add to the targeted NPC so that I have a 30 pct hit chance against it."
			);
			c->Message(Chat::White, "...Example: #tune FindAvoidance D 30");
			return;
		}
		if (!interval) {
			interval = 10;
		}
		if (!max_loop) {
			max_loop = 1000;
		}
		if (!acc_override) {
			acc_override = 0;
		}
		if (!info_level) {
			info_level = 0;
		}

		if (!strcasecmp(sep->arg[2], "A")) {
			c->TuneGetAvoidanceByHitChance(
				defender,
				attacker,
				hit_chance,
				interval,
				max_loop,
				acc_override,
				info_level
			);
		}
		else if (!strcasecmp(sep->arg[2], "D")) {
			c->TuneGetAvoidanceByHitChance(
				attacker,
				defender,
				hit_chance,
				interval,
				max_loop,
				acc_override,
				info_level
			);
		}
		else {
			c->Message(Chat::White, "#Tune - Error no category selcted. [#Tune help]");
			c->Message(
				Chat::White,
				"...#tune FindAvoidance [A/D] [hit chance] [interval][loop_max][Accuracy override][Info Level] "
			);
			c->Message(Chat::White, "...Usage: [A/D] You must input either A or D.");
			c->Message(Chat::White, "...Category [A] : YOU are the ATTACKER. YOUR TARGET is the DEFENDER.");
			c->Message(Chat::White, "...Category [D] : YOU are the DEFENDER. YOUR TARGET is the ATTACKER.");
			c->Message(Chat::White, "...If TARGET is in combat, DEFENDER is the TARGETs TARGET.");
			c->Message(Chat::White, "... ");
			c->Message(
				Chat::White,
				"...Example: Find the amount of Avoidance stat I need to add to the targeted NPC so that I have a 30 pct hit chance against it."
			);
			c->Message(Chat::White, "...Example: #tune FindAvoidance D 30");
		}

		return;
	}

	c->Message(Chat::White, "#Tune - Error no command [#Tune help]");
	return;
}

