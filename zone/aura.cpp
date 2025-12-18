#include "common/repositories/auras_repository.h"
#include "common/strings.h"
#include "zone/aura.h"
#include "zone/client.h"
#include "zone/raids.h"
#include "zone/string_ids.h"

Aura::Aura(NPCType *type_data, Mob *owner, AuraRecord &record)
	: NPC(type_data, 0, owner->GetPosition(), GravityBehavior::Flying), spell_id(record.spell_id),
	  distance(record.distance),
	  remove_timer(record.duration), movement_timer(1000), process_timer(1000), aura_id(-1)
{
	GiveNPCTypeData(type_data); // we will delete this later on
	m_owner = owner->GetID();

	if (record.cast_time) {
		cast_timer.SetTimer(record.cast_time);
		cast_timer.Disable(); // we don't want to be enabled yet
	}

	if (record.aura_type < static_cast<int>(AuraType::Max)) {
		type = static_cast<AuraType>(record.aura_type);
	}
	else {
		type = AuraType::OnAllGroupMembers;
	}

	if (record.spawn_type < static_cast<int>(AuraSpawns::Max)) {
		spawn_type = static_cast<AuraSpawns>(record.spawn_type);
	}
	else {
		spawn_type = AuraSpawns::GroupMembers;
	}

	if (record.movement < static_cast<int>(AuraMovement::Max)) {
		movement_type = static_cast<AuraMovement>(record.movement);
	}
	else {
		movement_type = AuraMovement::Follow;
	}

	switch (type) {
		case AuraType::OnAllFriendlies:
			process_func = &Aura::ProcessOnAllFriendlies;
			break;
		case AuraType::OnAllGroupMembers:
			process_func = &Aura::ProcessOnAllGroupMembers;
			break;
		case AuraType::OnGroupMembersPets:
			process_func = &Aura::ProcessOnGroupMembersPets;
			break;
		case AuraType::Totem:
			process_func = &Aura::ProcessTotem;
			break;
		case AuraType::EnterTrap:
			process_func = &Aura::ProcessEnterTrap;
			break;
		case AuraType::ExitTrap:
			process_func = &Aura::ProcessExitTrap;
			break;
		default:
			process_func = nullptr;
	}
}

Mob *Aura::GetOwner()
{
	return entity_list.GetMob(m_owner);
}

// not 100% sure how this one should work and PVP affects ...
void Aura::ProcessOnAllFriendlies(Mob *owner)
{
	auto          &mob_list = GetCloseMobList(distance);
	std::set<int> delayed_remove;
	bool          is_buff   = IsBuffSpell(spell_id); // non-buff spells don't cast on enter

	for (auto &e : mob_list) {
		auto mob = e.second;
		if (!mob) {
			continue;
		}
		if (mob->IsOfClientBotMerc() || mob->IsPetOwnerOfClientBot()) {
			auto it = casted_on.find(mob->GetID());

			if (it != casted_on.end()) { // we are already on the list, let's check for removal
				if (DistanceSquared(GetPosition(), mob->GetPosition()) > distance) {
					delayed_remove.insert(mob->GetID());
				}
			}
			else { // not on list, lets check if we're in range
				if (DistanceSquared(GetPosition(), mob->GetPosition()) <= distance) {
					casted_on.insert(mob->GetID());
					if (is_buff) {
						SpellFinished(spell_id, mob);
					}
				}
			}
		}
	}

	for (auto &e : delayed_remove) {
		auto mob = entity_list.GetMob(e);
		if (mob != nullptr && is_buff) { // some auras cast instant spells so no need to remove
			mob->BuffFadeBySpellIDAndCaster(spell_id, GetID());
		}
		casted_on.erase(e);
	}

	// so if we have a cast timer and our set isn't empty and timer is disabled we need to enable it
	if (cast_timer.GetDuration() > 0 && !cast_timer.Enabled() && !casted_on.empty()) {
		cast_timer.Start();
	}

	if (!cast_timer.Enabled() || !cast_timer.Check()) {
		return;
	}

	for (auto &e : casted_on) {
		auto mob = entity_list.GetMob(e);
		if (mob != nullptr) {
			SpellFinished(spell_id, mob);
		}
	}
}

