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

/*

	To add a new bot command 3 things must be done:

	1.	At the bottom of bot_command.h you must add a prototype for it.
	2.	Add the function in this file.
	3.	In the bot_command_init function you must add a call to bot_command_add
		for your function.

	Notes: If you want an alias for your bot command, add an entry to the
	`bot_command_settings` table in your database. The access level you
	set with bot_command_add is the default setting if the bot command isn't
	listed in the `bot_command_settings` db table.

*/

#include "bot_command.h"

#include "common/data_verification.h"
#include "common/eq_packet.h"
#include "common/features.h"
#include "common/ptimer.h"
#include "common/rulesys.h"
#include "common/say_link.h"
#include "common/serverinfo.h"
#include "common/strings.h"
#include "zone/bot_database.h"
#include "zone/mob.h"
#include "zone/qglobals.h"
#include "zone/queryserv.h"
#include "zone/quest_parser_collection.h"
#include "zone/titles.h"
#include "zone/water_map.h"
#include "zone/worldserver.h"
#include "zone/zonedb.h"

#include "fmt/format.h"
#include <cstring>

extern QueryServ* QServ;
extern WorldServer worldserver;

int bot_command_count;

int (*bot_command_dispatch)(Client *,char const *) = bot_command_not_avail;

std::map<std::string, BotCommandRecord *> bot_command_list;
std::map<std::string, std::string> bot_command_aliases;

LinkedList<BotCommandRecord *> cleanup_bot_command_list;

int bot_command_not_avail(Client *c, const char *message)
{
	c->Message(Chat::Yellow, "Bot commands not available.");
	return -1;
}

