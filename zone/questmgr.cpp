/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2005 EQEMu Development Team (http://eqemulator.net)

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

#include "common/classes.h"
#include "common/data_verification.h"
#include "common/events/player_event_logs.h"
#include "common/repositories/account_repository.h"
#include "common/repositories/completed_tasks_repository.h"
#include "common/repositories/grid_entries_repository.h"
#include "common/repositories/instance_list_repository.h"
#include "common/repositories/tradeskill_recipe_repository.h"
#include "common/rulesys.h"
#include "common/say_link.h"
#include "common/skills.h"
#include "common/spdat.h"
#include "common/strings.h"
#include "zone/bot.h"
#include "zone/dialogue_window.h"
#include "zone/entity.h"
#include "zone/event_codes.h"
#include "zone/guild_mgr.h"
#include "zone/qglobals.h"
#include "zone/queryserv.h"
#include "zone/quest_parser_collection.h"
#include "zone/questmgr.h"
#include "zone/spawn2.h"
#include "zone/worldserver.h"
#include "zone/zone.h"
#include "zone/zonedb.h"

#include <climits>
#include <iostream>
#include <list>
#include <numbers>

extern QueryServ* QServ;
extern Zone* zone;
extern WorldServer worldserver;
extern EntityList entity_list;

QuestManager quest_manager;

#define QuestManagerCurrentQuestVars() \
	Mob *owner = nullptr; \
	Client *initiator = nullptr; \
	EQ::ItemInstance* questitem = nullptr; \
	const SPDat_Spell_Struct* questspell = nullptr; \
	bool depop_npc = false; \
	std::string encounter = ""; \
	do { \
		if(!m_running_quests.empty()) { \
			RunningQuest e = m_running_quests.top(); \
			owner = e.owner; \
			initiator = e.initiator; \
			questitem = e.questitem; \
			questspell = e.questspell; \
			depop_npc = e.depop_npc; \
			encounter = e.encounter; \
		} \
	} while(0)

QuestManager::QuestManager() {
	HaveProximitySays = false;
}

QuestManager::~QuestManager() {
}

void QuestManager::Process() {
	std::list<QuestTimer>::iterator cur = QTimerList.begin(), end;

	end = QTimerList.end();
	while (cur != end) {
		if (cur->Timer_.Enabled() && cur->Timer_.Check()) {
			if (cur->mob) {
				if (cur->mob->IsEncounter()) {
					parse->EventEncounter(EVENT_TIMER, cur->mob->CastToEncounter()->GetEncounterName(), cur->name, 0, nullptr);
				} else {
					parse->EventMob(EVENT_TIMER, cur->mob, nullptr, [&]() { return cur->name; }, 0);
				}

				//we MUST reset our iterator since the quest could have removed/added any
				//number of timers... worst case we have to check a bunch of timers twice
				cur = QTimerList.begin();
				end = QTimerList.end();    //dunno if this is needed, cant hurt...
			}
			else {
				cur = QTimerList.erase(cur);
			}
		}
		else {
			++cur;
		}
	}

	auto cur_iter = STimerList.begin();
	while(cur_iter != STimerList.end()) {
		if(!cur_iter->Timer_.Enabled()) {
			cur_iter = STimerList.erase(cur_iter);
		} else if(cur_iter->Timer_.Check()) {
			entity_list.SignalMobsByNPCID(cur_iter->npc_id, cur_iter->signal_id);
			cur_iter = STimerList.erase(cur_iter);
		} else {
			++cur_iter;
		}
	}
}

void QuestManager::StartQuest(const RunningQuest& q)
{
	m_running_quests.push(q);
}

void QuestManager::EndQuest() {
	RunningQuest run = m_running_quests.top();

	if (run.depop_npc && run.owner->IsNPC()) {
		//clear out any timers for them...
		std::list<QuestTimer>::iterator cur = QTimerList.begin(), end;

		end = QTimerList.end();
		while (cur != end) {
			if (cur->mob == run.owner) {
				cur = QTimerList.erase(cur);
			} else {
				++cur;
			}
		}
		run.owner->Depop();
	}

	m_running_quests.pop();
}

void QuestManager::ClearAllTimers() {
	QTimerList.clear();
}

//quest perl functions
void QuestManager::echo(int colour, const char *str) {
	QuestManagerCurrentQuestVars();

	if (!owner) {
		return;
	}

	entity_list.MessageClose(owner, false, 200, colour, str);
}

void QuestManager::say(const char *str, Journal::Options &opts) {
	QuestManagerCurrentQuestVars();
	if (!owner) {
		LogQuests("QuestManager::say called with nullptr owner. Probably syntax error in quest file");
		return;
	}
	else {
		// if there is no initiator we still want stuff to work (timers, signals, waypoints, etc)
		if (!RuleB(NPC, EnableNPCQuestJournal) || initiator == nullptr)
			opts.journal_mode = Journal::Mode::None;
		owner->QuestJournalledSay(initiator, str, opts);
	}
}

void QuestManager::me(const char *str) {
	QuestManagerCurrentQuestVars();

	if (!owner) {
		return;
	}

	entity_list.MessageClose(owner, false, 200, 10, str);
}

void QuestManager::summonitem(uint32 itemid, int16 charges) {
	QuestManagerCurrentQuestVars();
	if(!initiator)
		return;
	initiator->SummonItem(itemid, charges);
}

void QuestManager::write(const char *file, const char *str) {
	FILE * pFile;
	pFile = fopen (fmt::format("{}/{}", PathManager::Instance()->GetServerPath(), file).c_str(), "a");
	if(!pFile)
		return;
	fprintf(pFile, "%s\n", str);
	fclose (pFile);
}

Mob* QuestManager::spawn2(int npc_id, int grid, int unused, const glm::vec4& position) {
	QuestManagerCurrentQuestVars();
	if (owner && owner->IsNPC()) {
		auto n = owner->CastToNPC();
		if (n->IsResumedFromZoneSuspend()) {
			LogZoneState("NPC [{}] is resuming from zone suspend, skipping quest call", n->GetCleanName());
			return nullptr;
		}
	}

	const NPCType* t = 0;
	if (t = content_db.LoadNPCTypesData(npc_id)) {
		auto npc = new NPC(t, nullptr, position, GravityBehavior::Water);
		npc->AddLootTable();
		if (npc->DropsGlobalLoot()) {
			npc->CheckGlobalLootTables();
		}

		entity_list.AddNPC(npc, true, true);

		if (grid) {
			npc->AssignWaypoints(grid);
		}

		return npc;
	}
	return nullptr;
}

Mob* QuestManager::unique_spawn(int npc_type, int grid, int unused, const glm::vec4& position) {
	QuestManagerCurrentQuestVars();
	if (owner && owner->IsNPC()) {
		auto n = owner->CastToNPC();
		if (n->IsResumedFromZoneSuspend()) {
			LogZoneState("NPC [{}] is resuming from zone suspend, skipping quest call", n->GetCleanName());
			return nullptr;
		}
	}

	Mob *other = entity_list.GetMobByNpcTypeID(npc_type);
	if(other != nullptr) {
		return other;
	}

	const NPCType* tmp = 0;
	if (tmp = content_db.LoadNPCTypesData(npc_type))
	{
		auto npc = new NPC(tmp, nullptr, position, GravityBehavior::Water);
		npc->AddLootTable();
		if (npc->DropsGlobalLoot())
			npc->CheckGlobalLootTables();
		entity_list.AddNPC(npc,true,true);
		if(grid > 0)
		{
			npc->AssignWaypoints(grid);
		}

		return npc;
	}
	return nullptr;
}

Mob *QuestManager::spawn_from_spawn2(uint32 spawn2_id)
{
	LinkedListIterator<Spawn2 *> iterator(zone->spawn2_list);
	iterator.Reset();
	Spawn2 *found_spawn = nullptr;

	while (iterator.MoreElements()) {
		Spawn2 *cur = iterator.GetData();
		iterator.Advance();
		if (cur->GetID() == spawn2_id) {
			found_spawn = cur;
			break;
		}
	}

	if (found_spawn) {
		SpawnGroup *spawn_group = zone->spawn_group_list.GetSpawnGroup(found_spawn->SpawnGroupID());
		if (!spawn_group) {
			content_db.LoadSpawnGroupsByID(found_spawn->SpawnGroupID(), &zone->spawn_group_list);
			spawn_group = zone->spawn_group_list.GetSpawnGroup(found_spawn->SpawnGroupID());
			if (!spawn_group) {
				return nullptr;
			}
		}

		uint16 condition_value=1;
		uint16 condition_id=found_spawn->GetSpawnCondition();

		if (condition_id > 0) {
			condition_value = zone->spawn_conditions.GetCondition(zone->GetShortName(), zone->GetInstanceID(), condition_id);
		}

		uint32 npcid = spawn_group->GetNPCType(condition_value);

		if (npcid == 0) {
			return nullptr;
		}

		const NPCType *tmp = content_db.LoadNPCTypesData(npcid);
		if (!tmp) {
			return nullptr;
		}

		if (tmp->unique_spawn_by_name) {
			if (!entity_list.LimitCheckName(tmp->name)) {
				return nullptr;
			}
		}

		if (tmp->spawn_limit > 0) {
			if (!entity_list.LimitCheckType(npcid, tmp->spawn_limit)) {
				return nullptr;
			}
		}

		database.UpdateRespawnTime(spawn2_id, zone->GetInstanceID(), 0);
		found_spawn->SetCurrentNPCID(npcid);

		auto position = glm::vec4(
			found_spawn->GetX(),
			found_spawn->GetY(),
			found_spawn->GetZ(),
			found_spawn->GetHeading()
		);

		auto npc = new NPC(tmp, found_spawn, position, GravityBehavior::Water);

		found_spawn->SetNPCPointer(npc);
		npc->AddLootTable();
		if (npc->DropsGlobalLoot()) {
			npc->CheckGlobalLootTables();
		}
		npc->SetSpawnGroupId(found_spawn->SpawnGroupID());
		entity_list.AddNPC(npc);
		entity_list.LimitAddNPC(npc);

		if (spawn_group->roamdist > 0) {
			npc->AI_SetRoambox(
				spawn_group->roamdist,
				spawn_group->roambox[0],
				spawn_group->roambox[1],
				spawn_group->roambox[2],
				spawn_group->roambox[3],
				spawn_group->delay,
				spawn_group->min_delay
			);
		}
		if (zone->InstantGrids()) {
			found_spawn->LoadGrid();
		}

		return npc;
	}

	return nullptr;
}

void QuestManager::enable_spawn2(uint32 spawn2_id)
{
	database.UpdateSpawn2Status(spawn2_id, 1, zone->GetInstanceID());
	auto pack = new ServerPacket(ServerOP_SpawnStatusChange, sizeof(ServerSpawnStatusChange_Struct));
	auto *ssc = (ServerSpawnStatusChange_Struct *) pack->pBuffer;
	ssc->id          = spawn2_id;
	ssc->new_status  = true;
	ssc->instance_id = zone->GetInstanceID();
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::disable_spawn2(uint32 spawn2_id)
{
	database.UpdateSpawn2Status(spawn2_id, 0, zone->GetInstanceID());
	auto pack = new ServerPacket(ServerOP_SpawnStatusChange, sizeof(ServerSpawnStatusChange_Struct));
	auto *ssc = (ServerSpawnStatusChange_Struct *) pack->pBuffer;
	ssc->id          = spawn2_id;
	ssc->new_status  = false;
	ssc->instance_id = zone->GetInstanceID();

	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::setstat(int stat, int value) {
	QuestManagerCurrentQuestVars();
	if (initiator)
		initiator->SetStats(stat, value);
}

void QuestManager::incstat(int stat, int value) {
	QuestManagerCurrentQuestVars();
	if (initiator)
		initiator->IncStats(stat, value);
}

void QuestManager::castspell(uint16 spell_id, uint16 target_id)
{
	QuestManagerCurrentQuestVars();

	if (owner) {
		Mob* t = entity_list.GetMob(target_id);
		if (t) {
			owner->SpellFinished(
				spell_id,
				t,
				EQ::spells::CastingSlot::Item,
				0,
				-1,
				spells[spell_id].resist_difficulty
			);
		}
	}
}

void QuestManager::selfcast(uint16 spell_id)
{
	QuestManagerCurrentQuestVars();

	if (initiator) {
		initiator->SpellFinished(
			spell_id,
			initiator,
			EQ::spells::CastingSlot::Item,
			0,
			-1,
			spells[spell_id].resist_difficulty
		);
	}
}

void QuestManager::addloot(
	int item_id,
	int charges,
	bool equipitem,
	int aug1,
	int aug2,
	int aug3,
	int aug4,
	int aug5,
	int aug6
) {
	QuestManagerCurrentQuestVars();
	if (!owner) {
		return;
	}

	if (item_id != 0) {
		if (owner->IsNPC()) {
			owner->CastToNPC()->AddItem(item_id, charges, equipitem, aug1, aug2, aug3, aug4, aug5, aug6);
		}
	}
}

void QuestManager::Zone(const char *zone_name) {
	QuestManagerCurrentQuestVars();
	if (initiator)
	{
		initiator->MoveZone(zone_name);
	}
}

void QuestManager::ZoneGroup(const char *zone_name) {
	QuestManagerCurrentQuestVars();
	if (initiator) {
		if (!initiator->GetGroup()) {
			initiator->MoveZone(zone_name);
		} else {
			auto client_group = initiator->GetGroup();
			for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
				if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
					auto group_member = client_group->members[member_index]->CastToClient();
					group_member->MoveZone(zone_name);
				}
			}
		}
	}
}

void QuestManager::ZoneRaid(const char *zone_name) {
	QuestManagerCurrentQuestVars();
	if (initiator) {
		if (!initiator->GetRaid()) {
			initiator->MoveZone(zone_name);
		} else {
			auto client_raid = initiator->GetRaid();
			for (const auto& m : client_raid->members) {
				if (m.is_bot) {
					continue;
				}

				if (m.member && m.member->IsClient()) {
					auto raid_member = m.member->CastToClient();
					raid_member->MoveZone(zone_name);
				}
			}
		}
	}
}

void QuestManager::settimer(const std::string& timer_name, uint32 seconds, Mob* m)
{
	QuestManagerCurrentQuestVars();

	if (questitem) {
		questitem->SetTimer(timer_name, seconds * 1000);

		if (parse->ItemHasQuestSub(questitem, EVENT_TIMER_START)) {
			const std::string& export_string = fmt::format(
				"{} {}",
				timer_name,
				seconds * 1000
			);

			parse->EventItem(EVENT_TIMER_START, nullptr, questitem, nullptr, export_string, 0);
		}

		return;
	}

	if (!m && !owner) {
		return;
	}

	Mob* mob = m ? m : owner;

	if (!mob) {
		return;
	}

	std::function<std::string()> f = [&]() {
		return fmt::format(
			"{} {}",
			timer_name,
			seconds * 1000
		);
	};

	if (!QTimerList.empty()) {
		for (auto& e : QTimerList) {
			if (e.mob && e.mob == mob && e.name == timer_name) {
				e.Timer_.Start(seconds * 1000, false);

				parse->EventMob(EVENT_TIMER_START, mob, nullptr, f);

				return;
			}
		}
	}

	QTimerList.emplace_back(QuestTimer(seconds * 1000, mob, timer_name));

	parse->EventMob(EVENT_TIMER_START, mob, nullptr, f);
}

void QuestManager::settimerMS(const std::string& timer_name, uint32 milliseconds)
{
	QuestManagerCurrentQuestVars();

	if (!owner) {
		return;
	}

	std::function<std::string()> f = [&]() {
		return fmt::format(
			"{} {}",
			timer_name,
			milliseconds
		);
	};

	if (questitem) {
		questitem->SetTimer(timer_name, milliseconds);

		if (parse->ItemHasQuestSub(questitem, EVENT_TIMER_START)) {
			const std::string& export_string = fmt::format(
				"{} {}",
				timer_name,
				milliseconds
			);

			parse->EventItem(EVENT_TIMER_START, nullptr, questitem, nullptr, export_string, 0);
		}

		return;
	}

	if (!QTimerList.empty()) {
		for (auto& e : QTimerList) {
			if (e.mob && e.mob == owner && e.name == timer_name) {
				e.Timer_.Start(milliseconds, false);

				parse->EventMob(EVENT_TIMER_START, owner, nullptr, f);

				return;
			}
		}
	}

	QTimerList.emplace_back(QuestTimer(milliseconds, owner, timer_name));

	parse->EventMob(EVENT_TIMER_START, owner, nullptr, f);
}

void QuestManager::settimerMS(const std::string& timer_name, uint32 milliseconds, EQ::ItemInstance* inst)
{
	if (inst) {
		inst->SetTimer(timer_name, milliseconds);
	}
}

void QuestManager::settimerMS(const std::string& timer_name, uint32 milliseconds, Mob* m)
{
	if (!m) {
		return;
	}

	std::function<std::string()> f = [&]() {
		return fmt::format(
			"{} {}",
			timer_name,
			milliseconds
		);
	};

	if (!QTimerList.empty()) {
		for (auto& e : QTimerList) {
			if (e.mob && e.mob == m && e.name == timer_name) {
				e.Timer_.Start(milliseconds, false);

				parse->EventMob(EVENT_TIMER_START, m, nullptr, f);

				return;
			}
		}
	}

	QTimerList.emplace_back(QuestTimer(milliseconds, m, timer_name));

	parse->EventMob(EVENT_TIMER_START, m, nullptr, f);
}

void QuestManager::stoptimer(const std::string& timer_name)
{
	QuestManagerCurrentQuestVars();

	if (questitem) {
		questitem->StopTimer(timer_name);

		if (parse->ItemHasQuestSub(questitem, EVENT_TIMER_STOP)) {
			parse->EventItem(EVENT_TIMER_STOP, nullptr, questitem, nullptr, timer_name, 0);
		}

		return;
	}

	if (!owner) {
		return;
	}

	if (QTimerList.empty()) {
		return;
	}

	for (auto e = QTimerList.begin(); e != QTimerList.end(); ++e) {
		if (e->mob && e->mob == owner && e->name == timer_name) {
			parse->EventMob(EVENT_TIMER_STOP, owner, nullptr, [&]() { return timer_name; });

			QTimerList.erase(e);
			break;
		}
	}
}

void QuestManager::stoptimer(const std::string& timer_name, EQ::ItemInstance* inst)
{
	if (inst) {
		inst->StopTimer(timer_name);
	}
}

void QuestManager::stoptimer(const std::string& timer_name, Mob* m)
{
	if (!m) {
		return;
	}

	if (QTimerList.empty()) {
		return;
	}

	for (auto e = QTimerList.begin(); e != QTimerList.end(); ++e) {
		if (e->mob && e->mob == m && e->name == timer_name) {
			parse->EventMob(EVENT_TIMER_STOP, m, nullptr, [&]() { return timer_name; });

			QTimerList.erase(e);
			break;
		}
	}
}

void QuestManager::stopalltimers()
{
	QuestManagerCurrentQuestVars();

	if (questitem) {
		if (parse->ItemHasQuestSub(questitem, EVENT_TIMER_STOP)) {
			auto item_timers = questitem->GetTimers();

			if (item_timers.empty()) {
				return;
			}

			for (auto e = item_timers.begin(); e != item_timers.end(); ++e) {
				if (parse->ItemHasQuestSub(questitem, EVENT_TIMER_STOP)) {
					parse->EventItem(EVENT_TIMER_STOP, nullptr, questitem, nullptr, e->first, 0);
				}
			}
		}

		questitem->ClearTimers();
		return;
	}

	if (!owner) {
		return;
	}

	if (QTimerList.empty()) {
		return;
	}

	for (auto e = QTimerList.begin(); e != QTimerList.end();) {
		if (e->mob && e->mob == owner) {
			parse->EventMob(EVENT_TIMER_STOP, owner, nullptr, [&]() { return e->name; });

			e = QTimerList.erase(e);
		} else {
			++e;
		}
	}
}

void QuestManager::stopalltimers(EQ::ItemInstance* inst)
{
	if (inst) {
		if (parse->ItemHasQuestSub(inst, EVENT_TIMER_STOP)) {
			auto item_timers = inst->GetTimers();

			if (item_timers.empty()) {
				return;
			}

			for (auto e = item_timers.begin(); e != item_timers.begin(); ++e) {
				if (parse->ItemHasQuestSub(inst, EVENT_TIMER_STOP)) {
					parse->EventItem(EVENT_TIMER_STOP, nullptr, inst, nullptr, e->first, 0);
				}
			}
		}

		inst->ClearTimers();
	}
}

void QuestManager::stopalltimers(Mob* m)
{
	if (!m) {
		return;
	}

	if (QTimerList.empty()) {
		return;
	}

	for (auto e = QTimerList.begin(); e != QTimerList.end();) {
		if (e->mob && e->mob == m) {
			parse->EventMob(EVENT_TIMER_STOP, m, nullptr, [&]() { return e->name; });

			e = QTimerList.erase(e);
		} else {
			++e;
		}
	}
}

void QuestManager::pausetimer(const std::string& timer_name, Mob* m)
{
	QuestManagerCurrentQuestVars();

	if (!m && !owner) {
		return;
	}

	Mob* mob = m ? m : owner;

	if (!mob) {
		return;
	}

	if (QTimerList.empty()) {
		return;
	}

	for (const auto& e : PTimerList) {
		if (e.owner && e.owner == mob && e.name == timer_name) {
			LogQuests("Timer [{}] is already paused for [{}]", timer_name, owner->GetName());
			return;
		}
	}

	uint32 milliseconds = 0;

	if (!QTimerList.empty()) {
		for (auto e = QTimerList.begin(); e != QTimerList.end(); ++e) {
			if (e->mob && e->mob == mob && e->name == timer_name) {
				milliseconds = e->Timer_.GetRemainingTime();
				QTimerList.erase(e);
				break;
			}
		}
	}

	PTimerList.emplace_back(
		PausedTimer{
			.owner = owner,
			.name = timer_name,
			.time = milliseconds
		}
	);

	parse->EventMob(EVENT_TIMER_PAUSE, mob, nullptr, [&]() {
		return fmt::format(
			"{} {}",
			timer_name,
			milliseconds
		);
	});

	LogQuests("Pausing timer [{}] for [{}] with [{}] ms remaining", timer_name, owner->GetName(), milliseconds);
}

void QuestManager::resumetimer(const std::string& timer_name, Mob* m)
{
	QuestManagerCurrentQuestVars();

	if (!m && !owner) {
		return;
	}

	uint32 milliseconds = 0;

	Mob* mob = m ? m : owner;

	if (!mob) {
		return;
	}

	if (PTimerList.empty()) {
		return;
	}

	for (auto e = PTimerList.begin(); e != PTimerList.end(); ++e) {
		if (e->owner && e->owner == mob && e->name == timer_name) {
			milliseconds = e->time;
			PTimerList.erase(e);
			break;
		}
	}

	if (!milliseconds) {
		LogQuests("Paused timer [{}] not found or has expired.", timer_name);
		return;
	}

	std::function<std::string()> f = [&]() {
		return fmt::format(
			"{} {}",
			timer_name,
			milliseconds
		);
	};

	if (!QTimerList.empty()) {
		for (auto e : QTimerList) {
			if (e.mob && e.mob == mob && e.name == timer_name) {
				e.Timer_.Enable();
				e.Timer_.Start(milliseconds, false);
				LogQuests(
					"Resuming timer [{}] for [{}] with [{}] ms remaining",
					timer_name,
					owner->GetName(),
					milliseconds
				);

				parse->EventMob(EVENT_TIMER_RESUME, mob, nullptr, f);

				return;
			}
		}
	}

	QTimerList.emplace_back(QuestTimer(milliseconds, mob, timer_name));

	parse->EventMob(EVENT_TIMER_RESUME, mob, nullptr, f);

	LogQuests(
		"Creating a new timer and resuming [{}] for [{}] with [{}] ms remaining",
		timer_name,
		owner->GetName(),
		milliseconds
	);

}

bool QuestManager::ispausedtimer(const std::string& timer_name, Mob* m)
{
	QuestManagerCurrentQuestVars();

	if (!m && !owner) {
		return false;
	}

	Mob* mob = m ? m : owner;

	if (!mob) {
		return false;
	}

	const auto& e = std::find_if(
		PTimerList.begin(),
		PTimerList.end(),
		[&timer_name, &mob](PausedTimer e) {
			return e.owner && e.owner == mob && e.name == timer_name;
		}
	);

	return e != PTimerList.end();
}

bool QuestManager::hastimer(const std::string& timer_name, Mob* m)
{
	QuestManagerCurrentQuestVars();

	if (!m && !owner) {
		return false;
	}

	Mob* mob = m ? m : owner;

	if (!mob) {
		return false;
	}

	const auto& e = std::find_if(
		QTimerList.begin(),
		QTimerList.end(),
		[&timer_name, &mob](QuestTimer e) {
			return e.mob && e.mob == mob && e.name == timer_name;
		}
	);

	return e != QTimerList.end();
}

uint32 QuestManager::getremainingtimeMS(const std::string& timer_name, Mob* m)
{
	QuestManagerCurrentQuestVars();

	if (!m && !owner) {
		return 0;
	}

	const auto mob = m ? m : owner;

	if (!mob) {
		return 0;
	}

	const auto& e = std::find_if(
		QTimerList.begin(),
		QTimerList.end(),
		[&timer_name, &mob](QuestTimer e) {
			return e.mob && e.mob == mob && e.name == timer_name;
		}
	);

	return e != QTimerList.end() ? e->Timer_.GetRemainingTime() : 0;
}

uint32 QuestManager::gettimerdurationMS(const std::string& timer_name, Mob* m)
{
	QuestManagerCurrentQuestVars();

	if (!m && !owner) {
		return 0;
	}

	const auto mob = m ? m : owner;

	if (!mob) {
		return 0;
	}

	const auto& e = std::find_if(
		QTimerList.begin(),
		QTimerList.end(),
		[&timer_name, &mob](QuestTimer e) {
			return e.mob && e.mob == mob && e.name == timer_name;
		}
	);

	return e != QTimerList.end() ? e->Timer_.GetDuration() : 0;
}

void QuestManager::emote(const char *str) {
	QuestManagerCurrentQuestVars();
	if (!owner) {
		LogQuests("QuestManager::emote called with nullptr owner. Probably syntax error in quest file");
		return;
	}
	else {
		owner->Emote(str);
	}
}

void QuestManager::shout(const char *str) {
	QuestManagerCurrentQuestVars();
	if (!owner) {
		LogQuests("QuestManager::shout called with nullptr owner. Probably syntax error in quest file");
		return;
	}
	else {
		owner->Shout(str);
	}
}

void QuestManager::shout2(const char *str) {
	QuestManagerCurrentQuestVars();
	if (!owner) {
		LogQuests("QuestManager::shout2 called with nullptr owner. Probably syntax error in quest file");
		return;
	} else {
		worldserver.SendEmoteMessage(
			0,
			0,
			AccountStatus::Player,
			Chat::Red,
			fmt::format(
				"{} shouts, '{}'",
				owner->GetCleanName(),
				str
			).c_str()
		);
	}
}

void QuestManager::gmsay(const char *str, uint32 color, bool send_to_world, uint32 to_guilddbid, uint32 to_minstatus) {
	QuestManagerCurrentQuestVars();
	if(send_to_world) {
		worldserver.SendEmoteMessage(
			0,
			to_guilddbid,
			to_minstatus,
			color,
			str
		);
	} else {
		entity_list.MessageStatus(
			to_guilddbid,
			to_minstatus,
			color,
			str
		);
	}
}

void QuestManager::depop(int npc_type) {
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC()) {
		LogQuests("QuestManager::depop called with nullptr owner or non-NPC owner. Probably syntax error in quest file");
		return;
	}
	else {
		if (npc_type != 0) {
			Mob * tmp = entity_list.GetMobByNpcTypeID(npc_type);
			if (tmp) {
				if (tmp != owner) {
					tmp->CastToNPC()->Depop();
				}
				else {
					RunningQuest e = m_running_quests.top();
					e.depop_npc = true;
					m_running_quests.pop();
					m_running_quests.push(e);
				}
			}
		}
		else {	//depop self
			RunningQuest e = m_running_quests.top();
			e.depop_npc = true;
			m_running_quests.pop();
			m_running_quests.push(e);
		}
	}
}