void Aura::ProcessOnAllGroupMembers(Mob *owner)
{
	auto          &mob_list = GetCloseMobList(distance);
	std::set<int> delayed_remove;
	bool          is_buff   = IsBuffSpell(spell_id); // non-buff spells don't cast on enter

	if (owner->IsRaidGrouped() && owner->IsOfClientBot()) { // currently raids are just client, but safety check
		auto raid = owner->GetRaid();
		if (raid == nullptr) { // well shit
			owner->RemoveAura(GetID(), false, true);
			return;
		}
		auto group_id = raid->GetGroup(owner->CastToClient());

		// some lambdas so the for loop is less horrible ...
		auto verify_raid_client = [&raid, &group_id, this](Client *c) {
			auto idx = raid->GetPlayerIndex(c);
			if (c->GetID() == m_owner) {
				return DistanceSquared(GetPosition(), c->GetPosition()) <= distance;
			}
			else if (idx == 0xFFFFFFFF || raid->members[idx].group_number != group_id ||
					 raid->members[idx].group_number == 0xFFFFFFFF) {
				return false;
			}
			else if (DistanceSquared(GetPosition(), c->GetPosition()) > distance) {
				return false;
			}
			return true;
		};

		auto verify_raid_client_pet = [&raid, &group_id, this](Mob *m) {
			auto idx = raid->GetPlayerIndex(m->GetOwner()->CastToClient());
			if (m->GetOwner()->GetID() == m_owner) {
				return DistanceSquared(GetPosition(), m->GetPosition()) <= distance;
			}
			else if (idx == 0xFFFFFFFF || raid->members[idx].group_number != group_id ||
					 raid->members[idx].group_number == 0xFFFFFFFF) {
				return false;
			}
			else if (DistanceSquared(GetPosition(), m->GetPosition()) > distance) {
				return false;
			}
			return true;
		};

		auto verify_raid_client_swarm = [&raid, &group_id, this](NPC *n) {
			auto owner = entity_list.GetMob(n->GetSwarmOwner());
			if (owner == nullptr) {
				return false;
			}
			auto idx = raid->GetPlayerIndex(owner->CastToClient());
			if (owner->GetID() == m_owner) {
				return DistanceSquared(GetPosition(), n->GetPosition()) <= distance;
			}
			else if (idx == 0xFFFFFFFF || raid->members[idx].group_number != group_id ||
					 raid->members[idx].group_number == 0xFFFFFFFF) {
				return false;
			}
			else if (DistanceSquared(GetPosition(), n->GetPosition()) > distance) {
				return false;
			}
			return true;
		};

		for (auto &e : mob_list) {
			auto mob = e.second;
			if (!mob) {
				continue;
			}
			// step 1: check if we're already managing this NPC's buff
			auto it  = casted_on.find(mob->GetID());
			if (it != casted_on.end()) {
				// verify still good!
				if (mob->IsOfClientBot()) {
					if (!verify_raid_client(mob->CastToClient())) {
						delayed_remove.insert(mob->GetID());
					}
				}
				else if (mob->IsPet() && mob->IsPetOwnerOfClientBot() && mob->GetOwner()) {
					if (!verify_raid_client_pet(mob)) {
						delayed_remove.insert(mob->GetID());
					}
				}
				else if (mob->IsNPC() && mob->IsPetOwnerOfClientBot()) {
					auto npc = mob->CastToNPC();
					if (!verify_raid_client_swarm(npc)) {
						delayed_remove.insert(mob->GetID());
					}
				}
			}
			else { // we're not on it!
				if (mob->IsOfClientBot() && verify_raid_client(mob->CastToClient())) {
					casted_on.insert(mob->GetID());
					if (is_buff) {
						SpellFinished(spell_id, mob);
					}
				}
				else if (mob->IsPet() && mob->IsPetOwnerOfClientBot() && mob->GetOwner() && verify_raid_client_pet(mob)) {
					casted_on.insert(mob->GetID());
					if (is_buff) {
						SpellFinished(spell_id, mob);
					}
				}
				else if (mob->IsNPC() && mob->IsPetOwnerOfClientBot()) {
					auto npc = mob->CastToNPC();
					if (verify_raid_client_swarm(npc)) {
						casted_on.insert(mob->GetID());
						if (is_buff) {
							SpellFinished(spell_id, mob);
						}
					}
				}
			}
		}
	}
	else if (owner->IsGrouped()) {
		auto group = owner->GetGroup();
		if (group == nullptr) { // uh oh
			owner->RemoveAura(GetID(), false, true);
			return;
		}

		// lambdas to make for loop less ugly
		auto verify_group_pet = [&group, this](Mob *m) {
			auto owner = m->GetOwner();
			if (owner != nullptr && group->IsGroupMember(owner) &&
				DistanceSquared(GetPosition(), m->GetPosition()) <= distance) {
				return true;
			}
			return false;
		};

		auto verify_group_swarm = [&group, this](NPC *n) {
			auto owner = entity_list.GetMob(n->GetSwarmOwner());
			if (owner != nullptr && group->IsGroupMember(owner) &&
				DistanceSquared(GetPosition(), n->GetPosition()) <= distance) {
				return true;
			}
			return false;
		};

		for (auto &e : mob_list) {
			auto mob = e.second;
			auto it  = casted_on.find(mob->GetID());

			if (it != casted_on.end()) { // make sure we're still valid
				if (mob->IsPet()) {
					if (!verify_group_pet(mob)) {
						delayed_remove.insert(mob->GetID());
					}
				}
				else if (mob->IsNPC() && mob->CastToNPC()->GetSwarmInfo()) {
					if (!verify_group_swarm(mob->CastToNPC())) {
						delayed_remove.insert(mob->GetID());
					}
				}
				else if (!group->IsGroupMember(mob) || DistanceSquared(GetPosition(), mob->GetPosition()) > distance) {
					delayed_remove.insert(mob->GetID());
				}
			}
			else { // not on, check if we should be!
				if (mob->IsPet() && verify_group_pet(mob)) {
					casted_on.insert(mob->GetID());
					if (is_buff) {
						SpellFinished(spell_id, mob);
					}
				}
				else if (mob->IsNPC() && mob->CastToNPC()->GetSwarmInfo() && verify_group_swarm(mob->CastToNPC())) {
					casted_on.insert(mob->GetID());
					if (is_buff) {
						SpellFinished(spell_id, mob);
					}
				}
				else if (group->IsGroupMember(mob) && DistanceSquared(GetPosition(), mob->GetPosition()) <= distance) {
					casted_on.insert(mob->GetID());
					if (is_buff) {
						SpellFinished(spell_id, mob);
					}
				}
			}
		}
	}
	else {
		auto      verify_solo = [&owner, this](Mob *m) {
			if (m->IsPet() && m->GetOwnerID() == owner->GetID()) {
				return true;
			}
			else if (m->IsNPC() && m->CastToNPC()->GetSwarmOwner() == owner->GetID()) {
				return true;
			}
			else if (m->GetID() == owner->GetID()) {
				return true;
			}
			else {
				return false;
			}
		};
		for (auto &e : mob_list) {
			auto mob  = e.second;
			auto it   = casted_on.find(mob->GetID());
			bool good = verify_solo(mob);

			if (it != casted_on.end()) { // make sure still valid
				if (!good || DistanceSquared(GetPosition(), mob->GetPosition()) > distance) {
					delayed_remove.insert(mob->GetID());
				}
			}
			else if (good && DistanceSquared(GetPosition(), mob->GetPosition()) <= distance) {
				casted_on.insert(mob->GetID());
				if (is_buff) {
					SpellFinished(spell_id, mob);
				}
			}
		}
	}

	for (auto &e : delayed_remove) {
		auto mob = entity_list.GetMob(e);
		if (mob != nullptr && is_buff) { // some auras cast instant spells so no need to remove
			mob->BuffFadeBySpellIDAndCaster(spell_id, GetID());
		}
		casted_on.erase(e);
	}

	// so if we have a cast timer and our set isn't empty and timer is disabled we need to enable it
	if (cast_timer.GetDuration() > 0 && !cast_timer.Enabled() && !casted_on.empty()) {
		cast_timer.Start();
	}

	if (!cast_timer.Enabled() || !cast_timer.Check()) {
		return;
	}

	// some auras have to recast (DRU for example, non-buff too)
	for (auto &e : casted_on) {
		auto mob = entity_list.GetMob(e);
		if (mob != nullptr) {
			SpellFinished(spell_id, mob);
		}
	}
}