int bot_command_init(void)
{
	bot_command_aliases.clear();

	if (
		bot_command_add("actionable", "Lists actionable command arguments and use descriptions", AccountStatus::Player, bot_command_actionable) ||
		bot_command_add("applypoison", "Applies cursor-held poison to a rogue bot's weapon", AccountStatus::Player, bot_command_apply_poison) ||
		bot_command_add("attack", "Orders bots to attack a designated target", AccountStatus::Player, bot_command_attack) ||
		bot_command_add("behindmob", "Toggles whether or not your bot tries to stay behind a mob", AccountStatus::Player, bot_command_behind_mob) ||
		bot_command_add("blockedbuffs", "Set, view and clear blocked buffs for the selected bot(s)", AccountStatus::Player, bot_command_blocked_buffs) ||
		bot_command_add("blockedpetbuffs", "Set, view and clear blocked pet buffs for the selected bot(s)", AccountStatus::Player, bot_command_blocked_pet_buffs) ||
		bot_command_add("bot", "Lists the available bot management [subcommands]", AccountStatus::Player, bot_command_bot) ||
		bot_command_add("botappearance", "Lists the available bot appearance [subcommands]", AccountStatus::Player, bot_command_appearance) ||
		bot_command_add("botbeardcolor", "Changes the beard color of a bot", AccountStatus::Player, bot_command_beard_color) ||
		bot_command_add("botbeardstyle", "Changes the beard style of a bot", AccountStatus::Player, bot_command_beard_style) ||
		bot_command_add("botcamp", "Orders a bot(s) to camp", AccountStatus::Player, bot_command_camp) ||
		bot_command_add("botclone", "Creates a copy of a bot", AccountStatus::GMMgmt, bot_command_clone) ||
		bot_command_add("botcreate", "Creates a new bot", AccountStatus::Player, bot_command_create) ||
		bot_command_add("botdelete", "Deletes all record of a bot", AccountStatus::Player, bot_command_delete) ||
		bot_command_add("botdetails", "Changes the Drakkin details of a bot", AccountStatus::Player, bot_command_details) ||
		bot_command_add("botdyearmor", "Changes the color of a bot's (bots') armor", AccountStatus::Player, bot_command_dye_armor) ||
		bot_command_add("boteyes", "Changes the eye colors of a bot", AccountStatus::Player, bot_command_eyes) ||
		bot_command_add("botface", "Changes the facial appearance of your bot", AccountStatus::Player, bot_command_face) ||
		bot_command_add("botfollowdistance", "Changes the follow distance(s) of a bot(s)", AccountStatus::Player, bot_command_follow_distance) ||
		bot_command_add("bothaircolor", "Changes the hair color of a bot", AccountStatus::Player, bot_command_hair_color) ||
		bot_command_add("bothairstyle", "Changes the hairstyle of a bot", AccountStatus::Player, bot_command_hairstyle) ||
		bot_command_add("botheritage", "Changes the Drakkin heritage of a bot", AccountStatus::Player, bot_command_heritage) ||
		bot_command_add("botinspectmessage", "Changes the inspect message of a bot", AccountStatus::Player, bot_command_inspect_message) ||
		bot_command_add("botlist", "Lists the bots that you own", AccountStatus::Player, bot_command_list_bots) ||
		bot_command_add("botreport", "Orders a bot to report its readiness", AccountStatus::Player, bot_command_report) ||
		bot_command_add("botsettings", "Lists settings related to spell types and bot combat", AccountStatus::Player, bot_command_bot_settings) ||
		bot_command_add("botspawn", "Spawns a created bot", AccountStatus::Player, bot_command_spawn) ||
		bot_command_add("botstance", "Changes the stance of a bot", AccountStatus::Player, bot_command_stance) ||
		bot_command_add("botstopmeleelevel", "Sets the level a caster or spell-casting fighter bot will stop melee combat", AccountStatus::Player, bot_command_stop_melee_level) ||
		bot_command_add("botsuffix", "Sets a bots suffix", AccountStatus::Player, bot_command_suffix) ||
		bot_command_add("botsummon", "Summons bot(s) to your location", AccountStatus::Player, bot_command_summon) ||
		bot_command_add("botsurname", "Sets a bots surname (last name)", AccountStatus::Player, bot_command_surname) ||
		bot_command_add("bottattoo", "Changes the Drakkin tattoo of a bot", AccountStatus::Player, bot_command_tattoo) ||
		bot_command_add("bottogglehelm", "Toggles the helm visibility of a bot between shown and hidden", AccountStatus::Player, bot_command_toggle_helm) ||
		bot_command_add("bottoggleranged", "Toggles a ranged bot between melee and ranged weapon use", AccountStatus::Player, bot_command_toggle_ranged) ||
		bot_command_add("bottitle", "Sets a bots title", AccountStatus::Player, bot_command_title) ||
		bot_command_add("botupdate", "Updates a bot to reflect any level changes that you have experienced", AccountStatus::Player, bot_command_update) ||
		bot_command_add("botwoad", "Changes the Barbarian woad of a bot", AccountStatus::Player, bot_command_woad) ||
		bot_command_add("cast", "Tells the first found specified bot to cast the given spell type", AccountStatus::Player, bot_command_cast) ||
		bot_command_add("discipline", "Uses aggressive/defensive disciplines or can specify spell ID", AccountStatus::Player, bot_command_discipline) ||
		bot_command_add("distanceranged", "Controls the range casters and ranged will try to stay away from a mob", AccountStatus::Player, bot_command_distance_ranged) ||
		bot_command_add("classracelist", "Lists the classes and races and their appropriate IDs", AccountStatus::Player, bot_command_class_race_list) ||
		bot_command_add("clickitem", "Orders your targeted bot to click the item in the provided inventory slot.", AccountStatus::Player, bot_command_click_item) ||
		bot_command_add("copysettings", "Copies settings from one bot to another", AccountStatus::Player, bot_command_copy_settings) ||
		bot_command_add("defaultsettings", "Restores a bot back to default settings", AccountStatus::Player, bot_command_default_settings) ||
		bot_command_add("depart", "Orders a bot to open a magical doorway to a specified destination", AccountStatus::Player, bot_command_depart) ||
		bot_command_add("enforcespellsettings", "Toggles your Bot to cast only spells in their spell settings list.", AccountStatus::Player, bot_command_enforce_spell_list) ||
		bot_command_add("findaliases", "Find available aliases for a bot command", AccountStatus::Player, bot_command_find_aliases) ||
		bot_command_add("follow", "Orders bots to follow a designated target (option 'chain' auto-links eligible spawned bots)", AccountStatus::Player, bot_command_follow) ||
		bot_command_add("guard", "Orders bots to guard their current positions", AccountStatus::Player, bot_command_guard) ||
		bot_command_add("healrotation", "Lists the available bot heal rotation [subcommands]", AccountStatus::Player, bot_command_heal_rotation) ||
		bot_command_add("healrotationadaptivetargeting", "Enables or disables adaptive targeting within the heal rotation instance", AccountStatus::Player, bot_command_heal_rotation_adaptive_targeting) ||
		bot_command_add("healrotationaddmember", "Adds a bot to a heal rotation instance", AccountStatus::Player, bot_command_heal_rotation_add_member) ||
		bot_command_add("healrotationaddtarget", "Adds target to a heal rotation instance", AccountStatus::Player, bot_command_heal_rotation_add_target) ||
		bot_command_add("healrotationadjustcritical", "Adjusts the critial HP limit of the heal rotation instance's Class Armor Type criteria", AccountStatus::Player, bot_command_heal_rotation_adjust_critical) ||
		bot_command_add("healrotationadjustsafe", "Adjusts the safe HP limit of the heal rotation instance's Class Armor Type criteria", AccountStatus::Player, bot_command_heal_rotation_adjust_safe) ||
		bot_command_add("healrotationcastingoverride", "Enables or disables casting overrides within the heal rotation instance", AccountStatus::Player, bot_command_heal_rotation_casting_override) ||
		bot_command_add("healrotationchangeinterval", "Changes casting interval between members within the heal rotation instance", AccountStatus::Player, bot_command_heal_rotation_change_interval) ||
		bot_command_add("healrotationclearhot", "Clears the HOT of a heal rotation instance", AccountStatus::Player, bot_command_heal_rotation_clear_hot) ||
		bot_command_add("healrotationcleartargets", "Removes all targets from a heal rotation instance", AccountStatus::Player, bot_command_heal_rotation_clear_targets) ||
		bot_command_add("healrotationcreate", "Creates a bot heal rotation instance and designates a leader", AccountStatus::Player, bot_command_heal_rotation_create) ||
		bot_command_add("healrotationdelete", "Deletes a bot heal rotation entry by leader", AccountStatus::Player, bot_command_heal_rotation_delete) ||
		bot_command_add("healrotationfastheals", "Enables or disables fast heals within the heal rotation instance", AccountStatus::Player, bot_command_heal_rotation_fast_heals) ||
		bot_command_add("healrotationlist", "Reports heal rotation instance(s) information", AccountStatus::Player, bot_command_heal_rotation_list) ||
		bot_command_add("healrotationremovemember", "Removes a bot from a heal rotation instance", AccountStatus::Player, bot_command_heal_rotation_remove_member) ||
		bot_command_add("healrotationremovetarget", "Removes target from a heal rotations instance", AccountStatus::Player, bot_command_heal_rotation_remove_target) ||
		bot_command_add("healrotationresetlimits", "Resets all Class Armor Type HP limit criteria in a heal rotation to its default value", AccountStatus::Player, bot_command_heal_rotation_reset_limits) ||
		bot_command_add("healrotationsave", "Saves a bot heal rotation entry by leader", AccountStatus::Player, bot_command_heal_rotation_save) ||
		bot_command_add("healrotationsethot", "Sets the HOT in a heal rotation instance", AccountStatus::Player, bot_command_heal_rotation_set_hot) ||
		bot_command_add("healrotationstart", "Starts a heal rotation", AccountStatus::Player, bot_command_heal_rotation_start) ||
		bot_command_add("healrotationstop", "Stops a heal rotation", AccountStatus::Player, bot_command_heal_rotation_stop) ||
		bot_command_add("help", "List available commands and their description - specify partial command as argument to search", AccountStatus::Player, bot_command_help) ||
		bot_command_add("hold", "Prevents a bot from attacking until released", AccountStatus::Player, bot_command_hold) ||
		bot_command_add("illusionblock", "Control whether or not illusion effects will land on the bot if casted by another player or bot", AccountStatus::Player, bot_command_illusion_block) ||
		bot_command_add("inventory", "Lists the available bot inventory [subcommands]", AccountStatus::Player, bot_command_inventory) ||
		bot_command_add("inventorygive", "Gives the item on your cursor to a bot", AccountStatus::Player, bot_command_inventory_give) ||
		bot_command_add("inventorylist", "Lists all items in a bot's inventory", AccountStatus::Player, bot_command_inventory_list) ||
		bot_command_add("inventoryremove", "Removes an item from a bot's inventory", AccountStatus::Player, bot_command_inventory_remove) ||
		bot_command_add("inventorywindow", "Displays all items in a bot's inventory in a pop-up window", AccountStatus::Player, bot_command_inventory_window) ||
		bot_command_add("itemuse", "Elicits a report from spawned bots that can use the item on your cursor (option 'empty' yields only empty slots)", AccountStatus::Player, bot_command_item_use) ||
		bot_command_add("maxmeleerange", "Toggles whether your bot is at max melee range or not. This will disable all special abilities, including taunt.", AccountStatus::Player, bot_command_max_melee_range) ||
		bot_command_add("owneroption", "Sets options available to bot owners", AccountStatus::Player, bot_command_owner_option) ||
		bot_command_add("pet", "Lists the available bot pet [subcommands]", AccountStatus::Player, bot_command_pet) ||
		bot_command_add("petgetlost", "Orders a bot to remove its summoned pet", AccountStatus::Player, bot_command_pet_get_lost) ||
		bot_command_add("petremove", "Orders a bot to remove its charmed pet", AccountStatus::Player, bot_command_pet_remove) ||
		bot_command_add("petsettype", "Orders a Magician bot to use a specified pet type", AccountStatus::Player, bot_command_pet_set_type) ||
		bot_command_add("picklock", "Orders a capable bot to pick the lock of the closest door", AccountStatus::Player, bot_command_pick_lock) ||
		bot_command_add("pickpocket", "Orders a capable bot to pickpocket a NPC", AccountStatus::Player, bot_command_pickpocket) ||
		bot_command_add("precombat", "Sets flag used to determine pre-combat behavior", AccountStatus::Player, bot_command_precombat) ||
		bot_command_add("pull", "Orders a designated bot to 'pull' an enemy", AccountStatus::Player, bot_command_pull) ||
		bot_command_add("release", "Releases a suspended bot's AI processing (with hate list wipe)", AccountStatus::Player, bot_command_release) ||
		bot_command_add("setassistee", "Sets your target to be the person your bots assist. Bots will always assist you before others", AccountStatus::Player, bot_command_set_assistee) ||
		bot_command_add("sithppercent", "HP threshold for a bot to start sitting in combat if allowed", AccountStatus::Player, bot_command_sit_hp_percent) ||
		bot_command_add("sitincombat", "Toggles whether or a not a bot will attempt to med or sit to heal in combat", AccountStatus::Player, bot_command_sit_in_combat) ||
		bot_command_add("sitmanapercent", "Mana threshold for a bot to start sitting in combat if allowed", AccountStatus::Player, bot_command_sit_mana_percent) ||
		bot_command_add("spellaggrochecks", "Toggles whether or not bots will cast a spell type if they think it will get them aggro", AccountStatus::Player, bot_command_spell_aggro_checks) ||
		bot_command_add("spellannouncecasts", "Turn on or off cast announcements by spell type", AccountStatus::Player, bot_command_spell_announce_cast) ||
		bot_command_add("spellengagedpriority", "Controls the order of casts by spell type when engaged in combat", AccountStatus::Player, bot_command_spell_engaged_priority) ||
		bot_command_add("spelldelays", "Controls the delay between casts for a specific spell type", AccountStatus::Player, bot_command_spell_delays) ||
		bot_command_add("spellholds", "Controls whether a bot holds the specified spell type or not", AccountStatus::Player, bot_command_spell_holds) ||
		bot_command_add("spellidlepriority", "Controls the order of casts by spell type when out of combat", AccountStatus::Player, bot_command_spell_idle_priority) ||
		bot_command_add("spellmaxhppct", "Controls at what HP percent a bot will stop casting different spell types", AccountStatus::Player, bot_command_spell_max_hp_pct) ||
		bot_command_add("spellmaxmanapct", "Controls at what mana percent a bot will stop casting different spell types", AccountStatus::Player, bot_command_spell_max_mana_pct) ||
		bot_command_add("spellmaxthresholds", "Controls the minimum target HP threshold for a spell to be cast for a specific type", AccountStatus::Player, bot_command_spell_max_thresholds) ||
		bot_command_add("spellminhppct", "Controls at what HP percent a bot will start casting different spell types", AccountStatus::Player, bot_command_spell_min_hp_pct) ||
		bot_command_add("spellminmanapct", "Controls at what mana percent a bot will start casting different spell types", AccountStatus::Player, bot_command_spell_min_mana_pct) ||
		bot_command_add("spellminthresholds", "Controls the maximum target HP threshold for a spell to be cast for a specific type", AccountStatus::Player, bot_command_spell_min_thresholds) ||
		bot_command_add("spellresistlimits", "Controls the resist limits for bots to cast spells on their target", AccountStatus::Player, bot_command_spell_resist_limits) ||
		bot_command_add("spellpursuepriority", "Controls the order of casts by spell type when pursuing in combat", AccountStatus::Player, bot_command_spell_pursue_priority) ||
		bot_command_add("spelltargetcount", "Sets the required target amount for group/AE spells by spell type", AccountStatus::Player, bot_command_spell_target_count) ||
		bot_command_add("spellinfo", "Opens a dialogue window with spell info", AccountStatus::Player, bot_spell_info_dialogue_window) ||
		bot_command_add("spells", "Lists all Spells learned by the Bot.", AccountStatus::Player, bot_command_spell_list) ||
		bot_command_add("spellsettings", "Lists a bot's spell setting entries", AccountStatus::Player, bot_command_spell_settings_list) ||
		bot_command_add("spellsettingsadd", "Add a bot spell setting entry", AccountStatus::Player, bot_command_spell_settings_add) ||
		bot_command_add("spellsettingsdelete", "Delete a bot spell setting entry", AccountStatus::Player, bot_command_spell_settings_delete) ||
		bot_command_add("spellsettingstoggle", "Toggle a bot spell use", AccountStatus::Player, bot_command_spell_settings_toggle) ||
		bot_command_add("spellsettingsupdate", "Update a bot spell setting entry", AccountStatus::Player, bot_command_spell_settings_update) ||
		bot_command_add("spelltypeids", "Lists spelltypes by ID", AccountStatus::Player, bot_command_spelltype_ids) ||
		bot_command_add("spelltypenames", "Lists spelltypes by shortname", AccountStatus::Player, bot_command_spelltype_names) ||
		bot_command_add("suspend", "Suspends a bot's AI processing until released", AccountStatus::Player, bot_command_suspend) ||
		bot_command_add("taunt", "Toggles taunt use by a bot", AccountStatus::Player, bot_command_taunt) ||
		bot_command_add("timer", "Checks or clears timers of the chosen type.", AccountStatus::GMMgmt, bot_command_timer) ||
		bot_command_add("track", "Orders a capable bot to track enemies", AccountStatus::Player, bot_command_track) ||
		bot_command_add("viewcombos", "Views bot race class combinations", AccountStatus::Player, bot_command_view_combos)
	) {
		bot_command_deinit();
		return -1;
	}

	std::map<std::string, std::pair<uint8, std::vector<std::string>>> bot_command_settings;
	database.botdb.LoadBotCommandSettings(bot_command_settings);

	std::vector<std::pair<std::string, uint8>> injected_bot_command_settings;
	std::vector<std::string> orphaned_bot_command_settings;

	if (RuleB(Bots, RunSpellTypeChecksOnBoot)) {
		LogBotSpellTypeChecks("Running SpellType checks. There may be some spells that are mislabeled as incorrect. Use this as a loose guideline.");
		database.botdb.CheckBotSpells();
	}

	database.botdb.MapCommandedSpellTypeMinLevels();

	for (auto bcs_iter : bot_command_settings) {

		auto bcl_iter = bot_command_list.find(bcs_iter.first);
		if (bcl_iter == bot_command_list.end()) {

			orphaned_bot_command_settings.push_back(bcs_iter.first);
			LogInfo(
				"Bot Command [{}] no longer exists... Deleting orphaned entry from `bot_command_settings` table",
				bcs_iter.first.c_str()
			);
		}
	}

	if (orphaned_bot_command_settings.size()) {
		if (!database.botdb.UpdateOrphanedBotCommandSettings(orphaned_bot_command_settings)) {
			LogInfo("Failed to process 'Orphaned Bot Commands' update operation.");
		}
	}

	auto working_bcl = bot_command_list;
	for (auto working_bcl_iter : working_bcl) {

		auto bcs_iter = bot_command_settings.find(working_bcl_iter.first);
		if (bcs_iter == bot_command_settings.end()) {

			injected_bot_command_settings.emplace_back(std::pair<std::string, uint8>(working_bcl_iter.first, working_bcl_iter.second->access));
			LogInfo(
				"New Bot Command [{}] found... Adding to `bot_command_settings` table with access [{}]",
				working_bcl_iter.first.c_str(),
				working_bcl_iter.second->access
			);

			if (working_bcl_iter.second->access == 0) {
				LogCommands(
					"bot_command_init(): Warning: Bot Command [{}] defaulting to access level 0!",
					working_bcl_iter.first.c_str()
				);
			}

			continue;
		}

		working_bcl_iter.second->access = bcs_iter->second.first;
		LogCommands(
			"bot_command_init(): - Bot Command [{}] set to access level [{}]",
			working_bcl_iter.first.c_str(),
			bcs_iter->second.first
		);

		if (bcs_iter->second.second.empty()) {
			continue;
		}

		for (auto alias_iter : bcs_iter->second.second) {
			if (alias_iter.empty()) {
				continue;
			}

			if (bot_command_list.find(alias_iter) != bot_command_list.end()) {
				LogCommands(
					"bot_command_init(): Warning: Alias [{}] already exists as a bot command - skipping!",
					alias_iter.c_str()
				);

				continue;
			}

			bot_command_list[alias_iter] = working_bcl_iter.second;
			bot_command_aliases[alias_iter] = working_bcl_iter.first;

			LogCommands(
				"bot_command_init(): - Alias [{}] added to bot command [{}]",
				alias_iter.c_str(),
				bot_command_aliases[alias_iter].c_str()
			);
		}
	}

	if (injected_bot_command_settings.size()) {
		if (!database.botdb.UpdateInjectedBotCommandSettings(injected_bot_command_settings)) {
			LogInfo("Failed to process 'Injected Bot Commands' update operation.");
		}
	}

	bot_command_dispatch = bot_command_real_dispatch;

	return bot_command_count;
}

