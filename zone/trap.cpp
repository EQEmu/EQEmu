/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2004 EQEMu Development Team (http://eqemulator.net)

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

#include "trap.h"

#include "common/repositories/criteria/content_filter_criteria.h"
#include "common/repositories/traps_repository.h"
#include "common/spdat.h"
#include "common/strings.h"
#include "zone/client.h"
#include "zone/entity.h"
#include "zone/mob.h"

/*

Schema:
CREATE TABLE traps (
	id int(11) NOT NULL auto_increment,
	zone varchar(16) NOT NULL default '',
	x int(11) NOT NULL default '0',
	y int(11) NOT NULL default '0',
	z int(11) NOT NULL default '0',
	chance tinyint NOT NULL default '0',
	maxzdiff float NOT NULL default '0',
	radius float NOT NULL default '0',
	effect int(11) NOT NULL default '0',
	effectvalue int(11) NOT NULL default '0',
	effectvalue2 int(11) NOT NULL default '0',
	message varcahr(200) NOT NULL;
	skill int(11) NOT NULL default '0',
	spawnchance int(11) NOT NULL default '0',
	PRIMARY KEY (id)
) TYPE=MyISAM;


*/

Trap::Trap() :
	Entity(),
	respawn_timer(600000),
	chkarea_timer(1000),
	reset_timer(5000),
	m_Position(glm::vec3())
{
	trap_id = 0;
	db_id = 0;
	maxzdiff = 0;
	radius = 0;
	effect = 0;
	effectvalue = 0;
	effectvalue2 = 0;
	skill = 0;
	level = 0;
	respawn_timer.Disable();
	reset_timer.Disable();
	detected = false;
	disarmed = false;
	respawn_time = 0;
	respawn_var = 0;
	SetHiddenTrigger(nullptr);
	chance = 0;
	triggered_number = 0;
	times_triggered = 0;
	group = 0;
	despawn_when_triggered = false;
	charid = 0;
	undetectable = false;
}

Trap::~Trap()
{
	//don't need to clean up mob as traps are always cleaned up same time as NPCs
	//cleaning up mob here can actually cause a crash via race condition
}

bool Trap::Process()
{
	if (chkarea_timer.Enabled() && chkarea_timer.Check() && !reset_timer.Enabled())
	{
		Mob* m = entity_list.GetTrapTrigger(this);
		const bool is_gm_client = m && m->IsClient() && m->CastToClient()->GetGM();
		if (m && !is_gm_client) {
			Trigger(m);
		}

		if (is_gm_client) {
			m->Message(Chat::White, "Your GM flag prevents you from triggering a trap.");
		}
	}
	else if (reset_timer.Enabled() && reset_timer.Check())
	{
		Log(Logs::General, Logs::Traps, "Reset timer disabled in Reset Check Process for trap %d.", trap_id);
		reset_timer.Disable();
		charid = 0;
	}

	if (respawn_timer.Enabled() && respawn_timer.Check())
	{
		detected = false;
		disarmed = false;
		chkarea_timer.Enable();
		respawn_timer.Disable();
	}


	return true;
}

void Trap::Trigger(Mob* trigger)
{
	Log(Logs::General, Logs::Traps, "Trap %d triggered by %s for the %d time!", trap_id, trigger->GetName(), times_triggered + 1);

	int i = 0;
	const NPCType* tmp = 0;
	switch (effect)
	{
		case trapTypeDebuff:
			if(message.empty())
			{
				entity_list.MessageClose(trigger,false,100,13,"%s triggers a trap!",trigger->GetName());
			}
			else
			{
				entity_list.MessageClose(trigger,false,100,13,"%s",message.c_str());
			}
			if(hiddenTrigger){
				hiddenTrigger->SpellFinished(effectvalue, trigger, EQ::spells::CastingSlot::Item, 0, -1, spells[effectvalue].resist_difficulty);
			}
			break;
		case trapTypeAlarm:
			if (message.empty())
			{
				entity_list.MessageClose(trigger,false,effectvalue,13,"A loud alarm rings out through the air...");
			}
			else
			{
				entity_list.MessageClose(trigger,false,effectvalue,13,"%s",message.c_str());
			}

			entity_list.SendAlarm(this,trigger, effectvalue2);
			break;
		case trapTypeMysticSpawn:
			if (message.empty())
			{
				entity_list.MessageClose(trigger,false,100,13,"The air shimmers...");
			}
			else
			{
				entity_list.MessageClose(trigger,false,100,13,"%s",message.c_str());
			}

			for (i = 0; i < effectvalue2; i++)
			{
				if ((tmp = content_db.LoadNPCTypesData(effectvalue)))
				{
					auto randomOffset = glm::vec4(zone->random.Int(-5, 5),zone->random.Int(-5, 5),zone->random.Int(-5, 5), zone->random.Int(0, 249));
					auto spawnPosition = randomOffset + glm::vec4(m_Position, 0.0f);
					auto new_npc = new NPC(tmp, nullptr, spawnPosition, GravityBehavior::Flying);
					new_npc->AddLootTable();
					if (new_npc->DropsGlobalLoot())
						new_npc->CheckGlobalLootTables();
					entity_list.AddNPC(new_npc);
					new_npc->AddToHateList(trigger,1);
				}
			}
			break;
		case trapTypeBanditSpawn:
			if (message.empty())
			{
				entity_list.MessageClose(trigger,false,100,13,"A bandit leaps out from behind a tree!");
			}
			else
			{
				entity_list.MessageClose(trigger,false,100,13,"%s",message.c_str());
			}

			for (i = 0; i < effectvalue2; i++)
			{
				if ((tmp = content_db.LoadNPCTypesData(effectvalue)))
				{
					auto randomOffset = glm::vec4(zone->random.Int(-2, 2), zone->random.Int(-2, 2), zone->random.Int(-2, 2), zone->random.Int(0, 249));
					auto spawnPosition = randomOffset + glm::vec4(m_Position, 0.0f);
					auto new_npc = new NPC(tmp, nullptr, spawnPosition, GravityBehavior::Flying);
					new_npc->AddLootTable();
					if (new_npc->DropsGlobalLoot())
						new_npc->CheckGlobalLootTables();
					entity_list.AddNPC(new_npc);
					new_npc->AddToHateList(trigger,1);
				}
			}
			break;
		case trapTypeDamage:
			if (message.empty())
			{
				entity_list.MessageClose(trigger,false,100,13,"%s triggers a trap!",trigger->GetName());
			}
			else
			{
				entity_list.MessageClose(trigger,false,100,13,"%s",message.c_str());
			}
			if (trigger && trigger->IsClient()) {
				auto outapp = new EQApplicationPacket(OP_Damage, sizeof(CombatDamage_Struct));
				CombatDamage_Struct* a = (CombatDamage_Struct*)outapp->pBuffer;
				int64 dmg = zone->random.Int(effectvalue, effectvalue2);
				trigger->SetHP(trigger->GetHP() - dmg);
				a->damage = dmg;
				a->hit_heading = 0.0f;
				a->source = GetHiddenTrigger()!=nullptr ? GetHiddenTrigger()->GetID() : trigger->GetID();
				a->spellid = 0;
				a->target = trigger->GetID();
				a->type = 253;
				trigger->CastToClient()->QueuePacket(outapp);
				safe_delete(outapp);
			}
	}

	if (trigger && trigger->IsClient()) {
		trigger->CastToClient()->trapid = trap_id;
		charid = trigger->CastToClient()->CharacterID();
	}

	bool update = false;
	if (despawn_when_triggered)
	{
		Log(Logs::General, Logs::Traps, "Trap %d is despawning after being triggered.", trap_id);
		update = true;
	}
	else
	{
		reset_timer.Start(5000);
	}

	if (triggered_number > 0)
		++times_triggered;

	if (triggered_number > 0 && triggered_number <= times_triggered)
	{
		Log(Logs::General, Logs::Traps, "Triggered number for trap %d reached. %d/%d", trap_id, times_triggered, triggered_number);
		update = true;
	}

	if (update)
	{
		UpdateTrap();
	}
}

Trap* EntityList::FindNearbyTrap(Mob* searcher, float max_dist, float &trap_curdist, bool detected)
{
	float dist = 999999;
	Trap* current_trap = nullptr;

	float max_dist2 = max_dist*max_dist;
	Trap *cur;

	for (auto it = trap_list.begin(); it != trap_list.end(); ++it) {
		cur = it->second;
		if(cur->disarmed || (detected && !cur->detected) || cur->undetectable)
			continue;

		auto diff = glm::vec3(searcher->GetPosition()) - cur->m_Position;
		float curdist = diff.x*diff.x + diff.y*diff.y;
		diff.z = std::abs(diff.z);

		if (curdist < max_dist2 && curdist < dist && diff.z <= cur->maxzdiff)
		{
			Log(Logs::General, Logs::Traps, "Trap %d is curdist %0.1f", cur->db_id, curdist);
			dist = curdist;
			current_trap = cur;
		}
	}

	if (current_trap != nullptr)
	{
		Log(Logs::General, Logs::Traps, "Trap %d is the closest trap.", current_trap->db_id);
		trap_curdist = dist;
	}
	else
		 trap_curdist = INVALID_INDEX;

	return current_trap;
}

Mob* EntityList::GetTrapTrigger(Trap* trap)
{

	float maxdist = trap->radius * trap->radius;
	for (auto it = client_list.begin(); it != client_list.end(); ++it)
	{
		Client* cur = it->second;

		auto diff = glm::vec3(cur->GetPosition()) - trap->m_Position;
		diff.z = std::abs(diff.z);

		if ((diff.x*diff.x + diff.y*diff.y) <= maxdist
			&& diff.z <= trap->maxzdiff)
		{
			//This prevents the trap from triggering on players while zoning.
			if (strcmp(cur->GetName(), "No name") == 0)
				continue;

			if (cur->trapid == 0 && !cur->GetGM() && (trap->chance == 0 || zone->random.Roll(trap->chance)))
			{
				Log(Logs::General, Logs::Traps, "%s is about to trigger trap %d of chance %d. diff: %0.2f maxdist: %0.2f zdiff: %0.2f maxzdiff: %0.2f", cur->GetName(), trap->trap_id, trap->chance, (diff.x*diff.x + diff.y*diff.y), maxdist, diff.z, trap->maxzdiff);
				return cur;
			}

			if (cur->GetGM()) {
				cur->Message(Chat::White, "Your GM flag prevents you from triggering a trap.");
			}
		}
		else
		{
			if (cur->trapid == trap->trap_id)
			{
				Log(Logs::General, Logs::Traps, "%s is clearing trapid for trap %d", cur->GetName(), trap->trap_id);
				cur->trapid = 0;
			}
		}
	}

	return nullptr;
}

bool EntityList::IsTrapGroupSpawned(uint32 trap_id, uint8 group)
{
	auto it = trap_list.begin();
	while (it != trap_list.end())
	{
		Trap* cur = it->second;
		if (cur->IsTrap() && cur->group == group && cur->trap_id != trap_id)
		{
			return true;
		}
		++it;
	}

	return false;
}

void EntityList::UpdateAllTraps(bool respawn, bool repopnow)
{
	auto it = trap_list.begin();
	while (it != trap_list.end())
	{
		Trap* cur = it->second;
		if (cur->IsTrap())
		{
			cur->UpdateTrap(respawn, repopnow);
		}
		++it;
	}

	Log(Logs::General, Logs::Traps, "All traps updated.");
}

void EntityList::GetTrapInfo(Client* c)
{
	uint32 trap_count  = 0;
	uint32 trap_number = 1;

	for (const auto& trap : trap_list) {
		auto t = trap.second;
		if (t->IsTrap()) {
			const bool is_set = (t->chkarea_timer.Enabled() && !t->reset_timer.Enabled());

			c->Message(
				Chat::White,
				fmt::format(
					"Trap {} | ID: {} Active: {} Coordinates: {:.2f}, {:.2f}, {:.2f}",
					trap_number,
					t->trap_id,
					is_set ? "Yes" : "No",
					t->m_Position.x,
					t->m_Position.y,
					t->m_Position.z
				).c_str()
			);

			c->Message(
				Chat::White,
				fmt::format(
					"Trap {} | Times Triggered: {} Group: {}",
					trap_number,
					t->times_triggered,
					t->group
				).c_str()
			);

			if (!t->message.empty()) {
				c->Message(
					Chat::White,
					fmt::format(
						"Trap {} | Message: {}",
						trap_number,
						t->message
					).c_str()
				);
			}

			trap_count++;
			trap_number++;
		}
	}

	if (!trap_count) {
		c->Message(Chat::White, "No traps were found.");
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Trap{} found.",
			trap_count,
			trap_count != 1 ? "s" : ""
		).c_str()
	);
}

