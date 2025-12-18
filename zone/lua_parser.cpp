#ifdef LUA_EQEMU

#include "lua_parser.h"

#include "common/spdat.h"
#include "zone/lua_bit.h"
#include "zone/lua_bot.h"
#include "zone/lua_buff.h"
#include "zone/lua_client.h"
#include "zone/lua_corpse.h"
#include "zone/lua_database.h"
#include "zone/lua_door.h"
#include "zone/lua_encounter.h"
#include "zone/lua_entity_list.h"
#include "zone/lua_entity.h"
#include "zone/lua_expedition.h"
#include "zone/lua_general.h"
#include "zone/lua_group.h"
#include "zone/lua_hate_list.h"
#include "zone/lua_inventory.h"
#include "zone/lua_item.h"
#include "zone/lua_iteminst.h"
#include "zone/lua_merc.h"
#include "zone/lua_mob.h"
#include "zone/lua_npc.h"
#include "zone/lua_object.h"
#include "zone/lua_packet.h"
#include "zone/lua_raid.h"
#include "zone/lua_spawn.h"
#include "zone/lua_spell.h"
#include "zone/lua_stat_bonuses.h"
#include "zone/lua_zone.h"
#include "zone/masterentity.h"
#include "zone/questmgr.h"
#include "zone/zone_config.h"
#include "zone/zone.h"

#include "lua.hpp"
#include "luabind/luabind.hpp"
#include "luabind/object.hpp"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <string>
#include <vector>

const char *LuaEvents[_LargestEventID] = {
	"event_say",
	"event_trade",
	"event_death",
	"event_spawn",
	"event_attack",
	"event_combat",
	"event_aggro",
	"event_slay",
	"event_npc_slay",
	"event_waypoint_arrive",
	"event_waypoint_depart",
	"event_timer",
	"event_signal",
	"event_hp",
	"event_enter",
	"event_exit",
	"event_enter_zone",
	"event_click_door",
	"event_loot",
	"event_zone",
	"event_level_up",
	"event_killed_merit",
	"event_cast_on",
	"event_task_accepted",
	"event_task_stage_complete",
	"event_task_update",
	"event_task_complete",
	"event_task_fail",
	"event_aggro_say",
	"event_player_pickup",
	"event_popup_response",
	"event_environmental_damage",
	"event_proximity_say",
	"event_cast",
	"event_cast_begin",
	"event_scale_calc",
	"event_item_enter_zone",
	"event_target_change",
	"event_hate_list",
	"event_spell_effect",
	"event_spell_effect",
	"event_spell_buff_tic",
	"event_spell_buff_tic",
	"event_spell_fade",
	"event_spell_effect_translocate_complete",
	"event_combine_success",
	"event_combine_failure",
	"event_item_click",
	"event_item_click_cast",
	"event_group_change",
	"event_forage_success",
	"event_forage_failure",
	"event_fish_start",
	"event_fish_success",
	"event_fish_failure",
	"event_click_object",
	"event_discover_item",
	"event_disconnect",
	"event_connect",
	"event_item_tick",
	"event_duel_win",
	"event_duel_lose",
	"event_encounter_load",
	"event_encounter_unload",
	"event_command",
	"event_drop_item",
	"event_destroy_item",
	"event_feign_death",
	"event_weapon_proc",
	"event_equip_item",
	"event_unequip_item",
	"event_augment_item",
	"event_unaugment_item",
	"event_augment_insert",
	"event_augment_remove",
	"event_enter_area",
	"event_leave_area",
	"event_respawn",
	"event_death_complete",
	"event_unhandled_opcode",
	"event_tick",
	"event_spawn_zone",
	"event_death_zone",
	"event_use_skill",
	"event_combine_validate",
	"event_bot_command",
	"event_warp",
	"event_test_buff",
	"event_combine",
	"event_consider",
	"event_consider_corpse",
	"event_loot_zone",
	"event_equip_item_client",
	"event_unequip_item_client",
	"event_skill_up",
	"event_language_skill_up",
	"event_alt_currency_merchant_buy",
	"event_alt_currency_merchant_sell",
	"event_merchant_buy",
	"event_merchant_sell",
	"event_inspect",
	"event_task_before_update",
	"event_aa_buy",
	"event_aa_gain",
	"event_aa_exp_gain",
	"event_exp_gain",
	"event_payload",
	"event_level_down",
	"event_gm_command",
	"event_despawn",
	"event_despawn_zone",
	"event_bot_create",
	"event_augment_insert_client",
	"event_augment_remove_client",
	"event_equip_item_bot",
	"event_unequip_item_bot",
	"event_damage_given",
	"event_damage_taken",
	"event_item_click_client",
	"event_item_click_cast_client",
	"event_destroy_item_client",
	"event_drop_item_client",
	"event_memorize_spell",
	"event_unmemorize_spell",
	"event_scribe_spell",
	"event_unscribe_spell",
	"event_loot_added",
	"event_ldon_points_gain",
	"event_ldon_points_loss",
	"event_alt_currency_gain",
	"event_alt_currency_loss",
	"event_crystal_gain",
	"event_crystal_loss",
	"event_timer_pause",
	"event_timer_resume",
	"event_timer_start",
	"event_timer_stop",
	"event_entity_variable_delete",
	"event_entity_variable_set",
	"event_entity_variable_update",
	"event_aa_loss",
	"event_spell_blocked",
	"event_read_item",
	"event_pet_command",
	"event_charm_start",
	"event_charm_end"
};

extern Zone *zone;

struct lua_registered_event {
	std::string encounter_name;
	luabind::adl::object lua_reference;
	QuestEventID event_id;
};

std::map<std::string, std::list<lua_registered_event>> lua_encounter_events_registered;
std::map<std::string, bool> lua_encounters_loaded;
std::map<std::string, Encounter *> lua_encounters;

// use debug.traceback() for errors (luaL_traceback is only in luajit and lua 5.2+)
static void PushErrorHandler(lua_State* L)
{
	lua_getglobal(L, "debug");
	lua_getfield(L, -1, "traceback");
	lua_remove(L, -2);
}

