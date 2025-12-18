
#pragma once

#include "common/timer.h"
#include "common/types.h"
#include "zone/mob.h"
#include "zone/npc.h"

#include <functional>
#include <set>

class Group;
class Raid;
class Mob;
struct NPCType;

enum class AuraType {
	OnAllFriendlies,		// AE PC/Pet basically (ex. Circle of Power)
	OnAllGroupMembers,		// Normal buffing aura (ex. Champion's Aura)
	OnGroupMembersPets,		// Hits just pets (ex. Rathe's Strength)
	Totem,					// Starts pulsing on a timer when an enemy enters (ex. Idol of Malos)
	EnterTrap,				// Casts once when an enemy enters (ex. Fire Rune)
	ExitTrap,				// Casts when they start to flee (ex. Poison Spikes Trap)
	FullyScripted,			// We just call script function not a predefined
	Max
};

enum class AuraSpawns {
	GroupMembers,			// most auras use this
	Everyone,				// this is like traps and clickies who cast on everyone
	Noone,					// custom!
	Max
};

enum class AuraMovement {
	Follow,					// follows caster
	Stationary,
	Pathing,				// some sorted pathing TODO: implement
	Max
};

class Aura : public NPC
{
	// NOTE: We may have to override more virtual functions if they're causing issues
public:
	Aura(NPCType *type_data, Mob *owner, AuraRecord &record);
	~Aura() { };

	bool IsAura() const { return true; }
	bool Process();
	void Depop(bool skip_strip = false);
	Mob *GetOwner();

	void ProcessOnAllFriendlies(Mob *owner);
	void ProcessOnAllGroupMembers(Mob *owner);
	void ProcessOnGroupMembersPets(Mob *owner);
	void ProcessTotem(Mob *owner);
	void ProcessEnterTrap(Mob *owner);
	void ProcessExitTrap(Mob *owner);
	void ProcessSpawns();

	// we only save auras that follow you, and player casted
	inline bool AuraZones() { return movement_type == AuraMovement::Follow && aura_id > -1; }
	inline int GetSpellID() { return spell_id; }
	inline int GetAuraID() { return aura_id; }
	inline void SetAuraID(int in) { aura_id = in; }

	bool ShouldISpawnFor(Client *c);
	// so when we join a group, we need to spawn not already spawned auras
	// This is only possible when spawn type is GroupMembers
	inline bool JoinGroupSpawnCheck() { return spawn_type == AuraSpawns::GroupMembers; }
private:
	int m_owner;
	int aura_id; // spell ID of the aura spell -1 if aura isn't from a casted spell
	int spell_id; // spell we cast
	float distance; // distance we remove
	Timer remove_timer; // when we depop
	Timer process_timer; // rate limit process calls
	Timer cast_timer; // some auras pulse
	Timer movement_timer; // rate limit movement updates
	AuraType type;
	AuraSpawns spawn_type;
	AuraMovement movement_type;

	std::function<void(Aura &, Mob *)> process_func;
	std::set<int> casted_on; // we keep track of the other entities we've casted on
	std::set<int> spawned_for;
};