void QuestManager::depop_withtimer(int npc_type) {
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC()) {
		LogQuests("QuestManager::depop_withtimer called with nullptr owner or non-NPC owner. Probably syntax error in quest file");
		return;
	}
	else {
		if (npc_type != 0) {
			Mob * tmp = entity_list.GetMobByNpcTypeID(npc_type);
			if (tmp) {
				if (tmp != owner) {
					tmp->CastToNPC()->Depop(true);
				}
				else {
					owner->Depop(true);
				}
			}
		}
		else {	//depop self
			owner->Depop(true);
		}
	}
}

void QuestManager::depopall(int npc_type) {
	if (npc_type) {
		entity_list.DepopAll(npc_type);
	} else {
		LogQuests("QuestManager::depopall called with nullptr owner, non-NPC owner, or invalid NPC Type ID. Probably syntax error in quest file.");
	}
}

void QuestManager::depopzone(bool StartSpawnTimer) {
	if(zone) {
		zone->Depop(StartSpawnTimer);
	}
	else {
		LogQuests("QuestManager::depopzone called with nullptr zone. Probably syntax error in quest file");
	}
}

void QuestManager::repopzone(bool is_forced)
{
	if (zone) {
		zone->Repop(is_forced);
	} else {
		LogQuests("QuestManager::repopzone called with nullptr zone. Probably syntax error in quest file");
	}
}

void QuestManager::processmobswhilezoneempty(bool on) {
	if (zone) {
		zone->quest_idle_override = on;
	} else {
		LogQuests(
			"QuestManager::processmobswhilezoneempty called with nullptr zone. Probably syntax error in quest file"
		);
	}
}

void QuestManager::settarget(const char *type, int target_id) {
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC())
		return;

	Mob* tmp = nullptr;
	if (!strcasecmp(type,"npctype"))
		tmp = entity_list.GetMobByNpcTypeID(target_id);
	else if (!strcasecmp(type, "entity"))
		tmp = entity_list.GetMob(target_id);

	if (tmp != nullptr)
		owner->SetTarget(tmp);
}

void QuestManager::follow(int entity_id, int distance) {
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC())
		return;

	owner->SetFollowID(entity_id);
	owner->SetFollowDistance(distance * distance);
}

void QuestManager::sfollow() {
	QuestManagerCurrentQuestVars();
	if (owner == nullptr || !owner->IsNPC())
		return;
	owner->SetFollowID(0);
}

void QuestManager::changedeity(uint32 deity_id) {
	QuestManagerCurrentQuestVars();
	//Changes the deity.
	if(initiator)
	{
		initiator->SetDeity(deity_id);
		initiator->Message(Chat::Yellow,"Your Deity has been changed/set to: %i", deity_id);
		initiator->Save(1);
		initiator->Kick("Deity change by QuestManager");
	}
}

void QuestManager::exp(int amt) {
	QuestManagerCurrentQuestVars();
	if (initiator)
		initiator->AddEXP(ExpSource::Quest, amt);
}

void QuestManager::level(int newlevel) {
	QuestManagerCurrentQuestVars();
	if (initiator)
		initiator->SetLevel(newlevel, true);
}

void QuestManager::traindisc(uint32 discipline_tome_item_id) {
	QuestManagerCurrentQuestVars();
	if (initiator) {
		initiator->TrainDiscipline(discipline_tome_item_id);
	}
}

bool QuestManager::isdisctome(uint32 item_id) {
	const auto &item = database.GetItem(item_id);
	if (!item) {
		return false;
	}

	return IsDisciplineTome(item);
}

std::string QuestManager::getracename(uint16 race_id) {
	return GetRaceIDName(race_id);
}

std::string QuestManager::getspellname(uint32 spell_id) {
	if (!IsValidSpell(spell_id)) {
		return "INVALID SPELL ID IN GETSPELLNAME";
	}

	std::string spell_name = GetSpellName(spell_id);
	return spell_name;
}

std::string QuestManager::getskillname(int skill_id) {
	return EQ::skills::GetSkillName(static_cast<EQ::skills::SkillType>(skill_id));
}

std::string QuestManager::getldonthemename(uint32 theme_id) {
	return LDoNTheme::GetName(theme_id);
}

std::string QuestManager::getfactionname(int faction_id) {
	return content_db.GetFactionName(faction_id);
}