void EntityList::ClearTrapPointers()
{
	auto it = trap_list.begin();
	while (it != trap_list.end())
	{
		Trap* cur = it->second;
		if (cur->IsTrap())
		{
			cur->DestroyHiddenTrigger();
		}
		++it;
	}
}




bool ZoneDatabase::LoadTraps(const std::string& zone_short_name, int16 instance_version)
{
	const auto& l = TrapsRepository::GetWhere(
		*this,
		fmt::format(
			"`zone` = '{}' AND `version` = {} {}",
			Strings::Escape(zone_short_name),
			instance_version,
			ContentFilterCriteria::apply()
		)
	);

	for (const auto& e : l) {
		if (e.group_) {
			if (entity_list.IsTrapGroupSpawned(e.id, e.group_)) {
				// If a member of our group is already spawned skip loading this trap.
				continue;
			}
		}

		auto t = new Trap();

		t->trap_id                = e.id;
		t->db_id                  = e.id;
		t->m_Position             = glm::vec3(e.x, e.y, e.z);
		t->effect                 = e.effect;
		t->effectvalue            = e.effectvalue;
		t->effectvalue2           = e.effectvalue2;
		t->skill                  = e.skill;
		t->maxzdiff               = e.maxzdiff;
		t->radius                 = e.radius;
		t->chance                 = e.chance;
		t->message                = e.message;
		t->respawn_time           = e.respawn_time;
		t->respawn_var            = e.respawn_var;
		t->level                  = e.level;
		t->group                  = e.group_;
		t->triggered_number       = e.triggered_number;
		t->despawn_when_triggered = e.despawn_when_triggered;
		t->undetectable           = e.undetectable;

		entity_list.AddTrap(t);

		t->CreateHiddenTrigger();
	}

	LogInfo(
		"Loaded [{}] Trap{}",
		Strings::Commify(l.size()),
		l.size() != 1 ? "s" : ""
	);

	return true;
}

