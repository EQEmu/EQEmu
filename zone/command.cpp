#include "command.h"

#include "common/data_bucket.h"
#include "common/eq_packet.h"
#include "common/events/player_event_logs.h"
#include "common/features.h"
#include "common/file.h"
#include "common/net/eqstream.h"
#include "common/ptimer.h"
#include "common/repositories/command_subsettings_repository.h"
#include "common/repositories/dynamic_zones_repository.h"
#include "common/rulesys.h"
#include "common/say_link.h"
#include "common/strings.h"
#include "zone/dynamic_zone.h"
#include "zone/fastmath.h"
#include "zone/mob_movement_manager.h"
#include "zone/npc_scale_manager.h"
#include "zone/queryserv.h"
#include "zone/quest_parser_collection.h"
#include "zone/titles.h"
#include "zone/water_map.h"
#include "zone/worldserver.h"

#include "fmt/format.h"
#include <algorithm>
#include <cstring>
#include <thread>

extern QueryServ* QServ;
extern WorldServer worldserver;
extern FastMath g_Math;
void CatchSignal(int sig_num);


int command_count; // how many commands we have

// this is the pointer to the dispatch function, updated once
// init has been performed to point at the real function
int (*command_dispatch)(Client *, std::string, bool) = command_notavail;

std::map<std::string, CommandRecord *>                        commandlist;
std::map<std::string, std::string>                            commandaliases;
std::vector<CommandRecord *>                                  command_delete_list;
std::map<std::string, uint8>                                  commands_map;
std::vector<CommandSubsettingsRepository::CommandSubsettings> command_subsettings;

/*
 * command_notavail
 * This is the default dispatch function when commands aren't loaded.
 *
 * Parameters:
 *	not used
 *
 */
int command_notavail(Client *c, std::string message, bool ignore_status)
{
	c->Message(Chat::White, "Commands not available.");
	return -1;
}

/*
 * command_init
 * initializes the command list, call at startup
 *
 * Parameters:
 *	none
 *
 * When adding a new command, only hard-code 'real' commands -
 * all command aliases are added later through a database call
 *
 */