void bot_command_deinit(void)
{
	bot_command_list.clear();
	bot_command_aliases.clear();

	bot_command_dispatch = bot_command_not_avail;
	bot_command_count = 0;
}

int bot_command_add(std::string bot_command_name, const char *desc, int access, BotCmdFuncPtr function)
{
	if (bot_command_name.empty()) {
		LogError("bot_command_add() - Bot command added with empty name string - check bot_command.cpp");
		return -1;
	}
	if (function == nullptr) {
		LogError("bot_command_add() - Bot command [{}] added without a valid function pointer - check bot_command.cpp", bot_command_name.c_str());
		return -1;
	}
	if (bot_command_list.count(bot_command_name) != 0) {
		LogError("bot_command_add() - Bot command [{}] is a duplicate bot command name - check bot_command.cpp", bot_command_name.c_str());
		return -1;
	}
	for (auto iter : bot_command_list) {
		if (iter.second->function != function)
			continue;
		LogError("bot_command_add() - Bot command [{}] equates to an alias of [{}] - check bot_command.cpp", bot_command_name.c_str(), iter.first.c_str());
		return -1;
	}

	BotCommandRecord *bcr = new BotCommandRecord;
	bcr->access = access;
	bcr->desc = desc;
	bcr->function = function;

	bot_command_list[bot_command_name] = bcr;
	bot_command_aliases[bot_command_name] = bot_command_name;
	cleanup_bot_command_list.Append(bcr);
	bot_command_count++;

	return 0;
}