LuaParser::LuaParser() {
	for (int i = 0; i < _LargestEventID; ++i) {
		NPCArgumentDispatch[i]       = handle_npc_null;
		PlayerArgumentDispatch[i]    = handle_player_null;
		ItemArgumentDispatch[i]      = handle_item_null;
		SpellArgumentDispatch[i]     = handle_spell_null;
		EncounterArgumentDispatch[i] = handle_encounter_null;
		BotArgumentDispatch[i]       = handle_bot_null;
		ZoneArgumentDispatch[i]      = handle_zone_null;
	}

	NPCArgumentDispatch[EVENT_SAY]                    = handle_npc_event_say;
	NPCArgumentDispatch[EVENT_AGGRO_SAY]              = handle_npc_event_say;
	NPCArgumentDispatch[EVENT_PROXIMITY_SAY]          = handle_npc_event_say;
	NPCArgumentDispatch[EVENT_TRADE]                  = handle_npc_event_trade;
	NPCArgumentDispatch[EVENT_HP]                     = handle_npc_event_hp;
	NPCArgumentDispatch[EVENT_TARGET_CHANGE]          = handle_npc_single_mob;
	NPCArgumentDispatch[EVENT_CAST_ON]                = handle_npc_cast;
	NPCArgumentDispatch[EVENT_KILLED_MERIT]           = handle_npc_single_client;
	NPCArgumentDispatch[EVENT_SLAY]                   = handle_npc_single_mob;
	NPCArgumentDispatch[EVENT_ENTER]                  = handle_npc_single_client;
	NPCArgumentDispatch[EVENT_EXIT]                   = handle_npc_single_client;
	NPCArgumentDispatch[EVENT_TASK_ACCEPTED]          = handle_npc_task_accepted;
	NPCArgumentDispatch[EVENT_POPUP_RESPONSE]         = handle_npc_popup;
	NPCArgumentDispatch[EVENT_WAYPOINT_ARRIVE]        = handle_npc_waypoint;
	NPCArgumentDispatch[EVENT_WAYPOINT_DEPART]        = handle_npc_waypoint;
	NPCArgumentDispatch[EVENT_HATE_LIST]              = handle_npc_hate;
	NPCArgumentDispatch[EVENT_COMBAT]                 = handle_npc_hate;
	NPCArgumentDispatch[EVENT_SIGNAL]                 = handle_npc_signal;
	NPCArgumentDispatch[EVENT_TIMER]                  = handle_npc_timer;
	NPCArgumentDispatch[EVENT_DEATH]                  = handle_npc_death;
	NPCArgumentDispatch[EVENT_DEATH_COMPLETE]         = handle_npc_death;
	NPCArgumentDispatch[EVENT_DEATH_ZONE]             = handle_npc_death;
	NPCArgumentDispatch[EVENT_CAST]                   = handle_npc_cast;
	NPCArgumentDispatch[EVENT_CAST_BEGIN]             = handle_npc_cast;
	NPCArgumentDispatch[EVENT_FEIGN_DEATH]            = handle_npc_single_client;
	NPCArgumentDispatch[EVENT_ENTER_AREA]             = handle_npc_area;
	NPCArgumentDispatch[EVENT_LEAVE_AREA]             = handle_npc_area;
	NPCArgumentDispatch[EVENT_LOOT_ZONE]              = handle_npc_loot_zone;
	NPCArgumentDispatch[EVENT_SPAWN_ZONE]             = handle_npc_spawn_zone;
	NPCArgumentDispatch[EVENT_PAYLOAD]                = handle_npc_payload;
	NPCArgumentDispatch[EVENT_DESPAWN_ZONE]           = handle_npc_despawn_zone;
	NPCArgumentDispatch[EVENT_DAMAGE_GIVEN]           = handle_npc_damage;
	NPCArgumentDispatch[EVENT_DAMAGE_TAKEN]           = handle_npc_damage;
	NPCArgumentDispatch[EVENT_LOOT_ADDED]             = handle_npc_loot_added;
	NPCArgumentDispatch[EVENT_TIMER_PAUSE]            = handle_npc_timer_pause_resume_start;
	NPCArgumentDispatch[EVENT_TIMER_RESUME]           = handle_npc_timer_pause_resume_start;
	NPCArgumentDispatch[EVENT_TIMER_START]            = handle_npc_timer_pause_resume_start;
	NPCArgumentDispatch[EVENT_TIMER_STOP]             = handle_npc_timer_stop;
	NPCArgumentDispatch[EVENT_ENTITY_VARIABLE_DELETE] = handle_npc_entity_variable;
	NPCArgumentDispatch[EVENT_ENTITY_VARIABLE_SET]    = handle_npc_entity_variable;
	NPCArgumentDispatch[EVENT_ENTITY_VARIABLE_UPDATE] = handle_npc_entity_variable;
	NPCArgumentDispatch[EVENT_SPELL_BLOCKED]          = handle_npc_spell_blocked;
	NPCArgumentDispatch[EVENT_PET_COMMAND]            = handle_npc_pet_command;
	NPCArgumentDispatch[EVENT_CHARM_START]            = handle_npc_single_mob;
	NPCArgumentDispatch[EVENT_CHARM_END]              = handle_npc_single_mob;

	PlayerArgumentDispatch[EVENT_SAY]                        = handle_player_say;
	PlayerArgumentDispatch[EVENT_ENVIRONMENTAL_DAMAGE]       = handle_player_environmental_damage;
	PlayerArgumentDispatch[EVENT_DEATH]                      = handle_player_death;
	PlayerArgumentDispatch[EVENT_DEATH_COMPLETE]             = handle_player_death;
	PlayerArgumentDispatch[EVENT_TIMER]                      = handle_player_timer;
	PlayerArgumentDispatch[EVENT_DISCOVER_ITEM]              = handle_player_discover_item;
	PlayerArgumentDispatch[EVENT_FISH_SUCCESS]               = handle_player_fish_forage_success;
	PlayerArgumentDispatch[EVENT_FORAGE_SUCCESS]             = handle_player_fish_forage_success;
	PlayerArgumentDispatch[EVENT_CLICK_OBJECT]               = handle_player_click_object;
	PlayerArgumentDispatch[EVENT_CLICK_DOOR]                 = handle_player_click_door;
	PlayerArgumentDispatch[EVENT_SIGNAL]                     = handle_player_signal;
	PlayerArgumentDispatch[EVENT_POPUP_RESPONSE]             = handle_player_popup_response;
	PlayerArgumentDispatch[EVENT_PLAYER_PICKUP]              = handle_player_pick_up;
	PlayerArgumentDispatch[EVENT_CAST]                       = handle_player_cast;
	PlayerArgumentDispatch[EVENT_CAST_BEGIN]                 = handle_player_cast;
	PlayerArgumentDispatch[EVENT_CAST_ON]                    = handle_player_cast;
	PlayerArgumentDispatch[EVENT_TASK_FAIL]                  = handle_player_task_fail;
	PlayerArgumentDispatch[EVENT_ZONE]                       = handle_player_zone;
	PlayerArgumentDispatch[EVENT_DUEL_WIN]                   = handle_player_duel_win;
	PlayerArgumentDispatch[EVENT_DUEL_LOSE]                  = handle_player_duel_loss;
	PlayerArgumentDispatch[EVENT_LOOT]                       = handle_player_loot;
	PlayerArgumentDispatch[EVENT_TASK_STAGE_COMPLETE]        = handle_player_task_stage_complete;
	PlayerArgumentDispatch[EVENT_TASK_ACCEPTED]              = handle_player_task_accepted;
	PlayerArgumentDispatch[EVENT_TASK_COMPLETE]              = handle_player_task_update;
	PlayerArgumentDispatch[EVENT_TASK_UPDATE]                = handle_player_task_update;
	PlayerArgumentDispatch[EVENT_TASK_BEFORE_UPDATE]         = handle_player_task_update;
	PlayerArgumentDispatch[EVENT_COMMAND]                    = handle_player_command;
	PlayerArgumentDispatch[EVENT_COMBINE_SUCCESS]            = handle_player_combine;
	PlayerArgumentDispatch[EVENT_COMBINE_FAILURE]            = handle_player_combine;
	PlayerArgumentDispatch[EVENT_FEIGN_DEATH]                = handle_player_feign;
	PlayerArgumentDispatch[EVENT_ENTER_AREA]                 = handle_player_area;
	PlayerArgumentDispatch[EVENT_LEAVE_AREA]                 = handle_player_area;
	PlayerArgumentDispatch[EVENT_RESPAWN]                    = handle_player_respawn;
	PlayerArgumentDispatch[EVENT_UNHANDLED_OPCODE]           = handle_player_packet;
	PlayerArgumentDispatch[EVENT_USE_SKILL]                  = handle_player_use_skill;
	PlayerArgumentDispatch[EVENT_TEST_BUFF]                  = handle_test_buff;
	PlayerArgumentDispatch[EVENT_COMBINE_VALIDATE]           = handle_player_combine_validate;
	PlayerArgumentDispatch[EVENT_BOT_COMMAND]                = handle_player_bot_command;
	PlayerArgumentDispatch[EVENT_WARP]                       = handle_player_warp;
	PlayerArgumentDispatch[EVENT_COMBINE]                    = handle_player_quest_combine;
	PlayerArgumentDispatch[EVENT_CONSIDER]                   = handle_player_consider;
	PlayerArgumentDispatch[EVENT_CONSIDER_CORPSE]            = handle_player_consider_corpse;
	PlayerArgumentDispatch[EVENT_EQUIP_ITEM_CLIENT]          = handle_player_equip_item;
	PlayerArgumentDispatch[EVENT_UNEQUIP_ITEM_CLIENT]        = handle_player_equip_item;
	PlayerArgumentDispatch[EVENT_SKILL_UP]                   = handle_player_skill_up;
	PlayerArgumentDispatch[EVENT_LANGUAGE_SKILL_UP]          = handle_player_language_skill_up;
	PlayerArgumentDispatch[EVENT_ALT_CURRENCY_MERCHANT_BUY]  = handle_player_alt_currency_merchant;
	PlayerArgumentDispatch[EVENT_ALT_CURRENCY_MERCHANT_SELL] = handle_player_alt_currency_merchant;
	PlayerArgumentDispatch[EVENT_MERCHANT_BUY]               = handle_player_merchant;
	PlayerArgumentDispatch[EVENT_MERCHANT_SELL]              = handle_player_merchant;
	PlayerArgumentDispatch[EVENT_INSPECT]                    = handle_player_inspect;
	PlayerArgumentDispatch[EVENT_AA_BUY]                     = handle_player_aa_buy;
	PlayerArgumentDispatch[EVENT_AA_GAIN]                    = handle_player_aa_gain;
	PlayerArgumentDispatch[EVENT_AA_EXP_GAIN]                = handle_player_aa_exp_gain;
	PlayerArgumentDispatch[EVENT_EXP_GAIN]                   = handle_player_exp_gain;
	PlayerArgumentDispatch[EVENT_PAYLOAD]                    = handle_player_payload;
	PlayerArgumentDispatch[EVENT_LEVEL_UP]                   = handle_player_level_up;
	PlayerArgumentDispatch[EVENT_LEVEL_DOWN]                 = handle_player_level_down;
	PlayerArgumentDispatch[EVENT_GM_COMMAND]                 = handle_player_gm_command;
	PlayerArgumentDispatch[EVENT_BOT_CREATE]                 = handle_player_bot_create;
	PlayerArgumentDispatch[EVENT_AUGMENT_INSERT_CLIENT]      = handle_player_augment_insert;
	PlayerArgumentDispatch[EVENT_AUGMENT_REMOVE_CLIENT]      = handle_player_augment_remove;
	PlayerArgumentDispatch[EVENT_DAMAGE_GIVEN]               = handle_player_damage;
	PlayerArgumentDispatch[EVENT_DAMAGE_TAKEN]               = handle_player_damage;
	PlayerArgumentDispatch[EVENT_ITEM_CLICK_CAST_CLIENT]     = handle_player_item_click;
	PlayerArgumentDispatch[EVENT_ITEM_CLICK_CLIENT]          = handle_player_item_click;
	PlayerArgumentDispatch[EVENT_DESTROY_ITEM_CLIENT]        = handle_player_destroy_item;
	PlayerArgumentDispatch[EVENT_TARGET_CHANGE]              = handle_player_target_change;
	PlayerArgumentDispatch[EVENT_DROP_ITEM_CLIENT]           = handle_player_drop_item;
	PlayerArgumentDispatch[EVENT_MEMORIZE_SPELL]             = handle_player_memorize_scribe_spell;
	PlayerArgumentDispatch[EVENT_UNMEMORIZE_SPELL]           = handle_player_memorize_scribe_spell;
	PlayerArgumentDispatch[EVENT_SCRIBE_SPELL]               = handle_player_memorize_scribe_spell;
	PlayerArgumentDispatch[EVENT_UNSCRIBE_SPELL]             = handle_player_memorize_scribe_spell;
	PlayerArgumentDispatch[EVENT_LDON_POINTS_GAIN]           = handle_player_ldon_points_gain_loss;
	PlayerArgumentDispatch[EVENT_LDON_POINTS_LOSS]           = handle_player_ldon_points_gain_loss;
	PlayerArgumentDispatch[EVENT_ALT_CURRENCY_GAIN]          = handle_player_alt_currency_gain_loss;
	PlayerArgumentDispatch[EVENT_ALT_CURRENCY_LOSS]          = handle_player_alt_currency_gain_loss;
	PlayerArgumentDispatch[EVENT_CRYSTAL_GAIN]               = handle_player_crystal_gain_loss;
	PlayerArgumentDispatch[EVENT_CRYSTAL_LOSS]               = handle_player_crystal_gain_loss;
	PlayerArgumentDispatch[EVENT_TIMER_PAUSE]                = handle_player_timer_pause_resume_start;
	PlayerArgumentDispatch[EVENT_TIMER_RESUME]               = handle_player_timer_pause_resume_start;
	PlayerArgumentDispatch[EVENT_TIMER_START]                = handle_player_timer_pause_resume_start;
	PlayerArgumentDispatch[EVENT_TIMER_STOP]                 = handle_player_timer_stop;
	PlayerArgumentDispatch[EVENT_ENTITY_VARIABLE_DELETE]     = handle_player_entity_variable;
	PlayerArgumentDispatch[EVENT_ENTITY_VARIABLE_SET]        = handle_player_entity_variable;
	PlayerArgumentDispatch[EVENT_ENTITY_VARIABLE_UPDATE]     = handle_player_entity_variable;
	PlayerArgumentDispatch[EVENT_AA_LOSS]                    = handle_player_aa_loss;
	PlayerArgumentDispatch[EVENT_SPELL_BLOCKED]              = handle_player_spell_blocked;
	PlayerArgumentDispatch[EVENT_READ_ITEM]                  = handle_player_read_item;
	PlayerArgumentDispatch[EVENT_CONNECT]                    = handle_player_connect;
	PlayerArgumentDispatch[EVENT_PET_COMMAND]                = handle_player_pet_command;

	ItemArgumentDispatch[EVENT_ITEM_CLICK]      = handle_item_click;
	ItemArgumentDispatch[EVENT_ITEM_CLICK_CAST] = handle_item_click;
	ItemArgumentDispatch[EVENT_TIMER]           = handle_item_timer;
	ItemArgumentDispatch[EVENT_WEAPON_PROC]     = handle_item_proc;
	ItemArgumentDispatch[EVENT_LOOT]            = handle_item_loot;
	ItemArgumentDispatch[EVENT_EQUIP_ITEM]      = handle_item_equip;
	ItemArgumentDispatch[EVENT_UNEQUIP_ITEM]    = handle_item_equip;
	ItemArgumentDispatch[EVENT_AUGMENT_ITEM]    = handle_item_augment;
	ItemArgumentDispatch[EVENT_UNAUGMENT_ITEM]  = handle_item_augment;
	ItemArgumentDispatch[EVENT_AUGMENT_INSERT]  = handle_item_augment_insert;
	ItemArgumentDispatch[EVENT_AUGMENT_REMOVE]  = handle_item_augment_remove;
	ItemArgumentDispatch[EVENT_TIMER_PAUSE]     = handle_item_timer_pause_resume_start;
	ItemArgumentDispatch[EVENT_TIMER_RESUME]    = handle_item_timer_pause_resume_start;
	ItemArgumentDispatch[EVENT_TIMER_START]     = handle_item_timer_pause_resume_start;
	ItemArgumentDispatch[EVENT_TIMER_STOP]      = handle_item_timer_stop;

	SpellArgumentDispatch[EVENT_SPELL_EFFECT_CLIENT]               = handle_spell_event;
	SpellArgumentDispatch[EVENT_SPELL_EFFECT_BUFF_TIC_CLIENT]      = handle_spell_event;
	SpellArgumentDispatch[EVENT_SPELL_FADE]                        = handle_spell_event;
	SpellArgumentDispatch[EVENT_SPELL_EFFECT_TRANSLOCATE_COMPLETE] = handle_translocate_finish;

	EncounterArgumentDispatch[EVENT_TIMER]            = handle_encounter_timer;
	EncounterArgumentDispatch[EVENT_ENCOUNTER_LOAD]   = handle_encounter_load;
	EncounterArgumentDispatch[EVENT_ENCOUNTER_UNLOAD] = handle_encounter_unload;

	BotArgumentDispatch[EVENT_CAST]                   = handle_bot_cast;
	BotArgumentDispatch[EVENT_CAST_BEGIN]             = handle_bot_cast;
	BotArgumentDispatch[EVENT_CAST_ON]                = handle_bot_cast;
	BotArgumentDispatch[EVENT_COMBAT]                 = handle_bot_combat;
	BotArgumentDispatch[EVENT_DEATH]                  = handle_bot_death;
	BotArgumentDispatch[EVENT_DEATH_COMPLETE]         = handle_bot_death;
	BotArgumentDispatch[EVENT_POPUP_RESPONSE]         = handle_bot_popup_response;
	BotArgumentDispatch[EVENT_SAY]                    = handle_bot_say;
	BotArgumentDispatch[EVENT_SIGNAL]                 = handle_bot_signal;
	BotArgumentDispatch[EVENT_SLAY]                   = handle_bot_slay;
	BotArgumentDispatch[EVENT_TARGET_CHANGE]          = handle_bot_target_change;
	BotArgumentDispatch[EVENT_TIMER]                  = handle_bot_timer;
	BotArgumentDispatch[EVENT_TRADE]                  = handle_bot_trade;
	BotArgumentDispatch[EVENT_USE_SKILL]              = handle_bot_use_skill;
	BotArgumentDispatch[EVENT_PAYLOAD]                = handle_bot_payload;
	BotArgumentDispatch[EVENT_EQUIP_ITEM_BOT]         = handle_bot_equip_item;
	BotArgumentDispatch[EVENT_UNEQUIP_ITEM_BOT]       = handle_bot_equip_item;
	BotArgumentDispatch[EVENT_DAMAGE_GIVEN]           = handle_bot_damage;
	BotArgumentDispatch[EVENT_DAMAGE_TAKEN]           = handle_bot_damage;
	BotArgumentDispatch[EVENT_LEVEL_UP]               = handle_bot_level_up;
	BotArgumentDispatch[EVENT_LEVEL_DOWN]             = handle_bot_level_down;
	BotArgumentDispatch[EVENT_TIMER_PAUSE]            = handle_bot_timer_pause_resume_start;
	BotArgumentDispatch[EVENT_TIMER_RESUME]           = handle_bot_timer_pause_resume_start;
	BotArgumentDispatch[EVENT_TIMER_START]            = handle_bot_timer_pause_resume_start;
	BotArgumentDispatch[EVENT_TIMER_STOP]             = handle_bot_timer_stop;
	BotArgumentDispatch[EVENT_ENTITY_VARIABLE_DELETE] = handle_bot_entity_variable;
	BotArgumentDispatch[EVENT_ENTITY_VARIABLE_SET]    = handle_bot_entity_variable;
	BotArgumentDispatch[EVENT_ENTITY_VARIABLE_UPDATE] = handle_bot_entity_variable;
	BotArgumentDispatch[EVENT_SPELL_BLOCKED]          = handle_bot_spell_blocked;

	ZoneArgumentDispatch[EVENT_CLICK_DOOR]     = handle_zone_click_door;
	ZoneArgumentDispatch[EVENT_CLICK_OBJECT]   = handle_zone_click_object;
	ZoneArgumentDispatch[EVENT_DEATH_ZONE]     = handle_zone_death;
	ZoneArgumentDispatch[EVENT_DESPAWN_ZONE]   = handle_zone_despawn;
	ZoneArgumentDispatch[EVENT_ENTER_ZONE]     = handle_zone_enter;
	ZoneArgumentDispatch[EVENT_LOOT_ZONE]      = handle_zone_loot;
	ZoneArgumentDispatch[EVENT_PAYLOAD]        = handle_zone_payload;
	ZoneArgumentDispatch[EVENT_PLAYER_PICKUP]  = handle_zone_pickup;
	ZoneArgumentDispatch[EVENT_POPUP_RESPONSE] = handle_zone_popup;
	ZoneArgumentDispatch[EVENT_SIGNAL]         = handle_zone_signal;
	ZoneArgumentDispatch[EVENT_SPAWN_ZONE]     = handle_zone_spawn;
	ZoneArgumentDispatch[EVENT_TIMER]          = handle_zone_timer;
	ZoneArgumentDispatch[EVENT_TIMER_PAUSE]    = handle_zone_timer_pause_resume_start;
	ZoneArgumentDispatch[EVENT_TIMER_RESUME]   = handle_zone_timer_pause_resume_start;
	ZoneArgumentDispatch[EVENT_TIMER_START]    = handle_zone_timer_pause_resume_start;
	ZoneArgumentDispatch[EVENT_TIMER_STOP]     = handle_zone_timer_stop;

	L = nullptr;
}

