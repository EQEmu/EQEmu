/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2003 EQEMu Development Team (http://eqemulator.net)

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

#include "entity.h"

#include "common/data_verification.h"
#include "common/features.h"
#include "common/guilds.h"
#include "zone/bot.h"
#include "zone/dialogue_window.h"
#include "zone/dynamic_zone.h"
#include "zone/guild_mgr.h"
#include "zone/npc_scale_manager.h"
#include "zone/petitions.h"
#include "zone/quest_parser_collection.h"
#include "zone/raids.h"
#include "zone/string_ids.h"
#include "zone/water_map.h"
#include "zone/worldserver.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <iostream>

extern Zone *zone;
extern volatile bool is_zone_loaded;
extern WorldServer worldserver;
extern uint32 numclients;

Entity::Entity()
{
	id              = 0;
	initial_id      = 0;
	spawn_timestamp = time(nullptr);
}

Entity::~Entity()
{

}

Client *Entity::CastToClient()
{
	if (this == 0x00) {
		LogError("CastToClient error (nullptr)");
		return 0;
	}
#ifdef _EQDEBUG
	if (!IsClient()) {
		LogError("CastToClient error (not client)");
		return 0;
	}
#endif

	return static_cast<Client *>(this);
}

NPC *Entity::CastToNPC()
{
#ifdef _EQDEBUG
	if (!IsNPC()) {
		LogError("CastToNPC error (Not NPC)");
		return 0;
	}
#endif
	return static_cast<NPC *>(this);
}

Mob *Entity::CastToMob()
{
#ifdef _EQDEBUG
	if (!IsMob()) {
		std::cout << "CastToMob error" << std::endl;
		return 0;
	}
#endif
	return static_cast<Mob *>(this);
}

Merc *Entity::CastToMerc()
{
#ifdef _EQDEBUG
	if (!IsMerc()) {
		std::cout << "CastToMerc error" << std::endl;
		return 0;
	}
#endif
	return static_cast<Merc *>(this);
}


Trap *Entity::CastToTrap()
{
#ifdef DEBUG
	if (!IsTrap()) {
		return 0;
	}
#endif
	return static_cast<Trap *>(this);
}

Corpse *Entity::CastToCorpse()
{
#ifdef _EQDEBUG
	if (!IsCorpse()) {
		std::cout << "CastToCorpse error" << std::endl;
		return 0;
	}
#endif
	return static_cast<Corpse *>(this);
}

Object *Entity::CastToObject()
{
#ifdef _EQDEBUG
	if (!IsObject()) {
		std::cout << "CastToObject error" << std::endl;
		return 0;
	}
#endif
	return static_cast<Object *>(this);
}

/*Group* Entity::CastToGroup() {
#ifdef _EQDEBUG
	if(!IsGroup()) {
		std::cout << "CastToGroup error" << std::endl;
		return 0;
	}
#endif
	return static_cast<Group*>(this);
}*/

Doors *Entity::CastToDoors()
{
	return static_cast<Doors *>(this);
}

Beacon *Entity::CastToBeacon()
{
	return static_cast<Beacon *>(this);
}

Encounter *Entity::CastToEncounter()
{
	return static_cast<Encounter *>(this);
}

const Client *Entity::CastToClient() const
{
	if (this == 0x00) {
		std::cout << "CastToClient error (nullptr)" << std::endl;
		return 0;
	}
#ifdef _EQDEBUG
	if (!IsClient()) {
		std::cout << "CastToClient error (not client?)" << std::endl;
		return 0;
	}
#endif
	return static_cast<const Client *>(this);
}

const NPC *Entity::CastToNPC() const
{
#ifdef _EQDEBUG
	if (!IsNPC()) {
		std::cout << "CastToNPC error" << std::endl;
		return 0;
	}
#endif
	return static_cast<const NPC *>(this);
}

const Mob *Entity::CastToMob() const
{
#ifdef _EQDEBUG
	if (!IsMob()) {
		std::cout << "CastToMob error" << std::endl;
		return 0;
	}
#endif
	return static_cast<const Mob *>(this);
}

const Merc *Entity::CastToMerc() const
{
#ifdef _EQDEBUG
	if (!IsMerc()) {
		std::cout << "CastToMerc error" << std::endl;
		return 0;
	}
#endif
	return static_cast<const Merc *>(this);
}

const Trap *Entity::CastToTrap() const
{
#ifdef DEBUG
	if (!IsTrap()) {
		return 0;
	}
#endif
	return static_cast<const Trap *>(this);
}

const Corpse *Entity::CastToCorpse() const
{
#ifdef _EQDEBUG
	if (!IsCorpse()) {
		std::cout << "CastToCorpse error" << std::endl;
		return 0;
	}
#endif
	return static_cast<const Corpse *>(this);
}

const Object *Entity::CastToObject() const
{
#ifdef _EQDEBUG
	if (!IsObject()) {
		std::cout << "CastToObject error" << std::endl;
		return 0;
	}
#endif
	return static_cast<const Object *>(this);
}

const Doors *Entity::CastToDoors() const
{
	return static_cast<const Doors *>(this);
}

const Beacon* Entity::CastToBeacon() const
{
	return static_cast<const Beacon *>(this);
}

const Encounter* Entity::CastToEncounter() const
{
	return static_cast<const Encounter *>(this);
}

Bot *Entity::CastToBot()
{
	return static_cast<Bot *>(this);
}

const Bot *Entity::CastToBot() const
{
	return static_cast<const Bot *>(this);
}

EntityList::EntityList()
	:
	object_timer(5000),
	door_timer(5000),
	corpse_timer(2000),
	group_timer(1000),
	raid_timer(1000),
	trap_timer(1000)
{
	// set up ids between 1 and 1500
	// neither client or server performs well if you have
	// enough entities to exhaust this list
	for (uint16 i = 1; i <= 1500; i++)
		free_ids.push(i);
}

EntityList::~EntityList()
{
	//must call this before the list is destroyed, or else it will try to
	//delete the NPCs in the list, which it cannot do.
	RemoveAllLocalities();
}

bool EntityList::CanAddHateForMob(Mob *p)
{
	int count = 0;

	auto it = npc_list.begin();
	while (it != npc_list.end()) {
		NPC *npc = it->second;
		if (npc->IsOnHatelist(p))
			count++;
		// no need to continue if we already hit the limit
		if (count > 3)
			return false;
		++it;
	}

	if (count <= 2)
		return true;
	return false;
}

void EntityList::AddClient(Client *client)
{
	client->SetID(GetFreeID());
	client_list.emplace(std::pair<uint16, Client *>(client->GetID(), client));
	mob_list.emplace(std::pair<uint16, Mob *>(client->GetID(), client));
}


void EntityList::TrapProcess()
{
	if (numclients < 1)
		return;

	if (trap_list.empty()) {
		trap_timer.Disable();
		return;
	}

	auto it = trap_list.begin();
	while (it != trap_list.end()) {
		if (!it->second->Process()) {
			safe_delete(it->second);
			free_ids.push(it->first);
			it = trap_list.erase(it);
		} else {
			++it;
		}
	}
}


// Debug function -- checks to see if group_list has any nullptr entries.
// Meant to be called after each group-related function, in order
// to track down bugs.
void EntityList::CheckGroupList (const char *fname, const int fline)
{
	std::list<Group *>::iterator it;

	for (it = group_list.begin(); it != group_list.end(); ++it)
	{
		if (*it == nullptr)
		{
			LogError("nullptr group, [{}]:[{}]", fname, fline);
		}
	}
}

void EntityList::GroupProcess()
{
	if (numclients < 1)
		return;

	if (group_list.empty()) {
		group_timer.Disable();
		return;
	}

	for (auto &group : group_list)
		group->Process();


}

void EntityList::QueueToGroupsForNPCHealthAA(Mob *sender, const EQApplicationPacket *app)
{
	for (auto &group : group_list)
		group->QueueHPPacketsForNPCHealthAA(sender, app);
}

void EntityList::RaidProcess()
{
	if (numclients < 1)
		return;

	if (raid_list.empty()) {
		raid_timer.Disable();
		return;
	}

	for (auto &raid : raid_list)
		raid->Process();
}

void EntityList::DoorProcess()
{
	if (zone && zone->IsIdleWhenEmpty()) {
		if (numclients < 1) {
			return;
		}
	}

	if (door_list.empty()) {
		door_timer.Disable();
		return;
	}

	auto it = door_list.begin();
	while (it != door_list.end()) {
		if (!it->second->Process()) {
			safe_delete(it->second);
			free_ids.push(it->first);
			it = door_list.erase(it);
		}
		++it;
	}
}

void EntityList::ObjectProcess()
{
	if (object_list.empty()) {
		object_timer.Disable();
		return;
	}

	auto it = object_list.begin();
	while (it != object_list.end()) {
		if (!it->second->Process()) {
			safe_delete(it->second);
			free_ids.push(it->first);
			it = object_list.erase(it);
		} else {
			++it;
		}
	}
}

void EntityList::CorpseProcess()
{
	if (corpse_list.empty()) {
		corpse_timer.Disable(); // No corpses in list
		return;
	}

	auto it = corpse_list.begin();
	while (it != corpse_list.end()) {
		if (!it->second->Process()) {
			safe_delete(it->second);
			free_ids.push(it->first);
			it = corpse_list.erase(it);
		} else {
			++it;
		}
	}
}

void EntityList::MobProcess()
{
	bool mob_dead;

	auto it = mob_list.begin();
	while (it != mob_list.end()) {
		uint16 id = it->first;
		Mob *mob = it->second;

		size_t sz = mob_list.size();

		static int   old_client_count  = 0;
		static Timer *mob_settle_timer = new Timer();

		if (zone->IsIdleWhenEmpty()) {
			if (
				numclients == 0 &&
				old_client_count > 0 &&
				zone->GetSecondsBeforeIdle() > 0
			) {
				LogInfo(
					"Zone will go into an idle state after [{}] second{}.",
					zone->GetSecondsBeforeIdle(),
					zone->GetSecondsBeforeIdle() != 1 ? "s" : ""
				);
				mob_settle_timer->Start(zone->GetSecondsBeforeIdle() * 1000);
			}

			if (numclients == 0 && mob_settle_timer->Check()) {
				LogInfo(
					"Zone has gone idle after [{}] second{}.",
					zone->GetSecondsBeforeIdle(),
					zone->GetSecondsBeforeIdle() != 1 ? "s" : ""
				);

				CheckToClearTraderAndBuyerTables();

				mob_settle_timer->Disable();
			}

			// Disable settle timer if someone zones into empty zone
			if (numclients > 0 || mob_settle_timer->Check()) {
				if (mob_settle_timer->Enabled()) {
					LogInfo("Zone is no longer scheduled to go idle.");
					mob_settle_timer->Disable();
				}
			}

			old_client_count = numclients;

			Spawn2* s2 = mob->CastToNPC()->respawn2;

			// Perform normal mob processing if any of these are true:
			//	-- zone is not empty
			//	-- the entity's spawn2 point is marked as path_while_zone_idle
			//	-- the zone is newly empty and we're allowing mobs to settle
			if (
				numclients > 0 || zone->quest_idle_override ||
				(mob && s2 && s2->PathWhenZoneIdle()) ||
				mob_settle_timer->Enabled()
			) {
				mob_dead = !mob->Process();
			} else {
				// spawn_events can cause spawns and deaths while zone empty.
				// At the very least, process that.
				mob_dead = mob->CastToNPC()->GetDepop();
			}
		} else {
			mob_dead = !mob->Process();
		}

		size_t a_sz = mob_list.size();

		if (a_sz > sz) {
			//increased size can potentially screw with iterators so reset it to current value
			//if buckets are re-orderered we may skip a process here and there but since
			//process happens so often it shouldn't matter much
			it = mob_list.find(id);
			++it;
		} else {
			++it;
		}

		if (mob_dead) {
			if (mob->IsMerc()) {
				entity_list.RemoveMerc(id);
			} else if (mob->IsBot()) {
				entity_list.RemoveBot(id);
			} else if (mob->IsNPC()) {
				entity_list.RemoveNPC(id);
			} else {
#ifdef _WINDOWS
				struct in_addr in;
				in.s_addr = mob->CastToClient()->GetIP();
				LogInfo("Dropping client: Process=false, ip=[{}] port=[{}]", inet_ntoa(in), mob->CastToClient()->GetPort());
#endif

				Group* g = GetGroupByMob(mob);
				if (g) {
					g->DelMember(mob);
				}

				Raid* r = entity_list.GetRaidByClient(mob->CastToClient());
				if (r) {
					r->MemberZoned(mob->CastToClient());
				}

				entity_list.RemoveClient(id);
			}

			entity_list.RemoveMob(id);
		}
	}
}

void EntityList::BeaconProcess()
{
	auto it = beacon_list.begin();
	while (it != beacon_list.end()) {
		if (!it->second->Process()) {
			safe_delete(it->second);
			free_ids.push(it->first);
			it = beacon_list.erase(it);
		} else {
			++it;
		}
	}
}

void EntityList::EncounterProcess()
{
	auto it = encounter_list.begin();
	while (it != encounter_list.end()) {
		if (!it->second->Process()) {
			// if Process is returning false here, we probably just got called from ReloadQuests .. oh well
			parse->RemoveEncounter(it->second->GetEncounterName());
			safe_delete(it->second);
			free_ids.push(it->first);
			it = encounter_list.erase(it);
		}
		else {
			++it;
		}
	}
}

void EntityList::AddGroup(Group *group)
{
	if (group == nullptr)	//this seems to be happening somehow...
		return;

	uint32 gid = worldserver.NextGroupID();
	if (gid == 0) {
		LogError("Unable to get new group ID from world server. group is going to be broken");
		return;
	}

	AddGroup(group, gid);

}

void EntityList::AddGroup(Group *group, uint32 gid)
{
	group->SetID(gid);
	group_list.push_back(group);
	if (!group_timer.Enabled())
		group_timer.Start();
#if EQDEBUG >= 5
	CheckGroupList(__FILE__, __LINE__);
#endif
}

void EntityList::AddRaid(Raid *raid)
{
	if (raid == nullptr)
		return;

	uint32 gid = worldserver.NextGroupID();
	if (gid == 0) {
		LogError("Unable to get new group ID from world server. group is going to be broken");
		return;
	}

	AddRaid(raid, gid);
}

void EntityList::AddRaid(Raid *raid, uint32 gid)
{
	raid->SetID(gid);
	raid_list.push_back(raid);
	if (!raid_timer.Enabled())
		raid_timer.Start();
}


void EntityList::AddCorpse(Corpse *corpse, uint32 in_id)
{
	if (corpse == 0)
		return;

	if (in_id == 0xFFFFFFFF)
		corpse->SetID(GetFreeID());
	else
		corpse->SetID(in_id);

	corpse->CalcCorpseName();
	corpse_list.emplace(std::pair<uint16, Corpse *>(corpse->GetID(), corpse));

	if (!corpse_timer.Enabled())
		corpse_timer.Start();
}

void EntityList::AddNPC(NPC *npc, bool send_spawn_packet, bool dont_queue)
{
	npc->SetID(GetFreeID());

	//If this is not set here we will despawn pets from new AC changes
	auto owner_id = npc->GetOwnerID();
	if (owner_id) {
		auto owner = entity_list.GetMob(owner_id);
		if (owner) {
			owner->SetPetID(npc->GetID());
		}
	}

	npc_list.emplace(std::pair<uint16, NPC *>(npc->GetID(), npc));
	mob_list.emplace(std::pair<uint16, Mob *>(npc->GetID(), npc));

	entity_list.ScanCloseMobs(npc);

	if (parse->HasQuestSub(npc->GetNPCTypeID(), EVENT_SPAWN)) {
		parse->EventNPC(EVENT_SPAWN, npc, nullptr, "", 0);
	}

	const uint32 emote_id = npc->GetEmoteID();
	if (emote_id) {
		npc->DoNPCEmote(EQ::constants::EmoteEventTypes::OnSpawn, emote_id);
	}

	npc->SetSpawned();

	if (send_spawn_packet) {
		if (dont_queue) {
			auto app = new EQApplicationPacket;
			npc->CreateSpawnPacket(app, npc);
			QueueClients(npc, app);
			npc->SendArmorAppearance();
			npc->SetAppearance(npc->GetGuardPointAnim(), false);

			if (!npc->IsTargetable()) {
				npc->SendTargetable(false);
			}

			safe_delete(app);
		} else {
			auto ns = new NewSpawn_Struct;
			memset(ns, 0, sizeof(NewSpawn_Struct));
			npc->FillSpawnStruct(ns, nullptr);
			AddToSpawnQueue(npc->GetID(), &ns);
			safe_delete(ns);
		}

		if (npc->IsFindable()) {
			UpdateFindableNPCState(npc, false);
		}
	}

	npc->SendPositionToClients();

	if (parse->HasQuestSub(ZONE_CONTROLLER_NPC_ID, EVENT_SPAWN_ZONE)) {
		npc->DispatchZoneControllerEvent(EVENT_SPAWN_ZONE, npc, "", 0, nullptr);
	}

	if (parse->ZoneHasQuestSub(EVENT_SPAWN_ZONE)) {
		std::vector<std::any> args = { npc };
		parse->EventZone(EVENT_SPAWN_ZONE, zone, "", 0, &args);
	}

	if (zone->HasMap() && zone->HasWaterMap()) {
		npc->SetSpawnedInWater(false);
		if (zone->watermap->InLiquid(npc->GetPosition())) {
			npc->SetSpawnedInWater(true);
		}
	}
}

void EntityList::AddMerc(Merc *merc, bool SendSpawnPacket, bool dontqueue)
{
	if (merc)
	{
		merc->SetID(GetFreeID());
		merc->SetSpawned();
		if (SendSpawnPacket)
		{
			if (dontqueue) {
				// Send immediately
				auto outapp = new EQApplicationPacket();
				merc->CreateSpawnPacket(outapp);
				outapp->priority = 6;
				QueueClients(merc, outapp, true);
				safe_delete(outapp);
			} else {
				// Queue the packet
				auto ns = new NewSpawn_Struct;
				memset(ns, 0, sizeof(NewSpawn_Struct));
				merc->FillSpawnStruct(ns, 0);
				AddToSpawnQueue(merc->GetID(), &ns);
				safe_delete(ns);
			}
		}

		merc_list.emplace(std::pair<uint16, Merc *>(merc->GetID(), merc));
		mob_list.emplace(std::pair<uint16, Mob *>(merc->GetID(), merc));

		if (parse->MercHasQuestSub(EVENT_SPAWN)) {
			parse->EventMerc(EVENT_SPAWN, merc, nullptr, "", 0);
		}
	}
}

void EntityList::AddObject(Object *obj, bool SendSpawnPacket)
{
	obj->SetID(GetFreeID());

	if (SendSpawnPacket) {
		EQApplicationPacket app;
		obj->CreateSpawnPacket(&app);
		#if (EQDEBUG >= 6)
			DumpPacket(&app);
		#endif
		QueueClients(0, &app,false);
	}

	object_list.emplace(std::pair<uint16, Object *>(obj->GetID(), obj));

	if (!object_timer.Enabled())
		object_timer.Start();
}

void EntityList::AddDoor(Doors *door)
{
	door->SetEntityID(GetFreeID());
	door_list.emplace(std::pair<uint16, Doors *>(door->GetEntityID(), door));

	if (!door_timer.Enabled())
		door_timer.Start();
}

void EntityList::AddTrap(Trap *trap)
{
	trap->SetID(GetFreeID());
	trap_list.emplace(std::pair<uint16, Trap *>(trap->GetID(), trap));
	if (!trap_timer.Enabled())
		trap_timer.Start();
}

void EntityList::AddBeacon(Beacon *beacon)
{
	beacon->SetID(GetFreeID());
	beacon_list.emplace(std::pair<uint16, Beacon *>(beacon->GetID(), beacon));
}

void EntityList::AddEncounter(Encounter *encounter)
{
	encounter->SetID(GetFreeID());
	encounter_list.emplace(std::pair<uint16, Encounter *>(encounter->GetID(), encounter));
}

void EntityList::AddToSpawnQueue(uint16 entityid, NewSpawn_Struct **ns)
{
	uint32 count;
	if ((count = (client_list.size())) == 0)
		return;
	SpawnQueue.Append(*ns);
	NumSpawnsOnQueue++;
	if (tsFirstSpawnOnQueue == 0xFFFFFFFF)
		tsFirstSpawnOnQueue = Timer::GetCurrentTime();
	*ns = nullptr;
}

void EntityList::CheckSpawnQueue()
{
	// Send the stuff if the oldest packet on the queue is older than 50ms -Quagmire
	if (tsFirstSpawnOnQueue != 0xFFFFFFFF && (Timer::GetCurrentTime() - tsFirstSpawnOnQueue) > 50) {
		LinkedListIterator<NewSpawn_Struct *> iterator(SpawnQueue);
		EQApplicationPacket *outapp = 0;

		iterator.Reset();
		NewSpawn_Struct	*ns;

		while(iterator.MoreElements()) {
			outapp = new EQApplicationPacket;
			ns = iterator.GetData();
			Mob::CreateSpawnPacket(outapp, ns);
			QueueClients(0, outapp);
			auto it = npc_list.find(ns->spawn.spawnId);
			if (it == npc_list.end()) {
				// We must of despawned, hope that's the reason!
				LogError("Error in EntityList::CheckSpawnQueue: Unable to find NPC for spawnId [{}]", ns->spawn.spawnId);
			}
			else {
				NPC *pnpc = it->second;
				pnpc->SendArmorAppearance();
				pnpc->SetAppearance(pnpc->GetGuardPointAnim(), false);
				if (!pnpc->IsTargetable()) {
					pnpc->SendTargetable(false);
				}
				pnpc->SendPositionToClients();
			}
			safe_delete(outapp);
			iterator.RemoveCurrent();
		}
		tsFirstSpawnOnQueue = 0xFFFFFFFF;
		NumSpawnsOnQueue = 0;
	}
}

Doors *EntityList::FindDoor(uint8 door_id)
{
	if (door_id == 0 || door_list.empty())
		return nullptr;

	auto it = door_list.begin();
	while (it != door_list.end()) {
		if (it->second->GetDoorID() == door_id)
			return it->second;
		++it;
	}

	return nullptr;
}

Object *EntityList::FindObject(uint32 object_id)
{
	if (object_id == 0 || object_list.empty())
		return nullptr;

	auto it = object_list.begin();
	while (it != object_list.end()) {
		if (it->second->GetDBID() == object_id)
			return it->second;
		++it;
	}

	return nullptr;
}

Object *EntityList::FindNearbyObject(float x, float y, float z, float radius)
{
	if (object_list.empty())
		return nullptr;

	float ox;
	float oy;
	float oz;

	auto it = object_list.begin();
	while (it != object_list.end()) {
		Object *object = it->second;

		object->GetLocation(&ox, &oy, &oz);

		ox = (x < ox) ? (ox - x) : (x - ox);
		oy = (y < oy) ? (oy - y) : (y - oy);
		oz = (z < oz) ? (oz - z) : (z - oz);

		if ((ox <= radius) && (oy <= radius) && (oz <= radius))
			return object;

		++it;
	}

	return nullptr;
}

bool EntityList::MakeDoorSpawnPacket(EQApplicationPacket *app, Client *client)
{
	if (door_list.empty())
		return false;

	uint32 mask_test = client->ClientVersionBit();
	int count = 0;

	auto it = door_list.begin();
	while (it != door_list.end()) {
		if ((it->second->GetClientVersionMask() & mask_test) &&
			strlen(it->second->GetDoorName()) > 3)
			count++;
		++it;
	}

	if (count == 0 || count > 500)
		return false;

	uint32 length = count * sizeof(Door_Struct);
	auto packet_buffer = new uchar[length];
	memset(packet_buffer, 0, length);
	uchar *ptr = packet_buffer;
	Doors *door;
	Door_Struct new_door;

	it = door_list.begin();
	while (it != door_list.end()) {
		door = it->second;
		if (door && (door->GetClientVersionMask() & mask_test) &&
			strlen(door->GetDoorName()) > 3) {
			memset(&new_door, 0, sizeof(new_door));
			memcpy(new_door.name, door->GetDoorName(), 32);

			auto position = door->GetPosition();

			new_door.xPos = position.x;
			new_door.yPos = position.y;
			new_door.zPos = position.z;
			new_door.heading = position.w;

			new_door.incline = door->GetIncline();
			new_door.size = door->GetSize();
			new_door.doorId = door->GetDoorID();
			new_door.opentype = door->GetOpenType();

			Log(Logs::General, Logs::Doors, "Door timer_disable: %s door_id: %u is_open: %s invert_state: %i",
				(door->GetDisableTimer() ? "true" : "false"),
				door->GetDoorID(),
				(door->IsDoorOpen() ? "true" : "false"),
				door->GetInvertState()
			);

			new_door.state_at_spawn = (door->GetInvertState() ? !door->IsDoorOpen() : door->IsDoorOpen());
			new_door.invert_state = door->GetInvertState();

			new_door.door_param = door->GetDoorParam();
			memcpy(ptr, &new_door, sizeof(new_door));
			ptr += sizeof(new_door);
			*(ptr - 1) = 0x01;
			*(ptr - 3) = 0x01;
		}
		++it;
	}

	app->SetOpcode(OP_SpawnDoor);
	app->size = length;
	app->pBuffer = packet_buffer;
	return true;
}

Entity *EntityList::GetEntityMob(uint16 id)
{
	auto it = mob_list.find(id);
	if (it != mob_list.end())
		return it->second;
	return nullptr;
}

Entity *EntityList::GetEntityMerc(uint16 id)
{
	auto it = merc_list.find(id);
	if (it != merc_list.end())
		return it->second;
	return nullptr;
}

Entity *EntityList::GetEntityMob(const char *name)
{
	if (name == 0 || mob_list.empty())
		return 0;

	auto it = mob_list.begin();
	while (it != mob_list.end()) {
		if (strcasecmp(it->second->GetName(), name) == 0)
			return it->second;
		++it;
	}

	return nullptr;
}

Entity *EntityList::GetEntityDoor(uint16 id)
{
	auto it = door_list.find(id);
	if (it != door_list.end())
		return it->second;
	return nullptr;
}

Entity *EntityList::GetEntityCorpse(uint16 id)
{
	auto it = corpse_list.find(id);
	if (it != corpse_list.end())
		return it->second;
	return nullptr;
}

Entity *EntityList::GetEntityCorpse(const char *name)
{
	if (name == 0 || corpse_list.empty())
		return nullptr;

	auto it = corpse_list.begin();
	while (it != corpse_list.end()) {
		if (strcasecmp(it->second->GetName(), name) == 0)
			return it->second;
		++it;
	}

	return nullptr;
}

Entity *EntityList::GetEntityTrap(uint16 id)
{
	auto it = trap_list.find(id);
	if (it != trap_list.end())
		return it->second;
	return nullptr;
}

Entity *EntityList::GetEntityObject(uint16 id)
{
	auto it = object_list.find(id);
	if (it != object_list.end())
		return it->second;
	return nullptr;
}

Entity *EntityList::GetEntityBeacon(uint16 id)
{
	auto it = beacon_list.find(id);
	if (it != beacon_list.end())
		return it->second;
	return nullptr;
}

Entity *EntityList::GetEntityEncounter(uint16 id)
{
	auto it = encounter_list.find(id);
	if (it != encounter_list.end())
		return it->second;
	return nullptr;
}

Entity *EntityList::GetID(uint16 get_id)
{
	Entity *ent = 0;
	if ((ent = entity_list.GetEntityMob(get_id)) != 0)
		return ent;
	else if ((ent=entity_list.GetEntityDoor(get_id)) != 0)
		return ent;
	else if ((ent=entity_list.GetEntityCorpse(get_id)) != 0)
		return ent;
	else if ((ent=entity_list.GetEntityObject(get_id)) != 0)
		return ent;
	else if ((ent=entity_list.GetEntityTrap(get_id)) != 0)
		return ent;
	else if ((ent=entity_list.GetEntityBeacon(get_id)) != 0)
		return ent;
	else if ((ent = entity_list.GetEntityEncounter(get_id)) != 0)
		return ent;
	else
		return 0;
}

NPC *EntityList::GetNPCByNPCTypeID(uint32 npc_id)
{
	if (npc_id == 0 || npc_list.empty())
		return nullptr;

	auto it = npc_list.begin();
	while (it != npc_list.end()) {
		if (it->second->GetNPCTypeID() == npc_id)
			return it->second;
		++it;
	}

	return nullptr;
}

NPC *EntityList::GetNPCBySpawnID(uint32 spawn_id)
{
	if (spawn_id == 0 || npc_list.empty()) {
		return nullptr;
	}

	auto it = npc_list.begin();
	while (it != npc_list.end()) {
		if (it->second->GetSpawnGroupId() == spawn_id) {
			return it->second;
		}
		++it;
	}
	return nullptr;
}

Mob *EntityList::GetMob(uint16 get_id)
{
	Entity *ent = nullptr;

	if (get_id == 0)
		return nullptr;

	if ((ent = entity_list.GetEntityMob(get_id)))
		return ent->CastToMob();
	else if ((ent = entity_list.GetEntityCorpse(get_id)))
		return ent->CastToMob();

	return nullptr;
}

Mob *EntityList::GetMob(const char *name)
{
	Entity* ent = nullptr;

	if (name == 0)
		return nullptr;

	if ((ent = entity_list.GetEntityMob(name)))
		return ent->CastToMob();
	else if ((ent = entity_list.GetEntityCorpse(name)))
		return ent->CastToMob();

	return nullptr;
}

Mob *EntityList::GetMobByNpcTypeID(uint32 get_id)
{
	if (get_id == 0 || mob_list.empty())
		return 0;

	auto it = mob_list.begin();
	while (it != mob_list.end()) {
		if (it->second->GetNPCTypeID() == get_id)
			return it->second;
		++it;
	}
	return nullptr;
}

bool EntityList::IsMobSpawnedByNpcTypeID(uint32 get_id)
{
	if (get_id == 0 || npc_list.empty())
		return false;

	auto it = npc_list.begin();
	while (it != npc_list.end()) {
		// Mobs will have a 0 as their GetID() if they're dead
		if (it->second->GetNPCTypeID() == get_id && it->second->GetID() != 0)
			return true;
		++it;
	}

	return false;
}

bool EntityList::IsNPCSpawned(std::vector<uint32> npc_ids)
{
	return CountSpawnedNPCs(npc_ids) != 0;
}

uint32 EntityList::CountSpawnedNPCs(std::vector<uint32> npc_ids)
{
	uint32 npc_count = 0;
	if (npc_list.empty() || npc_ids.empty()) {
		return npc_count;
	}

	for (auto current_npc : npc_list) {
		if (
			std::find(
				npc_ids.begin(),
				npc_ids.end(),
				current_npc.second->GetNPCTypeID()
			) != npc_ids.end() &&
			current_npc.second->GetID() != 0
		) {
			npc_count++;
		}
	}

	return npc_count;
}

Object *EntityList::GetObjectByDBID(uint32 id)
{
	if (id == 0 || object_list.empty())
		return nullptr;

	auto it = object_list.begin();
	while (it != object_list.end()) {
		if (it->second->GetDBID() == id)
			return it->second;
		++it;
	}
	return nullptr;
}

Doors *EntityList::GetDoorsByDBID(uint32 id)
{
	if (id == 0 || door_list.empty())
		return nullptr;

	auto it = door_list.begin();
	while (it != door_list.end()) {
		if (it->second->GetDoorDBID() == id)
			return it->second;
		++it;
	}

	return nullptr;
}

Doors *EntityList::GetDoorsByDoorID(uint32 id)
{
	if (id == 0 || door_list.empty())
		return nullptr;

	auto it = door_list.begin();
	while (it != door_list.end()) {
		if (it->second->CastToDoors()->GetDoorID() == id)
			return it->second;
		++it;
	}

	return nullptr;
}

uint16 EntityList::GetFreeID()
{
	if (free_ids.empty()) { // hopefully this will never be true
		// The client has a hard cap on entity count some where
		// Neither the client or server performs well with a lot entities either
		uint16 newid = 1500;
		while (true) {
			newid++;
			if (GetID(newid) == nullptr)
				return newid;
		}
	}

	uint16 newid = free_ids.front();
	free_ids.pop();
	return newid;
}

// if no language skill is specified, sent with 100 skill
void EntityList::ChannelMessage(Mob *from, uint8 chan_num, uint8 language, const char *message, ...)
{
	ChannelMessage(from, chan_num, language, Language::MaxValue, message);
}

void EntityList::ChannelMessage(Mob *from, uint8 chan_num, uint8 language,
		uint8 lang_skill, const char *message, ...)
{
	va_list argptr;
	char buffer[4096];

	va_start(argptr, message);
	vsnprintf(buffer, 4096, message, argptr);
	va_end(argptr);

	auto it = client_list.begin();
	while(it != client_list.end()) {
		Client *client = it->second;
		eqFilterType filter = FilterNone;
		if (chan_num == ChatChannel_Shout) //shout
			filter = FilterShouts;
		else if (chan_num == ChatChannel_Auction) //auction
			filter = FilterAuctions;
		//
		// Only say is limited in range
		if (chan_num != ChatChannel_Say || Distance(client->GetPosition(), from->GetPosition()) < 200)
			if (filter == FilterNone || client->GetFilter(filter) != FilterHide)
				client->ChannelMessageSend(from->GetName(), 0, chan_num, language, lang_skill, buffer);
		++it;
	}
}

void EntityList::SendZoneSpawns(Client *client)
{
	EQApplicationPacket *app;
	auto it = mob_list.begin();
	while (it != mob_list.end()) {
		Mob *ent = it->second;
		if (!ent->InZone() || !ent->ShouldISpawnFor(client)) {
			++it;
			continue;
		}

		app = new EQApplicationPacket;
		it->second->CastToMob()->CreateSpawnPacket(app); // TODO: Use zonespawns opcode instead
		client->QueuePacket(app, true, Client::CLIENT_CONNECTED);
		safe_delete(app);
		++it;
	}
}

void EntityList::SendZoneSpawnsBulk(Client *client)
{
	NewSpawn_Struct     ns{};
	Mob                 *spawn;
	EQApplicationPacket *app;

	uint32 max_spawns = 100;

	if (max_spawns > mob_list.size()) {
		max_spawns = static_cast<uint32>(mob_list.size());
	}

	auto            bulk_zone_spawn_packet = new BulkZoneSpawnPacket(client, max_spawns);
	const glm::vec4 &client_position       = client->GetPosition();
	const float     distance_max           = (600.0 * 600.0);

	for (auto & it : mob_list) {
		spawn = it.second;
		if (spawn && spawn->GetID() > 0 && spawn->Spawned()) {
			if (!spawn->ShouldISpawnFor(client)) {
				continue;
			}

			const glm::vec4 &spawn_position = spawn->GetPosition();

			bool is_delayed_packet = (
				DistanceSquared(client_position, spawn_position) > distance_max ||
				(spawn->IsClient() && (spawn->GetRace() == Race::MinorIllusion || spawn->GetRace() == Race::Tree))
			);

			if (is_delayed_packet) {
				app = new EQApplicationPacket;
				spawn->CreateSpawnPacket(app);
				client->QueuePacket(app, true, Client::CLIENT_CONNECTED);
				safe_delete(app);
			}
			else {
				memset(&ns, 0, sizeof(NewSpawn_Struct));
				spawn->FillSpawnStruct(&ns, client);
				bulk_zone_spawn_packet->AddSpawn(&ns);
			}

			spawn->SendArmorAppearance(client);

			/**
			 * Original code kept for spawn packet research
			 *
			 * int32 race = spawn->GetRace();
			 *
			 * Illusion races on PCs don't work as a mass spawn
			 * But they will work as an add_spawn AFTER CLIENT_CONNECTED.
			 * if (spawn->IsClient() && (race == Race::MinorIllusion || race == Race::Tree)) {
			 * 	app = new EQApplicationPacket;
			 * 	spawn->CreateSpawnPacket(app);
			 * 	client->QueuePacket(app, true, Client::CLIENT_CONNECTED);
			 * 	safe_delete(app);
			 * }
			 * else {
			 * 	memset(&ns, 0, sizeof(NewSpawn_Struct));
			 * 	spawn->FillSpawnStruct(&ns, client);
			 * 	bzsp->AddSpawn(&ns);
			 * }
			 *
			 * Despite being sent in the OP_ZoneSpawns packet, the client
			 * does not display worn armor correctly so display it.
			 * spawn->SendArmorAppearance(client);
			*/
		}
	}

	safe_delete(bulk_zone_spawn_packet);
}

//this is a hack to handle a broken spawn struct
void EntityList::SendZonePVPUpdates(Client *to)
{
	auto it = client_list.begin();
	while (it != client_list.end()) {
		Client *c = it->second;
		if(c->GetPVP())
			c->SendAppearancePacket(AppearanceType::PVP, c->GetPVP(), true, false, to);
		++it;
	}
}

void EntityList::SendZoneCorpses(Client *client)
{
	EQApplicationPacket *app;

	for (auto it = corpse_list.begin(); it != corpse_list.end(); ++it) {
		Corpse *ent = it->second;
		app = new EQApplicationPacket;
		ent->CreateSpawnPacket(app);
		client->QueuePacket(app, true, Client::CLIENT_CONNECTED);
		safe_delete(app);
	}
}

void EntityList::SendZoneCorpsesBulk(Client *client)
{
	NewSpawn_Struct ns;
	Corpse *spawn;
	uint32 maxspawns = 100;

	auto bzsp = new BulkZoneSpawnPacket(client, maxspawns);

	for (auto it = corpse_list.begin(); it != corpse_list.end(); ++it) {
		spawn = it->second;
		if (spawn && spawn->InZone()) {
			memset(&ns, 0, sizeof(NewSpawn_Struct));
			spawn->FillSpawnStruct(&ns, client);
			bzsp->AddSpawn(&ns);
		}
	}
	safe_delete(bzsp);
}

void EntityList::SendZoneObjects(Client *client)
{
	auto it = object_list.begin();
	while (it != object_list.end()) {
		auto app = new EQApplicationPacket;
		it->second->CreateSpawnPacket(app);
		client->FastQueuePacket(&app);
		++it;
	}
}

void EntityList::Save()
{
	auto it = client_list.begin();
	while (it != client_list.end()) {
		it->second->Save();
		++it;
	}
}

void EntityList::ReplaceWithTarget(Mob *pOldMob, Mob *pNewTarget)
{
	if (!pNewTarget)
		return;

	auto it = mob_list.begin();
	while (it != mob_list.end()) {
		if (it->second->IsAIControlled()) {
			// replace the old mob with the new one
			if (it->second->RemoveFromHateList(pOldMob))
					it->second->AddToHateList(pNewTarget, 1, 0);
		}
		++it;
	}
}

void EntityList::RemoveFromTargets(Mob *mob, bool RemoveFromXTargets)
{
	auto it = mob_list.begin();
	while (it != mob_list.end()) {
		Mob *m = it->second;
		++it;

		if (!m) {
			continue;
		}

		if (RemoveFromXTargets && mob) {
			if (m->IsClient() && (mob->CheckAggro(m) || mob->IsOnFeignMemory(m))) {
				m->CastToClient()->RemoveXTarget(mob, false);
			// FadingMemories calls this function passing the client.
			} else if (mob->IsClient() && (m->CheckAggro(mob) || m->IsOnFeignMemory(mob))) {
				mob->CastToClient()->RemoveXTarget(m, false);
			}
		}

		m->RemoveFromHateList(mob);
		m->RemoveFromRampageList(mob);
	}
}

void EntityList::RemoveFromTargetsFadingMemories(Mob *spell_target, bool RemoveFromXTargets, uint32 max_level)
{
	for (auto &e : mob_list) {
		auto &mob = e.second;

		if (!mob) {
			continue;
		}

		if (max_level && mob->GetLevel() > max_level) {
			continue;
		}

		if (mob->GetSpecialAbility(SpecialAbility::MemoryFadeImmunity)) {
			continue;
		}

		if (RemoveFromXTargets && spell_target) {
			if (mob->IsClient() && (spell_target->CheckAggro(mob) || spell_target->IsOnFeignMemory(mob))) {
				mob->CastToClient()->RemoveXTarget(spell_target, false);
			} else if (spell_target->IsClient() && (mob->CheckAggro(spell_target) || mob->IsOnFeignMemory(spell_target))) {
				spell_target->CastToClient()->RemoveXTarget(mob, false);
			}
		}

		mob->RemoveFromHateList(spell_target);
		mob->RemoveFromRampageList(spell_target);
	}
}

void EntityList::RemoveFromXTargets(Mob *mob)
{
	auto it = client_list.begin();
	while (it != client_list.end()) {
		it->second->RemoveXTarget(mob, false);
		++it;
	}
}

void EntityList::RemoveFromAutoXTargets(Mob *mob)
{
	auto it = client_list.begin();
	while (it != client_list.end()) {
		it->second->RemoveXTarget(mob, true);
		++it;
	}
}

void EntityList::RefreshAutoXTargets(Client *c)
{
	if (!c) {
		return;
	}

	auto it = mob_list.begin();
	while (it != mob_list.end()) {
		Mob *m = it->second;
		++it;

		if (!m || m->GetHP() <= 0) {
			continue;
		}

		if ((m->CheckAggro(c) || m->IsOnFeignMemory(c)) && !c->IsXTarget(m)) {
			c->AddAutoXTarget(m, false); // we only call this before a bulk, so lets not send right away
			break;
		}

	}
}

void EntityList::RefreshClientXTargets(Client *c)
{
	if (!c) {
		return;
	}

	auto it = client_list.begin();
	while (it != client_list.end()) {
		Client *c2 = it->second;
		++it;

		if (!c2) {
			continue;
		}

		if (c2->IsClientXTarget(c)) {
			c2->UpdateClientXTarget(c);
		}
	}
}

void EntityList::QueueClientsByTarget(Mob *sender, const EQApplicationPacket *app,
		bool iSendToSender, Mob *SkipThisMob, bool ackreq, bool HoTT, uint32 ClientVersionBits, bool inspect_buffs, bool clear_target_window)
{
	auto it = client_list.begin();
	while (it != client_list.end()) {
		Client *c = it->second;
		++it;

		Mob *Target = c->GetTarget();

		if (!Target)
			continue;

		Mob *TargetsTarget = nullptr;

		TargetsTarget = Target->GetTarget();

		bool Send = false;

		if (c == SkipThisMob)
			continue;

		if (iSendToSender)
			if (c == sender)
				Send = true;

		if (c != sender) {
			if (Target == sender) {
				if (inspect_buffs) { // if inspect_buffs is true we're sending a mob's buffs to those with the LAA
					Send = clear_target_window;
					if (c->GetGM() || RuleB(Spells, AlwaysSendTargetsBuffs)) {
						if (c->GetGM()) {
							if (!c->EntityVariableExists(SEE_BUFFS_FLAG)) {
								c->Message(Chat::White, "Your GM flag allows you to always see your targets' buffs.");
								c->SetEntityVariable(SEE_BUFFS_FLAG, "1");
							}
						}

						Send = !clear_target_window;
					} else if (c->IsRaidGrouped()) {
						Raid *raid = c->GetRaid();
						if (raid) {
							uint32 gid = raid->GetGroup(c);
							if (gid < MAX_RAID_GROUPS && raid->GroupCount(gid) >= 3) {
								if (raid->GetLeadershipAA(groupAAInspectBuffs, gid))
									Send = !clear_target_window;
							}
						}
					} else {
						Group *group = c->GetGroup();
						if (group && group->GroupCount() >= 3) {
							if (group->GetLeadershipAA(groupAAInspectBuffs)) {
								Send = !clear_target_window;
							}
						}
					}
				} else {
					Send = true;
				}
			} else if (HoTT && TargetsTarget == sender) {
				Send = true;
			}
		}

		if (Send && (c->ClientVersionBit() & ClientVersionBits)) {
			c->QueuePacket(app, ackreq);
		}
	}
}

void EntityList::QueueClientsByXTarget(Mob *sender, const EQApplicationPacket *app, bool iSendToSender, EQ::versions::ClientVersionBitmask client_version_bits)
{
	auto it = client_list.begin();
	while (it != client_list.end()) {
		Client *c = it->second;
		++it;

		if (!c || ((c == sender) && !iSendToSender))
			continue;

		if ((c->ClientVersionBit() & client_version_bits) == 0)
			continue;

		if (!c->IsXTarget(sender))
			continue;

		c->QueuePacket(app);
	}
}

void EntityList::QueueCloseClients(
	Mob *sender,
	const EQApplicationPacket *app,
	bool ignore_sender,
	float distance,
	Mob *skipped_mob,
	bool is_ack_required,
	eqFilterType filter
)
{
	if (sender == nullptr) {
		QueueClients(sender, app, ignore_sender);
		return;
	}

	if (distance <= 0) {
		distance = zone->GetClientUpdateRange();
	}

	float distance_squared = distance * distance;
	for (auto &e : sender->GetCloseMobList(distance)) {
		Mob *mob = e.second;
		if (!mob) {
			continue;
		}

		if (!mob->IsClient()) {
			continue;
		}

		Client *client = mob->CastToClient();

		if ((!ignore_sender || client != sender) && (client != skipped_mob)) {

			if (DistanceSquared(client->GetPosition(), sender->GetPosition()) >= distance_squared) {
				continue;
			}

			if (!client->Connected()) {
				continue;
			}

			eqFilterMode client_filter = client->GetFilter(filter);
			if (
				filter == FilterNone || client_filter == FilterShow ||
				(client_filter == FilterShowGroupOnly &&
				 (sender == client || (client->GetGroup() && client->GetGroup()->IsGroupMember(sender)))) ||
				(client_filter == FilterShowSelfOnly && client == sender)
				) {
				client->QueuePacket(app, is_ack_required, Client::CLIENT_CONNECTED);
			}
		}
	}
}

//sender can be null
void EntityList::QueueClients(
	Mob *sender, const EQApplicationPacket *app,
	bool ignore_sender, bool ackreq
)
{
	auto it = client_list.begin();
	while (it != client_list.end()) {
		Client *ent = it->second;

		if ((!ignore_sender || ent != sender))
			ent->QueuePacket(app, ackreq, Client::CLIENT_CONNECTED);

		++it;
	}
}

void EntityList::QueueClientsStatus(Mob *sender, const EQApplicationPacket *app,
		bool ignore_sender, uint8 minstatus, uint8 maxstatus)
{
	auto it = client_list.begin();
	while (it != client_list.end()) {
		if ((!ignore_sender || it->second != sender) &&
				(it->second->Admin() >= minstatus && it->second->Admin() <= maxstatus))
			it->second->QueuePacket(app);

		++it;
	}
}

void EntityList::DuelMessage(Mob *winner, Mob *loser, bool flee)
{
	if (winner->GetLevelCon(winner->GetLevel(), loser->GetLevel()) > 2) {
		if (parse->PlayerHasQuestSub(EVENT_DUEL_WIN)) {
			std::vector<std::any> args = { winner, loser };

			parse->EventPlayer(EVENT_DUEL_WIN, winner->CastToClient(), loser->GetName(), loser->CastToClient()->CharacterID(), &args);
		}

		if (parse->PlayerHasQuestSub(EVENT_DUEL_LOSE)) {
			std::vector<std::any> args = { winner, loser };

			parse->EventPlayer(EVENT_DUEL_LOSE, loser->CastToClient(), winner->GetName(), winner->CastToClient()->CharacterID(), &args);
		}
	}

	auto it = client_list.begin();
	while (it != client_list.end()) {
		Client *cur = it->second;
		//might want some sort of distance check in here?
		if (cur != winner && cur != loser) {
			if (flee)
				cur->MessageString(Chat::Yellow, DUEL_FLED, winner->GetName(),loser->GetName(),loser->GetName());
			else
				cur->MessageString(Chat::Yellow, DUEL_FINISHED, winner->GetName(),loser->GetName());
		}
		++it;
	}
}

Client *EntityList::GetClientByName(const char* name)
{
	for (const auto& e : client_list) {
		if (e.second && Strings::EqualFold(e.second->GetName(), name)) {
			return e.second;
		}
	}

	return nullptr;
}

Client *EntityList::GetClientByCharID(uint32 iCharID)
{
	auto it = client_list.begin();
	while (it != client_list.end()) {
		if (it->second->CharacterID() == iCharID)
			return it->second;
		++it;
	}
	return nullptr;
}

Client *EntityList::GetClientByWID(uint32 iWID)
{
	auto it = client_list.begin();
	while (it != client_list.end()) {
		if (it->second->GetWID() == iWID) {
			return it->second;
		}
		++it;
	}
	return nullptr;
}

Client *EntityList::GetClientByLSID(uint32 iLSID)
{
	auto it = client_list.begin();
	while (it != client_list.end()) {
		if (it->second->LSAccountID() == iLSID) {
			return it->second;
		}
		++it;
	}
	return nullptr;
}

Bot* EntityList::GetRandomBot(const glm::vec3& location, float distance, Bot* exclude_bot)
{
	auto is_whole_zone = false;
	if (location.x == 0.0f && location.y == 0.0f) {
		is_whole_zone = true;
	}

	auto distance_squared = (distance * distance);

	std::vector<Bot*> bots_in_range;

	for (const auto& b : bot_list) {
		if (
			b.second != exclude_bot &&
			(
				is_whole_zone ||
				DistanceSquared(static_cast<glm::vec3>(b.second->GetPosition()), location) <= distance_squared
				)
		) {
			bots_in_range.push_back(b.second);
		}
	}

	if (bots_in_range.empty()) {
		return nullptr;
	}

	return bots_in_range[zone->random.Int(0, bots_in_range.size() - 1)];

}

Client *EntityList::GetRandomClient(const glm::vec3& location, float distance, Client *exclude_client)
{
	auto is_whole_zone = false;
	if (location.x == 0.0f && location.y == 0.0f) {
		is_whole_zone = true;
	}

	auto distance_squared = (distance * distance);

	std::vector<Client*> clients_in_range;

	for (const auto& client : client_list) {
		if (
			client.second != exclude_client &&
			(
				is_whole_zone ||
				DistanceSquared(static_cast<glm::vec3>(client.second->GetPosition()), location) <= distance_squared
			)
		) {
			clients_in_range.push_back(client.second);
		}
	}

	if (clients_in_range.empty()) {
		return nullptr;
	}

	return clients_in_range[zone->random.Int(0, clients_in_range.size() - 1)];
}

NPC* EntityList::GetRandomNPC(const glm::vec3& location, float distance, NPC* exclude_npc)
{
	auto is_whole_zone = false;
	if (location.x == 0.0f && location.y == 0.0f) {
		is_whole_zone = true;
	}

	auto distance_squared = (distance * distance);

	std::vector<NPC*> npcs_in_range;

	for (const auto& npc : npc_list) {
		if (
			npc.second != exclude_npc &&
			(
				is_whole_zone ||
				DistanceSquared(static_cast<glm::vec3>(npc.second->GetPosition()), location) <= distance_squared
			)
		) {
			npcs_in_range.push_back(npc.second);
		}
	}

	if (npcs_in_range.empty()) {
		return nullptr;
	}

	return npcs_in_range[zone->random.Int(0, npcs_in_range.size() - 1)];
}

Mob* EntityList::GetRandomMob(const glm::vec3& location, float distance, Mob* exclude_mob)
{
	auto is_whole_zone = false;
	if (location.x == 0.0f && location.y == 0.0f) {
		is_whole_zone = true;
	}

	auto distance_squared = (distance * distance);

	std::vector<Mob*> mobs_in_range;

	for (const auto& mob : mob_list) {
		if (
			mob.second != exclude_mob &&
			(
				is_whole_zone ||
				DistanceSquared(static_cast<glm::vec3>(mob.second->GetPosition()), location) <= distance_squared
			)
		) {
			mobs_in_range.push_back(mob.second);
		}
	}

	if (mobs_in_range.empty()) {
		return nullptr;
	}

	return mobs_in_range[zone->random.Int(0, mobs_in_range.size() - 1)];
}

Corpse *EntityList::GetCorpseByOwner(Client *client)
{
	auto it = corpse_list.begin();
	while (it != corpse_list.end()) {
		if (it->second->IsPlayerCorpse())
			if (strcasecmp(it->second->GetOwnerName(), client->GetName()) == 0)
				return it->second;
		++it;
	}
	return nullptr;
}

Corpse *EntityList::GetCorpseByOwnerWithinRange(Client *client, Mob *center, int range)
{
	auto it = corpse_list.begin();
	while (it != corpse_list.end()) {
		if (it->second->IsPlayerCorpse())
			if (DistanceSquaredNoZ(center->GetPosition(), it->second->GetPosition()) < range &&
					strcasecmp(it->second->GetOwnerName(), client->GetName()) == 0)
				return it->second;
		++it;
	}
	return nullptr;
}

Corpse *EntityList::GetCorpseByDBID(uint32 dbid)
{
	auto it = corpse_list.begin();
	while (it != corpse_list.end()) {
		if (it->second->GetCorpseDBID() == dbid)
			return it->second;
		++it;
	}
	return nullptr;
}

Corpse *EntityList::GetCorpseByName(const char *name)
{
	auto it = corpse_list.begin();
	while (it != corpse_list.end()) {
		if (strcmp(it->second->GetName(), name) == 0)
			return it->second;
		++it;
	}
	return nullptr;
}

Spawn2 *EntityList::GetSpawnByID(uint32 id)
{
	if (!zone || !zone->IsLoaded())
		return nullptr;

	LinkedListIterator<Spawn2 *> iterator(zone->spawn2_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(iterator.GetData()->GetID() == id) {
			return iterator.GetData();
		}
		iterator.Advance();
	}

	return nullptr;
}

void EntityList::RemoveAllCorpsesByCharID(uint32 charid)
{
	auto it = corpse_list.begin();
	while (it != corpse_list.end()) {
		if (it->second->GetCharID() == charid) {
			safe_delete(it->second);
			free_ids.push(it->first);
			it = corpse_list.erase(it);
		} else {
			++it;
		}
	}
}

void EntityList::RemoveCorpseByDBID(uint32 dbid)
{
	auto it = corpse_list.begin();
	while (it != corpse_list.end()) {
		if (it->second->GetCorpseDBID() == dbid) {
			safe_delete(it->second);
			free_ids.push(it->first);
			it = corpse_list.erase(it);
		} else {
			++it;
		}
	}
}

int EntityList::RezzAllCorpsesByCharID(uint32 charid)
{
	int RezzExp = 0;

	auto it = corpse_list.begin();
	while (it != corpse_list.end()) {
		if (it->second->GetCharID() == charid) {
			RezzExp += it->second->GetRezExp();
			it->second->IsRezzed(true);
			it->second->CompleteResurrection();
		}
		++it;
	}
	return RezzExp;
}

Group *EntityList::GetGroupByMob(Mob *mob)
{
	std::list<Group *>::iterator iterator;

	iterator = group_list.begin();

	while (iterator != group_list.end()) {
		if ((*iterator)->IsGroupMember(mob))
			return *iterator;
		++iterator;
	}
	return nullptr;
}

Group *EntityList::GetGroupByMobName(const char* name)
{
	for (const auto& g : group_list) {
		for (const auto& m : g->membername) {
			if (strcmp(m, name) == 0) {
				return g;
			}
		}
	}
	return nullptr;
}

Group *EntityList::GetGroupByLeaderName(const char *leader)
{
	std::list<Group *>::iterator iterator;

	iterator = group_list.begin();

	while (iterator != group_list.end()) {
		if (!strcmp((*iterator)->GetLeaderName().c_str(), leader))
			return *iterator;
		++iterator;
	}
	return nullptr;
}

Group *EntityList::GetGroupByID(uint32 group_id)
{
	std::list<Group *>::iterator iterator;

	iterator = group_list.begin();

	while (iterator != group_list.end()) {
		if ((*iterator)->GetID() == group_id)
			return *iterator;
		++iterator;
	}
	return nullptr;
}

bool EntityList::IsInSameGroupOrRaidGroup(Client *client1, Client *client2) {
	Group* group = entity_list.GetGroupByClient(client1);
	Raid* raid = entity_list.GetRaidByClient(client1);

	return (group && group->IsGroupMember(client2))
		   || (raid && raid->IsRaidMember(client2->GetName()) && raid->GetGroup(client1) == raid->GetGroup(client2));
}

Group *EntityList::GetGroupByClient(Client *client)
{
	std::list <Group *>::iterator iterator;

	iterator = group_list.begin();

	while (iterator != group_list.end()) {
		if ((*iterator)->IsGroupMember(client->CastToMob())) {
			return *iterator;
		}
		++iterator;
	}

	return nullptr;
}

Raid *EntityList::GetRaidByID(uint32 id)
{
	std::list<Raid *>::iterator iterator;

	iterator = raid_list.begin();

	while (iterator != raid_list.end()) {
		if ((*iterator)->GetID() == id)
			return *iterator;
		++iterator;
	}
	return nullptr;
}

Raid* EntityList::GetRaidByClient(Client* client)
{
	if (client->p_raid_instance) {
		return client->p_raid_instance;
	}

	std::list<Raid*>::iterator iterator;
	iterator = raid_list.begin();

	while (iterator != raid_list.end()) {
		for (const auto& member : (*iterator)->members) {
			if (member.member && member.member == client) {
				client->p_raid_instance = *iterator;
				return *iterator;
			}
		}

		++iterator;
	}

	return nullptr;
}
Raid* EntityList::GetRaidByBotName(const char* name)
{
	for (const auto& r : raid_list) {
		for (const auto& m : r->members) {
			if (m.is_bot && strcmp(m.member_name, name) == 0) {
				return r;
			}
		}
	}

	return nullptr;
}

Raid* EntityList::GetRaidByBot(Bot* bot)
{
	if (bot->p_raid_instance) {
		return bot->p_raid_instance;
	}

	std::list<Raid*>::iterator iterator;
	iterator = raid_list.begin();

	while (iterator != raid_list.end()) {
		for (const auto& member : (*iterator)->members) {
			if (member.member && member.is_bot && member.member->CastToBot() == bot) {
				bot->p_raid_instance = *iterator;
				return *iterator;
			}
		}

		++iterator;
	}

	return nullptr;
}

Raid* EntityList::GetRaidByName(const char* name)
{
	for (const auto& r : raid_list) {
		for (const auto& m : r->members) {
			if (Strings::EqualFold(m.member_name, name)) {
				return r;
			}
		}
	}
	return nullptr;
}

Client *EntityList::GetClientByAccID(uint32 accid)
{
	auto it = client_list.begin();
	while (it != client_list.end()) {
		if (it->second->AccountID() == accid)
			return it->second;
		++it;
	}
	return nullptr;
}

void EntityList::ChannelMessageFromWorld(const char *from, const char *to,
		uint8 chan_num, uint32 guild_id, uint8 language, uint8 lang_skill, const char *message)
{
	for (auto it = client_list.begin(); it != client_list.end(); ++it) {
		Client *client = it->second;
		if (chan_num == ChatChannel_Guild) {
			if (!client->IsInGuild(guild_id))
				continue;
			if (client->ClientVersion() >= EQ::versions::ClientVersion::RoF) {
				if (!guild_mgr.CheckPermission(guild_id, client->GuildRank(), GUILD_ACTION_GUILD_CHAT_SEE))
					continue;
			}
			if (client->GetFilter(FilterGuildChat) == FilterHide)
				continue;
		} else if (chan_num == ChatChannel_OOC) {
			if (client->GetFilter(FilterOOC) == FilterHide)
				continue;
		}
		client->ChannelMessageSend(from, to, chan_num, language, lang_skill, message);
	}
}

void EntityList::Message(uint32 to_guilddbid, uint32 type, const char *message, ...)
{
	va_list argptr;
	char buffer[4096];

	va_start(argptr, message);
	vsnprintf(buffer, 4096, message, argptr);
	va_end(argptr);

	auto it = client_list.begin();
	while (it != client_list.end()) {
		Client *client = it->second;
		if (to_guilddbid == 0 || client->IsInGuild(to_guilddbid))
			client->Message(type, buffer);
		++it;
	}
}

void EntityList::QueueClientsGuild(const EQApplicationPacket *app, uint32 guild_id)
{
	auto it = client_list.begin();
	while (it != client_list.end()) {
		Client *client = it->second;
		if (client->IsInGuild(guild_id))
			client->QueuePacket(app, true);
		++it;
	}
}

void EntityList::QueueClientsGuildBankItemUpdate(GuildBankItemUpdate_Struct *gbius, uint32 guild_id)
{
	auto outapp = std::make_unique<EQApplicationPacket>(OP_GuildBank, sizeof(GuildBankItemUpdate_Struct));
	auto data = reinterpret_cast<GuildBankItemUpdate_Struct *>(outapp->pBuffer);

	memcpy(data, gbius, sizeof(GuildBankItemUpdate_Struct));

	for (auto const &[key, client]: client_list) {
		if (client->IsInGuild(guild_id)) {
			client->QueuePacket(outapp.get());
		}
	}
}

void EntityList::MessageStatus(uint32 to_guild_id, int to_minstatus, uint32 type, const char *message, ...)
{
	va_list argptr;
	char    buffer[4096];

	va_start(argptr, message);
	vsnprintf(buffer, 4096, message, argptr);
	va_end(argptr);

	auto it = client_list.begin();
	while (it != client_list.end()) {
		Client *client = it->second;
		if ((to_guild_id == 0 || client->IsInGuild(to_guild_id)) && client->Admin() >= to_minstatus) {
			client->Message(type, buffer);
		}
		++it;
	}
}

/**
 * @param sender
 * @param skipsender
 * @param dist
 * @param type
 * @param string_id
 * @param message1
 * @param message2
 * @param message3
 * @param message4
 * @param message5
 * @param message6
 * @param message7
 * @param message8
 * @param message9
 */
void EntityList::MessageCloseString(
	Mob *sender,
	bool skipsender,
	float dist,
	uint32 type,
	uint32 string_id,
	const char *message1,
	const char *message2,
	const char *message3,
	const char *message4,
	const char *message5,
	const char *message6,
	const char *message7,
	const char *message8,
	const char *message9
)
{
	Client *c;
	float  dist2 = dist * dist;

	for (auto & it : client_list) {
		c = it.second;
		if (c && DistanceSquared(c->GetPosition(), sender->GetPosition()) <= dist2 && (!skipsender || c != sender)) {
			c->MessageString(
				type,
				string_id,
				message1,
				message2,
				message3,
				message4,
				message5,
				message6,
				message7,
				message8,
				message9
			);
		}
	}
}

/**
 * @param sender
 * @param skipsender
 * @param dist
 * @param type
 * @param filter
 * @param string_id
 * @param message1
 * @param message2
 * @param message3
 * @param message4
 * @param message5
 * @param message6
 * @param message7
 * @param message8
 * @param message9
 */
void EntityList::FilteredMessageCloseString(
	Mob *sender, bool skipsender,
	float dist,
	uint32 type,
	eqFilterType filter,
	uint32 string_id,
	Mob *skip,
	const char *message1,
	const char *message2,
	const char *message3,
	const char *message4,
	const char *message5,
	const char *message6,
	const char *message7,
	const char *message8,
	const char *message9
)
{
	Client *c;
	float  dist2 = dist * dist;

	for (auto & it : client_list) {
		c = it.second;
		if (c && c != skip && DistanceSquared(c->GetPosition(), sender->GetPosition()) <= dist2 && (!skipsender || c != sender)) {
			c->FilteredMessageString(
				sender, type, filter, string_id,
				message1, message2, message3, message4, message5,
				message6, message7, message8, message9
			);
		}
	}
}

/**
 *
 * @param sender
 * @param skipsender
 * @param type
 * @param string_id
 * @param message1
 * @param message2
 * @param message3
 * @param message4
 * @param message5
 * @param message6
 * @param message7
 * @param message8
 * @param message9
 */
void EntityList::MessageString(
	Mob *sender,
	bool skipsender,
	uint32 type,
	uint32 string_id,
	const char *message1,
	const char *message2,
	const char *message3,
	const char *message4,
	const char *message5,
	const char *message6,
	const char *message7,
	const char *message8,
	const char *message9
)
{
	Client *c;

	for (auto & it : client_list) {
		c = it.second;
		if (c && (!skipsender || c != sender)) {
			c->MessageString(
				type,
				string_id,
				message1,
				message2,
				message3,
				message4,
				message5,
				message6,
				message7,
				message8,
				message9
			);
		}
	}
}

/**
 *
 * @param sender
 * @param skipsender
 * @param type
 * @param filter
 * @param string_id
 * @param message1
 * @param message2
 * @param message3
 * @param message4
 * @param message5
 * @param message6
 * @param message7
 * @param message8
 * @param message9
 */
void EntityList::FilteredMessageString(
	Mob *sender,
	bool skipsender,
	uint32 type,
	eqFilterType filter,
	uint32 string_id,
	const char *message1,
	const char *message2,
	const char *message3,
	const char *message4,
	const char *message5,
	const char *message6,
	const char *message7,
	const char *message8,
	const char *message9
)
{
	Client *c;

	for (auto & it : client_list) {
		c = it.second;
		if (c && (!skipsender || c != sender)) {
			c->FilteredMessageString(
				sender,
				type,
				filter,
				string_id,
				message1,
				message2,
				message3,
				message4,
				message5,
				message6,
				message7,
				message8,
				message9
			);
		}
	}
}

/**
 * @param sender
 * @param skipsender
 * @param dist
 * @param type
 * @param message
 * @param ...
 */
void EntityList::MessageClose(Mob *sender, bool skipsender, float dist, uint32 type, const char *message, ...)
{
	va_list argptr;
	char    buffer[4096];

	va_start(argptr, message);
	vsnprintf(buffer, 4095, message, argptr);
	va_end(argptr);

	float dist2 = dist * dist;

	auto it = client_list.begin();
	while (it != client_list.end()) {
		if (DistanceSquared(it->second->GetPosition(), sender->GetPosition()) <= dist2 &&
			(!skipsender || it->second != sender)) {
			it->second->Message(type, buffer);
		}
		++it;
	}
}

void EntityList::FilteredMessageClose(
	Mob *sender,
	bool skipsender,
	float dist,
	uint32 type,
	eqFilterType filter,
	const char *message,
	...
)
{
	va_list argptr;
	char    buffer[4096];

	va_start(argptr, message);
	vsnprintf(buffer, 4095, message, argptr);
	va_end(argptr);

	float dist2 = dist * dist;

	auto it = client_list.begin();
	while (it != client_list.end()) {
		if (DistanceSquared(it->second->GetPosition(), sender->GetPosition()) <= dist2 &&
			(!skipsender || it->second != sender)) {
				it->second->FilteredMessage(sender, type, filter, buffer);
		}
		++it;
	}
}

void EntityList::RemoveAllMobs()
{
	auto it = mob_list.begin();
	while (it != mob_list.end()) {
		if (!it->second) {
			++it;
			continue;
		}
		safe_delete(it->second);
		free_ids.push(it->first);
		it = mob_list.erase(it);
	}
}

void EntityList::RemoveAllClients()
{
	// doesn't clear the data
	client_list.clear();
}

void EntityList::RemoveAllNPCs()
{
	// doesn't clear the data
	npc_list.clear();
	npc_limit_list.clear();
}

void EntityList::RemoveAllBots()
{
	// doesn't clear the data
	bot_list.clear();
}

void EntityList::RemoveAllMercs()
{
	// doesn't clear the data
	merc_list.clear();
}

void EntityList::RemoveAllGroups()
{
	while (group_list.size()) {
		auto group = group_list.front();
		group_list.pop_front();
		safe_delete(group);
	}
}

void EntityList::RemoveAllRaids()
{
	while (raid_list.size()) {
		auto raid = raid_list.front();
		raid_list.pop_front();
		safe_delete(raid);
	}
}

void EntityList::RemoveAllDoors()
{
	auto it = door_list.begin();
	while (it != door_list.end()) {
		safe_delete(it->second);
		free_ids.push(it->first);
		it = door_list.erase(it);
	}
	DespawnAllDoors();
}

void EntityList::DespawnAllDoors()
{
	auto outapp = new EQApplicationPacket(OP_RemoveAllDoors, 0);
	for (auto it = client_list.begin(); it != client_list.end(); ++it) {
		if (it->second) {
			it->second->QueuePacket(outapp, true, Client::CLIENT_CONNECTED);
		}
	}
	safe_delete(outapp);
}

void EntityList::RespawnAllDoors()
{
	auto it = client_list.begin();
	while (it != client_list.end()) {
		if (it->second) {
			auto outapp = new EQApplicationPacket();
			MakeDoorSpawnPacket(outapp, it->second);
			it->second->FastQueuePacket(&outapp, true, Client::CLIENT_CONNECTED);
		}
		++it;
	}
}

void EntityList::RemoveAllCorpses()
{
	auto it = corpse_list.begin();
	while (it != corpse_list.end()) {
		safe_delete(it->second);
		free_ids.push(it->first);
		it = corpse_list.erase(it);
	}
}

void EntityList::RemoveAllObjects()
{
	auto it = object_list.begin();
	while (it != object_list.end()) {
		safe_delete(it->second);
		free_ids.push(it->first);
		it = object_list.erase(it);
	}
}

void EntityList::RemoveAllTraps()
{
	auto it = trap_list.begin();
	while (it != trap_list.end()) {
		safe_delete(it->second);
		free_ids.push(it->first);
		it = trap_list.erase(it);
	}
}

void EntityList::RemoveAllEncounters()
{
	auto it = encounter_list.begin();
	while (it != encounter_list.end()) {
		parse->RemoveEncounter(it->second->GetEncounterName());
		safe_delete(it->second);
		free_ids.push(it->first);
		it = encounter_list.erase(it);
	}
}

/**
 * @param delete_id
 * @return
 */
bool EntityList::RemoveMob(uint16 delete_id)
{
	if (delete_id == 0) {
		return true;
	}

	auto it = mob_list.find(delete_id);
	if (it != mob_list.end()) {
		if (!it->second) {
			return false;
		}
		if (npc_list.count(delete_id)) {
			entity_list.RemoveNPC(delete_id);
		}
		else if (client_list.count(delete_id)) {
			entity_list.RemoveClient(delete_id);
		}
		safe_delete(it->second);
		if (!corpse_list.count(delete_id)) {
			free_ids.push(it->first);
		}
		mob_list.erase(it);
		return true;
	}
	return false;
}

/**
 * @param delete_id
 * @return
 */
bool EntityList::RemoveNPC(uint16 delete_id)
{
	auto it = npc_list.find(delete_id);
	if (it != npc_list.end()) {
		NPC *npc = it->second;
		RemoveProximity(delete_id);
		npc_list.erase(it);

		if (npc_limit_list.count(delete_id)) {
			npc_limit_list.erase(delete_id);
		}

		return true;
	}
	return false;
}

/**
 * @param mob
 * @return
 */
bool EntityList::RemoveMobFromCloseLists(Mob *mob)
{
	uint16 entity_id = mob->GetID() > 0 ? mob->GetID() : mob->GetInitialId();

	LogEntityManagement(
		"Attempting to remove mob [{}] from close lists entity_id ({})",
		mob->GetCleanName(),
		entity_id
	);

	auto it = mob_list.begin();
	while (it != mob_list.end()) {
		LogEntityManagement(
			"Removing mob [{}] from [{}] close list entity_id ({})",
			mob->GetCleanName(),
			it->second->GetCleanName(),
			entity_id
		);

		it->second->m_close_mobs.erase(entity_id);
		it->second->m_last_seen_mob_position.erase(entity_id);

		++it;
	}

	return false;
}

/**
 * @param mob
 * @return
 */
void EntityList::RemoveAuraFromMobs(Mob *aura)
{
	LogEntityManagement(
		"Attempting to remove aura [{}] from mobs entity_id ({})",
		aura->GetCleanName(),
		aura->GetID()
	);

	for (auto &it : mob_list) {
		auto mob = it.second;
		mob->RemoveAura(aura->GetID());
	}
}

// The purpose of this system is so that we cache relevant entities that are "close"
//
// In general; it becomes incredibly expensive to run zone-wide checks against every single mob in the zone when in reality
// we only care about entities closest to us
//
// A very simple example of where this is relevant is Aggro, the below example is skewed because the overall implementation
// of Aggro was also tweaked in conjunction with close lists. We also scan more aggressively when entities are moving (1-6 seconds)
// versus 60 seconds when idle. We also have entities that are moving add themselves to those closest to them so that their close
// lists remain always up to date
//
// Before: Aggro checks for NPC to Client aggro | (40 clients in zone) x (525 npcs) x 2 (times a second) = 2,520,000 checks a minute
// After: Aggro checks for NPC to Client aggro | (40 clients in zone) x (20-30 npcs) x 2 (times a second) = 144,000 checks a minute (This is // tually far less today)
//
// Places in the code where this logic makes a huge impact
//
// Aggro checks (zone wide -> close)
// Aura processing (zone wide -> close)
// AE Taunt (zone wide -> close)
// AOE Spells (zone wide -> close)
// Bard Pulse AOE (zone wide -> close)
// Mass Group Buff (zone wide -> close)
// AE Attack (zone wide -> close)
// Packet QueueCloseClients (zone wide -> close)
// Check Close Beneficial Spells (Buffs; should I heal other npcs) (zone wide -> close)
// AI Yell for Help (NPC Assist other NPCs) (zone wide -> close)
//
// All of the above makes a tremendous impact on the bottom line of cpu cycle performance because we run an order of magnitude
// less checks by focusing our hot path logic down to a very small subset of relevant entities instead of looping an entire
// entity list (zone wide)

BenchTimer g_scan_bench_timer;

void EntityList::ScanCloseMobs(Mob *scanning_mob)
{
	if (!scanning_mob) {
		return;
	}

	if (scanning_mob->GetID() <= 0 || scanning_mob->IsZoneController()) {
		return;
	}

	g_scan_bench_timer.reset();

	float scan_range = RuleI(Range, MobCloseScanDistance);

	// Reserve memory in m_close_mobs to avoid frequent re-allocations if not already reserved.
	// Assuming mob_list.size() as an upper bound for reservation.
	if (scanning_mob->m_close_mobs.bucket_count() < mob_list.size()) {
		scanning_mob->m_close_mobs.reserve(mob_list.size());
	}

	scanning_mob->m_close_mobs.clear();

	for (auto &e : mob_list) {
		auto mob = e.second;

		if (mob && (mob->GetID() <= 0 || mob->IsZoneController())) {
			continue;
		}

		float distance = Distance(scanning_mob->GetPosition(), mob->GetPosition());
		if (distance <= scan_range || mob->GetAggroRange() >= scan_range) {
			// add mob to scanning_mob's close list and vice versa
			// check if the mob is already in the close mobs list before inserting
			if (mob->m_close_mobs.find(scanning_mob->GetID()) == mob->m_close_mobs.end()) {
				mob->m_close_mobs[scanning_mob->GetID()] = scanning_mob;
			}
			scanning_mob->m_close_mobs[mob->GetID()] = mob;
		}
	}

	LogAIScanClose(
		"[{}] Scanning close list > list_size [{}] moving [{}] elapsed [{}] us",
		scanning_mob->GetCleanName(),
		scanning_mob->m_close_mobs.size(),
		scanning_mob->IsMoving() ? "true" : "false",
		g_scan_bench_timer.elapsedMicroseconds()
	);
}

bool EntityList::RemoveMerc(uint16 delete_id)
{
	auto it = merc_list.find(delete_id);
	if (it != merc_list.end()) {
		merc_list.erase(it); // Already Deleted
		return true;
	}
	return false;
}

bool EntityList::RemoveClient(uint16 delete_id)
{
	auto it = client_list.find(delete_id);
	if (it != client_list.end()) {
		client_list.erase(it); // Already deleted
		return true;
	}
	return false;
}

// If our ID was deleted already
bool EntityList::RemoveClient(Client *delete_client)
{
	auto it = client_list.begin();
	while (it != client_list.end()) {
		if (it->second == delete_client) {
			client_list.erase(it);
			return true;
		}
		++it;
	}
	return false;
}

bool EntityList::RemoveObject(uint16 delete_id)
{
	auto it = object_list.find(delete_id);
	if (it != object_list.end()) {
		safe_delete(it->second);
		free_ids.push(it->first);
		object_list.erase(it);
		return true;
	}
	return false;
}

bool EntityList::RemoveTrap(uint16 delete_id)
{
	auto it = trap_list.find(delete_id);
	if (it != trap_list.end()) {
		safe_delete(it->second);
		free_ids.push(it->first);
		trap_list.erase(it);
		return true;
	}
	return false;
}

bool EntityList::RemoveDoor(uint16 delete_id)
{
	auto it = door_list.find(delete_id);
	if (it != door_list.end()) {
		safe_delete(it->second);
		free_ids.push(it->first);
		door_list.erase(it);
		return true;
	}
	return false;
}

bool EntityList::RemoveCorpse(uint16 delete_id)
{
	auto it = corpse_list.find(delete_id);
	if (it != corpse_list.end()) {
		safe_delete(it->second);
		free_ids.push(it->first);
		corpse_list.erase(it);
		return true;
	}
	return false;
}

bool EntityList::RemoveGroup(uint32 delete_id)
{
	auto it = std::find_if(group_list.begin(), group_list.end(),
			[delete_id](const Group *a) { return a->GetID() == delete_id; });
	if (it == group_list.end()) {
#if EQDEBUG >= 5
		CheckGroupList (__FILE__, __LINE__);
#endif
		return false;
	}
	auto group = *it;
	group_list.erase(it);
	safe_delete(group);
	return true;
}

void EntityList::Clear()
{
	RemoveAllClients();
	entity_list.RemoveAllTraps(); //we can have child npcs so we go first
	entity_list.RemoveAllMercs();
	entity_list.RemoveAllBots();
	entity_list.RemoveAllNPCs();
	entity_list.RemoveAllMobs();
	entity_list.RemoveAllCorpses();
	entity_list.RemoveAllGroups();
	entity_list.RemoveAllDoors();
	entity_list.RemoveAllObjects();
	entity_list.RemoveAllRaids();
	entity_list.RemoveAllLocalities();
}

void EntityList::UpdateWho(bool iSendFullUpdate)
{
	if ((!worldserver.Connected()) || !is_zone_loaded)
		return;
	uint32 tmpNumUpdates = numclients + 5;
	ServerPacket* pack = 0;
	ServerClientListKeepAlive_Struct* sclka = 0;
	if (!iSendFullUpdate) {
		pack = new ServerPacket(ServerOP_ClientListKA, sizeof(ServerClientListKeepAlive_Struct) + (tmpNumUpdates * 4));
		sclka = (ServerClientListKeepAlive_Struct*) pack->pBuffer;
	}

	auto it = client_list.begin();
	while (it != client_list.end()) {
		if (it->second->InZone()) {
			if (iSendFullUpdate) {
				it->second->UpdateWho();
			} else {
				if (sclka->numupdates >= tmpNumUpdates) {
					tmpNumUpdates += 10;
					uint8* tmp = pack->pBuffer;
					pack->pBuffer = new uint8[sizeof(ServerClientListKeepAlive_Struct) + (tmpNumUpdates * 4)];
					memset(pack->pBuffer, 0, sizeof(ServerClientListKeepAlive_Struct) + (tmpNumUpdates * 4));
					memcpy(pack->pBuffer, tmp, pack->size);
					pack->size = sizeof(ServerClientListKeepAlive_Struct) + (tmpNumUpdates * 4);
					safe_delete_array(tmp);
					sclka = (ServerClientListKeepAlive_Struct*) pack->pBuffer;
				}
				sclka->wid[sclka->numupdates] = it->second->GetWID();
				sclka->numupdates++;
			}
		}
		++it;
	}
	if (!iSendFullUpdate) {
		pack->size = sizeof(ServerClientListKeepAlive_Struct) + (sclka->numupdates * 4);
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void EntityList::RemoveEntity(uint16 id)
{
	if (id == 0)
		return;
	if (entity_list.RemoveMob(id))
		return;
	else if (entity_list.RemoveCorpse(id))
		return;
	else if (entity_list.RemoveDoor(id))
		return;
	else if (entity_list.RemoveGroup(id))
		return;
	else if (entity_list.RemoveTrap(id))
		return;
	else if (entity_list.RemoveMerc(id))
		return;
	else if (entity_list.RemoveBot(id))
		return;
	else
		entity_list.RemoveObject(id);
}

void EntityList::Process()
{
	CheckSpawnQueue();
}

void EntityList::Depop(bool StartSpawnTimer)
{
	for (auto it = npc_list.begin(); it != npc_list.end(); ++it) {
		NPC *pnpc = it->second;

		if (pnpc) {
			Mob *own = pnpc->GetOwner();
			//do not depop player/bot pets...
			if (own && own->IsOfClientBot()) {
				continue;
			}

			if (pnpc->IsHorse()) {
				continue;
			}

			if (pnpc->IsFindable()) {
				UpdateFindableNPCState(pnpc, true);
			}

			// Depop below will eventually remove this npc from the entity list
			// but that can happen AFTER we've already tried to spawn its replacement.
			// So go ahead and remove it from the limits so it doesn't count.
			if (npc_limit_list.count(pnpc->GetID())) {
				npc_limit_list.erase(pnpc->GetID());
			}

			pnpc->WipeHateList();
			pnpc->Depop(StartSpawnTimer);
		}
	}
}

void EntityList::DepopAll(int NPCTypeID, bool StartSpawnTimer)
{
	for (auto it = npc_list.begin(); it != npc_list.end(); ++it) {
		NPC *pnpc = it->second;
		if (pnpc && (pnpc->GetNPCTypeID() == (uint32)NPCTypeID))
			pnpc->Depop(StartSpawnTimer);
	}
}

void EntityList::SendTraders(Client *client)
{
	Client *trader = nullptr;
	auto it = client_list.begin();
	while (it != client_list.end()) {
		trader = it->second;
		if (trader->IsTrader())
			client->SendTraderPacket(trader);

		if (trader->IsBuyer())
			client->SendBuyerPacket(trader);

		++it;
	}
}

void EntityList::RemoveFromHateLists(Mob *mob, bool settoone)
{
	auto it = npc_list.begin();
	while (it != npc_list.end()) {
		if (it->second->CheckAggro(mob)) {
			if (!settoone) {
				it->second->RemoveFromHateList(mob);
				it->second->RemoveFromRampageList(mob);
				if (mob->IsClient()) {
					mob->CastToClient()->RemoveXTarget(it->second, false); // gotta do book keeping
				}
			} else {
				it->second->SetHateAmountOnEnt(mob, 1);
			}
		}
		++it;
	}
}

void EntityList::RemoveDebuffs(Mob *caster)
{
	auto it = mob_list.begin();
	while (it != mob_list.end()) {
		it->second->BuffFadeDetrimentalByCaster(caster);
		++it;
	}
}

char *EntityList::MakeNameUnique(char *name)
{
	bool used[300];
	memset(used, 0, sizeof(used));
	name[61] = 0; name[62] = 0; name[63] = 0;


	int len = strlen(name);
	auto it = mob_list.begin();
	while (it != mob_list.end()) {
		if (it->second->IsMob()) {
			if (strncasecmp(it->second->CastToMob()->GetName(), name, len) == 0) {
				if (Seperator::IsNumber(&it->second->CastToMob()->GetName()[len])) {
					used[Strings::ToInt(&it->second->CastToMob()->GetName()[len])] = true;
				}
			}
		}
		++it;
	}
	for (int i=0; i < 300; i++) {
		if (!used[i]) {
			#ifdef _WINDOWS
			snprintf(name, 64, "%s%03d", name, i);
			#else
			//glibc clears destination of snprintf
			//make a copy of name before snprintf--misanthropicfiend
			char temp_name[64];
			strn0cpy(temp_name, name, 64);
			snprintf(name, 64, "%s%03d", temp_name, i);
			#endif
			return name;
		}
	}
	LogError("Fatal error in EntityList::MakeNameUnique: Unable to find unique name for [{}]", name);
	char tmp[64] = "!";
	strn0cpy(&tmp[1], name, sizeof(tmp) - 1);
	strcpy(name, tmp);
	return MakeNameUnique(name);
}

char *EntityList::RemoveNumbers(char *name)
{
	char tmp[64];
	memset(tmp, 0, sizeof(tmp));
	int k = 0;
	for (unsigned int i=0; i<strlen(name) && i<sizeof(tmp); i++) {
		if (name[i] < '0' || name[i] > '9')
			tmp[k++] = name[i];
	}
	strn0cpy(name, tmp, sizeof(tmp));
	return name;
}

void EntityList::ListNPCCorpses(Client *client)
{
	uint32 corpse_count = 0;
	for (const auto& corpse : corpse_list) {
		uint32 corpse_number = (corpse_count + 1);
		if (corpse.second->IsNPCCorpse()) {
			client->Message(
				Chat::White,
				fmt::format(
					"Corpse {} | Name: {} ({})",
					corpse_number,
					corpse.second->GetName(),
					corpse.second->GetID()
				).c_str()
			);
			corpse_count++;
		}
	}

	if (corpse_count > 0) {
		client->Message(
			Chat::White,
			fmt::format(
				"{} NPC corpses listed.",
				corpse_count
			).c_str()
		);
	}
}

void EntityList::ListPlayerCorpses(Client *client)
{
	uint32 corpse_count = 0;
	for (const auto& corpse : corpse_list) {
		uint32 corpse_number = (corpse_count + 1);
		if (corpse.second->IsPlayerCorpse()) {
			client->Message(
				Chat::White,
				fmt::format(
					"Corpse {} | Name: {} ({})",
					corpse_number,
					corpse.second->GetName(),
					corpse.second->GetID()
				).c_str()
			);
			corpse_count++;
		}
	}

	if (corpse_count > 0) {
		client->Message(
			Chat::White,
			fmt::format(
				"{} Player corpses listed.",
				corpse_count
			).c_str()
		);
	}
}

// returns the number of corpses deleted. A negative number indicates an error code.
uint32 EntityList::DeleteNPCCorpses()
{
	uint32 corpse_count = 0;
	for (const auto& corpse : corpse_list) {
		if (corpse.second->IsNPCCorpse()) {
			corpse.second->DepopNPCCorpse();
			corpse_count++;
		}
	}
	return corpse_count;
}

void EntityList::CorpseFix(Client* c)
{
	uint32 fixed_count = 0;
	for (const auto& e : corpse_list) {
		auto cur = e.second;
		if (cur->IsNPCCorpse()) {
			if (DistanceNoZ(c->GetPosition(), cur->GetPosition()) < 100) {
				c->Message(
					Chat::White,
					fmt::format(
						"Attempting to fix {}.",
						cur->GetCleanName()
					).c_str()
				);

				cur->GMMove(
					cur->GetX(),
					cur->GetY(),
					cur->GetFixedZ(c->GetPosition()),
					c->GetHeading()
				);

				fixed_count++;
			}
		}
	}

	if (!fixed_count) {
		c->Message(Chat::White, "There were no nearby NPC corpses to fix.");
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Fixed {} nearby NPC corpse{}.",
			fixed_count,
			fixed_count != 1 ? "s" : ""
		).c_str()
	);
}

// returns the number of corpses deleted. A negative number indicates an error code.
uint32 EntityList::DeletePlayerCorpses()
{
	uint32 corpse_count = 0;
	for (const auto& corpse : corpse_list) {
		if (corpse.second->IsPlayerCorpse()) {
			corpse.second->Delete();
			corpse_count++;
		}
	}
	return corpse_count;
}

void EntityList::SendPetitionToAdmins()
{
	auto outapp = new EQApplicationPacket(OP_PetitionUpdate, sizeof(PetitionUpdate_Struct));
	PetitionUpdate_Struct *pcus = (PetitionUpdate_Struct*) outapp->pBuffer;
	pcus->petnumber = 0;		// Petition Number
	pcus->color = 0;
	pcus->status = 0xFFFFFFFF;
	pcus->senttime = 0;
	strcpy(pcus->accountid, "");
	strcpy(pcus->gmsenttoo, "");
	pcus->quetotal=0;
	auto it = client_list.begin();
	while (it != client_list.end()) {
		if (it->second->CastToClient()->Admin() >= AccountStatus::QuestTroupe)
			it->second->CastToClient()->QueuePacket(outapp);
		++it;
	}
	safe_delete(outapp);
}

void EntityList::SendPetitionToAdmins(Petition *pet)
{
	auto outapp = new EQApplicationPacket(OP_PetitionUpdate, sizeof(PetitionUpdate_Struct));
	PetitionUpdate_Struct *pcus = (PetitionUpdate_Struct*) outapp->pBuffer;
	pcus->petnumber = pet->GetID();		// Petition Number
	if (pet->CheckedOut()) {
		pcus->color = 0x00;
		pcus->status = 0xFFFFFFFF;
		pcus->senttime = pet->GetSentTime();
		strcpy(pcus->accountid, "");
		strcpy(pcus->gmsenttoo, "");
	} else {
		pcus->color = pet->GetUrgency();	// 0x00 = green, 0x01 = yellow, 0x02 = red
		pcus->status = pet->GetSentTime();
		pcus->senttime = pet->GetSentTime();			// 4 has to be 0x1F
		strcpy(pcus->accountid, pet->GetAccountName());
		strcpy(pcus->charname, pet->GetCharName());
	}
	pcus->quetotal = PetitionList::Instance()->GetTotalPetitions();
	auto it = client_list.begin();
	while (it != client_list.end()) {
		if (it->second->CastToClient()->Admin() >= AccountStatus::QuestTroupe) {
			if (pet->CheckedOut())
				strcpy(pcus->gmsenttoo, "");
			else
				strcpy(pcus->gmsenttoo, it->second->CastToClient()->GetName());
			it->second->CastToClient()->QueuePacket(outapp);
		}
		++it;
	}
	safe_delete(outapp);
}

void EntityList::ClearClientPetitionQueue()
{
	auto outapp = new EQApplicationPacket(OP_PetitionUpdate, sizeof(PetitionUpdate_Struct));
	PetitionUpdate_Struct *pet = (PetitionUpdate_Struct*) outapp->pBuffer;
	pet->color = 0x00;
	pet->status = 0xFFFFFFFF;
	pet->senttime = 0;
	strcpy(pet->accountid, "");
	strcpy(pet->gmsenttoo, "");
	strcpy(pet->charname, "");
	pet->quetotal = PetitionList::Instance()->GetTotalPetitions();
	auto it = client_list.begin();
	while (it != client_list.end()) {
		if (it->second->CastToClient()->Admin() >= AccountStatus::GMAdmin) {
			int x = 0;
			for (x = 0; x < 64; x++) {
				pet->petnumber = x;
				it->second->CastToClient()->QueuePacket(outapp);
			}
		}
		++it;
	}
	safe_delete(outapp);
	return;
}

BulkZoneSpawnPacket::BulkZoneSpawnPacket(Client *iSendTo, uint32 iMaxSpawnsPerPacket)
{
	data = nullptr;
	pSendTo = iSendTo;
	pMaxSpawnsPerPacket = iMaxSpawnsPerPacket;
}

BulkZoneSpawnPacket::~BulkZoneSpawnPacket()
{
	SendBuffer();
	safe_delete_array(data);
}

bool BulkZoneSpawnPacket::AddSpawn(NewSpawn_Struct *ns)
{
	if (!data) {
		data = new NewSpawn_Struct[pMaxSpawnsPerPacket];
		memset(data, 0, sizeof(NewSpawn_Struct) * pMaxSpawnsPerPacket);
		index = 0;
	}
	memcpy(&data[index], ns, sizeof(NewSpawn_Struct));
	index++;
	if (index >= pMaxSpawnsPerPacket) {
		SendBuffer();
		return true;
	}
	return false;
}

void BulkZoneSpawnPacket::SendBuffer()
{
	if (!data)
		return;

	uint32 tmpBufSize = (index * sizeof(NewSpawn_Struct));
	auto outapp = new EQApplicationPacket(OP_ZoneSpawns, (unsigned char *)data, tmpBufSize);

	if (pSendTo) {
		pSendTo->FastQueuePacket(&outapp);
	} else {
		entity_list.QueueClients(0, outapp);
		safe_delete(outapp);
	}
	memset(data, 0, sizeof(NewSpawn_Struct) * pMaxSpawnsPerPacket);
	index = 0;
}

void EntityList::DoubleAggro(Mob *who)
{
	auto it = npc_list.begin();
	while (it != npc_list.end()) {
		if (it->second->CheckAggro(who))
			it->second->SetHateAmountOnEnt(who, it->second->CastToNPC()->GetHateAmount(who),
					it->second->CastToNPC()->GetHateAmount(who) * 2);
		++it;
	}
}

void EntityList::HalveAggro(Mob *who)
{
	auto it = npc_list.begin();
	while (it != npc_list.end()) {
		if (it->second->CastToNPC()->CheckAggro(who))
			it->second->CastToNPC()->SetHateAmountOnEnt(who, it->second->CastToNPC()->GetHateAmount(who) / 2);
		++it;
	}
}

//removes "targ" from all hate lists, including feigned, in the zone
void EntityList::ClearAggro(Mob* targ, bool clear_caster_id)
{
	Client *c = nullptr;

	if (targ->IsClient()) {
		c = targ->CastToClient();
	}

	auto it = npc_list.begin();
	while (it != npc_list.end()) {
		if (clear_caster_id) {
			it->second->BuffDetachCaster(targ);
		}

		if (it->second->CheckAggro(targ)) {
			if (c) {
				c->RemoveXTarget(it->second, false);
			}

			it->second->RemoveFromHateList(targ);
			it->second->RemoveFromRampageList(targ, true);
		}

		if (c && it->second->IsOnFeignMemory(c)) {
			it->second->RemoveFromFeignMemory(c); //just in case we feigned
			c->RemoveXTarget(it->second, false);
		}
		++it;
	}
}

//removes "targ" from all hate lists of mobs that are water only.
void EntityList::ClearWaterAggro(Mob* targ)
{
	Client *c = nullptr;
	if (targ->IsClient()) {
		c = targ->CastToClient();
	}
	auto it = npc_list.begin();
	while (it != npc_list.end()) {
		if (it->second->IsUnderwaterOnly()) {
			if (it->second->CheckAggro(targ)) {
				if (c) {
					c->RemoveXTarget(it->second, false);
				}
				it->second->RemoveFromHateList(targ);
				it->second->RemoveFromRampageList(targ);
			}
			if (c && it->second->IsOnFeignMemory(c)) {
				it->second->RemoveFromFeignMemory(c); //just in case we feigned
				c->RemoveXTarget(it->second, false);
			}
		}
	++it;
	}
}


void EntityList::ClearFeignAggro(Mob *targ)
{
	auto it = npc_list.begin();
	while (it != npc_list.end()) {
		// add Feign Memory check because sometimes weird stuff happens
		if (it->second->CheckAggro(targ) || (targ->IsClient() && it->second->IsOnFeignMemory(targ))) {
			if (it->second->GetSpecialAbility(SpecialAbility::FeignDeathImmunity)) {
				++it;
				continue;
			}

			if (targ->IsClient()) {
				if (parse->PlayerHasQuestSub(EVENT_FEIGN_DEATH)) {
					std::vector<std::any> args = { it->second };

					int i = parse->EventPlayer(EVENT_FEIGN_DEATH, targ->CastToClient(), "", 0, &args);
					if (i != 0) {
						++it;
						continue;
					}
				}

				if (it->second->IsNPC()) {
					if (parse->HasQuestSub(it->second->GetNPCTypeID(), EVENT_FEIGN_DEATH)) {
						int i = parse->EventNPC(EVENT_FEIGN_DEATH, it->second->CastToNPC(), targ, "", 0);
						if (i != 0) {
							++it;
							continue;
						}
					}
				}
			}

			it->second->RemoveFromHateList(targ);

			if (it->second->GetSpecialAbility(SpecialAbility::Rampage)) {
				it->second->RemoveFromRampageList(targ, true);
			}

			if (targ->IsClient()) {
				if (it->second->GetLevel() >= 35 && zone->random.Roll(60)) {
					it->second->AddFeignMemory(targ);
				}
				else {
					targ->CastToClient()->RemoveXTarget(it->second, false);
				}
			}
			else if (targ->IsPet()){
				if (it->second->GetLevel() >= 35 && zone->random.Roll(60)) {
					it->second->AddFeignMemory(targ);
				}
			}
		}
		++it;
	}
}

void EntityList::ClearZoneFeignAggro(Mob *targ)
{
	auto it = npc_list.begin();
	while (it != npc_list.end()) {
		it->second->RemoveFromFeignMemory(targ);
		if (targ && targ->IsClient()) {
			targ->CastToClient()->RemoveXTarget(it->second, false);
		}
		++it;
	}
}

void EntityList::AggroZone(Mob *who, int64 hate)
{
	auto it = npc_list.begin();
	while (it != npc_list.end()) {
		it->second->AddToHateList(who, hate);
		++it;
	}
}

// Signal Quest command function
void EntityList::SignalMobsByNPCID(uint32 snpc, int signal_id)
{
	auto it = npc_list.begin();
	while (it != npc_list.end()) {
		NPC *pit = it->second;
		if (pit->GetNPCTypeID() == snpc)
			pit->SignalNPC(signal_id);
		++it;
	}
}

bool EntityList::MakeTrackPacket(Client *client)
{
	std::list<std::pair<Mob *, float> > tracking_list;
	auto distance = static_cast<float>(client->GetSkill(EQ::skills::SkillTracking) * client->GetClassTrackingDistanceMultiplier(client->GetClass()));

	if (distance <= 0.0f) {
		return false;
	}

	if (distance < 300.0f) {
		distance = 300.0f;
	}

	for (auto it = mob_list.cbegin(); it != mob_list.cend(); ++it) {
		if (
			!it->second ||
			it->second == client ||
			!it->second->IsTrackable() ||
			it->second->IsInvisible(client)
		) {
			continue;
		}

		const auto mob_distance = DistanceNoZ(it->second->GetPosition(), client->GetPosition());
		if (mob_distance > distance) {
			continue;
		}

		tracking_list.push_back(std::make_pair(it->second, mob_distance));
	}

	tracking_list.sort(
		[](const std::pair<Mob *, float> &a, const std::pair<Mob *, float> &b) {
			return a.first->GetSpawnTimeStamp() > b.first->GetSpawnTimeStamp();
		}
	);

	auto outapp = new EQApplicationPacket(OP_Track, sizeof(Track_Struct) * tracking_list.size());
	auto pack = (Tracking_Struct *) outapp->pBuffer;
	outapp->priority = 6;

	int index = 0;
	for (auto it = tracking_list.cbegin(); it != tracking_list.cend(); ++it, ++index) {
		pack->Entrys[index].entityid = static_cast<uint32>(it->first->GetID());
		pack->Entrys[index].distance = it->second;
		pack->Entrys[index].level = it->first->GetLevel();
		pack->Entrys[index].is_npc = !it->first->IsClient();
		strn0cpy(pack->Entrys[index].name, it->first->GetName(), sizeof(pack->Entrys[index].name));
		pack->Entrys[index].is_pet = it->first->IsPet();
		pack->Entrys[index].is_merc = it->first->IsMerc();
	}

	client->QueuePacket(outapp);
	safe_delete(outapp);

	return true;
}

void EntityList::MessageGroup(Mob *sender, bool skipclose, uint32 type, const char *message, ...)
{
	va_list argptr;
	char buffer[4096];

	va_start(argptr, message);
	vsnprintf(buffer, 4095, message, argptr);
	va_end(argptr);

	float dist2 = 100;

	if (skipclose)
		dist2 = 0;

	auto it = client_list.begin();
	while (it != client_list.end()) {
		if (it->second != sender &&
				(Distance(it->second->GetPosition(), sender->GetPosition()) <= dist2 || it->second->GetGroup() == sender->CastToClient()->GetGroup())) {
			it->second->Message(type, buffer);
		}
		++it;
	}
}

bool EntityList::Fighting(Mob *targ)
{
	auto it = npc_list.begin();
	while (it != npc_list.end()) {
		if (it->second->CheckAggro(targ))
			return true;
		++it;
	}
	return false;
}

void EntityList::AddHealAggro(Mob *target, Mob *caster, uint16 hate)
{
	if (hate == 0)
		return;

	for (auto &e : npc_list) {
		auto &npc = e.second;
		if (!npc->CheckAggro(target) || npc->IsFeared() || npc->IsPet())
			continue;

		if (zone->random.Roll(50)) // witness check -- place holder
			// This is either a level check (con color check?) or a stat roll
			continue;

		if ((npc->IsMezzed() || npc->IsStunned()) && hate > 4) // patch notes say stunned/mezzed NPCs get a fraction of the hate
			npc->AddToHateList(caster, hate / 4); // made up number
		else
			npc->AddToHateList(caster, hate);
	}
}

void EntityList::OpenDoorsNear(Mob *who)
{
	if (!who->CanOpenDoors()) {
		return;
	}

	for (auto &it : door_list) {
		Doors *door = it.second;
		if (!door || door->IsDoorOpen()) {
			continue;
		}

		auto  diff     = who->GetPosition() - door->GetPosition();
		float distance = diff.x * diff.x + diff.y * diff.y;
		if (diff.z * diff.z < 10 && distance <= 100) {
			door->Open(who);
		}
	}
}

void EntityList::SendAlarm(Trap *trap, Mob *currenttarget, uint8 kos)
{
	float preSquareDistance = trap->effectvalue * trap->effectvalue;

	for (auto it = npc_list.begin();it != npc_list.end(); ++it) {
		NPC *cur = it->second;

		auto diff = glm::vec3(cur->GetPosition()) - trap->m_Position;
		float curdist = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

		if (cur->GetOwner() || cur->IsEngaged() || curdist > preSquareDistance )
			continue;

		if (kos) {
			uint8 factioncon = currenttarget->GetReverseFactionCon(cur);
			if (factioncon == FACTION_THREATENINGLY || factioncon == FACTION_SCOWLS) {
				cur->AddToHateList(currenttarget,1);
			}
		}
		else
			cur->AddToHateList(currenttarget,1);
	}
}

void EntityList::AddProximity(NPC *proximity_for)
{
	RemoveProximity(proximity_for->GetID());

	proximity_list.push_back(proximity_for);

	proximity_for->proximity = new NPCProximity; // deleted in NPC::~NPC
}

bool EntityList::RemoveProximity(uint16 delete_npc_id)
{
	auto it = std::find_if(proximity_list.begin(), proximity_list.end(),
			[delete_npc_id](const NPC *a) { return a->GetID() == delete_npc_id; });
	if (it == proximity_list.end())
		return false;

	proximity_list.erase(it);
	return true;
}

void EntityList::RemoveAllLocalities()
{
	proximity_list.clear();
}

struct quest_proximity_event {
	QuestEventID event_id;
	Client *client;
	NPC *npc;
	int area_id;
	int area_type;
};

void EntityList::ProcessMove(Client *c, const glm::vec3& location)
{
	float last_x = c->ProximityX();
	float last_y = c->ProximityY();
	float last_z = c->ProximityZ();

	std::list<quest_proximity_event> events;
	for (auto iter = proximity_list.begin(); iter != proximity_list.end(); ++iter) {
		NPC *d = (*iter);
		NPCProximity *l = d->proximity;
		if (l == nullptr)
			continue;

		//check both bounding boxes, if either coords pairs
		//cross a boundary, send the event.
		bool old_in = true;
		bool new_in = true;
		if (last_x < l->min_x || last_x > l->max_x ||
				last_y < l->min_y || last_y > l->max_y ||
				last_z < l->min_z || last_z > l->max_z) {
			old_in = false;
		}
		if (location.x < l->min_x || location.x > l->max_x ||
				location.y < l->min_y || location.y > l->max_y ||
				location.z < l->min_z || location.z > l->max_z) {
			new_in = false;
		}

		if (old_in && !new_in) {
			quest_proximity_event evt;
			evt.event_id = EVENT_EXIT;
			evt.client = c;
			evt.npc = d;
			evt.area_id = 0;
			evt.area_type = 0;
			events.push_back(evt);
		} else if (new_in && !old_in) {
			quest_proximity_event evt;
			evt.event_id = EVENT_ENTER;
			evt.client = c;
			evt.npc = d;
			evt.area_id = 0;
			evt.area_type = 0;
			events.push_back(evt);
		}
	}

	for (auto iter = area_list.begin(); iter != area_list.end(); ++iter) {
		Area& a = (*iter);
		bool old_in = true;
		bool new_in = true;
		if (last_x < a.min_x || last_x > a.max_x ||
				last_y < a.min_y || last_y > a.max_y ||
				last_z < a.min_z || last_z > a.max_z) {
			old_in = false;
		}

		if (location.x < a.min_x || location.x > a.max_x ||
				location.y < a.min_y || location.y > a.max_y ||
				location.z < a.min_z || location.z > a.max_z ) {
			new_in = false;
		}

		if (old_in && !new_in) {
			//were in but are no longer.
			quest_proximity_event evt;
			evt.event_id = EVENT_LEAVE_AREA;
			evt.client = c;
			evt.npc = nullptr;
			evt.area_id = a.id;
			evt.area_type = a.type;
			events.push_back(evt);
		} else if (!old_in && new_in) {
			//were not in but now are
			quest_proximity_event evt;
			evt.event_id = EVENT_ENTER_AREA;
			evt.client = c;
			evt.npc = nullptr;
			evt.area_id = a.id;
			evt.area_type = a.type;
			events.push_back(evt);
		}
	}

	for (auto iter = events.begin(); iter != events.end(); ++iter) {
		quest_proximity_event& evt = (*iter);

		std::vector<std::any> args = { &evt.area_id, &evt.area_type };

		if (evt.npc) {
			if (evt.event_id == EVENT_ENTER) {
				if (parse->HasQuestSub(evt.npc->GetNPCTypeID(), EVENT_ENTER)) {
					parse->EventNPC(EVENT_ENTER, evt.npc, evt.client, "", 0);
				}
			} else if (evt.event_id == EVENT_EXIT) {
				if (parse->HasQuestSub(evt.npc->GetNPCTypeID(), EVENT_EXIT)) {
					parse->EventNPC(EVENT_EXIT, evt.npc, evt.client, "", 0);
				}
			} else if (evt.event_id == EVENT_ENTER_AREA) {
				if (parse->HasQuestSub(evt.npc->GetNPCTypeID(), EVENT_ENTER_AREA)) {
					parse->EventNPC(EVENT_ENTER_AREA, evt.npc, evt.client, "", 0, &args);
				}
			} else if (evt.event_id == EVENT_LEAVE_AREA) {
				if (parse->HasQuestSub(evt.npc->GetNPCTypeID(), EVENT_LEAVE_AREA)) {
					parse->EventNPC(EVENT_LEAVE_AREA, evt.npc, evt.client, "", 0, &args);
				}
			}
		} else {
			if (evt.event_id == EVENT_ENTER) {
				if (parse->PlayerHasQuestSub(EVENT_ENTER)) {
					parse->EventPlayer(EVENT_ENTER, evt.client, "", 0);
				}
			} else if (evt.event_id == EVENT_EXIT) {
				if (parse->PlayerHasQuestSub(EVENT_EXIT)) {
					parse->EventPlayer(EVENT_EXIT, evt.client, "", 0);
				}
			} else if (evt.event_id == EVENT_ENTER_AREA) {
				if (parse->PlayerHasQuestSub(EVENT_ENTER_AREA)) {
					parse->EventPlayer(EVENT_ENTER_AREA, evt.client, "", 0, &args);
				}
			} else if (evt.event_id == EVENT_LEAVE_AREA) {
				if (parse->PlayerHasQuestSub(EVENT_LEAVE_AREA)) {
					parse->EventPlayer(EVENT_LEAVE_AREA, evt.client, "", 0, &args);
				}
			}
		}
	}
}

void EntityList::ProcessMove(NPC *n, float x, float y, float z) {
	float last_x = n->GetX();
	float last_y = n->GetY();
	float last_z = n->GetZ();

	std::list<quest_proximity_event> events;

	for (const auto& a : area_list) {
		bool old_in = true;
		bool new_in = true;
		if (
			!EQ::ValueWithin(last_x, a.min_x, a.max_x) ||
			!EQ::ValueWithin(last_y, a.min_y, a.max_y) ||
			!EQ::ValueWithin(last_z, a.min_z, a.max_z)
		) {
			old_in = false;
		}

		if (
			!EQ::ValueWithin(x, a.min_x, a.max_x) ||
			!EQ::ValueWithin(y, a.min_y, a.max_y) ||
			!EQ::ValueWithin(z, a.min_z, a.max_z)
		) {
			new_in = false;
		}

		if (old_in && !new_in) {
			//were in but are no longer.
			quest_proximity_event evt;
			evt.event_id  = EVENT_LEAVE_AREA;
			evt.client    = nullptr;
			evt.npc       = n;
			evt.area_id   = a.id;
			evt.area_type = a.type;
			events.emplace_back(evt);
		}
		else if (!old_in && new_in) {
			//were not in but now are
			quest_proximity_event evt;
			evt.event_id  = EVENT_ENTER_AREA;
			evt.client    = nullptr;
			evt.npc       = n;
			evt.area_id   = a.id;
			evt.area_type = a.type;
			events.emplace_back(evt);
		}
	}

	for (const auto& evt : events) {
		std::vector<std::any> args = { &evt.area_id, &evt.area_type };

		if (evt.event_id == EVENT_ENTER) {
			if (parse->HasQuestSub(evt.npc->GetNPCTypeID(), EVENT_ENTER)) {
				parse->EventNPC(EVENT_ENTER, evt.npc, evt.client, "", 0);
			}
		} else if (evt.event_id == EVENT_EXIT) {
			if (parse->HasQuestSub(evt.npc->GetNPCTypeID(), EVENT_EXIT)) {
				parse->EventNPC(EVENT_EXIT, evt.npc, evt.client, "", 0);
			}
		} else if (evt.event_id == EVENT_ENTER_AREA) {
			if (parse->HasQuestSub(evt.npc->GetNPCTypeID(), EVENT_ENTER_AREA)) {
				parse->EventNPC(EVENT_ENTER_AREA, evt.npc, evt.client, "", 0, &args);
			}
		} else if (evt.event_id == EVENT_LEAVE_AREA) {
			if (parse->HasQuestSub(evt.npc->GetNPCTypeID(), EVENT_LEAVE_AREA)) {
				parse->EventNPC(EVENT_LEAVE_AREA, evt.npc, evt.client, "", 0, &args);
			}
		}
	}
}

void EntityList::AddArea(int id, int type, float min_x, float max_x, float min_y,
		float max_y, float min_z, float max_z)
{
	RemoveArea(id);
	Area a;
	a.id = id;
	a.type = type;
	if (min_x > max_x) {
		a.min_x = max_x;
		a.max_x = min_x;
	} else {
		a.min_x = min_x;
		a.max_x = max_x;
	}

	if (min_y > max_y) {
		a.min_y = max_y;
		a.max_y = min_y;
	} else {
		a.min_y = min_y;
		a.max_y = max_y;
	}

	if (min_z > max_z) {
		a.min_z = max_z;
		a.max_z = min_z;
	} else {
		a.min_z = min_z;
		a.max_z = max_z;
	}

	area_list.push_back(a);
}

void EntityList::RemoveArea(int id)
{
	auto it = std::find_if(area_list.begin(), area_list.end(),
			[id](const Area &a) { return a.id == id; });
	if (it == area_list.end())
		return;

	area_list.erase(it);
}

void EntityList::ClearAreas()
{
	area_list.clear();
}

void EntityList::ProcessProximitySay(const char *message, Client *c, uint8 language)
{
	if (!message || !c) {
		return;
	}

	for (const auto& n : proximity_list) {
		auto* p = n->proximity;
		if (!p || !p->say) {
			continue;
		}

		if (!parse->HasQuestSub(n->GetNPCTypeID(), EVENT_PROXIMITY_SAY)) {
			continue;
		}

		if (
			!EQ::ValueWithin(c->GetX(), p->min_x, p->max_x) ||
			!EQ::ValueWithin(c->GetY(), p->min_y, p->max_y) ||
			!EQ::ValueWithin(c->GetZ(), p->min_z, p->max_z)
		) {
			continue;
		}

		parse->EventNPC(EVENT_PROXIMITY_SAY, n, c, message, language);
	}
}

void EntityList::SaveAllClientsTaskState()
{
	auto it = client_list.begin();
	while (it != client_list.end()) {
		Client *client = it->second;
		if (client->IsTaskStateLoaded()) {
			client->SaveTaskState();
		}

		++it;
	}
}

void EntityList::ReloadAllClientsTaskState(int task_id)
{
	auto it = client_list.begin();
	while (it != client_list.end()) {
		Client *client = it->second;
		if (client->IsTaskStateLoaded()) {
			// If we have been passed a TaskID, only reload the client state if they have
			// that Task active.
			if ((!task_id) || (task_id && client->IsTaskActive(task_id))) {
				Log(Logs::General, Logs::Tasks, "[CLIENTLOAD] Reloading Task State For Client %s", client->GetName());
				client->RemoveClientTaskState();
				client->LoadClientTaskState();
				TaskManager::Instance()->SendActiveTasksToClient(client);
			}
		}
		++it;
	}
}

bool EntityList::IsMobInZone(Mob *who)
{
	//We don't use mob_list.find(who) because this code needs to be able to handle dangling pointers for the quest code.
	auto it = mob_list.begin();
	while(it != mob_list.end()) {
		if(it->second == who) {
			return true;
		}
		++it;
	}

	auto enc_it = encounter_list.begin();
	while (enc_it != encounter_list.end()) {
		if (enc_it->second == who) {
			return true;
		}
		++enc_it;
	}

	return false;
}

/*
Code to limit the amount of certain NPCs in a given zone.
Primarily used to make a named mob unique within the zone, but written
to be more generic allowing limits larger than 1.

Maintain this stuff in a seperate list since the number
of limited NPCs will most likely be much smaller than the number
of NPCs in the entire zone.
*/
void EntityList::LimitAddNPC(NPC *npc)
{
	if (!npc)
		return;

	SpawnLimitRecord r;

	uint16 eid = npc->GetID();
	r.spawngroup_id = npc->GetSpawnGroupId();
	r.npc_type = npc->GetNPCTypeID();

	npc_limit_list[eid] = r;
}

void EntityList::LimitRemoveNPC(NPC *npc)
{
	if (!npc)
		return;

	uint16 eid = npc->GetID();
	npc_limit_list.erase(eid);
}

//check a limit over the entire zone.
//returns true if the limit has not been reached
bool EntityList::LimitCheckType(uint32 npc_type, int count)
{
	if (count < 1)
		return true;

	std::map<uint16, SpawnLimitRecord>::iterator cur,end;
	cur = npc_limit_list.begin();
	end = npc_limit_list.end();

	for (; cur != end; ++cur) {
		if (cur->second.npc_type == npc_type) {
			count--;
			if (count == 0) {
				return false;
			}
		}
	}
	return true;
}

//check limits on an npc type in a given spawn group.
//returns true if the limit has not been reached
bool EntityList::LimitCheckGroup(uint32 spawngroup_id, int count)
{
	if (count < 1)
		return true;

	std::map<uint16, SpawnLimitRecord>::iterator cur,end;
	cur = npc_limit_list.begin();
	end = npc_limit_list.end();

	for (; cur != end; ++cur) {
		if (cur->second.spawngroup_id == spawngroup_id) {
			count--;
			if (count == 0) {
				return false;
			}
		}
	}
	return true;
}

bool EntityList::LimitCheckName(const char *npc_name)
{
	auto it = npc_list.begin();
	while (it != npc_list.end()) {
		NPC *npc = it->second;
		if (npc) {
			if (strcasecmp(npc_name, npc->GetRawNPCTypeName()) == 0) {
				return false;
			}
		}
		++it;
	}
	return true;
}

void EntityList::UpdateHoTT(Mob *target)
{
	auto it = client_list.begin();
	while (it != client_list.end()) {
		Client *c = it->second;
		if (c->GetTarget() == target) {
			if (target->GetTarget())
				c->SetHoTT(target->GetTarget()->GetID());
			else
				c->SetHoTT(0);

			c->UpdateXTargetType(TargetsTarget, target->GetTarget());
		}
		++it;
	}
}

void EntityList::DestroyTempPets(Mob *owner)
{
	auto it = npc_list.begin();
	while (it != npc_list.end()) {
		NPC* n = it->second;
		if (n->GetSwarmInfo()) {
			if (n->GetSwarmInfo()->owner_id == owner->GetID()) {
				n->Depop();
			}
		}
		++it;
	}
}

void EntityList::AddTempPetsToHateList(Mob *owner, Mob* other, bool bFrenzy)
{
	if (!other || !owner)
		return;

	auto it = npc_list.begin();
	while (it != npc_list.end()) {
		NPC* n = it->second;
		if (n && n->GetSwarmInfo()) {
			if (n->GetSwarmInfo()->owner_id == owner->GetID()) {
				if (
					!n->GetSpecialAbility(SpecialAbility::AggroImmunity) &&
					!(other->IsBot() && n->GetSpecialAbility(SpecialAbility::BotAggroImmunity)) &&
					!(other->IsClient() && n->GetSpecialAbility(SpecialAbility::ClientAggroImmunity)) &&
					!(other->IsNPC() && n->GetSpecialAbility(SpecialAbility::NPCAggroImmunity))
				) {
					n->hate_list.AddEntToHateList(other, 0, 0, bFrenzy);
				}
			}
		}
		++it;
	}
}

void EntityList::AddTempPetsToHateListOnOwnerDamage(Mob *owner, Mob* attacker, int32 spell_id)
{
	if (!attacker || !owner)
		return;

	auto it = npc_list.begin();
	while (it != npc_list.end()) {
		NPC* n = it->second;
		if (n && n->GetSwarmInfo()) {
			if (n->GetSwarmInfo()->owner_id == owner->GetID()) {
				if (
					attacker &&
					attacker != n &&
					!n->IsEngaged() &&
					!n->GetSpecialAbility(SpecialAbility::AggroImmunity) &&
					!(attacker->IsBot() && n->GetSpecialAbility(SpecialAbility::BotAggroImmunity)) &&
					!(attacker->IsClient() && n->GetSpecialAbility(SpecialAbility::ClientAggroImmunity)) &&
					!(attacker->IsNPC() && n->GetSpecialAbility(SpecialAbility::NPCAggroImmunity)) &&
					!attacker->IsTrap() &&
					!attacker->IsCorpse()
					) {
					n->AddToHateList(attacker, 1, 0, true, false, false, spell_id);
					n->SetTarget(attacker);
				}
			}
		}
		++it;
	}
}

void EntityList::QuestJournalledSayClose(
	Mob *sender, float dist, const char *mobname, const char *message,
	Journal::Options &opts
)
{
	SerializeBuffer buf(sizeof(SpecialMesgHeader_Struct) + 12 + 64 + 64);

	buf.WriteInt8(static_cast<int8>(opts.speak_mode));
	buf.WriteInt8(static_cast<int8>(opts.journal_mode));
	buf.WriteInt8(opts.language);
	buf.WriteInt32(opts.message_type);
	buf.WriteInt32(opts.target_spawn_id);
	buf.WriteString(mobname);
	buf.WriteInt32(0); // location, client doesn't seem to do anything with this
	buf.WriteInt32(0);
	buf.WriteInt32(0);

	if (RuleB(Chat, QuestDialogueUsesDialogueWindow)) {
		for (auto &e : sender->GetCloseMobList(dist)) {
			Mob *mob = e.second;
			if (!mob) {
				continue;
			}

			if (!mob->IsClient()) {
				continue;
			}

			Client *client = mob->CastToClient();

			if (client->GetTarget() && client->GetTarget()->IsMob() && client->GetTarget()->CastToMob() == sender) {
				std::string window_markdown = message;
				DialogueWindow::Render(client, window_markdown);
			}
		}

		return;
	}
	else if (RuleB(Chat, AutoInjectSaylinksToSay)) {
		std::string new_message = EQ::SayLinkEngine::InjectSaylinksIfNotExist(message);
		buf.WriteString(new_message);
	}
	else {
		buf.WriteString(message);
	}

	auto outapp = new EQApplicationPacket(OP_SpecialMesg, buf);

	// client only bothers logging if target spawn ID matches, safe to send to everyone
	QueueCloseClients(sender, outapp, false, dist);
	delete outapp;
}

bool Entity::CheckCoordLosNoZLeaps(float cur_x, float cur_y, float cur_z,
		float trg_x, float trg_y, float trg_z, float perwalk)
{
	if (zone->zonemap == nullptr) {
		return true;
	}

	glm::vec3 myloc;
	glm::vec3 oloc;
	glm::vec3 hit;

	myloc.x = cur_x;
	myloc.y = cur_y;
	myloc.z = cur_z+5;

	oloc.x = trg_x;
	oloc.y = trg_y;
	oloc.z = trg_z+5;

	if (myloc.x == oloc.x && myloc.y == oloc.y && myloc.z == oloc.z) {
		return true;
	}

	if (!zone->zonemap->LineIntersectsZoneNoZLeaps(myloc,oloc,perwalk,&hit)) {
		return true;
	}

	return false;
}

Corpse *EntityList::GetClosestCorpse(Mob *sender, const char *Name)
{
	if (!sender)
		return nullptr;

	uint32 CurrentDistance, ClosestDistance = 4294967295u;
	Corpse *CurrentCorpse, *ClosestCorpse = nullptr;

	auto it = corpse_list.begin();
	while (it != corpse_list.end()) {
		CurrentCorpse = it->second;

		++it;

		if (Name && strcasecmp(CurrentCorpse->GetOwnerName(), Name))
			continue;

		CurrentDistance = ((CurrentCorpse->GetY() - sender->GetY()) * (CurrentCorpse->GetY() - sender->GetY())) +
					((CurrentCorpse->GetX() - sender->GetX()) * (CurrentCorpse->GetX() - sender->GetX()));

		if (CurrentDistance < ClosestDistance) {
			ClosestDistance = CurrentDistance;

			ClosestCorpse = CurrentCorpse;

		}
	}
	return ClosestCorpse;
}

void EntityList::TryWakeTheDead(Mob *sender, Mob *target, int32 spell_id, uint32 max_distance, uint32 duration, uint32 amount_pets)
{
	if (!sender) {
		return;
	}

	std::vector<int> used_corpse_list;

	for (int i = 0; i < amount_pets; i++)
	{
		uint32 CurrentDistance, ClosestDistance = 4294967295u;
		Corpse *CurrentCorpse, *ClosestCorpse = nullptr;

		auto it = corpse_list.begin();
		while (it != corpse_list.end()) {
			CurrentCorpse = it->second;

			++it;

			bool corpse_already_used = false;
			for (auto itr = used_corpse_list.begin(); itr != used_corpse_list.end(); ++itr) {
				if ((*itr) && (*itr) == CurrentCorpse->GetID()) {
					corpse_already_used = true;
					continue;
				}
			}

			if (corpse_already_used) {
				continue;
			}

			CurrentDistance = static_cast<uint32>(sender->CalculateDistance(CurrentCorpse->GetX(), CurrentCorpse->GetY(), CurrentCorpse->GetZ()));

			if (max_distance && CurrentDistance > max_distance) {
				continue;
			}

			if (CurrentDistance < ClosestDistance) {
				ClosestDistance = CurrentDistance;
				ClosestCorpse = CurrentCorpse;
			}
		}

		if (ClosestCorpse) {
			sender->WakeTheDead(spell_id, ClosestCorpse, target, duration);
			used_corpse_list.push_back(ClosestCorpse->GetID());
		}
	}
}

void EntityList::ForceGroupUpdate(uint32 gid)
{
	auto it = client_list.begin();
	while (it != client_list.end()) {
		if (it->second){
			Group *g = nullptr;
			g = it->second->GetGroup();
			if (g) {
				if (g->GetID() == gid) {
					database.RefreshGroupFromDB(it->second);
				}
			}
		}
		++it;
	}
}

void EntityList::SendGroupLeave(uint32 gid, const char *name)
{
	auto it = client_list.begin();
	while (it != client_list.end()) {
		Client *c = it->second;
		if (c) {
			Group *g = nullptr;
			g = c->GetGroup();
			if (g) {
				if (g->GetID() == gid) {
					auto outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
					GroupJoin_Struct* gj = (GroupJoin_Struct*) outapp->pBuffer;
					strcpy(gj->membername, name);
					gj->action = groupActLeave;
					strcpy(gj->yourname, c->GetName());
					Mob *Leader = g->GetLeader();
					if (Leader)
						Leader->CastToClient()->GetGroupAAs(&gj->leader_aas);
					c->QueuePacket(outapp);
					safe_delete(outapp);
					g->DelMemberOOZ(name);
					if (g->IsLeader(c) && c->IsLFP())
						c->UpdateLFP();
				}
			}
		}
		++it;
	}
}

void EntityList::SendGroupJoin(uint32 gid, const char *name)
{
	auto it = client_list.begin();
	while (it != client_list.end()) {
		if (it->second){
			Group *g = nullptr;
			g = it->second->GetGroup();
			if (g) {
				if (g->GetID() == gid) {
					auto outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
					GroupJoin_Struct* gj = (GroupJoin_Struct*) outapp->pBuffer;
					strcpy(gj->membername, name);
					gj->action = groupActJoin;
					strcpy(gj->yourname, it->second->GetName());
					Mob *Leader = g->GetLeader();
					if (Leader)
						Leader->CastToClient()->GetGroupAAs(&gj->leader_aas);

					it->second->QueuePacket(outapp);
					safe_delete(outapp);
				}
			}
		}
		++it;
	}
}

void EntityList::GroupMessage(uint32 gid, const char *from, const char *message)
{
	auto it = client_list.begin();
	while (it != client_list.end()) {
		if (it->second) {
			Group *g = nullptr;
			g = it->second->GetGroup();
			if (g) {
				if (g->GetID() == gid)
					it->second->ChannelMessageSend(from, it->second->GetName(), ChatChannel_Group, Language::CommonTongue, Language::MaxValue, message);
			}
		}
		++it;
	}
}

uint16 EntityList::CreateGroundObject(uint32 item_id, const glm::vec4& position, uint32 decay_time)
{
	const auto is = database.GetItem(item_id);
	if (!is) {
		return 0;
	}

	auto inst = new EQ::ItemInstance(is, is->MaxCharges);
	if (!inst) {
		return 0;
	}

	auto object = new Object(inst, position.x, position.y, position.z, position.w, decay_time);

	entity_list.AddObject(object, true);

	safe_delete(inst);

	if (!object) {
		return 0;
	}

	return object->GetID();
}

uint16 EntityList::CreateGroundObjectFromModel(const char *model, const glm::vec4& position, uint8 type, uint32 decay_time)
{
	if (!model) {
		return 0;
	}

	auto object = new Object(model, position.x, position.y, position.z, position.w, type);

	entity_list.AddObject(object, true);

	if (!object) {
		return 0;
	}

	return object->GetID();
}

uint16 EntityList::CreateDoor(const char *model, const glm::vec4& position, uint8 opentype, uint16 size)
{
	if (!model)
		return 0; // fell through everything, this is bad/incomplete from perl

	auto door = new Doors(model, position, opentype, size);
	RemoveAllDoors();
	zone->LoadZoneDoors();
	entity_list.AddDoor(door);
	entity_list.RespawnAllDoors();

	if (door)
		return door->GetEntityID();

	return 0; // fell through everything, this is bad/incomplete from perl
}


Mob *EntityList::GetTargetForMez(Mob *caster)
{
	if (!caster)
		return nullptr;

	auto it = mob_list.begin();
	//TODO: make this smarter and not mez targets being damaged by dots
	while (it != mob_list.end()) {
		Mob *d = it->second;
		if (d) {
			if (d == caster) { //caster can't pick himself
				++it;
				continue;
			}

			if (caster->GetTarget() == d) { //caster can't pick his target
				++it;
				continue;
			}

			if (!caster->CheckAggro(d)) { //caster can't pick targets that aren't aggroed on himself
				++it;
				continue;
			}

			if (DistanceSquared(caster->GetPosition(), d->GetPosition()) > 22250) { //only pick targets within 150 range
				++it;
				continue;
			}

			if (!caster->CheckLosFN(d)) {	//this is wasteful but can't really think of another way to do it
				++it;						//that wont have us trying to los the same target every time
				continue;					//it's only in combat so it's impact should be minimal.. but stil.
			}
			return d;
		}
		++it;
	}
	return nullptr;
}

void EntityList::SendZoneAppearance(Client *c)
{
	if (!c) {
		return;
	}

	auto it = mob_list.begin();
	while (it != mob_list.end()) {
		Mob *cur = it->second;

		if (cur) {
			if (cur == c) {
				++it;
				continue;
			}
			if (cur->GetAppearance() != eaStanding) {
				cur->SendAppearancePacket(AppearanceType::Animation, cur->GetAppearanceValue(cur->GetAppearance()), false, true, c);
			}
			if (cur->GetSize() != cur->GetBaseSize()) {
				cur->SendAppearancePacket(AppearanceType::Size, (uint32) cur->GetSize(), false, true, c);
			}
		}
		++it;
	}
}

void EntityList::SendNimbusEffects(Client *c)
{
	if (!c) {
		return;
	}

	auto it = mob_list.begin();
	while (it != mob_list.end()) {
		Mob *cur = it->second;

		if (cur) {
			if (cur == c) {
				++it;
				continue;
			}
			if (cur->GetNimbusEffect1() != 0) {
				cur->SendSpellEffect(cur->GetNimbusEffect1(), 1000, 0, 1, 3000, false, c);
			}
			if (cur->GetNimbusEffect2() != 0) {
				cur->SendSpellEffect(cur->GetNimbusEffect2(), 2000, 0, 1, 3000, false, c);
			}
			if (cur->GetNimbusEffect3() != 0) {
				cur->SendSpellEffect(cur->GetNimbusEffect3(), 3000, 0, 1, 3000, false, c);
			}
		}
		++it;
	}
}

void EntityList::SendUntargetable(Client *c)
{
	if (!c) {
		return;
	}

	auto it = mob_list.begin();
	while (it != mob_list.end()) {
		Mob *cur = it->second;

		if (cur) {
			if (cur == c) {
				++it;
				continue;
			}
			if (!cur->IsTargetable()) {
				cur->SendTargetable(false, c);
			}
		}
		++it;
	}
}

void EntityList::SendAppearanceEffects(Client *c)
{
	if (!c) {
		return;
	}

	auto it = mob_list.begin();
	while (it != mob_list.end()) {
		Mob *cur = it->second;

		if (cur) {
			if (cur == c) {
				++it;
				continue;
			}
			cur->SendSavedAppearanceEffects(c);
		}
		++it;
	}
}

void EntityList::SendIllusionWearChange(Client *c)
{
	if (!c) {
		return;
	}

	for (auto &e : mob_list) {
		auto &mob = e.second;

		if (mob) {
			if (mob == c) {
				continue;
			}

			mob->SendIllusionWearChange(c);
		}
	}
}

void EntityList::ZoneWho(Client *c, Who_All_Struct *Who)
{
	// This is only called for SoF clients, as regular /who is now handled server-side for that client.
	uint32 PacketLength = 0;
	uint32 Entries = 0;
	uint8 WhomLength = strlen(Who->whom);

	std::list<Client *> client_sub_list;
	auto it = client_list.begin();
	while (it != client_list.end()) {
		Client *ClientEntry = it->second;
		++it;

		if (ClientEntry) {
			if (ClientEntry->GMHideMe(c))
				continue;
			if ((Who->wrace != 0xFFFFFFFF) && (ClientEntry->GetRace() != Who->wrace))
				continue;
			if ((Who->wclass != 0xFFFFFFFF) && (ClientEntry->GetClass() != Who->wclass))
				continue;
			if ((Who->lvllow != 0xFFFFFFFF) && (ClientEntry->GetLevel() < Who->lvllow))
				continue;
			if ((Who->lvlhigh != 0xFFFFFFFF) && (ClientEntry->GetLevel() > Who->lvlhigh))
				continue;
			if (Who->guildid != 0xFFFFFFFF) {
				if ((Who->guildid == 0xFFFFFFFC) && !ClientEntry->IsTrader())
					continue;
				if ((Who->guildid == 0xFFFFFFFB) && !ClientEntry->IsBuyer())
					continue;
				if (Who->guildid != ClientEntry->GuildID())
					continue;
			}
			if (WhomLength && strncasecmp(Who->whom, ClientEntry->GetName(), WhomLength) &&
					strncasecmp(guild_mgr.GetGuildName(ClientEntry->GuildID()), Who->whom, WhomLength))
				continue;

			Entries++;
			client_sub_list.push_back(ClientEntry);

			PacketLength = PacketLength + strlen(ClientEntry->GetName());

			if (strlen(guild_mgr.GetGuildName(ClientEntry->GuildID())) > 0)
				PacketLength = PacketLength + strlen(guild_mgr.GetGuildName(ClientEntry->GuildID())) + 2;
		}
	}

	PacketLength = PacketLength + sizeof(WhoAllReturnStruct) + (47 * Entries);
	auto outapp = new EQApplicationPacket(OP_WhoAllResponse, PacketLength);
	char *Buffer = (char *)outapp->pBuffer;
	WhoAllReturnStruct *WARS = (WhoAllReturnStruct *)Buffer;
	WARS->id = 0;
	WARS->playerineqstring = 5001;
	strncpy(WARS->line, "---------------------------", sizeof(WARS->line));
	WARS->unknown35 = 0x0a;
	WARS->unknown36 = 0;

	switch(Entries) {
		case 0:
			WARS->playersinzonestring = 5029;
			break;
		case 1:
			WARS->playersinzonestring = 5028; // 5028 There is %1 player in EverQuest.
			break;
		default:
			WARS->playersinzonestring = 5036; // 5036 There are %1 players in EverQuest.
	}

	WARS->unknown44[0] = 0;
	WARS->unknown44[1] = 0;
	WARS->unknown52 = Entries;
	WARS->unknown56 = Entries;
	WARS->playercount = Entries;
	Buffer += sizeof(WhoAllReturnStruct);

	auto sit = client_sub_list.begin();
	while (sit != client_sub_list.end()) {
		Client *ClientEntry = *sit;
		++sit;

		if (ClientEntry) {
			if (ClientEntry->GMHideMe(c))
				continue;
			if ((Who->wrace != 0xFFFFFFFF) && (ClientEntry->GetRace() != Who->wrace))
				continue;
			if ((Who->wclass != 0xFFFFFFFF) && (ClientEntry->GetClass() != Who->wclass))
				continue;
			if ((Who->lvllow != 0xFFFFFFFF) && (ClientEntry->GetLevel() < Who->lvllow))
				continue;
			if ((Who->lvlhigh != 0xFFFFFFFF) && (ClientEntry->GetLevel() > Who->lvlhigh))
				continue;
			if (Who->guildid != 0xFFFFFFFF) {
				if ((Who->guildid == 0xFFFFFFFC) && !ClientEntry->IsTrader())
					continue;
				if ((Who->guildid == 0xFFFFFFFB) && !ClientEntry->IsBuyer())
					continue;
				if (Who->guildid != ClientEntry->GuildID())
					continue;
			}
			if (WhomLength && strncasecmp(Who->whom, ClientEntry->GetName(), WhomLength) &&
					strncasecmp(guild_mgr.GetGuildName(ClientEntry->GuildID()), Who->whom, WhomLength))
				continue;
			std::string GuildName;
			if ((ClientEntry->GuildID() != GUILD_NONE) && (ClientEntry->GuildID() > 0)) {
				GuildName = "<";
				GuildName += guild_mgr.GetGuildName(ClientEntry->GuildID());
				GuildName += ">";
			}
			uint32 FormatMSGID = 5025; // 5025 %T1[%2 %3] %4 (%5) %6 %7 %8 %9
			if (ClientEntry->GetAnon() == 1)
				FormatMSGID = 5024; // 5024 %T1[ANONYMOUS] %2 %3
			else if (ClientEntry->GetAnon() == 2)
				FormatMSGID = 5023; // 5023 %T1[ANONYMOUS] %2 %3 %4
			uint32 PlayerClass = Class::None;
			uint32 PlayerLevel = 0;
			uint32 PlayerRace = Race::Doug;
			uint32 ZoneMSGID = 0xFFFFFFFF;

			if (ClientEntry->GetAnon()==0) {
				PlayerClass = ClientEntry->GetClass();
				PlayerLevel = ClientEntry->GetLevel();
				PlayerRace = ClientEntry->GetRace();
			}

			WhoAllPlayerPart1* WAPP1 = (WhoAllPlayerPart1*)Buffer;
			WAPP1->FormatMSGID = FormatMSGID;
			WAPP1->PIDMSGID = 0xFFFFFFFF;
			strcpy(WAPP1->Name, ClientEntry->GetName());
			Buffer += sizeof(WhoAllPlayerPart1) + strlen(WAPP1->Name);
			WhoAllPlayerPart2* WAPP2 = (WhoAllPlayerPart2*)Buffer;

			if (ClientEntry->IsTrader())
				WAPP2->RankMSGID = 12315;
			else if (ClientEntry->IsBuyer())
				WAPP2->RankMSGID = 6056;
			else if (ClientEntry->Admin() >= AccountStatus::Steward && ClientEntry->GetGM())
				WAPP2->RankMSGID = 12312;
			else
				WAPP2->RankMSGID = 0xFFFFFFFF;

			strcpy(WAPP2->Guild, GuildName.c_str());
			Buffer += sizeof(WhoAllPlayerPart2) + strlen(WAPP2->Guild);
			WhoAllPlayerPart3* WAPP3 = (WhoAllPlayerPart3*)Buffer;
			WAPP3->Unknown80[0] = 0xFFFFFFFF;

			if (ClientEntry->IsLD())
				WAPP3->Unknown80[1] = 12313; // LinkDead
			else
				WAPP3->Unknown80[1] = 0xFFFFFFFF;

			WAPP3->ZoneMSGID = ZoneMSGID;
			WAPP3->Zone = 0;
			WAPP3->Class_ = PlayerClass;
			WAPP3->Level = PlayerLevel;
			WAPP3->Race = PlayerRace;
			WAPP3->Account[0] = 0;
			Buffer += sizeof(WhoAllPlayerPart3);
			WhoAllPlayerPart4* WAPP4 = (WhoAllPlayerPart4*)Buffer;
			WAPP4->Unknown100 = 0;
			Buffer += sizeof(WhoAllPlayerPart4);
		}

	}

	c->QueuePacket(outapp);

	safe_delete(outapp);
}

void EntityList::UnMarkNPC(uint16 ID)
{
	// Designed to be called from the Mob destructor, this method calls Group::UnMarkNPC for
	// each group to remove the dead mobs entity ID from the groups list of NPCs marked via the
	// Group Leadership AA Mark NPC ability.
	//
	auto it = group_list.begin();
	while (it != group_list.end()) {
		if (*it)
			(*it)->UnMarkNPC(ID);
		++it;
	}
}

uint32 EntityList::CheckNPCsClose(Mob *center)
{
	uint32 count = 0;

	auto it = npc_list.begin();
	while (it != npc_list.end()) {
		NPC *cur = it->second;
		if (!cur || cur == center || cur->IsPet() || cur->GetClass() == Class::LDoNTreasure ||
				cur->GetBodyType() == BodyType::NoTarget || cur->GetBodyType() == BodyType::Special) {
			++it;
			continue;
		}

		float xDiff = cur->GetX() - center->GetX();
		float yDiff = cur->GetY() - center->GetY();
		float zDiff = cur->GetZ() - center->GetZ();
		float dist = ((xDiff * xDiff) + (yDiff * yDiff) + (zDiff * zDiff));

		if (dist <= RuleR(Adventure, DistanceForRescueAccept))
			count++;
		++it;
	}
	return count;
}

void EntityList::SignalAllClients(int signal_id)
{
	for (const auto& c : client_list) {
		if (c.second) {
			c.second->Signal(signal_id);
		}
	}
}

void EntityList::GetMobList(std::list<Mob *> &m_list)
{
	m_list.clear();
	auto it = mob_list.begin();
	while (it != mob_list.end()) {
		m_list.push_back(it->second);
		++it;
	}
}

void EntityList::GetNPCList(std::list<NPC *> &n_list)
{
	n_list.clear();
	auto it = npc_list.begin();
	while (it != npc_list.end()) {
		n_list.push_back(it->second);
		++it;
	}
}

void EntityList::GetClientList(std::list<Client *> &c_list)
{
	c_list.clear();
	auto it = client_list.begin();
	while (it != client_list.end()) {
		c_list.push_back(it->second);
		++it;
	}
}

void EntityList::GetBotList(std::list<Bot *> &b_list)
{
	b_list.clear();
	auto it = bot_list.begin();
	while (it != bot_list.end()) {
		b_list.push_back(it->second);
		++it;
	}
}

std::vector<Bot *> EntityList::GetBotListByCharacterID(uint32 character_id, uint8 class_id)
{
	std::vector<Bot *> client_bot_list;

	if (!character_id) {
		return client_bot_list;
	}

	auto it = bot_list.begin();

	while (it != bot_list.end()) {
		if (it->second->GetOwner() && it->second->GetBotOwnerCharacterID() == character_id && (!class_id || it->second->GetClass() == class_id)) {
			client_bot_list.push_back(it->second);
		}
		++it;
	}

	return client_bot_list;
}

std::vector<Bot *> EntityList::GetBotListByClientName(std::string client_name, uint8 class_id)
{
	std::vector<Bot *> client_bot_list;

	if (client_name.empty()) {
		return client_bot_list;
	}

	auto it = bot_list.begin();

	while (it != bot_list.end()) {
		if (it->second->GetOwner() && Strings::ToLower(it->second->GetOwner()->GetCleanName()) == Strings::ToLower(client_name) && (!class_id || it->second->GetClass() == class_id)) {
			client_bot_list.push_back(it->second);
		}
		++it;
	}

	return client_bot_list;
}

void EntityList::SignalAllBotsByOwnerCharacterID(uint32 character_id, int signal_id)
{
	auto client_bot_list = GetBotListByCharacterID(character_id);
	if (client_bot_list.empty()) {
		return;
	}

	for (const auto& b : client_bot_list) {
		b->Signal(signal_id);
	}
}

void EntityList::SignalAllBotsByOwnerName(std::string owner_name, int signal_id)
{
	auto client_bot_list = GetBotListByClientName(owner_name);
	if (client_bot_list.empty()) {
		return;
	}

	for (const auto& b : client_bot_list) {
		b->Signal(signal_id);
	}
}

void EntityList::SignalBotByBotID(uint32 bot_id, int signal_id)
{
	auto b = GetBotByBotID(bot_id);
	if (b) {
		b->Signal(signal_id);
	}
}

void EntityList::SignalBotByBotName(std::string bot_name, int signal_id)
{
	auto b = GetBotByBotName(bot_name);
	if (b) {
		b->Signal(signal_id);
	}
}

void EntityList::GetCorpseList(std::list<Corpse *> &c_list)
{
	c_list.clear();
	auto it = corpse_list.begin();
	while (it != corpse_list.end()) {
		c_list.push_back(it->second);
		++it;
	}
}

void EntityList::GetObjectList(std::list<Object *> &o_list)
{
	o_list.clear();
	auto it = object_list.begin();
	while (it != object_list.end()) {
		o_list.push_back(it->second);
		++it;
	}
}

void EntityList::GetDoorsList(std::list<Doors*> &o_list)
{
	o_list.clear();
	auto it = door_list.begin();
	while (it != door_list.end()) {
		o_list.push_back(it->second);
		++it;
	}
}

void EntityList::GetSpawnList(std::list<Spawn2*> &o_list)
{
	o_list.clear();
	if(zone) {
		LinkedListIterator<Spawn2*> iterator(zone->spawn2_list);
		iterator.Reset();
		while(iterator.MoreElements())
		{
			Spawn2 *ent = iterator.GetData();
			o_list.push_back(ent);
			iterator.Advance();
		}
	}
}

void EntityList::UpdateQGlobal(uint32 qid, QGlobal newGlobal)
{
	auto it = mob_list.begin();
	while (it != mob_list.end()) {
		Mob *ent = it->second;

		if (ent->IsClient()) {
			QGlobalCache *qgc = ent->CastToClient()->GetQGlobals();
			if (qgc) {
				uint32 char_id = ent->CastToClient()->CharacterID();
				if (newGlobal.char_id == char_id && newGlobal.npc_id == 0)
					qgc->AddGlobal(qid, newGlobal);
			}
		} else if (ent->IsNPC()) {
			QGlobalCache *qgc = ent->CastToNPC()->GetQGlobals();
			if (qgc) {
				uint32 npc_id = ent->GetNPCTypeID();
				if (newGlobal.npc_id == npc_id)
					qgc->AddGlobal(qid, newGlobal);
			}
		}
		++it;
	}
}

void EntityList::DeleteQGlobal(std::string name, uint32 npcID, uint32 charID, uint32 zoneID)
{
	auto it = mob_list.begin();
	while (it != mob_list.end()) {
		Mob *ent = it->second;

		if (ent->IsClient()) {
			QGlobalCache *qgc = ent->CastToClient()->GetQGlobals();
			if (qgc)
				qgc->RemoveGlobal(name, npcID, charID, zoneID);
		} else if (ent->IsNPC()) {
			QGlobalCache *qgc = ent->CastToNPC()->GetQGlobals();
			if (qgc)
				qgc->RemoveGlobal(name, npcID, charID, zoneID);
		}
		++it;
	}
}

void EntityList::SendFindableNPCList(Client *c)
{
	if (!c) {
		return;
	}

	static EQApplicationPacket p(OP_SendFindableNPCs, sizeof(FindableNPC_Struct));
	auto b = (FindableNPC_Struct*) p.pBuffer;
	b->Unknown109 = 0x16;
	b->Unknown110 = 0x06;
	b->Unknown111 = 0x24;
	b->Action = 0;

	auto it = npc_list.begin();
	while (it != npc_list.end()) {
		if (it->second) {
			NPC *n = it->second;

			if (n->IsFindable()) {
				b->EntityID = n->GetID();
				strn0cpy(b->Name, n->GetCleanName(), sizeof(b->Name));
				strn0cpy(b->LastName, n->GetLastName(), sizeof(b->LastName));
				b->Race = n->GetRace();
				b->Class = n->GetClass();

				c->QueuePacket(&p);
			}
		}
		++it;
	}
}

void EntityList::UpdateFindableNPCState(NPC *n, bool Remove)
{
	if (!n || !n->IsFindable()) {
		return;
	}

	static EQApplicationPacket p(OP_SendFindableNPCs, sizeof(FindableNPC_Struct));
	auto                       b = (FindableNPC_Struct *) p.pBuffer;
	b->Unknown109 = 0x16;
	b->Unknown110 = 0x06;
	b->Unknown111 = 0x24;

	b->Action   = Remove ? 1 : 0;
	b->EntityID = n->GetID();
	strn0cpy(b->Name, n->GetCleanName(), sizeof(b->Name));
	strn0cpy(b->LastName, n->GetLastName(), sizeof(b->LastName));
	b->Race  = n->GetRace();
	b->Class = n->GetClass();

	auto it = client_list.begin();
	while (it != client_list.end()) {
		Client *c = it->second;
		if (c && (c->ClientVersion() >= EQ::versions::ClientVersion::SoD)) {
			c->QueuePacket(&p);
		}

		++it;
	}
}

void EntityList::HideCorpses(Client *c, uint8 CurrentMode, uint8 NewMode)
{
	if (!c) {
		return;
	}

	if (NewMode == HideCorpseNone) {
		SendZoneCorpses(c);
		return;
	}

	Group *g = nullptr;

	if (NewMode == HideCorpseAllButGroup) {
		g = c->GetGroup();

		if (!g) {
			NewMode = HideCorpseAll;
		}
	}

	auto it = corpse_list.begin();
	while (it != corpse_list.end()) {
		Corpse *b = it->second;

		if (b && (b->GetCharID() != c->CharacterID())) {
			if ((NewMode == HideCorpseAll) || ((NewMode == HideCorpseNPC) && (b->IsNPCCorpse()))) {
				EQApplicationPacket outapp;
					b->CreateDespawnPacket(&outapp, false);
				c->QueuePacket(&outapp);
			} else if(NewMode == HideCorpseAllButGroup) {
				if (!g->IsGroupMember(b->GetOwnerName())) {
					EQApplicationPacket outapp;
						b->CreateDespawnPacket(&outapp, false);
					c->QueuePacket(&outapp);
				} else if((CurrentMode == HideCorpseAll)) {
					EQApplicationPacket outapp;
						b->CreateSpawnPacket(&outapp);
					c->QueuePacket(&outapp);
				}
			}
		}
		++it;
	}
}

void EntityList::AddLootToNPCS(uint32 item_id, uint32 count)
{
	if (count == 0)
		return;

	int npc_count = 0;
	auto it = npc_list.begin();
	while (it != npc_list.end()) {
		if (!it->second->IsPet()
				&& it->second->GetClass() != Class::LDoNTreasure
				&& it->second->GetBodyType() != BodyType::NoTarget
				&& it->second->GetBodyType() != BodyType::NoTarget2
				&& it->second->GetBodyType() != BodyType::Special)
			npc_count++;
		++it;
	}

	if (npc_count == 0)
		return;

	auto npcs = new NPC *[npc_count];
	auto counts = new int[npc_count];
	auto marked = new bool[npc_count];
	memset(counts, 0, sizeof(int) * npc_count);
	memset(marked, 0, sizeof(bool) * npc_count);

	int i = 0;
	it = npc_list.begin();
	while (it != npc_list.end()) {
		if (!it->second->IsPet()
				&& it->second->GetClass() != Class::LDoNTreasure
				&& it->second->GetBodyType() != BodyType::NoTarget
				&& it->second->GetBodyType() != BodyType::NoTarget2
				&& it->second->GetBodyType() != BodyType::Special)
			npcs[i++] = it->second;
		++it;
	}

	while (count > 0) {
		std::vector<int> selection;
		selection.reserve(npc_count);
		for (int j = 0; j < npc_count; ++j)
			selection.push_back(j);

		while (!selection.empty() && count > 0) {
			int k = zone->random.Int(0, selection.size() - 1);
			counts[selection[k]]++;
			count--;
			selection.erase(selection.begin() + k);
		}
	}

	for (int j = 0; j < npc_count; ++j)
		if (counts[j] > 0)
			for (int k = 0; k < counts[j]; ++k)
				npcs[j]->AddItem(item_id, 1);

	safe_delete_array(npcs);
	safe_delete_array(counts);
	safe_delete_array(marked);
}

void EntityList::CameraEffect(uint32 duration, float intensity)
{
	auto outapp = new EQApplicationPacket(OP_CameraEffect, sizeof(Camera_Struct));
	Camera_Struct* cs = (Camera_Struct*) outapp->pBuffer;
	cs->duration = duration;	// Duration in milliseconds
	cs->intensity = intensity;
	entity_list.QueueClients(0, outapp);
	safe_delete(outapp);
}


NPC *EntityList::GetClosestBanker(Mob *sender, uint32 &distance)
{
	if (!sender)
		return nullptr;

	distance = 4294967295u;
	NPC *nc = nullptr;

	auto it = npc_list.begin();
	while (it != npc_list.end()) {
		if (it->second->GetClass() == Class::Banker) {
			uint32 nd = ((it->second->GetY() - sender->GetY()) * (it->second->GetY() - sender->GetY())) +
				((it->second->GetX() - sender->GetX()) * (it->second->GetX() - sender->GetX()));
			if (nd < distance){
				distance = nd;
				nc = it->second;
			}
		}
		++it;
	}
	return nc;
}

void EntityList::ExpeditionWarning(uint32 minutes_left)
{
	auto outapp = new EQApplicationPacket(OP_DzExpeditionEndsWarning, sizeof(ExpeditionExpireWarning));
	ExpeditionExpireWarning *ew = (ExpeditionExpireWarning*)outapp->pBuffer;
	ew->minutes_remaining = minutes_left;

	auto it = client_list.begin();
	while (it != client_list.end()) {
		it->second->MessageString(Chat::Yellow, DZ_MINUTES_REMAIN, itoa((int)minutes_left));
		it->second->QueuePacket(outapp);
		++it;
	}
	safe_delete(outapp);
}

Mob *EntityList::GetClosestMobByBodyType(Mob *sender, uint8 BodyType, bool skip_client_pets)
{

	if (!sender)
		return nullptr;

	uint32 CurrentDistance, ClosestDistance = 4294967295u;
	Mob *CurrentMob, *ClosestMob = nullptr;

	auto it = mob_list.begin();
	while (it != mob_list.end()) {
		CurrentMob = it->second;
		++it;

		if (CurrentMob->GetBodyType() != BodyType)
			continue;

		// Do not detect client pets
		if (skip_client_pets && CurrentMob->IsPet() && CurrentMob->IsPetOwnerOfClientBot())
			continue;

		CurrentDistance = ((CurrentMob->GetY() - sender->GetY()) * (CurrentMob->GetY() - sender->GetY())) +
					((CurrentMob->GetX() - sender->GetX()) * (CurrentMob->GetX() - sender->GetX()));

		if (CurrentDistance < ClosestDistance) {
			ClosestDistance = CurrentDistance;
			ClosestMob = CurrentMob;
		}
	}
	return ClosestMob;
}

void EntityList::GetTargetsForConeArea(Mob *start, float min_radius, float radius, float height, int pcnpc, std::list<Mob*> &m_list)
{
	auto it = mob_list.begin();
	while (it !=  mob_list.end()) {
		Mob *ptr = it->second;
		if (ptr == start) {
			++it;
			continue;
		}
		// check PC/NPC only flag 1 = PCs, 2 = NPCs
		if (pcnpc == 1 && !ptr->IsClient() && !ptr->IsMerc() && !ptr->IsBot()) {
			++it;
			continue;
		} else if (pcnpc == 2 && (ptr->IsClient() || ptr->IsMerc() || ptr->IsBot())) {
			++it;
			continue;
		}
		if (ptr->IsClient() && !ptr->CastToClient()->ClientFinishedLoading()) {
			++it;
			continue;
		}
		if (ptr->IsAura() || ptr->IsTrap()) {
			++it;
			continue;
		}

		float x_diff = ptr->GetX() - start->GetX();
		float y_diff = ptr->GetY() - start->GetY();
		float z_diff = ptr->GetZ() - start->GetZ();

		x_diff *= x_diff;
		y_diff *= y_diff;
		z_diff *= z_diff;

		if ((x_diff + y_diff) <= (radius * radius) && (x_diff + y_diff) >= (min_radius * min_radius))
			if(z_diff <= (height * height))
				m_list.push_back(ptr);

		++it;
	}
}

Client *EntityList::FindCorpseDragger(uint16 CorpseID)
{
	auto it = client_list.begin();
	while (it != client_list.end()) {
		if (it->second->IsDraggingCorpse(CorpseID))
			return it->second;
		++it;
	}
	return nullptr;
}

std::vector<Mob*> EntityList::GetTargetsForVirusEffect(Mob *spreader, Mob *original_caster, int range, int pcnpc, int32 spell_id)
{
	/*
		Live Mechanics
		Virus spreader does NOT need LOS
		There is no max target limit
	*/
	if (!spreader) {
		return {};
	}

	std::vector<Mob *> spreader_list        = {};
	bool               is_detrimental_spell = IsDetrimentalSpell(spell_id);
	for (auto          &it : spreader->GetCloseMobList(range)) {
		Mob *mob = it.second;
		if (!mob) {
			continue;
		}

		if (mob == spreader) {
			continue;
		}

		// check PC/NPC only flag 1 = PCs, 2 = NPCs
		if (pcnpc == 1 && !mob->IsClient() && !mob->IsMerc() && !mob->IsBot()) {
			continue;
		}
		else if (pcnpc == 2 && (mob->IsClient() || mob->IsMerc() || mob->IsBot())) {
			continue;
		}
		if (mob->IsClient() && !mob->CastToClient()->ClientFinishedLoading()) {
			continue;
		}

		if (mob->IsAura() || mob->IsTrap()) {
			continue;
		}

		// Make sure the target is in range
		if (mob->CalculateDistance(spreader->GetX(), spreader->GetY(), spreader->GetZ()) <= range) {

			if (!original_caster) {
				continue;
			}

			//Do not allow detrimental spread to anything the original caster couldn't normally attack.
			if (is_detrimental_spell && !original_caster->IsAttackAllowed(mob, true)) {
				continue;
			}

			//For non-NPCs, do not allow beneficial spread to anything the original caster could normally attack.
			if (!is_detrimental_spell && !original_caster->IsNPC() && original_caster->IsAttackAllowed(mob, true)) {
				continue;
			}

			// If the spreader is an npc and NOT a PET, then spread to other npc controlled mobs that are not pets
			if (spreader->IsNPC() && !spreader->IsPet() && !spreader->IsTempPet() && mob->IsNPC() && !mob->IsPet() && !mob->IsTempPet()) {
				spreader_list.push_back(mob);
			}
			// If the spreader is an npc and NOT a PET, then spread to npc controlled pet
			else if (spreader->IsNPC() && !spreader->IsPet() && !spreader->IsTempPet() && mob->IsNPC() && (mob->IsPet() || mob->IsTempPet()) && mob->IsPetOwnerNPC()) {
				spreader_list.push_back(mob);
			}
			// If the spreader is an npc controlled PET it can spread to any other npc or an npc controlled pet
			else if (spreader->IsNPC() && (spreader->IsPet() || spreader->IsTempPet()) && spreader->IsPetOwnerNPC()) {
				if (mob->IsNPC() && (!mob->IsPet() || !mob->IsTempPet())) {
					spreader_list.push_back(mob);
				}
				else if (mob->IsNPC() && (mob->IsPet() || mob->IsTempPet()) && mob->IsPetOwnerNPC()) {
					spreader_list.push_back(mob);
				}
			}
			// if the spreader is anything else(bot, pet, etc) then it should spread to everything but non client controlled npcs
			else if (!spreader->IsNPC() && !mob->IsNPC()) {
				spreader_list.push_back(mob);
			}
			// if spreader is not an NPC, and Target is an NPC, then spread to non-NPC controlled pets
			else if (!spreader->IsNPC() && mob->IsNPC() && (mob->IsPet() || mob->IsTempPet()) && !mob->IsPetOwnerNPC()) {
				spreader_list.push_back(mob);
			}

			// if spreader is a non-NPC controlled pet we need to determine appropriate targets(pet to client, pet to pet, pet to bot, etc)
			else if (spreader->IsNPC() && (spreader->IsPet() || spreader->IsTempPet()) && !spreader->IsPetOwnerNPC()) {
				//Spread to non-NPCs
				if (!mob->IsNPC()) {
					spreader_list.push_back(mob);
				}
				//Spread to other non-NPC Pets
				else if (mob->IsNPC() && (mob->IsPet() || mob->IsTempPet()) && !mob->IsPetOwnerNPC()) {
					spreader_list.push_back(mob);
				}
			}
		}
	}

	return spreader_list;
}

void EntityList::StopMobAI()
{
	for (auto &mob : mob_list) {
		mob.second->AI_Stop();
		mob.second->AI_ShutDown();
	}
}

void EntityList::SendAlternateAdvancementStats() {
	for (auto &c : client_list) {
		c.second->Message(Chat::White, "Reloading AA");
		c.second->ReloadExpansionProfileSetting();
		if (!database.LoadAlternateAdvancement(c.second)) {
			c.second->Message(Chat::Red, "Error loading alternate advancement character data");
		}

		c.second->SendClearPlayerAA();
		c.second->SendAlternateAdvancementTable();
		c.second->SendAlternateAdvancementStats();
		c.second->SendAlternateAdvancementPoints();
	}
}

void EntityList::ReloadMerchants() {
	for (auto it = npc_list.begin();it != npc_list.end(); ++it) {
		NPC *cur = it->second;
		if (cur->MerchantType != 0) {
			zone->LoadNewMerchantData(cur->MerchantType);
		}
	}
}

/**
 * If we have a distance requested that is greater than our scanning distance
 * then we return the full list
 *
 * See comments @EntityList::ScanCloseMobs for system explanation
 */
std::unordered_map<uint16, Mob *> &EntityList::GetCloseMobList(Mob *mob, float distance)
{
	if (distance <= RuleI(Range, MobCloseScanDistance)) {
		return mob->m_close_mobs;
	}

	return mob_list;
}

void EntityList::GateAllClientsToSafeReturn()
{
	DynamicZone* dz = zone ? zone->GetDynamicZone() : nullptr;

	for (const auto& client_list_iter : client_list)
	{
		if (client_list_iter.second)
		{
			// falls back to gating clients to bind if dz invalid
			client_list_iter.second->GoToDzSafeReturnOrBind(dz);
		}
	}
}

int EntityList::MovePlayerCorpsesToGraveyard(bool force_move_from_instance)
{
	if (!zone)
	{
		return 0;
	}

	int moved_count = 0;

	for (auto it = corpse_list.begin(); it != corpse_list.end();)
	{
		bool moved = false;
		if (it->second && it->second->IsPlayerCorpse())
		{
			if (zone->HasGraveyard())
			{
				moved = it->second->MovePlayerCorpseToGraveyard();
			}
			else if (force_move_from_instance && zone->GetInstanceID() != 0)
			{
				moved = it->second->MovePlayerCorpseToNonInstance();
			}
		}

		if (moved)
		{
			safe_delete(it->second);
			free_ids.push(it->first);
			it = corpse_list.erase(it);
			++moved_count;
		}
		else
		{
			++it;
		}
	}

	return moved_count;
}

void EntityList::DespawnGridNodes(int32 grid_id) {
	for (auto m : mob_list) {
		Mob *mob = m.second;
		if (
			mob->IsNPC() &&
			mob->GetRace() == Race::Node &&
			mob->EntityVariableExists("grid_id") &&
			Strings::ToInt(mob->GetEntityVariable("grid_id")) == grid_id)
		{
			mob->Depop();
		}
	}
}

void EntityList::Marquee(uint32 type, std::string message, uint32 duration) {
	for (const auto& c : client_list) {
		if (c.second) {
			c.second->SendMarqueeMessage(type, message, duration);
		}
	}
}

void EntityList::Marquee(
	uint32 type,
	uint32 priority,
	uint32 fade_in,
	uint32 fade_out,
	uint32 duration,
	std::string message
) {
	for (const auto& c : client_list) {
		if (c.second) {
			c.second->SendMarqueeMessage(type, priority, fade_in, fade_out, duration, message);
		}
	}
}

std::vector<Mob*> EntityList::GetFilteredEntityList(Mob* sender, uint32 distance, EntityFilterType filter_type)
{
	std::vector<Mob*> l;
	if (!sender) {
		return l;
	}

	const auto squared_distance = (distance * distance);
	const auto position = sender->GetPosition();
	for (auto &m: mob_list) {
		if (!m.second) {
			continue;
		}

		if (m.second == sender) {
			continue;
		}

		if (
			distance &&
			DistanceSquaredNoZ(
				position,
				m.second->GetPosition()
			) > squared_distance
		) {
			continue;
		}

		if (
			(filter_type == EntityFilterType::Bots && !m.second->IsBot()) ||
			(filter_type == EntityFilterType::Clients && !m.second->IsClient()) ||
			(filter_type == EntityFilterType::NPCs && !m.second->IsNPC())
		) {
			continue;
		}

		l.push_back(m.second);
	}

	return l;
}

void EntityList::DamageArea(
	Mob* sender,
	int64 damage,
	uint32 distance,
	EntityFilterType filter_type,
	bool is_percentage
) {
	if (!sender) {
		return;
	}

	if (damage <= 0) {
		return;
	}

	const auto& l = GetFilteredEntityList(sender, distance, filter_type);
	for (const auto& e : l) {
		if (is_percentage) {
			const auto damage_percentage = EQ::Clamp(damage, static_cast<int64>(1), static_cast<int64>(100));
			const auto total_damage = (e->GetMaxHP() / 100) * damage_percentage;
			e->Damage(sender, total_damage, SPELL_UNKNOWN, EQ::skills::SkillEagleStrike);
		} else {
			e->Damage(sender, damage, SPELL_UNKNOWN, EQ::skills::SkillEagleStrike);
		}
	}
}

std::vector<NPC*> EntityList::GetNPCsByIDs(std::vector<uint32> npc_ids)
{
	std::vector<NPC*> v;

	for (const auto& e : GetNPCList()) {
		const auto& n = std::find(npc_ids.begin(), npc_ids.end(), e.second->GetNPCTypeID());
		if (e.second) {
			if (n != npc_ids.end()) {
				continue;
			}

			v.emplace_back(e.second);
		}
	}

	return v;
}

std::vector<NPC*> EntityList::GetExcludedNPCsByIDs(std::vector<uint32> npc_ids)
{
	std::vector<NPC*> v;

	for (const auto& e : GetNPCList()) {
		const auto& n = std::find(npc_ids.begin(), npc_ids.end(), e.second->GetNPCTypeID());
		if (e.second) {
			if (n == npc_ids.end()) {
				continue;
			}

			v.emplace_back(e.second);
		}
	}

	return v;
}

void EntityList::SendMerchantEnd(Mob* merchant)
{
	for (const auto& e : client_list) {
		Client* c = e.second;

		if (!c) {
			continue;
		}

		if (c->GetMerchantSessionEntityID() == merchant->GetID()) {
			c->SendMerchantEnd();
		}
	}
}

void EntityList::SendMerchantInventory(Mob* m, int32 slot_id, bool is_delete)
{
	if (!m || !m->IsNPC()) {
		return;
	}

	for (const auto& e : client_list) {
		Client* c = e.second;

		if (!c) {
			continue;
		}

		if (c->GetMerchantSessionEntityID() == m->GetID()) {
			if (!is_delete) {
				c->BulkSendMerchantInventory(m->CastToNPC()->MerchantType, m->GetNPCTypeID());
			} else {
				auto app = new EQApplicationPacket(OP_ShopDelItem, sizeof(Merchant_DelItem_Struct));
				auto d   = (Merchant_DelItem_Struct*)app->pBuffer;

				d->itemslot   = slot_id;
				d->npcid      = m->GetID();
				d->playerid   = c->GetID();

				app->priority = 6;

				c->QueuePacket(app);
				safe_delete(app);
			}
		}
	}

	return;
}

void EntityList::RestoreCorpse(NPC *npc, uint32_t decay_time)
{
	uint16 corpse_id = npc->GetID();
	npc->Death(npc, npc->GetHP() + 1, SPELL_UNKNOWN, EQ::skills::SkillHandtoHand);
	auto c = entity_list.GetCorpseByID(corpse_id);
	if (c) {
		c->UnLock();
		c->SetDecayTimer(decay_time);
	}
}

void EntityList::CheckToClearTraderAndBuyerTables()
{
	if (zone->GetZoneID() == Zones::BAZAAR) {
		TraderRepository::DeleteWhere(
			database,
			fmt::format(
				"`char_zone_id` = {} AND `char_zone_instance_id` = {}", zone->GetZoneID(), zone->GetInstanceID()
			)
		);
		BuyerRepository::DeleteBuyers(database, zone->GetZoneID(), zone->GetInstanceID());

		LogTradingDetail(
			"Removed trader and buyer entries for Zone ID [{}] and Instance ID [{}]",
			zone->GetZoneID(),
			zone->GetInstanceID()
		);
	}
}