int bot_command_real_dispatch(Client *c, const char *message)
{
	Seperator sep(message, ' ', 10, 100, true); // "three word argument" should be considered 1 arg

	std::string cstr(sep.arg[0]+1);

	if(bot_command_list.count(cstr) != 1) {
		return(-2);
	}

	BotCommandRecord *cur = bot_command_list[cstr];
	if(c->Admin() < cur->access){
		c->Message(Chat::Yellow, "Your access level is not high enough to use this bot command.");
		return(-1);
	}

	if(cur->access >= COMMANDS_LOGGING_MIN_STATUS) {
		LogCommands("[{}] ([{}]) used bot command: [{}] (target=[{}])",  c->GetName(), c->AccountName(), message, c->GetTarget()?c->GetTarget()->GetName():"NONE");
	}

	if(cur->function == nullptr) {
		LogError("Bot command [{}] has a null function\n", cstr.c_str());
		return(-1);
	} else {
		//dispatch C++ bot command
		cur->function(c, &sep);	// dispatch bot command
	}
	return 0;

}

bool helper_bot_appearance_fail(Client *bot_owner, Bot *my_bot, uint8 fail_type, const char* type_desc)
{
	switch (fail_type) {
		case AFT_Value:
			bot_owner->Message(Chat::Yellow, "Failed to change '%s' for %s due to invalid value for this command", type_desc, my_bot->GetCleanName());
			return true;
		case AFT_GenderRace:
			bot_owner->Message(Chat::Yellow, "Failed to change '%s' for %s due to invalid bot gender and/or race for this command", type_desc, my_bot->GetCleanName());
			return true;
		case AFT_Race:
			bot_owner->Message(Chat::Yellow, "Failed to change '%s' for %s due to invalid bot race for this command", type_desc, my_bot->GetCleanName());
			return true;
	default:
		return false;
	}
}