LuaParser::~LuaParser() {
	// valgrind didn't like when we didn't clean these up :P
	lua_encounters.clear();
	lua_encounter_events_registered.clear();
	lua_encounters_loaded.clear();
	if(L) {
		lua_close(L);
	}
}

int LuaParser::EventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						std::vector<std::any> *extra_pointers) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return 0;
	}

	if(!npc) {
		return 0;
	}

	if(!HasQuestSub(npc->GetNPCTypeID(), evt)) {
		return 0;
	}

	std::string package_name = "npc_" + std::to_string(npc->GetNPCTypeID());
	return _EventNPC(package_name, evt, npc, init, data, extra_data, extra_pointers);
}

int LuaParser::EventGlobalNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data,
							  std::vector<std::any> *extra_pointers) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return 0;
	}

	if(!npc) {
		return 0;
	}

	if(!HasGlobalQuestSub(evt)) {
		return 0;
	}

	return _EventNPC("global_npc", evt, npc, init, data, extra_data, extra_pointers);
}

int LuaParser::_EventNPC(std::string package_name, QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						 std::vector<std::any> *extra_pointers, luabind::adl::object *l_func) {
	const char *sub_name = LuaEvents[evt];

	int start = lua_gettop(L);

	try {
		int npop = 2;
		PushErrorHandler(L);
		if(l_func != nullptr) {
			l_func->push(L);
		} else {
			lua_getfield(L, LUA_REGISTRYINDEX, package_name.c_str());
			lua_getfield(L, -1, sub_name);
			npop = 3;
		}

		lua_createtable(L, 0, 0);
		//always push self
		Lua_NPC l_npc(npc);
		luabind::adl::object l_npc_o = luabind::adl::object(L, l_npc);
		l_npc_o.push(L);
		lua_setfield(L, -2, "self");

		auto arg_function = NPCArgumentDispatch[evt];
		arg_function(this, L, npc, init, data, extra_data, extra_pointers);
		Client *c = (init && init->IsClient()) ? init->CastToClient() : nullptr;

		RunningQuest q;

		q.owner     = npc;
		q.initiator = c;

		quest_manager.StartQuest(q);

		if(lua_pcall(L, 1, 1, start + 1)) {
			std::string error = lua_tostring(L, -1);
			AddError(error);
			quest_manager.EndQuest();
			lua_pop(L, npop);
			return 0;
		}
		quest_manager.EndQuest();

		if(lua_isnumber(L, -1)) {
			int ret = static_cast<int>(lua_tointeger(L, -1));
			lua_pop(L, npop);
			return ret;
		}

		lua_pop(L, npop);
	} catch(std::exception &ex) {
		AddError(fmt::format("Lua Exception | [{}] for NPC [{}] in [{}]: {}", sub_name, npc->GetNPCTypeID(), package_name, ex.what()));

		//Restore our stack to the best of our ability
		int end = lua_gettop(L);
		int n = end - start;
		if(n > 0) {
			lua_pop(L, n);
		}
	}

	return 0;
}

