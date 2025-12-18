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

#pragma once

#ifdef EMBPERL

#include "zone/embperl.h"
#include "zone/quest_interface.h"
#include "zone/quest_parser_collection.h"

#include <map>
#include <queue>
#include <string>

class Mob;
class Client;
class NPC;

namespace EQ {
	class ItemInstance;
}

typedef enum {
	questUnloaded,
	questLoaded,
	questFailedToLoad
} PerlQuestStatus;

enum class QuestType {
	Bot,
	BotGlobal,
	Item,
	ItemGlobal,
	Merc,
	MercGlobal,
	NPC,
	NPCGlobal,
	Player,
	PlayerGlobal,
	Spell,
	SpellGlobal,
	Zone,
	ZoneGlobal
};

class PerlembParser : public QuestInterface {
public:
	PerlembParser();
	~PerlembParser();

	virtual int EventNPC(
		QuestEventID event_id,
		NPC* npc,
		Mob* init,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	);

	virtual int EventGlobalNPC(
		QuestEventID event_id,
		NPC* npc,
		Mob* init,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	);

	virtual int EventPlayer(
		QuestEventID event_id,
		Client* client,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	);

	virtual int EventGlobalPlayer(
		QuestEventID event_id,
		Client* client,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	);

	virtual int EventItem(
		QuestEventID event_id,
		Client* client,
		EQ::ItemInstance* item,
		Mob* mob,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	);

	virtual int EventSpell(
		QuestEventID event_id,
		Mob* mob,
		Client* client,
		uint32 spell_id,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	);

	virtual int EventBot(
		QuestEventID event_id,
		Bot* bot,
		Mob* init,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	);

	virtual int EventGlobalBot(
		QuestEventID event_id,
		Bot* bot,
		Mob* init,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	);

	virtual int EventMerc(
		QuestEventID event_id,
		Merc* merc,
		Mob* init,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	);

	virtual int EventGlobalMerc(
		QuestEventID event_id,
		Merc* merc,
		Mob* init,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	);

	virtual int EventZone(
		QuestEventID event_id,
		Zone* zone,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	);

	virtual int EventGlobalZone(
		QuestEventID event_id,
		Zone* zone,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	);

	virtual bool HasQuestSub(uint32 npc_id, QuestEventID event_id);
	virtual bool HasGlobalQuestSub(QuestEventID event_id);
	virtual bool PlayerHasQuestSub(QuestEventID event_id);
	virtual bool GlobalPlayerHasQuestSub(QuestEventID event_id);
	virtual bool SpellHasQuestSub(uint32 spell_id, QuestEventID event_id);
	virtual bool ItemHasQuestSub(EQ::ItemInstance* inst, QuestEventID event_id);
	virtual bool BotHasQuestSub(QuestEventID event_id);
	virtual bool GlobalBotHasQuestSub(QuestEventID event_id);
	virtual bool MercHasQuestSub(QuestEventID event_id);
	virtual bool GlobalMercHasQuestSub(QuestEventID event_id);
	virtual bool ZoneHasQuestSub(QuestEventID event_id);
	virtual bool GlobalZoneHasQuestSub(QuestEventID event_id);

	virtual void LoadNPCScript(std::string filename, int npc_id);
	virtual void LoadGlobalNPCScript(std::string filename);
	virtual void LoadPlayerScript(std::string filename);
	virtual void LoadGlobalPlayerScript(std::string filename);
	virtual void LoadItemScript(std::string filename, EQ::ItemInstance* inst);
	virtual void LoadSpellScript(std::string filename, uint32 spell_id);
	virtual void LoadBotScript(std::string filename);
	virtual void LoadGlobalBotScript(std::string filename);
	virtual void LoadMercScript(std::string filename);
	virtual void LoadGlobalMercScript(std::string filename);
	virtual void LoadZoneScript(std::string filename);
	virtual void LoadGlobalZoneScript(std::string filename);

	virtual void AddVar(std::string name, std::string val);
	virtual std::string GetVar(std::string name);
	virtual void Init() override;
	virtual void ReloadQuests();
	virtual uint32 GetIdentifier() { return 0xf8b05c11; }

private:
	Embperl* perl;

	void ExportHash(const char* prefix, const char* hash_name, std::map<std::string, std::string>& vals);
	void ExportVar(const char* prefix, const char* variable_name, const char* value);
	void ExportVar(const char* prefix, const char* variable_name, int32 value);
	void ExportVar(const char* prefix, const char* variable_name, uint32 value);
	void ExportVar(const char* prefix, const char* variable_name, float value);
	void ExportVar(const char* prefix, const char* variable_name, const char* class_name, void* value);

	int EventCommon(
		QuestEventID event,
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
	);

	int SendCommands(
		const char* prefix,
		const char* event,
		uint32 spell_id,
		Mob* other,
		Mob* mob,
		EQ::ItemInstance* inst,
		const SPDat_Spell_Struct* spell,
		Zone* zone
	);

	void MapFunctions();

	QuestType GetQuestTypes(
		QuestEventID event,
		Mob* npc_mob,
		EQ::ItemInstance* inst,
		Mob* mob,
		Zone* zone,
		bool is_global
	);

	std::string GetQuestPackageName(
		QuestType quest_type,
		QuestEventID event,
		uint32 object_id,
		const char* data,
		Mob* npc_mob,
		EQ::ItemInstance* inst
	);

	void ExportCharID(const std::string& package_name, int& char_id, Mob* npc_mob, Mob* mob);

	void ExportQGlobals(
		QuestType quest_type,
		std::string& package_name,
		Mob* npc_mob,
		Mob* mob,
		int char_id
	);

	void ExportMobVariables(
		QuestType quest_type,
		std::string& package_name,
		Mob* mob,
		Mob* npc_mob
	);

	void ExportZoneVariables(std::string& package_name);

	void ExportItemVariables(std::string& package_name, Mob* mob);

	void ExportEventVariables(
		std::string& package_name,
		QuestEventID event,
		uint32 object_id,
		const char* data,
		Mob* npc_mob,
		EQ::ItemInstance* inst,
		Mob* mob,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	);

	std::map<uint32, PerlQuestStatus> npc_quest_status_;
	std::map<uint32, PerlQuestStatus> item_quest_status_;
	std::map<uint32, PerlQuestStatus> spell_quest_status_;

	PerlQuestStatus global_npc_quest_status_;
	PerlQuestStatus player_quest_status_;
	PerlQuestStatus global_player_quest_status_;
	PerlQuestStatus bot_quest_status_;
	PerlQuestStatus global_bot_quest_status_;
	PerlQuestStatus merc_quest_status_;
	PerlQuestStatus global_merc_quest_status_;
	PerlQuestStatus zone_quest_status_;
	PerlQuestStatus global_zone_quest_status_;

	SV* _empty_sv;

	std::map<std::string, std::string> vars_;
	std::map<std::string, int>         clear_vars_;
};

#endif // EMBPERL
