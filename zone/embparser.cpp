/*  EQEMu:  Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

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
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
*/

#ifdef EMBPERL

#include "common/features.h"
#include "common/misc_functions.h"
#include "common/seperator.h"
#include "common/strings.h"
#include "zone/embparser.h"
#include "zone/masterentity.h"
#include "zone/qglobals.h"
#include "zone/questmgr.h"
#include "zone/zone.h"

#include <algorithm>
#include <sstream>

extern Zone* zone;

#ifdef EMBPERL_XS
void perl_register_quest();
#ifdef EMBPERL_XS_CLASSES
void perl_register_mob();
void perl_register_npc();
void perl_register_client();
void perl_register_corpse();
void perl_register_entitylist();
void perl_register_perlpacket();
void perl_register_group();
void perl_register_raid();
void perl_register_inventory();
void perl_register_questitem();
void perl_register_questitem_data();
void perl_register_spawn();
void perl_register_spell();
void perl_register_stat_bonuses();
void perl_register_hateentry();
void perl_register_object();
void perl_register_doors();
void perl_register_expedition();
void perl_register_expedition_lock_messages();
void perl_register_bot();
void perl_register_buff();
void perl_register_merc();
void perl_register_database();
void perl_register_zone();
#endif // EMBPERL_XS_CLASSES
#endif // EMBPERL_XS

const char* QuestEventSubroutines[_LargestEventID] = {
	"EVENT_SAY",
	"EVENT_ITEM",
	"EVENT_DEATH",
	"EVENT_SPAWN",
	"EVENT_ATTACK",
	"EVENT_COMBAT",
	"EVENT_AGGRO",
	"EVENT_SLAY",
	"EVENT_NPC_SLAY",
	"EVENT_WAYPOINT_ARRIVE",
	"EVENT_WAYPOINT_DEPART",
	"EVENT_TIMER",
	"EVENT_SIGNAL",
	"EVENT_HP",
	"EVENT_ENTER",
	"EVENT_EXIT",
	"EVENT_ENTERZONE",
	"EVENT_CLICKDOOR",
	"EVENT_LOOT",
	"EVENT_ZONE",
	"EVENT_LEVEL_UP",
	"EVENT_KILLED_MERIT",
	"EVENT_CAST_ON",
	"EVENT_TASKACCEPTED",
	"EVENT_TASK_STAGE_COMPLETE",
	"EVENT_TASK_UPDATE",
	"EVENT_TASK_COMPLETE",
	"EVENT_TASK_FAIL",
	"EVENT_AGGRO_SAY",
	"EVENT_PLAYER_PICKUP",
	"EVENT_POPUPRESPONSE",
	"EVENT_ENVIRONMENTAL_DAMAGE",
	"EVENT_PROXIMITY_SAY",
	"EVENT_CAST",
	"EVENT_CAST_BEGIN",
	"EVENT_SCALE_CALC",
	"EVENT_ITEM_ENTER_ZONE",
	"EVENT_TARGET_CHANGE",
	"EVENT_HATE_LIST",
	"EVENT_SPELL_EFFECT_CLIENT",
	"EVENT_SPELL_EFFECT_NPC",
	"EVENT_SPELL_EFFECT_BUFF_TIC_CLIENT",
	"EVENT_SPELL_EFFECT_BUFF_TIC_NPC",
	"EVENT_SPELL_FADE",
	"EVENT_SPELL_EFFECT_TRANSLOCATE_COMPLETE",
	"EVENT_COMBINE_SUCCESS",
	"EVENT_COMBINE_FAILURE",
	"EVENT_ITEM_CLICK",
	"EVENT_ITEM_CLICK_CAST",
	"EVENT_GROUP_CHANGE",
	"EVENT_FORAGE_SUCCESS",
	"EVENT_FORAGE_FAILURE",
	"EVENT_FISH_START",
	"EVENT_FISH_SUCCESS",
	"EVENT_FISH_FAILURE",
	"EVENT_CLICK_OBJECT",
	"EVENT_DISCOVER_ITEM",
	"EVENT_DISCONNECT",
	"EVENT_CONNECT",
	"EVENT_ITEM_TICK",
	"EVENT_DUEL_WIN",
	"EVENT_DUEL_LOSE",
	"EVENT_ENCOUNTER_LOAD",
	"EVENT_ENCOUNTER_UNLOAD",
	"EVENT_COMMAND",
	"EVENT_DROP_ITEM",
	"EVENT_DESTROY_ITEM",
	"EVENT_FEIGN_DEATH",
	"EVENT_WEAPON_PROC",
	"EVENT_EQUIP_ITEM",
	"EVENT_UNEQUIP_ITEM",
	"EVENT_AUGMENT_ITEM",
	"EVENT_UNAUGMENT_ITEM",
	"EVENT_AUGMENT_INSERT",
	"EVENT_AUGMENT_REMOVE",
	"EVENT_ENTER_AREA",
	"EVENT_LEAVE_AREA",
	"EVENT_RESPAWN",
	"EVENT_DEATH_COMPLETE",
	"EVENT_UNHANDLED_OPCODE",
	"EVENT_TICK",
	"EVENT_SPAWN_ZONE",
	"EVENT_DEATH_ZONE",
	"EVENT_USE_SKILL",
	"EVENT_COMBINE_VALIDATE",
	"EVENT_BOT_COMMAND",
	"EVENT_WARP",
	"EVENT_TEST_BUFF",
	"EVENT_COMBINE",
	"EVENT_CONSIDER",
	"EVENT_CONSIDER_CORPSE",
	"EVENT_LOOT_ZONE",
	"EVENT_EQUIP_ITEM_CLIENT",
	"EVENT_UNEQUIP_ITEM_CLIENT",
	"EVENT_SKILL_UP",
	"EVENT_LANGUAGE_SKILL_UP",
	"EVENT_ALT_CURRENCY_MERCHANT_BUY",
	"EVENT_ALT_CURRENCY_MERCHANT_SELL",
	"EVENT_MERCHANT_BUY",
	"EVENT_MERCHANT_SELL",
	"EVENT_INSPECT",
	"EVENT_TASK_BEFORE_UPDATE",
	"EVENT_AA_BUY",
	"EVENT_AA_GAIN",
	"EVENT_AA_EXP_GAIN",
	"EVENT_EXP_GAIN",
	"EVENT_PAYLOAD",
	"EVENT_LEVEL_DOWN",
	"EVENT_GM_COMMAND",
	"EVENT_DESPAWN",
	"EVENT_DESPAWN_ZONE",
	"EVENT_BOT_CREATE",
	"EVENT_AUGMENT_INSERT_CLIENT",
	"EVENT_AUGMENT_REMOVE_CLIENT",
	"EVENT_EQUIP_ITEM_BOT",
	"EVENT_UNEQUIP_ITEM_BOT",
	"EVENT_DAMAGE_GIVEN",
	"EVENT_DAMAGE_TAKEN",
	"EVENT_ITEM_CLICK_CLIENT",
	"EVENT_ITEM_CLICK_CAST_CLIENT",
	"EVENT_DESTROY_ITEM_CLIENT",
	"EVENT_DROP_ITEM_CLIENT",
	"EVENT_MEMORIZE_SPELL",
	"EVENT_UNMEMORIZE_SPELL",
	"EVENT_SCRIBE_SPELL",
	"EVENT_UNSCRIBE_SPELL",
	"EVENT_LOOT_ADDED",
	"EVENT_LDON_POINTS_GAIN",
	"EVENT_LDON_POINTS_LOSS",
	"EVENT_ALT_CURRENCY_GAIN",
	"EVENT_ALT_CURRENCY_LOSS",
	"EVENT_CRYSTAL_GAIN",
	"EVENT_CRYSTAL_LOSS",
	"EVENT_TIMER_PAUSE",
	"EVENT_TIMER_RESUME",
	"EVENT_TIMER_START",
	"EVENT_TIMER_STOP",
	"EVENT_ENTITY_VARIABLE_DELETE",
	"EVENT_ENTITY_VARIABLE_SET",
	"EVENT_ENTITY_VARIABLE_UPDATE",
	"EVENT_AA_LOSS",
	"EVENT_SPELL_BLOCKED",
	"EVENT_READ_ITEM",
	"EVENT_PET_COMMAND",
	"EVENT_CHARM_START",
	"EVENT_CHARM_END",

	// Add new events before these or Lua crashes
	"EVENT_SPELL_EFFECT_BOT",
	"EVENT_SPELL_EFFECT_BUFF_TIC_BOT"
};

PerlembParser::PerlembParser() : perl(nullptr)
{
	global_npc_quest_status_    = questUnloaded;
	player_quest_status_        = questUnloaded;
	global_player_quest_status_ = questUnloaded;
	bot_quest_status_           = questUnloaded;
	global_bot_quest_status_    = questUnloaded;
	merc_quest_status_          = questUnloaded;
	global_merc_quest_status_   = questUnloaded;
	zone_quest_status_          = questUnloaded;
	global_zone_quest_status_   = questUnloaded;
}

PerlembParser::~PerlembParser()
{
	safe_delete(perl);
}

void PerlembParser::Init()
{
	ReloadQuests();
}

void PerlembParser::ReloadQuests()
{
	try {
		if (!perl) {
			perl = new Embperl;
		} else {
			perl->Reinit();
		}

		MapFunctions();
	}
	catch (std::exception& e) {
		if (perl) {
			delete perl;
			perl = nullptr;
		}

		LogInfo("Error Re-Initializing PerlEmbed: [{}]", e.what());
		throw e.what();
	}

	errors_.clear();
	npc_quest_status_.clear();

	global_npc_quest_status_    = questUnloaded;
	player_quest_status_        = questUnloaded;
	global_player_quest_status_ = questUnloaded;
	bot_quest_status_           = questUnloaded;
	global_bot_quest_status_    = questUnloaded;
	merc_quest_status_          = questUnloaded;
	global_merc_quest_status_   = questUnloaded;
	zone_quest_status_          = questUnloaded;
	global_zone_quest_status_   = questUnloaded;

	item_quest_status_.clear();
	spell_quest_status_.clear();
}