void Aura::ProcessOnGroupMembersPets(Mob *owner)
{
	auto          &mob_list    = GetCloseMobList(distance);
	std::set<int> delayed_remove;
	bool          is_buff      = IsBuffSpell(spell_id); // non-buff spells don't cast on enter
	// This type can either live on the pet (level 55/70 MAG aura) or on the pet owner (level 85 MAG aura)
	auto          group_member = owner->GetOwnerOrSelf();

	if (group_member->IsRaidGrouped() &&
		group_member->IsOfClientBot()) { // currently raids are just client, but safety check
		auto raid = group_member->GetRaid();
		if (raid == nullptr) { // well shit
			owner->RemoveAura(GetID(), false, true);
			return;
		}
		auto group_id = raid->GetGroup(group_member->CastToClient());

		// some lambdas so the for loop is less horrible ...
		auto verify_raid_client_pet = [&raid, &group_id, &group_member, this](Mob *m) {
			auto idx = raid->GetPlayerIndex(m->GetOwner()->CastToClient());
			if (m->GetOwner()->GetID() == group_member->GetID()) {
				return DistanceSquared(GetPosition(), m->GetPosition()) <= distance;
			}
			else if (idx == 0xFFFFFFFF || raid->members[idx].group_number != group_id ||
					 raid->members[idx].group_number == 0xFFFFFFFF) {
				return false;
			}
			else if (DistanceSquared(GetPosition(), m->GetPosition()) > distance) {
				return false;
			}
			return true;
		};

		auto verify_raid_client_swarm = [&raid, &group_id, &group_member, this](NPC *n) {
			auto owner = entity_list.GetMob(n->GetSwarmOwner());
			if (owner == nullptr) {
				return false;
			}
			auto idx = raid->GetPlayerIndex(owner->CastToClient());
			if (owner->GetID() == group_member->GetID()) {
				return DistanceSquared(GetPosition(), n->GetPosition()) <= distance;
			}
			else if (idx == 0xFFFFFFFF || raid->members[idx].group_number != group_id ||
					 raid->members[idx].group_number == 0xFFFFFFFF) {
				return false;
			}
			else if (DistanceSquared(GetPosition(), n->GetPosition()) > distance) {
				return false;
			}
			return true;
		};

		for (auto &e : mob_list) {
			auto mob = e.second;
			if (!mob) {
				continue;
			}
			// step 1: check if we're already managing this NPC's buff
			auto it  = casted_on.find(mob->GetID());
			if (it != casted_on.end()) {
				// verify still good!
				if (mob->IsPet() && mob->IsPetOwnerOfClientBot() && mob->GetOwner()) {
					if (!verify_raid_client_pet(mob)) {
						delayed_remove.insert(mob->GetID());
					}
				}
				else if (mob->IsNPC() && mob->IsPetOwnerOfClientBot()) {
					auto npc = mob->CastToNPC();
					if (!verify_raid_client_swarm(npc)) {
						delayed_remove.insert(mob->GetID());
					}
				}
			}
			else { // we're not on it!
				if (mob->IsOfClientBot()) {
					continue; // never hit client
				}
				else if (mob->IsPet() && mob->IsPetOwnerOfClientBot() && mob->GetOwner() && verify_raid_client_pet(mob)) {
					casted_on.insert(mob->GetID());
					if (is_buff) {
						SpellFinished(spell_id, mob);
					}
				}
				else if (mob->IsNPC() && mob->IsPetOwnerOfClientBot()) {
					auto npc = mob->CastToNPC();
					if (verify_raid_client_swarm(npc)) {
						casted_on.insert(mob->GetID());
						if (is_buff) {
							SpellFinished(spell_id, mob);
						}
					}
				}
			}
		}
	}
	else if (group_member->IsGrouped()) {
		auto group = group_member->GetGroup();
		if (group == nullptr) { // uh oh
			owner->RemoveAura(GetID(), false, true);
			return;
		}

		// lambdas to make for loop less ugly
		auto verify_group_pet = [&group, this](Mob *m) {
			auto owner = m->GetOwner();
			return owner != nullptr && group->IsGroupMember(owner) &&
				   DistanceSquared(GetPosition(), m->GetPosition()) <= distance;
		};

		auto verify_group_swarm = [&group, this](NPC *n) {
			auto owner = entity_list.GetMob(n->GetSwarmOwner());
			return owner != nullptr && group->IsGroupMember(owner) &&
				   DistanceSquared(GetPosition(), n->GetPosition()) <= distance;
		};

		for (auto &e : mob_list) {
			auto mob = e.second;
			auto it  = casted_on.find(mob->GetID());

			if (it != casted_on.end()) { // make sure we're still valid
				if (mob->IsPet()) {
					if (!verify_group_pet(mob)) {
						delayed_remove.insert(mob->GetID());
					}
				}
				else if (mob->IsNPC() && mob->CastToNPC()->GetSwarmInfo()) {
					if (!verify_group_swarm(mob->CastToNPC())) {
						delayed_remove.insert(mob->GetID());
					}
				}
			}
			else { // not on, check if we should be!
				if (mob->IsOfClientBot()) {
					continue;
				}
				else if (mob->IsPet() && verify_group_pet(mob)) {
					casted_on.insert(mob->GetID());
					if (is_buff) {
						SpellFinished(spell_id, mob);
					}
				}
				else if (mob->IsNPC() && mob->CastToNPC()->GetSwarmInfo() && verify_group_swarm(mob->CastToNPC())) {
					casted_on.insert(mob->GetID());
					if (is_buff) {
						SpellFinished(spell_id, mob);
					}
				}
			}
		}
	}
	else {
		auto      verify_solo = [&group_member, this](Mob *m) {
			if (m->IsPet() && m->GetOwnerID() == group_member->GetID()) {
				return true;
			}
			else if (m->IsNPC() && m->CastToNPC()->GetSwarmOwner() == group_member->GetID()) {
				return true;
			}
			else {
				return false;
			}
		};
		for (auto &e : mob_list) {
			auto mob  = e.second;
			auto it   = casted_on.find(mob->GetID());
			bool good = verify_solo(mob);

			if (it != casted_on.end()) { // make sure still valid
				if (!good || DistanceSquared(GetPosition(), mob->GetPosition()) > distance) {
					delayed_remove.insert(mob->GetID());
				}
			}
			else if (good && DistanceSquared(GetPosition(), mob->GetPosition()) <= distance) {
				casted_on.insert(mob->GetID());
				if (is_buff) {
					SpellFinished(spell_id, mob);
				}
			}
		}
	}

	for (auto &e : delayed_remove) {
		auto mob = entity_list.GetMob(e);
		if (mob != nullptr && is_buff) { // some auras cast instant spells so no need to remove
			mob->BuffFadeBySpellIDAndCaster(spell_id, GetID());
		}
		casted_on.erase(e);
	}

	// so if we have a cast timer and our set isn't empty and timer is disabled we need to enable it
	if (cast_timer.GetDuration() > 0 && !cast_timer.Enabled() && !casted_on.empty()) {
		cast_timer.Start();
	}

	if (!cast_timer.Enabled() || !cast_timer.Check()) {
		return;
	}

	// some auras have to recast (DRU for example, non-buff too)
	for (auto &e : casted_on) {
		auto mob = entity_list.GetMob(e);
		if (mob != nullptr) {
			SpellFinished(spell_id, mob);
		}
	}
}

