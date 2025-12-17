#pragma once

#include "common/repositories/base/base_command_subsettings_repository.h"

#include "common/database.h"
#include "common/strings.h"

class CommandSubsettingsRepository: public BaseCommandSubsettingsRepository {
public:

    /**
     * This file was auto generated and can be modified and extended upon
     *
     * Base repository methods are automatically
     * generated in the "base" version of this repository. The base repository
     * is immutable and to be left untouched, while methods in this class
     * are used as extension methods for more specific persistence-layer
     * accessors or mutators.
     *
     * Base Methods (Subject to be expanded upon in time)
     *
     * Note: Not all tables are designed appropriately to fit functionality with all base methods
     *
     * InsertOne
     * UpdateOne
     * DeleteOne
     * FindOne
     * GetWhere(std::string where_filter)
     * DeleteWhere(std::string where_filter)
     * InsertMany
     * All
     *
     * Example custom methods in a repository
     *
     * CommandSubsettingsRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * CommandSubsettingsRepository::GetWhereNeverExpires()
     * CommandSubsettingsRepository::GetWhereXAndY()
     * CommandSubsettingsRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here
	static std::vector<CommandSubsettingsRepository::CommandSubsettings> GetAll(Database& db)
	{
		// these are the base definitions for command_subsettings and can be over-ridden by the database
		std::vector<CommandSubsettingsRepository::CommandSubsettings> static_records = {
			{.parent_command = "find", .sub_command = "aa", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "findaa"},
			{.parent_command = "find", .sub_command = "account", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "findaccount"},
			{.parent_command = "find", .sub_command = "body_type", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "findbodytype"},
			{.parent_command = "find", .sub_command = "bug_category", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "findbugcategory"},
			{.parent_command = "find", .sub_command = "character", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "findcharacter"},
			{.parent_command = "find", .sub_command = "class", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "findclass"},
			{.parent_command = "find", .sub_command = "comparison_type", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "findcomparisontype"},
			{.parent_command = "find", .sub_command = "currency", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "findcurrency"},
			{.parent_command = "find", .sub_command = "deity", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "finddeity"},
			{.parent_command = "find", .sub_command = "emote", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "findemote"},
			{.parent_command = "find", .sub_command = "faction", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "findfaction"},
			{.parent_command = "find", .sub_command = "item", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "fi|finditem|itemsearch"},
			{.parent_command = "find", .sub_command = "language", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "findlanguage"},
			{.parent_command = "find", .sub_command = "npc_type", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "fn|findnpc|findnpctype"},
			{.parent_command = "find", .sub_command = "object_type", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "findobjecttype"},
			{.parent_command = "find", .sub_command = "race", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "findrace"},
			{.parent_command = "find", .sub_command = "recipe", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "findrecipe"},
			{.parent_command = "find", .sub_command = "skill", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "findskill"},
			{.parent_command = "find", .sub_command = "special_ability", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "fsa|findspecialability"},
			{.parent_command = "find", .sub_command = "stance", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "findstance"},
			{.parent_command = "find", .sub_command = "spell", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "fs|findspell"},
			{.parent_command = "find", .sub_command = "task", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "findtask"},
			{.parent_command = "find", .sub_command = "zone", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "fz|findzone"},
			{.parent_command = "set", .sub_command = "aa_exp", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "setaaxp|setaaexp"},
			{.parent_command = "set", .sub_command = "aa_points", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "setaapts|setaapoints"},
			{.parent_command = "set", .sub_command = "adventure_points", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "setadventurepoints"},
			{.parent_command = "set", .sub_command = "alternate_currency", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "setaltcurrency"},
			{.parent_command = "set", .sub_command = "animation", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "setanim"},
			{.parent_command = "set", .sub_command = "anon", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "setanon"},
			{.parent_command = "set", .sub_command = "auto_login", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "setautologin"},
			{.parent_command = "set", .sub_command = "bind", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "bind"},
			{.parent_command = "set", .sub_command = "checksum", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "updatechecksum"},
			{.parent_command = "set", .sub_command = "class_permanent", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "permaclass"},
			{.parent_command = "set", .sub_command = "crystals", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "setcrystals"},
			{.parent_command = "set", .sub_command = "date", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "date"},
			{.parent_command = "set", .sub_command = "endurance", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "setendurance"},
			{.parent_command = "set", .sub_command = "endurance_full", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "endurance"},
			{.parent_command = "set", .sub_command = "exp", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "setxp|setexp"},
			{.parent_command = "set", .sub_command = "flymode", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "flymode"},
			{.parent_command = "set", .sub_command = "freeze", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "freeze|unfreeze"},
			{.parent_command = "set", .sub_command = "gender", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "gender"},
			{.parent_command = "set", .sub_command = "gender_permanent", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "permagender"},
			{.parent_command = "set", .sub_command = "gm", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "gm"},
			{.parent_command = "set", .sub_command = "gm_speed", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "gmspeed"},
			{.parent_command = "set", .sub_command = "gm_status", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "flag"},
			{.parent_command = "set", .sub_command = "god_mode", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "godmode"},
			{.parent_command = "set", .sub_command = "haste", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "haste"},
			{.parent_command = "set", .sub_command = "hero_model", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "heromodel"},
			{.parent_command = "set", .sub_command = "hide_me", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "hideme"},
			{.parent_command = "set", .sub_command = "hp", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "sethp"},
			{.parent_command = "set", .sub_command = "hp_full", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "heal"},
			{.parent_command = "set", .sub_command = "invulnerable", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "invul|invulnerable"},
			{.parent_command = "set", .sub_command = "language", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "setlanguage"},
			{.parent_command = "set", .sub_command = "last_name", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "lastname"},
			{.parent_command = "set", .sub_command = "level", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "level"},
			{.parent_command = "set", .sub_command = "loginserver_info", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "setlsinfo"},
			{.parent_command = "set", .sub_command = "mana", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "setmana"},
			{.parent_command = "set", .sub_command = "mana_full", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "mana"},
			{.parent_command = "set", .sub_command = "motd", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "motd"},
			{.parent_command = "set", .sub_command = "name", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "name"},
			{.parent_command = "set", .sub_command = "ooc_mute", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "oocmute"},
			{.parent_command = "set", .sub_command = "password", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "setpass"},
			{.parent_command = "set", .sub_command = "pvp", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "pvp"},
			{.parent_command = "set", .sub_command = "pvp_points", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "setpvppoints"},
			{.parent_command = "set", .sub_command = "race", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "race"},
			{.parent_command = "set", .sub_command = "race_permanent", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "permarace"},
			{.parent_command = "set", .sub_command = "server_locked", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "lock|serverlock|serverunlock|unlock"},
			{.parent_command = "set", .sub_command = "skill", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "setskill"},
			{.parent_command = "set", .sub_command = "skill_all", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "setallskill|setallskills|setskillall"},
			{.parent_command = "set", .sub_command = "skill_all_max", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "maxskills"},
			{.parent_command = "set", .sub_command = "start_zone", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "setstartzone"},
			{.parent_command = "set", .sub_command = "temporary_name", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "tempname"},
			{.parent_command = "set", .sub_command = "texture", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "texture"},
			{.parent_command = "set", .sub_command = "time", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "time"},
			{.parent_command = "set", .sub_command = "time_zone", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "timezone"},
			{.parent_command = "set", .sub_command = "title", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "title"},
			{.parent_command = "set", .sub_command = "title_suffix", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "titlesuffix"},
			{.parent_command = "set", .sub_command = "weather", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "weather"},
			{.parent_command = "set", .sub_command = "zone", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "zclip|zcolor|zheader|zonelock|zsafecoords|zsky|zunderworld"},
			{.parent_command = "show", .sub_command = "aas", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "showaas"},
			{.parent_command = "show", .sub_command = "aa_points", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "showaapoints|showaapts"},
			{.parent_command = "show", .sub_command = "auto_login", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "showautologin"},
			{.parent_command = "show", .sub_command = "aggro", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "aggro"},
			{.parent_command = "show", .sub_command = "buffs", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "showbuffs"},
			{.parent_command = "show", .sub_command = "buried_corpse_count", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "getplayerburiedcorpsecount"},
			{.parent_command = "show", .sub_command = "client_version_summary", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "cvs"},
			{.parent_command = "show", .sub_command = "currencies", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "viewcurrencies"},
			{.parent_command = "show", .sub_command = "distance", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "distance"},
			{.parent_command = "show", .sub_command = "emote", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "emoteview"},
			{.parent_command = "show", .sub_command = "field_of_view", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "fov"},
			{.parent_command = "show", .sub_command = "flags", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "flags"},
			{.parent_command = "show", .sub_command = "group_info", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "ginfo"},
			{.parent_command = "show", .sub_command = "hatelist", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "hatelist"},
			{.parent_command = "show", .sub_command = "inventory", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "peekinv"},
			{.parent_command = "show", .sub_command = "ip_lookup", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "iplookup"},
			{.parent_command = "show", .sub_command = "keyring", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "showkeyring"},
			{.parent_command = "show", .sub_command = "line_of_sight", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "checklos"},
			{.parent_command = "show", .sub_command = "network", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "network"},
			{.parent_command = "show", .sub_command = "network_stats", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "netstats"},
			{.parent_command = "show", .sub_command = "npc_global_loot", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "shownpcgloballoot"},
			{.parent_command = "show", .sub_command = "npc_stats", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "npcstats"},
			{.parent_command = "show", .sub_command = "npc_type", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "viewnpctype"},
			{.parent_command = "show", .sub_command = "peqzone_flags", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "peqzone_flags"},
			{.parent_command = "show", .sub_command = "petition", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "listpetition|viewpetition"},
			{.parent_command = "show", .sub_command = "petition_info", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "petitioninfo"},
			{.parent_command = "show", .sub_command = "proximity", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "proximity"},
			{.parent_command = "show", .sub_command = "quest_errors", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "questerrors"},
			{.parent_command = "show", .sub_command = "quest_globals", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "globalview"},
			{.parent_command = "show", .sub_command = "recipe", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "viewrecipe"},
			{.parent_command = "show", .sub_command = "server_info", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "serverinfo"},
			{.parent_command = "show", .sub_command = "skills", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "showskills"},
			{.parent_command = "show", .sub_command = "spawn_status", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "spawnstatus"},
			{.parent_command = "show", .sub_command = "spells", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "showspells"},
			{.parent_command = "show", .sub_command = "spells_list", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "showspellslist"},
			{.parent_command = "show", .sub_command = "stats", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "showstats"},
			{.parent_command = "show", .sub_command = "timers", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "timers"},
			{.parent_command = "show", .sub_command = "traps", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "trapinfo"},
			{.parent_command = "show", .sub_command = "uptime", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "uptime"},
			{.parent_command = "show", .sub_command = "variable", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "getvariable"},
			{.parent_command = "show", .sub_command = "version", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "version"},
			{.parent_command = "show", .sub_command = "waypoints", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "wpinfo"},
			{.parent_command = "show", .sub_command = "who", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "who"},
			{.parent_command = "show", .sub_command = "xtargets", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "xtargets"},
			{.parent_command = "show", .sub_command = "zone_global_loot", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "showzonegloballoot"},
			{.parent_command = "show", .sub_command = "zone_loot", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "viewzoneloot"},
			{.parent_command = "show", .sub_command = "zone_points", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "showzonepoints"},
			{.parent_command = "show", .sub_command = "zone_stats", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "zstats"},
			{.parent_command = "show", .sub_command = "zone_status", .access_level = AccountStatus::QuestTroupe, .top_level_aliases = "zonestatus"},
		};

		auto db_sub_settings = All(db);

		// inject commands in the database that are not already there
		std::vector<CommandSubsettingsRepository::CommandSubsettings> new_records = {};
		for (const auto &r: static_records) {
			bool found = false;
			for (const auto &db_r: db_sub_settings) {
				if (r.parent_command == db_r.parent_command && r.sub_command == db_r.sub_command) {
					found = true;
				}
			}

			if (!found) {
				LogInfo("New command [{}] sub-command [{}] added to database table [{}] !", r.parent_command, r.sub_command, TableName());
				new_records.emplace_back(r);
			}
		}

		if (!new_records.empty()) {
			InsertMany(db, new_records);
			// if any were added, we need to reload the list
			db_sub_settings = All(db);
		}

		// remove commands from the database that are not in the static list
		bool any_deleted = false;
		for (const auto &db_r: db_sub_settings) {
			bool found = false;
			for (const auto &r: static_records) {
				if (r.parent_command == db_r.parent_command && r.sub_command == db_r.sub_command) {
					found = true;
				}
			}

			if (!found) {
				LogInfo("Command [{}] sub-command [{}] removed from database table [{}] !", db_r.parent_command, db_r.sub_command, TableName());
				DeleteWhere(db, fmt::format("parent_command = '{}' AND sub_command = '{}'", db_r.parent_command, db_r.sub_command));
				any_deleted = true;
			}
		}

		// if any were deleted, re-read the database
		if (any_deleted) {
			db_sub_settings = All(db);
		}

		// return the final list of what's in the database
		return db_sub_settings;
	}
};