int command_init(void)
{
	if (!commandaliases.empty()) {
		command_deinit();
	}

	if (
		command_add("acceptrules", "[acceptrules] - Accept the EQEmu Agreement", AccountStatus::Player, command_acceptrules) ||
		command_add("advnpcspawn", "[maketype|makegroup|addgroupentry|addgroupspawn][removegroupspawn|movespawn|editgroupbox|cleargroupbox]", AccountStatus::GMLeadAdmin, command_advnpcspawn) ||
		command_add("aggrozone", "[aggro] - Aggro every mob in the zone with X aggro. Default is 0. Not recommend if you're not invulnerable.", AccountStatus::GMAdmin, command_aggrozone) ||
		command_add("ai", "[factionid/spellslist/con/guard/roambox/stop/start] - Modify AI on NPC target", AccountStatus::GMAdmin, command_ai) ||
		command_add("appearance", "[type] [value] - Send an appearance packet for you or your target", AccountStatus::GMLeadAdmin, command_appearance) ||
		command_add("appearanceeffects", "[Help|Remove|Set|View] - Modify appearance effects on yourself or your target.", AccountStatus::GMAdmin, command_appearanceeffects) ||
		command_add("apply_shared_memory", "[shared_memory_name] - Tells every zone and world to apply a specific shared memory segment by name.", AccountStatus::GMImpossible, command_apply_shared_memory) ||
		command_add("attack", "[Entity Name] - Make your NPC target attack an entity by name", AccountStatus::GMLeadAdmin, command_attack) ||
		command_add("augmentitem", "Force augments an item. Must have the augment item window open.", AccountStatus::GMImpossible, command_augmentitem) ||
		command_add("ban", "[Character Name] [Reason] - Ban by character name", AccountStatus::GMLeadAdmin, command_ban) ||
		command_add("bugs", "[Close|Delete|Review|Search|View] - Handles player bug reports", AccountStatus::QuestTroupe, command_bugs) ||
		(RuleB(Bots, Enabled) && command_add("bot", "Type \"#bot help\" or \"^help\" to the see the list of available commands for bots.", AccountStatus::Player, command_bot)) ||
		command_add("camerashake", "[Duration (Milliseconds)] [Intensity (1-10)] - Shakes the camera on everyone's screen globally.", AccountStatus::QuestTroupe, command_camerashake) ||
		command_add("castspell", "[Spell ID] [Instant (0 = False, 1 = True, Default is 1 if Unused)] - Cast a spell", AccountStatus::Guide, command_castspell) ||
		command_add("chat", "[Channel ID] [Message] - Send a channel message to all zones", AccountStatus::GMMgmt, command_chat) ||
		command_add("clearxtargets", "Clears XTargets", AccountStatus::Player, command_clearxtargets) ||
		command_add("copycharacter", "[source_char_name] [dest_char_name] [dest_account_name] - Copies character to destination account", AccountStatus::GMImpossible, command_copycharacter) ||
		command_add("corpse", "Manipulate corpses, use with no arguments for help", AccountStatus::Guide, command_corpse) ||
		command_add("corpsefix", "Attempts to bring corpses from underneath the ground within close proximity of the player", AccountStatus::Player, command_corpsefix) ||
		command_add("countitem", "[Item ID] - Counts the specified Item ID in your or your target's inventory", AccountStatus::GMLeadAdmin, command_countitem) ||
		command_add("damage", "[Amount] - Damage yourself or your target", AccountStatus::GMAdmin, command_damage) ||
		command_add("databuckets", "View|Delete [key] [limit]- View data buckets, limit 50 default or Delete databucket by key", AccountStatus::QuestTroupe, command_databuckets) ||
		command_add("dbspawn2", "[Spawngroup ID] [Respawn] [Variance] [Condition ID] [Condition Minimum] - Spawn an NPC from a predefined row in the spawn2 table, Respawn and Variance are in Seconds (condition is optional)", AccountStatus::GMAdmin, command_dbspawn2) ||
		command_add("delacct", "[Account ID|Account Name] - Delete an account by ID or Name", AccountStatus::GMLeadAdmin, command_delacct) ||
		command_add("delpetition", "[petition number] - Delete a petition", AccountStatus::ApprenticeGuide, command_delpetition) ||
		command_add("depop", "[Start Spawn Timer] - Depop your NPC target and optionally start their spawn timer (false by default)", AccountStatus::Guide, command_depop) ||
		command_add("depopzone", "[Start Spawn Timers] - Depop the zone and optionally start spawn timers (false by default)", AccountStatus::GMAdmin, command_depopzone) ||
		command_add("devtools", "[menu|window] [enable|disable] - Manages Developer Tools (send no parameter for menu)", AccountStatus::GMMgmt, command_devtools) ||
		command_add("disablerecipe", "[Recipe ID] - Disables a Recipe", AccountStatus::QuestTroupe, command_disablerecipe) ||
		command_add("disarmtrap", "Analog for ldon disarm trap for the newer clients since we still don't have it working.", AccountStatus::QuestTroupe, command_disarmtrap) ||
		command_add("door", "Door editing command", AccountStatus::QuestTroupe, command_door) ||
		command_add("doanim", "[Animation ID|Animation Name] [Speed] - Send an animation by ID or name at the specified speed to you or your target (Speed is optional)", AccountStatus::Guide, command_doanim) ||
		command_add("dye", "[slot|'help'] [red] [green] [blue] [use_tint] - Dyes the specified armor slot to Red, Green, and Blue provided, allows you to bypass darkness limits.", AccountStatus::ApprenticeGuide, command_dye) ||
		command_add("dz", "Manage expeditions and dynamic zone instances", AccountStatus::QuestTroupe, command_dz) ||
		command_add("dzkickplayers", "Removes all players from current expedition. (/kickplayers alternative for pre-RoF clients)", AccountStatus::Player, command_dzkickplayers) ||
		command_add("editmassrespawn", "[name-search] [second-value] - Mass (Zone wide) NPC respawn timer editing command", AccountStatus::GMAdmin, command_editmassrespawn) ||
		command_add("emote", "[Name|World|Zone] [type] [message] - Send an emote message by name, to the world, or to your zone (^ separator allows multiple messages to be sent at once)", AccountStatus::QuestTroupe, command_emote) ||
		command_add("emptyinventory", "Clears your or your target's entire inventory (Equipment, General, Bank, and Shared Bank)", AccountStatus::GMImpossible, command_emptyinventory) ||
		command_add("enablerecipe", "[Recipe ID] - Enables a Recipe", AccountStatus::QuestTroupe, command_enablerecipe) ||
		command_add("entityvariable", "[clear|delete|set|view] - Modify entity variables for yourself or your target", AccountStatus::GMAdmin, command_entityvariable) ||
		command_add("exptoggle", "[Toggle] - Toggle your or your target's experience gain.", AccountStatus::QuestTroupe, command_exptoggle) ||
		command_add("evolve", "Evolving Item manipulation commands. Use argument help for more info.", AccountStatus::Steward, command_evolvingitems) ||
		command_add("faction", "[Find (criteria | all ) | Review (criteria | all) | Reset (id)] - Resets Player's Faction", AccountStatus::QuestTroupe, command_faction) ||
		command_add("factionassociation", "[factionid] [amount] - triggers a faction hits via association", AccountStatus::GMLeadAdmin, command_faction_association) ||
		command_add("feature", "Change your or your target's feature's temporarily", AccountStatus::QuestTroupe, command_feature) ||
		command_add("size", "Change your targets size (alias of #feature size)", AccountStatus::QuestTroupe, command_feature) ||
		command_add("find", "Search command used to find various things", AccountStatus::Guide, command_find) ||
		command_add("fish", "Fish for an item", AccountStatus::QuestTroupe, command_fish) ||
		command_add("fixmob", "[race|gender|texture|helm|face|hair|haircolor|beard|beardcolor|heritage|tattoo|detail] [next|prev] - Manipulate appearance of your target", AccountStatus::QuestTroupe, command_fixmob) ||
		command_add("flagedit", "Edit zone flags on your target. Use #flagedit help for more info.", AccountStatus::GMAdmin, command_flagedit) ||
		command_add("fleeinfo", "- Gives info about whether a NPC will flee or not, using the command issuer as top hate.", AccountStatus::QuestTroupe, command_fleeinfo) ||
		command_add("forage", "Forage an item", AccountStatus::QuestTroupe, command_forage) ||
		command_add("gearup", "Developer tool to quickly equip yourself or your target", AccountStatus::GMMgmt, command_gearup) ||
		command_add("giveitem", "[itemid] [charges] - Summon an item onto your target's cursor. Charges are optional.", AccountStatus::GMMgmt, command_giveitem) ||
		command_add("givemoney", "[Platinum] [Gold] [Silver] [Copper] - Gives specified amount of money to you or your player target", AccountStatus::GMMgmt, command_givemoney) ||
		command_add("gmzone", "[Zone ID|Zone Short Name] [Version] [Instance Identifier] - Zones to a private GM instance (Version defaults to 0 and Instance Identifier defaults to 'gmzone' if not used)", AccountStatus::GMAdmin, command_gmzone) ||
		command_add("goto", "[playername] or [x y z] [h] - Teleport to the provided coordinates or to your target", AccountStatus::Steward, command_goto) ||
		command_add("grantaa", "[level] - Grants a player all available AA points up the specified level, all AAs are granted if no level is specified.", AccountStatus::GMMgmt, command_grantaa) ||
		command_add("grid", "[add/delete] [grid_num] [wandertype] [pausetype] - Create/delete a wandering grid", AccountStatus::GMAreas, command_grid) ||
		command_add("guild", "Guild manipulation commands. Use argument help for more info.", AccountStatus::Steward, command_guild) ||
		command_add("help", "[Search Criteria] - List available commands and their description, specify partial command as argument to search", AccountStatus::Player, command_help) ||
		command_add("hotfix", "[hotfix_name] - Reloads shared memory into a hotfix, equiv to load_shared_memory followed by apply_shared_memory", AccountStatus::GMImpossible, command_hotfix) ||
		command_add("hp", "Refresh your HP bar from the server.", AccountStatus::Player, command_hp) ||
		command_add("illusionblock", "Controls whether or not illusion effects will land on you when cast by other players or bots", AccountStatus::Guide, command_illusion_block) ||
		command_add("instance", "Modify Instances", AccountStatus::GMMgmt, command_instance) ||
		command_add("interrogateinv", "use [help] argument for available options", AccountStatus::Player, command_interrogateinv) ||
		command_add("interrupt", "[Message ID] [Color] - Interrupt your casting. Arguments are optional.", AccountStatus::Guide, command_interrupt) ||
		command_add("invsnapshot", "Manipulates inventory snapshots for your current target", AccountStatus::QuestTroupe, command_invsnapshot) ||
		command_add("ipban", "[IP] - Ban IP", AccountStatus::GMMgmt, command_ipban) ||
		command_add("kick", "[Character Name] - Disconnect a player by name", AccountStatus::GMLeadAdmin, command_kick) ||
		command_add("kill", "Kill your target", AccountStatus::GMAdmin, command_kill) ||
		command_add("killallnpcs", "[npc_name] - Kills all npcs by search name, leave blank for all attackable NPC's", AccountStatus::GMMgmt, command_killallnpcs) ||
		command_add("list", "[npcs|players|corpses|doors|objects] [search] - Search entities", AccountStatus::ApprenticeGuide, command_list) ||
		command_add("lootsim", "[npc_type_id] [loottable_id] [iterations] - Runs benchmark simulations using real loot logic to report numbers and data", AccountStatus::GMImpossible, command_lootsim) ||
		command_add("load_shared_memory", "[shared_memory_name] - Reloads shared memory and uses the input as output", AccountStatus::GMImpossible, command_load_shared_memory) ||
		command_add("loc", "Print out your or your target's current location and heading", AccountStatus::Player, command_loc) ||
		command_add("logs", "Manage anything to do with logs", AccountStatus::GMImpossible, command_logs) ||
		command_add("makepet", "[Pet Name] - Make a pet", AccountStatus::Guide, command_makepet) ||
		command_add("memspell", "[Spell ID] [Spell Gem] - Memorize a Spell by ID to the specified Spell Gem for you or your target", AccountStatus::Guide, command_memspell) ||
		command_add("merchantshop", "Closes or opens your target merchant's shop", AccountStatus::GMAdmin, command_merchantshop) ||
		command_add("modifynpcstat", "[Stat] [Value] - Modifies an NPC's stats temporarily.", AccountStatus::GMLeadAdmin, command_modifynpcstat) ||
		command_add("movechar", "[Character ID|Character Name] [Zone ID|Zone Short Name] - Move an offline character to the specified zone", AccountStatus::Guide, command_movechar) ||
		command_add("movement", "Various movement commands", AccountStatus::GMMgmt, command_movement) ||
		command_add("myskills", "Show details about your current skill levels", AccountStatus::Player, command_myskills) ||
		command_add("mysql", "[Help|Query] [SQL Query] - Mysql CLI, see 'Help' for options.", AccountStatus::GMImpossible, command_mysql) ||
		command_add("mystats", "Show details about you or your pet", AccountStatus::Guide, command_mystats) ||
		command_add("npccast", "[targetname/entityid] [spellid] - Causes NPC target to cast spellid on targetname/entityid", AccountStatus::QuestTroupe, command_npccast) ||
		command_add("npcedit", "[column] [value] - Mega NPC editing command", AccountStatus::GMAdmin, command_npcedit) ||
		command_add("npceditmass", "[name-search] [column] [value] - Mass (Zone wide) NPC data editing command", AccountStatus::GMAdmin, command_npceditmass) ||
		command_add("npcemote", "[Message] - Make your NPC target emote a message.", AccountStatus::GMLeadAdmin, command_npcemote) ||
		command_add("npcloot", "Manipulate the loot an NPC is carrying. Use #npcloot help for more information.", AccountStatus::QuestTroupe, command_npcloot) ||
		command_add("npcsay", "[Message] - Make your NPC target say a message.", AccountStatus::GMLeadAdmin, command_npcsay) ||
		command_add("npcshout", "[Message] - Make your NPC target shout a message.", AccountStatus::GMLeadAdmin, command_npcshout) ||
		command_add("npcspawn", "[create/add/update/remove/delete] - Manipulate spawn DB", AccountStatus::GMAreas, command_npcspawn) ||
		command_add("npctypespawn", "[NPC ID] [Faction ID] - Spawn an NPC by ID from the database with an option of setting its Faction ID", AccountStatus::Steward, command_npctypespawn) ||
		command_add("nudge", "Nudge your target's current position by specific values", AccountStatus::QuestTroupe, command_nudge) ||
		command_add("nukebuffs", "[Beneficial|Detrimental|Help] - Strip all buffs by type on you or your target (no argument to remove all buffs)", AccountStatus::Guide, command_nukebuffs) ||
		command_add("nukeitem", "[Item ID] - Removes the specified Item ID from you or your player target's inventory", AccountStatus::GMLeadAdmin, command_nukeitem) ||
		command_add("object", "List|Add|Edit|Move|Rotate|Copy|Save|Undo|Delete - Manipulate static and tradeskill objects within the zone", AccountStatus::GMAdmin, command_object) ||
		command_add("opcode", "Reloads all opcodes from server patch files", AccountStatus::GMMgmt, command_reload) ||
		command_add("parcels", "View and edit the parcel system.  Requires parcels to be enabled in rules.", AccountStatus::GMMgmt, command_parcels) ||
		command_add("path", "view and edit pathing", AccountStatus::GMMgmt, command_path) ||
		command_add("peqzone", "[Zone ID|Zone Short Name] - Teleports you to the specified zone if you meet the requirements.", AccountStatus::Player, command_peqzone) ||
		command_add("petitems", "View your pet's items if you have one", AccountStatus::ApprenticeGuide, command_petitems) ||
		command_add("picklock", "Analog for ldon pick lock for the newer clients since we still don't have it working.", AccountStatus::Player, command_picklock) ||
		command_add("profanity", "Manage censored language.", AccountStatus::GMLeadAdmin, command_profanity) ||
		command_add("push", "[Back Push] [Up Push] - Lets you do spell push on an NPC", AccountStatus::GMLeadAdmin, command_push) ||
		command_add("raidloot", "[All|GroupLeader|RaidLeader|Selected] - Sets your Raid Loot Type if you have permission to do so.", AccountStatus::Player, command_raidloot) ||
		command_add("randomfeatures", "Temporarily randomizes the Facial Features of your target", AccountStatus::QuestTroupe, command_randomfeatures) ||
		command_add("refreshgroup", "Refreshes Group for you or your player target.", AccountStatus::Player, command_refreshgroup) ||
		command_add("reload", "Reloads different types of server data globally, use no argument for help menu.", AccountStatus::GMMgmt, command_reload) ||
		command_add("rq", "Reloads quests (alias of #reload quests).", AccountStatus::GMMgmt, command_reload) ||
		command_add("rl", "Reloads logs (alias of #reload logs).", AccountStatus::GMMgmt, command_reload) ||
		command_add("removeitem", "[Item ID] [Amount] - Removes the specified Item ID by Amount from you or your player target's inventory (Amount defaults to 1 if not used)", AccountStatus::GMAdmin, command_removeitem) ||
		command_add("repop", "[Force] - Repop the zone with optional force repop", AccountStatus::GMAdmin, command_repop) ||
		command_add("resetaa", "[aa|leadership] - Resets a player's AAs or Leadership AAs and refunds spent AAs (not Leadership AAs) to unspent, may disconnect player.", AccountStatus::GMMgmt, command_resetaa) ||
		command_add("resetaa_timer", "[All|Timer ID] - Command to reset AA cooldown timers for you or your player target.", AccountStatus::GMMgmt, command_resetaa_timer) ||
		command_add("resetdisc_timer", "[All|Timer ID] - Command to reset discipline timers.", AccountStatus::GMMgmt, command_resetdisc_timer) ||
		command_add("revoke", "[Character Name] [0|1] - Revokes or unrevokes a player's ability to talk in OOC by name (0 = Unrevoke, 1 = Revoke)", AccountStatus::GMMgmt, command_revoke) ||
		command_add("roambox", "[Remove|Set] [Box Size] [Delay (Milliseconds)] - Remove or set an NPC's roambox size and delay", AccountStatus::GMMgmt, command_roambox) ||
		command_add("rules", "(subcommand) - Manage server rules", AccountStatus::GMImpossible, command_rules) ||
		command_add("save", "Force your player or player corpse target to be saved to the database", AccountStatus::Guide, command_save) ||
		command_add("scale", "Handles NPC scaling", AccountStatus::GMLeadAdmin, command_scale) ||
		command_add("scribespell", "[Spell ID] - Scribe a spell by ID to your or your target's spell book.", AccountStatus::GMCoder, command_scribespell) ||
		command_add("scribespells", "[Max level] [Min level] - Scribe all spells for you or your player target that are usable by them, up to level specified. (may freeze client for a few seconds)", AccountStatus::GMLeadAdmin, command_scribespells) ||
		command_add("sendzonespawns", "Refresh spawn list for all clients in zone", AccountStatus::GMLeadAdmin, command_sendzonespawns) ||
		command_add("sensetrap", "Analog for ldon sense trap for the newer clients since we still don't have it working.", AccountStatus::Player, command_sensetrap) ||
		command_add("serverrules", "Show server rules", AccountStatus::Player, command_serverrules) ||
		command_add("set", "Set command used to set various things", AccountStatus::Guide, command_set) ||
		command_add("show", "Show command used to show various things", AccountStatus::Guide, command_show) ||
		command_add("shutdown", "Shut this zone process down", AccountStatus::GMLeadAdmin, command_shutdown) ||
		command_add("spawn", "[name] [race] [level] [material] [hp] [gender] [class] [priweapon] [secweapon] [merchantid] - Spawn an NPC", AccountStatus::Steward, command_spawn) ||
		command_add("spawneditmass", "[Search Criteria] [Edit Option] [Edit Value] [Apply] Mass editing spawn command (Apply is optional, 0 = False, 1 = True, default is False)", AccountStatus::GMLeadAdmin, command_spawneditmass) ||
		command_add("spawnfix", "Find targeted NPC in database based on its X/Y/heading and update the database to make it spawn at your current location/heading.", AccountStatus::GMAreas, command_spawnfix) ||
		command_add("stun", "[duration] - Stuns you or your target for duration", AccountStatus::GMAdmin, command_stun) ||
		command_add("summon", "[Character Name] - Summons your corpse, NPC, or player target, or by character name if specified", AccountStatus::QuestTroupe, command_summon) ||
		command_add("summonburiedplayercorpse", "Summons the target's oldest buried corpse, if any exist.", AccountStatus::GMAdmin, command_summonburiedplayercorpse) ||
		command_add("summonitem", "[itemid] [charges] - Summon an item onto your cursor. Charges are optional.", AccountStatus::GMMgmt, command_summonitem) ||
		command_add("suspend", "[name] [days] [reason] - Suspend by character name and for specificed number of days", AccountStatus::GMLeadAdmin, command_suspend) ||
		command_add("suspendmulti", "[Character Name One|Character Name Two|etc] [Days] [Reason] - Suspend multiple characters by name for specified number of days", AccountStatus::GMLeadAdmin, command_suspendmulti) ||
		command_add("takeplatinum", "[Platinum] - Takes specified amount of platinum from you or your player target", AccountStatus::GMMgmt, command_takeplatinum) ||
		command_add("task", "(subcommand) - Task system commands", AccountStatus::GMLeadAdmin, command_task) ||
		command_add("petname", "[newname] - Temporarily renames your pet. Leave name blank to restore the original name.", AccountStatus::GMAdmin, command_petname) ||
		command_add("traindisc", "[level] - Trains all the disciplines usable by the target, up to level specified. (may freeze client for a few seconds)", AccountStatus::GMLeadAdmin, command_traindisc) ||
		command_add("tune", "Calculate statistical values related to combat.", AccountStatus::GMAdmin, command_tune) ||
		command_add("undye", "Remove dye from all of your or your target's armor slots", AccountStatus::GMAdmin, command_undye) ||
		command_add("unmemspell", "[Spell ID] - Unmemorize a Spell by ID for you or your target", AccountStatus::Guide, command_unmemspell) ||
		command_add("unmemspells", " Unmemorize all spells for you or your target", AccountStatus::Guide, command_unmemspells) ||
		command_add("unscribespell", "[Spell ID] - Unscribe a spell from your or your target's spell book by Spell ID", AccountStatus::GMCoder, command_unscribespell) ||
		command_add("unscribespells", "Clear out your or your player target's spell book.", AccountStatus::GMCoder, command_unscribespells) ||
		command_add("untraindisc", "[Spell ID] - Untrain your or your target's discipline by Spell ID", AccountStatus::GMCoder, command_untraindisc) ||
		command_add("untraindiscs", "Untrains all disciplines from your target.", AccountStatus::GMCoder, command_untraindiscs) ||
		command_add("wc", "[Slot ID] [Material] [Hero Forge Model] [Elite Material] - Sets the specified slot for you or your target to a material, Hero Forge Model and Elite Material are optional", AccountStatus::GMMgmt, command_wc) ||
		command_add("worldshutdown", "Shut down world and all zones", AccountStatus::GMMgmt, command_worldshutdown) ||
		command_add("wp", "[add|delete] [grid_id] [pause] [waypoint_id] [-h] - Add or delete a waypoint by grid ID. (-h to use current heading)", AccountStatus::GMAreas, command_wp) ||
		command_add("wpadd", "[pause] [-h] - Add your current location as a waypoint to your NPC target's AI path. (-h to use current heading)", AccountStatus::GMAreas, command_wpadd) ||
		command_add("worldwide", "Performs world-wide GM functions such as cast (can be extended for other commands). Use caution", AccountStatus::GMImpossible, command_worldwide) ||
		command_add("zone", "[Zone ID|Zone Short Name] [X] [Y] [Z] - Teleport to specified Zone by ID or Short Name (coordinates are optional)", AccountStatus::Guide, command_zone) ||
		command_add("zonebootup", "[ZoneServerID] [shortname] - Make a zone server boot a specific zone", AccountStatus::GMLeadAdmin, command_zonebootup) ||
		command_add("zoneinstance", "[Instance ID] [X] [Y] [Z] - Teleport to specified Instance by ID (coordinates are optional)", AccountStatus::Guide, command_zone_instance) ||
		command_add("zoneshard", "[zone] [instance_id] - Teleport explicitly to a zone shard", AccountStatus::Player, command_zone_shard) ||
		command_add("zoneshutdown", "[instance|zone] [Instance ID|Zone ID|Zone Short Name] - Shut down a zone server by Instance ID, Zone ID, or Zone Short Name", AccountStatus::GMLeadAdmin, command_zoneshutdown) ||
		command_add("zonevariable", "[clear|delete|set|view] - Modify zone variables for your current zone", AccountStatus::GMAdmin, command_zonevariable) ||
		command_add("zsave", " Saves zheader to the database", AccountStatus::QuestTroupe, command_zsave)
	) {
		command_deinit();
		return -1;
	}

	std::map<std::string, std::pair<uint8, std::vector<std::string>>> command_settings;
	database.GetCommandSettings(command_settings);
	database.GetCommandSubSettings(command_subsettings);

	std::vector<std::pair<std::string, uint8>> injected_command_settings;
	std::vector<std::string> orphaned_command_settings;

	// inject static sub command aliases
	// .e.g old #fi routes to #find item or #itemsearch routes to #find item
	for (auto& cs : command_settings) {
		for (const auto& e : command_subsettings) {
			if (cs.first == e.parent_command) {
				for (const auto& alias : Strings::Split(e.top_level_aliases, "|")) {
					cs.second.second.emplace_back(alias);
				}
			}
		}
	}

	for (const auto& cs : command_settings) {
		auto cl = commandlist.find(cs.first);
		if (cl == commandlist.end()) {
			orphaned_command_settings.push_back(cs.first);
			LogInfo(
				"Command [{}] no longer exists. Deleting orphaned entry from `command_settings` table.",
				cs.first
			);
		}
	}

	if (!orphaned_command_settings.empty()) {
		if (!database.UpdateOrphanedCommandSettings(orphaned_command_settings)) {
			LogInfo("Failed to process 'Orphaned Commands' update operation.");
		}
	}

	auto working_cl = commandlist;
	for (const auto& w : working_cl) {
		auto cs = command_settings.find(w.first);
		if (cs == command_settings.end()) {
			injected_command_settings.emplace_back(w.first, w.second->admin);
			LogInfo(
				"New Command [{}] found. Adding to `command_settings` table with admin status [{}]",
				w.first,
				w.second->admin
			);

			if (w.second->admin == AccountStatus::Player) {
				LogCommands(
					"Warning: Command [{}] defaulting to admin level 0!",
					w.first
				);
			}

			continue;
		}

		w.second->admin = cs->second.first;

		LogCommands(
			"Command [{}] set to admin level [{}]",
			w.first,
			cs->second.first
		);

		if (cs->second.second.empty()) {
			continue;
		}

		for (const auto& a : cs->second.second) {
			if (a.empty()) {
				continue;
			}

			if (commandlist.find(a) != commandlist.end()) {
				LogCommands(
					"command_init(): Warning: Alias [{}] already exists as a command - skipping!",
					a
				);

				continue;
			}

			commandlist[a]    = w.second;
			commandaliases[a] = w.first;

			LogCommands(
				"command_init(): - Alias [{}] added to command [{}]",
				a,
				commandaliases[a]
			);
		}
	}

	if (!injected_command_settings.empty()) {
		if (!database.UpdateInjectedCommandSettings(injected_command_settings)) {
			LogInfo("Failed to process 'Injected Commands' update operation.");
		}
	}

	command_dispatch = command_realdispatch;

	return command_count;
}