std::string QuestManager::getlanguagename(uint8 language_id) {
	return EQ::constants::GetLanguageName(language_id);
}

std::string QuestManager::getbodytypename(uint8 body_type_id) {
	return BodyType::GetName(body_type_id);
}

std::string QuestManager::getconsiderlevelname(uint8 consider_level) {
	return EQ::constants::GetConsiderLevelName(consider_level);
}

void QuestManager::safemove() {
	QuestManagerCurrentQuestVars();
	if (initiator)
		initiator->GoToSafeCoords(zone->GetZoneID(), zone->GetInstanceID());
}

void QuestManager::rain(int weather) {
	QuestManagerCurrentQuestVars();
	zone->zone_weather = weather;
	auto outapp = new EQApplicationPacket(OP_Weather, 8);
	*((uint32*) &outapp->pBuffer[4]) = (uint32) weather; // Why not just use 0x01/2/3?
	entity_list.QueueClients(owner, outapp);
	safe_delete(outapp);
}

void QuestManager::snow(int weather) {
	QuestManagerCurrentQuestVars();
	zone->zone_weather = weather + 1;
	auto outapp = new EQApplicationPacket(OP_Weather, 8);
	outapp->pBuffer[0] = 0x01;
	*((uint32*) &outapp->pBuffer[4]) = (uint32)weather;
	entity_list.QueueClients(initiator, outapp);
	safe_delete(outapp);
}

void QuestManager::rename(std::string name) {
	QuestManagerCurrentQuestVars();
	if (initiator) {
		std::string current_name = initiator->GetName();
		if (initiator->ChangeFirstName(name)) {
			initiator->Message(
				Chat::White,
				fmt::format(
					"Successfully renamed to {}.",
					name
				).c_str()
			);
		} else {
			initiator->Message(
				Chat::Red,
				fmt::format(
					"Failed to rename {} to {}.",
					current_name, name
				).c_str()
			);
		}
	}
}

void QuestManager::surname(std::string last_name) {
	QuestManagerCurrentQuestVars();
	//Changes the last name.
	if (initiator) {
		initiator->ChangeLastName(last_name);
		initiator->Message(
			Chat::White,
			fmt::format(
				"Your last name has been set to \"{}\".",
				last_name
			).c_str()
		);
	}
}

void QuestManager::permaclass(int class_id) {
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return;
	}

	initiator->SetBaseClass(class_id);
	initiator->Save(2);
	initiator->Kick("Base class change by QuestManager");
}

void QuestManager::permarace(int race_id) {
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return;
	}

	initiator->SetBaseRace(race_id);
	initiator->Save(2);
	initiator->Kick("Base race change by QuestManager");
}

void QuestManager::permagender(int gender_id) {
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return;
	}

	initiator->SetBaseGender(gender_id);
	initiator->Save(2);
	initiator->Kick("Base gender change by QuestManager");
}

uint16 QuestManager::scribespells(uint8 max_level, uint8 min_level) {
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return 0;
	}

	return initiator->ScribeSpells(min_level, max_level);
}

uint16 QuestManager::traindiscs(uint8 max_level, uint8 min_level) {
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return 0;
	}

	return initiator->LearnDisciplines(min_level, max_level);
}

void QuestManager::unscribespells() {
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return;
	}

	initiator->UnscribeSpellAll();
}

void QuestManager::untraindiscs() {
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return;
	}

	initiator->UntrainDiscAll();
}

void QuestManager::givecash(uint32 copper, uint32 silver, uint32 gold, uint32 platinum) {
	QuestManagerCurrentQuestVars();
	if (
		initiator &&
		(
			copper ||
			silver ||
			gold ||
			platinum
		)
	) {
		initiator->CashReward(
			copper,
			silver,
			gold,
			platinum
		);
	}
}

void QuestManager::pvp(const char *mode) {
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return;
	}

	initiator->SetPVP(Strings::ToBool(mode));
}

void QuestManager::movepc(int zone_id, float x, float y, float z, float heading) {
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return;
	}

	initiator->MovePC(zone_id, x, y, z, heading);
}

void QuestManager::gmmove(float x, float y, float z) {
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return;
	}

	initiator->GMMove(x, y, z);
}

void QuestManager::movegrp(int zoneid, float x, float y, float z) {
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return;
	}

	if (Group *g = entity_list.GetGroupByClient(initiator)) {
		g->TeleportGroup(owner, zoneid, 0, x, y, z, 0.0f);
	} else {
		if (Raid *r = entity_list.GetRaidByClient(initiator)) {
			const auto group_id = r->GetGroup(initiator);
			if (EQ::ValueWithin(group_id, 0, MAX_RAID_GROUPS)) {
				r->TeleportGroup(owner, zoneid, 0, x, y, z, 0.0f, group_id);
			} else {
				initiator->MovePC(zoneid, x, y, z, 0.0f);
			}
		} else {
			initiator->MovePC(zoneid, x, y, z, 0.0f);
		}
	}
}

void QuestManager::doanim(int animation_id, int animation_speed, bool ackreq, eqFilterType filter) {
	QuestManagerCurrentQuestVars();

	if (!owner) {
		return;
	}

	owner->DoAnim(animation_id, animation_speed, ackreq, filter);
}

void QuestManager::addskill(int skill_id, int value) {
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return;
	}

	if (!EQ::ValueWithin(skill_id, EQ::skills::Skill1HBlunt, EQ::skills::HIGHEST_SKILL)) {
		return;
	}

	initiator->AddSkill((EQ::skills::SkillType) skill_id, value);
}

void QuestManager::setlanguage(uint8 language_id, uint8 language_skill) {
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return;
	}

	initiator->SetLanguageSkill(language_id, language_skill);
}

void QuestManager::setskill(int skill_id, int value) {
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return;
	}

	if (!EQ::ValueWithin(skill_id, EQ::skills::Skill1HBlunt, EQ::skills::HIGHEST_SKILL)) {
		return;
	}

	initiator->SetSkill((EQ::skills::SkillType) skill_id, value);
}

void QuestManager::setallskill(int value) {
	QuestManagerCurrentQuestVars();
	if (!initiator) {
		return;
	}

	for (const auto& s : EQ::skills::GetSkillTypeMap()) {
		initiator->SetSkill(s.first, value);
	}
}

void QuestManager::attack(const char *client_name) {
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC())
		return;

	Client* getclient = entity_list.GetClientByName(client_name);
	if (getclient && owner->IsAttackAllowed(getclient))
		owner->AddToHateList(getclient,1);
	else
		owner->Say("I am unable to attack %s.", client_name);
}

void QuestManager::attacknpc(int npc_entity_id) {
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC())
		return;

	Mob *it = entity_list.GetMob(npc_entity_id);
	if (it && owner->IsAttackAllowed(it)) {
		owner->AddToHateList(it,1);
	} else {
		if (it)
			owner->Say("I am unable to attack %s.", it->GetName());
		else
			owner->Say("I am unable to locate NPC entity %i", npc_entity_id);
	}
}

void QuestManager::attacknpctype(int npc_type_id) {
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC())
		return;

	Mob *it = entity_list.GetMobByNpcTypeID(npc_type_id);
	if (it && owner->IsAttackAllowed(it)) {
		owner->AddToHateList(it,1);
	} else {
		if (it)
			owner->Say("I am unable to attack %s.", it->GetName());
		else
			owner->Say("I am unable to locate NPC type %i", npc_type_id);
	}
}

void QuestManager::save() {
	QuestManagerCurrentQuestVars();
	if (initiator)
		initiator->Save();
}

void QuestManager::faction(int faction_id, int faction_value, int temp) {
	QuestManagerCurrentQuestVars();
	RunningQuest run = m_running_quests.top();
	if(run.owner->IsCharmed() == false && initiator) {
		if(faction_id != 0 && faction_value != 0) {
			initiator->SetFactionLevel2(
				initiator->CharacterID(),
				faction_id,
				initiator->GetBaseClass(),
				initiator->GetBaseRace(),
				initiator->GetDeity(),
				faction_value,
				temp);
		}
	}
}

void QuestManager::rewardfaction(int faction_id, int faction_value) {
	QuestManagerCurrentQuestVars();
	if (initiator) {
		if (faction_id != 0 && faction_value != 0) {
			zone->LoadFactionAssociation(faction_id);
			initiator->RewardFaction(faction_id, faction_value);
		}
	}
}

void QuestManager::setsky(uint8 new_sky) {
	QuestManagerCurrentQuestVars();
	if (zone)
		zone->newzone_data.sky = new_sky;
	auto outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
	memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
	entity_list.QueueClients(initiator, outapp);
	safe_delete(outapp);
}

void QuestManager::SetGuild(uint32 new_guild_id, uint8 new_rank) {
	QuestManagerCurrentQuestVars();
	if (initiator) {
		guild_mgr.SetGuild(initiator, new_guild_id, new_rank);
	}
}

void QuestManager::CreateGuild(const char *guild_name, const char *leader) {
	QuestManagerCurrentQuestVars();
	uint32 character_id = database.GetCharacterID(leader);
	if (character_id == 0) {
		worldserver.SendEmoteMessage(
			0,
			0,
			AccountStatus::QuestTroupe,
			Chat::Yellow,
			"Guild Error | Guild leader not found."
		);
		return;
	}

	uint32 tmp = guild_mgr.FindGuildByLeader(character_id);
	if (tmp != GUILD_NONE) {
		worldserver.SendEmoteMessage(
			0,
			0,
			AccountStatus::QuestTroupe,
			Chat::Yellow,
			fmt::format(
				"Guild Error | {} already is the leader of {} ({}).",
				leader,
				guild_mgr.GetGuildName(tmp),
				tmp
			).c_str()
		);
	} else {
		uint32 gid = guild_mgr.CreateGuild(guild_name, character_id);
		if (gid == GUILD_NONE) {
			worldserver.SendEmoteMessage(
				0,
				0,
				AccountStatus::QuestTroupe,
				Chat::Yellow,
				"Guild Error | Guild creation failed."
			);
		} else {
			worldserver.SendEmoteMessage(
				0,
				0,
				AccountStatus::QuestTroupe,
				Chat::Yellow,
				fmt::format(
					"Guild Created | Leader: {} ({}) ID: {}",
					leader,
					character_id,
					gid
				).c_str()
			);
			if (!guild_mgr.SetGuild(initiator, gid, GUILD_LEADER)) {
				worldserver.SendEmoteMessage(
					0,
					0,
					AccountStatus::QuestTroupe,
					Chat::Yellow,
					"Unable to set guild leader's guild in the database. Use #guild set."
				);
			}
		}
	}
}

void QuestManager::settime(uint8 new_hour, uint8 new_min, bool update_world /*= true*/)
{
	if (zone)
		zone->SetTime(new_hour, new_min, update_world);
}

void QuestManager::itemlink(int item_id) {
	QuestManagerCurrentQuestVars();
	if (initiator) {
		const EQ::ItemData* item = database.GetItem(item_id);
		if (item == nullptr)
			return;

		EQ::SayLinkEngine linker;
		linker.SetLinkType(EQ::saylink::SayLinkItemData);
		linker.SetItemData(item);

		initiator->Message(Chat::White, "%s tells you, %s", owner->GetCleanName(), linker.GenerateLink().c_str());
	}
}

void QuestManager::signalwith(int npc_id, int signal_id, int wait_ms) {
	if(wait_ms > 0) {
		STimerList.emplace_back(SignalTimer(wait_ms, npc_id, signal_id));
		return;
	} else {
		STimerList.emplace_back(SignalTimer(0, npc_id, signal_id));
		return;
	}
}

void QuestManager::signal(int npc_id, int wait_ms) {
	signalwith(npc_id, 0, wait_ms);
}

void QuestManager::setglobal(const char *varname, const char *newvalue, int options, const char *duration) {
	QuestManagerCurrentQuestVars();
	int qgZoneid = zone->GetZoneID();
	int qgCharid = 0;
	int qgNpcid = owner ? owner->GetNPCTypeID() : 0; // encounter scripts don't have an owner

	/*	options value determines the availability of global variables to NPCs when a quest begins
		------------------------------------------------------------------
			value		npcid		player		zone
		------------------------------------------------------------------
			0			this		this		this
			1			all			this		this
			2			this		all			this
			3			all			all			this
			4			this		this		all
			5			all			this		all
			6			this		all			all
			7			all			all			all
	*/

	if (initiator){ // some events like waypoint and spawn don't have a player involved
		qgCharid=initiator->CharacterID();
	}
	else {
		qgCharid=-qgNpcid;		// make char id negative npc id as a fudge
	}
	if (options < 0 || options > 7) {
		std::cerr << "Invalid options for global var " << varname << " using defaults" << std::endl;
	}	// default = 0 (only this npcid,player and zone)
	else {
		if (options & 1)
			qgNpcid=0;
		if (options & 2)
			qgCharid=0;
		if (options & 4)
			qgZoneid=0;
	}

	InsertQuestGlobal(qgCharid, qgNpcid, qgZoneid, varname, newvalue, QGVarDuration(duration));
}

/* Inserts global variable into quest_globals table */
int QuestManager::InsertQuestGlobal(int charid, int npcid, int zoneid, const char *varname, const char *varvalue, int duration) {

	// Make duration string either "unix_timestamp(now()) + xxx" or "NULL"
	std::string durationText = (duration == INT_MAX)? "NULL": StringFormat("unix_timestamp(now()) + %i", duration);

	/*
		NOTE: this should be escaping the contents of arglist
		npcwise a malicious script can arbitrarily alter the DB
	*/

	std::string query = StringFormat("REPLACE INTO quest_globals "
                                    "(charid, npcid, zoneid, name, value, expdate)"
                                    "VALUES (%i, %i, %i, '%s', '%s', %s)",
                                    charid, npcid, zoneid, varname, varvalue, durationText.c_str());
    auto results = database.QueryDatabase(query);
	if (!results.Success())
		std::cerr << "setglobal error inserting " << varname << " : " << results.ErrorMessage() << std::endl;

	if(!zone)
        return 0;

    /* Delete existing qglobal data and update zone processes */
	auto pack = new ServerPacket(ServerOP_QGlobalDelete, sizeof(ServerQGlobalDelete_Struct));
	ServerQGlobalDelete_Struct *qgd = (ServerQGlobalDelete_Struct *)pack->pBuffer;
	qgd->npc_id = npcid;
	qgd->char_id = charid;
	qgd->zone_id = zoneid;
	qgd->from_zone_id = zone->GetZoneID();
	qgd->from_instance_id = zone->GetInstanceID();
	strcpy(qgd->name, varname);

	entity_list.DeleteQGlobal(std::string((char *)qgd->name), qgd->npc_id, qgd->char_id, qgd->zone_id);
	zone->DeleteQGlobal(std::string((char *)qgd->name), qgd->npc_id, qgd->char_id, qgd->zone_id);

	worldserver.SendPacket(pack);
	safe_delete(pack);

	/* Create new qglobal data and update zone processes */
	pack = new ServerPacket(ServerOP_QGlobalUpdate, sizeof(ServerQGlobalUpdate_Struct));
	ServerQGlobalUpdate_Struct *qgu = (ServerQGlobalUpdate_Struct*) pack->pBuffer;
	qgu->npc_id = npcid;
	qgu->char_id = charid;
	qgu->zone_id = zoneid;

	qgu->expdate = (duration == INT_MAX)? 0xFFFFFFFF: Timer::GetTimeSeconds() + duration;

    strcpy((char*)qgu->name, varname);
    strn0cpy((char*)qgu->value, varvalue, 128);
	qgu->id = results.LastInsertedID();
	qgu->from_zone_id = zone->GetZoneID();
	qgu->from_instance_id = zone->GetInstanceID();

	QGlobal temp;
	temp.npc_id = npcid;
	temp.char_id = charid;
	temp.zone_id = zoneid;
	temp.expdate = qgu->expdate;
	temp.name.assign(qgu->name);
	temp.value.assign(qgu->value);
	entity_list.UpdateQGlobal(qgu->id, temp);
	zone->UpdateQGlobal(qgu->id, temp);

	worldserver.SendPacket(pack);
	safe_delete(pack);

	return 0;
}

void QuestManager::targlobal(const char *varname, const char *value, const char *duration, int qgNpcid, int qgCharid, int qgZoneid) {
	InsertQuestGlobal(qgCharid, qgNpcid, qgZoneid, varname, value, QGVarDuration(duration));
}

