/*  EQEMu:  Everquest Server Emulator
	Copyright (C) 2001-2006  EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifdef EMBPERL
#ifdef EMBPERL_XS

#include "common/content/world_content_service.h"
#include "common/data_bucket.h"
#include "common/events/player_event_logs.h"
#include "common/features.h"
#include "common/misc_functions.h"
#include "common/zone_store.h"
#include "zone/dialogue_window.h"
#include "zone/dynamic_zone.h"
#include "zone/embperl.h"
#include "zone/entity.h"
#include "zone/queryserv.h"
#include "zone/questmgr.h"
#include "zone/worldserver.h"
#include "zone/zone.h"

#include <cctype>

extern Zone      *zone;
extern QueryServ *QServ;
extern WorldServer worldserver;

#ifdef EMBPERL_XS_CLASSES

//Any creation of new Client objects gets the current quest Client
Client* Perl__Client_new()
{
	return quest_manager.GetInitiator();
}

//Any creation of new NPC objects gets the current quest NPC
NPC* Perl__NPC_new()
{
	return quest_manager.GetNPC();
}

//Any creation of new NPC objects gets the current quest NPC
EntityList* Perl__EntityList_new()
{
	return &entity_list;
}

//Any creation of new inventory gets the curreny inventory
EQ::InventoryProfile* Perl__Inventory_new()
{
	return quest_manager.GetInventory();
}

//Any creation of new quest items gets the current quest item
EQ::ItemInstance* Perl__QuestItem_new()
{
	return quest_manager.GetQuestItem();
}

//Any creation of new Spells gets the current Spell
SPDat_Spell_Struct* Perl__Spell_new()
{
	// should be safe, it's read only in perl (could also use proxy lika lua)
	return const_cast<SPDat_Spell_Struct*>(quest_manager.GetQuestSpell());
}

#endif //EMBPERL_XS_CLASSES


void Perl__echo(int emote_color_id, const char* message)
{
	quest_manager.echo(emote_color_id, message);
}

void Perl__say(const char* message)
{
	Journal::Options opts;
	// we currently default to these
	opts.speak_mode   = Journal::SpeakMode::Say;
	opts.journal_mode = Journal::Mode::Log2;
	opts.language     = Language::CommonTongue;
	opts.message_type = Chat::NPCQuestSay;

	quest_manager.say(message, opts);
}

void Perl__say(const char* message, uint8 language_id)
{
	Journal::Options opts;
	opts.speak_mode   = Journal::SpeakMode::Say;
	opts.journal_mode = Journal::Mode::Log2;
	opts.language     = language_id;
	opts.message_type = Chat::NPCQuestSay;

	quest_manager.say(message, opts);
}

void Perl__say(const char* message, uint8 language_id, int message_type)
{
	Journal::Options opts;
	opts.speak_mode   = Journal::SpeakMode::Say;
	opts.journal_mode = Journal::Mode::Log2;
	opts.language     = language_id;
	opts.message_type = message_type;

	quest_manager.say(message, opts);
}

void Perl__say(const char* message, uint8 language_id, int message_type, int speak_mode)
{
	Journal::Options opts;
	opts.speak_mode   = static_cast<Journal::SpeakMode>(speak_mode);
	opts.journal_mode = Journal::Mode::Log2;
	opts.language     = language_id;
	opts.message_type = message_type;

	quest_manager.say(message, opts);
}

void Perl__say(const char* message, uint8 language_id, int message_type, int speak_mode, int journal_mode)
{
	Journal::Options opts;
	opts.speak_mode   = static_cast<Journal::SpeakMode>(speak_mode);
	opts.journal_mode = static_cast<Journal::Mode>(journal_mode);
	opts.language     = language_id;
	opts.message_type = message_type;

	quest_manager.say(message, opts);
}

void Perl__me(const char* message)
{
	quest_manager.me(message);
}

void Perl__summonitem(int item_id)
{
	quest_manager.summonitem(item_id);
}

void Perl__summonitem(int item_id, int charges)
{
	quest_manager.summonitem(item_id, charges);
}

void Perl__write(const char* file, const char* message)
{
	quest_manager.write(file, message);
}

int Perl__spawn(int npc_type_id, int grid_id, int unused, float x, float y, float z)
{
	auto position = glm::vec4(x, y, z, 0.0f);
	Mob* mob = quest_manager.spawn2(npc_type_id, grid_id, unused, position);
	return mob ? mob->GetID() : 0;
}

int Perl__spawn2(int npc_type_id, int grid_id, int unused, float x, float y, float z, float heading)
{
	auto position = glm::vec4(x, y, z, heading);
	Mob* mob = quest_manager.spawn2(npc_type_id, grid_id, unused, position);
	return mob ? mob->GetID() : 0;
}

int Perl__unique_spawn(int npc_type_id, int grid_id, int unused, float x, float y, float z)
{
	auto position = glm::vec4(x, y, z, 0.0f);
	Mob* mob = quest_manager.unique_spawn(npc_type_id, grid_id, unused, position);
	return mob ? mob->GetID() : 0;
}

int Perl__unique_spawn(int npc_type_id, int grid_id, int unused, float x, float y, float z, float heading)
{
	auto position = glm::vec4(x, y, z, heading);
	Mob* mob = quest_manager.unique_spawn(npc_type_id, grid_id, unused, position);
	return mob ? mob->GetID() : 0;
}

int Perl__spawn_from_spawn2(uint32_t spawn2_id)
{
	Mob* mob = quest_manager.spawn_from_spawn2(spawn2_id);
	return mob ? mob->GetID() : 0;
}

void Perl__enable_spawn2(uint32_t spawn2_id)
{
	quest_manager.enable_spawn2(spawn2_id);
}

void Perl__disable_spawn2(uint32_t spawn2_id)
{
	quest_manager.disable_spawn2(spawn2_id);
}

void Perl__setstat(int stat_id, int int_value)
{
	quest_manager.setstat(stat_id, int_value);
}

void Perl__incstat(int stat_id, int int_value)
{
	quest_manager.incstat(stat_id, int_value);
}

int Perl__getinventoryslotid(std::string identifier)
{
	int result = EQ::invslot::SLOT_INVALID;

	for (std::string::size_type i = 0; i < identifier.size(); ++i)
		identifier[i] = std::tolower(identifier[i]);

	if (identifier.find('.') == std::string::npos) {
		if (identifier == "invalid")                    result = EQ::invslot::SLOT_INVALID;
		else if (identifier == "charm")                 result = EQ::invslot::slotCharm;
		else if (identifier == "ear1")                  result = EQ::invslot::slotEar1;
		else if (identifier == "head")                  result = EQ::invslot::slotHead;
		else if (identifier == "face")                  result = EQ::invslot::slotFace;
		else if (identifier == "ear2")                  result = EQ::invslot::slotEar2;
		else if (identifier == "neck")                  result = EQ::invslot::slotNeck;
		else if (identifier == "shoulders")             result = EQ::invslot::slotShoulders;
		else if (identifier == "arms")                  result = EQ::invslot::slotArms;
		else if (identifier == "back")                  result = EQ::invslot::slotBack;
		else if (identifier == "wrist1")                result = EQ::invslot::slotWrist1;
		else if (identifier == "wrist2")                result = EQ::invslot::slotWrist2;
		else if (identifier == "range")                 result = EQ::invslot::slotRange;
		else if (identifier == "hands")                 result = EQ::invslot::slotHands;
		else if (identifier == "primary")               result = EQ::invslot::slotPrimary;
		else if (identifier == "secondary")             result = EQ::invslot::slotSecondary;
		else if (identifier == "finger1")               result = EQ::invslot::slotFinger1;
		else if (identifier == "finger2")               result = EQ::invslot::slotFinger2;
		else if (identifier == "chest")                 result = EQ::invslot::slotChest;
		else if (identifier == "legs")                  result = EQ::invslot::slotLegs;
		else if (identifier == "feet")                  result = EQ::invslot::slotFeet;
		else if (identifier == "waist")                 result = EQ::invslot::slotWaist;
		else if (identifier == "powersource")           result = EQ::invslot::slotPowerSource;
		else if (identifier == "ammo")                  result = EQ::invslot::slotAmmo;
		else if (identifier == "general1")              result = EQ::invslot::slotGeneral1;
		else if (identifier == "general2")              result = EQ::invslot::slotGeneral2;
		else if (identifier == "general3")              result = EQ::invslot::slotGeneral3;
		else if (identifier == "general4")              result = EQ::invslot::slotGeneral4;
		else if (identifier == "general5")              result = EQ::invslot::slotGeneral5;
		else if (identifier == "general6")              result = EQ::invslot::slotGeneral6;
		else if (identifier == "general7")              result = EQ::invslot::slotGeneral7;
		else if (identifier == "general8")              result = EQ::invslot::slotGeneral8;
		else if (identifier == "general9")              result = EQ::invslot::slotGeneral9;
		else if (identifier == "general10")             result = EQ::invslot::slotGeneral10;
		else if (identifier == "cursor")                result = EQ::invslot::slotCursor;
		else if (identifier == "tradeskill")            result = EQ::invslot::SLOT_TRADESKILL_EXPERIMENT_COMBINE;
		else if (identifier == "augment")               result = EQ::invslot::SLOT_AUGMENT_GENERIC_RETURN;
	}
	else {
		if (identifier == "possessions.begin")          result = EQ::invslot::POSSESSIONS_BEGIN;
		else if (identifier == "possessions.end")       result = EQ::invslot::POSSESSIONS_END;
		else if (identifier == "equipment.begin")       result = EQ::invslot::EQUIPMENT_BEGIN;
		else if (identifier == "equipment.end")         result = EQ::invslot::EQUIPMENT_END;
		else if (identifier == "general.begin")         result = EQ::invslot::GENERAL_BEGIN;
		else if (identifier == "general.end")           result = EQ::invslot::GENERAL_END;
		else if (identifier == "possessionsbags.begin") result = EQ::invbag::GENERAL_BAGS_BEGIN;
		else if (identifier == "possessionsbags.end")   result = EQ::invbag::CURSOR_BAG_END;
		else if (identifier == "generalbags.begin")     result = EQ::invbag::GENERAL_BAGS_BEGIN;
		else if (identifier == "generalbags.end")       result = EQ::invbag::GENERAL_BAGS_END;
		else if (identifier == "general1bag.begin")     result = EQ::invbag::GENERAL_BAGS_BEGIN;
		else if (identifier == "general1bag.end")       result = EQ::invbag::GENERAL_BAGS_BEGIN + (EQ::invbag::SLOT_COUNT - 1);
		else if (identifier == "general2bag.begin")     result = EQ::invbag::GENERAL_BAGS_BEGIN + EQ::invbag::SLOT_COUNT;
		else if (identifier == "general2bag.end")       result = EQ::invbag::GENERAL_BAGS_BEGIN + ((EQ::invbag::SLOT_COUNT * 2) - 1);
		else if (identifier == "general3bag.begin")     result = EQ::invbag::GENERAL_BAGS_BEGIN + (EQ::invbag::SLOT_COUNT * 2);
		else if (identifier == "general3bag.end")       result = EQ::invbag::GENERAL_BAGS_BEGIN + ((EQ::invbag::SLOT_COUNT * 3) - 1);
		else if (identifier == "general4bag.begin")     result = EQ::invbag::GENERAL_BAGS_BEGIN + (EQ::invbag::SLOT_COUNT * 3);
		else if (identifier == "general4bag.end")       result = EQ::invbag::GENERAL_BAGS_BEGIN + ((EQ::invbag::SLOT_COUNT * 4) - 1);
		else if (identifier == "general5bag.begin")     result = EQ::invbag::GENERAL_BAGS_BEGIN + (EQ::invbag::SLOT_COUNT * 4);
		else if (identifier == "general5bag.end")       result = EQ::invbag::GENERAL_BAGS_BEGIN + ((EQ::invbag::SLOT_COUNT * 5) - 1);
		else if (identifier == "general6bag.begin")     result = EQ::invbag::GENERAL_BAGS_BEGIN + (EQ::invbag::SLOT_COUNT * 5);
		else if (identifier == "general6bag.end")       result = EQ::invbag::GENERAL_BAGS_BEGIN + ((EQ::invbag::SLOT_COUNT * 6) - 1);
		else if (identifier == "general7bag.begin")     result = EQ::invbag::GENERAL_BAGS_BEGIN + (EQ::invbag::SLOT_COUNT * 6);
		else if (identifier == "general7bag.end")       result = EQ::invbag::GENERAL_BAGS_BEGIN + ((EQ::invbag::SLOT_COUNT * 7) - 1);
		else if (identifier == "general8bag.begin")     result = EQ::invbag::GENERAL_BAGS_BEGIN + (EQ::invbag::SLOT_COUNT * 7);
		else if (identifier == "general8bag.end")       result = EQ::invbag::GENERAL_BAGS_BEGIN + ((EQ::invbag::SLOT_COUNT * 8) - 1);
		else if (identifier == "general9bag.begin")     result = EQ::invbag::GENERAL_BAGS_BEGIN + (EQ::invbag::SLOT_COUNT * 8);
		else if (identifier == "general9bag.end")       result = EQ::invbag::GENERAL_BAGS_BEGIN + ((EQ::invbag::SLOT_COUNT * 9) - 1);
		else if (identifier == "general10bag.begin")    result = EQ::invbag::GENERAL_BAGS_BEGIN + (EQ::invbag::SLOT_COUNT * 9);
		else if (identifier == "general10bag.end")      result = EQ::invbag::GENERAL_BAGS_BEGIN + ((EQ::invbag::SLOT_COUNT * 10) - 1);
		else if (identifier == "cursorbag.begin")       result = EQ::invbag::CURSOR_BAG_BEGIN;
		else if (identifier == "cursorbag.end")         result = EQ::invbag::CURSOR_BAG_END;
		else if (identifier == "bank.begin")            result = EQ::invslot::BANK_BEGIN;
		else if (identifier == "bank.end")              result = EQ::invslot::BANK_END;
		else if (identifier == "bankbags.begin")        result = EQ::invbag::BANK_BAGS_BEGIN;
		else if (identifier == "bankbags.end")          result = EQ::invbag::BANK_BAGS_END;
		else if (identifier == "sharedbank.begin")      result = EQ::invslot::SHARED_BANK_BEGIN;
		else if (identifier == "sharedbank.end")        result = EQ::invslot::SHARED_BANK_END;
		else if (identifier == "sharedbankbags.begin")  result = EQ::invbag::SHARED_BANK_BAGS_BEGIN;
		else if (identifier == "sharedbankbags.end")    result = EQ::invbag::SHARED_BANK_BAGS_END;
		else if (identifier == "bagslot.begin")         result = EQ::invbag::SLOT_BEGIN;
		else if (identifier == "bagslot.end")           result = EQ::invbag::SLOT_END;
		else if (identifier == "augsocket.begin")       result = EQ::invaug::SOCKET_BEGIN;
		else if (identifier == "augsocket.end")         result = EQ::invaug::SOCKET_END;
	}

	return result;
}

void Perl__castspell(uint16 spell_id, uint16 target_id)
{
	quest_manager.castspell(spell_id, target_id);
}

void Perl__selfcast(uint16 spell_id)
{
	quest_manager.selfcast(spell_id);
}

void Perl__addloot(int item_id)
{
	quest_manager.addloot(item_id);
}

void Perl__addloot(int item_id, int charges)
{
	quest_manager.addloot(item_id, charges);
}

void Perl__addloot(int item_id, int charges, bool equip_item)
{
	quest_manager.addloot(item_id, charges, equip_item);
}

void Perl__zone(const char* zone_name)
{
	quest_manager.Zone(zone_name);
}

void Perl__zonegroup(const char* zone_name)
{
	quest_manager.ZoneGroup(zone_name);
}

void Perl__zoneraid(const char* zone_name)
{
	quest_manager.ZoneRaid(zone_name);
}

bool Perl__hastimer(std::string timer_name)
{
	return quest_manager.hastimer(timer_name);
}

bool Perl__ispausedtimer(std::string timer_name)
{
	return quest_manager.ispausedtimer(timer_name);
}

uint32_t Perl__getremainingtimeMS(std::string timer_name)
{
	return quest_manager.getremainingtimeMS(timer_name);
}

uint32_t Perl__gettimerdurationMS(std::string timer_name)
{
	return quest_manager.gettimerdurationMS(timer_name);
}

void Perl__settimer(std::string timer_name, uint32 seconds)
{
	quest_manager.settimer(timer_name, seconds);
}

void Perl__settimer(std::string timer_name, uint32 seconds, Mob* m)
{
	quest_manager.settimer(timer_name, seconds, m);
}

void Perl__settimer(std::string timer_name, uint32 seconds, EQ::ItemInstance* inst)
{
	quest_manager.settimerMS(timer_name, seconds * 1000, inst);
}

void Perl__settimerMS(std::string timer_name, uint32 milliseconds)
{
	quest_manager.settimerMS(timer_name, milliseconds);
}

void Perl__settimerMS(std::string timer_name, uint32 milliseconds, Mob* m)
{
	quest_manager.settimerMS(timer_name, milliseconds, m);
}

void Perl__settimerMS(std::string timer_name, uint32 milliseconds, EQ::ItemInstance* inst)
{
	quest_manager.settimerMS(timer_name, milliseconds, inst);
}

void Perl__pausetimer(std::string timer_name)
{
	quest_manager.pausetimer(timer_name);
}

void Perl__resumetimer(std::string timer_name)
{
	quest_manager.resumetimer(timer_name);
}

void Perl__stoptimer(std::string timer_name)
{
	quest_manager.stoptimer(timer_name);
}

void Perl__stopalltimers()
{
	quest_manager.stopalltimers();
}

void Perl__emote(const char* message)
{
	quest_manager.emote(message);
}

void Perl__shout(const char* message)
{
	quest_manager.shout(message);
}

void Perl__shout2(const char* message)
{
	quest_manager.shout2(message);
}

void Perl__gmsay(const char* message)
{
	quest_manager.gmsay(message, Chat::LightGray, false, 0, AccountStatus::QuestTroupe);
}

void Perl__gmsay(const char* message, int color_id)
{
	quest_manager.gmsay(message, color_id, false, 0, AccountStatus::QuestTroupe);
}

void Perl__gmsay(const char* message, int color_id, bool send_to_world)
{
	quest_manager.gmsay(message, color_id, send_to_world, 0, AccountStatus::QuestTroupe);
}

void Perl__gmsay(const char* message, int color_id, bool send_to_world, int to_guilddbid)
{
	quest_manager.gmsay(message, color_id, send_to_world, to_guilddbid, AccountStatus::QuestTroupe);
}

void Perl__gmsay(const char* message, int color_id, bool send_to_world, int to_guilddbid, int to_minstatus)
{
	quest_manager.gmsay(message, color_id, send_to_world, to_guilddbid, to_minstatus);
}

void Perl__depop()
{
	quest_manager.depop();
}

void Perl__depop(int npc_type_id)
{
	quest_manager.depop(npc_type_id);
}

void Perl__depop_withtimer()
{
	quest_manager.depop_withtimer();
}

void Perl__depop_withtimer(int npc_type_id)
{
	quest_manager.depop_withtimer(npc_type_id);
}

void Perl__depopall()
{
	quest_manager.depopall();
}

void Perl__depopall(int npc_type_id)
{
	quest_manager.depopall(npc_type_id);
}

void Perl__settarget(const char* target_enum, int target_id)
{
	quest_manager.settarget(target_enum, target_id);
}

void Perl__follow(int entity_id)
{
	quest_manager.follow(entity_id, 10);
}

void Perl__follow(int entity_id, int distance)
{
	quest_manager.follow(entity_id, distance);
}

void Perl__sfollow()
{
	quest_manager.sfollow();
}

void Perl__changedeity(uint32 deity_id)
{
	quest_manager.changedeity(deity_id);
}

void Perl__exp(int amount)
{
	quest_manager.exp(amount);
}

void Perl__level(int new_level)
{
	quest_manager.level(new_level);
}

void Perl__traindisc(int tome_item_id)
{
	quest_manager.traindisc(tome_item_id);
}

bool Perl__isdisctome(int item_id)
{
	return quest_manager.isdisctome(item_id);
}

std::string Perl__getracename(uint16 race_id)
{
	return quest_manager.getracename(race_id);
}

std::string Perl__getspellname(uint32 spell_id)
{
	return quest_manager.getspellname(spell_id);
}

uint8 Perl__get_spell_level(uint16 spell_id, uint8 class_id)
{
	const auto spell_level = GetSpellLevel(spell_id, class_id);
	return spell_level > RuleI(Character, MaxLevel) ? UINT8_MAX : spell_level;
}

std::string Perl__getskillname(int skill_id)
{
	return quest_manager.getskillname(skill_id);
}

void Perl__safemove()
{
	quest_manager.safemove();
}

void Perl__rain(int weather)
{
	quest_manager.rain(weather);
}

void Perl__snow(int weather)
{
	quest_manager.snow(weather);
}

void Perl__surname(std::string last_name)
{
	quest_manager.surname(last_name);
}

void Perl__permaclass(int class_id)
{
	quest_manager.permaclass(class_id);
}

void Perl__permarace(int race_id)
{
	quest_manager.permarace(race_id);
}

void Perl__permagender(int gender_id)
{
	quest_manager.permagender(gender_id);
}

int Perl__scribespells(int max_level)
{
	return quest_manager.scribespells(max_level);
}

int Perl__scribespells(int max_level, int min_level)
{
	return quest_manager.scribespells(max_level, min_level);
}

int Perl__traindiscs(int max_level)
{
	return quest_manager.traindiscs(max_level);
}

int Perl__traindiscs(int max_level, int min_level)
{
	return quest_manager.traindiscs(max_level, min_level);
}

void Perl__unscribespells()
{
	quest_manager.unscribespells();
}

void Perl__untraindiscs()
{
	quest_manager.untraindiscs();
}

void Perl__givecash(uint32 copper)
{
	quest_manager.givecash(copper);
}

void Perl__givecash(uint32 copper, uint32 silver)
{
	quest_manager.givecash(copper, silver);
}

void Perl__givecash(uint32 copper, uint32 silver, uint32 gold)
{
	quest_manager.givecash(copper, silver, gold);
}

void Perl__givecash(uint32 copper, uint32 silver, uint32 gold, uint32 platinum)
{
	quest_manager.givecash(copper, silver, gold, platinum);
}

void Perl__pvp(const char* mode)
{
	quest_manager.pvp(mode);
}

void Perl__movepc(int zone_id, float x, float y, float z)
{
	quest_manager.movepc(zone_id, x, y, z, 0.0f);
}

void Perl__movepc(int zone_id, float x, float y, float z, float heading)
{
	quest_manager.movepc(zone_id, x, y, z, heading);
}

void Perl__gmmove(float x, float y, float z)
{
	quest_manager.gmmove(x, y, z);
}

void Perl__movegrp(int zone_id, float x, float y, float z)
{
	quest_manager.movegrp(zone_id, x, y, z);
}

void Perl__doanim(int animation_id)
{
	quest_manager.doanim(animation_id);
}

void Perl__doanim(int animation_id, int animation_speed)
{
	quest_manager.doanim(animation_id, animation_speed);
}

void Perl__doanim(int animation_id, int animation_speed, bool ackreq)
{
	quest_manager.doanim(animation_id, animation_speed, ackreq);
}

void Perl__doanim(int animation_id, int animation_speed, bool ackreq, int filter)
{
	quest_manager.doanim(animation_id, animation_speed, ackreq, static_cast<eqFilterType>(filter));
}

void Perl__addskill(int skill_id, int value)
{
	quest_manager.addskill(skill_id, value);
}

void Perl__setlanguage(uint8 language_id, uint8 language_skill)
{
	quest_manager.setlanguage(language_id, language_skill);
}

void Perl__setskill(int skill_id, int value)
{
	quest_manager.setskill(skill_id, value);
}

void Perl__setallskill(int value)
{
	quest_manager.setallskill(value);
}

void Perl__attack(const char* client_name)
{
	quest_manager.attack(client_name);
}

void Perl__attacknpc(int npc_entity_id)
{
	quest_manager.attacknpc(npc_entity_id);
}

void Perl__attacknpctype(int npc_type_id)
{
	quest_manager.attacknpctype(npc_type_id);
}

void Perl__save()
{
	quest_manager.save();
}

void Perl__faction(int faction_id, int value)
{
	quest_manager.faction(faction_id, value, 0);
}

void Perl__faction(int faction_id, int value, int temp)
{
	quest_manager.faction(faction_id, value, temp);
}

void Perl__rewardfaction(int faction_id, int value)
{
	quest_manager.rewardfaction(faction_id, value);
}

void Perl__setsky(uint8 new_sky)
{
	quest_manager.setsky(new_sky);
}

void Perl__setguild(uint32_t guild_id, uint8_t guild_rank_id)
{
	quest_manager.SetGuild(guild_id, guild_rank_id);
}

void Perl__createguild(const char* guild_name, const char* leader_name)
{
	quest_manager.CreateGuild(guild_name, leader_name);
}

void Perl__settime(int new_hour, int new_min)
{
	quest_manager.settime(new_hour, new_min);
}

void Perl__settime(int new_hour, int new_min, bool update_world)
{
	quest_manager.settime(new_hour, new_min, update_world);
}

void Perl__itemlink(int item_id)
{
	quest_manager.itemlink(item_id);
}

void Perl__signalwith(int npc_id, int signal_id)
{
	quest_manager.signalwith(npc_id, signal_id);
}

void Perl__signalwith(int npc_id, int signal_id, int wait_ms)
{
	quest_manager.signalwith(npc_id, signal_id, wait_ms);
}

void Perl__signal(int npc_id)
{
	quest_manager.signal(npc_id);
}

void Perl__signal(int npc_id, int wait_ms)
{
	quest_manager.signal(npc_id, wait_ms);
}

void Perl__setglobal(const char* key, const char* value, int options, const char* duration)
{
	quest_manager.setglobal(key, value, options, duration);
}

void Perl__targlobal(const char* key, const char* value, const char* duration, int npc_id, int char_id, int zone_id)
{
	quest_manager.targlobal(key, value, duration, npc_id, char_id, zone_id);
}

void Perl__delglobal(const char* key)
{
	quest_manager.delglobal(key);
}

void Perl__ding()
{
	quest_manager.ding();
}

void Perl__rebind(int zone_id, float x, float y, float z)
{
	quest_manager.rebind(zone_id, glm::vec3(x, y, z));
}

void Perl__rebind(int zone_id, float x, float y, float z, float heading)
{
	quest_manager.rebind(zone_id, glm::vec4(x, y, z, heading));
}

void Perl__start(int waypoint)
{
	quest_manager.start(waypoint);
}

void Perl__stop()
{
	quest_manager.stop();
}

void Perl__pause(int duration_ms)
{
	quest_manager.pause(duration_ms);
}

void Perl__moveto(float x, float y, float z)
{
	quest_manager.moveto(glm::vec4(x, y, z, 0.0f), false);
}

void Perl__moveto(float x, float y, float z, float h)
{
	quest_manager.moveto(glm::vec4(x, y, z, h), false);
}

void Perl__moveto(float x, float y, float z, float h, bool save_guard_location)
{
	quest_manager.moveto(glm::vec4(x, y, z, h), save_guard_location);
}

void Perl__resume()
{
	quest_manager.resume();
}

void Perl__addldonpoints(uint32 theme_id, int points)
{
	quest_manager.addldonpoints(theme_id, points);
}

void Perl__addldonwin(uint32 theme_id)
{
	quest_manager.addldonwin(theme_id);
}

void Perl__removeldonwin(uint32 theme_id)
{
	quest_manager.removeldonwin(theme_id);
}

void Perl__addldonloss(uint32 theme_id)
{
	quest_manager.addldonloss(theme_id);
}

void Perl__removeldonloss(uint32 theme_id)
{
	quest_manager.removeldonloss(theme_id);
}

void Perl__setnexthpevent(int at_mob_percentage)
{
	quest_manager.setnexthpevent(at_mob_percentage);
}

void Perl__setnextinchpevent(int at_mob_percentage)
{
	quest_manager.setnextinchpevent(at_mob_percentage);
}

void Perl__sethp(int mob_health_percentage)
{
	quest_manager.sethp(mob_health_percentage);
}

void Perl__respawn(int npc_type_id, int grid_id)
{
	quest_manager.respawn(npc_type_id, grid_id);
}

perl::scalar Perl__ChooseRandom(perl::array options)
{
	int index = zone->random.Int(0, static_cast<int>(options.size()) - 1);
	return options[index];
}

void Perl__set_proximity(float min_x, float max_x, float min_y, float max_y)
{
	quest_manager.set_proximity(min_x, max_x, min_y, max_y);
}

void Perl__set_proximity(float min_x, float max_x, float min_y, float max_y, float min_z, float max_z)
{
	quest_manager.set_proximity(min_x, max_x, min_y, max_y, min_z, max_z);
}

void Perl__set_proximity(float min_x, float max_x, float min_y, float max_y, float min_z, float max_z, bool enable_say)
{
	quest_manager.set_proximity(min_x, max_x, min_y, max_y, min_z, max_z, enable_say);
}

void Perl__clear_proximity()
{
	quest_manager.clear_proximity();
}

void Perl__enable_proximity_say()
{
	quest_manager.enable_proximity_say();
}

void Perl__disable_proximity_say()
{
	quest_manager.disable_proximity_say();
}

void Perl__setanim(int npc_type_id, int appearance_number)
{
	quest_manager.setanim(npc_type_id, appearance_number);
}

void Perl__showgrid(int grid_id)
{
	quest_manager.showgrid(grid_id);
}

void Perl__spawn_condition(const char* zone_short, uint16 condition_id, int16 value)
{
	quest_manager.spawn_condition(zone_short, zone->GetInstanceID(), condition_id, value);
}

void Perl__spawn_condition(const char* zone_short, uint32_t instance_id, uint16 condition_id, int16 value)
{
	quest_manager.spawn_condition(zone_short, instance_id, condition_id, value);
}

int Perl__get_spawn_condition(const char* zone_short, uint16 condition_id)
{
	return quest_manager.get_spawn_condition(zone_short, zone->GetInstanceID(), condition_id);
}

int Perl__get_spawn_condition(const char* zone_short, uint32 instance_id, uint16 condition_id)
{
	return quest_manager.get_spawn_condition(zone_short, instance_id, condition_id);
}

void Perl__toggle_spawn_event(int event_id, bool is_enabled, bool is_strict, bool reset_base)
{
	quest_manager.toggle_spawn_event(event_id, is_enabled, is_strict, reset_base);
}

bool Perl__has_zone_flag(int zone_id)
{
	return quest_manager.has_zone_flag(zone_id);
}

void Perl__set_zone_flag(int zone_id)
{
	quest_manager.set_zone_flag(zone_id);
}

void Perl__clear_zone_flag(int zone_id)
{
	quest_manager.clear_zone_flag(zone_id);
}

bool Perl__summonburiedplayercorpse(uint32 char_id, float dest_x, float dest_y, float dest_z, float dest_heading)
{
	auto position = glm::vec4(dest_x, dest_y, dest_z, dest_heading);
	return quest_manager.summonburiedplayercorpse(char_id, position);
}

bool Perl__summonallplayercorpses(uint32 char_id, float dest_x, float dest_y, float dest_z, float dest_heading)
{
	auto position = glm::vec4(dest_x, dest_y, dest_z, dest_heading);
	return quest_manager.summonallplayercorpses(char_id, position);
}

int64 Perl__getplayercorpsecount(uint32 character_id)
{
	return quest_manager.getplayercorpsecount(character_id);
}

int64 Perl__getplayercorpsecountbyzoneid(uint32 character_id, uint32 zone_id)
{
	return quest_manager.getplayercorpsecountbyzoneid(character_id, zone_id);
}

int64 Perl__getplayerburiedcorpsecount(uint32 char_id)
{
	return quest_manager.getplayerburiedcorpsecount(char_id);
}

bool Perl__buryplayercorpse(uint32 char_id)
{
	return quest_manager.buryplayercorpse(char_id);
}

void Perl__forcedooropen(uint32 door_id)
{
	quest_manager.forcedooropen(door_id, false);
}

void Perl__forcedooropen(uint32 door_id, bool alt_mode)
{
	quest_manager.forcedooropen(door_id, alt_mode);
}

void Perl__forcedoorclose(uint32 door_id)
{
	quest_manager.forcedoorclose(door_id, false);
}

void Perl__forcedoorclose(uint32 door_id, bool alt_mode)
{
	quest_manager.forcedoorclose(door_id, alt_mode);
}

void Perl__toggledoorstate(uint32 door_id)
{
	quest_manager.toggledoorstate(door_id);
}

bool Perl__isdooropen(uint32 door_id)
{
	return quest_manager.isdooropen(door_id);
}

void Perl__depopzone(bool start_spawn_status)
{
	quest_manager.depopzone(start_spawn_status);
}

void Perl__repopzone()
{
	quest_manager.repopzone();
}

void Perl__repopzone(bool is_forced)
{
	quest_manager.repopzone(is_forced);
}

void Perl__processmobswhilezoneempty(bool on)
{
	quest_manager.processmobswhilezoneempty(on);
}

void Perl__npcrace(uint16 race_id)
{
	quest_manager.npcrace(race_id);
}

void Perl__npcgender(uint8 gender_id)
{
	quest_manager.npcgender(gender_id);
}

void Perl__npcsize(float size)
{
	quest_manager.npcsize(size);
}

void Perl__npctexture(uint8 texture_id)
{
	quest_manager.npctexture(texture_id);
}

void Perl__playerrace(uint16 race_id)
{
	quest_manager.playerrace(race_id);
}

void Perl__playergender(uint8 gender_id)
{
	quest_manager.playergender(gender_id);
}

void Perl__playersize(float size)
{
	quest_manager.playersize(size);
}

void Perl__playertexture(uint8 texture_id)
{
	quest_manager.playertexture(texture_id);
}

void Perl__playerfeature(const char* feature, int value)
{
	quest_manager.playerfeature(feature, value);
}

void Perl__npcfeature(const char* feature, int value)
{
	quest_manager.npcfeature(feature, value);
}

int Perl__createbotcount()
{
	return quest_manager.createbotcount();
}

int Perl__createbotcount(uint8 class_id)
{
	return quest_manager.createbotcount(class_id);
}

int Perl__spawnbotcount()
{
	return quest_manager.spawnbotcount();
}

int Perl__spawnbotcount(uint8 class_id)
{
	return quest_manager.spawnbotcount(class_id);
}

bool Perl__botquest()
{
	return quest_manager.botquest();
}

bool Perl__createBot(const char* firstname, const char* lastname, int level, int race_id, int class_id, int gender_id)
{
	return quest_manager.createBot(firstname, lastname, level, race_id, class_id, gender_id);
}

void Perl__taskselector(perl::array task_ids)
{
	if (task_ids.size() > MAXCHOOSERENTRIES)
	{
		throw std::runtime_error(fmt::format("Exceeded max number of task offers [{}]", MAXCHOOSERENTRIES));
	}

	std::vector<int> tasks;
	for (int i = 0; i < task_ids.size(); ++i)
	{
		tasks.push_back(task_ids[i]);
	}
	quest_manager.taskselector(tasks);
}

void Perl__taskselector_nocooldown(perl::array task_ids)
{
	std::vector<int> tasks;
	for (int i = 0; i < task_ids.size() && i < MAXCHOOSERENTRIES; ++i)
	{
		tasks.push_back(task_ids[i]);
	}
	quest_manager.taskselector(tasks, true);
}

void Perl__task_setselector(int task_set_id)
{
	quest_manager.tasksetselector(task_set_id);
}

void Perl__task_setselector(int task_set_id, bool ignore_cooldown)
{
	quest_manager.tasksetselector(task_set_id, ignore_cooldown);
}

void Perl__enabletask(perl::array task_ids)
{
	int count = 0;
	int tasks[MAXCHOOSERENTRIES];
	for (int i = 0; i < task_ids.size() && i < MAXCHOOSERENTRIES; ++i)
	{
		tasks[i] = task_ids[i];
		++count;
	}

	quest_manager.enabletask(count, tasks);
}

void Perl__disabletask(perl::array task_ids)
{
	int count = 0;
	int tasks[MAXCHOOSERENTRIES];
	for (int i = 0; i < task_ids.size() && i < MAXCHOOSERENTRIES; ++i)
	{
		tasks[i] = task_ids[i];
		++count;
	}

	quest_manager.disabletask(count, tasks);
}

bool Perl__istaskenabled(int task_id)
{
	return quest_manager.istaskenabled(task_id);
}

bool Perl__istaskactive(int task_id)
{
	return quest_manager.istaskactive(task_id);
}

bool Perl__istaskactivityactive(int task_id, int activity_id)
{
	return quest_manager.istaskactivityactive(task_id, activity_id);
}

int Perl__gettaskactivitydonecount(int task_id, int activity_id)
{
	return quest_manager.gettaskactivitydonecount(task_id, activity_id);
}

void Perl__updatetaskactivity(int task_id, int activity_id)
{
	quest_manager.updatetaskactivity(task_id, activity_id, 1);
}

void Perl__updatetaskactivity(int task_id, int activity_id, int count)
{
	quest_manager.updatetaskactivity(task_id, activity_id, count);
}

void Perl__updatetaskactivity(int task_id, int activity_id, int count, bool ignore_quest_update)
{
	quest_manager.updatetaskactivity(task_id, activity_id, count, ignore_quest_update);
}

void Perl__resettaskactivity(int task_id, int activity_id)
{
	quest_manager.resettaskactivity(task_id, activity_id);
}

void Perl__assigntask(int task_id)
{
	quest_manager.assigntask(task_id);
}

void Perl__assigntask(int task_id, bool enforce_level_requirement)
{
	quest_manager.assigntask(task_id, enforce_level_requirement);
}

void Perl__failtask(int task_id)
{
	quest_manager.failtask(task_id);
}

bool Perl__completetask(int task_id)
{
	return quest_manager.completetask(task_id);
}

bool Perl__uncompletetask(int task_id)
{
	return quest_manager.uncompletetask(task_id);
}

int Perl__tasktimeleft(int task_id)
{
	return quest_manager.tasktimeleft(task_id);
}

bool Perl__istaskcompleted(int task_id)
{
	return quest_manager.istaskcompleted(task_id);
}

int Perl__enabledtaskcount(int task_set)
{
	return quest_manager.enabledtaskcount(task_set);
}

int Perl__firsttaskinset(int task_set)
{
	return quest_manager.firsttaskinset(task_set);
}

int Perl__lasttaskinset(int task_set)
{
	return quest_manager.lasttaskinset(task_set);
}

int Perl__nexttaskinset(int task_set, int task_id)
{
	return quest_manager.nexttaskinset(task_set, task_id);
}

int Perl__activespeaktask()
{
	return quest_manager.activespeaktask();
}

int Perl__activespeakactivity(int task_id)
{
	return quest_manager.activespeakactivity(task_id);
}

int Perl__activetasksinset(int task_set)
{
	return quest_manager.activetasksinset(task_set);
}

int Perl__completedtasksinset(int task_set)
{
	return quest_manager.completedtasksinset(task_set);
}

bool Perl__istaskappropriate(int task_id)
{
	return quest_manager.istaskappropriate(task_id);
}

std::string Perl__gettaskname(uint32 task_id)
{
	return quest_manager.gettaskname(task_id);
}

int Perl__get_dz_task_id()
{
	return quest_manager.GetCurrentDzTaskID();
}

void Perl__end_dz_task()
{
	quest_manager.EndCurrentDzTask();
}

void Perl__end_dz_task(bool send_fail)
{
	quest_manager.EndCurrentDzTask(send_fail);
}

void Perl__popup(const char* window_title, const char* message)
{
	quest_manager.popup(window_title, message, 0, 0, 0);
}

void Perl__popup(const char* window_title, const char* message, int popup_id)
{
	quest_manager.popup(window_title, message, popup_id, 0, 0);
}

void Perl__popup(const char* window_title, const char* message, int popup_id, int buttons)
{
	quest_manager.popup(window_title, message, popup_id, buttons, 0);
}

void Perl__popup(const char* window_title, const char* message, int popup_id, int buttons, int duration)
{
	quest_manager.popup(window_title, message, popup_id, buttons, duration);
}

void Perl__clearspawntimers()
{
	quest_manager.clearspawntimers();
}

void Perl__ze(int emote_color_id, const char* message)
{
	return quest_manager.ze(emote_color_id, message);
}

void Perl__we(int emote_color_id, const char* message)
{
	quest_manager.we(emote_color_id, message);
}

void Perl__message(int color, const char* message)
{
	quest_manager.message(color, message);
}

void Perl__whisper(const char* message)
{
	quest_manager.whisper(message);
}

int Perl__getlevel(uint8 type)
{
	return quest_manager.getlevel(type);
}

uint16 Perl__CreateGroundObject(uint32_t item_id, float x, float y, float z, float heading)
{
	return quest_manager.CreateGroundObject(item_id, glm::vec4(x, y, z, heading));
}

uint16 Perl__CreateGroundObject(uint32_t item_id, float x, float y, float z, float heading, uint32_t decay_time_ms)
{
	return quest_manager.CreateGroundObject(item_id, glm::vec4(x, y, z, heading), decay_time_ms);
}

uint16 Perl__CreateGroundObjectFromModel(const char* modelname, float x, float y, float z, float heading)
{
	return quest_manager.CreateGroundObjectFromModel(modelname, glm::vec4(x, y, z, heading));
}

uint16 Perl__CreateGroundObjectFromModel(const char* modelname, float x, float y, float z, float heading, uint8_t object_type)
{
	return quest_manager.CreateGroundObjectFromModel(modelname, glm::vec4(x, y, z, heading), object_type);
}

uint16 Perl__CreateGroundObjectFromModel(const char* modelname, float x, float y, float z, float heading, uint8_t object_type, uint32_t decay_time_ms)
{
	return quest_manager.CreateGroundObjectFromModel(modelname, glm::vec4(x, y, z, heading), object_type, decay_time_ms);
}

uint16 Perl__CreateDoor(const char* modelname, float x, float y, float z, float heading)
{
	return quest_manager.CreateDoor(modelname, x, y, z, heading, 58, 100);
}

uint16 Perl__CreateDoor(const char* modelname, float x, float y, float z, float heading, uint8_t object_type)
{
	return quest_manager.CreateDoor(modelname, x, y, z, heading, object_type, 100);
}

uint16 Perl__CreateDoor(const char* modelname, float x, float y, float z, float heading, uint8_t object_type, uint16_t size)
{
	return quest_manager.CreateDoor(modelname, x, y, z, heading, object_type, size);
}

void Perl__ModifyNPCStat(const char* key, const char* value)
{
	quest_manager.ModifyNPCStat(key, value);
}

int Perl__collectitems(uint32_t item_id, bool remove_item)
{
	return quest_manager.collectitems(item_id, remove_item);
}

uint32 Perl__countitem(uint32_t item_id)
{
	return quest_manager.countitem(item_id);
}

void Perl__removeitem(uint32_t item_id)
{
	quest_manager.removeitem(item_id);
}

void Perl__removeitem(uint32_t item_id, int quantity)
{
	quest_manager.removeitem(item_id, quantity);
}

std::string Perl__getitemcomment(uint32 item_id)
{
	return quest_manager.getitemcomment(item_id);
}

std::string Perl__getitemlore(uint32 item_id)
{
	return quest_manager.getitemlore(item_id);
}

std::string Perl__getitemname(uint32 item_id)
{
	return quest_manager.getitemname(item_id);
}

std::string Perl__getnpcnamebyid(uint32 npc_id)
{
	return quest_manager.getnpcnamebyid(npc_id);
}

void Perl__UpdateSpawnTimer(uint32 id, uint32 new_time)
{
	quest_manager.UpdateSpawnTimer(id, new_time);
}

void Perl__MerchantSetItem(uint32 npc_id, uint32 item_id)
{
	quest_manager.MerchantSetItem(npc_id, item_id);
}

void Perl__MerchantSetItem(uint32 npc_id, uint32 item_id, uint32 quantity)
{
	quest_manager.MerchantSetItem(npc_id, item_id, quantity);
}

uint32_t Perl__MerchantCountItem(uint32 npc_id, uint32 item_id)
{
	return quest_manager.MerchantCountItem(npc_id, item_id);
}

int Perl__CreateInstance(const char* zone_name, int16 version, int32 duration)
{
	return quest_manager.CreateInstance(zone_name, version, duration);
}

void Perl__DestroyInstance(uint16 id)
{
	quest_manager.DestroyInstance(id);
}

void Perl__UpdateInstanceTimer(int16 instance_id, uint32 duration)
{
	quest_manager.UpdateInstanceTimer(instance_id, duration);
}

uint32_t Perl__GetInstanceTimer()
{
	return quest_manager.GetInstanceTimer();
}

uint32_t Perl__GetInstanceTimerByID(uint16 instance_id)
{
	return quest_manager.GetInstanceTimerByID(instance_id);
}

int Perl__GetInstanceID(const char* zone_name, uint16 version)
{
	return quest_manager.GetInstanceID(zone_name, version);
}

uint8 Perl__GetInstanceVersionByID(uint16 instance_id)
{
	return database.GetInstanceVersion(instance_id);
}

uint32 Perl__GetInstanceZoneIDByID(uint16 instance_id)
{
	return database.GetInstanceZoneID(instance_id);
}

perl::array Perl__GetInstanceIDs(std::string zone_name)
{
	perl::array result;

	const auto instance_ids = quest_manager.GetInstanceIDs(zone_name);
	for (int i = 0; i < instance_ids.size(); i++) {
		result.push_back(instance_ids[i]);
	}

	return result;
}

int Perl__GetInstanceIDByCharID(const char* zone_name, int16 version, uint32 character_id)
{
	return quest_manager.GetInstanceIDByCharID(zone_name, version, character_id);
}

perl::array Perl__GetInstanceIDsByCharID(std::string zone_name, uint32 character_id)
{
	perl::array result;

	const auto instance_ids = quest_manager.GetInstanceIDs(zone_name, character_id);
	for (int i = 0; i < instance_ids.size(); i++) {
		result.push_back(instance_ids[i]);
	}

	return result;
}

std::string Perl__GetCharactersInInstance(uint16 instance_id)
{
	std::list<uint32> character_ids;
	std::string char_id_string = "No players in that instance.";

	database.GetCharactersInInstance(instance_id, character_ids);

	if (character_ids.size() > 0)
	{
		char_id_string = fmt::format("{} player(s) in instance: ", character_ids.size());
		auto iter = character_ids.begin();
		while (iter != character_ids.end()) {
			char_id_string += database.GetCharName(*iter);
			char_id_string += "(";
			char_id_string += itoa(*iter);
			char_id_string += ")";
			++iter;
			if (iter != character_ids.end())
				char_id_string += ", ";
		}
	}

	return char_id_string;
}

void Perl__AssignToInstance(uint16 instance_id)
{
	quest_manager.AssignToInstance(instance_id);
}

void Perl__AssignToInstanceByCharID(uint16 instance_id, uint32 character_id)
{
	quest_manager.AssignToInstanceByCharID(instance_id, character_id);
}

void Perl__AssignGroupToInstance(uint16 instance_id)
{
	quest_manager.AssignGroupToInstance(instance_id);
}

void Perl__AssignRaidToInstance(uint16 instance_id)
{
	quest_manager.AssignRaidToInstance(instance_id);
}

void Perl__RemoveFromInstance(uint16 instance_id)
{
	quest_manager.RemoveFromInstance(instance_id);
}

void Perl__RemoveFromInstanceByCharID(uint16 instance_id, uint32 character_id)
{
	quest_manager.RemoveFromInstanceByCharID(instance_id, character_id);
}

bool Perl__CheckInstanceByCharID(uint16 instance_id, uint32 character_id)
{
	return quest_manager.CheckInstanceByCharID(instance_id, character_id);
}

void Perl__RemoveAllFromInstance(uint16 instance_id)
{
	quest_manager.RemoveAllFromInstance(instance_id);
}

void Perl__MovePCInstance(int zone_id, int instance_id, float x, float y, float z)
{
	quest_manager.MovePCInstance(zone_id, instance_id, glm::vec4(x, y, z, 0.0f));
}

void Perl__MovePCInstance(int zone_id, int instance_id, float x, float y, float z, float heading)
{
	quest_manager.MovePCInstance(zone_id, instance_id, glm::vec4(x, y, z, heading));
}

void Perl__FlagInstanceByGroupLeader(uint32 zone, uint16 version)
{
	quest_manager.FlagInstanceByGroupLeader(zone, version);
}

void Perl__FlagInstanceByRaidLeader(uint32 zone, uint16 version)
{
	quest_manager.FlagInstanceByRaidLeader(zone, version);
}

std::string Perl__saylink(std::string text)
{
	return Saylink::Create(text);
}

std::string Perl__saylink(std::string text, bool silent)
{
	return Saylink::Create(text, silent);
}

std::string Perl__saylink(std::string text, bool silent, std::string link_name)
{
	return Saylink::Create(text, silent, link_name);
}

std::string Perl__getcharnamebyid(uint32 char_id)
{
	return quest_manager.getcharnamebyid(char_id);
}

uint32_t Perl__getcharidbyname(const char* name)
{
	return quest_manager.getcharidbyname(name);
}

std::string Perl__getclassname(uint8 class_id)
{
	return quest_manager.getclassname(class_id);
}

std::string Perl__getclassname(uint8 class_id, uint8 level)
{
	return quest_manager.getclassname(class_id, level);
}

uint32 Perl__getcurrencyitemid(uint32 currency_id)
{
	return quest_manager.getcurrencyitemid(currency_id);
}

uint32 Perl__getcurrencyid(uint32 item_id)
{
	return quest_manager.getcurrencyid(item_id);
}

std::string Perl__getguildnamebyid(int guild_id)
{
	return quest_manager.getguildnamebyid(guild_id);
}

int Perl__getguildidbycharid(uint32 char_id)
{
	return quest_manager.getguildidbycharid(char_id);
}

int Perl__getgroupidbycharid(uint32 char_id)
{
	return quest_manager.getgroupidbycharid(char_id);
}

int Perl__getraididbycharid(uint32 char_id)
{
	return quest_manager.getraididbycharid(char_id);
}

void Perl__SetRunning(bool is_running)
{
	quest_manager.SetRunning(is_running);
}

bool Perl__IsRunning()
{
	return quest_manager.IsRunning();
}

bool Perl__IsEffectInSpell(uint16 spell_id, int effect_id)
{
	return IsEffectInSpell(spell_id, effect_id);
}

bool Perl__IsBeneficialSpell(uint16 spell_id)
{
	return IsBeneficialSpell(spell_id);
}

bool Perl__IsDetrimentalSpell(uint16 spell_id)
{
	return IsDetrimentalSpell(spell_id);
}

bool Perl__IsTargetableAESpell(uint16 spell_id)
{
	return IsTargetableAESpell(spell_id);
}

bool Perl__IsSacrificeSpell(uint16 spell_id)
{
	return IsSacrificeSpell(spell_id);
}

bool Perl__IsLifetapSpell(uint16 spell_id)
{
	return IsLifetapSpell(spell_id);
}

bool Perl__IsMesmerizeSpell(uint16 spell_id)
{
	return IsMesmerizeSpell(spell_id);
}

bool Perl__IsStunSpell(uint16 spell_id)
{
	return IsStunSpell(spell_id);
}

bool Perl__IsSummonSpell(uint16 spell_id)
{
	return IsSummonSpell(spell_id);
}

bool Perl__IsDamageSpell(uint16 spell_id)
{
	return IsDamageSpell(spell_id);
}

bool Perl__IsFearSpell(uint16 spell_id)
{
	return IsFearSpell(spell_id);
}

bool Perl__IsCureSpell(uint16 spell_id)
{
	return IsCureSpell(spell_id);
}

bool Perl__IsHasteSpell(uint16 spell_id)
{
	return IsHasteSpell(spell_id);
}

bool Perl__IsHarmonySpell(uint16 spell_id)
{
	return IsHarmonySpell(spell_id);
}

bool Perl__IsPercentalHealSpell(uint16 spell_id)
{
	return IsPercentalHealSpell(spell_id);
}

bool Perl__IsGroupOnlySpell(uint16 spell_id)
{
	return IsGroupOnlySpell(spell_id);
}

bool Perl__IsInvisibleSpell(uint16 spell_id)
{
	return IsInvisibleSpell(spell_id);
}

bool Perl__IsInvulnerabilitySpell(uint16 spell_id)
{
	return IsInvulnerabilitySpell(spell_id);
}

bool Perl__IsCompleteHealDurationSpell(uint16 spell_id)
{
	return IsCompleteHealDurationSpell(spell_id);
}

bool Perl__IsPoisonCounterSpell(uint16 spell_id)
{
	return IsPoisonCounterSpell(spell_id);
}

bool Perl__IsDiseaseCounterSpell(uint16 spell_id)
{
	return IsDiseaseCounterSpell(spell_id);
}

bool Perl__IsSummonItemSpell(uint16 spell_id)
{
	return IsSummonItemSpell(spell_id);
}

bool Perl__IsSummonSkeletonSpell(uint16 spell_id)
{
	return IsSummonSkeletonSpell(spell_id);
}

bool Perl__IsSummonPetSpell(uint16 spell_id)
{
	return IsSummonPetSpell(spell_id);
}

bool Perl__IsPetSpell(uint16 spell_id)
{
	return IsPetSpell(spell_id);
}

bool Perl__IsSummonPCSpell(uint16 spell_id)
{
	return IsSummonPCSpell(spell_id);
}

bool Perl__IsCharmSpell(uint16 spell_id)
{
	return IsCharmSpell(spell_id);
}

bool Perl__IsBlindSpell(uint16 spell_id)
{
	return IsBlindSpell(spell_id);
}

bool Perl__IsHealthSpell(uint16 spell_id)
{
	return IsHealthSpell(spell_id);
}

bool Perl__IsCastTimeReductionSpell(uint16 spell_id)
{
	return IsCastTimeReductionSpell(spell_id);
}

bool Perl__IsIncreaseDurationSpell(uint16 spell_id)
{
	return IsIncreaseDurationSpell(spell_id);
}

bool Perl__IsManaCostReductionSpell(uint16 spell_id)
{
	return IsManaCostReductionSpell(spell_id);
}

bool Perl__IsIncreaseRangeSpell(uint16 spell_id)
{
	return IsIncreaseRangeSpell(spell_id);
}

bool Perl__IsImprovedHealingSpell(uint16 spell_id)
{
	return IsImprovedHealingSpell(spell_id);
}

bool Perl__IsImprovedDamageSpell(uint16 spell_id)
{
	return IsImprovedDamageSpell(spell_id);
}

bool Perl__IsAEDurationSpell(uint16 spell_id)
{
	return IsAEDurationSpell(spell_id);
}

bool Perl__IsPureNukeSpell(uint16 spell_id)
{
	return IsPureNukeSpell(spell_id);
}

bool Perl__IsAENukeSpell(uint16 spell_id)
{
	return IsAENukeSpell(spell_id);
}

bool Perl__IsPBAENukeSpell(uint16 spell_id)
{
	return IsPBAENukeSpell(spell_id);
}

bool Perl__IsAERainNukeSpell(uint16 spell_id)
{
	return IsAERainNukeSpell(spell_id);
}

bool Perl__IsPartialResistableSpell(uint16 spell_id)
{
	return IsPartialResistableSpell(spell_id);
}

bool Perl__IsResistableSpell(uint16 spell_id)
{
	return IsResistableSpell(spell_id);
}

bool Perl__IsGroupSpell(uint16 spell_id)
{
	return IsGroupSpell(spell_id);
}

bool Perl__IsTGBCompatibleSpell(uint16 spell_id)
{
	return IsTGBCompatibleSpell(spell_id);
}

bool Perl__IsBardSong(uint16 spell_id)
{
	return IsBardSong(spell_id);
}

bool Perl__IsPulsingBardSong(uint16 spell_id)
{
	return IsPulsingBardSong(spell_id);
}

bool Perl__IsDisciplineBuff(uint16 spell_id)
{
	return IsDisciplineBuff(spell_id);
}

bool Perl__IsDiscipline(uint16 spell_id)
{
	return IsDiscipline(spell_id);
}

bool Perl__IsCombatSkill(uint16 spell_id)
{
	return IsCombatSkill(spell_id);
}

bool Perl__IsResurrectionEffects(uint16 spell_id)
{
	return IsResurrectionEffects(spell_id);
}

bool Perl__IsRuneSpell(uint16 spell_id)
{
	return IsRuneSpell(spell_id);
}

bool Perl__IsMagicRuneSpell(uint16 spell_id)
{
	return IsMagicRuneSpell(spell_id);
}

bool Perl__IsManaTapSpell(uint16 spell_id)
{
	return IsManaTapSpell(spell_id);
}

bool Perl__IsAllianceSpell(uint16 spell_id)
{
	return IsAllianceSpell(spell_id);
}

bool Perl__IsDeathSaveSpell(uint16 spell_id)
{
	return IsDeathSaveSpell(spell_id);
}

bool Perl__IsPartialDeathSaveSpell(uint16 spell_id)
{
	return IsPartialDeathSaveSpell(spell_id);
}

bool Perl__IsFullDeathSaveSpell(uint16 spell_id)
{
	return IsFullDeathSaveSpell(spell_id);
}

bool Perl__IsShadowStepSpell(uint16 spell_id)
{
	return IsShadowStepSpell(spell_id);
}

bool Perl__IsSuccorSpell(uint16 spell_id)
{
	return IsSuccorSpell(spell_id);
}

bool Perl__IsTeleportSpell(uint16 spell_id)
{
	return IsTeleportSpell(spell_id);
}

bool Perl__IsTranslocateSpell(uint16 spell_id)
{
	return IsTranslocateSpell(spell_id);
}

bool Perl__IsGateSpell(uint16 spell_id)
{
	return IsGateSpell(spell_id);
}

bool Perl__IsIllusionSpell(uint16 spell_id)
{
	return IsIllusionSpell(spell_id);
}

bool Perl__IsLDoNObjectSpell(uint16 spell_id)
{
	return IsLDoNObjectSpell(spell_id);
}

bool Perl__IsHealOverTimeSpell(uint16 spell_id)
{
	return IsHealOverTimeSpell(spell_id);
}

bool Perl__IsCompleteHealSpell(uint16 spell_id)
{
	return IsCompleteHealSpell(spell_id);
}

bool Perl__IsFastHealSpell(uint16 spell_id)
{
	return IsFastHealSpell(spell_id);
}

bool Perl__IsVeryFastHealSpell(uint16 spell_id)
{
	return IsVeryFastHealSpell(spell_id);
}

bool Perl__IsRegularSingleTargetHealSpell(uint16 spell_id)
{
	return IsRegularSingleTargetHealSpell(spell_id);
}

bool Perl__IsRegularGroupHealSpell(uint16 spell_id)
{
	return IsRegularGroupHealSpell(spell_id);
}

bool Perl__IsGroupCompleteHealSpell(uint16 spell_id)
{
	return IsGroupCompleteHealSpell(spell_id);
}

bool Perl__IsGroupHealOverTimeSpell(uint16 spell_id)
{
	return IsGroupHealOverTimeSpell(spell_id);
}

bool Perl__IsDebuffSpell(uint16 spell_id)
{
	return IsDebuffSpell(spell_id);
}

bool Perl__IsResistDebuffSpell(uint16 spell_id)
{
	return IsResistDebuffSpell(spell_id);
}

bool Perl__IsSelfConversionSpell(uint16 spell_id)
{
	return IsSelfConversionSpell(spell_id);
}

bool Perl__IsBuffSpell(uint16 spell_id)
{
	return IsBuffSpell(spell_id);
}

bool Perl__IsPersistDeathSpell(uint16 spell_id)
{
	return IsPersistDeathSpell(spell_id);
}

bool Perl__IsSuspendableSpell(uint16 spell_id)
{
	return IsSuspendableSpell(spell_id);
}

bool Perl__IsCastOnFadeDurationSpell(uint16 spell_id)
{
	return IsCastOnFadeDurationSpell(spell_id);
}

bool Perl__IsDistanceModifierSpell(uint16 spell_id)
{
	return IsDistanceModifierSpell(spell_id);
}

bool Perl__IsRestAllowedSpell(uint16 spell_id)
{
	return IsRestAllowedSpell(spell_id);
}

bool Perl__IsNoDetrimentalSpellAggroSpell(uint16 spell_id)
{
	return IsNoDetrimentalSpellAggroSpell(spell_id);
}

bool Perl__IsStackableDOT(uint16 spell_id)
{
	return IsStackableDOT(spell_id);
}

bool Perl__IsShortDurationBuff(uint16 spell_id)
{
	return IsShortDurationBuff(spell_id);
}

bool Perl__IsTargetRequiredForSpell(uint16 spell_id)
{
	return IsTargetRequiredForSpell(spell_id);
}

bool Perl__IsVirusSpell(uint16 spell_id)
{
	return IsVirusSpell(spell_id);
}

bool Perl__IsValidSpell(uint16 spell_id)
{
	return IsValidSpell(spell_id);
}

bool Perl__IsEffectIgnoredInStacking(int effect_id)
{
	return IsEffectIgnoredInStacking(effect_id);
}

bool Perl__IsFocusLimit(int effect_id)
{
	return IsFocusLimit(effect_id);
}

bool Perl__IsBardOnlyStackEffect(int effect_id)
{
	return IsBardOnlyStackEffect(effect_id);
}

bool Perl__IsCastWhileInvisibleSpell(uint16 spell_id)
{
	return IsCastWhileInvisibleSpell(spell_id);
}

bool Perl__IsCastRestrictedSpell(uint16 spell_id)
{
	return IsCastRestrictedSpell(spell_id);
}

bool Perl__IsCastNotStandingSpell(uint16 spell_id)
{
	return IsCastNotStandingSpell(spell_id);
}

bool Perl__IsInstrumentModifierAppliedToSpellEffect(uint16 spell_id, int effect_id)
{
	return IsInstrumentModifierAppliedToSpellEffect(spell_id, effect_id);
}

bool Perl__IsBlankSpellEffect(uint16 spell_id, int effect_index)
{
	return IsBlankSpellEffect(spell_id, effect_index);
}

uint16 Perl__GetSpellTriggerSpellID(uint16 spell_id, int effect_id)
{
	return GetSpellTriggerSpellID(spell_id, effect_id);
}

uint8 Perl__GetSpellMinimumLevel(uint16 spell_id)
{
	return GetSpellMinimumLevel(spell_id);
}

int Perl__GetSpellResistType(uint16 spell_id)
{
	return GetSpellResistType(spell_id);
}

int Perl__GetSpellTargetType(uint16 spell_id)
{
	return GetSpellTargetType(spell_id);
}

int Perl__GetSpellPartialMeleeRuneReduction(uint16 spell_id)
{
	return GetSpellPartialMeleeRuneReduction(spell_id);
}

int Perl__GetSpellPartialMagicRuneReduction(uint16 spell_id)
{
	return GetSpellPartialMagicRuneReduction(spell_id);
}

int Perl__GetSpellPartialMeleeRuneAmount(uint16 spell_id)
{
	return GetSpellPartialMeleeRuneAmount(spell_id);
}

int Perl__GetSpellPartialMagicRuneAmount(uint16 spell_id)
{
	return GetSpellPartialMagicRuneAmount(spell_id);
}

int Perl__GetSpellViralMinimumSpreadTime(uint16 spell_id)
{
	return GetSpellViralMinimumSpreadTime(spell_id);
}

int Perl__GetSpellViralMaximumSpreadTime(uint16 spell_id)
{
	return GetSpellViralMaximumSpreadTime(spell_id);
}

int Perl__GetSpellViralSpreadRange(uint16 spell_id)
{
	return GetSpellViralSpreadRange(spell_id);
}

int Perl__GetSpellProcLimitTimer(uint16 spell_id, int proc_type)
{
	return GetSpellProcLimitTimer(spell_id, proc_type);
}

int Perl__GetSpellEffectDescriptionNumber(uint16 spell_id)
{
	return GetSpellEffectDescriptionNumber(spell_id);
}

int Perl__GetSpellFuriousBash(uint16 spell_id)
{
	return GetSpellFuriousBash(spell_id);
}

bool Perl__IsSpellUsableInThisZoneType(uint16 spell_id)
{
	return IsSpellUsableInThisZoneType(spell_id, zone->GetZoneType());
}

bool Perl__IsSpellUsableInThisZoneType(uint16 spell_id, uint8 zone_type)
{
	return IsSpellUsableInThisZoneType(spell_id, zone_type);
}

int Perl__GetSpellEffectIndex(uint16 spell_id, int effect_id)
{
	return GetSpellEffectIndex(spell_id, effect_id);
}

int Perl__CalculatePoisonCounters(uint16 spell_id)
{
	return CalculatePoisonCounters(spell_id);
}

int Perl__CalculateDiseaseCounters(uint16 spell_id)
{
	return CalculateDiseaseCounters(spell_id);
}

int Perl__CalculateCurseCounters(uint16 spell_id)
{
	return CalculateCurseCounters(spell_id);
}

int Perl__CalculateCorruptionCounters(uint16 spell_id)
{
	return CalculateCorruptionCounters(spell_id);
}

int Perl__CalculateCounters(uint16 spell_id)
{
	return CalculateCounters(spell_id);
}

int8 Perl__GetSpellResurrectionSicknessCheck(uint16 spell_id_one, uint16 spell_id_two)
{
	return GetSpellResurrectionSicknessCheck(spell_id_one, spell_id_two);
}

int Perl__GetSpellNimbusEffect(uint16 spell_id)
{
	return GetSpellNimbusEffect(spell_id);
}

void Perl__FlyMode(GravityBehavior flymode)
{
	quest_manager.FlyMode(flymode);
}

int Perl__FactionValue()
{
	return quest_manager.FactionValue();
}

void Perl__enabletitle(int title_set)
{
	quest_manager.enabletitle(title_set);
}

bool Perl__checktitle(int title_set)
{
	return quest_manager.checktitle(title_set);
}

void Perl__removetitle(int title_set)
{
	quest_manager.removetitle(title_set);
}

void Perl__wearchange(uint8 slot, uint32 texture_id)
{
	quest_manager.wearchange(slot, texture_id);
}

void Perl__wearchange(uint8 slot, uint32 texture_id, uint32 hero_forge_model_id)
{
	quest_manager.wearchange(slot, texture_id, hero_forge_model_id);
}

void Perl__wearchange(uint8 slot, uint32 texture_id, uint32 hero_forge_model_id, uint32 elite_material_id)
{
	quest_manager.wearchange(slot, texture_id, hero_forge_model_id, elite_material_id);
}

void Perl__voicetell(const char* client_name, int macro_id, int race_id, int gender_id)
{
	quest_manager.voicetell(client_name, macro_id, race_id, gender_id);
}

void Perl__LearnRecipe(int recipe_id)
{
	quest_manager.LearnRecipe(recipe_id);
}

void Perl__SendMail(const char* to, const char* from, const char* subject, const char* message)
{
	quest_manager.SendMail(to, from, subject, message);
}

int Perl__GetZoneID(const char* zone)
{
	return quest_manager.GetZoneID(zone);
}

std::string Perl__GetZoneLongName(std::string zone)
{
	return quest_manager.GetZoneLongName(zone);
}

std::string Perl__GetZoneLongNameByID(uint32 zone_id)
{
	return quest_manager.GetZoneLongNameByID(zone_id);
}

std::string Perl__GetZoneShortName(uint32 zone_id)
{
	return quest_manager.GetZoneShortName(zone_id);
}

uint32 Perl__GetTimeSeconds()
{
	return Timer::GetTimeSeconds();
}

bool Perl__enablerecipe(uint32 recipe_id)
{
	return quest_manager.EnableRecipe(recipe_id);
}

bool Perl__disablerecipe(uint32 recipe_id)
{
	return quest_manager.DisableRecipe(recipe_id);
}

void Perl__clear_npctype_cache(int npc_type_id)
{
	quest_manager.ClearNPCTypeCache(npc_type_id);
}

void Perl__reloadzonestaticdata()
{
	quest_manager.ReloadZoneStaticData();
}

void Perl__qs_send_query(std::string query)
{
	QServ->SendQuery(std::move(query));
}

void Perl__log(int category, const char* message)
{
	if (category < Logs::None || category >= Logs::MaxCategoryID)
		return;

	Log(Logs::General, static_cast<Logs::LogCategory>(category), message);
}

void Perl__debug(const char* message)
{
	Log(Logs::General, Logs::QuestDebug, message);
}

void Perl__debug(const char* message, int level)
{
	if (level < Logs::General || level > Logs::Detail)
		return;

	Log(static_cast<Logs::DebugLevel>(level), Logs::QuestDebug, message);
}

void Perl__log_combat(const char* message)
{
	Log(Logs::General, Logs::Combat, message);
}

void Perl__UpdateZoneHeader(std::string key, std::string value)
{
	quest_manager.UpdateZoneHeader(key, value);
}

void Perl__set_rule(const char* rule_name, const char* rule_value)
{
	RuleManager::Instance()->SetRule(rule_name, rule_value);
}

std::string Perl__get_rule(const char* rule_name)
{
	std::string rule_value;
	RuleManager::Instance()->GetRule(rule_name, rule_value);
	return rule_value;
}

std::string Perl__get_data(std::string bucket_key)
{
	return DataBucket::GetData(&database, bucket_key);
}

std::string Perl__get_data_expires(std::string bucket_key)
{
	return DataBucket::GetDataExpires(&database, bucket_key);
}

void Perl__set_data(std::string key, std::string value)
{
	DataBucket::SetData(&database, key, value);
}

void Perl__set_data(std::string key, std::string value, std::string expires_at)
{
	DataBucket::SetData(&database, key, value, expires_at);
}

bool Perl__delete_data(std::string bucket_key)
{
	return DataBucket::DeleteData(&database, bucket_key);
}

bool Perl__IsClassicEnabled()
{
	return WorldContentService::Instance()->IsClassicEnabled();
}

bool Perl__IsTheRuinsOfKunarkEnabled()
{
	return WorldContentService::Instance()->IsTheRuinsOfKunarkEnabled();
}

bool Perl__IsTheScarsOfVeliousEnabled()
{
	return WorldContentService::Instance()->IsTheScarsOfVeliousEnabled();
}

bool Perl__IsTheShadowsOfLuclinEnabled()
{
	return WorldContentService::Instance()->IsTheShadowsOfLuclinEnabled();
}

bool Perl__IsThePlanesOfPowerEnabled()
{
	return WorldContentService::Instance()->IsThePlanesOfPowerEnabled();
}

bool Perl__IsTheLegacyOfYkeshaEnabled()
{
	return WorldContentService::Instance()->IsTheLegacyOfYkeshaEnabled();
}

bool Perl__IsLostDungeonsOfNorrathEnabled()
{
	return WorldContentService::Instance()->IsLostDungeonsOfNorrathEnabled();
}

bool Perl__IsGatesOfDiscordEnabled()
{
	return WorldContentService::Instance()->IsGatesOfDiscordEnabled();
}

bool Perl__IsOmensOfWarEnabled()
{
	return WorldContentService::Instance()->IsOmensOfWarEnabled();
}

bool Perl__IsDragonsOfNorrathEnabled()
{
	return WorldContentService::Instance()->IsDragonsOfNorrathEnabled();
}

bool Perl__IsDepthsOfDarkhollowEnabled()
{
	return WorldContentService::Instance()->IsDepthsOfDarkhollowEnabled();
}

bool Perl__IsProphecyOfRoEnabled()
{
	return WorldContentService::Instance()->IsProphecyOfRoEnabled();
}

bool Perl__IsTheSerpentsSpineEnabled()
{
	return WorldContentService::Instance()->IsTheSerpentsSpineEnabled();
}

bool Perl__IsTheBuriedSeaEnabled()
{
	return WorldContentService::Instance()->IsTheBuriedSeaEnabled();
}

bool Perl__IsSecretsOfFaydwerEnabled()
{
	return WorldContentService::Instance()->IsSecretsOfFaydwerEnabled();
}

bool Perl__IsSeedsOfDestructionEnabled()
{
	return WorldContentService::Instance()->IsSeedsOfDestructionEnabled();
}

bool Perl__IsUnderfootEnabled()
{
	return WorldContentService::Instance()->IsUnderfootEnabled();
}

bool Perl__IsHouseOfThuleEnabled()
{
	return WorldContentService::Instance()->IsHouseOfThuleEnabled();
}

bool Perl__IsVeilOfAlarisEnabled()
{
	return WorldContentService::Instance()->IsVeilOfAlarisEnabled();
}

bool Perl__IsRainOfFearEnabled()
{
	return WorldContentService::Instance()->IsRainOfFearEnabled();
}

bool Perl__IsCallOfTheForsakenEnabled()
{
	return WorldContentService::Instance()->IsCallOfTheForsakenEnabled();
}

bool Perl__IsTheDarkenedSeaEnabled()
{
	return WorldContentService::Instance()->IsTheDarkenedSeaEnabled();
}

bool Perl__IsTheBrokenMirrorEnabled()
{
	return WorldContentService::Instance()->IsTheBrokenMirrorEnabled();
}

bool Perl__IsEmpiresOfKunarkEnabled()
{
	return WorldContentService::Instance()->IsEmpiresOfKunarkEnabled();
}

bool Perl__IsRingOfScaleEnabled()
{
	return WorldContentService::Instance()->IsRingOfScaleEnabled();
}

bool Perl__IsTheBurningLandsEnabled()
{
	return WorldContentService::Instance()->IsTheBurningLandsEnabled();
}

bool Perl__IsTormentOfVeliousEnabled()
{
	return WorldContentService::Instance()->IsTormentOfVeliousEnabled();
}

bool Perl__IsCurrentExpansionClassic()
{
	return WorldContentService::Instance()->IsCurrentExpansionClassic();
}

bool Perl__IsCurrentExpansionTheRuinsOfKunark()
{
	return WorldContentService::Instance()->IsCurrentExpansionTheRuinsOfKunark();
}

bool Perl__IsCurrentExpansionTheScarsOfVelious()
{
	return WorldContentService::Instance()->IsCurrentExpansionTheScarsOfVelious();
}

bool Perl__IsCurrentExpansionTheShadowsOfLuclin()
{
	return WorldContentService::Instance()->IsCurrentExpansionTheShadowsOfLuclin();
}

bool Perl__IsCurrentExpansionThePlanesOfPower()
{
	return WorldContentService::Instance()->IsCurrentExpansionThePlanesOfPower();
}

bool Perl__IsCurrentExpansionTheLegacyOfYkesha()
{
	return WorldContentService::Instance()->IsCurrentExpansionTheLegacyOfYkesha();
}

bool Perl__IsCurrentExpansionLostDungeonsOfNorrath()
{
	return WorldContentService::Instance()->IsCurrentExpansionLostDungeonsOfNorrath();
}

bool Perl__IsCurrentExpansionGatesOfDiscord()
{
	return WorldContentService::Instance()->IsCurrentExpansionGatesOfDiscord();
}

bool Perl__IsCurrentExpansionOmensOfWar()
{
	return WorldContentService::Instance()->IsCurrentExpansionOmensOfWar();
}

bool Perl__IsCurrentExpansionDragonsOfNorrath()
{
	return WorldContentService::Instance()->IsCurrentExpansionDragonsOfNorrath();
}

bool Perl__IsCurrentExpansionDepthsOfDarkhollow()
{
	return WorldContentService::Instance()->IsCurrentExpansionDepthsOfDarkhollow();
}

bool Perl__IsCurrentExpansionProphecyOfRo()
{
	return WorldContentService::Instance()->IsCurrentExpansionProphecyOfRo();
}

bool Perl__IsCurrentExpansionTheSerpentsSpine()
{
	return WorldContentService::Instance()->IsCurrentExpansionTheSerpentsSpine();
}

bool Perl__IsCurrentExpansionTheBuriedSea()
{
	return WorldContentService::Instance()->IsCurrentExpansionTheBuriedSea();
}

bool Perl__IsCurrentExpansionSecretsOfFaydwer()
{
	return WorldContentService::Instance()->IsCurrentExpansionSecretsOfFaydwer();
}

bool Perl__IsCurrentExpansionSeedsOfDestruction()
{
	return WorldContentService::Instance()->IsCurrentExpansionSeedsOfDestruction();
}

bool Perl__IsCurrentExpansionUnderfoot()
{
	return WorldContentService::Instance()->IsCurrentExpansionUnderfoot();
}

bool Perl__IsCurrentExpansionHouseOfThule()
{
	return WorldContentService::Instance()->IsCurrentExpansionHouseOfThule();
}

bool Perl__IsCurrentExpansionVeilOfAlaris()
{
	return WorldContentService::Instance()->IsCurrentExpansionVeilOfAlaris();
}

bool Perl__IsCurrentExpansionRainOfFear()
{
	return WorldContentService::Instance()->IsCurrentExpansionRainOfFear();
}

bool Perl__IsCurrentExpansionCallOfTheForsaken()
{
	return WorldContentService::Instance()->IsCurrentExpansionCallOfTheForsaken();
}

bool Perl__IsCurrentExpansionTheDarkenedSea()
{
	return WorldContentService::Instance()->IsCurrentExpansionTheDarkenedSea();
}

bool Perl__IsCurrentExpansionTheBrokenMirror()
{
	return WorldContentService::Instance()->IsCurrentExpansionTheBrokenMirror();
}

bool Perl__IsCurrentExpansionEmpiresOfKunark()
{
	return WorldContentService::Instance()->IsCurrentExpansionEmpiresOfKunark();
}

bool Perl__IsCurrentExpansionRingOfScale()
{
	return WorldContentService::Instance()->IsCurrentExpansionRingOfScale();
}

bool Perl__IsCurrentExpansionTheBurningLands()
{
	return WorldContentService::Instance()->IsCurrentExpansionTheBurningLands();
}

bool Perl__IsCurrentExpansionTormentOfVelious()
{
	return WorldContentService::Instance()->IsCurrentExpansionTormentOfVelious();
}

bool Perl__IsContentFlagEnabled(std::string flag_name)
{
	return WorldContentService::Instance()->IsContentFlagEnabled(flag_name);
}

void Perl__SetContentFlag(std::string flag_name, bool enabled)
{
	WorldContentService::Instance()->SetContentFlag(flag_name, enabled);
	zone->ReloadContentFlags();
}

DynamicZone* Perl__get_expedition()
{
	if (zone && zone->GetInstanceID() != 0)
	{
		return DynamicZone::FindExpeditionByZone(zone->GetZoneID(), zone->GetInstanceID());
	}

	return nullptr;
}

DynamicZone* Perl__get_expedition_by_char_id(uint32 char_id)
{
	return DynamicZone::FindExpeditionByCharacter(char_id);
}

DynamicZone* Perl__get_expedition_by_dz_id(uint32 dz_id)
{
	return DynamicZone::FindDynamicZoneByID(dz_id, DynamicZoneType::Expedition);
}

DynamicZone* Perl__get_expedition_by_zone_instance(uint32 zone_id, uint32 instance_id)
{
	return DynamicZone::FindExpeditionByZone(zone_id, instance_id);
}

perl::reference Perl__get_expedition_lockout_by_char_id(uint32 char_id, std::string expedition_name, std::string event_name)
{
	perl::hash table;

	auto lockouts = DynamicZone::GetCharacterLockouts(char_id);

	auto it = std::find_if(lockouts.begin(), lockouts.end(), [&](const DzLockout& lockout) {
		return lockout.IsSame(expedition_name, event_name);
	});

	if (it != lockouts.end())
	{
		table["remaining"] = it->GetSecondsRemaining();
		table["uuid"] = it->UUID();
	}

	return perl::reference(table);
}

perl::reference Perl__get_expedition_lockouts_by_char_id(uint32 char_id)
{
	perl::hash table;

	auto lockouts = DynamicZone::GetCharacterLockouts(char_id);
	for (const auto& lockout : lockouts)
	{
		if (!table.exists(lockout.DzName()))
		{
			table[lockout.DzName()] = perl::reference(perl::hash());
		}

		perl::hash expedition_table = table[lockout.DzName()];
		if (!expedition_table.exists(lockout.Event()))
		{
			expedition_table[lockout.Event()] = perl::reference(perl::hash());
		}

		perl::hash event_table = expedition_table[lockout.Event()];
		event_table["remaining"] = lockout.GetSecondsRemaining();
		event_table["uuid"] = lockout.UUID();
	}

	return perl::reference(table);
}

perl::reference Perl__get_expedition_lockouts_by_char_id(uint32 char_id, std::string expedition_name)
{
	perl::hash table;

	auto lockouts = DynamicZone::GetCharacterLockouts(char_id);
	for (const auto& lockout : lockouts)
	{
		if (lockout.DzName() == expedition_name)
		{
			if (!table.exists(lockout.Event()))
			{
				table[lockout.Event()] = perl::reference(perl::hash());
			}
			perl::hash event_table = table[lockout.Event()];
			event_table["remaining"] = lockout.GetSecondsRemaining();
			event_table["uuid"] = lockout.UUID();
		}
	}

	return perl::reference(table);
}

void Perl__add_expedition_lockout_all_clients(std::string expedition_name, std::string event_name, uint32 seconds)
{
	auto lockout = DzLockout::Create(expedition_name, event_name, seconds);
	DynamicZone::AddClientsLockout(lockout);
}

void Perl__add_expedition_lockout_all_clients(std::string expedition_name, std::string event_name, uint32 seconds, std::string uuid)
{
	auto lockout = DzLockout::Create(expedition_name, event_name, seconds, uuid);
	DynamicZone::AddClientsLockout(lockout);
}

void Perl__add_expedition_lockout_by_char_id(uint32 char_id, std::string expedition_name, std::string event_name, uint32 seconds)
{
	DynamicZone::AddCharacterLockout(char_id, expedition_name, event_name, seconds);
}

void Perl__add_expedition_lockout_by_char_id(uint32 char_id, std::string expedition_name, std::string event_name, uint32 seconds, std::string uuid)
{
	DynamicZone::AddCharacterLockout(char_id, expedition_name, event_name, seconds, uuid);
}

void Perl__remove_expedition_lockout_by_char_id(uint32 char_id, std::string expedition_name, std::string event_name)
{
	DynamicZone::RemoveCharacterLockouts(char_id, expedition_name, event_name);
}

void Perl__remove_all_expedition_lockouts_by_char_id(uint32 char_id)
{
	DynamicZone::RemoveCharacterLockouts(char_id);
}

void Perl__remove_all_expedition_lockouts_by_char_id(uint32 char_id, std::string expedition_name)
{
	DynamicZone::RemoveCharacterLockouts(char_id, expedition_name);
}

EQ::ItemInstance* Perl__createitem(uint32 item_id)
{
	return database.CreateItem(item_id);
}

EQ::ItemInstance* Perl__createitem(uint32 item_id, int16 charges)
{
	return database.CreateItem(item_id, charges);
}

EQ::ItemInstance* Perl__createitem(uint32 item_id, int16 charges, uint32 augment_one)
{
	return database.CreateItem(item_id, charges, augment_one);
}

EQ::ItemInstance* Perl__createitem(uint32 item_id, int16 charges, uint32 augment_one, uint32 augment_two)
{
	return database.CreateItem(item_id, charges, augment_one, augment_two);
}

EQ::ItemInstance* Perl__createitem(uint32 item_id, int16 charges, uint32 augment_one, uint32 augment_two, uint32 augment_three)
{
	return database.CreateItem(item_id, charges, augment_one, augment_two, augment_three);
}

EQ::ItemInstance* Perl__createitem(uint32 item_id, int16 charges, uint32 augment_one, uint32 augment_two, uint32 augment_three, uint32 augment_four)
{
	return database.CreateItem(item_id, charges, augment_one, augment_two, augment_three, augment_four);
}

EQ::ItemInstance* Perl__createitem(uint32 item_id, int16 charges, uint32 augment_one, uint32 augment_two, uint32 augment_three, uint32 augment_four, uint32 augment_five)
{
	return database.CreateItem(item_id, charges, augment_one, augment_two, augment_three, augment_four, augment_five);
}

EQ::ItemInstance* Perl__createitem(uint32 item_id, int16 charges, uint32 augment_one, uint32 augment_two, uint32 augment_three, uint32 augment_four, uint32 augment_five, uint32 augment_six)
{
	return database.CreateItem(item_id, charges, augment_one, augment_two, augment_three, augment_four, augment_five, augment_six);
}

EQ::ItemInstance* Perl__createitem(uint32 item_id, int16 charges, uint32 augment_one, uint32 augment_two, uint32 augment_three, uint32 augment_four, uint32 augment_five, uint32 augment_six, bool attuned)
{
	return database.CreateItem(item_id, charges, augment_one, augment_two, augment_three, augment_four, augment_five, augment_six, attuned);
}

std::string Perl__secondstotime(int duration)
{
	return Strings::SecondsToTime(duration);
}

uint32 Perl__timetoseconds(std::string time_string)
{
	return Strings::TimeToSeconds(time_string);
}

std::string Perl__gethexcolorcode(std::string color_name)
{
	return quest_manager.gethexcolorcode(color_name);
}

double Perl__getaaexpmodifierbycharid(uint32 character_id, uint32 zone_id)
{
	return quest_manager.GetAAEXPModifierByCharID(character_id, zone_id);
}

double Perl__getaaexpmodifierbycharid(uint32 character_id, uint32 zone_id, int16 instance_version)
{
	return quest_manager.GetAAEXPModifierByCharID(character_id, zone_id, instance_version);
}

double Perl__getexpmodifierbycharid(uint32 character_id, uint32 zone_id)
{
	return quest_manager.GetEXPModifierByCharID(character_id, zone_id);
}

double Perl__getexpmodifierbycharid(uint32 character_id, uint32 zone_id, int16 instance_version)
{
	return quest_manager.GetEXPModifierByCharID(character_id, zone_id, instance_version);
}

void Perl__setaaexpmodifierbycharid(uint32 character_id, uint32 zone_id, double aa_modifier)
{
	quest_manager.SetAAEXPModifierByCharID(character_id, zone_id, aa_modifier);
}

void Perl__setaaexpmodifierbycharid(uint32 character_id, uint32 zone_id, double aa_modifier, int16 instance_version)
{
	quest_manager.SetAAEXPModifierByCharID(character_id, zone_id, aa_modifier, instance_version);
}

void Perl__setexpmodifierbycharid(uint32 character_id, uint32 zone_id, double exp_modifier)
{
	quest_manager.SetEXPModifierByCharID(character_id, zone_id, exp_modifier);
}

void Perl__setexpmodifierbycharid(uint32 character_id, uint32 zone_id, double exp_modifier, int16 instance_version)
{
	quest_manager.SetEXPModifierByCharID(character_id, zone_id, exp_modifier, instance_version);
}

std::string Perl__getcleannpcnamebyid(uint32 npc_id)
{
	return quest_manager.getcleannpcnamebyid(npc_id);
}

std::string Perl__getgendername(uint32 gender_id)
{
	return quest_manager.getgendername(gender_id);
}

std::string Perl__getdeityname(uint32 deity_id)
{
	return quest_manager.getdeityname(deity_id);
}

std::string Perl__getinventoryslotname(int16 slot_id)
{
	return quest_manager.getinventoryslotname(slot_id);
}

void Perl__rename(std::string name)
{
	quest_manager.rename(name);
}

std::string Perl__get_data_remaining(std::string bucket_name)
{
	return DataBucket::GetDataRemaining(&database, bucket_name);
}

const int Perl__getitemstat(uint32 item_id, std::string identifier)
{
	return quest_manager.getitemstat(item_id, identifier);
}

int Perl__getspellstat(uint32 spell_id, std::string stat_identifier)
{
	return quest_manager.getspellstat(spell_id, stat_identifier);
}

int Perl__getspellstat(uint32 spell_id, std::string stat_identifier, uint8 slot)
{
	return quest_manager.getspellstat(spell_id, stat_identifier, slot);
}

void Perl__crosszoneaddldonlossbycharid(int character_id, uint32 theme_id)
{
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_Character, CZLDoNUpdateSubtype_AddLoss, character_id, theme_id);
}

void Perl__crosszoneaddldonlossbygroupid(int group_id, uint32 theme_id)
{
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_Group, CZLDoNUpdateSubtype_AddLoss, group_id, theme_id);
}

void Perl__crosszoneaddldonlossbyraidid(int raid_id, uint32 theme_id)
{
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_Raid, CZLDoNUpdateSubtype_AddLoss, raid_id, theme_id);
}

void Perl__crosszoneaddldonlossbyguildid(int guild_id, uint32 theme_id)
{
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_Guild, CZLDoNUpdateSubtype_AddLoss, guild_id, theme_id);
}

void Perl__crosszoneaddldonlossbyexpeditionid(uint32 expedition_id, uint32 theme_id)
{
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_Expedition, CZLDoNUpdateSubtype_AddLoss, expedition_id, theme_id);
}

void Perl__crosszoneaddldonlossbyclientname(const char* client_name, uint32 theme_id)
{
	int update_identifier = 0;
	int points = 1;
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_ClientName, CZLDoNUpdateSubtype_AddLoss, update_identifier, theme_id, points, client_name);
}

void Perl__crosszoneaddldonpointsbycharid(int character_id, uint32 theme_id, int points)
{
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_Character, CZLDoNUpdateSubtype_AddPoints, character_id, theme_id, points);
}

void Perl__crosszoneaddldonpointsbygroupid(int group_id, uint32 theme_id, int points)
{
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_Group, CZLDoNUpdateSubtype_AddPoints, group_id, theme_id, points);
}

void Perl__crosszoneaddldonpointsbyraidid(int raid_id, uint32 theme_id, int points)
{
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_Raid, CZLDoNUpdateSubtype_AddPoints, raid_id, theme_id, points);
}

void Perl__crosszoneaddldonpointsbyguildid(int guild_id, uint32 theme_id, int points)
{
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_Guild, CZLDoNUpdateSubtype_AddPoints, guild_id, theme_id, points);
}

void Perl__crosszoneaddldonpointsbyexpeditionid(uint32 expedition_id, uint32 theme_id, int points)
{
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_Expedition, CZLDoNUpdateSubtype_AddPoints, expedition_id, theme_id, points);
}

void Perl__crosszoneaddldonpointsbyclientname(const char* client_name, uint32 theme_id, int points)
{
	int update_identifier = 0;
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_ClientName, CZLDoNUpdateSubtype_AddPoints, update_identifier, theme_id, points, client_name);
}

void Perl__crosszoneaddldonwinbycharid(int character_id, uint32 theme_id)
{
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_Character, CZLDoNUpdateSubtype_AddWin, character_id, theme_id);
}

void Perl__crosszoneaddldonwinbygroupid(int group_id, uint32 theme_id)
{
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_Group, CZLDoNUpdateSubtype_AddWin, group_id, theme_id);
}

void Perl__crosszoneaddldonwinbyraidid(int raid_id, uint32 theme_id)
{
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_Raid, CZLDoNUpdateSubtype_AddWin, raid_id, theme_id);
}

void Perl__crosszoneaddldonwinbyguildid(int guild_id, uint32 theme_id)
{
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_Guild, CZLDoNUpdateSubtype_AddWin, guild_id, theme_id);
}

void Perl__crosszoneaddldonwinbyexpeditionid(uint32 expedition_id, uint32 theme_id)
{
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_Expedition, CZLDoNUpdateSubtype_AddWin, expedition_id, theme_id);
}

void Perl__crosszoneaddldonwinbyclientname(const char* client_name, uint32 theme_id)
{
	int update_identifier = 0;
	int points = 1;
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_ClientName, CZLDoNUpdateSubtype_AddWin, update_identifier, theme_id, points, client_name);
}

void Perl__crosszoneassigntaskbycharid(int character_id, uint32 task_id)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Character, CZTaskUpdateSubtype_AssignTask, character_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszoneassigntaskbycharid(int character_id, uint32 task_id, bool enforce_level_requirement)
{
	int task_subidentifier = -1;
	int update_count = 1;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Character, CZTaskUpdateSubtype_AssignTask, character_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszoneassigntaskbygroupid(int group_id, uint32 task_id)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Group, CZTaskUpdateSubtype_AssignTask, group_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszoneassigntaskbygroupid(int group_id, uint32 task_id, bool enforce_level_requirement)
{
	int task_subidentifier = -1;
	int update_count = 1;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Group, CZTaskUpdateSubtype_AssignTask, group_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszoneassigntaskbyraidid(int raid_id, uint32 task_id)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Raid, CZTaskUpdateSubtype_AssignTask, raid_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszoneassigntaskbyraidid(int raid_id, uint32 task_id, bool enforce_level_requirement)
{
	int task_subidentifier = -1;
	int update_count = 1;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Raid, CZTaskUpdateSubtype_AssignTask, raid_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszoneassigntaskbyguildid(int guild_id, uint32 task_id)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Guild, CZTaskUpdateSubtype_AssignTask, guild_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszoneassigntaskbyguildid(int guild_id, uint32 task_id, bool enforce_level_requirement)
{
	int task_subidentifier = -1;
	int update_count = 1;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Guild, CZTaskUpdateSubtype_AssignTask, guild_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszoneassigntaskbyexpeditionid(uint32 expedition_id, uint32 task_id)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Expedition, CZTaskUpdateSubtype_AssignTask, expedition_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszoneassigntaskbyexpeditionid(uint32 expedition_id, uint32 task_id, bool enforce_level_requirement)
{
	int task_subidentifier = -1;
	int update_count = 1;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Expedition, CZTaskUpdateSubtype_AssignTask, expedition_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszoneassigntaskbyclientname(const char* client_name, uint32 task_id)
{
	int update_identifier = 0;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_ClientName, CZTaskUpdateSubtype_AssignTask, update_identifier, task_id, task_subidentifier, update_count, enforce_level_requirement, client_name);
}

void Perl__crosszoneassigntaskbyclientname(const char* client_name, uint32 task_id, bool enforce_level_requirement)
{
	int update_identifier = 0;
	int task_subidentifier = -1;
	int update_count = 1;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_ClientName, CZTaskUpdateSubtype_AssignTask, update_identifier, task_id, task_subidentifier, update_count, enforce_level_requirement, client_name);
}

void Perl__crosszonecastspellbycharid(int character_id, uint32 spell_id)
{
	quest_manager.CrossZoneSpell(CZUpdateType_Character, CZSpellUpdateSubtype_Cast, character_id, spell_id);
}

void Perl__crosszonecastspellbygroupid(int group_id, uint32 spell_id)
{
	quest_manager.CrossZoneSpell(CZUpdateType_Group, CZSpellUpdateSubtype_Cast, group_id, spell_id);
}

void Perl__crosszonecastspellbyraidid(int raid_id, uint32 spell_id)
{
	quest_manager.CrossZoneSpell(CZUpdateType_Raid, CZSpellUpdateSubtype_Cast, raid_id, spell_id);
}

void Perl__crosszonecastspellbyguildid(int guild_id, uint32 spell_id)
{
	quest_manager.CrossZoneSpell(CZUpdateType_Guild, CZSpellUpdateSubtype_Cast, guild_id, spell_id);
}

void Perl__crosszonecastspellbyexpeditionid(uint32 expedition_id, uint32 spell_id)
{
	quest_manager.CrossZoneSpell(CZUpdateType_Expedition, CZSpellUpdateSubtype_Cast, expedition_id, spell_id);
}

void Perl__crosszonecastspellbyclientname(const char* client_name, uint32 spell_id)
{
	int update_identifier = 0;
	quest_manager.CrossZoneSpell(CZUpdateType_ClientName, CZSpellUpdateSubtype_Cast, update_identifier, spell_id, client_name);
}

void Perl__crosszonedialoguewindowbycharid(int character_id, const char* message)
{
	quest_manager.CrossZoneDialogueWindow(CZUpdateType_Character, character_id, message);
}

void Perl__crosszonedialoguewindowbygroupid(int group_id, const char* message)
{
	quest_manager.CrossZoneDialogueWindow(CZUpdateType_Group, group_id, message);
}

void Perl__crosszonedialoguewindowbyraidid(int raid_id, const char* message)
{
	quest_manager.CrossZoneDialogueWindow(CZUpdateType_Raid, raid_id, message);
}

void Perl__crosszonedialoguewindowbyguildid(int guild_id, const char* message)
{
	quest_manager.CrossZoneDialogueWindow(CZUpdateType_Guild, guild_id, message);
}

void Perl__crosszonedialoguewindowbyexpeditionid(uint32 expedition_id, const char* message)
{
	quest_manager.CrossZoneDialogueWindow(CZUpdateType_Expedition, expedition_id, message);
}

void Perl__crosszonedialoguewindowbyclientname(const char* client_name, const char* message)
{
	int update_identifier = 0;
	quest_manager.CrossZoneDialogueWindow(CZUpdateType_ClientName, update_identifier, message, client_name);
}

void Perl__crosszonedisabletaskbycharid(int character_id, uint32 task_id)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Character, CZTaskUpdateSubtype_DisableTask, character_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszonedisabletaskbygroupid(int group_id, uint32 task_id)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Group, CZTaskUpdateSubtype_DisableTask, group_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszonedisabletaskbyraidid(int raid_id, uint32 task_id)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Raid, CZTaskUpdateSubtype_DisableTask, raid_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszonedisabletaskbyguildid(int guild_id, uint32 task_id)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Guild, CZTaskUpdateSubtype_DisableTask, guild_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszonedisabletaskbyexpeditionid(uint32 expedition_id, uint32 task_id)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Expedition, CZTaskUpdateSubtype_DisableTask, expedition_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszonedisabletaskbyclientname(const char* client_name, uint32 task_id)
{
	int update_identifier = 0;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_ClientName, CZTaskUpdateSubtype_DisableTask, update_identifier, task_id, task_subidentifier, update_count, enforce_level_requirement, client_name);
}

void Perl__crosszoneenabletaskbycharid(int character_id, uint32 task_id)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Character, CZTaskUpdateSubtype_EnableTask, character_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszoneenabletaskbygroupid(int group_id, uint32 task_id)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Group, CZTaskUpdateSubtype_EnableTask, group_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszoneenabletaskbyraidid(int raid_id, uint32 task_id)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Raid, CZTaskUpdateSubtype_EnableTask, raid_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszoneenabletaskbyguildid(int guild_id, uint32 task_id)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Guild, CZTaskUpdateSubtype_EnableTask, guild_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszoneenabletaskbyexpeditionid(uint32 expedition_id, uint32 task_id)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Expedition, CZTaskUpdateSubtype_EnableTask, expedition_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszoneenabletaskbyclientname(const char* client_name, uint32 task_id)
{
	int update_identifier = 0;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_ClientName, CZTaskUpdateSubtype_EnableTask, update_identifier, task_id, task_subidentifier, update_count, enforce_level_requirement, client_name);
}

void Perl__crosszonefailtaskbycharid(int character_id, uint32 task_id)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Character, CZTaskUpdateSubtype_FailTask, character_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszonefailtaskbygroupid(int group_id, uint32 task_id)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Group, CZTaskUpdateSubtype_FailTask, group_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszonefailtaskbyraidid(int raid_id, uint32 task_id)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Raid, CZTaskUpdateSubtype_FailTask, raid_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszonefailtaskbyguildid(int guild_id, uint32 task_id)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Guild, CZTaskUpdateSubtype_FailTask, guild_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszonefailtaskbyexpeditionid(uint32 expedition_id, uint32 task_id)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Expedition, CZTaskUpdateSubtype_FailTask, expedition_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszonefailtaskbyclientname(const char* client_name, uint32 task_id)
{
	int update_identifier = 0;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_ClientName, CZTaskUpdateSubtype_FailTask, update_identifier, task_id, task_subidentifier, update_count, enforce_level_requirement, client_name);
}

void Perl__crosszonemarqueebycharid(int character_id, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message)
{
	quest_manager.CrossZoneMarquee(CZUpdateType_Character, character_id, type, priority, fade_in, fade_out, duration, message);
}

void Perl__crosszonemarqueebygroupid(int group_id, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message)
{
	quest_manager.CrossZoneMarquee(CZUpdateType_Group, group_id, type, priority, fade_in, fade_out, duration, message);
}

void Perl__crosszonemarqueebyraidid(int raid_id, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message)
{
	quest_manager.CrossZoneMarquee(CZUpdateType_Raid, raid_id, type, priority, fade_in, fade_out, duration, message);
}

void Perl__crosszonemarqueebyguildid(int guild_id, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message)
{
	quest_manager.CrossZoneMarquee(CZUpdateType_Guild, guild_id, type, priority, fade_in, fade_out, duration, message);
}

void Perl__crosszonemarqueebyexpeditionid(uint32 expedition_id, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message)
{
	quest_manager.CrossZoneMarquee(CZUpdateType_Expedition, expedition_id, type, priority, fade_in, fade_out, duration, message);
}

void Perl__crosszonemarqueebyclientname(const char* client_name, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message)
{
	int update_identifier = 0;
	quest_manager.CrossZoneMarquee(CZUpdateType_ClientName, update_identifier, type, priority, fade_in, fade_out, duration, message, client_name);
}

void Perl__crosszonemessageplayerbycharid(int character_id, uint32 type, const char* message)
{
	quest_manager.CrossZoneMessage(CZUpdateType_Character, character_id, type, message);
}

void Perl__crosszonemessageplayerbygroupid(int group_id, uint32 type, const char* message)
{
	quest_manager.CrossZoneMessage(CZUpdateType_Group, group_id, type, message);
}

void Perl__crosszonemessageplayerbyraidid(int raid_id, uint32 type, const char* message)
{
	quest_manager.CrossZoneMessage(CZUpdateType_Raid, raid_id, type, message);
}

void Perl__crosszonemessageplayerbyguildid(int guild_id, uint32 type, const char* message)
{
	quest_manager.CrossZoneMessage(CZUpdateType_Guild, guild_id, type, message);
}

void Perl__crosszonemessageplayerbyexpeditionid(uint32 expedition_id, uint32 type, const char* message)
{
	quest_manager.CrossZoneMessage(CZUpdateType_Expedition, expedition_id, type, message);
}

void Perl__crosszonemessageplayerbyname(const char* client_name, uint32 type, const char* message)
{
	int update_identifier = 0;
	quest_manager.CrossZoneMessage(CZUpdateType_ClientName, update_identifier, type, message, client_name);
}

void Perl__crosszonemoveplayerbycharid(uint32 character_id, std::string zone_short_name)
{
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.update_identifier = character_id,
			.update_type = CZUpdateType_Character,
			.update_subtype = CZMoveUpdateSubtype_MoveZone,
			.zone_short_name = zone_short_name,
		}
	);
}

void Perl__crosszonemoveplayerbycharid(uint32 character_id, std::string zone_short_name, float x, float y, float z)
{
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

void Perl__crosszonemoveplayerbycharid(uint32 character_id, std::string zone_short_name, float x, float y, float z, float heading)
{
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

void Perl__crosszonemoveplayerbygroupid(uint32 group_id, std::string zone_short_name)
{
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.update_identifier = group_id,
			.update_type = CZUpdateType_Group,
			.update_subtype = CZMoveUpdateSubtype_MoveZone,
			.zone_short_name = zone_short_name,
		}
	);
}

void Perl__crosszonemoveplayerbygroupid(uint32 group_id, std::string zone_short_name, float x, float y, float z)
{
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

void Perl__crosszonemoveplayerbygroupid(uint32 group_id, std::string zone_short_name, float x, float y, float z, float heading)
{
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

void Perl__crosszonemoveplayerbyraidid(uint32 raid_id, std::string zone_short_name)
{
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.update_identifier = raid_id,
			.update_type = CZUpdateType_Raid,
			.update_subtype = CZMoveUpdateSubtype_MoveZone,
			.zone_short_name = zone_short_name,
		}
	);
}

void Perl__crosszonemoveplayerbyraidid(uint32 raid_id, std::string zone_short_name, float x, float y, float z)
{
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

void Perl__crosszonemoveplayerbyraidid(uint32 raid_id, std::string zone_short_name, float x, float y, float z, float heading)
{
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

void Perl__crosszonemoveplayerbyguildid(uint32 guild_id, std::string zone_short_name)
{
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.update_identifier = guild_id,
			.update_type = CZUpdateType_Guild,
			.update_subtype = CZMoveUpdateSubtype_MoveZone,
			.zone_short_name = zone_short_name,
		}
	);
}

void Perl__crosszonemoveplayerbyguildid(uint32 guild_id, std::string zone_short_name, float x, float y, float z)
{
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

void Perl__crosszonemoveplayerbyguildid(uint32 guild_id, std::string zone_short_name, float x, float y, float z, float heading)
{
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

void Perl__crosszonemoveplayerbyexpeditionid(uint32 expedition_id, std::string zone_short_name)
{
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.update_identifier = expedition_id,
			.update_type = CZUpdateType_Expedition,
			.update_subtype = CZMoveUpdateSubtype_MoveZone,
			.zone_short_name = zone_short_name,
		}
	);
}

void Perl__crosszonemoveplayerbyexpeditionid(uint32 expedition_id, std::string zone_short_name, float x, float y, float z)
{
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

void Perl__crosszonemoveplayerbyexpeditionid(uint32 expedition_id, std::string zone_short_name, float x, float y, float z, float heading)
{
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

void Perl__crosszonemoveplayerbyname(std::string client_name, std::string zone_short_name)
{
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.client_name = client_name,
			.update_type = CZUpdateType_ClientName,
			.update_subtype = CZMoveUpdateSubtype_MoveZone,
			.zone_short_name = zone_short_name,
		}
	);
}

void Perl__crosszonemoveplayerbyname(std::string client_name, std::string zone_short_name, float x, float y, float z)
{
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

void Perl__crosszonemoveplayerbyname(std::string client_name, std::string zone_short_name, float x, float y, float z, float heading)
{
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

void Perl__crosszonemoveinstancebycharid(uint32 character_id, uint16 instance_id)
{
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.instance_id = instance_id,
			.update_identifier = character_id,
			.update_type = CZUpdateType_Character,
			.update_subtype = CZMoveUpdateSubtype_MoveZoneInstance,
		}
	);
}

void Perl__crosszonemoveinstancebycharid(uint32 character_id, uint16 instance_id, float x, float y, float z)
{
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

void Perl__crosszonemoveinstancebycharid(uint32 character_id, uint16 instance_id, float x, float y, float z, float heading)
{
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

void Perl__crosszonemoveinstancebygroupid(uint32 group_id, uint16 instance_id)
{
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.instance_id = instance_id,
			.update_identifier = group_id,
			.update_type = CZUpdateType_Group,
			.update_subtype = CZMoveUpdateSubtype_MoveZoneInstance,
		}
	);
}

void Perl__crosszonemoveinstancebygroupid(uint32 group_id, uint16 instance_id, float x, float y, float z)
{
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

void Perl__crosszonemoveinstancebygroupid(uint32 group_id, uint16 instance_id, float x, float y, float z, float heading)
{
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

void Perl__crosszonemoveinstancebyraidid(uint32 raid_id, uint16 instance_id)
{
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.instance_id = instance_id,
			.update_identifier = raid_id,
			.update_type = CZUpdateType_Raid,
			.update_subtype = CZMoveUpdateSubtype_MoveZoneInstance,
		}
	);
}

void Perl__crosszonemoveinstancebyraidid(uint32 raid_id, uint16 instance_id, float x, float y, float z)
{
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

void Perl__crosszonemoveinstancebyraidid(uint32 raid_id, uint16 instance_id, float x, float y, float z, float heading)
{
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

void Perl__crosszonemoveinstancebyguildid(uint32 guild_id, uint16 instance_id)
{
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.instance_id = instance_id,
			.update_identifier = guild_id,
			.update_type = CZUpdateType_Guild,
			.update_subtype = CZMoveUpdateSubtype_MoveZoneInstance,
		}
	);
}

void Perl__crosszonemoveinstancebyguildid(uint32 guild_id, uint16 instance_id, float x, float y, float z)
{
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

void Perl__crosszonemoveinstancebyguildid(uint32 guild_id, uint16 instance_id, float x, float y, float z, float heading)
{
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

void Perl__crosszonemoveinstancebyexpeditionid(uint32 expedition_id, uint16 instance_id)
{
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.instance_id = instance_id,
			.update_identifier = expedition_id,
			.update_type = CZUpdateType_Expedition,
			.update_subtype = CZMoveUpdateSubtype_MoveZoneInstance,
		}
	);
}

void Perl__crosszonemoveinstancebyexpeditionid(uint32 expedition_id, uint16 instance_id, float x, float y, float z)
{
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

void Perl__crosszonemoveinstancebyexpeditionid(uint32 expedition_id, uint16 instance_id, float x, float y, float z, float heading)
{
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

void Perl__crosszonemoveinstancebyclientname(std::string client_name, uint16 instance_id)
{
	quest_manager.CrossZoneMove(
		CZMove_Struct{
			.client_name = client_name,
			.instance_id = instance_id,
			.update_type = CZUpdateType_ClientName,
			.update_subtype = CZMoveUpdateSubtype_MoveZoneInstance,
		}
	);
}

void Perl__crosszonemoveinstancebyclientname(std::string client_name, uint16 instance_id, float x, float y, float z)
{
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

void Perl__crosszonemoveinstancebyclientname(std::string client_name, uint16 instance_id, float x, float y, float z, float heading)
{
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

void Perl__crosszoneremoveldonlossbycharid(int character_id, uint32 theme_id)
{
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_Character, CZLDoNUpdateSubtype_RemoveLoss, character_id, theme_id);
}

void Perl__crosszoneremoveldonlossbygroupid(int group_id, uint32 theme_id)
{
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_Group, CZLDoNUpdateSubtype_RemoveLoss, group_id, theme_id);
}

void Perl__crosszoneremoveldonlossbyraidid(int raid_id, uint32 theme_id)
{
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_Raid, CZLDoNUpdateSubtype_RemoveLoss, raid_id, theme_id);
}

void Perl__crosszoneremoveldonlossbyguildid(int guild_id, uint32 theme_id)
{
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_Guild, CZLDoNUpdateSubtype_RemoveLoss, guild_id, theme_id);
}

void Perl__crosszoneremoveldonlossbyexpeditionid(uint32 expedition_id, uint32 theme_id)
{
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_Expedition, CZLDoNUpdateSubtype_RemoveLoss, expedition_id, theme_id);
}

void Perl__crosszoneremoveldonlossbyclientname(const char* client_name, uint32 theme_id)
{
	int update_identifier = 0;
	int points = 1;
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_ClientName, CZLDoNUpdateSubtype_RemoveLoss, update_identifier, theme_id, points, client_name);
}

void Perl__crosszoneremoveldonwinbycharid(int character_id, uint32 theme_id)
{
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_Character, CZLDoNUpdateSubtype_RemoveWin, character_id, theme_id);
}

void Perl__crosszoneremoveldonwinbygroupid(int group_id, uint32 theme_id)
{
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_Group, CZLDoNUpdateSubtype_RemoveWin, group_id, theme_id);
}

void Perl__crosszoneremoveldonwinbyraidid(int raid_id, uint32 theme_id)
{
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_Raid, CZLDoNUpdateSubtype_RemoveWin, raid_id, theme_id);
}

void Perl__crosszoneremoveldonwinbyguildid(int guild_id, uint32 theme_id)
{
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_Guild, CZLDoNUpdateSubtype_RemoveWin, guild_id, theme_id);
}

void Perl__crosszoneremoveldonwinbyexpeditionid(uint32 expedition_id, uint32 theme_id)
{
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_Expedition, CZLDoNUpdateSubtype_RemoveWin, expedition_id, theme_id);
}

void Perl__crosszoneremoveldonwinbyclientname(const char* client_name, uint32 theme_id)
{
	int update_identifier = 0;
	int points = 1;
	quest_manager.CrossZoneLDoNUpdate(CZUpdateType_ClientName, CZLDoNUpdateSubtype_RemoveWin, update_identifier, theme_id, points, client_name);
}

void Perl__crosszoneremovespellbycharid(int character_id, uint32 spell_id)
{
	quest_manager.CrossZoneSpell(CZUpdateType_Character, CZSpellUpdateSubtype_Remove, character_id, spell_id);
}

void Perl__crosszoneremovespellbygroupid(int group_id, uint32 spell_id)
{
	quest_manager.CrossZoneSpell(CZUpdateType_Group, CZSpellUpdateSubtype_Remove, group_id, spell_id);
}

void Perl__crosszoneremovespellbyraidid(int raid_id, uint32 spell_id)
{
	quest_manager.CrossZoneSpell(CZUpdateType_Raid, CZSpellUpdateSubtype_Remove, raid_id, spell_id);
}

void Perl__crosszoneremovespellbyguildid(int guild_id, uint32 spell_id)
{
	quest_manager.CrossZoneSpell(CZUpdateType_Guild, CZSpellUpdateSubtype_Remove, guild_id, spell_id);
}

void Perl__crosszoneremovespellbyexpeditionid(uint32 expedition_id, uint32 spell_id)
{
	quest_manager.CrossZoneSpell(CZUpdateType_Expedition, CZSpellUpdateSubtype_Remove, expedition_id, spell_id);
}

void Perl__crosszoneremovespellbyclientname(const char* client_name, uint32 spell_id)
{
	int update_identifier = 0;
	quest_manager.CrossZoneSpell(CZUpdateType_ClientName, CZSpellUpdateSubtype_Remove, update_identifier, spell_id, client_name);
}

void Perl__crosszoneremovetaskbycharid(int character_id, uint32 task_id)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Character, CZTaskUpdateSubtype_RemoveTask, character_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszoneremovetaskbygroupid(int group_id, uint32 task_id)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Group, CZTaskUpdateSubtype_RemoveTask, group_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszoneremovetaskbyraidid(int raid_id, uint32 task_id)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Raid, CZTaskUpdateSubtype_RemoveTask, raid_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszoneremovetaskbyguildid(int guild_id, uint32 task_id)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Guild, CZTaskUpdateSubtype_RemoveTask, guild_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszoneremovetaskbyexpeditionid(uint32 expedition_id, uint32 task_id)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Expedition, CZTaskUpdateSubtype_RemoveTask, expedition_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__crosszoneremovetaskbyclientname(const char* client_name, uint32 task_id)
{
	int update_identifier = 0;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_ClientName, CZTaskUpdateSubtype_RemoveTask, update_identifier, task_id, task_subidentifier, update_count, enforce_level_requirement, client_name);
}

void Perl__crosszoneresetactivitybycharid(int character_id, uint32 task_id, int activity_id)
{
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Character, CZTaskUpdateSubtype_ActivityReset, character_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void Perl__crosszoneresetactivitybygroupid(int group_id, uint32 task_id, int activity_id)
{
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Group, CZTaskUpdateSubtype_ActivityReset, group_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void Perl__crosszoneresetactivitybyraidid(int raid_id, uint32 task_id, int activity_id)
{
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Raid, CZTaskUpdateSubtype_ActivityReset, raid_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void Perl__crosszoneresetactivitybyguildid(int guild_id, uint32 task_id, int activity_id)
{
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Guild, CZTaskUpdateSubtype_ActivityReset, guild_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void Perl__crosszoneresetactivitybyexpeditionid(uint32 expedition_id, uint32 task_id, int activity_id)
{
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Expedition, CZTaskUpdateSubtype_ActivityReset, expedition_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void Perl__crosszoneresetactivitybyclientname(const char* client_name, uint32 task_id, int activity_id)
{
	int update_identifier = 0;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_ClientName, CZTaskUpdateSubtype_ActivityReset, update_identifier, task_id, activity_id, update_count, enforce_level_requirement, client_name);
}

void Perl__crosszonesetentityvariablebycharid(int character_id, const char* variable_name, const char* variable_value)
{
	quest_manager.CrossZoneSetEntityVariable(CZUpdateType_Character, character_id, variable_name, variable_value);
}

void Perl__crosszonesetentityvariablebygroupid(int group_id, const char* variable_name, const char* variable_value)
{
	quest_manager.CrossZoneSetEntityVariable(CZUpdateType_Group, group_id, variable_name, variable_value);
}

void Perl__crosszonesetentityvariablebyraidid(int raid_id, const char* variable_name, const char* variable_value)
{
	quest_manager.CrossZoneSetEntityVariable(CZUpdateType_Raid, raid_id, variable_name, variable_value);
}

void Perl__crosszonesetentityvariablebyguildid(int guild_id, const char* variable_name, const char* variable_value)
{
	quest_manager.CrossZoneSetEntityVariable(CZUpdateType_Guild, guild_id, variable_name, variable_value);
}

void Perl__crosszonesetentityvariablebyexpeditionid(uint32 expedition_id, const char* variable_name, const char* variable_value)
{
	quest_manager.CrossZoneSetEntityVariable(CZUpdateType_Expedition, expedition_id, variable_name, variable_value);
}

void Perl__crosszonesetentityvariablebyclientname(const char* client_name, const char* variable_name, const char* variable_value)
{
	int update_identifier = 0;
	quest_manager.CrossZoneSetEntityVariable(CZUpdateType_ClientName, update_identifier, variable_name, variable_value, client_name);
}

void Perl__crosszonesetentityvariablebynpctypeid(int npc_id, const char* variable_name, const char* variable_value)
{
	quest_manager.CrossZoneSetEntityVariable(CZUpdateType_NPC, npc_id, variable_name, variable_value);
}

void Perl__crosszonesignalclientbycharid(int character_id, int signal_id)
{
	quest_manager.CrossZoneSignal(CZUpdateType_Character, character_id, signal_id);
}

void Perl__crosszonesignalclientbygroupid(int group_id, int signal_id)
{
	quest_manager.CrossZoneSignal(CZUpdateType_Group, group_id, signal_id);
}

void Perl__crosszonesignalclientbyraidid(int raid_id, int signal_id)
{
	quest_manager.CrossZoneSignal(CZUpdateType_Raid, raid_id, signal_id);
}

void Perl__crosszonesignalclientbyguildid(int guild_id, int signal_id)
{
	quest_manager.CrossZoneSignal(CZUpdateType_Guild, guild_id, signal_id);
}

void Perl__crosszonesignalclientbyexpeditionid(uint32 expedition_id, int signal_id)
{
	quest_manager.CrossZoneSignal(CZUpdateType_Expedition, expedition_id, signal_id);
}

void Perl__crosszonesignalclientbyname(const char* client_name, int signal_id)
{
	int update_identifier = 0;
	quest_manager.CrossZoneSignal(CZUpdateType_Expedition, update_identifier, signal_id, client_name);
}

void Perl__crosszonesignalnpcbynpctypeid(uint32 npc_id, int signal_id)
{
	quest_manager.CrossZoneSignal(CZUpdateType_NPC, npc_id, signal_id);
}

void Perl__crosszoneupdateactivitybycharid(int character_id, uint32 task_id, int activity_id)
{
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Character, CZTaskUpdateSubtype_ActivityUpdate, character_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void Perl__crosszoneupdateactivitybycharid(int character_id, uint32 task_id, int activity_id, int update_count)
{
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Character, CZTaskUpdateSubtype_ActivityUpdate, character_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void Perl__crosszoneupdateactivitybygroupid(int group_id, uint32 task_id, int activity_id)
{
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Group, CZTaskUpdateSubtype_ActivityUpdate, group_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void Perl__crosszoneupdateactivitybygroupid(int group_id, uint32 task_id, int activity_id, int update_count)
{
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Group, CZTaskUpdateSubtype_ActivityUpdate, group_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void Perl__crosszoneupdateactivitybyraidid(int raid_id, uint32 task_id, int activity_id)
{
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Raid, CZTaskUpdateSubtype_ActivityUpdate, raid_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void Perl__crosszoneupdateactivitybyraidid(int raid_id, uint32 task_id, int activity_id, int update_count)
{
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Raid, CZTaskUpdateSubtype_ActivityUpdate, raid_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void Perl__crosszoneupdateactivitybyguildid(int guild_id, uint32 task_id, int activity_id)
{
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Guild, CZTaskUpdateSubtype_ActivityUpdate, guild_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void Perl__crosszoneupdateactivitybyguildid(int guild_id, uint32 task_id, int activity_id, int update_count)
{
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Guild, CZTaskUpdateSubtype_ActivityUpdate, guild_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void Perl__crosszoneupdateactivitybyexpeditionid(uint32 expedition_id, uint32 task_id, int activity_id)
{
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Expedition, CZTaskUpdateSubtype_ActivityUpdate, expedition_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void Perl__crosszoneupdateactivitybyexpeditionid(uint32 expedition_id, uint32 task_id, int activity_id, int update_count)
{
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_Expedition, CZTaskUpdateSubtype_ActivityUpdate, expedition_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void Perl__crosszoneupdateactivitybyclientname(const char* client_name, uint32 task_id, int activity_id)
{
	int update_identifier = 0;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_ClientName, CZTaskUpdateSubtype_ActivityUpdate, update_identifier, task_id, activity_id, update_count, enforce_level_requirement, client_name);
}

void Perl__crosszoneupdateactivitybyclientname(const char* client_name, uint32 task_id, int activity_id, int update_count)
{
	int update_identifier = 0;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(CZUpdateType_ClientName, CZTaskUpdateSubtype_ActivityUpdate, update_identifier, task_id, activity_id, update_count, enforce_level_requirement, client_name);
}

void Perl__worldwideaddldonloss(uint32 theme_id)
{
	quest_manager.WorldWideLDoNUpdate(CZLDoNUpdateSubtype_AddLoss, theme_id);
}

void Perl__worldwideaddldonloss(uint32 theme_id, uint8 min_status)
{
	int points = 1;
	quest_manager.WorldWideLDoNUpdate(CZLDoNUpdateSubtype_AddLoss, theme_id, points, min_status);
}

void Perl__worldwideaddldonloss(uint32 theme_id, uint8 min_status, uint8 max_status)
{
	int points = 1;
	quest_manager.WorldWideLDoNUpdate(CZLDoNUpdateSubtype_AddLoss, theme_id, points, min_status, max_status);
}

void Perl__worldwideaddldonpoints(uint32 theme_id)
{
	int points = 1;
	quest_manager.WorldWideLDoNUpdate(CZLDoNUpdateSubtype_AddPoints, theme_id, points);
}

void Perl__worldwideaddldonpoints(uint32 theme_id, int points)
{
	quest_manager.WorldWideLDoNUpdate(CZLDoNUpdateSubtype_AddPoints, theme_id, points);
}

void Perl__worldwideaddldonpoints(uint32 theme_id, int points, uint8 min_status)
{
	quest_manager.WorldWideLDoNUpdate(CZLDoNUpdateSubtype_AddPoints, theme_id, points, min_status);
}

void Perl__worldwideaddldonpoints(uint32 theme_id, int points, uint8 min_status, uint8 max_status)
{
	quest_manager.WorldWideLDoNUpdate(CZLDoNUpdateSubtype_AddPoints, theme_id, points, min_status, max_status);
}

void Perl__worldwideaddldonwin(uint32 theme_id)
{
	int points = 1;
	quest_manager.WorldWideLDoNUpdate(CZLDoNUpdateSubtype_AddWin, theme_id, points);
}

void Perl__worldwideaddldonwin(uint32 theme_id, uint8 min_status)
{
	int points = 1;
	quest_manager.WorldWideLDoNUpdate(CZLDoNUpdateSubtype_AddWin, theme_id, points, min_status);
}

void Perl__worldwideaddldonwin(uint32 theme_id, uint8 min_status, uint8 max_status)
{
	int points = 1;
	quest_manager.WorldWideLDoNUpdate(CZLDoNUpdateSubtype_AddWin, theme_id, points, min_status, max_status);
}

void Perl__worldwideassigntask(uint32 task_id)
{
	quest_manager.WorldWideTaskUpdate(WWTaskUpdateType_AssignTask, task_id);
}

void Perl__worldwideassigntask(uint32 task_id, bool enforce_level_requirement)
{
	int task_subidentifier = -1;
	int update_count = 1;
	quest_manager.WorldWideTaskUpdate(WWTaskUpdateType_AssignTask, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void Perl__worldwideassigntask(uint32 task_id, bool enforce_level_requirement, uint8 min_status)
{
	int task_subidentifier = -1;
	int update_count = 1;
	quest_manager.WorldWideTaskUpdate(WWTaskUpdateType_AssignTask, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status);
}

void Perl__worldwideassigntask(uint32 task_id, bool enforce_level_requirement, uint8 min_status, uint8 max_status)
{
	int task_subidentifier = -1;
	int update_count = 1;
	quest_manager.WorldWideTaskUpdate(WWTaskUpdateType_AssignTask, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status, max_status);
}

void Perl__worldwidecastspell(uint32 spell_id)
{
	quest_manager.WorldWideSpell(WWSpellUpdateType_Cast, spell_id);
}

void Perl__worldwidecastspell(uint32 spell_id, uint8 min_status)
{
	quest_manager.WorldWideSpell(WWSpellUpdateType_Cast, spell_id, min_status);
}

void Perl__worldwidecastspell(uint32 spell_id, uint8 min_status, uint8 max_status)
{
	quest_manager.WorldWideSpell(WWSpellUpdateType_Cast, spell_id, min_status, max_status);
}

void Perl__worldwidedialoguewindow(const char* message)
{
	quest_manager.WorldWideDialogueWindow(message);
}

void Perl__worldwidedialoguewindow(const char* message, uint8 min_status)
{
	quest_manager.WorldWideDialogueWindow(message, min_status);
}

void Perl__worldwidedialoguewindow(const char* message, uint8 min_status, uint8 max_status)
{
	quest_manager.WorldWideDialogueWindow(message, min_status, max_status);
}

void Perl__worldwidedisabletask(uint32 task_id)
{
	quest_manager.WorldWideTaskUpdate(WWTaskUpdateType_DisableTask, task_id);
}

void Perl__worldwidedisabletask(uint32 task_id, uint8 min_status)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(WWTaskUpdateType_DisableTask, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status);
}

void Perl__worldwidedisabletask(uint32 task_id, uint8 min_status, uint8 max_status)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(WWTaskUpdateType_DisableTask, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status, max_status);
}

void Perl__worldwideenabletask(uint32 task_id)
{
	quest_manager.WorldWideTaskUpdate(WWTaskUpdateType_EnableTask, task_id);
}

void Perl__worldwideenabletask(uint32 task_id, uint8 min_status)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(WWTaskUpdateType_EnableTask, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status);
}

void Perl__worldwideenabletask(uint32 task_id, uint8 min_status, uint8 max_status)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(WWTaskUpdateType_EnableTask, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status, max_status);
}

void Perl__worldwidefailtask(uint32 task_id)
{
	quest_manager.WorldWideTaskUpdate(WWTaskUpdateType_FailTask, task_id);
}

void Perl__worldwidefailtask(uint32 task_id, uint8 min_status)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(WWTaskUpdateType_FailTask, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status);
}

void Perl__worldwidefailtask(uint32 task_id, uint8 min_status, uint8 max_status)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(WWTaskUpdateType_FailTask, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status, max_status);
}

void Perl__worldwidemarquee(uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message)
{
	quest_manager.WorldWideMarquee(type, priority, fade_in, fade_out, duration, message);
}

void Perl__worldwidemarquee(uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message, uint8 min_status)
{
	quest_manager.WorldWideMarquee(type, priority, fade_in, fade_out, duration, message, min_status);
}

void Perl__worldwidemarquee(uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message, uint8 min_status, uint8 max_status)
{
	quest_manager.WorldWideMarquee(type, priority, fade_in, fade_out, duration, message, min_status, max_status);
}

void Perl__worldwidemessage(uint32 type, const char* message)
{
	quest_manager.WorldWideMessage(type, message);
}

void Perl__worldwidemessage(uint32 type, const char* message, uint8 min_status)
{
	quest_manager.WorldWideMessage(type, message, min_status);
}

void Perl__worldwidemessage(uint32 type, const char* message, uint8 min_status, uint8 max_status)
{
	quest_manager.WorldWideMessage(type, message, min_status, max_status);
}

void Perl__worldwidemove(const char* zone_short_name)
{
	quest_manager.WorldWideMove(WWMoveUpdateType_MoveZone, zone_short_name);
}

void Perl__worldwidemove(const char* zone_short_name, uint8 min_status)
{
	uint16 instance_id = 0;
	quest_manager.WorldWideMove(WWMoveUpdateType_MoveZone, zone_short_name, instance_id, min_status);
}

void Perl__worldwidemove(const char* zone_short_name, uint8 min_status, uint8 max_status)
{
	uint16 instance_id = 0;
	quest_manager.WorldWideMove(WWMoveUpdateType_MoveZone, zone_short_name, instance_id, min_status, max_status);
}

void Perl__worldwidemoveinstance(uint16 instance_id)
{
	const char* zone_short_name = "";
	quest_manager.WorldWideMove(WWMoveUpdateType_MoveZoneInstance, zone_short_name, instance_id);
}

void Perl__worldwidemoveinstance(uint16 instance_id, uint8 min_status)
{
	const char* zone_short_name = "";
	quest_manager.WorldWideMove(WWMoveUpdateType_MoveZoneInstance, zone_short_name, instance_id, min_status);
}

void Perl__worldwidemoveinstance(uint16 instance_id, uint8 min_status, uint8 max_status)
{
	const char* zone_short_name = "";
	quest_manager.WorldWideMove(WWMoveUpdateType_MoveZoneInstance, zone_short_name, instance_id, min_status, max_status);
}

void Perl__worldwideremoveldonloss(uint32 theme_id)
{
	quest_manager.WorldWideLDoNUpdate(CZLDoNUpdateSubtype_RemoveLoss, theme_id);
}

void Perl__worldwideremoveldonloss(uint32 theme_id, uint8 min_status)
{
	int points = 1;
	quest_manager.WorldWideLDoNUpdate(CZLDoNUpdateSubtype_RemoveLoss, theme_id, points, min_status);
}

void Perl__worldwideremoveldonloss(uint32 theme_id, uint8 min_status, uint8 max_status)
{
	int points = 1;
	quest_manager.WorldWideLDoNUpdate(CZLDoNUpdateSubtype_RemoveLoss, theme_id, points, min_status, max_status);
}

void Perl__worldwideremoveldonwin(uint32 theme_id)
{
	quest_manager.WorldWideLDoNUpdate(CZLDoNUpdateSubtype_RemoveWin, theme_id);
}

void Perl__worldwideremoveldonwin(uint32 theme_id, uint8 min_status)
{
	int points = 1;
	quest_manager.WorldWideLDoNUpdate(CZLDoNUpdateSubtype_RemoveWin, theme_id, points, min_status);
}

void Perl__worldwideremoveldonwin(uint32 theme_id, uint8 min_status, uint8 max_status)
{
	int points = 1;
	quest_manager.WorldWideLDoNUpdate(CZLDoNUpdateSubtype_RemoveWin, theme_id, points, min_status, max_status);
}

void Perl__worldwideremovespell(uint32 spell_id)
{
	quest_manager.WorldWideSpell(WWSpellUpdateType_Remove, spell_id);
}

void Perl__worldwideremovespell(uint32 spell_id, uint8 min_status)
{
	quest_manager.WorldWideSpell(WWSpellUpdateType_Remove, spell_id, min_status);
}

void Perl__worldwideremovespell(uint32 spell_id, uint8 min_status, uint8 max_status)
{
	quest_manager.WorldWideSpell(WWSpellUpdateType_Remove, spell_id, min_status, max_status);
}

void Perl__worldwideremovetask(uint32 task_id)
{
	quest_manager.WorldWideTaskUpdate(WWTaskUpdateType_RemoveTask, task_id);
}

void Perl__worldwideremovetask(uint32 task_id, uint8 min_status)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(WWTaskUpdateType_RemoveTask, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status);
}

void Perl__worldwideremovetask(uint32 task_id, uint8 min_status, uint8 max_status)
{
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(WWTaskUpdateType_RemoveTask, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status, max_status);
}

void Perl__worldwideresetactivity(uint32 task_id, int activity_id)
{
	quest_manager.WorldWideTaskUpdate(WWTaskUpdateType_ActivityReset, task_id);
}

void Perl__worldwideresetactivity(uint32 task_id, int activity_id, uint8 min_status)
{
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(WWTaskUpdateType_ActivityReset, task_id, activity_id, update_count, enforce_level_requirement, min_status);
}

void Perl__worldwideresetactivity(uint32 task_id, int activity_id, uint8 min_status, uint8 max_status)
{
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(WWTaskUpdateType_ActivityReset, task_id, activity_id, update_count, enforce_level_requirement, min_status, max_status);
}

void Perl__worldwidesetentityvariableclient(const char* variable_name, const char* variable_value)
{
	quest_manager.WorldWideSetEntityVariable(WWSetEntityVariableUpdateType_Character, variable_name, variable_value);
}

void Perl__worldwidesetentityvariableclient(const char* variable_name, const char* variable_value, uint8 min_status)
{
	quest_manager.WorldWideSetEntityVariable(WWSetEntityVariableUpdateType_Character, variable_name, variable_value, min_status);
}

void Perl__worldwidesetentityvariableclient(const char* variable_name, const char* variable_value, uint8 min_status, uint8 max_status)
{
	quest_manager.WorldWideSetEntityVariable(WWSetEntityVariableUpdateType_Character, variable_name, variable_value, min_status, max_status);
}

void Perl__worldwidesetentityvariablenpc(const char* variable_name, const char* variable_value)
{
	quest_manager.WorldWideSetEntityVariable(WWSetEntityVariableUpdateType_NPC, variable_name, variable_value);
}

void Perl__worldwidesignalnpc(int signal_id)
{
	quest_manager.WorldWideSignal(WWSignalUpdateType_NPC, signal_id);
}

void Perl__worldwidesignalclient(int signal_id)
{
	quest_manager.WorldWideSignal(WWSignalUpdateType_Character, signal_id);
}

void Perl__worldwidesignalclient(int signal_id, uint8 min_status)
{
	quest_manager.WorldWideSignal(WWSignalUpdateType_Character, signal_id, min_status);
}

void Perl__worldwidesignalclient(int signal_id, uint8 min_status, uint8 max_status)
{
	quest_manager.WorldWideSignal(WWSignalUpdateType_Character, signal_id, min_status, max_status);
}

void Perl__worldwideupdateactivity(uint32 task_id, int activity_id)
{
	quest_manager.WorldWideTaskUpdate(WWTaskUpdateType_ActivityUpdate, task_id, activity_id);
}

void Perl__worldwideupdateactivity(uint32 task_id, int activity_id, int update_count)
{
	quest_manager.WorldWideTaskUpdate(WWTaskUpdateType_ActivityUpdate, task_id, activity_id, update_count);
}

void Perl__worldwideupdateactivity(uint32 task_id, int activity_id, int update_count, uint8 min_status)
{
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(WWTaskUpdateType_ActivityUpdate, task_id, activity_id, update_count, enforce_level_requirement, min_status);
}

void Perl__worldwideupdateactivity(uint32 task_id, int activity_id, int update_count, uint8 min_status, uint8 max_status)
{
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(WWTaskUpdateType_ActivityUpdate, task_id, activity_id, update_count, enforce_level_requirement, min_status, max_status);
}

bool Perl__isnpcspawned(perl::array npc_id_array)
{
	std::vector<uint32> npc_ids;
	for (int i = 0; i < npc_id_array.size(); ++i)
	{
		npc_ids.push_back(static_cast<uint32_t>(npc_id_array[i]));
	}
	return entity_list.IsNPCSpawned(npc_ids);
}

uint32_t Perl__countspawnednpcs(perl::array npc_id_array)
{
	std::vector<uint32> npc_ids;
	for (int i = 0; i < npc_id_array.size(); ++i)
	{
		npc_ids.push_back(static_cast<uint32_t>(npc_id_array[i]));
	}
	return entity_list.CountSpawnedNPCs(npc_ids);
}

SPDat_Spell_Struct* Perl__getspell(uint32 spell_id)
{
	// should be safe, it's read only in perl (could also use proxy lika lua)
	return const_cast<SPDat_Spell_Struct*>(quest_manager.getspell(spell_id));
}

std::string Perl__getldonthemename(uint32 theme_id)
{
	return quest_manager.getldonthemename(theme_id);
}

std::string Perl__getfactionname(int faction_id)
{
	return quest_manager.getfactionname(faction_id);
}

std::string Perl__getlanguagename(uint8 language_id)
{
	return quest_manager.getlanguagename(language_id);
}

std::string Perl__getbodytypename(uint8 body_type_id)
{
	return quest_manager.getbodytypename(body_type_id);
}

std::string Perl__getconsiderlevelname(uint8 consider_level)
{
	return quest_manager.getconsiderlevelname(consider_level);
}

std::string Perl__getenvironmentaldamagename(uint8 damage_type)
{
	return quest_manager.getenvironmentaldamagename(damage_type);
}

std::string Perl__commify(perl::scalar number)
{
	return Strings::Commify(number.c_str());
}

bool Perl__checknamefilter(std::string name)
{
	return database.CheckNameFilter(name);
}

void Perl__discordsend(std::string webhook_name, std::string message)
{
	zone->SendDiscordMessage(webhook_name, message);
}

void Perl__tracknpc(uint32 entity_id)
{
	quest_manager.TrackNPC(entity_id);
}

int Perl__getrecipemadecount(uint32 recipe_id)
{
	return quest_manager.GetRecipeMadeCount(recipe_id);
}

std::string Perl__getrecipename(uint32 recipe_id)
{
	return quest_manager.GetRecipeName(recipe_id);
}

bool Perl__hasrecipelearned(uint32 recipe_id)
{
	return quest_manager.HasRecipeLearned(recipe_id);
}

bool Perl__IsRaining()
{
	if (!zone) {
		return false;
	}

	return zone->IsRaining();
}

bool Perl__IsSnowing()
{
	if (!zone) {
		return false;
	}

	return zone->IsSnowing();
}

std::string Perl__getaaname(int aa_id)
{
	if (!zone) {
		return std::string();
	}

	return zone->GetAAName(aa_id);
}

std::string Perl__popupbreak() {
	return DialogueWindow::Break();
}

std::string Perl__popupbreak(uint32 break_count) {
	return DialogueWindow::Break(break_count);
}

std::string Perl__popupcentermessage(std::string message) {
	return DialogueWindow::CenterMessage(message);
}

std::string Perl__popupcolormessage(std::string color, std::string message) {
	return DialogueWindow::ColorMessage(color, message);
}

std::string Perl__popupindent() {
	return DialogueWindow::Indent();
}

std::string Perl__popupindent(uint32 indent_count) {
	return DialogueWindow::Indent(indent_count);
}

std::string Perl__popuplink(std::string link) {
	return DialogueWindow::Link(link);
}

std::string Perl__popuplink(std::string link, std::string message) {
	return DialogueWindow::Link(link, message);
}

std::string Perl__popuptable(std::string message) {
	return DialogueWindow::Table(message);
}

std::string Perl__popuptablecell() {
	return DialogueWindow::TableCell();
}

std::string Perl__popuptablecell(std::string message) {
	return DialogueWindow::TableCell(message);
}

std::string Perl__popuptablerow(std::string message) {
	return DialogueWindow::TableRow(message);
}

void Perl__marquee(uint32 type, std::string message)
{
	quest_manager.marquee(type, message);
}

void Perl__marquee(uint32 type, std::string message, uint32 duration)
{
	quest_manager.marquee(type, message, duration);
}

void Perl__marquee(uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, std::string message)
{
	quest_manager.marquee(type, priority, fade_in, fade_out, duration, message);
}

void Perl__zonemarquee(uint32 type, std::string message)
{
	if (!zone) {
		return;
	}

	entity_list.Marquee(type, message);
}

void Perl__zonemarquee(uint32 type, std::string message, uint32 duration)
{
	if (!zone) {
		return;
	}

	entity_list.Marquee(type, message, duration);
}

void Perl__zonemarquee(uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, std::string message)
{
	if (!zone) {
		return;
	}

	entity_list.Marquee(type, priority, fade_in, fade_out, duration, message);
}

bool Perl__ishotzone()
{
	if (!zone) {
		return false;
	}

	return zone->IsHotzone();
}

void Perl__sethotzone(bool is_hotzone)
{
	if (!zone) {
		return;
	}

	zone->SetIsHotzone(is_hotzone);
}

void Perl__set_proximity_range(float x_range, float y_range)
{
	quest_manager.set_proximity_range(x_range, y_range);
}

void Perl__set_proximity_range(float x_range, float y_range, float z_range)
{
	quest_manager.set_proximity_range(x_range, y_range, z_range);
}

void Perl__set_proximity_range(float x_range, float y_range, float z_range, bool enable_say)
{
	quest_manager.set_proximity_range(x_range, y_range, z_range, enable_say);
}

std::string Perl__varlink(EQ::ItemInstance* inst)
{
	return quest_manager.varlink(inst);
}

std::string Perl__varlink(uint32 item_id)
{
	return quest_manager.varlink(item_id);
}

std::string Perl__varlink(uint32 item_id, int16 charges)
{
	return quest_manager.varlink(item_id, charges);
}

std::string Perl__varlink(uint32 item_id, int16 charges, uint32 aug1)
{
	return quest_manager.varlink(item_id, charges, aug1);
}

std::string Perl__varlink(uint32 item_id, int16 charges, uint32 aug1, uint32 aug2)
{
	return quest_manager.varlink(item_id, charges, aug1, aug2);
}

std::string Perl__varlink(uint32 item_id, int16 charges, uint32 aug1, uint32 aug2, uint32 aug3)
{
	return quest_manager.varlink(item_id, charges, aug1, aug2, aug3);
}

std::string Perl__varlink(uint32 item_id, int16 charges, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4)
{
	return quest_manager.varlink(item_id, charges, aug1, aug2, aug3, aug4);
}

std::string Perl__varlink(uint32 item_id, int16 charges, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5)
{
	return quest_manager.varlink(item_id, charges, aug1, aug2, aug3, aug4, aug5);
}

std::string Perl__varlink(uint32 item_id, int16 charges, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5, uint32 aug6)
{
	return quest_manager.varlink(item_id, charges, aug1, aug2, aug3, aug4, aug5, aug6);
}

std::string Perl__varlink(uint32 item_id, int16 charges, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5, uint32 aug6, bool attuned)
{
	return quest_manager.varlink(item_id, charges, aug1, aug2, aug3, aug4, aug5, aug6, attuned);
}

bool Perl__do_augment_slots_match(uint32 item_one, uint32 item_two)
{
	return quest_manager.DoAugmentSlotsMatch(item_one, item_two);
}

int8 Perl__does_augment_fit(EQ::ItemInstance* inst, uint32 augment_id)
{
	return quest_manager.DoesAugmentFit(inst, augment_id);
}

int8 Perl__does_augment_fit_slot(EQ::ItemInstance* inst, uint32 augment_id, uint8 augment_slot)
{
	return quest_manager.DoesAugmentFit(inst, augment_id, augment_slot);
}

perl::array Perl__GetRecipeComponentItemIDs(uint32 recipe_id)
{
	perl::array result;

	const auto& l = content_db.GetRecipeComponentItemIDs(RecipeCountType::Component, recipe_id);

	if (!l.empty()) {
		result.reserve(l.size());

		for (int i = 0; i < l.size(); i++) {
			result.push_back(l[i]);
		}
	}

	return result;
}

perl::array Perl__GetRecipeContainerItemIDs(uint32 recipe_id)
{
	perl::array result;

	const auto& l = content_db.GetRecipeComponentItemIDs(RecipeCountType::Container, recipe_id);

	if (!l.empty()) {
		result.reserve(l.size());

		for (int i = 0; i < l.size(); i++) {
			result.push_back(l[i]);
		}
	}

	return result;
}

perl::array Perl__GetRecipeFailItemIDs(uint32 recipe_id)
{
	perl::array result;

	const auto &l = content_db.GetRecipeComponentItemIDs(RecipeCountType::Fail, recipe_id);

	if (!l.empty()) {
		result.reserve(l.size());

		for (int i = 0; i < l.size(); i++) {
			result.push_back(l[i]);
		}
	}

	return result;
}

perl::array Perl__GetRecipeSalvageItemIDs(uint32 recipe_id)
{
	perl::array result;

	const auto& l = content_db.GetRecipeComponentItemIDs(RecipeCountType::Salvage, recipe_id);

	if (!l.empty()) {
		result.reserve(l.size());

		for (int i = 0; i < l.size(); i++) {
			result.push_back(l[i]);
		}
	}

	return result;
}

perl::array Perl__GetRecipeSuccessItemIDs(uint32 recipe_id)
{
	perl::array result;

	const auto& l = content_db.GetRecipeComponentItemIDs(RecipeCountType::Success, recipe_id);

	if (!l.empty()) {
		result.reserve(l.size());

		for (int i = 0; i < l.size(); i++) {
			result.push_back(l[i]);
		}
	}

	return result;
}

int8 Perl__GetRecipeComponentCount(uint32 recipe_id, uint32 item_id)
{
	return content_db.GetRecipeComponentCount(RecipeCountType::Component, recipe_id, item_id);
}

int8 Perl__GetRecipeFailCount(uint32 recipe_id, uint32 item_id)
{
	return content_db.GetRecipeComponentCount(RecipeCountType::Fail, recipe_id, item_id);
}

int8 Perl__GetRecipeSalvageCount(uint32 recipe_id, uint32 item_id)
{
	return content_db.GetRecipeComponentCount(RecipeCountType::Salvage, recipe_id, item_id);
}

int8 Perl__GetRecipeSuccessCount(uint32 recipe_id, uint32 item_id)
{
	return content_db.GetRecipeComponentCount(RecipeCountType::Success, recipe_id, item_id);
}

void Perl__send_player_handin_event()
{
	quest_manager.SendPlayerHandinEvent();
}

float Perl__GetZoneSafeX(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneSafeCoordinates(zone_id).x;
}

float Perl__GetZoneSafeX(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneSafeCoordinates(zone_id, version).x;
}

float Perl__GetZoneSafeY(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneSafeCoordinates(zone_id).y;
}

float Perl__GetZoneSafeY(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneSafeCoordinates(zone_id, version).y;
}

float Perl__GetZoneSafeZ(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneSafeCoordinates(zone_id).z;
}

float Perl__GetZoneSafeZ(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneSafeCoordinates(zone_id, version).z;
}

float Perl__GetZoneSafeHeading(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneSafeCoordinates(zone_id).w;
}

float Perl__GetZoneSafeHeading(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneSafeCoordinates(zone_id, version).w;
}

float Perl__GetZoneGraveyardID(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneGraveyardID(zone_id);
}

float Perl__GetZoneGraveyardID(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneGraveyardID(zone_id, version);
}

uint8 Perl__GetZoneMinimumLevel(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneMinimumLevel(zone_id);
}

uint8 Perl__GetZoneMinimumLevel(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneMinimumLevel(zone_id, version);
}

uint8 Perl__GetZoneMaximumLevel(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneMaximumLevel(zone_id);
}

uint8 Perl__GetZoneMaximumLevel(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneMaximumLevel(zone_id, version);
}

uint8 Perl__GetZoneMinimumStatus(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneMinimumStatus(zone_id);
}

uint8 Perl__GetZoneMinimumStatus(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneMinimumStatus(zone_id, version);
}

int Perl__GetZoneTimeZone(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneTimeZone(zone_id);
}

int Perl__GetZoneTimeZone(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneTimeZone(zone_id, version);
}

int Perl__GetZoneMaximumPlayers(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneMaximumPlayers(zone_id);
}

int Perl__GetZoneMaximumPlayers(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneMaximumPlayers(zone_id, version);
}

uint32 Perl__GetZoneRuleSet(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneRuleSet(zone_id);
}

uint32 Perl__GetZoneRuleSet(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneRuleSet(zone_id, version);
}

std::string Perl__GetZoneNote(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneNote(zone_id);
}

std::string Perl__GetZoneNote(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneNote(zone_id, version);
}

float Perl__GetZoneUnderworld(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneUnderworld(zone_id);
}

float Perl__GetZoneUnderworld(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneUnderworld(zone_id, version);
}

float Perl__GetZoneMinimumClip(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneMinimumClip(zone_id);
}

float Perl__GetZoneMinimumClip(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneMinimumClip(zone_id, version);
}

float Perl__GetZoneMaximumClip(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneMaximumClip(zone_id);
}

float Perl__GetZoneMaximumClip(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneMaximumClip(zone_id, version);
}

float Perl__GetZoneFogMinimumClip(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneFogMinimumClip(zone_id);
}

float Perl__GetZoneFogMinimumClip(uint32 zone_id, uint8 slot)
{
	return ZoneStore::Instance()->GetZoneFogMinimumClip(zone_id, slot);
}

float Perl__GetZoneFogMinimumClip(uint32 zone_id, uint8 slot, int version)
{
	return ZoneStore::Instance()->GetZoneFogMinimumClip(zone_id, slot, version);
}

float Perl__GetZoneFogMaximumClip(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneFogMaximumClip(zone_id);
}

float Perl__GetZoneFogMaximumClip(uint32 zone_id, uint8 slot)
{
	return ZoneStore::Instance()->GetZoneFogMaximumClip(zone_id, slot);
}

float Perl__GetZoneFogMaximumClip(uint32 zone_id, uint8 slot, int version)
{
	return ZoneStore::Instance()->GetZoneFogMaximumClip(zone_id, slot, version);
}

uint8 Perl__GetZoneFogRed(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneFogRed(zone_id);
}

uint8 Perl__GetZoneFogRed(uint32 zone_id, uint8 slot)
{
	return ZoneStore::Instance()->GetZoneFogRed(zone_id, slot);
}

uint8 Perl__GetZoneFogRed(uint32 zone_id, uint8 slot, int version)
{
	return ZoneStore::Instance()->GetZoneFogRed(zone_id, slot, version);
}

uint8 Perl__GetZoneFogGreen(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneFogGreen(zone_id);
}

uint8 Perl__GetZoneFogGreen(uint32 zone_id, uint8 slot)
{
	return ZoneStore::Instance()->GetZoneFogGreen(zone_id, slot);
}

uint8 Perl__GetZoneFogGreen(uint32 zone_id, uint8 slot, int version)
{
	return ZoneStore::Instance()->GetZoneFogGreen(zone_id, slot, version);
}

uint8 Perl__GetZoneFogBlue(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneFogBlue(zone_id);
}

uint8 Perl__GetZoneFogBlue(uint32 zone_id, uint8 slot)
{
	return ZoneStore::Instance()->GetZoneFogBlue(zone_id, slot);
}

uint8 Perl__GetZoneFogBlue(uint32 zone_id, uint8 slot, int version)
{
	return ZoneStore::Instance()->GetZoneFogBlue(zone_id, slot, version);
}

uint8 Perl__GetZoneSky(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneSky(zone_id);
}

uint8 Perl__GetZoneSky(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneSky(zone_id, version);
}

uint8 Perl__GetZoneZType(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneZType(zone_id);
}

uint8 Perl__GetZoneZType(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneZType(zone_id, version);
}

float Perl__GetZoneExperienceMultiplier(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneExperienceMultiplier(zone_id);
}

float Perl__GetZoneExperienceMultiplier(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneExperienceMultiplier(zone_id, version);
}

float Perl__GetZoneWalkSpeed(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneWalkSpeed(zone_id);
}

float Perl__GetZoneWalkSpeed(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneWalkSpeed(zone_id, version);
}

uint8 Perl__GetZoneTimeType(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneTimeType(zone_id);
}

uint8 Perl__GetZoneTimeType(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneTimeType(zone_id, version);
}

float Perl__GetZoneFogDensity(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneFogDensity(zone_id);
}

float Perl__GetZoneFogDensity(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneFogDensity(zone_id, version);
}

std::string Perl__GetZoneFlagNeeded(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneFlagNeeded(zone_id);
}

std::string Perl__GetZoneFlagNeeded(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneFlagNeeded(zone_id, version);
}

int8 Perl__GetZoneCanBind(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneCanBind(zone_id);
}

int8 Perl__GetZoneCanBind(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneCanBind(zone_id, version);
}

int8 Perl__GetZoneCanCombat(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneCanCombat(zone_id);
}

int8 Perl__GetZoneCanCombat(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneCanCombat(zone_id, version);
}

int8 Perl__GetZoneCanLevitate(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneCanLevitate(zone_id);
}

int8 Perl__GetZoneCanLevitate(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneCanLevitate(zone_id, version);
}

int8 Perl__GetZoneCastOutdoor(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneCastOutdoor(zone_id);
}

int8 Perl__GetZoneCastOutdoor(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneCastOutdoor(zone_id, version);
}

uint8 Perl__GetZoneHotzone(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneHotzone(zone_id);
}

uint8 Perl__GetZoneHotzone(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneHotzone(zone_id, version);
}

uint8 Perl__GetZoneInstanceType(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneInstanceType(zone_id);
}

uint8 Perl__GetZoneInstanceType(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneInstanceType(zone_id, version);
}

uint64 Perl__GetZoneShutdownDelay(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneShutdownDelay(zone_id);
}

uint64 Perl__GetZoneShutdownDelay(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneShutdownDelay(zone_id, version);
}

int8 Perl__GetZonePEQZone(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZonePEQZone(zone_id);
}

int8 Perl__GetZonePEQZone(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZonePEQZone(zone_id, version);
}

int8 Perl__GetZoneExpansion(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneExpansion(zone_id);
}

int8 Perl__GetZoneExpansion(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneExpansion(zone_id, version);
}

int8 Perl__GetZoneBypassExpansionCheck(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneBypassExpansionCheck(zone_id);
}

int8 Perl__GetZoneBypassExpansionCheck(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneBypassExpansionCheck(zone_id, version);
}

uint8 Perl__GetZoneSuspendBuffs(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneSuspendBuffs(zone_id);
}

uint8 Perl__GetZoneSuspendBuffs(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneSuspendBuffs(zone_id, version);
}

int Perl__GetZoneRainChance(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneRainChance(zone_id);
}

int Perl__GetZoneRainChance(uint32 zone_id, uint8 slot)
{
	return ZoneStore::Instance()->GetZoneRainChance(zone_id, slot);
}

int Perl__GetZoneRainChance(uint32 zone_id, uint8 slot, int version)
{
	return ZoneStore::Instance()->GetZoneRainChance(zone_id, slot, version);
}

int Perl__GetZoneRainDuration(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneRainDuration(zone_id);
}

int Perl__GetZoneRainDuration(uint32 zone_id, uint8 slot)
{
	return ZoneStore::Instance()->GetZoneRainDuration(zone_id, slot);
}

int Perl__GetZoneRainDuration(uint32 zone_id, uint8 slot, int version)
{
	return ZoneStore::Instance()->GetZoneRainDuration(zone_id, slot, version);
}

int Perl__GetZoneSnowChance(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneSnowChance(zone_id);
}

int Perl__GetZoneSnowChance(uint32 zone_id, uint8 slot)
{
	return ZoneStore::Instance()->GetZoneSnowChance(zone_id, slot);
}

int Perl__GetZoneSnowChance(uint32 zone_id, uint8 slot, int version)
{
	return ZoneStore::Instance()->GetZoneSnowChance(zone_id, slot, version);
}

int Perl__GetZoneSnowDuration(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneSnowDuration(zone_id);
}

int Perl__GetZoneSnowDuration(uint32 zone_id, uint8 slot)
{
	return ZoneStore::Instance()->GetZoneSnowDuration(zone_id, slot);
}

int Perl__GetZoneSnowDuration(uint32 zone_id, uint8 slot, int version)
{
	return ZoneStore::Instance()->GetZoneSnowDuration(zone_id, slot, version);
}

float Perl__GetZoneGravity(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneGravity(zone_id);
}

float Perl__GetZoneGravity(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneGravity(zone_id, version);
}

int Perl__GetZoneType(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneType(zone_id);
}

int Perl__GetZoneType(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneType(zone_id, version);
}

int8 Perl__GetZoneSkyLock(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneSkyLock(zone_id);
}

int8 Perl__GetZoneSkyLock(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneSkyLock(zone_id, version);
}

int Perl__GetZoneFastRegenHP(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneFastRegenHP(zone_id);
}

int Perl__GetZoneFastRegenHP(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneFastRegenHP(zone_id, version);
}

int Perl__GetZoneFastRegenMana(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneFastRegenMana(zone_id);
}

int Perl__GetZoneFastRegenMana(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneFastRegenMana(zone_id, version);
}

int Perl__GetZoneFastRegenEndurance(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneFastRegenEndurance(zone_id);
}

int Perl__GetZoneFastRegenEndurance(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneFastRegenEndurance(zone_id, version);
}

int Perl__GetZoneNPCMaximumAggroDistance(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneNPCMaximumAggroDistance(zone_id);
}

int Perl__GetZoneNPCMaximumAggroDistance(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneNPCMaximumAggroDistance(zone_id, version);
}

int8 Perl__GetZoneMinimumExpansion(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneMinimumExpansion(zone_id);
}

int8 Perl__GetZoneMinimumExpansion(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneMinimumExpansion(zone_id, version);
}

int8 Perl__GetZoneMaximumExpansion(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneMaximumExpansion(zone_id);
}

int8 Perl__GetZoneMaximumExpansion(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneMaximumExpansion(zone_id, version);
}

std::string Perl__GetZoneContentFlags(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneContentFlags(zone_id);
}

std::string Perl__GetZoneContentFlags(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneContentFlags(zone_id, version);
}

std::string Perl__GetZoneContentFlagsDisabled(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneContentFlagsDisabled(zone_id);
}

std::string Perl__GetZoneContentFlagsDisabled(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneContentFlagsDisabled(zone_id, version);
}

int Perl__GetZoneUnderworldTeleportIndex(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneUnderworldTeleportIndex(zone_id);
}

int Perl__GetZoneUnderworldTeleportIndex(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneUnderworldTeleportIndex(zone_id, version);
}

int Perl__GetZoneLavaDamage(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneLavaDamage(zone_id);
}

int Perl__GetZoneLavaDamage(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneLavaDamage(zone_id, version);
}

int Perl__GetZoneMinimumLavaDamage(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneMinimumLavaDamage(zone_id);
}

int Perl__GetZoneMinimumLavaDamage(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneMinimumLavaDamage(zone_id, version);
}

uint8 Perl__GetZoneIdleWhenEmpty(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneIdleWhenEmpty(zone_id);
}

uint8 Perl__GetZoneIdleWhenEmpty(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneIdleWhenEmpty(zone_id, version);
}

uint32 Perl__GetZoneSecondsBeforeIdle(uint32 zone_id)
{
	return ZoneStore::Instance()->GetZoneSecondsBeforeIdle(zone_id);
}

uint32 Perl__GetZoneSecondsBeforeIdle(uint32 zone_id, int version)
{
	return ZoneStore::Instance()->GetZoneSecondsBeforeIdle(zone_id, version);
}

void Perl__send_channel_message(uint8 channel_number, uint32 guild_id, uint8 language_id, uint8 language_skill, const char* message)
{
	quest_manager.SendChannelMessage(channel_number, guild_id, language_id, language_skill, message);
}

void Perl__send_channel_message(Client* from, uint8 channel_number, uint32 guild_id, uint8 language_id, uint8 language_skill, const char* message)
{
	quest_manager.SendChannelMessage(from, channel_number, guild_id, language_id, language_skill, message);
}

void Perl__send_channel_message(Client* from, const char* to, uint8 channel_number, uint32 guild_id, uint8 language_id, uint8 language_skill, const char* message)
{
	quest_manager.SendChannelMessage(from, to, channel_number, guild_id, language_id, language_skill, message);
}

std::string Perl__convert_money_to_string(perl::hash table)
{
	uint64 platinum = table.exists("platinum") ? static_cast<uint64>(table["platinum"]) : 0;
	uint64 gold     = table.exists("gold") ? static_cast<uint64>(table["gold"]) : 0;
	uint64 silver   = table.exists("silver") ? static_cast<uint64>(table["silver"]) : 0;
	uint64 copper   = table.exists("copper") ? static_cast<uint64>(table["copper"]) : 0;

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

uint8 Perl__GetBotClassByID(uint32 bot_id)
{
	return database.botdb.GetBotClassByID(bot_id);
}

uint8 Perl__GetBotGenderByID(uint32 bot_id)
{
	return database.botdb.GetBotGenderByID(bot_id);
}

perl::array Perl__GetBotIDsByCharacterID(uint32 character_id)
{
	perl::array result;

	const auto bot_ids = database.botdb.GetBotIDsByCharacterID(character_id);

	for (int i = 0; i < bot_ids.size(); i++) {
		result.push_back(bot_ids[i]);
	}

	return result;
}

perl::array Perl__GetBotIDsByCharacterID(uint32 character_id, uint8 class_id)
{
	perl::array result;

	const auto bot_ids = database.botdb.GetBotIDsByCharacterID(character_id, class_id);

	for (int i = 0; i < bot_ids.size(); i++) {
		result.push_back(bot_ids[i]);
	}

	return result;
}

uint8 Perl__GetBotLevelByID(uint32 bot_id)
{
	return database.botdb.GetBotLevelByID(bot_id);
}

std::string Perl__GetBotNameByID(uint32 bot_id)
{
	return database.botdb.GetBotNameByID(bot_id);
}

uint16 Perl__GetBotRaceByID(uint32 bot_id)
{
	return database.botdb.GetBotRaceByID(bot_id);
}

std::string Perl__silent_saylink(std::string text)
{
	return Saylink::Silent(text);
}

std::string Perl__silent_saylink(std::string text, std::string link_name)
{
	return Saylink::Silent(text, link_name);
}

uint16 Perl__get_class_bitmask(uint8 class_id)
{
	return GetPlayerClassBit(class_id);
}

uint32 Perl__get_deity_bitmask(uint32 deity_id)
{
	return Deity::GetBitmask(deity_id);
}

uint16 Perl__get_race_bitmask(uint16 race_id)
{
	return GetPlayerRaceBit(race_id);
}

std::string Perl__GetAutoLoginCharacterNameByAccountID(uint32 account_id)
{
	return quest_manager.GetAutoLoginCharacterNameByAccountID(account_id);
}

bool Perl__SetAutoLoginCharacterNameByAccountID(uint32 account_id, std::string character_name)
{
	return quest_manager.SetAutoLoginCharacterNameByAccountID(account_id, character_name);
}

uint32 Perl__GetZoneIDByLongName(std::string zone_long_name)
{
	return ZoneStore::Instance()->GetZoneIDByLongName(zone_long_name);
}

std::string Perl__GetZoneShortNameByLongName(std::string zone_long_name)
{
	return ZoneStore::Instance()->GetZoneShortNameByLongName(zone_long_name);
}

bool Perl__send_parcel(perl::reference table_ref)
{
	perl::hash table = table_ref;
	if (
		(!table.exists("name") && !table.exists("character_id")) ||
		!table.exists("item_id") ||
		!table.exists("quantity")
	) {
		return false;
	}

	std::string name         = table.exists("name") ? table["name"] : std::string();
	uint32      character_id = table.exists("character_id") ? table["character_id"] : 0;

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

	const uint32 item_id         = table["item_id"];
	const int16  quantity        = table["quantity"];
	const uint32 augment_one     = table.exists("augment_one") ? table["augment_one"] : 0;
	const uint32 augment_two     = table.exists("augment_two") ? table["augment_two"] : 0;
	const uint32 augment_three   = table.exists("augment_three") ? table["augment_three"] : 0;
	const uint32 augment_four    = table.exists("augment_four") ? table["augment_four"] : 0;
	const uint32 augment_five    = table.exists("augment_five") ? table["augment_five"] : 0;
	const uint32 augment_six     = table.exists("augment_six") ? table["augment_six"] : 0;
	const std::string& from_name = table.exists("from_name") ? table["from_name"] : std::string();
	const std::string& note      = table.exists("note") ? table["note"] : std::string();

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

bool Perl__aretaskscompleted(perl::array task_ids)
{
	std::vector<int> v;

	for (const auto& e : task_ids) {
		v.emplace_back(static_cast<int>(e));
	}

	return quest_manager.aretaskscompleted(v);
}

void Perl__SpawnCircle(uint32 npc_id, float x, float y, float z, float heading, float radius, uint32 points)
{
	quest_manager.SpawnCircle(npc_id, glm::vec4(x, y, z, heading), radius, points);
}

void Perl__SpawnGrid(uint32 npc_id, float x, float y, float z, float heading, float spacing, uint32 spawn_count)
{
	quest_manager.SpawnGrid(npc_id, glm::vec4(x, y, z, heading), spacing, spawn_count);
}

bool Perl__handin(perl::reference handin_ref)
{
	perl::hash handin = handin_ref;

	std::map<std::string, uint32> handin_map;

	for (auto e: handin) {
		if (!e.first) {
			continue;
		}

		if (Strings::EqualFold(e.first, "0")) {
			continue;
		}

		const uint32 count = static_cast<uint32>(handin.at(e.first));
		handin_map[e.first] = count;
	}

	return quest_manager.handin(handin_map);
}

perl::array Perl__get_paused_timers(Mob* m)
{
	perl::array a;

	const auto& l = quest_manager.GetPausedTimers(m);

	if (!l.empty()) {
		a.reserve(l.size());

		for (const auto& v : l) {
			a.push_back(v);
		}
	}

	return a;
}

perl::array Perl__get_timers(Mob* m)
{
	perl::array a;

	const auto& l = quest_manager.GetTimers(m);

	if (!l.empty()) {
		a.reserve(l.size());

		for (const auto& v: l) {
			a.push_back(v);
		}
	}

	return a;
}

std::string Perl__get_pet_command_name(uint8 pet_command)
{
	return PetCommand::GetName(pet_command);
}

std::string Perl__get_pet_type_name(uint8 pet_type)
{
	return PetType::GetName(pet_type);
}

void perl_register_quest()
{
	perl::interpreter perl(PERL_GET_THX);

	auto package = perl.new_package("quest");

	package.add("botquest", &Perl__botquest);
	package.add("spawnbotcount", (int(*)())&Perl__spawnbotcount);
	package.add("spawnbotcount", (int(*)(uint8))&Perl__spawnbotcount);
	package.add("createbotcount", (int(*)())&Perl__createbotcount);
	package.add("createbotcount", (int(*)(uint8))&Perl__createbotcount);
	package.add("createBot", &Perl__createBot);
	package.add("CalculateCorruptionCounters", &Perl__CalculateCorruptionCounters);
	package.add("CalculateCounters", &Perl__CalculateCounters);
	package.add("CalculateCurseCounters", &Perl__CalculateCurseCounters);
	package.add("CalculateDiseaseCounters", &Perl__CalculateDiseaseCounters);
	package.add("CalculatePoisonCounters", &Perl__CalculatePoisonCounters);
	package.add("AssignGroupToInstance", &Perl__AssignGroupToInstance);
	package.add("AssignRaidToInstance", &Perl__AssignRaidToInstance);
	package.add("AssignToInstance", &Perl__AssignToInstance);
	package.add("AssignToInstanceByCharID", &Perl__AssignToInstanceByCharID);
	package.add("ChooseRandom", &Perl__ChooseRandom);
	package.add("CreateInstance", &Perl__CreateInstance);
	package.add("DestroyInstance", &Perl__DestroyInstance);
	package.add("UpdateInstanceTimer", &Perl__UpdateInstanceTimer);
	package.add("GetInstanceTimer", &Perl__GetInstanceTimer);
	package.add("GetInstanceTimerByID", &Perl__GetInstanceTimerByID);
	package.add("FlagInstanceByGroupLeader", &Perl__FlagInstanceByGroupLeader);
	package.add("FlagInstanceByRaidLeader", &Perl__FlagInstanceByRaidLeader);
	package.add("FlyMode", &Perl__FlyMode);
	package.add("GetAutoLoginCharacterNameByAccountID", &Perl__GetAutoLoginCharacterNameByAccountID);
	package.add("GetBotClassByID", &Perl__GetBotClassByID);
	package.add("GetBotGenderByID", &Perl__GetBotGenderByID);
	package.add("GetBotIDsByCharacterID", (perl::array(*)(uint32))&Perl__GetBotIDsByCharacterID);
	package.add("GetBotIDsByCharacterID", (perl::array(*)(uint32, uint8))&Perl__GetBotIDsByCharacterID);
	package.add("GetBotLevelByID", &Perl__GetBotLevelByID);
	package.add("GetBotNameByID", &Perl__GetBotNameByID);
	package.add("GetBotRaceByID", &Perl__GetBotRaceByID);
	package.add("GetCharactersInInstance", &Perl__GetCharactersInInstance);
	package.add("GetInstanceID", &Perl__GetInstanceID);
	package.add("GetInstanceIDByCharID", &Perl__GetInstanceIDByCharID);
	package.add("GetInstanceIDs", &Perl__GetInstanceIDs);
	package.add("GetInstanceIDsByCharID", &Perl__GetInstanceIDsByCharID);
	package.add("GetInstanceVersionByID", &Perl__GetInstanceVersionByID);
	package.add("GetInstanceZoneIDByID", &Perl__GetInstanceZoneIDByID);
	package.add("GetSpellPartialMagicRuneReduction", &Perl__GetSpellPartialMagicRuneReduction);
	package.add("GetSpellPartialMagicRuneAmount", &Perl__GetSpellPartialMagicRuneAmount);
	package.add("GetSpellPartialMeleeRuneReduction", &Perl__GetSpellPartialMeleeRuneReduction);
	package.add("GetSpellPartialMeleeRuneAmount", &Perl__GetSpellPartialMeleeRuneAmount);
	package.add("GetSpellProcLimitTimer", &Perl__GetSpellProcLimitTimer);
	package.add("GetSpellResurrectionSicknessCheck", &Perl__GetSpellResurrectionSicknessCheck);
	package.add("GetSpellEffectDescriptionNumber", &Perl__GetSpellEffectDescriptionNumber);
	package.add("GetSpellEffectIndex", &Perl__GetSpellEffectIndex);
	package.add("GetSpellFuriousBash", &Perl__GetSpellFuriousBash);
	package.add("GetSpellMinimumLevel", &Perl__GetSpellMinimumLevel);
	package.add("GetSpellNimbusEffect", &Perl__GetSpellNimbusEffect);
	package.add("GetSpellResistType", &Perl__GetSpellResistType);
	package.add("GetSpellTargetType", &Perl__GetSpellTargetType);
	package.add("GetSpellTriggerSpellID", &Perl__GetSpellTriggerSpellID);
	package.add("GetSpellViralMaximumSpreadTime", &Perl__GetSpellViralMaximumSpreadTime);
	package.add("GetSpellViralMinimumSpreadTime", &Perl__GetSpellViralMinimumSpreadTime);
	package.add("GetSpellViralSpreadRange", &Perl__GetSpellViralSpreadRange);
	package.add("GetTimeSeconds", &Perl__GetTimeSeconds);
	package.add("GetZoneBypassExpansionCheck", (int8(*)(uint32))&Perl__GetZoneBypassExpansionCheck);
	package.add("GetZoneBypassExpansionCheck", (int8(*)(uint32, int))&Perl__GetZoneBypassExpansionCheck);
	package.add("GetZoneCanBind", (int8(*)(uint32))&Perl__GetZoneCanBind);
	package.add("GetZoneCanBind", (int8(*)(uint32, int))&Perl__GetZoneCanBind);
	package.add("GetZoneCanCombat", (int8(*)(uint32))&Perl__GetZoneCanCombat);
	package.add("GetZoneCanCombat", (int8(*)(uint32, int))&Perl__GetZoneCanCombat);
	package.add("GetZoneCanLevitate", (int8(*)(uint32))&Perl__GetZoneCanLevitate);
	package.add("GetZoneCanLevitate", (int8(*)(uint32, int))&Perl__GetZoneCanLevitate);
	package.add("GetZoneCastOutdoor", (int8(*)(uint32))&Perl__GetZoneCastOutdoor);
	package.add("GetZoneCastOutdoor", (int8(*)(uint32, int))&Perl__GetZoneCastOutdoor);
	package.add("GetZoneContentFlags", (std::string(*)(uint32))&Perl__GetZoneContentFlags);
	package.add("GetZoneContentFlags", (std::string(*)(uint32, int))&Perl__GetZoneContentFlags);
	package.add("GetZoneContentFlagsDisabled", (std::string(*)(uint32))&Perl__GetZoneContentFlagsDisabled);
	package.add("GetZoneContentFlagsDisabled", (std::string(*)(uint32, int))&Perl__GetZoneContentFlagsDisabled);
	package.add("GetZoneGraveyardID", (float(*)(uint32))&Perl__GetZoneGraveyardID);
	package.add("GetZoneGraveyardID", (float(*)(uint32, int))&Perl__GetZoneGraveyardID);
	package.add("GetZoneHotzone", (uint8(*)(uint32))&Perl__GetZoneHotzone);
	package.add("GetZoneHotzone", (uint8(*)(uint32, int))&Perl__GetZoneHotzone);
	package.add("GetZoneIdleWhenEmpty", (uint8(*)(uint32))&Perl__GetZoneIdleWhenEmpty);
	package.add("GetZoneIdleWhenEmpty", (uint8(*)(uint32, int))&Perl__GetZoneIdleWhenEmpty);
	package.add("GetZoneInstanceType", (uint8(*)(uint32))&Perl__GetZoneInstanceType);
	package.add("GetZoneInstanceType", (uint8(*)(uint32, int))&Perl__GetZoneInstanceType);
	package.add("GetZoneID", &Perl__GetZoneID);
	package.add("GetZoneIDByLongName", (uint32(*)(std::string))&Perl__GetZoneIDByLongName);
	package.add("GetZoneExpansion", (int8(*)(uint32))&Perl__GetZoneExpansion);
	package.add("GetZoneExpansion", (int8(*)(uint32, int))&Perl__GetZoneExpansion);
	package.add("GetZoneExperienceMultiplier", (float(*)(uint32))&Perl__GetZoneExperienceMultiplier);
	package.add("GetZoneExperienceMultiplier", (float(*)(uint32, int))&Perl__GetZoneExperienceMultiplier);
	package.add("GetZoneFastRegenHP", (int(*)(uint32))&Perl__GetZoneFastRegenHP);
	package.add("GetZoneFastRegenHP", (int(*)(uint32, int))&Perl__GetZoneFastRegenHP);
	package.add("GetZoneFastRegenMana", (int(*)(uint32))&Perl__GetZoneFastRegenMana);
	package.add("GetZoneFastRegenMana", (int(*)(uint32, int))&Perl__GetZoneFastRegenMana);
	package.add("GetZoneFastRegenEndurance", (int(*)(uint32))&Perl__GetZoneFastRegenEndurance);
	package.add("GetZoneFastRegenEndurance", (int(*)(uint32, int))&Perl__GetZoneFastRegenEndurance);
	package.add("GetZoneFlagNeeded", (std::string(*)(uint32))&Perl__GetZoneFlagNeeded);
	package.add("GetZoneFlagNeeded", (std::string(*)(uint32, int))&Perl__GetZoneFlagNeeded);
	package.add("GetZoneFogMaximumClip", (float(*)(uint32))&Perl__GetZoneFogMaximumClip);
	package.add("GetZoneFogMaximumClip", (float(*)(uint32, uint8))&Perl__GetZoneFogMaximumClip);
	package.add("GetZoneFogMaximumClip", (float(*)(uint32, uint8, int))&Perl__GetZoneFogMaximumClip);
	package.add("GetZoneFogMinimumClip", (float(*)(uint32))&Perl__GetZoneFogMinimumClip);
	package.add("GetZoneFogMinimumClip", (float(*)(uint32, uint8))&Perl__GetZoneFogMinimumClip);
	package.add("GetZoneFogMinimumClip", (float(*)(uint32, uint8, int))&Perl__GetZoneFogMinimumClip);
	package.add("GetZoneFogBlue", (uint8(*)(uint32))&Perl__GetZoneFogBlue);
	package.add("GetZoneFogBlue", (uint8(*)(uint32, uint8))&Perl__GetZoneFogBlue);
	package.add("GetZoneFogBlue", (uint8(*)(uint32, uint8, int))&Perl__GetZoneFogBlue);
	package.add("GetZoneFogDensity", (float(*)(uint32))&Perl__GetZoneFogDensity);
	package.add("GetZoneFogDensity", (float(*)(uint32, int))&Perl__GetZoneFogDensity);
	package.add("GetZoneFogGreen", (uint8(*)(uint32))&Perl__GetZoneFogGreen);
	package.add("GetZoneFogGreen", (uint8(*)(uint32, uint8))&Perl__GetZoneFogGreen);
	package.add("GetZoneFogGreen", (uint8(*)(uint32, uint8, int))&Perl__GetZoneFogGreen);
	package.add("GetZoneFogRed", (uint8(*)(uint32))&Perl__GetZoneFogRed);
	package.add("GetZoneFogRed", (uint8(*)(uint32, uint8))&Perl__GetZoneFogRed);
	package.add("GetZoneFogRed", (uint8(*)(uint32, uint8, int))&Perl__GetZoneFogRed);
	package.add("GetZoneGravity", (float(*)(uint32))&Perl__GetZoneGravity);
	package.add("GetZoneGravity", (float(*)(uint32, int))&Perl__GetZoneGravity);
	package.add("GetZoneMaximumClip", (float(*)(uint32))&Perl__GetZoneMaximumClip);
	package.add("GetZoneMaximumClip", (float(*)(uint32, int))&Perl__GetZoneMaximumClip);
	package.add("GetZoneMaximumExpansion", (int8(*)(uint32))&Perl__GetZoneMaximumExpansion);
	package.add("GetZoneMaximumExpansion", (int8(*)(uint32, int))&Perl__GetZoneMaximumExpansion);
	package.add("GetZoneMaximumLevel", (uint8(*)(uint32))&Perl__GetZoneMaximumLevel);
	package.add("GetZoneMaximumLevel", (uint8(*)(uint32, int))&Perl__GetZoneMaximumLevel);
	package.add("GetZoneMaximumPlayers", (int(*)(uint32))&Perl__GetZoneMaximumPlayers);
	package.add("GetZoneMaximumPlayers", (int(*)(uint32, int))&Perl__GetZoneMaximumPlayers);
	package.add("GetZoneMinimumClip", (float(*)(uint32))&Perl__GetZoneMinimumClip);
	package.add("GetZoneMinimumClip", (float(*)(uint32, int))&Perl__GetZoneMinimumClip);
	package.add("GetZoneMinimumExpansion", (int8(*)(uint32))&Perl__GetZoneMinimumExpansion);
	package.add("GetZoneMinimumExpansion", (int8(*)(uint32, int))&Perl__GetZoneMinimumExpansion);
	package.add("GetZoneMinimumLavaDamage", (int(*)(uint32))&Perl__GetZoneMinimumLavaDamage);
	package.add("GetZoneMinimumLavaDamage", (int(*)(uint32, int))&Perl__GetZoneMinimumLavaDamage);
	package.add("GetZoneMinimumLevel", (uint8(*)(uint32))&Perl__GetZoneMinimumLevel);
	package.add("GetZoneMinimumLevel", (uint8(*)(uint32, int))&Perl__GetZoneMinimumLevel);
	package.add("GetZoneMinimumStatus", (uint8(*)(uint32))&Perl__GetZoneMinimumStatus);
	package.add("GetZoneMinimumStatus", (uint8(*)(uint32, int))&Perl__GetZoneMinimumStatus);
	package.add("GetZoneNote", (std::string(*)(uint32))&Perl__GetZoneNote);
	package.add("GetZoneNote", (std::string(*)(uint32, int))&Perl__GetZoneNote);
	package.add("GetZoneNPCMaximumAggroDistance", (int(*)(uint32))&Perl__GetZoneNPCMaximumAggroDistance);
	package.add("GetZoneNPCMaximumAggroDistance", (int(*)(uint32, int))&Perl__GetZoneNPCMaximumAggroDistance);
	package.add("GetZoneLavaDamage", (int(*)(uint32))&Perl__GetZoneLavaDamage);
	package.add("GetZoneLavaDamage", (int(*)(uint32, int))&Perl__GetZoneLavaDamage);
	package.add("GetZoneLongName", &Perl__GetZoneLongName);
	package.add("GetZoneLongNameByID", &Perl__GetZoneLongNameByID);
	package.add("GetZonePEQZone", (int8(*)(uint32))&Perl__GetZonePEQZone);
	package.add("GetZonePEQZone", (int8(*)(uint32, int))&Perl__GetZonePEQZone);
	package.add("GetZoneRainChance", (int(*)(uint32))&Perl__GetZoneRainChance);
	package.add("GetZoneRainChance", (int(*)(uint32, uint8))&Perl__GetZoneRainChance);
	package.add("GetZoneRainChance", (int(*)(uint32, uint8, int))&Perl__GetZoneRainChance);
	package.add("GetZoneRainDuration", (int(*)(uint32))&Perl__GetZoneRainDuration);
	package.add("GetZoneRainDuration", (int(*)(uint32, uint8))&Perl__GetZoneRainDuration);
	package.add("GetZoneRainDuration", (int(*)(uint32, uint8, int))&Perl__GetZoneRainDuration);
	package.add("GetZoneRuleSet", (uint32(*)(uint32))&Perl__GetZoneRuleSet);
	package.add("GetZoneRuleSet", (uint32(*)(uint32, int))&Perl__GetZoneRuleSet);
	package.add("GetZoneSafeHeading", (float(*)(uint32))&Perl__GetZoneSafeHeading);
	package.add("GetZoneSafeHeading", (float(*)(uint32, int))&Perl__GetZoneSafeHeading);
	package.add("GetZoneSafeX", (float(*)(uint32))&Perl__GetZoneSafeX);
	package.add("GetZoneSafeX", (float(*)(uint32, int))&Perl__GetZoneSafeX);
	package.add("GetZoneSafeY", (float(*)(uint32))&Perl__GetZoneSafeY);
	package.add("GetZoneSafeY", (float(*)(uint32, int))&Perl__GetZoneSafeY);
	package.add("GetZoneSafeZ", (float(*)(uint32))&Perl__GetZoneSafeZ);
	package.add("GetZoneSafeZ", (float(*)(uint32, int))&Perl__GetZoneSafeZ);
	package.add("GetZoneSecondsBeforeIdle", (uint32(*)(uint32))&Perl__GetZoneSecondsBeforeIdle);
	package.add("GetZoneSecondsBeforeIdle", (uint32(*)(uint32, int))&Perl__GetZoneSecondsBeforeIdle);
	package.add("GetZoneShortName", (std::string(*)(uint32))&Perl__GetZoneShortName);
	package.add("GetZoneShortNameByLongName", (std::string(*)(std::string))&Perl__GetZoneShortNameByLongName);
	package.add("GetZoneShutdownDelay", (uint64(*)(uint32))&Perl__GetZoneShutdownDelay);
	package.add("GetZoneShutdownDelay", (uint64(*)(uint32, int))&Perl__GetZoneShutdownDelay);
	package.add("GetZoneSky", (uint8(*)(uint32))&Perl__GetZoneSky);
	package.add("GetZoneSky", (uint8(*)(uint32, int))&Perl__GetZoneSky);
	package.add("GetZoneSkyLock", (int8(*)(uint32))&Perl__GetZoneSkyLock);
	package.add("GetZoneSkyLock", (int8(*)(uint32, int))&Perl__GetZoneSkyLock);
	package.add("GetZoneSnowChance", (int(*)(uint32))&Perl__GetZoneSnowChance);
	package.add("GetZoneSnowChance", (int(*)(uint32, uint8))&Perl__GetZoneSnowChance);
	package.add("GetZoneSnowChance", (int(*)(uint32, uint8, int))&Perl__GetZoneSnowChance);
	package.add("GetZoneSnowDuration", (int(*)(uint32))&Perl__GetZoneSnowDuration);
	package.add("GetZoneSnowDuration", (int(*)(uint32, uint8))&Perl__GetZoneSnowDuration);
	package.add("GetZoneSnowDuration", (int(*)(uint32, uint8, int))&Perl__GetZoneSnowDuration);
	package.add("GetZoneSuspendBuffs", (uint8(*)(uint32))&Perl__GetZoneSuspendBuffs);
	package.add("GetZoneSuspendBuffs", (uint8(*)(uint32, int))&Perl__GetZoneSuspendBuffs);
	package.add("GetZoneZType", (uint8(*)(uint32))&Perl__GetZoneZType);
	package.add("GetZoneZType", (uint8(*)(uint32, int))&Perl__GetZoneZType);
	package.add("GetZoneTimeType", (uint8(*)(uint32))&Perl__GetZoneTimeType);
	package.add("GetZoneTimeType", (uint8(*)(uint32, int))&Perl__GetZoneTimeType);
	package.add("GetZoneTimeZone", (int(*)(uint32))&Perl__GetZoneTimeZone);
	package.add("GetZoneTimeZone", (int(*)(uint32, int))&Perl__GetZoneTimeZone);
	package.add("GetZoneType", (int(*)(uint32))&Perl__GetZoneType);
	package.add("GetZoneType", (int(*)(uint32, int))&Perl__GetZoneType);
	package.add("GetZoneUnderworld", (float(*)(uint32))&Perl__GetZoneUnderworld);
	package.add("GetZoneUnderworld", (float(*)(uint32, int))&Perl__GetZoneUnderworld);
	package.add("GetZoneUnderworldTeleportIndex", (int(*)(uint32))&Perl__GetZoneUnderworldTeleportIndex);
	package.add("GetZoneUnderworldTeleportIndex", (int(*)(uint32, int))&Perl__GetZoneUnderworldTeleportIndex);
	package.add("GetZoneWalkSpeed", (float(*)(uint32))&Perl__GetZoneWalkSpeed);
	package.add("GetZoneWalkSpeed", (float(*)(uint32, int))&Perl__GetZoneWalkSpeed);
	package.add("IsAEDurationSpell", &Perl__IsAEDurationSpell);
	package.add("IsAENukeSpell", &Perl__IsAENukeSpell);
	package.add("IsAERainNukeSpell", &Perl__IsAERainNukeSpell);
	package.add("IsAllianceSpell", &Perl__IsAllianceSpell);
	package.add("IsBardOnlyStackEffect", &Perl__IsBardOnlyStackEffect);
	package.add("IsBardSong", &Perl__IsBardSong);
	package.add("IsBeneficialSpell", &Perl__IsBeneficialSpell);
	package.add("IsBlankSpellEffect", &Perl__IsBlankSpellEffect);
	package.add("IsBlindSpell", &Perl__IsBlindSpell);
	package.add("IsBuffSpell", &Perl__IsBuffSpell);
	package.add("IsCastOnFadeDurationSpell", &Perl__IsCastOnFadeDurationSpell);
	package.add("IsCastNotStandingSpell", &Perl__IsCastNotStandingSpell);
	package.add("IsCastRestrictedSpell", &Perl__IsCastRestrictedSpell);
	package.add("IsCastTimeReductionSpell", &Perl__IsCastTimeReductionSpell);
	package.add("IsCastWhileInvisibleSpell", &Perl__IsCastWhileInvisibleSpell);
	package.add("IsCharmSpell", &Perl__IsCharmSpell);
	package.add("IsCombatSkill", &Perl__IsCombatSkill);
	package.add("IsCompleteHealSpell", &Perl__IsCompleteHealSpell);
	package.add("IsCompleteHealDurationSpell", &Perl__IsCompleteHealDurationSpell);
	package.add("IsCureSpell", &Perl__IsCureSpell);
	package.add("IsDamageSpell", &Perl__IsDamageSpell);
	package.add("IsDeathSaveSpell", &Perl__IsDeathSaveSpell);
	package.add("IsDebuffSpell", &Perl__IsDebuffSpell);
	package.add("IsDetrimentalSpell", &Perl__IsDetrimentalSpell);
	package.add("IsDiscipline", &Perl__IsDiscipline);
	package.add("IsDisciplineBuff", &Perl__IsDisciplineBuff);
	package.add("IsDiseaseCounterSpell", &Perl__IsDiseaseCounterSpell);
	package.add("IsDistanceModifierSpell", &Perl__IsDistanceModifierSpell);
	package.add("IsEffectIgnoredInStacking", &Perl__IsEffectIgnoredInStacking);
	package.add("IsEffectInSpell", &Perl__IsEffectInSpell);
	package.add("IsFastHealSpell", &Perl__IsFastHealSpell);
	package.add("IsFearSpell", &Perl__IsFearSpell);
	package.add("IsFocusLimit", &Perl__IsFocusLimit);
	package.add("IsFullDeathSaveSpell", &Perl__IsFullDeathSaveSpell);
	package.add("IsGateSpell", &Perl__IsGateSpell);
	package.add("IsGroupCompleteHealSpell", &Perl__IsGroupCompleteHealSpell);
	package.add("IsGroupHealOverTimeSpell", &Perl__IsGroupHealOverTimeSpell);
	package.add("IsGroupOnlySpell", &Perl__IsGroupOnlySpell);
	package.add("IsGroupSpell", &Perl__IsGroupSpell);
	package.add("IsHarmonySpell", &Perl__IsHarmonySpell);
	package.add("IsHasteSpell", &Perl__IsHasteSpell);
	package.add("IsHealOverTimeSpell", &Perl__IsHealOverTimeSpell);
	package.add("IsHealthSpell", &Perl__IsHealthSpell);
	package.add("IsIllusionSpell", &Perl__IsIllusionSpell);
	package.add("IsImprovedDamageSpell", &Perl__IsImprovedDamageSpell);
	package.add("IsImprovedHealingSpell", &Perl__IsImprovedHealingSpell);
	package.add("IsIncreaseDurationSpell", &Perl__IsIncreaseDurationSpell);
	package.add("IsIncreaseRangeSpell", &Perl__IsIncreaseRangeSpell);
	package.add("IsInstrumentModifierAppliedToSpellEffect", &Perl__IsInstrumentModifierAppliedToSpellEffect);
	package.add("IsInvisibleSpell", &Perl__IsInvisibleSpell);
	package.add("IsInvulnerabilitySpell", &Perl__IsInvulnerabilitySpell);
	package.add("IsLDoNObjectSpell", &Perl__IsLDoNObjectSpell);
	package.add("IsLifetapSpell", &Perl__IsLifetapSpell);
	package.add("IsMagicRuneSpell", &Perl__IsMagicRuneSpell);
	package.add("IsManaCostReductionSpell", &Perl__IsManaCostReductionSpell);
	package.add("IsManaTapSpell", &Perl__IsManaTapSpell);
	package.add("IsMesmerizeSpell", &Perl__IsMesmerizeSpell);
	package.add("IsNoDetrimentalSpellAggroSpell", &Perl__IsNoDetrimentalSpellAggroSpell);
	package.add("IsPartialDeathSaveSpell", &Perl__IsPartialDeathSaveSpell);
	package.add("IsPartialResistableSpell", &Perl__IsPartialResistableSpell);
	package.add("IsPBAENukeSpell", &Perl__IsPBAENukeSpell);
	package.add("IsPercentalHealSpell", &Perl__IsPercentalHealSpell);
	package.add("IsPersistDeathSpell", &Perl__IsPersistDeathSpell);
	package.add("IsPetSpell", &Perl__IsPetSpell);
	package.add("IsPoisonCounterSpell", &Perl__IsPoisonCounterSpell);
	package.add("IsPulsingBardSong", &Perl__IsPulsingBardSong);
	package.add("IsPureNukeSpell", &Perl__IsPureNukeSpell);
	package.add("IsRaining", &Perl__IsRaining);
	package.add("IsRegularGroupHealSpell", &Perl__IsRegularGroupHealSpell);
	package.add("IsRegularSingleTargetHealSpell", &Perl__IsRegularSingleTargetHealSpell);
	package.add("IsResistableSpell", &Perl__IsResistableSpell);
	package.add("IsResistDebuffSpell", &Perl__IsResistDebuffSpell);
	package.add("IsRestAllowedSpell", &Perl__IsRestAllowedSpell);
	package.add("IsResurrectionEffects", &Perl__IsResurrectionEffects);
	package.add("IsRuneSpell", &Perl__IsRuneSpell);
	package.add("IsRunning", &Perl__IsRunning);
	package.add("IsSacrificeSpell", &Perl__IsSacrificeSpell);
	package.add("IsSelfConversionSpell", &Perl__IsSelfConversionSpell);
	package.add("IsShadowStepSpell", &Perl__IsShadowStepSpell);
	package.add("IsShortDurationBuff", &Perl__IsShortDurationBuff);
	package.add("IsSnowing", &Perl__IsSnowing);
	package.add("IsSpellUsableInThisZoneType", (bool(*)(uint16))&Perl__IsSpellUsableInThisZoneType);
	package.add("IsSpellUsableInThisZoneType", (bool(*)(uint16, uint8))&Perl__IsSpellUsableInThisZoneType);
	package.add("IsStackableDOT", &Perl__IsStackableDOT);
	package.add("IsStunSpell", &Perl__IsStunSpell);
	package.add("IsSuccorSpell", &Perl__IsSuccorSpell);
	package.add("IsSummonItemSpell", &Perl__IsSummonItemSpell);
	package.add("IsSummonPCSpell", &Perl__IsSummonPCSpell);
	package.add("IsSummonPetSpell", &Perl__IsSummonPetSpell);
	package.add("IsSummonSkeletonSpell", &Perl__IsSummonSkeletonSpell);
	package.add("IsSummonSpell", &Perl__IsSummonSpell);
	package.add("IsSuspendableSpell", &Perl__IsSuspendableSpell);
	package.add("IsTargetableAESpell", &Perl__IsTargetableAESpell);
	package.add("IsTargetRequiredForSpell", &Perl__IsTargetRequiredForSpell);
	package.add("IsTeleportSpell", &Perl__IsTeleportSpell);
	package.add("IsTranslocateSpell", &Perl__IsTranslocateSpell);
	package.add("IsTGBCompatibleSpell", &Perl__IsTGBCompatibleSpell);
	package.add("IsVeryFastHealSpell", &Perl__IsVeryFastHealSpell);
	package.add("IsVirusSpell", &Perl__IsVirusSpell);
	package.add("IsValidSpell", &Perl__IsValidSpell);
	package.add("LearnRecipe", &Perl__LearnRecipe);
	package.add("MerchantCountItem", &Perl__MerchantCountItem);
	package.add("MerchantSetItem", (void(*)(uint32, uint32))&Perl__MerchantSetItem);
	package.add("MerchantSetItem", (void(*)(uint32, uint32, uint32))&Perl__MerchantSetItem);
	package.add("MovePCInstance", (void(*)(int, int, float, float, float))&Perl__MovePCInstance);
	package.add("MovePCInstance", (void(*)(int, int, float, float, float, float))&Perl__MovePCInstance);
	package.add("RemoveAllFromInstance", &Perl__RemoveAllFromInstance);
	package.add("RemoveFromInstance", &Perl__RemoveFromInstance);
	package.add("RemoveFromInstanceByCharID", &Perl__RemoveFromInstanceByCharID);
	package.add("CheckInstanceByCharID", &Perl__CheckInstanceByCharID);
	package.add("SendMail", &Perl__SendMail);
	package.add("SetAutoLoginCharacterNameByAccountID", &Perl__SetAutoLoginCharacterNameByAccountID);
	package.add("SetRunning", &Perl__SetRunning);
	package.add("SpawnCircle", &Perl__SpawnCircle);
	package.add("SpawnGrid", &Perl__SpawnGrid);
	package.add("activespeakactivity", &Perl__activespeakactivity);
	package.add("activespeaktask", &Perl__activespeaktask);
	package.add("activetasksinset", &Perl__activetasksinset);
	package.add("add_expedition_lockout_all_clients", (void(*)(std::string, std::string, uint32))&Perl__add_expedition_lockout_all_clients);
	package.add("add_expedition_lockout_all_clients", (void(*)(std::string, std::string, uint32, std::string))&Perl__add_expedition_lockout_all_clients);
	package.add("add_expedition_lockout_by_char_id", (void(*)(uint32, std::string, std::string, uint32))&Perl__add_expedition_lockout_by_char_id);
	package.add("add_expedition_lockout_by_char_id", (void(*)(uint32, std::string, std::string, uint32, std::string))&Perl__add_expedition_lockout_by_char_id);
	package.add("addldonloss", &Perl__addldonloss);
	package.add("addldonpoints", &Perl__addldonpoints);
	package.add("addldonwin", &Perl__addldonwin);
	package.add("addloot", (void(*)(int))&Perl__addloot);
	package.add("addloot", (void(*)(int, int))&Perl__addloot);
	package.add("addloot", (void(*)(int, int, bool))&Perl__addloot);
	package.add("addskill", &Perl__addskill);
	package.add("aretaskscompleted", &Perl__aretaskscompleted);
	package.add("assigntask", (void(*)(int))&Perl__assigntask);
	package.add("assigntask", (void(*)(int, bool))&Perl__assigntask);
	package.add("attack", &Perl__attack);
	package.add("attacknpc", &Perl__attacknpc);
	package.add("attacknpctype", &Perl__attacknpctype);
	package.add("buryplayercorpse", &Perl__buryplayercorpse);
	package.add("castspell", &Perl__castspell);
	package.add("changedeity", &Perl__changedeity);
	package.add("checknamefilter", &Perl__checknamefilter),
	package.add("checktitle", &Perl__checktitle);
	package.add("clear_npctype_cache", &Perl__clear_npctype_cache);
	package.add("clear_proximity", &Perl__clear_proximity);
	package.add("clear_zone_flag", &Perl__clear_zone_flag);
	package.add("clearspawntimers", &Perl__clearspawntimers);
	package.add("collectitems", &Perl__collectitems);
	package.add("commify", &Perl__commify);
	package.add("completedtasksinset", &Perl__completedtasksinset);
	package.add("convert_money_to_string", &Perl__convert_money_to_string);
	package.add("countitem", &Perl__countitem);
	package.add("countspawnednpcs", &Perl__countspawnednpcs);
	package.add("createdoor", (uint16(*)(const char*, float, float, float, float))&Perl__CreateDoor);
	package.add("createdoor", (uint16(*)(const char*, float, float, float, float, uint8_t))&Perl__CreateDoor);
	package.add("createdoor", (uint16(*)(const char*, float, float, float, float, uint8_t, uint16_t))&Perl__CreateDoor);
	package.add("creategroundobject", (uint16(*)(uint32_t, float, float, float, float))&Perl__CreateGroundObject);
	package.add("creategroundobject", (uint16(*)(uint32_t, float, float, float, float, uint32_t))&Perl__CreateGroundObject);
	package.add("creategroundobjectfrommodel", (uint16(*)(const char*, float, float, float, float))&Perl__CreateGroundObjectFromModel);
	package.add("creategroundobjectfrommodel", (uint16(*)(const char*, float, float, float, float, uint8_t))&Perl__CreateGroundObjectFromModel);
	package.add("creategroundobjectfrommodel", (uint16(*)(const char*, float, float, float, float, uint8_t, uint32_t))&Perl__CreateGroundObjectFromModel);
	package.add("createguild", &Perl__createguild);
	package.add("createitem", (EQ::ItemInstance*(*)(uint32))&Perl__createitem);
	package.add("createitem", (EQ::ItemInstance*(*)(uint32, int16))&Perl__createitem);
	package.add("createitem", (EQ::ItemInstance*(*)(uint32, int16, uint32))&Perl__createitem);
	package.add("createitem", (EQ::ItemInstance*(*)(uint32, int16, uint32, uint32))&Perl__createitem);
	package.add("createitem", (EQ::ItemInstance*(*)(uint32, int16, uint32, uint32, uint32))&Perl__createitem);
	package.add("createitem", (EQ::ItemInstance*(*)(uint32, int16, uint32, uint32, uint32, uint32))&Perl__createitem);
	package.add("createitem", (EQ::ItemInstance*(*)(uint32, int16, uint32, uint32, uint32, uint32, uint32))&Perl__createitem);
	package.add("createitem", (EQ::ItemInstance*(*)(uint32, int16, uint32, uint32, uint32, uint32, uint32, uint32))&Perl__createitem);
	package.add("createitem", (EQ::ItemInstance*(*)(uint32, int16, uint32, uint32, uint32, uint32, uint32, uint32, bool))&Perl__createitem);
	package.add("crosszoneaddldonlossbycharid", &Perl__crosszoneaddldonlossbycharid);
	package.add("crosszoneaddldonlossbygroupid", &Perl__crosszoneaddldonlossbygroupid);
	package.add("crosszoneaddldonlossbyraidid", &Perl__crosszoneaddldonlossbyraidid);
	package.add("crosszoneaddldonlossbyguildid", &Perl__crosszoneaddldonlossbyguildid);
	package.add("crosszoneaddldonlossbyexpeditionid", &Perl__crosszoneaddldonlossbyexpeditionid);
	package.add("crosszoneaddldonlossbyclientname", &Perl__crosszoneaddldonlossbyclientname);
	package.add("crosszoneaddldonpointsbycharid", &Perl__crosszoneaddldonpointsbycharid);
	package.add("crosszoneaddldonpointsbygroupid", &Perl__crosszoneaddldonpointsbygroupid);
	package.add("crosszoneaddldonpointsbyraidid", &Perl__crosszoneaddldonpointsbyraidid);
	package.add("crosszoneaddldonpointsbyguildid", &Perl__crosszoneaddldonpointsbyguildid);
	package.add("crosszoneaddldonpointsbyexpeditionid", &Perl__crosszoneaddldonpointsbyexpeditionid);
	package.add("crosszoneaddldonpointsbyclientname", &Perl__crosszoneaddldonpointsbyclientname);
	package.add("crosszoneaddldonwinbycharid", &Perl__crosszoneaddldonwinbycharid);
	package.add("crosszoneaddldonwinbygroupid", &Perl__crosszoneaddldonwinbygroupid);
	package.add("crosszoneaddldonwinbyraidid", &Perl__crosszoneaddldonwinbyraidid);
	package.add("crosszoneaddldonwinbyguildid", &Perl__crosszoneaddldonwinbyguildid);
	package.add("crosszoneaddldonwinbyexpeditionid", &Perl__crosszoneaddldonwinbyexpeditionid);
	package.add("crosszoneaddldonwinbyclientname", &Perl__crosszoneaddldonwinbyclientname);
	package.add("crosszoneassigntaskbycharid", (void(*)(int, uint32))&Perl__crosszoneassigntaskbycharid);
	package.add("crosszoneassigntaskbycharid", (void(*)(int, uint32, bool))&Perl__crosszoneassigntaskbycharid);
	package.add("crosszoneassigntaskbygroupid", (void(*)(int, uint32))&Perl__crosszoneassigntaskbygroupid);
	package.add("crosszoneassigntaskbygroupid", (void(*)(int, uint32, bool))&Perl__crosszoneassigntaskbygroupid);
	package.add("crosszoneassigntaskbyraidid", (void(*)(int, uint32))&Perl__crosszoneassigntaskbyraidid);
	package.add("crosszoneassigntaskbyraidid", (void(*)(int, uint32, bool))&Perl__crosszoneassigntaskbyraidid);
	package.add("crosszoneassigntaskbyguildid", (void(*)(int, uint32))&Perl__crosszoneassigntaskbyguildid);
	package.add("crosszoneassigntaskbyguildid", (void(*)(int, uint32, bool))&Perl__crosszoneassigntaskbyguildid);
	package.add("crosszoneassigntaskbyexpeditionid", (void(*)(uint32, uint32))&Perl__crosszoneassigntaskbyexpeditionid);
	package.add("crosszoneassigntaskbyexpeditionid", (void(*)(uint32, uint32, bool))&Perl__crosszoneassigntaskbyexpeditionid);
	package.add("crosszoneassigntaskbyclientname", (void(*)(const char*, uint32))&Perl__crosszoneassigntaskbyclientname);
	package.add("crosszoneassigntaskbyclientname", (void(*)(const char*, uint32, bool))&Perl__crosszoneassigntaskbyclientname);
	package.add("crosszonecastspellbycharid", &Perl__crosszonecastspellbycharid);
	package.add("crosszonecastspellbygroupid", &Perl__crosszonecastspellbygroupid);
	package.add("crosszonecastspellbyraidid", &Perl__crosszonecastspellbyraidid);
	package.add("crosszonecastspellbyguildid", &Perl__crosszonecastspellbyguildid);
	package.add("crosszonecastspellbyexpeditionid", &Perl__crosszonecastspellbyexpeditionid);
	package.add("crosszonecastspellbyclientname", &Perl__crosszonecastspellbyclientname);
	package.add("crosszonedialoguewindowbycharid", &Perl__crosszonedialoguewindowbycharid);
	package.add("crosszonedialoguewindowbygroupid", &Perl__crosszonedialoguewindowbygroupid);
	package.add("crosszonedialoguewindowbyraidid", &Perl__crosszonedialoguewindowbyraidid);
	package.add("crosszonedialoguewindowbyguildid", &Perl__crosszonedialoguewindowbyguildid);
	package.add("crosszonedialoguewindowbyexpeditionid", &Perl__crosszonedialoguewindowbyexpeditionid);
	package.add("crosszonedialoguewindowbyclientname", &Perl__crosszonedialoguewindowbyclientname);
	package.add("crosszonedisabletaskbycharid", &Perl__crosszonedisabletaskbycharid);
	package.add("crosszonedisabletaskbygroupid", &Perl__crosszonedisabletaskbygroupid);
	package.add("crosszonedisabletaskbyraidid", &Perl__crosszonedisabletaskbyraidid);
	package.add("crosszonedisabletaskbyguildid", &Perl__crosszonedisabletaskbyguildid);
	package.add("crosszonedisabletaskbyexpeditionid", &Perl__crosszonedisabletaskbyexpeditionid);
	package.add("crosszonedisabletaskbyclientname", &Perl__crosszonedisabletaskbyclientname);
	package.add("crosszoneenabletaskbycharid", &Perl__crosszoneenabletaskbycharid);
	package.add("crosszoneenabletaskbygroupid", &Perl__crosszoneenabletaskbygroupid);
	package.add("crosszoneenabletaskbyraidid", &Perl__crosszoneenabletaskbyraidid);
	package.add("crosszoneenabletaskbyguildid", &Perl__crosszoneenabletaskbyguildid);
	package.add("crosszoneenabletaskbyexpeditionid", &Perl__crosszoneenabletaskbyexpeditionid);
	package.add("crosszoneenabletaskbyclientname", &Perl__crosszoneenabletaskbyclientname);
	package.add("crosszonefailtaskbycharid", &Perl__crosszonefailtaskbycharid);
	package.add("crosszonefailtaskbygroupid", &Perl__crosszonefailtaskbygroupid);
	package.add("crosszonefailtaskbyraidid", &Perl__crosszonefailtaskbyraidid);
	package.add("crosszonefailtaskbyguildid", &Perl__crosszonefailtaskbyguildid);
	package.add("crosszonefailtaskbyexpeditionid", &Perl__crosszonefailtaskbyexpeditionid);
	package.add("crosszonefailtaskbyclientname", &Perl__crosszonefailtaskbyclientname);
	package.add("crosszonemarqueebycharid", &Perl__crosszonemarqueebycharid);
	package.add("crosszonemarqueebygroupid", &Perl__crosszonemarqueebygroupid);
	package.add("crosszonemarqueebyraidid", &Perl__crosszonemarqueebyraidid);
	package.add("crosszonemarqueebyguildid", &Perl__crosszonemarqueebyguildid);
	package.add("crosszonemarqueebyexpeditionid", &Perl__crosszonemarqueebyexpeditionid);
	package.add("crosszonemarqueebyclientname", &Perl__crosszonemarqueebyclientname);
	package.add("crosszonemessageplayerbycharid", &Perl__crosszonemessageplayerbycharid);
	package.add("crosszonemessageplayerbygroupid", &Perl__crosszonemessageplayerbygroupid);
	package.add("crosszonemessageplayerbyraidid", &Perl__crosszonemessageplayerbyraidid);
	package.add("crosszonemessageplayerbyguildid", &Perl__crosszonemessageplayerbyguildid);
	package.add("crosszonemessageplayerbyexpeditionid", &Perl__crosszonemessageplayerbyexpeditionid);
	package.add("crosszonemessageplayerbyname", &Perl__crosszonemessageplayerbyname);
	package.add("crosszonemoveplayerbycharid", (void(*)(uint32, std::string))&Perl__crosszonemoveplayerbycharid);
	package.add("crosszonemoveplayerbycharid", (void(*)(uint32, std::string, float, float, float))&Perl__crosszonemoveplayerbycharid);
	package.add("crosszonemoveplayerbycharid", (void(*)(uint32, std::string, float, float, float, float))&Perl__crosszonemoveplayerbycharid);
	package.add("crosszonemoveplayerbygroupid", (void(*)(uint32, std::string))&Perl__crosszonemoveplayerbygroupid);
	package.add("crosszonemoveplayerbygroupid", (void(*)(uint32, std::string, float, float, float))&Perl__crosszonemoveplayerbygroupid);
	package.add("crosszonemoveplayerbygroupid", (void(*)(uint32, std::string, float, float, float, float))&Perl__crosszonemoveplayerbygroupid);
	package.add("crosszonemoveplayerbyraidid", (void(*)(uint32, std::string))&Perl__crosszonemoveplayerbyraidid);
	package.add("crosszonemoveplayerbyraidid", (void(*)(uint32, std::string, float, float, float))&Perl__crosszonemoveplayerbyraidid);
	package.add("crosszonemoveplayerbyraidid", (void(*)(uint32, std::string, float, float, float, float))&Perl__crosszonemoveplayerbyraidid);
	package.add("crosszonemoveplayerbyguildid", (void(*)(uint32, std::string))&Perl__crosszonemoveplayerbyguildid);
	package.add("crosszonemoveplayerbyguildid", (void(*)(uint32, std::string, float, float, float))&Perl__crosszonemoveplayerbyguildid);
	package.add("crosszonemoveplayerbyguildid", (void(*)(uint32, std::string, float, float, float, float))&Perl__crosszonemoveplayerbyguildid);
	package.add("crosszonemoveplayerbyexpeditionid", (void(*)(uint32, std::string))&Perl__crosszonemoveplayerbyexpeditionid);
	package.add("crosszonemoveplayerbyexpeditionid", (void(*)(uint32, std::string, float, float, float))&Perl__crosszonemoveplayerbyexpeditionid);
	package.add("crosszonemoveplayerbyexpeditionid", (void(*)(uint32, std::string, float, float, float, float))&Perl__crosszonemoveplayerbyexpeditionid);
	package.add("crosszonemoveplayerbyname", (void(*)(std::string, std::string))&Perl__crosszonemoveplayerbyname);
	package.add("crosszonemoveplayerbyname", (void(*)(std::string, std::string, float, float, float))&Perl__crosszonemoveplayerbyname);
	package.add("crosszonemoveplayerbyname", (void(*)(std::string, std::string, float, float, float, float))&Perl__crosszonemoveplayerbyname);
	package.add("crosszonemoveinstancebycharid", (void(*)(uint32, uint16))&Perl__crosszonemoveinstancebycharid);
	package.add("crosszonemoveinstancebycharid", (void(*)(uint32, uint16, float, float, float))&Perl__crosszonemoveinstancebycharid);
	package.add("crosszonemoveinstancebycharid", (void(*)(uint32, uint16, float, float, float, float))&Perl__crosszonemoveinstancebycharid);
	package.add("crosszonemoveinstancebygroupid", (void(*)(uint32, uint16))&Perl__crosszonemoveinstancebygroupid);
	package.add("crosszonemoveinstancebygroupid", (void(*)(uint32, uint16, float, float, float))&Perl__crosszonemoveinstancebygroupid);
	package.add("crosszonemoveinstancebygroupid", (void(*)(uint32, uint16, float, float, float, float))&Perl__crosszonemoveinstancebygroupid);
	package.add("crosszonemoveinstancebyraidid", (void(*)(uint32, uint16))&Perl__crosszonemoveinstancebyraidid);
	package.add("crosszonemoveinstancebyraidid", (void(*)(uint32, uint16, float, float, float))&Perl__crosszonemoveinstancebyraidid);
	package.add("crosszonemoveinstancebyraidid", (void(*)(uint32, uint16, float, float, float, float))&Perl__crosszonemoveinstancebyraidid);
	package.add("crosszonemoveinstancebyguildid", (void(*)(uint32, uint16))&Perl__crosszonemoveinstancebyguildid);
	package.add("crosszonemoveinstancebyguildid", (void(*)(uint32, uint16, float, float, float))&Perl__crosszonemoveinstancebyguildid);
	package.add("crosszonemoveinstancebyguildid", (void(*)(uint32, uint16, float, float, float, float))&Perl__crosszonemoveinstancebyguildid);
	package.add("crosszonemoveinstancebyexpeditionid", (void(*)(uint32, uint16))&Perl__crosszonemoveinstancebyexpeditionid);
	package.add("crosszonemoveinstancebyexpeditionid", (void(*)(uint32, uint16, float, float, float))&Perl__crosszonemoveinstancebyexpeditionid);
	package.add("crosszonemoveinstancebyexpeditionid", (void(*)(uint32, uint16, float, float, float, float))&Perl__crosszonemoveinstancebyexpeditionid);
	package.add("crosszonemoveinstancebyclientname", (void(*)(std::string, uint16))&Perl__crosszonemoveinstancebyclientname);
	package.add("crosszonemoveinstancebyclientname", (void(*)(std::string, uint16, float, float, float))&Perl__crosszonemoveinstancebyclientname);
	package.add("crosszonemoveinstancebyclientname", (void(*)(std::string, uint16, float, float, float, float))&Perl__crosszonemoveinstancebyclientname);
	package.add("crosszoneremoveldonlossbycharid", &Perl__crosszoneremoveldonlossbycharid);
	package.add("crosszoneremoveldonlossbygroupid", &Perl__crosszoneremoveldonlossbygroupid);
	package.add("crosszoneremoveldonlossbyraidid", &Perl__crosszoneremoveldonlossbyraidid);
	package.add("crosszoneremoveldonlossbyguildid", &Perl__crosszoneremoveldonlossbyguildid);
	package.add("crosszoneremoveldonlossbyexpeditionid", &Perl__crosszoneremoveldonlossbyexpeditionid);
	package.add("crosszoneremoveldonlossbyclientname", &Perl__crosszoneremoveldonlossbyclientname);
	package.add("crosszoneremoveldonwinbycharid", &Perl__crosszoneremoveldonwinbycharid);
	package.add("crosszoneremoveldonwinbygroupid", &Perl__crosszoneremoveldonwinbygroupid);
	package.add("crosszoneremoveldonwinbyraidid", &Perl__crosszoneremoveldonwinbyraidid);
	package.add("crosszoneremoveldonwinbyguildid", &Perl__crosszoneremoveldonwinbyguildid);
	package.add("crosszoneremoveldonwinbyexpeditionid", &Perl__crosszoneremoveldonwinbyexpeditionid);
	package.add("crosszoneremoveldonwinbyclientname", &Perl__crosszoneremoveldonwinbyclientname);
	package.add("crosszoneremovespellbycharid", &Perl__crosszoneremovespellbycharid);
	package.add("crosszoneremovespellbygroupid", &Perl__crosszoneremovespellbygroupid);
	package.add("crosszoneremovespellbyraidid", &Perl__crosszoneremovespellbyraidid);
	package.add("crosszoneremovespellbyguildid", &Perl__crosszoneremovespellbyguildid);
	package.add("crosszoneremovespellbyexpeditionid", &Perl__crosszoneremovespellbyexpeditionid);
	package.add("crosszoneremovespellbyclientname", &Perl__crosszoneremovespellbyclientname);
	package.add("crosszoneremovetaskbycharid", &Perl__crosszoneremovetaskbycharid);
	package.add("crosszoneremovetaskbygroupid", &Perl__crosszoneremovetaskbygroupid);
	package.add("crosszoneremovetaskbyraidid", &Perl__crosszoneremovetaskbyraidid);
	package.add("crosszoneremovetaskbyguildid", &Perl__crosszoneremovetaskbyguildid);
	package.add("crosszoneremovetaskbyexpeditionid", &Perl__crosszoneremovetaskbyexpeditionid);
	package.add("crosszoneremovetaskbyclientname", &Perl__crosszoneremovetaskbyclientname);
	package.add("crosszoneresetactivitybycharid", &Perl__crosszoneresetactivitybycharid);
	package.add("crosszoneresetactivitybygroupid", &Perl__crosszoneresetactivitybygroupid);
	package.add("crosszoneresetactivitybyraidid", &Perl__crosszoneresetactivitybyraidid);
	package.add("crosszoneresetactivitybyguildid", &Perl__crosszoneresetactivitybyguildid);
	package.add("crosszoneresetactivitybyexpeditionid", &Perl__crosszoneresetactivitybyexpeditionid);
	package.add("crosszoneresetactivitybyclientname", &Perl__crosszoneresetactivitybyclientname);
	package.add("crosszonesetentityvariablebycharid", &Perl__crosszonesetentityvariablebycharid);
	package.add("crosszonesetentityvariablebygroupid", &Perl__crosszonesetentityvariablebygroupid);
	package.add("crosszonesetentityvariablebyraidid", &Perl__crosszonesetentityvariablebyraidid);
	package.add("crosszonesetentityvariablebyguildid", &Perl__crosszonesetentityvariablebyguildid);
	package.add("crosszonesetentityvariablebyexpeditionid", &Perl__crosszonesetentityvariablebyexpeditionid);
	package.add("crosszonesetentityvariablebyclientname", &Perl__crosszonesetentityvariablebyclientname);
	package.add("crosszonesetentityvariablebynpctypeid", &Perl__crosszonesetentityvariablebynpctypeid);
	package.add("crosszonesignalclientbycharid", &Perl__crosszonesignalclientbycharid);
	package.add("crosszonesignalclientbygroupid", &Perl__crosszonesignalclientbygroupid);
	package.add("crosszonesignalclientbyraidid", &Perl__crosszonesignalclientbyraidid);
	package.add("crosszonesignalclientbyguildid", &Perl__crosszonesignalclientbyguildid);
	package.add("crosszonesignalclientbyexpeditionid", &Perl__crosszonesignalclientbyexpeditionid);
	package.add("crosszonesignalclientbyname", &Perl__crosszonesignalclientbyname);
	package.add("crosszonesignalnpcbynpctypeid", &Perl__crosszonesignalnpcbynpctypeid);
	package.add("crosszoneupdateactivitybycharid", (void(*)(int, uint32, int))&Perl__crosszoneupdateactivitybycharid);
	package.add("crosszoneupdateactivitybycharid", (void(*)(int, uint32, int, int))&Perl__crosszoneupdateactivitybycharid);
	package.add("crosszoneupdateactivitybygroupid", (void(*)(int, uint32, int))&Perl__crosszoneupdateactivitybygroupid);
	package.add("crosszoneupdateactivitybygroupid", (void(*)(int, uint32, int, int))&Perl__crosszoneupdateactivitybygroupid);
	package.add("crosszoneupdateactivitybyraidid", (void(*)(int, uint32, int))&Perl__crosszoneupdateactivitybyraidid);
	package.add("crosszoneupdateactivitybyraidid", (void(*)(int, uint32, int, int))&Perl__crosszoneupdateactivitybyraidid);
	package.add("crosszoneupdateactivitybyguildid", (void(*)(int, uint32, int))&Perl__crosszoneupdateactivitybyguildid);
	package.add("crosszoneupdateactivitybyguildid", (void(*)(int, uint32, int, int))&Perl__crosszoneupdateactivitybyguildid);
	package.add("crosszoneupdateactivitybyexpeditionid", (void(*)(uint32, uint32, int))&Perl__crosszoneupdateactivitybyexpeditionid);
	package.add("crosszoneupdateactivitybyexpeditionid", (void(*)(uint32, uint32, int, int))&Perl__crosszoneupdateactivitybyexpeditionid);
	package.add("crosszoneupdateactivitybyclientname", (void(*)(const char*, uint32, int))&Perl__crosszoneupdateactivitybyclientname);
	package.add("crosszoneupdateactivitybyclientname", (void(*)(const char*, uint32, int, int))&Perl__crosszoneupdateactivitybyclientname);
	package.add("worldwideaddldonloss", (void(*)(uint32))&Perl__worldwideaddldonloss);
	package.add("worldwideaddldonloss", (void(*)(uint32, uint8))&Perl__worldwideaddldonloss);
	package.add("worldwideaddldonloss", (void(*)(uint32, uint8, uint8))&Perl__worldwideaddldonloss);
	package.add("worldwideaddldonpoints", (void(*)(uint32))&Perl__worldwideaddldonpoints);
	package.add("worldwideaddldonpoints", (void(*)(uint32, int))&Perl__worldwideaddldonpoints);
	package.add("worldwideaddldonpoints", (void(*)(uint32, int, uint8))&Perl__worldwideaddldonpoints);
	package.add("worldwideaddldonpoints", (void(*)(uint32, int, uint8, uint8))&Perl__worldwideaddldonpoints);
	package.add("worldwideaddldonwin", (void(*)(uint32))&Perl__worldwideaddldonwin);
	package.add("worldwideaddldonwin", (void(*)(uint32, uint8))&Perl__worldwideaddldonwin);
	package.add("worldwideaddldonwin", (void(*)(uint32, uint8, uint8))&Perl__worldwideaddldonwin);
	package.add("worldwideassigntask", (void(*)(uint32))&Perl__worldwideassigntask);
	package.add("worldwideassigntask", (void(*)(uint32, bool))&Perl__worldwideassigntask);
	package.add("worldwideassigntask", (void(*)(uint32, bool, uint8))&Perl__worldwideassigntask);
	package.add("worldwideassigntask", (void(*)(uint32, bool, uint8, uint8))&Perl__worldwideassigntask);
	package.add("worldwidecastspell", (void(*)(uint32))&Perl__worldwidecastspell);
	package.add("worldwidecastspell", (void(*)(uint32, uint8))&Perl__worldwidecastspell);
	package.add("worldwidecastspell", (void(*)(uint32, uint8, uint8 max_status))&Perl__worldwidecastspell);
	package.add("worldwidedialoguewindow", (void(*)(const char*))&Perl__worldwidedialoguewindow);
	package.add("worldwidedialoguewindow", (void(*)(const char*, uint8))&Perl__worldwidedialoguewindow);
	package.add("worldwidedialoguewindow", (void(*)(const char*, uint8, uint8))&Perl__worldwidedialoguewindow);
	package.add("worldwidedisabletask", (void(*)(uint32))&Perl__worldwidedisabletask);
	package.add("worldwidedisabletask", (void(*)(uint32, uint8))&Perl__worldwidedisabletask);
	package.add("worldwidedisabletask", (void(*)(uint32, uint8, uint8))&Perl__worldwidedisabletask);
	package.add("worldwideenabletask", (void(*)(uint32))&Perl__worldwideenabletask);
	package.add("worldwideenabletask", (void(*)(uint32, uint8))&Perl__worldwideenabletask);
	package.add("worldwideenabletask", (void(*)(uint32, uint8, uint8))&Perl__worldwideenabletask);
	package.add("worldwidefailtask", (void(*)(uint32))&Perl__worldwidefailtask);
	package.add("worldwidefailtask", (void(*)(uint32, uint8))&Perl__worldwidefailtask);
	package.add("worldwidefailtask", (void(*)(uint32, uint8, uint8))&Perl__worldwidefailtask);
	package.add("worldwidemarquee", (void(*)(uint32, uint32, uint32, uint32, uint32, const char*))&Perl__worldwidemarquee);
	package.add("worldwidemarquee", (void(*)(uint32, uint32, uint32, uint32, uint32, const char*, uint8))&Perl__worldwidemarquee);
	package.add("worldwidemarquee", (void(*)(uint32, uint32, uint32, uint32, uint32, const char*, uint8, uint8))&Perl__worldwidemarquee);
	package.add("worldwidemessage", (void(*)(uint32, const char*))&Perl__worldwidemessage);
	package.add("worldwidemessage", (void(*)(uint32, const char*, uint8))&Perl__worldwidemessage);
	package.add("worldwidemessage", (void(*)(uint32, const char*, uint8, uint8))&Perl__worldwidemessage);
	package.add("worldwidemove", (void(*)(const char*))&Perl__worldwidemove);
	package.add("worldwidemove", (void(*)(const char*, uint8))&Perl__worldwidemove);
	package.add("worldwidemove", (void(*)(const char*, uint8, uint8))&Perl__worldwidemove);
	package.add("worldwidemoveinstance", (void(*)(uint16))&Perl__worldwidemoveinstance);
	package.add("worldwidemoveinstance", (void(*)(uint16, uint8))&Perl__worldwidemoveinstance);
	package.add("worldwidemoveinstance", (void(*)(uint16, uint8, uint8))&Perl__worldwidemoveinstance);
	package.add("worldwideremoveldonloss", (void(*)(uint32))&Perl__worldwideremoveldonloss);
	package.add("worldwideremoveldonloss", (void(*)(uint32, uint8))&Perl__worldwideremoveldonloss);
	package.add("worldwideremoveldonloss", (void(*)(uint32, uint8, uint8))&Perl__worldwideremoveldonloss);
	package.add("worldwideremoveldonwin", (void(*)(uint32))&Perl__worldwideremoveldonwin);
	package.add("worldwideremoveldonwin", (void(*)(uint32, uint8))&Perl__worldwideremoveldonwin);
	package.add("worldwideremoveldonwin", (void(*)(uint32, uint8, uint8))&Perl__worldwideremoveldonwin);
	package.add("worldwideremovespell", (void(*)(uint32))&Perl__worldwideremovespell);
	package.add("worldwideremovespell", (void(*)(uint32, uint8))&Perl__worldwideremovespell);
	package.add("worldwideremovespell", (void(*)(uint32, uint8, uint8))&Perl__worldwideremovespell);
	package.add("worldwideremovetask", (void(*)(uint32))&Perl__worldwideremovetask);
	package.add("worldwideremovetask", (void(*)(uint32, uint8))&Perl__worldwideremovetask);
	package.add("worldwideremovetask", (void(*)(uint32, uint8, uint8))&Perl__worldwideremovetask);
	package.add("worldwideresetactivity", (void(*)(uint32, int))&Perl__worldwideresetactivity);
	package.add("worldwideresetactivity", (void(*)(uint32, int, uint8))&Perl__worldwideresetactivity);
	package.add("worldwideresetactivity", (void(*)(uint32, int, uint8, uint8))&Perl__worldwideresetactivity);
	package.add("worldwidesetentityvariableclient", (void(*)(const char*, const char*))&Perl__worldwidesetentityvariableclient);
	package.add("worldwidesetentityvariableclient", (void(*)(const char*, const char*, uint8))&Perl__worldwidesetentityvariableclient);
	package.add("worldwidesetentityvariableclient", (void(*)(const char*, const char*, uint8, uint8))&Perl__worldwidesetentityvariableclient);
	package.add("worldwidesetentityvariablenpc", &Perl__worldwidesetentityvariablenpc);
	package.add("worldwidesignalclient", (void(*)(int))&Perl__worldwidesignalclient);
	package.add("worldwidesignalclient", (void(*)(int, uint8))&Perl__worldwidesignalclient);
	package.add("worldwidesignalclient", (void(*)(int, uint8, uint8))&Perl__worldwidesignalclient);
	package.add("worldwidesignalnpc", &Perl__worldwidesignalnpc);
	package.add("worldwideupdateactivity", (void(*)(uint32, int))&Perl__worldwideupdateactivity);
	package.add("worldwideupdateactivity", (void(*)(uint32, int, int))&Perl__worldwideupdateactivity);
	package.add("worldwideupdateactivity", (void(*)(uint32, int, int, uint8))&Perl__worldwideupdateactivity);
	package.add("worldwideupdateactivity", (void(*)(uint32, int, int, uint8, uint8))&Perl__worldwideupdateactivity);
	package.add("debug", (void(*)(const char*))&Perl__debug);
	package.add("debug", (void(*)(const char*, int))&Perl__debug);
	package.add("delete_data", &Perl__delete_data);
	package.add("delglobal", &Perl__delglobal);
	package.add("depop", (void(*)())&Perl__depop);
	package.add("depop", (void(*)(int))&Perl__depop);
	package.add("depop_withtimer", (void(*)())&Perl__depop_withtimer);
	package.add("depop_withtimer", (void(*)(int))&Perl__depop_withtimer);
	package.add("depopall", (void(*)())&Perl__depopall);
	package.add("depopall", (void(*)(int))&Perl__depopall);
	package.add("depopzone", &Perl__depopzone);
	package.add("ding", &Perl__ding);
	package.add("disable_proximity_say", &Perl__disable_proximity_say);
	package.add("disable_spawn2", &Perl__disable_spawn2);
	package.add("disablerecipe", &Perl__disablerecipe);
	package.add("disabletask", &Perl__disabletask);
	package.add("discordsend", &Perl__discordsend);
	package.add("doanim", (void(*)(int))&Perl__doanim);
	package.add("doanim", (void(*)(int, int))&Perl__doanim);
	package.add("doanim", (void(*)(int, int, bool))&Perl__doanim);
	package.add("doanim", (void(*)(int, int, bool, int))&Perl__doanim);
	package.add("do_augment_slots_match", &Perl__do_augment_slots_match);
	package.add("does_augment_fit", (int8(*)(EQ::ItemInstance*, uint32))&Perl__does_augment_fit);
	package.add("does_augment_fit_slot", (int8(*)(EQ::ItemInstance*, uint32, uint8))&Perl__does_augment_fit_slot);
	package.add("echo", &Perl__echo);
	package.add("emote", &Perl__emote);
	package.add("enable_proximity_say", &Perl__enable_proximity_say);
	package.add("enable_spawn2", &Perl__enable_spawn2);
	package.add("enabledtaskcount", &Perl__enabledtaskcount);
	package.add("enablerecipe", &Perl__enablerecipe);
	package.add("enabletask", &Perl__enabletask);
	package.add("enabletitle", &Perl__enabletitle);
	package.add("end_dz_task", (void(*)())&Perl__end_dz_task);
	package.add("end_dz_task", (void(*)(bool))&Perl__end_dz_task);
	package.add("exp", &Perl__exp);
	package.add("faction", (void(*)(int, int))&Perl__faction);
	package.add("faction", (void(*)(int, int, int))&Perl__faction);
	package.add("factionvalue", &Perl__FactionValue);
	package.add("failtask", &Perl__failtask);
	package.add("completetask", &Perl__completetask);
	package.add("uncompletetask", &Perl__uncompletetask);
	package.add("firsttaskinset", &Perl__firsttaskinset);
	package.add("follow", (void(*)(int))&Perl__follow);
	package.add("follow", (void(*)(int, int))&Perl__follow);
	package.add("forcedoorclose", (void(*)(uint32))&Perl__forcedoorclose);
	package.add("forcedoorclose", (void(*)(uint32, bool))&Perl__forcedoorclose);
	package.add("forcedooropen", (void(*)(uint32))&Perl__forcedooropen);
	package.add("forcedooropen", (void(*)(uint32, bool))&Perl__forcedooropen);
	package.add("getaaexpmodifierbycharid", (double(*)(uint32, uint32))&Perl__getaaexpmodifierbycharid);
	package.add("getaaexpmodifierbycharid", (double(*)(uint32, uint32, int16))&Perl__getaaexpmodifierbycharid);
	package.add("getaaname", (std::string(*)(int))&Perl__getaaname);
	package.add("getbodytypename", &Perl__getbodytypename);
	package.add("getcharidbyname", &Perl__getcharidbyname);
	package.add("getclassname", (std::string(*)(uint8))&Perl__getclassname);
	package.add("getclassname", (std::string(*)(uint8, uint8))&Perl__getclassname);
	package.add("getcleannpcnamebyid", &Perl__getcleannpcnamebyid);
	package.add("getconsiderlevelname", &Perl__getconsiderlevelname);
	package.add("gethexcolorcode", &Perl__gethexcolorcode);
	package.add("getcurrencyid", &Perl__getcurrencyid);
	package.add("get_class_bitmask", &Perl__get_class_bitmask);
	package.add("get_data", &Perl__get_data);
	package.add("get_data_expires", &Perl__get_data_expires);
	package.add("get_data_remaining", &Perl__get_data_remaining);
	package.add("get_deity_bitmask", &Perl__get_deity_bitmask);
	package.add("get_dz_task_id", &Perl__get_dz_task_id);
	package.add("getexpmodifierbycharid", (double(*)(uint32, uint32))&Perl__getexpmodifierbycharid);
	package.add("getexpmodifierbycharid", (double(*)(uint32, uint32, int16))&Perl__getexpmodifierbycharid);
	package.add("get_expedition", &Perl__get_expedition);
	package.add("get_expedition_by_char_id", &Perl__get_expedition_by_char_id);
	package.add("get_expedition_by_dz_id", &Perl__get_expedition_by_dz_id);
	package.add("get_expedition_by_zone_instance", &Perl__get_expedition_by_zone_instance);
	package.add("get_expedition_lockout_by_char_id", &Perl__get_expedition_lockout_by_char_id);
	package.add("get_expedition_lockouts_by_char_id", (perl::reference(*)(uint32))&Perl__get_expedition_lockouts_by_char_id);
	package.add("get_expedition_lockouts_by_char_id", (perl::reference(*)(uint32, std::string))&Perl__get_expedition_lockouts_by_char_id);
	package.add("getfactionname", &Perl__getfactionname);
	package.add("getinventoryslotid", &Perl__getinventoryslotid);
	package.add("getitemcomment", &Perl__getitemcomment);
	package.add("getitemlore", &Perl__getitemlore);
	package.add("getitemname", &Perl__getitemname);
	package.add("getitemstat", &Perl__getitemstat);
	package.add("getlanguagename", &Perl__getlanguagename);
	package.add("getldonthemename", &Perl__getldonthemename);
	package.add("getnpcnamebyid", &Perl__getnpcnamebyid);
	package.add("get_rule", &Perl__get_rule);
	package.add("get_spawn_condition", (int(*)(const char*, uint16))&Perl__get_spawn_condition);
	package.add("get_spawn_condition", (int(*)(const char*, uint32, uint16))&Perl__get_spawn_condition);
	package.add("getcharnamebyid", &Perl__getcharnamebyid);
	package.add("getcurrencyitemid", &Perl__getcurrencyitemid);
	package.add("getgendername", &Perl__getgendername);
	package.add("getdeityname", &Perl__getdeityname);
	package.add("getenvironmentaldamagename", &Perl__getenvironmentaldamagename);
	package.add("getguildnamebyid", &Perl__getguildnamebyid);
	package.add("getguildidbycharid", &Perl__getguildidbycharid);
	package.add("getgroupidbycharid", &Perl__getgroupidbycharid);
	package.add("getinventoryslotname", &Perl__getinventoryslotname);
	package.add("get_paused_timers", &Perl__get_paused_timers);
	package.add("get_pet_command_name", &Perl__get_pet_command_name);
	package.add("get_pet_type_name", &Perl__get_pet_type_name);
	package.add("getraididbycharid", &Perl__getraididbycharid);
	package.add("get_race_bitmask", &Perl__get_race_bitmask);
	package.add("get_recipe_component_item_ids", &Perl__GetRecipeComponentItemIDs);
	package.add("get_recipe_container_item_ids", &Perl__GetRecipeContainerItemIDs);
	package.add("get_recipe_fail_item_ids", &Perl__GetRecipeFailItemIDs);
	package.add("get_recipe_salvage_item_ids", &Perl__GetRecipeSalvageItemIDs);
	package.add("get_recipe_success_item_ids", &Perl__GetRecipeSuccessItemIDs);
	package.add("get_recipe_component_count", &Perl__GetRecipeComponentCount);
	package.add("get_recipe_fail_count", &Perl__GetRecipeFailCount);
	package.add("get_recipe_salvage_count", &Perl__GetRecipeSalvageCount);
	package.add("get_recipe_success_count", &Perl__GetRecipeSuccessCount);
	package.add("getracename", &Perl__getracename);
	package.add("getremainingtimeMS", &Perl__getremainingtimeMS);
	package.add("getspell", &Perl__getspell);
	package.add("getspellname", &Perl__getspellname);
	package.add("get_spell_level", &Perl__get_spell_level);
	package.add("getspellstat", (int(*)(uint32, std::string))&Perl__getspellstat);
	package.add("getspellstat", (int(*)(uint32, std::string, uint8))&Perl__getspellstat);
	package.add("getskillname", &Perl__getskillname);
	package.add("get_timers", &Perl__get_timers);
	package.add("getlevel", &Perl__getlevel);
	package.add("getplayerburiedcorpsecount", &Perl__getplayerburiedcorpsecount);
	package.add("getplayercorpsecount", &Perl__getplayercorpsecount);
	package.add("getplayercorpsecountbyzoneid", &Perl__getplayercorpsecountbyzoneid);
	package.add("getrecipemadecount", &Perl__getrecipemadecount);
	package.add("getrecipename", &Perl__getrecipename);
	package.add("gettaskactivitydonecount", &Perl__gettaskactivitydonecount);
	package.add("gettaskname", &Perl__gettaskname);
	package.add("gettimerdurationMS", &Perl__gettimerdurationMS);
	package.add("givecash", (void(*)(uint32))&Perl__givecash);
	package.add("givecash", (void(*)(uint32, uint32))&Perl__givecash);
	package.add("givecash", (void(*)(uint32, uint32, uint32))&Perl__givecash);
	package.add("givecash", (void(*)(uint32, uint32, uint32, uint32))&Perl__givecash);
	package.add("gmmove", &Perl__gmmove);
	package.add("gmsay", (void(*)(const char*))&Perl__gmsay);
	package.add("gmsay", (void(*)(const char*, int))&Perl__gmsay);
	package.add("gmsay", (void(*)(const char*, int, bool))&Perl__gmsay);
	package.add("gmsay", (void(*)(const char*, int, bool, int))&Perl__gmsay);
	package.add("gmsay", (void(*)(const char*, int, bool, int, int))&Perl__gmsay);
	package.add("handin", &Perl__handin);
	package.add("has_zone_flag", &Perl__has_zone_flag);
	package.add("hasrecipelearned", &Perl__hasrecipelearned);
	package.add("hastimer", &Perl__hastimer);
	package.add("incstat", &Perl__incstat);
	package.add("isdisctome", &Perl__isdisctome);
	package.add("isdooropen", &Perl__isdooropen);
	package.add("ishotzone", &Perl__ishotzone);
	package.add("isnpcspawned", &Perl__isnpcspawned);
	package.add("ispausedtimer", &Perl__ispausedtimer);
	package.add("istaskactive", &Perl__istaskactive);
	package.add("istaskactivityactive", &Perl__istaskactivityactive);
	package.add("istaskappropriate", &Perl__istaskappropriate);
	package.add("istaskcompleted", &Perl__istaskcompleted);
	package.add("istaskenabled", &Perl__istaskenabled);
	package.add("itemlink", &Perl__itemlink);
	package.add("lasttaskinset", &Perl__lasttaskinset);
	package.add("level", &Perl__level);
	package.add("log", &Perl__log);
	package.add("log_combat", &Perl__log_combat);
	package.add("marquee", (void(*)(uint32, std::string))&Perl__marquee);
	package.add("marquee", (void(*)(uint32, std::string, uint32))&Perl__marquee);
	package.add("marquee", (void(*)(uint32, uint32, uint32, uint32, uint32, std::string))&Perl__marquee);
	package.add("me", &Perl__me);
	package.add("message", &Perl__message);
	package.add("modifynpcstat", &Perl__ModifyNPCStat);
	package.add("movegrp", &Perl__movegrp);
	package.add("movepc",(void(*)(int, float, float, float))&Perl__movepc);
	package.add("movepc",(void(*)(int, float, float, float, float))&Perl__movepc);
	package.add("moveto", (void(*)(float, float, float))&Perl__moveto);
	package.add("moveto", (void(*)(float, float, float, float))&Perl__moveto);
	package.add("moveto", (void(*)(float, float, float, float, bool))&Perl__moveto);
	package.add("nexttaskinset", &Perl__nexttaskinset);
	package.add("npcfeature", &Perl__npcfeature);
	package.add("npcgender", &Perl__npcgender);
	package.add("npcrace", &Perl__npcrace);
	package.add("npcsize", &Perl__npcsize);
	package.add("npctexture", &Perl__npctexture);
	package.add("pause", &Perl__pause);
	package.add("pausetimer", &Perl__pausetimer);
	package.add("permaclass", &Perl__permaclass);
	package.add("permagender", &Perl__permagender);
	package.add("permarace", &Perl__permarace);
	package.add("playerfeature", &Perl__playerfeature);
	package.add("playergender", &Perl__playergender);
	package.add("playerrace", &Perl__playerrace);
	package.add("playersize", &Perl__playersize);
	package.add("playertexture", &Perl__playertexture);
	package.add("popup", (void(*)(const char*, const char*))&Perl__popup);
	package.add("popup", (void(*)(const char*, const char*, int))&Perl__popup);
	package.add("popup", (void(*)(const char*, const char*, int, int))&Perl__popup);
	package.add("popup", (void(*)(const char*, const char*, int, int, int))&Perl__popup);
	package.add("popupbreak", (std::string(*)())&Perl__popupbreak);
	package.add("popupbreak", (std::string(*)(uint32))&Perl__popupbreak);
	package.add("popupcentermessage", &Perl__popupcentermessage);
	package.add("popupcolormessage", &Perl__popupcolormessage);
	package.add("popupindent", (std::string(*)())&Perl__popupindent);
	package.add("popupindent", (std::string(*)(uint32))&Perl__popupindent);
	package.add("popuplink", (std::string(*)(std::string))&Perl__popuplink);
	package.add("popuplink", (std::string(*)(std::string, std::string))&Perl__popuplink);
	package.add("popuptable", &Perl__popuptable);
	package.add("popuptablecell", (std::string(*)())&Perl__popuptablecell);
	package.add("popuptablecell", (std::string(*)(std::string))&Perl__popuptablecell);
	package.add("popuptablerow", &Perl__popuptablerow);
	package.add("processmobswhilezoneempty", &Perl__processmobswhilezoneempty);
	package.add("pvp", &Perl__pvp);
	package.add("qs_send_query", &Perl__qs_send_query);
	package.add("rain", &Perl__rain);
	package.add("rebind", (void(*)(int, float, float, float))&Perl__rebind);
	package.add("rebind", (void(*)(int, float, float, float, float))&Perl__rebind);
	package.add("reloadzonestaticdata", &Perl__reloadzonestaticdata);
	package.add("remove_all_expedition_lockouts_by_char_id", (void(*)(uint32))&Perl__remove_all_expedition_lockouts_by_char_id);
	package.add("remove_all_expedition_lockouts_by_char_id", (void(*)(uint32, std::string))&Perl__remove_all_expedition_lockouts_by_char_id);
	package.add("remove_expedition_lockout_by_char_id", &Perl__remove_expedition_lockout_by_char_id);
	package.add("removeitem", (void(*)(uint32_t))&Perl__removeitem);
	package.add("removeitem", (void(*)(uint32_t, int))&Perl__removeitem);
	package.add("removeldonloss", &Perl__removeldonloss);
	package.add("removeldonwin", &Perl__removeldonwin);
	package.add("removetitle", &Perl__removetitle);
	package.add("rename", &Perl__rename);
	package.add("repopzone", (void(*)(void))&Perl__repopzone);
	package.add("repopzone", (void(*)(bool))&Perl__repopzone);
	package.add("resettaskactivity", &Perl__resettaskactivity);
	package.add("respawn", &Perl__respawn);
	package.add("resume", &Perl__resume);
	package.add("resumetimer", &Perl__resumetimer);
	package.add("rewardfaction", &Perl__rewardfaction);
	package.add("safemove", &Perl__safemove);
	package.add("save", &Perl__save);
	package.add("say", (void(*)(const char*))&Perl__say);
	package.add("say", (void(*)(const char*, uint8))&Perl__say);
	package.add("say", (void(*)(const char*, uint8, int))&Perl__say);
	package.add("say", (void(*)(const char*, uint8, int, int))&Perl__say);
	package.add("say", (void(*)(const char*, uint8, int, int, int))&Perl__say);
	package.add("saylink", (std::string(*)(std::string))&Perl__saylink);
	package.add("saylink", (std::string(*)(std::string, bool))&Perl__saylink);
	package.add("saylink", (std::string(*)(std::string, bool, std::string))&Perl__saylink);
	package.add("scribespells", (int(*)(int))&Perl__scribespells);
	package.add("scribespells", (int(*)(int, int))&Perl__scribespells);
	package.add("secondstotime", &Perl__secondstotime);
	package.add("selfcast", &Perl__selfcast);
	package.add("send_player_handin_event", &Perl__send_player_handin_event);
	package.add("send_channel_message", (void(*)(uint8, uint32, uint8, uint8, const char*))&Perl__send_channel_message);
	package.add("send_channel_message", (void(*)(Client*, uint8, uint32, uint8, uint8, const char*))&Perl__send_channel_message);
	package.add("send_channel_message", (void(*)(Client*, const char*, uint8, uint32, uint8, uint8, const char*))&Perl__send_channel_message);
	package.add("send_parcel", &Perl__send_parcel);
	package.add("setaaexpmodifierbycharid", (void(*)(uint32, uint32, double))&Perl__setaaexpmodifierbycharid);
	package.add("setaaexpmodifierbycharid", (void(*)(uint32, uint32, double, int16))&Perl__setaaexpmodifierbycharid);
	package.add("set_data", (void(*)(std::string, std::string))&Perl__set_data);
	package.add("set_data", (void(*)(std::string, std::string, std::string))&Perl__set_data);
	package.add("set_proximity", (void(*)(float, float, float, float))&Perl__set_proximity);
	package.add("set_proximity", (void(*)(float, float, float, float, float, float))&Perl__set_proximity);
	package.add("set_proximity", (void(*)(float, float, float, float, float, float, bool))&Perl__set_proximity);
	package.add("set_proximity_range", (void(*)(float, float))&Perl__set_proximity_range);
	package.add("set_proximity_range", (void(*)(float, float, float))&Perl__set_proximity_range);
	package.add("set_proximity_range", (void(*)(float, float, float, bool))&Perl__set_proximity_range);
	package.add("set_rule", &Perl__set_rule);
	package.add("set_zone_flag", &Perl__set_zone_flag);
	package.add("setallskill", &Perl__setallskill);
	package.add("setanim", &Perl__setanim);
	package.add("setexpmodifierbycharid", (void(*)(uint32, uint32, double))&Perl__setexpmodifierbycharid);
	package.add("setexpmodifierbycharid", (void(*)(uint32, uint32, double, int16))&Perl__setexpmodifierbycharid);
	package.add("setglobal", &Perl__setglobal);
	package.add("setguild", &Perl__setguild);
	package.add("sethp", &Perl__sethp);
	package.add("sethotzone", &Perl__sethotzone);
	package.add("setlanguage", &Perl__setlanguage);
	package.add("setnexthpevent", &Perl__setnexthpevent);
	package.add("setnextinchpevent", &Perl__setnextinchpevent);
	package.add("setskill", &Perl__setskill);
	package.add("setsky", &Perl__setsky);
	package.add("setstat", &Perl__setstat);
	package.add("settarget", &Perl__settarget);
	package.add("settime", (void(*)(int, int))&Perl__settime);
	package.add("settime", (void(*)(int, int, bool))&Perl__settime);
	package.add("settimer", (void(*)(std::string, uint32))&Perl__settimer);
	package.add("settimer", (void(*)(std::string, uint32, EQ::ItemInstance*))&Perl__settimer);
	package.add("settimer", (void(*)(std::string, uint32, Mob*))&Perl__settimer);
	package.add("settimerMS", (void(*)(std::string, uint32))&Perl__settimerMS);
	package.add("settimerMS", (void(*)(std::string, uint32, EQ::ItemInstance*))&Perl__settimerMS);
	package.add("settimerMS", (void(*)(std::string, uint32, Mob*))&Perl__settimerMS);
	package.add("sfollow", &Perl__sfollow);
	package.add("shout", &Perl__shout);
	package.add("shout2", &Perl__shout2);
	package.add("showgrid", &Perl__showgrid);
	package.add("signal", (void(*)(int))&Perl__signal);
	package.add("signal", (void(*)(int, int))&Perl__signal);
	package.add("signalwith", (void(*)(int, int))&Perl__signalwith);
	package.add("signalwith", (void(*)(int, int, int))&Perl__signalwith);
	package.add("silent_saylink", (std::string(*)(std::string))&Perl__silent_saylink);
	package.add("silent_saylink", (std::string(*)(std::string, std::string))&Perl__silent_saylink);
	package.add("snow", &Perl__snow);
	package.add("spawn", &Perl__spawn);
	package.add("spawn2", &Perl__spawn2);
	package.add("spawn_condition", (void(*)(const char*, uint16, int16))&Perl__spawn_condition);
	package.add("spawn_condition", (void(*)(const char*, uint32_t, uint16, int16))&Perl__spawn_condition);
	package.add("spawn_from_spawn2", &Perl__spawn_from_spawn2);
	package.add("start", &Perl__start);
	package.add("stop", &Perl__stop);
	package.add("stopalltimers", &Perl__stopalltimers);
	package.add("stoptimer", &Perl__stoptimer);
	package.add("summonallplayercorpses", &Perl__summonallplayercorpses);
	package.add("summonburiedplayercorpse", &Perl__summonburiedplayercorpse);
	package.add("summonitem", (void(*)(int))&Perl__summonitem);
	package.add("summonitem", (void(*)(int, int))&Perl__summonitem);
	package.add("surname", &Perl__surname);
	package.add("targlobal", &Perl__targlobal);
	package.add("taskselector", &Perl__taskselector);
	package.add("taskselector_nocooldown", &Perl__taskselector_nocooldown);
	package.add("task_setselector", (void(*)(int))&Perl__task_setselector);
	package.add("task_setselector", (void(*)(int, bool))&Perl__task_setselector);
	package.add("tasktimeleft", &Perl__tasktimeleft);
	package.add("timetoseconds", &Perl__timetoseconds);
	package.add("toggle_spawn_event", &Perl__toggle_spawn_event);
	package.add("toggledoorstate", &Perl__toggledoorstate);
	package.add("tracknpc", &Perl__tracknpc);
	package.add("traindisc", &Perl__traindisc);
	package.add("traindiscs", (int(*)(int))&Perl__traindiscs);
	package.add("traindiscs", (int(*)(int, int))&Perl__traindiscs);
	package.add("unique_spawn", (int(*)(int, int, int, float, float, float))&Perl__unique_spawn);
	package.add("unique_spawn", (int(*)(int, int, int, float, float, float, float))&Perl__unique_spawn);
	package.add("unscribespells", &Perl__unscribespells);
	package.add("untraindiscs", &Perl__untraindiscs);
	package.add("updatespawntimer", &Perl__UpdateSpawnTimer);
	package.add("updatetaskactivity", (void(*)(int, int))&Perl__updatetaskactivity);
	package.add("updatetaskactivity", (void(*)(int, int, int))&Perl__updatetaskactivity);
	package.add("updatetaskactivity", (void(*)(int, int, int, bool))&Perl__updatetaskactivity);
	package.add("UpdateZoneHeader", &Perl__UpdateZoneHeader);
	package.add("varlink", (std::string(*)(EQ::ItemInstance*))&Perl__varlink);
	package.add("varlink", (std::string(*)(uint32))&Perl__varlink);
	package.add("varlink", (std::string(*)(uint32, int16))&Perl__varlink);
	package.add("varlink", (std::string(*)(uint32, int16, uint32))&Perl__varlink);
	package.add("varlink", (std::string(*)(uint32, int16, uint32, uint32))&Perl__varlink);
	package.add("varlink", (std::string(*)(uint32, int16, uint32, uint32, uint32))&Perl__varlink);
	package.add("varlink", (std::string(*)(uint32, int16, uint32, uint32, uint32, uint32))&Perl__varlink);
	package.add("varlink", (std::string(*)(uint32, int16, uint32, uint32, uint32, uint32, uint32))&Perl__varlink);
	package.add("varlink", (std::string(*)(uint32, int16, uint32, uint32, uint32, uint32, uint32, uint32))&Perl__varlink);
	package.add("varlink", (std::string(*)(uint32, int16, uint32, uint32, uint32, uint32, uint32, uint32, bool))&Perl__varlink);
	package.add("voicetell", &Perl__voicetell);
	package.add("we", &Perl__we);
	package.add("wearchange", (void(*)(uint8, uint32))&Perl__wearchange);
	package.add("wearchange", (void(*)(uint8, uint32, uint32))&Perl__wearchange);
	package.add("wearchange", (void(*)(uint8, uint32, uint32, uint32))&Perl__wearchange);
	package.add("whisper", &Perl__whisper);
	package.add("write", &Perl__write);
	package.add("ze", &Perl__ze);
	package.add("zone", &Perl__zone);
	package.add("zonemarquee", (void(*)(uint32, std::string))&Perl__zonemarquee);
	package.add("zonemarquee", (void(*)(uint32, std::string, uint32))&Perl__zonemarquee);
	package.add("zonemarquee", (void(*)(uint32, uint32, uint32, uint32, uint32, std::string))&Perl__zonemarquee);
	package.add("zonegroup", &Perl__zonegroup);
	package.add("zoneraid", &Perl__zoneraid);

	/**
	 * Expansions
	 */
	package.add("is_classic_enabled", &Perl__IsClassicEnabled);
	package.add("is_the_ruins_of_kunark_enabled", &Perl__IsTheRuinsOfKunarkEnabled);
	package.add("is_the_scars_of_velious_enabled", &Perl__IsTheScarsOfVeliousEnabled);
	package.add("is_the_shadows_of_luclin_enabled", &Perl__IsTheShadowsOfLuclinEnabled);
	package.add("is_the_planes_of_power_enabled", &Perl__IsThePlanesOfPowerEnabled);
	package.add("is_the_legacy_of_ykesha_enabled", &Perl__IsTheLegacyOfYkeshaEnabled);
	package.add("is_lost_dungeons_of_norrath_enabled", &Perl__IsLostDungeonsOfNorrathEnabled);
	package.add("is_gates_of_discord_enabled", &Perl__IsGatesOfDiscordEnabled);
	package.add("is_omens_of_war_enabled", &Perl__IsOmensOfWarEnabled);
	package.add("is_dragons_of_norrath_enabled", &Perl__IsDragonsOfNorrathEnabled);
	package.add("is_depths_of_darkhollow_enabled", &Perl__IsDepthsOfDarkhollowEnabled);
	package.add("is_prophecy_of_ro_enabled", &Perl__IsProphecyOfRoEnabled);
	package.add("is_the_serpents_spine_enabled", &Perl__IsTheSerpentsSpineEnabled);
	package.add("is_the_buried_sea_enabled", &Perl__IsTheBuriedSeaEnabled);
	package.add("is_secrets_of_faydwer_enabled", &Perl__IsSecretsOfFaydwerEnabled);
	package.add("is_seeds_of_destruction_enabled", &Perl__IsSeedsOfDestructionEnabled);
	package.add("is_underfoot_enabled", &Perl__IsUnderfootEnabled);
	package.add("is_house_of_thule_enabled", &Perl__IsHouseOfThuleEnabled);
	package.add("is_veil_of_alaris_enabled", &Perl__IsVeilOfAlarisEnabled);
	package.add("is_rain_of_fear_enabled", &Perl__IsRainOfFearEnabled);
	package.add("is_call_of_the_forsaken_enabled", &Perl__IsCallOfTheForsakenEnabled);
	package.add("is_the_darkened_sea_enabled", &Perl__IsTheDarkenedSeaEnabled);
	package.add("is_the_broken_mirror_enabled", &Perl__IsTheBrokenMirrorEnabled);
	package.add("is_empires_of_kunark_enabled", &Perl__IsEmpiresOfKunarkEnabled);
	package.add("is_ring_of_scale_enabled", &Perl__IsRingOfScaleEnabled);
	package.add("is_the_burning_lands_enabled", &Perl__IsTheBurningLandsEnabled);
	package.add("is_torment_of_velious_enabled", &Perl__IsTormentOfVeliousEnabled);
	package.add("is_current_expansion_classic", &Perl__IsCurrentExpansionClassic);
	package.add("is_current_expansion_the_ruins_of_kunark", &Perl__IsCurrentExpansionTheRuinsOfKunark);
	package.add("is_current_expansion_the_scars_of_velious", &Perl__IsCurrentExpansionTheScarsOfVelious);
	package.add("is_current_expansion_the_shadows_of_luclin", &Perl__IsCurrentExpansionTheShadowsOfLuclin);
	package.add("is_current_expansion_the_planes_of_power", &Perl__IsCurrentExpansionThePlanesOfPower);
	package.add("is_current_expansion_the_legacy_of_ykesha", &Perl__IsCurrentExpansionTheLegacyOfYkesha);
	package.add("is_current_expansion_lost_dungeons_of_norrath", &Perl__IsCurrentExpansionLostDungeonsOfNorrath);
	package.add("is_current_expansion_gates_of_discord", &Perl__IsCurrentExpansionGatesOfDiscord);
	package.add("is_current_expansion_omens_of_war", &Perl__IsCurrentExpansionOmensOfWar);
	package.add("is_current_expansion_dragons_of_norrath", &Perl__IsCurrentExpansionDragonsOfNorrath);
	package.add("is_current_expansion_depths_of_darkhollow", &Perl__IsCurrentExpansionDepthsOfDarkhollow);
	package.add("is_current_expansion_prophecy_of_ro", &Perl__IsCurrentExpansionProphecyOfRo);
	package.add("is_current_expansion_the_serpents_spine", &Perl__IsCurrentExpansionTheSerpentsSpine);
	package.add("is_current_expansion_the_buried_sea", &Perl__IsCurrentExpansionTheBuriedSea);
	package.add("is_current_expansion_secrets_of_faydwer", &Perl__IsCurrentExpansionSecretsOfFaydwer);
	package.add("is_current_expansion_seeds_of_destruction", &Perl__IsCurrentExpansionSeedsOfDestruction);
	package.add("is_current_expansion_underfoot", &Perl__IsCurrentExpansionUnderfoot);
	package.add("is_current_expansion_house_of_thule", &Perl__IsCurrentExpansionHouseOfThule);
	package.add("is_current_expansion_veil_of_alaris", &Perl__IsCurrentExpansionVeilOfAlaris);
	package.add("is_current_expansion_rain_of_fear", &Perl__IsCurrentExpansionRainOfFear);
	package.add("is_current_expansion_call_of_the_forsaken", &Perl__IsCurrentExpansionCallOfTheForsaken);
	package.add("is_current_expansion_the_darkened_sea", &Perl__IsCurrentExpansionTheDarkenedSea);
	package.add("is_current_expansion_the_broken_mirror", &Perl__IsCurrentExpansionTheBrokenMirror);
	package.add("is_current_expansion_empires_of_kunark", &Perl__IsCurrentExpansionEmpiresOfKunark);
	package.add("is_current_expansion_ring_of_scale", &Perl__IsCurrentExpansionRingOfScale);
	package.add("is_current_expansion_the_burning_lands", &Perl__IsCurrentExpansionTheBurningLands);
	package.add("is_current_expansion_torment_of_velious", &Perl__IsCurrentExpansionTormentOfVelious);

	/**
	 * Content flags
	 */
	package.add("is_content_flag_enabled", &Perl__IsContentFlagEnabled);
	package.add("set_content_flag", &Perl__SetContentFlag);

}

#endif // EMBPERL_XS
#endif // EMBPERL