void Aura::ProcessTotem(Mob *owner)
{
	auto          &mob_list = GetCloseMobList(distance);
	std::set<int> delayed_remove;
	bool          is_buff   = IsBuffSpell(spell_id); // non-buff spells don't cast on enter

	for (auto &e : mob_list) {
		auto mob = e.second;
		if (!mob) {
			continue;
		}

		if (mob == this) {
			continue;
		}
		if (mob == owner) {
			continue;
		}
		if (owner->IsAttackAllowed(mob)) { // might need more checks ...
			bool in_range = DistanceSquared(GetPosition(), mob->GetPosition()) <= distance;
			auto it       = casted_on.find(mob->GetID());
			if (it != casted_on.end()) {
				if (!in_range) {
					delayed_remove.insert(mob->GetID());
				}
			}
			else if (in_range) {
				casted_on.insert(mob->GetID());
				SpellFinished(spell_id, mob);
			}
		}
	}

	for (auto &e : delayed_remove) {
		auto mob = entity_list.GetMob(e);
		if (mob != nullptr && is_buff) { // some auras cast instant spells so no need to remove
			mob->BuffFadeBySpellIDAndCaster(spell_id, GetID());
		}
		casted_on.erase(e);
	}

	// so if we have a cast timer and our set isn't empty and timer is disabled we need to enable it
	if (cast_timer.GetDuration() > 0 && !cast_timer.Enabled() && !casted_on.empty()) {
		cast_timer.Start();
	}

	if (!cast_timer.Enabled() || !cast_timer.Check()) {
		return;
	}

	for (auto &e : casted_on) {
		auto mob = entity_list.GetMob(e);
		if (mob != nullptr) {
			SpellFinished(spell_id, mob);
		}
	}
}