int LuaParser::EventPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return 0;
	}

	if(!client) {
		return 0;
	}

	if(!PlayerHasQuestSub(evt)) {
		return 0;
	}

	return _EventPlayer("player", evt, client, data, extra_data, extra_pointers);
}

int LuaParser::EventGlobalPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return 0;
	}

	if(!client) {
		return 0;
	}

	if(!GlobalPlayerHasQuestSub(evt)) {
		return 0;
	}

	return _EventPlayer("global_player", evt, client, data, extra_data, extra_pointers);
}

int LuaParser::_EventPlayer(std::string package_name, QuestEventID evt, Client *client, std::string data, uint32 extra_data,
							std::vector<std::any> *extra_pointers, luabind::adl::object *l_func) {
	const char *sub_name = LuaEvents[evt];
	int start = lua_gettop(L);

	try {
		int npop = 2;
		PushErrorHandler(L);
		if(l_func != nullptr) {
			l_func->push(L);
		} else {
			lua_getfield(L, LUA_REGISTRYINDEX, package_name.c_str());
			lua_getfield(L, -1, sub_name);
			npop = 3;
		}

		lua_createtable(L, 0, 0);
		//push self
		Lua_Client l_client(client);
		luabind::adl::object l_client_o = luabind::adl::object(L, l_client);
		l_client_o.push(L);
		lua_setfield(L, -2, "self");

		auto arg_function = PlayerArgumentDispatch[evt];
		arg_function(this, L, client, data, extra_data, extra_pointers);

		RunningQuest q;

		q.owner     = client;
		q.initiator = client;

		quest_manager.StartQuest(q);

		if(lua_pcall(L, 1, 1, start + 1)) {
			std::string error = lua_tostring(L, -1);
			AddError(error);
			quest_manager.EndQuest();
			lua_pop(L, npop);
			return 0;
		}
		quest_manager.EndQuest();

		if(lua_isnumber(L, -1)) {
			int ret = static_cast<int>(lua_tointeger(L, -1));
			lua_pop(L, npop);
			return ret;
		}

		lua_pop(L, npop);
	} catch(std::exception &ex) {
		AddError(fmt::format("Lua Exception | [{}] for Player in [{}]: {}", sub_name, package_name, ex.what()));

		//Restore our stack to the best of our ability
		int end = lua_gettop(L);
		int n = end - start;
		if(n > 0) {
			lua_pop(L, n);
		}
	}

	return 0;
}

int LuaParser::EventItem(QuestEventID evt, Client *client, EQ::ItemInstance *item, Mob *mob, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return 0;
	}

	if(!item) {
		return 0;
	}

	if(!ItemHasQuestSub(item, evt)) {
		return 0;
	}

	std::string package_name = "item_";
	package_name += std::to_string(item->GetID());
	return _EventItem(package_name, evt, client, item, mob, data, extra_data, extra_pointers);
}

int LuaParser::_EventItem(std::string package_name, QuestEventID evt, Client *client, EQ::ItemInstance *item, Mob *mob,
						  std::string data, uint32 extra_data, std::vector<std::any> *extra_pointers, luabind::adl::object *l_func) {
	const char *sub_name = LuaEvents[evt];

	int start = lua_gettop(L);

	try {
		int npop = 2;
		PushErrorHandler(L);
		if(l_func != nullptr) {
			l_func->push(L);
		} else {
			lua_getfield(L, LUA_REGISTRYINDEX, package_name.c_str());
			lua_getfield(L, -1, sub_name);
			npop = 3;
		}

		lua_createtable(L, 0, 0);
		//always push self
		Lua_ItemInst l_item(item);
		luabind::adl::object l_item_o = luabind::adl::object(L, l_item);
		l_item_o.push(L);
		lua_setfield(L, -2, "self");

		Lua_Client l_client(client);
		luabind::adl::object l_client_o = luabind::adl::object(L, l_client);
		l_client_o.push(L);
		lua_setfield(L, -2, "owner");

		//redo this arg function
		auto arg_function = ItemArgumentDispatch[evt];
		arg_function(this, L, client, item, mob, data, extra_data, extra_pointers);

		RunningQuest q;

		q.owner     = client;
		q.initiator = client;
		q.questitem = item;

		quest_manager.StartQuest(q);

		if(lua_pcall(L, 1, 1, start + 1)) {
			std::string error = lua_tostring(L, -1);
			AddError(error);
			quest_manager.EndQuest();
			lua_pop(L, npop);
			return 0;
		}
		quest_manager.EndQuest();

		if(lua_isnumber(L, -1)) {
			int ret = static_cast<int>(lua_tointeger(L, -1));
			lua_pop(L, npop);
			return ret;
		}

		lua_pop(L, npop);
	} catch(std::exception &ex) {
		uint32_t item_id = item->GetItem() ? item->GetItem()->ID : 0;
		AddError(fmt::format("Lua Exception | [{}] for Item [{}] in [{}]: {}", sub_name, item_id, package_name, ex.what()));

		//Restore our stack to the best of our ability
		int end = lua_gettop(L);
		int n = end - start;
		if(n > 0) {
			lua_pop(L, n);
		}
	}

	return 0;
}

int LuaParser::EventSpell(QuestEventID evt, Mob* mob, Client *client, uint32 spell_id, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return 0;
	}

	std::string package_name = "spell_" + std::to_string(spell_id);

	if(!SpellHasQuestSub(spell_id, evt)) {
		return 0;
	}

	return _EventSpell(package_name, evt, mob, client, spell_id, data, extra_data, extra_pointers);
}

int LuaParser::_EventSpell(std::string package_name, QuestEventID evt, Mob* mob, Client *client, uint32 spell_id, std::string data, uint32 extra_data,
						   std::vector<std::any> *extra_pointers, luabind::adl::object *l_func) {
	const char *sub_name = LuaEvents[evt];

	int start = lua_gettop(L);

	try {
		int npop = 2;
		PushErrorHandler(L);
		if(l_func != nullptr) {
			l_func->push(L);
		} else {
			lua_getfield(L, LUA_REGISTRYINDEX, package_name.c_str());
			lua_getfield(L, -1, sub_name);
			npop = 3;
		}

		lua_createtable(L, 0, 0);

		//always push self even if invalid
		if(IsValidSpell(spell_id)) {
			Lua_Spell l_spell(&spells[spell_id]);
			luabind::adl::object l_spell_o = luabind::adl::object(L, l_spell);
			l_spell_o.push(L);
		} else {
			Lua_Spell l_spell(nullptr);
			luabind::adl::object l_spell_o = luabind::adl::object(L, l_spell);
			l_spell_o.push(L);
		}
		lua_setfield(L, -2, "self");

		auto arg_function = SpellArgumentDispatch[evt];
		arg_function(this, L, mob, client, spell_id, data, extra_data, extra_pointers);

		RunningQuest q;

		q.owner      = client;
		q.initiator  = client;
		q.questspell = const_cast<SPDat_Spell_Struct*>(&spells[spell_id]);

		quest_manager.StartQuest(q);

		if(lua_pcall(L, 1, 1, start + 1)) {
			std::string error = lua_tostring(L, -1);
			AddError(error);
			quest_manager.EndQuest();
			lua_pop(L, npop);
			return 0;
		}
		quest_manager.EndQuest();

		if(lua_isnumber(L, -1)) {
			int ret = static_cast<int>(lua_tointeger(L, -1));
			lua_pop(L, npop);
			return ret;
		}

		lua_pop(L, npop);
	} catch(std::exception &ex) {
		AddError(fmt::format("Lua Exception | [{}] for Spell [{}] in [{}]: {}", sub_name, spell_id, package_name, ex.what()));

		//Restore our stack to the best of our ability
		int end = lua_gettop(L);
		int n = end - start;
		if(n > 0) {
			lua_pop(L, n);
		}
	}

	return 0;
}

int LuaParser::EventEncounter(QuestEventID evt, std::string encounter_name, std::string data, uint32 extra_data, std::vector<std::any> *extra_pointers) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return 0;
	}

	std::string package_name = "encounter_" + encounter_name;

	if(!EncounterHasQuestSub(encounter_name, evt)) {
		return 0;
	}

	return _EventEncounter(package_name, evt, encounter_name, data, extra_data, extra_pointers);
}