/*
 * command_deinit
 * clears the command list, freeing resources
 *
 * Parameters:
 *	none
 *
 */
void command_deinit(void)
{
	for (auto &c : command_delete_list) {
		delete c;
	}

	command_delete_list.clear();
	commandlist.clear();
	commandaliases.clear();

	command_dispatch = command_notavail;
	command_count = 0;
}

/*
 * command_add
 * adds a command to the command list; used by command_init
 *
 * Parameters:
 *	command_name - the command ex: "spawn"
 *	description - text description of command for #help
 *	admin - default admin level required to use command
 *	function - pointer to function that handles command
 *
 */
int command_add(std::string command_name, std::string description, uint8 admin, CmdFuncPtr function)
{
	if (command_name.empty()) {
		LogError("command_add() - Command added with empty name string - check command.cpp");
		return -1;
	}

	if (!function) {
		LogError("command_add() - Command [{}] added without a valid function pointer - check command.cpp", command_name);
		return -1;
	}

	if (commandlist.count(command_name)) {
		LogError("command_add() - Command [{}] is a duplicate command name - check command.cpp", command_name);
		return -1;
	}

	auto c = new CommandRecord;

	c->admin       = admin;
	c->description = description;
	c->function    = function;

	commands_map[command_name]   = admin;
	commandlist[command_name]    = c;
	commandaliases[command_name] = command_name;

	command_delete_list.push_back(c);
	command_count++;

	return 0;
}

