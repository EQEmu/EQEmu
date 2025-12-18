#ifdef LUA_EQEMU

#include "lua_general.h"

#include "common/classes.h"
#include "common/content/world_content_service.h"
#include "common/data_bucket.h"
#include "common/events/player_event_logs.h"
#include "common/rulesys.h"
#include "common/timer.h"
#include "zone/dialogue_window.h"
#include "zone/dynamic_zone.h"
#include "zone/encounter.h"
#include "zone/lua_client.h"
#include "zone/lua_encounter.h"
#include "zone/lua_entity_list.h"
#include "zone/lua_expedition.h"
#include "zone/lua_item.h"
#include "zone/lua_iteminst.h"
#include "zone/lua_npc.h"
#include "zone/lua_spell.h"
#include "zone/lua_zone.h"
#include "zone/qglobals.h"
#include "zone/quest_parser_collection.h"
#include "zone/questmgr.h"
#include "zone/worldserver.h"
#include "zone/zone.h"

#include "lua.hpp"
#include "luabind/luabind.hpp"
#include <list>
#include <map>
#include <sstream>

struct Events { };
struct Factions { };
struct Slots { };
struct Materials { };
struct ClientVersions { };
struct Appearances { };
struct Classes { };
struct Skills { };
struct BodyTypes { };
struct Filters { };
struct MessageTypes { };
struct Rule { };
struct Journal_SpeakMode { };
struct Journal_Mode { };
struct ZoneIDs { };
struct LanguageIDs { };

struct lua_registered_event {
	std::string encounter_name;
	luabind::adl::object lua_reference;
	QuestEventID event_id;
};

extern std::map<std::string, std::list<lua_registered_event>> lua_encounter_events_registered;
extern std::map<std::string, bool> lua_encounters_loaded;
extern std::map<std::string, Encounter *> lua_encounters;

extern void MapOpcodes();
extern void ClearMappedOpcode(EmuOpcode op);

extern WorldServer worldserver;

void unregister_event(std::string package_name, std::string name, int evt);

void load_encounter(std::string name) {
	if(lua_encounters_loaded.count(name) > 0)
		return;
	auto enc = new Encounter(name.c_str());
	entity_list.AddEncounter(enc);
	lua_encounters[name] = enc;
	lua_encounters_loaded[name] = true;
	parse->EventEncounter(EVENT_ENCOUNTER_LOAD, name, "", 0);
}

void load_encounter_with_data(std::string name, std::string info_str) {
	if(lua_encounters_loaded.count(name) > 0)
		return;
	auto enc = new Encounter(name.c_str());
	entity_list.AddEncounter(enc);
	lua_encounters[name] = enc;
	lua_encounters_loaded[name] = true;
	std::vector<std::any> info_ptrs;
	info_ptrs.push_back(&info_str);
	parse->EventEncounter(EVENT_ENCOUNTER_LOAD, name, "", 0, &info_ptrs);
}

void unload_encounter(std::string name) {
	if(lua_encounters_loaded.count(name) == 0)
		return;

	auto liter = lua_encounter_events_registered.begin();
	while(liter != lua_encounter_events_registered.end()) {
		std::list<lua_registered_event> &elist = liter->second;
		auto iter = elist.begin();
		while(iter != elist.end()) {
			if((*iter).encounter_name.compare(name) == 0) {
				iter = elist.erase(iter);
			} else {
				++iter;
			}
		}

		if(elist.size() == 0) {
			lua_encounter_events_registered.erase(liter++);
		} else {
			++liter;
		}
	}

	lua_encounters[name]->Depop();
	lua_encounters.erase(name);
	lua_encounters_loaded.erase(name);
	parse->EventEncounter(EVENT_ENCOUNTER_UNLOAD, name, "", 0);
}

void unload_encounter_with_data(std::string name, std::string info_str) {
	if(lua_encounters_loaded.count(name) == 0)
		return;

	auto liter = lua_encounter_events_registered.begin();
	while(liter != lua_encounter_events_registered.end()) {
		std::list<lua_registered_event> &elist = liter->second;
		auto iter = elist.begin();
		while(iter != elist.end()) {
			if((*iter).encounter_name.compare(name) == 0) {
				iter = elist.erase(iter);
			}
			else {
				++iter;
			}
		}

		if(elist.size() == 0) {
			lua_encounter_events_registered.erase(liter++);
		}
		else {
			++liter;
		}
	}

	lua_encounters[name]->Depop();
	lua_encounters.erase(name);
	lua_encounters_loaded.erase(name);
	std::vector<std::any> info_ptrs;
	info_ptrs.push_back(&info_str);
	parse->EventEncounter(EVENT_ENCOUNTER_UNLOAD, name, "", 0, &info_ptrs);
}

void register_event(std::string package_name, std::string name, int evt, luabind::adl::object func) {
	if(lua_encounters_loaded.count(name) == 0)
		return;

	unregister_event(package_name, name, evt);

	lua_registered_event e;
	e.encounter_name = name;
	e.lua_reference = func;
	e.event_id = static_cast<QuestEventID>(evt);

	auto liter = lua_encounter_events_registered.find(package_name);
	if(liter == lua_encounter_events_registered.end()) {
		std::list<lua_registered_event> elist;
		elist.push_back(e);
		lua_encounter_events_registered[package_name] = elist;
	} else {
		std::list<lua_registered_event> &elist = liter->second;
		elist.push_back(e);
	}
}

void unregister_event(std::string package_name, std::string name, int evt) {
	auto liter = lua_encounter_events_registered.find(package_name);
	if(liter != lua_encounter_events_registered.end()) {
		std::list<lua_registered_event> elist = liter->second;
		auto iter = elist.begin();
		while(iter != elist.end()) {
			if(iter->event_id == evt && iter->encounter_name.compare(name) == 0) {
				iter = elist.erase(iter);
				break;
			}
			++iter;
		}
		lua_encounter_events_registered[package_name] = elist;
	}
}

void register_npc_event(std::string name, int evt, int npc_id, luabind::adl::object func) {
	if(luabind::type(func) == LUA_TFUNCTION) {
		std::stringstream package_name;
		package_name << "npc_" << npc_id;

		register_event(package_name.str(), name, evt, func);
	}
}

void register_npc_event(int evt, int npc_id, luabind::adl::object func) {
	std::string name = quest_manager.GetEncounter();
	register_npc_event(name, evt, npc_id, func);
}

void unregister_npc_event(std::string name, int evt, int npc_id) {
	std::stringstream package_name;
	package_name << "npc_" << npc_id;

	unregister_event(package_name.str(), name, evt);
}

void unregister_npc_event(int evt, int npc_id) {
	std::string name = quest_manager.GetEncounter();
	unregister_npc_event(name, evt, npc_id);
}

void register_player_event(std::string name, int evt, luabind::adl::object func) {
	if(luabind::type(func) == LUA_TFUNCTION) {
		register_event("player", name, evt, func);
	}
}

void register_player_event(int evt, luabind::adl::object func) {
	std::string name = quest_manager.GetEncounter();
	register_player_event(name, evt, func);
}

void unregister_player_event(std::string name, int evt) {
	unregister_event("player", name, evt);
}

void unregister_player_event(int evt) {
	std::string name = quest_manager.GetEncounter();
	unregister_player_event(name, evt);
}

void register_item_event(std::string name, int evt, int item_id, luabind::adl::object func) {
	std::string package_name = "item_";
	package_name += std::to_string(item_id);

	if(luabind::type(func) == LUA_TFUNCTION) {
		register_event(package_name, name, evt, func);
	}
}

void register_item_event(int evt, int item_id, luabind::adl::object func) {
	std::string name = quest_manager.GetEncounter();
	register_item_event(name, evt, item_id, func);
}

void unregister_item_event(std::string name, int evt, int item_id) {
	std::string package_name = "item_";
	package_name += std::to_string(item_id);

	unregister_event(package_name, name, evt);
}

void unregister_item_event(int evt, int item_id) {
	std::string name = quest_manager.GetEncounter();
	unregister_item_event(name, evt, item_id);
}

void register_spell_event(std::string name, int evt, int spell_id, luabind::adl::object func) {
	if(luabind::type(func) == LUA_TFUNCTION) {
		std::stringstream package_name;
		package_name << "spell_" << spell_id;

		register_event(package_name.str(), name, evt, func);
	}
}

void register_spell_event(int evt, int spell_id, luabind::adl::object func) {
	std::string name = quest_manager.GetEncounter();
	register_spell_event(name, evt, spell_id, func);
}

void unregister_spell_event(std::string name, int evt, int spell_id) {
	std::stringstream package_name;
	package_name << "spell_" << spell_id;

	unregister_event(package_name.str(), name, evt);
}

void unregister_spell_event(int evt, int spell_id) {
	std::string name = quest_manager.GetEncounter();
	unregister_spell_event(name, evt, spell_id);
}

Lua_Mob lua_spawn2(int npc_type, int grid, int unused, double x, double y, double z, double heading) {
	auto position = glm::vec4(x, y, z, heading);
	return Lua_Mob(quest_manager.spawn2(npc_type, grid, unused, position));
}

Lua_Mob lua_unique_spawn(int npc_type, int grid, int unused, double x, double y, double z, double heading = 0.0) {
	auto position = glm::vec4(x, y, z, heading);
	return Lua_Mob(quest_manager.unique_spawn(npc_type, grid, unused, position));
}

Lua_Mob lua_spawn_from_spawn2(uint32 spawn2_id) {
	return Lua_Mob(quest_manager.spawn_from_spawn2(spawn2_id));
}

void lua_enable_spawn2(int spawn2_id) {
	quest_manager.enable_spawn2(spawn2_id);
}

void lua_disable_spawn2(int spawn2_id) {
	quest_manager.disable_spawn2(spawn2_id);
}

void lua_set_timer(std::string timer, uint32 time_ms) {
	quest_manager.settimerMS(timer, time_ms);
}

void lua_set_timer(std::string timer, uint32 time_ms, Lua_ItemInst inst) {
	quest_manager.settimerMS(timer, time_ms, inst);
}

void lua_set_timer(std::string timer, uint32 time_ms, Lua_Mob mob) {
	quest_manager.settimerMS(timer, time_ms, mob);
}

void lua_set_timer(std::string timer, uint32 time_ms, Lua_Encounter enc) {
	quest_manager.settimerMS(timer, time_ms, enc);
}

void lua_stop_timer(std::string timer) {
	quest_manager.stoptimer(timer);
}

void lua_stop_timer(std::string timer, Lua_ItemInst inst) {
	quest_manager.stoptimer(timer, inst);
}

void lua_stop_timer(std::string timer, Lua_Mob mob) {
	quest_manager.stoptimer(timer, mob);
}

void lua_stop_timer(std::string timer, Lua_Encounter enc) {
	quest_manager.stoptimer(timer, enc);
}

void lua_stop_all_timers() {
	quest_manager.stopalltimers();
}

void lua_stop_all_timers(Lua_ItemInst inst) {
	quest_manager.stopalltimers(inst);
}

void lua_stop_all_timers(Lua_Mob mob) {
	quest_manager.stopalltimers(mob);
}

void lua_stop_all_timers(Lua_Encounter enc) {
	quest_manager.stopalltimers(enc);
}

void lua_pause_timer(std::string timer) {
	quest_manager.pausetimer(timer);
}

void lua_resume_timer(std::string timer) {
	quest_manager.resumetimer(timer);
}

bool lua_is_paused_timer(std::string timer) {
	return quest_manager.ispausedtimer(timer);
}

bool lua_has_timer(std::string timer) {
	return quest_manager.hastimer(timer);
}

uint32 lua_get_remaining_time(std::string timer) {
	return quest_manager.getremainingtimeMS(timer);
}

uint32 lua_get_timer_duration(std::string timer) {
	return quest_manager.gettimerdurationMS(timer);
}

void lua_depop() {
	quest_manager.depop(0);
}

void lua_depop(int npc_type) {
	quest_manager.depop(npc_type);
}

void lua_depop_with_timer() {
	quest_manager.depop_withtimer(0);
}

void lua_depop_with_timer(int npc_type) {
	quest_manager.depop_withtimer(npc_type);
}

void lua_depop_all() {
	quest_manager.depopall(0);
}

void lua_depop_all(int npc_type) {
	quest_manager.depopall(npc_type);
}

void lua_depop_zone(bool start_spawn_status) {
	quest_manager.depopzone(start_spawn_status);
}

void lua_repop_zone() {
	quest_manager.repopzone();
}

void lua_repop_zone(bool is_forced) {
	quest_manager.repopzone(is_forced);
}

void lua_process_mobs_while_zone_empty(bool on) {
	quest_manager.processmobswhilezoneempty(on);
}

bool lua_is_disc_tome(int item_id) {
	return quest_manager.isdisctome(item_id);
}

std::string lua_get_race_name(uint32 race_id) {
	return quest_manager.getracename(race_id);
}

std::string lua_get_spell_name(uint32 spell_id) {
	return quest_manager.getspellname(spell_id);
}

std::string lua_get_skill_name(int skill_id) {
	return quest_manager.getskillname(skill_id);
}

void lua_safe_move() {
	quest_manager.safemove();
}

void lua_rain(int weather) {
	quest_manager.rain(weather);
}

void lua_snow(int weather) {
	quest_manager.rain(weather);
}

int lua_scribe_spells(int max) {
	return quest_manager.scribespells(max);
}

int lua_scribe_spells(int max, int min) {
	return quest_manager.scribespells(max, min);
}

int lua_train_discs(int max) {
	return quest_manager.traindiscs(max);
}

int lua_train_discs(int max, int min) {
	return quest_manager.traindiscs(max, min);
}

void lua_set_sky(int sky) {
	quest_manager.setsky(sky);
}

void lua_set_guild(int guild_id, int rank) {
	quest_manager.SetGuild(guild_id, rank);
}

void lua_create_guild(const char *name, const char *leader) {
	quest_manager.CreateGuild(name, leader);
}

void lua_set_time(int hour, int min) {
	quest_manager.settime(hour, min, true);
}

void lua_set_time(int hour, int min, bool update_world) {
	quest_manager.settime(hour, min, update_world);
}

void lua_signal(int npc_id, int signal_id) {
	quest_manager.signalwith(npc_id, signal_id);
}

void lua_signal(int npc_id, int signal_id, int wait) {
	quest_manager.signalwith(npc_id, signal_id, wait);
}

void lua_set_global(const char *name, const char *value, int options, const char *duration) {
	quest_manager.setglobal(name, value, options, duration);
}

void lua_target_global(const char *name, const char *value, const char *duration, int npc_id, int char_id, int zone_id) {
	quest_manager.targlobal(name, value, duration, npc_id, char_id, zone_id);
}

void lua_delete_global(const char *name) {
	quest_manager.delglobal(name);
}

void lua_start(int wp) {
	quest_manager.start(wp);
}

void lua_stop() {
	quest_manager.stop();
}

void lua_pause(int duration) {
	quest_manager.pause(duration);
}

void lua_move_to(float x, float y, float z) {
	quest_manager.moveto(glm::vec4(x, y, z, 0.0f), false);
}

void lua_move_to(float x, float y, float z, float h) {
	quest_manager.moveto(glm::vec4(x, y, z, h), false);
}

void lua_move_to(float x, float y, float z, float h, bool save_guard_spot) {
	quest_manager.moveto(glm::vec4(x, y, z, h), save_guard_spot);
}

void lua_path_resume() {
	quest_manager.resume();
}

void lua_set_next_hp_event(int hp) {
	quest_manager.setnexthpevent(hp);
}

void lua_set_next_inc_hp_event(int hp) {
	quest_manager.setnextinchpevent(hp);
}

void lua_respawn(int npc_type, int grid) {
	quest_manager.respawn(npc_type, grid);
}

void lua_set_proximity(float min_x, float max_x, float min_y, float max_y) {
	quest_manager.set_proximity(min_x, max_x, min_y, max_y);
}

void lua_set_proximity(float min_x, float max_x, float min_y, float max_y, float min_z, float max_z) {
	quest_manager.set_proximity(min_x, max_x, min_y, max_y, min_z, max_z);
}

void lua_set_proximity(float min_x, float max_x, float min_y, float max_y, float min_z, float max_z, bool enable_say) {
	quest_manager.set_proximity(min_x, max_x, min_y, max_y, min_z, max_z, enable_say);
}

void lua_clear_proximity() {
	quest_manager.clear_proximity();
}

void lua_enable_proximity_say() {
	quest_manager.enable_proximity_say();
}

void lua_disable_proximity_say() {
	quest_manager.disable_proximity_say();
}

void lua_set_anim(int npc_type, int anim_num) {
	quest_manager.setanim(npc_type, anim_num);
}

void lua_spawn_condition(const char *zone, uint32 instance_id, int condition_id, int value) {
	quest_manager.spawn_condition(zone, instance_id, condition_id, value);
}

int lua_get_spawn_condition(const char *zone, uint32 instance_id, int condition_id) {
	return quest_manager.get_spawn_condition(zone, instance_id, condition_id);
}

void lua_toggle_spawn_event(int event_id, bool enable, bool strict, bool reset) {
	quest_manager.toggle_spawn_event(event_id, enable, strict, reset);
}

void lua_summon_buried_player_corpse(uint32 char_id, float x, float y, float z, float h) {
	quest_manager.summonburiedplayercorpse(char_id, glm::vec4(x, y, z, h));
}

void lua_summon_all_player_corpses(uint32 char_id, float x, float y, float z, float h) {
	quest_manager.summonallplayercorpses(char_id, glm::vec4(x, y, z, h));
}

int64 lua_get_player_corpse_count(uint32 character_id) {
	return database.CountCharacterCorpses(character_id);
}

int64 lua_get_player_corpse_count_by_zone_id(uint32 character_id, uint32 zone_id) {
	return database.CountCharacterCorpsesByZoneID(character_id, zone_id);
}

int64 lua_get_player_buried_corpse_count(uint32 character_id) {
	return quest_manager.getplayerburiedcorpsecount(character_id);
}

bool lua_bury_player_corpse(uint32 char_id) {
	return quest_manager.buryplayercorpse(char_id);
}

void lua_task_selector(luabind::adl::object table, bool ignore_cooldown) {
	if(luabind::type(table) != LUA_TTABLE) {
		return;
	}

	std::vector<int> tasks;
	for (int i = 1; i <= MAXCHOOSERENTRIES; ++i)
	{
		if (luabind::type(table[i]) == LUA_TNUMBER)
		{
			tasks.push_back(luabind::object_cast<int>(table[i]));
		}
	}

	quest_manager.taskselector(tasks, ignore_cooldown);
}

void lua_task_selector(luabind::adl::object table) {
	lua_task_selector(table, false);
}

void lua_task_set_selector(int task_set) {
	quest_manager.tasksetselector(task_set);
}

void lua_task_set_selector(int task_set, bool ignore_cooldown) {
	quest_manager.tasksetselector(task_set, ignore_cooldown);
}

void lua_enable_task(luabind::adl::object table) {
	if(luabind::type(table) != LUA_TTABLE) {
		return;
	}

	int tasks[MAXCHOOSERENTRIES] = { 0 };
	int count = 0;

	for(int i = 1; i <= MAXCHOOSERENTRIES; ++i) {
		auto cur = table[i];
		int cur_value = 0;
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				cur_value = luabind::object_cast<int>(cur);
			} catch(luabind::cast_failed &) {
			}
		} else {
			count = i - 1;
			break;
		}

		tasks[i - 1] = cur_value;
	}

	quest_manager.enabletask(count, tasks);
}

void lua_disable_task(luabind::adl::object table) {
	if(luabind::type(table) != LUA_TTABLE) {
		return;
	}

	int tasks[MAXCHOOSERENTRIES] = { 0 };
	int count = 0;

	for(int i = 1; i <= MAXCHOOSERENTRIES; ++i) {
		auto cur = table[i];
		int cur_value = 0;
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				cur_value = luabind::object_cast<int>(cur);
			} catch(luabind::cast_failed &) {
			}
		} else {
			count = i - 1;
			break;
		}

		tasks[i - 1] = cur_value;
	}

	quest_manager.disabletask(count, tasks);
}

bool lua_is_task_enabled(int task) {
	return quest_manager.istaskenabled(task);
}

bool lua_is_task_active(int task) {
	return quest_manager.istaskactive(task);
}

bool lua_is_task_activity_active(int task, int activity) {
	return quest_manager.istaskactivityactive(task, activity);
}

int lua_get_task_activity_done_count(int task, int activity) {
	return quest_manager.gettaskactivitydonecount(task, activity);
}

void lua_update_task_activity(int task, int activity, int count) {
	quest_manager.updatetaskactivity(task, activity, count);
}

void lua_reset_task_activity(int task, int activity) {
	quest_manager.resettaskactivity(task, activity);
}

void lua_assign_task(int task_id) {
	quest_manager.assigntask(task_id);
}

void lua_fail_task(int task_id) {
	quest_manager.failtask(task_id);
}

bool lua_complete_task(int task_id) {
	return quest_manager.completetask(task_id);
}

bool lua_uncomplete_task(int task_id) {
	return quest_manager.uncompletetask(task_id);
}

int lua_task_time_left(int task_id) {
	return quest_manager.tasktimeleft(task_id);
}

int lua_is_task_completed(int task_id) {
	return quest_manager.istaskcompleted(task_id);
}

int lua_enabled_task_count(int task_set) {
	return quest_manager.enabledtaskcount(task_set);
}

int lua_first_task_in_set(int task_set) {
	return quest_manager.firsttaskinset(task_set);
}

int lua_last_task_in_set(int task_set) {
	return quest_manager.lasttaskinset(task_set);
}

int lua_next_task_in_set(int task_set, int task_id) {
	return quest_manager.nexttaskinset(task_set, task_id);
}

int lua_active_speak_task() {
	return quest_manager.activespeaktask();
}

int lua_active_speak_activity(int task_id) {
	return quest_manager.activespeakactivity(task_id);
}

int lua_active_tasks_in_set(int task_set) {
	return quest_manager.activetasksinset(task_set);
}

int lua_completed_tasks_in_set(int task_set) {
	return quest_manager.completedtasksinset(task_set);
}

bool lua_is_task_appropriate(int task) {
	return quest_manager.istaskappropriate(task);
}

std::string lua_get_task_name(uint32 task_id) {
	return quest_manager.gettaskname(task_id);
}

int lua_get_dz_task_id() {
	return quest_manager.GetCurrentDzTaskID();
}

void lua_end_dz_task() {
	quest_manager.EndCurrentDzTask();
}

void lua_end_dz_task(bool send_fail) {
	quest_manager.EndCurrentDzTask(send_fail);
}

void lua_popup(const char *title, const char *text) {
	quest_manager.popup(title, text, 0, 0, 0);
}

void lua_popup(const char *title, const char *text, uint32 id) {
	quest_manager.popup(title, text, id, 0, 0);
}

void lua_popup(const char *title, const char *text, uint32 id, uint32 buttons) {
	quest_manager.popup(title, text, id, buttons, 0);
}

void lua_popup(const char *title, const char *text, uint32 id, uint32 buttons, uint32 duration) {
	quest_manager.popup(title, text, id, buttons, duration);
}

void lua_clear_spawn_timers() {
	quest_manager.clearspawntimers();
}

void lua_zone_emote(int type, const char *str) {
	quest_manager.ze(type, str);
}

void lua_world_emote(int type, const char *str) {
	quest_manager.we(type, str);
}

void lua_message(int color, const char *message) {
	quest_manager.message(color, message);
}

void lua_whisper(const char *message) {
	quest_manager.whisper(message);
}

int lua_get_level(int type) {
	return quest_manager.getlevel(type);
}

uint16 lua_create_ground_object(uint32 item_id, float x, float y, float z, float h) {
	return quest_manager.CreateGroundObject(item_id, glm::vec4(x, y, z, h));
}

uint16 lua_create_ground_object(uint32 item_id, float x, float y, float z, float h, uint32 decay_time) {
	return quest_manager.CreateGroundObject(item_id, glm::vec4(x, y, z, h), decay_time);
}

uint16 lua_create_ground_object_from_model(const char *model, float x, float y, float z, float h) {
	return quest_manager.CreateGroundObjectFromModel(model, glm::vec4(x, y, z, h));
}

uint16 lua_create_ground_object_from_model(const char *model, float x, float y, float z, float h, uint8 type) {
	return quest_manager.CreateGroundObjectFromModel(model, glm::vec4(x, y, z, h), type);
}

uint16 lua_create_ground_object_from_model(const char *model, float x, float y, float z, float h, uint8 type, uint32 decay_time) {
	return quest_manager.CreateGroundObjectFromModel(model, glm::vec4(x, y, z, h), type, decay_time);
}

uint16 lua_create_door(const char *model, float x, float y, float z, float h) {
	return quest_manager.CreateDoor(model, x, y, z, h, 58, 100);
}

uint16 lua_create_door(const char *model, float x, float y, float z, float h, uint8 open_type) {
	return quest_manager.CreateDoor(model, x, y, z, h, open_type, 100);
}

uint16 lua_create_door(const char *model, float x, float y, float z, float h, uint8 open_type, uint16 size) {
	return quest_manager.CreateDoor(model, x, y, z, h, open_type, size);
}

void lua_modify_npc_stat(std::string stat, std::string value) {
	quest_manager.ModifyNPCStat(stat, value);
}

int lua_collect_items(uint32 item_id, bool remove) {
	return quest_manager.collectitems(item_id, remove);
}

int lua_count_item(uint32 item_id) {
	return quest_manager.countitem(item_id);
}

void lua_remove_item(uint32 item_id) {
	quest_manager.removeitem(item_id);
}

void lua_remove_item(uint32 item_id, uint32 quantity) {
	quest_manager.removeitem(item_id, quantity);
}

void lua_update_spawn_timer(uint32 id, uint32 new_time) {
	quest_manager.UpdateSpawnTimer(id, new_time);
}

void lua_merchant_set_item(uint32 npc_id, uint32 item_id) {
	quest_manager.MerchantSetItem(npc_id, item_id);
}

void lua_merchant_set_item(uint32 npc_id, uint32 item_id, uint32 quantity) {
	quest_manager.MerchantSetItem(npc_id, item_id, quantity);
}

int lua_merchant_count_item(uint32 npc_id, uint32 item_id) {
	return quest_manager.MerchantCountItem(npc_id, item_id);
}

std::string lua_get_item_comment(uint32 item_id) {
	return quest_manager.getitemcomment(item_id);
}

std::string lua_get_item_lore(uint32 item_id) {
	return quest_manager.getitemlore(item_id);
}

std::string lua_get_item_name(uint32 item_id) {
	return quest_manager.getitemname(item_id);
}

std::string lua_say_link(std::string  text) {
	return Saylink::Create(text);
}

std::string lua_say_link(std::string text, bool silent) {
	return Saylink::Create(text, silent, text);
}

std::string lua_say_link(std::string text, bool silent, std::string link_name) {
	return Saylink::Create(text, silent, link_name);
}

void lua_set_rule(std::string rule_name, std::string rule_value) {
	RuleManager::Instance()->SetRule(rule_name, rule_value);
}

std::string lua_get_rule(std::string rule_name) {
	std::string rule_value;
	RuleManager::Instance()->GetRule(rule_name, rule_value);
	return rule_value;
}

std::string lua_get_data(std::string bucket_key) {
	return DataBucket::GetData(&database, bucket_key);
}

std::string lua_get_data_expires(std::string bucket_key) {
	return DataBucket::GetDataExpires(&database, bucket_key);
}

void lua_set_data(std::string bucket_key, std::string bucket_value) {
	DataBucket::SetData(&database, bucket_key, bucket_value);
}

void lua_set_data(std::string bucket_key, std::string bucket_value, std::string expires_at) {
	DataBucket::SetData(&database, bucket_key, bucket_value, expires_at);
}

bool lua_delete_data(std::string bucket_key) {
	return DataBucket::DeleteData(&database, bucket_key);
}

std::string lua_get_char_name_by_id(uint32 char_id) {
	return database.GetCharNameByID(char_id);
}

uint32 lua_get_char_id_by_name(const char* name) {
	return quest_manager.getcharidbyname(name);
}

std::string lua_get_class_name(uint8 class_id) {
	return quest_manager.getclassname(class_id);
}

std::string lua_get_class_name(uint8 class_id, uint8 level) {
	return quest_manager.getclassname(class_id, level);
}

uint32 lua_get_currency_id(uint32 item_id) {
	return quest_manager.getcurrencyid(item_id);
}

uint32 lua_get_currency_item_id(uint32 currency_id) {
	return quest_manager.getcurrencyitemid(currency_id);
}

const char *lua_get_guild_name_by_id(uint32 guild_id) {
	return quest_manager.getguildnamebyid(guild_id);
}

int lua_get_guild_id_by_char_id(uint32 char_id) {
	return database.GetGuildIDByCharID(char_id);
}

int lua_get_group_id_by_char_id(uint32 char_id) {
	return database.GetGroupIDByCharID(char_id);
}

std::string lua_get_npc_name_by_id(uint32 npc_id) {
	return quest_manager.getnpcnamebyid(npc_id);
}

int lua_get_raid_id_by_char_id(uint32 char_id) {
	return database.GetRaidIDByCharID(char_id);
}

uint32 lua_create_instance(const char *zone, uint32 version, uint32 duration) {
	return quest_manager.CreateInstance(zone, version, duration);
}

void lua_destroy_instance(uint32 instance_id) {
	quest_manager.DestroyInstance(instance_id);
}

void lua_update_instance_timer(uint16 instance_id, uint32 new_duration) {
	quest_manager.UpdateInstanceTimer(instance_id, new_duration);
}

uint32 lua_get_instance_timer() {
	return quest_manager.GetInstanceTimer();
}

uint32 lua_get_instance_timer_by_id(uint16 instance_id) {
	return quest_manager.GetInstanceTimerByID(instance_id);
}

int lua_get_instance_id(const char *zone, uint32 version) {
	return quest_manager.GetInstanceID(zone, version);
}

uint8 lua_get_instance_version_by_id(uint16 instance_id) {
	return database.GetInstanceVersion(instance_id);
}

uint32 lua_get_instance_zone_id_by_id(uint16 instance_id) {
	return database.GetInstanceZoneID(instance_id);
}

luabind::adl::object lua_get_instance_ids(lua_State* L, std::string zone_name) {
	luabind::adl::object ret = luabind::newtable(L);

	auto instance_ids = quest_manager.GetInstanceIDs(zone_name);
	for (int i = 0; i < instance_ids.size(); i++) {
		ret[i + 1] = instance_ids[i];
	}

	return ret;
}

luabind::adl::object lua_get_instance_ids_by_char_id(lua_State* L, std::string zone_name, uint32 character_id) {
	luabind::adl::object ret = luabind::newtable(L);

	auto instance_ids = quest_manager.GetInstanceIDs(zone_name, character_id);
	for (int i = 0; i < instance_ids.size(); i++) {
		ret[i + 1] = instance_ids[i];
	}

	return ret;
}

int lua_get_instance_id_by_char_id(const char *zone, uint32 version, uint32 character_id) {
	return quest_manager.GetInstanceIDByCharID(zone, version, character_id);
}

void lua_assign_to_instance(uint32 instance_id) {
	quest_manager.AssignToInstance(instance_id);
}

void lua_assign_to_instance_by_char_id(uint32 instance_id, uint32 character_id) {
	quest_manager.AssignToInstanceByCharID(instance_id, character_id);
}

void lua_assign_group_to_instance(uint32 instance_id) {
	quest_manager.AssignGroupToInstance(instance_id);
}

void lua_assign_raid_to_instance(uint32 instance_id) {
	quest_manager.AssignRaidToInstance(instance_id);
}

void lua_remove_from_instance(uint32 instance_id) {
	quest_manager.RemoveFromInstance(instance_id);
}

void lua_remove_from_instance_by_char_id(uint32 instance_id, uint32 character_id) {
	quest_manager.RemoveFromInstanceByCharID(instance_id, character_id);
}

bool lua_check_instance_by_char_id(uint32 instance_id, uint32 character_id) {
	return quest_manager.CheckInstanceByCharID(instance_id, character_id);
}

void lua_remove_all_from_instance(uint32 instance_id) {
	quest_manager.RemoveAllFromInstance(instance_id);
}

void lua_flag_instance_by_group_leader(uint32 zone, uint32 version) {
	quest_manager.FlagInstanceByGroupLeader(zone, version);
}

void lua_flag_instance_by_raid_leader(uint32 zone, uint32 version) {
	quest_manager.FlagInstanceByRaidLeader(zone, version);
}

void lua_fly_mode(int flymode) {
	quest_manager.FlyMode(static_cast<GravityBehavior>(flymode));
}

int lua_faction_value() {
	return quest_manager.FactionValue();
}

bool lua_check_title(uint32 title_set) {
	return quest_manager.checktitle(title_set);
}

void lua_enable_title(uint32 title_set) {
	quest_manager.enabletitle(title_set);
}

void lua_remove_title(uint32 title_set) {
	quest_manager.removetitle(title_set);
}

void lua_wear_change(uint32 slot, uint32 texture) {
	quest_manager.wearchange(slot, texture);
}

void lua_voice_tell(const char *str, uint32 macro_num, uint32 race_num, uint32 gender_num) {
	quest_manager.voicetell(str, macro_num, race_num, gender_num);
}

void lua_send_mail(const char *to, const char *from, const char *subject, const char *message) {
	quest_manager.SendMail(to, from, subject, message);
}

luabind::adl::object lua_get_qglobals(lua_State *L, Lua_NPC npc, Lua_Client client) {
	luabind::adl::object ret = luabind::newtable(L);

	NPC *n = npc;
	Client *c = client;

	std::list<QGlobal> global_map;
	QGlobalCache::GetQGlobals(global_map, n, c, zone);
	auto iter = global_map.begin();
	while(iter != global_map.end()) {
		ret[(*iter).name] = (*iter).value;
		++iter;
	}
	return ret;
}

luabind::adl::object lua_get_qglobals(lua_State *L, Lua_Client client) {
	luabind::adl::object ret = luabind::newtable(L);

	NPC *n = nullptr;
	Client *c = client;

	std::list<QGlobal> global_map;
	QGlobalCache::GetQGlobals(global_map, n, c, zone);
	auto iter = global_map.begin();
	while (iter != global_map.end()) {
		ret[(*iter).name] = (*iter).value;
		++iter;
	}
	return ret;
}

luabind::adl::object lua_get_qglobals(lua_State *L, Lua_NPC npc) {
	luabind::adl::object ret = luabind::newtable(L);

	NPC *n = npc;
	Client *c = nullptr;

	std::list<QGlobal> global_map;
	QGlobalCache::GetQGlobals(global_map, n, c, zone);
	auto iter = global_map.begin();
	while (iter != global_map.end()) {
		ret[(*iter).name] = (*iter).value;
		++iter;
	}
	return ret;
}

luabind::adl::object lua_get_qglobals(lua_State *L) {
	luabind::adl::object ret = luabind::newtable(L);

	NPC *n = nullptr;
	Client *c = nullptr;

	std::list<QGlobal> global_map;
	QGlobalCache::GetQGlobals(global_map, n, c, zone);
	auto iter = global_map.begin();
	while (iter != global_map.end()) {
		ret[(*iter).name] = (*iter).value;
		++iter;
	}
	return ret;
}

Lua_EntityList lua_get_entity_list() {
	return Lua_EntityList(&entity_list);
}

void lua_zone(const char* zone_name) {
	quest_manager.Zone(zone_name);
}

void lua_zone_group(const char* zone_name) {
	quest_manager.ZoneGroup(zone_name);
}

void lua_zone_raid(const char* zone_name) {
	quest_manager.ZoneRaid(zone_name);
}

int lua_get_zone_id() {
	if(!zone)
		return 0;

	return zone->GetZoneID();
}

int lua_get_zone_id_by_name(const char* zone_name) {
	return ZoneID(zone_name);
}

const char *lua_get_zone_long_name() {
	if(!zone)
		return "";

	return zone->GetLongName();
}

const char *lua_get_zone_long_name_by_name(const char* zone_name) {
	return ZoneLongName(
		ZoneID(zone_name),
		true
	);
}

const char *lua_get_zone_long_name_by_id(uint32 zone_id) {
	return ZoneLongName(zone_id, true);
}

const char *lua_get_zone_short_name() {
	if(!zone)
		return "";

	return zone->GetShortName();
}

const char *lua_get_zone_short_name_by_id(uint32 zone_id) {
	return ZoneName(zone_id, true);
}

int lua_get_zone_instance_id() {
	if(!zone)
		return 0;

	return zone->GetInstanceID();
}

int lua_get_zone_instance_version() {
	if(!zone)
		return 0;

	return zone->GetInstanceVersion();
}

luabind::adl::object lua_get_characters_in_instance(lua_State *L, uint16 instance_id) {
	luabind::adl::object ret = luabind::newtable(L);

	std::list<uint32> character_ids;
	uint16 i = 1;
	database.GetCharactersInInstance(instance_id, character_ids);
	auto iter = character_ids.begin();
	while(iter != character_ids.end()) {
		ret[i] = *iter;
		++i;
		++iter;
	}
	return ret;
}

int lua_get_zone_weather() {
	if (!zone) {
		return EQ::constants::WeatherTypes::None;
	}

	return zone->zone_weather;
}

luabind::adl::object lua_get_zone_time(lua_State *L) {
	TimeOfDay_Struct eqTime{};
	zone->zone_time.GetCurrentEQTimeOfDay(time(0), &eqTime);

	luabind::adl::object ret = luabind::newtable(L);
	ret["zone_hour"] = eqTime.hour - 1;
	ret["zone_minute"] = eqTime.minute;
	ret["zone_time"] = (eqTime.hour - 1) * 100 + eqTime.minute;
	return ret;
}

void lua_add_area(int id, int type, float min_x, float max_x, float min_y, float max_y, float min_z, float max_z) {
	entity_list.AddArea(id, type, min_x, max_x, min_y, max_y, min_z, max_z);
}

void lua_remove_area(int id) {
	entity_list.RemoveArea(id);
}

void lua_clear_areas() {
	entity_list.ClearAreas();
}

void lua_remove_spawn_point(uint32 spawn2_id) {
	if(zone) {
		LinkedListIterator<Spawn2*> iter(zone->spawn2_list);
		iter.Reset();

		while(iter.MoreElements()) {
			Spawn2* cur = iter.GetData();
			if(cur->GetID() == spawn2_id) {
				cur->ForceDespawn();
				iter.RemoveCurrent(true);
				return;
			}

			iter.Advance();
		}
	}
}

void lua_add_spawn_point(luabind::adl::object table) {
	if(!zone)
		return;

	if(luabind::type(table) == LUA_TTABLE) {
		uint32 spawn2_id;
		uint32 spawngroup_id;
		float x;
		float y;
		float z;
		float heading;
		uint32 respawn;
		uint32 variance;
		uint32 timeleft = 0;
		uint32 grid = 0;
		bool path_when_zone_idle = false;
		int condition_id = 0;
		int condition_min_value = 0;
		bool enabled = true;
		int animation = 0;

		auto cur = table["spawn2_id"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				spawn2_id = luabind::object_cast<uint32>(cur);
			} catch(luabind::cast_failed &) {
				return;
			}
		} else {
			return;
		}

		cur = table["spawngroup_id"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				spawngroup_id = luabind::object_cast<uint32>(cur);
			} catch(luabind::cast_failed &) {
				return;
			}
		} else {
			return;
		}

		cur = table["x"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				x = luabind::object_cast<float>(cur);
			} catch(luabind::cast_failed &) {
				return;
			}
		} else {
			return;
		}

		cur = table["y"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				y = luabind::object_cast<float>(cur);
			} catch(luabind::cast_failed &) {
				return;
			}
		} else {
			return;
		}

		cur = table["z"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				z = luabind::object_cast<float>(cur);
			} catch(luabind::cast_failed &) {
				return;
			}
		} else {
			return;
		}

		cur = table["heading"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				heading = luabind::object_cast<float>(cur);
			} catch(luabind::cast_failed &) {
				return;
			}
		} else {
			return;
		}

		cur = table["respawn"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				respawn = luabind::object_cast<uint32>(cur);
			} catch(luabind::cast_failed &) {
				return;
			}
		} else {
			return;
		}

		cur = table["variance"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				variance = luabind::object_cast<uint32>(cur);
			} catch(luabind::cast_failed &) {
				return;
			}
		} else {
			return;
		}

		cur = table["timeleft"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				timeleft = luabind::object_cast<uint32>(cur);
			} catch(luabind::cast_failed &) {
			}
		}

		cur = table["grid"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				grid = luabind::object_cast<uint32>(cur);
			} catch(luabind::cast_failed &) {
			}
		}

		cur = table["path_when_zone_idle"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				path_when_zone_idle = luabind::object_cast<bool>(cur);
			} catch(luabind::cast_failed &) {
			}
		}

		cur = table["condition_id"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				condition_id = luabind::object_cast<int>(cur);
			} catch(luabind::cast_failed &) {
			}
		}

		cur = table["condition_min_value"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				condition_min_value = luabind::object_cast<int>(cur);
			} catch(luabind::cast_failed &) {
			}
		}

		cur = table["enabled"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				enabled = luabind::object_cast<bool>(cur);
			} catch(luabind::cast_failed &) {
			}
		}

		cur = table["animation"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				animation = luabind::object_cast<int>(cur);
			} catch(luabind::cast_failed &) {
			}
		}

		lua_remove_spawn_point(spawn2_id);

		auto t = new Spawn2(spawn2_id, spawngroup_id, x, y, z, heading, respawn,
			variance, timeleft, grid, path_when_zone_idle, condition_id,
			condition_min_value, enabled, static_cast<EmuAppearance>(animation));

		zone->spawn2_list.Insert(t);
	}
}

void lua_attack(const char *client_name) {
	quest_manager.attack(client_name);
}

void lua_attack_npc(int entity_id) {
	quest_manager.attacknpc(entity_id);
}

void lua_attack_npc_type(int npc_type) {
	quest_manager.attacknpctype(npc_type);
}

void lua_follow(int entity_id) {
	quest_manager.follow(entity_id, 10);
}

void lua_follow(int entity_id, int distance) {
	quest_manager.follow(entity_id, distance);
}

void lua_stop_follow() {
	quest_manager.sfollow();
}

Lua_Client lua_get_initiator() {
	return quest_manager.GetInitiator();
}

Lua_Mob lua_get_owner() {
	return quest_manager.GetOwner();
}

Lua_ItemInst lua_get_quest_item() {
	return quest_manager.GetQuestItem();
}

Lua_Spell lua_get_quest_spell() {
	return quest_manager.GetQuestSpell();
}

std::string lua_get_encounter() {
	return quest_manager.GetEncounter();
}

void lua_map_opcodes() {
	MapOpcodes();
}

void lua_clear_opcode(int op) {
	ClearMappedOpcode(static_cast<EmuOpcode>(op));
}

bool lua_enable_recipe(uint32 recipe_id) {
	return quest_manager.EnableRecipe(recipe_id);
}

bool lua_disable_recipe(uint32 recipe_id) {
	return quest_manager.DisableRecipe(recipe_id);
}

void lua_clear_npctype_cache(int npctype_id) {
	quest_manager.ClearNPCTypeCache(npctype_id);
}

void lua_reloadzonestaticdata() {
	quest_manager.ReloadZoneStaticData();
}

double lua_clock() {
	timeval read_time;
	gettimeofday(&read_time, nullptr);
	uint32 t = read_time.tv_sec * 1000 + read_time.tv_usec / 1000;
	return static_cast<double>(t) / 1000.0;
}

void lua_log(int category, std::string message) {
	if (category < Logs::None || category >= Logs::MaxCategoryID)
		return;

	Log(Logs::General, static_cast<Logs::LogCategory>(category), message.c_str());
}

void lua_debug(std::string message) {
	Log(Logs::General, Logs::QuestDebug, message.c_str());
}

void lua_debug(std::string message, int level) {
	if (level < Logs::General || level > Logs::Detail)
		return;

	Log(static_cast<Logs::DebugLevel>(level), Logs::QuestDebug, message.c_str());
}

void lua_log_combat(std::string message) {
	Log(Logs::General, Logs::Combat, message.c_str());
}

void lua_log_spells(std::string message) {
	Log(Logs::General, Logs::Spells, message.c_str());
}

void lua_update_zone_header(std::string type, std::string value) {
	quest_manager.UpdateZoneHeader(type, value);
}

/**
 * Expansions
 */

bool lua_is_classic_enabled() {
	return WorldContentService::Instance()->IsClassicEnabled();
}

bool lua_is_the_ruins_of_kunark_enabled() {
	return WorldContentService::Instance()->IsTheRuinsOfKunarkEnabled();
}

bool lua_is_the_scars_of_velious_enabled() {
	return WorldContentService::Instance()->IsTheScarsOfVeliousEnabled();
}

bool lua_is_the_shadows_of_luclin_enabled() {
	return WorldContentService::Instance()->IsTheShadowsOfLuclinEnabled();
}

bool lua_is_the_planes_of_power_enabled() {
	return WorldContentService::Instance()->IsThePlanesOfPowerEnabled();
}

bool lua_is_the_legacy_of_ykesha_enabled() {
	return WorldContentService::Instance()->IsTheLegacyOfYkeshaEnabled();
}

bool lua_is_lost_dungeons_of_norrath_enabled() {
	return WorldContentService::Instance()->IsLostDungeonsOfNorrathEnabled();
}

bool lua_is_gates_of_discord_enabled() {
	return WorldContentService::Instance()->IsGatesOfDiscordEnabled();
}

bool lua_is_omens_of_war_enabled() {
	return WorldContentService::Instance()->IsOmensOfWarEnabled();
}

bool lua_is_dragons_of_norrath_enabled() {
	return WorldContentService::Instance()->IsDragonsOfNorrathEnabled();
}

bool lua_is_depths_of_darkhollow_enabled() {
	return WorldContentService::Instance()->IsDepthsOfDarkhollowEnabled();
}

bool lua_is_prophecy_of_ro_enabled() {
	return WorldContentService::Instance()->IsProphecyOfRoEnabled();
}

bool lua_is_the_serpents_spine_enabled() {
	return WorldContentService::Instance()->IsTheSerpentsSpineEnabled();
}

bool lua_is_the_buried_sea_enabled() {
	return WorldContentService::Instance()->IsTheBuriedSeaEnabled();
}

bool lua_is_secrets_of_faydwer_enabled() {
	return WorldContentService::Instance()->IsSecretsOfFaydwerEnabled();
}

bool lua_is_seeds_of_destruction_enabled() {
	return WorldContentService::Instance()->IsSeedsOfDestructionEnabled();
}

bool lua_is_underfoot_enabled() {
	return WorldContentService::Instance()->IsUnderfootEnabled();
}

bool lua_is_house_of_thule_enabled() {
	return WorldContentService::Instance()->IsHouseOfThuleEnabled();
}

bool lua_is_veil_of_alaris_enabled() {
	return WorldContentService::Instance()->IsVeilOfAlarisEnabled();
}

bool lua_is_rain_of_fear_enabled() {
	return WorldContentService::Instance()->IsRainOfFearEnabled();
}

bool lua_is_call_of_the_forsaken_enabled() {
	return WorldContentService::Instance()->IsCallOfTheForsakenEnabled();
}

bool lua_is_the_darkened_sea_enabled() {
	return WorldContentService::Instance()->IsTheDarkenedSeaEnabled();
}

bool lua_is_the_broken_mirror_enabled() {
	return WorldContentService::Instance()->IsTheBrokenMirrorEnabled();
}

bool lua_is_empires_of_kunark_enabled() {
	return WorldContentService::Instance()->IsEmpiresOfKunarkEnabled();
}

bool lua_is_ring_of_scale_enabled() {
	return WorldContentService::Instance()->IsRingOfScaleEnabled();
}

bool lua_is_the_burning_lands_enabled() {
	return WorldContentService::Instance()->IsTheBurningLandsEnabled();
}

bool lua_is_torment_of_velious_enabled() {
	return WorldContentService::Instance()->IsTormentOfVeliousEnabled();
}

bool lua_is_current_expansion_classic() {
	return WorldContentService::Instance()->IsCurrentExpansionClassic();
}

bool lua_is_current_expansion_the_ruins_of_kunark() {
	return WorldContentService::Instance()->IsCurrentExpansionTheRuinsOfKunark();
}

bool lua_is_current_expansion_the_scars_of_velious() {
	return WorldContentService::Instance()->IsCurrentExpansionTheScarsOfVelious();
}

bool lua_is_current_expansion_the_shadows_of_luclin() {
	return WorldContentService::Instance()->IsCurrentExpansionTheShadowsOfLuclin();
}

bool lua_is_current_expansion_the_planes_of_power() {
	return WorldContentService::Instance()->IsCurrentExpansionThePlanesOfPower();
}

bool lua_is_current_expansion_the_legacy_of_ykesha() {
	return WorldContentService::Instance()->IsCurrentExpansionTheLegacyOfYkesha();
}

bool lua_is_current_expansion_lost_dungeons_of_norrath() {
	return WorldContentService::Instance()->IsCurrentExpansionLostDungeonsOfNorrath();
}

bool lua_is_current_expansion_gates_of_discord() {
	return WorldContentService::Instance()->IsCurrentExpansionGatesOfDiscord();
}

bool lua_is_current_expansion_omens_of_war() {
	return WorldContentService::Instance()->IsCurrentExpansionOmensOfWar();
}

bool lua_is_current_expansion_dragons_of_norrath() {
	return WorldContentService::Instance()->IsCurrentExpansionDragonsOfNorrath();
}

bool lua_is_current_expansion_depths_of_darkhollow() {
	return WorldContentService::Instance()->IsCurrentExpansionDepthsOfDarkhollow();
}

bool lua_is_current_expansion_prophecy_of_ro() {
	return WorldContentService::Instance()->IsCurrentExpansionProphecyOfRo();
}

bool lua_is_current_expansion_the_serpents_spine() {
	return WorldContentService::Instance()->IsCurrentExpansionTheSerpentsSpine();
}

bool lua_is_current_expansion_the_buried_sea() {
	return WorldContentService::Instance()->IsCurrentExpansionTheBuriedSea();
}

bool lua_is_current_expansion_secrets_of_faydwer() {
	return WorldContentService::Instance()->IsCurrentExpansionSecretsOfFaydwer();
}

bool lua_is_current_expansion_seeds_of_destruction() {
	return WorldContentService::Instance()->IsCurrentExpansionSeedsOfDestruction();
}

bool lua_is_current_expansion_underfoot() {
	return WorldContentService::Instance()->IsCurrentExpansionUnderfoot();
}

bool lua_is_current_expansion_house_of_thule() {
	return WorldContentService::Instance()->IsCurrentExpansionHouseOfThule();
}

bool lua_is_current_expansion_veil_of_alaris() {
	return WorldContentService::Instance()->IsCurrentExpansionVeilOfAlaris();
}

bool lua_is_current_expansion_rain_of_fear() {
	return WorldContentService::Instance()->IsCurrentExpansionRainOfFear();
}

bool lua_is_current_expansion_call_of_the_forsaken() {
	return WorldContentService::Instance()->IsCurrentExpansionCallOfTheForsaken();
}

bool lua_is_current_expansion_the_darkened_sea() {
	return WorldContentService::Instance()->IsCurrentExpansionTheDarkenedSea();
}

bool lua_is_current_expansion_the_broken_mirror() {
	return WorldContentService::Instance()->IsCurrentExpansionTheBrokenMirror();
}

bool lua_is_current_expansion_empires_of_kunark() {
	return WorldContentService::Instance()->IsCurrentExpansionEmpiresOfKunark();
}

bool lua_is_current_expansion_ring_of_scale() {
	return WorldContentService::Instance()->IsCurrentExpansionRingOfScale();
}

bool lua_is_current_expansion_the_burning_lands() {
	return WorldContentService::Instance()->IsCurrentExpansionTheBurningLands();
}

bool lua_is_current_expansion_torment_of_velious() {
	return WorldContentService::Instance()->IsCurrentExpansionTormentOfVelious();
}

bool lua_is_content_flag_enabled(std::string content_flag){
	return WorldContentService::Instance()->IsContentFlagEnabled(content_flag);
}

void lua_set_content_flag(std::string flag_name, bool enabled){
	WorldContentService::Instance()->SetContentFlag(flag_name, enabled);
	zone->ReloadContentFlags();
}

Lua_Expedition lua_get_expedition() {
	if (zone && zone->GetInstanceID() != 0)
	{
		return DynamicZone::FindExpeditionByZone(zone->GetZoneID(), zone->GetInstanceID());
	}
	return nullptr;
}

Lua_Expedition lua_get_expedition_by_char_id(uint32 char_id) {
	return DynamicZone::FindExpeditionByCharacter(char_id);
}

Lua_Expedition lua_get_expedition_by_dz_id(uint32 dz_id) {
	return DynamicZone::FindDynamicZoneByID(dz_id, DynamicZoneType::Expedition);
}

Lua_Expedition lua_get_expedition_by_zone_instance(uint32 zone_id, uint32 instance_id) {
	return DynamicZone::FindExpeditionByZone(zone_id, instance_id);
}

luabind::object lua_get_expedition_lockout_by_char_id(lua_State* L, uint32 char_id, std::string expedition_name, std::string event_name) {
	luabind::adl::object lua_table = luabind::newtable(L);

	auto lockouts = DynamicZone::GetCharacterLockouts(char_id);

	auto it = std::find_if(lockouts.begin(), lockouts.end(), [&](const DzLockout& lockout) {
		return lockout.IsSame(expedition_name, event_name);
	});

	if (it != lockouts.end())
	{
		lua_table["remaining"] = it->GetSecondsRemaining();
		lua_table["uuid"] = it->UUID();
	}

	return lua_table;
}

luabind::object lua_get_expedition_lockouts_by_char_id(lua_State* L, uint32 char_id) {
	luabind::adl::object lua_table = luabind::newtable(L);

	auto lockouts = DynamicZone::GetCharacterLockouts(char_id);
	for (const auto& lockout : lockouts)
	{
		auto lockout_table = lua_table[lockout.DzName()];
		if (luabind::type(lockout_table) != LUA_TTABLE)
		{
			lockout_table = luabind::newtable(L);
		}

		auto event_table = lockout_table[lockout.Event()];
		if (luabind::type(event_table) != LUA_TTABLE)
		{
			event_table = luabind::newtable(L);
		}

		event_table["remaining"] = lockout.GetSecondsRemaining();
		event_table["uuid"] = lockout.UUID();
	}
	return lua_table;
}

luabind::object lua_get_expedition_lockouts_by_char_id(lua_State* L, uint32 char_id, std::string expedition_name) {
	luabind::adl::object lua_table = luabind::newtable(L);

	auto lockouts = DynamicZone::GetCharacterLockouts(char_id);
	for (const auto& lockout : lockouts)
	{
		if (lockout.DzName() == expedition_name)
		{
			auto event_table = lua_table[lockout.Event()];
			if (luabind::type(event_table) != LUA_TTABLE)
			{
				event_table = luabind::newtable(L);
			}
			event_table["remaining"] = lockout.GetSecondsRemaining();
			event_table["uuid"] = lockout.UUID();
		}
	}
	return lua_table;
}

void lua_add_expedition_lockout_all_clients(std::string expedition_name, std::string event_name, uint32 seconds) {
	auto lockout = DzLockout::Create(expedition_name, event_name, seconds);
	DynamicZone::AddClientsLockout(lockout);
}

void lua_add_expedition_lockout_all_clients(std::string expedition_name, std::string event_name, uint32 seconds, std::string uuid) {
	auto lockout = DzLockout::Create(expedition_name, event_name, seconds, uuid);
	DynamicZone::AddClientsLockout(lockout);
}

void lua_add_expedition_lockout_by_char_id(uint32 char_id, std::string expedition_name, std::string event_name, uint32 seconds) {
	DynamicZone::AddCharacterLockout(char_id, expedition_name, event_name, seconds);
}

void lua_add_expedition_lockout_by_char_id(uint32 char_id, std::string expedition_name, std::string event_name, uint32 seconds, std::string uuid) {
	DynamicZone::AddCharacterLockout(char_id, expedition_name, event_name, seconds, uuid);
}

void lua_remove_expedition_lockout_by_char_id(uint32 char_id, std::string expedition_name, std::string event_name) {
	DynamicZone::RemoveCharacterLockouts(char_id, expedition_name, event_name);
}

void lua_remove_all_expedition_lockouts_by_char_id(uint32 char_id) {
	DynamicZone::RemoveCharacterLockouts(char_id);
}

void lua_remove_all_expedition_lockouts_by_char_id(uint32 char_id, std::string expedition_name) {
	DynamicZone::RemoveCharacterLockouts(char_id, expedition_name);
}

std::string lua_seconds_to_time(int duration) {
	return Strings::SecondsToTime(duration);
}

uint32 lua_time_to_seconds(std::string time_string) {
	return Strings::TimeToSeconds(time_string);
}

std::string lua_get_hex_color_code(std::string color_name) {
	return quest_manager.gethexcolorcode(color_name);
}

float lua_get_aa_exp_modifier_by_char_id(uint32 character_id, uint32 zone_id) {
	return quest_manager.GetAAEXPModifierByCharID(character_id, zone_id);
}

float lua_get_aa_exp_modifier_by_char_id(uint32 character_id, uint32 zone_id, int16 instance_version) {
	return quest_manager.GetAAEXPModifierByCharID(character_id, zone_id, instance_version);
}

float lua_get_exp_modifier_by_char_id(uint32 character_id, uint32 zone_id) {
	return quest_manager.GetEXPModifierByCharID(character_id, zone_id);
}

float lua_get_exp_modifier_by_char_id(uint32 character_id, uint32 zone_id, int16 instance_version) {
	return quest_manager.GetEXPModifierByCharID(character_id, zone_id, instance_version);
}

void lua_set_aa_exp_modifier_by_char_id(uint32 character_id, uint32 zone_id, float aa_modifier) {
	quest_manager.SetAAEXPModifierByCharID(character_id, zone_id, aa_modifier);
}

void lua_set_aa_exp_modifier_by_char_id(uint32 character_id, uint32 zone_id, float aa_modifier, int16 instance_version) {
	quest_manager.SetAAEXPModifierByCharID(character_id, zone_id, aa_modifier, instance_version);
}

void lua_set_exp_modifier_by_char_id(uint32 character_id, uint32 zone_id, float exp_modifier) {
	quest_manager.SetEXPModifierByCharID(character_id, zone_id, exp_modifier);
}

void lua_set_exp_modifier_by_char_id(uint32 character_id, uint32 zone_id, float exp_modifier, int16 instance_version) {
	quest_manager.SetEXPModifierByCharID(character_id, zone_id, exp_modifier, instance_version);
}

void lua_add_ldon_loss(uint32 theme_id) {
	quest_manager.addldonloss(theme_id);
}

void lua_add_ldon_points(uint32 theme_id, int points) {
	quest_manager.addldonpoints(theme_id, points);
}

void lua_add_ldon_win(uint32 theme_id) {
	quest_manager.addldonwin(theme_id);
}

void lua_remove_ldon_loss(uint32 theme_id) {
	quest_manager.removeldonloss(theme_id);
}

void lua_remove_ldon_win(uint32 theme_id) {
	quest_manager.removeldonwin(theme_id);
}

std::string lua_get_clean_npc_name_by_id(uint32 npc_id) {
	return quest_manager.getcleannpcnamebyid(npc_id);
}

std::string lua_get_gender_name(uint32 gender_id) {
	return quest_manager.getgendername(gender_id);
}

std::string lua_get_deity_name(uint32 deity_id) {
	return quest_manager.getdeityname(deity_id);
}

std::string lua_get_inventory_slot_name(int16 slot_id) {
	return quest_manager.getinventoryslotname(slot_id);
}

void lua_rename(std::string name) {
	quest_manager.rename(name);
}

std::string lua_get_data_remaining(std::string bucket_name) {
	return DataBucket::GetDataRemaining(&database, bucket_name);
}

const int lua_get_item_stat(uint32 item_id, std::string identifier) {
	return quest_manager.getitemstat(item_id, identifier);
}

int lua_get_spell_stat(uint32 spell_id, std::string stat_identifier) {
	return quest_manager.getspellstat(spell_id, stat_identifier);
}

int lua_get_spell_stat(uint32 spell_id, std::string stat_identifier, uint8 slot) {
	return quest_manager.getspellstat(spell_id, stat_identifier, slot);
}

void lua_cross_zone_add_ldon_loss_by_char_id(int character_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddLoss;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, character_id, theme_id);
}

void lua_cross_zone_add_ldon_loss_by_group_id(int group_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddLoss;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, group_id, theme_id);
}

void lua_cross_zone_add_ldon_loss_by_raid_id(int raid_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddLoss;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, raid_id, theme_id);
}

void lua_cross_zone_add_ldon_loss_by_guild_id(int guild_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddLoss;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, guild_id, theme_id);
}

void lua_cross_zone_add_ldon_loss_by_expedition_id(uint32 expedition_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddLoss;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, expedition_id, theme_id);
}

void lua_cross_zone_add_ldon_loss_by_client_name(const char* client_name, uint32 theme_id) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddLoss;
	int update_identifier = 0;
	int points = 1;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, update_identifier, theme_id, points, client_name);
}

void lua_cross_zone_add_ldon_points_by_char_id(int character_id, uint32 theme_id, int points) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddPoints;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, character_id, theme_id, points);
}

void lua_cross_zone_add_ldon_points_by_group_id(int group_id, uint32 theme_id, int points) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddPoints;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, group_id, theme_id, points);
}

void lua_cross_zone_add_ldon_points_by_raid_id(int raid_id, uint32 theme_id, int points) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddPoints;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, raid_id, theme_id, points);
}

void lua_cross_zone_add_ldon_points_by_guild_id(int guild_id, uint32 theme_id, int points) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddPoints;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, guild_id, theme_id, points);
}

void lua_cross_zone_add_ldon_points_by_expedition_id(uint32 expedition_id, uint32 theme_id, int points) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddPoints;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, expedition_id, theme_id, points);
}

void lua_cross_zone_add_ldon_points_by_client_name(const char* client_name, uint32 theme_id, int points) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddPoints;
	int update_identifier = 0;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, update_identifier, theme_id, points, client_name);
}

void lua_cross_zone_add_ldon_win_by_char_id(int character_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddWin;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, character_id, theme_id);
}

void lua_cross_zone_add_ldon_win_by_group_id(int group_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddWin;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, group_id, theme_id);
}

void lua_cross_zone_add_ldon_win_by_raid_id(int raid_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddWin;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, raid_id, theme_id);
}

void lua_cross_zone_add_ldon_win_by_guild_id(int guild_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddWin;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, guild_id, theme_id);
}

void lua_cross_zone_add_ldon_win_by_expedition_id(uint32 expedition_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddWin;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, expedition_id, theme_id);
}

void lua_cross_zone_add_ldon_win_by_client_name(const char* client_name, uint32 theme_id) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddWin;
	int update_identifier = 0;
	int points = 1;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, update_identifier, theme_id, points, client_name);
}

void lua_cross_zone_assign_task_by_char_id(int character_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZTaskUpdateSubtype_AssignTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, character_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_assign_task_by_char_id(int character_id, uint32 task_id, bool enforce_level_requirement) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZTaskUpdateSubtype_AssignTask;
	int task_subidentifier = -1;
	int update_count = 1;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, character_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_assign_task_by_group_id(int group_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZTaskUpdateSubtype_AssignTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, group_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_assign_task_by_group_id(int group_id, uint32 task_id, bool enforce_level_requirement) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZTaskUpdateSubtype_AssignTask;
	int task_subidentifier = -1;
	int update_count = 1;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, group_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_assign_task_by_raid_id(int raid_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZTaskUpdateSubtype_AssignTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, raid_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_assign_task_by_raid_id(int raid_id, uint32 task_id, bool enforce_level_requirement) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZTaskUpdateSubtype_AssignTask;
	int task_subidentifier = -1;
	int update_count = 1;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, raid_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_assign_task_by_guild_id(int guild_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZTaskUpdateSubtype_AssignTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, guild_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_assign_task_by_guild_id(int guild_id, uint32 task_id, bool enforce_level_requirement) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZTaskUpdateSubtype_AssignTask;
	int task_subidentifier = -1;
	int update_count = 1;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, guild_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_assign_task_by_expedition_id(uint32 expedition_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZTaskUpdateSubtype_AssignTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, expedition_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_assign_task_by_expedition_id(uint32 expedition_id, uint32 task_id, bool enforce_level_requirement) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZTaskUpdateSubtype_AssignTask;
	int task_subidentifier = -1;
	int update_count = 1;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, expedition_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_assign_task_by_client_name(const char* client_name, uint32 task_id) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZTaskUpdateSubtype_AssignTask;
	int update_identifier = 0;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, update_identifier, task_id, task_subidentifier, update_count, enforce_level_requirement, client_name);
}

void lua_cross_zone_assign_task_by_client_name(const char* client_name, uint32 task_id, bool enforce_level_requirement) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZTaskUpdateSubtype_AssignTask;
	int update_identifier = 0;
	int task_subidentifier = -1;
	int update_count = 1;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, update_identifier, task_id, task_subidentifier, update_count, enforce_level_requirement, client_name);
}

void lua_cross_zone_cast_spell_by_char_id(int character_id, uint32 spell_id) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZSpellUpdateSubtype_Cast;
	quest_manager.CrossZoneSpell(update_type, update_subtype, character_id, spell_id);
}

void lua_cross_zone_cast_spell_by_group_id(int group_id, uint32 spell_id) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZSpellUpdateSubtype_Cast;
	quest_manager.CrossZoneSpell(update_type, update_subtype, group_id, spell_id);
}

void lua_cross_zone_cast_spell_by_raid_id(int raid_id, uint32 spell_id) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZSpellUpdateSubtype_Cast;
	quest_manager.CrossZoneSpell(update_type, update_subtype, raid_id, spell_id);
}

void lua_cross_zone_cast_spell_by_guild_id(int guild_id, uint32 spell_id) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZSpellUpdateSubtype_Cast;
	quest_manager.CrossZoneSpell(update_type, update_subtype, guild_id, spell_id);
}

void lua_cross_zone_cast_spell_by_expedition_id(uint32 expedition_id, uint32 spell_id) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZSpellUpdateSubtype_Cast;
	quest_manager.CrossZoneSpell(update_type, update_subtype, expedition_id, spell_id);
}

void lua_cross_zone_cast_spell_by_client_name(const char* client_name, uint32 spell_id) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZSpellUpdateSubtype_Cast;
	int update_identifier = 0;
	quest_manager.CrossZoneSpell(update_type, update_subtype, update_identifier, spell_id, client_name);
}

void lua_cross_zone_dialogue_window_by_char_id(int character_id, const char* message) {
	uint8 update_type = CZUpdateType_Character;
	quest_manager.CrossZoneDialogueWindow(update_type, character_id, message);
}

void lua_cross_zone_dialogue_window_by_group_id(int group_id, const char* message) {
	uint8 update_type = CZUpdateType_Group;
	quest_manager.CrossZoneDialogueWindow(update_type, group_id, message);
}

void lua_cross_zone_dialogue_window_by_raid_id(int raid_id, const char* message) {
	uint8 update_type = CZUpdateType_Raid;
	quest_manager.CrossZoneDialogueWindow(update_type, raid_id, message);
}

void lua_cross_zone_dialogue_window_by_guild_id(int guild_id, const char* message) {
	uint8 update_type = CZUpdateType_Guild;
	quest_manager.CrossZoneDialogueWindow(update_type, guild_id, message);
}

void lua_cross_zone_dialogue_window_by_expedition_id(uint32 expedition_id, const char* message) {
	uint8 update_type = CZUpdateType_Expedition;
	quest_manager.CrossZoneDialogueWindow(update_type, expedition_id, message);
}

void lua_cross_zone_dialogue_window_by_client_name(const char* client_name, const char* message) {
	uint8 update_type = CZUpdateType_ClientName;
	int update_identifier = 0;
	quest_manager.CrossZoneDialogueWindow(update_type, update_identifier, message, client_name);
}

void lua_cross_zone_disable_task_by_char_id(int character_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZTaskUpdateSubtype_DisableTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, character_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_disable_task_by_group_id(int group_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZTaskUpdateSubtype_DisableTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, group_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_disable_task_by_raid_id(int raid_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZTaskUpdateSubtype_DisableTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, raid_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_disable_task_by_guild_id(int guild_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZTaskUpdateSubtype_DisableTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, guild_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_disable_task_by_expedition_id(uint32 expedition_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZTaskUpdateSubtype_DisableTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, expedition_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_disable_task_by_client_name(const char* client_name, uint32 task_id) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZTaskUpdateSubtype_DisableTask;
	int update_identifier = 0;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, update_identifier, task_id, task_subidentifier, update_count, enforce_level_requirement, client_name);
}

void lua_cross_zone_enable_task_by_char_id(int character_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZTaskUpdateSubtype_EnableTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, character_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_enable_task_by_group_id(int group_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZTaskUpdateSubtype_EnableTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, group_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_enable_task_by_raid_id(int raid_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZTaskUpdateSubtype_EnableTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, raid_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_enable_task_by_guild_id(int guild_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZTaskUpdateSubtype_EnableTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, guild_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_enable_task_by_expedition_id(uint32 expedition_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZTaskUpdateSubtype_EnableTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, expedition_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_enable_task_by_client_name(const char* client_name, uint32 task_id) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZTaskUpdateSubtype_EnableTask;
	int update_identifier = 0;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, update_identifier, task_id, task_subidentifier, update_count, enforce_level_requirement, client_name);
}

void lua_cross_zone_fail_task_by_char_id(int character_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZTaskUpdateSubtype_FailTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, character_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_fail_task_by_group_id(int group_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZTaskUpdateSubtype_FailTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, group_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_fail_task_by_raid_id(int raid_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZTaskUpdateSubtype_FailTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, raid_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_fail_task_by_guild_id(int guild_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZTaskUpdateSubtype_FailTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, guild_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_fail_task_by_expedition_id(uint32 expedition_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZTaskUpdateSubtype_FailTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, expedition_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_fail_task_by_client_name(const char* client_name, uint32 task_id) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZTaskUpdateSubtype_FailTask;
	int update_identifier = 0;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, update_identifier, task_id, task_subidentifier, update_count, enforce_level_requirement, client_name);
}

void lua_cross_zone_marquee_by_char_id(int character_id, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message) {
	uint8 update_type = CZUpdateType_Character;
	quest_manager.CrossZoneMarquee(update_type, character_id, type, priority, fade_in, fade_out, duration, message);
}

void lua_cross_zone_marquee_by_group_id(int group_id, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message) {
	uint8 update_type = CZUpdateType_Group;
	quest_manager.CrossZoneMarquee(update_type, group_id, type, priority, fade_in, fade_out, duration, message);
}

void lua_cross_zone_marquee_by_raid_id(int raid_id, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message) {
	uint8 update_type = CZUpdateType_Raid;
	quest_manager.CrossZoneMarquee(update_type, raid_id, type, priority, fade_in, fade_out, duration, message);
}

void lua_cross_zone_marquee_by_guild_id(int guild_id, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message) {
	uint8 update_type = CZUpdateType_Guild;
	quest_manager.CrossZoneMarquee(update_type, guild_id, type, priority, fade_in, fade_out, duration, message);
}

void lua_cross_zone_marquee_by_expedition_id(uint32 expedition_id, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message) {
	uint8 update_type = CZUpdateType_Expedition;
	quest_manager.CrossZoneMarquee(update_type, expedition_id, type, priority, fade_in, fade_out, duration, message);
}

void lua_cross_zone_marquee_by_client_name(const char* client_name, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message) {
	uint8 update_type = CZUpdateType_ClientName;
	int update_identifier = 0;
	quest_manager.CrossZoneMarquee(update_type, update_identifier, type, priority, fade_in, fade_out, duration, message, client_name);
}

void lua_cross_zone_message_player_by_char_id(uint32 type, int character_id, const char* message) {
	uint8 update_type = CZUpdateType_Character;
	quest_manager.CrossZoneMessage(update_type, character_id, type, message);
}

void lua_cross_zone_message_player_by_group_id(uint32 type, int group_id, const char* message) {
	uint8 update_type = CZUpdateType_Group;
	quest_manager.CrossZoneMessage(update_type, group_id, type, message);
}

void lua_cross_zone_message_player_by_raid_id(uint32 type, int raid_id, const char* message) {
	uint8 update_type = CZUpdateType_Raid;
	quest_manager.CrossZoneMessage(update_type, raid_id, type, message);
}

void lua_cross_zone_message_player_by_guild_id(uint32 type, int guild_id, const char* message) {
	uint8 update_type = CZUpdateType_Guild;
	quest_manager.CrossZoneMessage(update_type, guild_id, type, message);
}

void lua_cross_zone_message_player_by_expedition_id(uint32 type, int expedition_id, const char* message) {
	uint8 update_type = CZUpdateType_Expedition;
	quest_manager.CrossZoneMessage(update_type, expedition_id, type, message);
}

void lua_cross_zone_message_player_by_name(uint32 type, const char* client_name, const char* message) {
	uint8 update_type = CZUpdateType_ClientName;
	int update_identifier = 0;
	quest_manager.CrossZoneMessage(update_type, update_identifier, type, message, client_name);
}

void lua_cross_zone_move_player_by_char_id(uint32 character_id, std::string zone_short_name) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.update_identifier = character_id,
			.update_type = CZUpdateType_Character,
			.update_subtype = CZMoveUpdateSubtype_MoveZone,
			.zone_short_name = zone_short_name,
		}
	);
}

void lua_cross_zone_move_player_by_char_id(uint32 character_id, std::string zone_short_name, float x, float y, float z) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.coordinates = glm::vec4(x, y, z, 0.0f),
			.update_identifier = character_id,
			.update_type = CZUpdateType_Character,
			.update_subtype = CZMoveUpdateSubtype_MoveZone,
			.zone_short_name = zone_short_name,
		}
	);
}

void lua_cross_zone_move_player_by_char_id(uint32 character_id, std::string zone_short_name, float x, float y, float z, float heading) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.coordinates = glm::vec4(x, y, z, heading),
			.update_identifier = character_id,
			.update_type = CZUpdateType_Character,
			.update_subtype = CZMoveUpdateSubtype_MoveZone,
			.zone_short_name = zone_short_name,
		}
	);
}

void lua_cross_zone_move_player_by_group_id(uint32 group_id, std::string zone_short_name) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.update_identifier = group_id,
			.update_type = CZUpdateType_Group,
			.update_subtype = CZMoveUpdateSubtype_MoveZone,
			.zone_short_name = zone_short_name,
		}
	);
}

void lua_cross_zone_move_player_by_group_id(uint32 group_id, std::string zone_short_name, float x, float y, float z) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.coordinates = glm::vec4(x, y, z, 0.0f),
			.update_identifier = group_id,
			.update_type = CZUpdateType_Group,
			.update_subtype = CZMoveUpdateSubtype_MoveZone,
			.zone_short_name = zone_short_name,
		}
	);
}

void lua_cross_zone_move_player_by_group_id(uint32 group_id, std::string zone_short_name, float x, float y, float z, float heading) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.coordinates = glm::vec4(x, y, z, heading),
			.update_identifier = group_id,
			.update_type = CZUpdateType_Group,
			.update_subtype = CZMoveUpdateSubtype_MoveZone,
			.zone_short_name = zone_short_name,
		}
	);
}

void lua_cross_zone_move_player_by_raid_id(uint32 raid_id, std::string zone_short_name) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.update_identifier = raid_id,
			.update_type = CZUpdateType_Raid,
			.update_subtype = CZMoveUpdateSubtype_MoveZone,
			.zone_short_name = zone_short_name,
		}
	);
}

void lua_cross_zone_move_player_by_raid_id(uint32 raid_id, std::string zone_short_name, float x, float y, float z) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.coordinates = glm::vec4(x, y, z, 0.0f),
			.update_identifier = raid_id,
			.update_type = CZUpdateType_Raid,
			.update_subtype = CZMoveUpdateSubtype_MoveZone,
			.zone_short_name = zone_short_name,
		}
	);
}

void lua_cross_zone_move_player_by_raid_id(uint32 raid_id, std::string zone_short_name, float x, float y, float z, float heading) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.coordinates = glm::vec4(x, y, z, heading),
			.update_identifier = raid_id,
			.update_type = CZUpdateType_Raid,
			.update_subtype = CZMoveUpdateSubtype_MoveZone,
			.zone_short_name = zone_short_name,
		}
	);
}

void lua_cross_zone_move_player_by_guild_id(uint32 guild_id, std::string zone_short_name) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.update_identifier = guild_id,
			.update_type = CZUpdateType_Guild,
			.update_subtype = CZMoveUpdateSubtype_MoveZone,
			.zone_short_name = zone_short_name,
		}
	);
}

void lua_cross_zone_move_player_by_guild_id(uint32 guild_id, std::string zone_short_name, float x, float y, float z) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.coordinates = glm::vec4(x, y, z, 0.0f),
			.update_identifier = guild_id,
			.update_type = CZUpdateType_Guild,
			.update_subtype = CZMoveUpdateSubtype_MoveZone,
			.zone_short_name = zone_short_name,
		}
	);
}

void lua_cross_zone_move_player_by_guild_id(uint32 guild_id, std::string zone_short_name, float x, float y, float z, float heading) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.coordinates = glm::vec4(x, y, z, heading),
			.update_identifier = guild_id,
			.update_type = CZUpdateType_Guild,
			.update_subtype = CZMoveUpdateSubtype_MoveZone,
			.zone_short_name = zone_short_name,
		}
	);
}

void lua_cross_zone_move_player_by_expedition_id(uint32 expedition_id, std::string zone_short_name) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.update_identifier = expedition_id,
			.update_type = CZUpdateType_Expedition,
			.update_subtype = CZMoveUpdateSubtype_MoveZone,
			.zone_short_name = zone_short_name,
		}
	);
}

void lua_cross_zone_move_player_by_expedition_id(uint32 expedition_id, std::string zone_short_name, float x, float y, float z) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.coordinates = glm::vec4(x, y, z, 0.0f),
			.update_identifier = expedition_id,
			.update_type = CZUpdateType_Expedition,
			.update_subtype = CZMoveUpdateSubtype_MoveZone,
			.zone_short_name = zone_short_name,
		}
	);
}

void lua_cross_zone_move_player_by_expedition_id(uint32 expedition_id, std::string zone_short_name, float x, float y, float z, float heading) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.coordinates = glm::vec4(x, y, z, heading),
			.update_identifier = expedition_id,
			.update_type = CZUpdateType_Expedition,
			.update_subtype = CZMoveUpdateSubtype_MoveZone,
			.zone_short_name = zone_short_name,
		}
	);
}

void lua_cross_zone_move_player_by_client_name(std::string client_name, std::string zone_short_name) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.client_name = client_name,
			.update_type = CZUpdateType_ClientName,
			.update_subtype = CZMoveUpdateSubtype_MoveZone,
			.zone_short_name = zone_short_name,
		}
	);
}

void lua_cross_zone_move_player_by_client_name(std::string client_name, std::string zone_short_name, float x, float y, float z) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.client_name = client_name,
			.coordinates = glm::vec4(x, y, z, 0.0f),
			.update_type = CZUpdateType_ClientName,
			.update_subtype = CZMoveUpdateSubtype_MoveZone,
			.zone_short_name = zone_short_name,
		}
	);
}

void lua_cross_zone_move_player_by_client_name(std::string client_name, std::string zone_short_name, float x, float y, float z, float heading) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.client_name = client_name,
			.coordinates = glm::vec4(x, y, z, heading),
			.update_type = CZUpdateType_ClientName,
			.update_subtype = CZMoveUpdateSubtype_MoveZone,
			.zone_short_name = zone_short_name,
		}
	);
}

void lua_cross_zone_move_instance_by_char_id(uint32 character_id, uint16 instance_id) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.instance_id = instance_id,
			.update_identifier = character_id,
			.update_type = CZUpdateType_Character,
			.update_subtype = CZMoveUpdateSubtype_MoveZoneInstance,
		}
	);
}

void lua_cross_zone_move_instance_by_char_id(uint32 character_id, uint16 instance_id, float x, float y, float z) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.coordinates = glm::vec4(x, y, z, 0.0f),
			.instance_id = instance_id,
			.update_identifier = character_id,
			.update_type = CZUpdateType_Character,
			.update_subtype = CZMoveUpdateSubtype_MoveZoneInstance,
		}
	);
}

void lua_cross_zone_move_instance_by_char_id(uint32 character_id, uint16 instance_id, float x, float y, float z, float heading) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.coordinates = glm::vec4(x, y, z, heading),
			.instance_id = instance_id,
			.update_identifier = character_id,
			.update_type = CZUpdateType_Character,
			.update_subtype = CZMoveUpdateSubtype_MoveZoneInstance,
		}
	);
}

void lua_cross_zone_move_instance_by_group_id(uint32 group_id, uint16 instance_id) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.instance_id = instance_id,
			.update_identifier = group_id,
			.update_type = CZUpdateType_Group,
			.update_subtype = CZMoveUpdateSubtype_MoveZoneInstance,
		}
	);
}

void lua_cross_zone_move_instance_by_group_id(uint32 group_id, uint16 instance_id, float x, float y, float z) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.coordinates = glm::vec4(x, y, z, 0.0f),
			.instance_id = instance_id,
			.update_identifier = group_id,
			.update_type = CZUpdateType_Group,
			.update_subtype = CZMoveUpdateSubtype_MoveZoneInstance,
		}
	);
}

void lua_cross_zone_move_instance_by_group_id(uint32 group_id, uint16 instance_id, float x, float y, float z, float heading) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.coordinates = glm::vec4(x, y, z, heading),
			.instance_id = instance_id,
			.update_identifier = group_id,
			.update_type = CZUpdateType_Group,
			.update_subtype = CZMoveUpdateSubtype_MoveZoneInstance,
		}
	);
}

void lua_cross_zone_move_instance_by_raid_id(uint32 raid_id, uint16 instance_id) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.instance_id = instance_id,
			.update_identifier = raid_id,
			.update_type = CZUpdateType_Raid,
			.update_subtype = CZMoveUpdateSubtype_MoveZoneInstance,
		}
	);
}

void lua_cross_zone_move_instance_by_raid_id(uint32 raid_id, uint16 instance_id, float x, float y, float z) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.coordinates = glm::vec4(x, y, z, 0.0f),
			.instance_id = instance_id,
			.update_identifier = raid_id,
			.update_type = CZUpdateType_Raid,
			.update_subtype = CZMoveUpdateSubtype_MoveZoneInstance,
		}
	);
}

void lua_cross_zone_move_instance_by_raid_id(uint32 raid_id, uint16 instance_id, float x, float y, float z, float heading) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.coordinates = glm::vec4(x, y, z, heading),
			.instance_id = instance_id,
			.update_identifier = raid_id,
			.update_type = CZUpdateType_Raid,
			.update_subtype = CZMoveUpdateSubtype_MoveZoneInstance,
		}
	);
}

void lua_cross_zone_move_instance_by_guild_id(uint32 guild_id, uint16 instance_id) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.instance_id = instance_id,
			.update_identifier = guild_id,
			.update_type = CZUpdateType_Guild,
			.update_subtype = CZMoveUpdateSubtype_MoveZoneInstance,
		}
	);
}

void lua_cross_zone_move_instance_by_guild_id(uint32 guild_id, uint16 instance_id, float x, float y, float z) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.coordinates = glm::vec4(x, y, z, 0.0f),
			.instance_id = instance_id,
			.update_identifier = guild_id,
			.update_type = CZUpdateType_Guild,
			.update_subtype = CZMoveUpdateSubtype_MoveZoneInstance,
		}
	);
}

void lua_cross_zone_move_instance_by_guild_id(uint32 guild_id, uint16 instance_id, float x, float y, float z, float heading) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.coordinates = glm::vec4(x, y, z, heading),
			.instance_id = instance_id,
			.update_identifier = guild_id,
			.update_type = CZUpdateType_Guild,
			.update_subtype = CZMoveUpdateSubtype_MoveZoneInstance,
		}
	);
}

void lua_cross_zone_move_instance_by_expedition_id(uint32 expedition_id, uint16 instance_id) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.instance_id = instance_id,
			.update_identifier = expedition_id,
			.update_type = CZUpdateType_Expedition,
			.update_subtype = CZMoveUpdateSubtype_MoveZoneInstance,
		}
	);
}

void lua_cross_zone_move_instance_by_expedition_id(uint32 expedition_id, uint16 instance_id, float x, float y, float z) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.coordinates = glm::vec4(x, y, z, 0.0f),
			.instance_id = instance_id,
			.update_identifier = expedition_id,
			.update_type = CZUpdateType_Expedition,
			.update_subtype = CZMoveUpdateSubtype_MoveZoneInstance,
		}
	);
}

void lua_cross_zone_move_instance_by_expedition_id(uint32 expedition_id, uint16 instance_id, float x, float y, float z, float heading) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.coordinates = glm::vec4(x, y, z, heading),
			.instance_id = instance_id,
			.update_identifier = expedition_id,
			.update_type = CZUpdateType_Expedition,
			.update_subtype = CZMoveUpdateSubtype_MoveZoneInstance,
		}
	);
}

void lua_cross_zone_move_instance_by_client_name(std::string client_name, uint16 instance_id) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.client_name = client_name,
			.instance_id = instance_id,
			.update_type = CZUpdateType_ClientName,
			.update_subtype = CZMoveUpdateSubtype_MoveZoneInstance,
		}
	);
}

void lua_cross_zone_move_instance_by_client_name(std::string client_name, uint16 instance_id, float x, float y, float z) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.client_name = client_name,
			.coordinates = glm::vec4(x, y, z, 0.0f),
			.instance_id = instance_id,
			.update_type = CZUpdateType_ClientName,
			.update_subtype = CZMoveUpdateSubtype_MoveZoneInstance,
		}
	);
}

void lua_cross_zone_move_instance_by_client_name(std::string client_name, uint16 instance_id, float x, float y, float z, float heading) {
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.client_name = client_name,
			.coordinates = glm::vec4(x, y, z, heading),
			.instance_id = instance_id,
			.update_type = CZUpdateType_ClientName,
			.update_subtype = CZMoveUpdateSubtype_MoveZoneInstance,
		}
	);
}

void lua_cross_zone_remove_ldon_loss_by_char_id(int character_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZLDoNUpdateSubtype_RemoveLoss;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, character_id, theme_id);
}

void lua_cross_zone_remove_ldon_loss_by_group_id(int group_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZLDoNUpdateSubtype_RemoveLoss;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, group_id, theme_id);
}

void lua_cross_zone_remove_ldon_loss_by_raid_id(int raid_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZLDoNUpdateSubtype_RemoveLoss;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, raid_id, theme_id);
}

void lua_cross_zone_remove_ldon_loss_by_guild_id(int guild_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZLDoNUpdateSubtype_RemoveLoss;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, guild_id, theme_id);
}

void lua_cross_zone_remove_ldon_loss_by_expedition_id(uint32 expedition_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZLDoNUpdateSubtype_RemoveLoss;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, expedition_id, theme_id);
}

void lua_cross_zone_remove_ldon_loss_by_client_name(const char* client_name, uint32 theme_id) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZLDoNUpdateSubtype_RemoveLoss;
	int update_identifier = 0;
	int points = 1;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, update_identifier, theme_id, points, client_name);
}

void lua_cross_zone_remove_ldon_win_by_char_id(int character_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZLDoNUpdateSubtype_RemoveWin;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, character_id, theme_id);
}

void lua_cross_zone_remove_ldon_win_by_group_id(int group_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZLDoNUpdateSubtype_RemoveWin;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, group_id, theme_id);
}

void lua_cross_zone_remove_ldon_win_by_raid_id(int raid_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZLDoNUpdateSubtype_RemoveWin;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, raid_id, theme_id);
}

void lua_cross_zone_remove_ldon_win_by_guild_id(int guild_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZLDoNUpdateSubtype_RemoveWin;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, guild_id, theme_id);
}

void lua_cross_zone_remove_ldon_win_by_expedition_id(uint32 expedition_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZLDoNUpdateSubtype_RemoveWin;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, expedition_id, theme_id);
}

void lua_cross_zone_remove_ldon_win_by_client_name(const char* client_name, uint32 theme_id) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZLDoNUpdateSubtype_RemoveWin;
	int update_identifier = 0;
	int points = 1;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, update_identifier, theme_id, points, client_name);
}

void lua_cross_zone_remove_spell_by_char_id(int character_id, uint32 spell_id) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZSpellUpdateSubtype_Remove;
	quest_manager.CrossZoneSpell(update_type, update_subtype, character_id, spell_id);
}

void lua_cross_zone_remove_spell_by_group_id(int group_id, uint32 spell_id) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZSpellUpdateSubtype_Remove;
	quest_manager.CrossZoneSpell(update_type, update_subtype, group_id, spell_id);
}

void lua_cross_zone_remove_spell_by_raid_id(int raid_id, uint32 spell_id) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZSpellUpdateSubtype_Remove;
	quest_manager.CrossZoneSpell(update_type, update_subtype, raid_id, spell_id);
}

void lua_cross_zone_remove_spell_by_guild_id(int guild_id, uint32 spell_id) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZSpellUpdateSubtype_Remove;
	quest_manager.CrossZoneSpell(update_type, update_subtype, guild_id, spell_id);
}

void lua_cross_zone_remove_spell_by_expedition_id(uint32 expedition_id, uint32 spell_id) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZSpellUpdateSubtype_Remove;
	quest_manager.CrossZoneSpell(update_type, update_subtype, expedition_id, spell_id);
}

void lua_cross_zone_remove_spell_by_client_name(const char* client_name, uint32 spell_id) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZSpellUpdateSubtype_Remove;
	int update_identifier = 0;
	quest_manager.CrossZoneSpell(update_type, update_subtype, update_identifier, spell_id, client_name);
}

void lua_cross_zone_remove_task_by_char_id(int character_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZTaskUpdateSubtype_RemoveTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, character_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_remove_task_by_group_id(int group_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZTaskUpdateSubtype_RemoveTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, group_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_remove_task_by_raid_id(int raid_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZTaskUpdateSubtype_RemoveTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, raid_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_remove_task_by_guild_id(int guild_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZTaskUpdateSubtype_RemoveTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, guild_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_remove_task_by_expedition_id(uint32 expedition_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZTaskUpdateSubtype_RemoveTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, expedition_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_remove_task_by_client_name(const char* client_name, uint32 task_id) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZTaskUpdateSubtype_RemoveTask;
	int update_identifier = 0;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, update_identifier, task_id, task_subidentifier, update_count, enforce_level_requirement, client_name);
}

void lua_cross_zone_reset_activity_by_char_id(int character_id, uint32 task_id, int activity_id) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityReset;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, character_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void lua_cross_zone_reset_activity_by_group_id(int group_id, uint32 task_id, int activity_id) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityReset;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, group_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void lua_cross_zone_reset_activity_by_raid_id(int raid_id, uint32 task_id, int activity_id) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityReset;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, raid_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void lua_cross_zone_reset_activity_by_guild_id(int guild_id, uint32 task_id, int activity_id) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityReset;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, guild_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void lua_cross_zone_reset_activity_by_expedition_id(uint32 expedition_id, uint32 task_id, int activity_id) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityReset;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, expedition_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void lua_cross_zone_reset_activity_by_client_name(const char* client_name, uint32 task_id, int activity_id) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityReset;
	int update_identifier = 0;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, update_identifier, task_id, activity_id, update_count, enforce_level_requirement, client_name);
}

void lua_cross_zone_set_entity_variable_by_char_id(int character_id, const char* variable_name, const char* variable_value) {
	uint8 update_type = CZUpdateType_Character;
	quest_manager.CrossZoneSetEntityVariable(update_type, character_id, variable_name, variable_value);
}

void lua_cross_zone_set_entity_variable_by_group_id(int group_id, const char* variable_name, const char* variable_value) {
	uint8 update_type = CZUpdateType_Group;
	quest_manager.CrossZoneSetEntityVariable(update_type, group_id, variable_name, variable_value);
}

void lua_cross_zone_set_entity_variable_by_raid_id(int raid_id, const char* variable_name, const char* variable_value) {
	uint8 update_type = CZUpdateType_Raid;
	quest_manager.CrossZoneSetEntityVariable(update_type, raid_id, variable_name, variable_value);
}

void lua_cross_zone_set_entity_variable_by_guild_id(int guild_id, const char* variable_name, const char* variable_value) {
	uint8 update_type = CZUpdateType_Guild;
	quest_manager.CrossZoneSetEntityVariable(update_type, guild_id, variable_name, variable_value);
}

void lua_cross_zone_set_entity_variable_by_expedition_id(uint32 expedition_id, const char* variable_name, const char* variable_value) {
	uint8 update_type = CZUpdateType_Expedition;
	quest_manager.CrossZoneSetEntityVariable(update_type, expedition_id, variable_name, variable_value);
}

void lua_cross_zone_set_entity_variable_by_client_name(const char* character_name, const char* variable_name, const char* variable_value) {
	uint8 update_type = CZUpdateType_ClientName;
	int update_identifier = 0;
	quest_manager.CrossZoneSetEntityVariable(update_type, update_identifier, variable_name, variable_value, character_name);
}

void lua_cross_zone_signal_client_by_char_id(uint32 character_id, int signal_id) {
	uint8 update_type = CZUpdateType_Character;
	quest_manager.CrossZoneSignal(update_type, character_id, signal_id);
}

void lua_cross_zone_signal_client_by_group_id(uint32 group_id, int signal_id) {
	uint8 update_type = CZUpdateType_Group;
	quest_manager.CrossZoneSignal(update_type, group_id, signal_id);
}

void lua_cross_zone_signal_client_by_raid_id(uint32 raid_id, int signal_id) {
	uint8 update_type = CZUpdateType_Raid;
	quest_manager.CrossZoneSignal(update_type, raid_id, signal_id);
}

void lua_cross_zone_signal_client_by_guild_id(uint32 guild_id, int signal_id) {
	uint8 update_type = CZUpdateType_Guild;
	quest_manager.CrossZoneSignal(update_type, guild_id, signal_id);
}

void lua_cross_zone_signal_client_by_expedition_id(uint32 expedition_id, int signal_id) {
	uint8 update_type = CZUpdateType_Expedition;
	quest_manager.CrossZoneSignal(update_type, expedition_id, signal_id);
}

void lua_cross_zone_signal_client_by_name(const char* client_name, int signal_id) {
	uint8 update_type = CZUpdateType_ClientName;
	int update_identifier = 0;
	quest_manager.CrossZoneSignal(update_type, update_identifier, signal_id, client_name);
}

void lua_cross_zone_signal_npc_by_npctype_id(uint32 npctype_id, int signal_id) {
	uint8 update_type = CZUpdateType_NPC;
	quest_manager.CrossZoneSignal(update_type, npctype_id, signal_id);
}

void lua_cross_zone_update_activity_by_char_id(int character_id, uint32 task_id, int activity_id) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityUpdate;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, character_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void lua_cross_zone_update_activity_by_char_id(int character_id, uint32 task_id, int activity_id, int activity_count) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityUpdate;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, character_id, task_id, activity_id, activity_count, enforce_level_requirement);
}

void lua_cross_zone_update_activity_by_group_id(int group_id, uint32 task_id, int activity_id) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityUpdate;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, group_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void lua_cross_zone_update_activity_by_group_id(int group_id, uint32 task_id, int activity_id, int activity_count) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityUpdate;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, group_id, task_id, activity_id, activity_count, enforce_level_requirement);
}

void lua_cross_zone_update_activity_by_raid_id(int raid_id, uint32 task_id, int activity_id) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityUpdate;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, raid_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void lua_cross_zone_update_activity_by_raid_id(int raid_id, uint32 task_id, int activity_id, int activity_count) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityUpdate;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, raid_id, task_id, activity_id, activity_count, enforce_level_requirement);
}

void lua_cross_zone_update_activity_by_guild_id(int guild_id, uint32 task_id, int activity_id) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityUpdate;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, guild_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void lua_cross_zone_update_activity_by_guild_id(int guild_id, uint32 task_id, int activity_id, int activity_count) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityUpdate;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, guild_id, task_id, activity_id, activity_count, enforce_level_requirement);
}

void lua_cross_zone_update_activity_by_expedition_id(uint32 expedition_id, uint32 task_id, int activity_id) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityUpdate;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, expedition_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void lua_cross_zone_update_activity_by_expedition_id(uint32 expedition_id, uint32 task_id, int activity_id, int activity_count) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityUpdate;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, expedition_id, task_id, activity_id, activity_count, enforce_level_requirement);
}

void lua_cross_zone_update_activity_by_client_name(const char* client_name, uint32 task_id, int activity_id) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityUpdate;
	int update_identifier = 0;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, update_identifier, task_id, activity_id, update_count, enforce_level_requirement, client_name);
}

void lua_cross_zone_update_activity_by_client_name(const char* client_name, uint32 task_id, int activity_id, int activity_count) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityUpdate;
	int update_identifier = 0;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, update_identifier, task_id, activity_id, activity_count, enforce_level_requirement, client_name);
}

void lua_world_wide_add_ldon_loss(uint32 theme_id) {
	uint8 update_type = WWLDoNUpdateType_AddLoss;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id);
}

void lua_world_wide_add_ldon_loss(uint32 theme_id, uint8 min_status) {
	uint8 update_type = WWLDoNUpdateType_AddLoss;
	int points = 1;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id, points, min_status);
}

void lua_world_wide_add_ldon_loss(uint32 theme_id, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWLDoNUpdateType_AddLoss;
	int points = 1;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id, points, min_status, max_status);
}

void lua_world_wide_add_ldon_points(uint32 theme_id, int points) {
	uint8 update_type = WWLDoNUpdateType_AddPoints;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id, points);
}

void lua_world_wide_add_ldon_points(uint32 theme_id, int points, uint8 min_status) {
	uint8 update_type = WWLDoNUpdateType_AddPoints;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id, points, min_status);
}

void lua_world_wide_add_ldon_points(uint32 theme_id, int points, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWLDoNUpdateType_AddPoints;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id, points, min_status, max_status);
}

void lua_world_wide_add_ldon_win(uint32 theme_id) {
	uint8 update_type = WWLDoNUpdateType_AddWin;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id);
}

void lua_world_wide_add_ldon_win(uint32 theme_id, uint8 min_status) {
	uint8 update_type = WWLDoNUpdateType_AddWin;
	int points = 1;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id, points, min_status);
}

void lua_world_wide_add_ldon_win(uint32 theme_id, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWLDoNUpdateType_AddWin;
	int points = 1;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id, points, min_status, max_status);
}

void lua_world_wide_assign_task(uint32 task_id) {
	uint8 update_type = WWTaskUpdateType_AssignTask;
	quest_manager.WorldWideTaskUpdate(update_type, task_id);
}

void lua_world_wide_assign_task(uint32 task_id, bool enforce_level_requirement) {
	uint8 update_type = WWTaskUpdateType_AssignTask;
	int task_subidentifier = -1;
	int update_count = 1;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_world_wide_assign_task(uint32 task_id, bool enforce_level_requirement, uint8 min_status) {
	uint8 update_type = WWTaskUpdateType_AssignTask;
	int task_subidentifier = -1;
	int update_count = 1;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status);
}

void lua_world_wide_assign_task(uint32 task_id, bool enforce_level_requirement, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWTaskUpdateType_AssignTask;
	int task_subidentifier = -1;
	int update_count = 1;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status, max_status);
}

void lua_world_wide_cast_spell(uint32 spell_id) {
	uint8 update_type = WWSpellUpdateType_Cast;
	quest_manager.WorldWideSpell(update_type, spell_id);
}

void lua_world_wide_cast_spell(uint32 spell_id, uint8 min_status) {
	uint8 update_type = WWSpellUpdateType_Cast;
	quest_manager.WorldWideSpell(update_type, spell_id, min_status);
}

void lua_world_wide_cast_spell(uint32 spell_id, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWSpellUpdateType_Cast;
	quest_manager.WorldWideSpell(update_type, spell_id, min_status, max_status);
}

void lua_world_wide_dialogue_window(const char* message) {
	quest_manager.WorldWideDialogueWindow(message);
}

void lua_world_wide_dialogue_window(const char* message, uint8 min_status) {
	quest_manager.WorldWideDialogueWindow(message, min_status);
}

void lua_world_wide_dialogue_window(const char* message, uint8 min_status, uint8 max_status) {
	quest_manager.WorldWideDialogueWindow(message, min_status, max_status);
}

void lua_world_wide_disable_task(uint32 task_id) {
	uint8 update_type = WWTaskUpdateType_DisableTask;
	quest_manager.WorldWideTaskUpdate(update_type, task_id);
}

void lua_world_wide_disable_task(uint32 task_id, uint8 min_status) {
	uint8 update_type = WWTaskUpdateType_DisableTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status);
}

void lua_world_wide_disable_task(uint32 task_id, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWTaskUpdateType_DisableTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status, max_status);
}

void lua_world_wide_enable_task(uint32 task_id) {
	uint8 update_type = WWTaskUpdateType_EnableTask;
	quest_manager.WorldWideTaskUpdate(update_type, task_id);
}

void lua_world_wide_enable_task(uint32 task_id, uint8 min_status) {
	uint8 update_type = WWTaskUpdateType_EnableTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status);
}

void lua_world_wide_enable_task(uint32 task_id, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWTaskUpdateType_EnableTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status, max_status);
}

void lua_world_wide_fail_task(uint32 task_id) {
	uint8 update_type = WWTaskUpdateType_FailTask;
	quest_manager.WorldWideTaskUpdate(update_type, task_id);
}

void lua_world_wide_fail_task(uint32 task_id, uint8 min_status) {
	uint8 update_type = WWTaskUpdateType_FailTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status);
}

void lua_world_wide_fail_task(uint32 task_id, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWTaskUpdateType_FailTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status, max_status);
}

void lua_world_wide_marquee(uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message) {
	quest_manager.WorldWideMarquee(type, priority, fade_in, fade_out, duration, message);
}

void lua_world_wide_marquee(uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message, uint8 min_status) {
	quest_manager.WorldWideMarquee(type, priority, fade_in, fade_out, duration, message, min_status);
}

void lua_world_wide_marquee(uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message, uint8 min_status, uint8 max_status) {
	quest_manager.WorldWideMarquee(type, priority, fade_in, fade_out, duration, message, min_status, max_status);
}

void lua_world_wide_message(uint32 type, const char* message) {
	quest_manager.WorldWideMessage(type, message);
}

void lua_world_wide_message(uint32 type, const char* message, uint8 min_status) {
	quest_manager.WorldWideMessage(type, message, min_status);
}

void lua_world_wide_message(uint32 type, const char* message, uint8 min_status, uint8 max_status) {
	quest_manager.WorldWideMessage(type, message, min_status, max_status);
}

void lua_world_wide_move(const char* zone_short_name) {
	uint8 update_type = WWMoveUpdateType_MoveZone;
	quest_manager.WorldWideMove(update_type, zone_short_name);
}

void lua_world_wide_move(const char* zone_short_name, uint8 min_status) {
	uint8 update_type = WWMoveUpdateType_MoveZone;
	uint16 instance_id = 0;
	quest_manager.WorldWideMove(update_type, zone_short_name, instance_id, min_status);
}

void lua_world_wide_move(const char* zone_short_name, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWMoveUpdateType_MoveZone;
	uint16 instance_id = 0;
	quest_manager.WorldWideMove(update_type, zone_short_name, instance_id, min_status, max_status);
}

void lua_world_wide_move_instance(uint16 instance_id) {
	uint8 update_type = WWMoveUpdateType_MoveZoneInstance;
	const char* zone_short_name = "";
	quest_manager.WorldWideMove(update_type, zone_short_name, instance_id);
}

void lua_world_wide_move_instance(uint16 instance_id, uint8 min_status) {
	uint8 update_type = WWMoveUpdateType_MoveZoneInstance;
	const char* zone_short_name = "";
	quest_manager.WorldWideMove(update_type, zone_short_name, instance_id, min_status);
}

void lua_world_wide_move_instance(uint16 instance_id, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWMoveUpdateType_MoveZoneInstance;
	const char* zone_short_name = "";
	quest_manager.WorldWideMove(update_type, zone_short_name, instance_id, min_status, max_status);
}

void lua_world_wide_remove_ldon_loss(uint32 theme_id) {
	uint8 update_type = WWLDoNUpdateType_RemoveLoss;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id);
}

void lua_world_wide_remove_ldon_loss(uint32 theme_id, uint8 min_status) {
	uint8 update_type = WWLDoNUpdateType_RemoveLoss;
	int points = 1;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id, points, min_status);
}

void lua_world_wide_remove_ldon_loss(uint32 theme_id, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWLDoNUpdateType_RemoveLoss;
	int points = 1;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id, points, min_status, max_status);
}

void lua_world_wide_remove_ldon_win(uint32 theme_id) {
	uint8 update_type = WWLDoNUpdateType_RemoveWin;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id);
}

void lua_world_wide_remove_ldon_win(uint32 theme_id, uint8 min_status) {
	uint8 update_type = WWLDoNUpdateType_RemoveWin;
	int points = 1;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id, points, min_status);
}

void lua_world_wide_remove_ldon_win(uint32 theme_id, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWLDoNUpdateType_RemoveWin;
	int points = 1;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id, points, min_status, max_status);
}

void lua_world_wide_remove_spell(uint32 spell_id) {
	uint8 update_type = WWSpellUpdateType_Remove;
	quest_manager.WorldWideSpell(update_type, spell_id);
}

void lua_world_wide_remove_spell(uint32 spell_id, uint8 min_status) {
	uint8 update_type = WWSpellUpdateType_Remove;
	quest_manager.WorldWideSpell(update_type, spell_id, min_status);
}

void lua_world_wide_remove_spell(uint32 spell_id, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWSpellUpdateType_Remove;
	quest_manager.WorldWideSpell(update_type, spell_id, min_status, max_status);
}

void lua_world_wide_remove_task(uint32 task_id) {
	uint8 update_type = WWTaskUpdateType_RemoveTask;
	quest_manager.WorldWideTaskUpdate(update_type, task_id);
}

void lua_world_wide_remove_task(uint32 task_id, uint8 min_status) {
	uint8 update_type = WWTaskUpdateType_RemoveTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status);
}

void lua_world_wide_remove_task(uint32 task_id, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWTaskUpdateType_RemoveTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status, max_status);
}

void lua_world_wide_reset_activity(uint32 task_id, int activity_id) {
	uint8 update_type = WWTaskUpdateType_ActivityReset;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, activity_id);
}

void lua_world_wide_reset_activity(uint32 task_id, int activity_id, uint8 min_status) {
	uint8 update_type = WWTaskUpdateType_ActivityReset;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, activity_id, update_count, enforce_level_requirement, min_status);
}

void lua_world_wide_reset_activity(uint32 task_id, int activity_id, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWTaskUpdateType_ActivityReset;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, activity_id, update_count, enforce_level_requirement, min_status, max_status);
}

void lua_world_wide_set_entity_variable_client(const char* variable_name, const char* variable_value) {
	uint8 update_type = WWSetEntityVariableUpdateType_Character;
	quest_manager.WorldWideSetEntityVariable(update_type, variable_name, variable_value);
}

void lua_world_wide_set_entity_variable_client(const char* variable_name, const char* variable_value, uint8 min_status) {
	uint8 update_type = WWSetEntityVariableUpdateType_Character;
	quest_manager.WorldWideSetEntityVariable(update_type, variable_name, variable_value, min_status);
}

void lua_world_wide_set_entity_variable_client(const char* variable_name, const char* variable_value, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWSetEntityVariableUpdateType_Character;
	quest_manager.WorldWideSetEntityVariable(update_type, variable_name, variable_value, min_status, max_status);
}

void lua_world_wide_set_entity_variable_npc(const char* variable_name, const char* variable_value) {
	uint8 update_type = WWSetEntityVariableUpdateType_NPC;
	quest_manager.WorldWideSetEntityVariable(update_type, variable_name, variable_value);
}

void lua_world_wide_signal_client(int signal_id) {
	uint8 update_type = WWSignalUpdateType_Character;
	quest_manager.WorldWideSignal(update_type, signal_id);
}

void lua_world_wide_signal_client(int signal_id, uint8 min_status) {
	uint8 update_type = WWSignalUpdateType_Character;
	quest_manager.WorldWideSignal(update_type, signal_id, min_status);
}

void lua_world_wide_signal_client(int signal_id, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWSignalUpdateType_Character;
	quest_manager.WorldWideSignal(update_type, signal_id, min_status, max_status);
}

void lua_world_wide_signal_npc(int signal_id) {
	uint8 update_type = WWSignalUpdateType_NPC;
	quest_manager.WorldWideSignal(update_type, signal_id);
}

void lua_world_wide_update_activity(uint32 task_id, int activity_id) {
	uint8 update_type = WWTaskUpdateType_ActivityUpdate;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, activity_id);
}

void lua_world_wide_update_activity(uint32 task_id, int activity_id, int activity_count) {
	uint8 update_type = WWTaskUpdateType_ActivityUpdate;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, activity_id, activity_count);
}

void lua_world_wide_update_activity(uint32 task_id, int activity_id, int activity_count, uint8 min_status) {
	uint8 update_type = WWTaskUpdateType_ActivityUpdate;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, activity_id, activity_count, enforce_level_requirement, min_status);
}

void lua_world_wide_update_activity(uint32 task_id, int activity_id, int activity_count, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWTaskUpdateType_ActivityUpdate;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, activity_id, activity_count, enforce_level_requirement, min_status, max_status);
}

bool lua_is_npc_spawned(luabind::adl::object table) {
	if(luabind::type(table) != LUA_TTABLE) {
		return false;
	}

	std::vector<uint32> npc_ids;
	int index = 1;
	while (luabind::type(table[index]) != LUA_TNIL) {
		auto current_id = table[index];
		uint32 npc_id = 0;
		if(luabind::type(current_id) != LUA_TNIL) {
			try {
				npc_id = luabind::object_cast<int>(current_id);
			} catch(luabind::cast_failed &) {
			}
		} else {
			break;
		}

		npc_ids.push_back(npc_id);
		++index;
	}

	if (npc_ids.empty()) {
		return false;
	}

	return entity_list.IsNPCSpawned(npc_ids);
}

uint32 lua_count_spawned_npcs(luabind::adl::object table) {
	if(luabind::type(table) != LUA_TTABLE) {
		return 0;
	}

	std::vector<uint32> npc_ids;
	int index = 1;
	while (luabind::type(table[index]) != LUA_TNIL) {
		auto current_id = table[index];
		uint32 npc_id = 0;
		if(luabind::type(current_id) != LUA_TNIL) {
			try {
				npc_id = luabind::object_cast<int>(current_id);
			} catch(luabind::cast_failed &) {
			}
		} else {
			break;
		}

		npc_ids.push_back(npc_id);
		++index;
	}

	if (npc_ids.empty()) {
		return 0;
	}

	return entity_list.CountSpawnedNPCs(npc_ids);
}

Lua_Spell lua_get_spell(uint32 spell_id) {
	return Lua_Spell(spell_id);
}

std::string lua_get_ldon_theme_name(uint32 theme_id) {
	return quest_manager.getldonthemename(theme_id);
}

std::string lua_get_faction_name(int faction_id) {
	return quest_manager.getfactionname(faction_id);
}

std::string lua_get_language_name(uint8 language_id) {
	return quest_manager.getlanguagename(language_id);
}

std::string lua_get_body_type_name(uint8 body_type_id) {
	return quest_manager.getbodytypename(body_type_id);
}

std::string lua_get_consider_level_name(uint8 consider_level) {
	return quest_manager.getconsiderlevelname(consider_level);
}

std::string lua_get_environmental_damage_name(uint8 damage_type) {
	return quest_manager.getenvironmentaldamagename(damage_type);
}

std::string lua_commify(std::string number) {
	return Strings::Commify(number);
}

bool lua_check_name_filter(std::string name) {
	return database.CheckNameFilter(name);
}

void lua_discord_send(std::string webhook_name, std::string message) {
	zone->SendDiscordMessage(webhook_name, message);
}

void lua_track_npc(uint32 entity_id) {
	quest_manager.TrackNPC(entity_id);
}

int lua_get_recipe_made_count(uint32 recipe_id) {
	return quest_manager.GetRecipeMadeCount(recipe_id);
}

std::string lua_get_recipe_name(uint32 recipe_id) {
	return quest_manager.GetRecipeName(recipe_id);
}

bool lua_has_recipe_learned(uint32 recipe_id) {
	return quest_manager.HasRecipeLearned(recipe_id);
}

bool lua_is_raining() {
	if (!zone) {
		return false;
	}

	return zone->IsRaining();
}

bool lua_is_snowing() {
	if (!zone) {
		return false;
	}

	return zone->IsSnowing();
}

std::string lua_get_aa_name(int aa_id) {
	if (!zone) {
		return std::string();
	}

	return zone->GetAAName(aa_id);
}

std::string lua_popup_break() {
	return DialogueWindow::Break();
}

std::string lua_popup_break(uint32 break_count) {
	return DialogueWindow::Break(break_count);
}

std::string lua_popup_center_message(std::string message) {
	return DialogueWindow::CenterMessage(message);
}

std::string lua_popup_color_message(std::string color, std::string message) {
	return DialogueWindow::ColorMessage(color, message);
}

std::string lua_popup_indent() {
	return DialogueWindow::Indent();
}

std::string lua_popup_indent(uint32 indent_count) {
	return DialogueWindow::Indent(indent_count);
}

std::string lua_popup_link(std::string link) {
	return DialogueWindow::Link(link);
}

std::string lua_popup_link(std::string link, std::string message) {
	return DialogueWindow::Link(link, message);
}

std::string lua_popup_table(std::string message) {
	return DialogueWindow::Table(message);
}

std::string lua_popup_table_cell() {
	return DialogueWindow::TableCell();
}

std::string lua_popup_table_cell(std::string message) {
	return DialogueWindow::TableCell(message);
}

std::string lua_popup_table_row(std::string message) {
	return DialogueWindow::TableRow(message);
}

void lua_marquee(uint32 type, std::string message) {
	quest_manager.marquee(type, message);
}

void lua_marquee(uint32 type, std::string message, uint32 duration) {
	quest_manager.marquee(type, message, duration);
}

void lua_marquee(uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, std::string message) {
	quest_manager.marquee(type, priority, fade_in, fade_out, duration, message);
}

void lua_zone_marquee(uint32 type, std::string message) {
	if (!zone) {
		return;
	}

	entity_list.Marquee(type, message);
}

void lua_zone_marquee(uint32 type, std::string message, uint32 duration) {
	if (!zone) {
		return;
	}

	entity_list.Marquee(type, message, duration);
}

void lua_zone_marquee(uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, std::string message) {
	if (!zone) {
		return;
	}

	entity_list.Marquee(type, priority, fade_in, fade_out, duration, message);
}

bool lua_is_hotzone()
{
	if (!zone) {
		return false;
	}

	return zone->IsHotzone();
}

void lua_set_hotzone(bool is_hotzone)
{
	if (!zone) {
		return;
	}

	zone->SetIsHotzone(is_hotzone);
}

void lua_set_proximity_range(float x_range, float y_range)
{
	quest_manager.set_proximity_range(x_range, y_range);
}

void lua_set_proximity_range(float x_range, float y_range, float z_range)
{
	quest_manager.set_proximity_range(x_range, y_range, z_range);
}

void lua_set_proximity_range(float x_range, float y_range, float z_range, bool enable_say)
{
	quest_manager.set_proximity_range(x_range, y_range, z_range, enable_say);
}

void lua_do_anim(int animation_id)
{
	quest_manager.doanim(animation_id);
}

void lua_do_anim(int animation_id, int animation_speed)
{
	quest_manager.doanim(animation_id, animation_speed);
}

void lua_do_anim(int animation_id, int animation_speed, bool ackreq)
{
	quest_manager.doanim(animation_id, animation_speed, ackreq);
}

void lua_do_anim(int animation_id, int animation_speed, bool ackreq, int filter)
{
	quest_manager.doanim(animation_id, animation_speed, ackreq, static_cast<eqFilterType>(filter));
}

std::string lua_item_link(Lua_ItemInst inst) {
	return quest_manager.varlink(inst);
}

std::string lua_item_link(uint32 item_id) {
	return quest_manager.varlink(item_id);
}

std::string lua_item_link(uint32 item_id, int16 charges) {
	return quest_manager.varlink(item_id, charges);
}

std::string lua_item_link(uint32 item_id, int16 charges, uint32 aug1) {
	return quest_manager.varlink(item_id, charges, aug1);
}

std::string lua_item_link(uint32 item_id, int16 charges, uint32 aug1, uint32 aug2) {
	return quest_manager.varlink(item_id, charges, aug1, aug2);
}

std::string lua_item_link(uint32 item_id, int16 charges, uint32 aug1, uint32 aug2, uint32 aug3) {
	return quest_manager.varlink(item_id, charges, aug1, aug2, aug3);
}

std::string lua_item_link(uint32 item_id, int16 charges, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4) {
	return quest_manager.varlink(item_id, charges, aug1, aug2, aug3, aug4);
}

std::string lua_item_link(uint32 item_id, int16 charges, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5) {
	return quest_manager.varlink(item_id, charges, aug1, aug2, aug3, aug4, aug5);
}

std::string lua_item_link(uint32 item_id, int16 charges, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5, uint32 aug6) {
	return quest_manager.varlink(item_id, charges, aug1, aug2, aug3, aug4, aug5, aug6);
}

std::string lua_item_link(uint32 item_id, int16 charges, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5, uint32 aug6, bool attuned) {
	return quest_manager.varlink(item_id, charges, aug1, aug2, aug3, aug4, aug5, aug6, attuned);
}

bool lua_do_augment_slots_match(uint32 item_one, uint32 item_two)
{
	return quest_manager.DoAugmentSlotsMatch(item_one, item_two);
}

int8 lua_does_augment_fit(Lua_ItemInst inst, uint32 augment_id)
{
	return quest_manager.DoesAugmentFit(inst, augment_id);
}

int8 lua_does_augment_fit_slot(Lua_ItemInst inst, uint32 augment_id, uint8 augment_slot)
{
	return quest_manager.DoesAugmentFit(inst, augment_id, augment_slot);
}

luabind::object lua_get_recipe_component_item_ids(lua_State* L, uint32 recipe_id)
{
	auto lua_table = luabind::newtable(L);

	const auto& l = content_db.GetRecipeComponentItemIDs(RecipeCountType::Component, recipe_id);
	if (!l.empty()) {
		int index = 1;
		for (const auto& i : l) {
			lua_table[index] = i;
			index++;
		}
	}

	return lua_table;
}

luabind::object lua_get_recipe_container_item_ids(lua_State* L, uint32 recipe_id)
{
	auto lua_table = luabind::newtable(L);

	const auto& l = content_db.GetRecipeComponentItemIDs(RecipeCountType::Container, recipe_id);
	if (!l.empty()) {
		int index = 1;
		for (const auto& i : l) {
			lua_table[index] = i;
			index++;
		}
	}

	return lua_table;
}

luabind::object lua_get_recipe_fail_item_ids(lua_State* L, uint32 recipe_id)
{
	auto lua_table = luabind::newtable(L);

	const auto& l = content_db.GetRecipeComponentItemIDs(RecipeCountType::Fail, recipe_id);
	if (!l.empty()) {
		int index = 1;
		for (const auto& i : l) {
			lua_table[index] = i;
			index++;
		}
	}

	return lua_table;
}

luabind::object lua_get_recipe_salvage_item_ids(lua_State* L, uint32 recipe_id) {
	auto lua_table = luabind::newtable(L);

	const auto& l = content_db.GetRecipeComponentItemIDs(RecipeCountType::Salvage, recipe_id);
	if (!l.empty()) {
		int index = 1;
		for (const auto& i : l) {
			lua_table[index] = i;
			index++;
		}
	}

	return lua_table;
}

luabind::object lua_get_recipe_success_item_ids(lua_State* L, uint32 recipe_id) {
	auto lua_table = luabind::newtable(L);

	const auto& l = content_db.GetRecipeComponentItemIDs(RecipeCountType::Success, recipe_id);
	if (!l.empty()) {
		int index = 1;
		for (const auto& i : l) {
			lua_table[index] = i;
			index++;
		}
	}

	return lua_table;
}

int8 lua_get_recipe_component_count(uint32 recipe_id, uint32 item_id)
{
	return content_db.GetRecipeComponentCount(RecipeCountType::Component, recipe_id, item_id);
}

int8 lua_get_recipe_fail_count(uint32 recipe_id, uint32 item_id)
{
	return content_db.GetRecipeComponentCount(RecipeCountType::Fail, recipe_id, item_id);
}

int8 lua_get_recipe_salvage_count(uint32 recipe_id, uint32 item_id)
{
	return content_db.GetRecipeComponentCount(RecipeCountType::Salvage, recipe_id, item_id);
}

int8 lua_get_recipe_success_count(uint32 recipe_id, uint32 item_id)
{
	return content_db.GetRecipeComponentCount(RecipeCountType::Success, recipe_id, item_id);
}

void lua_send_player_handin_event()
{
	quest_manager.SendPlayerHandinEvent();
}

float lua_get_zone_safe_x(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneSafeCoordinates(zone_id).x;
}

float lua_get_zone_safe_x(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneSafeCoordinates(zone_id, version).x;
}

float lua_get_zone_safe_y(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneSafeCoordinates(zone_id).y;
}

float lua_get_zone_safe_y(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneSafeCoordinates(zone_id, version).y;
}

float lua_get_zone_safe_z(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneSafeCoordinates(zone_id).z;
}

float lua_get_zone_safe_z(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneSafeCoordinates(zone_id, version).z;
}

float lua_get_zone_safe_heading(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneSafeCoordinates(zone_id).w;
}

float lua_get_zone_safe_heading(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneSafeCoordinates(zone_id, version).w;
}

float lua_get_zone_graveyard_id(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneGraveyardID(zone_id);
}

float lua_get_zone_graveyard_id(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneGraveyardID(zone_id, version);
}

uint8 lua_get_zone_minimum_level(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneMinimumLevel(zone_id);
}

uint8 lua_get_zone_minimum_level(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneMinimumLevel(zone_id, version);
}

uint8 lua_get_zone_maximum_level(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneMaximumLevel(zone_id);
}

uint8 lua_get_zone_maximum_level(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneMaximumLevel(zone_id, version);
}

uint8 lua_get_zone_minimum_status(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneMinimumStatus(zone_id);
}

uint8 lua_get_zone_minimum_status(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneMinimumStatus(zone_id, version);
}

int lua_get_zone_time_zone(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneTimeZone(zone_id);
}

int lua_get_zone_time_zone(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneTimeZone(zone_id, version);
}

int lua_get_zone_maximum_players(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneMaximumPlayers(zone_id);
}

int lua_get_zone_maximum_players(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneMaximumPlayers(zone_id, version);
}

uint32 lua_get_zone_rule_set(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneRuleSet(zone_id);
}

uint32 lua_get_zone_rule_set(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneRuleSet(zone_id, version);
}

std::string lua_get_zone_note(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneNote(zone_id);
}

std::string lua_get_zone_note(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneNote(zone_id, version);
}

float lua_get_zone_underworld(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneUnderworld(zone_id);
}

float lua_get_zone_underworld(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneUnderworld(zone_id, version);
}

float lua_get_zone_minimum_clip(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneMinimumClip(zone_id);
}

float lua_get_zone_minimum_clip(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneMinimumClip(zone_id, version);
}

float lua_get_zone_maximum_clip(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneMaximumClip(zone_id);
}

float lua_get_zone_maximum_clip(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneMaximumClip(zone_id, version);
}

float lua_get_zone_fog_minimum_clip(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneFogMinimumClip(zone_id);
}

float lua_get_zone_fog_minimum_clip(uint32 zone_id, uint8 slot)
{
	return ZoneStore::Instance()->GetZoneFogMinimumClip(zone_id, slot);
}

float lua_get_zone_fog_minimum_clip(uint32 zone_id, uint8 slot, int version)
{
	return ZoneStore::Instance()->GetZoneFogMinimumClip(zone_id, slot);
}

float lua_get_zone_fog_maximum_clip(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneFogMaximumClip(zone_id);
}

float lua_get_zone_fog_maximum_clip(uint32 zone_id, uint8 slot)
{
	return ZoneStore::Instance()->GetZoneFogMaximumClip(zone_id, slot);
}

float lua_get_zone_fog_maximum_clip(uint32 zone_id, uint8 slot, int version)
{
	return ZoneStore::Instance()->GetZoneFogMaximumClip(zone_id, slot, version);
}

uint8 lua_get_zone_fog_red(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneFogRed(zone_id);
}

uint8 lua_get_zone_fog_red(uint32 zone_id, uint8 slot)
{
	return ZoneStore::Instance()->GetZoneFogRed(zone_id, slot);
}

uint8 lua_get_zone_fog_red(uint32 zone_id, uint8 slot, int version)
{
	return ZoneStore::Instance()->GetZoneFogRed(zone_id, slot, version);
}

uint8 lua_get_zone_fog_green(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneFogGreen(zone_id);
}

uint8 lua_get_zone_fog_green(uint32 zone_id, uint8 slot)
{
	return ZoneStore::Instance()->GetZoneFogGreen(zone_id, slot);
}

uint8 lua_get_zone_fog_green(uint32 zone_id, uint8 slot, int version)
{
	return ZoneStore::Instance()->GetZoneFogGreen(zone_id, slot, version);
}

uint8 lua_get_zone_fog_blue(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneFogBlue(zone_id);
}

uint8 lua_get_zone_fog_blue(uint32 zone_id, uint8 slot)
{
	return ZoneStore::Instance()->GetZoneFogBlue(zone_id, slot);
}

uint8 lua_get_zone_fog_blue(uint32 zone_id, uint8 slot, int version)
{
	return ZoneStore::Instance()->GetZoneFogBlue(zone_id, slot, version);
}

uint8 lua_get_zone_sky(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneSky(zone_id);
}

uint8 lua_get_zone_sky(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneSky(zone_id, version);
}

uint8 lua_get_zone_ztype(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneZType(zone_id);
}

uint8 lua_get_zone_ztype(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneZType(zone_id, version);
}

float lua_get_zone_experience_multiplier(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneExperienceMultiplier(zone_id);
}

float lua_get_zone_experience_multiplier(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneExperienceMultiplier(zone_id, version);
}

float lua_get_zone_walk_speed(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneWalkSpeed(zone_id);
}

float lua_get_zone_walk_speed(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneWalkSpeed(zone_id, version);
}

uint8 lua_get_zone_time_type(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneTimeType(zone_id);
}

uint8 lua_get_zone_time_type(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneTimeType(zone_id, version);
}

float lua_get_zone_fog_density(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneFogDensity(zone_id);
}

float lua_get_zone_fog_density(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneFogDensity(zone_id, version);
}

std::string lua_get_zone_flag_needed(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneFlagNeeded(zone_id);
}

std::string lua_get_zone_flag_needed(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneFlagNeeded(zone_id, version);
}

int8 lua_get_zone_can_bind(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneCanBind(zone_id);
}

int8 lua_get_zone_can_bind(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneCanBind(zone_id, version);
}

int8 lua_get_zone_can_combat(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneCanCombat(zone_id);
}

int8 lua_get_zone_can_combat(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneCanCombat(zone_id, version);
}

int8 lua_get_zone_can_levitate(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneCanLevitate(zone_id);
}

int8 lua_get_zone_can_levitate(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneCanLevitate(zone_id, version);
}

int8 lua_get_zone_cast_outdoor(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneCastOutdoor(zone_id);
}

int8 lua_get_zone_cast_outdoor(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneCastOutdoor(zone_id, version);
}

uint8 lua_get_zone_hotzone(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneHotzone(zone_id);
}

uint8 lua_get_zone_hotzone(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneHotzone(zone_id, version);
}

uint8 lua_get_zone_instance_type(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneInstanceType(zone_id);
}

uint8 lua_get_zone_instance_type(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneInstanceType(zone_id, version);
}

uint64 lua_get_zone_shutdown_delay(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneShutdownDelay(zone_id);
}

uint64 lua_get_zone_shutdown_delay(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneShutdownDelay(zone_id, version);
}

int8 lua_get_zone_peqzone(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZonePEQZone(zone_id);
}

int8 lua_get_zone_peqzone(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZonePEQZone(zone_id, version);
}

int8 lua_get_zone_expansion(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneExpansion(zone_id);
}

int8 lua_get_zone_expansion(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneExpansion(zone_id, version);
}

int8 lua_get_zone_bypass_expansion_check(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneBypassExpansionCheck(zone_id);
}

int8 lua_get_zone_bypass_expansion_check(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneBypassExpansionCheck(zone_id, version);
}

int8 lua_get_zone_suspend_buffs(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneSuspendBuffs(zone_id);
}

int8 lua_get_zone_suspend_buffs(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneSuspendBuffs(zone_id, version);
}

int lua_get_zone_rain_chance(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneRainChance(zone_id);
}

int lua_get_zone_rain_chance(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneRainChance(zone_id, version);
}

int lua_get_zone_rain_duration(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneRainDuration(zone_id);
}

int lua_get_zone_rain_duration(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneRainDuration(zone_id, version);
}

int lua_get_zone_snow_chance(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneSnowChance(zone_id);
}

int lua_get_zone_snow_chance(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneSnowChance(zone_id, version);
}

int lua_get_zone_snow_duration(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneSnowDuration(zone_id);
}

int lua_get_zone_snow_duration(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneSnowDuration(zone_id, version);
}

float lua_get_zone_gravity(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneGravity(zone_id);
}

float lua_get_zone_gravity(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneGravity(zone_id, version);
}

int lua_get_zone_type(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneType(zone_id);
}

int lua_get_zone_type(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneType(zone_id, version);
}

int lua_get_zone_sky_lock(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneSkyLock(zone_id);
}

int lua_get_zone_sky_lock(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneSkyLock(zone_id, version);
}

int lua_get_zone_fast_regen_hp(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneFastRegenHP(zone_id);
}

int lua_get_zone_fast_regen_hp(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneFastRegenHP(zone_id, version);
}

int lua_get_zone_fast_regen_mana(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneFastRegenMana(zone_id);
}

int lua_get_zone_fast_regen_mana(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneFastRegenMana(zone_id, version);
}

int lua_get_zone_fast_regen_endurance(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneFastRegenEndurance(zone_id);
}

int lua_get_zone_fast_regen_endurance(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneFastRegenEndurance(zone_id, version);
}

int lua_get_zone_npc_maximum_aggro_distance(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneNPCMaximumAggroDistance(zone_id);
}

int lua_get_zone_npc_maximum_aggro_distance(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneNPCMaximumAggroDistance(zone_id, version);
}

int8 lua_get_zone_minimum_expansion(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneMinimumExpansion(zone_id);
}

int8 lua_get_zone_minimum_expansion(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneMinimumExpansion(zone_id, version);
}

int8 lua_get_zone_maximum_expansion(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneMaximumExpansion(zone_id);
}

int8 lua_get_zone_maximum_expansion(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneMaximumExpansion(zone_id, version);
}

std::string lua_get_zone_content_flags(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneContentFlags(zone_id);
}

std::string lua_get_zone_content_flags(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneContentFlags(zone_id, version);
}

std::string lua_get_zone_content_flags_disabled(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneContentFlagsDisabled(zone_id);
}

std::string lua_get_zone_content_flags_disabled(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneContentFlagsDisabled(zone_id, version);
}

int lua_get_zone_underworld_teleport_index(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneUnderworldTeleportIndex(zone_id);
}

int lua_get_zone_underworld_teleport_index(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneUnderworldTeleportIndex(zone_id, version);
}

int lua_get_zone_lava_damage(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneLavaDamage(zone_id);
}

int lua_get_zone_lava_damage(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneLavaDamage(zone_id, version);
}

int lua_get_zone_minimum_lava_damage(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneMinimumLavaDamage(zone_id);
}

int lua_get_zone_minimum_lava_damage(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneMinimumLavaDamage(zone_id, version);
}

uint8 lua_get_zone_idle_when_empty(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneIdleWhenEmpty(zone_id);
}

uint8 lua_get_zone_idle_when_empty(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneIdleWhenEmpty(zone_id, version);
}

uint32 lua_get_zone_seconds_before_idle(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneSecondsBeforeIdle(zone_id);
}

uint32 lua_get_zone_seconds_before_idle(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneSecondsBeforeIdle(zone_id, version);
}

void lua_send_channel_message(uint8 channel_number, uint32 guild_id, uint8 language_id, uint8 language_skill, const char* message)
{
	quest_manager.SendChannelMessage(channel_number, guild_id, language_id, language_skill, message);
}

void lua_send_channel_message(Lua_Client from, uint8 channel_number, uint32 guild_id, uint8 language_id, uint8 language_skill, const char* message)
{
	quest_manager.SendChannelMessage(from, channel_number, guild_id, language_id, language_skill, message);
}

void lua_send_channel_message(Lua_Client from, const char* to, uint8 channel_number, uint32 guild_id, uint8 language_id, uint8 language_skill, const char* message)
{
	quest_manager.SendChannelMessage(from, to, channel_number, guild_id, language_id, language_skill, message);
}

uint8 lua_get_spell_level(uint16 spell_id, uint8 class_id)
{
	const auto spell_level = GetSpellLevel(spell_id, class_id);
	return spell_level > RuleI(Character, MaxLevel) ? UINT8_MAX : spell_level;
}

bool lua_is_effect_in_spell(uint16 spell_id, int effect_id)
{
	return IsEffectInSpell(spell_id, effect_id);
}

bool lua_is_beneficial_spell(uint16 spell_id)
{
	return IsBeneficialSpell(spell_id);
}

bool lua_is_detrimental_spell(uint16 spell_id)
{
	return IsDetrimentalSpell(spell_id);
}

bool lua_is_targetable_ae_spell(uint16 spell_id)
{
	return IsTargetableAESpell(spell_id);
}

bool lua_is_sacrifice_spell(uint16 spell_id)
{
	return IsSacrificeSpell(spell_id);
}

bool lua_is_lifetap_spell(uint16 spell_id)
{
	return IsLifetapSpell(spell_id);
}

bool lua_is_mesmerize_spell(uint16 spell_id)
{
	return IsMesmerizeSpell(spell_id);
}

bool lua_is_stun_spell(uint16 spell_id)
{
	return IsStunSpell(spell_id);
}

bool lua_is_summon_spell(uint16 spell_id)
{
	return IsSummonSpell(spell_id);
}

bool lua_is_damage_spell(uint16 spell_id)
{
	return IsDamageSpell(spell_id);
}

bool lua_is_fear_spell(uint16 spell_id)
{
	return IsFearSpell(spell_id);
}

bool lua_is_cure_spell(uint16 spell_id)
{
	return IsCureSpell(spell_id);
}

bool lua_is_haste_spell(uint16 spell_id)
{
	return IsHasteSpell(spell_id);
}

bool lua_is_harmony_spell(uint16 spell_id)
{
	return IsHarmonySpell(spell_id);
}

bool lua_is_percental_heal_spell(uint16 spell_id)
{
	return IsPercentalHealSpell(spell_id);
}

bool lua_is_group_only_spell(uint16 spell_id)
{
	return IsGroupOnlySpell(spell_id);
}

bool lua_is_invisible_spell(uint16 spell_id)
{
	return IsInvisibleSpell(spell_id);
}

bool lua_is_invulnerability_spell(uint16 spell_id)
{
	return IsInvulnerabilitySpell(spell_id);
}

bool lua_is_complete_heal_duration_spell(uint16 spell_id)
{
	return IsCompleteHealDurationSpell(spell_id);
}

bool lua_is_poison_counter_spell(uint16 spell_id)
{
	return IsPoisonCounterSpell(spell_id);
}

bool lua_is_disease_counter_spell(uint16 spell_id)
{
	return IsDiseaseCounterSpell(spell_id);
}

bool lua_is_summon_item_spell(uint16 spell_id)
{
	return IsSummonItemSpell(spell_id);
}

bool lua_is_summon_skeleton_spell(uint16 spell_id)
{
	return IsSummonSkeletonSpell(spell_id);
}

bool lua_is_summon_pet_spell(uint16 spell_id)
{
	return IsSummonPetSpell(spell_id);
}

bool lua_is_pet_spell(uint16 spell_id)
{
	return IsPetSpell(spell_id);
}

bool lua_is_summon_pc_spell(uint16 spell_id)
{
	return IsSummonPCSpell(spell_id);
}

bool lua_is_charm_spell(uint16 spell_id)
{
	return IsCharmSpell(spell_id);
}

bool lua_is_blind_spell(uint16 spell_id)
{
	return IsBlindSpell(spell_id);
}

bool lua_is_health_spell(uint16 spell_id)
{
	return IsHealthSpell(spell_id);
}

bool lua_is_cast_time_reduction_spell(uint16 spell_id)
{
	return IsCastTimeReductionSpell(spell_id);
}

bool lua_is_increase_duration_spell(uint16 spell_id)
{
	return IsIncreaseDurationSpell(spell_id);
}

bool lua_is_mana_cost_reduction_spell(uint16 spell_id)
{
	return IsManaCostReductionSpell(spell_id);
}

bool lua_is_increase_range_spell(uint16 spell_id)
{
	return IsIncreaseRangeSpell(spell_id);
}

bool lua_is_improved_healing_spell(uint16 spell_id)
{
	return IsImprovedHealingSpell(spell_id);
}

bool lua_is_improved_damage_spell(uint16 spell_id)
{
	return IsImprovedDamageSpell(spell_id);
}

bool lua_is_ae_duration_spell(uint16 spell_id)
{
	return IsAEDurationSpell(spell_id);
}

bool lua_is_pure_nuke_spell(uint16 spell_id)
{
	return IsPureNukeSpell(spell_id);
}

bool lua_is_ae_nuke_spell(uint16 spell_id)
{
	return IsAENukeSpell(spell_id);
}

bool lua_is_pbae_nuke_spell(uint16 spell_id)
{
	return IsPBAENukeSpell(spell_id);
}

bool lua_is_ae_rain_nuke_spell(uint16 spell_id)
{
	return IsAERainNukeSpell(spell_id);
}

bool lua_is_partial_resistable_spell(uint16 spell_id)
{
	return IsPartialResistableSpell(spell_id);
}

bool lua_is_resistable_spell(uint16 spell_id)
{
	return IsResistableSpell(spell_id);
}

bool lua_is_group_spell(uint16 spell_id)
{
	return IsGroupSpell(spell_id);
}

bool lua_is_tgb_compatible_spell(uint16 spell_id)
{
	return IsTGBCompatibleSpell(spell_id);
}

bool lua_is_bard_song(uint16 spell_id)
{
	return IsBardSong(spell_id);
}

bool lua_is_pulsing_bard_song(uint16 spell_id)
{
	return IsPulsingBardSong(spell_id);
}

bool lua_is_discipline_buff(uint16 spell_id)
{
	return IsDisciplineBuff(spell_id);
}

bool lua_is_discipline(uint16 spell_id)
{
	return IsDiscipline(spell_id);
}

bool lua_is_combat_skill(uint16 spell_id)
{
	return IsCombatSkill(spell_id);
}

bool lua_is_resurrection_effects(uint16 spell_id)
{
	return IsResurrectionEffects(spell_id);
}

bool lua_is_rune_spell(uint16 spell_id)
{
	return IsRuneSpell(spell_id);
}

bool lua_is_magic_rune_spell(uint16 spell_id)
{
	return IsMagicRuneSpell(spell_id);
}

bool lua_is_mana_tap_spell(uint16 spell_id)
{
	return IsManaTapSpell(spell_id);
}

bool lua_is_alliance_spell(uint16 spell_id)
{
	return IsAllianceSpell(spell_id);
}

bool lua_is_death_save_spell(uint16 spell_id)
{
	return IsDeathSaveSpell(spell_id);
}

bool lua_is_partial_death_save_spell(uint16 spell_id)
{
	return IsPartialDeathSaveSpell(spell_id);
}

bool lua_is_full_death_save_spell(uint16 spell_id)
{
	return IsFullDeathSaveSpell(spell_id);
}

bool lua_is_shadow_step_spell(uint16 spell_id)
{
	return IsShadowStepSpell(spell_id);
}

bool lua_is_succor_spell(uint16 spell_id)
{
	return IsSuccorSpell(spell_id);
}

bool lua_is_teleport_spell(uint16 spell_id)
{
	return IsTeleportSpell(spell_id);
}

bool lua_is_translocate_spell(uint16 spell_id)
{
	return IsTranslocateSpell(spell_id);
}

bool lua_is_gate_spell(uint16 spell_id)
{
	return IsGateSpell(spell_id);
}

bool lua_is_illusion_spell(uint16 spell_id)
{
	return IsIllusionSpell(spell_id);
}

bool lua_is_ldon_object_spell(uint16 spell_id)
{
	return IsLDoNObjectSpell(spell_id);
}

bool lua_is_heal_over_time_spell(uint16 spell_id)
{
	return IsHealOverTimeSpell(spell_id);
}

bool lua_is_complete_heal_spell(uint16 spell_id)
{
	return IsCompleteHealSpell(spell_id);
}

bool lua_is_fast_heal_spell(uint16 spell_id)
{
	return IsFastHealSpell(spell_id);
}

bool lua_is_very_fast_heal_spell(uint16 spell_id)
{
	return IsVeryFastHealSpell(spell_id);
}

bool lua_is_regular_single_target_heal_spell(uint16 spell_id)
{
	return IsRegularSingleTargetHealSpell(spell_id);
}

bool lua_is_regular_group_heal_spell(uint16 spell_id)
{
	return IsRegularGroupHealSpell(spell_id);
}

bool lua_is_group_complete_heal_spell(uint16 spell_id)
{
	return IsGroupCompleteHealSpell(spell_id);
}

bool lua_is_group_heal_over_time_spell(uint16 spell_id)
{
	return IsGroupHealOverTimeSpell(spell_id);
}

bool lua_is_debuff_spell(uint16 spell_id)
{
	return IsDebuffSpell(spell_id);
}

bool lua_is_resist_debuff_spell(uint16 spell_id)
{
	return IsResistDebuffSpell(spell_id);
}

bool lua_is_self_conversion_spell(uint16 spell_id)
{
	return IsSelfConversionSpell(spell_id);
}

bool lua_is_buff_spell(uint16 spell_id)
{
	return IsBuffSpell(spell_id);
}

bool lua_is_persist_death_spell(uint16 spell_id)
{
	return IsPersistDeathSpell(spell_id);
}

bool lua_is_suspendable_spell(uint16 spell_id)
{
	return IsSuspendableSpell(spell_id);
}

bool lua_is_cast_on_fade_duration_spell(uint16 spell_id)
{
	return IsCastOnFadeDurationSpell(spell_id);
}

bool lua_is_distance_modifier_spell(uint16 spell_id)
{
	return IsDistanceModifierSpell(spell_id);
}

bool lua_is_rest_allowed_spell(uint16 spell_id)
{
	return IsRestAllowedSpell(spell_id);
}

bool lua_is_no_detrimental_spell_aggro_spell(uint16 spell_id)
{
	return IsNoDetrimentalSpellAggroSpell(spell_id);
}

bool lua_is_stackable_dot(uint16 spell_id)
{
	return IsStackableDOT(spell_id);
}

bool lua_is_short_duration_buff(uint16 spell_id)
{
	return IsShortDurationBuff(spell_id);
}

bool lua_is_target_required_for_spell(uint16 spell_id)
{
	return IsTargetRequiredForSpell(spell_id);
}

bool lua_is_virus_spell(uint16 spell_id)
{
	return IsVirusSpell(spell_id);
}

bool lua_is_valid_spell(uint16 spell_id)
{
	return IsValidSpell(spell_id);
}

bool lua_is_effect_ignored_in_stacking(int effect_id)
{
	return IsEffectIgnoredInStacking(effect_id);
}

bool lua_is_focus_limit(int effect_id)
{
	return IsFocusLimit(effect_id);
}

bool lua_is_bard_only_stack_effect(int effect_id)
{
	return IsBardOnlyStackEffect(effect_id);
}

bool lua_is_cast_while_invisible_spell(uint16 spell_id)
{
	return IsCastWhileInvisibleSpell(spell_id);
}

bool lua_is_cast_restricted_spell(uint16 spell_id)
{
	return IsCastRestrictedSpell(spell_id);
}

bool lua_is_cast_not_standing_spell(uint16 spell_id)
{
	return IsCastNotStandingSpell(spell_id);
}

bool lua_is_instrument_modifier_applied_to_spell_effect(uint16 spell_id, int effect_id)
{
	return IsInstrumentModifierAppliedToSpellEffect(spell_id, effect_id);
}

bool lua_is_blank_spell_effect(uint16 spell_id, int effect_index)
{
	return IsBlankSpellEffect(spell_id, effect_index);
}

uint16 lua_get_spell_trigger_spell_id(uint16 spell_id, int effect_id)
{
	return GetSpellTriggerSpellID(spell_id, effect_id);
}

uint8 lua_get_spell_minimum_level(uint16 spell_id)
{
	return GetSpellMinimumLevel(spell_id);
}

int lua_get_spell_resist_type(uint16 spell_id)
{
	return GetSpellResistType(spell_id);
}

int lua_get_spell_target_type(uint16 spell_id)
{
	return GetSpellTargetType(spell_id);
}

int lua_get_spell_partial_melee_rune_reduction(uint16 spell_id)
{
	return GetSpellPartialMeleeRuneReduction(spell_id);
}

int lua_get_spell_partial_magic_rune_reduction(uint16 spell_id)
{
	return GetSpellPartialMagicRuneReduction(spell_id);
}

int lua_get_spell_partial_melee_rune_amount(uint16 spell_id)
{
	return GetSpellPartialMeleeRuneAmount(spell_id);
}

int lua_get_spell_partial_magic_rune_amount(uint16 spell_id)
{
	return GetSpellPartialMagicRuneAmount(spell_id);
}

int lua_get_spell_viral_minimum_spread_time(uint16 spell_id)
{
	return GetSpellViralMinimumSpreadTime(spell_id);
}

int lua_get_spell_viral_maximum_spread_time(uint16 spell_id)
{
	return GetSpellViralMaximumSpreadTime(spell_id);
}

int lua_get_spell_viral_spread_range(uint16 spell_id)
{
	return GetSpellViralSpreadRange(spell_id);
}

int lua_get_spell_proc_limit_timer(uint16 spell_id, int proc_type)
{
	return GetSpellProcLimitTimer(spell_id, proc_type);
}

int lua_get_spell_effect_description_number(uint16 spell_id)
{
	return GetSpellEffectDescriptionNumber(spell_id);
}

int lua_get_spell_furious_bash(uint16 spell_id)
{
	return GetSpellFuriousBash(spell_id);
}

bool lua_is_spell_usable_in_this_zone_type(uint16 spell_id)
{
	return IsSpellUsableInThisZoneType(spell_id, zone->GetZoneType());
}

bool lua_is_spell_usable_in_this_zone_type(uint16 spell_id, uint8 zone_type)
{
	return IsSpellUsableInThisZoneType(spell_id, zone_type);
}

int lua_get_spell_effect_index(uint16 spell_id, int effect_id)
{
	return GetSpellEffectIndex(spell_id, effect_id);
}

int lua_calculate_poison_counters(uint16 spell_id)
{
	return CalculatePoisonCounters(spell_id);
}

int lua_calculate_disease_counters(uint16 spell_id)
{
	return CalculateDiseaseCounters(spell_id);
}

int lua_calculate_curse_counters(uint16 spell_id)
{
	return CalculateCurseCounters(spell_id);
}

int lua_calculate_corruption_counters(uint16 spell_id)
{
	return CalculateCorruptionCounters(spell_id);
}

int lua_calculate_counters(uint16 spell_id)
{
	return CalculateCounters(spell_id);
}

int8 lua_get_spell_resurrection_sickness_check(uint16 spell_id_one, uint16 spell_id_two)
{
	return GetSpellResurrectionSicknessCheck(spell_id_one, spell_id_two);
}

int lua_get_spell_nimbus_effect(uint16 spell_id)
{
	return GetSpellNimbusEffect(spell_id);
}

std::string lua_convert_money_to_string(luabind::adl::object table)
{
	if (luabind::type(table) != LUA_TTABLE) {
		return std::string();
	}

	uint64 platinum = luabind::type(table["platinum"]) != LUA_TNIL ? luabind::object_cast<uint64>(table["platinum"]) : 0;
	uint64 gold     = luabind::type(table["gold"]) != LUA_TNIL ? luabind::object_cast<uint64>(table["gold"]) : 0;
	uint64 silver   = luabind::type(table["silver"]) != LUA_TNIL ? luabind::object_cast<uint64>(table["silver"]) : 0;
	uint64 copper   = luabind::type(table["copper"]) != LUA_TNIL ? luabind::object_cast<uint64>(table["copper"]) : 0;

	if (
		!copper &&
		!silver &&
		!gold &&
		!platinum
	) {
		return std::string();
	}

	return Strings::Money(platinum, gold, silver, copper);
}

void lua_cast_spell(uint16 spell_id, uint16 target_id)
{
	quest_manager.castspell(spell_id, target_id);
}

void lua_self_cast(uint16 spell_id)
{
	quest_manager.selfcast(spell_id);
}

uint8 lua_get_bot_class_by_id(uint32 bot_id)
{
	return database.botdb.GetBotClassByID(bot_id);
}

uint8 lua_get_bot_gender_by_id(uint32 bot_id)
{
	return database.botdb.GetBotGenderByID(bot_id);
}

luabind::object lua_get_bot_ids_by_character_id(lua_State* L, uint32 character_id)
{
	auto lua_table = luabind::newtable(L);

	const auto& l = database.botdb.GetBotIDsByCharacterID(character_id);

	if (!l.empty()) {
		int index = 1;
		for (const auto& i : l) {
			lua_table[index] = i;
			index++;
		}
	}

	return lua_table;
}

luabind::object lua_get_bot_ids_by_character_id(lua_State* L, uint32 character_id, uint8 class_id)
{
	auto lua_table = luabind::newtable(L);

	const auto& l = database.botdb.GetBotIDsByCharacterID(character_id, class_id);

	if (!l.empty()) {
		int index = 1;
		for (const auto& i : l) {
			lua_table[index] = i;
			index++;
		}
	}

	return lua_table;
}

uint8 lua_get_bot_level_by_id(uint32 bot_id)
{
	return database.botdb.GetBotLevelByID(bot_id);
}

std::string lua_get_bot_name_by_id(uint32 bot_id)
{
	return database.botdb.GetBotNameByID(bot_id);
}

uint16 lua_get_bot_race_by_id(uint32 bot_id)
{
	return database.botdb.GetBotRaceByID(bot_id);
}

std::string lua_silent_say_link(std::string text) {
	return Saylink::Silent(text);
}

std::string lua_silent_say_link(std::string text, std::string link_name) {
	return Saylink::Silent(text, link_name);
}

uint16 lua_get_class_bitmask(uint8 class_id) {
	return GetPlayerClassBit(class_id);
}

uint32 lua_get_deity_bitmask(uint32 deity_id) {
	return Deity::GetBitmask(deity_id);
}

uint16 lua_get_race_bitmask(uint16 race_id) {
	return GetPlayerRaceBit(race_id);
}

std::string lua_get_auto_login_character_name_by_account_id(uint32 account_id) {
	return quest_manager.GetAutoLoginCharacterNameByAccountID(account_id);
}

bool lua_set_auto_login_character_name_by_account_id(uint32 account_id, std::string character_name) {
	return quest_manager.SetAutoLoginCharacterNameByAccountID(account_id, character_name);
}

uint32 lua_get_zone_id_by_long_name(std::string zone_long_name) {
	return ZoneStore::Instance()->GetZoneIDByLongName(zone_long_name);
}

std::string lua_get_zone_short_name_by_long_name(std::string zone_long_name) {
	return ZoneStore::Instance()->GetZoneShortNameByLongName(zone_long_name);
}

bool lua_send_parcel(luabind::object lua_table)
{
	if (luabind::type(lua_table) != LUA_TTABLE) {
		return false;
	}

	if (
		(luabind::type(lua_table["name"]) == LUA_TNIL && luabind::type(lua_table["character_id"]) == LUA_TNIL) ||
		luabind::type(lua_table["item_id"]) == LUA_TNIL ||
		luabind::type(lua_table["quantity"]) == LUA_TNIL
	) {
		return false;
	}

	std::string name         = luabind::type(lua_table["name"]) != LUA_TNIL ? luabind::object_cast<std::string>(lua_table["name"]) : "";
	uint32      character_id = luabind::type(lua_table["character_id"]) != LUA_TNIL ? luabind::object_cast<uint32>(lua_table["character_id"]) : 0;

	if (character_id) {
		const std::string& character_name = database.GetCharName(character_id);
		if (character_name.empty()) {
			return false;
		}

		name = character_name;
	} else {
		auto v = CharacterParcelsRepository::GetParcelCountAndCharacterName(database, name);
		if (v.at(0).character_name.empty()) {
			return false;
		}

		character_id = v.at(0).char_id;
	}

	if (!character_id) {
		return false;
	}

	const int next_parcel_slot = CharacterParcelsRepository::GetNextFreeParcelSlot(database, character_id, RuleI(Parcel, ParcelMaxItems));
	if (next_parcel_slot == INVALID_INDEX) {
		return false;
	}

	const uint32 item_id         = luabind::object_cast<uint32>(lua_table["item_id"]);
	const int16  quantity        = luabind::object_cast<int16>(lua_table["quantity"]);
	const uint32 augment_one     = luabind::type(lua_table["augment_one"]) != LUA_TNIL ? luabind::object_cast<uint32>(lua_table["augment_one"]) : 0;
	const uint32 augment_two     = luabind::type(lua_table["augment_two"]) != LUA_TNIL ? luabind::object_cast<uint32>(lua_table["augment_two"]) : 0;
	const uint32 augment_three   = luabind::type(lua_table["augment_three"]) != LUA_TNIL ? luabind::object_cast<uint32>(lua_table["augment_three"]) : 0;
	const uint32 augment_four    = luabind::type(lua_table["augment_four"]) != LUA_TNIL ? luabind::object_cast<uint32>(lua_table["augment_four"]) : 0;
	const uint32 augment_five    = luabind::type(lua_table["augment_five"]) != LUA_TNIL ? luabind::object_cast<uint32>(lua_table["augment_five"]) : 0;
	const uint32 augment_six     = luabind::type(lua_table["augment_six"]) != LUA_TNIL ? luabind::object_cast<uint32>(lua_table["augment_six"]) : 0;
	const std::string& from_name = luabind::type(lua_table["from_name"]) != LUA_TNIL ? luabind::object_cast<std::string>(lua_table["from_name"]) : std::string();
	const std::string& note      = luabind::type(lua_table["note"]) != LUA_TNIL ? luabind::object_cast<std::string>(lua_table["note"]) : std::string();

	auto e = CharacterParcelsRepository::NewEntity();

	e.char_id    = character_id;
	e.item_id    = item_id;
	e.aug_slot_1 = augment_one;
	e.aug_slot_2 = augment_two;
	e.aug_slot_3 = augment_three;
	e.aug_slot_4 = augment_four;
	e.aug_slot_5 = augment_five;
	e.aug_slot_6 = augment_six;
	e.slot_id    = next_parcel_slot;
	e.quantity   = quantity;
	e.from_name  = from_name;
	e.note       = note;
	e.sent_date  = std::time(nullptr);

	auto out = CharacterParcelsRepository::InsertOne(database, e).id;
	if (out) {
		Parcel_Struct ps{};
		ps.item_slot = e.slot_id;
		strn0cpy(ps.send_to, name.c_str(), sizeof(ps.send_to));

		std::unique_ptr<ServerPacket> server_packet(new ServerPacket(ServerOP_ParcelDelivery, sizeof(Parcel_Struct)));
		auto                          data = (Parcel_Struct *) server_packet->pBuffer;

		data->item_slot = ps.item_slot;
		strn0cpy(data->send_to, ps.send_to, sizeof(data->send_to));

		worldserver.SendPacket(server_packet.get());
	}

	return out;
}

uint32 lua_get_zone_uptime()
{
	return Timer::GetCurrentTime() / 1000;
}

int lua_are_tasks_completed(luabind::object task_ids)
{
	if (luabind::type(task_ids) != LUA_TTABLE) {
		return 0;
	}

	std::vector<int> v;
	int index = 1;
	while (luabind::type(task_ids[index]) != LUA_TNIL) {
		auto current_id = task_ids[index];
		int task_id = 0;
		if (luabind::type(current_id) != LUA_TNIL) {
			try {
				task_id = luabind::object_cast<int>(current_id);
			} catch(luabind::cast_failed &) {
			}
		} else {
			break;
		}

		v.push_back(task_id);
		++index;
	}

	if (v.empty()) {
		return 0;
	}

	return quest_manager.aretaskscompleted(v);
}

void lua_spawn_circle(uint32 npc_id, float x, float y, float z, float heading, float radius, uint32 points)
{
	quest_manager.SpawnCircle(npc_id, glm::vec4(x, y, z, heading), radius, points);
}

void lua_spawn_grid(uint32 npc_id, float x, float y, float z, float heading, float spacing, uint32 spawn_count)
{
	quest_manager.SpawnGrid(npc_id, glm::vec4(x, y, z, heading), spacing, spawn_count);
}

Lua_Zone lua_get_zone()
{
	return Lua_Zone(zone);
}

bool lua_handin(luabind::adl::object handin_table)
{
	std::map<std::string, uint32> handin_map;

	for (luabind::iterator i(handin_table), end; i != end; i++) {
		std::string key;
		if (luabind::type(i.key()) == LUA_TSTRING) {
			key = luabind::object_cast<std::string>(i.key());
		}
		else if (luabind::type(i.key()) == LUA_TNUMBER) {
			key = fmt::format("{}", luabind::object_cast<int>(i.key()));
		}
		else {
			LogError("Handin key type [{}] not supported", luabind::type(i.key()));
		}

		if (!key.empty()) {
			handin_map[key] = luabind::object_cast<uint32>(handin_table[i.key()]);
			LogNpcHandinDetail("Handin key [{}] value [{}]", key, handin_map[key]);
		}
	}

	return quest_manager.handin(handin_map);
}

luabind::object lua_get_paused_timers(lua_State* L, Mob* m) {
	auto t = luabind::newtable(L);
	auto v = quest_manager.GetPausedTimers(m);
	int  i = 1;

	for (const auto& e : v) {
		t[i] = e;
		i++;
	}

	return t;
}

luabind::object lua_get_timers(lua_State* L, Mob* m) {
	auto t = luabind::newtable(L);
	auto v = quest_manager.GetTimers(m);
	int  i = 1;

	for (const auto& e : v) {
		t[i] = e;
		i++;
	}

	return t;
}

std::string lua_get_pet_command_name(uint8 pet_command)
{
	return PetCommand::GetName(pet_command);
}

std::string lua_get_pet_type_name(uint8 pet_type)
{
	return PetType::GetName(pet_type);
}

#define LuaCreateNPCParse(name, c_type, default_value) do { \
	cur = table[#name]; \
	if(luabind::type(cur) != LUA_TNIL) { \
		try { \
			npc_type->name = luabind::object_cast<c_type>(cur); \
		} \
		catch(luabind::cast_failed &) { \
			npc_type->size = default_value; \
		} \
	} \
	else { \
		npc_type->size = default_value; \
	} \
} while(0)

#define LuaCreateNPCParseString(name, str_length, default_value) do { \
	cur = table[#name]; \
	if(luabind::type(cur) != LUA_TNIL) { \
		try { \
			std::string tmp = luabind::object_cast<std::string>(cur); \
			strncpy(npc_type->name, tmp.c_str(), str_length); \
		} \
		catch(luabind::cast_failed &) { \
			strncpy(npc_type->name, default_value, str_length); \
		} \
	} \
	else { \
		strncpy(npc_type->name, default_value, str_length); \
	} \
} while(0)

void lua_create_npc(luabind::adl::object table, float x, float y, float z, float heading) {
	if(luabind::type(table) != LUA_TTABLE) {
		return;
	}

	auto npc_type = new NPCType;
	memset(npc_type, 0, sizeof(NPCType));


	luabind::adl::index_proxy<luabind::adl::object> cur = table["name"];
	LuaCreateNPCParseString(name, 64, "_");
	LuaCreateNPCParseString(lastname, 64, "");
	LuaCreateNPCParse(current_hp, int32, 30);
	LuaCreateNPCParse(max_hp, int32, 30);
	LuaCreateNPCParse(size, float, 6.0f);
	LuaCreateNPCParse(runspeed, float, 1.25f);
	LuaCreateNPCParse(gender, uint8, 0);
	LuaCreateNPCParse(race, uint16, 1);
	LuaCreateNPCParse(class_, uint8, Class::Warrior);
	LuaCreateNPCParse(bodytype, uint8, 0);
	LuaCreateNPCParse(deity, uint8, 0);
	LuaCreateNPCParse(level, uint8, 1);
	LuaCreateNPCParse(npc_id, uint32, 1);
	LuaCreateNPCParse(texture, uint8, 0);
	LuaCreateNPCParse(helmtexture, uint8, 0);
	LuaCreateNPCParse(loottable_id, uint32, 0);
	LuaCreateNPCParse(npc_spells_id, uint32, 0);
	LuaCreateNPCParse(npc_spells_effects_id, uint32, 0);
	LuaCreateNPCParse(npc_faction_id, int32, 0);
	LuaCreateNPCParse(merchanttype, uint32, 0);
	LuaCreateNPCParse(alt_currency_type, uint32, 0);
	LuaCreateNPCParse(adventure_template, uint32, 0);
	LuaCreateNPCParse(trap_template, uint32, 0);
	LuaCreateNPCParse(light, uint8, 0);
	LuaCreateNPCParse(AC, uint32, 0);
	LuaCreateNPCParse(Mana, uint32, 0);
	LuaCreateNPCParse(ATK, uint32, 0);
	LuaCreateNPCParse(STR, uint32, 0);
	LuaCreateNPCParse(STA, uint32, 0);
	LuaCreateNPCParse(DEX, uint32, 0);
	LuaCreateNPCParse(AGI, uint32, 0);
	LuaCreateNPCParse(INT, uint32, 0);
	LuaCreateNPCParse(WIS, uint32, 0);
	LuaCreateNPCParse(CHA, uint32, 0);
	LuaCreateNPCParse(MR, int32, 0);
	LuaCreateNPCParse(FR, int32, 0);
	LuaCreateNPCParse(CR, int32, 0);
	LuaCreateNPCParse(PR, int32, 0);
	LuaCreateNPCParse(DR, int32, 0);
	LuaCreateNPCParse(Corrup, int32, 0);
	LuaCreateNPCParse(PhR, int32, 0);
	LuaCreateNPCParse(haircolor, uint8, 0);
	LuaCreateNPCParse(beardcolor, uint8, 0);
	LuaCreateNPCParse(eyecolor1, uint8, 0);
	LuaCreateNPCParse(eyecolor2, uint8, 0);
	LuaCreateNPCParse(hairstyle, uint8, 0);
	LuaCreateNPCParse(luclinface, uint8, 0);
	LuaCreateNPCParse(beard, uint8, 0);
	LuaCreateNPCParse(drakkin_heritage, uint32, 0);
	LuaCreateNPCParse(drakkin_tattoo, uint32, 0);
	LuaCreateNPCParse(drakkin_details, uint32, 0);
	LuaCreateNPCParse(armor_tint.Head.Color, uint32, 0);
	LuaCreateNPCParse(armor_tint.Chest.Color, uint32, 0);
	LuaCreateNPCParse(armor_tint.Arms.Color, uint32, 0);
	LuaCreateNPCParse(armor_tint.Wrist.Color, uint32, 0);
	LuaCreateNPCParse(armor_tint.Hands.Color, uint32, 0);
	LuaCreateNPCParse(armor_tint.Legs.Color, uint32, 0);
	LuaCreateNPCParse(armor_tint.Feet.Color, uint32, 0);
	LuaCreateNPCParse(armor_tint.Primary.Color, uint32, 0);
	LuaCreateNPCParse(armor_tint.Secondary.Color, uint32, 0);
	LuaCreateNPCParse(min_dmg, uint32, 2);
	LuaCreateNPCParse(max_dmg, uint32, 4);
	LuaCreateNPCParse(attack_count, int16, 0);
	LuaCreateNPCParseString(special_abilities, 512, "");
	LuaCreateNPCParse(d_melee_texture1, uint32, 0);
	LuaCreateNPCParse(d_melee_texture2, uint32, 0);
	LuaCreateNPCParseString(ammo_idfile, 30, "");
	LuaCreateNPCParse(prim_melee_type, uint8, 0);
	LuaCreateNPCParse(sec_melee_type, uint8, 0);
	LuaCreateNPCParse(ranged_type, uint8, 0);
	LuaCreateNPCParse(hp_regen, int32, 1);
	LuaCreateNPCParse(mana_regen, int32, 1);
	LuaCreateNPCParse(aggroradius, int32, 0);
	LuaCreateNPCParse(assistradius, int32, 0);
	LuaCreateNPCParse(see_invis, uint8, 0);
	LuaCreateNPCParse(see_invis_undead, bool, false);
	LuaCreateNPCParse(see_hide, bool, false);
	LuaCreateNPCParse(see_improved_hide, bool, false);
	LuaCreateNPCParse(qglobal, bool, false);
	LuaCreateNPCParse(npc_aggro, bool, false);
	LuaCreateNPCParse(spawn_limit, uint8, false);
	LuaCreateNPCParse(mount_color, uint8, false);
	LuaCreateNPCParse(attack_speed, float, 0);
	LuaCreateNPCParse(attack_delay, uint8, 30);
	LuaCreateNPCParse(accuracy_rating, int, 0);
	LuaCreateNPCParse(avoidance_rating, int, 0);
	LuaCreateNPCParse(findable, bool, false);
	LuaCreateNPCParse(trackable, bool, false);
	LuaCreateNPCParse(slow_mitigation, int16, 0);
	LuaCreateNPCParse(maxlevel, uint8, 0);
	LuaCreateNPCParse(scalerate, uint32, 0);
	LuaCreateNPCParse(private_corpse, bool, false);
	LuaCreateNPCParse(unique_spawn_by_name, bool, false);
	LuaCreateNPCParse(underwater, bool, false);
	LuaCreateNPCParse(emoteid, uint32, 0);
	LuaCreateNPCParse(spellscale, float, 0);
	LuaCreateNPCParse(healscale, float, 0);
	LuaCreateNPCParse(no_target_hotkey, bool, false);
	LuaCreateNPCParse(raid_target, bool, false);

	NPC* npc = new NPC(npc_type, nullptr, glm::vec4(x, y, z, heading), GravityBehavior::Water);
	npc->GiveNPCTypeData(npc_type);
	entity_list.AddNPC(npc);
}

int random_int(int low, int high) {
	return zone->random.Int(low, high);
}

double random_real(double low, double high) {
	return zone->random.Real(low, high);
}

bool random_roll_int(int required) {
	return zone->random.Roll(required);
}

bool random_roll_real(double required) {
	return zone->random.Roll(required);
}

int random_roll0(int max) {
	return zone->random.Roll0(max);
}

int get_rulei(int rule) {
	return RuleManager::Instance()->GetIntRule((RuleManager::IntType)rule);
}

float get_ruler(int rule) {
	return RuleManager::Instance()->GetRealRule((RuleManager::RealType)rule);
}

bool get_ruleb(int rule) {
	return RuleManager::Instance()->GetBoolRule((RuleManager::BoolType)rule);
}

std::string get_rules(int rule) {
	return RuleManager::Instance()->GetStringRule((RuleManager::StringType)rule);
}

luabind::scope lua_register_general() {
	return luabind::namespace_("eq")
	[(
		luabind::def("load_encounter", &load_encounter),
		luabind::def("unload_encounter", &unload_encounter),
		luabind::def("load_encounter_with_data", &load_encounter_with_data),
		luabind::def("unload_encounter_with_data", &unload_encounter_with_data),
		luabind::def("register_npc_event", (void(*)(std::string, int, int, luabind::adl::object))&register_npc_event),
		luabind::def("register_npc_event", (void(*)(int, int, luabind::adl::object))&register_npc_event),
		luabind::def("unregister_npc_event", (void(*)(std::string, int, int))&unregister_npc_event),
		luabind::def("unregister_npc_event", (void(*)(int, int))&unregister_npc_event),
		luabind::def("register_player_event", (void(*)(std::string, int, luabind::adl::object))&register_player_event),
		luabind::def("register_player_event", (void(*)(int, luabind::adl::object))&register_player_event),
		luabind::def("unregister_player_event", (void(*)(std::string, int))&unregister_player_event),
		luabind::def("unregister_player_event", (void(*)(int))&unregister_player_event),
		luabind::def("register_item_event", (void(*)(std::string, int, int, luabind::adl::object))&register_item_event),
		luabind::def("register_item_event", (void(*)(int, int, luabind::adl::object))&register_item_event),
		luabind::def("unregister_item_event", (void(*)(std::string, int, int))&unregister_item_event),
		luabind::def("unregister_item_event", (void(*)(int, int))&unregister_item_event),
		luabind::def("register_spell_event", (void(*)(std::string, int, int, luabind::adl::object func))&register_spell_event),
		luabind::def("register_spell_event", (void(*)(int, int, luabind::adl::object func))&register_spell_event),
		luabind::def("unregister_spell_event", (void(*)(std::string, int, int))&unregister_spell_event),
		luabind::def("unregister_spell_event", (void(*)(int, int))&unregister_spell_event),
		luabind::def("spawn2", (Lua_Mob(*)(int,int,int,double,double,double,double))&lua_spawn2),
		luabind::def("unique_spawn", (Lua_Mob(*)(int,int,int,double,double,double))&lua_unique_spawn),
		luabind::def("unique_spawn", (Lua_Mob(*)(int,int,int,double,double,double,double))&lua_unique_spawn),
		luabind::def("spawn_from_spawn2", (Lua_Mob(*)(uint32))&lua_spawn_from_spawn2),
		luabind::def("enable_spawn2", &lua_enable_spawn2),
		luabind::def("disable_spawn2", &lua_disable_spawn2),
		luabind::def("has_timer", (bool(*)(std::string))&lua_has_timer),
		luabind::def("get_remaining_time", (uint32(*)(std::string))&lua_get_remaining_time),
		luabind::def("get_timer_duration", (uint32(*)(std::string))&lua_get_timer_duration),
		luabind::def("set_timer", (void(*)(std::string, uint32))&lua_set_timer),
		luabind::def("set_timer", (void(*)(std::string, uint32, Lua_ItemInst))&lua_set_timer),
		luabind::def("set_timer", (void(*)(std::string, uint32, Lua_Mob))&lua_set_timer),
		luabind::def("set_timer", (void(*)(std::string, uint32, Lua_Encounter))&lua_set_timer),
		luabind::def("stop_timer", (void(*)(std::string))&lua_stop_timer),
		luabind::def("stop_timer", (void(*)(std::string, Lua_ItemInst))&lua_stop_timer),
		luabind::def("stop_timer", (void(*)(std::string, Lua_Mob))&lua_stop_timer),
		luabind::def("stop_timer", (void(*)(std::string, Lua_Encounter))&lua_stop_timer),
		luabind::def("pause_timer", (void(*)(std::string))&lua_pause_timer),
		luabind::def("resume_timer", (void(*)(std::string))&lua_resume_timer),
		luabind::def("is_paused_timer", (bool(*)(std::string))&lua_is_paused_timer),
		luabind::def("stop_all_timers", (void(*)(void))&lua_stop_all_timers),
		luabind::def("stop_all_timers", (void(*)(Lua_ItemInst))&lua_stop_all_timers),
		luabind::def("stop_all_timers", (void(*)(Lua_Mob))&lua_stop_all_timers),
		luabind::def("stop_all_timers", (void(*)(Lua_Encounter))&lua_stop_all_timers),
		luabind::def("depop", (void(*)(void))&lua_depop),
		luabind::def("depop", (void(*)(int))&lua_depop),
		luabind::def("depop_with_timer", (void(*)(void))&lua_depop_with_timer),
		luabind::def("depop_with_timer", (void(*)(int))&lua_depop_with_timer),
		luabind::def("depop_all", (void(*)(void))&lua_depop_all),
		luabind::def("depop_all", (void(*)(int))&lua_depop_all),
		luabind::def("depop_zone", &lua_depop_zone),
		luabind::def("repop_zone", (void(*)(void))&lua_repop_zone),
		luabind::def("repop_zone", (void(*)(bool))&lua_repop_zone),
		luabind::def("process_mobs_while_zone_empty", &lua_process_mobs_while_zone_empty),
		luabind::def("is_disc_tome", &lua_is_disc_tome),
		luabind::def("get_race_name", (std::string(*)(uint16))&lua_get_race_name),
		luabind::def("get_spell_name", (std::string(*)(uint32))&lua_get_spell_name),
		luabind::def("get_skill_name", (std::string(*)(int))&lua_get_skill_name),
		luabind::def("safe_move", &lua_safe_move),
		luabind::def("rain", &lua_rain),
		luabind::def("snow", &lua_snow),
		luabind::def("scribe_spells", (int(*)(int))&lua_scribe_spells),
		luabind::def("scribe_spells", (int(*)(int,int))&lua_scribe_spells),
		luabind::def("train_discs", (int(*)(int))&lua_train_discs),
		luabind::def("train_discs", (int(*)(int,int))&lua_train_discs),
		luabind::def("set_sky", &lua_set_sky),
		luabind::def("set_guild", &lua_set_guild),
		luabind::def("create_guild", &lua_create_guild),
		luabind::def("set_time", (void(*)(int, int))&lua_set_time),
		luabind::def("set_time", (void(*)(int, int, bool))&lua_set_time),
		luabind::def("signal", (void(*)(int,int))&lua_signal),
		luabind::def("signal", (void(*)(int,int,int))&lua_signal),
		luabind::def("set_global", &lua_set_global),
		luabind::def("target_global", &lua_target_global),
		luabind::def("delete_global", &lua_delete_global),
		luabind::def("start", &lua_start),
		luabind::def("stop", &lua_stop),
		luabind::def("pause", &lua_pause),
		luabind::def("move_to", (void(*)(float,float,float))&lua_move_to),
		luabind::def("move_to", (void(*)(float,float,float,float))&lua_move_to),
		luabind::def("move_to", (void(*)(float,float,float,float,bool))&lua_move_to),
		luabind::def("resume", &lua_path_resume),
		luabind::def("set_next_hp_event", &lua_set_next_hp_event),
		luabind::def("set_next_inc_hp_event", &lua_set_next_inc_hp_event),
		luabind::def("respawn", &lua_respawn),
		luabind::def("set_proximity", (void(*)(float,float,float,float))&lua_set_proximity),
		luabind::def("set_proximity", (void(*)(float,float,float,float,float,float))&lua_set_proximity),
		luabind::def("set_proximity", (void(*)(float,float,float,float,float,float,bool))&lua_set_proximity),
		luabind::def("set_proximity_range", (void(*)(float,float))&lua_set_proximity_range),
		luabind::def("set_proximity_range", (void(*)(float,float,float))&lua_set_proximity_range),
		luabind::def("set_proximity_range", (void(*)(float,float,float,bool))&lua_set_proximity_range),
		luabind::def("clear_proximity", &lua_clear_proximity),
		luabind::def("enable_proximity_say", &lua_enable_proximity_say),
		luabind::def("disable_proximity_say", &lua_disable_proximity_say),
		luabind::def("set_anim", &lua_set_anim),
		luabind::def("spawn_condition", &lua_spawn_condition),
		luabind::def("get_spawn_condition", &lua_get_spawn_condition),
		luabind::def("toggle_spawn_event", &lua_toggle_spawn_event),
		luabind::def("summon_buried_player_corpse", &lua_summon_buried_player_corpse),
		luabind::def("summon_all_player_corpses", &lua_summon_all_player_corpses),
		luabind::def("get_player_corpse_count", &lua_get_player_corpse_count),
		luabind::def("get_player_corpse_count_by_zone_id", &lua_get_player_corpse_count_by_zone_id),
		luabind::def("get_player_buried_corpse_count", &lua_get_player_buried_corpse_count),
		luabind::def("bury_player_corpse", &lua_bury_player_corpse),
		luabind::def("task_selector", (void(*)(luabind::adl::object))&lua_task_selector),
		luabind::def("task_selector", (void(*)(luabind::adl::object, bool))&lua_task_selector),
		luabind::def("task_set_selector", (void(*)(int))&lua_task_set_selector),
		luabind::def("task_set_selector", (void(*)(int, bool))&lua_task_set_selector),
		luabind::def("enable_task", &lua_enable_task),
		luabind::def("disable_task", &lua_disable_task),
		luabind::def("is_task_enabled", &lua_is_task_enabled),
		luabind::def("is_task_active", &lua_is_task_active),
		luabind::def("is_task_activity_active", &lua_is_task_activity_active),
		luabind::def("get_task_activity_done_count", &lua_get_task_activity_done_count),
		luabind::def("update_task_activity", &lua_update_task_activity),
		luabind::def("reset_task_activity", &lua_reset_task_activity),
		luabind::def("assign_task", &lua_assign_task),
		luabind::def("fail_task", &lua_fail_task),
		luabind::def("complete_task", &lua_complete_task),
		luabind::def("uncomplete_task", &lua_uncomplete_task),
		luabind::def("task_time_left", &lua_task_time_left),
		luabind::def("is_task_completed", &lua_is_task_completed),
		luabind::def("enabled_task_count", &lua_enabled_task_count),
		luabind::def("first_task_in_set", &lua_first_task_in_set),
		luabind::def("last_task_in_set", &lua_last_task_in_set),
		luabind::def("next_task_in_set", &lua_next_task_in_set),
		luabind::def("active_speak_task", &lua_active_speak_task),
		luabind::def("active_speak_activity", &lua_active_speak_activity),
		luabind::def("active_tasks_in_set", &lua_active_tasks_in_set),
		luabind::def("completed_tasks_in_set", &lua_completed_tasks_in_set),
		luabind::def("is_task_appropriate", &lua_is_task_appropriate),
		luabind::def("get_task_name", (std::string(*)(uint32))&lua_get_task_name),
		luabind::def("get_dz_task_id", &lua_get_dz_task_id),
		luabind::def("end_dz_task", (void(*)())&lua_end_dz_task),
		luabind::def("end_dz_task", (void(*)(bool))&lua_end_dz_task),
		luabind::def("clear_spawn_timers", &lua_clear_spawn_timers),
		luabind::def("zone_emote", &lua_zone_emote),
		luabind::def("world_emote", &lua_world_emote),
		luabind::def("message", &lua_message),
		luabind::def("whisper", &lua_whisper),
		luabind::def("get_level", &lua_get_level),
		luabind::def("create_ground_object", (uint16(*)(uint32,float,float,float,float))&lua_create_ground_object),
		luabind::def("create_ground_object", (uint16(*)(uint32,float,float,float,float,uint32))&lua_create_ground_object),
		luabind::def("create_ground_object_from_model", (uint16(*)(const char*,float,float,float,float))&lua_create_ground_object_from_model),
		luabind::def("create_ground_object_from_model", (uint16(*)(const char*,float,float,float,float,uint8))&lua_create_ground_object_from_model),
		luabind::def("create_ground_object_from_model", (uint16(*)(const char*,float,float,float,float,uint8,uint32))&lua_create_ground_object_from_model),
		luabind::def("create_door", (uint16(*)(const char*,float,float,float,float))&lua_create_door),
		luabind::def("create_door", (uint16(*)(const char*,float,float,float,float,uint8))&lua_create_door),
		luabind::def("create_door", (uint16(*)(const char*,float,float,float,float,uint8,uint16))&lua_create_door),
		luabind::def("modify_npc_stat", &lua_modify_npc_stat),
		luabind::def("collect_items", &lua_collect_items),
		luabind::def("count_item", &lua_count_item),
		luabind::def("remove_item", (void(*)(uint32))&lua_remove_item),
		luabind::def("remove_item", (void(*)(uint32,uint32))&lua_remove_item),
		luabind::def("update_spawn_timer", &lua_update_spawn_timer),
		luabind::def("merchant_set_item", (void(*)(uint32,uint32))&lua_merchant_set_item),
		luabind::def("merchant_set_item", (void(*)(uint32,uint32,uint32))&lua_merchant_set_item),
		luabind::def("merchant_count_item", &lua_merchant_count_item),
		luabind::def("item_link", (std::string(*)(Lua_ItemInst))&lua_item_link),
		luabind::def("item_link", (std::string(*)(uint32))&lua_item_link),
		luabind::def("item_link", (std::string(*)(uint32,int16))&lua_item_link),
		luabind::def("item_link", (std::string(*)(uint32,int16,uint32))&lua_item_link),
		luabind::def("item_link", (std::string(*)(uint32,int16,uint32,uint32))&lua_item_link),
		luabind::def("item_link", (std::string(*)(uint32,int16,uint32,uint32,uint32))&lua_item_link),
		luabind::def("item_link", (std::string(*)(uint32,int16,uint32,uint32,uint32,uint32))&lua_item_link),
		luabind::def("item_link", (std::string(*)(uint32,int16,uint32,uint32,uint32,uint32,uint32))&lua_item_link),
		luabind::def("item_link", (std::string(*)(uint32,int16,uint32,uint32,uint32,uint32,uint32,uint32))&lua_item_link),
		luabind::def("item_link", (std::string(*)(uint32,int16,uint32,uint32,uint32,uint32,uint32,uint32,bool))&lua_item_link),
		luabind::def("get_item_comment", (std::string(*)(uint32))&lua_get_item_comment),
		luabind::def("get_item_lore", (std::string(*)(uint32))&lua_get_item_lore),
		luabind::def("get_item_name", (std::string(*)(uint32))&lua_get_item_name),
		luabind::def("say_link", (std::string(*)(std::string))&lua_say_link),
		luabind::def("say_link", (std::string(*)(std::string,bool))&lua_say_link),
		luabind::def("say_link", (std::string(*)(std::string,bool,std::string))&lua_say_link),
		luabind::def("set_rule", (void(*)(std::string, std::string))&lua_set_rule),
		luabind::def("get_rule", (std::string(*)(std::string))&lua_get_rule),
		luabind::def("get_data", (std::string(*)(std::string))&lua_get_data),
		luabind::def("get_data_expires", (std::string(*)(std::string))&lua_get_data_expires),
		luabind::def("set_data", (void(*)(std::string, std::string))&lua_set_data),
		luabind::def("set_data", (void(*)(std::string, std::string, std::string))&lua_set_data),
		luabind::def("delete_data", (bool(*)(std::string))&lua_delete_data),
		luabind::def("get_char_name_by_id", &lua_get_char_name_by_id),
		luabind::def("get_char_id_by_name", (uint32(*)(const char*))&lua_get_char_id_by_name),
		luabind::def("get_class_name", (std::string(*)(uint8))&lua_get_class_name),
		luabind::def("get_class_name", (std::string(*)(uint8,uint8))&lua_get_class_name),
		luabind::def("get_clean_npc_name_by_id", &lua_get_clean_npc_name_by_id),
		luabind::def("get_currency_id", &lua_get_currency_id),
		luabind::def("get_currency_item_id", &lua_get_currency_item_id),
		luabind::def("get_guild_name_by_id", &lua_get_guild_name_by_id),
		luabind::def("get_guild_id_by_char_id", &lua_get_guild_id_by_char_id),
		luabind::def("get_group_id_by_char_id", &lua_get_group_id_by_char_id),
		luabind::def("get_npc_name_by_id", &lua_get_npc_name_by_id),
		luabind::def("get_raid_id_by_char_id", &lua_get_raid_id_by_char_id),
		luabind::def("create_instance", &lua_create_instance),
		luabind::def("destroy_instance", &lua_destroy_instance),
		luabind::def("update_instance_timer", &lua_update_instance_timer),
		luabind::def("get_instance_id", &lua_get_instance_id),
		luabind::def("get_instance_id_by_char_id", &lua_get_instance_id_by_char_id),
		luabind::def("get_instance_ids", &lua_get_instance_ids),
		luabind::def("get_instance_ids_by_char_id", &lua_get_instance_ids_by_char_id),
		luabind::def("get_instance_timer", &lua_get_instance_timer),
		luabind::def("get_instance_timer_by_id", &lua_get_instance_timer_by_id),
		luabind::def("get_instance_version_by_id", &lua_get_instance_version_by_id),
		luabind::def("get_instance_zone_id_by_id", &lua_get_instance_zone_id_by_id),
		luabind::def("get_characters_in_instance", &lua_get_characters_in_instance),
		luabind::def("assign_to_instance", &lua_assign_to_instance),
		luabind::def("assign_to_instance_by_char_id", &lua_assign_to_instance_by_char_id),
		luabind::def("assign_group_to_instance", &lua_assign_group_to_instance),
		luabind::def("assign_raid_to_instance", &lua_assign_raid_to_instance),
		luabind::def("remove_from_instance", &lua_remove_from_instance),
		luabind::def("remove_from_instance_by_char_id", &lua_remove_from_instance_by_char_id),
		luabind::def("check_instance_by_char_id", (bool(*)(uint16, uint32))&lua_check_instance_by_char_id),
		luabind::def("remove_all_from_instance", &lua_remove_all_from_instance),
		luabind::def("flag_instance_by_group_leader", &lua_flag_instance_by_group_leader),
		luabind::def("flag_instance_by_raid_leader", &lua_flag_instance_by_raid_leader),
		luabind::def("fly_mode", &lua_fly_mode),
		luabind::def("faction_value", &lua_faction_value),
		luabind::def("check_title", &lua_check_title),
		luabind::def("enable_title", &lua_enable_title),
		luabind::def("remove_title", &lua_remove_title),
		luabind::def("wear_change", &lua_wear_change),
		luabind::def("voice_tell", &lua_voice_tell),
		luabind::def("send_mail", &lua_send_mail),
		luabind::def("get_qglobals", (luabind::adl::object(*)(lua_State*,Lua_NPC,Lua_Client))&lua_get_qglobals),
		luabind::def("get_qglobals", (luabind::adl::object(*)(lua_State*,Lua_Client))&lua_get_qglobals),
		luabind::def("get_qglobals", (luabind::adl::object(*)(lua_State*,Lua_NPC))&lua_get_qglobals),
		luabind::def("get_qglobals", (luabind::adl::object(*)(lua_State*))&lua_get_qglobals),
		luabind::def("get_entity_list", &lua_get_entity_list),
		luabind::def("zone", &lua_zone),
		luabind::def("zone_group", &lua_zone_group),
		luabind::def("zone_raid", &lua_zone_raid),
		luabind::def("get_zone_id", &lua_get_zone_id),
		luabind::def("get_zone_id_by_name", &lua_get_zone_id_by_name),
		luabind::def("get_zone_long_name", &lua_get_zone_long_name),
		luabind::def("get_zone_long_name_by_name", &lua_get_zone_long_name_by_name),
		luabind::def("get_zone_long_name_by_id", &lua_get_zone_long_name_by_id),
		luabind::def("get_zone_short_name", &lua_get_zone_short_name),
		luabind::def("get_zone_short_name_by_id", &lua_get_zone_short_name_by_id),
		luabind::def("get_zone_instance_id", &lua_get_zone_instance_id),
		luabind::def("get_zone_instance_version", &lua_get_zone_instance_version),
		luabind::def("get_zone_weather", &lua_get_zone_weather),
		luabind::def("get_zone_time", &lua_get_zone_time),
		luabind::def("add_area", &lua_add_area),
		luabind::def("remove_area", &lua_remove_area),
		luabind::def("clear_areas", &lua_clear_areas),
		luabind::def("add_spawn_point", &lua_add_spawn_point),
		luabind::def("remove_spawn_point", &lua_remove_spawn_point),
		luabind::def("attack", &lua_attack),
		luabind::def("attack_npc", &lua_attack_npc),
		luabind::def("attack_npc_type", &lua_attack_npc_type),
		luabind::def("follow", (void(*)(int))&lua_follow),
		luabind::def("follow", (void(*)(int,int))&lua_follow),
		luabind::def("stop_follow", &lua_stop_follow),
		luabind::def("get_initiator", &lua_get_initiator),
		luabind::def("get_owner", &lua_get_owner),
		luabind::def("get_quest_item", &lua_get_quest_item),
		luabind::def("get_quest_spell", &lua_get_quest_spell),
		luabind::def("get_encounter", &lua_get_encounter),
		luabind::def("map_opcodes", &lua_map_opcodes),
		luabind::def("clear_opcode", &lua_clear_opcode),
		luabind::def("enable_recipe", &lua_enable_recipe),
		luabind::def("disable_recipe", &lua_disable_recipe),
		luabind::def("clear_npctype_cache", &lua_clear_npctype_cache),
		luabind::def("reloadzonestaticdata", &lua_reloadzonestaticdata),
		luabind::def("update_zone_header", &lua_update_zone_header),
		luabind::def("clock", &lua_clock),
		luabind::def("create_npc", &lua_create_npc),
		luabind::def("log", (void(*)(int, std::string))&lua_log),
		luabind::def("debug", (void(*)(std::string))&lua_debug),
		luabind::def("debug", (void(*)(std::string, int))&lua_debug),
		luabind::def("log_combat", (void(*)(std::string))&lua_log_combat),
		luabind::def("log_spells", (void(*)(std::string))&lua_log_spells),
		luabind::def("seconds_to_time", &lua_seconds_to_time),
		luabind::def("time_to_seconds", &lua_time_to_seconds),
		luabind::def("get_hex_color_code", &lua_get_hex_color_code),
		luabind::def("get_aa_exp_modifier_by_char_id", (float(*)(uint32,uint32))&lua_get_aa_exp_modifier_by_char_id),
		luabind::def("get_aa_exp_modifier_by_char_id", (float(*)(uint32,uint32,int16))&lua_get_aa_exp_modifier_by_char_id),
		luabind::def("get_exp_modifier_by_char_id", (float(*)(uint32,uint32))&lua_get_exp_modifier_by_char_id),
		luabind::def("get_exp_modifier_by_char_id", (float(*)(uint32,uint32,int16))&lua_get_exp_modifier_by_char_id),
		luabind::def("set_aa_exp_modifier_by_char_id", (void(*)(uint32,uint32,float))&lua_set_aa_exp_modifier_by_char_id),
		luabind::def("set_aa_exp_modifier_by_char_id", (void(*)(uint32,uint32,float,int16))&lua_set_aa_exp_modifier_by_char_id),
		luabind::def("set_exp_modifier_by_char_id", (void(*)(uint32,uint32,float))&lua_set_exp_modifier_by_char_id),
		luabind::def("set_exp_modifier_by_char_id", (void(*)(uint32,uint32,float,int16))&lua_set_exp_modifier_by_char_id),
		luabind::def("add_ldon_loss", &lua_add_ldon_loss),
		luabind::def("add_ldon_points", &lua_add_ldon_points),
		luabind::def("add_ldon_win", &lua_add_ldon_win),
		luabind::def("remove_ldon_loss", &lua_remove_ldon_loss),
		luabind::def("remove_ldon_win", &lua_remove_ldon_win),
		luabind::def("get_gender_name", &lua_get_gender_name),
		luabind::def("get_deity_name", &lua_get_deity_name),
		luabind::def("get_inventory_slot_name", &lua_get_inventory_slot_name),
		luabind::def("rename", &lua_rename),
		luabind::def("get_data_remaining", &lua_get_data_remaining),
		luabind::def("get_item_stat", &lua_get_item_stat),
		luabind::def("get_spell_stat", (int(*)(uint32,std::string))&lua_get_spell_stat),
		luabind::def("get_spell_stat", (int(*)(uint32,std::string,uint8))&lua_get_spell_stat),
		luabind::def("is_npc_spawned", &lua_is_npc_spawned),
		luabind::def("count_spawned_npcs", &lua_count_spawned_npcs),
		luabind::def("get_spell", &lua_get_spell),
		luabind::def("get_ldon_theme_name", &lua_get_ldon_theme_name),
		luabind::def("get_faction_name", &lua_get_faction_name),
		luabind::def("get_language_name", &lua_get_language_name),
		luabind::def("get_body_type_name", &lua_get_body_type_name),
		luabind::def("get_consider_level_name", &lua_get_consider_level_name),
		luabind::def("get_environmental_damage_name", &lua_get_environmental_damage_name),
		luabind::def("commify", &lua_commify),
		luabind::def("check_name_filter", &lua_check_name_filter),
		luabind::def("discord_send", &lua_discord_send),
		luabind::def("track_npc", &lua_track_npc),
		luabind::def("get_recipe_made_count", &lua_get_recipe_made_count),
		luabind::def("get_recipe_name", &lua_get_recipe_name),
		luabind::def("has_recipe_learned", &lua_has_recipe_learned),
		luabind::def("is_raining", &lua_is_raining),
		luabind::def("is_snowing", &lua_is_snowing),
		luabind::def("get_aa_name", &lua_get_aa_name),
		luabind::def("popup", (void(*)(const char*,const char*))&lua_popup),
		luabind::def("popup", (void(*)(const char*,const char*,uint32))&lua_popup),
		luabind::def("popup", (void(*)(const char*,const char*,uint32,uint32))&lua_popup),
		luabind::def("popup", (void(*)(const char*,const char*,uint32,uint32,uint32))&lua_popup),
		luabind::def("popup_break", (std::string(*)(void))&lua_popup_break),
		luabind::def("popup_break", (std::string(*)(uint32))&lua_popup_break),
		luabind::def("popup_center_message", &lua_popup_center_message),
		luabind::def("popup_color_message", &lua_popup_color_message),
		luabind::def("popup_indent", (std::string(*)(void))&lua_popup_indent),
		luabind::def("popup_indent", (std::string(*)(uint32))&lua_popup_indent),
		luabind::def("popup_link", (std::string(*)(std::string))&lua_popup_link),
		luabind::def("popup_link", (std::string(*)(std::string,std::string))&lua_popup_link),
		luabind::def("popup_table", &lua_popup_table),
		luabind::def("popup_table_cell", (std::string(*)(void))&lua_popup_table_cell),
		luabind::def("popup_table_cell", (std::string(*)(std::string))&lua_popup_table_cell),
		luabind::def("popup_table_row", &lua_popup_table_row),
		luabind::def("marquee", (void(*)(uint32,std::string))&lua_marquee),
		luabind::def("marquee", (void(*)(uint32,std::string,uint32))&lua_marquee),
		luabind::def("marquee", (void(*)(uint32,uint32,uint32,uint32,uint32,std::string))&lua_marquee),
		luabind::def("zone_marquee", (void(*)(uint32,std::string))&lua_zone_marquee),
		luabind::def("zone_marquee", (void(*)(uint32,std::string,uint32))&lua_zone_marquee),
		luabind::def("zone_marquee", (void(*)(uint32,uint32,uint32,uint32,uint32,std::string))&lua_zone_marquee),
		luabind::def("is_hotzone", (bool(*)(void))&lua_is_hotzone),
		luabind::def("set_hotzone", (void(*)(bool))&lua_set_hotzone),
		luabind::def("do_anim", (void(*)(int))&lua_do_anim),
		luabind::def("do_anim", (void(*)(int,int))&lua_do_anim),
		luabind::def("do_anim", (void(*)(int,int,bool))&lua_do_anim),
		luabind::def("do_anim", (void(*)(int,int,bool,int))&lua_do_anim),
		luabind::def("do_augment_slots_match", &lua_do_augment_slots_match),
		luabind::def("does_augment_fit", (int8(*)(Lua_ItemInst, uint32))&lua_does_augment_fit),
		luabind::def("does_augment_fit_slot", (int8(*)(Lua_ItemInst, uint32, uint8))&lua_does_augment_fit_slot),
		luabind::def("get_recipe_component_item_ids", (luabind::object(*)(lua_State*,uint32))&lua_get_recipe_component_item_ids),
		luabind::def("get_recipe_container_item_ids", (luabind::object(*)(lua_State*,uint32))&lua_get_recipe_container_item_ids),
		luabind::def("get_recipe_fail_item_ids", (luabind::object(*)(lua_State*,uint32))&lua_get_recipe_fail_item_ids),
		luabind::def("get_recipe_salvage_item_ids", (luabind::object(*)(lua_State*,uint32))&lua_get_recipe_salvage_item_ids),
		luabind::def("get_recipe_success_item_ids", (luabind::object(*)(lua_State*,uint32))&lua_get_recipe_success_item_ids),
		luabind::def("get_recipe_component_count", (int8(*)(uint32,uint32))&lua_get_recipe_component_count),
		luabind::def("get_recipe_fail_count", (int8(*)(uint32,uint32))&lua_get_recipe_fail_count),
		luabind::def("get_recipe_salvage_count", (int8(*)(uint32,uint32))&lua_get_recipe_salvage_count),
		luabind::def("get_recipe_success_count", (int8(*)(uint32,uint32))&lua_get_recipe_success_count),
		luabind::def("send_player_handin_event", (void(*)(void))&lua_send_player_handin_event),
		luabind::def("get_zone_safe_x", (float(*)(uint32))&lua_get_zone_safe_x),
		luabind::def("get_zone_safe_x", (float(*)(uint32,int))&lua_get_zone_safe_x),
		luabind::def("get_zone_safe_y", (float(*)(uint32))&lua_get_zone_safe_y),
		luabind::def("get_zone_safe_y", (float(*)(uint32,int))&lua_get_zone_safe_y),
		luabind::def("get_zone_safe_z", (float(*)(uint32))&lua_get_zone_safe_z),
		luabind::def("get_zone_safe_z", (float(*)(uint32,int))&lua_get_zone_safe_z),
		luabind::def("get_zone_safe_heading", (float(*)(uint32))&lua_get_zone_safe_heading),
		luabind::def("get_zone_safe_heading", (float(*)(uint32,int))&lua_get_zone_safe_heading),
		luabind::def("get_zone_graveyard_id", (float(*)(uint32))&lua_get_zone_graveyard_id),
		luabind::def("get_zone_graveyard_id", (float(*)(uint32,int))&lua_get_zone_graveyard_id),
		luabind::def("get_zone_minimum_level", (uint8(*)(uint32))&lua_get_zone_minimum_level),
		luabind::def("get_zone_minimum_level", (uint8(*)(uint32,int))&lua_get_zone_minimum_level),
		luabind::def("get_zone_maximum_level", (uint8(*)(uint32))&lua_get_zone_maximum_level),
		luabind::def("get_zone_maximum_level", (uint8(*)(uint32,int))&lua_get_zone_maximum_level),
		luabind::def("get_zone_minimum_status", (uint8(*)(uint32))&lua_get_zone_minimum_status),
		luabind::def("get_zone_minimum_status", (uint8(*)(uint32,int))&lua_get_zone_minimum_status),
		luabind::def("get_zone_time_zone", (int(*)(uint32))&lua_get_zone_time_zone),
		luabind::def("get_zone_time_zone", (int(*)(uint32,int))&lua_get_zone_time_zone),
		luabind::def("get_zone_maximum_players", (int(*)(uint32))&lua_get_zone_maximum_players),
		luabind::def("get_zone_maximum_players", (int(*)(uint32,int))&lua_get_zone_maximum_players),
		luabind::def("get_zone_rule_set", (uint32(*)(uint32))&lua_get_zone_rule_set),
		luabind::def("get_zone_rule_set", (uint32(*)(uint32,int))&lua_get_zone_rule_set),
		luabind::def("get_zone_note", (std::string(*)(uint32))&lua_get_zone_note),
		luabind::def("get_zone_note", (std::string(*)(uint32,int))&lua_get_zone_note),
		luabind::def("get_zone_underworld", (float(*)(uint32))&lua_get_zone_underworld),
		luabind::def("get_zone_underworld", (float(*)(uint32,int))&lua_get_zone_underworld),
		luabind::def("get_zone_minimum_clip", (float(*)(uint32))&lua_get_zone_minimum_clip),
		luabind::def("get_zone_minimum_clip", (float(*)(uint32,int))&lua_get_zone_minimum_clip),
		luabind::def("get_zone_maximum_clip", (float(*)(uint32))&lua_get_zone_maximum_clip),
		luabind::def("get_zone_maximum_clip", (float(*)(uint32,int))&lua_get_zone_maximum_clip),
		luabind::def("get_zone_fog_minimum_clip", (float(*)(uint32))&lua_get_zone_fog_minimum_clip),
		luabind::def("get_zone_fog_minimum_clip", (float(*)(uint32,uint8))&lua_get_zone_fog_minimum_clip),
		luabind::def("get_zone_fog_minimum_clip", (float(*)(uint32,uint8,int))&lua_get_zone_fog_minimum_clip),
		luabind::def("get_zone_fog_maximum_clip", (float(*)(uint32))&lua_get_zone_fog_maximum_clip),
		luabind::def("get_zone_fog_maximum_clip", (float(*)(uint32,uint8))&lua_get_zone_fog_maximum_clip),
		luabind::def("get_zone_fog_maximum_clip", (float(*)(uint32,uint8,int))&lua_get_zone_fog_maximum_clip),
		luabind::def("get_zone_fog_red", (uint8(*)(uint32))&lua_get_zone_fog_red),
		luabind::def("get_zone_fog_red", (uint8(*)(uint32,uint8))&lua_get_zone_fog_red),
		luabind::def("get_zone_fog_red", (uint8(*)(uint32,uint8,int))&lua_get_zone_fog_red),
		luabind::def("get_zone_fog_green", (uint8(*)(uint32))&lua_get_zone_fog_green),
		luabind::def("get_zone_fog_green", (uint8(*)(uint32,uint8))&lua_get_zone_fog_green),
		luabind::def("get_zone_fog_green", (uint8(*)(uint32,uint8,int))&lua_get_zone_fog_green),
		luabind::def("get_zone_fog_blue", (uint8(*)(uint32))&lua_get_zone_fog_blue),
		luabind::def("get_zone_fog_blue", (uint8(*)(uint32,uint8))&lua_get_zone_fog_blue),
		luabind::def("get_zone_fog_blue", (uint8(*)(uint32,uint8,int))&lua_get_zone_fog_blue),
		luabind::def("get_zone_sky", (uint8(*)(uint32))&lua_get_zone_sky),
		luabind::def("get_zone_sky", (uint8(*)(uint32,int))&lua_get_zone_sky),
		luabind::def("get_zone_ztype", (uint8(*)(uint32))&lua_get_zone_ztype),
		luabind::def("get_zone_ztype", (uint8(*)(uint32,int))&lua_get_zone_ztype),
		luabind::def("get_zone_experience_multiplier", (float(*)(uint32))&lua_get_zone_experience_multiplier),
		luabind::def("get_zone_experience_multiplier", (float(*)(uint32,int))&lua_get_zone_experience_multiplier),
		luabind::def("get_zone_walk_speed", (float(*)(uint32))&lua_get_zone_walk_speed),
		luabind::def("get_zone_walk_speed", (float(*)(uint32,int))&lua_get_zone_walk_speed),
		luabind::def("get_zone_time_type", (uint8(*)(uint32))&lua_get_zone_time_type),
		luabind::def("get_zone_time_type", (uint8(*)(uint32,int))&lua_get_zone_time_type),
		luabind::def("get_zone_fog_density", (float(*)(uint32))&lua_get_zone_fog_density),
		luabind::def("get_zone_fog_density", (float(*)(uint32,int))&lua_get_zone_fog_density),
		luabind::def("get_zone_flag_needed", (std::string(*)(uint32))&lua_get_zone_flag_needed),
		luabind::def("get_zone_flag_needed", (std::string(*)(uint32,int))&lua_get_zone_flag_needed),
		luabind::def("get_zone_can_bind", (int8(*)(uint32))&lua_get_zone_can_bind),
		luabind::def("get_zone_can_bind", (int8(*)(uint32,int))&lua_get_zone_can_bind),
		luabind::def("get_zone_can_combat", (int8(*)(uint32))&lua_get_zone_can_combat),
		luabind::def("get_zone_can_combat", (int8(*)(uint32,int))&lua_get_zone_can_combat),
		luabind::def("get_zone_can_levitate", (int8(*)(uint32))&lua_get_zone_can_levitate),
		luabind::def("get_zone_can_levitate", (int8(*)(uint32,int))&lua_get_zone_can_levitate),
		luabind::def("get_zone_cast_outdoor", (int8(*)(uint32))&lua_get_zone_cast_outdoor),
		luabind::def("get_zone_cast_outdoor", (int8(*)(uint32,int))&lua_get_zone_cast_outdoor),
		luabind::def("get_zone_hotzone", (uint8(*)(uint32))&lua_get_zone_hotzone),
		luabind::def("get_zone_hotzone", (uint8(*)(uint32,int))&lua_get_zone_hotzone),
		luabind::def("get_zone_instance_type", (uint8(*)(uint32))&lua_get_zone_instance_type),
		luabind::def("get_zone_instance_type", (uint8(*)(uint32,int))&lua_get_zone_instance_type),
		luabind::def("get_zone_shutdown_delay", (uint64(*)(uint32))&lua_get_zone_shutdown_delay),
		luabind::def("get_zone_shutdown_delay", (uint64(*)(uint32,int))&lua_get_zone_shutdown_delay),
		luabind::def("get_zone_peqzone", (int8(*)(uint32))&lua_get_zone_peqzone),
		luabind::def("get_zone_peqzone", (int8(*)(uint32,int))&lua_get_zone_peqzone),
		luabind::def("get_zone_expansion", (int8(*)(uint32))&lua_get_zone_expansion),
		luabind::def("get_zone_expansion", (int8(*)(uint32,int))&lua_get_zone_expansion),
		luabind::def("get_zone_bypass_expansion_check", (int8(*)(uint32))&lua_get_zone_bypass_expansion_check),
		luabind::def("get_zone_bypass_expansion_check", (int8(*)(uint32,int))&lua_get_zone_bypass_expansion_check),
		luabind::def("get_zone_suspend_buffs", (int8(*)(uint32))&lua_get_zone_suspend_buffs),
		luabind::def("get_zone_suspend_buffs", (int8(*)(uint32,int))&lua_get_zone_suspend_buffs),
		luabind::def("get_zone_rain_chance", (int(*)(uint32))&lua_get_zone_rain_chance),
		luabind::def("get_zone_rain_chance", (int(*)(uint32,int))&lua_get_zone_rain_chance),
		luabind::def("get_zone_rain_duration", (int(*)(uint32))&lua_get_zone_rain_duration),
		luabind::def("get_zone_rain_duration", (int(*)(uint32,int))&lua_get_zone_rain_duration),
		luabind::def("get_zone_snow_chance", (int(*)(uint32))&lua_get_zone_snow_chance),
		luabind::def("get_zone_snow_chance", (int(*)(uint32,int))&lua_get_zone_snow_chance),
		luabind::def("get_zone_snow_duration", (int(*)(uint32))&lua_get_zone_snow_duration),
		luabind::def("get_zone_snow_duration", (int(*)(uint32,int))&lua_get_zone_snow_duration),
		luabind::def("get_zone_gravity", (float(*)(uint32))&lua_get_zone_gravity),
		luabind::def("get_zone_gravity", (float(*)(uint32,int))&lua_get_zone_gravity),
		luabind::def("get_zone_type", (int(*)(uint32))&lua_get_zone_type),
		luabind::def("get_zone_type", (int(*)(uint32,int))&lua_get_zone_type),
		luabind::def("get_zone_sky_lock", (int(*)(uint32))&lua_get_zone_sky_lock),
		luabind::def("get_zone_sky_lock", (int(*)(uint32,int))&lua_get_zone_sky_lock),
		luabind::def("get_zone_fast_regen_hp", (int(*)(uint32))&lua_get_zone_fast_regen_hp),
		luabind::def("get_zone_fast_regen_hp", (int(*)(uint32,int))&lua_get_zone_fast_regen_hp),
		luabind::def("get_zone_fast_regen_mana", (int(*)(uint32))&lua_get_zone_fast_regen_mana),
		luabind::def("get_zone_fast_regen_mana", (int(*)(uint32,int))&lua_get_zone_fast_regen_mana),
		luabind::def("get_zone_fast_regen_endurance", (int(*)(uint32))&lua_get_zone_fast_regen_endurance),
		luabind::def("get_zone_fast_regen_endurance", (int(*)(uint32,int))&lua_get_zone_fast_regen_endurance),
		luabind::def("get_zone_npc_maximum_aggro_distance", (int(*)(uint32))&lua_get_zone_npc_maximum_aggro_distance),
		luabind::def("get_zone_npc_maximum_aggro_distance", (int(*)(uint32,int))&lua_get_zone_npc_maximum_aggro_distance),
		luabind::def("get_zone_minimum_expansion", (int8(*)(uint32))&lua_get_zone_minimum_expansion),
		luabind::def("get_zone_minimum_expansion", (int8(*)(uint32,int))&lua_get_zone_minimum_expansion),
		luabind::def("get_zone_maximum_expansion", (int8(*)(uint32))&lua_get_zone_maximum_expansion),
		luabind::def("get_zone_maximum_expansion", (int8(*)(uint32,int))&lua_get_zone_maximum_expansion),
		luabind::def("get_zone_content_flags", (std::string(*)(uint32))&lua_get_zone_content_flags),
		luabind::def("get_zone_content_flags", (std::string(*)(uint32,int))&lua_get_zone_content_flags),
		luabind::def("get_zone_content_flags_disabled", (std::string(*)(uint32))&lua_get_zone_content_flags_disabled),
		luabind::def("get_zone_content_flags_disabled", (std::string(*)(uint32,int))&lua_get_zone_content_flags_disabled),
		luabind::def("get_zone_underworld_teleport_index", (int(*)(uint32))&lua_get_zone_underworld_teleport_index),
		luabind::def("get_zone_underworld_teleport_index", (int(*)(uint32,int))&lua_get_zone_underworld_teleport_index),
		luabind::def("get_zone_lava_damage", (int(*)(uint32))&lua_get_zone_lava_damage),
		luabind::def("get_zone_lava_damage", (int(*)(uint32,int))&lua_get_zone_lava_damage),
		luabind::def("get_zone_minimum_lava_damage", (int(*)(uint32))&lua_get_zone_minimum_lava_damage),
		luabind::def("get_zone_minimum_lava_damage", (int(*)(uint32,int))&lua_get_zone_minimum_lava_damage),
		luabind::def("get_zone_idle_when_empty", (uint8(*)(uint32))&lua_get_zone_idle_when_empty),
		luabind::def("get_zone_idle_when_empty", (uint8(*)(uint32,int))&lua_get_zone_idle_when_empty),
		luabind::def("get_zone_seconds_before_idle", (uint32(*)(uint32))&lua_get_zone_seconds_before_idle),
		luabind::def("get_zone_seconds_before_idle", (uint32(*)(uint32,int))&lua_get_zone_seconds_before_idle),
		luabind::def("send_channel_message", (void(*)(uint8,uint32,uint8,uint8,const char*))&lua_send_channel_message),
		luabind::def("send_channel_message", (void(*)(Lua_Client,uint8,uint32,uint8,uint8,const char*))&lua_send_channel_message),
		luabind::def("send_channel_message", (void(*)(Lua_Client,const char*,uint8,uint32,uint8,uint8,const char*))&lua_send_channel_message),
		luabind::def("get_spell_level", &lua_get_spell_level),
		luabind::def("is_effect_in_spell", &lua_is_effect_in_spell),
		luabind::def("is_beneficial_spell", &lua_is_beneficial_spell),
		luabind::def("is_detrimental_spell", &lua_is_detrimental_spell),
		luabind::def("is_targetable_ae_spell", &lua_is_targetable_ae_spell),
		luabind::def("is_sacrifice_spell", &lua_is_sacrifice_spell),
		luabind::def("is_lifetap_spell", &lua_is_lifetap_spell),
		luabind::def("is_mesmerize_spell", &lua_is_mesmerize_spell),
		luabind::def("is_stun_spell", &lua_is_stun_spell),
		luabind::def("is_summon_spell", &lua_is_summon_spell),
		luabind::def("is_damage_spell", &lua_is_damage_spell),
		luabind::def("is_fear_spell", &lua_is_fear_spell),
		luabind::def("is_cure_spell", &lua_is_cure_spell),
		luabind::def("is_haste_spell", &lua_is_haste_spell),
		luabind::def("is_harmony_spell", &lua_is_harmony_spell),
		luabind::def("is_percental_heal_spell", &lua_is_percental_heal_spell),
		luabind::def("is_group_only_spell", &lua_is_group_only_spell),
		luabind::def("is_invisible_spell", &lua_is_invisible_spell),
		luabind::def("is_invulnerability_spell", &lua_is_invulnerability_spell),
		luabind::def("is_complete_heal_duration_spell", &lua_is_complete_heal_duration_spell),
		luabind::def("is_poison_counter_spell", &lua_is_poison_counter_spell),
		luabind::def("is_disease_counter_spell", &lua_is_disease_counter_spell),
		luabind::def("is_summon_item_spell", &lua_is_summon_item_spell),
		luabind::def("is_summon_skeleton_spell", &lua_is_summon_skeleton_spell),
		luabind::def("is_summon_pet_spell", &lua_is_summon_pet_spell),
		luabind::def("is_pet_spell", &lua_is_pet_spell),
		luabind::def("is_summon_pc_spell", &lua_is_summon_pc_spell),
		luabind::def("is_charm_spell", &lua_is_charm_spell),
		luabind::def("is_blind_spell", &lua_is_blind_spell),
		luabind::def("is_health_spell", &lua_is_health_spell),
		luabind::def("is_cast_time_reduction_spell", &lua_is_cast_time_reduction_spell),
		luabind::def("is_increase_duration_spell", &lua_is_increase_duration_spell),
		luabind::def("is_mana_cost_reduction_spell", &lua_is_mana_cost_reduction_spell),
		luabind::def("is_increase_range_spell", &lua_is_increase_range_spell),
		luabind::def("is_improved_healing_spell", &lua_is_improved_healing_spell),
		luabind::def("is_improved_damage_spell", &lua_is_improved_damage_spell),
		luabind::def("is_ae_duration_spell", &lua_is_ae_duration_spell),
		luabind::def("is_pure_nuke_spell", &lua_is_pure_nuke_spell),
		luabind::def("is_ae_nuke_spell", &lua_is_ae_nuke_spell),
		luabind::def("is_pbae_nuke_spell", &lua_is_pbae_nuke_spell),
		luabind::def("is_ae_rain_nuke_spell", &lua_is_ae_rain_nuke_spell),
		luabind::def("is_partial_resistable_spell", &lua_is_partial_resistable_spell),
		luabind::def("is_resistable_spell", &lua_is_resistable_spell),
		luabind::def("is_group_spell", &lua_is_group_spell),
		luabind::def("is_tgb_compatible_spell", &lua_is_tgb_compatible_spell),
		luabind::def("is_bard_song", &lua_is_bard_song),
		luabind::def("is_pulsing_bard_song", &lua_is_pulsing_bard_song),
		luabind::def("is_discipline_buff", &lua_is_discipline_buff),
		luabind::def("is_discipline", &lua_is_discipline),
		luabind::def("is_combat_skill", &lua_is_combat_skill),
		luabind::def("is_resurrection_effects", &lua_is_resurrection_effects),
		luabind::def("is_rune_spell", &lua_is_rune_spell),
		luabind::def("is_magic_rune_spell", &lua_is_magic_rune_spell),
		luabind::def("is_mana_tap_spell", &lua_is_mana_tap_spell),
		luabind::def("is_alliance_spell", &lua_is_alliance_spell),
		luabind::def("is_death_save_spell", &lua_is_death_save_spell),
		luabind::def("is_partial_death_save_spell", &lua_is_partial_death_save_spell),
		luabind::def("is_full_death_save_spell", &lua_is_full_death_save_spell),
		luabind::def("is_shadow_step_spell", &lua_is_shadow_step_spell),
		luabind::def("is_succor_spell", &lua_is_succor_spell),
		luabind::def("is_teleport_spell", &lua_is_teleport_spell),
		luabind::def("is_translocate_spell", &lua_is_translocate_spell),
		luabind::def("is_gate_spell", &lua_is_gate_spell),
		luabind::def("is_illusion_spell", &lua_is_illusion_spell),
		luabind::def("is_ldon_object_spell", &lua_is_ldon_object_spell),
		luabind::def("is_heal_over_time_spell", &lua_is_heal_over_time_spell),
		luabind::def("is_complete_heal_spell", &lua_is_complete_heal_spell),
		luabind::def("is_fast_heal_spell", &lua_is_fast_heal_spell),
		luabind::def("is_very_fast_heal_spell", &lua_is_very_fast_heal_spell),
		luabind::def("is_regular_single_target_heal_spell", &lua_is_regular_single_target_heal_spell),
		luabind::def("is_regular_group_heal_spell", &lua_is_regular_group_heal_spell),
		luabind::def("is_group_complete_heal_spell", &lua_is_group_complete_heal_spell),
		luabind::def("is_group_heal_over_time_spell", &lua_is_group_heal_over_time_spell),
		luabind::def("is_debuff_spell", &lua_is_debuff_spell),
		luabind::def("is_resist_debuff_spell", &lua_is_resist_debuff_spell),
		luabind::def("is_self_conversion_spell", &lua_is_self_conversion_spell),
		luabind::def("is_buff_spell", &lua_is_buff_spell),
		luabind::def("is_persist_death_spell", &lua_is_persist_death_spell),
		luabind::def("is_suspendable_spell", &lua_is_suspendable_spell),
		luabind::def("is_cast_on_fade_duration_spell", &lua_is_cast_on_fade_duration_spell),
		luabind::def("is_distance_modifier_spell", &lua_is_distance_modifier_spell),
		luabind::def("is_rest_allowed_spell", &lua_is_rest_allowed_spell),
		luabind::def("is_no_detrimental_spell_aggro_spell", &lua_is_no_detrimental_spell_aggro_spell),
		luabind::def("is_stackable_dot", &lua_is_stackable_dot),
		luabind::def("is_short_duration_buff", &lua_is_short_duration_buff),
		luabind::def("is_target_required_for_spell", &lua_is_target_required_for_spell),
		luabind::def("is_virus_spell", &lua_is_virus_spell),
		luabind::def("is_valid_spell", &lua_is_valid_spell),
		luabind::def("is_effect_ignored_in_stacking", &lua_is_effect_ignored_in_stacking),
		luabind::def("is_focus_limit", &lua_is_focus_limit),
		luabind::def("is_bard_only_stack_effect", &lua_is_bard_only_stack_effect),
		luabind::def("is_cast_while_invisible_spell", &lua_is_cast_while_invisible_spell),
		luabind::def("is_cast_restricted_spell", &lua_is_cast_restricted_spell),
		luabind::def("is_cast_not_standing_spell", &lua_is_cast_not_standing_spell),
		luabind::def("is_instrument_modifier_applied_to_spell_effect", &lua_is_instrument_modifier_applied_to_spell_effect),
		luabind::def("is_blank_spell_effect", &lua_is_blank_spell_effect),
		luabind::def("get_spell_trigger_spell_id", &lua_get_spell_trigger_spell_id),
		luabind::def("get_spell_minimum_level", &lua_get_spell_minimum_level),
		luabind::def("get_spell_resist_type", &lua_get_spell_resist_type),
		luabind::def("get_spell_target_type", &lua_get_spell_target_type),
		luabind::def("get_spell_partial_melee_rune_reduction", &lua_get_spell_partial_melee_rune_reduction),
		luabind::def("get_spell_partial_magic_rune_reduction", &lua_get_spell_partial_magic_rune_reduction),
		luabind::def("get_spell_partial_melee_rune_amount", &lua_get_spell_partial_melee_rune_amount),
		luabind::def("get_spell_partial_magic_rune_amount", &lua_get_spell_partial_magic_rune_amount),
		luabind::def("get_spell_viral_minimum_spread_time", &lua_get_spell_viral_minimum_spread_time),
		luabind::def("get_spell_viral_maximum_spread_time", &lua_get_spell_viral_maximum_spread_time),
		luabind::def("get_spell_viral_spread_range", &lua_get_spell_viral_spread_range),
		luabind::def("get_spell_proc_limit_timer", &lua_get_spell_proc_limit_timer),
		luabind::def("get_spell_effect_description_number", &lua_get_spell_effect_description_number),
		luabind::def("get_spell_furious_bash", &lua_get_spell_furious_bash),
		luabind::def("is_spell_usable_in_this_zone_type", (bool(*)(uint16))&lua_is_spell_usable_in_this_zone_type),
		luabind::def("is_spell_usable_in_this_zone_type", (bool(*)(uint16,uint8))&lua_is_spell_usable_in_this_zone_type),
		luabind::def("get_spell_effect_index", &lua_get_spell_effect_index),
		luabind::def("calculate_poison_counters", &lua_calculate_poison_counters),
		luabind::def("calculate_disease_counters", &lua_calculate_disease_counters),
		luabind::def("calculate_curse_counters", &lua_calculate_curse_counters),
		luabind::def("calculate_corruption_counters", &lua_calculate_corruption_counters),
		luabind::def("calculate_counters", &lua_calculate_counters),
		luabind::def("get_spell_resurrection_sickness_check", &lua_get_spell_resurrection_sickness_check),
		luabind::def("get_spell_nimbus_effect", &lua_get_spell_nimbus_effect),
		luabind::def("convert_money_to_string", &lua_convert_money_to_string),
		luabind::def("cast_spell", &lua_cast_spell),
		luabind::def("self_cast", &lua_self_cast),
		luabind::def("get_bot_class_by_id", &lua_get_bot_class_by_id),
		luabind::def("get_bot_gender_by_id", &lua_get_bot_gender_by_id),
		luabind::def("get_bot_ids_by_character_id", (luabind::object(*)(lua_State*, uint32))&lua_get_bot_ids_by_character_id),
		luabind::def("get_bot_ids_by_character_id", (luabind::object(*)(lua_State*, uint32,uint8))&lua_get_bot_ids_by_character_id),
		luabind::def("get_bot_level_by_id", &lua_get_bot_level_by_id),
		luabind::def("get_bot_name_by_id", &lua_get_bot_name_by_id),
		luabind::def("get_bot_race_by_id", &lua_get_bot_race_by_id),
		luabind::def("silent_say_link", (std::string(*)(std::string))&lua_silent_say_link),
		luabind::def("silent_say_link", (std::string(*)(std::string,std::string))&lua_silent_say_link),
		luabind::def("get_class_bitmask", &lua_get_class_bitmask),
		luabind::def("get_deity_bitmask", &lua_get_deity_bitmask),
		luabind::def("get_race_bitmask", &lua_get_race_bitmask),
		luabind::def("get_auto_login_character_name_by_account_id", &lua_get_auto_login_character_name_by_account_id),
		luabind::def("set_auto_login_character_name_by_account_id", &lua_set_auto_login_character_name_by_account_id),
		luabind::def("get_zone_id_by_long_name", &lua_get_zone_id_by_long_name),
		luabind::def("get_zone_short_name_by_long_name", &lua_get_zone_short_name_by_long_name),
		luabind::def("send_parcel", &lua_send_parcel),
		luabind::def("get_zone_uptime", &lua_get_zone_uptime),
		luabind::def("are_tasks_completed", &lua_are_tasks_completed),
		luabind::def("spawn_circle", &lua_spawn_circle),
		luabind::def("spawn_grid", &lua_spawn_grid),
		luabind::def("get_zone", &lua_get_zone),
		luabind::def("handin", &lua_handin),
		luabind::def("get_paused_timers", &lua_get_paused_timers),
		luabind::def("get_timers", &lua_get_timers),
		luabind::def("get_pet_command_name", &lua_get_pet_command_name),
		luabind::def("get_pet_type_name", &lua_get_pet_type_name),
		/*
			Cross Zone
		*/
		luabind::def("cross_zone_add_ldon_loss_by_char_id", &lua_cross_zone_add_ldon_loss_by_char_id),
		luabind::def("cross_zone_add_ldon_loss_by_group_id", &lua_cross_zone_add_ldon_loss_by_group_id),
		luabind::def("cross_zone_add_ldon_loss_by_raid_id", &lua_cross_zone_add_ldon_loss_by_raid_id),
		luabind::def("cross_zone_add_ldon_loss_by_guild_id", &lua_cross_zone_add_ldon_loss_by_guild_id),
		luabind::def("cross_zone_add_ldon_loss_by_expedition_id", &lua_cross_zone_add_ldon_loss_by_expedition_id),
		luabind::def("cross_zone_add_ldon_loss_by_client_name", &lua_cross_zone_add_ldon_loss_by_client_name),
		luabind::def("cross_zone_add_ldon_points_by_char_id", &lua_cross_zone_add_ldon_points_by_char_id),
		luabind::def("cross_zone_add_ldon_points_by_group_id", &lua_cross_zone_add_ldon_points_by_group_id),
		luabind::def("cross_zone_add_ldon_points_by_raid_id", &lua_cross_zone_add_ldon_points_by_raid_id),
		luabind::def("cross_zone_add_ldon_points_by_guild_id", &lua_cross_zone_add_ldon_points_by_guild_id),
		luabind::def("cross_zone_add_ldon_points_by_expedition_id", &lua_cross_zone_add_ldon_points_by_expedition_id),
		luabind::def("cross_zone_add_ldon_points_by_client_name", &lua_cross_zone_add_ldon_points_by_client_name),
		luabind::def("cross_zone_add_ldon_win_by_char_id", &lua_cross_zone_add_ldon_win_by_char_id),
		luabind::def("cross_zone_add_ldon_win_by_group_id", &lua_cross_zone_add_ldon_win_by_group_id),
		luabind::def("cross_zone_add_ldon_win_by_raid_id", &lua_cross_zone_add_ldon_win_by_raid_id),
		luabind::def("cross_zone_add_ldon_win_by_guild_id", &lua_cross_zone_add_ldon_win_by_guild_id),
		luabind::def("cross_zone_add_ldon_win_by_expedition_id", &lua_cross_zone_add_ldon_win_by_expedition_id),
		luabind::def("cross_zone_add_ldon_win_by_client_name", &lua_cross_zone_add_ldon_win_by_client_name),
		luabind::def("cross_zone_assign_task_by_char_id", (void(*)(int,uint32))&lua_cross_zone_assign_task_by_char_id),
		luabind::def("cross_zone_assign_task_by_char_id", (void(*)(int,uint32,bool))&lua_cross_zone_assign_task_by_char_id),
		luabind::def("cross_zone_assign_task_by_group_id", (void(*)(int,uint32))&lua_cross_zone_assign_task_by_group_id),
		luabind::def("cross_zone_assign_task_by_group_id", (void(*)(int,uint32,bool))&lua_cross_zone_assign_task_by_group_id),
		luabind::def("cross_zone_assign_task_by_raid_id", (void(*)(int,uint32))&lua_cross_zone_assign_task_by_raid_id),
		luabind::def("cross_zone_assign_task_by_raid_id", (void(*)(int,uint32,bool))&lua_cross_zone_assign_task_by_raid_id),
		luabind::def("cross_zone_assign_task_by_guild_id", (void(*)(int,uint32))&lua_cross_zone_assign_task_by_guild_id),
		luabind::def("cross_zone_assign_task_by_guild_id", (void(*)(int,uint32,bool))&lua_cross_zone_assign_task_by_guild_id),
		luabind::def("cross_zone_assign_task_by_expedition_id", (void(*)(uint32,uint32))&lua_cross_zone_assign_task_by_expedition_id),
		luabind::def("cross_zone_assign_task_by_expedition_id", (void(*)(uint32,uint32,bool))&lua_cross_zone_assign_task_by_expedition_id),
		luabind::def("cross_zone_assign_task_by_client_name", (void(*)(const char*,uint32))&lua_cross_zone_assign_task_by_client_name),
		luabind::def("cross_zone_assign_task_by_client_name", (void(*)(const char*,uint32,bool))&lua_cross_zone_assign_task_by_client_name),
		luabind::def("cross_zone_cast_spell_by_char_id", &lua_cross_zone_cast_spell_by_char_id),
		luabind::def("cross_zone_cast_spell_by_group_id", &lua_cross_zone_cast_spell_by_group_id),
		luabind::def("cross_zone_cast_spell_by_raid_id", &lua_cross_zone_cast_spell_by_raid_id),
		luabind::def("cross_zone_cast_spell_by_guild_id", &lua_cross_zone_cast_spell_by_guild_id),
		luabind::def("cross_zone_cast_spell_by_expedition_id", &lua_cross_zone_cast_spell_by_expedition_id),
		luabind::def("cross_zone_cast_spell_by_client_name", &lua_cross_zone_cast_spell_by_client_name),
		luabind::def("cross_zone_dialogue_window_by_char_id", &lua_cross_zone_dialogue_window_by_char_id),
		luabind::def("cross_zone_dialogue_window_by_group_id", &lua_cross_zone_dialogue_window_by_group_id),
		luabind::def("cross_zone_dialogue_window_by_raid_id", &lua_cross_zone_dialogue_window_by_raid_id),
		luabind::def("cross_zone_dialogue_window_by_guild_id", &lua_cross_zone_dialogue_window_by_guild_id),
		luabind::def("cross_zone_dialogue_window_by_expedition_id", &lua_cross_zone_dialogue_window_by_expedition_id),
		luabind::def("cross_zone_dialogue_window_by_client_name", &lua_cross_zone_dialogue_window_by_client_name),
		luabind::def("cross_zone_disable_task_by_char_id", &lua_cross_zone_disable_task_by_char_id),
		luabind::def("cross_zone_disable_task_by_group_id", &lua_cross_zone_disable_task_by_group_id),
		luabind::def("cross_zone_disable_task_by_raid_id", &lua_cross_zone_disable_task_by_raid_id),
		luabind::def("cross_zone_disable_task_by_guild_id", &lua_cross_zone_disable_task_by_guild_id),
		luabind::def("cross_zone_disable_task_by_expedition_id", &lua_cross_zone_disable_task_by_expedition_id),
		luabind::def("cross_zone_disable_task_by_client_name", &lua_cross_zone_disable_task_by_client_name),
		luabind::def("cross_zone_enable_task_by_char_id", &lua_cross_zone_enable_task_by_char_id),
		luabind::def("cross_zone_enable_task_by_group_id", &lua_cross_zone_enable_task_by_group_id),
		luabind::def("cross_zone_enable_task_by_raid_id", &lua_cross_zone_enable_task_by_raid_id),
		luabind::def("cross_zone_enable_task_by_guild_id", &lua_cross_zone_enable_task_by_guild_id),
		luabind::def("cross_zone_enable_task_by_expedition_id", &lua_cross_zone_enable_task_by_expedition_id),
		luabind::def("cross_zone_enable_task_by_client_name", &lua_cross_zone_enable_task_by_client_name),
		luabind::def("cross_zone_fail_task_by_char_id", &lua_cross_zone_fail_task_by_char_id),
		luabind::def("cross_zone_fail_task_by_group_id", &lua_cross_zone_fail_task_by_group_id),
		luabind::def("cross_zone_fail_task_by_raid_id", &lua_cross_zone_fail_task_by_raid_id),
		luabind::def("cross_zone_fail_task_by_guild_id", &lua_cross_zone_fail_task_by_guild_id),
		luabind::def("cross_zone_fail_task_by_expedition_id", &lua_cross_zone_fail_task_by_expedition_id),
		luabind::def("cross_zone_fail_task_by_client_name", &lua_cross_zone_fail_task_by_client_name),
		luabind::def("cross_zone_marquee_by_char_id", &lua_cross_zone_marquee_by_char_id),
		luabind::def("cross_zone_marquee_by_group_id", &lua_cross_zone_marquee_by_group_id),
		luabind::def("cross_zone_marquee_by_raid_id", &lua_cross_zone_marquee_by_raid_id),
		luabind::def("cross_zone_marquee_by_guild_id", &lua_cross_zone_marquee_by_guild_id),
		luabind::def("cross_zone_marquee_by_expedition_id", &lua_cross_zone_marquee_by_expedition_id),
		luabind::def("cross_zone_marquee_by_client_name", &lua_cross_zone_marquee_by_client_name),
		luabind::def("cross_zone_message_player_by_char_id", &lua_cross_zone_message_player_by_char_id),
		luabind::def("cross_zone_message_player_by_group_id", &lua_cross_zone_message_player_by_group_id),
		luabind::def("cross_zone_message_player_by_raid_id", &lua_cross_zone_message_player_by_raid_id),
		luabind::def("cross_zone_message_player_by_guild_id", &lua_cross_zone_message_player_by_guild_id),
		luabind::def("cross_zone_message_player_by_expedition_id", &lua_cross_zone_message_player_by_expedition_id),
		luabind::def("cross_zone_message_player_by_name", &lua_cross_zone_message_player_by_name),
		luabind::def("cross_zone_move_player_by_char_id", (void(*)(uint32,std::string))&lua_cross_zone_move_player_by_char_id),
		luabind::def("cross_zone_move_player_by_char_id", (void(*)(uint32,std::string,float,float,float))&lua_cross_zone_move_player_by_char_id),
		luabind::def("cross_zone_move_player_by_char_id", (void(*)(uint32,std::string,float,float,float,float))&lua_cross_zone_move_player_by_char_id),
		luabind::def("cross_zone_move_player_by_group_id", (void(*)(uint32,std::string))&lua_cross_zone_move_player_by_group_id),
		luabind::def("cross_zone_move_player_by_group_id", (void(*)(uint32,std::string,float,float,float))&lua_cross_zone_move_player_by_group_id),
		luabind::def("cross_zone_move_player_by_group_id", (void(*)(uint32,std::string,float,float,float,float))&lua_cross_zone_move_player_by_group_id),
		luabind::def("cross_zone_move_player_by_raid_id", (void(*)(uint32,std::string))&lua_cross_zone_move_player_by_raid_id),
		luabind::def("cross_zone_move_player_by_raid_id", (void(*)(uint32,std::string,float,float,float))&lua_cross_zone_move_player_by_raid_id),
		luabind::def("cross_zone_move_player_by_raid_id", (void(*)(uint32,std::string,float,float,float,float))&lua_cross_zone_move_player_by_raid_id),
		luabind::def("cross_zone_move_player_by_guild_id", (void(*)(uint32,std::string))&lua_cross_zone_move_player_by_guild_id),
		luabind::def("cross_zone_move_player_by_guild_id", (void(*)(uint32,std::string,float,float,float))&lua_cross_zone_move_player_by_guild_id),
		luabind::def("cross_zone_move_player_by_guild_id", (void(*)(uint32,std::string,float,float,float,float))&lua_cross_zone_move_player_by_guild_id),
		luabind::def("cross_zone_move_player_by_expedition_id", (void(*)(uint32,std::string))&lua_cross_zone_move_player_by_expedition_id),
		luabind::def("cross_zone_move_player_by_expedition_id", (void(*)(uint32,std::string,float,float,float))&lua_cross_zone_move_player_by_expedition_id),
		luabind::def("cross_zone_move_player_by_expedition_id", (void(*)(uint32,std::string,float,float,float,float))&lua_cross_zone_move_player_by_expedition_id),
		luabind::def("cross_zone_move_player_by_client_name", (void(*)(std::string,std::string))&lua_cross_zone_move_player_by_client_name),
		luabind::def("cross_zone_move_player_by_client_name", (void(*)(std::string,std::string,float,float,float))&lua_cross_zone_move_player_by_client_name),
		luabind::def("cross_zone_move_player_by_client_name", (void(*)(std::string,std::string,float,float,float,float))&lua_cross_zone_move_player_by_client_name),
		luabind::def("cross_zone_move_instance_by_char_id", (void(*)(uint32,uint16))&lua_cross_zone_move_instance_by_char_id),
		luabind::def("cross_zone_move_instance_by_char_id", (void(*)(uint32,uint16,float,float,float))&lua_cross_zone_move_instance_by_char_id),
		luabind::def("cross_zone_move_instance_by_char_id", (void(*)(uint32,uint16,float,float,float,float))&lua_cross_zone_move_instance_by_char_id),
		luabind::def("cross_zone_move_instance_by_group_id", (void(*)(uint32,uint16))&lua_cross_zone_move_instance_by_group_id),
		luabind::def("cross_zone_move_instance_by_group_id", (void(*)(uint32,uint16,float,float,float))&lua_cross_zone_move_instance_by_group_id),
		luabind::def("cross_zone_move_instance_by_group_id", (void(*)(uint32,uint16,float,float,float,float))&lua_cross_zone_move_instance_by_group_id),
		luabind::def("cross_zone_move_instance_by_raid_id", (void(*)(uint32,uint16))&lua_cross_zone_move_instance_by_raid_id),
		luabind::def("cross_zone_move_instance_by_raid_id", (void(*)(uint32,uint16,float,float,float))&lua_cross_zone_move_instance_by_raid_id),
		luabind::def("cross_zone_move_instance_by_raid_id", (void(*)(uint32,uint16,float,float,float,float))&lua_cross_zone_move_instance_by_raid_id),
		luabind::def("cross_zone_move_instance_by_guild_id", (void(*)(uint32,uint16))&lua_cross_zone_move_instance_by_guild_id),
		luabind::def("cross_zone_move_instance_by_guild_id", (void(*)(uint32,uint16,float,float,float))&lua_cross_zone_move_instance_by_guild_id),
		luabind::def("cross_zone_move_instance_by_guild_id", (void(*)(uint32,uint16,float,float,float,float))&lua_cross_zone_move_instance_by_guild_id),
		luabind::def("cross_zone_move_instance_by_expedition_id", (void(*)(uint32,uint16))&lua_cross_zone_move_instance_by_expedition_id),
		luabind::def("cross_zone_move_instance_by_expedition_id", (void(*)(uint32,uint16,float,float,float))&lua_cross_zone_move_instance_by_expedition_id),
		luabind::def("cross_zone_move_instance_by_expedition_id", (void(*)(uint32,uint16,float,float,float,float))&lua_cross_zone_move_instance_by_expedition_id),
		luabind::def("cross_zone_move_instance_by_client_name", (void(*)(std::string,uint16))&lua_cross_zone_move_instance_by_client_name),
		luabind::def("cross_zone_move_instance_by_client_name", (void(*)(std::string,uint16,float,float,float))&lua_cross_zone_move_instance_by_client_name),
		luabind::def("cross_zone_move_instance_by_client_name", (void(*)(std::string,uint16,float,float,float,float))&lua_cross_zone_move_instance_by_client_name),
		luabind::def("cross_zone_remove_ldon_loss_by_char_id", &lua_cross_zone_remove_ldon_loss_by_char_id),
		luabind::def("cross_zone_remove_ldon_loss_by_group_id", &lua_cross_zone_remove_ldon_loss_by_group_id),
		luabind::def("cross_zone_remove_ldon_loss_by_raid_id", &lua_cross_zone_remove_ldon_loss_by_raid_id),
		luabind::def("cross_zone_remove_ldon_loss_by_guild_id", &lua_cross_zone_remove_ldon_loss_by_guild_id),
		luabind::def("cross_zone_remove_ldon_loss_by_expedition_id", &lua_cross_zone_remove_ldon_loss_by_expedition_id),
		luabind::def("cross_zone_remove_ldon_loss_by_client_name", &lua_cross_zone_remove_ldon_loss_by_client_name),
		luabind::def("cross_zone_remove_ldon_win_by_char_id", &lua_cross_zone_remove_ldon_win_by_char_id),
		luabind::def("cross_zone_remove_ldon_win_by_group_id", &lua_cross_zone_remove_ldon_win_by_group_id),
		luabind::def("cross_zone_remove_ldon_win_by_raid_id", &lua_cross_zone_remove_ldon_win_by_raid_id),
		luabind::def("cross_zone_remove_ldon_win_by_guild_id", &lua_cross_zone_remove_ldon_win_by_guild_id),
		luabind::def("cross_zone_remove_ldon_win_by_expedition_id", &lua_cross_zone_remove_ldon_win_by_expedition_id),
		luabind::def("cross_zone_remove_ldon_win_by_client_name", &lua_cross_zone_remove_ldon_win_by_client_name),
		luabind::def("cross_zone_remove_spell_by_char_id", &lua_cross_zone_remove_spell_by_char_id),
		luabind::def("cross_zone_remove_spell_by_group_id", &lua_cross_zone_remove_spell_by_group_id),
		luabind::def("cross_zone_remove_spell_by_raid_id", &lua_cross_zone_remove_spell_by_raid_id),
		luabind::def("cross_zone_remove_spell_by_guild_id", &lua_cross_zone_remove_spell_by_guild_id),
		luabind::def("cross_zone_remove_spell_by_expedition_id", &lua_cross_zone_remove_spell_by_expedition_id),
		luabind::def("cross_zone_remove_spell_by_client_name", &lua_cross_zone_remove_spell_by_client_name),
		luabind::def("cross_zone_remove_task_by_char_id", &lua_cross_zone_remove_task_by_char_id),
		luabind::def("cross_zone_remove_task_by_group_id", &lua_cross_zone_remove_task_by_group_id),
		luabind::def("cross_zone_remove_task_by_raid_id", &lua_cross_zone_remove_task_by_raid_id),
		luabind::def("cross_zone_remove_task_by_guild_id", &lua_cross_zone_remove_task_by_guild_id),
		luabind::def("cross_zone_remove_task_by_expedition_id", &lua_cross_zone_remove_task_by_expedition_id),
		luabind::def("cross_zone_remove_task_by_client_name", &lua_cross_zone_remove_task_by_client_name),
		luabind::def("cross_zone_reset_activity_by_char_id", &lua_cross_zone_reset_activity_by_char_id),
		luabind::def("cross_zone_reset_activity_by_group_id", &lua_cross_zone_reset_activity_by_group_id),
		luabind::def("cross_zone_reset_activity_by_raid_id", &lua_cross_zone_reset_activity_by_raid_id),
		luabind::def("cross_zone_reset_activity_by_guild_id", &lua_cross_zone_reset_activity_by_guild_id),
		luabind::def("cross_zone_reset_activity_by_expedition_id", &lua_cross_zone_reset_activity_by_expedition_id),
		luabind::def("cross_zone_reset_activity_by_client_name", &lua_cross_zone_reset_activity_by_client_name),
		luabind::def("cross_zone_set_entity_variable_by_char_id", &lua_cross_zone_set_entity_variable_by_char_id),
		luabind::def("cross_zone_set_entity_variable_by_group_id", &lua_cross_zone_set_entity_variable_by_group_id),
		luabind::def("cross_zone_set_entity_variable_by_raid_id", &lua_cross_zone_set_entity_variable_by_raid_id),
		luabind::def("cross_zone_set_entity_variable_by_guild_id", &lua_cross_zone_set_entity_variable_by_guild_id),
		luabind::def("cross_zone_set_entity_variable_by_expedition_id", &lua_cross_zone_set_entity_variable_by_expedition_id),
		luabind::def("cross_zone_set_entity_variable_by_client_name", &lua_cross_zone_set_entity_variable_by_client_name),
		luabind::def("cross_zone_signal_client_by_char_id", &lua_cross_zone_signal_client_by_char_id),
		luabind::def("cross_zone_signal_client_by_group_id", &lua_cross_zone_signal_client_by_group_id),
		luabind::def("cross_zone_signal_client_by_raid_id", &lua_cross_zone_signal_client_by_raid_id),
		luabind::def("cross_zone_signal_client_by_guild_id", &lua_cross_zone_signal_client_by_guild_id),
		luabind::def("cross_zone_signal_client_by_expedition_id", &lua_cross_zone_signal_client_by_expedition_id),
		luabind::def("cross_zone_signal_client_by_name", &lua_cross_zone_signal_client_by_name),
		luabind::def("cross_zone_signal_npc_by_npctype_id", &lua_cross_zone_signal_npc_by_npctype_id),
		luabind::def("cross_zone_update_activity_by_char_id", (void(*)(int,uint32,int))&lua_cross_zone_update_activity_by_char_id),
		luabind::def("cross_zone_update_activity_by_char_id", (void(*)(int,uint32,int,int))&lua_cross_zone_update_activity_by_char_id),
		luabind::def("cross_zone_update_activity_by_group_id", (void(*)(int,uint32,int))&lua_cross_zone_update_activity_by_group_id),
		luabind::def("cross_zone_update_activity_by_group_id", (void(*)(int,uint32,int,int))&lua_cross_zone_update_activity_by_group_id),
		luabind::def("cross_zone_update_activity_by_raid_id", (void(*)(int,uint32,int))&lua_cross_zone_update_activity_by_raid_id),
		luabind::def("cross_zone_update_activity_by_raid_id", (void(*)(int,uint32,int,int))&lua_cross_zone_update_activity_by_raid_id),
		luabind::def("cross_zone_update_activity_by_guild_id", (void(*)(int,uint32,int))&lua_cross_zone_update_activity_by_guild_id),
		luabind::def("cross_zone_update_activity_by_guild_id", (void(*)(int,uint32,int,int))&lua_cross_zone_update_activity_by_guild_id),
		luabind::def("cross_zone_update_activity_by_expedition_id", (void(*)(uint32,uint32,int))&lua_cross_zone_update_activity_by_expedition_id),
		luabind::def("cross_zone_update_activity_by_expedition_id", (void(*)(uint32,uint32,int,int))&lua_cross_zone_update_activity_by_expedition_id),
		luabind::def("cross_zone_update_activity_by_client_name", (void(*)(const char*,uint32,int))&lua_cross_zone_update_activity_by_client_name),
		luabind::def("cross_zone_update_activity_by_client_name", (void(*)(const char*,uint32,int,int))&lua_cross_zone_update_activity_by_client_name),

		/*
			World Wide
		*/
		luabind::def("world_wide_add_ldon_loss", (void(*)(uint32))&lua_world_wide_add_ldon_loss),
		luabind::def("world_wide_add_ldon_loss", (void(*)(uint32,uint8))&lua_world_wide_add_ldon_loss),
		luabind::def("world_wide_add_ldon_loss", (void(*)(uint32,uint8,uint8))&lua_world_wide_add_ldon_loss),
		luabind::def("world_wide_add_ldon_points", (void(*)(uint32,int))&lua_world_wide_add_ldon_points),
		luabind::def("world_wide_add_ldon_points", (void(*)(uint32,int,uint8))&lua_world_wide_add_ldon_points),
		luabind::def("world_wide_add_ldon_points", (void(*)(uint32,int,uint8,uint8))&lua_world_wide_add_ldon_points),
		luabind::def("world_wide_add_ldon_loss", (void(*)(uint32))&lua_world_wide_add_ldon_win),
		luabind::def("world_wide_add_ldon_loss", (void(*)(uint32,uint8))&lua_world_wide_add_ldon_win),
		luabind::def("world_wide_add_ldon_loss", (void(*)(uint32,uint8,uint8))&lua_world_wide_add_ldon_win),
		luabind::def("world_wide_assign_task", (void(*)(uint32))&lua_world_wide_assign_task),
		luabind::def("world_wide_assign_task", (void(*)(uint32,bool))&lua_world_wide_assign_task),
		luabind::def("world_wide_assign_task", (void(*)(uint32,bool,uint8))&lua_world_wide_assign_task),
		luabind::def("world_wide_assign_task", (void(*)(uint32,bool,uint8,uint8))&lua_world_wide_assign_task),
		luabind::def("world_wide_cast_spell", (void(*)(uint32))&lua_world_wide_cast_spell),
		luabind::def("world_wide_cast_spell", (void(*)(uint32,uint8))&lua_world_wide_cast_spell),
		luabind::def("world_wide_cast_spell", (void(*)(uint32,uint8,uint8))&lua_world_wide_cast_spell),
		luabind::def("world_wide_dialogue_window", (void(*)(const char*))&lua_world_wide_dialogue_window),
		luabind::def("world_wide_dialogue_window", (void(*)(const char*,uint8))&lua_world_wide_dialogue_window),
		luabind::def("world_wide_dialogue_window", (void(*)(const char*,uint8,uint8))&lua_world_wide_dialogue_window),
		luabind::def("world_wide_disable_task", (void(*)(uint32))&lua_world_wide_disable_task),
		luabind::def("world_wide_disable_task", (void(*)(uint32,uint8))&lua_world_wide_disable_task),
		luabind::def("world_wide_disable_task", (void(*)(uint32,uint8,uint8))&lua_world_wide_disable_task),
		luabind::def("world_wide_enable_task", (void(*)(uint32))&lua_world_wide_enable_task),
		luabind::def("world_wide_enable_task", (void(*)(uint32,uint8))&lua_world_wide_enable_task),
		luabind::def("world_wide_enable_task", (void(*)(uint32,uint8,uint8))&lua_world_wide_enable_task),
		luabind::def("world_wide_fail_task", (void(*)(uint32))&lua_world_wide_fail_task),
		luabind::def("world_wide_fail_task", (void(*)(uint32,uint8))&lua_world_wide_fail_task),
		luabind::def("world_wide_fail_task", (void(*)(uint32,uint8,uint8))&lua_world_wide_fail_task),
		luabind::def("world_wide_marquee", (void(*)(uint32,uint32,uint32,uint32,uint32,const char*))&lua_world_wide_marquee),
		luabind::def("world_wide_marquee", (void(*)(uint32,uint32,uint32,uint32,uint32,const char*,uint8))&lua_world_wide_marquee),
		luabind::def("world_wide_marquee", (void(*)(uint32,uint32,uint32,uint32,uint32,const char*,uint8,uint8))&lua_world_wide_marquee),
		luabind::def("world_wide_message", (void(*)(uint32,const char*))&lua_world_wide_message),
		luabind::def("world_wide_message", (void(*)(uint32,const char*,uint8))&lua_world_wide_message),
		luabind::def("world_wide_message", (void(*)(uint32,const char*,uint8,uint8))&lua_world_wide_message),
		luabind::def("world_wide_move", (void(*)(const char*))&lua_world_wide_move),
		luabind::def("world_wide_move", (void(*)(const char*,uint8))&lua_world_wide_move),
		luabind::def("world_wide_move", (void(*)(const char*,uint8,uint8))&lua_world_wide_move),
		luabind::def("world_wide_move_instance", (void(*)(uint16))&lua_world_wide_move_instance),
		luabind::def("world_wide_move_instance", (void(*)(uint16,uint8))&lua_world_wide_move_instance),
		luabind::def("world_wide_move_instance", (void(*)(uint16,uint8,uint8))&lua_world_wide_move_instance),
		luabind::def("world_wide_remove_ldon_loss", (void(*)(uint32))&lua_world_wide_remove_ldon_loss),
		luabind::def("world_wide_remove_ldon_loss", (void(*)(uint32,uint8))&lua_world_wide_remove_ldon_loss),
		luabind::def("world_wide_remove_ldon_loss", (void(*)(uint32,uint8,uint8))&lua_world_wide_remove_ldon_loss),
		luabind::def("world_wide_remove_ldon_win", (void(*)(uint32))&lua_world_wide_remove_ldon_win),
		luabind::def("world_wide_remove_ldon_win", (void(*)(uint32,uint8))&lua_world_wide_remove_ldon_win),
		luabind::def("world_wide_remove_ldon_win", (void(*)(uint32,uint8,uint8))&lua_world_wide_remove_ldon_win),
		luabind::def("world_wide_remove_spell", (void(*)(uint32))&lua_world_wide_remove_spell),
		luabind::def("world_wide_remove_spell", (void(*)(uint32,uint8))&lua_world_wide_remove_spell),
		luabind::def("world_wide_remove_spell", (void(*)(uint32,uint8,uint8))&lua_world_wide_remove_spell),
		luabind::def("world_wide_remove_task", (void(*)(uint32))&lua_world_wide_remove_task),
		luabind::def("world_wide_remove_task", (void(*)(uint32,uint8))&lua_world_wide_remove_task),
		luabind::def("world_wide_remove_task", (void(*)(uint32,uint8,uint8))&lua_world_wide_remove_task),
		luabind::def("world_wide_reset_activity", (void(*)(uint32,int))&lua_world_wide_reset_activity),
		luabind::def("world_wide_reset_activity", (void(*)(uint32,int,uint8))&lua_world_wide_reset_activity),
		luabind::def("world_wide_reset_activity", (void(*)(uint32,int,uint8,uint8))&lua_world_wide_reset_activity),
		luabind::def("world_wide_set_entity_variable_client", (void(*)(const char*,const char*))&lua_world_wide_set_entity_variable_client),
		luabind::def("world_wide_set_entity_variable_client", (void(*)(const char*,const char*,uint8))&lua_world_wide_set_entity_variable_client),
		luabind::def("world_wide_set_entity_variable_client", (void(*)(const char*,const char*,uint8,uint8))&lua_world_wide_set_entity_variable_client),
		luabind::def("world_wide_set_entity_variable_npc", &lua_world_wide_set_entity_variable_npc),
		luabind::def("world_wide_signal_client", (void(*)(int))&lua_world_wide_signal_client),
		luabind::def("world_wide_signal_client", (void(*)(int,uint8))&lua_world_wide_signal_client),
		luabind::def("world_wide_signal_client", (void(*)(int,uint8,uint8))&lua_world_wide_signal_client),
		luabind::def("world_wide_signal_npc", &lua_world_wide_signal_npc),
		luabind::def("world_wide_update_activity", (void(*)(uint32,int))&lua_world_wide_update_activity),
		luabind::def("world_wide_update_activity", (void(*)(uint32,int,int))&lua_world_wide_update_activity),
		luabind::def("world_wide_update_activity", (void(*)(uint32,int,int,uint8))&lua_world_wide_update_activity),
		luabind::def("world_wide_update_activity", (void(*)(uint32,int,int,uint8,uint8))&lua_world_wide_update_activity),

		/**
		 * Expansions
		 */
		luabind::def("is_classic_enabled", &lua_is_classic_enabled),
		luabind::def("is_the_ruins_of_kunark_enabled", &lua_is_the_ruins_of_kunark_enabled),
		luabind::def("is_the_scars_of_velious_enabled", &lua_is_the_scars_of_velious_enabled),
		luabind::def("is_the_shadows_of_luclin_enabled", &lua_is_the_shadows_of_luclin_enabled),
		luabind::def("is_the_planes_of_power_enabled", &lua_is_the_planes_of_power_enabled),
		luabind::def("is_the_legacy_of_ykesha_enabled", &lua_is_the_legacy_of_ykesha_enabled),
		luabind::def("is_lost_dungeons_of_norrath_enabled", &lua_is_lost_dungeons_of_norrath_enabled),
		luabind::def("is_gates_of_discord_enabled", &lua_is_gates_of_discord_enabled),
		luabind::def("is_omens_of_war_enabled", &lua_is_omens_of_war_enabled),
		luabind::def("is_dragons_of_norrath_enabled", &lua_is_dragons_of_norrath_enabled),
		luabind::def("is_depths_of_darkhollow_enabled", &lua_is_depths_of_darkhollow_enabled),
		luabind::def("is_prophecy_of_ro_enabled", &lua_is_prophecy_of_ro_enabled),
		luabind::def("is_the_serpents_spine_enabled", &lua_is_the_serpents_spine_enabled),
		luabind::def("is_the_buried_sea_enabled", &lua_is_the_buried_sea_enabled),
		luabind::def("is_secrets_of_faydwer_enabled", &lua_is_secrets_of_faydwer_enabled),
		luabind::def("is_seeds_of_destruction_enabled", &lua_is_seeds_of_destruction_enabled),
		luabind::def("is_underfoot_enabled", &lua_is_underfoot_enabled),
		luabind::def("is_house_of_thule_enabled", &lua_is_house_of_thule_enabled),
		luabind::def("is_veil_of_alaris_enabled", &lua_is_veil_of_alaris_enabled),
		luabind::def("is_rain_of_fear_enabled", &lua_is_rain_of_fear_enabled),
		luabind::def("is_call_of_the_forsaken_enabled", &lua_is_call_of_the_forsaken_enabled),
		luabind::def("is_the_darkened_sea_enabled", &lua_is_the_darkened_sea_enabled),
		luabind::def("is_the_broken_mirror_enabled", &lua_is_the_broken_mirror_enabled),
		luabind::def("is_empires_of_kunark_enabled", &lua_is_empires_of_kunark_enabled),
		luabind::def("is_ring_of_scale_enabled", &lua_is_ring_of_scale_enabled),
		luabind::def("is_the_burning_lands_enabled", &lua_is_the_burning_lands_enabled),
		luabind::def("is_torment_of_velious_enabled", &lua_is_torment_of_velious_enabled),
		luabind::def("is_current_expansion_classic", &lua_is_current_expansion_classic),
		luabind::def("is_current_expansion_the_ruins_of_kunark", &lua_is_current_expansion_the_ruins_of_kunark),
		luabind::def("is_current_expansion_the_scars_of_velious", &lua_is_current_expansion_the_scars_of_velious),
		luabind::def("is_current_expansion_the_shadows_of_luclin", &lua_is_current_expansion_the_shadows_of_luclin),
		luabind::def("is_current_expansion_the_planes_of_power", &lua_is_current_expansion_the_planes_of_power),
		luabind::def("is_current_expansion_the_legacy_of_ykesha", &lua_is_current_expansion_the_legacy_of_ykesha),
		luabind::def("is_current_expansion_lost_dungeons_of_norrath", &lua_is_current_expansion_lost_dungeons_of_norrath),
		luabind::def("is_current_expansion_gates_of_discord", &lua_is_current_expansion_gates_of_discord),
		luabind::def("is_current_expansion_omens_of_war", &lua_is_current_expansion_omens_of_war),
		luabind::def("is_current_expansion_dragons_of_norrath", &lua_is_current_expansion_dragons_of_norrath),
		luabind::def("is_current_expansion_depths_of_darkhollow", &lua_is_current_expansion_depths_of_darkhollow),
		luabind::def("is_current_expansion_prophecy_of_ro", &lua_is_current_expansion_prophecy_of_ro),
		luabind::def("is_current_expansion_the_serpents_spine", &lua_is_current_expansion_the_serpents_spine),
		luabind::def("is_current_expansion_the_buried_sea", &lua_is_current_expansion_the_buried_sea),
		luabind::def("is_current_expansion_secrets_of_faydwer", &lua_is_current_expansion_secrets_of_faydwer),
		luabind::def("is_current_expansion_seeds_of_destruction", &lua_is_current_expansion_seeds_of_destruction),
		luabind::def("is_current_expansion_underfoot", &lua_is_current_expansion_underfoot),
		luabind::def("is_current_expansion_house_of_thule", &lua_is_current_expansion_house_of_thule),
		luabind::def("is_current_expansion_veil_of_alaris", &lua_is_current_expansion_veil_of_alaris),
		luabind::def("is_current_expansion_rain_of_fear", &lua_is_current_expansion_rain_of_fear),
		luabind::def("is_current_expansion_call_of_the_forsaken", &lua_is_current_expansion_call_of_the_forsaken),
		luabind::def("is_current_expansion_the_darkened_sea", &lua_is_current_expansion_the_darkened_sea),
		luabind::def("is_current_expansion_the_broken_mirror", &lua_is_current_expansion_the_broken_mirror),
		luabind::def("is_current_expansion_empires_of_kunark", &lua_is_current_expansion_empires_of_kunark),
		luabind::def("is_current_expansion_ring_of_scale", &lua_is_current_expansion_ring_of_scale),
		luabind::def("is_current_expansion_the_burning_lands", &lua_is_current_expansion_the_burning_lands),
		luabind::def("is_current_expansion_torment_of_velious", &lua_is_current_expansion_torment_of_velious),

		/**
		 * Content flags
		 */
		luabind::def("is_content_flag_enabled", (bool(*)(std::string))&lua_is_content_flag_enabled),
		luabind::def("set_content_flag", (void(*)(std::string, bool))&lua_set_content_flag),

		luabind::def("get_expedition", &lua_get_expedition),
		luabind::def("get_expedition_by_char_id", &lua_get_expedition_by_char_id),
		luabind::def("get_expedition_by_dz_id", &lua_get_expedition_by_dz_id),
		luabind::def("get_expedition_by_zone_instance", &lua_get_expedition_by_zone_instance),
		luabind::def("get_expedition_lockout_by_char_id", &lua_get_expedition_lockout_by_char_id),
		luabind::def("get_expedition_lockouts_by_char_id", (luabind::object(*)(lua_State*, uint32))&lua_get_expedition_lockouts_by_char_id),
		luabind::def("get_expedition_lockouts_by_char_id", (luabind::object(*)(lua_State*, uint32, std::string))&lua_get_expedition_lockouts_by_char_id),
		luabind::def("add_expedition_lockout_all_clients", (void(*)(std::string, std::string, uint32))&lua_add_expedition_lockout_all_clients),
		luabind::def("add_expedition_lockout_all_clients", (void(*)(std::string, std::string, uint32, std::string))&lua_add_expedition_lockout_all_clients),
		luabind::def("add_expedition_lockout_by_char_id", (void(*)(uint32, std::string, std::string, uint32))&lua_add_expedition_lockout_by_char_id),
		luabind::def("add_expedition_lockout_by_char_id", (void(*)(uint32, std::string, std::string, uint32, std::string))&lua_add_expedition_lockout_by_char_id),
		luabind::def("remove_expedition_lockout_by_char_id", &lua_remove_expedition_lockout_by_char_id),
		luabind::def("remove_all_expedition_lockouts_by_char_id", (void(*)(uint32))&lua_remove_all_expedition_lockouts_by_char_id),
		luabind::def("remove_all_expedition_lockouts_by_char_id", (void(*)(uint32, std::string))&lua_remove_all_expedition_lockouts_by_char_id)
	)];
}

luabind::scope lua_register_random() {
	return luabind::namespace_("Random")
		[(
			luabind::def("Int", &random_int),
			luabind::def("Real", &random_real),
			luabind::def("Roll", &random_roll_int),
			luabind::def("RollReal", &random_roll_real),
			luabind::def("Roll0", &random_roll0)
		)];
}

luabind::scope lua_register_events() {
	return luabind::class_<Events>("Event")
		.enum_("constants")
		[(
			luabind::value("say", static_cast<int>(EVENT_SAY)),
			luabind::value("trade", static_cast<int>(EVENT_TRADE)),
			luabind::value("death", static_cast<int>(EVENT_DEATH)),
			luabind::value("spawn", static_cast<int>(EVENT_SPAWN)),
			luabind::value("combat", static_cast<int>(EVENT_COMBAT)),
			luabind::value("slay", static_cast<int>(EVENT_SLAY)),
			luabind::value("waypoint_arrive", static_cast<int>(EVENT_WAYPOINT_ARRIVE)),
			luabind::value("waypoint_depart", static_cast<int>(EVENT_WAYPOINT_DEPART)),
			luabind::value("timer", static_cast<int>(EVENT_TIMER)),
			luabind::value("signal", static_cast<int>(EVENT_SIGNAL)),
			luabind::value("hp", static_cast<int>(EVENT_HP)),
			luabind::value("enter", static_cast<int>(EVENT_ENTER)),
			luabind::value("exit", static_cast<int>(EVENT_EXIT)),
			luabind::value("enter_zone", static_cast<int>(EVENT_ENTER_ZONE)),
			luabind::value("click_door", static_cast<int>(EVENT_CLICK_DOOR)),
			luabind::value("loot", static_cast<int>(EVENT_LOOT)),
			luabind::value("zone", static_cast<int>(EVENT_ZONE)),
			luabind::value("level_up", static_cast<int>(EVENT_LEVEL_UP)),
			luabind::value("killed_merit", static_cast<int>(EVENT_KILLED_MERIT)),
			luabind::value("cast_on", static_cast<int>(EVENT_CAST_ON)),
			luabind::value("task_accepted", static_cast<int>(EVENT_TASK_ACCEPTED)),
			luabind::value("task_stage_complete", static_cast<int>(EVENT_TASK_STAGE_COMPLETE)),
			luabind::value("environmental_damage", static_cast<int>(EVENT_ENVIRONMENTAL_DAMAGE)),
			luabind::value("task_update", static_cast<int>(EVENT_TASK_UPDATE)),
			luabind::value("task_complete", static_cast<int>(EVENT_TASK_COMPLETE)),
			luabind::value("task_fail", static_cast<int>(EVENT_TASK_FAIL)),
			luabind::value("aggro_say", static_cast<int>(EVENT_AGGRO_SAY)),
			luabind::value("player_pickup", static_cast<int>(EVENT_PLAYER_PICKUP)),
			luabind::value("popup_response", static_cast<int>(EVENT_POPUP_RESPONSE)),
			luabind::value("proximity_say", static_cast<int>(EVENT_PROXIMITY_SAY)),
			luabind::value("cast", static_cast<int>(EVENT_CAST)),
			luabind::value("cast_begin", static_cast<int>(EVENT_CAST_BEGIN)),
			luabind::value("scale_calc", static_cast<int>(EVENT_SCALE_CALC)),
			luabind::value("item_enter_zone", static_cast<int>(EVENT_ITEM_ENTER_ZONE)),
			luabind::value("target_change", static_cast<int>(EVENT_TARGET_CHANGE)),
			luabind::value("hate_list", static_cast<int>(EVENT_HATE_LIST)),
			luabind::value("spell_effect", static_cast<int>(EVENT_SPELL_EFFECT_CLIENT)),
			luabind::value("spell_buff_tic", static_cast<int>(EVENT_SPELL_EFFECT_BUFF_TIC_CLIENT)),
			luabind::value("spell_fade", static_cast<int>(EVENT_SPELL_FADE)),
			luabind::value("spell_effect_translocate_complete", static_cast<int>(EVENT_SPELL_EFFECT_TRANSLOCATE_COMPLETE)),
			luabind::value("combine_success ", static_cast<int>(EVENT_COMBINE_SUCCESS )),
			luabind::value("combine_failure ", static_cast<int>(EVENT_COMBINE_FAILURE )),
			luabind::value("item_click", static_cast<int>(EVENT_ITEM_CLICK)),
			luabind::value("item_click_cast", static_cast<int>(EVENT_ITEM_CLICK_CAST)),
			luabind::value("group_change", static_cast<int>(EVENT_GROUP_CHANGE)),
			luabind::value("forage_success", static_cast<int>(EVENT_FORAGE_SUCCESS)),
			luabind::value("forage_failure", static_cast<int>(EVENT_FORAGE_FAILURE)),
			luabind::value("fish_start", static_cast<int>(EVENT_FISH_START)),
			luabind::value("fish_success", static_cast<int>(EVENT_FISH_SUCCESS)),
			luabind::value("fish_failure", static_cast<int>(EVENT_FISH_FAILURE)),
			luabind::value("click_object", static_cast<int>(EVENT_CLICK_OBJECT)),
			luabind::value("discover_item", static_cast<int>(EVENT_DISCOVER_ITEM)),
			luabind::value("disconnect", static_cast<int>(EVENT_DISCONNECT)),
			luabind::value("connect", static_cast<int>(EVENT_CONNECT)),
			luabind::value("duel_win", static_cast<int>(EVENT_DUEL_WIN)),
			luabind::value("duel_lose", static_cast<int>(EVENT_DUEL_LOSE)),
			luabind::value("encounter_load", static_cast<int>(EVENT_ENCOUNTER_LOAD)),
			luabind::value("encounter_unload", static_cast<int>(EVENT_ENCOUNTER_UNLOAD)),
			luabind::value("command", static_cast<int>(EVENT_COMMAND)),
			luabind::value("drop_item", static_cast<int>(EVENT_DROP_ITEM)),
			luabind::value("destroy_item", static_cast<int>(EVENT_DESTROY_ITEM)),
			luabind::value("feign_death", static_cast<int>(EVENT_FEIGN_DEATH)),
			luabind::value("weapon_proc", static_cast<int>(EVENT_WEAPON_PROC)),
			luabind::value("equip_item", static_cast<int>(EVENT_EQUIP_ITEM)),
			luabind::value("unequip_item", static_cast<int>(EVENT_UNEQUIP_ITEM)),
			luabind::value("augment_item", static_cast<int>(EVENT_AUGMENT_ITEM)),
			luabind::value("unaugment_item", static_cast<int>(EVENT_UNAUGMENT_ITEM)),
			luabind::value("augment_insert", static_cast<int>(EVENT_AUGMENT_INSERT)),
			luabind::value("augment_remove", static_cast<int>(EVENT_AUGMENT_REMOVE)),
			luabind::value("enter_area", static_cast<int>(EVENT_ENTER_AREA)),
			luabind::value("leave_area", static_cast<int>(EVENT_LEAVE_AREA)),
			luabind::value("death_complete", static_cast<int>(EVENT_DEATH_COMPLETE)),
			luabind::value("unhandled_opcode", static_cast<int>(EVENT_UNHANDLED_OPCODE)),
			luabind::value("tick", static_cast<int>(EVENT_TICK)),
			luabind::value("spawn_zone", static_cast<int>(EVENT_SPAWN_ZONE)),
			luabind::value("death_zone", static_cast<int>(EVENT_DEATH_ZONE)),
			luabind::value("use_skill", static_cast<int>(EVENT_USE_SKILL)),
			luabind::value("warp", static_cast<int>(EVENT_WARP)),
			luabind::value("test_buff", static_cast<int>(EVENT_TEST_BUFF)),
			luabind::value("consider", static_cast<int>(EVENT_CONSIDER)),
			luabind::value("consider_corpse", static_cast<int>(EVENT_CONSIDER_CORPSE)),
			luabind::value("loot_zone", static_cast<int>(EVENT_LOOT_ZONE)),
			luabind::value("equip_item_client", static_cast<int>(EVENT_EQUIP_ITEM_CLIENT)),
			luabind::value("unequip_item_client", static_cast<int>(EVENT_UNEQUIP_ITEM_CLIENT)),
			luabind::value("skill_up", static_cast<int>(EVENT_SKILL_UP)),
			luabind::value("language_skill_up", static_cast<int>(EVENT_LANGUAGE_SKILL_UP)),
			luabind::value("alt_currency_merchant_buy", static_cast<int>(EVENT_ALT_CURRENCY_MERCHANT_BUY)),
			luabind::value("alt_currency_merchant_sell", static_cast<int>(EVENT_ALT_CURRENCY_MERCHANT_SELL)),
			luabind::value("merchant_buy", static_cast<int>(EVENT_MERCHANT_BUY)),
			luabind::value("merchant_sell", static_cast<int>(EVENT_MERCHANT_SELL)),
			luabind::value("inspect", static_cast<int>(EVENT_INSPECT)),
			luabind::value("task_before_update", static_cast<int>(EVENT_TASK_BEFORE_UPDATE)),
			luabind::value("aa_buy", static_cast<int>(EVENT_AA_BUY)),
			luabind::value("aa_gained", static_cast<int>(EVENT_AA_GAIN)),
			luabind::value("aa_exp_gained", static_cast<int>(EVENT_AA_EXP_GAIN)),
			luabind::value("exp_gain", static_cast<int>(EVENT_EXP_GAIN)),
			luabind::value("payload", static_cast<int>(EVENT_PAYLOAD)),
			luabind::value("level_down", static_cast<int>(EVENT_LEVEL_DOWN)),
			luabind::value("gm_command", static_cast<int>(EVENT_GM_COMMAND)),
			luabind::value("despawn", static_cast<int>(EVENT_DESPAWN)),
			luabind::value("despawn_zone", static_cast<int>(EVENT_DESPAWN_ZONE)),
			luabind::value("bot_create", static_cast<int>(EVENT_BOT_CREATE)),
			luabind::value("augment_insert_client", static_cast<int>(EVENT_AUGMENT_INSERT_CLIENT)),
			luabind::value("augment_remove_client", static_cast<int>(EVENT_AUGMENT_REMOVE_CLIENT)),
			luabind::value("equip_item_bot", static_cast<int>(EVENT_EQUIP_ITEM_BOT)),
			luabind::value("unequip_item_bot", static_cast<int>(EVENT_UNEQUIP_ITEM_BOT)),
			luabind::value("damage_given", static_cast<int>(EVENT_DAMAGE_GIVEN)),
			luabind::value("damage_taken", static_cast<int>(EVENT_DAMAGE_TAKEN)),
			luabind::value("item_click_client", static_cast<int>(EVENT_ITEM_CLICK_CLIENT)),
			luabind::value("item_click_cast_client", static_cast<int>(EVENT_ITEM_CLICK_CAST_CLIENT)),
			luabind::value("destroy_item_client", static_cast<int>(EVENT_DESTROY_ITEM_CLIENT)),
			luabind::value("drop_item_client", static_cast<int>(EVENT_DROP_ITEM_CLIENT)),
			luabind::value("memorize_spell", static_cast<int>(EVENT_MEMORIZE_SPELL)),
			luabind::value("unmemorize_spell", static_cast<int>(EVENT_UNMEMORIZE_SPELL)),
			luabind::value("scribe_spell", static_cast<int>(EVENT_SCRIBE_SPELL)),
			luabind::value("unscribe_spell", static_cast<int>(EVENT_UNSCRIBE_SPELL)),
			luabind::value("loot_added", static_cast<int>(EVENT_LOOT_ADDED)),
			luabind::value("ldon_points_gain", static_cast<int>(EVENT_LDON_POINTS_GAIN)),
			luabind::value("ldon_points_loss", static_cast<int>(EVENT_LDON_POINTS_LOSS)),
			luabind::value("alt_currency_gain", static_cast<int>(EVENT_ALT_CURRENCY_GAIN)),
			luabind::value("alt_currency_loss", static_cast<int>(EVENT_ALT_CURRENCY_LOSS)),
			luabind::value("crystal_gain", static_cast<int>(EVENT_CRYSTAL_GAIN)),
			luabind::value("crystal_loss", static_cast<int>(EVENT_CRYSTAL_LOSS)),
			luabind::value("timer_pause", static_cast<int>(EVENT_TIMER_PAUSE)),
			luabind::value("timer_resume", static_cast<int>(EVENT_TIMER_RESUME)),
			luabind::value("timer_start", static_cast<int>(EVENT_TIMER_START)),
			luabind::value("timer_stop", static_cast<int>(EVENT_TIMER_STOP)),
			luabind::value("entity_variable_delete", static_cast<int>(EVENT_ENTITY_VARIABLE_DELETE)),
			luabind::value("entity_variable_set", static_cast<int>(EVENT_ENTITY_VARIABLE_SET)),
			luabind::value("entity_variable_update", static_cast<int>(EVENT_ENTITY_VARIABLE_UPDATE)),
			luabind::value("aa_loss", static_cast<int>(EVENT_AA_LOSS)),
			luabind::value("read", static_cast<int>(EVENT_READ_ITEM)),
			luabind::value("pet_command", static_cast<int>(EVENT_PET_COMMAND)),
			luabind::value("charm_start", static_cast<int>(EVENT_CHARM_START)),
			luabind::value("charm_end", static_cast<int>(EVENT_CHARM_END))
		)];
}

luabind::scope lua_register_faction() {
	return luabind::class_<Factions>("Faction")
		.enum_("constants")
		[(
			luabind::value("Ally", static_cast<int>(FACTION_ALLY)),
			luabind::value("Warmly", static_cast<int>(FACTION_WARMLY)),
			luabind::value("Kindly", static_cast<int>(FACTION_KINDLY)),
			luabind::value("Amiable", static_cast<int>(FACTION_AMIABLY)),
			luabind::value("Indifferent", static_cast<int>(FACTION_INDIFFERENTLY)),
			luabind::value("Apprehensive", static_cast<int>(FACTION_APPREHENSIVELY)),
			luabind::value("Dubious", static_cast<int>(FACTION_DUBIOUSLY)),
			luabind::value("Threatenly", static_cast<int>(FACTION_THREATENINGLY)),
			luabind::value("Scowls", static_cast<int>(FACTION_SCOWLS))
		)];
}

luabind::scope lua_register_slot() {
	return luabind::class_<Slots>("Slot")
		.enum_("constants")
		[(
			luabind::value("Charm", static_cast<int>(EQ::invslot::slotCharm)),
			luabind::value("Ear1", static_cast<int>(EQ::invslot::slotEar1)),
			luabind::value("Head", static_cast<int>(EQ::invslot::slotHead)),
			luabind::value("Face", static_cast<int>(EQ::invslot::slotFace)),
			luabind::value("Ear2", static_cast<int>(EQ::invslot::slotEar2)),
			luabind::value("Neck", static_cast<int>(EQ::invslot::slotNeck)),
			luabind::value("Shoulders", static_cast<int>(EQ::invslot::slotShoulders)),
			luabind::value("Arms", static_cast<int>(EQ::invslot::slotArms)),
			luabind::value("Back", static_cast<int>(EQ::invslot::slotBack)),
			luabind::value("Wrist1", static_cast<int>(EQ::invslot::slotWrist1)),
			luabind::value("Wrist2", static_cast<int>(EQ::invslot::slotWrist2)),
			luabind::value("Range", static_cast<int>(EQ::invslot::slotRange)),
			luabind::value("Hands", static_cast<int>(EQ::invslot::slotHands)),
			luabind::value("Primary", static_cast<int>(EQ::invslot::slotPrimary)),
			luabind::value("Secondary", static_cast<int>(EQ::invslot::slotSecondary)),
			luabind::value("Finger1", static_cast<int>(EQ::invslot::slotFinger1)),
			luabind::value("Finger2", static_cast<int>(EQ::invslot::slotFinger2)),
			luabind::value("Chest", static_cast<int>(EQ::invslot::slotChest)),
			luabind::value("Legs", static_cast<int>(EQ::invslot::slotLegs)),
			luabind::value("Feet", static_cast<int>(EQ::invslot::slotFeet)),
			luabind::value("Waist", static_cast<int>(EQ::invslot::slotWaist)),
			luabind::value("PowerSource", static_cast<int>(EQ::invslot::slotPowerSource)),
			luabind::value("Ammo", static_cast<int>(EQ::invslot::slotAmmo)),
			luabind::value("General1", static_cast<int>(EQ::invslot::slotGeneral1)),
			luabind::value("General2", static_cast<int>(EQ::invslot::slotGeneral2)),
			luabind::value("General3", static_cast<int>(EQ::invslot::slotGeneral3)),
			luabind::value("General4", static_cast<int>(EQ::invslot::slotGeneral4)),
			luabind::value("General5", static_cast<int>(EQ::invslot::slotGeneral5)),
			luabind::value("General6", static_cast<int>(EQ::invslot::slotGeneral6)),
			luabind::value("General7", static_cast<int>(EQ::invslot::slotGeneral7)),
			luabind::value("General8", static_cast<int>(EQ::invslot::slotGeneral8)),
			luabind::value("General9", static_cast<int>(EQ::invslot::slotGeneral9)),
			luabind::value("General10", static_cast<int>(EQ::invslot::slotGeneral10)),
			luabind::value("Cursor", static_cast<int>(EQ::invslot::slotCursor)),
			luabind::value("PossessionsBegin", static_cast<int>(EQ::invslot::POSSESSIONS_BEGIN)),
			luabind::value("PossessionsEnd", static_cast<int>(EQ::invslot::POSSESSIONS_END)),
			luabind::value("EquipmentBegin", static_cast<int>(EQ::invslot::EQUIPMENT_BEGIN)),
			luabind::value("EquipmentEnd", static_cast<int>(EQ::invslot::EQUIPMENT_END)),
			luabind::value("GeneralBegin", static_cast<int>(EQ::invslot::GENERAL_BEGIN)),
			luabind::value("GeneralEnd", static_cast<int>(EQ::invslot::GENERAL_END)),
			luabind::value("PossessionsBagsBegin", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN)),
			luabind::value("PossessionsBagsEnd", static_cast<int>(EQ::invbag::CURSOR_BAG_END)),
			luabind::value("GeneralBagsBegin", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN)),
			luabind::value("GeneralBagsEnd", static_cast<int>(EQ::invbag::GENERAL_BAGS_END)),
			luabind::value("General1BagBegin", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN)),
			luabind::value("General1BagEnd", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 9),
			luabind::value("General2BagBegin", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 10),
			luabind::value("General2BagEnd", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 19),
			luabind::value("General3BagBegin", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 20),
			luabind::value("General3BagEnd", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 29),
			luabind::value("General4BagBegin", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 30),
			luabind::value("General4BagEnd", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 39),
			luabind::value("General5BagBegin", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 40),
			luabind::value("General5BagEnd", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 49),
			luabind::value("General6BagBegin", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 50),
			luabind::value("General6BagEnd", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 59),
			luabind::value("General7BagBegin", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 60),
			luabind::value("General7BagEnd", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 69),
			luabind::value("General8BagBegin", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 70),
			luabind::value("General8BagEnd", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 79),
			luabind::value("General9BagBegin", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 80),
			luabind::value("General9BagEnd", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 89),
			luabind::value("General10BagBegin", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 90),
			luabind::value("General10BagEnd", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 99),
			luabind::value("CursorBagBegin", static_cast<int>(EQ::invbag::CURSOR_BAG_BEGIN)),
			luabind::value("CursorBagEnd", static_cast<int>(EQ::invbag::CURSOR_BAG_END)),
			luabind::value("Tradeskill", static_cast<int>(EQ::invslot::SLOT_TRADESKILL_EXPERIMENT_COMBINE)),
			luabind::value("Augment", static_cast<int>(EQ::invslot::SLOT_AUGMENT_GENERIC_RETURN)),
			luabind::value("BankBegin", static_cast<int>(EQ::invslot::BANK_BEGIN)),
			luabind::value("BankEnd", static_cast<int>(EQ::invslot::BANK_END)),
			luabind::value("BankBagsBegin", static_cast<int>(EQ::invbag::BANK_BAGS_BEGIN)),
			luabind::value("BankBagsEnd", static_cast<int>(EQ::invbag::BANK_BAGS_END)),
			luabind::value("SharedBankBegin", static_cast<int>(EQ::invslot::SHARED_BANK_BEGIN)),
			luabind::value("SharedBankEnd", static_cast<int>(EQ::invslot::SHARED_BANK_END)),
			luabind::value("SharedBankBagsBegin", static_cast<int>(EQ::invbag::SHARED_BANK_BAGS_BEGIN)),
			luabind::value("SharedBankBagsEnd", static_cast<int>(EQ::invbag::SHARED_BANK_BAGS_END)),
			luabind::value("BagSlotBegin", static_cast<int>(EQ::invbag::SLOT_BEGIN)),
			luabind::value("BagSlotEnd", static_cast<int>(EQ::invbag::SLOT_END)),
			luabind::value("AugSocketBegin", static_cast<int>(EQ::invaug::SOCKET_BEGIN)),
			luabind::value("AugSocketEnd", static_cast<int>(EQ::invaug::SOCKET_END)),
			luabind::value("Invalid", static_cast<int>(EQ::invslot::SLOT_INVALID)),

			luabind::value("Shoulder", static_cast<int>(EQ::invslot::slotShoulders)), // deprecated
			luabind::value("Bracer1", static_cast<int>(EQ::invslot::slotWrist1)), // deprecated
			luabind::value("Bracer2", static_cast<int>(EQ::invslot::slotWrist2)), // deprecated
			luabind::value("Ring1", static_cast<int>(EQ::invslot::slotFinger1)), // deprecated
			luabind::value("Ring2", static_cast<int>(EQ::invslot::slotFinger2)), // deprecated
			luabind::value("PersonalBegin", static_cast<int>(EQ::invslot::GENERAL_BEGIN)), // deprecated
			luabind::value("PersonalEnd", static_cast<int>(EQ::invslot::GENERAL_END)), // deprecated
			luabind::value("CursorEnd", 0xFFFE) // deprecated (not in use..and never valid vis-a-vis client behavior)
		)];
}

luabind::scope lua_register_material() {
	return luabind::class_<Materials>("Material")
		.enum_("constants")
		[(
			luabind::value("Head", static_cast<int>(EQ::textures::armorHead)),
			luabind::value("Chest", static_cast<int>(EQ::textures::armorChest)),
			luabind::value("Arms", static_cast<int>(EQ::textures::armorArms)),
			luabind::value("Wrist", static_cast<int>(EQ::textures::armorWrist)),
			luabind::value("Hands", static_cast<int>(EQ::textures::armorHands)),
			luabind::value("Legs", static_cast<int>(EQ::textures::armorLegs)),
			luabind::value("Feet", static_cast<int>(EQ::textures::armorFeet)),
			luabind::value("Primary", static_cast<int>(EQ::textures::weaponPrimary)),
			luabind::value("Secondary", static_cast<int>(EQ::textures::weaponSecondary)),
			luabind::value("Count", static_cast<int>(EQ::textures::materialCount)),
			luabind::value("Invalid", static_cast<int>(EQ::textures::materialInvalid)),

			luabind::value("Bracer", static_cast<int>(EQ::textures::armorWrist)), // deprecated
			luabind::value("Max", static_cast<int>(EQ::textures::materialCount)) // deprecated
		)];
}

luabind::scope lua_register_client_version() {
	return luabind::class_<ClientVersions>("ClientVersion")
		.enum_("constants")
		[(
			luabind::value("Unknown", static_cast<int>(EQ::versions::ClientVersion::Unknown)),
			luabind::value("Titanium", static_cast<int>(EQ::versions::ClientVersion::Titanium)),
			luabind::value("SoF", static_cast<int>(EQ::versions::ClientVersion::SoF)),
			luabind::value("SoD", static_cast<int>(EQ::versions::ClientVersion::SoD)),
			luabind::value("Underfoot", static_cast<int>(EQ::versions::ClientVersion::UF)), // deprecated
			luabind::value("UF", static_cast<int>(EQ::versions::ClientVersion::UF)),
			luabind::value("RoF", static_cast<int>(EQ::versions::ClientVersion::RoF)),
			luabind::value("RoF2", static_cast<int>(EQ::versions::ClientVersion::RoF2))
		)];
}

luabind::scope lua_register_appearance() {
	return luabind::class_<Appearances>("Appearance")
		.enum_("constants")
		[(
			luabind::value("Standing", static_cast<int>(eaStanding)),
			luabind::value("Sitting", static_cast<int>(eaSitting)),
			luabind::value("Crouching", static_cast<int>(eaCrouching)),
			luabind::value("Dead", static_cast<int>(eaDead)),
			luabind::value("Looting", static_cast<int>(eaLooting))
		)];
}

luabind::scope lua_register_classes() {
	return luabind::class_<Classes>("Class")
		.enum_("constants")
		[(
			luabind::value("WARRIOR", Class::Warrior),
			luabind::value("CLERIC", Class::Cleric),
			luabind::value("PALADIN", Class::Paladin),
			luabind::value("RANGER", Class::Ranger),
			luabind::value("SHADOWKNIGHT", Class::ShadowKnight),
			luabind::value("DRUID", Class::Druid),
			luabind::value("MONK", Class::Monk),
			luabind::value("BARD", Class::Bard),
			luabind::value("ROGUE", Class::Rogue),
			luabind::value("SHAMAN", Class::Shaman),
			luabind::value("NECROMANCER", Class::Necromancer),
			luabind::value("WIZARD", Class::Wizard),
			luabind::value("MAGICIAN", Class::Magician),
			luabind::value("ENCHANTER", Class::Enchanter),
			luabind::value("BEASTLORD", Class::Beastlord),
			luabind::value("BERSERKER", Class::Berserker),
			luabind::value("WARRIORGM", Class::WarriorGM),
			luabind::value("CLERICGM", Class::ClericGM),
			luabind::value("PALADINGM", Class::PaladinGM),
			luabind::value("RANGERGM", Class::RangerGM),
			luabind::value("SHADOWKNIGHTGM", Class::ShadowKnightGM),
			luabind::value("DRUIDGM", Class::DruidGM),
			luabind::value("MONKGM", Class::MonkGM),
			luabind::value("BARDGM", Class::BardGM),
			luabind::value("ROGUEGM", Class::RogueGM),
			luabind::value("SHAMANGM", Class::ShamanGM),
			luabind::value("NECROMANCERGM", Class::NecromancerGM),
			luabind::value("WIZARDGM", Class::WizardGM),
			luabind::value("MAGICIANGM", Class::MagicianGM),
			luabind::value("ENCHANTERGM", Class::EnchanterGM),
			luabind::value("BEASTLORDGM", Class::BeastlordGM),
			luabind::value("BERSERKERGM", Class::BerserkerGM),
			luabind::value("BANKER", Class::Banker),
			luabind::value("MERCHANT", Class::Merchant),
			luabind::value("DISCORD_MERCHANT", Class::DiscordMerchant),
			luabind::value("ADVENTURE_RECRUITER", Class::AdventureRecruiter),
			luabind::value("ADVENTURE_MERCHANT", Class::AdventureMerchant),
			luabind::value("LDON_TREASURE", Class::LDoNTreasure),
			luabind::value("TRIBUTE_MASTER", Class::TributeMaster),
			luabind::value("GUILD_TRIBUTE_MASTER", Class::GuildTributeMaster),
			luabind::value("GUILD_BANKER", Class::GuildBanker),
			luabind::value("NORRATHS_KEEPERS_MERCHANT", Class::NorrathsKeepersMerchant),
			luabind::value("DARK_REIGN_MERCHANT", Class::DarkReignMerchant),
			luabind::value("FELLOWSHIP_MASTER", Class::FellowshipMaster),
			luabind::value("ALT_CURRENCY_MERCHANT", Class::AlternateCurrencyMerchant),
			luabind::value("MERCENARY_MASTER", Class::MercenaryLiaison)
		)];
}

luabind::scope lua_register_skills() {
	return luabind::class_<Skills>("Skill")
		.enum_("constants")
		[(
			luabind::value("1HBlunt", EQ::skills::Skill1HBlunt),
			luabind::value("Blunt1H", EQ::skills::Skill1HBlunt),
			luabind::value("1HSlashing", EQ::skills::Skill1HSlashing),
			luabind::value("Slashing1H", EQ::skills::Skill1HSlashing),
			luabind::value("2HBlunt", EQ::skills::Skill2HBlunt),
			luabind::value("Blunt2H", EQ::skills::Skill2HBlunt),
			luabind::value("2HSlashing", EQ::skills::Skill2HSlashing),
			luabind::value("Slashing2H", EQ::skills::Skill2HSlashing),
			luabind::value("Abjuration", EQ::skills::SkillAbjuration),
			luabind::value("Alteration", EQ::skills::SkillAlteration),
			luabind::value("ApplyPoison", EQ::skills::SkillApplyPoison),
			luabind::value("Archery", EQ::skills::SkillArchery),
			luabind::value("Backstab", EQ::skills::SkillBackstab),
			luabind::value("BindWound", EQ::skills::SkillBindWound),
			luabind::value("Bash", EQ::skills::SkillBash),
			luabind::value("Block", EQ::skills::SkillBlock),
			luabind::value("BrassInstruments", EQ::skills::SkillBrassInstruments),
			luabind::value("Channeling", EQ::skills::SkillChanneling),
			luabind::value("Conjuration", EQ::skills::SkillConjuration),
			luabind::value("Defense", EQ::skills::SkillDefense),
			luabind::value("Disarm", EQ::skills::SkillDisarm),
			luabind::value("DisarmTraps", EQ::skills::SkillDisarmTraps),
			luabind::value("Divination", EQ::skills::SkillDivination),
			luabind::value("Dodge", EQ::skills::SkillDodge),
			luabind::value("DoubleAttack", EQ::skills::SkillDoubleAttack),
			luabind::value("DragonPunch", EQ::skills::SkillDragonPunch),
			luabind::value("TailRake", EQ::skills::SkillTailRake),
			luabind::value("DualWield", EQ::skills::SkillDualWield),
			luabind::value("EagleStrike", EQ::skills::SkillEagleStrike),
			luabind::value("Evocation", EQ::skills::SkillEvocation),
			luabind::value("FeignDeath", EQ::skills::SkillFeignDeath),
			luabind::value("FlyingKick", EQ::skills::SkillFlyingKick),
			luabind::value("Forage", EQ::skills::SkillForage),
			luabind::value("HandtoHand", EQ::skills::SkillHandtoHand),
			luabind::value("Hide", EQ::skills::SkillHide),
			luabind::value("Kick", EQ::skills::SkillKick),
			luabind::value("Meditate", EQ::skills::SkillMeditate),
			luabind::value("Mend", EQ::skills::SkillMend),
			luabind::value("Offense", EQ::skills::SkillOffense),
			luabind::value("Parry", EQ::skills::SkillParry),
			luabind::value("PickLock", EQ::skills::SkillPickLock),
			luabind::value("1HPiercing", EQ::skills::Skill1HPiercing),
			luabind::value("Piercing1H", EQ::skills::Skill1HPiercing),
			luabind::value("Riposte", EQ::skills::SkillRiposte),
			luabind::value("RoundKick", EQ::skills::SkillRoundKick),
			luabind::value("SafeFall", EQ::skills::SkillSafeFall),
			luabind::value("SenseHeading", EQ::skills::SkillSenseHeading),
			luabind::value("Singing", EQ::skills::SkillSinging),
			luabind::value("Sneak", EQ::skills::SkillSneak),
			luabind::value("SpecializeAbjure", EQ::skills::SkillSpecializeAbjure),
			luabind::value("SpecializeAlteration", EQ::skills::SkillSpecializeAlteration),
			luabind::value("SpecializeConjuration", EQ::skills::SkillSpecializeConjuration),
			luabind::value("SpecializeDivination", EQ::skills::SkillSpecializeDivination),
			luabind::value("SpecializeEvocation", EQ::skills::SkillSpecializeEvocation),
			luabind::value("PickPockets", EQ::skills::SkillPickPockets),
			luabind::value("StringedInstruments", EQ::skills::SkillStringedInstruments),
			luabind::value("Swimming", EQ::skills::SkillSwimming),
			luabind::value("Throwing", EQ::skills::SkillThrowing),
			luabind::value("TigerClaw", EQ::skills::SkillTigerClaw),
			luabind::value("Tracking", EQ::skills::SkillTracking),
			luabind::value("WindInstruments", EQ::skills::SkillWindInstruments),
			luabind::value("Fishing", EQ::skills::SkillFishing),
			luabind::value("MakePoison", EQ::skills::SkillMakePoison),
			luabind::value("Tinkering", EQ::skills::SkillTinkering),
			luabind::value("Research", EQ::skills::SkillResearch),
			luabind::value("Alchemy", EQ::skills::SkillAlchemy),
			luabind::value("Baking", EQ::skills::SkillBaking),
			luabind::value("Tailoring", EQ::skills::SkillTailoring),
			luabind::value("SenseTraps", EQ::skills::SkillSenseTraps),
			luabind::value("Blacksmithing", EQ::skills::SkillBlacksmithing),
			luabind::value("Fletching", EQ::skills::SkillFletching),
			luabind::value("Brewing", EQ::skills::SkillBrewing),
			luabind::value("AlcoholTolerance", EQ::skills::SkillAlcoholTolerance),
			luabind::value("Begging", EQ::skills::SkillBegging),
			luabind::value("JewelryMaking", EQ::skills::SkillJewelryMaking),
			luabind::value("Pottery", EQ::skills::SkillPottery),
			luabind::value("PercussionInstruments", EQ::skills::SkillPercussionInstruments),
			luabind::value("Intimidation", EQ::skills::SkillIntimidation),
			luabind::value("Berserking", EQ::skills::SkillBerserking),
			luabind::value("Taunt", EQ::skills::SkillTaunt),
			luabind::value("Frenzy", EQ::skills::SkillFrenzy),
			luabind::value("RemoveTraps", EQ::skills::SkillRemoveTraps),
			luabind::value("TripleAttack", EQ::skills::SkillTripleAttack),
			luabind::value("2HPiercing", EQ::skills::Skill2HPiercing),
			luabind::value("Piercing2H", EQ::skills::Skill2HPiercing),
			luabind::value("HIGHEST_SKILL", EQ::skills::HIGHEST_SKILL)
		)];
}

luabind::scope lua_register_bodytypes() {
	return luabind::class_<BodyTypes>("BT")
		.enum_("constants")
		[(
			luabind::value("Humanoid", 1),
			luabind::value("Lycanthrope", 2),
			luabind::value("Undead", 3),
			luabind::value("Giant", 4),
			luabind::value("Construct", 5),
			luabind::value("Extraplanar", 6),
			luabind::value("Magical", 7),
			luabind::value("SummonedUndead", 8),
			luabind::value("RaidGiant", 9),
			luabind::value("NoTarget", 11),
			luabind::value("Vampire", 12),
			luabind::value("Atenha_Ra", 13),
			luabind::value("Greater_Akheva", 14),
			luabind::value("Khati_Sha", 15),
			luabind::value("Seru", 16),
			luabind::value("Draz_Nurakk", 18),
			luabind::value("Zek", 19),
			luabind::value("Luggald", 20),
			luabind::value("Animal", 21),
			luabind::value("Insect", 22),
			luabind::value("Monster", 23),
			luabind::value("Summoned", 24),
			luabind::value("Plant", 25),
			luabind::value("Dragon", 26),
			luabind::value("Summoned2", 27),
			luabind::value("Summoned3", 28),
			luabind::value("VeliousDragon", 30),
			luabind::value("Dragon3", 32),
			luabind::value("Boxes", 33),
			luabind::value("Muramite", 34),
			luabind::value("NoTarget2", 60),
			luabind::value("SwarmPet", 63),
			luabind::value("InvisMan", 66),
			luabind::value("Special", 67)
	)];
}

luabind::scope lua_register_filters() {
	return luabind::class_<Filters>("Filter")
		.enum_("constants")
		[(
			luabind::value("None", FilterNone),
			luabind::value("GuildChat", FilterGuildChat),
			luabind::value("Socials", FilterSocials),
			luabind::value("GroupChat", FilterGroupChat),
			luabind::value("Shouts", FilterShouts),
			luabind::value("Auctions", FilterAuctions),
			luabind::value("OOC", FilterOOC),
			luabind::value("BadWords", FilterBadWords),
			luabind::value("PCSpells", FilterPCSpells),
			luabind::value("NPCSpells", FilterNPCSpells),
			luabind::value("BardSongs", FilterBardSongs),
			luabind::value("SpellCrits", FilterSpellCrits),
			luabind::value("MeleeCrits", FilterMeleeCrits),
			luabind::value("SpellDamage", FilterSpellDamage),
			luabind::value("MyMisses", FilterMyMisses),
			luabind::value("OthersMiss", FilterOthersMiss),
			luabind::value("OthersHit", FilterOthersHit),
			luabind::value("MissedMe", FilterMissedMe),
			luabind::value("DamageShields", FilterDamageShields),
			luabind::value("DOT", FilterDOT),
			luabind::value("PetHits", FilterPetHits),
			luabind::value("PetMisses", FilterPetMisses),
			luabind::value("FocusEffects", FilterFocusEffects),
			luabind::value("PetSpells", FilterPetSpells),
			luabind::value("HealOverTime", FilterHealOverTime),
			luabind::value("ItemSpeech", FilterItemSpeech),
			luabind::value("Strikethrough", FilterStrikethrough),
			luabind::value("Stuns", FilterStuns),
			luabind::value("BardSongsOnPets", FilterBardSongsOnPets)
		)];
}

luabind::scope lua_register_message_types() {
	return luabind::class_<MessageTypes>("MT")
		.enum_("constants")
		[(
			luabind::value("White", Chat::White),
			luabind::value("DimGray", Chat::DimGray),
			luabind::value("Default", Chat::Default),
			luabind::value("Green", Chat::Green),
			luabind::value("BrightBlue", Chat::BrightBlue),
			luabind::value("LightBlue", Chat::LightBlue),
			luabind::value("Magenta", Chat::Magenta),
			luabind::value("Gray", Chat::Gray),
			luabind::value("LightGray", Chat::LightGray),
			luabind::value("NPCQuestSay", Chat::NPCQuestSay),
			luabind::value("DarkGray", Chat::DarkGray),
			luabind::value("Red", Chat::Red),
			luabind::value("Lime", Chat::Lime),
			luabind::value("Yellow", Chat::Yellow),
			luabind::value("Blue", Chat::Blue),
			luabind::value("LightNavy", Chat::LightNavy),
			luabind::value("Cyan", Chat::Cyan),
			luabind::value("Black", Chat::Black),
			luabind::value("Say", Chat::Say),
			luabind::value("Tell", Chat::Tell),
			luabind::value("Group", Chat::Group),
			luabind::value("Guild", Chat::Guild),
			luabind::value("OOC", Chat::OOC),
			luabind::value("Auction", Chat::Auction),
			luabind::value("Shout", Chat::Shout),
			luabind::value("Emote", Chat::Emote),
			luabind::value("Spells", Chat::Spells),
			luabind::value("YouHitOther", Chat::YouHitOther),
			luabind::value("OtherHitsYou", Chat::OtherHitYou),
			luabind::value("YouMissOther", Chat::YouMissOther),
			luabind::value("OtherMissesYou", Chat::OtherMissYou),
			luabind::value("Broadcasts", Chat::Broadcasts),
			luabind::value("Skills", Chat::Skills),
			luabind::value("Disciplines", Chat::Disciplines),
			luabind::value("Unused1", Chat::Unused1),
			luabind::value("DefaultText", Chat::DefaultText),
			luabind::value("Unused2", Chat::Unused2),
			luabind::value("MerchantOffer", Chat::MerchantOffer),
			luabind::value("MerchantBuySell", Chat::MerchantExchange),
			luabind::value("YourDeath", Chat::YourDeath),
			luabind::value("OtherDeath", Chat::OtherDeath),
			luabind::value("OtherHits", Chat::OtherHitOther),
			luabind::value("OtherMisses", Chat::OtherMissOther),
			luabind::value("Who", Chat::Who),
			luabind::value("YellForHelp", Chat::YellForHelp),
			luabind::value("NonMelee", Chat::NonMelee),
			luabind::value("WornOff", Chat::SpellWornOff),
			luabind::value("MoneySplit", Chat::MoneySplit),
			luabind::value("LootMessages", Chat::Loot),
			luabind::value("DiceRoll", Chat::DiceRoll),
			luabind::value("OtherSpells", Chat::OtherSpells),
			luabind::value("SpellFailure", Chat::SpellFailure),
			luabind::value("Chat", Chat::ChatChannel),
			luabind::value("Channel1", Chat::Chat1),
			luabind::value("Channel2", Chat::Chat2),
			luabind::value("Channel3", Chat::Chat3),
			luabind::value("Channel4", Chat::Chat4),
			luabind::value("Channel5", Chat::Chat5),
			luabind::value("Channel6", Chat::Chat6),
			luabind::value("Channel7", Chat::Chat7),
			luabind::value("Channel8", Chat::Chat8),
			luabind::value("Channel9", Chat::Chat9),
			luabind::value("Channel10", Chat::Chat10),
			luabind::value("CritMelee", Chat::MeleeCrit),
			luabind::value("SpellCrits", Chat::SpellCrit),
			luabind::value("TooFarAway", Chat::TooFarAway),
			luabind::value("NPCRampage", Chat::NPCRampage),
			luabind::value("NPCFlurry", Chat::NPCFlurry),
			luabind::value("NPCEnrage", Chat::NPCEnrage),
			luabind::value("SayEcho", Chat::EchoSay),
			luabind::value("TellEcho", Chat::EchoTell),
			luabind::value("GroupEcho", Chat::EchoGroup),
			luabind::value("GuildEcho", Chat::EchoGuild),
			luabind::value("OOCEcho", Chat::EchoOOC),
			luabind::value("AuctionEcho", Chat::EchoAuction),
			luabind::value("ShoutECho", Chat::EchoShout),
			luabind::value("EmoteEcho", Chat::EchoEmote),
			luabind::value("Chat1Echo", Chat::EchoChat1),
			luabind::value("Chat2Echo", Chat::EchoChat2),
			luabind::value("Chat3Echo", Chat::EchoChat3),
			luabind::value("Chat4Echo", Chat::EchoChat4),
			luabind::value("Chat5Echo", Chat::EchoChat5),
			luabind::value("Chat6Echo", Chat::EchoChat6),
			luabind::value("Chat7Echo", Chat::EchoChat7),
			luabind::value("Chat8Echo", Chat::EchoChat8),
			luabind::value("Chat9Echo", Chat::EchoChat9),
			luabind::value("Chat10Echo", Chat::EchoChat10),
			luabind::value("DoTDamage", Chat::DotDamage),
			luabind::value("ItemLink", Chat::ItemLink),
			luabind::value("RaidSay", Chat::RaidSay),
			luabind::value("MyPet", Chat::MyPet),
			luabind::value("DS", Chat::DamageShield),
			luabind::value("Leadership", Chat::LeaderShip),
			luabind::value("PetFlurry", Chat::PetFlurry),
			luabind::value("PetCrit", Chat::PetCritical),
			luabind::value("FocusEffect", Chat::FocusEffect),
			luabind::value("Experience", Chat::Experience),
			luabind::value("System", Chat::System),
			luabind::value("PetSpell", Chat::PetSpell),
			luabind::value("PetResponse", Chat::PetResponse),
			luabind::value("ItemSpeech", Chat::ItemSpeech),
			luabind::value("StrikeThrough", Chat::StrikeThrough),
			luabind::value("Stun", Chat::Stun)
	)];
}

luabind::scope lua_register_zone_types() {
	return luabind::class_<ZoneIDs>("Zone")
		.enum_("constants")
		[(
			luabind::value("qeynos", Zones::QEYNOS),
			luabind::value("qeynos2", Zones::QEYNOS2),
			luabind::value("qrg", Zones::QRG),
			luabind::value("qeytoqrg", Zones::QEYTOQRG),
			luabind::value("highpass", Zones::HIGHPASS),
			luabind::value("highkeep", Zones::HIGHKEEP),
			luabind::value("freportn", Zones::FREPORTN),
			luabind::value("freportw", Zones::FREPORTW),
			luabind::value("freporte", Zones::FREPORTE),
			luabind::value("runnyeye", Zones::RUNNYEYE),
			luabind::value("qey2hh1", Zones::QEY2HH1),
			luabind::value("northkarana", Zones::NORTHKARANA),
			luabind::value("southkarana", Zones::SOUTHKARANA),
			luabind::value("eastkarana", Zones::EASTKARANA),
			luabind::value("beholder", Zones::BEHOLDER),
			luabind::value("blackburrow", Zones::BLACKBURROW),
			luabind::value("paw", Zones::PAW),
			luabind::value("rivervale", Zones::RIVERVALE),
			luabind::value("kithicor", Zones::KITHICOR),
			luabind::value("commons", Zones::COMMONS),
			luabind::value("ecommons", Zones::ECOMMONS),
			luabind::value("erudnint", Zones::ERUDNINT),
			luabind::value("erudnext", Zones::ERUDNEXT),
			luabind::value("nektulos", Zones::NEKTULOS),
			luabind::value("cshome", Zones::CSHOME),
			luabind::value("lavastorm", Zones::LAVASTORM),
			luabind::value("nektropos", Zones::NEKTROPOS),
			luabind::value("halas", Zones::HALAS),
			luabind::value("everfrost", Zones::EVERFROST),
			luabind::value("soldunga", Zones::SOLDUNGA),
			luabind::value("soldungb", Zones::SOLDUNGB),
			luabind::value("misty", Zones::MISTY),
			luabind::value("nro", Zones::NRO),
			luabind::value("sro", Zones::SRO),
			luabind::value("befallen", Zones::BEFALLEN),
			luabind::value("oasis", Zones::OASIS),
			luabind::value("tox", Zones::TOX),
			luabind::value("hole", Zones::HOLE),
			luabind::value("neriaka", Zones::NERIAKA),
			luabind::value("neriakb", Zones::NERIAKB),
			luabind::value("neriakc", Zones::NERIAKC),
			luabind::value("neriakd", Zones::NERIAKD),
			luabind::value("najena", Zones::NAJENA),
			luabind::value("qcat", Zones::QCAT),
			luabind::value("innothule", Zones::INNOTHULE),
			luabind::value("feerrott", Zones::FEERROTT),
			luabind::value("cazicthule", Zones::CAZICTHULE),
			luabind::value("oggok", Zones::OGGOK),
			luabind::value("rathemtn", Zones::RATHEMTN),
			luabind::value("lakerathe", Zones::LAKERATHE),
			luabind::value("grobb", Zones::GROBB),
			luabind::value("aviak", Zones::AVIAK),
			luabind::value("gfaydark", Zones::GFAYDARK),
			luabind::value("akanon", Zones::AKANON),
			luabind::value("steamfont", Zones::STEAMFONT),
			luabind::value("lfaydark", Zones::LFAYDARK),
			luabind::value("crushbone", Zones::CRUSHBONE),
			luabind::value("mistmoore", Zones::MISTMOORE),
			luabind::value("kaladima", Zones::KALADIMA),
			luabind::value("felwithea", Zones::FELWITHEA),
			luabind::value("felwitheb", Zones::FELWITHEB),
			luabind::value("unrest", Zones::UNREST),
			luabind::value("kedge", Zones::KEDGE),
			luabind::value("guktop", Zones::GUKTOP),
			luabind::value("gukbottom", Zones::GUKBOTTOM),
			luabind::value("kaladimb", Zones::KALADIMB),
			luabind::value("butcher", Zones::BUTCHER),
			luabind::value("oot", Zones::OOT),
			luabind::value("cauldron", Zones::CAULDRON),
			luabind::value("airplane", Zones::AIRPLANE),
			luabind::value("fearplane", Zones::FEARPLANE),
			luabind::value("permafrost", Zones::PERMAFROST),
			luabind::value("kerraridge", Zones::KERRARIDGE),
			luabind::value("paineel", Zones::PAINEEL),
			luabind::value("hateplane", Zones::HATEPLANE),
			luabind::value("arena", Zones::ARENA),
			luabind::value("fieldofbone", Zones::FIELDOFBONE),
			luabind::value("warslikswood", Zones::WARSLIKSWOOD),
			luabind::value("soltemple", Zones::SOLTEMPLE),
			luabind::value("droga", Zones::DROGA),
			luabind::value("cabwest", Zones::CABWEST),
			luabind::value("swampofnohope", Zones::SWAMPOFNOHOPE),
			luabind::value("firiona", Zones::FIRIONA),
			luabind::value("lakeofillomen", Zones::LAKEOFILLOMEN),
			luabind::value("dreadlands", Zones::DREADLANDS),
			luabind::value("burningwood", Zones::BURNINGWOOD),
			luabind::value("kaesora", Zones::KAESORA),
			luabind::value("sebilis", Zones::SEBILIS),
			luabind::value("citymist", Zones::CITYMIST),
			luabind::value("skyfire", Zones::SKYFIRE),
			luabind::value("frontiermtns", Zones::FRONTIERMTNS),
			luabind::value("overthere", Zones::OVERTHERE),
			luabind::value("emeraldjungle", Zones::EMERALDJUNGLE),
			luabind::value("trakanon", Zones::TRAKANON),
			luabind::value("timorous", Zones::TIMOROUS),
			luabind::value("kurn", Zones::KURN),
			luabind::value("erudsxing", Zones::ERUDSXING),
			luabind::value("stonebrunt", Zones::STONEBRUNT),
			luabind::value("warrens", Zones::WARRENS),
			luabind::value("karnor", Zones::KARNOR),
			luabind::value("chardok", Zones::CHARDOK),
			luabind::value("dalnir", Zones::DALNIR),
			luabind::value("charasis", Zones::CHARASIS),
			luabind::value("cabeast", Zones::CABEAST),
			luabind::value("nurga", Zones::NURGA),
			luabind::value("veeshan", Zones::VEESHAN),
			luabind::value("veksar", Zones::VEKSAR),
			luabind::value("iceclad", Zones::ICECLAD),
			luabind::value("frozenshadow", Zones::FROZENSHADOW),
			luabind::value("velketor", Zones::VELKETOR),
			luabind::value("kael", Zones::KAEL),
			luabind::value("skyshrine", Zones::SKYSHRINE),
			luabind::value("thurgadina", Zones::THURGADINA),
			luabind::value("eastwastes", Zones::EASTWASTES),
			luabind::value("cobaltscar", Zones::COBALTSCAR),
			luabind::value("greatdivide", Zones::GREATDIVIDE),
			luabind::value("wakening", Zones::WAKENING),
			luabind::value("westwastes", Zones::WESTWASTES),
			luabind::value("crystal", Zones::CRYSTAL),
			luabind::value("necropolis", Zones::NECROPOLIS),
			luabind::value("templeveeshan", Zones::TEMPLEVEESHAN),
			luabind::value("sirens", Zones::SIRENS),
			luabind::value("mischiefplane", Zones::MISCHIEFPLANE),
			luabind::value("growthplane", Zones::GROWTHPLANE),
			luabind::value("sleeper", Zones::SLEEPER),
			luabind::value("thurgadinb", Zones::THURGADINB),
			luabind::value("erudsxing2", Zones::ERUDSXING2),
			luabind::value("shadowhaven", Zones::SHADOWHAVEN),
			luabind::value("bazaar", Zones::BAZAAR),
			luabind::value("nexus", Zones::NEXUS),
			luabind::value("echo_", Zones::ECHO_),
			luabind::value("acrylia", Zones::ACRYLIA),
			luabind::value("sharvahl", Zones::SHARVAHL),
			luabind::value("paludal", Zones::PALUDAL),
			luabind::value("fungusgrove", Zones::FUNGUSGROVE),
			luabind::value("vexthal", Zones::VEXTHAL),
			luabind::value("sseru", Zones::SSERU),
			luabind::value("katta", Zones::KATTA),
			luabind::value("netherbian", Zones::NETHERBIAN),
			luabind::value("ssratemple", Zones::SSRATEMPLE),
			luabind::value("griegsend", Zones::GRIEGSEND),
			luabind::value("thedeep", Zones::THEDEEP),
			luabind::value("shadeweaver", Zones::SHADEWEAVER),
			luabind::value("hollowshade", Zones::HOLLOWSHADE),
			luabind::value("grimling", Zones::GRIMLING),
			luabind::value("mseru", Zones::MSERU),
			luabind::value("letalis", Zones::LETALIS),
			luabind::value("twilight", Zones::TWILIGHT),
			luabind::value("thegrey", Zones::THEGREY),
			luabind::value("tenebrous", Zones::TENEBROUS),
			luabind::value("maiden", Zones::MAIDEN),
			luabind::value("dawnshroud", Zones::DAWNSHROUD),
			luabind::value("scarlet", Zones::SCARLET),
			luabind::value("umbral", Zones::UMBRAL),
			luabind::value("akheva", Zones::AKHEVA),
			luabind::value("arena2", Zones::ARENA2),
			luabind::value("jaggedpine", Zones::JAGGEDPINE),
			luabind::value("nedaria", Zones::NEDARIA),
			luabind::value("tutorial", Zones::TUTORIAL),
			luabind::value("load", Zones::LOAD),
			luabind::value("load2", Zones::LOAD2),
			luabind::value("hateplaneb", Zones::HATEPLANEB),
			luabind::value("shadowrest", Zones::SHADOWREST),
			luabind::value("tutoriala", Zones::TUTORIALA),
			luabind::value("tutorialb", Zones::TUTORIALB),
			luabind::value("clz", Zones::CLZ),
			luabind::value("codecay", Zones::CODECAY),
			luabind::value("pojustice", Zones::POJUSTICE),
			luabind::value("poknowledge", Zones::POKNOWLEDGE),
			luabind::value("potranquility", Zones::POTRANQUILITY),
			luabind::value("ponightmare", Zones::PONIGHTMARE),
			luabind::value("podisease", Zones::PODISEASE),
			luabind::value("poinnovation", Zones::POINNOVATION),
			luabind::value("potorment", Zones::POTORMENT),
			luabind::value("povalor", Zones::POVALOR),
			luabind::value("bothunder", Zones::BOTHUNDER),
			luabind::value("postorms", Zones::POSTORMS),
			luabind::value("hohonora", Zones::HOHONORA),
			luabind::value("solrotower", Zones::SOLROTOWER),
			luabind::value("powar", Zones::POWAR),
			luabind::value("potactics", Zones::POTACTICS),
			luabind::value("poair", Zones::POAIR),
			luabind::value("powater", Zones::POWATER),
			luabind::value("pofire", Zones::POFIRE),
			luabind::value("poeartha", Zones::POEARTHA),
			luabind::value("potimea", Zones::POTIMEA),
			luabind::value("hohonorb", Zones::HOHONORB),
			luabind::value("nightmareb", Zones::NIGHTMAREB),
			luabind::value("poearthb", Zones::POEARTHB),
			luabind::value("potimeb", Zones::POTIMEB),
			luabind::value("gunthak", Zones::GUNTHAK),
			luabind::value("dulak", Zones::DULAK),
			luabind::value("torgiran", Zones::TORGIRAN),
			luabind::value("nadox", Zones::NADOX),
			luabind::value("hatesfury", Zones::HATESFURY),
			luabind::value("guka", Zones::GUKA),
			luabind::value("ruja", Zones::RUJA),
			luabind::value("taka", Zones::TAKA),
			luabind::value("mira", Zones::MIRA),
			luabind::value("mmca", Zones::MMCA),
			luabind::value("gukb", Zones::GUKB),
			luabind::value("rujb", Zones::RUJB),
			luabind::value("takb", Zones::TAKB),
			luabind::value("mirb", Zones::MIRB),
			luabind::value("mmcb", Zones::MMCB),
			luabind::value("gukc", Zones::GUKC),
			luabind::value("rujc", Zones::RUJC),
			luabind::value("takc", Zones::TAKC),
			luabind::value("mirc", Zones::MIRC),
			luabind::value("mmcc", Zones::MMCC),
			luabind::value("gukd", Zones::GUKD),
			luabind::value("rujd", Zones::RUJD),
			luabind::value("takd", Zones::TAKD),
			luabind::value("mird", Zones::MIRD),
			luabind::value("mmcd", Zones::MMCD),
			luabind::value("guke", Zones::GUKE),
			luabind::value("ruje", Zones::RUJE),
			luabind::value("take", Zones::TAKE),
			luabind::value("mire", Zones::MIRE),
			luabind::value("mmce", Zones::MMCE),
			luabind::value("gukf", Zones::GUKF),
			luabind::value("rujf", Zones::RUJF),
			luabind::value("takf", Zones::TAKF),
			luabind::value("mirf", Zones::MIRF),
			luabind::value("mmcf", Zones::MMCF),
			luabind::value("gukg", Zones::GUKG),
			luabind::value("rujg", Zones::RUJG),
			luabind::value("takg", Zones::TAKG),
			luabind::value("mirg", Zones::MIRG),
			luabind::value("mmcg", Zones::MMCG),
			luabind::value("gukh", Zones::GUKH),
			luabind::value("rujh", Zones::RUJH),
			luabind::value("takh", Zones::TAKH),
			luabind::value("mirh", Zones::MIRH),
			luabind::value("mmch", Zones::MMCH),
			luabind::value("ruji", Zones::RUJI),
			luabind::value("taki", Zones::TAKI),
			luabind::value("miri", Zones::MIRI),
			luabind::value("mmci", Zones::MMCI),
			luabind::value("rujj", Zones::RUJJ),
			luabind::value("takj", Zones::TAKJ),
			luabind::value("mirj", Zones::MIRJ),
			luabind::value("mmcj", Zones::MMCJ),
			luabind::value("chardokb", Zones::CHARDOKB),
			luabind::value("soldungc", Zones::SOLDUNGC),
			luabind::value("abysmal", Zones::ABYSMAL),
			luabind::value("natimbi", Zones::NATIMBI),
			luabind::value("qinimi", Zones::QINIMI),
			luabind::value("riwwi", Zones::RIWWI),
			luabind::value("barindu", Zones::BARINDU),
			luabind::value("ferubi", Zones::FERUBI),
			luabind::value("snpool", Zones::SNPOOL),
			luabind::value("snlair", Zones::SNLAIR),
			luabind::value("snplant", Zones::SNPLANT),
			luabind::value("sncrematory", Zones::SNCREMATORY),
			luabind::value("tipt", Zones::TIPT),
			luabind::value("vxed", Zones::VXED),
			luabind::value("yxtta", Zones::YXTTA),
			luabind::value("uqua", Zones::UQUA),
			luabind::value("kodtaz", Zones::KODTAZ),
			luabind::value("ikkinz", Zones::IKKINZ),
			luabind::value("qvic", Zones::QVIC),
			luabind::value("inktuta", Zones::INKTUTA),
			luabind::value("txevu", Zones::TXEVU),
			luabind::value("tacvi", Zones::TACVI),
			luabind::value("qvicb", Zones::QVICB),
			luabind::value("wallofslaughter", Zones::WALLOFSLAUGHTER),
			luabind::value("bloodfields", Zones::BLOODFIELDS),
			luabind::value("draniksscar", Zones::DRANIKSSCAR),
			luabind::value("causeway", Zones::CAUSEWAY),
			luabind::value("chambersa", Zones::CHAMBERSA),
			luabind::value("chambersb", Zones::CHAMBERSB),
			luabind::value("chambersc", Zones::CHAMBERSC),
			luabind::value("chambersd", Zones::CHAMBERSD),
			luabind::value("chamberse", Zones::CHAMBERSE),
			luabind::value("chambersf", Zones::CHAMBERSF),
			luabind::value("provinggrounds", Zones::PROVINGGROUNDS),
			luabind::value("anguish", Zones::ANGUISH),
			luabind::value("dranikhollowsa", Zones::DRANIKHOLLOWSA),
			luabind::value("dranikhollowsb", Zones::DRANIKHOLLOWSB),
			luabind::value("dranikhollowsc", Zones::DRANIKHOLLOWSC),
			luabind::value("dranikcatacombsa", Zones::DRANIKCATACOMBSA),
			luabind::value("dranikcatacombsb", Zones::DRANIKCATACOMBSB),
			luabind::value("dranikcatacombsc", Zones::DRANIKCATACOMBSC),
			luabind::value("draniksewersa", Zones::DRANIKSEWERSA),
			luabind::value("draniksewersb", Zones::DRANIKSEWERSB),
			luabind::value("draniksewersc", Zones::DRANIKSEWERSC),
			luabind::value("riftseekers", Zones::RIFTSEEKERS),
			luabind::value("harbingers", Zones::HARBINGERS),
			luabind::value("dranik", Zones::DRANIK),
			luabind::value("broodlands", Zones::BROODLANDS),
			luabind::value("stillmoona", Zones::STILLMOONA),
			luabind::value("stillmoonb", Zones::STILLMOONB),
			luabind::value("thundercrest", Zones::THUNDERCREST),
			luabind::value("delvea", Zones::DELVEA),
			luabind::value("delveb", Zones::DELVEB),
			luabind::value("thenest", Zones::THENEST),
			luabind::value("guildlobby", Zones::GUILDLOBBY),
			luabind::value("guildhall", Zones::GUILDHALL),
			luabind::value("barter", Zones::BARTER),
			luabind::value("illsalin", Zones::ILLSALIN),
			luabind::value("illsalina", Zones::ILLSALINA),
			luabind::value("illsalinb", Zones::ILLSALINB),
			luabind::value("illsalinc", Zones::ILLSALINC),
			luabind::value("dreadspire", Zones::DREADSPIRE),
			luabind::value("drachnidhive", Zones::DRACHNIDHIVE),
			luabind::value("drachnidhivea", Zones::DRACHNIDHIVEA),
			luabind::value("drachnidhiveb", Zones::DRACHNIDHIVEB),
			luabind::value("drachnidhivec", Zones::DRACHNIDHIVEC),
			luabind::value("westkorlach", Zones::WESTKORLACH),
			luabind::value("westkorlacha", Zones::WESTKORLACHA),
			luabind::value("westkorlachb", Zones::WESTKORLACHB),
			luabind::value("westkorlachc", Zones::WESTKORLACHC),
			luabind::value("eastkorlach", Zones::EASTKORLACH),
			luabind::value("eastkorlacha", Zones::EASTKORLACHA),
			luabind::value("shadowspine", Zones::SHADOWSPINE),
			luabind::value("corathus", Zones::CORATHUS),
			luabind::value("corathusa", Zones::CORATHUSA),
			luabind::value("corathusb", Zones::CORATHUSB),
			luabind::value("nektulosa", Zones::NEKTULOSA),
			luabind::value("arcstone", Zones::ARCSTONE),
			luabind::value("relic", Zones::RELIC),
			luabind::value("skylance", Zones::SKYLANCE),
			luabind::value("devastation", Zones::DEVASTATION),
			luabind::value("devastationa", Zones::DEVASTATIONA),
			luabind::value("rage", Zones::RAGE),
			luabind::value("ragea", Zones::RAGEA),
			luabind::value("takishruins", Zones::TAKISHRUINS),
			luabind::value("takishruinsa", Zones::TAKISHRUINSA),
			luabind::value("elddar", Zones::ELDDAR),
			luabind::value("elddara", Zones::ELDDARA),
			luabind::value("theater", Zones::THEATER),
			luabind::value("theatera", Zones::THEATERA),
			luabind::value("freeporteast", Zones::FREEPORTEAST),
			luabind::value("freeportwest", Zones::FREEPORTWEST),
			luabind::value("freeportsewers", Zones::FREEPORTSEWERS),
			luabind::value("freeportacademy", Zones::FREEPORTACADEMY),
			luabind::value("freeporttemple", Zones::FREEPORTTEMPLE),
			luabind::value("freeportmilitia", Zones::FREEPORTMILITIA),
			luabind::value("freeportarena", Zones::FREEPORTARENA),
			luabind::value("freeportcityhall", Zones::FREEPORTCITYHALL),
			luabind::value("freeporttheater", Zones::FREEPORTTHEATER),
			luabind::value("freeporthall", Zones::FREEPORTHALL),
			luabind::value("northro", Zones::NORTHRO),
			luabind::value("southro", Zones::SOUTHRO),
			luabind::value("crescent", Zones::CRESCENT),
			luabind::value("moors", Zones::MOORS),
			luabind::value("stonehive", Zones::STONEHIVE),
			luabind::value("mesa", Zones::MESA),
			luabind::value("roost", Zones::ROOST),
			luabind::value("steppes", Zones::STEPPES),
			luabind::value("icefall", Zones::ICEFALL),
			luabind::value("valdeholm", Zones::VALDEHOLM),
			luabind::value("frostcrypt", Zones::FROSTCRYPT),
			luabind::value("sunderock", Zones::SUNDEROCK),
			luabind::value("vergalid", Zones::VERGALID),
			luabind::value("direwind", Zones::DIREWIND),
			luabind::value("ashengate", Zones::ASHENGATE),
			luabind::value("highpasshold", Zones::HIGHPASSHOLD),
			luabind::value("commonlands", Zones::COMMONLANDS),
			luabind::value("oceanoftears", Zones::OCEANOFTEARS),
			luabind::value("kithforest", Zones::KITHFOREST),
			luabind::value("befallenb", Zones::BEFALLENB),
			luabind::value("highpasskeep", Zones::HIGHPASSKEEP),
			luabind::value("innothuleb", Zones::INNOTHULEB),
			luabind::value("toxxulia", Zones::TOXXULIA),
			luabind::value("mistythicket", Zones::MISTYTHICKET),
			luabind::value("kattacastrum", Zones::KATTACASTRUM),
			luabind::value("thalassius", Zones::THALASSIUS),
			luabind::value("atiiki", Zones::ATIIKI),
			luabind::value("zhisza", Zones::ZHISZA),
			luabind::value("silyssar", Zones::SILYSSAR),
			luabind::value("solteris", Zones::SOLTERIS),
			luabind::value("barren", Zones::BARREN),
			luabind::value("buriedsea", Zones::BURIEDSEA),
			luabind::value("jardelshook", Zones::JARDELSHOOK),
			luabind::value("monkeyrock", Zones::MONKEYROCK),
			luabind::value("suncrest", Zones::SUNCREST),
			luabind::value("deadbone", Zones::DEADBONE),
			luabind::value("blacksail", Zones::BLACKSAIL),
			luabind::value("maidensgrave", Zones::MAIDENSGRAVE),
			luabind::value("redfeather", Zones::REDFEATHER),
			luabind::value("shipmvp", Zones::SHIPMVP),
			luabind::value("shipmvu", Zones::SHIPMVU),
			luabind::value("shippvu", Zones::SHIPPVU),
			luabind::value("shipuvu", Zones::SHIPUVU),
			luabind::value("shipmvm", Zones::SHIPMVM),
			luabind::value("mechanotus", Zones::MECHANOTUS),
			luabind::value("mansion", Zones::MANSION),
			luabind::value("steamfactory", Zones::STEAMFACTORY),
			luabind::value("shipworkshop", Zones::SHIPWORKSHOP),
			luabind::value("gyrospireb", Zones::GYROSPIREB),
			luabind::value("gyrospirez", Zones::GYROSPIREZ),
			luabind::value("dragonscale", Zones::DRAGONSCALE),
			luabind::value("lopingplains", Zones::LOPINGPLAINS),
			luabind::value("hillsofshade", Zones::HILLSOFSHADE),
			luabind::value("bloodmoon", Zones::BLOODMOON),
			luabind::value("crystallos", Zones::CRYSTALLOS),
			luabind::value("guardian", Zones::GUARDIAN),
			luabind::value("steamfontmts", Zones::STEAMFONTMTS),
			luabind::value("cryptofshade", Zones::CRYPTOFSHADE),
			luabind::value("dragonscaleb", Zones::DRAGONSCALEB),
			luabind::value("oldfieldofbone", Zones::OLDFIELDOFBONE),
			luabind::value("oldkaesoraa", Zones::OLDKAESORAA),
			luabind::value("oldkaesorab", Zones::OLDKAESORAB),
			luabind::value("oldkurn", Zones::OLDKURN),
			luabind::value("oldkithicor", Zones::OLDKITHICOR),
			luabind::value("oldcommons", Zones::OLDCOMMONS),
			luabind::value("oldhighpass", Zones::OLDHIGHPASS),
			luabind::value("thevoida", Zones::THEVOIDA),
			luabind::value("thevoidb", Zones::THEVOIDB),
			luabind::value("thevoidc", Zones::THEVOIDC),
			luabind::value("thevoidd", Zones::THEVOIDD),
			luabind::value("thevoide", Zones::THEVOIDE),
			luabind::value("thevoidf", Zones::THEVOIDF),
			luabind::value("thevoidg", Zones::THEVOIDG),
			luabind::value("oceangreenhills", Zones::OCEANGREENHILLS),
			luabind::value("oceangreenvillage", Zones::OCEANGREENVILLAGE),
			luabind::value("oldblackburrow", Zones::OLDBLACKBURROW),
			luabind::value("bertoxtemple", Zones::BERTOXTEMPLE),
			luabind::value("discord", Zones::DISCORD),
			luabind::value("discordtower", Zones::DISCORDTOWER),
			luabind::value("oldbloodfield", Zones::OLDBLOODFIELD),
			luabind::value("precipiceofwar", Zones::PRECIPICEOFWAR),
			luabind::value("olddranik", Zones::OLDDRANIK),
			luabind::value("toskirakk", Zones::TOSKIRAKK),
			luabind::value("korascian", Zones::KORASCIAN),
			luabind::value("rathechamber", Zones::RATHECHAMBER),
			luabind::value("brellsrest", Zones::BRELLSREST),
			luabind::value("fungalforest", Zones::FUNGALFOREST),
			luabind::value("underquarry", Zones::UNDERQUARRY),
			luabind::value("coolingchamber", Zones::COOLINGCHAMBER),
			luabind::value("shiningcity", Zones::SHININGCITY),
			luabind::value("arthicrex", Zones::ARTHICREX),
			luabind::value("foundation", Zones::FOUNDATION),
			luabind::value("lichencreep", Zones::LICHENCREEP),
			luabind::value("pellucid", Zones::PELLUCID),
			luabind::value("stonesnake", Zones::STONESNAKE),
			luabind::value("brellstemple", Zones::BRELLSTEMPLE),
			luabind::value("convorteum", Zones::CONVORTEUM),
			luabind::value("brellsarena", Zones::BRELLSARENA),
			luabind::value("weddingchapel", Zones::WEDDINGCHAPEL),
			luabind::value("weddingchapeldark", Zones::WEDDINGCHAPELDARK),
			luabind::value("dragoncrypt", Zones::DRAGONCRYPT),
			luabind::value("feerrott2", Zones::FEERROTT2),
			luabind::value("thulehouse1", Zones::THULEHOUSE1),
			luabind::value("thulehouse2", Zones::THULEHOUSE2),
			luabind::value("housegarden", Zones::HOUSEGARDEN),
			luabind::value("thulelibrary", Zones::THULELIBRARY),
			luabind::value("well", Zones::WELL),
			luabind::value("fallen", Zones::FALLEN),
			luabind::value("morellcastle", Zones::MORELLCASTLE),
			luabind::value("somnium", Zones::SOMNIUM),
			luabind::value("alkabormare", Zones::ALKABORMARE),
			luabind::value("miragulmare", Zones::MIRAGULMARE),
			luabind::value("thuledream", Zones::THULEDREAM),
			luabind::value("neighborhood", Zones::NEIGHBORHOOD),
			luabind::value("argath", Zones::ARGATH),
			luabind::value("arelis", Zones::ARELIS),
			luabind::value("sarithcity", Zones::SARITHCITY),
			luabind::value("rubak", Zones::RUBAK),
			luabind::value("beastdomain", Zones::BEASTDOMAIN),
			luabind::value("resplendent", Zones::RESPLENDENT),
			luabind::value("pillarsalra", Zones::PILLARSALRA),
			luabind::value("windsong", Zones::WINDSONG),
			luabind::value("cityofbronze", Zones::CITYOFBRONZE),
			luabind::value("sepulcher", Zones::SEPULCHER),
			luabind::value("eastsepulcher", Zones::EASTSEPULCHER),
			luabind::value("westsepulcher", Zones::WESTSEPULCHER),
			luabind::value("shardslanding", Zones::SHARDSLANDING),
			luabind::value("xorbb", Zones::XORBB),
			luabind::value("kaelshard", Zones::KAELSHARD),
			luabind::value("eastwastesshard", Zones::EASTWASTESSHARD),
			luabind::value("crystalshard", Zones::CRYSTALSHARD),
			luabind::value("breedinggrounds", Zones::BREEDINGGROUNDS),
			luabind::value("eviltree", Zones::EVILTREE),
			luabind::value("grelleth", Zones::GRELLETH),
			luabind::value("chapterhouse", Zones::CHAPTERHOUSE),
			luabind::value("arttest", Zones::ARTTEST),
			luabind::value("fhalls", Zones::FHALLS),
			luabind::value("apprentice", Zones::APPRENTICE)
	)];
}

luabind::scope lua_register_languages() {
	return luabind::class_<LanguageIDs>("Language")
		.enum_("constants")
		[(
			luabind::value("CommonTongue", Language::CommonTongue),
			luabind::value("Barbarian", Language::Barbarian),
			luabind::value("Erudian", Language::Erudian),
			luabind::value("Elvish", Language::Elvish),
			luabind::value("DarkElvish", Language::DarkElvish),
			luabind::value("Dwarvish", Language::Dwarvish),
			luabind::value("Troll", Language::Troll),
			luabind::value("Ogre", Language::Ogre),
			luabind::value("Gnomish", Language::Gnomish),
			luabind::value("Halfling", Language::Halfling),
			luabind::value("ThievesCant", Language::ThievesCant),
			luabind::value("OldErudian", Language::OldErudian),
			luabind::value("ElderElvish", Language::ElderElvish),
			luabind::value("Froglok", Language::Froglok),
			luabind::value("Goblin", Language::Goblin),
			luabind::value("Gnoll", Language::Gnoll),
			luabind::value("CombineTongue", Language::CombineTongue),
			luabind::value("ElderTeirDal", Language::ElderTeirDal),
			luabind::value("Lizardman", Language::Lizardman),
			luabind::value("Orcish", Language::Orcish),
			luabind::value("Faerie", Language::Faerie),
			luabind::value("Dragon", Language::Dragon),
			luabind::value("ElderDragon", Language::ElderDragon),
			luabind::value("DarkSpeech", Language::DarkSpeech),
			luabind::value("VahShir", Language::VahShir),
			luabind::value("Alaran", Language::Alaran),
			luabind::value("Hadal", Language::Hadal),
			luabind::value("Unknown27", Language::Unknown27),
			luabind::value("MaxValue", Language::MaxValue)
	)];
}

luabind::scope lua_register_rulei() {
	return luabind::namespace_("RuleI")
		[
			luabind::def("Get", &get_rulei)
		];
}

luabind::scope lua_register_ruler() {
	return luabind::namespace_("RuleR")
		[
			luabind::def("Get", &get_ruler)
		];
}

luabind::scope lua_register_ruleb() {
	return luabind::namespace_("RuleB")
		[
			luabind::def("Get", &get_ruleb)
		];
}

luabind::scope lua_register_rules() {
	return luabind::namespace_("RuleS")
		[
			luabind::def("Get", &get_rules)
		];
}

luabind::scope lua_register_journal_speakmode() {
	return luabind::class_<Journal_SpeakMode>("SpeakMode")
		.enum_("constants")
		[(
			luabind::value("Raw", static_cast<int>(Journal::SpeakMode::Raw)),
			luabind::value("Say", static_cast<int>(Journal::SpeakMode::Say)),
			luabind::value("Shout", static_cast<int>(Journal::SpeakMode::Shout)),
			luabind::value("EmoteAlt", static_cast<int>(Journal::SpeakMode::EmoteAlt)),
			luabind::value("Emote", static_cast<int>(Journal::SpeakMode::Emote)),
			luabind::value("Group", static_cast<int>(Journal::SpeakMode::Group))
		)];
}

luabind::scope lua_register_journal_mode() {
	return luabind::class_<Journal_Mode>("JournalMode")
		.enum_("constants")
		[(
			luabind::value("None", static_cast<int>(Journal::Mode::None)),
			luabind::value("Log1", static_cast<int>(Journal::Mode::Log1)),
			luabind::value("Log2", static_cast<int>(Journal::Mode::Log2))
		)];
}

luabind::scope lua_register_exp_source() {
	return luabind::class_<ExpSource>("ExpSource")
		.enum_("constants")
		[(
			luabind::value("Quest", static_cast<int>(ExpSource::Quest)),
			luabind::value("GM", static_cast<int>(ExpSource::GM)),
			luabind::value("Kill", static_cast<int>(ExpSource::Kill)),
			luabind::value("Death", static_cast<int>(ExpSource::Death)),
			luabind::value("Resurrection", static_cast<int>(ExpSource::Resurrection)),
			luabind::value("LDoNChest", static_cast<int>(ExpSource::LDoNChest)),
			luabind::value("Task", static_cast<int>(ExpSource::Task)),
			luabind::value("Sacrifice", static_cast<int>(ExpSource::Sacrifice))
		)];
}

#endif // LUA_EQEMU
