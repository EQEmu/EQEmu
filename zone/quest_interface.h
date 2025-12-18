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

#include "common/types.h"
#include "zone/event_codes.h"

#include <any>

class Client;
class NPC;

namespace EQ {
	class ItemInstance;
}

class QuestInterface {
public:
	virtual int EventNPC(
		QuestEventID event_id,
		NPC* npc,
		Mob* init,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	)
	{
		return 0;
	}

	virtual int EventGlobalNPC(
		QuestEventID event_id,
		NPC* npc,
		Mob* init,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	)
	{
		return 0;
	}

	virtual int EventPlayer(
		QuestEventID event_id,
		Client* client,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	)
	{
		return 0;
	}

	virtual int EventGlobalPlayer(
		QuestEventID event_id,
		Client* client,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	)
	{
		return 0;
	}

	virtual int EventItem(
		QuestEventID event_id,
		Client* client,
		EQ::ItemInstance* inst,
		Mob* mob,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	)
	{
		return 0;
	}

	virtual int EventSpell(
		QuestEventID event_id,
		Mob* mob,
		Client* client,
		uint32 spell_id,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	)
	{
		return 0;
	}

	virtual int EventEncounter(
		QuestEventID event_id,
		std::string encounter_name,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	)
	{
		return 0;
	}

	virtual int EventBot(
		QuestEventID event_id,
		Bot* bot,
		Mob* init,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	)
	{
		return 0;
	}

	virtual int EventGlobalBot(
		QuestEventID event_id,
		Bot* bot,
		Mob* init,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	)
	{
		return 0;
	}

	virtual int EventMerc(
		QuestEventID event_id,
		Merc* merc,
		Mob* init,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	)
	{
		return 0;
	}

	virtual int EventGlobalMerc(
		QuestEventID event_id,
		Merc* merc,
		Mob* init,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	)
	{
		return 0;
	}

	virtual int EventZone(
		QuestEventID event_id,
		Zone* zone,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	)
	{
		return 0;
	}

	virtual int EventGlobalZone(
		QuestEventID event_id,
		Zone* zone,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	)
	{
		return 0;
	}

	virtual bool HasQuestSub(uint32 npc_id, QuestEventID event_id)
	{
		return false;
	}

	virtual bool HasGlobalQuestSub(QuestEventID event_id)
	{
		return false;
	}

	virtual bool PlayerHasQuestSub(QuestEventID event_id)
	{
		return false;
	}

	virtual bool GlobalPlayerHasQuestSub(QuestEventID event_id)
	{
		return false;
	}

	virtual bool SpellHasQuestSub(uint32 spell_id, QuestEventID event_id)
	{
		return false;
	}

	virtual bool ItemHasQuestSub(EQ::ItemInstance* itm, QuestEventID event_id)
	{
		return false;
	}

	virtual bool EncounterHasQuestSub(std::string encounter_name, QuestEventID event_id)
	{
		return false;
	}

	virtual bool HasEncounterSub(const std::string& package_name, QuestEventID event_id)
	{
		return false;
	}

	virtual bool BotHasQuestSub(QuestEventID event_id)
	{
		return false;
	}

	virtual bool GlobalBotHasQuestSub(QuestEventID event_id)
	{
		return false;
	}

	virtual bool MercHasQuestSub(QuestEventID event_id)
	{
		return false;
	}

	virtual bool GlobalMercHasQuestSub(QuestEventID event_id)
	{
		return false;
	}

	virtual bool ZoneHasQuestSub(QuestEventID event_id)
	{
		return false;
	}

	virtual bool GlobalZoneHasQuestSub(QuestEventID event_id)
	{
		return false;
	}

	virtual void LoadNPCScript(std::string filename, int npc_id) { }
	virtual void LoadGlobalNPCScript(std::string filename) { }
	virtual void LoadPlayerScript(std::string filename) { }
	virtual void LoadGlobalPlayerScript(std::string filename) { }
	virtual void LoadItemScript(std::string filename, EQ::ItemInstance* inst) { }
	virtual void LoadSpellScript(std::string filename, uint32 spell_id) { }
	virtual void LoadEncounterScript(std::string filename, std::string encounter_name) { }
	virtual void LoadBotScript(std::string filename) { }
	virtual void LoadGlobalBotScript(std::string filename) { }
	virtual void LoadMercScript(std::string filename) { }
	virtual void LoadGlobalMercScript(std::string filename) { }
	virtual void LoadZoneScript(std::string filename) { }
	virtual void LoadGlobalZoneScript(std::string filename) { }

	virtual int DispatchEventNPC(
		QuestEventID event_id,
		NPC* npc,
		Mob* init,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	)
	{
		return 0;
	}

	virtual int DispatchEventPlayer(
		QuestEventID event_id,
		Client* client,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	)
	{
		return 0;
	}

	virtual int DispatchEventItem(
		QuestEventID event_id,
		Client* client,
		EQ::ItemInstance* inst,
		Mob* mob,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	)
	{
		return 0;
	}

	virtual int DispatchEventSpell(
		QuestEventID event_id,
		Mob* mob,
		Client* client,
		uint32 spell_id,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	)
	{
		return 0;
	}

	virtual int DispatchEventBot(
		QuestEventID event_id,
		Bot* bot,
		Mob* init,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	)
	{
		return 0;
	}

	virtual int DispatchEventMerc(
		QuestEventID event_id,
		Merc* merc,
		Mob* init,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	)
	{
		return 0;
	}

	virtual int DispatchEventZone(
		QuestEventID event_id,
		Zone* zone,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	)
	{
		return 0;
	}

	virtual void AddVar(std::string name, std::string val) { }
	virtual std::string GetVar(std::string name)
	{
		return std::string();
	}
	virtual void Init() { }
	virtual void ReloadQuests() { }
	virtual uint32 GetIdentifier() = 0;
	virtual void RemoveEncounter(const std::string& name) { }

	virtual void GetErrors(std::list<std::string>& quest_errors)
	{
		quest_errors.insert(quest_errors.end(), errors_.begin(), errors_.end());
	}

	virtual void AddError(std::string error)
	{
		LogQuests("{}", error);
		LogQuestErrors("{}", Strings::Trim(error));

		errors_.push_back(error);

		if (errors_.size() > RuleI(World, MaximumQuestErrors)) {
			errors_.pop_front();
		}
	}

protected:
	std::list<std::string> errors_;
};