uint8 GetCommandStatus(std::string command_name)
{
	return commands_map[command_name];
}

/*
 *
 * command_realdispatch
 * Calls the correct function to process the client's command string.
 * Called from Client::ChannelMessageReceived if message starts with
 * command character (#).
 *
 * Parameters:
 *	c			- pointer to the calling client object
 *	message		- what the client typed
 *
 */
int command_realdispatch(Client *c, std::string message, bool ignore_status)
{
	Seperator sep(message.c_str(), ' ', 10, 100, true); // "three word argument" should be considered 1 arg

	std::string command(sep.arg[0] + 1);

	if (commandlist.count(command) != 1) {
		return -2;
	}

	const CommandRecord* current_command = commandlist[command];

	bool is_subcommand            = false;
	bool can_use_subcommand       = false;
	bool found_subcommand_setting = false;

	const auto arguments = sep.argnum;

	if (arguments) {
		const std::string& sub_command = sep.arg[1];

		for (const auto &e : command_subsettings) {
			if (e.parent_command == command && e.sub_command == sub_command) {
				can_use_subcommand       = c->Admin() >= static_cast<int16>(e.access_level);
				is_subcommand            = true;
				found_subcommand_setting = true;
				break;
			}
		}

		if (!found_subcommand_setting) {
			for (const auto &e: command_subsettings) {
				if (e.parent_command == command && e.sub_command == sub_command) {
					can_use_subcommand = c->Admin() >= static_cast<int16>(e.access_level);
					is_subcommand      = true;
					break;
				}
			}
		}
	}

	if (!ignore_status) {
		if (!is_subcommand && c->Admin() < current_command->admin) {
			c->Message(Chat::White, "Your status is not high enough to use this command.");
			return -1;
		}

		if (is_subcommand && !can_use_subcommand) {
			c->Message(Chat::White, "Your status is not high enough to use this subcommand.");
			return -1;
		}
	}

	if (current_command->admin >= COMMANDS_LOGGING_MIN_STATUS) {
		LogCommands(
			"[{}] ([{}]) used command: [{}] (target=[{}])",
			c->GetName(),
			c->AccountName(),
			message,
			c->GetTarget() ? c->GetTarget()->GetName() : "NONE"
		);
	}

	if (!current_command->function) {
		LogError("Command [{}] has a null function", command);
		return -1;
	}

	if (parse->PlayerHasQuestSub(EVENT_GM_COMMAND)) {
		parse->EventPlayer(EVENT_GM_COMMAND, c, message, 0);
	}

	bool log_command = true;
	for (auto &cmd: Strings::Split(RuleS(Logging, PlayerEventsIgnoreGMCommands), ",")) {
		if (Strings::Contains(command, Strings::ToLower(cmd))) {
			log_command = false;
			break;
		}
	}

	if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::GM_COMMAND) && log_command) {
		auto e = PlayerEvent::GMCommandEvent{
			.message = message,
			.target = c->GetTarget() ? c->GetTarget()->GetName() : "NONE"
		};

		RecordPlayerEventLogWithClient(c, PlayerEvent::GM_COMMAND, e);
	}

	current_command->function(c, &sep);	// Dispatch C++ Command

	return 0;
}

