#pragma once

#ifdef LUA_EQEMU

#include "common/types.h"

#include <any>
#include <string>
#include <vector>

class Bot;
class Client;
class Encounter;
class Merc;
class Mob;
class NPC;
class QuestInterface;
class Zone;
struct lua_State;

namespace EQ
{
	class ItemInstance;
}

typedef void(*NPCArgumentHandler)(QuestInterface*, lua_State*, NPC*, Mob*, std::string, uint32, std::vector<std::any>*);
typedef void(*PlayerArgumentHandler)(QuestInterface*, lua_State*, Client*, std::string, uint32, std::vector<std::any>*);
typedef void(*ItemArgumentHandler)(QuestInterface*, lua_State*, Client*, EQ::ItemInstance*, Mob*, std::string, uint32, std::vector<std::any>*);
typedef void(*SpellArgumentHandler)(QuestInterface*, lua_State*, Mob*, Client*, uint32, std::string, uint32, std::vector<std::any>*);
typedef void(*EncounterArgumentHandler)(QuestInterface*, lua_State*, Encounter* encounter, std::string, uint32, std::vector<std::any>*);
typedef void(*BotArgumentHandler)(QuestInterface*, lua_State*, Bot*, Mob*, std::string, uint32, std::vector<std::any>*);
typedef void(*MercArgumentHandler)(QuestInterface*, lua_State*, Merc*, Mob*, std::string, uint32, std::vector<std::any>*);
typedef void(*ZoneArgumentHandler)(QuestInterface*, lua_State*, Zone*, std::string, uint32, std::vector<std::any>*);

// NPC
void handle_npc_event_say(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_npc_event_trade(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_npc_event_hp(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_npc_single_mob(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_npc_single_client(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_npc_task_accepted(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_npc_popup(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_npc_waypoint(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_npc_hate(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_npc_signal(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_npc_timer(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_npc_death(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_npc_cast(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_npc_area(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_npc_null(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_npc_loot_zone(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_npc_spawn_zone(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_npc_payload(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_npc_despawn_zone(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_npc_damage(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_npc_loot_added(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_npc_timer_pause_resume_start(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_npc_timer_stop(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_npc_entity_variable(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_npc_spell_blocked(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_npc_pet_command(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

// Player
void handle_player_say(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_environmental_damage(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_death(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_timer(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_discover_item(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_fish_forage_success(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_click_object(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_click_door(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_signal(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_popup_response(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_pick_up(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_cast(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_task_fail(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_zone(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_duel_win(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_duel_loss(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_loot(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_task_stage_complete(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_task_accepted(
	QuestInterface* parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
);

void handle_player_task_update(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_command(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_combine(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_feign(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_area(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_respawn(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_packet(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_null(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_use_skill(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_test_buff(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_combine_validate(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_bot_command(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_warp(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_quest_combine(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_consider(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_consider_corpse(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_equip_item(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_skill_up(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_language_skill_up(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_alt_currency_merchant(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_merchant(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_inspect(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_aa_buy(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_aa_gain(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_aa_exp_gain(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_exp_gain(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_payload(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_level_up(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_level_down(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_gm_command(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_bot_create(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_augment_insert(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_augment_remove(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_damage(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_item_click(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_destroy_item(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_target_change(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_drop_item(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_memorize_scribe_spell(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_ldon_points_gain_loss(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_alt_currency_gain_loss(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_crystal_gain_loss(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_timer_pause_resume_start(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_timer_stop(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_entity_variable(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_aa_loss(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_spell_blocked(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_read_item(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_connect(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_player_pet_command(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

// Item
void handle_item_click(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	EQ::ItemInstance* item,
	Mob *mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_item_timer(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	EQ::ItemInstance* item,
	Mob *mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_item_proc(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	EQ::ItemInstance* item,
	Mob *mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_item_loot(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	EQ::ItemInstance* item,
	Mob *mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_item_equip(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	EQ::ItemInstance* item,
	Mob *mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_item_augment(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	EQ::ItemInstance* item,
	Mob *mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_item_augment_insert(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	EQ::ItemInstance* item,
	Mob *mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_item_augment_remove(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	EQ::ItemInstance* item,
	Mob *mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_item_null(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	EQ::ItemInstance* item,
	Mob *mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_item_timer_pause_resume_start(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	EQ::ItemInstance* item,
	Mob *mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_item_timer_stop(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	EQ::ItemInstance* item,
	Mob *mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

// Spell
void handle_spell_event(
	QuestInterface *parse,
	lua_State* L,
	Mob* mob,
	Client* client,
	uint32 spell_id,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_translocate_finish(
	QuestInterface *parse,
	lua_State* L,
	Mob* mob,
	Client* client,
	uint32 spell_id,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_spell_null(
	QuestInterface *parse,
	lua_State* L,
	Mob* mob,
	Client* client,
	uint32 spell_id,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

// Encounter
void handle_encounter_timer(
	QuestInterface *parse,
	lua_State* L,
	Encounter* encounter,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_encounter_load(
	QuestInterface *parse,
	lua_State* L,
	Encounter* encounter,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_encounter_unload(
	QuestInterface *parse,
	lua_State* L,
	Encounter* encounter,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_encounter_null(
	QuestInterface *parse,
	lua_State* L,
	Encounter* encounter,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

// Bot
void handle_bot_null(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_bot_cast(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_bot_combat(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_bot_death(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_bot_popup_response(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_bot_say(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_bot_signal(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_bot_slay(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_bot_target_change(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_bot_timer(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_bot_trade(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_bot_use_skill(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_bot_payload(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_bot_equip_item(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_bot_damage(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_bot_level_up(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_bot_level_down(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_bot_timer_pause_resume_start(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_bot_timer_stop(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_bot_entity_variable(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_bot_spell_blocked(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

// Zone
void handle_zone_null(
	QuestInterface *parse,
	lua_State* L,
	Zone* zone,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_zone_click_door(
	QuestInterface *parse,
	lua_State* L,
	Zone* zone,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_zone_click_object(
	QuestInterface *parse,
	lua_State* L,
	Zone* zone,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_zone_death(
	QuestInterface *parse,
	lua_State* L,
	Zone* zone,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_zone_despawn(
	QuestInterface *parse,
	lua_State* L,
	Zone* zone,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_zone_enter(
	QuestInterface *parse,
	lua_State* L,
	Zone* zone,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_zone_loot(
	QuestInterface *parse,
	lua_State* L,
	Zone* zone,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_zone_payload(
	QuestInterface *parse,
	lua_State* L,
	Zone* zone,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_zone_pickup(
	QuestInterface *parse,
	lua_State* L,
	Zone* zone,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_zone_popup(
	QuestInterface *parse,
	lua_State* L,
	Zone* zone,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_zone_signal(
	QuestInterface *parse,
	lua_State* L,
	Zone* zone,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_zone_spawn(
	QuestInterface *parse,
	lua_State* L,
	Zone* zone,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_zone_timer(
	QuestInterface *parse,
	lua_State* L,
	Zone* zone,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_zone_timer_pause_resume_start(
	QuestInterface *parse,
	lua_State* L,
	Zone* zone,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

void handle_zone_timer_stop(
	QuestInterface *parse,
	lua_State* L,
	Zone* zone,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
);

#endif // LUA_EQEMU