int PerlembParser::EventCommon(
	QuestEventID event_id,
	uint32 object_id,
	const char* data,
	Mob* npc_mob,
	EQ::ItemInstance* inst,
	const SPDat_Spell_Struct* spell,
	Mob* mob,
	Zone* zone,
	uint32 extra_data,
	bool is_global,
	std::vector<std::any>* extra_pointers
)
{
	if (!perl || event_id >= _LargestEventID) {
		return 0;
	}

	QuestType quest_type = GetQuestTypes(
		event_id,
		npc_mob,
		inst,
		mob,
		zone,
		is_global
	);

	std::string package_name = GetQuestPackageName(
		quest_type,
		event_id,
		object_id,
		data,
		npc_mob,
		inst
	);

	const std::string& sub_name = QuestEventSubroutines[event_id];

	if (!perl->SubExists(package_name.c_str(), sub_name.c_str())) {
		return 0;
	}

	int char_id = 0;

	ExportCharID(package_name, char_id, npc_mob, mob);

	/* Check for QGlobal export event enable */
	if (parse->perl_event_export_settings[event_id].qglobals) {
		ExportQGlobals(
			quest_type,
			package_name,
			npc_mob,
			mob,
			char_id
		);
	}

	/* Check for Mob export event enable */
	if (parse->perl_event_export_settings[event_id].mob) {
		ExportMobVariables(
			quest_type,
			package_name,
			mob,
			npc_mob
		);
	}

	/* Check for Zone export event enable */
	if (parse->perl_event_export_settings[event_id].zone) {
		ExportZoneVariables(package_name);
	}

	/* Check for Item export event enable */
	if (parse->perl_event_export_settings[event_id].item) {
		ExportItemVariables(package_name, mob);
	}

	/* Check for Event export event enable */
	if (parse->perl_event_export_settings[event_id].event_variables) {
		ExportEventVariables(package_name, event_id, object_id, data, npc_mob, inst, mob, extra_data, extra_pointers);
	}

	if (quest_type == QuestType::Player || quest_type == QuestType::PlayerGlobal) {
		return SendCommands(package_name.c_str(), QuestEventSubroutines[event_id], 0, mob, mob, nullptr, nullptr, nullptr);
	} else if (
		quest_type == QuestType::Bot ||
		quest_type == QuestType::BotGlobal ||
		quest_type == QuestType::Merc ||
		quest_type == QuestType::MercGlobal
	) {
		return SendCommands(package_name.c_str(), QuestEventSubroutines[event_id], 0, npc_mob, mob, nullptr, nullptr, nullptr);
	} else if (quest_type == QuestType::Item || quest_type == QuestType::ItemGlobal) {
		return SendCommands(package_name.c_str(), QuestEventSubroutines[event_id], 0, mob, mob, inst, nullptr, nullptr);
	} else if (quest_type == QuestType::Spell || quest_type == QuestType::SpellGlobal) {
		if (mob) {
			return SendCommands(package_name.c_str(), QuestEventSubroutines[event_id], 0, mob, mob, nullptr, spell, nullptr);
		} else {
			return SendCommands(package_name.c_str(), QuestEventSubroutines[event_id], 0, npc_mob, mob, nullptr, spell, nullptr);
		}
	} else if (quest_type == QuestType::NPC || quest_type == QuestType::NPCGlobal) {
		return SendCommands(
			package_name.c_str(),
			QuestEventSubroutines[event_id],
			object_id,
			npc_mob,
			mob,
			nullptr,
			nullptr,
			nullptr
		);
	} else if (quest_type == QuestType::Zone || quest_type == QuestType::ZoneGlobal) {
		return SendCommands(
			package_name.c_str(),
			QuestEventSubroutines[event_id],
			0,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			zone
		);
	}
}

int PerlembParser::EventNPC(
	QuestEventID event_id,
	NPC* npc,
	Mob* mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	return EventCommon(
		event_id,
		npc->GetNPCTypeID(),
		data.c_str(),
		npc,
		nullptr,
		nullptr,
		mob,
		nullptr,
		extra_data,
		false,
		extra_pointers
	);
}

int PerlembParser::EventGlobalNPC(
	QuestEventID event_id,
	NPC* npc,
	Mob* mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	return EventCommon(
		event_id,
		npc->GetNPCTypeID(),
		data.c_str(),
		npc,
		nullptr,
		nullptr,
		mob,
		nullptr,
		extra_data,
		true,
		extra_pointers
	);
}

int PerlembParser::EventPlayer(
	QuestEventID event_id,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	return EventCommon(
		event_id,
		0,
		data.c_str(),
		nullptr,
		nullptr,
		nullptr,
		client,
		nullptr,
		extra_data,
		false,
		extra_pointers
	);
}

int PerlembParser::EventGlobalPlayer(
	QuestEventID event_id,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	return EventCommon(
		event_id,
		0,
		data.c_str(),
		nullptr,
		nullptr,
		nullptr,
		client,
		nullptr,
		extra_data,
		true,
		extra_pointers
	);
}

int PerlembParser::EventItem(
	QuestEventID event_id,
	Client* client,
	EQ::ItemInstance* inst,
	Mob* mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	if (!inst) {
		return 0;
	}

	return EventCommon(
		event_id,
		inst->GetID(),
		nullptr,
		nullptr,
		inst,
		nullptr,
		client,
		nullptr,
		extra_data,
		false,
		extra_pointers
	);
}

int PerlembParser::EventSpell(
	QuestEventID event_id,
	Mob* mob,
	Client* client,
	uint32 spell_id,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	return EventCommon(
		event_id,
		spell_id,
		data.c_str(),
		mob,
		nullptr,
		&spells[spell_id],
		client,
		nullptr,
		extra_data,
		false,
		extra_pointers
	);
}

bool PerlembParser::HasQuestSub(uint32 npc_id, QuestEventID event_id)
{
	if (!perl || event_id >= _LargestEventID) {
		return false;
	}

	auto iter = npc_quest_status_.find(npc_id);
	if (iter == npc_quest_status_.end() || iter->second == QuestFailedToLoad) {
		return false;
	}

	const std::string& package_name = fmt::format(
		"qst_npc_{}",
		npc_id
	);

	return perl->SubExists(package_name.c_str(), QuestEventSubroutines[event_id]);
}

bool PerlembParser::HasGlobalQuestSub(QuestEventID event_id)
{
	if (
		!perl ||
		global_npc_quest_status_ != questLoaded ||
		event_id >= _LargestEventID
		) {
		return false;
	}

	return perl->SubExists("qst_global_npc", QuestEventSubroutines[event_id]);
}

bool PerlembParser::PlayerHasQuestSub(QuestEventID event_id)
{
	if (
		!perl ||
		player_quest_status_ != questLoaded ||
		event_id >= _LargestEventID
		) {
		return false;
	}

	return perl->SubExists("qst_player", QuestEventSubroutines[event_id]);
}

bool PerlembParser::GlobalPlayerHasQuestSub(QuestEventID event_id)
{
	if (
		!perl ||
		global_player_quest_status_ != questLoaded ||
		event_id >= _LargestEventID
	) {
		return false;
	}

	return perl->SubExists("qst_global_player", QuestEventSubroutines[event_id]);
}

bool PerlembParser::SpellHasQuestSub(uint32 spell_id, QuestEventID event_id)
{
	if (!perl || event_id >= _LargestEventID) {
		return false;
	}

	auto iter = spell_quest_status_.find(spell_id);
	if (iter == spell_quest_status_.end() || iter->second == QuestFailedToLoad) {
		return false;
	}

	const std::string& package_name = fmt::format(
		"qst_spell_{}",
		spell_id
	);

	return perl->SubExists(package_name.c_str(), QuestEventSubroutines[event_id]);
}

bool PerlembParser::ItemHasQuestSub(EQ::ItemInstance* inst, QuestEventID event_id)
{
	if (!perl || !inst || event_id >= _LargestEventID) {
		return false;
	}

	auto iter = item_quest_status_.find(inst->GetID());
	if (iter == item_quest_status_.end() || iter->second == QuestFailedToLoad) {
		return false;
	}

	const std::string& package_name = fmt::format(
		"qst_item_{}",
		inst->GetID()
	);

	return perl->SubExists(package_name.c_str(), QuestEventSubroutines[event_id]);
}

void PerlembParser::LoadNPCScript(std::string filename, int npc_id)
{
	if (!perl) {
		return;
	}

	auto iter = npc_quest_status_.find(npc_id);
	if (iter != npc_quest_status_.end()) {
		return;
	}

	const std::string& package_name = fmt::format(
		"qst_npc_{}",
		npc_id
	);

	try {
		perl->eval_file(package_name.c_str(), filename.c_str());
	} catch (std::string e) {
		AddError(
			fmt::format(
				"Error Compiling NPC Quest File [{}] NPC ID [{}] Error [{}]",
				filename,
				npc_id,
				e
			)
		);

		npc_quest_status_[npc_id] = questFailedToLoad;
		return;
	}

	npc_quest_status_[npc_id] = questLoaded;
}

void PerlembParser::LoadGlobalNPCScript(std::string filename)
{
	if (!perl || global_npc_quest_status_ != questUnloaded) {
		return;
	}

	try {
		perl->eval_file("qst_global_npc", filename.c_str());
	} catch (std::string e) {
		AddError(
			fmt::format(
				"Error Compiling Global NPC Quest File [{}] Error [{}]",
				filename,
				e
			)
		);

		global_npc_quest_status_ = questFailedToLoad;
		return;
	}

	global_npc_quest_status_ = questLoaded;
}

void PerlembParser::LoadPlayerScript(std::string filename)
{
	if (!perl || player_quest_status_ != questUnloaded) {
		return;
	}

	try {
		perl->eval_file("qst_player", filename.c_str());
	} catch (std::string e) {
		AddError(
			fmt::format(
				"Error Compiling Player Quest File [{}] Error [{}]",
				filename,
				e
			)
		);

		player_quest_status_ = questFailedToLoad;
		return;
	}

	player_quest_status_ = questLoaded;
}

void PerlembParser::LoadGlobalPlayerScript(std::string filename)
{
	if (!perl || global_player_quest_status_ != questUnloaded) {
		return;
	}

	try {
		perl->eval_file("qst_global_player", filename.c_str());
	} catch (std::string e) {
		AddError(
			fmt::format(
				"Error Compiling Global Player Quest File [{}] Error [{}]",
				filename,
				e
			)
		);

		global_player_quest_status_ = questFailedToLoad;
		return;
	}

	global_player_quest_status_ = questLoaded;
}

void PerlembParser::LoadItemScript(std::string filename, EQ::ItemInstance* inst)
{
	if (!inst || !perl) {
		return;
	}

	auto iter = item_quest_status_.find(inst->GetID());
	if (iter != item_quest_status_.end()) {
		return;
	}

	const std::string& package_name = fmt::format(
		"qst_item_{}",
		inst->GetID()
	);

	try {
		perl->eval_file(package_name.c_str(), filename.c_str());
	} catch (std::string e) {
		AddError(
			fmt::format(
				"Error Compiling Item Quest File [{}] Item ID [{}] Error [{}]",
				filename,
				inst->GetID(),
				e
			)
		);

		item_quest_status_[inst->GetID()] = questFailedToLoad;
		return;
	}

	item_quest_status_[inst->GetID()] = questLoaded;
}

void PerlembParser::LoadSpellScript(std::string filename, uint32 spell_id)
{
	if (!perl) {
		return;
	}

	auto iter = spell_quest_status_.find(spell_id);
	if (iter != spell_quest_status_.end()) {
		return;
	}

	const std::string& package_name = fmt::format(
		"qst_spell_{}",
		spell_id
	);

	try {
		perl->eval_file(package_name.c_str(), filename.c_str());
	} catch (std::string e) {
		AddError(
			fmt::format(
				"Error Compiling Spell Quest File [{}] Spell ID [{}] Error [{}]",
				filename,
				spell_id,
				e
			)
		);

		spell_quest_status_[spell_id] = questFailedToLoad;
		return;
	}

	spell_quest_status_[spell_id] = questLoaded;
}

void PerlembParser::AddVar(std::string name, std::string val)
{
	vars_[name] = val;
}