void command_help(Client *c, const Seperator *sep)
{
	int found_count = 0;
	std::string command_link;
	std::string search_criteria = Strings::ToLower(sep->argplus[1]);

	for (const auto& cur : commandlist) {
		if (!search_criteria.empty()) {
			if (
				!Strings::Contains(cur.first, search_criteria) &&
				!Strings::Contains(cur.second->description, search_criteria)
			) {
				continue;
			}
		}

		if (c->Admin() < cur.second->admin) {
			continue;
		}

		command_link = Saylink::Silent(
			fmt::format(
				"{}{}",
				COMMAND_CHAR,
				cur.first
			)
		);

		c->Message(
			Chat::White,
			fmt::format(
				"{} | {}",
				command_link,
				cur.second->description
			).c_str()
		);

		found_count++;
	}

	if (parse->PlayerHasQuestSub(EVENT_COMMAND)) {
		auto event_parse = parse->EventPlayer(EVENT_COMMAND, c, sep->msg, 0);
		if (event_parse >= 1) {
			found_count += event_parse;
		}
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Command{} listed{}.",
			found_count,
			found_count != 1 ? "s" : "",
			(
				!search_criteria.empty() ?
				fmt::format(
					" matching '{}'",
					search_criteria
				) :
				""
			)
		).c_str()
	);
}