void helper_bot_appearance_form_final(Client *bot_owner, Bot *my_bot)
{
	if (!MyBots::IsMyBot(bot_owner, my_bot))
		return;
	if (!my_bot->Save()) {
		bot_owner->Message(Chat::Yellow, "Failed to save appearance change for %s due to unknown cause...", my_bot->GetCleanName());
		return;
	}

	helper_bot_appearance_form_update(my_bot);
	bot_owner->Message(Chat::White, "Successfully changed appearance for %s!", my_bot->GetCleanName());
}

void helper_bot_appearance_form_update(Bot *my_bot)
{
	if (!my_bot) {
		return;
	}

	my_bot->SendIllusionPacket(
		AppearanceStruct{
			.beard = my_bot->GetBeard(),
			.beard_color = my_bot->GetBeardColor(),
			.drakkin_details = my_bot->GetDrakkinDetails(),
			.drakkin_heritage = my_bot->GetDrakkinHeritage(),
			.drakkin_tattoo = my_bot->GetDrakkinTattoo(),
			.eye_color_one = my_bot->GetEyeColor1(),
			.eye_color_two = my_bot->GetEyeColor2(),
			.face = my_bot->GetLuclinFace(),
			.gender_id = my_bot->GetGender(),
			.hair = my_bot->GetHairStyle(),
			.hair_color = my_bot->GetHairColor(),
			.helmet_texture = my_bot->GetHelmTexture(),
			.race_id = my_bot->GetRace(),
			.size = my_bot->GetSize(),
			.texture = my_bot->GetTexture(),
		}
	);
}