int LuaParser::_EventEncounter(std::string package_name, QuestEventID evt, std::string encounter_name, std::string data, uint32 extra_data,
							   std::vector<std::any> *extra_pointers) {
	const char *sub_name = LuaEvents[evt];

	int start = lua_gettop(L);

	try {
		PushErrorHandler(L);
		lua_getfield(L, LUA_REGISTRYINDEX, package_name.c_str());
		lua_getfield(L, -1, sub_name);

		lua_createtable(L, 0, 0);
		lua_pushstring(L, encounter_name.c_str());
		lua_setfield(L, -2, "name");

		Encounter *enc = lua_encounters[encounter_name];

		auto arg_function = EncounterArgumentDispatch[evt];
		arg_function(this, L, enc, data, extra_data, extra_pointers);

		RunningQuest q;

		q.owner     = enc;
		q.encounter = encounter_name;

		quest_manager.StartQuest(q);

		if(lua_pcall(L, 1, 1, start + 1)) {
			std::string error = lua_tostring(L, -1);
			AddError(error);
			quest_manager.EndQuest();
			lua_pop(L, 3);
			return 0;
		}
		quest_manager.EndQuest();

		if(lua_isnumber(L, -1)) {
			int ret = static_cast<int>(lua_tointeger(L, -1));
			lua_pop(L, 3);
			return ret;
		}

		lua_pop(L, 3);
	} catch(std::exception &ex) {
		AddError(fmt::format("Lua Exception | [{}] for Encounter [{}]: {}", sub_name, encounter_name, ex.what()));

		//Restore our stack to the best of our ability
		int end = lua_gettop(L);
		int n = end - start;
		if(n > 0) {
			lua_pop(L, n);
		}
	}

	return 0;
}

bool LuaParser::HasQuestSub(uint32 npc_id, QuestEventID evt) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return false;
	}

	std::string package_name = "npc_" + std::to_string(npc_id);

	const char *subname = LuaEvents[evt];
	return HasFunction(subname, package_name);
}

bool LuaParser::HasGlobalQuestSub(QuestEventID evt) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return false;
	}

	const char *subname = LuaEvents[evt];
	return HasFunction(subname, "global_npc");
}

bool LuaParser::PlayerHasQuestSub(QuestEventID evt) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return false;
	}

	const char *subname = LuaEvents[evt];
	return HasFunction(subname, "player");
}

bool LuaParser::GlobalPlayerHasQuestSub(QuestEventID evt) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return false;
	}

	const char *subname = LuaEvents[evt];
	return HasFunction(subname, "global_player");
}

bool LuaParser::SpellHasQuestSub(uint32 spell_id, QuestEventID evt) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return false;
	}

	std::string package_name = "spell_" + std::to_string(spell_id);

	const char *subname = LuaEvents[evt];
	return HasFunction(subname, package_name);
}

bool LuaParser::ItemHasQuestSub(EQ::ItemInstance *itm, QuestEventID evt) {
	if (itm == nullptr) {
		return false;
	}
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return false;
	}

	std::string package_name = "item_";
	package_name += std::to_string(itm->GetID());

	const char *subname = LuaEvents[evt];
	return HasFunction(subname, package_name);
}

bool LuaParser::EncounterHasQuestSub(std::string encounter_name, QuestEventID evt) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return false;
	}

	std::string package_name = "encounter_" + encounter_name;

	const char *subname = LuaEvents[evt];
	return HasFunction(subname, package_name);
}

void LuaParser::LoadNPCScript(std::string filename, int npc_id) {
	std::string package_name = "npc_" + std::to_string(npc_id);

	LoadScript(filename, package_name);
}

void LuaParser::LoadGlobalNPCScript(std::string filename) {
	LoadScript(filename, "global_npc");
}

void LuaParser::LoadPlayerScript(std::string filename) {
	LoadScript(filename, "player");
}

void LuaParser::LoadGlobalPlayerScript(std::string filename) {
	LoadScript(filename, "global_player");
}

void LuaParser::LoadItemScript(std::string filename, EQ::ItemInstance *item) {
	if (item == nullptr)
		return;
	std::string package_name = "item_";
	package_name += std::to_string(item->GetID());

	LoadScript(filename, package_name);
}

void LuaParser::LoadSpellScript(std::string filename, uint32 spell_id) {
	std::string package_name = "spell_" + std::to_string(spell_id);

	LoadScript(filename, package_name);
}

void LuaParser::LoadEncounterScript(std::string filename, std::string encounter_name) {
	std::string package_name = "encounter_" + encounter_name;

	LoadScript(filename, package_name);
}

void LuaParser::AddVar(std::string name, std::string val) {
	vars_[name] = val;
}

std::string LuaParser::GetVar(std::string name) {
	auto iter = vars_.find(name);
	if(iter != vars_.end()) {
		return iter->second;
	}

	return std::string();
}

void LuaParser::Init() {
	ReloadQuests();
}

void LuaParser::ReloadQuests() {
	loaded_.clear();
	errors_.clear();
	mods_.clear();
	lua_encounter_events_registered.clear();
	lua_encounters_loaded.clear();

	for (auto encounter : lua_encounters) {
		encounter.second->Depop();
	}

	lua_encounters.clear();
	// so the Depop function above depends on the Process being called again so ...
	// And there is situations where it wouldn't be :P
	entity_list.EncounterProcess();

	if(L) {
		lua_close(L);
	}

	L = luaL_newstate();
	luaL_openlibs(L);

	auto top = lua_gettop(L);

	if(luaopen_bit(L) != 1) {
		std::string error = lua_tostring(L, -1);
		AddError(error);
	}

	if(luaL_dostring(L, "math.randomseed(os.time())")) {
		std::string error = lua_tostring(L, -1);
		AddError(error);
	}

#ifdef SANITIZE_LUA_LIBS
	//io
	lua_pushnil(L);
	//lua_setglobal(L, "io");

	//some os/debug are okay some are not
	lua_getglobal(L, "os");
	lua_pushnil(L);
	lua_setfield(L, -2, "exit");
	lua_pushnil(L);
	lua_setfield(L, -2, "execute");
	lua_pushnil(L);
	lua_setfield(L, -2, "getenv");
	lua_pushnil(L);
	lua_setfield(L, -2, "remove");
	lua_pushnil(L);
	lua_setfield(L, -2, "rename");
	lua_pushnil(L);
	lua_setfield(L, -2, "setlocale");
	lua_pushnil(L);
	lua_setfield(L, -2, "tmpname");
	lua_pop(L, 1);

	lua_pushnil(L);
	lua_setglobal(L, "collectgarbage");

	lua_pushnil(L);
	lua_setglobal(L, "loadfile");

#endif

	// lua 5.2+ defines these
#if defined(LUA_VERSION_MAJOR) && defined(LUA_VERSION_MINOR)
	const char lua_version[] = LUA_VERSION_MAJOR "." LUA_VERSION_MINOR;
#elif LUA_VERSION_NUM == 501
	const char lua_version[] = "5.1";
#else
#error Incompatible lua version
#endif

#ifdef _WINDOWS
	const char libext[] = ".dll";
#else
	// lua doesn't care OSX doesn't use sonames
	const char libext[] = ".so";
#endif

	lua_getglobal(L, "package");
	lua_getfield(L, -1, "path");
	std::string module_path = lua_tostring(L, -1);
	lua_pop(L, 1);

	for (const auto& dir : PathManager::Instance()->GetLuaModulePaths()) {
		module_path += fmt::format(";{}/?.lua", dir);
		module_path += fmt::format(";{}/?/init.lua", dir);
		module_path += fmt::format(";{}/share/lua/{}/?.lua", dir, lua_version);
		module_path += fmt::format(";{}/share/lua/{}/?/init.lua", dir, lua_version);
	}

	lua_pushstring(L, module_path.c_str());
	lua_setfield(L, -2, "path");
	lua_pop(L, 1);

	lua_getglobal(L, "package");
	lua_getfield(L, -1, "cpath");
	module_path = lua_tostring(L, -1);
	lua_pop(L, 1);

	for (const auto& dir : PathManager::Instance()->GetLuaModulePaths()) {
		module_path += fmt::format(";{}/?{}", dir, libext);
		module_path += fmt::format(";{}/lib/lua/{}/?{}", dir, lua_version, libext);
	}

	lua_pushstring(L, module_path.c_str());
	lua_setfield(L, -2, "cpath");
	lua_pop(L, 1);

	MapFunctions(L);

	// load init
	for (auto& dir : PathManager::Instance()->GetQuestPaths()) {
		std::string filename = fmt::format("{}/{}/script_init.lua", dir, QUEST_GLOBAL_DIRECTORY);

		FILE* f = fopen(filename.c_str(), "r");
		if (f) {
			fclose(f);

			if (luaL_dofile(L, filename.c_str())) {
				std::string error = lua_tostring(L, -1);
				AddError(error);
			}
		}
	}

	//zone init - always loads after global
	if (zone) {
		for (auto& dir : PathManager::Instance()->GetQuestPaths()) {
			std::string zone_script = fmt::format(
				"{}/{}/script_init_v{}.lua",
				dir,
				zone->GetShortName(),
				zone->GetInstanceVersion()
			);

			FILE* f = fopen(zone_script.c_str(), "r");
			if (f) {
				fclose(f);

				if (luaL_dofile(L, zone_script.c_str())) {
					std::string error = lua_tostring(L, -1);
					AddError(error);
				}
			}
			else {
				zone_script = fmt::format("{}/{}/script_init.lua", dir, zone->GetShortName());

				f = fopen(zone_script.c_str(), "r");
				if (f) {
					fclose(f);

					if (luaL_dofile(L, zone_script.c_str())) {
						std::string error = lua_tostring(L, -1);
						AddError(error);
					}
				}
			}
		}
	}

	FILE *load_order = fopen(fmt::format("{}/load_order.txt", PathManager::Instance()->GetLuaModsPath()).c_str(), "r");
	if (load_order) {
		char file_name[256] = { 0 };
		while (fgets(file_name, 256, load_order) != nullptr) {
			for (char & i : file_name) {
				auto c = i;
				if (c == '\n' || c == '\r' || c == ' ') {
					i = 0;
					break;
				}
			}

			LoadScript(fmt::format("{}/{}", PathManager::Instance()->GetLuaModsPath(), std::string(file_name)), file_name);
			mods_.emplace_back(L, this, file_name);
		}

		fclose(load_order);
	}

	auto end = lua_gettop(L);
	int n = end - top;
	if (n > 0) {
		lua_pop(L, n);
	}
}