void Trap::CreateHiddenTrigger()
{
	if(hiddenTrigger)
		return;

	const NPCType *base_type = content_db.LoadNPCTypesData(500);
	auto make_npc = new NPCType;
	memcpy(make_npc, base_type, sizeof(NPCType));
	make_npc->max_hp = 100000;
	make_npc->current_hp = 100000;
	strcpy(make_npc->name, "a_trap");
	make_npc->runspeed = 0.0f;
	make_npc->bodytype = BodyType::Special;
	make_npc->race = 127;
	make_npc->gender = Gender::Male;
	make_npc->loottable_id = 0;
	make_npc->npc_spells_id = 0;
	make_npc->d_melee_texture1 = 0;
	make_npc->d_melee_texture2 = 0;
	make_npc->trackable = 0;
	make_npc->level = level;
	strcpy(make_npc->special_abilities, "19,1^20,1^24,1^25,1");
	NPC* npca = new NPC(make_npc, nullptr, glm::vec4(m_Position, 0.0f), GravityBehavior::Flying);
	npca->GiveNPCTypeData(make_npc);
	entity_list.AddNPC(npca);

	SetHiddenTrigger(npca);
}

bool ZoneDatabase::SetTrapData(Trap* t, bool repop)
{
	const auto& l = TrapsRepository::GetWhere(
		*this,
		fmt::format(
			"`zone` = '{}' AND `id` = {}{}",
			zone->GetShortName(),
			t->db_id,
			(
				t->group ?
				fmt::format(
					" AND `group` = {} ORDER BY RAND() LIMIT 1",
					t->group
				) :
				""
			)
		)
	);

	if (l.empty()) {
		return false;
	}

	const uint32 before_id = t->db_id;

	for (const auto& e : l) {
		t->db_id                  = e.id;
		t->m_Position             = glm::vec3(e.x, e.y, e.z);
		t->effect                 = e.effect;
		t->effectvalue            = e.effectvalue;
		t->effectvalue2           = e.effectvalue2;
		t->skill                  = e.skill;
		t->maxzdiff               = e.maxzdiff;
		t->radius                 = e.radius;
		t->chance                 = e.chance;
		t->message                = e.message;
		t->respawn_var            = e.respawn_var;
		t->level                  = e.level;
		t->triggered_number       = e.triggered_number;
		t->despawn_when_triggered = e.despawn_when_triggered;
		t->undetectable           = e.undetectable;
		t->respawn_time           = e.respawn_time;

		t->CreateHiddenTrigger();


		if (repop) {
			t->chkarea_timer.Enable();
		} else {
			t->respawn_timer.Start((t->respawn_time + zone->random.Int(0, t->respawn_var)) * 1000);
		}

		if (t->trap_id != t->db_id) {
			LogTraps("Trap ({}) DBID has changed from {} to {}.", t->trap_id, before_id, t->db_id);
		}

		return true;
	}

	return false;
}

void Trap::UpdateTrap(bool respawn, bool repopnow)
{
	respawn_timer.Disable();
	chkarea_timer.Disable();
	reset_timer.Disable();
	if (hiddenTrigger)
	{
		hiddenTrigger->Depop();
		SetHiddenTrigger(nullptr);
	}
	times_triggered = 0;
	Client* trigger = entity_list.GetClientByCharID(charid);
	if (trigger)
	{
		trigger->trapid = 0;
	}
	charid = 0;
	if (respawn)
	{
		content_db.SetTrapData(this, repopnow);
	}
}