std::string PerlembParser::GetVar(std::string name)
{
	auto iter = vars_.find(name);
	if (iter != vars_.end()) {
		return iter->second;
	}

	return std::string();
}

void PerlembParser::ExportHash(const char* prefix, const char* hash_name, std::map<std::string, std::string>& vals)
{
	if (!perl) {
		return;
	}

	try {
		perl->sethash(
			fmt::format(
				"{}::{}",
				prefix,
				hash_name
			).c_str(),
			vals
		);
	} catch (std::string e) {
		AddError(
			fmt::format(
				"Error exporting Perl hash [{}]",
				e
			)
		);
	}
}

void PerlembParser::ExportVar(const char* prefix, const char* variable_name, int value)
{
	if (!perl) {
		return;
	}

	try {
		perl->seti(
			fmt::format(
				"{}::{}",
				prefix,
				variable_name
			).c_str(),
			value
		);
	} catch (std::string e) {
		AddError(
			fmt::format(
				"Error exporting Perl variable [{}]",
				e
			)
		);
	}
}

void PerlembParser::ExportVar(const char* prefix, const char* variable_name, unsigned int value)
{
	if (!perl) {
		return;
	}

	try {
		perl->seti(
			fmt::format(
				"{}::{}",
				prefix,
				variable_name
			).c_str(),
			value
		);
	} catch (std::string e) {
		AddError(
			fmt::format(
				"Error exporting Perl variable [{}]",
				e
			)
		);
	}
}

void PerlembParser::ExportVar(const char* prefix, const char* variable_name, float value)
{
	if (!perl) {
		return;
	}

	try {
		perl->setd(
			fmt::format(
				"{}::{}",
				prefix,
				variable_name
			).c_str(),
			value
		);
	} catch (std::string e) {
		AddError(
			fmt::format(
				"Error exporting Perl variable [{}]",
				e
			)
		);
	}
}

void PerlembParser::ExportVar(const char* prefix, const char* variable_name, const char* value)
{
	if (!perl) {
		return;
	}

	try {
		perl->setstr(
			fmt::format(
				"{}::{}",
				prefix,
				variable_name
			).c_str(),
			value
		);
	} catch (std::string e) {
		AddError(
			fmt::format(
				"Error exporting Perl variable [{}]",
				e
			)
		);
	}
}


void PerlembParser::ExportVar(const char* prefix, const char* variable_name, const char* class_name, void* value)
{
	if (!perl) {
		return;
	}

	try {
		perl->setptr(
			fmt::format(
				"{}::{}",
				prefix,
				variable_name
			).c_str(),
			class_name,
			value
		);
	} catch (std::string e) {
		AddError(fmt::format("Error exporting Perl variable [{}]", e));
	}
}

int PerlembParser::SendCommands(
	const char* prefix,
	const char* event_id,
	uint32 object_id,
	Mob* other,
	Mob* mob,
	EQ::ItemInstance* inst,
	const SPDat_Spell_Struct* spell,
	Zone* zone
)
{
	if (!perl) {
		return 0;
	}

	int ret_value = 0;
	RunningQuest q;

	q.owner      = other;
	q.questitem  = inst;
	q.questspell = spell;

	if (mob && mob->IsClient()) {
		q.initiator  = mob->CastToClient();
	}

	if (zone) {
		q.zone = zone;
	}

	quest_manager.StartQuest(q);

	try {
		perl->eval(fmt::format("package {};", prefix).c_str());

#ifdef EMBPERL_XS_CLASSES
		dTHX;
		{
			const std::vector<std::string>& suffixes = {
				"bot",
				"client",
				"entity_list",
				"merc",
				"npc",
				"questitem",
				"spell",
				"zone"
			};

			for (const auto& suffix : suffixes) {
				const std::string& key = fmt::format("${}::{}", prefix, suffix);
				if (clear_vars_.find(suffix) != clear_vars_.end()) {
					auto e = fmt::format("{} = undef;", key);
					perl->eval(e.c_str());
				}
			}
		}

		std::string buf;

		//init a couple special vars: client, npc, entity_list
		Client* c = quest_manager.GetInitiator();
		buf = fmt::format("{}::client", prefix);
		SV* client = get_sv(buf.c_str(), true);
		if (c) {
			sv_setref_pv(client, "Client", c);
		} else {
			//clear out the value, mainly to get rid of blessedness
			sv_setsv(client, _empty_sv);
		}

		if (other) {
			if (other->IsBot()) {
				Bot* b = quest_manager.GetBot();
				buf = fmt::format("{}::bot", prefix);
				SV* bot = get_sv(buf.c_str(), true);
				sv_setref_pv(bot, "Bot", b);
			} else if (other->IsMerc()) {
				Merc* m = quest_manager.GetMerc();
				buf = fmt::format("{}::merc", prefix);
				SV* merc = get_sv(buf.c_str(), true);
				sv_setref_pv(merc, "Merc", m);
			} else if (other->IsNPC()) {
				NPC* n = quest_manager.GetNPC();
				buf = fmt::format("{}::npc", prefix);
				SV* npc = get_sv(buf.c_str(), true);
				sv_setref_pv(npc, "NPC", n);
			}
		}

		//only export QuestItem if it's an inst quest
		if (inst) {
			auto i = quest_manager.GetQuestItem();
			buf = fmt::format("{}::questitem", prefix);
			SV* questitem = get_sv(buf.c_str(), true);
			sv_setref_pv(questitem, "QuestItem", i);
		}

		if (spell) {
			const auto current_spell = quest_manager.GetQuestSpell();
			auto       real_spell    = const_cast<SPDat_Spell_Struct*>(current_spell);
			buf = fmt::format("{}::spell", prefix);
			SV* spell = get_sv(buf.c_str(), true);
			sv_setref_pv(spell, "Spell", (void*) real_spell);
		}


		buf = fmt::format("{}::entity_list", prefix);
		SV* el = get_sv(buf.c_str(), true);
		sv_setref_pv(el, "EntityList", &entity_list);
#endif

		//now call the requested sub
		const std::string& sub_key = fmt::format("{}::{}", prefix, event_id);
		ret_value = perl->dosub(sub_key.c_str());

#ifdef EMBPERL_XS_CLASSES
		{
			const std::vector<std::string>& suffixes = {
				"bot",
				"client",
				"entity_list",
				"merc",
				"npc",
				"questitem",
				"spell",
				"zone"
			};

			for (const auto& suffix : suffixes) {
				const std::string& key = fmt::format("${}::{}", prefix, suffix);
				clear_vars_[key] = 1;
			}
		}
#endif

	} catch (std::string e) {
		AddError(
			fmt::format(
				"Script Error | Package [{}] Event [{}] Error [{}]",
				prefix,
				event_id,
				Strings::Trim(e)
			)
		);
	}

	quest_manager.EndQuest();

#ifdef EMBPERL_XS_CLASSES
	if (!quest_manager.QuestsRunning()) {
		std::string eval_str;
		for (const auto& v: clear_vars_) {
			eval_str += fmt::format("{} = undef;", v.first);
		}

		clear_vars_.clear();

		try {
			perl->eval(eval_str.c_str());
		} catch (std::string e) {
			AddError(
				fmt::format(
					"Script Clear Error | Error [{}]",
					e
				)
			);
		}
	}
#endif

	return ret_value;
}

void PerlembParser::MapFunctions()
{
	dTHX;
	_empty_sv = newSV(0);

	perl_register_quest();
#ifdef EMBPERL_XS_CLASSES
	perl_register_mob();
	perl_register_npc();
	perl_register_client();
	perl_register_corpse();
	perl_register_entitylist();
	perl_register_perlpacket();
	perl_register_group();
	perl_register_raid();
	perl_register_inventory();
	perl_register_questitem();
	perl_register_questitem_data();
	perl_register_spawn();
	perl_register_spell();
	perl_register_stat_bonuses();
	perl_register_hateentry();
	perl_register_object();
	perl_register_doors();
	perl_register_expedition();
	perl_register_expedition_lock_messages();
	perl_register_bot();
	perl_register_buff();
	perl_register_merc();
	perl_register_database();
	perl_register_zone();
#endif // EMBPERL_XS_CLASSES
}

QuestType PerlembParser::GetQuestTypes(
	QuestEventID event_id,
	Mob* npc_mob,
	EQ::ItemInstance* inst,
	Mob* mob,
	Zone* zone,
	bool is_global
)
{
	if (
		event_id == EVENT_SPELL_EFFECT_CLIENT ||
		event_id == EVENT_SPELL_EFFECT_NPC ||
		event_id == EVENT_SPELL_EFFECT_BOT ||
		event_id == EVENT_SPELL_EFFECT_BUFF_TIC_CLIENT ||
		event_id == EVENT_SPELL_EFFECT_BUFF_TIC_NPC ||
		event_id == EVENT_SPELL_EFFECT_BUFF_TIC_BOT ||
		event_id == EVENT_SPELL_FADE ||
		event_id == EVENT_SPELL_EFFECT_TRANSLOCATE_COMPLETE
	) {
		return is_global ? QuestType::SpellGlobal : QuestType::Spell;
	} else {
		if (npc_mob) {
			if (!inst) {
				if (npc_mob->IsBot()) {
					return is_global ? QuestType::BotGlobal : QuestType::Bot;
				} else if (npc_mob->IsMerc()) {
					return is_global ? QuestType::MercGlobal : QuestType::Merc;
				} else if (npc_mob->IsNPC()) {
					return is_global ? QuestType::NPCGlobal : QuestType::NPC;
				}
			} else {
				return is_global ? QuestType::ItemGlobal : QuestType::Item;
			}
		} else if (!npc_mob && mob) {
			if (!inst) {
				if (mob->IsClient()) {
					return is_global ? QuestType::PlayerGlobal : QuestType::Player;
				}
			} else {
				return is_global ? QuestType::ItemGlobal : QuestType::Item;
			}
		} else if (zone) {
			return is_global ? QuestType::ZoneGlobal : QuestType::Zone;
		}
	}
}

std::string PerlembParser::GetQuestPackageName(
	QuestType quest_type,
	QuestEventID event_id,
	uint32 object_id,
	const char* data,
	Mob* npc_mob,
	EQ::ItemInstance* inst
)
{
	if (quest_type == QuestType::NPC) {
		return fmt::format("qst_npc_{}", npc_mob->GetNPCTypeID());
	} else if (quest_type == QuestType::NPCGlobal) {
		return "qst_global_npc";
	} else if (quest_type == QuestType::Item || quest_type == QuestType::ItemGlobal) {
		if (!inst) {
			return "";
		}

		return fmt::format("qst_item_{}", inst->GetID());
	} else if (quest_type == QuestType::Player) {
		return "qst_player";
	} else if (quest_type == QuestType::PlayerGlobal) {
		return "qst_global_player";
	} else if (quest_type == QuestType::Bot) {
		return "qst_bot";
	} else if (quest_type == QuestType::BotGlobal) {
		return "qst_global_bot";
	} else if (quest_type == QuestType::Merc) {
		return "qst_merc";
	} else if (quest_type == QuestType::MercGlobal) {
		return "qst_global_merc";
	} else if (quest_type == QuestType::Spell || quest_type == QuestType::SpellGlobal) {
		return fmt::format("qst_spell_{}", object_id);
	} else if (quest_type == QuestType::Zone) {
		return "qst_zone";
	} else if (quest_type == QuestType::ZoneGlobal) {
		return "qst_global_zone";
	}

	return "";
}