/*
 * This function is intended only to clean up lua_encounters when the Encounter object is
 * about to be destroyed. It won't clean up memory else where, since the caller of this
 * function is responsible for that
 */
void LuaParser::RemoveEncounter(const std::string &name)
{
	lua_encounters.erase(name);
}

void LuaParser::LoadScript(std::string filename, std::string package_name) {
	auto iter = loaded_.find(package_name);
	if(iter != loaded_.end()) {
		return;
	}

	auto top = lua_gettop(L);
	PushErrorHandler(L);
	if(luaL_loadfile(L, filename.c_str())) {
		std::string error = lua_tostring(L, -1);
		AddError(error);
		lua_pop(L, 2);
		return;
	}

	//This makes an env table named: package_name
	//And makes it so we can see the global table _G from it
	//Then sets it so this script is called from that table as an env

	lua_createtable(L, 0, 0); // anon table
	lua_getglobal(L, "_G"); // get _G
	lua_setfield(L, -2, "__index"); //anon table.__index = _G

	lua_pushvalue(L, -1); //copy table to top of stack
	lua_setmetatable(L, -2); //setmetatable(anon_table, copied table)

	lua_pushvalue(L, -1); //put the table we made into the registry
	lua_setfield(L, LUA_REGISTRYINDEX, package_name.c_str());

	lua_setfenv(L, -2); //set the env to the table we made

	if(lua_pcall(L, 0, 0, top + 1)) {
		std::string error = lua_tostring(L, -1);
		AddError(error);
		lua_pop(L, 2);
	}
	else {
		loaded_[package_name] = true;
	}

	auto end = lua_gettop(L);
	int n = end - top;
	if (n > 0) {
		lua_pop(L, n);
	}
}

bool LuaParser::HasFunction(std::string subname, std::string package_name) {
	//std::transform(subname.begin(), subname.end(), subname.begin(), ::tolower);

	auto iter = loaded_.find(package_name);
	if(iter == loaded_.end()) {
		return false;
	}

	lua_getfield(L, LUA_REGISTRYINDEX, package_name.c_str());
	lua_getfield(L, -1, subname.c_str());

	if(lua_isfunction(L, -1)) {
		lua_pop(L, 2);
		return true;
	}

	lua_pop(L, 2);
	return false;
}

bool LuaParser::HasEncounterSub(const std::string& package_name, QuestEventID evt)
{
	auto it = lua_encounter_events_registered.find(package_name);
	if (it != lua_encounter_events_registered.end()) {
		for (auto & riter : it->second) {
			if (riter.event_id == evt) {
				return true;
			}
		}
	}
	return false;
}

void LuaParser::MapFunctions(lua_State *L) {

	try {
		luabind::open(L);

		luabind::module(L)
		[(
			lua_register_general(),
			lua_register_random(),
			lua_register_events(),
			lua_register_faction(),
			lua_register_slot(),
			lua_register_material(),
			lua_register_client_version(),
			lua_register_appearance(),
			lua_register_classes(),
			lua_register_skills(),
			lua_register_bodytypes(),
			lua_register_filters(),
			lua_register_message_types(),
			lua_register_zone_types(),
			lua_register_languages(),
			lua_register_entity(),
			lua_register_encounter(),
			lua_register_mob(),
			lua_register_special_abilities(),
			lua_register_npc(),
			lua_register_client(),
			lua_register_bot(),
			lua_register_merc(),
			lua_register_inventory(),
			lua_register_inventory_where(),
			lua_register_iteminst(),
			lua_register_item(),
			lua_register_spell(),
			lua_register_spawn(),
			lua_register_hate_entry(),
			lua_register_hate_list(),
			lua_register_entity_list(),
			lua_register_mob_list(),
			lua_register_client_list(),
			lua_register_bot_list(),
			lua_register_npc_list(),
			lua_register_corpse_list(),
			lua_register_object_list(),
			lua_register_door_list(),
			lua_register_spawn_list(),
			lua_register_corpse_loot_list(),
			lua_register_npc_loot_list(),
			lua_register_group(),
			lua_register_raid(),
			lua_register_corpse(),
			lua_register_door(),
			lua_register_object(),
			lua_register_packet(),
			lua_register_packet_opcodes(),
			lua_register_stat_bonuses(),
			lua_register_rulei(),
			lua_register_ruler(),
			lua_register_ruleb(),
			lua_register_rules(),
			lua_register_journal_speakmode(),
			lua_register_journal_mode(),
			lua_register_expedition(),
			lua_register_expedition_lock_messages(),
			lua_register_buff(),
			lua_register_exp_source(),
			lua_register_database(),
			lua_register_zone()
		)];

	} catch(std::exception &ex) {
		std::string error = ex.what();
		AddError(error);
	}
}

int LuaParser::DispatchEventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data,
								 std::vector<std::any> *extra_pointers) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return 0;
	}

	if(!npc)
		return 0;

	std::string package_name = "npc_" + std::to_string(npc->GetNPCTypeID());
	int ret = 0;

	auto iter = lua_encounter_events_registered.find(package_name);
	if(iter != lua_encounter_events_registered.end()) {
		auto riter = iter->second.begin();
		while(riter != iter->second.end()) {
			if(riter->event_id == evt) {
				std::string package_name = "encounter_" + riter->encounter_name;
				int i = _EventNPC(package_name, evt, npc, init, data, extra_data, extra_pointers, &riter->lua_reference);
                if(i != 0)
                    ret = i;
			}
			++riter;
		}
	}

	iter = lua_encounter_events_registered.find("npc_-1");
	if(iter == lua_encounter_events_registered.end()) {
		return ret;
	}

	auto riter = iter->second.begin();
	while(riter != iter->second.end()) {
		if(riter->event_id == evt) {
			std::string package_name = "encounter_" + riter->encounter_name;
			int i = _EventNPC(package_name, evt, npc, init, data, extra_data, extra_pointers, &riter->lua_reference);
            if(i != 0)
                ret = i;
		}
		++riter;
	}

    return ret;
}

int LuaParser::DispatchEventPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data,
									std::vector<std::any> *extra_pointers) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return 0;
	}

	std::string package_name = "player";

	auto iter = lua_encounter_events_registered.find(package_name);
	if(iter == lua_encounter_events_registered.end()) {
		return 0;
	}

    int ret = 0;
	auto riter = iter->second.begin();
	while(riter != iter->second.end()) {
		if(riter->event_id == evt) {
			std::string package_name = "encounter_" + riter->encounter_name;
			int i = _EventPlayer(package_name, evt, client, data, extra_data, extra_pointers, &riter->lua_reference);
            if(i != 0)
                ret = i;
		}
		++riter;
	}

    return ret;
}

int LuaParser::DispatchEventItem(QuestEventID evt, Client *client, EQ::ItemInstance *item, Mob *mob, std::string data, uint32 extra_data,
								  std::vector<std::any> *extra_pointers) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return 0;
	}

	if(!item)
		return 0;

	std::string package_name = "item_";
	package_name += std::to_string(item->GetID());
	int ret = 0;

	auto iter = lua_encounter_events_registered.find(package_name);
	if(iter != lua_encounter_events_registered.end()) {
		auto riter = iter->second.begin();
		while(riter != iter->second.end()) {
			if(riter->event_id == evt) {
				std::string package_name = "encounter_" + riter->encounter_name;
				int i = _EventItem(package_name, evt, client, item, mob, data, extra_data, extra_pointers, &riter->lua_reference);
                if(i != 0)
                    ret = i;
			}
			++riter;
		}
	}

	iter = lua_encounter_events_registered.find("item_-1");
	if(iter == lua_encounter_events_registered.end()) {
		return ret;
	}

	auto riter = iter->second.begin();
	while(riter != iter->second.end()) {
		if(riter->event_id == evt) {
			std::string package_name = "encounter_" + riter->encounter_name;
			int i = _EventItem(package_name, evt, client, item, mob, data, extra_data, extra_pointers, &riter->lua_reference);
            if(i != 0)
                ret = i;
		}
		++riter;
	}
    return ret;
}

int LuaParser::DispatchEventSpell(QuestEventID evt, Mob* mob, Client *client, uint32 spell_id, std::string data, uint32 extra_data,
								   std::vector<std::any> *extra_pointers) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return 0;
	}

	std::string package_name = "spell_" + std::to_string(spell_id);

    int ret = 0;
	auto iter = lua_encounter_events_registered.find(package_name);
	if(iter != lua_encounter_events_registered.end()) {
	    auto riter = iter->second.begin();
		while(riter != iter->second.end()) {
			if(riter->event_id == evt) {
				std::string package_name = "encounter_" + riter->encounter_name;
				int i = _EventSpell(package_name, evt, mob, client, spell_id, data, extra_data, extra_pointers, &riter->lua_reference);
                if(i != 0) {
                    ret = i;
                }
			}
			++riter;
		}
	}

	iter = lua_encounter_events_registered.find("spell_-1");
	if(iter == lua_encounter_events_registered.end()) {
		return ret;
	}

	auto riter = iter->second.begin();
	while(riter != iter->second.end()) {
		if(riter->event_id == evt) {
			std::string package_name = "encounter_" + riter->encounter_name;
			int i = _EventSpell(package_name, evt, mob, client, spell_id, data, extra_data, extra_pointers, &riter->lua_reference);
            if(i != 0)
                ret = i;
		}
		++riter;
	}
    return ret;
}