uint32 helper_bot_create(Client *bot_owner, std::string bot_name, uint8 bot_class, uint16 bot_race, uint8 bot_gender)
{
	uint32 bot_id = 0;
	if (!bot_owner) {
		return bot_id;
	}

	if (!Bot::IsValidName(bot_name)) {
		bot_owner->Message(
			Chat::Yellow,
			fmt::format(
				"'{}' is an invalid name. You may only use characters 'A-Z' or 'a-z' and it must be between 4 and 15 characters with no spaces. Mixed case {} allowed.",
				bot_name, RuleB(Bots, AllowCamelCaseNames) ? "is" : "is not"
			).c_str()
		);

		return bot_id;
	}

	bool available_flag = false;

	!database.botdb.QueryNameAvailability(bot_name, available_flag);

	if (!available_flag) {
		bot_owner->Message(
			Chat::Yellow,
			fmt::format(
				"The name '{}' is already being used or prohibited. Please choose a different name",
				bot_name
			).c_str()
		);

		return bot_id;
	}

	if (!Bot::IsValidRaceClassCombo(bot_race, bot_class)) {
		const std::string bot_race_name = GetRaceIDName(bot_race);
		const std::string bot_class_name = GetClassIDName(bot_class);

		bot_owner->Message(
			Chat::Yellow,
			fmt::format(
				"{} {} is an invalid race-class combination, would you like to {} proper combinations for {}?",
				bot_race_name,
				bot_class_name,
				Saylink::Silent(
					fmt::format("^viewcombos {}", bot_race),
					"view"
				),
				bot_race_name
			).c_str()
		);

		return bot_id;
	}

	if (!EQ::ValueWithin(bot_gender, Gender::Male, Gender::Female)) {
		bot_owner->Message(
			Chat::White,
			fmt::format(
				"Gender: {} ({}) or {} ({})",
				GetGenderName(Gender::Male),
				Gender::Male,
				GetGenderName(Gender::Female),
				Gender::Female
			).c_str()
		);
		return bot_id;
	}

	if (!Bot::CheckHighEnoughLevelForBots(bot_owner)) {
		return bot_id;
	}

	if (!Bot::CheckHighEnoughLevelForBots(bot_owner, bot_class)) {
		return bot_id;
	}

	uint32 bot_count = 0;
	uint32 bot_class_count = 0;

	if (!database.botdb.QueryBotCount(bot_owner->CharacterID(), bot_class, bot_count, bot_class_count)) {
		bot_owner->Message(Chat::Yellow, "Failed to query bot count.");

		return bot_id;
	}

	if (!Bot::CheckCreateLimit(bot_owner, bot_count)) {
		return bot_id;
	}

	if (!Bot::CheckCreateLimit(bot_owner, bot_class_count, bot_class)) {
		return bot_id;
	}

	auto my_bot = new Bot(Bot::CreateDefaultNPCTypeStructForBot(bot_name, "", bot_owner->GetLevel(), bot_race, bot_class, bot_gender), bot_owner);

	if (!my_bot->Save()) {
		bot_owner->Message(
			Chat::Yellow,
			fmt::format(
				"Failed to create '{}' due to unknown cause.",
				my_bot->GetCleanName()
			).c_str()
		);
		safe_delete(my_bot);
		return bot_id;
	}

	bot_owner->Message(
		Chat::White,
		fmt::format(
			"Bot Created | Name: {} ID: {} Race: {} Class: {}",
			my_bot->GetCleanName(),
			my_bot->GetBotID(),
			GetRaceIDName(my_bot->GetRace()),
			GetClassIDName(my_bot->GetClass())
		).c_str()
	);

	bot_id = my_bot->GetBotID();
	if (parse->PlayerHasQuestSub(EVENT_BOT_CREATE)) {
		const auto& export_string = fmt::format(
			"{} {} {} {} {}",
			bot_name,
			bot_id,
			bot_race,
			bot_class,
			bot_gender
		);

		parse->EventPlayer(EVENT_BOT_CREATE, bot_owner, export_string, 0);
	}

	my_bot->AddBotStartingItems(bot_race, bot_class);

	safe_delete(my_bot);

	return bot_id;
}