void PerlembParser::ExportCharID(const std::string& package_name, int& char_id, Mob* npc_mob, Mob* mob)
{
	if (mob && mob->IsClient()) {  // some events like waypoint and spawn don't have a player involved
		char_id = mob->CastToClient()->CharacterID();
	} else {
		if (npc_mob) {
			char_id = -static_cast<int>(npc_mob->GetNPCTypeID());  // make char id negative npc id as a fudge
		} else if (mob && mob->IsNPC()) {
			char_id = -static_cast<int>(mob->CastToNPC()->GetNPCTypeID());  // make char id negative npc id as a fudge
		}
	}

	ExportVar(package_name.c_str(), "charid", char_id);
}

void PerlembParser::ExportQGlobals(
	QuestType quest_type,
	std::string& package_name,
	Mob* npc_mob,
	Mob* mob,
	int char_id
)
{
	//NPC quest
	if (quest_type == QuestType::NPC || quest_type == QuestType::NPCGlobal) {
		//only export for npcs that are global enabled.
		if (npc_mob && npc_mob->GetQglobal()) {
			std::map<std::string, std::string> globhash;
			QGlobalCache* npc_c  = nullptr;
			QGlobalCache* char_c = nullptr;
			QGlobalCache* zone_c = nullptr;

			//retrieve our globals
			if (npc_mob) {
				if (npc_mob->IsNPC()) {
					npc_c = npc_mob->CastToNPC()->GetQGlobals();
				} else if (npc_mob->IsClient()) {
					char_c = npc_mob->CastToClient()->GetQGlobals();
				}
			}

			if (mob && mob->IsClient()) {
				char_c = mob->CastToClient()->GetQGlobals();
			}

			zone_c = zone->GetQGlobals();

			if (!npc_c) {
				if (npc_mob && npc_mob->IsNPC()) {
					npc_c = npc_mob->CastToNPC()->CreateQGlobals();
					npc_c->LoadByNPCID(npc_mob->GetNPCTypeID());
				}
			}

			if (!char_c) {
				if (mob && mob->IsClient()) {
					char_c = mob->CastToClient()->CreateQGlobals();
					char_c->LoadByCharID(mob->CastToClient()->CharacterID());
				}
			}

			if (!zone_c) {
				zone_c = zone->CreateQGlobals();
				zone_c->LoadByZoneID(zone->GetZoneID());
				zone_c->LoadByGlobalContext();
			}

			std::list<QGlobal> globalMap;
			if (npc_c) {
				QGlobalCache::Combine(
					globalMap,
					npc_c->GetBucket(),
					npc_mob->GetNPCTypeID(),
					char_id,
					zone->GetZoneID()
				);
			}

			if (char_c) {
				QGlobalCache::Combine(
					globalMap,
					char_c->GetBucket(),
					npc_mob->GetNPCTypeID(),
					char_id,
					zone->GetZoneID()
				);
			}

			if (zone_c) {
				QGlobalCache::Combine(
					globalMap,
					zone_c->GetBucket(),
					npc_mob->GetNPCTypeID(),
					char_id,
					zone->GetZoneID()
				);
			}

			auto iter = globalMap.begin();
			while (iter != globalMap.end()) {
				globhash[(*iter).name] = (*iter).value;
				ExportVar(package_name.c_str(), (*iter).name.c_str(), (*iter).value.c_str());
				++iter;
			}

			ExportHash(package_name.c_str(), "qglobals", globhash);
		}
	} else {
		std::map<std::string, std::string> globhash;
		QGlobalCache* char_c = nullptr;
		QGlobalCache* zone_c = nullptr;

		//retrieve our globals
		if (mob && mob->IsClient()) {
			char_c = mob->CastToClient()->GetQGlobals();
		}

		zone_c = zone->GetQGlobals();

		if (!char_c) {
			if (mob && mob->IsClient()) {
				char_c = mob->CastToClient()->CreateQGlobals();
				char_c->LoadByCharID(mob->CastToClient()->CharacterID());
			}
		}

		if (!zone_c) {
			zone_c = zone->CreateQGlobals();
			zone_c->LoadByZoneID(zone->GetZoneID());
			zone_c->LoadByGlobalContext();
		}

		std::list<QGlobal> globalMap;
		if (char_c) {
			QGlobalCache::Combine(globalMap, char_c->GetBucket(), 0, char_id, zone->GetZoneID());
		}

		if (zone_c) {
			QGlobalCache::Combine(globalMap, zone_c->GetBucket(), 0, char_id, zone->GetZoneID());
		}

		auto iter = globalMap.begin();
		while (iter != globalMap.end()) {
			globhash[(*iter).name] = (*iter).value;
			ExportVar(package_name.c_str(), (*iter).name.c_str(), (*iter).value.c_str());
			++iter;
		}

		ExportHash(package_name.c_str(), "qglobals", globhash);
	}
}

void PerlembParser::ExportMobVariables(
	QuestType quest_type,
	std::string& package_name,
	Mob* mob,
	Mob* npc_mob
)
{
	uint8 fac = 0;
	if (mob && mob->IsClient()) {
		ExportVar(package_name.c_str(), "uguild_id", mob->CastToClient()->GuildID());
		ExportVar(package_name.c_str(), "uguildrank", mob->CastToClient()->GuildRank());
		ExportVar(package_name.c_str(), "status", mob->CastToClient()->Admin());
	}

	if (mob && mob->IsBot()) {
		ExportVar(package_name.c_str(), "bot_id", mob->CastToBot()->GetBotID());
		ExportVar(package_name.c_str(), "bot_owner_char_id", mob->CastToBot()->GetBotOwnerCharacterID());
	}

	if (quest_type == QuestType::NPC || quest_type == QuestType::NPCGlobal) {
		if (mob && mob->IsClient() && npc_mob && npc_mob->IsNPC()) {
			Client* c = mob->CastToClient();

			fac = c->GetFactionLevel(
				c->CharacterID(),
				npc_mob->GetID(),
				c->GetFactionRace(),
				c->GetClass(),
				c->GetDeity(),
				npc_mob->GetPrimaryFaction(),
				npc_mob
			);
		}
	}

	if (mob) {
		ExportVar(package_name.c_str(), "name", mob->GetName());
		ExportVar(package_name.c_str(), "race", GetRaceIDName(mob->GetRace()));
		ExportVar(package_name.c_str(), "class", GetClassIDName(mob->GetClass()));
		ExportVar(package_name.c_str(), "ulevel", mob->GetLevel());
		ExportVar(package_name.c_str(), "userid", mob->GetID());
	}

	if (quest_type == QuestType::NPC || quest_type == QuestType::NPCGlobal) {
		if (npc_mob->IsNPC()) {
			ExportVar(package_name.c_str(), "mname", npc_mob->GetName());
			ExportVar(package_name.c_str(), "mobid", npc_mob->GetID());
			ExportVar(package_name.c_str(), "mlevel", npc_mob->GetLevel());
			ExportVar(package_name.c_str(), "hpratio", npc_mob->GetHPRatio());
			ExportVar(package_name.c_str(), "x", npc_mob->GetX());
			ExportVar(package_name.c_str(), "y", npc_mob->GetY());
			ExportVar(package_name.c_str(), "z", npc_mob->GetZ());
			ExportVar(package_name.c_str(), "h", npc_mob->GetHeading());
			if (npc_mob->GetTarget()) {
				ExportVar(package_name.c_str(), "targetid", npc_mob->GetTarget()->GetID());
				ExportVar(package_name.c_str(), "targetname", npc_mob->GetTarget()->GetName());
			}
		}

		if (fac) {
			ExportVar(package_name.c_str(), "faction", itoa(fac));
		}
	}
}

void PerlembParser::ExportZoneVariables(std::string& package_name)
{
	if (zone) {
		ExportVar(package_name.c_str(), "instanceid", zone->GetInstanceID());
		ExportVar(package_name.c_str(), "instanceversion", zone->GetInstanceVersion());
		TimeOfDay_Struct eqTime{ };
		zone->zone_time.GetCurrentEQTimeOfDay(time(0), &eqTime);
		ExportVar(package_name.c_str(), "zone", "Zone", zone);
		ExportVar(package_name.c_str(), "zonehour", eqTime.hour - 1);
		ExportVar(package_name.c_str(), "zoneid", zone->GetZoneID());
		ExportVar(package_name.c_str(), "zoneln", zone->GetLongName());
		ExportVar(package_name.c_str(), "zonemin", eqTime.minute);
		ExportVar(package_name.c_str(), "zonesn", zone->GetShortName());
		ExportVar(package_name.c_str(), "zonetime", (eqTime.hour - 1) * 100 + eqTime.minute);
		ExportVar(package_name.c_str(), "zoneuptime", Timer::GetCurrentTime() / 1000);
		ExportVar(package_name.c_str(), "zoneweather", zone->zone_weather);
	}
}

void PerlembParser::ExportItemVariables(std::string& package_name, Mob* mob)
{
	if (mob && mob->IsClient()) {
		perl->eval(fmt::format("%{}::hasitem = ();", package_name).c_str());

		for (int slot = EQ::invslot::EQUIPMENT_BEGIN; slot <= EQ::invslot::GENERAL_END; slot++) {
			int item_id = mob->CastToClient()->GetItemIDAt(slot);
			if (item_id != -1 && item_id != 0) {
				auto hi_decl = fmt::format("push (@{{${0}::hasitem{{{1}}}}},{2});", package_name, item_id, slot);
				perl->eval(hi_decl.c_str());
			}
		}
	}

	if (mob && mob->IsClient()) {
		perl->eval(fmt::format("%{}::oncursor = ();", package_name).c_str());

		int item_id = mob->CastToClient()->GetItemIDAt(EQ::invslot::slotCursor);
		if (item_id != -1 && item_id != 0) {
			auto hi_decl = fmt::format("push (@{{${0}::oncursor{{{1}}}}},{2});", package_name, item_id, EQ::invslot::slotCursor);
			perl->eval(hi_decl.c_str());
		}
	}
}