void Aura::ProcessEnterTrap(Mob *owner)
{
	for (auto &e : GetCloseMobList(distance)) {
		auto mob = e.second;
		if (!mob) {
			continue;
		}

		if (mob == this) {
			continue;
		}
		// might need more checks ...
		if (mob != owner && owner->IsAttackAllowed(mob) && DistanceSquared(GetPosition(), mob->GetPosition()) <= distance) {
			SpellFinished(spell_id, mob);
			owner->RemoveAura(GetID(), false); // if we're a buff (ex. NEC) we don't want to strip :P
			break;
		}
	}
}

void Aura::ProcessExitTrap(Mob *owner)
{
	for (auto &e : GetCloseMobList(distance)) {
		auto mob = e.second;
		if (!mob) {
			continue;
		}

		if (mob == this) {
			continue;
		}
		// might need more checks ...
		if (mob != owner && owner->IsAttackAllowed(mob)) {
			bool in_range = DistanceSquared(GetPosition(), mob->GetPosition()) <= distance;
			auto it       = casted_on.find(mob->GetID());
			if (it != casted_on.end()) {
				if (!in_range) {
					SpellFinished(spell_id, mob);
					owner->RemoveAura(GetID(), false); // if we're a buff we don't want to strip :P
					break;
				}
			}
			else if (in_range) {
				casted_on.insert(mob->GetID());
			}
		}
	}
}