void command_hotfix(Client *c, const Seperator *sep)
{
	auto items_count = database.GetItemsCount();
	auto shared_items_count = database.GetSharedItemsCount();
	if (items_count != shared_items_count) {
		c->Message(Chat::Yellow, "Your database does not have the same item count as your shared memory.");

		c->Message(
			Chat::Yellow,
			fmt::format(
				"Database Count: {} Shared Memory Count: {}",
				items_count,
				shared_items_count
			).c_str()
		);

		c->Message(Chat::Yellow, "If you want to be able to add new items to your server while it is online, you need to create placeholder entries in the database ahead of time and do not add or remove rows/entries. Only modify the existing placeholder rows/entries to safely use #hotfix.");

		return;
	}

	auto spells_count = database.GetSpellsCount();
	auto shared_spells_count = database.GetSharedSpellsCount();
	if (spells_count != shared_spells_count) {
		c->Message(Chat::Yellow, "Your database does not have the same spell count as your shared memory.");

		c->Message(
			Chat::Yellow,
			fmt::format(
				"Database Count: {} Shared Memory Count: {}",
				spells_count,
				shared_spells_count
			).c_str()
		);

		c->Message(Chat::Yellow, "If you want to be able to add new spells to your server while it is online, you need to create placeholder entries in the database ahead of time and do not add or remove rows/entries. Only modify the existing placeholder rows/entries to safely use #hotfix.");

		c->Message(Chat::Yellow, "Note: You may still have to distribute a spell file, even with dynamic changes.");

		return;
	}

	std::string hotfix;
	database.GetVariable("hotfix_name", hotfix);

	std::string hotfix_name;
	if (!strcasecmp(hotfix.c_str(), "hotfix_")) {
		hotfix_name = "";
	}
	else {
		hotfix_name = "hotfix_";
	}

	c->Message(Chat::White, "Creating and applying hotfix");
	std::thread t1(
		[c, hotfix_name]() {

			std::string shared_memory_path;

#ifdef WIN32
			shared_memory_path = "shared_memory";
			if (File::Exists("bin/shared_memory.exe")) {
				shared_memory_path = "bin\\shared_memory.exe";
			}

			std::string hotfix_command;
			if (hotfix_name.length() > 0) {
				hotfix_command = fmt::format("\"{}\" -hotfix={}", shared_memory_path, hotfix_name);
			}
			else {
				hotfix_command = fmt::format("\"{}\"", shared_memory_path);
			}

			LogInfo("Running hotfix command [{}]", hotfix_command);
			if (system(hotfix_command.c_str())) {}
#else
			shared_memory_path = "./shared_memory";
			if (File::Exists("./bin/shared_memory")) {
				shared_memory_path = "./bin/shared_memory";
			}

			if (hotfix_name.length() > 0) {
				if (system(StringFormat("%s -hotfix=%s", shared_memory_path.c_str(), hotfix_name.c_str()).c_str())) {}
			}
			else {
				if (system(StringFormat("%s", shared_memory_path.c_str()).c_str())) {}
			}
#endif
			database.SetVariable("hotfix_name", hotfix_name);

			ServerPacket pack(ServerOP_ChangeSharedMem, hotfix_name.length() + 1);
			if (hotfix_name.length() > 0) {
				strcpy((char *) pack.pBuffer, hotfix_name.c_str());
			}
			worldserver.SendPacket(&pack);

			worldserver.SendEmoteMessage(0, 0, AccountStatus::ApprenticeGuide, Chat::Yellow, "Hotfix applied");
		}
	);

	t1.detach();
}