void PerlembParser::ExportEventVariables(
	std::string& package_name,
	QuestEventID event_id,
	uint32 object_id,
	const char* data,
	Mob* npc_mob,
	EQ::ItemInstance* inst,
	Mob* mob,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	switch (event_id) {
		case EVENT_SAY: {
			if (npc_mob && npc_mob->IsNPC() && mob) {
				npc_mob->CastToNPC()->DoQuestPause(mob);
			}

			ExportVar(package_name.c_str(), "data", object_id);
			ExportVar(package_name.c_str(), "text", data);
			ExportVar(package_name.c_str(), "langid", extra_data);
			break;
		}

		case EVENT_TRADE: {
			if (extra_pointers) {
				size_t      sz = extra_pointers->size();
				for (size_t i  = 0; i < sz; ++i) {
					auto* inst = std::any_cast<EQ::ItemInstance*>(extra_pointers->at(i));
					const uint32 item_id      = inst ? inst->GetItem()->ID : 0;
					const int16  item_charges = inst ? inst->GetCharges() : 0;
					const auto   is_attuned   = inst ? inst->IsAttuned() : false;

					auto var_name = fmt::format("item{}", i + 1);
					ExportVar(package_name.c_str(), var_name.c_str(), item_id);

					auto temp_var_name = fmt::format("{}_charges", var_name);
					ExportVar(package_name.c_str(), temp_var_name.c_str(), item_charges);

					temp_var_name = fmt::format("{}_attuned", var_name);
					ExportVar(package_name.c_str(), temp_var_name.c_str(), is_attuned);

					temp_var_name = fmt::format("{}_inst", var_name);
					if (inst) {
						ExportVar(package_name.c_str(), temp_var_name.c_str(), "QuestItem", inst);
					} else {
						ExportVar(package_name.c_str(), temp_var_name.c_str(), 0);
					}
				}
			}

			auto unique_id = npc_mob->GetNPCTypeID();
			if (npc_mob->IsBot()) {
				unique_id = npc_mob->CastToBot()->GetBotID();
			}

			ExportVar(package_name.c_str(), "copper", GetVar(fmt::format("copper.{}", unique_id)).c_str());
			ExportVar(package_name.c_str(), "silver", GetVar(fmt::format("silver.{}", unique_id)).c_str());
			ExportVar(package_name.c_str(), "gold", GetVar(fmt::format("gold.{}", unique_id)).c_str());
			ExportVar(package_name.c_str(), "platinum", GetVar(fmt::format("platinum.{}", unique_id)).c_str());

			auto hash_name = fmt::format("{}::itemcount", package_name);
			perl->eval(fmt::format("%{} = ();", hash_name).c_str());
			perl->eval(fmt::format("++${}{{${}::item1}};", hash_name, package_name).c_str());
			perl->eval(fmt::format("++${}{{${}::item2}};", hash_name, package_name).c_str());
			perl->eval(fmt::format("++${}{{${}::item3}};", hash_name, package_name).c_str());
			perl->eval(fmt::format("++${}{{${}::item4}};", hash_name, package_name).c_str());

			if (npc_mob->IsBot()) {
				perl->eval(fmt::format("++${}{{${}::item5}};", hash_name, package_name).c_str());
				perl->eval(fmt::format("++${}{{${}::item6}};", hash_name, package_name).c_str());
				perl->eval(fmt::format("++${}{{${}::item7}};", hash_name, package_name).c_str());
				perl->eval(fmt::format("++${}{{${}::item8}};", hash_name, package_name).c_str());
			}

			break;
		}

		case EVENT_TARGET_CHANGE: {
			if (extra_pointers && extra_pointers->size() == 1) {
				ExportVar(package_name.c_str(), "target", "Mob", std::any_cast<Mob*>(extra_pointers->at(0)));
			}

			break;
		}

		case EVENT_WAYPOINT_ARRIVE:
		case EVENT_WAYPOINT_DEPART: {
			ExportVar(package_name.c_str(), "wp", data);
			break;
		}

		case EVENT_HP: {
			ExportVar(package_name.c_str(), "hpevent", extra_data ? "-1" : data);
			ExportVar(package_name.c_str(), "inchpevent", extra_data ? data : "-1");
			break;
		}

		case EVENT_TIMER:
		case EVENT_TIMER_STOP: {
			ExportVar(package_name.c_str(), "timer", data);
			break;
		}

		case EVENT_TIMER_PAUSE:
		case EVENT_TIMER_RESUME:
		case EVENT_TIMER_START: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "timer", sep.arg[0]);
			ExportVar(package_name.c_str(), "duration", sep.arg[1]);
			break;
		}

		case EVENT_SIGNAL: {
			ExportVar(package_name.c_str(), "signal", data);
			break;
		}

		case EVENT_PAYLOAD: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "payload_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "payload_value", sep.argplus[1]);
			break;
		}

		case EVENT_NPC_SLAY: {
			ExportVar(package_name.c_str(), "killed", mob->GetNPCTypeID());
			ExportVar(package_name.c_str(), "killed_npc", "NPC", mob->CastToNPC());
			break;
		}

		case EVENT_COMBAT: {
			ExportVar(package_name.c_str(), "combat_state", data);
			break;
		}

		case EVENT_CLICK_DOOR: {
			ExportVar(package_name.c_str(), "doorid", data);
			ExportVar(package_name.c_str(), "version", zone->GetInstanceVersion());

			if (extra_pointers && extra_pointers->size() >= 1) {
				ExportVar(package_name.c_str(), "door", "Doors", std::any_cast<Doors*>(extra_pointers->at(0)));
			}

			if (extra_pointers && extra_pointers->size() == 2) {
				ExportVar(package_name.c_str(), "player", "Client", std::any_cast<Client*>(extra_pointers->at(1)));
			}

			break;
		}

		case EVENT_LOOT_ZONE:
		case EVENT_LOOT: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "looted_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "looted_charges", sep.arg[1]);
			ExportVar(package_name.c_str(), "corpse_name", sep.arg[2]);
			ExportVar(package_name.c_str(), "corpse_id", sep.arg[3]);

			if (extra_pointers && extra_pointers->size() >= 1) {
				ExportVar(
					package_name.c_str(),
					"item",
					"QuestItem",
					std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0))
				);
			}

			if (extra_pointers && extra_pointers->size() >= 2) {
				ExportVar(package_name.c_str(), "corpse", "Corpse", std::any_cast<Corpse*>(extra_pointers->at(1)));
			}

			if (extra_pointers && extra_pointers->size() == 3) {
				ExportVar(package_name.c_str(), "player", "Client", std::any_cast<Client*>(extra_pointers->at(2)));
			}

			break;
		}

		case EVENT_ZONE: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "from_zone_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "from_instance_id", sep.arg[1]);
			ExportVar(package_name.c_str(), "from_instance_version", sep.arg[2]);
			ExportVar(package_name.c_str(), "target_zone_id", sep.arg[3]);
			ExportVar(package_name.c_str(), "target_instance_id", sep.arg[4]);
			ExportVar(package_name.c_str(), "target_instance_version", sep.arg[5]);
			break;
		}

		case EVENT_CAST_ON:
		case EVENT_CAST:
		case EVENT_CAST_BEGIN: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "spell_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "caster_id", sep.arg[1]);
			ExportVar(package_name.c_str(), "caster_level", sep.arg[2]);
			ExportVar(package_name.c_str(), "target_id", sep.arg[3]);

			if (extra_pointers && extra_pointers->size() == 1) {
				ExportVar(package_name.c_str(), "target", "Mob", std::any_cast<Mob*>(extra_pointers->at(0)));
			}

			if (IsValidSpell(Strings::ToUnsignedInt(sep.arg[0]))) {
				ExportVar(package_name.c_str(), "spell", "Spell", (void*) &spells[Strings::ToUnsignedInt(sep.arg[0])]);
			}

			break;
		}

		case EVENT_TASK_ACCEPTED: {
			ExportVar(package_name.c_str(), "task_id", data);
			break;
		}

		case EVENT_TASK_STAGE_COMPLETE: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "task_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "activity_id", sep.arg[1]);
			break;
		}

		case EVENT_TASK_FAIL: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "task_id", sep.arg[0]);
			break;
		}

		case EVENT_TASK_COMPLETE:
		case EVENT_TASK_BEFORE_UPDATE:
		case EVENT_TASK_UPDATE: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "donecount", sep.arg[0]);
			ExportVar(package_name.c_str(), "activity_id", sep.arg[1]);
			ExportVar(package_name.c_str(), "task_id", sep.arg[2]);
			break;
		}

		case EVENT_PLAYER_PICKUP: {
			ExportVar(package_name.c_str(), "picked_up_id", data);
			ExportVar(package_name.c_str(), "picked_up_entity_id", extra_data);

			if (extra_pointers && extra_pointers->size() >= 1) {
				ExportVar(
					package_name.c_str(),
					"item",
					"QuestItem",
					std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0))
				);
			}

			if (extra_pointers && extra_pointers->size() == 2) {
				ExportVar(package_name.c_str(), "player", "Client", std::any_cast<Client*>(extra_pointers->at(1)));
			}

			break;
		}

		case EVENT_AGGRO_SAY: {
			ExportVar(package_name.c_str(), "data", object_id);
			ExportVar(package_name.c_str(), "text", data);
			ExportVar(package_name.c_str(), "langid", extra_data);
			break;
		}

		case EVENT_POPUP_RESPONSE: {
			ExportVar(package_name.c_str(), "popupid", data);

			if (extra_pointers && extra_pointers->size() == 1) {
				ExportVar(package_name.c_str(), "player", "Client", std::any_cast<Client*>(extra_pointers->at(0)));
			}

			break;
		}

		case EVENT_ENVIRONMENTAL_DAMAGE: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "env_damage", sep.arg[0]);
			ExportVar(package_name.c_str(), "env_damage_type", sep.arg[1]);
			ExportVar(package_name.c_str(), "env_final_damage", sep.arg[2]);
			break;
		}

		case EVENT_PROXIMITY_SAY: {
			ExportVar(package_name.c_str(), "data", object_id);
			ExportVar(package_name.c_str(), "text", data);
			ExportVar(package_name.c_str(), "langid", extra_data);
			break;
		}

		case EVENT_SCALE_CALC:
		case EVENT_ITEM_ENTER_ZONE: {
			// need a valid EQ::ItemInstance pointer check here..unsure how to cancel this process
			ExportVar(package_name.c_str(), "itemid", object_id);
			ExportVar(package_name.c_str(), "itemname", inst->GetItem()->Name);
			break;
		}

		case EVENT_ITEM_CLICK_CAST:
		case EVENT_ITEM_CLICK: {
			// need a valid EQ::ItemInstance pointer check here..unsure how to cancel this process
			ExportVar(package_name.c_str(), "itemid", object_id);
			ExportVar(package_name.c_str(), "itemname", inst->GetItem()->Name);
			ExportVar(package_name.c_str(), "slotid", extra_data);
			ExportVar(package_name.c_str(), "spell_id", inst->GetItem()->Click.Effect);

			if (IsValidSpell(inst->GetItem()->Click.Effect)) {
				ExportVar(package_name.c_str(), "spell", "Spell", (void*) &spells[inst->GetItem()->Click.Effect]);
			}

			break;
		}

		case EVENT_ITEM_CLICK_CAST_CLIENT:
		case EVENT_ITEM_CLICK_CLIENT: {
			ExportVar(package_name.c_str(), "slot_id", data);

			if (extra_pointers && extra_pointers->size() == 1) {
				auto* inst = std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0));
				if (inst) {
					ExportVar(package_name.c_str(), "item_id", inst->GetID());
					ExportVar(package_name.c_str(), "item_name", inst->GetItem()->Name);
					ExportVar(package_name.c_str(), "spell_id", inst->GetItem()->Click.Effect);
					ExportVar(package_name.c_str(), "item", "QuestItem", inst);

					if (IsValidSpell(inst->GetItem()->Click.Effect)) {
						ExportVar(
							package_name.c_str(),
							"spell",
							"Spell",
							(void*) &spells[inst->GetItem()->Click.Effect]
						);
					}
				}
			}

			break;
		}

		case EVENT_GROUP_CHANGE: {
			if (mob && mob->IsClient()) {
				ExportVar(package_name.c_str(), "grouped", mob->IsGrouped());
				ExportVar(package_name.c_str(), "raided", mob->IsRaidGrouped());
			}
			break;
		}

		case EVENT_HATE_LIST: {
			ExportVar(package_name.c_str(), "hate_state", data);
			ExportVar(package_name.c_str(), "hate_entity", "Mob", mob);
			break;
		}

		case EVENT_SPELL_EFFECT_BUFF_TIC_BOT:
		case EVENT_SPELL_EFFECT_BUFF_TIC_CLIENT:
		case EVENT_SPELL_EFFECT_BUFF_TIC_NPC:
		case EVENT_SPELL_EFFECT_BOT:
		case EVENT_SPELL_EFFECT_CLIENT:
		case EVENT_SPELL_EFFECT_NPC:
		case EVENT_SPELL_FADE: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "spell_id", object_id);
			ExportVar(package_name.c_str(), "caster_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "tics_remaining", sep.arg[1]);
			ExportVar(package_name.c_str(), "caster_level", sep.arg[2]);
			ExportVar(package_name.c_str(), "buff_slot", sep.arg[3]);

			if (IsValidSpell(object_id)) {
				ExportVar(package_name.c_str(), "spell", "Spell", (void*) &spells[object_id]);
			}

			break;
		}

		case EVENT_SPELL_BLOCKED: {
			Seperator sep(data);
			const uint32 blocking_spell_id = Strings::ToUnsignedInt(sep.arg[0]);
			const uint32 cast_spell_id = Strings::ToUnsignedInt(sep.arg[1]);

			ExportVar(package_name.c_str(), "blocking_spell_id", blocking_spell_id);
			ExportVar(package_name.c_str(), "cast_spell_id", cast_spell_id);

			if (IsValidSpell(blocking_spell_id)) {
				ExportVar(package_name.c_str(), "blocking_spell", "Spell", (void*) &spells[blocking_spell_id]);
			}

			if (IsValidSpell(cast_spell_id)) {
				ExportVar(package_name.c_str(), "cast_spell", "Spell", (void*) &spells[cast_spell_id]);
			}

			break;
		}

			//tradeskill events
		case EVENT_COMBINE_SUCCESS:
		case EVENT_COMBINE_FAILURE: {
			ExportVar(package_name.c_str(), "recipe_id", extra_data);
			ExportVar(package_name.c_str(), "recipe_name", data);
			break;
		}

		case EVENT_FORAGE_SUCCESS: {
			ExportVar(package_name.c_str(), "foraged_item", extra_data);

			if (extra_pointers && extra_pointers->size() == 1) {
				ExportVar(
					package_name.c_str(),
					"item",
					"QuestItem",
					std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0))
				);
			}

			break;
		}

		case EVENT_FISH_SUCCESS: {
			ExportVar(package_name.c_str(), "fished_item", extra_data);

			if (extra_pointers && extra_pointers->size() == 1) {
				ExportVar(
					package_name.c_str(),
					"item",
					"QuestItem",
					std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0))
				);
			}

			break;
		}

		case EVENT_CLICK_OBJECT: {
			ExportVar(package_name.c_str(), "objectid", data);
			ExportVar(package_name.c_str(), "clicker_id", extra_data);

			if (extra_pointers && extra_pointers->size() >= 1) {
				ExportVar(package_name.c_str(), "object", "Object", std::any_cast<Object*>(extra_pointers->at(0)));
			}

			if (extra_pointers && extra_pointers->size() == 2) {
				ExportVar(package_name.c_str(), "player", "Client", std::any_cast<Client*>(extra_pointers->at(1)));
			}

			break;
		}

		case EVENT_DISCOVER_ITEM: {
			ExportVar(package_name.c_str(), "itemid", extra_data);

			if (extra_pointers && extra_pointers->size() == 1) {
				ExportVar(
					package_name.c_str(),
					"item",
					"QuestItem",
					std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0))
				);
			}

			break;
		}

		case EVENT_COMMAND: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "command", (sep.arg[0] + 1));
			ExportVar(package_name.c_str(), "args", (sep.argnum >= 1 ? (&data[strlen(sep.arg[0]) + 1]) : "0"));
			ExportVar(package_name.c_str(), "data", object_id);
			ExportVar(package_name.c_str(), "text", data);
			ExportVar(package_name.c_str(), "langid", extra_data);
			break;
		}

		case EVENT_RESPAWN: {
			ExportVar(package_name.c_str(), "option", data);
			ExportVar(package_name.c_str(), "resurrect", extra_data);
			break;
		}

		case EVENT_DEATH_ZONE:
		case EVENT_DEATH:
		case EVENT_DEATH_COMPLETE: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "killer_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "killer_damage", sep.arg[1]);
			ExportVar(package_name.c_str(), "killer_spell", sep.arg[2]);
			ExportVar(package_name.c_str(), "killer_skill", sep.arg[3]);
			ExportVar(package_name.c_str(), "killed_entity_id", sep.arg[4]);

			if (sep.arg[5]) {
				ExportVar(package_name.c_str(), "combat_start_time", sep.arg[5]);
				ExportVar(package_name.c_str(), "combat_end_time", sep.arg[6]);
				ExportVar(package_name.c_str(), "damage_received", sep.arg[7]);
				ExportVar(package_name.c_str(), "healing_received", sep.arg[8]);
			}

			if (extra_pointers && extra_pointers->size() >= 1) {
				Corpse* corpse = std::any_cast<Corpse*>(extra_pointers->at(0));
				if (corpse) {
					ExportVar(package_name.c_str(), "killed_corpse_id", corpse->GetID());
					ExportVar(package_name.c_str(), "killed_x", corpse->GetX());
					ExportVar(package_name.c_str(), "killed_y", corpse->GetY());
					ExportVar(package_name.c_str(), "killed_z", corpse->GetZ());
					ExportVar(package_name.c_str(), "killed_h", corpse->GetHeading());
				}
			}

			// EVENT_DEATH_ZONE only
			if (extra_pointers && extra_pointers->size() >= 2) {
				NPC* killed = std::any_cast<NPC*>(extra_pointers->at(1));
				if (killed) {
					ExportVar(
						package_name.c_str(),
						"killed_bot_id",
						killed->IsBot() ? killed->CastToBot()->GetBotID() : 0
					);
					ExportVar(package_name.c_str(), "killed_merc_id", killed->IsMerc() ? killed->CastToMerc()->GetMercenaryID() : 0);
					ExportVar(package_name.c_str(), "killed_npc_id", !killed->IsMerc() && killed->IsNPC() ? killed->GetNPCTypeID() : 0);
				}
			}

			if (extra_pointers && extra_pointers->size() == 3) {
				Mob* killer = std::any_cast<Mob*>(extra_pointers->at(2));
				if (killer) {
					ExportVar(package_name.c_str(), "killer", "Mob", killer);
				}
			}
			break;
		}

		case EVENT_DROP_ITEM: {
			ExportVar(package_name.c_str(), "quantity", inst->IsStackable() ? inst->GetCharges() : 1);
			ExportVar(package_name.c_str(), "itemname", inst->GetItem()->Name);
			ExportVar(package_name.c_str(), "itemid", inst->GetItem()->ID);
			ExportVar(package_name.c_str(), "spell_id", inst->GetItem()->Click.Effect);
			ExportVar(package_name.c_str(), "slotid", extra_data);
			break;
		}

		case EVENT_DROP_ITEM_CLIENT: {
			if (extra_pointers && extra_pointers->size() == 1) {
				auto item_instance = std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0));
				ExportVar(package_name.c_str(), "quantity", item_instance->IsStackable() ? item_instance->GetCharges() : 1);
				ExportVar(package_name.c_str(), "item_name", item_instance->GetItem()->Name);
				ExportVar(package_name.c_str(), "item_id", item_instance->GetItem()->ID);
				ExportVar(package_name.c_str(), "spell_id", item_instance->GetItem()->Click.Effect);
				ExportVar(package_name.c_str(), "slot_id", extra_data);
				ExportVar(package_name.c_str(), "item", "QuestItem", item_instance);
			}

			break;
		}

		case EVENT_SPAWN_ZONE: {
			if (mob) {
				ExportVar(package_name.c_str(), "spawned", "Mob", mob);
				ExportVar(package_name.c_str(), "spawned_bot_id", mob->IsBot() ? mob->CastToBot()->GetBotID() : 0);
				ExportVar(package_name.c_str(), "spawned_entity_id", mob->GetID());
				ExportVar(package_name.c_str(), "spawned_npc_id", mob->IsNPC() ? mob->GetNPCTypeID() : 0);
			}

			if (extra_pointers && extra_pointers->size() == 1) {
				NPC* spawn_npc = std::any_cast<NPC*>(extra_pointers->at(0));
				ExportVar(package_name.c_str(), "spawned", "NPC", spawn_npc);
				ExportVar(package_name.c_str(), "spawned_bot_id", spawn_npc->IsBot() ? spawn_npc->CastToBot()->GetBotID() : 0);
				ExportVar(package_name.c_str(), "spawned_entity_id", spawn_npc->GetID());
				ExportVar(package_name.c_str(), "spawned_npc_id", spawn_npc->IsNPC() ? spawn_npc->GetNPCTypeID() : 0);
			}

			break;
		}

		case EVENT_USE_SKILL: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "skill_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "skill_level", sep.arg[1]);
			break;
		}

		case EVENT_COMBINE_VALIDATE: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "recipe_id", extra_data);
			ExportVar(package_name.c_str(), "validate_type", sep.arg[0]);

			std::string zone_id       = "-1";
			std::string tradeskill_id = "-1";
			if (strcmp(sep.arg[0], "check_zone") == 0) {
				zone_id = sep.arg[1];
			} else if (strcmp(sep.arg[0], "check_tradeskill") == 0) {
				tradeskill_id = sep.arg[1];
			}

			ExportVar(package_name.c_str(), "zone_id", zone_id.c_str());
			ExportVar(package_name.c_str(), "tradeskill_id", tradeskill_id.c_str());
			break;
		}

		case EVENT_BOT_COMMAND: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "bot_command", (sep.arg[0] + 1));
			ExportVar(package_name.c_str(), "args", (sep.argnum >= 1 ? (&data[strlen(sep.arg[0]) + 1]) : "0"));
			ExportVar(package_name.c_str(), "data", object_id);
			ExportVar(package_name.c_str(), "text", data);
			ExportVar(package_name.c_str(), "langid", extra_data);
			break;
		}

		case EVENT_WARP: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "from_x", sep.arg[0]);
			ExportVar(package_name.c_str(), "from_y", sep.arg[1]);
			ExportVar(package_name.c_str(), "from_z", sep.arg[2]);
			break;
		}

		case EVENT_CONSIDER: {
			ExportVar(package_name.c_str(), "entity_id", Strings::ToInt(data));

			if (extra_pointers && extra_pointers->size() == 1) {
				ExportVar(package_name.c_str(), "target", "Mob", std::any_cast<Mob*>(extra_pointers->at(0)));
			}

			break;
		}

		case EVENT_CONSIDER_CORPSE: {
			ExportVar(package_name.c_str(), "corpse_entity_id", Strings::ToInt(data));

			if (extra_pointers && extra_pointers->size() == 1) {
				ExportVar(package_name.c_str(), "corpse", "Corpse", std::any_cast<Corpse*>(extra_pointers->at(0)));
			}

			break;
		}

		case EVENT_COMBINE: {
			ExportVar(package_name.c_str(), "container_slot", Strings::ToInt(data));
			break;
		}

		case EVENT_EQUIP_ITEM_CLIENT:
		case EVENT_UNEQUIP_ITEM_CLIENT: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "item_id", extra_data);
			ExportVar(package_name.c_str(), "item_quantity", sep.arg[0]);
			ExportVar(package_name.c_str(), "slot_id", sep.arg[1]);

			if (extra_pointers && extra_pointers->size() == 1) {
				ExportVar(package_name.c_str(), "item", "QuestItem", std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0)));
			}

			break;
		}

		case EVENT_EQUIP_ITEM_BOT:
		case EVENT_UNEQUIP_ITEM_BOT: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "item_id", extra_data);
			ExportVar(package_name.c_str(), "item_quantity", sep.arg[0]);
			ExportVar(package_name.c_str(), "slot_id", sep.arg[1]);

			if (extra_pointers && extra_pointers->size() == 1) {
				ExportVar(package_name.c_str(), "item", "QuestItem", std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0)));
			}

			break;
		}

		case EVENT_AUGMENT_INSERT_CLIENT: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "item_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "item_slot", sep.arg[1]);
			ExportVar(package_name.c_str(), "augment_id", sep.arg[2]);
			ExportVar(package_name.c_str(), "augment_slot", sep.arg[3]);

			if (extra_pointers && extra_pointers->size() >= 1) {
				ExportVar(package_name.c_str(), "item", "QuestItem", std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0)));
			}

			if (extra_pointers && extra_pointers->size() >= 2) {
				ExportVar(package_name.c_str(), "augment", "QuestItem", std::any_cast<EQ::ItemInstance*>(extra_pointers->at(1)));
			}

			break;
		}

		case EVENT_AUGMENT_REMOVE_CLIENT: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "item_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "item_slot", sep.arg[1]);
			ExportVar(package_name.c_str(), "augment_id", sep.arg[2]);
			ExportVar(package_name.c_str(), "augment_slot", sep.arg[3]);
			ExportVar(package_name.c_str(), "destroyed", sep.arg[4]);

			if (extra_pointers && extra_pointers->size() >= 1) {
				ExportVar(package_name.c_str(), "item", "QuestItem", std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0)));
			}

			if (extra_pointers && extra_pointers->size() >= 3) {
				ExportVar(package_name.c_str(), "augment", "QuestItem", std::any_cast<EQ::ItemInstance*>(extra_pointers->at(2)));
			}

			break;
		}

		case EVENT_SKILL_UP: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "skill_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "skill_value", sep.arg[1]);
			ExportVar(package_name.c_str(), "skill_max", sep.arg[2]);
			ExportVar(package_name.c_str(), "is_tradeskill", sep.arg[3]);
			break;
		}

		case EVENT_LANGUAGE_SKILL_UP: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "skill_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "skill_value", sep.arg[1]);
			ExportVar(package_name.c_str(), "skill_max", sep.arg[2]);
			break;
		}

		case EVENT_ALT_CURRENCY_MERCHANT_BUY:
		case EVENT_ALT_CURRENCY_MERCHANT_SELL: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "currency_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "npc_id", sep.arg[1]);
			ExportVar(package_name.c_str(), "merchant_id", sep.arg[2]);
			ExportVar(package_name.c_str(), "item_id", sep.arg[3]);
			ExportVar(package_name.c_str(), "item_cost", sep.arg[4]);
			break;
		}

		case EVENT_MERCHANT_BUY:
		case EVENT_MERCHANT_SELL: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "npc_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "merchant_id", sep.arg[1]);
			ExportVar(package_name.c_str(), "item_id", sep.arg[2]);
			ExportVar(package_name.c_str(), "item_quantity", sep.arg[3]);
			ExportVar(package_name.c_str(), "item_cost", sep.arg[4]);
			break;
		}

		case EVENT_AA_BUY: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "aa_cost", sep.arg[0]);
			ExportVar(package_name.c_str(), "aa_id", sep.arg[1]);
			ExportVar(package_name.c_str(), "aa_previous_id", sep.arg[2]);
			ExportVar(package_name.c_str(), "aa_next_id", sep.arg[3]);
			break;
		}

		case EVENT_AA_GAIN: {
			ExportVar(package_name.c_str(), "aa_gained", data);
			break;
		}

		case EVENT_AA_LOSS: {
			ExportVar(package_name.c_str(), "aa_lost", data);
			break;
		}

		case EVENT_AA_EXP_GAIN: {
			ExportVar(package_name.c_str(), "aa_exp_gained", data);
			break;
		}

		case EVENT_EXP_GAIN: {
			ExportVar(package_name.c_str(), "exp_gained", data);
			break;
		}

		case EVENT_INSPECT: {
			ExportVar(package_name.c_str(), "target_id", extra_data);

			if (extra_pointers && extra_pointers->size() == 1) {
				ExportVar(package_name.c_str(), "target", "Mob", std::any_cast<Mob*>(extra_pointers->at(0)));
			}

			break;
		}

		case EVENT_LEVEL_UP: {
			ExportVar(package_name.c_str(), "levels_gained", data);
			break;
		}

		case EVENT_LEVEL_DOWN: {
			ExportVar(package_name.c_str(), "levels_lost", data);
			break;
		}

		case EVENT_GM_COMMAND: {
			ExportVar(package_name.c_str(), "message", data);
			break;
		}

		case EVENT_ENTER_AREA:
		case EVENT_LEAVE_AREA: {
			if (extra_pointers && extra_pointers->size() >= 2) {
				ExportVar(package_name.c_str(), "area_id", *std::any_cast<int*>(extra_pointers->at(0)));
				ExportVar(package_name.c_str(), "area_type", *std::any_cast<int*>(extra_pointers->at(1)));
			}

			break;
		}

		case EVENT_DESPAWN: {
			ExportVar(package_name.c_str(), "despawned", "Mob", npc_mob);
			ExportVar(package_name.c_str(), "despawned_entity_id", npc_mob->GetID());
			ExportVar(package_name.c_str(), "despawned_bot_id", npc_mob->IsBot() ? npc_mob->CastToBot()->GetBotID() : 0);
			ExportVar(package_name.c_str(), "despawned_merc_id", npc_mob->IsMerc() ? npc_mob->CastToMerc()->GetMercenaryID() : 0);
			ExportVar(package_name.c_str(), "despawned_npc_id", npc_mob->IsNPC() ? npc_mob->GetNPCTypeID() : 0);
			break;
		}

		case EVENT_DESPAWN_ZONE: {
			if (mob) {
				ExportVar(package_name.c_str(), "despawned", "Mob", mob);
				ExportVar(package_name.c_str(), "despawned_bot_id", mob->IsBot() ? mob->CastToBot()->GetBotID() : 0);
				ExportVar(package_name.c_str(), "despawned_entity_id", mob->GetID());
				ExportVar(package_name.c_str(), "despawned_npc_id", mob->IsNPC() ? mob->GetNPCTypeID() : 0);
			}

			if (extra_pointers && extra_pointers->size() == 1) {
				NPC* spawn_npc = std::any_cast<NPC*>(extra_pointers->at(0));
				ExportVar(package_name.c_str(), "despawned", "NPC", spawn_npc);
				ExportVar(package_name.c_str(), "despawned_bot_id", spawn_npc->IsBot() ? spawn_npc->CastToBot()->GetBotID() : 0);
				ExportVar(package_name.c_str(), "despawned_entity_id", spawn_npc->GetID());
				ExportVar(package_name.c_str(), "despawned_npc_id", spawn_npc->IsNPC() ? spawn_npc->GetNPCTypeID() : 0);
			}

			break;
		}

		case EVENT_BOT_CREATE: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "bot_name", sep.arg[0]);
			ExportVar(package_name.c_str(), "bot_id", sep.arg[1]);
			ExportVar(package_name.c_str(), "bot_race", sep.arg[2]);
			ExportVar(package_name.c_str(), "bot_class", sep.arg[3]);
			ExportVar(package_name.c_str(), "bot_gender", sep.arg[4]);
			break;
		}

		case EVENT_DAMAGE_GIVEN:
		case EVENT_DAMAGE_TAKEN: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "entity_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "damage", sep.arg[1]);
			ExportVar(package_name.c_str(), "spell_id", sep.arg[2]);
			ExportVar(package_name.c_str(), "skill_id", sep.arg[3]);
			ExportVar(package_name.c_str(), "is_damage_shield", sep.arg[4]);
			ExportVar(package_name.c_str(), "is_avoidable", sep.arg[5]);
			ExportVar(package_name.c_str(), "buff_slot", sep.arg[6]);
			ExportVar(package_name.c_str(), "is_buff_tic", sep.arg[7]);
			ExportVar(package_name.c_str(), "special_attack", sep.arg[8]);

			if (IsValidSpell(Strings::ToUnsignedInt(sep.arg[2]))) {
				ExportVar(package_name.c_str(), "spell", "Spell", (void*) &spells[Strings::ToUnsignedInt(sep.arg[2])]);
			}

			break;
		}

		case EVENT_DESTROY_ITEM_CLIENT: {
			if (extra_pointers && extra_pointers->size() == 1) {
				auto inst = std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0));
				ExportVar(package_name.c_str(), "item_id", inst->GetID());
				ExportVar(package_name.c_str(), "item_name", inst->GetItem()->Name);
				ExportVar(package_name.c_str(), "quantity", inst->IsStackable() ? inst->GetCharges() : 1);
				ExportVar(package_name.c_str(), "item", "QuestItem", inst);
			}

			break;
		}

		case EVENT_MEMORIZE_SPELL:
		case EVENT_UNMEMORIZE_SPELL:
		case EVENT_SCRIBE_SPELL:
		case EVENT_UNSCRIBE_SPELL: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "slot_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "spell_id", sep.arg[1]);

			if (IsValidSpell(Strings::ToUnsignedInt(sep.arg[1]))) {
				ExportVar(package_name.c_str(), "spell", "Spell", (void*) &spells[Strings::ToUnsignedInt(sep.arg[1])]);
			}

			break;
		}

		case EVENT_LOOT_ADDED: {
			if (extra_pointers && extra_pointers->size() == 1) {
				auto inst = std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0));
				if (inst) {
					ExportVar(package_name.c_str(), "item", "QuestItem", inst);
					ExportVar(package_name.c_str(), "item_id", inst->GetID());
					ExportVar(package_name.c_str(), "item_name", inst->GetItem()->Name);
					ExportVar(package_name.c_str(), "item_charges", inst->GetCharges());
					ExportVar(package_name.c_str(), "augment_one", inst->GetAugmentItemID(EQ::invaug::SOCKET_BEGIN));
					ExportVar(package_name.c_str(), "augment_two", inst->GetAugmentItemID(EQ::invaug::SOCKET_BEGIN + 1));
					ExportVar(package_name.c_str(), "augment_three", inst->GetAugmentItemID(EQ::invaug::SOCKET_BEGIN + 2));
					ExportVar(package_name.c_str(), "augment_four", inst->GetAugmentItemID(EQ::invaug::SOCKET_BEGIN + 3));
					ExportVar(package_name.c_str(), "augment_five", inst->GetAugmentItemID(EQ::invaug::SOCKET_BEGIN + 4));
					ExportVar(package_name.c_str(), "augment_six", inst->GetAugmentItemID(EQ::invaug::SOCKET_END));
				}
			}
		}

		case EVENT_LDON_POINTS_GAIN:
		case EVENT_LDON_POINTS_LOSS: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "theme_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "points", sep.arg[1]);
			break;
		}

		case EVENT_ALT_CURRENCY_GAIN:
		case EVENT_ALT_CURRENCY_LOSS: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "currency_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "amount", sep.arg[1]);
			ExportVar(package_name.c_str(), "total", sep.arg[2]);
			break;
		}

		case EVENT_CRYSTAL_GAIN:
		case EVENT_CRYSTAL_LOSS: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "ebon_amount", sep.arg[0]);
			ExportVar(package_name.c_str(), "radiant_amount", sep.arg[1]);
			ExportVar(package_name.c_str(), "is_reclaim", sep.arg[2]);
			break;
		}

		case EVENT_ENTITY_VARIABLE_DELETE:
		case EVENT_ENTITY_VARIABLE_SET: {
			if (extra_pointers && extra_pointers->size() == 2) {
				ExportVar(package_name.c_str(), "variable_name", std::any_cast<std::string>(extra_pointers->at(0)).c_str());
				ExportVar(package_name.c_str(), "variable_value", std::any_cast<std::string>(extra_pointers->at(1)).c_str());
			}

			break;
		}

		case EVENT_ENTITY_VARIABLE_UPDATE: {
			if (extra_pointers && extra_pointers->size() == 3) {
				ExportVar(package_name.c_str(), "variable_name", std::any_cast<std::string>(extra_pointers->at(0)).c_str());
				ExportVar(package_name.c_str(), "old_value", std::any_cast<std::string>(extra_pointers->at(1)).c_str());
				ExportVar(package_name.c_str(), "new_value", std::any_cast<std::string>(extra_pointers->at(2)).c_str());
			}

			break;
		}

		case EVENT_READ_ITEM: {;
			ExportVar(package_name.c_str(), "item_id", extra_data);
			ExportVar(package_name.c_str(), "text_file", data);

			if (extra_pointers && extra_pointers->size() == 7) {
				ExportVar(package_name.c_str(), "book_text", std::any_cast<std::string>(extra_pointers->at(0)).c_str());
				ExportVar(package_name.c_str(), "can_cast", std::any_cast<int8>(extra_pointers->at(1)));
				ExportVar(package_name.c_str(), "can_scribe", std::any_cast<int8>(extra_pointers->at(2)));
				ExportVar(package_name.c_str(), "slot_id", std::any_cast<int16>(extra_pointers->at(3)));
				ExportVar(package_name.c_str(), "target_id", std::any_cast<int>(extra_pointers->at(4)));
				ExportVar(package_name.c_str(), "type", std::any_cast<uint8>(extra_pointers->at(5)));
				ExportVar(
					package_name.c_str(),
					"item",
					"QuestItem",
					std::any_cast<EQ::ItemInstance*>(extra_pointers->at(6))
				);
			}

			break;
		}

		case EVENT_CONNECT: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "last_login", sep.arg[0]);
			ExportVar(package_name.c_str(), "seconds_since_last_login", sep.arg[1]);
			ExportVar(package_name.c_str(), "is_first_login", sep.arg[2]);
			break;
		}

		case EVENT_ENTER_ZONE: {
			if (extra_pointers && extra_pointers->size() == 1) {
				ExportVar(package_name.c_str(), "player", "Client", std::any_cast<Client*>(extra_pointers->at(0)));
			}

			break;
		}

		case EVENT_PET_COMMAND: {
			ExportVar(package_name.c_str(), "pet_command", extra_data);
			ExportVar(package_name.c_str(), "pet_command_name", data);
			break;
		}

		default: {
			break;
		}
	}
}