// this is less than ideal, but other solutions are a bit all over the place
// and hard to reason about
void Aura::ProcessSpawns()
{
	for (auto &e: GetCloseMobList(distance)) {
		if (!e.second) {
			continue;
		}

		if (!e.second->IsClient()) {
			continue;
		}

		auto c = e.second->CastToClient();

		if (!c) {
			continue;
		}

		bool spawned = spawned_for.find(c->GetID()) != spawned_for.end();
		if (ShouldISpawnFor(c)) {
			if (!spawned) {
				EQApplicationPacket app;
				CreateSpawnPacket(&app, this);
				c->QueuePacket(&app);
				SendArmorAppearance(c);
				spawned_for.insert(c->GetID());
			}
		}
		else if (spawned) {
			EQApplicationPacket app;
			CreateDespawnPacket(&app, false);
			c->QueuePacket(&app);
			spawned_for.erase(c->GetID());
		}
	}
}

bool Aura::Process()
{
	// Aura::Depop clears buffs
	if (p_depop) {
		return false;
	}

	auto owner = entity_list.GetMob(m_owner);
	if (owner == nullptr) {
		Depop();
		return true;
	}

	if (remove_timer.Check()) {
		owner->RemoveAura(GetID(), false, true);
		return true;
	}

	if (movement_type == AuraMovement::Follow && GetPosition() != owner->GetPosition() && movement_timer.Check()) {
		m_Position = owner->GetPosition();

		static EQApplicationPacket packet(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
		auto                       spu = (PlayerPositionUpdateServer_Struct *) packet.pBuffer;

		MakeSpawnUpdate(spu);
		auto it = spawned_for.begin();
		while (it != spawned_for.end()) {
			auto client = entity_list.GetClientByID(*it);
			if (client) {
				client->QueuePacket(&packet);
				++it;
			}
			else {
				it = spawned_for.erase(it);
			}
		}
	}
	// TODO: waypoints?

	if (!process_timer.Check()) {
		return true;
	}

	if (spawn_type != AuraSpawns::Noone) {
		ProcessSpawns();
	} // bit of a hack

	if (process_func) {
		process_func(*this, owner);
	}

	// TODO: quest calls
	return true;
}

bool Aura::ShouldISpawnFor(Client *c)
{
	if (spawn_type == AuraSpawns::Noone) {
		return false;
	}

	if (spawn_type == AuraSpawns::Everyone) {
		return true;
	}

	// hey, it's our owner!
	if (c->GetID() == m_owner) {
		return true;
	}

	// so this one is a bit trickier
	auto owner = GetOwner();
	if (owner == nullptr) {
		return false;
	} // hmm

	owner = owner->GetOwnerOrSelf(); // pet auras we need the pet's owner
	if (owner == nullptr) { // shouldn't really be needed
		return false;
	}

	// gotta check again for pet aura case -.-
	if (owner == c) {
		return true;
	}

	if (owner->IsRaidGrouped() && owner->IsOfClientBot()) {
		auto raid = owner->GetRaid();
		if (raid == nullptr) {
			return false;
		} // hmm
		auto group_id = raid->GetGroup(owner->CastToClient());
		if (group_id == 0xFFFFFFFF) { // owner handled above, and they're in a raid and groupless
			return false;
		}

		auto idx = raid->GetPlayerIndex(c);
		if (idx == 0xFFFFFFFF) { // they're not in our raid!
			return false;
		}

		if (raid->members[idx].group_number != group_id) { // in our raid, but not our group
			return false;
		}

		return true; // we got here so we know that 1 they're in our raid and 2 they're in our group!
	}
	else if (owner->IsGrouped()) {
		auto group = owner->GetGroup();
		if (group == nullptr) {
			return false;
		} // hmm

		// easy, in our group
		return group->IsGroupMember(c);
	}

	// our owner is not raided or grouped, and they're handled above so we don't spawn!
	return false;
}

void Aura::Depop(bool skip_strip)
{
	// NEC trap casts a dot, so we need some way to not strip :P
	if (!skip_strip && IsBuffSpell(spell_id)) {
		for (auto &e : casted_on) {
			auto mob = entity_list.GetMob(e);
			if (mob != nullptr) {
				mob->BuffFadeBySpellIDAndCaster(spell_id, GetID());
			}
		}
	}
	casted_on.clear();
	p_depop = true;
}

void Mob::MakeAura(uint16 spell_id)
{
	// TODO: verify room in AuraMgr
	if (!IsValidSpell(spell_id)) {
		return;
	}

	AuraRecord record{};
	if (!content_db.GetAuraEntry(spell_id, record)) {
		Message(Chat::Red, "Unable to find data for aura %s", spells[spell_id].name);
		LogError("Unable to find data for aura [{}], check auras table", spell_id);
		return;
	}

	if (!IsValidSpell(record.spell_id)) {
		Message(Chat::Red, "Casted spell (%d) is not valid for aura %s", record.spell_id, spells[spell_id].name);
		LogError("Casted spell ([{}]) is not valid for aura [{}], check auras table",
				 record.spell_id, spell_id);
		return;
	}

	if (record.aura_type > static_cast<int>(AuraType::Max)) {
		return; // TODO: log
	}

	bool trap = false;

	switch (static_cast<AuraType>(record.aura_type)) {
		case AuraType::ExitTrap:
		case AuraType::EnterTrap:
		case AuraType::Totem:
			trap = true;
			break;
		default:
			trap = false;
			break;
	}

	if (!CanSpawnAura(trap)) {
		return;
	}

	const auto base = content_db.LoadNPCTypesData(record.npc_type);
	if (base == nullptr) {
		Message(Chat::Red, "Unable to load NPC data for aura %s", spells[spell_id].teleport_zone);
		LogError(
			"Unable to load NPC data for aura [{}] (NPC ID [{}]), check auras and npc_types tables",
			spells[spell_id].teleport_zone,
			record.npc_type
		);
		return;
	}

	auto npc_type = new NPCType;
	memcpy(npc_type, base, sizeof(NPCType));

	strn0cpy(npc_type->name, record.name, 64);

	auto npc = new Aura(npc_type, this, record);
	npc->SetAuraID(spell_id);
	if (trap) {
		npc->TryMoveAlong(5.0f, 0.0f, false);
	} // try to place 5 units in front
	entity_list.AddNPC(npc, false);

	if (trap) {
		AddTrap(npc, record);
	}
	else {
		AddAura(npc, record);
	}
}

bool ZoneDatabase::GetAuraEntry(uint16 spell_id, AuraRecord& r)
{
	const auto& l = AurasRepository::GetWhere(
		*this,
		fmt::format(
			"`type` = {}",
			spell_id
		)
	);

	if (l.empty()) {
		return false;
	}

	auto& e = l.front();

	strn0cpy(r.name, e.name.c_str(), sizeof(r.name));

	r.npc_type   = e.npc_type;
	r.spell_id   = e.spell_id;
	r.distance   = e.distance * e.distance;
	r.aura_type  = e.aura_type;
	r.spawn_type = e.spawn_type;
	r.movement   = e.movement;
	r.duration   = e.duration * 1000; // Database is in seconds
	r.icon       = e.icon;
	r.cast_time  = e.cast_time * 1000; // Database is in seconds

	return true;
}

void Mob::AddAura(Aura *aura, AuraRecord &record)
{
	if (!aura) {
		return;
	}

	LogAura(
		"aura owner [{}] spawn_id [{}] aura_name [{}]",
		GetCleanName(),
		aura->GetID(),
		aura->GetCleanName()
	);

	// this is called only when it's safe
	strn0cpy(aura_mgr.auras[aura_mgr.count].name, aura->GetCleanName(), 64);
	aura_mgr.auras[aura_mgr.count].spawn_id = aura->GetID();
	aura_mgr.auras[aura_mgr.count].aura     = aura;
	if (record.icon == -1) {
		aura_mgr.auras[aura_mgr.count].icon = spells[record.spell_id].new_icon;
	}
	else {
		aura_mgr.auras[aura_mgr.count].icon = record.icon;
	}

	if (IsClient()) {
		auto outapp      = new EQApplicationPacket(OP_UpdateAura, sizeof(AuraCreate_Struct));
		auto aura_create = (AuraCreate_Struct *) outapp->pBuffer;
		aura_create->action = 0;
		aura_create->type   = 1; // this can be 0 sometimes too
		strn0cpy(aura_create->aura_name, aura_mgr.auras[aura_mgr.count].name, 64);
		aura_create->entity_id = aura_mgr.auras[aura_mgr.count].spawn_id;
		aura_create->icon      = aura_mgr.auras[aura_mgr.count].icon;
		CastToClient()->FastQueuePacket(&outapp);
	}
	// we can increment this now
	aura_mgr.count++;
}

void Mob::AddTrap(Aura *aura, AuraRecord &record)
{
	if (!aura) {
		return;
	}

	LogAura(
		"aura owner [{}] spawn_id [{}] aura_name [{}]",
		GetCleanName(),
		aura->GetID(),
		aura->GetCleanName()
	);

	// this is called only when it's safe
	strn0cpy(trap_mgr.auras[trap_mgr.count].name, aura->GetCleanName(), 64);
	trap_mgr.auras[trap_mgr.count].spawn_id = aura->GetID();
	trap_mgr.auras[trap_mgr.count].aura     = aura;
	if (record.icon == -1) {
		trap_mgr.auras[trap_mgr.count].icon = spells[record.spell_id].new_icon;
	}
	else {
		trap_mgr.auras[trap_mgr.count].icon = record.icon;
	}
	// doesn't send to client
	trap_mgr.count++;
}

bool Mob::CanSpawnAura(bool trap)
{
	if (trap && !HasFreeTrapSlots()) {
		MessageString(Chat::SpellFailure, NO_MORE_TRAPS);
		return false;
	}
	else if (!trap && !HasFreeAuraSlots()) {
		MessageString(Chat::SpellFailure, NO_MORE_AURAS);
		return false;
	}

	return true;
}

void Mob::RemoveAllAuras()
{
	if (IsClient()) {
		database.SaveAuras(CastToClient());
		EQApplicationPacket outapp(OP_UpdateAura, 4);
		outapp.WriteUInt32(2);
		CastToClient()->QueuePacket(&outapp);
	}

	// this is sent on camp/zone, so it just despawns?
	if (aura_mgr.count) {
		for (auto &e : aura_mgr.auras) {
			if (e.aura) {
				LogAura(
					"aura owner [{}] spawn_id [{}] aura_name [{}]",
					GetCleanName(),
					e.spawn_id,
					e.name
				);

				e.aura->Depop();
			}
		}
	}

	aura_mgr.count = 0;

	if (trap_mgr.count) {
		for (auto &e : trap_mgr.auras) {
			if (e.aura) {
				LogAura(
					"trap owner [{}] spawn_id [{}] aura_name [{}]",
					GetCleanName(),
					e.spawn_id,
					e.name
				);

				e.aura->Depop();
			}
		}
	}

	trap_mgr.count = 0;
}

void Mob::RemoveAura(int spawn_id, bool skip_strip, bool expired)
{
	for (int i = 0; i < aura_mgr.count; ++i) {
		auto &aura = aura_mgr.auras[i];
		if (aura.spawn_id == spawn_id) {
			LogAura(
				"mob [{}] spawn_id [{}] skip_strip [{}] expired [{}]",
				GetCleanName(),
				spawn_id,
				skip_strip ? "true" : "false",
				expired ? "true" : "false"
			);

			if (aura.aura) {
				aura.aura->Depop(skip_strip);
			}
			if (expired && IsClient()) {
				// TODO: verify color
				CastToClient()->SendColoredText(
					Chat::Yellow,
					StringFormat("%s has expired.", aura.name)
				);
				// need to update client UI too
				auto app = new EQApplicationPacket(OP_UpdateAura, sizeof(AuraDestory_Struct));
				auto ads = (AuraDestory_Struct *) app->pBuffer;
				ads->action    = 1; // delete
				ads->entity_id = spawn_id;
				CastToClient()->QueuePacket(app);
				safe_delete(app);
			}
			while (aura_mgr.count - 1 > i) {
				i++;
				aura.spawn_id          = aura_mgr.auras[i].spawn_id;
				aura.icon              = aura_mgr.auras[i].icon;
				aura.aura              = aura_mgr.auras[i].aura;
				aura_mgr.auras[i].aura = nullptr;
				strn0cpy(aura.name, aura_mgr.auras[i].name, 64);
			}
			aura_mgr.count--;
			return;
		}
	}

	for (int i = 0; i < trap_mgr.count; ++i) {
		auto &aura = trap_mgr.auras[i];
		if (aura.spawn_id == spawn_id) {
			if (aura.aura) {
				aura.aura->Depop(skip_strip);
			}
			if (expired && IsClient()) {
				CastToClient()->SendColoredText(
					Chat::Yellow, StringFormat("%s has expired.", aura.name));
			} // TODO: verify color
			while (trap_mgr.count - 1 > i) {
				i++;
				aura.spawn_id          = trap_mgr.auras[i].spawn_id;
				aura.icon              = trap_mgr.auras[i].icon;
				aura.aura              = trap_mgr.auras[i].aura;
				trap_mgr.auras[i].aura = nullptr;
				strn0cpy(aura.name, trap_mgr.auras[i].name, 64);
			}
			trap_mgr.count--;
			return;
		}
	}

}