int helper_bot_follow_option_chain(Client* bot_owner)
{
	if (!bot_owner) {
		return 0;
	}

	std::vector<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(bot_owner, sbl);
	if (sbl.empty()) {
		return 0;
	}

	int chain_follow_count = 0;
	Mob* followee = bot_owner;

	// only add groups that do not belong to bot_owner
	std::map<uint32, Group*> bot_group_map;
	for (auto bot_iter : sbl) {

		if (!bot_iter || bot_iter->GetManualFollow() || bot_iter->GetGroup() == bot_owner->GetGroup()) {
			continue;
		}

		Group* bot_group = bot_iter->GetGroup();
		if (!bot_iter->GetGroup()) {
			continue;
		}

		bot_group_map[bot_group->GetID()] = bot_group;
	}

	std::list<Bot*> bot_member_list;
	if (bot_owner->GetGroup()) {

		bot_owner->GetGroup()->GetBotList(bot_member_list);
		for (auto bot_member_iter : bot_member_list) {

			if (!bot_member_iter || bot_member_iter->GetBotOwnerCharacterID() != bot_owner->CharacterID() || bot_member_iter == followee || bot_member_iter->GetManualFollow()) {
				continue;
			}

			bot_member_iter->SetFollowID(followee->GetID());
			followee = bot_member_iter;
			++chain_follow_count;
		}
	}

	for (auto bot_group_iter : bot_group_map) {

		if (!bot_group_iter.second) {
			continue;
		}

		bot_group_iter.second->GetBotList(bot_member_list);
		for (auto bot_member_iter : bot_member_list) {

			if (!bot_member_iter || bot_member_iter->GetBotOwnerCharacterID() != bot_owner->CharacterID() || bot_member_iter == followee || bot_member_iter->GetManualFollow()) {
				continue;
			}

			bot_member_iter->SetFollowID(followee->GetID());
			followee = bot_member_iter;
			++chain_follow_count;
		}
	}

	return chain_follow_count;
}

bool helper_command_disabled(Client* bot_owner, bool rule_value, const char* command)
{
	if (!rule_value) {
		bot_owner->Message(Chat::Yellow, "Bot command %s is not enabled on this server.", command);
		return true;
	}

	return false;
}

bool helper_command_alias_fail(Client *bot_owner, const char* command_handler, const char *alias, const char *command)
{
	auto alias_iter = bot_command_aliases.find(&alias[1]);
	if (alias_iter == bot_command_aliases.end() || alias_iter->second.compare(command)) {
		bot_owner->Message(Chat::Yellow, "Undefined linker usage in %s (%s)", command_handler, &alias[1]);
		return true;
	}

	return false;
}

bool helper_is_help_or_usage(const char* arg)
{
	if (!arg)
		return false;
	if (strcasecmp(arg, "help") && strcasecmp(arg, "usage"))
		return false;

	return true;
}

bool helper_no_available_bots(Client *bot_owner, Bot *my_bot)
{
	if (!bot_owner)
		return true;
	if (!my_bot) {
		bot_owner->Message(Chat::Yellow, "No bots are capable of performing this action");
		return true;
	}

	return false;
}

void helper_send_available_subcommands(Client* bot_owner, const char* command_simile, std::vector<const char*> subcommand_list)
{
	bot_owner->Message(Chat::White, "Available %s management subcommands:", command_simile);

	int bot_subcommands_shown = 0;
	for (const auto subcommand_iter : subcommand_list) {
		auto find_iter = bot_command_list.find(subcommand_iter);
		if (find_iter == bot_command_list.end())
			continue;
		if (bot_owner->Admin() < find_iter->second->access)
			continue;

		bot_owner->Message(
			Chat::White,
			fmt::format(
				"{} - {}",
				Saylink::Silent(
					fmt::format("^{} help", subcommand_iter),
					fmt::format("^{}", subcommand_iter)
				),
				find_iter != bot_command_list.end() ? find_iter->second->desc : "No Description"
			).c_str()
		);

		++bot_subcommands_shown;
	}

	bot_owner->Message(Chat::White, "%d bot subcommand%s listed.", bot_subcommands_shown, bot_subcommands_shown != 1 ? "s" : "");
}