void PerlembParser::LoadBotScript(std::string filename)
{
	if (!perl || bot_quest_status_ != questUnloaded) {
		return;
	}

	try {
		perl->eval_file("qst_bot", filename.c_str());
	} catch (std::string e) {
		AddError(
			fmt::format(
				"Error Compiling Bot Quest File [{}] Error [{}]",
				filename,
				e
			)
		);

		bot_quest_status_ = questFailedToLoad;
		return;
	}

	bot_quest_status_ = questLoaded;
}

void PerlembParser::LoadGlobalBotScript(std::string filename)
{
	if (!perl || global_bot_quest_status_ != questUnloaded) {
		return;
	}

	try {
		perl->eval_file("qst_global_bot", filename.c_str());
	} catch (std::string e) {
		AddError(
			fmt::format(
				"Error Compiling Global Bot Quest File [{}] Error [{}]",
				filename,
				e
			)
		);

		global_bot_quest_status_ = questFailedToLoad;
		return;
	}

	global_bot_quest_status_ = questLoaded;
}

bool PerlembParser::BotHasQuestSub(QuestEventID event_id)
{
	if (
		!perl ||
		bot_quest_status_ != questLoaded ||
		event_id >= _LargestEventID
	) {
		return false;
	}

	return perl->SubExists("qst_bot", QuestEventSubroutines[event_id]);
}