QuestEventID LuaParser::ConvertLuaEvent(QuestEventID evt) {
	switch(evt) {
	case EVENT_SLAY:
	case EVENT_NPC_SLAY:
		return EVENT_SLAY;
		break;
	case EVENT_SPELL_EFFECT_BOT:
	case EVENT_SPELL_EFFECT_CLIENT:
	case EVENT_SPELL_EFFECT_NPC:
		return EVENT_SPELL_EFFECT_CLIENT;
		break;
	case EVENT_SPELL_EFFECT_BUFF_TIC_BOT:
	case EVENT_SPELL_EFFECT_BUFF_TIC_CLIENT:
	case EVENT_SPELL_EFFECT_BUFF_TIC_NPC:
		return EVENT_SPELL_EFFECT_BUFF_TIC_CLIENT;
		break;
	case EVENT_AGGRO:
	case EVENT_ATTACK:
		return _LargestEventID;
		break;
	default:
		return evt;
	}
}

void LuaParser::MeleeMitigation(Mob *self, Mob *attacker, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault)
{
	for (auto &mod : mods_) {
		mod.MeleeMitigation(self, attacker, hit, opts, ignoreDefault);
	}
}

void LuaParser::ApplyDamageTable(Mob *self, DamageHitInfo &hit, bool &ignoreDefault)
{
	for (auto &mod : mods_) {
		mod.ApplyDamageTable(self, hit, ignoreDefault);
	}
}

bool LuaParser::AvoidDamage(Mob *self, Mob *other, DamageHitInfo &hit, bool & ignoreDefault)
{
	bool retval = false;
	for (auto &mod : mods_) {
		mod.AvoidDamage(self, other, hit, retval, ignoreDefault);
	}
	return retval;
}

bool LuaParser::CheckHitChance(Mob *self, Mob *other, DamageHitInfo &hit, bool &ignoreDefault)
{
	bool retval = false;
	for (auto &mod : mods_) {
		mod.CheckHitChance(self, other, hit, retval, ignoreDefault);
	}
	return retval;
}

void LuaParser::TryCriticalHit(Mob *self, Mob *defender, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault)
{
	for (auto &mod : mods_) {
		mod.TryCriticalHit(self, defender, hit, opts, ignoreDefault);
	}
}

void LuaParser::CommonOutgoingHitSuccess(Mob *self, Mob *other, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault)
{
	for (auto &mod : mods_) {
		mod.CommonOutgoingHitSuccess(self, other, hit, opts, ignoreDefault);
	}
}

uint32 LuaParser::GetRequiredAAExperience(Client *self, bool &ignoreDefault)
{
	uint32 retval = 0;
	for (auto &mod : mods_) {
		mod.GetRequiredAAExperience(self, retval, ignoreDefault);
	}
	return retval;
}

uint32 LuaParser::GetEXPForLevel(Client *self, uint16 level, bool &ignoreDefault)
{
	uint32 retval = 0;
	for (auto &mod : mods_) {
		mod.GetEXPForLevel(self, level, retval, ignoreDefault);
	}
	return retval;
}

uint64 LuaParser::GetExperienceForKill(Client *self, Mob *against, bool &ignoreDefault)
{
	uint64 retval = 0;
	for (auto &mod : mods_) {
		mod.GetExperienceForKill(self, against, retval, ignoreDefault);
	}
	return retval;
}


int64 LuaParser::CommonDamage(Mob *self, Mob* attacker, int64 value, uint16 spell_id, int skill_used, bool avoidable, int8 buff_slot, bool buff_tic, int special, bool &ignore_default)
{
	int64 retval = 0;
	for (auto &mod : mods_) {
		mod.CommonDamage(self, attacker, value, spell_id, skill_used, avoidable, buff_slot, buff_tic, special, retval, ignore_default);
	}
	return retval;
}

uint64 LuaParser::HealDamage(Mob *self, Mob* caster, uint64 value, uint16 spell_id, bool &ignore_default)
{
	uint64 retval = 0;
	for (auto &mod : mods_) {
		mod.HealDamage(self, caster, value, spell_id, retval, ignore_default);
	}
	return retval;
}

bool LuaParser::IsImmuneToSpell(Mob *self, Mob *caster, uint16 spell_id, bool &ignore_default)
{
	bool retval = false;
	for (auto &mod : mods_) {
		mod.IsImmuneToSpell(self, caster, spell_id, retval, ignore_default);
	}
	return retval;
}

int64 LuaParser::CalcSpellEffectValue_formula(Mob *self, uint32 formula, int64 base_value, int64 max_value, int caster_level, uint16 spell_id, int ticsremaining, bool &ignoreDefault)
{
	int64 retval = 0;
	for (auto &mod : mods_) {
		mod.CalcSpellEffectValue_formula(self, formula, base_value, max_value, caster_level, spell_id, ticsremaining, retval, ignoreDefault);
	}
	return retval;
}

int32 LuaParser::UpdatePersonalFaction(Mob *self, int32 npc_value, int32 faction_id, int32 current_value, int32 temp, int32 this_faction_min, int32 this_faction_max, bool &ignore_default)
{
	int32 retval = 0;
	for (auto &mod : mods_) {
		mod.UpdatePersonalFaction(self, npc_value, faction_id, current_value, temp, this_faction_min, this_faction_max, retval, ignore_default);
	}
	return retval;
}

void LuaParser::RegisterBug(Client *self, BaseBugReportsRepository::BugReports bug, bool &ignore_default)
{
	for (auto &mod : mods_) {
		mod.RegisterBug(self, bug, ignore_default);
	}
}


uint64 LuaParser::SetEXP(Mob *self, ExpSource exp_source, uint64 current_exp, uint64 set_exp, bool is_rezz_exp, bool &ignore_default)
{
	uint64 retval = 0;
	for (auto &mod : mods_) {
		mod.SetEXP(self, exp_source, current_exp, set_exp, is_rezz_exp, retval, ignore_default);
	}
	return retval;
}

uint64 LuaParser::SetAAEXP(Mob *self, ExpSource exp_source, uint64 current_aa_exp, uint64 set_aa_exp, bool is_rezz_exp, bool &ignore_default)
{
	uint64 retval = 0;
	for (auto &mod : mods_) {
		mod.SetAAEXP(self, exp_source, current_aa_exp, set_aa_exp, is_rezz_exp, retval, ignore_default);
	}
	return retval;
}

int LuaParser::EventBot(
	QuestEventID evt,
	Bot *bot,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	evt = ConvertLuaEvent(evt);
	if (evt >= _LargestEventID) {
		return 0;
	}

	if (!bot) {
		return 0;
	}

	if (!BotHasQuestSub(evt)) {
		return 0;
	}

	return _EventBot("bot", evt, bot, init, data, extra_data, extra_pointers);
}

int LuaParser::EventGlobalBot(
	QuestEventID evt,
	Bot *bot,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	evt = ConvertLuaEvent(evt);
	if (evt >= _LargestEventID) {
		return 0;
	}

	if (!bot) {
		return 0;
	}

	if (!GlobalBotHasQuestSub(evt)) {
		return 0;
	}

	return _EventBot("global_bot", evt, bot, init, data, extra_data, extra_pointers);
}

int LuaParser::_EventBot(
	std::string package_name,
	QuestEventID evt,
	Bot *bot,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers,
	luabind::adl::object *l_func
) {
	const char *sub_name = LuaEvents[evt];
	int start = lua_gettop(L);

	try {
		int npop = 2;
		PushErrorHandler(L);
		if(l_func != nullptr) {
			l_func->push(L);
		} else {
			lua_getfield(L, LUA_REGISTRYINDEX, package_name.c_str());
			lua_getfield(L, -1, sub_name);
			npop = 3;
		}

		lua_createtable(L, 0, 0);
		//push self
		Lua_Bot l_bot(bot);
		luabind::adl::object l_bot_o = luabind::adl::object(L, l_bot);
		l_bot_o.push(L);
		lua_setfield(L, -2, "self");

		auto arg_function = BotArgumentDispatch[evt];
		arg_function(this, L, bot, init, data, extra_data, extra_pointers);
		auto* c = (init && init->IsClient()) ? init->CastToClient() : nullptr;

		RunningQuest q;

		q.owner      = bot;
		q.initiator  = c;

		quest_manager.StartQuest(q);

		if(lua_pcall(L, 1, 1, start + 1)) {
			std::string error = lua_tostring(L, -1);
			AddError(error);
			quest_manager.EndQuest();
			lua_pop(L, npop);
			return 0;
		}
		quest_manager.EndQuest();

		if(lua_isnumber(L, -1)) {
			int ret = static_cast<int>(lua_tointeger(L, -1));
			lua_pop(L, npop);
			return ret;
		}

		lua_pop(L, npop);
	} catch(std::exception &ex) {
		AddError(fmt::format("Lua Exception | [{}] for Bot [{}] in [{}]: {}", sub_name, bot->GetBotID(), package_name, ex.what()));

		//Restore our stack to the best of our ability
		int end = lua_gettop(L);
		int n = end - start;
		if(n > 0) {
			lua_pop(L, n);
		}
	}

	return 0;
}

int LuaParser::DispatchEventBot(
	QuestEventID evt,
	Bot *bot,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	evt = ConvertLuaEvent(evt);
	if (evt >= _LargestEventID) {
		return 0;
	}

	std::string package_name = "bot";

	auto iter = lua_encounter_events_registered.find(package_name);
	if (iter == lua_encounter_events_registered.end()) {
		return 0;
	}

	int ret = 0;
	auto riter = iter->second.begin();
	while (riter != iter->second.end()) {
		if (riter->event_id == evt) {
			package_name = fmt::format("encounter_{}", riter->encounter_name);
			int i = _EventBot(package_name, evt, bot, init, data, extra_data, extra_pointers, &riter->lua_reference);
			if (i != 0) {
				ret = i;
			}
		}

		++riter;
	}

	return ret;
}