void command_load_shared_memory(Client *c, const Seperator *sep) {
	std::string hotfix;
	database.GetVariable("hotfix_name", hotfix);

	std::string hotfix_name;
	if(strcasecmp(hotfix.c_str(), sep->arg[1]) == 0) {
		c->Message(Chat::White, "Cannot attempt to load this shared memory segment as it is already loaded.");
		return;
	}

	hotfix_name = sep->arg[1];
	c->Message(Chat::White, "Loading shared memory segment %s", hotfix_name.c_str());
	std::thread t1([c,hotfix_name]() {
#ifdef WIN32
		if(hotfix_name.length() > 0) {
			if(system(StringFormat("shared_memory -hotfix=%s", hotfix_name.c_str()).c_str()));
		} else {
			if(system(StringFormat("shared_memory").c_str()));
		}
#else
		if(hotfix_name.length() > 0) {
			if(system(StringFormat("./shared_memory -hotfix=%s", hotfix_name.c_str()).c_str()));
		}
		else {
			if(system(StringFormat("./shared_memory").c_str()));
		}
#endif
		c->Message(Chat::White, "Shared memory segment finished loading.");
	});

	t1.detach();
}

void command_apply_shared_memory(Client *c, const Seperator *sep) {
	std::string hotfix;
	database.GetVariable("hotfix_name", hotfix);
	std::string hotfix_name = sep->arg[1];

	c->Message(Chat::White, "Applying shared memory segment %s", hotfix_name.c_str());
	database.SetVariable("hotfix_name", hotfix_name);

	ServerPacket pack(ServerOP_ChangeSharedMem, hotfix_name.length() + 1);
	if(hotfix_name.length() > 0) {
		strcpy((char*)pack.pBuffer, hotfix_name.c_str());
	}
	worldserver.SendPacket(&pack);
}

#include "bot_command.h"
// Function delegate to support the command interface for Bots with the client.
void command_bot(Client *c, const Seperator *sep)
{
	if (RuleB(Bots, Enabled)) {
		std::string bot_message = sep->msg;
		if (bot_message.compare("#bot") == 0) {
			bot_message[0] = BOT_COMMAND_CHAR;
		}
		else {
			bot_message = bot_message.substr(bot_message.find_first_not_of("#bot"));
			bot_message[0] = BOT_COMMAND_CHAR;
		}

		if (bot_command_dispatch(c, bot_message.c_str()) == -2) {
			if (parse->PlayerHasQuestSub(EVENT_BOT_COMMAND)) {
				int i = parse->EventPlayer(EVENT_BOT_COMMAND, c, bot_message, 0);
				if (i == 0 && !RuleB(Chat, SuppressCommandErrors)) {
					c->Message(Chat::Red, "Bot command '%s' not recognized.", bot_message.c_str());
				}
			}
			else {
				if (!RuleB(Chat, SuppressCommandErrors)) {
					c->Message(Chat::Red, "Bot command '%s' not recognized.", bot_message.c_str());
				}
			}
		}
	} else {
		c->Message(Chat::Red, "Bots are disabled on this server.");
	}
}