bool PerlembParser::GlobalBotHasQuestSub(QuestEventID event_id)
{
	if (
		!perl ||
		global_bot_quest_status_ != questLoaded ||
		event_id >= _LargestEventID
	) {
		return false;
	}

	return (perl->SubExists("qst_global_bot", QuestEventSubroutines[event_id]));
}

int PerlembParser::EventBot(
	QuestEventID event_id,
	Bot* bot,
	Mob* mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	return EventCommon(
		event_id,
		0,
		data.c_str(),
		bot,
		nullptr,
		nullptr,
		mob,
		nullptr,
		extra_data,
		false,
		extra_pointers
	);
}

int PerlembParser::EventGlobalBot(
	QuestEventID event_id,
	Bot* bot,
	Mob* mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	return EventCommon(
		event_id,
		0,
		data.c_str(),
		bot,
		nullptr,
		nullptr,
		mob,
		nullptr,
		extra_data,
		true,
		extra_pointers
	);
}

void PerlembParser::LoadMercScript(std::string filename)
{
	if (!perl || merc_quest_status_ != questUnloaded) {
		return;
	}

	try {
		perl->eval_file("qst_merc", filename.c_str());
	} catch (std::string e) {
		AddError(
			fmt::format(
				"Error Compiling Merc Quest File [{}] Error [{}]",
				filename,
				e
			)
		);

		merc_quest_status_ = questFailedToLoad;
		return;
	}

	merc_quest_status_ = questLoaded;
}