void QuestManager::delglobal(const char *varname) {
	QuestManagerCurrentQuestVars();
	int qgZoneid = zone->GetZoneID();
	int qgCharid = 0;
	int qgNpcid = owner ? owner->GetNPCTypeID() : 0; // encounter scripts don't have an owner

	if (initiator) // some events like waypoint and spawn don't have a player involved
		qgCharid=initiator->CharacterID();
	else
		qgCharid=-qgNpcid;		// make char id negative npc id as a fudge

    std::string query = StringFormat("DELETE FROM quest_globals "
                                    "WHERE name = '%s' "
                                    "&& (npcid=0 || npcid=%i) "
                                    "&& (charid=0 || charid=%i) "
                                    "&& (zoneid=%i || zoneid=0)",
                                    varname, qgNpcid, qgCharid, qgZoneid);
    auto results = database.QueryDatabase(query);
	if (!results.Success())
		std::cerr << "delglobal error deleting " << varname << " : " << results.ErrorMessage() << std::endl;

	if(!zone)
        return;

	auto pack = new ServerPacket(ServerOP_QGlobalDelete, sizeof(ServerQGlobalDelete_Struct));
	ServerQGlobalDelete_Struct *qgu = (ServerQGlobalDelete_Struct *)pack->pBuffer;

	qgu->npc_id = qgNpcid;
	qgu->char_id = qgCharid;
	qgu->zone_id = qgZoneid;
	strcpy(qgu->name, varname);

	entity_list.DeleteQGlobal(std::string((char *)qgu->name), qgu->npc_id, qgu->char_id, qgu->zone_id);
	zone->DeleteQGlobal(std::string((char *)qgu->name), qgu->npc_id, qgu->char_id, qgu->zone_id);

	worldserver.SendPacket(pack);
	safe_delete(pack);
}

// Converts duration string to duration value (in seconds)
// Return of INT_MAX indicates infinite duration
int QuestManager::QGVarDuration(const char *fmt)
{
	int duration = 0;

	// format:	Y#### or D## or H## or M## or S## or T###### or C#######

	int len = strlen(fmt);

	// Default to no duration
	if (len < 1)
		return 0;

	// Set val to value after type character
	// e.g., for "M3924", set to 3924
	int val = Strings::ToInt(&fmt[0] + 1);

	switch (fmt[0])
	{
		// Forever
		case 'F':
		case 'f':
			duration = INT_MAX;
			break;
		// Years
		case 'Y':
		case 'y':
			duration = val * 31556926;
			break;
		case 'D':
		case 'd':
			duration = val * 86400;
			break;
		// Hours
		case 'H':
		case 'h':
			duration = val * 3600;
			break;
		// Minutes
		case 'M':
		case 'm':
			duration = val * 60;
			break;
		// Seconds
		case 'S':
		case 's':
			duration = val;
			break;
		// Invalid
		default:
			duration = 0;
			break;
	}

	return duration;
}

void QuestManager::ding() {
	QuestManagerCurrentQuestVars();
	//makes a sound.
	if (initiator)
		initiator->SendSound();

}

void QuestManager::rebind(int zone_id, const glm::vec3& location) {
	QuestManagerCurrentQuestVars();
	if(initiator) {
		initiator->SetBindPoint(0, zone_id, 0, location);
	}
}

void QuestManager::rebind(int zone_id, const glm::vec4& location) {
	QuestManagerCurrentQuestVars();
	if(initiator) {
		initiator->SetBindPoint2(0, zone_id, 0, location);
	}
}

void QuestManager::start(int32 wp) {
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC())
		return;

	owner->CastToNPC()->AssignWaypoints(wp);
}

void QuestManager::stop() {
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC())
		return;

	owner->CastToNPC()->StopWandering();
}

void QuestManager::pause(int duration) {
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC())
		return;

	owner->CastToNPC()->PauseWandering(duration);
}

void QuestManager::moveto(const glm::vec4& position, bool saveguardspot) {
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC())
		return;

	owner->CastToNPC()->MoveTo(position, saveguardspot);
}

void QuestManager::resume() {
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC())
		return;

	owner->CastToNPC()->ResumeWandering();
}

void QuestManager::addldonpoints(uint32 theme_id, int points)
{
	QuestManagerCurrentQuestVars();
	if (initiator) {
		initiator->UpdateLDoNPoints(theme_id, points);
	}
}

void QuestManager::addldonloss(uint32 theme_id)
{
	QuestManagerCurrentQuestVars();
	if (initiator) {
		initiator->UpdateLDoNWinLoss(theme_id);
	}
}

void QuestManager::addldonwin(uint32 theme_id)
{
	QuestManagerCurrentQuestVars();
	if (initiator) {
		initiator->UpdateLDoNWinLoss(theme_id, true);
	}
}

void QuestManager::removeldonloss(uint32 theme_id)
{
	QuestManagerCurrentQuestVars();
	if (initiator) {
		initiator->UpdateLDoNWinLoss(theme_id, false, true);
	}
}

void QuestManager::removeldonwin(uint32 theme_id)
{
	QuestManagerCurrentQuestVars();
	if (initiator) {
		initiator->UpdateLDoNWinLoss(theme_id, true, true);
	}
}

void QuestManager::setnexthpevent(int at) {
	QuestManagerCurrentQuestVars();
	if (owner)
		owner->SetNextHPEvent(at);
}

void QuestManager::setnextinchpevent(int at) {
	QuestManagerCurrentQuestVars();
	if (owner)
		owner->SetNextIncHPEvent(at);
}

void QuestManager::respawn(int npcTypeID, int grid) {
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC())
		return;

	RunningQuest e = m_running_quests.top();
	e.depop_npc = true;
	m_running_quests.pop();
	m_running_quests.push(e);

	const NPCType* npcType = nullptr;
	if ((npcType = content_db.LoadNPCTypesData(npcTypeID)))
	{
		owner = new NPC(npcType, nullptr, owner->GetPosition(), GravityBehavior::Water);
		owner->CastToNPC()->AddLootTable();
		if (owner->CastToNPC()->DropsGlobalLoot())
			owner->CastToNPC()->CheckGlobalLootTables();
		entity_list.AddNPC(owner->CastToNPC(),true,true);
		if(grid > 0)
			owner->CastToNPC()->AssignWaypoints(grid);
	}
}

void QuestManager::set_proximity_range(float x_range, float y_range, float z_range, bool enable_say)
{
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC()) {
		return;
	}

	auto n = owner->CastToNPC();

	entity_list.AddProximity(n);

	n->proximity->min_x         = n->GetX() - x_range;
	n->proximity->max_x         = n->GetX() + x_range;
	n->proximity->min_y         = n->GetY() - y_range;
	n->proximity->max_y         = n->GetY() + y_range;
	n->proximity->min_z         = n->GetZ() - z_range;
	n->proximity->max_z         = n->GetZ() + z_range;
	n->proximity->say           = enable_say;
	n->proximity->proximity_set = true;

	if (enable_say) {
		HaveProximitySays = enable_say;
	}
}

void QuestManager::set_proximity(float min_x, float max_x, float min_y, float max_y, float min_z, float max_z, bool enable_say)
{
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC()) {
		return;
	}

	auto n = owner->CastToNPC();

	entity_list.AddProximity(n);

	n->proximity->min_x         = min_x;
	n->proximity->max_x         = max_x;
	n->proximity->min_y         = min_y;
	n->proximity->max_y         = max_y;
	n->proximity->min_z         = min_z;
	n->proximity->max_z         = max_z;
	n->proximity->say           = enable_say;
	n->proximity->proximity_set = true;

	if (enable_say) {
		HaveProximitySays = enable_say;
	}
}

void QuestManager::clear_proximity() {
	QuestManagerCurrentQuestVars();
	if(!owner || !owner->IsNPC())
		return;

	entity_list.RemoveProximity(owner->GetID());
	safe_delete(owner->CastToNPC()->proximity);
}

void QuestManager::enable_proximity_say() {
	HaveProximitySays = true;
}

void QuestManager::disable_proximity_say() {
	HaveProximitySays = false;
}

void QuestManager::setanim(int npc_type, int animnum) {
	//adds appearance changes
	Mob* thenpc = entity_list.GetMobByNpcTypeID(npc_type);
	if(!thenpc || animnum < 0 || animnum >= _eaMaxAppearance)
		return;
	thenpc->SetAppearance(EmuAppearance(animnum));
}

//displays an in game path based on a waypoint grid
void QuestManager::showgrid(int grid_id)
{
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return;
	}

	std::vector<FindPerson_Point> v;

	v.push_back(
		FindPerson_Point{
			.y = initiator->GetY(),
			.x = initiator->GetX(),
			.z = initiator->GetZ()
		}
	);

	const auto& l = GridEntriesRepository::GetWhere(
		content_db,
		fmt::format(
			"`gridid` = {} AND `zoneid` = {} ORDER BY `number`",
			grid_id,
			zone->GetZoneID()
		)
	);

	if (l.empty()) {
		return;
	}

	for (const auto& e : l) {
		v.push_back(
			FindPerson_Point{
				.y = e.y,
				.x = e.x,
				.z = e.z
			}
		);
	}

    initiator->SendPathPacket(v);

}

//change the value of a spawn condition
void QuestManager::spawn_condition(const char *zone_short, uint32 instance_id, uint16 condition_id, short new_value) {
	zone->spawn_conditions.SetCondition(zone_short, instance_id, condition_id, new_value);
}

//get the value of a spawn condition
short QuestManager::get_spawn_condition(const char *zone_short, uint32 instance_id, uint16 condition_id) {
	return(zone->spawn_conditions.GetCondition(zone_short, instance_id, condition_id));
}

//toggle a spawn event
void QuestManager::toggle_spawn_event(int event_id, bool enable, bool strict, bool reset_base) {
	zone->spawn_conditions.ToggleEvent(event_id, enable, strict, reset_base);
}

bool QuestManager::has_zone_flag(int zone_id) {
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return false;
	}

	return initiator->HasZoneFlag(zone_id);
}

void QuestManager::set_zone_flag(int zone_id) {
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return;
	}

	initiator->SetZoneFlag(zone_id);
}

void QuestManager::clear_zone_flag(int zone_id) {
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return;
	}

	initiator->ClearZoneFlag(zone_id);
}

void QuestManager::sethp(int64 hpperc) {
	QuestManagerCurrentQuestVars();

	if (!owner) {
		return;
	}

	int64 newhp = (owner->GetMaxHP() * (100 - hpperc)) / 100;
	owner->Damage(owner, newhp, SPELL_UNKNOWN, EQ::skills::SkillHandtoHand, false, 0, false);
}

bool QuestManager::summonburiedplayercorpse(uint32 char_id, const glm::vec4& position) {
	bool Result = false;

	if(char_id <= 0)
        return false;

	Corpse* PlayerCorpse = database.SummonBuriedCharacterCorpses(char_id, zone->GetZoneID(), zone->GetInstanceID(), position);
	if(!PlayerCorpse)
		return false;

	return true;
}

bool QuestManager::summonallplayercorpses(uint32 char_id, const glm::vec4& position) {

	if(char_id <= 0)
        return false;

	Client* c = entity_list.GetClientByCharID(char_id);
	c->SummonAllCorpses(position);

	return true;
}

int64 QuestManager::getplayercorpsecount(uint32 character_id) {
	return character_id ? database.CountCharacterCorpses(character_id) : 0;

}

int64 QuestManager::getplayercorpsecountbyzoneid(uint32 character_id, uint32 zone_id) {
	return (character_id && zone_id) ? database.CountCharacterCorpsesByZoneID(character_id, zone_id) : 0;
}

int64 QuestManager::getplayerburiedcorpsecount(uint32 character_id) {
	return character_id ? database.GetCharacterBuriedCorpseCount(character_id) : 0;
}

bool QuestManager::buryplayercorpse(uint32 char_id)
{
	bool Result = false;

	if(char_id > 0)
	{
		uint32 PlayerCorpse = database.GetFirstCorpseID(char_id);
		if(PlayerCorpse > 0)
		{
			database.BuryCharacterCorpse(PlayerCorpse);
			Corpse* corpse = entity_list.GetCorpseByDBID(PlayerCorpse);
			if(corpse)
			{
				corpse->Save();
				corpse->DepopPlayerCorpse();
			}
			else
			{
				Client *c = entity_list.GetClientByCharID(char_id);
				c->DepopPlayerCorpse(PlayerCorpse);
			}
			Result = true;
		}
	}
	return Result;
}

void QuestManager::forcedooropen(uint32 doorid, bool altmode) {
	Doors* d = entity_list.FindDoor(doorid);
	if(d){
		if(GetInitiator())
			d->ForceOpen(GetInitiator(), altmode);
		else if(GetOwner())
			d->ForceOpen(GetOwner(), altmode);
	}
}

void QuestManager::forcedoorclose(uint32 doorid, bool altmode) {
	Doors* d = entity_list.FindDoor(doorid);
	if(d){
		if(GetInitiator())
			d->ForceClose(GetInitiator(), altmode);
		else if(GetOwner())
			d->ForceClose(GetOwner(), altmode);
	}
}

void QuestManager::toggledoorstate(uint32 doorid) {
	Doors* d = entity_list.FindDoor(doorid);
	if(d){
		if(GetInitiator())
			d->ToggleState(GetInitiator());
		else if(GetOwner())
			d->ToggleState(GetOwner());
	}
}

bool QuestManager::isdooropen(uint32 doorid) {
	Doors* d = entity_list.FindDoor(doorid);
	if(d){
		return d->IsDoorOpen();
	}
	return false;
}

void QuestManager::npcrace(uint16 race_id)
{
	QuestManagerCurrentQuestVars();

	if (!owner) {
		return;
	}

	owner->SendIllusionPacket(
		AppearanceStruct{
			.race_id = race_id,
		}
	);
}

void QuestManager::npcgender(uint8 gender_id)
{
	QuestManagerCurrentQuestVars();

	if (!owner) {
		return;
	}

	owner->SendIllusionPacket(
		AppearanceStruct{
			.gender_id = gender_id,
			.race_id = owner->GetRace(),
		}
	);
}

void QuestManager::npcsize(float size)
{
	QuestManagerCurrentQuestVars();

	if (!owner) {
		return;
	}

	owner->ChangeSize(size, true);
}

void QuestManager::npctexture(uint8 texture)
{
	QuestManagerCurrentQuestVars();

	if (!owner) {
		return;
	}

	owner->SendIllusionPacket(
		AppearanceStruct{
			.race_id = owner->GetRace(),
			.texture = texture,
		}
	);
}

void QuestManager::playerrace(uint16 race_id)
{
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return;
	}

	initiator->SendIllusionPacket(
		AppearanceStruct{
			.race_id = race_id,
		}
	);
}

void QuestManager::playergender(uint8 gender_id)
{
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return;
	}

	initiator->SendIllusionPacket(
		AppearanceStruct{
			.gender_id = gender_id,
			.race_id = initiator->GetRace(),
		}
	);
}

void QuestManager::playersize(float size)
{
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return;
	}

	initiator->ChangeSize(size, true);
}

void QuestManager::playertexture(uint8 texture)
{
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return;
	}

	initiator->SendIllusionPacket(
		AppearanceStruct{
			.race_id = initiator->GetRace(),
			.texture = texture,
		}
	);
}