bool LuaParser::BotHasQuestSub(QuestEventID evt) {
	evt = ConvertLuaEvent(evt);
	if (evt >= _LargestEventID) {
		return false;
	}

	const char *subname = LuaEvents[evt];
	return HasFunction(subname, "bot");
}

bool LuaParser::GlobalBotHasQuestSub(QuestEventID evt) {
	evt = ConvertLuaEvent(evt);
	if (evt >= _LargestEventID) {
		return false;
	}

	const char *subname = LuaEvents[evt];
	return HasFunction(subname, "global_bot");
}

void LuaParser::LoadBotScript(std::string filename) {
	LoadScript(filename, "bot");
}

void LuaParser::LoadGlobalBotScript(std::string filename) {
	LoadScript(filename, "global_bot");
}

int LuaParser::EventMerc(
	QuestEventID evt,
	Merc *merc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	evt = ConvertLuaEvent(evt);
	if (evt >= _LargestEventID) {
		return 0;
	}

	if (!merc) {
		return 0;
	}

	if (!MercHasQuestSub(evt)) {
		return 0;
	}

	return _EventMerc("merc", evt, merc, init, data, extra_data, extra_pointers);
}

int LuaParser::EventGlobalMerc(
	QuestEventID evt,
	Merc *merc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	evt = ConvertLuaEvent(evt);
	if (evt >= _LargestEventID) {
		return 0;
	}

	if (!merc) {
		return 0;
	}

	if (!GlobalMercHasQuestSub(evt)) {
		return 0;
	}

	return _EventMerc("global_merc", evt, merc, init, data, extra_data, extra_pointers);
}

int LuaParser::_EventMerc(
	std::string package_name,
	QuestEventID evt,
	Merc *merc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers,
	luabind::adl::object *l_func
) {
	const char *sub_name = LuaEvents[evt];
	int start = lua_gettop(L);

	try {
		int npop = 2;
		PushErrorHandler(L);
		if(l_func != nullptr) {
			l_func->push(L);
		} else {
			lua_getfield(L, LUA_REGISTRYINDEX, package_name.c_str());
			lua_getfield(L, -1, sub_name);
			npop = 3;
		}

		lua_createtable(L, 0, 0);
		//push self
		Lua_Merc l_merc(merc);
		luabind::adl::object l_merc_o = luabind::adl::object(L, l_merc);
		l_merc_o.push(L);
		lua_setfield(L, -2, "self");

		auto arg_function = NPCArgumentDispatch[evt];
		arg_function(this, L, merc, init, data, extra_data, extra_pointers);
		auto* c = (init && init->IsClient()) ? init->CastToClient() : nullptr;

		RunningQuest q;

		q.owner      = merc;
		q.initiator  = c;

		quest_manager.StartQuest(q);

		if(lua_pcall(L, 1, 1, start + 1)) {
			std::string error = lua_tostring(L, -1);
			AddError(error);
			quest_manager.EndQuest();
			lua_pop(L, npop);
			return 0;
		}
		quest_manager.EndQuest();

		if(lua_isnumber(L, -1)) {
			int ret = static_cast<int>(lua_tointeger(L, -1));
			lua_pop(L, npop);
			return ret;
		}

		lua_pop(L, npop);
	} catch(std::exception &ex) {
		AddError(
			fmt::format(
				"Lua Exception | [{}] for Merc [{}] in [{}]: {}",
				sub_name,
				merc->GetID(),
				package_name,
				ex.what()
			)
		);

		//Restore our stack to the best of our ability
		int end = lua_gettop(L);
		int n = end - start;
		if(n > 0) {
			lua_pop(L, n);
		}
	}

	return 0;
}

int LuaParser::DispatchEventMerc(
	QuestEventID evt,
	Merc *merc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	evt = ConvertLuaEvent(evt);
	if (evt >= _LargestEventID) {
		return 0;
	}

	std::string package_name = "merc";

	auto iter = lua_encounter_events_registered.find(package_name);
	if (iter == lua_encounter_events_registered.end()) {
		return 0;
	}

	int ret = 0;
	auto riter = iter->second.begin();
	while (riter != iter->second.end()) {
		if (riter->event_id == evt) {
			package_name = fmt::format("encounter_{}", riter->encounter_name);
			int i = _EventMerc(package_name, evt, merc, init, data, extra_data, extra_pointers, &riter->lua_reference);
			if (i != 0) {
				ret = i;
			}
		}

		++riter;
	}

	return ret;
}

bool LuaParser::MercHasQuestSub(QuestEventID evt) {
	evt = ConvertLuaEvent(evt);
	if (evt >= _LargestEventID) {
		return false;
	}

	const char *subname = LuaEvents[evt];
	return HasFunction(subname, "merc");
}

bool LuaParser::GlobalMercHasQuestSub(QuestEventID evt) {
	evt = ConvertLuaEvent(evt);
	if (evt >= _LargestEventID) {
		return false;
	}

	const char *subname = LuaEvents[evt];
	return HasFunction(subname, "global_merc");
}

void LuaParser::LoadMercScript(std::string filename) {
	LoadScript(filename, "merc");
}

void LuaParser::LoadGlobalMercScript(std::string filename) {
	LoadScript(filename, "global_merc");
}

int LuaParser::EventZone(
	QuestEventID evt,
	Zone *zone,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	evt = ConvertLuaEvent(evt);
	if (evt >= _LargestEventID) {
		return 0;
	}

	if (!zone) {
		return 0;
	}

	if (!ZoneHasQuestSub(evt)) {
		return 0;
	}

	return _EventZone("zone", evt, zone, data, extra_data, extra_pointers);
}

int LuaParser::EventGlobalZone(
	QuestEventID evt,
	Zone *zone,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	evt = ConvertLuaEvent(evt);
	if (evt >= _LargestEventID) {
		return 0;
	}

	if (!zone) {
		return 0;
	}

	if (!GlobalZoneHasQuestSub(evt)) {
		return 0;
	}

	return _EventZone("global_zone", evt, zone, data, extra_data, extra_pointers);
}

int LuaParser::_EventZone(
	std::string package_name,
	QuestEventID evt,
	Zone *zone,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers,
	luabind::adl::object *l_func
) {
	const char *sub_name = LuaEvents[evt];
	int start = lua_gettop(L);

	try {
		int npop = 2;
		PushErrorHandler(L);
		if(l_func != nullptr) {
			l_func->push(L);
		} else {
			lua_getfield(L, LUA_REGISTRYINDEX, package_name.c_str());
			lua_getfield(L, -1, sub_name);
			npop = 3;
		}

		lua_createtable(L, 0, 0);
		//push self
		Lua_Zone l_zone(zone);
		luabind::adl::object l_zone_o = luabind::adl::object(L, l_zone);
		l_zone_o.push(L);
		lua_setfield(L, -2, "self");

		auto arg_function = ZoneArgumentDispatch[evt];
		arg_function(this, L, zone, data, extra_data, extra_pointers);

		RunningQuest q;

		q.zone = zone;

		quest_manager.StartQuest(q);

		if(lua_pcall(L, 1, 1, start + 1)) {
			std::string error = lua_tostring(L, -1);
			AddError(error);
			quest_manager.EndQuest();
			lua_pop(L, npop);
			return 0;
		}
		quest_manager.EndQuest();

		if(lua_isnumber(L, -1)) {
			int ret = static_cast<int>(lua_tointeger(L, -1));
			lua_pop(L, npop);
			return ret;
		}

		lua_pop(L, npop);
	} catch(std::exception &ex) {
		AddError(
			fmt::format(
				"Lua Exception | [{}] for Zone [{}] in [{}]: {}",
				sub_name,
				zone->GetShortName(),
				package_name,
				ex.what()
			)
		);

		//Restore our stack to the best of our ability
		int end = lua_gettop(L);
		int n = end - start;
		if(n > 0) {
			lua_pop(L, n);
		}
	}

	return 0;
}

int LuaParser::DispatchEventZone(
	QuestEventID evt,
	Zone *zone,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	evt = ConvertLuaEvent(evt);
	if (evt >= _LargestEventID) {
		return 0;
	}

	std::string package_name = "zone";

	auto iter = lua_encounter_events_registered.find(package_name);
	if (iter == lua_encounter_events_registered.end()) {
		return 0;
	}

	int ret = 0;
	auto riter = iter->second.begin();
	while (riter != iter->second.end()) {
		if (riter->event_id == evt) {
			package_name = fmt::format("encounter_{}", riter->encounter_name);
			int i = _EventZone(package_name, evt, zone, data, extra_data, extra_pointers, &riter->lua_reference);
			if (i != 0) {
				ret = i;
			}
		}

		++riter;
	}

	return ret;
}

bool LuaParser::ZoneHasQuestSub(QuestEventID evt) {
	evt = ConvertLuaEvent(evt);
	if (evt >= _LargestEventID) {
		return false;
	}

	const char *subname = LuaEvents[evt];
	return HasFunction(subname, "zone");
}

bool LuaParser::GlobalZoneHasQuestSub(QuestEventID evt) {
	evt = ConvertLuaEvent(evt);
	if (evt >= _LargestEventID) {
		return false;
	}

	const char *subname = LuaEvents[evt];
	return HasFunction(subname, "global_zone");
}

void LuaParser::LoadZoneScript(std::string filename) {
	LoadScript(filename, "zone");
}

void LuaParser::LoadGlobalZoneScript(std::string filename) {
	LoadScript(filename, "global_zone");
}

#endif // LUA_EQEMU