void PerlembParser::LoadGlobalMercScript(std::string filename)
{
	if (!perl || global_merc_quest_status_ != questUnloaded) {
		return;
	}

	try {
		perl->eval_file("qst_global_merc", filename.c_str());
	} catch (std::string e) {
		AddError(
			fmt::format(
				"Error Compiling Global Merc Quest File [{}] Error [{}]",
				filename,
				e
			)
		);

		global_merc_quest_status_ = questFailedToLoad;
		return;
	}

	global_merc_quest_status_ = questLoaded;
}

bool PerlembParser::MercHasQuestSub(QuestEventID event_id)
{
	if (
		!perl ||
		merc_quest_status_ != questLoaded ||
		event_id >= _LargestEventID
	) {
		return false;
	}

	return perl->SubExists("qst_merc", QuestEventSubroutines[event_id]);
}

bool PerlembParser::GlobalMercHasQuestSub(QuestEventID event_id)
{
	if (
		!perl ||
		global_merc_quest_status_ != questLoaded ||
		event_id >= _LargestEventID
		) {
		return false;
	}

	return (perl->SubExists("qst_global_merc", QuestEventSubroutines[event_id]));
}

int PerlembParser::EventMerc(
	QuestEventID event_id,
	Merc* merc,
	Mob* mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	return EventCommon(
		event_id,
		0,
		data.c_str(),
		merc,
		nullptr,
		nullptr,
		mob,
		nullptr,
		extra_data,
		false,
		extra_pointers
	);
}

int PerlembParser::EventGlobalMerc(
	QuestEventID event_id,
	Merc* merc,
	Mob* mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	return EventCommon(
		event_id,
		0,
		data.c_str(),
		merc,
		nullptr,
		nullptr,
		mob,
		nullptr,
		extra_data,
		true,
		extra_pointers
	);
}

void PerlembParser::LoadZoneScript(std::string filename)
{
	if (!perl || zone_quest_status_ != questUnloaded) {
		return;
	}

	try {
		perl->eval_file("qst_zone", filename.c_str());
	} catch (std::string e) {
		AddError(
			fmt::format(
				"Error Compiling Zone Quest File [{}] Error [{}]",
				filename,
				e
			)
		);

		zone_quest_status_ = questFailedToLoad;
		return;
	}

	zone_quest_status_ = questLoaded;
}

void PerlembParser::LoadGlobalZoneScript(std::string filename)
{
	if (!perl || global_zone_quest_status_ != questUnloaded) {
		return;
	}

	try {
		perl->eval_file("qst_global_zone", filename.c_str());
	} catch (std::string e) {
		AddError(
			fmt::format(
				"Error Compiling Global Zone Quest File [{}] Error [{}]",
				filename,
				e
			)
		);

		global_zone_quest_status_ = questFailedToLoad;
		return;
	}

	global_zone_quest_status_ = questLoaded;
}

bool PerlembParser::ZoneHasQuestSub(QuestEventID event_id)
{
	if (
		!perl ||
		zone_quest_status_ != questLoaded ||
		event_id >= _LargestEventID
	) {
		return false;
	}

	return perl->SubExists("qst_zone", QuestEventSubroutines[event_id]);
}

bool PerlembParser::GlobalZoneHasQuestSub(QuestEventID event_id)
{
	if (
		!perl ||
		global_zone_quest_status_ != questLoaded ||
		event_id >= _LargestEventID
	) {
		return false;
	}

	return perl->SubExists("qst_global_zone", QuestEventSubroutines[event_id]);
}

int PerlembParser::EventZone(
	QuestEventID event_id,
	Zone* zone,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	return EventCommon(
		event_id,
		0,
		data.c_str(),
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		zone,
		extra_data,
		false,
		extra_pointers
	);
}

int PerlembParser::EventGlobalZone(
	QuestEventID event_id,
	Zone* zone,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
)
{
	return EventCommon(
		event_id,
		0,
		data.c_str(),
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		zone,
		extra_data,
		true,
		extra_pointers
	);
}

#endif // EMBPERL