void QuestManager::playerfeature(const char* feature, int setting)
{
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return;
	}

	uint16 Race            = initiator->GetRace();
	uint8  Gender          = initiator->GetGender();
	uint8  Texture         = 0xFF;
	uint8  HelmTexture     = 0xFF;
	uint8  HairColor       = initiator->GetHairColor();
	uint8  BeardColor      = initiator->GetBeardColor();
	uint8  EyeColor1       = initiator->GetEyeColor1();
	uint8  EyeColor2       = initiator->GetEyeColor2();
	uint8  HairStyle       = initiator->GetHairStyle();
	uint8  LuclinFace      = initiator->GetLuclinFace();
	uint8  Beard           = initiator->GetBeard();
	uint32 DrakkinHeritage = initiator->GetDrakkinHeritage();
	uint32 DrakkinTattoo   = initiator->GetDrakkinTattoo();
	uint32 DrakkinDetails  = initiator->GetDrakkinDetails();
	float  Size            = initiator->GetSize();

	if (!strcasecmp(feature, "race")) {
		Race = setting;
	} else if (!strcasecmp(feature, "gender")) {
		Gender = setting;
	} else if (!strcasecmp(feature, "texture")) {
		Texture = setting;
	} else if (!strcasecmp(feature, "helm")) {
		HelmTexture = setting;
	} else if (!strcasecmp(feature, "haircolor")) {
		HairColor = setting;
	} else if (!strcasecmp(feature, "beardcolor")) {
		BeardColor = setting;
	} else if (!strcasecmp(feature, "eyecolor1")) {
		EyeColor1 = setting;
	} else if (!strcasecmp(feature, "eyecolor2")) {
		EyeColor2 = setting;
	} else if (!strcasecmp(feature, "hair")) {
		HairStyle = setting;
	} else if (!strcasecmp(feature, "face")) {
		LuclinFace = setting;
	} else if (!strcasecmp(feature, "beard")) {
		Beard = setting;
	} else if (!strcasecmp(feature, "heritage")) {
		DrakkinHeritage = setting;
	} else if (!strcasecmp(feature, "tattoo")) {
		DrakkinTattoo = setting;
	} else if (!strcasecmp(feature, "details")) {
		DrakkinDetails = setting;
	} else if (!strcasecmp(feature, "size")) {
		Size = (float) setting / 10;    //dividing by 10 to allow 1 decimal place for adjusting size
	} else {
		return;
	}

	initiator->SendIllusionPacket(
		AppearanceStruct{
			.beard = Beard,
			.beard_color = BeardColor,
			.drakkin_details = DrakkinDetails,
			.drakkin_heritage = DrakkinHeritage,
			.drakkin_tattoo = DrakkinTattoo,
			.eye_color_one = EyeColor1,
			.eye_color_two = EyeColor2,
			.face = LuclinFace,
			.gender_id = Gender,
			.hair = HairStyle,
			.hair_color = HairColor,
			.helmet_texture = HelmTexture,
			.race_id = Race,
			.size = Size,
			.texture = Texture,
		}
	);
}

void QuestManager::npcfeature(const char* feature, int setting)
{
	QuestManagerCurrentQuestVars();

	if (!owner) {
		return;
	}

	uint16 Race            = owner->GetRace();
	uint8  Gender          = owner->GetGender();
	uint8  Texture         = owner->GetTexture();
	uint8  HelmTexture     = owner->GetHelmTexture();
	uint8  HairColor       = owner->GetHairColor();
	uint8  BeardColor      = owner->GetBeardColor();
	uint8  EyeColor1       = owner->GetEyeColor1();
	uint8  EyeColor2       = owner->GetEyeColor2();
	uint8  HairStyle       = owner->GetHairStyle();
	uint8  LuclinFace      = owner->GetLuclinFace();
	uint8  Beard           = owner->GetBeard();
	uint32 DrakkinHeritage = owner->GetDrakkinHeritage();
	uint32 DrakkinTattoo   = owner->GetDrakkinTattoo();
	uint32 DrakkinDetails  = owner->GetDrakkinDetails();
	float  Size            = owner->GetSize();

	if (!strcasecmp(feature, "race")) {
		Race = setting;
	} else if (!strcasecmp(feature, "gender")) {
		Gender = setting;
	} else if (!strcasecmp(feature, "texture")) {
		Texture = setting;
	} else if (!strcasecmp(feature, "helm")) {
		HelmTexture = setting;
	} else if (!strcasecmp(feature, "haircolor")) {
		HairColor = setting;
	} else if (!strcasecmp(feature, "beardcolor")) {
		BeardColor = setting;
	} else if (!strcasecmp(feature, "eyecolor1")) {
		EyeColor1 = setting;
	} else if (!strcasecmp(feature, "eyecolor2")) {
		EyeColor2 = setting;
	} else if (!strcasecmp(feature, "hair")) {
		HairStyle = setting;
	} else if (!strcasecmp(feature, "face")) {
		LuclinFace = setting;
	} else if (!strcasecmp(feature, "beard")) {
		Beard = setting;
	} else if (!strcasecmp(feature, "heritage")) {
		DrakkinHeritage = setting;
	} else if (!strcasecmp(feature, "tattoo")) {
		DrakkinTattoo = setting;
	} else if (!strcasecmp(feature, "details")) {
		DrakkinDetails = setting;
	} else if (!strcasecmp(feature, "size")) {
		Size = (float) setting / 10;    //dividing by 10 to allow 1 decimal place for adjusting size
	} else {
		return;
	}

	owner->SendIllusionPacket(
		AppearanceStruct{
			.beard = Beard,
			.beard_color = BeardColor,
			.drakkin_details = DrakkinDetails,
			.drakkin_heritage = DrakkinHeritage,
			.drakkin_tattoo = DrakkinTattoo,
			.eye_color_one = EyeColor1,
			.eye_color_two = EyeColor2,
			.face = LuclinFace,
			.gender_id = Gender,
			.hair = HairStyle,
			.hair_color = HairColor,
			.helmet_texture = HelmTexture,
			.race_id = Race,
			.size = Size,
			.texture = Texture,
		}
	);
}

void QuestManager::popup(const char *title, const char *text, uint32 popupid, uint32 buttons, uint32 Duration)
{
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return;
	}

	initiator->SendPopupToClient(title, text, popupid, buttons, Duration);
}

int QuestManager::createbotcount(uint8 class_id) {
	QuestManagerCurrentQuestVars();
	if (initiator) {
		return initiator->GetBotCreationLimit(class_id);
	}

	return RuleI(Bots, CreationLimit);
}

int QuestManager::spawnbotcount(uint8 class_id) {
	QuestManagerCurrentQuestVars();
	if (initiator) {
		return initiator->GetBotSpawnLimit(class_id);
	}

	return RuleI(Bots, SpawnLimit);
}

bool QuestManager::botquest()
{
	return RuleB(Bots, QuestableSpawnLimit);
}

bool QuestManager::createBot(const char *name, const char *lastname, uint8 level, uint16 race, uint8 botclass, uint8 gender)
{
	QuestManagerCurrentQuestVars();

	if (initiator) {
		auto bot_creation_limit = initiator->GetBotCreationLimit();
		auto bot_creation_limit_class = initiator->GetBotCreationLimit(botclass);
		auto bot_spawn_limit = initiator->GetBotSpawnLimit();
		auto bot_spawn_limit_class = initiator->GetBotSpawnLimit(botclass);

		uint32 bot_count = 0;
		uint32 bot_class_count = 0;
		if (!database.botdb.QueryBotCount(initiator->CharacterID(), botclass, bot_count, bot_class_count)) {
			initiator->Message(Chat::White, "Failed to query bot count.");
			return false;
		}

		if (bot_creation_limit >= 0 && bot_count >= bot_creation_limit) {
			std::string message;

			if (bot_creation_limit) {
				message = fmt::format(
					"You cannot create anymore than {} bot{}.",
					bot_creation_limit,
					bot_creation_limit != 1 ? "s" : ""
				);
			} else {
				message = "You cannot create any bots.";
			}

			initiator->Message(Chat::White, message.c_str());
			return false;
		}

		if (bot_creation_limit_class >= 0 && bot_class_count >= bot_creation_limit_class) {
			std::string message;

			if (bot_creation_limit_class) {
				message = fmt::format(
					"You cannot create anymore than {} {} bot{}.",
					bot_creation_limit_class,
					GetClassIDName(botclass),
					bot_creation_limit_class != 1 ? "s" : ""
				);
			} else {
				message = fmt::format(
					"You cannot create any {} bots.",
					GetClassIDName(botclass)
				);
			}

			initiator->Message(Chat::White, message.c_str());
			return false;
		}

		auto spawned_bot_count = Bot::SpawnedBotCount(initiator->CharacterID());

		if (bot_spawn_limit >= 0 && spawned_bot_count >= bot_spawn_limit) {
			if (!initiator->GetGM()) {
				std::string message;
				if (bot_spawn_limit) {
					message = fmt::format(
						"You cannot have more than {} spawned bot{}.",
						bot_spawn_limit,
						bot_spawn_limit != 1 ? "s" : ""
					);
				} else {
					message = "You are not currently allowed to spawn any bots.";
				}

				initiator->Message(Chat::White, message.c_str());
				return false;
			} else {
				initiator->Message(Chat::White, "Your GM flag allows you to bypass bot spawn limits.");
			}
		}

		auto spawned_bot_count_class = Bot::SpawnedBotCount(initiator->CharacterID(), botclass);

		if (bot_spawn_limit_class >= 0 && spawned_bot_count_class >= bot_spawn_limit_class) {
			if (!initiator->GetGM()) {
				std::string message;
				if (bot_spawn_limit_class) {
					message = fmt::format(
						"You cannot have more than {} spawned {} bot{}.",
						bot_spawn_limit_class,
						GetClassIDName(botclass),
						bot_spawn_limit_class != 1 ? "s" : ""
					);
				} else {
					message = fmt::format(
						"You are not currently allowed to spawn any {} bots.",
						GetClassIDName(botclass)
					);
				}

				initiator->Message(Chat::White, message.c_str());
				return false;
			} else {
				initiator->Message(Chat::White, "Your GM flag allows you to bypass bot class-based spawn limits.");
			}
		}

		std::string test_name = name;
		bool available_flag = false;
		if (!database.botdb.QueryNameAvailability(test_name, available_flag)) {
			initiator->Message(
				Chat::White,
				fmt::format(
					"Failed to query name availability for '{}'.",
					test_name
				).c_str()
			);
			return false;
		}

		if (!available_flag) {
			initiator->Message(
				Chat::White,
				fmt::format(
					"The name {} is already being used or is invalid. Please choose a different name.",
					test_name
				).c_str()
			);
			return false;
		}

		Bot* new_bot = new Bot(Bot::CreateDefaultNPCTypeStructForBot(name, lastname, level, race, botclass, gender), initiator);

		if (!new_bot->IsValidRaceClassCombo()) {
			initiator->Message(Chat::White, "That Race/Class combination cannot be created.");
			return false;
		}

		if (!new_bot->IsValidName()) {
			initiator->Message(
				Chat::White,
				fmt::format(
					"{} has invalid characters. You can use only the A-Z, a-z and _ characters in a bot name and it must be between 4 and 15 characters long.",
					new_bot->GetCleanName()
				).c_str()
			);
			return false;
		}

		// Now that all validation is complete, we can save our newly created bot
		if (!new_bot->Save()) {
			initiator->Message(
				Chat::White,
				fmt::format(
					"Unable to save {} as a bot.",
					new_bot->GetCleanName()
				).c_str()
			);
		} else {
			new_bot->AddBotStartingItems(race, botclass);

			initiator->Message(
				Chat::White,
				fmt::format(
					"{} saved as bot ID {}.",
					new_bot->GetCleanName(),
					new_bot->GetBotID()
				).c_str()
			);

			if (parse->PlayerHasQuestSub(EVENT_BOT_CREATE)) {
				const auto &export_string = fmt::format(
					"{} {} {} {} {}",
					name,
					new_bot->GetBotID(),
					race,
					botclass,
					gender
				);

				parse->EventPlayer(EVENT_BOT_CREATE, initiator, export_string, 0);
			}

			return true;
		}
	}
	return false;
}

void QuestManager::taskselector(const std::vector<int>& tasks, bool ignore_cooldown) {
	QuestManagerCurrentQuestVars();
	if(RuleB(TaskSystem, EnableTaskSystem) && initiator && owner)
		initiator->TaskQuestSetSelector(owner, tasks, ignore_cooldown);
}
void QuestManager::enabletask(int taskcount, int *tasks) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		initiator->EnableTask(taskcount, tasks);
}

void QuestManager::disabletask(int taskcount, int *tasks) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		initiator->DisableTask(taskcount, tasks);
}

bool QuestManager::istaskenabled(int taskid) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		return initiator->IsTaskEnabled(taskid);

	return false;
}

void QuestManager::tasksetselector(int tasksetid, bool ignore_cooldown) {
	QuestManagerCurrentQuestVars();
	Log(Logs::General, Logs::Tasks, "[UPDATE] TaskSetSelector called for task set %i", tasksetid);
	if(RuleB(TaskSystem, EnableTaskSystem) && initiator && owner)
		initiator->TaskSetSelector(owner, tasksetid, ignore_cooldown);
}

bool QuestManager::istaskactive(int task) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		return initiator->IsTaskActive(task);

	return false;
}

bool QuestManager::istaskactivityactive(int task, int activity) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		return initiator->IsTaskActivityActive(task, activity);

	return false;
}

int QuestManager::gettaskactivitydonecount(int task, int activity) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		return initiator->GetTaskActivityDoneCountFromTaskID(task, activity);

	return 0;

}

void QuestManager::updatetaskactivity(int task, int activity, int count, bool ignore_quest_update /*= false*/)
{
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		initiator->UpdateTaskActivity(task, activity, count, ignore_quest_update);
}

void QuestManager::resettaskactivity(int task, int activity) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		initiator->ResetTaskActivity(task, activity);
}

void QuestManager::assigntask(int taskid, bool enforce_level_requirement) {
	QuestManagerCurrentQuestVars();

	if (RuleB(TaskSystem, EnableTaskSystem) && initiator && owner)
		initiator->AssignTask(taskid, owner->GetID(), enforce_level_requirement);
}

void QuestManager::failtask(int taskid) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		initiator->FailTask(taskid);
}

bool QuestManager::completetask(int task_id) {
	QuestManagerCurrentQuestVars();

	if (!RuleB(TaskSystem, EnableTaskSystem) || !initiator) {
		return false;

	}

	return initiator->CompleteTask(task_id);
}

bool QuestManager::uncompletetask(int task_id) {
	QuestManagerCurrentQuestVars();

	if (!RuleB(TaskSystem, EnableTaskSystem) || !initiator) {
		return false;
	}

	return initiator->UncompleteTask(task_id);
}

int QuestManager::tasktimeleft(int taskid) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		return initiator->TaskTimeLeft(taskid);

	return -1;
}

int QuestManager::enabledtaskcount(int taskset) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		return initiator->EnabledTaskCount(taskset);

	return -1;
}

int QuestManager::firsttaskinset(int taskset) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem))
		return TaskManager::Instance()->FirstTaskInSet(taskset);

	return -1;
}

int QuestManager::lasttaskinset(int taskset) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem))
		return TaskManager::Instance()->LastTaskInSet(taskset);

	return -1;
}

int QuestManager::nexttaskinset(int taskset, int taskid) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem))
		return TaskManager::Instance()->NextTaskInSet(taskset, taskid);

	return -1;
}

int QuestManager::activespeaktask() {
	QuestManagerCurrentQuestVars();

	if (RuleB(TaskSystem, EnableTaskSystem) && initiator && owner && owner->IsNPC())
		return initiator->ActiveSpeakTask(owner->CastToNPC());
	return 0;
}

int QuestManager::activespeakactivity(int taskid) {
	QuestManagerCurrentQuestVars();

	if (RuleB(TaskSystem, EnableTaskSystem) && initiator && owner && owner->IsNPC())
		return initiator->ActiveSpeakActivity(owner->CastToNPC(), taskid);

	return 0;
}

bool QuestManager::istaskcompleted(int task_id)
{
	QuestManagerCurrentQuestVars();

	if (initiator && RuleB(TaskSystem, EnableTaskSystem)) {
		return initiator->IsTaskCompleted(task_id);
	}

	return false;
}

bool QuestManager::aretaskscompleted(const std::vector<int>& task_ids)
{
	QuestManagerCurrentQuestVars();

	if (initiator && RuleB(TaskSystem, EnableTaskSystem)) {
		return initiator->AreTasksCompleted(task_ids);
	}

	return false;
}

int QuestManager::activetasksinset(int taskset) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		return initiator->ActiveTasksInSet(taskset);

	return -1;
}

int QuestManager::completedtasksinset(int taskset) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		return initiator->CompletedTasksInSet(taskset);

	return -1;
}

bool QuestManager::istaskappropriate(int task) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		return TaskManager::Instance()->ValidateLevel(task, initiator->GetLevel());

	return false;
}

std::string QuestManager::gettaskname(uint32 task_id) {
	QuestManagerCurrentQuestVars();

	if (RuleB(TaskSystem, EnableTaskSystem)) {
		return TaskManager::Instance()->GetTaskName(task_id);
	}

	return std::string();
}