void helper_send_usage_required_bots(Client *bot_owner, uint16 spell_type)
{
	if (!bot_owner) {
		return;
	}

	auto sbl = entity_list.GetBotListByCharacterID(bot_owner->CharacterID());
	Bot* bot = nullptr;

	for (const auto& b : sbl) {
		if (b) {
			bot = b;

			break;
		}
	}

	auto spell_map = database.botdb.GetCommandedSpellTypesMinLevels();

	if (spell_map.empty()) {
		bot_owner->Message(Chat::Yellow, "No bots are capable of casting this spell type.");
		return;
	}

	bool found = false;
	std::string description;

	for (int i = Class::Warrior; i <= Class::Berserker; ++i) {
		auto spell_type_itr = spell_map.find(spell_type);
		auto class_itr = spell_type_itr->second.find(i);
		const auto& spell_info = class_itr->second;

		if (spell_info.min_level < UINT8_MAX) {
			found = true;

			if (!description.empty()) {
				description.append(", ");
			}
			else {
				bot_owner->Message(Chat::Yellow, "Required bots to cast: Class [Class ID]: [Level]");
			}

			description.append(spell_info.description);
		}
	}

	if (!found || description.empty()) {
		bot_owner->Message(Chat::Yellow, "No bots are capable of casting this spell type");

		return;
	}

	bot_owner->Message(Chat::Green, "%s", description.c_str());
}

void SendSpellTypeWindow(Client* c, const Seperator* sep) {
	std::string arg0 = sep->arg[0];
	std::string arg1 = sep->arg[1];

	uint8 min_count = 0;
	uint8 max_count = 0;
	bool client_only = false;

	if (BotSpellTypes::END <= 19) {
		min_count = BotSpellTypes::START;
		max_count = BotSpellTypes::END;
	}
	else if (!arg1.compare("0-19")) {
		min_count = BotSpellTypes::START;
		max_count = 19;
	}
	else if (!arg1.compare("20-39")) {
		min_count = std::min(static_cast<uint8_t>(20), static_cast<uint8_t>(BotSpellTypes::END));
		max_count = std::min(static_cast<uint8_t>(39), static_cast<uint8_t>(BotSpellTypes::END));
	}
	else if (!arg1.compare("40+")) {
		min_count = std::min(static_cast<uint8_t>(40), static_cast<uint8_t>(BotSpellTypes::END));
		max_count = BotSpellTypes::END;
	}
	else if (!arg1.compare("commanded")) {
		min_count = BotSpellTypes::COMMANDED_START;
		max_count = BotSpellTypes::COMMANDED_END;
	}
	else if (!arg1.compare("client")) {
		min_count = BotSpellTypes::START;
		max_count = BotSpellTypes::END;
		client_only = true;
	}
	else {
		c->Message(Chat::Yellow, "You must choose a valid range option");

		return;
	}

	const std::string& indian_red = "indian_red";
	const std::string& gold = "gold";
	const std::string& slate_blue = "slate_blue";
	const std::string& forest_green = "forest_green";
	const std::string& goldenrod = "goldenrod";

	std::string filler_line = "-----------";
	std::string spell_type_field = "Spell Type";
	std::string id_field = "ID";
	std::string shortname_field = "Short Name";

	std::string popup_text = DialogueWindow::TableRow(
		DialogueWindow::TableCell(DialogueWindow::ColorMessage(goldenrod, spell_type_field))
		+
		DialogueWindow::TableCell((!arg0.compare("^spelltypeids") ? DialogueWindow::ColorMessage(goldenrod, id_field) : DialogueWindow::ColorMessage(goldenrod, shortname_field)))
	);

	popup_text += DialogueWindow::TableRow(
		DialogueWindow::TableCell(DialogueWindow::ColorMessage(gold, filler_line))
		+
		DialogueWindow::TableCell(DialogueWindow::ColorMessage(gold, filler_line))
	);

	for (int i = min_count; i <= max_count; ++i) {
		if (client_only && !IsClientBotSpellType(i)) {
			continue;
		}

		popup_text += DialogueWindow::TableRow(
			DialogueWindow::TableCell(DialogueWindow::ColorMessage(forest_green, Bot::GetSpellTypeNameByID(i)))
			+
			DialogueWindow::TableCell((!arg0.compare("^spelltypeids") ? DialogueWindow::ColorMessage(slate_blue, std::to_string(i)) : DialogueWindow::ColorMessage(slate_blue, Bot::GetSpellTypeShortNameByID(i))))
		);
	}

	popup_text = DialogueWindow::Table(popup_text);

	c->SendPopupToClient("Spell Types", popup_text.c_str());
}