int QuestManager::GetCurrentDzTaskID() {
	QuestManagerCurrentQuestVars();

	if (RuleB(TaskSystem, EnableTaskSystem) && zone) {
		return TaskManager::Instance()->GetCurrentDzTaskID();
	}

	return 0;
}

void QuestManager::EndCurrentDzTask(bool send_fail) {
	QuestManagerCurrentQuestVars();

	if (RuleB(TaskSystem, EnableTaskSystem) && zone) {
		TaskManager::Instance()->EndCurrentDzTask(send_fail);
	}
}

void QuestManager::clearspawntimers() {
	if (!zone) {
        return;
	}

	zone->ClearSpawnTimers();
}

void QuestManager::ze(int type, const char *str) {
	entity_list.Message(0, type, str);
}

void QuestManager::we(int type, const char *str) {
	worldserver.SendEmoteMessage(
		0,
		0,
		type,
		str
	);
}

void QuestManager::SendChannelMessage(uint8 channel_number, uint32 guild_id, uint8 language_id, uint8 language_skill, const char* message)
{
	worldserver.SendChannelMessage(0, 0, channel_number, guild_id, language_id, language_skill, message);
}

void QuestManager::SendChannelMessage(Client* from, uint8 channel_number, uint32 guild_id, uint8 language_id, uint8 language_skill, const char* message)
{
	worldserver.SendChannelMessage(from, 0, channel_number, guild_id, language_id, language_skill, message);
}

void QuestManager::SendChannelMessage(Client* from, const char* to, uint8 channel_number, uint32 guild_id, uint8 language_id, uint8 language_skill, const char* message)
{
	worldserver.SendChannelMessage(from, to, channel_number, guild_id, language_id, language_skill, message);
}

void QuestManager::message(uint32 type, const char *message) {
	QuestManagerCurrentQuestVars();
	if (!initiator) {
		return;
	}

	initiator->Message(type, message);
}

void QuestManager::whisper(const char *message) {
	QuestManagerCurrentQuestVars();
	if (!initiator || !owner) {
		return;
	}

	initiator->Message(
		Chat::EchoChat1,
		fmt::format(
			"{} whispers, '{}'",
			owner->GetCleanName(),
			message
		).c_str()
	);
}

int QuestManager::getlevel(uint8 type)
{
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return 0;
	}

	if (type == 0) {
		return initiator->GetLevel();
	} else if (type == 1) {
		if (Group *g = entity_list.GetGroupByClient(initiator)) {
			return g->GetAvgLevel();
		} else {
			return 0;
		}
	} else if (type == 2) {
		if (Raid *r = entity_list.GetRaidByClient(initiator)) {
			return r->GetAvgLevel();
		} else {
			return 0;
		}
	} else if (type == 3) {
		if (Raid *r = entity_list.GetRaidByClient(initiator)) {
			return r->GetAvgLevel();
		}

		if (Group *g = entity_list.GetGroupByClient(initiator)) {
			return g->GetAvgLevel();
		} else {
			return initiator->GetLevel();
		}
	} else if (type == 4) {
		return initiator->CastToClient()->GetLevel2();
	} else {
		return 0;
	}
}

uint16 QuestManager::CreateGroundObject(uint32 itemid, const glm::vec4& position, uint32 decay_time)
{
	uint16 entid = 0; //safety check
	entid = entity_list.CreateGroundObject(itemid, position, decay_time);
	return entid;
}

uint16 QuestManager::CreateGroundObjectFromModel(const char *model, const glm::vec4& position, uint8 type, uint32 decay_time)
{
	uint16 entid = 0; //safety check
	entid = entity_list.CreateGroundObjectFromModel(model, position, type, decay_time);
	return entid;
}

void QuestManager::ModifyNPCStat(std::string stat, std::string value)
{
	QuestManagerCurrentQuestVars();
	if (owner && owner->IsNPC()) {
		owner->CastToNPC()->ModifyNPCStat(stat, value);
	}
}

int QuestManager::collectitems_processSlot(
	int16 slot_id,
	uint32 item_id,
	bool remove
) {
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return 0;
	}

	const auto item = initiator->GetInv().GetItem(slot_id);

	// If we have found matching item, add quantity
	if (item && item->GetID() == item_id) {
		// If item is stackable, add its charges (quantity)
		const auto quantity = item->IsStackable() ? item->GetCharges() : 1;

		// Remove item from inventory
		if (remove) {
			initiator->DeleteItemInInventory(slot_id, 0, true);
		}

		return quantity;
	}

	return 0;
}

// Returns number of item_id that exist in inventory
// If remove is true, items are removed as they are counted.
int QuestManager::collectitems(uint32 item_id, bool remove)
{
	int quantity = 0;
	int slot_id;

	for (slot_id = EQ::invslot::GENERAL_BEGIN; slot_id <= EQ::invslot::GENERAL_END; ++slot_id) {
		quantity += collectitems_processSlot(slot_id, item_id, remove);
	}

	for (slot_id = EQ::invbag::GENERAL_BAGS_BEGIN; slot_id <= EQ::invbag::GENERAL_BAGS_END; ++slot_id) {
		quantity += collectitems_processSlot(slot_id, item_id, remove);
	}

	return quantity;
}

uint32 QuestManager::countitem(uint32 item_id) {
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return 0;
	}

	return initiator->CountItem(item_id);
}

void QuestManager::removeitem(uint32 item_id, uint32 quantity) {
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return;
	}

	initiator->RemoveItem(item_id, quantity);
}

void QuestManager::UpdateSpawnTimer(uint32 spawn2_id, uint32 new_time)
{
	bool found = false;

	database.UpdateRespawnTime(spawn2_id, 0, (new_time / 1000));

	LinkedListIterator<Spawn2*> iterator(zone->spawn2_list);

	iterator.Reset();

	while (iterator.MoreElements()) {
		if (iterator.GetData()->GetID() == spawn2_id) {
			if (!iterator.GetData()->NPCPointerValid()) {
				iterator.GetData()->SetTimer(new_time);
			}

			found = true;
			break;
		}

		iterator.Advance();
	}

	if (!found) {
		//Spawn wasn't in this zone...
		//Tell the other zones to update their spawn time for this spawn point
		auto pack = new ServerPacket(ServerOP_UpdateSpawn, sizeof(UpdateSpawnTimer_Struct));

		auto ust  = (UpdateSpawnTimer_Struct*) pack->pBuffer;

		ust->id       = spawn2_id;
		ust->duration = new_time;

		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

// used to set the number of an item in the selected merchant's temp item list. Defaults to zero if no quantity is specified.
void QuestManager::MerchantSetItem(uint32 NPCid, uint32 itemid, uint32 quantity) {
	Mob* merchant = entity_list.GetMobByNpcTypeID(NPCid);

	if (merchant == 0 || !merchant->IsNPC() || (merchant->GetClass() != Class::Merchant))
		return;	// don't do anything if NPCid isn't a merchant

	const EQ::ItemData* item = nullptr;
	item = database.GetItem(itemid);
	if (!item) return;		// if the item id doesn't correspond to a real item, do nothing

	zone->SaveTempItem(merchant->CastToNPC()->MerchantType, NPCid, itemid, quantity);
}

uint32 QuestManager::MerchantCountItem(uint32 NPCid, uint32 itemid) {
	Mob* merchant = entity_list.GetMobByNpcTypeID(NPCid);

	if (merchant == 0 || !merchant->IsNPC() || (merchant->GetClass() != Class::Merchant))
		return 0;	// if it isn't a merchant, it doesn't have any items

	const EQ::ItemData* item = nullptr;
	item = database.GetItem(itemid);
	if (!item)
		return 0;	// if it isn't a valid item, the merchant doesn't have any

	// look for the item in the merchant's temporary list
	std::list<TempMerchantList> MerchList = zone->tmpmerchanttable[NPCid];
	std::list<TempMerchantList>::const_iterator itr;
	uint32 Quant = 0;

	for (itr = MerchList.begin(); itr != MerchList.end(); ++itr) {
		if (itr->item == itemid) {	// if this is the item we're looking for
			Quant = itr->charges;
			break;
		}
	}

	return Quant;	// return the quantity of itemid (0 if it was never found)
}

std::string QuestManager::varlink(EQ::ItemInstance* inst)
{
	QuestManagerCurrentQuestVars();

	if (!inst) {
		return "INVALID ITEM INSTANCE IN VARLINK";
	}

	EQ::SayLinkEngine linker;
	linker.SetLinkType(EQ::saylink::SayLinkItemInst);
	linker.SetItemInst(inst);

	return linker.GenerateLink();
}

// Item Link for use in Variables - "my $example_link = quest::varlink(item_id);"
std::string QuestManager::varlink(
	uint32 item_id,
	int16 charges,
	uint32 aug1,
	uint32 aug2,
	uint32 aug3,
	uint32 aug4,
	uint32 aug5,
	uint32 aug6,
	bool attuned
) {
	QuestManagerCurrentQuestVars();
	const auto *item = database.CreateItem(
		item_id,
		charges,
		aug1,
		aug2,
		aug3,
		aug4,
		aug5,
		aug6,
		attuned
	);
	if (!item) {
		return "INVALID ITEM ID IN VARLINK";
	}

	EQ::SayLinkEngine linker;
	linker.SetLinkType(EQ::saylink::SayLinkItemInst);
	linker.SetItemInst(item);

	auto link = linker.GenerateLink();
	safe_delete(item);
	return link;
}

std::string QuestManager::getitemcomment(uint32 item_id) {
	const auto* item_data = database.GetItem(item_id);
	if (!item_data) {
		return "INVALID ITEM ID IN GETITEMCOMMENT";
	}

	std::string item_comment = item_data->Comment;
	return item_comment;
}

std::string QuestManager::getitemlore(uint32 item_id) {
	const auto* item_data = database.GetItem(item_id);
	if (!item_data) {
		return "INVALID ITEM ID IN GETITEMLORE";
	}

	std::string item_lore = item_data->Lore;
	return item_lore;
}

std::string QuestManager::getitemname(uint32 item_id) {
	const auto* item_data = database.GetItem(item_id);
	if (!item_data) {
		return "INVALID ITEM ID IN GETITEMNAME";
	}

	std::string item_name = item_data->Name;
	return item_name;
}

std::string QuestManager::getnpcnamebyid(uint32 npc_id) {
	std::string res;
	if (npc_id > 0) {
		res = database.GetNPCNameByID(npc_id);
	}
	return res;
}

std::string QuestManager::getcleannpcnamebyid(uint32 npc_id) {
	std::string res;
	if (npc_id > 0) {
		res = database.GetCleanNPCNameByID(npc_id);
	}
	return res;
}

uint16 QuestManager::CreateInstance(const std::string& zone_short_name, int16 instance_version, uint32 duration)
{
	QuestManagerCurrentQuestVars();

	uint32 zone_id = ZoneID(zone_short_name);
	if (!zone_id) {
		return 0;
	}

	uint16 instance_id = 0;
	if (!database.GetUnusedInstanceID(instance_id)) {
		if (initiator) {
			initiator->Message(Chat::Red, "Server was unable to find a free instance id.");
		}

		return 0;
	}

	if (!database.CreateInstance(instance_id, zone_id, instance_version, duration)) {
		if (initiator) {
			initiator->Message(Chat::Red, "Server was unable to create a new instance.");
		}

		return 0;
	}

	return instance_id;
}

void QuestManager::DestroyInstance(uint16 instance_id)
{
	database.DeleteInstance(instance_id);
}

void QuestManager::UpdateInstanceTimer(uint16 instance_id, uint32 new_duration)
{
	auto e = InstanceListRepository::FindOne(database, instance_id);

	if (!e.id) {
		return;
	}

	e.duration   = new_duration;
	e.start_time = std::time(nullptr);
	e.expire_at  = e.start_time + e.duration;

	const int updated = InstanceListRepository::UpdateOne(database, e);

	if (updated) {
		auto pack = new ServerPacket(ServerOP_InstanceUpdateTime, sizeof(ServerInstanceUpdateTime_Struct));

		auto ut = (ServerInstanceUpdateTime_Struct*) pack->pBuffer;

		ut->instance_id  = instance_id;
		ut->new_duration = new_duration;

		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

uint32 QuestManager::GetInstanceTimer()
{
	if (zone && zone->GetInstanceID() && zone->GetInstanceTimer()) {
		return zone->GetInstanceTimer()->GetRemainingTime();
	}

	return 0;
}

uint32 QuestManager::GetInstanceTimerByID(uint16 instance_id)
{
	return instance_id ? InstanceListRepository::GetRemainingTimeByInstanceID(database, instance_id) : 0;
}

uint16 QuestManager::GetInstanceID(const char *zone, int16 version)
{
	QuestManagerCurrentQuestVars();

	return initiator ? database.GetInstanceID(ZoneID(zone), initiator->CharacterID(), version) : 0;
}

std::vector<uint16> QuestManager::GetInstanceIDs(std::string zone_name, uint32 character_id)
{
	if (!character_id) {
		QuestManagerCurrentQuestVars();

		if (initiator) {
			return database.GetInstanceIDs(ZoneID(zone_name), initiator->CharacterID());
		}

		return { };
	}

	return database.GetInstanceIDs(ZoneID(zone_name), character_id);
}

uint16 QuestManager::GetInstanceIDByCharID(
	const std::string &zone_short_name,
	int16 instance_version,
	uint32 character_id
)
{
	return database.GetInstanceID(ZoneID(zone_short_name), character_id, instance_version);
}

void QuestManager::AssignToInstance(uint16 instance_id)
{
	QuestManagerCurrentQuestVars();

	if (initiator) {
		database.AddClientToInstance(instance_id, initiator->CharacterID());
	}
}

void QuestManager::AssignToInstanceByCharID(uint16 instance_id, uint32 character_id)
{
	database.AddClientToInstance(instance_id, character_id);
}

void QuestManager::AssignGroupToInstance(uint16 instance_id)
{
	QuestManagerCurrentQuestVars();

	if (initiator) {
		Group* g = initiator->GetGroup();
		if (g) {
			database.AssignGroupToInstance(g->GetID(), instance_id);
		}
	}
}

void QuestManager::AssignRaidToInstance(uint16 instance_id)
{
	QuestManagerCurrentQuestVars();

	if (initiator) {
		Raid* r = initiator->GetRaid();
		if (r) {
			database.AssignRaidToInstance(r->GetID(), instance_id);
		}
	}
}

void QuestManager::RemoveFromInstance(uint16 instance_id)
{
	QuestManagerCurrentQuestVars();

	if (initiator) {
		if (database.RemoveClientFromInstance(instance_id, initiator->CharacterID())) {
			initiator->Message(Chat::Say, "Removed client from instance.");
		} else {
			initiator->Message(Chat::Say, "Failed to remove client from instance.");
		}
	}
}

void QuestManager::RemoveFromInstanceByCharID(uint16 instance_id, uint32 char_id) {
	database.RemoveClientFromInstance(instance_id, char_id);
}

bool QuestManager::CheckInstanceByCharID(uint16 instance_id, uint32 char_id) {
	return database.CheckInstanceByCharID(instance_id, char_id);
}

void QuestManager::RemoveAllFromInstance(uint16 instance_id)
{
	QuestManagerCurrentQuestVars();

	if (initiator) {
		std::list<uint32> character_ids;

		if (database.RemoveClientsFromInstance(instance_id)) {
			initiator->Message(Chat::Say, "Removed all players from instance.");
		} else {
			database.GetCharactersInInstance(instance_id, character_ids);
			initiator->Message(
				Chat::Say,
				fmt::format(
					"Failed to remove {} player{} from instance.",
					character_ids.size(),
					character_ids.size() != 1 ? "s" : ""
				).c_str()
			);
		}
	}
}

void QuestManager::MovePCInstance(int zone_id, int instance_id, const glm::vec4& position)
{
	QuestManagerCurrentQuestVars();

	if (initiator) {
		initiator->MovePC(zone_id, instance_id, position.x, position.y, position.z, position.w);
	}
}

void QuestManager::FlagInstanceByGroupLeader(uint32 zone, int16 version)
{
	QuestManagerCurrentQuestVars();

	if (initiator) {
		Group* g = initiator->GetGroup();
		if (g) {
			database.FlagInstanceByGroupLeader(zone, version, initiator->CharacterID(), g->GetID());
		}
	}
}

void QuestManager::FlagInstanceByRaidLeader(uint32 zone, int16 version)
{
	QuestManagerCurrentQuestVars();

	if (initiator) {
		Raid* r = initiator->GetRaid();
		if (r) {
			database.FlagInstanceByRaidLeader(zone, version, initiator->CharacterID(), r->GetID());
		}
	}
}

std::string QuestManager::getcharnamebyid(uint32 char_id) {
	std::string res;
	if (char_id > 0) {
		res = database.GetCharNameByID(char_id);
	}
	return res;
}

uint32 QuestManager::getcharidbyname(const char* name) {
	return database.GetCharacterID(name);
}

std::string QuestManager::getclassname(uint8 class_id, uint8 level) {
	return GetClassIDName(class_id, level);
}

uint32 QuestManager::getcurrencyid(uint32 item_id) {
	return zone->GetCurrencyID(item_id);
}

uint32 QuestManager::getcurrencyitemid(uint32 currency_id) {
	return zone->GetCurrencyItemID(currency_id);
}

const char* QuestManager::getguildnamebyid(int guild_id) {
	if (guild_id > 0)
		return guild_mgr.GetGuildName(guild_id);
	else
		return("");
}

int QuestManager::getguildidbycharid(uint32 char_id) {
    if (char_id > 0) {
        return database.GetGuildIDByCharID(char_id);
    }
    return 0;
}

int QuestManager::getgroupidbycharid(uint32 char_id) {
    if (char_id > 0) {
        return database.GetGroupIDByCharID(char_id);
    }
    return 0;
}

int QuestManager::getraididbycharid(uint32 char_id) {
    if (char_id > 0) {
        return database.GetRaidIDByCharID(char_id);
    }
    return 0;
}

void QuestManager::SetRunning(bool val)
{
	QuestManagerCurrentQuestVars();
	if(!owner)
		return;
	owner->SetRunning(val);
}

bool QuestManager::IsRunning()
{
	QuestManagerCurrentQuestVars();
	if(!owner)
		return false;
	return owner->IsRunning();
}

void QuestManager::FlyMode(GravityBehavior flymode)
{
	QuestManagerCurrentQuestVars();
	if(initiator)
	{
		initiator->SendAppearancePacket(AppearanceType::FlyMode, static_cast<int>(flymode));
		initiator->SetFlyMode(flymode);
	}
	else if(owner)
	{
		owner->SendAppearancePacket(AppearanceType::FlyMode, static_cast<int>(flymode));
		owner->SetFlyMode(flymode);
	}
}

uint8 QuestManager::FactionValue()
{
	QuestManagerCurrentQuestVars();
	FACTION_VALUE oldfac;
	uint8         newfac = 0;
	if (initiator && owner && owner->IsNPC()) {
		oldfac = initiator->GetFactionLevel(
			initiator->GetID(),
			owner->GetID(),
			initiator->GetFactionRace(),
			initiator->GetClass(),
			initiator->GetDeity(),
			owner->GetPrimaryFaction(),
			owner
		);

		// now, reorder the faction to have it make sense (higher values are better)
		switch (oldfac) {
			case FACTION_SCOWLS:
				newfac = 1;
				break;
			case FACTION_THREATENINGLY:
				newfac = 2;
				break;
			case FACTION_DUBIOUSLY:
				newfac = 3;
				break;
			case FACTION_APPREHENSIVELY:
				newfac = 4;
				break;
			case FACTION_INDIFFERENTLY:
				newfac = 5;
				break;
			case FACTION_AMIABLY:
				newfac = 6;
				break;
			case FACTION_KINDLY:
				newfac = 7;
				break;
			case FACTION_WARMLY:
				newfac = 8;
				break;
			case FACTION_ALLY:
				newfac = 9;
				break;
		}
	}

	return newfac;
}

void QuestManager::enabletitle(int titleset) {
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return;
	}

	initiator->EnableTitle(titleset);
}

bool QuestManager::checktitle(int titleset) {
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return false;
	}

	return initiator->CheckTitle(titleset);
}

void QuestManager::removetitle(int titleset) {
	QuestManagerCurrentQuestVars();

	if (!initiator) {
		return;
	}

	initiator->RemoveTitle(titleset);
}

void QuestManager::wearchange(uint8 slot, uint32 texture, uint32 hero_forge_model, uint32 elite_material)
{
	QuestManagerCurrentQuestVars();

	if (!owner) {
		return;
	}

	owner->SendTextureWC(slot, texture, hero_forge_model, elite_material);
	if (owner->IsNPC()) {
		owner->CastToNPC()->NPCSlotTexture(slot, texture);
	}
}

void QuestManager::voicetell(const char *str, int macronum, int racenum, int gendernum)
{
	QuestManagerCurrentQuestVars();

	if (!owner) {
		return;
	}

	if (str) {
		Client *c = entity_list.GetClientByName(str);

		if (c) {
			auto outapp = new EQApplicationPacket(OP_VoiceMacroOut, sizeof(VoiceMacroOut_Struct));
			auto* vmo = (VoiceMacroOut_Struct *) outapp->pBuffer;
			strn0cpy(vmo->From, owner->GetCleanName(), sizeof(vmo->From));
			vmo->Type        = 1;
			vmo->Voice       = (racenum * 2) + gendernum;
			vmo->MacroNumber = macronum;
			c->QueuePacket(outapp);
			safe_delete(outapp);
		} else {
			LogQuests("from [{}]. Client [{}] not found", owner->GetName(), str);
		}
	}
}

void QuestManager::SendMail(const char *to, const char *from, const char *subject, const char *message) {
	if (!to || !from || !subject || !message) {
		return;
	}

	uint32 message_len = strlen(message) + 1;
	auto pack = new ServerPacket(ServerOP_UCSMailMessage, sizeof(ServerMailMessageHeader_Struct) + message_len);
	ServerMailMessageHeader_Struct* mail = (ServerMailMessageHeader_Struct*) pack->pBuffer;

	strn0cpy(mail->to, to, 64);
	strn0cpy(mail->from, from, 64);
	strn0cpy(mail->subject, subject, 128);
	strcpy(mail->message, message);

	worldserver.SendPacket(pack);
	safe_delete(pack);
}

uint16 QuestManager::CreateDoor(const char* model, float x, float y, float z, float heading, uint8 opentype, uint16 size)
{
	uint16 entid = 0; //safety check
	entid = entity_list.CreateDoor(model, glm::vec4(x, y, z, heading), opentype, size);
	return entid;
}

int32 QuestManager::GetZoneID(const char *zone) {
	return static_cast<int32>(ZoneID(zone));
}

std::string QuestManager::GetZoneLongName(std::string zone_short_name)
{
	return ZoneLongName(ZoneID(zone_short_name), true);
}

std::string QuestManager::GetZoneLongNameByID(uint32 zone_id) {
	return ZoneLongName(zone_id, true);
}

std::string QuestManager::GetZoneShortName(uint32 zone_id) {
	return ZoneName(zone_id, true);
}

bool QuestManager::EnableRecipe(uint32 recipe_id)
{
	bool success = false;
	if (recipe_id > 0)
		success = content_db.EnableRecipe(recipe_id);
	return (success);
}

bool QuestManager::DisableRecipe(uint32 recipe_id)
{
	bool success = false;
	if (recipe_id > 0)
		success = content_db.DisableRecipe(recipe_id);
	return (success);
}

void QuestManager::ClearNPCTypeCache(int npctype_id) {
	if (zone) {
		zone->ClearNPCTypeCache(npctype_id);
	}
}

void QuestManager::ReloadZoneStaticData()
{
	if (zone) {
		zone->ReloadStaticData();
	}
}

Client* QuestManager::GetInitiator() const
{
	if (!m_running_quests.empty()) {
		RunningQuest e = m_running_quests.top();
		return e.initiator;
	}

	return nullptr;
}

NPC* QuestManager::GetNPC() const
{
	if (!m_running_quests.empty()) {
		RunningQuest e = m_running_quests.top();
		return (e.owner && e.owner->IsNPC()) ? e.owner->CastToNPC() : nullptr;
	}

	return nullptr;
}

Bot* QuestManager::GetBot() const
{
	if (!m_running_quests.empty()) {
		RunningQuest e = m_running_quests.top();
		return (e.owner && e.owner->IsBot()) ? e.owner->CastToBot() : nullptr;
	}

	return nullptr;
}

Merc* QuestManager::GetMerc() const
{
	if (!m_running_quests.empty()) {
		RunningQuest e = m_running_quests.top();
		return (e.owner && e.owner->IsMerc()) ? e.owner->CastToMerc() : nullptr;
	}

	return nullptr;
}

Mob* QuestManager::GetOwner() const
{
	if (!m_running_quests.empty()) {
		RunningQuest e = m_running_quests.top();
		return e.owner;
	}

	return nullptr;
}

EQ::InventoryProfile* QuestManager::GetInventory() const
{
	if (!m_running_quests.empty()) {
		RunningQuest e = m_running_quests.top();
		return &e.initiator->GetInv();
	}

	return nullptr;
}

EQ::ItemInstance* QuestManager::GetQuestItem() const
{
	if (!m_running_quests.empty()) {
		RunningQuest e = m_running_quests.top();
		return e.questitem;
	}

	return nullptr;
}

const SPDat_Spell_Struct* QuestManager::GetQuestSpell()
{
	if (!m_running_quests.empty()) {
		RunningQuest e = m_running_quests.top();
		return e.questspell;
	}

	return nullptr;
}

std::string QuestManager::GetEncounter() const
{
	if (!m_running_quests.empty()) {
		RunningQuest e = m_running_quests.top();
		return e.encounter;
	}

	return "";
}

void QuestManager::UpdateZoneHeader(std::string type, std::string value) {
	if (!strcasecmp(type.c_str(), "ztype"))
		zone->newzone_data.ztype = Strings::ToInt(value);
	else if (!strcasecmp(type.c_str(), "fog_red")) {
		for (int i = 0; i < 4; i++) {
			zone->newzone_data.fog_red[i] = Strings::ToInt(value);
		}
	} else if (!strcasecmp(type.c_str(), "fog_green")) {
		for (int i = 0; i < 4; i++) {
			zone->newzone_data.fog_green[i] = Strings::ToInt(value);
		}
	} else if (!strcasecmp(type.c_str(), "fog_blue")) {
		for (int i = 0; i < 4; i++) {
			zone->newzone_data.fog_blue[i] = Strings::ToInt(value);
		}
	} else if (!strcasecmp(type.c_str(), "fog_minclip")) {
		for (int i = 0; i < 4; i++) {
			zone->newzone_data.fog_minclip[i] = Strings::ToFloat(value);
		}
	} else if (!strcasecmp(type.c_str(), "fog_maxclip")) {
		for (int i = 0; i < 4; i++) {
			zone->newzone_data.fog_maxclip[i] = Strings::ToFloat(value);
		}
	} else if (!strcasecmp(type.c_str(), "gravity")) {
		zone->newzone_data.gravity = Strings::ToFloat(value);
	} else if (!strcasecmp(type.c_str(), "time_type")) {
		zone->newzone_data.time_type = Strings::ToInt(value);
	} else if (!strcasecmp(type.c_str(), "rain_chance")) {
		for (int i = 0; i < 4; i++) {
			zone->newzone_data.rain_chance[i] = Strings::ToInt(value);
		}
	} else if (!strcasecmp(type.c_str(), "rain_duration")) {
		for (int i = 0; i < 4; i++) {
			zone->newzone_data.rain_duration[i] = Strings::ToInt(value);
		}
	} else if (!strcasecmp(type.c_str(), "snow_chance")) {
		for (int i = 0; i < 4; i++) {
			zone->newzone_data.snow_chance[i] = Strings::ToInt(value);
		}
	} else if (!strcasecmp(type.c_str(), "snow_duration")) {
		for (int i = 0; i < 4; i++) {
			zone->newzone_data.snow_duration[i] = Strings::ToInt(value);
		}
	} else if (!strcasecmp(type.c_str(), "sky")) {
		zone->newzone_data.sky = Strings::ToInt(value);
	} else if (!strcasecmp(type.c_str(), "safe_x")) {
		zone->newzone_data.safe_x = Strings::ToFloat(value);
	} else if (!strcasecmp(type.c_str(), "safe_y")) {
		zone->newzone_data.safe_y = Strings::ToFloat(value);
	} else if (!strcasecmp(type.c_str(), "safe_z")) {
		zone->newzone_data.safe_z = Strings::ToFloat(value);
	} else if (!strcasecmp(type.c_str(), "safe_heading")) {
		zone->newzone_data.safe_heading = Strings::ToFloat(value);
	} else if (!strcasecmp(type.c_str(), "max_z")) {
		zone->newzone_data.max_z = Strings::ToFloat(value);
	} else if (!strcasecmp(type.c_str(), "underworld")) {
		zone->newzone_data.underworld = Strings::ToFloat(value);
	} else if (!strcasecmp(type.c_str(), "minclip")) {
		zone->newzone_data.minclip = Strings::ToFloat(value);
	} else if (!strcasecmp(type.c_str(), "maxclip")) {
		zone->newzone_data.maxclip = Strings::ToFloat(value);
	} else if (!strcasecmp(type.c_str(), "fog_density")) {
		zone->newzone_data.fog_density = Strings::ToFloat(value);
	} else if (!strcasecmp(type.c_str(), "suspendbuffs")) {
		zone->newzone_data.suspend_buffs = Strings::ToInt(value);
	} else if (!strcasecmp(type.c_str(), "lavadamage")) {
		zone->newzone_data.lava_damage = Strings::ToInt(value);
	} else if (!strcasecmp(type.c_str(), "minlavadamage")) {
		zone->newzone_data.min_lava_damage = Strings::ToInt(value);
	}

	auto outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
	memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
	entity_list.QueueClients(0, outapp);
	safe_delete(outapp);
}

EQ::ItemInstance *QuestManager::CreateItem(uint32 item_id, int16 charges, uint32 augment_one, uint32 augment_two, uint32 augment_three, uint32 augment_four, uint32 augment_five, uint32 augment_six, bool attuned) const {
	if (database.GetItem(item_id)) {
		return database.CreateItem(
			item_id,
			charges,
			augment_one,
			augment_two,
			augment_three,
			augment_four,
			augment_five,
			augment_six,
			attuned
		);
	}
	return nullptr;
}

std::string QuestManager::gethexcolorcode(std::string color_name) {

	for (auto color : html_colors) {
		if (!strcasecmp(color.first.c_str(), color_name.c_str())) {
			return color.second;
		}
	}

	return std::string();
}

float QuestManager::GetAAEXPModifierByCharID(uint32 character_id, uint32 zone_id, int16 instance_version) const {
	return database.GetAAEXPModifierByCharID(character_id, zone_id, instance_version);
}

float QuestManager::GetEXPModifierByCharID(uint32 character_id, uint32 zone_id, int16 instance_version) const {
	return database.GetEXPModifierByCharID(character_id, zone_id, instance_version);
}

void QuestManager::SetAAEXPModifierByCharID(uint32 character_id, uint32 zone_id, float aa_modifier, int16 instance_version) {
	database.SetAAEXPModifierByCharID(character_id, zone_id, aa_modifier, instance_version);
}

void QuestManager::SetEXPModifierByCharID(uint32 character_id, uint32 zone_id, float exp_modifier, int16 instance_version) {
	database.SetEXPModifierByCharID(character_id, zone_id, exp_modifier, instance_version);
}

std::string QuestManager::getgendername(uint32 gender_id) {
	std::string gender_name = GetGenderName(gender_id);
	return gender_name;
}

std::string QuestManager::getdeityname(uint32 deity_id) {
	return Deity::GetName(deity_id);
}

std::string QuestManager::getinventoryslotname(int16 slot_id) {
	return EQ::invslot::GetInvPossessionsSlotName(slot_id);
}

const int QuestManager::getitemstat(uint32 item_id, std::string stat_identifier) {
	QuestManagerCurrentQuestVars();
	return EQ::InventoryProfile::GetItemStatValue(item_id, stat_identifier);
}

int QuestManager::getspellstat(uint32 spell_id, std::string stat_identifier, uint8 slot) {
	QuestManagerCurrentQuestVars();
	return GetSpellStatValue(spell_id, stat_identifier.c_str(), slot);
}

void QuestManager::CrossZoneDialogueWindow(uint8 update_type, int update_identifier, const char* message, const char* client_name) {
	auto pack = new ServerPacket(ServerOP_CZDialogueWindow, sizeof(CZDialogueWindow_Struct));
	CZDialogueWindow_Struct* CZDW = (CZDialogueWindow_Struct*)pack->pBuffer;
	CZDW->update_type = update_type;
	CZDW->update_identifier = update_identifier;
	strn0cpy(CZDW->message, message, 4096);
	strn0cpy(CZDW->client_name, client_name, 64);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneLDoNUpdate(uint8 update_type, uint8 update_subtype, int update_identifier, uint32 theme_id, int points, const char* client_name) {
	auto pack = new ServerPacket(ServerOP_CZLDoNUpdate, sizeof(CZLDoNUpdate_Struct));
	CZLDoNUpdate_Struct* CZLU = (CZLDoNUpdate_Struct*)pack->pBuffer;
	CZLU->update_type = update_type;
	CZLU->update_subtype = update_subtype;
	CZLU->update_identifier = update_identifier;
	CZLU->theme_id = theme_id;
	CZLU->points = points;
	strn0cpy(CZLU->client_name, client_name, 64);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneMarquee(uint8 update_type, int update_identifier, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message, const char* client_name) {
	auto pack = new ServerPacket(ServerOP_CZMarquee, sizeof(CZMarquee_Struct));
	CZMarquee_Struct* CZM = (CZMarquee_Struct*)pack->pBuffer;
	CZM->update_type = update_type;
	CZM->update_identifier = update_identifier;
	CZM->type = type;
	CZM->priority = priority;
	CZM->fade_in = fade_in;
	CZM->fade_out = fade_out;
	CZM->duration = duration;
	strn0cpy(CZM->message, message, 512);
	strn0cpy(CZM->client_name, client_name, 64);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneMessage(uint8 update_type, int update_identifier, uint32 type, const char* message, const char* client_name) {
	auto pack = new ServerPacket(ServerOP_CZMessage, sizeof(CZMarquee_Struct));
	CZMessage_Struct* CZM = (CZMessage_Struct*)pack->pBuffer;
	CZM->update_type = update_type;
	CZM->update_identifier = update_identifier;
	CZM->type = type;
	strn0cpy(CZM->message, message, 512);
	strn0cpy(CZM->client_name, client_name, 64);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneMove(const CZMove_Struct& m)
{
	auto pack = new ServerPacket(ServerOP_CZMove, sizeof(CZMove_Struct));
	auto s = (CZMove_Struct*) pack->pBuffer;

	if (!m.client_name.empty()) {
		s->client_name = m.client_name;
	}

	s->coordinates       = m.coordinates;
	s->instance_id       = m.instance_id;
	s->update_type       = m.update_type;
	s->update_subtype    = m.update_subtype;
	s->update_identifier = m.update_identifier;

	if (!m.zone_short_name.empty()) {
		s->zone_short_name = m.zone_short_name;
	}

	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneSetEntityVariable(uint8 update_type, int update_identifier, const char* variable_name, const char* variable_value, const char* client_name) {
	auto pack = new ServerPacket(ServerOP_CZSetEntityVariable, sizeof(CZSetEntityVariable_Struct));
	CZSetEntityVariable_Struct* CZM = (CZSetEntityVariable_Struct*)pack->pBuffer;
	CZM->update_type = update_type;
	CZM->update_identifier = update_identifier;
	strn0cpy(CZM->variable_name, variable_name, 256);
	strn0cpy(CZM->variable_value, variable_value, 256);
	strn0cpy(CZM->client_name, client_name, 64);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneSignal(uint8 update_type, int update_identifier, int signal_id, const char* client_name) {
	auto pack = new ServerPacket(ServerOP_CZSignal, sizeof(CZSignal_Struct));
	CZSignal_Struct* CZS = (CZSignal_Struct*)pack->pBuffer;
	CZS->update_type = update_type;
	CZS->update_identifier = update_identifier;
	CZS->signal_id = signal_id;
	strn0cpy(CZS->client_name, client_name, 64);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneSpell(uint8 update_type, uint8 update_subtype, int update_identifier, uint32 spell_id, const char* client_name) {
	auto pack = new ServerPacket(ServerOP_CZSpell, sizeof(CZSpell_Struct));
	CZSpell_Struct* CZS = (CZSpell_Struct*)pack->pBuffer;
	CZS->update_type = update_type;
	CZS->update_subtype = update_subtype;
	CZS->update_identifier = update_identifier;
	CZS->spell_id = spell_id;
	strn0cpy(CZS->client_name, client_name, 64);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneTaskUpdate(uint8 update_type, uint8 update_subtype, int update_identifier, uint32 task_identifier, int task_subidentifier, int update_count, bool enforce_level_requirement, const char* client_name) {
	auto pack = new ServerPacket(ServerOP_CZTaskUpdate, sizeof(CZTaskUpdate_Struct));
	CZTaskUpdate_Struct* CZTU = (CZTaskUpdate_Struct*)pack->pBuffer;
	CZTU->update_type = update_type;
	CZTU->update_subtype = update_subtype;
	CZTU->update_identifier = update_identifier;
	CZTU->task_identifier = task_identifier;
	CZTU->task_subidentifier = task_subidentifier;
	CZTU->update_count = update_count;
	CZTU->enforce_level_requirement = enforce_level_requirement;
	strn0cpy(CZTU->client_name, client_name, 64);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideDialogueWindow(const char* message, uint8 min_status, uint8 max_status) {
	auto pack = new ServerPacket(ServerOP_WWDialogueWindow, sizeof(WWDialogueWindow_Struct));
	WWDialogueWindow_Struct* WWDW = (WWDialogueWindow_Struct*)pack->pBuffer;
	strn0cpy(WWDW->message, message, 4096);
	WWDW->min_status = min_status;
	WWDW->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideLDoNUpdate(uint8 update_type, uint32 theme_id, int points, uint8 min_status, uint8 max_status) {
	auto pack = new ServerPacket(ServerOP_WWLDoNUpdate, sizeof(WWLDoNUpdate_Struct));
	WWLDoNUpdate_Struct* WWLU = (WWLDoNUpdate_Struct*)pack->pBuffer;
	WWLU->update_type = update_type;
	WWLU->theme_id = theme_id;
	WWLU->points = points;
	WWLU->min_status = min_status;
	WWLU->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideMarquee(uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message, uint8 min_status, uint8 max_status) {
	auto pack = new ServerPacket(ServerOP_WWMarquee, sizeof(WWMarquee_Struct));
	WWMarquee_Struct* WWM = (WWMarquee_Struct*)pack->pBuffer;
	WWM->type = type;
	WWM->priority = priority;
	WWM->fade_in = fade_in;
	WWM->fade_out = fade_out;
	WWM->duration = duration;
	strn0cpy(WWM->message, message, 512);
	WWM->min_status = min_status;
	WWM->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideMessage(uint32 type, const char* message, uint8 min_status, uint8 max_status) {
	auto pack = new ServerPacket(ServerOP_WWMessage, sizeof(WWMarquee_Struct));
	WWMessage_Struct* WWM = (WWMessage_Struct*)pack->pBuffer;
	WWM->type = type;
	strn0cpy(WWM->message, message, 512);
	WWM->min_status = min_status;
	WWM->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideMove(uint8 update_type, const char* zone_short_name, uint16 instance_id, uint8 min_status, uint8 max_status) {
	auto pack = new ServerPacket(ServerOP_WWMove, sizeof(WWMove_Struct));
	WWMove_Struct* WWM = (WWMove_Struct*)pack->pBuffer;
	WWM->update_type = update_type;
	strn0cpy(WWM->zone_short_name, zone_short_name, 32);
	WWM->instance_id = instance_id;
	WWM->min_status = min_status;
	WWM->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideSetEntityVariable(uint8 update_type, const char* variable_name, const char* variable_value, uint8 min_status, uint8 max_status) {
	auto pack = new ServerPacket(ServerOP_WWSetEntityVariable, sizeof(WWSetEntityVariable_Struct));
	WWSetEntityVariable_Struct* WWSEV = (WWSetEntityVariable_Struct*)pack->pBuffer;
	WWSEV->update_type = update_type;
	strn0cpy(WWSEV->variable_name, variable_name, 256);
	strn0cpy(WWSEV->variable_value, variable_value, 256);
	WWSEV->min_status = min_status;
	WWSEV->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideSignal(uint8 update_type, int signal_id, uint8 min_status, uint8 max_status) {
	auto pack = new ServerPacket(ServerOP_WWSignal, sizeof(WWSignal_Struct));
	WWSignal_Struct* WWS = (WWSignal_Struct*)pack->pBuffer;
	WWS->update_type = update_type;
	WWS->signal_id = signal_id;
	WWS->min_status = min_status;
	WWS->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideSpell(uint8 update_type, uint32 spell_id, uint8 min_status, uint8 max_status) {
	auto pack = new ServerPacket(ServerOP_WWSpell, sizeof(WWSpell_Struct));
	WWSpell_Struct* WWS = (WWSpell_Struct*)pack->pBuffer;
	WWS->update_type = update_type;
	WWS->spell_id = spell_id;
	WWS->min_status = min_status;
	WWS->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideTaskUpdate(uint8 update_type, uint32 task_identifier, int task_subidentifier, int update_count, bool enforce_level_requirement, uint8 min_status, uint8 max_status) {
	auto pack = new ServerPacket(ServerOP_WWTaskUpdate, sizeof(WWTaskUpdate_Struct));
	WWTaskUpdate_Struct* WWTU = (WWTaskUpdate_Struct*)pack->pBuffer;
	WWTU->update_type = update_type;
	WWTU->task_identifier = task_identifier;
	WWTU->task_subidentifier = task_subidentifier;
	WWTU->update_count = update_count;
	WWTU->enforce_level_requirement = enforce_level_requirement;
	WWTU->min_status = min_status;
	WWTU->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

const SPDat_Spell_Struct* QuestManager::getspell(uint32 spell_id) {
    if (spells[spell_id].id) {
        return &spells[spell_id];
    }
    return nullptr;
}

std::string QuestManager::getenvironmentaldamagename(uint8 damage_type) {
	std::string environmental_damage_name = EQ::constants::GetEnvironmentalDamageName(damage_type);
	return environmental_damage_name;
}

void QuestManager::TrackNPC(uint32 entity_id) {
	QuestManagerCurrentQuestVars();
	if (!initiator) {
		return;
	}

	initiator->SetTrackingID(entity_id);
}

int QuestManager::GetRecipeMadeCount(uint32 recipe_id) {
	QuestManagerCurrentQuestVars();
	if (!initiator) {
		return 0;
	}

	return initiator->GetRecipeMadeCount(recipe_id);
}

std::string QuestManager::GetRecipeName(uint32 recipe_id) {
	auto r = TradeskillRecipeRepository::GetWhere(
		content_db,
		fmt::format("id = {}", recipe_id)
	);

	if (!r.empty() && r[0].id) {
		return r[0].name;
	}

	return std::string();
}

bool QuestManager::HasRecipeLearned(uint32 recipe_id) {
	QuestManagerCurrentQuestVars();
	if (!initiator) {
		return false;
	}

	return initiator->HasRecipeLearned(recipe_id);
}

void QuestManager::LearnRecipe(uint32 recipe_id) {
	QuestManagerCurrentQuestVars();
	if (!initiator) {
		return;
	}

	initiator->LearnRecipe(recipe_id);
}

void QuestManager::marquee(uint32 type, std::string message, uint32 duration)
{
	QuestManagerCurrentQuestVars();
	if (!initiator) {
		return;
	}

	initiator->SendMarqueeMessage(type, message, duration);
}

void QuestManager::marquee(uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, std::string message)
{
	QuestManagerCurrentQuestVars();
	if (!initiator) {
		return;
	}

	initiator->SendMarqueeMessage(type, priority, fade_in, fade_out, duration, message);
}

bool QuestManager::DoAugmentSlotsMatch(uint32 item_one, uint32 item_two)
{
	const auto* inst_one = database.GetItem(item_one);
	if (!inst_one) {
		return false;
	}

	const auto* inst_two = database.GetItem(item_two);
	if (!inst_two) {
		return false;
	}

	for (auto i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++) {
		if (inst_one->AugSlotType[i] != inst_two->AugSlotType[i]) {
			return false;
		}
	}

	return true;
}

int8 QuestManager::DoesAugmentFit(EQ::ItemInstance* inst, uint32 augment_id, uint8 augment_slot)
{
	if (!inst) {
		return INVALID_INDEX;
	}

	const auto* aug_inst = database.GetItem(augment_id);
	if (!aug_inst) {
		return INVALID_INDEX;
	}

	if (augment_slot != 255) {
		return !inst->IsAugmentSlotAvailable(aug_inst->AugType, augment_slot) ? INVALID_INDEX : augment_slot;
	}

	return inst->AvailableAugmentSlot(aug_inst->AugType);
}

void QuestManager::SendPlayerHandinEvent() {
	return;
}

std::string QuestManager::GetAutoLoginCharacterNameByAccountID(uint32 account_id)
{
	return AccountRepository::GetAutoLoginCharacterNameByAccountID(database, account_id);
}

bool QuestManager::SetAutoLoginCharacterNameByAccountID(uint32 account_id, const std::string& character_name)
{
	return AccountRepository::SetAutoLoginCharacterNameByAccountID(database, account_id, character_name);
}

void QuestManager::SpawnCircle(uint32 npc_id, glm::vec4 position, float radius, uint32 points)
{
	const NPCType* t = content_db.LoadNPCTypesData(npc_id);
	if (!t) {
		return;
	}

	glm::vec4 npc_position = position;

	for (uint32 i = 0; i < points; i++) {
		float angle = 2 * std::numbers::pi * i / points;

		npc_position.x = position.x + radius * std::cos(angle);
		npc_position.y = position.y + radius * std::sin(angle);

		NPC* n = new NPC(t, nullptr, npc_position, GravityBehavior::Water);

		n->FixZ();

		n->AddLootTable();

		if (n->DropsGlobalLoot()) {
			n->CheckGlobalLootTables();
		}

		entity_list.AddNPC(n, true, true);
	}
}

void QuestManager::SpawnGrid(uint32 npc_id, glm::vec4 position, float spacing, uint32 spawn_count)
{
	const NPCType* t = content_db.LoadNPCTypesData(npc_id);
	if (!t) {
		return;
	}

	glm::vec4 npc_position = position;

	uint32 columns = std::ceil(std::sqrt(spawn_count));
	uint32 rows    = std::ceil(spawn_count / columns);

	float total_width  = ((columns - 1) * spacing);
	float total_height = ((rows - 1) * spacing);

	float start_x = position.x - total_width / 2;
	float start_y = position.y - total_height / 2;

	uint32 spawned = 0;

	for (uint32 row = 0; row < rows; row++) {
		for (uint32 column = 0; column < columns; column++) {
			if (spawned >= spawn_count) {
				break;
			}

			npc_position.x = start_x + column * spacing;
			npc_position.y = start_y + row * spacing;

			NPC* n = new NPC(t, nullptr, npc_position, GravityBehavior::Water);

			n->FixZ();

			n->AddLootTable();

			if (n->DropsGlobalLoot()) {
				n->CheckGlobalLootTables();
			}

			entity_list.AddNPC(n, true, true);

			spawned++;
		}
	}
}

bool QuestManager::handin(std::map<std::string, uint32> required) {
	QuestManagerCurrentQuestVars();
	if (!owner || !initiator) {
		LogQuests("QuestManager::handin called with nullptr owner. Probably syntax error in quest file");
		return false;
	}

	if (owner && !owner->IsNPC()) {
		LogQuests("QuestManager::handin called with non-NPC owner. Probably syntax error in quest file");
		return false;
	}

	return owner->CastToNPC()->CheckHandin(initiator, {}, required, {});
}

std::vector<std::string> QuestManager::GetPausedTimers(Mob* m)
{
	std::vector<std::string> v;

	if (m && !PTimerList.empty()) {
		for (auto e = PTimerList.begin(); e != PTimerList.end(); e++) {
			if (e->owner == m) {
				v.emplace_back(e->name);
			}
		}
	}

	return v;
}

std::vector<std::string> QuestManager::GetTimers(Mob* m)
{
	std::vector<std::string> v;

	if (m && !QTimerList.empty()) {
		for (auto e = QTimerList.begin(); e != QTimerList.end(); e++) {
			if (e->mob == m) {
				v.emplace_back(e->name);
			}
		}
	}

	return v;
}
